
/* $Id: arad_pp_eg_ac.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_EG_AC_INCLUDED__
/* { */
#define __ARAD_PP_EG_AC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_eg_ac.h>
#include <soc/dpp/PPC/ppc_api_general.h>


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
  SOC_PPC_EG_AC_INFO_SET = ARAD_PP_PROC_DESC_BASE_EG_AC_FIRST,
  SOC_PPC_EG_AC_INFO_SET_PRINT,
  SOC_PPC_EG_AC_INFO_SET_UNSAFE,
  SOC_PPC_EG_AC_INFO_SET_VERIFY,
  SOC_PPC_EG_AC_INFO_GET,
  SOC_PPC_EG_AC_INFO_GET_PRINT,
  SOC_PPC_EG_AC_INFO_GET_VERIFY,
  SOC_PPC_EG_AC_INFO_GET_UNSAFE,
  SOC_PPC_EG_AC_MP_INFO_SET,
  SOC_PPC_EG_AC_MP_INFO_SET_PRINT,
  SOC_PPC_EG_AC_MP_INFO_SET_UNSAFE,
  SOC_PPC_EG_AC_MP_INFO_SET_VERIFY,
  SOC_PPC_EG_AC_MP_INFO_GET,
  SOC_PPC_EG_AC_MP_INFO_GET_PRINT,
  SOC_PPC_EG_AC_MP_INFO_GET_VERIFY,
  SOC_PPC_EG_AC_MP_INFO_GET_UNSAFE,
  ARAD_PP_EG_AC_PORT_VSI_INFO_ADD,
  ARAD_PP_EG_AC_PORT_VSI_INFO_ADD_PRINT,
  ARAD_PP_EG_AC_PORT_VSI_INFO_ADD_UNSAFE,
  ARAD_PP_EG_AC_PORT_VSI_INFO_ADD_VERIFY,
  ARAD_PP_EG_AC_PORT_VSI_INFO_REMOVE,
  ARAD_PP_EG_AC_PORT_VSI_INFO_REMOVE_PRINT,
  ARAD_PP_EG_AC_PORT_VSI_INFO_REMOVE_UNSAFE,
  ARAD_PP_EG_AC_PORT_VSI_INFO_REMOVE_VERIFY,
  ARAD_PP_EG_AC_PORT_VSI_INFO_GET,
  ARAD_PP_EG_AC_PORT_VSI_INFO_GET_PRINT,
  ARAD_PP_EG_AC_PORT_VSI_INFO_GET_UNSAFE,
  ARAD_PP_EG_AC_PORT_VSI_INFO_GET_VERIFY,
  ARAD_PP_EG_AC_PORT_CVID_INFO_ADD,
  ARAD_PP_EG_AC_PORT_CVID_INFO_ADD_PRINT,
  ARAD_PP_EG_AC_PORT_CVID_INFO_ADD_UNSAFE,
  ARAD_PP_EG_AC_PORT_CVID_INFO_ADD_VERIFY,
  ARAD_PP_EG_AC_PORT_CVID_INFO_REMOVE,
  ARAD_PP_EG_AC_PORT_CVID_INFO_REMOVE_PRINT,
  ARAD_PP_EG_AC_PORT_CVID_INFO_REMOVE_UNSAFE,
  ARAD_PP_EG_AC_PORT_CVID_INFO_REMOVE_VERIFY,
  ARAD_PP_EG_AC_PORT_CVID_INFO_GET,
  ARAD_PP_EG_AC_PORT_CVID_INFO_GET_PRINT,
  ARAD_PP_EG_AC_PORT_CVID_INFO_GET_UNSAFE,
  ARAD_PP_EG_AC_PORT_CVID_INFO_GET_VERIFY,
  ARAD_PP_EG_AC_PORT_CVID_MAP,
  ARAD_PP_EG_AC_PORT_CVID_MAP_PRINT,
  ARAD_PP_EG_AC_PORT_CVID_MAP_UNSAFE,
  ARAD_PP_EG_AC_PORT_CVID_MAP_VERIFY,
  ARAD_PP_EG_AC_PORT_VSI_MAP,
  ARAD_PP_EG_AC_PORT_VSI_MAP_PRINT,
  ARAD_PP_EG_AC_PORT_VSI_MAP_UNSAFE,
  ARAD_PP_EG_AC_PORT_VSI_MAP_VERIFY,
  ARAD_PP_EG_AC_GET_PROCS_PTR,
  ARAD_PP_EG_AC_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_AC_PROCEDURE_DESC_LAST
} ARAD_PP_EG_AC_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_EG_AC_SUCCESS_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_EG_AC_FIRST,
  ARAD_PP_EG_AC_VLAN_DOMAIN_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_AC_PEP_EDIT_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_AC_PCP_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_AC_EDIT_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_AC_NOF_TAGS_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_AC_MAX_LEVEL_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_AC_TYPE_INVALID_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */
  ARAD_PP_ESEM_REMOVE_FAILED,

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_AC_ERR_LAST
} ARAD_PP_EG_AC_ERR;

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
  arad_pp_eg_ac_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the editing information for packets
 *   associated with AC.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx -
 *     AC ID. Should be equal to the LIF ID in the incoming LIF
 *     module.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info -
 *     Information according to which to edit the
 *     packet.
 * REMARKS:
 *   - Packets associated with AC by the ingress are
 *   manipulated according to this setting. For packets
 *   without out-AC-ID associated, see
 *   soc_ppd_eg_ac_port_vsi_info()_set and
 *   soc_ppd_eg_ac_port_cvid_info_set()
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info
  );

uint32
  arad_pp_eg_ac_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info
  );

uint32
  arad_pp_eg_ac_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_ac_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_ac_info_set_unsafe" API for details.
*********************************************************************/
uint32
  arad_pp_eg_ac_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_mp_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   This function is used to define a Maintenance Point (MP)
 *   on an outgoing Attachment-Circuit (port x VSI) and
 *   MD-level, and to determine the action to perform. If the
 *   MP is one of the 4K accelerated MEPs, the function
 *   configures the related OAMP databases and associates the
 *   AC and MD-Level with a user-provided handle. This handle
 *   is later used by user to access OAMP database for this
 *   MEP.
 * INPUT:
 *   SOC_SAND_IN  int                          unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx -
 *     AC ID. Should be equal to the LIF ID in the incoming LIF
 *     module.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_MP_INFO                *info -
 *     MP configuration information.
 * REMARKS:
 *   Should only be used if the MP is down MEP or MIP.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_mp_info_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_MP_INFO                *info
  );

uint32
  arad_pp_eg_ac_mp_info_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_MP_INFO                *info
  );

uint32
  arad_pp_eg_ac_mp_info_get_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_ac_mp_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_ac_mp_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_ac_mp_info_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                        out_ac_ndx,
    SOC_SAND_OUT SOC_PPC_EG_AC_MP_INFO                *info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_port_vsi_info_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the editing information for packets NOT
 *   associated with AC and to be transmitted from VBP port
 *   (not CEP port).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx -
 *     AC ID, for T20E: this is not relevant and may be
 *     ignored. For Arad-B this is the out-AC that the port x
 *     VSI is mapped to.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key -
 *     The key (port (vlan-domain) x VSI) to set editing
 *     information to. In Arad this is also is mapped to the
 *     give AC.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info -
 *     Information according to which to edit the
 *     packet.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds (upon add). Operation may
 *     fail upon unavailable resources (Exact Match). In T20E
 *     this operation always success.
 * REMARKS:
 *   - Packets NOT associated with AC by the ingress and to
 *   be transmitted out through VBP port are manipulated
 *   according to this setting.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_port_vsi_info_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_eg_ac_port_vsi_info_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_port_vsi_info_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Removes editing information of packets NOT
 *   associated with AC and to be transmitted from VBP port
 *   (not CEP port).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key -
 *     The key (port (vlan-domain) x VSI) to set editing
 *     information to. In Arad this is also is mapped to the
 *     give AC.
 *   SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac -
 *     AC ID associated with the Vbp_key, for T20E: this is not
 *     relevant and may be ignored. For Arad-B this is the
 *     out-AC that the port x VSI is mapped to.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_port_vsi_info_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac
  );

uint32
  arad_pp_eg_ac_port_vsi_info_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_port_vsi_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the editing information for packets NOT
 *   associated with AC and to be transmitted from VBP port
 *   (not CEP port).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key -
 *     The key (port(vlan-domain)x VSI) to set editing
 *     information to. In Arad this is also is mapped to the
 *     give AC.
 *   SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac -
 *     AC ID associated with the Vbp_key, for T20E: this is not
 *     relevant and may be ignored. For Arad-B this is the
 *     out-AC that the port x VSI is mapped to.
 *   SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info -
 *     Information according to which to edit the
 *     packet.
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: The entry was found, 'ac_info' is validFALSE: The
 *     entry was not found, 'ac_info' is invalid
 * REMARKS:
 *   - Packets NOT associated with AC by the ingress and to
 *   be transmitted out through VBP port are manipulated
 *   according to this setting.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_port_vsi_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac,
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_eg_ac_port_vsi_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY                       *vbp_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_port_cvid_info_add_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the editing information for packets NOT
 *   associated with AC and to be transmitted from CEP port
 *   (not VBP port).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx -
 *     AC ID, for T20E: this is not relevant and may be
 *     ignored. For Arad-B this is the out-AC that the port x
 *     CVID is mapped to.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key -
 *     The key (port(vlan-domain) x CVID) to set editing
 *     information to. In Arad this is also is mapped to the
 *     give AC.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info -
 *     Information according to which to edit the
 *     packet.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success -
 *     Whether the operation succeeds (upon add). Operation may
 *     fail upon unavailable resources (Exact Match). In T20E
 *     this operation always success.
 * REMARKS:
 *   - Packets NOT associated with AC by the ingress and to
 *   be transmitted out through CEP port are manipulated
 *   according to this setting.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_port_cvid_info_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  );

uint32
  arad_pp_eg_ac_port_cvid_info_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_AC_ID                               out_ac_ndx,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO                          *ac_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_port_cvid_info_remove_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Removes editing information of packets NOT
 *   associated with AC and to be transmitted from CEP port
 *   (not VBP port).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key -
 *     The key (port(vlan-domain) x CVID) to set editing
 *     information to. In Arad this is also is mapped to the
 *     give AC.
 *   SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac -
 *     AC ID associated with the Vbp_key, for T20E: this is not
 *     relevant and may be ignored. For Arad-B this is the
 *     out-AC that the port x CVID is mapped to.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_port_cvid_info_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac
  );

uint32
  arad_pp_eg_ac_port_cvid_info_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_port_cvid_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Gets the editing information for packets NOT
 *   associated with AC and to be transmitted from CEP port
 *   (not VBP port).
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key -
 *     The key (port(vlan-domain) x CVID) to set editing
 *     information to. In Arad this is also is mapped to the
 *     give AC.
 *   SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac -
 *     AC ID associated with the Vbp_key, for T20E: this is not
 *     relevant and may be ignored. For Arad-B this is the
 *     out-AC that the port x CVID is mapped to.
 *   SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info -
 *     Information according to which to edit the
 *     packet.
 *   SOC_SAND_OUT uint8                                 *found -
 *     TRUE: The entry was found, 'ac_info' is validFALSE: The
 *     entry was not found, 'ac_info' is invalid
 * REMARKS:
 *   - Packets NOT associated with AC by the ingress and to
 *   be transmitted out through CEP port are manipulated
 *   according to this setting.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_ac_port_cvid_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key,
    SOC_SAND_OUT SOC_PPC_AC_ID                               *out_ac,
    SOC_SAND_OUT SOC_PPC_EG_AC_INFO                          *ac_info,
    SOC_SAND_OUT uint8                                 *found
  );

uint32
  arad_pp_eg_ac_port_cvid_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY                  *cep_key
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_eg_ac module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_eg_ac_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_eg_ac_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_eg_ac module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_eg_ac_get_errs_ptr(void);

uint32
  SOC_PPC_EG_AC_VBP_KEY_verify(
    SOC_SAND_IN  SOC_PPC_EG_AC_VBP_KEY *info
  );

uint32
  SOC_PPC_EG_AC_CEP_PORT_KEY_verify(
    SOC_SAND_IN  SOC_PPC_EG_AC_CEP_PORT_KEY *info
  );

uint32
  SOC_PPC_EG_VLAN_EDIT_CEP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_CEP_INFO *info
  );

uint32
  SOC_PPC_EG_VLAN_EDIT_VLAN_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_VLAN_EDIT_VLAN_INFO *info
  );

uint32
  SOC_PPC_EG_AC_VLAN_EDIT_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_AC_VLAN_EDIT_INFO *info
  );

uint32
  SOC_PPC_EG_AC_INFO_verify(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  SOC_PPC_EG_AC_INFO *info
  );

uint32
  SOC_PPC_EG_AC_MP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_EG_AC_MP_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_EG_AC_INCLUDED__*/
#endif



