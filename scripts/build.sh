#!/bin/bash

BCM_SDK=$1
DST_DIR=$2

function print_usage
{
	echo "$0: Example: $0  <BCM SDK Path> <Install Dir Path>"
	echo "$0: Example: $0  ~/dev/git/bcm_sdk /tmp/bcm_sdk_artifacts"
}

################################################################################
# Check script user-supplied parameters
#
if [ -z $BCM_SDK ] ; then
	echo "$0: Error: Provide BCM SDK directory!"
	print_usage
	exit 1;
fi

if [ -z $DST_DIR ] ; then
	echo "Error: Provide destination directory for build artifacts!"
	print_usage
	exit 1;
fi

################################################################################
# Run the actual script
#
$BCM_SDK/scripts/compile.sh $BCM_SDK
if [ $? -ne 0 ];
then
	echo -e " FAILED\nERROR: failed to build BCM_SDK $BCM_SDK"
	exit 1
fi;
$BCM_SDK/scripts/copy_artifacts.sh $BCM_SDK $DST_DIR
if [ $? -ne 0 ]; then
	exit 1;
fi
