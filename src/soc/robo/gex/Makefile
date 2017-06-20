# -*- Makefile -*-
# $Id: Makefile,v 1.5 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for SOC driver Library
#
LOCALDIR = src/soc/robo/gex

include ${SDK}/make/Make.config

ifdef BCM_53115_A0
SUBDIR_VULCAN = vulcan
endif

ifdef BCM_53118_A0
SUBDIR_BLACKBIRD = blackbird
endif

ifdef BCM_53101_A0
SUBDIR_LOTUS = lotus
endif

ifdef BCM_53125_A0
SUBDIR_STARFIGHTER = starfighter
endif

ifdef BCM_53134_A0
SUBDIR_STARFIGHTER3 = starfighter3
endif

ifdef BCM_53128_A0
SUBDIR_BLACKBIRD2 = blackbird2
endif

ifdef BCM_89500_A0
SUBDIR_POLAR = polar
endif

ifdef BCM_53010_A0
SUBDIR_NORTHSTAR = northstar
endif

ifdef BCM_5389_A0
SUBDIR_DINO8 = dino8
endif

ifdef BCM_53020_A0
SUBDIR_NORTHSTARPLUS = northstarplus
endif

ifdef BCM_5396_A0
SUBDIR_DINO16 = dino16
endif

subdirs := \
        ${SUBDIR_VULCAN} \
        ${SUBDIR_BLACKBIRD} \
        ${SUBDIR_LOTUS} \
        ${SUBDIR_STARFIGHTER} \
        ${SUBDIR_BLACKBIRD2} \
        ${SUBDIR_POLAR} \
        ${SUBDIR_NORTHSTAR} \
        ${SUBDIR_DINO8} \
        ${SUBDIR_NORTHSTARPLUS} \
        ${SUBDIR_DINO16} \
        ${SUBDIR_STARFIGHTER3}

include ${SDK}/make/Make.subdirs 

lib = libsocrobo_gex
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
