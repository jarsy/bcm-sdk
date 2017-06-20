/* $Id: tmu_hash_ut.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/defs.h>

#if defined(BCM_CALADAN3_SUPPORT)

#include "../c3sw_test.h"
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/hash.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/sbx/caladan3/hpcm.h>
#include <shared/alloc.h>
#include <sal/core/libc.h>
#include <sal/core/time.h>
#include <soc/sbx/caladan3/lrp.h>
#include <appl/test/caladan3/c3hppc_lrp.h>

/*#define TMU_HASH_UT_DEBUG*/
soc_field_t fifo_aempty[] = {FREE_CHAIN_FIFO0_AEMPTYf, FREE_CHAIN_FIFO1_AEMPTYf, 
                             FREE_CHAIN_FIFO2_AEMPTYf, FREE_CHAIN_FIFO3_AEMPTYf};

/* unit test helpers */
extern int soc_sbx_tmu_hash_ut_fake_chain_recyle(int unit, int fifoid, int num_entries);
extern int soc_sbx_tmu_hash_ut_fake_chain_free_fifo(int unit, int fifoid, int num_entries, uint8 drain); 
extern int soc_sbx_tmu_hash_ut_recyle_entry_count(int unit, int fifoid, int *num_entries);
extern void soc_sbx_tmu_hash_ut_reset_recycle_ring(int unit);
    extern int soc_sbx_tmu_hw_chain_entry_dump(int unit, 
                                    soc_sbx_tmu_hash_handle_t handle,
                                               uint32 chain_idx, uint32 table_id);

static int tmu_hash_ut_test_result;


/*********************/
/** HPCM unit tests **/
/*********************/
int hpcm_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    soc_heap_mem_chunk_t  *hpcm;
#define HPCM_TEST_CHUNK_SIZE (900)
    soc_heap_mem_elem_t *hpcm_elem[HPCM_TEST_CHUNK_SIZE];

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    unsigned int res0_curr, res0_max;
    unsigned int res1_curr, res1_max;

    sal_alloc_resource_usage_get(&res0_curr, &res0_max);
#ifdef TMU_HASH_UT_DEBUG
    cli_out("\n mem initial curr(%d)/max(%d)\n", res0_curr, res0_max);
#endif
#endif
    /* allocate a hpcm chunk */
    rv = hpcm_init(unit, HPCM_TEST_CHUNK_SIZE, sizeof(int)*4, &hpcm);

    for (index=0; index < HPCM_TEST_CHUNK_SIZE && SOC_SUCCESS(rv); index++) {
        assert(hpcm->alloc_count == index);
        assert(hpcm->free_count == HPCM_TEST_CHUNK_SIZE - index);
        rv = hpcm_alloc(unit, hpcm, &hpcm_elem[index]);
    }

    if (SOC_SUCCESS(rv)) {
        assert(hpcm_empty(unit, hpcm));
    }

    for (index=0; index < HPCM_TEST_CHUNK_SIZE && SOC_SUCCESS(rv); index++) {
        assert(hpcm->free_count == index);
        assert(hpcm->alloc_count == HPCM_TEST_CHUNK_SIZE - index);
        hpcm_free(unit, hpcm_elem[index]);
    }

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
#ifdef TMU_HASH_UT_DEBUG
    sal_alloc_resource_usage_get(&res1_curr, &res1_max);
    cli_out(" mem hpcm curr(%d)/max(%d) - alloc chunk bytes(%d)\n", res1_curr, res1_max, sizeof(int)*4);
    cli_out(" total space allocated for hpcm = %d \n", res1_curr - res0_curr);
#endif 
#endif 
    rv = hpcm_destroy(unit, hpcm);

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&res1_curr, &res1_max);
#ifdef TMU_HASH_UT_DEBUG
    cli_out(" total space after hpcm free curr(%d):initial(%d):max(%d)\n", res1_curr, res0_curr, res1_max);
#endif
    assert(res1_curr == res0_curr); /* check for leaks */
#endif
    return rv;
}


/*********************/
/** EML unit tests **/
/*********************/
typedef struct _hash_table_param_s {
    soc_sbx_tmu_hash_param_t param;
    soc_sbx_tmu_hash_handle_t handle;
    unsigned int mem_begin, mem_end, mem_max;
} hash_table_param_t; 

static hash_table_param_t hash_test_param[2];

int eml_table_init(int unit, void *data)
{
    int rv=SOC_E_NONE;
    soc_sbx_tmu_hash_param_t *param;

    sal_memset(&hash_test_param[0],0,sizeof(hash_table_param_t));

    param = &hash_test_param[0].param;
    param->capacity = /*16000; */ 1048576;
    param->key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS;
    param->chain_length = 6;
    param->num_hash_table_entries = /*16 * 1024; */ 4194304;
    param->num_chain_table_entries = /*4096;*/ 262144;

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&hash_test_param[0].mem_begin, &hash_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", hash_test_param[0].mem_begin, hash_test_param[0].mem_max);
#endif
    rv =  soc_sbx_caladan3_tmu_hash_table_alloc(unit, param, &hash_test_param[0].handle);
    tmu_hash_ut_test_result = 0;


    return rv;
}

int eml_table_dual_64_init(int unit, void *data)
{
    int rv=SOC_E_NONE;
    soc_sbx_tmu_hash_param_t *param;

    sal_memset(&hash_test_param[0],0,sizeof(hash_table_param_t));

    param = &hash_test_param[0].param;
    param->capacity = 1048576;
    param->key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS;
    param->chain_length = 6;
    param->num_hash_table_entries = 4194304;;
    param->num_chain_table_entries = 262144;

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&hash_test_param[0].mem_begin, &hash_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", hash_test_param[0].mem_begin, hash_test_param[0].mem_max);
#endif
    rv =  soc_sbx_caladan3_tmu_hash_table_alloc(unit, param, &hash_test_param[0].handle);
    tmu_hash_ut_test_result = 0;


    return rv;
}

int eml_table_dual_144_init(int unit, void *data)
{
    int rv=SOC_E_NONE;
    soc_sbx_tmu_hash_param_t *param;

    sal_memset(&hash_test_param[0],0,sizeof(hash_table_param_t));

    param = &hash_test_param[0].param;
    param->capacity = 1048576;
    param->key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS;
    param->chain_length = 6;
    param->num_hash_table_entries = 4194304;;
    param->num_chain_table_entries = 262144;

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&hash_test_param[0].mem_begin, &hash_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", hash_test_param[0].mem_begin, hash_test_param[0].mem_max);
#endif
    rv =  soc_sbx_caladan3_tmu_hash_table_alloc(unit, param, &hash_test_param[0].handle);
    tmu_hash_ut_test_result = 0;


    return rv;
}

int eml_hash_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x00000000, 0x11223344};
    uint32 chain_key[] = {0x00000000, 0xaabbccdd};
    uint32 value[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0xdeadbeef};
    uint32 read_value[] = {0, 0, 0, 0};

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                    &key[0], &value[0], FALSE);
    }

    if (SOC_SUCCESS(rv)) {
        /* verify added value */
        rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
        if (SOC_SUCCESS(rv)) {
            for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                if (value[index] != read_value[index]) {
                    cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                            read_value[index], value[index], index);
                    return SOC_E_FAIL;
                }
            }

            /* try to reinsert same key expect errors */
            rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
            if (SOC_SUCCESS(rv)) {
                cli_out("ERROR: Allowed duplicate key insertion !!!!\n");
                rv = SOC_E_FAIL;
            } else {
                rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle,
                                                         &chain_key[0], &value[0]);     
                if (SOC_SUCCESS(rv)) {    
                    cli_out("######### Chain entry verification ############\n");
                    rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                                &chain_key[0], &value[0], FALSE);
                }
                if (SOC_SUCCESS(rv)) {
                    rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle,
                                                             &chain_key[0], &read_value[0]);
                    if (SOC_SUCCESS(rv)) {
                        for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                            if (value[index] != read_value[index]) {
                                cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                        read_value[index], value[index], index);
                                return SOC_E_FAIL;
                            }
                        }
                        
                        rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle,
                                                                 &chain_key[0], &value[0]);
                        if (SOC_SUCCESS(rv)) {
                            cli_out("ERROR: Allowed duplicate key insertion !!!!\n");
                            rv = SOC_E_FAIL;
                        } else {
                            rv = SOC_E_NONE;
                        }
                    }
                }           
            }

            if (SOC_SUCCESS(rv)) {
                /* delete the bucket node & see if chain becomes the new bucket node */
                rv = soc_sbx_caladan3_tmu_hash_entry_delete(unit, hash_test_param[0].handle, &key[0]);
                if (SOC_SUCCESS(rv)) {
                    rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
                    if (SOC_SUCCESS(rv)) {
                        cli_out("ERROR: bucket key not yet deleted !!!!\n");
                        rv = SOC_E_FAIL;
                    } else {
                        sal_memset(&read_value[0], 0, sizeof(read_value));
                        rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle,
                                                                 &chain_key[0], &read_value[0]);
                        if (SOC_SUCCESS(rv)) {
                            for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                                if (value[index] != read_value[index]) {
                                    cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                            read_value[index], value[index], index);
                                    return SOC_E_FAIL;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}

int eml_test_cleanup(int unit, void *data)
{
    int rv=SOC_E_NONE;

    if (tmu_hash_ut_test_result  == 0) /* dont cleanup if failed for debuggint */
        rv = soc_sbx_caladan3_tmu_hash_table_free(unit, hash_test_param[0].handle);
#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&hash_test_param[0].mem_end, &hash_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) End(%d) Max(%d)\n",
            hash_test_param[0].mem_begin, hash_test_param[0].mem_end, hash_test_param[0].mem_max);
#endif
    return rv;
}

/*********************/
/** EML iterator tests **/
/*********************/
#define _NUM_KEY_ (100)
#define _KEY_WORDS_ (2) /* 64 bits */
#define _PYLD_WORDS_ (4) /* 119 bits */

int _gen_rand_keys_(int unit, uint32 *key, uint32 *value, uint32 num_key)
{
    uint32 seed=0;
    int index=0, kidx=0, rv=SOC_E_NONE;
    
    if (!key || !value) return SOC_E_PARAM;

    seed = sal_time();
    sal_srand(seed);
    cli_out("%s: Random Seed: 0x%x \n", FUNCTION_NAME(), seed);
    
    do {
        do {
            for (kidx=0; kidx < _KEY_WORDS_; kidx++) {
                key[index * _KEY_WORDS_ + kidx]   = (unsigned int) sal_rand();
            }
            
            for (kidx=0; kidx < _PYLD_WORDS_; kidx++) {
                value[index * _PYLD_WORDS_ + kidx] = (unsigned int) sal_rand();
            }
            
            rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle,
                                                     key+index * _KEY_WORDS_,
                                                     value+index * _PYLD_WORDS_);
        } while (rv == SOC_E_EXISTS);

        cli_out("Key[%d] = 0x%x 0x%x \n", index,
                key[index * _KEY_WORDS_], key[index * _KEY_WORDS_+1]);
        index++;
    } while (index < num_key && SOC_SUCCESS(rv));
    

    return rv;
}

int _iter_score_board(int unit,
                      uint32 *key,
                      uint32 *iter, 
                      uint32 *score_buffer)
{
    int index=0, rv=SOC_E_NONE;

    if (!iter || !score_buffer) return SOC_E_PARAM;

    for (index=0; index < _NUM_KEY_; index++) {
        if (sal_memcmp(iter, key + index * _KEY_WORDS_, 
                       sizeof(uint32) * _KEY_WORDS_) == 0) {
            if (score_buffer[index] == 0) {
                score_buffer[index] = TRUE;
                cli_out("Score Board: Key[%d] = 0x%x 0x%x \n", index,
                        key[index * _KEY_WORDS_], key[index * _KEY_WORDS_+1]);
            } else {
                cli_out("!!! Got duplicate key on iteration: "
                        "Key[%d] = 0x%x 0x%x \n", index,
                        key[index * _KEY_WORDS_], key[index * _KEY_WORDS_+1]);
                rv = SOC_E_EXISTS;
            }
            break;
        }
    }

    if (index == _NUM_KEY_) rv = SOC_E_NOT_FOUND;
    return rv;
}

int eml_hash_iter_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE;
    uint32 *key = sal_alloc(_NUM_KEY_ * sizeof(uint32) * _KEY_WORDS_ , "hsh-iter-key");
    uint32 *value = sal_alloc(_NUM_KEY_ * sizeof(uint32) * _PYLD_WORDS_, "hsh-iter-val");
    uint32 *check = sal_alloc(_NUM_KEY_ * sizeof(uint32), "hshtest");
    uint32 iter[_KEY_WORDS_], next_iter[_KEY_WORDS_];

    sal_memset(key, 0, _NUM_KEY_ * sizeof(uint32) * _KEY_WORDS_);
    sal_memset(value, 0, _NUM_KEY_ * sizeof(uint32) * _PYLD_WORDS_);
    sal_memset(check, 0, _NUM_KEY_ * sizeof(uint32));
    
    /* generate random set of keys, insert */
    rv = _gen_rand_keys_(unit, key, value, _NUM_KEY_);

    /* iterate and verify */
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_iterator_first(unit, hash_test_param[0].handle, iter);

        rv = _iter_score_board(unit, key, iter, check);
        if (SOC_SUCCESS(rv)) {
            do {
                rv = soc_sbx_caladan3_tmu_hash_iterator_next(unit, 
                                                             hash_test_param[0].handle,
                                                             iter, next_iter);
                if (SOC_SUCCESS(rv)) {
                    rv = _iter_score_board(unit, key, next_iter, check);
                    if (SOC_FAILURE(rv)) {
                        cli_out("\n Next Iterator Scoreboard failed after "
                                "Key: 0x%x 0x%x !!! \n", iter[0], iter[1]);
                    } else {
                        sal_memcpy(iter, next_iter, sizeof(uint32) * _KEY_WORDS_);
                    }
                }

            } while (SOC_SUCCESS(rv));
                     
            if (rv == SOC_E_LIMIT) {
                cli_out(" END of Iteration $$ \n");
                rv = SOC_E_NONE;
            }

        } else {
            cli_out("\n Iterator Init failed !!! \n");
        }
    }

    sal_free(key);
    sal_free(value);
    sal_free(check);

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}

/**************************/
/** FIFO feed unit tests **/
/**************************/
int fifo_feed_test_run(int unit, void *data)
{
    int fifoid = -1, rv = SOC_E_NONE;
    uint32 regval=0;
 
    rv = soc_sbx_caladan3_tmu_hash_table_fifoid_get(unit, hash_test_param[0].handle, &fifoid); 
    if (SOC_SUCCESS(rv)) {
        READ_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, &regval);
        soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, fifo_aempty[fifoid], 1);
        WRITE_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, regval);            

        (void)soc_sbx_caladan3_tmu_hash_fifo_feed_trigger(unit);
        sal_usleep(MILLISECOND_USEC); /* ensures fifo thread grabs the mutex */

        /* verify if fifo is fed & status is unmasked */
        /* use this function so it access critical section gracefully with the thread */
        regval = soc_sbx_tmu_hash_table_fifo_push_empty_get(unit, fifoid);
        assert(regval == 0);
    }

    return rv;
}

/**************************/
/**  Recycle unit tests  **/
/**************************/
int fifo_recycle_only_test_run(int unit, void *data)
{
    int fifoid = -1, rv = SOC_E_NONE, num_recycle=128;
    soc_sbx_tmu_hash_param_t *param = &hash_test_param[0].param;


    rv = soc_sbx_caladan3_tmu_hash_table_fifoid_get(unit, hash_test_param[0].handle, &fifoid); 
    if (SOC_SUCCESS(rv)) {

        /* drain entire free chain to only recycle */
        rv = soc_sbx_tmu_hash_ut_fake_chain_free_fifo(unit, fifoid, param->num_chain_table_entries, TRUE);
        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_tmu_hash_ut_fake_chain_recyle(unit, fifoid, num_recycle);
            if (SOC_SUCCESS(rv)) {
                int count=0;

                sal_usleep(50 * MILLISECOND_USEC); /* ensures fifo thread grabs the mutex & recycles */

                rv = soc_sbx_tmu_hash_ut_recyle_entry_count(unit, fifoid, &count);
                if (SOC_SUCCESS(rv)) {
                    assert(count == num_recycle);
                    rv = soc_sbx_tmu_hash_ut_fake_chain_free_fifo(unit, fifoid, param->num_chain_table_entries, FALSE);
                    soc_sbx_tmu_hash_ut_reset_recycle_ring(unit);
                }
            }
        }
    }

    return rv;
}

/**********************************************/
/**  Recycle & free fifo trigger unit tests  **/
/**********************************************/
int fifo_recycle_fifo_trigger_test_init(int unit, void *data)
{
    int rv=SOC_E_NONE;
    soc_sbx_tmu_hash_param_t *param;

    sal_memset(&hash_test_param[0],0,sizeof(hash_table_param_t));
    param = &hash_test_param[0].param;
    param->capacity = 1000000;
    param->key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS;
    param->chain_length = 6;
    param->num_hash_table_entries = 4194304;
    param->num_chain_table_entries = 128;

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&hash_test_param[0].mem_begin, &hash_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) Max(%d) \n", hash_test_param[0].mem_begin, hash_test_param[0].mem_max);
#endif
    rv =  soc_sbx_caladan3_tmu_hash_table_alloc(unit, param, &hash_test_param[0].handle);
    
    return rv;
}

int fifo_recycle_fifo_trigger_test_run(int unit, void *data)
{
    int fifoid = -1, rv = SOC_E_NONE, num_recycle=50;
    uint32 regval=0;
    soc_sbx_tmu_hash_param_t *param = &hash_test_param[0].param;


    rv = soc_sbx_caladan3_tmu_hash_table_fifoid_get(unit, hash_test_param[0].handle, &fifoid); 
    if (SOC_SUCCESS(rv)) {

        /* drain entire free chain to only recycle */
        rv = soc_sbx_tmu_hash_ut_fake_chain_recyle(unit, fifoid, num_recycle);
        if (SOC_SUCCESS(rv)) {
            int count=0;

            sal_usleep(50 * MILLISECOND_USEC); /* ensures fifo thread grabs the mutex & recycles */

            rv = soc_sbx_tmu_hash_ut_recyle_entry_count(unit, fifoid, &count);
            if (SOC_SUCCESS(rv)) {
                assert(count == num_recycle);

                READ_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, &regval);
                soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, fifo_aempty[fifoid], 1);
                WRITE_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, regval); 
                
                (void)soc_sbx_caladan3_tmu_hash_fifo_feed_trigger(unit);
                sal_usleep(50 * MILLISECOND_USEC); /* ensures fifo thread grabs the mutex */

                /* verify if fifo is fed & status is unmasked */
                /* use this function so it access critical section gracefully with the thread */
                regval = soc_sbx_tmu_hash_table_fifo_push_empty_get(unit, fifoid);
                assert(regval == 0);

                sal_usleep(50 * MILLISECOND_USEC); /* ensures fifo thread grabs the mutex & recycles */
                rv = soc_sbx_tmu_hash_ut_recyle_entry_count(unit, fifoid, &count);
                if (SOC_SUCCESS(rv)) {
                    assert(count == 0);  /* all recycle entries must have been dma'ed */
                }
 
                rv = soc_sbx_tmu_hash_ut_fake_chain_free_fifo(unit, fifoid, param->num_chain_table_entries, FALSE);
                soc_sbx_tmu_hash_ut_reset_recycle_ring(unit);
            }
        }
    }

    return rv;
}

/*************************************************/
/** EML stress test to kick fifo feed interrupt **/
/*************************************************/
static int _hash_capacity= 512*1024; /*82711*/ /*269851*/ /*0x41e1d;*/ /*0x41e1f*/

int _verify_chain(int unit, void *data)
{
    int rv=SOC_E_NONE;
    uint32 key[] = {0x00000000, 0x00000001};
    uint32 value[] = {0,0,0xdeadbeef,0xba5eba11};
#if 0
    int index;
    for (index=0; index < 3; index++) {
        key[1] = 1;
        if (index == 0) {
            key[0] = 0xf259;
        } else if (index == 1) {
            key[0] = 0x12685;
        } else {
            key[0] = 0x41e1f;
        }
        value[1] = key[0];
        value[0] = key[1];
        
        rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
        assert(rv == SOC_E_NONE);
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, 
                                                    hash_test_param[0].handle, 
                                                    key, value, TRUE);
        assert(rv == SOC_E_NONE);
        soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x331bf, 2, key, value);
    }
#endif
#if 1
    key[1] = 1;
    key[0] = 0x12de9;
    value[1] = key[0];
    value[0] = key[1];
    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
    assert(rv == SOC_E_NONE);
    rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, 
                                                hash_test_param[0].handle, 
                                                key, value, TRUE);
#endif
    assert(rv == SOC_E_NONE);
    
    return rv;
}


int eml_hash_multi_entry_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index, vindex, iteration=1;
    uint32 key[] = {0x00000000, 0x00000001};
    uint32 value[] = {0,0,0xdeadbeef,0xba5eba11};
    sal_usecs_t start, end, accum=0;
    uint32 fkey[] = {0x13070, 0x1};
    uint32 vkey[] = {0x00000000, 0x00000001};
    uint32 vvalue[] = {0,0,0xdeadbeef,0xba5eba11};

    COMPILER_REFERENCE(vkey);
    COMPILER_REFERENCE(vvalue);
    COMPILER_REFERENCE(fkey);
    
    cli_out("### Hash Capacity Add test run: \n");


    if (0) {
        return  soc_sbx_tmu_hw_chain_entry_dump(unit,  hash_test_param[0].handle, 0/*721*/, 0);
    }

    if(0) {
        return _verify_chain(unit, data);
    }

    for (index=0, accum=0; index < _hash_capacity && SOC_SUCCESS(rv); index++) {

        value[1] = key[0];
        value[0] = key[1];

        start = sal_time_usecs();

#if 0
        if (key[0] == 0x7f35c || key[0] == 0xef0a || key[0] == 0x13070 || key[0] == 0x7d817 || key[0] == 0x7bcae) {
            cli_out("$$$ Key Inserted: 0x%x 0x%x \n", key[1], key[0]);
            cli_out("%%%% Before insertation Bucket state SW-Hash:0x3f315 SW-Bucket:7\n");
            /*(7 << hash_test_param[0].handle->table->host_max_num_entries_msb_pos) | 0x3f315);*/
            soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x3f315 , 7, key, value);

            vkey[0] = fkey[0];
            vkey[1] = fkey[1];
            vvalue[1] = vkey[0];
            vvalue[0] = vkey[1];
            rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, 
                                                        hash_test_param[0].handle, 
                                                        vkey, vvalue, TRUE);
        }
#endif

        rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
        if (SOC_SUCCESS(rv)) {
#ifdef TMU_HASH_UT_DEBUG
            cli_out("Key 0x%x 0x%x inserted \n", key[0], key[1]);
#endif
            end = sal_time_usecs();
            accum += SAL_USECS_SUB(end,start);

            /*if (iteration > 0 && (index == (iteration * 16 * 1024))) {*/
            /*if (index > (256 * 1024)) {*/
            /*if (index >= _hash_capacity-1) {*/
            /*if (index >= 82709) {*/
            /*if (key[0] == fkey[0] || key[0] == 0x7f35c) {*/
            /*if (key[0] == 0x7f35c || key[0] == 0xef0a || key[0] == 0x13070 || key[0] == 0x7d817 || key[0] == 0x7bcae) {*/
            /*if (iteration > 0 && (index == (iteration * 100 * 1024))) {*/
            /*if (index == 112332) {*/
            if (0) {
                vkey[0] = 0; vkey[1] = 1;
                
                cli_out("### Key 0x%x 0x%x inserted \n", key[1], key[0]);
                rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, 
                                                            hash_test_param[0].handle, 
                                                            key, value, TRUE);
                cli_out("Verification Point:[%d] \n", index);

                cli_out("After Insertion State of bucket \n");
                /* Hash Key Dump: 00000001 0001b6cc */
                soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x3ad89, 3, key, value);

                cli_out("After Insertion State of bucket \n");
                soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x3f315 , 7, key, value);

                iteration++;

                /* verify all key's added till this point */
                for (vindex=0; vindex <= index && SOC_SUCCESS(rv); vindex++) {
                    vvalue[1] = vkey[0];
                    vvalue[0] = vkey[1];
                    rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, 
                                                                hash_test_param[0].handle, 
                                                                &vkey[0], &vvalue[0], FALSE);
                    if (SOC_FAILURE(rv)) {
                        cli_out("!!!!! VERIFICATION Failed for vindex[%d] Index[%d] \b"
                                "Verification Key: 0x%x 0x%x \n Key: 0x%x 0x%x\n", 
                                vindex, index, vkey[1], vkey[0],
                                key[1], key[0]);
                        assert(0);
                    }
                    
                    if (vkey[0] == 0xffffffff) {
                        vkey[1]++;
                        vkey[0] = 0;
                    } else {
                        vkey[0]++;
                    }
                }
            }

            if (SOC_SUCCESS(rv)) {
            
                if (key[0] == 0xffffffff) {
                    key[1]++;
                    key[0] = 0;
                } else {
                    key[0]++;
                }
            }
        }
    }

    cli_out("@@@ Time for Adding [%d] mac - Time:[%u] @@@\n", index, accum);

    if (0)/*(SOC_FAILURE(rv))*/ {
        cli_out(" Verifying FIB ..... \n");
        key[0] = 0;
        key[1] = 1;
        for (index=0; index < _hash_capacity/* && SOC_SUCCESS(rv)*/; index++) {
            rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, 
                                                        hash_test_param[0].handle, 
                                                        &key[0], &value[0], FALSE/*TRUE*/);
            if (SOC_FAILURE(rv)) {
                assert(0);
            }

            if (key[0] == 0xffffffff) {
                key[1]++;
                key[0] = 0;
            } else {
                key[0]++;
            }
        }
    }
#if 0    
    /* 00000001 00041e1b - Hash-Idx:0x2576e Bucket-Id:0x0 */
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x2576e, 0, key, value);

    /* Hash Key Dump: 00000001 00041e1c  Hash-Idx:0x5e66d Bucket-Id:0x2 */
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x5e66d, 2, key, value);

    /*### Key 0x1 0x41e1d inserted
      #Hash Key Dump: 00000001 00041e1d Hash-Idx:0x50b2b Bucket-Id:0x0*/
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x50b2b, 0, key, value);

    /* Hash Key Dump: 00000001 00041e1e - Hash-Idx:0x25616 Bucket-Id:0x2 */
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x25616, 2, key, value);

    /* Key: 00000001 00041e1f - Hash:[0x331bf] Bucket-Idx:[0x2] */
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x331bf, 2, key, value);

    /* Hash Key Dump: 00000001 00014316 Hash-Idx:0x16edd Bucket-Id:0x5 */
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x16edd, 5, key, value);

    /* Hash Key Dump: 00000001 00012de9 Hash-Idx:0x4cfeb Bucket-Id:0x5 */
    soc_sbx_tmu_fib_get(unit,  hash_test_param[0].handle, 0x4cfeb, 5, key, value);
#endif

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}

int eml_hash_multi_entry_test_cleanup(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x00000000, 0x00000001};
    uint32 value[] = {0,0,0xdeadbeef,0xba5eba11};
    sal_usecs_t start, end, accum=0;

    /*return 0;*/
    COMPILER_REFERENCE(value);

    if (tmu_hash_ut_test_result  == 0) { /* dont cleanup if failed for debugging */
        cli_out("### Hash Capacity Delete/Cleanup run: \n");

        for (index=0, accum=0; index < _hash_capacity && SOC_SUCCESS(rv); index++) {

            start = sal_time_usecs();

            rv = soc_sbx_caladan3_tmu_hash_entry_delete(unit, hash_test_param[0].handle, &key[0]);
            if (SOC_SUCCESS(rv)) {
#ifdef TMU_HASH_UT_DEBUG
                cli_out("Key 0x%x 0x%x deleted \n", key[0], key[1]);
#endif
                end = sal_time_usecs();
                accum += SAL_USECS_SUB(end,start);                
 
                if (key[0] == 0xffffffff) {
                    key[1]++;
                    key[0] = 0;
                } else {
                    key[0]++;
                }
            }
        }

        cli_out("@@@ Time for Deleting [%d] mac - Time:[%u] @@@\n", index, accum);

        if (SOC_SUCCESS(rv)) {
            rv = soc_sbx_caladan3_tmu_hash_table_free(unit, hash_test_param[0].handle);
        }
    }

#if defined(INCLUDE_BCM_SAL_PROFILE) && defined(BROADCOM_DEBUG)
    sal_alloc_resource_usage_get(&hash_test_param[0].mem_end, &hash_test_param[0].mem_max);
    cli_out("\n Mem Begin(%d) End(%d) Max(%d)\n",
            hash_test_param[0].mem_begin, hash_test_param[0].mem_end, hash_test_param[0].mem_max);
#endif
    return rv;
}

int eml_hash_bulk_delete_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x00000000, 0x00000001};
    uint32 value[] = {0,0,0xdeadbeef,0xba5eba11};
    sal_usecs_t start, end, accum=0;
    uint32 fkey[] = {0x13070, 0x1};
    uint32 vkey[] = {0x00000000, 0x00000001};
    uint32 vvalue[] = {0,0,0xdeadbeef,0xba5eba11};
    uint32 filter_key[6], filter_key_mask[6], filter_value[4], filter_value_mask[4];    

    COMPILER_REFERENCE(vkey);
    COMPILER_REFERENCE(vvalue);
    COMPILER_REFERENCE(fkey);
    
    cli_out("### Hash Capacity Bulk delete test run: \n");

    _hash_capacity = 5*1024;
    /* set up filter to delete everything */
    for (index = 0; index < 6; index++) {
	filter_key[index]=0;
	filter_key_mask[index]=0;
    }
    for (index = 0; index < 4; index++) {
	filter_value[index]=0;
	filter_value_mask[index]=0;
    }

    for (index=0, accum=0; index < _hash_capacity && SOC_SUCCESS(rv); index++) {
        value[1] = key[0];
        value[0] = key[1];

        start = sal_time_usecs();

        rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
        if (SOC_SUCCESS(rv)) {
#ifdef TMU_HASH_UT_DEBUG
            cli_out("Key 0x%x 0x%x inserted \n", key[0], key[1]);
#endif
            end = sal_time_usecs();
            accum += SAL_USECS_SUB(end,start);

            if (SOC_SUCCESS(rv)) {
            
                if (key[0] == 0xffffffff) {
                    key[1]++;
                    key[0] = 0;
                } else {
                    key[0]++;
                }
            }
        }
    }

    cli_out("@@@ Time for Adding [%d] mac - Time:[%u] @@@\n", index, accum);

    /* issue bulk delete */
    rv = soc_sbx_caladan3_tmu_hash_bulk_delete(unit, hash_test_param[0].handle, &filter_key[0],
					       &filter_key_mask[0], &filter_value[0], &filter_value_mask[0]);

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}


#define _HASH_LRP_PRG_NUM_ (3)
#define _HASH_TMU_PRG_NUM_ (3)

static unsigned int _ucode_pyld[BITS2WORDS(128)];
static unsigned int _ucode_pyld2[BITS2WORDS(128)];

int tmu_hash_ut_setup_ucode_lookup(int unit)
{
    soc_sbx_caladan3_tmu_program_info_t prog_info;    
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    uint32 rval=0;
    unsigned int tableid;
    sal_memset(&prog_info, 0, sizeof(prog_info));
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* note set config variable c3_ucode_path before executing */
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_hash_ut.txt");

    soc_sbx_caladan3_tmu_hash_table_id_get(unit, hash_test_param[0].handle, &tableid);

    /* key format:
     * VRF <bit 48-32> , IPV4<31-0> */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _HASH_TMU_PRG_NUM_;
    prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_EML_64; 
    prog_info.key_info[0].shift[0] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = 8;
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 0;      
    prog_info.key_info[0].valid = TRUE;
    prog_info.key_info[0].tableid = tableid;
    prog_info.key_shift[0] = 0; 
    
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));

    SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _HASH_LRP_PRG_NUM_,
                                  _HASH_TMU_PRG_NUM_, 0,
                                  TRUE, FALSE));

    /* ignore errors for back to back runs */
    soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info);             

    return SOC_E_NONE;
}

int tmu_hash_ut_setup_dual_64_ucode_lookup(int unit)
{
    soc_sbx_caladan3_tmu_program_info_t prog_info;    
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    uint32 rval=0;
    unsigned int tableid;
    sal_memset(&prog_info, 0, sizeof(prog_info));
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* note set config variable c3_ucode_path before executing */
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_hash144_ut.txt");

    soc_sbx_caladan3_tmu_hash_table_id_get(unit, hash_test_param[0].handle, &tableid);

    /* key format: 64 bits, skip bits <63,32> */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _HASH_TMU_PRG_NUM_;
    prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_EML_64; 
    prog_info.key_info[0].shift[0] = 8; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = 4;
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 4;      
    prog_info.key_info[0].valid = TRUE;
    prog_info.key_info[0].tableid = tableid;
    prog_info.key_shift[0] = 4; 

    /* key2 format: 64 bits */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _HASH_TMU_PRG_NUM_;
    prog_info.key_info[1].lookup = SOC_SBX_TMU_LKUP_EML_64; 
    prog_info.key_info[1].shift[0] = 4; /* bytes */
    prog_info.key_info[1].bytes_to_mask[0] = 4;
    prog_info.key_info[1].shift[1] = 0; /* bytes */
    prog_info.key_info[1].bytes_to_mask[1] = 4;      
    prog_info.key_info[1].valid = TRUE;
    prog_info.key_info[1].tableid = tableid;
    prog_info.key_shift[1] = 4; 
    
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));

    SOC_IF_ERROR_RETURN(c3hppc_lrp_set_results_timer(unit, 0x7fffFFF));

    SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _HASH_LRP_PRG_NUM_,
                                  _HASH_TMU_PRG_NUM_, 0,
                                  TRUE, FALSE));

    /* ignore errors for back to back runs */
    soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info);             

    return SOC_E_NONE;
}

int tmu_hash_ut_setup_dual_144_ucode_lookup(int unit)
{
    soc_sbx_caladan3_tmu_program_info_t prog_info;    
    c3hppc_lrp_control_info_t   c3hppcLrpControlInfo;
    uint32 rval=0;
    unsigned int tableid;
    sal_memset(&prog_info, 0, sizeof(prog_info));
    sal_memset(&c3hppcLrpControlInfo, 0, sizeof(c3hppc_lrp_control_info_t));

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

    c3hppcLrpControlInfo.nBankSelect = 0;
    c3hppcLrpControlInfo.nEpochLength = 0;  /* A value of 0 means derive from file */
    c3hppcLrpControlInfo.nNumberOfActivePEs = 64;
    c3hppcLrpControlInfo.bDuplex = 1;
    c3hppcLrpControlInfo.bBypass = 0;
    c3hppcLrpControlInfo.bLoaderEnable = 0;
    /* note set config variable c3_ucode_path before executing */
    /* coverity[secure_coding] */
    sal_strcpy( c3hppcLrpControlInfo.sUcodeFileName, "tmu_hash144_ut.txt");

    soc_sbx_caladan3_tmu_hash_table_id_get(unit, hash_test_param[0].handle, &tableid);

    /* key format: 96 bits */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _HASH_TMU_PRG_NUM_;
    prog_info.key_info[1].lookup = SOC_SBX_TMU_LKUP_EML_144; 
    prog_info.key_info[1].shift[0] = 8; /* bytes */
    prog_info.key_info[1].bytes_to_mask[0] = 4;
    prog_info.key_info[1].shift[1] = 0; /* bytes */
    prog_info.key_info[1].bytes_to_mask[1] = 8;      
    prog_info.key_info[1].valid = TRUE;
    prog_info.key_info[1].tableid = tableid;
    prog_info.key_shift[1] = 8; 

    /* key2 format: 64 bits, skip bits <63,32> */
    prog_info.flag = SOC_SBX_TMU_PRG_FLAG_WITH_ID;
    prog_info.program_num = _HASH_TMU_PRG_NUM_;
    prog_info.key_info[0].lookup = SOC_SBX_TMU_LKUP_EML_144; 
    prog_info.key_info[0].shift[0] = 8; /* bytes */
    prog_info.key_info[0].bytes_to_mask[0] = 4;
    prog_info.key_info[0].shift[1] = 0; /* bytes */
    prog_info.key_info[0].bytes_to_mask[1] = 4;      
    prog_info.key_info[0].valid = TRUE;
    prog_info.key_info[0].tableid = tableid;
    prog_info.key_shift[0] = 4; 
    
    READ_CX_SOFT_RESET_0r(unit, &rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 0);
    WRITE_CX_SOFT_RESET_0r(unit, rval);
    soc_reg_field_set(unit, CX_SOFT_RESET_0r, &rval, LR_RESET_Nf, 1);
    WRITE_CX_SOFT_RESET_0r(unit, rval);

    SOC_IF_ERROR_RETURN(c3hppc_lrp_hw_init(unit, &c3hppcLrpControlInfo));

    SOC_IF_ERROR_RETURN(c3hppc_lrp_set_results_timer(unit, 0x7fffFFF));

    SOC_IF_ERROR_RETURN(soc_sbx_lrp_setup_tmu_program(unit, _HASH_LRP_PRG_NUM_,
                                  _HASH_TMU_PRG_NUM_, 0,
                                  TRUE, FALSE));

    /* ignore errors for back to back runs */
    /* coverity[check_return] */
    soc_sbx_caladan3_tmu_program_alloc(unit, &prog_info);             

    return SOC_E_NONE;
}

int tmu_hash_ut_verify_ucode_lookup(int unit, unsigned int *_ucode_key)
{
    int index;

#define _DBG_VERBOSE

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

#ifdef _DBG_VERBOSE
    /* write shared register */
    cli_out("\n$ Ucode Lookup: Sk0: 0x%x 0x%x \n ", 
            _ucode_key[1], _ucode_key[0]);
#endif
    /* subkey0, 31-0 p0 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*0, _ucode_key[0])); 
    /* subkey0, 63-32 p1 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*1, _ucode_key[1])); 

    /* start microcode execution */
    { uint64 uuTmp = COMPILER_64_INIT(0,100);
      SOC_IF_ERROR_RETURN(c3hppc_lrp_start_control(unit, uuTmp));
    }

    
    for (index=0; index < 4; index++) {
        _ucode_pyld[index] =  c3hppc_lrp_read_shared_register(unit,16*index+1);
#ifdef _DBG_VERBOSE
        cli_out("Payload shared reg = 0x%x \n", _ucode_pyld[index]);
#endif
    }

    /* verify.. */
    cli_out("Check Reg Val Key_31_0= 0x%x / Key_63_32=0x%x \n", 
            c3hppc_lrp_read_shared_register(unit,2),
            c3hppc_lrp_read_shared_register(unit,16+2));
 
    cli_out("Sanity values = 0x%x 0x%x\n", 
            c3hppc_lrp_read_shared_register(unit,16*2 + 2),
            c3hppc_lrp_read_shared_register(unit,16* 3 +2));
   
    return SOC_E_NONE;
}

int tmu_hash_ut_verify_dual_144_ucode_lookup(int unit, unsigned int *_ucode_key)
{
    int index;

#define _DBG_VERBOSE

    if (SAL_BOOT_PLISIM) return SOC_E_NONE;

#ifdef _DBG_VERBOSE
    /* write shared register */
    cli_out("\n$ Ucode Lookup: Key: 0x%x 0x%x 0x%x \n ", 
            _ucode_key[2], _ucode_key[1], _ucode_key[0]);
#endif
    /* subkey0, 31-0 p0 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*0, _ucode_key[0])); 
    /* subkey0, 63-32 p1 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*1, _ucode_key[1])); 
    /* subkey0, 95-64 p1 */
    SOC_IF_ERROR_RETURN(c3hppc_lrp_write_shared_register(unit, 16*2, _ucode_key[2])); 

    /* start microcode execution */
    { uint64 uuTmp = COMPILER_64_INIT(0,100);
      SOC_IF_ERROR_RETURN(c3hppc_lrp_start_control(unit, uuTmp));
    }

    /* read back payload for subkey 0 */
    for (index=0; index < 4; index++) {
        _ucode_pyld[index] =  c3hppc_lrp_read_shared_register(unit,16*index+1);
#ifdef _DBG_VERBOSE
        cli_out("SubKey 0 Payload shared reg = 0x%x \n", _ucode_pyld[index]);
#endif
    }

    /* read back payload for subkey 1 */
    for (index=0; index < 4; index++) {
        _ucode_pyld2[index] =  c3hppc_lrp_read_shared_register(unit,16*index+2);
#ifdef _DBG_VERBOSE
        cli_out("SubKey 1 Payload shared reg = 0x%x \n", _ucode_pyld2[index]);
#endif
    }

    return SOC_E_NONE;
}

/*extern unsigned int tmu_dma_rx_debug, tmu_dma_tx_debug;*/

int eml_hash_ucode_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x55667788, 0x11223344};
    uint32 value[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0x2dbeef};
    uint32 read_value[] = {0, 0, 0, 0};

    /*    tmu_dma_rx_debug=1;
          tmu_dma_tx_debug=1;  */

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                    &key[0], &value[0], FALSE);
    }

    if (SOC_SUCCESS(rv)) {
        /* verify added value */
        rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
        if (SOC_SUCCESS(rv)) {
            for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                if (value[index] != read_value[index]) {
                    cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                            read_value[index], value[index], index);
                    return SOC_E_FAIL;
                }
            }

            rv = tmu_hash_ut_setup_ucode_lookup(unit);
            if (SOC_SUCCESS(rv)) {
                rv = tmu_hash_ut_verify_ucode_lookup(unit, key);
                if (SOC_SUCCESS(rv)) {
                    for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                        if (value[index] != _ucode_pyld[index]) {
                            cli_out("\nUcode: Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                    _ucode_pyld[index], value[index], index);
                            return SOC_E_FAIL;
                        }
                    }
                } else {
                    cli_out("ERROR: Failed on ucode lookup !!!!\n");
                    rv = SOC_E_FAIL;
                }
            } else {
                cli_out("ERROR: Failed to set up ucode lookup !!!!\n");
                rv = SOC_E_FAIL;
            }
        }
    }

    /*tmu_dma_rx_debug=0;
      tmu_dma_tx_debug=0;*/

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}

int eml_hash_dual_64_ucode_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x55667788, 0x11223344, 0, 0, 0, 0};
    uint32 value[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0x2dbeef};
    uint32 key2[] = {0x55667788, 0xdeadbeef, 0x11223344, 0, 0, 0};
    uint32 value2[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0x3dbeef};
    uint32 read_value[] = {0, 0, 0, 0};

    /*    tmu_dma_rx_debug=1;
          tmu_dma_tx_debug=1;  */

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
    /*
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                    &key[0], &value[0], FALSE);
    }
    */

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key2[0], &value2[0]);
    /*
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                    &key2[0], &value2[0], FALSE);
    }
    */

    if (SOC_SUCCESS(rv)) {
        /* verify added value */
        rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
        if (SOC_SUCCESS(rv)) {
            for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                if (value[index] != read_value[index]) {
                    cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                            read_value[index], value[index], index);
                    return SOC_E_FAIL;
                }
            }

            rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key2[0], &read_value[0]);
            if (SOC_SUCCESS(rv)) {
                for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                    if (value2[index] != read_value[index]) {
                        cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                read_value[index], value2[index], index);
                        return SOC_E_FAIL;
                    }
                }
            }

            /* do a lookup, the lookup should return 2 results */
            rv = tmu_hash_ut_setup_dual_64_ucode_lookup(unit);
            if (SOC_SUCCESS(rv)) {
                rv = tmu_hash_ut_verify_dual_144_ucode_lookup(unit, key2);
                if (SOC_SUCCESS(rv)) {
                    for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                        if (value[index] != _ucode_pyld[index]) {
                            cli_out("\nUcode: Subkey 0 Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                    _ucode_pyld[index], value[index], index);
                            return SOC_E_FAIL;
                        }
                    }
                    for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                        if (value2[index] != _ucode_pyld2[index]) {
                            cli_out("\nUcode: Subkey 1 Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                    _ucode_pyld2[index], value2[index], index);
                            return SOC_E_FAIL;
                        }
                    }
                } else {
                    cli_out("ERROR: Failed on ucode lookup !!!!\n");
                    rv = SOC_E_FAIL;
                }
            } else {
                cli_out("ERROR: Failed to set up ucode lookup !!!!\n");
                rv = SOC_E_FAIL;
            }
        }
    }

    /*tmu_dma_rx_debug=0;
      tmu_dma_tx_debug=0;*/

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}

int eml_hash_dual_144_ucode_test_run(int unit, void *data)
{
    int rv=SOC_E_NONE, index;
    uint32 key[] = {0x55667788, 0x11223344, 0, 0, 0, 0};
    uint32 value[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0x2dbeef};
    uint32 key2[] = {0x55667788, 0xdeadbeef, 0x11223344, 0, 0, 0};
    uint32 value2[] = {0xdeadbeef, 0xdeadbeef, 0xdeadbeef, 0x3dbeef};
    uint32 read_value[] = {0, 0, 0, 0};

    /*    tmu_dma_rx_debug=1;
          tmu_dma_tx_debug=1;  */

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key[0], &value[0]);
    /*
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                    &key[0], &value[0], FALSE);
    }
    */

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_test_param[0].handle, &key2[0], &value2[0]);
    /*
    if (SOC_SUCCESS(rv)) {    
        rv = soc_sbx_caladan3_tmu_hash_entry_verify(unit, hash_test_param[0].handle, 
                                                    &key2[0], &value2[0], FALSE);
    }
    */

    if (SOC_SUCCESS(rv)) {
        /* verify added value */
        rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key[0], &read_value[0]);
        if (SOC_SUCCESS(rv)) {
            for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                if (value[index] != read_value[index]) {
                    cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                            read_value[index], value[index], index);
                    return SOC_E_FAIL;
                }
            }

            rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, hash_test_param[0].handle, &key2[0], &read_value[0]);
            if (SOC_SUCCESS(rv)) {
                for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                    if (value2[index] != read_value[index]) {
                        cli_out("\n Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                read_value[index], value2[index], index);
                        return SOC_E_FAIL;
                    }
                }
            }

            /* do a lookup, the lookup should return 2 results */
            rv = tmu_hash_ut_setup_dual_144_ucode_lookup(unit);
            if (SOC_SUCCESS(rv)) {
                rv = tmu_hash_ut_verify_dual_144_ucode_lookup(unit, key2);
                if (SOC_SUCCESS(rv)) {
                    for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                        if (value[index] != _ucode_pyld[index]) {
                            cli_out("\nUcode: Subkey 0 Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                    _ucode_pyld[index], value[index], index);
                            return SOC_E_FAIL;
                        }
                    }
                    for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
                        if (value2[index] != _ucode_pyld2[index]) {
                            cli_out("\nUcode: Subkey 1 Received value: 0x%x doesnt match expected : 0x%x : Idx[%d]\n",
                                    _ucode_pyld2[index], value2[index], index);
                            return SOC_E_FAIL;
                        }
                    }
                } else {
                    cli_out("ERROR: Failed on ucode lookup !!!!\n");
                    rv = SOC_E_FAIL;
                }
            } else {
                cli_out("ERROR: Failed to set up ucode lookup !!!!\n");
                rv = SOC_E_FAIL;
            }
        }
    }

    /*tmu_dma_rx_debug=0;
      tmu_dma_tx_debug=0;*/

    if (SOC_SUCCESS(rv)) {
        tmu_hash_ut_test_result = 0;
    } else {
        tmu_hash_ut_test_result = -1;
    }
    return rv;
}

/*********************/
/*     Tests         */
/*********************/
/* test database */
test_call_back_t test_cback_array[] = {
    {0, NULL, hpcm_test_run, NULL},
    {1, eml_table_init, eml_hash_test_run, eml_test_cleanup},
    {2, eml_table_init, fifo_feed_test_run, eml_test_cleanup},
    {3, eml_table_init, fifo_recycle_only_test_run, eml_test_cleanup},
    {4, fifo_recycle_fifo_trigger_test_init, fifo_recycle_fifo_trigger_test_run, eml_test_cleanup},
    {5, eml_table_init, eml_hash_multi_entry_test_run, eml_hash_multi_entry_test_cleanup},
    {6, eml_table_init, eml_hash_iter_test_run, eml_test_cleanup},
    {7, eml_table_init, eml_hash_ucode_test_run, eml_test_cleanup},
    {8, eml_table_init, eml_hash_bulk_delete_test_run, eml_test_cleanup},
    {9, eml_table_dual_64_init, eml_hash_dual_64_ucode_test_run, eml_test_cleanup},
    {9, eml_table_dual_144_init, eml_hash_dual_144_ucode_test_run, eml_test_cleanup}
};


/*********************/
/*     Test Runner   */
/*********************/
int c3_ut_tmu_hash_test_init(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;
    
    if (0) {
        /* this is causing issues, need to fix uninit, reinit 4/10/2014 */
        /*
          rv = soc_sbx_caladan3_tmu_driver_init(testinfo->unit);
          if ((rv != SOC_E_INIT) && SOC_FAILURE(rv)) {
          cli_out("C3 %d TMU driver init failed\n",testinfo->unit);
          return rv;
          }
        */
    } else {
        /* only free the used program */
        soc_sbx_caladan3_tmu_program_free(testinfo->unit, _HASH_TMU_PRG_NUM_);
    }

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(test_cback_array)) return SOC_E_PARAM;

    if (test_cback_array[testinfo->testid].init)
        rv = test_cback_array[testinfo->testid].init(testinfo->unit, userdata);

    return rv;
}

int
c3_ut_tmu_hash_test_run(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(test_cback_array)) return SOC_E_PARAM;

    if (test_cback_array[testinfo->testid].run)
        rv = test_cback_array[testinfo->testid].run(testinfo->unit, userdata);

    return rv;
}

int
c3_ut_tmu_hash_test_done(c3sw_test_info_t *testinfo, void *userdata)
{
    int rv=SOC_E_NONE;

    if (testinfo->testid < 0 || testinfo->testid >= COUNTOF(test_cback_array)) return SOC_E_PARAM;

    if (test_cback_array[testinfo->testid].clean)
        rv = test_cback_array[testinfo->testid].clean(testinfo->unit, userdata);

    return rv;
}

#endif /* #ifdef BCM_CALADAN3_SUPPORT */
