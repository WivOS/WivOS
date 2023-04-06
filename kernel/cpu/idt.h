#pragma once

#include <utils/common.h>
#include <stdbool.h>

#define RESCHED_IPI_IRQ 0x40

typedef struct irq_regs {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8,
			 rdi, rsi, rbp, rdx, rcx, rbx, rax;
	uint64_t int_no, err;
	uint64_t rip, cs, rflags;
	uint64_t rsp, ss;
} irq_regs_t;

typedef struct {
    uint16_t offsetLo;
    uint16_t segment;
    uint8_t ist;
    uint8_t typeAttr;
    uint16_t offsetMid;
    uint32_t offsetHi;
    uint32_t reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) idtr_t;

typedef bool (*irq_fn_t)(irq_regs_t *);
extern irq_fn_t IRQFunctions[0x40];

void idt_init();

void idt_add_pci_handler(uint8_t gsi, irq_fn_t func, uint16_t flags);