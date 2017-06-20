/*
 * $Id: caladan3_cmds.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        caladan3_cmds.c
 * Purpose:     Caladan3-specific diagnostic shell commands
 * Requires:
 */

#ifndef _APPL_DIAG_CALADAN3_CMDS_H
#define _APPL_DIAG_CALADAN3_CMDS_H

#include <appl/diag/diag.h>

extern cmd_result_t
cmd_sbx_caladan3_pd_hc(int unit, int queue, int on, int drop);

extern cmd_result_t
cmd_sbx_caladan3_ppe_hc(int unit, int on, int clear,
                        int queue, int str, int var, int varmask);
cmd_result_t
cmd_sbx_caladan3_ped_hd(int unit, int pedin, int pedout, int parsed);

cmd_result_t
cmd_sbx_caladan3_ppe_hd(int unit, int parsed);

cmd_result_t
cmd_sbx_caladan3_ped_show(int unit);

cmd_result_t
cmd_sbx_caladan3_ppe_show(int unit);

cmd_result_t
cmd_sbx_caladan3_ped_check(int unit, int word, int data, int data_mask);

cmd_result_t
cmd_sbx_caladan3_ped_clear(int unit);

extern 
int bb_caladan3_current(int unit, int bx);

extern 
int bb_caladan3_voltage(int unit, int bx, char *source, char* value);

extern void cmd_sbx_caladan3_print_queue_info(int unit, int queue);

#endif
