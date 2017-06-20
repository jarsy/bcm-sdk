# Makefile - makefile for Wind River BCM53000 BSP
# $Id: Makefile,v 1.20 2011/07/21 16:14:28 yshtil Exp $
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

# Endian is determined by VX_TARGET_BIG_ENDIAN if BE is missing
ifdef BE
export VX_TARGET_BIG_ENDIAN = ${BE}
endif 

CPU             = MIPSI32R2
ifeq ($(VX_TARGET_BIG_ENDIAN),1)
TOOL            = sfgnu
ENDIAN_FLAGS    = -DIL_BIGENDIAN=1 -DBE_HOST=1
else
TOOL            = sfgnule
ENDIAN_FLAGS    =
endif
CPU_VARIANT     = _mti24kx

ifeq "$(HOSTTYPE)" "Windows2000PC"
export WIND_HOME ?= c:/windriver/
export WIND_BASE ?= $(WIND_HOME)/vxworks-6.6
export WIND_HOST_TYPE ?= x86-win32
TCL_BASE ?= $(WIND_HOME)/setup/$(WIND_HOST_TYPE)/
else
export WIND_HOME ?= /tools/windriver/vxworks/6.6
export WIND_BASE ?= $(WIND_HOME)/vxworks-6.6
export WIND_HOST_TYPE ?= x86-linux2
endif

ifneq (, $(findstring bootrom, $(MAKECMDGOALS)))
BLD_FLAG += -DBUILD_VXBOOT
override EXTRA_DEFINE += -DBUILD_VXBOOT
else
BLD_FLAG += -DCFE_API_ALL
BLD_FLAG += -DSHUNT_COMMAND
override EXTRA_DEFINE += -DHAZARD_FILE=mipsR2Hazards.h
endif

BLD_FLAG += -DBCMDRIVER -DintUnlock=intUnlockSafe


ifeq ($(ROBODVT),1)
BLD_FLAG += -DROBODVT
endif

TGT_DIR = $(WIND_BASE)/target

EXTRA_INCLUDE += -I$(WIND_BASE)/target/h/nfs
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.6/osconfig/vxworks/src
EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps

ifeq ($(VX_TARGET_BIG_ENDIAN),1)
LD_LINK_PATH_EXTRA += -L$(WIND_BASE)/../components/obj/vxworks-6.6/krnl/lib/mips/MIPSI32R2/sfcommon
else
LD_LINK_PATH_EXTRA += -L$(WIND_BASE)/../components/obj/vxworks-6.6/krnl/lib/mips/MIPSI32R2/sfcommonle
endif
include $(TGT_DIR)/h/make/defs.bsp

CONFIG_ALL      = all

CFLAGS   += -Dvxworks ${ENDIAN_FLAGS} ${BLD_FLAG}

WRVX_COMPBASE = $(WIND_BASE)/../components
WIND_PLATFORM = vxworks-6.6
COMP_IPNET2 = ip_net2-6.6

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.


TARGET_DIR	= bcm53000
VENDOR		= Broadcom 
BOARD		= Keystone 

RELEASE		+= bootrom_uncmp.hex bootrom_uncmp.bin 

override EXTRA_DEFINE += -DBROADCOM_BSP -DIP_PORT_VXWORKS=66

ifeq "$(HOSTTYPE)" "Windows2000PC"
ifneq "$(TCL_BASE)" ""
TCL		= $(WIND_HOME)/setup/$(WIND_HOST_TYPE)/bin/tclsh.exe
else
TCL		= tclsh
endif
endif

## The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
## in config.h and Makefile. All definitions for these constants must be
## identical.

ROM_TEXT_ADRS           = bfc00000      # ROM entry address
ROM_SIZE                = 00200000      # Number of bytes of ROM space

RAM_LOW_ADRS            = 80100000      # RAM text/data address
RAM_HIGH_ADRS           = 83c00000

MACH_EXTRA       = 

BOOT_EXTRA       = 
LIB_EXTRA        = 

VMA_START        = 0x0
USR_ENTRY        = usrInit

ifdef ROBO_CHIPS
# Et robo driver
ETROBO_FLAGS = $(CFLAGS) -DMBZ -DVXWORKS
ETROBO_SRC_DIR = ../../../../drv/et
ETROBO_OBJS = et/etc_robo_spi.o
else
ETROBO_OBJS =
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
                  flashDrvLib.o flashFsLib.o flashutl.o sflash.o\
                  chipc_i2c.o i2cM41T81Clock.o\
                $(ETROBO_OBJS) 

local_clean:
	$(RM) depend.$(TARGET_DIR)
	$(RM) et/etc_robo_spi.o

## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.


include $(TGT_DIR)/h/make/rules.bsp

# Place make rule of files that outside BSP directory here.
# $(_IPCOM_IPDOMAIN_FILE_) is defined in rules.bsp and its include files.
et/%.o : $(ETROBO_SRC_DIR)/%.c | $(_IPCOM_IPDOMAIN_FILE_)
	mkdir -p et
	$(CC) -I${SDK}/include -I$(ETROBO_SRC_DIR) $(ETROBO_FLAGS) -c $< -o $@

