#pragma once

#include <util/util.h>
#include <boot/stivale2.h>

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
} rsdt_t;

typedef struct xsdt {
    sdt_t sdt;
    uint64_t sdtAddresses[];
} xsdt_t;

void acpi_init(stivale2_struct_tag_rsdp_t *rsdp);
void *acpi_find_sdt(const char *signature, size_t index);