#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

typedef uint64_t sc_qword_t;

static sc_qword_t syscall2(int sc, sc_qword_t arg1, sc_qword_t arg2) {
    sc_qword_t ret;
    asm volatile("syscall"
                 : "=a"(ret)
                 : "a"(sc), "D"(arg1), "S"(arg2)
                 : "rcx", "r11", "memory");
    return ret;
}

int syscall_load_module(int fd, uint64_t flags) {
    return (int)syscall2(0xB, (sc_qword_t)fd, (sc_qword_t)flags);
}

int load_module(const char *path, uint64_t flags) {
    int fd = open(path, 0);
    if(fd < 0 || syscall_load_module(fd, flags) != 0) {
        printf("Failed to load module\n");
        return -1;
    }
    close(fd);

    return 0;
}

int load_modules_from_list(const char *list) {
    FILE *fp = fopen(list, "r");
    if(fp == NULL) {
        return -1;
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t readVal;
    while((readVal = getline(&line, &len, fp)) != -1) {
        if(readVal == 0) continue; //Avoid newlines at the end
        if(line[readVal-1] == '\n') line[readVal-1] = 0;
        if(load_module(line, 0) < 0) {
            printf("Failed to load module from %s\n", line);

            if(line)
                free(line);
            fclose(fp);
            return -1;
        }
    }

    if(line)
        free(line);
    fclose(fp);
    return 0;
}

int main() {
    printf("WivOS Init program\n");

    load_modules_from_list("/etc/init_modules.cfg");

    while(1);

    const char *path = "/tests/test3";
    const char *args[] = { path, NULL };
    const char *environ[] = { NULL };
    execve(path, (char *const *)args, (char *const *)environ);

    printf("ERROR: Init program reached end\n");
    while(1);

    return 0;
}