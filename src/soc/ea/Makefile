# $Id: Makefile,v 1.8 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for SOC EA driver Library
#
LOCALDIR = src/soc/ea

subdirs = 
include ${SDK}/make/Make.config

ifdef BCM_TK371X_SUPPORT
subdirs += tk371x  
endif

include ${SDK}/make/Make.subdirs

