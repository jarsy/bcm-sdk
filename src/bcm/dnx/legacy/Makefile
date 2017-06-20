# $Id: Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM driver Library
#

LOCALDIR = src/bcm/dnx/legacy

include ${SDK}/make/Make.config

include ${SDK}/make/Make.subdirs

lib = libbcm_dnxlegacy
#
# Extend C language standard to C99 (plus some GNU goodies)
#
CFGFLAGS += -std=gnu99

include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
