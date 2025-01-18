#pragma once

#ifndef _USERMODE_

#include <utils/common.h>
#include <modules/modules.h>
#include <devices/pci.h>

#include <tasking/scheduler.h>

#else

typedef size_t kpid_t;
typedef size_t ktid_t;

#endif

struct usb_controller;

typedef struct {
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) usb_request_t;

typedef struct {
    uint8_t length;
    uint8_t type;
    uint8_t number;
    uint8_t alternateSetting;
    uint8_t endpointCount;
    uint8_t class;
    uint8_t subClass;
    uint8_t protocol;
    uint8_t interface;
} __attribute__((packed)) usb_interface_t;

typedef struct {
    uint8_t length;
    uint8_t descriptorType;
    uint16_t usbVersion;
    uint8_t deviceClass;
    uint8_t deviceSubClass;
    uint8_t protocol;
    uint8_t maxPacketSize;
    uint16_t vendorId;
    uint16_t productId;
    uint16_t deviceRelease;
    uint8_t manufacturer;
    uint8_t product;
    uint8_t serialNumber;
    uint8_t configCount;
} __attribute__((packed)) usb_device_descriptor_t;

typedef struct {
    uint32_t address;
    size_t maxPacketSize;
    struct usb_controller *controller;
    void *data;
    size_t endpointCount;
    size_t claimed;
    size_t controllerDevNumber;
    usb_interface_t interface;

    uint16_t vendor;
    uint16_t product;
    uint16_t usbVersion;
    uint16_t deviceVersion;
    uint8_t class;
    uint8_t subClass;

    size_t configCount;

    bool assigned;
    usb_device_descriptor_t descriptor;
} usb_device_t;

typedef struct usb_controller {
    bool (*send_control)(usb_device_t *, void *, usb_request_t, bool);
    bool (*send_bulk)(usb_device_t *, char *, size_t, uint32_t, uint32_t);
    bool (*probe)(void *);
    size_t (*setup_endpoint)(usb_device_t *, uint32_t, uint32_t ,uint32_t);
} usb_controller_t;

typedef struct {
    uint8_t length;
    uint8_t descriptorType;
    char uString[];
} __attribute__((packed)) usb_string_descriptor_t;

typedef struct {
    uint8_t length;
    uint8_t type;
    uint16_t totalLength;
    uint8_t interfaceCount;
    uint8_t configurationValue;
    uint8_t configuration;
    uint8_t attributes;
    uint8_t maxPower;
} __attribute__((packed)) usb_config_t;

typedef struct {
    uint8_t length;
    uint8_t type;
    uint8_t address;
    uint8_t attributes;
    uint16_t maxPacketSize;
    uint8_t interval;
} __attribute__((packed)) usb_endpoint_data_t;

typedef struct {
    uint8_t address;
    uint8_t attributes;
    size_t controllerEndpointNumber;
    size_t interval;
} __attribute__((packed)) usb_endpoint_t;

typedef enum {
    USB_ENDPOINT_CONTROL = 0,
    USB_ENDPOINT_ISOCFRONOUS = 1,
    USB_ENDPOINT_BULK = 2,
    USB_ENDPOINT_INTERRUPT = 3
} usb_endpoint_type_t;

typedef enum {
    USB_MATCH_VENDOR = 0x1,
    USB_MATCH_PRODUCT = 0x2,
    USB_MATCH_USB_VERSION = 0x4,
    USB_MATCH_DEVICE_VERSION = 0x8,
    USB_MATCH_CLASS = 0x10,
    USB_MATCH_SUBCLASS = 0x20,
    USB_MATCH_PROTOCOL = 0x40
} usb_match_t;

typedef struct {
    usb_match_t match;
    uint16_t vendor;
    uint16_t product;
    uint16_t usbVersion;
    uint16_t deviceVersion;
    uint8_t class;
    uint8_t subClass;
    uint8_t protocol;

    bool (*probe)(usb_device_t *, usb_interface_t *, usb_endpoint_t *);
} usb_driver_t;

bool usb_add_device(usb_device_t device, size_t devNumber);
void usb_register_driver(usb_driver_t driver);

usb_endpoint_t *usb_get_endpoint(usb_endpoint_t *endpoints, usb_endpoint_type_t endpointType, bool in);
bool usb_get_descriptor(usb_device_t *device, uint8_t descriptorType, uint8_t descriptorIndex, uint16_t languageIndex, void *dest, uint16_t size);