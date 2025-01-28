#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <wivc.h>
#include <compositor.h>

void client_fun() {
    /*FILE *client = fopen("/dev/pmgr/compositor", "r+");
    setbuf(client, NULL);

    uint8_t testBuffer[1024];
    compositor_window_create_t *windowCreateHeader = (compositor_window_create_t *)testBuffer;

	windowCreateHeader->header.command = COMMAND_ID_COMPOSITOR_CREATE_WINDOW;
	windowCreateHeader->width = 1280;
	windowCreateHeader->height = 1024;

	compositor_window_create_response_t *createResponse;
	size_t size = compositor_client_send_wait(client, windowCreateHeader, &createResponse, sizeof(compositor_window_create_t));

	uint64_t shmSize = 0;
	uint32_t *windowAddr = shm_open(createResponse->ctx, &shmSize);
    size_t windowId = createResponse->id;

	printf("Received SHM: %s, 0x%llx\n", createResponse->ctx, windowAddr);

	free(createResponse);

	//Draw here

	if(windowAddr != NULL && windowAddr != (void *)-1) {
		for(size_t i = 0; i < 1024; i++) {
			for(size_t j = 0; j < 1280; j++) {
				uint32_t *pixel = &windowAddr[j + i * 1280];
				*pixel = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
			}
		}
	}


	compositor_window_draw_t *windowDrawHeader = (compositor_window_draw_t *)testBuffer;

	windowDrawHeader->header.command = COMMAND_ID_COMPOSITOR_WINDOW_DRAW;
    windowDrawHeader->id = windowId;

	compositor_window_draw_response_t *drawResponse;
	size = compositor_client_send_wait(client, windowDrawHeader, &drawResponse, sizeof(compositor_window_draw_t));

	free(drawResponse);*/

    /*fwrite((const void *)testBuffer, sizeof(compositor_header_t), 1, client);

	while(1) {
        size_t size = server_client_read(client, testBuffer, 1024);

        compositor_response_header_t *header = (compositor_response_header_t *)testBuffer;
		printf("[PaketFS] Response Received: response 0x%X\n", header->response);
    }*/

	stdin = fopen("/dev/kbd", "rb");

    compositor_ctx_t *cctx = graphics_init_compositor();
	{ //First window
		window_t *window = graphics_compositor_create_window(cctx, 1024 + 2, 768 + MENU_BAR_HEIGHT + 1);

		fb_context_t *wctx = graphics_init_fb_compositor(window);

		graphics_rect_t bounds = {0};
		graphics_decoration_bounds(window, &bounds);

		graphics_fill(wctx, 0xFFCCCCCC);

		if(window->buffer != NULL && window->buffer != (void *)-1) {
			printf("Yeah, window created\n");
			for(size_t i = 0; i < bounds.height; i++) {
				for(size_t j = 0; j < bounds.width; j++) {
					size_t x = bounds.x + j;
					size_t y = bounds.y + i;
					CTX_PIX(wctx, x, y) = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
				}
			}
		}

		//Decorations and tests

		graphics_draw_decorations(wctx, window, "Test Program");

		menu_bar_t menu_bar = (menu_bar_t){0};
		menu_bar.width = window->width;
		menu_bar.x = 0;
		menu_bar.y = 0;
		menu_bar.bgColor  = 0xFF7F858F;
		menu_bar.barColor = 0xFFE1EBFC;

		menu_bar_entry_t menu_entries[] = {
			{"File", "file"},
			{"Help", "help"},
			{NULL, NULL},
		};
		menu_bar.entries = menu_entries;

		graphics_draw_menu_bar(wctx, &menu_bar);


		graphics_compositor_flip(cctx, window);

		graphics_compositor_move(cctx, window, 50, 50);

		int xMove = 50;
		int yMove = 50;
		for(;;) {
			char c = getchar();
			if(c == 0x4F)
				xMove += 10;
			else if(c == 0x50)
				xMove -= 10;
			else if(c == 0x51)
				yMove += 10;
			else if(c == 0x52)
				yMove -= 10;

			graphics_compositor_move(cctx, window, xMove, yMove);
		}
	}

	printf("Hola Yeah\n");

    while(1);
}

int main() {
    printf("Test2\n");

    client_fun();
    while(1);

    return 0;
}