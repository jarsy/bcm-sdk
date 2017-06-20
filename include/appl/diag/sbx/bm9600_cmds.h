/*
 * $Id: bm9600_cmds.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        bm9600_cmds.h
 * Purpose:     BM-9600 diagnostic shell commands
 * Requires:    
 */

#ifndef _APPL_DIAG_BM9600_CMDS_H
#define _APPL_DIAG_BM9600_CMDS_H

#include <appl/diag/diag.h>

extern int sbBm9600MemShow(int unit, char *memname, int rangemin, int rangemax, int instance);
extern int sbBm9600MemSetField(int unit, char *memFieldName, int addr, int val, int instance);
extern void sbBm9600ShowMemNames(void);

#endif	/* !_APPL_DIAG_BM9600_CMDS_H */


