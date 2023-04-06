#include "string.h"

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memcpy64(void *dest, const void *src, size_t n) {
    uint64_t *pdest = dest;
    const uint64_t *psrc = src;

    for (size_t i = 0; i < (n / sizeof(uint64_t)); i++) {
        pdest[i] = psrc[i];
    }

    return dest;
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
        for (size_t i = 0; i < n; i++) {
            pdest[n - i - 1] = psrc[n - i - 1];
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

size_t strlen(char *str) {
    size_t len = 0;
    for(;*str++;len++);
    return len;
}

char *strdup(char *str) {
    size_t length = strlen(str) + 1;
    char *strNew = kmalloc(length);
    memcpy(strNew, str, length);

    return strNew;
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

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

#define ALIGN (sizeof(size_t))
#define ONES ((size_t)-1/255)
#define HIGHS (ONES * (255/2+1))
#define HASZERO(X) (((X)-ONES) & ~(X) & HIGHS)

#define BITOP(A, B, OP) \
 ((A)[(size_t)(B)/(8*sizeof *(A))] OP (size_t)1<<((size_t)(B)%(8*sizeof *(A))))

size_t strspn(const char * s, const char * c) {
	const char * a = s;
	size_t byteset[32/sizeof(size_t)] = { 0 };

	if (!c[0]) {
		return 0;
	}
	if (!c[1]) {
		for (; *s == *c; s++);
		return s-a;
	}

	for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
	for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);

	return s-a;
}

char * strchrnul(const char * s, int c) {
	size_t * w;
	size_t k;

	c = (unsigned char)c;
	if (!c) {
		return (char *)s + strlen((char *)s);
	}

	for (; (uintptr_t)s % ALIGN; s++) {
		if (!*s || *(unsigned char *)s == c) {
			return (char *)s;
		}
	}

	k = ONES * c;
	for (w = (void *)s; !HASZERO(*w) && !HASZERO(*w^k); w++);
	for (s = (void *)w; *s && *(unsigned char *)s != c; s++);
	return (char *)s;
}

size_t strcspn(const char * s, const char * c) {
	const char *a = s;
	if (c[0] && c[1]) {
		size_t byteset[32/sizeof(size_t)] = { 0 };
		for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
		for (; *s && !BITOP(byteset, *(unsigned char *)s, &); s++);
		return s-a;
	}
	return strchrnul(s, *c)-a;
}

char * strpbrk(const char * s, const char * b) {
	s += strcspn(s, b);
	return *s ? (char *)s : 0;
}

char * strchr(const char * s, int c) {
	char *r = strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

size_t lfind(const char * str, const char accept) {
	return (size_t)strchr(str, accept);
}

char * strtok_r(char * str, const char * delim, char ** saveptr) {
	char * token;
	if (str == NULL) {
		str = *saveptr;
	}
	str += strspn(str, delim);
	if (*str == '\0') {
		*saveptr = str;
		return NULL;
	}
	token = str;
	str = strpbrk(token, delim);
	if (str == NULL) {
		*saveptr = (char *)lfind(token, '\0');
	} else {
		*str = '\0';
		*saveptr = str + 1;
	}
	return token;
}