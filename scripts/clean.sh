#!/bin/bash

BCM_SDK=$1

################################################################################
# Check script user-supplied parameters
#
if [ -z $BCM_SDK ] ; then
	echo "$0: Error: Provide BCM SDK directory!"
	print_usage
	exit 1;
fi

echo "cleaning bcm sdk arch..."
source $BCM_SDK/scripts/env_export_sdk-6.5.7
if [ $? -ne 0 ]; then
	exit 1;
fi

# ib_console
make clean
rm -rf ./build
echo -e "cleaning bcm sdk done"

date 1>&2
