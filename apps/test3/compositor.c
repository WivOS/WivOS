#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <wivc.h>

#define _USERMODE_
#include "../../modules/virtiogpu/virtiogpu.h"

#include "compositor.h"

static virtgpu_set_scanout_t scanoutData = {0};
static gentree_t *window_tree = NULL;

static size_t windowIdAllocator = 1;

void compositor_add_window(compositor_window_t *window) {
    gentree_node_t *node = gentree_insert(window_tree, window_tree->root, window);
    window->id = windowIdAllocator++;
}

compositor_window_t *compositor_get_window(size_t id) {
    foreach(node, window_tree->root->children) {
        gentree_node_t *gnode = (gentree_node_t *)(node->value);
        compositor_window_t *window = (compositor_window_t *)gnode->value;
        if(window->id == id)
            return window;
    }

    return NULL;
}

void redraw_windows(fb_context_t *ctx) {
    graphics_fill(ctx, 0xFF5371A6); //83, 113, 166

    foreach(node, window_tree->root->children) {
        gentree_node_t *gnode = (gentree_node_t *)node->value;
        compositor_window_t *window = (compositor_window_t *)gnode->value;

        graphics_draw_sprite(ctx, window->sprite, window->rect.x, window->rect.y);
    }

    graphics_fb_flip(ctx);
}

int compositor_main() {
    fb_context_t *ctx = graphics_init_fb_fs();

    for(size_t i = 0; i < ctx->height; i++) {
        for(size_t j = 0; j < ctx->width / 2; j++) {
            uint32_t *pixel = &((uint32_t *)ctx->buffer)[j + i * ctx->width];
            *pixel = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
        }
    }

    graphics_fb_flip(ctx);

    FILE *server = fopen("/dev/pmgr/compositor", "w+b");
    setbuf(server, NULL);

    if(!fork()) //TODO: Implement fd duping
        return 0;

    uint8_t testBuffer[1024];

    window_tree = gentree_create();
    gentree_insert(window_tree, NULL, NULL); //Root is always null

    while(1) {
        size_t size = server_client_read(server, testBuffer, 1024);

        pmgr_packet_t *packet = (pmgr_packet_t *)testBuffer;
        compositor_header_t *header = (compositor_header_t *)((uint64_t)testBuffer + sizeof(pmgr_packet_t));

		switch(header->command) {
			case COMMAND_ID_COMPOSITOR_CREATE_WINDOW:
				{
					compositor_window_create_t *request = (compositor_window_create_t *)header;
					compositor_window_create_response_t *response = malloc(sizeof(compositor_window_create_response_t));

                    compositor_window_t *window = (compositor_window_t *)malloc(sizeof(compositor_window_t));
                    window->rect.height = request->height;
                    window->rect.width = request->width;
                    window->rect.x = 0;
                    window->rect.y = 0;

                    compositor_add_window(window);

                    sprintf(response->ctx, "compositor.%lld", window->id);
                    window->ctx = strdup(response->ctx);

                    response->id = window->id;

                    window->sprite = (sprite_t *)malloc(sizeof(sprite_t));
                    window->sprite->width = window->rect.width;
                    window->sprite->height = window->rect.height;
                    window->sprite->alpha = ALPHA_OPAQUE;
                    window->sprite->masks = NULL;
                    window->sprite->blank = 0;

					uint64_t shmSize = window->rect.height * window->rect.width * 4;
					window->sprite->buffer = shm_open(response->ctx, &shmSize);
					strcpy(response->ctx, window->ctx);

					server_client_write(server, packet->source, response, sizeof(compositor_window_create_response_t));
                    free(response);
				}
				break;
			case COMMAND_ID_COMPOSITOR_WINDOW_DRAW:
				{
					compositor_window_draw_t *request = (compositor_window_draw_t *)header;
					compositor_window_draw_response_t *response = malloc(sizeof(compositor_window_draw_response_t));

                    size_t windowId = request->id;
                    compositor_window_t *window = compositor_get_window(windowId);
                    if(window != NULL) {
                        redraw_windows(ctx);
                    }

					server_client_write(server, packet->source, response, sizeof(compositor_window_draw_response_t));
                    free(response);
				}
				break;
            case COMMAND_ID_COMPOSITOR_WINDOW_MOVE:
                {
					compositor_window_move_t *request = (compositor_window_move_t *)header;
					compositor_window_move_response_t *response = malloc(sizeof(compositor_window_move_response_t));

                    size_t windowId = request->id;
                    compositor_window_t *window = compositor_get_window(windowId);
                    if(window != NULL) {
                        window->rect.x = request->x;
                        window->rect.y = request->y;
                        redraw_windows(ctx);
                    }

					server_client_write(server, packet->source, response, sizeof(compositor_window_move_response_t));
                    free(response);
				}
                break;
			default:
				printf("[PaketFS] Test Received: command 0x%X, id 0x%X\n", header->command);
		}
    }

    while(1);
}