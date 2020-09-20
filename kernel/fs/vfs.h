#pragma once

#include <util/util.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

struct vfs_node;

typedef struct vfs_functions {
    size_t (*read)(struct vfs_node *, char *, size_t, size_t);
    size_t (*write)(struct vfs_node *, char *, size_t, size_t);
    void (*open)(struct vfs_node *, uint32_t flags);
    void (*close)(struct vfs_node *);
    struct vfs_node *(*finddir)(struct vfs_node *, char *name);
} vfs_functions_t;

typedef struct vfs_node {
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
} vfs_node_t;

typedef struct vfs_entry {
    char *name;
    vfs_node_t *file;
} vfs_entry_t;

// Function wrappers
size_t vfs_read(vfs_node_t *node, char *buffer, size_t offset, size_t size);
size_t vfs_write(vfs_node_t *node, char *buffer, size_t offset, size_t size);
void vfs_open(vfs_node_t *node, uint32_t flags);
void vfs_close(vfs_node_t *node);
vfs_node_t *vfs_finddir(vfs_node_t *node, char *name);

// Utils
char *vfs_remove_dot_chars(char *input);
void print_vfstree();

vfs_node_t *get_mountpoint(char **path);

void vfs_init();
void vfs_mount(char *path, vfs_node_t *fsNode);

vfs_node_t *kopen(const char *fileName, unsigned int flags);