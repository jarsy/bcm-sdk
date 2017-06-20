# -*- Makefile -*-
# $Id: Makefile,v 1.1 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for SOC driver Library
#
LOCALDIR = src/soc/robo/tbx

include ${SDK}/make/Make.config

ifdef BCM_53280_A0
SUBDIR_THUNDERBOLT = thunderbolt
endif

ifdef BCM_53280_B0
SUBDIR_THUNDERBOLT = thunderbolt
endif

ifdef BCM_53600_A0
SUBDIR_VOYAGER = voyager
endif

subdirs := \
	${SUBDIR_THUNDERBOLT}\
        ${SUBDIR_VOYAGER}

include ${SDK}/make/Make.subdirs

lib = libsocrobo_tbx
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
