#include "packetfs.h"

#define SYSCALL_ID(module, id) ((((size_t)(uint8_t)module & 0xFF) << 8) | ((size_t)id & 0xFF))

typedef uint64_t sc_qword_t;
static sc_qword_t syscall2(int sc, sc_qword_t arg1, sc_qword_t arg2) {
    sc_qword_t ret;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2)
                 : "rcx", "r11", "memory");
    return ret;
}

void *shm_open(char *path, size_t *size) {
    return (void *)syscall2(SYSCALL_ID('S', 0x00), (sc_qword_t)path, (sc_qword_t)size);
}

size_t server_client_read(FILE *socket, uint8_t *buffer, size_t size) {
    ssize_t retSize = -1;
    while(retSize == -1) { //Just to be sure, but not needed as this is blocking read
        retSize = read(fileno(socket), (void *)buffer, size);
    }

    return retSize;
}

size_t server_client_write(FILE *socket, void *target, uint8_t *buffer, size_t size) {
	pmgr_header_t *header = malloc(size + sizeof(pmgr_header_t));
	header->target = target;
	memcpy((void *)((size_t)header + sizeof(pmgr_header_t)), buffer, size);

    ssize_t retSize = -1;
    while(retSize == -1) { //Just to be sure, but not needed as this is blocking read
        retSize = write(fileno(socket), (void *)header, size + sizeof(pmgr_header_t));
    }

    return retSize;
}