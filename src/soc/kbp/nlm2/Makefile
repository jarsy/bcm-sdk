# $Id: Makefile,v 1.4 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
#
PLATFORM=m32

include ${SDK}/make/Make.config

LIBDIRS = nlmdevmgr nlmrangemgr nlmgenerictblmgr nlmfibtblmgr nlmxpt 
APPDIRS = nlmgenerictblmgr_refapp nlmgtmftm_refapp diagnostic 
APPDIRS += nlmdevmgr_refapp nlmrangemgr_refapp nlmrangemgr_refapp2 \
        nlmrangemgr_refapp3 nlmfibtblmgr_refapp

$(foreach lib, $(LIBDIRS),$(eval subdirs += $(lib)))
$(foreach lib, $(APPDIRS),$(eval subdirs += $(lib)))

include ${SDK}/make/Make.subdirs

include ${SDK}/make/Make.depend

