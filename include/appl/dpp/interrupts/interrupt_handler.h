/*
 * $Id: interrupts_handler.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _DPP_INTERRUPTS_HANDLER_H_
#define _DPP_INTERRUPTS_HANDLER_H_

#include <soc/dpp/ARAD/arad_interrupts.h>
#include <appl/dcmn/interrupts/interrupt_handler.h>

/*
* fill common parameters 
*/
int arad_interrupt_handler_init_cmn_param(int unit, interrupt_common_params_t* interrupt_common_params);

#endif /*__DPP_INTERRUPTS_HANDLER_H_ */
