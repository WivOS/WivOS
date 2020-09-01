CFLAGS := -Wall -Werror -ffreestanding -fno-pic -Ikernel -O2 -g
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