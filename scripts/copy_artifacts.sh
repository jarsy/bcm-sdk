#!/bin/bash -x

from_dir_bcm_sdk=$1     # BCM SDK source code directory (probably, just current directory)
to_dir_bcm_artifacts=$2 # BCM SDK artifacts directory in "main" project

# let's shorten names
from=$from_dir_bcm_sdk
to=$to_dir_bcm_artifacts

$from/scripts/cp_hal_bcm_kernel_modules.sh $from $to
$from/scripts/cp_hal_bcm_headers_kosta.sh  $from $to
$from/scripts/cp_hal_bcm_x86_libs_kosta.sh $from $to
