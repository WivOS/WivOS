#pragma once

#include <util/util.h>

typedef struct tar_header {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
} tar_header_t;

size_t parseTarInitrd(void *initrd);