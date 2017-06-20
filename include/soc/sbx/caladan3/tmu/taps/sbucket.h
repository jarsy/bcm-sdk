/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: sbucket.h,v 1.40.12.1 Broadcom SDK $
 *
 * TAPS sram bucket defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TAPS_SBUCKET_H_
#define _SBX_CALADN3_TAPS_SBUCKET_H_

#include <soc/sbx/caladan3/tmu/taps/taps.h>

#define _SBKT_WILD_CHAR_RSVD_POS_ (0)
#define _BRR_INVALID_CPE_ (0x1FFFF)

/* Sram pivot handle object & interfaces */
struct taps_spivot_s {
    trie_node_t           node;
    taps_sbucket_handle_t sbh;   /* parent buckets handle */
    taps_dbucket_handle_t dbh;   /* dram bucket handle */
    uint16                index; /* pivot index with sram bucket */
    uint8                 is_prefix; /* is this pivot a prefix */
    unsigned int          payload[_TAPS_PAYLOAD_WORDS_]; /* for mode 0/1 */
    void                  *cookie;
    unsigned int          length; /* pivot length */
    unsigned int          pivot[1];  /* pivot - array size based on max key length */
};

struct taps_sbucket_s {
    trie_t     *trie; /* sram pivot trie */
    int         domain; /* tcam domain id */
	uint32      tcam_shift_len;
    uint32      padding_len;
    uint16      sbucket_format;
    uint16      prefix_number;
    SHR_BITDCL  pivot_bmap[BITS2WORDS(_MAX_SBUCKET_PIVOT_)];
    taps_spivot_handle_t wsph; /* pointer to wild pivot */
};

/*
 *
 * Function:
 *     taps_sbucket_alloc
 * Purpose:
 *     allocate sram bucket
 */
extern int taps_sbucket_create(int unit, 
                               const taps_handle_t taps, 
                               const taps_wgroup_handle_t *wgroup,
                               unsigned int domain,  /* segment id of domain */
                               taps_dbucket_handle_t wdbh, /* pointer to wild pivot dbucket */
                               uint8 is_create_default_entry, /* if set inserts wild pivot into trie 
                                                             and create a invalid sbucket pivot 
                                                             for wild tcam entry */
                               uint32 key_shift_len,
                               taps_sbucket_handle_t *handle);
/*
 *
 * Function:
 *     taps_sbucket_free
 * Purpose:
 *     destroy sram bucket 
 */

extern int taps_sbucket_destroy(int unit,
                                const taps_handle_t taps, 
                                const taps_wgroup_handle_t *wgroup,
                                taps_sbucket_handle_t handle);

/*
 *
 * Function:
 *     taps_sbucket_prefix_find
 * Purpose:
 *     find sram pivot for given prefix
 */
extern int taps_sbucket_prefix_find(int unit,
                                    taps_sbucket_handle_t handle,
                                    uint32 *prefix, 
                                    unsigned int len, 
                                    taps_spivot_handle_t *sph /*out*/);

/*
 *
 * Function:
 *     soc_sbx_caladan3_taps_sbucket_driver_init
 * Purpose:
 *     Bring up TAPS sram bucket drivers
 */
extern int taps_sbucket_insert_pivot(int unit, 
                                     const taps_handle_t taps,
                                     const taps_wgroup_handle_t *work_group,
                                     unsigned int pivot_index,
                                     taps_sbucket_handle_t sbh,
                                     uint32 *pivot, int pivot_len,
                                     uint8 is_prefix,
                                     uint32 *payload,
				     void *cookie,
                                     taps_dbucket_handle_t dbh,
                                     uint32 *bpm,
                                     /* OUT */
                                     taps_spivot_handle_t *nsph);

/*
 *
 * Function:
 *     taps_sbucket_insert_default_entry
 * Purpose:
 *     Insert default entry in sram for slave unit
 */
extern int taps_sbucket_insert_default_entry(int unit, taps_handle_t taps, 
                                    taps_wgroup_handle_t work_group);


/*
 *
 * Function:
 *     taps_sbucket_delete_pivot
 * Purpose:
 *     Delete a pivot into the sram bucket.
 */
extern int taps_sbucket_delete_pivot(int unit, 
                                     const taps_handle_t taps,
                                     const taps_wgroup_handle_t *work_group,
                                     taps_sbucket_handle_t sbh,
                                     taps_spivot_handle_t sph);
/*
 *
 * Function:
 *     taps_sbucket_pivot_len_get
 * Purpose:
 *     Get a pivot len of a new sbucket
 */
extern int taps_sbucket_pivot_len_get(int unit, 
                       const taps_handle_t taps,
                       taps_sbucket_handle_t sbh,
                       /* out */
                       uint32 *pivot_len);

/*
 *
 * Function:
 *     taps_sbucket_merge
 * Purpose:
 *     Merge src_sbh to dst_sbh
 */
extern int taps_sbucket_merge(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t dst_sbh,
                       taps_sbucket_handle_t src_sbh,
                       taps_spivot_handle_t *roll_back_sph);
/*
 *
 * Function:
 *     taps_sbucket_redundant_pivot_clear
 * Purpose:
 *     Remove redundant sphs
 */
extern int taps_sbucket_redundant_pivot_clear(int unit, 
                       const taps_handle_t taps,
                       taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       taps_spivot_handle_t *sphs,
                       int sph_num);
/*
 *
 * Function:
 *     taps_sbucket_split_stage1
 * Purpose:
 *     Split a given sram bucket, and get the root node of the sub trie
 */
extern int taps_sbucket_split_stage1(int unit, 
                       const taps_handle_t taps,
                       taps_sbucket_handle_t sbh,
                       /* out */
                       uint32 *pivot,
                       uint32 *pivot_len,
                       uint32 *bpm,
                       trie_node_t **split_trie_root);


/*
 *
 * Function:
 *     taps_sbucket_split_onchip_stage2
 * Purpose:
 *     Move entry from old_sbh to new_sbh
 */
extern int taps_sbucket_split_onchip_stage2(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       unsigned int domain, /* for split bucket */
                       trie_node_t *split_trie_root,
                       uint32 pivot_len,
                       taps_sbucket_handle_t *nsbh,
                       taps_spivot_handle_t *roll_back_sph);

/*
 *
 * Function:
 *     taps_sbucket_split_offchip_stage2
 * Purpose:
 *     Move entry from old_sbh to new_sbh
 */
extern int taps_sbucket_split_offchip_stage2(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       unsigned int domain, /* for split bucket */
                       trie_node_t *split_trie_root,
                       taps_dbucket_handle_t wdbh,
                       taps_sbucket_handle_t *nsbh);

/*
 *
 * Function:
 *     taps_sbucket_domain_move
 * Purpose:
 *     Move whole domain to new_sbh
 */
extern int taps_sbucket_domain_move(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       taps_sbucket_handle_t sbh,
                       unsigned int domain);

/*
 *
 * Function:
 *     taps_sbucket_propagate_prefix
 * Purpose:
 *     Propagate BPM for inserted prefix 
 * when local_pointer == _BRR_INVALID_CPE_, search for local pointer
 *     this requires a consistent dbase in tcam/sbucket/dbucket. if
 *     that's not available, caller needs to calculate local_pointer
 *     and pass it in.
 */
extern int taps_sbucket_propagate_prefix(int unit, 
                                         const taps_handle_t taps,
                                         const taps_sbucket_handle_t sbh,
                                         const taps_wgroup_handle_t *wgroup,
                                         uint32  *prefix,
                                         uint32   prefix_len,
                     uint32   local_pointer,
                                         /* >0 - add prefix, 0 - delete prefix */
                                         unsigned int add,
                                         uint8 *isbpm /* OUT */);
/*
 *
 * Function:
 *     taps_sbucket_commit
 * Purpose:
 *     commit sram bucket on a given work group
 */
extern int taps_sbucket_commit(int unit, taps_wgroup_handle_t wgroup);

/*
 *
 * Function:
 *     taps_sbucket_free_work_queue
 * Purpose:
 *     free all outstanding work items
 */
extern int taps_sbucket_free_work_queue(int unit, 
                                        taps_wgroup_handle_t wgroup);

/*
 *
 * Function:
 *     taps_sbucket_prefix_find
 * Purpose:
 *     find sram pivot for given a prefix
 */
extern int taps_sbucket_prefix_pivot_find(int unit,
                                          taps_sbucket_handle_t handle,
                                          uint32 *prefix, 
                                          unsigned int len, 
                                          taps_search_mode_e_t mode,
                                          taps_spivot_handle_t *sph);

/*
 *
 * Function:
 *     taps_sbucket_pivot_id_alloc
 * Purpose:
 *     allocate sram pivot id
 */
extern int taps_sbucket_pivot_id_alloc(int unit, 
                                       taps_handle_t taps, 
                                       taps_sbucket_handle_t sbh,
                                       unsigned int *id);

/*
 *
 * Function:
 *     taps_sbucket_pivot_id_free
 * Purpose:
 *     free sram pivot id
 */
extern int taps_sbucket_pivot_id_free(int unit, 
                                      taps_handle_t taps, 
                                      taps_sbucket_handle_t sbh,
                                      unsigned int id);
/*
 *
 * Function:
 *     taps_sbucket_stat_get
 * Purpose:
 *     get status of sram bucket
 */
extern int taps_sbucket_stat_get(int unit,
                                taps_sbucket_handle_t dbh,
                                 taps_bucket_stat_e_t *stat);

typedef int (*sbucket_traverse_cb_f)(taps_spivot_handle_t sph, void *user_data);

/*
 *
 * Function:
 *     taps_sbucket_traverse
 * Purpose:
 *     Traverse the sram bucket & invoke call back
 */
extern int taps_sbucket_traverse(int unit, 
                                 const taps_handle_t taps,
                                 const taps_wgroup_handle_t *wgroup,
                                 const taps_sbucket_handle_t sbh, 
                                 void *user_data,
                                 sbucket_traverse_cb_f cb);

/*
 *
 * Function:
 *     taps_sbucket_destroy_traverse
 * Purpose:
 *     Traverse the sram bucket & invoke call back
 */
extern int taps_sbucket_destroy_traverse(int unit, 
                                         const taps_handle_t taps,
                                         const taps_wgroup_handle_t *wgroup,
                                         const taps_sbucket_handle_t sbh, 
                                         void *user_data,
                                         sbucket_traverse_cb_f cb);


/*
 *
 * Function:
 *     taps_sbucket_pm_traverse
 * Purpose:
 *     Search the sram bucket & invoke call back on all 
 *     prefixes shorter or equal length and matching the specified prefix
 */
extern int taps_sbucket_pm_traverse(int unit, 
                    const taps_handle_t taps,
                    const taps_wgroup_handle_t *wgroup,
                    const taps_sbucket_handle_t sbh, 
                    uint32 *pivot,
                    uint32 pivot_len,
                    void *user_data,
                    sbucket_traverse_cb_f cb);

/*
 * Dump a SBucket software and hardware state
 */
extern int taps_sbucket_dump(int unit, const taps_handle_t taps,
                 const taps_sbucket_handle_t sbh,
                             uint32 flags);

/*
 * Dump a SBucket SPH software and hardware state
 */
extern int taps_sbucket_pivot_dump(int unit, const taps_handle_t taps,
                   const taps_spivot_handle_t sph,
                                   uint32 flags);

/*
 * Dump a SBucket entry
 */
extern int taps_sbucket_entry_dump(int unit, int taps_instance, int segment,
                   int bucket, int entry, int format,
                                   uint32 flags);

extern int _taps_sbucket_find_bpm(int unit, const taps_handle_t taps, 
                                  taps_sbucket_handle_t sbh,
                                  uint32 *key, uint32 length,
                                  uint32 *bpm_length, uint32 *bpm_local_index);

extern int taps_sbucket_find_bpm(int unit, 
                                 const taps_handle_t taps, 
                                 taps_sbucket_handle_t sbh,
                                 uint32 *key, uint32 length,
                                 uint32 *bpm_length);
extern int taps_sbucket_enqueue_update_assodata_work(int unit, 
                                 const taps_handle_t taps,
                                 const taps_sbucket_handle_t sbh,
                                 const taps_spivot_handle_t sph,
                                 const taps_wgroup_handle_t *wgroup,
                                 unsigned int asso_data);

extern int taps_update_offchip_payload_work(int unit, 
                        const taps_handle_t taps,
                        const taps_wgroup_handle_t *wgroup,
                        taps_spivot_handle_t sph);

extern int taps_sbucket_first_pivot_handle_get(int unit,
                                        taps_handle_t taps,         
                                        taps_sbucket_handle_t sbh, 
                                        taps_spivot_handle_t *sph);

extern int taps_sbucket_next_pivot_handle_get(taps_handle_t taps, 
                                        int start_index, 
                                        taps_sbucket_handle_t sbh,
                                        taps_spivot_handle_t *sph);

extern int taps_sbucket_first_pivot_get(int unit,
                                    taps_handle_t taps,         
                                    taps_sbucket_handle_t sbh, 
                                    uint32 *key,
                                    uint32 *key_length);

extern int taps_sbucket_next_pivot_get(taps_handle_t taps, 
                                    int start_index, 
                                    taps_sbucket_handle_t sbh,
                                    uint32 *key,
                                    uint32 *key_length); 

extern int taps_sbucket_first_bucket_handle_get(int unit, 
                                        taps_handle_t taps, 
                                        taps_sbucket_handle_t *sbh);

extern int taps_sbucket_instance_and_bucket_id_get(int unit, const taps_handle_t taps, 
                                int global_id, int *hwinstance, int *local_id);

#endif /* _SBX_CALADN3_TAPS_SBUCKET_H_ */
