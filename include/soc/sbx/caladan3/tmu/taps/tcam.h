/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: tcam.h,v 1.19.54.2 Broadcom SDK $
 *
 * TAPS internal tcam defines/interfaces
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TAPS_TCAM_H_
#define _SBX_CALADN3_TAPS_TCAM_H_

#include <soc/sbx/caladan3/tmu/taps/taps.h>

#define TAPS_TCAM_NUM_ENTRIES (1024)

/* Uses for unified mode, return ture is entrya and entryb is in the same instance */
#define _ENTRY_IN_SAME_INSTANCE(entrya, entryb, split_point) \
        ((((entrya) >= (split_point)) && ((entryb) >= (split_point))) \
        || (((entrya) < (split_point)) && ((entryb) < (split_point))))
        
/* Uses for unified mode, return ture if entry is in instance0 */		
#define _ENTRY_IN_TAPS0(entry, split_point) ((entry) < (split_point))

typedef struct taps_tcam_subsegment_s taps_tcam_subsegment_t;

/*
 * Following object manages a TCAM entry
 */
typedef struct taps_tcam_pivot_s {
    trie_node_t node;              /* trie node, DON'T MOVE, HAVE TO BE FIRST MEMBER */
    taps_sbucket_handle_t sbucket; /* sram bucket handle */
    uint16  entry;                 /* tcam entry ID */
    uint8   length;                /* key length */
    uint32  key[BITS2WORDS(TAPS_IPV4_KEY_SIZE)]; /* variable length key */
} taps_tcam_pivot_t, *taps_tcam_pivot_handle_t;

/*
 * Following object manages a TCAM segment (a collection of TCAM entries)
 */
struct taps_tcam_s {
    uint32  flags;          /* flags */
    uint8   tapsi;          /* taps instance mask */
    uint8   key_size;       /* key size */
    uint8   segment;        /* segment id */
    uint32  base;           /* base tcam entry*/
    uint32  size;           /* number of tcam entries */
    uint32  use_count;      /* number of tcam entries being used */
    uint16  pivot_size;     /* size of a pivot structure (depends on the key_size) */
    trie_t *trie;           /* trie */
    taps_handle_t taps;     /* taps handle */

    /* pivot dbase, use hpcm libaray for alloc/free */
    soc_heap_mem_chunk_t *pivots_hpcm;

    /* following data structure is allocated based on segment size
     * and is used by TCAM arbitration algorithm
     */
    /* tcam entry in use bitmask, 1 bit for each tcam entry. set if tcam entry is in use */
    uint32 *in_use;       
    /* tcam offset to tcam_pivot handle reverse map */
    taps_tcam_pivot_handle_t *map; 
    /* tcam subsegment info */
    taps_tcam_subsegment_t *ssegs;
};

typedef struct tcam_bpm_cb_info_s {
    uint8         isbpm;
    uint8         unified_mode; /* Is unified mode or parallel mode*/
    uint8         propagate_taps0; /* need to do propagation in taps0 or not */
    uint8         propagate_taps1; /* need to do propagation in taps1 or not */
    int           split_line;   /* Split line of the two instances */
} tcam_bpm_cb_info_t;

typedef struct asso_data_update_cb_data_s {
    int                    unit;
    taps_wgroup_handle_t  *wgroup;
    taps_handle_t          taps;
    int                    add;
    unsigned int          *payload;
} asso_data_update_cb_data_t;

/*
 *
 * Function:
 *     taps_tcam_driver_init
 * Purpose:
 *     Bring up TAPS internal tcam drivers
 */
extern int taps_tcam_driver_init(int unit);

/*
 *
 * Function:
 *     taps_tcam_create
 * Purpose:
 *     Bring up TAPS internal tcam segment
 */
extern int taps_tcam_create(int unit, 
                            const taps_handle_t taps,
			    taps_tcam_handle_t *p_handle);

/*
 * Function:
 *
 * Purpose:
 *
 */
extern void taps_tcam_destroy(int unit, taps_tcam_handle_t handle);

/*
 * Function:
 *
 * Purpose:
 *  Insert a pivot into Tcam
 */
extern int taps_tcam_insert_pivot(int unit, taps_tcam_handle_t handle,
				  taps_wgroup_handle_t *work_group,
				  uint32 *key, uint32 length,
				  uint32 *bpm_mask,
				  taps_sbucket_handle_t sbh,
				  taps_tcam_pivot_handle_t *p_ntph,
				  int allocated_entry,
			      uint8 with_id);

/*
 * Function:
 *
 * Purpose:
 *  Delete a pivot from Tcam
 */
extern int taps_tcam_delete_pivot(int unit, taps_tcam_handle_t handle,
				  taps_wgroup_handle_t *work_group,
				  taps_tcam_pivot_handle_t tph);

/*
 *   Function
 *      taps_tcam_insert_default_entry
 *   Purpose
 *      Insert default entry(0/0) in tcam. 
 *      No software update. Insert this entry as the last entry in tcam.
 */
extern int taps_tcam_insert_default_entry(int unit, taps_handle_t taps, 
                                taps_wgroup_handle_t work_group);

/*
 * Function:
 *
 * Purpose:
 *  Lookup a prefix, return matched tcam pivot and bpm_length in Tcam
 */
extern int taps_tcam_lookup_prefix(int unit, taps_tcam_handle_t handle,
				   uint32 *key, uint32 length,
				   taps_tcam_pivot_handle_t *p_tph);

/*
 *   Function
 *      
 *   Purpose
 *      Propagation for mode zero. Since we have a wild sph in each sbucket for mode zero, 
 *      just update the asso data rather than update the global index in tcam.
 */
extern int taps_tcam_propagate_prefix_for_modezero(int unit, taps_tcam_handle_t handle,
			       taps_wgroup_handle_t *wgroup,
                                    taps_tcam_pivot_handle_t tph, int add,
                                    uint32 *key, uint32 length, unsigned int *payload);
/*
 * Function:
 *
 * Purpose:
 *  propagate an added/deleted prefix in Tcam
 * Note:
 *  if tph is not NULL, assuming the prefix was added/deleted in the
 *  sbucket associated with the tph, otherwise we start searching
 *  from root trie node (??)
 */
extern int taps_tcam_propagate_prefix(int unit, taps_tcam_handle_t handle,
				      taps_wgroup_handle_t *work_group,
				      taps_tcam_pivot_handle_t tph, int add,
				      int32 global_index_added,
				      uint32 *key, uint32 length,
                                      uint8 *isbpm/*out*/);

/*
 * Function:
 *
 * Purpose:
 *  commit tcam workgroup
 * Note:
 */
extern int taps_tcam_commit(int unit, taps_wgroup_handle_t wgroup);

/*
 *
 * Function:
 *     taps_tcam_free_work_queue
 * Purpose:
 *     free all outstanding work items
 */
extern int taps_tcam_free_work_queue(int unit, 
                                     taps_wgroup_handle_t wgroup);

typedef int (*tcam_traverse_cb_f)(taps_tcam_pivot_handle_t tph, void *user_data);

/*
 *
 * Function:
 *     taps_tcam_traverse
 * Purpose:
 *     Traverse the tcam database & invoke call back
 */
extern int taps_tcam_traverse(int unit, 
                              const taps_handle_t taps,
                              const taps_tcam_handle_t handle,
                              void *user_data,
                              tcam_traverse_cb_f cb);

/*
 *
 * Function:
 *     taps_tcam_destroy_traverse
 * Purpose:
 *     Traverse the tcam database & invoke call back
 */
extern int taps_tcam_destroy_traverse(int unit, 
                                      const taps_handle_t taps,
                                      const taps_tcam_handle_t handle,
                                      void *user_data,
                                      tcam_traverse_cb_f cb);

/*
 * Function:
 *
 * Purpose:
 *  Dump Tcam info
 */
extern int taps_tcam_dump(int unit, taps_tcam_t *handle, uint32 flags);

/*
 * Dump a TCAM TPH software and hardware state 
 */
extern int taps_tcam_pivot_dump(int unit, taps_tcam_handle_t handle,
				taps_tcam_pivot_handle_t tph,
                                uint32 flags);

/*
 * Dump a TCAM hardware entry
 */
extern int taps_tcam_entry_dump(int unit, int taps_instance, int segment,
				int entry, uint32 flags);

/*
 * Dump a TCAM subsegment
 */
extern int taps_tcam_subsegment_dump(int unit, const taps_tcam_handle_t handle,
				     int segment, int subsegment);

/*
 * TCAM arbitration unit test
 */
extern int taps_tcam_ut(int unit, const taps_handle_t taps, int test);

/*
 * Find bpm in TCAM
 */
extern int _taps_tcam_find_bpm(int unit, taps_tcam_t *handle, uint32 *key,
			       uint32 length, uint32 *bpm_length, uint32 *bpm_global_index); 
			       
extern int taps_tcam_find_bpm(int unit, taps_tcam_t *handle, uint32 *key,
                              uint32 length, uint32 *bpm_length);

/*
 *   Function
 *      taps_tcam_entry_alloc
 *   Purpose
 *      Allocate a free tcam entry for a particular key length
 */
extern int taps_tcam_entry_alloc(int unit, taps_tcam_handle_t handle,
				  taps_wgroup_handle_t *wgroup,
				  int key_length, int *allocated_entry);
/*
 *   Function
 *      taps_tcam_entry_free
 *   Purpose
 *      free a tcam entry
 */
extern int taps_tcam_entry_free(int unit, taps_tcam_handle_t handle,
				 taps_wgroup_handle_t *wgroup,
				 int entry);

/*
 *   Function
 *      taps_tcam_entry_bitmap_clear
 *   Purpose
 *      clear the tcam entry from bit map
 */
extern int taps_tcam_entry_bitmap_clear(int unit, taps_tcam_handle_t handle,
				 unsigned int length, int entry);


#endif /* _SBX_CALADN3_TAPS_TCAM_H_ */
