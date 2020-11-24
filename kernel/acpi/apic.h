#pragma once

#include <stdint.h>

void lapic_write(uint32_t reg, uint32_t value);
uint32_t lapic_read(uint32_t reg);
void lapic_connect_gsi_to_vec(int cpu, uint8_t vec, uint32_t gsi, uint16_t flags, int status);
void lapic_legacy_irq(int cpu, uint8_t irq, int status);

void lapic_enable();

void lapic_init();