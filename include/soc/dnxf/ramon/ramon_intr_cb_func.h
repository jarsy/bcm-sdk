/*
 * $Id: ramon_intr_cb_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     Implement CallBacks function for ramon interrupts.
 */
 
#ifndef _RAMON_INTR_CB_FUNC_H_
#define _RAMON_INTR_CB_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dnxf/ramon/ramon_intr.h>

/*************
 * FUNCTIONS *
 *************/
/* init function - add handler function to db */
void ramon_interrupt_cb_init(int unit);

int ramon_intr_handle_generic_none(int unit, int block_instance, ramon_interrupt_type en_ramon_interrupt,char *msg);
int ramon_intr_recurring_action_handle_generic_none(int unit, int block_instance, ramon_interrupt_type en_ramon_interrupt, char *msg);

#endif /* _RAMON_INTR_CB_FUNC_H_ */
