/* $Id: arad_pp_frwrd_bmact.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_BMACT_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_BMACT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>

#include <soc/dpp/PPC/ppc_api_frwrd_bmact.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_LEM_ACCESS_ASD_LEARN_TYPE                     (16)
#define ARAD_PP_LEM_ACCESS_ASD_LEARN_TYPE_LEN                 (1)
#define ARAD_PP_LEM_ACCESS_ASD_LEARN_FEC_PTR                  (0)
#define ARAD_PP_LEM_ACCESS_ASD_DOMAIN                         (17)
#define ARAD_PP_LEM_ACCESS_ASD_DOMAIN_LEN                     (5)

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
  ARAD_PP_FRWRD_BMACT_BVID_INFO_SET = ARAD_PP_PROC_DESC_BASE_FRWRD_BMACT_FIRST,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_PRINT,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_SET_VERIFY,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_GET,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_PRINT,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_VERIFY,
  ARAD_PP_FRWRD_BMACT_BVID_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_PRINT,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_VERIFY,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_PRINT,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_VERIFY,
  ARAD_PP_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_ENTRY_ADD,
  ARAD_PP_FRWRD_BMACT_ENTRY_ADD_PRINT,
  ARAD_PP_FRWRD_BMACT_ENTRY_ADD_UNSAFE,
  ARAD_PP_FRWRD_BMACT_ENTRY_ADD_VERIFY,
  ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE,
  ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_PRINT,
  ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_BMACT_ENTRY_REMOVE_VERIFY,
  ARAD_PP_FRWRD_BMACT_ENTRY_GET,
  ARAD_PP_FRWRD_BMACT_ENTRY_GET_PRINT,
  ARAD_PP_FRWRD_BMACT_ENTRY_GET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_ENTRY_GET_VERIFY,
  ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET_VERIFY,
  ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_GET_UNSAFE,
  ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_GET_VERIFY,
  ARAD_PP_FRWRD_BMACT_GET_PROCS_PTR,
  ARAD_PP_FRWRD_BMACT_GET_ERRS_PTR,
  ARAD_PP_FRWRD_BMACT_INIT,
  ARAD_PP_FRWRD_BMACT_INIT_UNSAFE,
  ARAD_PP_FRWRD_BMACT_INIT_PRINT,
  ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET,
  ARAD_PP_FRWRD_BMACT_EG_VLAN_PCP_MAP_GET,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_BMACT_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_BMACT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_BMACT_SUCCESS_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_BMACT_FIRST,
  ARAD_PP_FRWRD_BMACT_STP_TOPOLOGY_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_BMACT_B_FID_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_BMACT_SYS_PORT_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_BMACT_I_SID_DOMAIN_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_BMACT_KEY_FLAGS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_BMACT_DA_NOT_FOUND_ACTION_PROFILE_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_BMACT_MAC_IN_MAC_CHECK_IF_ENABLED_ERR,
  ARAD_PP_FRWRD_BMACT_SA_AUTH_ENABLED_ERR,
  ARAD_PP_FRWRD_BMACT_PCP_PROFILE_NDX_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_BMACT_ERR_LAST
} ARAD_PP_FRWRD_BMACT_ERR;

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
*   arad_pp_frwrd_bmact_init_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   Init device to support Mac in mac.
*   User cannot disable it once configured.
*   Only reboot can reset the configurations.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
*   arad_pp_is_mac_in_mac_enabled
* TYPE:
*   PROC
* FUNCTION:
*   Shows whether Mac in mac is enabled on the device.
* INPUT:
*   SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*   SOC_SAND_OUT uint8                                 *enabled -
*     Shows if Mac in mac is enabled or not.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_is_mac_in_mac_enabled(
  SOC_SAND_IN  int     unit,
  SOC_SAND_OUT  uint8     *enabled
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_mac_in_mac_enable
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set Mac-in-Mac TPID profile. This also shows Mac-in_mac is
 *   enbled on the device.
 * INPUT:
 *   SOC_SAND_IN  int           unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   none.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_mac_in_mac_enable(
    SOC_SAND_IN  int           unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_bvid_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the B-VID bridging attributes. Backbone Mac
 *   addresses that do not serve as MyMAC for I-components
 *   that are processed according to their B-VID
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                           bvid_ndx -
 *     Backbone VID
 *   SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                     *bvid_info -
 *     B-VID attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_bvid_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx,
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                *bvid_info
  );

uint32
  arad_pp_frwrd_bmact_bvid_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx,
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                *bvid_info
  );

uint32
  arad_pp_frwrd_bmact_bvid_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_bmact_bvid_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_bmact_bvid_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_bvid_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                    bvid_ndx,
    SOC_SAND_OUT SOC_PPC_BMACT_BVID_INFO                *bvid_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_pbb_te_bvid_range_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the BVID range for Traffic Engineered Provider
 *   Backbone Bridging
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE              *pbb_te_bvids -
 *     Range of B-VIDs, to be used as PBB-TE services
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE         *pbb_te_bvids
  );

uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE         *pbb_te_bvids
  );

uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_bmact_pbb_te_bvid_range_set_unsafe" API.
 *     Refer to
 *     "arad_pp_frwrd_bmact_pbb_te_bvid_range_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_pbb_te_bvid_range_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_BMACT_PBB_TE_VID_RANGE         *pbb_te_bvids
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_entry_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to the B-MACT DB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key -
 *     B-VID and B-MAC
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO                    *bmact_entry_info -
 *     B-MACT entry attributes
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     Exact Match table
 * REMARKS:
 *   Unless PBB-TE is expected to be called by the B-MAC
 *   learning process.
 *   For ARAD, BMACT forwrading and learning information are located on two
 *   different logical databases. BMACT DB for forwarding and MIM Tunnel
 *   Learn information DB for learning in case of Termination.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO               *bmact_entry_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
  );

uint32
  arad_pp_frwrd_bmact_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO               *bmact_entry_info
  );

uint32
  arad_pp_frwrd_bmact_key_parse(
    SOC_SAND_IN   int                                unit,
    SOC_SAND_IN   ARAD_PP_LEM_ACCESS_KEY                *key,
    SOC_SAND_OUT  SOC_PPC_BMACT_ENTRY_KEY               *bmac_key
  );

uint32
  arad_pp_frwrd_bmact_payload_convert(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  uint32                             flags,
    SOC_SAND_IN  ARAD_PP_LEM_ACCESS_PAYLOAD         *payload,
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO           *bmact_entry_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_entry_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry from the B-MACT DB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key -
 *     B-VID and B-MAC
 * REMARKS:
 *   Unless PBB-TE is expected to be called by the B-MAC
 *   learning process
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key
  );

uint32
  arad_pp_frwrd_bmact_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_entry_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry from the B-MACT DB.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key -
 *     B-VID and B-MAC
 *   SOC_SAND_OUT  SOC_PPC_BMACT_ENTRY_INFO                   *bmact_entry_info -
 *     B-MACT entry attributes
 *   SOC_SAND_OUT  uint8                                *found -
 *     FALSE: the entry was not found
 * REMARKS:
 *   Unless PBB-TE is expected to be called by the B-MAC
 *   learning process
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_bmact_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key,
    SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO               *bmact_entry_info,
    SOC_SAND_OUT uint8                                  *found
  );

uint32
  arad_pp_frwrd_bmact_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                *bmac_key
  );

uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_set_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_set_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
    );

uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_get_unsafe(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

uint32
  arad_pp_frwrd_bmact_eg_vlan_pcp_map_get_verify(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
    );
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_bmact module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_bmact_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_bmact_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_bmact module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_bmact_get_errs_ptr(void);

uint32
  SOC_PPC_BMACT_BVID_INFO_verify(
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO *info
  );

uint32
  SOC_PPC_BMACT_PBB_TE_VID_RANGE_verify(
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE *info
  );

uint32
  SOC_PPC_BMACT_ENTRY_KEY_verify(
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY *info
  );

uint32
  SOC_PPC_BMACT_ENTRY_INFO_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_BMACT_INCLUDED__*/
#endif
