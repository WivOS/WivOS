#include "devfs.h"
#include <fs/vfs.h>

gentree_t *devfs_tree;

vfs_node_t *devfs_finddir(vfs_node_t *parent, char *name) {
    printf("DevFS: Name: %s\n", name);

    foreach(child, devfs_tree->root->children) {
        gentreenode_t *tempChild = (gentreenode_t *)child->val;
        devfs_entry_t *entry = (devfs_entry_t *)tempChild->val;
        if(!strcmp(entry->name, name)) {
            vfs_node_t *node = (vfs_node_t *)kcalloc(sizeof(vfs_node_t *), 1);
            node->functions = entry->node->functions;
            memcpy(node->name, entry->node->name, 1024);
            node->flags = entry->node->flags;
            node->device = entry->node->device;
            return node;
        }
    }

    return NULL;
}

void devfs_mount(char *name, devfs_node_t *node) {
    devfs_entry_t *entry = (devfs_entry_t *)kcalloc(sizeof(devfs_entry_t *), 1);
    entry->name = strdup(name);
    entry->node = node;
    tree_insert(devfs_tree, devfs_tree->root, entry);
}

void initDevFS() {
    devfs_tree = tree_create();
    devfs_entry_t *root = (devfs_entry_t *)kmalloc(sizeof(devfs_entry_t *));
    root->name = strdup("root");
    root->node = NULL;
    tree_insert(devfs_tree, NULL, root);

    vfs_node_t *devfsNode = (vfs_node_t *)kcalloc(sizeof(vfs_node_t *), 1);
    strcpy(devfsNode->name, "DevFS");
    devfsNode->functions.finddir = devfs_finddir;
    devfsNode->flags |= FS_DIRECTORY;
    vfs_mount("/dev/", devfsNode);
}