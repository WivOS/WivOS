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

typedef struct gdt_pointer {
    uint16_t size;
    gdt_entry_t **entries;
} __attribute__((packed)) gdt_pointer_t;

void gdt_init();