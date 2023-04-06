#include "timer.h"

#include <mem/pmm.h>
#include <cpu/cpu.h>

#include <tasking/scheduler.h>

static hpet_table_t *HPETTable = NULL;
static hpet_t *HPET = NULL;
static uint64_t HPETFrequency = 1;
static uint64_t HPETFrequencyNano = 1;

void timer_init() {
    outb(0xa1, 0xff);
    outb(0x21, 0xff); //Disable pic

    HPETTable = acpi_get_table("HPET", 0);

    if(!HPETTable) {
        printf("This kernels uses HPET but it isn't supported\n");
        while(1);
    }

    if(HPETTable->addressSpaceId) {
        printf("Only memory mapped HPET is supported\n");
        while(1);
    }

    HPET = (hpet_t *)(HPETTable->address + MEM_PHYS_OFFSET);

    uint64_t capabilities = HPET->generalCapabilities;

    if(!(capabilities & (1 << 15))) {
        printf("Legacy replacement HPET is supported\n");
        while(1);
    }

    uint64_t clkPeriod = capabilities >> 32;
    uint64_t frequency = 1000000000000000 / clkPeriod;
    HPETFrequency = frequency;
    HPETFrequencyNano = 1000000 / clkPeriod;

    //Enable legacy replacement
    capabilities |= 0x2;
    HPET->generalCapabilities = capabilities;

    //Reset counter value
    HPET->mainCounterValue = 0;

    capabilities = HPET->timers[0].configAndCapabilities;
    if(!(capabilities & (1 << 4))) {
        printf("Timer 0 does not support periodic mode\n");
        while(1);
    }

    capabilities |= 0x4C;
    HPET->timers[0].configAndCapabilities = capabilities;
    HPET->timers[0].comparatorValue = frequency / TIMER_FREQ; //1000 Hz

    capabilities = HPET->generalConfiguration;
    capabilities |= 0x1;
    HPET->generalConfiguration = capabilities;

    IRQFunctions[0] = (irq_fn_t)timer_handler;

    ioapic_redirect_legacy_irq(0, 0, true);
}

extern void scheduler_schedule_smp(irq_regs_t *regs);

volatile size_t TimerCounter = 0;
volatile size_t TimerSeconds = 0;
bool timer_handler(irq_regs_t *regs) {
    TimerCounter++;

    if(!(TimerCounter % TIMER_FREQ)) {
        TimerSeconds = HPET->mainCounterValue / HPETFrequency;
    }

    if(!SchedulerRunning) return true;

    scheduler_schedule_smp(regs);

	return false;
}

void ksleep(uint32_t ms) {
    if(HPETTable) { //Don't halt when hpet is not initialized
        uint32_t expectedCounter = TimerCounter + ( (ms * TIMER_FREQ) / 1000);
        while(TimerCounter < expectedCounter);
    }
}

uint64_t timer_get_nanoseconds() {
    if(!HPETTable) return 0;
    return HPET->mainCounterValue / (HPETFrequencyNano);
}