#pragma once

#include <utils/common.h>
#include <boot/limine.h>

#define MEM_PHYS_OFFSET 0xFFFF800000000000
#define PAGE_SIZE       0x1000

void pmm_init(limine_memmap_entry_t **entries, size_t count);

void *pmm_alloc(size_t count);
void *pmm_alloc_advanced(size_t count, size_t alignment);
void pmm_free(void *addr, size_t count);