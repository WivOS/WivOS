#pragma once

#include <stdatomic.h>
#include <stdbool.h>

#include "common.h"

#define _DEBUG_

#define DEADLOCK_MAX_ITER 0x4000000

#ifdef _DEBUG_

typedef struct {
    const char *file;
    const char *func;
    int line;
} last_acquirer_t;

typedef struct spinlock {
    uint32_t lock;
    last_acquirer_t last_acquirer;
} spinlock_t;

#else

typedef struct spinlock {
    uint32_t lock;
} spinlock_t;

#endif

void qemu_debug_puts_urgent(const char *str);

#define locked_read(type, VAR) ({ \
    type ret = 0; \
    __asm__ __volatile__("lock xadd %0, %1;" : "+r"(ret) : "m"(*(VAR)) : "memory", "cc"); \
    ret; \
})

#define locked_write(type, VAR, value) ({ \
    type ret = value; \
    __asm__ __volatile__("lock xchg %0, %1;" : "+r"(ret) : "m"(*(VAR)) : "memory"); \
    ret; \
})

#define locked_inc(VAR) ({ \
    uint32_t ret = 0; \
    __asm__ __volatile__("lock incw %1;" : "=@ccnz"(ret) : "m"(*(VAR)) : "memory"); \
    ret; \
})

#define locked_dec(VAR) ({ \
    uint32_t ret = 0; \
    __asm__ __volatile__("lock decw %1;" : "=@ccnz"(ret) : "m"(*(VAR)) : "memory"); \
    ret; \
})

#define __puts_uint(val) ({ \
    char buf[21] = {0}; \
    int i; \
    int val_copy = (uint64_t)(val); \
    if (!val_copy) { \
        buf[0] = '0'; \
        buf[1] = 0; \
        i = 0; \
    } else { \
        for (i = 19; val_copy; i--) { \
            buf[i] = (val_copy % 10) + '0'; \
            val_copy /= 10; \
        } \
        i++; \
    } \
    qemu_debug_puts_urgent(buf + i); \
})

#ifdef _DEBUG_

__attribute__((unused)) static uint32_t deadlock_detect_lock = 0;

__attribute__((noinline)) __attribute__((unused)) static void deadlock_detect(const char *file,
                       const char *function,
                       int line,
                       const char *lockname,
                       volatile spinlock_t *lock,
                       size_t iter) {
    while (locked_write(int, &deadlock_detect_lock, 1));
    qemu_debug_puts_urgent("\n---\npossible deadlock at: spinlock_lock(");
    qemu_debug_puts_urgent(lockname);
    qemu_debug_puts_urgent(");");
    qemu_debug_puts_urgent("\nfile: ");
    qemu_debug_puts_urgent(file);
    qemu_debug_puts_urgent("\nfunction: ");
    qemu_debug_puts_urgent(function);
    qemu_debug_puts_urgent("\nline: ");
    __puts_uint(line);
    qemu_debug_puts_urgent("\n---\nlast acquirer:");
    qemu_debug_puts_urgent("\nfile: ");
    qemu_debug_puts_urgent(lock->last_acquirer.file);
    qemu_debug_puts_urgent("\nfunction: ");
    qemu_debug_puts_urgent(lock->last_acquirer.func);
    qemu_debug_puts_urgent("\nline: ");
    __puts_uint(lock->last_acquirer.line);
    qemu_debug_puts_urgent("\n---\nassumed locked after it spun for ");
    __puts_uint(iter);
    qemu_debug_puts_urgent(" iterations\n---\n");
    locked_write(int, &deadlock_detect_lock, 0);
}


#define spinlock_try_lock(LOCK) ({ \
    uint32_t ret; \
    __asm__ __volatile__("lock btrw $0, %0;" : "+m"((LOCK)->lock), "=@ccc"(ret) :: "memory"); \
    if (ret) { \
        (LOCK)->last_acquirer.file = __FILE__; \
        (LOCK)->last_acquirer.func = __func__; \
        (LOCK)->last_acquirer.line = __LINE__; \
    } \
    ret; \
})

#define spinlock_lock(LOCK) ({ \
    __label__ retry; \
    __label__ out; \
retry:; \
    for (size_t i = 0; i < DEADLOCK_MAX_ITER; i++) \
        if (spinlock_try_lock(LOCK)) \
            goto out; \
    deadlock_detect(__FILE__, __func__, __LINE__, #LOCK, LOCK, DEADLOCK_MAX_ITER); \
    goto retry; \
out:; \
})

#else

#define spinlock_try_lock(LOCK) ({ \
    uint32_t ret; \
    __asm__ __volatile__("lock btrw $0, %0;" : "+m"((LOCK)->lock), "=@ccc"(ret) :: "memory"); \
    ret; \
})

#define spinlock_lock(LOCK) ({ \
    __asm__ __volatile__("1: lock btrw $0, %0;" \
                         "jc 2f;" \
                         "pause;" \
                         "jmp 1b; 2:" : "+m"((LOCK)->lock) :: "memory", "cc"); \
})

#endif

#undef __puts_uint

#define spinlock_unlock(LOCK) ({ \
    __asm__ __volatile__("lock btsw $0, %0;" : "+m"((LOCK)->lock) :: "memory", "cc"); \
})

#ifdef _DEBUG_
#define INIT_SPINLOCK() ((spinlock_t){1, (last_acquirer_t){"N/A", "N/A", 0}})
#else
#define INIT_SPINLOCK() ((spinlock_t){1})
#endif