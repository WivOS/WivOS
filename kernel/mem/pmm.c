#include "pmm.h"
#include <utils/spinlock.h>
#include <cpu/inlines.h>

static uint64_t *PmmBitmap = NULL;
static uint64_t PmmLength = 0;
static uint64_t PmmFreePages = 0;

static volatile spinlock_t PmmLock = INIT_SPINLOCK();

void pmm_init(limine_memmap_entry_t **entries, size_t count) {
    printf("[PMM] Memmap:\n");

    uint64_t maxBase = 0;
    uint64_t maxRegionUsableSize = 0;
    for(int i = 0; i < count; i++) {
        limine_memmap_entry_t *entry = entries[i];
        printf("\t0x%016llX - 0x%016llX: ", entry->base, entry->base + entry->length);

        char *name = "Unknown";
        switch(entry->type) {
            case LIMINE_MEMMAP_USABLE: name = "Usable"; break;
            case LIMINE_MEMMAP_RESERVED: name = "Reserved"; break;
            case LIMINE_MEMMAP_ACPI_RECLAIMABLE: name = "ACPI Reclaimable"; break;
            case LIMINE_MEMMAP_ACPI_NVS: name = "ACPI NVS"; break;
            case LIMINE_MEMMAP_BAD_MEMORY: name = "Bad Memory"; break;
            case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE: name = "Bootloader Reclaimable"; break;
            case LIMINE_MEMMAP_KERNEL_AND_MODULES: name = "Kernel And Modules"; break;
            case LIMINE_MEMMAP_FRAMEBUFFER: name = "Framebuffer"; break;
        }
        printf("%s\n", name);

        if(entry->type == LIMINE_MEMMAP_USABLE && entry->base > (uint64_t)PmmBitmap) {
            uint64_t newLength = (entry->base + entry->length + PAGE_SIZE - 1) / PAGE_SIZE;
            if(entry->length > newLength) {
                PmmBitmap = (uint64_t *)(entry->base);
                PmmLength = newLength;
                maxBase = entry->base;
            }
        }
    }

    if(PmmBitmap == NULL) while(1);
    PmmBitmap = (void *)((uint64_t)PmmBitmap + MEM_PHYS_OFFSET);

    memset(PmmBitmap, 0xFF, PmmLength / 8); //Reserve the bitmap as used
    for(int i = 0; i < count; i++) {
        limine_memmap_entry_t *entry = entries[i];
        if(entry->type == LIMINE_MEMMAP_USABLE && entry->base <= maxBase) {
            if(entry->base == maxBase) //Dont free the bitmap itself
                pmm_free((void *)(entry->base + (((PmmLength / 8) + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE), (entry->length / PAGE_SIZE) - ((PmmLength / 8) / PAGE_SIZE));
            else
                pmm_free((void *)entry->base, entry->length / PAGE_SIZE);
        }
    }

    printf("[PMM] Inited, 0x%llX free pages, bitmap address 0x%016llX\n", PmmFreePages, (uint64_t)PmmBitmap);

    set_bit(PmmBitmap, 0x1000 / PAGE_SIZE);
    set_bit(PmmBitmap, 0x2000 / PAGE_SIZE); //Reserve the first two pages to the smp trampoline,
                                            //the low 0x0000 - 0x1000 is reserved but used as variables on trampoline
                                            //when porting to new arch this should be considered
}

void *pmm_alloc(size_t count) {
    void *addr = pmm_alloc_advanced(count, 1);
    memset((void *)(addr + MEM_PHYS_OFFSET), 0, PAGE_SIZE * count);
    return addr;
}

static bool is_bitmap_page_free(size_t index, size_t count) {
    for(; count; index++, count--) {
        if(test_bit(PmmBitmap, index))
            return false;
    }

    return true;
}

void *pmm_alloc_advanced(size_t count, size_t alignment) {
    size_t index = 0;
    spinlock_lock(&PmmLock);
    while(index < PmmLength) {
        if(!is_bitmap_page_free(index, count)) {
            index += alignment;
            continue;
        }
        if(PmmFreePages)
            PmmFreePages -= count;
        for(size_t i = index; i < (count + index); i++)
            set_bit(PmmBitmap, i);

        spinlock_unlock(&PmmLock);
        return (void *)(index * PAGE_SIZE);
    }
    spinlock_unlock(&PmmLock);

    return NULL;
}

void pmm_free(void *addr, size_t count) {
    if((uint64_t)addr >= MEM_PHYS_OFFSET) while(1);
    uint64_t index = (uint64_t)addr / PAGE_SIZE;
    spinlock_lock(&PmmLock);
    PmmFreePages += count;
    for(; count; index++, count--) {
        reset_bit(PmmBitmap, index);
    }
    spinlock_unlock(&PmmLock);
}