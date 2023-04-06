#pragma once

#include <utils/common.h>
#include <cpu/acpi.h>
#include <cpu/idt.h>

typedef struct hpet_table {
    sdt_t sdt;
    uint8_t hardwareRevId;
    uint8_t comparatorCount : 5;
    uint8_t counterSize : 1;
    uint8_t reserved : 1;
    uint8_t legacyReplacement : 1;
    uint16_t pciVendorId;

    uint8_t addressSpaceId;
    uint8_t registerBitWidth;
    uint8_t registerBitOffset;
    uint8_t addressReserved;
    uint64_t address;

    uint8_t hpetNumber;
    uint16_t minimumClock;
    uint8_t pageProtection;
} __attribute__((packed)) hpet_table_t;

typedef struct hpet_timer {
    volatile uint64_t configAndCapabilities;
    volatile uint64_t comparatorValue;
    volatile uint64_t fsbInterruptRoute;
    volatile uint64_t unused;
} __attribute__((packed)) hpet_timer_t;

typedef struct hpet {
    volatile uint64_t generalCapabilities;
    volatile uint64_t unused0;
    volatile uint64_t generalConfiguration;
    volatile uint64_t unused1;
    volatile uint64_t generalInterruptStatus;
    volatile uint64_t unused2;
    volatile uint64_t unused3[2][12];
    volatile uint64_t mainCounterValue;
    volatile uint64_t unused4;
    hpet_timer_t timers[];
} __attribute__((packed)) hpet_t;

#define TIMER_FREQ 1000

void timer_init();

bool timer_handler(irq_regs_t *regs);
uint64_t timer_get_nanoseconds();

void ksleep(uint32_t ms);