
/*
 * $Id: t3p1_cmu_sim.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * g3p1_cmu_sim.h: Guadalupe2k V1.3 CMU counter access
 *
 */
 
#ifndef _SOC_SBX_T3P1_CMU_SIM_H
#define _SOC_SBX_T3P1_CMU_SIM_H

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT

#define MAX_BUFFER_SIZE 8*1024
#define MAX_NUM_FIELD 128

extern int soc_sbx_t3p1_cmu_counter_read_ext(int unit, int segment_num, int first_counter_num, int num_counters_to_read, uint64 *counters);

extern int soc_sbx_t3p1_cmu_counter_clear_ext(int unit, int segment_num);

#endif

#endif


