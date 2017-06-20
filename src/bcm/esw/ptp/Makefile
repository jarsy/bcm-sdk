# $Id: Makefile,v 1.3 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM driver Library
#

LOCALDIR = src/bcm/esw/ptp

include ${SDK}/make/Make.config

ifneq (,$(findstring -DBCM_KSCPU_SUPPORT,$(CFGFLAGS)))
CFLAGS += -DPTP_KEYSTONE_STACK
endif
#CFLAGS += -DPTP_RX_CALLBACK_DEBUG

lib = libptp_esw

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend

