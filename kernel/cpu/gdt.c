#include "gdt.h"

static volatile gdt_entry_t gdtEntries[7];

static void gdt_populate(int idx, uint32_t base, uint32_t limit, uint8_t flags, uint8_t access) {
    gdtEntries[idx].limitLo = limit;
    gdtEntries[idx].baseLo = base;
    gdtEntries[idx].baseMid = base >> 16;
    gdtEntries[idx].access = access;
    gdtEntries[idx].limitHiAndFlags = ((limit >> 16) & 0xFF) | (flags << 4);
    gdtEntries[idx].baseHi = base >> 24;
}

static void gdt_populate_tss(int idx, uint64_t base, uint32_t limit, uint8_t flags, uint8_t access) {
    gdt_populate(idx, base, limit, flags, access);
    *((uint64_t *)&gdtEntries[idx + 1]) = base >> 32;
}

void gdt_init() {
    gdt_populate(0                        , 0, 0  , 0   , 0   );
    gdt_populate(KERNEL_CODE_SEGMENT / 0x8, 0, 0  , 0x2 , 0x9A); // 0x08: Kernel code
    gdt_populate(KERNEL_DATA_SEGMENT / 0x8, 0, 0  , 0   , 0x92); // 0x10: Kernel data
    gdt_populate(USER_DATA_SEGMENT   / 0x8, 0, 0  , 0   , 0xF2); // 0x18: User data
    gdt_populate(USER_CODE_SEGMENT   / 0x8, 0, 0  , 0x2 , 0xFA); // 0x20: User code
    gdt_populate_tss(TSS_SEGMENT     / 0x8, 0, 104, 0   , 0x89);

    gdtr_t gdtr;
    gdtr.size = sizeof(gdtEntries) - 1;
    gdtr.offset = (uint64_t)gdtEntries;

    __asm__ volatile("lgdt %0" :: "m"(gdtr));

    asm volatile(
        "movq %%rsp, %%rax\n"
        "pushq $16\n"
        "pushq %%rax\n"
        "pushfq\n"
        "pushq $8\n"
        "lea fun, %%rax\n"
        "pushq %%rax\n"
        "iretq\n"
        "fun:\n"
        "movw $16, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
    ::: "memory", "rax");
}

void gdt_setup_tss(uint64_t address) {
    gdt_populate_tss(TSS_SEGMENT / 0x8, address, 104, 0, 0x89); //Set the gdt address
}