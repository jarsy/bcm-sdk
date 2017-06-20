/* $Id: arad_pp_mpls_term.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_JER_PP_MPLS_TERM_INCLUDED__
/* { */
#define __SOC_JER_PP_MPLS_TERM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/PPD/ppd_api_lif.h>
#include <shared/swstate/sw_state.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define JER_MPLS_TERM  sw_state_access[unit].dpp.soc.jericho.pp.mpls_term

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */
typedef struct jer_pp_mpls_term_s {
    SOC_PPC_LIF_ID local_mldp_dummy_lif_id[2]; /* In-LIF-ID of IN-LIF-dummy for temination of mLDP,must be set if mLDP is support */
} jer_pp_mpls_term_t; 

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


/*********************************************************************
* NAME:
 *   soc_jer_pp_mpls_termination_spacial_labels_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Init MPLS special labels termination mechanism
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t soc_jer_pp_mpls_termination_spacial_labels_init(int unit);

/********************************************************************* 
This function receives information regarding a certain label range (defined by the user) and places it in a given entry.
@params: 
        1) entry_index : a free entry in which the range will be allocated. Range: 0-7.
        2) range_action_info: info regarding all the relvant fields to be inserted into the entry. Members of this struct:
           label_low: lower limit of the label range.
           label_high: higher limit of the label range.
           bos_value: label expected bos value.
           bos_value_mask: when it equals 0, we check the bos value.
@returns: 
        No return value. 
*********************************************************************/
soc_error_t soc_jer_pp_mpls_termination_range_action_set(int unit, uint32 entry_index, SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO *range_action_info);
soc_error_t soc_jer_pp_mpls_termination_range_action_get(int unit, uint32 entry_index, SOC_PPC_MPLS_TERM_RANGE_ACTION_INFO *range_action_info);

/********************************************************************* 
This function receives information regarding a profile of a certain label range and places it in a 
calculated offset in the register, according to the given entry in the label range tabel 
(which is set in soc_jer_pp_mpls_termination_range_action_set).
@params: 
        1) entry_index : the entry to which this profile is attached. Range: 0-7.
        2) range_profile_info: info regarding the profile. Members of this struct:
           mpls_label_range_tag_mode: tag mode indication.
           mpls_label_range_has_cw: has cw above label indication.
           mpls_label_range_set_outer_vid: outer vid valid indicaiton.
           mpls_label_range_set_inner_vid: inner vid valid indication.
           mpls_label_range_use_base: If set, replace label with lower limit of range.
@returns: 
        No return value. 
*********************************************************************/
soc_error_t soc_jer_pp_mpls_termination_range_profile_set(int unit, uint32 entry_index, SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO *range_profile_info);
soc_error_t soc_jer_pp_mpls_termination_range_profile_get(int unit, uint32 entry_index, SOC_PPC_MPLS_TERM_RANGE_PROFILE_INFO *range_profile_info);

/********************************************************************* 
This function sets global configuration of mpls vccv type 3 (TTL) termination.
@params: 
        1) vccv_type_ttl1_oam_classification_enabled - if set vccv type 3 is supported through classifier.
           If unset supported through CPU trap. 
@returns: 
        No return value. 
*********************************************************************/
soc_error_t soc_jer_pp_mpls_termination_vccv_type_ttl1_oam_classification_set(int unit, uint8 vccv_type_ttl1_oam_classification_enabled);
soc_error_t soc_jer_pp_mpls_termination_vccv_type_ttl1_oam_classification_get(int unit, uint8 * vccv_type_ttl1_oam_classification_enabled);
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_MPLS_TERM_INCLUDED__*/
#endif
