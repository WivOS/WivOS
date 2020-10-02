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

#define spinlock_lock(LOCK) ({ \
    asm volatile ( \
        "1: " \
        "lock btrw $0, %0;" \
        "jc 2f;" \
        "pause;" \
        "jmp 1b;" \
        "2: " \
        : "+m" ((LOCK)->lock) \
        : \
        : "memory", "cc" \
    ); \
})

#define spinlock_unlock(LOCK) ({ \
    asm volatile ( \
        "lock btsw $0, %0;" \
        : "+m" ((LOCK)->lock) \
        : \
        : "memory", "cc" \
    ); \
})

#define spinlock_try_lock(LOCK) ({ \
    int ret; \
    asm volatile ( \
        "lock btrw $0, %0;" \
        : "+m" ((LOCK)->lock), "=@ccc" (ret) \
        : \
        : "memory" \
    ); \
    ret; \
})

typedef struct lock {
    int lock;
} spinlock_t;

#define INIT_LOCK() ((spinlock_t){1})