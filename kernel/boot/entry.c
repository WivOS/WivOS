#include "stivale2.h"
#include <stddef.h>

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

extern void debug_out_char(char o);

void kentry() {
    debug_out_char('H');
    debug_out_char('o');
    debug_out_char('l');
    debug_out_char('a');
    while(1);
}
