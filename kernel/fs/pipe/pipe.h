#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <utils/common.h>
#include <fs/vfs.h>

bool pipe_create(vfs_node_t **pipeNodes);
void pipe_set_read_at_first_data(vfs_node_t *pipe, bool set);