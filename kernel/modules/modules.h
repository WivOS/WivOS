#pragma once

#include <util/util.h>

typedef struct {
    char *name;
    int (* initialize)(void);
    int (* finalize)(void);
} module_defines_t;

typedef struct {
    module_defines_t *modInfo;
    void *binData;
    hashmap_t *symbols;
    uint64_t end;
    size_t depsLength;
    char *deps;
} module_data_t;

void *module_load(char *filename);
void module_unload(char *moduleName);
void modules_init();

hashmap_t *modules_get_list();
hashmap_t *modules_get_symbols();

#define MODULE_DEPENDS(n) \
    static char _mod_dependency_ ## n [] __attribute__((section("moddeps"), used)) = #n

#define MODULE_DEF(n, init, fini) \
    module_defines_t module_info_ ## n = { \
        .name = #n, \
        .initialize = &init, \
        .finalize = &fini \
    }
