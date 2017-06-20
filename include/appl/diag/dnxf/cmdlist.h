/*
 * $Id: cmdlist.h,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    cmdlist.h
 * Purpose: Extern declarations for command functions and
 *          their associated usage strings.
 */

#ifndef DNXF_CMDLIST_H_INCLUDED
#define DNXF_CMDLIST_H_INCLUDED

#ifndef BCM_DNXF_SUPPORT
#error "This file is for use by DNXF (Ramon) family only!"
#endif

#include <appl/diag/diag.h>

extern int   bcm_dnxf_cmd_cnt;
extern cmd_t bcm_dnxf_cmd_list[];

cmd_result_t
cmd_dnxf_verify(int unit);

#endif  /*  DNXF_CMDLIST_H_INCLUDED */
