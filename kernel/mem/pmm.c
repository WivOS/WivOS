#include <mem/pmm.h>
#include <util/util.h>
#include <util/string.h>

uint64_t *pmmBitmap = NULL;
uint64_t pmmLength = 0;
uint64_t pmmFreePages = 0;
uint64_t pmmTotalPages = 0;

static int pmm_read_bit(size_t index) {
    return (pmmBitmap[(index / 64UL)] & (1UL << (index % 64UL))) == (1UL << (index % 64UL));
}

static void pmm_write_bit(size_t index, int bit, size_t count) {
    for (; count; count--, index++) {

        if(bit)
            pmmBitmap[(index / 64UL)] |= (1UL << (index % 64UL));
        else
            pmmBitmap[(index / 64UL)] &= ~(1UL << (index % 64UL));
    }
}

static int pmm_bitmap_isfree(size_t index, size_t count) {
    for (; count; index++, count--) {
        if(pmm_read_bit(index))
            return 0;
    }

    return 1;
}

void pmm_init(stivale2_struct_t *stivale) {
    printf("[PMM]Initialisation in progress\n");

    uint64_t memEntry = 0;
    stivale2_struct_tag_memmap_t *memmaptag = NULL;

    uint64_t stivaleTags = stivale->tags;
    while(stivaleTags != (size_t)NULL) { // TODO: Move this to a custom function taht retrieves tags from stivale2 struct
        stivale2_hdr_tag_t *currTag = (stivale2_hdr_tag_t *)stivaleTags;
        stivaleTags = currTag->next;
        switch(currTag->identifier) {
            case TAG_MEMMAP:
                {
                    memmaptag = (stivale2_struct_tag_memmap_t *)currTag;
                    stivaleTags = (size_t)NULL;
                }
                break;
            default:
                break;
        }
    }
    for(int i = 0; i < memmaptag->entries; i++) {
        stivale2_mmap_entry_t* entry = (stivale2_mmap_entry_t *)((size_t)&memmaptag->memmap[i]);
        switch(entry->type) {
            case USABLE: 
                {
                    if((entry->base + entry->length) > (memmaptag->memmap[memEntry].base + memmaptag->memmap[memEntry].length)) {
                        memEntry = i;
                    }
                }
                break;
            default:
                break;
        }
    }

    uint64_t memBase = memmaptag->memmap[memEntry].base;
    uint64_t memLength = memmaptag->memmap[memEntry].length;

    pmmBitmap = (uint64_t *)(0x2000000ULL + VIRT_PHYS_BASE);
    pmmLength = (memBase + memLength + PAGE_SIZE - 1) / PAGE_SIZE;

    memset(pmmBitmap, 0xFF, pmmLength / 8);

    for(int i = 0; i < memmaptag->entries; i++) {
        stivale2_mmap_entry_t* entry = (stivale2_mmap_entry_t *)((size_t)&memmaptag->memmap[i]);
        switch(entry->type) {
            case USABLE: 
                {
                    uint64_t start = entry->base;
                    uint64_t length = entry->length;
                    if(start + length < 0x2000000ULL) continue;

                    if(start < 0x2000000ULL) {
                        length -= 0x2000000ULL - start;
                        start = 0x2000000ULL;
                    }

                    if(0x2000000ULL >= start / 8 && (0x2000000ULL + pmmLength) <= (start + length)) {
                        if(start < 0x2000000ULL) {
                            pmm_free((void *)start, (start - 0x2000000ULL) / PAGE_SIZE);
                        }

                        start = 0x2000000ULL + (length / 8);
                        length -= pmmLength / 8;
                    }

                    pmm_free((void *)start, length / PAGE_SIZE);
                }
                break;
            default:
                break;
        }
    }
    
    pmmTotalPages = pmmFreePages;

    pmm_write_bit(0x2000000ULL / PAGE_SIZE, 1, (pmmLength / 8 + PAGE_SIZE - 1) / PAGE_SIZE);

    pmm_alloc_nonzero((pmmLength / 8 + PAGE_SIZE - 1) / PAGE_SIZE);

    printf("[PMM]Initialisation done with %lu free pages\n", pmmFreePages);
}

void pmm_free(void *addr, size_t count) {
    size_t index = (size_t)addr / PAGE_SIZE;
    pmm_write_bit(index, 0, count);
    pmmFreePages += count;
}

void *pmm_alloc(size_t count) {
    void *addrToZero = pmm_alloc_nonzero(count);
    memset((void *)((size_t)addrToZero + VIRT_PHYS_BASE), 0x00, PAGE_SIZE * count);
    return addrToZero;
}

void *pmm_alloc_nonzero(size_t count) {
    return pmm_alloc_advanced(count, 1, 0);
}

void *pmm_alloc_advanced(size_t count, size_t alignment, uint64_t upper) {
    size_t index = 0x2000000ULL / PAGE_SIZE;
    size_t max_idx = 0;

    if(!upper) {
        max_idx = pmmLength;
    } else {
        max_idx = pmmLength < (upper / PAGE_SIZE) ? pmmLength : (upper / PAGE_SIZE);
    }

    while(index < max_idx) {
        if(!pmm_bitmap_isfree(index, count)) {
            index += alignment;
            continue;
        }
        pmm_write_bit(index, 1, count);
        if(pmmTotalPages)
            pmmFreePages -= count;

        memset((void *)(index * PAGE_SIZE + VIRT_PHYS_BASE), 0, PAGE_SIZE);

        return (void *)(index * PAGE_SIZE);
    }

    return NULL;
}
