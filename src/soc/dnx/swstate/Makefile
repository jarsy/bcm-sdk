#
# $Id:$
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# 
# Makefile for SOC swstate subdirectory
#
LOCALDIR = src/soc/dnx/swstate

include ${SDK}/make/Make.config
#
# Take the subdirs 'access layout diagnostic' if either DPP or DNX are defined
#
#subdirs = 
#ifdef DPP_CHIPS  
#  subdirs += access layout diagnostic
#endif

include ${SDK}/make/Make.subdirs

lib = libsoc_swstate

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend
