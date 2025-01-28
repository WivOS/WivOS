#include <stdint.h>
#include <stddef.h>

#include "string.h"

#define SYS_DEBUG   0x0
#define SYS_READ    0x1
#define SYS_WRITE   0x2
#define SYS_OPEN    0x3
#define SYS_CLOSE   0x4
#define SYS_ALLOC   0x5
#define SYS_SEEK    0x6
#define SYS_TCB_SET 0x7

typedef uint64_t sc_qword_t;

static sc_qword_t syscall0(int sc) {
    sc_qword_t ret;
    asm volatile("syscall" : "=a"(ret) : "a"(sc) : "rcx", "r11", "memory");
    return ret;
}

static sc_qword_t syscall1(int sc, uint64_t arg1) {
    sc_qword_t ret;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1)
                 : "rcx", "r11", "memory");
    return ret;
}

static sc_qword_t syscall2(int sc, sc_qword_t arg1, sc_qword_t arg2) {
    sc_qword_t ret;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2)
                 : "rcx", "r11", "memory");
    return ret;
}

static sc_qword_t syscall3(int sc, sc_qword_t arg1, sc_qword_t arg2,
                          sc_qword_t arg3) {
    sc_qword_t ret;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3)
                 : "rcx", "r11", "memory");
    return ret;
}

static sc_qword_t syscall4(int sc, sc_qword_t arg1, sc_qword_t arg2,
                          sc_qword_t arg3, sc_qword_t arg4) {
    sc_qword_t ret;

    register sc_qword_t arg4_reg asm("r10") = arg4;

    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4_reg)
                 : "rcx", "r11", "memory");
    return ret;
}

static sc_qword_t syscall5(int sc, sc_qword_t arg1, sc_qword_t arg2,
                          sc_qword_t arg3, sc_qword_t arg4, sc_qword_t arg5) {
    sc_qword_t ret;

    register sc_qword_t arg4_reg asm("r10") = arg4;
    register sc_qword_t arg5_reg asm("r8") = arg5;

    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4_reg),
                   "r"(arg5_reg)
                 : "rcx", "r11", "memory");
    return ret;
}

static sc_qword_t syscall6(int sc, sc_qword_t arg1, sc_qword_t arg2,
                          sc_qword_t arg3, sc_qword_t arg4, sc_qword_t arg5,
                          sc_qword_t arg6) {
    sc_qword_t ret;

    register sc_qword_t arg4_reg asm("r10") = arg4;
    register sc_qword_t arg5_reg asm("r8") = arg5;
    register sc_qword_t arg6_reg asm("r9") = arg6;

    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2), "d"(arg3), "r"(arg4_reg),
                   "r"(arg5_reg), "r"(arg6_reg)
                 : "rcx", "r11", "memory");
    return ret;
}

void debug_print(char *str) {
    syscall1(SYS_DEBUG, (uint64_t)str);
}

size_t open(char *path, uint32_t mode) {
    return syscall2(SYS_OPEN, (sc_qword_t)path, (sc_qword_t)mode);
}

size_t write(size_t fd, char *buffer, size_t length) {
    return syscall3(SYS_WRITE, (sc_qword_t)fd, (sc_qword_t)buffer, (sc_qword_t)length);
}

size_t read(size_t fd, char *buffer, size_t length) {
    return syscall3(SYS_READ, (sc_qword_t)fd, (sc_qword_t)buffer, (sc_qword_t)length);
}


//Program itself

typedef struct {
    void *source;
    size_t size;
    uint8_t data[];
} pmgr_packet_t;

typedef struct {
    void *target;
    uint8_t data[];
} pmgr_header_t;

void main() {
    debug_print("Hola");
    while(1);

    //size_t server_fd = open("/dev/pmgr/testServer", 0x10);
//
    //size_t client_fd = open("/dev/pmgr/testServer", 0);
//
    //uint8_t testBuffer[1024];
    //pmgr_header_t *testHeader = (pmgr_header_t *)testBuffer;
//
    //testHeader->target = NULL;
    //strcpy((char *)testHeader->data, "Hola\0");
    //write(server_fd, (char *)testBuffer, sizeof(pmgr_header_t) + 5);
//
    //size_t size = read(client_fd, (void *)testBuffer, 1024);
    //debug_print((char *)testBuffer);
    ////printf("[PaketFS] Test Received: %.*s\n", size, (char *)testBuffer);
//
    //while(1);
}