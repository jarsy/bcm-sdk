/*
 * $Id: cmdlist.h,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    cmdlist.h
 * Purpose: Extern declarations for command functions and
 *          their associated usage strings.
 */

#ifndef _INCLUDE_DNX_CMDLIST_H
#define _INCLUDE_DNX_CMDLIST_H

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <appl/diag/diag.h>

extern int   bcm_dnx_cmd_cnt;
extern cmd_t bcm_dnx_cmd_list[];

cmd_result_t
cmd_dnx_verify(int unit);

#endif  /*  _INCLUDE_DNX_CMDLIST_H */
