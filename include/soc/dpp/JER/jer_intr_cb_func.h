/*
 * $Id: jer_appl_intr_cb_func.h, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:     Implement CallBacks function for ARAD interrupts.
 */
 
#ifndef _JER_INTR_CB_FUNC_H_
#define _JER_INTR_CB_FUNC_H_

/*************
 * INCLUDES  *
 *************/
#include <soc/dpp/JER/jer_intr.h>

/*************
 * FUNCTIONS *
 *************/
/* init function - add handler function to db */
void jer_interrupt_cb_init(int unit);

int jer_intr_handle_generic_none(int unit, int block_instance, jer_interrupt_type en_jer_interrupt,char *msg);
int jer_intr_recurring_action_handle_generic_none(int unit, int block_instance, jer_interrupt_type en_jer_interrupt, char *msg);

int jer_interrupt_handle_oamppendingevent(int unit, int block_instance, jer_interrupt_type en_arad_interrupt, char *msg);
int jer_interrupt_handle_oamppendingstatevent(int unit, int block_instance, jer_interrupt_type en_arad_interrupt, char *msg);

#endif /* _JER_INTR_CB_FUNC_H_ */
