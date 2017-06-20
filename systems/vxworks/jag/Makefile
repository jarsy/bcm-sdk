# $Id: Makefile,v 1.15 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for VxWorks PPC BSP with BCM driver library
# VxWorks configuration variables
#
# To Make robodv image, define ROBODV in the shell.  

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

override-target=vxworks-jag

LOCALDIR = systems/vxworks/jag
LOCAL_CLEAN = bsplib_clean
# COND_LIB_BSP = tffs/libTFFS.a

# default is Big endian
export VX_TARGET_BIG_ENDIAN ?= 1
#ifeq ($(VX_TARGET_BIG_ENDIAN),1)
#export bldroot_suffix=be
#else
#export bldroot_suffix=le
#endif

#
#
#VX_VERSION=542
#VX_VERSION=24

VX_VERSIONS :=55

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
SRC_DIR=${SDK}/systems/jag

#
# Define rule to build ${VX_BINARY}
#

IMAGE_DEPENDENCY = ImageBanner

# stripppc.exe is broken in Cygnus
$(DEST_DIR)/${VX_BINARY}: ${IMAGE_DEPENDENCY} bde bcm_libraries driver_libraries  \
		${BLDDIR}/.tree ${BLDDIR}/socdiag.o ${BLDDIR}/version.o\
		${BLDDIR}/platform_defines.o ${BLDDIR}/hal.o ${SRC_DIR}/${VX_TARGET}
	mkdir -p $(@D) ;\
	${CP} ${SRC_DIR}/${VX_TARGET} $@
	${CP} ${SRC_DIR}/${VX_TARGET} $@.sym
ifneq ($(OS),Windows_Cygnus)
	${STRIP} $@ ;\
	${STRIPFIX} ${SRC_DIR}/${VX_TARGET} $@ 88 5
endif

.PHONY : .FORCE

${SRC_DIR}/${VX_TARGET} : .FORCE
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
		VX_VERSION="${VX_VERSION}"\
		WIND_BASE_542="${WIND_BASE_542}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}

# \todo Remove below once tested
${VX_BINARY} vxWorks.st vxWorks.st.mips: \
		${IMAGE_DEPENDENCY} bde bcm_libraries driver_libraries \
		${BLDDIR}/.tree ${BLDDIR}/socdiag.o ${BLDDIR}/version.o\
		${BLDDIR}/platform_defines.o ${BLDDIR}/hal.o
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
		VX_VERSION="${VX_VERSION}"\
		WIND_BASE_542="${WIND_BASE_542}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}.sym
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}
	${STRIP} ${VX_BINARY}
	${STRIPFIX} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY} 88 5
	${MKBEEP}

ImageBanner:
ifdef ROBODV
	echo "****	Building ROBODV image  	  ****"
else
	echo "****	Building bcm4704(JAG) image  	  ****"
endif
