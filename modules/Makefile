SHELL = /bin/bash

build:
	make -C test build
	make -C test2 build
	make -C virtiogpu build
	make -C sb16 build
	make -C pcnet build

install:
	make -C test install
	make -C test2 install
	make -C virtiogpu install
	make -C sb16 install
	make -C pcnet install

clean:
	make -C test clean
	make -C test2 clean
	make -C virtiogpu clean
	make -C sb16 clean
	make -C pcnet clean