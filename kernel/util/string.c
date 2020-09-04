#include <util/string.h>

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

char *strcpy(char *dest, const char *src) {
    size_t i;

    for (i = 0; src[i]; i++)
        dest[i] = src[i];

    dest[i] = 0;

    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *rdest = dest;

    while (*dest) dest++;
    while (*dest++ = *src++);
    return rdest;
}

size_t strlen(const char *str) {
    size_t len;

    for (len = 0; str[len]; len++);

    return len;
}