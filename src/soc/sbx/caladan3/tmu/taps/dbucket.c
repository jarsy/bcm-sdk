/*
 * $Id: dbucket.c,v 1.49.12.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    dbucket.c
 * Purpose: Caladan3 TAPS dram bucket driver
 * Requires:
 */
#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/hpcm.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/taps/trie.h>
#include <soc/sbx/caladan3/tmu/taps/dbucket.h>
#include <soc/sbx/caladan3/tmu/taps/taps_util.h>

extern soc_heap_sl_mem_chunk_t *g_dpfx_hpcm;

static taps_work_type_e_t work_type[] = {TAPS_DBUCKET_DATA_WORK, TAPS_DBUCKET_WORK};

/*
 *   Function
 *      _hpcm_dbucket_alloc
 *   Purpose
 *      Allocate memory for a dbucket object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (OUT)handle : handle for allocated dbucket object
 *   Returns
 *       SOC_E_NONE - successfully allocated
 *       SOC_E_* as appropriate otherwise
 */
static int _hpcm_dbucket_alloc(int unit, const taps_handle_t taps, taps_dbucket_handle_t *handle)
{
    int rv = SOC_E_NONE;
    int cache_block_size;

    if (_TAPS_KEY_IPV4(taps)) {
	cache_block_size = _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX;
    } else {
	cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;
    }
    *handle = sal_alloc(sizeof(taps_dbucket_t) + sizeof(unsigned int) *	\
			(_TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(taps->param.dbucket_attr.num_dbucket_pfx, cache_block_size)*2 - 1),
			"taps-dbucket");
    if (*handle == NULL) {
        rv = SOC_E_MEMORY;
    } else {
        sal_memset(*handle, 0, sizeof(taps_dbucket_t) + \
                   sizeof(unsigned int) *	\
	     	(_TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(taps->param.dbucket_attr.num_dbucket_pfx, cache_block_size)*2 - 1));
    }

    return rv;
}

/*
 *   Function
 *      _hpcm_dbucket_free
 *   Purpose
 *      free memory of a dbucket object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : handle for dbucket object to be freed
 *   Returns
 *       SOC_E_NONE - successfully freed
 *       SOC_E_* as appropriate otherwise
 */
static int _hpcm_dbucket_free(int unit, taps_dbucket_handle_t handle)
{
    int rv = SOC_E_NONE;

    sal_free(handle);
    return rv;
}

/*
 *   Function
 *      _hpcm_dprefix_alloc
 *   Purpose
 *      Allocate memory for a dbucket prefix object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (OUT)handle : handle for allocated dbucket prefix object
 *   Returns
 *       SOC_E_NONE - successfully allocated
 *       SOC_E_* as appropriate otherwise
 */
static int _hpcm_dprefix_alloc(int unit, const taps_handle_t taps, 
                                taps_dprefix_handle_t *handle, void **hpcm_elem_handle)
{
    int rv = SOC_E_NONE;

    if (mem_pool_enable) {
        rv = hpcm_sl_alloc_payload(unit, g_dpfx_hpcm, (void **)handle, hpcm_elem_handle);
    } else {
        *handle = sal_alloc(sizeof(taps_dprefix_t) + \
                            (BITS2WORDS(taps->param.key_attr.lookup_length)-1) * sizeof(unsigned int), 
                            "taps-dbucket");
        if (*handle == NULL) {
            rv = SOC_E_MEMORY;
        } else {
            sal_memset(*handle, 0, 
                       sizeof(taps_dprefix_t) + 
                       (BITS2WORDS(taps->param.key_attr.lookup_length)-1) * sizeof(unsigned int));
        }
    }
    
    return rv;
}

/*
 *   Function
 *      _hpcm_dprefix_free
 *   Purpose
 *      free memory of a dbucket prefix object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : handle for dbucket prefix object to be freed
 *   Returns
 *       SOC_E_NONE - successfully freed
 *       SOC_E_* as appropriate otherwise
 */
static int _hpcm_dprefix_free(int unit, taps_dprefix_handle_t handle)
{
    int rv = SOC_E_NONE;

    if (mem_pool_enable) {
        rv = hpcm_sl_free_payload(unit, g_dpfx_hpcm, (void *)handle->hpcm_elem_handle);
    } else {
        sal_free(handle);
    }
    
    return rv;
}

/*
 * This is for IPV6 key only.
 * prefix/length in the format expecte by other functions and trie library 
 * return
 * hash[0] - 8 msb bits of crc40 at lsb
 * hash[1] - 32 lsb bits of crc40
 */
int _taps_key_2_psig_128(uint32 *prefix, int length, uint32 hash_adjust, uint32 valid, uint64 *psig)
{
    int rv = SOC_E_NONE;
    uint32 adjust_val, temp;
    uint64 b;
    uint64 b40;
    uint64 crc40;
    uint64 crc40_bit39;
    uint32 adjusted_prefix[TAPS_IPV6_KEY_SIZE_WORDS-1];
    uint32 shifted_prefix[TAPS_IPV6_KEY_SIZE_WORDS];
    int word, shift;

    if ((prefix == NULL) || (psig == NULL)) {
	return SOC_E_PARAM;
    }

    /* shift the prefix/length and mask to 127 bits 
     * step 1: copy the prefix into shifted_prefix buffer
     * step 2: shift the prefix to full 144 bits
     * step 3: get bits 127:0 and put the bits to shifted_prefix[0-3]
     * step 4: mask out bit 127.
     */
    for (word = 0; word < TAPS_IPV6_KEY_SIZE_WORDS; word++) {
	shifted_prefix[word] = prefix[word];
    }    
    rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, &shifted_prefix[0], length, length-TAPS_IPV6_KEY_SIZE);
    if (SOC_FAILURE(rv)) {
	return rv;
    }
    for (word = 0; word < TAPS_IPV6_KEY_SIZE_WORDS-1; word++) {	
	shifted_prefix[word] = shifted_prefix[word+1];
    }
    shifted_prefix[0] &= 0x7FFFFFFF;


    COMPILER_64_ZERO(crc40);
    
    /* hash adjust */
    for (word = TAPS_IPV6_KEY_SIZE_WORDS-2; word >= 0; --word) {
	adjusted_prefix[word] = 0;
	temp = shifted_prefix[word];
	for (shift = 0; shift < 32; shift += 8) {
	    adjust_val = (((temp >> shift) & 0xff) + hash_adjust) & 0xff;
	    if ((word == 0) && (shift == 24)) {
		adjust_val &= 0x7f;
	    }
	    adjusted_prefix[word] |= adjust_val << shift;
	}
    }
    
    /* crc40 */
    for (word = 0; word < TAPS_IPV6_KEY_SIZE_WORDS-1; word++) {
	for (shift = ((word == 0)?30:31); shift >= 0; --shift) {
            uint64 uuTmp;
	    COMPILER_64_SET(b, 0, (adjusted_prefix[word] >> shift) & 1);

	    COMPILER_64_ZERO(b40);
	    crc40_bit39 = crc40;
	    COMPILER_64_SHR(crc40_bit39, 39);
            COMPILER_64_SET(uuTmp, 0, 1);
            COMPILER_64_AND(crc40_bit39, uuTmp);
	    
	    COMPILER_64_XOR(b, crc40_bit39);
	    COMPILER_64_SHL(b, 0);  /* << 0 */
	    COMPILER_64_OR(b40, b);
	    COMPILER_64_SHL(b, 3);  /* << 3 */
	    COMPILER_64_OR(b40, b);
	    COMPILER_64_SHL(b, 14); /* << 17 */
	    COMPILER_64_OR(b40, b);
	    COMPILER_64_SHL(b, 6);  /* << 23 */
	    COMPILER_64_OR(b40, b);
	    COMPILER_64_SHL(b, 3);  /* << 26 */
	    COMPILER_64_OR(b40, b);
	    
	    COMPILER_64_SHL(crc40, 1);
            COMPILER_64_SET(uuTmp, 0xff, 0xffffffff);
	    COMPILER_64_AND(crc40, uuTmp);
	    COMPILER_64_XOR(crc40, b40);
	}
    }
    
    *psig = crc40;
    COMPILER_64_SHL(*psig, 8);
    COMPILER_64_ADD_32(*psig, (((((TAPS_IPV6_KEY_SIZE-length)>=0x7F)?0:(0x7F+length-TAPS_IPV6_KEY_SIZE))<<1) | (valid?1:0)));

    return SOC_E_NONE;
}

/*
 *   Function
 *      taps_dbucket_enqueue_update_payload_work
 *   Purpose
 *      enqueue tmu commands to write payload of a dbucket prefix object into dram
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle for dbucket object which manages the dbucket prefix object
 *      (IN) dph    : handle for dbucket prefix object to be write
 *   Returns
 *       SOC_E_NONE - successfully enqueued the work
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_enqueue_update_payload_work(int unit, 
					     const taps_handle_t taps,
					     const taps_wgroup_handle_t *wgroup,
					     taps_dbucket_handle_t dbh,
					     taps_dprefix_handle_t dph)
{
    int rv = SOC_E_NONE;
    int table_id, entry_num, size, bucket_size;
    soc_sbx_caladan3_tmu_cmd_t *dpld_cmd=NULL;
    int prefix_length, key_length, word;
    uint32 shifted_prefix[TAPS_IPV6_KEY_SIZE_WORDS];
    uint32 original_prefix[TAPS_IPV6_KEY_SIZE_WORDS];
    uint32 payload[_TAPS_PAYLOAD_WORDS_];
     
    /* allocate tmu command */
    rv = tmu_cmd_alloc(unit, &dpld_cmd); 
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate TMU dbucket payload update commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }
    
    /* post tmu xl_write command to write the payload table
     *   table_id:   assigned at init time
     *   entry_num:  each prefix is counted as 1 entry
     *   offset:     0. We don't put multiple payload in 1 entry
     *   lookup:     assigned at init time
     *   size:       assigned at init time, depends on IPV4/IPV6
     *   value_data: 
     *   value_size: same as size
     */
    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    entry_num = ((dbh->domain * taps->param.sbucket_attr.max_pivot_number) + dbh->bucket) * bucket_size * 2 + dph->index;
    size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    
    dpld_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
    dpld_cmd->cmd.xlwrite.table = table_id;
    if (_TAPS_KEY_IPV4(taps)) {
	dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_DATA;
    } else {
	dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
    dpld_cmd->post_dpfx_index = dph->index;
    }
    dpld_cmd->cmd.xlwrite.entry_num = entry_num;
    dpld_cmd->cmd.xlwrite.offset = 0;
    dpld_cmd->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
    dpld_cmd->cmd.xlwrite.value_size = size;    

    if (_TAPS_KEY_IPV4(taps)) {
	/* IPV4: copy over the payload */
	for (word=0; word < BITS2WORDS(dpld_cmd->cmd.xlwrite.value_size); word++) {
	    dpld_cmd->cmd.xlwrite.value_data[word] = dph->payload[word];
	}			
    } else {
	/* IPV6: calculate CPE and copy over payload */
	prefix_length = dph->length;
	sal_memcpy(original_prefix, dph->pfx, sizeof(uint32) * TAPS_IPV6_KEY_SIZE_WORDS);
	sal_memcpy(payload, dph->payload, sizeof(uint32) * _TAPS_PAYLOAD_WORDS_);

	/* convert the key/length to cpe format */
	for (word = 0; word < TAPS_IPV6_KEY_SIZE_WORDS; word++) {
	    shifted_prefix[word] = original_prefix[word];
	}    

	if (prefix_length > (TAPS_IPV6_KEY_SIZE-_TAPS_DBUCKET_IPV6_PREFIX_LENGTH)) {
	    key_length = prefix_length-(TAPS_IPV6_KEY_SIZE-_TAPS_DBUCKET_IPV6_PREFIX_LENGTH);
	    /* shift "key_length" number of lsb bits to bits 127-(127-key_length+1)
	     * pad the lower bits with 0
	     */
	    rv = taps_key_shift(TAPS_IPV6_KEY_SIZE, &shifted_prefix[0],
				    key_length, key_length-128);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to shift key %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		return rv;
	    }
	} else {
	    key_length = 0;
	    for (word = 0; word < TAPS_IPV6_KEY_SIZE_WORDS; word++) {
		shifted_prefix[word] = 0;
	    }
	}

	/* set bit (127-key_length), bits 127-0 is on word1 to word4 */
	_TAPS_SET_KEY_BIT(shifted_prefix, 127-key_length, TAPS_IPV6_KEY_SIZE);
	
	/* write entries */
	for (word=0; word < BITS2WORDS(dpld_cmd->cmd.xlwrite.value_size); word++) {
	    if (word < (_TAPS_DBUCKET_IPV6_PREFIX_LENGTH+31)/32) {
		/* put in the cpe, which is in word1-word4 */
		dpld_cmd->cmd.xlwrite.value_data[word] = shifted_prefix[((_TAPS_DBUCKET_IPV6_PREFIX_LENGTH+31)/32)-word];
	    } else {
		/* put in the payload */
		dpld_cmd->cmd.xlwrite.value_data[word] = payload[word-(_TAPS_DBUCKET_IPV6_PREFIX_LENGTH+31)/32];
	    }
	}
    }

    rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_DBUCKET_DATA_WORK, dpld_cmd);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	tmu_cmd_free(unit, dpld_cmd);
	return rv;
    } else {
	LOG_DEBUG(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "%s payload update for domain %d bucket %d index %d (entry %d).\n"),
	           FUNCTION_NAME(), dbh->domain, dbh->bucket, dph->index, entry_num));
    }

    return rv;
}


#define _MAX_NUM_IPV6_PSIGS (40)
/*
 * info passed for the bucket rehash callback
 */
typedef struct _taps_rehash_dbkt_info_s {
    int unit;
    taps_handle_t taps;
    taps_dbucket_handle_t dbh;
    int start_entry;
    int end_entry;
    uint8 hash_adj;  /* current hash adjust used */
    int num_psigs;   /* number of valid psigs rehashed so far */
    uint64 psigs[_MAX_NUM_IPV6_PSIGS];  /* value of the psigs */
    int status; 
} _taps_rehash_dbkt_info_t;

/*
 * rehash each prefix in the dbucket. Unused entry is not untouched.
 */
static int _taps_dbkt_rehash_cb(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE, index;
    uint64 psig;
    _taps_rehash_dbkt_info_t *info = (_taps_rehash_dbkt_info_t *)user_data;

    /* check if failed before */
    if (SOC_FAILURE(info->status)) {
	return rv;
    }

    if ((dph->index < info->start_entry) || (dph->index >= info->end_entry)) {
	/* do nothing if dph point to an entry outside of rehash range */
	return rv;
    }

    /* rehash the dph */
    rv = _taps_key_2_psig_128(&(dph->pfx[0]), dph->length, info->hash_adj, TRUE, &psig);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to calculate psig for ipv6 prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), info->unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* verify if the newly added psig has any collision with existing psigs */
    for (index = 0; index < info->num_psigs; index++) {
	if (COMPILER_64_EQ(info->psigs[index], psig)) {
	    info->status = SOC_E_EXISTS;
	    return rv;
	}
    }

    /* record the new psig */
    info->psigs[info->num_psigs] = psig;
    info->num_psigs++;
    return rv;
}

/*
 * rehash each prefix in the dbucket with provided hash adjust (assumed to has no collision).
 * and update dbh.cache to reflect the adjust.
 */
static int _taps_dbkt_rehash_update_cache_cb(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE;
    int slave_idx, slave_unit;
    uint64 psig;
    uint32 *buff;
    soc_mem_t mem;
    soc_field_t field;
    _taps_rehash_dbkt_info_t *info = (_taps_rehash_dbkt_info_t *)user_data;
    int bucket_size, cache_block_size;
    soc_field_t ipv6_fields[_TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX]=
	{PSIG0f, PSIG1f, PSIG2f, PSIG3f, PSIG4f};


    /* check if failed before */
    if (SOC_FAILURE(info->status)) {
	return rv;
    }

    if ((dph->index < info->start_entry) || (dph->index >= info->end_entry)) {
	/* do nothing if dph point to an entry outside of rehash range */
	return rv;
    }

    /* rehash the dph */
    rv = _taps_key_2_psig_128(&(dph->pfx[0]), dph->length, info->hash_adj, TRUE, &psig);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to calculate psig for ipv6 prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), info->unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* update dbh cache with new psig and hash_adj */
    bucket_size = info->taps->param.dbucket_attr.num_dbucket_pfx;
    cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;
    
    if ((((dph->index)%bucket_size) >= (bucket_size/cache_block_size)*cache_block_size) &&
	((bucket_size%cache_block_size) <= (cache_block_size/2))) {
	mem = TMB_LPM_DBUCKET_IPV6_128m;
    } else {
	mem = TMB_LPM_DBUCKET_IPV6_256m;
    }
    field = ipv6_fields[((dph->index%bucket_size)%cache_block_size)];

    buff = &info->dbh->cache[0];
    buff += _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX((dph->index)%bucket_size, cache_block_size);    
    soc_mem_field64_set(info->unit, mem, buff, field, psig);    
    soc_mem_field32_set(info->unit, mem, buff, HADJf, info->hash_adj); 

    if (_IS_MASTER_SHARE_LPM_TABLE(info->unit, info->taps->master_unit, info->taps->param.host_share_table)) {
        for (slave_idx = 0; slave_idx < info->taps->num_slaves; slave_idx++) {
            slave_unit = SOC_SBX_CONTROL(info->unit)->slave_units[slave_idx];
            soc_mem_field64_set(slave_unit, mem, buff, field, psig);    
            soc_mem_field32_set(slave_unit, mem, buff, HADJf, info->hash_adj); 
        }
    }
    
    return rv;
}

/*
 *   Function
 *      _taps_rehash_dbucket
 *   Purpose
 *      rehash all dbucket prefix objects in a dbucket object using
 *      specified hash adjust.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) dbh    : handle of the dbucket object to rehash
 *   Returns
 *       SOC_E_NONE - successfully rehashed
 *       SOC_E_EXISTS - the hash adjust value will result in hash collision
 *                    among existing prefixes.
 *       SOC_E_* as appropriate otherwise
 */
static int _taps_rehash_dbucket(int unit, 
				const taps_handle_t taps,
				taps_dbucket_handle_t dbh,
				int start_entry,
				int end_entry,
				uint8 hash_adj)
{
    int rv = SOC_E_NONE;
    _taps_rehash_dbkt_info_t info;

    /* write the payload table, use traverse function */
    info.unit = unit;
    info.taps = taps;
    info.dbh = dbh;
    info.num_psigs = 0;
    info.hash_adj = hash_adj;
    info.start_entry = start_entry;
    info.end_entry = end_entry;
    info.status = SOC_E_NONE;
    rv = taps_dbucket_traverse(unit, taps, NULL, dbh, (void *)(&info), _taps_dbkt_rehash_cb);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failure to rehash dbucket prefix tables %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    if (SOC_SUCCESS(info.status)) {
	/* update dbh cache to reflect the new hash adj */
	rv = taps_dbucket_traverse(unit, taps, NULL, dbh, (void *)(&info), _taps_dbkt_rehash_update_cache_cb);
	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failure to update cache for dbucket prefix tables rehash%d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	    return rv;
	}	
    }

    return info.status;
}

/*
 * Check if there is any hash collisions in the dbh. Adjust hash if so. 
 * Param:
 *   unit
 *   taps
 *   dbh
 *   buff
 *   size
 *   hash_adjusted
 * NOTE: 
 *   hash adjust could be different for each 256/128 bits (untested by hardware).
 *   for now, software is forcing hash adjust in same dbucket to be same.
 *   Software is assuming by now the dph associated with psig has been inserted
 *   into dbucket trie. So the rehash process will be able to recalculate the
 *   psig.
 */
static int _taps_dbucket_collision_detection_and_adjust(int unit,
							const taps_handle_t taps,
							taps_dbucket_handle_t dbh,
							taps_dprefix_handle_t dph,
							uint64 psig,
							uint8 old_hash_adj,
							int *hash_adjusted)
{
    int rv = SOC_E_NONE;
    int start_entry, end_entry;
    uint8 new_hash_adj;
    uint64 tmp_psig;
    uint32 *buff;
    soc_mem_t mem;
    soc_field_t field;
    int index, bucket_size, cache_block_size;
    soc_field_t ipv6_fields[_TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX]=
	{PSIG0f, PSIG1f, PSIG2f, PSIG3f, PSIG4f};

    /* check if psig inserted is uniqe (assuming there is no collision among
     * existing psigs. use existing hash adjust if there is no collison.
     */
    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    if (dph->index > bucket_size) {
	/* check 1* bucket */
	start_entry = bucket_size;
	end_entry = bucket_size* 2;
    } else {
	/* check 0* bucket */
	start_entry = 0;
	end_entry = bucket_size;	
    }

    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;

    *hash_adjusted = 0;
    for (index = start_entry; index < end_entry; index++) {
	if (index == dph->index) {
	    /* skip itself */
	    continue;
	}

	/* get the tmp_psig from cache */
	if ((((index)%bucket_size) >= (bucket_size/cache_block_size)*cache_block_size) &&
	    ((bucket_size%cache_block_size) <= (cache_block_size/2))) {
	    mem = TMB_LPM_DBUCKET_IPV6_128m;
	} else {
	    mem = TMB_LPM_DBUCKET_IPV6_256m;
	}
	field = ipv6_fields[((index%bucket_size)%cache_block_size)];

	buff = &dbh->cache[0];
	buff += _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX((index)%bucket_size, cache_block_size);    
	soc_mem_field64_get(unit, mem, buff, field, &tmp_psig);

	/* compare and check if there is collision */
	if (COMPILER_64_EQ(tmp_psig, psig)) {
	    *hash_adjusted = 1;
	    break;
	}
    }

    if ((*hash_adjusted) == 0) {
	/* no collision, done */
	return SOC_E_NONE;
    }

    /* If collision exist. rehash the whole dram bucket and make sure every psig
     * is uniqe. We try all hash adjust value till we exhaust all possible
     * value (increment till it hit old value).
     */
    for (new_hash_adj = (old_hash_adj+1)%0x80; new_hash_adj != old_hash_adj; new_hash_adj=(new_hash_adj+1)%0x80) {
	rv = _taps_rehash_dbucket(unit, taps, dbh, start_entry, end_entry, new_hash_adj);
	if (SOC_SUCCESS(rv)) {
	    /* rehashed correctly */
	    return rv;
	}
    }

    /* at this point, we tried all hash values and it's still has collison.
     * give up and return error.
     */    
    return SOC_E_EXISTS;
}

/* we only need up to 16 commands since max entry is 8Kb, 
 * each command can handle 1Kb, and we need to process 
 * upto 2 entries (0* and 1*)
 */
#define __TAPS_DBKT_FLUSH_MAX_PFS_CMDS   (16)

int taps_dbucket_collision_rehash(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup, 
                                    uint32 *col_key, uint32 length,
                                    taps_dbucket_handle_t dbh, taps_dprefix_handle_t dph,
                                    uint8 *collision)
{
    int rv = SOC_E_NONE;
    uint32 *buff;
    int cache_index, index, old_hash_adj, bucket_size, new_hash_adj, start_entry, end_entry;
    int table_id, entry, cmd_cnt, total_bytes, bytes_sent, offset, size;
    soc_mem_t mem;
    soc_sbx_caladan3_tmu_cmd_t *dpfx_cmds[__TAPS_DBKT_FLUSH_MAX_PFS_CMDS];
    uint64 col_psig, dph_psig;
    
    if (!taps || !dbh ||!col_key || !dph ||!collision) return SOC_E_PARAM; 
    
    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    cache_index = _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX(dph->index%bucket_size,  _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX);
    cache_index += (dph->index/bucket_size) 
                  * _TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(bucket_size, _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX);
    buff = &(dbh->cache[cache_index]);  

    if (dph->index > bucket_size) {
    	/* check 1* bucket */
    	start_entry = bucket_size;
    	end_entry = bucket_size * 2;
    } else {
    	/* check 0* bucket */
    	start_entry = 0;
    	end_entry = bucket_size;	
    }
    index = dph->index % bucket_size;
    mem = TMB_LPM_DBUCKET_IPV6_256m;
    /* Check if we have a trailing 3 prefix block */
    if(bucket_size%_TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX != 0) {
		if (index >= bucket_size - _TAPS_DBUCKET_IPV6_128BIT_DBKT_NUM_PFX) {
		    /* We are in the trailing 128  bit 3-prefix blocks */
		    mem = TMB_LPM_DBUCKET_IPV6_128m;
		}
    }
    old_hash_adj = soc_mem_field32_get(unit, mem, buff, HADJf);

    /* Detect whether col_key collide with dph */
    _taps_key_2_psig_128(col_key, TAPS_IPV6_KEY_SIZE, old_hash_adj, TRUE, &col_psig);
    _taps_key_2_psig_128(dph->pfx, dph->length, old_hash_adj, TRUE, &dph_psig);

    if (COMPILER_64_EQ(col_psig, dph_psig)) {
        *collision = TRUE;
        for (new_hash_adj = (old_hash_adj+1)%0x80; new_hash_adj != old_hash_adj; new_hash_adj=(new_hash_adj+1)%0x80) {
        	rv = _taps_rehash_dbucket(unit, taps, dbh, start_entry, end_entry, new_hash_adj);
        	if (SOC_SUCCESS(rv)) {
        	    /* rehashed correctly */
        	    break;
        	}
        }
    	/* flush all 0* or 1* dbucket prefixes, 1kb each tmu command till reach end of buckets */
    	table_id =  taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];

    	/* Set up the command array */
    	for (cmd_cnt = 0; cmd_cnt < __TAPS_DBKT_FLUSH_MAX_PFS_CMDS; cmd_cnt++) {
    	    dpfx_cmds[cmd_cnt] = NULL;
    	}

    	/* Start with the first location in the command array */
    	cmd_cnt = 0;

    	/* The location of the entry with msb=0 */

        entry = ((dbh->domain * taps->param.sbucket_attr.max_pivot_number) + dbh->bucket) * 2
                + dph->index/bucket_size;

        /* How many bytes we need to send.  The dbucket cache size in bytes  */
        total_bytes = 4 * _TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(bucket_size, _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX);
        bytes_sent = 0;
        
        /* Send over the cache */
        while(bytes_sent < total_bytes)
        {	    
        	/* allocate tmu command */
        	rv = tmu_cmd_alloc(unit, &dpfx_cmds[cmd_cnt]);
        	if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate TMU dbucket prefix flush commands %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        	    break;
        	}
        	
        	/* location of next block of bytes */
        	buff = (uint32 *)((char *)dbh->cache + bytes_sent + (entry%2)*total_bytes);
            
        	/* Where to write the chunk to: in units of  8 bytes */
        	offset = bytes_sent / 8;
        	
        	/* The size we can write */
        	if(8*(total_bytes - bytes_sent) >=1024) {
        	    size=1024;
        	} else {
        	    size= 8 * (total_bytes-bytes_sent);
        	}
        	
        	/* Set up the command */
        	dpfx_cmds[cmd_cnt]->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
        	dpfx_cmds[cmd_cnt]->cmd.xlwrite.table = table_id;
        	dpfx_cmds[cmd_cnt]->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
        	dpfx_cmds[cmd_cnt]->cmd.xlwrite.entry_num = entry;
        	dpfx_cmds[cmd_cnt]->cmd.xlwrite.offset = offset;
        	dpfx_cmds[cmd_cnt]->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
        	dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_size = size;
        	for (index=0; index < BITS2WORDS(dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_size); index++) {
        	    dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_data[index] = buff[index];
        	}
            
        	/* Enqueue it for transmissiont */
            rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_DBUCKET_WORK, dpfx_cmds[cmd_cnt]);
        	if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        	    break;
        	}
        	
        	/* Bump up the count of bytes sent */
        	bytes_sent += size/8;
        	
        	/* Cue up the next command */
        	cmd_cnt++;
        }
           
    	if (SOC_FAILURE(rv)) {
    	    /* free all commands enqueued so far */
    	    for (; (int)cmd_cnt>=0;cmd_cnt--) {
        		if (dpfx_cmds[cmd_cnt] != NULL) {
        		    tmu_cmd_free(unit, dpfx_cmds[cmd_cnt]);
        		}
    	    }
    	} 
    }

    return rv;
}

/*
 *   Function
 *      _taps_dbucket_enqueue_work
 *   Purpose
 *      enqueue tmu commands for operations on a dbucket object.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle for dbucket object which manages the dbucket prefix object
 *      (IN) dph    : handle for dbucket prefix object to be write. Not valid
 *                    if (op == TAPS_DBUCKET_OP_FLUSH_PFX)
 *      (IN) op     : operations. see "taps_dbucket_operation_type_e_t"
 *                TAPS_DBUCKET_OP_INSERT:
 *                   insert a prefix. update both dbucket prefix table and payload table
 *                TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE:
 *                   same as TAPS_DBUCKET_OP_INSERT but don't enqueue tmu cmd
 *                TAPS_DBUCKET_OP_DELETE
 *                   delete a prefix. update dbucket prefix table only.
 *                TAPS_DBUCKET_OP_DELETE_NO_ENQUEUE
 *                   same as TAPS_DBUCKET_OP_DELETE but don't enqueue tmu cmd
 *                TAPS_DBUCKET_OP_UPDATE
 *                   update payload of a prefix. update dbucket payload table only.
 *                TAPS_DBUCKET_OP_INSERT_DEFAULT
 *                   insert a wildcard "-" entry of dbucket. update dbucket prefix table
 *                   to match nothing (similar to DELETE), update payload table
 *                TAPS_DBUCKET_OP_INSERT_DEFAULT_NO_ENQUEUE
 *                   same as TAPS_DBUCKET_OP_INSERT_DEFAULT but don't enqueue tmu cmd
 *                TAPS_DBUCKET_OP_FLUSH_PFX
 *                   update the dbucket prefix table for all prefixes in dbucket.
 *                   no update for the payload of the prefixes.
 *   Returns
 *       SOC_E_NONE - successfully enqueued all works
 *       SOC_E_* as appropriate otherwise
 *   Future: 
 *       IPV6 support
 */
static int _taps_dbucket_enqueue_work(int unit, 
				      const taps_handle_t taps,
				      const taps_wgroup_handle_t *wgroup,
				      taps_dbucket_handle_t dbh,
				      taps_dprefix_handle_t dph,
				      taps_dbucket_operation_type_e_t op)
{
    int rv = SOC_E_NONE;
    int bucket_size, cache_index, cache_block_size, index;
    int table_id, entry_num, size, offset;
    int old_hash_adj, hash_adjusted, flush_pfx_bucket = FALSE;
    uint32 *buff;
    uint32 cpe, length, cmd_cnt;
    uint64 psig;
    soc_mem_t mem;
    soc_field_t field;
    soc_field_t ipv6_fields[_TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX]=
	{PSIG0f, PSIG1f, PSIG2f, PSIG3f, PSIG4f};
    soc_sbx_caladan3_tmu_cmd_t *dpfx_cmd=NULL;
    soc_sbx_caladan3_tmu_cmd_t *dpfx_cmds[__TAPS_DBKT_FLUSH_MAX_PFS_CMDS];
    int entry, total_bytes, bytes_sent;

    /*== parameter sanity check */
    if (!taps || !wgroup || !dbh || op >= TAPS_DBUCKET_OP_MAX) {
	return SOC_E_PARAM;
    }

    if ((op != TAPS_DBUCKET_OP_FLUSH_PFX) && !dph) {
        return SOC_E_PARAM;
    }

    if (op == TAPS_DBUCKET_OP_FLUSH_PFX) {
	flush_pfx_bucket = TRUE;
    }

    /*== update dbucket prefix */
    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    if (_TAPS_KEY_IPV4(taps)) {
	cache_block_size = _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX;
    } else {
	cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;
    }
    if ((op == TAPS_DBUCKET_OP_INSERT) ||
	(op == TAPS_DBUCKET_OP_INSERT_DEFAULT) ||
	(op == TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE) ||
	(op == TAPS_DBUCKET_OP_INSERT_DEFAULT_NO_ENQUEUE) ||
	(op == TAPS_DBUCKET_OP_DELETE) ||
	(op == TAPS_DBUCKET_OP_DELETE_NO_ENQUEUE)) {

	if ((op == TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE) ||
	    (op == TAPS_DBUCKET_OP_INSERT_DEFAULT_NO_ENQUEUE) ||
	    (op == TAPS_DBUCKET_OP_DELETE_NO_ENQUEUE)) {
	    /* update cache only, no tmu command enqueue */
	    dpfx_cmd = NULL;
	} else {
	    /* allocate tmu command */
	    rv = tmu_cmd_alloc(unit, &dpfx_cmd); 
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to allocate TMU dbucket prefix update commands %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		return rv;
	    }
	}

	/* locate and update the cached copy of the dbucket prefix */
	cache_index = _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX(dph->index%bucket_size, cache_block_size);
	cache_index += (dph->index/bucket_size) * _TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(bucket_size, cache_block_size);
	
	buff = &(dbh->cache[cache_index]);
	
	if (_TAPS_KEY_IPV4(taps)) {
	    /* Common calcs for all cases */
	    index = dph->index % bucket_size; /* runs from 0 to bucket_size - 1 */
	    offset = (index/cache_block_size) * 4;
	    mem = TMB_LPM_DBUCKET_IPV4_256m;
	    size = 256;
	    
	    /* Check if we have a trailing 3 prefix block */
	    if(bucket_size%_TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX != 0) {
		if (index >= bucket_size - _TAPS_DBUCKET_IPV4_128BIT_DBKT_NUM_PFX) {
		    /* We are in the trailing 128  bit 3-prefix blocks */
		    mem = TMB_LPM_DBUCKET_IPV4_128m;
		    size = 128;
		}
	    }
	    
	    if ((op == TAPS_DBUCKET_OP_INSERT) ||
		(op == TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE)) {
		/* IPV4 convert low 31 bits of prefix to CPE format */
		if (dph->length>(TAPS_IPV4_KEY_SIZE-_TAPS_DBUCKET_IPV4_PREFIX_LENGTH)) {
		    length = dph->length-(TAPS_IPV4_KEY_SIZE-_TAPS_DBUCKET_IPV4_PREFIX_LENGTH);
		    cpe = dph->pfx[1];
		} else {
		    length = 0;
		    cpe = 0;
		}
        /* coverity[large_shift : FALSE] */
		cpe = _TAPS_KEY_2_CPE_31B(cpe, length);
	    } else {
		/* DELETE or INSERT_DEFAULT. CPE == 0 means unused/not matched */
		cpe = 0;
		length = 0;
	    }
	    buff[index%cache_block_size] = cpe;
	} else {
	    /* IPV6: depends on bucket_size and the dph->index in bucket, we might need to
	     * use different mem type (256 bits vs 128 bits). We detect it by checking
	     * if index are in the last cache block, and if the last cache block 
	     * is 128 bits.
	     */
	    index = dph->index % bucket_size; /* runs from 0 to bucket_size - 1 */
	    offset = (index/cache_block_size) * 4;
	    field = ipv6_fields[index%cache_block_size];
	    mem = TMB_LPM_DBUCKET_IPV6_256m;
	    size = 256;
	    
	    /* Check if we have a trailing 3 prefix block */
	    if(bucket_size%_TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX != 0) {
		if (index >= bucket_size - _TAPS_DBUCKET_IPV6_128BIT_DBKT_NUM_PFX) {
		    /* We are in the trailing 128  bit 3-prefix blocks */
		    mem = TMB_LPM_DBUCKET_IPV6_128m;
		    size = 128;
		}
	    }
	    
	    old_hash_adj = soc_mem_field32_get(unit, mem, buff, HADJf);

	    if ((op == TAPS_DBUCKET_OP_INSERT) ||
		(op == TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE)) {
		/* IPV6 perform hashing to caculate PSIG. 48bits */
		rv = _taps_key_2_psig_128(&(dph->pfx[0]), dph->length, old_hash_adj, TRUE, &psig);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to calculate psig for ipv6 prefix %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		    return rv;
		}		
	    } else {
		/* DELETE or INSERT_DEFAULT. PSIG == 0 means unused, 48bits */
		COMPILER_64_ZERO(psig);
	    }	    
	    soc_mem_field64_set(unit, mem, buff, field, psig);
	    
	    /* ipv6 collision handling */
	    if ((op == TAPS_DBUCKET_OP_INSERT) ||
		(op == TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE)) {
		/* detect hash collision in the dbucket for the prefixes that are not shadowing each other.
		 * we should try all hash adjust value. If no hash adjust value could be found
		 * to avoid collison, then we should return error. We can check whether hash adjust
		 * value is different to determine if the whole dbucket prefixes need to be flushed
		 */		
		rv = _taps_dbucket_collision_detection_and_adjust(unit, taps, dbh, dph, psig,
								  old_hash_adj, &hash_adjusted);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to avoid psig collision for ipv6 prefix %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		    return rv;
		}		

		if (hash_adjusted) {
		    flush_pfx_bucket = TRUE;
		}
	    }
	}
	
	/* post tmu xl_write command to write the dbucket prefix table
	 *   table_id:   assigned at init time
	 *   entry_num:  0* and 1* each count as 1 entry. 0* at even, 1* at odd position
	 *   offset:     which cache block the dph->index falls in.
	 *   lookup:     assigned at init time
	 *   size:       256 or 128 bits
	 *   value_data: 
	 *   value_size: same as size
	 */
	table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
	entry_num = ((dbh->domain * taps->param.sbucket_attr.max_pivot_number) + dbh->bucket) * 2 + dph->index/bucket_size;
	
	if (dpfx_cmd) {
	    dpfx_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
	    dpfx_cmd->cmd.xlwrite.table = table_id;
	    if (_TAPS_KEY_IPV4(taps)) {
		dpfx_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_BUCKET;
	    } else {
		dpfx_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
            if (op != TAPS_DBUCKET_OP_DELETE) {
                dpfx_cmd->post_dpfx_index = dph->index;
            }
	    }
	    dpfx_cmd->cmd.xlwrite.entry_num = entry_num;
	    dpfx_cmd->cmd.xlwrite.offset = offset;
	    dpfx_cmd->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
	    dpfx_cmd->cmd.xlwrite.value_size = size;
	    for (index=0; index < BITS2WORDS(dpfx_cmd->cmd.xlwrite.value_size); index++) {
		dpfx_cmd->cmd.xlwrite.value_data[index] = buff[index];
	    }		

        rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_DBUCKET_WORK, dpfx_cmd);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		tmu_cmd_free(unit, dpfx_cmd);
		return rv;
	    }
	}
	
	LOG_DEBUG(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "%s prefix update for domain %d bucket %d index %d (entry %d).\n"),
	           FUNCTION_NAME(), dbh->domain, dbh->bucket, dph->index, entry_num));
    }
    
    if (flush_pfx_bucket == TRUE) {
	/* flush all 0* and 1* dbucket prefixes, 1kb each tmu command till reach end of buckets */
	table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];

	/* Set up the command array */
	for (cmd_cnt = 0; cmd_cnt < __TAPS_DBKT_FLUSH_MAX_PFS_CMDS; cmd_cnt++) {
	    dpfx_cmds[cmd_cnt] = NULL;
	}

	/* Start with the first location in the command array */
	cmd_cnt = 0;

	/* The location of the entry with msb=0 */
	for (entry = ((dbh->domain * taps->param.sbucket_attr.max_pivot_number) + dbh->bucket) * 2;
	     entry <= ((dbh->domain * taps->param.sbucket_attr.max_pivot_number) + dbh->bucket) * 2 + 1;
	     entry++) {

	    /* How many bytes we need to send.  The dbucket cache size in bytes  */
	    total_bytes = 4 * _TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(bucket_size, cache_block_size);
	    bytes_sent = 0;
	    
	    /* Send over the cache */
	    while(bytes_sent < total_bytes)
	    {	    
		/* allocate tmu command */
		rv = tmu_cmd_alloc(unit, &dpfx_cmds[cmd_cnt]);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate TMU dbucket prefix flush commands %d:%s !!!\n"),
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		    break;
		}
		
		/* location of next block of bytes */
		buff = (uint32 *)((char *)dbh->cache + bytes_sent + (entry%2)*total_bytes);
	    
		/* Where to write the chunk to: in units of  8 bytes */
		offset = bytes_sent / 8;
		
		/* The size we can write */
		if(8*(total_bytes - bytes_sent) >=1024) {
		    size=1024;
		} else {
		    size= 8 * (total_bytes-bytes_sent);
		}
		
		/* Set up the command */
		dpfx_cmds[cmd_cnt]->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
		dpfx_cmds[cmd_cnt]->cmd.xlwrite.table = table_id;
		if (_TAPS_KEY_IPV4(taps)) {
		    dpfx_cmds[cmd_cnt]->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_BUCKET;
		} else {
		    dpfx_cmds[cmd_cnt]->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
		}
		dpfx_cmds[cmd_cnt]->cmd.xlwrite.entry_num = entry;
		dpfx_cmds[cmd_cnt]->cmd.xlwrite.offset = offset;
		dpfx_cmds[cmd_cnt]->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
		dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_size = size;
		for (index=0; index < BITS2WORDS(dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_size); index++) {
		    dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_data[index] = buff[index];
		}

		/* Enqueue it for transmissiont */
        rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_DBUCKET_WORK, dpfx_cmds[cmd_cnt]);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"),
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
		    break;
		}
		
		/* Bump up the count of bytes sent */
		bytes_sent += size/8;
		
		/* Cue up the next command */
		cmd_cnt++;
	    }
	    if (SOC_FAILURE(rv)) {
		break;
	    }
	}

	if (SOC_FAILURE(rv)) {
	    /* free all commands enqueued so far */
	    for (; (int)cmd_cnt>=0;cmd_cnt--) {
		if (dpfx_cmds[cmd_cnt] != NULL) {
		    tmu_cmd_free(unit, dpfx_cmds[cmd_cnt]);
		}
	    }
	} else {
	    LOG_DEBUG(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "%s flush all prefix for dbucket domain %d bucket %d.\n"),
	               FUNCTION_NAME(), dbh->domain, dbh->bucket));
	}

	return rv;
    }

    /*== update payload */
    if ((op != TAPS_DBUCKET_OP_DELETE) &&
	(op != TAPS_DBUCKET_OP_DELETE_NO_ENQUEUE) &&
	(op != TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE) &&
	(op != TAPS_DBUCKET_OP_INSERT_DEFAULT_NO_ENQUEUE) &&
	!((op == TAPS_DBUCKET_OP_FLUSH_PFX) && (_TAPS_KEY_IPV4(taps)))) {
        rv = taps_dbucket_enqueue_update_payload_work(unit, taps, wgroup, dbh, dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue dbucket payload update work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	    if (dpfx_cmd) {
		tmu_cmd_free(unit, dpfx_cmd);
	    }
	    return rv;
        }	
    }

    /* debug mode only
     * NOTE: doing this are not following the update hardware command order
     * so might cause glitchs before all hardware commands play through.
     */
#if 0
	if (op == TAPS_DBUCKET_OP_DELETE) {
	    rv = taps_dbucket_entry_debug_init(unit, taps, dbh->domain, dbh->bucket, dph->index);
	}
#endif
    return rv;
}

/*
* Clear one entry in prefix table 
*/
int taps_dbucket_prefix_table_entry_clear(int unit, taps_handle_t taps,
    taps_wgroup_handle_t wgroup, int domain_id, int dbucket_id)
{
    int rv = SOC_E_NONE;
    int bucket_size, cache_block_size, index;
    int table_id, size, offset;
    int entry, total_bytes, bytes_sent, cmd_cnt;
    soc_sbx_caladan3_tmu_cmd_t *dpfx_cmds[__TAPS_DBKT_FLUSH_MAX_PFS_CMDS];

    /*== parameter sanity check */
    if (!taps || !wgroup) {
	    return SOC_E_PARAM;
    }

    /*== update dbucket prefix */
    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    if (_TAPS_KEY_IPV4(taps)) {
	cache_block_size = _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX;
    } else {
	cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;
    }

    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    cmd_cnt = 0;

    /* The location of the entry with msb=0 */
    for (entry = ((domain_id * taps->param.sbucket_attr.max_pivot_number) + dbucket_id) * 2;
         entry <= ((domain_id * taps->param.sbucket_attr.max_pivot_number) + dbucket_id) * 2 + 1;
         entry++) {

        /* How many bytes we need to send.  The dbucket cache size in bytes  */
        total_bytes = 4 * _TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(bucket_size, cache_block_size);
        bytes_sent = 0;
        
        /* Send over the cache */
        while(bytes_sent < total_bytes)
        {       
        /* allocate tmu command */
        rv = tmu_cmd_alloc(unit, &dpfx_cmds[cmd_cnt]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate TMU dbucket prefix flush commands %d:%s !!!\n"),
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        
        /* Where to write the chunk to: in units of  8 bytes */
        offset = bytes_sent / 8;
        
        /* The size we can write */
        if(8*(total_bytes - bytes_sent) >=1024) {
            size=1024;
        } else {
            size= 8 * (total_bytes-bytes_sent);
        }
        
        /* Set up the command */
        dpfx_cmds[cmd_cnt]->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
        dpfx_cmds[cmd_cnt]->cmd.xlwrite.table = table_id;
        if (_TAPS_KEY_IPV4(taps)) {
            dpfx_cmds[cmd_cnt]->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_BUCKET;
        } else {
            dpfx_cmds[cmd_cnt]->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
        }
        dpfx_cmds[cmd_cnt]->cmd.xlwrite.entry_num = entry;
        dpfx_cmds[cmd_cnt]->cmd.xlwrite.offset = offset;
        dpfx_cmds[cmd_cnt]->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
        dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_size = size;
        for (index=0; index < BITS2WORDS(dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_size); index++) {
            dpfx_cmds[cmd_cnt]->cmd.xlwrite.value_data[index] = 0;
        }

        /* Enqueue it for transmissiont */
        rv = taps_work_enqueue(unit, wgroup, TAPS_DBUCKET_WORK, &dpfx_cmds[cmd_cnt]->wq_list_elem);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"),
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        
        /* Bump up the count of bytes sent */
        bytes_sent += size/8;
        
        /* Cue up the next command */
        cmd_cnt++;
        }
        if (SOC_FAILURE(rv)) {
        break;
        }
    }
         
    if (SOC_FAILURE(rv)) {
        /* free all commands enqueued so far */
        for (; cmd_cnt >= 0;cmd_cnt--) {
            if (dpfx_cmds[cmd_cnt] != NULL) {
             tmu_cmd_free(unit, dpfx_cmds[cmd_cnt]);
            }
        }
    }

    return rv;
}

/*
 *   Function
 *      taps_dbucket_alloc
 *   Purpose
 *      create and initialize a dram bucket object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) domain : tcam domain
 *      (IN) bucket : sram bucket id
 *      (OUT) dbh   : handle of the dbucket object created
 *   Returns
 *       SOC_E_NONE - successfully created
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_create(int unit, 
                        const taps_handle_t taps, 
                        const taps_wgroup_handle_t wgroup,
			unsigned int domain,
                        unsigned int bucket,
                        taps_dbucket_handle_t *dbh)
{
    int rv = SOC_E_NONE;
    
    /* parameter sanity check */
    if (!dbh || !wgroup || !taps) return SOC_E_PARAM;  

    if (bucket >= taps->param.sbucket_attr.max_pivot_number) {
	/* sbucket size is hardcoded for now, hardware only support 1 size anyhow */
	return SOC_E_PARAM;
    }

    /* alloc memory */
    rv = _hpcm_dbucket_alloc(unit, taps, dbh);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate memory for dram bucket:%d :%s !!!\n"),
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* trie init */
    rv = trie_init(taps->param.key_attr.lookup_length, &((*dbh)->trie0));
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to init dbucket 0* trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	(void)taps_dbucket_destroy(unit, taps, wgroup, *dbh);
	return rv;
    }

    rv = trie_init(taps->param.key_attr.lookup_length, &((*dbh)->trie1));
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to init dbucket 0* trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	(void)taps_dbucket_destroy(unit, taps, wgroup, *dbh);
	return rv;
    }

    /* update info */
    (*dbh)->bucket = bucket;
    (*dbh)->domain = domain;

    rv = taps_dbucket_prefix_table_entry_clear(unit, taps, wgroup, domain,
                                            bucket);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to clear prefix table :%d :%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        (void)taps_dbucket_destroy(unit, taps, wgroup, *dbh);
        return rv;
    } 
    
    return SOC_E_NONE;
}

/*
 *   Function
 *      taps_dbucket_alloc
 *   Purpose
 *      Destroy a dram bucket object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle of the dbucket object to be destroyed
 *   Returns
 *       SOC_E_NONE - successfully destroyed
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_destroy(int unit, 
                         const taps_handle_t taps, 
                         const taps_wgroup_handle_t wgroup,
                         taps_dbucket_handle_t dbh)
{
    int rv = SOC_E_NONE;

    /* parameter sanity check */
    if (!taps || !wgroup || !dbh) return SOC_E_PARAM;

    
    if (dbh->trie0) {
	rv = trie_destroy(dbh->trie0);
	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to destroy dbucket trie0 %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	    return rv;
	}
    }

    if (dbh->trie1) {
	rv = trie_destroy(dbh->trie1);
	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to destroy dbucket trie1 %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	    return rv;
	}
    }

    /* free the dbucket itself */
    rv = _hpcm_dbucket_free(unit, dbh);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to destroy dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     taps_dbucket_prefix_id_alloc
 * Purpose:
 *     allocate dram prefix id
 * Note:
 *     not used for now
 */
int taps_dbucket_prefix_id_alloc(int unit, 
                                 taps_handle_t taps, 
                                 taps_dbucket_handle_t dbh,
                                 unsigned int *id)
{
    /* do nothing for now, we might not need upper layer
     * to know the prefix id, so hide everything in the
     * insert/delete/find_prefix interfaces
     */
    *id = _TAPS_INV_ID_;
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     taps_dbucket_prefix_id_free
 * Purpose:
 *     free dram prefix id
 * Note:
 *     not used for now
 */
int taps_dbucket_prefix_id_free(int unit, 
                                taps_handle_t taps, 
                                taps_dbucket_handle_t dbh,
                                unsigned int id)
{
    /* do nothing for now, we might not need upper layer
     * to know the prefix id, so hide everything in the
     * insert/delete/find_prefix interfaces
     */
    return SOC_E_NONE;
}

#define _TAPS_DBUCKET_INSERT_PREFIX_FLAG_NO_ENQUEUE    (1<<0)
#define _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID    (1<<1)

/*
 *   Function
 *      _taps_dbucket_insert_prefix
 *   Purpose
 *      Internal function to insert a prefix into dbucket object, return the dbucket 
 *      prefix object handle if insert succeeds
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle of the dbucket object to insert the prefix
 *      (IN) prefix : prefix key buffer
 *      (IN) length : prefix key length
 *      (IN) payload: payload buffer for the prefix
 *      (IN) pivot_len: dbucket's corresponding sbucket entry pivot length
 *      (IN) flags  : flags, 
 *                   _TAPS_DBUCKET_INSERT_PREFIX_FLAG_NO_ENQUEUE
 *                       do not enqueue work into work_queue if specified
 *                   _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID
 *                       pass in dbucket prefix object instead of allocate a new one
 *      (IN/OUT)ndph:pass in dbucket prefix object handle if 
 *                   _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID is specified.
 *                   otherwise return the handle for newly allocated dbucket prefix object
 *   Returns
 *       SOC_E_NONE - successfully inserted
 *       SOC_E_* as appropriate otherwise
 */
static int _taps_dbucket_insert_prefix(int unit, 
				       const taps_handle_t taps,
				       const taps_wgroup_handle_t *wgroup,
				       taps_dbucket_handle_t dbh,
				       uint32 *prefix, 
				       int length,
				       uint32 *payload,
				       void *cookie,
				       uint32 pivot_len,
				       int flags,
				       taps_dprefix_handle_t *ndph)
{
    int bit=0, rv=SOC_E_NONE, msb=0, word_idx=0;
    SHR_BITDCL *bmap = NULL;
    SHR_BITDCL bpm[SHR_BITALLOCSIZE(TAPS_IPV6_KEY_SIZE)];
    trie_t     *trie = NULL;
    trie_node_t *payload_node = NULL;
    taps_dbucket_operation_type_e_t op;
    void *hpcm_elem_handle = NULL;
    
    if (!taps || !wgroup || !dbh || !prefix || !ndph) {
	return SOC_E_PARAM;
    }
    
    if ((flags & _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID) &&
	((*ndph) == NULL)) {
	return SOC_E_PARAM;
    }

    if (length < pivot_len) {
	rv = SOC_E_PARAM;
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Try to insert prefix with length %d"
                              "shorter than dbucket pivot length %d %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, length, pivot_len, rv, soc_errmsg(rv)));
	return rv;
    } else {
	if (length == pivot_len) {
	    /* both bucket should have room at this point, we always
	     * put it in 0* bucket for the dbucket pivot itself
	     */
	    msb = 0;
	} else {
	    /* check which sub-bucket the prefix should go to 
	     * it should be the msb bit after the dbucket pivot lsb bit
	     */
	    msb = _TAPS_GET_KEY_BIT(prefix, (length-pivot_len-1), taps->param.key_attr.lookup_length)?1:0;
	}

	if (msb) {
	    bmap = &(dbh->pfx_bmap1[0]);
	    trie = dbh->trie1;
	} else {
	    bmap = &(dbh->pfx_bmap0[0]);
	    trie = dbh->trie0;
	}

	/* allocate an entry in corresponding dbucket, error when none avaiable
	 * we always allocate from bit 0 so that if user create the wildcard entry
	 * at dbucket create time, we will end up using entry 0 of 0* bucket
	 */
	for (bit = 0; bit <  taps->param.dbucket_attr.num_dbucket_pfx; bit++) {
	    if (SHR_BITGET(&(bmap[bit>>5]), (bit&0x1F)) == 0) {
		break;
	    }
	}
	if (bit == taps->param.dbucket_attr.num_dbucket_pfx) {
	    /* it should never happen due to proactive dbucket spliting */
	    rv = SOC_E_FULL;
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate an entry in dbucket 0x%x %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, (uint32)dbh, rv, soc_errmsg(rv)));
	    return rv;
	}
    }
    
    if ((flags & _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID) == 0) {
    	/* allocate dph */
        rv = _hpcm_dprefix_alloc(unit, taps, ndph, &hpcm_elem_handle);
    	if (SOC_FAILURE(rv)) {
    	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to create dbuket prefix structure %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    	    return rv;
    	}
        (*ndph)->hpcm_elem_handle = hpcm_elem_handle;
    }
    
    /* insert into trie dbase, bpm bitmask is itself */
    sal_memset(&(bpm[0]), 0, SHR_BITALLOCSIZE(TAPS_IPV6_KEY_SIZE));
    if ((length) && (length >= pivot_len)) {
	/* bpm for the pivot is maintained in sbucket, no need to set here */
	_TAPS_SET_KEY_BIT(bpm, 0, taps->param.key_attr.lookup_length);
    }

    rv = trie_insert(trie, prefix, &(bpm[0]), length, &(*ndph)->node);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to insert dbuket prefix into dbucket trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* update dph info, need to be done before the enqueue_work 
     * when caller pass in dph, we might need to recalculate index
     * since it might be in new subbucket in the new dbucket
     * other should be same.
     */
    (*ndph)->index = bit + msb * taps->param.dbucket_attr.num_dbucket_pfx;

    if ((flags & _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID) == 0) {
	(*ndph)->bpm = FALSE;  /* init to false, upper layer might change it */
	(*ndph)->length = length;
	for (word_idx = 0; word_idx < BITS2WORDS(taps->param.key_attr.lookup_length); word_idx++) {
	    (*ndph)->pfx[word_idx] = prefix[word_idx];
	}
	for (word_idx = 0; word_idx < _TAPS_PAYLOAD_WORDS_; word_idx++) {
	    (*ndph)->payload[word_idx] = payload[word_idx];
	}
	(*ndph)->cookie = cookie;
    }

    if ((flags & _TAPS_DBUCKET_INSERT_PREFIX_FLAG_NO_ENQUEUE) == 0) {
	/* update cache only and no enqueue */
	if (length == pivot_len) {
	    op = TAPS_DBUCKET_OP_INSERT_DEFAULT;
	} else {
	    op = TAPS_DBUCKET_OP_INSERT;
	}
    } else {
	if (length == pivot_len) {
	    op = TAPS_DBUCKET_OP_INSERT_DEFAULT_NO_ENQUEUE;
	} else {
	    op = TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE;
	}
    }

    rv = _taps_dbucket_enqueue_work(unit, taps, wgroup, dbh, *ndph, op);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to build work item for dbuket prefix insert %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	/* clean up trie and free the dph structure */
	trie_delete(trie, prefix, length, &payload_node);
	if (!(flags & _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID)) {
	_hpcm_dprefix_free(unit, *ndph);
    *ndph = NULL;
    }
	return rv;
    }

    /* mark entry as used */
    SHR_BITSET(&(bmap[bit>>5]), (bit&0x1F));
    
    return rv;
}

/*
 *   Function
 *      taps_dbucket_insert_prefix
 *   Purpose
 *      Insert a prefix into dbucket object, return the dbucket 
 *      prefix object handle if insert succeeds
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle of the dbucket object to insert the prefix
 *      (IN) prefix : prefix key buffer
 *      (IN) length : prefix key length
 *      (IN) payload: payload buffer for the prefix
 *      (IN) pivot_len: dbucket's corresponding sbucket entry pivot length
 *      (OUT)ndph   : the handle for newly allocated dbucket prefix object
 *   Returns
 *       SOC_E_NONE - successfully inserted
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_insert_prefix(int unit, 
			       const taps_handle_t taps,
			       const taps_wgroup_handle_t *wgroup,
			       taps_dbucket_handle_t dbh,
			       uint32 *prefix, 
			       int length,       /* prefix length */
			       uint32 *payload,
			       void *cookie,
			       uint32 pivot_len, /* dbucket pivot length */
			       /* OUT */
			       taps_dprefix_handle_t *ndph)
{
    *ndph = NULL;
    return _taps_dbucket_insert_prefix(unit, taps, wgroup, dbh, prefix, length, payload, cookie,
				       pivot_len, 0, ndph);
}

/*
 *   Function
 *      taps_dbucket_delete_prefix
 *   Purpose
 *      Delete a prefix from the dram bucket.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle of the dbucket object to delete from
 *      (IN) dph    : handle of the dbucket prefix object to delete
 *   Returns
 *       SOC_E_NONE - successfully deleted
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_delete_prefix(int unit, 
                               const taps_handle_t taps,
                               const taps_wgroup_handle_t *wgroup,
                               taps_dbucket_handle_t dbh,
                               taps_dprefix_handle_t dph)
{
    int bit = 0, rv=SOC_E_NONE;
    SHR_BITDCL *bmap = NULL;
    trie_t     *trie = NULL;
    trie_node_t *payload = NULL;

    if (!taps || !wgroup || !dbh || !dph) {
	return SOC_E_PARAM;
    }

    /* by checking index we know which sub-bucket we should delete from */
    if (dph->index >= taps->param.dbucket_attr.num_dbucket_pfx) {
	bmap = &(dbh->pfx_bmap1[0]);
	trie = dbh->trie1;
	bit = dph->index - taps->param.dbucket_attr.num_dbucket_pfx;
    } else {
	bmap = &(dbh->pfx_bmap0[0]);
	trie = dbh->trie0;
	bit = dph->index;
    }

    /* remove from trie */
    rv = trie_delete(trie, &(dph->pfx[0]), dph->length, &payload);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to delete dbuket prefix from trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    /* During error_clean_up, if error happen in trie_insert, it is possible there is no dph's node in trie
        * Should continue to free the dph to avoid memory leak
        */
    /* return rv;*/
    }
    
    /* build work items in dbucket queue */
    rv = _taps_dbucket_enqueue_work(unit, taps, wgroup, dbh, dph, TAPS_DBUCKET_OP_DELETE);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to build work item for dbuket prefix delete %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* free the dph */
    rv = _hpcm_dprefix_free(unit, dph);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to free dbuket prefix structure %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	/* even if we failed to free the dph memory, we still mark it as freed
	 * memory free should be pretty straight forward, and if it's wrong
	 * it indicated something else is seriously wrong, we should return 
	 * error and simply leak the dph memory here instead of roll-back
	 * previous delete steps.
	 */
	SHR_BITCLR(&(bmap[bit>>5]), (bit&0x1F));
	return rv;
    }    

    /* mark entry as free */
    SHR_BITCLR(&(bmap[bit>>5]), (bit&0x1F));

    return rv;
}

/*
 *   Function
 *      taps_dbucket_insert_default_entry
 *   Purpose
 *      Insert default entry in dram for slave unit
 */
int taps_dbucket_insert_default_entry(int unit, taps_handle_t taps, 
                                    taps_wgroup_handle_t wgroup)
{
    int rv = SOC_E_NONE;
    int index;
    int size;
    soc_sbx_caladan3_tmu_cmd_t *dpfx_cmd=NULL;
    soc_sbx_caladan3_tmu_cmd_t *dpld_cmd=NULL;
    uint32 shifted_prefix[TAPS_IPV6_KEY_SIZE_WORDS];
    
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* allocate tmu command to update prefix table */
        rv = tmu_cmd_alloc(unit, &dpfx_cmd); 
        if (SOC_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d failed to allocate TMU dbucket "
                                 "prefix update commands %d:%s !!!\n"), 
                      FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
           return rv;
        }

        size = 256;
        dpfx_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
        dpfx_cmd->cmd.xlwrite.table = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
        if (_TAPS_KEY_IPV4(taps)) {
           dpfx_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_BUCKET;
        } else {
           dpfx_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
        }
        dpfx_cmd->cmd.xlwrite.entry_num = 0;
        dpfx_cmd->cmd.xlwrite.offset = 0;
        dpfx_cmd->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
        dpfx_cmd->cmd.xlwrite.value_size = size;
        for (index=0; index < BITS2WORDS(dpfx_cmd->cmd.xlwrite.value_size); index++) {
           dpfx_cmd->cmd.xlwrite.value_data[index] = 0;
        }       

        rv = taps_work_enqueue(unit, wgroup, TAPS_DBUCKET_WORK, &(dpfx_cmd->wq_list_elem));
        if (SOC_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"), 
                      FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
           tmu_cmd_free(unit, dpfx_cmd);
           return rv;
        }                                                
    }
   
    /* allocate tmu command to update payload table */
    rv = tmu_cmd_alloc(unit, &dpld_cmd); 
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate TMU dbucket "
                              "payload update commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    	return rv;
    }
    
    size = _tmu_dbase[unit]->table_cfg.table_attr[taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE]].entry_size_bits;
    
    dpld_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
    dpld_cmd->cmd.xlwrite.table = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    if (_TAPS_KEY_IPV4(taps)) {
	dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_DATA;
    } else {
	dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
    }
    dpld_cmd->cmd.xlwrite.entry_num = 0;
    dpld_cmd->cmd.xlwrite.offset = 0;
    dpld_cmd->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
    dpld_cmd->cmd.xlwrite.value_size = size;    

    if (_TAPS_KEY_IPV4(taps)) {
    	/* IPV4: copy over the payload */
    	for (index = 0; index < BITS2WORDS(dpld_cmd->cmd.xlwrite.value_size); index++) {
    	    dpld_cmd->cmd.xlwrite.value_data[index] = taps->defpayload[index];
    	}			
    } else {
    	/* IPV6: calculate CPE and copy over payload */
    	/* convert the key/length to cpe format */
    	for (index = 0; index < TAPS_IPV6_KEY_SIZE_WORDS; index++) {
    	    shifted_prefix[index] = 0;
    	}    

    	/* set bit (127-key_length), bits 127-0 is on word1 to word4 */
    	_TAPS_SET_KEY_BIT(shifted_prefix, 127, TAPS_IPV6_KEY_SIZE);
    	
    	/* write entries */
    	for (index = 0; index < BITS2WORDS(dpld_cmd->cmd.xlwrite.value_size); index++) {
    	    if (index < (_TAPS_DBUCKET_IPV6_PREFIX_LENGTH+31)/32) {
        		/* put in the cpe, which is in word1-word4 */
        		dpld_cmd->cmd.xlwrite.value_data[index] = 
        		            shifted_prefix[((_TAPS_DBUCKET_IPV6_PREFIX_LENGTH+31)/32)-index];
    	    } else {
        		/* put in the payload */
        		dpld_cmd->cmd.xlwrite.value_data[index] = 
        		            taps->defpayload[index-(_TAPS_DBUCKET_IPV6_PREFIX_LENGTH+31)/32];
    	    }
    	}
    }
    rv = taps_work_enqueue(unit, wgroup, TAPS_DBUCKET_WORK, &(dpld_cmd->wq_list_elem));
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to enqueue dbucket work item %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    	tmu_cmd_free(unit, dpld_cmd);
    }

    return rv;
}

/*
 *   Function
 *      taps_dbucket_find_prefix
 *   Purpose
 *      Find a prefix from the dram bucket and return the dbucket 
 *      prefix handle if found.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) dbh    : handle of the dbucket object to search
 *      (IN) key    : key buffer of prefix
 *      (IN) len    : key length
 *      (OUT)dph    : handle of the matching dbucket prefix object
 *   Returns
 *       SOC_E_NONE - found
 *       SOC_E_NOT_FOUND - not found
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_find_prefix(int unit, 
                             taps_dbucket_handle_t dbh,
                             uint32 *key, 
                             unsigned int len,
                             taps_dprefix_handle_t *dph)
{
    int rv = SOC_E_NONE;
    trie_node_t *payload;

    if (!dbh || !key || !dph) return SOC_E_PARAM;

    /* search both trie to find prefix, search in 1* trie first */
    rv = trie_search(dbh->trie1, key, len, &payload);
    if (SOC_SUCCESS(rv)) {
	*dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
    } else {
	/* NOT_FOUND is OK, but should report other failures */
	if (rv != SOC_E_NOT_FOUND) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failure in dbuket trie1 search %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	}

	/* search in 0* trie next */
	rv = trie_search(dbh->trie0, key, len, &payload);
	if (SOC_SUCCESS(rv)) {
	    *dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
	} else if (rv != SOC_E_NOT_FOUND) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failure in dbuket trie0 search %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	}
    }

    return rv;
}

/*
 * info passed for the bucket flush callback
 */
typedef struct _taps_flush_dbkt_info_s {
    int unit;
    taps_handle_t taps;
    const taps_wgroup_handle_t *wgroup;
    taps_dbucket_handle_t dbh;
} _taps_flush_dbkt_info_t;

/*
 * update payload for each prefix in the dbucket. Unused entry is not untouched.
 */
static int _taps_dbkt_flush_cb(taps_dprefix_handle_t dph, void *user_data)
{
    int rv = SOC_E_NONE;
    _taps_flush_dbkt_info_t *info = (_taps_flush_dbkt_info_t *)user_data;

    rv = taps_dbucket_enqueue_update_payload_work(info->unit, info->taps, info->wgroup,
						   info->dbh, dph);
    return rv;
}

/*
 *   Function
 *      _taps_flush_dbucket
 *   Purpose
 *      flush all dbucket prefix objects in a dbucket object and enqueue all work items
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle of the dbucket object to flush
 *   Returns
 *       SOC_E_NONE - successfully flushed
 *       SOC_E_* as appropriate otherwise
 */
static int _taps_flush_dbucket(int unit, 
			       const taps_handle_t taps,
			       const taps_wgroup_handle_t *wgroup,
			       taps_dbucket_handle_t dbh)
{
    int rv = SOC_E_NONE;
    _taps_flush_dbkt_info_t info;

    /* write the payload table, use traverse function */
    info.unit = unit;
    info.taps = taps;
    info.wgroup = wgroup;
    info.dbh = dbh;
    rv = taps_dbucket_traverse(unit, taps, wgroup, dbh, (void *)(&info), _taps_dbkt_flush_cb);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failure to flush dbucket payload tables %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* write the cached copy of the dbucket prefix table */
    rv = _taps_dbucket_enqueue_work(unit, taps, wgroup, dbh, NULL, TAPS_DBUCKET_OP_FLUSH_PFX);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failure to flush dbucket prefixes tables %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }
    
    return rv;
}
/*
 *   Function
 *      taps_dbucket_merge
 *   Purpose
 *      Merge two dbucket
 */
int taps_dbucket_merge(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       int pivot_len,
                       taps_dbucket_handle_t dst_dbh,
                       taps_dbucket_handle_t src_dbh,
                       taps_dprefix_handle_t *roll_back_dph)
{
    int rv = SOC_E_NONE;
    taps_dprefix_handle_t ndph = NULL;
    taps_dprefix_handle_t dph = NULL;
    trie_node_t *payload = NULL;
    trie_t *trie = NULL;

    trie = src_dbh->trie0;
    while(1) {
        rv = trie_iter_get_first(trie, &payload);
        if (rv == SOC_E_EMPTY) {
            /* no nodes left */
            if (trie == src_dbh->trie0) {
                trie = src_dbh->trie1;
                continue;
            } else {
                rv = SOC_E_NONE;
                break;
            }
        }

        dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);

        rv = taps_dbucket_insert_prefix(unit, taps, wgroup,
                            dst_dbh, dph->pfx, dph->length,
                            dph->payload, dph->cookie, pivot_len, &ndph);
        if (SOC_FAILURE(rv)){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to insert dprefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        if (roll_back_dph != NULL) {
            if (*roll_back_dph == dph) {
                *roll_back_dph = ndph;
            }
        }
        rv =  taps_dbucket_delete_prefix(unit, taps, wgroup, src_dbh, dph);
        if (SOC_FAILURE(rv)){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to delete dprefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
    }
    return rv;
}


/*
 *   Function
 *      taps_dbucket_split
 *   Purpose
 *      Split a full dbucket into two. return the new dbucket handle
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) bucket_id : bucket id for the new dbucket object
 *      (IN) dbh    : handle of the dbucket object to split
 *      (OUT)pivot  : pivot key buffer for the new dbucket object
 *      (OUT)pivot_len : pivot length
 *      (OUT)bpm    : best prefix match(bpm) mask 
 *      (OUT)ndph   : handle of the new dbucket prefix object due to split
 *   Returns
 *       SOC_E_NONE - successfully split
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_split(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       const unsigned int bucket_id,
                       taps_dbucket_handle_t dbh,
                       uint32 *pivot,
                       uint32 *pivot_len,
                       uint32 *bpm,
                       taps_dbucket_handle_t *ndbh)
{
    int rv = SOC_E_NONE, rv_in = SOC_E_NONE, bsel;
    trie_t     *trie = NULL;
    trie_node_t *split_trie_root = NULL;
    int bucket_depth;
    trie_t split_trie;
    taps_dprefix_handle_t *tmp_dpfx = NULL;
    unsigned int max_split_len, bit_pos;
    trie_node_t *payload=NULL;
    taps_dprefix_handle_t dph;
    int index = 0, dph_idx = 0;
    SHR_BITDCL tmp_bpm[SHR_BITALLOCSIZE(TAPS_IPV6_KEY_SIZE)];

    if (!taps || !wgroup || !dbh || !pivot || !pivot_len || !bpm || !ndbh) {
	return SOC_E_PARAM;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\n#### DBUCKET SPLIT #####\n")));

    /* determine the trie to split */
    *ndbh = NULL;
    bucket_depth = taps->param.dbucket_attr.num_dbucket_pfx;
    if ((dbh->pfx_bmap0[0] == TP_MASK_LO32(bucket_depth)) && 
	((dbh->pfx_bmap0[1] & TP_MASK_HI32(bucket_depth)) == TP_MASK_HI32(bucket_depth))) {
	trie = dbh->trie0;
        bsel = 0;
    } else if ((dbh->pfx_bmap1[0] == TP_MASK_LO32(bucket_depth)) &&
	       ((dbh->pfx_bmap1[1] & TP_MASK_HI32(bucket_depth)) == TP_MASK_HI32(bucket_depth))) {
	trie = dbh->trie1;
        bsel = 1;
    } else {
	rv = SOC_E_PARAM;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d not support spliting a non-full Dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* create a new bucket */
    rv = taps_dbucket_create(unit, taps, wgroup[unit], dbh->domain, bucket_id, ndbh);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d new Dbucket creation Failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	return rv;
    }

    /* trie split */
    max_split_len =  taps->param.key_attr.lookup_length;
    for (bit_pos = 0; bit_pos <= 8; bit_pos++) {
	/* expecting the dbucket could be less than 8 bits long (256 entries)
	 * the max split pivot should be short enough to cover at least
	 * the number of prefixes a dbucket pair could hold.
	 */
	if ((1<<bit_pos) >= taps->param.dbucket_attr.num_dbucket_pfx) {
	    break;
	}
    }
    max_split_len -= bit_pos;
    rv = trie_split(trie, max_split_len, TRUE, pivot, pivot_len, &split_trie_root, bpm, FALSE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to split Dbucket %d trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, (trie==dbh->trie0)?0:1, rv, soc_errmsg(rv)));
	    goto dkt_split_error_handle;
    }

    tmp_dpfx = sal_alloc(sizeof(taps_dprefix_handle_t) * bucket_depth, "pfx for error handle");
    dph_idx = 0;
    /* insert all dph to be moved into the new dbh */
    split_trie.trie = split_trie_root;
    split_trie.v6_key = trie->v6_key;
    while(1) {
        rv = trie_iter_get_first(&split_trie, &payload);
        if (rv == SOC_E_EMPTY) {
            /* no nodes left */
            rv = SOC_E_NONE;
            break;
        } else if (SOC_FAILURE(rv)) {
            /* failure */
            break;
        }
        
        dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
        
        /* remove the prefix from old trie */
        trie_delete(&split_trie, dph->pfx, dph->length, &payload);
      
        /* invalidate dph on old bucket */
        SHR_BITCLR((bsel)?(&(dbh->pfx_bmap1[(dph->index % bucket_depth)>>5])):(&(dbh->pfx_bmap0[(dph->index % bucket_depth)>>5])),  
                   (dph->index % bucket_depth) & 0x1F);

	/* update dbucket cache on old bucket to set cpe = 0 */
	tmp_dpfx[dph_idx] = dph;
	rv = _taps_dbucket_enqueue_work(unit, taps, wgroup,
					dbh, dph, TAPS_DBUCKET_OP_DELETE);
        if (SOC_FAILURE(rv)) {
            /* failed to update dbucket cache of old bucket */
            goto dkt_split_error_handle;
        }       

        /* insert the prefix into trie of new dbucket */
        rv = _taps_dbucket_insert_prefix(unit, taps, wgroup, *ndbh,
                                         &(dph->pfx[0]), dph->length, &(dph->payload[0]), dph->cookie,
                                         *pivot_len,
                                         _TAPS_DBUCKET_INSERT_PREFIX_FLAG_NO_ENQUEUE | 
                                         _TAPS_DBUCKET_INSERT_PREFIX_FLAG_DPH_WITHID,
                                         &dph);
        if (SOC_FAILURE(rv)) {
            /* failed to insert into new trie */
            goto dkt_split_error_handle;
        }
        dph_idx++;
    }

    if (SOC_SUCCESS(rv)) {
	/* flush the new Dbucket to commit to hardware */
	rv = _taps_flush_dbucket(unit, taps, wgroup, *ndbh);
    }

dkt_split_error_handle:
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to move prefixes into new Dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        for (index = 0; index < dph_idx; index++) {
            /* mark entry as used */
            SHR_BITSET((bsel)?(&(dbh->pfx_bmap1[(tmp_dpfx[index]->index % bucket_depth)>>5])):(&(dbh->pfx_bmap0[(tmp_dpfx[index]->index % bucket_depth)>>5])),  
                   (tmp_dpfx[index]->index % bucket_depth) & 0x1F);
			/* Insert back prefix which is in new trie*/	   
            _TAPS_SET_KEY_BIT(tmp_bpm, 0, taps->param.key_attr.lookup_length);
            trie_insert(trie, tmp_dpfx[index]->pfx, &(tmp_bpm[0]), tmp_dpfx[index]->length, &tmp_dpfx[index]->node);
        }
        while(1) {
            rv_in = trie_iter_get_first(&split_trie, &payload);
            if (rv_in == SOC_E_EMPTY) {
                /* no nodes left */
                rv_in = SOC_E_NONE;
                break;
            } 
            assert(SOC_SUCCESS(rv_in));
                
            dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
            /* remove the prefix from old trie */
            trie_delete(&split_trie, dph->pfx, dph->length, &payload);
			/* Insert back prefix which is in split trie */
            _TAPS_SET_KEY_BIT(tmp_bpm, 0, taps->param.key_attr.lookup_length);
            trie_insert(trie, dph->pfx, &(tmp_bpm[0]), dph->length, &dph->node);
        }
    	if (*ndbh) {
              (void)taps_dbucket_destroy(unit, taps, wgroup[unit], *ndbh);
              *ndbh = NULL;
    	}
    }

    sal_free(tmp_dpfx);
    return rv;
}

/*
 *   Function
 *      taps_dbucket_commit
 *   Purpose
 *      Commit commands from dram bucket operation to hardware
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) wgroup : work group handle (for future API batching)
 *   Returns
 *       SOC_E_NONE - successfully commited
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_commit(int unit, taps_wgroup_handle_t wgroup)
{
    if (!wgroup) return SOC_E_PARAM;

    return taps_work_commit(unit, wgroup,
                            work_type, COUNTOF(work_type),
                            _TAPS_BULK_COMMIT);
}

/*
 *
 * Function:
 *     taps_dbucket_free_work_queue
 * Purpose:
 *     free all outstanding work items
 */
int taps_dbucket_free_work_queue(int unit, 
                                 taps_wgroup_handle_t wgroup)
{
    if (!wgroup) return SOC_E_PARAM;

    return taps_free_work_queue(unit, wgroup,
                                work_type, 
                                COUNTOF(work_type));
}


/*
 *   Function
 *      taps_dbucket_stat_get
 *   Purpose
 *      Get the vacancy status of a dbucket
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) dbh    : handle of the dbucket object to get status
 *      (OUT)stat   : vacancy status
 *                    see "taps_bucket_stat_e_t" definition
 *   Returns
 *       SOC_E_NONE - successfully commited
 *       SOC_E_* as appropriate otherwise
 */
int taps_dbucket_stat_get(int unit,
			  const taps_handle_t taps,
			  taps_dbucket_handle_t dbh,
			  taps_bucket_stat_e_t *stat)
{
    int rv = SOC_E_NONE;
    int bucket_depth;
    int tp_mask_lo32, tp_mask_hi32;

    if (!taps || !dbh || !stat) return SOC_E_PARAM;

    /* check both sub-buckets to return status */
    bucket_depth = taps->param.dbucket_attr.num_dbucket_pfx;
    tp_mask_lo32 = TP_MASK_LO32(bucket_depth);
    tp_mask_hi32 = TP_MASK_HI32(bucket_depth);
    
    if (((dbh->pfx_bmap0[0] == tp_mask_lo32) && 
	 ((dbh->pfx_bmap0[1] & tp_mask_hi32) == tp_mask_hi32)) ||
	((dbh->pfx_bmap1[0] == tp_mask_lo32) &&
	 ((dbh->pfx_bmap1[1] & tp_mask_hi32) == tp_mask_hi32))) {
	/* if either 0* or 1* bucket is full, return _TAPS_BKT_FULL */
	*stat = _TAPS_BKT_FULL;
    } else if (((dbh->pfx_bmap0[0] & tp_mask_lo32) == 0) &&
	       ((dbh->pfx_bmap0[1] & tp_mask_hi32) == 0) &&
	       ((dbh->pfx_bmap1[0] & tp_mask_lo32) == 0) &&
	       ((dbh->pfx_bmap1[1] & tp_mask_hi32) == 0)) {
	/* if both 0* and 1* bucket are empty, return _TAPS_BKT_EMPTY */
	*stat = _TAPS_BKT_EMPTY;
    } else if (((dbh->pfx_bmap0[0] | tp_mask_lo32) != tp_mask_lo32) ||
	       ((dbh->pfx_bmap0[1] | tp_mask_hi32) != tp_mask_hi32) ||
	       ((dbh->pfx_bmap1[0] | tp_mask_lo32) != tp_mask_lo32) ||
	       ((dbh->pfx_bmap1[1] | tp_mask_hi32) != tp_mask_hi32)) {
	/* if any bits not in bucket got set, return _TAPS_BKT_ERROR */
	*stat = _TAPS_BKT_ERROR;
    } else {
	/* otherwise returns _TAPS_BKT_VACANCY */
	*stat = _TAPS_BKT_VACANCY;
    }

    return rv;
}

/*
 * info passed for dbucket traverse
 */
typedef struct _taps_dbkt_cb_info_s {
    void         *user_data;
    dbucket_traverse_cb_f cb;
    taps_dprefix_handle_t *dpfxs;
    int num_pfxs;   
} _taps_dbkt_cb_info_t;

/*
 * dbucket traverse callback function
 */
static int _taps_dbucket_trie_traverse_cb(trie_node_t *payload, void *info)
{
    taps_dprefix_handle_t dph = NULL;

    dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, payload, node);
    /* only callback on the payload nodes */
    if (payload && (payload->type == PAYLOAD)) {
	if (((_taps_dbkt_cb_info_t *)info)->dpfxs == NULL) {
	    /* call back directly */
	    if (((_taps_dbkt_cb_info_t *)info)->cb) {
		((_taps_dbkt_cb_info_t *)info)->cb(dph, ((_taps_dbkt_cb_info_t *)info)->user_data);
	    }
	} else {
	    /* accumulate dphs */
	    ((_taps_dbkt_cb_info_t *)info)->dpfxs[(((_taps_dbkt_cb_info_t *)info)->num_pfxs)++] = dph;
	}
    }
    return SOC_E_NONE;
}

/*
 *   Function
 *      taps_dbucket_traverse
 *   Purpose
 *      Traverse the dram bucket & invoke call back on all dbucket prefix objects
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) dbh    : handle of the dbucket object to traverse
 *      (IN) user_data: user data
 *      (IN) cb     : user provided callback function pointer
 *   Returns
 *       SOC_E_NONE - successfully traversed and no errors in callback functions
 *       SOC_E_* as appropriate otherwise
 *   Note:
 *       dbucket stop traversing on any errors
 *   Future:
 *       change the implementation to traverse collect dph pointer
 *       then use for loop to call back on every dph. This would
 *       be safer when callback function is changing trie structure
 */
int taps_dbucket_traverse(int unit, 
                          const taps_handle_t taps,
                          const taps_wgroup_handle_t *wgroup,
                          taps_dbucket_handle_t dbh, 
                          void *user_data,
                          dbucket_traverse_cb_f cb)
{
    int rv = SOC_E_NONE;
    _taps_dbkt_cb_info_t info;

    info.user_data = user_data;
    info.cb = cb;
    info.num_pfxs = 0;
    info.dpfxs = NULL;

    /* traverse on both tri0 and tri1 */
    rv = trie_traverse(dbh->trie0, _taps_dbucket_trie_traverse_cb,
                       &info, _TRIE_PREORDER_TRAVERSE);

    if (SOC_SUCCESS(rv)) {
	rv = trie_traverse(dbh->trie1, _taps_dbucket_trie_traverse_cb,
                           &info, _TRIE_PREORDER_TRAVERSE);
    }

    return rv;
}

int taps_dbucket_destroy_traverse(int unit, 
                                  const taps_handle_t taps,
                                  const taps_wgroup_handle_t *wgroup,
                                  taps_dbucket_handle_t dbh, 
                                  void *user_data,
                                  dbucket_traverse_cb_f cb)
{
    int rv = SOC_E_NONE, index;
    _taps_dbkt_cb_info_t info;

    info.user_data = user_data;
    info.cb = cb;
    info.num_pfxs = 0;
    info.dpfxs = sal_alloc(sizeof(taps_dprefix_handle_t) * (taps->param.dbucket_attr.num_dbucket_pfx*2+1),
			   "taps-dbucket-traverse");;;

    /* traverse on both tri0 and tri1 */
    rv = trie_traverse(dbh->trie0, _taps_dbucket_trie_traverse_cb,
                       &info, _TRIE_POSTORDER_TRAVERSE);

    if (SOC_SUCCESS(rv)) {
	rv = trie_traverse(dbh->trie1, _taps_dbucket_trie_traverse_cb,
                           &info, _TRIE_POSTORDER_TRAVERSE);
    }

    if (SOC_SUCCESS(rv)) {
	if (info.num_pfxs <= taps->param.dbucket_attr.num_dbucket_pfx*2) {
	    /* call back on all dph accumulated */
	    for (index = 0; index < info.num_pfxs; index++) {
		rv = cb(info.dpfxs[index], user_data);
		if (SOC_FAILURE(rv)) {
		    break;
		}
	    }
	} else {
	    /* we should never get more than what dbucket can hold */
	    rv = SOC_E_INTERNAL;
	}
    }

    if (info.dpfxs) {
	sal_free(info.dpfxs);
    }

    return rv;
}

/*
 *   Function
 *      taps_relocate_dbucket
 *   Purpose
 *      Relocate dbucket object to another tcam domain and enqueue all work items.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) reloc_domain_id : new domain id the dbucket object should be in
 *      (IN) dbh    : handle of the dbucket object to flush
 *   Returns
 *       SOC_E_NONE - successfully relocated
 *       SOC_E_* as appropriate otherwise
 */
int taps_relocate_dbucket(int unit, 
                          const taps_handle_t taps,
                          const taps_wgroup_handle_t *wgroup,
                          const unsigned int reloc_domain_id,
                          taps_dbucket_handle_t dbh)
{
    int rv = SOC_E_NONE;
    int old_domain = dbh->domain;

    /* for now, assuming the reloc_domain_id is already
     * checked by the caller, so no need to do boundary check again
     */
    if (old_domain == reloc_domain_id) {
	return SOC_E_NONE;
    }

    /* modify the domain id, bucket id should be same */
    dbh->domain = reloc_domain_id;

    rv = _taps_flush_dbucket(unit, taps, wgroup, dbh);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failure to relocate dbucket from "
                              "domain %d to domain %d. error %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, old_domain, reloc_domain_id, rv, soc_errmsg(rv)));
    }

    return rv;
}

/*
 *   Function
 *      taps_dbucket_next_prefix_handle_get
 *   Purpose
 *      Get the next prefix of taps 
 *   Parameters
 *      (IN) taps              : Valid taps
 *      (IN) start_index    : Beginning search index
 *      (IN) dbh               : Dbucket
 *      (OUT) dph            : DRAM prefix handle
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
int taps_dbucket_next_prefix_handle_get(taps_handle_t taps,
                                        int start_index,
                                        taps_dbucket_handle_t dbh,
                                        taps_dprefix_handle_t *dph)
{
    int rv = SOC_E_NONE;
    trie_node_t *next_node = NULL;
    taps_dbucket_handle_t cur_dbh;

    cur_dbh = dbh;
    if (start_index < taps->param.dbucket_attr.num_dbucket_pfx) {
        /* Searching in trie0 */
        rv = taps_bucket_next_node_get(cur_dbh->trie0, DRAM_PREFIX_TRIE,
				       &cur_dbh->pfx_bmap0[0], start_index, 
				       taps->param.dbucket_attr.num_dbucket_pfx, &next_node);
        if (rv == SOC_E_NOT_FOUND) {
            /* Searching in trie1 */
            start_index = 0;
            rv = taps_bucket_next_node_get(cur_dbh->trie1, DRAM_PREFIX_TRIE,
					   &cur_dbh->pfx_bmap1[0], start_index, 
					   taps->param.dbucket_attr.num_dbucket_pfx, &next_node);
        }
    } else {            
        start_index -= taps->param.dbucket_attr.num_dbucket_pfx;
        /* Searching in trie1 */
        rv = taps_bucket_next_node_get(cur_dbh->trie1, DRAM_PREFIX_TRIE,
				       &cur_dbh->pfx_bmap1[0], start_index, 
				       taps->param.dbucket_attr.num_dbucket_pfx, &next_node);
    }
       
    if(SOC_SUCCESS(rv)) {
        *dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, next_node, node);
        if ((*dph)->length == 0) {
            /* Wild entry */
            rv = taps_dbucket_next_prefix_handle_get(taps, (*dph)->index + 1, dbh, dph);
        }
    }
    
    return rv;
}

/*
 *   Function
 *      taps_dbucket_first_prefix_handle_get
 *   Purpose
 *      Get the next prefix of taps 
 *   Parameters
 *      (IN) taps              : Valid taps
 *      (IN) dbh               : Dbucket
 *      (OUT) dph            : DRAM prefix handle
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
int taps_dbucket_first_prefix_handle_get(int unit, taps_handle_t taps, 
                                    taps_dbucket_handle_t dbh, taps_dprefix_handle_t *dph)
{
    int rv = SOC_E_NONE;
    int start_index = 0;
    
    rv = taps_dbucket_next_prefix_handle_get(taps, start_index, dbh, dph);
    return rv;
}

/*
 *   Function
 *      taps_dbucket_next_prefix_get
 *   Purpose
 *      Get the next prefix of taps 
 *   Parameters
 *      (IN) taps              : Valid taps
 *      (IN) start_index    : Beginning search index
 *      (IN) dbh               : Dbucket
 *      (OUT) key             : First key in this taps
 *      (OUT) key_length  : Length of first key
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
int taps_dbucket_next_prefix_get(taps_handle_t taps,
                                        int start_index,
                                        taps_dbucket_handle_t dbh,
                                        uint32 *key,
                                        uint32 *key_length)
{
    int rv = SOC_E_NONE;
    taps_dprefix_handle_t dph;
    rv = taps_dbucket_next_prefix_handle_get(taps, start_index, dbh, &dph);
    if (SOC_SUCCESS(rv)) {
        *key_length = dph->length;
        sal_memcpy(key, dph->pfx, 
                BITS2WORDS(taps->param.key_attr.lookup_length) * sizeof(uint32));
    } else if (rv == SOC_E_NOT_FOUND){
        /* No more key, traverse complete. Just return SOC_E_NOT_FOUND*/
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Find to get the next key in dbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), rv, soc_errmsg(rv)));
    }
    return rv;
}

/*
 *   Function
 *      taps_dbucket_first_prefix_get
 *   Purpose
 *      Get the next prefix of taps 
 *   Parameters
 *      (IN) unit              : unit number of the device
 *      (IN) taps              : Valid taps
 *      (IN) dbh              : Dbucket
 *      (OUT) key            : First key in this taps
 *      (OUT) key_length  : Length of first key
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_NOT_FOUND - Traverse completed.
 *       SOC_E_* - Error return 
 */
int taps_dbucket_first_prefix_get(int unit, taps_handle_t taps, 
                                    taps_dbucket_handle_t dbh, 
                                    uint32 *key,
                                    uint32 *key_length)
{
    int rv = SOC_E_NONE;
    taps_dprefix_handle_t dph;
    rv = taps_dbucket_first_prefix_handle_get(unit, taps, dbh, &dph);
    if (SOC_SUCCESS(rv)) {
        *key_length = dph->length;
        sal_memcpy(key, dph->pfx, 
            BITS2WORDS(taps->param.key_attr.lookup_length) * sizeof(uint32));
    } else if (rv == SOC_E_NOT_FOUND) {
        /* No more key, traverse complete. Just return SOC_E_NOT_FOUND*/
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to get the first dph %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}

/*
* Fuction : 
*        taps_dbucket_find_bpm
* Purpose:
*        Get lpm entry of this route using bpm mask of dbucket trie, return bpm_length
*/
int taps_dbucket_find_bpm(int unit, const taps_handle_t taps, 
                                  taps_dbucket_handle_t dbh,
                                  uint32 *key, uint32 length, 
                                  uint32 pivot_len,
                                  uint32 *bpm_length)
{
    int rv = SOC_E_NONE;
    int msb;
    
    if (!taps || !dbh || !key || !bpm_length) {
        return SOC_E_PARAM;
    }
    if (length < pivot_len) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d dbucket prefix length is %d"
                              "shorter than dbucket pivot length %d %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, length, pivot_len, rv, soc_errmsg(rv)));
        return SOC_E_PARAM;

    } else if (length == pivot_len) {
        msb = 0;
    } else {
        msb = _TAPS_GET_KEY_BIT(key, (length - pivot_len - 1), 
                        taps->param.key_attr.lookup_length)?1:0;
    }

    if (msb) {
        rv = trie_find_prefix_bpm(dbh->trie1, key, length, bpm_length);
    } else {
        rv = trie_find_prefix_bpm(dbh->trie0, key, length, bpm_length);
    }
   
    return rv;
}

/*
 * info passed for the dbucket dump callback
 */
typedef struct _taps_dump_dbkt_info_s {
    int unit;
    taps_handle_t taps;
    taps_dbucket_handle_t dbh;
    uint32 flags;
} _taps_dump_dbkt_info_t;

/*
 * Call back function for sbucket dump
 */
static int _taps_dbkt_dump_cb(trie_node_t *node, void *user_data)
{
    int rv = SOC_E_NONE;

    if (node && user_data) {
        if (node->type == PAYLOAD) {
            _taps_dump_dbkt_info_t *info = (_taps_dump_dbkt_info_t*)user_data;
            taps_dprefix_handle_t dph = TRIE_ELEMENT_GET(taps_dprefix_handle_t, node, node);
            /* dump trie pivot */
            taps_dbucket_prefix_dump(info->unit, info->taps, info->dbh, dph, info->flags);
        } else {
            LOG_CLI((BSL_META("+ dbkt trie_node skip_len %d skip_addr 0x%x type INTERNAL count %d bpm 0x%08x\n"),
                     node->skip_len, node->skip_addr, node->count, node->bpm));
        }
    }

    return rv;
}

/*
 * Dump a Dbucket software and hardware state
 */
int taps_dbucket_dump(int unit, const taps_handle_t taps,
		      const taps_dbucket_handle_t dbh,
                      uint32 flags)
{
    int rv = SOC_E_NONE;
    _taps_dump_dbkt_info_t info;
    int cache_block_size, cache_index, index;

    if (!taps || !dbh) return SOC_E_PARAM;
    if (!(TAPS_DUMP_DRAM_FLAGS(flags))) return SOC_E_NONE;

    if (flags & TAPS_DUMP_DRAM_SW_BKT) {
        /* dump bucket info */
        LOG_CLI((BSL_META_U(unit,
                            "\n@@@@@ DBucket dump: %d %p dump @@@@@\n"), dbh->domain, (void *)dbh));
        if (flags & TAPS_DUMP_DRAM_VERB) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: Dumping unit %d Taps handle 0x%x DBH handle 0x%x \n"),
                     FUNCTION_NAME(), unit, (uint32)taps, (uint32)dbh));
            LOG_CLI((BSL_META_U(unit,
                                "%s: trie0 handle 0x%x trie1 handle 0x%x domain %d bucket %d \n"),
                     FUNCTION_NAME(), (uint32)dbh->trie0, (uint32)dbh->trie1,
                     dbh->domain, dbh->bucket));
        }
        LOG_CLI((BSL_META_U(unit,
                            "%s: prefix bmap0 0x%x%08x prefix bmap1 0x%x%08x \n"),
                 FUNCTION_NAME(), dbh->pfx_bmap0[1], dbh->pfx_bmap0[0],
                 dbh->pfx_bmap1[1], dbh->pfx_bmap1[0]));
        /*LOG_CLI((BSL_META_U(unit,
                              "Total number of Dram prefix: %d (0):%d (1):%d \n"),
                   dbh->trie0->trie->count+dbh->trie1->trie->count,
                   dbh->trie0->trie->count, dbh->trie1->trie->count));*/

        if (flags & TAPS_DUMP_DRAM_VERB) {
            /* dump bucket cache */
            if (_TAPS_KEY_IPV4(taps)) {
                cache_block_size = _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX;
            } else {
                cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;
            }
            for (cache_index = 0;
                 cache_index <_TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(taps->param.dbucket_attr.num_dbucket_pfx, cache_block_size)*2;
                 cache_index += 4) {
                LOG_CLI((BSL_META_U(unit,
                                    "%s: dbucket cache 0x%x == 0x%08x 0x%08x 0x%08x 0x%08x ==\n"),
                         FUNCTION_NAME(), cache_index, dbh->cache[cache_index], dbh->cache[cache_index+1],
                         dbh->cache[cache_index+2], dbh->cache[cache_index+3]));
            }
        }
    }

    /* dump all prefixes */
    info.unit = unit;
    info.taps = taps;
    info.dbh = dbh;
    info.flags = flags;

    trie_dump(dbh->trie0, _taps_dbkt_dump_cb, &info);
    trie_dump(dbh->trie1, _taps_dbkt_dump_cb, &info);

    /* blindly dump all entry without considering if it's valid
     * useful for debug
     */
    if (flags & TAPS_DUMP_DRAM_VERB) {
	LOG_CLI((BSL_META_U(unit,
                            "%s: DUMPPING all dbucket 0x%x domain %d bucket %d entries ==\n"),
                 FUNCTION_NAME(), (uint32)dbh, dbh->domain, dbh->bucket));
	for (index = 0; index < (taps->param.dbucket_attr.num_dbucket_pfx * 2); index++) {
	    taps_dbucket_entry_dump(unit, taps, dbh->domain, dbh->bucket, index, flags, FALSE);
	}
    }
    return rv;
}

/*
 * Dump a Dbucket DPH software and hardware state
 */
int taps_dbucket_prefix_dump(int unit, const taps_handle_t taps,
			     const taps_dbucket_handle_t dbh,
			     const taps_dprefix_handle_t dph,
                             uint32 flags)
{
    int rv = SOC_E_NONE;

    if (!taps || !dbh || !dph) return SOC_E_PARAM;

    if (flags & TAPS_DUMP_DRAM_SW_PFX) {
        if (flags & TAPS_DUMP_DRAM_VERB) {
            /* dump software info */
            LOG_CLI((BSL_META_U(unit,
                                "%s: Dumping unit %d Taps handle 0x%x DBH 0x%x DPH 0x%x \n"),
                     FUNCTION_NAME(), unit, (uint32)taps, (uint32)dbh, (uint32)dph));
            LOG_CLI((BSL_META_U(unit,
                                "%s: trie_node skip_len %d skip_addr 0x%x type %s count %d bpm 0x%08x\n"),
                     FUNCTION_NAME(), dph->node.skip_len, dph->node.skip_addr,
                     (dph->node.type==PAYLOAD)?"PAYLOAD":"INTERNAL", dph->node.count, dph->node.bpm));
            LOG_CLI((BSL_META_U(unit,
                                "%s: prefix index %d is bpm %s\n"),
                     FUNCTION_NAME(), dph->index, (dph->bpm)?"TRUE":"FALSE"));
        }

        if (taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
	    LOG_CLI((BSL_META_U(unit,
                                "%s: key_length %d key_data 0x%04x %08x\n"),
                     FUNCTION_NAME(), dph->length, dph->pfx[0], dph->pfx[1]));
        } else {
	    LOG_CLI((BSL_META_U(unit,
                                "%s: key_length %d key_data 0x%04x %08x %08x %08x %08x\n"),
                     FUNCTION_NAME(), dph->length, dph->pfx[0], dph->pfx[1],
                     dph->pfx[2], dph->pfx[3], dph->pfx[4]));
        }    
        LOG_CLI((BSL_META_U(unit,
                            "%s: prefix payload 0x%08x %08x %08x %08x cookie 0x%p\n"),
                 FUNCTION_NAME(), dph->payload[0], dph->payload[1],
                 dph->payload[2], dph->payload[3], dph->cookie));
    }    

    /* dump associated hw entry */
    rv = taps_dbucket_entry_dump(unit, taps, dbh->domain, dbh->bucket, dph->index, flags, FALSE);
    return rv;
}

/*
 * Dump a Dbucket entry
 */
int taps_dbucket_entry_dump(int unit, const taps_handle_t taps,
			    int sbucket, int dbucket, int entry,
                uint32 flags, int is_global_index)
{
    int rv = SOC_E_NONE;
    int table_id, size, bucket_size, entry_num, cache_block_size;
    soc_sbx_caladan3_tmu_cmd_t *dpfx_cmd=NULL;
    soc_sbx_caladan3_tmu_cmd_t *dpld_cmd=NULL;
    soc_mem_t mem;
    soc_field_t field;
    uint32 *buff, *p_buff;
    soc_field_t ipv4_fields[_TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX]=
	{PREFIX0f, PREFIX1f, PREFIX2f, PREFIX3f, PREFIX4f, PREFIX5f, PREFIX6f};
    soc_field_t ipv6_fields[_TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX]=
	{PSIG0f, PSIG1f, PSIG2f, PSIG3f, PSIG4f};
    uint32    mem_val[SOC_MAX_MEM_WORDS];

    if (!(flags & TAPS_DUMP_DRAM_HW_PFX)) return SOC_E_NONE;

    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;

    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    if (size < _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits) {
	size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    }
    p_buff = sal_alloc(BITS2BYTES(size), "temp buffer");
    if (p_buff == NULL) {
	return SOC_E_MEMORY;
    }

    buff=p_buff;

    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
    /* read prefix */
    rv = tmu_cmd_alloc(unit, &dpfx_cmd); 
    if (SOC_FAILURE(rv) || (dpfx_cmd == NULL)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate TMU dbucket prefix read commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	if (p_buff) sal_free(p_buff);
	return rv;
    }
    
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
	if (is_global_index) {
		entry_num = entry/bucket_size;
	} else {
		entry_num = ((sbucket * taps->param.sbucket_attr.max_pivot_number) + dbucket) * 2 + entry/bucket_size;		
	}
    
    dpfx_cmd->opcode = SOC_SBX_TMU_CMD_XL_READ;
    dpfx_cmd->cmd.xlread.table = table_id;
    if (_TAPS_KEY_IPV4(taps)) {
	dpfx_cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_BUCKET;
    } else{
	dpfx_cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_BUCKET;
    }
    dpfx_cmd->cmd.xlread.entry_num = entry_num;
    dpfx_cmd->cmd.xlread.kv_pairs = 0;

    sal_memset(p_buff, 0, BITS2BYTES(size));

    TMU_LOCK(unit);
    
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                       dpfx_cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
	rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
					   dpfx_cmd, p_buff, BITS2WORDS(size));
	

   TMU_UNLOCK(unit);	    

	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Invalid response !!!\n"), 
                       FUNCTION_NAME(), unit));
	    if (p_buff) sal_free(p_buff);
	 
	    return rv;
	}
    
    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to post TAPS dbucket prefix read command !!!\n"), 
                   FUNCTION_NAME(), unit));
	if (p_buff) sal_free(p_buff);
	TMU_UNLOCK(unit);	    
	return rv;
    }
    
    if (_TAPS_KEY_IPV4(taps)) {
	/* IPV4 */
	cache_block_size = _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX;

	if (((entry%bucket_size) >= (bucket_size/cache_block_size)*cache_block_size) &&
	    ((bucket_size%cache_block_size) <= (cache_block_size/2))) {
	    mem = TMB_LPM_DBUCKET_IPV4_128m;
	} else {
	    mem = TMB_LPM_DBUCKET_IPV4_256m;
	}
	field = ipv4_fields[((entry%bucket_size)%cache_block_size)];

	buff += _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX(entry%bucket_size, cache_block_size);
	
	LOG_CLI((BSL_META_U(unit,
                            "%s: entry cpe     0x%08x\n"),
                 FUNCTION_NAME(), soc_mem_field32_get(unit, mem, buff, field)));
	
    } else {
	/* IPV6 */
	cache_block_size = _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX;

	if (((entry%bucket_size) >= (bucket_size/cache_block_size)*cache_block_size) &&
	    ((bucket_size%cache_block_size) <= (cache_block_size/2))) {
	    mem = TMB_LPM_DBUCKET_IPV6_128m;
	} else {
	    mem = TMB_LPM_DBUCKET_IPV6_256m;
	}
	field = ipv6_fields[((entry%bucket_size)%cache_block_size)];

	buff += _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX(entry%bucket_size, cache_block_size);

	soc_mem_field_get(unit, mem, buff, field, mem_val);
	LOG_CLI((BSL_META_U(unit,
                            "%s: entry psig    0x%08x %08x\n"),
                 FUNCTION_NAME(), mem_val[0], mem_val[1]));
    }

    tmu_cmd_free(unit, dpfx_cmd);
    }
    /* read payload */
    rv = tmu_cmd_alloc(unit, &dpld_cmd); 
    if (SOC_FAILURE(rv) || (dpld_cmd == NULL)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate TMU dbucket payload read commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	if (p_buff) sal_free(p_buff);
	return rv;
    }
    
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
	if (is_global_index) {
		entry_num = entry;
	} else {
    	entry_num = ((sbucket * taps->param.sbucket_attr.max_pivot_number) + dbucket) * bucket_size * 2 + entry;
	}
    dpld_cmd->opcode = SOC_SBX_TMU_CMD_XL_READ;
    dpld_cmd->cmd.xlread.table = table_id;
    if (_TAPS_KEY_IPV4(taps)) {
	dpld_cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_DATA;
    } else {
	dpld_cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
    }
    dpld_cmd->cmd.xlread.entry_num = entry_num;
    dpld_cmd->cmd.xlread.kv_pairs = 0;

    sal_memset(p_buff, 0, BITS2BYTES(size));

    TMU_LOCK(unit);
    
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                       dpld_cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
	rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
					   dpld_cmd, p_buff, BITS2WORDS(size));
	TMU_UNLOCK(unit);

	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Invalid response !!!\n"), 
                       FUNCTION_NAME(), unit));
	    if (p_buff) sal_free(p_buff);
	    return rv;
	}
    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to post TAPS dbucket payload read command !!!\n"), 
                   FUNCTION_NAME(), unit));
	if (p_buff) sal_free(p_buff);
	TMU_UNLOCK(unit);
	return rv;
    }
    
    if (_TAPS_KEY_IPV4(taps)) {
	/* dump ipv4 payload */
	LOG_CLI((BSL_META_U(unit,
                            "%s: entry payload 0x%08x %08x %08x %08x\n"),
                 FUNCTION_NAME(), p_buff[0], p_buff[1], p_buff[2], p_buff[3]));
    } else {
	/* dump ipv6 key/payload */
	LOG_CLI((BSL_META_U(unit,
                            "%s: entry key     0x%08x %08x %08x %08x\n"),
                 FUNCTION_NAME(), p_buff[0], p_buff[1], p_buff[2], p_buff[3]));
	LOG_CLI((BSL_META_U(unit,
                            "%s: entry payload 0x%08x %08x %08x %08x\n"),
                 FUNCTION_NAME(), p_buff[4], p_buff[5], p_buff[6], p_buff[7]));
    }

    tmu_cmd_free(unit, dpld_cmd);

    if (p_buff) sal_free(p_buff);
    return rv;
}

/*
 * Dump a entry in Dbucket payload table 
 */
int taps_dbucket_payload_entry_dump(int unit, const taps_handle_t taps,
			    int sbucket, int entry, uint32 flags)
{
    int rv = SOC_E_NONE;
    int table_id, size, entry_num;
    soc_sbx_caladan3_tmu_cmd_t *dpld_cmd=NULL;
    uint32 *p_buff = NULL;

    if (!(flags & TAPS_DUMP_DRAM_HW_BKT)) return SOC_E_NONE;

    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
	size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    p_buff = sal_alloc(BITS2BYTES(size), "temp buffer");
    if (p_buff == NULL) {
	    return SOC_E_MEMORY;
    }

    /* read payload */
    rv = tmu_cmd_alloc(unit, &dpld_cmd); 
    if (SOC_FAILURE(rv) || (dpld_cmd == NULL)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate TMU dbucket payload read commands %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    	if (p_buff) sal_free(p_buff);
    	return rv;
    }
    
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    entry_num = sbucket * (1 << (taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_ + entry;

    dpld_cmd->opcode = SOC_SBX_TMU_CMD_XL_READ;
    dpld_cmd->cmd.xlread.table = table_id;
    if (_TAPS_KEY_IPV4(taps)) {
	    dpld_cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_DATA;
    } else {
	    dpld_cmd->cmd.xlread.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
    }
    dpld_cmd->cmd.xlread.entry_num = entry_num;
    dpld_cmd->cmd.xlread.kv_pairs = 0;

    sal_memset(p_buff, 0, BITS2BYTES(size));

    TMU_LOCK(unit);
    
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                       dpld_cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
    	rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
    					   dpld_cmd, p_buff, BITS2WORDS(size));
    	TMU_UNLOCK(unit);

    	if (SOC_FAILURE(rv)) {
    	    LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Invalid response !!!\n"), 
                           FUNCTION_NAME(), unit));
    	    if (p_buff) sal_free(p_buff);
    	    return rv;
    	}
    } else {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to post TAPS dbucket payload read command !!!\n"), 
                       FUNCTION_NAME(), unit));
    	if (p_buff) sal_free(p_buff);
    	TMU_UNLOCK(unit);
    	return rv;
    }
    
    if (_TAPS_KEY_IPV4(taps)) {
    	/* dump ipv4 payload */
    	LOG_CLI((BSL_META_U(unit,
                                "%s: entry payload 0x%08x %08x %08x %08x\n"),
                     FUNCTION_NAME(), p_buff[0], p_buff[1], p_buff[2], p_buff[3]));
    } else {
    	/* dump ipv6 key/payload */
    	LOG_CLI((BSL_META_U(unit,
                                "%s: entry key     0x%08x %08x %08x %08x\n"),
                     FUNCTION_NAME(), p_buff[0], p_buff[1], p_buff[2], p_buff[3]));
    	LOG_CLI((BSL_META_U(unit,
                                "%s: entry payload 0x%08x %08x %08x %08x\n"),
                     FUNCTION_NAME(), p_buff[4], p_buff[5], p_buff[6], p_buff[7]));
    }

    tmu_cmd_free(unit, dpld_cmd);

    if (p_buff) sal_free(p_buff);
    return rv;
}

int taps_dbucket_entry_range_dump(int unit, const taps_handle_t taps,
				  int sbucket, int dbucket, int start_entry, int end_entry,
				  uint32 flags, int is_global_index)
{
    int entry, rv = SOC_E_NONE;
    for (entry = start_entry; entry <= end_entry; entry++) {
	rv = taps_dbucket_entry_dump(unit, taps, sbucket, dbucket, entry, flags, is_global_index);
	if (SOC_FAILURE(rv)) {
	    break;
	}
    }
    return rv;
}

#if 0
/*
 * Debug function only: init the dbucket entry payload to a special patter
 * that includes the global index of the dbucket entry
 */
static int taps_dbucket_entry_debug_init(int unit, const taps_handle_t taps,
					 int sbucket, int dbucket, int entry)
{
    int rv = SOC_E_NONE;
    int table_id, size, bucket_size, entry_num;
    soc_sbx_caladan3_tmu_cmd_t *dpld_cmd=NULL;
    uint32 *buff;

    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;

    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PREFIX_TABLE];
    size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    if (size < _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits) {
	size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    }
    buff = sal_alloc(BITS2BYTES(size), "temp buffer");
    if (buff == NULL) {
	return SOC_E_MEMORY;
    }

    /* write payload */
    rv = tmu_cmd_alloc(unit, &dpld_cmd); 
    if (SOC_FAILURE(rv) || (dpld_cmd == NULL)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate TMU dbucket payload read commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	if (buff) sal_free(buff);
	return rv;
    }
    
    bucket_size = taps->param.dbucket_attr.num_dbucket_pfx;
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    entry_num = ((sbucket * taps->param.sbucket_attr.max_pivot_number) + dbucket) * bucket_size * 2 + entry;
    sal_memset(buff, 0, BITS2BYTES(size));

    dpld_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
    dpld_cmd->cmd.xlwrite.table = table_id;
    if (_TAPS_KEY_IPV4(taps)) {
	dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_DATA;
    } else {
	dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
    }
    dpld_cmd->cmd.xlwrite.entry_num = entry_num;
    dpld_cmd->cmd.xlwrite.offset = 0;
    dpld_cmd->cmd.xlwrite.size = (size+SOC_SBX_TMU_CMD_WORD_SIZE-1)/SOC_SBX_TMU_CMD_WORD_SIZE;
    dpld_cmd->cmd.xlwrite.value_size = size;

    TMU_LOCK(unit);
    
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                       dpld_cmd, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
	rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
					   dpld_cmd, buff, BITS2WORDS(size));
	TMU_UNLOCK(unit);

	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Invalid response !!!\n"), 
                       FUNCTION_NAME(), unit));
	    if (buff) sal_free(buff);
	    return rv;
	}

    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to post TAPS dbucket payload read command !!!\n"), 
                   FUNCTION_NAME(), unit));
	if (buff) sal_free(buff);
	TMU_UNLOCK(unit);
	return rv;
    }
    
    tmu_cmd_free(unit, dpld_cmd);

    if (buff) sal_free(buff);
    return rv;    
}
#endif 

#endif /* BCM_CALADAN3_SUPPORT */
