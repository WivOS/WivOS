SHELL = /bin/bash

build:
	make -C test build
	make -C virtio build
	make -C virtiogpu build
	make -C packetfs build
	make -C shm build
	make -C usb build
	make -C xhci build
	make -C usbhid build

install:
	make -C test install
	make -C virtio install
	make -C virtiogpu install
	make -C packetfs install
	make -C shm install
	make -C usb install
	make -C xhci install
	make -C usbhid install

clean:
	make -C test clean
	make -C virtio clean
	make -C virtiogpu clean
	make -C packetfs clean
	make -C shm clean
	make -C usb clean
	make -C xhci clean
	make -C usbhid clean