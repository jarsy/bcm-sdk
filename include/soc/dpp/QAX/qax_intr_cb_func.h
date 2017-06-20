/*
 * $Id: qax_appl_intr_cb_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     Implement CallBacks function for ARAD interrupts.
 */
 
#ifndef _QAX_INTR_CB_FUNC_H_
#define _QAX_INTR_CB_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dpp/QAX/qax_intr.h>

/*************
 * FUNCTIONS *
 *************/
/* init function - add handler function to db */
void qax_interrupt_cb_init(int unit);

int qax_intr_handle_generic_none(int unit, int block_instance, qax_interrupt_type en_qax_interrupt,char *msg);
int qax_intr_recurring_action_handle_generic_none(int unit, int block_instance, qax_interrupt_type en_qax_interrupt, char *msg);

#endif /* _QAX_INTR_CB_FUNC_H_ */
