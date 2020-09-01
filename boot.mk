CFLAGS += -mno-sse -mno-sse2 -mno-mmx -mno-80387 -m64
CFLAGS += -mno-red-zone -mcmodel=kernel

CFLAGS += -D__WIVOS__

LDFLAGS += -z max-page-size=0x1000
LDFLAGS += -T linker.ld

SRCS += kernel/boot/entry.c