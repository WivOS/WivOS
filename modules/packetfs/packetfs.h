#pragma once

#include <utils/common.h>

#define MAX_PACKET_SIZE 1024

typedef struct {
    list_t *filesystem;
    volatile spinlock_t lock;
} pmgr_t;

typedef struct {
    volatile spinlock_t lock;
    char *name;
    bool new;
    vfs_node_t *server_pipe;
    list_t *clients;
} pmgr_instance_t;

typedef struct {
    vfs_node_t *pipe;
    pmgr_instance_t *parent;
} __attribute__((packed)) pmgr_client_t;

typedef struct {
    pmgr_client_t *source;
    size_t size;
    uint8_t data[];
} __attribute__((packed)) pmgr_packet_t;

typedef struct {
    pmgr_client_t *target;
    uint8_t data[];
} __attribute__((packed)) pmgr_header_t;