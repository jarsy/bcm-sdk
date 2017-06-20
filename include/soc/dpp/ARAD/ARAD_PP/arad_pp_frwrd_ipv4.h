
/* $Id: arad_pp_frwrd_ipv4.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_IPV4_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_IPV4_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_IPV4_VRF_BITMAP_SIZE (128)

#define JER_PP_KAPS_PAYLOAD_IS_FEC(_payload)     (_payload & (1 << 17))
#define JER_PP_KAPS_PAYLOAD_IS_MC(_payload)      (_payload & (1 << 18))
#define JER_PP_KAPS_PAYLOAD_IS_DEFAULT(_payload) (_payload & (1 << 19))

#define JER_PP_KAPS_FEC_ENCODE(_payload)     (_payload | (1 << 17))
#define JER_PP_KAPS_MC_ENCODE(_payload)      (_payload | (1 << 18))
#define JER_PP_KAPS_DEFAULT_ENCODE(_payload) (_payload | (1 << 19))

#define JER_PP_KAPS_FEC_DECODE(_payload)     (_payload & ~(1 << 17))
#define JER_PP_KAPS_MC_DECODE(_payload)      (_payload & ~(1 << 18))
#define JER_PP_KAPS_DEFAULT_DECODE(_payload) (_payload & ~(1 << 19))


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
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_SET = ARAD_PP_PROC_DESC_BASE_FRWRD_IPV4_FIRST,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_SET_PRINT,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_SET_UNSAFE,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_SET_VERIFY,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_GET,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_GET_PRINT,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_GET_VERIFY,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_MODE_ENABLE_GET_UNSAFE,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_COMMIT,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_COMMIT_PRINT,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_COMMIT_UNSAFE,
  ARAD_PP_FRWRD_IP_ROUTES_CACHE_COMMIT_VERIFY,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_SET,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_SET_PRINT,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_GET,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_GET_PRINT,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_ADD,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_ADD_PRINT,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_ADD_UNSAFE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_ADD_VERIFY,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_PRINT,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_UNSAFE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_VERIFY,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_REMOVE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_REMOVE_PRINT,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_IPV4_UC_ROUTING_TABLE_CLEAR,
  ARAD_PP_FRWRD_IPV4_UC_ROUTING_TABLE_CLEAR_PRINT,
  ARAD_PP_FRWRD_IPV4_UC_ROUTING_TABLE_CLEAR_UNSAFE,
  ARAD_PP_FRWRD_IPV4_UC_ROUTING_TABLE_CLEAR_VERIFY,
  ARAD_PP_FRWRD_IPV4_HOST_ADD,
  ARAD_PP_FRWRD_IPV4_HOST_ADD_PRINT,
  ARAD_PP_FRWRD_IPV4_HOST_ADD_UNSAFE,
  ARAD_PP_FRWRD_IPV4_HOST_ADD_VERIFY,
  ARAD_PP_FRWRD_IPV4_HOST_GET,
  ARAD_PP_FRWRD_IPV4_HOST_GET_PRINT,
  ARAD_PP_FRWRD_IPV4_HOST_GET_UNSAFE,
  ARAD_PP_FRWRD_IPV4_HOST_GET_VERIFY,
  ARAD_PP_FRWRD_IPV4_HOST_GET_BLOCK,
  ARAD_PP_FRWRD_IPV4_HOST_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_IPV4_HOST_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_IPV4_HOST_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_IPV4_HOST_REMOVE,
  ARAD_PP_FRWRD_IPV4_HOST_REMOVE_PRINT,
  ARAD_PP_FRWRD_IPV4_HOST_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_IPV4_HOST_REMOVE_VERIFY,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_ADD,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_ADD_PRINT,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_ADD_UNSAFE,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_ADD_VERIFY,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_PRINT,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_UNSAFE,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_VERIFY,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_REMOVE,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_REMOVE_PRINT,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_IPV4_MC_ROUTE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_IPV4_MC_ROUTING_TABLE_CLEAR,
  ARAD_PP_FRWRD_IPV4_MC_ROUTING_TABLE_CLEAR_PRINT,
  ARAD_PP_FRWRD_IPV4_MC_ROUTING_TABLE_CLEAR_UNSAFE,
  ARAD_PP_FRWRD_IPV4_MC_ROUTING_TABLE_CLEAR_VERIFY,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_SET,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_SET_PRINT,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_GET,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_GET_PRINT,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_IPV4_VRF_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_ADD,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_ADD_PRINT,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_ADD_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_ADD_VERIFY,
  ARAD_PP_FRWRD_IPV4_VRF_IS_SUPPORTED_VERIFY,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_PRINT,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_VERIFY,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_REMOVE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_REMOVE_PRINT,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTING_TABLE_CLEAR,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTING_TABLE_CLEAR_PRINT,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTING_TABLE_CLEAR_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ROUTING_TABLE_CLEAR_VERIFY,
  ARAD_PP_FRWRD_IPV4_VRF_ALL_ROUTING_TABLES_CLEAR,
  ARAD_PP_FRWRD_IPV4_VRF_ALL_ROUTING_TABLES_CLEAR_PRINT,
  ARAD_PP_FRWRD_IPV4_VRF_ALL_ROUTING_TABLES_CLEAR_UNSAFE,
  ARAD_PP_FRWRD_IPV4_VRF_ALL_ROUTING_TABLES_CLEAR_VERIFY,
  SOC_PPC_FRWRD_IPV4_MEM_STATUS_GET,
  SOC_PPC_FRWRD_IPV4_MEM_STATUS_GET_UNSAFE,
  SOC_PPC_FRWRD_IPV4_MEM_STATUS_GET_VERIFY,
  ARAD_PP_FRWRD_IPV4_MEM_DEFRAGE,
  ARAD_PP_FRWRD_IPV4_MEM_DEFRAGE_UNSAFE,
  ARAD_PP_FRWRD_IPV4_MEM_DEFRAGE_VERIFY,
  ARAD_PP_FRWRD_IPV4_GET_PROCS_PTR,
  ARAD_PP_FRWRD_IPV4_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_FRWRD_IPV4_DEF_ACTION_SET,
  ARAD_PP_FRWRD_IPV4_L3_VPN_DEFAULT_ROUTING_ENABLE_SET,
  ARAD_PP_FRWRD_IPV4_L3_VPN_DEFAULT_ROUTING_ENABLE_GET,
  ARAD_PP_FRWRD_IPV4_LPM_ROUTE_ADD,
  ARAD_PP_FRWRD_IPV4_LPM_ROUTE_IS_EXIST,
  ARAD_PP_FRWRD_IPV4_LPM_ROUTE_CLEAR,
  ARAD_PP_FRWRD_IPV4_LPM_ROUTE_REMOVE,
  ARAD_PP_FRWRD_IPV4_LPM_ROUTE_GET,
  ARAD_PP_FRWRD_IPV4_LPM_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_IPV4_LEM_ROUTE_ADD,
  ARAD_PP_FRWRD_IPV4_LEM_ROUTE_GET,
  ARAD_PP_FRWRD_IPV4_LEM_ROUTE_REMOVE,
  ARAD_PP_FRWRD_IPV4_UC_OR_VPN_ROUTE_ADD,
  ARAD_PP_FRWRD_IPV4_UC_OR_VPN_ROUTE_GET,
  ARAD_PP_FRWRD_IPV4_UC_OR_VPN_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_IPV4_UC_OR_VPN_ROUTE_REMOVE,
  ARAD_PP_FRWRD_IPV4_UC_OR_VPN_ROUTING_TABLE_CLEAR,

/*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_IPV4_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_IPV4_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_IPV4_ROUTE_TYPES_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_IPV4_FIRST,
  ARAD_PP_FRWRD_IPV4_SUCCESS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_ROUTE_STATUS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_LOCATION_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_FOUND_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_ROUTES_STATUS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_ROUTES_LOCATION_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_EXACT_MATCH_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_MEM_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_ROUTE_VAL_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_UC_TABLE_RESOUCES_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_MC_TABLE_RESOUCES_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_IP_CACHE_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_GROUP_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_IPV4_EEP_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_FRWRD_IPV4_MC_ILLEGAL_DEST_TYPE_ERR,
  ARAD_PP_FRWRD_IPV4_MUST_ALL_VRF_ERR,
  ARAD_PP_FRWRD_IPV4_CACHE_NOT_SUPPORTED_ERR,
  ARAD_PP_FRWRD_IPV4_DEFRAGE_NOT_SUPPORTED_ERR,
  ARAD_PP_FRWRD_IPV4_OPERATION_NOT_SUPPORTED_ON_VRF_ERR,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_IPV4_ERR_LAST
} ARAD_PP_FRWRD_IPV4_ERR;

/* We may move it in the future to arad_pp_frwrd_ipv4_lpm_mngr.h */
typedef struct
{
  uint32
    default_fec;
  uint32
    nof_lpm_entries_in_lpm;
  uint32
    nof_vrfs;
  SHR_BITDCL
    vrf_modified_bitmask[ARAD_PP_IPV4_VRF_BITMAP_SIZE];
  uint8
    cache_modified;
  uint32
    cache_mode;
#ifdef SOC_SAND_DEBUG
  /* in debug mode, if TRUE lem_route_add will fail. used for denug purposes */
  uint8
    lem_add_fail;
#endif

} ARAD_PP_IPV4_INFO;

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

uint32
  arad_pp_frwrd_ipv4_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ip_routes_cache_mode_enable_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set for which IP routes (IPv4/6 UC/MC) to enable caching
 *   by SW
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 0 - 255. Use VRF 0 for default
 *     routing table. Use SOC_PPC_FRWRD_IP_ALL_VRFS_ID to apply
 *     setting to all VRFs.
 *   SOC_SAND_IN  uint32                                  route_types -
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
  arad_pp_frwrd_ip_routes_cache_mode_enable_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  uint32                                  route_types
  );

uint32
  arad_pp_frwrd_ip_routes_cache_mode_enable_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  uint32                                  route_types
  );

uint32
  arad_pp_frwrd_ip_routes_cache_mode_enable_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_ip_routes_cache_mode_enable_set_unsafe"
 *     API.
 *     Refer to
 *     "arad_pp_frwrd_ip_routes_cache_mode_enable_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_ip_routes_cache_mode_enable_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_OUT uint32                                  *route_types
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ip_routes_cache_commit_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Synchronize the routing table in the Data plane (HW)
 *   with the routing table in the control plane (SW)
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                  route_types -
 *     IP Routes to commit changes for them. use
 *     SOC_SAND_PP_IP_TYPE to select which IP types to commit into
 *     Hardware
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 0 - 255. Use VRF 0 for default
 *     routing table. Use SOC_PPC_FRWRD_IP_ALL_VRFS_ID to apply
 *     setting to all VRFs.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
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
  arad_pp_frwrd_ip_routes_cache_commit_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  route_types,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ip_routes_cache_commit_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  route_types,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_glbl_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the IP routing (including
 *   resources to use)
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info
  );

uint32
  arad_pp_frwrd_ipv4_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info
  );

uint32
  arad_pp_frwrd_ipv4_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_ipv4_glbl_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_ipv4_glbl_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_GLBL_INFO                *glbl_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_uc_route_add_unsafe
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
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key -
 *     The routing key: IPv4 subnet
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
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
  arad_pp_frwrd_ipv4_uc_route_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ipv4_uc_route_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_uc_route_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                                 exact_match -
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
 *   SOC_SAND_OUT uint8                                 *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - the lookup is performed in the SW shadow. - Found is
 *   always TRUE if exact_match is FALSE, due to the default
 *   route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_uc_route_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  uint8                                 exact_match,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_id,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *location,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_frwrd_ipv4_uc_route_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  uint8                                 exact_match
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_uc_route_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv4 UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of entries in returned arrays.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_uc_route_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_keys,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_ipv4_uc_route_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_uc_route_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
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
  arad_pp_frwrd_ipv4_uc_route_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ipv4_uc_route_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY             *route_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_uc_routing_table_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the IPv4 UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_uc_routing_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_frwrd_ipv4_uc_routing_table_clear_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_host_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add IPv4 entry to the Host table. Binds between Host and
 *   next hop information.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key -
 *     VRF-ID and IP address
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info -
 *     Routing information. See remarks.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success -
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
  arad_pp_frwrd_ipv4_host_add_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
  );

uint32
  arad_pp_frwrd_ipv4_host_add_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_host_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information associated with the given
 *   route key on VRF.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key -
 *     The routing key (IPv4 address and a virtual router id)
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
 *   SOC_SAND_OUT uint8                             *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - If the host is not found in the host DB, then the host
 *   (exact route/32) is lookuped in the LPM
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_host_get_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routing_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                             *found
  );

uint32
  arad_pp_frwrd_ipv4_host_get_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_host_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the host table.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key -
 *     Defines the range of routing entries to retrieve.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routes_info -
 *     Array of routing information for each host key.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                              *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - The lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_host_get_block_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO      *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT uint32                              *nof_entries
  );

uint32
  arad_pp_frwrd_ipv4_host_get_block_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_host_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv4 route entry from the routing table of a
 *   virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
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
  arad_pp_frwrd_ipv4_host_remove_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key
  );

uint32
  arad_pp_frwrd_ipv4_host_remove_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY             *host_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mc_route_add_unsafe
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
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key -
 *     The routing key IPv4 subnet
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info -
 *     Routing information (FEC-id, or MC-group).
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success -
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
  arad_pp_frwrd_ipv4_mc_route_add_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
  );

uint32
  arad_pp_frwrd_ipv4_mc_route_add_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mc_route_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                             exact_match -
 *     If TRUE returns exact match only; if FALSE returns
 *     longest prefix match.
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
 *   SOC_SAND_OUT uint8                             *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - Found is always TRUE if exact_match is FALSE, due to
 *   the default route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_mc_route_get_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY         *route_key,
    SOC_SAND_IN  uint8                             exact_match,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION         *location,
    SOC_SAND_OUT uint8                             *found
  );

uint32
  arad_pp_frwrd_ipv4_mc_route_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_key,
    SOC_SAND_IN  uint8                                 exact_match
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mc_route_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the Ipv4 MC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_keys -
 *     Array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids -
 *     Array if fec-ids
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location -
 *     For each route Indicates whether it exsits in host table
 *     or LPM/TCAM. If the entry is not commited yet then this
 *     is the location the route will be inserted into. Set
 *     this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - the lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_mc_route_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_ipv4_mc_route_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mc_route_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   PE uint32 soc_ppd_frwrd_ipv4_mc_route_remove( SOC_SAND_IN
 *   uint32 unit, SOC_SAND_IN SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY
 *   *route_key, SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE *success);
 *   Remove IPv4 route entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_mc_route_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ipv4_mc_route_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mc_routing_table_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the IPv4 MC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_mc_routing_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_frwrd_ipv4_mc_routing_table_clear_verify(
    SOC_SAND_IN  int                                 unit
  );


uint32
  arad_pp_frwrd_ipmc_rp_add_unsafe(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO      *rp_info
  );

uint32
  arad_pp_frwrd_ipmc_rp_get_block_unsafe(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  rp_id,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE            *block_range_key,
    SOC_SAND_OUT int32                                  *l3_intfs,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_ipmc_rp_remove_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO      *rp_info
  );

uint32
  arad_pp_frwrd_ipmc_rp_add_verify(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO      *rp_info
  );

uint32
  arad_pp_frwrd_ipmc_rp_get_block_verify(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  uint32                                  rp_id,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE            *block_range_key,
    SOC_SAND_OUT int32                                  *l3_intfs,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_ipmc_rp_remove_verify(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IP_MC_RP_INFO      *rp_info
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the VRF including
 *   (defaults forwarding).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
  arad_pp_frwrd_ipv4_vrf_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VRF_INFO                 *vrf_info
  );

uint32
  arad_pp_frwrd_ipv4_vrf_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VRF_INFO                 *vrf_info
  );

uint32
  arad_pp_frwrd_ipv4_vrf_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_ipv4_vrf_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_ipv4_vrf_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_vrf_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VRF_INFO                 *vrf_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_route_add_unsafe
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
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key -
 *     The routing key IPv4 subnet MC/UC.
 *   SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
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
  arad_pp_frwrd_ipv4_vrf_route_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id,
    SOC_SAND_IN  uint32                                      flags,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ipv4_vrf_route_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_IN  SOC_PPC_FEC_ID                              fec_id,
    SOC_SAND_IN  uint32                                      flags
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_route_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key on VRF.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_IN  uint8                                 exact_match -
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
 *   SOC_SAND_OUT uint8                                 *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - Found is always TRUE if exact_match is FALSE, due to
 *   the default route.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_vrf_route_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_IN  uint8                                 exact_match,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_id,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *route_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *location,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_frwrd_ipv4_vrf_route_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_IN  uint8                                 exact_match
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_route_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing table of a virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
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
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of entries in returned Arrays.
 * REMARKS:
 *   - the lookup is performed in the SW shadow.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_vrf_route_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_keys,
    SOC_SAND_OUT SOC_PPC_FEC_ID                              *fec_ids,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_ipv4_vrf_route_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_route_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove IPv4 route entry from the routing table of a
 *   virtual router (VRF).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key -
 *     The routing key (IPv4 subnet and a virtual router id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeded.
 * REMARKS:
 *   - use soc_ppd_frwrd_ip_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_vrf_route_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_ipv4_vrf_route_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY            *route_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_routing_table_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear IPv4 routing table of VRF
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx -
 *     Virtual router id. Range: 1 - 255.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_vrf_routing_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx
  );

uint32
  arad_pp_frwrd_ipv4_vrf_routing_table_clear_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_VRF_ID                              vrf_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_vrf_all_routing_tables_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear IPv4 routing tables for all VRFs.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_vrf_all_routing_tables_clear_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 flags
  );

uint32
  arad_pp_frwrd_ipv4_vrf_all_routing_tables_clear_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 flags
  );

uint32
    arad_pp_frwrd_em_dest_to_fec(
      SOC_SAND_IN int                 unit,
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_PAYLOAD  *payload,
      SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO *route_info
    );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mem_status_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the status and usage of memory.
 * INPUT:
 *   SOC_SAND_IN  int                        unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                         mem_ndx -
 *     memory/table id. Range: 0 - 4.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MEM_STATUS      *mem_status -
 *     Status and usage of memory
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_mem_status_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                         mem_ndx,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MEM_STATUS      *mem_status
  );

uint32
  arad_pp_frwrd_ipv4_mem_status_get_verify(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                         mem_ndx
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_mem_defrage_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns the status and usage of memory.
 * INPUT:
 *   SOC_SAND_IN  int                        unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                         mem_ndx -
 *     memory/table id. Range: 0 - 4.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *defrag_info -
 *     Defragment parameters, determine how many iterations and
 *     table access to perform before this call returns.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_ipv4_mem_defrage_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                         mem_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *defrag_info
  );

uint32
  arad_pp_frwrd_ipv4_mem_defrage_verify(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                         mem_ndx,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *defrag_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_ipv4 module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_ipv4_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_ipv4_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_ipv4 module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_ipv4_get_errs_ptr(void);

uint32
    arad_pp_frwrd_ipv4_host_lem_payload_build(
      SOC_SAND_IN int                               unit,
      SOC_SAND_IN SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO   *routing_info,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD          *payload
    );

uint32
    arad_pp_frwrd_ipv4_em_dest_to_sand_dest(
      SOC_SAND_IN int               unit,
      SOC_SAND_IN uint32                dest,
      SOC_SAND_OUT SOC_SAND_PP_DESTINATION_ID *dest_id
    );
uint32
    arad_pp_frwrd_ipv4_sand_dest_to_em_dest(
      SOC_SAND_IN int              unit,
      SOC_SAND_IN SOC_SAND_PP_DESTINATION_ID *dest_id,
      SOC_SAND_OUT uint32 *dest
    );
uint32
    arad_pp_frwrd_ipv4_sand_dest_to_fwd_decision(
      SOC_SAND_IN int              unit,
      SOC_SAND_IN SOC_SAND_PP_DESTINATION_ID *dest_id,
      SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO *em_dest
    );

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
    arad_pp_frwrd_ipv4_uc_rpf_kaps_dbal_route_get(
      SOC_SAND_IN  int                               unit,
      SOC_SAND_IN  uint32                               vrf_ndx,
      SOC_SAND_IN  SOC_SAND_PP_IPV4_SUBNET              *route_key,
      SOC_SAND_IN  uint8                                route_scale,
      SOC_SAND_OUT uint32                               *fec_id,
      SOC_SAND_OUT uint8                                *found
      );
#endif




soc_error_t
  arad_pp_frwrd_ip_ipmc_ssm_add(
     SOC_SAND_IN  int unit,
     SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY* route_key,
     SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO* route_info,
     SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE* success
   );

soc_error_t
  arad_pp_frwrd_ip_ipmc_ssm_delete(
     SOC_SAND_IN int unit,
     SOC_SAND_IN SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY* route_key
   );

soc_error_t
  arad_pp_frwrd_ip_ipmc_ssm_get(
     SOC_SAND_IN  int unit,
     SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY* route_key,
     SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO* route_info,
     SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS*    route_status,
     SOC_SAND_OUT uint8* found
   );




uint32
  arad_pp_frwrd_ip_ipmc_ssm_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY             *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO        *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT SOC_PPC_FRWRD_IP_ROUTE_LOCATION             *routes_location,
    SOC_SAND_OUT uint32                                  *nof_entries
  );


uint32
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_VAL *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_DEFAULT_ACTION *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_ROUTER_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_ROUTER_INFO *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_VRF_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VRF_INFO *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_GLBL_INFO *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_UC_ROUTE_KEY *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_KEY *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_VPN_ROUTE_KEY *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_HOST_KEY_verify(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_KEY *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO_verify(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_HOST_ROUTE_INFO *info
  );

uint32
  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MC_ROUTE_INFO *info
  );


uint32
  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_IPV4_MEM_DEFRAG_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_IPV4_INCLUDED__*/
#endif



