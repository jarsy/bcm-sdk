#
# $Id: Makefile,v 1.2 Broadcom SDK $
#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

# make -f Makefile -C $(PATH_TO_KERNEL) M=$(CURR_DIR) CROSS_COMPILE=$(CROSS_COMPILER) $(EXTRA_FLAGS) V=1 


# /tools/oss/packages/x86_64-rhel5/make/default/bin/make -C /projects/ntsw-sw/linux/wrslinux_2.0/gto/glibc_std/build/linux-2.6.21-standard CROSS_COMPILE=powerpc-wrs-linux-gnu-ppc_e500v2-glibc_std- M=/projects/NTSW_SW_USRS/eliad/cvs/sdk/systems/linux/kernel/modules/bcm-pcp-net modules


CURR_DIR= $(shell pwd)
#PATH_TO_KERNEL=/projects/ntsw-sw/linux/wrslinux_2.0/gto/glibc_std/build/linux-2.6.21-standard
PATH_TO_KERNEL=/projects/ntsw-tools/linux/wrslinux_2.0/gto/glibc_std/build/linux-2.6.21-standard
CROSS_COMPILER=/projects/ntsw-sw3/linux/wrslinux_2.0/gto/glibc_std/host-cross/powerpc-wrs-linux-gnu/x86-linux2/powerpc-wrs-linux-gnu-ppc_e500v2-glibc_std-

EXTRA_FLAGS += -I$(PATH_TO_KERNEL)/arch/ppc/
EXTRA_FLAGS += ARCH=ppc


all: modls libs bins
modls:
bins:



MAKE_CMD = make -f Makefile -C $(PATH_TO_KERNEL) M=$(CURR_DIR) \
  CROSS_COMPILE=$(CROSS_COMPILER) $(EXTRA_FLAGS) V=1\
  
  
libs:
	$(MAKE_CMD) modules

clean_all:
	$(MAKE_CMD) clean



obj-m := bcm-pcp-dma.o
#bcm_pcp_dma-objs := bcm-pcp-dma.o
