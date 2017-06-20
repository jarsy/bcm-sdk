# $Id: Makefile,v 1.15 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#

VX_VERSIONS :=55 66

SDK :=$(shell if [ -n "$$SDK" ] ; then\
	echo $$SDK;\
	else\
	cd $(dir $(lastword $(MAKEFILE_LIST))); while /usr/bin/test ! -e RELEASE ; do \
	dir=`cd ../;pwd`;			\
	if [ "$$dir" = "/" ] ; then		\
	   echo Cannot find SDK in $(lastword $(MAKEFILE_LIST)) 1>&2;		\
	   exit 1;				\
	fi ;					\
	cd $$dir;				\
	done ;					\
	pwd;					\
	fi)

ifeq ($(SDK),)					
$(error Please run this in a tree)		
endif 				

export SDK
ifeq ($(strip ${MERGE}),1)
ALL_CHIPS=1
export ALL_CHIPS
endif

KS_BUILD=1
export KS_BUILD

LOCALDIR=systems/vxworks/keystone
LOCAL_CLEAN=local_clean

override-target=vxworks-keystone

LOCAL_D_FILE=1
export LOCAL_D_FILE

export VX_TARGET_BIG_ENDIAN ?= 1

#VX_VERSION=66 
export VX_VERSION

#VX_BSP_DIR = vxworks-6.6
VX_BSP_DIR = vxworks-${VXWORKS_VERSION}

# To separate compilation output
ifeq ($(VX_TARGET_BIG_ENDIAN),1)
bcm_suffix =.be
else
bcm_suffix =.le
endif

include ${SDK}/make/Make.config
include ${SDK}/make/Make.vxworks

#export ROBODVT = 1

#
# This optional override builds a binary that loads the symbol table
# over the network instead of having it compiled in.
#
#override VX_TARGET = vxWorks

#
# Point to the VxWorks BSP source directory
#

# To separate compilation output
ifeq ($(VX_TARGET_BIG_ENDIAN),1)
SRC_DIR =${SDK}/systems/vxworks/keystone/bcm53000/${VX_BSP_DIR}
else 
SRC_DIR =${SDK}/systems/vxworks/keystone/bcm53000/${VX_BSP_DIR}.le
endif

IMAGE_DEPENDENCY = ImageBanner

#
# Define rule to build ${VX_BINARY}
#

# stripppc.exe is broken in Cygnus
$(DEST_DIR)/${VX_BINARY}: ${IMAGE_DEPENDENCY} bde bcm_libraries driver_libraries  \
		${BLDDIR}/.tree ${BLDDIR}/socdiag.o ${BLDDIR}/version.o\
		${BLDDIR}/platform_defines.o ${BLDDIR}/hal.o ${SRC_DIR}/${VX_TARGET}
	mkdir -p $(@D) 
	${CP} ${SRC_DIR}/${VX_TARGET} $@ 
	${CP} ${SRC_DIR}/${VX_TARGET} $@.sym 
ifneq ($(OS),Windows_Cygnus)
	${STRIP} $@ 
	${STRIPFIX} ${SRC_DIR}/${VX_TARGET} $@ 56 5 
endif

.PHONY : .FORCE

${SRC_DIR}/${VX_TARGET} : .FORCE
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
	VX_VERSION="${VX_VERSION}"\
		WIND_BASE_${VX_VERSION}="${WIND_BASE_${VX_VERSION}}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}

# \todo Remove below once tested
${VX_BINARY} vxWorks.st vxWorks.st.mips: \
		${IMAGE_DEPENDENCY} bde bcm_libraries driver_libraries \
		${BLDDIR}/.tree ${BLDDIR}/socdiag.o ${BLDDIR}/version.o\
		${BLDDIR}/platform_defines.o ${BLDDIR}/hal.o
	${RM} ${BLDDIRWITHDRIVE}/hal.p 
	${RM} ${BLDDIRWITHDRIVE}/socdiag.p 
	${MAKE} -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
	VX_VERSION="${VX_VERSION}"\
		WIND_BASE_${VX_VERSION}="${WIND_BASE_${VX_VERSION}}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}.sym
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}
	${STRIP} ${VX_BINARY}
	${STRIPFIX} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY} 56 5
	${MKBEEP}

bootrom_uncmp:
	${MAKE} -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
	MAKEFLAGS="${MAKEFLAGS}" bootrom_uncmp
	${CP} ${SRC_DIR}/bootrom_uncmp bootrom_uncmp

vxWorks.res_rom:
	${MAKE} -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
	MAKEFLAGS="${MAKEFLAGS}" vxWorks.res_rom
	${CP} ${SRC_DIR}/vxWorks.res_rom vxWorks.res_rom

ImageBanner:
	echo "****  Building Keystone image (VxWorks ${VXWORKS_VERSION}) (Make Version ${MAKE_VERSION})  ****"
