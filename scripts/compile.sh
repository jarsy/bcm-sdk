#!/bin/bash

BCM_SDK=$1
COMPILE_DIR=$BCM_SDK/systems/linux/user/x86-smp_generic-2_6

export ARCH=x86
export TARGET=linux-x86-smp_generic-2_6
export TOOLCHAIN_DIR=~/clovis_2015/buildtools/i686-nptl-linux-gnu
export PATH=/usr/bin:$TOOLCHAIN_DIR/bin:$PATH
export CROSS_COMPILE=i686-nptl-linux-gnu-
export LDFLAGS="-L $TOOLCHAIN_DIR/lib"
export OPT_CFLAGS=" -m32"
export MODULE_LDFLAGS=" -m elf_i386"
export KERNDIR=~/My/MRV/OPX/2016_05_23_git/main_dev/dist/boot_src/linux

cd $COMPILE_DIR
make -s -j

cd -
