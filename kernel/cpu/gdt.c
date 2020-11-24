#include <cpu/gdt.h>

void lgdt(gdt_pointer_t* gdt) {
    asm volatile ( "lgdt %0" : : "m" (*gdt));
}

static gdt_t gdt;
static gdt_pointer_t gdtPointer;

void gdt_init() {
    // Null descriptor.
    gdt.entries[0].limit       = 0;
    gdt.entries[0].base_low  = 0;
    gdt.entries[0].base_mid   = 0;
    gdt.entries[0].access      = 0;
    gdt.entries[0].granularity = 0;
    gdt.entries[0].base_hi  = 0;

    // Kernel code 64.
    gdt.entries[1].limit       = 0;
    gdt.entries[1].base_low  = 0;
    gdt.entries[1].base_mid   = 0;
    gdt.entries[1].access      = 0b10011010;
    gdt.entries[1].granularity = 0b00100000;
    gdt.entries[1].base_hi  = 0;

    // Kernel data 64.
    gdt.entries[2].limit       = 0;
    gdt.entries[2].base_low  = 0;
    gdt.entries[2].base_mid   = 0;
    gdt.entries[2].access      = 0b10010010;
    gdt.entries[2].granularity = 0;
    gdt.entries[2].base_hi  = 0;

    // User data 64.
    gdt.entries[3].limit       = 0;
    gdt.entries[3].base_low  = 0;
    gdt.entries[3].base_mid   = 0;
    gdt.entries[3].access      = 0b11110010;
    gdt.entries[3].granularity = 0;
    gdt.entries[3].base_hi  = 0;

    // User code 64.
    gdt.entries[4].limit       = 0;
    gdt.entries[4].base_low  = 0;
    gdt.entries[4].base_mid   = 0;
    gdt.entries[4].access      = 0b11111010;
    gdt.entries[4].granularity = 0b00100000;
    gdt.entries[4].base_hi  = 0;

    // TSS.
    gdt.tss.length       = 104;
    gdt.tss.base_low16   = 0;
    gdt.tss.base_mid8    = 0;
    gdt.tss.flags1       = 0b10001001;
    gdt.tss.flags2       = 0;
    gdt.tss.base_high8   = 0;
    gdt.tss.base_upper32 = 0;
    gdt.tss.reserved     = 0;

    // Set the pointer.
    gdtPointer.size    = sizeof(gdt) - 1;
    gdtPointer.address = (uint64_t)&gdt;

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
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
    ::: "memory", "rax");
}

void gdt_load_tss(size_t addr) {
    gdt.tss.base_low16   = (uint16_t)addr;
    gdt.tss.base_mid8    = (uint8_t)(addr >> 16);
    gdt.tss.flags1       = 0b10001001;
    gdt.tss.flags2       = 0;
    gdt.tss.base_high8   = (uint8_t)(addr >> 24);
    gdt.tss.base_upper32 = (uint32_t)(addr >> 32);
    gdt.tss.reserved     = 0;
}