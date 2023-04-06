CFLAGS := -g -Wall -Wno-unused-variable -Wno-unused-function -no-pie -z noexecstack -I$(MAIN_PATH)/kernel/ -I$(MAIN_PATH)/external/lai/include/

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

GCC := x86_64-wivos-gcc

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(GCC) $(CFLAGS) -c $< -o $@

build: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(GCC) -o $(BIN_DIR)/$(APP_NAME) $(OBJS) $(CFLAGS)

install: build
	cp $(BIN_DIR)/$(APP_NAME) $(MAIN_PATH)/initrd/tests/

clean:
	rm -fr bin build