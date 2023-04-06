#pragma once

#ifndef _USERMODE_

#include <utils/common.h>
#include <modules/modules.h>
#include <devices/pci.h>

#include <tasking/scheduler.h>

#endif

struct shm_node;

typedef struct {
    struct shm_node *parent;
    volatile spinlock_t lock;
    int64_t ref_count;
    size_t pageCount;
    void *phys;
} shm_chunk_t;

typedef struct shm_node {
    char name[256];
    shm_chunk_t *chunk;
} shm_node_t;

typedef struct {
    shm_chunk_t *chunk;
    volatile spinlock_t lock;
    void *virt;
} shm_mapping_t;