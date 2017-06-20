/*
 * $Id: qe2000_cmds.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qe2000_cmds.h
 * Purpose:     QE-2000 diagnostic shell commands
 * Requires:    
 */

#ifndef _APPL_DIAG_QE2000_CMDS_H
#define _APPL_DIAG_QE2000_CMDS_H

#include <appl/diag/diag.h>

extern int cmd_sbx_qe2000_print_queue_info(int unit, int queue);
extern int sbQe2000MemShow(int unit, char *memname, int rangemin, int rangemax);
extern int sbQe2000MemSetField(int unit, char *memFieldName, int addr, int val);

#endif	/* !_APPL_DIAG_FE2000_CMDS_H */

