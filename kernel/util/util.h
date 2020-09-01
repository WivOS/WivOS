#pragma once

#include <stdint.h>

void outb(uint16_t p, uint8_t d);
void outw(uint16_t p, uint16_t d);
void outl(uint16_t p, uint32_t d);

uint8_t inb(uint16_t p);
uint16_t inw(uint16_t p);
uint32_t inl(uint16_t p);