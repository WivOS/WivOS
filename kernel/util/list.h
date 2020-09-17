#pragma once

#include <stddef.h>

typedef struct lnode {
    void *val;
    struct lnode *prev;
    struct lnode *next;
} lnode_t;

typedef struct list {
    lnode_t *head;
    lnode_t *tail;
    size_t listLength;
} list_t;

#include <util/util.h>

list_t *list_create();

size_t list_size(list_t *list);

lnode_t *list_insert_front(list_t *list, void *value);
lnode_t *list_insert_back(list_t *list, void *value);
void *list_remove_front(list_t *list);
void *list_remove_back(list_t *list);
void *list_remove_node(list_t *list, lnode_t *node);

lnode_t *list_pop(list_t *list);

void *list_get_front(list_t *list);
void *list_get_back(list_t *list);

size_t list_contain(list_t *list, void *value);
lnode_t *list_get_node_by_index(list_t *list, size_t index);
void *list_remove_index(list_t *list, size_t index);
void list_destroy(list_t *list);
void lnode_destroy(lnode_t *node);

#define foreach(value, list) for(lnode_t *value = list->head; value != NULL; value = value->next)

typedef struct gentreenode {
    list_t *children;
    void *val;
} gentreenode_t;

typedef struct gentree {
    gentreenode_t *root;
} gentree_t;

gentree_t *tree_create();
gentreenode_t *treenode_create(void *value);
gentreenode_t *tree_insert(gentree_t *tree, gentreenode_t *subroot, void *value);
void tree_remove(gentree_t *tree, gentreenode_t *removeNode);

gentreenode_t *tree_find_parent(gentree_t *tree, gentreenode_t *removeNode, size_t *childIndex);
gentreenode_t *tree_find_parent_recur(gentree_t *tree, gentreenode_t *removeNode, gentreenode_t *subroot, size_t *childIndex);

void tree2list_recur(gentreenode_t *subroot, list_t *list);
void tree2list(gentree_t *tree, list_t *list);

void tree2array_recur(gentreenode_t *subroot, void **array, size_t *size);
void tree2array(gentree_t *tree, void **array, size_t *size);