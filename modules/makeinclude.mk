CFLAGS := -fno-pie -mno-red-zone -Wall -ffreestanding -Werror -Wno-unused-variable -Wno-unused-function -fno-pic -I$(MAIN_PATH)/kernel -I$(MAIN_PATH)/external/lai/include -O2 -mcmodel=large -mstack-alignment=8

ifeq ($(DEBUG), 1)
	BIN_DIR := bin/DEBUG
else
	BIN_DIR := bin/RELEASE
endif

build: $(SRCS)
	@mkdir -p $(BIN_DIR)
	clang -m64 -MMD -c -o $(BIN_DIR)/$(MODULE_NAME).wko $(SRCS) $(CFLAGS)

install: build
	cp $(BIN_DIR)/$(MODULE_NAME).wko $(MAIN_PATH)/initrd/$(INSTALL_PATH)/

clean:
	rm -fr bin