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

        size_t pageCount = phdr[i].p_memsz / 0x1000;
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

        ret = vfs_read(fd, (char *)buf, phdr[i].p_filesz);
        if(ret == -1) {
            kfree(phdr);
            return -1;
        }
    }

    kfree(phdr);

    *entry = header.e_entry;
    return 0;
}

typedef struct {
    uint8_t capLength;
    uint8_t reserved;
    uint16_t hciVersion;
    uint32_t hcsParameters1;
    uint32_t hcsParameters2;
    uint32_t hcsParameters3;
    uint32_t hccParameters1;
    uint32_t doorbellOffset;
    uint32_t rtsOffset;
    uint32_t hccParameters2;
} __attribute__((packed)) xhci_cap_regs_t;

typedef struct {
    uint32_t portSrc;
    uint32_t portPMSC;
    uint32_t portLinkInfo;
    uint32_t reserved;
} __attribute__((packed)) xhci_port_regs_t;

typedef struct {
    uint32_t usbCommand;
    uint32_t usbStatus;
    uint32_t pageSize;
    uint8_t rsvd1[0x14 - 0x0C];
    uint32_t dnControl;
    uint64_t cmdRingControl;
    uint8_t rsvd2[0x30 - 0x20];
    uint64_t DCBAAP;
    uint32_t config;
    uint8_t reserved2[964];
    volatile xhci_port_regs_t ports[256];
} __attribute__((packed)) xhci_op_regs_t;

typedef struct {
    uint32_t iman;
    uint32_t imod;
    uint32_t erstsz;
    uint32_t reserved;
    uint64_t erstba;
    uint64_t erdp;
} __attribute__((packed)) xhci_int_regs_t;

typedef struct {
    uint32_t mfIndex;
    uint8_t reserved[28];
    volatile xhci_int_regs_t irs[1024];
} __attribute__((packed)) xhci_run_regs_t;

typedef struct {
    uint32_t doorbell[256];
} __attribute__((packed)) xhci_doorbell_regs_t;

#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )

static volatile uint32_t currOpenglResource = 1;

void set_sub_ctx(vfs_node_t *gpuNode, uint32_t contextResourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_SUB_CTX;
    command.option = 0;
    command.length = 1;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = contextResourceID; // ctx id

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

uint32_t create_opengl_context(vfs_node_t *gpuNode) {
    virtgpu_3d_command_t command = {0};

    uint32_t contextResourceID = currOpenglResource++;

    command.command = VIRGL_CCMD_CREATE_SUB_CTX;
    command.option = 0;
    command.length = 1;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = contextResourceID; // ctx id

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);

    return contextResourceID;
}


//TODO: Check every type
uint32_t create_opengl_object(vfs_node_t *gpuNode, uint32_t objectType, uint32_t *parameters, size_t count, uint8_t bindIt) {
    virtgpu_3d_command_t command = {0};

    uint32_t objectResourceID = currOpenglResource++;

    command.command = VIRGL_CCMD_CREATE_OBJECT;
    command.option = objectType;
    command.length = (1 + count);
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * (1 + count));
    command.parameters[0] = objectResourceID; // Surface id
    for(size_t i = 0; i < count; i++) {
        command.parameters[1 + i] = parameters[i];
    }

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    if(!bindIt) {
        kfree(command.parameters);
        return objectResourceID;
    }

    command.command = VIRGL_CCMD_BIND_OBJECT;
    command.option = objectType;
    command.length = 1;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 1);
    command.parameters[0] = objectResourceID; // Ve id

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);

    return objectResourceID;
}

void set_framebuffer_state(vfs_node_t *gpuNode, uint32_t depthBuffer, uint32_t *surfaces, size_t count) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_FRAMEBUFFER_STATE;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = count;
    command.parameters[1] = depthBuffer;
    for(size_t i = 0; i < count; i++) {
        command.parameters[2 + i] = surfaces[i];
    }

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

void clearResource(vfs_node_t *gpuNode, float red, float green, float blue, float alpha, double depth, uint32_t stencil, uint32_t clearFlags) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_CLEAR;
    command.option = 0;
    command.length = 8;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 8);
    command.parameters[0] = clearFlags; //(1 << 2);
    *((float *)&command.parameters[1]) = red;
    *((float *)&command.parameters[2]) = green;
    *((float *)&command.parameters[3]) = blue;
    *((float *)&command.parameters[4]) = alpha;
    *((double *)&command.parameters[5]) = depth;
    command.parameters[7] = stencil;

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

uint32_t create_simple_buffer(vfs_node_t *gpuNode, uint32_t width, uint32_t bind, uint32_t **buffer) {
    virtgpu_create_resource_3d_t createResource3D = {0};
    createResource3D.target = PIPE_BUFFER;
    createResource3D.format = 64;
    createResource3D.width = width;
    createResource3D.height = 1;
    createResource3D.depth = 1;
    createResource3D.arraySize = 1;
    createResource3D.lastLevel = 0;
    createResource3D.nrSamples = 0;
    createResource3D.bind = bind;
    createResource3D.flags = 0;

    uint32_t resource3D = vfs_ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_3D, &createResource3D);
    
    virtgpu_attach_resource_3d_t attachResource3D = {0};
    attachResource3D.resourceID = resource3D;
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ATTACH_RESOURCE_3D, &attachResource3D);

    //Later we will store this in an array, for now omit it
    uint32_t vboFbSize = createResource3D.width * createResource3D.height;
    uint32_t *vboFb = (uint32_t *)kmalloc(vboFbSize);
    if(buffer)
        *buffer = vboFb;

    virtgpu_attach_backing_t attachBackingData = {0};
    attachBackingData.address = (uint64_t)vboFb;
    attachBackingData.length = vboFbSize;

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resource3D);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);

    return resource3D;
}

void send_inline_write(vfs_node_t *gpuNode, uint32_t resourceID, uint32_t level, uint32_t usage, virtio_box_t box, const void *data, uint32_t stride, uint32_t layerStride) {
    virtgpu_3d_command_t command = {0};

    uint32_t length;
    uint32_t size;
    uint32_t layerSize;
    uint32_t strideInternal = stride;
    uint32_t layerStrideInternal = layerStride;

    if(!stride) strideInternal = box.width * 1; // Asume format of 1 byte
    layerSize = box.height * strideInternal;
    if(layerStride && layerStride < layerSize) return;
    if(!layerStride) layerStrideInternal = layerSize;
    size = layerStrideInternal * box.depth;

    //TODO: what happens when not enough space

    command.command = VIRGL_CCMD_RESOURCE_INLINE_WRITE;
    command.option = 0;
    command.length = ((size + 3) / 4) + 11;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * (((size + 3) / 4) + 11));
    command.parameters[0] = resourceID; // res id
    command.parameters[1] = level; // level
    command.parameters[2] = usage; // Usage
    command.parameters[3] = stride; // Stride
    command.parameters[4] = layerStride; // Layer Stride
    command.parameters[5] = box.x; // x
    command.parameters[6] = box.y; // y
    command.parameters[7] = box.z; // z
    command.parameters[8] = box.width; // width
    command.parameters[9] = box.height; // height
    command.parameters[10] = box.depth; // depth
    for(size_t i = 0; i < size; i++) {
        command.parameters[11 + i] = ((uint32_t *)(data))[i];
    }

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

void set_vertex_buffers(vfs_node_t *gpuNode, uint32_t stride, uint32_t bufferOffset, uint32_t resourceID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_VERTEX_BUFFERS;
    command.option = 0;
    command.length = 3;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 3);
    command.parameters[0] = stride; // Stride
    command.parameters[1] = bufferOffset; // Buffer Offset
    command.parameters[2] = resourceID; // Ve id

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

void set_streamout_targets(vfs_node_t *gpuNode, uint32_t bitmask, uint32_t streamoutID) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_STREAMOUT_TARGETS;
    command.option = 0;
    command.length = 2;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 2);
    command.parameters[0] = bitmask; // Bitmask
    command.parameters[1] = streamoutID; // SO id

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

uint32_t create_shader(vfs_node_t *gpuNode, const char *tgsiAssembly, uint32_t shaderType, uint32_t bindIt) {
    uint32_t shader_len = strlen(tgsiAssembly) + 1;

    uint32_t *parameters = (uint32_t *)kmalloc(sizeof(uint32_t) * (4 + ((shader_len + 3) / 4)));
    parameters[0] = shaderType; // Type, 0 -> vertex, 1 -> fragment
    parameters[1] = (((shader_len) & 0x7fffffff) << 0); // First instruction ???
    parameters[2] = 300; // Number of tokens
    parameters[3] = 0; // Stream so count
    memcpy((void *)&parameters[4], tgsiAssembly, shader_len);
    if(shader_len % 4) {
        parameters[4 + shader_len] = '\0';
    }

    uint32_t shaderID = create_opengl_object(gpuNode, VIRGL_OBJECT_SHADER, parameters, (4 + ((shader_len + 3) / 4)), 0);

    kfree(parameters);


    if(!bindIt) return shaderID;

    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_BIND_SHADER;
    command.option = 0;
    command.length = 2;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 2);
    command.parameters[0] = shaderID; // Ve id
    command.parameters[1] = shaderType; // Type, 0 -> vertex, 1 -> fragment

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);

    return shaderID;
}

void set_viewport_state(vfs_node_t *gpuNode, uint32_t offset, float stateA, float stateB, float stateC, float translateA, float translateB, float translateC) {
    virtgpu_3d_command_t command = {0};

    command.command = VIRGL_CCMD_SET_VIEWPORT_STATE;
    command.option = 0;
    command.length = 7;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 7);
    command.parameters[0] = offset; // Offset
    *((float *)&command.parameters[1]) = stateA;
    *((float *)&command.parameters[2]) = stateB;
    *((float *)&command.parameters[3]) = stateC;
    *((float *)&command.parameters[4]) = translateA;
    *((float *)&command.parameters[5]) = translateB;
    *((float *)&command.parameters[6]) = translateC;

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    kfree(command.parameters);
}

struct vertex {
   float position[4];
   float color[4];
};

static struct vertex vertices[6] =
{
   {
      { -0.9f, -0.9f, -2.1f, 1.0f },
      { 1.0f, 0.0f, 0.0f, 1.0f }
   },
   {
      { -0.9f, 0.9f, -2.1f, 1.0f },
      { 0.0f, 1.0f, 0.0f, 1.0f }
   },
   {
      { 0.9f, 0.9f, -2.1f, 1.0f },
      { 0.0f, 0.0f, 1.0f, 1.0f }
   },
   {
      { -0.9f, -0.9f, -2.1f, 1.0f },
      { 1.0f, 0.0f, 0.0f, 1.0f }
   },
   {
      { 0.9f, 0.9f, -2.1f, 1.0f },
      { 0.0f, 0.0f, 1.0f, 1.0f }
   },
   {
      { 0.9f, -0.9f, -2.1f, 1.0f },
      { 1.0f, 0.0f, 0.0f, 1.0f }
   },
};

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

    print_vfstree();

    /*pci_device_t *xhciDevice = pci_get_device_by_class_subclass(0x0C, 0x03, 0x30, 0);
    if(xhciDevice) {
        printf("Yeah\n");

        pci_enable_busmastering(xhciDevice);

        pci_bar_t bar = {0};
        pci_read_bar(xhciDevice, 0, &bar); // Read the bar 0

        printf("0x%lx:0x%lx mmio: %x, prefetch: %x\n", bar.base, bar.size, bar.is_mmio, bar.is_prefetchable);
    
        xhci_cap_regs_t *capRegisters = (xhci_cap_regs_t *)(bar.base + VIRT_PHYS_BASE);
        xhci_op_regs_t *opRegisters = (xhci_op_regs_t *)(bar.base + capRegisters->capLength + VIRT_PHYS_BASE);
        xhci_run_regs_t *runRegisters = (xhci_run_regs_t *)(bar.base + capRegisters->rtsOffset + VIRT_PHYS_BASE);
        xhci_doorbell_regs_t *doorbellRegisters = (xhci_doorbell_regs_t *)(bar.base + capRegisters->doorbellOffset + VIRT_PHYS_BASE);

        uint32_t cparams = capRegisters->hccParameters1;

        size_t eoff = (((cparams & 0xFFFF0000) >> 16) * 4);
        volatile uint32_t *extcap = (uint32_t *)((size_t)capRegisters + eoff);
        if(!extcap) {
            while(1);
        }

        while(1) {
            uint32_t val = *extcap;
            if(val == 0xFFFFFFFF) break;
            if(!(val & 0xFF)) break;

            printf("[XHCI] found extcap: %X %X\n", val & 0xFF, extcap);
            if ((val & 0xff) == 1) {
                volatile uint8_t *bios_sem = (uint8_t *)((size_t)(extcap) + 0x2);
                if (*bios_sem) {
                    printf("[XHCI] device is bios-owned\n");
                    volatile uint8_t *os_sem = (uint8_t *)((size_t)(extcap) + 0x3);
                    *os_sem = 1;
                    while (1) {
                        bios_sem = (uint8_t *)((size_t)(extcap) + 0x2);
                        if (*bios_sem == 0) {
                            printf("[XHCI] device is no longer bios-owned\n");
                            break;
                        }
                        ksleep(100);
                    }
                }
            }

            uint32_t *old = (uint32_t *)extcap;
            extcap = (uint32_t *)((size_t)extcap + (((val >> 8) & 0xFF) << 2));
            if (old == extcap) {
                break;
            }
        }

        //TODO, switch to xhci ports

        printf("Version: %x\n", opRegisters->pageSize);
    }*/

    /*vfs_node_t *gpuNode = kopen("/dev/gpu", 0);

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

    virtgpu_create_gpu_context_t createContext = {0};
    createContext.name = strdup("Test Context");
    createContext.nameLength = strlen("Test Context");

    uint32_t contextID = vfs_ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_GPU_CONTEXT, &createContext);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SET_CONTEXT_ID, &contextID);

    virtgpu_3d_command_t command = {0};
    char *flag = "all=1";
    uint32_t slen = strlen(flag) + 1;
    uint32_t sslen = (slen + 3) & ~3;
    uint32_t string_length = (uint32_t)MIN2(sslen, slen);

    command.command = VIRGL_CCMD_SET_DEBUG_FLAGS;
    command.option = 0;
    command.length = sslen / 4;
    command.parameters = (uint32_t *)krealloc(command.parameters, sslen);
    memcpy(command.parameters, flag, string_length);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);


    uint32_t *parameters = NULL;


    uint32_t ctxID = create_opengl_context(gpuNode);
    set_sub_ctx(gpuNode, ctxID);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);


    //Atach the screen resource to the context
    virtgpu_attach_resource_3d_t attachResource3D = {0};
    attachResource3D.resourceID = resourceID;
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ATTACH_RESOURCE_3D, &attachResource3D);

    parameters = (uint32_t *)krealloc(parameters, sizeof(uint32_t) * 4);
    parameters[0] = resourceID; // Resource ID
    parameters[1] = 1; // Format
    parameters[2] = 0; // Format specific 1
    parameters[3] = 0; // Format specific 2
    uint32_t surfaceID = create_opengl_object(gpuNode, VIRGL_OBJECT_SURFACE, parameters, 4, 0);

    set_framebuffer_state(gpuNode, 0, &surfaceID, 1);

    clearResource(gpuNode, 0.0f, 1.0f, 0.0f, 1.0f, 0.0, 0, PIPE_CLEAR_COLOR0);


    parameters = (uint32_t *)krealloc(parameters, sizeof(uint32_t) * 8);
    parameters[0 * 4 + 0] = offsetof(struct vertex, position); // src offset
    parameters[0 * 4 + 1] = 0; // instance divisor
    parameters[0 * 4 + 2] = 0; // vertex buffer index
    parameters[0 * 4 + 3] = 31; // src format

    parameters[1 * 4 + 0] = offsetof(struct vertex, color); // src offset
    parameters[1 * 4 + 1] = 0; // instance divisor
    parameters[1 * 4 + 2] = 0; // vertex buffer index
    parameters[1 * 4 + 3] = 31; // src format
    uint32_t vertexElementsID = create_opengl_object(gpuNode, VIRGL_OBJECT_VERTEX_ELEMENTS, parameters, 8, 1);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    uint32_t resource3D = create_simple_buffer(gpuNode, sizeof(vertices), PIPE_BIND_VERTEX_BUFFER, NULL);

    send_inline_write(gpuNode, resource3D, 0, 0, (virtio_box_t){
        0, 0, 0,
        sizeof(vertices), 1, 1
    }, &vertices, sizeof(vertices), 0);

    set_vertex_buffers(gpuNode, sizeof(struct vertex), 0, resource3D);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);


    uint32_t xfbID = create_simple_buffer(gpuNode, 6 * sizeof(vertices), PIPE_BIND_STREAM_OUTPUT, NULL);

    parameters = (uint32_t *)krealloc(parameters, sizeof(uint32_t) * 3);
    parameters[0] = xfbID; // Resource ID
    parameters[1] = 0; // Buffer offset
    parameters[2] = 3 * sizeof(vertices);
    uint32_t streamoutID = create_opengl_object(gpuNode, VIRGL_OBJECT_STREAMOUT_TARGET, parameters, 3, 0);

    set_streamout_targets(gpuNode, 0, streamoutID);


    const char *geometryShader =
	   "GEOM\n"
	   "PROPERTY GS_INPUT_PRIMITIVE TRIANGLES\n"
	   "PROPERTY GS_OUTPUT_PRIMITIVE TRIANGLE_STRIP\n"
	   "PROPERTY GS_MAX_OUTPUT_VERTICES 3\n"
	   "PROPERTY GS_INVOCATIONS 1\n"
	   "DCL IN[][0], POSITION\n"
	   "DCL IN[][1], GENERIC[20]\n"
	   "DCL OUT[0], POSITION\n"
	   "DCL OUT[1], GENERIC[20]\n"
	   "IMM[0] INT32 {0, 0, 0, 0}\n"
	   "0:MOV OUT[0], IN[0][0]\n"
	   "1:MOV OUT[1], IN[0][1]\n"
	   "2:EMIT IMM[0].xxxx\n"
	   "3:MOV OUT[0], IN[1][0]\n"
	   "4:MOV OUT[1], IN[0][1]\n"
	   "5:EMIT IMM[0].xxxx\n"
	   "6:MOV OUT[0], IN[2][0]\n"
	   "7:MOV OUT[1], IN[2][1]\n"
	   "8:EMIT IMM[0].xxxx\n"
	   "9:END\n";

    const char *vertexString =
	   "VERT\n"
	   "DCL IN[0]\n"
	   "DCL IN[1]\n"
	   "DCL OUT[0], POSITION\n"
	   "DCL OUT[1], COLOR\n"
       "DCL CONST[1][0..12]\n"
       "DCL TEMP[0..3], ARRAY(1), LOCAL\n"
       "DCL TEMP[4..7], ARRAY(2), LOCAL\n"
       "DCL TEMP[8..20], LOCAL\n"
       "IMM[0] UINT32 {0, 128, 144, 160}\n"
       "IMM[1] UINT32 {176, 192, 112, 96}\n"
       "IMM[2] FLT32 {0xbc23d70a, 0x00000000, 0x00000000, 0x00000000}\n"
       "IMM[3] UINT32 {80, 64, 16, 48}\n"
       "  0: MOV TEMP[0], CONST[1][8]\n"
       "  1: MOV TEMP[1], CONST[1][9]\n"
       "  2: MOV TEMP[2], CONST[1][10]\n"
       "  3: MOV TEMP[3], CONST[1][11]\n"
       "  4: MOV TEMP[4], TEMP[0]\n"
       "  5: MOV TEMP[5], TEMP[1]\n"
       "  6: MOV TEMP[6], TEMP[2]\n"
       "  7: MOV TEMP[7], TEMP[3]\n"
       "  8: U2F TEMP[8].x, CONST[1][12].xxxx\n"
       "  9: MUL TEMP[9].x, TEMP[8].xxxx, IMM[2].xxxx\n"
       "  10: MOV TEMP[6].w, TEMP[9].xxxx\n"
       "  11: MUL TEMP[10], TEMP[4], IN[0].xxxx\n"
       "  12: MAD TEMP[11], TEMP[5], IN[0].yyyy, TEMP[10]\n"
       "  13: MAD TEMP[12], TEMP[6], IN[0].zzzz, TEMP[11]\n"
       "  14: MAD TEMP[13], TEMP[7], IN[0].wwww, TEMP[12]\n"
       "  15: MUL TEMP[14], CONST[1][4], TEMP[13].xxxx\n"
       "  16: MAD TEMP[15], CONST[1][5], TEMP[13].yyyy, TEMP[14]\n"
       "  17: MAD TEMP[16], CONST[1][6], TEMP[13].zzzz, TEMP[15]\n"
       "  18: MAD TEMP[17], CONST[1][7], TEMP[13].wwww, TEMP[16]\n"
       "  19: ADD TEMP[18], IN[1], CONST[1][1]\n"
       "  20: RCP TEMP[19], CONST[1][3].xxxx\n"
       "  21: MUL TEMP[20], TEMP[18], TEMP[19]\n"
       "  22: MOV OUT[0], TEMP[17]\n"
       "  23: MOV OUT[1], TEMP[20]\n"
       "  24: END\n";

    const char *fragmentString =
	   "FRAG\n"
	    "DCL IN[0], COLOR, LINEAR\n"
	    "DCL OUT[0], COLOR\n"
	    "  0: MOV OUT[0], IN[0]\n"
	    "  1: END\n";

    uint32_t vertShaderID = create_shader(gpuNode, vertexString, 0, 1);
    uint32_t fragShaderID = create_shader(gpuNode, fragmentString, 1, 1);
    //uint32_t geomShaderID = create_shader(gpuNode, geometryShader, 2, 1);

    float constants[] = 
    {  0.4, 0, 0,  1,
        1,   1, 1,  1,
        2,   2, 2,  2,
        4,   8, 16, 32,

        1,  0, 0, 0,
        0, (1024.0f / 768.0f), 0, 0,
        0,  0, -((1000.1f) / (999.9f)), -1,
        0,  0, -((200.0f) / (999.9f)), 0,

        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
        0,
    };

    uint32_t resourceUniform = create_simple_buffer(gpuNode, sizeof(constants), PIPE_BIND_CONSTANT_BUFFER, NULL);

    send_inline_write(gpuNode, resourceUniform, 0, (1 << 1), (virtio_box_t){
        0, 0, 0,
        sizeof(constants), 1, 1
    }, &constants, sizeof(constants), 0);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    command.command = VIRGL_CCMD_SET_UNIFORM_BUFFER;
    command.option = 0;
    command.length = 5;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 5);
    command.parameters[0] = 0;
    command.parameters[1] = 1; // Index
    command.parameters[2] = 0; // Offset
    command.parameters[3] = sizeof(constants); // Length
    command.parameters[4] = resourceUniform;

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    parameters = (uint32_t *)krealloc(parameters, sizeof(uint32_t) * 10);
    parameters[0] = 0;
    parameters[1] = 0; // format
    parameters[2] = (0xF << 27);
    parameters[3] = 0;
    parameters[4] = 0;
    parameters[5] = 0;
    parameters[6] = 0;
    parameters[7] = 0;
    parameters[8] = 0;
    parameters[9] = 0;
    uint32_t blendID = create_opengl_object(gpuNode, VIRGL_OBJECT_BLEND, parameters, 10, 1);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    parameters = (uint32_t *)krealloc(parameters, sizeof(uint32_t) * 4);
    parameters[0] = (1 << 2) | (1 << 1); // Bitfield1
    parameters[1] = 0; // Bitfield2
    parameters[2] = 0; // Bitfield3
    *((float *)&parameters[4]) = 0.0f; // Alpha state
    uint32_t dsaID = create_opengl_object(gpuNode, VIRGL_OBJECT_DSA, parameters, 4, 1);

    parameters = (uint32_t *)krealloc(parameters, sizeof(uint32_t) * 8);
    command.parameters[0] = (1 << 30) | (1 << 29) | (1 << 1); // Bitfield1
    *((float *)&command.parameters[1]) = 0.0f; // Point size
    command.parameters[2] = 0; // Sprit coord enabled
    command.parameters[3] = 0; // Bitfield2
    *((float *)&command.parameters[4]) = 0.0f; // Line width
    *((float *)&command.parameters[5]) = 0.0f; // Offset units
    *((float *)&command.parameters[6]) = 0.0f; // Offset scale
    *((float *)&command.parameters[7]) = 0.0f; // Offset clamp
    *((float *)&parameters[8]) = 0.0f; // Alpha state
    uint32_t rasterizerID = create_opengl_object(gpuNode, VIRGL_OBJECT_RASTERIZER, parameters, 8, 1);

    float znear = 0.1f, zfar = 1000.0f;
    float half_w = 1024.0f / 2.0f;
	float half_h = 768.0f / 2.0f;
	float half_d = (zfar - znear) / 2.0f;

    set_viewport_state(gpuNode, 0, half_w, half_h, half_d, half_w, half_h, half_d + znear);


    command.command = VIRGL_CCMD_DRAW_VBO;
    command.option = 0;
    command.length = 12;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 12);
    command.parameters[0] = 0; // Start
    command.parameters[1] = 6; // Count
    command.parameters[2] = 4; // Mode PIPE_PRIM_TRIANGLES
    command.parameters[3] = 0; // Indexed
    command.parameters[4] = 0; // Instace Count
    command.parameters[5] = 0; // Index Bias
    command.parameters[6] = 0; // Start Instance
    command.parameters[7] = 0; // Primitive Restart
    command.parameters[8] = 0; // Restart Index
    command.parameters[9] = 0; // Min Index
    command.parameters[10] = 0; // Max Index
    command.parameters[11] = 0; // Cso

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

    ksleep(10);

    transferAndFlushData.notTransfer = 1;
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);
    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);*/




    /*command.command = VIRGL_CCMD_CREATE_OBJECT;
    command.option = VIRGL_OBJECT_SURFACE;
    command.length = 5;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 5);
    command.parameters[0] = 0x6; // Surface id
    command.parameters[1] = resourceID;
    command.parameters[2] = 1; // format
    command.parameters[3] = 0;
    command.parameters[4] = 0;

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);*/

    /*
    command.command = VIRGL_CCMD_RESOURCE_INLINE_WRITE;
    command.option = 0;
    command.length = (((sizeof(vertices) * 1 * 1) + 3) / 4) + 11;
    command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * ((((sizeof(vertices) * 1 * 1) + 3) / 4) + 11));
    command.parameters[0] = resource3D; // res id
    command.parameters[1] = 0; // level
    command.parameters[2] = 0; // Usage
    command.parameters[3] = sizeof(vertices); // Stride
    command.parameters[4] = 0; // Layer Stride
    command.parameters[5] = 0; // x
    command.parameters[6] = 0; // y
    command.parameters[7] = 0; // z
    command.parameters[8] = sizeof(vertices); // width
    command.parameters[9] = 1; // height
    command.parameters[10] = 1; // depth
    memcpy((void *)&command.parameters[11], (void *)&vertices, sizeof(vertices));

    vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);
    */

    // Test the test program, redundant

    vfs_node_t *testProgram = kopen("/tests/test", 0);

    void *pml4 = create_new_pml4();
    pid_t pid = proc_create(pml4);

    uint64_t entry = 0;
    size_t returnVal = elf_load(testProgram, pml4, &entry);

    printf("Ret: %lx, Entry: %lx\n", returnVal, entry);

    thread_create(pid, (void *)entry);

    /*size_t i2 = 0;
    while(1) {
        clearResource(gpuNode, 0.0f, 1.0f, 0.0f, 1.0f, 0.0, 0, PIPE_CLEAR_COLOR0);
        float constants2[] = 
        {  0.4, 0, 0,  1,
            1,   1, 1,  1,
            2,   2, 2,  2,
            4,   8, 16, 32,

            1,  0, 0, 0,
            0, (1024.0f / 768.0f), 0, 0,
            0,  0, -((1000.1f) / (999.9f)), -1,
            0,  0, -((200.0f) / (999.9f)), 0,

            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,

            0
        };
        *((uint32_t *)&constants2[48]) = i2;

        send_inline_write(gpuNode, resourceUniform, 0, (1 << 1), (virtio_box_t){
            0, 0, 0,
            sizeof(constants2), 1, 1
        }, &constants2, sizeof(constants2), 0);

        vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

        command.command = VIRGL_CCMD_SET_UNIFORM_BUFFER;
        command.option = 0;
        command.length = 5;
        command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 5);
        command.parameters[0] = 0;
        command.parameters[1] = 1; // Index
        command.parameters[2] = 0; // Offset
        command.parameters[3] = sizeof(constants2); // Length
        command.parameters[4] = resourceUniform;

        vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

        command.command = VIRGL_CCMD_DRAW_VBO;
        command.option = 0;
        command.length = 12;
        command.parameters = (uint32_t *)krealloc(command.parameters, sizeof(uint32_t) * 12);
        command.parameters[0] = 0; // Start
        command.parameters[1] = 6; // Count
        command.parameters[2] = 4; // Mode PIPE_PRIM_TRIANGLES
        command.parameters[3] = 0; // Indexed
        command.parameters[4] = 0; // Instace Count
        command.parameters[5] = 0; // Index Bias
        command.parameters[6] = 0; // Start Instance
        command.parameters[7] = 0; // Primitive Restart
        command.parameters[8] = 0; // Restart Index
        command.parameters[9] = 0; // Min Index
        command.parameters[10] = 0; // Max Index
        command.parameters[11] = 0; // Cso

        vfs_ioctl(gpuNode, VIRTGPU_IOCTL_ADD_3D_COMMAND_TO_QUEUE, &command);

        vfs_ioctl(gpuNode, VIRTGPU_IOCTL_SUBMIT_3D_COMMAND_QUEUE, NULL);

        vfs_ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);
        ksleep(50);
        i2 += 20;
    }*/
    while(1);
}
