#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "wivc.h"

typedef struct {
    uint32_t command;
} __attribute__((packed)) compositor_header_t;

#define COMMAND_ID_COMPOSITOR_CREATE_WINDOW 0x1
#define COMMAND_ID_COMPOSITOR_WINDOW_DRAW 0x2
#define COMMAND_ID_COMPOSITOR_WINDOW_MOVE 0x3

typedef struct {
	compositor_header_t header;
	size_t width;
	size_t height;
} __attribute__((packed)) compositor_window_create_t;

typedef struct {
	compositor_header_t header;
    size_t id;
} __attribute__((packed)) compositor_window_draw_t;

typedef struct {
	compositor_header_t header;
    size_t id;
    int32_t x;
    int32_t y;
} __attribute__((packed)) compositor_window_move_t;


typedef struct {
    uint32_t magic; //Not used
} __attribute__((packed)) compositor_response_header_t;

typedef struct {
    compositor_response_header_t header;
    size_t id;
	char ctx[256];
} __attribute__((packed)) compositor_window_create_response_t;

typedef struct {
    compositor_response_header_t header;
} __attribute__((packed)) compositor_window_draw_response_t;

typedef struct {
    compositor_response_header_t header;
} __attribute__((packed)) compositor_window_move_response_t;

size_t compositor_client_send_wait(FILE *client, compositor_header_t *header, compositor_response_header_t **response, size_t size);

typedef struct {
    uint32_t width;
    uint32_t height;
    int32_t x;
    int32_t y;
} __attribute__((packed)) compositor_rect_t;

typedef struct {
    compositor_rect_t rect;
    char title[256];
    sprite_t *sprite;
    char *ctx;
    size_t id;
} __attribute__((packed)) compositor_window_t;