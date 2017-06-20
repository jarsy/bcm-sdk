/* $Id: ppd_api_frwrd_ipv4.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_ipv4.h
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

#ifndef __SOC_PPD_API_FRWRD_IPV4_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_IPV4_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_frwrd_ipv4.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>

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
  SOC_PPD_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_IPV4_FIRST,
  SOC_PPD_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_SET_PRINT,
  SOC_PPD_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_GET,
  SOC_PPD_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_GET_PRINT,
  SOC_PPD_FRWRD_IP_ROUTES_CACHE_COMMIT,
  SOC_PPD_FRWRD_IP_ROUTES_CACHE_COMMIT_PRINT,
  SOC_PPD_FRWRD_IPV4_GLBL_INFO_SET,
  SOC_PPD_FRWRD_IPV4_GLBL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_IPV4_GLBL_INFO_GET,
  SOC_PPD_FRWRD_IPV4_GLBL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_ADD,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_GET,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_REMOVE,
  SOC_PPD_FRWRD_IPV4_UC_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV4_UC_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_IPV4_UC_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV4_HOST_ADD,
  SOC_PPD_FRWRD_IPV4_HOST_ADD_PRINT,
  SOC_PPD_FRWRD_IPV4_HOST_GET,
  SOC_PPD_FRWRD_IPV4_HOST_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_HOST_GET_BLOCK,
  SOC_PPD_FRWRD_IPV4_HOST_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV4_HOST_REMOVE,
  SOC_PPD_FRWRD_IPV4_HOST_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_ADD,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_GET,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_REMOVE,
  SOC_PPD_FRWRD_IPV4_MC_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV4_MC_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_IPV4_MC_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_INFO_SET,
  SOC_PPD_FRWRD_IPV4_VRF_INFO_SET_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_INFO_GET,
  SOC_PPD_FRWRD_IPV4_VRF_INFO_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_ADD,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_GET,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_REMOVE,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_IPV4_VRF_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV4_VRF_ALL_ROUTING_TABLES_CLEAR,
  SOC_PPD_FRWRD_IPV4_VRF_ALL_ROUTING_TABLES_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV4_MEM_STATUS_GET,
  SOC_PPD_FRWRD_IPV4_MEM_STATUS_GET_PRINT,
  SOC_PPD_FRWRD_IPV4_MEM_DEFRAGE,
  SOC_PPD_FRWRD_IPV4_MEM_DEFRAGE_PRINT,
  SOC_PPD_FRWRD_IPV4_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_IPV4_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_IPV4_PROCEDURE_DESC;

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
 *   soc_ppd_frwrd_ip_routes_cache_mode_enable_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set for which IP routes (IPv4/6 UC/MC) to enable caching
 *   by SW
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 0 - 255. Use VRF 0 for default
 *     routing table. Use SOC_PPC_FRWRD_IP_ALL_VRFS_ID to apply
 *     setting to all VRFs.
 *   SOC_SAND_IN  uint32                                route_types -
 *     IP Routes to enable caching for them, use
 *     SOC_SAND_PP_IP_TYPE to select which IP types to enable
 *     caching for them.
 * REMARKS:
 *   - Caching is supported only For IPv4 UC.- For route
 *   types which caching was enabled for them, the Add/remove
 *   operation will affect the SW only.- when cache is
 *   disabled then routes already cached will not be commited
 *   (till calling soc_ppd_frwrd_ip_routes_cache_commit) - use
 *   soc_ppd_frwrd_ip_routes_commit() to synchronize the HW with
 *   the changes made in the SW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ip_routes_cache_mode_enable_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  uint32                                route_types
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_ip_routes_cache_mode_enable_set" API.
 *     Refer to "soc_ppd_frwrd_ip_routes_cache_mode_enable_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_ip_routes_cache_mode_enable_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_OUT uint32                                *route_types
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ip_routes_cache_commit
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Synchronize the routing table in the Data plane (HW)
 *   with the routing table in the control plane (SW)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                route_types -
 *     IP Routes to commit changes for them. use
 *     SOC_SAND_PP_IP_TYPE to select which IP types to commit into
 *     Hardware
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 0 - 255. Use VRF 0 for default
 *     routing table. Use SOC_PPC_FRWRD_IP_ALL_VRFS_ID to apply
 *     setting to all VRFs.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Commit may fail if LPM or Host DB (LEM) has no available
 *     entries. In order to examine what is the status of the
 *     SW/HW DBs use soc_ppd_frwrd_ipv4_uc_route_get_block() (or
 *     VRF/MC/IPv6 relevant get_block functions)
 * REMARKS:
 *   - When user select to update the SW only (using
 *   soc_ppd_frwrd_ip_routes_cache_mode_set), this API comes to
 *   synchronize the HW with the changes made in the SW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ip_routes_cache_commit(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                route_types,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_glbl_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the IP routing (including
 *   resources to use)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_glbl_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_ipv4_glbl_info_set" API.
 *     Refer to "soc_ppd_frwrd_ipv4_glbl_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_glbl_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv4 route entry to the routing table. Binds between
 *   Ipv4 Unicast route key (IPv4-address/prefix) and a FEC
 *   entry identified by fec_id for a given router. As a
 *   result of this operation, Unicast Ipv4 packets
 *   designated to the IP address matching the given key (as
 *   long there is no more-specific route key) will be routed
 *   according to the information in the FEC entry identified
 *   by fec_id.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key -
 *     The routing key: IPv4 subnet
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the routing DB (LEM/LPM).
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                               exact_match -
 *     If TRUE returns exact match only; if FALSE returns
 *     longest prefix match.
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_id -
 *     Routing information (system-fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *location -
 *     Indicates whether the returned entry exsit in host table
 *     or LPM/TCAM. Relvant only if found is TRUE. If the entry
 *     is not commited yet then this is the location the route
 *     will be inserted into
 *   SOC_SAND_OUT uint8                               *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - the lookup is performed in the SW shadow. - Found is
 *   always TRUE if exact_match is FALSE, due to the default
 *   route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  uint8                               exact_match,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_id,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *location,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv4 UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_keys -
 *     Array of routing keys (IPv4 subnets)
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids -
 *     Array of fec-ids.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location -
 *     For each route Indicates whether it exsits in host table
 *     or LPM/TCAM. If the entry is not commited yet then this
 *     is the location the route will be inserted into. Set
 *     this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of entries in returned arrays.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_keys,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_iterator_table_is_initiated
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Check whether the table is initiated.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN SOC_DPP_DBAL_SW_TABLE_IDS         table_id -
 *     The SW table to iterate on.
 *   SOC_SAND_OUT int                             *is_table_initiated -
 *     Is the table initiated
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_iterator_table_is_initiated(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_DPP_DBAL_SW_TABLE_IDS        table_id,
    SOC_SAND_OUT int                             *is_table_initiated
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_iterator_get_next
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the next entry from the table iterator.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN SOC_DPP_DBAL_SW_TABLE_IDS         table_id -
 *     The SW table to iterate on.
 *   SOC_SAND_IN  uint32                           flags -
 *     The flags passed to the table traverse
 *   SOC_SAND_OUT SOC_PPC_FP_QUAL_VAL              qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX] -
 *     Array of qualifiers.
 *   SOC_SAND_OUT void                            *payload -
 *     The entry payload.
 *   SOC_SAND_OUT uint32                          *priority -
 *     Entry priority.
 *   SOC_SAND_OUT uint32                          *hit_bit -
 *     Whether the entry was accessed.
 *   SOC_SAND_OUT uint8                           *found -
 *     Entry found.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_iterator_get_next(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_DPP_DBAL_SW_TABLE_IDS        table_id,
    SOC_SAND_IN  uint32                           flags,
    SOC_SAND_OUT SOC_PPC_FP_QUAL_VAL              qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
    SOC_SAND_OUT void                            *payload,
    SOC_SAND_OUT uint32                          *priority,
    SOC_SAND_OUT uint8                           *hit_bit,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_iterator_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initialize table iterator.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN SOC_DPP_DBAL_SW_TABLE_IDS         table_id -
 *     The SW table to iterate on.
 *   SOC_SAND_OUT SOC_DPP_DBAL_PHYSICAL_DB_TYPES    physical_db_type -
 *     The HW DB in which the SW table resides, important for the allowed types of payload.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_iterator_init(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_DPP_DBAL_SW_TABLE_IDS        table_id,
    SOC_SAND_OUT SOC_DPP_DBAL_PHYSICAL_DB_TYPES    *physical_db_type
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_iterator_deinit
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Free table iterator.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN SOC_DPP_DBAL_SW_TABLE_IDS         table_id -
 *     The SW table to iterate on.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_iterator_deinit(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_DPP_DBAL_SW_TABLE_IDS        table_id
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds. Remove operation may
 *     fail if there is no place in the IPv4 DB (LPM), due to
 *     fail of decompression.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.- remove un-exist entry considered
 *   as succeeded operation
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_route_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_uc_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the IPv4 UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_uc_routing_table_clear(
    SOC_SAND_IN  int                               unit
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_host_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv4 entry to the Host table. Binds between Host and
 *   next hop information.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key -
 *     VRF-ID and IP address
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info -
 *     Routing information. See remarks.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the LEM DB.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW. - Supports virtual routing tables
 *   using the vrf_id - user has to supply in routing info
 *   FEC-ptr which set the out-RIF, TM-destination and EEP.
 *   Optionally, user can supply EEP directly from the host
 *   table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_host_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_host_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information associated with the given
 *   route key on VRF.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key -
 *     The routing key (IPv4 address and a virtual router id)
 *   SOC_SAND_IN  uint32                                  flags -
 *     flags, see SOC_PPD_FRWRD_IP_HOST_
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info -
 *     Routing information (fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location -
 *     Indicates whether the returned entry exsit in host table
 *     or LPM/TCAM. Relvant only if found is TRUE. If the entry
 *     is not commited yet then this is the location the route
 *     will be inserted into
 *   SOC_SAND_OUT uint8                           *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - If the host is not found in the host DB, then the host
 *   (exact route/32) is lookuped in the LPM
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_host_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_host_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the host table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range_key -
 *     Defines the range of routing entries to retrieve.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routes_info -
 *     Array of routing information for each host key.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                            *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - The lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_host_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range_key,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_host_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv4 route entry from the routing table of a
 *   virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_host_remove(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mc_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv4 MC route entry to the routing table. Binds
 *   between Ipv4 Unicast route key (IPv4-address/prefix) and
 *   a FEC entry identified by fec_id for a given router. As
 *   a result of this operation, Unicast Ipv4 packets
 *   designated to IP address matching the given key (as long
 *   there is no more-specific route key) will be routed
 *   according to the information in the FEC entry identified
 *   by fec_id.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key -
 *     The routing key IPv4 subnet
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info -
 *     Routing information (FEC-id, or MC-group).
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no available place.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mc_route_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mc_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                               flags -
 *     see SOC_PPD_FRWRD_IP_ flags:
 *     SOC_PPC_FRWRD_IP_EXACT_MATCH: If present returns exact match only; if FALSE returns
 *     longest prefix match.
 *     SOC_PPC_FRWRD_IP_CLEAR_ON_GET: clear hit indication. (ARAD only)
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info -
 *     Routing information (FEC-id, or MC-group).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location -
 *     Indicates whether the returned entry exsit in host table
 *     or LPM/TCAM. Relvant only if found is TRUE. If the entry
 *     is not commited yet then this is the location the route
 *     will be inserted into
 *   SOC_SAND_OUT uint8                           *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - Found is always TRUE if exact_match is FALSE, due to
 *   the default route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mc_route_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  uint8                           flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mc_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv4 MC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_keys -
 *     Array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *routes_info -
 *     Routing information (FEC-id, or MC-group).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location -
 *     For each route Indicates whether it exsits in host table
 *     or LPM/TCAM. If the entry is not commited yet then this
 *     is the location the route will be inserted into. Set
 *     this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                            *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - the lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mc_route_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mc_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   PE uint32 soc_ppd_frwrd_ipv4_mc_route_remove( SOC_SAND_IN
 *   uint32 unit, SOC_SAND_IN SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY
 *   *route_key, SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE *success);
 *   Remove IPv4 route entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mc_route_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mc_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the IPv4 MC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mc_routing_table_clear(
    SOC_SAND_IN  int                               unit
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipmc_rp_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPMC RP. 
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO             *rp_info -
 *     RP-ID and active l3-intf
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW. - Supports virtual routing tables
 *   using the vrf_id - user has to supply in routing info
 *   FEC-ptr which set the out-RIF, TM-destination and EEP.
 *   Optionally, user can supply EEP directly from the host
 *   table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipmc_rp_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO      *rp_info
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipmc_rp_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the host table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range_key -
 *     Defines the range of routing entries to retrieve.
 *   SOC_SAND_IN  uint32                                  rp_id -
 *    RP-ID
 *   SOC_SAND_OUT uint32      *l3_intfs -
 *     Array of active l3-interfaces on given RP-ID
 *   SOC_SAND_OUT uint32                            *nof_entries -
 *     Number of entries in returned Arrays.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipmc_rp_get_block(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  rp_id,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE            *block_range_key,
    SOC_SAND_OUT int32                                  *l3_intfs,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipmc_rp_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPMC route entry from the routing table of a
 *   virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO             *rp_info -
 *     The routing key (IPMC subnet and a virtual router id)
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipmc_rp_remove(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO      *rp_info
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the VRF including
 *   (defaults forwarding).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VRF_INFO                 *vrf_info -
 *     VRF information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VRF_INFO                 *vrf_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_ipv4_vrf_info_set" API.
 *     Refer to "soc_ppd_frwrd_ipv4_vrf_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VRF_INFO                 *vrf_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv4 route entry to the virtual routing table (VRF).
 *   Binds between Ipv4 route key (UC/MC IPv4-address\prefix)
 *   and a FEC entry identified by fec_id for a given virtual
 *   router. As a result of this operation, Unicast Ipv4
 *   packets designated to IP address matching the given key
 *   (as long there is no more-specific route key) will be
 *   routed according to the information in the FEC entry
 *   identified by fec_id.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key -
 *     The routing key IPv4 subnet MC/UC.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the ILM DB (LEM/LPM).
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW. - For VPN routing, there is no
 *   difference in the lookup functionality between IPV4 MC
 *   and Ipv4 UC. - Supports virtual routing tables using the
 *   vrf_id
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_route_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id,
    SOC_SAND_IN  uint32                                      flags,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key on VRF.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                               exact_match -
 *     If TRUE returns exact match only; if FALSE returns
 *     longest prefix match.
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_id -
 *     Routing information (system-fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *location -
 *     Indicates whether the returned entry exsit in host table
 *     or LPM/TCAM. Relvant only if found is TRUE. If the entry
 *     is not commited yet then this is the location the route
 *     will be inserted into
 *   SOC_SAND_OUT uint8                               *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - Found is always TRUE if exact_match is FALSE, due to
 *   the default route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_route_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_IN  uint8                               exact_match,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_id,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *location,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing table of a virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids -
 *     Array of fec-ids
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location -
 *     For each route Indicates whether it exsits in host table
 *     or LPM/TCAM. If the entry is not commited yet then this
 *     is the location the route will be inserted into. Set
 *     this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - the lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_route_get_block(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_keys,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location,
    SOC_SAND_OUT uint32                                *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv4 route entry from the routing table of a
 *   virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeded.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_route_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear IPv4 routing table of VRF
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_routing_table_clear(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_vrf_all_routing_tables_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear IPv4 routing tables for all VRFs.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_vrf_all_routing_tables_clear(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                                 flags
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mem_status_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the status and usage of memory.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                       mem_ndx -
 *     memory/table id. Range: 0 - 4.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MEM_STATUS      *mem_status -
 *     Status and usage of memory
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mem_status_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                       mem_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MEM_STATUS      *mem_status
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mem_defrage
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the status and usage of memory.
 * INPUT:
 *   SOC_SAND_IN  int                      unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                       mem_ndx -
 *     memory/table id. Range: 0 - 4.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *defrag_info -
 *     Defragment parameters, determine how many iterations and
 *     table access to perform before this call returns.
 * REMARKS:
 *   for Soc_petra-B can be called only if according to operation mode defragement is enabled
 *   see SOC_PB_PP_MGMT_IPV4_OP_MODE_SUPPORT_DEFRAG, soc_pb_pp_mgmt_operation_mode_set
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mem_defrage(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                       mem_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *defrag_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv4_mc_ssm_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv4 MC ssm table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_keys -
 *     Array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *routes_info -
 *     Routing information (FEC-id, or MC-group).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location -
 *     For each route Indicates whether it exsits in host table
 *     or LPM/TCAM. If the entry is not commited yet then this
 *     is the location the route will be inserted into. Set
 *     this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                            *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - the lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv4_mc_ssm_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_IPV4_INCLUDED__*/
#endif

