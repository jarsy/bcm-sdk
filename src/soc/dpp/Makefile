#
# $Id: Makefile,v 1.4 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for dpp subdirectory
#

LOCALDIR = src/soc/dpp

include ${SDK}/make/Make.config

subdirs := PPC          \
	SAND/Management \
	SAND/SAND_FM    \
	SAND/Utils      \
	PPD             \
	TMC             \


ifdef BCM_88650_A0
subdirs += ARAD ARAD/ARAD_PP DRC PORT ARAD/NIF
endif


ifdef BCM_88470_A0
subdirs += QAX
endif

ifdef BCM_88270_A0
subdirs += QUX
endif

ifdef BCM_88675_A0
subdirs += JER DRC 
endif

ifdef BCM_88680_A0
subdirs += JERP
endif

include ${SDK}/make/Make.subdirs

lib = libsoc_dpp

include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend

