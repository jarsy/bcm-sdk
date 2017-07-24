#!/bin/bash

bcm_sdk_dir=$1 # BCM SDK source code directory (probably, just current directory)
bcm_to_dir=$2  # BCM SDK artifacts directory in "main" project

to_dir_deploy=$bcm_to_dir/deploy
if [ ! -d $to_dir_deploy ] ; then
    mkdir -p $to_dir_deploy
fi

from_dir=$bcm_sdk_dir/systems/linux/user/x86-smp_generic-4_9
cp $from_dir/linux-kernel-bde.ko $to_dir_deploy
cp $from_dir/linux-user-bde.ko   $to_dir_deploy
