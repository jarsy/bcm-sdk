# -*- Makefile -*-
# $Id: Makefile,v 1.3 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
LOCALDIR = src/appl/diag/robo

include ${SDK}/make/Make.config

ifdef IMP_PROTECT
CFLAGS += -DIMP_SW_PROTECT
endif

#ifdef COND_LIB_BCMX_DIAG
#subdirs := bcmx
#include ${SDK}/make/Make.subdirs
#endif

lib = libdiagrobo
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
