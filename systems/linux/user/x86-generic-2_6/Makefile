# -*- Makefile -*-
# $Id: Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

################################################################################
# Find SDK root folder.
# For example, if SDK 6.4.4 is placed at:
# `~/MRV/OPX/BCM_SDK/6.4.4/systems/linux/user/x86-generic-2_6`
# then `SDK` variable will be:
# `~/MRV/OPX/BCM_SDK/6.4.4/`
# because the file named `RELEASE` is in `~/MRV/OPX/BCM_SDK/6.4.4/`.
################################################################################
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
platform=x86-generic-$(kernel_version)

LINUX_MAKE_USER=1
include ${SDK}/make/Make.linux
