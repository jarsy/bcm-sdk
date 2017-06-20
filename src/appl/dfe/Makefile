# $Id: Makefile,v 1.5 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# 

include ${SDK}/make/Make.config

export TARGET
export BLDDIR
export LIBDIR

ifneq (,$(findstring -DBCM_DFE_SUPPORT,$(CFGFLAGS)))
subdirs+=interrupts
endif

include ${SDK}/make/Make.subdirs
