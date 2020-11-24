#include "stivale2.h"
#include <util/util.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <mem/mm.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <fs/vfs.h>
#include <fs/initrd/tar.h>
#include <fs/devfs/devfs.h>
#include <cpu/pci.h>
#include <modules/modules.h>

#include <acpi/acpi.h>
#include <proc/smp.h>

#include <console/font.h>

//Test
#include "../../modules/virtiogpu/virtiogpu.h"

uint8_t g_bootstrap_stack[0x5000] = {0};

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
    .tags = (uint64_t)NULL//&g_header_tag_framebuffer
};

void kentry_threaded();

static stivale2_module_t *initrdModule = NULL;

void kentry(stivale2_struct_t *stivale) {
    printf("Initializing WivOS\n");

    printf("[Stivale2] Showing structs:\n");
    printf("[Stivale2]\tBootloader brand: %s\n", stivale->bootloader_brand);
    printf("[Stivale2]\tBootloader version: %s\n", stivale->bootloader_version);
    
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

    smp_init();

    scheduler_init();
    thread_create(0, kentry_threaded);

    printf("WivOS Booted, scheduling\n");

    asm volatile("sti");
    while(1) {
        //asm volatile("hlt");
    }
}

char * special_thing = "Hola :D\n";

static void print(char *str) {
    //printf("%lx %lx\n", (size_t)str, (size_t)special_thing);
    printf(str);
}

static void render_char(uint32_t *framebuffer, uint32_t pitch, uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    uint32_t *line = framebuffer + y * 16 * pitch + x * 8;
    
    for(size_t i = 0; i < 16; i++) {
        uint32_t *dest = line;
        uint8_t dc = (c >= 32 && c <= 127) ? c : 127;
        uint8_t fontbits = fontBitmap[(dc - 32) * 16 + i];
        for(size_t j = 0; j < 8; j++) {
            int bit = (1 << ((8 - 1) - j));
            *dest++ = (fontbits & bit) ? fg : bg;
        }
        line += pitch;
    }

    asm volatile ("" : : : "memory");
}

void render_string(uint32_t *framebuffer, uint32_t x, uint32_t y, const char *c, uint32_t fg, uint32_t bg) {
    size_t count = strlen(c);
    size_t currX = x;
    for(size_t i = 0; i < count; i++) {
        if(c[i] == '\n') {
            y++;
            x = 0;
            currX = 0;
            continue;
        }
        if(x + currX >= (1024 / 8)) {
            y++;
            x = 0;
            currX = 0;
        }
        render_char(framebuffer, 1024, currX, y, c[i], fg, bg);
        currX++;
    }
}

extern void test_function();
extern void end_test_function();

void kentry_threaded() {
    printf("Threading started :D\n");

    vfs_init();

    parseTarInitrd((void *)(initrdModule->begin + VIRT_PHYS_BASE));
    initDevFS();

    kopen("/dev/asa", 0);

    pci_init();

    modules_init();
    module_load("/test.wko");
    module_load("/test2.wko");
    module_load("/virtiogpu.wko");

    print_vfstree();

    vfs_node_t *gpuNode = kopen("/dev/gpu", 0);
    uint64_t framebufferAddr;
    /*vfs_read(gpuNode, (char *)&framebufferAddr, 8);
    uint32_t *framebuffer = (uint32_t *)(framebufferAddr);

    for(size_t i = 0; i < 768; i++) {
        for(size_t j = 0; j < 1024; j++) {
            uint32_t *pixel = &framebuffer[j + i * 1024];
            *pixel = 0xFF000000 | ((i * j) & 0x0FF0) << (4 + 8) | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
        }
    }

    vfs_write(gpuNode, NULL, 1);*/

    virtio_mode_t *virtioModes = kmalloc(sizeof(virtio_mode_t) * 16);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_GET_DISPLAY_INFO, (void *)virtioModes);

    virtgpu_create_resource_2d_t resourceData = {0};
    virtgpu_attach_backing_t attachBackingData = {0};
    virtgpu_set_scanout_t scanoutData = {0};
    virtgpu_transfer_and_flush_t transferAndFlushData = {0};

    resourceData.format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;

    for(size_t i = 0; i < 16; i++) {
        if(virtioModes[i].enabled) {
            printf("%u: x %u, y %u, w %u, h %u, flags 0x%x\n", i, virtioModes[i].rect.x, virtioModes[i].rect.y, virtioModes[i].rect.width, virtioModes[i].rect.height, virtioModes[i].flags);
            resourceData.width = scanoutData.width = transferAndFlushData.width = virtioModes[i].rect.width;
            resourceData.height = scanoutData.height = transferAndFlushData.height = virtioModes[i].rect.height;
            scanoutData.scanoutID = i;
        }
    }

    uint32_t resourceID = (uint32_t)vfs_ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_2D, (void *)&resourceData);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);

    uint32_t framebufferSize = resourceData.width * resourceData.height * 4;
    uint32_t *framebuffer = (uint32_t *)kmalloc(framebufferSize);

    attachBackingData.address = (uint64_t)framebuffer;
    attachBackingData.length = framebufferSize;

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SET_SCANOUT, (void *)&scanoutData);

    for(size_t i = 0; i < 768; i++) {
        for(size_t j = 0; j < 1024; j++) {
            uint32_t *pixel = &framebuffer[j + i * 1024];
            *pixel = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
        }
    }

    render_string(framebuffer, 0, 0, "Hola que tal?\n Yo bien y tu?\n\nPues tambien bien", 0xFFFFFFFF, 0x0);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);

    void *pml4 = create_new_pml4();
    pid_t pid = proc_create(pml4);

    void *codeStart = pmm_alloc(1);
    vmm_map_pages(pml4, (void *)(0x400000000), codeStart, 1, 0x7);
    void *codeStartKernel = (void *)((uint64_t)codeStart + VIRT_PHYS_BASE);
    memcpy(codeStartKernel, (const void *)test_function, ((uint64_t)end_test_function - (uint64_t)test_function));
    thread_create(pid, (void *)0x400000000);

    while(1);
}
