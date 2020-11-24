#pragma once

#include <util/util.h>
#include <fs/vfs.h>

typedef struct devfs_node {
    char name[1024];
    void *device;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    size_t size;

    size_t offset;
    unsigned nlink;
    size_t refcount;

    vfs_functions_t functions;
} devfs_node_t;

typedef struct devfs_entry {
    char *name;
    devfs_node_t *node;
} devfs_entry_t;

void initDevFS();
void devfs_mount(char *name, devfs_node_t *node);