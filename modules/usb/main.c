#include "usb.h"

#include <tasking/scheduler.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <cpu/cpu.h>

#include <fs/devfs/devfs.h>

#include <utils/lists.h>

#include <devices/timer.h>

#define _DEBUG 0

static list_t *usbDrivers;
static list_t *usbDevices;

bool usb_get_descriptor(usb_device_t *device, uint8_t descriptorType, uint8_t descriptorIndex, uint16_t languageIndex, void *dest, uint16_t size) {
    usb_request_t request = {0};
    request.length = size;
    request.type = 0x80;
    request.request = 6;
    request.value = (descriptorType << 8) | descriptorIndex;
    request.index = languageIndex;
    return device->controller->send_control(device, dest, request, false);
}

void *usb_get_configuration_space(usb_device_t *device, uint8_t number) {
    usb_config_t config = {0};
    usb_get_descriptor(device, 2, 0, 0, &config, sizeof(usb_config_t));
    void *res = kmalloc(config.totalLength);
    usb_get_descriptor(device, 2, number, 0, res, config.totalLength);

    return res;
}

bool usb_set_configuration(usb_device_t *device, uint8_t number) {
    usb_request_t request = {0};
    request.length = 0;
    request.request = 9;
    request.value = number;
    return device->controller->send_control(device, NULL, request, true);
}

static char *get_string_from_index(usb_device_t *device, uint8_t index, uint16_t languageIndex) {
    usb_string_descriptor_t strDescriptorTemp = {0};
    usb_get_descriptor(device, 3, index, 0x0409, &strDescriptorTemp, sizeof(usb_string_descriptor_t));

    usb_string_descriptor_t *strDescriptor = kmalloc(strDescriptorTemp.length + 1);
    usb_get_descriptor(device, 3, index, 0x0409, strDescriptor, strDescriptorTemp.length);

    char *str = kmalloc((strDescriptorTemp.length - 0x2) / 0x2 + 1);
    for(int i = 0; i < (strDescriptorTemp.length - 0x2) / 0x2; i++) {
        str[i] = strDescriptor->uString[i * 2];
    }
    str[(strDescriptorTemp.length - 0x2) / 0x2] = 0;

    kfree(strDescriptor);
    return str;
}

usb_endpoint_t *usb_get_endpoint(usb_endpoint_t *endpoints, usb_endpoint_type_t endpointType, bool in) {
    for(size_t i = 0; i < 15; i++) {
        bool isIn = (endpoints[i].address & 0x80) > 0;
        usb_endpoint_type_t type = endpoints[i].attributes & 0x3;
        if(isIn == in && endpointType == type)
            return &endpoints[i];
    }

    return NULL;
}

void probe_drivers(usb_device_t *device) {
    if(device == NULL || device->assigned) return;

    bool assigned = false;
    for(size_t i = 0; i < device->descriptor.configCount; i++) {
        usb_config_t *config = usb_get_configuration_space(device, i);
        size_t interfaceBase = ((size_t)config + config->length);
        for(size_t j = 0; j < config->interfaceCount;) {
            usb_interface_t *interface = (usb_interface_t *)interfaceBase;
            interfaceBase += interface->length;

            if(interface->type != 0x4) continue; //INTERFACE

            if(_DEBUG) printf("[USB] Interface class: 0x%x subclass: 0x%x protocol: 0x%x\n", interface->class, interface->subClass, interface->protocol);

            foreach(node, usbDrivers) {
                usb_driver_t *driver = (usb_driver_t *)node->value;
                if(!driver) break;

                if(((driver->match & USB_MATCH_VENDOR) && (driver->vendor != device->vendor)) ||
                   ((driver->match & USB_MATCH_PRODUCT) && (driver->product != device->product)) ||
                   ((driver->match & USB_MATCH_USB_VERSION) && (driver->usbVersion != device->usbVersion)) ||
                   ((driver->match & USB_MATCH_DEVICE_VERSION) && (driver->deviceVersion != device->deviceVersion)) ||
                   ((driver->match & USB_MATCH_CLASS) && (driver->class != interface->class)) ||
                   ((driver->match & USB_MATCH_PROTOCOL) && (driver->protocol != interface->protocol)))
                    continue;

                //Driver match
                usb_endpoint_t endpoints[15] = {0};
                uint64_t base = (uint64_t)interface + interface->length;
                for(size_t l = 0; l < interface->endpointCount;) {
                    usb_endpoint_data_t *endpointInfo = (usb_endpoint_data_t *)base;
                    base += endpointInfo->length;
                    if(endpointInfo->type != 0x05) continue;

                    endpoints[l].address = endpointInfo->address;
                    endpoints[l].attributes = endpointInfo->attributes;
                    if((endpointInfo->attributes & 0x3) == 1) {
                        endpoints[l].interval = 1 << endpointInfo->interval; //Convert to ms
                    } else { //Camp at 8 to be sure low speed works for now
                        endpoints[l].interval = endpointInfo->interval < 8 ? 8 : endpointInfo->interval; //Convert to ms
                    }
                    endpoints[l].controllerEndpointNumber = device->controller->setup_endpoint(device, endpointInfo->address, endpointInfo->attributes, endpointInfo->maxPacketSize);
                    if(_DEBUG) printf("[USB] Endpoint addr: %x attr: %x num: %x\n", endpointInfo->address, endpointInfo->attributes, endpoints[l].controllerEndpointNumber);

                    l++;
                }

                usb_set_configuration(device, config->configurationValue);

                usb_interface_t *newInterface = kmalloc(sizeof(usb_interface_t));
                memcpy(newInterface, interface, sizeof(usb_interface_t));

                usb_endpoint_t *newEdpoints = kmalloc(sizeof(usb_endpoint_t) * 15);
                memcpy(newEdpoints, endpoints, sizeof(usb_endpoint_t) * 15);

                if(driver->probe(device, newInterface, newEdpoints)) assigned = true;
                else {
                    kfree(newInterface);
                }
            }

            j++;
        }
        kfree(config);
        if(assigned) {
            device->assigned = true;
            break;
        }
    }
}

bool usb_add_device(usb_device_t device, size_t devNumber) {
    device.controllerDevNumber = devNumber;

    usb_device_descriptor_t descriptor = {0};
    usb_get_descriptor(&device, 1, 0, 0, &descriptor, sizeof(usb_device_descriptor_t));
    char *serialNumber = get_string_from_index(&device, descriptor.serialNumber, 0x0409);

    printf("[USB] Vendor: 0x%x Product: 0x%x Class: 0x%x SubClass: 0x%x Serial: %s\n", descriptor.vendorId, descriptor.productId, descriptor.deviceClass, descriptor.deviceSubClass, serialNumber);
    kfree(serialNumber);

    device.configCount = descriptor.configCount;
    device.class = descriptor.deviceClass;
    device.subClass = descriptor.deviceSubClass;
    device.vendor = descriptor.vendorId;
    device.product = descriptor.productId;
    device.usbVersion = descriptor.usbVersion;
    device.deviceVersion = descriptor.deviceRelease;
    device.descriptor = descriptor;

    probe_drivers(&device);

    usb_device_t *deviceToAdd = kmalloc(sizeof(usb_device_t));
    memcpy(deviceToAdd, &device, sizeof(usb_device_t));
    list_push_back(usbDevices, deviceToAdd);

    return true;
}

void usb_register_driver(usb_driver_t driver) {
    usb_driver_t *newDriver = kmalloc(sizeof(usb_driver_t));
    *newDriver = driver;
    list_push_back(usbDrivers, newDriver);

    foreach(device_node, usbDevices) {
        probe_drivers((usb_device_t *)device_node->value);
    }
}

static size_t _init() {
    usbDrivers = list_create();
    usbDevices = list_create();

    return 1;
}

static size_t _exit() {
    printf("Goodbye :D\n");

    return 1;
}

MODULE_DEF(usb, _init, _exit);