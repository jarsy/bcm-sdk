#
# $Id: Makefile,v 1.9 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for esw subdirectory
#

LOCALDIR = src/soc/esw/tdm

include ${SDK}/make/Make.config

ifdef BCM_56850_A0
SUBDIR_TDM_CORE = core
SUBDIR_TDM_TRIDENT2P = trident2p
endif

ifdef BCM_56860_A0
SUBDIR_TDM_CORE = core
SUBDIR_TDM_TRIDENT2P = trident2p
endif

ifdef BCM_56560_A0
SUBDIR_TDM_CORE = core
SUBDIR_TDM_APACHE = apache
SUBDIR_TDM_TRIDENT2P = trident2p
endif

ifdef BCM_56960_A0
SUBDIR_TDM_CORE = core
SUBDIR_TDM_TRIDENT2P = trident2p
SUBDIR_TDM_TOMAHAWK = tomahawk
endif

ifdef BCM_56965_A0
SUBDIR_TDM_CORE = core
SUBDIR_TDM_TRIDENT2P = trident2p
SUBDIR_TDM_TOMAHAWK = tomahawk
endif

ifdef BCM_56970_A0
SUBDIR_TDM_CORE = core
SUBDIR_TDM_TRIDENT2P = trident2p
SUBDIR_TDM_TOMAHAWK = tomahawk
SUBDIR_TDM_TOMAHAWK2 = tomahawk2
endif

subdirs := $(SUBDIR_TDM_CORE) $(SUBDIR_TDM_TRIDENT2P) $(SUBDIR_TDM_TRIDENT2) $(SUBDIR_TDM_TOMAHAWK) $(SUBDIR_TDM_APACHE) $(SUBDIR_TDM_TOMAHAWK2)

include ${SDK}/make/Make.subdirs

lib = libsoc_tdm
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend


