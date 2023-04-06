#pragma once

#include <utils/common.h>
#include <stdbool.h>
#include <cpu/idt.h>

#include "process.h"

#define MAX_PROCESSES 256
#define MAX_THREADS 256

#define MAX_MODULE_SYSCALLS 0x10000

extern bool SchedulerRunning;
extern process_t *SchedulerProcesses[MAX_PROCESSES];
extern volatile thread_t *ActiveTasks[MAX_THREADS*2];

extern volatile spinlock_t ReSchedulerLock;
extern volatile spinlock_t SchedulerLock;

void scheduler_init(void *first_task);
ktid_t scheduler_add_task(kpid_t pid, ktid_t tid);

void scheduler_schedule(irq_regs_t *regs);

kpid_t scheduler_get_current_pid();

bool syscall_privilege_check(void *base, size_t size);

void yield();

typedef size_t (*syscall_fn_t)(irq_regs_t *);
bool syscall_install(syscall_fn_t func, size_t index);

#define SYSCALL_ID(module, id) ((((size_t)(uint8_t)module & 0xFF) << 8) | ((size_t)id & 0xFF))