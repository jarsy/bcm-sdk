# $Id: Makefile,v 1.26 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# 

include ${SDK}/make/Make.config

export TARGET
export BLDDIR
export LIBDIR

ifdef ROBO_CHIPS
subdirs=diag
endif

ifdef ESW_CHIPS
subdirs += diag
endif

ifdef SBX_CHIPS
subdirs += diag
endif

ifdef DPP_CHIPS
subdirs += diag
subdirs += dpp
subdirs += dcmn
endif

ifdef DFE_CHIPS
subdirs += diag
subdirs += dfe
subdirs += dcmn
endif

ifdef DNX_CHIPS
subdirs += diag
endif

ifdef EA_CHIPS
subdirs += diag
endif

ifdef COND_LIB_PIONEER
subdirs += pioneer_host
endif

ifdef COND_LIB_TEST
subdirs += test
endif

ifdef COND_LIB_TCL
subdirs += tcl8.3.3
endif

ifdef COND_LIB_ACL
subdirs += acl
endif

ifdef COND_LIB_CPUDB
subdirs += cpudb
endif

ifdef COND_LIB_CPUTRANS
subdirs += cputrans
endif

ifdef COND_LIB_DISCOVER
subdirs += discover
endif

ifdef COND_LIB_STKTASK
subdirs += stktask
endif

ifdef COND_LIB_AEDEV
subdirs += aedev
endif

ifdef COND_LIB_CINT
subdirs += cint
endif

ifdef COND_LIB_C_UNIT
subdirs += c_unit
endif

ifdef PORTMOD_STAND_ALONE
subdirs += diag
subdirs += portmod
endif

ifdef PE_SDK
subdirs += pelib
endif

ifdef PHYMOD_EPIL_LIB_PATH
subdirs += epil
endif

include ${SDK}/make/Make.subdirs
