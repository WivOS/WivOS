#include "util.h"

void debug_out_char(char o) {
    outb(0xE9, o);
}