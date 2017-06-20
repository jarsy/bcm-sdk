# $Id: Makefile,v 1.17 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for BCM5 driver Library
#

LOCALDIR = src/bcm

CFGFLAGS += -DSOURCE_CHECK_LIBBCM

include ${SDK}/make/Make.config

subdirs = common
ifneq (,$(findstring -DBCM_ESW_SUPPORT,$(CFGFLAGS)))
subdirs+=esw
endif
ifneq (,$(findstring -DBCM_SHADOW_SUPPORT,$(CFGFLAGS)))
subdirs+=shadow
endif
ifneq (,$(findstring -DBCM_RPC_SUPPORT,$(CFGFLAGS)))
subdirs+=rpc
subdirs+=compat
endif
ifneq (,$(findstring -DBCM_ROBO_SUPPORT,$(CFGFLAGS)))
subdirs+=robo
endif
ifneq (,$(findstring -DBCM_SBX_SUPPORT,$(CFGFLAGS)))
subdirs+=sbx
endif
ifneq (,$(findstring -DBCM_PETRA_SUPPORT,$(CFGFLAGS)))
subdirs+=dpp
endif
ifneq (,$(findstring -DBCM_DFE_SUPPORT,$(CFGFLAGS)))
subdirs+=dfe
endif
ifneq (,$(findstring -DBCM_DNX_SUPPORT,$(CFGFLAGS)))
subdirs+=dnx/legacy
subdirs+=dnx/vlan
subdirs+=dnx/init
subdirs+=dnx/l2
subdirs+=dnx/lif
subdirs+=dnx/port
subdirs+=dnx/stg
subdirs+=dnx/rx
subdirs+=dnx/tx
subdirs+=dnx/algo
subdirs+=dnx/mpls
subdirs+=dnx/stk
subdirs+=dnx/switch
subdirs+=dnx/qos
subdirs+=dnx/l3
subdirs+=dnx/mirror
subdirs+=dnx/crps
subdirs+=dnx/vswitch
endif
ifneq (,$(findstring -DBCM_DNXF_SUPPORT,$(CFGFLAGS)))
subdirs+=dnxf
endif
ifneq (,$(findstring -DBCM_EA_SUPPORT,$(CFGFLAGS)))
subdirs+=ea
endif

include ${SDK}/make/Make.subdirs

lib = libbcm
include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend
