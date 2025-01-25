#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <utils/common.h>
#include <fs/vfs.h>

#define EXT2_SIGNATURE 0xEF53

#define EXT2_INODE_ROOT 2
#define EXT2_INODE_DIRECTORY 0x4000

void ext2_init();