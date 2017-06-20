/*
 * $Id: hash.c,v 1.59.10.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    hash.c
 * Purpose: Caladan3 Hash table lookup libraries 
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/hash.h>
#include <soc/sbx/caladan3/hpcm.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <sal/core/time.h>


#define TIME_STAMP_DBG
#undef TIME_STAMP_DBG
#ifdef TIME_STAMP_DBG 
sal_usecs_t        start;
#define TIME_STAMP_START start = sal_time_usecs();
#define TIME_STAMP(msg)                                             \
  do {                                                              \
    LOG_CLI((BSL_META("\n %s: Time Stamp: [%u]"), msg, SAL_USECS_SUB(sal_time_usecs(), start))); \
  } while(0);
#else
#define TIME_STAMP_START 
#define TIME_STAMP(msg)   
#endif


/* Knobs to consider:
 * 1 - thread priority
 * 2 - recycle dma size, periodicity, entries to consume in one cycle
 */

static int retry_flag=0;

#ifdef TMU_HASH_TBL_LOCK_SUPPORT
#define TMU_HTBL_LOCK(p) \
    sal_mutex_take((p)->lock, sal_mutex_FOREVER)

#define TMU_HTBL_UNLOCK(p) \
    sal_mutex_give((p)->lock)
#else
#define TMU_HTBL_LOCK(p)
#define TMU_HTBL_UNLOCK(p)
#endif


int _g_tmu_key_size_bits[] = {64, 64 * 3, 64 * 5, 64 * 7};

#define SOC_SBX_TMU_HASH_RECYCLE_TIMEOUT (10  * MILLISECOND_USEC) /* 10msec */


STATIC soc_sbx_tmu_hash_dbase_t* _tmu_hash_dbase[SOC_MAX_NUM_DEVICES];

STATIC soc_sbx_tmu_hash_fifo_mgr_t *_tmu_hash_fifo_mgr[SOC_MAX_NUM_DEVICES];

#define TMU_HASH_FIFO_MGR(unit, fifoid) _tmu_hash_fifo_mgr[unit]->fifo_cfg[fifoid]

#define TMU_HASH_RECYCLE_MGR(unit) _tmu_hash_fifo_mgr[unit]->recycle_ring_mgr

#define TMU_HASH_LOCK(unit) \
  sal_mutex_take(_tmu_hash_dbase[unit]->mutex, sal_mutex_FOREVER)

#define TMU_HASH_UNLOCK(unit) \
  sal_mutex_give(_tmu_hash_dbase[unit]->mutex)

#define TMU_HASH_FIFOMGR_LOCK(unit) \
  sal_mutex_take(_tmu_hash_fifo_mgr[unit]->mutex, sal_mutex_FOREVER)

#define TMU_HASH_FIFOMGR_UNLOCK(unit) \
  sal_mutex_give(_tmu_hash_fifo_mgr[unit]->mutex)

#define TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit) \
 do { if (!_tmu_hash_dbase[unit]) return SOC_E_INIT; } while(0);

#define TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit) \
 do { if (!_tmu_hash_fifo_mgr[unit]) return SOC_E_INIT; } while(0);

#define START (1)
#define STOP  (0)

int tmu_hash_entry_add_commit(int unit, tmu_hash_table_t *table,
                              tmu_hash_entry_t *entry, soc_sbx_tmu_hash_key_type_e_t key_type);
int tmu_hash_entry_delete_commit(int unit, tmu_hash_table_t *table, 
                                 tmu_hash_entry_t *entry, soc_sbx_tmu_hash_key_type_e_t key_type);
void tmu_hash_fifo_manager_thread(void *arg);
int tmu_hash_fifo_recycle_thread_toggle(int unit,
                                        uint8 start /* TRUE-start, FALSE-stop */);
int soc_sbx_caladan3_tmu_hash_fifo_feed(int unit, int fifoid);


soc_sbx_tmu_hash_dbase_t* tmu_hash_dbase_get(int unit) {
    return _tmu_hash_dbase[unit];
}

soc_sbx_tmu_hash_fifo_mgr_t* tmu_hash_fifo_mgr_get(int unit) {
    return _tmu_hash_fifo_mgr[unit];
}

/*
 *   Function
 *     _soc_sbx_caladan3_tmu_eml_144_mode_set
 *   Purpose
 *     enable/disable EML144 mode for bcm88030 B0 and Above revision
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) enable : TRUE: enable.  FALSE: disable
 *   Returns
 *      Status
 *   NOTE:
 *      only supported on B0 and above revision.
 */
STATIC int _soc_sbx_caladan3_tmu_eml_144_mode_set(int unit, int enable)
{
    uint32 regval=0;
    int qe = 0;
    uint16              dev_id;
    uint8               rev_id;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    if ((rev_id == BCM88030_A0_REV_ID) ||
        (rev_id == BCM88030_A1_REV_ID)) {
        if (enable) {
            /* EML144 is not supported on A0/A1 */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d doesn't support EML144 mode !!!\n"), 
                       FUNCTION_NAME(), unit));
            return SOC_E_PARAM;
        } else {
            /* do nothing */
            return SOC_E_NONE;
        }
    }

    for (qe = 0; qe < SOC_SBX_CALADAN3_TMU_QE_INSTANCE_NUM; ++qe) {
        soc_reg32_get(unit, TM_QE_CONFIGr, SOC_BLOCK_PORT(unit,qe), 0, &regval);
        soc_reg_field_set(unit, TM_QE_CONFIGr, &regval, EML_144_MODEf, enable?1:0);
        soc_reg32_set(unit, TM_QE_CONFIGr, SOC_BLOCK_PORT(unit,qe), 0, regval);
    }

    SOC_IF_ERROR_RETURN(READ_TMB_CONTROLr(unit, &regval));
    soc_reg_field_set( unit, TMB_CONTROLr, &regval, EML_144_MODEf, enable?1:0);
    SOC_IF_ERROR_RETURN(WRITE_TMB_CONTROLr(unit, regval));

    return SOC_E_NONE;
}

/*
 * Function:
 *   _tmu_hash_table_node_blk_alloc
 * Purpose:
 *   Create and initialize a hash table node pool.
 * Notes:
 */
STATIC int _tmu_hash_table_node_blk_alloc(int unit, tmu_hash_table_t *tbl)
{
    int rv = SOC_E_NONE, index;
    tmu_hash_table_node_t *blk=NULL;

    if (!tbl) return SOC_E_PARAM;

    for (index=0; index < tbl->alloc_blk_cnt && SOC_SUCCESS(rv); index++) {
        blk = sal_alloc(sizeof(tmu_hash_table_node_t), "tmu-tbl-node-pool");
        if (blk == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate memory %d !!!\n"), 
                       FUNCTION_NAME(), unit, rv));
            rv = SOC_E_MEMORY;
        } else {
            sal_memset(blk, 0, sizeof(tmu_hash_table_node_t));
            DQ_INSERT_HEAD(&tbl->table_node_free_list, &blk->list_node);
            DQ_INIT(&blk->bucket_list);
            tbl->num_free_table_nodes++;
        }
    }

    return rv;
}

/*
 * Function:
 *   _tmu_hash_table_node_blk_free
 * Purpose:
 *   Create and initialize a hash table.
 * Notes:
 */
STATIC int _tmu_hash_table_node_blk_free(int unit, tmu_hash_table_t *tbl, uint8 free_all)
{
    int rv = SOC_E_NONE, index=0;
    dq_p_t elem;

    if (!tbl) return SOC_E_PARAM;

    if (!DQ_EMPTY(&tbl->table_node_free_list)) {
        DQ_TRAVERSE(&tbl->table_node_free_list, elem) {
            tmu_hash_table_node_t *entry = DQ_ELEMENT_GET(tmu_hash_table_node_t*, elem, list_node);
            DQ_REMOVE(&entry->list_node);
            sal_free(entry);
            index++;
            if (!free_all && index == tbl->alloc_blk_cnt) break;
        } DQ_TRAVERSE_END(&tbl->table_node_free_list, elem);
    }

    tbl->num_free_table_nodes -= index;
    return rv;
}

/*
 * Function:
 *   tmu_hash_table_node_alloc
 * Purpose:
 *   allocate table node
 * Notes:
 */
tmu_hash_table_node_t*
tmu_hash_table_node_alloc(int unit, tmu_hash_table_t *tbl)
{
    int rv = SOC_E_NONE;
    tmu_hash_table_node_t *entry = NULL;
    
    /* if the free list is empty, allocate a block of entries to the pool */
    if (tbl->num_free_table_nodes == 0) {        
        rv = _tmu_hash_table_node_blk_alloc(unit, tbl);
    }

    if (SOC_SUCCESS(rv)) {
        /* free list must be non-zero here.  Pop a free entry & return */
        DQ_REMOVE_HEAD(&tbl->table_node_free_list, entry);
        /* entry set above if on double linked list */
        /* coverity[check_after_deref] */
        if (entry) {
            DQ_INIT(&entry->bucket_list);
        }
        tbl->num_free_table_nodes--;
    } 

    return entry;
}

/*
 * Function:
 *   _tmu_hash_table_node_free
 * Purpose:
 *   free table node
 * Notes:
 */
void  
tmu_hash_table_node_free(int unit, tmu_hash_table_t *tbl, tmu_hash_table_node_t **entry)
{
    if (!tbl || !entry) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad input argument !!!\n"), 
                   FUNCTION_NAME(), unit));
        return;
    }

    assert(DQ_EMPTY(&(*entry)->bucket_list)); /* buckets must be freed before the table nodes */

    DQ_INSERT_TAIL(&tbl->table_node_free_list, &(*entry)->list_node);
    *entry = NULL;
    tbl->num_free_table_nodes++;

    /* try to free up one block for other applications */
    if (tbl->num_free_table_nodes > (2 * tbl->alloc_blk_cnt)) {
        _tmu_hash_table_node_blk_free(unit, tbl, FALSE);
    }
}

/*
 * Function:
 *   _tmu_hash_table_entry_blk_alloc
 * Purpose:
 *   
 * Notes:
 */
STATIC int _tmu_hash_table_entry_blk_alloc(int unit, tmu_hash_table_t *tbl)
{
    int rv = SOC_E_NONE, index;
    tmu_hash_entry_t *blk=NULL;

    if (!tbl) return SOC_E_PARAM;

    for (index=0; index < tbl->alloc_blk_cnt && SOC_SUCCESS(rv); index++) {
        blk = sal_alloc(sizeof(tmu_hash_entry_t), "tmu-hash-entry-pool");
        if (blk == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate memory %d !!!\n"), 
                       FUNCTION_NAME(), unit, rv));
            rv = SOC_E_MEMORY;
        } else {
            sal_memset(blk, 0, sizeof(tmu_hash_entry_t));
            blk->key = sal_alloc(sizeof(uint32) * BITS2WORDS(tbl->key_size_bits), "tmu-hash-key-pool");
            if (blk->key == NULL) {
                sal_free(blk);
                rv = SOC_E_MEMORY;
            } else {
                DQ_INSERT_HEAD(&tbl->entry_free_list, &blk->list_node);
                tbl->num_free_entry++;
            }
        }
    }

    return rv;
}

/*
 * Function:
 *   _tmu_hash_table_entry_blk_free
 * Purpose:
 *   
 * Notes:
 */
STATIC int _tmu_hash_table_entry_blk_free(int unit, tmu_hash_table_t *tbl, uint8 free_all)
{
    int rv = SOC_E_NONE, index=0;
    dq_p_t elem;

    if (!tbl) return SOC_E_PARAM;

    if (!DQ_EMPTY(&tbl->entry_free_list)) {
        DQ_TRAVERSE(&tbl->entry_free_list, elem) {
            tmu_hash_entry_t *entry = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
            DQ_REMOVE(&entry->list_node);
            sal_free(entry->key);
            sal_free(entry);
            index++;
            if (!free_all && index == tbl->alloc_blk_cnt) break;
        } DQ_TRAVERSE_END(&tbl->entry_free_list, elem);
    }

    tbl->num_free_entry -= index;
    return rv;
}

/*
 * Function:
 *   _tmu_hash_table_entry_alloc
 * Purpose:
 *   allocate hash entry
 * Notes:
 */
STATIC tmu_hash_entry_t*
_tmu_hash_table_entry_alloc(int unit, tmu_hash_table_t *tbl)
{
    int rv = SOC_E_NONE;
    tmu_hash_entry_t *entry = NULL;
    
    /* if the free list is empty, allocate a block of entries to the pool */
    if (tbl->num_free_entry == 0) {        
        rv = _tmu_hash_table_entry_blk_alloc(unit, tbl);
    }

    if (SOC_SUCCESS(rv)) {
        /* free list must be non-zero here.  Pop a free entry & return */
        DQ_REMOVE_HEAD(&tbl->entry_free_list, entry);
        /* coverity[check_after_deref] */
        if (entry) {
            sal_memset(entry->key, 0, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits));
        }
        tbl->num_free_entry--;
    } 

    return entry;
}

/*
 * Function:
 *   _tmu_hash_table_entry_free
 * Purpose:
 *   free hash entry
 * Notes:
 */
STATIC void  
_tmu_hash_table_entry_free(int unit, tmu_hash_table_t *tbl, tmu_hash_entry_t **entry)
{
    if (entry && tbl) {
        DQ_REMOVE(&(*entry)->list_node);
        DQ_INSERT_TAIL(&tbl->entry_free_list, &(*entry)->list_node);
        *entry = NULL;
        tbl->num_free_entry++;
        
        /* try to free up one block for other applications */
        if (tbl->num_free_entry > (2 * tbl->alloc_blk_cnt)) {
            _tmu_hash_table_entry_blk_free(unit, tbl, FALSE);
        }
    } else {
        assert(0);
    }
}

/*
 * Function:
 *   _tmu_crc32_ieee802
 */
STATIC 
uint32 _tmu_crc32_ieee802(uint32 data) 
{
    const uint32 poly = 0x04c11db7;
    int index;
  
    for (index = 0; index < 32; index++) {
        if (data & 0x80000000)
            data = (data << 1) ^ poly;
        else
            data <<= 1;
    }
    
    return data;
}

/*
 * Function:
 *   _tmu_crc64
 */
STATIC uint64 _tmu_crc64(uint64 uuDataIn) {
  int index;
  uint64 uuResult, poly;

  COMPILER_64_SET(poly, 0x42F0E1EB, 0xA9EA3693);
  uuResult =  uuDataIn;

  for (index = 0; index < 64; index++) {

      if (COMPILER_64_BITTEST(uuResult, 63)) {
          COMPILER_64_SHL(uuResult, 1);
          COMPILER_64_XOR(uuResult, poly);
      } else {
          COMPILER_64_SHL(uuResult, 1);
      }
  }

  return uuResult;
}

/*
 * Function:
 *   tmu_hash_table_entry_find
 * Purpose:
 *   free hash entry
 * Notes:
 */
int
tmu_hash_function(int unit,
                  tmu_hash_table_t *tbl,
                  uint32 *key, /* IN */
                  uint32 *hash_idx, 
                  uint32 *bucket_idx)
{
    int rv = SOC_E_NONE, keyword=0;
    int hash_tbl_idx=0, index=0;
    uint32 accum32=0;
    uint64 accum64, keybits;
#define _TMU_KEY_SIZE_IN_64b_WORDS_ (7)
    uint64 _key[_TMU_KEY_SIZE_IN_64b_WORDS_];

    if (!key || !hash_idx || !bucket_idx || !tbl) {
        return SOC_E_PARAM;
    }

    TMU_HASH_LOCK(unit);

    /* if hash is enabled */
    if (_tmu_hash_dbase[unit]->hashtable && _tmu_hash_dbase[unit]->hash_adjust) {

        COMPILER_64_ZERO(accum64);
        accum32 = 0;
        COMPILER_64_ZERO(keybits);
        sal_memset(_key, 0, sizeof(_key));

        for (index=0; index < BITS2WORDS(tbl->key_size_bits); index+=2) {
            COMPILER_64_SET(_key[index/2], key[index+1], key[index]);
        }

        /* Adjust key and compute CRCs... */
        for (keyword = _TMU_KEY_SIZE_IN_64b_WORDS_ - 1; keyword >= 0; --keyword) {
            keybits = _key[keyword];
            COMPILER_64_ADD_32(keybits, _tmu_hash_dbase[unit]->hash_adjust[0]);

            accum32 = _tmu_crc32_ieee802(accum32 ^ COMPILER_64_HI(_key[keyword])); /* high */
            accum32 = _tmu_crc32_ieee802(accum32 ^ COMPILER_64_LO(_key[keyword])); /* low */

            COMPILER_64_XOR(accum64, keybits);
            accum64 = _tmu_crc64(accum64);
        }

        /* Permute */
        for (index = 0; index < TMU_HASH_MAX_TABLE; index++) {
            uint64 mask;
            keybits = accum64;
            COMPILER_64_SHR(keybits, (8 * index));
            COMPILER_64_SET(mask, 0, TMU_HASH_MAX_TABLE_ENTRY-1);
            COMPILER_64_AND(keybits, mask);
            hash_tbl_idx = COMPILER_64_LO(keybits);
            accum32 ^= _tmu_hash_dbase[unit]->hashtable[index * TMU_HASH_MAX_TABLE_ENTRY + hash_tbl_idx];
        }

        /* Fold and reduce to table size */
        accum32 ^= (accum32 >> tbl->device_max_num_entries_msb_pos);
        accum32 &= (1 << tbl->device_max_num_entries_msb_pos) - 1;

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "%s:%d HASH:0x%08x \n"), FUNCTION_NAME(), unit, accum32));

        *hash_idx = accum32 & ((1 << tbl->host_max_num_entries_msb_pos) - 1);
        *bucket_idx = accum32 >> tbl->host_max_num_entries_msb_pos;
        *bucket_idx &= ((1 << (tbl->device_max_num_entries_msb_pos -
                               tbl->host_max_num_entries_msb_pos)) - 1);

    } else {
        /* use least significant key word to hash */
        *hash_idx = key[0] & ((1 << tbl->host_max_num_entries_msb_pos) - 1);
        *bucket_idx = key[0] & ((1 << tbl->device_max_num_entries_msb_pos) - 1);
        *bucket_idx &=  ((1 << (tbl->device_max_num_entries_msb_pos - tbl->host_max_num_entries_msb_pos)) - 1);
    }

    TMU_HASH_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *   _tmu_hash_table_entry_find
 * Purpose:
 *   find hash entry
 * Notes:
 */
STATIC int
_tmu_hash_table_entry_find(int unit, tmu_hash_table_t *tbl,
                           uint32 hash_idx, uint32 bucket_idx,
                           uint32 *key, tmu_hash_entry_t **table_entry /*OUT*/)
{
    tmu_hash_entry_t *entry = NULL, *bucket = NULL;
    tmu_hash_table_node_t *tbl_node = NULL;
    int rv = SOC_E_NONE;
    dq_p_t elem;

    if (!tbl || !key || !table_entry) return SOC_E_PARAM;

    TMU_HTBL_LOCK(tbl);

    /* validate hash & bucket index */
    if (hash_idx > tbl->host_table_size/TMU_HASH_DEFAULT_LOADING_FACTOR) {
        /* bad hash index */
        rv = SOC_E_PARAM;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad Hash Index %d !!!\n"), 
                   FUNCTION_NAME(), unit, hash_idx));
    }

    if (bucket_idx >= (1 << (tbl->device_max_num_entries_msb_pos - tbl->host_max_num_entries_msb_pos))) {
        rv = SOC_E_PARAM;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Bad Bucket Index %d !!!\n"), 
                   FUNCTION_NAME(), unit, bucket_idx));
    }
    
    if (SOC_SUCCESS(rv)) {
        tbl_node = tbl->table[hash_idx];

        if (tbl_node) {
            /* Reverse logic to deal with coverity issue and
             * remove assert as a means to abort task
             */
            if (DQ_EMPTY(&tbl_node->bucket_list)) {
                assert(0);
                return SOC_E_INTERNAL;
            }

            DQ_TRAVERSE(&tbl_node->bucket_list, elem) {
                bucket = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                if (bucket->bucket_idx == bucket_idx) break;
            } DQ_TRAVERSE_END(&tbl->bucket_list, elem);
            
            if (bucket->bucket_idx == bucket_idx) {
                if (sal_memcmp(bucket->key, key, BITS2BYTES(tbl->key_size_bits)) == 0) {
                    *table_entry = bucket;
                    rv = SOC_E_NONE;
                } else {
                    rv = SOC_E_NOT_FOUND;
                    DQ_TRAVERSE(&bucket->chain_list, elem) {
                        entry = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                        if (sal_memcmp(entry->key, key, BITS2BYTES(tbl->key_size_bits)) == 0) {
                            *table_entry = entry;
                            rv = SOC_E_NONE;
                            break;
                        }
                    } DQ_TRAVERSE_END(&bucket->chain_list, elem);
                }

            } else {
                rv = SOC_E_NOT_FOUND;
            }
        } else {
            rv = SOC_E_NOT_FOUND;
        }
    }

    if (SOC_FAILURE(rv)) {
        *table_entry = NULL;
    }
        
    TMU_HTBL_UNLOCK(tbl);

    return rv;
}

/*
 * Function:
 *   tmu_hash_table_entry_get
 * Purpose:
 *   get hash entry
 * Notes:
 */
int
tmu_hash_table_entry_get(int unit,
                         soc_sbx_tmu_hash_handle_t handle,
                         tmu_hash_table_t *tbl,
                         uint32 *key, 
                         uint32 *value /* OUT */)
{
    uint32 hash_idx=0, bucket_idx=0;
    uint32 hw_key[16];
    uint32 hw_value[BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS)];
    int rv = SOC_E_NONE;
    tmu_hash_entry_t *entry = NULL;

    if (!tbl || !key || !value) {
        return SOC_E_PARAM;
    }
    
    TMU_HTBL_LOCK(tbl);

    sal_memcpy(hw_key, key, BITS2BYTES(tbl->key_size_bits));

    rv = tmu_hash_function(unit, tbl, key, &hash_idx, &bucket_idx);

    if (SOC_SUCCESS(rv)) {
        rv = _tmu_hash_table_entry_find(unit, tbl,
                                        hash_idx, bucket_idx,
                                        key, &entry);
        if (SOC_SUCCESS(rv)) {
            sal_memcpy(value, entry->value, sizeof(uint32) *
                       BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));
        }
    }

    if (!(SOC_SUCCESS(rv)) && (tbl->host_hw_fib_get)) {
        rv = soc_sbx_tmu_fib_get(unit,
                                 handle,
                                 hash_idx,
                                 bucket_idx,
                                 hw_key,
                                 hw_value);
        if (SOC_SUCCESS(rv)) {
            if (sal_memcmp(key, hw_key, BITS2BYTES(tbl->key_size_bits)) != 0) {
                rv = SOC_E_NOT_FOUND; 
            } else {
                sal_memcpy(value, hw_value, sizeof(uint32) *
                           BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));
            }
        }
    }

    TMU_HTBL_UNLOCK(tbl);
    return rv;
}

/*
 * Function:
 *   tmu_hash_table_entry_hw_get
 * Purpose:
 *   get hash entry from hardware
 * Notes:
 */
int
tmu_hash_table_entry_hw_get(int unit,
                         soc_sbx_tmu_hash_handle_t handle,
                         tmu_hash_table_t *tbl,
                         uint32 *key, 
                         uint32 *value /* OUT */)
{
    uint32 hash_idx=0, bucket_idx=0;
    uint32 hw_key[16];
    uint32 hw_value[BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS)];
    int rv = SOC_E_NONE;

    if (!tbl || !key || !value) {
        return SOC_E_PARAM;
    }
    
    TMU_HTBL_LOCK(tbl);

    sal_memcpy(hw_key, key, BITS2BYTES(tbl->key_size_bits));

    rv = tmu_hash_function(unit, tbl, key, &hash_idx, &bucket_idx);

    if (SOC_SUCCESS(rv)) {
        rv = soc_sbx_tmu_fib_get(unit,
                             handle,
                             hash_idx,
                             bucket_idx,
                             hw_key,
                             hw_value);
        if (SOC_SUCCESS(rv)) {
            if (sal_memcmp(key, hw_key, BITS2BYTES(tbl->key_size_bits)) != 0) {
                rv = SOC_E_NOT_FOUND; 
            } else {
                sal_memcpy(value, hw_value, sizeof(uint32) *
                           BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));
            }
        }
    }

    TMU_HTBL_UNLOCK(tbl);
    return rv;
}

/*
 *
 * Function:
 *     tmu_hash_iterator_first
 * Purpose:
 *     get first node starting from hash table index
 */
int tmu_hash_iterator_first(int unit, 
                            tmu_hash_table_t *tbl,
                            int index,
                            uint32 *key)
{
    int status = SOC_E_NONE;
    tmu_hash_entry_t *bucket = NULL;
    tmu_hash_table_node_t *tbl_node = NULL;
    dq_p_t elem;

    if (!tbl || !key) return SOC_E_PARAM;

    if (index >= (1 << tbl->host_max_num_entries_msb_pos)) return SOC_E_PARAM;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    /* return the first encountered node */
    TMU_HTBL_LOCK(tbl);


    for (; index < (1 << tbl->host_max_num_entries_msb_pos); index++) {
        if (tbl->table[index]) {
            tbl_node = tbl->table[index];
            assert(!DQ_EMPTY(&tbl_node->bucket_list));
            DQ_TRAVERSE(&tbl_node->bucket_list, elem) {
                bucket = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                sal_memcpy(key, bucket->key, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits));
                break;
            } DQ_TRAVERSE_END(&tbl->bucket_list, elem);                
            break;
        }
    }

    if (index >= (1 << tbl->host_max_num_entries_msb_pos)) {
        status = SOC_E_LIMIT; 
    }
    
    TMU_HTBL_UNLOCK(tbl);
    return status;
}


/*
 *
 * Function:
 *     tmu_hash_iterator_next
 * Purpose:
 *     get the next applicable entry
 */
int tmu_hash_iterator_next(int unit, 
                           tmu_hash_table_t *tbl,
                           uint32 *key,
                           uint32 *next_key)
{
    uint32 hash_idx=0, bucket_idx=0;
    int rv = SOC_E_NONE;
    tmu_hash_entry_t *entry = NULL, *bucket = NULL;
    tmu_hash_table_node_t *tbl_node = NULL;
    dq_p_t elem;


    if (!tbl || !key || !next_key) {
        return SOC_E_PARAM;
    }
    
    TMU_HTBL_LOCK(tbl);

    rv = tmu_hash_function(unit, tbl, key, &hash_idx, &bucket_idx);
    if (SOC_SUCCESS(rv)) {
        tbl_node = tbl->table[hash_idx];
        if (tbl_node) {
          /* Reverse logic to deal with coverity issue and
           * remove assert as a means to abort task
           */
            if (DQ_EMPTY(&tbl_node->bucket_list)) {
                assert(0);
                return SOC_E_INTERNAL;
            }

            DQ_TRAVERSE(&tbl_node->bucket_list, elem) {
                bucket = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                if (bucket->bucket_idx == bucket_idx) break;
            } DQ_TRAVERSE_END(&tbl->bucket_list, elem);
            if (bucket != NULL) {
            if (bucket->bucket_idx == bucket_idx) {
                if (sal_memcmp(bucket->key, key, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits)) == 0) {
                    rv = SOC_E_NONE;
                    entry = bucket;
                } else {
                    rv = SOC_E_NOT_FOUND;
                    DQ_TRAVERSE(&bucket->chain_list, elem) {
                        entry = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                        if (sal_memcmp(entry->key, key, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits)) == 0) {
                            rv = SOC_E_NONE;
                            break;
                        }
                    } DQ_TRAVERSE_END(&bucket->chain_list, elem);
                }
            } else {
        rv = SOC_E_NOT_FOUND;
            }
        }
        } else {
            rv = SOC_E_NOT_FOUND;
        }

        /* key found */
        if (SOC_SUCCESS(rv) && (entry != NULL)) {
            uint8 iterate=FALSE;

            if (entry->type == TMU_HASH_NODE_CHAIN) {
                if (DQ_TAIL(&bucket->chain_list,dq_p_t) == &entry->list_node) {
                    iterate = TRUE;
                } else {
                    entry = DQ_ELEMENT_GET(tmu_hash_entry_t*,
                                           DQ_NEXT(&entry->list_node,dq_p_t),
                                           list_node);
                } 
            } else if (entry->type == TMU_HASH_NODE_BUCKET) {
                if (DQ_EMPTY(&bucket->chain_list)) {
                    iterate = TRUE;
                } else {
                    entry = DQ_ELEMENT_GET(tmu_hash_entry_t*,
                                           DQ_HEAD(&bucket->chain_list,dq_p_t),
                                           list_node);
                }
                    
            } else {
                assert(0);
            }
            
            if (iterate) {
                /* go to next bucket */
                if (DQ_TAIL(&tbl_node->bucket_list,dq_p_t) == &bucket->list_node) {
                    if (hash_idx == ((1 << tbl->host_max_num_entries_msb_pos) - 1)) {
                        rv = SOC_E_LIMIT;
                    } else {
                        rv = tmu_hash_iterator_first(unit, tbl, hash_idx+1, next_key);
                    }
                } else {
                    entry = DQ_ELEMENT_GET(tmu_hash_entry_t*,
                                           DQ_NEXT(&bucket->list_node,dq_p_t),
                                           list_node); 
                    sal_memcpy(next_key, entry->key, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits));
                }
            } else {
                sal_memcpy(next_key, entry->key, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits));
            }
        } 
    }
    
    TMU_HTBL_UNLOCK(tbl);
    return rv;
}

/*
 * Function:
 *   tmu_hash_table_entry_insert
 * Purpose:
 *   insert hash entry
 * Notes:
 */
int
tmu_hash_table_entry_insert(int unit,
                            tmu_hash_table_t *tbl,
                            uint32 *key, 
                            uint32 *value,
                            soc_sbx_tmu_hash_key_type_e_t key_type)
{
    uint32 hash_idx=0, bucket_idx=0;
    tmu_hash_entry_t *entry = NULL, *bucket = NULL;
    tmu_hash_table_node_t *tbl_node = NULL;
    int rv = SOC_E_NONE;
    dq_p_t elem;

    if (!tbl || !key || !value) return SOC_E_PARAM;

    TMU_HTBL_LOCK(tbl);

    rv = tmu_hash_function(unit, tbl, key, &hash_idx, &bucket_idx);
    if (SOC_SUCCESS(rv)) {
        if (0) {
            LOG_CLI((BSL_META_U(unit,
                                "++++++ Insert: Key: %08x %08x - Hash:[0x%x] Bucket-Idx:[0x%x] HW-Hash[0x%x]\n HW-Root[0x%x] \n"
                                " Host-MSB[%d] Device-MSB[%d] \n"), 
                     key[1], key[0], hash_idx, bucket_idx, 
                     (bucket_idx << tbl->host_max_num_entries_msb_pos) | hash_idx,
                     ((bucket_idx << tbl->host_max_num_entries_msb_pos) | hash_idx) & 
                     ((1<<tbl->device_max_num_entries_msb_pos)-1),
                     tbl->host_max_num_entries_msb_pos,
                     tbl->device_max_num_entries_msb_pos));  
        }

        rv = _tmu_hash_table_entry_find(unit, tbl, 
                                        hash_idx, bucket_idx, 
                                        key, &entry);
        if (rv == SOC_E_NOT_FOUND) { /* insert */

            rv = SOC_E_NONE;

            /* limit insertion to maximum size specified */
            if (tbl->num_entries >= tbl->max_num_entries) {
                rv = SOC_E_FULL;
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Hash Table Full %d !!!\n"), 
                           FUNCTION_NAME(), unit, rv));
            } else {

                entry = _tmu_hash_table_entry_alloc(unit, tbl);
                if (entry) {

                    sal_memcpy(entry->key, key, sizeof(uint32) * BITS2WORDS(tbl->key_size_bits));

                    sal_memcpy(entry->value, value, sizeof(uint32) * 
                               BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));

                    tbl_node = tbl->table[hash_idx];

                    if (tbl_node) {

                        /* Reverse logic to deal with coverity issue and
                         * remove assert as a means to abort task
                         */
                        if (DQ_EMPTY(&tbl_node->bucket_list)) {
                            assert(0);
                            return SOC_E_INTERNAL;
                        }

                        /* look for bucket with bucket hash match. If not create one */
                        DQ_TRAVERSE(&tbl_node->bucket_list, elem) {
                            bucket = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                            if (bucket->bucket_idx == bucket_idx) break;
                        } DQ_TRAVERSE_END(&tbl->bucket_list, elem);

                        if (bucket != NULL) {
                        if (bucket->bucket_idx == bucket_idx) {
                            /* if a bucket is found insert the entry on chain */
                            entry->type = TMU_HASH_NODE_CHAIN;
                            DQ_INSERT_HEAD(&bucket->chain_list, &entry->list_node);
                        } else {
                            /* if bucket is not found create a bucket node & push the entry into it */
                            entry->type = TMU_HASH_NODE_BUCKET;
                            entry->bucket_idx = bucket_idx;
                            DQ_INSERT_HEAD(&tbl_node->bucket_list, &entry->list_node);
                            DQ_INIT(&entry->chain_list);
                        }
                        }
                    } else {
                        tbl_node = tmu_hash_table_node_alloc(unit, tbl);
                        if (tbl_node) {
                            tbl->table[hash_idx] = tbl_node;
                            entry->type = TMU_HASH_NODE_BUCKET;
                            entry->bucket_idx = bucket_idx;
                            DQ_INSERT_HEAD(&tbl_node->bucket_list, &entry->list_node);
                            DQ_INIT(&entry->chain_list);

                        } else {
                            rv = SOC_E_MEMORY;
                        }
                    }
                } else {
                    rv = SOC_E_MEMORY;
                }

                if (SOC_SUCCESS(rv)) {
                    tbl->num_entries++;
#ifdef BCM_WARM_BOOT_SUPPORT
                    /* only update the hardware for cold boot */
                    if (!SOC_WARM_BOOT(unit)) {
#endif
                        /* call add hardware call back */
                        rv = tbl->commit_add_f(unit, tbl, entry, key_type);
#ifdef BCM_WARM_BOOT_SUPPORT
                }
#endif
                }

                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Key Insert Failed: %d !!!\n"), 
                               FUNCTION_NAME(), unit, rv));
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Key: %08x %08x - Hash:[0x%x] Bucket-Idx:[0x%x] \n"), 
                               key[1], key[0], hash_idx, bucket_idx));
                }
            }
        } else {
            rv = SOC_E_EXISTS;
        }
    }

    TMU_HTBL_UNLOCK(tbl);
    return rv;
}

/*
 * Function:
 *   tmu_hash_table_entry_update
 * Purpose:
 *   insert hash entry
 * Notes:
 */
int
tmu_hash_table_entry_update(int unit,
                            tmu_hash_table_t *tbl,
                            uint32 *key, 
                            uint32 *value,
                            soc_sbx_tmu_hash_key_type_e_t key_type)
{
    uint32 hash_idx=0, bucket_idx=0;
    tmu_hash_entry_t *entry = NULL;
    int rv = SOC_E_NONE;

    if (!tbl || !key || !value) return SOC_E_PARAM;

    TMU_HTBL_LOCK(tbl);

    rv = tmu_hash_function(unit, tbl, key, &hash_idx, &bucket_idx);
    if (SOC_SUCCESS(rv)) {
        rv = _tmu_hash_table_entry_find(unit, tbl, 
                                        hash_idx, bucket_idx, 
                                        key, &entry);
        if (SOC_SUCCESS(rv)) {
            sal_memcpy(entry->value, value, sizeof(uint32) * 
                       BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));

            /* call add hardware call back */
            rv = tbl->commit_add_f(unit, tbl, entry, key_type);

        } else {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d Hash Table Entry not found %d !!!\n"), 
                         FUNCTION_NAME(), unit, rv));
        }
    }

    TMU_HTBL_UNLOCK(tbl);
    return rv;
}


/*
 * Function:
 *   tmu_hash_table_entry_delete_ext
 * Purpose:
 *   delete hash entry, support no_commit option, which means
 *   only the driver software state is updated and no hardware
 *   commit command is issued.
 * Notes:
 */
STATIC int
tmu_hash_table_entry_delete_ext(int unit,
                tmu_hash_table_t *tbl,
                uint32 *key,
                soc_sbx_tmu_hash_key_type_e_t key_type,
                uint32 no_commit)
{
    uint32 hash_idx=0, bucket_idx=0;
    tmu_hash_entry_t *entry = NULL, *chain=NULL;
    tmu_hash_table_node_t *tbl_node = NULL;
    int rv = SOC_E_NONE;

    if (!tbl || !key) return SOC_E_PARAM;

    TMU_HTBL_LOCK(tbl);

    rv = tmu_hash_function(unit, tbl, key, &hash_idx, &bucket_idx);
    if (SOC_SUCCESS(rv)) {
        rv = _tmu_hash_table_entry_find(unit, tbl, 
                                        hash_idx, bucket_idx, 
                                        key, &entry);
        if (rv == SOC_E_NONE) { /* delete */
            /* if entry is a bucket node & last on the bucket, delete the table node */
            if (entry->type == TMU_HASH_NODE_BUCKET) {
                tbl_node = tbl->table[hash_idx];

        if (!no_commit) {
            /* call delete hardware call back */
            rv = tbl->commit_del_f(unit, tbl, entry, key_type);
        }

                /* if the chain list is empty convert one of the chain entry into bucket node */
                if (!DQ_EMPTY(&entry->chain_list)) {

                    chain = DQ_ELEMENT_GET(tmu_hash_entry_t*, DQ_HEAD(&entry->chain_list, dq_p_t), list_node);

                    sal_memcpy(entry->value, chain->value, 
                               sizeof(uint32) * BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));

                    sal_memcpy(entry->key, chain->key, 
                               sizeof(uint32) * BITS2WORDS(tbl->key_size_bits));

                    _tmu_hash_table_entry_free(unit, tbl, &chain);

                } else {
                    _tmu_hash_table_entry_free(unit, tbl, &entry);
                }

                if (DQ_EMPTY(&tbl_node->bucket_list)) {
                    tmu_hash_table_node_free(unit, tbl, &tbl_node);
                    tbl->table[hash_idx] = NULL;
                }

            } else {
                /* chain node */
                assert(entry->type == TMU_HASH_NODE_CHAIN);

        if (!no_commit) {
            /* call delete hardware call back */
            rv = tbl->commit_del_f(unit, tbl, entry, key_type);
        }

                _tmu_hash_table_entry_free(unit, tbl, &entry);
            }

            tbl->num_entries--;
        }
    }

    TMU_HTBL_UNLOCK(tbl);
    return rv;
}

/*
 * Function:
 *   tmu_hash_table_entry_delete
 * Purpose:
 *   delete hash entry
 * Notes:
 */
int
tmu_hash_table_entry_delete(int unit,
                            tmu_hash_table_t *tbl,
                            uint32 *key,
                            soc_sbx_tmu_hash_key_type_e_t key_type)
{
    return tmu_hash_table_entry_delete_ext(unit, tbl, key, key_type, FALSE);
}

/*
 * Function:
 *   tm_hash_table_create
 * Purpose:
 *   Create and initialize a hash table.
 * Notes:
 */
STATIC int _tmu_hash_table_create(int unit, tmu_hash_table_t **tbl,
                                  int max_num_entries,
                                  int key_size, 
                                  int device_hash_table_size,
                                  int max_chain_length,
                                  int max_chains,
                                  char *tbl_name,
                                  int tmu_root_table_id) 
{
    int rv = SOC_E_NONE;
    int table_mem_size;
    tmu_hash_table_t *ht;

    if (!tbl || !tbl_name) {
        return SOC_E_PARAM;
    }

    if(!(SOC_SBX_POWER_OF_TWO(device_hash_table_size))) {
        return SOC_E_PARAM;
    }

    ht = sal_alloc(sizeof(tmu_hash_table_t), "tmu_hash_tbl");
    if (ht == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(ht, 0, sizeof(tmu_hash_table_t));

#ifdef TMU_HASH_TBL_LOCK_SUPPORT
    ht->lock = sal_mutex_create(tbl_name);
    if (ht->lock == NULL) {
        sal_free(ht);
        return SOC_E_RESOURCE;
    }
#endif

    ht->max_num_entries = max_num_entries;
    ht->host_table_size = max_num_entries;
    /* table size must be a power of 2 */
    if(!(SOC_SBX_POWER_OF_TWO(max_num_entries))) {
        /* round up? to nearest power of 2 */
        soc_sbx_caladan3_round_power_of_two(unit, (unsigned int*) &ht->host_table_size, FALSE);
    }
    ht->max_chain_length= max_chain_length;
    ht->max_chains      = max_chains;
    ht->num_chains      = 0;
    ht->num_entries     = 0;
#ifdef TMU_HASH_HW_DEBUG
    ht->host_hw_fib_get = 1;
#else
    ht->host_hw_fib_get = 0;
#endif
    ht->key_size_bits   = key_size;
    ht->alloc_blk_cnt   = TMU_HASH_ALLOC_BLK_CNT;
    DQ_INIT(&ht->entry_free_list);
    DQ_INIT(&ht->table_node_free_list);
    ht->host_max_num_entries_msb_pos = soc_sbx_caladan3_msb_bit_pos(ht->host_table_size/TMU_HASH_DEFAULT_LOADING_FACTOR);
    ht->device_max_num_entries_msb_pos = soc_sbx_caladan3_msb_bit_pos(device_hash_table_size);
    ht->commit_add_f   = tmu_hash_entry_add_commit;
    ht->commit_del_f   = tmu_hash_entry_delete_commit;
    ht->tmu_hw_root_table_id = tmu_root_table_id;

    /* hash table is an array of pointers */
    table_mem_size = (ht->host_table_size/TMU_HASH_DEFAULT_LOADING_FACTOR) * sizeof(tmu_hash_table_node_t*);
    ht->table = sal_alloc(table_mem_size, tbl_name);
    if (ht->table == NULL) {
#ifdef TMU_HASH_TBL_LOCK_SUPPORT
        sal_mutex_destroy(ht->lock);
#endif
        sal_free(ht);
        return SOC_E_MEMORY;
    }

    sal_memset(ht->table, 0, table_mem_size);
    *tbl = ht;
    return rv;
}

/*
 * Function:
 *   tm_hash_table_destroy
 * Purpose:
 *   destroy a hash table.
 * Notes:
 */
STATIC int _tmu_hash_table_destory(int unit, tmu_hash_table_t **tmu_hash_tbl)
{
    int rv = SOC_E_NONE, i;
    tmu_hash_table_t *tbl;
    tmu_hash_entry_t *entry = NULL, *bucket = NULL;
    tmu_hash_table_node_t *tbl_node = NULL;
    dq_p_t elem, chain_elem;
    soc_sbx_tmu_hash_key_type_e_t key_type;

    if (!tmu_hash_tbl) {
        return SOC_E_PARAM;
    }
    tbl = *tmu_hash_tbl;

    if (tbl->key_size_bits <= _g_tmu_key_size_bits[SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS]) {
        key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS;
    } else if (tbl->key_size_bits <= _g_tmu_key_size_bits[SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS]) {
        key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS;
    } else if (tbl->key_size_bits <= _g_tmu_key_size_bits[SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS]) {
        key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS;
    } else if (tbl->key_size_bits <= _g_tmu_key_size_bits[SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS]) {
        key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS;        
    } else {
        key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS;
    }

    TMU_HTBL_LOCK(tbl);

    for (i=0; i < tbl->host_table_size/TMU_HASH_DEFAULT_LOADING_FACTOR; i++) {

        tbl_node = tbl->table[i];
        if (tbl_node) {

            DQ_TRAVERSE(&tbl_node->bucket_list, elem) {

                bucket = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 

                DQ_TRAVERSE(&bucket->chain_list, chain_elem) {

                    entry = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 

                    /* call delete hardware call back */
                    rv = tbl->commit_del_f(unit, tbl, entry, key_type);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Failed to delete key from tmu %d !!!\n"), 
                                   FUNCTION_NAME(), unit, rv));
                    }

                    _tmu_hash_table_entry_free(unit, tbl, &entry);

                } DQ_TRAVERSE_END(&bucket->chain_list, chain_elem);

                rv = tbl->commit_del_f(unit, tbl, bucket, key_type);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to delete key from tmu %d !!!\n"), 
                               FUNCTION_NAME(), unit, rv));
                }

                _tmu_hash_table_entry_free(unit, tbl, &bucket);

            } DQ_TRAVERSE_END(&tbl->bucket_list, elem);

            tmu_hash_table_node_free(unit, tbl, &tbl_node);
        }
    }

    _tmu_hash_table_node_blk_free(unit, tbl, TRUE);
    _tmu_hash_table_entry_blk_free(unit, tbl, TRUE);

    TMU_HTBL_UNLOCK(tbl);

#ifdef TMU_HASH_TBL_LOCK_SUPPORT
    sal_mutex_destroy(tbl->lock);
#endif
    sal_free(tbl->table);
    sal_free(tbl);
    return rv;
}

/*
 *
 * Function:
 *     _tmu_hash_fifo_lvl_intr_toggle
 * Purpose:
 *     Allocate a free fifo
 */
STATIC int
_tmu_hash_fifo_lvl_intr_toggle(int unit, int fifoid, uint8 enable)
{
    int field[] = {FREE_CHAIN_FIFO0_AEMPTY_DISINTf, FREE_CHAIN_FIFO1_AEMPTY_DISINTf, 
                   FREE_CHAIN_FIFO2_AEMPTY_DISINTf, FREE_CHAIN_FIFO3_AEMPTY_DISINTf};    
    uint32 regval=0;


    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    if (fifoid < 0 || fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO) return SOC_E_PARAM;

    /* enable/disable fifo threshold & interrupt mask */
    SOC_IF_ERROR_RETURN(READ_TMB_UPDATER_FIFO_PUSH_STATUS_MASKr(unit, &regval));
    LOG_INFO(BSL_LS_SOC_INTR,
             (BSL_META_U(unit,
                         "# %s: unit %d FIFO-ID(%d) FIFO push OLD mask: 0x%x\n"), 
              FUNCTION_NAME(), unit, fifoid, regval));

    soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUS_MASKr, 
                      &regval, field[fifoid], (enable)?0:1);

    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_FIFO_PUSH_STATUS_MASKr(unit, regval));

    LOG_INFO(BSL_LS_SOC_INTR,
             (BSL_META_U(unit,
                         "# %s: unit %d FIFO-ID(%d) FIFO push NEW mask: 0x%x\n"), 
              FUNCTION_NAME(), unit, fifoid, regval));

    /* un-mask the block interrupt if it is not */
    SOC_IF_ERROR_RETURN(soc_cmicm_intr3_enable(unit, 1<<SOC_SBX_CALADAN3_TMB_INTR_POS));
    sal_usleep(50*MILLISECOND_USEC);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_hash_fifo_id_alloc
 * Purpose:
 *     Allocate a free fifo
 */
STATIC int
_soc_sbx_caladan3_tmu_hash_fifo_id_alloc(int unit, 
                                         soc_sbx_tmu_hash_param_t *param,
                                         int *fifoid)
{
    int index, status = SOC_E_NONE;

    if (!param || !fifoid)
        return SOC_E_PARAM;

    *fifoid = -1;

    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    /* walk through fifo's used and look for matching fifo */
    for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO && *fifoid < 0; index++) {
        if (!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, index).chain_list)) {
            if (TMU_HASH_FIFO_MGR(unit, index).chain_length == param->chain_length &&
                TMU_HASH_FIFO_MGR(unit, index).key_size_bits == _g_tmu_key_size_bits[param->key_type]) {
                *fifoid = index;
            }
        }
    }

    for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO && *fifoid < 0; index++) {
        if (DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, index).chain_list)) {
            *fifoid = index;
        }
    }

    if (*fifoid < 0) {
        /* no fifo to accomodate */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d: No Free Hardware FIFO available to manage this Hash table !!!\n"), 
                   FUNCTION_NAME(), unit));
        status = SOC_E_RESOURCE;
    } else {
        TMU_HASH_FIFO_MGR(unit, *fifoid).chain_length = param->chain_length;
        TMU_HASH_FIFO_MGR(unit, *fifoid).key_size_bits = _g_tmu_key_size_bits[param->key_type];
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_hash_fifo_add_chain
 * Purpose:
 *     Add chain to free fifo
 */
STATIC int
_soc_sbx_caladan3_tmu_hash_fifo_add_chain(int unit, 
                                          soc_sbx_tmu_chain_alloc_handle_t *chain,
                                          uint8 *first_chain)
{
    int status = SOC_E_NONE;

    if (!chain || !first_chain)
        return SOC_E_PARAM;

    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    *first_chain = (DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, chain->fifoid).chain_list)) ? TRUE:FALSE;
    DQ_INSERT_TAIL(&TMU_HASH_FIFO_MGR(unit, chain->fifoid).chain_list, &chain->free_chain_list_node);
    TMU_HASH_FIFO_MGR(unit, chain->fifoid).refcount++;
    TMU_HASH_FIFO_MGR(unit, chain->fifoid).tbl_fifo_map[chain->table_id] = chain->fifoid;

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_hash_fifo_del_chain
 * Purpose:
 *     Remove chain to free fifo, used only for cleanup on API failure
 */
STATIC int
_soc_sbx_caladan3_tmu_hash_fifo_del_chain(int unit, 
                                          soc_sbx_tmu_chain_alloc_handle_t *chain)
{
    int status = SOC_E_NONE;

    if (!chain)
        return SOC_E_PARAM;

    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    DQ_REMOVE(&chain->free_chain_list_node);
    TMU_HASH_FIFO_MGR(unit, chain->fifoid).refcount--;
    TMU_HASH_FIFO_MGR(unit, chain->fifoid).tbl_fifo_map[chain->table_id] = -1;

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     _soc_sbx_caladan3_tmu_hash_fifo_deref_remove_chain
 * Purpose:
 *     removes chain from free fifo
 */
STATIC int
_soc_sbx_caladan3_tmu_hash_fifo_deref_remove_chain(int unit, int fifoid, uint8 *no_chain)
{
    int status = SOC_E_NONE;
    dq_p_t elem;

    if (fifoid < 0 || fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO || !no_chain) {
        return SOC_E_PARAM;
    }

    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    *no_chain = FALSE;
 
    /* NOTE: All associated memories in hardware is freed except chain. Chain's 
     * could be used or shared by other hash tables pre-existing in memory. If the chain
     * length is not 1 then chain tables on DRAM of TMU will be forced to leak. There is
     * no efficient mechanism to scoreboard and release the table */
    if(--TMU_HASH_FIFO_MGR(unit,fifoid).refcount == 0) {
            soc_sbx_tmu_chain_alloc_handle_t *chain=NULL, *oldchain=NULL;

            if (DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list)) {
                assert(0);
                return SOC_E_INTERNAL;
            }

            /* free all chain tables, there are no more hash tables using this FIFO */
            DQ_TRAVERSE(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, elem) {
                chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*, elem, free_chain_list_node);
                if (!oldchain) {
                    oldchain = chain;
                } else {
                    DQ_REMOVE(&oldchain->free_chain_list_node);
                    sal_free(oldchain);
                }
                
                /* free back the chain table to tmu dram pool */
                soc_sbx_caladan3_tmu_table_free(unit, chain->table_id);

                TMU_HASH_FIFO_MGR(unit, chain->fifoid).tbl_fifo_map[chain->table_id] = -1;

            } DQ_TRAVERSE_END(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, elem);
            if (chain != NULL) {    
            if (!DQ_EMPTY(&chain->free_chain_list_node)) {
                DQ_REMOVE(&chain->free_chain_list_node);
            }
            sal_free(chain);
            }
            *no_chain = TRUE;
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_table_param_validate
 * Purpose:
 *     validate alloc Hash table parameter
 */
int soc_sbx_caladan3_tmu_hash_table_param_validate(int unit, 
                                                   soc_sbx_tmu_hash_param_t *param)
{
    if (!param) {
        return SOC_E_PARAM;
    } 

    /* capacity validation? */

    /* validate key size bits */
    if (param->key_type < 0 || param->key_type >= SOC_SBX_CALADAN3_TMU_HASH_KEY_MAX) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d: Bad Key type = %d !!!\n"), 
                   FUNCTION_NAME(), unit, param->key_type));
        return SOC_E_PARAM;
    }

    if (!SOC_SBX_POWER_OF_TWO(param->num_hash_table_entries)) {
        soc_sbx_caladan3_round_power_of_two(unit, &param->num_hash_table_entries, FALSE);
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s: unit %d: Hash table size rounded to power or 2 = %d !!!\n"), 
                  FUNCTION_NAME(), unit, param->num_hash_table_entries));
    }

    if (SOC_SBX_CALADAN3_TMU_HASH_DEF_MAX_CHAIN_LEN < param->chain_length) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d: Bad Chain length = %d Max = %d!!!\n"), 
                   FUNCTION_NAME(), unit, param->chain_length,
                   SOC_SBX_CALADAN3_TMU_HASH_DEF_MAX_CHAIN_LEN));
        return SOC_E_PARAM;
    }

    /* validate & round chain tables */

    return SOC_E_NONE;
}

int _soc_sbx_tmu_hash_handle_validate(int unit, soc_sbx_tmu_hash_handle_t handle)
{
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle) return SOC_E_PARAM;

    if (hash_cfg->table_id < 0 || hash_cfg->table_id >= SOC_SBX_CALADAN3_TMU_MAX_TABLE ||
        hash_cfg->fifoid < 0 || hash_cfg->fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO || !hash_cfg->table) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d: Bad Hash table handle!!!\n"), 
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_hash_table_alloc
 * Purpose:
 *     Allocate a Hash table
 */
int soc_sbx_caladan3_tmu_hash_table_alloc(int unit, 
                                          soc_sbx_tmu_hash_param_t *param,
                                          soc_sbx_tmu_hash_handle_t *handle)
{
    int status = SOC_E_NONE, fifoid=0;
    soc_sbx_caladan3_table_attr_t table_attr, chain_attr;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = NULL;
    tmu_hash_table_t *hash_table = NULL;
    soc_sbx_tmu_chain_alloc_handle_t *chain=NULL;
    soc_sbx_caladan3_chain_table_attr_t chain_cfg;
    soc_sbx_tmu_hash_key_type_e_t sw_key_type;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* validate */
    status = soc_sbx_caladan3_tmu_hash_table_param_validate(unit, param);

    /* allocate TMU DRAM table space required to manage this hash table on device */
    sal_memset(&table_attr, 0, sizeof(table_attr));
    sal_memset(&chain_attr, 0, sizeof(chain_attr));
    table_attr.id = -1; /* for invalid checking during cleanup */
    chain_attr.id = -1; /* for invalid checking during cleanup */

    if (SOC_SUCCESS(status)) {
        hash_cfg = sal_alloc(sizeof(soc_sbx_tmu_hash_cfg_t), "tmu-hash-cfg-table");
        chain = sal_alloc(sizeof(soc_sbx_tmu_chain_alloc_handle_t), "tmu-hash-chain");
        if (!chain || !hash_cfg) {
            status = SOC_E_MEMORY;
        }
    }

    sw_key_type = param->key_type;
    if (SOC_SUCCESS(status)) {
        if (param->key_type == SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS) {
            /* 144 bits key types are same as 176 bits here 
             * it's only a define used at init time to control mode of chip 
             */
            param->key_type = SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS;
        }
        
        status = _soc_sbx_caladan3_tmu_hash_fifo_id_alloc(unit, param, &fifoid);
    }

    if (SOC_SUCCESS(status)) {
        table_attr.num_entries = param->num_hash_table_entries;
        table_attr.entry_size_bits = _g_tmu_key_size_bits[param->key_type] + 
                                     SOC_SBX_TMU_HASH_CONTROL_WORD_SIZE_BITS + 
                                     SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS;
        switch(sw_key_type) {
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
            table_attr.lookup = SOC_SBX_TMU_LKUP_EML_64;
            chain_attr.lookup = SOC_SBX_TMU_LKUP_EML2ND_64;
            break;
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS:
            if (_tmu_dbase[unit]->control_cfg.eml_144_mode == SOC_SBX_TMU_EML_144BITS_MODE_OFF) {
                /* device in EML176 mode, EML144 not supported */
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d in EML176 mode, EML144 lookup not supported !!!\n"),
                           FUNCTION_NAME(), unit));
                return SOC_E_PARAM;
            } else if (_tmu_dbase[unit]->control_cfg.eml_144_mode == SOC_SBX_TMU_EML_144BITS_MODE_UNKNOWN) {
                /* if one valid lookup is in EML144 mode, turn on EML144 mode  */
                _tmu_dbase[unit]->control_cfg.eml_144_mode = SOC_SBX_TMU_EML_144BITS_MODE_ON;

                status = _soc_sbx_caladan3_tmu_eml_144_mode_set(unit, TRUE);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d: Failed to enable EML 144 lookup for table: %d!!!\n"), 
                               FUNCTION_NAME(), unit, status));
                }
            }

            table_attr.lookup = SOC_SBX_TMU_LKUP_EML_144;
            chain_attr.lookup = SOC_SBX_TMU_LKUP_EML2ND_144;
            break;
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
            if (_tmu_dbase[unit]->control_cfg.eml_144_mode == SOC_SBX_TMU_EML_144BITS_MODE_ON) {
                /* device in EML144 mode, EML176 not supported */
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d in EML144 mode, EML176 lookup not supported !!!\n"),
                           FUNCTION_NAME(), unit));
                return SOC_E_PARAM;
            } else if (_tmu_dbase[unit]->control_cfg.eml_144_mode == SOC_SBX_TMU_EML_144BITS_MODE_UNKNOWN) {
                /* if one valid lookup is in EML176 mode, turn off EML144 mode  */
                _tmu_dbase[unit]->control_cfg.eml_144_mode = SOC_SBX_TMU_EML_144BITS_MODE_OFF;

                status = _soc_sbx_caladan3_tmu_eml_144_mode_set(unit, FALSE);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d: Failed to disable EML 144 lookup for table: %d!!!\n"), 
                               FUNCTION_NAME(), unit, status));
                }                
            }

            table_attr.lookup = SOC_SBX_TMU_LKUP_EML_176;
            chain_attr.lookup = SOC_SBX_TMU_LKUP_EML2ND_176;
            break;
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
            table_attr.lookup = SOC_SBX_TMU_LKUP_EML_304;
            chain_attr.lookup = SOC_SBX_TMU_LKUP_EML2ND_304;
            break;
        case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:
            table_attr.lookup = SOC_SBX_TMU_LKUP_EML_424;
            chain_attr.lookup = SOC_SBX_TMU_LKUP_EML2ND_424;
            break;
        default:
            assert(0);
            break;
        }

        sal_memset(&chain_cfg, 0, sizeof(chain_cfg));
        chain_cfg.fifo_bitmap = 1 << fifoid;
        chain_cfg.length      = param->chain_length;
        chain_cfg.hw_managed  = TRUE;

        status = soc_sbx_caladan3_tmu_chain_table_alloc(unit, &table_attr, &chain_cfg); /* root table alloc */
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d: Failed to allocate DRAM space for hash table!!!\n"), 
                       FUNCTION_NAME(), unit));
        } else {
            /* allocate chain table */
            chain_attr.flags = SOC_SBX_TMU_TABLE_FLAG_CHAIN;
            chain_attr.num_entries = param->num_chain_table_entries;
            chain_attr.entry_size_bits = (_g_tmu_key_size_bits[param->key_type] + 
                                          SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) * param->chain_length;

            status = soc_sbx_caladan3_tmu_chain_table_alloc(unit, &chain_attr, &chain_cfg); /* chain table alloc */
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d: Failed to allocate DRAM space for chain table!!!\n"), 
                           FUNCTION_NAME(), unit));
            }
        }
    }

    if (SOC_SUCCESS(status)) {
        /* allocate software space required to manage the given hash table */
        status = _tmu_hash_table_create(unit, &hash_table, 
                                        param->capacity, _g_tmu_key_size_bits[param->key_type],
                                        param->num_hash_table_entries, param->chain_length, 
                                        param->num_chain_table_entries, "tmu-hash-table", table_attr.id);
        if (SOC_FAILURE(status)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d: Failed to allocate Hash Datastructure table: %d!!!\n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    /* clear root table entries on QT since this is faster than CI initialization */
    if (SOC_SUCCESS(status) && !TMU_CI_DDR_PATTERN_INIT
        && !SAL_BOOT_BCMSIM && !SAL_BOOT_PLISIM) {
        LOG_INFO(BSL_LS_SOC_MEM,
                 (BSL_META_U(unit,
                             "n### Clearing table id (%d) entries (%d) \n"),
                  table_attr.id, param->num_hash_table_entries));
        status = soc_sbx_caladan3_tmu_table_clear(unit, table_attr.id, param->num_hash_table_entries);        
    }
#if 0
    /* clear  chain table */
    LOG_CLI((BSL_META_U(unit,
                        "### clearing chain table ###\n")));
    soc_sbx_caladan3_tmu_table_clear(unit, chain_attr.id, param->num_chain_table_entries);
#endif
    /* generate free list of chains & feed appropriately onto free chain fifo */
    if (SOC_SUCCESS(status)) {
        uint8 first_chain=FALSE;
        chain->table_id = chain_attr.id;
        chain->fifoid = fifoid;
        chain->free_ptr = 0;
        chain->size = param->num_chain_table_entries;

        /* exclude table id 0, entry 0 */
        if (chain->table_id == 0) {
            chain->free_ptr++;
            chain->size--;
        }

        /* Insert to the chain list on fifo manager */
        status = _soc_sbx_caladan3_tmu_hash_fifo_add_chain(unit, chain, &first_chain);
        
        if (SOC_SUCCESS(status)) {
            /* if this is the first table enabled, start fifo thread */
            if (FALSE == _tmu_hash_fifo_mgr[unit]->thread_running) {
                status = tmu_hash_fifo_recycle_thread_toggle(unit, START);
            }

            if (SOC_SUCCESS(status)) {
                /* update TMU hash database */
                DQ_INSERT_HEAD(&_tmu_hash_dbase[unit]->hash_list, &hash_cfg->tmu_hash_dbase_list_node);
                hash_cfg->table    = hash_table;
                hash_cfg->table_id = table_attr.id;
                hash_cfg->fifoid   = fifoid;
                sal_memcpy(&hash_cfg->param, param, sizeof(soc_sbx_tmu_hash_param_t));

                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "### HASH table id (%d) FIFOID (%d) \n"),
                             table_attr.id, fifoid));
                
                /* set handle */
                *handle = hash_cfg;

                /* this place is guaranteed success & databases are sane, so kick start interrupts */
                /* fifo manager mutex protects the critical session between thread & this function */
                /* if this is the first chain on this fifo, enable interrupts */
                if (first_chain) {
                    status = _tmu_hash_fifo_lvl_intr_toggle(unit, fifoid, START);
                    /* feed the free fifo */
                    status = soc_sbx_caladan3_tmu_hash_fifo_feed(unit, fifoid);
                }
            }

            /* if failure remove this chain from chain list, removing from chain implicitly free's fifoid */
            if (SOC_FAILURE(status)) {
                _soc_sbx_caladan3_tmu_hash_fifo_del_chain(unit, chain);
            }
        }
    }

    /* clean-up */
    if (SOC_FAILURE(status)) {
        if (hash_cfg) sal_free(hash_cfg);
        if (chain) sal_free(chain);
        if ((int)table_attr.id >= 0) soc_sbx_caladan3_tmu_table_free(unit, table_attr.id);
        if ((int)chain_attr.id >= 0) soc_sbx_caladan3_tmu_table_free(unit, chain_attr.id);
        if (hash_table) _tmu_hash_table_destory(unit, &hash_table);
    }
    
    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_table_free
 * Purpose:
 *     Free an preallocated hash table
 */
int soc_sbx_caladan3_tmu_hash_table_free(int unit, soc_sbx_tmu_hash_handle_t handle)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;
    uint8 no_chain = FALSE;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);

    if (!handle) {
        return SOC_E_PARAM;
    }

    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {

        /* dereference fifo manager & fifo manager will free back chain's if applicable */
        _soc_sbx_caladan3_tmu_hash_fifo_deref_remove_chain(unit, hash_cfg->fifoid, &no_chain);

        /* destroy the hash table */
        _tmu_hash_table_destory(unit, &hash_cfg->table);

        /* remove hash table from hash database */
        DQ_REMOVE(&hash_cfg->tmu_hash_dbase_list_node);

        /* free back hash table to TMU pool */
        soc_sbx_caladan3_tmu_table_free(unit, hash_cfg->table_id);

        /* if no more hash table's stop the fifo/recyle thread */
        if (DQ_EMPTY(&_tmu_hash_dbase[unit]->hash_list)) {
            status = tmu_hash_fifo_recycle_thread_toggle(unit, STOP);
        }

        if (no_chain) {
            /* disable fifo threshold & interrupt mask */
            status = _tmu_hash_fifo_lvl_intr_toggle(unit, hash_cfg->fifoid, STOP);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d: could not disable fifo level interrupts %d !!!\n"), 
                           FUNCTION_NAME(), unit, status));
            }
        }

        sal_free(hash_cfg);
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/**
 * helper functions
 */

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_add
 * Purpose:
 *     add entry to hash table
 */
int soc_sbx_caladan3_tmu_hash_entry_add(int unit, 
                                        soc_sbx_tmu_hash_handle_t handle,
                                        uint32 *key, uint32 *value)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key || !value) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_table_entry_insert(unit, hash_cfg->table, key, 
                                             value, hash_cfg->param.key_type);

    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_delete
 * Purpose:
 *     delete entry from hash table
 */
int soc_sbx_caladan3_tmu_hash_entry_delete(int unit, 
                                           soc_sbx_tmu_hash_handle_t handle, 
                                           uint32 *key)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_table_entry_delete(unit, hash_cfg->table, key, hash_cfg->param.key_type);
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_update
 * Purpose:
 *     update entry on hash table
 */
int soc_sbx_caladan3_tmu_hash_entry_update(int unit,
                                           soc_sbx_tmu_hash_handle_t handle, 
                                           uint32 *key, 
                                           uint32 *value)
{

    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key || !value) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_table_entry_update(unit, hash_cfg->table, key, 
                                             value, hash_cfg->param.key_type);
    }

    TMU_HASH_UNLOCK(unit);

    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_add
 * Purpose:
 *     add entry to hash table
 */
int soc_sbx_caladan3_tmu_hash_entry_get(int unit, 
                                        soc_sbx_tmu_hash_handle_t handle, 
                                        uint32 *key, 
                                        uint32 *value)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key || !value) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_table_entry_get(unit, handle, hash_cfg->table, key, value);
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_entry_hw_get
 * Purpose:
 *     get hash entry to from hardware
 */
int soc_sbx_caladan3_tmu_hash_entry_hw_get(int unit, 
                                        soc_sbx_tmu_hash_handle_t handle, 
                                        uint32 *key, 
                                        uint32 *value)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key || !value) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_table_entry_hw_get(unit, handle, hash_cfg->table, key, value);
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *    soc_sbx_caladan3_tmu_hash_iterator_first
 * Purpose:
 *    initialize iterator
 * returns SOC_E_LIMIT if uninitialized
 */
int soc_sbx_caladan3_tmu_hash_iterator_first(int unit, 
                                             soc_sbx_tmu_hash_handle_t handle, 
                                             uint32 *key)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_iterator_first(unit, hash_cfg->table, 
                                         0, key);
    }
    
    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_iterator_next
 * Purpose:
 *   iterates
 *   returns SOC_E_LIMIT if uninitialized
 */
int soc_sbx_caladan3_tmu_hash_iterator_next(int unit, 
                                            soc_sbx_tmu_hash_handle_t handle,
                                            uint32 *key,
                                            uint32 *next_key)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !key || !next_key) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status  = tmu_hash_iterator_next(unit, hash_cfg->table,
                                         key, next_key);
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_table_id_get
 * Purpose:
 *     get table id from hash handle table
 */
int soc_sbx_caladan3_tmu_hash_table_id_get(int unit, 
                                        soc_sbx_tmu_hash_handle_t handle, 
                                        uint32 *id)
{
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle || !id) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
      *id = hash_cfg->table->tmu_hw_root_table_id;
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hw_hash_init
 * Purpose:
 *     initialize tmu hardware hash table
 */
static int _soc_sbx_caladan3_tmu_hw_hash_init(int unit)
{
    int status = SOC_E_NONE, index, entry_idx;
    int hash_tbl_size, hash_tbl_entry_size;
    uint32 *dmabuf=NULL;
    int tma_hash0_memory[TMU_HASH_MAX_TABLE] = {TMA_HASH0_RANDTABLE0m, TMA_HASH0_RANDTABLE1m,
                                                TMA_HASH0_RANDTABLE2m, TMA_HASH0_RANDTABLE3m,
                                                TMA_HASH0_RANDTABLE4m, TMA_HASH0_RANDTABLE5m,
                                                TMA_HASH0_RANDTABLE6m, TMA_HASH0_RANDTABLE7m};
    int tma_hash1_memory[TMU_HASH_MAX_TABLE] = {TMA_HASH1_RANDTABLE0m, TMA_HASH1_RANDTABLE1m,
                                                TMA_HASH1_RANDTABLE2m, TMA_HASH1_RANDTABLE3m,
                                                TMA_HASH1_RANDTABLE4m, TMA_HASH1_RANDTABLE5m,
                                                TMA_HASH1_RANDTABLE6m, TMA_HASH1_RANDTABLE7m};
    int tmb_hash0_memory[TMU_HASH_MAX_TABLE] = {TMB_HASH0_RANDTABLE0m, TMB_HASH0_RANDTABLE1m,
                                                TMB_HASH0_RANDTABLE2m, TMB_HASH0_RANDTABLE3m,
                                                TMB_HASH0_RANDTABLE4m, TMB_HASH0_RANDTABLE5m,
                                                TMB_HASH0_RANDTABLE6m, TMB_HASH0_RANDTABLE7m};

    if (_tmu_hash_dbase[unit]->hashtable && _tmu_hash_dbase[unit]->hash_adjust) {

        hash_tbl_size = soc_mem_index_max(unit, tmb_hash0_memory[0]) + 1;
        hash_tbl_entry_size = soc_mem_entry_words(unit, tmb_hash0_memory[0]); /* num words */
        sal_srand(0); /* use clock */

        dmabuf = (uint32 *) soc_cm_salloc(unit, sizeof(uint32) *
                                         hash_tbl_size * 
                                         hash_tbl_entry_size,
                                         "tmu-hash-table-dma-buffer" ); 
        sal_memset(dmabuf, 0, sizeof(uint32) * hash_tbl_size * hash_tbl_entry_size);
    
        for (index=0; index < COUNTOF(tma_hash0_memory) && SOC_SUCCESS(status); index++) {
            for (entry_idx = 0; entry_idx < hash_tbl_size; entry_idx++) {
                _tmu_hash_dbase[unit]->hashtable[index * TMU_HASH_MAX_TABLE_ENTRY + entry_idx] = sal_rand();
                soc_mem_field_set(unit, tmb_hash0_memory[index],
                                  (dmabuf + hash_tbl_entry_size * entry_idx),
                                  RAND_NUMf, 
                                  &_tmu_hash_dbase[unit]->hashtable[index * TMU_HASH_MAX_TABLE_ENTRY + entry_idx]);
            }

            if (SOC_SUCCESS(status)) {
                /*    coverity[negative_returns : FALSE]    */
                status = soc_mem_write_range(unit, tma_hash0_memory[index], MEM_BLOCK_ANY, 0,
                                             (hash_tbl_size - 1), dmabuf);
            }

            if (SOC_SUCCESS(status)) {
                /*    coverity[negative_returns : FALSE]    */
                status = soc_mem_write_range(unit, tma_hash1_memory[index], MEM_BLOCK_ANY, 0,
                                             (hash_tbl_size - 1), dmabuf);
            }

            if (SOC_SUCCESS(status)) {
                /*    coverity[negative_returns : FALSE]    */
                status = soc_mem_write_range(unit, tmb_hash0_memory[index], MEM_BLOCK_ANY, 0,
                                             (hash_tbl_size - 1), dmabuf);
            }
        }
        soc_cm_sfree(unit, dmabuf);

        /* for now ignore adjust */
        sal_memset(_tmu_hash_dbase[unit]->hash_adjust, 0, sizeof(uint32) * TMU_HASH_MAX_ADJUST_SELECT);
    } else {
        status = SOC_E_INIT;
    }

    return status;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_init
 * Purpose:
 *     initialize tmu hash drivers
 */
int soc_sbx_caladan3_tmu_hash_init(int unit, uint8 bypass_hash)
{
    int status = SOC_E_NONE, index, hpcmidx;
    soc_heap_mem_chunk_t *hpcm=NULL;

    if (_tmu_hash_dbase[unit]) {
        return SOC_E_INIT;
    }
  
    _tmu_hash_dbase[unit] = sal_alloc(sizeof(soc_sbx_tmu_hash_dbase_t), "tmu-hash-dbase");
    if (!_tmu_hash_dbase[unit]) {
        status = SOC_E_MEMORY;
    } else {
        sal_memset(_tmu_hash_dbase[unit], 0, sizeof(soc_sbx_tmu_hash_dbase_t));
        DQ_INIT(&_tmu_hash_dbase[unit]->hash_list);

        if (bypass_hash) {
            _tmu_hash_dbase[unit]->hashtable = NULL;
            _tmu_hash_dbase[unit]->hash_adjust = NULL;
        } else {
            _tmu_hash_dbase[unit]->hashtable = sal_alloc(SOC_SBX_HASHTABLE_SIZE,
                                                         "tmu-rand-hash-table");

            _tmu_hash_dbase[unit]->hash_adjust = sal_alloc(sizeof(uint32) * TMU_HASH_MAX_ADJUST_SELECT, 
                                                           "tmu-hash-adjust");
            
            if (_tmu_hash_dbase[unit]->hashtable && _tmu_hash_dbase[unit]->hash_adjust) {
                /* initialize hardware hash table */
                status = _soc_sbx_caladan3_tmu_hw_hash_init(unit);
            } else {
                status = SOC_E_MEMORY;
            }
        }

        if (SOC_SUCCESS(status)) {
            _tmu_hash_dbase[unit]->mutex = sal_mutex_create("tmu-hash-mutex");
            if (_tmu_hash_dbase[unit]->mutex == NULL) {
                status = SOC_E_RESOURCE;
            }
        }

        if (SOC_FAILURE(status)) {
            if (_tmu_hash_dbase[unit]->hashtable) sal_free(_tmu_hash_dbase[unit]->hashtable);
            if (_tmu_hash_dbase[unit]->hash_adjust) sal_free(_tmu_hash_dbase[unit]->hash_adjust);
            sal_free(_tmu_hash_dbase[unit]);
            return status;
        }
            
        /* initialize fifo manager */
        _tmu_hash_fifo_mgr[unit] = sal_alloc(sizeof(soc_sbx_tmu_hash_fifo_mgr_t), "tmu-hash-fifo-mgr");
        sal_memset(_tmu_hash_fifo_mgr[unit], 0, sizeof(soc_sbx_tmu_hash_fifo_mgr_t));

        _tmu_hash_fifo_mgr[unit]->mutex = sal_mutex_create("tmu-hash-fifo-mgr-mutex");
        if (_tmu_hash_fifo_mgr[unit]->mutex == NULL) {
            status = SOC_E_RESOURCE;
        } else {
            _tmu_hash_fifo_mgr[unit]->fifo_trigger = sal_sem_create("tmu-hash-fifo-trigger",
                                                                    sal_sem_BINARY, 0);
            if (_tmu_hash_fifo_mgr[unit]->fifo_trigger == NULL) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s Could not create TMU fifo thread trigger semaphore on unit %d\n"),
                           FUNCTION_NAME(), unit));
                status = SOC_E_RESOURCE;
            } else {

                _tmu_hash_fifo_mgr[unit]->thread_running = FALSE;

                _tmu_hash_fifo_mgr[unit]->low_lvl_threshold = TMU_HASH_DEF_LOW_MARK_THRESH;

                /* allocate dma buffer for FIFO DMA */
                _tmu_hash_fifo_mgr[unit]->dma_buffer_len = soc_mem_index_max(unit,TMB_UPDATER_FREE_CHAIN_FIFO0m) - \
                                                           soc_mem_index_min(unit,TMB_UPDATER_FREE_CHAIN_FIFO0m) + 1;

                _tmu_hash_fifo_mgr[unit]->dma_buffer_len = (_tmu_hash_fifo_mgr[unit]->dma_buffer_len *
                                                            (100 - _tmu_hash_fifo_mgr[unit]->low_lvl_threshold)) / 100;
                
                _tmu_hash_fifo_mgr[unit]->dma_buffer = \
                    (uint32 *) soc_cm_salloc(unit, _tmu_hash_fifo_mgr[unit]->dma_buffer_len * sizeof(uint32), 
                                             "tmu-hash-free-fifo-dma-buffer" );

                if (_tmu_hash_fifo_mgr[unit]->dma_buffer == NULL) {
                    status = SOC_E_RESOURCE;
                } else {
                    
                    TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len = \
                               soc_mem_entry_words(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm) * 
                        SOC_SBX_TMU_RECYCLE_RESP_DMA_MGR_RING_SIZE;
                    
                    TMU_HASH_RECYCLE_MGR(unit).dma_buffer = \
                        (uint32 *) soc_cm_salloc(unit, 
                                                 TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len *
                                                 sizeof(uint32) * 
                                                 soc_mem_entry_words(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm),
                                                 "tmu-hash-recycle-dma-buffer");
                    
                    if (TMU_HASH_RECYCLE_MGR(unit).dma_buffer == NULL) {
                        status = SOC_E_RESOURCE;
                    } else {
                        sal_memset((uint32*)&TMU_HASH_RECYCLE_MGR(unit).dma_buffer[0], 0, 
                                   sizeof(uint32) * TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len * 
                                   soc_mem_entry_words(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm));
                        TMU_HASH_RECYCLE_MGR(unit).ring_pointer = &TMU_HASH_RECYCLE_MGR(unit).dma_buffer[0];
                        /* Setup CMIC_CMC0_FIFO_CH1_RD_DMA for recycle ring processing */
                        TMU_HASH_RECYCLE_MGR(unit).channel = 1;
                    }
                }
            }
        }

        for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO && SOC_SUCCESS(status); index++) {
            DQ_INIT(&TMU_HASH_FIFO_MGR(unit,index).chain_list);
            DQ_INIT(&TMU_HASH_FIFO_MGR(unit,index).recycle_entry_list);
            DQ_INIT(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list);
            DQ_INIT(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.full_hpcm_list);
            TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.max_free_lvl = SOC_SBX_TMU_DEF_FREE_LVL;
            TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.max_alloc_lvl = SOC_SBX_TMU_DEF_ALLOC_LVL;
            sal_memset(TMU_HASH_FIFO_MGR(unit, index).tbl_fifo_map, -1, 
                       sizeof(TMU_HASH_FIFO_MGR(unit, index).tbl_fifo_map));
        }

        for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO && SOC_SUCCESS(status); index++) {
            for (hpcmidx=0; hpcmidx < TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.max_free_lvl && SOC_SUCCESS(status); hpcmidx++) {
                status = hpcm_init(unit, SOC_SBX_TMU_DEF_HPCM_SIZE, sizeof(soc_sbx_tmu_free_fifo_entry_t), &hpcm);   
                if (SOC_SUCCESS(status)) {
                    DQ_INSERT_HEAD(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list, hpcm);
                }
            }
        }

        if (SOC_FAILURE(status)) {
            if (_tmu_hash_dbase[unit]) {
                if (_tmu_hash_dbase[unit]->mutex) sal_mutex_destroy(_tmu_hash_dbase[unit]->mutex);
                if (_tmu_hash_fifo_mgr[unit]->dma_buffer) soc_cm_sfree(unit, (void*)_tmu_hash_fifo_mgr[unit]->dma_buffer);
                if (TMU_HASH_RECYCLE_MGR(unit).dma_buffer) soc_cm_sfree(unit, (void*)TMU_HASH_RECYCLE_MGR(unit).dma_buffer);
                sal_free(_tmu_hash_dbase[unit]);
            }

            if (_tmu_hash_fifo_mgr[unit]->mutex) sal_free(_tmu_hash_fifo_mgr[unit]->mutex);
            if (_tmu_hash_fifo_mgr[unit]->fifo_trigger) sal_sem_destroy(_tmu_hash_fifo_mgr[unit]->fifo_trigger);

            for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO; index++) {
                if (!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list)) {
                    dq_p_t elem;
                    DQ_TRAVERSE(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list, elem) {
                        hpcm = DQ_ELEMENT_GET(soc_heap_mem_chunk_t *, elem, list_node);
                        DQ_REMOVE(&hpcm->list_node);
                        sal_free(hpcm);
                    } DQ_TRAVERSE_END(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list, elem);
                }
            }
        }
    }

    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_uninit
 * Purpose:
 *     uninitialize tmu hash drivers
 */
int soc_sbx_caladan3_tmu_hash_uninit(int unit)
{
    int count = 100, index, status COMPILER_ATTRIBUTE((unused)) ;
    soc_heap_mem_chunk_t *hpcm=NULL;

    soc_mem_fifo_dma_stop(unit, TMU_HASH_RECYCLE_MGR(unit).channel);
    _tmu_hash_fifo_mgr[unit]->thread_running = FALSE;
    
    while((((uint32)_tmu_hash_fifo_mgr[unit]->thread_id) > 0) && (--count > 0))    {
        sal_usleep(1000);
    }

    /* free hpcm_mgr */
    for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO; index++) {
        if (!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list)) {
            dq_p_t elem;
            DQ_TRAVERSE(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list, elem) {
                hpcm = DQ_ELEMENT_GET(soc_heap_mem_chunk_t *, elem, list_node);
                DQ_REMOVE(&hpcm->list_node);
                status = hpcm_destroy(unit, hpcm);
            } DQ_TRAVERSE_END(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.free_hpcm_list, elem);
        }
    }

    sal_sem_destroy(_tmu_hash_fifo_mgr[unit]->fifo_trigger);
    soc_cm_sfree(unit, (void*)_tmu_hash_fifo_mgr[unit]->dma_buffer);
    soc_cm_sfree(unit, (void*)TMU_HASH_RECYCLE_MGR(unit).dma_buffer);
    sal_mutex_destroy(_tmu_hash_fifo_mgr[unit]->mutex);
    sal_free(_tmu_hash_fifo_mgr[unit]);
    _tmu_hash_fifo_mgr[unit]=NULL;

    sal_mutex_destroy(_tmu_hash_dbase[unit]->mutex);
    if (_tmu_hash_dbase[unit]->hashtable) 
        sal_free(_tmu_hash_dbase[unit]->hashtable);
    if (_tmu_hash_dbase[unit]->hash_adjust) 
        sal_free(_tmu_hash_dbase[unit]->hash_adjust);
    sal_free(_tmu_hash_dbase[unit]);
    _tmu_hash_dbase[unit] = NULL;

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_miss_pattern_int
 * Purpose:
 *   Intialize miss pattern hash entry. Hash lookup returns this value 
 *   when hash lookup fails. For now 
 */
int soc_sbx_caladan3_tmu_hash_miss_pattern_init(int unit, unsigned int *entry)
{
    int status = SOC_E_NONE;
    int index;
    int mem[] = {TMB_COMPLETION_MISS_ENTRY0_31_0r,
                 TMB_COMPLETION_MISS_ENTRY0_63_32r,
                 TMB_COMPLETION_MISS_ENTRY0_95_64r,
                 TMB_COMPLETION_MISS_ENTRY0_127_96r};

    if (!entry) return SOC_E_PARAM;

    for (index=0; index < COUNTOF(mem) && SOC_SUCCESS(status); index++) {
        status = soc_reg32_set(unit, mem[index], REG_PORT_ANY, 0, entry[index]);
    }

    return status;
}


/*
 *
 * Function:
 *     tmu_hash_fifo_recycle_thread_toggle
 * Purpose:
 *     start/stop tmu fifo/recycle thread
 */
int tmu_hash_fifo_recycle_thread_toggle(int unit,
                                      uint8 start /* TRUE-start, FALSE-stop */) 
{
    int status = SOC_E_NONE;
    uint32 regval=0;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* thread not started in LRP_BYPASS */
    if (start && !(soc_property_get(unit, spn_LRP_BYPASS, 0))) {
        if (_tmu_hash_fifo_mgr[unit]->thread_running == TRUE) { /* NOP */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Thread already started on unit %d\n"),
                       FUNCTION_NAME(), unit));
        } else {
            /* start recycle thread */
            _tmu_hash_fifo_mgr[unit]->thread_priority = 60;
            _tmu_hash_fifo_mgr[unit]->thread_running = TRUE;
            _tmu_hash_fifo_mgr[unit]->thread_id = sal_thread_create("tmuHashFifoMgr",
                                                                    SAL_THREAD_STKSZ,
                                                                    _tmu_hash_fifo_mgr[unit]->thread_priority,
                                                                    tmu_hash_fifo_manager_thread,
                                                                    INT_TO_PTR(unit));
            if (_tmu_hash_fifo_mgr[unit]->thread_id == SAL_THREAD_ERROR) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s Could not create TMU recycle thread on unit %d\n"),
                           FUNCTION_NAME(), unit));
                _tmu_hash_fifo_mgr[unit]->thread_running = FALSE;
                status = SOC_E_RESOURCE;
            }

            /* start rx fifo dma */
            if (SOC_SUCCESS(status) && !SAL_BOOT_PLISIM) {
                READ_CMIC_CMC1_CONFIGr( unit, &regval );
                soc_reg_field_set( unit, CMIC_CMC1_CONFIGr, &regval, ENABLE_SBUSDMA_CH0_FLOW_CONTROLf, 1 );
                soc_reg_field_set( unit, CMIC_CMC1_CONFIGr, &regval, ENABLE_SBUSDMA_CH1_FLOW_CONTROLf, 1 );
                soc_reg_field_set( unit, CMIC_CMC1_CONFIGr, &regval, ENABLE_SBUSDMA_CH2_FLOW_CONTROLf, 1 );
                WRITE_CMIC_CMC1_CONFIGr( unit, regval );
#if 0
                READ_CMIC_CMC0_CONFIGr(unit, &regval);
                soc_reg_field_set(unit, CMIC_CMC0_CONFIGr, &regval, ENABLE_SBUSDMA_CH1_FLOW_CONTROLf, 1);
                WRITE_CMIC_CMC0_CONFIGr(unit, regval);
#endif
                status = soc_mem_fifo_dma_start(unit, TMU_HASH_RECYCLE_MGR(unit).channel,
                                                TMB_UPDATER_RECYCLE_CHAIN_FIFOm,
                                                MEM_BLOCK_ANY,
                                                TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len /
                                                soc_mem_entry_words(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm),
                                                (uint32 *)TMU_HASH_RECYCLE_MGR(unit).dma_buffer);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s Could not start TMU recycle FIFO DMA on unit %d: Error: %d\n"),
                               FUNCTION_NAME(), unit, status));
                }
            }
        }
    } else {
        if (_tmu_hash_fifo_mgr[unit]->thread_running == FALSE) { /* NOP */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s Thread already stopped on unit %d\n"),
                       FUNCTION_NAME(), unit));
        } else {
            soc_mem_fifo_dma_stop(unit, TMU_HASH_RECYCLE_MGR(unit).channel);

            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s Stopping TMU fifo/recycle thread on unit %d\n"),
                         FUNCTION_NAME(), unit));
            _tmu_hash_fifo_mgr[unit]->thread_running = FALSE;
        }
    }
    
    TMU_HASH_UNLOCK(unit);
    return status;
}


/*
 *
 * Function:
 *     tmu_hash_entry_add_commit
 * Purpose:
 *     Commit's the added entry to hardware
 */

int tmu_hash_entry_add_commit(int unit, 
                              tmu_hash_table_t *table,
                              tmu_hash_entry_t *entry, 
                              soc_sbx_tmu_hash_key_type_e_t key_type)
{
    soc_sbx_caladan3_tmu_cmd_t *cmd;
    int status = SOC_E_NONE;
    int retry_count = 10;

    if (SAL_BOOT_PLISIM) {
        return SOC_E_NONE;
    }

    /* Build command's for command fifo */
    status = tmu_cmd_alloc(unit, &cmd);
    if (SOC_SUCCESS(status)) {
        cmd->opcode = SOC_SBX_TMU_CMD_EML_INS_END;
        cmd->cmd.eml.key_type = key_type;
        cmd->cmd.eml.key = entry->key;
        cmd->cmd.eml.value = &entry->value[0];
        cmd->cmd.eml.filter = FALSE;
        cmd->cmd.eml.table_id = table->tmu_hw_root_table_id;

	TMU_LOCK(unit)

        status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                               cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
        if (SOC_SUCCESS(status)) {
            status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                   cmd, NULL, 0);

	    while((status==SOC_E_FAIL) && retry_count--){
	        sal_usleep(10000);
		retry_flag=0x111;
		status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
						       cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
		
		if(status){
		    break;
		}
		
		status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
						       cmd, NULL, 0);
		if(!status) {
		  LOG_CLI((BSL_META_U(unit,
                                      "%s: Successed with %d retry, Ignore previous error message\n"),
                           FUNCTION_NAME(), 10 - retry_count));    
		}
		
	    }
	    
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Invalid response !!!\n"), 
                           FUNCTION_NAME(), unit));
            }
        }

	TMU_UNLOCK(unit)    
    }
      
    tmu_cmd_free(unit, cmd);
    return status;
}

/*
 *
 * Function:
 *     tmu_hash_entry_delete_commit
 * Purpose:
 *     remove the deleted entry from hardware
 */
int tmu_hash_entry_delete_commit(int unit, 
                                 tmu_hash_table_t *table, 
                                 tmu_hash_entry_t *entry, 
                                 soc_sbx_tmu_hash_key_type_e_t key_type)
{
    soc_sbx_caladan3_tmu_cmd_t *cmd;
    int status = SOC_E_NONE;
    int retry_count = 10;


    if (SAL_BOOT_PLISIM) {
        return SOC_E_NONE;
    }

    /* Build command's for command fifo */
    status = tmu_cmd_alloc(unit, &cmd);
    if (SOC_SUCCESS(status)) {
        cmd->opcode = SOC_SBX_TMU_CMD_EML_DELETE;
        cmd->cmd.eml.key_type = key_type;
        cmd->cmd.eml.key = entry->key;
        cmd->cmd.eml.filter = FALSE; /* bulk delete filter */
        cmd->cmd.eml.table_id = table->tmu_hw_root_table_id;

	TMU_LOCK(unit);

        status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                               cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
        if (SOC_SUCCESS(status)) {
            status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                                   cmd, NULL, 0);

	    while((status==SOC_E_FAIL) && retry_count--){
	        sal_usleep(10000);
		retry_flag=0x222;
		status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
						       cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
		if(status){
		    break;
		}

		status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
						       cmd, NULL, 0);

		if(!status) {
		     LOG_CLI((BSL_META_U(unit,
                                         "%s: Successed with %d retry, Ignore previous error message\n"),
                              __FUNCTION__, 10 - retry_count));    
		}
	    }
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Invalid response !!!\n"), 
                           FUNCTION_NAME(), unit));
            }
        }

	TMU_UNLOCK(unit);
    }

    tmu_cmd_free(unit, cmd);
    return status;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hw_hash_table_init
 * Purpose:
 *     Initialize hash table
 */
int soc_sbx_caladan3_tmu_hw_hash_table_init(int unit)
{
    int status = SOC_E_NONE;
    return status;
}

/*
 *
 * Function:
 *     _tmu_hash_fifo_entry_alloc
 * Purpose:
 *     Allocate a fifo entry from hpcm pool
 */
static int _tmu_hash_hpcm_alloc(int unit, int fifoid, soc_heap_mem_chunk_t **hpcm)
{
    int status = SOC_E_NONE;
   
    if (fifoid < 0 || fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO || !hpcm) {
        return SOC_E_PARAM;
    } 
    
    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    if (TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.max_alloc_lvl > 0) {
    }

    status = hpcm_init(unit, SOC_SBX_TMU_DEF_HPCM_SIZE, sizeof(soc_sbx_tmu_free_fifo_entry_t), hpcm);   
    if (SOC_SUCCESS(status)) {
        DQ_INSERT_HEAD(&TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.free_hpcm_list, (*hpcm));
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);

    return status;
}


/*
 *
 * Function:
 *     _tmu_hash_fifo_entry_free
 * Purpose:
 *     free a fifo entry from hpcm pool
 */
static int _tmu_hash_hpcm_free(int unit, int fifoid, soc_heap_mem_chunk_t *hpcm)
{
    int status = SOC_E_NONE, count=0;
   
    if (fifoid < 0 || fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO || !hpcm) {
        return SOC_E_PARAM;
    } 
    
    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    DQ_LENGTH(&TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.free_hpcm_list, count);
    if (count == TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.max_free_lvl) {
        DQ_REMOVE(hpcm);
        status = hpcm_destroy(unit, hpcm);
    } else {
        DQ_REMOVE(hpcm);
        DQ_INSERT_HEAD(&TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.free_hpcm_list,hpcm);
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);

    return status;
}

/*
 *
 * Function:
 *     _tmu_hash_fifo_entry_alloc
 * Purpose:
 *     Allocate a fifo entry from hpcm pool
 */
static int _tmu_hash_fifo_entry_alloc(int unit, int table_id, int entry_id, soc_sbx_tmu_free_fifo_entry_t **entry)
{
    int status = SOC_E_NONE, fifoid=-1, index;
    soc_heap_mem_elem_t  *hpcm_elem = NULL;
    soc_heap_mem_chunk_t *hpcm = NULL;


    if (!entry) return SOC_E_PARAM;

    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO && fifoid < 0; index++) {
        if (TMU_HASH_FIFO_MGR(unit, index).tbl_fifo_map[table_id] >= 0) {
            fifoid = index;
        }
    }

    if (fifoid < 0) {
        status = SOC_E_INTERNAL;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table ID %d not expected - HW error !!!\n"), 
                   FUNCTION_NAME(), unit, table_id));
    } 

    if (fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO) {
        status = SOC_E_INTERNAL;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table ID %d not mapped to correct FIFO-ID !!!\n"), 
                   FUNCTION_NAME(), unit, table_id));
    } 

    if (SOC_SUCCESS(status)) {
        if (!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.free_hpcm_list)) {
            hpcm = DQ_ELEMENT_GET(soc_heap_mem_chunk_t *,
                                  DQ_HEAD(&TMU_HASH_FIFO_MGR(unit,fifoid).hpcm_mgr.free_hpcm_list, dq_p_t),
                                  list_node);
        } else {
            status = _tmu_hash_hpcm_alloc(unit, fifoid, &hpcm);
        }

        if (SOC_SUCCESS(status)) {
            status = hpcm_alloc(unit, hpcm, &hpcm_elem);
            if (SOC_SUCCESS(status)) {
                /* safe-typecast */
                *entry = (soc_sbx_tmu_free_fifo_entry_t*)hpcm_elem->elem;
                DQ_INSERT_TAIL(&TMU_HASH_FIFO_MGR(unit,fifoid).recycle_entry_list, *entry);
                (*entry)->table_id = table_id;
                (*entry)->entry_id = entry_id;
                (*entry)->handle = hpcm_elem;

                /* move hpcm to full list if fully allocated */
                if (hpcm_empty(unit,hpcm)) {
                    DQ_REMOVE(hpcm);
                    DQ_INSERT_HEAD(&TMU_HASH_FIFO_MGR(unit,index).hpcm_mgr.full_hpcm_list, hpcm);
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed on internal memory allocation %d!!!\n"), 
                           FUNCTION_NAME(), unit, status));
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed on internal memory allocation %d!!!\n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     _tmu_hash_fifo_entry_free
 * Purpose:
 *     Free a fifo entry back to hpcm pool
 */
static int _tmu_hash_fifo_entry_free(int unit, soc_sbx_tmu_free_fifo_entry_t *entry)
{
    int status = SOC_E_NONE, fifoid=-1, index;
    soc_heap_mem_elem_t  *hpcm_elem = NULL;
    soc_heap_mem_chunk_t *hpcm = NULL;

    TMU_HASH_FIFOMGR_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    for (index=0; index < TMU_HASH_MAX_FREE_PAGE_FIFO && fifoid < 0; index++) {
        if (TMU_HASH_FIFO_MGR(unit, index).tbl_fifo_map[entry->table_id] >= 0) {
            fifoid = index;
        }
    }

    if (fifoid < 0) {
        status = SOC_E_INTERNAL;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table ID %d not expected - HW error !!!\n"), 
                   FUNCTION_NAME(), unit, entry->table_id));
    } 

    if (fifoid >= TMU_HASH_MAX_FREE_PAGE_FIFO) {
        status = SOC_E_INTERNAL;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Table ID %d not mapped to correct FIFO-ID !!!\n"), 
                   FUNCTION_NAME(), unit, entry->table_id));
    } 

    hpcm_elem = (soc_heap_mem_elem_t*)entry->handle;
    hpcm =  hpcm_elem->parent;

    if (SOC_SUCCESS(status)) {
        status = hpcm_free(unit,hpcm_elem);
        if (SOC_SUCCESS(status)) {
            if (hpcm_is_unused(unit, hpcm)) {
                status = _tmu_hash_hpcm_free(unit, fifoid, hpcm);
                if (SOC_SUCCESS(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed on free memory %d!!!\n"), 
                               FUNCTION_NAME(), unit, status));
                }
            } 
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed on free memory %d!!!\n"), 
                       FUNCTION_NAME(), unit, status));
        }
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *   Function
 *     soc_sbx_caladan3_tmu_hash_fifo_feed
 *   Purpose
 *      Feeds free fifo from free chain entry or recycle entry
 */
int soc_sbx_caladan3_tmu_hash_fifo_feed(int unit, int fifoid)
{
    int status = SOC_E_NONE, index=0;
    uint32 *dmabuf = NULL, field=0, regval, depth=0;
    int free_page_count=0, recycle_count=0;
    soc_sbx_tmu_chain_alloc_handle_t *chain;
    soc_sbx_tmu_free_fifo_entry_t *recycle_entry;
    dq_p_t elem;
    soc_mem_t tmu_fifo_mem[] = { TMB_UPDATER_FREE_CHAIN_FIFO0m, TMB_UPDATER_FREE_CHAIN_FIFO1m,
                                 TMB_UPDATER_FREE_CHAIN_FIFO2m, TMB_UPDATER_FREE_CHAIN_FIFO3m };
 
#define _FIFO_MAX_FREE_PAGES_(len,depth) ((len)-(depth)-TMU_HASH_DEF_FLOW_CTRL_DISABLE_OFFSET-1)

    TMU_HASH_FIFOMGR_LOCK(unit);

    /* if free page fifo is greater than aempty threshold do not feed it */
    status = READ_TMB_UPDATER_FREE_CHAIN_FIFO_DEPTHr(unit, fifoid, &regval);
    if (SOC_SUCCESS(status)) {
        depth = soc_reg_field_get(unit, TMB_UPDATER_FREE_CHAIN_FIFO_DEPTHr, regval, DEPTHf);
        if (depth > (_tmu_hash_fifo_mgr[unit]->low_lvl_threshold * TMU_HASH_FREE_PAGE_FIFO_SIZE)/100) {
            /* do not feed the fifo */
            status = SOC_E_FULL;
            LOG_INFO(BSL_LS_SOC_SOCMEM,
                     (BSL_META_U(unit,
                                 "%s:%d Skip feeding fifo(%d) depth(%d) \n"),
                      FUNCTION_NAME(), unit, fifoid, depth));
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to get fifo(%d) depth: %d!!!\n"), 
                   FUNCTION_NAME(), unit, fifoid, status));
    }

    if (SOC_SUCCESS(status)) {
        /* identify fifo that has to be service */
        dmabuf = (uint32*) _tmu_hash_fifo_mgr[unit]->dma_buffer;
        free_page_count = 0;
        recycle_count =0; 
        index=0;
        
        /* Least recently used scheme: allocate fresh pages from chains & if chains run out, allocate
         * from recycle fifo head to tail */
        DQ_TRAVERSE(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, elem) {
            chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*, elem, free_chain_list_node);
            if (chain->free_ptr < chain->size) {
                
                for (;index < (((chain->size - chain->free_ptr) >
                                (_FIFO_MAX_FREE_PAGES_(_tmu_hash_fifo_mgr[unit]->dma_buffer_len,depth))) ? 
                               _tmu_hash_fifo_mgr[unit]->dma_buffer_len:chain->size - chain->free_ptr); index++) {
                    field = chain->table_id;
                    soc_mem_field_set(unit, tmu_fifo_mem[fifoid],
                                      dmabuf + index, N_TABLEf, &field);
                    
                    field = index + chain->free_ptr;
                    
                    soc_mem_field_set(unit, tmu_fifo_mem[fifoid],
                                      dmabuf + index, N_ENTRYf, &field);
                }
                
                free_page_count = index;
            }

            LOG_INFO(BSL_LS_SOC_SOCMEM,
                     (BSL_META_U(unit,
                                 "%s:%d feed free fifo(%d) with (%d) entries from chain table (%d) \n"),
                      FUNCTION_NAME(), unit, fifoid, index, chain->table_id));

            /* all entries allocated from free chains */
            if (index == _FIFO_MAX_FREE_PAGES_(_tmu_hash_fifo_mgr[unit]->dma_buffer_len,depth))
                break;

        } DQ_TRAVERSE_END(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, elem);
        
        /* feed from recycle fifo */
        if ((!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, fifoid).recycle_entry_list)) && 
            (index < _FIFO_MAX_FREE_PAGES_(_tmu_hash_fifo_mgr[unit]->dma_buffer_len,depth))) {
            
            DQ_TRAVERSE(&TMU_HASH_FIFO_MGR(unit, fifoid).recycle_entry_list, elem) {
                
                recycle_entry = DQ_ELEMENT_GET(soc_sbx_tmu_free_fifo_entry_t*, elem, list_node);
                
                soc_mem_field_set(unit, tmu_fifo_mem[fifoid],
                                  dmabuf + index, N_TABLEf, (uint32*)&recycle_entry->table_id);
                
                soc_mem_field_set(unit, tmu_fifo_mem[fifoid],
                                  dmabuf + index, N_ENTRYf, (uint32*)&recycle_entry->entry_id); 
                
                recycle_count++;
                if (index++ == _FIFO_MAX_FREE_PAGES_(_tmu_hash_fifo_mgr[unit]->dma_buffer_len,depth))
                    break;

            } DQ_TRAVERSE_END(&TMU_HASH_FIFO_MGR(unit, fifoid).recycle_entry_list, elem);

            LOG_INFO(BSL_LS_SOC_SOCMEM,
                     (BSL_META_U(unit,
                                 "%s:%d feed free fifo(%d) with (%d) entries from recylce fifo (%d) \n"),
                      FUNCTION_NAME(), unit, fifoid, index, recycle_count));
        }
        
        if (index > 0) {
            /*    coverity[negative_returns : FALSE]    */
            status = soc_mem_write_range(unit, tmu_fifo_mem[fifoid], MEM_BLOCK_ANY, 0, index-1, dmabuf);
            if (SOC_SUCCESS(status)) {

                LOG_INFO(BSL_LS_SOC_SOCMEM,
                         (BSL_META_U(unit,
                                     "%s:%d feed free fifo(%d) with (%d) entries "
                                     "recycled:(%d)\n"), FUNCTION_NAME(), unit, fifoid, index-1, recycle_count));

                /* offset free pointer */
                DQ_TRAVERSE(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, elem) {
                    chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*, elem, free_chain_list_node);
                    if (chain->free_ptr < chain->size) {
                        
                        if (free_page_count > chain->size - chain->free_ptr) {
                            chain->free_ptr = chain->size;
                            free_page_count -= (chain->size - chain->free_ptr);
                        } else {
                            chain->free_ptr += free_page_count;
                            break;
                        }
                    }
                } DQ_TRAVERSE_END(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, elem);

                /* free back recycle entries */
                for (index = 0; index < recycle_count; index++) {
                    DQ_REMOVE_HEAD(&TMU_HASH_FIFO_MGR(unit, fifoid).recycle_entry_list, recycle_entry);
                    assert(recycle_entry->handle != NULL);
                    _tmu_hash_fifo_entry_free(unit, recycle_entry);
                }
            } else {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d DMA failed to feed free fifo / recycle: %d!!!\n"), 
                           FUNCTION_NAME(), unit, status));
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d NULL fifo processing !!!\n"), 
                       FUNCTION_NAME(), unit));
        }
    } else {
        if (SOC_E_FULL == status) status = SOC_E_NONE;
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     tmu_hash_free_page_manager_thread
 * Purpose:
 *     Manages free page FIFO
 */
void tmu_hash_fifo_manager_thread(void *arg)
{
    int status = SOC_E_NONE, index=0, fifoid=0;
    uint32 *ring_ptr=NULL, regval=0, num_data_beats, num_entries_processed=0;
    soc_sbx_tmu_free_fifo_entry_t *recycle_entry;
    int unit = PTR_TO_INT(arg);

    num_data_beats = soc_mem_entry_words(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm);

    while (_tmu_hash_fifo_mgr[unit]->thread_running) {
        if (retry_flag) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: hey I am still alive %d\n"),
                     __FUNCTION__, retry_flag)); retry_flag=0;
        }
        
	/* wait on semaphore for timeout - recycle handler timeout */
        status = sal_sem_take(_tmu_hash_fifo_mgr[unit]->fifo_trigger, SOC_SBX_TMU_HASH_RECYCLE_TIMEOUT);

        TMU_HASH_FIFOMGR_LOCK(unit);

        /* Timeout, process recycle fifo */
        if (SOC_FAILURE(status)) {
            uint32 table_id, entry_id, invalidate_words=0;

            status = SOC_E_NONE;

            /* process recycle ring */
            if (!SAL_BOOT_PLISIM) {

                assert(TMU_HASH_RECYCLE_MGR(unit).ring_pointer - TMU_HASH_RECYCLE_MGR(unit).dma_buffer
                       < TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len);

                invalidate_words = TMU_HASH_RECYCLE_MGR(unit).ring_pointer - \
                               TMU_HASH_RECYCLE_MGR(unit).dma_buffer;

                soc_cm_sinval(unit, 
                          (void *)TMU_HASH_RECYCLE_MGR(unit).ring_pointer, 
                              WORDS2BYTES(num_data_beats * invalidate_words));
                              
                /* ring wrap case */
                if (invalidate_words < SOC_SBX_TMU_RECYCLE_CONSUME_WINDOW) {
                    soc_cm_sinval(unit, 
                                  (void *)TMU_HASH_RECYCLE_MGR(unit).dma_buffer,
                                  WORDS2BYTES(num_data_beats * 
                                   (SOC_SBX_TMU_RECYCLE_CONSUME_WINDOW - invalidate_words)));                    
                }
            }
            
            ring_ptr = (uint32*) TMU_HASH_RECYCLE_MGR(unit).ring_pointer;
            num_entries_processed=0;
            
            for (index=0; index < SOC_SBX_TMU_RECYCLE_CONSUME_WINDOW && SOC_SUCCESS(status); index++) {

                if (*ring_ptr == 0) {
                    break;
                }

                /*LOG_CLI((BSL_META_U(unit,
                                      "### Recycle Entry Dump: 0x%x \n"), *ring_ptr));*/
                soc_mem_field_get(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm, ring_ptr, N_TABLEf, &table_id);
                soc_mem_field_get(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm, ring_ptr, N_ENTRYf, &entry_id );

                /* clear the consumed word */
                *ring_ptr = 0;
                num_entries_processed++;

                LOG_INFO(BSL_LS_SOC_SOCMEM,
                         (BSL_META_U(unit,
                                     "%s: unit(%d) Recycling entries from recycle fifo: Tbl(%d) entry(%d) \n"),
                          FUNCTION_NAME(), unit, table_id, entry_id));

                status = _tmu_hash_fifo_entry_alloc(unit, table_id, entry_id, &recycle_entry);
                if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to allocate free fifo memory %d!!!\n"), 
                               FUNCTION_NAME(), unit, status));
                } else {
                    ring_ptr += num_data_beats;

                    if (ring_ptr - TMU_HASH_RECYCLE_MGR(unit).dma_buffer >= TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len) {
                        ring_ptr = (uint32 *) TMU_HASH_RECYCLE_MGR(unit).dma_buffer;
                        /*LOG_CLI((BSL_META_U(unit,
                                              "$$$$ wrapping recycle dma buffer .... \n")));*/
                    } 
                }
            }

            /* Advance ring pointer */
            if (num_entries_processed > 0) {
                TMU_HASH_RECYCLE_MGR(unit).ring_pointer = ring_ptr;
#if 0
                WRITE_CMIC_CMC0_FIFO_CH1_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEMr(unit, num_entries_processed);
#else
                soc_pci_write(unit, 
                              CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEM_OFFSET(0, 
                              TMU_HASH_RECYCLE_MGR(unit).channel), num_entries_processed); 
#endif
            }

        } else { /* got the semaphore this is a fifo low water mark hit interrupt */
            soc_field_t fifo_aempty[] = {FREE_CHAIN_FIFO0_AEMPTYf, FREE_CHAIN_FIFO1_AEMPTYf, 
                                         FREE_CHAIN_FIFO2_AEMPTYf, FREE_CHAIN_FIFO3_AEMPTYf};

            READ_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, &regval);

            if (soc_reg_field_get(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, regval, fifo_aempty[0])) fifoid = 0;
            else if (soc_reg_field_get(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, regval, fifo_aempty[1])) fifoid = 1;
            else if (soc_reg_field_get(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, regval, fifo_aempty[2])) fifoid = 2;
            else if (soc_reg_field_get(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, regval, fifo_aempty[3])) fifoid = 3;
            else assert(0);

            LOG_INFO(BSL_LS_SOC_SOCMEM,
                     (BSL_META_U(unit,
                                 "%s:%d feeding free fifo(%d) FIFO push status(0x%x)\n"),
                      FUNCTION_NAME(), unit, fifoid, regval));

            status = soc_sbx_caladan3_tmu_hash_fifo_feed(unit, fifoid);
            if (SOC_FAILURE(status)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to feed fifo %d!!!\n"), 
                           FUNCTION_NAME(), unit, status));
            }

            /* clear status */
            soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, fifo_aempty[fifoid], 1);
            WRITE_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, regval);

            /* renable tmb interrupt */
            soc_cmicm_intr3_enable(unit, 1<<SOC_SBX_CALADAN3_TMB_INTR_POS);
        }                

        TMU_HASH_FIFOMGR_UNLOCK(unit);
    }

    _tmu_hash_fifo_mgr[unit]->thread_id = (sal_thread_t)-1;

    sal_thread_exit(0);
    return;
}

/*
 *   Function
 *     soc_sbx_caladan3_tmu_hash_fifo_feed_trigger
 *   Purpose
 *      Wakeup TMU fifo manager thread when the free fifo falls below configured
 *      low water mark threshold
 */
int soc_sbx_caladan3_tmu_hash_fifo_feed_trigger(int unit)
{
    int rv = SOC_E_NONE;
    uint32 regval=0;

    /* wake up ring processing thread */
    if (_tmu_hash_fifo_mgr[unit]->thread_running && _tmu_hash_fifo_mgr[unit]->fifo_trigger) {
        LOG_INFO(BSL_LS_SOC_INTR,
                 (BSL_META_U(unit,
                             "%s unit %d: Feeding Fifo\n"),
                  FUNCTION_NAME(), unit));
    sal_sem_give(_tmu_hash_fifo_mgr[unit]->fifo_trigger);
    } else {
        /* unexpected, disable interrupt */
         LOG_INFO(BSL_LS_SOC_INTR,
                  (BSL_META_U(unit,
                              "!!! Error: %s unit %d: unexpected TMU free FIFO interrupt, no FIFO/recycle thread running,"
                              " disabling all FIFO interrupts \n"),
                   FUNCTION_NAME(), unit));
        READ_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, &regval);
        soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO0_AEMPTYf, 1);
        soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO1_AEMPTYf, 1);
        soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO2_AEMPTYf, 1);
        soc_reg_field_set(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, &regval, FREE_CHAIN_FIFO3_AEMPTYf, 1); 
        WRITE_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, regval);   
        rv = SOC_E_FAIL;
    }
    return rv;
}

/****************/
/** FIB verify **/
/****************/
extern unsigned int tmu_dma_rx_debug, tmu_dma_tx_debug;

/* !!!! NOTE:
 * Chain must be XL cleared inorder to be read arbitrarily. If not cleared explicity
 * XL reads can only read applicable kv pairs specified by hardware */
int soc_sbx_tmu_hw_chain_entry_dump(int unit, 
                                    soc_sbx_tmu_hash_handle_t handle,
                                    uint32 chain_idx, uint32 table_id)
{
    int status = SOC_E_NONE, resp_size_words=0, index=0;
    uint32 buffer[64], read_key[15], read_value[5];
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;
    soc_sbx_caladan3_tmu_cmd_t *cmd=NULL;    
    int retry_count = 10;

    if (!handle) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /*tmu_dma_rx_debug=tmu_dma_tx_debug=1;*/

    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_cmd_alloc(unit, &cmd);
        if (SOC_SUCCESS(status)) {
            cmd->opcode = SOC_SBX_TMU_CMD_XL_READ;
            cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_64;
            cmd->cmd.xlread.entry_num = chain_idx;
            cmd->cmd.xlread.kv_pairs = 5;
            cmd->cmd.xlread.table = table_id;
            
            resp_size_words = BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) + \
                BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]); /* no control word */
            
            resp_size_words *= cmd->cmd.xlread.kv_pairs;

	    TMU_LOCK(unit);

            status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
                                                       &buffer[0], resp_size_words);
               
		while((status==SOC_E_FAIL) && retry_count--){
		    sal_usleep(10000);
		    retry_flag=0x333;

		    status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
							   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);

		    if(status){
		        break;
		    }
		    status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
                                                       &buffer[0], resp_size_words);

		    if(!status) {
		        LOG_CLI((BSL_META_U(unit,
                                            "%s: Successed with %d retry, Ignore previous error message\n"),
                                 __FUNCTION__, 10 - retry_count));    
		    }

		}

		if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Invalid response !!!\n"), 
                               FUNCTION_NAME(), unit));
                } else {
                    for (index=0; index < cmd->cmd.xlread.kv_pairs && SOC_SUCCESS(status); index++) {
                        status = tmu_cmd_eml_kv_unpack(unit, 
                                                       &buffer[index *  \
                                                               (BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) +
                                                                BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]))],
                                                       read_key, read_value, hash_cfg->param.key_type);
                        if (SOC_SUCCESS(status)) {
                            LOG_CLI((BSL_META_U(unit,
                                                "Chain-id(%d) Key: %08x %08x Value: %08x %08x %08x %08x \n"),
                                     index, read_key[1], read_key[0],
                                     read_value[0], read_value[1], 
                                     read_value[2], read_value[3]));
                        }
                    }
                }
            }
	    TMU_UNLOCK(unit);
        }
    }

    tmu_dma_rx_debug=tmu_dma_tx_debug=0;

    if (cmd) {
        tmu_cmd_free(unit, cmd);
    }
    TMU_HASH_UNLOCK(unit);
    return status;
}

int soc_sbx_tmu_hw_fib_get(int unit, 
                        soc_sbx_tmu_hash_handle_t handle,
                        uint32 hw_hash_idx,
                        uint32 *key,
                        uint32 *value)
{
    int status = SOC_E_NONE, resp_size_words=0, index=0;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;
    soc_sbx_caladan3_tmu_cmd_t *cmd = NULL;    
    uint32 buffer[64], read_key[15], read_value[5];
    int miss=1;
    uint32 splitter;
    int retry_count = 10;
    if (!handle) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    sal_memset(buffer,0,sizeof(buffer));

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_cmd_alloc(unit, &cmd);
        if (SOC_SUCCESS(status)) {
            cmd->opcode = SOC_SBX_TMU_CMD_XL_READ;
            cmd->cmd.xlread.table = hash_cfg->table_id;
            cmd->cmd.xlread.entry_num = hw_hash_idx;
            cmd->cmd.xlread.kv_pairs = 0;
            
            switch (hash_cfg->param.key_type) {
            case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_64;
                break;
                
            case SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS:
                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_144;
                break;

            case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_176;
                break;
                
            case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_304;
                break;
                
            case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:
                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_424;
                break;
                
            default:
                status = SOC_E_PARAM;
                break;
            }
        }
        
        /* first access root */
        /* coverity[overrun-local] */
        resp_size_words = BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) + \
                          BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]) + \
                          BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE);
                       
        if (SOC_SUCCESS(status)) {

            /*tmu_dma_rx_debug=tmu_dma_tx_debug=1;*/

	  TMU_LOCK(unit);

            status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
            
            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
                                                       &buffer[0], resp_size_words);
             
		while((status==SOC_E_FAIL) && retry_count--){
		    sal_usleep(10000);
		    retry_flag=0x444;
		    status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);

		    if(status){
		        break;
		    }
		    status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
							   &buffer[0], resp_size_words);

		    if(!status) {
		        LOG_CLI((BSL_META_U(unit,
                                            "%s: Successed with %d retry, Ignore previous error message\n"),
                                 __FUNCTION__, 10 - retry_count));    
		    }

		}

		if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Failed to access root: bad response !!!\n"), 
                               FUNCTION_NAME(), unit));
                } else {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "# Root Dump \n")));
                    status = tmu_cmd_eml_kv_unpack(unit, &buffer[BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE)],
                                                   read_key, read_value, hash_cfg->param.key_type);
                    if (SOC_SUCCESS(status)) {
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "Root Key: %08x %08x Value: %08x %08x %08x %08x \n"),
                                     read_key[1], read_key[0],
                                     read_value[0], read_value[1], 
                                     read_value[2], read_value[3]));

                        /* walk through the chain and dump all */
                        /* key doesnt match look for chain */
                        cmd->cmd.xlread.table = buffer[1] >> 27;
                        
                        /* get entry index from command word */
                        cmd->cmd.xlread.entry_num = buffer[1] >> 3;
                        cmd->cmd.xlread.entry_num &= ((1 << 24) - 1);
                        
                        /* NLE */
                        cmd->cmd.xlread.kv_pairs = (buffer[1] & 0x7);
                        cmd->cmd.xlread.kv_pairs <<= 1;
                        cmd->cmd.xlread.kv_pairs |= buffer[0] >> 31;

                        splitter = buffer[0] & ((1<<26)-1);
                        
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "# Chain Table[%d] Entry[%d] NLE[%d] Splitter[%d] \n"),
                                     cmd->cmd.xlread.table, cmd->cmd.xlread.entry_num, 
                                     cmd->cmd.xlread.kv_pairs, splitter));
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "Raw dump: 0x%x 0x%x \n"),
                                     buffer[1], buffer[0]));

                        if (sal_memcmp(key, &read_key[0], BITS2BYTES(_g_tmu_key_size_bits[hash_cfg->param.key_type])) == 0) {
                            /* if Splitter!=0, then entry is valid and update value */
                            if (splitter != 0) {
                                miss = 0;
                                value[0] = read_value[0];
                                value[1] = read_value[1];
                                value[2] = read_value[2];
                                value[3] = read_value[3];
                            }
                        }

                        if (cmd->cmd.xlread.kv_pairs != 15) {
                            switch (hash_cfg->param.key_type) {
                            case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
                                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_64;
                                break;
                                
                            case SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS:
                                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_144;
                                break;

                            case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
                                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_176;
                                break;
                                
                            case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
                                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_304;
                                break;
                                
                            case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:
                                cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_424;
                                break;
                                
                            default:
                                status = SOC_E_PARAM;
                                break;
                            }
                            



                            status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
                            
                            resp_size_words = BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) + \
                                BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]); /* no control word */
                            
                            resp_size_words *= cmd->cmd.xlread.kv_pairs;
                            
                            if (SOC_SUCCESS(status)) {
                                status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
                                                                       &buffer[0], resp_size_words);
                               

				while((status==SOC_E_FAIL) && retry_count--){
				    sal_usleep(10000);
				    retry_flag=0x555;

				    status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
									   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
				    
				    if(status){
				      break;
				    }
				    
				    resp_size_words = BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) + \
				      BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]); /* no control word */
                            
				    resp_size_words *= cmd->cmd.xlread.kv_pairs;

				    status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
									   &buffer[0], resp_size_words);

				    if(!status) {
				      LOG_CLI((BSL_META_U(unit,
                                                          "%s: Successed with %d retry, Ignore previous error message\n"),
                                               __FUNCTION__, 10 - retry_count));    
				    }
		
				}
	    
				
				if (SOC_FAILURE(status)) {
                                    LOG_ERROR(BSL_LS_SOC_COMMON,
                                              (BSL_META_U(unit,
                                                          "%s: unit %d Chain table access failed !!!\n"), 
                                               FUNCTION_NAME(), unit));
                                } else {
                                    for (index=0; index < cmd->cmd.xlread.kv_pairs && SOC_SUCCESS(status); index++) {
                                        status = tmu_cmd_eml_kv_unpack(unit, 
                                                                       &buffer[index * \
                                                                               (BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) +
                                                                                BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]))],
                                                                       read_key, read_value, hash_cfg->param.key_type);
                                        if (SOC_SUCCESS(status)) {
                                            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                                        (BSL_META_U(unit,
                                                                    "Chain-id(%d) Key: %08x %08x Value: %08x %08x %08x %08x \n"),
                                                         index, read_key[1], read_key[0],
                                                         read_value[0], read_value[1], 
                                                         read_value[2], read_value[3]));
                                            if (sal_memcmp(key, &read_key[0], BITS2BYTES(_g_tmu_key_size_bits[hash_cfg->param.key_type])) == 0) {
                                                /* if read_key matches expected key, update value */
                                                miss = 0;
                                                value[0] = read_value[0];
                                                value[1] = read_value[1];
                                                value[2] = read_value[2];
                                                value[3] = read_value[3];
                                            }
                                        }
                                    }
                                } 
                            } 
                        }
                    }
                }
            }
	    
	    TMU_UNLOCK(unit);
        }
    }

    if (miss) {
        sal_memset(key, 0, BITS2BYTES(_g_tmu_key_size_bits[hash_cfg->param.key_type]));
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Splitter=0 or read_key does not match, clear key\n")));
    }

    tmu_dma_rx_debug=tmu_dma_tx_debug=0;

    if (cmd != NULL) {
    tmu_cmd_free(unit, cmd);
    }
    TMU_HASH_UNLOCK(unit);
    return status;
}

int soc_sbx_tmu_fib_get(int unit, 
                        soc_sbx_tmu_hash_handle_t handle,
                        uint32 sw_hash_idx, 
                        uint32 sw_bucket_idx,
                        uint32 *key,
                        uint32 *value)
{
    uint32 hw_hash_idx=0;
    int status = SOC_E_NONE;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;

    if (!handle) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        hw_hash_idx = (sw_bucket_idx << hash_cfg->table->host_max_num_entries_msb_pos) | sw_hash_idx;
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "### %s: Hash(0x%x) Bucket(0x%x) Hw-Hash(0x%x) HW-Root[0x%x] \n"),
                     FUNCTION_NAME(), sw_hash_idx, sw_bucket_idx, hw_hash_idx,
                     hw_hash_idx & ((1<<hash_cfg->table->device_max_num_entries_msb_pos)-1)));
        status = soc_sbx_tmu_hw_fib_get(unit, handle, hw_hash_idx, key, value);
    }

    TMU_HASH_UNLOCK(unit);
    return status;
}


int soc_sbx_caladan3_tmu_hash_entry_verify(int unit, 
                                           soc_sbx_tmu_hash_handle_t handle,
                                           uint32 *key, 
                                           uint32 *value,
                                           uint8  dump /* only dumps skips value verification */)
{
    int status = SOC_E_NONE, resp_size_words=0, index=0;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;
    soc_sbx_caladan3_tmu_cmd_t *cmd = NULL;    
    uint32 hash_idx=0, bucket_idx=0;
    uint32 buffer[64], read_key[15], read_value[5];
    int retry_count = 10;

    /* Hash key 144 bits is converted to 176 bits by the caller */

    if (!handle || !key || !value || 
        (hash_cfg->param.key_type == SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS)) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    sal_memset(buffer,0,sizeof(buffer));

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
        status = tmu_hash_function(unit, hash_cfg->table, key, &hash_idx, &bucket_idx);
        if (SOC_SUCCESS(status)) {
            status = tmu_cmd_alloc(unit, &cmd);
            if (SOC_SUCCESS(status)) {
                cmd->opcode = SOC_SBX_TMU_CMD_XL_READ;
                cmd->cmd.xlread.table = hash_cfg->table_id;
                cmd->cmd.xlread.entry_num = (bucket_idx << hash_cfg->table->host_max_num_entries_msb_pos) | hash_idx;
                cmd->cmd.xlread.kv_pairs = 0;
                
                switch (hash_cfg->param.key_type) {
                case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_64;
                    break;
                    
                case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_176;
                    break;
                    
                case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_304;
                    break;
                    
                case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:
                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML_424;
                    break;
                    
                default:
                    status = SOC_E_PARAM;
                    break;
                }
            }

            if (dump) {
                LOG_CLI((BSL_META_U(unit,
                                    "#Hash Key Dump: ")));
                for (index = BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type])-1; index >= 0; index--) {
                    LOG_CLI((BSL_META_U(unit,
                                        "%08x "), key[index]));
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n Hash-Idx:0x%x Bucket-Id:0x%x \n"),
                         hash_idx, bucket_idx));
            }
        }
        
        /* first access root */
        resp_size_words = BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) + \
                          BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]) + \
                          BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE);
                       
        if (SOC_SUCCESS(status)) {

	  TMU_LOCK(unit);
            status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
            
            if (SOC_SUCCESS(status)) {
                status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
                                                       &buffer[0], resp_size_words);
                
		while((status==SOC_E_FAIL) && retry_count--){
		    sal_usleep(10000);
		    retry_flag=0x666;
		    status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);

		    	
		    if(status){
		      break;
		    }

		    status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
							   &buffer[0], resp_size_words);

		    if(!status) {
		        LOG_CLI((BSL_META_U(unit,
                                            "%s: Successed with %d retry, Ignore previous error message\n"),
                                 __FUNCTION__, 10 - retry_count));    
		    }
		
		}

		if (SOC_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Invalid response !!!\n"), 
                               FUNCTION_NAME(), unit));
                } else {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "RRR %s:%d Root Access response: RRR\n"), 
                                 FUNCTION_NAME(), unit));
                    for (index=0; index < resp_size_words; index++) {
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "0x%08x \t"), buffer[index]));
                    } 
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "\n")));

                    status = tmu_cmd_eml_kv_unpack(unit, &buffer[BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE)],
                                                   read_key, read_value, hash_cfg->param.key_type);
                    if (SOC_SUCCESS(status)) {
                        if (sal_memcmp(key, &read_key[0], BITS2BYTES(_g_tmu_key_size_bits[hash_cfg->param.key_type]))) {
                            /* key doesnt match look for chain */
                            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                        (BSL_META_U(unit,
                                                    "# Looking for chains ...... \n")));
                            cmd->cmd.xlread.table = buffer[1] >> 27;

                            /* get entry index from command word */
                            cmd->cmd.xlread.entry_num = buffer[1] >> 3;
                            cmd->cmd.xlread.entry_num &= ((1 << 24) - 1);

                            /* NLE */
                            cmd->cmd.xlread.kv_pairs = (buffer[1] & 0x7);
                            cmd->cmd.xlread.kv_pairs <<= 1;
                            cmd->cmd.xlread.kv_pairs |= buffer[0] >> 31;

                            switch (hash_cfg->param.key_type) {
                                case SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS:
                                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_64;
                                    break;
                                    
                                case SOC_SBX_CALADAN3_TMU_HASH_KEY_144_BITS:
                                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_144;
                                    break;

                                case SOC_SBX_CALADAN3_TMU_HASH_KEY_176_BITS:
                                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_176;
                                    break;
                                    
                                case SOC_SBX_CALADAN3_TMU_HASH_KEY_304_BITS:
                                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_304;
                                    break;
                                    
                                case SOC_SBX_CALADAN3_TMU_HASH_KEY_424_BITS:
                                    cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_EML2ND_424;
                                    break;
                                    
                                default:
                                    break;
                            }

                            if (dump) {
                                LOG_CLI((BSL_META_U(unit,
                                                    "Root Key: %08x %08x Value: %08x %08x %08x %08x \n"
                                                    "Chain Table(%d) Entry(%d) KV-Pairs(%d)\n"),
                                         read_key[1], read_key[0],
                                         read_value[0], read_value[1], 
                                         read_value[2], read_value[3],
                                         cmd->cmd.xlread.table,
                                         cmd->cmd.xlread.entry_num,
                                         cmd->cmd.xlread.kv_pairs));
                            }

			    TMU_LOCK(unit);

			    retry_count=10;
                            status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);

                            resp_size_words = BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) + \
                                BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]); /* no control word */

                            resp_size_words *= cmd->cmd.xlread.kv_pairs;

                            if (SOC_SUCCESS(status)) {
                                status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
                                                                       &buffer[0], resp_size_words);


				while((status==SOC_E_FAIL) && retry_count--){
				    sal_usleep(10000);
				    retry_flag=0x777;

				    status = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                                                   cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);

				    if(status){
				        break;
				    }
		
				    status = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, cmd,
									   &buffer[0], resp_size_words);
				    
				    if(!status) {
				        LOG_CLI((BSL_META_U(unit,
                                                            "%s: Successed with %d retry, Ignore previous error message\n"),
                                                 __FUNCTION__, 10 - retry_count));    
				    }
		
				} 

				TMU_UNLOCK(unit);
				
				if (SOC_FAILURE(status)) {
                                    LOG_ERROR(BSL_LS_SOC_COMMON,
                                              (BSL_META_U(unit,
                                                          "%s: unit %d Invalid response !!!\n"), 
                                               FUNCTION_NAME(), unit));
                                } else {
                                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                                (BSL_META_U(unit,
                                                            "CCC %s:%d Chain Access response: CCC\n"), FUNCTION_NAME(), unit));
                                    for (index=0; index < resp_size_words; index++) {
                                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                                    (BSL_META_U(unit,
                                                                "0x%08x \t"), buffer[index]));
                                    } 
                                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                                (BSL_META_U(unit,
                                                            "\n")));
                                    
                                    for (index=0; index < cmd->cmd.xlread.kv_pairs && SOC_SUCCESS(status); index++) {
                                        status = tmu_cmd_eml_kv_unpack(unit, 
                                                                       &buffer[index * \
                                                                               (BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS) +
                                                                                BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]))],
                                                                       read_key, read_value, hash_cfg->param.key_type);
                                        if (SOC_SUCCESS(status)) {
                                            if (sal_memcmp(key, &read_key[0], 
                                                           BITS2BYTES(_g_tmu_key_size_bits[hash_cfg->param.key_type]))
                                                           == 0) {
                                                if (dump) {
                                                    LOG_CLI((BSL_META_U(unit,
                                                                        "c- Chain Key match idx:%d \n"), index));  
                                                }
                                                break;
                                            } else {
                                                if (dump) {
                                                    LOG_CLI((BSL_META_U(unit,
                                                                        "-- Mismatch Chain entry: %d \n"), index));
                                                    LOG_CLI((BSL_META_U(unit,
                                                                        "Miss Key: %08x %08x Value: %08x %08x %08x %08x \n"),
                                                             read_key[1], read_key[0],
                                                             read_value[0], read_value[1], 
                                                             read_value[2], read_value[3]));
                                                }
                                            }
                                        }
                                    }
                                    
                                    if (index == cmd->cmd.xlread.kv_pairs) {
                                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                                  (BSL_META_U(unit,
                                                              "!!!%s:%d Key: 0x%x 0x%x "
                                                              "Hash-Idx:0x%x Bucket-Id:0x%x Not found !!!\n"),
                                                   FUNCTION_NAME(), unit, key[1], key[0], 
                                                   hash_idx, bucket_idx));
                                        status = SOC_E_NOT_FOUND;
                                    }
                                }
                            }
                        } else {
                            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                        (BSL_META_U(unit,
                                                    "%s:%d RRR Key match on root RRRR\n"),
                                         FUNCTION_NAME(), unit));
                            if (dump) {
                                LOG_CLI((BSL_META_U(unit,
                                                    "+ Root Key match \n")));
                            }
                        }
                    }
                }
            }
	    TMU_UNLOCK(unit);
        }
    }

    /* dump payload */
    if (SOC_SUCCESS(status) && dump) {
        LOG_CLI((BSL_META_U(unit,
                            "Payload Dump: ")));
        for (index=0; index < BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); index++) {
            LOG_CLI((BSL_META_U(unit,
                                "%08x "), read_value[index]));
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
    }

    if (cmd != NULL) {
        tmu_cmd_free(unit, cmd);
    }
    TMU_HASH_UNLOCK(unit);
    return status;
}

/********************************/
/** unit test support routines **/
/********************************/
/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_table_fifoid_get
 * Purpose:
 *     return fifo id associated to a hash table
 */
int soc_sbx_caladan3_tmu_hash_table_fifoid_get(int unit, 
                                               soc_sbx_tmu_hash_handle_t handle,
                                               int *fifoid)
{
    int rv = SOC_E_NONE;

    if (!handle || !fifoid) return SOC_E_PARAM;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* validate */
    rv = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(rv)) {
        soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;
        *fifoid = hash_cfg->fifoid;
    }

    TMU_HASH_UNLOCK(unit);
    return rv;
}

/*
 *
 * Function:
 *     soc_sbx_tmu_hash_table_fifo_push_empty_get
 * Purpose:
 *     return's fifo state
 */
int soc_sbx_tmu_hash_table_fifo_push_empty_get(int unit, int fifoid) 
{
    int rv = 0;
    uint32 regval=0;
    soc_field_t fifo_aempty[] = {FREE_CHAIN_FIFO0_AEMPTYf, FREE_CHAIN_FIFO1_AEMPTYf, 
                                 FREE_CHAIN_FIFO2_AEMPTYf, FREE_CHAIN_FIFO3_AEMPTYf};

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    SOC_IF_ERROR_RETURN(READ_TMB_UPDATER_FIFO_PUSH_STATUSr(unit, &regval));
    rv = soc_reg_field_get(unit, TMB_UPDATER_FIFO_PUSH_STATUSr, regval, fifo_aempty[fifoid]);

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return rv;
}

int soc_sbx_tmu_hash_ut_recyle_entry_count(int unit, int fifoid, int *num_entries) 
{
    if (!num_entries) return SOC_E_PARAM;
    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);
    DQ_LENGTH(&TMU_HASH_FIFO_MGR(unit, fifoid).recycle_entry_list, *num_entries);
    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_hash_table_alloc
 * Purpose:
 *     Allocate a Hash table
 */
int soc_sbx_tmu_hash_ut_fake_chain_recyle(int unit, int fifoid, int num_entries) 
{
    int index, entry_id;
    uint32 field;
    soc_sbx_tmu_chain_alloc_handle_t *chain;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    assert(!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list));

    chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*,
                           DQ_HEAD(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, dq_p_t),
                           free_chain_list_node);
    /*LOG_CLI((BSL_META_U(unit,
                          "%s\n"), FUNCTION_NAME()));*/
    /* keep it simple dont wrap */
    index = (uint32*)TMU_HASH_RECYCLE_MGR(unit).ring_pointer - (uint32*)TMU_HASH_RECYCLE_MGR(unit).dma_buffer;
    assert(num_entries <= SOC_SBX_TMU_RECYCLE_RESP_DMA_MGR_RING_SIZE - index);

    if (chain->free_ptr > num_entries) {
        entry_id = 0;
    } else {
        assert(chain->free_ptr + num_entries + 1 <= chain->size);
        entry_id = chain->free_ptr;
        chain->free_ptr += num_entries;
    }

    /* exclue 0,0 */
    if (chain->table_id == 0 && entry_id == 0) entry_id++;

    for (index=0; index < num_entries; index++) {
        field = chain->table_id;
        soc_mem_field_set(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm, 
                          (uint32*)TMU_HASH_RECYCLE_MGR(unit).ring_pointer + index, N_TABLEf, &field);
        field = entry_id + index;
        soc_mem_field_set(unit, TMB_UPDATER_RECYCLE_CHAIN_FIFOm,
                          (uint32*)TMU_HASH_RECYCLE_MGR(unit).ring_pointer + index, N_ENTRYf, &field);
        /*LOG_CLI((BSL_META_U(unit,
                              "0x%x \t"), TMU_HASH_RECYCLE_MGR(unit).ring_pointer[index]));*/
    }
    /*LOG_CLI((BSL_META_U(unit,
                          "\n")));*/
    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     soc_sbx_caladan3_hash_table_alloc
 * Purpose:
 *     Allocate a Hash table
 */
int soc_sbx_tmu_hash_ut_fake_chain_free_fifo(int unit, int fifoid, int num_entries, uint8 drain) 
{
    soc_sbx_tmu_chain_alloc_handle_t *chain;

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    assert(!DQ_EMPTY(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list));

    chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*,
                           DQ_HEAD(&TMU_HASH_FIFO_MGR(unit, fifoid).chain_list, dq_p_t),
                           free_chain_list_node);
    if (drain) {
        assert(chain->free_ptr + num_entries >= chain->size);
        chain->free_ptr += num_entries;
    } else {
        chain->free_ptr -= num_entries;
    }

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *   soc_sbx_tmu_hash_ut_reset_recycle_ring
 * Purpose:
 */
int soc_sbx_tmu_hash_ut_reset_recycle_ring(int unit) 
{
    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_FIFOMGR_LOCK(unit);

    TMU_HASH_RECYCLE_MGR(unit).ring_pointer = TMU_HASH_RECYCLE_MGR(unit).dma_buffer;

    sal_memset((uint32*)&TMU_HASH_RECYCLE_MGR(unit).dma_buffer[0], 0, 
               sizeof(uint32) * TMU_HASH_RECYCLE_MGR(unit).dma_buffer_len);

    TMU_HASH_FIFOMGR_UNLOCK(unit);
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_hash_bulk_delete
 * Purpose:
 *     delete all entries matching ((entry.key & key_mask) == key) && ((entry.value & value_mask) == value)
 * Note:
 *     key/key_mask are passed in as array of 6 words (up to 176 bits) with following word order.
 *       key[0] - bits 31-0
 *       key[1] - bits 63-32
 *       key[2] - bits 95-64
 *       key[3] - bits 127-96
 *       key[4] - bits 159-128
 *       key[5] - bits 175-160
 *     Please note that even if the chained hash/bulk delete support keys longer than 176 bits,
 *     the bulk delete interface can only specify a filter on lower 176 bits.
 *     value/value_mask are passed in as array of 4 words (up to 119 bits) with following word order.
 *       value[0] - bits 31-0
 *       value[1] - bits 63-32
 *       value[2] - bits 95-64
 *       value[3] - bits 118-96
 *     set mask bits to 0 will ignore the corresponding key/value bits
 *     driver will force the bulk delete be effective immediately by using the 
 *     filter enable feature of query engine so that all keys matching 
 *     will return miss once bulk delete process started (might take a while to
 *     go through all entries)
 *     
 *     Depending on size of table and how many entries need to be deleted, this operation
 *     might take very long time (not measured for now), timeout is set to 5 seconds assuming
 *     this won't be done by regular API.
 */
int soc_sbx_caladan3_tmu_hash_bulk_delete(int unit, 
                      soc_sbx_tmu_hash_handle_t handle, 
                      uint32 *filter_key,
                      uint32 *filter_key_mask,
                      uint32 *filter_value,
                      uint32 *filter_value_mask)
{
    int status = SOC_E_NONE, rv = SOC_E_NONE, done;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = handle;
    uint32 regval=0;
    soc_timeout_t  timeout;
    uint32 *iter = NULL, *next_iter = NULL;
    uint32 value[BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS)];
   
    if (!handle || !filter_key || !filter_key_mask ||
    !filter_value || !filter_value_mask) {
        return SOC_E_PARAM;
    }

    TMU_HASH_UNIT_INIT_ERROR_CHECK_RETURN(unit);
    TMU_HASH_LOCK(unit);

    /* just ensure the pointer is not a junk */
    status = _soc_sbx_tmu_hash_handle_validate(unit, handle);
    if (SOC_SUCCESS(status)) {
    /* set up key/data/masks */
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_31_0r(unit, filter_key[0]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_63_32r(unit, filter_key[1]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_95_64r(unit, filter_key[2]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_127_96r(unit, filter_key[3]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_159_128r(unit, filter_key[4]));
    regval = (filter_key[5] & 0xFFFF);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_175_160r(unit, regval));

    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_31_0r(unit, filter_key_mask[0]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_63_32r(unit, filter_key_mask[1]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_95_64r(unit, filter_key_mask[2]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_127_96r(unit, filter_key_mask[3]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_159_128r(unit, filter_key_mask[4]));
    regval = (filter_key_mask[5] & 0xFFFF);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_KEY_MASK_175_160r(unit, regval));

    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_31_0r(unit, filter_value[0]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_63_32r(unit, filter_value[1]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_95_64r(unit, filter_value[2]));
    regval = (filter_value[3] & 0x7fFFFF);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_118_96r(unit, regval));

    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_MASK_31_0r(unit, filter_value_mask[0]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_MASK_63_32r(unit, filter_value_mask[1]));
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_MASK_95_64r(unit, filter_value_mask[2]));
    regval = (filter_value_mask[3] & 0x7fFFFF);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_DATA_MASK_118_96r(unit, regval));

    /* start */
    regval = 0;
    soc_reg_field_set(unit, TMB_UPDATER_BULK_DELETE_CONFIGr, 
              &regval, KEY_SIZEf, hash_cfg->param.key_type);
    soc_reg_field_set(unit, TMB_UPDATER_BULK_DELETE_CONFIGr, 
              &regval, ROOT_TABLEf, hash_cfg->table->tmu_hw_root_table_id);
    soc_reg_field_set(unit, TMB_UPDATER_BULK_DELETE_CONFIGr, 
              &regval, FILTER_ENf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_CONFIGr(unit, regval));

    soc_reg_field_set(unit, TMB_UPDATER_BULK_DELETE_CONFIGr, 
              &regval, GOf, 1);
    SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_CONFIGr(unit, regval));    
    
    /* wait for done, assuming this won't be used in regular API,
     *  set timeout to 5 seconds
     */
    done = FALSE;
    soc_timeout_init(&timeout, 5*SECOND_USEC, 0);
    while(!soc_timeout_check(&timeout)) {
        status = READ_TMB_UPDATER_BULK_DELETE_EVENTr(unit, &regval);
        if (SOC_FAILURE(status)) {
        break;
        }
        done = soc_reg_field_get(unit, TMB_UPDATER_BULK_DELETE_EVENTr, regval, DONEf);
        if (done) {
        WRITE_TMB_UPDATER_BULK_DELETE_EVENTr(unit, regval);
        break;
        }
    }
    
    if (done) {
        /* turn off the filter enable since it will impact regular lookup */
        regval = 0;
        SOC_IF_ERROR_RETURN(WRITE_TMB_UPDATER_BULK_DELETE_CONFIGr(unit, regval));

        /* check status and error handling */
        SOC_IF_ERROR_RETURN(READ_TMB_UPDATER_BULK_DELETE_STATUSr(unit, &regval));        
        regval = soc_reg_field_get(unit, TMB_UPDATER_BULK_DELETE_STATUSr, regval, STATUSf);
        switch (regval) {
        case 0:
            /* remove all driver state */

            /* alloc iterators */
            iter = sal_alloc(sizeof(uint32) * BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]), "tmu-hash-key-iter1");
            if (iter == NULL) {
            return SOC_E_MEMORY;
            }
            next_iter = sal_alloc(sizeof(uint32) * BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]), "tmu-hash-key-iter2");
            if (next_iter == NULL) {
            sal_free(iter);
            return SOC_E_MEMORY;
            }

            /* iterate the hash table, delete all matching key/data */
            status = soc_sbx_caladan3_tmu_hash_iterator_first(unit, handle, iter);
            if (SOC_SUCCESS(status)) {
            do {
                status = soc_sbx_caladan3_tmu_hash_iterator_next(unit, handle, iter, next_iter);
                /* check if key/mask match, for 64 bits keys, only check 64 bits */
                if (((iter[0]&filter_key_mask[0]) == filter_key[0]) &&
                ((iter[1]&filter_key_mask[1]) == filter_key[1]) &&
                ((hash_cfg->param.key_type == SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS) ||
                 ((hash_cfg->param.key_type != SOC_SBX_CALADAN3_TMU_HASH_KEY_64_BITS) &&
                  ((iter[2]&filter_key_mask[2]) == filter_key[2]) &&
                  ((iter[3]&filter_key_mask[3]) == filter_key[3]) &&
                  ((iter[4]&filter_key_mask[4]) == filter_key[4]) &&
                  ((iter[5]&filter_key_mask[5]) == filter_key[5])))) {
                    /* get data */
                    rv = soc_sbx_caladan3_tmu_hash_entry_get(unit, handle, iter, value);
                    if (SOC_SUCCESS(rv)) {
                        /* check if data/mask match, delete if so */
                        if (((value[0]&filter_value_mask[0]) == filter_value[0]) &&
                        ((value[1]&filter_value_mask[1]) == filter_value[1]) &&
                        ((value[2]&filter_value_mask[2]) == filter_value[2]) &&
                        ((value[3]&filter_value_mask[3]) == filter_value[3])) {
                            /* value matched, delete the key */
                            rv = tmu_hash_table_entry_delete_ext(unit, hash_cfg->table, iter, hash_cfg->param.key_type, TRUE);
                            if (SOC_FAILURE(rv)) {
                                /* failure here means something wrong internally when deleting the key */
                                break;
                            }
                        }
                    } else {
                        /* failure here means something wrong internally when getting value for the key */
                        break;
                    }
                }
                if (SOC_SUCCESS(status)) {
                /* prepare for next key */
                sal_memcpy(iter, next_iter, sizeof(uint32) * BITS2WORDS(_g_tmu_key_size_bits[hash_cfg->param.key_type]));
                }
            } while (SOC_SUCCESS(status));
            }
            
            if (status == SOC_E_LIMIT) {
                /* done with iteration (or if empty) */
                status = SOC_E_NONE;
            }

            /* free iterators */
            if (iter) {
            sal_free(iter);
            }
            if (next_iter) {
            sal_free(next_iter);
            }
            break;
        case 1:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TMU bulk delete ecc error on root entry !!!\n"), 
                       FUNCTION_NAME(), unit));
            status = SOC_E_FAIL;
            break;
        case 2:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TMU bulk delete ecc error on le chain !!!\n"), 
                       FUNCTION_NAME(), unit));
            status = SOC_E_FAIL;
            break;
        case 3:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TMU bulk delete ecc error on gt chain !!!\n"), 
                       FUNCTION_NAME(), unit));
            status = SOC_E_FAIL;
            break;
        case 4:
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d TMU bulk delete ecc error on internal SRAM buffer !!!\n"), 
                       FUNCTION_NAME(), unit));
            status = SOC_E_FAIL;
            break;
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s:%d bulk deleted timed out\n"),
                   FUNCTION_NAME(), unit));
        status = SOC_E_TIMEOUT;
    }
    }
    
    TMU_HASH_UNLOCK(unit);
    return status;
}

unsigned int
soc_sbx_tmu_hash_fifo_crc(int unit)
{
    soc_sbx_tmu_hash_fifo_mgr_t *fifo_mgr = _tmu_hash_fifo_mgr[unit];
    unsigned int crc = 0;
    int i,j;

    crc ^= (unsigned int)fifo_mgr->thread_running;
    crc ^= (unsigned int)fifo_mgr->thread_priority;
    crc ^= (unsigned int)fifo_mgr->dma_buffer_len;
    crc ^= (unsigned int)fifo_mgr->low_lvl_threshold;
    /* crc for soc_sbx_tmu_free_fifo_info_t fifo_cfg[TMU_HASH_MAX_FREE_PAGE_FIFO]; */
    for (i=0; i<TMU_HASH_MAX_FREE_PAGE_FIFO; i++) {
        crc ^= (unsigned int)fifo_mgr->fifo_cfg[i].key_size_bits;
        crc ^= (unsigned int)fifo_mgr->fifo_cfg[i].chain_length;
        for (j=0; j<SOC_SBX_CALADAN3_TMU_MAX_TABLE; j++) {
            crc ^= (unsigned int)fifo_mgr->fifo_cfg[i].tbl_fifo_map[j];
        }
        crc ^= (unsigned int)fifo_mgr->fifo_cfg[i].refcount;
        /* hpcm_mgr */
        crc ^= (unsigned int)fifo_mgr->fifo_cfg[i].hpcm_mgr.max_free_lvl;
        crc ^= (unsigned int)fifo_mgr->fifo_cfg[i].hpcm_mgr.max_alloc_lvl;
        /* recycle_ring_mgr */
        crc ^= (unsigned int)fifo_mgr->recycle_ring_mgr.dma_buffer_len;
        crc ^= (unsigned int)fifo_mgr->recycle_ring_mgr.channel;
    }
    
    return crc;
}

#endif /* BCM_CALADAN3_SUPPORT */
