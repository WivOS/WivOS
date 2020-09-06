#pragma once

#include <stdint.h>
#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *d, void *s, size_t n);

char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
size_t strlen(const char *str);