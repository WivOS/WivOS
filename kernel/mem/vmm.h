#pragma once

#include <utils/common.h>
#include <boot/limine.h>

#include <utils/spinlock.h>

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

#define VIRT_KRNL_BASE 0xFFFFFFFF80000000UL

typedef struct {
    uint64_t entries[512];
} pt_entries_t;

typedef struct {
    pt_entries_t *entries;
    spinlock_t lock;
} pt_t;

extern pt_t *KernelPml4;

void vmm_init(void *kernelBase, void *physKernelBase, limine_memmap_entry_t **entries, size_t count);
pt_t *vmm_setup_pml4();

void vmm_free_pml4(pt_t *pml4);

bool vmm_map(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions);
bool vmm_map_huge(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions);
bool vmm_unmap(pt_t *pml4, void *addr, size_t size);

void vmm_read(pt_t *pml4addr, void *addr, uint8_t *buffer, size_t size);
void *vmm_get_phys(pt_t *pml4addr, void *addr);

void vmm_map_if_not_mapped(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions);

pt_t *vmm_copy_pml4(pt_t *pml4);