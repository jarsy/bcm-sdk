# $Id: Makefile,v 1.23 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for SOC driver Library
#
LOCALDIR = src/soc

CFGFLAGS += -DSOURCE_CHECK_LIBSOC

include ${SDK}/make/Make.config

subdirs = phy common shared

ifdef ESW_CHIPS
subdirs += mcm
else
ifdef SBX_CHIPS
ifdef BCM_88230_A0
subdirs += mcm
endif
ifdef BCM_88230_B0
subdirs += mcm
endif
ifdef BCM_88230_C0
subdirs += mcm
endif
ifdef BCM_88030_A0
subdirs += mcm
endif
endif
endif

ifdef ESW_CHIPS
subdirs += esw
endif

ifdef SBX_CHIPS
subdirs += sbx
endif

ifdef KBP_DEVICE
subdirs += kbp
endif

#
# The 'dnx' directory is reserved for JR2 family (JR2 and up)
# The 'dnxf' directory is reserved for Ramon family (Ramon and up)
# 
ifdef DNX_CHIPS
subdirs += dnx
subdirs += mcm
subdirs += dcmn
subdirs += dnxc
endif

ifdef DNXF_CHIPS
subdirs += dnxf
endif

ifdef DPP_CHIPS
subdirs += dpp
subdirs += mcm
subdirs += dcmn
subdirs += hwstate
endif

ifdef DFE_CHIPS
subdirs += dfe
subdirs += mcm
subdirs += dcmn
subdirs += hwstate
endif

ifdef PORTMOD_SUPPORT
ifdef PORTMOD_STAND_ALONE
subdirs += mcm
endif
subdirs+= portmod
endif

ifdef ROBO_CHIPS
subdirs += robo
endif

ifdef EA_CHIPS
subdirs += ea
endif

ifdef COND_LIB_SOC_I2C
subdirs += i2c
endif

ifdef BCM_56440_A0
ifdef COND_LIB_CES
subdirs += ces
endif
endif

ifdef FEATURE_FCMAP
subdirs += phy/fcmap/src
endif

subdirs += phy/chip/koi/merlin_koi_src
subdirs += phy/chip/orca/eagle_orca_src

ifdef IMACSEC_HOME
subdirs += ${IMACSEC_HOME}
endif

include ${SDK}/make/Make.subdirs

lib = libsoc
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
