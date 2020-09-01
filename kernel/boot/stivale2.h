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

//TODO: more tags

#endif