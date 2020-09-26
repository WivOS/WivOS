#include "proc.h"

static process_t **processes = NULL;

void scheduler_init() {
    processes = kcalloc(sizeof(process_t *), MAX_PROCESSES);

    processes[0] = kmalloc(sizeof(process_t));
    processes[0]->threads = kcalloc(sizeof(thread_t *), MAX_THREADS);
    processes[0]->process_pml4 = kernel_pml4;
    processes[0]->pid = 0;
}

void schedule(thread_regs_t *regs) {
    
}