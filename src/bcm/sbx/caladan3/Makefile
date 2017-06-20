
# $Id: Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for caladan3 mbcm driver Library
#

LOCALDIR = src/bcm/sbx/caladan3

include ${SDK}/make/Make.config

subdirs := $(SUBDIR_NONE)

subdirs += g3p1

subdirs += oam

include ${SDK}/make/Make.subdirs

lib = libcaladan3
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
