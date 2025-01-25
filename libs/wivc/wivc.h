#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct list_node
{
    void *value;
    struct list_node *previous;
    struct list_node *next;
} list_node_t;

typedef struct {
    size_t length;
    list_node_t *root;
    list_node_t *tail;
} list_t;

list_t *list_create();
void list_destroy(list_t *list);
void list_destroy_node(list_node_t *node);

list_node_t *list_push_before(list_t *list, list_node_t *lastNode, void *value);
list_node_t *list_push_back(list_t *list, void *value);
list_node_t *list_push_front(list_t *list, void *value);

void *list_remove_back(list_t *list);
void *list_remove_front(list_t *list);

list_node_t *list_pop(list_t *list);

void *list_remove(list_t *list, list_node_t *node);

void *list_get_back(list_t *list);
void *list_get_front(list_t *list);

size_t list_contains(list_t *list, void *value);

list_node_t *list_get_indexed(list_t *list, size_t index);
void *list_remove_indexed(list_t *list, size_t index);

#define foreach(value, list) for(list_node_t *value = list->root; value != NULL; value = value->next)

typedef struct
{
    void *value;
    list_t *children;
} gentree_node_t;

typedef struct {
    gentree_node_t *root;
} gentree_t;

gentree_t *gentree_create();
gentree_node_t *gentree_node_create(void *value);
gentree_node_t *gentree_insert(gentree_t *tree, gentree_node_t *subroot, void *value);

gentree_node_t *gentree_get_parent(gentree_t *tree, gentree_node_t *node, size_t *index);

void gentree_remove(gentree_t *tree, gentree_node_t *node);

typedef struct {
    uint32_t width;
    uint32_t height;
    uint16_t depth;
    uint32_t size;
    uint8_t *buffer;
    uint32_t stride;
    size_t gpuFd;
    uint32_t fbResID;
    uint32_t cursorResID;
} __attribute__((packed)) fb_context_t;

typedef enum {
    ALPHA_OPAQUE = 0,
    ALPHA_EMBEDDED
} sprite_alpha_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t *buffer;
    uint32_t *masks;
    uint32_t blank;
    sprite_alpha_t alpha;
}  __attribute__((packed)) sprite_t;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t x;
    uint32_t y;
}  __attribute__((packed)) graphics_rect_t;

#define CTX_PIX(ctx, x, y) *((uint32_t *)&((ctx)->buffer)[(ctx)->stride * (y) + (x) * ((ctx)->depth / 8)])
#define CTX_PIX_REF(ctx, x, y) ((uint32_t *)&((ctx)->buffer)[(ctx)->stride * (y) + (x) * ((ctx)->depth / 8)])
#define SPRITE(sprite, x, y) (sprite)->buffer[sprite->width * (y) + (x)]

typedef struct {
    size_t id;
    uint32_t width;
    uint32_t height;
    char *ctx;
    void *buffer;
} __attribute__((packed)) window_t;

typedef struct {
    FILE *client;
} __attribute__((packed)) compositor_ctx_t;

fb_context_t *graphics_init_fb_fs();
fb_context_t *graphics_init_fb_space(fb_context_t *ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
fb_context_t *graphics_init_fb_compositor(window_t *window);
void graphics_fb_flip(fb_context_t *ctx);

void graphics_draw_sprite(fb_context_t *ctx, sprite_t *sprite, int32_t x, int32_t y);
void graphics_draw_string(fb_context_t *ctx, uint32_t x, uint32_t y, const char *c, uint32_t color);
uint32_t graphics_get_string_wide(const char *c);
void graphics_fill(fb_context_t *ctx, uint32_t color);
void graphics_fill_rect(fb_context_t *ctx, graphics_rect_t *rect, uint32_t color);

compositor_ctx_t *graphics_init_compositor();
window_t *graphics_compositor_create_window(compositor_ctx_t *ctx, uint32_t width, uint32_t height);
void graphics_compositor_flip(compositor_ctx_t *ctx, window_t *window);
void graphics_compositor_move(compositor_ctx_t *ctx, window_t *window, int32_t x, int32_t y);

void graphics_init_cursor(fb_context_t *ctx, uint32_t x, uint32_t y);

//Menus
#define MENU_BAR_HEIGHT 24

typedef struct {
    char *title;
    char *command; //TODO: Maybe rename this
} __attribute__((packed)) menu_bar_entry_t;

typedef struct {
    uint32_t x;
    uint32_t y;
    uint32_t width;

    menu_bar_entry_t *entries;

    uint32_t bgColor;
    uint32_t barColor;
} __attribute__((packed)) menu_bar_t;

void graphics_draw_menu_bar(fb_context_t *ctx, menu_bar_t *bar);
void graphics_draw_decorations(fb_context_t *ctx, window_t *window, char *title);
void graphics_decoration_bounds(window_t *window, graphics_rect_t *rect);