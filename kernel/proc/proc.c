#include "proc.h"
#include "smp.h"
#include <acpi/apic.h>
#include <util/util.h>

volatile spinlock_t schedulerLock = INIT_LOCK();
volatile static spinlock_t syscallsLock = INIT_LOCK();
volatile static spinlock_t syscalls2Lock = INIT_LOCK();

volatile static process_t **processes = NULL;
volatile thread_t **activeThreads = NULL;
volatile size_t totalThreads = 0;
volatile int sched_ready = 0;

volatile static uint8_t default_fxstate[512] __attribute__((aligned(16)));

thread_t *get_active_thread(tid_t activeThread) {
    if(activeThread >= 4096) return NULL;

    return (thread_t *)activeThreads[activeThread];
}

process_t *get_active_process(pid_t pid) {
    if(pid >= 4096) return NULL;

    return (process_t *)processes[pid];
}

void thread_main() {
    while(1) {
        asm volatile("hlt");
    }
}

#define fxsave(ptr) ({ \
    asm volatile ( \
                "fxsave %0;" \
                : \
                : "m" (*(ptr)) \
    ); \
})

#define fxrstor(ptr) ({ \
    asm volatile ( \
                "fxrstor %0;" \
                : \
                : "m" (*(ptr)) \
    ); \
})

void scheduler_init() {
    processes = kcalloc(sizeof(process_t *), MAX_PROCESSES);
    fxsave(&default_fxstate);

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

void task_return_context(void *, void *);

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

extern volatile size_t pit_count;

volatile thread_t *get_next_task(size_t currentActiveThread) {
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
        if(thread->eventPtr) {
            int wake = 0;
            for(int i = 0; i < (thread->eventNum); i++) {
                if(locked_read(event_t, thread->eventPtr[i])) {
                    wake = 1;
                    locked_dec(thread->eventPtr[i]);
                    thread->outEventPtr[i] = 1;
                }

                if(thread->eventTimeout <= pit_count && (thread->eventTimeout != 0) && !wake) {
                    thread->eventPtr = 0;
                    thread->eventTimeout = 0;
                    wake = 1;
                }

                if(wake) {
                    thread->eventPtr = 0;
                } else {
                    spinlock_unlock(&thread->lock);
                    goto next;
                }
            }
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

    volatile cpu_t *cpuLocal = &cpuLocals[current_cpu];
    volatile pid_t currentProcess = cpuLocal->currentProcess;
    volatile tid_t currentThread = cpuLocal->currentThread;
    volatile size_t currentActiveThread = cpuLocal->currentActiveThread;

    if(currentActiveThread != -1) {
        volatile thread_t *current_thread = activeThreads[currentActiveThread];
        if(current_thread == NULL) { // TODO
            goto skip_invalid_thread_context_save;
        }
        current_thread->context_regs = *regs;
        current_thread->cpuNumber = -1;
        if(currentProcess) {
            //printf("RBP: %lx\n", cpuLocal->threadUserStack);
            current_thread->ustack_addr = (void *)cpuLocal->threadUserStack;
            fxsave(&current_thread->fxstate);
        }
        //TODO: FPU and add spinlocks to the threads
        spinlock_unlock(&current_thread->lock);
    }
skip_invalid_thread_context_save:
    (void)0;

    volatile thread_t *next_task = get_next_task(currentActiveThread);
    if(next_task == NULL) {
        //printf("idle %x\n", current_cpu);
        idle();
    }

    /*if(currentActiveThread != next_task->schedTid)
        printf("Schedule %x\n", current_cpu);*/

    cpuLocal->currentProcess = next_task->pid;
    cpuLocal->currentThread = next_task->tid;
    cpuLocal->currentActiveThread = next_task->schedTid;

    if(next_task->pid != 0) {
        cpuLocal->threadKernelStack = (size_t)next_task->kstack_addr;
        cpuLocal->threadUserStack = (size_t)next_task->ustack_addr;
        fxrstor(&next_task->fxstate);
        //TODO User mode
    }

    next_task->cpuNumber = current_cpu;
    asm volatile("cli");
    if(currentProcess == -1 || processes[next_task->pid]->process_pml4 != processes[currentProcess]->process_pml4) {
        spinlock_unlock(&schedulerLock);
        spinlock_unlock(&syscallsLock);
        task_return_context((thread_regs_t *)&next_task->context_regs, (void *)processes[next_task->pid]->process_pml4);
    } else {
        spinlock_unlock(&schedulerLock);
        spinlock_unlock(&syscallsLock);
        task_return_context((thread_regs_t *)&next_task->context_regs, 0);
    }
    //TODO, change context
}

extern volatile void force_resched();

void yield() {
    spinlock_lock(&schedulerLock);
    force_resched();
}

pid_t proc_create(void *pml4) {
    spinlock_lock(&schedulerLock);

    pid_t pid = 0;
    for(size_t i = 0; i < MAX_PROCESSES; i++) {
        if(processes[i] == 0) {
            pid = i;
            break;
        }
    }

    if(pid == 0) {
        spinlock_unlock(&schedulerLock);
        return -1; // Error: max processes reached.
    }

    processes[pid] = kmalloc(sizeof(process_t));
    processes[pid]->threads = kcalloc(sizeof(thread_t *), MAX_THREADS);
    processes[pid]->process_pml4 = pml4;
    processes[pid]->pid = pid;
    for(size_t i = 0; i < MAX_THREADS; i++) {
        processes[pid]->threads[i] = NULL;
    }

    processes[pid]->brkAddress = BASE_BRK_LOCATION;
    spinlock_unlock(&processes[pid]->brkLock);

    processes[pid]->fileHandles = (vfs_node_t **)kmalloc(sizeof(vfs_node_t *) * MAX_FILE_HANDLES);
    spinlock_unlock(&processes[pid]->filesLock);

    spinlock_unlock(&schedulerLock);

    return pid;
}

#define STACK_LOCATION_TOP ((size_t)0x0000800000000000)

tid_t thread_create(pid_t pid, void *function) {
    process_t *proc = (process_t *)processes[pid];
    if(!proc) return -1;

    spinlock_lock(&schedulerLock);

    tid_t tid = 0;
    for(; tid < MAX_THREADS; tid++) {
        if(!proc->threads[tid] || proc->threads[tid] == (void *)-1) {
            break;
        }
        if(tid == MAX_THREADS - 1) {
            spinlock_unlock(&schedulerLock);
            return -1;
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
            return -1;
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
    if(!pid) {
        thread->context_regs.cs = 0x08;
        thread->context_regs.rflags = 0x202;
        thread->context_regs.ss = 0x10;
    } else {
        thread->context_regs.cs = 0x23;
        thread->context_regs.rflags = 0x202;
        thread->context_regs.ss = 0x1b;
    }
    thread->context_regs.rip = (uint64_t)function;
    thread->kstack_addr = (void *)kmalloc(0x8000) + 0x8000;
    thread->kstack_addr -= sizeof(uint64_t);
    *((size_t *)thread->kstack_addr) = 0;

    if(pid) {
        size_t stackGuardPage = STACK_LOCATION_TOP - (0x1000 * 4 + 0x1000) * (tid + 1);
        size_t stackBottom = stackGuardPage + PAGE_SIZE;

        void *memoryStackAddr = (void *)pmm_alloc(4);

        vmm_map_pages((void *)proc->process_pml4, (void *)(stackBottom), memoryStackAddr, 4, 0x7);
        vmm_unmap_pages((void *)proc->process_pml4, (void *)stackGuardPage, 1);

        thread->context_regs.rsp = (uint64_t)(stackBottom) + 0x1000 * 4 - 8;
    } else {
        thread->context_regs.rsp = (uint64_t)thread->kstack_addr;
    }

    memcpy((void *)thread->fxstate, (void *)default_fxstate, 512);

    spinlock_unlock(&thread->lock);

    activeThreads[activeThreadID] = thread;
    totalThreads++;

    spinlock_unlock(&schedulerLock);

    return tid;
}