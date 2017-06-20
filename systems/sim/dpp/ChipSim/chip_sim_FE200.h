/* $Id: chip_sim_FE200.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/



#ifndef _CHIP_SIM_FE200_H_
/* { */
#define _CHIP_SIM_FE200_H_


#include "chip_sim.h"
#include "chip_sim_counter.h"
#include "chip_sim_interrupts.h"
#include "chip_sim_indirect.h"

extern
  CHIP_SIM_INDIRECT_BLOCK
    Fe_indirect_blocks[];
extern
  CHIP_SIM_COUNTER
    Fe_counters[];
extern
  CHIP_SIM_INTERRUPT
    Fe_interrupts[];

/* } _CHIP_SIM_FE200_H_*/
#endif

