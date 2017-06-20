/* 
 * $Id: counter.h,v 1.3.474.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        counter.h
 * Purpose:     SBX Software Counter Collection module definitions.
 */

#ifndef   _SOC_SBX_COUNTER_H_
#define   _SOC_SBX_COUNTER_H_

#include <sal/types.h>
#include <soc/types.h>

/*
 * Counter Block Definition
 *
 * Contains definition for counter blocks available in a device.
 *
 * Definitions / Assumptions:
 *   - a 'block' contains 1 or more equal 'sets' of counters,
 *   - a 'set' contains 1 or more 'counters'
 */
typedef int (*fn_read)(int unit, int set, int counter, uint64 *val, int *width);
typedef int (*fn_write)(int unit, int set, int counter, uint64 val);

typedef struct soc_sbx_counter_block_info_s {
    int           block;         /* ID for a counter block */
    int           num_sets;      /* Number of counter sets in block */
    int           num_counters;  /* Number of counters in each set */
    fn_read       read;          /* Function to read a given counter */
    fn_write      write;         /* Function to write a given counter */
} soc_sbx_counter_block_info_t;


/*
 * External Functions
 */
extern int soc_sbx_counter_init(int unit,
                                soc_sbx_counter_block_info_t *block_info,
                                int block_count);
extern int soc_sbx_counter_detach(int unit);

extern int soc_sbx_counter_bset_add(int unit, int block, int set);

extern int soc_sbx_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp);
extern int soc_sbx_counter_status(int unit, uint32 *flags, int *interval, pbmp_t *pbmp);
extern int soc_sbx_counter_stop(int unit);
extern int soc_sbx_counter_sync(int unit);

extern int soc_sbx_counter_get(int unit, int block, int set,
                               int counter, uint64 *val);

extern int soc_sbx_counter_get_zero(int unit, int block, int set,
                                    int counter, uint64 *val);

extern int soc_sbx_counter_set(int unit, int block, int set,
                               int counter, uint64 val);

extern int soc_sbx_controlled_counter_clear(int unit, soc_port_t port);

extern int soc_sbx_counter_dump(int unit);

extern int soc_sbx_counter_bset_clear(int unit, int block);

extern int
soc_sbx_caladan3_controlled_counters_num_get(void);

soc_controlled_counter_t *
soc_sbx_caladan3_controlled_counters_get(int unit, int index);

int
soc_sbx_caladan3_controlled_counter_init(int unit);

extern int
_soc_counter_sbusdma_setup(int unit);

extern int 
_soc_counter_sbudma_desc_free_all(int unit);

extern int 
soc_controlled_counters_collect64(int unit, int discard);


#endif /* _SOC_SBX_COUNTER_H_ */
