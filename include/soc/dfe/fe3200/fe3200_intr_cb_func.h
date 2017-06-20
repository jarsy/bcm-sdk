/*
 * $Id: fe3200_appl_intr_cb_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     Implement CallBacks function for fe3200 interrupts.
 */
 
#ifndef _FE3200_INTR_CB_FUNC_H_
#define _FE3200_INTR_CB_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dfe/fe3200/fe3200_intr.h>

/*************
 * FUNCTIONS *
 *************/
/* init function - add handler function to db */
void fe3200_interrupt_cb_init(int unit);

int fe3200_intr_handle_generic_none(int unit, int block_instance, fe3200_interrupt_type en_fe3200_interrupt,char *msg);
int fe3200_intr_recurring_action_handle_generic_none(int unit, int block_instance, fe3200_interrupt_type en_fe3200_interrupt, char *msg);

#endif /* _FE3200_INTR_CB_FUNC_H_ */
