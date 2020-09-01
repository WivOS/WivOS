#include "util.h"

void outb(uint16_t p, uint8_t d) {
    asm volatile("outb %b0, %w1" :: "a"(d), "Nd"(p));
}

void outw(uint16_t p, uint16_t d) {
    asm volatile("outw %w0, %w1" :: "a"(d), "Nd"(p));
}

void outl(uint16_t p, uint32_t d) {
    asm volatile("outl %0, %w1" :: "a"(d), "Nd"(p));
}

uint8_t inb(uint16_t p) {
    uint8_t retVal;
    asm volatile("inb %w1, %b0" : "=a"(retVal) : "Nd"(p));
    return retVal;
}

uint16_t inw(uint16_t p) {
    uint16_t retVal;
    asm volatile("inw %w1, %w0" : "=a"(retVal) : "Nd"(p));
    return retVal;
}

uint32_t inl(uint16_t p) {
    uint32_t retVal;
    asm volatile("inl %w1, %0" : "=a"(retVal) : "Nd"(p));
    return retVal;
}