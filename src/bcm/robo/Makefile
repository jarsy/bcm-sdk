# $Id: Makefile,v 1.8 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM5 robo driver Library
#

LOCALDIR = src/bcm/robo

ifeq (${STP_BY_EAP_BLK},1)
CFLAGS += -DSTP_BY_EAP_BLK
ifeq (${BCM_5398_A0},1)
$(error The feature STP_BY_EAP_BLK didn't support on BCM5398 currently!)
endif
ifeq (${BCM_5348_A0},1)
$(error The feature STP_BY_EAP_BLK didn't support on BCM5348 currently!)
endif

endif

include ${SDK}/make/Make.config

include ${SDK}/make/Make.subdirs

lib = libbcm_robo
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
