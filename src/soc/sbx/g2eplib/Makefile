# $Id: Makefile,v 1.9 Broadcom SDK $
#
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

export SDK

LOCALDIR = src/soc/sbx/g2eplib

include ${SDK}/make/Make.config

G2_INC  = ${SDK}/include/soc/sbx/g2eplib

#GENERATE_C_FILES=1
#GEN_INCS = $(G2_INC)

#export GENERATE_C_FILES
#export GEN_INCS

ZFS      = $(wildcard *.zf)
ZFCS     = $(addsuffix .c, $(basename ${ZFS})) \
	 	$(addsuffix Console.c, $(basename ${ZFS}))
ZFHXS    = $(addprefix ${G2_INC}/, $(addsuffix .hx, $(basename ${ZFS})) \
			 $(addsuffix Console.hx, $(basename ${ZFS})))
ZFOS     = $(addsuffix .o, $(basename ${ZFS}))

BOBJS    += $(addprefix ${BLDDIR}/, ${ZFOS})

lib = libsoc_g2eplib_sbx

include ${SDK}/make/Make.lib

#clean::
#	$Q$(RM) $(ZFHXS) $(ZFCS)

.SECONDARY:: ${ZFCS} ${ZFHXS} ${MMAPHX}

include ${SDK}/make/Make.depend
