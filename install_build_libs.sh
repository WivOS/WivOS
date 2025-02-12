#!/bin/bash

cp toolchain/build/lib*.so initrd/lib/
cp toolchain/build/lib*.so initrd/usr/lib/
cp toolchain/build/ld.so initrd/lib/
cp toolchain/build/ld.so initrd/usr/lib/