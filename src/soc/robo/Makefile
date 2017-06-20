# -*- Makefile -*-
# $Id: Makefile,v 1.27 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for SOC driver Library
#
LOCALDIR = src/soc/robo

ifeq (${IMP_PROTECT},1)
CFLAGS += -DIMP_SW_PROTECT
endif

ifeq (${STP_BY_EAP_BLK},1)
CFLAGS += -DSTP_BY_EAP_BLK
endif

include ${SDK}/make/Make.config

ifdef BCM_53242_A0
SUBDIR_HARRIER = harrier
endif

ifdef BCM_53262_A0
SUBDIR_HARRIER = harrier
endif

ifdef BCM_53115_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53118_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53280_A0
SUBDIR_TBX = tbx
endif

ifdef BCM_53280_B0
SUBDIR_TBX = tbx
endif

ifdef BCM_53600_A0
SUBDIR_TBX = tbx
endif

ifdef BCM_53101_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53125_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53128_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53134_A0
SUBDIR_GEX = gex
endif

ifdef BCM_89500_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53010_A0
SUBDIR_GEX = gex
endif

ifdef BCM_5389_A0
SUBDIR_GEX = gex
endif

ifdef BCM_53020_A0
SUBDIR_GEX = gex
endif

ifdef BCM_5396_A0
SUBDIR_GEX = gex
endif

subdirs := \
        common \
        ${SUBDIR_GEX} \
        ${SUBDIR_HARRIER} \
        ${SUBDIR_TBX} \
	mcm

CFLAGS += -DBCM47XX_CHOPS -DDMA

include ${SDK}/make/Make.subdirs

lib = libsocrobo
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
