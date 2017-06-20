/*
 * $Id: bm3200_cmds.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        bm3200_cmds.h
 * Purpose:     BM-3200 diagnostic shell commands
 * Requires:    
 */

#ifndef _APPL_DIAG_BM3200_CMDS_H
#define _APPL_DIAG_BM3200_CMDS_H

#include <appl/diag/diag.h>

extern int sbBm3200MemShow(int unit, char *memname, int rangemin, int rangemax, int instance);
extern int sbBm3200MemSetField(int unit, char *memFieldName, int addr, int val, int instance);
extern void sbBm3200ShowMemNames(void);



#endif	/* !_APPL_DIAG_BM3200_CMDS_H */


