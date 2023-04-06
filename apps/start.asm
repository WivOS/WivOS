global _start
_start:
    extern main
    call main

    jmp $