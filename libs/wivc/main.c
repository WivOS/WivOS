#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <sys/ioctl.h>

#include "wivc.h"

#define _USERMODE_
#include "../../modules/virtiogpu/virtiogpu.h"

#include "compositor.h"
#include "packetfs.h"

list_t *list_create() {
    list_t *list = (list_t *)malloc(sizeof(list_t));
    list->root = NULL;
    list->tail = NULL;
    list->length = 0;

    return list;
}

void list_destroy(list_t *list) {
    list_node_t *node = list->root;
    while(node != NULL) {
        list_node_t *nodeToDestroy = node;
        node = node->next;
        free((void *)nodeToDestroy);
    }
    free((void *)list);
}

void list_destroy_node(list_node_t *node) {
    free((void *)node);
}

list_node_t *list_push_before(list_t *list, list_node_t *lastNode, void *value) {
    list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));

    node->value = value;
    node->previous = list->tail;
    node->next = lastNode;

    if(list->tail == lastNode) {
        node->previous = NULL;
        list->tail = node;
    }
    lastNode->previous = node;

    list->length++;

    return node;
}

list_node_t *list_push_back(list_t *list, void *value) {
    list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));

    node->value = value;
    node->previous = list->tail;
    node->next = NULL;

    if(list->tail) list->tail->next = node;
    list->tail = node;

    if(!list->root) list->root = node;

    list->length++;

    return node;
}

list_node_t *list_push_front(list_t *list, void *value) {
    list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));

    node->value = value;
    node->previous = NULL;
    node->next = list->root;

    if(list->root) list->root->previous = node;
    else list->tail = node;
    list->root = node;

    list->length++;

    return node;
}

void *list_remove_back(list_t *list) {
    if(!list->tail) return NULL;

    list_node_t *node = list->tail;
    void *value = node->value;

    list->tail = node->previous;
    if(list->tail) list->tail->next = NULL;
    else list->root = NULL; //We are done

    free((void *)node);

    list->length--;

    return value;
}

void *list_remove_front(list_t *list) {
    if(!list->root) return NULL;

    list_node_t *node = list->root;
    void *value = node->value;

    list->root = node->next;
    if(list->root) list->root->previous = NULL;
    else list->tail = NULL; //We are done

    free((void *)node);

    list->length--;

    return value;
}

list_node_t *list_pop(list_t *list) {
    if(!list->tail) return NULL;

    list_node_t *node = list->tail;

    list->tail = node->previous;
    if(list->tail) list->tail->next = NULL;
    else list->root = NULL; //We are done

    list->length--;

    return node;
}

void *list_remove(list_t *list, list_node_t *node) {
    void *value = node->value;

    if(node == list->root)
        list_remove_front(list);
    else if(node == list->tail)
        list_remove_back(list);
    else {
        node->previous->next = node->next;
        node->next->previous = node->previous;
        list->length--;
        free((void *)node);
    }

    return value;
}

void *list_get_back(list_t *list) {
    if(!list->tail) return NULL;

    void *value = list->tail->value;

    return value;
}

void *list_get_front(list_t *list) {
    if(!list->root) return NULL;

    void *value = list->root->value;

    return value;
}

size_t list_contains(list_t *list, void *value) {
    size_t index = 0;
    foreach(node, list) {
        if(node->value == value)
            break;
        index++;
    }

    return index;
}

list_node_t *list_get_indexed(list_t *list, size_t index) {
    size_t currentIndex = 0;
    foreach(node, list) {
        if(currentIndex == index)
            return node;
        currentIndex++;
    }

    return NULL;
}

void *list_remove_indexed(list_t *list, size_t index) {
    list_node_t *node = list_get_indexed(list, index);
    return list_remove(list, node);
}


gentree_t *gentree_create() {
    gentree_t *tree = (gentree_t *)malloc(sizeof(gentree_t));
    tree->root = NULL;
    return tree;
}

gentree_node_t *gentree_node_create(void *value) {
    gentree_node_t *node = (gentree_node_t *)malloc(sizeof(gentree_node_t));
    node->value = value;
    node->children = list_create();
    return node;
}

gentree_node_t *gentree_insert(gentree_t *tree, gentree_node_t *subroot, void *value) {
    gentree_node_t *node = gentree_node_create(value);

    if(!tree->root) tree->root = node;
    else list_push_back(subroot->children, node);

    return node;
}

static gentree_node_t *gentree_get_parent_recursive(gentree_t *tree, gentree_node_t *node, gentree_node_t *subroot, size_t *index) {
    size_t currentIndex = 0;
    if((currentIndex = list_contains(subroot->children, node)) != -1) {
        if(index) *index = currentIndex;
        return subroot;
    }

    foreach(child, subroot->children) {
        gentree_node_t *returnNode = gentree_get_parent_recursive(tree, node, (gentree_node_t *)child->value, index);
        if(returnNode != NULL)
            return returnNode;
    }

    return NULL;
}

gentree_node_t *gentree_get_parent(gentree_t *tree, gentree_node_t *node, size_t *index) {
    if(node == tree->root)
        return NULL;

    return gentree_get_parent_recursive(tree, node, tree->root, index);
}

void gentree_remove(gentree_t *tree, gentree_node_t *node) {
    size_t index = -1;
    gentree_node_t *parent = gentree_get_parent(tree, node, &index);
    if(parent != NULL) {
        gentree_node_t *nodeToFree = list_remove_indexed(parent->children, index);
        free((void *)nodeToFree);
    }
}


//Graphics

fb_context_t *graphics_init_fb_fs() {
    fb_context_t *ctx = (fb_context_t *)malloc(sizeof(fb_context_t));

    FILE *gpuNodeFile = fopen("/dev/gpu", "wb");
    ctx->gpuFd = fileno(gpuNodeFile);

    virtio_mode_t *virtioModes = (virtio_mode_t *)malloc(sizeof(virtio_mode_t) * 16);
    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_GET_DISPLAY_INFO, (void *)virtioModes);

    virtgpu_set_scanout_t scanoutData = {0};
    virtgpu_create_resource_2d_t resourceData = {0};
    virtgpu_attach_backing_t attachBackingData = {0};

    resourceData.format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;

    for(size_t i = 0; i < 16; i++) {
        if(virtioModes[i].enabled) {
            resourceData.width = scanoutData.width = virtioModes[i].rect.width;
            resourceData.height = scanoutData.height = virtioModes[i].rect.height;
            scanoutData.scanoutID = i;
        }
    }

    free(virtioModes);

    uint32_t resourceID = (uint32_t)ioctl(ctx->gpuFd, VIRTGPU_IOCTL_CREATE_RESOURCE_2D, (void *)&resourceData); //TODO: Maybe store this on ctx
    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);
    ctx->fbResID = resourceID;

    uint32_t framebufferSize = resourceData.width * resourceData.height * 4;
    uint32_t *framebuffer = (uint32_t *)malloc(framebufferSize);

    attachBackingData.address = (uint64_t)framebuffer;
    attachBackingData.length = framebufferSize;

    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);
    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_SET_SCANOUT, (void *)&scanoutData);

    ctx->width = resourceData.width;
    ctx->height = resourceData.height;
    ctx->depth = 32;
    ctx->stride = 4 * ctx->width;
    ctx->size = 0;
    ctx->buffer = (uint8_t *)framebuffer;

    return ctx;
}

void graphics_init_cursor(fb_context_t *ctx, uint32_t x, uint32_t y) {
    virtgpu_create_resource_2d_t resourceData = {0};
    virtgpu_attach_backing_t attachBackingData = {0};

    resourceData.format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
    resourceData.width = 64;
    resourceData.height = 64;

    uint32_t resourceID = (uint32_t)ioctl(ctx->gpuFd, VIRTGPU_IOCTL_CREATE_RESOURCE_2D, (void *)&resourceData);
    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);

    ctx->cursorResID = resourceID;

    virtgpu_update_cursor_t updateCursor = {0};
    updateCursor.resourceID = resourceID;
    updateCursor.x = x;
    updateCursor.y = y;
    updateCursor.hotX = 0;
    updateCursor.hotY = 0;

    uint32_t framebufferSize = resourceData.width * resourceData.height * 4;
    uint32_t *framebuffer = (uint32_t *)malloc(framebufferSize);

    attachBackingData.address = (uint64_t)framebuffer;
    attachBackingData.length = framebufferSize;

    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);

    for(int y = 0; y < 64; y++) {
        for(int x = 0; x < 64; x++) {
            if((x < 16) & (y < 16))
                framebuffer[y * 64 + x] = 0xFFFF0000; //R
            else
                framebuffer[y * 64 + x] = 0x00000000; //R
        }
    }

    virtgpu_transfer_and_flush_t transferAndFlushData = {0};

    transferAndFlushData.width = 64;
    transferAndFlushData.height = 64;
    transferAndFlushData.notFlush = 1;

    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);
    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_SET_RESOURCE_ID, &ctx->fbResID);

    ioctl(ctx->gpuFd, VIRTGPU_IOCTL_UPDATE_CURSOR, (void *)&updateCursor);
}

fb_context_t *graphics_init_fb_space(fb_context_t *ctx, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    fb_context_t *new_ctx = (fb_context_t *)malloc(sizeof(fb_context_t));

    new_ctx->width = width;
    new_ctx->height = height;
    new_ctx->depth = 32;
    new_ctx->stride = ctx->stride;
    new_ctx->size = 0;
    new_ctx->buffer = ctx->buffer + x * 4 + (ctx->stride * y);

    return new_ctx;
}

fb_context_t *graphics_init_fb_compositor(window_t *window) {
    fb_context_t *ctx = (fb_context_t *)malloc(sizeof(fb_context_t));

    ctx->width = window->width;
    ctx->height = window->height;
    ctx->depth = 32;
    ctx->stride = 4 * ctx->width;
    ctx->size = 0;
    ctx->buffer = window->buffer;

    return ctx;
}

void graphics_fb_flip(fb_context_t *ctx) {
    if(ctx->gpuFd != -1) {
        //Fullscreen
        virtgpu_transfer_and_flush_t transferAndFlushData = {0};

        transferAndFlushData.width = ctx->width;
        transferAndFlushData.height = ctx->height;

        ioctl(ctx->gpuFd, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);
    } else {
        //Non needed to flip
    }
}

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

void graphics_draw_sprite(fb_context_t *ctx, sprite_t *sprite, int32_t x, int32_t y) {
    int32_t left = max(x, 0);
    int32_t top = max(y, 0);
    int32_t right = min(x + sprite->width, ctx->width - 1);
    int32_t bottom = min(y + sprite->height, ctx->height - 1);

    switch(sprite->alpha) {
        case ALPHA_OPAQUE:
            {
                for(size_t cy = (y < top) ? (top - y) : 0; cy < sprite->height && (y + cy) <= bottom; cy++) {
                    //if(y + cy < top) continue;
                    //if(y + cy > bottom) break;
                    for(size_t cx = (x < left) ? (left - x) : 0; cx < sprite->width && (x + cx) <= right; cx++)
                        CTX_PIX(ctx, x + cx, y + cy) = SPRITE(sprite, cx, cy) | 0xFF000000;
                }
            }
            break;
        default: //TODO
            break;
    }
}

void graphics_fill(fb_context_t *ctx, uint32_t color) {
    for (size_t y = 0; y < ctx->height; ++y) {
		for (size_t x = 0; x < ctx->width; ++x) {
			CTX_PIX(ctx, x, y) = color;
		}
	}
}

void graphics_fill_rect(fb_context_t *ctx, graphics_rect_t *rect, uint32_t color) {
    uint32_t x0 = min(rect->x, ctx->width);
    uint32_t y0 = min(rect->y, ctx->height);
    uint32_t x1 = min(rect->width + rect->x, ctx->width);
    uint32_t y1 = min(rect->height + rect->y, ctx->height);

    for (size_t y = y0; y < y1; ++y) {
		for (size_t x = x0; x < x1; ++x) {
			CTX_PIX(ctx, x, y) = color;
		}
	}
}

compositor_ctx_t *graphics_init_compositor() {
    compositor_ctx_t *ctx = (compositor_ctx_t *)malloc(sizeof(compositor_ctx_t));

    FILE *client = fopen("/dev/pmgr/compositor", "r+");
    setbuf(client, NULL);

    ctx->client = client;

    return ctx;
}

window_t *graphics_compositor_create_window(compositor_ctx_t *ctx, uint32_t width, uint32_t height) {
    window_t *window = (window_t *)malloc(sizeof(window_t));

    uint8_t testBuffer[1024];
    compositor_window_create_t windowCreateHeader = (compositor_window_create_t){0};

	windowCreateHeader.header.command = COMMAND_ID_COMPOSITOR_CREATE_WINDOW;
	windowCreateHeader.width = width;
	windowCreateHeader.height = height;

	compositor_window_create_response_t *createResponse;
	size_t size = compositor_client_send_wait(ctx->client, (compositor_header_t *)&windowCreateHeader, (compositor_response_header_t **)&createResponse, sizeof(compositor_window_create_t));

    window->width = width;
    window->height = height;
    window->ctx = strdup(createResponse->ctx);
    window->id = createResponse->id;

    free(createResponse);

    uint64_t shmSize = 0;
    window->buffer = shm_open(window->ctx, &shmSize);

    return window;
}

void graphics_compositor_flip(compositor_ctx_t *ctx, window_t *window) {
    compositor_window_draw_t windowDrawHeader = (compositor_window_draw_t){0};

	windowDrawHeader.header.command = COMMAND_ID_COMPOSITOR_WINDOW_DRAW;
    windowDrawHeader.id = window->id;

	compositor_window_draw_response_t *drawResponse;
	size_t size = compositor_client_send_wait(ctx->client, (compositor_header_t *)&windowDrawHeader, (compositor_response_header_t **)&drawResponse, sizeof(compositor_window_draw_t));

	free(drawResponse);
}

void graphics_compositor_move(compositor_ctx_t *ctx, window_t *window, int32_t x, int32_t y) {
    compositor_window_move_t windowMoveHeader = (compositor_window_move_t){0};

	windowMoveHeader.header.command = COMMAND_ID_COMPOSITOR_WINDOW_MOVE;
    windowMoveHeader.id = window->id;
    windowMoveHeader.x = x;
    windowMoveHeader.y = y;

	compositor_window_move_response_t *moveResponse;
	size_t size = compositor_client_send_wait(ctx->client, (compositor_header_t *)&windowMoveHeader, (compositor_response_header_t **)&moveResponse, sizeof(compositor_window_move_t));

	free(moveResponse);
}

void graphics_draw_menu_bar(fb_context_t *ctx, menu_bar_t *bar) {
    fb_context_t *subctx = graphics_init_fb_space(ctx, bar->x, bar->y, bar->width, MENU_BAR_HEIGHT);

    //TODO: Draw entries
    menu_bar_entry_t *entries = bar->entries;
    uint32_t offset = 10 + 8 *40;

    graphics_rect_t bar_rect_init = {
        .height = MENU_BAR_HEIGHT,
        .width = 1,
        .x = offset,
        .y = 0
    };
    graphics_fill_rect(subctx, &bar_rect_init, bar->barColor);

    while(entries->title) {
        uint32_t width = graphics_get_string_wide(entries->title) + 11;

        graphics_draw_string(subctx, offset + 7, 5, entries->title, 0xFFFFFFFF);
        offset += width;

        graphics_rect_t bar_rect = {
            .height = MENU_BAR_HEIGHT,
            .width = 1,
            .x = offset,
            .y = 0
        };
        graphics_fill_rect(subctx, &bar_rect, bar->barColor);

        entries++;
    }
}

void graphics_draw_decorations(fb_context_t *ctx, window_t *window, char *title) {
    graphics_rect_t rect = {
            .height = MENU_BAR_HEIGHT,
            .width = window->width,
            .x = 0,
            .y = 0
        };
    graphics_fill_rect(ctx, &rect, 0xFF7F858F);

    for(size_t i = MENU_BAR_HEIGHT; i < window->height; i++) {
        CTX_PIX(ctx, 0, i) = 0xFF7F858F;
        CTX_PIX(ctx, window->width - 1, i) = 0xFF7F858F;
    }

    graphics_draw_string(ctx, 10, 5, title, 0xFFFFFFFF);
    graphics_draw_string(ctx, window->width - 20, 5, "x", 0xFFFFFFFF);

    for(size_t i = 0; i < window->width; i++) {
        CTX_PIX(ctx, i, 0) = 0xFF7F858F;
        CTX_PIX(ctx, i, window->height - 1) = 0xFF7F858F;
    }
}

void graphics_decoration_bounds(window_t *window, graphics_rect_t *rect) {
    rect->x = 1;
    rect->y = MENU_BAR_HEIGHT;
    rect->width = window->width - rect->x - 1;
    rect->height = window->height - rect->y - 1;
}