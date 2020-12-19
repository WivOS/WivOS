#pragma once

#include <util/util.h>
#include <fs/vfs.h>

#define MAX_PROCESSES 256
#define MAX_THREADS 256
#define MAX_FILE_HANDLES 512

typedef size_t pid_t;
typedef size_t tid_t;

typedef struct thread_regs {
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8,
			 rdi, rsi, rbp, rdx, rcx, rbx, rax;
	uint64_t rip, cs, rflags;
	uint64_t rsp, ss;
} thread_regs_t;

#define BASE_BRK_LOCATION ((size_t)0x0000780000000000)

typedef struct thread {
    volatile thread_regs_t context_regs;
    volatile tid_t tid;
    volatile tid_t schedTid;
    volatile pid_t pid;
    volatile void *kstack_addr;
    volatile void *ustack_addr;
    volatile size_t cpuNumber;
    volatile spinlock_t lock;
    volatile uint32_t guardPadding;
    volatile uint8_t fxstate[512] __attribute__((aligned(16)));
} thread_t;

typedef struct process {
    volatile pid_t pid;
    volatile void *process_pml4;
    volatile thread_t **threads;
    volatile spinlock_t process_lock;
    volatile spinlock_t brkLock;
    volatile uint64_t brkAddress;
    vfs_node_t **fileHandles;
    volatile spinlock_t filesLock;
} process_t;

void scheduler_init();

void schedule(thread_regs_t *regs);
pid_t proc_create(void *pml4);
tid_t thread_create(pid_t pid, void *function);
thread_t *get_active_thread(tid_t activeThread);
process_t *get_active_process(tid_t activeThread);