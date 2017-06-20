/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: dbucket.h,v 1.27.14.2 Broadcom SDK $
 *
 * TAPS dram bucket defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TAPS_DBUCKET_H_
#define _SBX_CALADN3_TAPS_DBUCKET_H_

#include <soc/sbx/caladan3/tmu/taps/taps.h>

/* dbucket cache is a shadow copy of what's in dram for the dbucket
 * prefix table, it's format is a number of 256bits block followed
 * by an optional 128bits block. Each block could house number of 
 * prefixes and block format depends on the prefix type(ipv4/ipv6)
 * the prefixes are in format of 32 bits CPE(for ipv4) or 48 bits
 * signature(for ipv6). 
 */
/* convert dbucket index into word offset in cache */
#define _TAPS_DBUCKET_INDEX_TO_CACHE_INDEX(index, cache_block_size)	\
    ((index)/(cache_block_size)*BITS2WORDS(256))

/* convert dbucket index into field index of cache */
#define _TAPS_DBUCKET_INDEX_TO_CACHE_FIELD(index, cache_block_size)	\
    ((index)%(cache_block_size))

/* convert dbucket size in number of prefix to cache size in number of 32 bits words */
#define _TAPS_DBUCKET_SIZE_TO_CACHE_SIZE(size, cache_block_size)        \
  ((((size)/(cache_block_size))*BITS2WORDS(256)) +			\
   (((size)%(cache_block_size))?((((size)%(cache_block_size))>((cache_block_size)/2))?BITS2WORDS(256):BITS2WORDS(128)):0))

#define _TAPS_DBUCKET_IPV4_PREFIX_LENGTH (31)
#define _TAPS_DBUCKET_IPV6_PREFIX_LENGTH (127)

/* convert 31bits prefix to single-ended 0-run CPE format,
 * see TAPS spec for formula. length is required to be <= 32
 * length 32 will give all 0 (invalid pattern for unused field)
 * assuming key has the "length: number of bits in LSB bits
 */
#define _TAPS_KEY_2_CPE_31B(key, length) \
    (TP_SHL(((key)&(TP_MASK(length))), (32-(length))) | (((length)>31)?0:(1<<(31-(length)))))

/* Dram bucket handle object & interfaces 
 * DBH is a container of dram pivots. 
 * NOTE: 
 *    this structure is assuming max dbucket size is 32 prefixes
 *    for 0* and 1* sub-bucket, so total is 64 prefixes max.
 */
struct taps_dbucket_s {
    int          domain;    /* tcam domain id */
    int          bucket;    /* dram bucket id */
    trie_t      *trie0;     /* dram pivot trie for 0* */
    SHR_BITDCL   pfx_bmap0[2]; /* dram prefix allocation bitmap for 0* */
    trie_t      *trie1;     /* dram pivot trie for 1* */
    SHR_BITDCL   pfx_bmap1[2]; /* dram prefix allocation bitmap for 1* */
    unsigned int cache[1];  /* dram dbucket cache, variable length */
};

/* Dram prefix handle object & interfaces */
struct taps_dprefix_s {
    trie_node_t   node;
    uint8         index;
    uint8         bpm;     /* TRUE if this prefix is a best prefix for some pivots */
    void *hpcm_elem_handle;
    unsigned int  payload[_TAPS_PAYLOAD_WORDS_];
    void          *cookie;
    unsigned int  length;  /* prefix length */
    unsigned int  pfx[1];  /* prefix, variable length */
};

typedef enum taps_dbucket_operation_type_e_s {
    TAPS_DBUCKET_OP_INSERT=0,
    TAPS_DBUCKET_OP_DELETE,
    TAPS_DBUCKET_OP_UPDATE,
    TAPS_DBUCKET_OP_INSERT_DEFAULT,
    TAPS_DBUCKET_OP_FLUSH_PFX,
    TAPS_DBUCKET_OP_INSERT_NO_ENQUEUE,
    TAPS_DBUCKET_OP_DELETE_NO_ENQUEUE,
    TAPS_DBUCKET_OP_INSERT_DEFAULT_NO_ENQUEUE,
    TAPS_DBUCKET_OP_MAX
} taps_dbucket_operation_type_e_t;

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
extern int taps_dbucket_enqueue_update_payload_work(int unit, 
						    const taps_handle_t taps,
						    const taps_wgroup_handle_t *wgroup,
						    taps_dbucket_handle_t dbh,
						    taps_dprefix_handle_t dph);
/*
 *
 * Function:
 *     taps_dbucket_alloc
 * Purpose:
 *     allocate dram bucket
 */
extern int taps_dbucket_create(int unit, 
                               const taps_handle_t taps, 
                               const taps_wgroup_handle_t wgroup,
                               unsigned int domain, /* sbucket id or tcam domain id */
                               unsigned int bucket, /* spivot id or dbucket id */
                               taps_dbucket_handle_t *dbh);

/*
 *
 * Function:
 *     taps_dbucket_free
 * Purpose:
 *     destroy dram bucket 
 */

extern int taps_dbucket_destroy(int unit, 
                                const taps_handle_t taps, 
                                const taps_wgroup_handle_t wgroup,
                                taps_dbucket_handle_t dbh);

/*
 *
 * Function:
 *     taps_dbucket_insert_prefix
 * Purpose:
 *     Insert a prefix into the dram bucket.
 *     The function returns a new dram prefix handle if insert succeeds
 */
extern int taps_dbucket_insert_prefix(int unit, 
                                      const taps_handle_t taps,
                                      const taps_wgroup_handle_t *wgroup,
                                      taps_dbucket_handle_t dbh,
                                      uint32 *prefix, 
                                      int length,       /* prefix length */
                                      uint32 *payload,
				      void *cookie,
				      uint32 pivot_len, /* dbucket pivot length */
                                      /* OUT */
                                      taps_dprefix_handle_t *ndph);

/*
 *
 * Function:
 *     taps_dbucket_delete_prefix
 * Purpose:
 *     Delete a prefix into the dram bucket.
 */
extern int taps_dbucket_delete_prefix(int unit, 
                                      const taps_handle_t taps,
                                      const taps_wgroup_handle_t *wgroup,
                                      taps_dbucket_handle_t dbh,
                                      taps_dprefix_handle_t dph);

/*
  *Function
  *   taps_dbucket_insert_default_entry
  *Purpose
  *   Insert default entry in dram for slave unit
  */
extern int taps_dbucket_insert_default_entry(int unit, taps_handle_t taps, 
                                    taps_wgroup_handle_t wgroup);

/*
 *
 * Function:
 *     taps_dbucket_find_prefix
 * Purpose:
 *     find a prefix into the dram bucket.
 */
extern int taps_dbucket_find_prefix(int unit, 
                                    taps_dbucket_handle_t dbh,
                                    uint32 *key, unsigned int len,
                                    taps_dprefix_handle_t *dph);

/*
 *   Function
 *      taps_dbucket_merge
 *   Purpose
 *      Merge two dbucket
 */
extern int taps_dbucket_merge(int unit, 
                       const taps_handle_t taps,
                       const taps_wgroup_handle_t *wgroup,
                       int pivot_len,
                       taps_dbucket_handle_t dst_dbh,
                       taps_dbucket_handle_t src_dbh,
                       taps_dprefix_handle_t *roll_back_dph);
/*
 *
 * Function:
 *     taps_split_sbucket
 * Purpose:
 *     Split a given sram bucket 
 */
extern int taps_dbucket_split(int unit, 
                              const taps_handle_t taps,
                              const taps_wgroup_handle_t *wgroup,
                              const unsigned int bucketid, /* new bucket id for split bucket */
                              taps_dbucket_handle_t dbh,
                              /* out */
                              uint32 *pivot,
                              uint32 *pivot_len,
                              uint32 *bpm,
                              taps_dbucket_handle_t *ndbh);

/*
 *
 * Function:
 *     taps_relocate_dbucket
 * Purpose:
 *     Relocate a dram bucket to a different physical location (reloc bucket id )
 */
extern int taps_relocate_dbucket(int unit, 
                                 const taps_handle_t taps,
                                 const taps_wgroup_handle_t *wgroup,
                                 const unsigned int reloc_domain_id,
                                 taps_dbucket_handle_t dbh);

/*
 *
 * Function:
 *     taps_dbucket_commit
 * Purpose:
 *     commit dram bucket on a given work group
 */
extern int taps_dbucket_commit(int unit, 
                               taps_wgroup_handle_t wgroup);

/*
 *
 * Function:
 *     taps_dbucket_free_work_queue
 * Purpose:
 *     free all outstanding work items
 */
extern int taps_dbucket_free_work_queue(int unit, 
                                        taps_wgroup_handle_t wgroup);

/*
 *
 * Function:
 *     taps_dbucket_stat_get
 * Purpose:
 *     free space on dram bucket
 */
extern int taps_dbucket_stat_get(int unit,
                                 const taps_handle_t taps,
                                 taps_dbucket_handle_t dbh,
                                 taps_bucket_stat_e_t *stat);

typedef int (*dbucket_traverse_cb_f)(taps_dprefix_handle_t dph, void *user_data);

/*
 *
 * Function:
 *     taps_dbucket_traverse
 * Purpose:
 *     Traverse the dram bucket & invoke call back
 */
extern int taps_dbucket_traverse(int unit, 
                                 const taps_handle_t taps,
                                 const taps_wgroup_handle_t *wgroup,
                                 taps_dbucket_handle_t dbh, 
                                 void *user_data,
                                 dbucket_traverse_cb_f cb);

extern int taps_dbucket_destroy_traverse(int unit, 
                                         const taps_handle_t taps,
                                         const taps_wgroup_handle_t *wgroup,
                                         taps_dbucket_handle_t dbh, 
                                         void *user_data,
                                         dbucket_traverse_cb_f cb);

extern int taps_dbucket_next_prefix_handle_get(taps_handle_t taps,
                                        int start_index,
                                        taps_dbucket_handle_t dbh,
                                        taps_dprefix_handle_t *dph);

extern int taps_dbucket_next_prefix_get(taps_handle_t taps,
                                        int start_index,
                                        taps_dbucket_handle_t dbh,
                                        uint32 *key,
                                        uint32 *key_length);

extern int taps_dbucket_first_prefix_handle_get(int unit, taps_handle_t taps, 
                                    taps_dbucket_handle_t dbh, taps_dprefix_handle_t *dph);

extern int taps_dbucket_first_prefix_get(int unit, taps_handle_t taps, 
                                    taps_dbucket_handle_t dbh, 
                                    uint32 *key,
                                    uint32 *key_length);

extern int taps_dbucket_find_bpm(int unit, const taps_handle_t taps, 
                                  taps_dbucket_handle_t dbh,
                                  uint32 *key, uint32 length, 
                                  uint32 pivot_len,
                                  uint32 *bpm_length);


/*
 * Dump a Dbucket software and hardware state
 */
extern int taps_dbucket_dump(int unit, const taps_handle_t taps,
			     const taps_dbucket_handle_t dbh,
                             uint32 flags);

/*
 * Dump a Dbucket DPH software and hardware state
 */
extern int taps_dbucket_prefix_dump(int unit, const taps_handle_t taps,
				    const taps_dbucket_handle_t dbh,
				    const taps_dprefix_handle_t dph,
                                    uint32 flags);

/*
 * Dump a Dbucket entry
 */
extern int taps_dbucket_entry_dump(int unit, const taps_handle_t taps,
				   int sbucket, int dbucket, int entry,
                                   uint32 flags, int is_global_index);

/*
 * Dump a entry in Dbucket payload table 
 */
extern int taps_dbucket_payload_entry_dump(int unit, const taps_handle_t taps,
			    int sbucket, int entry, uint32 flags);

/*
* Clear one entry in prefix table 
*/
extern int taps_dbucket_prefix_table_entry_clear(int unit, taps_handle_t taps,
    taps_wgroup_handle_t wgroup, int domain_id, int dbucket_id);

extern int taps_dbucket_collision_rehash(int unit, taps_handle_t taps, taps_wgroup_handle_t *wgroup, 
                                    uint32 *col_key, uint32 length,
                                    taps_dbucket_handle_t dbh, taps_dprefix_handle_t dph,
                                    uint8 *collision);
#endif /* _SBX_CALADN3_TAPS_DBUCKET_H_ */
