/* $Id: arad_pp_port.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_PPC_PORT_INCLUDED__
/* { */
#define __SOC_PPC_PORT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_port.h>

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
  SOC_PPC_PORT_INFO_SET = ARAD_PP_PROC_DESC_BASE_PORT_FIRST,
  SOC_PPC_PORT_INFO_SET_PRINT,
  SOC_PPC_PORT_INFO_SET_UNSAFE,
  SOC_PPC_PORT_INFO_SET_VERIFY,
  SOC_PPC_PORT_INFO_GET,
  SOC_PPC_PORT_INFO_GET_PRINT,
  SOC_PPC_PORT_INFO_GET_VERIFY,
  SOC_PPC_PORT_INFO_GET_UNSAFE,
  SOC_PPC_PORT_STP_STATE_SET,
  SOC_PPC_PORT_STP_STATE_SET_PRINT,
  SOC_PPC_PORT_STP_STATE_SET_UNSAFE,
  SOC_PPC_PORT_STP_STATE_SET_VERIFY,
  SOC_PPC_PORT_STP_STATE_GET,
  SOC_PPC_PORT_STP_STATE_GET_PRINT,
  SOC_PPC_PORT_STP_STATE_GET_VERIFY,
  SOC_PPC_PORT_STP_STATE_GET_UNSAFE,
  SOC_PPC_PORT_GET_PROCS_PTR,
  SOC_PPC_PORT_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */
   SOC_PPC_PORTS_REGS_INIT,
   SOC_PPC_PORTS_INIT,

  /*
   * Last element. Do no touch.
   */
  SOC_PPC_PORT_PROCEDURE_DESC_LAST
} SOC_PPC_PORT_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPC_PORT_TOPOLOGY_ID_NDX_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_PORT_FIRST,
  SOC_PPC_PORT_STP_STATE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_SYS_PHY_PORT_ID_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_DIRECTION_NDX_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_LEARN_DEST_TYPE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_PORT_PROFILE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_TPID_PROFILE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_VLAN_DOMAIN_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_PORT_TYPE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_ORIENTATION_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_ETHER_TYPE_BASED_PROFILE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_MTU_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_DA_NOT_FOUND_PROFILE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_VT_PROFILE_OUT_OF_RANGE_ERR,
  SOC_PPC_PORT_VSI_PROFILE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PPC_PORT_ERR_LAST
} SOC_PPC_PORT_ERR;


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
  arad_pp_port_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );


/*********************************************************************
* NAME:
 *   arad_pp_port_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set main attributes of the port. Generally, these
 *   attributes identify the port and may have use in more
 *   than one module.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  SOC_PPC_PORT_INFO                           *port_info -
 *     Port attributes, including main attributes of the port.
 * REMARKS:
 *   - In order to configure the port default AC use
 *   soc_ppd_l2_lif_ac_add() while ignoring the VIDs i.e. set
 *   VIDs to SOC_PPC_LIF_IGNORE_INNER_VID and
 *   SOC_PPC_LIF_IGNORE_OUTER_VID.- Note that not all port
 *   attributes are included in this configuration. Some
 *   attributes that are used by a single module are
 *   configured by APIs in that module. Typically the API
 *   name is soc_ppd_<module_name>_port_info_set.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_port_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_PORT_INFO                           *port_info
  );


uint32
  arad_pp_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_PORT_INFO                           *port_info
  );

uint32
  arad_pp_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_port_info_set_unsafe" API.
 *     Refer to "arad_pp_port_info_set_unsafe" API for details.
*********************************************************************/
uint32
  arad_pp_port_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_PORT_INFO                           *port_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_port_stp_state_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets the STP state of a port in a specific topology ID.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Local port ID.
 *   SOC_SAND_IN  uint32                                  topology_id_ndx -
 *     Topology ID. Set using soc_ppd_vsi_info_set(). Range: 0 -
 *     63.
 *   SOC_SAND_IN  SOC_PPC_PORT_STP_STATE                      stp_state -
 *     STP state of the port (discard/learn/forward)
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_port_stp_state_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  uint32                                  topology_id_ndx,
    SOC_SAND_IN  SOC_PPC_PORT_STP_STATE                      stp_state
  );

uint32
  arad_pp_port_stp_state_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  uint32                                  topology_id_ndx,
    SOC_SAND_IN  SOC_PPC_PORT_STP_STATE                      stp_state
  );

uint32
  arad_pp_port_stp_state_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  uint32                                  topology_id_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_port_stp_state_set_unsafe" API.
 *     Refer to "arad_pp_port_stp_state_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_port_stp_state_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  uint32                                  topology_id_ndx,
    SOC_SAND_OUT SOC_PPC_PORT_STP_STATE                      *stp_state
  );
uint32
  SOC_PPC_PORT_INFO_verify(
    SOC_SAND_IN  int               unit,
    SOC_SAND_IN  SOC_PPC_PORT_INFO *info
  );

soc_error_t arad_pp_port_property_set(int unit, int core, int port, SOC_PPC_PORT_PROPERTY port_property, uint32 value);
soc_error_t arad_pp_port_property_get(int unit, int core, int port, SOC_PPC_PORT_PROPERTY port_property, uint32 *value);

/*********************************************************************
* NAME:
*     arad_pp_port_additional_tpids_set
* TYPE:
*   PROC
* FUNCTION:
*     Set the additional TPIDs.
* INPUT:
*  int                  unit - Identifier of the device to access.
*  SOC_PPC_ADDITIONAL_TPID_VALUES additional_tpids - Array of additional TPIDs.
*     For ARAD/ARAD+, only additional_tpids->tpid_vals[0] is used.
*     For Jericho, 
*     additional_tpids->tpid_vals[0] is used as Tpid1AdditionalValue1. 
*     additional_tpids->tpid_vals[1] is used as Tpid1AdditionalValue2.
*     additional_tpids->tpid_vals[2] is used as Tpid2AdditionalValue1.
*     additional_tpids->tpid_vals[3] is used as Tpid2AdditionalValue2.
* RETURNS:
*     SOC_E_XXX.
*********************************************************************/
soc_error_t arad_pp_port_additional_tpids_set(int unit, SOC_PPC_ADDITIONAL_TPID_VALUES *additional_tpids);

/*********************************************************************
* NAME:
*     arad_pp_port_additional_tpids_get
* TYPE:
*   PROC
* FUNCTION:
*     Get the additional TPIDs.
* INPUT:
*  int                  unit - Identifier of the device to access.
*  SOC_PPC_ADDITIONAL_TPID_VALUES additional_tpids - Array of additional TPIDs.
*     For ARAD/ARAD+, only additional_tpids->tpid_vals[0] is used.
*     For Jericho, 
*     additional_tpids->tpid_vals[0] is used as Tpid1AdditionalValue1. 
*     additional_tpids->tpid_vals[1] is used as Tpid1AdditionalValue2.
*     additional_tpids->tpid_vals[2] is used as Tpid2AdditionalValue1.
*     additional_tpids->tpid_vals[3] is used as Tpid2AdditionalValue2.
* RETURNS:
*     SOC_E_XXX.
*********************************************************************/
soc_error_t arad_pp_port_additional_tpids_get(int unit, SOC_PPC_ADDITIONAL_TPID_VALUES *additional_tpids);

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_PORT_INCLUDED__*/
#endif
