#include <modules/modules.h>
#include <utils/common.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

#include "shm.h"

char module_name[] = "shm";

static spinlock_t SHMLock = INIT_SPINLOCK();
gentree_t *SHMTree = NULL;

static shm_node_t *_get_node(char *shmPath, bool create, gentree_node_t *from) {
    char *pch, *save;
    pch = strtok_r(shmPath, ".", &save);

    gentree_node_t *tnode = from;
    foreach(node, tnode->children) {
        gentree_node_t *_node = (gentree_node_t *)node->value;
        shm_node_t *snode = (shm_node_t *)_node->value;

        if(!strcmp(snode->name, pch)) {
            if(*save == '\0') return snode;
            return _get_node(save, create, _node);
        }
    }

    if(!create) return NULL;

    shm_node_t *nsnode = kmalloc(sizeof(shm_node_t));
    memcpy(nsnode->name, pch, strlen(pch) + 1);
    nsnode->chunk = NULL;

    gentree_node_t *nnode = gentree_insert(SHMTree, from, nsnode);

    if(*save == '\0') return nsnode;

    return _get_node(save, create, nnode);
}

static shm_node_t *get_node(char *shmPath, bool create) {
    char *_path = kmalloc(strlen(shmPath) + 1);
    memcpy(_path, shmPath, strlen(shmPath) + 1);

    shm_node_t *node = _get_node(_path, create, SHMTree->root);

    kfree(_path);
    return node;
}

static shm_chunk_t *create_chunk(shm_node_t *parent, size_t size) {
    if(!size) return NULL;

    shm_chunk_t *chunk = (shm_chunk_t *)kmalloc(sizeof(shm_chunk_t));
    if(chunk == NULL) return NULL;

    chunk->parent = parent;
    chunk->lock = INIT_SPINLOCK();
    chunk->ref_count = 1;

    chunk->pageCount = ROUND_UP(size, PAGE_SIZE) / PAGE_SIZE;

    chunk->phys = pmm_alloc(chunk->pageCount);

    return chunk;
}

static uint64_t shm_sbrk(size_t count, process_t *proc) {
    uint64_t initial = proc->shm_alloc;

    if(initial & (PAGE_SIZE - 1)) {
        initial += PAGE_SIZE - (initial & (PAGE_SIZE - 1));
        proc->shm_alloc = initial;
    }
    proc->shm_alloc += count << 12;

    return initial;
}

static void *shm_map(shm_chunk_t *chunk, process_t *proc) {
    if(!chunk) return NULL;

    shm_mapping_t *mapping = (shm_mapping_t *)kmalloc(sizeof(shm_mapping_t));
    mapping->chunk = chunk;
    mapping->lock = INIT_SPINLOCK();

    uint64_t lastAddress = USER_SHM_LOW;
    if(proc->shm_map == NULL) proc->shm_map = list_create();
    else {
        //Find holes
        foreach(node, proc->shm_map) {
            shm_mapping_t *m = (shm_mapping_t *)node->value;
            if((uint64_t)m->virt > lastAddress) {
                size_t space = (size_t)m->virt - lastAddress;
                if(space >= chunk->pageCount * PAGE_SIZE) {
                    mapping->virt = (void *)lastAddress;
                    vmm_map((pt_t *)proc->page_table, mapping->virt, chunk->phys, chunk->pageCount, 0x7);

                    list_push_before((list_t *)proc->shm_map, node, mapping);

                    return mapping->virt;
                }
            }

            lastAddress = (uint64_t)m->virt + m->chunk->pageCount * PAGE_SIZE;
        }
    }

    if(proc->shm_alloc > lastAddress) {
        size_t space = proc->shm_alloc - lastAddress;
        if(space >= chunk->pageCount * PAGE_SIZE) {
            mapping->virt = (void *)lastAddress;
            vmm_map((pt_t *)proc->page_table, mapping->virt, chunk->phys, chunk->pageCount, 0x7);

            list_push_back((list_t *)proc->shm_map, mapping);

            return mapping->virt;
        }
    }

    uint64_t new_page = shm_sbrk(chunk->pageCount, proc);
    mapping->virt = (void *)new_page;
    vmm_map((pt_t *)proc->page_table, mapping->virt, chunk->phys, chunk->pageCount, 0x7);

    list_push_back((list_t *)proc->shm_map, mapping);

    return mapping->virt;
}

static void shm_release_chunk(shm_chunk_t *chunk) {
    if(chunk == NULL) return;

    chunk->ref_count--;

    if(chunk->ref_count <= 0) {
        pmm_free(chunk->phys, chunk->pageCount);

        chunk->parent->chunk = NULL;
        kfree(chunk);
    }
}

void *shm_obtain(char *path, size_t *size) {
    spinlock_lock(&SHMLock);

    process_t *proc = SchedulerProcesses[scheduler_get_current_pid()];

    shm_node_t *node = get_node(path, true);
    shm_chunk_t *chunk = node->chunk;

    if(chunk == NULL) {
        if(!size) {
            spinlock_unlock(&SHMLock);
            return NULL;
        }

        chunk = create_chunk(node, *size);
        if(chunk == NULL) {
            spinlock_unlock(&SHMLock);
            return NULL;
        }

        node->chunk = chunk;
    } else {
        chunk->ref_count++;
    }

    //Map
    void *virtStart = shm_map(chunk, proc);
    *size = chunk->pageCount * PAGE_SIZE;

    spinlock_unlock(&SHMLock);

    return virtStart;
}

size_t shm_release(char *path) {
    spinlock_lock(&SHMLock);

    process_t *proc = SchedulerProcesses[scheduler_get_current_pid()];

    shm_node_t *shmNode = get_node(path, false);
    if(shmNode == NULL) {
        spinlock_unlock(&SHMLock);
        return -1;
    }
    shm_chunk_t *chunk = shmNode->chunk;

    list_node_t *node = NULL;
    foreach(n, proc->shm_map) {
        shm_mapping_t *m = (shm_mapping_t *)n->value;
        if(m->chunk == chunk) {
            node = n;
            break;
        }
    }
    if(node == NULL) {
        spinlock_unlock(&SHMLock);
        return -1;
    }

    shm_mapping_t *mapping = (shm_mapping_t *)node->value;

    vmm_unmap((pt_t *)proc->page_table, mapping->virt, mapping->chunk->pageCount);

    shm_release_chunk(chunk);
    list_remove((list_t *)proc->shm_map, node);
    kfree(mapping);

    spinlock_unlock(&SHMLock);
    return 0;
}

size_t syscall_shm_obtain(irq_regs_t *regs) {
    char *path = (char *)regs->rdi;
    size_t *size = (size_t *)regs->rsi;

    if(syscall_privilege_check((void *)path, strlen(path) + 1)) return -1;
    if(syscall_privilege_check((void *)size, sizeof(size_t))) return -1;

    return (size_t)shm_obtain(path, size);
}

size_t syscall_shm_release(irq_regs_t *regs) {
    char *path = (char *)regs->rdi;

    if(syscall_privilege_check((void *)path, strlen(path) + 1)) return -1;

    return (size_t)shm_release(path);
}

static size_t _init() {
    printf("Initing shm module\n");

    SHMTree = gentree_create();
    gentree_insert(SHMTree, NULL, NULL);

    if(!syscall_install(syscall_shm_obtain, SYSCALL_ID('S', 0x00)) || //SHM goes to S module
       !syscall_install(syscall_shm_release, SYSCALL_ID('S', 0x01))) {
        printf("[SHM] Failed to assign syscalls\n");
    }

    return 0;
}

static size_t _exit() {
    return 0;
}

MODULE_DEF(shm, _init, _exit);