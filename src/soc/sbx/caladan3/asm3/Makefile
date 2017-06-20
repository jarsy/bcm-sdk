# $Id: Makefile,v 1.1.16.1 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#

LOCALDIR = src/soc/sbx/caladan3/asm3

SDK :=$(shell if [ -n "$$SDK" ] ; then\
        echo $$SDK;\
        else\
        cd $(dir $(lastword $(MAKEFILE_LIST))); while /usr/bin/test ! -e RELEASE ; do \
        dir=`cd ../;pwd`;                       \
        if [ "$$dir" = "/" ] ; then             \
           echo Cannot find SDK in $(lastword $(MAKEFILE_LIST)) 1>&2;           \
           exit 1;                              \
        fi ;                                    \
        cd $$dir;                               \
        done ;                                  \
        pwd;                                    \
        fi)

ifeq ($(SDK),)
$(error Please run this in a tree)
endif

export SDK

include ${SDK}/make/Make.config


ifeq ($(INSTALL_DIR),)
INSTALL_DIR=.
endif

lib = libsoc_sbx_caladan3_asm3

include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend


