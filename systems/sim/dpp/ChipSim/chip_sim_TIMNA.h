/* $Id: chip_sim_TIMNA.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/



#ifndef _CHIP_SIM_TIMNA_H_
/* { */
#define _CHIP_SIM_TIMNA_H_


#include "chip_sim.h"
#include "chip_sim_counter.h"
#include "chip_sim_interrupts.h"
#include "chip_sim_indirect.h"

extern
  CHIP_SIM_INDIRECT_BLOCK
    Timna_indirect_blocks[];
extern
  CHIP_SIM_COUNTER
    Timna_counters[];
extern
  CHIP_SIM_INTERRUPT
    Timna_interrupts[];

/* } _CHIP_SIM_TIMNA_H_*/
#endif

