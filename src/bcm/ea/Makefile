# $Id: Makefile,v 1.6 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM5 driver Library
#

LOCALDIR = src/bcm/ea

include ${SDK}/make/Make.config

ifdef BCM_TK371X_SUPPORT
subdirs = common tk371x
endif

include ${SDK}/make/Make.subdirs

lib = libbcm_ea
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
