#include "usbhid.h"

#include <tasking/scheduler.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <cpu/cpu.h>

#include <fs/devfs/devfs.h>
#include <fs/pipe/pipe.h>

#include <utils/lists.h>

#include <devices/timer.h>

#include "../usb/usb.h"

#define _DEBUG 0

typedef enum {
    HID_KEYBOARD = 0,
    HID_MOUSE = 1,
    HID_TABLET = 2
} hid_device_type_t;

typedef struct {
    usb_device_t *usb;
    //usb_interface_t *interface; //Maybe we need it later
    usb_endpoint_t *dataEndpoint;

    size_t pollingInterval;

    hid_device_type_t type;

    size_t startSleepTick;
    size_t waitUntilTick;

    vfs_node_t *pipes[2];
} hid_device_t;

static list_t *hidDevices;
static ktid_t hidThread = -1;

typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keypress1;
    uint8_t keypress2;
    uint8_t keypress3;
    uint8_t keypress4;
    uint8_t keypress5;
    uint8_t keypress6;
} __attribute__((packed)) usb_keyboard_report_t;

typedef struct {
    uint8_t buttons;
    uint16_t xOffset;
    uint16_t yOffset;
    int8_t zOffset;
} __attribute__((packed)) usb_mouse_report_t;

extern volatile size_t TimerCounter;
static void hid_thread_handler() {
    usb_keyboard_report_t *testReport = kmalloc(sizeof(usb_keyboard_report_t));

    uint64_t reportPrintTicks = 0;
    for(;;) {
        spinlock_lock(&hidDevices->lock);

        size_t timeToSleep = -1;
        foreach(deviceNode, hidDevices) {
            hid_device_t *device = (hid_device_t *)deviceNode->value;
            if(device == NULL) continue;

            if(device->waitUntilTick != device->startSleepTick && device->waitUntilTick > TimerCounter) {
                size_t timeToWait = ((device->waitUntilTick - TimerCounter) * 1000) / TIMER_FREQ;
                if((int64_t)timeToSleep > timeToWait)
                    timeToSleep = timeToWait;
                continue;
            }

            if(device->type == HID_KEYBOARD) {

                //Get report
                device->usb->controller->send_bulk(device->usb, (char *)testReport, sizeof(usb_keyboard_report_t), device->dataEndpoint->controllerEndpointNumber, true);

                if(reportPrintTicks % 5 == 0) {
                    uint8_t *keys = (uint8_t *)&testReport->keypress1;
                    uint8_t lastKey = 0;
                    for(;lastKey < 6; lastKey++) {
                        uint8_t key = keys[lastKey];
                        if(key == 0) {
                            if(lastKey)
                                lastKey--;
                            else
                                lastKey = 5;
                            break;
                        }
                    }
                    if(lastKey < 5)
                        vfs_write(device->pipes[1], (char *)&keys[lastKey], 1);
                }
                if(reportPrintTicks % 100 == 0) {
                    //printf("Keypresses: %x %x %x\n", testReport->modifier, testReport->keypress1, testReport->keypress2);
                }
            } else if(device->type == HID_TABLET) {
                //Get report here
                device->usb->controller->send_bulk(device->usb, (char *)testReport, sizeof(usb_mouse_report_t), device->dataEndpoint->controllerEndpointNumber, true);
                usb_mouse_report_t *mouseReport = (usb_mouse_report_t *)testReport;
                if(reportPrintTicks % 10 == 0) {
                    printf("Mouse status %02x %04x %04x %d\n", mouseReport->buttons, mouseReport->xOffset, mouseReport->yOffset, mouseReport->zOffset);
                }
            }

            device->startSleepTick = TimerCounter;
            device->waitUntilTick = TimerCounter + ( (device->pollingInterval * TIMER_FREQ) / 1000);
            if((int64_t)timeToSleep > device->pollingInterval)
                timeToSleep = device->pollingInterval;
        }
        reportPrintTicks++;

        spinlock_unlock(&hidDevices->lock);

        if((int64_t)timeToSleep > 0)
            thread_sleep(timeToSleep);
    }
}


static size_t usbhid_read(struct vfs_node *node, char *buffer, size_t size) {
    hid_device_t *device = (hid_device_t *)node->data;

    return vfs_read(device->pipes[0], buffer, size);
}

static bool test_hid_probe(usb_device_t *device, usb_interface_t *interface, usb_endpoint_t *endpoints) {
    hid_device_t *hidDevice = NULL;

    //Detect if the device is a keyboard a mouse or unsupported device:
    if(interface->protocol == 1) {
        //Keyboard
        hidDevice = (hid_device_t *)kmalloc(sizeof(hid_device_t));

        usb_endpoint_t *dataEndpoint = usb_get_endpoint(endpoints, USB_ENDPOINT_INTERRUPT, true);

        //Set polling rate to minimum, device will update this to minimum that supports
        usb_request_t request = {0};
        request.type = 0b00100001;
        request.request = 0xA; //SET_IDLE
        request.value = 0x100;
        request.length = 0;
        device->controller->send_control(device, NULL, request, true);

        usb_keyboard_report_t *testReport = kmalloc(sizeof(usb_keyboard_report_t));
        device->controller->send_bulk(device, (char *)testReport, sizeof(usb_keyboard_report_t), dataEndpoint->controllerEndpointNumber, true);

        printf("Keypresses: %x %x %x\n", testReport->modifier, testReport->keypress1, testReport->keypress2);

        hidDevice->usb = device;
        hidDevice->type = HID_KEYBOARD;
        hidDevice->dataEndpoint = dataEndpoint;
        hidDevice->pollingInterval = dataEndpoint->interval;
        hidDevice->startSleepTick = 0;
        hidDevice->waitUntilTick = 0;

        pipe_create(hidDevice->pipes);
        pipe_set_read_at_first_data(hidDevice->pipes[0], true);

        devfs_node_t *logNode = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
        strcpy(logNode->name, "USBHidLog");
        logNode->functions.read = usbhid_read;
        logNode->data = hidDevice;
        devfs_mount("kbd", logNode);

        printf("Probed success\n");
    } else if(interface->protocol == 0) {
        //Keyboard
        hidDevice = (hid_device_t *)kmalloc(sizeof(hid_device_t));

        usb_endpoint_t *dataEndpoint = usb_get_endpoint(endpoints, USB_ENDPOINT_INTERRUPT, true);

        //Set polling rate to minimum, device will update this to minimum that supports
        usb_request_t request = {0};
        request.type = 0b00100001;
        request.request = 0xA; //SET_IDLE
        request.value = 0x100;
        request.length = 0;
        device->controller->send_control(device, NULL, request, true);

        usb_mouse_report_t *testReport = kmalloc(sizeof(usb_mouse_report_t));
        device->controller->send_bulk(device, (char *)testReport, sizeof(usb_mouse_report_t), dataEndpoint->controllerEndpointNumber, true);

        printf("Mouse status: %x %d %d %d\n", testReport->buttons, testReport->xOffset, testReport->yOffset, testReport->zOffset);

        hidDevice->usb = device;
        hidDevice->type = HID_TABLET;
        hidDevice->dataEndpoint = dataEndpoint;
        hidDevice->pollingInterval = dataEndpoint->interval;
        hidDevice->startSleepTick = 0;
        hidDevice->waitUntilTick = 0;

        pipe_create(hidDevice->pipes);
        pipe_set_read_at_first_data(hidDevice->pipes[0], true);

        devfs_node_t *logNode = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
        strcpy(logNode->name, "USBHidLog");
        logNode->functions.read = usbhid_read;
        logNode->data = hidDevice;
        devfs_mount("mouse", logNode);

        printf("Probed success\n");
    } else {
        return false;
    }

    list_push_back(hidDevices, hidDevice);

    //Create handler task
    if(hidThread == -1) {
        hidThread = thread_create(0, thread_parameter_entry, thread_entry_data((void *)hid_thread_handler, NULL));

        scheduler_add_task(0, hidThread);
    }

    return true;
}

static size_t _init() {
    hidDevices = list_create();

    usb_driver_t testDriver = {0};
    testDriver.match = USB_MATCH_CLASS;
    testDriver.probe = test_hid_probe;
    testDriver.class = 0x3;
    usb_register_driver(testDriver);

    return 1;
}

static size_t _exit() {
    printf("Goodbye :D\n");

    return 1;
}

MODULE_DEF(usbhid, _init, _exit);