# Makefile - makefile for Wind River wrSbc8548 BSP
# $Id$
# Copyright (c) 2006-2007 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
# 
# modification history
# --------------------
# 01b,28mar07,b_m  update CPU & CPU_VARIANT.
# 01a,30jan06,kds  Modify from cds8548/Makefile/01a
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# Wind River SBC8548 reference board.
#
# INCLUDES
#     makeTarget
#*/

CPU             = PPC32
ifeq (,$(TOOL))
TOOL            = sfgnu
#TOOL            = sfdiab
endif
CPU_VARIANT     = _ppc85XX

# Support for 36 bit physical addresses in the MMU can be added for
# vxWorks 6.2 and later with the following CPU_VARIANT.

#CPU_VARIANT 	= _ppc85XX_e500v2

TGT_DIR = $(WIND_BASE)/target

EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps

include $(TGT_DIR)/h/make/defs.bsp

CONFIG_ALL      = all

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.


TARGET_DIR	= mpc8548
VENDOR		= Broadcom 
BOARD		= GTO 

RELEASE		+= bootrom_uncmp.hex bootrom_uncmp.bin 

EXTRA_DEFINE    = -DBROADCOM_BSP

## The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
## in config.h and Makefile. All definitions for these constants must be
## identical.

ROM_TEXT_ADRS    = fff00100 # ROM entry address
ROM_SIZE         = 00100000 # number of bytes of ROM space
ROM_WARM_ADRS    = fff00108 # ROM warm entry address 
ROM_LINK_ADRS	 = fff00000

RAM_LOW_ADRS     = 00100000 # RAM text/data address
RAM_HIGH_ADRS    = 08000000 # RAM text/data address

MACH_EXTRA       = 

BOOT_EXTRA       = 
LIB_EXTRA        = 

VMA_START        = 0x0
USR_ENTRY        = usrInit
## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

MACH_EXTRA = i2cM41T81Clock.o i2cPCF8547Ioport.o flash29GL1GDrvLib.o \
             flashFsLib.o flashDrvLib.o srecLoad.o
local_clean:
	$(RM) *.o
	$(RM) *.a
	$(RM) depend.$(TARGET_DIR)

include $(TGT_DIR)/h/make/rules.bsp

