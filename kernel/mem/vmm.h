#pragma once

#include <stdint.h>
#include <stddef.h>
#include <mem/pmm.h>

#define VMM_FLAG_MASK (0xFFF | (1ull << 63))
#define VMM_ADDR_MASK ~(VMM_FLAG_MASK)

#define VMM_PRESENT (1ULL<<0)
#define VMM_WRITE (1ULL<<1)
#define VMM_USER (1ULL<<2)
#define VMM_PAT0 (1ULL<<3)
#define VMM_PAT1 (1ULL<<4)
#define VMM_PAT2 (1ULL<<7)
#define VMM_DIRTY (1ULL<<5)
#define VMM_LARGE (1ULL<<7)
#define VMM_NX (1ULL<<63)

typedef struct {
    size_t pml4_off;
    size_t pdp_off;
	size_t pd_off;
	size_t pt_off;
} pt_offset_t;

typedef struct pt {
    uint64_t entries[512];
} pt_t;

extern pt_t *kernel_pml4;

void vmm_init();
void vmm_map_pages(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions);
void vmm_unmap_pages(pt_t *pml4, void *addr, size_t size);
void *vmm_get_phys(pt_t *pml4param, void *addr);
void vmm_map_pages_huge(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions);
void vmm_unmap_pages_huge(pt_t *pml4, void *addr, size_t size);
pt_t *create_new_pml4();
pt_t *fork_pml4(pt_t *pml4);