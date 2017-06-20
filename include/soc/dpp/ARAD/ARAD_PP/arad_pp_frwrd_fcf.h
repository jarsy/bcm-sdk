
/* $Id: arad_pp_frwrd_fcf.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_FCF_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_FCF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>

#include <soc/dpp/PPC/ppc_api_frwrd_fcf.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */



/******** VFT header defines **********/ 

/* how many bits in D_ID/ S_ID*/
#define ARAD_PP_FRWRD_FCF_D_ID_BITS (24)
/* how many bits in D_ID/ S_ID*/
#define ARAD_PP_FRWRD_FCF_S_ID_BITS (24)
/* bits for VFI as take one bit from Reserved */
#define ARAD_PP_FRWRD_FCF_VFI_BITS (13)
/* bits for route/domain */
#define ARAD_PP_FRWRD_FCF_D_ID_ROUTE_BITS (8)


/* bits for route/domain */
#define ARAD_PP_FRWRD_FCF_D_ID_ROUTE_LSB (ARAD_PP_FRWRD_FCF_D_ID_BITS-ARAD_PP_FRWRD_FCF_D_ID_ROUTE_BITS)
/* mask bits n bits [n-1:0]*/
#define ARADP_PP_FRWRD_FCF_MASK_BITS(__n)   ((1 << (__n)) - 1)
#define ARAD_PP_FRWRD_FCF_GET_DOMAIN(__d_id)  ((__d_id >> (ARAD_PP_FRWRD_FCF_D_ID_ROUTE_LSB)) & ARADP_PP_FRWRD_FCF_MASK_BITS(ARAD_PP_FRWRD_FCF_D_ID_ROUTE_BITS))


#define ARAD_PP_FRWRD_FCF_SET_DOMAIN(__d_id, __domain)  ((__d_id) |= ((__domain & ARADP_PP_FRWRD_FCF_MASK_BITS(ARAD_PP_FRWRD_FCF_D_ID_ROUTE_BITS)) << ARAD_PP_FRWRD_FCF_D_ID_ROUTE_LSB))


/******** KEY building defines **********/ 

    /* ip-address encoding: <VFI, D_ID> */
    /*******************************************************/
    /*   31  - 20    * 19   *      18    -   11 *   10 - 0  */
    /*******************************************************/
    /*     VFI       *  R   *    D_ID[23:16]    *  masked   */
    /*******************************************************/

/* D_ID_LSB: 12 = 32 - 12 - 8*/

#define ARAD_PP_FRWRD_FCF_KEY_NOF_R_BITS (1)
#define ARAD_PP_FRWRD_FCF_KEY_D_ID_LSB     (32 - ARAD_PP_FRWRD_FCF_VFI_BITS - ARAD_PP_FRWRD_FCF_D_ID_ROUTE_BITS)
#define ARAD_PP_FRWRD_FCF_KEY_VFI_LSB     (32 - ARAD_PP_FRWRD_FCF_VFI_BITS + ARAD_PP_FRWRD_FCF_KEY_NOF_R_BITS) 

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
  SOC_PPC_FRWRD_FCF_GLBL_INFO_SET,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_SET_PRINT,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_GET,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_GET_PRINT,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_FCF_GLBL_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_FCF_ROUTE_ADD,
  ARAD_PP_FRWRD_FCF_ROUTE_ADD_PRINT,
  ARAD_PP_FRWRD_FCF_ROUTE_ADD_UNSAFE,
  ARAD_PP_FRWRD_FCF_ROUTE_ADD_VERIFY,
  ARAD_PP_FRWRD_FCF_ROUTE_GET,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_PRINT,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_UNSAFE,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_VERIFY,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_FCF_ROUTE_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_FCF_ROUTE_REMOVE,
  ARAD_PP_FRWRD_FCF_ROUTE_REMOVE_PRINT,
  ARAD_PP_FRWRD_FCF_ROUTE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_FCF_ROUTE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_FCF_ROUTING_TABLE_CLEAR,
  ARAD_PP_FRWRD_FCF_ROUTING_TABLE_CLEAR_PRINT,
  ARAD_PP_FRWRD_FCF_ROUTING_TABLE_CLEAR_UNSAFE,
  ARAD_PP_FRWRD_FCF_ROUTING_TABLE_CLEAR_VERIFY,
  ARAD_PP_FRWRD_FCF_ZONING_ADD,
  ARAD_PP_FRWRD_FCF_ZONING_ADD_PRINT,
  ARAD_PP_FRWRD_FCF_ZONING_ADD_UNSAFE,
  ARAD_PP_FRWRD_FCF_ZONING_ADD_VERIFY,
  ARAD_PP_FRWRD_FCF_ZONING_GET,
  ARAD_PP_FRWRD_FCF_ZONING_GET_PRINT,
  ARAD_PP_FRWRD_FCF_ZONING_GET_UNSAFE,
  ARAD_PP_FRWRD_FCF_ZONING_GET_VERIFY,
  ARAD_PP_FRWRD_FCF_ZONING_GET_BLOCK,
  ARAD_PP_FRWRD_FCF_ZONING_GET_BLOCK_PRINT,
  ARAD_PP_FRWRD_FCF_ZONING_GET_BLOCK_UNSAFE,
  ARAD_PP_FRWRD_FCF_ZONING_GET_BLOCK_VERIFY,
  ARAD_PP_FRWRD_FCF_ZONING_REMOVE,
  ARAD_PP_FRWRD_FCF_ZONING_REMOVE_PRINT,
  ARAD_PP_FRWRD_FCF_ZONING_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_FCF_ZONING_REMOVE_VERIFY,

  ARAD_PP_FRWRD_FCF_ZONING_TABLE_CLEAR,
  ARAD_PP_FRWRD_FCF_ZONING_TABLE_CLEAR_PRINT,
  ARAD_PP_FRWRD_FCF_GET_PROCS_PTR,
  ARAD_PP_FRWRD_FCF_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_FRWRD_FCF_DEF_ACTION_SET,
  ARAD_PP_FRWRD_FCF_L3_VPN_DEFAULT_ROUTING_ENABLE_SET,
  ARAD_PP_FRWRD_FCF_L3_VPN_DEFAULT_ROUTING_ENABLE_GET,
  ARAD_PP_FRWRD_FCF_LPM_ROUTE_ADD,
  ARAD_PP_FRWRD_FCF_LPM_ROUTE_IS_EXIST,
  ARAD_PP_FRWRD_FCF_LPM_ROUTE_CLEAR,
  ARAD_PP_FRWRD_FCF_LPM_ROUTE_REMOVE,
  ARAD_PP_FRWRD_FCF_LPM_ROUTE_GET,
  ARAD_PP_FRWRD_FCF_LPM_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_FCF_LEM_ROUTE_ADD,
  ARAD_PP_FRWRD_FCF_LEM_ROUTE_GET,
  ARAD_PP_FRWRD_FCF_LEM_ROUTE_REMOVE,
  ARAD_PP_FRWRD_FCF_OR_VPN_ROUTE_ADD,
  ARAD_PP_FRWRD_FCF_OR_VPN_ROUTE_GET,
  ARAD_PP_FRWRD_FCF_OR_VPN_ROUTE_GET_BLOCK,
  ARAD_PP_FRWRD_FCF_OR_VPN_ROUTE_REMOVE,
  ARAD_PP_FRWRD_FCF_OR_VPN_ROUTING_TABLE_CLEAR,

/*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_FCF_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_FCF_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_FCF_ROUTE_TYPES_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_FCF_FIRST,
  ARAD_PP_FRWRD_FCF_SUCCESS_OUT_OF_RANGE_ERR,
  SOC_PPC_FRWRD_FCF_ROUTE_STATUS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_FOUND_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_ROUTES_STATUS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_NOF_ENTRIES_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_EXACT_MATCH_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_MEM_NDX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_ROUTE_VAL_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_TABLE_RESOUCES_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_MC_TABLE_RESOUCES_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_CACHE_MODE_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_GROUP_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_EEP_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_FRWRD_FCF_MC_ILLEGAL_DEST_TYPE_ERR,
  ARAD_PP_FRWRD_FCF_MUST_ALL_VFI_ERR,
  ARAD_PP_FRWRD_FCF_CACHE_NOT_SUPPORTED_ERR,
  ARAD_PP_FRWRD_FCF_DEFRAGE_NOT_SUPPORTED_ERR,
  ARAD_PP_FRWRD_FCF_OPERATION_NOT_SUPPORTED_ON_VFI_ERR,
  ARAD_PP_FRWRD_FCF_IP_VRF_NOT_MATCH_RESERVED_ERR,
  ARAD_PP_FRWRD_FCF_FRWRD_D_ID_PREFIX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_FRWRD_INTERNAL_IMP_ERR,
  ARAD_PP_FRWRD_FCF_FRWRD_ROUTE_PREFIX_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_FCF_FRWRD_ROUTE_LPM_DEST_ILLEGAL_ERR,


  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_FCF_ERR_LAST
} ARAD_PP_FRWRD_FCF_ERR;

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
  arad_pp_frwrd_fcf_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_glbl_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the IP routing (including
 *   resources to use)
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  );

uint32
  arad_pp_frwrd_fcf_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  );

uint32
  arad_pp_frwrd_fcf_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_fcf_glbl_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_fcf_glbl_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_route_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add fcf route entry to the routing table. Binds between
 *   fcf Unicast route key (fcf-address/prefix) and a FEC
 *   entry identified by route_info for a given router. As a
 *   result of this operation, Unicast fcf packets
 *   designated to the IP address matching the given key (as
 *   long there is no more-specific route key) will be routed
 *   according to the information in the FEC entry identified
 *   by route_info.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key -
 *     The routing key: fcf subnet
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO            *route_info -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the routing DB (LEM/LPM).
 * REMARKS:
 *   - use arad_pp_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_route_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO            *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_fcf_route_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO            *route_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_route_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key -
 *     The routing key (fcf subnet and a virtual fabric identifier id)
 *   SOC_SAND_IN  uint8                                 exact_match -
 *     If TRUE returns exact match only; if FALSE returns
 *     longest prefix match.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO                  *route_info -
 *     Routing information (system-fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
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
  arad_pp_frwrd_fcf_route_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_IN  uint8                                 exact_match,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO                  *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *route_status,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_frwrd_fcf_route_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_IN  uint8                                 exact_match
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_route_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the fcf UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_keys -
 *     Array of routing keys (fcf subnets)
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO                  *routes_info -
 *     Array of fec-ids.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                                  *nof_entries -
 *     Number of entries in returned arrays.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_route_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO                  *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *routes_status,
    SOC_SAND_OUT uint32                                  *nof_entries
  );

uint32
  arad_pp_frwrd_fcf_route_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_route_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key -
 *     The routing key (fcf subnet and a virtual fabric identifier id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds. Remove operation may
 *     fail if there is no place in the fcf DB (LPM), due to
 *     fail of decompression.
 * REMARKS:
 *   - use arad_pp_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.- remove un-exist entry considered
 *   as succeeded operation
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_route_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_frwrd_fcf_route_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_routing_table_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the fcf UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_routing_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_VFI                vfi_ndx,
    SOC_SAND_IN  uint32                               flags

  );

uint32
  arad_pp_frwrd_fcf_routing_table_clear_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_VFI                vfi_ndx,
    SOC_SAND_IN  uint32                               flags
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_zoning_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add fcf entry to the zoning table. Binds between Host and
 *   next hop information.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key -
 *     VFI-ID and IP address
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info -
 *     Routing information. See remarks.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the LEM DB.
 * REMARKS:
 *   - use arad_pp_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW. - Supports virtual routing tables
 *   using the vfi_id - user has to supply in routing info
 *   FEC-ptr which set the out-RIF, TM-destination and EEP.
 *   Optionally, user can supply EEP directly from the host
 *   table.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_zoning_add_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                  *success
  );

uint32
  arad_pp_frwrd_fcf_zoning_add_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_zoning_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information associated with the given
 *   route key on VFI.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key -
 *     The routing key (fcf address and a virtual fabric identifier id)
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info -
 *     Routing information (fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
 *   SOC_SAND_OUT uint8                             *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - If the host is not found in the host DB, then the host
 *   (exact route/32) is lookuped in the LPM
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_zoning_get_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT uint8                             *found
  );

uint32
  arad_pp_frwrd_fcf_zoning_get_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_zoning_get_block_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the zoning table.
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key -
 *     Defines the range of routing entries to retrieve.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routes_info -
 *     Array of routing information for each host key.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *routes_status -
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
  arad_pp_frwrd_fcf_zoning_get_block_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT uint32                              *nof_entries
  );

uint32
  arad_pp_frwrd_fcf_zoning_get_block_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                *block_range_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_zoning_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove fcf route entry from the routing table of a
 *   virtual fabric identifier (VFI).
 * INPUT:
 *   SOC_SAND_IN  int                             unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key -
 *     The routing key (fcf subnet and a virtual fabric identifier id)
 * REMARKS:
 *   - use arad_pp_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_zoning_remove_unsafe(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key
  );

uint32
  arad_pp_frwrd_fcf_zoning_remove_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_zoning_table_clear_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear fcf routing table of VFI
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_VFI                              vfi_ndx -
 *     Virtual Fabric Identifier id. Range: 1 - 255.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_fcf_zoning_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                               flags,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY         *key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO        *info
  );

uint32
  arad_pp_frwrd_fcf_zoning_table_clear_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                               flags,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY         *key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO        *info
  );

void
    arad_pp_fcf_route_key_from_lem_key(
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY *key,
      SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY *route_key
    );
	
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_npv_switch_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_fcf module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_fcf_npv_switch_set(
    int unit,
    int enable
  );

soc_error_t
  arad_pp_frwrd_fcf_vsan_mode_set(
    int                                 unit,
    int                                 is_vft_from_vsi
  );
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_npv_switch_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_fcf module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_fcf_npv_switch_get(
    int unit,
    int* enable
  );

soc_error_t
  arad_pp_frwrd_fcf_vsan_mode_get(
    int                                 unit,
    int*                                is_vsan_from_vsi
  );
  
/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_fcf module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_fcf_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_fcf_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_fcf module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_fcf_get_errs_ptr(void);



uint32
  SOC_PPC_FRWRD_FCF_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO *info
  );

uint32
  SOC_PPC_FRWRD_FCF_ROUTE_KEY_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY *info
  );


uint32
  SOC_PPC_FRWRD_FCF_ROUTE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO *info
  );

uint32
  SOC_PPC_FRWRD_FCF_ZONING_KEY_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY *info
  );

uint32
  SOC_PPC_FRWRD_FCF_ZONING_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO *info
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_FCF_INCLUDED__*/
#endif



