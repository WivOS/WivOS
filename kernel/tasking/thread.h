#pragma once

#include <utils/common.h>
#include <stdbool.h>

#include <cpu/idt.h>
#include <utils/spinlock.h>

#include <modules/modules.h>

#include "event.h"

#undef tid_t
#undef pid_t
typedef size_t ktid_t;
typedef size_t kpid_t;

typedef struct thread {
    volatile irq_regs_t saved_regs;
    volatile uint32_t guardValue;
    volatile ktid_t tid;
    volatile kpid_t pid;
    volatile ktid_t taskID;
    volatile void *kstack_address;
    volatile void *ustack_address;
    volatile size_t cpu_number;
    volatile spinlock_t lock;
    volatile size_t fs_base;
    volatile uint8_t *fxstate;
    event_t **event_pointer;
    bool *out_event_pointer;
    size_t event_timeout;
    size_t event_count;
    event_t event_abort;

    uint64_t yield_target;
    void *memoryStackPhys;
    uint32_t dont_save;
} __attribute__((packed)) thread_t;

typedef enum {
    thread_parameter_entry,
    thread_parameter_exec
} thread_parameter_types_t;

typedef struct {
    void *entry;
    void *arg;
} thread_parameter_entry_data_t;

typedef struct {
    void *entry;
    const char **argv;
    const char **envp;
    elf_value_t *value;
} thread_parameter_exec_data_t;

#define thread_entry_data(entry_, arg_) &((thread_parameter_entry_data_t){.entry=entry_, .arg=arg_})
#define thread_call_data(entry_, argv_, envp_, value_) \
    &((thread_parameter_exec_data_t){.entry=entry_, .argv=argv_, .envp=envp_, .value=value_, })

ktid_t thread_create(kpid_t pid, thread_parameter_types_t dataType, void *data);
ktid_t thread_recreate(kpid_t pid, kpid_t tid, thread_parameter_types_t dataType, void *data, bool free_args);