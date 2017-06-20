# $Id: Makefile,v 1.3 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
LOCALDIR = systems/bde/vxworks

include ${SDK}/make/Make.config

ifdef ROBO_CHIPS
CFLAGS += -Dvxworks -DBCMTYPES_H
endif

# Add shared BDE sources
VPATH += ../shared
LSRCS += shbde_pci.c shbde_iproc.c shbde_mdio.c
CFLAGS += -I../shared/include

lib = libvxbde

include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
