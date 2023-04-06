#pragma once

#include <utils/common.h>
#include <stdbool.h>

#include <mem/vmm.h>
#include "thread.h"

#include <fs/vfs.h>

#define BASE_ALLOC_ADDRESS ((size_t)0x0000780000000000)
#define USER_SHM_LOW      ((size_t)0x0000400100000000UL)

#define MAX_FILE_HANDLES 256

typedef struct process {
    volatile kpid_t pid;
    volatile pt_t *page_table;
    volatile thread_t **threads;
    volatile spinlock_t lock;
    volatile uint64_t current_alloc_address;
    volatile spinlock_t alloc_lock;
    volatile vfs_node_t **file_handles;
    volatile spinlock_t file_handles_lock;
    volatile kpid_t parent_pid;
    volatile list_t *shm_map;
    volatile size_t shm_alloc;
} __attribute__((packed)) process_t;

kpid_t process_create(pt_t *page_table);