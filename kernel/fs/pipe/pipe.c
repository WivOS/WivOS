#include "pipe.h"
#include <utils/spinlock.h>
#include <tasking/event.h>

#define PIPE_BUFFER_STEP 32768

typedef struct {
    void *buffer;
    size_t size;
    event_t event;
    size_t reference_count;
    uint32_t *poll_status;
    spinlock_t lock;

    size_t flags;
} pipe_data_t;

size_t pipe_read(struct vfs_node *node, char *buffer, size_t size) {
    pipe_data_t *data = node->data;
    if(data == NULL) return -1;

    spinlock_lock(&data->lock);

    while(size > data->size) {
        if(data->flags & O_NONBLOCK || data->reference_count == 1) {
            size = data->size;
            break;
        } else {
            spinlock_unlock(&data->lock);
            if(!event_await(&data->event)) {
                //TODO: Errno
                printf("TODO: Errno\n");
                return -1;
            }
            spinlock_lock(&data->lock);
        }
    }

    event_reset(&data->event); //Clear the prior data writes events

    size_t pipeSizeSteps = ROUND_UP(data->size, PIPE_BUFFER_STEP);
    size_t newPipeSize = data->size - size;
    size_t newPipeSizeSteps = ROUND_UP(newPipeSize, PIPE_BUFFER_STEP);

    memcpy(buffer, data->buffer, size);
    memmove(data->buffer, data->buffer + size, data->size - size);

    if(newPipeSizeSteps < pipeSizeSteps)
        data->buffer = krealloc(data->buffer, newPipeSizeSteps);

    data->size = newPipeSize;

    if(!newPipeSize)
        locked_write(uint32_t, data->poll_status, 0);

    spinlock_unlock(&data->lock);
    return size;
}

size_t pipe_write(struct vfs_node *node, char *buffer, size_t size) {
    pipe_data_t *data = node->data;
    if(data == NULL) return -1;

    spinlock_lock(&data->lock);

    size_t pipeSizeSteps = ROUND_UP(data->size, PIPE_BUFFER_STEP);
    size_t newPipeSize = data->size + size;
    size_t newPipeSizeSteps = ROUND_UP(newPipeSize, PIPE_BUFFER_STEP);

    if(newPipeSizeSteps > pipeSizeSteps)
        data->buffer = krealloc(data->buffer, newPipeSizeSteps);

    memcpy(data->buffer + data->size, buffer, size);

    data->size = newPipeSize;

    locked_write(uint32_t, data->poll_status, POLLIN);
    event_notify(&data->event);

    spinlock_unlock(&data->lock);
    return size;
}

size_t pipe_close(struct vfs_node *node) {
    pipe_data_t *data = node->data;
    if(data == NULL) return -1;

    spinlock_lock(&data->lock);

    data->reference_count--;
    if(data->reference_count) {
        event_notify(&data->event);
        spinlock_unlock(&data->lock);
        return 0;
    }
    if(data->size)
        kfree(data->buffer);

    kfree(data);

    spinlock_unlock(&data->lock);
    return 0;
}

bool pipe_create(vfs_node_t **pipeNodes) {
    pipe_data_t *newPipe = (pipe_data_t *)kmalloc(sizeof(pipe_data_t));
    newPipe->reference_count = 2;
    newPipe->lock = INIT_SPINLOCK();
    newPipe->size = 0;
    event_reset(&newPipe->event);

    vfs_node_t *readNode = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    vfs_node_t *writeNode = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    newPipe->poll_status = &readNode->status;

    vfs_functions_t functions = {0};
    functions.read = pipe_read;
    functions.write = pipe_write;
    functions.close = pipe_close;

    readNode->functions = functions;
    readNode->data = newPipe;
    writeNode->functions = functions;
    writeNode->data = newPipe;

    locked_write(uint32_t, &writeNode->status, POLLOUT);

    pipeNodes[0] = readNode;
    pipeNodes[1] = writeNode;

    return true;
}