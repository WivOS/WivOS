#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util/string.h>

typedef struct gdt_entry {
    uint16_t limit;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_hi;
} __attribute__((packed)) gdt_entry_t;

typedef struct tss_entry {
    uint16_t length;
    uint16_t base_low16;
    uint8_t  base_mid8;
    uint8_t  flags1;
    uint8_t  flags2;
    uint8_t  base_high8;
    uint32_t base_upper32;
    uint32_t reserved;
} __attribute__((packed)) tss_entry_t;

typedef struct gdt {
    gdt_entry_t entries[5];
    tss_entry_t tss;
} __attribute__((packed)) gdt_t;

typedef struct gdt_pointer {
    uint16_t size;
    uint64_t address;
} __attribute__((packed)) gdt_pointer_t;

void gdt_init();