#pragma once

#include <util/util.h>

#define PCI_ROOT_INDEX 0xCF8
#define PCI_ROOT_DATA 0xCFC

typedef struct pci_device {
    uint8_t bus;
    uint8_t slot;
    uint8_t function;
} pci_device_t;

void pci_init();