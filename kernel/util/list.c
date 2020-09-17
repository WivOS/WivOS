#include "list.h"

list_t *list_create() {
    list_t *list = (list_t *)kcalloc(sizeof(list_t *), 1);
    list->head = NULL;
    list->tail = NULL;
    list->listLength = 0;
    return list;
}

size_t list_size(list_t *list) {
    if(!list) return 0;
	return list->listLength;
}

lnode_t *list_insert_front(list_t *list, void *value) {
    if(!list) return NULL;
    lnode_t *newNode = (lnode_t *)kcalloc(sizeof(lnode_t *), 1);
    if(!list->head) {
        list->tail = newNode;
    } else {
        list->head->prev = newNode;
    }

    newNode->next = list->head;
    newNode->prev = NULL;
    newNode->val = value;

    list->head = newNode;
    list->listLength++;
    return newNode;
}

lnode_t *list_insert_back(list_t *list, void *value) {
    if(!list) return NULL;
    lnode_t *newNode = (lnode_t *)kcalloc(sizeof(lnode_t *), 1);
    newNode->next = NULL;
    newNode->prev = list->tail;
    newNode->val = value;

    if(list->tail)
        list->tail->next = newNode;
    list->tail = newNode;
    
    if(!list->head) {
        list->head = newNode;
    }

    list->listLength++;
    return newNode;
}

void *list_remove_front(list_t *list) {
    if(!list || !list->head) return NULL;
    void *val = list->head->val;

    lnode_t *oldHead = list->head;
    list->head = oldHead->next;
    if(list->head)
        list->head->prev = NULL;

    kfree((void *)oldHead);
    list->listLength--;
    return val;
}

void *list_remove_back(list_t *list) {
    if(!list || !list->head) return NULL;
    void *val = list->tail->val;

    lnode_t *oldTail = list->tail;
    list->tail = oldTail->prev;
    if(list->tail)
        list->tail->next = NULL;

    kfree((void *)oldTail);
    list->listLength--;
    return val;
}

void *list_remove_node(list_t *list, lnode_t *node) {
    if(!list || !node) return NULL;
    void *value = node->val;
    if(node == list->head)
        list_remove_front(list);
    else if(node == list->tail)
        list_remove_back(list);
    else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
        list->listLength--;
        kfree((void *)node);
    }
    return value;
}

lnode_t *list_pop(list_t *list) {
    if(!list || !list->head) return NULL;
    lnode_t *oldTail = list->tail;
    list->tail = oldTail->prev;
    if(list->tail)
        list->tail->next = NULL;

    list->listLength--;
    return oldTail;
}

void *list_get_front(list_t *list) {
    if(!list || !list->head) return NULL;
    return list->head->val;
}

void *list_get_back(list_t *list) {
    if(!list || !list->tail) return NULL;
    return list->tail->val;
}

size_t list_contain(list_t *list, void *value) {
    size_t index = 0;
    foreach(listNode, list) {
        if(listNode->val == value)
            return index;
        index++;
    }
    return -1;
}

lnode_t *list_get_node_by_index(list_t *list, size_t index) {
    if(index < 0 || index >= list_size(list)) return NULL;
    size_t current = 0;
    foreach(listNode, list) {
        if(index == current) return listNode;
        current++;
    }
    return NULL;
}

void *list_remove_index(list_t *list, size_t index) {
    lnode_t *node = list_get_node_by_index(list, index);
    return list_remove_node(list, node);
}

void list_destroy(list_t *list) {
    if(!list) return;
    lnode_t *node = list->head;
    while(node != NULL) {
        lnode_t *nodeSave = node;
        node = node->next;
        kfree((void *)node);
    }
    kfree((void *)list);
}

void lnode_destroy(lnode_t *node) {
    kfree((void *)node);
}


gentree_t *tree_create() {
    return (gentree_t *)kcalloc(sizeof(gentree_t *), 1);
}

gentreenode_t *treenode_create(void *value) {
    gentreenode_t *node = (gentreenode_t *)kcalloc(sizeof(gentreenode_t *), 1);
    node->val = value;
    node->children = list_create();
    return node;
}

gentreenode_t *tree_insert(gentree_t *tree, gentreenode_t *subroot, void *value) {
    gentreenode_t *treeNode = (gentreenode_t *)kcalloc(sizeof(gentreenode_t *), 1);
    treeNode->children = list_create();
    treeNode->val = value;

    if(!tree->root)
        tree->root = treeNode;
    else
        list_insert_front(subroot->children, treeNode);

    return treeNode;
}

gentreenode_t *tree_find_parent(gentree_t *tree, gentreenode_t *removeNode, size_t *childIndex) {
    if(removeNode == tree->root) return NULL;
    return tree_find_parent_recur(tree, removeNode, tree->root, childIndex);
}

gentreenode_t *tree_find_parent_recur(gentree_t *tree, gentreenode_t *removeNode, gentreenode_t *subroot, size_t *childIndex) {
    int index;
    if((index = list_contain(subroot->children, removeNode)) != -1) {
        *childIndex = index;
        return subroot;
    }
    foreach(child, subroot->children) {
        gentreenode_t *ret = tree_find_parent_recur(tree, removeNode, child->val, childIndex);
        if(ret != NULL) return ret;
    }
    return NULL;
}

void tree_remove(gentree_t *tree, gentreenode_t *removeNode) {
    size_t childIndex = -1;
    gentreenode_t *parent = tree_find_parent(tree, removeNode, &childIndex);
    if(parent != NULL) {
        gentreenode_t *toFree = list_remove_index(parent->children, childIndex);
        kfree(toFree);
    }
}

void tree2list_recur(gentreenode_t *subroot, list_t *list) {
    if(subroot == NULL) return;
    foreach(child, subroot->children) {
        gentreenode_t *currentTreeNode = (gentreenode_t *)child->val;
        void *currentValue = currentTreeNode->val;
        list_insert_back(list, currentValue);
        tree2list_recur(child->val, list);
    }
}

void tree2list(gentree_t *tree, list_t *list) {
    tree2list_recur(tree->root, list);
}

void tree2array_recur(gentreenode_t *subroot, void **array, size_t *size) {
    if(subroot == NULL) return;
    void *currentValue = (void *)subroot->val;
    array[*size] = currentValue;
    *size = *size + 1;
    foreach(child, subroot->children) {
        tree2array_recur(child->val, array, size);
    }
}

void tree2array(gentree_t *tree, void **array, size_t *size) {
    tree2array_recur(tree->root, array, size);
}