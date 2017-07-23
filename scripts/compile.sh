#!/bin/bash

BCM_SDK=$1
COMPILE_DIR=$BCM_SDK/systems/linux/user/x86-smp_generic-4_9

################################################################################
# Check script user-supplied parameters
#
if [ -z $BCM_SDK ] ; then
	echo "$0: Error: Provide BCM SDK directory!"
	echo "$0: Example: $0 ~/My/MRV/OPX/BCM_SDK/Dev/git/bcm_sdk /tmp/bcm_sdk_artifacts"
	exit 1;
fi

export ARCH=x86
export TARGET=linux-kernel-4_9
export TOOLCHAIN_DIR=$HOME/clovis_2015/buildtools/i686-nptl-linux-gnu
export PATH=/usr/bin:$TOOLCHAIN_DIR/bin:$PATH
export CROSS_COMPILE=i686-nptl-linux-gnu-
export LDFLAGS="-L $TOOLCHAIN_DIR/lib"
export OPT_CFLAGS=" -m32 -DMRV_STANDALONE"
export MODULE_LDFLAGS=" -m elf_i386"

cd $COMPILE_DIR
if [ -s /opt/incredibuild/bin/ib_console ] && [ -n "$IB_CORES" ];
then
	echo "using incredibuild"
	export LINUX_MAKE_FLAGS="-j $IB_CORES"
	/opt/incredibuild/bin/ib_console make -j $IB_CORES
else
	make
fi;
cd -
