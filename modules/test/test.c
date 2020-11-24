#include <modules/modules.h>
#include <util/util.h>

char module_name[] = "test-string-please-ignore";

static int _init() {
    printf("Initing test module\n");
    return 0;
}

int a_function(void) {
    printf("Yeah i'm loaded from another module\n");
    return 2;
}

static int _exit() {
    printf("Goodbye :D\n");
    return 0;
}

MODULE_DEF(test, _init, _exit);