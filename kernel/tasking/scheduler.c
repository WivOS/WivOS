#include "scheduler.h"
#include "process.h"
#include "thread.h"

#include <cpu/cpu.h>
#include <cpu/inlines.h>
#include <devices/timer.h>

#include "event.h"

bool SchedulerRunning = false;
process_t *SchedulerProcesses[MAX_PROCESSES];
volatile thread_t *ActiveTasks[MAX_THREADS*2];
static size_t TotalActiveTasks = 0;

volatile spinlock_t ReSchedulerLock = INIT_SPINLOCK();
volatile spinlock_t SchedulerLock = INIT_SPINLOCK();
volatile spinlock_t SecondSchedulerLock = INIT_SPINLOCK();

uint8_t* DefaultFxstate;

static bool smp_resched(irq_regs_t *regs) {
    lapic_write(0xB0, 0);
    scheduler_schedule(regs);

    return false;
}

void scheduler_init(void *first_task) {
    DefaultFxstate = kmalloc(CpuSimdRegionSize);

    cpu_save_simd(DefaultFxstate);

    for(kpid_t i = 0; i < MAX_PROCESSES; i++) SchedulerProcesses[i] = (process_t *)NULL;

    for(ktid_t i = 0; i < MAX_THREADS * 2; i++) ActiveTasks[i] = (thread_t *)NULL;

    IRQFunctions[0x20] = smp_resched;

    //Invoke the first call for the kernel pid -> 0x0
    process_create(KernelPml4);

    scheduler_add_task(0, thread_create(0, thread_parameter_entry, thread_entry_data((void *)first_task, 0)));

    SchedulerRunning = true;
}

extern volatile size_t TimerCounter;
static volatile thread_t *scheduler_get_next_task(kpid_t currentTask) {
    currentTask++;

    for(size_t i = 0; i < TotalActiveTasks; ) {
        volatile thread_t *thread = ActiveTasks[currentTask];
        if(thread == NULL) {
            currentTask = 0;
            thread = ActiveTasks[currentTask];
            if(thread == NULL) break; //There are no more tasks
        }
        if((void *)thread == (void *)-1) goto skip; //This task is deleted;
        if(thread->yield_target > TimerCounter) goto next;
        if(!spinlock_try_lock(&thread->lock)) goto next;
        if(thread->event_pointer) {
            if(!thread->event_abort) {
                bool wake = false;
                for(size_t i = 0; i < thread->event_count; i++) {
                    if(locked_read(event_t, thread->event_pointer[i])) {
                        wake = true;
                        locked_dec(thread->event_pointer[i]);
                        thread->out_event_pointer[i] = true;
                    }
                }

                if(thread->event_timeout <= TimerCounter && (thread->event_timeout != 0) && !wake) {
                    thread->event_pointer = NULL;
                    thread->event_timeout = 0;
                    wake = true;
                }

                if(wake) {
                    thread->event_pointer = NULL;
                } else {
                    spinlock_unlock(&thread->lock);
                    goto next;
                }
            }
        }
        return thread;
        next: i++;
        skip: if(++currentTask == MAX_THREADS*2) currentTask = 0;
    }

    return NULL;
}

__attribute__((noinline)) static void scheduler_idle_halt() {
    int _CurrentCPU = CurrentCPU;
    CPULocals[_CurrentCPU].currentTaskID = -1;
    CPULocals[_CurrentCPU].currentPid = -1;
    CPULocals[_CurrentCPU].currentTid = -1;

    spinlock_unlock(&SecondSchedulerLock);
    spinlock_unlock(&SchedulerLock);
    spinlock_unlock(&ReSchedulerLock);
    __asm__ __volatile__("sti; 1: hlt; jmp 1b;");
}

__attribute__((noinline)) static void scheduler_idle() {
    __asm__ __volatile__(
        "movq %%cr3, %%rbx;"
        "cmp %%rax, %%rbx;"
        "je 1f;"
        "movq %%rax, %%cr3;"
        "1: movq %%gs:(8), %%rsp;"
        "jmp scheduler_idle_halt;" :: "a" ((size_t)KernelPml4->entries)
    );
    scheduler_idle_halt(); //Do this to let c understand to compile the above function
}

extern void scheduler_switch_task(volatile irq_regs_t *regs, volatile pt_entries_t *page_table);
int printf_scheduler(const char *format, ...);

void scheduler_schedule(irq_regs_t *regs) {
    spinlock_lock(&ReSchedulerLock);

    asm("cli");
    if(!spinlock_try_lock(&SchedulerLock)) {
        spinlock_unlock(&ReSchedulerLock);
        return;
    }

    if(!spinlock_try_lock(&SecondSchedulerLock)) {
        spinlock_unlock(&SchedulerLock);
        spinlock_unlock(&ReSchedulerLock);
        return;
    }

    volatile cpu_t *cpuLocal = &CPULocals[CurrentCPU];
    volatile kpid_t currentPid = cpuLocal->currentPid;
    volatile ktid_t currentTid = cpuLocal->currentTid;
    volatile ktid_t currentTaskID = cpuLocal->currentTaskID;

    if(currentTaskID != -1) {
        volatile thread_t *thread = ActiveTasks[currentTaskID];
        if(thread == NULL || thread == (void *)-1) goto skip;
        //if(thread->dont_save != false) {
        //    thread->dont_save = false;
        //    goto skip;
        //}

        //memcpy((void *)&thread->saved_regs, regs, sizeof(irq_regs_t));
        thread->saved_regs = *regs;
        thread->cpu_number = -1;

        cpu_save_simd((uint8_t *)thread->fxstate);
        if(currentPid) { //User mode
            thread->ustack_address = (void *)cpuLocal->threadUserStack;
        }

        spinlock_unlock(&thread->lock);
    }
    skip: (void)0;

    volatile thread_t *nextTask = scheduler_get_next_task(currentTaskID);
    if(nextTask == NULL) {
        scheduler_idle();
    }

    cpuLocal->currentPid = nextTask->pid;
    cpuLocal->currentTid = nextTask->tid;
    cpuLocal->currentTaskID = nextTask->taskID;

    cpu_restore_simd((uint8_t *)nextTask->fxstate);
    if(nextTask->pid != 0) {
        cpuLocal->threadKernelStack = (size_t)nextTask->kstack_address;
        cpuLocal->threadUserStack = (size_t)nextTask->ustack_address;

        load_fs_base(nextTask->fs_base);
    }

    nextTask->cpu_number = CurrentCPU;

    //printf_scheduler("Scheduling #%d #%d: Task %d 0x%llx\n", CurrentCPU, nextTask->pid, nextTask->taskID, SchedulerProcesses[nextTask->pid]->page_table);
    if(currentPid == -1 || SchedulerProcesses[nextTask->pid]->page_table != SchedulerProcesses[currentPid]->page_table) {
        scheduler_switch_task(&nextTask->saved_regs, SchedulerProcesses[nextTask->pid]->page_table->entries);
    } else {
        scheduler_switch_task(&nextTask->saved_regs, NULL);
    }
}

void scheduler_schedule_smp(irq_regs_t *regs) {
    lapic_write(0xB0, 0);

    for(size_t i = 1; i < MAX_CPUS; i++) {
        if(CPULocals[i].currentCpu == -1) break;

        lapic_write(0x310, CPULocals[i].lapicID << 24);
        lapic_write(0x300, RESCHED_IPI_IRQ); //INIT destination
    }

    scheduler_schedule(regs);
}

ktid_t scheduler_add_task(kpid_t pid, ktid_t tid) {
    process_t *process = SchedulerProcesses[pid];
    if(process == NULL) return -1;

    spinlock_lock(&process->lock);

    volatile thread_t *thread = process->threads[tid];
    if(thread == NULL || thread->taskID != -1) {
        spinlock_unlock(&process->lock);
        return -1;
    }

    spinlock_lock(&ReSchedulerLock);
    ktid_t taskID = 0;
    for(; taskID < MAX_THREADS*2; taskID++) {
        if(ActiveTasks[taskID] == NULL || ActiveTasks[taskID] == (void *)-1) {
            thread->taskID = taskID;
            ActiveTasks[taskID] = thread;
            TotalActiveTasks++;

            spinlock_unlock(&ReSchedulerLock);
            spinlock_unlock(&process->lock);
            return taskID;
        }
    }
    printf("Max task number allocated reached\n");
    spinlock_unlock(&ReSchedulerLock);
    spinlock_unlock(&process->lock);
    return -1;
}

extern void force_reschedule();
void yield() {
    spinlock_lock(&SchedulerLock);
    force_reschedule();
}

void thread_sleep(size_t ms) {
    spinlock_lock(&SchedulerLock);
    uint32_t yieldTarget = TimerCounter + ( (ms * TIMER_FREQ) / 1000);

    volatile thread_t *currentThread = ActiveTasks[CPULocals[CurrentCPU].currentTaskID];
    currentThread->yield_target = yieldTarget;

    force_reschedule();
}

kpid_t scheduler_get_current_pid() {
    spinlock_lock(&SchedulerLock);
    kpid_t pid = CPULocals[CurrentCPU].currentPid;
    spinlock_unlock(&SchedulerLock);

    return pid;
}