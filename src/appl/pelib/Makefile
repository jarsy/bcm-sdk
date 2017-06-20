# $Id: Makefile,v 1.3 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for Broadcom PE-LIB Mode Tests
#

LOCALDIR = src/appl/pelib

CFGFLAGS += -DSOURCE_CHECK_LIBAPPL
CFGFLAGS += -DPEMODE_BCM_BUILD
#CFGFLAGS += -DMDIO_DRY_RUN
CFGFLAGS += -I$(SDK)/src/appl/pelib/include

subdirs+=src
include ${SDK}/make/Make.subdirs

include ${SDK}/make/Make.config

lib = libpelib
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
