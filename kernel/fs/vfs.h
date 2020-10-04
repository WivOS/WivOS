#pragma once

#include <util/util.h>

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

typedef int64_t off_t;

typedef uint64_t dev_t;
typedef uint64_t ino_t;
typedef int32_t mode_t;
typedef int32_t nlink_t;
typedef int64_t blksize_t;
typedef int64_t blkcnt_t;

typedef int32_t uid_t;
typedef int32_t gid_t;

typedef int64_t time_t;

typedef struct timespec {
    time_t tv_sec;
    long tv_nsec;
} timespec_t;

typedef struct stat {
    dev_t st_dev;
    ino_t st_ino;
    mode_t st_mode;
    nlink_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    timespec_t st_atim;
    timespec_t st_mtim;
    timespec_t st_ctim;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
} stat_t;

struct vfs_node;

typedef struct vfs_functions {
    size_t (*read)(struct vfs_node *, char *, size_t);
    size_t (*write)(struct vfs_node *, char *, size_t);
    size_t (*fstat)(struct vfs_node *, stat_t *);
    size_t (*lseek)(struct vfs_node *, off_t, int);
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
size_t vfs_read(vfs_node_t *node, char *buffer, size_t size);
size_t vfs_write(vfs_node_t *node, char *buffer, size_t size);
size_t vfs_fstat(vfs_node_t *node, stat_t *stat);
size_t vfs_lseek(vfs_node_t *node, off_t offset, int type);
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