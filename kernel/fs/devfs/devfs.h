#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <utils/common.h>
#include <fs/vfs.h>

typedef struct {
    char name[1024];
    void *data;
    uint32_t flags;

    size_t internIndex;

    vfs_functions_t functions;
} devfs_node_t;

typedef struct {
    char *name;

    devfs_node_t *node;
} devfs_entry_t;

//Internal but usable functions
vfs_node_t *devfs_finddir(vfs_node_t *node, char *name, char **path);

void devfs_init();
void devfs_mount(char *name, devfs_node_t *node);