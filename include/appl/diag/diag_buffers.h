/*
 * $Id: diag_pp.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        diag_buffers.h
 * Purpose:     
 */

#ifndef   _DIAG_BUFFERS_H_
#define   _DIAG_BUFFERS_H_

#include <appl/diag/shell.h>
#include <appl/diag/parse.h>

/* Functions */
cmd_result_t cmd_dpp_diag_buffers(int unit, args_t* a); 

void cmd_dpp_diag_buffers_usage(int unit);

#endif /* _DIAG_BUFFERS_H_ */
