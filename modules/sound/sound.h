#pragma once

#include <modules/modules.h>
#include <util/util.h>
#include <cpu/pci.h>

#define INTERNALBUFFER_SIZE 0xFFFF

typedef struct {
    uint8_t *internalBuffer;
    uint64_t readPos;
    uint64_t writePos;
    uint8_t alignSize;
} dspNode_t;

typedef struct {
    size_t id;

    char name[64];
    void *device;
} sound_device_t;

void register_sound_device(sound_device_t *device);
