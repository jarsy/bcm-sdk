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

EXTRA_INCLUDE += -I$(WIND_BASE)/target/h/nfs
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipnet2/src
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipnet2/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipnet2/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/windml-5.0/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/dcom-2.3/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/opc-3.1/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/webcli-4.6/target/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/webservices-1.4/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/osconfig/vxworks/src
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/windml-5.0/osconfig/vxworks/src
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/osconfig/vxworks/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/windml-5.0/osconfig/vxworks/h
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipappl/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipappl/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipcci/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipcci/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipcom/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipcom/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipcom/port/vxworks/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipcom/port/vxworks/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcpc6/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcpc6/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcpr/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcpr/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcps/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcps/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcps6/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipdhcps6/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipeap/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipeap/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipfirewall/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipfirewall/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/iphwcrypto/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/iphwcrypto/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipike/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipike/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipipsec2/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipipsec2/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmcp/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmcp/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmip6/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmip6/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmip6mn/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmip6mn/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmipfa/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmipfa/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmipha/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmipha/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmipmn/config 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmipmn/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmpls/config 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipmpls/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipppp/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipppp/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipradius/config 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipradius/include 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/iprip/config 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/iprip/include 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipripng/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipripng/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipsntp/config 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipsntp/include 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipssh/config
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/ipssh/include
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/iptcp/config 
EXTRA_INCLUDE += -I$(WIND_BASE)/../components/ip_net2-6.5/iptcp/include 
EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps

LD_LINK_PATH_EXTRA += -L$(WIND_BASE)/../components/obj/vxworks-6.5/krnl/lib/ppc/PPC32/sfcommon
include $(TGT_DIR)/h/make/defs.bsp

CONFIG_ALL      = all

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.


TARGET_DIR	= mpc8548
VENDOR		= Broadcom 
BOARD		= GTO 

RELEASE		+= bootrom_uncmp.hex bootrom_uncmp.bin 

EXTRA_DEFINE    = -DBROADCOM_BSP -DIP_PORT_VXWORKS=65

## The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
## in config.h and Makefile. All definitions for these constants must be
## identical.

ROM_TEXT_ADRS    = fff00100 # ROM entry address
ROM_SIZE         = 00100000 # number of bytes of ROM space
ROM_WARM_ADRS    = fff00108 # ROM warm entry address 
ROM_LINK_ADRS	 = fff00000

RAM_LOW_ADRS     = 00100000 # RAM text/data address
RAM_HIGH_ADRS    = 09000000 # RAM text/data address

MACH_EXTRA       = 

BOOT_EXTRA       = 
LIB_EXTRA        = 

VMA_START        = 0x0
USR_ENTRY        = usrInit
## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

MACH_EXTRA = i2cM41T81Clock.o i2cPCF8547Ioport.o flash29GL1GDrvLib.o \
             flashFsLib.o flashDrvLib.o srecLoad.o

ADDED_MODULES += onelib.o

CFLAGS += -mlongcall

LIBS	=  $(LIB_EXTRA)

LD_SCRIPT_RAM = -defsym wrs_kernel_rom_size=0x0$(ROM_SIZE) -T link.DOTBOOTRAM
            
local_clean:
	$(RM) *.o
	$(RM) *.a
	$(RM) depend.$(TARGET_DIR)

include $(TGT_DIR)/h/make/rules.bsp


usrappl:
	- @ $(RM) usrappl.o 
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) -o version.o \
	      $(CONFIG_ALL)/version.c
	$(LD_PARTIAL) -o usrappl.o dataSegPad.o sysALib.o sysLib.o  $(MACH_EXTRA) \
	      usrConfig_st.o $(ROMFS_IMAGE) version.o \
		  $(LD_PARTIAL_START_GROUP) $(LIB_EXTRA) $(LD_PARTIAL_END_GROUP)
	

symdep: usrappl
	- @ $(RM) depSymTbl.c
	$(TCL) $(SDK)/systems/vxworks/makeDepSymTbl.tcl $(TOOLENV) usrappl.o depSymTbl.c


onelib.o: symdep
	- @ $(RM) onelib.o 
	- @ $(RM) usrappl.o 
	$(COMPILE_SYMTBL) depSymTbl.c
	$(LD_PARTIAL)  -o onelib.o depSymTbl.o \
		  $(LD_PARTIAL_START_GROUP) $(LD_LINK_PATH) $(VX_OS_LIBS) $(LD_PARTIAL_END_GROUP)
		     

 
