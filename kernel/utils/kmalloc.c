#include "common.h"
#include "ctype.h"
#include <stdarg.h>

#include <mem/pmm.h>

typedef struct kmem_header {
    size_t pages;
    size_t size;
} __attribute__((packed)) kmem_header_t;

void *kmalloc(size_t size) {
    size_t pageCount = size / PAGE_SIZE;
    if(size % PAGE_SIZE) pageCount++;

    char *retAddr = pmm_alloc(pageCount + 1);
    if(!retAddr) return NULL;

    retAddr += MEM_PHYS_OFFSET;
    kmem_header_t *header = (kmem_header_t *)retAddr;

    header->pages = pageCount;
    header->size = size;

    return (void *)((uint64_t)retAddr + PAGE_SIZE);
}

void kfree(void *addr) {
    kmem_header_t *header = (kmem_header_t *)((uint64_t)addr - PAGE_SIZE);
    pmm_free((void*)((uint64_t)header - MEM_PHYS_OFFSET), header->pages + 1);
}

void *krealloc(void *addr, size_t size) {
    if(!addr) return kmalloc(size);
    if(!size) {
        kfree(addr);
        return NULL;
    }

    kmem_header_t *header = (kmem_header_t *)((uint64_t)addr - PAGE_SIZE);

    if(ROUND_UP(header->size, PAGE_SIZE) == ROUND_UP(size, PAGE_SIZE)) {
        header->size = size;
        return addr;
    }

    void *newAddr = kmalloc(size);
    if(!newAddr) return NULL;

    if(header->size > size) memcpy(newAddr, addr, size);
    else memcpy(newAddr, addr, header->size);

    kfree(addr);

    return newAddr;
}

void *kcalloc(size_t num, size_t nsize) {
    if(!num || !nsize) return NULL;

    size_t size = num * nsize;
    if(nsize != (size / num)) return NULL;

    void *addr = kmalloc(size);
    if(!addr) return NULL;

    memset(addr, 0, size);
    return addr;
}