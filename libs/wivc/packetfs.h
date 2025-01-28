#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    void *source;
    size_t size;
    uint8_t data[];
} pmgr_packet_t;

typedef struct {
    void *target;
    uint8_t data[];
} pmgr_header_t;

size_t server_client_read(FILE *socket, uint8_t *buffer, size_t size);
size_t server_client_write(FILE *socket, void *target, uint8_t *buffer, size_t size);

void *shm_open(char *path, size_t *size);