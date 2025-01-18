#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <utils/common.h>

struct vfs_node;

#define VFS_FILE        0x01
#define VFS_DIRECTORY   0x02
#define VFS_FILESYSTEM  0x04

#define SEEK_CUR       1
#define SEEK_END       2
#define SEEK_SET       0

#define POLLIN 0x01
#define POLLOUT 0x02
#define POLLPRI 0x04
#define POLLHUP 0x08
#define POLLERR 0x10
#define POLLRDHUP 0x20
#define POLLNVAL 0x40

#define O_CREAT    0x0010
#define O_NONBLOCK 0x0400

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

typedef struct {
    uint64_t d_ino;
    uint64_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[1024];
} vsf_dirent_t;

typedef struct {
    size_t (*open)(struct vfs_node *node, uint32_t flags);
    size_t (*close)(struct vfs_node *node);
    size_t (*read)(struct vfs_node *node, char *buffer, size_t size);
    size_t (*write)(struct vfs_node *node, char *buffer, size_t size);
    size_t (*lseek)(struct vfs_node *node, size_t offset, size_t type);
    size_t (*ioctl)(struct vfs_node *node, size_t request, void *arg);
    struct vfs_node *(*finddir)(struct vfs_node *node, char *name, char **path);
    size_t (*readdir)(struct vfs_node *node, vsf_dirent_t *dirent);
    struct vfs_node *(*create)(struct vfs_node *node, char *name, uint32_t flags);

    size_t (*mount)(struct vfs_node *node, char *device, size_t flags, char *path, void *data); //Call that calls vfs_mount at the end

    void (*print_tree)(struct vfs_node *node, size_t parentOffset);
} vfs_functions_t;

typedef struct vfs_node {
    char name[1024];

    void *data;  //Generic data defined by the device itself, could be anything
    uint32_t flags;

    size_t internIndex;

    size_t size;
    size_t referenceCount;

    size_t offset;
    uint32_t status;

    vfs_functions_t functions;
} vfs_node_t;

typedef struct {
    char *name;
    vfs_node_t *node;
} vfs_entry_t;

void vfs_init();

void vfs_open(vfs_node_t *node, uint32_t flags);
void vfs_close(vfs_node_t *node);
size_t vfs_read(vfs_node_t *node, char *buffer, size_t size);
size_t vfs_write(vfs_node_t *node, char *buffer, size_t size);
size_t vfs_lseek(vfs_node_t *node, size_t offset, size_t type);
size_t vfs_ioctl(vfs_node_t *node, size_t request, void *arg);
vfs_node_t *vfs_finddir(vfs_node_t *node, char *name, char **path);
size_t vfs_readdir(vfs_node_t *node, vsf_dirent_t *dirent);
vfs_node_t *vfs_create(vfs_node_t *node, char *name, uint32_t flafs);

size_t vfs_node_mount(vfs_node_t *node, char *device, size_t flags, char *path, void *data);

vfs_node_t *vfs_get_mountpoint(char **path);
vfs_node_t *kopen(const char *name, uint32_t flags);

void vfs_mount(char *path, vfs_node_t *node);

void vfs_print_tree();