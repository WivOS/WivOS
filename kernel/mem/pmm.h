#pragma once

#define VIRT_PHYS_BASE 0xFFFF800000000000UL
#define PAGE_SIZE 0x1000

#include <boot/stivale2.h>
#include <stdint.h>
#include <stddef.h>

void pmm_init(stivale2_struct_t *stivale2);
void pmm_free(void *addr, size_t count);
void *pmm_alloc(size_t count);
void *pmm_alloc_nonzero(size_t count);
void *pmm_alloc_advanced(size_t count, size_t alignment, uint64_t upper);
