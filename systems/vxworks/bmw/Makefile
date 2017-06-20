# $Id: Makefile,v 1.16 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for VxWorks PPC BSP with BCM driver library
# VxWorks (sp) configuration variables
#

VX_VERSIONS :=55 62

LOCALDIR=systems/vxworks/bmw
LOCAL_CLEAN=local_clean

override-target=vxworks-bmw

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

include ${SDK}/make/Make.config
include ${SDK}/make/Make.vxworks

#
# This optional override builds a binary that loads the symbol table
# over the network instead of having it compiled in.
#
#override VX_TARGET = vxWorks

#
# Point to the VxWorks BSP source directory
#
SRC_DIR=${SDK}/systems/vxworks/bmw/mpc824x

#
# Define rule to build ${VX_BINARY}
#

IMAGE_DEPENDENCY = ImageBanner

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
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}

# \todo Remove below once tested
${VX_BINARY} vxWorks.st vxWorks.st.ppc: \
		bde bcm_libraries driver_libraries \
		${BLDDIR}/.tree ${BLDDIR}/socdiag.o ${BLDDIR}/version.o\
		${BLDDIR}/platform_defines.o ${BLDDIR}/hal.o
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}.sym
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}
  ifneq ($(OS),Windows_Cygnus)
  # stripppc.exe is broken
	${STRIP} ${VX_BINARY}
	${STRIPFIX} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY} 56 5
  endif
	${MKBEEP}

bootrom_uncmp:
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
	MAKEFLAGS="${MAKEFLAGS}" bootrom_uncmp
	${CP} ${SRC_DIR}/bootrom_uncmp bootrom_uncmp

vxWorks.res_rom:
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
	MAKEFLAGS="${MAKEFLAGS}" vxWorks.res_rom
	${CP} ${SRC_DIR}/vxWorks.res_rom vxWorks.res_rom


ImageBanner:
	echo "****  Building BMW image (VxWorks ${VXWORKS_VERSION})  ****"
