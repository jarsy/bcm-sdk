#
#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#

LOCALDIR = src/shared/utilex

include ${SDK}/make/Make.config

INCDIR += -I${SDK}/include/shared

lib = libshared_utilex
#
# Extend C language standard to C99 (plus some GNU goodies)
#
CFGFLAGS += -std=gnu99

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend