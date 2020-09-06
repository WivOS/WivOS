CFLAGS := -Wall -Werror -ffreestanding -Wno-unused-variable -Wno-parentheses -Wno-unused-function -fno-pic -Ikernel -O2 -g
LDFLAGS := -nostdlib -no-pie

ifeq ($(DEBUG), 1)
	BIN_DIR := bin/DEBUG
	BUILD_DIR := build/DEBUG
else
	BIN_DIR := bin/RELEASE
	BUILD_DIR := build/RELEASE
endif

SRCS += kernel/util/debug.c
SRCS += kernel/util/util.c
SRCS += kernel/util/string.c
SRCS += kernel/util/lock.c
SRCS += kernel/mem/pmm.c
SRCS += kernel/mem/vmm.c
SRCS += kernel/mem/mm.c
SRCS += kernel/cpu/gdt.c
SRCS += kernel/cpu/idt.c

SRCS += kernel/cpu/isr.asm

#CFLAGS += -Ideps/lai/include
#SRCS += deps/lai/core/error.c
#SRCS += deps/lai/core/eval.c
#SRCS += deps/lai/core/exec.c
#SRCS += deps/lai/core/exec-operand.c
#SRCS += deps/lai/core/libc.c
#SRCS += deps/lai/core/ns.c
#SRCS += deps/lai/core/object.c
#SRCS += deps/lai/core/opregion.c
#SRCS += deps/lai/core/os_methods.c
#SRCS += deps/lai/core/variable.c
#SRCS += deps/lai/core/vsnprintf.c
#SRCS += deps/lai/helpers/pci.c
#SRCS += deps/lai/helpers/pm.c
#SRCS += deps/lai/helpers/resource.c
#SRCS += deps/lai/helpers/sci.c

include boot.mk

.PHONY: default qemu image clean clean-all

default: image

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:%.o=%.d)
-include $(DEPS)

$(BIN_DIR)/wivos.elf: $(OBJS)
	@mkdir -p $(@D)
	ld $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	gcc -m64 -MMD $(CFLAGS) -D__FILENAME__="\"$<\"" -D__MODULE__="\"$(notdir $(basename $<))\"" -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf64 -o $@

qemu: $(BIN_DIR)/image.hdd
	qemu-system-x86_64.exe -hdd $^ --accel whpx -m 4G -smp 4 -machine q35 -debugcon stdio

image: $(BIN_DIR)/image.hdd

$(BIN_DIR)/image.hdd: $(BIN_DIR)/wivos.elf boot/limine.cfg boot/limine.bin boot/limine-install
	@mkdir -p $(@D)
	@echo "Creating disk"
	@rm -rf $@
	@dd if=/dev/zero bs=1M count=0 seek=64 of=$@
	@echo "Creating echfs partition"
	@parted -s $@ mklabel msdos
	@parted -s $@ mkpart primary 1 100%
	@echfs-utils -m -p0 $@ quick-format 32768
	@echo "Importing files"
	@echfs-utils -m -p0 $@ import $(BIN_DIR)/wivos.elf wivos.elf
	@echfs-utils -m -p0 $@ import boot/limine.cfg limine.cfg
	@echo "Installing limine"
	@boot/limine-install boot/limine.bin $@

clean:
	rm -fr build

clean-all: clean
	rm -fr bin