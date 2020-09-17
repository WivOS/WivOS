#include <fs/vfs.h>

gentree_t *vfs_tree;

// Function wrappers
size_t vfs_read(vfs_node_t *node, char *buffer, size_t offset, size_t size) {
    if(!node || !node->functions.read) return -1;
    return node->functions.read((struct vfs_node_t *)node, buffer, offset, size);
}

size_t vfs_write(vfs_node_t *node, char *buffer, size_t offset, size_t size) {
    if(!node || !node->functions.read) return -1;
    return node->functions.write((struct vfs_node_t *)node, buffer, offset, size);
}

void vfs_open(vfs_node_t *node, uint32_t flags) {
    if(!node || !node->functions.open) return;
    if(node->refcount >= 0) node->refcount++;
    node->functions.open((struct vfs_node_t *)node, flags);
}
void vfs_close(vfs_node_t *node) {
    if(!node || !node->functions.close) return -1;
    node->refcount--;
    if(node->refcount == 0) node->functions.close((struct vfs_node_t *)node);
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
    } else {
        //TODO
    }
}