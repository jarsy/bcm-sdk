/*
 * $Id: cache_mem_test.h,v 1.0 2014/08/14 MiryH Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _CACHE_MEM_TEST_H
#define _CACHE_MEM_TEST_H

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SBX_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)

#include <appl/diag/shell.h>

int		do_cache_mem_test(int , args_t *a, void *);
int		do_cache_mem_test_init(int, args_t *, void **);
int		do_cache_mem_test_done(int, void *);



typedef enum cache_mem_test_write_value_pattern_e {
	cache_mem_test_write_value_pattern_all_ones = 0,
	cache_mem_test_write_value_pattern_all_zeroes,
	cache_mem_test_write_value_pattern_incremental,
	cache_mem_test_write_value_pattern_smart
}cache_mem_test_write_value_pattern_t;

typedef enum cache_mem_test_type_e {
	cache_mem_test_type_single = 0,
	cache_mem_test_type_all_mems
}cache_mem_test_type_t;

typedef enum cache_mem_test_write_type_e {
	cache_mem_test_write_type_dma = 0,
	cache_mem_test_write_type_schan
}cache_mem_test_write_type_t;

typedef enum cache_mem_test_partial_e {
	cache_mem_test_full = 0,
	cache_mem_test_write_only,
	cache_mem_test_read_only,
	cache_mem_test_cache_only
}cache_mem_test_partial_t;

typedef struct tr_do_cache_mem_test_e
{
	cache_mem_test_type_t				test_type;
	cache_mem_test_write_type_t			write_type;
	cache_mem_test_write_value_pattern_t write_value_pattern;
	cache_mem_test_partial_t			test_part;
    int                                 stat_mem_not_tested_cnt;
    int                                 stat_mem_succeed_cnt;
    int                                 stat_mem_fail_cnt;
    int                                 stat_mem_total_cnt;
	uint32								mem_id;
	cmd_result_t						result;
} tr_do_cache_mem_test_t;

#endif /*#(BCM_ESW_SUPPORT) || (BCM_SBX_SUPPORT) || (BCM_PETRA_SUPPORT) || (BCM_DFE_SUPPORT) */
#endif /*_CACHE_MEM_TEST_H*/

