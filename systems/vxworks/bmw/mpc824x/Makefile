# Makefie - makefile for bmw
# $Id$
# modification history
# --------------------
# 01l,01oct02,jmb   Build all TOD objects
# 01k,13jun02,jmb   Get rid of unused flash objects
# 01j,27may02,jmb   Simplify build flags for 5701 driver.  Build at -O0, even
#                   though -O2 is probably OK.
# 01i,11may02,jmb   Move BCM network driver sources out of BSP; get
#                   network driver cflags out of global cflags; add
#                   local_clean to delete 570x drivers objects
# 01h,30apr02,jmb   don't need sysCache.c
# 01g,14July98,mo   change memory address constants to correspond to config.h
# 01f,19June98,mo   clean up comments and set target names
# 01e,02June98,mo   Use if_dc.obj from WindRiver 
# 01d,27March98,mo  Change RAM_LOW_ADRS to 4000 (must be on 4k
#                   boundry.   
# 01c,09Mar98,mo    Change the def of BOOTINIT and LDFLAGS
# 01b,02mar98,mo&my port Vxworks to yellowknife
# 01a,02Mar98,mo    from ultra
#
#
# DESCRIPTION
# This file contains rules for building VxWorks for the template.
#
# INCLUDES
#     makeTarget
#*/

CPU              = PPC603
TOOL             = gnu

TGT_DIR = $(WIND_BASE)/target

ifeq ($(VX_VERSION),62)
EXTRA_INCLUDE += -I$(WIND_BASE)/target/config/comps
endif

include $(TGT_DIR)/h/make/defs.bsp
ifeq ($(VX_VERSION),62)
include $(TGT_DIR)/h/tool/$(TOOL)/make.$(CPU)$(TOOL)
else
include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)
endif

ifeq ($(HOSTTYPE),Windows_Cygnus)
include $(TGT_DIR)/h/make/defs.unix
MKSYMTBL = $(ENV_BIN)makeSymTbl $(VX_CPU_FAMILY)
ROMSIZEPROG = $(ENV_BIN)romsize $(VX_CPU_FAMILY)
VXSIZEPROG = $(ENV_BIN)vxsize $(VX_CPU_FAMILY)
else
include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)
endif

CONFIG_ALL=${SRCDIR}/all
DRV_DIR=../../../drv


## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.
#
#  BRINGUP flag causes LEDs or serial to be written before interrupts enabled
#
ifeq ($(VX_VERSION),55)
ADDED_CFLAGS   += -DBROADCOM_BSP -DBRINGUP -DVX_VERSION=55
else
ifeq ($(VX_VERSION),62)
ADDED_CFLAGS   += -DBROADCOM_BSP -DBRINGUP -DVX_VERSION=62
else
ADDED_CFLAGS   += -DBROADCOM_BSP -DBRINGUP
endif
endif

TARGET_DIR      = mpc824x
VENDOR          = Broadcom
BOARD           = Model-1

#
# The constants ROM_TEXT_ADRS, ROM_SIZE, and RAM_HIGH_ADRS are defined
# in config.h and Makefile.
# All definitions for these constants must be identical.
# Specify values in hexadecimal, without a leading 0x.
#



ROM_TEXT_ADRS   = ff000100 # ROM cold entry address
ROM_WARM_ADRS   = ff000104 # ROM warm entry address
ROM_SIZE        = 00080000 # number of bytes of ROM space (512K)
RAM_LOW_ADRS    = 00010000 # RAM text/data address for vxWorks
RAM_HIGH_ADRS   = 02e00000 # RAM text/data address for bootrom
LOCAL_MEM_SIZE  = 04000000 # 64 meg memory

#This macro HEX_FLAGS must be set to:
#    -a 100 for  ROM resident targets
#    -a 0   for all other targets
ifeq ($(VX_VERSION),55)
HEX_FLAGS       =
USR_ENTRY       = usrInit
else
ifeq ($(VX_VERSION),62)
HEX_FLAGS       =
USR_ENTRY       = usrEntry
else
HEX_FLAGS       = -a 0
USR_ENTRY       = usrInit
endif
endif
ifeq ($(VX_VERSION),55)
LD_LINK_PATH    = $(LD_LINK_PATH_EXTRA) \
		  -L$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL)
endif

# Broadcom BCM570x Gigabit Ethernet Driver

BCM570X_SRC_DIR = $(DRV_DIR)/bcm570x

BCM570x_OBJS = bcm570x/bcm570xEnd.o bcm570x/tigon3.o bcm570x/autoneg.o \
               bcm570x/5701rls.o

BCM570X_FLAGS = -DINCLUDE_5701_AX_FIX=1 -DVXWORKS -DDBG=0 # -DRNG_DBG

bcm570x/%.o : $(BCM570X_SRC_DIR)/%.c
	$(CC) -I$(BCM570X_SRC_DIR) $(CFLAGS) $(BCM570X_FLAGS) -O0 -c $< -o $@

# et mux Ethernet Driver
# Uncomment the following for RCPU mode
ETH_DRV_SRC_DIR = $(DRV_DIR)/eth
ETH_DRV_OBJS = eth/eth_drv.o eth/mux_drv.o
ETH_DRV_FLAGS = -DVXWORKS -DDBG=0 -DBMW -DET_DRV_TXRX_DEBUG
eth/%.o : $(ETH_DRV_SRC_DIR)/%.c
	mkdir -p ./eth
	@ $(RM) $@
	$(CC) -I$(ETH_DRV_SRC_DIR) $(CFLAGS) $(ETH_DRV_FLAGS) -O0 -c $< -o $@

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
            tffs/if_mpc8245.o

ifeq ($(VX_VERSION),55)
TFFS_CFLAGS = -I$(WIND_BASE)/target/h -I. -I./all -g -ansi -pedantic -nostdlib -fno-for-scope  -Wall -DCPU=PPC603 -DVX_VERSION=55 -fno-builtin -DBROADCOM_BSP -DVXWORKS -DBIG_ENDIAN_HOST=1
else
ifeq ($(VX_VERSION),62)
TFFS_CFLAGS = -I$(WIND_BASE)/target/h -I. -I./all -I$(WIND_BASE)/target/h/wrn/coreip -g -ansi -pedantic -nostdlib -fno-for-scope  -Wall -DCPU=PPC603 -DVX_VERSION=62 -fno-builtin -DBROADCOM_BSP -DVXWORKS -DBIG_ENDIAN_HOST=1
else
TFFS_CFLAGS = -I$(WIND_BASE)/target/h -I. -I./all -g -ansi -pedantic -nostdinc -fno-for-scope  -Wall -DCPU=PPC603 -DVX_VERSION=54 -fno-builtin -DBROADCOM_BSP -DVXWORKS -DBIG_ENDIAN_HOST=1
endif
endif

tffs/%.o : $(TFFS_SRC_DIR)/%.c
	mkdir -p ./tffs
	@ $(RM) $@
	$(CC) -I${TFFS_SRC_DIR} $(TFFS_CFLAGS) -c $< -o $@

# Uncomment these next lines to build TFFS if you have the sources.
# Otherwise, it is assumed tffs/libTFFS.a already exists.
#ifeq ($(VX_VERSION),55)
#tffs/libTFFS55.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#else
#ifeq ($(VX_VERSION),54)
#tffs/libTFFS54.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#else
#ifeq ($(VX_VERSION),62)
#tffs/libTFFS62.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#else
#tffs/libTFFS.a: $(TFFS_OBJS)
#	$(AR) crus $@ $(TFFS_OBJS)
#endif
#endif
#endif


bsplib_clean:
	@ $(RM) bcm570x/*.o
	@ $(RM) tffs/*.o

MACH_EXTRA = \
	epic.o ds1743.o m48t59y.o \
	sysSerial.o \
        flashLib.o \
    flashFsLib.o \
	ftpXfer2.o srecLoad.o pciSupport.o\
	$(BCM570x_OBJS)

ifeq ($(VX_VERSION),55)
MACH_EXTRA += tffs/libTFFS55.a

override LIB_EXTRA      += \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libarch.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libcommoncc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libdcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libdrv.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libgcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libnet.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libos.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/librpc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwdb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/$(CPU)/$(TOOL_COMMON_DIR)/libwind.a
else
ifeq ($(VX_VERSION),54)
MACH_EXTRA += tffs/libTFFS54.a
else
ifeq ($(VX_VERSION),62)

MACH_EXTRA += tffs/libTFFS62.a
override VX_OS_LIBS    = -lnet -larch -lhwdb -lcplus -lgnucplus -laim -lar521x -lc_wr -lcci -lcommoncc -ldcc -ldebug -ldevice -ldrv -ldsi -lerfLib -lfp -lgcc -los -lppp -lrpc -ltipc -lusb -lusb2 -lvnode -lvxmp -lwdb -lwind -lwindview -lxbd  

override LIB_EXTRA      += \
	${SRCDIR}/tffs/libTFFS62.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libcommoncc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libdcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libdrv.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libgcc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/liberfLib.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libnet.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libos.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libaim.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libxbd.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libc_wr.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libwdb.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/librpc.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/$(TOOL_COMMON_DIR)/libwind.a \
	$(TGT_DIR)/lib/$(VX_CPU_FAMILY)/PPC32/common_ppc603/libarch.a
else
MACH_EXTRA += tffs/libTFFS.a
endif
endif
endif

BOOTINIT        = bootInit.c

# "make clean" does not remove mousse.map
#LDFLAGS = -X -N -Map mousse.map 
LDFLAGS = -X -N

local_clean :
	$(RM) $(BCM570x_OBJS)

## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

include $(TGT_DIR)/h/make/rules.bsp

ifeq ($(OS),Windows_Cygnus)
include $(TGT_DIR)/h/make/rules.unix
else
include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)
endif

ifeq ($(HOSTTYPE),Windows2000PC)

MY_NM=nmppc

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
