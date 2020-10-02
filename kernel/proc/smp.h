#pragma once

#include <util/util.h>
#include "proc.h"
#include <acpi/acpi.h>

typedef struct madt {
    sdt_t sdt;
    uint32_t localAPICAddress;
    uint32_t flags;
    uint8_t madt_entries;
} madt_t;

typedef struct madt_entry_header {
    uint8_t entryType;
    uint8_t recordLength;
} madt_entry_header_t;

typedef struct madt_processor {
    madt_entry_header_t header;
    uint8_t processorID;
    uint8_t apicID;
    uint32_t flags;
} madt_processor_t;

typedef struct madt_ioapic {
    madt_entry_header_t header;
    uint8_t ioapicID;
    uint8_t reserved;
    uint32_t ioapicAddress;
    uint32_t globalSystemInterruptBase;
} madt_ioapic_t;

typedef struct madt_interrupt {
    madt_entry_header_t header;
    uint8_t busSource;
    uint8_t irqSource;
    uint32_t globalSystemInterrupt;
    uint16_t flags;
} madt_interrupt_t;

typedef struct madt_nonMaskable_interrupt {
    madt_entry_header_t header;
    uint8_t busSource;
    uint16_t flags;
    uint8_t lint;
} madt_nonMaskable_interrupt_t;

typedef struct madt_lapic {
    madt_entry_header_t header;
    uint16_t reserved;
    uint64_t lapicAddress;
} madt_lapic_t;

typedef struct cpu {
    size_t currentCpu;
    size_t kernelStack;
    tid_t currentThread;
    pid_t currentProcess;
    size_t currentActiveThread;
    uint8_t lapicId;
} cpu_t;

#define current_cpu ({ \
    size_t cpuNumber; \
    asm volatile("movq %%gs:(0), %0" \
                   : "=r"(cpuNumber) : \
                   : "memory", "cc"); \
    (int)cpuNumber; \
})

extern cpu_t cpuLocals[];

extern madt_t *madt;
extern madt_processor_t **processors;
extern size_t processors_count;

void smp_init();