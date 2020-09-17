#pragma once

#include <stdatomic.h>
#include <stdbool.h>

#define locked_inc(var) ({ \
    int ret; \
    asm volatile ( \
        "lock inc %1;" \
        : "=@ccnz" (ret) \
        : "m" (*(var)) \
        : "memory" \
    ); \
    ret; \
})

#define locked_dec(var) ({ \
    int ret; \
    asm volatile ( \
        "lock dec %1;" \
        : "=@ccnz" (ret) \
        : "m" (*(var)) \
        : "memory" \
    ); \
    ret; \
})

typedef struct lock {
    atomic_size_t lock;
    atomic_size_t next_lock;
} spinlock_t;

#define INIT_LOCK() ((spinlock_t){0})

void spinlock_lock(spinlock_t* lock);
bool spinlock_try_lock(spinlock_t* lock);
void spinlock_unlock(spinlock_t* lock);