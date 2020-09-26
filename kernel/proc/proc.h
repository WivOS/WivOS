#pragma once

#include <util/util.h>

#define MAX_PROCESSES 256
#define MAX_THREADS 256

typedef size_t pid_t;
typedef size_t tid_t;

typedef struct thread_regs {
	uint64_t ds;
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8,
			 rdi, rsi, rbp, rdx, rcx, rbx, rax;
	uint64_t rip, cs, rflags;
	uint64_t rsp, ss;
} thread_regs_t;

typedef struct thread {
    tid_t tid;
    void *stack_addr;
    thread_regs_t context_regs;
} thread_t;

typedef struct process {
    pid_t pid;
    void *process_pml4;
    thread_t **threads;
    spinlock_t process_lock;
} process_t;

void scheduler_init();