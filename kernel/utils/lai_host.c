#include "common.h"
#include "ctype.h"
#include <stdarg.h>

#include <mem/pmm.h>
#include <cpu/acpi.h>

#include <devices/pci.h>
#include <devices/timer.h>

void laihost_log(int level, const char *msg) {
    printf("[LAI] %s\n", msg);
}

__attribute__((noreturn)) void laihost_panic(const char *msg) {
    printf("[LAI] Panic: %s\n", msg);
    while(1);
}

void *laihost_malloc(size_t size) {
    return kmalloc(size);
}

void *laihost_realloc(void *oldptr, size_t newsize, size_t oldsize) {
    (void)oldsize;
    return krealloc(oldptr, newsize);
}

void laihost_free(void *ptr, size_t size) {
    (void)size;
    kfree(ptr);
}

void *laihost_map(size_t address, size_t count) {
    (void)count;
    return (void *)(address + MEM_PHYS_OFFSET);
}

void laihost_unmap(void *pointer, size_t count) {
    (void)pointer;
    (void)count;
}

void *laihost_scan(const char *sig, size_t index) {
    return acpi_get_table((char *)sig, index);
}

void laihost_outb(uint16_t port, uint8_t val) {
    outb(port, val);
}

void laihost_outw(uint16_t port, uint16_t val) {
    outw(port, val);
}

void laihost_outd(uint16_t port, uint32_t val) {
    outl(port, val);
}

uint8_t laihost_inb(uint16_t port) {
    return inb(port);
}

uint16_t laihost_inw(uint16_t port) {
    return inw(port);
}

uint32_t laihost_ind(uint16_t port) {
    return inl(port);
}

//TODO: Important
void laihost_sleep(uint64_t ms) {
    ksleep(ms);
}

uint64_t laihost_timer(void) {
    return timer_get_nanoseconds() / 100;
}

void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
    pci_write_config(seg, bus, slot, fun, offset, val, 1);
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
    pci_write_config(seg, bus, slot, fun, offset, val, 2);
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
    pci_write_config(seg, bus, slot, fun, offset, val, 4);
}

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    return pci_read_config(seg, bus, slot, fun, offset, 1);
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    return pci_read_config(seg, bus, slot, fun, offset, 2);
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    return pci_read_config(seg, bus, slot, fun, offset, 4);
}
