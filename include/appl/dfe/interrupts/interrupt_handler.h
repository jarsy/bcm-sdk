/*
 * $Id: interrupt_handler.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _DFE_INTERRUPTS_HANDLER_H_
#define _DFE_INTERRUPTS_HANDLER_H_

#include <appl/dcmn/interrupts/interrupt_handler.h>

/*
* fill common parameters 
*/
int fe1600_interrupt_handler_init_cmn_param(int unit, interrupt_common_params_t* common_params);

#endif /*__DFE_INTERRUPTS_HANDLER_H_ */
