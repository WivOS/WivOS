#pragma once

#include <utils/common.h>
#include <cpu/acpi.h>

#include <lai/helpers/pci.h>

typedef struct pci_config_alloc {
    uint64_t base;
    uint16_t seg;
    uint8_t startBus;
    uint8_t endBus;
    uint32_t reserved;
} __attribute__((packed)) pci_config_alloc_t;

typedef struct mcfg {
    sdt_t sdt;
    uint64_t reserved;
    pci_config_alloc_t configs[];
} __attribute__((packed)) mcfg_t;

typedef struct {
    uint16_t seg;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
    size_t parent;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t rev_id;
    uint8_t subclass;
    uint8_t device_class;
    uint8_t prog_if;
    uint8_t irq_pin;
    uint32_t gsi;
    uint16_t gsi_flags;
    bool multifunction;

    lai_nsnode_t *acpiNode;

    bool has_prt;
    lai_variable_t acpiPrt;
} pci_device_t;

typedef struct {
    uint64_t base;
    size_t size;

    bool isMmio;
    bool isPrefetchable;
} pci_bar_t;

void pci_preinit();
void pci_init();

void pci_write_config(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t func, uint16_t offset,
                      uint32_t value, uint8_t writeSize);
uint32_t pci_read_config(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t func, uint16_t offset,
                         uint8_t readSize);

void pci_device_write_config(pci_device_t *device, uint16_t offset,
                      uint32_t value, uint8_t writeSize);
uint32_t pci_device_read_config(pci_device_t *device, uint16_t offset,
                         uint8_t readSize);

void pci_enable_busmastering(pci_device_t *device);
void pci_enable_mmio(pci_device_t *device);

pci_device_t *pci_get_device(uint8_t class, uint8_t subclass, uint8_t prog_if, size_t index);
pci_device_t *pci_get_device_by_vendor(uint16_t vendorID, uint16_t deviceID, size_t index);
bool pci_read_bar(pci_device_t *device, uint8_t bar, pci_bar_t *out);