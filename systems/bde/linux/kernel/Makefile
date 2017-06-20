# -*- Makefile -*-
# $Id: Makefile,v 1.18 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for Linux kernel BDE
#
LOCALDIR = systems/bde/linux/kernel

# Make sure we build for the kernel if this is a user-mode build
ifneq ($(platform), )
override-target=linux-$(platform)
endif

include $(SDK)/make/Make.config

LIBS = $(LIBDIR)/libkern.a

BDE = linux-kernel-bde.o

ifdef ROBO_CHIPS
CFLAGS += -I$(ET_ROBO) -I${SDK}/include/shared/et
ET_ROBO = ${SDK}/systems/drv/et
endif

# need to add vpath sources
VPATH = ../shared $(ET_ROBO)

# Add the srcs to be found by vpath
LSRCS += mpool.c
ifdef ROBO_CHIPS
platformsplt = $(subst -, , ${platform})    # change hyphens to spaces
platformbase = $(word 1,${platformsplt})

ifeq ($(platformbase), keystone)
    LSRCS += etc_robo_spi.c aiutils.c
else
    ifeq ($(platformbase), keystone_le)
        LSRCS += etc_robo_spi.c aiutils.c
    else
        ifeq ($(platformbase), iproc)
             LSRCS += robo_srab.c robo_spi.c aiutils.c
        endif
    endif
endif # platformbase

endif # ROBO_CHIPS

# Add shared BDE sources
VPATH += ../../shared
LSRCS += shbde_pci.c shbde_iproc.c shbde_mdio.c
CFLAGS += -I../../shared/include

LHDRS += mpool.h
LOBJS = $(addsuffix .o, $(basename $(LSRCS)))
BOBJS = $(addprefix $(BLDDIR)/,$(LOBJS))

ifneq ($(kernel_version),2_4)
KERNEL_MODULE_DIR = kernel_module

THIS_MOD_NAME := linux-kernel-bde
MODULE = $(LIBDIR)/$(THIS_MOD_NAME).o
KMODULE = $(LIBDIR)/$(THIS_MOD_NAME).ko

build: kernel_libs $(MODULE) $(KMODULE)
else
MODULE = $(LIBDIR)/linux-kernel-bde.o

build: kernel_libs $(MODULE)
endif

$(MODULE): $(BLDDIR)/.tree kernel_libs $(BOBJS)
	mkdir -p $(@D)
	$(LD) $(MODULE_LDFLAGS) -r -d $(BOBJS) $(LIBS) -o $@
ifneq ($(kernel_version),2_4)
$(KMODULE): $(MODULE)
	rm -fr $(BLDDIR)/$(KERNEL_MODULE_DIR)
	mkdir $(BLDDIR)/$(KERNEL_MODULE_DIR)
	cp ${SDK}/make/Makefile.linux-kmodule $(BLDDIR)/$(KERNEL_MODULE_DIR)/Makefile
	MOD_NAME=$(THIS_MOD_NAME) $(MAKE) -C $(BLDDIR)/$(KERNEL_MODULE_DIR) $(THIS_MOD_NAME).ko
endif

kernel_libs:
	$(MAKE) -C $(SDK)/systems/linux/kernel/modules/shared

include $(SDK)/make/Make.depend

# Make.depend is before clean:: so that Make.depend's clean:: runs first.

clean::
	$(MAKE) -C $(SDK)/systems/linux/kernel/modules/shared $@
	$(RM) $(BOBJS) $(MODULE)
	$(RM) $(BLDDIR)/$(KERNEL_MODULE_DIR)

distclean::

ifneq ($(kernel_version),2_4)
.PHONY: build kernel_libs
endif
