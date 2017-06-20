/*
 * $Id: qux_appl_intr_cb_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     Implement CallBacks function for ARAD interrupts.
 */
 
#ifndef _QUX_INTR_CB_FUNC_H_
#define _QUX_INTR_CB_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dpp/QUX/qux_intr.h>

/*************
 * FUNCTIONS *
 *************/
/* init function - add handler function to db */
void qux_interrupt_cb_init(int unit);

int qux_intr_handle_generic_none(int unit, int block_instance, qux_interrupt_type en_qux_interrupt,char *msg);
int qux_intr_recurring_action_handle_generic_none(int unit, int block_instance, qux_interrupt_type en_qux_interrupt, char *msg);

#endif /* _QUX_INTR_CB_FUNC_H_ */
