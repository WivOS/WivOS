#include <cpu/gdt.h>

void lgdt(gdt_pointer_t* gdt) {
    asm volatile ( "lgdt %0" : : "m" (*gdt));
}

static gdt_entry_t gdtEntries[5] = {
    { //null
        .limit = 0x0000,
        .base_low = 0x0000,
        .base_mid = 0x00,
        .base_hi = 0x00,
        .access = 0b00000000,
        .granularity = 0b00000000
    },
    { //kernel code
        .limit = 0x0000,
        .base_low = 0x0000,
        .base_mid = 0x00,
        .base_hi = 0x00,
        .access = 0b10011010,
        .granularity = 0b10101111
    },
    { //kernel data
        .limit = 0x0000,
        .base_low = 0x0000,
        .base_mid = 0x00,
        .base_hi = 0x00,
        .access = 0b10010010,
        .granularity = 0b00000000
    },
    { //user data
        .limit = 0x0000,
        .base_low = 0x0000,
        .base_mid = 0x00,
        .base_hi = 0x00,
        .access = 0b11110010,
        .granularity = 0b00000000
    },
    { //user code
        .limit = 0x0000,
        .base_low = 0x0000,
        .base_mid = 0x00,
        .base_hi = 0x00,
        .access = 0b11111010,
        .granularity = 0b00100000
    },
};

static gdt_pointer_t gdtPointer = {
    .size = sizeof(gdt_entry_t) * 5 - 1,
    .entries = (gdt_entry_t **)&gdtEntries
};

void gdt_init() {
    lgdt(&gdtPointer);

    asm volatile(
        "movq %%rsp, %%rax\n"
        "pushq $16\n"
        "pushq %%rax\n"
        "pushfq\n"
        "pushq $8\n"
        "pushq $1f\n"
        "iretq\n"
        "1:\n"
        "movw $16, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw $0x1b, %%ax\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
    ::: "memory", "rax");
}