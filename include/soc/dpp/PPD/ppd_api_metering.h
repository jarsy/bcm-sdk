/* $Id: ppd_api_metering.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_metering.h
*
* MODULE PREFIX:  soc_ppd_metering
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

#ifndef __SOC_PPD_API_METERING_INCLUDED__
/* { */
#define __SOC_PPD_API_METERING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_metering.h>

#include <soc/dpp/PPD/ppd_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     indicates not to assign meter to the traffic            */

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
  SOC_PPD_MTR_METERS_GROUP_INFO_SET = SOC_PPD_PROC_DESC_BASE_METERING_FIRST,
  SOC_PPD_MTR_METERS_GROUP_INFO_SET_PRINT,
  SOC_PPD_MTR_METERS_GROUP_INFO_GET,
  SOC_PPD_MTR_METERS_GROUP_INFO_GET_PRINT,
  SOC_PPD_MTR_BW_PROFILE_ADD,
  SOC_PPD_MTR_BW_PROFILE_ADD_PRINT,
  SOC_PPD_MTR_BW_PROFILE_GET,
  SOC_PPD_MTR_BW_PROFILE_GET_PRINT,
  SOC_PPD_MTR_BW_PROFILE_REMOVE,
  SOC_PPD_MTR_BW_PROFILE_REMOVE_PRINT,
  SOC_PPD_MTR_METER_INS_TO_BW_PROFILE_MAP_SET,
  SOC_PPD_MTR_METER_INS_TO_BW_PROFILE_MAP_SET_PRINT,
  SOC_PPD_MTR_METER_INS_TO_BW_PROFILE_MAP_GET,
  SOC_PPD_MTR_METER_INS_TO_BW_PROFILE_MAP_GET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_ENABLE_SET,
  SOC_PPD_MTR_ETH_POLICER_ENABLE_SET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_ENABLE_GET,
  SOC_PPD_MTR_ETH_POLICER_ENABLE_GET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_PARAMS_SET,
  SOC_PPD_MTR_ETH_POLICER_PARAMS_SET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_PARAMS_GET,
  SOC_PPD_MTR_ETH_POLICER_PARAMS_GET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_SET,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_SET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_GET,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_GET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_MAP_SET,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_MAP_SET_PRINT,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_MAP_GET,
  SOC_PPD_MTR_ETH_POLICER_GLBL_PROFILE_MAP_GET_PRINT,
  SOC_PPD_METERING_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_METERING_PROCEDURE_DESC_LAST
} SOC_PPD_METERING_PROCEDURE_DESC;

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
 *   soc_ppd_mtr_meters_group_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                               core_id -
 *     Meter Core ID.
 *   SOC_SAND_IN  uint32                            mtr_group_ndx -
 *     Meters Group. Range 0 - 1.
 *   SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO            *mtr_group_info -
 *     Per metering group information
 * REMARKS:
 *   - Relevant only for Soc_petra-B.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_meters_group_info_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  uint32                                mtr_group_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO                *mtr_group_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mtr_meters_group_info_set" API.
 *     Refer to "soc_ppd_mtr_meters_group_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_mtr_meters_group_info_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  uint32                                mtr_group_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_GROUP_INFO                *mtr_group_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mtr_bw_profile_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add Bandwidth Profile and set it attributes
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                               core_id -
 *     Meter Core ID.
 *   SOC_SAND_IN  uint32                                mtr_group_ndx -
 *     Meters Group. In T20E has to be zero. In Soc_petra-B Range 0
 *     - 1.
 *   SOC_SAND_IN  uint32                                bw_profile_ndx -
 *     Bandwidth Profile ID
 *   SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO                 *bw_profile_info -
 *     Bandwidth profile attributes
 *   SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO                 *exact_bw_profile_info -
 *     Exact Bandwidth profile attributes as written to the
 *     device.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds (upon add). Operation may
 *     fail upon unavailable resources. In Soc_petra-B this
 *     operation always success.
 * REMARKS:
 *   This function configures bandwith profile, to assign
 *   meter instance with this profile use
 *   soc_ppd_mtr_meter_ins_to_bw_profile_map_set()- Soc_petra-B if
 *   High-rate metering is enabled then - 0-447 are use for
 *   normal profiles - 448-511 used for high rate profile. if
 *   High-rate metering is disabled then - 0-511 are use for
 *   normal profiles In Normal Profile: Information Rates
 *   (CIR and EIR) are comprised between 64 Kbps and 19 Gbps.
 *   The burst sizes (CBS and EBS) are comprised between 64B
 *   and 1,040,384B. In High-rate Profile: Information Rates
 *   (CIR and EIR) are between 9.6 Gbps and 120 Gbps. The
 *   burst sizes (CBS and EBS) are comprised between 64B and
 *   4,161,536B
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_bw_profile_add(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  uint32                                mtr_group_ndx,
    SOC_SAND_IN  uint32                                bw_profile_ndx, 
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO           *bw_profile_info,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO           *exact_bw_profile_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE              *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mtr_bw_profile_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get Bandwidth Profile attributes
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                   core_id -
 *     Meter Core ID.
 *   SOC_SAND_IN  uint32                                mtr_group_ndx -
 *     Meters Group. In T20E has to be zero. In Soc_petra-B Range 0
 *     - 1.
 *   SOC_SAND_IN  uint32                                bw_profile_ndx -
 *     Bandwidth Profile ID
 *   SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO           *bw_profile_info -
 *     Bandwidth profile attributes
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_bw_profile_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  uint32                                mtr_group_ndx,
    SOC_SAND_IN  uint32                                bw_profile_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO           *bw_profile_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mtr_bw_profile_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove Bandwidth Profile
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                   core_id -
 *     Meter Core ID.
 *   SOC_SAND_IN  uint32                                mtr_group_ndx -
 *     Meters Group. In T20E has to be zero. In Soc_petra-B Range 0
 *     - 1.
 *   SOC_SAND_IN  uint32                                bw_profile_ndx -
 *     Bandwidth Profile ID
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_bw_profile_remove(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  uint32                                mtr_group_ndx,
    SOC_SAND_IN  uint32                                bw_profile_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mtr_meter_ins_to_bw_profile_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set meter attributes by mapping meter instance to
 *   bandwidth profile.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                                   core_id -
 *     Meter Core ID.
 *   SOC_SAND_IN  SOC_PPC_MTR_METER_ID                  *meter_ins_ndx -
 *     Metering Instance ID
 *   SOC_SAND_IN  uint32                                bw_profile_id -
 *     bandwidth profile ID.
 * REMARKS:
 *   - in T20E the group in SOC_PPC_MTR_METER_ID has to be zero-
 *   in Soc_petra-B the meter instance mapped into profile in the
 *   same group the meter instance belongs to.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_meter_ins_to_bw_profile_map_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID                  *meter_ins_ndx,
    SOC_SAND_IN  uint32                                bw_profile_id
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mtr_meter_ins_to_bw_profile_map_set" API.
 *     Refer to "soc_ppd_mtr_meter_ins_to_bw_profile_map_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_mtr_meter_ins_to_bw_profile_map_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                                   core_id,
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID                  *meter_ins_ndx,
    SOC_SAND_OUT uint32                                *bw_profile_id
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mtr_eth_policer_enable_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable / Disable Ethernet policing.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                               enable -
 *     TRUE: Enable Ethernet policing.
 * REMARKS:
 *   - Soc_petra-B only, if called for T20E error is returned.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_eth_policer_enable_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint8                               enable
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mtr_eth_policer_enable_set" API.
 *     Refer to "soc_ppd_mtr_eth_policer_enable_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_mtr_eth_policer_enable_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT uint8                               *enable
  );

/*********************************************************************
* NAME:
 *   soc_ppd_mtr_eth_policer_params_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set policer attributes of the Ethernet policer. Enable
 *   policing per ingress port and Ethernet type.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                port_ndx -
 *     Port ID
 *   SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx -
 *     Ethernet traffic type (UC/BC/...)
 *   SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info -
 *     Policer attributes
 * REMARKS:
 *   - Soc_petra-B only, if called for T20E error is returned.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_mtr_eth_policer_params_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_mtr_eth_policer_params_set" API.
 *     Refer to "soc_ppd_mtr_eth_policer_params_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_mtr_eth_policer_params_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE                        eth_type_ndx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO                 *policer_info
  );

/*********************************************************************
* NAME:         
 *   soc_ppd_mtr_eth_policer_glbl_profile_set
 * TYPE:         
 *   PROC        
 * FUNCTION:       
 *   Set Ethernet policer Global Profile attributes.         
 * INPUT:
 *   SOC_SAND_IN  int               unit - 
 *     Identifier of the device to access.                     
 *   SOC_SAND_IN  uint32                glbl_profile_idx - 
 *     Global Profile index                                    
 *   SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *policer_info - 
 *     Policer attributes                                      
 * REMARKS:         
 *   - uses only cir, cbs, cir_disable fileds from 
 *   SOC_PPC_MTR_BW_PROFILE_INFO to configure policer attributes. 
 * RETURNS:         
 *   OK or ERROR indication.
*********************************************************************/
uint32  
  soc_ppd_mtr_eth_policer_glbl_profile_set(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  uint32                glbl_profile_idx,
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *policer_info
  );

/*********************************************************************
*     Gets the configuration set by the 
 *     "soc_ppd_mtr_eth_policer_glbl_profile_set" API.             
 *     Refer to "soc_ppd_mtr_eth_policer_glbl_profile_set" API for 
 *     details.                                                
*********************************************************************/
uint32  
  soc_ppd_mtr_eth_policer_glbl_profile_get(
    SOC_SAND_IN  int               unit,
    SOC_SAND_OUT uint32                glbl_profile_idx,
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO *policer_info
  );

/*********************************************************************
* NAME:         
 *   soc_ppd_mtr_eth_policer_glbl_profile_map_set
 * TYPE:         
 *   PROC        
 * FUNCTION:       
 *   Map Ethernet policer per ingress port and Ethernet type 
 *   to Ethernet policer Global Profile.                     
 * INPUT:
 *   SOC_SAND_IN  int               unit - 
 *     Identifier of the device to access.                     
 *   SOC_SAND_IN  SOC_PPC_PORT                port_ndx - 
 *     Port ID                                                 
 *   SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx - 
 *     Ethernet traffic type (UC/BC/...)                       
 *   SOC_SAND_IN  uint32                glbl_profile_idx - 
 *     Global Profile index                                    
 * REMARKS:         
 *   None                                                    
 * RETURNS:         
 *   OK or ERROR indication.
*********************************************************************/
uint32  
  soc_ppd_mtr_eth_policer_glbl_profile_map_set(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  SOC_PPC_PORT                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx,
    SOC_SAND_IN  uint32                glbl_profile_idx
  );

/*********************************************************************
*     Gets the configuration set by the 
 *     "soc_ppd_mtr_eth_policer_glbl_profile_map_set" API.         
 *     Refer to "soc_ppd_mtr_eth_policer_glbl_profile_map_set" API 
 *     for details.                                            
*********************************************************************/
uint32  
  soc_ppd_mtr_eth_policer_glbl_profile_map_get(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  SOC_PPC_PORT                port_ndx,
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE        eth_type_ndx,
    SOC_SAND_OUT uint32                *glbl_profile_idx
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_METERING_INCLUDED__*/
#endif

