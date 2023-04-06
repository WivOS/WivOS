#pragma once

#include "acpi.h"
#include <utils/common.h>
#include <stdbool.h>

#include <tasking/thread.h>

typedef struct {
    volatile size_t currentCpu;
    volatile size_t kernelStack;
    volatile size_t threadKernelStack;
    volatile size_t threadUserStack;
    volatile size_t lapicID;
    volatile kpid_t currentPid;
    volatile ktid_t currentTid;
    volatile ktid_t currentTaskID;
} __attribute__((packed)) cpu_t;

extern cpu_t CPULocals[MAX_CPUS];

#define CurrentCPU ({ \
    size_t cpuNumber; \
    asm volatile("movq %%gs:(0), %0" : "=r"(cpuNumber) :: "memory", "cc"); \
    (uint32_t)cpuNumber; \
})

extern void (*cpu_save_simd)(void *);
extern void (*cpu_restore_simd)(void *);
extern uint64_t CpuSimdRegionSize;

void smp_init();