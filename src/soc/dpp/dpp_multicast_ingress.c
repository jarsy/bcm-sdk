/* $Id: dpp_multicast_ingress.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

#include <soc/dpp/multicast_imp.h>
#include <soc/dcmn/error.h>


#include <soc/mcm/memregs.h>


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_multicast_ingress.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{ /* the values are derived from hardware (IDR_IRDBm) */
  irdb_value_group_closed     = 0, /* The group is closed */
  irdb_value_group_open       = 1, /* The group is open and its replications do not fit in a mini multicast buffer */
  irdb_value_group_open_mmc   = 3, /* The group is open and its replications fit in a mini multicast buffer */
  irdb_value_group_max_value  = 3  /* The maximum allowed value for the fields, and their mask */
} irdb_value_t;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */


/*
 * This function writes the ingress format hardware fields to a arad_mcdb_entry_t structure.
 * The input is the replication data (destination and outlif) and a pointer to next entry.
 */
uint32 dpp_ing_mc_group_entry_to_mcdb_entry(
    SOC_SAND_IN    int                    unit,
    SOC_SAND_INOUT dpp_mcdb_entry_t       *mcdb_entry, /* mcdb to write to */
    SOC_SAND_IN    SOC_TMC_MULT_ING_ENTRY *ing_entry,  /* replication data */
    SOC_SAND_IN    uint32                 next_entry   /* the next entry */
);

/*
 * given a mcid of an ingress group, configures the ingress properties of the group in IDR_IRDBm.
 * Given the enum, two hardware bits are set:
 *  1. If the ingress group is to be replicated. We use it to note if it is open and with at leat one replication.
 *  2. If the group uses a unicast buffer - has no more than one replication.
 */
uint32 dpp_mult_properties_set_unsafe(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32  mcid,
    SOC_SAND_IN  irdb_value_t value /* properties value to set */
)
{
  uint32 entry[2] = {0};
  const unsigned bit_offset = 2 * (mcid % 16);
  const int table_index = mcid / 16;
  irdb_value_t value_internal;

  SOCDNX_INIT_FUNC_DEFS;

  /* SDK-112054 - MINI multicast configuration is not allowed */
  value_internal = value;
  if (value == irdb_value_group_open_mmc) {
      value_internal = irdb_value_group_open;
  }

  if (mcid >= SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("MCID out of range")));
  }

  SOCDNX_IF_ERR_EXIT(READ_IDR_IRDBm(unit, MEM_BLOCK_ANY, table_index, entry));

  if (((*entry >> bit_offset) & irdb_value_group_max_value) != value_internal) { /* If setting a different value, update hardware */
    *entry &= ~(((uint32)irdb_value_group_max_value) << bit_offset);
    *entry |= (((uint32)value_internal) << bit_offset);
    SOCDNX_IF_ERR_EXIT(WRITE_IDR_IRDBm(unit, MEM_BLOCK_ANY, table_index, entry));
  }
exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
 *  This function gets ingress properties for the given MC ID:
 *  1. If the multicast id is to be handled in ingress (not only by the fabric).
 *  2. If the multicast id is marked to be replicated no more than once in the ingress.
 *********************************************************************/
/*
 * given a mcid of an ingress group, gets the ingress properties configuration of the group in IDR_IRDBm.
 * Given the enum, two hardware bits are checked:
 *  1. If the ingress group is to be replicated. We use it to note if it is open and with at least one replication.
 *  2. If the group uses a unicast buffer - has no more than one replication.
 */
uint32 dpp_mult_properties_get_unsafe(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32  mcid,
    SOC_SAND_OUT irdb_value_t *value /* properties value to set */
)
{
  uint32 entry[2];
  const unsigned bit_offset = 2 * (mcid % 16);
  const int table_index = mcid / 16;

  SOCDNX_INIT_FUNC_DEFS;
  if (mcid >= SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("MCID out of range")));
  }
  SOCDNX_NULL_CHECK(value);

  SOCDNX_IF_ERR_EXIT(READ_IDR_IRDBm(unit, MEM_BLOCK_ANY, table_index, entry));

  *value = ((*entry) >> bit_offset) & irdb_value_group_max_value;

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * This API sets the ingress group to the given replications,
 * configuring its linked list.
 * If the group does not exist, it will be created or an error will be returned based on allow_create.
 * Creation may involve relocating the mcdb entry which will be the start
 * of the group, and possibly other consecutive entries.
 * Creation enables the group in IDR_IRDBm.
 *
 * This function also configures the table which indicates
 * per Multicast ID whether to perform ingress replication
 * or not, if ingress replication is not chosen, fabric or
 * egress multicast will be performed on the packet.
 */
uint32 dpp_mult_ing_group_set(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  dpp_mc_id_t         multicast_id_ndx,
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY  *mc_group,
    SOC_SAND_IN  uint32               mc_group_size,
    SOC_SAND_IN  uint8                allow_create, /* if non zero, will create the group if it does not exist */
    SOC_SAND_OUT SOC_TMC_ERROR        *out_err      /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  int group_exists = 0, group_start_alloced = 0, failed = 1;
  uint32 group_entry_type;
  uint32 nof_entries_needed, remaining_group_entries = mc_group_size;
  uint32 old_group_entries = MCDS_INGRESS_LINK_END(mcds), linked_list = MCDS_INGRESS_LINK_END(mcds);
  uint32 block_start = 0;
  dpp_free_entries_block_size_t block_size = 0;
  arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, multicast_id_ndx);
  arad_mcdb_entry_t start_entry = {0};
  irdb_value_t prev_prop_value, new_prop_value;

  SOCDNX_INIT_FUNC_DEFS;
  if (mc_group_size) {
    SOCDNX_NULL_CHECK(mc_group);
    if (mc_group_size > mcds->max_nof_ingr_replications) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("ingress MC group size is too big")));
    }
  }
  SOCDNX_NULL_CHECK(out_err);
  DPP_MC_ASSERT(multicast_id_ndx < SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids);


  group_entry_type = DPP_MCDS_GET_TYPE(mcds, multicast_id_ndx);
  group_exists = group_entry_type == DPP_MCDS_TYPE_VALUE_INGRESS_START;
  if (!group_exists && !allow_create) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("MC group is not created")));
  }

  /* If this is a new group, we need to reserve its first entry, and possibly relocate the entry there */
  if (!group_exists && multicast_id_ndx != DPP_MC_EGRESS_LINK_PTR_END) {
    if (DPP_MCDS_TYPE_IS_USED(group_entry_type)) { /* relocate conflicting entries if needed */
      SOCDNX_IF_ERR_EXIT(dpp_mcdb_relocate_entries(unit, multicast_id_ndx, 0, 0, out_err));
      if (*out_err) { /* If failed due to lack of memories, do the same */
        SOC_EXIT;
      }
    } else { /* just allocate the start entry of the group */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_reserve_group_start(mcds, multicast_id_ndx));
      DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, DPP_MCDS_TYPE_VALUE_INGRESS_START); /* mark the first group entry as the start of an ingress group */
    }
    group_start_alloced = 1;
  }

  /* calculate the needed free entries */
  nof_entries_needed = mc_group_size;
  if (mc_group_size) { /* first group entry does not need to be allocated */
    --nof_entries_needed;
  }
  /* Now we have the number of free entries that we still need for the operation to succeed */
  if (nof_entries_needed > dpp_mcds_unoccupied_get(mcds)) { /* check if we have enough free entries for the operation */
    *out_err = _SHR_E_FULL;
    SOC_EXIT;
  }
  *out_err = _SHR_E_NONE;

  /* build first group entry for writing after building all the linked list */
  if (mc_group_size) { /* set first group entry for a non empty group */
    SOCDNX_IF_ERR_EXIT(dpp_ing_mc_group_entry_to_mcdb_entry(unit, &start_entry, mc_group, 0));
    --remaining_group_entries;
  } else { /* store an empty entry for an empty group */
    start_entry.word0 = mcds->empty_ingr_value[0];
    start_entry.word1 = mcds->empty_ingr_value[1];
  }

  /* build the linked list except for the first entry */
  while (remaining_group_entries) { /* write the group entries except for the first */
    dpp_free_entries_block_size_t max_block_size = DPP_MCDS_MAX_FREE_BLOCK_SIZE;
    uint32 cur_entry, next_entry = linked_list;
    if (remaining_group_entries < DPP_MCDS_MAX_FREE_BLOCK_SIZE) {
      max_block_size = remaining_group_entries;
    }
    SOCDNX_IF_ERR_EXIT( /* allocate free entries */
      dpp_mcds_get_free_entries_block(mcds, DPP_MCDS_GET_FREE_BLOCKS_PREFER_SMALL,
        1, max_block_size, &block_start, &block_size)); /* allocation should not fail according to the needed entries calculation */
    DPP_MC_ASSERT(block_size > 0 && block_size <= max_block_size && remaining_group_entries >= block_size);

    for (cur_entry = block_start + block_size; cur_entry > block_start; ) { /* fill the allocated blocks with entries */
      --cur_entry;
      DPP_MC_ASSERT(remaining_group_entries);
      SOCDNX_IF_ERR_EXIT(dpp_mult_ing_multicast_group_entry_to_tbl( /* update and write the current entry */
        unit, cur_entry, mc_group + (remaining_group_entries--), next_entry,
        (cur_entry == block_start ? multicast_id_ndx : cur_entry - 1)));
      next_entry = cur_entry;
    } /* finished filling block */

    if (linked_list != MCDS_INGRESS_LINK_END(mcds)) { /* set software back link from the previously generated linked list to the last entry in this block */
      DPP_MCDS_SET_PREV_ENTRY(mcds, linked_list, block_start + block_size - 1);
    }
    linked_list = block_start; /* update the built linked list to start with this block, also used for cleanup on errors */
    block_size = 0; /* do not clean currently allocated block on error, it is cleaned up with the linked list */
  }

  /* finished writing the linked list except for the first group entry */

  if (group_exists) { /* keep the old list for freeing */
    old_group_entries = soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf);
  }

  /* build and write the first group entry to activate the newly written linked list */
  mcdb_entry->word0 = start_entry.word0; /* copy previously filled hardware fields (next pinter not set) */
  mcdb_entry->word1 &= DPP_MCDS_WORD1_KEEP_BITS_MASK;
  mcdb_entry->word1 |= start_entry.word1;
  DPP_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, multicast_id_ndx); /* set software link to previous entry */
  DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, DPP_MCDS_TYPE_VALUE_INGRESS_START); /* set type */

  /* calculate group properties - existing and new required ones.
   * A change of small group to a big one must be done before activating the change in the mcdb.
   * This is to prevent a small time where a group with > 1 replications uses a unicast buffer.
   */
  new_prop_value = mc_group_size > mcds->max_nof_mmc_replications ? irdb_value_group_open : irdb_value_group_open_mmc;
  if (group_exists) {
    SOCDNX_IF_ERR_EXIT(dpp_mult_properties_get_unsafe(unit, multicast_id_ndx, &prev_prop_value));
    if (mc_group_size > mcds->max_nof_mmc_replications && prev_prop_value != irdb_value_group_open) { /* need to make the change now */
      DPP_MC_ASSERT(new_prop_value == irdb_value_group_open);
      SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe(unit, multicast_id_ndx, new_prop_value));
      prev_prop_value = new_prop_value;
    }
  } else {
    prev_prop_value = irdb_value_group_closed;
  }

  SOCDNX_IF_ERR_EXIT(MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
    mcds, unit, multicast_id_ndx, DPP_MCDS_TYPE_VALUE_INGRESS_START, linked_list));
  linked_list = MCDS_INGRESS_LINK_END(mcds); /* prevent the freeing of the linked list on error */

  if (old_group_entries != MCDS_INGRESS_LINK_END(mcds)) { /* free old linked list of the group not used any more */
    SOCDNX_IF_ERR_EXIT(dpp_mcdb_free_linked_list(
      unit, old_group_entries, DPP_MCDS_TYPE_VALUE_INGRESS));
  }

  if (prev_prop_value != new_prop_value) { /* set ingress group properties for the new group, if needed */
    SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe(unit, multicast_id_ndx, new_prop_value));
  }

  failed = 0;
exit:
  if (linked_list != MCDS_INGRESS_LINK_END(mcds)) { /* free the linked list allocated so far on error */
    DPP_MC_ASSERT(failed);
    dpp_mcdb_free_linked_list(unit, linked_list, DPP_MCDS_TYPE_VALUE_INGRESS);
  }
  if (block_size) { /* free the current block on error */
    DPP_MC_ASSERT(failed);
    dpp_mcds_build_free_blocks(unit, mcds,
      block_start, block_start + block_size - 1, dpp_mcds_get_region(mcds, block_start), McdsFreeBuildBlocksAdd_AllMustBeUsed);
  }
  if (group_start_alloced && failed) { /* free group start entry if needed */
    DPP_MC_ASSERT(!group_exists);
    dpp_mcds_build_free_blocks(unit, mcds,
      multicast_id_ndx, multicast_id_ndx, dpp_mcds_get_region(mcds, multicast_id_ndx), McdsFreeBuildBlocksAdd_AllMustBeUsed);
  }

  SOCDNX_FUNC_RETURN;
}


/*
 * This API sets the ingress group to the given replications,
 * configuring its linked list; and creates the group if it did not exist.
 */
uint32 dpp_mult_ing_group_open(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *mc_group,        /* group replications to set */
    SOC_SAND_IN  uint32                 mc_group_size,    /* number of group replications (size of mc_group) */
    SOC_SAND_OUT SOC_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  SOCDNX_INIT_FUNC_DEFS;

  SOCDNX_IF_ERR_EXIT(dpp_mult_ing_group_set(
    unit, multicast_id_ndx, mc_group, mc_group_size, TRUE, out_err));
exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * Gets the ingress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact
 * The group's replication number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 dpp_mult_ing_get_group(
    SOC_SAND_IN  int         unit,
    SOC_SAND_IN  dpp_mc_id_t group_mcid,           /* group id */
    SOC_SAND_IN  uint32      mc_group_size,        /* maximum replications to return */
    SOC_SAND_OUT soc_gport_t *ports,               /* output ports (array of size mc_group_size) */
    SOC_SAND_OUT soc_if_t    *cuds,                /* output ports (array of size mc_group_size) */
    SOC_SAND_OUT uint32      *exact_mc_group_size, /* the number of replications in the group will be returned here */
    SOC_SAND_OUT uint8       *is_open              /* will return if the group is open (false or true) */
)
{
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(exact_mc_group_size);
  SOCDNX_NULL_CHECK(is_open);
  if (mc_group_size && (!ports || !cuds)) { /* we check that the output data pointers are not null if we need to return data */
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("NULL pointer")));
  }


  SOCDNX_IF_ERR_EXIT(dpp_mult_does_group_exist(unit, group_mcid, FALSE, is_open));
  if (*is_open) {
    uint16 group_size;
    SOCDNX_IF_ERR_EXIT(dpp_mcds_get_group(
      unit, DPP_MC_CORE_BITAMAP_CORE_0, TRUE, TRUE, group_mcid, DPP_MCDS_TYPE_VALUE_INGRESS, mc_group_size, &group_size));
    *exact_mc_group_size = group_size;
    SOCDNX_IF_ERR_EXIT(dpp_mcds_copy_replications_to_arrays(unit, 0, mc_group_size, ports, cuds, 0));
  } else { /* group is not open */
    *exact_mc_group_size = 0;
  }

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * This API sets the ingress group to the given replications, configuring its linked list.
 * The group must exist.
 */
uint32 dpp_mult_ing_group_update(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  dpp_mc_id_t          multicast_id_ndx,    /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY   *mc_group,           /* group replications to set */
    SOC_SAND_IN  uint32                mc_group_size,       /* number of group replications (size of mc_group) */
    SOC_SAND_OUT SOC_TMC_ERROR         *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  SOCDNX_INIT_FUNC_DEFS;

  SOCDNX_IF_ERR_EXIT(dpp_mult_ing_group_set(
    unit, multicast_id_ndx, mc_group, mc_group_size, FALSE, out_err));
exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Closes the ingress muticast group, freeing its linked list.
 * Do nothing if the group is not open.
 */
uint32 dpp_mult_ing_group_close(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  dpp_mc_id_t      multicast_id_ndx /* group mcid to close */
)
{
  uint8 does_exist;
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_IF_ERR_EXIT(dpp_mult_does_group_exist(unit, multicast_id_ndx, FALSE, &does_exist));
  if (does_exist) { /* do nothing if not open */
    dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
    SOC_TMC_ERROR out_err;

    SOCDNX_IF_ERR_EXIT(dpp_mult_ing_group_set( /* empty the group */
      unit, multicast_id_ndx, 0, 0, FALSE, &out_err));
    if (out_err) { /* should never require more entries, so if this happens it is an internal error */
      SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
    }
    SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_closed));

    if (multicast_id_ndx != DPP_MC_EGRESS_LINK_PTR_END) { /* free the group's starting entry, this also marks the group as closed */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks(
        unit, mcds, multicast_id_ndx, multicast_id_ndx, dpp_mcds_get_region(mcds, multicast_id_ndx), McdsFreeBuildBlocksAdd_AllMustBeUsed));
    } else { /* treat the non allocable first entry specifically */
      arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, DPP_MC_EGRESS_LINK_PTR_END);
      mcdb_entry->word0 = ((dpp_mcds_base_t*)mcds)->free_value[0];
      mcdb_entry->word1 &= DPP_MCDS_WORD1_KEEP_BITS_MASK;
      mcdb_entry->word1 |= ((dpp_mcds_base_t*)mcds)->free_value[1];
      SOCDNX_IF_ERR_EXIT(dpp_mcds_write_entry(unit, DPP_MC_EGRESS_LINK_PTR_END)); /* write to hardware */
    }
  }

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * Traverse an ingress linked list till the given replication is found, or till end of MC group.
 * If more than the given maximum number of entries is traversed, an error is returned.
 * If found the entry containing the replication is returned, otherwise the last entry in the linked list is returned.
 */

uint32 dpp_mult_traverse_ingress_list(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32            list_start,   /* the mcdb index to start traversing from */
    SOC_SAND_IN  arad_mcdb_entry_t *replication, /* replication data (outlif, dest) to search for. If other bits are on, no match will be made */
    SOC_SAND_IN  uint16            max_size,     /* it is an error to traverse more than this number of entries */
    SOC_SAND_OUT uint16            *found_size,  /* the number of entries traversed */
    SOC_SAND_OUT uint8             *was_found,   /* boolean - returns non zero if the replication was found */
    SOC_SAND_OUT uint32            *found_entry  /* the index of the found/last entry */
)
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  uint32 index = list_start;
  uint16 list_size = 1;
  SOCDNX_INIT_FUNC_DEFS;

  SOCDNX_NULL_CHECK(replication);
  SOCDNX_NULL_CHECK(found_size);
  SOCDNX_NULL_CHECK(found_entry);
  DPP_MC_ASSERT(DPP_MCDS_TYPE_IS_INGRESS(DPP_MCDS_GET_TYPE(mcds, list_start)) || list_start == MCDS_INGRESS_LINK_END(mcds));

  for (;; ++list_size) {
    arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, index);
    uint32 next_entry;

    if (list_size > max_size) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("passed max allowed entries number to traverse")));
    }
    /* try to see if the current entry is a match to the searched one */
    if (replication->word0 ==  mcdb_entry->word0 &&
        replication->word1 == (mcdb_entry->word1 & mcds->ingr_word1_replication_mask)) {
      *was_found = TRUE;
      break;
    }
    next_entry = soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf);
    if (next_entry == MCDS_INGRESS_LINK_END(mcds)) { /* is this the last MC group entry? */
      *was_found = FALSE;
      break;
    }
    index = next_entry;
    DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, next_entry) == DPP_MCDS_TYPE_VALUE_INGRESS);
  }
  *found_size = list_size;
  *found_entry = index;

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Adds the given replication to the ingress multicast group.
 * It is an error if the group is not open.
 */
uint32 dpp_mult_ing_destination_add(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *replication,     /* replication to add */
    SOC_SAND_OUT SOC_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore : insufficient memory or duplicate replication */
)
{

  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *work_entry = MCDS_GET_MCDB_ENTRY(mcds, multicast_id_ndx);
  uint32 next_index, add_index, block_start;
  uint16 found_size;
  dpp_free_entries_block_size_t block_size;
  uint8 does_exist;
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(out_err);

  SOCDNX_IF_ERR_EXIT(dpp_mult_does_group_exist(unit, multicast_id_ndx, FALSE, &does_exist));
  if (!does_exist) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("MC group is not created")));
  }
  DPP_MC_ASSERT(DPP_MCDS_ENTRY_GET_TYPE(work_entry) == DPP_MCDS_TYPE_VALUE_INGRESS_START);

  *out_err = _SHR_E_NONE;
  next_index = soc_mem_field32_get(unit, IRR_MCDBm, work_entry, LINK_PTRf);
  if (soc_mem_field32_get(unit, IRR_MCDBm, work_entry, DESTINATIONf) == DPP_MC_ING_DESTINATION_DISABLED) { /* if the first entry is disabled */
    DPP_MC_ASSERT(next_index == MCDS_INGRESS_LINK_END(mcds));
    /* We can add the replication to the empty first entry of the group */
#ifdef DPP_INGR_MC_PERFORM_UNNEEDED_UPDATES
    SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_open_mmc)); /* update properties before we update the MCDB */
#endif
    SOCDNX_IF_ERR_EXIT(dpp_mult_ing_multicast_group_entry_to_tbl( /* update and write the start entry */
      unit, multicast_id_ndx, replication, next_index, multicast_id_ndx));
    SOC_EXIT;
  }

  if (!(SOC_DPP_CONFIG(unit)->tm.mc_mode & DPP_MC_ALLOW_DUPLICATES_MODE)) { /* need to check duplicate replications */

    arad_mcdb_entry_t replication_entry = {0};
    uint8 was_found = 0;
    SOCDNX_IF_ERR_EXIT(dpp_ing_mc_group_entry_to_mcdb_entry(unit, &replication_entry, replication, 0));

    /* search for the replication or end of group or group too big */
    SOCDNX_IF_ERR_EXIT(dpp_mult_traverse_ingress_list(unit, multicast_id_ndx, &replication_entry,
      mcds->max_nof_ingr_replications, &found_size, &was_found, &add_index));
    if (was_found) {
      *out_err = _SHR_E_EXISTS; /* replication is already in the group */
      SOC_EXIT;
    }

  } else { /* no need to check duplicate replications, but need to check the group size for buffer type decision */
    /* calc the new group size (sizes above mcds->max_nof_mmc_replications are the same for our needs, so no need to continue) */
    for (found_size = 1; found_size < mcds->max_nof_ingr_replications && next_index != MCDS_INGRESS_LINK_END(mcds); ++found_size) {
      next_index = soc_mem_field32_get(unit, IRR_MCDBm, MCDS_GET_MCDB_ENTRY(mcds, next_index), LINK_PTRf);
    }
  }
    if (found_size >= mcds->max_nof_ingr_replications) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("ingress MC group size is too big to add to")));
    }

  add_index = multicast_id_ndx; /* always add replication after the start of the linked list */

  /* allocate an entry and add it after entry add_index */
  work_entry = MCDS_GET_MCDB_ENTRY(mcds, add_index);
  next_index = soc_mem_field32_get(unit, IRR_MCDBm, work_entry, LINK_PTRf);
  SOCDNX_IF_ERR_EXIT(dpp_mcds_get_free_entries_block( /* allocate free entry */
    mcds, DPP_MCDS_GET_FREE_BLOCKS_DONT_FAIL,
    1, 1, &block_start, &block_size));
  if (block_size != 1) { /* could not get free entries */
    *out_err = _SHR_E_FULL;
    DPP_MC_ASSERT(!block_size);
    SOC_EXIT;
  }

  SOCDNX_IF_ERR_EXIT(dpp_mult_ing_multicast_group_entry_to_tbl( /* update and write the allocated entry with the new replication */
    unit, block_start, replication, next_index, add_index));

  if (next_index != MCDS_INGRESS_LINK_END(mcds)) {
    DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, next_index) == DPP_MCDS_TYPE_VALUE_INGRESS);
    DPP_MCDS_SET_PREV_ENTRY(mcds, next_index, block_start); /* make entry next_index point back to new entry */
  }

  SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe( /* update properties before we update the MCDB */
    unit, multicast_id_ndx, found_size >= mcds->max_nof_mmc_replications ? irdb_value_group_open : irdb_value_group_open_mmc));

  SOCDNX_IF_ERR_EXIT(MCDS_SET_NEXT_POINTER( /* set next pointer and write to the add_index entry, to activate the change */
    mcds, unit, add_index, DPP_MCDS_TYPE_VALUE_INGRESS, block_start));

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Removes the given replication from the ingress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 dpp_mult_ing_destination_remove(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* group mcid */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *entry,           /* replication to remove */
    SOC_SAND_OUT SOC_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore : replication does not exist */
)
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t replication_entry = {0}, *found_entry;
  uint16 found_size;
  uint32 found_index, next_index, prev_index, freed_index;
  uint8 was_found;
  uint8 does_exist;
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(entry);

  SOCDNX_IF_ERR_EXIT(dpp_mult_does_group_exist(unit, multicast_id_ndx, FALSE, &does_exist));
  if (!does_exist) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_SOCDNX_MSG("MC group is not created")));
  }

  SOCDNX_IF_ERR_EXIT(dpp_ing_mc_group_entry_to_mcdb_entry(unit, &replication_entry, entry, 0));

  SOCDNX_IF_ERR_EXIT(dpp_mult_traverse_ingress_list(
    unit, multicast_id_ndx, &replication_entry, mcds->max_nof_ingr_replications,
    &found_size, &was_found, &found_index));
  if (!was_found) {
    *out_err = _SHR_E_NOT_FOUND; /* replication was not found */
    SOC_EXIT;
  }
  *out_err = _SHR_E_NONE;

  found_entry = MCDS_GET_MCDB_ENTRY(mcds, found_index);
  next_index = soc_mem_field32_get(unit, IRR_MCDBm, found_entry, LINK_PTRf);
  prev_index = DPP_MCDS_GET_PREV_ENTRY(mcds, found_index);

  if (found_index == multicast_id_ndx) { /* the first replication/entry in the group is to be removed (not removing the first group entry) */

    DPP_MC_ASSERT(prev_index == multicast_id_ndx && found_size == 1);
    if (next_index == MCDS_INGRESS_LINK_END(mcds)) { /* this is the only entry in the group, just update it */
      found_entry->word0 = ((dpp_mcds_base_t*)mcds)->empty_ingr_value[0];
      found_entry->word1 &= ~mcds->msb_word_mask;
      found_entry->word1 |= ((dpp_mcds_base_t*)mcds)->empty_ingr_value[1];
      SOCDNX_IF_ERR_EXIT(dpp_mcds_write_entry(unit, found_index)); /* write group start entry to hardware */
#ifdef DPP_INGR_MC_PERFORM_UNNEEDED_UPDATES /* update group properties after we update the MCDB */
      SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_open_mmc));
#endif
      SOC_EXIT;
    } else { /* This group has a second entry, which will be copied to the first and removed */
      arad_mcdb_entry_t *entry2 = MCDS_GET_MCDB_ENTRY(mcds, next_index);
      DPP_MC_ASSERT(DPP_MCDS_ENTRY_GET_TYPE(entry2) == DPP_MCDS_TYPE_VALUE_INGRESS);
      found_entry->word0 = entry2->word0;
      found_entry->word1 &= ~mcds->msb_word_mask;
      found_entry->word1 |= (entry2->word1 & mcds->msb_word_mask);
      freed_index = next_index;
      next_index = soc_mem_field32_get(unit, IRR_MCDBm, entry2, LINK_PTRf); /* select entry to update the back_pointer in */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_write_entry(unit, found_index)); /* write group start entry to hardware */
    }

  } else { /* the replication entry is not the start of the group, it will be removed */

    DPP_MC_ASSERT(found_size > 2  ? (prev_index != multicast_id_ndx) : (found_size == 2 && prev_index == multicast_id_ndx));
    /* remove found_index from the linked list by making its preceding entry point to its succeeding entry */
    SOCDNX_IF_ERR_EXIT(MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
      mcds, unit, prev_index, DPP_MCDS_TYPE_VALUE_INGRESS, next_index));
    freed_index = found_index;
    --found_size; /* found size represents the group size till prev_index */

  }

  if (next_index != MCDS_INGRESS_LINK_END(mcds)) {
    DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, next_index) == DPP_MCDS_TYPE_VALUE_INGRESS);
    DPP_MCDS_SET_PREV_ENTRY(mcds, next_index, prev_index); /* make entry next_index point back to prev_index */
    /* calc the new group size (sizes above mcds->max_nof_mmc_replications are the same for our needs, so no need to continue) */
    for (; found_size <= mcds->max_nof_mmc_replications && next_index != MCDS_INGRESS_LINK_END(mcds); ++found_size) {
      next_index = soc_mem_field32_get(unit, IRR_MCDBm, MCDS_GET_MCDB_ENTRY(mcds, next_index), LINK_PTRf);
    }
  }

  if (found_size == mcds->max_nof_mmc_replications) { /* update group properties after we update the MCDB */
    SOCDNX_IF_ERR_EXIT(dpp_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_open_mmc));
  }

  SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks( /* free the entry freed_index */
    unit, mcds, freed_index, freed_index, dpp_mcds_get_region(mcds, freed_index), McdsFreeBuildBlocksAdd_AllMustBeUsed));

  /* update group properties if needed */

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Returns the size of the multicast group with the
*     specified multicast id.
*********************************************************************/
uint32 dpp_mult_ing_group_size_get(
    SOC_SAND_IN  int         unit,
    SOC_SAND_IN  dpp_mc_id_t multicast_id_ndx,
    SOC_SAND_OUT uint32      *mc_group_size
)
{
  uint8 is_open;
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(mc_group_size);

  SOCDNX_IF_ERR_EXIT(dpp_mult_ing_get_group(
    unit, multicast_id_ndx, 0, 0, 0, mc_group_size, &is_open));

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * This function writes ingress format to a mcds mcdb entry and then to hardware the input.
 * The input is the replication data (destination and outlif), pointer to next entry,
 * and the pointer to the previous entry.
 * Receives the replication data already processed, compared to dpp_mult_ing_multicast_group_entry_to_tbl.
 */
uint32 dpp_mult_ing_multicast_set_entry(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  dpp_mc_id_t         multicast_id_ndx, /* mcdb to write to */
    SOC_SAND_IN  uint32               outlif,           /* outlif replication data */
    SOC_SAND_IN  uint32               destination,      /* destination replication data */
    SOC_SAND_IN  uint32               next_entry,       /* the next entry */
    SOC_SAND_IN  uint32               prev_entry        /* the previous entry written only to mcds */
)

{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *mcdb_entry = dpp_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  SOCDNX_INIT_FUNC_DEFS;

  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, OUT_LIFf, outlif);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, DESTINATIONf, destination);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf, next_entry);
  SOCDNX_IF_ERR_EXIT(dpp_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DPP_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DPP_MCDS_TYPE_VALUE_INGRESS_START : DPP_MCDS_TYPE_VALUE_INGRESS);

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * This function writes the ingress format hardware fields to a arad_mcdb_entry_t structure.
 * The input is the replication data (destination and outlif) and a pointer to next entry.
 */
uint32 dpp_ing_mc_group_entry_to_mcdb_entry(
    SOC_SAND_IN    int                    unit,
    SOC_SAND_INOUT dpp_mcdb_entry_t       *mcdb_entry, /* mcdb to write to */
    SOC_SAND_IN    SOC_TMC_MULT_ING_ENTRY *ing_entry,  /* replication data */
    SOC_SAND_IN    uint32                 next_entry   /* the next entry */
)
{
  SOCDNX_INIT_FUNC_DEFS;

  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, OUT_LIFf, ing_entry->cud);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, DESTINATIONf, ing_entry->destination.id);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf, next_entry);

  SOCDNX_FUNC_RETURN;
}


/*
 * This function writes ingress format to a mcds mcdb entry and then to hardware the input.
 * The input is the replication data (destination and outlif), pointer to next entry,
 * and the pointer to the previous entry.
 */
uint32 dpp_mult_ing_multicast_group_entry_to_tbl(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  dpp_mc_id_t            multicast_id_ndx, /* mcdb to write to */
    SOC_SAND_IN  SOC_TMC_MULT_ING_ENTRY *ing_entry,       /* replication data */
    SOC_SAND_IN  uint32                 next_entry,       /* the next entry */
    SOC_SAND_IN  uint32                 prev_entry        /* the previous entry written only to mcds */
)
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *mcdb_entry = dpp_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(ing_entry);

  /* write the hardware fields to memory */
  SOCDNX_IF_ERR_EXIT(dpp_ing_mc_group_entry_to_mcdb_entry(unit, mcdb_entry, ing_entry, next_entry));

  SOCDNX_IF_ERR_EXIT(dpp_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DPP_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DPP_MCDS_TYPE_VALUE_INGRESS_START : DPP_MCDS_TYPE_VALUE_INGRESS);

exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 dpp_mult_ing_traffic_class_map_set(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_TMC_MULT_ING_TR_CLS_MAP *map
)
{
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("unsupported for the device")));
exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 dpp_mult_ing_traffic_class_map_get(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_OUT SOC_TMC_MULT_ING_TR_CLS_MAP *map
)
{
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("unsupported for the device")));
exit:
  SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

