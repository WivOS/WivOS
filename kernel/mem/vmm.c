#include <mem/vmm.h>
#include <util/string.h>

static pt_offset_t addr_to_offsets(void *addr) {
    uint64_t addrtemp = (uint64_t)addr;

    pt_offset_t offset = {
        .pml4_off   = (addrtemp & ((size_t)0x1FF << 39)) >> 39,
        .pdp_off    = (addrtemp & ((size_t)0x1FF << 30)) >> 30,
        .pd_off     = (addrtemp & ((size_t)0x1FF << 21)) >> 21,
        .pt_off     = (addrtemp & ((size_t)0x1FF << 12)) >> 12,
    };

    return offset;
}

pt_t *kernel_pml4;

void vmm_init() {
    kernel_pml4 = create_new_pml4();
    vmm_map_pages_huge(kernel_pml4, (void *)0x0, NULL, 0x10, 3);
    asm volatile ("mov %%rax, %%cr3" : : "a"(kernel_pml4) : "memory");
}

static inline pt_t *vmm_getalloc(pt_t *pt, size_t offset, int flags) {
    uint64_t addr = pt->entries[offset];
    if (!(addr & VMM_PRESENT)) {
        addr = pt->entries[offset] = (uint64_t)pmm_alloc(1);
        if(!addr) {
            while(1);
        }
        pt->entries[offset] |= flags | VMM_PRESENT;
    }

    return (pt_t *)((addr & VMM_ADDR_MASK) + VIRT_PHYS_BASE);
}

static inline pt_t *vmm_getnull(pt_t *pt, size_t offset) {
    uint64_t addr = pt->entries[offset];
    if (!(addr & VMM_PRESENT)) {
        return NULL;
    }

    return (pt_t *)((addr & VMM_ADDR_MASK) + VIRT_PHYS_BASE);
}

void vmm_map_pages(pt_t *pml4param, void *addr, void *phys, size_t size, uint64_t permissions) {
    while(size--) {
        pt_offset_t offset = addr_to_offsets(addr);

        pt_t *pml4 = (pt_t *)((uint64_t)pml4param + VIRT_PHYS_BASE);
        pt_t *pdp = vmm_getalloc(pml4, offset.pml4_off, VMM_WRITE | VMM_USER);
        pt_t *pd = vmm_getalloc(pdp, offset.pdp_off, VMM_WRITE | VMM_USER);
        pt_t *pt = vmm_getalloc(pd, offset.pd_off, VMM_WRITE | VMM_USER);
        pt->entries[offset.pt_off] = (uint64_t)phys | permissions | VMM_PRESENT;

        addr = (void *)((uint64_t)addr + 0x1000);
        phys = (void *)((uint64_t)phys + 0x1000);
    }
}

void vmm_unmap_pages(pt_t *pml4param, void *addr, size_t size) {
    while(size--) {
        pt_offset_t offset = addr_to_offsets(addr);

        pt_t *pml4 = (pt_t *)((uint64_t)pml4param + VIRT_PHYS_BASE);
        if(!pml4) return;
        pt_t *pdp = vmm_getnull(pml4, offset.pml4_off);
        if(!pdp) return;
        pt_t *pd = vmm_getnull(pdp, offset.pdp_off);
        if(!pd) return;
        pt_t *pt = vmm_getnull(pd, offset.pd_off);
        if(!pt) return;
        pt->entries[offset.pt_off] = (uint64_t)0;

        addr = (void *)((uint64_t)addr + 0x1000);
    }
}

void *vmm_get_phys(pt_t *pml4param, void *addr) {
    pt_offset_t offset = addr_to_offsets(addr);

    pt_t *pml4 = (pt_t *)((uint64_t)pml4param + VIRT_PHYS_BASE);
    if(!pml4) return NULL;
    pt_t *pdp = vmm_getnull(pml4, offset.pml4_off);
    if(!pdp) return NULL;
    pt_t *pd = vmm_getnull(pdp, offset.pdp_off);
    if(!pd) return NULL;
    pt_t *pt = vmm_getnull(pd, offset.pd_off);
    if(!pt) return NULL;
    return (void *)(pt->entries[offset.pt_off] & ~0xFFF);
}

void vmm_map_pages_huge(pt_t *pml4param, void *addr, void *phys, size_t size, uint64_t permissions) {
    while(size--) {
        pt_offset_t offset = addr_to_offsets(addr);

        pt_t *pml4 = (pt_t *)((uint64_t)pml4param + VIRT_PHYS_BASE);
        pt_t *pdp = vmm_getalloc(pml4, offset.pml4_off, VMM_WRITE | VMM_USER);
        pt_t *pd = vmm_getalloc(pdp, offset.pdp_off, VMM_WRITE | VMM_USER);
        pd->entries[offset.pd_off] = (uint64_t)phys | permissions | VMM_PRESENT | VMM_LARGE;

        addr = (void *)((uint64_t)addr + 0x200000);
        phys = (void *)((uint64_t)phys + 0x200000);
    }
}

void vmm_unmap_pages_huge(pt_t *pml4param, void *addr, size_t size) {
    while(size--) {
        pt_offset_t offset = addr_to_offsets(addr);

        pt_t *pml4 = (pt_t *)((uint64_t)pml4param + VIRT_PHYS_BASE);
        if(!pml4) return;
        pt_t *pdp = vmm_getnull(pml4, offset.pml4_off);
        if(!pdp) return;
        pt_t *pd = vmm_getnull(pdp, offset.pdp_off);
        if(!pd) return;
        pd->entries[offset.pd_off] = (uint64_t)0;

        addr = (void *)((uint64_t)addr + 0x200000);
    }
}

pt_t *create_new_pml4() {
    pt_t *pml4 = (pt_t*)pmm_alloc(1);
    
    vmm_map_pages_huge(pml4, (void *)0xFFFF800000000000, NULL, 4096 * 4, 3);
    vmm_map_pages_huge(pml4, (void *)0xFFFFFFFF80000000, NULL, 64, 3);

    return pml4;
}

static inline size_t entries_to_virt_addr(size_t pml4_entry,
                                   size_t pdpt_entry,
                                   size_t pd_entry,
                                   size_t pt_entry) {
    size_t virt_addr = 0;
    virt_addr |= pml4_entry << 39;
    virt_addr |= pdpt_entry << 30;
    virt_addr |= pd_entry << 21;
    virt_addr |= pt_entry << 12;
    return virt_addr;
}

void *memcpy64(void *dest, const void *src, size_t n) {
    uint64_t *pdest = dest;
    const uint64_t *psrc = src;

    for (size_t i = 0; i < (n / sizeof(uint64_t)); i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

pt_t *fork_pml4(pt_t *pml4) {
    pt_t *newPml4 = create_new_pml4();

    pt_t *pdpt;
    pt_t *pd;
    pt_t *pt;

    struct {
        uint8_t data[PAGE_SIZE];
    } *pool;

    size_t poolSize = 4096;
    pool = pmm_alloc(poolSize);
    size_t poolPtr = 0;

    for(size_t i = 0; i < 256; i++) {
        if(((pt_t *)((size_t)pml4 + VIRT_PHYS_BASE))->entries[i] & VMM_PRESENT) {
            pdpt = (pt_t *)((((pt_t *)((size_t)pml4 + VIRT_PHYS_BASE))->entries[i] & 0xFFFFFFFFFFFFF000) + VIRT_PHYS_BASE);
            for(size_t j = 0; j < 512; j++) {
                if(pdpt->entries[j] & VMM_PRESENT) {
                    pd = (pt_t *)((pdpt->entries[j] & 0xFFFFFFFFFFFFF000) + VIRT_PHYS_BASE);
                    for(size_t k = 0; k < 512; k++) {
                        if(pd->entries[k] & VMM_PRESENT) {
                            pt = (pt_t *)((pd->entries[k] & 0xFFFFFFFFFFFFF000) + VIRT_PHYS_BASE);
                            for(size_t l = 0; l < 512; l++) {
                                if(pt->entries[l] & VMM_PRESENT) {
                                    if(poolPtr == poolSize) {
                                        pool = pmm_alloc(poolSize);
                                        poolPtr = 0;
                                    }
                                    size_t newPage = (size_t)&pool[poolPtr++];
                                    memcpy64((void *)(newPage + VIRT_PHYS_BASE), (void *)((pt->entries[l] & 0xFFFFFFFFFFFFF000) + VIRT_PHYS_BASE), PAGE_SIZE);
                                    vmm_map_pages(newPml4, (void *)entries_to_virt_addr(i, j, k, l), (void *)newPage, 1, (pt->entries[l] & 0xFFF));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    pmm_free(pool + poolPtr, poolSize - poolPtr);

    return newPml4;
}
