#include "devfs.h"
#include <utils/lists.h>

#include <fs/ioctl.h>

list_t *DevFSList;

vfs_node_t *devfs_finddir(vfs_node_t *node_dev, char *name, char **path) {
    spinlock_lock(&DevFSList->lock);
    foreach(child, DevFSList) {
        devfs_entry_t *entry = (devfs_entry_t *)child->value;
        if(!strcmp(entry->name, name)) {
            vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
            node->functions = entry->node->functions;
            memcpy(node->name, entry->node->name, 1024);
            node->flags = entry->node->flags;
            node->data = entry->node->data;
            node->internIndex = entry->node->internIndex;
            spinlock_unlock(&DevFSList->lock);
            return node;
        }
    }
    spinlock_unlock(&DevFSList->lock);

    return NULL;
}

void devfs_print_tree(struct vfs_node *node, size_t parentOffset) {
    spinlock_lock(&DevFSList->lock);
    foreach(child, DevFSList) {
        devfs_entry_t *entry = (devfs_entry_t *)child->value;
        for(size_t i = 0; i < parentOffset; ++i) {
            printf(" ");
        }
        printf("%s(0x%lx, %s)\n", entry->name, (size_t)entry->node, entry->node->name);
    }
    spinlock_unlock(&DevFSList->lock);
}

void devfs_mount(char *name, devfs_node_t *node) {
    devfs_entry_t *entry = (devfs_entry_t *)kmalloc(sizeof(devfs_entry_t));
    entry->name = strdup(name);
    entry->node = node;

    list_push_back(DevFSList, entry);
}

static size_t devfs_log_write(struct vfs_node *node, char *buffer, size_t size) {
    (void)node;

    printf("%.*s", size, buffer);
    return size;
}

static size_t devfs_log_ioctl(struct vfs_node *node, size_t request, void *arg) {
    (void)node;

    if(request == TIOCGWINSZ) {
        //TODO: return winsize
        return 0;
    }

    return -1;
}

void devfs_init() {
    DevFSList = list_create();

    vfs_node_t *devfsNode = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    strcpy(devfsNode->name, "DevFS");
    devfsNode->functions.finddir = devfs_finddir;
    devfsNode->functions.print_tree = devfs_print_tree;
    devfsNode->flags |= VFS_DIRECTORY;
    vfs_mount("/dev/", devfsNode);

    devfs_node_t *logNode = (devfs_node_t *)kmalloc(sizeof(devfs_node_t));
    strcpy(logNode->name, "SerialLog");
    logNode->functions.write = devfs_log_write;
    logNode->functions.ioctl = devfs_log_ioctl;
    devfs_mount("log", logNode);
}