
/* $Id: arad_pp_frwrd_trill.h,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_TRILL_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_TRILL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_sa_auth.h>
#include <soc/dpp/ARAD/arad_reg_access.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define ARAD_PP_TRILL_SA_AUTH_ENTRY_IS_DYNAMIC (FALSE)
#define ARAD_PP_TRILL_SA_AUTH_ENTRY_AGE (3)

#define SOC_DPP_PP_FRWRD_TRILL_EEDB_INVALID_ENTRY(unit) ((1 << soc_reg_field_length(unit, EGQ_TRILL_CONFIGr, TRILL_OUT_LIF_BRIDGEf)) - 1)
#define ARAD_PP_FRWRD_TRILL_EEDB_INVALID_ENTRY_B0       0


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
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_SET = ARAD_PP_PROC_DESC_BASE_FRWRD_TRILL_FIRST,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_SET_PRINT,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_SET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_SET_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_GET,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_GET_PRINT,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_GET_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_KEY_MASK_GET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_ADD,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_ADD_PRINT,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_ADD_UNSAFE,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_ADD_VERIFY,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_GET,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_GET_PRINT,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_GET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_GET_VERIFY,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_REMOVE,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_REMOVE_PRINT,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_TRILL_UNICAST_ROUTE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_ADD,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_ADD_PRINT,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_ADD_UNSAFE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_ADD_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_GET,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_GET_PRINT,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_GET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_GET_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_REMOVE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_REMOVE_PRINT,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_ROUTE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_SET,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_SET_PRINT,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_SET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_SET_VERIFY,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_GET,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_GET_PRINT,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_GET_VERIFY,
  ARAD_PP_FRWRD_TRILL_ADJ_INFO_GET_UNSAFE,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_SET,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_SET_PRINT,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_SET_UNSAFE,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_SET_VERIFY,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_GET,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_GET_PRINT,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_GET_VERIFY,
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_GET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_GET_PROCS_PTR,
  ARAD_PP_FRWRD_TRILL_GET_ERRS_PTR,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_ADD_UNSAFE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_ADD_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_ADD,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_REMOVE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_GET,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_REMOVE_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_GET_VERIFY,
  ARAD_PP_FRWRD_TRILL_MULTICAST_SOURCE_GET_UNSAFE,
  ARAD_PP_FRWRD_TRILL_VSI_ENTRY_ADD,
  ARAD_PP_FRWRD_TRILL_VSI_ENTRY_ADD_VERIFY,
  ARAD_PP_FRWRD_TRILL_VSI_ENTRY_ADD_UNSAFE,
  ARAD_PP_FRWRD_TRILL_VSI_ENTRY_REMOVE,
  ARAD_PP_FRWRD_TRILL_VSI_ENTRY_REMOVE_VERIFY,
  ARAD_PP_FRWRD_TRILL_VSI_ENTRY_REMOVE_UNSAFE,
  ARAD_PP_FRWRD_TRILL_NATIVE_INNER_TPID_ADD,
  ARAD_PP_FRWRD_TRILL_NATIVE_INNER_TPID_ADD_UNSAFE,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_TRILL_PROCEDURE_DESC_LAST
} ARAD_PP_FRWRD_TRILL_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_FRWRD_TRILL_NICKNAME_KEY_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_FRWRD_TRILL_FIRST,
  ARAD_PP_FRWRD_TRILL_SUCCESS_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_TRILL_MC_ID_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_TRILL_TREE_NICK_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_TRILL_ING_NICK_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_TRILL_ADJACENT_EEP_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_TRILL_EXPECT_ADJACENT_EEP_OUT_OF_RANGE_ERR,
  ARAD_PP_FRWRD_TRILL_CFG_TTL_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  ARAD_PP_FRWRD_TRILL_ERR_LAST
} ARAD_PP_FRWRD_TRILL_ERR;

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
  arad_pp_frwrd_trill_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pp_l2_lif_trill_uc_get_internal_unsafe(
    SOC_SAND_IN   int                                unit,
    SOC_SAND_IN   uint32                             nickname,
    SOC_SAND_IN   uint8                              ignore_key,
    SOC_SAND_OUT  SOC_PPC_LIF_ID                    *lif_index,
    SOC_SAND_OUT  SOC_PPC_L2_LIF_TRILL_INFO         *trill_info,
    SOC_SAND_OUT  uint8                             *is_found
  );

uint32
  arad_pp_l2_lif_trill_add_internal_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                              nickname,
    SOC_SAND_IN  uint8                               ignore_key,
    SOC_SAND_IN  SOC_PPC_LIF_ID                      lif_index,
    SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO           *trill_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE            *success
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_multicast_key_mask_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the key type of TRILL multicast routes lookup. The
 *   following fields are optional: Ing-Nick-key;
 *   Adjacent-EEP-key; FID-key
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields -
 *     Trill multicast route fields to be masked upon lookup
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_multicast_key_mask_set_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields
  );

uint32
  arad_pp_frwrd_trill_multicast_key_mask_set_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields
  );

uint32
  arad_pp_frwrd_trill_multicast_key_mask_get_verify(
    SOC_SAND_IN  int                     unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_trill_multicast_key_mask_set_unsafe" API.
 *     Refer to
 *     "arad_pp_frwrd_trill_multicast_key_mask_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_multicast_key_mask_get_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_OUT SOC_PPC_TRILL_MC_MASKED_FIELDS  *masked_fields
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_unicast_route_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map nick-name to a FEC entry ID. Used for forwarding
 *   packets with the nick name as destination to the FEC
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_IN  ASOC_PPC_FRWRD_DECISION_INFO *fwd_decision
 *     Forward decision for trill transit. dest_id=fec_id
 *     fec_id - FEC Entry ID. The FEC is expected to hold the out-port as
 *              destination and an EEP that points to the link layer
 *              encapsulation towards the adjacent RBridge. The FEC ID
 *              may also point to ECMP with list of adjacent RBridges,
 *              and the connected ports.
 *     learn_enable - indicate if learn enable on this interface.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success -
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
  arad_pp_frwrd_trill_unicast_route_add_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                       nickname_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO *fwd_decision,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
  );

uint32
  arad_pp_frwrd_trill_unicast_route_add_verify(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                      nickname_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO *fwd_decision
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_unicast_route_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get mapping of TRILL nickname to FEC ID and LIF index
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO *learn_info,
 *     Forward decision for trill transit. dest_id=fec_id
 *   SOC_SAND_OUT uint8                     *is_found -
 *     indicates if entry was found
 * REMARKS:
 *   The 'lif_index' is returned to the user to enable the
 *   LIF table management
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_unicast_route_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     nickname_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO *learn_info,
    SOC_SAND_OUT uint8                     *is_found
  );

uint32
  arad_pp_frwrd_trill_unicast_route_get_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                      nickname_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_unicast_route_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove TRILL nick-name
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      nickname_key -
 *     TRILL NickName key
 * REMARKS:
 *   Removed from both the SEM table and to the LEM table. The
 *   'lif_index' is returned to user to enable his management
 *   of the LIF table
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_unicast_route_remove_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                      nickname_key
  );

uint32
  arad_pp_frwrd_trill_unicast_route_remove_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                      nickname_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_ingress_lif_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map nick-name to a FEC entry ID. Used to associate 
 *   the FEC as learning information, upon receiving packets 
 *    with the Nick-Name as the source address
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_LIF_ID                  lif_index -
 *     Index to the Logical interfaces table
 *   SOC_SAND_IN  uint32                      nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO *trill_info
 *     trill_info attributes:
 *     fec_id - FEC Entry ID. The FEC is expected to hold the out-port as
 *              destination and an EEP that points to the link layer
 *              encapsulation towards the adjacent RBridge. The FEC ID
 *              may also point to ECMP with list of adjacent RBridges,
 *              and the connected ports.
 *     learn_enable - indicate if learn enable on this interface.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success -
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
  arad_pp_frwrd_trill_ingress_lif_add_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID               lif_index,
    SOC_SAND_IN  uint32                       nickname_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO   *trill_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
  );

uint32
  arad_pp_frwrd_trill_ingress_lif_add_verify(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_LIF_ID              lif_index,
    SOC_SAND_IN  uint32                      nickname_key,
    SOC_SAND_IN  SOC_PPC_L2_LIF_TRILL_INFO  *trill_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_ingress_lif_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get mapping of TRILL nickname to FEC ID and LIF index
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      nickname_key -
 *     TRILL NickName key
 *   SOC_SAND_OUT SOC_PPC_LIF_ID                  *lif_index -
 *     Index to the Logical interfaces table
 *   SOC_SAND_OUT SOC_PPC_L2_LIF_TRILL_INFO *trill_info,
 *     Trill attributes(FEC Entry ID, Learning enable/disable).
 *   SOC_SAND_OUT uint8                     *is_found -
 *     indicates if entry was found
 * REMARKS:
 *   The 'lif_index' is returned to the user to enable the
 *   LIF table management
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_ingress_lif_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     nickname_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID            *lif_index,
    SOC_SAND_OUT SOC_PPC_L2_LIF_TRILL_INFO *trill_info,
    SOC_SAND_OUT uint8                     *is_found
  );

uint32
  arad_pp_frwrd_trill_ingress_lif_get_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                      nickname_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_ingress_lif_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove TRILL nick-name
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                      nickname_key -
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
  arad_pp_frwrd_trill_ingress_lif_remove_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                      nickname_key,
    SOC_SAND_OUT SOC_PPC_LIF_ID                  *lif_index
  );

uint32
  arad_pp_frwrd_trill_ingress_lif_remove_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                      nickname_key
  );


/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_multicast_route_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map a TRILL distribution tree to a FEC
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key -
 *     TRILL multicast key. Contain the Distribution tree, and
 *     may also contain: FID, Originator RBridge Nick-Name, and
 *     Adjacent RBridge Nick-Name
 *   SOC_SAND_IN  uint32                      mc_id -
 *     Multicast ID. The multicast ID should contain the L2
 *     assigned forwarder ports and a list of the adjacent
 *     RBridges, according to the distribution tree topology.
 *     An adjacent RBridge is pointed via the port connected to
 *     it, and a Copy Unique Data that points to the
 *     encapsulation pointer that contains its link-layer
 *     encapsulation.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success -
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
  arad_pp_frwrd_trill_multicast_route_add_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key,
    SOC_SAND_IN  uint32                      mc_id,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success
  );

uint32
  arad_pp_frwrd_trill_multicast_route_add_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key,
    SOC_SAND_IN  uint32                      mc_id
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_multicast_route_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get Mapping of TRILL distribution tree to a FEC
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key -
 *     TRILL multicast key. Contain the Distribution tree, and
 *     may also contain: FID, Originator RBridge Nick-Name, and
 *     Adjacent RBridge Nick-Name
 *   SOC_SAND_OUT uint32                      *mc_id -
 *     Multicast ID. The multicast ID should contain the L2
 *     assigned forwarder ports and a list of the adjacent
 *     RBridges, according to the distribution tree topology.
 *     An adjacent RBridge is pointed via the port connected to
 *     it, and a Copy Unique Data that points to the
 *     encapsulation pointer that contains its link-layer
 *     encapsulation.
 *   SOC_SAND_OUT uint8                     *is_found -
 *     indicates if entry was found
 * REMARKS:
 *   none
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_multicast_route_get_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key,
    SOC_SAND_OUT uint32                      *mc_id,
    SOC_SAND_OUT uint8                     *is_found
  );

uint32
  arad_pp_frwrd_trill_multicast_route_get_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_multicast_route_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remove a TRILL distribution tree mapping
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
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
  arad_pp_frwrd_trill_multicast_route_remove_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key
  );

uint32
  arad_pp_frwrd_trill_multicast_route_remove_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY      *trill_mc_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_adj_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Map SA MAC adress to expected adjacent EEP and expected
 *   system port in SA-Based_adj db. Used for authenticating
 *   incoming trill packets
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     The device id
 *   SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS           *mac_address_key -
 *     MAC address to set authentication over it.
 *   SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info -
 *     Authentication information for the given MAC.
 *   SOC_SAND_IN  uint8                     enable -
 *     Set to TRUE to add the entry, and to FALSE to remove the
 *     entry from the DB.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success -
 *     Whether the operation succeeds (upon add). Add operation
 *     may fail if there is no place in the SA Auth DB.
 * REMARKS:
 *   - The DB used for SA Based Adj is also shared for SA
 *   Authorization
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_adj_info_set_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS           *mac_address_key,
    SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info,
    SOC_SAND_IN  uint8                     enable,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE          *success
  );

uint32
  arad_pp_frwrd_trill_adj_info_set_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS           *mac_address_key,
    SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info,
    SOC_SAND_IN  uint8                     enable
  );

uint32
  arad_pp_frwrd_trill_adj_info_get_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS           *mac_address_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_trill_adj_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_trill_adj_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_adj_info_get_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_SAND_PP_MAC_ADDRESS           *mac_address_key,
    SOC_SAND_OUT SOC_PPC_TRILL_ADJ_INFO          *mac_auth_info,
    SOC_SAND_OUT uint8                     *enable
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_global_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set TRILL global attributes
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info -
 *     global settings info
 * REMARKS:
 *   Sets TRILL initiale TTL value
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_global_info_set_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info
  );

uint32
  arad_pp_frwrd_trill_global_info_set_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info
  );

uint32
  arad_pp_frwrd_trill_global_info_get_verify(
    SOC_SAND_IN  int                     unit
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_frwrd_trill_global_info_set_unsafe" API.
 *     Refer to "arad_pp_frwrd_trill_global_info_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_frwrd_trill_global_info_get_unsafe(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *glbl_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_frwrd_trill module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_trill_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_frwrd_trill_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_frwrd_trill module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_trill_get_errs_ptr(void);

uint32
  SOC_PPC_TRILL_MC_MASKED_FIELDS_verify(
    SOC_SAND_IN  SOC_PPC_TRILL_MC_MASKED_FIELDS *info
  );

uint32
  SOC_PPC_TRILL_MC_ROUTE_KEY_verify(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_PPC_TRILL_MC_ROUTE_KEY *info
  );

uint32
  SOC_PPC_TRILL_ADJ_INFO_verify(
    SOC_SAND_IN  SOC_PPC_TRILL_ADJ_INFO *info
  );

uint32
  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_TRILL_GLOBAL_INFO *info
  );

uint32
arad_pp_frwrd_trill_vsi_entry_add_unsafe(
   SOC_SAND_IN  int                                 unit,
   SOC_SAND_IN  uint32                                 vsi,
   SOC_SAND_IN  uint32                                 flags,
   SOC_SAND_IN  uint32                                 high_vid,
   SOC_SAND_IN  uint32                                 low_vid,
   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success);
uint32
  arad_pp_frwrd_trill_vsi_entry_add_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     vsi,
    SOC_SAND_IN  uint32                     high_vid,
    SOC_SAND_IN  uint32                     low_vid
  );
uint32
arad_pp_frwrd_trill_vsi_entry_remove_unsafe(
   SOC_SAND_IN  int                                 unit,
   SOC_SAND_IN  uint32                                 vsi,
   SOC_SAND_IN  uint32                                 flags,
   SOC_SAND_IN  uint32                                 high_vid,
   SOC_SAND_IN  uint32                                 low_vid);

uint32
  arad_pp_frwrd_trill_vsi_entry_remove_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  uint32                     vsi,
    SOC_SAND_IN  uint32                     high_vid,
    SOC_SAND_IN  uint32                     low_vid
  );

uint32
arad_pp_frwrd_trill_native_inner_tpid_add_unsafe(
   SOC_SAND_IN  int                                 unit,
   SOC_SAND_IN  uint32                                 tpid,
   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
 );
uint32
arad_pp_frwrd_trill_native_inner_tpid_add_verify(
   SOC_SAND_IN  int                     unit,
   SOC_SAND_IN  uint32                     tpid
 );

uint32
arad_pp_frwrd_trill_native_inner_tpid_remove_unsafe(
   SOC_SAND_IN  int                                 unit,
   SOC_SAND_IN  uint32                                 tpid
 );
uint32
arad_pp_frwrd_trill_native_inner_tpid_remove_verify(
   SOC_SAND_IN  int                     unit,
   SOC_SAND_IN  uint32                     tpid
 );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_TRILL_INCLUDED__*/
#endif



