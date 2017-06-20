#
# $Id: Makefile,v 1.1 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for phy/chip/orca/eagle_orca_src subdirectory
#
LOCALDIR = src/soc/phy/chip/orca/eagle_orca_src

include ${SDK}/make/Make.config

CFGFLAGS+=  -UCOMPILER_HAS_DOUBLE

LSRCS    =\
            eagle_phy_1_functions.c eagle_phy_1_dependencies.c\

OBJS := $(LSRCS:.c=.o)

lib = libsoc_orca
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
