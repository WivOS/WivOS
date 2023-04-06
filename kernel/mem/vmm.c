#include "vmm.h"
#include <mem/pmm.h>

typedef struct {
    size_t pml4_off;
    size_t pdp_off;
	size_t pd_off;
	size_t pt_off;
} pt_offset_t;

pt_t *KernelPml4 = NULL;
static void *KernelBase;
static void *PhysicalKernelBase;
static size_t KernelSize;

void vmm_init(void *kernel_base, void *phys_kernel_base, limine_memmap_entry_t **entries, size_t count) {
    KernelBase = kernel_base;
    PhysicalKernelBase = phys_kernel_base;

    for(int i = 0; i < count; i++) {
        if(entries[i]->type == LIMINE_MEMMAP_KERNEL_AND_MODULES && entries[i]->base == (uint64_t)PhysicalKernelBase) {
            if((uint64_t)PhysicalKernelBase % 0x200000) {
                void *newPhysBase = pmm_alloc_advanced(ROUND_UP(entries[i]->length, PAGE_SIZE) / PAGE_SIZE, 0x200);
                PhysicalKernelBase = newPhysBase;
            }
            KernelSize = entries[i]->length;
            break;
        }
    }

    KernelPml4 = vmm_setup_pml4();
    vmm_map_huge(KernelPml4, NULL, NULL, 0x10, 3);
    memcpy((void*)((uint64_t)PhysicalKernelBase + MEM_PHYS_OFFSET), KernelBase, KernelSize);
    asm volatile ("mov %%rax, %%cr3" : : "a"(KernelPml4->entries) : "memory"); //TODO: Move this

    if(PhysicalKernelBase != phys_kernel_base) pmm_free(phys_kernel_base, ROUND_UP(KernelSize, PAGE_SIZE) / PAGE_SIZE);

    printf("[VMM] Inited\n");
}

pt_t *vmm_setup_pml4() {
    pt_t *pml4 = kmalloc(sizeof(pt_t *));
    pml4->entries = (pt_entries_t*)pmm_alloc(1);
    pml4->lock = INIT_SPINLOCK();

    vmm_map(pml4, KernelBase, PhysicalKernelBase, ROUND_UP(KernelSize, 0x1000) / 0x1000, 3);
    vmm_map_huge(pml4, (void*)MEM_PHYS_OFFSET, NULL, 4096 * 3, 3);

    return pml4;
}

static pt_offset_t get_addr_offsets(void *addr) {
    uint64_t addrValue = (uint64_t)addr;

    pt_offset_t offset = {
        .pml4_off   = (addrValue & ((size_t)0x1FF << 39)) >> 39,
        .pdp_off    = (addrValue & ((size_t)0x1FF << 30)) >> 30,
        .pd_off     = (addrValue & ((size_t)0x1FF << 21)) >> 21,
        .pt_off     = (addrValue & ((size_t)0x1FF << 12)) >> 12
    };

    return offset;
}

static pt_entries_t *vmm_get_or_alloc(pt_entries_t *pt, size_t offset, uint32_t flags) {
    uint64_t addr = pt->entries[offset];
    if(!(addr & VMM_PRESENT)) {
        addr = pt->entries[offset] = (uint64_t)pmm_alloc(1);
        if(!addr) while(1);
        pt->entries[offset] |= flags | VMM_PRESENT;
    }

    return (pt_entries_t *)((addr & VMM_ADDR_MASK) + MEM_PHYS_OFFSET);
}

static pt_entries_t *vmm_get_or_null(pt_entries_t *pt, size_t offset) {
    uint64_t addr = pt->entries[offset];
    if(!(addr & VMM_PRESENT)) {
        return NULL;
    }

    return (pt_entries_t *)((addr & VMM_ADDR_MASK) + MEM_PHYS_OFFSET);
}

static bool vmm_map_advanced(pt_t *pml4addr, void *addr, void *phys, size_t size, uint64_t permissions, bool huge) {
    spinlock_lock(&pml4addr->lock);
    while(size--) {
        pt_offset_t offset = get_addr_offsets(addr);

        pt_entries_t *pml4 = (pt_entries_t *)((uint64_t)pml4addr->entries + MEM_PHYS_OFFSET);
        if(!pml4) {
            spinlock_unlock(&pml4addr->lock);
            return false;
        }
        pt_entries_t *pdp = vmm_get_or_alloc(pml4, offset.pml4_off, VMM_WRITE | VMM_USER);
        if(!pdp) {
            spinlock_unlock(&pml4addr->lock);
            return false;
        }
        pt_entries_t *pd  = vmm_get_or_alloc(pdp, offset.pdp_off, VMM_WRITE | VMM_USER);
        if(!pd) {
            spinlock_unlock(&pml4addr->lock);
            return false;
        }
        if(huge) {
            pd->entries[offset.pd_off] = (uint64_t)phys | permissions | VMM_PRESENT | VMM_LARGE;
        } else {
            pt_entries_t *pt  = vmm_get_or_alloc(pd, offset.pd_off, VMM_WRITE | VMM_USER);
            if(!pt) {
                spinlock_unlock(&pml4addr->lock);
                return false;
            }
            pt->entries[offset.pt_off] = (uint64_t)phys | permissions | VMM_PRESENT;
        }

        addr = (void *)((uint64_t)addr + (huge ? 0x200000 : 0x1000));
        phys = (void *)((uint64_t)phys + (huge ? 0x200000 : 0x1000));
    }
    spinlock_unlock(&pml4addr->lock);

    return true;
}

bool vmm_map(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions) {
    return vmm_map_advanced(pml4, addr, phys, size, permissions, false);
}
bool vmm_map_huge(pt_t *pml4, void *addr, void *phys, size_t size, uint64_t permissions) {
    return vmm_map_advanced(pml4, addr, phys, size, permissions, true);
}

bool vmm_unmap(pt_t *pml4addr, void *addr, size_t size) {
    spinlock_lock(&pml4addr->lock);
    while(size-- > 0) {
        pt_offset_t offset = get_addr_offsets(addr);

        pt_entries_t *pml4 = (pt_entries_t *)((uint64_t)pml4addr->entries + MEM_PHYS_OFFSET);
        if(!pml4) {
            spinlock_unlock(&pml4addr->lock);
            return false;
        }
        pt_entries_t *pdp = vmm_get_or_null(pml4, offset.pml4_off);
        if(!pdp) {
            spinlock_unlock(&pml4addr->lock);
            return false;
        }
        pt_entries_t *pd  = vmm_get_or_null(pdp, offset.pdp_off);
        if(!pd) {
            spinlock_unlock(&pml4addr->lock);
            return false;
        }

        bool huge = (pd->entries[offset.pd_off] & VMM_LARGE);
        if(huge) {
            if(size < 0x1FF) {
                spinlock_unlock(&pml4addr->lock);
                return false; //We cannot unmap a singl page from a large page
            }

            pd->entries[offset.pd_off] = (uint64_t)0;
            size -= 0x1FF; // size is on normal pages
        } else {
            pt_entries_t *pt  = vmm_get_or_null(pd, offset.pd_off);
            pt->entries[offset.pt_off] = (uint64_t)0;
        }

        addr = (void *)((uint64_t)addr + (huge ? 0x200000 : 0x1000));
    }

    spinlock_unlock(&pml4addr->lock);

    return true;
}

void vmm_read(pt_t *pml4addr, void *addr, uint8_t *buffer, size_t size) {
    spinlock_lock(&pml4addr->lock);
    while(true) {
        pt_offset_t offset = get_addr_offsets(addr);

        pt_entries_t *pml4 = (pt_entries_t *)((uint64_t)pml4addr->entries + MEM_PHYS_OFFSET);
        if(!pml4) {
            spinlock_unlock(&pml4addr->lock);
            return;
        }
        pt_entries_t *pdp = vmm_get_or_null(pml4, offset.pml4_off);
        if(!pdp) {
            spinlock_unlock(&pml4addr->lock);
            return;
        }
        pt_entries_t *pd  = vmm_get_or_null(pdp, offset.pdp_off);
        if(!pd) {
            spinlock_unlock(&pml4addr->lock);
            return;
        }
        uint64_t misalign = 0;
        bool huge = pd->entries[offset.pd_off] & VMM_LARGE;
        if(huge) {
            misalign = (uint64_t)addr & 0x1FFFFF;
            uint64_t phys_addr = pd->entries[offset.pd_off] & ~0xFFF;
            memcpy(buffer, (void *)(phys_addr + misalign + MEM_PHYS_OFFSET), size > (0x200000 - misalign) ? (0x200000 - misalign) : size);
            if(size <= (0x200000 - misalign)) {
                spinlock_unlock(&pml4addr->lock);
                return;
            }
            size -= 0x200000;
        } else {
            misalign = (uint64_t)addr & 0xFFF;
            pt_entries_t *pt = vmm_get_or_null(pd, offset.pd_off);
            if(!pt) {
                spinlock_unlock(&pml4addr->lock);
                return;
            }
            uint64_t phys_addr = pt->entries[offset.pt_off] & ~(uint64_t)0xFFF;
            memcpy(buffer, (void *)(phys_addr + misalign + MEM_PHYS_OFFSET), (size > (PAGE_SIZE - misalign)) ? (PAGE_SIZE - misalign) : size);
            if(size <= (PAGE_SIZE - misalign)) {
                spinlock_unlock(&pml4addr->lock);
                return;
            }
            size -= PAGE_SIZE;
        }

        addr = (void *)(((uint64_t)addr & (huge ? (~(uint64_t)0x1FFFFF) : (~(uint64_t)0xFFF))) + (huge ? 0x200000 : 0x1000));
        buffer = (void *)((uint64_t)buffer + (huge ? 0x200000 : 0x1000));
    }
    spinlock_unlock(&pml4addr->lock);
}

void *vmm_get_phys(pt_t *pml4addr, void *addr) {
    spinlock_lock(&pml4addr->lock);
    pt_offset_t offset = get_addr_offsets(addr);

    pt_entries_t *pml4 = (pt_entries_t *)((uint64_t)pml4addr->entries + MEM_PHYS_OFFSET);
    if(!pml4) {
        spinlock_unlock(&pml4addr->lock);
        return NULL;
    }
    pt_entries_t *pdp = vmm_get_or_null(pml4, offset.pml4_off);
    if(!pdp) {
        spinlock_unlock(&pml4addr->lock);
        return NULL;
    }
    pt_entries_t *pd  = vmm_get_or_null(pdp, offset.pdp_off);
    if(!pd) {
        spinlock_unlock(&pml4addr->lock);
        return NULL;
    }
    uint64_t misalign = 0;
    bool huge = pd->entries[offset.pd_off] & VMM_LARGE;
    if(huge) {
        misalign = (uint64_t)addr & 0x1FFFFF;
        spinlock_unlock(&pml4addr->lock);
        return (void *)((pd->entries[offset.pd_off] & ~0xFFF) | misalign);
    }

    misalign = (uint64_t)addr & 0xFFF;
    pt_entries_t *pt = vmm_get_or_null(pd, offset.pd_off);
    if(!pt) {
        spinlock_unlock(&pml4addr->lock);
        return NULL;
    }
    spinlock_unlock(&pml4addr->lock);
    return (void *)((pt->entries[offset.pt_off] & ~(uint64_t)0xFFF) | misalign);
}

void vmm_map_if_not_mapped(pt_t *pml4addr, void *addr, void *phys, size_t size, uint64_t permissions) {
    spinlock_lock(&pml4addr->lock);
    while(size--) {
        pt_offset_t offset = get_addr_offsets(addr);

        pt_entries_t *pml4 = (pt_entries_t *)((uint64_t)pml4addr->entries + MEM_PHYS_OFFSET);
        pt_entries_t *pdp = vmm_get_or_alloc(pml4, offset.pml4_off, VMM_WRITE | VMM_USER);
        pt_entries_t *pd  = vmm_get_or_alloc(pdp, offset.pdp_off, VMM_WRITE | VMM_USER);
        if(pd->entries[offset.pd_off] & VMM_LARGE) {
            spinlock_unlock(&pml4addr->lock);
            return;
        } else {
            pt_entries_t *pt  = vmm_get_or_alloc(pd, offset.pd_off, VMM_WRITE | VMM_USER);
            if(pt->entries[offset.pt_off] & VMM_PRESENT) return;
            pt->entries[offset.pt_off] = (uint64_t)phys | permissions | VMM_PRESENT;
        }

        addr = (void *)((uint64_t)addr + 0x1000);
        phys = (void *)((uint64_t)phys + 0x1000);
    }
    spinlock_unlock(&pml4addr->lock);
}

void vmm_free_pml4(pt_t *pml4) {
    spinlock_lock(&pml4->lock);

    for(size_t i = 0; i < 256; i++) {
        pt_entries_t *pdp = vmm_get_or_null((pt_entries_t *)((size_t)pml4->entries + MEM_PHYS_OFFSET), i);
        if(pdp) {
            for(size_t j = 0; j < 512; j++) {
                pt_entries_t *pd = vmm_get_or_null(pdp, j);
                if(pd) {
                    for(size_t k = 0; k < 512; k++) {
                        bool huge = (pd->entries[k] & VMM_LARGE);
                        if(!huge) {
                            pt_entries_t *pt = vmm_get_or_null(pd, k);
                            if(pt) {
                                for(size_t l = 0; l < 512; l++) {
                                    if(pt->entries[l] & 0x1) {
                                        pmm_free((void *)((pt->entries[l] & VMM_ADDR_MASK)), 1);
                                    }
                                }
                                pmm_free((void *)((uint64_t)pt - MEM_PHYS_OFFSET), 1);
                            }
                        }
                    }
                    pmm_free((void *)((uint64_t)pd - MEM_PHYS_OFFSET), 1);
                }
            }
            pmm_free((void *)((uint64_t)pdp - MEM_PHYS_OFFSET), 1);
        }
    }

    pmm_free((void *)((uint64_t)pml4->entries), 1);
    kfree(pml4);
}

static inline size_t vmm_entries_to_virt_addr(size_t pml4_entry,size_t pdpt_entry,
                                          size_t pd_entry,size_t pt_entry) {
    size_t virt_addr = (pml4_entry & 0x1FF) << 39;
    virt_addr |= (pdpt_entry & 0x1FF) << 30;
    virt_addr |= (pd_entry & 0x1FF) << 21;
    virt_addr |= (pt_entry & 0x1FF) << 12;

    return virt_addr;
}

pt_t *vmm_copy_pml4(pt_t *pml4) {
    struct {
        uint8_t data[PAGE_SIZE];
    } __attribute__((packed)) *pool;

    size_t pool_size = 4096;
    pool = pmm_alloc(pool_size);
    size_t pool_ptr = 0;

    spinlock_lock(&pml4->lock);

    pt_t *new_pml4 = kmalloc(sizeof(pt_t *));
    new_pml4->entries = (pt_entries_t*)pmm_alloc(1);
    new_pml4->lock = INIT_SPINLOCK();

    for(size_t i = 0; i < 256; i++) { // 0
        pt_entries_t *pdp = vmm_get_or_null((pt_entries_t *)((size_t)pml4->entries + MEM_PHYS_OFFSET), i);
        if(pdp) {
            for(size_t j = 0; j < 512; j++) { // 1
                pt_entries_t *pd = vmm_get_or_null(pdp, j);
                if(pd) {
                    for(size_t k = 0; k < 512; k++) { // 8
                        bool huge = (pd->entries[k] & VMM_LARGE);
                        if(!huge) {
                            pt_entries_t *pt = vmm_get_or_null(pd, k);
                            if(pt) {
                                for(size_t l = 0; l < 512; l++) { // 23
                                    if(pt->entries[l] & 0x1) {
                                        if(pool_ptr == pool_size) {
                                            pool_ptr = 0;
                                            pool = pmm_alloc(pool_size);
                                        }
                                        size_t new_page = (size_t)&pool[pool_ptr++];
                                        memcpy64((void *)(new_page + MEM_PHYS_OFFSET), (char *)((pt->entries[l] & VMM_ADDR_MASK) + MEM_PHYS_OFFSET), PAGE_SIZE);
                                        vmm_map(new_pml4, (void *)vmm_entries_to_virt_addr(i, j, k, l), (void *)new_page, 1, pt->entries[l] & VMM_FLAG_MASK);
                                    }
                                }
                            }
                        } else {
                            size_t new_page = (size_t)pmm_alloc(0x200);
                            memcpy64((void *)(new_page + MEM_PHYS_OFFSET), (char *)((pd->entries[k] & VMM_ADDR_MASK) + MEM_PHYS_OFFSET), PAGE_SIZE * 0x200);
                            vmm_map_huge(new_pml4, (void *)vmm_entries_to_virt_addr(i, j, k, 0), (void *)new_page, 1, pd->entries[k] & VMM_FLAG_MASK);
                        }
                    }
                }
            }
        }
    }

    pmm_free((void *)(pool + pool_ptr), pool_size - pool_ptr);

    for(size_t i = 256; i < 512; i++) {
        ((pt_entries_t *)((size_t)new_pml4->entries + MEM_PHYS_OFFSET))->entries[i] = ((pt_entries_t *)((size_t)pml4->entries + MEM_PHYS_OFFSET))->entries[i];
    }

    spinlock_unlock(&pml4->lock);

    return new_pml4;
}