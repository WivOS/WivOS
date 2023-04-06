#include "cpu.h"
#include "acpi.h"

#include "inlines.h"

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <devices/timer.h>

#include <cpu/gdt.h>

typedef struct {
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
} __attribute__((packed)) tss_t;

cpu_t CPULocals[MAX_CPUS];
static tss_t CPUTss[MAX_CPUS] __attribute__((aligned(16)));

#define CPU_STACK_SIZE 16384
struct stack_t {
    uint8_t guardPage[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
    uint8_t stack[CPU_STACK_SIZE] __attribute__((aligned(PAGE_SIZE)));
};

static struct stack_t CPUStacks[MAX_CPUS] __attribute__((aligned(PAGE_SIZE)));

extern void *smp_init_trampoline(void *, void *, void *, void *, void *);

extern void syscall_entry();
void smp_init_cpu_misc() {
    uint64_t cr0 = read_cr("0");
    cr0 &= ~(1 << 2);
    cr0 |= (1 << 1);
    write_cr("0", cr0);

    uint64_t cr4 = read_cr("4");
    cr4 |= (3 << 9);
    write_cr("4", cr4);

    uint64_t pat_msr = rdmsr(0x277);
    pat_msr &= 0xffffffff;
    pat_msr |= (uint64_t)0x0105 << 32;
    wrmsr(0x277, pat_msr);

    uint64_t efer = rdmsr(0xc0000080);
    efer |= 1;
    wrmsr(0xc0000080, efer);

    wrmsr(0xC0000081, 0x0013000800000000);
    wrmsr(0xC0000082, (uint64_t)syscall_entry);
    wrmsr(0xC0000084, (uint64_t)(~(uint32_t)0x002));
}

static void smp_entry_ap() {
    printf("[SMP] Started AP #%u\n", CurrentCPU);

    smp_init_cpu_misc();
    lapic_write(0xf0, lapic_read(0xf0) | 0x1ff);

    asm volatile("sti");

    for (;;) __asm__ __volatile__("hlt");
}

static bool smp_init_cpu(int cpu) {
    void *trampoline = smp_init_trampoline(smp_entry_ap, KernelPml4->entries, &CPUStacks[cpu].stack[CPU_STACK_SIZE], &CPULocals[cpu], &CPUTss[cpu]);

    lapic_write(0x280, 0); // Erase all errors

    lapic_write(0x310, CPULocals[cpu].lapicID << 24);
    lapic_write(0x300, 0x500); //INIT destination

    ksleep(10);

    lapic_write(0x310, CPULocals[cpu].lapicID << 24);
    lapic_write(0x300, 0x600 | (uint32_t)(size_t)trampoline); //INIT destination

    for(int i = 0; i < 1000; i++) {
        ksleep(1);
        if(*((volatile uint8_t *)0x510)) {
            return true;
        }
    }

    return false;
}

void (*cpu_save_simd)(void *);
void (*cpu_restore_simd)(void *);
uint64_t CpuSimdRegionSize;
static inline void xsave(void *region) {
    asm volatile ("xsave (%0)"
                  :
                  : "r" (region), "a" (0xFFFFFFFF), "d" (0xFFFFFFFF)
                  : "memory");
}

static inline void xrstor(void *region) {
    asm volatile ("xrstor (%0)"
                  :
                  : "r" (region), "a" (0xFFFFFFFF), "d" (0xFFFFFFFF)
                  : "memory");
}

static inline void fxsave(void *region) {
    asm volatile ("fxsave (%0)"
                  :
                  : "r" (region)
                  : "memory");
}

static inline void fxrstor(void *region) {
    asm volatile ("fxrstor (%0)"
                  :
                  : "r" (region)
                  : "memory");
}

void smp_init() {
    for(int i = 0; i < MAX_CPUS; i++) {
        CPULocals[i].kernelStack = (volatile size_t)&CPUStacks[i].stack[CPU_STACK_SIZE];
        if(CPULocals[i].currentCpu == -1) continue;

        vmm_unmap(KernelPml4, (void *)((size_t)&CPUStacks[i].guardPage[0]), 1);
        //Here fill CPULocals

        CPUTss[i].rsp0 = (uint64_t)&CPUStacks[i].stack[CPU_STACK_SIZE];
        CPUTss[i].ist1 = (uint64_t)&CPUStacks[i].stack[CPU_STACK_SIZE];

        if(i == 0) {
            gdt_setup_tss((uint64_t)&CPUTss[0]);

            __asm__ __volatile__(
                "movw $0x1B, %ax;"
                "movw %ax, %fs;"
                "movw %ax, %gs;"

                "movw $0x28, %ax;"
                "ltr %ax;"
            );
            wrmsr(0xC0000101, (size_t)&CPULocals[0]);
            smp_init_cpu_misc();

            uint32_t a = 0, b = 0, c = 0, d = 0;
            cpuid(1, 0, &a, &b, &c, &d);

            if((c & (1 << 26))) {
                uint64_t cr4 = read_cr("4");
                cr4 |= (1 << 18);
                write_cr("4", cr4);

                uint64_t xcr0 = 0x2;
                if((c & (1 << 28))) xcr0 |= (1 << 2);

                if(cpuid(7, 0, &a, &b, &c, &d)) {
                    if((b & (1 << 16))) xcr0 |= (1 << 5) | (1 << 6) | (1 << 7);
                }
                wrxcr(0, xcr0);

                if(cpuid(0xD, 0, &a, &b, &c, &d)) {
                    CpuSimdRegionSize = c;
                }

                cpu_save_simd = xsave;
                cpu_restore_simd = xrstor;
            } else {
                CpuSimdRegionSize = 512;
                cpu_save_simd = fxsave;
                cpu_restore_simd = fxrstor;
            }

            continue;
        }

        printf("[SMP] Starting AP #%d\n", i);

        if(!smp_init_cpu(i)) {
            printf("[SMP] Failed to start AP #%d\n", i);
        }

        ksleep(10); //Let the cpu load gdt and other things
    }
}