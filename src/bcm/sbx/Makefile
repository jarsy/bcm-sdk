# $Id: Makefile,v 1.10 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM driver Library
#

LOCALDIR = src/bcm/sbx

include ${SDK}/make/Make.config
CFGFLAGS += -DBCM_CES_SDK -DAG_GCC -DAG_STAM
ifdef BCM_QE2000_A0
SUBDIR_QE2000 = qe2000
endif

ifdef BCM_BME3200_A0
SUBDIR_BME3200 = bme3200
endif

ifdef BCM_BME3200_B0
SUBDIR_BME3200 = bme3200
endif

ifdef BCM_BM9600_A0
SUBDIR_BM9600 = bm9600
endif

ifdef BCM_BM9600_A1
SUBDIR_BM9600 = bm9600
endif

ifdef BCM_BM9600_B0
SUBDIR_BM9600 = bm9600
endif

ifdef BCM_88030_A0
SUBDIR_CALADAN3 = caladan3
endif

ifdef BCM_88230_A0
SUBDIR_SIRIUS = sirius
endif

ifdef BCM_88230_B0
SUBDIR_SIRIUS = sirius
endif

ifdef BCM_88230_C0
SUBDIR_SIRIUS = sirius
endif

subdirs := \
	common \
	$(SUBDIR_QE2000) \
	$(SUBDIR_BME3200) \
	$(SUBDIR_BM9600) \
	$(SUBDIR_FE2000) \
	$(SUBDIR_CALADAN3) \
	$(SUBDIR_SIRIUS) \
	$(SUBDIR_NONE)

include ${SDK}/make/Make.subdirs

lib = libbcm_sbx
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
