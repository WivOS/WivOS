#pragma once

#include "common.h"

void *memset(void *s, int c, size_t n);
void *memcpy(void *d, const void *s, size_t n);
void *memcpy64(void *dest, const void *src, size_t n);

int memcmp(const void *ptr1, const void *ptr2, size_t n);
void *memmove(void *dest, const void *src, size_t n);

int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);

char *strchrnul(const char *s, int c);

size_t strlen(char *str);
char *strdup(char *str);
void strcpy(char *d, char *s);

char *strsep(char **stringPointer, const char *delimiter);

char *strstr(const char *in, const char *str);

char *strtok_r(char * str, const char * delim, char ** saveptr);