#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util/list.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *d, void *s, size_t n);

char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
size_t strlen(const char *str);
char *strtok(char *s, char *delim);
char *strdup(const char *src);
int strcmp(const char *dst, char *src);
char *strsep(char **stringp, const char *delim);

list_t *strtoklist(char *s, char *delim, size_t *nTokens);
char *list2str(list_t *list, const char* delim);