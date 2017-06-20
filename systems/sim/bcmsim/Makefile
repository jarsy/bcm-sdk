#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# $Id: Makefile,v 1.11 Broadcom SDK $
ifdef SDK
include ${SDK}/make/Make.config
endif

ifdef BCMSIM_ROOT
bcmsbxlcm_lib=${BCMSIM_ROOT}/lib/libbcmsbxlcm_c2.a
endif

ifdef BCMSBXLCM_LIB
bcmsbxlcm_lib=${BCMSBXLCM_LIB}
endif

sdk_env_check:
ifndef SDK
	@echo "SDK environment variable not set. "
	exit 1
endif

lcm_env_check:
ifndef BCMSBXLCM_LIB
ifndef BCMSIM_ROOT
	@echo "BCMSIM_ROOT environment variable not set "
	exit 1
endif
endif

BCMSIM_DIAG_LIBS = \
	${COND_LIB_EDITLINE} \
	libdiag.$(libext) \
	${ROBO_DIAG_LIBS} \
	${ESW_DIAG_LIBS} \
	${SBX_DIAG_LIBS} \
	${COND_LIB_BCMX_DIAG} \
	${COND_LIB_TEST} \
	${COND_LIB_TCL}  \
	$(COND_LIB_SAL_APPL) \
	$(COND_LIB_SAL_APPL_PLATFORM)
    
BCM_LIBS = ${APPL_LIBS} ${BCMSIM_DIAG_LIBS} ${DRV_LIBS}
BCM_LIBS_BLD = $(addprefix ${LIBDIR}/,${BCM_LIBS})

pcid: sdk_env_check
	$(MAKE) -C ${SDK}/systems/sim

sbxlcm: lcm_env_check pcid
	${CXX} -o bcmsbxlcm_bcmsim.linux ${BLDDIR}/systems/sim/version.o \
		${BLDDIR}/systems/sim/platform_defines.o \
		${bcmsbxlcm_lib}\
		${BCM_LIBS_BLD} ${BCM_LIBS_BLD} ${BCM_LIBS_BLD} ${LDFLAGS} -lstdc++

help:
	@echo " Before doing a make, "
	@echo " Set BCMSIM_ROOT env variable to use the prebuilt LCM library (BCMSIM_ROOT/lib/libbcmsbxlcm_g2p2.a) "
	@echo " Or set BCMSBXLCM_LIB env variable to point to the LCM library you built "

all: pcid
