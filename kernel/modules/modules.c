#include "modules.h"
#include "elf.h"
#include <fs/vfs.h>

#define SYMBOLTABLE_HASHMAP_SIZE 10
#define MODULE_HASHMAP_SIZE 10

static hashmap_t *kernelSymbolTable = NULL;
static hashmap_t *modules = NULL;

typedef struct {
    uint64_t addr;
    char name[];
} kernel_symbol_t;

extern char kernel_symbols_start[];
extern char kernel_symbols_end[];

void *module_load(char *filename) {
    vfs_node_t *node = kopen(filename, 0);

    size_t fileSize = vfs_lseek(node, 0, SEEK_END);
    vfs_lseek(node, 0, SEEK_SET);

    elf64_header_t *elf64hdr = (elf64_header_t *)kmalloc(fileSize);
    vfs_read(node, (char *)elf64hdr, fileSize);
    
    if(elf64hdr->e_ident[EI_MAG0] == ELFMAG0 && elf64hdr->e_ident[EI_MAG1] == ELFMAG1 && elf64hdr->e_ident[EI_MAG2] == ELFMAG2 && elf64hdr->e_ident[EI_MAG3] == ELFMAG3 && elf64hdr->e_ident[EI_CLASS] == ELFCLASS64 && elf64hdr->e_machine == EM_AMD64 && elf64hdr->e_type == ET_REL) {
    
    } else {
        return NULL;
    }

    elf64_shdr_t *shdrTable = (elf64_shdr_t *)((size_t)elf64hdr + elf64hdr->e_shoff);

    char *shstrtab = (char *)((size_t)elf64hdr + shdrTable[elf64hdr->e_shstrndx].sh_offset);

    char *symstrtab = NULL;
    elf64_shdr_t *symShdr = NULL;
    elf64_rela_t *section_rela = NULL;

    for(size_t i = 1; i < elf64hdr->e_shnum; i++) {
        elf64_shdr_t *section = &shdrTable[i];

        uint64_t sectionNameOffset = section->sh_name;

        if(section->sh_type == SHT_STRTAB && (!strcmp(&shstrtab[sectionNameOffset], ".strtab"))) {
            symstrtab = (char *)((size_t)elf64hdr + section->sh_offset);
        }
        
        if(section->sh_type == SHT_SYMTAB) {
            symShdr = section;
        }
    }

    char *deps = NULL;
	size_t depsLength = 0;

    {
        for(size_t i = 1; i < elf64hdr->e_shnum; i++) {
            elf64_shdr_t *section = &shdrTable[i];
            if((!strcmp((char *)((size_t)shstrtab + section->sh_name), "moddeps"))) {
                deps = (char*)((uint64_t)elf64hdr + section->sh_offset);
                depsLength = section->sh_size;

                uint64_t i = 0;
                while(i < depsLength) {
                    if(strlen(&deps[i]) && !hashmap_get(modules, &deps[i])) {
                        printf("\t%s - not loaded", &deps[i]);
                        goto mod_load_err_unload;
                    }
                    printf("\t%s", &deps[i]);
                    i += strlen(&deps[i]) + 1;
                }
            }
        }
    }

    {
        for(size_t i = 1; i < elf64hdr->e_shnum; i++) {
            elf64_shdr_t *section = &shdrTable[i];
            if(section->sh_type == SHT_NOBITS) {
                section->sh_addr = (uint64_t)kmalloc(section->sh_size);
                memset((void *)section->sh_addr, 0x00, section->sh_size);
            } else {
                section->sh_addr = (uint64_t)elf64hdr + section->sh_offset;
            }
        }
    }

    hashmap_t *local_symbols = hashmap_create(10);

    {
        elf64_sym_t *table = (elf64_sym_t *)((size_t)elf64hdr + symShdr->sh_offset);
        while((size_t)table - ((size_t)elf64hdr + symShdr->sh_offset) < symShdr->sh_size) {
            if(table->st_name) {
                if(ELF64_ST_BIND(table->st_info) == STB_GLOBAL) {
                    char *name = (char *)((size_t)symstrtab + table->st_name);
                    if(table->st_shndx == 0) {

                    } else {
                        //printf("\t\t%s: ", name);
                        elf64_shdr_t *s = NULL;
                        {
                            size_t i = 0;
                            for(size_t x = 0; x < elf64hdr->e_shentsize * elf64hdr->e_shnum; x += elf64hdr->e_shentsize) {
                                elf64_shdr_t *shdr = (elf64_shdr_t *)((size_t)elf64hdr + (elf64hdr->e_shoff + x));
                                if(i == table->st_shndx) {
                                    s = shdr;
                                    break;
                                }
                                i++;
                            }
                        }
                        if(s) {
                            uint64_t final = s->sh_addr + table->st_value;
                            /*printf("Final location 0x%lx", final);
                            if (!strcmp(name, "module_name")) {
                                printf(", here goes nothing: %s", final);
                            }
                            printf("\n");*/
                            hashmap_set(kernelSymbolTable, name, (void *)final);
                            hashmap_set(local_symbols, name, (void *)final);
                        }
                    }
                }
            }
            table++;
        }
    }

    for(size_t i = 1; i < elf64hdr->e_shnum; i++) {
        elf64_shdr_t *section = &shdrTable[i];

        if(section->sh_type == SHT_RELA) {
            section_rela = (elf64_rela_t *)(section->sh_addr);
            elf64_rela_t *table = section_rela;
            elf64_sym_t *symtable = (elf64_sym_t *)(symShdr->sh_addr);
            while((size_t)table - (section->sh_addr) < section->sh_size) {
                elf64_sym_t *sym = &symtable[ELF64_R_SYM(table->r_info)];
                elf64_shdr_t *rs = (elf64_shdr_t *)((size_t)elf64hdr + (elf64hdr->e_shoff + section->sh_info * elf64hdr->e_shentsize));
                
                uint64_t addend = 0;
                uint64_t place  = 0;
                uint64_t symbol = 0;
                uint64_t *ptr   = NULL;

                if(ELF64_ST_TYPE(sym->st_info) == STT_SECTION) {
                    elf64_shdr_t *s = (elf64_shdr_t *)((size_t)elf64hdr + (elf64hdr->e_shoff + sym->st_shndx * elf64hdr->e_shentsize));
                    ptr = (uint64_t *)(table->r_offset + rs->sh_addr);
                    addend = table->r_addend;
                    place = (uint64_t)ptr;
                    symbol = s->sh_addr;
                } else {
                    char *name = (char *)((size_t)symstrtab + sym->st_name);
                    //printf("\n\t\t\t\tSymbol name: %s", name);
                    //printf("\n\t\t\t\tSymbol addr: 0x%lx", (size_t)special_thing);
                    ptr = (uint64_t *)(table->r_offset + rs->sh_addr);
                    addend = table->r_addend;
                    place = (uint64_t)ptr;
                    symbol = (uint64_t)hashmap_get(kernelSymbolTable, name);
                }

                switch(ELF64_R_TYPE(table->r_info)) {
                    case 1:
                        *ptr = (uint64_t)(addend + symbol);
                        break;
                    case 2:
                        *((uint32_t *)ptr) = (uint32_t)(addend + symbol - place);
                        break;
                    case 11:
                        *((int32_t *)ptr) = (int32_t)(addend + symbol);
                        break;
                }

                table++;
            }
        }
    }

    printf("Locating module information...\n");
    __attribute__((unused)) module_defines_t *modInfo = NULL;
    list_t *hash_keys = hashmap_keys(local_symbols);
    foreach(_key, hash_keys) {
        char *key = (char *)_key->val;
        if(startswith(key, "module_info_")) {
            modInfo = hashmap_get(local_symbols, key);
        }
    }
    list_destroy(hash_keys);

    if(modInfo == NULL) {
        printf("Module info cannot be found\n");
        return NULL;
    }

    int ret = modInfo->initialize();

    printf("Module %s inited with value %d\n", modInfo->name, ret);

    /*
    hashmap_free(local_symbols);
    kfree(local_symbols);
    */

    module_data_t *modData = kmalloc(sizeof(module_data_t));
    modData->modInfo = modInfo;
    modData->binData = (void *)elf64hdr;
    modData->symbols = local_symbols;
    modData->end = (uint64_t)elf64hdr + fileSize;
    modData->deps = deps;
    modData->depsLength = depsLength;

    hashmap_set(modules, modInfo->name, modData);

    return (void *)modData;

mod_load_err_unload:
    kfree(elf64hdr);
    return (void *)-1;
}

void module_unload(char *name) {

}

void modules_init() {
    kernelSymbolTable = hashmap_create(SYMBOLTABLE_HASHMAP_SIZE);

    kernel_symbol_t *k = (kernel_symbol_t *)&kernel_symbols_start;

    while((size_t)k < (size_t)&kernel_symbols_end) { // I'm assuming the symbol pointert to the symbol is in the kernel space and the module is in the kmalloc space
        hashmap_set(kernelSymbolTable, k->name, (void *)(uint64_t)((uint64_t)k->addr - VIRT_KRNL_BASE + VIRT_PHYS_BASE));
        k = (kernel_symbol_t *)((size_t)k + sizeof(kernel_symbol_t) + strlen(k->name) + 1);
    }

    hashmap_set(kernelSymbolTable, "kernel_symbols_start", &kernel_symbols_start);
	hashmap_set(kernelSymbolTable, "kernel_symbols_end", &kernel_symbols_end);

    modules = hashmap_create(MODULE_HASHMAP_SIZE);
}

hashmap_t *modules_get_list() {
	return modules;
}

hashmap_t *modules_get_symbols() {
	return kernelSymbolTable;
}