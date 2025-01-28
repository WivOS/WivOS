CFLAGS := -g -fpic -Wall -Wno-unused-variable -Wno-unused-function -z noexecstack

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

include $(MAIN_PATH)/env.mk
GCC := $(TOOLPATH)/x86_64-wivos-gcc

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(GCC) $(CFLAGS) -c $< -o $@

build: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(GCC) -shared -o $(BIN_DIR)/lib$(LIB_NAME).so $(OBJS) $(CFLAGS)

install: build
	cp $(BIN_DIR)/lib$(LIB_NAME).so $(MAIN_PATH)/initrd/lib/

clean:
	rm -fr bin build