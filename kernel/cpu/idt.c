#include "idt.h"
#include "cpu.h"

#include "acpi.h"

#include "../tasking/scheduler.h"

static idt_entry_t idtEntries[256];

typedef void (*isr_fn_t)(void);

static isr_fn_t handlers[256];
irq_fn_t IRQFunctions[0x40];

#define MAX_GSI_PINS 32

irq_pci_fn_t pciIrqPins[MAX_GSI_PINS][10];
void *pciIrqPinsData[MAX_GSI_PINS][10];
static bool idt_irq_pins_connected[MAX_GSI_PINS];

static void idt_populate(int idx, uint64_t offset, uint8_t segment, uint8_t ist, uint8_t typeAttr) {
	idtEntries[idx].offsetLo = offset;
    idtEntries[idx].segment = segment;
    idtEntries[idx].ist = ist;
    idtEntries[idx].typeAttr = typeAttr;
    idtEntries[idx].offsetMid = offset >> 16;
    idtEntries[idx].offsetHi = offset >> 32;
    idtEntries[idx].reserved = 0;
}

void dispatch_interrupt(irq_regs_t *regs) {

	if(regs->int_no >= 0x90 && regs->int_no < (0x90 + MAX_GSI_PINS)) {
		bool found = false;
		for(int i = 0; i < 10; i++) {
			if(pciIrqPins[regs->int_no - 0x90][i] != NULL) {
				if(pciIrqPins[regs->int_no - 0x90][i](regs, pciIrqPinsData[regs->int_no - 0x90][i])) {
					lapic_write(0xB0, 0);
					found = true;
					break;
				}
			}
		}
		if(!found) lapic_write(0xB0, 0);
	} else if(regs->int_no < 0x20 || regs->int_no >= 0x60) {
		ktid_t current_tid = CPULocals[CurrentCPU].currentTid;
		kpid_t current_pid = CPULocals[CurrentCPU].currentPid;
		printf("[IRQ] Unknown system interrupt 0x%x received, error: 0x%lx, RIP: 0x%lx, pid: 0x%lx, tid: 0x%lx\n", regs->int_no, regs->err, regs->rip, current_pid, current_tid);
        if(regs->int_no == 0xD || regs->int_no == 0xE) {
            asm volatile("cli");
            uint64_t cr2;
            asm volatile("mov %%cr2, %%rax\n\t"
                         "mov %%rax, %0"
                         : "=m"(cr2) : : "%rax");
			uint64_t cr3;
            asm volatile("mov %%cr3, %%rax\n\t"
                         "mov %%rax, %0"
                         : "=m"(cr3) : : "%rax");
            printf("CR2: %llx\n", cr2);
			printf("CR3: %llx\n", cr3);
            printf("RSP: %llx\n", regs->rsp);
			printf("RBP: %llx\n", regs->rbp);

			process_t *process = SchedulerProcesses[0x2];
			uint8_t buffer[0x60];
			vmm_read((pt_t *)process->page_table, (void *)(0x41017410 - 0x30), buffer, 0x60);
			for(int i = 0; i < 0x60; i++) {
				printf("%02X, ", buffer[i]);
				if((i % 0x10) == 0xF) printf("\n");
			}
			printf("");
            while(1) { asm volatile("hlt"); }
        }
	} else {
		if(IRQFunctions[(regs->int_no) - 0x20]) {
			if(IRQFunctions[(regs->int_no) - 0x20](regs)) {
				lapic_write(0xB0, 0);
			}
		} else {
			lapic_write(0xB0, 0);
		}
	}
}

void idt_init() {
    __asm__ volatile("cli");

    for(int i = 0; i < 256; i++)
        idt_populate(i, (uint64_t)handlers[i], 0x8, 0x00, 0x8E);

    idtr_t idtr;
    idtr.size = sizeof(idtEntries) - 1;
    idtr.offset = (uint64_t)idtEntries;

	for(int i = 0; i < 0x40; i++) IRQFunctions[i] = (irq_fn_t)NULL;

	for(int i = 0; i < MAX_GSI_PINS; i++) {
		idt_irq_pins_connected[i] = false;
	}

    __asm__ volatile("lidt %0; sti;" :: "m"(idtr));
}

void idt_add_pci_handler(uint8_t gsi, irq_pci_fn_t func, uint16_t flags, void *data) {
	if(gsi >= MAX_GSI_PINS) return;

	for(int i = 0; i < 10; i++) {
		if(pciIrqPins[gsi][i] == NULL) {
			pciIrqPins[gsi][i] = func;
			pciIrqPinsData[gsi][i] = data;
			break;
		}
	}

	if(!idt_irq_pins_connected[gsi]) {
		ioapic_redirect_gsi(0x90 + gsi, gsi, flags, 0, true);
		idt_irq_pins_connected[gsi] = true;
	}
}

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);
extern void isr32(void);
extern void isr33(void);
extern void isr34(void);
extern void isr35(void);
extern void isr36(void);
extern void isr37(void);
extern void isr38(void);
extern void isr39(void);
extern void isr40(void);
extern void isr41(void);
extern void isr42(void);
extern void isr43(void);
extern void isr44(void);
extern void isr45(void);
extern void isr46(void);
extern void isr47(void);
extern void isr48(void);
extern void isr49(void);
extern void isr50(void);
extern void isr51(void);
extern void isr52(void);
extern void isr53(void);
extern void isr54(void);
extern void isr55(void);
extern void isr56(void);
extern void isr57(void);
extern void isr58(void);
extern void isr59(void);
extern void isr60(void);
extern void isr61(void);
extern void isr62(void);
extern void isr63(void);
extern void isr64(void);
extern void isr65(void);
extern void isr66(void);
extern void isr67(void);
extern void isr68(void);
extern void isr69(void);
extern void isr70(void);
extern void isr71(void);
extern void isr72(void);
extern void isr73(void);
extern void isr74(void);
extern void isr75(void);
extern void isr76(void);
extern void isr77(void);
extern void isr78(void);
extern void isr79(void);
extern void isr80(void);
extern void isr81(void);
extern void isr82(void);
extern void isr83(void);
extern void isr84(void);
extern void isr85(void);
extern void isr86(void);
extern void isr87(void);
extern void isr88(void);
extern void isr89(void);
extern void isr90(void);
extern void isr91(void);
extern void isr92(void);
extern void isr93(void);
extern void isr94(void);
extern void isr95(void);
extern void isr96(void);
extern void isr97(void);
extern void isr98(void);
extern void isr99(void);
extern void isr100(void);
extern void isr101(void);
extern void isr102(void);
extern void isr103(void);
extern void isr104(void);
extern void isr105(void);
extern void isr106(void);
extern void isr107(void);
extern void isr108(void);
extern void isr109(void);
extern void isr110(void);
extern void isr111(void);
extern void isr112(void);
extern void isr113(void);
extern void isr114(void);
extern void isr115(void);
extern void isr116(void);
extern void isr117(void);
extern void isr118(void);
extern void isr119(void);
extern void isr120(void);
extern void isr121(void);
extern void isr122(void);
extern void isr123(void);
extern void isr124(void);
extern void isr125(void);
extern void isr126(void);
extern void isr127(void);
extern void isr128(void);
extern void isr129(void);
extern void isr130(void);
extern void isr131(void);
extern void isr132(void);
extern void isr133(void);
extern void isr134(void);
extern void isr135(void);
extern void isr136(void);
extern void isr137(void);
extern void isr138(void);
extern void isr139(void);
extern void isr140(void);
extern void isr141(void);
extern void isr142(void);
extern void isr143(void);
extern void isr144(void);
extern void isr145(void);
extern void isr146(void);
extern void isr147(void);
extern void isr148(void);
extern void isr149(void);
extern void isr150(void);
extern void isr151(void);
extern void isr152(void);
extern void isr153(void);
extern void isr154(void);
extern void isr155(void);
extern void isr156(void);
extern void isr157(void);
extern void isr158(void);
extern void isr159(void);
extern void isr160(void);
extern void isr161(void);
extern void isr162(void);
extern void isr163(void);
extern void isr164(void);
extern void isr165(void);
extern void isr166(void);
extern void isr167(void);
extern void isr168(void);
extern void isr169(void);
extern void isr170(void);
extern void isr171(void);
extern void isr172(void);
extern void isr173(void);
extern void isr174(void);
extern void isr175(void);
extern void isr176(void);
extern void isr177(void);
extern void isr178(void);
extern void isr179(void);
extern void isr180(void);
extern void isr181(void);
extern void isr182(void);
extern void isr183(void);
extern void isr184(void);
extern void isr185(void);
extern void isr186(void);
extern void isr187(void);
extern void isr188(void);
extern void isr189(void);
extern void isr190(void);
extern void isr191(void);
extern void isr192(void);
extern void isr193(void);
extern void isr194(void);
extern void isr195(void);
extern void isr196(void);
extern void isr197(void);
extern void isr198(void);
extern void isr199(void);
extern void isr200(void);
extern void isr201(void);
extern void isr202(void);
extern void isr203(void);
extern void isr204(void);
extern void isr205(void);
extern void isr206(void);
extern void isr207(void);
extern void isr208(void);
extern void isr209(void);
extern void isr210(void);
extern void isr211(void);
extern void isr212(void);
extern void isr213(void);
extern void isr214(void);
extern void isr215(void);
extern void isr216(void);
extern void isr217(void);
extern void isr218(void);
extern void isr219(void);
extern void isr220(void);
extern void isr221(void);
extern void isr222(void);
extern void isr223(void);
extern void isr224(void);
extern void isr225(void);
extern void isr226(void);
extern void isr227(void);
extern void isr228(void);
extern void isr229(void);
extern void isr230(void);
extern void isr231(void);
extern void isr232(void);
extern void isr233(void);
extern void isr234(void);
extern void isr235(void);
extern void isr236(void);
extern void isr237(void);
extern void isr238(void);
extern void isr239(void);
extern void isr240(void);
extern void isr241(void);
extern void isr242(void);
extern void isr243(void);
extern void isr244(void);
extern void isr245(void);
extern void isr246(void);
extern void isr247(void);
extern void isr248(void);
extern void isr249(void);
extern void isr250(void);
extern void isr251(void);
extern void isr252(void);
extern void isr253(void);
extern void isr254(void);
extern void isr255(void);

static isr_fn_t handlers[256] = {
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