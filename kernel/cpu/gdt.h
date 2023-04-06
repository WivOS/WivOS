#pragma once

#include <utils/common.h>

typedef struct {
    uint16_t limitLo;
    uint16_t baseLo;
    uint8_t baseMid;
    uint8_t access;
    uint8_t limitHiAndFlags;
    uint8_t baseHi;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) gdtr_t;

#define KERNEL_CODE_SEGMENT     0x08
#define KERNEL_DATA_SEGMENT     0x10
#define USER_DATA_SEGMENT       0x18
#define USER_CODE_SEGMENT       0x20
#define TSS_SEGMENT             0x28

void gdt_init();
void gdt_setup_tss(uint64_t address);