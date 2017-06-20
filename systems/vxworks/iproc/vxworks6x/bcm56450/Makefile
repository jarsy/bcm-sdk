# Makefile - makefile for BRCM katana2 BSP
# $Id: Makefile,v 1.10 2011/07/21 16:14:17 yshtil Exp $
# Copyright (c) 2013 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
# 
# modification history
# --------------------
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# BRCM Katana2 board.
#
# INCLUDES
#     makeTarget
#*/

CPU             = ARMARCH7
TOOL            = gnu

override EXTRA_DEFINE  += -DCPU_CORTEXA8 -DARMMMU=ARMMMU_CORTEXA8 \
                -DARMCACHE=ARMCACHE_CORTEXA8 -DIP_LITTLE_ENDIAN
VFPVARIANT    = VFPv3

# Support for 36 bit physical addresses in the MMU can be added for
# vxWorks 6.2 and later with the following CPU_VARIANT.


TGT_DIR = $(WIND_BASE)/target


EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.8/osconfig/vxworks/src

include $(TGT_DIR)/h/make/defs.bsp

CONFIG_ALL      = all

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.


TARGET_DIR	= bcm56450
VENDOR		= Broadcom 
BOARD		= Katana2

ELEASE_CMD   += bootrom.bin

MACH_EXTRA    += genpll.o srecLoad.o serdes.o iProcSmbus.o flashDrvLib.o flashFsLib.o flashSflashDrvLib.o sflash.o

#
# The constants ROM_TEXT_ADRS and ROM_SIZE are defined
# in config.h and Makefile.
# All definitions for these constants must be identical.
#

ROM_TEXT_ADRS  = 41000000 # ROM entry address
ROM_SIZE       = 00100000 # number of bytes of ROM space

RAM_LOW_ADRS   = 68100000 # RAM text/data address
RAM_HIGH_ADRS  = 6c000000 # RAM text/data address

VMA_START    = 0x$(ROM_TEXT_ADRS)
USR_ENTRY        = usrInit

bsplist_clean:
	$(RM) depend.$(TARGET_DIR)
	$(RM) *.o
	$(RM) build_vars_list local_clean ipcom_ipdomain vxWorks.st content.romfs romfs_image.s
	$(RM) clean*

# Besides the official release vxworks 6.8.3, PCIe in iProc has to be supported.
# WindRiver gives some updates :
#   (1) libdrv.a  (should be put in $(WIND_BASE)/target/lib/arm/ARMARCH7/common)
#    => Put in sdk/systems/vxworks/iproc/vxworks6x/bcm56450/rules.
#   (2) libusb2.a (should be put in $(WIND_BASE)/target/lib/arm/ARMARCH7/common)
#    => Put in sdk/systems/vxworks/iproc/vxworks6x/bcm56450/rules.
#   (3) vxbUsrCmdLine.c (should be put in $(WIND_BASE)/target/config/all)
#    => Put in sdk/systems/vxworks/iproc/vxworks6x/bcm56450/all.
#
# Add LD_LINK_PATH for (1) and (2) to reference while linking.

#LD_LINK_PATH = -L${SRC_DIR}/rules
#LD_LINK_PATH += -L${TGT_DIR}/lib/arm/${CPU}/gnu_standard
#LD_LINK_PATH += -L${TGT_DIR}/lib/arm/${CPU}/gnu
#LD_LINK_PATH += -L${TGT_DIR}/lib/arm/${CPU}/common

# Using original rules.bsp will cause following compile error :
#     sal/appl/config.h: No such file or directory 
# It is due to double compile in platform_defines.c and version.c.
# Modify its rule for vxWorks.st(#629 ~ #631) will fix this proble.  

#include $(TGT_DIR)/h/make/rules.bsp
include ${SRC_DIR}/rules/rules.bsp

