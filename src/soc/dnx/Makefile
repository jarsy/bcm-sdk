#
# $Id: Makefile,v 1.4 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#


LOCALDIR = src/soc/dnx

include ${SDK}/make/Make.config

subdirs :=	dbal     \
			swstate  \
			cmodel   \
			mdb      \
			pemladrv \
			drv	    \
			dnx_data \
			dnx_data/jer2_a0\
			legacy\
			

include ${SDK}/make/Make.subdirs

lib = libsoc_dnx

include ${SDK}/make/Make.lib

include ${SDK}/make/Make.depend

