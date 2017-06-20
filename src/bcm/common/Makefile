# -*- Makefile -*-
# $Id: Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM rpc driver Library
#

LOCALDIR = src/bcm/common

include ${SDK}/make/Make.config

ifneq (,$(findstring -DINCLUDE_PTP,$(CFGFLAGS)))
SUBDIR_PTP = ptp
endif

subdirs := \
	$(SUBDIR_PTP) \
	$(SUBDIR_NONE)

include ${SDK}/make/Make.subdirs

lib = libbcm_common
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
