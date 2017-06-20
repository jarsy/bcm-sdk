/*
 * $Id: sbucket.c,v 1.72.14.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    sbucket.c
 * Purpose: Caladan3 TAPS sram bucket driver
 * Requires:
 */
#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/tmu/taps/taps.h>
#include <soc/sbx/caladan3/tmu/taps/tcam.h>
#include <soc/sbx/caladan3/tmu/taps/sbucket.h>
#include <soc/sbx/caladan3/tmu/taps/dbucket.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <soc/sbx/caladan3/tmu/taps/taps_util.h>


/* Sram bucket handle object & interfaces 
 * SBH is a container of Sram pivots. 
 */

typedef struct _sbucket_common_datum_s {
    /* common for propagation / clone */
    int unit;
    const taps_wgroup_handle_t *wgroup;
    taps_handle_t taps;
} sbucket_common_datum_t;

typedef struct _sbucket_clone_datum_s {
    sbucket_common_datum_t common;
    taps_sbucket_handle_t sbh;
    taps_sbucket_handle_t nsbh;
} sbucket_clone_datum_t;

typedef struct _sbucket_propagate_datum_s {
    sbucket_common_datum_t common;
    unsigned int ads_local_pointer;
    uint8 *isbpm;
    unsigned int add;
} sbucket_propagate_datum_t;

typedef struct _sbucket_traverse_datum_s {
    sbucket_common_datum_t common;
    void *user_data;
    sbucket_traverse_cb_f user_cb;
    taps_spivot_handle_t *spivots;
    int num_spivots;
} sbucket_traverse_datum_t;

static taps_work_type_e_t work_type[] = {TAPS_SBUCKET_WORK};

int _hpcm_sbucket_alloc(int unit, taps_sbucket_handle_t *handle)
{
    int rv = SOC_E_NONE;
    *handle = sal_alloc(sizeof(taps_sbucket_t), "taps-sbucket");
    if (*handle == NULL) {
        rv = SOC_E_MEMORY;
    } else {
        sal_memset(*handle, 0, sizeof(taps_sbucket_t));
    }
    return rv;
}

int _hpcm_sbucket_free(int unit, taps_sbucket_handle_t handle)
{
    int rv = SOC_E_NONE;
    sal_free(handle);
    return rv;
}

int _hpcm_spivot_alloc(int unit, unsigned int max_key_length, taps_spivot_handle_t *handle)
{
    int rv = SOC_E_NONE;
    *handle = sal_alloc(sizeof(taps_spivot_t) + 
                        BITS2WORDS(max_key_length) * sizeof(unsigned int), 
                        "taps-sbucket");
    if (*handle == NULL) {
        rv = SOC_E_MEMORY;
    } else {
        sal_memset(*handle, 0, 
                   sizeof(taps_spivot_t) + 
                   BITS2WORDS(max_key_length) * sizeof(unsigned int));
    }
    return rv;
}

int _hpcm_spivot_free(int unit, taps_spivot_handle_t handle)
{
    int rv = SOC_E_NONE;
    sal_free(handle);
    return rv;
}

int taps_sbucket_find_bpm(int unit, 
                          const taps_handle_t taps, 
                          taps_sbucket_handle_t sbh,
                          uint32 *key, uint32 length,
                          uint32 *bpm_length)
{
    int rv = SOC_E_NONE;

    if (!taps || !sbh || !key || !bpm_length) return SOC_E_PARAM;

    rv = trie_find_prefix_bpm(sbh->trie, key, length, bpm_length);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d Failed"
                          " to find BPM for following key\n"),
               FUNCTION_NAME(), unit));
    (void)taps_show_prefix(taps->param.key_attr.lookup_length, key, length);
    return rv;  
    }

    return rv;
}

static int _taps_sbucket_get_bpm_pointer(int unit, const taps_handle_t taps,
                                         uint32 *key, uint32 length, 
                                         uint32 bpm_length, uint32 *bpm_local_index)
{
    int rv = SOC_E_NONE, word_idx;    
    uint32 bpm_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)];

    if (!taps || !key || !bpm_local_index) return SOC_E_PARAM;

    for (word_idx = 0; word_idx < BITS2WORDS(taps->param.key_attr.lookup_length); word_idx++) {
    bpm_key[word_idx]=key[word_idx];
    }

    /* right shift key to construct bpm_key */
    rv = taps_key_shift(taps->param.key_attr.lookup_length, bpm_key, length, length-bpm_length);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d Failed to construct BPM key\n"),
               FUNCTION_NAME(), unit));
    return rv;  
    }   

    /* find the global_index of the bpm_key */
    rv = taps_find_prefix_local_pointer(unit, taps, bpm_key, bpm_length, bpm_local_index);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d Failed to find global_index for BPM key\n"),
               FUNCTION_NAME(), unit));
    return rv;  
    }

    return rv;
}

int _taps_sbucket_find_bpm(int unit, const taps_handle_t taps, 
                                  taps_sbucket_handle_t sbh,
                                  uint32 *key, uint32 length,
                                  uint32 *bpm_length, uint32 *bpm_local_index)
{
    int rv = SOC_E_NONE;

    if (!taps || !sbh || !key || !bpm_length || !bpm_local_index) return SOC_E_PARAM;

    rv = trie_find_prefix_bpm(sbh->trie, key, length, bpm_length);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d Failed"
                          " to find BPM for following key\n"),
               FUNCTION_NAME(), unit));
    (void)taps_show_prefix(taps->param.key_attr.lookup_length, key, length);
    return rv;  
    }

    if (*bpm_length > 0) {
        rv = _taps_sbucket_get_bpm_pointer(unit, taps, key, length, 
                                           *bpm_length, bpm_local_index);
    } else {
        *bpm_local_index  = _BRR_INVALID_CPE_;
    }

    return rv;
}
/* allocate and enqueue work item for BBx & BRR update for given pivot */
int taps_sbucket_enqueue_update_assodata_work(int unit, 
                                         const taps_handle_t taps,
                                         const taps_sbucket_handle_t sbh,
                                         const taps_spivot_handle_t sph,
                                         const taps_wgroup_handle_t *wgroup,
                                         unsigned int asso_data)
{
    soc_sbx_caladan3_tmu_cmd_t *brr=NULL;
    int rv = SOC_E_NONE;
    int hw_instance = 0;
    int hw_bucket_id = 0;
    
    if (!taps || !sbh || !sph || 
        !wgroup || asso_data > _BRR_INVALID_CPE_) return SOC_E_PARAM;

    sph->payload[0] = asso_data;
    /* Get hardware instance and bucket id */
    rv = taps_sbucket_instance_and_bucket_id_get(unit, taps, 
                                    sbh->domain, &hw_instance, &hw_bucket_id);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to get hardware instance and bucket id\n"),
                   FUNCTION_NAME(), unit));
        return rv;
    }

    rv = tmu_cmd_alloc(unit, &brr); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate internal BRR update commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    
    if (SOC_SUCCESS(rv)) {
        /* Enqueue work items for BRR updates */
        brr->opcode = SOC_SBX_TMU_CMD_TAPS;
        brr->cmd.taps.instance = hw_instance;
        brr->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BRR;
        brr->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE;
        brr->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
        brr->cmd.taps.subcmd.brr_write.segment = taps->segment;
        brr->cmd.taps.subcmd.brr_write.offset = hw_bucket_id;
        brr->cmd.taps.subcmd.brr_write.prefix_id = sph->index;
        brr->cmd.taps.subcmd.brr_write.format = sbh->sbucket_format;
        brr->cmd.taps.subcmd.brr_write.adata = asso_data;

        rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_SBUCKET_WORK, brr);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue brr work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            tmu_cmd_free(unit, brr);
        } else {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s sbucket local index update for domain %d index %d with local index %d.\n"),
                   FUNCTION_NAME(), sbh->domain, sph->index, asso_data));
        }

    }
    return rv;
}

/* allocate and enqueue work item for BBx & BRR update for given pivot */
int _taps_sbucket_enqueue_propagate_work(int unit, 
                                         const taps_handle_t taps,
                                         const taps_sbucket_handle_t sbh,
                                         const taps_spivot_handle_t sph,
                                         const taps_wgroup_handle_t *wgroup,
                                         unsigned int ads_local_pointer /* bpm local pointer */)
{
    soc_sbx_caladan3_tmu_cmd_t *brr=NULL;
    int rv = SOC_E_NONE;
    int hw_instance = 0;
    int hw_bucket_id = 0;
    
    if (!taps || !sbh || !sph || 
        !wgroup || ads_local_pointer > _BRR_INVALID_CPE_) return SOC_E_PARAM;

    /* Get hardware instance and bucket id */
    rv = taps_sbucket_instance_and_bucket_id_get(unit, taps, 
                                    sbh->domain, &hw_instance, &hw_bucket_id);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to get hardware instance and bucket id\n"),
                   FUNCTION_NAME(), unit));
        return rv;
    }

    rv = tmu_cmd_alloc(unit, &brr); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate internal BRR update commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    if (SOC_SUCCESS(rv)) {
        /* Enqueue work items for BRR updates */
        brr->opcode = SOC_SBX_TMU_CMD_TAPS;
        brr->cmd.taps.instance = hw_instance;
        brr->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BRR;
        brr->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE;
        brr->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
        brr->cmd.taps.subcmd.brr_write.segment = taps->segment;
        brr->cmd.taps.subcmd.brr_write.offset = hw_bucket_id;
        brr->cmd.taps.subcmd.brr_write.prefix_id = sph->index;
        brr->cmd.taps.subcmd.brr_write.format = taps->param.sbucket_attr.format;
        brr->cmd.taps.subcmd.brr_write.adata = ads_local_pointer;

        rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_SBUCKET_WORK, brr);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue brr work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            tmu_cmd_free(unit, brr);
        } else {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s sbucket local index update for domain %d index %d with local index %d.\n"),
                   FUNCTION_NAME(), sbh->domain, sph->index, ads_local_pointer));
        }
    }
    return rv;
}

/* allocate and enqueue work item for BBx & BRR update for given pivot */
int _taps_sbucket_enqueue_pivot_work(int unit, 
                                     const taps_handle_t taps,
                                     const taps_sbucket_handle_t sbh,
                                     const taps_spivot_handle_t sph,
                                     const taps_wgroup_handle_t *work_group,
                                     unsigned int ads_local_pointer /* bpm local pointer */,
                                     uint8 insert /* 1-insert, 0-delete */)
{
    soc_sbx_caladan3_tmu_cmd_t *bbx=NULL, *brr=NULL;
    int rv = SOC_E_NONE;
    uint32 key[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 key_length = 0;
    uint8 valid_words = 0;
    uint8 words_num = 0;
    int words_index = 0;
    int hw_instance = 0;
    int hw_bucket_id = 0;
    
    if (!taps || !sbh || !sph || 
        !work_group || ads_local_pointer > _BRR_INVALID_CPE_) return SOC_E_PARAM;

    words_num = BITS2WORDS(taps->param.key_attr.lookup_length);

    sal_memset(&key[0], 0, 
                sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);

    if (taps->param.mode == TAPS_OFFCHIP_ALL || sph->length == _TAPS_INVALIDATE_PFX_LEN_ 
        || sph->length == 0) {
        /*For offchip mode,no need to do key shift, so keep the value of key and length */
        key_length = sph->length;
        sal_memcpy(&key[0], &sph->pivot[0], 
                sizeof(unsigned int) * words_num);
    } else {
        /*For onchip mode, key shift is needed*/
        /*key length should be longer than shift len*/
        assert(sph->length >= (sbh->tcam_shift_len - TAPS_PADDING_BITS(taps)));

        key_length = sph->length - (sbh->tcam_shift_len - TAPS_PADDING_BITS(taps));
        valid_words = BITS2WORDS(key_length);
        
        /* Get new key after shifting*/
        if (key_length == 0 && sph->length != 0 
            && taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS
            && _TAPS_KEY_IPV6(taps)) {
            /* Fix V6_COLLISION error for Mode 1*/
            key_length = _TAPS_INVALIDATE_PFX_LEN_;
        } else {
            for (words_index = words_num - 1; 
                (words_index >= 0) && (valid_words > 0); 
                words_index--, valid_words--) {
                if ((valid_words - 1) != 0) {
                    key[words_index] = sph->pivot[words_index];
                } else {
                    /* Last words, we should mask the shift bits*/
                    key[words_index] = sph->pivot[words_index] 
                                    & TP_MASK(key_length 
                                            - WORDS2BITS(words_num - 1 - words_index));
                }
            }
        }
    }

    rv = tmu_cmd_alloc(unit, &bbx); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate internal update commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    } else if (SOC_SUCCESS(rv) && insert && (taps->param.mode != TAPS_ONCHIP_SEARCH_OFFCHIP_ADS)) {
        rv = tmu_cmd_alloc(unit, &brr); 
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate internal update commands %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    if (SOC_SUCCESS(rv)) {
        /* Get hardware instance and bucket id */
        rv = taps_sbucket_instance_and_bucket_id_get(unit, taps, 
                                        sbh->domain, &hw_instance, &hw_bucket_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to get hardware instance and bucket id\n"),
                       FUNCTION_NAME(), unit));
        }
    }
    
    /* BRR is useless in mode 1*/
    if (SOC_SUCCESS(rv) && insert && (taps->param.mode != TAPS_ONCHIP_SEARCH_OFFCHIP_ADS)) {
        /* Enqueue work items for BBx & BRR updates */
        brr->opcode = SOC_SBX_TMU_CMD_TAPS;
        brr->cmd.taps.instance = hw_instance;
        brr->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BRR;
        brr->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE;
        brr->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
        brr->cmd.taps.subcmd.brr_write.segment = taps->segment;
        brr->cmd.taps.subcmd.brr_write.offset = hw_bucket_id;
        brr->cmd.taps.subcmd.brr_write.prefix_id = sph->index;
        brr->cmd.taps.subcmd.brr_write.format = sbh->sbucket_format;
        brr->cmd.taps.subcmd.brr_write.adata = ads_local_pointer;
        
        rv = taps_command_enqueue_for_all_devices(unit, taps, work_group, TAPS_SBUCKET_WORK, brr);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue brr work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } else {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s sbucket local index update for domain %d index %d with local index %d.\n"),
                   FUNCTION_NAME(), sbh->domain, sph->index, ads_local_pointer));
        }
    }
    if (SOC_SUCCESS(rv)) {
        bbx->opcode = SOC_SBX_TMU_CMD_TAPS;
        bbx->cmd.taps.instance = hw_instance;
        bbx->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BB;
        bbx->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE;
        bbx->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
        bbx->cmd.taps.subcmd.bb_write.segment = taps->segment;
        bbx->cmd.taps.subcmd.bb_write.offset = hw_bucket_id;
        bbx->cmd.taps.subcmd.bb_write.prefix_id = sph->index;
        bbx->cmd.taps.subcmd.bb_write.kshift = 0;
        bbx->cmd.taps.subcmd.bb_write.align_right = FALSE;
        sal_memcpy(&bbx->cmd.taps.subcmd.bb_write.key[0], 
                &key[0], 
                sizeof(unsigned int) * BITS2WORDS(taps->param.key_attr.lookup_length));
        bbx->cmd.taps.subcmd.bb_write.length = (insert)?key_length:_TAPS_INVALIDATE_PFX_LEN_;
        bbx->cmd.taps.subcmd.bb_write.format = sbh->sbucket_format;
        
        rv = taps_command_enqueue_for_all_devices(unit, taps, work_group, TAPS_SBUCKET_WORK, bbx);
        if (SOC_FAILURE(rv)) {
            dq_p_t elem=NULL;
            
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue bbx work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));

            /* remove the queued works and free the cmd items */
            taps_work_dequeue(unit, work_group[unit], TAPS_SBUCKET_WORK, &elem, _WQ_DEQUEUE_TAIL_);
        } else {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s sbucket prefix update for domain %d index %d with key length %d.\n"),
                   FUNCTION_NAME(), sbh->domain, sph->index, bbx->cmd.taps.subcmd.bb_write.length));
        }
    }

    if (SOC_FAILURE(rv)) {
        if (bbx) tmu_cmd_free(unit, bbx);
        if (brr) tmu_cmd_free(unit, brr);
    }

    return rv;
}

#define _SBKT_PIVOT_CLONE

static int _taps_sbucket_pivot_clone_propagate_cb(trie_node_t *trie_node, trie_bpm_cb_info_t *info) 
{
    /* do nothing here, we only need to update trie bpm mask */
    return SOC_E_NONE;
}

int _taps_sbucket_pivot_clone(trie_node_t *payload, void *datum)
{
    int rv = SOC_E_NONE;
    sbucket_clone_datum_t *clone;
    taps_spivot_handle_t  sph;
    taps_dprefix_handle_t dph;
    unsigned int bpm_pfx_len=0, ads_local_pointer=0;
    trie_bpm_cb_info_t info;
#ifdef _SBKT_PIVOT_CLONE
    uint32 bpm_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)], word_idx;
    taps_spivot_handle_t  bpm_sph;
#endif

    if (!payload || !datum) return SOC_E_PARAM;
    if (payload->type != PAYLOAD) return SOC_E_NONE;

    clone = (sbucket_clone_datum_t*)datum;
    sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);

    if (clone->common.taps->param.mode == TAPS_ONCHIP_ALL) {
        /* For mode 0, associate data should be the payload */
        ads_local_pointer = sph->payload[0];
    } else if (clone->common.taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        /* For mode 1, associate data is useless */
        ads_local_pointer = _BRR_INVALID_CPE_;
    } else {
    /*taps_sbucket_pivot_dump(clone->common.unit, clone->common.taps,
      sph, 1);*/

    /* figure out local pointer for bpm */
#ifdef _SBKT_PIVOT_CLONE
    /* find bpm length in the new sbucket 
     * we have to do it this way since the new tcam pivot is not 
     * inserted into tcam trie yet.
     */
    rv = trie_find_prefix_bpm(clone->nsbh->trie, sph->pivot, sph->length, &bpm_pfx_len);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META("%s: unit %d Failed"
                        " to find BPM for following key\n"),
               FUNCTION_NAME(), clone->common.unit));
    (void)taps_show_prefix(clone->common.taps->param.key_attr.lookup_length,
                   sph->pivot, sph->length);
    return rv;  
    }

    if (bpm_pfx_len == 0) {
    if (sph->is_prefix) {
        /* pivot itself is a prefix, should point to itself */
        /* find dph of the bpm route */
        rv = taps_dbucket_find_prefix(clone->common.unit,
                      sph->dbh,
                      sph->pivot, 
                      sph->length,
                      &dph);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to find dram prefix for BPM key %d:%s !!!\n"), 
                   FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
        return rv;
        }
        
        /* calculate the local pointer of the bpm route */
        rv = taps_calculate_prefix_local_pointer(clone->common.unit,
                             clone->common.taps,
                             sph, dph,
                             &ads_local_pointer);
        
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to find prefix local pointer %d:%s !!!\n"), 
                   FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
        return rv;
        }       
    } else {
        /* didn't find bpm in the new sbucket, and the pivot itself is
         * not a prefix. We can search this way since the shorter prefix
         * is cloned first.
         */
        ads_local_pointer = _BRR_INVALID_CPE_;
    }
    } else {
    /* find the spivot in the new sbucket for the bpm route */
    for (word_idx = 0; word_idx < BITS2WORDS(clone->common.taps->param.key_attr.lookup_length); word_idx++) {
        bpm_key[word_idx] = sph->pivot[word_idx];
    }
    
    rv = taps_key_shift(clone->common.taps->param.key_attr.lookup_length, bpm_key, sph->length,
                sph->length - bpm_pfx_len);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d Failed to construct BPM key\n"),
                   FUNCTION_NAME(), clone->common.unit));
        return rv;  
    }
    
    rv = taps_sbucket_prefix_pivot_find(clone->common.unit,
                        clone->nsbh,
                        bpm_key, 
                        bpm_pfx_len, 
                        TAPS_OFFCHIP_ALL,
                        &bpm_sph);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d Failed to find sram pivot in sbucket 0x%x for BPM key\n"),
                   FUNCTION_NAME(), clone->common.unit, (uint32)clone->nsbh));
        (void)taps_show_prefix(clone->common.taps->param.key_attr.lookup_length,
                   bpm_key, bpm_pfx_len);       
        return rv;  
    } else {
        /* find dph of the bpm route */
        rv = taps_dbucket_find_prefix(clone->common.unit,
                      bpm_sph->dbh,
                      bpm_key, 
                      bpm_pfx_len,
                      &dph);
        if (SOC_FAILURE(rv)) {
        /* it's possible that bpm prefix stuck in the old sbucket, search there
         * to verify, we need to find it there, otherwise it's internal error.
         */
        rv = taps_sbucket_prefix_pivot_find(clone->common.unit,
                            clone->sbh,
                            bpm_key, 
                            bpm_pfx_len,
                            TAPS_OFFCHIP_ALL,
                            &bpm_sph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d Failed to find sram pivot in old sbucket 0x%x for BPM key\n"),
                       FUNCTION_NAME(), clone->common.unit, (uint32)clone->sbh));
            return rv;
        }

        rv = taps_dbucket_find_prefix(clone->common.unit,
                          bpm_sph->dbh,
                          bpm_key, 
                          bpm_pfx_len,
                          &dph);
        if (SOC_FAILURE(rv)) {
            /* missed in both new sbucket and old sbucket, something is wrong */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to find dram prefix in dbucket 0x%x for BPM key %d:%s !!!\n"), 
                       FUNCTION_NAME(), clone->common.unit, (uint32)bpm_sph->dbh, rv, soc_errmsg(rv)));
            (void)taps_show_prefix(clone->common.taps->param.key_attr.lookup_length,
                       bpm_key, bpm_pfx_len);
            return rv;
        } else {
            /* in this case, there is no point of point the ADS for now,
             * set it to _BRR_INVALID_CPE_ for now.
             * if the bpm route is moved later into the new sbucket,
             * we will do propagation and recalculate the bpm pointer
             * at the move time. To make sure that will be correct
             * we need to do a delete propagate in the new sbucket trie
             */
            info.user_data = NULL;
            info.pfx = bpm_key;
            info.len = bpm_pfx_len;
            rv = trie_propagate_prefix(clone->nsbh->trie, bpm_key, bpm_pfx_len, FALSE,
                           _taps_sbucket_pivot_clone_propagate_cb, &info);
            if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to update bpm %d:%s !!!\n"), 
                       FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
            } else {
            ads_local_pointer = _BRR_INVALID_CPE_;
            bpm_pfx_len = 0;
            }
        }
        } else {
        /* calculate the local pointer of the bpm route */
        rv = taps_calculate_prefix_local_pointer(clone->common.unit,
                             clone->common.taps,
                             bpm_sph, dph,
                             &ads_local_pointer);
        
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to calculate prefix local pointer %d:%s !!!\n"), 
                       FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
            return rv;
        }
        }       
    }
    }
#else
    rv = _taps_sbucket_find_bpm(clone->common.unit, 
                                clone->common.taps,
                                clone->sbh, sph->pivot, sph->length,
                                &bpm_pfx_len, &ads_local_pointer);
    if (SOC_SUCCESS(rv)) {
        if (bpm_pfx_len == 0) {
        if (sph->is_prefix) {
        /* find dph of the bpm route */
        rv = taps_dbucket_find_prefix(clone->common.unit,
                          sph->dbh,
                          sph->pivot, 
                          sph->length,
                          &dph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to find dram prefix for BPM key %d:%s !!!\n"), 
                       FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
            return rv;
        }
        
        /* calculate the local pointer of the bpm route */
        rv = taps_calculate_prefix_local_pointer(clone->common.unit,
                             clone->common.taps,
                             sph, dph,
                             &ads_local_pointer);
        
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to find prefix local pointer %d:%s !!!\n"), 
                       FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
            return rv;
        }
        } else {
        ads_local_pointer = _BRR_INVALID_CPE_;
        }
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: unit %d failed to find bpm prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
    }
#endif
    }
    if (SOC_SUCCESS(rv)) {
        rv = _taps_sbucket_enqueue_pivot_work(clone->common.unit, 
                                              clone->common.taps, clone->nsbh, 
                                              sph, clone->common.wgroup,
                                              ads_local_pointer, TRUE);
        if (SOC_SUCCESS(rv)) {
            if (clone->nsbh->domain != clone->sbh->domain) {
			/* Invalid the old entry*/
            if (clone->common.taps->param.mode == TAPS_OFFCHIP_ALL) {
		/* we can only commit this work after TCAM pivot is inserted
		 * for now, use the TAPS_REDISTRIBUTE_STAGE3_WORK type.
		 * We should realy support different stages for same type.
		 */
		/*
		 * we should never invalidate the wsph or catch all entry in sbucket 
		 */
		if (sph->length != 0){
		    clone->common.wgroup[clone->common.unit]->force_work_type_enable = TRUE;
		    clone->common.wgroup[clone->common.unit]->forced_work_type = TAPS_REDISTRIBUTE_STAGE3_WORK;
		    rv = _taps_sbucket_enqueue_pivot_work(clone->common.unit, 
							  clone->common.taps, clone->sbh, sph,
							  clone->common.wgroup, _BRR_INVALID_CPE_, FALSE); 
		    if (SOC_FAILURE(rv)) {
			LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("%s: unit %d Failed to update sbucket %d:%s !!!\n"), 
                                   FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
		    }
		    clone->common.wgroup[clone->common.unit]->force_work_type_enable = FALSE;
		}
	    } else {
                rv = _taps_sbucket_enqueue_pivot_work(clone->common.unit, 
                     clone->common.taps, clone->sbh, sph,
                     clone->common.wgroup, _BRR_INVALID_CPE_, FALSE); 
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d Failed to update sbucket %d:%s !!!\n"), 
                               FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
                }
            }
            }
            /* clear all pivots except wild pivot */
            if (sph->index != _SBKT_WILD_CHAR_RSVD_POS_) {
                /* clear the pivot from the old bucket */
                SHR_BITCLR(clone->sbh->pivot_bmap, sph->index);
            }

            /* set the pivot from on new bucket */
            SHR_BITSET(clone->nsbh->pivot_bmap, sph->index);
            sph->sbh = clone->nsbh;
            if (clone->common.taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
                /* For mode 1, need update DRAM payload table */
                rv = taps_update_offchip_payload_work(clone->common.unit, 
                        clone->common.taps, clone->common.wgroup, sph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unit %d failed to update payload %d:%s !!!\n"), 
                               FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
                }
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d failed to enqueue sph pivot work %d:%s !!!\n"), 
                       FUNCTION_NAME(), clone->common.unit, rv, soc_errmsg(rv)));
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_commit
 * Purpose:
 *     commit sram bucket
 */
int taps_sbucket_commit(int unit, taps_wgroup_handle_t wgroup)
{
    if (!wgroup) return SOC_E_PARAM;

    return taps_work_commit(unit, wgroup,
                            work_type, COUNTOF(work_type),
                            _TAPS_SEQ_COMMIT);
}

/*
 *
 * Function:
 *     taps_sbucket_free_work_queue
 * Purpose:
 *     free all outstanding work items
 */
int taps_sbucket_free_work_queue(int unit, 
                                 taps_wgroup_handle_t wgroup)
{
    if (!wgroup) return SOC_E_PARAM;

    return taps_free_work_queue(unit, wgroup,
                                work_type, 
                                COUNTOF(work_type));
}

/*
 *
 * Function:
 *     taps_sbucket_pivot_alloc
 * Purpose:
 *     allocate sram bucket pivot
 */
int taps_sbucket_pivot_alloc(int unit,
                             taps_sbucket_handle_t sbh, 
                             unsigned int pivot_index,
                             unsigned int max_key_length,
                             taps_spivot_handle_t *nsph)
{
    int rv = SOC_E_NONE;
    
    if (!sbh || !nsph || pivot_index >= sbh->prefix_number) return SOC_E_PARAM;

    if (SHR_BITGET(sbh->pivot_bmap, pivot_index)) return SOC_E_BUSY;

    rv = _hpcm_spivot_alloc(unit, max_key_length, nsph);
    if (SOC_SUCCESS(rv)) {
        (*nsph)->index = pivot_index;
        SHR_BITSET(sbh->pivot_bmap, pivot_index);
        (*nsph)->sbh = sbh;
        (*nsph)->node.type = PAYLOAD;
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_pivot_free
 * Purpose:
 *     free sram bucket pivot
 */
int taps_sbucket_pivot_free(int unit, 
                            taps_sbucket_handle_t sbh,
                            taps_spivot_handle_t sph)
{
    int rv = SOC_E_NONE;

    if (!sbh || !sph) return SOC_E_PARAM;
    
    assert(sph->index < sbh->prefix_number);

    /* Note: object must be destroyed before the index is freed */
    if (!SHR_BITGET(sbh->pivot_bmap, sph->index)) return SOC_E_PARAM;

    rv = _hpcm_spivot_free(unit, sph);

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_create_for_offchip_mode
 * Purpose:
 *     allocate sram bucket
 */
static int taps_sbucket_create_for_offchip_mode(int unit, 
                        const taps_handle_t taps, 
                        const taps_wgroup_handle_t *wgroup,
                        unsigned int domain,  /* segment id of domain */
                        taps_dbucket_handle_t wdbh, /* pointer to wild pivot dbucket */
                        uint8 populate_wild_pivot, /* if set inserts wild pivot into trie */
                        taps_sbucket_handle_t *handle)
{
    int rv = SOC_E_NONE;
    taps_spivot_handle_t wsph=NULL;

    if (!handle || !taps || !wdbh) return SOC_E_PARAM;

    /* validate the segment offset provided */
    if (domain >= taps->param.seg_attr.seginfo[taps->param.instance].num_entry) {
        return SOC_E_PARAM;
    }

    rv = _hpcm_sbucket_alloc(unit, handle);
    if (SOC_SUCCESS(rv)) {
        (*handle)->domain = domain;
        /* For offchip mode, no need to do key shift, so don't need modify sbucket format*/
        (*handle)->prefix_number = taps->param.sbucket_attr.max_pivot_number;
        (*handle)->sbucket_format = taps->param.sbucket_attr.format;
        /* set the wild char entry to valid pfx len 0/0 & set brr to invalid to fall back to
         * tcam pivot */
        rv = taps_sbucket_pivot_alloc(unit, *handle, _SBKT_WILD_CHAR_RSVD_POS_,
                                      taps->param.key_attr.lookup_length, &wsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate sram"
                                  " wild char pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } 

        if (SOC_SUCCESS(rv)) {
            rv = _taps_sbucket_enqueue_pivot_work(unit, taps, *handle, wsph,
                                                  wgroup, _BRR_INVALID_CPE_, TRUE);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to enqueue sram"
                                      " wild char pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }

        if (SOC_SUCCESS(rv)) {
        rv = trie_init(taps->param.key_attr.lookup_length, &((*handle)->trie));
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to init sbucket trie %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }

        if (SOC_SUCCESS(rv) && populate_wild_pivot) {
            /* insert on to the sbucket trie so the dram bucket could be used to house prefix */
            rv = trie_insert((*handle)->trie, &wsph->pivot[0], 
                             &wsph->pivot[0], 0, &wsph->node);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to insert sram wild pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }

        if (SOC_SUCCESS(rv)) {
            (*handle)->wsph   = wsph;
            wsph->dbh = wdbh;
        } else {
            if (wsph) taps_sbucket_pivot_free(unit, *handle, wsph);
            _hpcm_sbucket_free(unit, *handle);
        }
    }

    return rv;
}
static uint8 taps_sbucket_format_get(uint32 key_length)
{
    uint8 sbucket_format = 0;
    
    switch ((key_length + _BB_ENTRY_SIZE_ - 1)/ _BB_ENTRY_SIZE_) {
    case 0:
    case 1:
    case 2:
        sbucket_format = SOC_SBX_TMU_TAPS_BB_FORMAT_2ENTRIES;
        break;
    case 3:
        sbucket_format = SOC_SBX_TMU_TAPS_BB_FORMAT_3ENTRIES;
        break;
    case 4:
        sbucket_format = SOC_SBX_TMU_TAPS_BB_FORMAT_4ENTRIES;
        break;
    case 5:
    case 6:
        sbucket_format = SOC_SBX_TMU_TAPS_BB_FORMAT_6ENTRIES;
        break;
    case 7:
    case 8:
        sbucket_format = SOC_SBX_TMU_TAPS_BB_FORMAT_8ENTRIES;
        break;
    case 9:
    case 10:
    case 11:
    case 12:
        sbucket_format = SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES;
        break;
    default:
        break;
    }

    return sbucket_format;
}

/*
 *
 * Function:
 *     taps_sbucket_create_for_onchip_mode
 * Purpose:
 *     allocate sram bucket
 */
static int taps_sbucket_create_for_onchip_mode(int unit, 
                        const taps_handle_t taps, 
                        const taps_wgroup_handle_t *wgroup,
                        unsigned int domain,  /* segment id of domain */
                        taps_dbucket_handle_t wdbh, /* pointer to wild pivot dbucket */
                        uint8 is_create_default_entry, /* if set inserts wild pivot into trie 
                                                                and create a invalid sbucket pivot 
                                                                for wild tcam entry */
                        uint32 key_shift_len,
                        taps_sbucket_handle_t *handle)
{
    int rv = SOC_E_NONE;
    unsigned int  best_match_data = 0;
    taps_spivot_handle_t wsph=NULL;
    int insert_wsph = FALSE;
    
    if (!handle || !taps) return SOC_E_PARAM;

    /* validate the segment offset provided */
    if (domain >= taps->param.seg_attr.seginfo[taps->param.instance].num_entry) {
        return SOC_E_PARAM;
    }

    rv = _hpcm_sbucket_alloc(unit, handle);

    if (SOC_SUCCESS(rv)) {
        (*handle)->domain = domain;
        (*handle)->padding_len = TAPS_PADDING_BITS(taps);
        (*handle)->tcam_shift_len = key_shift_len + TAPS_PADDING_BITS(taps);
        (*handle)->sbucket_format = taps_sbucket_format_get(taps->param.key_attr.lookup_length 
                                                            - (*handle)->tcam_shift_len);
        /* Invalid divide by 0 condition check */
        if ((*handle)->sbucket_format == 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Invalid parameter causes divide by zero condition !!!\n"), 
                       FUNCTION_NAME(), unit));
            _hpcm_sbucket_free(unit, *handle);
            return SOC_E_PARAM;
        }
        (*handle)->prefix_number = (BB_PREFIX_NUMBER((*handle)->sbucket_format) * (1<<taps->param.tcam_layout));
        if (taps->param.sbucket_attr.max_pivot_number < (*handle)->prefix_number) {
            taps->param.sbucket_attr.max_pivot_number = (*handle)->prefix_number;
        }    
        rv = trie_init(taps->param.key_attr.lookup_length, &((*handle)->trie));
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to init sbucket trie %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            _hpcm_sbucket_free(unit, *handle);
        }
    }

    if (taps->param.mode == TAPS_ONCHIP_ALL) {
        /* For mode zero, we need insert a wild sph for each sbucket */
        insert_wsph = TRUE;
    }
    
    if (SOC_SUCCESS(rv) && (is_create_default_entry || insert_wsph)) {
        /* If insert default entry for mode 0/1, just need to reserve a invalid entry to store
            the default payload, this entry should not be matched*/
        rv = taps_sbucket_pivot_alloc(unit, *handle, _SBKT_WILD_CHAR_RSVD_POS_,
                                      taps->param.key_attr.lookup_length, &wsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to allocate sram"
                                  " wild char pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        } else {
            wsph->length = 0;
            if (taps->param.mode == TAPS_ONCHIP_ALL) {
                best_match_data = taps->param.defpayload[0] 
                                  & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
                wsph->payload[0] = best_match_data;
            } else if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS){
                /* Default entry's asso data is at index 0*/
                best_match_data = 0;
                sal_memcpy(&wsph->payload[0], &taps->param.defpayload[0], 
                        sizeof(unsigned int) * _TAPS_PAYLOAD_WORDS_);
		wsph->cookie = NULL;
                rv = taps_update_offchip_payload_work(unit, taps, 
                                            wgroup, wsph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to update payload %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            } 
        }
        if (SOC_SUCCESS(rv)) {
            rv = _taps_sbucket_enqueue_pivot_work(unit, taps, *handle, wsph,
                                            wgroup, best_match_data, TRUE);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to enqueue sram"
                                      " wild char pivot %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            
            }
        }
        
        if (SOC_SUCCESS(rv)) {
            if (insert_wsph) {
                (*handle)->wsph = wsph;
            } else {
                if (wsph) _hpcm_spivot_free(unit, wsph);
                (*handle)->wsph = NULL;
            }
        } else {
            if (wsph) taps_sbucket_pivot_free(unit, *handle, wsph);
            _hpcm_sbucket_free(unit, *handle);
        }         
    }
    
    return rv;
}


int taps_sbucket_create(int unit, 
                        const taps_handle_t taps, 
                        const taps_wgroup_handle_t *wgroup,
                        unsigned int domain,  /* segment id of domain */
                        taps_dbucket_handle_t wdbh, /* pointer to wild pivot dbucket */
                        uint8 is_create_default_entry, /* if set inserts wild pivot into trie 
                                                        and create a invalid sbucket pivot 
                                                        for wild tcam entry */
                        uint32 key_shift_len,
                        taps_sbucket_handle_t *handle) 
{
    int rv = SOC_E_NONE;
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        rv = taps_sbucket_create_for_offchip_mode(unit, taps, wgroup,
                                            domain, wdbh, 
                                            is_create_default_entry, handle);
    } else {
        rv = taps_sbucket_create_for_onchip_mode(unit, taps, wgroup,
                                            domain, wdbh, is_create_default_entry, 
                                            key_shift_len, handle);
    }
    
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_free
 * Purpose:
 *     destroy sram bucket 
 */

int taps_sbucket_destroy(int unit,
                         const taps_handle_t taps, 
                         const taps_wgroup_handle_t *wgroup,
                         taps_sbucket_handle_t sbh)
{
    int rv = SOC_E_NONE;

    if (!taps || !wgroup || !sbh) return SOC_E_PARAM;

    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        /* free wild pivot */
        rv = taps_sbucket_delete_pivot(unit, taps, wgroup, sbh, sbh->wsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to destroy wild sram pivot 0x%x in sbh 0x%x. %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, (uint32)sbh->wsph, (uint32)sbh, rv, soc_errmsg(rv)));
            taps_sbucket_dump(unit, taps, sbh, TAPS_DUMP_SRAM_SW_BKT| \
                      TAPS_DUMP_SRAM_SW_PIVOT | TAPS_DUMP_SRAM_VERB);
        }
    } else if (taps->param.mode == TAPS_ONCHIP_ALL) {
        /* For mode zero, wsph didn't insert into the sbucket trie, just need to free the memory */
        rv = taps_sbucket_pivot_free(unit, sbh, sbh->wsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to free sph handle %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }
    if (sbh->trie) {
        rv = trie_destroy(sbh->trie);
    }
    if (SOC_SUCCESS(rv)) {
        rv = _hpcm_sbucket_free(unit, sbh);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to destroy sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_pivot_id_alloc
 * Purpose:
 *     allocate sram pivot id
 */
int taps_sbucket_pivot_id_alloc(int unit, 
                                taps_handle_t taps, 
                                taps_sbucket_handle_t sbh,
                                unsigned int *id)
{
    int count=0, rv=SOC_E_NONE;

    if (!taps || !sbh || !id) return SOC_E_PARAM;

    SHR_BITCOUNT_RANGE(sbh->pivot_bmap, count, 0, sbh->prefix_number);

    if (sbh->prefix_number != count) {
        if (taps->param.mode == TAPS_ONCHIP_ALL) {
            /*For mode zero, reserve index 0 used as wild sph */
            count = 1;
        } else {
            count = 0;
        }
        
        /* allocate a pivot index within the bucket */
        while(SHR_BITGET(sbh->pivot_bmap, count)) {
            count++;
        }

        *id = count;
        assert(*id < sbh->prefix_number);
    } else {
        rv = SOC_E_FULL;
    }
    
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_pivot_id_free
 * Purpose:
 *     free sram pivot id
 */
int taps_sbucket_pivot_id_free(int unit, 
                               taps_handle_t taps, 
                               taps_sbucket_handle_t sbh,
                               unsigned int id)
{
    int rv=SOC_E_NONE;

    if (!taps || !sbh || id >= sbh->prefix_number) return SOC_E_PARAM;

    if (taps->param.mode == TAPS_ONCHIP_ALL && id == 0) {
        /* For mode zero, always reserve index 0 for wild sph */
        return SOC_E_NONE;
    }
    if (SHR_BITGET(sbh->pivot_bmap, id)) {
        SHR_BITCLR(sbh->pivot_bmap, id);
    } else {
        rv = SOC_E_PARAM;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Trying to free unallocated ID %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}


/*
 *
 * Function:
 *     taps_sbucket_pivot_find
 * Purpose:
 *     find sram pivot within the bucket given a pivot
 */
int taps_sbucket_pivot_find(int unit,
                            taps_sbucket_handle_t handle,
                            uint32 *pivot, 
                            unsigned int len, 
                            taps_spivot_handle_t *sph /*out*/)
{
    int rv = SOC_E_NONE;
    trie_node_t *payload;

    if (!handle || !sph || !pivot) return SOC_E_PARAM;

    rv = trie_search(handle->trie, pivot, len, &payload);
    if (rv != SOC_E_NOT_FOUND) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Pivot Insertion Failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    } else {
        *sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_prefix_find
 * Purpose:
 *     find sram pivot for given a prefix
 */
int taps_sbucket_prefix_pivot_find(int unit,
                                   taps_sbucket_handle_t handle,
                                   uint32 *prefix, 
                                   unsigned int len, 
                                   taps_search_mode_e_t mode,
                                   taps_spivot_handle_t *sph /*out*/)
{
    int rv = SOC_E_NONE;
    trie_node_t *payload;

    if (!handle || !sph || !prefix) return SOC_E_PARAM;

    if (mode == TAPS_OFFCHIP_ALL) {
        rv = trie_find_lpm(handle->trie, prefix, len, &payload);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Prefix find Failed %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));

        }
    } else {
        rv = trie_search(handle->trie, prefix, len, &payload);
    }
    if (SOC_SUCCESS(rv)) {
        *sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);
    } 
    return rv;
}

/*
 *   Function
 *      taps_update_offchip_payload_work
 *   Purpose
 *      enqueue tmu commands to write payload of a dbucket prefix object into dram
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps   : taps object handle
 *      (IN) wgroup : work group handle (for future API batching)
 *      (IN) sph    : handle for sbucket object which manages the sbucket prefix object
 *   Returns
 *       SOC_E_NONE - successfully enqueued the work
 *       SOC_E_* as appropriate otherwise
 */
int taps_update_offchip_payload_work(int unit, 
                         const taps_handle_t taps,
                         const taps_wgroup_handle_t *wgroup,
                         taps_spivot_handle_t sph)
{
    int rv = SOC_E_NONE;
    int table_id, entry_num, size;
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
                              "%s: unit %d failed to allocate TMU DRAM payload table update commands %d:%s !!!\n"), 
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
    table_id = taps->param.dbucket_attr.table_id[TAPS_DDR_PAYLOAD_TABLE];
    entry_num = sph->sbh->domain * (1 << (taps->param.tcam_layout)) * _MAX_SBUCKET_PIVOT_PER_BB_ + sph->index;
    size = _tmu_dbase[unit]->table_cfg.table_attr[table_id].entry_size_bits;
    
    dpld_cmd->opcode = SOC_SBX_TMU_CMD_XL_WRITE;
    dpld_cmd->cmd.xlwrite.table = table_id;
    if (_TAPS_KEY_IPV4(taps)) {
        dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV4_DATA;
    } else {
        dpld_cmd->cmd.xlwrite.lookup = SOC_SBX_TMU_LKUP_TAPS_IPV6_DATA;
    }
    dpld_cmd->cmd.xlwrite.entry_num = entry_num;
    dpld_cmd->cmd.xlwrite.offset = 0;
    dpld_cmd->cmd.xlwrite.size = (size + SOC_SBX_TMU_CMD_WORD_SIZE - 1)/SOC_SBX_TMU_CMD_WORD_SIZE;
    dpld_cmd->cmd.xlwrite.value_size = size;
    if (_TAPS_KEY_IPV4(taps)) {
	/* IPV4: copy over the payload */
	for (word=0; word < BITS2WORDS(dpld_cmd->cmd.xlwrite.value_size); word++) {
	    dpld_cmd->cmd.xlwrite.value_data[word] = sph->payload[word];
	}			
    } else {
	/* IPV6: calculate CPE and copy over payload */
	prefix_length = sph->length + sph->sbh->padding_len;
	sal_memcpy(original_prefix, sph->pivot, sizeof(uint32) * TAPS_IPV6_KEY_SIZE_WORDS);
	sal_memcpy(payload, sph->payload, sizeof(uint32) * _TAPS_PAYLOAD_WORDS_);

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
    
    rv = taps_command_enqueue_for_all_devices(unit, taps, wgroup, TAPS_SBUCKET_WORK, dpld_cmd);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to enqueue sbucket work item %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        tmu_cmd_free(unit, dpld_cmd);
        return rv;
    } else {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s payload update for domain %d  index %d (entry %d).\n"),
                   FUNCTION_NAME(), sph->sbh->domain, sph->index, entry_num));
    }

    return rv;
}


/*
 *
 * Function:
 *     taps_sbucket_insert_pivot
 * Purpose:
 *     Insert a pivot into the sram bucket.
 *     The function returns a new sram pivot handle if insert succeeds
 */
int taps_sbucket_insert_pivot(int unit, 
                              const taps_handle_t taps,
                              const taps_wgroup_handle_t *work_group,
                              unsigned int pivot_index,
                              taps_sbucket_handle_t sbh,
                              uint32 *pivot, 
                              int pivot_len,
                              uint8 is_prefix,
                              uint32 *payload,
			      void *cookie,
                              taps_dbucket_handle_t dbh,
                              uint32 *bpm,
                              /* OUT */
                              taps_spivot_handle_t *nsph)
{
    int rv = SOC_E_NONE;

    if (!taps || !bpm || !pivot || !nsph || !sbh || !work_group) return SOC_E_PARAM;

    if (pivot_len > taps->param.key_attr.lookup_length) return SOC_E_PARAM;

    *nsph = NULL;

    if ((taps->param.mode == TAPS_ONCHIP_ALL) && (pivot_len == sbh->tcam_shift_len)) {
        /* For mode zero, if the pivot len equal to key shift len, that means the this sph's length 
        * would be 0 after key shift. But we already have a wild sph which length is 0. So ranther than 
        * alloc a new handle, just use the wild sph handle*/
        *nsph = sbh->wsph;
    } else {
        rv = taps_sbucket_pivot_alloc(unit, sbh, pivot_index, 
                                      taps->param.key_attr.lookup_length, nsph);
    }

    if (SOC_SUCCESS(rv)) {
        (*nsph)->dbh = dbh;
        (*nsph)->is_prefix = is_prefix;
        sal_memcpy(&(*nsph)->pivot[0], pivot, sizeof(unsigned int) * BITS2WORDS(taps->param.key_attr.lookup_length));
        (*nsph)->length = pivot_len;
        if (payload != NULL) {
            if (taps->param.mode == TAPS_ONCHIP_ALL) {
                (*nsph)->payload[0] = payload[0] 
                                & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
            } else if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
                sal_memcpy(&(*nsph)->payload[0], payload, 
                    sizeof(unsigned int) * _TAPS_PAYLOAD_WORDS_);
            }
	    (*nsph)->cookie = cookie;
        }

        rv = trie_insert(sbh->trie, pivot, bpm, pivot_len, &(*nsph)->node);
        if (SOC_SUCCESS(rv)) {
            unsigned int bpm_pfx_len=0, asso_data=0;
            if (taps->param.mode == TAPS_ONCHIP_ALL) {
                asso_data = (*nsph)->payload[0];
            } else if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
                /* BRR is useless in mode 1*/
                asso_data = _BRR_INVALID_CPE_;
                rv = taps_update_offchip_payload_work(unit, taps, 
                                                    work_group, *nsph);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to update payload %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            } else {
                 /* figure out local pointer for bpm in the splited dbucket */
                rv = taps_get_bpm_pfx(bpm, pivot_len, taps->param.key_attr.lookup_length, &bpm_pfx_len);
                if (SOC_SUCCESS(rv)) {
                    if (bpm_pfx_len == 0) {
                        /* figure out local pointer for bpm in current sbucket */
                        rv = trie_find_prefix_bpm(sbh->trie, pivot, pivot_len, &bpm_pfx_len);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to find taps bpm prefix %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                            return rv;
                        }
                    }
                    if (bpm_pfx_len > 0) {
                        /* calculate the local pointer */
                        rv = _taps_sbucket_get_bpm_pointer(unit, taps,
                                                           pivot, pivot_len, 
                                                           bpm_pfx_len,
                                                           &asso_data);
                        if (SOC_FAILURE(rv)) {
                            LOG_ERROR(BSL_LS_SOC_COMMON,
                                      (BSL_META_U(unit,
                                                  "%s: unit %d failed to find taps bpm prefix %d:%s !!!\n"), 
                                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                        }
                    } else {
                        /* bpm is outside of this sbucket domain */
                        assert(bpm_pfx_len == 0);
                        asso_data = _BRR_INVALID_CPE_;
                    }
                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to find bpm prefix %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
            }
            if (SOC_SUCCESS(rv)) {
                rv = _taps_sbucket_enqueue_pivot_work(unit, taps, sbh, *nsph, 
                                                      work_group, asso_data, TRUE);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "%s: unit %d failed to enqueue sph pivot work %d:%s !!!\n"), 
                               FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                }
                
                /*taps_sbucket_dump(unit, taps, sbh, 1);*/
            } 

            if (SOC_FAILURE(rv)) {
                trie_node_t *pyld;
                /* remove the sph from trie */
                trie_delete(sbh->trie, pivot, pivot_len, &pyld);
                taps_sbucket_pivot_free(unit, sbh, *nsph);
                *nsph = NULL;
            }
        } 
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Pivot allocation failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_insert_default_entry
 * Purpose:
 *     Insert default entry in sram for slave unit
 */
int taps_sbucket_insert_default_entry(int unit, taps_handle_t taps, 
                                taps_wgroup_handle_t work_group)
{
    int rv;
    int instance;
    soc_sbx_caladan3_tmu_cmd_t *bbx=NULL, *brr=NULL;

    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
        instance = taps->param.instance;
    } else {
        if (taps->param.divide_ratio != ceiling_ratio) {
            instance = TAPS_INST_1;
        } else {
            instance = TAPS_INST_0;
        }
    }

    if(taps->param.mode != TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
        rv = tmu_cmd_alloc(unit, &brr); 
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to allocate internal update commands %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        
        /* Enqueue work items for BBx & BRR updates */
        brr->opcode = SOC_SBX_TMU_CMD_TAPS;
        brr->cmd.taps.instance = SW_INST_CONVERT_TO_HW_INST(instance);
        brr->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BRR;
        brr->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE;
        brr->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
        brr->cmd.taps.subcmd.brr_write.segment = taps->segment;
        brr->cmd.taps.subcmd.brr_write.offset = 0;
        brr->cmd.taps.subcmd.brr_write.prefix_id = 0;
        brr->cmd.taps.subcmd.brr_write.format = taps->param.sbucket_attr.format;
        if (taps->param.mode == TAPS_OFFCHIP_ALL) {
            brr->cmd.taps.subcmd.brr_write.adata = _BRR_INVALID_CPE_;
        } else {
            brr->cmd.taps.subcmd.brr_write.adata = taps->param.defpayload[0] 
                                  & TP_MASK(_TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_);
        }
        
        rv = taps_work_enqueue(unit, work_group,
                               TAPS_SBUCKET_WORK, &brr->wq_list_elem);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to enqueue brr work item %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    }

    rv = tmu_cmd_alloc(unit, &bbx); 
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate internal update commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    
    bbx->opcode = SOC_SBX_TMU_CMD_TAPS;
    bbx->cmd.taps.instance = SW_INST_CONVERT_TO_HW_INST(instance);
    bbx->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BB;
    bbx->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE;
    bbx->cmd.taps.max_key_size = taps->param.key_attr.lookup_length;
    bbx->cmd.taps.subcmd.bb_write.segment = taps->segment;
    bbx->cmd.taps.subcmd.bb_write.offset = 0;
    bbx->cmd.taps.subcmd.bb_write.prefix_id = 0;
    bbx->cmd.taps.subcmd.bb_write.kshift = 0;
    bbx->cmd.taps.subcmd.bb_write.align_right = FALSE;
    sal_memset(&bbx->cmd.taps.subcmd.bb_write.key[0], 0, 
            sizeof(unsigned int) * BITS2WORDS(taps->param.key_attr.lookup_length));
    bbx->cmd.taps.subcmd.bb_write.length = 0;
    bbx->cmd.taps.subcmd.bb_write.format = taps->param.sbucket_attr.format;

    rv = taps_work_enqueue(unit, work_group,
                           TAPS_SBUCKET_WORK, &bbx->wq_list_elem);
    if (SOC_FAILURE(rv)) {
        dq_p_t elem=NULL;
        
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to enqueue bbx work item %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));

        /* remove the queued works and free the cmd items */
        taps_work_dequeue(unit, work_group, TAPS_SBUCKET_WORK, &elem, _WQ_DEQUEUE_TAIL_);
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_delete_pivot
 * Purpose:
 *     Delete a pivot into the sram bucket.
 */
int taps_sbucket_delete_pivot(int unit, 
                              const taps_handle_t taps,
                              const taps_wgroup_handle_t *work_group,
                              taps_sbucket_handle_t sbh,
                              taps_spivot_handle_t sph)
{
    int rv = SOC_E_NONE;
    trie_node_t *pyld=NULL;

    if (!taps || !sph || !sbh || !work_group) return SOC_E_PARAM;
    
    /* verify the pivot index on bucket */
    if (SHR_BITGET(sbh->pivot_bmap, sph->index)) {
    rv = trie_delete(sbh->trie, sph->pivot, sph->length, &pyld);

        if (SOC_SUCCESS(rv)) {
            if ((taps->param.mode == TAPS_ONCHIP_ALL) && (sph->length == sbh->tcam_shift_len)) {
                /* For mode zero, this sph is wsph. So just delete it in the trie. 
                * Will update the asso data in the propagation step.*/ 
                sph->length = 0;
                sph->is_prefix = 0;
                sph->cookie = NULL;
            } else {
                rv = _taps_sbucket_enqueue_pivot_work(unit, taps, sbh, sph,
                                                  work_group,
                                                  _BRR_INVALID_CPE_,
                                                  FALSE); 
                if (SOC_SUCCESS(rv)) {
                    rv = taps_sbucket_pivot_free(unit, sbh, sph);
                }
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d sph 0x%x not found in sbh 0x%x %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, (uint32)sph, (uint32)sbh, rv, soc_errmsg(rv)));
        }
    } else {
        rv = SOC_E_NOT_FOUND;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Pivot index:%d not found on bucket Failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, sph->index, rv, soc_errmsg(rv)));
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_pivot_len_get
 * Purpose:
 *     Get a pivot len of a new sbucket
 */
int taps_sbucket_pivot_len_get(int unit, 
                       const taps_handle_t taps,
                       taps_sbucket_handle_t sbh,
                       /* out */
                       uint32 *pivot_len)
{
    int rv = SOC_E_NONE;
    unsigned int max_split_len;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\n$$$$$ SBUCKET SPLIT $$$$$\n")));

    if (!taps || !sbh || !pivot_len) return SOC_E_PARAM;

    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        max_split_len = taps->param.key_attr.length;
    } else {
        max_split_len = taps->param.key_attr.length - _MIN_SBUCKET_KEY_LEN_;
    }
    rv = trie_split_len_get(sbh->trie, max_split_len, pivot_len);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to split Sbucket trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
   
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_trie_merge
 * Purpose:
 *     Merge trie from src_sbh to dst_sbh, no hardware commit
 */
static int taps_sbucket_trie_merge(int unit, 
                       const taps_handle_t taps,
                       taps_sbucket_handle_t dst_sbh,
                       taps_sbucket_handle_t src_sbh)
{
    int rv = SOC_E_NONE;
    int pivot_id = 0;
    taps_spivot_handle_t sph = NULL;
    trie_node_t *payload = NULL;
    unsigned int bpm_mask[TAPS_MAX_KEY_SIZE_WORDS];

    /* Delete wild sph */
    if (src_sbh->wsph != NULL) {
        pivot_id = src_sbh->wsph->index;
        trie_delete(src_sbh->trie, src_sbh->wsph->pivot, src_sbh->wsph->length, &payload);
        rv = taps_sbucket_pivot_free(unit, src_sbh, src_sbh->wsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free sram pivot handle %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        rv = taps_sbucket_pivot_id_free(unit, taps, src_sbh, pivot_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free sram pivot id %u %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, pivot_id, rv, soc_errmsg(rv)));
        }
    }
   
    while(1) {
        rv = trie_iter_get_first(src_sbh->trie, &payload);
        if (rv == SOC_E_EMPTY) {
            /* no nodes left */
            rv = SOC_E_NONE;
            break;
        }
        sal_memset(&bpm_mask[0], 0, sizeof(unsigned int) * TAPS_MAX_KEY_SIZE_WORDS);
        sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);

        /* Get bpm mask */
        rv = trie_bpm_mask_get(src_sbh->trie, sph->pivot, sph->length, bpm_mask);
        if (SOC_FAILURE(rv)) {
            /* failed to insert into new trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to get bpm_mask in src sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }

        /* remove the prefix from old trie */
        rv = trie_delete(src_sbh->trie, sph->pivot, sph->length, &payload);
        if (SOC_FAILURE(rv)) {
            /* failed to insert into new trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to delete sph %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        sal_memset(&sph->node, 0, sizeof(trie_node_t));
        rv = trie_insert(dst_sbh->trie, sph->pivot, bpm_mask, sph->length, &sph->node);
        if (SOC_FAILURE(rv)) {
            /* failed to insert into new trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to insert back sph %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        SHR_BITSET(dst_sbh->pivot_bmap, sph->index);
        sph->sbh = dst_sbh;
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_merge_for_onchip
 * Purpose:
 *     Merge sbucket for mode 0/1
 */
static int taps_sbucket_merge_for_onchip(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t dst_sbh,
                       taps_sbucket_handle_t src_sbh,
                       taps_spivot_handle_t *roll_back_sph)
{
    int rv = SOC_E_NONE, update_roll_back_sph = FALSE;
    unsigned int pivot_id = 0;
    taps_spivot_handle_t sph = NULL;
    trie_node_t *payload = NULL;
    unsigned int bpm_mask[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 redist_sph_len;
    uint32 redist_sph_pivot[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 redist_sph_payload[_TAPS_PAYLOAD_WORDS_];
    void *redist_sph_cookie;

    if (src_sbh->wsph != NULL && src_sbh->wsph->length == 0) {
        pivot_id = src_sbh->wsph->index;
        trie_delete(src_sbh->trie, src_sbh->wsph->pivot, src_sbh->wsph->length, &payload);
        rv = taps_sbucket_pivot_free(unit, src_sbh, src_sbh->wsph);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free sram pivot handle %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
        rv = taps_sbucket_pivot_id_free(unit, taps, src_sbh, pivot_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to free sram pivot id %u %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, pivot_id, rv, soc_errmsg(rv)));
        }
    }
    
    while(1) {
        payload = NULL;
        sph = NULL;

        rv = trie_iter_get_first(src_sbh->trie, &payload);
        if (rv == SOC_E_EMPTY) {
            /* no nodes left */
            rv = SOC_E_NONE;
            break;
        }

        sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);
        
        if (roll_back_sph != NULL) {
            if (*roll_back_sph == sph) {
                update_roll_back_sph = TRUE;
            }
        }
        pivot_id = sph->index;
        redist_sph_len = sph->length;
        sal_memcpy(redist_sph_pivot, sph->pivot, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
        sal_memcpy(redist_sph_payload, sph->payload, sizeof(uint32) * _TAPS_PAYLOAD_WORDS_);
	    redist_sph_cookie = sph->cookie;

        /* remove the prefix from old trie */
        rv = trie_delete(src_sbh->trie, sph->pivot, sph->length, &payload);
        if (SOC_FAILURE(rv)) {
            /* failed to delete from old trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to delete trie node %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }

        rv = _taps_sbucket_enqueue_pivot_work(unit, taps, src_sbh, sph,
                                              wgroup,
                                              _BRR_INVALID_CPE_,
                                              FALSE); 
        if (SOC_FAILURE(rv)) {
            /* failed to update sbucket cache of old bucket */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to update sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        } else {
            rv = taps_sbucket_pivot_free(unit, src_sbh, sph);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to free sph handle %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                break;
            } else {
                sph = NULL;
            }
        }   
        rv = taps_sbucket_pivot_id_free(unit, taps, src_sbh, pivot_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to free sph id %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }       

        /* insert the prefix into trie of new dbucket */
        rv = taps_sbucket_pivot_id_alloc(unit, taps, dst_sbh, &pivot_id);
        if (SOC_FAILURE(rv)) {
           pivot_id = _TAPS_INV_ID_;
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d vrf route failed to allocate spivot %d:%s !!!\n"), 
                      FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
           break;
        }

        sal_memset(&bpm_mask[0], 0, TAPS_MAX_KEY_SIZE_WORDS * sizeof(unsigned int));
        _TAPS_SET_KEY_BIT(bpm_mask, 0, taps->param.key_attr.lookup_length);
        rv = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                                       pivot_id, dst_sbh,
                                       &redist_sph_pivot[0], redist_sph_len,
                                       TRUE, &redist_sph_payload[0], redist_sph_cookie,
                                       NULL, &bpm_mask[0], &sph);
        if (SOC_FAILURE(rv)) {
            /* failed to insert into new trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to insert prefix in new sbh %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        if (update_roll_back_sph) {
            *roll_back_sph = sph;
            update_roll_back_sph = FALSE;
        }
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_merge_for_offchip
 * Purpose:
 *     Merge sbucket for mode 2
 */
static int taps_sbucket_merge_for_offchip(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t dst_sbh,
                       taps_sbucket_handle_t src_sbh)
{
    int rv = SOC_E_NONE;
    sbucket_clone_datum_t datum;
    
    datum.common.unit = unit;
    datum.common.taps = (taps_handle_t) taps;
    datum.common.wgroup = wgroup;
    datum.sbh  = src_sbh;
    datum.nsbh = dst_sbh;
    
    rv = trie_traverse(src_sbh->trie, _taps_sbucket_pivot_clone, 
                       &datum, _TRIE_PREORDER_TRAVERSE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to clone sbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    
    rv = taps_sbucket_trie_merge(unit, taps, dst_sbh, src_sbh);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to merge sbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_merge
 * Purpose:
 *     Merge sbucket
 */
int taps_sbucket_merge(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t dst_sbh,
                       taps_sbucket_handle_t src_sbh,
                       taps_spivot_handle_t *roll_back_sph)
{
    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        return taps_sbucket_merge_for_offchip(unit, taps, wgroup, dst_sbh, src_sbh);
    } else {
        return taps_sbucket_merge_for_onchip(unit, taps, wgroup, dst_sbh, src_sbh, roll_back_sph);
    }
}

/*
 *
 * Function:
 *     taps_sbucket_redundant_pivot_clear
 * Purpose:
 *     Remove ths spviot which there is no pfx in its dbucket
 */
int taps_sbucket_redundant_pivot_clear(int unit, 
                       const taps_handle_t taps,
                       taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       taps_spivot_handle_t *sphs,
                       int sph_num)
{
    int rv = SOC_E_NONE;
    int sph_idx, pivot_id;
    taps_bucket_stat_e_t bkt_stat;
    /* Clean up redundant sph */
    for (sph_idx = 0; sph_idx < sph_num; sph_idx++) {
        /* destroy the non-wild dbucket object */
        rv = taps_dbucket_destroy(unit, taps, wgroup[unit], sphs[sph_idx]->dbh);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to free dbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        sphs[sph_idx]->dbh = NULL; /* unlink dbh & sram pivot */
        pivot_id = sphs[sph_idx]->index;
        /* destroy sram pivot object */
        rv = taps_sbucket_delete_pivot(unit, taps, wgroup,
                                       sbh, 
                                       sphs[sph_idx]);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to free sram pivot %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        } 

        /* double check that the sram pivot is not wild (fixed position at index==0) */
        assert(pivot_id != 0);
        
        /* free back pivot id */
        rv = taps_sbucket_pivot_id_free(unit, taps, 
                                        sbh, 
                                        pivot_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution failed to free sram pivot id:%u %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, pivot_id, rv, soc_errmsg(rv)));
            break;
        }

        /* check sbucket depth */
        rv = taps_sbucket_stat_get(unit, sbh, &bkt_stat);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to compute sbucket vacancy %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        } else if (bkt_stat == _TAPS_BKT_EMPTY) {
                /* this should be a very rare case and we are not handling it for now
                 * basically we are spliting a sbucket but due to the redistribution
                 * process we moved all prefixes into the new sbucket. Not sure if 
                 * this is possible, simply put assert here to capture this in case
                 * it ever happens.....
                 */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution empty out the old sbucket !!!\n"), 
                       FUNCTION_NAME(), unit));
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d redistribution FATAL ERROR CODE 4 !!!\n"), 
                       FUNCTION_NAME(), unit));
        }
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_split_stage1
 * Purpose:
 *     Split a given sram bucket 
 */
int taps_sbucket_split_stage1(int unit, 
                       const taps_handle_t taps,
                       taps_sbucket_handle_t sbh,
                       /* out */
                       uint32 *pivot,
                       uint32 *pivot_len,
                       uint32 *bpm,
                       trie_node_t **split_trie_root)
{
    int rv = SOC_E_NONE;
    unsigned int max_split_len;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\n$$$$$ SBUCKET SPLIT $$$$$\n")));

    if (!pivot || !pivot_len || !taps) return SOC_E_PARAM;

    if (taps->param.mode == TAPS_OFFCHIP_ALL) {
        max_split_len = taps->param.key_attr.length;
    } else {
        max_split_len = taps->param.key_attr.length - _MIN_SBUCKET_KEY_LEN_;
    }
    rv = trie_split(sbh->trie, max_split_len, FALSE, pivot, pivot_len, split_trie_root, bpm, FALSE);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to split Sbucket trie %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
   
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_split_onchip_stage2
 * Purpose:
 *     Move entry from old_sbh to new_sbh
 */
int taps_sbucket_split_onchip_stage2(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       unsigned int domain, /* for split bucket */
                       trie_node_t *split_trie_root,
                       uint32 pivot_len,
                       taps_sbucket_handle_t *nsbh,
                       taps_spivot_handle_t *roll_back_sph)
{
    int rv = SOC_E_NONE, rv_in = SOC_E_NONE, update_roll_back_sph = FALSE;
    trie_t split_trie;
    trie_node_t *payload;
    taps_spivot_handle_t sph;
    unsigned int sph_id;
    unsigned int bpm[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 redist_sph_len = 0;
    uint32 redist_sph_pivot[TAPS_MAX_KEY_SIZE_WORDS];
    uint32 redist_sph_payload[_TAPS_PAYLOAD_WORDS_];
    void *redist_sph_cookie = NULL;
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "\n$$$$$ SBUCKET SPLIT $$$$$\n")));
    /*LOG_CLI((BSL_META_U(unit,
                          "\n$$$$$ SBUCKET SPLIT $$$$$\n")));*/

    if (!sbh || !split_trie_root || !nsbh || !taps || !wgroup) return SOC_E_PARAM;

    *nsbh = NULL;
            
    rv = taps_sbucket_create(unit, taps, wgroup, 
                             domain, NULL, FALSE, 
                             pivot_len, nsbh);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d new Sbucket allocation Failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }
    
    /* insert all sph to be moved into the new sbh */
    split_trie.trie = split_trie_root;
    split_trie.v6_key = sbh->trie->v6_key;
    while(1) {
        payload = NULL;
        sph = NULL;

        rv = trie_iter_get_first(&split_trie, &payload);
        if (rv == SOC_E_EMPTY) {
            /* no nodes left */
            rv = SOC_E_NONE;
            break;
        } else if (SOC_FAILURE(rv)) {
            /* failure */
            break;
        }

        sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);
        if (roll_back_sph != NULL) {
            if (*roll_back_sph == sph) {
                update_roll_back_sph = TRUE;
            }
        }
        sph_id = sph->index;
        redist_sph_len = sph->length;
        sal_memcpy(redist_sph_pivot, sph->pivot, sizeof(uint32) * TAPS_MAX_KEY_SIZE_WORDS);
        sal_memcpy(redist_sph_payload, sph->payload, sizeof(uint32) * _TAPS_PAYLOAD_WORDS_);
	redist_sph_cookie = sph->cookie;

        /* remove the prefix from old trie */
        rv = trie_delete(&split_trie, sph->pivot, sph->length, &payload);
        if (SOC_FAILURE(rv)) {
            /* failed to delete from old trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to delete trie node %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }

        rv = _taps_sbucket_enqueue_pivot_work(unit, taps, sbh, sph,
                                              wgroup,
                                              _BRR_INVALID_CPE_,
                                              FALSE); 
        if (SOC_FAILURE(rv)) {
            sal_memset(&bpm[0], 0, TAPS_MAX_KEY_SIZE_WORDS * sizeof(unsigned int));
            _TAPS_SET_KEY_BIT(bpm, 0, taps->param.key_attr.lookup_length);
            trie_insert(sbh->trie, sph->pivot, bpm, sph->length, &sph->node);
            /* failed to update sbucket cache of old bucket */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to update sbucket %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        } else {
            rv = taps_sbucket_pivot_free(unit, sbh, sph);
            if (SOC_FAILURE(rv)) {
                sal_memset(&bpm[0], 0, TAPS_MAX_KEY_SIZE_WORDS * sizeof(unsigned int));
                _TAPS_SET_KEY_BIT(bpm, 0, taps->param.key_attr.lookup_length);
                trie_insert(sbh->trie, sph->pivot, bpm, sph->length, &sph->node);
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d Failed to free sph handle %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
                break;
            } else {
                sph = NULL;
            }
        }   
        rv = taps_sbucket_pivot_id_free(unit, taps, sbh, sph_id);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to free sph id %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }       

        /* insert the prefix into trie of new dbucket */
        rv = taps_sbucket_pivot_id_alloc(unit, taps, *nsbh, &sph_id);
        if (SOC_FAILURE(rv)) {
           sph_id = _TAPS_INV_ID_;
           LOG_ERROR(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: unit %d vrf route failed to allocate spivot %d:%s !!!\n"), 
                      FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
           break;
        }

        sal_memset(&bpm[0], 0, TAPS_MAX_KEY_SIZE_WORDS * sizeof(unsigned int));
        _TAPS_SET_KEY_BIT(bpm, 0, taps->param.key_attr.lookup_length);
        rv = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                                       sph_id, *nsbh,
                                       &redist_sph_pivot[0], redist_sph_len,
                                       TRUE, &redist_sph_payload[0], redist_sph_cookie,
                                       NULL, &bpm[0], &sph);
        if (SOC_FAILURE(rv)) {
            /* failed to insert into new trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to insert prefix in new sbh %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            break;
        }
        if (update_roll_back_sph) {
            *roll_back_sph = sph;
            update_roll_back_sph= FALSE;
        }
    }

    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to move prefixes into new sbucket %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        /* Insert back current sph  */
        if (sph == NULL) {
            /* insert the prefix into trie of new dbucket */
            rv_in = taps_sbucket_pivot_id_alloc(unit, taps, sbh, &sph_id);
            if (SOC_FAILURE(rv_in)) {
               LOG_ERROR(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s: unit %d failed to allocate spivot %d:%s !!!\n"), 
                          FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
            rv_in = taps_sbucket_insert_pivot(unit, taps, wgroup, 
                               sph_id, sbh,
                               &redist_sph_pivot[0], redist_sph_len,
                               TRUE, &redist_sph_payload[0], redist_sph_cookie,
                               NULL, &bpm[0], &sph);
            if (SOC_FAILURE(rv_in)) {
               LOG_ERROR(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s: unit %d failed to insert spivot %d:%s !!!\n"), 
                          FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }
        }
        
        /* Insert back the prefix which is still in the split trie */
        while(1) {
            payload = NULL;
            sph = NULL;

            rv_in = trie_iter_get_first(&split_trie, &payload);
            if (rv_in == SOC_E_EMPTY) {
                /* no nodes left */
                rv_in = SOC_E_NONE;
                break;
            } else if (SOC_FAILURE(rv_in)) {
                /* failure */
                break;
            }

            sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);
            /* remove the prefix from old trie */
            trie_delete(&split_trie, sph->pivot, sph->length, &payload);

            sal_memset(&bpm[0], 0, TAPS_MAX_KEY_SIZE_WORDS * sizeof(unsigned int));
            _TAPS_SET_KEY_BIT(bpm, 0, taps->param.key_attr.lookup_length);
            trie_insert(sbh->trie, sph->pivot, &(bpm[0]), sph->length, &sph->node);
        }
        /* Insert back the prefix which is in the new sbucket */
        taps_sbucket_merge_for_onchip(unit, taps, wgroup, sbh, *nsbh, roll_back_sph);
        trie_destroy((*nsbh)->trie);
        _hpcm_sbucket_free(unit, (*nsbh));
        *nsbh = NULL;
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_split_offchip_stage2
 * Purpose:
 *     Move entry from old_sbh to new_sbh
 */
int taps_sbucket_split_offchip_stage2(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       unsigned int domain, /* for split bucket */
                       trie_node_t *split_trie_root,
                       taps_dbucket_handle_t wdbh,
                       taps_sbucket_handle_t *nsbh)
{
    int rv = SOC_E_NONE, rv_in = SOC_E_NONE;

    if (!sbh || !split_trie_root || !nsbh || !taps || !wgroup) return SOC_E_PARAM;

    *nsbh = NULL;

    rv = taps_sbucket_create(unit, taps, wgroup, 
                             domain, wdbh, 
                             FALSE, 0, nsbh);
    if (SOC_SUCCESS(rv)) {
        sbucket_clone_datum_t datum;

        (*nsbh)->trie->trie = split_trie_root;
        (*nsbh)->trie->v6_key = sbh->trie->v6_key;
        (*nsbh)->domain = domain;
        /**** sal_memset((*nsbh)->pivot_bmap, 0, BITS2BYTES(_MAX_SBUCKET_PIVOT_));*/

    /* insert on to the sbucket trie so the dram bucket could be used to house prefix
     * NOTE: it's impossible for the split_trie_root to be at wsph position (id 0).
     * if the split_trie_root is payload node, the * entry will have an empty dbucket
     * and will not be used for future insertion/search since the split_trie_root
     * will be a longer match than the wsph. This surely will waste a spivot, but will
     * simplify the management a lot.
     */
        rv = trie_insert((*nsbh)->trie, &(*nsbh)->wsph->pivot[0], 
                         &(*nsbh)->wsph->pivot[0], 0, 
                         &(*nsbh)->wsph->node);

        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Failed to insert the sbucket trie so the dram bucket could be used to house prefix %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv))); 
        }


        /*taps_sbucket_dump(unit, taps, *nsbh, 1);*/

        datum.common.unit = unit;
        datum.common.taps = (taps_handle_t) taps;
        datum.common.wgroup = wgroup;
        datum.sbh  = sbh;
        datum.nsbh = *nsbh;

        rv = trie_traverse((*nsbh)->trie, _taps_sbucket_pivot_clone, 
                           &datum, _TRIE_PREORDER_TRAVERSE);
        if (SOC_FAILURE(rv)) {
            /* try to recover bit map & un-split the trie */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d Hardware bucket split Failed %d:%s !!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            rv_in = taps_sbucket_trie_merge(unit, taps, sbh, *nsbh);
            if (SOC_FAILURE(rv_in)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: unit %d  Sbucket merge failed %d:%s !!!\n"), 
                           FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            }

            trie_destroy((*nsbh)->trie);
            _hpcm_sbucket_free(unit, (*nsbh));
            *nsbh = NULL;
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d new Sbucket allocation Failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }

    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_domain_move
 * Purpose:
 *     Move whole domain to new_sbh
 */
int taps_sbucket_domain_move(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       unsigned int domain)
{
    int rv = SOC_E_NONE;
    sbucket_clone_datum_t datum;
    taps_sbucket_t old_sbh;

    if (!sbh || !taps || !wgroup) return SOC_E_PARAM;

    /* Store the sbh*/
    old_sbh = *sbh;

    /* Update domain of the new sbh*/
    sbh->domain = domain;
    
    datum.common.unit = unit;
    datum.common.taps = (taps_handle_t) taps;
    datum.common.wgroup = wgroup;
    datum.sbh  = &old_sbh;
    datum.nsbh = sbh;

    rv = trie_traverse(sbh->trie, _taps_sbucket_pivot_clone, 
                       &datum, _TRIE_PREORDER_TRAVERSE);
    if (SOC_FAILURE(rv)) {
        /* try to recover bit map & un-split the trie */
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Hardware bucket split Failed %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    if (taps->param.mode == TAPS_ONCHIP_ALL 
        && sbh->wsph->length == 0) {
        /* For mode zero, we need to move the wild sph */
        rv = _taps_sbucket_enqueue_pivot_work(unit, taps, sbh, sbh->wsph,
                                            wgroup, sbh->wsph->payload[0], TRUE);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s: unit %d failed to update asso data for mode 0 %d:%s!!!\n"), 
                       FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
            return rv;
        } 
    }
    
    return SOC_E_NONE;
}

int _taps_sbucket_propagate_prefix_cb(trie_node_t *trie_node, trie_bpm_cb_info_t *info) 
{
    taps_spivot_handle_t sph; 
    int rv = SOC_E_NONE;
    sbucket_propagate_datum_t *datum;

    if (trie_node) {
        datum = (sbucket_propagate_datum_t*) info->user_data;
        sph = TRIE_ELEMENT_GET(taps_spivot_t*, trie_node, node);

        if (SOC_SUCCESS(rv)) {
            /* Queue up a propagation work item to update bpm of this pivot */
            rv = _taps_sbucket_enqueue_propagate_work(datum->common.unit, datum->common.taps,
                                                      sph->sbh, sph, datum->common.wgroup,
                                                      datum->ads_local_pointer);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: unit %d failed to enqueue propage work item: %d:%s !!!\n"), 
                           FUNCTION_NAME(), datum->common.unit, rv, soc_errmsg(rv)));
            } else {
                if (datum->add) {
                    *datum->isbpm = TRUE; /* this is a best prefix match for this pivot */
                }
            }
        }
    }
    
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_propagate_prefix
 * Purpose:
 *     Propagate BPM for inserted prefix 
 * TBD: consider passing local pointer to this routine if feasible
 */
int taps_sbucket_propagate_prefix(int unit, 
                                  const taps_handle_t taps,
                                  const taps_sbucket_handle_t sbh,
                                  const taps_wgroup_handle_t *wgroup,
                                  uint32  *prefix,
                                  uint32   prefix_len,
                  uint32   local_pointer,
                                  /* >0 - add prefix, 0 - delete prefix */
                                  unsigned int add,
                                  /* OUT */
                                  uint8 *isbpm)
{
    int rv = SOC_E_NONE;
    trie_bpm_cb_info_t info;
    sbucket_propagate_datum_t datum;

    if (!taps || !sbh || !wgroup || !prefix || !isbpm) return SOC_E_PARAM;

    sal_memset(&datum, 0, sizeof(datum));
    datum.common.unit = unit;
    datum.common.taps = (taps_handle_t) taps;
    datum.common.wgroup = wgroup;
    datum.isbpm = isbpm;
    datum.add = add;
    datum.ads_local_pointer = _BRR_INVALID_CPE_;
    info.user_data = &datum;
    info.pfx = prefix;
    info.len = prefix_len;
    
    if (local_pointer == _BRR_INVALID_CPE_) {
    /* There are some scope of optimization such as expecting the pivot from the caller
     * and propagating at arbitrary point of trie. But for now since this interface is doing
     * add and del propagate, always start from the top of trie */
    if (add) {
        rv = taps_find_prefix_local_pointer(unit, taps, 
                        prefix, prefix_len,
                        &datum.ads_local_pointer);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to find taps bpm prefix %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        }
    } else {
        uint32 bpm_key[BITS2WORDS(TAPS_IPV6_KEY_SIZE)], word_idx, del_bpm_len=0;
        
        for (word_idx = 0; word_idx < BITS2WORDS(taps->param.key_attr.lookup_length); word_idx++) {
        bpm_key[word_idx] = prefix[word_idx];
        }
        
        rv = taps_key_shift(taps->param.key_attr.lookup_length, bpm_key, prefix_len, 1);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to construct BPM key\n"),
                   FUNCTION_NAME(), unit));
        return rv;  
        }
        
        rv = _taps_sbucket_find_bpm(unit, taps, sbh,
                    bpm_key, prefix_len - 1,
                    &del_bpm_len,
                    &datum.ads_local_pointer);
        if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Failed to find delete BPM key\n"),
                   FUNCTION_NAME(), unit));
        return rv;  
        }
    }
    } else {
    datum.ads_local_pointer = local_pointer;
    }

    if (SOC_SUCCESS(rv)) {
        rv = trie_propagate_prefix(sbh->trie, prefix, prefix_len, add,
                                   _taps_sbucket_propagate_prefix_cb, &info);
    }
    
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_stat_get
 * Purpose:
 *     get status of sram bucket
 */
int taps_sbucket_stat_get(int unit,
                          taps_sbucket_handle_t sbh,
                          taps_bucket_stat_e_t *stat)
{
    int count=0;

    if (!sbh || !stat) return SOC_E_PARAM;

    SHR_BITCOUNT_RANGE(sbh->pivot_bmap, count, 0, sbh->prefix_number);
    
    if (count == 0) {
        *stat = _TAPS_BKT_EMPTY;
    } else if (sbh->prefix_number == count) {
        *stat = _TAPS_BKT_FULL;
    } else if (count == 1) {
        *stat = _TAPS_BKT_WILD;
    } else if (count == (sbh->prefix_number-1)) {
        *stat = _TAPS_BKT_ALMOST_FULL;
    } else if (count < sbh->prefix_number) {
        *stat = _TAPS_BKT_VACANCY;
    } else {
        *stat = _TAPS_BKT_ERROR;
    }

    return SOC_E_NONE;
}

int _taps_sbucket_traverse(trie_node_t *payload, void *trav_datum)
{
    taps_spivot_handle_t  sph;
    sbucket_traverse_datum_t *datum=NULL;
    int rv=SOC_E_NONE;

    if (!payload || !trav_datum) return SOC_E_PARAM;
    if (payload->type != PAYLOAD) return SOC_E_NONE;

    datum = (sbucket_traverse_datum_t*)trav_datum;
    sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, payload, node);

    if (datum->spivots == NULL) {
	/* no accumulation, call back directly */
	if (datum->user_cb != NULL) {
	    rv = datum->user_cb(sph, datum->user_data);
	}
    } else {
	/* accumulate and call back after traverse is done */
	datum->spivots[datum->num_spivots++] = sph;
    }
    return rv;
}


int taps_sbucket_traverse(int unit, 
                          const taps_handle_t taps,
                          const taps_wgroup_handle_t *wgroup,
                          const taps_sbucket_handle_t sbh, 
                          void *user_data,
                          sbucket_traverse_cb_f cb)
{
    int rv = SOC_E_NONE;
    sbucket_traverse_datum_t datum;

    if (!sbh || !cb) return SOC_E_PARAM;

    datum.user_data = user_data;
    datum.user_cb = cb;
    datum.common.unit = unit;
    datum.common.taps = (taps_handle_t) taps;
    datum.common.wgroup = wgroup;
    datum.num_spivots = 0;
    datum.spivots = NULL;
    
    rv = trie_traverse(sbh->trie, _taps_sbucket_traverse,
                       &datum, _TRIE_PREORDER_TRAVERSE);
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_destroy_traverse
 * Purpose:
 *     Traverse the sram bucket & invoke call back
 */
int taps_sbucket_destroy_traverse(int unit, 
                                  const taps_handle_t taps,
                                  const taps_wgroup_handle_t *wgroup,
                                  const taps_sbucket_handle_t sbh, 
                                  void *user_data,
                                  sbucket_traverse_cb_f cb)
{
    int rv = SOC_E_NONE, index;
    sbucket_traverse_datum_t datum;

    if (!sbh || !cb) return SOC_E_PARAM;

    datum.user_data = user_data;
    datum.user_cb = cb;
    datum.common.unit = unit;
    datum.common.taps = (taps_handle_t) taps;
    datum.common.wgroup = wgroup;
    datum.num_spivots = 0;
    datum.spivots = sal_alloc(sizeof(taps_spivot_handle_t) * (taps->param.sbucket_attr.max_pivot_number+1),
			      "taps-sbucket-traverse");;

    /* traverse trie to get all sph */
    rv = trie_traverse(sbh->trie, _taps_sbucket_traverse, 
                       &datum, _TRIE_POSTORDER_TRAVERSE);

    if (SOC_SUCCESS(rv)) {
	if (datum.num_spivots <= taps->param.sbucket_attr.max_pivot_number) {
	    /* call back on all sph accumulated */
	    for (index = 0; index < datum.num_spivots; index++) {
		rv = cb(datum.spivots[index], user_data);
		if (SOC_FAILURE(rv)) {
		    break;
		}
	    }
	} else {
	    /* we should never get more than what dbucket can hold */
	    rv = SOC_E_INTERNAL;
	}
    }

    if (datum.spivots) {
	sal_free(datum.spivots);
    }
    return rv;
}

/*
 *
 * Function:
 *     taps_sbucket_pm_traverse
 * Purpose:
 *     Search the sram bucket & invoke call back on all 
 *     prefixes matching the specified prefix
 */
int taps_sbucket_pm_traverse(int unit, 
                 const taps_handle_t taps,
                 const taps_wgroup_handle_t *wgroup,
                 const taps_sbucket_handle_t sbh, 
                 uint32 *pivot,
                 uint32 pivot_len,
                 void *user_data,
                 sbucket_traverse_cb_f cb)
{
    int rv = SOC_E_NONE;
    sbucket_traverse_datum_t datum;

    if (!sbh || !cb) return SOC_E_PARAM;

    datum.user_data = user_data;
    datum.user_cb = cb;
    datum.common.unit = unit;
    datum.common.taps = (taps_handle_t) taps;
    datum.common.wgroup = wgroup;
    datum.num_spivots = 0;
    datum.spivots = NULL;
    
    rv = trie_find_pm(sbh->trie, pivot, pivot_len,
              _taps_sbucket_traverse, &datum);
    return rv;
}

/*
 *   Function
 *      taps_sbucket_first_pivot_handle_get
 *   Purpose
 *      Get the next sram pivot handle
 *   Parameters
 *      (IN) unit               : Unit number
 *      (IN) taps               : Valid taps
 *      (IN) sbh                : Specified sbucket
 *      (OUT) sph             : First sram pivot handle
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */
int taps_sbucket_first_pivot_handle_get(int unit,
                                        taps_handle_t taps,         
                                        taps_sbucket_handle_t sbh, 
                                        taps_spivot_handle_t *sph)
{
    int rv = SOC_E_NONE;
    int start_index;

    start_index = 0; 

    rv = taps_sbucket_next_pivot_handle_get(taps, start_index, sbh, sph);
    return rv;
}

/*
 *   Function
 *      _taps_sbucket_next_handle_get
 *   Purpose
 *      Get the next sram pivot handle
 *   Parameters

 *      (IN) sbh               : Specified sbh
 *      (IN) start_index     : Beginning in this sbucket
 *      (OUT) sph             : Next sram pivot handle
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */
static int _taps_sbucket_next_handle_get(taps_sbucket_handle_t sbh,
                                        int start_index,
                                        taps_spivot_handle_t *sph) 
{
    int rv = SOC_E_NONE;
    trie_node_t *next_node = NULL;
    rv = taps_bucket_next_node_get(sbh->trie, SRAM_PIVOT_TRIE,
                                sbh->pivot_bmap, start_index, 
                                sbh->prefix_number, &next_node);
    if(SOC_SUCCESS(rv)) {
        *sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, next_node, node);
    } 
    
    return rv;
}

/*
 *   Function
 *      _taps_sbucket_next_bucket_get
 *   Purpose
 *      Get the next sbucket
 *   Parameters

 *      (IN) taps               : Valid taps
 *      (IN) start_index     : Beginning in this taps
 *      (OUT) next_sbh      : Next sbucket
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */
static int _taps_sbucket_next_bucket_get(taps_handle_t taps,
                                            int start_index,
                                            taps_sbucket_handle_t *next_sbh)
{
    int rv = SOC_E_NONE;
    trie_node_t *next_node = NULL;
    taps_tcam_pivot_handle_t tph = NULL;
    rv = taps_bucket_next_node_get(taps->tcam_hdl->trie, 
                                TCAM_PIVOT_TRIE,
                                taps->allocator, 
                                start_index, 
                                taps->param.seg_attr.seginfo[taps->param.instance].num_entry,
                                &next_node);
    if(SOC_SUCCESS(rv)) {
        tph = TRIE_ELEMENT_GET(taps_tcam_pivot_handle_t, next_node, node);
        *next_sbh = tph->sbucket;
    }
    
    return rv;
}

/*
 *   Function
 *      taps_sbucket_first_pivot_get
 *   Purpose
 *      Get the next spivot in a specified sbucket
 *   Parameters

 *      (IN) taps               : Valid taps
 *      (IN) start_index     : Beginning of this sbucket
 *      (IN) sbh                : Specified sbucket
 *      (OUT) sph             :  SRAM pivot handle
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */
int taps_sbucket_next_pivot_handle_get(taps_handle_t taps, 
                                        int start_index, 
                                        taps_sbucket_handle_t sbh,
                                        taps_spivot_handle_t *sph)
{
    int rv = SOC_E_NONE;
    int found = FALSE;
    taps_sbucket_handle_t cur_sbh = NULL;
    int next_domain_id = 0;
    int iterator_index = 0;
    int wild_entry_domainid = 0;

    if (taps->param.mode != TAPS_OFFCHIP_ALL) {
        if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
            wild_entry_domainid = 0;
        } else {
            wild_entry_domainid = taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry;
        }
    }
    
    iterator_index = start_index;
    cur_sbh = sbh;
    while (!found) {
        if ((taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS 
            && cur_sbh->domain == wild_entry_domainid 
            && iterator_index == 0) || 
            (taps->param.mode == TAPS_ONCHIP_ALL 
            && cur_sbh->wsph->length == 0
            && iterator_index == 0)) {
            /* Skip wild entry since it don't  exist in the trie, but set bit in bitmap field*/
            iterator_index = 1;
        }
            
        rv = _taps_sbucket_next_handle_get(cur_sbh, 
                                iterator_index, sph);
        if (SOC_SUCCESS(rv)) {
            /* Get sph successful*/
            found = TRUE;
        } else if (rv == SOC_E_NOT_FOUND) {
            /* Not found in this sbucket. Get the next sbucket */
            next_domain_id = cur_sbh->domain + 1;
            iterator_index = 0;
            rv = _taps_sbucket_next_bucket_get(taps, next_domain_id, &cur_sbh);
            if (SOC_FAILURE(rv)) {
                if (rv == SOC_E_NOT_FOUND) {
                    /* No more sbucket in the taps, traverse complete*/
                } else {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: No prefix in the taps %d:%s !!!\n"), 
                               FUNCTION_NAME(), rv, soc_errmsg(rv)));
                }
                break;
            }
        } else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error in find next sbucket handle %d:%s !!!\n"), 
                       FUNCTION_NAME(), rv, soc_errmsg(rv)));
        }
    }
    return rv;
}

/*
 *   Function
 *      taps_sbucket_first_pivot_get
 *   Purpose
 *      Get the next spivot in a specified sbucket
 *   Parameters

 *      (IN) taps               : Valid taps
 *      (IN) start_index     : Beginning of this sbucket
 *      (IN) sbh                : Specified sbucket
 *      (OUT) key             : First key 
 *      (OUT) key_length   : First key length
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */
int taps_sbucket_first_pivot_get(int unit,
                                    taps_handle_t taps,         
                                    taps_sbucket_handle_t sbh, 
                                    uint32 *key,
                                    uint32 *key_length)
{
    int rv = SOC_E_NONE;
    taps_spivot_handle_t sph;
    
    rv = taps_sbucket_first_pivot_handle_get(unit, taps, sbh, &sph);
    if (SOC_SUCCESS(rv)) {
        *key_length = sph->length;
        sal_memcpy(key, sph->pivot, 
            BITS2WORDS(taps->param.key_attr.lookup_length) * sizeof(uint32));
    } else if (rv == SOC_E_NOT_FOUND) {
        /* No more key, traverse complete. Just return SOC_E_NOT_FOUND*/
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Fail to get the first sph %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    }
    return rv;
}

/*
 *   Function
 *      taps_sbucket_next_pivot_get
 *   Purpose
 *      Get the next spivot in a specified sbucket
 *   Parameters

 *      (IN) taps               : Valid taps
 *      (IN) start_index     : Beginning of this sbucket
 *      (IN) sbh                : Specified sbucket
 *      (OUT) key             : Next key 
 *      (OUT) key_length   : Next key length
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */
int taps_sbucket_next_pivot_get(taps_handle_t taps, 
                                    int start_index, 
                                    taps_sbucket_handle_t sbh,
                                    uint32 *key,
                                    uint32 *key_length) 
{
    int rv = SOC_E_NONE;
    taps_spivot_handle_t sph;
    rv = taps_sbucket_next_pivot_handle_get(taps, start_index, sbh, &sph);
    if (SOC_SUCCESS(rv)) {
        *key_length = sph->length;
        sal_memcpy(key, sph->pivot, BITS2WORDS(taps->param.key_attr.lookup_length) * sizeof(uint32));
    } else if (rv == SOC_E_NOT_FOUND) {
        /* No more key, traverse complete. Just return SOC_E_NOT_FOUND*/
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: Fail to get spivot handle %d:%s !!!\n"), 
                   FUNCTION_NAME(), rv, soc_errmsg(rv)));
    }
    return rv;
}

/*
 *   Function
 *      taps_sbucket_first_bucket_handle_get
 *   Purpose
 *      Get the first SRAM bucket in the specified taps segment
 *   Parameters
 *      (IN) unit              : unit number of the device
 *      (IN) taps             : Valid taps
 *      (OUT) sbh            : First sbucket
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */

int taps_sbucket_first_bucket_handle_get(int unit, 
                                        taps_handle_t taps, 
                                        taps_sbucket_handle_t *sbh)
{
    int rv = SOC_E_NONE;
    int next_domain_id;

    /* Start to search from sbucket 0 */
    next_domain_id = 0;
     
    rv = _taps_sbucket_next_bucket_get(taps, next_domain_id, sbh);

    return rv;
}

/*
 *   Function
 *      taps_sbucket_instance_and_bucket_id_get
 *   Purpose
 *      Get the hardware instance and hardware sbucket id
 *   Parameters
 *      (IN) unit              : unit number of the device
 *      (IN) taps             : Valid taps
 *      (IN) sbucket_id      : original sbucket_id
 *      (OUT) hwinstance     : hardware instance 
 *      (OUT) hw_sbucket_id  : hardware sbucket id
 *   Returns
 *       SOC_E_NONE - successfully rehashed.
 *       SOC_E_* as appropriate otherwise
 */

int taps_sbucket_instance_and_bucket_id_get(int unit, const taps_handle_t taps, 
                                int sbucket_id, int *hwinstance, int *hw_sbucket_id)
{
    if (hwinstance == NULL
        || hw_sbucket_id == NULL) {
        return SOC_E_PARAM;
    }
    if (_TAPS_IS_PARALLEL_MODE_(taps->param.instance)) {
       *hwinstance = taps->hwinstance;
       *hw_sbucket_id = sbucket_id;
    } else {
        if (sbucket_id < taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry) {
            *hwinstance = SW_INST_CONVERT_TO_HW_INST(TAPS_INST_0);
            *hw_sbucket_id = sbucket_id;
        } else {
            *hwinstance = SW_INST_CONVERT_TO_HW_INST(TAPS_INST_1);
            *hw_sbucket_id = sbucket_id
                    - taps->param.seg_attr.seginfo[TAPS_INST_0].num_entry;
        }
    }
    return SOC_E_NONE;
}
/*
 * info passed for the bucket dump callback
 */
typedef struct _taps_dump_sbkt_info_s {
    int unit;
    taps_handle_t taps;
    uint32 flags;
} _taps_dump_sbkt_info_t;

/*
 * Call back function for sbucket dump
 */
static int _taps_sbkt_dump_cb(trie_node_t *node, void *user_data)

{
    int rv = SOC_E_NONE;

    if (node && user_data) {
        if (node->type == PAYLOAD) {
            _taps_dump_sbkt_info_t *info = (_taps_dump_sbkt_info_t *)user_data;
            taps_spivot_handle_t sph = TRIE_ELEMENT_GET(taps_spivot_handle_t, node, node);
            rv = taps_sbucket_pivot_dump(info->unit, info->taps, sph, info->flags);
        } else {
            LOG_CLI((BSL_META("+ sbkt trie_node skip_len %d skip_addr 0x%x type INTERNAL count %d bpm 0x%08x\n"),
                     node->skip_len, node->skip_addr, node->count, node->bpm));
        }
    }

    return rv;
}

/*
 * Dump a SBucket software and hardware state
 */
int taps_sbucket_dump(int unit, const taps_handle_t taps,
              const taps_sbucket_handle_t sbh,
              uint32 flags)
{
    int rv = SOC_E_NONE;
    _taps_dump_sbkt_info_t info;

    if (!taps || !sbh) return SOC_E_PARAM;
    if (!TAPS_DUMP_SRAM_FLAGS(flags)) return SOC_E_NONE;

    /* dump bucket info */
    if (flags & TAPS_DUMP_SRAM_SW_BKT) {
        LOG_CLI((BSL_META_U(unit,
                            "\n#### Sbucket: %d %p dump ####\n"), sbh->domain, (void *)sbh));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Dumping unit %d Taps handle 0x%x SBH handle 0x%x \n"),
                 FUNCTION_NAME(), unit, (uint32)taps, (uint32)sbh));
        LOG_CLI((BSL_META_U(unit,
                            "%s: domain %d WSPH handle 0x%x \n"),
                 FUNCTION_NAME(), sbh->domain, (uint32)(sbh->wsph)));
        LOG_CLI((BSL_META_U(unit,
                            "%s: pivot bmap 0x%8x %x \n"),
                 FUNCTION_NAME(), sbh->pivot_bmap[1], sbh->pivot_bmap[0]));
        LOG_CLI((BSL_META_U(unit,
                            "Total number of Sram pivots: %d \n"), sbh->trie->trie->count));
    }

    /* dump all pivots */
    info.unit = unit;
    info.taps = taps;
    info.flags = flags;
    trie_dump(sbh->trie, _taps_sbkt_dump_cb, &info);
    return rv;
}


/*
 * Dump a SBucket SPH software and hardware state
 */
int taps_sbucket_pivot_dump(int unit, const taps_handle_t taps,
                const taps_spivot_handle_t sph,
                            uint32 flags)
{
    int rv = SOC_E_NONE;
    int hw_instance, hw_bucket_id;

    if (!taps || !sph) return SOC_E_PARAM;
    if (!(flags & TAPS_DUMP_SRAM_SW_PIVOT)) return SOC_E_NONE;

    /* dump software info */
    if (flags & TAPS_DUMP_SRAM_VERB) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: Dumping unit %d Taps handle 0x%x SPH handle 0x%x \n"),
                 FUNCTION_NAME(), unit, (uint32)taps, (uint32)sph));
        LOG_CLI((BSL_META_U(unit,
                            "%s: Assocated SBucket handle 0x%x DBucket handle 0x%x\n"),
                 FUNCTION_NAME(), (uint32)sph->sbh, (uint32)sph->dbh));
        LOG_CLI((BSL_META_U(unit,
                            "%s: trie_node skip_len %d skip_addr 0x%x type %s count %d bpm 0x%08x\n"),
                 FUNCTION_NAME(), sph->node.skip_len, sph->node.skip_addr,
                 (sph->node.type==PAYLOAD)?"PAYLOAD":"INTERNAL", sph->node.count, sph->node.bpm));
        LOG_CLI((BSL_META_U(unit,
                            "%s: pivot is_prefix %s\n"), FUNCTION_NAME(), (sph->is_prefix)?"TRUE":"FALSE"));
    }

    if (taps->param.key_attr.type == TAPS_IPV4_KEY_TYPE) {
    LOG_CLI((BSL_META_U(unit,
                        "%s: key_length %d key_data 0x%04x %08x\n"),
             FUNCTION_NAME(), sph->length, sph->pivot[0], sph->pivot[1]));
    } else {
    LOG_CLI((BSL_META_U(unit,
                        "%s: key_length %d key_data 0x%04x %08x %08x %08x %08x\n"),
             FUNCTION_NAME(), sph->length, sph->pivot[0], sph->pivot[1],
             sph->pivot[2], sph->pivot[3], sph->pivot[4]));
    }    

    
    /* Get hardware instance and entry id */
    rv = taps_sbucket_instance_and_bucket_id_get(unit, taps, 
                                   sph->sbh->domain, &hw_instance, &hw_bucket_id);
    if (SOC_FAILURE(rv)) {
       LOG_ERROR(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s: unit %d Failed to get hardware instance and entry id\n"),
                  FUNCTION_NAME(), unit));
       return rv;
    }

    /* dump associated hw entry */
    rv = taps_sbucket_entry_dump(unit, hw_instance, taps->segment,
                 hw_bucket_id, sph->index, sph->sbh->sbucket_format,
                 flags);
    if (SOC_FAILURE(rv)) {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d Failed to dump sbucket %d hardware entry %d !!!\n"), 
               FUNCTION_NAME(), unit, sph->sbh->domain, sph->index));
    return rv;
    }

    if (TAPS_DUMP_DRAM_FLAGS(flags)) {
        if (taps->param.mode == TAPS_ONCHIP_SEARCH_OFFCHIP_ADS) {
            rv = taps_dbucket_payload_entry_dump(unit, taps, sph->sbh->domain, sph->index, flags);
        } else {
            rv = taps_dbucket_dump(unit, taps, sph->dbh, flags);
        }
    }
    
    return rv;
}

/*
 *   Function
 *      taps_sbucket_entry_dump
 *   Purpose
 *      Dump a Sbucket SPH hardware state
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) taps_instance   : taps instance
 *      (IN) segment: segment
 *      (IN) bucket : sram bucket id
 *      (IN) entry  : SPH index in sram bucket
 *      (IN) level  : dump level
 *   Returns
 *       SOC_E_NONE - successfully relocated
 *       SOC_E_* as appropriate otherwise
 */
int taps_sbucket_entry_dump(int unit, int taps_instance, int segment,
                int bucket, int entry, int format,
                            uint32 flags)
{
    int rv = SOC_E_NONE;
    soc_sbx_caladan3_tmu_cmd_t *bbx=NULL, *brr=NULL;
    uint32    mem_val[SOC_MAX_MEM_WORDS*2];

    if (!(flags & TAPS_DUMP_SRAM_HW_PIVOT)) return SOC_E_NONE;

    /* if (SAL_BOOT_PLISIM) return SOC_E_NONE; */

    /* check the format and nothing else */
    if (!_TAPS_SUPPORTED_BBX_FORMAT_(format)) {
        rv = SOC_E_PARAM;
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d unsupported sram BBX format %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
        return rv;
    }

    LOG_CLI((BSL_META_U(unit,
                        "%s: Dumping unit %d Taps %d segment %d bucket %d entry "
                        "%d with format %d\n"),
             FUNCTION_NAME(), unit, (taps_instance-1), segment,
             bucket, entry, format));

    /* Read BBX */
    rv = tmu_cmd_alloc(unit, &bbx); 
    if (SOC_FAILURE(rv) || (bbx == NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate internal TMU commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    }

    bbx->opcode = SOC_SBX_TMU_CMD_TAPS;
    bbx->cmd.taps.instance = taps_instance;
    bbx->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BB;
    bbx->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BB_SUBCMD_READ;
    if (format == SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES) {
    bbx->cmd.taps.max_key_size = TAPS_IPV6_KEY_SIZE;
    } else {
    bbx->cmd.taps.max_key_size = TAPS_IPV4_KEY_SIZE;
    }
    bbx->cmd.taps.subcmd.bb_read.segment = segment;
    bbx->cmd.taps.subcmd.bb_read.offset = bucket;
    bbx->cmd.taps.subcmd.bb_read.prefix_id = entry;
    bbx->cmd.taps.subcmd.bb_read.format = format;
    bbx->cmd.taps.subcmd.bb_read.kshift = 0;
    bbx->cmd.taps.subcmd.bb_read.align_right = FALSE;

    sal_memset(mem_val, 0, sizeof(mem_val));
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                                       bbx, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
    rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                       bbx, mem_val, SOC_MAX_MEM_WORDS*2);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d Invalid response !!!\n"), 
                   FUNCTION_NAME(), unit));
        return rv;
    }
    tmu_cmd_free(unit, bbx);
    } else {
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "%s: unit %d Failed to post TAPS BBX read command !!!\n"), 
               FUNCTION_NAME(), unit));
    return rv;
    }

    LOG_CLI((BSL_META_U(unit,
                        "%s: key_length %d key_data 0x%04x %08x %08x %08x %08x\n"),
             FUNCTION_NAME(),
             soc_mem_field32_get(unit, TAPS_BB_CMD_WRITEm, mem_val, PLENGTHf),
             soc_mem_field32_get(unit, TAPS_BB_CMD_WRITEm, mem_val, PDATA143_128f),
             soc_mem_field32_get(unit, TAPS_BB_CMD_WRITEm, mem_val, PDATA127_96f),
             soc_mem_field32_get(unit, TAPS_BB_CMD_WRITEm, mem_val, PDATA95_64f),
             soc_mem_field32_get(unit, TAPS_BB_CMD_WRITEm, mem_val, PDATA63_32f),
             soc_mem_field32_get(unit, TAPS_BB_CMD_WRITEm, mem_val, PDATA31_0f)));

    if (bbx) {
    tmu_cmd_free(unit, bbx);    
    }

    /* Read BRR */
    rv = tmu_cmd_alloc(unit, &brr); 
    if (SOC_FAILURE(rv) || (brr == NULL)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: unit %d failed to allocate internal TMU commands %d:%s !!!\n"), 
                   FUNCTION_NAME(), unit, rv, soc_errmsg(rv)));
    return rv;
    }

    brr->opcode = SOC_SBX_TMU_CMD_TAPS;
    brr->cmd.taps.instance = taps_instance;
    brr->cmd.taps.blk = SOC_SBX_TMU_TAPS_BLOCK_BRR;
    brr->cmd.taps.opcode = SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ;
    if (format == SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES) {
    brr->cmd.taps.max_key_size = TAPS_IPV6_KEY_SIZE;
    } else {
    brr->cmd.taps.max_key_size = TAPS_IPV4_KEY_SIZE;
    }
    brr->cmd.taps.subcmd.brr_read.segment = segment;
    brr->cmd.taps.subcmd.brr_read.offset = bucket;
    brr->cmd.taps.subcmd.brr_read.prefix_id = entry;
    brr->cmd.taps.subcmd.brr_read.format = format;

    sal_memset(mem_val, 0, sizeof(mem_val));
    rv = soc_sbx_caladan3_tmu_post_cmd(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO, 
                                       brr, SOC_SBX_TMU_CMD_POST_FLAG_NONE);
    if (SOC_SUCCESS(rv)) {
    rv = soc_sbx_caladan3_tmu_get_resp(unit, SOC_SBX_CALADAN3_TMU_DEF_CMD_FIFO,
                       brr, mem_val, SOC_MAX_MEM_WORDS*2);
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
                          "%s: unit %d Failed to post TAPS BRR read command !!!\n"), 
               FUNCTION_NAME(), unit));
    return rv;
    }

    LOG_CLI((BSL_META_U(unit,
                        "%s: associated data 0%08x\n"),
             FUNCTION_NAME(),
             soc_mem_field32_get(unit, TAPS_BRR_CMD_WRITEm, mem_val, ADATAf)));
    tmu_cmd_free(unit, brr);

    return rv;
}

int taps_sbucket_entry_range_dump(int unit, int taps_instance, int segment,
				  int bucket, int start_entry, int end_entry, int format,
				  uint32 flags)
{
    int entry, rv = SOC_E_NONE;
    for (entry = start_entry; entry <= end_entry; entry++) {
	rv = taps_sbucket_entry_dump(unit, taps_instance, segment, bucket, entry, format, flags);
	if (SOC_FAILURE(rv)) {
	    break;
	}
    }
    return rv;
}

#endif /* BCM_CALADAN3_SUPPORT */
