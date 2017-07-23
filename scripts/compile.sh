#!/bin/bash

BCM_SDK=$1
COMPILE_DIR=$BCM_SDK/systems/linux/user/x86-smp_generic-4_9

################################################################################
# Check script user-supplied parameters
#
if [ -z "$BCM_SDK" ] ; then
	echo "$0: Error: Provide BCM SDK directory!"
	echo "$0: Example: $0 ~/My/MRV/OPX/BCM_SDK/Dev/git/bcm_sdk /tmp/bcm_sdk_artifacts"
	exit 1;
fi

. $BCM_SDK/scripts/env_export_sdk-6.5.7_lk-4.9.env

cd $COMPILE_DIR
if [ -s /opt/incredibuild/bin/ib_console ] && [ -n "$IB_CORES" ];
then
	echo "using incredibuild"
	export LINUX_MAKE_FLAGS="-j $IB_CORES"
	/opt/incredibuild/bin/ib_console make -j $IB_CORES
else
	make
fi;
if [ $? != 0 ];
then
	echo -e " FAILED\nERROR: failed to compile BCM_SDK"
	exit 1
fi;
cd -
