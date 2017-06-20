# $Id: $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

# Makefile - Standalone makefile for BCM570x driver
#
# modification history
# --------------------
# 01a,11jun02,jmb   Written
#
#
# DESCRIPTION
#
#    This is a standalone makefile for building the BCM570x network driver.
#    (It is here for illustrative purposes only.  For Strataswitch builds, the
#    driver is built in the BSP directory.)
#
#    Here's how to build for a PPC8245
#
#        make -f Makefile CPU=PPC603 TOOL=gnu
#
#    The results of the build will be a partially linked object module
#    called $(CPU)$(TOOL)/bcm570xdrv.o
#
#    Here's an example MIPS build
#
#        make -f Makefile CPU=RX32364 TOOL=sfgnu
#
#    N.B:  The standalone MIPS build for the RC32364 is currently broken because the
#          driver requires a header file in the idtrp334 BSP.  This needs to be
#          fixed at some point -- drivers shouldn't require any BSP headers.
#

BCM570X_OBJ_DIR = $(CPU)$(TOOL)

exe : $(BCM570X_OBJ_DIR) $(BCM570X_OBJ_DIR)/bcm570xdrv.o

TGT_DIR = $(WIND_BASE)/target

include $(TGT_DIR)/h/make/defs.bsp
include $(TGT_DIR)/h/make/make.$(CPU)$(TOOL)

ifeq ($(HOSTTYPE),Windows_Cygnus)
include $(TGT_DIR)/h/make/defs.unix
else
include $(TGT_DIR)/h/make/defs.$(WIND_HOST_TYPE)
endif

## Only redefine make definitions below this point, or your definitions will
## be overwritten by the makefile stubs above.

ADDED_CFLAGS   += -DBROADCOM_BSP

# Broadcom BCM570x Gigabit Ethernet Driver

BCM570X_OBJS = $(BCM570X_OBJ_DIR)/bcm570xEnd.o $(BCM570X_OBJ_DIR)/tigon3.o \
               $(BCM570X_OBJ_DIR)/autoneg.o $(BCM570X_OBJ_DIR)/5701rls.o

BCM570X_FLAGS = -DINCLUDE_5701_AX_FIX=1 -DVXWORKS -DDBG=0 

ifeq ($(CPU),RC32364)
BCM570X_FLAGS += -DBIG_ENDIAN_PCI=1
endif

$(BCM570X_OBJ_DIR):
	-mkdir $(BCM570X_OBJ_DIR)

$(BCM570X_OBJ_DIR)/bcm570xdrv.o : $(BCM570X_OBJS)
	$(LD_PARTIAL) -o $@ $(BCM570X_OBJS)

$(BCM570X_OBJ_DIR)/%.o : %.c
	$(CC) -I. $(CFLAGS) $(BCM570X_FLAGS) -O0 -c $< -o $@

clean :
	$(RM) $(BCM570X_OBJ_DIR)/*.o


## Only redefine make definitions above this point, or the expansion of 
## makefile target dependencies may be incorrect.

ifeq ($(OS),Windows_Cygnus)
include $(TGT_DIR)/h/make/rules.unix
else
include $(TGT_DIR)/h/make/rules.$(WIND_HOST_TYPE)
endif
