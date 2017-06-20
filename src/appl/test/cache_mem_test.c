/*
 * $Id: cache_mem_test.c,v 1.0 2014/08/14 MiryH Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/* we need this junk function only to avoid building error of pedantic compilation */
void ___junk_function_cache_mem_test(void){
}

#if defined(BCM_ESW_SUPPORT) || defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
#include <shared/bsl.h>
#include <sal/types.h>
#include <sal/compiler.h>
#include <sal/core/libc.h>
#include <soc/defs.h>
#include <soc/mcm/memregs.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>

#include <soc/shared/sat.h>
#include <appl/diag/system.h>

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <bcm_int/dpp/counters.h>
#endif
#include "cache_mem_test.h"

/* EXTERN Definitions */
extern int bcm_common_linkscan_enable_set(int,int);

/* MACRO Definitions */
#define SOC_MEM_ENTRY_LENGTH_MAX   120
/* FUNCTION Definitions */
int cache_mem_test_read_and_compare(SOC_SAND_IN int unit, SOC_SAND_IN soc_mem_t mem, void* data);
uint32 cache_mem_test_generate_value(uint32 unit, soc_mem_t mem, uint32 array_index, uint32 index, cache_mem_test_write_value_pattern_t pattern);
void cache_mem_test_create_mask(SOC_SAND_IN  uint32 unit,SOC_SAND_IN soc_mem_t mem, SOC_SAND_IN size_t mask_len, SOC_SAND_OUT uint32* mask);
void do_cache_mem_test_print_usage(void);


uint32
cache_mem_test_generate_value(uint32 unit, soc_mem_t mem, uint32 array_index, uint32 index, cache_mem_test_write_value_pattern_t pattern)
{
    switch (pattern)
    {
        case    cache_mem_test_write_value_pattern_all_ones:
            return 0xFFFFFFFF;
        case    cache_mem_test_write_value_pattern_all_zeroes:
            return 0;
        case    cache_mem_test_write_value_pattern_incremental:
            return index;
        case    cache_mem_test_write_value_pattern_smart:
            return (unit + mem + array_index + index);
    }
    return 0;
}

void cache_mem_test_create_mask(
        SOC_SAND_IN  uint32                 unit,
        SOC_SAND_IN  soc_mem_t              mem,
        SOC_SAND_IN  size_t                 mask_len,
        SOC_SAND_OUT uint32                 *mask)
{
    int                     i;
    uint32                  entry_dw = soc_mem_entry_words(unit, mem);
    soc_mem_info_t          *meminfo;
    sal_memset(mask, 0, mask_len);

    meminfo         = &SOC_MEM_INFO(unit, mem);

    for ( i = 0; i < meminfo->nFields; i++)
    {
        /* Skip the Read-only/Write-Only/Signal/Reserved fields */
        if ((meminfo->fields[i].flags & SOCF_RO) ||  (meminfo->fields[i].flags & SOCF_WO) || (meminfo->fields[i].flags & SOCF_SIG) || (meminfo->fields[i].flags & SOCF_RES) )
            continue;
        SHR_BITSET_RANGE(mask, meminfo->fields[i].bp, meminfo->fields[i].len);
    }

    /* Print the Mask */
    LOG_DEBUG(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: Mask for memory %d %s, nFields %d mask 0x"),
                        mem, SOC_MEM_NAME(unit, mem), meminfo->nFields));

    for ( i = entry_dw - 1; i >= 0; i--)
    {
        LOG_DEBUG(BSL_LS_APPL_TESTS, (BSL_META("%X"), mask[i]));
    }

}

uint32 cache_mem_test_fill_values_soc_mem_write_callback(uint32 unit, soc_mem_t mem, uint32     i_array, tr_do_cache_mem_test_t *test_params)
{
    uint32      start_index, end_index, index;
    uint32      entry_index = 0;
    uint32      write_value[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32      mem_field_mask[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32                  entry_dw = soc_mem_entry_words(unit, mem);
    soc_error_t             rv = SOC_E_NONE;
    soc_mem_t               mem_orig = mem;
    SOC_MEM_ALIAS_TO_ORIG(unit, mem_orig);

    start_index     = parse_memory_index(unit, mem, "min");
    end_index       = parse_memory_index(unit, mem, "max");

    cache_mem_test_create_mask(unit, mem, sizeof(mem_field_mask), mem_field_mask);

    LOG_INFO(BSL_LS_APPL_TESTS,
                (BSL_META("CACHE_MEM_TEST: WRITE SOC: mem %d %s, num_of_entries %d, entry_dw %d\n"),
                         mem, SOC_MEM_NAME(unit, mem), end_index - start_index + 1/*table_size*/, entry_dw));

    for (index = start_index; index <= end_index; index++ )
    {
        for (entry_index = 0; entry_index < entry_dw; entry_index++)
        {
            write_value[entry_index]  = cache_mem_test_generate_value(unit, mem_orig, i_array, index, test_params->write_value_pattern) & mem_field_mask[entry_index];
        }
        rv = soc_mem_array_write(unit, mem, i_array, MEM_BLOCK_ALL, index, write_value);
    }

    if (rv != SOC_E_NONE)
        test_params->result |= rv;

    return rv;
}



int cache_mem_test_fill_values_dma_callback(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int copyno,
    SOC_SAND_IN int array_index,
    SOC_SAND_IN int index,
    SOC_SAND_OUT uint32 *value,
    SOC_SAND_IN int entry_sz,
    SOC_SAND_IN void *opaque)
{
    uint32          i               = 0;
    uint32          entry_size      = entry_sz;
    uint32          mem_field_mask[SOC_MEM_ENTRY_LENGTH_MAX];
    tr_do_cache_mem_test_t *test_params= ((tr_do_cache_mem_test_t *)opaque);
    soc_mem_t       mem             = test_params->mem_id;
    soc_mem_t       mem_orig        = mem;
    SOC_MEM_ALIAS_TO_ORIG(unit, mem_orig);
    *value = 0;

    cache_mem_test_create_mask(unit, mem, sizeof(mem_field_mask), mem_field_mask);

    for (i = 0; i < entry_size; i++)
    {
        value[i] = cache_mem_test_generate_value(unit, mem_orig, array_index, index, test_params->write_value_pattern) & mem_field_mask[i];
        LOG_DEBUG(BSL_LS_APPL_TESTS,
                    (BSL_META("CACHE_MEM_TEST: WRITE DMA: mem  %d %s i %d value 0x%X mask 0x%X \n"),
                            mem, SOC_MEM_NAME(unit, mem), i, value[i], mem_field_mask[i]));

    }

    return SOC_E_NONE;
}

int
cache_mem_test_write_iter_callback(int unit, soc_mem_t mem, void* data)
{
    uint32      rv = SOC_E_NONE;
    uint32      start_array_index = 0, end_array_index = 0;
    uint32      i_array;
    tr_do_cache_mem_test_t *test_params = (tr_do_cache_mem_test_t *)data;

    test_params->stat_mem_total_cnt++;

#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit))
    {
        if(!soc_mem_is_cachable(unit, mem))
        {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: WRITE: non-cachable memory %d %s\n"), mem, SOC_MEM_NAME(unit, mem)));
            test_params->stat_mem_not_tested_cnt++;
            return rv;
        }
    }
#endif

    if (SOC_MEM_IS_ARRAY(unit,mem))
    {
        start_array_index = parse_memory_array_index(unit, mem, "min");
        end_array_index = parse_memory_array_index(unit, mem, "max");
    }

    /*The following statement will be removed after SDK-91354 will be fixed.*/
    if (mem == EDB_LINK_LAYER_OR_ARP_NEW_FORMATm || mem == EDB_PROTECTION_ENTRYm || mem == EDB_TRILL_FORMAT_FULL_ENTRYm || mem == EDB_TRILL_FORMAT_HALF_ENTRYm || mem == KAPS_TCMm || mem == SER_ACC_TYPE_MAPm)
    {
        return 0;
    }

#ifdef BCM_PETRA_SUPPORT
    if( SOC_MEM_IS_VALID(unit, SOC_MEM_ALIAS_MAP(unit, mem))) {
	    return 0;
	}

    
    if (mem == KAPS_RPB_TCAM_CPU_COMMANDm ||
        mem == KAPS_RPB_TCAM_HIT_INDICATIONm ||
        mem == OAMP_FLEX_VER_MASK_TEMPm ||
        mem == PPDB_B_LARGE_EM_FORMAT_0_TYPE_0m ||
        mem == SCH_FORCE_STATUS_MESSAGEm ||
        mem == SCH_SCHEDULER_INITm)
    {
        return 0;
    }

    
    if(test_params->write_type == cache_mem_test_write_type_dma &&
	   soc_mem_entry_words(unit, mem) + 2 > CMIC_SCHAN_WORDS(unit)) {
        return 0;
    }
#endif

    switch (test_params->write_type)
    {
        case cache_mem_test_write_type_dma:
        {
            for (i_array = start_array_index; i_array <= end_array_index; i_array++)
            {
                LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: WRITE DMA: write memory %d %s \n"), mem, SOC_MEM_NAME(unit, mem)));
                test_params->mem_id = mem;
#ifdef BCM_PETRA_SUPPORT
                /* DMA write via caching currently Implemented only in Arad.*/
                rv = arad_fill_table_with_variable_values_by_caching(unit, mem, i_array, MEM_BLOCK_ALL, -1, -1,
                        cache_mem_test_fill_values_dma_callback, test_params);
#else
                /* Fill the values in the non-dma way - as we do for schan writes */
                
                rv = cache_mem_test_fill_values_soc_mem_write_callback( unit, mem, i_array, data);
#endif
            }
            break;
        }
        case cache_mem_test_write_type_schan:
        {
            for (i_array = start_array_index; i_array <= end_array_index; i_array++)
            {
                rv = cache_mem_test_fill_values_soc_mem_write_callback( unit, mem, i_array, data);
            }
        }
    }

    if (rv != SOC_E_NONE) {
        test_params->stat_mem_fail_cnt++;
        test_params->result |= rv;
    } else {
        test_params->stat_mem_succeed_cnt++;
    }

    return rv;
}


int
cache_mem_test_read_and_compare_wo_expected(int unit, soc_mem_t mem, void* data)
{
    soc_error_t rv = SOC_E_NONE;
    uint32      start_array_index = 0, end_array_index = 0, i_array;
    uint32      start_index, end_index, index;
    uint32      entry_index = 0;
    uint32      read_value[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32      read_cache_value[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32      mem_field_mask[SOC_MEM_ENTRY_LENGTH_MAX];
    tr_do_cache_mem_test_t *test_params = (tr_do_cache_mem_test_t *)data;
    uint32                  entry_dw;

    SOC_MEM_ALIAS_TO_ORIG(unit, mem);

    test_params->stat_mem_total_cnt++;

#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit))
    {
        if(!soc_mem_is_cachable(unit, mem))
        {
            /* skip memory*/
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ:  non-cachable memory %d %s\n"), mem, SOC_MEM_NAME(unit, mem)));
            test_params->stat_mem_not_tested_cnt++;
            test_params->stat_mem_succeed_cnt--;
            goto done;
        }
        if (!soc_mem_cache_get(unit, mem, MEM_BLOCK_ALL)) {
            test_params->stat_mem_not_tested_cnt++;
            test_params->stat_mem_succeed_cnt--;
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ: uncached memory %d %s\n"), mem, SOC_MEM_NAME(unit, mem)));
            goto done;
        }
    }
#endif
    entry_dw        = soc_mem_entry_words(unit, mem);
    start_index     = parse_memory_index(unit, mem, "min");
    end_index       = parse_memory_index(unit, mem, "max");
    cache_mem_test_create_mask(unit, mem, sizeof(mem_field_mask), mem_field_mask);

    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ read memory %d %s, num_of_entries %d  \n"),
             mem, SOC_MEM_NAME(unit, mem), end_index - start_index + 1/*table_size*/));

    if (SOC_MEM_IS_ARRAY(unit,mem)) {
        start_array_index = parse_memory_array_index(unit, mem, "min");
        end_array_index = parse_memory_array_index(unit, mem, "max");
    }

    /*The following statement will be removed after SDK-91354 will be fixed.*/
    if (mem == EDB_LINK_LAYER_OR_ARP_NEW_FORMATm || mem == EDB_PROTECTION_ENTRYm || mem == EDB_TRILL_FORMAT_FULL_ENTRYm || mem == EDB_TRILL_FORMAT_HALF_ENTRYm || mem == KAPS_TCMm || mem == SER_ACC_TYPE_MAPm)
    {
        return 0;
    }

    for (i_array = start_array_index; i_array <= end_array_index; i_array++)
    {
        for (index = start_index; index <= end_index; index++ )
        {
            rv = soc_mem_array_read(unit, mem, i_array, MEM_BLOCK_ANY, index, read_cache_value);
            rv |= soc_mem_array_read_flags(unit, mem, i_array, MEM_BLOCK_ANY, index, read_value, SOC_MEM_DONT_USE_CACHE);
            if (rv != SOC_E_NONE)
            {
                LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST:Read FAILED rv %d: read_value 0x%X cache_value 0x%X mask 0x%X mem %d %s, index %d, array %d\n"),
                                        rv, read_value[entry_index], read_cache_value[entry_index], mem_field_mask[entry_index], mem, SOC_MEM_NAME(unit, mem), index, i_array));
                goto done;
            }

            for (entry_index = 0; entry_index < entry_dw; entry_index++)
            {
                if ((read_value[entry_index] & mem_field_mask[entry_index]) == (read_cache_value[entry_index] & mem_field_mask[entry_index]))
                {
                    LOG_DEBUG(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ: read_value 0x%X cache_value 0x%X, mask 0x%X, mem %d, index %d, entry_index, %d array %d, len %d\n"),
                            read_value[entry_index] & mem_field_mask[entry_index],
                            read_cache_value[entry_index] & mem_field_mask[entry_index],
                            mem_field_mask[entry_index],
                            mem, index, entry_index, i_array, entry_dw));
                }
                else
                {
                    LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: COMPARE FAILED: read_value 0x%X, cache_value 0x%X, (0x%X != 0x%X), mask 0x%X mem %d, index %d, entry_index %d array %d\n"),
                            read_value[entry_index] & mem_field_mask[entry_index],
                            read_cache_value[entry_index] & mem_field_mask[entry_index],
                            read_value[entry_index], read_cache_value[entry_index],
                            mem_field_mask[entry_index],
                            mem, index, entry_index, i_array));
                    rv = SOC_E_LIMIT /* Unknown error */;
                    goto done;
                }
            }
        }
    }

done:
    if (rv != SOC_E_NONE) {
        test_params->stat_mem_fail_cnt++;
        test_params->result |= rv;
    } else {
        test_params->stat_mem_succeed_cnt++;
    }

    return rv;

}

int
cache_mem_test_read_and_compare(SOC_SAND_IN int unit, SOC_SAND_IN soc_mem_t mem, void* data)
{
    soc_error_t rv = SOC_E_NONE;
    uint32      start_array_index = 0, end_array_index = 0, i_array;
    uint32      start_index, end_index, index;
    uint32      entry_index = 0;
    uint32      read_value[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32      read_cache_value[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32      expected_value[SOC_MEM_ENTRY_LENGTH_MAX];
    uint32      mem_field_mask[SOC_MEM_ENTRY_LENGTH_MAX];
    tr_do_cache_mem_test_t *test_params = (tr_do_cache_mem_test_t *)data;
    uint32                  entry_dw;
    soc_mem_t               mem_orig = mem;
    SOC_MEM_ALIAS_TO_ORIG(unit, mem_orig);

    SOC_REG_ABOVE_64_CLEAR(read_value);
    SOC_REG_ABOVE_64_CLEAR(read_cache_value);
    SOC_REG_ABOVE_64_CLEAR(expected_value);
    SOC_REG_ABOVE_64_CLEAR(mem_field_mask);

    if (test_params->test_part != cache_mem_test_full) {
        test_params->stat_mem_total_cnt++;
    }

    /*The following statement will be removed after SDK-91354 will be fixed.*/
    if (mem == EDB_LINK_LAYER_OR_ARP_NEW_FORMATm || mem == EDB_PROTECTION_ENTRYm || mem == EDB_TRILL_FORMAT_FULL_ENTRYm || mem == EDB_TRILL_FORMAT_HALF_ENTRYm || mem == KAPS_TCMm || mem == SER_ACC_TYPE_MAPm)
    {
        return 0;
    }

#ifdef BCM_PETRA_SUPPORT
    if( SOC_MEM_IS_VALID(unit, SOC_MEM_ALIAS_MAP(unit, mem))) {
	    return 0;
	}

    
    if (mem == KAPS_RPB_TCAM_CPU_COMMANDm ||
        mem == KAPS_RPB_TCAM_HIT_INDICATIONm ||
        mem == OAMP_FLEX_VER_MASK_TEMPm ||
        mem == PPDB_B_LARGE_EM_FORMAT_0_TYPE_0m ||
        mem == SCH_FORCE_STATUS_MESSAGEm ||
        mem == SCH_SCHEDULER_INITm)
    {
        return 0;
    }

    
    if(test_params->write_type == cache_mem_test_write_type_dma &&
	   soc_mem_entry_words(unit, mem) + 2 > CMIC_SCHAN_WORDS(unit)) {
        return 0;
    }
#endif

#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit))
    {
        if(!soc_mem_is_cachable(unit, mem))
        {
            /* skip memory*/
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ: uncachable memory %d %s\n"), mem, SOC_MEM_NAME(unit, mem)));
            if (test_params->test_part != cache_mem_test_full) {
                test_params->stat_mem_not_tested_cnt++;
                test_params->stat_mem_succeed_cnt--;
            }
            goto done;
        }

        if (!soc_mem_cache_get(unit, mem, MEM_BLOCK_ALL)) {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ: uncached memory %d %s\n"), mem, SOC_MEM_NAME(unit, mem)));
            if (test_params->test_part != cache_mem_test_full) {
                test_params->stat_mem_not_tested_cnt++;
                test_params->stat_mem_succeed_cnt--;
            }
            goto done;
        }
    }
#endif
    entry_dw        = soc_mem_entry_words(unit, mem);
    start_index     = parse_memory_index(unit, mem, "min");
    end_index       = parse_memory_index(unit, mem, "max");
    cache_mem_test_create_mask(unit, mem, sizeof(mem_field_mask), mem_field_mask);

    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ read memory %d %s, num_of_entries %d  \n"),
             mem, SOC_MEM_NAME(unit, mem), end_index - start_index + 1/*table_size*/));

    if (SOC_MEM_IS_ARRAY(unit,mem)) {
        start_array_index = parse_memory_array_index(unit, mem, "min");
        end_array_index = parse_memory_array_index(unit, mem, "max");
    }

    for (i_array = start_array_index; i_array <= end_array_index; i_array++)
    {
        for (index = start_index; index <= end_index; index++ )
        {
            rv = soc_mem_array_read(unit, mem, i_array, MEM_BLOCK_ANY, index, read_cache_value);
            rv |= soc_mem_array_read_flags(unit, mem, i_array, MEM_BLOCK_ANY, index, read_value, SOC_MEM_DONT_USE_CACHE);
            if (rv != SOC_E_NONE)
            {
                LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST:Read FAILED rv %d: read_value 0x%X cache_value 0x%X mask 0x%X mem %d %s, index %d, array %d\n"),
                                        rv, read_value[entry_index], read_cache_value[entry_index], mem_field_mask[entry_index], mem, SOC_MEM_NAME(unit, mem), index, i_array));
                goto done;
            }

            for (entry_index = 0; entry_index < entry_dw; entry_index++)
            {
                expected_value[entry_index] = cache_mem_test_generate_value(unit,  mem_orig, i_array, index, test_params->write_value_pattern) & mem_field_mask[entry_index];

                if ((read_value[entry_index] & mem_field_mask[entry_index] ) == (expected_value[entry_index] & mem_field_mask[entry_index]) &&
                        (read_cache_value[entry_index] & mem_field_mask[entry_index] ) == (expected_value[entry_index] & mem_field_mask[entry_index]))
                {
                    LOG_DEBUG(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: READ: read_value 0x%X cache_value 0x%X == expected_value 0x%X, (0x%X == 0x%X), mask 0x%X, mem %d, index %d, entry_index, %d array %d, len %d\n"),
                            read_value[entry_index] & mem_field_mask[entry_index],
                            read_cache_value[entry_index] & mem_field_mask[entry_index],
                            expected_value[entry_index] & mem_field_mask[entry_index],
                            read_value[entry_index], expected_value[entry_index],
                            mem_field_mask[entry_index],
                            mem, index, entry_index, i_array, entry_dw));
                }
                else
                {
                    LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: COMPARE FAILED: read_value 0x%X, cache_value 0x%X, expected_value 0x%X, (0x%X != 0x%X != 0x%X), mask 0x%X mem %d, index %d, entry_index %d array %d\n"),
                            read_value[entry_index] & mem_field_mask[entry_index],
                            read_cache_value[entry_index] & mem_field_mask[entry_index],
                            expected_value[entry_index] & mem_field_mask[entry_index],
                            read_value[entry_index], read_cache_value[entry_index], expected_value[entry_index],
                            mem_field_mask[entry_index],
                            mem, index, entry_index, i_array));
                    rv = SOC_E_LIMIT /* Unknown error */;
                    goto done;
                }
            }
        }
    }
done:
    if (rv != SOC_E_NONE) {
        test_params->stat_mem_fail_cnt++;
        test_params->stat_mem_succeed_cnt--;
        test_params->result |= rv;
    } else {
        if (test_params->test_part != cache_mem_test_full) {
            test_params->stat_mem_succeed_cnt++;
        }
    }

    return rv;

}

int
cache_mem_test_write_read_and_compare(SOC_SAND_IN int unit, SOC_SAND_IN soc_mem_t mem, void* data) {

    if (cache_mem_test_write_iter_callback(unit, mem, data) < 0) {
        return BCM_E_INTERNAL;
    }
    if (cache_mem_test_read_and_compare(unit, mem, data) < 0) {
        return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}

void do_cache_mem_test_print_usage(void)
{
    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("Usage for Cache Memory Test: \n"
            "test_type=X         where X=0 for specific memory, X=1 for all memories\n"
            "write_type=X        where X=0 for DMA write,       X=1 for SCHAN write\n"
            "pattern=X           where X=0 for All-Ones,  X=1 for All-Zeroes,      X=2 for Incremental,    X=3 for Smart pattern\n"
            "part=X              where X=0 for Full-Test, X=1 for Only-Write-Part, X=2 for Only-Read-Part, X=3 for Cache VS HW test\n")));
    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("mem_id=X            where X is memory id for specific memory test (applicable with test_type=0)\n")));
}

int
do_cache_mem_test_init(int unit, args_t *a, void **p)
{

    cmd_result_t            rv      = CMD_OK;
    tr_do_cache_mem_test_t  *test_params;
    char                    *tab;
    int                     num = 0;
    cache_mem_test_partial_t test_part;

    *p = NULL;

    if ((test_params = sal_alloc(sizeof (tr_do_cache_mem_test_t), "cache_mem_test")) == 0)
    {
        rv = SOC_E_MEMORY;
        goto done;
    }
    memset(test_params, 0, sizeof (*test_params));

    *p = (void *) test_params;

    /* Set default params */
    test_params->test_type                  = cache_mem_test_type_all_mems;
    test_params->write_type                 = cache_mem_test_write_type_schan;
    test_params->write_value_pattern        = cache_mem_test_write_value_pattern_incremental;
    test_params->test_part                  = cache_mem_test_full;
    test_params->mem_id                     = 0;
    test_params->result                     = CMD_OK;

    if ((tab = ARG_GET(a)) == NULL) {
        goto usage;
    }

    /* Get Test Type */
    tab = sal_strstr(tab, "test_type");
    if (tab != NULL)
    {
        num = sal_ctoi(tab + 10/*'test_type=' is 10 chars*/, NULL);
        if (num <= cache_mem_test_type_all_mems && num >= cache_mem_test_type_single )
        {
            test_params->test_type = num;
        }
        else goto usage;
    }
    if ((tab = ARG_GET(a)) == NULL) {
        goto usage;
    }

    /* Get Write Type */
    tab = sal_strstr(tab, "write_type");
    if (tab != NULL)
    {
        num = sal_ctoi(tab + 11/*'write_type=' is 11 chars*/, NULL);
        if (num <= cache_mem_test_write_type_schan && num >= cache_mem_test_write_type_dma)
        {
            test_params->write_type = num;
        }
        else goto usage;

    }
    if ((tab = ARG_GET(a)) == NULL) {
        goto usage;
    }


    /* Get Pattern */
    tab = sal_strstr(tab, "pattern");
    if (tab != NULL)
    {
        cache_mem_test_write_value_pattern_t write_value_pattern;
        write_value_pattern = sal_ctoi(tab + 8/*'pattern=' is 8 chars*/, NULL);
        if ((int)write_value_pattern <= cache_mem_test_write_value_pattern_smart && write_value_pattern >= cache_mem_test_write_value_pattern_all_ones )
        {
            test_params->write_value_pattern = write_value_pattern;
        }
        else goto usage;

    }
    if ((tab = ARG_GET(a)) == NULL) {
        goto usage;
    }

    /* Get Test partial */
    tab = sal_strstr(tab, "part");
    if (tab != NULL)
    {
        num = sal_ctoi(tab + 5/*'part=' is 5 chars*/, NULL);
        test_part = (cache_mem_test_partial_t)num;
        if (test_part <= cache_mem_test_cache_only && test_part >= cache_mem_test_full)
        {
            test_params->test_part = test_part;
        }
        else goto usage;

    }

    if ((tab = ARG_GET(a)) == NULL) {
        goto usage;
    }

    /* Get Memory ID */
    tab = sal_strstr(tab, "mem_id");
    if (tab != NULL)
    {
        num = sal_ctoi(tab + 7/*'mem_id=' is 7 chars*/, NULL);
        if (num <= NUM_SOC_MEM && num >= 0)
        {
            test_params->mem_id = num;
        }
        else goto usage;
    }


#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit))
    {
        bcm_common_linkscan_enable_set(unit,0);
        soc_counter_stop(unit);
        rv = bcm_dpp_counter_bg_enable_set(unit, FALSE);
        if (CMD_OK == rv)
        {
            LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST:unit %d counter processor background accesses suspended\n"), unit));
        }
        else
        {
            LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META("CACHE_MEM_TEST: unit %d counter processor background access suspend failed: %d (%s)\n"),
                    unit, rv, _SHR_ERRMSG(rv)));
        }

        if ((rv |= soc_dpp_device_reset(unit, SOC_DPP_RESET_MODE_REG_ACCESS, SOC_DPP_RESET_ACTION_INOUT_RESET)) < 0)
        {
            LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "CACHE_MEM_TEST: unit %d ERROR: Unable to reinit  \n"), unit));
        }
		goto done;
    }
#endif

usage:
	do_cache_mem_test_print_usage();
    rv = CMD_USAGE;

done:
    if (rv != CMD_OK && test_params != NULL)
    	if (test_params)
    		test_params->result |= rv;

    return rv;
}



int
do_cache_mem_test_done(int unit,  void *p)
{
    soc_error_t rv;
    tr_do_cache_mem_test_t  *test_params = (tr_do_cache_mem_test_t*)p;

    rv = test_params->result;

    sal_free (p);

    LOG_INFO(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "CACHE_MEM_TEST: unit %d Done: Total %u, Failed %u, Not tested %u, Succeeded %u\n"), unit, test_params->stat_mem_total_cnt, test_params->stat_mem_fail_cnt, test_params->stat_mem_not_tested_cnt, test_params->stat_mem_succeed_cnt));
    return rv;
}


int
do_cache_mem_test(int unit,  args_t *a, void* tr_do_cache_mem_test)
{
    soc_error_t rv = BCM_E_NONE;
    tr_do_cache_mem_test_t *test_params = tr_do_cache_mem_test;
#ifdef BCM_PETRA_SUPPORT
    uint32                  cache_enable = 1;
#endif


    switch(test_params->test_type)
    {
        case cache_mem_test_type_all_mems:  /* Run tests for all memories */
        {

#ifdef BCM_PETRA_SUPPORT
            /* Turn on cache memory for all tables */
            if (test_params->test_part == cache_mem_test_full || test_params->test_part == cache_mem_test_write_only)
            {
                if (soc_mem_iterate(unit, arad_tbl_mem_cache_mem_set, &cache_enable) < 0)
                    LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "CACHE_MEM_TEST: unit %d cache enable failed\n"), unit));
            }
#endif

            /* Do the write/read part of the test */
            if (test_params->test_part == cache_mem_test_full)
            {
                if ((soc_mem_iterate(unit, cache_mem_test_write_read_and_compare, tr_do_cache_mem_test)) < 0) {
                    rv = BCM_E_INTERNAL;
                }
                if (test_params->result) {
                    break;
                }
            }
            if (test_params->test_part == cache_mem_test_write_only)
            {
                if ((soc_mem_iterate(unit, cache_mem_test_write_iter_callback, tr_do_cache_mem_test)) < 0) {
                    rv = BCM_E_INTERNAL;
                }
                if (test_params->result) {
                    break;
                }
            }
            /* Do the read part of the test */
            if (test_params->test_part == cache_mem_test_read_only)
            {
                if ((soc_mem_iterate(unit, cache_mem_test_read_and_compare, tr_do_cache_mem_test)) < 0) {
                    rv = BCM_E_INTERNAL;
                }
                if (test_params->result) {
                    break;
                }
            }
            /* Do the HW vs Cache part of the test */
            if (test_params->test_part == cache_mem_test_cache_only)
            {
                if ((soc_mem_iterate(unit, cache_mem_test_read_and_compare_wo_expected, tr_do_cache_mem_test)) < 0) {
                    rv = BCM_E_INTERNAL;
                }
                if (test_params->result) {
                    break;
                }
            }

#ifdef BCM_PETRA_SUPPORT
            /* Turn off cache memory for all tables */
            cache_enable = 0;
            if (test_params->test_part == cache_mem_test_full || test_params->test_part == cache_mem_test_read_only)
            {
                if (soc_mem_iterate(unit, arad_tbl_mem_cache_mem_set, &cache_enable) < 0)
                    LOG_ERROR(BSL_LS_APPL_TESTS, (BSL_META_U(unit, "CACHE_MEM_TEST: unit %d cache disable failed\n"), unit));
            }
#endif
            break;

        }
        case cache_mem_test_type_single:    /* Run test for specific memory */
        {
#ifdef BCM_PETRA_SUPPORT
            /* Turn on cache memory for all tables */
            arad_tbl_mem_cache_mem_set(unit, test_params->mem_id, &cache_enable);
#endif
            /* Do the write part of the test */
            if (test_params->test_part == cache_mem_test_full || test_params->test_part == cache_mem_test_write_only)
            {
                rv = cache_mem_test_write_iter_callback(unit, test_params->mem_id, tr_do_cache_mem_test);
                if (rv) {
                    break;
                }
            }
            /* Do the read part of the test */
            if (test_params->test_part == cache_mem_test_full || test_params->test_part == cache_mem_test_read_only)
            {
                rv = cache_mem_test_read_and_compare(unit, test_params->mem_id, tr_do_cache_mem_test);
                if (rv) {
                    break;
                }
            }
            /* Do the HW vs Cache part of the test */
            if (test_params->test_part == cache_mem_test_cache_only)
            {
                rv = cache_mem_test_read_and_compare_wo_expected(unit, test_params->mem_id, tr_do_cache_mem_test);
                if (rv) {
                    break;
                }
            }
#ifdef BCM_PETRA_SUPPORT
            /* Turn off cache memory for all tables */
            cache_enable = 0;
            arad_tbl_mem_cache_mem_set(unit, test_params->mem_id, &cache_enable);
#endif
            break;
        }
    }


    if (rv != SOC_E_NONE)
        test_params->result |= rv;

    return rv;
}

int soc_mem_single_test(int unit, soc_mem_t mem)
{

	tr_do_cache_mem_test_t test_params;
	test_params.mem_id = mem;
	test_params.test_type = cache_mem_test_type_single;
	test_params.write_type = cache_mem_test_write_type_schan;
	test_params.write_value_pattern = cache_mem_test_write_value_pattern_smart;
	test_params.test_part = cache_mem_test_full;

	do_cache_mem_test(unit,  NULL, &test_params);

    return 0;

}
#endif /*(BCM_ESW_SUPPORT) || (BCM_PETRA_SUPPORT) || (BCM_DFE_SUPPORT) */
