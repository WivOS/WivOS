#include <fs/vfs.h>

gentree_t *vfs_tree;

// Function wrappers
size_t vfs_read(vfs_node_t *node, char *buffer, size_t size) {
    if(!node || !node->functions.read) return -1;
    return node->functions.read((struct vfs_node *)node, buffer, size);
}

size_t vfs_write(vfs_node_t *node, char *buffer, size_t size) {
    if(!node || !node->functions.read) return -1;
    return node->functions.write((struct vfs_node *)node, buffer, size);
}

size_t vfs_fstat(vfs_node_t *node, stat_t *stat) {
    if(!node || !node->functions.fstat || !stat) return -1;
    return node->functions.fstat((struct vfs_node *)node, stat);
}

size_t vfs_lseek(vfs_node_t *node, off_t offset, int type) {
    if(!node || !node->functions.lseek) return -1;
    return node->functions.lseek((struct vfs_node *)node, offset, type);
}

void vfs_open(vfs_node_t *node, uint32_t flags) {
    if(!node || !node->functions.open) return;
    if(node->refcount >= 0) node->refcount++;
    node->functions.open((struct vfs_node *)node, flags);
}

void vfs_close(vfs_node_t *node) {
    if(!node || !node->functions.close) return;
    node->refcount--;
    if(node->refcount == 0) node->functions.close((struct vfs_node *)node);
}

vfs_node_t *vfs_finddir(vfs_node_t *node, char *name) {
    if(!node || !(node->flags & FS_DIRECTORY) || !node->functions.finddir) return NULL;
    return (vfs_node_t *)node->functions.finddir((struct vfs_node *)node, name);
}

// Utils
char *vfs_remove_dot_chars(char *input) {
    list_t *inputList = strtoklist(input, "/", NULL);
    char *ret = list2str(inputList, "/");
    return ret;
}

static void print_vfstree_recur(gentreenode_t *node, int parentOffset) {
    if(!node) return;
    char *tmp = kmalloc(1024);
    int length = 0;
    memset((void *)tmp, 0, 512);
    for(size_t i = 0; i < parentOffset; ++i) {
        strcat(tmp, " ");
    }
    char *current = tmp + strlen(tmp);
    vfs_entry_t *fnode = (vfs_entry_t *)node->val;
    if(fnode->file)
        sprintf(current, "%s(0x%lx, %s)", fnode->name, (size_t)fnode->file, fnode->file->name);
    else
        sprintf(current, "%s(empty)", fnode->name);
    printf("%s\n", tmp);
    length = strlen(fnode->name);
    kfree(tmp);
    foreach(child, node->children) {
        print_vfstree_recur(child->val, parentOffset + length + 1);
    }
}

void print_vfstree() {
    print_vfstree_recur(vfs_tree->root, 0);
}


void vfs_init() {
    vfs_tree = tree_create();
    vfs_entry_t *root = (vfs_entry_t *)kmalloc(sizeof(vfs_entry_t *));
    root->name = strdup("root");
    root->file = NULL;
    tree_insert(vfs_tree, NULL, root);
}

vfs_node_t *get_mountpoint_recur(char **path, gentreenode_t *subroot) {
    bool found = false;
    char delim = '/';
    char *currentToken = strsep(path, &delim);

    if(currentToken == NULL || !strcmp(currentToken, "")) {
        vfs_entry_t *entry = (vfs_entry_t *)subroot->val;
        return entry->file;
    }

    foreach(child, subroot->children) {
        gentreenode_t *tempChild = (gentreenode_t *)child->val;
        vfs_entry_t *entry = (vfs_entry_t *)tempChild->val;
        if(strcmp(entry->name, currentToken) == 0) {
            found = 1;
            subroot = tempChild;
            break;
        }
    }

    if(!found) {
        *path = currentToken;
        return ((vfs_entry_t *)(subroot->val))->file;
    }
    return get_mountpoint_recur(path, subroot);
}

vfs_node_t *get_mountpoint(char **path) {
    if(strlen(*path) > 1 && (*path)[strlen(*path) - 1] == '/')
        *(path)[strlen(*path) - 1] = '\0';
    if(!*path || *(path)[0] != '/') return NULL;
    if(strlen(*path) == 1) {
        *path = '\0';
        vfs_entry_t *entry = (vfs_entry_t *)vfs_tree->root->val;
        return entry->file;
    }
    (*path)++;
    return get_mountpoint_recur(path, vfs_tree->root);
}

vfs_node_t *kopen(const char *fileName, unsigned int flags) {
    char *currentToken = NULL;
    char *filename = strdup(fileName);
    char *freeFilename = filename;
    char *save = strdup(filename);
    char *originalFilename = filename;
    char *newStart = NULL;
    vfs_node_t *nextNode = NULL;
    vfs_node_t *startPoint = get_mountpoint(&filename);
    if(!startPoint) return NULL;
    if(filename)
        newStart = strstr(save + (filename - originalFilename), filename);
    while(filename != NULL && ((currentToken = strsep(&newStart, (const char *)"/")) != NULL)) {
        nextNode = vfs_finddir(startPoint, currentToken);
        if(!nextNode) return NULL;
        startPoint = nextNode;
    }
    if(!nextNode)
        nextNode = startPoint;
    vfs_open(nextNode, flags);
    kfree(save);
    kfree(freeFilename);
    return nextNode;
}

static void vfs_mount_recur(char *path, gentreenode_t *subroot, vfs_node_t *fsNode) {
    bool found = false;
    char *currentToken = strsep(&path, "/");

    if(currentToken == NULL || !strcmp(currentToken, "")) {
        vfs_entry_t *entry = (vfs_entry_t *)subroot->val;
        if(entry->file) {
            printf("Another node already mounted on this path\n");
            return;
        }
        entry->file = fsNode;
        return;
    }

    foreach(child, subroot->children) {
        gentreenode_t *tempChild = (gentreenode_t *)child->val;
        vfs_entry_t *entry = (vfs_entry_t *)tempChild->val;
        if(strcmp(entry->name, currentToken) == 0) {
            found = 1;
            subroot = tempChild;
        }
    }

    if(!found) {
        vfs_entry_t *entry = (vfs_entry_t *)kcalloc(sizeof(vfs_entry_t *), 1);
        entry->name = strdup(currentToken);
        subroot = tree_insert(vfs_tree, subroot, entry);
    }
    vfs_mount_recur(path, subroot, fsNode);
}

void vfs_mount(char *path, vfs_node_t *fsNode) {
    fsNode->refcount = -1;
    if(path[0] == '/' && strlen(path) == 1) {
        vfs_entry_t *entry = (vfs_entry_t *)vfs_tree->root->val;
        if(entry->file) {
            printf("Another node already mounted on this path\n");
            return;
        }
        entry->file = fsNode;
        return;
    }
    vfs_mount_recur(path + 1, vfs_tree->root, fsNode);
}