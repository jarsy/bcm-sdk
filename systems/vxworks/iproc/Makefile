# $Id: Makefile,v 1.20 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for VxWorks PPC BSP with BCM driver library
# VxWorks configuration variables
#
# Makefile include architecture:
#    sdk/systems/vxworks/iproc/Makefile, 
#        - include sdk/make/Make.config      (chip definition and SDK compile)
#            - include sdk/make/Make.local     (SDK compile include definition)
#            - include sdk/make/Make.tools     (SYSNAME = Linux, define unix commands, using Pearl to generate some files like bcm2c, version)
#            - include sdk/make/Makefile.vxworks-iproc   (Vxworks path definition, config definition, define vxworks commands)
#                - include sdk/make/Makefile.vxworks-common
#            - include sdk/make/Make.boards    (BCM board level definition)
#            - include sdk/make/Make.targets   (define path/files for KERNFLAGS)
#        - include sdk/make/Make.vxworks     (SDK module compile routines like bde, bcm_libraries, driver_libraries)
#    <SDK compile> is in $(DEST_DIR)/${VX_BINARY}: ${IMAGE_DEPENDENCY} bde bcm_libraries driver_libraries.....
#          in bde module, it will change to sdk/systems/bde/vxworks and execute Makefile.
#          in bcm_libraries module, it will change to sdk and execute Makefile.
#          in driver libraries module, it will change to sdk/systems/drv and execute Makefile.
#        Usually the architecture in those Makefile(s) are :
#              - include sdk/make/Make.config (it will include other files like above)
#              - define "subdirs" to tell which sub-directories under current location should be compiled
#              - include sdk/make/Make.subdirs 
#                    it will jump to those sub-directories and run the Makefile.... (recursive)
#              - include sdk/make/Make.depend
#    <Vxworks compile> is in $(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}"......
#          it will change directory to $(SRC_DIR) = sdk/systems/vxworks/iproc/vxworks6x/bcm56450 and run Makefile.
#    Combine SDK and Vxworks after "${VX_BINARY} vxWorks.st vxWorks.st.arm".........

VX_VERSIONS :=68

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

override-target=vxworks-iproc

LOCALDIR = systems/vxworks/iproc
LOCAL_CLEAN = bsplist_clean

# default is Big endian
#export VX_TARGET_BIG_ENDIAN ?= 0
#ifeq ($(VX_TARGET_BIG_ENDIAN),1)
#export bldroot_suffix=be
#else
#export bldroot_suffix=le
#endif

ifeq (${VX_HR2},1)
export VX_BSP_DIR = bcm56150
else
export VX_BSP_DIR = bcm56450
VX_KT2 := 1
endif

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
SRC_DIR=${SDK}/systems/vxworks/iproc/vxworks6x/$(VX_BSP_DIR)

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


# Following section will change directory to SRC_DIR first,
#  (SRC_DIR = systems/vxworks/iproc/vxworks6x/bcm56450) 
# then execute the Makefile.

.PHONY : .FORCE

${SRC_DIR}/${VX_TARGET} : .FORCE
	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
		VX_VERSION="${VX_VERSION}"\
		WIND_BASE="${WIND_BASE}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}

# \todo Remove below once tested
${VX_BINARY} vxWorks.st vxWorks.st.arm: \
		${IMAGE_DEPENDENCY} bde bcm_libraries driver_libraries \
		${BLDDIR}/.tree ${BLDDIR}/socdiag.o ${BLDDIR}/version.o\
		${BLDDIR}/platform_defines.o ${BLDDIR}/hal.o
#	$(MAKE) -C ${SRC_DIR} WIND_BASE="${WIND_BASE}" \
		VX_VERSION="${VX_VERSION}"\
		WIND_BASE="${WIND_BASE}" \
		MAKEFLAGS="${MAKEFLAGS}" ${KERNFLAGS} ${VX_TARGET}
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}.sym
	${CP} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY}
	${STRIP} ${VX_BINARY}
	${STRIPFIX} ${SRC_DIR}/${VX_TARGET} ${VX_BINARY} 56 5
	${MKBEEP}

ImageBanner:
	echo "****	Building IPROC($(VX_BSP_DIR)) image (VxWorks ${VXWORKS_VERSION}) (Make Version ${MAKE_VERSION})  ****"
