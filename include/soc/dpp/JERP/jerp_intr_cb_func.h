/*
 * $Id: jerp_appl_intr_cb_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     Implement CallBacks function for ARAD interrupts.
 */
 
#ifndef _JERP_INTR_CB_FUNC_H_
#define _JERP_INTR_CB_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dpp/JERP/jerp_intr.h>

/*************
 * FUNCTIONS *
 *************/
/* init function - add handler function to db */
void jerp_interrupt_cb_init(int unit);

int jerp_intr_handle_generic_none(int unit, int block_instance, jerp_interrupt_type en_jerp_interrupt,char *msg);
int jerp_intr_recurring_action_handle_generic_none(int unit, int block_instance, jerp_interrupt_type en_jerp_interrupt, char *msg);

#endif /* _JERP_INTR_CB_FUNC_H_ */

