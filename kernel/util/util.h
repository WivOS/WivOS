#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "./string.h"
#include "lock.h"
#include <mem/mm.h>
#include "list.h"

void outb(uint16_t p, uint8_t d);
void outw(uint16_t p, uint16_t d);
void outl(uint16_t p, uint32_t d);

uint8_t inb(uint16_t p);
uint16_t inw(uint16_t p);
uint32_t inl(uint16_t p);

void debug_out_char(char o);

int printf(const char* format, ...);
int sprintf(char* buffer, const char* format, ...);

void ksleep(size_t ms);