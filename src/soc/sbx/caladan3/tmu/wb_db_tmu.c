/*
 * $Id$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <soc/types.h>
#include <shared/bitop.h>
#include <shared/util.h>
#include <shared/bsl.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <soc/sbx/wb_db_cmn.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/hash.h>
#include <soc/sbx/caladan3/tmu/wb_db_tmu.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_int.h>


#ifdef BCM_WARM_BOOT_SUPPORT
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif /* _ERR_MSG_MODULE_NAME */

/* there isn't a soc module specific for g3p1. init is as close as it gets */
#define _ERR_MSG_MODULE_NAME SOC_DBG_INIT

/* 2 = 1 for cmd, 1 for resp */
#define SOC_SBX_DMA_DBASE_SAVE_SIZE (sizeof(int) * SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO * 2)

/* global data structures */
/*------------------------*/
static _soc_sbx_tmu_wb_state_scache_info_t *_soc_sbx_tmu_wb_state_scache_info_p[SOC_MAX_NUM_DEVICES] = { 0 };


/* macros */
/*--------*/
#define SBX_SCACHE_INFO_PTR(unit) (_soc_sbx_tmu_wb_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_soc_sbx_tmu_wb_state_scache_info_p[unit]) != NULL \
                                     && (_soc_sbx_tmu_wb_state_scache_info_p[unit]->init_done == TRUE))


/* static functions */
/*------------------*/


/*
 *  Function
 *     _soc_sbx_tmu_hashtable_wb_dump_state
 *  Description:
 *     CRC the contents of the TMU buffer and display on the console.
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */


unsigned int 
_soc_sbx_tmu_hashtable_wb_dump_state(int unit)
{
    unsigned int crc = 0;
    int index;
    int entry_idx;
    soc_sbx_tmu_hash_dbase_t* tmu_hash_dbase;

    /* iterate through each entry in all hash tables and CRC the contents */
    tmu_hash_dbase = tmu_hash_dbase_get(unit);
    for (index=0; index < TMU_HASH_MAX_TABLE; index++) {
        for (entry_idx = 0; entry_idx < TMU_HASH_MAX_TABLE_ENTRY; entry_idx++) {
            crc ^= tmu_hash_dbase->hashtable[(index*TMU_HASH_MAX_TABLE_ENTRY) + entry_idx]; 
        }
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s unit %d, soc TMU warm boot scache signature 0x%08x.\n"),
                 FUNCTION_NAME(), unit, crc));
    return crc;
}

/*
 *  Enumerate the types of dbase data structures
 */

typedef enum _soc_sbx_tmu_dbase_type_e_s {
    SOC_SBX_TMU_DBASE_DRAM,
    SOC_SBX_TMU_DBASE_REGION,
    SOC_SBX_TMU_DBASE_TABLE,
    SOC_SBX_TMU_DBASE_CMD_DMA,
    SOC_SBX_TMU_DBASE_RESP_DMA,
    SOC_SBX_TMU_DBASE_PROGRAM,
    SOC_SBX_TMU_DBASE_CONTROL,
    SOC_SBX_TMU_DBASE_MAX_TYPES
} soc_sbx_tmu_dbase_type_e_t;


/*
 *  Function
 *     _soc_sbx_tmu_dma_crc
 *  Description:
 *     CRC the contents of the specified TMU dma_mgr.
 *     Handles both cmd and resp data structures (they are the same).
 *     Only includes entries that are static over a warm boot.
 *  Inputs:
 *     unit - device number
 *     tmu_dma_mgr_dbase_t *dma_mgr
 *  Outputs: 
 *     returns: CRC
 */

STATIC unsigned int
_soc_sbx_tmu_dma_crc(int unit, tmu_dma_mgr_dbase_t *dma_mgr)
{
    unsigned int crc = 0;

    crc ^= (unsigned int)dma_mgr->dma_buffer_len;
    crc ^= (unsigned int)dma_mgr->valid;
    crc ^= (unsigned int)dma_mgr->thread_running;
    crc ^= (unsigned int)dma_mgr->thread_priority;
    crc ^= (unsigned int)dma_mgr->seq_num;
    crc ^= (unsigned int)dma_mgr->unit;
    crc ^= (unsigned int)dma_mgr->channel;
    crc ^= (unsigned int)dma_mgr->fifoid;
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "tmu dma: len %d v %d thr_run %d thr_pri %d, seq %d unit %d ch %d fifo %d\n"),
                 dma_mgr->dma_buffer_len,
                 dma_mgr->valid,
                 dma_mgr->thread_running,
                 dma_mgr->thread_priority,
                 dma_mgr->seq_num,
                 dma_mgr->unit,
                 dma_mgr->channel,
                 dma_mgr->fifoid));

    return crc;
}


/*
 *  Function
 *     _soc_sbx_tmu_dbase_wb_dump_state
 *  Description:
 *     CRC the contents of various TMU data structure.
 *     Only includes entries that are static over a warm boot.
 *  Inputs:
 *     unit - device number
 *     soc_sbx_tmu_dbase_type_e_t type - enum to select data structure
 *  Outputs: 
 *     returns: CRC
 */

unsigned int 
_soc_sbx_tmu_dbase_wb_dump_state(int unit, soc_sbx_tmu_dbase_type_e_t type)
{
    unsigned int crc = 0;
    int i;

    switch (type) {

    case SOC_SBX_TMU_DBASE_DRAM:
        crc ^= _shr_crc32(0, (unsigned char*)&_tmu_dbase[unit]->dram_cfg, 
                          sizeof(soc_sbx_caladan3_dram_config_t)); 
        break;
        
    case SOC_SBX_TMU_DBASE_REGION:
        crc ^= _shr_crc32(0, (unsigned char*)&_tmu_dbase[unit]->region_cfg, 
                          sizeof(soc_sbx_caladan3_region_config_t));
        break;
        
    case SOC_SBX_TMU_DBASE_TABLE:
        crc ^= _shr_crc32(0, (unsigned char*)&_tmu_dbase[unit]->table_cfg, 
                          sizeof(soc_sbx_caladan3_table_config_t));
        break;
        
    case SOC_SBX_TMU_DBASE_CMD_DMA:
        /* only crc fields carried over from cold boot */
        for (i=0; i<SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO; i++) {
            crc ^= _soc_sbx_tmu_dma_crc(unit, &_tmu_dbase[unit]->cmd_dma_cfg.cmdmgr[i]);
        }
        break;

    case SOC_SBX_TMU_DBASE_RESP_DMA:
        /* only crc fields carried over from cold boot */
        for (i=0; i<SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO; i++) {
            crc ^= _soc_sbx_tmu_dma_crc(unit, &_tmu_dbase[unit]->resp_dma_cfg.respmgr[i]);
        }
        break;

    case SOC_SBX_TMU_DBASE_PROGRAM:
        /* take care of extra level of indirection */
        crc = _shr_crc32(0, (unsigned char*)_tmu_dbase[unit]->program_cfg.alloc_bmap,
                         sizeof(_tmu_dbase[unit]->program_cfg.alloc_bmap)); 
        crc ^= _shr_crc32(0, (unsigned char*)_tmu_dbase[unit]->program_cfg.info, 
                          sizeof(soc_sbx_caladan3_tmu_program_info_t));
        break;

    case SOC_SBX_TMU_DBASE_CONTROL:
        crc ^= _shr_crc32(0, (unsigned char*)&_tmu_dbase[unit]->control_cfg, 
                          sizeof(soc_sbx_caladan3_tmu_control_t));
        break;

    default:
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d. Invalid dbase type %d (must be < %d)\n"), 
                   unit, type, SOC_SBX_TMU_DBASE_MAX_TYPES));
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s unit %d, soc TMU warm boot dbase[%d] scache signature 0x%08x.\n"),
                 FUNCTION_NAME(), unit, type, crc));
    return crc;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_state_scache_alloc
 *  Description:
 *     alloc scache for soc tmu module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_tmu_wb_state_scache_alloc(int unit)
{
    int rv = SOC_E_NONE;

    _soc_sbx_tmu_wb_state_scache_info_p[unit] = sal_alloc(sizeof(_soc_sbx_tmu_wb_state_scache_info_t), 
                                                          "Scache for TMU Alloc warm boot");

    if (_soc_sbx_tmu_wb_state_scache_info_p[unit] == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed to allocate scache for for soc soc TMU, unit %d \n"),
                   unit));
        rv = SOC_E_MEMORY;
    }
    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_state_scache_free
 *  Description:
 *     free scache for soc tmu module
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC void
_soc_sbx_tmu_wb_state_scache_free(int unit)
{
    sal_free(_soc_sbx_tmu_wb_state_scache_info_p[unit]);
}


/* save/restore global TMU info */
/*------------------------------*/


/*
 *  Function
 *     _soc_sbx_tmu_wb_dbase_save
 *  Description:
 *      save the sequence numbers for the cmd fifos so they can be restored
 *      on warm boot. The C3 will ignore commands if the sequence numbers are
 *      not sequential.
 *  Inputs:
 *     unit - device number
 *  Outputs:
 *     returns: OK
 */

STATIC int
_soc_sbx_tmu_wb_dbase_save(int unit) {
    int rv = SOC_E_NONE;
    int i;
    
    /* save the sequence numbers for the cmd fifos */
    for (i=0; i<SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO; i++) {
        SBX_WB_DB_SYNC_VARIABLE(int, 1, _tmu_dbase[unit]->cmd_dma_cfg.cmdmgr[i].seq_num);
    }
    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_dbase_restore
 *  Description:
 *      restore the sequence numbers for the cmd fifos
 *  Inputs:
 *     unit - device number
 *  Outputs:
 *     returns: OK
 */

STATIC int
_soc_sbx_tmu_wb_dbase_restore(int unit) {
    int rv = SOC_E_NONE;
    int i;

    /* restore the sequence numbers for the cmd fifos */
    for (i=0; i<SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO; i++) {
        SBX_WB_DB_RESTORE_VARIABLE(int, 1, _tmu_dbase[unit]->cmd_dma_cfg.cmdmgr[i].seq_num);
    }
    return rv;
}


/* save/restore DM table */


/*
 *  Function
 *     _soc_sbx_tmu_wb_dm_save
 *  Description:
 *      Nothing needs to be saved for TMU DMA tables
 *  Inputs:
 *     unit - device number
 *  Outputs:
 *     returns: OK
 */

STATIC int
_soc_sbx_tmu_wb_dm_save(int unit, soc_sbx_g3p1_tmu_table_manager_t *tm, int index) {
    int rv = SOC_E_NONE;

    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_dm_save
 *  Description:
 *      Nothing needs to be restored for TMU DMA tables
 *  Inputs:
 *     unit - device number
 *  Outputs:
 *     returns: OK
 */

STATIC int
_soc_sbx_tmu_wb_dm_restore(int unit, soc_sbx_g3p1_tmu_table_manager_t *tm, int index) {
    int rv = SOC_E_NONE;

    return rv;
}

/* save/restore HASH table */


/*
 *  Function
 *     _soc_sbx_tmu_hash_entry_size
 *  Description:
 *      Returns the size of a hash entry for a specified table
 *  Inputs:
 *     unit - device number
 *     hash_table - ptr to a hash table
 *  Outputs:
 *     returns: number of bytes in a table entry
 */

STATIC int
_soc_sbx_tmu_hash_entry_size(tmu_hash_table_t *hash_table) 
{
    /* size of key and value */
    return (BITS2BYTES(hash_table->key_size_bits) + BITS2BYTES(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS));
}


/*
 *  Function
 *     _soc_sbx_tmu_hash_entry_save
 *  Description:
 *      Save the key and value for a hash table entry
 *  Inputs:
 *     unit - device number
 *     entry - entry in the hash table
 *     hash_table - ptr to a hash table
 *  Outputs:
 *     returns: none
 */

STATIC void
_soc_sbx_tmu_hash_entry_save(int unit, tmu_hash_entry_t* entry, tmu_hash_table_t *hash_table) 
{
    int i;

    /* save key and value */
    for (i=0; i<BITS2WORDS(hash_table->key_size_bits); i++)
        SBX_WB_DB_SYNC_VARIABLE(uint32, 1, entry->key[i]);

    for (i=0; i<BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); i++)
        SBX_WB_DB_SYNC_VARIABLE(uint32, 1, entry->value[i]);
}


/*
 *  Function
 *     _soc_sbx_tmu_hash_entry_restore
 *  Description:
 *      Restore the key and value for the next hash table entry saved
 *      during the previous sync.
 *  Inputs:
 *     unit - device number
 *     hash_handle - ptr to a hash table
 *  Outputs:
 *     returns: status
 */

STATIC int
_soc_sbx_tmu_hash_entry_restore(int unit, soc_sbx_tmu_hash_handle_t hash_handle) 
{
    int i;
    int rv = SOC_E_NONE;
    uint32 key[BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_KEY_MAX_BITS)+1];
    uint32 value[BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS)+1];
    soc_sbx_tmu_hash_cfg_t *hash_cfg = (soc_sbx_tmu_hash_cfg_t*)hash_handle;
    tmu_hash_table_t *hash_table = hash_cfg->table;

    for (i=0; i<BITS2WORDS(hash_table->key_size_bits); i++)
        SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, key[i]);

    for (i=0; i<BITS2WORDS(SOC_SBX_CALADAN3_TMU_HASH_VALUE_SIZE_BITS); i++)
        SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, value[i]);

    rv = soc_sbx_caladan3_tmu_hash_entry_add(unit, hash_handle, key, value);
    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_hash_save
 *  Description:
 *      Save a hash table to scache. First create a table header and save that
 *      Then walk through the hash table and save each valid entry.
 *  Inputs:
 *     unit - device number
 *     tm - pointer to the table mngr data structure
 *     index - which table to save
 *  Outputs:
 *     returns: status
 */

STATIC int
_soc_sbx_tmu_wb_hash_save(int unit, soc_sbx_g3p1_tmu_table_manager_t *tm, int index) {
    int rv = SOC_E_NONE;
    int i;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = (soc_sbx_tmu_hash_cfg_t*)tm->tables[index].handle;
    tmu_hash_table_t *hash_table;
    tmu_hash_table_node_t *tbl_node;
    dq_p_t elem, elem1;
    tmu_hash_entry_t *bucket, *bucket1;
    int max_entries;
    int cnt = 0;
    int bytes_saved = 0;
    uint8 *scache_ptr_num_entries;

    if (hash_cfg == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d. NULL handle for hash table %d\n"), unit, index));
        return SOC_E_INIT;
    }

    hash_table = hash_cfg->table;
    max_entries = hash_cfg->param.capacity;

    /* table header - save table index, type and size & max entries */

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "hash save --- start scache_ptr %p\n"), SBX_SCACHE_INFO_PTR(unit)->scache_ptr));
    SBX_WB_DB_SYNC_VARIABLE(int, 1, index);
    bytes_saved += sizeof(int);
    SBX_WB_DB_SYNC_VARIABLE(int, 1, SOC_SBX_G3P1_TMU_HASH_TABLE_TYPE);
    bytes_saved += sizeof(int);
    SBX_WB_DB_SYNC_VARIABLE(int, 1, max_entries);
    bytes_saved += sizeof(int);
    /* save ptr to # entries - fill in later when the number is known */
    scache_ptr_num_entries = (uint8*)SBX_SCACHE_INFO_PTR(unit)->scache_ptr;
    SBX_WB_DB_SYNC_VARIABLE(int, 1, 0);
    bytes_saved += sizeof(int);

    /* walk the hash table and save each entry found */

    for (i=0; i<(hash_table->host_table_size/TMU_HASH_DEFAULT_LOADING_FACTOR); i++) {
        if (hash_table->table[i] != 0) {
            tbl_node = hash_table->table[i];

            DQ_TRAVERSE(&tbl_node->bucket_list, elem) {
                bucket = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem, list_node); 
                if (bucket == NULL) {
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "sw_hash_get(%d): NULL\n"), i ));
                    break;
                } else {
                    cnt++;
                    _soc_sbx_tmu_hash_entry_save(unit, bucket, hash_table);
                }
                DQ_TRAVERSE(&bucket->chain_list, elem1) {
                    bucket1 = DQ_ELEMENT_GET(tmu_hash_entry_t*, elem1, list_node);
                    if (bucket1 == NULL) {
                        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                    (BSL_META_U(unit,
                                                "sw_hash_get(%d): NULL BUCKET key 0x%x 0x%08x, value 0x%08x, 0x%08x, 0x%08x, 0x%08x\n"),
                                     i, bucket->key[0], bucket->key[1], bucket->value[0], 
                                     bucket->value[1], bucket->value[2], bucket->value[3]));
                        break;
                    } else {
                        cnt++;
                        _soc_sbx_tmu_hash_entry_save(unit, bucket1, hash_table);
                    }
                } DQ_TRAVERSE_END(&bucket->chain_list, elem1);
            } DQ_TRAVERSE_END(&tbl_node->buck_list, elem);
        }
    }
    bytes_saved += cnt * _soc_sbx_tmu_hash_entry_size(hash_table);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "hash save --- count %d saved %d"), cnt, bytes_saved));

    /* save # entries in table header */
    *(int*)scache_ptr_num_entries = cnt;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "hash save --- end   scache_ptr %p\n"), SBX_SCACHE_INFO_PTR(unit)->scache_ptr));
    return rv;
}


/*
 *  Function_soc_sbx_tmu_wb_fifo_restore
 *     
 *  Description:
 *      Save the fifo free list for each table
 *  Inputs:
 *     unit - device number
 *  Outputs:
 *     returns: status
 */


STATIC int _soc_sbx_tmu_wb_fifo_save(int unit) {
    int rv = SOC_E_NONE;
    int i;
    dq_p_t elem;
    soc_sbx_tmu_chain_alloc_handle_t *chain;
    int bytes_saved = 0;
    int cnt = 0;
    uint8 *scache_ptr_num_entries;
    soc_sbx_tmu_hash_fifo_mgr_t *hash_fifo_mgr = tmu_hash_fifo_mgr_get(unit);

    /* save ptr to # entries - fill in later when the number is known */
    scache_ptr_num_entries = (uint8*)SBX_SCACHE_INFO_PTR(unit)->scache_ptr;
    SBX_WB_DB_SYNC_VARIABLE(int, 1, 0);
    bytes_saved += sizeof(int);

    for (i=0; i<TMU_HASH_MAX_FREE_PAGE_FIFO; i++) {
        DQ_TRAVERSE(&hash_fifo_mgr->fifo_cfg[i].chain_list, elem) {
            chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*, elem, free_chain_list_node);

            SBX_WB_DB_SYNC_VARIABLE(int, 1, chain->free_ptr);
            bytes_saved += sizeof(int);
            cnt++;
        } DQ_TRAVERSE_END(&hash_fifo_mgr->fifo_cfg[i].chain_list, elem);
    }
    /* save # entries in table header */
    *(int*)scache_ptr_num_entries = cnt;
    return rv;
}


/*
 *  Function_soc_sbx_tmu_wb_fifo_restore
 *     
 *  Description:
 *      Restore the fifo free list for each table
 *  Inputs:
 *     unit - device number
 *  Outputs:
 *     returns: status
 */

STATIC int _soc_sbx_tmu_wb_fifo_restore(int unit) {
    int rv = SOC_E_NONE;
    int i;
    dq_p_t elem;
    soc_sbx_tmu_chain_alloc_handle_t *chain;
    int cnt = 0;
    soc_sbx_tmu_hash_fifo_mgr_t *hash_fifo_mgr = tmu_hash_fifo_mgr_get(unit);

    /* restore the fifo table count */
    SBX_WB_DB_RESTORE_VARIABLE(int, 1, cnt);


    for (i=0; i<TMU_HASH_MAX_FREE_PAGE_FIFO; i++) {
        DQ_TRAVERSE(&hash_fifo_mgr->fifo_cfg[i].chain_list, elem) {
            chain = DQ_ELEMENT_GET(soc_sbx_tmu_chain_alloc_handle_t*, elem, free_chain_list_node);

            SBX_WB_DB_RESTORE_VARIABLE(int, 1, chain->free_ptr);
            cnt++;
        } DQ_TRAVERSE_END(&hash_fifo_mgr->fifo_cfg[i].chain_list, elem);
    }
    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_hash_restore
 *  Description:
 *      Restore a hash table from scache. First read the table header
 *      Then for each entry saved, add it back into the hash table.
 *  Inputs:
 *     unit - device number
 *     tm - pointer to the table mngr data structure
 *     index - which table to save
 *  Outputs:
 *     returns: status
 */

STATIC int
_soc_sbx_tmu_wb_hash_restore(int unit, soc_sbx_g3p1_tmu_table_manager_t *tm, int index) {
    int rv = SOC_E_NONE;
    int i;
    soc_sbx_tmu_hash_cfg_t *hash_cfg = (soc_sbx_tmu_hash_cfg_t*)tm->tables[index].handle;
    int max_entries;
    int cnt = 0;
    int bytes_restored = 0;
    soc_sbx_g3p1_tmu_table_type_t table_type;

    /* table header - restore table index, type and size & max entries */

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "hash restore --- start scache_ptr %p\n"), SBX_SCACHE_INFO_PTR(unit)->scache_ptr));

    SBX_WB_DB_RESTORE_VARIABLE(int, 1, index);
    bytes_restored += sizeof(int);
    SBX_WB_DB_RESTORE_VARIABLE(int, 1, table_type);
    bytes_restored += sizeof(int);
    SBX_WB_DB_RESTORE_VARIABLE(int, 1, max_entries);
    bytes_restored += sizeof(int);
    SBX_WB_DB_RESTORE_VARIABLE(int, 1, cnt);
    bytes_restored += sizeof(int);

    if (hash_cfg == 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d. NULL handle for hash table %d\n"), unit, index));
        return SOC_E_INIT;
    }

    if (cnt > max_entries) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d. Restore entry num %d > max entries %d for hash table %d\n"), 
                   unit, cnt, max_entries, index));
        return SOC_E_INIT;
    }

    if (table_type != SOC_SBX_G3P1_TMU_HASH_TABLE_TYPE) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d. Expected hash table (%d), read type %d\n"), 
                   unit, SOC_SBX_G3P1_TMU_HASH_TABLE_TYPE, table_type));
        return SOC_E_INIT;
    }

    if (max_entries != hash_cfg->param.capacity) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d. Restore max entries %d != expected %d for hash table %d\n"), 
                   unit, max_entries, hash_cfg->param.capacity, index));
        return SOC_E_INIT;
    }

    /* restore each entry into hash table*/

    for (i=0; i<cnt; i++) {
        rv = _soc_sbx_tmu_hash_entry_restore(unit, tm->tables[index].handle);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "TMU warm boot unit %d, error restoring HASH table %d, entry %d\n"), 
                       unit, index, i));
            break;
        }
    }
    bytes_restored += cnt*_soc_sbx_tmu_hash_entry_size(hash_cfg->table);
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "hash restore --- count %d bytes %d\n"), cnt, bytes_restored));

    /* inc scache ptr to end of allocated entries (max table size)
     * Is this needed?
     * we always allocate the correct size tables, but may as well
     * compress data into the start and not leave gaps. It doesn't
     * really matter either way.
     */
    /*
    SBX_SCACHE_INFO_PTR(unit)->scache_ptr += (max_entries-cnt) * _soc_sbx_tmu_hash_entry_size(hash_cfg->table);
    */

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "hash restore --- end   scache_ptr %p\n"), 
                            SBX_SCACHE_INFO_PTR(unit)->scache_ptr));
    return rv;
}


/* save/restore TAPS table */

/*
 *  Function
 *     _soc_sbx_tmu_wb_taps_save
 *  Description:
 *      TAPS functionality not implemented at this time.
 *  Inputs:
 *     unit - device number
 *     tm - pointer to the table mngr data structure
 *     index - which table to save
 *  Outputs:
 *     returns: OK
 */

STATIC int
_soc_sbx_tmu_wb_taps_save(int unit, soc_sbx_g3p1_tmu_table_manager_t *tm, int index) {
    int rv = SOC_E_NONE;

    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_taps_restore
 *  Description:
 *      TAPS functionality not implemented at this time.
 *  Inputs:
 *     unit - device number
 *     tm - pointer to the table mngr data structure
 *     index - which table to save
 *  Outputs:
 *     returns: OK
 */

STATIC int
_soc_sbx_tmu_wb_taps_restore(int unit, soc_sbx_g3p1_tmu_table_manager_t *tm, int index) {
    int rv = SOC_E_NONE;

    return rv;
}

/*
 *  Function
 *       _soc_sbx_tmu_wb_alloc_layout_init
 *  Description:
 *       sets up the scache structure for soc tmu module
 *  Inputs:
 *     unit - device number
 *     version - version id
 *  Outputs: 
 *     scache_len - length of scache in bytes
 */

STATIC int
_soc_sbx_tmu_wb_alloc_layout_init(int unit, int version, 
                                  unsigned int *scache_len)
{
    int rv = SOC_E_NONE;
    _soc_sbx_tmu_wb_state_scache_info_t *wb_info_ptr = NULL;
    int i;
    soc_sbx_g3p1_state_t *fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;    
    soc_sbx_g3p1_tmu_table_manager_t *tm;
    soc_sbx_tmu_hash_cfg_t *hash_cfg;

    if (fe == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot TMU soc sbx drv not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    tm = fe->tmu_mgr;
    if (tm == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot TMU table manager not initialized for unit %d \n"), unit));
          return SOC_E_INIT;
    }

    wb_info_ptr = _soc_sbx_tmu_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Warm boot not initialized for unit %d \n"), unit));
            return SOC_E_INIT;
    }
    switch(version) {
        case SOC_CALADAN3_TMU_WB_VERSION_1_0:
            wb_info_ptr->version = version;
            /* the maximum size is:
             *   size of hashtable (8k)
             *   size of all entries in each hash table
             */

            /* TMU global data */
            *scache_len = SOC_SBX_DMA_DBASE_SAVE_SIZE;

            /* hashtable size */
            *scache_len += SOC_SBX_HASHTABLE_SIZE;

            /* calc max size for each TMU table */
            for (i=0; i<SOC_SBX_G3P1_TMU_MAX_TABLE_ID; i++) {
                switch (tm->tables[i].type) {

                case SOC_SBX_G3P1_TMU_DM_TABLE_TYPE:
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "DM   table %s\n"), tm->entries[i].name));
                    break;

                case SOC_SBX_G3P1_TMU_HASH_TABLE_TYPE:
                    hash_cfg = (soc_sbx_tmu_hash_cfg_t*)tm->tables[i].handle;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "HASH table %s, size %d\n"), 
                                            tm->entries[i].name, hash_cfg->param.capacity));
                    *scache_len += hash_cfg->param.capacity * _soc_sbx_tmu_hash_entry_size(hash_cfg->table);
                    break;
  
                case SOC_SBX_G3P1_TMU_TAPS_TABLE_TYPE:
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "TAPS table %s\n"), tm->entries[i].name));
                    break;

                default:
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "TMU warm boot unit %d, unknown table type %d\n"), 
                               unit, tm->tables[i].type));
                    return SOC_E_INIT;
                }
            }
            wb_info_ptr->scache_len = *scache_len;
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "==> TMU size %d\n"), *scache_len));
            break;
            
    default:
        rv = SOC_E_INTERNAL;
        SOC_IF_ERROR_RETURN(rv);
        /* coverity [dead_error_line] */
        break;
    }
    return rv;
}


/*
 *  Function
 *     _soc_sbx_tmu_wb_alloc_restore
 *  Description:
 *     Restore the soc tmu data from scache
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

STATIC int
_soc_sbx_tmu_wb_alloc_restore(int unit)
{
    int rv = SOC_E_NONE;
    int i;
    _soc_sbx_tmu_wb_state_scache_info_t *wb_info_ptr = NULL;
    int index;
    int entry_idx;
    soc_sbx_tmu_hash_dbase_t* tmu_hash_dbase;
    soc_sbx_g3p1_state_t *fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;    
    soc_sbx_g3p1_tmu_table_manager_t *tm;
    soc_sbx_tmu_hash_cfg_t *hash_cfg;

    if (fe == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot TMU soc sbx drv not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    tm = fe->tmu_mgr;
    if (tm == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot TMU table manager not initialized for unit %d \n"), unit));
          return SOC_E_INIT;
    }

    wb_info_ptr = _soc_sbx_tmu_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL)
        {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Warm boot not initialized for unit %d \n"), unit));

            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit, "Warm boot not initialized for unit %d \n"), unit));
            return SOC_E_INIT;
        }

    switch(wb_info_ptr->version)
        {
        case SOC_CALADAN3_TMU_WB_VERSION_1_0:

            /* first restore the TMU global data */
            rv = _soc_sbx_tmu_wb_dbase_restore(unit);

            /* restore the hash FIFO free ptrs */
            rv = _soc_sbx_tmu_wb_fifo_restore(unit);

            /* next restore the hashtable */
            tmu_hash_dbase = tmu_hash_dbase_get(unit);
            for (index=0; index < TMU_HASH_MAX_TABLE; index++) {
                for (entry_idx = 0; entry_idx < TMU_HASH_MAX_TABLE_ENTRY; entry_idx++) {

                    SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, 
                                               tmu_hash_dbase->hashtable[(index*TMU_HASH_MAX_TABLE_ENTRY)
                                                                         + entry_idx]);
                }
            }
                         
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "Restored soc TMU from warm boot scache. "
                                    "unit %d loaded %d bytes\n"), 
                         unit, SOC_SBX_HASHTABLE_SIZE));

            /* restore max entries for each hash table */

            /* add max entries for each hash table */
            for (i=0; i<SOC_SBX_G3P1_TMU_MAX_TABLE_ID; i++) {
                switch (tm->tables[i].type) {

                case SOC_SBX_G3P1_TMU_DM_TABLE_TYPE:
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "restoring DM   table %s\n"), tm->entries[i].name));
                    rv = _soc_sbx_tmu_wb_dm_restore(unit, tm, i);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                          "TMU warm boot unit %d, error restoring %s DM table\n"), 
                                   unit, tm->entries[i].name));
                    }
                    break;

                case SOC_SBX_G3P1_TMU_HASH_TABLE_TYPE:
                    hash_cfg = (soc_sbx_tmu_hash_cfg_t*)tm->tables[i].handle;
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "restoring HASH table %s, size %d\n"), 
                                            tm->entries[i].name, hash_cfg->param.capacity));
                    rv = _soc_sbx_tmu_wb_hash_restore(unit, tm, i);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                          "TMU warm boot unit %d, error restoring %s HASH table\n"), 
                                   unit, tm->entries[i].name));
                    }
                    break;

                case SOC_SBX_G3P1_TMU_TAPS_TABLE_TYPE:
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "restoring TAPS table %s\n"), tm->entries[i].name));
                    rv = _soc_sbx_tmu_wb_taps_restore(unit, tm, i);
                    if (SOC_FAILURE(rv)) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                          "TMU warm boot unit %d, error restoring %s TAPS table\n"), 
                                   unit, tm->entries[i].name));
                    }
                    break;

                default:
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "TMU warm bootrestore, unit %d, unknown table type %d\n"), 
                               unit, tm->tables[i].type));
                    return SOC_E_INIT;
                }
            }
            _soc_sbx_tmu_hashtable_wb_dump_state(unit);
            break;

        default:
            rv = SOC_E_INTERNAL;
            SOC_IF_ERROR_RETURN(rv);
            /* coverity [dead_error_line] */
            break;
        }
    return rv;
}


/* Externally accessible functions */
/*---------------------------------*/

/* compress and store soc tmu info in warm boot scache */

extern int
soc_sbx_tmu_wb_state_sync(int unit)
{
    int rv = SOC_E_NONE;
    int i;
    uint8 *scache_ptr_orig = NULL;
    int index;
    int entry_idx;
    soc_sbx_tmu_hash_dbase_t* tmu_hash_dbase;
    soc_sbx_g3p1_state_t *fe = (soc_sbx_g3p1_state_t *) SOC_SBX_CONTROL(unit)->drv;    
    soc_sbx_g3p1_tmu_table_manager_t *tm;
    soc_sbx_tmu_hash_cfg_t *hash_cfg;

    _soc_sbx_tmu_wb_state_scache_info_t *wb_info_ptr = NULL;


    if (fe == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot TMU soc sbx drv not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    tm = fe->tmu_mgr;
    if (tm == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot TMU table manager not initialized for unit %d \n"), unit));
          return SOC_E_INIT;
    }

    if((_soc_sbx_tmu_wb_state_scache_info_p[unit] == NULL)
       || (_soc_sbx_tmu_wb_state_scache_info_p[unit]->init_done != TRUE))
        {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit, "soc TMU Warm boot scache not initialized for unit %d \n"), unit));
            return SOC_E_INIT;
        }

    wb_info_ptr = _soc_sbx_tmu_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL)
        {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit, "Warm boot not initialized for unit %d \n"), unit));
            return SOC_E_INIT;
        }

    /* Save the original scache ptr, for allowing multiple syncs */
    scache_ptr_orig = wb_info_ptr->scache_ptr;

    switch(wb_info_ptr->version) {
    case SOC_CALADAN3_TMU_WB_VERSION_1_0:
        
        /* first save the TMU global data */
        rv = _soc_sbx_tmu_wb_dbase_save(unit);
        
        /* save the TMU FIFO free ptrs */
        rv = _soc_sbx_tmu_wb_fifo_save(unit);
        
        /* next save the hashtable */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Memcpy from 0x%p to 0x%p len %d.\n"), 
                                SBX_SCACHE_INFO_PTR(unit)->scache_ptr, 
                     tmu_hash_dbase_get(unit), SOC_SBX_HASHTABLE_SIZE));
        
        
        tmu_hash_dbase = tmu_hash_dbase_get(unit);
        for (index=0; index < TMU_HASH_MAX_TABLE; index++) {
            for (entry_idx = 0; entry_idx < TMU_HASH_MAX_TABLE_ENTRY; entry_idx++) {
                
                SBX_WB_DB_SYNC_VARIABLE(uint32, 1, 
                                        tmu_hash_dbase->hashtable[(index*TMU_HASH_MAX_TABLE_ENTRY) 
                                                                  + entry_idx]);
            }
        }
        
        
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "Saved soc TMU to warm boot scache. "
                                "unit %d saved %d bytes\n"), 
                     unit, SOC_SBX_HASHTABLE_SIZE));
        
        /* add max entries for each hash table */
        for (i=0; i<SOC_SBX_G3P1_TMU_MAX_TABLE_ID; i++) {
            switch (tm->tables[i].type) {
                
            case SOC_SBX_G3P1_TMU_DM_TABLE_TYPE:
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "saving DM   table %s\n"), tm->entries[i].name));
                rv = _soc_sbx_tmu_wb_dm_save(unit, tm, i);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "TMU warm boot unit %d, error saving %s DM table\n"), 
                               unit, tm->entries[i].name));
                }
                break;
                
            case SOC_SBX_G3P1_TMU_HASH_TABLE_TYPE:
                hash_cfg = (soc_sbx_tmu_hash_cfg_t*)tm->tables[i].handle;
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "saving HASH table %s, size %d\n"), 
                                        tm->entries[i].name, hash_cfg->param.capacity));
                rv = _soc_sbx_tmu_wb_hash_save(unit, tm, i);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "TMU warm boot unit %d, error saving %s HASH table\n"), 
                               unit, tm->entries[i].name));
                }
                break;
                
            case SOC_SBX_G3P1_TMU_TAPS_TABLE_TYPE:
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META_U(unit,
                                        "saving TAPS table %s\n"), tm->entries[i].name));
                rv = _soc_sbx_tmu_wb_taps_save(unit, tm, i);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "TMU warm boot unit %d, error saving %s TAPS table\n"), 
                               unit, tm->entries[i].name));
                }
                break;
                
            default:
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "TMU warm boot unit %d, unknown table type %d\n"), 
                           unit, tm->tables[i].type));
                return SOC_E_INIT;
            }
        }

        /* Restore the scache ptr to original */
        wb_info_ptr->scache_ptr = scache_ptr_orig;
        
        _soc_sbx_tmu_hashtable_wb_dump_state(unit);
        break;
        
        
    default:
        rv = SOC_E_INTERNAL;
        SOC_IF_ERROR_RETURN(rv);
        /* coverity [dead_error_line] */
        break;
    }
    return rv;
}


/* called from tmu.c when executing an init during warm boot. */

extern int
soc_sbx_tmu_wb_state_init(int unit)
{
    int rv = SOC_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    soc_scache_handle_t handle = 0;
    unsigned int scache_len = 0;
    uint8 *scache_ptr = NULL;
    uint16 version = SOC_CALADAN3_TMU_WB_VERSION_CURR;
    uint16 recovered_version = 0;
    int exists = 0;
    _soc_sbx_tmu_wb_state_scache_info_t *wb_info_ptr = NULL;

    if (SBX_SCACHE_INFO_PTR(unit)) {
        _soc_sbx_tmu_wb_state_scache_free(unit);
    }

    SOC_IF_ERROR_RETURN(_soc_sbx_tmu_wb_state_scache_alloc(unit));
    wb_info_ptr = _soc_sbx_tmu_wb_state_scache_info_p[unit];
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit, "Warm boot not initialized for unit %d \n"), unit));
        return SOC_E_INIT;
    }

    SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_TMU, 0);

    if (SOC_WARM_BOOT(unit)) {
        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRetrieve, 
                                         flags, &scache_len, 
                                         &scache_ptr, version, 
                                         &recovered_version, &exists);
        if (rv == SOC_E_NONE) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "unit %d loading TMU Alloc backing store state\n"),
                         unit));

            wb_info_ptr->scache_ptr = scache_ptr;
            SOC_IF_ERROR_RETURN(_soc_sbx_tmu_wb_alloc_layout_init(unit, 
                                                                  version, 
                                                                  &scache_len));
            if(scache_len != wb_info_ptr->scache_len) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit, "Scache length %d is not same as stored "
                                      "length %d\n"),
                           scache_len, wb_info_ptr->scache_len));
            }
            
            SOC_IF_ERROR_RETURN(_soc_sbx_tmu_wb_alloc_restore(unit));
            if (rv == SOC_E_NONE) {
                if (version != recovered_version) {
                    /* set up layout for the preferred version */
                    SOC_IF_ERROR_RETURN(_soc_sbx_tmu_wb_alloc_layout_init(unit,
                                                                          version,
                                                                          &scache_len));
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META_U(unit,
                                            "unit %d reallocate %d bytes warm" 
                                            " boot backing store space\n"), 
                                 unit, scache_len));

                    /* reallocate the warm boot space */
                    rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheRealloc,
                                                     flags, &scache_len, &scache_ptr,
                                                     version, &recovered_version,
                                                     &exists);
                    if (rv != SOC_E_NONE) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit, "unable to reallocate %d bytes"
                                              " warm boot space for unit %d" 
                                              " soc TMU instance: %d (%s)\n"), 
                                   scache_len, unit, rv, _SHR_ERRMSG(rv)));
                        return rv;
                    }
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == rv) */

            wb_info_ptr->scache_ptr = scache_ptr;
            _soc_sbx_tmu_wb_state_scache_info_p[unit]->init_done = TRUE;
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit, "unable to get current warm boot state for" 
                                  " unit %d soc TMU instance: %d (%s)\n"), 
                       unit, rv, _SHR_ERRMSG(rv)));
            return rv;
        }


    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        SOC_IF_ERROR_RETURN(_soc_sbx_tmu_wb_alloc_layout_init(unit, version, 
                                                              &scache_len));
        
        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing" 
                                " store space\n"), unit, scache_len));

        rv = soc_caladan3_scache_ptr_get(unit, handle, socScacheCreate, 
                                         flags, &scache_len, &scache_ptr, 
                                         version, &recovered_version, &exists);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,"unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"), 
                       scache_len, unit, rv, _SHR_ERRMSG(rv)));
            return rv;
        } else {
            wb_info_ptr->scache_ptr = scache_ptr;
            _soc_sbx_tmu_wb_state_scache_info_p[unit]->init_done = TRUE;
        }
    }
    return rv;
}


/*
 *  Function
 *       soc_sbx_tmu_signature
 *  Description:
 *       Display the CRC signatures of the various TMU data structures
 *  Inputs:
 *     unit - device number
 *  Outputs: 
 */

void soc_sbx_tmu_signature(int unit)
{
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu hashtable 0x%08x\n"), 
                 _soc_sbx_tmu_hashtable_wb_dump_state(unit)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu dram      0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_DRAM)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu region    0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_REGION)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu table     0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_TABLE)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu cmd_dma   0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_CMD_DMA)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu resp_dma  0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_RESP_DMA)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu program   0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_PROGRAM)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu control   0x%08x\n"), 
                 _soc_sbx_tmu_dbase_wb_dump_state(unit, SOC_SBX_TMU_DBASE_CONTROL)));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,"soc tmu fifo      0x%08x\n"), 
                 soc_sbx_tmu_hash_fifo_crc(unit)));
}

#endif /* BCM_WARM_BOOT_SUPPORT */

