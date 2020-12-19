#pragma once

#include <util/util.h>
#include <lai/core.h>

#define PCI_ROOT_INDEX 0xCF8
#define PCI_ROOT_DATA 0xCFC

typedef struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
    int64_t parent;
    uint16_t vendorID;
    uint16_t deviceID;
    uint8_t revID;
    uint8_t progIF;
    uint8_t subclass;
    uint8_t classCode;
    uint8_t multifunction;
    uint8_t irqPin;

    lai_nsnode_t *acpiNode;

    int hasPtr;
    lai_variable_t acpiPtr;

    uint32_t gsi;
    uint16_t gsiFlags;
} pci_device_t;

typedef struct {
    uint64_t base;
    size_t size;

    int is_mmio;
    int is_prefetchable;
} pci_bar_t;

uint8_t pci_read_byte(pci_device_t *device, uint32_t offset);
uint16_t pci_read_word(pci_device_t *device, uint32_t offset);
uint32_t pci_read_dword(pci_device_t *device, uint32_t offset);

void pci_write_byte(pci_device_t *device, uint32_t offset, uint8_t value);
void pci_write_word(pci_device_t *device, uint32_t offset, uint16_t value);
void pci_write_dword(pci_device_t *device, uint32_t offset, uint32_t value);

void pci_enable_busmastering(pci_device_t *device);

size_t pci_read_bar(pci_device_t *device, int bar, pci_bar_t *out);
pci_device_t *pci_get_device_by_vendor(uint16_t vendor, uint16_t device);
pci_device_t *pci_get_device_by_class_subclass(uint8_t class, uint8_t subclass, uint8_t progIF, size_t index);
size_t pci_register_msi(pci_device_t *device, uint8_t vector);

void pci_init();