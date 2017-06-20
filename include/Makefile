# $Id: Makefile,v 1.2 Broadcom SDK $
# $Copyright: (c) 2016 Broadcom.
# Broadcom Proprietary and Confidential. All rights reserved.$
#
# Include standard Makefile configuration
#

include ${SDK}/make/Make.config
include ${SDK}/make/Make.depend

all::

install:: ${install_files}

${install_files}::
	${INSTALL} -d ${install_headers}/$(dir $@)
	${INSTALL} -m0644 -c $@ ${install_headers}/$(dir $@)

clean::

distclean::
	$(RM) ${install_headers}
