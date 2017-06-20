#
# $Id:$
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# 
# Makefile for SOC swstate subdirectory
#
LOCALDIR = src/shared/swstate

include ${SDK}/make/Make.config
#
# Take the subdirs 'access layout diagnostic' if either DPP or DNX are defined
#
subdirs = 
ifdef DPP_CHIPS  
  subdirs += access layout diagnostic
endif
#
# In the future, there may be specific subdirs for swstate of JR2
#    subdirs += ../swstate_v2/access ../swstate_v2/layout ../swstate_v2/diagnostic
#
ifdef DNX_CHIPS  
  subdirs += access layout diagnostic
endif 

include ${SDK}/make/Make.subdirs

lib = libshared_swstate

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend
