#
# $Id: Makefile,v 1.4 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Makefile for dpp subdirectory
#

LOCALDIR = src/soc/dnx/legacy

include ${SDK}/make/Make.config

subdirs := SAND/Management \
	SAND/SAND_FM    \
	SAND/Utils      \
	TMC             \
	ARAD			\
	QAX				\
	JER				\
	JER2				\


include ${SDK}/make/Make.subdirs

lib = libsoc_dnxlegacy

include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend

