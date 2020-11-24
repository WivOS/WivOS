#include <modules/modules.h>

extern int a_function(void);

static int _init() {
    printf("Initing test2 module\n");
    printf("List of loaded modules before this:\n");
    list_t *modulesList = hashmap_values(modules_get_list());
    foreach(_key, modulesList) {
        module_data_t *modData = (module_data_t *)_key->val;
        printf("\t%s at 0x%lx {.init=0x%lx, .fini=0x%lx}", modData->modInfo->name, modData->binData, modData->modInfo->initialize, modData->modInfo->finalize);
    
        if(modData->deps) {
            uint64_t i = 0;
            printf(" Deps: ");
            while(i < modData->depsLength) {
                printf("%s ", &modData->deps[i]);
                i += strlen(&modData->deps[i]) + 1;
            }
        }

        printf("\n");
    }
    return a_function();
}

static int _exit() {
    printf("Goodbye :D\n");
    return 0;
}

MODULE_DEF(test2, _init, _exit);
MODULE_DEPENDS(test);