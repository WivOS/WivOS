#include "scheduler.h"
#include "process.h"
#include "thread.h"

#include <cpu/cpu.h>
#include <cpu/inlines.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

bool syscall_privilege_check(void *base, size_t size) {
    return ((size_t)base & (size_t)0x800000000000 || ((size_t)base + size) & (size_t)0x800000000000) != 0;
}

//Debug: [rdi] String address
size_t syscall_debug(irq_regs_t *regs) {
    if(syscall_privilege_check((void *)regs->rdi, strlen((char *)regs->rdi) + 1))
        return -1;

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    kpid_t tid = CPULocals[CurrentCPU].currentTid;
    spinlock_unlock(&SchedulerLock);

    printf("[Debug pid: %d, tid: %d] %s\n", pid, tid, (char *)regs->rdi);

    return 0;
}

//Read: [rdi] File descriptor, [rsi] buffer, [rdx] Length
size_t syscall_read(irq_regs_t *regs) {
    int fd = (int)regs->rdi;
    char *buffer = (char *)regs->rsi;
    size_t length = regs->rdx;

    if(fd >= MAX_FILE_HANDLES || fd < 0) return -1;

    //TODO: Errno

    if(syscall_privilege_check((void *)buffer, length)) return -1;

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    spinlock_lock(&process->file_handles_lock);

    vfs_node_t *node = (vfs_node_t *)process->file_handles[fd];
    if(!node || node == (void *)-1) {
        spinlock_unlock(&process->file_handles_lock);
        return -1;
    }

    size_t currentIndex = 0;
    while(currentIndex < length) {
        size_t step = (currentIndex + 16777216 > length) ? (length % 16777216) : 16777216;
        size_t retValue = vfs_read(node, (char *)((uint64_t)buffer + currentIndex), length);
        currentIndex += retValue;
        if(retValue < step) break;
    }

    spinlock_unlock(&process->file_handles_lock);

    return currentIndex;
}

//Write: [rdi] File descriptor, [rsi] buffer, [rdx] Length
size_t syscall_write(irq_regs_t *regs) {
    int fd = (int)regs->rdi;
    char *buffer = (char *)regs->rsi;
    size_t length = regs->rdx;

    if(fd >= MAX_FILE_HANDLES || fd < 0) return -1;

    //TODO: Errno

    if(syscall_privilege_check((void *)buffer, length)) return -1;

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    spinlock_lock(&process->file_handles_lock);

    vfs_node_t *node = (vfs_node_t *)process->file_handles[fd];
    if(!node || node == (void *)-1) {
        spinlock_unlock(&process->file_handles_lock);
        return -1;
    }

    size_t currentIndex = 0;
    while(currentIndex < length) {
        size_t step = (currentIndex + 16777216 > length) ? (length % 16777216) : 16777216;
        size_t retValue = vfs_write(node, (char *)((uint64_t)buffer + currentIndex), length);
        currentIndex += retValue;
        if(retValue < step) break;
    }

    spinlock_unlock(&process->file_handles_lock);

    return currentIndex;
}

//Open: [rdi] Path string, [rsi] Mode
int syscall_open(irq_regs_t *regs) {
    char *path = (char *)regs->rdi;
    uint64_t mode = regs->rsi;

    //TODO: Errno

    if(syscall_privilege_check((void *)path, strlen(path) + 1)) return -1;

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    spinlock_lock(&process->file_handles_lock);

    int file_fd;
    for(file_fd = 0; process->file_handles[file_fd] != ((void *)-1); file_fd++)
        if((file_fd + 1) == MAX_FILE_HANDLES) {
            spinlock_unlock(&process->file_handles_lock);
            return -1;
        }

    vfs_node_t *node = kopen(path, mode);
    if(!node) {
        spinlock_unlock(&process->file_handles_lock);
        return -1;
    }

    process->file_handles[file_fd] = node;

    spinlock_unlock(&process->file_handles_lock);

    return file_fd;
}

//Close: [rdi] File descriptor
int syscall_close(irq_regs_t *regs) {
    int fd = (int)regs->rdi;

    if(fd >= MAX_FILE_HANDLES || fd < 0) return -1;

    //TODO: Errno

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    spinlock_lock(&process->file_handles_lock);

    vfs_node_t *node = (vfs_node_t *)process->file_handles[fd];
    if(!node || node == (void *)-1) {
        spinlock_unlock(&process->file_handles_lock);
        return -1;
    }

    vfs_close(node);
    kfree(node);
    process->file_handles[fd] = NULL;

    spinlock_unlock(&process->file_handles_lock);

    return 0;
}

//Alloc: [rdi] Requested address, [rsi] Page count
void *syscall_alloc(irq_regs_t *regs) {
    uint64_t requestedAddress = regs->rdi;
    uint64_t requestedPages = regs->rsi;

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    //TODO: Errno

    uint64_t mapAddress;
    if(requestedAddress) {
        if(syscall_privilege_check((void *)requestedAddress, regs->rdi * PAGE_SIZE)) return NULL;
        mapAddress = requestedAddress;
    } else {
        spinlock_lock(&process->alloc_lock);
        mapAddress = process->current_alloc_address;
        if(syscall_privilege_check((void *)mapAddress, requestedPages * PAGE_SIZE)) {
            spinlock_unlock(&process->alloc_lock);
            return NULL;
        }
        process->current_alloc_address += requestedPages * PAGE_SIZE;
        spinlock_unlock(&process->alloc_lock);
    }

    //printf("mapped: %llx %llx %\n", requestedAddress, requestedPages, mapAddress);

    void *memory = pmm_alloc(requestedPages);
    if(!memory) {
        return NULL;
    }

    if(!vmm_map((pt_t *)process->page_table, (void *)mapAddress, memory, requestedPages, 0x07)) {
        pmm_free(memory, requestedPages);
        return NULL;
    }

    return (void *)mapAddress;
}

//Seek: [rdi] File descriptor, [rsi] offset, [rdx] whence
size_t syscall_seek(irq_regs_t *regs) {
    int fd = (int)regs->rdi;
    uint64_t offset = regs->rsi;
    size_t whence = regs->rdx;

    if(fd >= MAX_FILE_HANDLES || fd < 0) return -1;

    //TODO: Errno

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    spinlock_lock(&process->file_handles_lock);

    vfs_node_t *node = (vfs_node_t *)process->file_handles[fd];
    if(!node || node == (void *)-1) {
        spinlock_unlock(&process->file_handles_lock);
        return -1;
    }

    size_t retValue = vfs_lseek(node, offset, whence);

    spinlock_unlock(&process->file_handles_lock);

    return retValue;
}

//Tcb Set: [rdi] Tcb address
size_t syscall_tcb_set(irq_regs_t *regs) {
    size_t fs_base = (size_t)regs->rdi;

    spinlock_lock(&SchedulerLock);
    kpid_t taskID = CPULocals[CurrentCPU].currentTaskID;
    thread_t *thread = (thread_t *)ActiveTasks[taskID];

    thread->fs_base = fs_base;
    load_fs_base(fs_base);

    spinlock_unlock(&SchedulerLock);

    return 0;
}

//Fork
size_t syscall_fork(irq_regs_t *regs) {
    spinlock_lock(&SchedulerLock);

    ktid_t taskID = CPULocals[CurrentCPU].currentTaskID;
    kpid_t old_pid = CPULocals[CurrentCPU].currentPid;
    thread_t *old_thread = (thread_t *)ActiveTasks[taskID];
    process_t *old_process = SchedulerProcesses[old_pid];

    pt_t *new_pagemap = vmm_copy_pml4((pt_t *)old_process->page_table);

    spinlock_unlock(&SchedulerLock);

    kpid_t new_pid = process_create(new_pagemap);
    process_t *new_process = SchedulerProcesses[new_pid];

    new_process->parent_pid = old_thread->pid;
    new_process->current_alloc_address = old_process->current_alloc_address;

    //TODO: Dupe thism this is not real, only for specific devices like log
    for(size_t i = 0; i < MAX_FILE_HANDLES; i++)
        new_process->file_handles[i] = old_process->file_handles[i];

    thread_t *new_thread = (thread_t *)kmalloc(sizeof(thread_t));
    new_process->threads[0] = (volatile thread_t *)new_thread;

    new_thread->tid = 0;
    new_thread->pid = new_pid;
    new_thread->taskID = -1;
    new_thread->lock = INIT_SPINLOCK();
    new_thread->cpu_number = -1;
    new_thread->kstack_address = (void*)((uint64_t)kmalloc(0x8000) + 0x8000);
    new_thread->fs_base = old_thread->fs_base;
    new_thread->saved_regs = *(volatile irq_regs_t *)regs;
    new_thread->saved_regs.rax = 0;
    new_thread->fxstate = kmalloc(CpuSimdRegionSize);

    cpu_save_simd((void *)new_thread->fxstate);

    //printf("%llX %d\n", new_thread->saved_regs.rbp, new_pid);

    scheduler_add_task(new_thread->pid, new_thread->tid);

    return new_pid;
}

//IoCtl: [rdi] File descriptor, [rsi] request, [rdx] arg
size_t syscall_ioctl(irq_regs_t *regs) {
    int fd = (int)regs->rdi;
    uint64_t request = regs->rsi;
    void *arg = (void *)regs->rdx;

    if(fd >= MAX_FILE_HANDLES || fd < 0) return -1;

    //TODO: Errno

    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    process_t *process = SchedulerProcesses[pid];
    spinlock_unlock(&SchedulerLock);

    spinlock_lock(&process->file_handles_lock);

    vfs_node_t *node = (vfs_node_t *)process->file_handles[fd];
    if(!node || node == (void *)-1) {
        spinlock_unlock(&process->file_handles_lock);
        return -1;
    }

    size_t retValue = vfs_ioctl(node, request, arg);

    spinlock_unlock(&process->file_handles_lock);

    return retValue;
}

static spinlock_t syscall_function_lock = INIT_SPINLOCK();
static syscall_fn_t syscall_module_functions[MAX_MODULE_SYSCALLS];
static size_t syscall_module_functions_length = 0;

bool syscall_install(syscall_fn_t func, size_t index) {
    spinlock_lock(&syscall_function_lock);
    if(index >= MAX_MODULE_SYSCALLS) {
        return false;
    }

    if(syscall_module_functions[index] != NULL) {
        spinlock_unlock(&syscall_function_lock);
        return false;
    }

    //printf("[Syscall] Registering syscall 0x%x to 0x%llx\n", index, (uint64_t)func);

    syscall_module_functions[index] = func;
    spinlock_unlock(&syscall_function_lock);

    return true;
}

//Invalid or module handler
size_t syscall_module_or_invalid(irq_regs_t *regs) {
    if(regs->rax >= MAX_MODULE_SYSCALLS) return -1;

    spinlock_lock(&syscall_function_lock);
    syscall_fn_t func = syscall_module_functions[regs->rax];
    spinlock_unlock(&syscall_function_lock);

    if(func == NULL) {
        printf("[Syscall] Unknown syscall 0x%x\n", regs->rax);
        return -1;
    }

    return func(regs);
}