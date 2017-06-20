#
# $Id$
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
LOCALDIR = libs/phymod

ifndef PHYMOD
PHYMOD = $(CURDIR)
endif

include ${PHYMOD}/make/config.mk

# Core library
VPATH = acc core diag
VSRCS = $(wildcard acc/*.c) $(wildcard core/*.c) $(wildcard diag/*.c)

# Symbolic debug
VPATH += sym sym/chip
VSRCS += $(wildcard sym/*.c) $(wildcard sym/chip/*.c)

# PHY simulators
VPATH += sim sim/chip
VSRCS += $(wildcard sim/*.c) $(wildcard sim/chip/*.c)

# PHY util
VPATH += util 
VSRCS += $(wildcard util/*.c) 

#
# The PHYMOD_CHIPS make variable can be used to compile the sources
# for only one chip (or a subset of chips).
#
# Please see $PHYMOD/examples/libonly/Makefile for examples
#
ifneq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS := $(PHYMOD_CHIPS)
endif

# Eagle chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += EAGLE
endif
ifneq ($(filter EAGLE,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/eagle chip/eagle/tier2 chip/eagle/tier1
VSRCS += $(wildcard chip/eagle/*.c)
VSRCS += $(wildcard chip/eagle/tier2/*.c)
VSRCS += $(wildcard chip/eagle/tier1/*.c)
endif

# Eagle dpll chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += EAGLE_DPLL
endif
ifneq ($(filter EAGLE_DPLL,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/eagle_dpll chip/eagle_dpll/tier2 chip/eagle_dpll/tier1
VSRCS += $(wildcard chip/eagle_dpll/*.c)
VSRCS += $(wildcard chip/eagle_dpll/tier2/*.c)
VSRCS += $(wildcard chip/eagle_dpll/tier1/*.c)
endif

# Falcon chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += FALCON
endif
ifneq ($(filter FALCON,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/falcon chip/falcon/tier2 chip/falcon/tier1
VSRCS += $(wildcard chip/falcon/*.c)
VSRCS += $(wildcard chip/falcon/tier2/*.c)
VSRCS += $(wildcard chip/falcon/tier1/*.c)
endif

# QSGMII/Eagle chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += QSGMIIE
endif
ifneq ($(filter QSGMIIE,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/qsgmiie chip/qsgmiie/tier2 chip/qsgmiie/tier1
VSRCS += $(wildcard chip/qsgmiie/*.c)
VSRCS += $(wildcard chip/qsgmiie/tier2/*.c)
VSRCS += $(wildcard chip/qsgmiie/tier1/*.c)
endif

# TSC/Eagle chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += TSCE
endif
ifneq ($(filter TSCE,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/tsce chip/tsce/tier2 chip/tsce/tier1
VSRCS += $(wildcard chip/tsce/*.c)
VSRCS += $(wildcard chip/tsce/tier2/*.c)
VSRCS += $(wildcard chip/tsce/tier1/*.c)
endif

# TSC/Eagle DPLL chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += TSCE_DPLL
endif
ifneq ($(filter TSCE_DPLL,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/tsce_dpll chip/tsce_dpll/tier2 chip/tsce_dpll/tier1
VSRCS += $(wildcard chip/tsce_dpll/*.c)
VSRCS += $(wildcard chip/tsce_dpll/tier2/*.c)
VSRCS += $(wildcard chip/tsce_dpll/tier1/*.c)
endif

# TSC/Falcon chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += TSCF
endif
ifneq ($(filter TSCF,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/tscf chip/tscf/tier2 chip/tscf/tier1
VSRCS += $(wildcard chip/tscf/*.c)
VSRCS += $(wildcard chip/tscf/tier2/*.c)
VSRCS += $(wildcard chip/tscf/tier1/*.c)
endif

# Viper chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += VIPER
endif
ifneq ($(filter VIPER,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/viper chip/viper/tier2 chip/viper/tier1
VSRCS += $(wildcard chip/viper/*.c)
VSRCS += $(wildcard chip/viper/tier2/*.c)
VSRCS += $(wildcard chip/viper/tier1/*.c)
endif

# Furia chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += FURIA
endif
ifneq ($(filter FURIA,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/furia chip/furia/tier2 chip/furia/tier1 chip/furia/tier1/falcon_furia_src chip/furia_82212 chip/furia_82212/tier2
VSRCS += $(wildcard chip/furia/*.c)
VSRCS += $(wildcard chip/furia/tier2/*.c)
VSRCS += $(wildcard chip/furia/tier1/*.c)
VSRCS += $(wildcard chip/furia/tier1/falcon_furia_src/falcon_furia_functions.c)
VSRCS += $(wildcard chip/furia/tier1/falcon_furia_src/falcon_furia_dependencies.c)

VSRCS += $(wildcard chip/furia_82212/*.c)
VSRCS += $(wildcard chip/furia_82212/tier2/*.c)
endif

# QTC chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += QTC
endif
ifneq ($(filter QTC,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/qtce chip/qtce/tier2 chip/qtce/tier1
VSRCS += $(wildcard chip/qtce/*.c)
VSRCS += $(wildcard chip/qtce/tier2/*.c)
VSRCS += $(wildcard chip/qtce/tier1/*.c)
endif

# Sesto chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += SESTO
endif
ifneq ($(filter SESTO,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/sesto chip/sesto/tier2 chip/sesto/tier1 chip/sesto/tier1/sesto_serdes/falcon_furia_sesto_src chip/sesto/tier1/sesto_serdes/merlin_sesto_src
VSRCS += $(wildcard chip/sesto/*.c)
VSRCS += $(wildcard chip/sesto/tier2/*.c)
VSRCS += $(wildcard chip/sesto/tier1/*.c)
VSRCS += $(wildcard chip/sesto/tier1/sesto_serdes/falcon_furia_sesto_src/falcon_furia_sesto_functions.c)
VSRCS += $(wildcard chip/sesto/tier1/sesto_serdes/falcon_furia_sesto_src/falcon_furia_sesto_dependencies.c)
VSRCS += $(wildcard chip/sesto/tier1/sesto_serdes/merlin_sesto_src/merlin_sesto_functions.c)
VSRCS += $(wildcard chip/sesto/tier1/sesto_serdes/merlin_sesto_src/merlin_sesto_dependencies.c)
endif


# Madura chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += MADURA
endif
ifneq ($(filter MADURA,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/madura chip/madura/tier2 chip/madura/tier1 chip/madura/tier1/madura_serdes/falcon_madura_src  chip/madura/tier1/madura_serdes/falcon2_madura_src
VSRCS += $(wildcard chip/madura/*.c)
VSRCS += $(wildcard chip/madura/tier2/*.c)
VSRCS += $(wildcard chip/madura/tier1/*.c)
VSRCS += $(wildcard chip/madura/tier1/madura_serdes/falcon_madura_src/falcon_madura_functions.c)
VSRCS += $(wildcard chip/madura/tier1/madura_serdes/falcon_madura_src/falcon_madura_dependencies.c)
VSRCS += $(wildcard chip/madura/tier1/madura_serdes/falcon2_madura_src/falcon2_madura_functions.c)
VSRCS += $(wildcard chip/madura/tier1/madura_serdes/falcon2_madura_src/falcon2_madura_dependencies.c)
endif


# quadra28 chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += QUADRA28
endif
ifneq ($(filter QUADRA28,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/quadra28 chip/quadra28/tier2 chip/quadra28/tier1 chip/quadra28/tier1/quadra28_serdes
VSRCS += $(wildcard chip/quadra28/*.c)
VSRCS += $(wildcard chip/quadra28/tier2/*.c)
VSRCS += $(wildcard chip/quadra28/tier1/*.c)
VSRCS += $(wildcard chip/quadra28/tier1/quadra28_serdes/merlin_quadra28_functions.c)
VSRCS += $(wildcard chip/quadra28/tier1/quadra28_serdes/merlin_quadra28_dependencies.c)
endif


# huracan  chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += HURACAN
endif
ifneq ($(filter HURACAN,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/huracan chip/huracan/tier2 chip/huracan/tier1 
VSRCS += $(wildcard chip/huracan/*.c)
VSRCS += $(wildcard chip/huracan/tier2/*.c)
VSRCS += $(wildcard chip/huracan/tier1/*.c)
endif

# Dino chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += DINO 
endif
ifneq ($(filter DINO,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/dino chip/dino/tier2 chip/dino/tier1 chip/dino/tier1/dino_serdes/falcon2_dino_src chip/dino/tier1/dino_serdes/merlin_dino_src
VSRCS += $(wildcard chip/dino/*.c)
VSRCS += $(wildcard chip/dino/tier2/*.c)
VSRCS += $(wildcard chip/dino/tier1/*.c)
VSRCS += $(wildcard chip/dino/tier1/dino_serdes/falcon2_dino_src/falcon2_dino_functions.c)
VSRCS += $(wildcard chip/dino/tier1/dino_serdes/falcon2_dino_src/falcon2_dino_dependencies.c)
VSRCS += $(wildcard chip/dino/tier1/dino_serdes/merlin_dino_src/merlin_dino_functions.c)
VSRCS += $(wildcard chip/dino/tier1/dino_serdes/merlin_dino_src/merlin_dino_dependencies.c)
endif
# phy8806x support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += PHY8806X
endif
ifneq ($(filter PHY8806X,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/phy8806x chip/phy8806x/tier2 chip/phy8806x/tier1
VSRCS += $(wildcard chip/phy8806x/*.c)
VSRCS += $(wildcard chip/phy8806x/tier2/*.c)
VSRCS += $(wildcard chip/phy8806x/tier1/*.c)
endif

# Falcon16 chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += FALCON16
endif
ifneq ($(filter FALCON16,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/falcon16 chip/falcon16/tier2 chip/falcon16/tier1 chip/falcon16/tier1/src
VSRCS += $(wildcard chip/falcon16/*.c)
VSRCS += $(wildcard chip/falcon16/tier2/*.c)
VSRCS += $(wildcard chip/falcon16/tier1/*.c)
VSRCS += $(wildcard chip/falcon16/tier1/src/*.c)
endif

# TSCF16 chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += TSCF16
endif
ifneq ($(filter TSCF16,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/tscf16 chip/tscf16/tier2 chip/tscf16/tier1
VSRCS += $(wildcard chip/tscf16/*.c)
VSRCS += $(wildcard chip/tscf16/tier2/*.c)
VSRCS += $(wildcard chip/tscf16/tier1/*.c)
endif

# MERLIN16 chip support
ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += MERLIN16 
endif
ifneq ($(filter MERLIN16,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/merlin16 chip/merlin16/tier2 chip/merlin16/tier1
VSRCS += $(wildcard chip/merlin16/*.c)
VSRCS += $(wildcard chip/merlin16/tier2/*.c)
VSRCS += $(wildcard chip/merlin16/tier1/*.c)
endif

ifeq ($(PHYMOD_CHIPS),)
MAKE_PHYMOD_CHIPS += TSCE16 
endif
ifneq ($(filter TSCE16,$(MAKE_PHYMOD_CHIPS)),)
VPATH += chip/tsce16 chip/tsce16/tier2 chip/tsce16/tier1
VSRCS += $(wildcard chip/tsce16/*.c)
VSRCS += $(wildcard chip/tsce16/tier2/*.c)
VSRCS += $(wildcard chip/tsce16/tier1/*.c)
endif

LSRCS = $(notdir $(VSRCS))
LOBJS = $(addsuffix .o, $(basename $(LSRCS)))
BOBJS = $(addprefix $(BLDDIR)/,$(LOBJS))

LIBNAME = libphymod
include ${PHYMOD}/make/lib.mk

include ${PHYMOD}/make/depend.mk
