# $Id: Makefile,v 1.7 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Top level Makefile for BSPs
#

include ${SDK}/make/Make.tools
-include ${SDK}/make/Make.local

# Currently certain BSPs compile properly only with Solaris-based tools.

subdirs := mousse bmw
ifeq ($(HOSTTYPE),sun4)
subdirs += jag nsx raptor
endif

include ${SDK}/make/Make.subdirs

reldirs := $(subdirs)
release: ${reldirs}
