#include "common.h"
#include "ctype.h"
#include <stdarg.h>

#include "spinlock.h"

typedef struct printf_function {
    void (*write)(struct printf_function *, char);
    void *data;
    size_t currentPos;
    size_t size;
} printf_function_t;

#define PRINT_FLAG_LEFT         0x1
#define PRINT_FLAG_PLUS         0x2
#define PRINT_FLAG_SPACE        0x4
#define PRINT_FLAG_HASH         0x8
#define PRINT_FLAG_ZEROPAD      0x10
#define PRINT_FLAG_PRECISION   0x20
#define PRINT_FLAG_CHAR         0x40
#define PRINT_FLAG_SHORT        0x80
#define PRINT_FLAG_LONG         0x100
#define PRINT_FLAG_LONG_LONG    0x200
#define PRINT_FLAG_UPPERCASE    0x400

#define PRINTF_NTOA_BUFFER_SIZE 32U

static void _out_rev(printf_function_t *func, const char* buf, size_t len, uint32_t width, uint32_t flags) {
    size_t idx = 0;

    if(!(flags & PRINT_FLAG_LEFT) && !(flags & PRINT_FLAG_ZEROPAD)) {
        for (size_t i = len; i < width; i++) {
            func->write(func, ' ');
            idx++;
        }
    }

    while (len) {
        func->write(func, buf[--len]);
        idx++;
    }

    if (flags & PRINT_FLAG_LEFT) {
        while (idx < width) {
            func->write(func, ' ');
            idx++;
        }
    }
}

static void _ntoa_format(printf_function_t *func, char *buf, size_t len, bool negative, uint32_t base, uint32_t prec, uint32_t width, uint32_t flags) {
    // pad leading zeros
    if(!(flags & PRINT_FLAG_LEFT)) {
        if(width && (flags & PRINT_FLAG_ZEROPAD) && (negative || (flags & (PRINT_FLAG_PLUS | PRINT_FLAG_SPACE)))) {
            width--;
        }
        while((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
        while((flags & PRINT_FLAG_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
    }

    // handle hash
    if(flags & PRINT_FLAG_HASH) {
        if(!(flags & PRINT_FLAG_PRECISION) && len && ((len == prec) || (len == width))) {
            len--;
            if (len && (base == 16U)) {
                len--;
            }
        }
        if((base == 16U) && !(flags & PRINT_FLAG_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'x';
        }
        else if((base == 16U) && (flags & PRINT_FLAG_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'X';
        }
        else if((base == 2U) && (len < PRINTF_NTOA_BUFFER_SIZE)) {
            buf[len++] = 'b';
        }
        if(len < PRINTF_NTOA_BUFFER_SIZE) {
            buf[len++] = '0';
        }
    }

    if(len < PRINTF_NTOA_BUFFER_SIZE) {
        if(negative) {
            buf[len++] = '-';
        }
        else if(flags & PRINT_FLAG_PLUS) {
            buf[len++] = '+';  // ignore the space if the '+' exists
        }
        else if(flags & PRINT_FLAG_SPACE) {
            buf[len++] = ' ';
        }
    }

    _out_rev(func, buf, len, width, flags);
}

static void _ntoa_long(printf_function_t *func, unsigned long value, bool negative, uint32_t base, uint32_t prec, uint32_t width, uint32_t flags) {
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0;

    if(!value) flags &= ~PRINT_FLAG_HASH;

    if(!(flags & PRINT_FLAG_PRECISION) || value) {
        do {
            char digit = (char)(value % base);
            buf[len++] = (digit < 10) ? ('0' + digit) : ((PRINT_FLAG_UPPERCASE ? 'A' : 'a') + digit - 10);
            value /= base;
        } while(value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    _ntoa_format(func, buf, len, negative, base, prec, width, flags);
}

static void _ntoa_long_long(printf_function_t *func, unsigned long long value, bool negative, uint32_t base, uint32_t prec, uint32_t width, uint32_t flags) {
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0;

    if(!value) flags &= ~PRINT_FLAG_HASH;

    if(!(flags & PRINT_FLAG_PRECISION) || value) {
        do {
            char digit = (char)(value % base);
            buf[len++] = (digit < 10) ? ('0' + digit) : ((PRINT_FLAG_UPPERCASE ? 'A' : 'a') + digit - 10);
            value /= base;
        } while(value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    _ntoa_format(func, buf, len, negative, base, prec, width, flags);
}

int _vsnprintf_internal(printf_function_t *func, const char *format, va_list args) {
    char *fmt = (char *)format;
    while(*fmt) {
        if(*fmt == '%') {
            fmt++;

            uint16_t flags = 0;

            bool running = true;
            while(running) {
                switch(*fmt) {
                    case '-': flags |= PRINT_FLAG_LEFT;     break;
                    case '+': flags |= PRINT_FLAG_PLUS;     break;
                    case ' ': flags |= PRINT_FLAG_SPACE;    break;
                    case '#': flags |= PRINT_FLAG_HASH;     break;
                    case '0': flags |= PRINT_FLAG_ZEROPAD;  break;
                    default: running = false; break;
                }
                if(running) fmt++;
            }

            uint32_t width = 0;
            if(isdigit(*fmt))
                while(isdigit(*fmt)) {
                    width *= 10;
                    width += ((*fmt++) - '0');
                }
            else if(*fmt == '*') {
                int w = va_arg(args, int);
                if(w < 0) {
                    flags |= PRINT_FLAG_LEFT;
                    width = (uint32_t)-w;
                }
                else width = (uint32_t)w;

                fmt++;
            }

            uint32_t precission = 0;
            if(*fmt == '.') {
                fmt++;
                flags |= PRINT_FLAG_PRECISION;

                if(isdigit(*fmt))
                    while(isdigit(*fmt)) {
                        precission *= 10;
                        precission += ((*fmt++) - '0');
                    }
                else if(*fmt == '*') {
                    int p = va_arg(args, int);
                    precission = p > 0 ? (uint32_t)p : 0;

                    fmt++;
                }
            }

            switch(*fmt++) {
                case 'l':
                    {
                        flags |= PRINT_FLAG_LONG;
                        if(*fmt == 'l') {
                            flags |= PRINT_FLAG_LONG_LONG;
                            fmt++;
                        }
                    }
                    break;
                case 'h':
                    {
                        flags |= PRINT_FLAG_SHORT;
                        if(*fmt == 'h') {
                            flags |= PRINT_FLAG_CHAR;
                            fmt++;
                        }
                    }
                    break;
                case 'j':
                    flags |= (sizeof(intmax_t) == sizeof(long) ? PRINT_FLAG_LONG : PRINT_FLAG_LONG_LONG);
                    break;
                case 'z':
                    flags |= (sizeof(size_t) == sizeof(long) ? PRINT_FLAG_LONG : PRINT_FLAG_LONG_LONG);
                    break;
                default:
                    fmt--;
                    break;
            }

            switch(*fmt) {
                case 'd':
                case 'i':
                case 'u':
                case 'x':
                case 'X':
                case 'o':
                case 'b':
                    {
                        unsigned int base;
                        if(*fmt == 'x' || *fmt == 'X') {
                            base = 16;
                        }
                        else if(*fmt == 'o') {
                            base =  8;
                        }
                        else if(*fmt == 'b') {
                            base =  2;
                        }
                        else {
                            base = 10U;
                            flags &= ~PRINT_FLAG_HASH;
                        }

                        if(*fmt == 'X') {
                            flags |= PRINT_FLAG_UPPERCASE;
                        }

                        if((*fmt != 'i') && (*fmt != 'd')) {
                            flags &= ~(PRINT_FLAG_PLUS | PRINT_FLAG_SPACE);
                        }

                        if(flags & PRINT_FLAG_PRECISION) {
                            flags &= ~PRINT_FLAG_ZEROPAD;
                        }

                        if((*fmt == 'i') || (*fmt == 'd')) {
                            if(flags & PRINT_FLAG_LONG_LONG) {
                                long long value = va_arg(args, long long);
                                _ntoa_long_long(func, (unsigned long long)((value > 0) ? value : (0 - value)), value < 0, base, precission, width, flags);
                            }
                            else if(flags & PRINT_FLAG_LONG) {
                                long value = va_arg(args, long);
                                _ntoa_long(func, (unsigned long)((value > 0) ? value : (0 - value)), value < 0, base, precission, width, flags);
                            }
                            else {
                                int value = (flags & PRINT_FLAG_CHAR) ? (char)va_arg(args, int) : (flags & PRINT_FLAG_SHORT) ? (short int)va_arg(args, int) : va_arg(args, int);
                                _ntoa_long(func, (unsigned int)((value > 0) ? value : (0 - value)), value < 0, base, precission, width, flags);
                            }
                        } else {
                            if(flags & PRINT_FLAG_LONG_LONG) {
                                _ntoa_long_long(func, va_arg(args, unsigned long long), false, base, precission, width, flags);
                            }
                            else if(flags & PRINT_FLAG_LONG) {
                                _ntoa_long_long(func, va_arg(args, unsigned long), false, base, precission, width, flags);
                            }
                            else {
                                uint32_t value = (flags & PRINT_FLAG_CHAR) ? (uint8_t)va_arg(args, uint32_t) : (flags & PRINT_FLAG_SHORT) ? (uint16_t)va_arg(args, uint32_t) : va_arg(args, uint32_t);
                                _ntoa_long_long(func, value, false, base, precission, width, flags);
                            }
                        }
                    }
                    fmt++;
                    break;
                case 'c':
                    {
                        uint32_t l = 1;
                        if(!(flags & PRINT_FLAG_LEFT)) {
                            while(l++ < width) {
                                func->write(func, ' ');
                            }
                        }
                        func->write(func, (char)va_arg(args, int));
                        if(flags & PRINT_FLAG_LEFT) {
                            while(l++ < width) {
                                func->write(func, ' ');
                            }
                        }
                    }
                    fmt++;
                    break;
                case 's':
                    {
                        const char* p = va_arg(args, char*);

                        size_t maxsize = precission ? precission : (size_t)-1;
                        char* s;
                        for (s = (char *)p; *s && maxsize--; ++s);
                        uint32_t l = (s - p);

                        if(flags & PRINT_FLAG_PRECISION) l = ((l < precission) ? l : precission);
                        if(!(flags & PRINT_FLAG_LEFT))
                            while(l++ < width)
                                func->write(func, ' ');

                        while(*p && (!(flags & PRINT_FLAG_PRECISION) || precission--))
                            func->write(func, *(p++));

                        if(flags & PRINT_FLAG_LEFT)
                            while(l++ < width)
                                func->write(func, ' ');
                    }
                    fmt++;
                    break;

                case 'p':
                    {
                        width = sizeof(void*) * 2;
                        flags |= PRINT_FLAG_ZEROPAD | PRINT_FLAG_UPPERCASE;
                        const bool isLl = sizeof(uintptr_t) == sizeof(long long);
                        if(isLl) _ntoa_long_long(func, (uintptr_t)va_arg(args, void*), false, 16, precission, width, flags);
                        else _ntoa_long(func, (unsigned long)(uintptr_t)va_arg(args, void*), false, 16, precission, width, flags);
                    }
                    fmt++;
                    break;

                default:
                    func->write(func, *fmt++);
                    break;
            }
        } else {
            func->write(func, *fmt++);
        }
    }

    func->write(func, '\0');

    return func->currentPos;
}

//int snprintf(const char *s, size_t n, const char *format, ...) {
//
//}

void printf_write_char_e8(printf_function_t *func, char c) {
    if(!func->size || func->size < func->currentPos) {
        outb(0xE9, c);
        func->currentPos++;
    }
}

void printf_write_char_str(printf_function_t *func, char c) {
    if(func->data && (!func->size || func->size < func->currentPos)) {
        ((char *)func->data)[func->currentPos] = c;
        func->currentPos++;
    }
}

static volatile spinlock_t PrintLock = INIT_SPINLOCK();
static volatile spinlock_t PrintLock2 = INIT_SPINLOCK();

int sprintf(const char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf_function_t func = (printf_function_t){0};
    func.write = printf_write_char_str;
    func.data = (void *)str;

    spinlock_lock(&PrintLock);
    int ret = _vsnprintf_internal(&func, format, args);
    spinlock_unlock(&PrintLock);

    va_end(args);

    return ret;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf_function_t func = (printf_function_t){0};
    func.write = printf_write_char_e8;

    spinlock_lock(&PrintLock);
    int ret = _vsnprintf_internal(&func, format, args);
    spinlock_unlock(&PrintLock);

    va_end(args);

    return ret;
}

int printf_scheduler(const char *format, ...) {
    va_list args;
    va_start(args, format);

    printf_function_t func = (printf_function_t){0};
    func.write = printf_write_char_e8;

    spinlock_lock(&PrintLock2);
    int ret = _vsnprintf_internal(&func, format, args);
    spinlock_unlock(&PrintLock2);

    va_end(args);

    return ret;
}

static uint32_t block = 0;

void qemu_debug_puts_urgent(const char *str) {
    locked_inc(&block);
    for(size_t i = 0; str[i]; i++)
        outb(0xE9, str[i]);
    locked_dec(&block);
}