#include "lists.h"

list_t *list_create() {
    list_t *list = (list_t *)kmalloc(sizeof(list_t));
    list->root = NULL;
    list->tail = NULL;
    list->length = 0;
    list->lock = INIT_SPINLOCK();

    return list;
}

void list_destroy(list_t *list) {
    list_node_t *node = list->root;
    while(node != NULL) {
        list_node_t *nodeToDestroy = node;
        node = node->next;
        kfree((void *)nodeToDestroy);
    }
    kfree((void *)list);
}

void list_destroy_node(list_node_t *node) {
    kfree((void *)node);
}

list_node_t *list_push_before(list_t *list, list_node_t *lastNode, void *value) {
    spinlock_lock(&list->lock);

    list_node_t *node = (list_node_t *)kmalloc(sizeof(list_node_t));

    node->value = value;
    node->previous = list->tail;
    node->next = lastNode;

    if(list->tail == lastNode) {
        node->previous = NULL;
        list->tail = node;
    }
    lastNode->previous = node;

    list->length++;

    spinlock_unlock(&list->lock);

    return node;
}

list_node_t *list_push_back(list_t *list, void *value) {
    spinlock_lock(&list->lock);

    list_node_t *node = (list_node_t *)kmalloc(sizeof(list_node_t));

    node->value = value;
    node->previous = list->tail;
    node->next = NULL;

    if(list->tail) list->tail->next = node;
    list->tail = node;

    if(!list->root) list->root = node;

    list->length++;

    spinlock_unlock(&list->lock);

    return node;
}

list_node_t *list_push_front(list_t *list, void *value) {
    spinlock_lock(&list->lock);

    list_node_t *node = (list_node_t *)kmalloc(sizeof(list_node_t));

    node->value = value;
    node->previous = NULL;
    node->next = list->root;

    if(list->root) list->root->previous = node;
    else list->tail = node;
    list->root = node;

    list->length++;

    spinlock_unlock(&list->lock);

    return node;
}

void *list_remove_back(list_t *list) {
    spinlock_lock(&list->lock);

    if(!list->tail) {
        spinlock_unlock(&list->lock);
        return NULL;
    }

    list_node_t *node = list->tail;
    void *value = node->value;

    list->tail = node->previous;
    if(list->tail) list->tail->next = NULL;
    else list->root = NULL; //We are done

    kfree((void *)node);

    list->length--;

    spinlock_unlock(&list->lock);

    return value;
}

void *list_remove_front(list_t *list) {
    spinlock_lock(&list->lock);

    if(!list->root) {
        spinlock_unlock(&list->lock);
        return NULL;
    }

    list_node_t *node = list->root;
    void *value = node->value;

    list->root = node->next;
    if(list->root) list->root->previous = NULL;
    else list->tail = NULL; //We are done

    kfree((void *)node);

    list->length--;

    spinlock_unlock(&list->lock);

    return value;
}

list_node_t *list_pop(list_t *list) {
    spinlock_lock(&list->lock);

    if(!list->tail) {
        spinlock_unlock(&list->lock);
        return NULL;
    }

    list_node_t *node = list->tail;

    list->tail = node->previous;
    if(list->tail) list->tail->next = NULL;
    else list->root = NULL; //We are done

    list->length--;

    spinlock_unlock(&list->lock);

    return node;
}

void *list_remove(list_t *list, list_node_t *node) {
    spinlock_lock(&list->lock);

    void *value = node->value;

    if(node == list->root) {
        spinlock_unlock(&list->lock);
        list_remove_front(list);
    }
    else if(node == list->tail) {
        spinlock_unlock(&list->lock);
        list_remove_back(list);
    }
    else {
        node->previous->next = node->next;
        node->next->previous = node->previous;
        list->length--;
        kfree((void *)node);
        spinlock_unlock(&list->lock);
    }

    return value;
}

void *list_get_back(list_t *list) {
    spinlock_lock(&list->lock);

    if(!list->tail) {
        spinlock_unlock(&list->lock);
        return NULL;
    }

    void *value = list->tail->value;

    spinlock_unlock(&list->lock);

    return value;
}

void *list_get_front(list_t *list) {
    spinlock_lock(&list->lock);

    if(!list->root) {
        spinlock_unlock(&list->lock);
        return NULL;
    }

    void *value = list->root->value;

    spinlock_unlock(&list->lock);

    return value;
}

size_t list_contains(list_t *list, void *value) {
    spinlock_lock(&list->lock);

    size_t index = 0;
    foreach(node, list) {
        if(node->value == value)
            break;
        index++;
    }

    spinlock_unlock(&list->lock);

    return index;
}

list_node_t *list_get_indexed(list_t *list, size_t index) {
    spinlock_lock(&list->lock);

    size_t currentIndex = 0;
    foreach(node, list) {
        if(currentIndex == index) {
            spinlock_unlock(&list->lock);
            return node;
        }
        currentIndex++;
    }

    spinlock_unlock(&list->lock);

    return NULL;
}

void *list_remove_indexed(list_t *list, size_t index) {
    list_node_t *node = list_get_indexed(list, index);
    return list_remove(list, node);
}


gentree_t *gentree_create() {
    gentree_t *tree = (gentree_t *)kmalloc(sizeof(gentree_t));
    tree->root = NULL;
    tree->lock = INIT_SPINLOCK();
    return tree;
}

gentree_node_t *gentree_node_create(void *value) {
    gentree_node_t *node = (gentree_node_t *)kmalloc(sizeof(gentree_node_t));
    node->value = value;
    node->children = list_create();
    return node;
}

gentree_node_t *gentree_insert(gentree_t *tree, gentree_node_t *subroot, void *value) {
    spinlock_lock(&tree->lock);

    gentree_node_t *node = gentree_node_create(value);

    if(!tree->root) tree->root = node;
    else list_push_front(subroot->children, node);

    spinlock_unlock(&tree->lock);

    return node;
}

static gentree_node_t *gentree_get_parent_recursive(gentree_t *tree, gentree_node_t *node, gentree_node_t *subroot, size_t *index) {
    size_t currentIndex = 0;
    if((currentIndex = list_contains(subroot->children, node)) != -1) {
        if(index) *index = currentIndex;
        return subroot;
    }

    spinlock_lock(&subroot->children->lock);
    foreach(child, subroot->children) {
        spinlock_unlock(&subroot->children->lock);
        gentree_node_t *returnNode = gentree_get_parent_recursive(tree, node, (gentree_node_t *)child->value, index);
        spinlock_lock(&subroot->children->lock);
        if(returnNode != NULL) {
            spinlock_unlock(&subroot->children->lock);
            return returnNode;
        }
    }
    spinlock_unlock(&subroot->children->lock);

    return NULL;
}

gentree_node_t *gentree_get_parent(gentree_t *tree, gentree_node_t *node, size_t *index) {
    spinlock_lock(&tree->lock);

    if(node == tree->root) {
        spinlock_unlock(&tree->lock);
        return NULL;
    }

    spinlock_unlock(&tree->lock);
    return gentree_get_parent_recursive(tree, node, tree->root, index);
}

void gentree_remove(gentree_t *tree, gentree_node_t *node) {
    size_t index = -1;
    gentree_node_t *parent = gentree_get_parent(tree, node, &index);
    if(parent != NULL) {
        gentree_node_t *nodeToFree = list_remove_indexed(parent->children, index);
        kfree((void *)nodeToFree);
    }
}


uint32_t hashmap_string_hash(void *_key) {
	uint32_t hash = 0;
	char * key = (char *)_key;
	int c;
	while ((c = *key++)) {
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

hashmap_t *hashmap_create(size_t hashSize) {
    hashmap_t *hashmap = (hashmap_t *)kmalloc(sizeof(hashmap_t));

    hashmap->hashSize = hashSize;
    hashmap->hashmap = kmalloc(sizeof(hashmap_entry_t *) * hashSize);

    hashmap->lock = INIT_SPINLOCK();

    return hashmap;
}

void hashmap_delete(hashmap_t *map) {
    spinlock_lock(&map->lock);

    for(size_t i = 0; i < map->hashSize; i++) {
        hashmap_entry_t *entry = map->hashmap[i];
        while(entry) {
            hashmap_entry_t *currEntry = entry;
            entry = entry->next;

            kfree(currEntry->key);
            kfree(currEntry);
        }
    }

    kfree(map);
}

void *hashmap_set(hashmap_t *map, char *key, void *value) {
    uint32_t hash = hashmap_string_hash(key);

    spinlock_lock(&map->lock);

    hashmap_entry_t *entry = map->hashmap[hash % map->hashSize];
    if(entry) {
        hashmap_entry_t *lastEntry = entry;
        while(entry) {
            //printf("%llx %llx %llx %llx\n", entry->key, entry, lastEntry, entry->next);
            lastEntry = entry;
            if(!strcmp(entry->key, key)) {
                void *valueOut = entry->value;
                entry->value = value;

                spinlock_unlock(&map->lock);
                return valueOut;
            }
            entry = entry->next;
        }

        entry = kmalloc(sizeof(hashmap_entry_t));
        entry->key = strdup(key);
        entry->next = NULL;
        entry->value = value;

        lastEntry->next = entry;
        spinlock_unlock(&map->lock);
        return NULL;
    }
    entry = kmalloc(sizeof(hashmap_entry_t));
    entry->key = strdup(key);
    entry->next = NULL;
    entry->value = value;

    map->hashmap[hash % map->hashSize] = entry;

    spinlock_unlock(&map->lock);

    return NULL;
}

void *hashmap_get(hashmap_t *map, char *key) {
    uint64_t hash = hashmap_string_hash(key);

    spinlock_lock(&map->lock);

    hashmap_entry_t *entry = map->hashmap[hash % map->hashSize];
    if(entry) {
        while(entry) {
            if(!strcmp(entry->key, key)) {
                void *valueOut = entry->value;

                spinlock_unlock(&map->lock);
                return valueOut;
            }
            entry = entry->next;
        }
    }

    spinlock_unlock(&map->lock);
    return NULL;
}

void *hashmap_remove(hashmap_t *map, char *key) {
    uint64_t hash = hashmap_string_hash(key);

    spinlock_lock(&map->lock);

    hashmap_entry_t *entry = map->hashmap[hash % map->hashSize];
    if(entry) {
        hashmap_entry_t *lastEntry = NULL;
        while(entry) {
            if(!strcmp(entry->key, key)) {
                void *valueOut = entry->value;

                if(lastEntry) lastEntry->next = entry->next;
                else {
                    map->hashmap[hash % map->hashSize] = entry->next;
                }

                kfree(entry->key);
                kfree(entry);

                spinlock_unlock(&map->lock);
                return valueOut;
            }
            lastEntry = entry;
            entry = entry->next;
        }
    }

    spinlock_unlock(&map->lock);
    return NULL;
}

bool hashmap_has(hashmap_t *map, char *key) {
    uint64_t hash = hashmap_string_hash(key);

    spinlock_lock(&map->lock);

    hashmap_entry_t *entry = map->hashmap[hash % map->hashSize];
    if(entry) {
        while(entry) {
            if(!strcmp(entry->key, key)) {
                spinlock_unlock(&map->lock);
                return true;
            }
            entry = entry->next;
        }
    }

    spinlock_unlock(&map->lock);
    return false;
}

list_t *hashmap_to_list(hashmap_t *map) {
    spinlock_lock(&map->lock);

    list_t *list = list_create();

    for(size_t i = 0; i < map->hashSize; i++) {
        hashmap_entry_t *entry = map->hashmap[i];
        while(entry) {
            list_push_back(list, entry->key);
            entry = entry->next;
        }
    }

    spinlock_unlock(&map->lock);
    return list;
}