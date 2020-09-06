#include <mem/mm.h>
#include <util/lock.h>
#include <util/string.h>

typedef struct header {
    size_t pages;
    size_t size;
} header_t;

void *kmalloc(size_t size) {
    size_t page_size = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    char *ret = pmm_alloc(page_size + 1);

    if(!ret) {
        return NULL;
    }

    ret += VIRT_PHYS_BASE;

    header_t *header = (header_t *)ret;
    ret += PAGE_SIZE;

    header->pages = page_size;
    header->size = size;

    return (void *)ret;
}

void kfree(void *addr) {
    header_t *header = (header_t *)((size_t)addr - PAGE_SIZE);

    pmm_free((void *)((size_t)header - VIRT_PHYS_BASE), header->pages + 1);
}

void *krealloc(void *addr, size_t newSize) {
    if(!addr) return kmalloc(newSize);
    if(!newSize) {
        kfree(addr);
        return NULL;
    }

    header_t *header = (header_t *)((size_t)addr - PAGE_SIZE);
    
    if((header->size + PAGE_SIZE - 1) / PAGE_SIZE == (newSize + PAGE_SIZE - 1) / PAGE_SIZE) {
        header->size = newSize;
        return addr;
    }

    char *new_addr;
    if((new_addr = kmalloc(newSize)) == 0) {
        return NULL;
    }

    if(header->size > newSize)
        memcpy(new_addr, (char *)addr, newSize);
    else
        memcpy(new_addr, (char *)addr, header->size);

    kfree(addr);

    return (void *)new_addr;
}

void *kcalloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;
	if (!num || !nsize)
		return NULL;
	size = num * nsize;
	if (nsize != size / num)
		return NULL;
	block = kmalloc(size);
	if (!block)
		return NULL;
	memset(block, 0, size);
	return block;
}