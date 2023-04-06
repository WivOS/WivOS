#include "string.h"
#include <stdbool.h>

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memcpy(void *d, const void *s, size_t n) {
    uint8_t *dest = (uint8_t *)d; uint8_t *source = (uint8_t *)s;
    for(;n;n--) *dest++ = *source++;
    return d;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, size_t n) {
    uint8_t *s1 = (uint8_t *)ptr1; uint8_t *s2 = (uint8_t *)ptr2;
    for(;n;n--) {
        uint8_t a = *s1++;
        uint8_t b = *s2++;
        if(a == b) continue;
        if(a < b) return -1;
        if(a > b) return 1;
    }

    return 0;
}

int strcmp(const char *str1, const char *str2) {
    uint8_t *s1 = (uint8_t *)str1; uint8_t *s2 = (uint8_t *)str2;
    while(true) {
        uint8_t a = *s1++;
        uint8_t b = *s2++;
        if(a == b && a != 0) continue;
        if(a < b) return -1;
        if(a > b) return 1;
        if(!a || !b) break;;
    }

    return 0;
}

int strncmp(const char *str1, const char *str2, size_t n) {
    uint8_t *s1 = (uint8_t *)str1; uint8_t *s2 = (uint8_t *)str2;
    for(;n;n--) {
        uint8_t a = *s1++;
        uint8_t b = *s2++;
        if(a == b && a != 0) continue;
        if(a < b) return -1;
        if(a > b) return -1;
        if(!a || !b) break;;
    }

    return 0;
}

char *strchrnul(const char *s, int c) {
    while(*s)
        if((*s++) == c) break;

    return (char *)s;
}

size_t strlen(char *str) {
    size_t len = 0;
    for(;*str++;len++);
    return len;
}

void strcpy(char *d, char *s) {
    memcpy(d, s, strlen(s) + 1);
}

char *strsep(char **stringPointer, const char *delimiter) {
    char *str;
    if((str = *stringPointer) == NULL) return NULL;

    for(char *token = str;;) {
        char c = *str++;
        const char *spanP = delimiter;
        char sc;
        do {
            if((sc = *spanP++) == c) {
                if(c == 0) str = NULL;
                else str[-1] = 0;
                *stringPointer = str;
                return token;
            }
        } while(sc != 0);
    }
}

char *strstr(const char *in, const char *str) {
    char c = *str++;
    if(!c) return (char *)in;

    size_t length = strlen((char *)str);
    do {
        char sc;
        do {
            sc = *in++;
            if(!sc) return 0;
        } while(sc != c);
    } while(strncmp(in, str, length));

    return (char *)(in - 1);
}