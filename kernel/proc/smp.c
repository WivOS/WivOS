#include "smp.h"
#include <acpi/acpi.h>
#include <acpi/apic.h>

cpu_t cpuLocals[128];

struct stack_t {
    uint8_t guardPage[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
    uint8_t stack[4096] __attribute__((aligned(PAGE_SIZE)));
};

static struct stack_t cpuStacks[128] __attribute__((aligned(PAGE_SIZE)));

static inline void setup_cpu_local(int cpuNumber, uint8_t lapicId) {
    vmm_unmap_pages(kernel_pml4, (void *)((size_t)&cpuStacks[cpuNumber].guardPage[0]), 1);

    cpuLocals[cpuNumber].currentCpu = cpuNumber;
    cpuLocals[cpuNumber].currentProcess = -1;
    cpuLocals[cpuNumber].currentThread = -1;
    cpuLocals[cpuNumber].lapicId = lapicId;
}

static int init_cpu_ap(madt_processor_t *processor) {
    

    return 0;
}

static void init_first_cpu() {
    setup_cpu_local(0, 0);

    cpu_t *cpuLocal = &cpuLocals[0];

}

madt_t *madt;
madt_processor_t **processors;
size_t processors_count = 0;

void smp_init() {
    init_first_cpu();

    madt = (madt_t *)acpi_find_sdt("APIC", 0);

    processors = kcalloc(sizeof(madt_processor_t *), 256);
    for (uint8_t *madt_ptr = (uint8_t *)(&madt->madt_entries); (size_t)madt_ptr < (size_t)madt + madt->sdt.length; madt_ptr += *(madt_ptr + 1)) {
        switch(*(madt_ptr)) {
            case 0:
                printf("[SMP/MADT] local APIC #%u\n", processors_count);
                processors[processors_count++] = (madt_processor_t *)madt_ptr;
                //TODO: Start CPU
                
                
                
                break;
            default:
                break;
        }
    }

    lapic_init();

    for(size_t i = 0 ; i < processors_count; i++) {
        if(init_cpu_ap(processors[i])) {

        }
    }
}