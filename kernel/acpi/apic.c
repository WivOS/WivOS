#include "apic.h"
#include <proc/smp.h>

static uint32_t *lapic_base;
madt_ioapic_t **ioapics;
static size_t ioapics_count = 0;

static uint32_t ioapic_read(int ioapicIndex, uint32_t index) {
    if(ioapicIndex >= ioapics_count) return -1;
    *(volatile uint32_t*)(ioapics[ioapicIndex]->ioapicAddress + VIRT_PHYS_BASE) = index;
    return *(volatile uint32_t*)(ioapics[ioapicIndex]->ioapicAddress + 0x10 + VIRT_PHYS_BASE);
}

static void ioapic_write(int ioapicIndex, uint32_t index, uint32_t value) {
    if(ioapicIndex >= ioapics_count) return;
    *(volatile uint32_t*)(ioapics[ioapicIndex]->ioapicAddress + VIRT_PHYS_BASE) = index;
    *(volatile uint32_t*)(ioapics[ioapicIndex]->ioapicAddress + 0x10 + VIRT_PHYS_BASE) = value;
}

static uint8_t ioapic_max_redirects(int ioapicIndex) {
    return (ioapic_read(ioapicIndex, 0x1) >> 16) & 0xFF;
} 

void lapic_redirect(uint8_t vector, uint32_t gsi, uint16_t flags, int cpu, int status) {
    size_t ioapicIndex = -1;
    for(size_t i = 0; i < ioapics_count; i++) {
        if(ioapics[i]->globalSystemInterruptBase <= gsi && ioapics[i]->globalSystemInterruptBase + ioapic_max_redirects(i) > gsi) {
            ioapicIndex = i;
            break;
        }
    }

    size_t redirect = vector;

    if(flags & 0x2) redirect |= (1 << 13);
    if(flags & 0x8) redirect |= (1 << 15);
    if(!status) redirect |= (1 << 16);

    redirect |= ((uint64_t)cpuLocals[cpu].lapicId) << 56;
    
    uint32_t index = 0x10 + (gsi - ioapics[ioapicIndex]->globalSystemInterruptBase) * 2;

    ioapic_write(ioapicIndex, index, redirect);
    ioapic_write(ioapicIndex, index + 1, (redirect >> 32));
}

void lapic_legacy_irq(int cpu, uint8_t irq, int status) {
    for (uint8_t *madt_ptr = (uint8_t *)(&madt->madt_entries); (size_t)madt_ptr < (size_t)madt + madt->sdt.length; madt_ptr += *(madt_ptr + 1)) {
        switch(*(madt_ptr)) {
            case 2:
                {
                    madt_interrupt_t *iso = (madt_interrupt_t *)madt_ptr;
                    if(iso->irqSource == irq) {
                        lapic_redirect(iso->irqSource + 0x20, iso->globalSystemInterrupt, iso->flags, cpu, status);
                        return;
                    }
                    break;
                }
            default:
                break;
        }
    }
    lapic_redirect(irq + 0x20, irq, 0, cpu, status);
}

void lapic_connect_gsi_to_vec(int cpu, uint8_t vec, uint32_t gsi, uint16_t flags, int status) {
    lapic_redirect(vec, gsi, flags, cpu, status);
}

void lapic_write(uint32_t reg, uint32_t value) {
    *((uint32_t *)((uint64_t)lapic_base + reg)) = value;
}

uint32_t lapic_read(uint32_t reg) {
    return *((uint32_t *)((uint64_t)lapic_base + reg));
}

void lapic_enable() {
    lapic_write(0xF0, lapic_read(0xF0) | 0x1FF);
}

void lapic_init() {
    outb(0x20, 0x11);
    outb(0xa0, 0x11);

    outb(0x21, 0xa0);
    outb(0xa1, 0xa8);
    outb(0x21, 4);
    outb(0xa1, 2);
    outb(0x21, 0x01);
    outb(0xa1, 0x01);

    outb(0xA1, 0xFF);
    outb(0x21, 0xFF);

    ioapics = kcalloc(sizeof(madt_ioapic_t *), 256);
    for (uint8_t *madt_ptr = (uint8_t *)(&madt->madt_entries); (size_t)madt_ptr < (size_t)madt + madt->sdt.length; madt_ptr += *(madt_ptr + 1)) {
        switch(*(madt_ptr)) {
            case 1:
                printf("[SMP/MADT] IO APIC #%u\n", ioapics_count);
                ioapics[ioapics_count++] = (madt_ioapic_t *)madt_ptr;  
                break;
            default:
                break;
        }
    }

    lapic_base = (uint32_t *)(madt->localAPICAddress + VIRT_PHYS_BASE);
    lapic_enable();

    uint32_t x = 1193182 / 1000;

    uint8_t l = (uint8_t)(x & 0xFF);
    uint8_t h = (uint8_t)( (x>>8) & 0xFF );

    outb(0x43, 0x36);
    outb(0x40, l);
    outb(0x40, h);

    lapic_legacy_irq(0, 0, 1);
}