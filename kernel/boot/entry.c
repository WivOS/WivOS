#include "stivale2.h"
#include <util/util.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/mm.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <fs/vfs.h>
#include <fs/initrd/tar.h>

#include <acpi/acpi.h>
#include <proc/smp.h>

uint8_t g_bootstrap_stack[0x1000] = {0};

stivale2_hdr_tag_framebuffer_t g_header_tag_framebuffer = {
    .identifier = 0x3ecc1bc43d0f7971,
    .next = (uint64_t)NULL,
    .framebuffer_width = 1024,
    .framebuffer_height = 768,
    .framebuffer_bpp = 32
};

__attribute__((used, section(".stivale2hdr")))
stivale2_header_t header = {
    .entry_point = 0,
    .stack = (uint64_t) (g_bootstrap_stack + sizeof(g_bootstrap_stack)),
    .flags = 0,
    .tags = (uint64_t)&g_header_tag_framebuffer
};

static void testOpen() {
    return;
}

static size_t testRead(vfs_node_t *node, char *buffer, size_t offset, size_t size) {
    printf("Drive readed, Offset: 0x%lx Size: 0x%lx\n", offset, size);
    return 1;
}

void kentry(stivale2_struct_t *stivale) {
    printf("Initializing WivOS\n");

    printf("[Stivale2] Showing structs:\n");
    printf("[Stivale2]\tBootloader brand: %s\n", stivale->bootloader_brand);
    printf("[Stivale2]\tBootloader version: %s\n", stivale->bootloader_version);
    
    stivale2_module_t *initrdModule = NULL;
    stivale2_struct_tag_rsdp_t *rsdptag = NULL;

    printf("[Stivale2]\tTags:\n");
    uint64_t stivaleTags = stivale->tags;
    while(1) {
        if(stivaleTags == (uint64_t)NULL) break;

        stivale2_hdr_tag_t *currTag = (stivale2_hdr_tag_t *)stivaleTags;
        printf("[Stivale2]\t\tType: %lx %lx\n", currTag->identifier, stivaleTags);
        switch(currTag->identifier) {
            case TAG_MEMMAP:
                {
                    stivale2_struct_tag_memmap_t *memmaptag = (stivale2_struct_tag_memmap_t *)currTag;
                    printf("[Stivale2]\t\t\tMemory map with %d entries:\n", memmaptag->entries);
                    for(int i = 0; i < memmaptag->entries; i++) {
                        stivale2_mmap_entry_t* entry = (stivale2_mmap_entry_t *)((size_t)&memmaptag->memmap[i]);
                        char *str = "Unknown";
                        switch(entry->type) {
                            case USABLE:                    str = "USABLE"; break;
                            case RESERVED:                  str = "RESERVED"; break;
                            case ACPI_RECLAIMABLE:          str = "ACPI_RECLAIMABLE"; break;
                            case ACPI_NVS:                  str = "ACPI_NVS"; break;
                            case BAD_MEMORY:                str = "BAD_MEMORY"; break;
                            case BOOTLOADER_RECLAIMABLE:    str = "BOOTLOADER_RECLAIMABLE"; break;
                            case KERNEL_AND_MODULES:        str = "KERNEL_AND_MODULES"; break;
                            default: break;
                        }
                        printf("[Stivale2]\t\t\t%016lx -> %016lx: %s\n", entry->base, (entry->base + entry->length), str);
                    }
                }
                break;
            case TAG_FRAMEBUFFER:
                {
                    stivale2_struct_tag_framebuffer_t *framebuffertag = (stivale2_struct_tag_framebuffer_t *)currTag;
                    printf("[Stivale2]\t\t\tFramebuffer:\n[Stivale2]\t\t\tAddr: 0x%lx\n[Stivale2]\t\t\tWidth: %dpx\n[Stivale2]\t\t\tHeight: %dpx\n[Stivale2]\t\t\tPitch: 0x%x\n[Stivale2]\t\t\tBpp: %d\n", framebuffertag->framebuffer_addr, framebuffertag->framebuffer_width, framebuffertag->framebuffer_height, framebuffertag->framebuffer_pitch, framebuffertag->framebuffer_bpp);
                }
                break;
            case TAG_MODULES:
                {
                    stivale2_struct_tag_modules_t *modulestag = (stivale2_struct_tag_modules_t *)currTag;
                    printf("[Stivale2]\t\t\tModules count: %lx\n", modulestag->module_count);
                    for(size_t i = 0; i < modulestag->module_count; i++) {
                        stivale2_module_t *module = (stivale2_module_t *)&modulestag->modules[i];
                        printf("[Stivale2]\t\t\tModules string: %s\n", module->string);
                        printf("[Stivale2]\t\t\t\tStart: 0x%lx, End 0x%lx: %s\n", module->begin, module->end);
                        if(strcmp(module->string, "initrd") == 0) {
                            initrdModule = (void *)((uint64_t)module + VIRT_PHYS_BASE);
                        }
                    }
                }
                break;
            case TAG_EPOCH:
                {
                    stivale2_struct_tag_epoch_t *epochtag = (stivale2_struct_tag_epoch_t *)currTag;
                    printf("[Stivale2]\t\t\tUnix Epoch 0x%lx\n", epochtag->epoch);
                }
                break;
            case TAG_CMDLINE:
                {
                    stivale2_struct_tag_cmdline_t *cmdlinetag = (stivale2_struct_tag_cmdline_t *)currTag;
                    printf("[Stivale2]\t\t\tCmdline: %s\n", ((const char *)cmdlinetag->cmdline));
                }
                break;
            case TAG_RSDP:
                {
                    rsdptag = (stivale2_struct_tag_rsdp_t *)((uint64_t)currTag + VIRT_PHYS_BASE);
                    printf("[Stivale2]\t\t\tRSDP address: 0x%lx, TODO\n", rsdptag->rsdp);

                }
                break;
            case TAG_FIRMWARE:
                {
                    stivale2_struct_tag_firmware_t *firmwaretag = (stivale2_struct_tag_firmware_t *)currTag;
                    printf("[Stivale2]\t\t\tFirmware; running on %s compatible bootloader\n", firmwaretag->flags & 0x1 ? "BIOS" : "UEFI");
                }
                break;
            default:
                printf("[Stivale2]\t\t\tUnknown tag\n");
                break;
        }
        stivaleTags = currTag->next;
    }

    gdt_init();
    idt_init();
    pmm_init(stivale);
    vmm_init();

    acpi_init(rsdptag);

    vfs_init();

    //Test mounting
    vfs_node_t *rootNode = (vfs_node_t *)kcalloc(sizeof(vfs_node_t *), 1);
    strcpy(rootNode->name, "VFS");
    rootNode->functions.open = testOpen;
    rootNode->functions.read = testRead;
    vfs_mount("/dev/", rootNode);

    parseTarInitrd((void *)(initrdModule->begin + VIRT_PHYS_BASE));

    print_vfstree();

    vfs_node_t *node = kopen("/hola.txt", 0);
    char buffer[1000];
    vfs_read(node, buffer, 0, 999);
    buffer[999] = '\0';
    printf("%s\n", buffer);

    smp_init();

    asm volatile("sti");

    printf("WivOS Booted, halting\n");
    while(1) {
        //asm volatile("hlt");
    }
}
