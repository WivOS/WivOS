#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "common.h"
#include "spinlock.h"

typedef struct list_node
{
    void *value;
    struct list_node *previous;
    struct list_node *next;
} list_node_t;

typedef struct {
    size_t length;
    list_node_t *root;
    list_node_t *tail;

    volatile spinlock_t lock;
} list_t;

list_t *list_create();
void list_destroy(list_t *list);
void list_destroy_node(list_node_t *node);

list_node_t *list_push_before(list_t *list, list_node_t *lastNode, void *value);
list_node_t *list_push_back(list_t *list, void *value);
list_node_t *list_push_front(list_t *list, void *value);

void *list_remove_back(list_t *list);
void *list_remove_front(list_t *list);

list_node_t *list_pop(list_t *list);

void *list_remove(list_t *list, list_node_t *node);

void *list_get_back(list_t *list);
void *list_get_front(list_t *list);

size_t list_contains(list_t *list, void *value);

list_node_t *list_get_indexed(list_t *list, size_t index);
void *list_remove_indexed(list_t *list, size_t index);

#define foreach(value, list) for(list_node_t *value = list->root; value != NULL; value = value->next)

typedef struct
{
    void *value;
    list_t *children;
} gentree_node_t;

typedef struct {
    gentree_node_t *root;
    volatile spinlock_t lock;
} gentree_t;

gentree_t *gentree_create();
gentree_node_t *gentree_node_create(void *value);
gentree_node_t *gentree_insert(gentree_t *tree, gentree_node_t *subroot, void *value);

gentree_node_t *gentree_get_parent(gentree_t *tree, gentree_node_t *node, size_t *index);

void gentree_remove(gentree_t *tree, gentree_node_t *node);

typedef struct hashmap_entry {
    char *key;
    void *value;

    struct hashmap_entry *next;
} __attribute__((packed)) hashmap_entry_t;

typedef struct {
    volatile spinlock_t lock;
    size_t hashSize;

    hashmap_entry_t **hashmap;
} hashmap_t;

hashmap_t *hashmap_create(size_t hashSize);
void hashmap_delete(hashmap_t *map);

void *hashmap_set(hashmap_t *map, char *key, void *value);
void *hashmap_get(hashmap_t *map, char *key);
void *hashmap_remove(hashmap_t *map, char *key);

bool hashmap_has(hashmap_t *map, char *key);
list_t *hashmap_to_list(hashmap_t *map);