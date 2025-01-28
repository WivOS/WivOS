#include "thread.h"
#include "process.h"
#include "scheduler.h"

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <cpu/cpu.h>

#define STACK_LOCATION_TOP ((size_t)0x0000800000000000)

extern uint8_t* DefaultFxstate;

ktid_t thread_create(kpid_t pid, thread_parameter_types_t dataType, void *data) {
    process_t *process = SchedulerProcesses[pid];
    if(process == NULL) return -1;

    spinlock_lock(&process->lock);

    ktid_t tid = 0;
    for(; tid < MAX_THREADS; tid++) {
        if(process->threads[tid] == NULL || process->threads[tid] == (void *)-1) {
            goto done;
        }
    }
    printf("Max thread number allocated reached\n");
    spinlock_unlock(&process->lock);
    return -1;

done: (void)tid;

    thread_t *newThread = kmalloc(sizeof(thread_t));
    newThread->lock = INIT_SPINLOCK();
    newThread->pid = pid;
    newThread->tid = tid;
    newThread->taskID = -1;
    newThread->fs_base = 0;
    newThread->event_abort = 0;
    newThread->yield_target = 0;
    newThread->fxstate = kmalloc(CpuSimdRegionSize);
    newThread->memoryStackPhys = NULL;
    newThread->dont_save = false;
    if(pid == 0x0) {
        newThread->saved_regs.cs = 0x08;
        newThread->saved_regs.rflags = 0x202;
        newThread->saved_regs.ss = 0x10;
    } else {
        newThread->saved_regs.cs = 0x23;
        newThread->saved_regs.rflags = 0x202;
        newThread->saved_regs.ss = 0x1B;
    }

    newThread->kstack_address = (void*)((uint64_t)kmalloc(0x8000) + 0x8000);
    *((size_t *)((uint64_t)newThread->kstack_address - sizeof(size_t))) = 0;

    if(!pid) {
        newThread->saved_regs.rsp = (uint64_t)newThread->kstack_address;
    } else {
        size_t stackGuardPage = STACK_LOCATION_TOP - (0x1000 * 4 + 0x1000) * (tid + 1);
        size_t stackBottom = stackGuardPage + PAGE_SIZE;

        void *memoryStack = (void *)pmm_alloc(4);
        size_t *sbase = (size_t *)(memoryStack + 0x1000 * 4 + MEM_PHYS_OFFSET);

        size_t *sp;
        if(dataType == thread_parameter_exec) {
            thread_parameter_exec_data_t *execData = (thread_parameter_exec_data_t *)data;

            char *strp = (char *)sbase;
            size_t nenv = 0;
            for(char **elem = (char **)execData->envp; *elem; elem++) {
                strp -= strlen(*elem) + 1;
                strcpy(strp, *elem);
                nenv++;
            }

            size_t nargs = 0;
            for(char **elem = (char **)execData->argv; *elem; elem++) {
                strp -= strlen(*elem) + 1;
                strcpy(strp, *elem);
                nargs++;
            }

            strp -= (size_t)strp & 0xF;

            sp = (size_t *)strp;
            if((nargs + nenv + 1) & 0x1) --sp;

            *(--sp) = 0;
            *(--sp) = 0;
            sp -= 2; *sp = 9; *(sp + 1) = execData->value->entry;
            sp -= 2; *sp = 3; *(sp + 1) = execData->value->phdr;
            sp -= 2; *sp = 4; *(sp + 1) = execData->value->phent;
            sp -= 2; *sp = 5; *(sp + 1) = execData->value->phnum;

            size_t sa = (size_t)(stackBottom + 0x1000 * 4);
            *(--sp) = 0;
            sp -= nenv;
            for(size_t i = 0; i < nenv; i++) {
                sa -= strlen((char *)execData->envp[i]) + 1;
                sp[i] = sa;
            }

            *(--sp) = 0;
            sp -= nargs;
            for(size_t i = 0; i < nargs; i++) {
                sa -= strlen((char *)execData->argv[i]) + 1;
                sp[i] = sa;
            }

            *(--sp) = nargs;
        } else {
            sp = sbase;
        }

        vmm_map((pt_t *)process->page_table, (void *)stackBottom, memoryStack, 4, 0x7);
        vmm_unmap((pt_t *)process->page_table, (void *)stackGuardPage, 1);

        newThread->saved_regs.rsp = (uint64_t)stackBottom + 0x1000 * 4 - ((sbase - sp) * sizeof(size_t));
        newThread->memoryStackPhys = memoryStack;
    }

    if(dataType == thread_parameter_entry) {
        thread_parameter_entry_data_t *entryData = (thread_parameter_entry_data_t *)data;
        newThread->saved_regs.rip = (uint64_t)entryData->entry;
        newThread->saved_regs.rdi = (uint64_t)entryData->arg;
    } else {
        thread_parameter_exec_data_t *execData = (thread_parameter_exec_data_t *)data;
        newThread->saved_regs.rip = (uint64_t)execData->entry;
    }

    memcpy((void *)newThread->fxstate, (void *)DefaultFxstate, CpuSimdRegionSize);

    process->threads[tid] = newThread;

    spinlock_unlock(&process->lock);

    return tid;
}

ktid_t thread_recreate(kpid_t pid, kpid_t tid, thread_parameter_types_t dataType, void *data, bool free_args) {
    process_t *process = SchedulerProcesses[pid];
    if(process == NULL || process->threads[tid] == NULL || process->threads[tid] == (void *)-1) {
        return -1;
    }
    spinlock_lock(&process->lock);

    thread_t *newThread = (thread_t *)process->threads[tid];
    spinlock_try_lock(&newThread->lock);
    spinlock_unlock(&newThread->lock);
    newThread->pid = pid;
    newThread->tid = tid;
    newThread->cpu_number = -1;
    newThread->taskID = -1;
    newThread->fs_base = 0;
    newThread->event_abort = 0;
    newThread->yield_target = 0;
    newThread->memoryStackPhys = NULL;
    newThread->fs_base = 0;
    newThread->ustack_address = NULL;
    newThread->event_pointer = NULL;
    newThread->out_event_pointer = NULL;
    newThread->event_timeout = 0;
    newThread->event_count = 0;
    //newThread->fxstate = kmalloc(CpuSimdRegionSize);
    memset((void *)&newThread->saved_regs, 0, sizeof(irq_regs_t));
    if(pid == 0x0) {
        newThread->saved_regs.cs = 0x08;
        newThread->saved_regs.rflags = 0x202;
        newThread->saved_regs.ss = 0x10;
    } else {
        newThread->saved_regs.cs = 0x23;
        newThread->saved_regs.rflags = 0x202;
        newThread->saved_regs.ss = 0x1B;
    }

    //newThread->kstack_address = (void*)((uint64_t)kmalloc(0x8000) + 0x8000);
    *((size_t *)((uint64_t)newThread->kstack_address - sizeof(size_t))) = 0;

    if(!pid) {
        newThread->saved_regs.rsp = (uint64_t)newThread->kstack_address;
    } else {
        size_t stackGuardPage = STACK_LOCATION_TOP - (0x1000 * 4 + 0x1000) * (tid + 1);
        size_t stackBottom = stackGuardPage + PAGE_SIZE;

        void *memoryStack = newThread->memoryStackPhys;
        if(newThread->memoryStackPhys == NULL) {
            memoryStack = (void *)pmm_alloc(4);
        } else {
            memset((void *)((uintptr_t)newThread->memoryStackPhys + MEM_PHYS_OFFSET), 0, 4 * PAGE_SIZE);
        }
        size_t *sbase = (size_t *)(memoryStack + 0x1000 * 4 + MEM_PHYS_OFFSET);

        size_t *sp;
        if(dataType == thread_parameter_exec) {
            thread_parameter_exec_data_t *execData = (thread_parameter_exec_data_t *)data;

            char *strp = (char *)sbase;
            size_t nenv = 0;
            for(char **elem = (char **)execData->envp; *elem; elem++) {
                strp -= strlen(*elem) + 1;
                strcpy(strp, *elem);
                nenv++;
            }

            size_t nargs = 0;
            for(char **elem = (char **)execData->argv; *elem; elem++) {
                strp -= strlen(*elem) + 1;
                strcpy(strp, *elem);
                nargs++;
            }

            strp -= (size_t)strp & 0xF;

            sp = (size_t *)strp;
            if((nargs + nenv + 1) & 0x1) --sp;

            *(--sp) = 0;
            *(--sp) = 0;
            sp -= 2; *sp = 9; *(sp + 1) = execData->value->entry;
            sp -= 2; *sp = 3; *(sp + 1) = execData->value->phdr;
            sp -= 2; *sp = 4; *(sp + 1) = execData->value->phent;
            sp -= 2; *sp = 5; *(sp + 1) = execData->value->phnum;

            size_t sa = (size_t)(stackBottom + 0x1000 * 4);
            *(--sp) = 0;
            sp -= nenv;
            for(size_t i = 0; i < nenv; i++) {
                sa -= strlen((char *)execData->envp[i]) + 1;
                sp[i] = sa;
            }

            *(--sp) = 0;
            sp -= nargs;
            for(size_t i = 0; i < nargs; i++) {
                sa -= strlen((char *)execData->argv[i]) + 1;
                sp[i] = sa;
            }

            *(--sp) = nargs;

            if(free_args) {
                for(int i = 0; i < nenv; i++) {
                    kfree((void *)execData->envp[i]);
                }
                kfree((void *)execData->envp);

                for(int i = 0; i < nargs; i++) {
                    kfree((void *)execData->argv[i]);
                }
                kfree((void *)execData->argv);
            }
        } else {
            sp = sbase;
        }

        vmm_map((pt_t *)process->page_table, (void *)stackBottom, memoryStack, 4, 0x7);
        vmm_unmap((pt_t *)process->page_table, (void *)stackGuardPage, 1);

        newThread->saved_regs.rsp = (uint64_t)stackBottom + 0x1000 * 4 - ((sbase - sp) * sizeof(size_t));
        newThread->memoryStackPhys = memoryStack;
    }

    if(dataType == thread_parameter_entry) {
        thread_parameter_entry_data_t *entryData = (thread_parameter_entry_data_t *)data;
        newThread->saved_regs.rip = (uint64_t)entryData->entry;
        newThread->saved_regs.rdi = (uint64_t)entryData->arg;
    } else {
        thread_parameter_exec_data_t *execData = (thread_parameter_exec_data_t *)data;
        newThread->saved_regs.rip = (uint64_t)execData->entry;
    }

    memcpy((void *)newThread->fxstate, (void *)DefaultFxstate, CpuSimdRegionSize);

    newThread->dont_save = true; //Don't save current context to the regenerated thread
    process->threads[tid] = newThread;

    spinlock_unlock(&process->lock);

    return tid;
}