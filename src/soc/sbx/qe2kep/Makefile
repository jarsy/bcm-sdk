# $Id: Makefile,v 1.7 Broadcom SDK $
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

LOCALDIR = src/soc/sbx/qe2kep

include ${SDK}/make/Make.config

QE2KEP_INC  = ${SDK}/include/soc/sbx/qe2kep

#GENERATE_C_FILES=1
#GEN_INCS = $(QE2KEP_INC)

#export GENERATE_C_FILES
#export GEN_INCS

QE2KEP_ZFS      = $(wildcard *.zf)
QE2KEP_ZFCS     = $(addsuffix .c, $(basename ${QE2KEP_ZFS})) \
		$(addsuffix Console.c, $(basename ${QE2KEP_ZFS}))
QE2KEP_ZFHXS    = $(addprefix ${QE2KEP_INC}/, \
			$(addsuffix .hx, $(basename ${QE2KEP_ZFS})) \
		 $(addsuffix Console.hx, $(basename ${QE2KEP_ZFS})))
QE2KEP_ZFOS     = $(addsuffix .o, $(basename ${QE2KEP_ZFCS}))

BOBJS    += $(addprefix ${BLDDIR}/, ${QE2KEP_ZFOS})

lib = libsoc_sbx_qe2kep
include ${SDK}/make/Make.lib

#clean::
#	$Q$(RM) $(QE2KEP_ZFHXS) $(QE2KEP_ZFCS)

.SECONDARY:: ${QE2KEP_ZFCS} ${QE2KEP_ZFHXS}

include ${SDK}/make/Make.depend

