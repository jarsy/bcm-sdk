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
#include <soc/dnxc/legacy/dnxc_mem.h>


#include <soc/mcm/memregs.h>

#include <shared/bsl.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/mbcm.h>

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
 * Initialization of the Arad blocks configured in this module.
 * Called as part of the initialization sequence.
 */
uint32 dnx_mc_init(
    DNX_SAND_IN  int                 unit
)
{
#ifdef _JER2_ARAD_MC_TEST_UNNEEDED_FABRIC_CODE_0
  uint32 reg_idx, fld_idx;
  DNX_TMC_MULT_FABRIC_ACTIVE_LINKS links;
#endif
  uint32 mc_tc, idx;
  DNX_SAND_U32_RANGE fmc_q_range;
  DNXC_INIT_FUNC_DEFS;

  /*
   *  Traffic Class to multicast traffic class mapping - set MULT-TC = TC/2
   */
  for (idx = 0; idx < JER2_ARAD_NOF_TRAFFIC_CLASSES; idx ++)
  {
    mc_tc = idx/2;
    DNXC_SAND_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mult_fabric_traffic_class_to_multicast_cls_map_set, (unit, idx, mc_tc)));
  }

#ifdef _JER2_ARAD_MC_TEST_UNNEEDED_FABRIC_CODE_0
  /*
   *  Enable auto-detect of active fabric links, with no mask.
   *  For this purpose - set all links as "active", and then enable auto-refresh
   */
  for (idx = 0; idx < SOC_DNX_DEFS_GET(unit, nof_fabric_links); idx++)
  {
    reg_idx = JER2_ARAD_REG_IDX_GET(idx, DNX_SAND_REG_SIZE_BITS);
    fld_idx = JER2_ARAD_FLD_IDX_GET(idx, DNX_SAND_REG_SIZE_BITS);
    DNX_SAND_SET_BIT(links.bitmap[reg_idx], 0x1, fld_idx);
  }

  DNXC_SAND_IF_ERR_EXIT(jer2_arad_mult_fabric_active_links_set_unsafe(unit, &links, TRUE));
#endif

#ifdef _JER2_ARAD_MC_TEST_DOUBLE_INITIALIZATION_0
  {
    DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE
      eg_vlan_rep;
    eg_vlan_rep.mc_id_low  = 0;
    eg_vlan_rep.mc_id_high = DNX_TMC_MULT_EG_VLAN_NOF_IDS_MAX - 1;
    DNXC_IF_ERR_EXIT(dnx_mult_eg_bitmap_group_range_set(unit, &eg_vlan_rep));
  }
#endif
  
    dnx_sand_SAND_U32_RANGE_clear(&fmc_q_range);
    fmc_q_range.start = 0;
    fmc_q_range.end = 3;
    DNXC_SAND_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_mult_fabric_enhanced_set,(unit, BCM_CORE_ALL, &fmc_q_range)));

exit:
  DNXC_FUNC_RETURN;
}



/*
 * This function checks if the multicast group is open (possibly empty).
 * returns TRUE if the group is open (start of a group), otherwise FALSE.
 */
int dnx_mult_does_group_exist(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  mcid,       /* MC ID of the group */
    DNX_SAND_IN  int     is_egress,  /* is the MC group an egress group */
    DNX_SAND_OUT uint8   *does_edxist
)
{
  const dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  uint32 nof_mc_ids = is_egress ? SOC_DNX_CONFIG(unit)->tm.nof_mc_ids : SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids;

  DNXC_INIT_FUNC_DEFS;

  if (mcid >= nof_mc_ids) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST, (BSL_META_U(unit, "multicast ID %u is not under the number of multicast IDs: %u\n"), mcid, nof_mc_ids));
    *does_edxist = FALSE;
    SOC_EXIT;
  }
  if (!is_egress) { /* ingress group */
    if (DNX_MCDS_GET_TYPE(mcds, mcid) != DNX_MCDS_TYPE_VALUE_INGRESS_START) {
      *does_edxist = FALSE;
      SOC_EXIT;
    }
    DNX_MC_ASSERT(mcid == DNX_MCDS_GET_PREV_ENTRY(mcds, mcid)); /* verify that mcid is a start of a linked list */
    *does_edxist = TRUE;
    SOC_EXIT;
  } else { /* egress group */
    DNXC_IF_ERR_EXIT(dnx_egress_group_open_get(unit, mcid, does_edxist));
  }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * This function checks if the multicast group is open (possibly empty).
 * returns TRUE if the group is open (start of a group), otherwise FALSE.
 */
uint32 dnx_mult_does_group_exist_ext( 
    DNX_SAND_IN  int             unit,
    DNX_SAND_IN  DNX_TMC_MULT_ID mcid,      /* MC ID of the group */
    DNX_SAND_IN  int             is_egress, /* is the MC group an egress group */
    DNX_SAND_OUT uint8           *is_open   /* returns FALSE if not open */
)
{
  const dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  uint32 nof_mc_ids = is_egress ? SOC_DNX_CONFIG(unit)->tm.nof_mc_ids : SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids;
  DNXC_INIT_FUNC_DEFS;

  if (mcid >= nof_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("multicast ID %u is not under the number of multicast IDs: %u"), mcid, nof_mc_ids));
  }
  if (!is_egress) { /* ingress group */
    if (DNX_MCDS_GET_TYPE(mcds, mcid) != DNX_MCDS_TYPE_VALUE_INGRESS_START) {
      *is_open = FALSE;
    } else {
      DNX_MC_ASSERT(mcid == DNX_MCDS_GET_PREV_ENTRY(mcds, mcid)); /* verify that mcid is a start of a linked list */
      *is_open = TRUE;
    }
  } else { /* egress group */
    DNXC_IF_ERR_EXIT(dnx_egress_group_open_get(unit, mcid, is_open));
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
 */

uint32
  dnx_mcdb_relocate_entries(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  uint32                        mcdb_index,             /* table index needed for the start of a group */
    DNX_SAND_IN  uint32                        relocation_block_start, /* the start of the relocation block */
    DNX_SAND_IN  dnx_free_entries_block_size_t relocation_block_size,  /* the size of the relocation block */
    DNX_SAND_OUT DNX_TMC_ERROR                 *out_err                /* return possible errors that the caller may want to ignore: insufficient memory */
)
{
    uint32 start = relocation_block_start, found_block_start, prev_entry;
    dnx_free_entries_block_size_t size = relocation_block_size, found_block_size;
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint32 entry_type = DNX_MCDS_GET_TYPE(mcds, mcdb_index), cud2 = DNX_MC_NO_2ND_CUD;
    dnx_free_entries_blocks_region_t *region = 0;
    int free_alloced_block = 0;
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(out_err);

    if (!size) { /* need to calculate the relocated entries */
        DNXC_IF_ERR_EXIT(dnx_mcds_get_relocation_block(mcds, mcdb_index, &start, &size));
        if (!size) {
            DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_FREE(DNX_MCDS_GET_TYPE(mcds, mcdb_index)));
            SOC_EXIT; /* a relocation is not needed */
        }
    }

    /* entry must be used, and if a block (size>1) is relocated, this must be non tdm egress */
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_USED(entry_type) && (size == 1 || entry_type == DNX_MCDS_TYPE_VALUE_EGRESS));

    DNXC_IF_ERR_EXIT( /* get free entries */
        dnx_mcds_get_free_entries_block(mcds, DNX_MCDS_GET_FREE_BLOCKS_PREFER_SIZE | DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL,
            size, size, &found_block_start, &found_block_size));
    if (!found_block_size) {
        *out_err = _SHR_E_FULL;
        SOC_EXIT;
    }
    DNX_MC_ASSERT(found_block_size <= size);
    prev_entry = DNX_MCDS_GET_PREV_ENTRY(mcds, start);

    free_alloced_block = 1;
    if (found_block_size == size) { /* can relocate all the block to the allocated block */
        dnx_free_entries_block_size_t s;
        uint32 after_block; /* the index if the entry after the block (pointed to by the last block entry) */

        DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds,unit, start + size - 1, entry_type, &after_block));

        for (s = 0; s < size; ++s) {
            DNXC_IF_ERR_EXIT( /* copy entry to new location and write to hardware */
                dnx_mcdb_copy_write(unit, start + s, found_block_start + s));
            DNX_MCDS_SET_PREV_ENTRY(mcds, found_block_start + s, s ? found_block_start + (s-1) : prev_entry);
        }
        DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* point to the new block instead of to the old one */
            mcds, unit, prev_entry, entry_type, found_block_start));
        free_alloced_block = 0;
        if (after_block != DNX_MC_EGRESS_LINK_PTR_END && after_block != DNX_MCDS_INGRESS_LINK_END(mcds)) { /* link the entry after the relocated block back to the new block */
            DNX_MCDS_SET_PREV_ENTRY(mcds, after_block, found_block_start + size - 1);
        }
    } else { /* can not relocate as one block, is non-tdm egress */
        uint32 list_next; /* the next entry in the list after the block to be relocated */
        DNXC_IF_ERR_EXIT(dnx_mcds_copy_replications_from_egress_block( /* set mcdb with the block's entries */
            unit, 1, start, size, &cud2, &list_next));
        free_alloced_block = 0;

        DNXC_IF_ERR_EXIT(mcds->set_egress_linked_list( /* replace block by a linked list */
            unit, FALSE, prev_entry, list_next, found_block_start, found_block_size, DNX_MC_CORE_BITAMAP_CORE_0, &found_block_start, out_err));
        if (*out_err) {
            SOC_EXIT;
        }
    }

    /* after reallocating, need to free the old block except for the entry which will start the linked list of the new group */
    if (mcdb_index > start) { /* free entries in relocated block before the entry to be used as a start of a MC group */
        if (!region) {
            region = dnx_mcds_get_region(mcds, mcdb_index);
        }
        DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, start, mcdb_index - 1, region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }
    if (mcdb_index + 1 < start + size) { /* free entries in relocated block before the entry to be used as a start of a MC group */
        if (!region) {
            region = dnx_mcds_get_region(mcds, mcdb_index);
        }
        DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, mcdb_index + 1, start + (size - 1), region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }
    *out_err = _SHR_E_NONE;

exit:
    if (free_alloced_block) {
        DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, 
            found_block_start, found_block_start + (found_block_size - 1), dnx_mcds_get_region(mcds, found_block_start), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
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
  dnx_mcdb_free_linked_list_till_my_end(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32    first_index,  /* table index of the first entry in the linked list to free */
    DNX_SAND_IN  uint32    entries_type, /* the type of the entries in the list */
    DNX_SAND_IN  uint32    end_index     /* the index of the end of the list to be freed */
)
{
  uint32 cur_index;
  uint32 block_start = 0, block_end = 0; /* will mark a block of consecutive entries in the linked list to free together */
  uint32 range_start = 0, range_end = 0; /* will mark the consecutive range of the region in which the block can grow */
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  dnx_free_entries_blocks_region_t *region = 0;
  DNXC_INIT_FUNC_DEFS;

  for (cur_index = first_index; cur_index != end_index; ) { /* loop over the linked list */
    DNX_MC_ASSERT(cur_index > 0 && cur_index < DNX_MCDS_INGRESS_LINK_END(mcds));

    if (block_end) { /* we are in a block, try enlarging it with the entry */
      if (cur_index == block_end + 1 && cur_index <= range_end) { /* Can this entry join the block at its end? */
        block_end = cur_index;
      } else if (cur_index + 1 == block_start && cur_index >= range_start) { /* Can this entry join the block at its start? */
        block_start = cur_index;
      } else { /* can't enlarge block, add block to free list and start a new one */
        DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, block_start, block_end, region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
        region = dnx_mcds_get_region_and_consec_range(mcds, cur_index, &range_start, &range_end); /* get range in which we can accumulate a block */
        block_start = block_end = cur_index;
      }
    } else { /* we are at at the first entry */
      region = dnx_mcds_get_region_and_consec_range(mcds, cur_index, &range_start, &range_end); /* get range in which we can accumulate a block */
      block_start = block_end = cur_index;
    }
    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, cur_index, entries_type, &cur_index));
  }
  if (block_end) { /* write the last discovered consecutive */
    DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, block_start, block_end, region, dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
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
  dnx_mcdb_free_linked_list(
    DNX_SAND_IN int     unit,
    DNX_SAND_IN uint32  first_index, /* table index of the first entry in the linked list to free */
    DNX_SAND_IN uint32  entries_type /* the type of the entries in the list */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  return dnx_mcdb_free_linked_list_till_my_end(unit, first_index, entries_type, 
    (DNX_MCDS_TYPE_IS_INGRESS(entries_type) ? DNX_MCDS_INGRESS_LINK_END(mcds) : DNX_MC_EGRESS_LINK_PTR_END));
}


/*
 * Return the contents of a multicast linked list group of a given type.
 * The contents is returned in the mcds buffer. Egress destination ports are TM/PP ports.
 * if the group size is bigger than the specified max_size, only max_size entries are returned,
 * it will not be an error, and the actual size of the group is returned.
 * The group must be open.
 */

uint32 dnx_mcds_get_group(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  dnx_mc_core_bitmap_t cores_to_get, /* the linked list cores to get */
    DNX_SAND_IN  uint8                do_clear,     /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint8                get_bm_reps,  /* Should the function return bitmap replications in every core (if non zero) or only in core 0 */
    DNX_SAND_IN  uint32               group_id,     /* the mcid of the group */
    DNX_SAND_IN  uint32               group_type,   /* the type of the group (of the entries in the list) */
    DNX_SAND_IN  uint16               max_size,     /* the maximum number of members to return from the group */
    DNX_SAND_OUT uint16               *group_size   /* the returned actual group size */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint32 entry_type = DNX_MCDS_TYPE_GET_NONE_START(group_type); /* type for the non first entry */
    const uint16 max_group_size = DNX_MCDS_TYPE_IS_INGRESS(group_type) ? DNX_MULT_MAX_INGRESS_REPLICATIONS : DNX_MULT_MAX_EGRESS_REPLICATIONS;
    uint32 end_index = DNX_MCDS_TYPE_IS_INGRESS(group_type) ? DNX_MCDS_INGRESS_LINK_END(mcds) : DNX_MC_EGRESS_LINK_PTR_END;
    uint32 cur_index = group_id, cud2;
    uint16 entries_remaining = max_size > max_group_size ? max_group_size : max_size; /* max replications to return */
    uint8 get_bm_reps_in_this_core;
    int core;
    dnx_mc_core_bitmap_t cores_bm;
    unsigned is_egress = DNX_MCDS_TYPE_IS_EGRESS(group_type);

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(group_size);
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_USED(group_type));
    *group_size = 0;

    if (do_clear) {
      dnx_mcds_clear_replications(mcds, group_type); /* reset the returned replications to none */
    }

    DNX_MC_FOREACH_CORE(cores_to_get, cores_bm, core) {
        cud2 = DNX_MC_NO_2ND_CUD;
        get_bm_reps_in_this_core = 1;
        if (is_egress) { /* calculate index of first group entry */
            cur_index = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, group_id, core);
            if (get_bm_reps == 0 && core != 0) {
                get_bm_reps_in_this_core = 0;
            }
        }
        /* get replications from the first entry */
        DNXC_IF_ERR_EXIT(mcds->get_replications_from_entry(unit, core, get_bm_reps_in_this_core, cur_index,
          DNX_MCDS_TYPE_GET_START(group_type), &cud2, &entries_remaining, group_size, &cur_index));

        /* get replications from the rest of the entries */
        while (cur_index != end_index) {
            DNX_MC_ASSERT(cur_index > 0 && cur_index < DNX_MCDS_INGRESS_LINK_END(mcds));
            DNXC_IF_ERR_EXIT(mcds->get_replications_from_entry(unit, core, get_bm_reps_in_this_core, cur_index, entry_type, &cud2, &entries_remaining, group_size, &cur_index));
            if (*group_size > max_group_size) { /* group is somehow bigger than allowed - internal error */
                DNX_MC_ASSERT(0);
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("group is too big")));
            }
        }
    }
    DNX_MC_ASSERT(core <= (is_egress ? SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores : 1));

exit:
  DNXC_FUNC_RETURN;
}


/*********************************************************************
* Initialize MC replication database
* The initialization accesses the replication table as if it was an
* Ingress replication, for all entries (including Egress MC)
**********************************************************************/
uint32 dnx_mult_rplct_tbl_entry_unoccupied_set_all(
    DNX_SAND_IN  int unit
)
{
  const dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  DNXC_INIT_FUNC_DEFS;
  DNXC_PCID_LITE_SKIP(unit);

#ifdef PLISIM
  if (!SAL_BOOT_PLISIM) /* do not init MCDB hardware in simulation, later instead of reading it into the mcds, use the init values */
#endif
  {
    DNXC_IF_ERR_EXIT(dnxc_fill_table_with_entry(unit, IRR_MCDBm, MEM_BLOCK_ANY, mcds->free_value));
  }

exit:
  DNXC_FUNC_RETURN;
}




/*
* Return the contents of the MCDB hardware entry, for reconstruction in case of SER.
*/
soc_error_t dnx_mult_get_entry(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 mcdb_index,
    DNX_SAND_OUT uint32 *entry /* output MCDB entry */
)
{
    const dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, mcdb_index);
    DNXC_INIT_FUNC_DEFS;

    if (mcdb_index > DNX_MCDS_INGRESS_LINK_END(mcds)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCDB entry %u is out of range"), mcdb_index));
    }
    if (DNX_MCDS_TYPE_IS_USED(DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry))) { /* relocate conflicting entries if needed */
        entry[0] = mcdb_entry->word0;
        entry[1] = mcdb_entry->word1 & mcds->msb_word_mask;
    } else {
        entry[0] = mcds->free_value[0];
        entry[1] = mcds->free_value[1];
    }

exit:
    DNXC_FUNC_RETURN;
}



#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

