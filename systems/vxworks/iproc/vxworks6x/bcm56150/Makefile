# Makefile - make rules for target/config/brcm_hurricane2 BSP
#
# Copyright (c) 2010,2013 Wind River Systems, Inc.
# 
# The right to copy, distribute, modify or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
#
# modification history
# --------------------
# 01a,25sep13,dnb  created from arm_a9_ctx  rev 01a
#
# DESCRIPTION
# This file contains rules for building VxWorks for the BRCM Hurricane2 board
# fitted with an ARM Cotex-A9 MPCore module.
#
# INCLUDES
#     makeTarget
#*/

CPU           = ARMARCH7
#TOOL          = sfdiab
TOOL          = sfgnu
override EXTRA_DEFINE  = -DCPU_CORTEXA8 -DARMMMU=ARMMMU_CORTEXA8 \
                -DARMCACHE=ARMCACHE_CORTEXA8 -DIP_LITTLE_ENDIAN -DBROADCOM_BSP
VFPVARIANT    =

TGT_DIR = $(WIND_BASE)/target

EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.8/osconfig/vxworks/src

include $(TGT_DIR)/h/make/defs.bsp

CONFIG_ALL   = all
USR_ENTRY    = usrInit

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

TARGET_DIR    = brcm_hurricane2
VENDOR        = BROADCOM
BOARD         = Hurricane2

RELEASE_CMD   += bootrom.bin

MACH_EXTRA    += genpll.o serdes.o iProcQspi.o iProcSmbus.o \
                 srecLoad.o flashDrvLib.o flashFsLib.o flashSflashDrvLib.o sflash.o

#
# The constants ROM_TEXT_ADRS and ROM_SIZE are defined
# in config.h and Makefile.
# All definitions for these constants must be identical.
#

#ROM_TEXT_ADRS  = 7fe00000 # ROM entry address
ROM_TEXT_ADRS  = 9fe00000 # ROM entry address
ROM_SIZE       = 00100000 # number of bytes of ROM space

#RAM_LOW_ADRS   = 68100000 # RAM text/data address
#RAM_HIGH_ADRS  = 7e000000 # RAM text/data address

RAM_LOW_ADRS   = 81100000 # RAM text/data address
RAM_HIGH_ADRS  = 9e000000 # RAM text/data address


VMA_START    = 0x$(ROM_TEXT_ADRS)

bsplist_clean:
	$(RM) depend.$(TARGET_DIR)
	$(RM) *.o
	$(RM) build_vars_list local_clean ipcom_ipdomain vxWorks.st content.romfs romfs_image.s
	$(RM) clean*

# Binary version of VxWorks ROM images, suitable for programming
# into Flash using tools provided by ARM.  If other ROM images need to
# be put into Flash, add similar rules here.

#bootrom.bin: bootrom
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary bootrom $@

#bootrom_res.bin: bootrom_res
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary bootrom_res $@

#bootrom_uncmp.bin: bootrom_uncmp
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary bootrom_uncmp $@

#vxWorks_rom.bin: vxWorks_rom
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary vxWorks_rom $@

#vxWorks.st_rom.bin: vxWorks.st_rom
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary vxWorks.st_rom $@

#vxWorks.res_rom.bin: vxWorks.res_rom
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary vxWorks.res_rom $@

#vxWorks.res_rom_nosym.bin: vxWorks.res_rom_nosym
#	- @ $(RM) $@
#	$(EXTRACT_BIN) -O binary vxWorks.res_rom_nosym $@

## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.

#include $(TGT_DIR)/h/make/rules.bsp
include ${SRC_DIR}/rules/rules.bsp

