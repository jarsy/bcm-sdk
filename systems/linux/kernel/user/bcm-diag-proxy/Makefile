# -*- Makefile -*-
# $Id: Makefile,v 1.9 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
LOCALDIR = systems/linux/kernel/user/bcm-diag-proxy

include $(SDK)/make/Make.config

CFLAGS += -I../../modules/include

BCM_PROXY = $(LIBDIR)/bcm.user.proxy

ifndef NO_SAL_APPL
COND_LIB_SAL_APPL=libsal_appl.a
COND_LIB_SAL_APPL_PLATFORM=libsal_appl_plat.a
endif

USER_LIBS = \
	$(COND_LIB_SAL_APPL) \
	$(COND_LIB_EDITLINE) \
	$(COND_LIB_SAL_APPL_PLATFORM) \
	libsal_core.a libsal_core_plat.a \
	libuser.a

USER_LIBS_BLD = $(addprefix $(LIBDIR)/,$(USER_LIBS))

build: user_libs $(BCM_PROXY)

# BCM Proxy

$(BCM_PROXY): $(BLDDIR)/.tree $(BOBJS) $(USER_LIBS_BLD)
	mkdir -p $(@D)
	@rm -f $@	# Prevent core dump if currently running
	$(CC) $(CFLAGS) -o $(BCM_PROXY) $(BOBJS) $(USER_LIBS_BLD) $(LDFLAGS)

# Libraries

.PHONY: user_libs

user_libs:
	$(MAKE) -C $(SDK)/src/sal
	$(MAKE) -C $(SDK)/systems/linux/kernel/user/shared

include $(SDK)/make/Make.depend

# Make.depend is before clean:: so that Make.depend's clean:: runs first.

clean::
	$(MAKE) -C $(SDK)/src/sal $@
	$(MAKE) -C $(SDK)/systems/linux/kernel/user/shared $@
	$(RM) $(BOBJS) $(BCM_PROXY)
