#include "acpi.h"
#include "cpu.h"

#include <mem/pmm.h>

#include <acpispec/tables.h>
#include <lai/helpers/sci.h>

static xsdt_t *XSDTTable = NULL;
static rsdt_t *RSDTTable = NULL;
static rsdp_t *RSDPHeader = NULL;

static madt_t *MADTTable = NULL;
static int MADTCount = 0;

typedef struct {
    madt_entry_t header;
    uint8_t id;
    uint8_t reserved;
    uint32_t address;
    uint32_t gsiBase;
} ioapic_entry_t;

typedef struct {
    madt_entry_t header;
    uint8_t busSource;
    uint8_t irqSource;
    uint32_t gsi;
    uint16_t flags;
} ioapic_irq_source_t;

typedef struct {
    int length;
    ioapic_entry_t entries[];
} ioapic_list_t;

static ioapic_list_t *IOAPICList = NULL;

void acpi_init(void *rsdpAddress) {
    RSDPHeader = (rsdp_t *)(rsdpAddress);

    if(RSDPHeader->revision >= 2 && RSDPHeader->xsdtAddress) {
        XSDTTable = (xsdt_t *)(RSDPHeader->xsdtAddress + MEM_PHYS_OFFSET);
    } else {
        RSDTTable = (rsdt_t *)(RSDPHeader->rsdtAddress + MEM_PHYS_OFFSET);
    }

    MADTTable = acpi_get_table("APIC", 0);

    for(int i = 0; i < MAX_CPUS; i++) {
        CPULocals[i].currentCpu = -1;
        CPULocals[i].lapicID = -1;
        CPULocals[i].currentPid = -1;
        CPULocals[i].currentTid = -1;
        CPULocals[i].currentTaskID = -1;
    }

    uint32_t entriesLength = MADTTable->sdt.length - sizeof(madt_t);
    int numberOfIoApic = 0;
    madt_entry_t *entry = &MADTTable->entries[0];
    while(entriesLength) {
        if(entry->type == 1) numberOfIoApic++;
        MADTCount++;

        entriesLength -= entry->length;
        entry = (madt_entry_t *)((uint64_t)entry + entry->length);
    }

    IOAPICList = kmalloc(sizeof(ioapic_list_t) + sizeof(ioapic_entry_t) * numberOfIoApic);
    IOAPICList->length = numberOfIoApic;
    numberOfIoApic = 0;

    entry = &MADTTable->entries[0];
    for (int i = 0; i < MADTCount; i++) {
        switch(entry->type) {
            case 0x0:
                {
                    ioapic_entry_t *lapicEntry = (ioapic_entry_t *)entry;
                    CPULocals[lapicEntry->id].currentCpu = lapicEntry->id;
                    CPULocals[lapicEntry->id].lapicID = lapicEntry->address ? lapicEntry->reserved : -1;
                }
                break;
            case 0x1:
                {
                    ioapic_entry_t *ioapicEntry = (ioapic_entry_t *)entry;
                    IOAPICList->entries[ioapicEntry->id] = *ioapicEntry;
                }
                break;
        }

        entry = (madt_entry_t *)((uint64_t)entry + entry->length);
    }

    lapic_write(0xf0, lapic_read(0xf0) | 0x1ff);
}

void acpi_post_init() {
    lai_set_acpi_revision(RSDPHeader->revision);
    lai_create_namespace();
    lai_enable_acpi(1);
}

void *acpi_get_table(char *sig, size_t index) {
    if(!strncmp(sig, "DSDT", 4)) {
        if(index > 0) return NULL;

        acpi_fadt_t *fadtTable = (acpi_fadt_t *)acpi_get_table("FACP", 0);
        void *dsdtTable = (char *)((size_t)fadtTable->dsdt + MEM_PHYS_OFFSET);
        return dsdtTable;
    } else {
        size_t curr = 0;
        if(XSDTTable != NULL) {
            size_t entries = (XSDTTable->sdt.length - sizeof(XSDTTable->sdt)) / 4;

            for(size_t i = 0; i < entries; i++) {
                sdt_t *sdtHeader = (sdt_t *)(XSDTTable->sdtAddresses[i] + MEM_PHYS_OFFSET);
                if(!strncmp(sdtHeader->signature, sig, 4)) {
                    if(curr++ == index)
                        return (void *)sdtHeader;
                }
            }
        } else {
            size_t entries = (RSDTTable->sdt.length - sizeof(RSDTTable->sdt)) / 4;

            for(size_t i = 0; i < entries; i++) {
                sdt_t *sdtHeader = (sdt_t *)(RSDTTable->sdtAddresses[i] + MEM_PHYS_OFFSET);
                if(!strncmp(sdtHeader->signature, sig, 4)) {
                    if(curr++ == index)
                        return (void *)sdtHeader;
                }
            }
        }

        return NULL;
    }
}

void lapic_write(uint32_t reg, uint32_t data) {
    if(!MADTTable) return;
    size_t lapicBase = (size_t)MADTTable->lapicBase + MEM_PHYS_OFFSET;
    *((volatile uint32_t *)(lapicBase + reg)) = data;
}

uint32_t lapic_read(uint32_t reg) {
    if(!MADTTable) return 0;

    size_t lapicBase = (size_t)MADTTable->lapicBase + MEM_PHYS_OFFSET;
    return *((volatile uint32_t *)(lapicBase + reg));
}

void ioapic_write(size_t ioapic, uint32_t reg, uint32_t data) {
    if(ioapic == -1) while(1);
    volatile uint32_t *ioapicBase = (volatile uint32_t *)((size_t)IOAPICList->entries[ioapic].address + MEM_PHYS_OFFSET);
    *ioapicBase = reg;
    *(ioapicBase + 4) = data;
}

uint32_t ioapic_read(size_t ioapic, uint32_t reg) {
    if(ioapic == -1) while(1);
    volatile uint32_t *ioapicBase = (volatile uint32_t *)((size_t)IOAPICList->entries[ioapic].address + MEM_PHYS_OFFSET);
    *ioapicBase = reg;
    return *(ioapicBase + 4);
}

static uint32_t ioapic_get_max_redirections(size_t ioapic) {
    return ((ioapic_read(ioapic, 0x1) >> 16) & 0xFF) + 1; //Zero means 1
}

size_t ioapic_get_id_from_gsi(uint32_t gsi) {
    for(int i = 0; i < IOAPICList->length; i++) {
        ioapic_entry_t *entry = &IOAPICList->entries[i];
        if(entry->gsiBase <= gsi && (entry->gsiBase + ioapic_get_max_redirections(i)) > gsi) {
            //Hit
            return i;
        }
    }

    return -1;
}

void ioapic_redirect_gsi(uint8_t vector, uint32_t gsi, uint16_t flags, int cpu, bool status) {
    size_t ioapic = ioapic_get_id_from_gsi(gsi);
    if(ioapic == -1) return;

    uint64_t redirectValue = vector;

    redirectValue |= (flags & 0xA) << 13;

    if(!status) redirectValue |= (1 << 16);

    redirectValue |= ((uint64_t)CPULocals[cpu].lapicID) << 56;
    uint32_t ioredtbl = (gsi - IOAPICList->entries[ioapic].gsiBase) * 2 + 16;

    ioapic_write(ioapic, ioredtbl, redirectValue);
    ioapic_write(ioapic, ioredtbl + 1, redirectValue >> 32);
}

void ioapic_redirect_legacy_irq(int cpu, uint8_t irq, bool status) {
    madt_entry_t *entry = &MADTTable->entries[0];
    for (int i = 0; i < MADTCount; i++) {
        if(entry->type == 0x2) {
            ioapic_irq_source_t *irqSourceEntry = (ioapic_irq_source_t *)entry;
            if(irqSourceEntry->irqSource == irq) {
                ioapic_redirect_gsi(irqSourceEntry->irqSource + 0x20, irqSourceEntry->gsi, irqSourceEntry->flags, cpu, status);
                return;
            }
        }

        entry = (madt_entry_t *)((uint64_t)entry + entry->length);
    }

    ioapic_redirect_gsi(0x20 + irq, irq, 0, cpu, status);
}