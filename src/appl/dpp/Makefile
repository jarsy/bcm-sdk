# $Id: Makefile,v 1.6 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# 

include ${SDK}/make/Make.config

export TARGET
export BLDDIR
export LIBDIR

subdirs += sweep 
ifeq (DUNE_UI,$(findstring DUNE_UI,$(FEATURE_LIST)))
subdirs += UserInterface
endif

ifneq (,$(findstring -DBCM_PETRA_SUPPORT,$(CFGFLAGS)))
ifeq (INTR,$(findstring INTR,$(FEATURE_LIST)))
subdirs += interrupts
endif
endif


ifdef DPP_CHIPS
subdirs += FecAllocation
subdirs += FecPerformance
endif


include ${SDK}/make/Make.subdirs


