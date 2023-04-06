#include <modules/modules.h>
#include <utils/common.h>

char module_name[] = "test-string-please-ignore";

static size_t _init() {
    printf("Initing test module\n");
    return 0;
}

size_t a_function(void) {
    printf("Yeah i'm loaded from another module\n");
    return 2;
}

static size_t _exit() {
    printf("Goodbye :D\n");
    return 0;
}

MODULE_DEF(test, _init, _exit);