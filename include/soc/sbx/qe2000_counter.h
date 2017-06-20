/* 
 * $Id: qe2000_counter.h,v 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        qe2000_counter.h
 * Purpose:     SBX Software Counter Collection module definitions for QE2000.
 */

#ifndef   _SOC_SBX_QE2000_COUNTER_H_
#define   _SOC_SBX_QE2000_COUNTER_H_

#include <soc/sbx/counter.h>

#define    QE2000_COUNTER_QM_COUNT 24


extern int soc_sbx_qe2000_counter_init(int unit, uint32 flags, int interval, pbmp_t pbmp);

int soc_sbx_qe2000_test(int unit);

extern int soc_sbx_qe2000_counter_port_get(int unit, int port, int block, int set,
                                           int counter, uint64 *val);

extern int soc_sbx_qe2000_counter_port_set(int unit, int port, int block , int set,
                                           int counter, uint64 val);

extern int soc_sbx_qe2000_counter_bset_collect( int unit, int discard, int block, int set);


extern int soc_qe2000_counter_enable_get(int unit, int *base, int *result);

extern int soc_qe2000_counter_enable_set(int unit, int base );

extern int soc_qe2000_counter_enable_clear(int unit);



#endif /* _SOC_SBX_QE2000_COUNTER_H_ */
