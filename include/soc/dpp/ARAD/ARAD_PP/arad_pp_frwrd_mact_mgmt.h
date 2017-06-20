/* $Id: arad_pp_frwrd_mact_mgmt.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __ARAD_PP_FRWRD_MACT_MGMT_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_MACT_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

#include <soc/dpp/PPC/ppc_api_frwrd_mact_mgmt.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*Channel 3 is used in complience with the ECI general configuration register.*/
#define ARAD_LEARNING_DMA_CHANNEL_USED SOC_MEM_FIFO_DMA_CHANNEL_0


#define ARAD_PP_FRWRD_MACT_ENTRY_NOF_AGES                                  (8)
#define ARAD_PP_FRWRD_MACT_ENTRY_NOF_IS_OWNED                              (2)

/* MACT learn limit LIF mapping range limits */
#define ARAD_PP_FRWRD_MACT_LIMIT_RANGE_MAP_SIZE             (0x4000)
#define ARAD_PP_FRWRD_MACT_LIMIT_MAPPED_RANGE_SIZE          (0x4000)

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

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_SET = ARAD_PP_PROC_DESC_BASE_FRWRD_MACT_MGMT_FIRST,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_GET,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_AGING_INFO_SET,
  ARAD_PP_FRWRD_MACT_ONE_PASS_SET,
  SOC_PPC_FRWRD_MACT_AGING_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_AGING_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_AGING_ONE_PASS_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_AGING_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_AGING_INFO_GET,
  SOC_PPC_FRWRD_MACT_AGING_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_AGING_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_AGING_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_SET,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_GET,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_INFO_GET_UNSAFE,
  ARAD_PP_AGING_NUM_OF_CYCLES_GET,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_SET,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_SET_PRINT,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_SET_VERIFY,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_GET,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_GET_PRINT,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_FID_PROFILE_TO_FID_MAP_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_SET,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_GET,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_SET,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_SET_PRINT,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_GET,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_PER_TUNNEL_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_SET,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_SET_PRINT,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_SET_VERIFY,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_GET,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_GET_PRINT,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_LEARN_PROFILE_LIMIT_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_GET,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_SET,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_SET_PRINT,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_SET_VERIFY,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_GET,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_GET_PRINT,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_SET,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_GET,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_SET,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_GET,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_LEARN_MSGS_DISTRIBUTION_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_SET,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_SET_PRINT,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_SET_VERIFY,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_GET,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_GET_PRINT,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_SHADOW_MSGS_DISTRIBUTION_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_PORT_INFO_SET,
  SOC_PPC_FRWRD_MACT_PORT_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_PORT_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_PORT_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_PORT_INFO_GET,
  SOC_PPC_FRWRD_MACT_PORT_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_PORT_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_PORT_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_SET,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_SET_PRINT,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_SET_VERIFY,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_GET,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_GET_PRINT,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_TRAP_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_SET,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_SET_PRINT,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_GET,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_GET_PRINT,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_EVENT_GET,
  ARAD_PP_FRWRD_MACT_EVENT_GET_PRINT,
  ARAD_PP_FRWRD_MACT_EVENT_GET_UNSAFE,
  ARAD_PP_FRWRD_MACT_EVENT_GET_VERIFY,
  ARAD_PP_FRWRD_MACT_EVENT_PARSE,
  ARAD_PP_FRWRD_MACT_EVENT_PARSE_PRINT,
  ARAD_PP_FRWRD_MACT_EVENT_PARSE_UNSAFE,
  ARAD_PP_FRWRD_MACT_EVENT_PARSE_VERIFY,
  ARAD_PP_FRWRD_MACT_MGMT_GET_PROCS_PTR,
  ARAD_PP_FRWRD_MACT_MGMT_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

   ARAD_PP_FRWRD_MACT_MAX_AGE_GET,
   ARAD_PP_FRWRD_MACT_AGE_CONF_DEFAULT_GET,
   ARAD_PP_FRWRD_MACT_AGE_CONF_WRITE,
   ARAD_PP_FRWRD_MACT_AGE_MODIFY_GET,
   ARAD_PP_FRWRD_MACT_AGE_CONF_READ,
   ARAD_PP_FRWRD_MACT_EVENT_KEY_INDEX_GET,
   SOC_PPC_FRWRD_MACT_LEARN_MSGS_OLP_MSG_SET,
   SOC_PPC_FRWRD_MACT_LEARN_MSGS_OLP_MSG_GET,
   ARAD_PP_FRWRD_MACT_EVENT_WRITE,
   SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_VERIFY,
   ARAD_PP_FRWRD_MACT_INIT,
   ARAD_PP_FRWRD_MACT_MGMT_INIT,
   ARAD_PP_FRWRD_MACT_REGS_INIT,
   ARAD_PP_FRWRD_MACT_IS_INGRESS_LEARNING_GET,
   SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF_GET,
   SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE,
   SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_UNSAFE,
   SOC_PPC_FRWRD_MACT_LOOKUP_TYPE_SET,
   SOC_PPC_FRWRD_MACT_LOOKUP_TYPE_SET_UNSAFE, 
   ARAD_PP_FRWRD_MACT_MIM_INIT_SET,
   ARAD_PP_FRWRD_MACT_MIM_INIT_GET,
   ARAD_PP_FRWRD_MACT_ROUTED_LEARNING_SET,
   ARAD_PP_FRWRD_MACT_ROUTED_LEARNING_GET,
  /*
   * LAST element. Do no touch.
   */
  ARAD_PP_FRWRD_MACT_MGMT_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_MACT_MGMT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_MACT_FID_PROFILE_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_MACT_MGMT_FIRST,
  ARAD_PP_FRWRD_MACT_MAC_LEARN_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_EVENT_HANDLE_PROFILE_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_MACT_TRAP_TYPE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_VSI_EVENT_HANDLE_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_HEADER_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_MGMT_TYPE_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_MACT_SHADOW_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_ACTION_WHEN_EXCEED_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_SA_DROP_ACTION_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_SA_UNKNOWN_ACTION_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_DA_UNKNOWN_ACTION_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_MGMT_BUFF_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_MGMT_BUFF_LEN_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  ARAD_PP_FRWRD_MACT_MGMT_SEC_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_FID_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_MAC_LIMIT_GENERATE_EVENT_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY_IS_LAG_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_ACTION_TRAP_CODE_LSB_INVALID_ERR,
  ARAD_PP_FRWRD_MACT_FID_FAIL_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_FIFO_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_GLBL_LIMIT_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_FID_LIMIT_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_FID_LIMIT_DIS_ERR,
  ARAD_PP_FRWRD_MACT_MSG_LEN_ERR,
  ARAD_PP_FRWRD_MACT_MSG_IS_NOT_LEARN_MSG_ERR,
  ARAD_PP_FRARD_MACT_FRWRD_MACT_LOOKUP_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_MACT_SA_LOOKUP_MIM_AND_SA_AUTH_CANNOT_COEXIST_ERR,
  ARAD_PP_FRWRD_MACT_MIM_IS_NOT_ENABLED_ERR,
  ARAD_PP_FRWRD_MACT_FID_AGING_PROFILE_OUT_OF_RANGE_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_MACT_MGMT_ERR_LAST
} ARAD_PP_FRWRD_MACT_MGMT_ERR;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE then the aging process internally deletes aged
   *  out entries.
   */
  uint8 deleted;
  /*
   *  If TRUE then the aging process generates events for aged
   *  out entries. Should be set to TRUE in a centralized
   *  mode.
   */
  uint8 aged_out;
  /*
   *  If TRUE then the aging process generates events for
   *  entries refreshed in the last traverse of the MACT.
   */
  uint8 refreshed;

} ARAD_PP_FRWRD_MACT_AGING_EVENT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE then the aging process internally deletes aged
   *  out entries.
   */
   ARAD_PP_FRWRD_MACT_AGING_EVENT age_action[ARAD_PP_FRWRD_MACT_ENTRY_NOF_AGES][ARAD_PP_FRWRD_MACT_ENTRY_NOF_IS_OWNED];

} ARAD_PP_FRWRD_MACT_AGING_EVENT_TABLE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Modification age to insert delete action.
   */
  uint32 age_delete[ARAD_PP_FRWRD_MACT_ENTRY_NOF_IS_OWNED];

  /*
   *  Modification age to insert delete aged-out.
   */
  uint32 age_aged_out[ARAD_PP_FRWRD_MACT_ENTRY_NOF_IS_OWNED];

  /*
   *  Modification age to insert refresh action.
   */
  uint32 age_refresh[ARAD_PP_FRWRD_MACT_ENTRY_NOF_IS_OWNED];

} ARAD_PP_FRWRD_MACT_AGING_MODIFICATION;

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


soc_error_t arad_pp_frwrd_mact_is_dma_supported(SOC_SAND_IN int unit, SOC_SAND_OUT uint32 *dma_supported);


/*
 *  Returns true if aging resolution is low for unit
 */
uint32
  arad_pp_frwrd_mact_is_age_resolution_low(
    SOC_SAND_IN  int   unit,
    SOC_SAND_OUT uint8 *is_age_resolution_low
  );

uint32
  arad_pp_frwrd_mact_mgmt_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint8
  arad_pp_frwrd_mact_mgmt_is_b0_high_resolution(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_frwrd_mact_lookup_type_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_LOOKUP_TYPE         lookup_type,
    SOC_SAND_IN  uint8                                  learn_enable /* disable/enable learning */
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_cpu_counter_learn_limit_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Whether the OLP generate learn limit messeges.
 * INPUT:
 *   SOC_SAND_IN  int   unit - Identifier of the device to access.
 *   SOC_SAND_IN  uint8 enable -  TRUE to enable suppression FALSE to disable.
 * REMARKS:
 *   supports only for ARAD+
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_mact_cpu_counter_learn_limit_set(
    SOC_SAND_IN  int           unit,
    SOC_SAND_IN  uint8         disable
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_cpu_counter_learn_limit_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Whether the OLP generate learn limit messeges.
 * INPUT:
 *   SOC_SAND_IN  int   unit -Identifier of the device to access.
 *   SOC_SAND_OUT uint8 *is_enabled - is suppression enabled
 * REMARKS:
 *   - supports only for ARAD+
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_mact_cpu_counter_learn_limit_get(
    SOC_SAND_IN  int           unit,
    SOC_SAND_OUT  uint8        *is_enabled
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_transplant_static_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enables or disables transplant events on static entries.
 * INPUT:
 *   SOC_SAND_IN  int   unit - Identifier of the device to access.
 *   SOC_SAND_IN uint8  enable - 1 for enable transplant events; 0 for disabling
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_mact_transplant_static_set (
    SOC_SAND_IN int     unit,
    SOC_SAND_IN uint8   enable
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_transplant_static_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns whether the device supports transplant events on static entries.
 * INPUT:
 *   SOC_SAND_IN  int   unit - Identifier of the device to access.
 *   SOC_SAND_OUT uint8*  is_enabled - 1 for transplant events enabled; 0 for disabled
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_mact_transplant_static_get (
    SOC_SAND_IN  int     unit,
    SOC_SAND_OUT uint8*  is_enabled
  );
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_oper_mode_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the mode of the MACT, including - ingress vs.
 *   egress learning- how each device responds internally to
 *   events (learn/aged-out/refresh) - which events to inform
 *   other devices.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info -
 *     MACT learning setting
 * REMARKS:
 *   - T20E supports only Egress independent and centralized
 *   learning modes
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_oper_mode_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info
  );

uint32
  arad_pp_frwrd_mact_oper_mode_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_oper_mode_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_mact_oper_mode_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_oper_mode_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_OPER_MODE_INFO           *oper_mode_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_aging_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the aging info including enable aging and aging
 *   time.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info -
 *     Whether to perform aging over the MAC entries and time
 *     of aging.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_aging_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info
  );

uint32
  arad_pp_frwrd_mact_aging_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info
  );

/* Set the aging events configuration of an aging cfg ptr. */
uint32
  arad_pp_frwrd_mact_age_conf_write(
      SOC_SAND_IN  int                      unit,
      SOC_SAND_IN  uint8                       aging_cfg_ptr,
      SOC_SAND_IN  ARAD_PP_FRWRD_MACT_AGING_EVENT_TABLE  *conf
    );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_aging_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_mact_aging_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_aging_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_INFO               *aging_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_aging_one_pass_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   perform one aging iteration
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO               *pass_info -
 *     Whether to perform aging over the MAC entries and time
 *     of aging.
 *   SOC_SAND_IN  SOC_SAND_SUCCESS_FAILURE               *success -
 *     success or fail
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_aging_one_pass_set_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO   *pass_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE             *success
  );



/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_aging_events_handle_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the device action upon events invoked by the aging
 *   process: - Whether the device deletes aged-out entries
 *   internally - Whether the device generates an event for
 *   aged-out entries - Whether the device generates an event
 *   for refreshed entries
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE     *aging_info -
 *     Device actions upon aging-related events.
 * REMARKS:
 *   - use soc_ppd_frwrd_mact_event_handle_info_set() to set how
 *   the OLP should distribute age-out and refresh events.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_aging_events_handle_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE            *aging_info
  );

uint32
  arad_pp_frwrd_mact_aging_events_handle_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE            *aging_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_aging_events_handle_info_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_frwrd_mact_aging_events_handle_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_aging_events_handle_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE            *aging_info
  );

soc_error_t arad_pp_aging_num_of_cycles_get (
    SOC_SAND_IN int     unit,
    SOC_SAND_IN uint32  aging_profile,
    SOC_SAND_OUT int     *fid_aging_cycles
);
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_fid_profile_to_fid_map_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Maps FID-Profile to FID, for shared learning.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  fid_profile_ndx -
 *     FID Profile. Range: 1 - 7. The 0 value is used when FID
 *     = VSI.
 *   SOC_SAND_IN  SOC_PPC_FID                                 fid -
 *     Filtering ID. Range: 0 - 16K-1.
 * REMARKS:
 *   - The FID-profile is an attribute of the VSI.- For VSIs
 *   with an FID-profile = 0, FID = VSI.- T20E: This API can
 *   be omitted since for all VSIs, FID = VSI.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_fid_profile_to_fid_map_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  fid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_FID                                 fid
  );

uint32
  arad_pp_frwrd_mact_fid_profile_to_fid_map_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  fid_profile_ndx,
    SOC_SAND_IN  SOC_PPC_FID                                 fid
  );

uint32
  arad_pp_frwrd_mact_fid_profile_to_fid_map_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  fid_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_fid_profile_to_fid_map_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_frwrd_mact_fid_profile_to_fid_map_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_fid_profile_to_fid_map_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  fid_profile_ndx,
    SOC_SAND_OUT SOC_PPC_FID                                 *fid
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mac_limit_glbl_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable the MAC limit feature, which limits per fid the
 *   maximum number of entries allowed to be in the MAC
 *   Table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info -
 *     Limitation settings, including if this feature is
 *     enabled and how to act when static entry tries to exceed
 *     the limit.
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_mac_limit_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info
  );

uint32
  arad_pp_frwrd_mact_mac_limit_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_mac_limit_glbl_info_set_unsafe" API.
 *     Refer to
 *     "arad_pp_frwrd_mact_mac_limit_glbl_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_mac_limit_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO           *limit_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_learn_profile_limit_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the limit profile information including the
 *   MAC-limit (i.e., the maximum number of entries an FID
 *   can hold in the MAC Table), and the notification action
 *   if the configured limit is exceeded.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  mac_learn_profile_ndx -
 *     MAC-learn-profile ID. Range: 0 - 7.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO   *limit_info -
 *     Limit profile information including the maximum number
 *     of entries that can be learned/inserted; the action to
 *     perform when an entry is tryied to be inserted/learned,
 *     exceeding the limitation.
 * REMARKS:
 *   - No limitation for a specific profile can be configured
 *   by setting is_limited to FALSE at limit_info.-
 *   mac_learn_profile_ndx is set according to
 *   soc_ppd_vsi_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_learn_profile_limit_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO   *limit_info
  );

uint32
  arad_pp_frwrd_mact_learn_profile_limit_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO   *limit_info
  );

uint32
  arad_pp_frwrd_mact_learn_profile_limit_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_learn_profile_limit_info_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_frwrd_mact_learn_profile_limit_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_learn_profile_limit_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO   *limit_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_event_handle_profile_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the mac-learn-profile to the event-handle profile.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  mac_learn_profile_ndx -
 *     MAC-learn-profile ID. Range: 0 - 7.
 *   SOC_SAND_IN  uint32                                  event_handle_profile -
 *     Profile used to set how MACT events are handled. Range:
 *     0 - 1.
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E. -
 *   mac_learn_profile_ndx is set according to
 *   soc_ppd_vsi_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_event_handle_profile_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx,
    SOC_SAND_IN  uint32                                  event_handle_profile
  );

uint32
  arad_pp_frwrd_mact_event_handle_profile_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx,
    SOC_SAND_IN  uint32                                  event_handle_profile
  );

uint32
  arad_pp_frwrd_mact_event_handle_profile_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_event_handle_profile_set_unsafe" API.
 *     Refer to
 *     "arad_pp_frwrd_mact_event_handle_profile_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_event_handle_profile_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  mac_learn_profile_ndx,
    SOC_SAND_OUT uint32                                  *event_handle_profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_fid_aging_profile_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map the mac-learn-profile to the fid_aging profile.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                           mac_learn_profile_ndx -
 *     MAC-learn-profile ID. Range: 0 - 7.
 *   SOC_SAND_IN  uint32                           fid_aging_profile -
 *     Profile used to set how MACT events are handled. Range:
 *     0 - 3.
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E. -
 *   mac_learn_profile_ndx is set according to
 *   soc_ppd_vsi_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_fid_aging_profile_set_unsafe(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  uint32         mac_learn_profile_ndx,
    SOC_SAND_IN  uint32         fid_aging_profile
  );

uint32
  arad_pp_frwrd_mact_fid_aging_profile_set_verify(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  uint32         mac_learn_profile_ndx,
    SOC_SAND_IN  uint32         fid_aging_profile
  );

uint32
  arad_pp_frwrd_mact_fid_aging_profile_get_verify(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  uint32         mac_learn_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_fid_aging_profile_set_unsafe" API.
 *     Refer to
 *     "arad_pp_frwrd_mact_fid_aging_profile_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_fid_aging_profile_get_unsafe(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  uint32         mac_learn_profile_ndx,
    SOC_SAND_OUT uint32        *fid_aging_profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_event_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the last event parameters for the direct access mode
 *   (i.e., if no OLP messages are sent in case of event)
 *   from the Event FIFO.
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_INFO         *info -
 *     Event information parameters (Key, Payload, Command
 *     type).
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E. -
 *   This API must be called only if the Learn / Shadow
 *   distribution type is through direct access to the Event
 *   FIFO
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_event_get_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_event_handle_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set how to handle an event according to the event key
 *   parameters (event-type,vsi-handle-profile,is-lag)
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY               *event_key -
 *     The key that identifies the event
 *     (event-type,vsi-handle-profile,is-lag).
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO              *handle_info -
 *     How to handle the given events, including where (i.e.,
 *     which FIFO) to send these events.
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E. - To
 *   set the FIFO configuration, use for the System learn
 *   FIFO soc_ppd_frwrd_mact_sys_learn_msgs_distribution_info_set
 *   and for the Shadow FIFO
 *   soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set. - For
 *   the get API, a single event type must be set in the
 *   event-key parameter.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_event_handle_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY               *event_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO              *handle_info
  );

uint32
  arad_pp_frwrd_mact_event_handle_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY               *event_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO              *handle_info
  );

uint32
  arad_pp_frwrd_mact_event_handle_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY               *event_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_event_handle_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_mact_event_handle_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_event_handle_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY               *event_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO              *handle_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_learn_msgs_distribution_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set how to distribute the learn messages to other
 *   devices/CPU.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info -
 *     Distribution information, in particular the header to
 *     associate with the learn messages.
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E. -
 *   Arad-B: both learn messages and shadow message must
 *   have the same external header, i.e. either both have an
 *   ITMH Header or none of them. - The get API returns also
 *   the EtherType if not inserted by the user.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_learn_msgs_distribution_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info
  );

uint32
  arad_pp_frwrd_mact_learn_msgs_distribution_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_learn_msgs_distribution_info_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_frwrd_mact_learn_msgs_distribution_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_learn_msgs_distribution_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_shadow_msgs_distribution_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set how to distribute the shadow messages to the other
 *   devices/CPU.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info -
 *     Distribution information, in particular the header to
 *     associate with the shadow messages.
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E. -
 *   Arad-B: both learn messages and shadow message must
 *   have the same external header, i.e. either both have an
 *   ITMH Header or none of them. - The get API returns also
 *   the EtherType if not inserted by the user.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_shadow_msgs_distribution_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info
  );

uint32
  arad_pp_frwrd_mact_shadow_msgs_distribution_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_shadow_msgs_distribution_info_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_frwrd_mact_shadow_msgs_distribution_info_set_unsafe"
 *     API for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_shadow_msgs_distribution_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO                 *distribution_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mac_limit_exceeded_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the information if the MAC limitation is exceeded,
 *   i.e. when a MAC Table entry is tryied to be inserted and
 *   exceeds the limitation set per FID. This insertion can
 *   be triggered by CPU or after a packet learning.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO  *exceed_info -
 *     Information if the MAC limit was exceed, including the
 *     last FID which caused this limitation violation.
 * REMARKS:
 *   - Arad-B only. Error is return if called for T20E.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_mac_limit_exceeded_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO  *exceed_info
  );



/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mac_limit_range_map_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the mapping information for the a specific mapped values
 *   range. The mapping information contanins the required bit
 *   manipulation for a mapped value in the range, in order to
 *   get the matching entry in the common MACT Limit table.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int8                               range_num -
 *     Range number of the mapped values.
 *     1 up to SOC_PPC_MAX_NOF_MACT_LIMIT_LIF_RANGES.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO *map_info -
 *     The mapping information per range.
 * REMARKS:
 *   - Arad+ only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_mac_limit_range_map_info_get_unsafe(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN  int8                                           range_num,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO    *map_info
  );

uint32
  arad_pp_frwrd_mact_mac_limit_range_map_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int8                                   range_num
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mac_limit_mapping_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get all the MACT Limit mapping information.
 *   The information includes an entry pointer for invalid mapped
 *   values, range end values for mapped value various ranges.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO *map_info -
 *     General MACT limit mapping information.
 * REMARKS:
 *   - Arad+ only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_mac_limit_mapping_info_get_unsafe(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO      *map_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_limit_mapped_val_to_table_index_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map a value to the common MACT limit table. The function
 *   performs 'HW Like' bit manipulation, exactly the way the
 *   HW does them on packet mapped value.
 * INPUT:
 *   SOC_SAND_IN  int                        unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                        mapped_val -
 *     The value that needs to be mapped to a limit table index.
 *   SOC_SAND_OUT uint32                        *limit_tbl_idx -
 *     A MACT Limit table index that the value was mapped to.
 *   SOC_SAND_OUT uint32                          *is_reserved -
 *     Flag indicating whether the resulting index points to
 *     the reserved entry for invalid mappings.
 * REMARKS:
 *   - Arad+ only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_limit_mapped_val_to_table_index_get_unsafe(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  uint32                                 mapped_val,
      SOC_SAND_OUT uint32                                 *limit_tbl_idx,
      SOC_SAND_OUT uint32                                 *is_reserved
    );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_port_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set per port MACT management information including which
 *   profile to activate when SA is known in this port.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO                *port_info -
 *     Port information.
 * REMARKS:
 *   - Use soc_ppd_frwrd_mact_trap_info_set() to set the drop
 *   action for an SA MAC.- Arad-B only, error when called
 *   over T20E device.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_port_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO                *port_info
  );

uint32
  arad_pp_frwrd_mact_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO                *port_info
  );

uint32
  arad_pp_frwrd_mact_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_port_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_mact_port_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_port_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_PORT_INFO                *port_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_trap_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   For each trap type, set the action profile. Different
 *   actions may be assigned to the same trap type according
 *   to the port-profile (4 possibilities).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE                trap_type_ndx -
 *     Type of the Trap.
 *   SOC_SAND_IN  uint32                                  port_profile_ndx -
 *     Per port profile, to enable the setting of different
 *     actions for the same trap type. To set this profile for
 *     a port, use soc_ppd_frwrd_mact_port_info_set().
 *   SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile -
 *     Trap information including snoop/forwarding action. The
 *     trap_code is necessarily 0.
 * REMARKS:
 *   - For the Trap is
 *   SOC_PPC_FRWRD_MACT_TRAP_TYPE_SAME_INTERFACE port_profile_ndx
 *   has to be 0, since there is port profile is not relevant
 *   for this Trap.- Use soc_ppd_frwrd_mact_port_info_set() to
 *   set the mapping from the port profile to the profile for
 *   action.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_trap_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE                trap_type_ndx,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile
  );

uint32
  arad_pp_frwrd_mact_trap_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE                trap_type_ndx,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_IN  SOC_PPC_ACTION_PROFILE                      *action_profile
  );

uint32
  arad_pp_frwrd_mact_trap_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE                trap_type_ndx,
    SOC_SAND_IN  uint32                                  port_profile_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_trap_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_mact_trap_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_trap_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE                trap_type_ndx,
    SOC_SAND_IN  uint32                                  port_profile_ndx,
    SOC_SAND_OUT SOC_PPC_ACTION_PROFILE                      *action_profile
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_ip_compatible_mc_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the information for bridging compatible Multicast
 *   MAC addresses.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO    *info -
 *     Compatible Multicast MAC information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_ip_compatible_mc_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO    *info
  );

uint32
  arad_pp_frwrd_mact_ip_compatible_mc_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO    *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_mact_ip_compatible_mc_info_set_unsafe" API.
 *     Refer to
 *     "arad_pp_frwrd_mact_ip_compatible_mc_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_ip_compatible_mc_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO    *info
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_event_parse_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   The MACT may report different events using the event
 *   FIFO (e.g., learn, age, transplant, and retrieve). This
 *   API Parses the event buffer into a meaningful structure.
 * INPUT:
 *   SOC_SAND_IN  int                                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf -
 *     Buffer includes MACT event
 *   SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_INFO              *mact_event -
 *     MACT Event parsed into structure
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_mact_event_parse_unsafe(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf,
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_INFO              *mact_event
  );

uint32
  arad_pp_frwrd_mact_learn_msg_parse_unsafe(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN   SOC_PPC_FRWRD_MACT_LEARN_MSG                  *learn_msg,
    SOC_SAND_IN   SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF             *learn_msg_conf,
    SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO       *learn_events
  );

uint32
arad_pp_frwrd_mact_learn_msg_conf_get_unsafe(
  SOC_SAND_IN   int                             unit,
  SOC_SAND_IN   SOC_PPC_FRWRD_MACT_LEARN_MSG            *learn_msg,
  SOC_SAND_OUT  SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF       *learn_msg_conf
  );

uint32
  arad_pp_frwrd_mact_mim_init_set_unsafe(
      SOC_SAND_IN   int                                   unit,
      SOC_SAND_IN   uint8                                    mim_initialized
  );

uint32
  arad_pp_frwrd_mact_mim_init_get_unsafe(
    SOC_SAND_IN   int                           unit,
    SOC_SAND_OUT  uint8                            *mim_initialized
  );

/*********************************************************************
 * NAME:
 *   arad_pp_frwrd_mact_routed_learning_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets routed pcakets learning for given application types.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                 appFlags -
 *     Flags indicating the applications to set the routed learning for.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_pp_frwrd_mact_routed_learning_set_unsafe(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_IN  uint32                appFlags
  );

/*********************************************************************
 * NAME:
 *   arad_pp_frwrd_mact_routed_learning_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the for which applications the routed pcakets learning is enabled.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                 *appFlags -
 *     Flags indicating the applications for which the routed learning
 *     is enabled.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_pp_frwrd_mact_routed_learning_get_unsafe(
    SOC_SAND_IN  int                unit, 
    SOC_SAND_OUT uint32                *appFlags
  );

uint32
  arad_pp_frwrd_mact_event_parse_verify(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER            *event_buf
  );
void
  ARAD_PP_FRWRD_MACT_AGING_MODIFICATION_clear(
    SOC_SAND_OUT ARAD_PP_FRWRD_MACT_AGING_MODIFICATION  *info
  );

void
  ARAD_PP_FRWRD_MACT_AGING_EVENT_TABLE_clear(
    SOC_SAND_OUT ARAD_PP_FRWRD_MACT_AGING_EVENT_TABLE  *info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mgmt_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_mact_mgmt module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_mact_mgmt_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_mact_mgmt_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_mact_mgmt module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_mact_mgmt_get_errs_ptr(void);

uint32
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_AGING_INFO_verify(
    SOC_SAND_IN   int                                   unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY *info
  );

uint32
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE *info
  );

uint32
  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO_verify(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_verify(
    SOC_SAND_IN  int                                        unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_verify(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_PORT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_EVENT_INFO_verify(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_INFO *info
  );

uint32
  SOC_PPC_FRWRD_MACT_EVENT_BUFFER_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER *info
  );

soc_error_t arad_pp_frwrd_mact_event_handle_info_set_dma(SOC_SAND_IN int unit, SOC_SAND_IN int enable_dma);

soc_error_t arad_pp_frwrd_mact_learning_dma_set( SOC_SAND_IN  int unit );

soc_error_t arad_pp_frwrd_mact_learning_dma_unset(SOC_SAND_IN int unit);

void arad_pp_frwrd_mact_learning_dma_event_handler(
      SOC_SAND_INOUT  void  *unit_ptr,
      SOC_SAND_INOUT  void  *event_type_ptr,
      SOC_SAND_INOUT  void  *cmc_ptr,
      SOC_SAND_INOUT  void  *ch_ptr,
      SOC_SAND_INOUT  void  *unused4);

soc_error_t arad_pp_frwrd_mact_clear_access_bit(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 fid,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS             *mac
  );

soc_error_t arad_pp_frwrd_mact_opport_mode_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT  uint32                *opport
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_MACT_INCLUDED__*/


#endif
