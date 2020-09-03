#ifndef WIVOS_STIVALE2
#define WIVOS_STIVALE2

#include <stdint.h>

typedef struct stivale2_header {
    uint64_t entry_point;
    uint64_t stack;
    uint64_t flags;
    uint64_t tags;
} __attribute__((packed)) stivale2_header_t;

typedef struct stivale2_struct {
    char bootloader_brand[64];    // Bootloader null-terminated brand string
    char bootloader_version[64];  // Bootloader null-terminated version string

    uint64_t tags;          // Pointer to the first of the linked list of tags.
                            // see "stivale2 structure tags" section.
                            // NULL = no tags.
} __attribute__((packed)) stivale2_struct_t;

typedef struct stivale2_hdr_tag {
    uint64_t identifier;
    uint64_t next;
} __attribute__((packed)) stivale2_hdr_tag_t;

typedef struct stivale2_hdr_tag_framebuffer {
    uint64_t identifier;          // Identifier: 0x3ecc1bc43d0f7971
    uint64_t next;
    uint16_t framebuffer_width;   // If all values are set to 0
    uint16_t framebuffer_height;  // then the bootloader will pick the best possible
    uint16_t framebuffer_bpp;     // video mode automatically.
} __attribute__((packed)) stivale2_hdr_tag_framebuffer_t;

#define TAG_CMDLINE         0xe5e76a1b4597a781
typedef struct stivale2_struct_tag_cmdline {
    uint64_t identifier;          // Identifier: 0xe5e76a1b4597a781
    uint64_t next;
    uint64_t cmdline;             // Pointer to a null-terminated cmdline
} __attribute__((packed)) stivale2_struct_tag_cmdline_t;

enum stivale2_mmap_type {
    USABLE                 = 1,
    RESERVED               = 2,
    ACPI_RECLAIMABLE       = 3,
    ACPI_NVS               = 4,
    BAD_MEMORY             = 5,
    BOOTLOADER_RECLAIMABLE = 0x1000,
    KERNEL_AND_MODULES     = 0x1001
};

typedef struct stivale2_mmap_entry {
    uint64_t base;      // Base of the memory section
    uint64_t length;    // Length of the section
    uint32_t type;      // Type
    uint32_t unused;
} __attribute__((packed)) stivale2_mmap_entry_t;

#define TAG_MEMMAP          0x2187f79e8612de07
typedef struct stivale2_struct_tag_memmap {
    uint64_t identifier;          // Identifier: 0x2187f79e8612de07
    uint64_t next;
    uint64_t entries;             // Count of memory map entries
    struct stivale2_mmap_entry memmap[];  // Array of memory map entries
} __attribute__((packed)) stivale2_struct_tag_memmap_t;

#define TAG_FRAMEBUFFER     0x506461d2950408fa
typedef struct stivale2_struct_tag_framebuffer {
    uint64_t identifier;          // Identifier: 0x506461d2950408fa
    uint64_t next;
    uint64_t framebuffer_addr;    // Address of the framebuffer and related info
    uint16_t framebuffer_width;
    uint16_t framebuffer_height;
    uint16_t framebuffer_pitch;
    uint16_t framebuffer_bpp;
} __attribute__((packed)) stivale2_struct_tag_framebuffer_t;

typedef struct stivale2_module {
    uint64_t begin;         // Address where the module is loaded
    uint64_t end;           // End address of the module
    char string[128];       // 0-terminated string passed to the module
} __attribute__((packed)) stivale2_module_t;

#define TAG_MODULES         0x4b6fe466aade04ce
typedef struct stivale2_struct_tag_modules {
    uint64_t identifier;          // Identifier: 0x4b6fe466aade04ce
    uint64_t next;
    uint64_t module_count;        // Count of loaded modules
    struct stivale2_module modules[]; // Array of module descriptors
} __attribute__((packed)) stivale2_struct_tag_modules_t;

#define TAG_RSDP            0x9e1786930a375e78
typedef struct stivale2_struct_tag_rsdp {
    uint64_t identifier;        // Identifier: 0x9e1786930a375e78
    uint64_t next;
    uint64_t rsdp;              // Pointer to the ACPI RSDP structure
} __attribute__((packed)) stivale2_struct_tag_rsdp_t;

#define TAG_EPOCH           0x566a7bed888e1407
typedef struct stivale2_struct_tag_epoch {
    uint64_t identifier;        // Identifier: 0x566a7bed888e1407
    uint64_t next;
    uint64_t epoch;             // UNIX epoch at boot, read from system RTC
} __attribute__((packed)) stivale2_struct_tag_epoch_t;

#define TAG_FIRMWARE        0x359d837855e3858c
typedef struct stivale2_struct_tag_firmware {
    uint64_t identifier;        // Identifier: 0x359d837855e3858c
    uint64_t next;
    uint64_t flags;             // Bit 0: 0 = UEFI, 1 = BIOS
} __attribute__((packed)) stivale2_struct_tag_firmware_t;

//TODO: more tags

#endif