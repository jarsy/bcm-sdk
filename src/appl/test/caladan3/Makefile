# Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# $Id: Makefile,v 1.3 2012/03/02 20:23:20 balas Exp $
# Makefile for Caladan3 tests Library
#

#LOCALDIR = src/appl/test/caladan3 

include ${SDK}/make/Make.config

lib = libtest_sbx_caladan3

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend

ifdef BCM_88030_A0
# for now do a manual make within ucode as required
#subdirs += ucode
include ${SDK}/make/Make.subdirs
endif
