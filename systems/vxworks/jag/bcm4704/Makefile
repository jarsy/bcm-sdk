
# Makefile - make rules for systems/bcm4704/bcm4704
# $Id$
# modification history
# --------------------
# --------------------
# 01f,21oct02,jmb   new TOD chip, BRINGUP flag for LED printing
# 01e,17sep02,jmb   added -DMBZ flag to TFFS build to activate XOR 3
# 01d,10sep02,jmb   TFFS build mods
# 01c,06sep02,jmb   add PCI shared interrupt handler
# 01b,10aug02,jmb   add MBZ define for xmodem build (for TOD BSP dependencies)
# 01a,10aug02,jmb   created by merging IDT and BMW Makefiles and using build flags
#                   from HNBU Makefile
#
# DESCRIPTION
# This file contains rules for building VxWorks for the BCM 4702.
#  
#  This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
#  the contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of Broadcom Corporation.
#  
##################################################################

CPU             = MIPS32
ifeq ($(VX_TARGET_BIG_ENDIAN),1)
TOOL            = sfgnu
ENDIAN_FLAGS    = -DIL_BIGENDIAN=1
else
TOOL            = sfgnule
ENDIAN_FLAGS    =
endif


TGT_DIR=$(WIND_BASE)/target
include $(TGT_DIR)/h/make/defs.bsp
include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)

CONFIG_ALL=$(SRC_DIR)/all
DRV_DIR=../../../drv

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

#Disable compiler optimization DEBUG DEBUG
#C_OPTIM_NORMAL = -O0
#CC_OPTIM_TARGET = -O0


# Add some options to CFLAGS for drivers and platform
CFLAGS   += -DBROADCOM_BSP -DVXWORKS  -DBCM4704

ifeq ($(WIND_HOST_TYPE),x86-linux)
LDFLAGS += -T $(WIND_BASE)/host/sun4-solaris2/mips-wrs-vxworks/lib/ldscripts/vxw5ebmip.x
endif
ifdef ROBODV
   CFLAGS += -DROBO
else 
   CFLAGS += -DMBZ
endif

TARGET_DIR      = bcm4704
VENDOR          = Broadcom
BOARD           = Model-1

#EXTRA_DEFINE     =-DCPU_VAR=RC32364 -DBROADCOM_BSP -DTARGETOS_vxWorks -Dvxworks 

# For BCM4704A0/A1, due to a chip erratum the define -DPATCH_INT_STUBS must
# be used.  This patches the VxWorks MIPS interrupt stubs at run-time to
# work around the erratum.  This workaround is ONLY compatible with
# VxWorks 5.4.1 libraries.  For BCM4704A2, the workaround is not needed
# and -DPATCH_INT_STUBS define should be not be used.

ifeq ($(VX_VERSION),542)
CFLAGS     += -DCPU_VAR=RC32364 -DTARGETOS_vxWorks -Dvxworks -DPATCH_INT_STUBS -DVX_VERSION=${VX_VERSION} ${ENDIAN_FLAGS}
else
CFLAGS     += -DCPU_VAR=RC32364 -DTARGETOS_vxWorks -Dvxworks -DVX_VERSION=${VX_VERSION} ${ENDIAN_FLAGS}
endif

#Use Disk-on_chip as flash: device
#This should be uncommented to use Disk-On-Chip on JAG card
#Also see $SDK/make/Makefile.vxworks-jag
#CFLAGS     += -DDOC_IS_FLASH

################################################################## 
## Only redefine make definitions below this point, or your 
## definitions will be overwritten by the makefile stubs above.
################################################################## 

################################################################## 
# The constants ROM_TEXT_ADRS, ROM_SIZE, RAM_LOW_ADRS and
# RAM_HIGH_ADRS are defined in config.h and MakeSkel.
# All definitions for these constants must be identical.
################################################################## 

ROM_TEXT_ADRS           = bfc00000      # ROM entry address
ROM_SIZE                = 00080000      # Number of bytes of ROM space
RAM_LOW_ADRS            = 80100000      # RAM text/data address
RAM_HIGH_ADRS           = 82c00000

#This macro LD_LINK_PATH is for link path
ifeq ($(VX_VERSION),55)
LD_LINK_PATH    = $(LD_LINK_PATH_EXTRA) \
		  -L$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL)
LD_LINK_PATH_EXTRA = -L$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL)$(CPU_VARIANT)
endif

# M-Systems TFFS (DiskOnChip) Driver Support Libs
TFFS_SUB_DIR = ./tffs
TFFS_SRC_DIR = $(DRV_DIR)/tffs
TFFS_OBJS = tffs/blockdev.o tffs/docsoc.o tffs/fatfilt.o \
            tffs/flflash.o tffs/flsocket.o tffs/mdocplus.o\
            tffs/diskonc.o tffs/docsys.o tffs/flbase.o \
            tffs/flioctl.o tffs/flsysvxw.o tffs/nftllite.o\
            tffs/doc2exb.o tffs/flcustom.o \
            tffs/fltl.o tffs/protectp.o\
            tffs/docbdk.o tffs/dosformt.o tffs/fldrvvxw.o\
            tffs/flsim.o tffs/inftl.o tffs/reedsol.o\
            tffs/if_bcm47xx.o

TFFS_CFLAGS = $(CFLAGS) -DMBZ 

#tffs/%.o : $(TFFS_SRC_DIR)/%.c
#	mkdir -p ./tffs
#	@ $(RM) $@
#	$(CC) -I${TFFS_SRC_DIR} $(CFLAGS) $(TFFS_CFLAGS) -c $< -o $@


#NVRAM_OBJS =
NVRAM_OBJS = nvramstubs.o bcmsrom.o

# Uncomment these next lines to build TFFS if you have the sources.
# Otherwise, it is assumed tffs/libTFFS.a already exists.
#ifeq ($(VX_VERSION),55)
#tffs/libTFFS55.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#else
#ifeq ($(VX_VERSION),542)
#tffs/libTFFS54.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#else
#tffs/libTFFS.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#endif
#endif
ifeq (, $(findstring bootrom, $(MAKECMDGOALS)))
# Et robo driver
ETROBO_FLAGS = $(CFLAGS) -DMBZ -Dvxworks
ETROBO_SRC_DIR = ../../../drv/et
ETROBO_OBJS = et/etc_robo.o
et/%.o : $(ETROBO_SRC_DIR)/%.c
	mkdir -p et
	$(CC) -I$(ETROBO_SRC_DIR) $(ETROBO_FLAGS) -c $< -o $@
endif

bsplib_clean:
	$(RM) tffs/*.o
	$(RM) et/*.o

# Add driver objects to MACH_EXTRA for building bootrom and ram image


MACH_EXTRA = srecLoad.o ns16550Sio.o \
                cacheLib.o cacheALib.o pciConfigLib.o pciIntLib.o \
                sysSerial.o et_vx.o etc.o etc47xx.o vx_osl.o hnddma.o \
                sbutils.o bcmutils.o m48t59y.o ds1743.o \
		flash29l160DrvLib.o flash28f320DrvLib.o \
		flashDrvLib.o flashFsLib.o \
                ftpXfer2.o \
                $(NVRAM_OBJS) \
                $(ETROBO_OBJS) \
                $(WRS_OBJS)

ifeq ($(VX_VERSION),55)
MACH_EXTRA += tffs/libTFFS55.a

override LIB_EXTRA	+= $(TGT_DIR)/lib/mips/MIPS32/sfcommon_rc32xxx/libarch.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libcommoncc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libdcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libdrv.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libgcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libnet.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libos.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/librpc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libusb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwdb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwind.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwindview.a
else
ifeq ($(VX_VERSION),542)
MACH_EXTRA += tffs/libTFFS54.a
else
MACH_EXTRA += tffs/libTFFS.a
endif
endif

#if_robo.o osl_vxworks.o swapi.o et_virt.o
ifdef ROBODV
	MACH_EXTRA += robotxrx.c
endif

# uncomment below for map file
#LDFLAGS += -X -N -Map $@.map

################################################################## 
## Only redefine make definitions above this point, or the expansion of
## makefile target dependencies may be incorrect.
################################################################## 

include $(TGT_DIR)/h/make/rules.bsp
include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)

HEXBIN=$(BIN_DIR)/$(OBJCPY)$(TOOLENV) -I srec -O binary
SWAP=$(PERL) ${SDK}/tools/swap.pl

bootrom_swap$(bcm_suffix).hex:
	${HEXBIN} bootrom$(bcm_suffix).hex bootrom$(bcm_suffix).bin
	${SWAP} bootrom$(bcm_suffix).bin bootrom_swap$(bcm_suffix).bin
	${BINHEX} -I binary bootrom_swap$(bcm_suffix).bin bootrom$(bcm_suffix).hex
ifeq ($(HOSTTYPE),Windows2000PC)
	${CP} bootrom$(bcm_suffix).hex ../bootrom$(bcm_suffix).hex
else
	${CP} ${SRC_DIR}/bootrom$(bcm_suffix).hex ../bootrom$(bcm_suffix).hex
endif
	rm bootrom$(bcm_suffix).bin bootrom_swap$(bcm_suffix).bin

ifeq ($(HOSTTYPE),Windows2000PC)
MY_NM		=	ccmips

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
	    -o $@ dataSegPad.o $(MACH_DEP) \
	    usrConfig_st.o version.o $(LD_LINK_PATH) -( \
	    $(LIBS) -) $(CC_LIB) symTbl.o ctdt.o $(LD_SCRIPT_RAM) -Map vxWorksSt.map
	$(LDOUT_SYMS) $@
	$(LDOUT_CONV) $@
	$(VXSIZEPROG) -v $(RAM_HIGH_ADRS) $(RAM_LOW_ADRS) vxWorks.st
	- @ $(RM) tmp.?
endif
