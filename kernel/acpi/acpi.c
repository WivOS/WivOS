#include "acpi.h"
#include <lai/core.h>

static xsdt_t *xsdtTable = NULL;
static rsdt_t *rsdtTable = NULL;

void acpi_init(stivale2_struct_tag_rsdp_t *rsdp) {
    rsdp_t *rsdpHeader = (rsdp_t *)(rsdp->rsdp + VIRT_PHYS_BASE);

    if(rsdpHeader->revision >= 2 && rsdpHeader->xsdtAddress) {
        xsdtTable = (xsdt_t *)(rsdpHeader->xsdtAddress + VIRT_PHYS_BASE);
    } else {
        rsdtTable = (rsdt_t *)(rsdpHeader->rsdtAddress + VIRT_PHYS_BASE);
    }

    lai_set_acpi_revision(rsdpHeader->revision);
    lai_create_namespace();
}

void *acpi_find_sdt(const char *signature, size_t index) {
    size_t curr = 0;
    if(xsdtTable != NULL) {
        size_t entries = (xsdtTable->sdt.length - sizeof(xsdtTable->sdt)) / 4;

        for(size_t i = 0; i < entries; i++) {
            sdt_t *sdtHeader = (sdt_t *)(xsdtTable->sdtAddresses[i] + VIRT_PHYS_BASE);
            if(!strncmp(sdtHeader->signature, signature, 4)) {
                if(curr++ == index)
                    return (void *)sdtHeader;
            }
        }
    } else {
        size_t entries = (rsdtTable->sdt.length - sizeof(rsdtTable->sdt)) / 4;

        for(size_t i = 0; i < entries; i++) {
            sdt_t *sdtHeader = (sdt_t *)(rsdtTable->sdtAddresses[i] + VIRT_PHYS_BASE);
            if(!strncmp(sdtHeader->signature, signature, 4)) {
                if(curr++ == index)
                    return (void *)sdtHeader;
            }
        }
    }

    return NULL;
}