#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# $Id: Makefile,v 1.4 Broadcom SDK $
LOCALDIR = src/appl/diag/dfe

include ${SDK}/make/Make.config

lib = libdiag_dfe

OBJS= fabric.o

ifeq (gto-2_6, $(platform))
OBJS  += utils_fe1600_card.o
endif

ifeq (wrx-3_7, $(platform))
OBJS  += utils_fe1600_card.o
endif

BOBJS = $(addprefix ${BLDDIR}/, ${OBJS})

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend
