#include "proc.h"
#include "smp.h"
#include <acpi/apic.h>
#include <util/util.h>

static volatile spinlock_t syscalls_lock = INIT_LOCK();
extern volatile spinlock_t schedulerLock;

static inline int privilegeCheck(void *base, size_t len) {
    if((size_t)base & (size_t)0x800000000000 || ((size_t)base + len) & (size_t)0x800000000000)
        return 1;
    else
        return 0;
}

size_t debug_syscall(thread_regs_t *regs) {
    if(privilegeCheck((void *)regs->rdi, strlen((const char *)regs->rdi) + 1))
        return -1;

    spinlock_lock(&syscalls_lock);
    printf("[Debug pid: %x, tid: %x] %s", cpuLocals[current_cpu].currentProcess, cpuLocals[current_cpu].currentThread, regs->rdi);
    spinlock_unlock(&syscalls_lock);
    return 0;
}

size_t open_syscall(thread_regs_t *regs) { // rdi: path, rsi: flags
    char *filepath = (char *)regs->rdi;
    size_t flags = regs->rsi;

    if(privilegeCheck(filepath, strlen(filepath) + 1))
        return -1;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentActiveThread);

    vfs_node_t *node = kopen(filepath, flags);
    if(!node) {
        return -1;
    }

    spinlock_lock(&currentProcess->filesLock);

    size_t fd = 0;
    for(; fd <= MAX_FILE_HANDLES; fd++) {
        if(fd == MAX_FILE_HANDLES) {
            spinlock_unlock(&currentProcess->filesLock);
            return -1;
        }

        if(currentProcess->fileHandles[fd] == NULL) {
            currentProcess->fileHandles[fd] = (vfs_node_t *)-1; // Mark as used but not allocated for now, for check errors later
            break;
        }
    }

    spinlock_unlock(&currentProcess->filesLock);

    currentProcess->fileHandles[fd] = node;

    return fd;
}

size_t read_syscall(thread_regs_t *regs) { // rdi: fd, rsi: buffer, rdx: count
    size_t fd = regs->rdi;
    void *buffer = (void *)regs->rsi;
    size_t count = regs->rdx;

    if(privilegeCheck(buffer, count))
        return -1;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentActiveThread);

    spinlock_lock(&currentProcess->filesLock);

    if(fd >= MAX_FILE_HANDLES || !currentProcess->fileHandles[fd] || currentProcess->fileHandles[fd] == (vfs_node_t *)-1) {
        spinlock_unlock(&currentProcess->filesLock);
        return -1;
    }

    vfs_node_t *currentFD = currentProcess->fileHandles[fd];
    size_t ret = vfs_read(currentFD, buffer, count);

    spinlock_unlock(&currentProcess->filesLock);

    return ret;
}

size_t write_syscall(thread_regs_t *regs) { // rdi: fd, rsi: buffer, rdx: count
    size_t fd = regs->rdi;
    void *buffer = (void *)regs->rsi;
    size_t count = regs->rdx;

    if(privilegeCheck(buffer, count))
        return -1;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentActiveThread);

    spinlock_lock(&currentProcess->filesLock);

    if(fd >= MAX_FILE_HANDLES || !currentProcess->fileHandles[fd] || currentProcess->fileHandles[fd] == (vfs_node_t *)-1) {
        spinlock_unlock(&currentProcess->filesLock);
        return -1;
    }

    vfs_node_t *currentFD = currentProcess->fileHandles[fd];
    size_t ret = vfs_write(currentFD, buffer, count);

    spinlock_unlock(&currentProcess->filesLock);

    return ret;
}

size_t close_syscall(thread_regs_t *regs) { // rdi: fd
    size_t fd = regs->rdi;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentActiveThread);

    spinlock_lock(&currentProcess->filesLock);

    if(fd >= MAX_FILE_HANDLES || !currentProcess->fileHandles[fd] || currentProcess->fileHandles[fd] == (vfs_node_t *)-1) {
        spinlock_unlock(&currentProcess->filesLock);
        return -1;
    }

    vfs_node_t *currentFD = currentProcess->fileHandles[fd];
    vfs_close(currentFD);

    currentProcess->fileHandles[fd] = (vfs_node_t *)NULL;

    spinlock_unlock(&currentProcess->filesLock);

    return 0;
}

size_t ioctl_syscall(thread_regs_t *regs) { // rdi: fd, rsi: requestType, rdx: argp
    size_t fd = regs->rdi;
    size_t requestType = regs->rsi;
    void *argp = (void *)regs->rdx;

    if((argp != NULL) && privilegeCheck(argp, 1))
        return -1;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentActiveThread);

    spinlock_lock(&currentProcess->filesLock);

    if(fd >= MAX_FILE_HANDLES || !currentProcess->fileHandles[fd] || currentProcess->fileHandles[fd] == (vfs_node_t *)-1) {
        spinlock_unlock(&currentProcess->filesLock);
        return -1;
    }

    vfs_node_t *currentFD = currentProcess->fileHandles[fd];
    size_t ret = vfs_ioctl(currentFD, requestType, argp);

    spinlock_unlock(&currentProcess->filesLock);

    return ret;
}

void *alloc_at_syscall(thread_regs_t *regs) { // rdi: address, rsi: count
    void *address = (void *)regs->rdi;
    size_t count = regs->rsi;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentProcess);

    size_t baseAddress;
    if(address) {
        baseAddress = (size_t)address;
        if(privilegeCheck((void *)baseAddress, count * PAGE_SIZE))
            return (void *)0;
    } else {
        spinlock_lock(&currentProcess->brkLock);
        baseAddress = currentProcess->brkAddress;
        if(privilegeCheck((void *)baseAddress, count * PAGE_SIZE)) {
            spinlock_unlock(&currentProcess->brkLock);
            return (void *)0;
        }
        currentProcess->brkAddress += count * PAGE_SIZE;
        spinlock_unlock(&currentProcess->brkLock);
    }
    if(!count) return (void *)baseAddress;

    void *ptr = pmm_alloc(count);
    if(!ptr) {
        return (void *)0;
    }
    vmm_map_pages((void *)currentProcess->process_pml4, (void *)baseAddress, ptr, count, 0x7);

    return (void *)baseAddress;
}

extern volatile thread_t **activeThreads;
extern volatile size_t totalThreads;

size_t fork_syscall(thread_regs_t *regs) {

    thread_t *currentThread = get_active_thread(cpuLocals[current_cpu].currentActiveThread);
    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentProcess);

    pt_t *newPml4 = fork_pml4((pt_t *)currentProcess->process_pml4);
    pid_t newPid = proc_create(newPml4);
    process_t *newProcess = get_active_process(newPid);

    newProcess->brkAddress = currentProcess->brkAddress;

    for(size_t i = 0; i < MAX_FILE_HANDLES; i++) {
        //TODO dup, for now it's not neede because we use pointers and any driver uses internal data
        newProcess->fileHandles[i] = currentProcess->fileHandles[i];
    }

    newProcess->threads[0] = (thread_t *)kmalloc(sizeof(thread_t));
    thread_t *newThread = (thread_t *)newProcess->threads[0];

    spinlock_lock(&schedulerLock);
    size_t activeThreadID = 0;
    for(; activeThreadID < 4096; activeThreadID++) {
        if(!activeThreads[activeThreadID] || activeThreads[activeThreadID] == (void *)-1) {
            break;
        }
        if(activeThreadID == 4095) {
            newProcess->threads[0] = NULL;
            spinlock_unlock(&schedulerLock);
            //TODO: Handle bad returns, free things
            return -1;
        }
    }
    activeThreads[activeThreadID] = (void *)-1;
    spinlock_unlock(&schedulerLock);

    newThread->lock = INIT_LOCK();
    newThread->cpuNumber = -1;
    newThread->pid = newPid;
    newThread->tid = 0;
    newThread->schedTid = activeThreadID;
    newThread->context_regs = *regs;
    newThread->context_regs.rax = 0;
    newThread->kstack_addr = (void *)((size_t)kmalloc(0x8000) + 0x8000);

    spinlock_unlock(&newThread->lock);

    activeThreads[activeThreadID] = newProcess->threads[0];
    totalThreads++;

    return newPid;
}

size_t lseek_syscall(thread_regs_t *regs) { // rdi: fd, rsi: offset, rdx: type
    size_t fd = regs->rdi;
    size_t offset = (size_t)regs->rsi;
    size_t type = regs->rdx;

    process_t *currentProcess = get_active_process(cpuLocals[current_cpu].currentActiveThread);

    spinlock_lock(&currentProcess->filesLock);

    if(fd >= MAX_FILE_HANDLES || !currentProcess->fileHandles[fd] || currentProcess->fileHandles[fd] == (vfs_node_t *)-1) {
        spinlock_unlock(&currentProcess->filesLock);
        return -1;
    }

    vfs_node_t *currentFD = currentProcess->fileHandles[fd];
    size_t ret = vfs_lseek(currentFD, offset, type);

    spinlock_unlock(&currentProcess->filesLock);

    return ret;
}
