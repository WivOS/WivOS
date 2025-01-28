#pragma once

#include <utils/common.h>
#include <utils/lists.h>

#include <fs/vfs.h>

typedef struct {
    char *name;
    size_t (*init)(void);
    size_t (*exit)(void);
} module_info_t;

typedef struct {
    module_info_t *info;
    void *data;
    //TODO
} module_data_t;

#define MODULE_DEPENDS(n) \
    static char _mod_dependency_ ## n [] __attribute__((section("moddeps"), used)) = #n

#define MODULE_DEF(n, finit, fexit) \
    module_info_t module_info_ ## n = { \
        .name = #n, \
        .init = &(finit), \
        .exit = &(fexit) \
    }

extern hashmap_t *KernelSymbolsHashmap;

void modules_init();

bool module_load(char *path);

typedef struct {
    uint64_t phdr;
    uint64_t phent;
    uint64_t phnum;
    uint64_t entry;
} elf_value_t;

bool elf_load(vfs_node_t *node, void *pml4, size_t base, elf_value_t *value, char **out_ld_path);
bool exec(size_t pid, const char *filename, const char *argv[], const char *envp[]);
bool execve(size_t pid, const char *filename, const char *argv[], const char *envp[]);