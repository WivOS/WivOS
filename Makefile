SHELL = /bin/bash

CFLAGS := -g -Wall -ffreestanding -Werror -Wno-unused-variable -Wno-strict-aliasing -Wno-unused-function -fno-pic -Ikernel -Iexternal/lai/include -O2 -g -mavx -Wno-address-of-packed-member
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
SRCS += kernel/util/list.c
SRCS += kernel/mem/pmm.c
SRCS += kernel/mem/vmm.c
SRCS += kernel/mem/mm.c
SRCS += kernel/cpu/gdt.c
SRCS += kernel/cpu/idt.c
SRCS += kernel/cpu/pci.c
SRCS += kernel/acpi/laihost.c
SRCS += kernel/acpi/acpi.c
SRCS += kernel/acpi/apic.c

SRCS += kernel/proc/smp.c
SRCS += kernel/proc/proc.c
SRCS += kernel/proc/syscalls.c

SRCS += kernel/fs/vfs.c

SRCS += kernel/fs/initrd/tar.c

SRCS += kernel/fs/devfs/devfs.c

SRCS += kernel/modules/modules.c

SRCS += kernel/cpu/isr.asm

CFLAGS += -Iexternal/lai/include
SRCS += external/lai/core/error.c
SRCS += external/lai/core/eval.c
SRCS += external/lai/core/exec.c
SRCS += external/lai/core/exec-operand.c
SRCS += external/lai/core/libc.c
SRCS += external/lai/core/ns.c
SRCS += external/lai/core/object.c
SRCS += external/lai/core/opregion.c
SRCS += external/lai/core/os_methods.c
SRCS += external/lai/core/variable.c
SRCS += external/lai/core/vsnprintf.c
SRCS += external/lai/helpers/pci.c
SRCS += external/lai/helpers/pm.c
SRCS += external/lai/helpers/resource.c
SRCS += external/lai/helpers/sci.c

include boot.mk

.PHONY: default qemu image clean clean-all

default: image

INITRD := initrd

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
external := $(OBJS:%.o=%.d)
-include $(external)

$(BIN_DIR)/wivos.elf: $(OBJS) $(BUILD_DIR)/kernel/symbols.o
	@mkdir -p $(@D)
	ld $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/kernel/symbols.o: $(OBJS) generate_symbols.py
	@mkdir -p $(@D)
	ld $(LDFLAGS) -o $(BUILD_DIR)/wivost.elf $(OBJS)
	nm $(BUILD_DIR)/wivost.elf -g | python2 generate_symbols.py > kernel/symbols.s
	nasm kernel/symbols.s -f elf64 -o $@

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	gcc -m64 -MMD $(CFLAGS) -D__FILENAME__="\"$<\"" -D__MODULE__="\"$(notdir $(basename $<))\"" -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf64 -o $@

$(BUILD_DIR)/%.bin: %.real
	@mkdir -p $(@D)
	nasm $< -f bin -o $@

qemu: $(BIN_DIR)/image.hdd
	powershell.exe -File run.ps1

qemu2: $(BIN_DIR)/image.hdd
	VREND_DEBUG="guestallow" /usr/local/bin/qemu-system-x86_64 --enable-kvm -hdd bin/RELEASE/image.hdd -m 4G -smp 4 -machine q35 -debugcon stdio -device virtio-gpu,virgl=on -display sdl,gl=on -device qemu-xhci -drive id=pendrive,file=test.img,format=raw,if=none -device usb-storage,drive=pendrive

image: $(BIN_DIR)/image.hdd

modulesMake:
	@make -C modules install

appsMake:
	@make -C usertests install

$(BIN_DIR)/initrd.tar: $(INITRD)/*
	@shopt -s dotglob && pushd $(INITRD) > /dev/null && tar -cf ../$@ * && popd > /dev/null

$(BIN_DIR)/image.hdd: $(BUILD_DIR)/kernel/proc/trampoline.bin $(BIN_DIR)/wivos.elf modulesMake appsMake $(BIN_DIR)/initrd.tar boot/limine.cfg boot/limine.bin boot/limine-install
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
	@echfs-utils -m -p0 $@ import $(BIN_DIR)/initrd.tar initrd.tar
	@echfs-utils -m -p0 $@ import boot/limine.cfg limine.cfg
	@echo "Installing limine"
	@boot/limine-install boot/limine.bin $@

clean:
	rm -fr build

clean-all: clean
	rm -fr bin