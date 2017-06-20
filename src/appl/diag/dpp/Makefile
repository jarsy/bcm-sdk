#
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
# $Id: Makefile,v 1.11 Broadcom SDK $
LOCALDIR = src/appl/diag/dpp

include ${SDK}/make/Make.config

INCDIR += -I${SDK}/include/soc/dpp


CFLAGS += -DCINT_CONFIG_INCLUDE_SDK_SAL=1
CFLAGS += -DCINT_CONFIG_INCLUDE_STUBS=0

lib = libdiag_dpp


OBJS  = \
	l2.o \
	l3.o \
	vlan.o \
	utils_gen_err_ssf.o \
	txrx.o \
	dpp.o \
	sw_state_dump.o \
	fabric.o \
	stg.o \
	test_oam.o \
	test_bfd.o

ifeq (KBP,$(findstring KBP,$(FEATURE_LIST)))
ifneq (,$(findstring KBP_12K, $(KBP_DEVICE)))
OBJS += kbp.o
endif
endif

ifeq (KBP,$(findstring KBP,$(FEATURE_LIST)))
ifeq (KBP_ALG,$(findstring KBP_ALG, $(KBP_DEVICE)))
OBJS += kbp.o
endif
endif

ifeq (KNET,$(findstring KNET,$(FEATURE_LIST)))
OBJS += knetctrl.o
endif

ifeq (DUNE_UI,$(findstring DUNE_UI,$(FEATURE_LIST)))
OBJS += utils_char_queue_ssf.o \
	utils_tid_task.o \
	utils_string.o 	\
	utils_d_os_ssf.o
endif

ifeq (gto-2_6, $(platform))
OBJS  += \
        utils_arad_card.o
endif

ifeq (wrx-3_7, $(platform))
OBJS  += \
        utils_arad_card.o
endif


ifeq (vxworks-negev,$(TARGETTYPE))
OBJS  += \
	bsp_drv_flash28f.o \
	bsp_drv_flash28f_utils.o \
	bsp_info_access.o \
	dffs_driver.o \
	dffs_low.o \
	utils_dffs_cpu_mez.o \
	ref_design_petra_starter.o \
	ref_design_starter.o \
	utils_dune_fpga_download.o \
	utils_front_end_host_card.o \
	utils_host_board.o \
	utils_i2c_mem.o	\
	utils_line_PTG.o		
endif

BOBJS = $(addprefix ${BLDDIR}/, ${OBJS})

include ${SDK}/make/Make.subdirs
include ${SDK}/make/Make.lib
include ${SDK}/make/Make.depend
