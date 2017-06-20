# Makefile - makefile for Wind River BCM53000 BSP
# $Id$
# Copyright (c) 2006-2007 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.
# 
# modification history
# --------------------
#
# DESCRIPTION
#
# INCLUDES
#     makeTarget
# Separated so that we can build .le independent from .be
#*/


override export VX_TARGET_BIG_ENDIAN = 0
__BASE :=../vxworks-6.6

include $(__BASE)/Makefile

EXTRA_INCLUDE += -I$(__BASE)

all/%.c: 
	ln -s $(__BASE)/all

$(notdir $(wildcard $(__BASE)/*.c $(__BASE)/*.h $(__BASE)/*.s)):  
	ln -s $(__BASE)/$@

