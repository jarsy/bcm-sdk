/* $Id: ppd_api_frwrd_trill.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_frwrd_trill.h
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

#ifndef __SOC_PPD_API_FRWRD_TRILL_INCLUDED__
/* { */
#define __SOC_PPD_API_FRWRD_TRILL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPC/ppc_api_frwrd_trill.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_lif.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* TRILL VSI flags */
#define SOC_PPD_TRILL_VSI_TRANSPARENT_SERVICE  0x00000001 

/*     Use to accept SA from all source system ports in
 *     soc_ppd_frwrd_trill_ adj _info_set ()                       */

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
  SOC_PPD_FRWRD_TRILL_MULTICAST_KEY_MASK_SET = SOC_PPD_PROC_DESC_BASE_FRWRD_TRILL_FIRST,
  SOC_PPD_FRWRD_TRILL_MULTICAST_KEY_MASK_SET_PRINT,
  SOC_PPD_FRWRD_TRILL_MULTICAST_KEY_MASK_GET,
  SOC_PPD_FRWRD_TRILL_MULTICAST_KEY_MASK_GET_PRINT,
  SOC_PPD_FRWRD_TRILL_UNICAST_ROUTE_ADD,
  SOC_PPD_FRWRD_TRILL_UNICAST_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_TRILL_UNICAST_ROUTE_GET,
  SOC_PPD_FRWRD_TRILL_UNICAST_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_TRILL_UNICAST_ROUTE_REMOVE,
  SOC_PPD_FRWRD_TRILL_UNICAST_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_TRILL_MULTICAST_ROUTE_ADD,
  SOC_PPD_FRWRD_TRILL_MULTICAST_ROUTE_ADD_PRINT,
  SOC_PPD_FRWRD_TRILL_MULTICAST_ROUTE_GET,
  SOC_PPD_FRWRD_TRILL_MULTICAST_ROUTE_GET_PRINT,
  SOC_PPD_FRWRD_TRILL_MULTICAST_ROUTE_REMOVE,
  SOC_PPD_FRWRD_TRILL_MULTICAST_ROUTE_REMOVE_PRINT,
  SOC_PPD_FRWRD_TRILL_ADJ_INFO_SET,
  SOC_PPD_FRWRD_TRILL_ADJ_INFO_SET_PRINT,
  SOC_PPD_FRWRD_TRILL_ADJ_INFO_GET,
  SOC_PPD_FRWRD_TRILL_ADJ_INFO_GET_PRINT,
  SOC_PPD_FRWRD_TRILL_GLOBAL_INFO_SET,
  SOC_PPD_FRWRD_TRILL_GLOBAL_INFO_SET_PRINT,
  SOC_PPD_FRWRD_TRILL_GLOBAL_INFO_GET,
  SOC_PPD_FRWRD_TRILL_GLOBAL_INFO_GET_PRINT,
  SOC_PPD_FRWRD_TRILL_GET_PROCS_PTR,
  SOC_PPD_FRWRD_TRILL_MULTICAST_SOURCE_ADD,
  SOC_PPD_FRWRD_TRILL_MULTICAST_SOURCE_REMOVE,
  SOC_PPD_FRWRD_TRILL_MULTICAST_SOURCE_GET,
  SOC_PPD_FRWRD_TRILL_NATIVE_INNER_TPID_ADD,
  SOC_PPD_FRWRD_TRILL_VSI_ENTRY_ADD,
  SOC_PPD_FRWRD_TRILL_VSI_ENTRY_REMOVE,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_FRWRD_TRILL_PROCEDURE_DESC_LAST
} SOC_PPD_FRWRD_TRILL_PROCEDURE_DESC;


typedef enum
{
    SOC_PPD_TRILL_MODE_DISABLED = 0,
    SOC_PPD_TRILL_MODE_VL = 1,
    SOC_PPD_TRILL_MODE_FGL = 2
} SOC_PPD_TRILL_MODE;

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
 *   soc_ppd_frwrd_trill_multicast_key_mask_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the key type of TRILL multicast routes lookup. The
 *   following fields are optional: Ing-Nick-key;
 *   Adjacent-EEP-key; FID-key
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields -
 *     Trill multicast route fields to be masked upon lookup
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_multicast_key_mask_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_trill_multicast_key_mask_set" API.
 *     Refer to "soc_ppd_frwrd_trill_multicast_key_mask_set" API
 *     for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_multicast_key_mask_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_unicast_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map nick-name to a FEC entry ID. Used for forwarding
 *   packets with the nick name as destination to the FEC,
 *   and to associate the FEC as learning information, upon
 *   receiving packets with the Nick-Name as the source
 *   address
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                  lif_index -
 *     Index to the Logical interfaces table
 *   SOC_SAND_IN  uint32                    nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO *learn_info
 *     Forward decision for trill transit. dest_id=fec_id
 *     fec_id - FEC Entry ID. The FEC is expected to hold the out-port as
 *              destination and an EEP that points to the link layer
 *              encapsulation towards the adjacent RBridge. The FEC ID
 *              may also point to ECMP with list of adjacent RBridges,
 *              and the connected ports.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE        *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in
 *     SEMSAND_FAILURE_OUT_OF_RESOURCES_2: There is no space in
 *     LEM
 * REMARKS:
 *   Written to both the Logical interfaces table for
 *   learning purposes, and to the LEM table for forwarding
 *   purposes
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_unicast_route_add(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                      nickname_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO *learn_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_unicast_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get mapping of TRILL nickname to FEC ID and LIF index
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                  *lif_index -
 *     Index to the Logical interfaces table
 *   SOC_SAND_OUT SSOC_PPC_FRWRD_DECISION_INFO *learn_info -
 *      Forward decision for trill transit. dest_id=fec_id
 *   SOC_SAND_OUT uint8                   *is_found -
 *     indicates if entry was found
 * REMARKS:
 *   The 'lif_index' is returned to the user to enable the
 *   LIF table management
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_unicast_route_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     nickname_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO *learn_info,
    SOC_SAND_OUT uint8                      *is_found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_unicast_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove TRILL nick-name
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                  lif_index -
 *     Index to the Logical interfaces table
 * REMARKS:
 *   Removed from both the SEM table and to the LEM table. The
 *   'lif_index' is returned to user to enable his management
 *   of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_unicast_route_remove(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                    nickname_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_ingress_lif_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map nick-name to a FEC entry ID. Used for forwarding
 *   packets with the nick name as destination to the FEC,
 *   and to associate the FEC as learning information, upon
 *   receiving packets with the Nick-Name as the source
 *   address
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                  lif_index -
 *     Index to the Logical interfaces table
 *   SOC_SAND_IN  uint32                    nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO  *trill_info
 *     trill_info attributes:
 *     fec_id - FEC Entry ID. The FEC is expected to hold the out-port as
 *              destination and an EEP that points to the link layer
 *              encapsulation towards the adjacent RBridge. The FEC ID
 *              may also point to ECMP with list of adjacent RBridges,
 *              and the connected ports.
 *     learn_enable - indicate if learn enable on this interface.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE        *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in
 *     SEMSAND_FAILURE_OUT_OF_RESOURCES_2: There is no space in
 *     LEM
 * REMARKS:
 *   Written to both the Logical interfaces table for
 *   learning purposes, and to the LEM table for forwarding
 *   purposes
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_ingress_lif_add(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID              lif_index,
    SOC_SAND_IN  uint32                      nickname_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO  *trill_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_ingress_lif_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get mapping of TRILL nickname to FEC ID and LIF index
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                  *lif_index -
 *     Index to the Logical interfaces table
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_TRILL_INFO    *trill_info -
 *     Trill attributes(FEC Entry ID,learn_enable).
 *   SOC_SAND_OUT uint8                   *is_found -
 *     indicates if entry was found
 * REMARKS:
 *   The 'lif_index' is returned to the user to enable the
 *   LIF table management
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_ingress_lif_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     nickname_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID            *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_TRILL_INFO *trill_info,
    SOC_SAND_OUT uint8                      *is_found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_ingress_lif_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove TRILL nick-name
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                  lif_index -
 *     Index to the Logical interfaces table
 * REMARKS:
 *   Removed from both the SEM table and to the LEM table. The
 *   'lif_index' is returned to user to enable his management
 *   of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_ingress_lif_remove(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  uint32                    nickname_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                  *lif_index
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_multicast_route_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map a TRILL distribution tree to a FEC
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key -
 *     TRILL multicast key. Contain the Distribution tree, and
 *     may also contain: FID, Originator RBridge Nick-Name, and
 *     Adjacent RBridge Nick-Name
 *   SOC_SAND_IN  uint32                    mc_id -
 *     Multicast ID. The multicast ID should contain the L2
 *     assigned forwarder ports and a list of the adjacent
 *     RBridges, according to the distribution tree topology.
 *     An adjacent RBridge is pointed via the port connected to
 *     it, and a Copy Unique Data that points to the
 *     encapsulation pointer that contains its link-layer
 *     encapsulation.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE        *success -
 *     SOC_SAND_FAILURE_OUT_OF_RESOURCES: There is no space in the
 *     Exact Match table
 * REMARKS:
 *   To fully support TRILL RPF,
 *   soc_ppd_frwrd_trill_multicast_route_set() is called for each
 *   Ingress-NickName, each time with the allowed valid
 *   Adjacent, according to the tree structure. This way, the
 *   Trill RPF is supported. Trill RPF validate that the
 *   packet did not arrive to the RBridge from adjacent that
 *   is not allowed for the packet originator. ECMP with list
 *   of multicast FEC destination is supported. However, the
 *   common usage of this option is when a flooding is
 *   originated upon an unknown MAC address or multicast MAC
 *   address. When the key to the multicast is the TRILL
 *   header, the distribution tree is already chosen, and
 *   therefore the route is to a known FEC that points to a
 *   multicast group.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_multicast_route_add(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key,
    SOC_SAND_IN  uint32                    mc_id,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE        *success
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_multicast_route_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get Mapping of TRILL distribution tree to a FEC
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key -
 *     TRILL multicast key. Contain the Distribution tree, and
 *     may also contain: FID, Originator RBridge Nick-Name, and
 *     Adjacent RBridge Nick-Name
 *   SOC_SAND_OUT uint32                    *mc_id -
 *     Multicast ID. The multicast ID should contain the L2
 *     assigned forwarder ports and a list of the adjacent
 *     RBridges, according to the distribution tree topology.
 *     An adjacent RBridge is pointed via the port connected to
 *     it, and a Copy Unique Data that points to the
 *     encapsulation pointer that contains its link-layer
 *     encapsulation.
 *   SOC_SAND_OUT uint8                   *is_found -
 *     indicates if entry was found
 * REMARKS:
 *   none
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_multicast_route_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key,
    SOC_SAND_OUT uint32                    *mc_id,
    SOC_SAND_OUT uint8                   *is_found
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_multicast_route_remove
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a TRILL distribution tree mapping
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key -
 *     TRILL multicast key. Contain the Distribution tree, and
 *     may also contain: FID, Originator RBridge Nick-Name, and
 *     Adjacent RBridge Nick-Name
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_multicast_route_remove(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_adj_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map SA MAC adress to expected adjacent EEP and expected
 *   system port in SA-Based_adj db. Used for authenticating
 *   incoming trill packets
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     The device id
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS         *mac_address_key -
 *     MAC address to set authentication over it.
 *   SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info -
 *     Authentication information for the given MAC.
 *   SOC_SAND_IN  uint8                   enable -
 *     Set to TRUE to add the entry, and to FALSE to remove the
 *     entry from the DB.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE        *success -
 *     Whether the operation succeeds (upon add). Add operation
 *     may fail if there is no place in the SA Auth DB.
 * REMARKS:
 *   - The DB used for SA Based Adj is also shared for SA
 *   Authorization
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_adj_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS         *mac_address_key,
    SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info,
    SOC_SAND_IN  uint8                   enable,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE        *success
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_trill_adj_info_set" API.
 *     Refer to "soc_ppd_frwrd_trill_adj_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_adj_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS         *mac_address_key,
    SOC_SAND_OUT SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info,
    SOC_SAND_OUT uint8                   *enable
  );

/*********************************************************************
* NAME:
 *   soc_ppd_frwrd_trill_global_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TRILL global attributes
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info -
 *     global settings info
 * REMARKS:
 *   Sets TRILL initiale TTL value
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_global_info_set(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_frwrd_trill_global_info_set" API.
 *     Refer to "soc_ppd_frwrd_trill_global_info_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_global_info_get(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info
  );
/*********************************************************************
*    Sets the inner tpid - used for fine-grained trill
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_native_inner_tpid_add(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  uint32                             tpid,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE           *success
  );
/*********************************************************************
 *    Map high-vid, low-vid to vsi. Used for FGL forwarding
 *    for  VL only high-vid is used.
 *    Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  soc_ppd_frwrd_trill_vsi_entry_add(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     vsi,
    SOC_SAND_IN  uint32                     flags,
    SOC_SAND_IN  uint32                     high_vid,
    SOC_SAND_IN  uint32                     low_vid,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  );
uint32
  soc_ppd_frwrd_trill_vsi_entry_remove(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     vsi,
    SOC_SAND_IN  uint32                     flags,
    SOC_SAND_IN  uint32                     high_vid,
    SOC_SAND_IN  uint32                     low_vid
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_FRWRD_TRILL_INCLUDED__*/
#endif

