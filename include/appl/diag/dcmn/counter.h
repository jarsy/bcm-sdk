/* 
 * $Id: counter.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        counter.h
 * Purpose:     Extern declarations for counter support.
 */

#ifndef   _DIAG_DCMN_COUNTER_H_
#define   _DIAG_DCMN_COUNTER_H_

extern int
do_show_dpp_counters(int unit, soc_reg_t ctr_reg, soc_pbmp_t pbmp, int flags);

/*access to diag counter data base*/
extern void
dpp_counter_val_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                    int ar_idx, uint64 val);

/*access to diag counter data base*/
extern void
dpp_counter_val_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
                int ar_idx, uint64 *val);

#endif /* _DIAG_DCMN_COUNTER_H_ */
