#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>

#define _USERMODE_
#include "../../modules/virtiogpu/virtiogpu.h"

#define SYSCALL_ID(module, id) ((((size_t)(uint8_t)module & 0xFF) << 8) | ((size_t)id & 0xFF))

typedef struct {
    void *source;
    size_t size;
    uint8_t data[];
} pmgr_packet_t;

typedef struct {
    void *target;
    uint8_t data[];
} pmgr_header_t;

typedef struct {
    uint32_t command;
} __attribute__((packed)) compositor_header_t;

#define COMMAND_ID_COMPOSITOR_CREATE_WINDOW 0x1
#define COMMAND_ID_COMPOSITOR_WINDOW_DRAW 0x2

typedef struct {
	compositor_header_t header;
	size_t width;
	size_t height;
} __attribute__((packed)) compositor_window_create_t;

typedef struct {
	compositor_header_t header;
} __attribute__((packed)) compositor_window_draw_t;


typedef struct {
    uint32_t response;
} __attribute__((packed)) compositor_response_header_t;

typedef struct {
    compositor_response_header_t header;
	char ctx[256];
} __attribute__((packed)) compositor_window_create_response_t;

typedef struct {
    compositor_response_header_t header;
} __attribute__((packed)) compositor_window_draw_response_t;

size_t server_client_read(FILE *socket, uint8_t *buffer, size_t size) {
    ssize_t retSize = -1;
    while(retSize == -1) { //Just to be sure, but not needed as this is blocking read
        retSize = read(fileno(socket), (void *)buffer, size);
    }

    return retSize;
}

size_t server_client_write(FILE *socket, void *target, uint8_t *buffer, size_t size) {
	pmgr_header_t *header = malloc(size + sizeof(pmgr_header_t));
	header->target = target;
	memcpy((void *)((size_t)header + sizeof(pmgr_header_t)), buffer, size);

    ssize_t retSize = -1;
    while(retSize == -1) { //Just to be sure, but not needed as this is blocking read
        retSize = write(fileno(socket), (void *)header, size + sizeof(pmgr_header_t));
    }

    return retSize;
}

uint8_t fontBitmap[];
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

static virtgpu_set_scanout_t scanoutData = {0};

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
        if(x + currX >= (scanoutData.width / 8)) {
            y++;
            x = 0;
            currX = 0;
        }
        render_char(framebuffer, scanoutData.width, currX, y, c[i], fg, bg);
        currX++;
    }
}

size_t compositor_client_send_wait(FILE *client, compositor_header_t *header, compositor_response_header_t **response, size_t size) {
	fwrite((const void *)header, size, 1, client);

	*response = (compositor_response_header_t *)malloc(1024);
    return server_client_read(client, (void *)*response, 1024);
}

typedef uint64_t sc_qword_t;
static sc_qword_t syscall2(int sc, sc_qword_t arg1, sc_qword_t arg2) {
    sc_qword_t ret;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2)
                 : "rcx", "r11", "memory");
    return ret;
}

void client_fun() {
    FILE *client = fopen("/dev/pmgr/compositor", "r+");
    setbuf(client, NULL);

    uint8_t testBuffer[1024];
    compositor_window_create_t *windowCreateHeader = (compositor_window_create_t *)testBuffer;

	windowCreateHeader->header.command = COMMAND_ID_COMPOSITOR_CREATE_WINDOW;
	windowCreateHeader->width = 1280;
	windowCreateHeader->height = 1024;

	compositor_window_create_response_t *createResponse;
	size_t size = compositor_client_send_wait(client, windowCreateHeader, &createResponse, sizeof(compositor_window_create_t));

	uint64_t shmSize = 0;
	uint32_t *windowAddr = syscall2(SYSCALL_ID('S', 0x00), createResponse->ctx, &shmSize);

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

	windowCreateHeader->header.command = COMMAND_ID_COMPOSITOR_WINDOW_DRAW;

	compositor_window_draw_response_t *drawResponse;
	size = compositor_client_send_wait(client, windowCreateHeader, &drawResponse, sizeof(compositor_window_draw_response_t));

	free(drawResponse);

    /*fwrite((const void *)testBuffer, sizeof(compositor_header_t), 1, client);

	while(1) {
        size_t size = server_client_read(client, testBuffer, 1024);

        compositor_response_header_t *header = (compositor_response_header_t *)testBuffer;
		printf("[PaketFS] Response Received: response 0x%X\n", header->response);
    }*/

    while(1);
}

void main() {
    fprintf(stderr, "Hola\n");

    FILE *gpuNodeFile = fopen("/dev/gpu", "wb");
    size_t gpuNode = fileno(gpuNodeFile);

    virtio_mode_t *virtioModes = (virtio_mode_t *)malloc(sizeof(virtio_mode_t) * 16);
    ioctl(gpuNode, VIRTGPU_IOCTL_GET_DISPLAY_INFO, (void *)virtioModes);

    virtgpu_create_resource_2d_t resourceData = {0};
    virtgpu_attach_backing_t attachBackingData = {0};
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

    uint32_t resourceID = (uint32_t)ioctl(gpuNode, VIRTGPU_IOCTL_CREATE_RESOURCE_2D, (void *)&resourceData);
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_RESOURCE_ID, &resourceID);

    uint32_t framebufferSize = resourceData.width * resourceData.height * 4;
    uint32_t *framebuffer = (uint32_t *)malloc(framebufferSize + 0x1000);

    attachBackingData.address = (uint64_t)framebuffer;
    attachBackingData.length = framebufferSize;

    ioctl(gpuNode, VIRTGPU_IOCTL_RESOURCE_ATTACH_BACKING, (void *)&attachBackingData);
    ioctl(gpuNode, VIRTGPU_IOCTL_SET_SCANOUT, (void *)&scanoutData);

    for(size_t i = 0; i < resourceData.height; i++) {
        for(size_t j = 0; j < resourceData.width / 2; j++) {
            uint32_t *pixel = &framebuffer[j + i * resourceData.width];
            *pixel = 0xFF000000 | ((i * j) & 0x0FF0) << 12 | ((i * j) & 0xFF00) | ((i * j) & 0xFF);
        }
    }

    render_string(framebuffer, 0, 0, "Hola que tal?\n Yo bien y tu?\n\nPues tambien bien :D", 0xFFFFFFFF, 0x0);

    ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);

    fprintf(stderr, "Hola 2\n");

	//uint64_t shmSize = 1024*768*4;
	//void *addr = syscall2(SYSCALL_ID('S', 0x00), "test.test0", &shmSize);

	//fprintf(stderr, "SHM: %llx, %llx\n", (uint64_t)addr, (uint64_t)shmSize);

    FILE *server = fopen("/dev/pmgr/compositor", "w+b");
    setbuf(server, NULL);

    int x = 1;
    if(!fork()) //TODO: Implement fd duping
        client_fun();

    uint8_t testBuffer[1024];

	//Test parameters to test windows before programming it xD
	char testCompositor[] = "compositor.test0";
	void *windowAddr = NULL;
	size_t windowWidth = 0;
	size_t windowHeight = 0;

    while(1) {
        size_t size = server_client_read(server, testBuffer, 1024);

        pmgr_packet_t *packet = (pmgr_packet_t *)testBuffer;
        compositor_header_t *header = (compositor_header_t *)((uint64_t)testBuffer + sizeof(pmgr_packet_t));

		switch(header->command) {
			case COMMAND_ID_COMPOSITOR_CREATE_WINDOW:
				{
					compositor_window_create_t *request = (compositor_window_create_t *)header;
					compositor_window_create_response_t *response = malloc(sizeof(compositor_window_create_response_t));

					windowWidth = request->width;
					windowHeight = request->height;
					uint64_t shmSize = request->height * request->width * 4;
					windowAddr = syscall2(SYSCALL_ID('S', 0x00), testCompositor, &shmSize);
					strcpy(response->ctx, testCompositor);

					server_client_write(server, packet->source, response, sizeof(compositor_window_create_response_t));
				}
				break;
			case COMMAND_ID_COMPOSITOR_WINDOW_DRAW:
				{
					compositor_response_header_t *request = (compositor_window_create_t *)header;
					compositor_window_create_response_t *response = malloc(sizeof(compositor_window_create_response_t));

					if(windowAddr != NULL && windowAddr != (void *)-1) {
						for(size_t i = 0; i < windowHeight; i++) {
							for(size_t j = 0; j < windowWidth; j++) {
								if(j < resourceData.width && i < resourceData.height) framebuffer[j + i * resourceData.width] = ((uint32_t *)windowAddr)[j + i * windowWidth];
							}
						}

						ioctl(gpuNode, VIRTGPU_IOCTL_TRANSFER_AND_FLUSH, (void *)&transferAndFlushData);
					}

					server_client_write(server, packet->source, response, sizeof(compositor_window_create_response_t));
				}
				break;
			default:
				printf("[PaketFS] Test Received: command 0x%X, id 0x%X\n", header->command);
		}
    }

    while(1);
}

//Server -> Client
/*
    uint8_t testBuffer[1024];
    pmgr_header_t *testHeader = (pmgr_header_t *)testBuffer;

    testHeader->target = NULL;

    compositor_header_t *header = (compositor_header_t *)testHeader->data;
    header->command = 0x5;
    header->id = 0x3;

    fwrite((const void *)testBuffer, sizeof(pmgr_header_t) + sizeof(compositor_header_t), 1, server);
*/

//Receive Client
/*
    size_t size = -1;
    while(size == -1) {
        size = read(fileno(client), (void *)testBuffer, 1024);
    }

    compositor_header_t *header = (compositor_header_t *)testBuffer;
    printf("[PaketFS] Test Received: command 0x%X, id 0x%X\n", header->command, header->id);
*/





// Same data as the Linux kernel uses, extracted from the PC VGA font.
uint8_t fontBitmap[] = {
	// 32 0x20 ' '
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 33 0x21 '!'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00111100,
	0b00111100,
	0b00111100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 34 0x22 '"'
	0b00000000,
	0b01100110,
	0b01100110,
	0b01100110,
	0b00100100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 35 0x23 '#'
	0b00000000,
	0b00000000,
	0b00000000,
	0b01101100,
	0b01101100,
	0b11111110,
	0b01101100,
	0b01101100,
	0b01101100,
	0b11111110,
	0b01101100,
	0b01101100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 36 0x24 '$'
	0b00011000,
	0b00011000,
	0b01111100,
	0b11000110,
	0b11000010,
	0b11000000,
	0b01111100,
	0b00000110,
	0b00000110,
	0b10000110,
	0b11000110,
	0b01111100,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,

	// 37 0x25 '%'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000010,
	0b11000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000110,
	0b10000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 38 0x26 '&'
	0b00000000,
	0b00000000,
	0b00111000,
	0b01101100,
	0b01101100,
	0b00111000,
	0b01110110,
	0b11011100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 39 0x27 '''
	0b00000000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b01100000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 40 0x28 '('
	0b00000000,
	0b00000000,
	0b00001100,
	0b00011000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 41 0x29 ')'
	0b00000000,
	0b00000000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00011000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 42 0x2a '*'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01100110,
	0b00111100,
	0b11111111,
	0b00111100,
	0b01100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 43 0x2b '+'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b01111110,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 44 0x2c ','
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 45 0x2d '-'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 46 0x2e '.'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 47 0x2f '/'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000000,
	0b10000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 48 0x30 '0'
	0b00000000,
	0b00000000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11010110,
	0b11000110,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 49 0x31 '1'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00111000,
	0b01111000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b01111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 50 0x32 '2'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000000,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 51 0x33 '3'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b00000110,
	0b00000110,
	0b00111100,
	0b00000110,
	0b00000110,
	0b00000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 52 0x34 '4'
	0b00000000,
	0b00000000,
	0b00001100,
	0b00011100,
	0b00111100,
	0b01101100,
	0b11001100,
	0b11111110,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00011110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 53 0x35 '5'
	0b00000000,
	0b00000000,
	0b11111110,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11111100,
	0b00000110,
	0b00000110,
	0b00000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 54 0x36 '6'
	0b00000000,
	0b00000000,
	0b00111000,
	0b01100000,
	0b11000000,
	0b11000000,
	0b11111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 55 0x37 '7'
	0b00000000,
	0b00000000,
	0b11111110,
	0b11000110,
	0b00000110,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 56 0x38 '8'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 57 0x39 '9'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00001100,
	0b01111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 58 0x3a ':'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 59 0x3b ';'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 60 0x3c '<'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 61 0x3d '='
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111110,
	0b00000000,
	0b00000000,
	0b01111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 62 0x3e '>'
	0b00000000,
	0b00000000,
	0b00000000,
	0b01100000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 63 0x3f '?'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b00001100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 64 0x40 '@'
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11011110,
	0b11011110,
	0b11011110,
	0b11011100,
	0b11000000,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 65 0x41 'A'
	0b00000000,
	0b00000000,
	0b00010000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b11000110,
	0b11111110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 66 0x42 'B'
	0b00000000,
	0b00000000,
	0b11111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b11111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 67 0x43 'C'
	0b00000000,
	0b00000000,
	0b00111100,
	0b01100110,
	0b11000010,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000010,
	0b01100110,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 68 0x44 'D'
	0b00000000,
	0b00000000,
	0b11111000,
	0b01101100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01101100,
	0b11111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 69 0x45 'E'
	0b00000000,
	0b00000000,
	0b11111110,
	0b01100110,
	0b01100010,
	0b01101000,
	0b01111000,
	0b01101000,
	0b01100000,
	0b01100010,
	0b01100110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 70 0x46 'F'
	0b00000000,
	0b00000000,
	0b11111110,
	0b01100110,
	0b01100010,
	0b01101000,
	0b01111000,
	0b01101000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 71 0x47 'G'
	0b00000000,
	0b00000000,
	0b00111100,
	0b01100110,
	0b11000010,
	0b11000000,
	0b11000000,
	0b11011110,
	0b11000110,
	0b11000110,
	0b01100110,
	0b00111010,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 72 0x48 'H'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11111110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 73 0x49 'I'
	0b00000000,
	0b00000000,
	0b00111100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 74 0x4a 'J'
	0b00000000,
	0b00000000,
	0b00011110,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 75 0x4b 'K'
	0b00000000,
	0b00000000,
	0b11100110,
	0b01100110,
	0b01100110,
	0b01101100,
	0b01111000,
	0b01111000,
	0b01101100,
	0b01100110,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 76 0x4c 'L'
	0b00000000,
	0b00000000,
	0b11110000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100010,
	0b01100110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 77 0x4d 'M'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11101110,
	0b11111110,
	0b11111110,
	0b11010110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 78 0x4e 'N'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11100110,
	0b11110110,
	0b11111110,
	0b11011110,
	0b11001110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 79 0x4f 'O'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 80 0x50 'P'
	0b00000000,
	0b00000000,
	0b11111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01100000,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 81 0x51 'Q'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11011110,
	0b01111100,
	0b00001100,
	0b00001110,
	0b00000000,
	0b00000000,

	// 82 0x52 'R'
	0b00000000,
	0b00000000,
	0b11111100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01101100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 83 0x53 'S'
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b01100000,
	0b00111000,
	0b00001100,
	0b00000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 84 0x54 'T'
	0b00000000,
	0b00000000,
	0b01111110,
	0b01111110,
	0b01011010,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 85 0x55 'U'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 86 0x56 'V'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00010000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 87 0x57 'W'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11111110,
	0b11101110,
	0b01101100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 88 0x58 'X'
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b01101100,
	0b01111100,
	0b00111000,
	0b00111000,
	0b01111100,
	0b01101100,
	0b11000110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 89 0x59 'Y'
	0b00000000,
	0b00000000,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b00111100,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 90 0x5a 'Z'
	0b00000000,
	0b00000000,
	0b11111110,
	0b11000110,
	0b10000110,
	0b00001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000010,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 91 0x5b '['
	0b00000000,
	0b00000000,
	0b00111100,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 92 0x5c '\'
	0b00000000,
	0b00000000,
	0b00000000,
	0b10000000,
	0b11000000,
	0b11100000,
	0b01110000,
	0b00111000,
	0b00011100,
	0b00001110,
	0b00000110,
	0b00000010,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 93 0x5d ']'
	0b00000000,
	0b00000000,
	0b00111100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 94 0x5e '^'
	0b00010000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 95 0x5f '_'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111111,
	0b00000000,
	0b00000000,

	// 96 0x60 '`'
	0b00000000,
	0b00110000,
	0b00011000,
	0b00001100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 97 0x61 'a'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111000,
	0b00001100,
	0b01111100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 98 0x62 'b'
	0b00000000,
	0b00000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b01111000,
	0b01101100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 99 0x63 'c'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000000,
	0b11000000,
	0b11000000,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 100 0x64 'd'
	0b00000000,
	0b00000000,
	0b00011100,
	0b00001100,
	0b00001100,
	0b00111100,
	0b01101100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 101 0x65 'e'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11111110,
	0b11000000,
	0b11000000,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 102 0x66 'f'
	0b00000000,
	0b00000000,
	0b00011100,
	0b00110110,
	0b00110010,
	0b00110000,
	0b01111000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b01111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 103 0x67 'g'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01110110,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01111100,
	0b00001100,
	0b11001100,
	0b01111000,
	0b00000000,

	// 104 0x68 'h'
	0b00000000,
	0b00000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b01101100,
	0b01110110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 105 0x69 'i'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00111000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 106 0x6a 'j'
	0b00000000,
	0b00000000,
	0b00000110,
	0b00000110,
	0b00000000,
	0b00001110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b00000110,
	0b01100110,
	0b01100110,
	0b00111100,
	0b00000000,

	// 107 0x6b 'k'
	0b00000000,
	0b00000000,
	0b11100000,
	0b01100000,
	0b01100000,
	0b01100110,
	0b01101100,
	0b01111000,
	0b01111000,
	0b01101100,
	0b01100110,
	0b11100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 108 0x6c 'l'
	0b00000000,
	0b00000000,
	0b00111000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 109 0x6d 'm'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11101100,
	0b11111110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 110 0x6e 'n'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11011100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 111 0x6f 'o'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 112 0x70 'p'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11011100,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01100110,
	0b01111100,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,

	// 113 0x71 'q'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01110110,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01111100,
	0b00001100,
	0b00001100,
	0b00011110,
	0b00000000,

	// 114 0x72 'r'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11011100,
	0b01110110,
	0b01100110,
	0b01100000,
	0b01100000,
	0b01100000,
	0b11110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 115 0x73 's'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b01111100,
	0b11000110,
	0b01100000,
	0b00111000,
	0b00001100,
	0b11000110,
	0b01111100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 116 0x74 't'
	0b00000000,
	0b00000000,
	0b00010000,
	0b00110000,
	0b00110000,
	0b11111100,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110000,
	0b00110110,
	0b00011100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 117 0x75 'u'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b11001100,
	0b01110110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 118 0x76 'v'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 119 0x77 'w'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11010110,
	0b11010110,
	0b11010110,
	0b11111110,
	0b01101100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 120 0x78 'x'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b01101100,
	0b00111000,
	0b00111000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 121 0x79 'y'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11000110,
	0b01111110,
	0b00000110,
	0b00001100,
	0b11111000,
	0b00000000,

	// 122 0x7a 'z'
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b11111110,
	0b11001100,
	0b00011000,
	0b00110000,
	0b01100000,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 123 0x7b '{'
	0b00000000,
	0b00000000,
	0b00001110,
	0b00011000,
	0b00011000,
	0b00011000,
	0b01110000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00001110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 124 0x7c '|'
	0b00000000,
	0b00000000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 125 0x7d '}'
	0b00000000,
	0b00000000,
	0b01110000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00001110,
	0b00011000,
	0b00011000,
	0b00011000,
	0b00011000,
	0b01110000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 126 0x7e '~'
	0b00000000,
	0b01110110,
	0b11011100,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,

	// 127 0x7f ''
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00010000,
	0b00111000,
	0b01101100,
	0b11000110,
	0b11000110,
	0b11000110,
	0b11111110,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
};