/* $Id: ppd_api_vsi.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_vsi.h
*
* MODULE PREFIX:  soc_ppd_vsi
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

#ifndef __SOC_PPD_API_VSI_INCLUDED__
/* { */
#define __SOC_PPD_API_VSI_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_vsi.h>

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
  SOC_PPD_VSI_MAP_ADD = SOC_PPD_PROC_DESC_BASE_VSI_FIRST,
  SOC_PPD_VSI_MAP_ADD_PRINT,
  SOC_PPD_VSI_MAP_REMOVE,
  SOC_PPD_VSI_MAP_REMOVE_PRINT,
  SOC_PPD_VSI_INFO_SET,
  SOC_PPD_VSI_INFO_SET_PRINT,
  SOC_PPD_VSI_INFO_GET,
  SOC_PPD_VSI_INFO_GET_PRINT,
  SOC_PPD_VSI_GET_PROCS_PTR,
  SOC_PPD_VSI_EGRESS_PROFILE_SET,
  SOC_PPD_VSI_EGRESS_PROFILE_SET_PRINT,
  SOC_PPD_VSI_EGRESS_PROFILE_GET,
  SOC_PPD_VSI_EGRESS_PROFILE_GET_PRINT,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_VSI_PROCEDURE_DESC_LAST
} SOC_PPD_VSI_PROCEDURE_DESC;

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
 *   soc_ppd_vsi_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Virtual Switch Instance information. After
 *   setting the VSI, the user may attach L2 Logical
 *   Interfaces to it: ACs; PWEs
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx -
 *     System VSID. Range: 0-64K
 *   SOC_SAND_IN  SOC_PPC_VSI_INFO                            *vsi_info -
 *     VSI attributes
 * REMARKS:
 *   - Default forwarding destination:T20E: The destination
 *   is fully configurablePetra-B: The destination must
 *   correspond to one of the action pointers configured by
 *   soc_ppd_frwrd_mact_vsi_default_info_set()- Soc_petra-B Flooding:
 *   When the flooding multicast ID mapping from the local
 *   VSI is insufficient, the user may either set the
 *   destination as FEC ID with multicast destination, or
 *   utilize the ingress multicast table to remap the MID.-
 *   Soc_petra-B: When the local VSI is > 4K: The EEI cannot be
 *   used as I-SID; Topology ID. Enable-My-MAC and Enable
 *   routing have to be negated. The FID is the VSID.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_vsi_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_IN  SOC_PPC_VSI_INFO                            *vsi_info
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_vsi_info_set"
 *     API.
 *     Refer to "soc_ppd_vsi_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_vsi_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsi_ndx,
    SOC_SAND_OUT SOC_PPC_VSI_INFO                            *vsi_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_vsi_egress_mtu_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set mtu val to vsi.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               vsi_profile_ndx -
 *     vsi profile index. Range: 1-3
 *   SOC_SAND_IN  SOC_SAND_IN  SOC_PPD_MTU               mtu_val -
 *     mtu value to set into the profile.
 * REMARKS:
 * RETURNS:
 *   OK or ERROR indication.
********************************************************************/
uint32 soc_ppd_vsi_egress_mtu_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint8                                is_forwarding_mtu_filter,
    SOC_SAND_IN  uint32                               vsi_profile_ndx,
    SOC_SAND_IN  uint32                               mtu_val
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_vsi_egress_vsi_set" API.
 *     Refer to "soc_ppd_vsi_egress_vsi_set" API for details.
*********************************************************************/
uint32
  soc_ppd_vsi_egress_mtu_get(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint8              is_forwarding_mtu_filter,
    SOC_SAND_IN  uint32             vsi_profile_ndx,
    SOC_SAND_OUT uint32             *mtu_val
  );

/*********************************************************************
 * NAME:
 *   soc_ppd_vsi_l2cp_trap_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *     Sets Trap information for Layer 2 control protocol
 *     frames. Packet is an MEF layer 2 control protocol
 *     service frame When DA matches 01-80-c2-00-00-XX where XX
 * = 8'b00xx_xxxx.
 *     Details: in the H file. (search for prototype)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_L2CP_KEY                 *l2cp_key -
 *     profile and da key.
 *   SOC_SAND_IN  SOC_PPC_VSI_L2CP_HANDLE_TYPE         handle_type -
 *     type (trap/drop/normal/peer).
 * REMARKS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_vsi_l2cp_trap_set(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_VSI_L2CP_KEY                       *l2cp_key,
    SOC_SAND_IN  SOC_PPC_VSI_L2CP_HANDLE_TYPE               handle_type
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_vsi_l2cp_trap_set" API.
 *     Refer to "soc_ppd_vsi_l2cp_trap_set" API for details.
*********************************************************************/
uint32
  soc_ppd_vsi_l2cp_trap_get(
    SOC_SAND_IN  int                                     unit,
    SOC_SAND_IN  SOC_PPC_VSI_L2CP_KEY                       *l2cp_key,
    SOC_SAND_OUT SOC_PPC_VSI_L2CP_HANDLE_TYPE               *handle_type
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_VSI_INCLUDED__*/
#endif

