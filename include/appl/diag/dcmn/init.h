/* 
 * $Id: init.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        init.h
 * Purpose:     Extern declarations for DPP init routines.
 */

#ifndef _DIAG_DCMN_INIT_H
#define _DIAG_DCMN_INIT_H

#include <appl/diag/dcmn/init_deinit.h>

int appl_dpp_bcm_diag_init(int unit, appl_dcmn_init_param_t* init_param);
int appl_dpp_stk_diag_init(int unit);
int appl_dpp_cosq_diag_init(int unit);

#endif /* _DIAG_DCMN_INIT_H */
