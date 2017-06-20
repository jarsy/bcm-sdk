/* $Id: dnx_multicast_ingress.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

#include <soc/dnx/legacy/multicast_imp.h>
#include <soc/dnxc/legacy/error.h>


#include <soc/mcm/memregs.h>


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/TMC/tmc_api_multicast_ingress.h>


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
 * This function writes the ingress format hardware fields to a jer2_arad_mcdb_entry_t structure.
 * The input is the replication data (destination and outlif) and a pointer to next entry.
 */
uint32 dnx_ing_mc_group_entry_to_mcdb_entry(
    DNX_SAND_IN    int                    unit,
    DNX_SAND_INOUT dnx_mcdb_entry_t       *mcdb_entry, /* mcdb to write to */
    DNX_SAND_IN    DNX_TMC_MULT_ING_ENTRY *ing_entry,  /* replication data */
    DNX_SAND_IN    uint32                 next_entry   /* the next entry */
);

/*
 * given a mcid of an ingress group, configures the ingress properties of the group in IDR_IRDBm.
 * Given the enum, two hardware bits are set:
 *  1. If the ingress group is to be replicated. We use it to note if it is open and with at leat one replication.
 *  2. If the group uses a unicast buffer - has no more than one replication.
 */
uint32 dnx_mult_properties_set_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  mcid,
    DNX_SAND_IN  irdb_value_t value /* properties value to set */
)
{
  uint32 entry[2] = {0};
  const unsigned bit_offset = 2 * (mcid % 16);
  const int table_index = mcid / 16;

  DNXC_INIT_FUNC_DEFS;
  if (mcid >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCID out of range")));
  }

  DNXC_IF_ERR_EXIT(READ_IDR_IRDBm(unit, MEM_BLOCK_ANY, table_index, entry));

  if (((*entry >> bit_offset) & irdb_value_group_max_value) != value) { /* If setting a different value, update hardware */
    *entry &= ~(((uint32)irdb_value_group_max_value) << bit_offset);
    *entry |= (((uint32)value) << bit_offset);
    DNXC_IF_ERR_EXIT(WRITE_IDR_IRDBm(unit, MEM_BLOCK_ANY, table_index, entry));
  }
exit:
  DNXC_FUNC_RETURN;
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
uint32 dnx_mult_properties_get_unsafe(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32  mcid,
    DNX_SAND_OUT irdb_value_t *value /* properties value to set */
)
{
  uint32 entry[2];
  const unsigned bit_offset = 2 * (mcid % 16);
  const int table_index = mcid / 16;

  DNXC_INIT_FUNC_DEFS;
  if (mcid >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCID out of range")));
  }
  DNXC_NULL_CHECK(value);

  DNXC_IF_ERR_EXIT(READ_IDR_IRDBm(unit, MEM_BLOCK_ANY, table_index, entry));

  *value = ((*entry) >> bit_offset) & irdb_value_group_max_value;

exit:
  DNXC_FUNC_RETURN;
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
uint32 dnx_mult_ing_group_set(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY  *mc_group,
    DNX_SAND_IN  uint32               mc_group_size,
    DNX_SAND_IN  uint8                allow_create, /* if non zero, will create the group if it does not exist */
    DNX_SAND_OUT DNX_TMC_ERROR        *out_err      /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  int group_exists = 0, group_start_alloced = 0, failed = 1;
  uint32 group_entry_type;
  uint32 nof_entries_needed, remaining_group_entries = mc_group_size;
  uint32 old_group_entries = DNX_MCDS_INGRESS_LINK_END(mcds), linked_list = DNX_MCDS_INGRESS_LINK_END(mcds);
  uint32 block_start = 0;
  dnx_free_entries_block_size_t block_size = 0;
  jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, multicast_id_ndx);
  jer2_arad_mcdb_entry_t start_entry = {0};
  irdb_value_t prev_prop_value, new_prop_value;

  DNXC_INIT_FUNC_DEFS;
  if (mc_group_size) {
    DNXC_NULL_CHECK(mc_group);
    if (mc_group_size > mcds->max_nof_ingr_replications) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ingress MC group size is too big")));
    }
  }
  DNXC_NULL_CHECK(out_err);
  DNX_MC_ASSERT(multicast_id_ndx < SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids);


  group_entry_type = DNX_MCDS_GET_TYPE(mcds, multicast_id_ndx);
  group_exists = group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS_START;
  if (!group_exists && !allow_create) {
    DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
  }

  /* If this is a new group, we need to reserve its first entry, and possibly relocate the entry there */
  if (!group_exists && multicast_id_ndx != DNX_MC_EGRESS_LINK_PTR_END) {
    if (DNX_MCDS_TYPE_IS_USED(group_entry_type)) { /* relocate conflicting entries if needed */
      DNXC_IF_ERR_EXIT(dnx_mcdb_relocate_entries(unit, multicast_id_ndx, 0, 0, out_err));
      if (*out_err) { /* If failed due to lack of memories, do the same */
        SOC_EXIT;
      }
    } else { /* just allocate the start entry of the group */
      DNXC_IF_ERR_EXIT(dnx_mcds_reserve_group_start(mcds, multicast_id_ndx));
      DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS_START); /* mark the first group entry as the start of an ingress group */
    }
    group_start_alloced = 1;
  }

  /* calculate the needed free entries */
  nof_entries_needed = mc_group_size;
  if (mc_group_size) { /* first group entry does not need to be allocated */
    --nof_entries_needed;
  }
  /* Now we have the number of free entries that we still need for the operation to succeed */
  if (nof_entries_needed > dnx_mcds_unoccupied_get(mcds)) { /* check if we have enough free entries for the operation */
    *out_err = _SHR_E_FULL;
    SOC_EXIT;
  }
  *out_err = _SHR_E_NONE;

  /* build first group entry for writing after building all the linked list */
  if (mc_group_size) { /* set first group entry for a non empty group */
    DNXC_IF_ERR_EXIT(dnx_ing_mc_group_entry_to_mcdb_entry(unit, &start_entry, mc_group, 0));
    --remaining_group_entries;
  } else { /* store an empty entry for an empty group */
    start_entry.word0 = mcds->empty_ingr_value[0];
    start_entry.word1 = mcds->empty_ingr_value[1];
  }

  /* build the linked list except for the first entry */
  while (remaining_group_entries) { /* write the group entries except for the first */
    dnx_free_entries_block_size_t max_block_size = DNX_MCDS_MAX_FREE_BLOCK_SIZE;
    uint32 cur_entry, next_entry = linked_list;
    if (remaining_group_entries < DNX_MCDS_MAX_FREE_BLOCK_SIZE) {
      max_block_size = remaining_group_entries;
    }
    DNXC_IF_ERR_EXIT( /* allocate free entries */
      dnx_mcds_get_free_entries_block(mcds, DNX_MCDS_GET_FREE_BLOCKS_PREFER_SMALL,
        1, max_block_size, &block_start, &block_size)); /* allocation should not fail according to the needed entries calculation */
    DNX_MC_ASSERT(block_size > 0 && block_size <= max_block_size && remaining_group_entries >= block_size);

    for (cur_entry = block_start + block_size; cur_entry > block_start; ) { /* fill the allocated blocks with entries */
      --cur_entry;
      DNX_MC_ASSERT(remaining_group_entries);
      DNXC_IF_ERR_EXIT(dnx_mult_ing_multicast_group_entry_to_tbl( /* update and write the current entry */
        unit, cur_entry, mc_group + (remaining_group_entries--), next_entry,
        (cur_entry == block_start ? multicast_id_ndx : cur_entry - 1)));
      next_entry = cur_entry;
    } /* finished filling block */

    if (linked_list != DNX_MCDS_INGRESS_LINK_END(mcds)) { /* set software back link from the previously generated linked list to the last entry in this block */
      DNX_MCDS_SET_PREV_ENTRY(mcds, linked_list, block_start + block_size - 1);
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
  mcdb_entry->word1 &= DNX_MCDS_WORD1_KEEP_BITS_MASK;
  mcdb_entry->word1 |= start_entry.word1;
  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, multicast_id_ndx); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS_START); /* set type */

  /* calculate group properties - existing and new required ones.
   * A change of small group to a big one must be done before activating the change in the mcdb.
   * This is to prevent a small time where a group with > 1 replications uses a unicast buffer.
   */
  new_prop_value = mc_group_size > mcds->max_nof_mmc_replications ? irdb_value_group_open : irdb_value_group_open_mmc;
  if (group_exists) {
    DNXC_IF_ERR_EXIT(dnx_mult_properties_get_unsafe(unit, multicast_id_ndx, &prev_prop_value));
    if (mc_group_size > mcds->max_nof_mmc_replications && prev_prop_value != irdb_value_group_open) { /* need to make the change now */
      DNX_MC_ASSERT(new_prop_value == irdb_value_group_open);
      DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe(unit, multicast_id_ndx, new_prop_value));
      prev_prop_value = new_prop_value;
    }
  } else {
    prev_prop_value = irdb_value_group_closed;
  }

  DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
    mcds, unit, multicast_id_ndx, DNX_MCDS_TYPE_VALUE_INGRESS_START, linked_list));
  linked_list = DNX_MCDS_INGRESS_LINK_END(mcds); /* prevent the freeing of the linked list on error */

  if (old_group_entries != DNX_MCDS_INGRESS_LINK_END(mcds)) { /* free old linked list of the group not used any more */
    DNXC_IF_ERR_EXIT(dnx_mcdb_free_linked_list(
      unit, old_group_entries, DNX_MCDS_TYPE_VALUE_INGRESS));
  }

  if (prev_prop_value != new_prop_value) { /* set ingress group properties for the new group, if needed */
    DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe(unit, multicast_id_ndx, new_prop_value));
  }

  failed = 0;
exit:
  if (linked_list != DNX_MCDS_INGRESS_LINK_END(mcds)) { /* free the linked list allocated so far on error */
    DNX_MC_ASSERT(failed);
    dnx_mcdb_free_linked_list(unit, linked_list, DNX_MCDS_TYPE_VALUE_INGRESS);
  }
  if (block_size) { /* free the current block on error */
    DNX_MC_ASSERT(failed);
    dnx_mcds_build_free_blocks(unit, mcds,
      block_start, block_start + block_size - 1, dnx_mcds_get_region(mcds, block_start), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed);
  }
  if (group_start_alloced && failed) { /* free group start entry if needed */
    DNX_MC_ASSERT(!group_exists);
    dnx_mcds_build_free_blocks(unit, mcds,
      multicast_id_ndx, multicast_id_ndx, dnx_mcds_get_region(mcds, multicast_id_ndx), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed);
  }

  DNXC_FUNC_RETURN;
}


/*
 * This API sets the ingress group to the given replications,
 * configuring its linked list; and creates the group if it did not exist.
 */
uint32 dnx_mult_ing_group_open(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *mc_group,        /* group replications to set */
    DNX_SAND_IN  uint32                 mc_group_size,    /* number of group replications (size of mc_group) */
    DNX_SAND_OUT DNX_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(dnx_mult_ing_group_set(
    unit, multicast_id_ndx, mc_group, mc_group_size, TRUE, out_err));
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
uint32 dnx_mult_ing_get_group(
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


  DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, group_mcid, FALSE, is_open));
  if (*is_open) {
    uint16 group_size;
    DNXC_IF_ERR_EXIT(dnx_mcds_get_group(
      unit, DNX_MC_CORE_BITAMAP_CORE_0, TRUE, TRUE, group_mcid, DNX_MCDS_TYPE_VALUE_INGRESS, mc_group_size, &group_size));
    *exact_mc_group_size = group_size;
    DNXC_IF_ERR_EXIT(dnx_mcds_copy_replications_to_arrays(unit, 0, mc_group_size, ports, cuds, 0));
  } else { /* group is not open */
    *exact_mc_group_size = 0;
  }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * This API sets the ingress group to the given replications, configuring its linked list.
 * The group must exist.
 */
uint32 dnx_mult_ing_group_update(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  dnx_mc_id_t          multicast_id_ndx,    /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY   *mc_group,           /* group replications to set */
    DNX_SAND_IN  uint32                mc_group_size,       /* number of group replications (size of mc_group) */
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(dnx_mult_ing_group_set(
    unit, multicast_id_ndx, mc_group, mc_group_size, FALSE, out_err));
exit:
  DNXC_FUNC_RETURN;
}

/*
 * Closes the ingress muticast group, freeing its linked list.
 * Do nothing if the group is not open.
 */
uint32 dnx_mult_ing_group_close(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t      multicast_id_ndx /* group mcid to close */
)
{
  uint8 does_exist;
  DNXC_INIT_FUNC_DEFS;
  DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, multicast_id_ndx, FALSE, &does_exist));
  if (does_exist) { /* do nothing if not open */
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    DNX_TMC_ERROR out_err;

    DNXC_IF_ERR_EXIT(dnx_mult_ing_group_set( /* empty the group */
      unit, multicast_id_ndx, 0, 0, FALSE, &out_err));
    if (out_err) { /* should never require more entries, so if this happens it is an internal error */
      DNXC_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);
    }
    DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_closed));

    if (multicast_id_ndx != DNX_MC_EGRESS_LINK_PTR_END) { /* free the group's starting entry, this also marks the group as closed */
      DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(
        unit, mcds, multicast_id_ndx, multicast_id_ndx, dnx_mcds_get_region(mcds, multicast_id_ndx), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    } else { /* treat the non allocable first entry specifically */
      jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, DNX_MC_EGRESS_LINK_PTR_END);
      mcdb_entry->word0 = ((dnx_mcds_base_t*)mcds)->free_value[0];
      mcdb_entry->word1 &= DNX_MCDS_WORD1_KEEP_BITS_MASK;
      mcdb_entry->word1 |= ((dnx_mcds_base_t*)mcds)->free_value[1];
      DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, DNX_MC_EGRESS_LINK_PTR_END)); /* write to hardware */
    }
  }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Traverse an ingress linked list till the given replication is found, or till end of MC group.
 * If more than the given maximum number of entries is traversed, an error is returned.
 * If found the entry containing the replication is returned, otherwise the last entry in the linked list is returned.
 */

uint32 dnx_mult_traverse_ingress_list(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  uint32            list_start,   /* the mcdb index to start traversing from */
    DNX_SAND_IN  jer2_arad_mcdb_entry_t *replication, /* replication data (outlif, dest) to search for. If other bits are on, no match will be made */
    DNX_SAND_IN  uint16            max_size,     /* it is an error to traverse more than this number of entries */
    DNX_SAND_OUT uint16            *found_size,  /* the number of entries traversed */
    DNX_SAND_OUT uint8             *was_found,   /* boolean - returns non zero if the replication was found */
    DNX_SAND_OUT uint32            *found_entry  /* the index of the found/last entry */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  uint32 index = list_start;
  uint16 list_size = 1;
  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(replication);
  DNXC_NULL_CHECK(found_size);
  DNXC_NULL_CHECK(found_entry);
  DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_INGRESS(DNX_MCDS_GET_TYPE(mcds, list_start)) || list_start == DNX_MCDS_INGRESS_LINK_END(mcds));

  for (;; ++list_size) {
    jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, index);
    uint32 next_entry;

    if (list_size > max_size) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("passed max allowed entries number to traverse")));
    }
    /* try to see if the current entry is a match to the searched one */
    if (replication->word0 ==  mcdb_entry->word0 &&
        replication->word1 == (mcdb_entry->word1 & mcds->ingr_word1_replication_mask)) {
      *was_found = TRUE;
      break;
    }
    next_entry = soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf);
    if (next_entry == DNX_MCDS_INGRESS_LINK_END(mcds)) { /* is this the last MC group entry? */
      *was_found = FALSE;
      break;
    }
    index = next_entry;
    DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, next_entry) == DNX_MCDS_TYPE_VALUE_INGRESS);
  }
  *found_size = list_size;
  *found_entry = index;

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Adds the given replication to the ingress multicast group.
 * It is an error if the group is not open.
 */
uint32 dnx_mult_ing_destination_add(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *replication,     /* replication to add */
    DNX_SAND_OUT DNX_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore : insufficient memory or duplicate replication */
)
{

  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *work_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, multicast_id_ndx);
  uint32 next_index, add_index, block_start;
  uint16 found_size;
  dnx_free_entries_block_size_t block_size;
  uint8 does_exist;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(out_err);

  DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, multicast_id_ndx, FALSE, &does_exist));
  if (!does_exist) {
    DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
  }
  DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_TYPE(work_entry) == DNX_MCDS_TYPE_VALUE_INGRESS_START);

  *out_err = _SHR_E_NONE;
  next_index = soc_mem_field32_get(unit, IRR_MCDBm, work_entry, LINK_PTRf);
  if (soc_mem_field32_get(unit, IRR_MCDBm, work_entry, DESTINATIONf) == DNX_MC_ING_DESTINATION_DISABLED) { /* if the first entry is disabled */
    DNX_MC_ASSERT(next_index == DNX_MCDS_INGRESS_LINK_END(mcds));
    /* We can add the replication to the empty first entry of the group */
#ifdef DNX_INGR_MC_PERFORM_UNNEEDED_UPDATES
    DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_open_mmc)); /* update properties before we update the MCDB */
#endif
    DNXC_IF_ERR_EXIT(dnx_mult_ing_multicast_group_entry_to_tbl( /* update and write the start entry */
      unit, multicast_id_ndx, replication, next_index, multicast_id_ndx));
    SOC_EXIT;
  }

  if (!(SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_ALLOW_DUPLICATES_MODE)) { /* need to check duplicate replications */

    jer2_arad_mcdb_entry_t replication_entry = {0};
    uint8 was_found = 0;
    DNXC_IF_ERR_EXIT(dnx_ing_mc_group_entry_to_mcdb_entry(unit, &replication_entry, replication, 0));

    /* search for the replication or end of group or group too big */
    DNXC_IF_ERR_EXIT(dnx_mult_traverse_ingress_list(unit, multicast_id_ndx, &replication_entry,
      mcds->max_nof_ingr_replications, &found_size, &was_found, &add_index));
    if (was_found) {
      *out_err = _SHR_E_EXISTS; /* replication is already in the group */
      SOC_EXIT;
    }

  } else { /* no need to check duplicate replications, but need to check the group size for buffer type decision */
    /* calc the new group size (sizes above mcds->max_nof_mmc_replications are the same for our needs, so no need to continue) */
    for (found_size = 1; found_size < mcds->max_nof_ingr_replications && next_index != DNX_MCDS_INGRESS_LINK_END(mcds); ++found_size) {
      next_index = soc_mem_field32_get(unit, IRR_MCDBm, DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index), LINK_PTRf);
    }
  }
    if (found_size >= mcds->max_nof_ingr_replications) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ingress MC group size is too big to add to")));
    }

  add_index = multicast_id_ndx; /* always add replication after the start of the linked list */

  /* allocate an entry and add it after entry add_index */
  work_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, add_index);
  next_index = soc_mem_field32_get(unit, IRR_MCDBm, work_entry, LINK_PTRf);
  DNXC_IF_ERR_EXIT(dnx_mcds_get_free_entries_block( /* allocate free entry */
    mcds, DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL,
    1, 1, &block_start, &block_size));
  if (block_size != 1) { /* could not get free entries */
    *out_err = _SHR_E_FULL;
    DNX_MC_ASSERT(!block_size);
    SOC_EXIT;
  }

  DNXC_IF_ERR_EXIT(dnx_mult_ing_multicast_group_entry_to_tbl( /* update and write the allocated entry with the new replication */
    unit, block_start, replication, next_index, add_index));

  if (next_index != DNX_MCDS_INGRESS_LINK_END(mcds)) {
    DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, next_index) == DNX_MCDS_TYPE_VALUE_INGRESS);
    DNX_MCDS_SET_PREV_ENTRY(mcds, next_index, block_start); /* make entry next_index point back to new entry */
  }

  DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe( /* update properties before we update the MCDB */
    unit, multicast_id_ndx, found_size >= mcds->max_nof_mmc_replications ? irdb_value_group_open : irdb_value_group_open_mmc));

  DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to the add_index entry, to activate the change */
    mcds, unit, add_index, DNX_MCDS_TYPE_VALUE_INGRESS, block_start));

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Removes the given replication from the ingress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 dnx_mult_ing_destination_remove(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *entry,           /* replication to remove */
    DNX_SAND_OUT DNX_TMC_ERROR          *out_err          /* return possible errors that the caller may want to ignore : replication does not exist */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t replication_entry = {0}, *found_entry;
  uint16 found_size;
  uint32 found_index, next_index, prev_index, freed_index;
  uint8 was_found;
  uint8 does_exist;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(entry);

  DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, multicast_id_ndx, FALSE, &does_exist));
  if (!does_exist) {
    DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
  }

  DNXC_IF_ERR_EXIT(dnx_ing_mc_group_entry_to_mcdb_entry(unit, &replication_entry, entry, 0));

  DNXC_IF_ERR_EXIT(dnx_mult_traverse_ingress_list(
    unit, multicast_id_ndx, &replication_entry, mcds->max_nof_ingr_replications,
    &found_size, &was_found, &found_index));
  if (!was_found) {
    *out_err = _SHR_E_NOT_FOUND; /* replication was not found */
    SOC_EXIT;
  }
  *out_err = _SHR_E_NONE;

  found_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, found_index);
  next_index = soc_mem_field32_get(unit, IRR_MCDBm, found_entry, LINK_PTRf);
  prev_index = DNX_MCDS_GET_PREV_ENTRY(mcds, found_index);

  if (found_index == multicast_id_ndx) { /* the first replication/entry in the group is to be removed (not removing the first group entry) */

    DNX_MC_ASSERT(prev_index == multicast_id_ndx && found_size == 1);
    if (next_index == DNX_MCDS_INGRESS_LINK_END(mcds)) { /* this is the only entry in the group, just update it */
      found_entry->word0 = ((dnx_mcds_base_t*)mcds)->empty_ingr_value[0];
      found_entry->word1 &= ~mcds->msb_word_mask;
      found_entry->word1 |= ((dnx_mcds_base_t*)mcds)->empty_ingr_value[1];
      DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, found_index)); /* write group start entry to hardware */
#ifdef DNX_INGR_MC_PERFORM_UNNEEDED_UPDATES /* update group properties after we update the MCDB */
      DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_open_mmc));
#endif
      SOC_EXIT;
    } else { /* This group has a second entry, which will be copied to the first and removed */
      jer2_arad_mcdb_entry_t *entry2 = DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index);
      DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_TYPE(entry2) == DNX_MCDS_TYPE_VALUE_INGRESS);
      found_entry->word0 = entry2->word0;
      found_entry->word1 &= ~mcds->msb_word_mask;
      found_entry->word1 |= (entry2->word1 & mcds->msb_word_mask);
      freed_index = next_index;
      next_index = soc_mem_field32_get(unit, IRR_MCDBm, entry2, LINK_PTRf); /* select entry to update the back_pointer in */
      DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, found_index)); /* write group start entry to hardware */
    }

  } else { /* the replication entry is not the start of the group, it will be removed */

    DNX_MC_ASSERT(found_size > 2  ? (prev_index != multicast_id_ndx) : (found_size == 2 && prev_index == multicast_id_ndx));
    /* remove found_index from the linked list by making its preceding entry point to its succeeding entry */
    DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
      mcds, unit, prev_index, DNX_MCDS_TYPE_VALUE_INGRESS, next_index));
    freed_index = found_index;
    --found_size; /* found size represents the group size till prev_index */

  }

  if (next_index != DNX_MCDS_INGRESS_LINK_END(mcds)) {
    DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, next_index) == DNX_MCDS_TYPE_VALUE_INGRESS);
    DNX_MCDS_SET_PREV_ENTRY(mcds, next_index, prev_index); /* make entry next_index point back to prev_index */
    /* calc the new group size (sizes above mcds->max_nof_mmc_replications are the same for our needs, so no need to continue) */
    for (; found_size <= mcds->max_nof_mmc_replications && next_index != DNX_MCDS_INGRESS_LINK_END(mcds); ++found_size) {
      next_index = soc_mem_field32_get(unit, IRR_MCDBm, DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index), LINK_PTRf);
    }
  }

  if (found_size == mcds->max_nof_mmc_replications) { /* update group properties after we update the MCDB */
    DNXC_IF_ERR_EXIT(dnx_mult_properties_set_unsafe(unit, multicast_id_ndx, irdb_value_group_open_mmc));
  }

  DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks( /* free the entry freed_index */
    unit, mcds, freed_index, freed_index, dnx_mcds_get_region(mcds, freed_index), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));

  /* update group properties if needed */

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Returns the size of the multicast group with the
*     specified multicast id.
*********************************************************************/
uint32 dnx_mult_ing_group_size_get(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx,
    DNX_SAND_OUT uint32      *mc_group_size
)
{
  uint8 is_open;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mc_group_size);

  DNXC_IF_ERR_EXIT(dnx_mult_ing_get_group(
    unit, multicast_id_ndx, 0, 0, 0, mc_group_size, &is_open));

exit:
  DNXC_FUNC_RETURN;
}


/*
 * This function writes ingress format to a mcds mcdb entry and then to hardware the input.
 * The input is the replication data (destination and outlif), pointer to next entry,
 * and the pointer to the previous entry.
 * Receives the replication data already processed, compared to dnx_mult_ing_multicast_group_entry_to_tbl.
 */
uint32 dnx_mult_ing_multicast_set_entry(
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx, /* mcdb to write to */
    DNX_SAND_IN  uint32               outlif,           /* outlif replication data */
    DNX_SAND_IN  uint32               destination,      /* destination replication data */
    DNX_SAND_IN  uint32               next_entry,       /* the next entry */
    DNX_SAND_IN  uint32               prev_entry        /* the previous entry written only to mcds */
)

{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = dnx_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;

  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, OUT_LIFf, outlif);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, DESTINATIONf, destination);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf, next_entry);
  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_INGRESS_START : DNX_MCDS_TYPE_VALUE_INGRESS);

exit:
  DNXC_FUNC_RETURN;
}


/*
 * This function writes the ingress format hardware fields to a jer2_arad_mcdb_entry_t structure.
 * The input is the replication data (destination and outlif) and a pointer to next entry.
 */
uint32 dnx_ing_mc_group_entry_to_mcdb_entry(
    DNX_SAND_IN    int                    unit,
    DNX_SAND_INOUT dnx_mcdb_entry_t       *mcdb_entry, /* mcdb to write to */
    DNX_SAND_IN    DNX_TMC_MULT_ING_ENTRY *ing_entry,  /* replication data */
    DNX_SAND_IN    uint32                 next_entry   /* the next entry */
)
{
  DNXC_INIT_FUNC_DEFS;

  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, OUT_LIFf, ing_entry->cud);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, DESTINATIONf, ing_entry->destination.id);
  soc_mem_field32_set(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf, next_entry);

  DNXC_FUNC_RETURN;
}


/*
 * This function writes ingress format to a mcds mcdb entry and then to hardware the input.
 * The input is the replication data (destination and outlif), pointer to next entry,
 * and the pointer to the previous entry.
 */
uint32 dnx_mult_ing_multicast_group_entry_to_tbl(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  dnx_mc_id_t            multicast_id_ndx, /* mcdb to write to */
    DNX_SAND_IN  DNX_TMC_MULT_ING_ENTRY *ing_entry,       /* replication data */
    DNX_SAND_IN  uint32                 next_entry,       /* the next entry */
    DNX_SAND_IN  uint32                 prev_entry        /* the previous entry written only to mcds */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = dnx_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(ing_entry);

  /* write the hardware fields to memory */
  DNXC_IF_ERR_EXIT(dnx_ing_mc_group_entry_to_mcdb_entry(unit, mcdb_entry, ing_entry, next_entry));

  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_INGRESS_START : DNX_MCDS_TYPE_VALUE_INGRESS);

exit:
  DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 dnx_mult_ing_traffic_class_map_set(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  DNX_TMC_MULT_ING_TR_CLS_MAP *map
)
{
  DNXC_INIT_FUNC_DEFS;
  DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("unsupported for the device")));
exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32 dnx_mult_ing_traffic_class_map_get(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_OUT DNX_TMC_MULT_ING_TR_CLS_MAP *map
)
{
  DNXC_INIT_FUNC_DEFS;
  DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("unsupported for the device")));
exit:
  DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

