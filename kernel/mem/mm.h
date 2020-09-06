#pragma once

#include <mem/vmm.h>

void *kmalloc(size_t size);
void kfree(void *block);
void *krealloc(void *addr, size_t newSize);
void *kcalloc(size_t num, size_t nsize);