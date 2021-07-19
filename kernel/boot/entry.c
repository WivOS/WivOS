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
#include <modules/elf.h>

#include <acpi/acpi.h>
#include <proc/smp.h>

#include <console/font.h>

//Test
#include "../../modules/virtiogpu/virtiogpu.h"

uint8_t g_bootstrap_stack[0x8000] = {0};

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

size_t elf_load(vfs_node_t *fd, void *pml4, uint64_t *entry) {
    size_t ret = vfs_lseek(fd, 0, SEEK_SET);
    if(ret == -1) return -1;

    elf64_header_t header;

    char *magic = "\177ELF";

    ret = vfs_read(fd, (char *)&header, sizeof(elf64_header_t));
    if(ret == -1) return -1;

    for(size_t i = 0; i < 4; i++) {
        if(header.e_ident[i] != magic[i]) return -1;
    }

    if(header.e_ident[EI_CLASS] != 0x02) return -1;
    if(header.e_ident[EI_DATA] != ELFDATA2LSB) return -1;
    if(header.e_ident[EI_OSABI] != ABI_SYSV) return -1;
    if(header.e_machine != EM_AMD64) return -1;

    ret = vfs_lseek(fd, header.e_phoff, SEEK_SET);
    if(ret == -1) return -1;

    elf64_phdr_t *phdr = kmalloc(header.e_phnum * sizeof(elf64_phdr_t));
    if(!phdr) return -1;

    ret = vfs_read(fd, (char *)phdr, header.e_phnum * sizeof(elf64_phdr_t));
    if(ret == -1) {
        kfree(phdr);
        return -1;
    }

    for(size_t i = 0; i < header.e_phnum; i++) {
        if(phdr[i].p_type != PT_LOAD)
            continue;

        size_t misalign = phdr[i].p_vaddr & (PAGE_SIZE - 1);
        size_t pageCount = (misalign + phdr[i].p_memsz + (PAGE_SIZE - 1)) / PAGE_SIZE;
        if(phdr[i].p_memsz % 0x1000) pageCount++;

        void *addr = pmm_alloc(pageCount);
        if(!addr) {
            kfree(phdr);
            return -1;
        }

        vmm_map_pages(pml4, (void *)phdr[i].p_vaddr, addr, pageCount, 0x7);

        void *buf = (void *)((size_t)addr + VIRT_PHYS_BASE);

        ret = vfs_lseek(fd, phdr[i].p_offset, SEEK_SET);
        if(ret == -1) {
            kfree(phdr);
            return -1;
        }

        ret = vfs_read(fd, (char *)(buf + misalign), phdr[i].p_filesz);
        if(ret == -1) {
            kfree(phdr);
            return -1;
        }
    }

    kfree(phdr);

    *entry = header.e_entry;
    return 0;
}

void kentry_threaded() {
    printf("Threading started :D\n");

    vfs_init();

    parseTarInitrd((void *)(initrdModule->begin + VIRT_PHYS_BASE));
    initDevFS();

    pci_init();

    modules_init();
    module_load("/test.wko");
    module_load("/test2.wko");
    module_load("/virtiogpu.wko");
    module_load("/sb16.wko");
    module_load("/pcnet.wko");

    print_vfstree();

    // Test the test program, redundant

    vfs_node_t *testProgram = kopen("/tests/test2", 0);

    void *pml4 = create_new_pml4();
    pid_t pid = proc_create(pml4);
    process_t *proc = get_active_process(pid);
    vfs_node_t *ttynode = kopen("/dev/tty0", 0);
    proc->fileHandles[0] = ttynode;
    proc->fileHandles[1] = ttynode;
    proc->fileHandles[2] = ttynode;

    uint64_t entry = 0;
    size_t returnVal = elf_load(testProgram, pml4, &entry);

    printf("Ret: %lx, Entry: %lx\n", returnVal, entry);

    thread_create(pid, (void *)entry);

    while(1);
}
