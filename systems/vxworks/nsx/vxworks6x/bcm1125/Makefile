# Makefile - make rules for bsp/config/bcm1250
# $Id: Makefile,v 1.7 2011/07/21 16:14:48 yshtil Exp $
# Copyright 2002 Wind River Systems, Inc.
#
# This file has been developed or significantly modified by the
# MIPS Center of Excellence Dedicated Engineering Staff.
# This notice is as per the MIPS Center of Excellence Master Partner
# Agreement, do not remove this notice without checking first with
# WR/Platforms MIPS Center of Excellence engineering management.
#
# modification history
# --------------------
# 01a,10dec01,agf  remove prj & elf rules; redesign private lib
# 01a,15nov01,agf  created.
#
# DESCRIPTION
# This file contains rules for building VxWorks for the
# BCM1250.
#
# INCLUDES
#     makeTarget
#*/

CPU              = MIPS64
TOOL             = gnu

EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps

TGT_DIR=$(WIND_BASE)/target

include $(TGT_DIR)/h/make/defs.bsp
#include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

CONFIG_ALL=all

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

TARGET_DIR       = bcm1250_cpu0
VENDOR           = Broadcom
BOARD            = Broadcom NSX

#CFLAGS += -DUSE_SIBYTE_INTR
ifeq ($(WIND_HOST_TYPE),x86-linux)
LDFLAGS += -T $(WIND_BASE)/host/sun4-solaris2/mips-wrs-vxworks/lib/ldscripts/vxw5ebmip.x
endif

#
# The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
# RAM_HIGH_ADRS are defined in config.h and MakeSkel.
# All definitions for these constants must be identical.
#

ROM_TEXT_ADRS    	= bfc00000	# ROM entry address
ROM_SIZE         	= 00100000	# 1MB ROM space
RAM_LOW_ADRS     	= 80100000	# RAM text/data address
RAM_HIGH_ADRS    	= 84000000	# RAM text/data address

RELEASE		= vxWorks vxWorks.st

CFLAGS     +=-DCPU_VAR=RC32364 -DTARGETOS_vxWorks -Dvxworks

ifeq ($(INCLUDE_SBPROF), yes)
CFLAGS += -DPROF_DRIVER -DBCM1250_SB1_PROFILING 
#CFLAGS += -I../../../../tools/src/sbutils/ \
#          -I../../../../tools/src/sbprof/src/comm \
#	  -DPROF_DRIVER -DBCM1250_SB1_PROFILING 
endif

#CFLAGS += -DBATCH=8 -DNBMACS=2 -D_PREFETCH_ -DIPV4 -ffixed-s7 -O3

MACH_EXTRA	= \
        mips3_ldsd.o        hs_rdwr.o          \
        bcm1250DuartSio.o  \
        bcm1250IntLib.o     bcm1250JTAGSio.o   \
        bcm1250Timer.o      bcm1250PciLib.o    \
        sysBcm1250MacEnd.o  bcm1250MacEnd.o endLib.o \
	flash28f128DrvLib.o flashDrvLib.o \
	flashFsLib.o        srecLoad.o \
	m41t81clock.o
#        sbeth.o tree.o trace.o dma.o pkt.o rand.o types.o rx.o 
#        sysBcm1250MacEnd.o  sysDec21x40End.o
BOOT_EXTRA	= \
        mips3_ldsd.o        hs_rdwr.o          \
        bcm1250DramInit.o

LD_LINK_PATH    = $(LD_LINK_PATH_EXTRA) \
		  -L$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL)
LD_LINK_PATH_EXTRA = -L$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL)$(CPU_VARIANT)
ADDED_CFLAGS += -DCPU=MIPS64 -DCPU_FAMILY=MIPS
CFLAGS += -DCPU=MIPS64 -DCPU_FAMILY=MIPS

ifeq ($(INCLUDE_SBPROF), yes)
MACH_EXTRA += bcm1250_sbprof.o
endif

HW_PRIVATE_OBJS_COMMON=	bcm1250L2Cache.o
HW_PRIVATE_OBJS_MACH=	bcm1250MacEnd.o bcm1250Dec21x40End.o
HW_PRIVATE_OBJS_BOOT=
HW_PRIVATE_OBJS=	$(HW_PRIVATE_OBJS_COMMON) $(HW_PRIVATE_OBJS_MACH) \
			$(HW_PRIVATE_OBJS_BOOT)

$(LIB_EXTRA):
	$(RM) $@
	$(MAKE) $(HW_PRIVATE_OBJS)
	$(AR) crus $@ $(HW_PRIVATE_OBJS)

bsplib_clean:
	echo "bsplib_clean"

override LIB_EXTRA	+= $(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libcommoncc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libdcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libgcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libnet.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libos.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/librpc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libusb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/liberfLib.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libxbd.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwdb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libhwdb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libaim.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwind.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libc_wr.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libfp.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libdevice.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwindview.a \
	$(TGT_DIR)/lib/mips/MIPS64/common_bcm125x/libarch.a

## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp
include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)

ifeq ($(HOSTTYPE),Windows2000PC)
MY_NM           =       nmmips

usrConfig_st.o : depend.$(BSP_NAME) $(USRCONFIG) usrConfig.o
	- @ $(RM) $@
	$(CP) .\all\usrConfig.c usrConfig_st.c
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) -DSTANDALONE usrConfig_st.c
	- @ $(RM) usrConfig_st.c


vxWorks.st : depend.$(BSP_NAME) usrConfig_st.o dataSegPad.o \
		$(MACH_DEP) $(LDDEPS) $(patsubst -l%,lib%.a,$(LIBS)) \
		$(CC_LIB)
	- @ $(RM) $@
	- @ $(RM) symTbl.c
	- @ $(RM) symTbl.o
	- @ $(RM) tmp.o
	- @ $(RM) tmp.2
	- @ $(RM) version.o
	- @ $(RM) ctdt.c ctdt.o
	$(CC) $(OPTION_OBJECT_ONLY) $(CFLAGS) -o version.o $(CONFIG_ALL)/version.c
	$(LD_PARTIAL) -o tmp.o dataSegPad.o $(MACH_DEP) \
	    usrConfig_st.o version.o $(LD_PARTIAL_START_GROUP) $(LD_LINK_PATH) \
	    $(LIBS) $(LD_PARTIAL_END_GROUP) $(CC_LIB)
	$(MY_NM) tmp.o | $(MUNCH) >ctdt.c
	$(MAKE) CC_COMPILER="$(OPTION_DOLLAR_SYMBOLS)" ctdt.o
	$(CP) tmp.o tmp.2
	$(LDOUT_SYMS) tmp.o
	$(MKSYMTBL) tmp.o symTbl.c
	$(COMPILE_SYMTBL) symTbl.c
	$(LD) $(LDFLAGS) $(LD_ENTRY_OPT) $(SYS_ENTRY) $(LD_LOW_FLAGS) \
	    -o $@ tmp.2 symTbl.o ctdt.o $(LD_SCRIPT_RAM)
	$(LDOUT_SYMS) $@
	$(LDOUT_CONV) $@
	$(VXSIZEPROG) -v $(RAM_HIGH_ADRS) $(RAM_LOW_ADRS) vxWorks.st
	- @ $(RM) tmp.?
endif
