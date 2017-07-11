# -*- Makefile -*-
# $Id: Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$

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

CFGFLAGS += -D__DUNE_LINUX_BCM_CPU_PCIE__
CFGFLAGS += -D_SIMPLE_MEMORY_ALLOCATION_=1

export SDK

override kernel_version=2_6
platform=x86-smp_generic-$(kernel_version)

LINUX_MAKE_USER=1
include ${SDK}/make/Make.linux
