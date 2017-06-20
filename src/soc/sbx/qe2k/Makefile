# $Id: Makefile,v 1.8 Broadcom SDK $
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

.PHONY: zframes
LOCALDIR = src/soc/sbx/qe2k

include ${SDK}/make/Make.config

QE2K_INC  = ${SDK}/include/soc/sbx/qe2k

#GENERATE_C_FILES=1
#GEN_INCS = $(QE2K_INC)

#export GENERATE_C_FILES
#export GEN_INCS


ZFS      = $(wildcard *.zf)
HFS      = $(wildcard *.hx)
CXS      = $(wildcard *.cx)

QE2K_ZFS      = $(wildcard *.zf)
QE2K_ZFCS     = $(addsuffix .c, $(basename ${QE2K_ZFS})) \
		$(addsuffix Console.c, $(basename ${QE2K_ZFS}))
QE2K_ZFHXS    = $(addprefix ${QE2K_INC}/, $(addsuffix .hx, $(basename ${QE2K_ZFS})) \
			 $(addsuffix Console.hx, $(basename ${QE2K_ZFS})))
# Note diff from gu2: QE2K_ZFOS     = $(addsuffix .o, $(basename ${QE2K_ZFS}))
QE2K_ZFOS     = $(addsuffix .o, $(basename ${QE2K_ZFCS}))

MMAPHX   = ${QE2K_INC}/sbG2FeMemMap.hx

BOBJS    += $(addprefix ${BLDDIR}/, ${QE2K_ZFOS})

lib = libsoc_sbx_qe2k

include ${SDK}/make/Make.lib

MMAP_GEN = $(PERL) ${SDK}/tools/GenMemMap.pl

${QE2K_INC}/%.hx: %.mmap
ifdef QUIET
	@${ECHO} Converting mmap $(notdir $@)
endif
	$Q$(MMAP_GEN) $^ $@

#clean::
#	$Q$(RM) $(QE2K_ZFHXS) $(QE2K_ZFCS)

.SECONDARY:: ${QE2K_ZFCS} ${QE2K_ZFHXS}

include ${SDK}/make/Make.depend

