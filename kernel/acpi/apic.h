#pragma once

#include <stdint.h>

void lapic_write(uint32_t reg, uint32_t value);
uint32_t lapic_read(uint32_t reg);

void lapic_init();