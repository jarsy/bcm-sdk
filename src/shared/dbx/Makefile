#
#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#

LOCALDIR = src/shared/dbx

include ${SDK}/make/Make.config

INCDIR += -I${SDK}/include/shared

lib = libshared_dbx

OBJS  = \
	dbx_file.o

ifeq (,$(findstring -DNO_FILEIO,$(CFGFLAGS)))
OBJS += dbx_xml.o
endif

BOBJS = $(addprefix ${BLDDIR}/, ${OBJS})

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend