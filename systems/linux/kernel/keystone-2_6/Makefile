# -*- Makefile -*-
# $Id: Makefile,v 1.6 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

#
# This make job requires the following environment variables to be set:
#
# SDK                - path to StrataXGS SDK root directory
#
# Optionally the following environment variables can be set to
# override the default build server configuration:
#
# MIPS_TOOLS_DIR     - path to build tools (if not in PATH already)
# MIPS_CROSS_COMPILE - cross compile tools prefix
# LINUX_INCLUDE      - path to Linux kernel include directory
#

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

override kernel_version=2_6
platform=keystone-$(kernel_version)

KS_BUILD=1
export KS_BUILD

# Select build platform
#BUILD_PLATFORM=WR_LINUX
#BUILD_PLATFORM=ELDK

#Select the LINUX KERNEL VERSION
#KERN_VER = 2.6.27.18

#Select the ELDK version
#ELDK_VERSION=4.1

# Windriver linux version
#WRS_LINUX_VERSION=2.0
#WRS_LINUX_VERSION=3.0

# Keystone Highmem support (>128M)
#KS_HIGHMEM=1

export KS_HIGHMEM
export KERN_VER
export ELDK_VERSION
export BUILD_PLATFORM
export WRS_LINUX_VERSION


include ${SDK}/make/Make.linux

