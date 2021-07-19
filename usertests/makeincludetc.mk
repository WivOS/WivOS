CFLAGS := -Wall -Wno-unused-variable -Wno-unused-function

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
	$(MAIN_PATH)/sysroot/bin/x86_64-wivos-gcc $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.asm.o: %.asm
	@mkdir -p $(@D)
	nasm $< -f elf64 -o $@

build: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(MAIN_PATH)/sysroot/bin/x86_64-wivos-gcc -o $(BIN_DIR)/$(APP_NAME) $(OBJS) $(CFLAGS)

install: build
	cp $(BIN_DIR)/$(APP_NAME) $(MAIN_PATH)/initrd/tests/

clean:
	rm -fr bin build