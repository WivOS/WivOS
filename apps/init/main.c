#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

int main() {
    printf("WivOS Init program\n");

    const char *path = "/tests/test3";
    const char *args[] = { path, NULL };
    const char *environ[] = { NULL };
    execve(path, args, environ);

    printf("ERROR: Init program reached end\n");
    while(1);

    return 0;
}