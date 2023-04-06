SHELL = /bin/bash

build:
	make -C test build
	make -C virtio build
	make -C virtiogpu build
	make -C packetfs build
	make -C shm build

install:
	make -C test install
	make -C virtio install
	make -C virtiogpu install
	make -C packetfs install
	make -C shm install

clean:
	make -C test clean
	make -C virtio clean
	make -C virtiogpu clean
	make -C packetfs clean
	make -C shm clean