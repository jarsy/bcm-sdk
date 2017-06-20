/* $Id: jer2_qax_multicast_imp.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/cosq.h>
#include <soc/dnx/legacy/QAX/qax_multicast_imp.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_mem.h>
#include <shared/swstate/access/sw_state_access.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* The value used for free=unused MCDB entries, also used for empty egress linked lists */
#define JER2_QAX_MC_FREE_ENTRY_0 0
#define JER2_QAX_MC_FREE_ENTRY_1 0
#define JER2_QAX_MC_FREE_ENTRY_2 0

/* value used for empty ingress multicast linked lists */
#define JER2_JER_MC_ING_EMPTY_ENTRY_0 0x00000000
#define JER2_JER_MC_ING_EMPTY_ENTRY_1 0xfffffff0
#define JER2_JER_MC_ING_EMPTY_ENTRY_2 0x0000a47f

/* constants used for the interpretation of ingress multicast destinations */
#define JER2_QAX_MC_ING_DESTINATION_ID_BITS 15
#define JER2_QAX_MC_ING_DESTINATION_Q_TYPE 0
#define JER2_QAX_MC_ING_DESTINATION_SYSPORT_TYPE 2
#define JER2_QAX_MC_ING_DESTINATION_SYSPORT_LAG_TYPE 3
#define JER2_QAX_MC_ING_DESTINATION_SYSPORT_MASK ((1 << JER2_QAX_MC_ING_DESTINATION_ID_BITS) - 1)
#define JER2_QAX_MC_ING_DESTINATION_ID_MASK_TM_FLOW 0x1ffff

#define JER2_QAX_MC_MCIDS_PER_IS_ING_MC_ENTRY 32

#define FDT_IPT_MESH_MC_ENTRY_SIZE 5
#define EGR_PER_CORE_REP_MAX_ENTRY_SIZE FDT_IPT_MESH_MC_ENTRY_SIZE
#define FDT_IPT_MESH_MC_BITS_PER_GROUP 4
#define FDT_IPT_MESH_MC_GROUPS_PER_ENTRY 32
#define FDT_IPT_MESH_MC_OFFSET_IN_GROUP_BITS 2

/* } */


/*
 * Get, set, increase and decrease the stored number of free entries
 */
uint32 jer2_qax_mcds_unoccupied_get(
    DNX_SAND_IN jer2_qax_mcds_t *mcds
)
{
  return mcds->nof_unoccupied;
}

static void
  jer2_qax_mcds_unoccupied_increase(
    DNX_SAND_INOUT jer2_qax_mcds_t *mcds,
    DNX_SAND_IN    uint32               delta
)
{
  mcds->nof_unoccupied += delta;
  DNX_MC_ASSERT(mcds->nof_unoccupied <= JER2_QAX_LAST_MCDB_ENTRY);
}

static void
  jer2_qax_mcds_unoccupied_decrease(
    DNX_SAND_INOUT jer2_qax_mcds_t *mcds,
    DNX_SAND_IN    uint32               delta
)
{
  DNX_MC_ASSERT(mcds->nof_unoccupied >= delta);
  mcds->nof_unoccupied -= delta;
}

static uint32 
jer2_qax_self_replication_set (DNX_SAND_IN  int unit,       /* input: device */
                          DNX_SAND_IN  uint32 group_mcid,
                          DNX_SAND_IN  int on_off){

    int index, offset; /* index in the table being changed and bit offset inside the table entry */
    uint32 *word_to_change, orig_word; /* word to change in the table entry, and its original value */
    uint32 groups_per_entry, bits_per_group, offset_in_group_bits;
    soc_mem_t table;
    uint32 data[EGR_PER_CORE_REP_MAX_ENTRY_SIZE];

    DNXC_INIT_FUNC_DEFS;
    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOC_EXIT;
    }

    if ((SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_EGR_CORE_MESH_MODE) == 0) {
        SOC_EXIT;
    }

    groups_per_entry = FDT_IPT_MESH_MC_GROUPS_PER_ENTRY;
    bits_per_group = FDT_IPT_MESH_MC_BITS_PER_GROUP;
    offset_in_group_bits = FDT_IPT_MESH_MC_OFFSET_IN_GROUP_BITS;
    table = FDT_IPT_MESH_MCm;

    index = group_mcid / groups_per_entry;
    offset = bits_per_group * (group_mcid % groups_per_entry) + offset_in_group_bits;
    word_to_change = data + offset / DNX_SAND_NOF_BITS_IN_UINT32;

    DNXC_IF_ERR_EXIT(soc_mem_read(unit, table, MEM_BLOCK_ANY, index, data));
    orig_word = *word_to_change;

    if (on_off) {
        *word_to_change |= 1 << offset;
    }
    else{
        *word_to_change &= ~(1 << offset);
    }

    if (*word_to_change != orig_word) { /* If the value is changed, write the new value */
        DNXC_IF_ERR_EXIT(soc_mem_write(unit, table, MEM_BLOCK_ALL, index, data));
    }
	
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Free blocks handling functions specific to JER2_QAX.
 */

/*
 * Return the region corresponding to the given mcdb index,
 * and get the max consecutive entries sub range from the range that includes mcdb_index.
 */
dnx_free_entries_blocks_region_t*
  jer2_qax_mcds_get_region_and_consec_range(jer2_qax_mcds_t *mcds, uint32 mcdb_index, uint32 *range_start, uint32 *range_end)
{
    dnx_free_entries_blocks_region_t *range = jer2_qax_mcds_get_region(mcds, mcdb_index);
    *range_start = range->range_start;
    *range_end = range->range_end;
    /* This code depends on all ranges being consecutive  */
    DNX_MC_ASSERT(*range_start <= *range_end && mcdb_index >= *range_start && mcdb_index <= *range_end);
    return range;
}

/*
 * Copy the src_index entry to the dst_index entry, and write the dst_index entry to hardware.
 * Both the hardware and mcds is copied. So be be very careful if using this to copy a free entry.
 */
uint32 jer2_qax_mcdb_copy_write(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 src_index, /* index of entry to be copied */
    DNX_SAND_IN uint32 dst_index  /* index of entry to be copied to and written to disk */
)
{
  uint32 data[JER2_QAX_MC_ENTRY_SIZE];
  jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
  jer2_qax_mcdb_entry_t *src_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, src_index);
  jer2_qax_mcdb_entry_t *dst_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, dst_index);
  DNXC_INIT_FUNC_DEFS;

  data[0] = dst_entry->word0 = src_entry->word0;
  data[1] = dst_entry->word1 = src_entry->word1;
  dst_entry->word2 &= JER2_QAX_MCDS_LAST_WORD_KEEP_BITS_MASK;
  dst_entry->word2 |= src_entry->word1 & ~JER2_QAX_MCDS_LAST_WORD_KEEP_BITS_MASK;
  data[2] = src_entry->word2 & JER2_QAX_MC_ENTRY_MASK_VAL;
  DNXC_IF_ERR_EXIT(WRITE_TAR_MCDBm(unit, MEM_BLOCK_ANY, dst_index, data));

exit:
  DNXC_FUNC_RETURN;
}


static INLINE void jer2_qax_mcdb_entry_clear_mcdb_bits(jer2_qax_mcdb_entry_t *mcdb_entry)
{
    mcdb_entry->word1 = mcdb_entry->word0 = 0;
    mcdb_entry->word2 &= ~JER2_QAX_MC_ENTRY_MASK_VAL;
}


/*
 * Remove the given free entries block from the given block list.
 * Does not modify the block itself.
 */
static void _jer2_qax_mcds_remove_free_entries_block_from_list(jer2_qax_mcds_t *mcds, dnx_free_entries_block_list_t *list, uint32 block, const dnx_free_entries_block_size_t block_size)
{
  uint32 next, prev;

  DNX_MC_ASSERT(block <= JER2_QAX_LAST_MCDB_ENTRY && block > 0);
  DNX_MC_ASSERT(JER2_QAX_MCDS_GET_TYPE(mcds, block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
  DNX_MC_ASSERT(block_size > 0 && block_size == JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block));
  DNX_MC_ASSERT(list == jer2_qax_mcds_get_region(mcds, block)->lists + (block_size-1));
  next = JER2_QAX_MCDS_GET_FREE_NEXT_ENTRY(mcds, block);
  prev = JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, block);
  if (next == block) { /* this was the only entry in the list */
    DNX_MC_ASSERT(prev == block && list->first == block);
    list->first = DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY; /* make list as empty */
  } else { /* the list has more entries */
    DNX_MC_ASSERT(prev != block && 
      JER2_QAX_MCDS_GET_TYPE(mcds, prev) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
      JER2_QAX_MCDS_GET_TYPE(mcds, next) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    DNX_MC_ASSERT(JER2_QAX_MCDS_GET_FREE_NEXT_ENTRY(mcds, prev) == block &&
                JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, next) == block);
    JER2_QAX_MCDS_SET_FREE_NEXT_ENTRY(mcds, prev, next);
    JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, next, prev);
    if (list->first == block) {
      list->first = next; /* If this was the list start, advance list start to the next block */
    }
  }
  LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
              (BSL_META("removed(%u,%u) "), block, block_size));
  jer2_qax_mcds_unoccupied_decrease(mcds, block_size); /* subtract the block size from the number of free entries */
}

/*
 * Remove the given free entries block from the given block list.
 * Does not modify the block itself.
 */
static INLINE void
  jer2_qax_mcds_remove_free_entries_block_from_list(jer2_qax_mcds_t *mcds, dnx_free_entries_block_list_t *list, uint32 block)
{
  _jer2_qax_mcds_remove_free_entries_block_from_list(mcds, list, block, JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block));
}

/*
 * Get the region corresponding to the given mcdb index
 */
dnx_free_entries_blocks_region_t* jer2_qax_mcds_get_region(jer2_qax_mcds_t *mcds, uint32 mcdb_index)
{
  DNX_MC_ASSERT(mcds && mcdb_index > 0 && mcdb_index <= JER2_QAX_LAST_MCDB_ENTRY);

  if (mcdb_index >= mcds->ingress_starts.range_start && mcdb_index <= mcds->ingress_starts.range_end) {
    return &mcds->ingress_starts;
  }
  if (mcdb_index >= mcds->egress_starts.range_start && mcdb_index <= mcds->egress_starts.range_end) {
    return &mcds->egress_starts;
  }
  return &mcds->no_starts;
}

/*
 * Remove the given free entries block from the given region's block list.
 * Does not modify the block itself.
 */
static INLINE void
  jer2_qax_mcds_remove_free_entries_block_from_region(jer2_qax_mcds_t *mcds, dnx_free_entries_blocks_region_t *region, uint32 block, dnx_free_entries_block_size_t block_size)
{
  dnx_free_entries_block_list_t *list = region->lists + (block_size-1);

  DNX_MC_ASSERT(block_size <= region->max_size);
  _jer2_qax_mcds_remove_free_entries_block_from_list(mcds, list, block, block_size);
}

/*
 * Check if the given free entries block list is empty.
 * return non zero if empty.
 */
static INLINE int
  jer2_qax_mcds_is_empty_free_entries_block_list(const jer2_qax_mcds_t *mcds, const dnx_free_entries_block_list_t *list)
{ 

  if (list->first == DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY)
    return 1;
  DNX_MC_ASSERT(list->first <= JER2_QAX_LAST_MCDB_ENTRY);
  return 0;
}

/*
 * Add the given free entries block to the given block list.
 * Does not modify the block itself.
 */
static void
  jer2_qax_mcds_add_free_entries_block_to_list(jer2_qax_mcds_t *mcds, dnx_free_entries_block_list_t *list, uint32 block)
{
  uint32 next, prev;
  dnx_free_entries_block_size_t block_size = JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block);

  if (jer2_qax_mcds_is_empty_free_entries_block_list(mcds, list)) {
    list->first = prev = next = block;
  } else {
    next = list->first;
    prev = JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, next);
    DNX_MC_ASSERT(JER2_QAX_MCDS_GET_FREE_NEXT_ENTRY(mcds, prev) == next &&
      JER2_QAX_MCDS_GET_TYPE(mcds, next) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, next, block);
    JER2_QAX_MCDS_SET_FREE_NEXT_ENTRY(mcds, prev, block);
  }
  JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, block, prev);
  JER2_QAX_MCDS_SET_FREE_NEXT_ENTRY(mcds, block, next);
  jer2_qax_mcds_unoccupied_increase(mcds, block_size);
} 

/*
 * Return the first entry of the given free entries block list, or 0 if it is empty.
 * If to_remove is non zero, the found block will be removed from the list (and not otherwise changed).
 */
uint32 jer2_qax_mcds_get_free_entries_block_from_list(jer2_qax_mcds_t *mcds, dnx_free_entries_block_list_t *list, int to_remove)
{

  uint32 block = list->first;
  if (block == DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY)
    return 0;
  DNX_MC_ASSERT(block <= JER2_QAX_LAST_MCDB_ENTRY);
  DNX_MC_ASSERT(JER2_QAX_MCDS_GET_TYPE(mcds, block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);

  if (to_remove) {
    jer2_qax_mcds_remove_free_entries_block_from_list(mcds, list, block);
  }
  return block;
}


/*
 * Create a free block of a given size at the given entry.
 * add it to a free entries block list at the given region.
 * The block entries are assumed to be (marked as) free.
 * The created block or part of it may be merge with adjacent free blocks based on flags.
 */
#define JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV  1 /* Will not merge with the previous consecutive free block */
#define JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT  2 /* Will not merge with the next consecutive free block */
#define JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE          3 /* Will not merge with consecutive free blocks */
static uint32 jer2_qax_mcds_create_free_entries_block(
    DNX_SAND_INOUT jer2_qax_mcds_t                 *mcds,
    DNX_SAND_IN    uint32                           flags,             /* DNX_MCDS_GET_FREE_BLOCKS_* flags that affect merging with adjacent free blocks */
    DNX_SAND_IN    uint32                           block_start_index, /* start index of the free block */
    DNX_SAND_IN    dnx_free_entries_block_size_t    block_size,        /* number of entries in the block */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region            /* region to contain the block in its lists */
)
{
  int unit = mcds->unit;
  uint32 i, current_block_start_index = block_start_index;
  uint32 block_end = block_start_index + block_size; /* the index of the entry immediately after the block */
  dnx_free_entries_block_size_t current_block_size = block_size, joint_block_size;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(region);
  if (block_start_index + block_size > mcds->ingress_bitmap_start || !block_start_index) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("block out of range")));
  }
  if (block_size > region->max_size || block_size < 1) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid block size")));
  }

  /* check the block's entries */
  for (i = block_start_index; i < block_end; ++i) {
    if (DNX_MCDS_TYPE_IS_USED(JER2_QAX_MCDS_GET_TYPE(mcds, i))) {
      DNX_MC_ASSERT(0); /* the entries of the block must be free */
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("attempted to add a used entry number %u to a free block"), i));
    }
  }

  /* if block is not of max size, attempt to merge with adjacent blocks */
  if (block_size < region->max_size) {
    const uint32 next_block = block_start_index + block_size;
    uint32 prev_block = block_start_index - 1;
    dnx_free_entries_block_size_t prev_block_size = 0, next_block_size = 0;
    
    if (!(flags & JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV) && /* get information on the previous adjacent block */
        prev_block >= region->range_start && prev_block <= region->range_end &&
        region == jer2_qax_mcds_get_region(mcds, prev_block) &&
        DNX_MCDS_TYPE_IS_FREE(i = JER2_QAX_MCDS_GET_TYPE(mcds, prev_block))) {
      prev_block_size = 1;
      if (i != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) { /* block size > 1 */
        prev_block = JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, prev_block);
        prev_block_size = block_start_index - prev_block;
        DNX_MC_ASSERT(JER2_QAX_MCDS_GET_TYPE(mcds, prev_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
          prev_block < block_start_index - 1 && prev_block_size <= region->max_size);
      }
      DNX_MC_ASSERT(prev_block_size == JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, prev_block));
      if (prev_block_size == region->max_size) {
        prev_block_size = 0; /* do not merge with max size blocks */
      }
    }
 
    if (!(flags & JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT) && /* get information on the next adjacent block */
        next_block >= region->range_start && next_block <= region->range_end &&
        region == jer2_qax_mcds_get_region(mcds, next_block) &&
        JER2_QAX_MCDS_GET_TYPE(mcds, next_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
      next_block_size = JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, next_block);
      if (next_block_size == region->max_size) {
        next_block_size = 0; /* do not merge with max size blocks */

      } else if (prev_block_size) { /* if we can merge in both directions, choose one direction */
        if (block_size + prev_block_size <= region->max_size) { /* If we can merge with the whole previous block, select it */
        } else if (block_size + next_block_size <= region->max_size) { /* If we can merge with the whole next block, select it */
          prev_block_size = 0;
        } else if (prev_block_size > next_block_size) { /* else merge with the smaller block ,or with the previous if they are of the same size */
          prev_block_size = 0;
        }
      }
    }

    if (prev_block_size) { /* merge with the previous block */

      joint_block_size = prev_block_size + block_size;
      if (joint_block_size > region->max_size) {
        current_block_size = joint_block_size - region->max_size;
        joint_block_size = region->max_size;
      } else {
        current_block_size = 0;
      }
      current_block_start_index = prev_block + joint_block_size;
      DNX_MC_ASSERT(joint_block_size + current_block_size == prev_block_size + block_size &&
        prev_block + joint_block_size == block_start_index + block_size - current_block_size);
      jer2_qax_mcds_remove_free_entries_block_from_region(mcds, region, prev_block, prev_block_size); /* remove the previous block from free blocks list */

      LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                  (BSL_META_U(unit,
                              "merge with prev free block: prev:%u,%u  freed:%u,%u\n"), prev_block, prev_block_size, block_start_index, block_size));
      JER2_QAX_MCDS_SET_FREE_BLOCK_SIZE(mcds, prev_block, joint_block_size); /* mark the new previous block size */
      /* mark the type of the block entries added to the previous block */
      for (i = block_start_index; i < current_block_start_index; ++i) {
        JER2_QAX_MCDS_SET_TYPE(mcds, i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
        JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, i, prev_block);
      }
      jer2_qax_mcds_add_free_entries_block_to_list(mcds, region->lists + (joint_block_size-1), prev_block); /* add the previous block to different free blocks list */

    } else if (next_block_size) { /* merge with the next block */

      joint_block_size = next_block_size + block_size;
      if (joint_block_size > region->max_size) {
        current_block_size = joint_block_size - region->max_size;
        joint_block_size = region->max_size;
      } else {
        current_block_size = 0;
      }
      current_block_start_index += joint_block_size;
      DNX_MC_ASSERT(joint_block_size + current_block_size == next_block_size + block_size &&
        block_start_index + joint_block_size == next_block + next_block_size - current_block_size);

      LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                  (BSL_META_U(unit,
                              "merge with next free block: next:%u,%u  freed:%u,%u\n"), next_block, next_block_size, block_start_index, block_size));
      jer2_qax_mcds_remove_free_entries_block_from_region(mcds, region, next_block, next_block_size); /* remove the next block from free blocks list */
      /* mark the type of the block entries */
      JER2_QAX_MCDS_SET_FREE_BLOCK_SIZE(mcds, block_start_index, joint_block_size); /* set the block size, to be called for the first block entry */ \
      JER2_QAX_MCDS_SET_TYPE(mcds, block_start_index, DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
      for (i = block_start_index + 1; i < current_block_start_index; ++i) {
        JER2_QAX_MCDS_SET_TYPE(mcds, i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
        JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, i, block_start_index);
      }
      jer2_qax_mcds_add_free_entries_block_to_list(mcds, region->lists + (joint_block_size-1), block_start_index); /* add the previous block to different free blocks list */
 
    }

  }

  if (current_block_size) {
    /* mark the block's size */
    JER2_QAX_MCDS_SET_FREE_BLOCK_SIZE(mcds, current_block_start_index, current_block_size); /* set the block size, to be called for the first block entry */ \
    LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                (BSL_META_U(unit,
                            "added free block: %u,%u\n"), current_block_start_index, current_block_size));
    /* mark the type of the block entries */
    JER2_QAX_MCDS_SET_TYPE(mcds, current_block_start_index, DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    block_end = current_block_start_index + current_block_size;
    for (i = current_block_start_index + 1; i < block_end; ++i) {
      JER2_QAX_MCDS_SET_TYPE(mcds, i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
      JER2_QAX_MCDS_SET_FREE_PREV_ENTRY(mcds, i, current_block_start_index);
    }

    /* add the block to the appropriate list of free blocks */
    jer2_qax_mcds_add_free_entries_block_to_list(mcds, region->lists + (current_block_size-1), current_block_start_index);
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * split a given free block to two blocks: a block of a given size, and the remaining entries.
 * The remaining entries will be added as a new block or merged to an existing block based on flags.
 * It is assumed that the block entries are marked appropriately as free.
 * The new details of the block are returned. Its position changes if the remaining entries are
 * placed at the start of the original block to enable their merge.
 * The input block must not belong to a block list.
 * If a merge is performed, the involved block lists are updated
 */
static uint32
  jer2_qax_mcds_split_free_entries_block(
    DNX_SAND_INOUT jer2_qax_mcds_t                       *mcds,       /* MC Data Structure object */
    DNX_SAND_IN    uint32                           flags,       /* RAD_SWDB_MCDB_GET_FREE_BLOCKS_* flags that affect what the function does group */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region,     /* region containing the block */
    DNX_SAND_IN    dnx_free_entries_block_size_t    orig_size,   /* the size of block to be split (must be bigger than new_size) */
    DNX_SAND_IN    dnx_free_entries_block_size_t    new_size,    /* The new block size (of the sub block that will be returned) if one would have been returned, split it */
    DNX_SAND_INOUT uint32                           *block_start /* the start of the block, updated by the function */
)
{
  int unit = mcds->unit;
  uint32 i;
  const uint32 next_block = *block_start + orig_size;
  uint32 prev_block = *block_start - 1;
  const dnx_free_entries_block_size_t remaining_size = orig_size - new_size;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mcds);
  DNXC_NULL_CHECK(block_start);
  if (orig_size > region->max_size || new_size < 1 || new_size >= orig_size) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("illegal size parameters")));
  }

  if (!(flags & JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT) &&  /* attempt to merge to next block */
      next_block >= region->range_start && next_block <= region->range_end &&
      region == jer2_qax_mcds_get_region(mcds, next_block) &&
      JER2_QAX_MCDS_GET_TYPE(mcds, next_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
    dnx_free_entries_block_size_t merged_block_size = JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, next_block);
    dnx_free_entries_block_size_t joint_block_size = merged_block_size + remaining_size;
    if (joint_block_size <= region->max_size) { /* The merged block will not be too big, perform the merge with the next block */
      DNX_MC_ASSERT(next_block - remaining_size == new_size + *block_start);
      jer2_qax_mcds_remove_free_entries_block_from_region(mcds, region, next_block, merged_block_size);
      DNXC_IF_ERR_EXIT(jer2_qax_mcds_create_free_entries_block( /* add the merged block to the region */
        mcds, JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, next_block - remaining_size, joint_block_size, region));
      goto exit;
    }
  }
  
  if (!(flags & JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV) &&  /* attempt to merge to previous block */
             prev_block >= region->range_start && prev_block <= region->range_end &&
             region == jer2_qax_mcds_get_region(mcds, prev_block) &&
             DNX_MCDS_TYPE_IS_FREE(i = JER2_QAX_MCDS_GET_TYPE(mcds, prev_block))) {
    dnx_free_entries_block_size_t merged_block_size = 1, joint_block_size;
    if (i != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) { /* block size > 1 */
      prev_block = JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, prev_block);
      merged_block_size = *block_start - prev_block;
      DNX_MC_ASSERT(JER2_QAX_MCDS_GET_TYPE(mcds, prev_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
        prev_block < *block_start - 1 && merged_block_size <= region->max_size);
    }
    DNX_MC_ASSERT(merged_block_size == JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, prev_block));
    joint_block_size = merged_block_size + remaining_size;
    if (joint_block_size <= region->max_size) { /* The merged block will not be too big, perform the merge with the previous block */
      DNX_MC_ASSERT(prev_block + joint_block_size == remaining_size + *block_start);
      jer2_qax_mcds_remove_free_entries_block_from_region(mcds, region, prev_block, merged_block_size);
      DNXC_IF_ERR_EXIT(jer2_qax_mcds_create_free_entries_block( /* add the merged block to the region */
        mcds, JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT, prev_block, joint_block_size, region));
      *block_start += remaining_size;
      goto exit;
    }
  }
  
  /* did not merge, add the remaining entries as a new block */
  DNXC_IF_ERR_EXIT(jer2_qax_mcds_create_free_entries_block( /* add the merged block to the region */
    mcds, JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, *block_start + new_size, remaining_size, region));

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Add free entries in the given range as blocks to the lists of the given region.
 * If (check_free) add only blocks marked as free.
 * Otherwise add all entries in the range are expected to be marked used and they will be marked as free.
 */
uint32 jer2_qax_mcds_build_free_blocks(
    DNX_SAND_IN    int                              unit,   /* used only if check_free is zero */
    DNX_SAND_INOUT jer2_qax_mcds_t                       *mcds,
    DNX_SAND_IN    uint32                           start_index, /* start index of the range to work on */
    DNX_SAND_IN    uint32                           end_index,   /* end index of the range to work on, if smaller than start_index then do nothing */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region,     /* region to contain the block in its lists */
    DNX_SAND_IN    dnx_mcds_free_build_option_t         entry_option /* which option to use in selecting entries to add and verifying them */
)
{
    dnx_free_entries_block_size_t max_size, block_size;
    uint32 block_start, cur_entry;
    int check_free = entry_option == dnxMcdsFreeBuildBlocksAddOnlyFree;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(region);
    DNX_MC_ASSERT(check_free || mcds == dnx_get_mcds(unit));
    if (start_index > end_index) {
        SOC_EXIT;
    }
    if (end_index >= mcds->ingress_bitmap_start || !start_index) { /* index out of allowed range */
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("block out of range")));
    }

    max_size = region->max_size;

    for (block_start = start_index; block_start <= end_index; block_start += block_size) { /* loop over the index range */
        if (check_free) {
            block_size = 0;
            for (; block_start <= end_index && DNX_MCDS_TYPE_IS_USED(JER2_QAX_MCDS_GET_TYPE(mcds, block_start));
                ++block_start) {} /* find the next free entry */
            if (block_start <= end_index) { /* found a block start */
                block_size = 1;
                for (cur_entry = block_start + 1; block_size < max_size && cur_entry <= end_index && /* get the current free entries block */
                  DNX_MCDS_TYPE_IS_FREE(JER2_QAX_MCDS_GET_TYPE(mcds, cur_entry)); ++cur_entry ) {
                    ++block_size;
                }
            }
        } else { /* add all entries */
            block_size = block_start + max_size <= end_index ? max_size : end_index - block_start + 1;
        }
        if (block_size) { /* found a free entries block (at least one entry) */
            DNX_MC_ASSERT(block_size <= max_size);
            if (!check_free) { /* mark the block entries as free if working in the mode in which they are used */
                dnx_free_entries_block_size_t i;
                for (i = 0; i < block_size; ++i) {
                    DNX_MC_ASSERT(entry_option != dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed ||
                      DNX_MCDS_TYPE_IS_USED(JER2_QAX_MCDS_GET_TYPE(mcds, block_start + i)));
                    JER2_QAX_MCDS_SET_TYPE(mcds, block_start + i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
                    /* write the entry as free in hardware, can be optimized by DMA */
                    DNXC_IF_ERR_EXIT(WRITE_TAR_MCDBm(unit, MEM_BLOCK_ANY, block_start + i, &((jer2_qax_mcds_t*)mcds)->free_value));
                }
            }
            DNXC_IF_ERR_EXIT( /* add the found block to the region */
              jer2_qax_mcds_create_free_entries_block(mcds, 0, block_start, block_size, region));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

/* Get the type of a MCDB entry */
uint32 jer2_qax_get_mcdb_entry_type(
    DNX_SAND_IN  dnx_mcdb_entry_t* entry
)
{
    return JER2_QAX_MCDS_ENTRY_GET_TYPE((jer2_qax_mcdb_entry_t*)entry);
}
/* set the type of a MCDB entry */
void jer2_qax_set_mcdb_entry_type(
    DNX_SAND_INOUT  dnx_mcdb_entry_t* entry,
    DNX_SAND_IN     uint32 type_value
)
{
    jer2_qax_mcdb_entry_t *e = (jer2_qax_mcdb_entry_t*)entry;
    JER2_QAX_MCDS_ENTRY_SET_TYPE(e, type_value);
}

/*
 * Get a pointer to the mcdb entry with the givin index in the mcds
 */
dnx_mcdb_entry_t*
  jer2_qax_mcds_get_mcdb_entry(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 mcdb_index
)
{
  return ((jer2_qax_mcds_t*)dnx_get_mcds(unit))->mcdb + mcdb_index;
}
/*
 * Get a pointer to the mcdb entry with the given index in the mcds
 */
dnx_mcdb_entry_t* jer2_qax_mcds_get_mcdb_entry_from_mcds(
    DNX_SAND_IN  dnx_mcds_t* mcds,
    DNX_SAND_IN  uint32 mcdb_index
)
{
  return ((jer2_qax_mcds_t*)mcds)->mcdb + mcdb_index;
}


/*
 * Get the (pointer to the) next entry from the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 */
uint32 jer2_qax_mcdb_get_next_pointer(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry,      /* entry from which to get the next entry pointer */
    DNX_SAND_IN  uint32  entry_type, /* the type of the entry */
    DNX_SAND_OUT uint32  *next_entry /* the output next entry */
)
{
    jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry = mcds->mcdb + entry;
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(DNX_MCDS_TYPES_ARE_THE_SAME(entry_type, JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry)));

    if (DNX_MCDS_TYPE_IS_INGRESS(entry_type)) { /* set ingress entry pointer */
        if (mcdb_entry->word2 & 0x80) { /* select memory format based on the format type soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, FROMATf) */
            *next_entry = (mcdb_entry->word2 & 0x40) ? JER2_QAX_MC_INGRESS_LINK_PTR_END : entry + 1; /* soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, LASTf); */
        } else { /* we have a LINK_PTR field */
            if (SOC_IS_QUX(unit)) {
                /* LINK_PTRf: 68:53 */
                *next_entry = (mcdb_entry->word1 >> 21) | ((mcdb_entry->word2 & 0x1f) << 11); /* soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, LINK_PTRf); */
            } else {
                /* LINK_PTRf: 69:53 */
                *next_entry = (mcdb_entry->word1 >> 21) | ((mcdb_entry->word2 & 0x3f) << 11); /* soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, LINK_PTRf); */
            }            
        }
    } else { /* get egress entry pointer */
        *next_entry = (mcdb_entry->word1 >> 20) | ((mcdb_entry->word2 & 0x1f) << 12); /* soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, LINK_PTRf); */
    }
    DNX_MC_ASSERT(*next_entry < mcds->ingress_bitmap_start || *next_entry == JER2_QAX_MC_INGRESS_LINK_PTR_END);

    DNXC_FUNC_RETURN;
}


/*
 * Set the pointer to the next entry in the given entry.
 * The entry type (ingress/egress) is given as an argument.
 * changes both the mcds and hardware.
 */
uint32 jer2_qax_mcdb_set_next_pointer(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  entry_to_set, /* index of the entry in which to set the pointer */
    DNX_SAND_IN  uint32  entry_type,   /* the type of entry_to_set */
    DNX_SAND_IN  uint32  next_entry    /* the entry that entry_to_set will point to */
)
{
    jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry = mcds->mcdb + entry_to_set;
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(DNX_MCDS_TYPES_ARE_THE_SAME(entry_type, JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry)));

    DNX_MC_ASSERT(next_entry < mcds->ingress_bitmap_start || next_entry == JER2_QAX_MC_INGRESS_LINK_PTR_END);
    if (DNX_MCDS_TYPE_IS_INGRESS(entry_type)) { /* set ingress entry pointer */
        if (mcdb_entry->word2 & 0x80) { /* select memory format based on the format type soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, FROMATf) */
            mcdb_entry->word2 &= ~(uint32)0x40;
            if (next_entry == JER2_QAX_MC_INGRESS_LINK_PTR_END) {
                mcdb_entry->word2 |= 0x40;
            
            } else if ( next_entry != entry_to_set + 1) {
                DNX_MC_ASSERT(0);
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("cannot set pointer in this entry type")));
            }
        } else { /* we have a LINK_PTR field */
            mcdb_entry->word1 &= 0xffe00000; /* soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, LINK_PTRf, next_entry); */
            mcdb_entry->word1 |= (next_entry << 21);
            if (SOC_IS_QUX(unit)) {
                mcdb_entry->word2 &= 0xffffffe0;
            } else {    
                mcdb_entry->word2 &= 0xffffffc0;
            }
            mcdb_entry->word2 |= (next_entry >> 11);
        }
    } else { /* set egress entry pointer */
        mcdb_entry->word1 &= 0xfff00000; /* soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, LINK_PTRf, next_entry); */
        mcdb_entry->word1 |= (next_entry << 20);
        mcdb_entry->word2 &= 0xffffffe0;
        mcdb_entry->word2 |= (next_entry >> 12);
    }

    DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, entry_to_set)); /* write to hardware */

exit:
    DNXC_FUNC_RETURN;
}



/* Allocate and init the mcds structure, not allocating memories it points to */
uint32 jer2_qax_init_mcds(
    DNX_SAND_IN    int         unit
)
{
    jer2_qax_mcds_t *jer2_qax_mcds;
    soc_dnx_config_jer2_qax_t *jer2_qax_conf = SOC_DNX_CONFIG(unit)->jer2_qax;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_alloc_mcds(unit, sizeof(*jer2_qax_mcds), (void*)&jer2_qax_mcds));

    jer2_qax_mcds->unit = unit;
    jer2_qax_mcds->common.flags = 0;
    jer2_qax_mcds->common.get_mcdb_entry_type = jer2_qax_get_mcdb_entry_type;
    jer2_qax_mcds->common.set_mcdb_entry_type = jer2_qax_set_mcdb_entry_type;
    jer2_qax_mcds->common.get_mcdb_entry_from_mcds = jer2_qax_mcds_get_mcdb_entry_from_mcds;
    jer2_qax_mcds->common.get_next_pointer = jer2_qax_mcdb_get_next_pointer;
    jer2_qax_mcds->common.set_next_pointer = jer2_qax_mcdb_set_next_pointer;
    jer2_qax_mcds->common.ingress_link_end = JER2_QAX_MC_INGRESS_LINK_PTR_END;
    jer2_qax_mcds->free_value.word0 = JER2_QAX_MC_FREE_ENTRY_0;
    jer2_qax_mcds->free_value.word1 = JER2_QAX_MC_FREE_ENTRY_1;
    jer2_qax_mcds->free_value.word2 = JER2_QAX_MC_FREE_ENTRY_2;
    jer2_qax_mcds->empty_ingr_value.word0 = JER2_JER_MC_ING_EMPTY_ENTRY_0;
    jer2_qax_mcds->empty_ingr_value.word1 = JER2_JER_MC_ING_EMPTY_ENTRY_1;
    jer2_qax_mcds->empty_ingr_value.word2 = JER2_JER_MC_ING_EMPTY_ENTRY_2;
    /* The offset in the MCDB to which the MCID is added to get the first entry of the group of core 0 */
    jer2_qax_mcds->egress_mcdb_offset = SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids;

    jer2_qax_mcds->egress_bitmap_start = JER2_QAX_NOF_MCDB_ENTRIES - jer2_qax_conf->nof_egress_bitmaps * jer2_qax_conf->nof_egress_bitmap_bytes;
    jer2_qax_mcds->ingress_bitmap_start = jer2_qax_mcds->egress_bitmap_start - jer2_qax_conf->nof_ingress_bitmaps * jer2_qax_conf->nof_ingress_bitmap_bytes;
    DNX_MC_ASSERT(jer2_qax_mcds->egress_mcdb_offset + SOC_DNX_CONFIG(unit)->tm.nof_mc_ids <= jer2_qax_mcds->ingress_bitmap_start &&
      jer2_qax_conf->nof_ingress_bitmap_bytes <= 4 && jer2_qax_conf->nof_egress_bitmap_bytes <= 4 &&
      jer2_qax_conf->nof_ingress_bitmap_bytes && jer2_qax_conf->nof_egress_bitmap_bytes);
exit:
    DNXC_FUNC_RETURN;
}


/* De-allocate the mcds structure, not de-allocating memories it points to */
uint32 jer2_qax_deinit_mcds(
    DNX_SAND_IN    int         unit
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_dealloc_mcds(unit));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * De-initialize the MultiCast Data Structures.
 */
uint32
    jer2_qax_mcds_multicast_terminate(
        DNX_SAND_IN int unit
    )
{
    jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnxc_free_mem(unit, (void**)&mcds->mcdb));
    DNXC_IF_ERR_EXIT(jer2_qax_deinit_mcds(unit));

    SOC_EXIT;
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Initialize the MultiCast Data Structures.
 * Do not fill the data from hardware yet.
 * jer2_qax_mcds_multicast_init2() will be called to do so when we can access the MCDB using DMA.
 */
uint32
    jer2_qax_mcds_multicast_init(
      DNX_SAND_IN int unit
)
{
    uint32 start, end;
    jer2_qax_mcds_t *mcds = NULL;
    int failed = 1;
    uint32 table_size;
    uint8 is_allocated;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;
    if (!SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCDS not initialized")));
    }
    DNXC_IF_ERR_EXIT(jer2_qax_init_mcds(unit)); /* allocate and init mcds */
    mcds = dnx_get_mcds(unit);
    table_size = JER2_QAX_LAST_MCDB_ENTRY + 1;

    /* init the members of jer2_qax_mcds_t */
    mcds->nof_unoccupied = 0;
    mcds->mcdb = NULL;

    /* Init the free entries region where ingress linked lists may start */
    start = SOC_DNX_CONFIG(unit)->tm.ingress_mc_id_alloc_range_start;
    DNX_MC_ASSERT(start <= 0);
    end   = SOC_DNX_CONFIG(unit)->tm.ingress_mc_id_alloc_range_end;
    if (start < 1) { /* both does not let allocating entry 0, and handles a -1 value for start */
        start = 1;
    }
    if (end >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids) {
        end = SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids - 1;
    }
    if (end < start)  { /* if the region should be empty */
        start = JER2_QAX_LAST_MCDB_ENTRY + 1;
        end = JER2_QAX_LAST_MCDB_ENTRY;
    }
    dnx_mcds_init_region(&mcds->ingress_starts, DNX_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED, start, end);

    dnx_mcds_init_region(&mcds->no_starts, DNX_MCDS_MAX_FREE_BLOCK_SIZE_GENERAL, 1, mcds->ingress_bitmap_start - 1); /* all of the MCDB entries except for the first and the last ones can be used as free entries */

    /* Init the free entries region where egress linked lists may start */
    if (SOC_DNX_CONFIG(unit)->tm.nof_mc_ids > 0) {
        start = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, 0);
        end = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids - 1);
        DNX_MC_ASSERT(SOC_DNX_CONFIG(unit)->tm.ingress_mc_id_alloc_range_end + 1 == start &&
          SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids == start && end < mcds->ingress_bitmap_start);
    } else {
        start = JER2_QAX_LAST_MCDB_ENTRY + 1;
        end = JER2_QAX_LAST_MCDB_ENTRY;
    }
    dnx_mcds_init_region(&mcds->egress_starts, DNX_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED, start, end);

    assert(sizeof(jer2_qax_mcdb_entry_t) == sizeof(uint32)*JER2_QAX_MC_ENTRY_SIZE &&
           sizeof(int) >= sizeof(int32)); /* The mcds will not work if for some reason this is not true */

    { /* allocate memory for the arrays */
        DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &mcds->mcdb, sizeof(jer2_qax_mcdb_entry_t) * table_size, "mcds-mc-mcdb"));
        if(!SOC_WARM_BOOT(unit)) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.is_allocated(unit, &is_allocated);
            DNXC_IF_ERR_EXIT(rv);
            if(!is_allocated) {
                rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.alloc(unit);
                DNXC_IF_ERR_EXIT(rv);
            }
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.alloc_bitmap(unit, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high = -1; /* From JER2_QAX forward, there are no direct bitmap MC groups */
    failed = 0;
exit:
    if (failed && mcds) {
        jer2_qax_mcds_multicast_terminate(unit);
    }
    DNXC_FUNC_RETURN;
}

/*
 * Initialize the MultiCast Data Structures.
 * Must be run after jer2_qax_mcds_multicast_init() was called successfully, and when DMA is up.
 * fills the multicast data from hardware.
 */
uint32
    jer2_qax_mcds_multicast_init2(
      DNX_SAND_IN int unit
)
{
    unsigned i;
    uint32 *alloced_mem = NULL; /* memory allocated for the duration of this function */
    uint32 *dest32;
    jer2_qax_mcdb_entry_t *dest;
    jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
    uint32 table_size, irdb_table_nof_entries, r32;
    int do_not_read = !SOC_WARM_BOOT(unit);
    int use_dma = !do_not_read &&
#ifdef PLISIM
      !SAL_BOOT_PLISIM &&
#endif
      soc_mem_dmaable(unit, TAR_MCDBm, SOC_MEM_BLOCK_ANY(unit, TAR_MCDBm)); /* check if we can use DMA */
    int failed = 1;
    uint32 range_start, range_end, last_end;

    DNXC_INIT_FUNC_DEFS;
    if (!SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) {
        SOC_EXIT;
    }
    if (!mcds || !mcds->mcdb) { /* jer2_qax_mcds_multicast_init() was not called successfully */
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCDS not initialized")));
    }
    table_size = JER2_QAX_LAST_MCDB_ENTRY + 1;
    irdb_table_nof_entries = (SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids + (JER2_QAX_NOF_GROUPS_PER_IRDB_ENTRY -1)) / JER2_QAX_NOF_GROUPS_PER_IRDB_ENTRY; /*number of used entries in the CGM_IS_ING_MC table */

    if (!SOC_WARM_BOOT(unit)) {
        if (!SOC_DNX_CONFIG(unit)->jer2_arad->init.pp_enable) {
            DNXC_IF_ERR_EXIT(WRITE_EGQ_INVALID_OUTLIFr(unit, REG_PORT_ANY, DNX_MC_EGR_CUD_INVALID));
            DNXC_IF_ERR_EXIT(WRITE_EPNI_INVALID_OUTLIFr(unit, REG_PORT_ANY, DNX_MC_EGR_CUD_INVALID));
        }
        /* loop over active cores, setting egress MC MCDB offset for each core */
        DNXC_IF_ERR_EXIT(READ_EGQ_MULTICAST_OFFSET_ADDRESSr(unit, 0, &r32));
        soc_reg_field_set(unit, EGQ_MULTICAST_OFFSET_ADDRESSr, &r32, MCDB_OFFSETf, JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, 0));
        DNXC_IF_ERR_EXIT(WRITE_EGQ_MULTICAST_OFFSET_ADDRESSr(unit, 0, r32));
    }

    dest = mcds->mcdb;

    /* fill mcdb from hardware, including if each entry is used or not, use (read) TAR_MCDBm and IDR_IRDBm */
    /* allocate buffers and read tables differently depending on if DMA is enabled */
    if (use_dma) { /* use DMA */
        jer2_qax_mcdb_entry_t *src;
        /* alloced_mem will first contain TAR_MCDBm and later contain IDR_IRDBm */
        alloced_mem = soc_cm_salloc(unit, 4 * (table_size * JER2_QAX_MC_ENTRY_SIZE), "dma-mc-buffer");
        if (alloced_mem == NULL) {
            DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate DMA buffer")));
        }
        DNXC_IF_ERR_EXIT(soc_mem_read_range(unit, TAR_MCDBm, MEM_BLOCK_ANY, 0, JER2_QAX_LAST_MCDB_ENTRY, alloced_mem));

        /* copy mcdb from dma buffer to to software database */
        src = (void*)alloced_mem;
        for (i = table_size ; i ; --i) {
            dest->word0 = src->word0;
            dest->word1 = src->word1;
            dest++->word2 = src++->word2 & JER2_QAX_MC_ENTRY_MASK_VAL;
        }

    } else { /* do not use DMA, read tables entry by entry */
        DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &alloced_mem, sizeof(uint32) * irdb_table_nof_entries * IRDB_TABLE_ENTRY_WORDS, "mcds-irdb-tmp"));

        /* read mcdb entry by entry into software database. */

        if (do_not_read) { /* if not in warm boot, the MCDB and IRDB tables that we have just filled */
            for (i = JER2_QAX_LAST_MCDB_ENTRY + 1 ; i ; --i) {
                *dest = mcds->free_value;
                ++dest;
            }
        } else {
            DNXC_IF_ERR_EXIT(soc_mem_read_range(unit, TAR_MCDBm, MEM_BLOCK_ANY, 0, JER2_QAX_LAST_MCDB_ENTRY, dest));
            for (i = table_size; i; --i) {
                dest++->word2 &= JER2_QAX_MC_ENTRY_MASK_VAL;
            }
        }

    }
    if (!do_not_read && irdb_table_nof_entries > 0) {
        DNXC_IF_ERR_EXIT(soc_mem_read_range(unit, CGM_IS_ING_MCm, MEM_BLOCK_ANY, 0, irdb_table_nof_entries - 1, alloced_mem));
    }

#ifdef BCM_WARM_BOOT_SUPPORT /* #if defined(MCAST_WARM_BOOT_UPDATE_ENABLED) && defined(BCM_WARM_BOOT_SUPPORT) ### */
    if (SOC_WARM_BOOT(unit)) { /* if warm boot */
      /* We now have the mcds initialized from TAR_MCDB, and all entries are marked as unused. We will now process ingress groups */
        uint32 mcid = 0;
        unsigned j;
        uint16 group_entries;
        dest32 = alloced_mem;
        for (i = 0 ; i < irdb_table_nof_entries; ++i) {
            uint32 bits = *dest32;
            for (j = 0 ; j < JER2_QAX_NOF_GROUPS_PER_IRDB_ENTRY; ++j) {
                if (bits & 1) { /* we found an open ingress multicast group and will traverse it */
                    /* traverse group, needed for warm boot support, mark them with the correct type and update prev_entries */
                    jer2_qax_mcdb_entry_t *mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, mcid);
                    uint32 cur_entry = soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, LINK_PTRf);
                    uint32 prev_entry = mcid;

                    if (mcid >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids || JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
                        DNX_MC_ASSERT(0); /* MCID is out of range or the entry is already used */
                        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("found invalid hardware table values")));
                    }
                    JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS_START); /* mark the first group entry as the start of an ingress group */
                    JER2_QAX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, prev_entry);
                    group_entries = 1;
                    while (cur_entry < mcds->ingress_bitmap_start) { /* mark the rest of the group as non first entries of an ingress group. */
                        mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, cur_entry);
                        if (JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START || ++group_entries > JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS) {
                            DNX_MC_ASSERT(0);
                            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("entry already used or too many group entries")));
                        }
                        JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS);
                        JER2_QAX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, prev_entry);
                        prev_entry = cur_entry;
                        cur_entry = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, LINK_PTRf);
                    }
                    DNX_MC_ASSERT(cur_entry == JER2_QAX_MC_INGRESS_LINK_PTR_END); /* If this fails, we have an illegal pointer value */
                }
                bits >>= 1;
                ++mcid;
            }
            dest32 += IRDB_TABLE_ENTRY_WORDS;
        }

        /* We will now traverse the egress groups (bitmap and regular) and build their data. */
        for (mcid = 0; mcid < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids; ++mcid) {
            uint8 bit_val;
            DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(unit, mcid, &bit_val));
            if (bit_val) { /* we found an open egress multicast group */
                /* traverse linked list group, needed for warm boot support, mark them with the correct type and update prev_entries */
                uint32 prev_entry = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid);
                jer2_qax_mcdb_entry_t *mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, prev_entry);
                uint32 cur_entry;
 
                if (JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
                    DNX_MC_ASSERT(0);
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("entry already used")));
                }
                JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_EGRESS_START); /* mark the first group entry as the start of an egress group */
                JER2_QAX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, prev_entry);
                DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, prev_entry, DNX_MCDS_TYPE_VALUE_EGRESS_START, &cur_entry)); /* Get the next entry */
                group_entries = 1;
                while (cur_entry != DNX_MC_EGRESS_LINK_PTR_END) { /* mark the rest of the group as non first entries of an egress group. */
                    mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, cur_entry);
                    if (JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START || ++group_entries > JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS) {
                        DNX_MC_ASSERT(0);
                        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("entry already used or too many group entries")));
                    }
                    JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_EGRESS);
                    JER2_QAX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, prev_entry);
                    prev_entry = cur_entry;
                    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, prev_entry, DNX_MCDS_TYPE_VALUE_EGRESS, &cur_entry)); /* Get the next entry */
                }
            }
        }

    } else
#endif /* BCM_WARM_BOOT_SUPPORT */
    {
        /* We now have the mcds initialized from TAR_MCDB, and all entries are marked as unused. We will now process ingress groups */
        dest32 = alloced_mem;
        for (i = 0 ; i < irdb_table_nof_entries; ++i) {
            if (*dest32) {
                DNX_MC_ASSERT(0);
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Illegal TAR_MCDB content")));
            }
        dest32 += IRDB_TABLE_ENTRY_WORDS;
        }

    }

    /* Now we finished marking all the used entries.
     * We will now process the rest of the entries (excluding the first entry)
     * and create free blocks from them. */

    /* add free entries from the first half of the table after the ingress allocation range */
    range_start = mcds->ingress_starts.range_start;
    range_end = mcds->ingress_starts.range_end;
    if (range_start < mcds->ingress_bitmap_start && range_end >= range_start) { /* If we have a valid ingress range */
        DNX_MC_ASSERT(range_start >= 1);
        /* add free entries before the ingress allocation/linked list start range */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, 1, range_start - 1, &mcds->no_starts, dnxMcdsFreeBuildBlocksAddOnlyFree));
        /* add free entries from the ingress allocation/linked list start range */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, range_start, range_end, &mcds->ingress_starts, dnxMcdsFreeBuildBlocksAddOnlyFree));
        last_end = range_end;
    } else {
        last_end = 0; /* set the start of the next general free entries range */
    }
    /* add free entries from the first half of the table after the ingress allocation range, and from the second half before the egress allocation range */
    range_start = mcds->egress_starts.range_start;
    range_end = mcds->egress_starts.range_end;
    if (range_start < mcds->ingress_bitmap_start && range_end >= range_start) { /* If we have a valid egress range */
        DNX_MC_ASSERT(range_start == last_end + 1 && range_end < mcds->ingress_bitmap_start);
        /* add free entries from the egress allocation/linked list start range */
        if (range_end >= mcds->ingress_bitmap_start) {
            range_end = mcds->ingress_bitmap_start - 1;
        }
        /* add free entries from the egress allocation/linked list start range */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, range_start, range_end, &mcds->egress_starts, dnxMcdsFreeBuildBlocksAddOnlyFree));
        last_end = range_end;
    }
    /* add the remaining entries as free */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, last_end + 1, mcds->ingress_bitmap_start - 1, &mcds->no_starts, dnxMcdsFreeBuildBlocksAddOnlyFree));
    failed = 0;

exit:
    if (alloced_mem) {
        if (use_dma) {
            soc_cm_sfree(unit, alloced_mem);
        } else {
            DNXC_IF_ERR_EXIT(dnxc_free_mem(unit, (void*)&alloced_mem));
        }
    }
    if (failed) {
        jer2_qax_mcds_multicast_terminate(unit);
    }
    DNXC_FUNC_RETURN;
}



/*
 * free blocks handling
 */


/* function to compare replications, assumes the comparison function used by dnx_sand_os_qsort does not require negatice return values  */
int jer2_qax_rep_data_t_compare(void *a, void *b)
{ /* assumes sizeof(int) >= sizeof(uint32) which is already assumed in the driver */
  const jer2_qax_rep_data_t *ca = b;
  const jer2_qax_rep_data_t *cb = a;
  return COMPILER_64_LT(*cb, *ca) ? 0 : 1;
}


/*
 * Functions to add replications to the MCDS, used to store them temporarily during an API call.
 */

static void jer2_qax_add_ingress_replication_dest( /* Add a destination + CUD/s replication to the CMDS replications data structure */
  jer2_qax_mcds_t   *mcds,
  const uint32 dest,
  const uint32 cud,
  const uint32 cud2
)
{
    jer2_qax_rep_data_t *rep = mcds->reps + mcds->nof_reps;
    DNX_MC_ASSERT(mcds->group_type == DNX_MCDS_TYPE_VALUE_INGRESS && mcds->nof_reps < JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS &&
      mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps == mcds->nof_reps);
    JER2_QAX_MCDS_REP_DATA_CLEAR(rep);
    JER2_QAX_MCDS_REP_DATA_SET_DEST(rep, dest);
    JER2_QAX_MCDS_REP_DATA_SET_CUD1(rep, cud);
    JER2_QAX_MCDS_REP_DATA_SET_CUD2(rep, cud2);
    /* no need for: JER2_QAX_MCDS_REP_DATA_SET_TYPE(rep, JER2_QAX_MCDS_REP_TYPE_DEST); since its value is 0 */
    ++mcds->nof_reps;
    ++mcds->nof_dest_cud_reps;
}

static void jer2_qax_add_ingress_replication_bitmap( /* Add a bitmap pointer + CUD/s replication to the CMDS replications data structure */
  jer2_qax_mcds_t   *mcds,
  const uint32 bm_id,
  const uint32 cud,
  const uint32 cud2
)
{
    jer2_qax_rep_data_t *rep = mcds->reps + mcds->nof_reps;
    DNX_MC_ASSERT(mcds->group_type == DNX_MCDS_TYPE_VALUE_INGRESS && mcds->nof_reps < JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS &&
      mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps == mcds->nof_reps);
    JER2_QAX_MCDS_REP_DATA_CLEAR(rep);
    JER2_QAX_MCDS_REP_DATA_SET_DEST(rep, bm_id);
    JER2_QAX_MCDS_REP_DATA_SET_CUD1(rep, cud);
    JER2_QAX_MCDS_REP_DATA_SET_CUD2(rep, cud2);
    JER2_QAX_MCDS_REP_DATA_SET_TYPE(rep, JER2_QAX_MCDS_REP_TYPE_BM);
    ++mcds->nof_reps;
    ++mcds->nof_bitmap_reps;
}


static void jer2_qax_add_egress_replication_port( /* Add a destination + CUD/s replication to the CMDS replications data structure */
  jer2_qax_mcds_t *mcds,
  const uint32     tm_port,
  const uint32     cud,
  const uint32     cud2
)
{
    jer2_qax_rep_data_t *rep = mcds->reps + mcds->nof_reps;
    DNX_MC_ASSERT(mcds->group_type == DNX_MCDS_TYPE_VALUE_EGRESS && mcds->nof_reps < JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps == mcds->nof_reps);
    JER2_QAX_MCDS_REP_DATA_CLEAR(rep);
    JER2_QAX_MCDS_REP_DATA_SET_DEST(rep, tm_port);
    JER2_QAX_MCDS_REP_DATA_SET_CUD1(rep, cud);
    JER2_QAX_MCDS_REP_DATA_SET_CUD2(rep, cud2);
    /* no need for: JER2_QAX_MCDS_REP_DATA_SET_TYPE(rep, JER2_QAX_MCDS_REP_TYPE_DEST); since its value is 0 */
    ++mcds->nof_reps;
    ++mcds->nof_dest_cud_reps;
}

static void jer2_qax_add_egress_replication_bitmap( /* Add a bitmap pointer + CUD replication to the CMDS replications data structure */
  jer2_qax_mcds_t   *mcds,
  const uint32 bm_id,
  const uint32 cud
)
{
    jer2_qax_rep_data_t *rep = mcds->reps + mcds->nof_reps;
    DNX_MC_ASSERT(mcds->group_type == DNX_MCDS_TYPE_VALUE_EGRESS && mcds->nof_reps < JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps == mcds->nof_reps);
    JER2_QAX_MCDS_REP_DATA_CLEAR(rep);
    JER2_QAX_MCDS_REP_DATA_SET_DEST(rep, bm_id);
    JER2_QAX_MCDS_REP_DATA_SET_CUD1(rep, cud);
    /* no need for: JER2_QAX_MCDS_REP_DATA_SET_CUD2(rep, DNX_MC_NO_2ND_CUD); since its value is 0 */
    JER2_QAX_MCDS_REP_DATA_SET_TYPE(rep, JER2_QAX_MCDS_REP_TYPE_BM);
    ++mcds->nof_reps;
    ++mcds->nof_bitmap_reps;
}


/*
 * Adds the contents of a mcdb entry of the given type to the replications stored for the current API in mcds.
 * No more than *max_size replications are added, and the max_size value
 * is decreased by the number of added replications.
 * *group_size is increased by the number of found replications.
 * The next entry pointed to by this entry is returned in next_entry.
 * Does not handle bitmap entries.
 */
static uint32 jer2_qax_mcds_get_replications_from_entry(
    DNX_SAND_IN    int     unit,
    DNX_SAND_IN    uint32  entry_index, /* table index of the entry */
    DNX_SAND_IN    uint32  entry_type,  /* the type of the entry */
    DNX_SAND_INOUT uint16  *max_size,   /* the maximum number of replications to return from the group, decreased by the number of returned replications */
    DNX_SAND_INOUT uint16  *group_size, /* incremented by the number of found replications (even if they are not returned) */
    DNX_SAND_OUT   uint32  *next_entry  /* the next entry */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry = mcds->mcdb + entry_index;
    uint16 found = 0, max = *max_size;
    uint32 format, dest, cud1, cud2, temp;
    soc_mem_t mcdb_bmp_repl_mem;
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(entry_type == JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */
    mcdb_bmp_repl_mem = TAR_MCDB_BITMAP_REPLICATION_ENTRYm;
    if (DNX_MCDS_TYPE_IS_INGRESS(entry_type)) { /* get replications and next entry from an ingress format */

        format = soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, FORMATf); /* similiar format field for all ingress formats */
        cud1 = soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, CUD_0f); /* same 1st CUD field for all ingress formats */
        cud2 = soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, CUD_1f); /* same 1st CUD field for all ingress formats */
        switch (format) { /* select memory format based on the format field */
          case 0:
            temp = soc_mem_field32_get(unit, mcdb_bmp_repl_mem, mcdb_entry, BMP_PTRf);
            dest = (temp - mcds->ingress_bitmap_start) / SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmap_bytes;
            DNX_MC_ASSERT(temp >= mcds->ingress_bitmap_start && temp < mcds->egress_bitmap_start &&
              temp == mcds->ingress_bitmap_start + dest * SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmap_bytes);
            if (++found <= max) { /* no Invalid value for this destination */
                jer2_qax_add_ingress_replication_bitmap(mcds, dest, cud1, cud2);
            }
            *next_entry = soc_mem_field32_get(unit, mcdb_bmp_repl_mem, mcdb_entry, LINK_PTRf);
            break;

          case 1:
            dest = soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, DESTINATIONf);
            if (dest != JER2_QAX_MC_ING_DESTINATION_DISABLED && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_ingress_replication_dest(mcds, dest, cud1, cud2);
            }
            *next_entry = soc_mem_field32_get(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, LINK_PTRf);
            break;

          default:
            /* The DNX_MULT_EGRESS_PORT_INVALID destination value was not validated to disable replication in this format */
            if (++found <= max) {
                jer2_qax_add_ingress_replication_dest(mcds, soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, DESTINATION_0f), cud1, DNX_MC_NO_2ND_CUD);
            }
            if (++found <= max) {
                jer2_qax_add_ingress_replication_dest(mcds, soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, DESTINATION_1f), cud2, DNX_MC_NO_2ND_CUD);
            }
            *next_entry = soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, LASTf) ? JER2_QAX_MC_INGRESS_LINK_PTR_END : entry_index + 1;
        } /* end of the switch */

    } else if (DNX_MCDS_TYPE_IS_EGRESS_NORMAL(entry_type)) { /* get replications and next entry from an egress format */
        format = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, ENTRY_FORMATf); /* same format field for all egress formats */
        cud1 = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, OUTLIFf); /* same 1st CUD field for all egress formats */
        *next_entry = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, LINK_PTRf); /* same link pointer field for all egress formats */
        switch (format) { /* select memory format based on the format field */
          case 0:

            temp = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, BMP_PTRf);
            if (temp != DNX_MC_EGR_BITMAP_DISABLED && ++found <= max) { /* replication not disabled for this entry */
            dest = (temp - mcds->egress_bitmap_start) / SOC_DNX_CONFIG(unit)->jer2_qax->nof_egress_bitmap_bytes;
            DNX_MC_ASSERT(temp >= mcds->egress_bitmap_start && temp < JER2_QAX_NOF_MCDB_ENTRIES && temp == mcds->egress_bitmap_start + dest * SOC_DNX_CONFIG(unit)->jer2_qax->nof_egress_bitmap_bytes);
                jer2_qax_add_egress_replication_bitmap(mcds, dest, cud1);
            }            
            break;

          case 1:
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, PP_DSP_Af);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, DNX_MC_NO_2ND_CUD);
            }
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, PP_DSP_Bf);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, soc_mem_field32_get(unit,
                  TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, OUTLIF_Bf), DNX_MC_NO_2ND_CUD);
            }
            break;

          case 2:
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_Bf);
            cud2 = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, OUTLIF_2f);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, cud2);
            }
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_Af);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, cud2);
            }
            break;

          case 5:
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_1f);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, DNX_MC_NO_2ND_CUD);
            }
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_2f);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, DNX_MC_NO_2ND_CUD);
            }
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_3f);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, DNX_MC_NO_2ND_CUD);
            }
            dest = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_4f);
            if (dest != DNX_MULT_EGRESS_PORT_INVALID && ++found <= max) { /* replication not disabled for this entry part */
                jer2_qax_add_egress_replication_port(mcds, dest, cud1, DNX_MC_NO_2ND_CUD);
            }
            break;

          default:
            DNX_MC_ASSERT(0);
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("unsupported egress format found")));
        } /* end of the switch */
    }

    if (found < max) {
        *max_size -= found;
    } else {
        *max_size = 0;
    }
    *group_size += found;

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Returns a gport destination from an ingress MC destination encoding.
 * Converts the hardware encoding to the appropriate destination type.
 */
static uint32 jer2_qax_convert_ingress_destination_hw2api(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint32       dest,      /* destination to be converted */
    DNX_SAND_OUT soc_gport_t  *out_gport /* output array to contain ports/destinations */
)
{
    uint32 type_bits = dest >> JER2_QAX_MC_ING_DESTINATION_ID_BITS;
    DNXC_INIT_FUNC_DEFS;

    /* interpret destination type and set gport accordingly */
    if (type_bits == JER2_QAX_MC_ING_DESTINATION_Q_TYPE) { /* the destination is a queue */
        _SHR_GPORT_UNICAST_QUEUE_GROUP_SET(*out_gport, dest);
    } else if (type_bits == JER2_QAX_MC_ING_DESTINATION_SYSPORT_TYPE) { /* the destination is a system port */
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        uint32 fap_id, fap_port_id;
        DNXC_SAND_IF_ERR_EXIT(jer2_arad_sys_phys_to_local_port_map_get(unit, dest & JER2_QAX_MC_ING_DESTINATION_SYSPORT_MASK, &fap_id, &fap_port_id)); 
        _SHR_GPORT_MODPORT_SET(*out_gport, fap_id, fap_port_id);
#endif 
    } else if (type_bits == JER2_QAX_MC_ING_DESTINATION_SYSPORT_LAG_TYPE) { /* the destination is a LAG */
        _SHR_GPORT_TRUNK_SET(*out_gport, dest & JER2_QAX_MC_ING_DESTINATION_SYSPORT_MASK);
    } else {
          DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("unexpected destination type")));
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * clear the replications data in the mcds
 */
static void jer2_qax_mcds_clear_replications(int unit, jer2_qax_mcds_t *mcds, const uint32 group_type)
{
    mcds->group_type = DNX_MCDS_TYPE_GET_NONE_START(group_type);
    mcds->group_type_start = DNX_MCDS_TYPE_GET_START(group_type);
    mcds->nof_dest_cud_reps = mcds->nof_bitmap_reps = mcds->nof_reps = 0;
    if (mcds->group_type == DNX_MCDS_TYPE_VALUE_EGRESS) {
        mcds->alloc_flags = DNX_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS | DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL;
        mcds->hw_end = DNX_MC_EGRESS_LINK_PTR_END;
    } else {
        mcds->alloc_flags = DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL;
        mcds->hw_end = JER2_QAX_MC_INGRESS_LINK_PTR_END;
    }
}

/*
 * This functions copies the replication data from the mcds into the given gport and encap_id arrays.
 * It is an error if more entries are to be copied than available in the arrays.
 */
uint32 jer2_qax_mcds_copy_replications_to_arrays(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint8        is_egress,           /* are the replications for an egress multicast group (opposed to ingress) */
    DNX_SAND_IN  uint32       arrays_size,         /* size of output arrays */
    DNX_SAND_OUT soc_gport_t  *port_array,         /* output array to contain logical ports/destinations, used if !reps */
    DNX_SAND_OUT soc_if_t     *encap_id_array,     /* output array to contain encapsulations/CUDs/outlifs, used if !reps */
    DNX_SAND_OUT soc_multicast_replication_t *reps /* output replication array (array of size mc_group_size*/
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 dest, rep_type, cud1, cud2;
    uint16 size;
    jer2_qax_rep_data_t *rep;
    soc_gport_t out_gport = 0;
    soc_port_t local_logical_port;
    DNXC_INIT_FUNC_DEFS;

    /* this code depends on the implementation of the mcds storage, but is faster */
    if ((size = mcds->nof_reps) > arrays_size) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications to return")));
    }

    /* loop over the replications returning them in the appropriate arrays */
    for (rep = mcds->reps; size; --size, ++rep) {
        JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, dest, cud1, cud2, rep_type); /* get the replication fields */
        if ((cud1 == SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud) &&
            (!is_egress)) {
            cud1 = 0;
        }
        if (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST) {
            if (is_egress) {
                DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, 0, dest, &local_logical_port));
                _SHR_GPORT_LOCAL_SET(out_gport, local_logical_port); /* set the local logical port */
            } else { /* handle ingress destination replications */
                /* convert ingress hardware fields to API representation */
                DNXC_IF_ERR_EXIT(jer2_qax_convert_ingress_destination_hw2api(unit, dest, &out_gport));
            }
        } else { /* bitmap replication */
            _SHR_GPORT_MCAST_SET(out_gport, dest); /* set the gport representation of the bitmap id */
        }

        if (reps) {
            reps->flags = (cud2 == DNX_MC_NO_2ND_CUD) ? 0 : SOC_MUTICAST_REPLICATION_ENCAP2_VALID;
            reps++->port = out_gport;   /* destination */
            reps->encap2 = cud2;
            reps->encap1 = cud1;
        } else {
            *(port_array++) = out_gport;
            *(encap_id_array++) = cud1;
        }
    }

exit:
  DNXC_FUNC_RETURN;
}


/* get the tm port from the gport (local port) after it was processed by bcm - not in gport format */
static soc_error_t jer2_qax_get_tm_port_from_gport(int unit, const uint32 port, uint32 *tm_port)
{
    int core;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, tm_port, &core));
    if (*tm_port >= DNX_MULT_EGRESS_PORT_INVALID) { /* disabled or invalid port */
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid port")));
    }
    DNX_MC_ASSERT(core == 0);
exit:
    DNXC_FUNC_RETURN;
}


/*
 * This functions copies the egress replication data from the given replication array into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 * The function also converts each logical port to core + TM port in Jericho.
 */
uint32 jer2_qax_mcds_copy_egress_replications_from_arrays(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint8     do_clear,        /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32    arrays_size,     /* size of output arrays */
    DNX_SAND_IN  dnx_mc_replication_t *reps /* input array containing replications using logical ports */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 size_left = arrays_size, dest, cud, cud2;
    DNXC_INIT_FUNC_DEFS;

    if (do_clear) {
        jer2_qax_mcds_clear_replications(unit, mcds, DNX_MCDS_TYPE_VALUE_EGRESS);
    }
    DNX_MC_ASSERT(mcds->group_type == DNX_MCDS_TYPE_VALUE_EGRESS && mcds->nof_reps == (int32)mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps);
    if (mcds->nof_reps + arrays_size > JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    }
    for (; size_left; --size_left) {
        dest = reps->dest;
        cud = reps->cud;
        cud2 = (reps++)->additional_cud & ~DNX_MC_2ND_CUD_IS_EEI; 
        if (dest == _SHR_GPORT_INVALID) { /* CUD only replication */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("The device does not support CUD only replications")));
        } else if (dest & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* CUD+bitmap replication, added to all cores */
            if (cud2 != DNX_MC_NO_2ND_CUD) { /* We have a 2nd CUD */
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("The device does not support egress bitmap replications with a 2nd CUD")));
            }
            jer2_qax_add_egress_replication_bitmap(mcds, dest & ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT, cud);
        } else { /* port+CUD/s replication */
            DNXC_IF_ERR_EXIT(jer2_qax_get_tm_port_from_gport(unit, dest, &dest));
            jer2_qax_add_egress_replication_port(mcds, dest, cud, cud2);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This functions copies the replication data from a consecutive egress entries block into the mcds (core 0).
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * All the block entries except for the last point implicitly to the next entry using formats 5,7.
 */
uint32
  jer2_qax_mcds_copy_replications_from_egress_block( 
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  uint8                         do_clear,    /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32                        block_start, /* index of the block start */
    DNX_SAND_IN  dnx_free_entries_block_size_t block_size,  /* number of entries in the block */
    DNX_SAND_INOUT uint32                      *cud2,       /* the current 2nd CUD of replications */
    DNX_SAND_OUT uint32                        *next_entry  /* the next entry pointed to by the last block entry */
)
{
  jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
  uint32 cur_index = block_start;
  dnx_free_entries_block_size_t entries_remaining = block_size;
  uint16 nof_reps = 0, reps_left;
  DNXC_INIT_FUNC_DEFS;

  DNX_MC_ASSERT(block_start + block_size < mcds->ingress_bitmap_start);
  if (do_clear) {
    jer2_qax_mcds_clear_replications(unit, mcds, DNX_MCDS_TYPE_VALUE_EGRESS);
  } else {
    nof_reps = mcds->nof_reps;
  }
  DNX_MC_ASSERT(nof_reps == mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps &&
    nof_reps < JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS && mcds->nof_reps == nof_reps);
  reps_left = JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS - nof_reps;

  /* get replications from the rest of the entries */
  while (entries_remaining) {
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_replications_from_entry( /* add replications to mcds from cur_index entry */
      unit, cur_index, DNX_MCDS_TYPE_VALUE_EGRESS, &reps_left, &nof_reps, next_entry));
    ++cur_index;
    --entries_remaining;
    if (nof_reps > JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS) {
      DNX_MC_ASSERT(0); /* group is somehow bigger than allowed - internal error */
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    } else if (entries_remaining && *next_entry == DNX_MC_EGRESS_LINK_PTR_END) {
      DNX_MC_ASSERT(0); /* entry does not point to next entry in the block - internal error */
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("block too small")));
    }
  }

exit:
  DNXC_FUNC_RETURN;
}

#if 0 
/*
 * This function the processes the replications of an ingress group checking for duplicate replications if needed.
 * If duplicate replications exist, and this is forbidden, an error will be returned.
 * If the group type is wrong, an error will be returned.
 */
uint32 jer2_qax_mult_process_ingress_replications(
    DNX_SAND_IN  int           unit,
    DNX_SAND_OUT uint16        *nof_replications, /* The number of replications in the mcds */
    DNX_SAND_OUT DNX_TMC_ERROR *out_err           /* return possible errors that the caller may want to ignore */
)
{
  jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(nof_replications);
  if (mcds->nof_dest_cud_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unexpected egress replications")));
  }
  *nof_replications = mcds->nof_reps;
  DNX_MC_ASSERT(*nof_replications <= JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS && *nof_replications == mcds->nof_ingr_reps);

  if (!(SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_ALLOW_DUPLICATES_MODE)) { /* need to check duplicate replications */
    uint16 i;
    jer2_qax_rep_data_t *cur_rep = mcds->reps;
    dnx_sand_os_qsort(cur_rep, *nof_replications, sizeof(jer2_qax_rep_data_t), jer2_qax_rep_data_t_compare);
    for (i = *nof_replications; i > 1; --i) {
      jer2_qax_rep_data_t *next_rep = cur_rep+1;
      if (JER2_QAX_EQ_REP_DATA(cur_rep, next_rep)) {
        *out_err = _SHR_E_PARAM;
        SOC_EXIT; /* We found duplicate replications in the input */
      }
      cur_rep = next_rep;
    }
  }

  *out_err = _SHR_E_NONE;
exit:
  DNXC_FUNC_RETURN;
}
#endif /* 0 */


/*
 * Get a free block of size 1 at a given location.
 * Used for getting the first entry of a multicast group.
 * Does not mark mcdb_index as used.
 */
uint32 jer2_qax_mcds_reserve_group_start(
    DNX_SAND_INOUT jer2_qax_mcds_t *mcds,
    DNX_SAND_IN    uint32           mcdb_index /* the mcdb index to reserve */
)
{
    int unit = mcds->unit;
    uint32 entry_type;
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(mcdb_index < mcds->ingress_bitmap_start);

    entry_type = JER2_QAX_MCDS_GET_TYPE(mcds, mcdb_index);
    if (DNX_MCDS_TYPE_IS_USED(entry_type)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("entry must be free")));
    }
    if (mcdb_index > 0 ) { /* entry needs allocation */
        dnx_free_entries_blocks_region_t* region = jer2_qax_mcds_get_region(mcds, mcdb_index);
        const uint32 block_start = entry_type == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START ?
          mcdb_index : JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, mcdb_index);
        const dnx_free_entries_block_size_t block_size = JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block_start);
        const uint32 block_end = block_start + block_size - 1;
        DNX_MC_ASSERT(block_start <= mcdb_index && block_start + region->max_size >= mcdb_index && block_size <= region->max_size);

        jer2_qax_mcds_remove_free_entries_block_from_region(mcds, region, block_start, block_size); /* remove the existing free block from the free list */
        if (block_start < mcdb_index) { /* create free block for entries before mcdb_index */
            DNXC_IF_ERR_EXIT(jer2_qax_mcds_create_free_entries_block(
              mcds, JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT, block_start, mcdb_index - block_start, region));
        }
        if (block_end > mcdb_index) { /* create free block for entries after mcdb_index */
            DNXC_IF_ERR_EXIT(jer2_qax_mcds_create_free_entries_block(
              mcds, JER2_QAX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, mcdb_index + 1, block_end - mcdb_index, region));
        }
    }
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Get a free entries block of a given size, according to flags that needs to be used to start a multicast group.
 * Returns the start index and the number of entries in the block.
 */
uint32 jer2_qax_mcds_get_free_entries_block(
    DNX_SAND_INOUT jer2_qax_mcds_t                    *mcds,
    DNX_SAND_IN    uint32                        flags,        /* DNX_MCDS_GET_FREE_BLOCKS_* flags that affect what the function does group */
    DNX_SAND_IN    dnx_free_entries_block_size_t wanted_size,  /* needed size of the free block group */
    DNX_SAND_IN    dnx_free_entries_block_size_t max_size,     /* do not return blocks above this size, if one would have been returned, split it */
    DNX_SAND_OUT   uint32                        *block_start, /* the start of the relocation block */
    DNX_SAND_OUT   dnx_free_entries_block_size_t *block_size   /* the size of the returned block */
)
{
    int unit = mcds->unit;
    dnx_free_entries_blocks_region_t *regions[JER2_ARAD_MCDS_NOF_REGIONS];
    int do_change = !(flags & DNX_MCDS_GET_FREE_BLOCKS_NO_UPDATES);
    uint32 block = 0;
    int r, s, loop_start, loop_end;
    int size_loop1_start = wanted_size, size_loop1_increase;
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(mcds);
    DNXC_NULL_CHECK(block_start);
    DNXC_NULL_CHECK(block_size);
    if (wanted_size > DNX_MCDS_MAX_FREE_BLOCK_SIZE || wanted_size > max_size || 1 > wanted_size) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("illegal wanted size")));
    }

    regions[0] = &mcds->no_starts;
    if (flags & DNX_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS) { /* select region order according to flags */
        regions[1] = &mcds->ingress_starts;
        regions[2] = &mcds->egress_starts;
    } else {
        regions[1] = &mcds->egress_starts;
        regions[2] = &mcds->ingress_starts;
    }

    /* select (loop) free entries block size order according to flags */
    size_loop1_increase = (flags & DNX_MCDS_GET_FREE_BLOCKS_PREFER_SMALL) ? -1 : 1;

    if (flags & DNX_MCDS_GET_FREE_BLOCKS_PREFER_SIZE) { /* Will prefer to return a block of a better size than in a better region */

        /* first loop over block sizes */
        loop_start = size_loop1_start;
        if (size_loop1_increase >= 0) { /* increasing loop */
            loop_end = DNX_MCDS_MAX_FREE_BLOCK_SIZE + 1;
        } else { /* decreasing loop */
            loop_end = 0;
        }
        for (s = loop_start; s != loop_end; s += size_loop1_increase) {
            for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
                dnx_free_entries_blocks_region_t *region = regions[r];
                if (region->max_size >= s) { /* if the current block size is supported by the region */
                    if ((block = jer2_qax_mcds_get_free_entries_block_from_list(mcds, region->lists + (s - 1), do_change))) {
                        goto found;
                    }
                }
            }
        }
        /* second loop over block sizes */
        loop_start = size_loop1_start - size_loop1_increase;
        if (size_loop1_increase <= 0) { /* increasing loop */
            loop_end = DNX_MCDS_MAX_FREE_BLOCK_SIZE + 1;
        } else { /* decreasing loop */
            loop_end = 0;
        }
        for (s = loop_start; s != loop_end; s -= size_loop1_increase) {
            for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
                dnx_free_entries_blocks_region_t *region = regions[r];
                if (region->max_size >= s) { /* if the current block size is supported by the region */
                    if ((block = jer2_qax_mcds_get_free_entries_block_from_list(mcds, region->lists + (s - 1), do_change))) {
                        goto found;
                    }
                }
            }
        }

    } else { /* Will prefer to return a block in a better region than a block of a better size. */

        for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
            dnx_free_entries_blocks_region_t *region = regions[r];
            /* coverity[pointer_outside_base_object:FALSE] */
            dnx_free_entries_block_list_t *lists = region->lists - 1;

            /* first loop over block sizes */
            loop_start = size_loop1_start;
            if (size_loop1_increase >= 0) { /* increasing loop */
                loop_end = region->max_size + 1;
                if (loop_start > loop_end) loop_start = loop_end;
            } else { /* decreasing loop */
                loop_end = 0;
                if (loop_start > region->max_size) loop_start = region->max_size;
            }
            for (s = loop_start; s != loop_end; s += size_loop1_increase) {
             if ((block = jer2_qax_mcds_get_free_entries_block_from_list(mcds, lists + s, do_change))) {
                 goto found;
             }
            }
            /* second loop over block sizes */
            loop_start = size_loop1_start - size_loop1_increase;
            if (size_loop1_increase <= 0) { /* increasing loop */
                loop_end = region->max_size + 1;
                if (loop_start > loop_end) loop_start = loop_end;
            } else { /* decreasing loop */
                loop_end = 0;
                if (loop_start > region->max_size) loop_start = region->max_size;
            }
            for (s = loop_start; s != loop_end; s -= size_loop1_increase) {
             if ((block = jer2_qax_mcds_get_free_entries_block_from_list(mcds, lists + s, do_change))) {
                 goto found;
             }
            }

        } /* end of regions loop */

    } /* end of preferred better region */

    DNX_MC_ASSERT(!mcds->nof_unoccupied);
    if (flags & DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL) {
        *block_start = 0;
        *block_size = 0;
        SOC_EXIT;
    }
    DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("did not find any free block")));

found:
    if (do_change && s > max_size) {
        DNX_MC_ASSERT(s <= DNX_MCDS_MAX_FREE_BLOCK_SIZE);
        DNXC_IF_ERR_EXIT( /* get free entries */
            jer2_qax_mcds_split_free_entries_block(mcds, flags, regions[r], s, max_size, &block));
        s = max_size;
    }

    *block_start = block;
    *block_size = s;
    DNX_MC_ASSERT(block && s);

exit:
    DNXC_FUNC_RETURN;
}

/* 
 * Given an MCDB index checks if that entry has no pointer and continues ti the following entry.
 * Assumes this is an ingress MC linked list entry.
 * returns 0 for entries with a pointer or no pointer which are the end of the linked list.
 * returns non zero for entries with no linked list pointer continuing to the next entry in the MCDB.
 */
static INLINE int jer2_qax_mcds_is_consecutive_format(
    DNX_SAND_IN jer2_qax_mcds_t *mcds,
    DNX_SAND_IN uint32     mcdb_index
)
{
    /* check the bit which says if this is a TAR_MCDB_DOUBLE_REPLICATIONm entry */
    /* return soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, JER2_QAX_GET_MCDB_ENTRY(mcds, mcdb_index), FORMATf) &&
         !soc_mem_field32_get(unit, TAR_MCDB_DOUBLE_REPLICATIONm, JER2_QAX_GET_MCDB_ENTRY(mcds, mcdb_index), FORMATf); */
    return ((JER2_QAX_GET_MCDB_ENTRY(mcds, mcdb_index)->word2 >> 6) & 3) == 2;
}

/*
 * Given a table index that needs to be used to start a multicast group,
 * returns the start index and the number of entries that need to be relocated.
 * If the number of entries returned is 0, a relocation is not needed.
 */
uint32
  jer2_qax_mcds_get_relocation_block(
    DNX_SAND_IN  jer2_qax_mcds_t                    *mcds,
    DNX_SAND_IN  uint32                        mcdb_index,              /* table index needed for the start of a group */
    DNX_SAND_OUT uint32                        *relocation_block_start, /* the start of the relocation block */
    DNX_SAND_OUT dnx_free_entries_block_size_t *relocation_block_size   /* the size of the relocation block, 0 if relocation is not needed */
)
{
    int unit = mcds->unit;
    uint32 group_entry_type, start = mcdb_index;
    dnx_free_entries_block_size_t size = 1;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(mcds);
    DNXC_NULL_CHECK(relocation_block_size);

    group_entry_type = JER2_QAX_MCDS_GET_TYPE(mcds, mcdb_index);
    /* relocation is needed if the entry is used and is not the start of a group */
    if (DNX_MCDS_TYPE_IS_USED(group_entry_type) && !DNX_MCDS_TYPE_IS_START(group_entry_type)) {
        if (group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS) { /* egress groups do not have used consecutive blocks */

            uint32 entry, next_entry;
            DNX_MC_ASSERT(group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS);
            for (entry = mcdb_index; ; entry = next_entry) { /* look for consecutive entries before the given one */
                if ((next_entry = JER2_QAX_MCDS_GET_PREV_ENTRY(mcds, entry)) + 1 != entry ||
                  !jer2_qax_mcds_is_consecutive_format(mcds, next_entry)) { /* previous entry is consecutive and has no link pointer */
                    break;
                }
                ++size;
                DNX_MC_ASSERT(next_entry && size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE &&
                  JER2_QAX_MCDS_GET_TYPE(mcds, next_entry) == DNX_MCDS_TYPE_VALUE_INGRESS); /* must be an ingress entry, and not the group start */
            }
            start = entry;

            /* look for consecutive entries after the given one */
            for (entry = mcdb_index; jer2_qax_mcds_is_consecutive_format(mcds, next_entry); entry = next_entry) {
                next_entry = entry + 1;
                ++size;
                DNX_MC_ASSERT(JER2_QAX_MCDS_GET_PREV_ENTRY(mcds,next_entry) == entry && next_entry < mcds->ingress_bitmap_start &&
                  next_entry && size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE &&
                  JER2_QAX_MCDS_GET_TYPE(mcds, next_entry) == DNX_MCDS_TYPE_VALUE_INGRESS); /* must be an ingress entry, and not the group start */
            }
            DNX_MC_ASSERT(entry - start + 1 == size);

        }
    } else { /* no relocation is needed */
        DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_FREE(group_entry_type) || (mcdb_index < SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids ? /* is ingress group */
          group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS_START : (DNX_MCDS_TYPE_IS_EGRESS_START(group_entry_type) &&
          mcdb_index < JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids))));
    }

    *relocation_block_size = size;
    if (relocation_block_start) {
        *relocation_block_start = start;
    }

exit:
    DNXC_FUNC_RETURN;
}



/*
 * Write a MCDB entry to hardware from the mcds.
 * Using only this function for writes, and using it after mcds mcdb used
 * entries updates, ensures consistency between the mcds and the hardware.
 */

uint32
  jer2_qax_mcds_write_entry(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 mcdb_index /* index of entry to write */
)
{
  uint32 data[JER2_QAX_MC_ENTRY_SIZE];
  jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
  jer2_qax_mcdb_entry_t *entry = JER2_QAX_GET_MCDB_ENTRY(mcds, mcdb_index);

  DNXC_INIT_FUNC_DEFS;

  data[0] = entry->word0;
  data[1] = entry->word1;
  data[2] = entry->word2 & JER2_QAX_MC_ENTRY_MASK_VAL;
  DNXC_IF_ERR_EXIT(WRITE_TAR_MCDBm(unit, MEM_BLOCK_ANY, mcdb_index, data));

exit:
  DNXC_FUNC_RETURN;
}

#if 0 
int _dnx_mcds_test_free_entries(
    DNX_SAND_IN int unit
)
{
  uint32 nof_unoccupied = 0;
  jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
  jer2_qax_mcdb_entry_t *entry, *entry2;
  JER2_ARAD_MULT_ID mcid;


  /* init test bit to be 1 if the entry is free, and count free entries */
  for (mcid = 0; mcid <= JER2_QAX_LAST_MCDB_ENTRY; ++mcid) {
    entry = JER2_QAX_GET_MCDB_ENTRY(mcds, mcid);
    if (DNX_MCDS_TYPE_IS_FREE(JER2_QAX_MCDS_ENTRY_GET_TYPE(entry))) {
      JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_ON(entry);
      ++nof_unoccupied;
    } else {
      JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    }
  }
  /* decrease from the free entry count the entriy which may not be allocated in case it is free */
  if (DNX_MCDS_TYPE_IS_FREE(JER2_QAX_MCDS_ENTRY_GET_TYPE(entry =
      JER2_QAX_GET_MCDB_ENTRY(mcds, DNX_MC_EGRESS_LINK_PTR_END)))) {
    DNX_MC_ASSERT(nof_unoccupied);
    JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    --nof_unoccupied;
  }
  if (nof_unoccupied != mcds->nof_unoccupied) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST,
             (BSL_META_U(unit,
                         "The mcdb has %lu free allocatable entries and in the mcds the value is %lu\n"), (unsigned long)nof_unoccupied, (unsigned long)mcds->nof_unoccupied));
    DNX_MC_ASSERT(0);
    return 10;
  }

  /* process over the free block lists */
  nof_unoccupied = 0;
  {
      dnx_free_entries_blocks_region_t *regions[JER2_ARAD_MCDS_NOF_REGIONS];
    int r;
    dnx_free_entries_block_size_t size, size_i;
    uint32 block, first_block, prev_block;
    regions[0] = &mcds->no_starts;
    regions[1] = &mcds->ingress_starts;
    regions[2] = &mcds->egress_starts;

    /* loop over regions, processing the entries of each block of each list; checking their validity and counting entries */
    for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) {
      dnx_free_entries_blocks_region_t *region = regions[r];
      dnx_free_entries_block_list_t *lists = region->lists;
      DNX_MC_ASSERT(region->max_size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE && region->max_size > 0);

      for (size = region->max_size; size; --size) { /* loop over the block sizes of the region */
        /* loop over the blocks in the list */
        if ((block = jer2_qax_mcds_get_free_entries_block_from_list(mcds, lists + size - 1, 0))) { /* if the list is not empty */
          prev_block = JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, block);
          first_block = block;

          /* loop over the free block in the list */
          do {
            entry = JER2_QAX_GET_MCDB_ENTRY(mcds, block);
            DNX_MC_ASSERT(block >= region->range_start && block + size - 1 <= region->range_end);
            DNX_MC_ASSERT(JER2_QAX_MCDS_ENTRY_GET_TYPE(entry) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
            DNX_MC_ASSERT(JER2_QAX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block) == size);
            DNX_MC_ASSERT(prev_block == JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, block));
            if (!JER2_QAX_MCDS_ENTRY_GET_TEST_BIT(entry)) {
              LOG_ERROR(BSL_LS_SOC_MULTICAST,
                       (BSL_META_U(unit,
                                   "Free block %lu of size %u appeared previously in a linked list\n"), (unsigned long)block, size));
              DNX_MC_ASSERT(0);
              return 20;
            }
            JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
            entry2 = entry;

            for (size_i = 1; size_i < size;  ++ size_i) { /* loop over remianing entries of the block */
              ++entry2;
              DNX_MC_ASSERT(JER2_QAX_MCDS_ENTRY_GET_TYPE(entry2) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
              DNX_MC_ASSERT(JER2_QAX_MCDS_ENTRY_GET_FREE_PREV_ENTRY(entry2) == block);
              if (!JER2_QAX_MCDS_ENTRY_GET_TEST_BIT(entry2)) {
                LOG_ERROR(BSL_LS_SOC_MULTICAST,
                         (BSL_META_U(unit,
                                     "Free entry %lu of free block %lu of size %u appeared previously in a linked list\n"),
                                     (unsigned long)(block + size ), (unsigned long)block, size));
                DNX_MC_ASSERT(0);
                return 30;
              }
            JER2_QAX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry2);
            }
            nof_unoccupied += size;
            prev_block = block;
            block = JER2_QAX_MCDS_GET_FREE_NEXT_ENTRY(mcds, block); /* move to new block */
          } while (block != first_block);
          DNX_MC_ASSERT(prev_block == JER2_QAX_MCDS_GET_FREE_PREV_ENTRY(mcds, block));
        }
      }
    }
  }
  if (nof_unoccupied != mcds->nof_unoccupied) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST,
             (BSL_META_U(unit,
                         "The mcdb free block lists contain %lu entries and in the mcds the value is %lu\n"), (unsigned long)nof_unoccupied, (unsigned long)mcds->nof_unoccupied));
    DNX_MC_ASSERT(0);
    return 40;
  }

  return 0;
}

#endif /* 0 */

/*
 * Code that use ti be in multicast_linked_list.c
 */


/*
 * This function checks if the multicast group is open (possibly empty).
 * returns TRUE if the group is open (start of a group), otherwise FALSE.
 */
uint32 jer2_qax_mult_does_group_exist(
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_MULT_ID mcid,       /* MC ID of the group */
    DNX_SAND_IN  int             is_egress,  /* is the MC group an egress group */
    DNX_SAND_OUT uint8           *does_exist /* output: if the group exists */
)
{
    const jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 nof_mc_ids = is_egress ? SOC_DNX_CONFIG(unit)->tm.nof_mc_ids : SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids;

    DNXC_INIT_FUNC_DEFS;

    *does_exist = FALSE;
    if (mcid >= nof_mc_ids) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("multicast ID %u is not under the number of multicast IDs: %u"), mcid, nof_mc_ids));
    }
    if (!is_egress) { /* ingress group */
        if (JER2_QAX_MCDS_GET_TYPE(mcds, mcid) == DNX_MCDS_TYPE_VALUE_INGRESS_START) {
            DNX_MC_ASSERT(mcid == JER2_QAX_MCDS_GET_PREV_ENTRY(mcds, mcid)); /* verify that mcid is a start of a linked list */
            *does_exist = TRUE;
        }
    } else { /* egress group */
        DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(unit, mcid, does_exist));
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Relocate the given used entries, not disturbing multicast traffic to
 * the group containing the entries.
 * SWDB and hardware are updated accordingly.
 * If relocation_block_size is 0, then the function calculates
 * relocation_block_start and relocation_block_size by itself to suit mcdb_index.
 * After successful relocation, the block that was relocated is freed, except for entry mcdb_index.
 *
 * This function may overwrite the mc group replications stored in the mcds.
 * Returns possible errors that the caller may want to ignore in mcds->out_err: insufficient memory.
 */

uint32
  jer2_qax_mcdb_relocate_entries(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  uint32                        mcdb_index,             /* table index needed for the start of a group */
    DNX_SAND_IN  uint32                        relocation_block_start, /* the start of the relocation block */
    DNX_SAND_IN  dnx_free_entries_block_size_t relocation_block_size   /* the size of the relocation block */
)
{
    uint32 start = relocation_block_start, found_block_start, prev_entry;
    dnx_free_entries_block_size_t size = relocation_block_size, found_block_size;
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 entry_type = JER2_QAX_MCDS_GET_TYPE(mcds, mcdb_index), cud2 = DNX_MC_NO_2ND_CUD;
    dnx_free_entries_blocks_region_t *region = 0;
    int free_alloced_block = 0;
    DNXC_INIT_FUNC_DEFS;

    if (!size) { /* need to calculate the relocated entries */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_relocation_block(mcds, mcdb_index, &start, &size));
        if (!size) {
            DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_FREE(JER2_QAX_MCDS_GET_TYPE(mcds, mcdb_index)));
            SOC_EXIT; /* a relocation is not needed */
        }
    }

    /* entry must be used, and if a block (size>1) is relocated, this must be non tdm egress */
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_USED(entry_type) && (size == 1 || entry_type == DNX_MCDS_TYPE_VALUE_INGRESS));

    DNXC_IF_ERR_EXIT( /* get free entries */
        jer2_qax_mcds_get_free_entries_block(mcds, DNX_MCDS_GET_FREE_BLOCKS_PREFER_SIZE | DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL,
            size, size, &found_block_start, &found_block_size));
    if (!found_block_size) {
        mcds->out_err = _SHR_E_FULL;
        SOC_EXIT;
    }
    DNX_MC_ASSERT(found_block_size <= size);
    prev_entry = JER2_QAX_MCDS_GET_PREV_ENTRY(mcds, start);

    free_alloced_block = 1;
    if (found_block_size == size) { /* can relocate all the block to the allocated block */
        dnx_free_entries_block_size_t s;
        uint32 after_block; /* the index if the entry after the block (pointed to by the last block entry) */

        DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds,unit, start + size - 1, entry_type, &after_block));

        for (s = 0; s < size; ++s) {
            DNXC_IF_ERR_EXIT( /* copy entry to new location and write to hardware */
                jer2_qax_mcdb_copy_write(unit, start + s, found_block_start + s));
            JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, found_block_start + s, s ? found_block_start + (s-1) : prev_entry);
        }
        DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* point to the new block instead of to the old one */
            mcds, unit, prev_entry, entry_type, found_block_start));
        free_alloced_block = 0;
        if (after_block != JER2_QAX_MC_INGRESS_LINK_PTR_END) { /* link the entry after the relocated block back to the new block */
            JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, after_block, found_block_start + size - 1);
        }
    } else { /* can not relocate as one block, try to build replacement linked list */
        uint32 list_next; /* the next entry in the list after the block to be relocated */
        free_alloced_block = 0;
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_replications_from_egress_block( /* set mcdb with the block's entries */
            unit, TRUE, start, size, &cud2, &list_next));

        DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list( /* replace block by a linked list */
            unit, DNX_MCDS_TYPE_VALUE_INGRESS, prev_entry, list_next, found_block_start, found_block_size));
        found_block_start = mcds->linked_list;
        if (mcds->out_err) {
            SOC_EXIT;
        }
    }

    /* after reallocating, need to free the old block except for the entry which will start the linked list of the new group */
    if (mcdb_index > start) { /* free entries in relocated block before the entry to be used as a start of a MC group */
        if (!region) {
            region = jer2_qax_mcds_get_region(mcds, mcdb_index);
        }
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, start, mcdb_index - 1, region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }
    if (mcdb_index + 1 < start + size) { /* free entries in relocated block before the entry to be used as a start of a MC group */
        if (!region) {
            region = jer2_qax_mcds_get_region(mcds, mcdb_index);
        }
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, mcdb_index + 1, start + (size - 1), region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }
    mcds->out_err = _SHR_E_NONE;

exit:
    if (free_alloced_block) {
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, 
            found_block_start, found_block_start + (found_block_size - 1), jer2_qax_mcds_get_region(mcds, found_block_start), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }
    DNXC_FUNC_RETURN;
}


/*
 * Free a linked list that is not used any more, updating mcds and hardware.
 * Stop at the given entry, and do not free it.
 * The given linked list must not include the first entry of the group.
 * All entries in the linked list should be of the given type.
 */

uint32
  jer2_qax_mcdb_free_linked_list_till_my_end(
    DNX_SAND_IN int    unit,
    DNX_SAND_IN uint32 first_index,  /* table index of the first entry in the linked list to free */
    DNX_SAND_IN uint32 entries_type, /* the type of the entries in the list */
    DNX_SAND_IN uint32 end_index     /* the index of the end of the list to be freed */
)
{
    uint32 cur_index;
    uint32 block_start = 0, block_end = 0; /* will mark a block of consecutive entries in the linked list to free together */
    uint32 range_start = 0, range_end = 0; /* will mark the consecutive range of the region in which the block can grow */
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    dnx_free_entries_blocks_region_t *region = 0;
    DNXC_INIT_FUNC_DEFS;

    for (cur_index = first_index; cur_index != end_index; ) { /* loop over the linked list */
        DNX_MC_ASSERT(cur_index > 0 && cur_index < mcds->ingress_bitmap_start);

        if (block_end) { /* we are in a block, try enlarging it with the entry */
            if (cur_index == block_end + 1 && cur_index <= range_end) { /* Can this entry join the block at its end? */
                block_end = cur_index;
            } else if (cur_index + 1 == block_start && cur_index >= range_start) { /* Can this entry join the block at its start? */
                block_start = cur_index;
            } else { /* can't enlarge block, add block to free list and start a new one */
                DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, block_start, block_end, region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
                region = jer2_qax_mcds_get_region_and_consec_range(mcds, cur_index, &range_start, &range_end); /* get range in which we can accumulate a block */
                block_start = block_end = cur_index;
            }
        } else { /* we are at at the first entry */
            region = jer2_qax_mcds_get_region_and_consec_range(mcds, cur_index, &range_start, &range_end); /* get range in which we can accumulate a block */
            block_start = block_end = cur_index;
        }
        DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, cur_index, entries_type, &cur_index));
    }
    if (block_end) { /* write the last discovered consecutive */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, block_start, block_end, region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Free a linked list that is not used any more, updating mcds and hardware.
 * The given linked list must not include the first entry of the group.
 * All entries in the linked list should be of the given type.
 */

uint32
  jer2_qax_mcdb_free_linked_list(
    DNX_SAND_IN int     unit,
    DNX_SAND_IN uint32  first_index, /* table index of the first entry in the linked list to free */
    DNX_SAND_IN uint32  entries_type /* the type of the entries in the list */
)
{
    return jer2_qax_mcdb_free_linked_list_till_my_end(unit, first_index, entries_type, 
      (DNX_MCDS_TYPE_IS_INGRESS(entries_type) ? JER2_QAX_MC_INGRESS_LINK_PTR_END : DNX_MC_EGRESS_LINK_PTR_END));
}


/*
 * Return the contents of a multicast linked list group of a given type.
 * The contents is returned in the mcds buffer. Egress destination ports are TM/PP ports.
 * if the group size is bigger than the specified max_size, only max_size entries are returned,
 * it will not be an error, and the actual size of the group is returned.
 * The group must be open.
 */

uint32 jer2_qax_mcds_get_group(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint8                do_clear,     /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32               group_id,     /* the mcid of the group */
    DNX_SAND_IN  uint32               group_type,   /* the type of the group (of the entries in the list) */
    DNX_SAND_IN  uint16               max_size,     /* the maximum number of members to return from the group */
    DNX_SAND_OUT uint16               *group_size   /* the returned actual group size */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 entry_type = DNX_MCDS_TYPE_GET_NONE_START(group_type); /* type for the non first entry */
    const uint16 max_group_size = DNX_MCDS_TYPE_IS_INGRESS(group_type) ? JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS : JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS;
    uint32 cur_index = group_id;
    uint16 entries_remaining = max_size > max_group_size ? max_group_size : max_size; /* max replications to return */
    unsigned is_egress = DNX_MCDS_TYPE_IS_EGRESS(group_type);

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(group_size);
    *group_size = 0;

    if (do_clear) {
      jer2_qax_mcds_clear_replications(unit, mcds, group_type); /* reset the returned replications to none */
    }
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_USED(group_type) && mcds->hw_end == (DNX_MCDS_TYPE_IS_INGRESS(group_type) ?
      JER2_QAX_MC_INGRESS_LINK_PTR_END : DNX_MC_EGRESS_LINK_PTR_END));

    if (is_egress) { /* calculate index of first group entry */
        cur_index = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, group_id);
    }
    /* get replications from the first entry */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_replications_from_entry(unit, cur_index,
      DNX_MCDS_TYPE_GET_START(group_type), &entries_remaining, group_size, &cur_index));

    /* get replications from the rest of the entries */
    while (cur_index != mcds->hw_end) {
        DNX_MC_ASSERT(cur_index > 0 && cur_index < mcds->ingress_bitmap_start);
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_replications_from_entry(unit, cur_index, entry_type, &entries_remaining, group_size, &cur_index));
        if (*group_size > max_group_size) { /* group is somehow bigger than allowed - internal error */
            DNX_MC_ASSERT(0);
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("group is too big")));
        }
    }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Remove the given replications from the given linked list MC group, by changing existing entries in the linked list.
 * Does not allocate MCDB entries, so does not fail due to the MCDB being too full.
 * Leaves non optimal linked list.
 * Does not change the MC group at once when needing to change multiple entries.
 * then remove the replication from the linked list. If it is the only
 * replication in the linked list, the entry is removed from the group.
 * Does not support the TDM format.
 */
uint32 jer2_qax_mcds_remove_replications_from_group(
    DNX_SAND_IN int                  unit,       /* device */
    DNX_SAND_IN dnx_mc_id_t          group_mcid, /* group mcid */
    DNX_SAND_IN uint32               nof_reps,   /* number of replications to remove */
    DNX_SAND_IN dnx_mc_replication_t *reps       /* input array containing replications to remove */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    const uint32 entry_index = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, group_mcid); /* table index of the first entry in the group */
    uint32 i, freed_index = entry_index; /* entry to be freed, the entry_index value means: can not free the entry */
    uint32 nof_dest_reps = 0, nof_bm_reps; /* number of replications to remove by type */
    jer2_qax_mcdb_entry_t *mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, entry_index);
    uint32 format = 0, next_index = entry_index, cur_index = 0, prev_index = entry_index, bitmap = -1;
    int is_outlif_only_replication = 0, is_outlif_cud_replication = 0, is_bitmap_cud_replication = 0;
    int remove_entry = 0, found = 0;
    uint16 entries_left = JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS; /* If we loop over more iterations this this it is certainly an error */
    const dnx_mc_replication_t *inp_rep = reps;
    dnx_mc_replication_t *rep, stack_reps[10], *reps_to_remove = stack_reps, *dest_reps, *bm_reps;
    DNXC_INIT_FUNC_DEFS;

    DNX_MC_ASSERT(DNX_MCDS_TYPE_VALUE_EGRESS_START == JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */
    if (nof_reps > 10) { /* if stack reps is too small for the input, allocate memory for input replications */
        reps_to_remove = NULL;
        DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &reps_to_remove, sizeof(*reps_to_remove) * nof_reps, "dnxreps2remove"));
    }

    /* copy input replications to reps_to_remove, dividing them  (into two groups) by replication type */
    bm_reps = (dest_reps = reps_to_remove) + nof_reps; /* Init the location to store replications by type */
    for (i = 0; i < nof_reps; ++i, ++inp_rep) {
        if (inp_rep->dest & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* bitmap + outlif replication */
            *--bm_reps = *inp_rep;
            bm_reps->dest &= ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT;
        } else {
            *(dest_reps++) = *inp_rep;
        }
    }
    nof_dest_reps = dest_reps - reps_to_remove;
    nof_bm_reps = nof_reps - nof_dest_reps;
    DNX_MC_ASSERT(dest_reps == bm_reps && nof_reps > nof_dest_reps);
    dest_reps = reps_to_remove;


    dest_reps->dest += nof_bm_reps; 
    rep = dest_reps;; 

    /* process the destination to search for and delete */
    if (rep->dest == _SHR_GPORT_INVALID) { /*outlif only, or port+outlif replication */
        is_outlif_only_replication = 1;
        if (rep->cud == DNX_MC_EGR_OUTLIF_DISABLED) { /* Invalid value, used to mark no replication in hardware */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid replication to delete")));
        }
    } else if (rep->dest & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* bitmap + outlif replication */
        bitmap = rep->dest & ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT;
        is_bitmap_cud_replication = 1;
        if (bitmap == DNX_MC_EGR_OUTLIF_DISABLED) { /* Invalid value, used to mark no replication in hardware */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid replication to delete")));
        }
    } else {
        is_outlif_cud_replication = 1;
        if (rep->dest == DNX_MULT_EGRESS_PORT_INVALID) { /* Invalid value, used to mark no replication in hardware */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid replication to delete")));
        }
    }

    for (; entries_left; --entries_left) {
        cur_index = next_index;

        switch (format = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, ENTRY_FORMATf)) {
          /* select memory format based on the format type */
          case 0:
            next_index = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, LINK_PTRf);
            if (is_outlif_cud_replication && soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, OUTLIF_1f) == rep->cud) {
                uint32 port = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, PP_DSP_1Af);
                if (rep->dest == port) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, PP_DSP_1Af, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                    if (soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, PP_DSP_1Bf) == JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID) {
                        remove_entry = 1;
                    }
                } else if (rep->dest < JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID && rep->dest == soc_mem_field32_get(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, PP_DSP_1Bf)) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, PP_DSP_1Bf, JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID); /* remove replication */
                    if (port == DNX_MULT_EGRESS_PORT_INVALID) {
                        remove_entry = 1;
                    }
                }
            }
            break;

          case 1:
            next_index = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, LINK_PTRf);
            if (is_bitmap_cud_replication && bitmap == soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, BMP_PTRf) && 
                rep->cud == soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, OUTLIF_1f)) {
                soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, BMP_PTRf, DNX_MC_EGR_BITMAP_DISABLED); /* remove replication */ 
                found = remove_entry = 1;
            }
            break;

            case 2:
            case 3:
              next_index = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, LINK_PTRf);
              if (is_outlif_only_replication) {
                  uint32 outlif1 = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, OUTLIF_1f);
                  uint32 outlif2 = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, OUTLIF_2f);
                  if (outlif1 == rep->cud) {
                      found = 1;
                      soc_mem_field32_set(unit, TAR_MCDBm, mcdb_entry, OUTLIF_1f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                      if (outlif2 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                          remove_entry = 1;
                      }
                  } else if (outlif2 == rep->cud) {
                      found = 1;
                      soc_mem_field32_set(unit, TAR_MCDBm, mcdb_entry, OUTLIF_2f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                      if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                          remove_entry = 1;
                      }
                  }
              }
              break;

          case 4:
          case 5:
            if (is_outlif_cud_replication) {
                uint32 port1 = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_1f);
                uint32 port2 = soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_2f);
                if (port1 == rep->dest && soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, OUTLIF_1f) == rep->cud) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_1f, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                    if (port2 == DNX_MULT_EGRESS_PORT_INVALID) { /* replication disabled for this entry part */
                        remove_entry = 1;
                    }
                } else if (port2 == rep->dest && soc_mem_field32_get(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, OUTLIF_2f) == rep->cud) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_2f, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                    if (port1 == DNX_MULT_EGRESS_PORT_INVALID) { /* replication disabled for this entry part */
                        remove_entry = 1;
                    }
                }
            }
            break;

          default: /* formats 6,7 */
            if (is_outlif_only_replication) {
                uint32 outlif1 = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, OUTLIF_1f);
                uint32 outlif2 = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, OUTLIF_2f);
                uint32 outlif3 = soc_mem_field32_get(unit, TAR_MCDBm, mcdb_entry, OUTLIF_3f);
                if (outlif1 == rep->cud) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDBm, mcdb_entry, OUTLIF_1f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                    if (outlif2 == DNX_MC_EGR_OUTLIF_DISABLED && outlif3 == DNX_MC_EGR_OUTLIF_DISABLED) { /* This is the only replication in the entry */
                        remove_entry = 1;
                    }
                } else if (outlif2 == rep->cud) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDBm, mcdb_entry, OUTLIF_2f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                    if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED && outlif3 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                        remove_entry = 1;
                    }
                } else if (outlif3 == rep->cud) {
                    found = 1;
                    soc_mem_field32_set(unit, TAR_MCDBm, mcdb_entry, OUTLIF_3f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                    if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED && outlif2 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                        remove_entry = 1;
                    }
                }
            }
        }

        /* get the next entry */
        if (format >= 4) {
            if (format & 1) {
                next_index = cur_index + 1;
                DNX_MC_ASSERT(next_index < mcds->ingress_bitmap_start);
            } else {
                next_index = DNX_MC_EGRESS_LINK_PTR_END;
            }
        }
        if (found) {
            break;
        }

        if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* The replication was not found till the end of the group */
          DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("replication not found in group")));
        }
        prev_index = cur_index;
        mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, next_index);
        DNX_MC_ASSERT(DNX_MCDS_TYPE_VALUE_EGRESS == JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */
    }

    if (!found) {
        DNX_MC_ASSERT(0);
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("The replication was not found till the legal size of a group")));
    }

    if (remove_entry) { /* If we want to remove the entry since no replication was left in it */
        int try_to_copy_the_next_entry = 0;
        if (cur_index == entry_index) { /* the first entry in the group needs to be removed */
            DNX_MC_ASSERT(prev_index == entry_index);
            if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* this is the only entry in the group, just update it */
                mcdb_entry->word0 = mcds->free_value.word0;
                mcdb_entry->word1 = mcds->free_value.word1;
                mcdb_entry->word2 &= ~JER2_QAX_MC_ENTRY_MASK_VAL;
                mcdb_entry->word2 |= mcds->free_value.word2;
                jer2_qax_self_replication_set(unit, group_mcid , FALSE);
            } else  { /* The removed entry points to a second entry */
                try_to_copy_the_next_entry = 1;
            }
        } else { /* the replication entry is not the start of the group, it will be removed */
            jer2_qax_mcdb_entry_t *prev_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, prev_index);
            uint32 prev_format = soc_mem_field32_get(unit, TAR_MCDBm, prev_entry, ENTRY_FORMATf); 
            DNX_MC_ASSERT(prev_index != cur_index);
            if (prev_format < 4) {
                /* We will remove the entry by having the previous entry point to the next entry */
                DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
                  mcds, unit, prev_index, DNX_MCDS_TYPE_VALUE_EGRESS, next_index));
                freed_index = cur_index;
            } else if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* We can remove the last entry even if there is no pointer to it */
                DNX_MC_ASSERT(prev_format & 1);
                soc_mem_field32_set(unit, TAR_MCDBm, prev_entry, ENTRY_FORMATf, prev_format & ~1); /* mark previous entry as end of group */ 
                DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, prev_index));
                freed_index = cur_index;
            } else {
                try_to_copy_the_next_entry = 1;
            }
        }

        if (try_to_copy_the_next_entry) { /* If the next entry is the end of the group or has a pointer, we can copy it on top of this entry */
            jer2_qax_mcdb_entry_t *next_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, next_index);
            uint32 next_format = soc_mem_field32_get(unit, TAR_MCDBm, next_entry, ENTRY_FORMATf); 
            DNX_MC_ASSERT(JER2_QAX_MCDS_ENTRY_GET_TYPE(next_entry) == DNX_MCDS_TYPE_VALUE_EGRESS);
            if (next_format < 4 || !(next_format & 1)) { /* If the next entry has a pointer or is the last entry in the group */
                mcdb_entry->word0 = next_entry->word0;
                mcdb_entry->word1 &= ~JER2_QAX_MC_ENTRY_MASK_VAL;
                mcdb_entry->word1 |= (next_entry->word1 & JER2_QAX_MC_ENTRY_MASK_VAL);
                freed_index = next_index;
                DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds,unit, next_index, DNX_MCDS_TYPE_VALUE_EGRESS, &next_index)); /* get entry to update the back_pointer in */
            }
        }
    }

    if (freed_index != cur_index || !remove_entry) { /* if not freeing the current entry, it needs to be updated in hardware */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, cur_index)); /* write group start entry to hardware */
    }

    if (freed_index != entry_index) { /* if an entry was removed, free it */
        if (next_index != DNX_MC_EGRESS_LINK_PTR_END) {
            DNX_MC_ASSERT(JER2_QAX_MCDS_GET_TYPE(mcds, next_index) == DNX_MCDS_TYPE_VALUE_EGRESS);
            JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, next_index, prev_index); /* make entry next_index point back to prev_index */
        }
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks( /* free the entry freed_index */
          unit, mcds, freed_index, freed_index, jer2_qax_mcds_get_region(mcds, freed_index), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }

exit:
    if (reps_to_remove != stack_reps) {
        dnxc_free_mem_if_not_null(unit, (void*)&reps_to_remove);
    }
    DNXC_FUNC_RETURN;
}

/*
 * This functions removes the given replications from the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 */
uint32 jer2_qax_mult_remove_replications(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32               nof_reps,   /* number of replications to remove */
    DNX_SAND_IN  dnx_mc_replication_t *reps       /* input array containing replications to remove */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_rep_data_t *rep = mcds->reps, stack_reps[10], *reps_to_remove = stack_reps, *to_remove;
    uint16 size_left = 0;
    uint32 i, dest;
    DNXC_INIT_FUNC_DEFS;

    mcds->out_err = _SHR_E_NONE;
    if (nof_reps == 0) {
        SOC_EXIT;
    } else if (nof_reps > 10) { /* if stack reps is too small for the input, allocate memory for input replications */
        reps_to_remove = NULL;
        DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &reps_to_remove, sizeof(*reps_to_remove) * nof_reps, "reps2remove"));
    }
    for (i = 0, to_remove = reps_to_remove; i < nof_reps; ++i, ++to_remove) { /* copy input replications to reps_to_remove */
        COMPILER_64_ZERO(*to_remove);
        dest = reps->dest;
        if (reps->dest & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* bitmap + outlif replication */
            dest &= ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT;
            JER2_QAX_MCDS_REP_DATA_SET_TYPE(to_remove, JER2_QAX_MCDS_REP_TYPE_BM);
        }
        JER2_QAX_MCDS_REP_DATA_SET_DEST(to_remove, dest);
        JER2_QAX_MCDS_REP_DATA_SET_CUD1(to_remove, reps->cud);
        JER2_QAX_MCDS_REP_DATA_SET_CUD2(to_remove, reps->additional_cud);
        ++reps;
    }
    to_remove = reps_to_remove;
        
        
    if (nof_reps > 1) { /* sort the input replications and the mcds replications */
        dnx_sand_os_qsort(reps_to_remove, nof_reps, sizeof(*reps_to_remove), jer2_qax_rep_data_t_compare); /* sort input replications to remove */
        dnx_sand_os_qsort(rep, mcds->nof_reps, sizeof(*rep), jer2_qax_rep_data_t_compare); /* sort mcds replications to remove from */
        

    } else { /* find and remove the single replication */
        for (size_left = mcds->nof_reps; size_left; --size_left, ++rep) {
            if (JER2_QAX_EQ_REP_DATA(rep, to_remove)) {
                JER2_QAX_MCDS_REP_DATA_GET_TYPE(to_remove, i);
                *rep = mcds->reps[--mcds->nof_reps];
                --*(i == JER2_QAX_MCDS_REP_TYPE_DEST ? &mcds->nof_dest_cud_reps : &mcds->nof_bitmap_reps);
                SOC_EXIT;
            }
        }
        mcds->out_err = _SHR_E_NOT_FOUND;
    }

exit:
    if (reps_to_remove != stack_reps) {
        dnxc_free_mem_if_not_null(unit, (void*)&reps_to_remove);
    }
    DNXC_FUNC_RETURN;
}


/*********************************************************************
* Initialize MC replication database
* The initialization accesses the replication table as if it was an
* Ingress replication, for all entries (including Egress MC)
**********************************************************************/
uint32 jer2_qax_mult_rplct_tbl_entry_unoccupied_set_all(
    DNX_SAND_IN  int unit
)
{
  const jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
  DNXC_INIT_FUNC_DEFS;
  DNXC_PCID_LITE_SKIP(unit);

#ifdef PLISIM
  if (!SAL_BOOT_PLISIM) /* do not init MCDB hardware in simulation, later instead of reading it into the mcds, use the init values */
#endif
  {
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, TAR_MCDBm, MEM_BLOCK_ANY, &mcds->free_value));
  }

exit:
  DNXC_FUNC_RETURN;
}



/*
 * Egress functions
 */


/* Check if the given egress group is created=open, will return 1 if the group is marked as open, or 0 */
static int jer2_qax_egress_group_open_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 group_id, /* multicast ID */
    DNX_SAND_OUT uint8 *is_open
)
{
    DNXC_INIT_FUNC_DEFS
    DNX_MC_ASSERT(group_id < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
    DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(unit, group_id, is_open));
exit:
    DNXC_FUNC_RETURN
}


/*
 * Gets the egress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact group size.
 * The group's replications number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 jer2_qax_mult_eg_get_group(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t    group_mcid,           /* group id */
    DNX_SAND_IN  uint32         mc_group_size,        /* maximum replications to return */
    DNX_SAND_OUT soc_gport_t    *ports,               /* output logical ports (array of size mc_group_size) used if !reps */
    DNX_SAND_OUT soc_if_t       *cuds,                /* output CUDs (array of size mc_group_size) used if !reps */
    DNX_SAND_OUT soc_multicast_replication_t *reps,   /* output replication array (array of size mc_group_size*/
    DNX_SAND_OUT uint32         *exact_mc_group_size, /* the number of replications in the group will be returned here */
    DNX_SAND_OUT uint8          *is_open              /* will return if the group is open (false or true) */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(exact_mc_group_size);
    DNXC_NULL_CHECK(is_open);
    if (mc_group_size && !reps && (!ports || !cuds)) { /* we check that the output data pointers are not null if we need to return data */
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("NULL pointer")));
    }

    DNXC_IF_ERR_EXIT(jer2_qax_mult_does_group_exist(unit, group_mcid, TRUE, is_open)) ; /* check if the group is open */

    if (*is_open) {
        uint16 group_size;
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_group(
            unit, TRUE, group_mcid, DNX_MCDS_TYPE_VALUE_EGRESS, mc_group_size, &group_size));
        *exact_mc_group_size = group_size;
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_replications_to_arrays(unit, 1, mc_group_size,  ports, cuds, reps));
    } else { /* group is not open */
        *exact_mc_group_size = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}


#if 0 /* JER2_QAX implementation */
/*
 * Read a bitmap from the egress MC bitmap table.
 */
uint32 jer2_qax_egq_vlan_table_tbl_get(
    DNX_SAND_IN   int    unit,
    DNX_SAND_IN   uint32 entry_offset,
    DNX_SAND_OUT  uint32 *vlan_table_tbl_data
)
{
    int core, nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    uint32 *bitmap_ptr = vlan_table_tbl_data;
    DNXC_INIT_FUNC_DEFS;

    for (core = 0; core < nof_active_cores; ++core) {
        if (!SOC_IS_QAX(unit)) {  
            DNXC_IF_ERR_EXIT(READ_EGQ_VLAN_TABLEm(unit, EGQ_BLOCK(unit, core), entry_offset, bitmap_ptr));
            bitmap_ptr += DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32;
        }
    }
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write a bitmap to the egress MC bitmap table.
 */
uint32 jer2_qax_egq_vlan_table_tbl_set(
    DNX_SAND_IN   int     unit,
    DNX_SAND_IN   uint32  entry_offset,
    DNX_SAND_IN   uint32* vlan_table_tbl_data
)
{
    int core, i, nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dnx_mc_core_bitmap_t non_zero_cores = DNX_MC_CORE_BITAMAP_NO_CORES; /* The cores whose ports have replications */
    dnx_mc_core_bitmap_t prev_cores_with_reps, new_cores_with_reps; /* The cores whose ports previously had/now have replications */
    const uint32 *bitmap_ptr = vlan_table_tbl_data;
    DNXC_INIT_FUNC_DEFS;

    for (core = 0; core < nof_active_cores; ++core) { /* find which cores have replications */
        for (i = 0; i < DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32; ++i) {
            if (bitmap_ptr[i]) {
                non_zero_cores |= DNX_MC_CORE_BITAMAP_CORE_0 << core;
                break;
            }
        }
        bitmap_ptr += DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32;
    }

    DNXC_IF_ERR_EXIT(dnx_egress_group_get_core_replication(unit, entry_offset, &prev_cores_with_reps)); /* get egress cores replicated to */
    new_cores_with_reps = non_zero_cores & prev_cores_with_reps; /* zero cores who lost all their port replications */
    if (new_cores_with_reps != prev_cores_with_reps) { /* Do we have to disable replication to egress cores? */
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, entry_offset, new_cores_with_reps));
        prev_cores_with_reps = new_cores_with_reps;
    }

    /* Set the new bitmaps */
    bitmap_ptr = vlan_table_tbl_data;
    for (core = 0; core < nof_active_cores; ++core) {
        if (!SOC_IS_QAX(unit)) {  
            DNXC_IF_ERR_EXIT(WRITE_EGQ_VLAN_TABLEm(unit, EGQ_BLOCK(unit, core), entry_offset, (uint32*)bitmap_ptr));
            bitmap_ptr += DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32;
        }
    }

    if (non_zero_cores != prev_cores_with_reps) { /* Do we have to enable replication to egress cores? */
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, entry_offset, non_zero_cores));
    }
exit:
    DNXC_FUNC_RETURN;
}



/*
 * Gets the egress replications (ports) of the given bitmap.
 * If the bitmap is a vlan egress group, it does not have to be open/created.
 */
uint32 jer2_qax_mult_eg_bitmap_group_get(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  dnx_mc_id_t                           bitmap_id, /* ID of the bitmap */
    DNX_SAND_OUT DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *group     /* output TM port bitmap */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(group);
    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, bitmap_id, &group->bitmap[0]));
exit:
    DNXC_FUNC_RETURN;
}


/* Exchange the contents of two jer2_qax_mcds_t structs */
static INLINE void dnx_exchange_jer2_qax_rep_data_t(jer2_qax_rep_data_t *a, jer2_qax_rep_data_t *b)
{
    jer2_qax_rep_data_t temp = *a;
    *a = *b;
    *b = temp;
}
#endif /* 0 JER2_QAX implementation */


/*
 * Initialize the mcds fields used internally inside an API to write a linked list.
 * This function needs to be called once before calling jer2_qax_*_mc_write_entry_*().
 * Must be called as the first thing in the function to always have mcds->state initialized properly.
 */

void jer2_qax_mc_start_linked_list(
    DNX_SAND_IN int                           unit,
    DNX_SAND_IN uint32                        in_block_start, /* start index of an allocated block to use for the free list, relevant only if in_block_size>0 */
    DNX_SAND_IN dnx_free_entries_block_size_t in_block_size,  /* size of the allocated block to use, should be 0 if none. If 0 then we are setting the whole linked list*/
    DNX_SAND_IN uint32                        start,          /* The group id or entry that the created linked list will start in, (one entry before the start if in_block_size!=0 */
    DNX_SAND_IN uint32                        end,            /* The entry that end of the created linked list will point to, (marks end of list if in_block_size==0 */
    DNX_SAND_IN uint8                         list_type       /* specifies type of list and if it is a group start to be filled or not */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_NORMAL(list_type) && !in_block_size == DNX_MCDS_TYPE_IS_START(list_type)); /* assumes IS_START returns 0/1 */

    mcds->out_err = _SHR_E_NONE; /* init returned error to none */
    if (in_block_size == 0) { /* Is the created linked list a while MC group? */
        mcds->is_group_start_to_be_filled = mcds->writing_full_list = TRUE;
        mcds->start_entry = mcds->free_value;
    } else { /* We are writing a partial linked list, not starting at the start of the group */
        mcds->is_group_start_to_be_filled = mcds->writing_full_list = FALSE;
    }

    mcds->linked_list_end = JER2_QAX_NO_MCDB_INDEX; /* empty created linked list so far */
    mcds->linked_list = mcds->list_end = end;
    mcds->list_start = list_type == DNX_MCDS_TYPE_VALUE_EGRESS_START ? JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, start) : start;
    mcds->nof_possible_reps = 0; /* number of possible replications in the linked list starts at zero */
    mcds->block_start = in_block_start; /* store the input block size, relevant only if in_block_size>0 */
    mcds->block_size = in_block_size;
    mcds->state = JER2_QAX_MCDS_STATE_INITED;
}


/*
 * Finish writing the linked list. Either:
 * Write the first entry of the linked list, making it take effect, and free the previous linked list
 * In case of previous failure, free the entries of the linked list written so far.
 */

uint32 jer2_qax_mc_end_linked_list(
    DNX_SAND_IN int  unit
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, mcds->list_start);
    jer2_qax_mcdb_entry_t old_start_entry = *mcdb_entry; /* backup copy of the list_start entry */

    DNXC_INIT_FUNC_DEFS;
    /* the current linked list state is: list_start x-x linked_list <-> created entries <-> linked_list_end x-> list_end */

    /* now connect the written linked list to its preceding and succeeding (if any) entries */
    if (mcds->writing_full_list) { /* the linked list written is the whole MC group */
        DNX_MC_ASSERT(mcds->list_end == mcds->hw_end);
        /* build first group entry for writing after building all the linked list */
        mcdb_entry->word0 = mcds->start_entry.word0; /* copy previously filled hardware fields (next pinter not set) */
        mcdb_entry->word1 = mcds->start_entry.word1;
        mcdb_entry->word2 &= JER2_QAX_MCDS_LAST_WORD_KEEP_BITS_MASK;
        /* coverity[uninit_use:FALSE] */
        mcdb_entry->word2 |= mcds->start_entry.word2;

        JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, mcds->list_start, mcds->list_start); /* set software link to previous entry */
        JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, mcds->group_type_start); /* set type */
    }
    /* set the (hardware) forward link from the previous entry of the linked list (before the created segment) , making the changes take effect */
    soc_mem_field32_set(unit, mcds->group_type == DNX_MCDS_TYPE_VALUE_INGRESS ? TAR_MCDB_SINGLE_REPLICATIONm : TAR_MCDB_EGRESS_TDM_FORMATm,
      mcdb_entry, LINK_PTRf, mcds->linked_list); /* same field location for all ingress/egress formats */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, mcds->list_start)); /* write to hardware, activating the new linked list */
 
    if (mcds->linked_list != mcds->list_end) {
        /* set the (software) backward link from the first entry of the created linked list (excluding group start entry) to the preceding entry */
        JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, mcds->linked_list, mcds->list_start);
        if (mcds->list_end != mcds->hw_end) { /* if group continues, (software) back-link the continuation to the created linked list */
            /* set the (software) backward link to the current entry from the next one in the list, which is the previous added entry */
            JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, mcds->list_end, mcds->linked_list_end);
        }
    } else {
        DNX_MC_ASSERT(mcds->writing_full_list && mcds->linked_list_end == JER2_QAX_NO_MCDB_INDEX);
    }
    mcds->state = JER2_QAX_MCDS_STATE_FINISHED;
exit:
    if (mcds->state != JER2_QAX_MCDS_STATE_FINISHED) { /* cleanup on list_start write failure */
        *mcdb_entry = old_start_entry;
    }
    DNXC_FUNC_RETURN;
}

/*
 * Get a new entry for the linked list being created.
 * If this is the first entry in a linked list, it is not written to mcds->mcdb yet.
 * Handles free block allocation.
 * The new entry is set in mcds->cur_entry_index and in *mcdb_entry = mcds->cur_entry.
 */
static uint32 jer2_qax_mc_get_linked_list_entry(
    DNX_SAND_IN  int              unit,
    DNX_SAND_OUT jer2_qax_mcdb_entry_t **mcdb_entry /* output: The entry structure to set */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    DNXC_INIT_FUNC_DEFS;

    if (mcds->is_group_start_to_be_filled) { /* Do we need to fill (and not yet write) the first entry of a MC group? */
        DNX_MC_ASSERT(mcds->writing_full_list && mcds->block_size == 0);
        mcds->cur_entry_index = mcds->list_start;
        *mcdb_entry = mcds->cur_entry = &mcds->start_entry;
        mcds->is_group_start_to_be_filled = FALSE;

    } else { /* we will use a non group start entry from a free entries block */

        if (mcds->block_size == 0) { /* no available MCDB entries, we need to allocate */
            
            DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_free_entries_block(mcds, mcds->alloc_flags, /* allocate free entries */
              1, 1 , &mcds->block_start, &mcds->block_size));
            if (!mcds->block_size) { /* could not get free entries */
                mcds->out_err = _SHR_E_FULL;
                SOC_EXIT; /* will free the linked list allocated so far */
            }
            DNX_MC_ASSERT(mcds->block_size == 1);
        }
        mcds->cur_entry_index = mcds->block_start;
        *mcdb_entry = mcds->cur_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, mcds->block_start);
        --mcds->block_size;
        ++mcds->block_start;
    }
    DNX_MC_ASSERT(mcds->cur_entry_index < mcds->ingress_bitmap_start);
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Set a new entry for the linked list being created.
 * If this is the first entry in a linked list, it is not written to mcds->mcdb yet.
 * Handles linking to other enrties.
 * jer2_qax_mc_get_linked_list_entry() must be called before this function, which sets
 * mcds->cur_entry_index and mcds->cur_entry to the entry to be set.
 */
static uint32 jer2_qax_mc_set_linked_list_entry(
    DNX_SAND_IN int  unit
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    DNXC_INIT_FUNC_DEFS;

    if (mcds->cur_entry_index == mcds->list_start) { /* Is this the first entry of the MC group */
        DNX_MC_ASSERT(mcds->writing_full_list && mcds->block_size == 0 && mcds->linked_list == mcds->list_end && mcds->linked_list_end == JER2_QAX_NO_MCDB_INDEX && mcds->state == JER2_QAX_MCDS_STATE_INITED);
        JER2_QAX_MCDS_ENTRY_SET_TYPE(mcds->cur_entry, mcds->group_type_start); /* Set entry type */
    } else { /* This is not the first entry of the MC group linked list */
        DNX_MC_ASSERT(mcds->cur_entry == JER2_QAX_GET_MCDB_ENTRY(mcds, mcds->cur_entry_index) && mcds->cur_entry_index + 1 == mcds->block_start);

        if (mcds->linked_list_end == JER2_QAX_NO_MCDB_INDEX) { /* Is this the last entry of the linked list (which is written first)? */
            DNX_MC_ASSERT(mcds->list_start != mcds->cur_entry_index && mcds->linked_list == mcds->list_end && mcds->state == JER2_QAX_MCDS_STATE_INITED);
            mcds->linked_list_end = mcds->cur_entry_index; /* Store the last entry of the linked list */
            mcds->state = JER2_QAX_MCDS_STATE_STARTED;
        } else { /* a previous entry was written */
            DNX_MC_ASSERT(mcds->linked_list != mcds->list_end && mcds->linked_list_end != JER2_QAX_NO_MCDB_INDEX && mcds->state == JER2_QAX_MCDS_STATE_STARTED);
            /* set the (software) backward link to the current entry from the next one in the list, which is the previous added entry */
            JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, mcds->linked_list, mcds->cur_entry_index);
        }
        /* set the (hardware) forward link from the current entry to the next one in the list, which is the previous added entry */
        soc_mem_field32_set(unit, mcds->group_type == DNX_MCDS_TYPE_VALUE_INGRESS ? TAR_MCDB_SINGLE_REPLICATIONm : TAR_MCDB_EGRESS_TDM_FORMATm,
            mcds->cur_entry, LINK_PTRf, mcds->linked_list); /* same field location for all ingress/egress formats */

        mcds->linked_list = mcds->cur_entry_index; /* update the start of the linked list created so far */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, mcds->cur_entry_index)); /* write the entry to hardware */
        JER2_QAX_MCDS_ENTRY_SET_TYPE(mcds->cur_entry, mcds->group_type); /* Set entry type */

    }
exit:
    DNXC_FUNC_RETURN;
}


/*
 * create an ingress linked list block of entries without a link pointer, possibly except the last entry in the block.
 * The first entry of the group must already be handled before calling this function.
 * Handles free block allocation.
 * May write a smaller block than requested if the given/allocated free block is smaller.
 * This function handles internally what jer2_qax_mc_{g,s}et_linked_list_entry() do.
 */
static uint32 jer2_qax_ing_no_ptr_block(
    DNX_SAND_IN    int            unit,
    DNX_SAND_INOUT jer2_qax_rep_data_t **rep,         /* The replication for the last entry containing a link pointer, if one is needed. */
                                                 /* If it is needed, *--rep is used and *rep is decreased by one. */
    DNX_SAND_INOUT jer2_qax_rep_data_t **couples,     /* The replications of the entries with no link pointer, the input pointer is updated */
    DNX_SAND_INOUT int            *couples_left, /* The amount of possible replication couples left for no pointer entries, updated by the function */
    DNX_SAND_INOUT int            *nof_entries   /* input: The wanted block size (including pointer entry) output: actual size */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry;
    uint32 cur_entry_index, block_end, dest, cud1, cud2, rep_type;
    int couples_used, entries_left; /* number of entries left to be written in the block */
    soc_mem_t mcdb_bmp_repl_mem;
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(mcds->is_group_start_to_be_filled == 0 && *nof_entries && *couples_left && *couples_left && *nof_entries > 0 && *nof_entries <= DNX_MCDS_MAX_FREE_BLOCK_SIZE);
    mcdb_bmp_repl_mem = TAR_MCDB_BITMAP_REPLICATION_ENTRYm;

    if (mcds->block_size <= 0) { /* no previously allocated MCDB entries, we need to allocate */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_free_entries_block(mcds, DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL,
          *nof_entries, DNX_MCDS_MAX_FREE_BLOCK_SIZE, &mcds->block_start, &mcds->block_size)); /* allocate free entries */
        if (!mcds->block_size) { /* could not get free entries */
            mcds->out_err = _SHR_E_FULL;
            SOC_EXIT; /* will free the linked list allocated so far */
        }
        DNX_MC_ASSERT(mcds->block_size > 0);
    }
    if (mcds->block_size >= *nof_entries) {
        entries_left = *nof_entries;
    } else {
        *nof_entries = entries_left = mcds->block_size;
    }
    couples_used = *nof_entries;

    cur_entry_index = block_end = mcds->block_start + (entries_left - 1);
    mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, cur_entry_index);

    /* Create entries block, starting with its last entry */
    if (mcds->linked_list != JER2_QAX_MC_INGRESS_LINK_PTR_END) { /* is a pointer entry needed at the end of the block */
        DNX_MC_ASSERT(mcds->linked_list_end != JER2_QAX_NO_MCDB_INDEX && *couples_left + 1 >= entries_left);
        /* create pointer entry , write format to the jer2_qax_mcdb_entry_t entry */
        --*rep;
        JER2_QAX_MCDS_REP_DATA_GET_FIELDS(*rep, dest, cud1, cud2, rep_type);
        jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);
        if (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST) {
            soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, CUD_0f, cud1); /* set the 1st CUD */
            soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, CUD_1f, cud2); /* set the 2nd CUD */
            soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, DESTINATIONf, dest); /* set the destination */
            soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, FORMATf, 1); /* set the entry format */
        } else {
            soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, CUD_0f, cud1); /* set the 1st CUD */
            soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, CUD_1f, cud2); /* set the 2nd CUD */
            soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, BMP_PTRf, /* set the bitmap pointer */
              mcds->ingress_bitmap_start + dest * SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmap_bytes);
            soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, FORMATf, 0); /* set the entry format */
        }
        /* set the (hardware) forward link from the current entry to the next one in the list, which is the previous added entry */
        soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, LINK_PTRf, mcds->linked_list); /* same field location for all ingress pointer formats */
        --couples_used;
    } else {
        DNX_MC_ASSERT(*couples_left >= entries_left);
        JER2_QAX_MCDS_REP_DATA_GET_FIELDS(*couples, dest, cud1, cud2, rep_type);
        ++*couples;
        DNX_MC_ASSERT (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST && cud2 == DNX_MC_NO_2ND_CUD);
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, CUD_0f, cud1); /* set the 1st CUD */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, DESTINATION_0f, dest); /* set the destination */
        JER2_QAX_MCDS_REP_DATA_GET_FIELDS(*couples, dest, cud1, cud2, rep_type);
        ++*couples;
        DNX_MC_ASSERT (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST && cud2 == DNX_MC_NO_2ND_CUD);
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, CUD_1f, cud1); /* set the 2nd CUD */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, DESTINATION_1f, dest); /* set the destination */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, LASTf, 1); /* set the entry format */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, FORMATf, 1); /* set the entry format */
    }
    JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS); /* Set entry type */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, cur_entry_index)); /* write the entry to hardware */

    for (--entries_left; entries_left > 0; --entries_left) {
        --cur_entry_index;
        JER2_QAX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, cur_entry_index); /* set back link from the previous entry written in the block */
        --mcdb_entry;
        JER2_QAX_MCDS_REP_DATA_GET_FIELDS(*couples, dest, cud1, cud2, rep_type);
        ++*couples;
        DNX_MC_ASSERT (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST && cud2 == DNX_MC_NO_2ND_CUD);
        jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, CUD_0f, cud1); /* set the 1st CUD */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, DESTINATION_0f, dest); /* set the destination */
        JER2_QAX_MCDS_REP_DATA_GET_FIELDS(*couples, dest, cud1, cud2, rep_type);
        ++*couples;
        DNX_MC_ASSERT (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST && cud2 == DNX_MC_NO_2ND_CUD);
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, CUD_1f, cud1); /* set the 2nd CUD */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, DESTINATION_1f, dest); /* set the destination */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, LASTf, 0); /* set the entry format */
        soc_mem_field32_set(unit, TAR_MCDB_DOUBLE_REPLICATIONm, mcdb_entry, FORMATf, 1); /* set the entry format */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_write_entry(unit, cur_entry_index)); /* write the entry to hardware */
        JER2_QAX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS); /* Set entry type */

    }

    if (mcds->linked_list_end == JER2_QAX_NO_MCDB_INDEX) { /* Is this block the end of the linked list (which is written first)? */
        DNX_MC_ASSERT(mcds->linked_list == mcds->list_end && mcds->state == JER2_QAX_MCDS_STATE_INITED);
        mcds->linked_list_end = block_end; /* Store the last entry of the linked list */
        mcds->state = JER2_QAX_MCDS_STATE_STARTED;
    } else { /* a previous entry was written */
        DNX_MC_ASSERT(mcds->linked_list != mcds->list_end && mcds->linked_list_end != JER2_QAX_NO_MCDB_INDEX && mcds->state == JER2_QAX_MCDS_STATE_STARTED);
        /* set the (software) backward link to the current entry from the next one in the list, which is the previous added entry */
        JER2_QAX_MCDS_SET_PREV_ENTRY(mcds, mcds->linked_list, block_end);
    }
    mcds->linked_list = cur_entry_index; /* update the start of the linked list created so far */

    DNX_MC_ASSERT(mcds->block_size >= *nof_entries && cur_entry_index == mcds->block_start && mcdb_entry == JER2_QAX_GET_MCDB_ENTRY(mcds, cur_entry_index));
    mcds->block_size -= *nof_entries;
    mcds->block_start += *nof_entries;
    *couples_left -= couples_used;
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Write an MCDB entry of the egress MC format supporting two replications, each with a destination port and a single CUD.
 * If this is the first entry in the group, it is not written to the MCDB, and kept to be written with the linked list is finished.
 * We assume that jer2_qax_mc_start_linked_list() was called once for the linked list being created.
 * We assume that jer2_qax_mc_end_linked_list() will be called once after all the linked list entries are written.
 *
 * The linked list entries are written from the last to the first.
 * So this function writes the pointer to the next entry in the list, and writes the software pointer
 * to the previous entry of the next entry in the linked list which was previously written.
 */

uint32 jer2_qax_eg_mc_write_entry_2ports_cud(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 port1, /* The destination PP port of the 1st replication */
    DNX_SAND_IN  uint32 cud1,  /* The 1st CUD of the 1st replication */
    DNX_SAND_IN  uint32 port2, /* The destination PP port of the 2nd replication */
    DNX_SAND_IN  uint32 cud2   /* The 1st CUD of the 2nd replication */
)
{
    jer2_qax_mcdb_entry_t *mcdb_entry;
    DNXC_INIT_FUNC_DEFS;

    /* Get entry/first group entry to write to, handle free block allocation */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_get_linked_list_entry(unit, &mcdb_entry));
    jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);

    /* write format to the jer2_qax_mcdb_entry_t entry */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, OUTLIF_Af, cud1); /* set the CUD of the 1st replication */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, OUTLIF_Bf, cud2); /* set the CUD of the 2nd replication */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, PP_DSP_Af, port1); /* set the destination port of the 1st replication */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, PP_DSP_Bf, port2); /* set the destination port of the 2nd replication */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TWO_COPIES_FORMATm, mcdb_entry, ENTRY_FORMATf, 1); /* set the entry format */

    /* write backward link to previous start of linked list; update linked list state */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_set_linked_list_entry(unit));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write an MCDB entry of the egress MC format supporting four replications, each with a destination port and a single CUD.
 * The CUD is shared between all the replications.
 * If this is the first entry in the group, it is not written to the MCDB, and kept to be written with the linked list is finished.
 * We assume that jer2_qax_mc_start_linked_list() was called once for the linked list being created.
 * We assume that jer2_qax_mc_end_linked_list() will be called once after all the linked list entries are written.
 *
 * The linked list entries are written from the last to the first.
 * So this function writes the pointer to the next entry in the list, and writes the software pointer
 * to the previous entry of the next entry in the linked list which was previously written.
 */

uint32 jer2_qax_eg_mc_write_entry_4ports_cud(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 port1, /* The destination PP port of the 1st replication */
    DNX_SAND_IN  uint32 port2, /* The destination PP port of the 2nd replication */
    DNX_SAND_IN  uint32 port3, /* The destination PP port of the 3rd replication */
    DNX_SAND_IN  uint32 port4, /* The destination PP port of the 4th replication */
    DNX_SAND_IN  uint32 cud   /* The 1st CUD of all the replications */
)
{
    jer2_qax_mcdb_entry_t *mcdb_entry;
    DNXC_INIT_FUNC_DEFS;

    /* Get entry/first group entry to write to, handle free block allocation */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_get_linked_list_entry(unit, &mcdb_entry));
    jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);

    /* write format to the jer2_qax_mcdb_entry_t entry */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, OUTLIFf, cud); /* set the joint 1st CUD */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_1f, port1); /* set the four destination ports */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_2f, port2);
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_3f, port3);
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_4f, port4);
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, ENTRY_FORMATf, 5); /* set the entry format */

    /* write backward link to previous start of linked list; update linked list state */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_set_linked_list_entry(unit));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write an MCDB entry of the egress MC format supporting two replications, each with a destination port and a two CUDs.
 * Both the 1st and the 2nd CUDs are shared between the two replications.
 * If this is the first entry in the group, it is not written to the MCDB, and kept to be written with the linked list is finished.
 * We assume that jer2_qax_mc_start_linked_list() was called once for the linked list being created.
 * We assume that jer2_qax_mc_end_linked_list() will be called once after all the linked list entries are written.
 *
 * The linked list entries are written from the last to the first.
 * So this function writes the pointer to the next entry in the list, and writes the software pointer
 * to the previous entry of the next entry in the linked list which was previously written.
 */

uint32 jer2_qax_eg_mc_write_entry_2ports_2cuds(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 port1, /* The destination PP port of the 1st replication */
    DNX_SAND_IN  uint32 port2, /* The destination PP port of the 2nd replication */
    DNX_SAND_IN  uint32 cud1,  /* The 1st CUD of both replications */
    DNX_SAND_IN  uint32 cud2   /* The 2nd CUD of both replications */
)
{
    jer2_qax_mcdb_entry_t *mcdb_entry;
    DNXC_INIT_FUNC_DEFS;

    /* Get entry/first group entry to write to, handle free block allocation */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_get_linked_list_entry(unit, &mcdb_entry));
    jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);

    /* write format to the jer2_qax_mcdb_entry_t entry */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, OUTLIF_1f, cud1); /* set the 1st CUD of both replications */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, OUTLIF_2f, cud2); /* set the 2nd CUD of both replications */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_Af, port1); /* set the destination port of the 1st replication */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, PP_DSP_Bf, port2); /* set the destination port of the 2nd replication */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm, mcdb_entry, ENTRY_FORMATf, 2); /* set the entry format */

    /* write backward link to previous start of linked list; update linked list state */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_set_linked_list_entry(unit));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write an MCDB entry of the egress MC format supporting two replications, each with a destination port and a single CUD.
 * If this is the first entry in the group, it is not written to the MCDB, and kept to be written with the linked list is finished.
 * We assume that jer2_qax_mc_start_linked_list() was called once for the linked list being created.
 * We assume that jer2_qax_mc_end_linked_list() will be called once after all the linked list entries are written.
 *
 * The linked list entries are written from the last to the first.
 * So this function writes the pointer to the next entry in the list, and writes the software pointer
 * to the previous entry of the next entry in the linked list which was previously written.
 */

uint32 jer2_qax_eg_mc_write_entry_bitmap_cud(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 bitmap_id, /* The destination bitmap ID of the replication */
    DNX_SAND_IN  uint32 cud1       /* The 1st CUD of the bitmap replication (for all replications of bitmap ports) */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry;
    DNXC_INIT_FUNC_DEFS;

    /* Get entry/first group entry to write to, handle free block allocation */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_get_linked_list_entry(unit, &mcdb_entry));
    jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);

    /* write format to the jer2_qax_mcdb_entry_t entry */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, OUTLIF_1f, cud1); /* set the CUD */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, BMP_PTRf,
      mcds->egress_bitmap_start + bitmap_id * SOC_DNX_CONFIG(unit)->jer2_qax->nof_egress_bitmap_bytes); /* set the bitmap pointer */
    soc_mem_field32_set(unit, TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm, mcdb_entry, ENTRY_FORMATf, 0); /* set the entry format */

    /* write backward link to previous start of linked list; update linked list state */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_set_linked_list_entry(unit));
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Write an MCDB entry of an ingress MC format containing a link pointer.
 * The MCDB bits of the entry are first cleared.
 * The input replication determines the format to be used.
 * If this is the first entry in the group, it is not written to the MCDB, and kept to be written with the linked list is finished.
 * We assume that jer2_qax_mc_start_linked_list() was called once for the linked list being created.
 * We assume that jer2_qax_mc_end_linked_list() will be called once after all the linked list entries are written.
 *
 * The linked list entries are written from the last to the first.
 * So this function writes the pointer to the next entry in the list, and writes the software pointer
 * to the previous entry of the next entry in the linked list which was previously written.
 */

static uint32 jer2_qax_ing_mc_one_rep_entry(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  jer2_qax_rep_data_t *rep /* The input replication */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry;
    uint32 dest, cud1, cud2, rep_type;
    soc_mem_t mcdb_bmp_repl_mem;
    DNXC_INIT_FUNC_DEFS;

    mcdb_bmp_repl_mem = TAR_MCDB_BITMAP_REPLICATION_ENTRYm;
    JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, dest, cud1, cud2, rep_type);
    /* Get entry/first group entry to write to, handle free block allocation */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_get_linked_list_entry(unit, &mcdb_entry));
    jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);

    jer2_qax_mcdb_entry_clear_mcdb_bits(mcdb_entry);
    /* write format to the jer2_qax_mcdb_entry_t entry */
    if (rep_type == JER2_QAX_MCDS_REP_TYPE_DEST) {
        soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, CUD_0f, cud1); /* set the 1st CUD */
        soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, CUD_1f, cud2); /* set the 2nd CUD */
        soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, DESTINATIONf, dest); /* set the destination */
        soc_mem_field32_set(unit, TAR_MCDB_SINGLE_REPLICATIONm, mcdb_entry, FORMATf, 1); /* set the entry format */
    } else {
        soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, CUD_0f, cud1); /* set the 1st CUD */
        soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, CUD_1f, cud2); /* set the 2nd CUD */
        soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, BMP_PTRf, /* set the bitmap pointer */
          mcds->ingress_bitmap_start + dest * SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmap_bytes);
        soc_mem_field32_set(unit, mcdb_bmp_repl_mem, mcdb_entry, FORMATf, 0); /* set the entry format */
    }

    /* write backward link to previous start of linked list; update linked list state */
    DNXC_IF_ERR_EXIT(jer2_qax_mc_set_linked_list_entry(unit));
exit:
    DNXC_FUNC_RETURN;
}


/*
 * Set a linked list of the input egress entries, possibly using a provided free block as the first allocation.
 * The replications are taken form the mcds.
 * If is_group_start is non zero, then list_prev is the (free and reserved) group start entry and it is set with replications.
 * Otherwise we do not handle the start of the egress group so there is no need for special handling of the first entry.
 * If the function fails, it will free the given allocated block.
 * In the start_block_index entry, link to the previous entry according to the previous entry of input_block_index.
 * On failure allocated entries are freed, including alloced_block_start.
 * Linked lists replaced by new linked lists are not freed by this function.
 */

uint32 jer2_qax_mcds_set_linked_list (
    DNX_SAND_IN int                           unit,
    DNX_SAND_IN uint8                         list_type,           /* specifies if list_prev is a group start to be filled or not */
    DNX_SAND_IN uint32                        group_id,            /* If list_type is *_START this is the group ID, otherwise this is
                                                                      the entry preceding the single linked list to be created */
    DNX_SAND_IN uint32                        list_end,            /* The entry that end of the created linked list will point to, Same one for all given cores */
    DNX_SAND_IN uint32                        alloced_block_start, /* start index of an allocated block to use for the free list */
    DNX_SAND_IN dnx_free_entries_block_size_t alloced_block_size   /* size of the allocated block to use, should be 0 if none */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    jer2_qax_rep_data_t *rep, *rep_ptr, *couples_ptr;
    int nof_reps_left_in_core;    /* The number of replications left to be processed in the current core */
    int i, couples_left;
    uint32 dest, cud1, cud2, rep_type, next_dest, next_cud1, next_cud2, next_rep_type, port2, ports34[2];

    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_NORMAL(list_type) && DNX_MCDS_TYPE_GET_NONE_START(list_type) == mcds->group_type);
    /* init the linked list created so far to empty */
    jer2_qax_mc_start_linked_list(unit, alloced_block_start, alloced_block_size, group_id, list_end, list_type);

    /* start creating the new linked list */
    nof_reps_left_in_core = mcds->nof_reps;
    rep = mcds->reps;
    if (nof_reps_left_in_core > 0) {
        /* sorting is always needed for separating replications by 2nd CUD and for finding port+outlif couples */
        dnx_sand_os_qsort(rep, nof_reps_left_in_core, sizeof(jer2_qax_rep_data_t), jer2_qax_rep_data_t_compare);

        if (!(SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_ALLOW_DUPLICATES_MODE)) { /* need to check duplicate replications */
            for (i = nof_reps_left_in_core; i > 1; --i) {
                rep_ptr = rep+1;
                if (JER2_QAX_EQ_REP_DATA(rep, rep_ptr)) {
                    mcds->out_err = _SHR_E_PARAM;
                    SOC_EXIT; /* We found duplicate replications in the input */
                }
                rep = rep_ptr;
            }
            rep = mcds->reps;
        }

        if (DNX_MCDS_TYPE_IS_EGRESS(list_type)) { /* Build egress linked list:
            * JER2_QAX egress MC has four formats:
            * TAR_MCDB_EGRESS_BITMAP_POINTER_FORMATm: "one" bitmap replication with one CUD. Uses bitmap pointer, CUD, link pointer. The only format for bitmap replications
            * TAR_MCDB_EGRESS_TWO_COPIES_FORMATm: Two replications with one CUD each. Uses 2x(port, CUD), link pointer.
            * TAR_MCDB_EGRESS_DOUBLE_CUD_FORMATm: Two replications with the same two CUDs. Uses 2xPort, 2xCUD, link pointer. The only format supporting port replications with two CUDs.
            * TAR_MCDB_EGRESS_TDM_FORMATm: Four replications with the same single CUD. Uses 4xPort, CUD, link pointer.
            *   Preferred to TAR_MCDB_EGRESS_TWO_COPIES_FORMATm if multiple port+one CUD replications share the same CUD.
            */
            do { /* loop over port + one CUD replications */
                JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, dest, cud1, cud2, rep_type);
                if (rep_type != JER2_QAX_MCDS_REP_TYPE_DEST || cud2 != DNX_MC_NO_2ND_CUD) {
                    break;
                }
                ++rep;
                if (--nof_reps_left_in_core > 0) {
                    JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, next_dest, next_cud1, next_cud2, next_rep_type);
                    if (next_rep_type == JER2_QAX_MCDS_REP_TYPE_DEST && next_cud2 == DNX_MC_NO_2ND_CUD) { /* If we have a 2nd replication with a single CUD */
                        mcds->nof_possible_reps +=2;
                        ++rep;
                        --nof_reps_left_in_core; /* If we have a 2nd replication with the same single CUD */
                        if (cud1 != next_cud1) { /* Add a 2x(port+cud1) entry with two replications */
                            DNXC_IF_ERR_EXIT(jer2_qax_eg_mc_write_entry_2ports_cud(unit, dest, cud1, next_dest, next_cud1));
                        } else { /* Add a 4xPort+cud1 entry with at least two replications */
                            port2 = next_dest;
                            ports34[1] = ports34[0] = DNX_MULT_EGRESS_PORT_INVALID;
                            for (i = 0; i < 2 && nof_reps_left_in_core > 0; ++i, ++rep, ++mcds->nof_possible_reps, --nof_reps_left_in_core) {
                                JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, next_dest, next_cud1, cud2, next_rep_type);
                                if (rep_type != JER2_QAX_MCDS_REP_TYPE_DEST || cud2 != DNX_MC_NO_2ND_CUD || cud1 != next_cud1) {
                                    break;
                                }
                                ports34[i] = next_dest;
                            }
                            DNXC_IF_ERR_EXIT(jer2_qax_eg_mc_write_entry_4ports_cud(unit, dest, port2, ports34[0], ports34[1], cud1));
                        }
                        continue;
                    }
                }
                /* Add a 2x(port+cud1) entry with one replication */
                ++mcds->nof_possible_reps;
                DNXC_IF_ERR_EXIT(jer2_qax_eg_mc_write_entry_2ports_cud(unit, dest, cud1, DNX_MULT_EGRESS_PORT_INVALID, DNX_MC_NO_2ND_CUD));
            } while (nof_reps_left_in_core > 0);

            /* loop over port + two CUDs replications */
            while (nof_reps_left_in_core > 0) {
                JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, dest, cud1, cud2, rep_type);
                if (rep_type != JER2_QAX_MCDS_REP_TYPE_DEST) {
                    break;
                }
                DNX_MC_ASSERT(cud2 != DNX_MC_NO_2ND_CUD);
                ++rep;
                port2 = DNX_MULT_EGRESS_PORT_INVALID;
                ++mcds->nof_possible_reps;
                if (--nof_reps_left_in_core > 0) {
                    JER2_QAX_MCDS_REP_DATA_GET_FIELDS(rep, next_dest, next_cud1, next_cud2, next_rep_type);
                    if (next_rep_type == JER2_QAX_MCDS_REP_TYPE_DEST && next_cud1 == cud1 && next_cud2 == cud2) {
                        port2 = next_dest;
                        ++rep;
                        ++mcds->nof_possible_reps;
                        --nof_reps_left_in_core;
                    }
                }
                /* Add a 2x(port+2 cuds) entry with one/two replications */
                DNXC_IF_ERR_EXIT(jer2_qax_eg_mc_write_entry_2ports_2cuds(unit, dest, port2, cud1, cud2));
            }

            /* loop over bitmap + one CUD replications */
            while (nof_reps_left_in_core > 0) {
                DNX_MC_ASSERT(rep_type != JER2_QAX_MCDS_REP_TYPE_DEST && cud2 == DNX_MC_NO_2ND_CUD);
                ++rep;
                --nof_reps_left_in_core;
                mcds->nof_possible_reps += (DNX_MC_NOF_EGRESS_PORTS-1); /* assume every possibler port is replicated to */
                /* Add a bitmap pointer+cud entry */
                DNXC_IF_ERR_EXIT(jer2_qax_eg_mc_write_entry_bitmap_cud(unit, dest, cud1));
            }

        } else { /* Build ingress linked list:
            * JER2_QAX ingress MC has three formats:
            * TAR_MCDB_BITMAP_REPLICATION_ENTRYm: "one" bitmap replication with two CUDs. Uses bitmap pointer, 2xCUD, link pointer. The only format for bitmap replications
            * TAR_MCDB_SINGLE_REPLICATIONm: One replications with two CUDs. Uses destination, 2xCUD, link pointer.
            * TAR_MCDB_DOUBLE_REPLICATIONm: Two replications, each with one CUD. Uses 2x(Port, CUD), link bit.
            *   The only format with no link pointer, either the end of the group or links to the next entry. 
            */
            int full_block_size, pointer_entry;
            /* Find couples with the same 1st CUD and no 2nd CUD, and move them to the start of the replications */
            for (couples_ptr = rep; nof_reps_left_in_core > 0; --nof_reps_left_in_core, ++couples_ptr) {
                JER2_QAX_MCDS_REP_DATA_GET_FIELDS(couples_ptr, dest, cud1, cud2, rep_type);
                if (rep_type != JER2_QAX_MCDS_REP_TYPE_DEST || cud2 != DNX_MC_NO_2ND_CUD) {
                    break;
                }
            }
            couples_left = couples_ptr - mcds->reps;
            DNX_MC_ASSERT(couples_left >= 0 && nof_reps_left_in_core + couples_left == mcds->nof_reps);
            rep = (couples_ptr = mcds->reps) + (nof_reps_left_in_core = mcds->nof_reps);
            couples_left /= 2; /* holds number of couples left */
            nof_reps_left_in_core -= couples_left * 2; /* will now hold the number of replications left not in couples */

            if (mcds->is_group_start_to_be_filled) { /* Do we need to fill (and not yet write) the first entry of a MC group? */
                DNXC_IF_ERR_EXIT(jer2_qax_ing_mc_one_rep_entry(unit, --rep));
                if (nof_reps_left_in_core > 0) {
                    --nof_reps_left_in_core;
                } else {
                    ++nof_reps_left_in_core;
                    --couples_left;
                }
                DNX_MC_ASSERT(!mcds->is_group_start_to_be_filled && couples_left >= 0);
            }
            DNX_MC_ASSERT(nof_reps_left_in_core + couples_left * 2 == rep - couples_ptr && couples_left >= 0); 
            while (couples_left > 0) {
                pointer_entry = mcds->linked_list == JER2_QAX_MC_INGRESS_LINK_PTR_END ? 0 : 1; /* is a pointer entry needed at the end of the block */
                if (pointer_entry && nof_reps_left_in_core <= 0) {
                    nof_reps_left_in_core +=2;
                    --couples_left;
                    if (couples_left == 0) {
                        break;
                    }
                }
                full_block_size = couples_left + pointer_entry; /* calculate size of entries block to write */
                if (full_block_size > DNX_MCDS_MAX_FREE_BLOCK_SIZE) {
                    full_block_size = DNX_MCDS_MAX_FREE_BLOCK_SIZE;
                }
                DNX_MC_ASSERT(full_block_size > pointer_entry && nof_reps_left_in_core >= pointer_entry);
                DNXC_IF_ERR_EXIT(jer2_qax_ing_no_ptr_block(unit, &rep, &couples_ptr, &couples_left, &full_block_size)); /* writes block and updates couples_ptr, couples_left */
                nof_reps_left_in_core -= pointer_entry;
                DNX_MC_ASSERT(nof_reps_left_in_core + couples_left * 2 == rep - couples_ptr && couples_left >= 0); 
            }

            for (; nof_reps_left_in_core > 0; --nof_reps_left_in_core) { /* write the remaining entries with pointers */
                DNX_MC_ASSERT(nof_reps_left_in_core == rep - couples_ptr); 
                DNXC_IF_ERR_EXIT(jer2_qax_ing_mc_one_rep_entry(unit, --rep));
            }
            DNX_MC_ASSERT(couples_ptr == rep && (nof_reps_left_in_core | couples_left) == 0);
        }
        /* set self replication */
        jer2_qax_self_replication_set(unit , group_id, TRUE);
    } else if (DNX_MCDS_TYPE_IS_INGRESS(list_type)) {
        /* For no replications the start entry should be different for ingress groups */
        mcds->start_entry = mcds->empty_ingr_value;
    }
    
    DNXC_IF_ERR_EXIT(jer2_qax_mc_end_linked_list(unit)); /* Finish writing the linked list, activating it in one write */

exit:
    if (mcds->state == JER2_QAX_MCDS_STATE_STARTED) {  /* free the linked list written so far on error or when running out of mcdb entries */
        DNX_MC_ASSERT(mcds->linked_list != mcds->list_end);
        jer2_qax_mcdb_free_linked_list_till_my_end(unit, mcds->linked_list, DNX_MCDS_TYPE_VALUE_EGRESS, list_end);
    } else {
        DNX_MC_ASSERT(mcds->state == JER2_QAX_MCDS_STATE_INITED || mcds->state == JER2_QAX_MCDS_STATE_FINISHED);
    }
    mcds->state = 0;
    if (mcds->block_size) { /* free the remaining allocated free block */
        jer2_qax_mcds_build_free_blocks(unit, mcds, mcds->block_start, mcds->block_start + mcds->block_size - 1,
          jer2_qax_mcds_get_region(mcds, mcds->block_start), dnxMcdsFreeBuildBlocksAddAll);
    }
    DNXC_FUNC_RETURN;
}


/*
 * Sets the egress group to the given replications configuring its linked list.
 * If the group does not exist, it will be created or an error will be returned based on allow_create.
 * Creation may involve relocating the MCDB entry which will be the start
 * of the group, and possibly other consecutive entries.
 *
 * We always want to create entries with pointers from port+outlif couples and from bitmaps.?????
 * We need to leave one entry with a pointer for the start of the group.
 * every block of entries with no pointers ends with an entry pointer, except for the end of the group.
 */
uint32 jer2_qax_mult_eg_group_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  dnx_mc_id_t          mcid,         /* the group mcid */
    DNX_SAND_IN  uint8                allow_create, /* if non zero, will create the group if it does not exist */
    DNX_SAND_IN  uint32               group_size,   /* size of ports and cuds to read group replication data from */
    DNX_SAND_IN  dnx_mc_replication_t *reps,        /* input array containing replications (using logical ports) */
    DNX_SAND_OUT DNX_TMC_ERROR        *out_err      /* return possible errors that the caller may want to ignore */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint8 group_exists = 0;
    int failed = 1, group_start_alloced = 0;
    uint32 entry_type, group_entry_id;
    uint32 old_group_entries = DNX_MC_EGRESS_LINK_PTR_END; /* the linked lists of the previous group content */

    DNXC_INIT_FUNC_DEFS;

    DNX_MC_ASSERT(mcid < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
    DNXC_IF_ERR_EXIT(jer2_qax_egress_group_open_get(unit, mcid, &group_exists));
    if (!(group_exists | allow_create)) {
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }
    { /* For each core handle its existing linked list or its first linked list entries */
        group_entry_id = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid);
        entry_type = JER2_QAX_MCDS_GET_TYPE(mcds, group_entry_id);

        if (group_exists) { /* for an existing group store its linked list (to be freed) at old_group_entries */
            DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_EGRESS_START(entry_type));
            DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, group_entry_id, DNX_MCDS_TYPE_VALUE_EGRESS, &old_group_entries));
        } else {
            /* If this is a new group, we need to reserve its first entry, and possibly relocate the entry there */
            if (DNX_MCDS_TYPE_IS_USED(entry_type)) { /* relocate conflicting entries if needed */
                DNX_MC_ASSERT(!DNX_MCDS_TYPE_IS_START(entry_type));
                DNXC_IF_ERR_EXIT(jer2_qax_mcdb_relocate_entries(unit, group_entry_id, 0, 0));
                if (mcds->out_err) { /* If failed due to lack of memories, do the same */
                    SOC_EXIT;
                }
            } else { /* just allocate the start entry of the group */
                DNXC_IF_ERR_EXIT(jer2_qax_mcds_reserve_group_start(mcds, group_entry_id));
            }
            group_start_alloced = 1;
        }

    }

    DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_egress_replications_from_arrays(unit, TRUE, group_size, reps)); /* copy group replications to mcds */

    DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list( /* build the group, including the first entry */
      unit, DNX_MCDS_TYPE_VALUE_EGRESS_START, mcid, DNX_MC_EGRESS_LINK_PTR_END, 0, 0));
    if (mcds->out_err) { /* If failed due to lack of memories, do the same */
        SOC_EXIT;
    }

    if (group_exists) { /* for an existing group free its previous linked lists */
        if (old_group_entries != DNX_MC_EGRESS_LINK_PTR_END) { /* free previous linked list of the group not used any more */
            DNXC_IF_ERR_EXIT(jer2_qax_mcdb_free_linked_list(unit, old_group_entries, DNX_MCDS_TYPE_VALUE_EGRESS));
        }
    } else {
        DNXC_IF_ERR_EXIT(dnx_egress_group_open_set(unit, mcid, 1));
    }

    failed = 0;
exit:
    if (group_start_alloced & failed) { /* free linked list start entries if needed */
        DNX_MC_ASSERT(!group_exists);
        group_entry_id = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid);
        if (jer2_qax_mcds_build_free_blocks(unit, mcds, group_entry_id, group_entry_id,
          jer2_qax_mcds_get_region(mcds, group_entry_id), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed) != SOC_E_NONE) {
            cli_out("jer2_qax_mcds_build_free_blocks failed\n");
        }
    }
    if (out_err != NULL && _rv == SOC_E_NONE) {
        *out_err = mcds->out_err;
    }
    DNXC_FUNC_RETURN;
}


/*
 * This API closes an egress-multicast-replication group for the given multicast-id.
 * The user only specifies the multicast-id.
 * All inner link-list entries are freed and handled by the driver.
 */
uint32 jer2_qax_mult_eg_group_close(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t mcid
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 group_entry_id = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid);
    DNX_TMC_ERROR internal_err;
    uint8 does_exist;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_qax_mult_does_group_exist(unit, mcid, TRUE, &does_exist));
    if (does_exist) { /* do nothing if not open */
        DNXC_IF_ERR_EXIT(jer2_qax_mult_eg_group_set( /* empty the group */
          unit, mcid, FALSE, 0, 0, &internal_err));
        if (internal_err) { /* should never require more entries, so if this happens it is an internal error */
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("too many entries")));
        }

        /* Free the linked list start entry */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(unit, mcds, group_entry_id, group_entry_id,
          &mcds->egress_starts, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));

        DNXC_IF_ERR_EXIT(dnx_egress_group_open_set(unit, mcid, 0));
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Adds the given replications to the egress multicast group.
 * It is an error if the group is not open.
 */
uint32 jer2_qax_mult_eg_reps_add(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  dnx_mc_id_t           group_mcid, /* group mcid */
    DNX_SAND_IN  uint32                nof_reps,   /* number of replications to add */
    DNX_SAND_IN  dnx_mc_replication_t  *reps,      /* input array containing replications to add*/
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err    /* return possible errors that the caller may want to ignore */
)
{
    uint16 group_size;
    uint32 linked_list_start;
    uint32 old_group_entries = DNX_MC_EGRESS_LINK_PTR_END; /* the linked list  of the previous group content */
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint8 is_open;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reps);
    DNXC_IF_ERR_EXIT(jer2_qax_egress_group_open_get(unit, group_mcid, &is_open));
    if (!is_open) { /* group is not open */
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }

    /* clear the replications in mcds and add the new replication */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_egress_replications_from_arrays(unit, TRUE, nof_reps, reps));

    /* store current linked list */
    linked_list_start = JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, group_mcid);
    DNX_MC_ASSERT(JER2_QAX_MCDS_GET_TYPE(mcds, linked_list_start) == DNX_MCDS_TYPE_VALUE_EGRESS_START);
    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER( /* store previous linked lists to be freed */
      mcds, unit, linked_list_start, DNX_MCDS_TYPE_VALUE_EGRESS, &old_group_entries));

    /* get the replications of the current group into the mcds */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_group(unit, FALSE, group_mcid,
      DNX_MCDS_TYPE_VALUE_EGRESS, JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS - nof_reps, &group_size));
    if (group_size > JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS - nof_reps) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    }

    /* recreate the group with the extra replication by building the linked lists, including the first entry */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list(unit, DNX_MCDS_TYPE_VALUE_EGRESS_START, group_mcid,
      DNX_MC_EGRESS_LINK_PTR_END, 0, 0));
    if (mcds->out_err) { /* If failed, do some error code fixing */
        if (mcds->out_err == _SHR_E_PARAM) {
            mcds->out_err = _SHR_E_EXISTS;
        }
    } else if (old_group_entries != DNX_MC_EGRESS_LINK_PTR_END) {
        /* If succeeded, free previous linked lists that are not used any more */
        DNXC_IF_ERR_EXIT(jer2_qax_mcdb_free_linked_list(unit, old_group_entries, DNX_MCDS_TYPE_VALUE_EGRESS));
    }

exit:
    if (out_err != NULL && _rv == SOC_E_NONE) {
        *out_err = mcds->out_err;
    }
    DNXC_FUNC_RETURN;
}

/*
 * Removes the given replications from the non bitmap egress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 jer2_qax_mult_eg_reps_remove(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  dnx_mc_id_t           group_mcid,   /* group mcid */
    DNX_SAND_IN  uint32                nof_reps,     /* number of replications to remove */
    DNX_SAND_IN  dnx_mc_replication_t  *reps,        /* input array containing replications to remove */
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err      /* return possible errors that the caller may want to ignore */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint16 group_size;
    uint8 is_open;
    uint32 old_group_entries = 0;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reps);

    DNXC_IF_ERR_EXIT(jer2_qax_egress_group_open_get(unit, group_mcid, &is_open));
    if (!is_open) { /* group is not open */
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }

    /* get the replications of the group into the mcds */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_group(unit, TRUE, group_mcid,
        DNX_MCDS_TYPE_VALUE_EGRESS, JER2_QAX_MULT_MAX_EGRESS_REPLICATIONS, &group_size));

    /* remove the given replication from the mcds */
    DNXC_IF_ERR_EXIT(jer2_qax_mult_remove_replications(unit, nof_reps, reps));
    if (mcds->out_err) { /* If the replication was not found, exit */
        SOC_EXIT;
    }

    /* recreate the group with the removed replications */
    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER( /* store previous linked list to be freed */
      mcds, unit, JER2_QAX_MCDS_GET_EGRESS_GROUP_START(mcds, group_mcid), DNX_MCDS_TYPE_VALUE_EGRESS, &old_group_entries));

    DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list( /* build the group, including the first entry */
        unit, DNX_MCDS_TYPE_VALUE_EGRESS_START, group_mcid, DNX_MC_EGRESS_LINK_PTR_END, 0, 0));
    if (mcds->out_err) { /* If failed due to lack of free entries */
#ifndef JER2_QAX_EGRESS_MC_DELETE_FAILS_ON_FULL_MCDB
        /* If we can not reconstruct the group due to MCDB being full, just remove the replication/entry */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_remove_replications_from_group(unit, group_mcid, nof_reps, reps));
        mcds->out_err = _SHR_E_NONE;
#endif /* JER2_ARAD_EGRESS_MC_DELETE_FAILS_ON_FULL_MCDB */
    } else if (old_group_entries != DNX_MC_EGRESS_LINK_PTR_END) {
        /* free previous linked list of the group not used any more */
        DNXC_IF_ERR_EXIT(jer2_qax_mcdb_free_linked_list(unit, old_group_entries, DNX_MCDS_TYPE_VALUE_EGRESS));
    }

exit:
    if (out_err != NULL && _rv == SOC_E_NONE) {
        *out_err = mcds->out_err;
    }
    DNXC_FUNC_RETURN;
}


/*
 * Returns the size of the multicast group with the specified multicast id.
 * Not needed for bcm APIs, so not tested.
 * returns 0 for non open groups.
 */
uint32 jer2_qax_mult_eg_group_size_get(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  dnx_mc_id_t  multicast_id_ndx,
    DNX_SAND_OUT uint32       *mc_group_size
)
{
    uint8 is_open;
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(mc_group_size);

    DNXC_IF_ERR_EXIT(jer2_qax_mult_eg_get_group(
      unit, multicast_id_ndx, 0, 0, 0, 0, mc_group_size, &is_open));

exit:
  DNXC_FUNC_RETURN;
}


#if 0 /* JER2_QAX implementation */
/*
 * This function creates or destroys a bitmap group, Setting the bitmap to not replicate (0).
 */
uint32 jer2_qax_mult_eg_bitmap_group_zero(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  uint8       create            /* should be 1 for create, and 0 for destroy */
)
{
    uint32 data[DNX_EGQ_VLAN_TABLE_TBL_ENTRY_SIZE] = {0};
    dnx_mc_core_bitmap_t old_cores, new_cores = DNX_MC_CORE_BITAMAP_NO_CORES;

    DNXC_INIT_FUNC_DEFS;
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCID is not a bitmap group")));

    DNXC_IF_ERR_EXIT(dnx_egress_group_get_core_replication(unit, multicast_id_ndx, &old_cores)); /* get egress cores replicated to */
    if (old_cores != new_cores) {
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, multicast_id_ndx, new_cores)); /* mark no replications to cores */
    }
    if (!SOC_IS_QAX(unit)) {  
        DNXC_IF_ERR_EXIT(WRITE_EGQ_VLAN_TABLEm(unit, EGQ_BLOCK(unit, SOC_CORE_ALL), multicast_id_ndx, data)); /* mark bitmap as containing no ports */
    }
    DNXC_IF_ERR_EXIT(dnx_egress_group_open_set(unit, multicast_id_ndx, create)); /* mark the group as open */

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This function opens a bitmap group, and sets it to replicate to the given ports.
 */
uint32 jer2_qax_mult_eg_bitmap_group_create(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx /* group mcid */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(jer2_qax_mult_eg_bitmap_group_zero(unit, multicast_id_ndx, 1));
exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Set the bitmap of the given egress bitmap group to the given bitmap.
*     The bitmap is of TM ports (and not of local ports).
*********************************************************************/
uint32 jer2_qax_mult_eg_bitmap_group_update(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  dnx_mc_id_t                           multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *group /* TM port bitmap to set */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(group);
    if (multicast_id_ndx >= JER2_QAX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, &group->bitmap[0]));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * This function closed a bitmap group, clearing its hardware replications.
 */
uint32 jer2_qax_mult_eg_bitmap_group_close(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  dnx_mc_id_t  multicast_id_ndx
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(jer2_qax_mult_eg_bitmap_group_zero(unit, multicast_id_ndx, 0));
exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*   Get the index inside a bitmap from a local port.
*   The bitmap index is the index in the bitmap depending on the core ID and TM port.
*********************************************************************/
uint32 jer2_qax_mult_eg_bitmap_local_port_2_bitmap_bit_index(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_FAP_PORT_ID port,          /* local port */
    DNX_SAND_OUT uint32              *out_bit_index /* returned bit index */
)
{
    DNXC_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO(unit)) {
        uint32 tm_port;
        int core;
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
        *out_bit_index = tm_port + DNX_TMC_NOF_FAP_PORTS_PER_CORE * core;
    } else {
        *out_bit_index = port;
    }
exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Add port replications to an Egress-Multicast bitmap group. - replaced by jer2_qax_mult_eg_bitmap_group_bm_add
*********************************************************************/
uint32 jer2_qax_mult_eg_bitmap_group_port_add(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_FAP_PORT_ID port,         /* local port to add */
    DNX_SAND_OUT DNX_TMC_ERROR       *out_err      /* return possible errors that the caller may want to ignore */
)
{
    uint32 bit_index, word_index, bit_mask;
    dnx_mc_egr_bitmap_t bitmap = {0};

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= JER2_QAX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }
    DNXC_IF_ERR_EXIT(jer2_qax_mult_eg_bitmap_local_port_2_bitmap_bit_index(unit, port, &bit_index));
    word_index = bit_index / DNX_SAND_NOF_BITS_IN_UINT32; 
    bit_mask = (uint32)1 << (bit_index & (DNX_SAND_NOF_BITS_IN_UINT32 - 1));

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    if (bitmap[word_index] & bit_mask) {
        *out_err = _SHR_E_EXISTS; /* port is already replicated to */
    } else {
        bitmap[word_index] |= bit_mask;
        DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
        *out_err = _SHR_E_NONE;
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Add port replications from a bitmap to an Egress-Multicast bitmap group.
*********************************************************************/
uint32 jer2_qax_mult_eg_bitmap_group_bm_add(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *add_bm, /* TM ports to add */
    DNX_SAND_OUT DNX_TMC_ERROR                         *out_err /* return possible errors that the caller may want to ignore */
)
{
    unsigned nof_tm_port_words = (DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dnx_mc_egr_bitmap_t bitmap = {0};
    const uint32 *changes = &add_bm->bitmap[0];
    uint32 *to_write = bitmap;

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= JER2_QAX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    for (; nof_tm_port_words; --nof_tm_port_words) { /* add new ports to bitmap */
        if (*to_write & *changes) {
            *out_err = _SHR_E_EXISTS; /* some ports are already replicated to */
            SOC_EXIT;
        }
        *(to_write++) |= *(changes++);
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
    *out_er*out_err;

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Removes a port replication of an egress multicast bitmap group. - replaced by jer2_qax_mult_eg_bitmap_group_bm_remove
*********************************************************************/
uint32 jer2_qax_mult_eg_bitmap_group_port_remove(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_FAP_PORT_ID port,         /* local port to remove */
    DNX_SAND_OUT DNX_TMC_ERROR       *out_err      /* return possible errors that the caller may want to ignore */
)
{
    uint32 bit_index, word_index, bit_mask;
    dnx_mc_egr_bitmap_t bitmap = {0};

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= JER2_QAX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }
    DNXC_IF_ERR_EXIT(jer2_qax_mult_eg_bitmap_local_port_2_bitmap_bit_index(unit, port, &bit_index));
    word_index = bit_index / DNX_SAND_NOF_BITS_IN_UINT32;
    bit_mask = (uint32)1 << (bit_index & (DNX_SAND_NOF_BITS_IN_UINT32 - 1));

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    if (bitmap[word_index] & bit_mask) {
        bitmap[word_index] &= ~bit_mask;
        DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
        *out_err = _SHR_E_NONE;
    } else {
        *out_err = _SHR_E_NOT_FOUND; /* port is not replicated to */
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Add port replications from a bitmap to an Egress-Multicast bitmap group.
*********************************************************************/
uint32 jer2_qax_mult_eg_bitmap_group_bm_remove(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *rem_bm, /* TM ports to remove */
    DNX_SAND_OUT DNX_TMC_ERROR                         *out_err /* return possible errors that the caller may want to ignore */
)
{
    unsigned nof_tm_port_words = (DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dnx_mc_egr_bitmap_t bitmap = {0};
    const uint32 *changes = &rem_bm->bitmap[0];
    uint32 *to_write = bitmap;

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= JER2_QAX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    for (; nof_tm_port_words; --nof_tm_port_words) { /* add new ports to bitmap */
        if (~*to_write & *changes) {
            *out_err = _SHR_E_NOT_FOUND; /* some ports are not replicated to */
            SOC_EXIT;
        }
        *(to_write++) &= ~*(changes++);
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
    *out_err = _SHR_E_NONE;

exit:
    DNXC_FUNC_RETURN;
}
#endif /* 0 JER2_QAX implementation */


/*
 * Ingress multicast code
 */

/*
 * given a MCID configures it to be an ingress group or not (fabric+egress) in CGM_IS_ING_MCm.
 */
uint32 jer2_qax_mcid_type_set_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  mcid,
    DNX_SAND_IN  uint32 is_ingress /* value to set, must be 0 or 1 (ingress or not) */
)
{
  uint32 entry[2] = {0};
  const unsigned bit_offset = mcid % JER2_QAX_MC_MCIDS_PER_IS_ING_MC_ENTRY;
  const int table_index = mcid / JER2_QAX_MC_MCIDS_PER_IS_ING_MC_ENTRY;

  DNXC_INIT_FUNC_DEFS;
  if (mcid >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCID out of range")));
  }

  DNXC_IF_ERR_EXIT(READ_CGM_IS_ING_MCm(unit, MEM_BLOCK_ANY, table_index, entry));

  if (((*entry >> bit_offset) & 1) != is_ingress) { /* If setting a different value, update hardware */
    *entry &= ~(((uint32)1) << bit_offset);
    *entry |= (((uint32)is_ingress) << bit_offset);
    DNXC_IF_ERR_EXIT(WRITE_CGM_IS_ING_MCm(unit, MEM_BLOCK_ANY, table_index, entry));
  }
exit:
  DNXC_FUNC_RETURN;
}

/*
 *  This function checks if an given MCID is an ingress group by checking CGM_IS_ING_MCm.
 */
uint32 jer2_qax_mcid_type_get_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  mcid,
    DNX_SAND_OUT uint32 *is_ingress /* returns here the value 1/0 for ingress group/not */
)
{
  uint32 entry[2];
  const unsigned bit_offset = mcid % JER2_QAX_MC_MCIDS_PER_IS_ING_MC_ENTRY;
  const int table_index = mcid / JER2_QAX_MC_MCIDS_PER_IS_ING_MC_ENTRY;

  DNXC_INIT_FUNC_DEFS;
  if (mcid >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCID out of range")));
  }
  DNXC_NULL_CHECK(is_ingress);

  DNXC_IF_ERR_EXIT(READ_CGM_IS_ING_MCm(unit, MEM_BLOCK_ANY, table_index, entry));
  *is_ingress = ((*entry) >> bit_offset) & 1;

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Calculate ingress multicast destination encoding
 */
static uint32 jer2_qax_gport2ing_mc_dest_encoding(
    DNX_SAND_IN int         unit,
    DNX_SAND_IN soc_gport_t gport,          /* input destination gport */
    DNX_SAND_OUT uint32     *dest_encoding, /* output destination encoding */
    DNX_SAND_OUT uint32     *is_bitmap      /* output boolean reporting if the destination is a bitmap index */
)
{
    uint32 dest;
    DNX_TMC_DEST_INFO dest_info = {0};
    DNXC_INIT_FUNC_DEFS;

    /* find the destination type and ID */
    DNXC_IF_ERR_EXIT(dnx_gport_to_tm_dest_info(unit, gport, &dest_info));
    dest = dest_info.id;
    *is_bitmap = 0;

    /* check for legal destination id and encode the destination hardware field */
    if (dest_info.type == DNX_TMC_DEST_TYPE_QUEUE) { /* direct Queue_id */
        if (dest > SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_queue) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Queue %u, is over %u."),
              (unsigned)dest, (unsigned)SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_queue));
        }
        dest |= (JER2_QAX_MC_ING_DESTINATION_Q_TYPE << JER2_QAX_MC_ING_DESTINATION_ID_BITS);
    } else if (dest_info.type == DNX_TMC_DEST_TYPE_SYS_PHY_PORT) { /* system port */
        if (dest >= SOC_DNX_CONFIG(unit)->tm.ingress_mc_nof_sysports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("system port out of range")));
        }
        dest |= (JER2_QAX_MC_ING_DESTINATION_SYSPORT_TYPE << JER2_QAX_MC_ING_DESTINATION_ID_BITS);
    } else if (dest_info.type == DNX_TMC_DEST_TYPE_LAG) { /* LAG/trunk */
        dest |= (JER2_QAX_MC_ING_DESTINATION_SYSPORT_LAG_TYPE << JER2_QAX_MC_ING_DESTINATION_ID_BITS);
    } else if (dest_info.type == DNX_TMC_DEST_TYPE_MULTICAST) { /* bitmap replication */
        if (dest >= SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmaps) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ingress multicast bitmap ID %u, is not under %u."),
              (unsigned)dest, (unsigned)SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmaps));
        }
        *is_bitmap = 1;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsupported ingress multicast destination type in given gport")));
    }
    *dest_encoding = dest;
exit:
    DNXC_FUNC_RETURN;
}


/*
 * This functions copies the ingress replication data from the given replication array into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * The destination translation (logical ports and encoding) is done by this function and not by bcm code.
 */
static uint32 jer2_qax_mcds_copy_ingress_replications_from_arrays(
    DNX_SAND_IN int       unit,
    DNX_SAND_IN uint8     do_clear,    /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN uint32    arrays_size, /* size of output arrays */
    DNX_SAND_IN soc_multicast_replication_t *reps /* input array containing replications using logical ports */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint32 size_left = arrays_size, dest, cud1, cud2, is_bitmap = 0;
    DNXC_INIT_FUNC_DEFS;

    if (do_clear) {
        jer2_qax_mcds_clear_replications(unit, mcds, DNX_MCDS_TYPE_VALUE_INGRESS);
    }
    DNX_MC_ASSERT(mcds->group_type == DNX_MCDS_TYPE_VALUE_INGRESS && mcds->nof_reps == (int32)mcds->nof_dest_cud_reps + mcds->nof_bitmap_reps);
    if (mcds->nof_reps + arrays_size > JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    }
    for (; size_left; --size_left, ++reps) {
        cud1 = reps->encap1 ? reps->encap1 : SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud;
        cud2 = reps->flags & SOC_MUTICAST_REPLICATION_ENCAP2_VALID ? reps->encap2 : DNX_MC_NO_2ND_CUD;
        if (cud1 > SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud || cud2 > SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG(
              "Invalid ingress encapsulation ID, should be between 0 and 0x%x"), SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud));
        }
        
        /* calculate destination encoding and type; and store replication in mcds */
        DNXC_IF_ERR_EXIT(jer2_qax_gport2ing_mc_dest_encoding(unit, reps->port, &dest, &is_bitmap));
        if (is_bitmap) {
            jer2_qax_add_ingress_replication_bitmap(mcds, dest * SOC_DNX_CONFIG(unit)->jer2_qax->nof_ingress_bitmap_bytes + mcds->ingress_bitmap_start, cud1, cud2);
        } else {
            jer2_qax_add_ingress_replication_dest(mcds, dest, cud1, cud2);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This functions copies the ingress replication data from the given replication array into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * The destination translation (logical ports and encoding) is done by this function and not by bcm code.
 */
static uint32 jer2_qax_convert_soc2dnx_rep_array(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  uint32                      arrays_size, /* size of output arrays */
    DNX_SAND_IN  soc_multicast_replication_t *reps,       /* input replication array containing replications using logical ports */
    DNX_SAND_OUT dnx_mc_replication_t        *oreps       /* output replications array */
)
{
    uint32 size_left = arrays_size, dest, is_bitmap;
    DNXC_INIT_FUNC_DEFS;

    for (; size_left; --size_left, ++reps) {
        oreps->cud = reps->encap1 ? reps->encap1 : SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud;
        oreps->additional_cud = reps->flags & SOC_MUTICAST_REPLICATION_ENCAP2_VALID ? reps->encap2 : DNX_MC_NO_2ND_CUD;
        if (oreps->cud > SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud || oreps->additional_cud > SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG(
              "Invalid ingress encapsulation ID, should be between 0 and 0x%x"), SOC_DNX_CONFIG(unit)->tm.ingress_mc_max_cud));
        }
        
        /* calculate destination encoding and type; and store replication in mcds */
        DNXC_IF_ERR_EXIT(jer2_qax_gport2ing_mc_dest_encoding(unit, reps->port, &dest, &is_bitmap));
        if (is_bitmap) {
            dest |= JER2_ARAD_MC_EGR_IS_BITMAP_BIT; /* bitmap + outlif replication */
        }
        oreps->dest = dest;
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This function sets the ingress group to the given replications,
 * configuring its linked list.
 * If the group does not exist, it will be created or an error will be returned based on allow_create.
 * Creation may involve relocating the mcdb entry which will be the start
 * of the group, and possibly other consecutive entries.
 * Creation enables the group in CGM_IS_ING_MCm.
 *
 * This function also configures the table which indicates
 * per Multicast ID whether to perform ingress replication
 * or not, if ingress replication is not chosen, fabric or
 * egress multicast will be performed on the packet.
 */
uint32 jer2_qax_mult_ing_group_set(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  dnx_mc_id_t                 mcid,
    DNX_SAND_IN  soc_multicast_replication_t *reps,
    DNX_SAND_IN  uint32                      mc_group_size,
    DNX_SAND_IN  uint8                       allow_create, /* if non zero, will create the group if it does not exist */
    DNX_SAND_IN  uint8                       to_delete,    /* if non zero, will destroy the group */
    DNX_SAND_OUT DNX_TMC_ERROR               *out_err      /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    int group_exists = 0, group_start_alloced = 0, failed = 1;
    uint32 group_entry_type;
    uint32 old_group_entries = JER2_QAX_MC_INGRESS_LINK_PTR_END; /* the linked lists of the previous group content */

    DNXC_INIT_FUNC_DEFS;
    if (mc_group_size) {
        DNXC_NULL_CHECK(reps);
        DNX_MC_ASSERT(!to_delete);
        if (mc_group_size > JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ingress MC group size is too big")));
        }
    }
    mcds->out_err = _SHR_E_NONE;
    DNX_MC_ASSERT(mcid < SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids);

    group_entry_type = JER2_QAX_MCDS_GET_TYPE(mcds, mcid);
    group_exists = group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS_START;
    if (!group_exists && !allow_create) {
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }

    /* handle existing linked list or first linked list entry */
    if (group_exists) { /* for an existing group store its linked list (to be freed) at old_group_entries */
        DNX_MC_ASSERT(group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS_START);
        DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, mcid, DNX_MCDS_TYPE_VALUE_INGRESS, &old_group_entries));
    } else if (mcid != DNX_MC_EGRESS_LINK_PTR_END) {
        /* If this is a new group, we need to reserve its first entry, and possibly relocate the entry there */
        if (DNX_MCDS_TYPE_IS_USED(group_entry_type)) {
            DNX_MC_ASSERT(!DNX_MCDS_TYPE_IS_START(group_entry_type));
            DNXC_IF_ERR_EXIT(jer2_qax_mcdb_relocate_entries(unit, mcid, 0, 0)); /* relocate conflicting entries */
            if (mcds->out_err) { /* If failed due to lack of memories, do the same */
                SOC_EXIT;
            }
        } else { /* just allocate the start entry of the group */
            DNXC_IF_ERR_EXIT(jer2_qax_mcds_reserve_group_start(mcds, mcid));
        }
        group_start_alloced = 1;
    }

    DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_ingress_replications_from_arrays(unit, TRUE, mc_group_size, reps)); /* copy group replications to mcds */
    /* create the linked list */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list(unit, DNX_MCDS_TYPE_VALUE_INGRESS_START, mcid, JER2_QAX_MC_INGRESS_LINK_PTR_END, 0, 0));
    if (mcds->out_err) { /* If failed due to lack of memories, do the same */
        SOC_EXIT;
    }

    /* Set MCID to be an ingress MC group */
    DNXC_IF_ERR_EXIT(jer2_qax_mcid_type_set_unsafe(unit, mcid, to_delete ? 0 : 1));

    if (old_group_entries != JER2_QAX_MC_INGRESS_LINK_PTR_END) { /* free previous linked list of the group not used any more */
        DNXC_IF_ERR_EXIT(jer2_qax_mcdb_free_linked_list(unit, old_group_entries, DNX_MCDS_TYPE_VALUE_INGRESS));
    }

    failed = 0;
exit:
    if (out_err != NULL && _rv == SOC_E_NONE) {
        *out_err = mcds->out_err;
    }
    if (group_start_alloced & failed) { /* free linked list start entries if needed */
        DNX_MC_ASSERT(!group_exists);
        if(jer2_qax_mcds_build_free_blocks(unit, mcds, mcid, mcid, jer2_qax_mcds_get_region(mcds, mcid), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed) != SOC_E_NONE){
            cli_out("jer2_qax_mcds_build_free_blocks failed\n");
        }
    }
    DNXC_FUNC_RETURN;
}


/*
 * Ingres multicast MBCM functions
 */


/*
 * This API sets the ingress group to the given replications,
 * configuring its linked list; and creates the group if it did not exist.
 */
uint32 jer2_qax_mult_ing_group_open(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *mc_group,        /* group replications to set */
    DNX_SAND_IN  uint32                 mc_group_size,    /* number of group replications (size of mc_group) */
    DNX_SAND_OUT DNX_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(jer2_qax_mult_ing_group_set(unit, multicast_id_ndx, (DNX_SAND_IN soc_multicast_replication_t *)mc_group, mc_group_size, TRUE, FALSE, out_err));
exit:
  DNXC_FUNC_RETURN;
}


/*
 * Closes the ingress multicast group, freeing its linked list.
 * Do nothing if the group is not open.
 */
uint32 jer2_qax_mult_ing_group_close(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t    mcid /* group mcid to close */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    DNX_TMC_ERROR out_err;
    uint8 does_exist;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_qax_mult_does_group_exist(unit, mcid, FALSE, &does_exist));
    if (does_exist) { /* do nothing if not open */
        DNXC_IF_ERR_EXIT(jer2_qax_mult_ing_group_set(unit, mcid, 0, 0, FALSE, TRUE, &out_err)); /* empty the group and mark it does not exist */
        if (out_err) { /* should never require more entries, so if this happens it is an internal error */
            DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
        }

        if (mcid != DNX_MC_EGRESS_LINK_PTR_END) { /* free the group's starting entry, this also marks the group as closed */
            DNXC_IF_ERR_EXIT(jer2_qax_mcds_build_free_blocks(
              unit, mcds, mcid, mcid, &mcds->ingress_starts, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
        } else {
            JER2_QAX_MCDS_SET_TYPE(mcds, DNX_MC_EGRESS_LINK_PTR_END, DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START); /* mark group as not existing */ 
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This API sets the existing ingress group to the given replications, configuring its linked list.
 * The group must exist.
 */
uint32 jer2_qax_mult_ing_group_update(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            mcid,          /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *mc_group,     /* group replications to set */
    DNX_SAND_IN  uint32                 mc_group_size, /* number of group replications (size of mc_group) */
    DNX_SAND_OUT DNX_TMC_ERROR          *out_err       /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(jer2_qax_mult_ing_group_set(
    unit, mcid, (DNX_SAND_IN soc_multicast_replication_t *)mc_group, mc_group_size, FALSE, FALSE, out_err));
exit:
  DNXC_FUNC_RETURN;
}


/*
 * Adds the given replications to the existing ingress multicast group.
 * It is an error if the group is not open.
 */
uint32 jer2_qax_mult_ing_add_replications(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  dnx_mc_id_t                 mcid,     /* group mcid */
    DNX_SAND_IN  uint32                      nof_reps, /* number of replications to add */
    DNX_SAND_IN  soc_multicast_replication_t *reps,    /* input array containing replications to add */
    DNX_SAND_OUT DNX_TMC_ERROR               *out_err  /* return possible errors that the caller may want to ignore : insufficient memory or duplicate replication */
)
{
    uint16 group_size;
    uint32 old_group_entries = JER2_QAX_MC_INGRESS_LINK_PTR_END; /* the linked list  of the previous group content */
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reps);
    if (JER2_QAX_MCDS_GET_TYPE(mcds, mcid) != DNX_MCDS_TYPE_VALUE_INGRESS_START) { /* if group is not open */
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }

    /* clear the replications in mcds and add the new replication */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_ingress_replications_from_arrays(unit, TRUE, nof_reps, reps));

    /* store current linked list */
    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER( /* store previous linked lists to be freed */
      mcds, unit, mcid, DNX_MCDS_TYPE_VALUE_INGRESS, &old_group_entries));

    /* add the replications of the current group into the mcds */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_group(unit, FALSE, mcid,
      DNX_MCDS_TYPE_VALUE_INGRESS, JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS - nof_reps, &group_size));
    if (group_size > JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS - nof_reps) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    }
    DNX_MC_ASSERT(mcds->nof_reps <= JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS);

    /* recreate the group with the extra replication by building the linked lists, including the first entry */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list(unit, DNX_MCDS_TYPE_VALUE_INGRESS_START, mcid,
      JER2_QAX_MC_INGRESS_LINK_PTR_END, 0, 0));
    if (mcds->out_err) { /* If failed, do some error code fixing */
        if (out_err != NULL) {
            if (mcds->out_err == _SHR_E_PARAM) {
                *out_err = _SHR_E_EXISTS;
            } else {
                *out_err = mcds->out_err;
            }
        }
    } else if (old_group_entries != JER2_QAX_MC_INGRESS_LINK_PTR_END) {
        /* If succeeded, free previous linked lists that are not used any more */
        DNXC_IF_ERR_EXIT(jer2_qax_mcdb_free_linked_list(unit, old_group_entries, DNX_MCDS_TYPE_VALUE_INGRESS));
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Removes the given replications from the existing ingress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 jer2_qax_mult_ing_remove_replications(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  dnx_mc_id_t                 mcid,     /* group mcid */
    DNX_SAND_IN  uint32                      nof_reps, /* number of replications to remove */
    DNX_SAND_IN  soc_multicast_replication_t *reps,    /* input array containing replications to add */
    DNX_SAND_OUT DNX_TMC_ERROR               *out_err  /* return possible errors that the caller may want to ignore : replication does not exist */
)
{
    jer2_qax_mcds_t *mcds = dnx_get_mcds(unit);
    uint16 group_size;
    uint32 old_group_entries = JER2_QAX_MC_INGRESS_LINK_PTR_END;
    dnx_mc_replication_t stack_reps[10], *dnx_reps = stack_reps;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reps);

    if (JER2_QAX_MCDS_GET_TYPE(mcds, mcid) != DNX_MCDS_TYPE_VALUE_INGRESS_START) { /* if group is not open */
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }

    /* get the replications of the group into the mcds */
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_group(unit, TRUE, mcid,
        DNX_MCDS_TYPE_VALUE_INGRESS, JER2_QAX_MULT_MAX_INGRESS_REPLICATIONS, &group_size));

    if (nof_reps > 10) { /* if stack reps is too small for the input, allocate memory for temporary dnx replications */
        dnx_reps = NULL;
        DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &dnx_reps, sizeof(*dnx_reps) * nof_reps, "dnxreps2remove"));
    }
    /* convert input replications */
    DNXC_IF_ERR_EXIT(jer2_qax_convert_soc2dnx_rep_array(unit, nof_reps, reps, dnx_reps));

    /* remove the given replication from the mcds */
    DNXC_IF_ERR_EXIT(jer2_qax_mult_remove_replications(unit, nof_reps, dnx_reps));
    if (mcds->out_err) { /* If the replication was not found, exit */
        SOC_EXIT;
    }

    /* recreate the group with the removed replications */
    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, mcid, DNX_MCDS_TYPE_VALUE_INGRESS, &old_group_entries)); /* store previous linked list to be freed */

    DNXC_IF_ERR_EXIT(jer2_qax_mcds_set_linked_list( /* build the group, including the first entry */
        unit, DNX_MCDS_TYPE_VALUE_INGRESS_START, mcid, JER2_QAX_MC_INGRESS_LINK_PTR_END, 0, 0));
    if (mcds->out_err) { /* If failed due to lack of free entries */
#ifndef JER2_QAX_EGRESS_MC_DELETE_FAILS_ON_FULL_MCDB
        /* If we can not reconstruct the group due to MCDB being full, just remove the replication/entry */
        DNXC_IF_ERR_EXIT(jer2_qax_mcds_remove_replications_from_group(unit, mcid, nof_reps, dnx_reps)); 
        mcds->out_err = _SHR_E_NONE;
#endif /* JER2_ARAD_EGRESS_MC_DELETE_FAILS_ON_FULL_MCDB */
    }
    if (mcds->out_err == _SHR_E_NONE && old_group_entries != JER2_QAX_MC_INGRESS_LINK_PTR_END) {
        /* free previous linked list of the group not used any more */
        DNXC_IF_ERR_EXIT(jer2_qax_mcdb_free_linked_list(unit, old_group_entries, DNX_MCDS_TYPE_VALUE_INGRESS));
    }

exit:
    if (out_err != NULL && _rv == SOC_E_NONE) {
        *out_err = mcds->out_err;
    }
    if (dnx_reps != stack_reps) {
        dnxc_free_mem_if_not_null(unit, (void*)&dnx_reps);
    }
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Returns the size of the multicast group with the
*     specified multicast id.
*********************************************************************/
uint32 jer2_qax_mult_ing_group_size_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx,
    DNX_SAND_OUT uint32      *mc_group_size
)
{
  uint8 is_open;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mc_group_size);

  DNXC_IF_ERR_EXIT(jer2_qax_mult_ing_get_group(unit, multicast_id_ndx, 0, 0, 0, mc_group_size, &is_open));

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Gets the ingress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact
 * The group's replication number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 jer2_qax_mult_ing_get_group(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t group_mcid,           /* group id */
    DNX_SAND_IN  uint32      mc_group_size,        /* maximum replications to return */
    DNX_SAND_OUT soc_gport_t *ports,               /* output ports (array of size mc_group_size) */
    DNX_SAND_OUT soc_if_t    *cuds,                /* output ports (array of size mc_group_size) */
    DNX_SAND_OUT uint32      *exact_mc_group_size, /* the number of replications in the group will be returned here */
    DNX_SAND_OUT uint8       *is_open              /* will return if the group is open (false or true) */
)
{
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(exact_mc_group_size);
  DNXC_NULL_CHECK(is_open);
  if (mc_group_size && (!ports || !cuds)) { /* we check that the output data pointers are not null if we need to return data */
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("NULL pointer")));
  }


  DNXC_IF_ERR_EXIT(jer2_qax_mult_does_group_exist(unit, group_mcid, FALSE, is_open));
  if (*is_open) {
    uint16 group_size;
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_get_group(
      unit, TRUE, group_mcid, DNX_MCDS_TYPE_VALUE_INGRESS, mc_group_size, &group_size));
    *exact_mc_group_size = group_size;
    DNXC_IF_ERR_EXIT(jer2_qax_mcds_copy_replications_to_arrays(unit, 0, mc_group_size, ports, cuds, 0));
  } else { /* group is not open */
    *exact_mc_group_size = 0;
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
* Return the contents of the MCDB hardware entry, for reconstruction in case of SER.
*/
soc_error_t jer2_qax_mult_get_entry(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 mcdb_index,
    DNX_SAND_OUT uint32 *entry /* output MCDB entry */
)
{
    const jer2_qax_mcds_t* mcds = dnx_get_mcds(unit);
    jer2_qax_mcdb_entry_t *mcdb_entry = JER2_QAX_GET_MCDB_ENTRY(mcds, mcdb_index);
    DNXC_INIT_FUNC_DEFS;

    if (mcdb_index >= JER2_QAX_NOF_MCDB_ENTRIES) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCDB entry %u is out of range"), mcdb_index));
    }
    if (DNX_MCDS_TYPE_IS_USED(JER2_QAX_MCDS_ENTRY_GET_TYPE(mcdb_entry))) { /* relocate conflicting entries if needed */
        entry[0] = mcdb_entry->word0;
        entry[1] = mcdb_entry->word1;
        entry[2] = mcdb_entry->word2 & JER2_QAX_MC_ENTRY_MASK_VAL;
    } else {
        entry[0] = JER2_QAX_MC_FREE_ENTRY_0;
        entry[1] = JER2_QAX_MC_FREE_ENTRY_1;
        entry[2] = JER2_QAX_MC_FREE_ENTRY_2;
    }

exit:
    DNXC_FUNC_RETURN;
}


