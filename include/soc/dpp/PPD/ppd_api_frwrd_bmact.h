/* $Id: ppd_api_frwrd_bmact.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_bmact.h
*
* MODULE PREFIX:  soc_ppd_frwrd
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_PPD_API_FRWRD_BMACT_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_BMACT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_frwrd_bmact.h>

#include <soc/dpp/PPD/ppd_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

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
  SOC_PPD_FRWRD_BMACT_BVID_INFO_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_BMACT_FIRST,
  SOC_PPD_FRWRD_BMACT_BVID_INFO_SET_PRINT,
  SOC_PPD_FRWRD_BMACT_BVID_INFO_GET,
  SOC_PPD_FRWRD_BMACT_BVID_INFO_GET_PRINT,
  SOC_PPD_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET,
  SOC_PPD_FRWRD_BMACT_PBB_TE_BVID_RANGE_SET_PRINT,
  SOC_PPD_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET,
  SOC_PPD_FRWRD_BMACT_PBB_TE_BVID_RANGE_GET_PRINT,
  SOC_PPD_FRWRD_BMACT_ENTRY_ADD,
  SOC_PPD_FRWRD_BMACT_ENTRY_ADD_PRINT,
  SOC_PPD_FRWRD_BMACT_ENTRY_REMOVE,
  SOC_PPD_FRWRD_BMACT_ENTRY_REMOVE_PRINT,
  SOC_PPD_FRWRD_BMACT_ENTRY_GET,
  SOC_PPD_FRWRD_BMACT_ENTRY_GET_PRINT,
  SOC_PPD_FRWRD_BMACT_GET_PROCS_PTR,
  SOC_PPD_FRWRD_BMACT_INIT,
  SOC_PPD_FRWRD_BMACT_INIT_PRINT,
  SOC_PPD_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET,
  SOC_PPD_FRWRD_BMACT_EG_VLAN_PCP_MAP_SET_PRINT,
  SOC_PPD_FRWRD_BMACT_EG_VLAN_PCP_MAP_GET,
  SOC_PPD_FRWRD_BMACT_EG_VLAN_PCP_MAP_GET_PRINT,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_BMACT_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_BMACT_PROCEDURE_DESC;

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
*     Init device to support Mac in mac.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_init(
    SOC_SAND_IN  int                               unit
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_bmact_bvid_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the B-VID bridging attributes. Backbone Mac
 *   addresses that do not serve as MyMAC for I-components
 *   that are processed according to their B-VID
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         bvid_ndx -
 *     Backbone VID
 *   SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                     *bvid_info -
 *     B-VID attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_bvid_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         bvid_ndx,
    SOC_SAND_IN  SOC_PPC_BMACT_BVID_INFO                     *bvid_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_bmact_bvid_info_set" API.
 *     Refer to "soc_ppd_frwrd_bmact_bvid_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_bvid_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_SAND_PP_VLAN_ID                         bvid_ndx,
    SOC_SAND_OUT SOC_PPC_BMACT_BVID_INFO                     *bvid_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_bmact_pbb_te_bvid_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the BVID range for Traffic Engineered Provider
 *   Backbone Bridging
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE              *pbb_te_bvids -
 *     Range of B-VIDs, to be used as PBB-TE services
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_pbb_te_bvid_range_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_BMACT_PBB_TE_VID_RANGE              *pbb_te_bvids
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_bmact_pbb_te_bvid_range_set" API.
 *     Refer to "soc_ppd_frwrd_bmact_pbb_te_bvid_range_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_pbb_te_bvid_range_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_BMACT_PBB_TE_VID_RANGE              *pbb_te_bvids
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_bmact_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add an entry to the B-MACT DB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key -
 *     B-VID and B-MAC
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO                    *bmact_entry_info -
 *     B-MACT entry attributes
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
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
  soc_ppd_frwrd_bmact_entry_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_INFO                    *bmact_entry_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_bmact_entry_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an entry from the B-MACT DB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
  soc_ppd_frwrd_bmact_entry_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_bmact_entry_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get an entry from the B-MACT DB.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key -
 *     B-VID and B-MAC
 *   SOC_SAND_OUT  SOC_PPC_BMACT_ENTRY_INFO                 *bmact_entry_info -
 *     B-MACT entry attributes
 *   SOC_SAND_OUT  uint8                              *found -
 *     FALSE: the entry was not found
 * REMARKS:
 *   Unless PBB-TE is expected to be called by the B-MAC
 *   learning process
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_entry_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_BMACT_ENTRY_KEY                     *bmac_key,
	SOC_SAND_OUT SOC_PPC_BMACT_ENTRY_INFO                    *bmact_entry_info,
	SOC_SAND_OUT uint8                               *found
  );
  
/*********************************************************************
* NAME:
*   soc_ppd_frwrd_bmact_eg_vlan_pcp_map_set
* TYPE:
*   PROC
* FUNCTION:
*   Set mapping from COS parameters (DP and TC) to the PCP
*   and DEI values to be set in the transmitted packet's
*   I-tag. This is the mapping to be used when the incoming
*   packet has no tags or pcp profile is set to use TC and
*   DP for the mapping.
* INPUT:
*   SOC_SAND_IN  int                               unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                                pcp_profile_ndx -
*     The PCP profile is set according to OUT-AC setting.
*     Range: 0 - 0.
*   SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx -
*     Traffic Class. Calculated at the ingress. See COS
*     module. Range: 0 - 7.
*   SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx -
*     Drop Precedence. Calculated at the ingress. See COS
*     module. Range: 0 - 3.
*   SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp -
*     The mapped PCP to set in the transmitted packet header.
*     Range: 0 - 7.
*   SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei -
*     The mapped DEI to set in the transmitted packet header.
*     Range: 0 - 1.
* REMARKS:
*   - This mapping is used when the packet has no Tags or
*   pcp profile set to use TC and DP for the mapping. This value of PCP and
*   DEI will be used when the source of PCP DEI is selected
*   to be SOC_PPD_bmac_eg_vlanTAG_PCP_DEI_SRC_MAP.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_eg_vlan_pcp_map_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_IN  SOC_SAND_PP_PCP_UP                          out_pcp,
    SOC_SAND_IN  SOC_SAND_PP_DEI_CFI                         out_dei
  );

/*********************************************************************
*     Gets the configuration set by the
*     "soc_ppd_frwrd_bmact_eg_vlan_pcp_map_set" API.
*     Refer to "soc_ppd_frwrd_bmact_eg_vlan_pcp_map_set" API for
*     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_bmact_eg_vlan_pcp_map_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                pcp_profile_ndx,
    SOC_SAND_IN  SOC_SAND_PP_TC                              tc_ndx,
    SOC_SAND_IN  SOC_SAND_PP_DP                              dp_ndx,
    SOC_SAND_OUT SOC_SAND_PP_PCP_UP                          *out_pcp,
    SOC_SAND_OUT SOC_SAND_PP_DEI_CFI                         *out_dei
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_BMACT_INCLUDED__*/
#endif

