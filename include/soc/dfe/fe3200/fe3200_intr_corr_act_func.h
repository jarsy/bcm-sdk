/*
 * $Id: fe3200_intr_corr_act_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement header correction action functions for fe3200icho interrupts.
 */

#ifndef _FE3200_INTR_CORR_ACT_FUNC_H_
#define _FE3200_INTR_CORR_ACT_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dfe/fe3200/fe3200_intr.h>
#include <soc/dcmn/dcmn_intr_corr_act_func.h>

/* Corrective Action main function */
int fe3200_interrupt_handles_corrective_action(int unit, int block_instance, fe3200_interrupt_type interrupt_id, char *msg, dcmn_int_corr_act_type corr_act, void *param1, void *param2);

/*
 *  Corrective Action functions    
 */
int fe3200_interrupt_handles_corrective_action_shutdown_fbr_link(int unit,int block_instance,fe3200_interrupt_type interrupt_id,char *msg);
int
fe3200_interrupt_data_collection_for_shadowing(int unit, int block_instance, fe3200_interrupt_type interrupt_id, char *special_msg,
                                               dcmn_int_corr_act_type* p_corrective_action,
                                               dcmn_interrupt_mem_err_info* shadow_correct_info) ;
#endif /* _FE3200_INTR_CORR_ACT_FUNC_H_ */
