#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# $Id: Makefile,v 1.11 Broadcom SDK $
LOCALDIR = src/appl/diag/sand

include ${SDK}/make/Make.config

CFGFLAGS += -std=gnu99 -D_GNU_SOURCE

lib = libdiag_sand

OBJS  = \
	diag_sand_dsig.o \
	diag_sand_signals.o \
	diag_sand_mem.o \
	diag_sand_reg.o \
	diag_sand_prt.o \
	diag_sand_access.o \
	diag_sand_framework.o \
	diag_sand_resource.o \
	diag_sand_utils.o 

BOBJS = $(addprefix ${BLDDIR}/, ${OBJS})

include ${SDK}/make/Make.subdirs
include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend
