/* $Id: ppd_api_frwrd_ipv6.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_ipv6.h
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

#ifndef __SOC_PPD_API_FRWRD_IPV6_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_IPV6_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_frwrd_ipv6.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ipv4.h>

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
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_ADD = SOC_PPD_PROC_DESC_BASE_FRWRD_IPV6_FIRST,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_GET,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_REMOVE,
  SOC_PPD_FRWRD_IPV6_UC_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV6_UC_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_IPV6_UC_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_ADD,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_GET,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_REMOVE,
  SOC_PPD_FRWRD_IPV6_MC_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV6_MC_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_IPV6_MC_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_INFO_SET,
  SOC_PPD_FRWRD_IPV6_VRF_INFO_SET_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_INFO_GET,
  SOC_PPD_FRWRD_IPV6_VRF_INFO_GET_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_ADD,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_GET,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_REMOVE,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_IPV6_VRF_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV6_VRF_ALL_ROUTING_TABLES_CLEAR,
  SOC_PPD_FRWRD_IPV6_VRF_ALL_ROUTING_TABLES_CLEAR_PRINT,
  SOC_PPD_FRWRD_IPV6_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_IPV6_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_IPV6_PROCEDURE_DESC;

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
 *   soc_ppd_frwrd_ipv6_uc_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv6 route entry to the routing table. Binds between
 *   Ipv6 Unicast route key (IPv6-address/prefix) and a FEC
 *   entry identified by fec_id for a given virtual router.
 *   As a result of this operation, Unicast Ipv6 packets
 *   designated to IP address matching the given key (as long
 *   there is no more-specific route key) will be routed
 *   according to the information in the FEC entry identified
 *   by fec_id.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_key -
 *     The routing key IPv6 subnet
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                          fec_id -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the ILM DB (LEM/LPM).
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_uc_route_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO            *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_uc_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_key -
 *     The routing key (IPv6 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                               flags -
 *     see SOC_PPD_FRWRD_IP_ flags:
 *     SOC_PPC_FRWRD_IP_EXACT_MATCH: If present returns exact match only; if FALSE returns
 *     longest prefix match.
 *     SOC_PPC_FRWRD_IP_CLEAR_ON_GET: clear hit indication. (ARAD only)
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                          *fec_id -
 *     Routing information (system-fec-id).
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
 *   - the lookup is performed in the SW shadow. - Found is
 *   always TRUE if exact_match is FALSE, due to the default
 *   route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_uc_route_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  uint8                           flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO             *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_uc_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv6 UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                          *fec_ids -
 *     Array of fec-ids
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
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_uc_route_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO             *route_infos,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_uc_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv6 route entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_key -
 *     The routing key (IPv6 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeded.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_uc_route_remove(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_uc_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the IPv6 UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_uc_routing_table_clear(
    SOC_SAND_IN  int                           unit
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_host_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv6 entry to the Host table. Binds between Host and
 *   next hop information.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                        vrf_ndx -
 *      Virtual router id.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *host_key -
 *     VRF-ID and IP address
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info -
 *     Routing information.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the LEM DB.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_host_add(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  uint32                                   vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *host_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_host_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *     Get an IPv6 entry from the Host table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                        vrf_ndx -
 *      Virtual router id.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *host_key -
 *     VRF-ID and IP address
 *   SOC_SAND_OUT  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info -
 *     Routing information.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the LEM DB.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_host_get(
    SOC_SAND_IN   int                                      unit,
    SOC_SAND_IN   uint32                                   vrf_ndx,
    SOC_SAND_IN   SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *host_key,
    SOC_SAND_OUT  SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info,
    SOC_SAND_OUT  uint8                                   *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_host_delete
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove an IPv6 entry from the Host table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                        vrf_ndx -
 *      Virtual router id.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *host_key -
 *     VRF-ID and IP address
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the LEM DB.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_host_delete(
    SOC_SAND_IN  int                                      unit,
    SOC_SAND_IN  uint32                                   vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_UC_ROUTE_KEY         *host_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_mc_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv6 MC route entry to the routing table. Binds
 *   between Ipv6 Unicast route key (IPv6-address/prefix) and
 *   a FEC entry identified by fec_id for a given virtual
 *   router. As a result of this operation, Unicast Ipv6
 *   packets designated to IP address matching the given key
 *   (as long there is no more-specific route key) will be
 *   routed according to the information in the FEC entry
 *   identified by fec_id.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key -
 *     The routing key IPv6 subnet
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO        *route_info -
 *     Routing information, according to which to route
 *     packets.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the ILM DB (LEM/LPM).
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_mc_route_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_mc_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key -
 *     The routing key (IPv6 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                               flags -
 *     see SOC_PPD_FRWRD_IP_ flags:
 *     SOC_PPC_FRWRD_IP_EXACT_MATCH: If present returns exact match only; if FALSE returns
 *     longest prefix match.
 *     SOC_PPC_FRWRD_IP_CLEAR_ON_GET: clear hit indication. (ARAD only)
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO        *route_info -
 *     Routing information
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
  soc_ppd_frwrd_ipv6_mc_route_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  uint8                           flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_mc_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv6 MC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key -
 *     s - array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO        *routes_info -
 *     Array of fec-ids
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
  soc_ppd_frwrd_ipv6_mc_route_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_MC_ROUTE_INFO        *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_mc_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv6 route entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key -
 *     The routing key (IPv6 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeded.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_mc_route_remove(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_MC_ROUTE_KEY         *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_mc_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the IPv6 MC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_mc_routing_table_clear(
    SOC_SAND_IN  int                           unit
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the VRF including
 *   (defaults forwarding).
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VRF_INFO             *vrf_info -
 *     VRF information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_info_set(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VRF_INFO             *vrf_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_ipv6_vrf_info_set" API.
 *     Refer to "soc_ppd_frwrd_ipv6_vrf_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_info_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_VRF_INFO             *vrf_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv6 route entry to the virtual routing table (VRF).
 *   Binds between Ipv6 route key (UC/MC IPv6-address\prefix)
 *   and a FEC entry identified by fec_id for a given virtual
 *   router. As a result of this operation, Unicast Ipv6
 *   packets designated to IP address matching the given key
 *   (as long there is no more-specific route key) will be
 *   routed according to the information in the FEC entry
 *   identified by fec_id.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_key -
 *     The routing key IPv6 subnet MC/UC.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                          fec_id -
 *     Routing information, according to which to route
 *     packets.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the ILM DB (LEM/LPM).
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW. - For VPN routing there is no
 *   difference in the lookup functionality between IPV6 MC
 *   and Ipv6 UC. - Supports virtual routing tables using the
 *   vrf_id
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_route_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO            *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key on VRF.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_key -
 *     The routing key IPv6 subnet MC/UC.
 *   SOC_SAND_IN  uint8                               flags -
 *     see SOC_PPD_FRWRD_IP_ flags:
 *     SOC_PPC_FRWRD_IP_EXACT_MATCH: If present returns exact match only; if FALSE returns
 *     longest prefix match.
 *     SOC_PPC_FRWRD_IP_CLEAR_ON_GET: clear hit indication. (ARAD only)
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                          *fec_id -
 *     Routing information (system-fec-id).
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
 *   - the lookup is performed in the SW shadow. - Found is
 *   always TRUE if exact_match is FALSE, due to the default
 *   route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_route_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_key,
    SOC_SAND_IN  uint8                                   flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO            *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing table of a virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                          *fec_ids -
 *     Array of fec-ids
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
  soc_ppd_frwrd_ipv6_vrf_route_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE          *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO            *route_infos,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *routes_location,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv6 route entry from the routing table of a
 *   virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_key -
 *     The routing key (IPv6 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeded.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_route_remove(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV6_VPN_ROUTE_KEY        *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear IPv6 routing table of VRF
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_routing_table_clear(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                          vrf_ndx
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_ipv6_vrf_all_routing_tables_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear IPv6 routing tables for all VRFs.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_ipv6_vrf_all_routing_tables_clear(
    SOC_SAND_IN  int                           unit
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_IPV6_INCLUDED__*/
#endif

