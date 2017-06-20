/* 
 * $Id: sirius_counter.h,v 1.12 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sirius_counter.h
 * Purpose:     SBX Software Counter Collection module definitions for SIRIUS.
 */

#ifndef   _SOC_SBX_SIRIUS_COUNTER_H_
#define   _SOC_SBX_SIRIUS_COUNTER_H_

#include <soc/sbx/counter.h>



extern int soc_sbx_sirius_counter_init(int unit, 
				       uint32 flags, 
				       int interval,
				       pbmp_t pbmp);
extern int soc_sbx_sirius_process_slq_stats(int unit);
extern int soc_sbx_sirius_process_global_stats(int unit);
extern int soc_sbx_sirius_process_fd_drop_stats(int unit, int clear);
extern int soc_sbx_sirius_get_segment(uint32 unit, 
				      uint8 cu,
				      int32 *rSegment);
extern int soc_sbx_sirius_free_segment(uint32 unit, 
				       uint32 segment);
extern int soc_sbx_sirius_create_group(uint32 unit, 
					 uint32 segment, 
					 uint32 cu_num, 
					 uint32 cntrId, 
					 uint32 size);
extern int soc_sbx_sirius_remove_group(uint32 unit, 
				       uint32 segment, 
				       uint32 cu_num, 
				       uint32 cntrId, 
				       uint32 size);
extern int soc_sbx_sirius_provision_group(uint32 unit, 
					    uint32 segment, 
					    uint64 *ullCntAddr);
extern int soc_sbx_sirius_init_group(uint32 unit, 
				     uint32 segment, 
				     uint32 cntr_id, 
				     uint32 num_cntrs);
extern int soc_sbx_sirius_flush_segment(uint32 unit, 
					uint32 segment, 
					uint32 cntrId, 
					uint32 num_cntrs);

#define    SIRIUS_COUNTER_QM_COUNT 32

int soc_sbx_sirius_test(int unit);

extern uint32 soc_sirius_qm_counter_base_set(int unit,
					     int32 nBaseQueue,
					     int32 enable);

extern uint32 soc_sirius_qm_counter_base_get(int unit, int32 *nBaseQueue);

extern uint32 soc_sirius_qm_counter_read(int unit, int32 set, uint32 *puCounterBase);

extern int soc_sbx_sirius_counter_port_get(int unit, int port, int block, int set,
                                           int counter, uint64 *val);

extern int soc_sbx_sirius_counter_port_set(int unit, int port, int block , int set,
                                           int counter, uint64 val);

extern int soc_sbx_sirius_counter_bset_collect( int unit, int discard, int block, int set);

extern int soc_sirius_counter_enable_get(int unit, int *base, int *result);

extern int soc_sirius_counter_enable_set(int unit, int base );

extern int soc_sirius_counter_enable_clear(int unit);

#endif /* _SOC_SBX_SIRIUS_COUNTER_H_ */
