/* $Id: ppd_api_frwrd_fcf.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_fcf.h
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

#ifndef __SOC_PPD_API_FRWRD_FCF_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_FCF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_frwrd_fcf.h>

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
  SOC_PPD_FRWRD_FCF_GLBL_INFO_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_FCF_FIRST,
  SOC_PPD_FRWRD_FCF_GLBL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_FCF_GLBL_INFO_GET,
  SOC_PPD_FRWRD_FCF_GLBL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_FCF_ROUTE_ADD,
  SOC_PPD_FRWRD_FCF_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_FCF_ROUTE_GET,
  SOC_PPD_FRWRD_FCF_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_FCF_ROUTE_GET_BLOCK,
  SOC_PPD_FRWRD_FCF_ROUTE_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_FCF_ROUTE_REMOVE, 
  SOC_PPD_FRWRD_FCF_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_FCF_ROUTING_TABLE_CLEAR,
  SOC_PPD_FRWRD_FCF_ROUTING_TABLE_CLEAR_PRINT,
  SOC_PPD_FRWRD_FCF_ZONING_ADD,
  SOC_PPD_FRWRD_FCF_ZONING_ADD_PRINT,
  SOC_PPD_FRWRD_FCF_ZONING_GET,
  SOC_PPD_FRWRD_FCF_ZONING_GET_PRINT,
  SOC_PPD_FRWRD_FCF_ZONING_GET_BLOCK,
  SOC_PPD_FRWRD_FCF_ZONING_GET_BLOCK_PRINT,
  SOC_PPD_FRWRD_FCF_ZONING_REMOVE,
  SOC_PPD_FRWRD_FCF_ZONING_REMOVE_PRINT,
  SOC_PPD_FRWRD_FCF_ZONING_TABLE_CLEAR,
  SOC_PPD_FRWRD_FCF_ZONING_TABLE_CLEAR_PRINT,
  
  SOC_PPD_FRWRD_FCF_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_FCF_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_FCF_PROCEDURE_DESC;


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
 *   soc_ppd_frwrd_fcf_glbl_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting global information of the IP routing (including
 *   resources to use)
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info -
 *     Global information.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_glbl_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_fcf_glbl_info_set" API.
 *     Refer to "soc_ppd_frwrd_fcf_glbl_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_glbl_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_GLBL_INFO                *glbl_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add FCF route entry to the routing table. Binds between
 *   FCF Unicast route key (FCF-address/prefix) and a FEC
 *   entry identified by route_info for a given router. As a
 *   result of this operation, Unicast FCF packets
 *   designated to the IP address matching the given key (as
 *   long there is no more-specific route key) will be routed
 *   according to the information in the FEC entry identified
 *   by route_info.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key -
 *     The routing key: FCF subnet
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO            *route_info -
 *     FEC ID.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the routing DB (LEM/LPM).
 * REMARKS:
 *   - use soc_ppd_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_route_add(
    SOC_SAND_IN  int                                  unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_INFO            *route_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information (system-fec-id) associated
 *   with the given route key.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key -
 *     The routing key (FCF subnet and a virtual fabric identifier id)
 *   SOC_SAND_IN  uint8                               exact_match -
 *     If TRUE returns exact match only; if FALSE returns
 *     longest prefix match.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO                  *route_info -
 *     Routing information (system-fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
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
  soc_ppd_frwrd_fcf_route_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY         *route_key,
    SOC_SAND_IN  uint8                               exact_match,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO        *route_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS      *route_status,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_route_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the FCF UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE              *block_range -
 *     Defines the range and the order of the block of routing
 *     entries.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_keys -
 *     Array of routing keys (FCF subnets)
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO        *routes_info -
 *     Array of fec-ids.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS               *routes_status -
 *     For each route Indicates whether it exists in HW
 *     (commited) or pending (either for remove or addition).
 *     Set this parameter to NULL in order to be ignored
 *   SOC_SAND_OUT uint32                                *nof_entries -
 *     Number of entries in returned arrays.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_route_get_block(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_PPC_IP_ROUTING_TABLE_RANGE      *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_KEY            *route_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO           *routes_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS         *routes_status,
    SOC_SAND_OUT uint32                                 *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove entry from the routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY             *route_key -
 *     The routing key (FCF subnet and a virtual fabric identifier id)
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     Whether the operation succeeds. Remove operation may
 *     fail if there is no place in the FCF DB (LPM), due to
 *     fail of decompression.
 * REMARKS:
 *   - use soc_ppd_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.- remove un-exist entry considered
 *   as succeeded operation
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_route_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ROUTE_KEY          *route_key,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE             *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_routing_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear the FCF UC routing table.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_routing_table_clear(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_VFI                vfi_ndx,
    SOC_SAND_IN  uint32                               flags
   );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_zoning_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add FCF entry to the zoning table. Binds between Host and
 *   next hop information.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key -
 *     VFI-ID and IP address
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info -
 *     Routing information. See remarks.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success -
 *     Whether the operation succeeds. Add operation may fail
 *     if there is no place in the LEM DB.
 * REMARKS:
 *   - use soc_ppd_frwrd_fcf_routes_cache_mode_enable_set() to
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
  soc_ppd_frwrd_fcf_zoning_add(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_zoning_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the routing information associated with the given
 *   route key on VFI.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key -
 *     The routing key (FCF address and a virtual fabric identifier id)
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO      *routing_info -
 *     Routing information (fec-id).
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *route_status -
 *     Indicates whether the returned entry exist in HW
 *     (commited) or pending (either for remove or addition),
 *     relevant only if found is TRUE.
 *   SOC_SAND_OUT uint8                           *found -
 *     If TRUE then route_val has valid data.
 * REMARKS:
 *   - If the host is not found in the host DB, then the host
 *   (exact route/32) is lookuped in the LPM
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_get(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *routing_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *route_status,
    SOC_SAND_OUT uint8                           *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_zoning_get_block
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the zoning table.
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range_key -
 *     Defines the range of routing entries to retrieve.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_keys -
 *     array of routing keys
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_INFO      *routes_info -
 *     Array of routing information for each host key.
 *   SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *routes_status -
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
  soc_ppd_frwrd_fcf_zoning_get_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE              *block_range_key,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ZONING_INFO      *zoning_info,
    SOC_SAND_OUT SOC_PPC_FRWRD_FCF_ROUTE_STATUS           *routes_status,
    SOC_SAND_OUT uint32                            *nof_entries
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_zoning_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove FCF route entry from the routing table of a
 *   virtual fabric identifier (VFI).
 * INPUT:
 *   SOC_SAND_IN  int                           unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key -
 *     The routing key (FCF subnet and a virtual fabric identifier id)
 * REMARKS:
 *   - use soc_ppd_frwrd_fcf_routes_cache_mode_enable_set() to
 *   determine wehter to cache this route in SW or to write
 *   it directly into HW.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_remove(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY             *zoning_key
  );


/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_zoning_table_clear
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Clear FCF routing table of VFI
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_FCF_VFI                vfi -
 *     Virtual Fabric Identifier id. Range: 1 - 255.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_fcf_zoning_table_clear(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               flags,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_KEY         *key,
    SOC_SAND_IN  SOC_PPC_FRWRD_FCF_ZONING_INFO        *info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_fcf_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   soc_ppd_api_frwrd_fcf module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  soc_ppd_frwrd_fcf_get_procs_ptr(void);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_FCF_INCLUDED__*/
#endif

