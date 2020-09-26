#include <util/util.h>
#include <lai/host.h>
#include <cpu/pci.h>
#include "acpi.h"
#include <acpispec/tables.h>

void *laihost_malloc(size_t size) {
    return kmalloc(size);
}

void *laihost_realloc(void *address, size_t size) {
    return krealloc(address, size);
}

void laihost_free(void *address) {
    kfree(address);
}

void laihost_log(int logLevel, const char *msg) {
    switch(logLevel) {
        case LAI_DEBUG_LOG:
            printf("[LAI] [Debug] %s\n", msg);
            break;
        case LAI_WARN_LOG:
            printf("[LAI] [Warn] %s\n", msg);
            break;
        default:
            printf("[LAI] [Unknown] %s\n", msg);
            break;
    }
}

void laihost_panic(const char *msg) {
    printf("[LAI] [Panic] %s\n", msg);
    printf("Panic, halting...");
    asm volatile("cli");
    while(1) {
        asm volatile("hlt");
    }
}

void *laihost_map(size_t address, size_t size) {
    return (void *)(address + VIRT_PHYS_BASE);
}

void laihost_unmap(void *addr, size_t size) { // TODO
    return;
}

void *laihost_scan(const char *signature, size_t index) {
    if(!strncmp(signature, "DSDT", 4)) {
        if(index > 0) return NULL;

        acpi_fadt_t *fadtTable = (acpi_fadt_t *)acpi_find_sdt("FACP", 0);
        void *dsdtTable = (char *)((size_t)fadtTable->dsdt + VIRT_PHYS_BASE);
        return dsdtTable;
    } else {
        return acpi_find_sdt(signature, index);
    }
}

void laihost_outb(uint16_t p, uint8_t d) {
    outb(p, d);
}

void laihost_outw(uint16_t p, uint16_t d) {
    outw(p, d);
}

void laihost_outd(uint16_t p, uint32_t d) {
    outl(p, d);
}

uint8_t laihost_inb(uint16_t p) {
    return inb(p);
}

uint16_t laihost_inw(uint16_t p) {
    return inw(p);
}

uint32_t laihost_ind(uint16_t p) {
    return inl(p);
}

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint8_t value) {
    if(seg != 0) laihost_panic("Seg should be 0 for now");
    pci_device_t pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.slot = device;
    pciDevice.function = function;
    pci_write_byte(&pciDevice, offset, value);
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
    if(seg != 0) laihost_panic("Seg should be 0 for now");
    pci_device_t pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.slot = device;
    pciDevice.function = function;
    return pci_read_byte(&pciDevice, offset);
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint16_t value) {
    if(seg != 0) laihost_panic("Seg should be 0 for now");
    pci_device_t pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.slot = device;
    pciDevice.function = function;
    pci_write_word(&pciDevice, offset, value);
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
    if(seg != 0) laihost_panic("Seg should be 0 for now");
    pci_device_t pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.slot = device;
    pciDevice.function = function;
    return pci_read_word(&pciDevice, offset);
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint32_t value) {
    if(seg != 0) laihost_panic("Seg should be 0 for now");
    pci_device_t pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.slot = device;
    pciDevice.function = function;
    pci_write_dword(&pciDevice, offset, value);
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
    if(seg != 0) laihost_panic("Seg should be 0 for now");
    pci_device_t pciDevice = {0};
    pciDevice.bus = bus;
    pciDevice.slot = device;
    pciDevice.function = function;
    return pci_read_dword(&pciDevice, offset);
}