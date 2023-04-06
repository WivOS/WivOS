#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include "string.h"

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))
#define ROUND_DOWN(N,S) ((N / S) * S)

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "dN"(port));
}

static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ volatile ("outw %0, %1" :: "a"(val), "dN"(port));
}

static inline void outl(uint16_t port, uint32_t val)
{
    __asm__ volatile ("outl %0, %1" :: "a"(val), "dN"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

static inline uint8_t inw(uint16_t port)
{
    uint16_t val;
    __asm__ volatile ("inw %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

static inline uint8_t inl(uint16_t port)
{
    uint32_t val;
    __asm__ volatile ("inl %1, %0" : "=a"(val) : "dN"(port));
    return val;
}

int printf(const char *format, ...);
int sprintf(const char *str, const char *format, ...);

void *kmalloc(size_t size);
void kfree(void *addr);
void *krealloc(void *addr, size_t size);
void *kcalloc(size_t num, size_t nsize);

uint32_t crc32_calculate_buffer(uint8_t *buffer, size_t size);