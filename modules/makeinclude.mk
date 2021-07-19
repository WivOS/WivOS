CFLAGS := -no-pie -g -mno-red-zone -Wall -ffreestanding -Werror -Wno-unused-variable -Wno-unused-function -fno-pic -I$(MAIN_PATH)/kernel -I$(MAIN_PATH)/external/lai/include -O2 -mcmodel=large

ifeq ($(DEBUG), 1)
	BIN_DIR := bin/DEBUG
else
	BIN_DIR := bin/RELEASE
endif

build: $(SRCS)
	@mkdir -p $(BIN_DIR)
	gcc -m64 -c -o $(BIN_DIR)/$(MODULE_NAME).wko $(SRCS) $(CFLAGS)

install: build
	cp $(BIN_DIR)/$(MODULE_NAME).wko $(MAIN_PATH)/initrd/

clean:
	rm -fr bin
