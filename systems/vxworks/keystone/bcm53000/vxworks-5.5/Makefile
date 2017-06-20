# Makefile - makefile for Wind River BCM53000 BSP
# $Id: Makefile,v 1.8 2011/07/21 16:14:24 yshtil Exp $
# Copyright (c) 2006-2007 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
# 
# modification history
# --------------------
#
# DESCRIPTION
#
# INCLUDES
#     makeTarget
#*/

CPU             = MIPS32
ifeq ($(VX_TARGET_BIG_ENDIAN),1)
TOOL            = sfgnu
ENDIAN_FLAGS    = -DIL_BIGENDIAN=1 -DBE_HOST=1
#BSP_LIB         = libkeystonebe.a
else
TOOL            = sfgnule
ENDIAN_FLAGS    =
#BSP_LIB         = libkeystonele.a
endif
CPU_VARIANT	= _mti4kx

ifeq "$(HOSTTYPE)" "Windows2000PC"
export WIND_BASE ?= c:/tornado2.2/
export WIND_HOST_TYPE ?= x86-win32
else
export WIND_BASE ?= /projects/ntsw-tools/wrs/tornado/t2.2.1
export WIND_HOST_TYPE ?= sun4-solaris2
endif

#PATH := /projects/ntsw-tools/wrs/tornado/t2.2.1/host/sun4-solaris2/bin:${PATH}

#ifeq ($(BOOT),yes)
ifneq (, $(findstring bootrom, $(MAKECMDGOALS)))
BLD_FLAG = -DBUILD_VXBOOT -DVX_VERSION=${VX_VERSION} 
override EXTRA_DEFINE += -DBUILD_VXBOOT
else
BLD_FLAG = -DCFE_API_ALL
BLD_FLAG += -DSHUNT_COMMAND
#override EXTRA_DEFINE += -DHAZARD_FILE=mipsR2Hazards.h
endif

BLD_FLAG += -DBCMDRIVER -DintUnlock=intUnlockSafe

ifeq "$(HOSTTYPE)" "Windows2000PC"
NEW_RULES = TRUE
USRCONFIG	= $(CONFIG_ALL)/usrConfig.c
BOOTCONFIG	= $(CONFIG_ALL)/bootConfig.c
BOOTINIT	= $(CONFIG_ALL)/bootInit.c 
DATASEGPAD	= $(CONFIG_ALL)/dataSegPad.c
CONFIG_ALL_H	= $(CONFIG_ALL)/configAll.h

export NEW_RULES
export USRCONFIG
export BOOTCONFIG
export BOOTINIT
export DATASEGPAD
export CONFIG_ALL_H
endif

TGT_DIR = $(WIND_BASE)/target
include $(TGT_DIR)/h/make/defs.bsp

ifeq "$(HOSTTYPE)" "Windows2000PC"
CP	= cp
LDOUT_CONV	= true
endif

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

CONFIG_ALL = all


#TARGET_DIR	= bcm53000_mips32sf
TARGET_DIR	= bcm53000
VENDOR		= Broadcom 
BOARD		= Keystone 

RELEASE		+= bootrom_uncmp.hex bootrom_uncmp.bin 

ifeq "$(HOSTTYPE)" "Windows2000PC"
CFLAGS		+= -DHAZARD_FILE="mipsR2Hazards.h"
else
CFLAGS		+= -DHAZARD_FILE="\"mipsR2Hazards.h\""
endif

CFLAGS   += -Dvxworks ${ENDIAN_FLAGS} ${BLD_FLAG}

#ENDIAN_FLAGS    = -DIL_BIGENDIAN=1 -DBE_HOST=1

EXTRA_DEFINE  = -Dvxworks $(ENDIAN_FLAGS) $(BLD_FLAG) \
		-DBROADCOM_BSP -DIP_PORT_VXWORKS=55 
	    
EXTRA_INCLUDE = -I$(WIND_BASE)/target/h/nfs
EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps
EXTRA_INCLUDE += -Iproto 

## The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
## in config.h and Makefile. All definitions for these constants must be
## identical.

ROM_TEXT_ADRS           = bfc00000      # ROM entry address
ROM_SIZE                = 00200000      # Number of bytes of ROM space

RAM_LOW_ADRS            = 80100000      # RAM text/data address
RAM_HIGH_ADRS           = 83c00000

BOOT_EXTRA       = 

VMA_START        = 0x0
USR_ENTRY        = usrInit

ifneq (, $(findstring bootrom, $(MAKECMDGOALS)))
ETROBO_OBJS =
else
ifdef ROBO_CHIPS
# Et robo driver
ETROBO_FLAGS = $(CFLAGS) -DMBZ -DVXWORKS
ETROBO_SRC_DIR = ../../../../drv/et
ETROBO_OBJS = et/etc_robo_spi.o
et/%.o : $(ETROBO_SRC_DIR)/%.c 
	mkdir -p et 
	$(CC) -I${SDK}/include -I$(ETROBO_SRC_DIR) $(ETROBO_FLAGS) -c $< -o $@
else
ETROBO_OBJS =
endif
endif

MACH_EXTRA      = srecLoad.o ns16550Sio.o \
                  cacheLib.o \
                  et_vx.o etc.o etcgmac.o vx_osl.o hnddma.o \
                  nicpci.o hndmips.o hndpci.o \
                  sysSerial.o hndchipc.o\
                  sbutils.o siutils.o aiutils.o bcmutils.o pciConfigLib.o pciIntLib.o \
                  hndpmu.o bcmallocache.o bcmsrom.o nvram_vxworks.o nvram.o \
                  flash29l640DrvLib.o flash29l320DrvLib.o flash29l160DrvLib.o \
                  flash28f320DrvLib.o flash29gl128DrvLib.o flashS29GL256PDrvLib.o \
                  flashSflashDrvLib.o \
                  flashDrvLib.o flashFsLib.o flashutl.o sflash.o \
                  chipc_i2c.o i2cM41T81Clock.o snprintf.o \
                  $(ETROBO_OBJS) 

#LIB_EXTRA = libkeystone.a 
#LIB_EXTRA = BSP_LIB
ifeq ($(VX_TARGET_BIG_ENDIAN),1)
LIB_EXTRA=libkeystonebe.a
else
LIB_EXTRA=libkeystonele.a
endif


local_clean:
	$(RM) *.o
	$(RM) depend.$(TARGET_DIR)
	$(RM) et/*.o
 
## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp

