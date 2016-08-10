#!/bin/bash

SCRIPTS_DIR=$1
BCM_SDK=$2
DST_DIR=$3

./$SCRIPTS_DIR/compile.sh $BCM_SDK
./$SCRIPTS_DIR/copy_artifacts.sh $BCM_SDK $DST_DIR
