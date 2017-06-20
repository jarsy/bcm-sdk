/* $Id: ppd_api_lag.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_lag.h
*
* MODULE PREFIX:  soc_ppd_lag
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

#ifndef __SOC_PPD_API_LAG_INCLUDED__
/* { */
#define __SOC_PPD_API_LAG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_lag.h>

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
  SOC_PPD_LAG_SET = SOC_PPD_PROC_DESC_BASE_LAG_FIRST,
  SOC_PPD_LAG_SET_PRINT,
  SOC_PPD_LAG_GET,
  SOC_PPD_LAG_GET_PRINT,
  SOC_PPD_LAG_MEMBER_ADD,
  SOC_PPD_LAG_MEMBER_ADD_PRINT,
  SOC_PPD_LAG_MEMBER_REMOVE,
  SOC_PPD_LAG_MEMBER_REMOVE_PRINT,
  SOC_PPD_LAG_HASHING_GLOBAL_INFO_SET,
  SOC_PPD_LAG_HASHING_GLOBAL_INFO_SET_PRINT,
  SOC_PPD_LAG_HASHING_GLOBAL_INFO_GET,
  SOC_PPD_LAG_HASHING_GLOBAL_INFO_GET_PRINT,
  SOC_PPD_LAG_HASHING_PORT_INFO_SET,
  SOC_PPD_LAG_HASHING_PORT_INFO_SET_PRINT,
  SOC_PPD_LAG_HASHING_PORT_INFO_GET,
  SOC_PPD_LAG_HASHING_PORT_INFO_GET_PRINT,
  SOC_PPD_LAG_HASHING_MASK_SET,
  SOC_PPD_LAG_HASHING_MASK_SET_PRINT,
  SOC_PPD_LAG_HASHING_MASK_GET,
  SOC_PPD_LAG_HASHING_MASK_GET_PRINT,
  SOC_PPD_LAG_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_LAG_PROCEDURE_DESC_LAST
} SOC_PPD_LAG_PROCEDURE_DESC;

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
 *   soc_ppd_lag_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure a LAG. A LAG is defined by a group of System
 *   Physical Ports that compose it. This configuration
 *   affects 1. LAG resolution: when the destination of
 *   packet is LAG 2. Learning: when packet source port
 *   belongs to LAG, then the LAG is learnt.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                lag_ndx -
 *     LAG ID. Range: 0 - 255.
 *   SOC_SAND_IN  SOC_PPC_LAG_INFO                            *lag_info -
 *     Lag members. Maximal number of out-going LAG members is
 *     16. The number of incoming LAG members is not limited,
 *     and it can be the number of Local FAP ports in each
 *     device.
 * REMARKS:
 *   1. Local to system port mapping must be configured
 *   before using this API (Incoming and Outgoing) - for LAG
 *   pruning. 2. LAG configuration must be consistent
 *   system-wide, for incoming and outgoing ports. 3. The
 *   member index inside the LAG (0-255) is defined by the
 *   index of the appropriate port in the members array. 4.
 *   Setting LAG with a group of system ports, will first
 *   clean-up any previous configuration of the LAG. For
 *   example, setting LAG 1 with system members 1,2,3,4 and
 *   then setting the same LAG with members 3,4,5,6 will
 *   clean up the effect of the previous configuration and
 *   set up the new configuration.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lag_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_INFO                            *lag_info
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_lag_set" API.
 *     Refer to "soc_ppd_lag_set" API for details.
*********************************************************************/
uint32
  soc_ppd_lag_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                lag_ndx,
    SOC_SAND_OUT SOC_PPC_LAG_INFO                            *lag_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lag_member_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add a system port as a member in LAG.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                lag_ndx -
 *     LAG ID. Range: 0 - 255.
 *   SOC_SAND_IN  SOC_PPC_LAG_MEMBER                          *member -
 *     System port to be added as a member, and the
 *     member-index.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds (upon add). Add operation
 *     may fail if there is no place in the SA Auth DB.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lag_member_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER                          *member,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lag_member_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a system port from a LAG.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                lag_ndx -
 *     LAG ID. Range: 0 - 255.
 *   SOC_SAND_IN  uint32                                sys_port -
 *     System port to be removed as a member.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lag_member_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               lag_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_MEMBER                   *member
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lag_hashing_global_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the LAG hashing global attributes
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LAG_HASH_GLOBAL_INFO                *glbl_hash_info -
 *     LAG Hashing global settings
 * REMARKS:
 *   The hashing function result is 8 bit value. The TM use
 *   the value to choose the LAG port member, to which the
 *   packet is sent
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lag_hashing_global_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_GLOBAL_INFO                *glbl_hash_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lag_hashing_global_info_set" API.
 *     Refer to "soc_ppd_lag_hashing_global_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_lag_hashing_global_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_LAG_HASH_GLOBAL_INFO                *glbl_hash_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lag_hashing_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the LAG hashing per-lag attributes
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                port_ndx -
 *     In-PP Port ID
 *   SOC_SAND_IN  SOC_PPC_LAG_HASH_PORT_INFO                  *lag_hash_info -
 *     LAG Hashing per-ingress port settings
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lag_hashing_port_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_IN  SOC_PPC_LAG_HASH_PORT_INFO                  *lag_hash_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lag_hashing_port_info_set" API.
 *     Refer to "soc_ppd_lag_hashing_port_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_lag_hashing_port_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                port_ndx,
    SOC_SAND_OUT SOC_PPC_LAG_HASH_PORT_INFO                  *lag_hash_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_lag_hashing_mask_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the key used by hash functions for LAG/ECMP load
 *   balancing.
 * INPUT:
 *   SOC_SAND_IN  int                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_HASH_MASK_INFO       *mask_info -
 *     how to build the key used as input by hash functions for
 *     LAG/ECMP load balancing
 * REMARKS:
 *   This setting is mutual to the ECMP hashing function
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_lag_hashing_mask_set(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  SOC_PPC_HASH_MASK_INFO       *mask_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_lag_hashing_mask_set" API.
 *     Refer to "soc_ppd_lag_hashing_mask_set" API for details.
*********************************************************************/
uint32
  soc_ppd_lag_hashing_mask_get(
    SOC_SAND_IN  int                unit,
    SOC_SAND_OUT SOC_PPC_HASH_MASK_INFO       *mask_info
  );


uint32
  soc_ppd_lag_lb_key_range_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_LAG_INFO                            *lag_info);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_LAG_INCLUDED__*/
#endif

