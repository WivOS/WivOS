#include <util/lock.h>
#include <stddef.h>

void spinlock_lock(spinlock_t* spinlock) {
    size_t lock = atomic_fetch_add_explicit(&spinlock->next_lock, 1, memory_order_relaxed);
    while(atomic_load_explicit(&spinlock->lock, memory_order_acquire) != lock) {
        asm volatile("hlt");
    }
}

bool spinlock_try_lock(spinlock_t* spinlock) {
    size_t lock = atomic_load_explicit(&spinlock->lock, memory_order_relaxed);
    return atomic_compare_exchange_strong_explicit(&spinlock->lock, &lock, lock + 1, memory_order_acquire, memory_order_relaxed);
}

void spinlock_unlock(spinlock_t* spinlock) {
    size_t lock = atomic_load_explicit(&spinlock->lock, memory_order_relaxed) + 1;
    atomic_store_explicit(&spinlock->lock, lock, memory_order_release);
}