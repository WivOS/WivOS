#pragma once

#include "common.h"

inline int isdigit(int c) {
    return (uint8_t)(c - '0') <= 9;
}