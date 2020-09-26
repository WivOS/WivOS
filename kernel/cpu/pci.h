#pragma once

#include <util/util.h>

#define PCI_ROOT_INDEX 0xCF8
#define PCI_ROOT_DATA 0xCFC

typedef struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
} pci_device_t;

uint8_t pci_read_byte(pci_device_t *device, uint32_t offset);
uint16_t pci_read_word(pci_device_t *device, uint32_t offset);
uint32_t pci_read_dword(pci_device_t *device, uint32_t offset);

void pci_write_byte(pci_device_t *device, uint32_t offset, uint8_t value);
void pci_write_word(pci_device_t *device, uint32_t offset, uint16_t value);
void pci_write_dword(pci_device_t *device, uint32_t offset, uint32_t value);

void pci_init();