/* $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#if !defined(__PPD_API_SLB_INCLUDED__) && !defined(PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY)

#define __PPD_API_SLB_INCLUDED__

/*************
 * INCLUDES  *
 *************/

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_slb.h>

#include <soc/dpp/PPD/ppd_api_general.h>


/*************
 * FUNCTIONS *
 *************/

/**
 * Set/get the global SLB configuration. 
 * The item that is set/get is determined according to the passed object. 
 * The item action is documented over the item class. 
 *  
 * INPUT 
 *   unit [IN]
 *   configuration_item [IN]                                    - The item to set/get. The supported items are:
 *     SOC_PPC_SLB_CONFIGURATION_ITEM_SLB_ENTRY_TTL_IN_SECONDS    - Set/get the TTL of SLB entries.
 *  
 * RETURN
 *   OK or ERROR indication.
 *  
 */
uint32 
  soc_ppd_slb_set_global_cfg(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_IN     SOC_PPC_SLB_CONFIGURATION_ITEM *          configuration_item
  );

uint32 
  soc_ppd_slb_get_global_cfg(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_INOUT  SOC_PPC_SLB_CONFIGURATION_ITEM *          configuration_item
  );

/**
 *  
 * Traverse the LEM, looking for matching SLB entries. 
 * For matching entries apply the passed action.
 *  
 * INPUT
 *   unit
 *   match_rules         - An array with rules to match against (see the section about match rules later).
 *   nof_match_rules     - The number of match rules (amount of elemnts in the match_rules array).
 *   action              - The action to perform for each matched entry (see the section about actions later).
 *   nof_matched_entries - Will be filled with the amount of matched entries.
 *  
 * REMARKS
 *   Match Rules 
 *     The match_rules array should hold non-abstract TRAVERSE_MATCH_RULE instances
 *     inheriting from SOC_PPC_SLB_TRAVERSE_MATCH_RULE.
 *     The possible match rules are:
 *       SOC_PPC_SLB_TRAVERSE_MATCH_RULE_ALL_LAG              - Match all LAG entries.
 *       SOC_PPC_SLB_TRAVERSE_MATCH_RULE_ALL_ECMP             - Match all ECMP entries.
 *       SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_LAG         - Match a LAG group.
 *       SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_ECMP        - Match an ECMP group.
 *       SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_LAG  - Match a LAG group member.
 *       SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_ECMP - Match an ECMP group member.
 *     The allowed combinations are:
 *       1) A single ALL rule.
 *       2) A combination of LB_GROUP and LB_GROUP_MEMBER - at least one, at most two, at most one of each.
 *  
 *   Match Rules Limitations
 *     At most one SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP may be supplied.
 *     At most one SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER may be supplied.
 *     The LB group type must match the LB group member type (LAG/ECMP).
 *  
 *   Actions
 *     The action should hold a non-abstract TRAVERSE_ACTION instance inherting
 *     from SOC_PPC_SLB_TRAVERSE_ACTION.
 *     The possible actions are:
 *       SOC_PPC_SLB_TRAVERSE_ACTION_COUNT - Count the entries matching the rules.
 *       SOC_PPC_SLB_TRAVERSE_ACTION_REMOVE - Remove the entry.
 *       SOC_PPC_SLB_TRAVERSE_ACTION_UPDATE - Update the entry.
 *  
 * RETURN
 *   OK or ERROR indication. 
 */
uint32 
  soc_ppd_slb_traverse(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_IN     SOC_PPC_SLB_TRAVERSE_MATCH_RULE * const * match_rules,
    SOC_SAND_IN     uint32                                    nof_match_rules,
    SOC_SAND_IN     SOC_PPC_SLB_TRAVERSE_ACTION *             action,
    SOC_SAND_OUT    uint32 *                                  nof_matched_entries
  );

/**
 *  
 * Traverse the LEM, looking for matching SLB entries. 
 * The matching entries are filled in slb_keys and slb_vals.
 *  
 * INPUT
 *   unit
 *   match_rules      - An array with rules to match against (see the section about match rules in the
 *                      description of soc_ppd_slb_traverse for more info about match rules).
 *   nof_match_rules  - The number of match rules (amount of elemnts in the match_rules array).
 *   block_range      - The range to traverse in the HW table.
 *                      Exactly SOC_PPC_SLB_MAX_ENTRIES_FOR_GET_BLOCK entries_to_act
 *                      are allowed.
 *   slb_keys         - The array to include SLB entry keys for entries that matched.
 *                      Note that this is not an object array, but a contiguous ENTRY_KEY array.
 *   slb_vals         - The array to include SLB entry values for entries that matched.
 *                      Note that this is not an object array, but a contiguous ENTRY_VALUE array.
 *   nof_entries      - The amount of entries that were matched (and consequently, the amount of
 *                      items filled in slb_keys and slb_vals).
 *  
 * REMARKS 
 *   1) slb_keys and slb_vals should have memory for at least
 *      block_range->entries_to_act items.
 *   2) For help about using SOC_SAND_TABLE_BLOCK_RANGE see soc_ppd_frwrd_mact_get_block.
 *   3) There must be at least 130 because of the flush machine that uses a FIFO of size 128).
 *      If under 130 entries are used, then the function will not update the iter member of the
 *      block range.
 *   4) get_block can be extended to allow more than 130 entries_to_act (which is why
 *      it is not ignored).
 *  
 * RETURN
 *   OK or ERROR indication.
 */
uint32
  soc_ppd_slb_get_block(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_IN     SOC_PPC_SLB_TRAVERSE_MATCH_RULE * const * match_rules,
    SOC_SAND_IN     uint32                                    nof_match_rules,
    SOC_SAND_INOUT  SOC_SAND_TABLE_BLOCK_RANGE *              block_range,
    SOC_SAND_OUT    SOC_PPC_SLB_ENTRY_KEY *                   slb_keys,
    SOC_SAND_OUT    SOC_PPC_SLB_ENTRY_VALUE *                 slb_vals,
    SOC_SAND_OUT    uint32 *                                  nof_entries
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>

#elif defined(PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY)

#ifndef PPD_API_SLB_INTERNAL_STD_IMPL
#error "PPD_API_SLB_INTERNAL_STD_IMPL must be defined."
#endif

#ifndef PPD_API_SLB_INTERNAL_PREFIX
#error "PPD_API_SLB_INTERNAL_PREFIX must be defined"
#endif

uint32 
  PPD_API_SLB_INTERNAL_PREFIX(slb_set_global_cfg)(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_IN     SOC_PPC_SLB_CONFIGURATION_ITEM *          configuration_item
  )
PPD_API_SLB_INTERNAL_STD_IMPL(slb_set_global_cfg, ( unit, configuration_item))

uint32 
  PPD_API_SLB_INTERNAL_PREFIX(slb_get_global_cfg)(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_INOUT  SOC_PPC_SLB_CONFIGURATION_ITEM *          configuration_item
  )
PPD_API_SLB_INTERNAL_STD_IMPL(slb_get_global_cfg, (unit, configuration_item))

uint32 
  PPD_API_SLB_INTERNAL_PREFIX(slb_traverse)(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_IN     SOC_PPC_SLB_TRAVERSE_MATCH_RULE * const * match_rules,
    SOC_SAND_IN     uint32                                    nof_match_rules,
    SOC_SAND_IN     SOC_PPC_SLB_TRAVERSE_ACTION *             action,
    SOC_SAND_OUT    uint32 *                                  nof_matched_entries
  )
PPD_API_SLB_INTERNAL_STD_IMPL(slb_traverse, (unit, match_rules, nof_match_rules, action, nof_matched_entries))

uint32
  PPD_API_SLB_INTERNAL_PREFIX(slb_get_block)(
    SOC_SAND_IN     int                                    unit,
    SOC_SAND_IN     SOC_PPC_SLB_TRAVERSE_MATCH_RULE * const * match_rules,
    SOC_SAND_IN     uint32                                    nof_match_rules,
    SOC_SAND_INOUT  SOC_SAND_TABLE_BLOCK_RANGE *              block_range,
    SOC_SAND_OUT    SOC_PPC_SLB_ENTRY_KEY *                   slb_keys,
    SOC_SAND_OUT    SOC_PPC_SLB_ENTRY_VALUE *                 slb_vals,
    SOC_SAND_OUT    uint32 *                                  nof_entries
  )
PPD_API_SLB_INTERNAL_STD_IMPL(slb_get_block, (unit, match_rules, nof_match_rules, block_range, slb_keys, slb_vals, nof_entries))

#endif /* !defined(__PPD_API_SLB_INCLUDED__) && !defined(PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY) */

