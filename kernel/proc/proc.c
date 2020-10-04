#include "proc.h"
#include "smp.h"
#include <acpi/apic.h>
#include <util/util.h>

volatile static spinlock_t schedulerLock = INIT_LOCK();
volatile static spinlock_t syscallsLock = INIT_LOCK();
volatile static spinlock_t syscalls2Lock = INIT_LOCK();

volatile static process_t **processes = NULL;
volatile static thread_t **activeThreads = NULL;
volatile static size_t totalThreads = 0;
volatile int sched_ready = 0;

void thread_main() {
    while(1) {
    }
}

void scheduler_init() {
    processes = kcalloc(sizeof(process_t *), MAX_PROCESSES);

    processes[0] = kmalloc(sizeof(process_t));
    processes[0]->threads = kcalloc(sizeof(thread_t *), MAX_THREADS);
    processes[0]->process_pml4 = kernel_pml4;
    processes[0]->pid = 0;
    for(size_t i = 0; i < MAX_THREADS; i++) {
        processes[0]->threads[i] = NULL;
    }
    
    activeThreads = kcalloc(sizeof(thread_t *), 4096);
    for(size_t i = 0; i < 4096; i++) {
        activeThreads[i] = NULL;
    }

    sched_ready = 1;
}

void task_return_context(void *);

__attribute__((noinline)) static void _idle(void) {
    int _current_cpu = current_cpu;
    cpuLocals[_current_cpu].currentActiveThread = -1;
    cpuLocals[_current_cpu].currentProcess = -1;
    cpuLocals[_current_cpu].currentThread = -1;
    lapic_write(0xB0, 0);
    spinlock_unlock(&schedulerLock);
    spinlock_unlock(&syscallsLock);
    asm volatile(
        "sti;"
        "1: "
        "hlt;"
        "jmp 1b;"
    );
}

__attribute__((noinline)) static void idle(void) {
    asm volatile(
        "movq %%cr3, %%rbx;"
        "cmp %%rax, %%rbx;"
        "je 1f;"
        "movq %%rax, %%cr3;"
        "1: "
        "movq %%gs:(8), %%rsp;"
        "jmp _idle;"
        :
        : "a" ((size_t)kernel_pml4)
    );
    _idle();
}

static volatile size_t currentActiveThread = -1;

volatile thread_t *get_next_task() {
    if(currentActiveThread != -1) {
        currentActiveThread++;
    } else {
        currentActiveThread = 0;
    }

    for(int64_t i = 0; i < totalThreads; ) {
        volatile thread_t *thread = activeThreads[currentActiveThread];
        if(!thread) {
            currentActiveThread = 0;
            thread = activeThreads[currentActiveThread];
            if(!thread) {
                break;
            }
        }
        if((void *)thread == (void *)-1) {
            goto skip;
        }
        if(!spinlock_try_lock(&thread->lock)) {
            goto next;
        }
        return thread;
        next:
        i++;
        skip:
        if(++currentActiveThread == 4096)
            currentActiveThread = 0;
    }

    return NULL;
}

void schedule(thread_regs_t *regs) {
    spinlock_lock(&schedulerLock);
    //printf("Scheduling cpu %u\n", current_cpu);

    if(!spinlock_try_lock(&syscallsLock)) {
        spinlock_unlock(&schedulerLock);
        return;
    }

    cpu_t *cpuLocal = &cpuLocals[current_cpu];
    pid_t currentProcess = cpuLocal->currentProcess;
    tid_t currentThread = cpuLocal->currentThread;
    currentActiveThread = cpuLocal->currentActiveThread;

    if(currentActiveThread != -1) {
        volatile thread_t *current_thread = activeThreads[currentActiveThread];
        if(current_thread == NULL) { // TODO
            goto skip_invalid_thread_context_save;
        }
        current_thread->context_regs = *regs;
        processes[currentProcess]->threads[currentThread]->cpuNumber = -1;
        //TODO: FPU and add spinlocks to the threads
        spinlock_unlock(&current_thread->lock);
    }
skip_invalid_thread_context_save:
    (void)0;

    volatile thread_t *next_task = get_next_task();
    if(next_task == NULL) {
        //printf("idle %x\n", current_cpu);
        idle();
    }

    /*if(currentActiveThread != next_task->schedTid)
        printf("Schedule %x\n", current_cpu);*/

    cpuLocal->currentProcess = next_task->pid;
    cpuLocal->currentThread = next_task->tid;
    cpuLocal->currentActiveThread = currentActiveThread;

    if(next_task->pid != 0) {
        //TODO User mode
    }

    next_task->cpuNumber = current_cpu;
    asm volatile("cli");
    if(currentProcess == -1 || processes[next_task->pid]->process_pml4 != processes[currentProcess]->process_pml4) {
        asm volatile ("mov %%rax, %%cr3" : : "a"(processes[next_task->pid]->process_pml4) : "memory");
    }
    spinlock_unlock(&schedulerLock);
    spinlock_unlock(&syscallsLock);
    task_return_context((thread_regs_t *)&next_task->context_regs);
    //TODO, change context
}

void thread_create(pid_t pid, void *function) {
    process_t *proc = (process_t *)processes[pid];
    if(!proc) return;

    spinlock_lock(&schedulerLock);

    tid_t tid = 0;
    for(; tid < MAX_THREADS; tid++) {
        if(!proc->threads[tid] || proc->threads[tid] == (void *)-1) {
            break;
        }
        if(tid == MAX_THREADS - 1) {
            spinlock_unlock(&schedulerLock);
            return;
        }
    }
    proc->threads[tid] = (void *)-2;

    size_t activeThreadID = 0;
    for(; activeThreadID < 4096; activeThreadID++) {
        if(!activeThreads[activeThreadID] || activeThreads[activeThreadID] == (void *)-1) {
            break;
        }
        if(activeThreadID == 4095) {
            proc->threads[tid] = NULL;
            spinlock_unlock(&schedulerLock);
            return;
        }
    }
    activeThreads[activeThreadID] = (void *)-2;

    thread_t *thread = kmalloc(sizeof(thread_t));

    thread->lock = INIT_LOCK();
    thread->cpuNumber = -1;
    thread->pid = pid;
    thread->tid = tid;
    thread->schedTid = activeThreadID;
    thread->context_regs = (thread_regs_t){0};
    thread->context_regs.cs = 0x08;
    thread->context_regs.rflags = 0x202;
    thread->context_regs.ss = 0x10;
    thread->context_regs.rip = (uint64_t)function;
    thread->kstack_addr = (void *)kmalloc(4096) + 4096;
    thread->kstack_addr -= sizeof(uint64_t);
    *((size_t *)thread->kstack_addr) = 0;

    thread->context_regs.rsp = (uint64_t)thread->kstack_addr;

    activeThreads[activeThreadID] = thread;
    totalThreads++;

    spinlock_unlock(&schedulerLock);
}