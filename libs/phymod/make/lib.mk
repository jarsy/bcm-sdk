# $Id$
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# PHYMOD make rules for libraries
#

include $(PHYMOD)/make/config.mk

include $(PHYMOD)/make/rules.mk

BLIBNAME = $(LIBDIR)/$(LIBNAME).$(LIBSUFFIX)

.SECONDARY:: $(BOBJS)

all:: $(BLDDIR)/.tree $(BLIBNAME)

clean::
	@$(ECHO) Cleaning objects for $(notdir $(BLIBNAME))
	$(Q)$(RM) $(BLDDIR)/.tree $(BOBJS) $(BLIBNAME)

include $(PHYMOD)/make/depend.mk
