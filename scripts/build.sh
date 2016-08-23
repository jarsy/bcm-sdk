#!/bin/bash

SCRIPTS_DIR=$1
BCM_SDK=$2
DST_DIR=$3

function print_usage
{
	echo "$0: Example: $0 <Scripts Dir> <BCM SDK Path> <Install Dir Path>"
	echo "$0: Example: $0 ./scripts ~/dev/git/bcm_sdk /tmp/bcm_sdk_artifacts"
}

################################################################################
# Check script user-supplied parameters
#
if [ -z $SCRIPTS_DIR ] ; then
	echo "$0: Error: Provide scripts directory!"
	print_usage
	exit 1;
fi

if [ -z $BCM_SDK ] ; then
	echo "$0: Error: Provide BCM SDK directory!"
	print_usage
	exit 1;
fi

if [ -z $DST_DIR ] ; then
	echo "Error: Provide destination directory!"
	print_usage
	exit 1;
fi

################################################################################
# Run the actual script
#
./$SCRIPTS_DIR/compile.sh $BCM_SDK
./$SCRIPTS_DIR/copy_artifacts.sh $BCM_SDK $DST_DIR
