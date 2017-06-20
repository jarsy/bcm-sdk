# $Id: Makefile,v 1.14 Broadcom SDK $
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

LOCALDIR = src/soc/sbx/fabric

include ${SDK}/make/Make.config

SBX_FABRIC_INC  = ${SDK}/include/soc/sbx/fabric

#GENERATE_C_FILES=1
#GEN_INCS = $(SBX_FABRIC_INC)

#export GENERATE_C_FILES
#export GEN_INCS


ZFS	= $(wildcard *.zf)
ZFCS	= $(addsuffix .c, $(basename ${ZFS})) \
		$(addsuffix Console.c, $(basename ${ZFS}))
ZFHXS	= $(addprefix ${SBX_FABRIC_INC}/, $(addsuffix .hx, $(basename ${ZFS}))\
		$(addsuffix Console.hx, $(basename ${ZFS})))
ZFOS	= $(addsuffix .o, $(basename ${ZFS}))

# not every .zf file generates a %Console.c file 
# by adding them to the object files list they will be
# linked into the library
ZFOS += \
sbZfFabBm9600BwAllocCfgBaseEntryConsole.o \
sbZfFabBm9600BwAllocRateEntryConsole.o \
sbZfFabBm9600BwFetchDataEntryConsole.o \
sbZfFabBm9600BwFetchSumEntryConsole.o \
sbZfFabBm9600BwFetchValidEntryConsole.o \
sbZfFabBm9600BwR0BagEntryConsole.o \
sbZfFabBm9600BwR0BwpEntryConsole.o \
sbZfFabBm9600BwR0WdtEntryConsole.o \
sbZfFabBm9600BwR1BagEntryConsole.o \
sbZfFabBm9600BwR1Wct0AEntryConsole.o \
sbZfFabBm9600BwR1Wct0BEntryConsole.o \
sbZfFabBm9600BwR1Wct1AEntryConsole.o \
sbZfFabBm9600BwR1Wct1BEntryConsole.o \
sbZfFabBm9600BwR1Wct2AEntryConsole.o \
sbZfFabBm9600BwR1Wct2BEntryConsole.o \
sbZfFabBm9600BwR1WstEntryConsole.o \
sbZfFabBm9600BwWredCfgBaseEntryConsole.o \
sbZfFabBm9600BwWredDropNPart1EntryConsole.o \
sbZfFabBm9600BwWredDropNPart2EntryConsole.o \
sbZfFabBm9600FoLinkStateTableEntryConsole.o \
sbZfFabBm9600FoTestInfoConsole.o \
sbZfFabBm9600InaEsetPriEntryConsole.o \
sbZfFabBm9600InaHi1Selected_0EntryConsole.o \
sbZfFabBm9600InaHi1Selected_1EntryConsole.o \
sbZfFabBm9600InaHi2Selected_0EntryConsole.o \
sbZfFabBm9600InaHi2Selected_1EntryConsole.o \
sbZfFabBm9600InaHi3Selected_0EntryConsole.o \
sbZfFabBm9600InaHi3Selected_1EntryConsole.o \
sbZfFabBm9600InaHi4Selected_0EntryConsole.o \
sbZfFabBm9600InaHi4Selected_1EntryConsole.o \
sbZfFabBm9600InaNmPriorityUpdateConsole.o \
sbZfFabBm9600InaPortPriEntryConsole.o \
sbZfFabBm9600InaRandomNumGenEntryConsole.o \
sbZfFabBm9600InaSysportMapEntryConsole.o \
sbZfFabBm9600LinkFailureInfoConsole.o \
sbZfFabBm9600NmEgressRankerEntryConsole.o \
sbZfFabBm9600NmEmtEntryConsole.o \
sbZfFabBm9600NmEmt_0EntryConsole.o \
sbZfFabBm9600NmEmt_0_1EntryConsole.o \
sbZfFabBm9600NmEmt_1EntryConsole.o \
sbZfFabBm9600NmEmtdebugbank0EntryConsole.o \
sbZfFabBm9600NmEmtdebugbank1EntryConsole.o \
sbZfFabBm9600NmIngressRankerEntryConsole.o \
sbZfFabBm9600NmPortsetInfoEntryConsole.o \
sbZfFabBm9600NmPortsetLinkEntryConsole.o \
sbZfFabBm9600NmRandomNumGenEntryConsole.o \
sbZfFabBm9600NmSysportArrayEntryConsole.o \
sbZfFabBm9600XbXcfgRemapEntryConsole.o \
sbZfFabBm9600NmFullStatusEntryConsole.o


BOBJS    += $(addprefix ${BLDDIR}/, ${ZFOS})

lib = lib_sbx_fabric

include $(SDK)/make/Make.lib

#clean::
#	$Q$(RM) $(ZFHXS) $(ZFCS) $(wildcard *.cx) $(wildcard *.hx)

.SECONDARY:: ${ZFCS} ${ZFHXS}

include ${SDK}/make/Make.depend
