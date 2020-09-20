#include "pci.h"

static void pci_setAddress(pci_device_t *device, uint32_t offset) {
    uint32_t address = 0x80000000 | (device->bus << 16) | ((device->slot & 0x1F) << 11) | ((device->function & 0x7) << 8) | (offset & ~((uint32_t)(3)));
    outl(PCI_ROOT_INDEX, address);
}

uint8_t pci_read_byte(pci_device_t *device, uint32_t offset) {
    pci_setAddress(device, offset);
    return inb(PCI_ROOT_DATA);
}

uint16_t pci_read_word(pci_device_t *device, uint32_t offset) {
    pci_setAddress(device, offset);
    return inw(PCI_ROOT_DATA);
}

uint32_t pci_read_dword(pci_device_t *device, uint32_t offset) {
    pci_setAddress(device, offset);
    return inl(PCI_ROOT_DATA);
}

void pci_write_byte(pci_device_t *device, uint32_t offset, uint8_t value) {
    pci_setAddress(device, offset);
    outb(PCI_ROOT_DATA, value);
}

void pci_write_word(pci_device_t *device, uint32_t offset, uint16_t value) {
    pci_setAddress(device, offset);
    outw(PCI_ROOT_DATA, value);
}

void pci_write_dword(pci_device_t *device, uint32_t offset, uint32_t value) {
    pci_setAddress(device, offset);
    outl(PCI_ROOT_DATA, value);
}

void pci_init() {

}