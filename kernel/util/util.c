#include "util.h"
#include <stdbool.h>
#include <util/string.h>

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

char* __int_str(uint64_t i, char b[], int base, bool plusSignIfNeeded, bool spaceSignIfNeeded,
                int paddingNo, bool justify, bool zeroPad) {
 
    char digit[32] = {0};
    memset(digit, 0, 32);
    strcpy(digit, "0123456789");
 
    if (base == 16) {
        strcat(digit, "ABCDEF");
    } else if (base == 17) {
        strcat(digit, "abcdef");
        base = 16;
    }
 
    char* p = b;
    if (i < 0) {
        *p++ = '-';
        i *= -1;
    } else if (plusSignIfNeeded) {
        *p++ = '+';
    } else if (!plusSignIfNeeded && spaceSignIfNeeded) {
        *p++ = ' ';
    }
 
    intmax_t shifter = i;
    do {
        ++p;
        shifter = shifter / base;
    } while (shifter);
 
    *p = '\0';
    do {
        *--p = digit[i % base];
        i = i / base;
    } while (i);
 
    int padding = paddingNo - (int)strlen(b);
    if (padding < 0) padding = 0;
 
    if (justify) {
        while (padding--) {
            if (zeroPad) {
                b[strlen(b)] = '0';
            } else {
                b[strlen(b)] = ' ';
            }
        }
 
    } else {
        char a[256] = {0};
        while (padding--) {
            if (zeroPad) {
                a[strlen(a)] = '0';
            } else {
                a[strlen(a)] = ' ';
            }
        }
        strcat(a, b);
        strcpy(b, a);
    }
 
    return b;
}

static inline bool _is_digit(char ch)
{
    return (ch >= '0') && (ch <= '9');
}

typedef void (*out_fct_type)(char character, void* buffer, size_t idx, size_t maxlen);

static inline void _out_char(char character, void* buffer, size_t idx, size_t maxlen)
{
    (void)buffer; (void)idx; (void)maxlen;
    if (character) {
        debug_out_char(character);
    }
}

void displayCharacter(char c, int* a, out_fct_type out, char* buffer, size_t maxlen) {
    out(c, buffer, *a++, maxlen);
}
 
void displayString(char* c, int* a, out_fct_type out, char* buffer, size_t maxlen) {
    for (int i = 0; c[i]; ++i) {
        displayCharacter(c[i], a, out, buffer, maxlen);
    }
}
 
int _vsnprintf(out_fct_type out, char* buffer, const size_t maxlen, const char* format, va_list list)
{
    int chars        = 0;
    char intStrBuffer[256] = {0};
 
    for (int i = 0; format[i]; ++i) {
 
        char specifier   = '\0';
        char length      = '\0';
 
        int  lengthSpec  = 0; 
        int  precSpec    = 0;
        bool leftJustify = false;
        bool zeroPad     = false;
        bool spaceNoSign = false;
        bool altForm     = false;
        bool plusSign    = false;
        int  expo        = 0;
 
        if (format[i] == '%') {
            ++i;
 
            bool extBreak = false;
            while (1) {
 
                switch (format[i]) {
                    case '-':
                        leftJustify = true;
                        ++i;
                        break;
 
                    case '+':
                        plusSign = true;
                        ++i;
                        break;
 
                    case '#':
                        altForm = true;
                        ++i;
                        break;
 
                    case ' ':
                        spaceNoSign = true;
                        ++i;
                        break;
 
                    case '0':
                        zeroPad = true;
                        ++i;
                        break;
 
                    default:
                        extBreak = true;
                        break;
                }
 
                if (extBreak) break;
            }
 
            while (_is_digit(format[i])) {
                lengthSpec *= 10;
                lengthSpec += format[i] - 48;
                ++i;
            }
 
            if (format[i] == '*') {
                lengthSpec = va_arg(list, int);
                ++i;
            }
 
            if (format[i] == '.') {
                ++i;
                while (_is_digit(format[i])) {
                    precSpec *= 10;
                    precSpec += format[i] - 48;
                    ++i;
                }
 
                if (format[i] == '*') {
                    precSpec = va_arg(list, int);
                    ++i;
                }
            } else {
                precSpec = 6;
            }
 
            if (format[i] == 'h' || format[i] == 'l' || format[i] == 'j' ||
                   format[i] == 'z' || format[i] == 't' || format[i] == 'L') {
                length = format[i];
                ++i;
                if (format[i] == 'h') {
                    length = 'H';
                } else if (format[i] == 'l') {
                    length = 'q';
                }
            }
            specifier = format[i];
 
            memset(intStrBuffer, 0, 256);
 
            int base = 10;
            if (specifier == 'o') {
                base = 8;
                specifier = 'u';
                if (altForm) {
                    displayString("0", &chars, out, buffer, maxlen);
                }
            }
            if (specifier == 'p') {
                base = 16;
                length = 'z';
                specifier = 'u';
            }
            switch (specifier) {
                case 'X':
                    base = 16;
                case 'x':
                    base = base == 10 ? 17 : base;
                    if (altForm) {
                        displayString("0x", &chars, out, buffer, maxlen);
                    }
 
                case 'u':
                {
                    switch (length) {
                        case 0:
                        {
                            unsigned int integer = va_arg(list, unsigned int);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'H':
                        {
                            unsigned char integer = (unsigned char) va_arg(list, unsigned int);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'h':
                        {
                            unsigned short int integer = va_arg(list, unsigned int);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'l':
                        {
                            unsigned long integer = va_arg(list, unsigned long);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'q':
                        {
                            unsigned long long integer = va_arg(list, unsigned long long);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'j':
                        {
                            uintmax_t integer = va_arg(list, uintmax_t);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 'z':
                        {
                            size_t integer = va_arg(list, size_t);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        case 't':
                        {
                            ptrdiff_t integer = va_arg(list, ptrdiff_t);
                            __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                            displayString(intStrBuffer, &chars, out, buffer, maxlen);
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                }
 
                case 'd':
                case 'i':
                {
                    switch (length) {
                    case 0:
                    {
                        int integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'H':
                    {
                        signed char integer = (signed char) va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'h':
                    {
                        short int integer = va_arg(list, int);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'l':
                    {
                        long integer = va_arg(list, long);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'q':
                    {
                        long long integer = va_arg(list, long long);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'j':
                    {
                        intmax_t integer = va_arg(list, intmax_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 'z':
                    {
                        size_t integer = va_arg(list, size_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    case 't':
                    {
                        ptrdiff_t integer = va_arg(list, ptrdiff_t);
                        __int_str(integer, intStrBuffer, base, plusSign, spaceNoSign, lengthSpec, leftJustify, zeroPad);
                        displayString(intStrBuffer, &chars, out, buffer, maxlen);
                        break;
                    }
                    default:
                        break;
                    }
                    break;
                }
 
                case 'c':
                {
                    displayCharacter(va_arg(list, int), &chars, out, buffer, maxlen);
 
                    break;
                }
 
                case 's':
                {
                    displayString(va_arg(list, char*), &chars, out, buffer, maxlen);
                    break;
                }
 
                case 'n':
                {
                    switch (length) {
                        case 'H':
                            *(va_arg(list, signed char*)) = chars;
                            break;
                        case 'h':
                            *(va_arg(list, short int*)) = chars;
                            break;
 
                        case 0: {
                            int* a = va_arg(list, int*);
                            *a = chars;
                            break;
                        }
 
                        case 'l':
                            *(va_arg(list, long*)) = chars;
                            break;
                        case 'q':
                            *(va_arg(list, long long*)) = chars;
                            break;
                        case 'j':
                            *(va_arg(list, intmax_t*)) = chars;
                            break;
                        case 'z':
                            *(va_arg(list, size_t*)) = chars;
                            break;
                        case 't':
                            *(va_arg(list, ptrdiff_t*)) = chars;
                            break;
                        default:
                            break;
                    }
                    break;
                }
 
                case 'a':
                case 'A':
                    //ACK! Hexadecimal floating points...
                    break;
 
                default:
                    break;
            }
 
            if (specifier == 'e') {
                displayString("e+", &chars, out, buffer, maxlen);
            } else if (specifier == 'E') {
                displayString("E+", &chars, out, buffer, maxlen);
            }
 
            if (specifier == 'e' || specifier == 'E') {
                __int_str(expo, intStrBuffer, 10, false, false, 2, false, true);
                displayString(intStrBuffer, &chars, out, buffer, maxlen);
            }
 
        } else {
            displayCharacter(format[i], &chars, out, buffer, maxlen);
        }
    }
 
    return chars;
}

int printf(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    char buffer[1];
    const int ret = _vsnprintf(_out_char, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}