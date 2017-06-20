/*
 * $Id: ramon_intr_corr_act_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement header correction action functions for ramonicho interrupts.
 */

#ifndef _RAMON_INTR_CORR_ACT_FUNC_H_
#define _RAMON_INTR_CORR_ACT_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dnxf/ramon/ramon_intr.h>
#include <soc/dnxc/legacy/dnxc_intr_corr_act_func.h>

/* Corrective Action main function */
int ramon_interrupt_handles_corrective_action(int unit, int block_instance, ramon_interrupt_type interrupt_id, char *msg, dnxc_int_corr_act_type corr_act, void *param1, void *param2);

/*
 *  Corrective Action functions    
 */
int ramon_interrupt_handles_corrective_action_shutdown_fbr_link(int unit,int block_instance,ramon_interrupt_type interrupt_id,char *msg);
int
ramon_interrupt_data_collection_for_shadowing(int unit, int block_instance, ramon_interrupt_type interrupt_id, char *special_msg,
                                               dnxc_int_corr_act_type* p_corrective_action,
                                               dnxc_interrupt_mem_err_info* shadow_correct_info) ;
#endif /* _RAMON_INTR_CORR_ACT_FUNC_H_ */
