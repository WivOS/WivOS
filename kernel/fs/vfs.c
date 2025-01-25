#include "vfs.h"

#include <utils/lists.h>

#include "devfs/devfs.h"
#include "fat32/fat32.h"
#include "ext2/ext2.h"

gentree_t *VFSTree;

void vfs_open(vfs_node_t *node, uint32_t flags) {
    if(!node || !node->functions.open) return;
    node->functions.open(node, flags);
}

void vfs_close(vfs_node_t *node) {
    if(!node || !node->functions.close) return;
    node->functions.close(node);
}

size_t vfs_read(vfs_node_t *node, char *buffer, size_t size) {
    if(!node || !node->functions.read) return -1;
    return node->functions.read(node, buffer, size);
}

size_t vfs_write(vfs_node_t *node, char *buffer, size_t size) {
    if(!node || !node->functions.write) return -1;
    return node->functions.write(node, buffer, size);
}

size_t vfs_lseek(vfs_node_t *node, size_t offset, size_t type) {
    if(!node || !node->functions.lseek) return -1;
    return node->functions.lseek(node, offset, type);
}

size_t vfs_ioctl(vfs_node_t *node, size_t request, void *arg) {
    if(!node || !node->functions.ioctl) return -1;
    return node->functions.ioctl(node, request, arg);
}

vfs_node_t *vfs_finddir(vfs_node_t *node, char *name, char **path) {
    if(!node || !(node->flags & VFS_DIRECTORY) || !node->functions.finddir) return NULL;
    return node->functions.finddir(node, name, path);
}

size_t vfs_readdir(vfs_node_t *node, vsf_dirent_t *dirent) {
    if(!node || !(node->flags & VFS_DIRECTORY) || !node->functions.readdir) return -1;
    return node->functions.readdir(node, dirent);
}

vfs_node_t *vfs_create(vfs_node_t *node, char *name, uint32_t flags) {
    if(!node || !(node->flags & VFS_DIRECTORY) || !node->functions.create) return NULL;
    return node->functions.create(node, name, flags);
}

size_t vfs_node_mount(vfs_node_t *node, char *device, size_t flags, char *path, void *data) {
    if(!node || !(node->flags & VFS_FILESYSTEM) || !node->functions.mount) return -1;
    return node->functions.mount(node, device, flags, path, data);
}

void vfs_init() {
    VFSTree = gentree_create();
    vfs_entry_t *root = (vfs_entry_t *)kmalloc(sizeof(vfs_entry_t));
    root->name = strdup("root");
    root->node = NULL;
    gentree_insert(VFSTree, NULL, root);

    devfs_init();

    //Init filesystems here
    fat32_init();
    ext2_init();
}

static vfs_node_t *vfs_get_mountpoint_recursive(char **path, gentree_node_t *subroot) {
    bool found = false;
    char *currentToken = strsep(path, "/");

    if(currentToken == NULL || !strcmp(currentToken, "")) {
        vfs_entry_t *entry = (vfs_entry_t *)subroot->value;
        return entry->node;
    }

    spinlock_lock(&subroot->children->lock);
    foreach(child, subroot->children) {
        gentree_node_t *newChild = (gentree_node_t *)child->value;
        vfs_entry_t *entry = (vfs_entry_t *)newChild->value;
        if(!strcmp(entry->name, currentToken)) { //Found tne new parent
            found = true;
            spinlock_unlock(&subroot->children->lock);
            subroot = newChild;
            spinlock_lock(&subroot->children->lock);
            break;
        }
    }
    spinlock_unlock(&subroot->children->lock);

    if(!found) {
        *path = currentToken;
        return ((vfs_entry_t *)subroot->value)->node;
    }
    return vfs_get_mountpoint_recursive(path, subroot);
}

vfs_node_t *vfs_get_mountpoint(char **path) {
    if(*path && strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
        (*path)[strlen(*path) - 1] = '\0';
    if(!*path || *(path)[0] != '/') return NULL;

    if(strlen(*path) == 1) {
        *path = NULL;
        vfs_entry_t *entry = (vfs_entry_t *)VFSTree->root->value;
        return entry->node;
    }
    (*path)++;
    return vfs_get_mountpoint_recursive(path, VFSTree->root);
}

vfs_node_t *kopen(const char *name, uint32_t flags) {
    char *filename = strdup((char *)name);
    char *freeFilename = filename;
    const char *originalName = filename;

    char *save = strdup((char *)filename);

    vfs_node_t *nextNode = NULL;
    vfs_node_t *startPoint = vfs_get_mountpoint((char **)&filename);
    if(!startPoint) {
        kfree(save);
        kfree(freeFilename);
        return NULL;
    }

    char *newStart = NULL;
    if(filename) newStart = strstr(save + (filename - originalName), filename);

    char *currentToken = NULL;
    while(filename != NULL && newStart && ((currentToken = strsep(&newStart, "/")) != NULL)) {
        nextNode = vfs_finddir(startPoint, currentToken, &newStart);
        if(!nextNode) {
            if(flags & O_CREAT) {
                //Create the file
                nextNode = vfs_create(startPoint, currentToken, flags);
                break;
            }

            kfree(save);
            kfree(freeFilename);
            return NULL;
        }
        startPoint = nextNode;
    }

    if(!nextNode) nextNode = startPoint;
    vfs_open(nextNode, flags);
    kfree(save);
    kfree(freeFilename);

    return nextNode;
}

static void vfs_mount_recursive(char *path, gentree_node_t *subroot, vfs_node_t *node) {
    bool found = false;
    char *currentToken = strsep(&path, "/");

    if(currentToken == NULL || !strcmp(currentToken, "")) {
        vfs_entry_t *entry = (vfs_entry_t *)subroot->value;
        if(entry->node) {
            printf("[VFS] %s: Already mounted\n", path);
            return;
        }
        entry->node = node;
        return;
    }

    spinlock_lock(&subroot->children->lock);
    foreach(child, subroot->children) {
        gentree_node_t *newChild = (gentree_node_t *)child->value;
        vfs_entry_t *entry = (vfs_entry_t *)newChild->value;
        if(!strcmp(entry->name, currentToken)) { //Found tne new parent
            found = true;
            spinlock_unlock(&subroot->children->lock);
            subroot = newChild;
            spinlock_lock(&subroot->children->lock);
            break;
        }
    }
    spinlock_unlock(&subroot->children->lock);

    if(!found) { //Create a new branch with the not found name
        vfs_entry_t *entry = (vfs_entry_t *)kmalloc(sizeof(vfs_entry_t));
        entry->name = strdup(currentToken);
        entry->node = NULL;
        subroot = gentree_insert(VFSTree, subroot, entry);
    }
    vfs_mount_recursive(path, subroot, node);
}

void vfs_mount(char *path, vfs_node_t *node) {
    node->referenceCount = -1;
    if(path[0] == '/' && strlen(path) == 1) { //Mount root
        vfs_entry_t *entry = (vfs_entry_t *)VFSTree->root->value;
        if(entry->node) {
            printf("[VFS] %s: Already mounted\n", path);
            return;
        }
        entry->node = node;
        return;
    }
    vfs_mount_recursive(path + 1, VFSTree->root, node);
}

static void vfs_print_tree_recursive(gentree_node_t *node, int parentOffset) {
    if(!node) return;

    for(size_t i = 0; i < parentOffset; ++i) {
        printf(" ");
    }

    vfs_entry_t *fnode = (vfs_entry_t *)node->value;
    size_t length = strlen(fnode->name);

    if(fnode->node) {
        printf("%s(0x%lx, %s)\n", fnode->name, (size_t)fnode->node, fnode->node->name);
        if(fnode->node->functions.print_tree) {
            fnode->node->functions.print_tree(fnode->node, parentOffset + length + 1);
        }
    }
    else
        printf("%s(empty)\n", fnode->name);

    foreach(children, node->children) {
        vfs_print_tree_recursive(children->value, parentOffset + length + 1);
    }
}

void vfs_print_tree() {
    vfs_print_tree_recursive(VFSTree->root, 0);
}