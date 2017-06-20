/* $Id: chip_sim_log.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __CHIP_SIM_LOG_H__
/* { */
#define __CHIP_SIM_LOG_H__


/*
 * INCLUDE FILES:
 * {
 */

#ifdef SAND_LOW_LEVEL_SIMULATION
/* { */
/* } */
#else
/* { */
/* } */
#endif

/*  } */

STATUS chip_sim_log_malloc(SOC_SAND_IN int log_char_size);
STATUS chip_sim_log_free(void);
STATUS chip_sim_log_run(SOC_SAND_IN char str[]);
char* chip_sim_log_get_and_lock(void);
void  chip_sim_log_unlock_clear(void);
uint8 chip_sim_log_get_spy_low_mem(void);
void chip_sim_log_set_spy_low_mem(uint8 b);

/* } __CHIP_SIM_LOG_H__*/
#endif
