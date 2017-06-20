#!/bin/sh
# $Id: auto_sbx.sh,v 1.11 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

cd /broadcom
config_bcm_set=0

# First, try to mount fatfs and locate config.bcm
if [ -b /dev/mtdblock1 ]
then 
    mount -t vfat /dev/mtdblock1 /mnt
    if [ -f  /mnt/config.bcm ]
    then
        export BCM_CONFIG_FILE=/mnt/config.bcm
        config_bcm_set=1
    fi
fi

# If could not find config.bcm, then use board detection
if [ $config_bcm_set -ne 1 ] 
then
    if [ -r  /proc/bus/pci/00/14.0 ]
        then
        export BCM_CONFIG_FILE=/broadcom/config-sbx-polaris.bcm
    elif [ -d /proc/bus/pci/02 ]
        then
        export BCM_CONFIG_FILE=/broadcom/config-sbx-sirius.bcm
    elif [ -d /proc/bus/pci/11.0 ]
        then
        export BCM_CONFIG_FILE=/broadcom/config-sbx-fe2kxt.bcm
    fi
fi
./bcm.user

