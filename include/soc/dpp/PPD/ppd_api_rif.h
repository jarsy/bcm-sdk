/* $Id: ppd_api_rif.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_rif.h
*
* MODULE PREFIX:  soc_ppd_rif
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

#ifndef __SOC_PPD_API_RIF_INCLUDED__
/* { */
#define __SOC_PPD_API_RIF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_rif.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_mpls_term.h>

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
  SOC_PPD_RIF_MPLS_LABELS_RANGE_SET = SOC_PPD_PROC_DESC_BASE_RIF_FIRST,
  SOC_PPD_RIF_MPLS_LABELS_RANGE_GET,
  SOC_PPD_RIF_MPLS_LABEL_MAP_ADD,
  SOC_PPD_RIF_MPLS_LABEL_MAP_REMOVE,
  SOC_PPD_RIF_MPLS_LABEL_MAP_GET,
  SOC_PPD_RIF_IP_TUNNEL_TERM_ADD,
  SOC_PPD_RIF_IP_TUNNEL_TERM_REMOVE,
  SOC_PPD_RIF_IP_TUNNEL_TERM_GET,
  SOC_PPD_RIF_VSID_MAP_SET,
  SOC_PPD_RIF_VSID_MAP_SET_PRINT,
  SOC_PPD_RIF_VSID_MAP_GET,
  SOC_PPD_RIF_VSID_MAP_GET_PRINT,
  SOC_PPD_RIF_INFO_SET,
  SOC_PPD_RIF_INFO_SET_PRINT,
  SOC_PPD_RIF_INFO_GET,
  SOC_PPD_RIF_INFO_GET_PRINT,
  SOC_PPD_RIF_TTL_SCOPE_SET,
  SOC_PPD_RIF_TTL_SCOPE_SET_PRINT,
  SOC_PPD_RIF_TTL_SCOPE_GET,
  SOC_PPD_RIF_TTL_SCOPE_GET_PRINT,
  SOC_PPD_RIF_GET_PROCS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_RIF_PROCEDURE_DESC_LAST
} SOC_PPD_RIF_PROCEDURE_DESC;

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
 *   soc_ppd_rif_mpls_labels_range_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the MPLS labels that may be mapped to Router
 *   Interfaces
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range -
 *     First and Last MPLS Labels to be mapped to Router
 *     Interfaces. Range Size: 0-896K
 * REMARKS:
 *   - Soc_petra-B: Ignored- T20E: Set the MPLS labels that may
 *   be mapped to Router Interfaces.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_mpls_labels_range_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_rif_mpls_labels_range_set" API.
 *     Refer to "soc_ppd_rif_mpls_labels_range_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_rif_mpls_labels_range_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT SOC_PPC_RIF_MPLS_LABELS_RANGE               *rif_labels_range
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_mpls_label_map_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable MPLS labels termination and setting the Router
 *   interface according to the terminated MPLS label.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key -
 *     The MPLS label, and optionally the VSID, to be
 *     terminated and mapped to RIF
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     LIF table index
 *   SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO                      *term_info -
 *     MPLS Termination info including type of termination
 *     (pipe/uniform), RIF
 *   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
 *     RIF attributes. RIF ID, ...
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     Exact Match table
 * REMARKS:
 *   - Soc_petra-B: label can be terminated upon lookup or range.
 *     range termination has priority over lookup.
 *     if label is in on of the ranges, but the label is not
 *     set to be terminated then label will not be terminated at all
 *   T20E: The lif_index is ignored. Terminating the label via
 *   'soc_ppd_rif_mpls_label_map_add()' enables:- Mapping to a
 *   dedicated RIF, while range termination map to a global
 *   RIF- Termination of labels outside the range-
 *   Termination according to both MPLS label and the VSID,
 *   and not only according to the MPLS label. The VSID was
 *   assigned to the packet, according to the Link Layer
 *   Ethernet header's AC.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_mpls_label_map_add(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_MPLS_TERM_INFO                      *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                    *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_mpls_label_map_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove MPLS label that was mapped to a RIF-Tunnel
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key -
 *     The MPLS label, and optionally the VSID, to be
 *     terminated and mapped to RIF
 *   SOC_SAND_INOUT SOC_PPC_LIF_ID                              *lif_index -
 *     LIF table index. Input when LIF key is LOOKUP_NONE
 * REMARKS:
 *   - Unbind the mapping of the MPLS label to the LIF table
 *   from the SEM table- Invalidate the 'lif_index' entry in
 *   the LIF table- The 'lif_index' is returned to the user
 *   to enable management of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_mpls_label_map_remove(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_INOUT SOC_PPC_LIF_ID                              *lif_index
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_mpls_label_map_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get MPLS label termination and Router interface info
 *   according to the terminated MPLS label.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key -
 *     The MPLS label, and optionally the VSID, to be
 *     terminated and mapped to RIF
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     LIF table index
 *   SOC_SAND_OUT SOC_PPC_MPLS_TERM_INFO                      *term_info -
 *     MPLS Termination info including type of termination
 *     (pipe/uniform), RIF-id>. The term_info.cos-profile is
 *     ignored. Instead the rif_info.cos-profile is used.
 *   SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info -
 *     RIF attributes. RIF ID, ...
 *   SOC_SAND_OUT uint8                               *found -
 *     Was key found
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_mpls_label_map_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_MPLS_LABEL_RIF_KEY                  *mpls_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_MPLS_TERM_INFO                      *term_info,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT uint8                               *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_ip_tunnel_term_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable IP Tunnels termination and setting the Router
 *   interface according to the terminated IP tunnel.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                 flags -
 *     Flags. Use to identify if lif_index is really a lif (for boundaries check)
 *            or it's a learned FEC, or my_vtep_index
 *   SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY               *term_key -
 *     Destination IP
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index -
 *     Entry in the LIF table
 *   SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO                    *term_info -
 *     IP tunnel Termination info including type of termination
 *     RIF-id, cos-profile.
 *   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
 *     RIF attributes. RIF ID, ...
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     Exact Match table
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_ip_tunnel_term_add(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                  flags,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY               *term_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                              lif_index,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_INFO                    *term_info,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_ip_tunnel_term_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove the IP Tunnel
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY               *term_key -
 *     Destination IP
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Entry in the LIF table
 * REMARKS:
 *   - Unbind the mapping of the IP Tunnel to the LIF table
 *   from the SEM table- Invalidate the 'lif_index' entry in
 *   the LIF table- The 'lif_index' is returned to the user
 *   to enable management of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_ip_tunnel_term_remove(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY               *term_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_ip_tunnel_term_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get IP Tunnels termination and Router interface info
 *   according to the terminated IP tunnel.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY               *term_key -
 *     Destination IP
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index -
 *     Entry in the LIF table
 *   SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_INFO                    *term_info -
 *     IP tunnel Termination info including type of termination
 *     RIF-id. The term_info.cos-profile is ignored. Instead
 *     the rif_info.cos-profile is used.
 *   SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info -
 *     RIF attributes. RIF ID, ...
 *   SOC_SAND_OUT uint8                                 *found -
 *     Was key found
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_ip_tunnel_term_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_RIF_IP_TERM_KEY               *term_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                              *lif_index,
    SOC_SAND_OUT SOC_PPC_RIF_IP_TERM_INFO                    *term_info,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info,
    SOC_SAND_OUT uint8                                 *found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_vsid_map_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Router Interface according to the VSID.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx -
 *     VSID. Equal to the RIF-ID
 *   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
 *     RIF attributes. in this case RIF_ID has to be equal to
 *     vsid_ndx
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_vsid_map_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_rif_vsid_map_set"
 *     API.
 *     Refer to "soc_ppd_rif_vsid_map_set" API for details.
*********************************************************************/
uint32
  soc_ppd_rif_vsid_map_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_VSI_ID                              vsid_ndx,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Router Interface according to the VSID.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx -
 *     Router Interface ID. Range: 0 - 4K-1.
 *   SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info -
 *     RIF attributes. CoS-Profile relvant only if the RIF-ID
 *     used as VSI-RIF.
 * REMARKS:
 *   - use this API to set attributes of RIF obtained upon
 *   MPLS tunnel termination by range, or reserved values.-
 *   can be used also to update RIF attributes of RIF set
 *   according to IP/MPLS tunnel termination or VSI.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_info_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx,
    SOC_SAND_IN  SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
*     Gets the configuration set by the "soc_ppd_rif_info_set"
 *     API.
 *     Refer to "soc_ppd_rif_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_rif_info_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_RIF_ID                              rif_ndx,
    SOC_SAND_OUT SOC_PPC_RIF_INFO                            *rif_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_rif_ttl_scope_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TTL value for TTL-scope.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                               ttl_scope_ndx -
 *     TTL scope index. Range: 0-7. set according to RIF. See
 *     SOC_PPC_RIF_INFO.
 *   SOC_SAND_IN  SOC_SAND_PP_IP_TTL                          ttl_val -
 *     TTL value. Range: 0 -255.
 * REMARKS:
 *   - When packet is routed (IP/MPLS routing) to this RIF
 *   then packet's TTL is compared againsy the TTL if it less
 *   or equal then packet is filter. Range: 0 - 7. use
 *   soc_ppd_trap_eg_profile_info_set(SOC_PPC_TRAP_EG_TYPE_TTL_SCOPE,
 *   eg_trap_info) to set how to handle packets match
 *   condition of this filter.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_rif_ttl_scope_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               ttl_scope_ndx,
    SOC_SAND_IN  SOC_SAND_PP_IP_TTL                          ttl_val
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_rif_ttl_scope_set" API.
 *     Refer to "soc_ppd_rif_ttl_scope_set" API for details.
*********************************************************************/
uint32
  soc_ppd_rif_ttl_scope_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               ttl_scope_ndx,
    SOC_SAND_OUT SOC_SAND_PP_IP_TTL                          *ttl_val
  );


/*********************************************************************
 * NAME:
 *   soc_ppd_rif_native_routing_vlan_tags_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *        set native vsi compensation per rif.
 *        native vsi compensation indicate how many vlan tags to expect at the native ethernet header.
 *        For packet with UDP header, help to calculate udp length and udp checksum.
 *        Since udp header is built at the egress tunnel block, before EVE (applied at the egress LL block). 
 *        
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_RIF_ID                               intf_id -
 *     RIF id. 
 *   SOC_SAND_IN  uint8                          native_routing_vlan_tags -
 *     Number of tags in native ethernet. Range: 0 - 2.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
soc_ppd_rif_native_routing_vlan_tags_set(
   SOC_SAND_IN  int                                    unit, 
   SOC_SAND_IN  SOC_PPC_RIF_ID                                 intf_id, 
   SOC_SAND_IN  uint8                                  native_routing_vlan_tags
   );

uint32
soc_ppd_rif_native_routing_vlan_tags_get(
   SOC_SAND_IN  int                                    unit, 
   SOC_SAND_IN  SOC_PPC_RIF_ID                         intf_id, 
   SOC_SAND_OUT  uint8                                 *native_routing_vlan_tags
   );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_RIF_INCLUDED__*/
#endif

