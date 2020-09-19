#include <util/util.h>
#include <lai/host.h>

void *laihost_malloc(size_t size) {
    return kmalloc(size);
}

void *laihost_realloc(void *address, size_t size) {
    return krealloc(address, size);
}

void laihost_free(void *address) {
    return kfree(address);
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
    size_t pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    vmm_map_pages(kernel_pml4, (void *)(address + VIRT_PHYS_BASE), (void *)address, pageCount, 0x3);

    return (void *)(address + VIRT_PHYS_BASE);
}

void laihost_unmap(void *addr, size_t size) { // TODO
    return;
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