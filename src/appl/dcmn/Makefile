# $Id: Makefile,v 1.3 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# 

include ${SDK}/make/Make.config

export TARGET
export BLDDIR
export LIBDIR

ifeq (INTR,$(findstring INTR,$(FEATURE_LIST)))
# {
subdirs+=rx_los interrupts
# }
else
# {
#
# If INTR is not included, we still need 'rx_los' for DPP (KBP).
# This only happens on CMODEL
#
ifneq (,$(findstring -DCMODEL_SERVER_MODE,$(CFGFLAGS)))
# {
subdirs += rx_los
# }
endif
# }
endif




include ${SDK}/make/Make.subdirs

