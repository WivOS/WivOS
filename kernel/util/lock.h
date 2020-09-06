#pragma once

#include <stdatomic.h>
#include <stdbool.h>

typedef struct lock {
    atomic_size_t lock;
    atomic_size_t next_lock;
} spinlock_t;

#define INIT_LOCK() ((spinlock_t){0})

void spinlock_lock(spinlock_t* lock);
bool spinlock_try_lock(spinlock_t* lock);
void spinlock_unlock(spinlock_t* lock);