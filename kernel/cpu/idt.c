#include <cpu/idt.h>
#include <mem/mm.h>
#include <util/util.h>
#include <proc/smp.h>
#include <acpi/apic.h>

static idt_entry_t idtEntries[256];
static idt_pointer_t idtPointer = {
    .size = (sizeof(idt_entry_t) * 256) - 1,
    .entries = (idt_entry_t **)&idtEntries
};

void lidt(idt_pointer_t* idt) {
    asm volatile ( "lidt %0" : : "m" (*idt));
}

static void idt_register_int(size_t vector, void *handler, uint8_t ist, uint8_t type) {
    uint64_t pointer = (uint64_t)handler;

    idtEntries[vector].offset_low = pointer & 0xFFFF;
    idtEntries[vector].offset_mid = (pointer >> 16) & 0xFFFF;
    idtEntries[vector].offset_hi = (pointer >> 32) & 0xFFFFFFFF;
    idtEntries[vector].ist = ist;
    idtEntries[vector].type_attributes = type;
    idtEntries[vector].selector = 0x08;
    idtEntries[vector].zero = 0;
}

idt_fn_t irq_functions[0x30]; // Random value, to be filled

typedef void (*isr_fn_t)(void);

isr_fn_t handlers[256] = {
	isr0,   isr1,   isr2,   isr3,   isr4,   isr5,   isr6,   isr7,
	isr8,   isr9,   isr10,  isr11,  isr12,  isr13,  isr14,  isr15,
	isr16,  isr17,  isr18,  isr19,  isr20,  isr21,  isr22,  isr23,
	isr24,  isr25,  isr26,  isr27,  isr28,  isr29,  isr30,  isr31,
	isr32,  isr33,  isr34,  isr35,  isr36,  isr37,  isr38,  isr39,
	isr40,  isr41,  isr42,  isr43,  isr44,  isr45,  isr46,  isr47,
	isr48,  isr49,  isr50,  isr51,  isr52,  isr53,  isr54,  isr55,
	isr56,  isr57,  isr58,  isr59,  isr60,  isr61,  isr62,  isr63,
	isr64,  isr65,  isr66,  isr67,  isr68,  isr69,  isr70,  isr71,
	isr72,  isr73,  isr74,  isr75,  isr76,  isr77,  isr78,  isr79,
	isr80,  isr81,  isr82,  isr83,  isr84,  isr85,  isr86,  isr87,
	isr88,  isr89,  isr90,  isr91,  isr92,  isr93,  isr94,  isr95,
	isr96,  isr97,  isr98,  isr99,  isr100, isr101, isr102, isr103,
	isr104, isr105, isr106, isr107, isr108, isr109, isr110, isr111,
	isr112, isr113, isr114, isr115, isr116, isr117, isr118, isr119,
	isr120, isr121, isr122, isr123, isr124, isr125, isr126, isr127,
	isr128, isr129, isr130, isr131, isr132, isr133, isr134, isr135,
	isr136, isr137, isr138, isr139, isr140, isr141, isr142, isr143,
	isr144, isr145, isr146, isr147, isr148, isr149, isr150, isr151,
	isr152, isr153, isr154, isr155, isr156, isr157, isr158, isr159,
	isr160, isr161, isr162, isr163, isr164, isr165, isr166, isr167,
	isr168, isr169, isr170, isr171, isr172, isr173, isr174, isr175,
	isr176, isr177, isr178, isr179, isr180, isr181, isr182, isr183,
	isr184, isr185, isr186, isr187, isr188, isr189, isr190, isr191,
	isr192, isr193, isr194, isr195, isr196, isr197, isr198, isr199,
	isr200, isr201, isr202, isr203, isr204, isr205, isr206, isr207,
	isr208, isr209, isr210, isr211, isr212, isr213, isr214, isr215,
	isr216, isr217, isr218, isr219, isr220, isr221, isr222, isr223,
	isr224, isr225, isr226, isr227, isr228, isr229, isr230, isr231,
	isr232, isr233, isr234, isr235, isr236, isr237, isr238, isr239,
	isr240, isr241, isr242, isr243, isr244, isr245, isr246, isr247,
	isr248, isr249, isr250, isr251, isr252, isr253, isr254, isr255,
};

void dispatch_interrupt(irq_regs_t *regs) {
    if(regs->int_no < 0x20) {
        printf("[IRQ] Unknown system interrupt 0x%x received, error: 0x%lx, RIP: 0x%lx\n", regs->int_no, regs->err, regs->rip);
        if(regs->int_no == 0xD || regs->int_no == 0xE) {
            asm volatile("cli");
            while(1) { asm volatile("hlt"); }
        }
    } else if(regs->int_no == 0x41) {
        lapic_write(0xB0, 0);
        irq_functions[0x21](regs);
    } else if((sizeof(irq_functions) / sizeof(idt_fn_t)) > (regs->int_no - 0x20) && irq_functions[(regs->int_no - 0x20)]) {
        lapic_write(0xB0, 0);
        irq_functions[(regs->int_no - 0x20)](regs); // handle this manually
    } else {
        printf("[IRQ] Interrupt 0x%x received but not handled\n", regs->int_no);
        //lapic_write(0xB0, 0); -> Enable this to receive more interrupts AKA(EOI)
    }

    return;
}

extern void int_handler();

static size_t pit_count = 0;
extern int sched_ready;
extern size_t processors_count;
void pit_handler(thread_regs_t *regs) {
    pit_count++;

    if(pit_count % 10) {
        lapic_write(0xB0, 0);
        return;
    }

    if(!sched_ready) {
        lapic_write(0xB0, 0);
        return;
    }

    lapic_write(0xB0, 0);

    for(int i = 1; i < processors_count; i++) {
        lapic_write(0x310, ((uint32_t)cpuLocals[i].lapicId) << 24);
        lapic_write(0x300, 0x41);
    }

    schedule(regs);
}

void ipi_resched(thread_regs_t *regs) {
    lapic_write(0xB0, 0);
    schedule(regs);
}

void ksleep(size_t ms) {
    size_t ticksToWait = pit_count + ms + 1;

    while(pit_count < ticksToWait) {
        asm volatile("hlt");
    }
}

void service_interrupt2();

void idt_init() {
    for(int i = 0; i < 256; i++)
        idt_register_int(i, handlers[i], 0, 0x8E);

    lidt(&idtPointer);

    irq_functions[0] = (idt_fn_t)pit_handler;
}