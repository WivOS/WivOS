#include <util/string.h>

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memcpy(void *d, void *s, size_t n) {
    uint8_t *p = d;
    uint8_t *p2 = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = p2[i];
    }

    return d;
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

static int is_delim(char c, char *delim) {
    while(*delim != '\0') {
        if(c == *delim)
            return 1;
        delim++;
    }
    return 0;
}

char *strtok(char *s, char *delim) {
    static char *p;
    if(!s) { s = p; return NULL; }

    while(1) {
        if(is_delim(*s, delim)) {
            s++;
            continue;
        }
        if(*s == '\0') return NULL;
        break;
    }

    char *ret = s;
    while(1) {
        if(*s == '\0') {
            p = s;
            return ret;
        }
        if(is_delim(*s, delim)) {
            *s = '\0';
            p = s + 1;
            return ret;
        }
        s++;
    }
}

char *strdup(const char *src) {
    int len = strlen(src) + 1;
    char * dst = (char *)kmalloc(len);
    memcpy((void *)dst, (void *)src, len);
    return dst;
}

int strcmp(const char *dst, char *src) {
    int i = 0;

    while ((dst[i] == src[i])) {
        if (src[i++] == 0)
            return 0;
    }

    return 1;
}

char *strsep(char **stringp, const char *delim) {
    char *s;
    const char *spanp;
    int c, sc;
    char *tok;
    if ((s = *stringp) == NULL)
        return (NULL);
    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
}

list_t *strtoklist(char *str, char *delim, size_t *nTokens) {
    list_t *retList = list_create();
    char *s = strdup(str);
    char *token, *rest = s;
    while((token = strsep(&rest, delim)) != NULL) {
        if(!strcmp(token, ".")) continue;
        if(!strcmp(token, "..")) {
            if(list_size(retList) > 0) list_remove_front(retList);
            continue;
        }
        list_insert_front(retList, strdup(token));
        if(nTokens) (*nTokens)++;
    }
    kfree(s);
    return retList;
}

char *list2str(list_t *list, const char* delim) {
    char *ret = (char *)kmalloc(256);
    memset((void *)ret, 0, 256);
    int length = 0, retLength = 256;
    while(list_size(list) > 0) {
        char *temp = list_pop(list)->val;
        int lengthTemp = strlen(temp);
        if(length + lengthTemp + 2 > retLength) {
            retLength = retLength * 2;
            ret = (char *)krealloc((void *)ret, retLength);
            length = length + lengthTemp + 1;
        }
        strcat(ret, temp);
        strcat(ret, delim);
    }
    return ret;
}