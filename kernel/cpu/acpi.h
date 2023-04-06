#pragma once

#include <utils/common.h>
#include <stdbool.h>

#define MAX_CPUS 256

typedef struct rsdp {
    char signature[8];
    uint8_t checksum;
    char oemId[6];
    uint8_t revision;
    uint32_t rsdtAddress;

    //rev 2
    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t extendedChecksum;
    uint8_t reserved[2];
} __attribute__((packed)) rsdp_t;

typedef struct sdt {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemId[6];
    char oemTableId[8];
    uint32_t oemRev;
    uint32_t creatorId;
    uint32_t creatorRev;
} __attribute__((packed)) sdt_t;

typedef struct rsdt {
    sdt_t sdt;
    uint32_t sdtAddresses[];
} __attribute__((packed)) rsdt_t;

typedef struct xsdt {
    sdt_t sdt;
    uint64_t sdtAddresses[];
} __attribute__((packed)) xsdt_t;

typedef struct madt_entry {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_entry_t;

typedef struct madt {
    sdt_t sdt;
    uint32_t lapicBase;
    uint32_t flags;
    madt_entry_t entries[];
} __attribute__((packed)) madt_t;

void acpi_init(void *rsdpAddress);
void acpi_post_init();

void *acpi_get_table(char *sig, size_t index);

void lapic_write(uint32_t reg, uint32_t data);
uint32_t lapic_read(uint32_t reg);

void ioapic_write(size_t ioapic, uint32_t reg, uint32_t data);
uint32_t ioapic_read(size_t ioapic, uint32_t reg);
size_t ioapic_get_id_from_gsi(uint32_t gsi);

void ioapic_redirect_gsi(uint8_t vector, uint32_t gsi, uint16_t flags, int cpu, bool status);
void ioapic_redirect_legacy_irq(int cpu, uint8_t irq, bool status);