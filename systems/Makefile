# $Id: Makefile,v 1.3 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Top level Makefile for Systems
#

include ${SDK}/make/Make.tools
-include ${SDK}/make/Make.local

ifeq ($(HOSTTYPE),i386-linux)
linux_dir=linux
endif

subdirs := vxworks sim ${linux_dir}
include ${SDK}/make/Make.subdirs

reldirs := vxworks sim
release: ${reldirs}
