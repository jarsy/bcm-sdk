#
# $Id: Makefile,v 1.9 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for esw subdirectory
#

LOCALDIR = src/soc/esw

include ${SDK}/make/Make.config

ifdef BCM_56640_A0
SUBDIR_TRIUMPH3 = triumph3
endif

ifdef BCM_56850_A0
SUBDIR_TDM = tdm
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
endif

ifdef BCM_56860_A0
SUBDIR_TDM = tdm
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
SUBDIR_TRIDENT2P = trident2p
endif

ifdef BCM_56340_A0
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
endif

ifdef BCM_56450_A0
SUBDIR_KATANA2 = katana2
SUBDIR_TRIUMPH3 = triumph3
endif

ifdef BCM_53400_A0
SUBDIR_GHREYHOUND = greyhound
endif

ifdef BCM_56960_A0
SUBDIR_TDM = tdm
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
SUBDIR_TOMAHAWK = tomahawk
SUBDIR_ALPM = alpm
endif

ifdef BCM_56965_A0
SUBDIR_TDM = tdm
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
SUBDIR_TOMAHAWK = tomahawk
SUBDIR_ALPM = alpm
endif

ifdef BCM_56160_A0
SUBDIR_GHREYHOUND = greyhound
SUBDIR_HURRICANE3 = hurricane3
endif

ifdef BCM_53570_A0
SUBDIR_GHREYHOUND = greyhound
SUBDIR_HURRICANE3 = hurricane3
SUBDIR_GREYHOUND2 = greyhound2
endif

ifdef BCM_56260_A0
SUBDIR_SABER2 = saber2
endif

ifdef BCM_56970_A0
SUBDIR_TDM = tdm
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
SUBDIR_TOMAHAWK = tomahawk
SUBDIR_TOMAHAWK2 = tomahawk2
SUBDIR_ALPM = alpm
SUBDIR_FLEXPORT = flexport
endif

ifdef BCM_56270_A0
SUBDIR_METROLITE = metrolite
endif

ifdef BCM_56560_A0
SUBDIR_TDM = tdm
SUBDIR_APACHE = apache
SUBDIR_TRIUMPH3 = triumph3
SUBDIR_TRIDENT2 = trident2
SUBDIR_TRIDENT2P = trident2p
SUBDIR_ALPM = alpm
endif

subdirs := $(SUBDIR_TRIUMPH3) $(SUBDIR_TRIDENT2) $(SUBDIR_KATANA2) $(SUBDIR_TOMAHAWK) $(SUBDIR_GHREYHOUND) $(SUBDIR_TRIDENT2P) $(SUBDIR_TDM) $(SUBDIR_ALPM) $(SUBDIR_SABER2) $(SUBDIR_HURRICANE3) $(SUBDIR_APACHE) $(SUBDIR_METROLITE) $(SUBDIR_TOMAHAWK2) $(SUBDIR_FLEXPORT) $(SUBDIR_GREYHOUND2)

include ${SDK}/make/Make.subdirs

lib = libsoc_esw
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend


