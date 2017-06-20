/*
 * $Id: tcam.c,v 1.46.14.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    tcam.c
 * Purpose: Caladan3 TAPS internal tcam driver
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/bitop.h>
#include <soc/sbx/caladan3/hpcm.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/taps/trie.h>
#include <soc/sbx/caladan3/tmu/taps/trie_v6.h>
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/tmu/taps/tcam.h>
#include <soc/sbx/caladan3/tmu/taps/sbucket.h>
#include <soc/sbx/caladan3/tmu/taps/dbucket.h>
#include <soc/sbx/caladan3/tmu/taps/taps_util.h>

/* key_size and key_size-1 don't have subsegment since 2 LSBs are ignored by TCAM hardware */
#define TAPS_TCAM_NUM_SSEGS(key_size) \
    ((key_size) - 1)

#define TAPS_TCAM_MAX_SSEG(key_size) \
    ((key_size) - 2)

#define TAPS_TCAM_SSEG_FULL(handle, id) \
    (((handle)->ssegs[(id)].use_count) >= ((handle)->ssegs[(id)].size))

#define TAPS_IS_IPV4(handle) \
    ((handle)->key_size <= TAPS_IPV4_KEY_SIZE)

#define TAPS_IS_IPV6(handle) \
    (((handle)->key_size <= TAPS_IPV6_KEY_SIZE) && ((handle)->key_size > TAPS_IPV4_KEY_SIZE))

static int _debug = FALSE;
#define _TAPS_TCAM_DEBUG

/*
 * Following object manages a TCAM subsegment
 *   all the tcam entry has same mask length in the subsegment
 */
struct taps_tcam_subsegment_s {
    int16 start;       /* start ID of subsegment */
    uint16 size;       /* size of subsegment */
    uint16 use_count;  /* number of tcam entries used */
};

static int _taps_tcam_entry_move(int unit, taps_tcam_t *handle,
				 taps_wgroup_handle_t *wgroup,
				 int candidate_entry, int free_entry);

typedef struct _tcam_traverse_datum_s {
    void *user_data;
    tcam_traverse_cb_f user_cb;
    taps_tcam_pivot_handle_t *tpivots;
    int num_tpivots;   
} tcam_traverse_datum_t;

static taps_work_type_e_t work_type[] = {TAPS_TCAM_WORK};


/*
 *   Function
 *      taps_tcam_driver_init
 *   Purpose
 *      Misc driver init for tcam
 *   Parameters
 *      (IN) unit     : unit number of the device
 *   Returns
 *       SOC_E_NONE - successfuly initialized
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_driver_init(int unit)
{
    int rv = SOC_E_NONE;

    /* seems nothing need here so far */

    return rv;
}

/*
 *   Function
 *      taps_tcam_create
 *   Purpose
 *      create the tcam object
 *   Parameters
 *      (IN) unit     : unit number of the device
 *      (IN) taps     : taps object handle
 *      (OUT)p_handle : tcam object handle
 *   Returns
 *       SOC_E_NONE - successfuly created
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_create(int unit, 
                     const taps_handle_t taps,
		     taps_tcam_handle_t *p_handle) 
{
     int rv = SOC_E_NONE;
     unsigned int key_length, average_space, remain_entries, remain_ssegs, key_offset;
     taps_tcam_handle_t handle;
     taps_key_attr_t *key_attr=NULL;
     taps_instance_segment_attr_t *seginfo=NULL;

     /* init parameter sanity check */
     if (!p_handle || !taps) return SOC_E_PARAM;

     key_attr = &taps->param.key_attr;

     seginfo = &taps->param.seg_attr.seginfo[taps->param.instance];

     if (seginfo->offset >= TAPS_TCAM_NUM_ENTRIES) {
	 /* NOTE: since TCAM ignore last 2 bits of key, the key size have to be larger than 2 */
	 return SOC_E_PARAM;
     }

     if ((key_attr->lookup_length != TAPS_IPV4_KEY_SIZE) &&
	 (key_attr->lookup_length != TAPS_IPV6_KEY_SIZE)) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d unsupported key size %d on Tcam segment.\n"),
                    FUNCTION_NAME(), unit, key_attr->lookup_length));
	 return SOC_E_PARAM;
     }

     /* allocate tcam segment management database */
     *p_handle = sal_alloc(sizeof(taps_tcam_t), "taps tcam segment dbase");
     if (*p_handle == NULL) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d Failed to allocate memory for Tcam segment\n"),
                    FUNCTION_NAME(), unit));
	 return SOC_E_MEMORY;
     } else {
	 handle = *p_handle;
	 sal_memset(handle, 0, sizeof(taps_tcam_t));
         handle->tapsi = taps->hwinstance;
	 handle->key_size = key_attr->lookup_length;
	 handle->base = seginfo->offset;
	 handle->size = seginfo->num_entry;
	 handle->segment = taps->segment;
         handle->taps = taps;
	 handle->use_count = 0;
     }

     
     rv = trie_init(taps->param.key_attr.lookup_length, &(handle->trie));
     if (SOC_FAILURE(rv)) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d Failed to allocate trie dbase memory for Tcam segment\n"),
                    FUNCTION_NAME(), unit));
	 taps_tcam_destroy(unit, handle);
	 return rv;
     }

     /* alloc the pivot dbase, use hpcm library to manage it */
     if (TAPS_IS_IPV4(handle)) {
	 handle->pivot_size = sizeof(taps_tcam_pivot_t);
     } else {
	 handle->pivot_size = sizeof(taps_tcam_pivot_t) + 
	     sizeof(uint32)*(BITS2WORDS(TAPS_IPV6_KEY_SIZE)-BITS2WORDS(TAPS_IPV4_KEY_SIZE));
     }
     rv = hpcm_init(unit, handle->size, handle->pivot_size, &(handle->pivots_hpcm));
     if (SOC_FAILURE(rv)) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d Failed to allocate pivot hpcm memory for Tcam segment\n"),
                    FUNCTION_NAME(), unit));
	 taps_tcam_destroy(unit, handle);	 
	 return rv;
     }

     /* alloc the in_use bitmask */
     handle->in_use = sal_alloc(sizeof(uint32) * SHR_BITALLOCSIZE(handle->size), "taps tcam in_use dbase");
     if (handle->in_use == NULL) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d Failed to allocate in_use dbase memory for Tcam segment\n"),
                    FUNCTION_NAME(), unit));
	 taps_tcam_destroy(unit, handle);
	 return SOC_E_MEMORY;
     } else {
	 sal_memset(handle->in_use, 0, sizeof(uint32) * SHR_BITALLOCSIZE(handle->size));
     }

     /* alloc the id to tcam_pivot handle map */
     handle->map = sal_alloc(sizeof(taps_tcam_pivot_t*) * handle->size,
			     "taps tcam map dbase");
     if (handle->map == NULL) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d Failed to allocate map dbase memory for Tcam segment\n"),
                    FUNCTION_NAME(), unit));
	 taps_tcam_destroy(unit, handle);
	 return SOC_E_MEMORY;	 
     } else {
	 sal_memset(handle->map, (uint32)NULL, sizeof(taps_tcam_pivot_t*) * (handle->size));
     }

     /* alloc the tcam subsegment dbase */
     handle->ssegs = sal_alloc(sizeof(taps_tcam_subsegment_t) * 
			       TAPS_TCAM_NUM_SSEGS(handle->key_size),
			       "taps tcam subsegment dbase");
     if (handle->ssegs == NULL) {
	 LOG_ERROR(BSL_LS_SOC_COMMON,
                   (BSL_META_U(unit,
                               "%s: unit %d Failed to allocate subsegment dbase memory for Tcam segment\n"),
                    FUNCTION_NAME(), unit));
	 taps_tcam_destroy(unit, handle);
	 return SOC_E_MEMORY;
     } else {
	 sal_memset(handle->ssegs, 0, sizeof(taps_tcam_subsegment_t) * 
		    TAPS_TCAM_NUM_SSEGS(handle->key_size));
     }

     if (handle->size < TAPS_TCAM_NUM_SSEGS(handle->key_size)) {
	 /* initially give all TCAM entry to the subsegment of key_length==0 
	  * arbitration routine will search and find free entry later
	  */
	 handle->ssegs[0].start = 0;
	 handle->ssegs[0].size = handle->size;
     } else {

	 /* chop the tcam segment into subsegments, each subsegment contains all tcam entry
	  * with the same key length.
	  * Rules:
	  *    (1) size of each subsegment will be <= 2^(key length). for example, when key length is
	  *        1, we need 2 tcam entries at most, one for 0*, the other for 1*.
	  *    (2) given TCAM don't care last 2 LSB, we don't need to allocate space for those 2 length
	  *    (3) shorter length subsegment take larger offset in the tcam segment
	  *    (4) when all above rules are followed, evenly divide the remaining space,
	  *        if there are any left over, give it to the subsegments with key_length of 16 (to help
	  *        the case of vrf case, which is 16 bits key length)
	  */
	 remain_entries = handle->size;
	 remain_ssegs = TAPS_TCAM_NUM_SSEGS(handle->key_size);
	 average_space = handle->size / remain_ssegs;
	 for (key_length = 0; key_length < TAPS_TCAM_NUM_SSEGS(handle->key_size); key_length++) {
	     if (key_length <= 31 && ((1<<key_length) <= average_space)) {
		 /* NOTE: before the key_length >= 12, we are going to run out of
		  * tcam entry and start even distribution among remaining subsegments
		  * no need to worry about shift of more than 32 bits here.
		  * and we will always hit the key_length == 16 case below
		  */
		 handle->ssegs[key_length].size = (1<<key_length);
		 remain_entries -= (1<<key_length);
		 remain_ssegs--;
		 average_space = remain_entries / remain_ssegs;
	     } else {
		 /* here we start the even divide */
		 if (key_length == 16) {
		     handle->ssegs[key_length].size = average_space + (remain_entries % remain_ssegs);
		 } else {
		     handle->ssegs[key_length].size = average_space;
		 }
	     }
	 }

	 /* now we have all the size info, calculate the start for each subsegment */
	 key_offset = handle->size;
	 for (key_length = 0; key_length < TAPS_TCAM_NUM_SSEGS(handle->key_size); key_length++) {
	     handle->ssegs[key_length].use_count = 0;
	     handle->ssegs[key_length].start = key_offset - handle->ssegs[key_length].size;
	     key_offset -= handle->ssegs[key_length].size;
	     if (handle->ssegs[key_length].start < 0) {
		 LOG_ERROR(BSL_LS_SOC_COMMON,
                           (BSL_META_U(unit,
                                       "%s: unit %d Failed to divide Tcam segment of size %d with handle->key_size %d \n"),
                            FUNCTION_NAME(), unit, handle->size, handle->key_size));
		 taps_tcam_destroy(unit, handle);
		 return SOC_E_INTERNAL;
	     }
	 }
     }

     if (_debug) {
	 (void) taps_tcam_subsegment_dump(unit, handle, taps->segment, -1);
     }

     return rv;
}

/*
 *   Function
 *      taps_tcam_destroy
 *   Purpose
 *      destroy the tcam object
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *   Returns
 *       SOC_E_NONE - successfuly destroyed
 *       SOC_E_* as appropriate otherwise
 */
void taps_tcam_destroy(int unit, taps_tcam_handle_t handle)
{
    if (handle == NULL) {
	return;
    }

    if (handle->pivots_hpcm != NULL) {
	(void)hpcm_destroy(unit, handle->pivots_hpcm);
    }

    if (handle->in_use != NULL) {
	sal_free(handle->in_use);
    }

    if (handle->map != NULL) {
	sal_free(handle->map);
    }

    if (handle->ssegs != NULL) {
	sal_free(handle->ssegs);
    }

    if (handle->trie != NULL) {
	trie_destroy(handle->trie);
    }

    sal_free(handle);

    return;
}

int taps_tcam_find_bpm(int unit, taps_tcam_t *handle, uint32 *key,
                        uint32 length, uint32 *bpm_length)
{
    int rv = SOC_E_NONE;

    if (!handle || !key || !bpm_length) return SOC_E_PARAM;

    rv = trie_find_prefix_bpm(handle->trie, key, length, bpm_length);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find BPM for following key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, key, length);
    }
    return rv;	
}


/*
 *   Function
 *      _taps_tcam_find_bpm
 *   Purpose
 *      Find Best Prefix Match (BPM) length and global_index for a given
 *      key/length in the tcam database. Note that it doesn't search down
 *      the SBucket trie for addtional BPM info, so should only be used
 *      for tcam propagation.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) key    : key buffer of the prefix
 *      (IN) length : key length of the prefix
 *      (OUT) bpm_length      : bpm key length of the prefix
 *      (OUT) bpm_global_index: global index of bpm key
 *   Returns
 *       SOC_E_NONE - found (we should always find one assuming * is in system)
 *       SOC_E_* as appropriate otherwise
 */
int _taps_tcam_find_bpm(int unit, taps_tcam_t *handle,
                            uint32 *key, uint32 length,
                            uint32 *bpm_length, uint32 *bpm_global_index) 
{
    int rv = SOC_E_NONE, word_idx;
    uint32 bpm_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)];

    rv = trie_find_prefix_bpm(handle->trie, key, length, bpm_length);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find BPM for following key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, key, length);
	return rv;	
    }

    for (word_idx = 0; word_idx < BITS2WORDS(handle->key_size); word_idx++) {
	bpm_key[word_idx]=key[word_idx];
    }

    /* right shift key to construct bpm_key */
    rv = taps_key_shift(handle->key_size, bpm_key, length, (length-(*bpm_length)));
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to construct BPM key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	return rv;	
    }	

    /* find the global_index of the bpm_key */
    rv = taps_find_prefix_global_pointer(unit, handle->taps, 
                                bpm_key, *bpm_length, 
                                bpm_global_index);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find global_index for BPM key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	return rv;	
    }

    return rv;
}

/*
 *   Function
 *      taps_tcam_insert_pivot
 *   Purpose
 *      insert a tcam pivot object into tcam 
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) key    : key buffer of the pivot
 *      (IN) length : key length of the pivot
 *      (IN) bpm_mask: bpm_mask of the pivot
 *      (IN) sbh    : sbucket object handle assocated with tcam pivot object
 *      (IN) p_ntph : tcam pivot object handle to created due to insert
 *   Returns
 *       SOC_E_NONE - successfully inserted
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_insert_pivot(int unit, taps_tcam_handle_t handle,
			   taps_wgroup_handle_t *wgroup,
			   uint32 *key, uint32 length,
			   uint32 *bpm_mask,
			   taps_sbucket_handle_t sbh,
			   taps_tcam_pivot_handle_t *p_ntph,
			   int allocated_entry,
			   uint8 with_id)
{
    int rv = SOC_E_NONE, word_idx=0;    
    int tcam_entry = -1;
    int hw_instance = 0;
    int hw_tcam_entry = 0;
    int hw_bucket_id = 0;
    soc_sbx_caladan3_tmu_cmd_t *rpb_write = NULL;
    uint32 bpm_length;
    uint32 bpm_global_index;
    uint32 bpm_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)];
    trie_node_t *payload = NULL;

    /*=== parameter check */
    if ((handle == NULL) || (key == NULL) || (bpm_mask == NULL) || 
	(sbh == NULL) || (p_ntph == NULL)) {
	    return SOC_E_PARAM;
    }

    *p_ntph = NULL;
    /* get the bpm_length from bpm_mask */
    rv = taps_get_bpm_pfx(bpm_mask, length, handle->key_size, &bpm_length);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find proper bpm in bpm_mask passed in\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, bpm_mask, handle->key_size);	
	rv = SOC_E_INTERNAL;
    goto tcam_insert_error;
    } 

    if ((bpm_length == 0) && (length != 0)) {
	/* there is no bpm in the splited domain, get the bpm_length from
	 * the tcam trie.
	 */
	rv = trie_find_prefix_bpm(handle->trie, key, length, &bpm_length);
	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to find taps bpm prefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
	    goto tcam_insert_error;
	}
    }

    /*=== allocate pivot */
    rv = hpcm_alloc_payload(unit, handle->pivots_hpcm, (void **)p_ntph);
    if (SOC_FAILURE(rv)) {	
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate pivot for tcam segment %d\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	goto tcam_insert_error;
    }

    /*=== try to insert into trie, need to maintain bpm_mask */
    rv = trie_insert(handle->trie, key, bpm_mask, length, (trie_node_t*)*p_ntph);
    if (SOC_FAILURE(rv)) {
	/* failed to insert, it should only happen if our bucket split algorithm
	 * is wrong so that somehow a duplicated pivot is elected
	 */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam segment %d failed to insert pivot into trie\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, key, length);
	goto tcam_insert_error;
    } else {
        /* init the new TPH object, trie_node part of the object already done by
         * the trie_insert function. The prefix part is variable length, so not
         * part of taps_tcam_pivot_t structure, however, we allocated the object
         * in such a way so that the memory immediately following will have proper
         * size based on key_size and used for prefix.
         * NOTE: we need to init the new tph so that the following function
         * taps_find_prefix_global_pointer can have a consistent database when
         * the pivot itself is the bpm of itself.
         */
        (*p_ntph)->sbucket = sbh;
        (*p_ntph)->length = length;
        (*p_ntph)->key[0] = key[0];
        (*p_ntph)->key[1] = key[1];
        
        if (TAPS_IS_IPV6(handle)) {
            /* coverity[overrun-local : FALSE] */
            (*p_ntph)->key[2] = key[2];
            /* coverity[overrun-local : FALSE] */
            (*p_ntph)->key[3] = key[3];
            /* coverity[overrun-local : FALSE] */
            (*p_ntph)->key[4] = key[4];
        }
    }
    if (with_id == TRUE) {
        tcam_entry = allocated_entry;
    } else {
        /*=== trie insert done, find a tcam free entry to insert */
        rv = taps_tcam_entry_alloc(unit, handle, wgroup, length, &tcam_entry);
        if (SOC_FAILURE(rv)) {
        	/* TCAM full, we should prevent this from happening by checking TCAM use counters */
        	LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Tcam segment %d failed to alloc entry for length %d\n"),
                           FUNCTION_NAME(), unit, handle->segment, length));
        	goto tcam_insert_error;
        }
    } 
    (*p_ntph)->entry = tcam_entry;
    /* construct TAPS_TCAM_WRITE command to initialize the tcam entry 
     *    cmd.blk             --> 1
     *    cmd.opcode          --> 0x2 (write)
     *    cmd.target          --> 0x7 (TCAM, TDM and ADS)
     *    cmd.seg             --> handle->segment
     *    cmd.offset          --> tcam_entry
     *    cmd.bpm_length      --> bpm_length passed in (calculated by sbucket split algorithm)
     *    cmd.kshift          --> 0 (offchip mode no key shifting)
     *                            --> length (onchip mode shift length bits)
     *    cmd.bucket          --> sbh->bucket
     *    cmd.best_match      --> bpm_length is known, perform exact match on bpm_key/bpm_length to get global_index
     *    cmd.a               --> FALSE (always align left)
     *    cmd.g               --> 0 (no MSB wildcard)
     *    cmd.plength         --> length
     *    cmd.pdata           --> key
     */

    for (word_idx = 0; word_idx < BITS2WORDS(handle->key_size); word_idx++) {
	bpm_key[word_idx]=key[word_idx];
    }

    /* right shift key to construct bpm_key */
    rv = taps_key_shift(handle->key_size, bpm_key, length, length-bpm_length);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to construct BPM key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	goto tcam_insert_error;
    }

    /* get the bpm_global_index */
    rv = taps_find_prefix_global_pointer(unit, handle->taps,  
                                    bpm_key, bpm_length, &bpm_global_index);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find global index for prefix "
                              "with bpm_mask : 0x%x, 0x%x, 0x%x, 0x%x, 0x%x \n"),
                   FUNCTION_NAME(), unit, handle->segment,
                   bpm_mask[0], bpm_mask[1], bpm_mask[2], bpm_mask[3], bpm_mask[4]));
	rv = SOC_E_INTERNAL;
    goto tcam_insert_error;
    }
    
    /* enqueue the TCAM WRITE ENTRY command */
    rv = tmu_cmd_alloc(unit, &rpb_write);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	goto tcam_insert_error;
    }
    
    /* Get hardware instance and entry id */
    rv = taps_instance_and_entry_id_get(unit, handle->taps, 
                                    tcam_entry, sbh->domain,
                                    &hw_instance, &hw_tcam_entry, &hw_bucket_id);
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to get hardware instance and entry id\n"),
                   FUNCTION_NAME(), unit));
    	goto tcam_insert_error;
    }

    rpb_write->opcode = SOC_SBX_TMU_CMD_TAPS;
    rpb_write->cmd.taps.instance =  hw_instance;
    rpb_write->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    rpb_write->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE;
    rpb_write->cmd.taps.max_key_size = handle->key_size;
    rpb_write->cmd.taps.subcmd.rpb_write.segment = handle->segment;
    rpb_write->cmd.taps.subcmd.rpb_write.offset = hw_tcam_entry;
    rpb_write->cmd.taps.subcmd.rpb_write.target = SOC_SBX_TMU_TAPS_RPB_ALL;
    rpb_write->cmd.taps.subcmd.rpb_write.bpm_length = bpm_length;
    if (handle->taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* Don't need to do key shift for offchip mode*/
        rpb_write->cmd.taps.subcmd.rpb_write.kshift = 0;
        rpb_write->cmd.taps.subcmd.rpb_write.length = length;
    } else {
        rpb_write->cmd.taps.subcmd.rpb_write.kshift = sbh->tcam_shift_len;
        rpb_write->cmd.taps.subcmd.rpb_write.length = sbh->tcam_shift_len;
    }
    rpb_write->cmd.taps.subcmd.rpb_write.bucket = hw_bucket_id;
    if (handle->taps->param.mode == TAPS_ONCHIP_ALL) {
        rpb_write->cmd.taps.subcmd.rpb_write.best_match = 
            sbh->domain * (1 << (handle->taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_;
    } else {
        rpb_write->cmd.taps.subcmd.rpb_write.best_match = bpm_global_index;
    }
    rpb_write->cmd.taps.subcmd.rpb_write.align_right = FALSE;
    
    rpb_write->cmd.taps.subcmd.rpb_write.key[0] = key[0];
    rpb_write->cmd.taps.subcmd.rpb_write.key[1] = key[1];
    if (TAPS_IS_IPV6(handle)) {
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[2] = key[2];
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[3] = key[3];
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[4] = key[4];
    }
    
    rv = taps_command_enqueue_for_all_devices(unit, handle->taps, wgroup, TAPS_TCAM_WORK, rpb_write);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	goto tcam_insert_error;
    } else {
	LOG_DEBUG(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "%s tcam insert entry %d with bucket %d bpm_length %d global_index %d.\n"),
	           FUNCTION_NAME(), tcam_entry, sbh->domain, bpm_length, bpm_global_index));
    }

    handle->map[tcam_entry] = *p_ntph;

tcam_insert_error:
    if (SOC_FAILURE(rv)){
        if (tcam_entry >= 0) {
            SHR_BITCLR(handle->in_use, tcam_entry);
            handle->map[tcam_entry] = NULL;
            handle->ssegs[length].use_count--;
            handle->use_count--;
        }
        if (*p_ntph != NULL && handle != NULL && key != NULL) {
            trie_delete(handle->trie, key, length, &payload);
            (void)hpcm_free_payload(unit, handle->pivots_hpcm, (void *)(*p_ntph));
            *p_ntph = NULL;
        }
        if (rpb_write != NULL) {
	        tmu_cmd_free(unit, rpb_write);
        }
    }
    /* no need to do propagation, since we are not adding routes, we are just
     * re-organizing tcam and buckets
     */

    return rv;
}

/*
 *   Function
 *      taps_tcam_delete_pivot
 *   Purpose
 *      delete a tcam pivot object from tcam 
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) tph    : tcam pivot object handle to be deleted
 *   Returns
 *       SOC_E_NONE - successfully deleted
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_delete_pivot(int unit, taps_tcam_handle_t handle,
			   taps_wgroup_handle_t *wgroup,
			   taps_tcam_pivot_handle_t tph)
{
    int rv = SOC_E_NONE;
    uint32 key[TAPS_MAX_KEY_SIZE_WORDS], length;
    trie_node_t *payload;

    /*=== delete tph.prefix.key/length. trie_delete will leave payload node alone */
    length = tph->length;
    key[0] = tph->key[0];
    key[1] = tph->key[1];

    if (TAPS_IS_IPV6(handle)) {
	/* coverity[overrun-local : FALSE] */
	key[2] = tph->key[2];
	/* coverity[overrun-local : FALSE] */
	key[3] = tph->key[3];
	/* coverity[overrun-local : FALSE] */
	key[4] = tph->key[4];
    }

    rv = trie_delete(handle->trie, key, length, &payload);
    if (SOC_FAILURE(rv)) {
	/* tph is 1-1 mapped to a prefix in the trie, so it should never fail,
	 * unless somehow the trie dbase is corrupted
	 */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam Segment %d Failed to delete pivot\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, key, length);
	return SOC_E_NOT_FOUND;
    }

    /*=== invalidate the tcam entry in hardware */
    rv = taps_tcam_entry_free(unit, handle, wgroup, tph->entry);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam Segment %d Failed to free entry %d\n"),
                   FUNCTION_NAME(), unit, handle->segment, tph->entry));
	return rv;
    }

    handle->map[tph->entry] = NULL;

    /*=== free the tcam entry payload in software dbase */
    rv = hpcm_free_payload(unit, handle->pivots_hpcm, tph);    
    return rv;
}

/*
 *   Function
 *      taps_tcam_insert_default_entry
 *   Purpose
 *      Insert default entry(0/0) in tcam. 
 *      No software update. Insert this entry as the last entry in tcam.
 */
int taps_tcam_insert_default_entry(int unit, taps_handle_t taps, 
                                taps_wgroup_handle_t work_group)
{
    int rv;
    int instance, tcam_entry;
    soc_sbx_caladan3_tmu_cmd_t *rpb_write = NULL;
    
    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
       instance = taps->param.instance;
       tcam_entry = taps->param.seg_attr.seginfo[taps->param.instance].num_entry - 1;   
    } else {
       if (taps->param.divide_ratio != ceiling_ratio) {
           instance = TAPS_INST_1;
           tcam_entry = taps->param.seg_attr.seginfo[TAPS_INST_1].num_entry - 1;
       } else {
	       /* No entry in TAPS1 */
           instance = TAPS_INST_0;
           tcam_entry = taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry - 1;
       }
    }

    if (taps->tcam_hdl->size < TAPS_TCAM_NUM_SSEGS(taps->tcam_hdl->key_size)) {
        tcam_entry = 0;
    }

    /* enqueue the TCAM WRITE ENTRY command */
    rv = tmu_cmd_alloc(unit, &rpb_write);
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed "
                              "to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, taps->segment));
    	return rv;
    }
    
    rpb_write->opcode = SOC_SBX_TMU_CMD_TAPS;
    rpb_write->cmd.taps.instance =  SW_INST_CONVERT_TO_HW_INST(instance);
    rpb_write->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    rpb_write->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE;
    rpb_write->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
    rpb_write->cmd.taps.subcmd.rpb_write.segment = taps->segment;
    rpb_write->cmd.taps.subcmd.rpb_write.offset = tcam_entry;
    rpb_write->cmd.taps.subcmd.rpb_write.target = SOC_SBX_TMU_TAPS_RPB_ALL;
    rpb_write->cmd.taps.subcmd.rpb_write.bpm_length = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.kshift = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.length = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.bucket = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.best_match = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.align_right = FALSE;
    
    sal_memset(&rpb_write->cmd.taps.subcmd.rpb_write.key[0], 0, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);

    rv = taps_work_enqueue(unit, work_group, TAPS_TCAM_WORK, &(rpb_write->wq_list_elem));
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                   FUNCTION_NAME(), unit, taps->segment));
    	tmu_cmd_free(unit, rpb_write);
    }

    return rv;
}

/*
 *   Function
 *      taps_tcam_replace_hw_set
 *   Purpose
 *      Set hardware configuration for replace action 
 */
int taps_tcam_replace_hw_set(int unit, taps_tcam_handle_t handle, 
                                    taps_wgroup_handle_t *wgroup, 
                                    int instance, int length, uint32 *key)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_tmu_cmd_t *tcam_cmd = NULL;
    uint32 bpm_length = 0;
    uint32 bpm_global_index = 0;

    if (!handle || !key) {
        return SOC_E_PARAM;
    }

    /* find out the bpm of the deleted route */
    rv = _taps_tcam_find_bpm(unit, handle, key, length,
                        &bpm_length, &bpm_global_index);
    if (SOC_FAILURE(rv)) {
    /* we should always be able to find a bpm here unless something internal is wrong */
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d segment %d Failed to find BPM info for key\n"),
               FUNCTION_NAME(), unit, handle->segment));
    (void)taps_show_prefix(handle->key_size, key, length);
    return rv;  
    }
        
    /* enqueue TCAM REPLACE command */
    rv = tmu_cmd_alloc(unit, &tcam_cmd);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
        return rv;
    }

    tcam_cmd->opcode = SOC_SBX_TMU_CMD_TAPS;
    tcam_cmd->cmd.taps.instance = SW_INST_CONVERT_TO_HW_INST(instance);
    tcam_cmd->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    tcam_cmd->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_REPLACE;
    tcam_cmd->cmd.taps.max_key_size = handle->key_size;
    tcam_cmd->cmd.taps.subcmd.rpb_replace.segment = handle->segment;
    tcam_cmd->cmd.taps.subcmd.rpb_replace.bpm_length = bpm_length;
    tcam_cmd->cmd.taps.subcmd.rpb_replace.best_match = bpm_global_index;
    tcam_cmd->cmd.taps.subcmd.rpb_replace.align_right = FALSE;
    tcam_cmd->cmd.taps.subcmd.rpb_replace.length = length;
    tcam_cmd->cmd.taps.subcmd.rpb_replace.key[0] = key[0];
    tcam_cmd->cmd.taps.subcmd.rpb_replace.key[1] = key[1];
    if (TAPS_IS_IPV6(handle)) {
        /* coverity[overrun-local : FALSE] */
        tcam_cmd->cmd.taps.subcmd.rpb_replace.key[2] = key[2];
        /* coverity[overrun-local : FALSE] */
        tcam_cmd->cmd.taps.subcmd.rpb_replace.key[3] = key[3];
        /* coverity[overrun-local : FALSE] */
        tcam_cmd->cmd.taps.subcmd.rpb_replace.key[4] = key[4];
    }
    
    rv = taps_command_enqueue_for_all_devices(unit, handle->taps, wgroup, TAPS_TCAM_WORK, tcam_cmd);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
        tmu_cmd_printf(unit, tcam_cmd);
        tmu_cmd_free(unit, tcam_cmd);
        return rv;
    } else {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s tcam replace for bpm length %d global_index %d.\n"),
                   FUNCTION_NAME(), bpm_length, bpm_global_index));       
    }                           

    return rv;
}

/*
 *   Function
 *      taps_tcam_propagate_hw_set
 *   Purpose
 *      Set hardware configuration for propagate action 
 */
int taps_tcam_propagate_hw_set(int unit, taps_tcam_handle_t handle, 
                                    taps_wgroup_handle_t *wgroup, 
                                    int instance, int length, uint32 *key, int global_index)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_tmu_cmd_t *tcam_cmd = NULL;

    if (!handle || !key) {
        return SOC_E_PARAM;
    }
    
    if (global_index < 0) {
	    /* caller don't know the global_index of added index, search
	     * database to find it out.
	     */
	    rv = taps_find_prefix_global_pointer(unit, handle->taps, key, length,
						      (uint32 *)&global_index);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d segment %d Failed to find global_index for key\n"),
                           FUNCTION_NAME(), unit, handle->segment));
		return rv;	
	    }
	}

    
    /*=== construct TAPS_TCAM_PROPAGATION command and add it to the work queue to
	 *    mark TCAM entry as invalid.
	 *    cmd.tapsi           --> handle->tapsi
	 *    cmd.blk             --> 1
	 *    cmd.opcode          --> 0x4 (Propagate)
	 *    cmd.seg             --> handle->segment
	 *    cmd.best_match      --> global_index_added
	 *    cmd.a               --> FALSE
	 *    cmd.plength         --> length
	 *    cmd.pdata           --> *key
	 */
	rv = tmu_cmd_alloc(unit, &tcam_cmd);
        if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                       FUNCTION_NAME(), unit, handle->segment));
	    return rv;
	}
	
	tcam_cmd->opcode = SOC_SBX_TMU_CMD_TAPS;
	tcam_cmd->cmd.taps.instance = SW_INST_CONVERT_TO_HW_INST(instance); 
	tcam_cmd->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
	tcam_cmd->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_PROPAGATE;
	tcam_cmd->cmd.taps.max_key_size = handle->key_size;
	tcam_cmd->cmd.taps.subcmd.rpb_propagate.segment = handle->segment;
	tcam_cmd->cmd.taps.subcmd.rpb_propagate.best_match = global_index;
	tcam_cmd->cmd.taps.subcmd.rpb_propagate.align_right = FALSE;
	tcam_cmd->cmd.taps.subcmd.rpb_propagate.length = length;
	tcam_cmd->cmd.taps.subcmd.rpb_propagate.key[0] = key[0];
	tcam_cmd->cmd.taps.subcmd.rpb_propagate.key[1] = key[1];
	if (TAPS_IS_IPV6(handle)) {
	    /* coverity[overrun-local : FALSE] */
	    tcam_cmd->cmd.taps.subcmd.rpb_propagate.key[2] = key[2];
	    /* coverity[overrun-local : FALSE] */
	    tcam_cmd->cmd.taps.subcmd.rpb_propagate.key[3] = key[3];
	    /* coverity[overrun-local : FALSE] */
	    tcam_cmd->cmd.taps.subcmd.rpb_propagate.key[4] = key[4];
	}

    rv = taps_command_enqueue_for_all_devices(unit, handle->taps, wgroup, TAPS_TCAM_WORK, tcam_cmd);
    if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                       FUNCTION_NAME(), unit, handle->segment));
	    tmu_cmd_printf(unit, tcam_cmd);
	    tmu_cmd_free(unit, tcam_cmd);
	    return rv;
	} else {
	    LOG_DEBUG(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "%s tcam propagation for key length %d global_index %d.\n"),
	               FUNCTION_NAME(), length, global_index));	    
	}

    return rv;
}
    
/*
 *   Function
 *      taps_tcam_lookup_prefix
 *   Purpose
 *      Lookup a prefix, return matched tcam pivot in Tcam
 *      if tph is not NULL, assuming the prefix was added/deleted in the sbucket associated
 *      with the tph, otherwise we start searching from root trie node.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) key    : key buffer of the prefix
 *      (IN) length : key length of the prefix
 *      (OUT) p_tph : matching tcam pivot object handle if found
 *   Returns
 *       SOC_E_NONE - found
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_lookup_prefix(int unit, taps_tcam_handle_t handle,
                            uint32 *key, uint32 length,
                            taps_tcam_pivot_handle_t *p_tph)
{
    int rv = SOC_E_NONE;    
    trie_node_t *trie_node;

    if ((handle == NULL) || (key == NULL) || (p_tph == NULL)) {
    return SOC_E_PARAM;
    }

    /*=== search trie, return the last payload node we found */
    *p_tph = NULL;
    rv = trie_find_lpm(handle->trie, key, length, &trie_node);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam Segment %d Failed to find LPM for key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, key, length);
    } else {
	*p_tph = (taps_tcam_pivot_handle_t)trie_node;
    }

    return rv;
}

/*
 * propagate callback function
 */
static int _taps_propagate_assodata_update_cb(trie_node_t *trie, trie_bpm_cb_info_t *info)
{
    int rv, unit, add;
    taps_wgroup_handle_t  *wgroup;
    taps_handle_t          taps;
    taps_tcam_pivot_handle_t tph = NULL;
    asso_data_update_cb_data_t *user_data = NULL;
    unsigned int  asso_data;
    unsigned int *payload = NULL;
    
    user_data = (asso_data_update_cb_data_t *)info->user_data;
    tph = (taps_tcam_pivot_handle_t)trie;

    unit = user_data->unit;
    wgroup = user_data->wgroup;
    taps = user_data->taps;
    add = user_data->add;

    if (add) {
        payload = user_data->payload;
    } else {
        /* For delete, we need to find out the bpm of this prefix */
        rv = taps_find_bpm_asso_data(unit, taps->tcam_hdl, info->pfx, info->len, NULL, &payload);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Failed to get asso data error %d:%s\n"),
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            return rv;
        }
    }

    /* Only for mode zero */
    asso_data = payload[0] & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);

    rv = taps_sbucket_enqueue_update_assodata_work(unit, 
                                     taps,
                                     tph->sbucket,
                                     tph->sbucket->wsph,
                                     wgroup,
                                     asso_data);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to update asso data for mode zero %d:%s!!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    } 
    return SOC_E_NONE;
}

/*
 *   Function
 *      taps_tcam_propagate_prefix_for_modezero
 *   Purpose
 *      Propagation for mode zero. Since we have a wild sph in each sbucket for mode zero, 
 *      just update the asso data rather than update the global index in tcam.
 */
int taps_tcam_propagate_prefix_for_modezero(int unit, taps_tcam_handle_t handle,
			       taps_wgroup_handle_t *wgroup,
                                    taps_tcam_pivot_handle_t tph, int add,
                                    uint32 *key, uint32 length, unsigned int *payload)
{
    int rv = SOC_E_NONE;
    trie_node_t *pivot = NULL;
    trie_bpm_cb_info_t asso_data_udpate_cb_info;
    asso_data_update_cb_data_t *asso_data_update_user_data = NULL;

    if ((handle == NULL) || (key == NULL) || (tph == NULL) || (add && (!payload))) {
        return SOC_E_PARAM; 
    }

    asso_data_update_user_data = sal_alloc(sizeof(asso_data_update_cb_data_t), "asso_data_update_user_data");

    if (asso_data_update_user_data == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to alloc mem for user data %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return SOC_E_FAIL;
    }
    sal_memset(asso_data_update_user_data, 0, sizeof(asso_data_update_cb_data_t));
    pivot = (trie_node_t*) tph;
    
    asso_data_update_user_data->unit = unit;
    asso_data_update_user_data->wgroup = wgroup;
    asso_data_update_user_data->taps = handle->taps;
    asso_data_update_user_data->add = add;
    asso_data_update_user_data->payload = payload;

    asso_data_udpate_cb_info.pfx = key;
    asso_data_udpate_cb_info.len = length;
    asso_data_udpate_cb_info.user_data = asso_data_update_user_data;
    
    if (_TAPS_KEY_IPV6(handle->taps)) {
    rv = trie_v6_pivot_propagate_prefix(pivot, tph->length, key, length,
                        add?1:0, _taps_propagate_assodata_update_cb, 
                        &asso_data_udpate_cb_info); 
    } else {
    rv = trie_pivot_propagate_prefix(pivot, tph->length, key, length,
                     add?1:0, _taps_propagate_assodata_update_cb, 
                     &asso_data_udpate_cb_info);
    }
    
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to propagate prefix in tcam  %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }  

    sal_free(asso_data_update_user_data);
    return rv;
}

/*
 * propagate callback function
 */
int taps_tcam_propagate_prefix_cb(trie_node_t *trie, trie_bpm_cb_info_t *info)
{
    taps_tcam_pivot_handle_t tph = NULL;
    tcam_bpm_cb_info_t *user_data = NULL;

    user_data = (tcam_bpm_cb_info_t *)info->user_data;
    tph = (taps_tcam_pivot_handle_t)trie;

    /* set the user_data to something not NULL */
    if (user_data != NULL) {
        user_data->isbpm = TRUE; 
        if (!user_data->unified_mode) {
            /* Paralle mode */
            /* set the return code to SOC_E_LIMIT to allow propagation code exit faster */
            return SOC_E_LIMIT;
        } else {
    		/* Unified mode
    		 * Propagate may happen on either of the two taps. 
    		 * So can't return SOC_E_LIMIT like parallel mode */
            if (_ENTRY_IN_TAPS0(tph->entry, user_data->split_line)) {
                user_data->propagate_taps0 = TRUE;
            } else {
                user_data->propagate_taps1 = TRUE;
            }
            if (user_data->propagate_taps0 && user_data->propagate_taps1) {
                /* If both of two taps need to do propagation,
    			* then set the return code to SOC_E_LIMIT to allow propagation code exit faster */
                return SOC_E_LIMIT;
            }
        }
    }
    return SOC_E_NONE;
}

/*
 *   Function
 *      taps_tcam_propagate_prefix
 *   Purpose
 *      Propagate an added/deleted prefix in Tcam.
 *      if tph is not NULL, assuming the prefix was added/deleted in the sbucket associated
 *      with the tph, otherwise we start searching from root trie node.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) tph    : tcam pivot object handle
 *      (IN) add    : propagation due to adding a prefix if set. otherwise due to deleting
 *      (IN) global_index_added : valid only if add==TRUE, the global_index of the 
 *                    dbucket prefix object added.
 *      (IN) key    : key buffer of the prefix
 *      (IN) length : key length of the prefix
 *      (OUT) isbpm : return TRUE if added prefix is BPM of a TCAM pivot
 *   Returns
 *       SOC_E_NONE - successfully propagated
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_propagate_prefix(int unit, taps_tcam_handle_t handle,
			       taps_wgroup_handle_t *wgroup,
                                    taps_tcam_pivot_handle_t tph, int add,
                                    int32 global_index_added,
                                    uint32 *key, uint32 length,
                                    uint8 *isbpm)
{
    int rv = SOC_E_NONE;
    trie_node_t *pivot = NULL;
    trie_bpm_cb_info_t tcam_cb_info;
    tcam_bpm_cb_info_t *tcam_user_data = NULL;

    if ((handle == NULL) || (key == NULL) || \
        (add && !isbpm) || (tph == NULL)) {
        return SOC_E_PARAM; 
    }
    
    tcam_user_data = sal_alloc(sizeof(tcam_bpm_cb_info_t), "tcam_user_data");

    if (tcam_user_data == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to alloc mem for user data %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return SOC_E_FAIL;
    }
    sal_memset(tcam_user_data, 0, sizeof(tcam_bpm_cb_info_t));
    /*  call trie library to update bitmask based on the result,
     *  we can know whether we need to propagate the propagation.
     */
    pivot = (trie_node_t*) tph;

    if(_TAPS_IS_PARALLEL_MODE_(handle->taps->param.instance)) {
        /* Paralle mode, don't need to init other params */
        tcam_user_data->unified_mode = FALSE;
    } else {
        tcam_user_data->unified_mode = TRUE;
        tcam_user_data->propagate_taps0 = FALSE;
        tcam_user_data->propagate_taps1 = FALSE;
        tcam_user_data->split_line = handle->taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry;
    }
    tcam_cb_info.pfx = key;
    tcam_cb_info.len = length;
    tcam_cb_info.user_data = tcam_user_data;
        
    if (_TAPS_KEY_IPV6(handle->taps)) {
    rv = trie_v6_pivot_propagate_prefix(pivot, tph->length, key, length,
                        add?1:0, taps_tcam_propagate_prefix_cb, 
                        &tcam_cb_info); 
    } else {
    rv = trie_pivot_propagate_prefix(pivot, tph->length, key, length,
                     add?1:0, taps_tcam_propagate_prefix_cb, 
                     &tcam_cb_info);
    }
    
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to propagate prefix in tcam  %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }  

    *isbpm = tcam_user_data->isbpm;
    if (!(*isbpm)) {
    /* no need to propagate/replace */
    sal_free(tcam_user_data);
    return SOC_E_NONE;
    }

    /* update the bpm(best prefix match) bitmap for the add/deleted route */
    if (add == TRUE) {
        if(_TAPS_IS_PARALLEL_MODE_(handle->taps->param.instance)) {
            /* Parallel mode, the propagation only happen on the specified instance */
            rv = taps_tcam_propagate_hw_set(unit, handle, wgroup, handle->taps->param.instance, 
                                length, key, global_index_added);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d segment %d Failed to "
                                      "configure hardware for propagation \n"),
                           FUNCTION_NAME(), unit, handle->segment));
                sal_free(tcam_user_data);
                return rv;  
            }
        } else {
            /* Unified mode, the propagation could happen on single instance or both on two instances */
            if (tcam_user_data->propagate_taps0) {
                rv = taps_tcam_propagate_hw_set(unit, handle, wgroup, TAPS_INST_0, 
                                length, key, global_index_added);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d segment %d Failed to "
                                          "configure hardware for propagation \n"),
                               FUNCTION_NAME(), unit, handle->segment));
                    sal_free(tcam_user_data);
                    return rv;  
                }
            }
            
            if (tcam_user_data->propagate_taps1) {
                rv = taps_tcam_propagate_hw_set(unit, handle, wgroup, TAPS_INST_1, 
                                length, key, global_index_added);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d segment %d Failed to "
                                          "configure hardware for propagation \n"),
                               FUNCTION_NAME(), unit, handle->segment));
                    sal_free(tcam_user_data);
                    return rv;  
                }
            }
        }
    } else {
        if(_TAPS_IS_PARALLEL_MODE_(handle->taps->param.instance)) {
            /* Parallel mode, the propagation only happen on the specified instance */
            rv = taps_tcam_replace_hw_set(unit, handle, wgroup, handle->taps->param.instance, 
                                length, key);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d segment %d Failed to "
                                      "configure hardware for replace action \n"),
                           FUNCTION_NAME(), unit, handle->segment));
                sal_free(tcam_user_data);
                return rv;  
            }
        } else {
            /* Unified mode, the propagation could happen on single instance or both on two instances */
            if (tcam_user_data->propagate_taps0) {
                rv = taps_tcam_replace_hw_set(unit, handle, wgroup, TAPS_INST_0, 
                                length, key);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d segment %d Failed to "
                                          "configure hardware for replace action \n"),
                               FUNCTION_NAME(), unit, handle->segment));
                    sal_free(tcam_user_data);
                    return rv;  
                }
            }
            
            if (tcam_user_data->propagate_taps1) {
                rv = taps_tcam_replace_hw_set(unit, handle, wgroup, TAPS_INST_1, 
                                length, key);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d segment %d Failed to "
                                          "configure hardware for replace action \n"),
                               FUNCTION_NAME(), unit, handle->segment));
                    sal_free(tcam_user_data);
                    return rv;  
                }
            }
        }
    }
    sal_free(tcam_user_data);
    return rv;
}

/*
 *   Function
 *      taps_tcam_commit
 *   Purpose
 *      Commit all work items in tcam queue to hardware
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) wgroup : work group handle (for future API batching)
 *   Returns
 *       SOC_E_NONE - successfully commited
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_commit(int unit, taps_wgroup_handle_t wgroup)
{
    

    if (!wgroup) return SOC_E_PARAM;

    return taps_work_commit(unit, wgroup,
                            work_type, COUNTOF(work_type),
                            _TAPS_SEQ_COMMIT);
}

/*
 *
 * Function:
 *     taps_tcam_free_work_queue
 * Purpose:
 *     free all outstanding work items
 */
int taps_tcam_free_work_queue(int unit, 
                              taps_wgroup_handle_t wgroup)
{
    if (!wgroup) return SOC_E_PARAM;

    return taps_free_work_queue(unit, wgroup,
                                work_type, 
                                COUNTOF(work_type));
}

/*
 *   Function
 *      taps_tcam_entry_alloc
 *   Purpose
 *      Internal function. enqueue tmu command to allocate a free tcam entry
 *         for a particular key length
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) key_length       : the pivot key length of the free tcam entry
 *      (IN) allocated_entry  : entry allocated
 *   Returns
 *       SOC_E_NONE - successfully allocated
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_entry_alloc(int unit, taps_tcam_handle_t handle,
				  taps_wgroup_handle_t *wgroup,
				  int key_length, int *allocated_entry)
{
    int rv = SOC_E_NONE;    
    int found, distance, sseg, start_sseg, free_entry, candidate_entry;

    if (key_length >= TAPS_TCAM_NUM_SSEGS(handle->key_size)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d key length %d too long for tcam segment %d\n"),
                   FUNCTION_NAME(), unit, key_length, handle->segment));
	return SOC_E_PARAM;
    }

    /* search for a subsegment with free entry, the subsegment need to be  */
    found = FALSE;
    for (distance = 0; distance < handle->key_size; distance++) {
	if ((key_length + distance) <= TAPS_TCAM_MAX_SSEG(handle->key_size)) {
	    if (TAPS_TCAM_SSEG_FULL(handle, key_length+distance) == FALSE) {
		found = TRUE;
		start_sseg = key_length + distance;
		break;
	    }
	}
	
	if ((key_length - distance) >= 0) {
	    if (TAPS_TCAM_SSEG_FULL(handle, key_length-distance) == FALSE) {
		found = TRUE;
		start_sseg = key_length - distance;
		break;
	    }
	}
    }

    if (found == FALSE) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam segment %d full\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	return SOC_E_RESOURCE;
    } else {
	
	for (free_entry = handle->ssegs[start_sseg].start;
	     free_entry < (handle->ssegs[start_sseg].start+handle->ssegs[start_sseg].size);
	     free_entry++) {
	    if (SHR_BITGET(handle->in_use, free_entry) == 0) {
		break;
	    }
	}

	if (free_entry == handle->ssegs[start_sseg].start+handle->ssegs[start_sseg].size) {
	    /* it should never happen unless internal software state is not consistent */
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Tcam Segment %d subsegment %d dbase inconsistent !!!\n"),
                       FUNCTION_NAME(), unit, handle->segment, key_length));
	    return SOC_E_INTERNAL;
	}

	if (start_sseg == key_length) {
	    /* there is a free entry in the subsegment */
	    SHR_BITSET(handle->in_use, free_entry);
	    *allocated_entry = free_entry;
	    handle->ssegs[key_length].use_count++;
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d Tcam Segment %d allocated entry %d\n"),
                         FUNCTION_NAME(), unit, handle->segment, free_entry));

	    if (_debug) {
		(void) taps_tcam_subsegment_dump(unit, handle, handle->taps->segment, -1);
	    }
	    
	    handle->use_count++;
	    return SOC_E_NONE;
	} else {
	    /* move the free entry to the top or bottom of the start subsegment depends on 
	     * if start_subsegment is above or below the target subsegment (the one with key length
	     * of "key_length")
	     */
	    if (start_sseg > key_length) {
		/* pick the last one of the subsegment */
		candidate_entry = handle->ssegs[start_sseg].start+handle->ssegs[start_sseg].size-1;
	    } else {
		candidate_entry = handle->ssegs[start_sseg].start;
	    }

	    rv = _taps_tcam_entry_move(unit, handle, wgroup, candidate_entry, free_entry);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to move Tcam Segment %d entry %d to entry %d\n"),
                           FUNCTION_NAME(), unit, handle->segment, candidate_entry, free_entry));
		return rv;
	    }
	    free_entry = candidate_entry;
	}

	/* shuffle the free entry till it reaches the subsegment of "key_length" 
	 * handle the case of a subsegment with size == 0.
	 */
	sseg = start_sseg;
	while (1) {
	    /* 
	     * remove the free_entry from current subsegment, move to next subsegment
	     * then add the free_entry into next subsegment. move the entry at the other
	     * end of subsegment to be candidate to move to the free_entry
	     */
	    if (sseg > key_length) {
		handle->ssegs[sseg].size--;
		sseg--;	    
		handle->ssegs[sseg].start--;
		handle->ssegs[sseg].size++;
		if (sseg == key_length) {
		    free_entry = handle->ssegs[sseg].start;
		    break;
		} else {
		    candidate_entry = handle->ssegs[sseg].start+handle->ssegs[sseg].size-1;
		}
	    } else {
		handle->ssegs[sseg].size--;
		handle->ssegs[sseg].start++;
		sseg++;
		handle->ssegs[sseg].size++;
		if (sseg == key_length) {
		    free_entry = handle->ssegs[sseg].start+handle->ssegs[sseg].size-1;
		    break;
		} else {
		    candidate_entry = handle->ssegs[sseg].start;
		}
	    }

	    if (handle->ssegs[sseg].size == 1) {
		/* free entry is the only entry in the subsegment, no need to move, just
		 * move on to next subsegment
		 */
		continue;
	    }

	    rv = _taps_tcam_entry_move(unit, handle, wgroup, candidate_entry, free_entry);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to move Tcam Segment %d entry %d to entry %d\n"),
                           FUNCTION_NAME(), unit, handle->segment, candidate_entry, free_entry));
		return rv;
	    }
	    
	    /* the candidate entry becomes the new free entry */
	    free_entry = candidate_entry;
	}

    }
    
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d Tcam Segment %d allocated entry %d\n"),
                 FUNCTION_NAME(), unit, handle->segment, free_entry));
    SHR_BITSET(handle->in_use, free_entry);
    handle->ssegs[key_length].use_count++;
    *allocated_entry = free_entry;
    handle->use_count++;

    if (_debug) {
	(void) taps_tcam_subsegment_dump(unit, handle, handle->taps->segment, -1);
    }

    return rv;    
}

/*
 *   Function
 *      taps_tcam_entry_free
 *   Purpose
 *      Internal function. enqueue tmu command to free a tcam entry
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) entry  : entry to be freed
 *   Returns
 *       SOC_E_NONE - successfully freed
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_entry_free(int unit, taps_tcam_handle_t handle,
				 taps_wgroup_handle_t *wgroup,
				 int entry)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_tmu_cmd_t *rpb_write;
    int key_length;
    int hw_instance;
    int hw_tcam_entry;
    int hw_bucket_id;
    /*=== sanity check */
    if ((handle == NULL) || (entry < 0) || (entry >= handle->size)) {
	return SOC_E_PARAM;
    }

    if ((handle->map[entry] == NULL) || _debug) {
	if (_debug) {
	    /* debug mode, don't issue any hardware command */
	    if (SHR_BITGET(handle->in_use, entry) == 0) {
		return SOC_E_PARAM;
	    }

	    SHR_BITCLR(handle->in_use, entry);
	    handle->map[entry] = NULL;
	    for (key_length = 0;
		 key_length < TAPS_TCAM_NUM_SSEGS(handle->key_size);
		 key_length++) {
		if ((entry >= handle->ssegs[key_length].start) &&
		    (entry < (handle->ssegs[key_length].start + handle->ssegs[key_length].size))) {
		    break;
		}
	    }
	    if (key_length < TAPS_TCAM_NUM_SSEGS(handle->key_size)) {
		handle->ssegs[key_length].use_count--;
		(void) taps_tcam_subsegment_dump(unit, handle, handle->taps->segment, -1);
		handle->use_count--;
		return SOC_E_NONE;
	    } else {
		return SOC_E_PARAM;
	    }
	} else {
	    return SOC_E_PARAM;
	}
    } else {
	key_length = handle->map[entry]->length;
    }

    if (SHR_BITGET(handle->in_use, entry) == 0) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Tcam Segment %d entry %d is already free\n"),
                   FUNCTION_NAME(), unit, handle->segment, entry));
	return SOC_E_PARAM;
    }

    /*=== construct TAPS_TCAM_WRITE command and add it to the work queue to
     *    mark TCAM entry as invalid.
     *    cmd.tapsi           --> handle->tapsi
     *    cmd.blk             --> 1
     *    cmd.opcode          --> 0x2 (write)
     *    cmd.target          --> 0x7 (TCAM, TDM and ADS)
     *    cmd.seg             --> handle->segment
     *    cmd.offset          --> entry
     *    cmd.bpm_length      --> 0 (don't care on invalid entry)
     *    cmd.kshift          --> 0 (no key shifting)
     *    cmd.bucket          --> 0 (don't care on invalid entry)
     *    cmd.best_match      --> 0 (don't care on invalid entry)
     *    cmd.a               --> 0 (don't care on invalid entry)
     *    cmd.g               --> 0 (no MSB wildcard)
     *    cmd.plength         --> 0xFF (will invalidate the tcam entry)
     *    cmd.pdata           --> 0 (don't care on invalid entry)
     */
    rv = tmu_cmd_alloc(unit, &rpb_write);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	return rv;
    }

	/* Get hardware instance and entry id */
    rv = taps_instance_and_entry_id_get(unit, handle->taps, 
                                    entry, 0,
                                    &hw_instance, &hw_tcam_entry, &hw_bucket_id);
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to get hardware instance and entry id\n"),
                   FUNCTION_NAME(), unit));
    	return rv;
    }
    
    rpb_write->opcode = SOC_SBX_TMU_CMD_TAPS;
    rpb_write->cmd.taps.instance = hw_instance;
    rpb_write->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    rpb_write->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE;
    rpb_write->cmd.taps.max_key_size = handle->key_size;
    rpb_write->cmd.taps.subcmd.rpb_write.segment = handle->segment;
    rpb_write->cmd.taps.subcmd.rpb_write.offset = hw_tcam_entry;
    rpb_write->cmd.taps.subcmd.rpb_write.target = SOC_SBX_TMU_TAPS_RPB_ALL;
    rpb_write->cmd.taps.subcmd.rpb_write.bpm_length = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.kshift = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.bucket = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.best_match = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.align_right = FALSE;
    rpb_write->cmd.taps.subcmd.rpb_write.length = _TAPS_INVALIDATE_PFX_LEN_;
    rpb_write->cmd.taps.subcmd.rpb_write.key[0] = 0;
    rpb_write->cmd.taps.subcmd.rpb_write.key[1] = 0;
    if (TAPS_IS_IPV6(handle)) {
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[2] = 0;
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[3] = 0;
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[4] = 0;
    }

    rv = taps_command_enqueue_for_all_devices(unit, handle->taps, wgroup, TAPS_TCAM_WORK, rpb_write);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	tmu_cmd_printf(unit, rpb_write);
	tmu_cmd_free(unit, rpb_write);
	return rv;
    } else {
	LOG_DEBUG(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "%s tcam invalidate entry %d.\n"),
	           FUNCTION_NAME(), entry));
    }

    /*=== mark the entry as free at in_use dbase */
    SHR_BITCLR(handle->in_use, entry);
    handle->map[entry] = NULL;
    handle->ssegs[key_length].use_count--;
    handle->use_count--;

    return rv;
}

/*
 *   Function
 *      taps_tcam_entry_bitmap_clear
 *   Purpose
 *      Clear the tcam entry from the bitmap of tcam_handle, also decrease the use_count;
 */
int taps_tcam_entry_bitmap_clear(int unit, taps_tcam_handle_t handle,
				 unsigned int length, int entry)
{
    if (SHR_BITGET(handle->in_use, entry)) {
        SHR_BITCLR(handle->in_use, entry); 
        handle->map[entry] = NULL;
        handle->ssegs[length].use_count--;
        handle->use_count--;
    }
    
    return SOC_E_NONE;
}

/*
 *   Function
 *      _taps_tcam_entry_move
 *   Purpose
 *      Internal function. enqueue tmu command to move a tcam entry to a different location
 *      Handle the in_use mask and map dbase, no maintainance of the subsegments dbase.
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) candidate_entry : entry to be moved
 *      (IN) free_entry      : entry to move to
 *   Returns
 *       SOC_E_NONE - successfully moved
 *       SOC_E_* as appropriate otherwise
 */
static int _taps_tcam_entry_move(int unit, taps_tcam_handle_t handle,
				 taps_wgroup_handle_t *wgroup,
				 int candidate_entry, int free_entry)
{
    int rv = SOC_E_NONE;
    int hw_instance;
    int hw_tcam_entry;
    int hw_bucket_id;
    taps_tcam_pivot_handle_t tph = NULL;
    uint32 bpm_length, bpm_global_index;
    uint32 key[TAPS_MAX_KEY_SIZE_WORDS], length;
    soc_sbx_caladan3_tmu_cmd_t *rpb_invalidate;
    soc_sbx_caladan3_tmu_cmd_t *rpb_write;

    /*=== parameter checks */
    if ((candidate_entry < 0) || (candidate_entry > handle->size) ||
	(free_entry < 0) || (free_entry > handle->size)) {
	return SOC_E_PARAM;
    }

    if ((candidate_entry == free_entry) ||
	(SHR_BITGET(handle->in_use, candidate_entry) == 0)) {
	/* candidate_entry and free_entry are same, or candidate_entry itself is free, do nothing */
	return SOC_E_NONE;
    }

    /* For unified mode, if free_entry and candidate_entry are in different instance,
        * then move the whole domain from old instance to new instance. 
        */
    if (!_TAPS_IS_PARALLEL_MODE_(handle->taps->param.instance)
        && !_ENTRY_IN_SAME_INSTANCE(candidate_entry, free_entry, 
                        handle->taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry)) {
        /* Move the whole domain */
        rv = taps_domain_move(unit, handle, wgroup, candidate_entry, free_entry);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to move domain %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            return rv;
        } 
    }

    /*=== construct TAPS_TCAM_WRITE command to write what's in candidate_entry to free_entry
     *    cmd.tapsi           --> handle->tapsi
     *    cmd.blk             --> 1
     *    cmd.opcode          --> 0x2 (write)
     *    cmd.target          --> 0x7 (TCAM, TDM and ADS)
     *    cmd.seg             --> handle->segment
     *    cmd.offset          --> free_entry
     *    cmd.bpm_length      --> perform bpm search on the handle->map[candidate_entry]->prefix.key/length
     *    cmd.kshift          --> 0 (offchip mode no key shifting)
     *                            --> length (onchip mode shift length bits)
     *    cmd.bucket          --> handle->map[candidate_entry]->sbucket->domain
     *    cmd.best_match      --> once bpm_length is known, perform exact match on bpm_key/bpm_length to get global_index
     *    cmd.a               --> ?? (what does left align/right align means??)
     *    cmd.g               --> 0 (no MSB wildcard)
     *    cmd.plength         --> handle->map[candidate_entry]->length
     *    cmd.pdata           --> handle->map[candidate_entry]->key
     */
    tph = handle->map[candidate_entry];
    if ((tph == NULL) || _debug) {
	if (_debug) {
	    /* in debug mode, don't do any real hardware commands */
	    SHR_BITCLR(handle->in_use, candidate_entry);
	    SHR_BITSET(handle->in_use, free_entry);
	    handle->map[free_entry] = handle->map[candidate_entry];
	    handle->map[free_entry]->entry = free_entry;
	    handle->map[candidate_entry] = NULL;
	    
	    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META_U(unit,
                                    "%s: unit %d Tcam Segment %d moved entry %d to entry %d\n"),
                         FUNCTION_NAME(), unit, handle->segment, candidate_entry, free_entry));
	    return SOC_E_NONE;
	} else {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Tcam Segment %d move candidate entry %d invalid!!!\n"),
                       FUNCTION_NAME(), unit, handle->segment, candidate_entry));
	    return SOC_E_PARAM;
	}
    }

    /* prepare the search key/length */
    length = tph->length;
    key[0] = tph->key[0];
    key[1] = tph->key[1];
    if (TAPS_IS_IPV6(handle)) {
	/* coverity[overrun-local : FALSE] */
	key[2] = tph->key[2];
	/* coverity[overrun-local : FALSE] */
	key[3] = tph->key[3];
	/* coverity[overrun-local : FALSE] */
	key[4] = tph->key[4];
    }

    /* find the bpm info */
    rv = _taps_tcam_find_bpm(unit, handle, key, length, 
                    &bpm_length, &bpm_global_index);
    if (SOC_FAILURE(rv)) {
	/* we should always be able to find a bpm here unless something internal is wrong */
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to find BPM info for key\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	(void)taps_show_prefix(handle->key_size, key, length);
	return rv;	
    }

    /* enqueue the INVALIDATE command */
    /* to safely write the tcam entry while it's passing traffic (even though there is
     * a duplicate tcam entry from previous move, it's possible the old entry is still being
     * used due to the entry at higher tcam priority), we need to invalidate this tcam entry
     * first to ensure key lookup doesn't use the ADS of the new tcam entry (because hardware
     * might write ADS before write the key/mask)
     */
    rv = tmu_cmd_alloc(unit, &rpb_invalidate);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	return rv;
    }	

	/* Get hardware instance and entry id */
    rv = taps_instance_and_entry_id_get(unit, handle->taps, 
                                    free_entry, tph->sbucket->domain,
                                    &hw_instance, &hw_tcam_entry, &hw_bucket_id);
    if (SOC_FAILURE(rv)) {
    	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to get hardware instance and entry id\n"),
                   FUNCTION_NAME(), unit));
    	return rv;
    }
    rpb_invalidate->opcode = SOC_SBX_TMU_CMD_TAPS;
    rpb_invalidate->cmd.taps.instance = hw_instance;
    rpb_invalidate->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    rpb_invalidate->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE;
    rpb_invalidate->cmd.taps.max_key_size = handle->key_size;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.segment = handle->segment;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.offset = hw_tcam_entry;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.target = SOC_SBX_TMU_TAPS_RPB_ALL;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.bpm_length = 0;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.kshift = 0;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.bucket = 0;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.best_match = 0;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.align_right = FALSE;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.length = _TAPS_INVALIDATE_PFX_LEN_;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.key[0] = 0;
    rpb_invalidate->cmd.taps.subcmd.rpb_write.key[1] = 0;
    if (TAPS_IS_IPV6(handle)) {
	/* coverity[overrun-local : FALSE] */
	rpb_invalidate->cmd.taps.subcmd.rpb_write.key[2] = 0;
	/* coverity[overrun-local : FALSE] */
	rpb_invalidate->cmd.taps.subcmd.rpb_write.key[3] = 0;
	/* coverity[overrun-local : FALSE] */
	rpb_invalidate->cmd.taps.subcmd.rpb_write.key[4] = 0;
    }    

    rv = taps_command_enqueue_for_all_devices(unit, handle->taps, wgroup, TAPS_TCAM_WORK, rpb_invalidate);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	tmu_cmd_printf(unit, rpb_invalidate);
	tmu_cmd_free(unit, rpb_invalidate);
	return rv;
    } else {
	LOG_DEBUG(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "%s tcam invalidate entry %d.\n"),
	           FUNCTION_NAME(), free_entry));
    }

    /* enqueue the WRITE command */
    rv = tmu_cmd_alloc(unit, &rpb_write);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	tmu_cmd_free(unit, rpb_invalidate);
	return rv;
    }	

    rpb_write->opcode = SOC_SBX_TMU_CMD_TAPS;
    rpb_write->cmd.taps.instance = hw_instance;
    rpb_write->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    rpb_write->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE;
    rpb_write->cmd.taps.max_key_size = handle->key_size;
    rpb_write->cmd.taps.subcmd.rpb_write.segment = handle->segment;
    rpb_write->cmd.taps.subcmd.rpb_write.offset = hw_tcam_entry;
    rpb_write->cmd.taps.subcmd.rpb_write.target = SOC_SBX_TMU_TAPS_RPB_ALL;
    rpb_write->cmd.taps.subcmd.rpb_write.bpm_length = bpm_length;
    if (handle->taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* Don't need to do key shift for offchip mode*/
        rpb_write->cmd.taps.subcmd.rpb_write.kshift = 0;
        rpb_write->cmd.taps.subcmd.rpb_write.length = length;
    } else {
        rpb_write->cmd.taps.subcmd.rpb_write.kshift = tph->sbucket->tcam_shift_len;
        rpb_write->cmd.taps.subcmd.rpb_write.length = tph->sbucket->tcam_shift_len;
    }
    rpb_write->cmd.taps.subcmd.rpb_write.bucket = hw_bucket_id;
    rpb_write->cmd.taps.subcmd.rpb_write.best_match = bpm_global_index;
    rpb_write->cmd.taps.subcmd.rpb_write.align_right = FALSE;
    
    rpb_write->cmd.taps.subcmd.rpb_write.key[0] = key[0];
    rpb_write->cmd.taps.subcmd.rpb_write.key[1] = key[1];
    if (TAPS_IS_IPV6(handle)) {
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[2] = key[2];
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[3] = key[3];
	/* coverity[overrun-local : FALSE] */
	rpb_write->cmd.taps.subcmd.rpb_write.key[4] = key[4];
    }    
    
    rv = taps_command_enqueue_for_all_devices(unit, handle->taps, wgroup, TAPS_TCAM_WORK, rpb_write);
    if (SOC_FAILURE(rv)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to enqueue command to work queue\n"),
                   FUNCTION_NAME(), unit, handle->segment));
	tmu_cmd_printf(unit, rpb_write);
	tmu_cmd_free(unit, rpb_invalidate);
	tmu_cmd_free(unit, rpb_write);
	return rv;
    } else {
	LOG_DEBUG(BSL_LS_SOC_COMMON,
	          (BSL_META_U(unit,
	                      "%s tcam insert entry %d with bucket %d bpm_length %d global_index %d.\n"),
	           FUNCTION_NAME(), free_entry, tph->sbucket->domain, bpm_length, bpm_global_index));
    }

    /*=== mark the candidate_entry as free, and the old free_entry as in use */
    SHR_BITCLR(handle->in_use, candidate_entry);
    SHR_BITSET(handle->in_use, free_entry);
    handle->map[free_entry] = handle->map[candidate_entry];
    handle->map[free_entry]->entry = free_entry;
    handle->map[candidate_entry] = NULL;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s: unit %d Tcam Segment %d moved entry %d to entry %d\n"),
                 FUNCTION_NAME(), unit, handle->segment, candidate_entry, free_entry));
    return rv;
}

int _taps_tcam_traverse(trie_node_t *payload, void *trav_datum)
{
    taps_tcam_pivot_handle_t  tph;
    int rv=SOC_E_NONE;
    tcam_traverse_datum_t *datum = NULL;

    if (!payload || !trav_datum) return SOC_E_PARAM;
    if (payload->type != PAYLOAD) return SOC_E_NONE;

    datum = (tcam_traverse_datum_t*)trav_datum;
    tph = TRIE_ELEMENT_GET(taps_tcam_pivot_handle_t, payload, node);

    if (datum->tpivots == NULL) {
	/* no accumulation, call back directly */
	if (datum->user_cb != NULL) {
	    rv = datum->user_cb(tph, datum->user_data);
	}
    } else {
	/* accumulate and call back after traverse is done */
	datum->tpivots[datum->num_tpivots++] = tph;
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_tcam_traverse
 * Purpose:
 *     Traverse the tcam database & invoke call back
 */
int taps_tcam_traverse(int unit, 
                       const taps_handle_t taps,
                       const taps_tcam_handle_t handle,
                       void *user_data,
                       tcam_traverse_cb_f cb)
{
    int rv = SOC_E_NONE;
    tcam_traverse_datum_t datum;

    if (!handle || !cb) return SOC_E_PARAM;

    datum.user_data = user_data;
    datum.user_cb = cb;
    datum.num_tpivots = 0;
    datum.tpivots = NULL;
    
    rv = trie_traverse(handle->trie, _taps_tcam_traverse, &datum, _TRIE_PREORDER_TRAVERSE);
    return rv;
}

/*
 *
 * Function:
 *     taps_tcam_destroy_traverse
 * Purpose:
 *     Traverse the tcam database & invoke call back
 */
int taps_tcam_destroy_traverse(int unit, 
                       const taps_handle_t taps,
                       const taps_tcam_handle_t handle,
                       void *user_data,
                       tcam_traverse_cb_f cb)
{
    int rv = SOC_E_NONE, index;
    tcam_traverse_datum_t datum;

    if (!handle || !cb) return SOC_E_PARAM;

    datum.user_data = user_data;
    datum.user_cb = cb;
    datum.num_tpivots = 0;
    datum.tpivots = sal_alloc(sizeof(taps_tcam_pivot_handle_t) * (handle->size+1),
			      "taps-tcam-traverse");;
   
    rv = trie_traverse(handle->trie, _taps_tcam_traverse, &datum, _TRIE_POSTORDER_TRAVERSE);
    if (SOC_SUCCESS(rv)) {
	if (datum.num_tpivots <= handle->size) {
	    /* call back on all tph accumulated */
	    for (index = 0; index < datum.num_tpivots; index++) {
		rv = cb(datum.tpivots[index], user_data);
		if (SOC_FAILURE(rv)) {
		    break;
		}
	    }
	} else {
	    /* we should never get more than what dbucket can hold */
	    rv = SOC_E_INTERNAL;
	}
    }

    if (datum.tpivots) {
	sal_free(datum.tpivots);
    }

    return rv;
}

typedef struct _tcam_dump_datum_s {
    int unit;
    taps_tcam_handle_t handle;
    uint32 flags;
} tcam_dump_datum_t;

int _taps_trie_dump_cb(trie_node_t *node, void *user_data)

{
    int rv = SOC_E_NONE;

    if (node && user_data) {
        if (node->type == PAYLOAD) {
            tcam_dump_datum_t *datum = (tcam_dump_datum_t*)user_data;
            taps_tcam_pivot_handle_t tph = TRIE_ELEMENT_GET(taps_tcam_pivot_handle_t, node, node);
            /* dump trie pivot */
            taps_tcam_pivot_dump(datum->unit, datum->handle,
                                 tph, datum->flags);
        } else {
            LOG_CLI((BSL_META("+\n")));
        }
    }

    return rv;
}

/*
 *   Function
 *      taps_tcam_dump
 *   Purpose
 *      Dump tcam info
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) handle : tcam object handle
 *      (IN) flags  : flags
 *   Returns
 *       SOC_E_NONE - successfully dumped
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_dump(int unit, taps_tcam_t *handle, uint32 flags)
{
    int rv = SOC_E_NONE;
    int key_length, tcam_entry;
    tcam_dump_datum_t datum;

    /* dump the general info */
    if (!TAPS_DUMP_TCAM_FLAGS(flags)) {
	return SOC_E_NONE;
    }

    LOG_CLI((BSL_META_U(unit,
                        "%s:====================================================\n"),
             FUNCTION_NAME()));
    LOG_CLI((BSL_META_U(unit,
                        "%s: unit %d Tcam Segment %d Key size %d Tcam base %d Tcam Size %d\n"),
             FUNCTION_NAME(), unit, handle->segment, handle->key_size, handle->base/4, handle->size));
    LOG_CLI((BSL_META_U(unit,
                        "Total number of TCAM pivots: %d \n"), handle->trie->trie->count));
    LOG_CLI((BSL_META_U(unit,
                        "%s:====================================================\n"),
             FUNCTION_NAME()));

    sal_memset(&datum, 0, sizeof(tcam_dump_datum_t));
    datum.handle = handle;
    datum.unit = unit;
    datum.flags = flags;

    trie_dump(handle->trie, _taps_trie_dump_cb, &datum);
    
    if (flags & TAPS_DUMP_TCAM_VERB) {
        /* dump the tcam subsegment info */
        for (key_length = 0; key_length < (handle->key_size-1); key_length++) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: unit %d subsegment with key length %d, start %d, size %d use_count %d\n"),
                     FUNCTION_NAME(), unit, key_length, handle->ssegs[key_length].start,
                     handle->ssegs[key_length].size, handle->ssegs[key_length].use_count));
            
            for (tcam_entry = handle->ssegs[key_length].start;
                 tcam_entry < (handle->ssegs[key_length].start + handle->ssegs[key_length].size);
                 tcam_entry++) {
                if (SHR_BITGET(handle->in_use, tcam_entry)) {
            	/* make sure tcam entry in use is pointing to a Tcam entry object, and reverse mapping is consistent */
            	if ((handle->map[tcam_entry] == NULL) ||
            	    (handle->map[tcam_entry]->entry != tcam_entry) ||
            	    (handle->map[tcam_entry]->length != key_length)) {
            	    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d Tcam Segment %d subsegment %d entry %d dbase inconsistent !!!\n"),
                               FUNCTION_NAME(), unit, handle->segment, key_length, tcam_entry));
            	    return SOC_E_INTERNAL;
            	} else {
            	    LOG_CLI((BSL_META_U(unit,
                                        "%s: unit %d entry %d in use\n"), FUNCTION_NAME(), unit, tcam_entry));
            	}
                } else {
                    /* make sure tcam entry not in use is not pointing to a Tcam entry object */
                    if (handle->map[tcam_entry] != NULL) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d Tcam Segment %d subsegment %d invalid entry %d dbase inconsistent !!!\n"),
                                   FUNCTION_NAME(), unit, handle->segment, key_length, tcam_entry));
                        return SOC_E_INTERNAL;
                    }
                }
            }
        }
    }
    
    return rv;
}

/*
 *   Function
 *      taps_tcam_pivot_dump
 *   Purpose
 *      Dump a TCAM TPH software and hardware state 
 *   Parameters
 *      (IN) unit           : unit number of the device
 *      (IN) handle         : taps tcam object handle
 *      (IN) tph            : tph object handle
 *      (IN) flags          : flags
 *   Returns
 *       SOC_E_NONE - successfully dumped
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_pivot_dump(int unit, taps_tcam_handle_t handle,
			 taps_tcam_pivot_handle_t tph, 
                         uint32 flags)
{
    int rv = SOC_E_NONE;
    int hw_instance, hw_tcam_entry, hw_bucket_id;
    
    if (!handle || !tph) return SOC_E_PARAM;

    if ((flags & TAPS_DUMP_TCAM_SW_PIVOT) ||
        (flags & TAPS_DUMP_TCAM_HW_PIVOT)) {

        /* dump software info */
        LOG_CLI((BSL_META_U(unit,
                            "TTTTT TCAM pivot: %p dump TTTTT\n"), (void *)tph));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Dumping unit %d Taps handle 0x%x TPH handle 0x%x entry %d Associated Sbucket 0x%x\n"),
                 FUNCTION_NAME(), unit, (uint32)handle, (uint32)tph, (uint32)tph->entry,(uint32)tph->sbucket));
        LOG_CLI((BSL_META_U(unit,
                            "%s: trie_node skip_len %d skip_addr %d type %s count %d bpm 0x%08x\n"),
                 FUNCTION_NAME(), tph->node.skip_len, tph->node.skip_addr,
                 (tph->node.type==PAYLOAD)?"PAYLOAD":"INTERNAL", tph->node.count, tph->node.bpm));
        if (TAPS_IS_IPV4(handle)) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: key_length %d key_data 0x%04x %08x\n"),
                     FUNCTION_NAME(), tph->length, tph->key[0], tph->key[1]));
        } else {
	    /* coverity[overrun-local : FALSE] */
            LOG_CLI((BSL_META_U(unit,
                                "%s: key_length %d key_data 0x%04x %08x %08x %08x %08x\n"),
                     FUNCTION_NAME(), tph->length, tph->key[0], tph->key[1],
                     tph->key[2], tph->key[3], tph->key[4]));
        }
        
        /* Get hardware instance and entry id */
       rv = taps_instance_and_entry_id_get(unit, handle->taps, 
                                       tph->entry, 0, &hw_instance, &hw_tcam_entry, &hw_bucket_id);
       if (SOC_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d Failed to get hardware instance and entry id\n"),
                      FUNCTION_NAME(), unit));
           return rv;
       }
        if (flags & TAPS_DUMP_TCAM_HW_PIVOT) {
            /* dump the assocated hw entry */
            rv = taps_tcam_entry_dump(unit, hw_instance, handle->segment,
                                      hw_tcam_entry, flags);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to dump tcam hardware entry %d !!!\n"), 
                           FUNCTION_NAME(), unit, tph->entry));
                return rv;
            }
        }

        if (TAPS_DUMP_SRAM_FLAGS(flags)) {
            rv = taps_sbucket_dump(unit, handle->taps, tph->sbucket, flags);
        }
    }

    return rv;
}

/*
 *   Function
 *      taps_tcam_entry_dump
 *   Purpose
 *      Dump a TCAM hardware entry
 *   Parameters
 *      (IN) unit           : unit number of the device
 *      (IN) taps_instancd  : taps instance
 *      (IN) segment        : tcam segment
 *      (IN) entry          : tcam entry
 *      (IN) flags          : flags
 *   Returns
 *       SOC_E_NONE - successfully dumped
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_entry_dump(int unit, int taps_instance, int segment,
			 int entry, uint32 flags)
{
    int rv = SOC_E_NONE;
    int tapsi;
    soc_sbx_caladan3_tmu_cmd_t *rpb_read;
    uint32    mem_val[SOC_MAX_MEM_WORDS*2];

    /* read hw entry */
    rv = tmu_cmd_alloc(unit, &rpb_read);
    if (SOC_FAILURE(rv) || (rpb_read == NULL)) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d segment %d Failed to allocate command for work queue\n"),
                   FUNCTION_NAME(), unit, segment));
	return rv;
    }	
    rpb_read->opcode = SOC_SBX_TMU_CMD_TAPS;
    rpb_read->cmd.taps.instance = taps_instance & 0x3;
    rpb_read->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_RPB;
    rpb_read->cmd.taps.opcode = SOC_SBX_TMU_TAPS_RPB_SUBCMD_READ;
    rpb_read->cmd.taps.max_key_size = TAPS_IPV6_KEY_SIZE; /* we don't really use key size here, put in max */
    rpb_read->cmd.taps.subcmd.rpb_read.segment = segment;
    rpb_read->cmd.taps.subcmd.rpb_read.offset = entry;
    rpb_read->cmd.taps.subcmd.rpb_read.target = SOC_SBX_TMU_TAPS_RPB_ALL;

    /* post tmu command and process response buffer here */
    sal_memset(mem_val, 0, sizeof(mem_val));
    TMU_LOCK(unit);
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                       rpb_read, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
	rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
					   rpb_read, mem_val, SOC_MAX_MEM_WORDS*2);
    TMU_UNLOCK(unit);
	if (SOC_FAILURE(rv)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Invalid response !!!\n"), 
                       FUNCTION_NAME(), unit));
	    return rv;
	}
    } else {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to post TAPS RPB read command !!!\n"), 
                   FUNCTION_NAME(), unit));
	return rv;
    }

    /* for now, assume that we only target 1 tap instance in our command
     * so there is only 1 response back
     */
    if (SAL_BOOT_PLISIM) {
	tapsi = rpb_read->cmd.taps.instance;
    } else {
	tapsi = soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, TAPSIf);
    }
    if ((tapsi != 1) && (tapsi != 2)) {
	/* NOTE: response should not be from 2 instances, check with hardware team */
	return SOC_E_UNAVAIL;
    }

    LOG_CLI((BSL_META_U(unit,
                        "%s: Dumping unit %d Taps %d Tcam Segment %d entry %d\n"),
             FUNCTION_NAME(), unit, (tapsi-1), segment, entry));
    LOG_CLI((BSL_META_U(unit,
                        "%s: bpm_length %d kshift %d bucket %d best_match %d align_right %d\n"),
             FUNCTION_NAME(),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, BPM_LENGTHf),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, KSHIFTf),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, BUCKETf),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, BEST_MATCHf),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, ALIGN_RIGHTf)));
    LOG_CLI((BSL_META_U(unit,
                        "%s: key_length %d key_data 0x%04x %08x %08x %08x %08x\n"),
             FUNCTION_NAME(),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, PLENGTHf),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, PDATA143_128f),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, PDATA127_96f),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, PDATA95_64f),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, PDATA63_32f),
             soc_mem_field32_get(unit, TAPS_RPB_CMD_WRITEm, mem_val, PDATA31_0f)));

    tmu_cmd_free(unit, rpb_read);

    return rv;
}

int taps_tcam_entry_range_dump(int unit, int taps_instance, int segment,
			       int start_entry, int end_entry, uint32 flags)
{
    int entry, rv = SOC_E_NONE;
    for (entry = start_entry; entry <= end_entry; entry++) {
	rv = taps_tcam_entry_dump(unit, taps_instance, segment, entry, flags);
	if (SOC_FAILURE(rv)) {
	    break;
	}
    }
    return rv;
}

/*
 *   Function
 *      taps_tcam_subsegment_dump
 *   Purpose
 *      Dump a TCAM subsegment info
 *   Parameters
 *      (IN) unit        : unit number of the device
 *      (IN) taps        : taps object handle
 *      (IN) subsegment  : subsegment to be dumpped, -1 means dump all
 *   Returns
 *       SOC_E_NONE - successfully dumped
 *       SOC_E_* as appropriate otherwise
 */
int taps_tcam_subsegment_dump(int unit, const taps_tcam_handle_t handle,
			      int segment, int subsegment)
{
    int rv = SOC_E_NONE;
    int ss, ss_start, ss_end;

    if (!handle->ssegs) return SOC_E_PARAM;

    if ((subsegment >= 0) && subsegment <   TAPS_TCAM_NUM_SSEGS(handle->key_size)) {
	ss_start = subsegment;
	ss_end = subsegment+1;
    } else {
	ss_start = 0;
	ss_end = TAPS_TCAM_NUM_SSEGS(handle->key_size);
    }

    LOG_CLI((BSL_META_U(unit,
                        "%s: Dumping unit %d Taps %d Tcam Segment %d subsegment info\n"),
             FUNCTION_NAME(), unit, (handle->tapsi-1), handle->segment)); 
    for (ss = ss_start; ss < ss_end; ss++) {
	LOG_CLI((BSL_META_U(unit,
                            "%s: subsegment %d start %d size %d use_count %d\n"), 
                 FUNCTION_NAME(), ss, handle->ssegs[ss].start,
                 handle->ssegs[ss].size, handle->ssegs[ss].use_count));
    }

    
    
    return rv;
}

/*
 * Tcam module unit test.
 */
int taps_tcam_ut(int unit, const taps_handle_t taps, int test)
{
    int rv = SOC_E_NONE;
    int ss, entry, tcam_entry;
    taps_wgroup_handle_t wgroup = NULL;
    taps_tcam_pivot_handle_t ntph = NULL;

    rv = taps_work_group_create(unit, taps->wqueue[unit], 
                                _TAPS_DEFAULT_WGROUP_,
                                &wgroup);
    if (SOC_FAILURE(rv)) {
	return rv;
    }

    switch (test) {
	case 1:
	    /* TCam arbitration test */
	    _debug = TRUE;

	    /* Case 1: allocate upto 5 entries for each subsegment */
	    for (ss = 0; ss < TAPS_TCAM_NUM_SSEGS(taps->tcam_hdl->key_size); ss++) {
		for (entry = 0;
		     ((entry < (1<<ss)) && (entry < 5));
		     entry++) {
		    rv = hpcm_alloc_payload(unit, taps->tcam_hdl->pivots_hpcm, (void **)&ntph);
		    if (SOC_FAILURE(rv)) {	
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to allocate pivot for tcam segment %d\n"),
                                   FUNCTION_NAME(), unit, taps->tcam_hdl->segment));
			return rv;
		    }

		    rv = taps_tcam_entry_alloc(unit, taps->tcam_hdl, &wgroup, ss, &tcam_entry);
		    if (SOC_FAILURE(rv)) {
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "%s: unit %d failed to allocated tcam entry for length %d.\n"),
                                   FUNCTION_NAME(), unit, ss));
			return rv;
		    }	

		    (ntph)->entry = tcam_entry;
		    (ntph)->length = ss;
		    taps->tcam_hdl->map[tcam_entry] = ntph;	   
		}
	    }

	    /* allocate some from 1 subsegment */
	    ss= 18;
	    for (entry = 0; entry < 100; entry++) {
		rv = hpcm_alloc_payload(unit, taps->tcam_hdl->pivots_hpcm, (void **)&ntph);
		if (SOC_FAILURE(rv)) {	
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate pivot for tcam segment %d\n"),
                               FUNCTION_NAME(), unit, taps->tcam_hdl->segment));
		    return rv;
		}
		
		rv = taps_tcam_entry_alloc(unit, taps->tcam_hdl, &wgroup, ss, &tcam_entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocated tcam entry for length %d.\n"),
                               FUNCTION_NAME(), unit, ss));
		    return rv;
		}	
		(ntph)->entry = tcam_entry;
		(ntph)->length = ss;
		taps->tcam_hdl->map[tcam_entry] = ntph;
	    }

	    /* free all entries, including 0 */
	    for (entry = 0;
		 entry < taps->param.seg_attr.seginfo[taps->param.instance].num_entry;
		 entry++) {		
		if (SHR_BITGET(taps->tcam_hdl->in_use, entry) == 0) {
		    continue;
		}
		rv = taps_tcam_entry_free(unit, taps->tcam_hdl, &wgroup, entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free tcam entry %d.\n"),
                               FUNCTION_NAME(), unit, entry));
		    return rv;
		}		
	    }

	    /* Case 2: allocate all from the same subsegment, NOTE that * entry has been allocated
	     * at init time.
	     */
	    ss = 41;
	    for (entry = 0;
		 /* entry < (taps->param.seg_attr.seginfo[taps->param.instance].num_entry-1); */
		 entry < 500;
		 entry++) {
		rv = hpcm_alloc_payload(unit, taps->tcam_hdl->pivots_hpcm, (void **)&ntph);
		if (SOC_FAILURE(rv)) {	
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocate pivot for tcam segment %d\n"),
                               FUNCTION_NAME(), unit, taps->tcam_hdl->segment));
		    return rv;
		}

		rv = taps_tcam_entry_alloc(unit, taps->tcam_hdl, &wgroup, ss, &tcam_entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocated tcam entry for length %d.\n"),
                               FUNCTION_NAME(), unit, ss));
		    return rv;
		}

		(ntph)->entry = tcam_entry;
		(ntph)->length = ss;		
		taps->tcam_hdl->map[tcam_entry] = ntph;
	    }

	    /* dump it */
	    rv = taps_tcam_dump(unit, taps->tcam_hdl, 0);
	    if (SOC_FAILURE(rv)) {
		LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d failed to dump tcam.\n"),
                           FUNCTION_NAME(), unit));
		return rv;
	    }

	    /* free all entries, including 0 */
	    for (entry = 0;
		 entry < taps->param.seg_attr.seginfo[taps->param.instance].num_entry;
		 entry++) {		
		if (SHR_BITGET(taps->tcam_hdl->in_use, entry) == 0) {
		    continue;
		}
		rv = taps_tcam_entry_free(unit, taps->tcam_hdl, &wgroup, entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free tcam entry %d.\n"),
                               FUNCTION_NAME(), unit, entry));
		    return rv;
		}		
	    }

	    /* allocate for a different subsegment */
	    ss = 17;
	    for (entry = 0;
		 entry < (taps->param.seg_attr.seginfo[taps->param.instance].num_entry-1);
		 entry++) {
		rv = taps_tcam_entry_alloc(unit, taps->tcam_hdl, &wgroup, ss, &tcam_entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to allocated tcam entry for length %d.\n"),
                               FUNCTION_NAME(), unit, ss));
		    return rv;
		}
	    }

	    /* free all in use entries */
	    for (entry = 0;
		 entry < taps->param.seg_attr.seginfo[taps->param.instance].num_entry;
		 entry++) {		
		if (SHR_BITGET(taps->tcam_hdl->in_use, entry) == 0) {
		    continue;
		}

		rv = taps_tcam_entry_free(unit, taps->tcam_hdl, &wgroup, entry);
		if (SOC_FAILURE(rv)) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to free tcam entry %d.\n"),
                               FUNCTION_NAME(), unit, entry));
		    return rv;
		}		
	    }

	    _debug = FALSE;
	    break;
	default :
	    rv = SOC_E_UNAVAIL;
    }

    if (wgroup) {
	taps_work_group_destroy(unit, wgroup);
    }

    return rv;
}

#endif /* BCM_CALADAN3_SUPPORT */

