SHELL = /bin/bash

CFLAGS := -g -mno-red-zone -Wall -ffreestanding -Werror -Wno-unused-variable -Wno-strict-aliasing -Wno-unused-function -fno-pic -Ikernel -Iexternal/lai/include -O2 -mavx -Wno-address-of-packed-member -fno-omit-frame-pointer -fsanitize=undefined
LDFLAGS := -nostdlib -no-pie

INITRD_DIR := ./initrd

TOOLCHAIN_DESTDIR := /home/jesus/proyectos/RWivOS/toolchain/sysroot/

ifeq ($(DEBUG), 1)
	BIN_DIR := bin/DEBUG
	BUILD_DIR := build/DEBUG
else
	BIN_DIR := bin/RELEASE
	BUILD_DIR := build/RELEASE
endif

include boot.mk

include lai.mk
SRCS += kernel/utils/lai_host.c

SRCS += kernel/cpu/cpu.c
SRCS += kernel/cpu/cpu.asm
SRCS += kernel/cpu/gdt.c
SRCS += kernel/cpu/idt.c
SRCS += kernel/cpu/isr.asm
SRCS += kernel/cpu/acpi.c

SRCS += kernel/mem/pmm.c
SRCS += kernel/mem/vmm.c

SRCS += kernel/devices/pci.c
SRCS += kernel/devices/timer.c
SRCS += kernel/devices/nvme.c

SRCS += kernel/tasking/process.c
SRCS += kernel/tasking/thread.c
SRCS += kernel/tasking/scheduler.c
SRCS += kernel/tasking/syscalls.c
SRCS += kernel/tasking/syscalls.asm
SRCS += kernel/tasking/event.c

SRCS += kernel/fs/vfs.c
SRCS += kernel/fs/devfs/devfs.c
SRCS += kernel/fs/pipe/pipe.c
SRCS += kernel/fs/partfs/partfs.c
SRCS += kernel/fs/fat32/fat32.c

SRCS += kernel/modules/modules.c

SRCS += kernel/utils/common.c
SRCS += kernel/utils/system.c
SRCS += kernel/utils/string.c
SRCS += kernel/utils/kmalloc.c
SRCS += kernel/utils/lists.c
SRCS += kernel/utils/ubsan.c

.PHONY: default qemu image clean clean-all toolchain

default: image

INITRD := initrd

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
external := $(OBJS:%.o=%.d)
-include $(external)

$(BIN_DIR)/wivos.elf: $(BUILD_DIR)/kernel/boot/trampoline.bin $(OBJS) $(BUILD_DIR)/kernel/symbols.o
	@mkdir -p $(@D)
	clang -fuse-ld=lld $(LDFLAGS) -o $@ $(OBJS) $(BUILD_DIR)/kernel/symbols.o

$(BUILD_DIR)/kernel/symbols.o: $(BUILD_DIR)/kernel/boot/trampoline.bin $(OBJS) generate_symbols.py
	@mkdir -p $(@D)
	clang -fuse-ld=lld $(LDFLAGS) -o $(BUILD_DIR)/wivost.elf $(OBJS)
	nm $(BUILD_DIR)/wivost.elf -g -P | python2 generate_symbols.py > kernel/symbols.s
	nasm kernel/symbols.s -f elf64 -o $@

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	clang -m64 -MMD $(CFLAGS) -D__FILENAME__="\"$<\"" -D__MODULE__="\"$(notdir $(basename $<))\"" -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf64 -o $@

$(BUILD_DIR)/%.bin: %.real
	@mkdir -p $(@D)
	nasm $< -f bin -o $@

build-modules:
	@make -C modules install

build-libs:
	@make -C libs install

build-apps: build-libs
	@make -C apps install

image: $(BIN_DIR)/image.hdd

$(BIN_DIR)/image.hdd: $(BIN_DIR)/wivos.elf build-modules build-apps toolchain/sysroot/usr/lib/ld.so toolchain/sysroot/usr/lib/libc.so
	@cp -rf toolchain/sysroot/usr/lib/ld.so initrd/lib/ld.so
	@cp -rf toolchain/sysroot/usr/lib/ld.so initrd/usr/lib/ld.so
	@cp -rf toolchain/sysroot/usr/lib/libc.so initrd/lib/libc.so
	@cp -rf toolchain/sysroot/usr/lib/libc.so initrd/usr/lib/libc.so
	@cp -rf toolchain/sysroot/usr/lib initrd/usr
	@mkdir -p $(@D)
	@echo "Creating disk"
	@rm -rf $@
	@dd if=/dev/zero bs=1M count=0 seek=512 of=$@ #64
	@echo "Creating echfs partition"
	@parted -s $@ mklabel gpt
	@parted -s $@ mkpart primary 2048s 100%
	@rm -rf test_image/
	@mkdir test_image
	@sudo losetup -Pf --show $(BIN_DIR)/image.hdd > loopback_dev
	@sudo partprobe `cat loopback_dev`
	@sudo mkfs.fat -F 32 `cat loopback_dev`p1
	@sudo mount `cat loopback_dev`p1 test_image
	@sudo mkdir test_image/boot
	@sudo cp $(BIN_DIR)/wivos.elf test_image/wivos.elf
	@sudo cp boot/limine.cfg test_image/boot/limine.cfg
	@sudo mkdir -p test_image/EFI/BOOT
	@sudo cp boot/BOOTX64.EFI test_image/EFI/BOOT/
	@sudo cp -rf $(INITRD_DIR)/. test_image/
	@sync
	@sudo umount test_image/

	@sudo losetup -d `cat loopback_dev`
	@rm -rf test_image loopback_dev

qemu: $(BIN_DIR)/image.hdd
	source ~/.bashrc && qemu-system-x86_64 -enable-kvm -cpu host -m 1024M -M q35 -bios boot/OVMF.fd -net none -smp 4 -drive file=$(BIN_DIR)/image.hdd,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm -debugcon stdio -device virtio-vga-gl,disable-legacy=on,xres=1280,yres=1024 -display sdl,gl=on -device qemu-xhci -device usb-kbd -device usb-tablet --no-reboot --no-shutdown

qemu-debug:
	qemu-system-x86_64 -enable-kvm -cpu host -m 1024M -M q35 -bios boot/OVMF.fd -net none -smp 4 -drive file=$(BIN_DIR)/image.hdd,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm -debugcon stdio -device virtio-vga-gl,xres=1280,yres=1024 -display sdl,gl=on --trace "virtio_gpu_cmd_*" -device qemu-xhci -device usb-kbd --trace "usb_desc_*" --no-reboot --no-shutdown -S -s

clean:
	rm -fr build

clean-all: clean
	rm -fr bin $(BIN_DIR) $(BUILD_DIR) test_image

toolchain:
	@pushd toolchain/build ; \
	DESTDIR=$(TOOLCHAIN_DESTDIR) ninja install ; \
	popd