#include "process.h"
#include "thread.h"
#include "scheduler.h"

kpid_t process_create(pt_t *page_table) {
    kpid_t pid = 0;
    for(; pid < MAX_PROCESSES; pid++) {
        if(SchedulerProcesses[pid] == NULL) {
            goto done;
        }
    }
    printf("Max process number allocated reached\n");
    return -1;

done: (void)pid;

    process_t *newProcess = kmalloc(sizeof(process_t));
    newProcess->threads = kcalloc(sizeof(thread_t), MAX_THREADS);
    newProcess->page_table = page_table;
    newProcess->pid = pid;
    for(size_t i = 0; i < MAX_THREADS; i++) newProcess->threads[i] = NULL;

    newProcess->current_alloc_address = BASE_ALLOC_ADDRESS;
    newProcess->alloc_lock = INIT_SPINLOCK();

    newProcess->file_handles = kcalloc(sizeof(vfs_node_t *), MAX_FILE_HANDLES);
    for(size_t i = 0; i < MAX_FILE_HANDLES; i++) newProcess->file_handles[i] = (void *)-1;
    newProcess->file_handles_lock = INIT_SPINLOCK();

    newProcess->shm_alloc = USER_SHM_LOW;

    newProcess->lock = INIT_SPINLOCK();

    SchedulerProcesses[pid] = newProcess;
    return pid;
}