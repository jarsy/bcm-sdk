# $Id: Makefile,v 1.21.20.1 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

LOCALDIR = src/soc/sbx

include ${SDK}/make/Make.config

ifdef BCM_QE2000_A0
subdirs += fabric
subdirs += qe2k
subdirs += qe2kep
subdirs += g2eplib
endif

ifdef BCM_88230_A0
subdirs += fabric
endif 

ifdef BCM_88230_B0
subdirs += fabric
endif 

ifdef BCM_88230_C0
subdirs += fabric
endif 

ifdef BCM_88030_A0
subdirs += caladan3 
subdirs += g3p1 
subdirs += t3p1 
endif

ifdef BCM_88030_A1
subdirs += caladan3 
subdirs += g3p1 
subdirs += t3p1 
endif

ifdef BCM_88030_B0
subdirs += caladan3 
subdirs += g3p1 
subdirs += t3p1 
endif

include ${SDK}/make/Make.subdirs

lib = libsoc_sbx
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend

