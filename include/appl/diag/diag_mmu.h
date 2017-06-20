/*
 * $Id: diag_mmu.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        diag_mmu.h
 * Purpose:     
 */

#ifndef   _DIAG_MMU_H_
#define   _DIAG_MMU_H_

#include <appl/diag/shell.h>
#include <appl/diag/parse.h>

/* Functions */
cmd_result_t cmd_dpp_diag_mmu(int unit, args_t* a); 

void cmd_dpp_diag_mmu_usage(int unit);

#endif /* _DIAG_MMU_H_ */
