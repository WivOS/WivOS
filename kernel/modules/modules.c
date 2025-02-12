#include "modules.h"

#include <fs/vfs.h>
#include <utils/lists.h>

#include <mem/pmm.h>
#include <mem/vmm.h>
#include <cpu/cpu.h>

#include <tasking/scheduler.h>

typedef struct {
    uint64_t addr;
    uint64_t size;
    char name[];
} kernel_symbol_t;

extern char kernel_symbols_start[];
extern char kernel_symbols_end[];

hashmap_t *KernelSymbolsHashmap = NULL;

typedef struct {
    uint32_t magic;
    uint8_t x86_64_flag;
    uint8_t big_endian_flag;
    uint8_t elf_header_version;
    uint8_t os_abi;
    uint64_t padding;
    uint16_t executable_and_core_flag;
    uint16_t instruction_set;
    uint32_t elf_version;
    uint64_t entry_address;
    uint64_t program_header_table_address;
    uint64_t section_header_table_address;
    uint32_t flags;
    uint16_t header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t section_names_index;
} __attribute__((packed)) elf64_header_t;

typedef struct {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t address;
    uint64_t file_offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entry_size;
} __attribute__((packed)) elf64_section_header_t;

typedef struct {
    uint32_t name;
    uint8_t info;
    uint8_t other;
    uint16_t section_table_index;
    uint64_t value;
    uint64_t size;
} __attribute__((packed)) elf64_symbol_t;

typedef struct {
    uint64_t offset;
    uint64_t info;
    int64_t addend;
} __attribute__((packed)) elf64_rela_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t file_offset;
    uint64_t virtual_address;
    uint64_t physical_address;
    uint64_t file_size;
    uint64_t memory_size;
    uint64_t align;
} __attribute__((packed)) elf64_program_header_t;

bool module_load_node(vfs_node_t *elfNode) __attribute__((no_sanitize("alignment"))) {
    vfs_lseek(elfNode, 0, SEEK_SET);

    elf64_header_t header;
    vfs_read(elfNode, (char *)&header, sizeof(elf64_header_t));

    if(header.magic != 0x464C457F || header.x86_64_flag != 0x2 ||
       header.big_endian_flag != 0x1 || header.instruction_set != 0x3E) {
        vfs_close(elfNode);
        kfree(elfNode);
        printf("asa2\n");
        return false;
    }

    elf64_section_header_t *section_table = (elf64_section_header_t *)kmalloc(header.section_header_entry_count * header.section_header_entry_size);
    vfs_lseek(elfNode, header.section_header_table_address, SEEK_SET);
    vfs_read(elfNode, (char *)section_table, header.section_header_entry_count * header.section_header_entry_size);

    elf64_section_header_t shstrtab_header = section_table[header.section_names_index];
    char *shstrtab = (char *)kmalloc(shstrtab_header.size);

    vfs_lseek(elfNode, shstrtab_header.file_offset, SEEK_SET);
    vfs_read(elfNode, (char *)shstrtab, shstrtab_header.size);

    char *symstrtab = NULL;
    elf64_section_header_t symShdr = {0};
    for(int i = 1; i < header.section_header_entry_count; i++) {
        elf64_section_header_t sheader = section_table[i];
        //printf("[%s]\t\tType: 0x%x, Size: 0x%x\n", &shstrtab[sheader.name], sheader.type, sheader.size);

        if(sheader.type == 0x3 && !strcmp(&shstrtab[sheader.name], ".strtab")) {
            symstrtab = (char *)kmalloc(sheader.size);
            vfs_lseek(elfNode, sheader.file_offset, SEEK_SET);
            vfs_read(elfNode, (char *)symstrtab, sheader.size);
        }
        else if(sheader.type == 0x2) {
            symShdr = sheader;
        }
    }
    (void)symShdr;

    {
        for(int i = 1; i < header.section_header_entry_count; i++) {
            elf64_section_header_t sheader = section_table[i];
            if(sheader.type == 0x1 && !strcmp(&shstrtab[sheader.name], "moddeps")) {
                char *deps = (char *)kmalloc(sheader.size);
                vfs_lseek(elfNode, sheader.file_offset, SEEK_SET);
                vfs_read(elfNode, (char *)deps, sheader.size);

                uint64_t j = 0;
                while(j < sheader.size) {
                    printf("Module dependecy: %s\n", &deps[j]);
                    j += strlen(&deps[j]) + 1;
                }

                kfree(deps);
            }
        }
    }

    {
        //Allocate the sections, we do the relocation, not the compiler, the kernel can run in any memory region
        //as kernel can do anything, is not required to move the program to other ram section
        for(int i = 1; i < header.section_header_entry_count; i++) {
            elf64_section_header_t *sheader = &section_table[i];
            if(sheader->type == 0x8) {
                sheader->address = (uint64_t)kmalloc(sheader->size);
            } else if(sheader->type == 0x1 && sheader->flags & 0x7) {
                char *dataPointer = (char *)kmalloc(sheader->size);
                vfs_lseek(elfNode, sheader->file_offset, SEEK_SET);
                vfs_read(elfNode, (char *)dataPointer, sheader->size);

                sheader->address = (uint64_t)dataPointer;
            }
        }
    }

    elf64_symbol_t *symbolTable = (elf64_symbol_t *)kmalloc(symShdr.size);
    vfs_lseek(elfNode, symShdr.file_offset, SEEK_SET);
    vfs_read(elfNode, (char *)symbolTable, symShdr.size);

    hashmap_t *local_symbols = hashmap_create(20);
    {
        elf64_symbol_t *tableEntry = symbolTable;
        while(((uint64_t)tableEntry - (uint64_t)symbolTable) < symShdr.size) {
            if(tableEntry->name && (tableEntry->info >> 4) == 0x1) {
                if(tableEntry->section_table_index != 0) {
                    char *name = (char *)(symstrtab + tableEntry->name);

                    if(tableEntry->section_table_index < header.section_header_entry_count) {
                        elf64_section_header_t *sheader = &section_table[tableEntry->section_table_index];
                        uint64_t finalAddress = sheader->address + tableEntry->value;

                        hashmap_set(local_symbols, name, (void *)finalAddress);
                        hashmap_set(KernelSymbolsHashmap, name, (void *)finalAddress);
                    }
                }
            }
            tableEntry++;
        }
    }

    bool failed = false;

    for(int i = 1; i < header.section_header_entry_count; i++) {
        elf64_section_header_t *sheader = &section_table[i];
        if(sheader->type == 0x4) {
            elf64_rela_t *symbol_rela = (elf64_rela_t *)kmalloc(sizeof(elf64_rela_t));
            vfs_lseek(elfNode, sheader->file_offset, SEEK_SET);
            vfs_read(elfNode, (char *)symbol_rela, sheader->size);

            elf64_rela_t *tableEntry = symbol_rela;
            while(((uint64_t)tableEntry - (uint64_t)symbol_rela) < sheader->size) {
                elf64_symbol_t *symbol = &symbolTable[tableEntry->info >> 32];
                elf64_section_header_t *rela_sheader = &section_table[sheader->info]; //Info used as index on rela

                uint64_t *ptr = NULL;
                uint64_t symbolAddress = 0;

                char *name = (char *)(symstrtab + symbol->name);

                if((symbol->info & 0xF) == 0x3) { //Section
                    elf64_section_header_t *section_sheader = &section_table[symbol->section_table_index];

                    name = (char *)(shstrtab + section_sheader->name);

                    ptr = (uint64_t *)(rela_sheader->address + tableEntry->offset);
                    symbolAddress = section_sheader->address;
                } else {
                    name = (char *)(symstrtab + symbol->name);
                    ptr = (uint64_t *)(rela_sheader->address + tableEntry->offset);

                    symbolAddress = (uint64_t)hashmap_get(local_symbols, name);
                    if(!symbolAddress) symbolAddress = (uint64_t)hashmap_get(KernelSymbolsHashmap, name);
                    if(!symbolAddress) {
                        printf("Symbol %s not found\n", name);

                        kfree(symbol_rela);
                        failed = true;
                        goto clean;
                    }
                }

                switch(tableEntry->info & 0xFFFFFFFF) {
                    case 1:
                        *ptr = (uint64_t)(tableEntry->addend + symbolAddress);
                        break;
                    case 2:
                        *((uint32_t *)ptr) = (uint32_t)(tableEntry->addend + symbolAddress - (uint64_t)ptr);
                        break;
                    case 10:
                        {
                            uint64_t value = tableEntry->addend + symbolAddress;
                            uint32_t truncatedValue = value;
                            if(value != truncatedValue) {
                                printf("Relocation R_AMD64_32 doesn't match the original value: %s %llx %llx\n", name, value, truncatedValue);

                                kfree(symbol_rela);
                                failed = true;
                                goto clean;
                            }
                            *((uint32_t *)ptr) = truncatedValue;
                        }
                        break;
                    case 11:
                        {
                            uint64_t value = tableEntry->addend + symbolAddress;
                            uint32_t truncatedValue = (uint32_t)(tableEntry->addend + symbolAddress);
                            if((int64_t)value != (int64_t)(int32_t)truncatedValue) {
                                printf("Relocation R_AMD64_32S doesn't match the original value: %s\n", name);

                                kfree(symbol_rela);
                                failed = true;
                                goto clean;
                            }
                            *((uint32_t *)ptr) = truncatedValue;
                        }
                        break;
                    default:
                        printf("Unknown relocation type 0x%x\n", tableEntry->info);
                }

                tableEntry++;
            }

            kfree(symbol_rela);
        }
    }

    list_t *local_symbols_keys = hashmap_to_list(local_symbols);
    module_info_t *module_info = NULL;

    failed = true;
    foreach(_key, local_symbols_keys) {
        char *key = (char *)_key->value;
        if(!strncmp(key, "module_info_", 12)) {
            module_info = (module_info_t *)hashmap_get(local_symbols, key);
            failed = false;
            break;
        }
    }
    list_destroy(local_symbols_keys);
    hashmap_delete(local_symbols);

    if(module_info) {
        printf("Module base 0x%llx\n", module_info->init);
        module_info->init();

        printf("Module %s loaded\n", module_info->name);
    } else {
        printf("[Module] Failed to locate module_info_ structure\n");
    }

clean:
    kfree(symbolTable);
    kfree(symstrtab);
    kfree(shstrtab);
    kfree(section_table);

    return !failed;
}

bool module_load(char *path) {
    vfs_node_t *elfNode = kopen(path, 0);
    if(!elfNode) {
        printf("asa1\n");
        return false;
    }

    bool result = module_load_node(elfNode);

    vfs_close(elfNode);
    kfree(elfNode);

    return result;
}

bool elf_load(vfs_node_t *node, void *pml4, size_t base, elf_value_t *value, char **out_ld_path) {
    vfs_lseek(node, 0, SEEK_SET);

    elf64_header_t header;
    vfs_read(node, (char *)&header, sizeof(elf64_header_t));

    if(header.magic != 0x464C457F || header.x86_64_flag != 0x2 ||
       header.big_endian_flag != 0x1 || header.instruction_set != 0x3E) {
        return false;
    }

    value->phdr = 0;
    value->phent = sizeof(elf64_program_header_t);
    value->phnum = header.program_header_entry_count;

    elf64_program_header_t *program_table = (elf64_program_header_t *)kmalloc(header.program_header_entry_count * header.program_header_entry_size);
    vfs_lseek(node, header.program_header_table_address, SEEK_SET);
    vfs_read(node, (char *)program_table, header.program_header_entry_count * header.program_header_entry_size);

    char *ldPath = NULL;
    for(size_t i = 0; i < header.program_header_entry_count; i++) {
        elf64_program_header_t *pheader = &program_table[i];
        if(pheader->type == 0x3) {
            if(!out_ld_path) continue;

            ldPath = (char *)kmalloc(pheader->file_size + 1);

            vfs_lseek(node, pheader->file_offset, SEEK_SET);
            vfs_read(node, ldPath, pheader->file_size);

            ldPath[pheader->file_size] = 0;
        } else if(pheader->type == 0x6) value->phdr = (uint64_t)base + pheader->virtual_address;
        else if(pheader->type != 0x1) continue;

        size_t misalign = pheader->virtual_address & (PAGE_SIZE - 1);
        size_t pageCount = (misalign + pheader->memory_size + (PAGE_SIZE - 1)) / PAGE_SIZE;

        void *address = pmm_alloc(pageCount);
        if(!address) { //TODO: Remove previous sections loaded here
            kfree(address);
            kfree(program_table);
            return false;
        }

        //printf("Mapping 0x%llx\n", (void *)((uint64_t)base + pheader->virtual_address));
        vmm_map(pml4, (void *)((uint64_t)base + pheader->virtual_address), address, pageCount, (pheader->flags & 0x2) ? 0x7 : 0x5);

        void *buffer = (void *)((size_t)address + MEM_PHYS_OFFSET);
        vfs_lseek(node, pheader->file_offset, SEEK_SET);
        vfs_read(node, (char *)(buffer + misalign), pheader->file_size);
    }

    kfree(program_table);

    //printf("%x\n", value->phdr);

    if(value) value->entry = (uint64_t)base + header.entry_address;
    if(out_ld_path) *out_ld_path = ldPath;
    return true;
}

void modules_init() {
    KernelSymbolsHashmap = hashmap_create(20);

    kernel_symbol_t *symbol = (kernel_symbol_t *)&kernel_symbols_start;

    while((uint64_t)symbol < (uint64_t)&kernel_symbols_end) {
        hashmap_set(KernelSymbolsHashmap, symbol->name, (void *)symbol->addr);
        uintptr_t new_address = (uintptr_t)symbol + sizeof(kernel_symbol_t) + strlen(symbol->name) + 1;
        symbol = (kernel_symbol_t *)ROUND_UP(new_address, sizeof(uintptr_t));
    }

    hashmap_set(KernelSymbolsHashmap, "kernel_symbols_start", kernel_symbols_start);
    hashmap_set(KernelSymbolsHashmap, "kernel_symbols_end", kernel_symbols_end);
}

bool exec(kpid_t pid, const char *filename, const char *argv[], const char *envp[]) {
    process_t *process = SchedulerProcesses[pid];
    pt_t *oldPagemap = (pt_t *)process->page_table;

    vfs_node_t *programNode = kopen(filename, 0);
    if(!programNode) return false;

    pt_t *pml4 = vmm_setup_pml4();

    char *ldPath = NULL;

    elf_value_t value = (elf_value_t){0};
    elf_load(programNode, pml4, 0, &value, &ldPath);

    vfs_close(programNode);
    kfree(programNode);

    uint64_t entry = 0;
    if(!ldPath) entry = value.entry;
    else {
        programNode = kopen(ldPath, 0);
        printf("Linker: %s\n", ldPath);

        elf_value_t ld_value = {0};
        elf_load(programNode, pml4, 0x40000000, &ld_value, NULL);
        vfs_close(programNode);
        kfree(programNode);

        kfree(ldPath);
        entry = ld_value.entry;
    }

    for(size_t i = 256; i < 512; i++) {
        ((pt_entries_t *)((size_t)pml4->entries + MEM_PHYS_OFFSET))->entries[i] = ((pt_entries_t *)((size_t)SchedulerProcesses[0]->page_table->entries + MEM_PHYS_OFFSET))->entries[i];
    }

    //Destroy threads here when implementing real threading on user space

    process->page_table = pml4;
    vmm_free_pml4(oldPagemap);

    ktid_t tid = thread_create(pid, thread_parameter_exec, thread_call_data((void *)entry, argv, envp, &value));
    scheduler_add_task(pid, tid);

    return true;
}

extern void force_reschedule();
bool execve(kpid_t pid, const char *filename, const char *argv[], const char *envp[]) {
    process_t *process = SchedulerProcesses[pid];
    spinlock_lock(&process->lock);
    pt_t *oldPagemap = (pt_t *)process->page_table;

    vfs_node_t *programNode = kopen(filename, 0);
    if(!programNode) {
        spinlock_unlock(&process->lock);
        return false;
    }

    pt_t *pml4 = vmm_setup_pml4();

    char *ldPath = NULL;

    elf_value_t value = (elf_value_t){0};
    elf_load(programNode, pml4, 0, &value, &ldPath);

    vfs_close(programNode);
    kfree(programNode);

    uint64_t entry = 0;
    if(!ldPath) entry = value.entry;
    else {
        programNode = kopen(ldPath, 0);
        printf("Linker: %s\n", ldPath);

        elf_value_t ld_value = {0};
        elf_load(programNode, pml4, 0x40000000, &ld_value, NULL);
        vfs_close(programNode);
        kfree(programNode);

        kfree(ldPath);
        entry = ld_value.entry;
    }

    for(size_t i = 256; i < 512; i++) {
        ((pt_entries_t *)((size_t)pml4->entries + MEM_PHYS_OFFSET))->entries[i] = ((pt_entries_t *)((size_t)SchedulerProcesses[0]->page_table->entries + MEM_PHYS_OFFSET))->entries[i];
    }

    //Destroy threads here when implementing real threading on user space

    for(ktid_t tid = 0; tid < MAX_THREADS; tid++) {
        volatile thread_t *thread = process->threads[tid];
        if(thread != NULL && thread != (void *)-1) {
            if(thread->taskID != (ktid_t)-1) {
                ActiveTasks[thread->taskID] = (volatile thread_t *)NULL; //TODO: Move this to scheduler
            }

            thread->taskID = -1; //Indicate this thread is dead, TODO: Delte this thread when rescheduling
        }
    }

    //Delete all threads, i will use thread zero later
    for(ktid_t tid = 1; tid < MAX_THREADS; tid++) {
        volatile thread_t *thread = process->threads[tid];
        if(thread != NULL && thread != (void *)-1) {
            process->threads[tid] = (volatile thread_t *)NULL;
            kfree((void *)thread->fxstate);
            if(thread->kstack_address) kfree((void *)thread->kstack_address);
            if(thread->memoryStackPhys) pmm_free(thread->memoryStackPhys, 4);
            kfree((void *)thread);
        }
    }

    process->page_table = pml4;
    process->current_alloc_address = BASE_ALLOC_ADDRESS;
    kfree((void *)filename); //TODO: This should be handled in other way
    spinlock_unlock(&process->lock);

    ktid_t tid = thread_recreate(pid, 0, thread_parameter_exec, thread_call_data((void *)entry, argv, envp, &value), true);
    scheduler_add_task(pid, tid);

    vmm_free_pml4(oldPagemap);
    printf("Reached rescheduling\n");

    volatile cpu_t *cpuLocal = &CPULocals[CurrentCPU];
    cpuLocal->currentTaskID = -1;
    force_reschedule();

    return true;
}