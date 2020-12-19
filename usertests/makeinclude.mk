CFLAGS := -g -Wall -Wno-unused-variable -Wno-unused-function -no-pie -nostdlib -fno-builtin -I$(MAIN_PATH)/kernel/ -I$(MAIN_PATH)/external/lai/include/

ifeq ($(DEBUG), 1)
	BIN_DIR := bin/DEBUG
	BUILD_DIR := build/DEBUG
else
	BIN_DIR := bin/RELEASE
	BUILD_DIR := build/RELEASE
endif

OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
external := $(OBJS:%.o=%.d)
-include $(external)

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	gcc -m64 $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf64 -o $@

$(BUILD_DIR)/start.asm.o: ../start.asm
	@mkdir -p $(@D)
	nasm $< -f elf64 -o $@

build: $(OBJS) $(BUILD_DIR)/start.asm.o
	@mkdir -p $(BIN_DIR)
	gcc -m64 -o $(BIN_DIR)/$(APP_NAME) $(OBJS) $(BUILD_DIR)/start.asm.o $(CFLAGS) -T $(LINK_SCRIPT)

install: build
	cp $(BIN_DIR)/$(APP_NAME) $(MAIN_PATH)/initrd/tests/

clean:
	rm -fr bin build