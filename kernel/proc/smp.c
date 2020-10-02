#include "smp.h"
#include <acpi/acpi.h>
#include <acpi/apic.h>
#include <util/util.h>

cpu_t cpuLocals[128];

struct tss_t {
    uint32_t unused0 __attribute__((aligned(16)));
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t unused1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t unused2;
    uint32_t iopb_offset;
} __attribute__((packed));

struct stack_t {
    uint8_t guardPage[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
    uint8_t stack[4096] __attribute__((aligned(PAGE_SIZE)));
};

static struct stack_t cpuStacks[128] __attribute__((aligned(PAGE_SIZE)));
static struct tss_t cpuTss[128] __attribute__((aligned(16)));

static void kernel_init_ap() {
    printf("[SMP] Started AP #%u\n", current_cpu);

    lapic_enable();

    asm volatile("sti");

    for (;;) asm volatile ("hlt");
}

static inline void setup_cpu_local(int cpuNumber, uint8_t lapicId) {
    vmm_unmap_pages(kernel_pml4, (void *)((size_t)&cpuStacks[cpuNumber].guardPage[0]), 1);

    cpuLocals[cpuNumber].currentCpu = cpuNumber;
    cpuLocals[cpuNumber].currentProcess = -1;
    cpuLocals[cpuNumber].currentThread = -1;
    cpuLocals[cpuNumber].currentActiveThread = -1;
    cpuLocals[cpuNumber].kernelStack = (size_t)&cpuStacks[cpuNumber].stack[4096];
    cpuLocals[cpuNumber].lapicId = lapicId;

    cpuTss[cpuNumber].rsp0 = (uint64_t)&cpuStacks[cpuNumber].stack[4096];
    cpuTss[cpuNumber].ist1 = (uint64_t)&cpuStacks[cpuNumber].stack[4096];
}

void smp_init_cpu0_local(void *, void *);
void *smp_init_trampoline(void *, void *, void *, void *, void *);
uint8_t smp_check_ap_flag();

static int init_cpu_ap(madt_processor_t *processor, int cpuNumber) {
    setup_cpu_local(cpuNumber, processor->apicID);

    void *trampoline = smp_init_trampoline(kernel_init_ap, kernel_pml4, &cpuStacks[cpuNumber].stack[4096], &cpuLocals[cpuNumber], &cpuTss[cpuNumber]);

    lapic_write(0x310, ((uint32_t)processor->apicID) << 24);
    lapic_write(0x300, 0x500);
    ksleep(10);

    lapic_write(0x310, ((uint32_t)processor->apicID) << 24);
    lapic_write(0x300, 0x600 | (uint32_t)(size_t)trampoline);

    for(int i = 0; i < 1000; i++) {
        ksleep(1);
        if(smp_check_ap_flag()) return 0;
    }

    return -1;
}

static void init_first_cpu() {
    setup_cpu_local(0, 0);

    cpu_t *cpuLocal = &cpuLocals[0];
    struct tss_t *tss = &cpuTss[0];

    smp_init_cpu0_local(cpuLocal, tss);
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
                break;
            default:
                break;
        }
    }

    lapic_init();

    asm volatile("sti");

    //TODO: Check processor flags
    for(size_t i = 1 ; i < processors_count; i++) {
        printf("[SMP] Starting AP #%u\n", i);

        if(init_cpu_ap(processors[i], i)) {
            printf("[SMP] Failed to start AP #%u\n", i);
        }

        ksleep(10);
    }

    asm volatile("cli");
}