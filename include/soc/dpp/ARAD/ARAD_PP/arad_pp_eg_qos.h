/* $Id: arad_pp_eg_qos.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_EG_QOS_INCLUDED__
/* { */
#define __ARAD_PP_EG_QOS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_eg_qos.h>
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
  SOC_PPC_EG_QOS_PORT_INFO_SET = ARAD_PP_PROC_DESC_BASE_EG_QOS_FIRST,
  SOC_PPC_EG_QOS_PORT_INFO_SET_PRINT,
  SOC_PPC_EG_QOS_PORT_INFO_SET_UNSAFE,
  SOC_PPC_EG_QOS_PORT_INFO_SET_VERIFY,
  SOC_PPC_EG_QOS_PORT_INFO_GET,
  SOC_PPC_EG_QOS_PORT_INFO_GET_PRINT,
  SOC_PPC_EG_QOS_PORT_INFO_GET_VERIFY,
  SOC_PPC_EG_QOS_PORT_INFO_GET_UNSAFE,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_PRINT,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_UNSAFE,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_SET_VERIFY,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_PRINT,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_VERIFY,
  SOC_PPC_EG_QOS_PARAMS_PHP_REMARK_GET_UNSAFE,
  SOC_PPC_EG_QOS_PARAMS_REMARK_SET,
  SOC_PPC_EG_QOS_PARAMS_REMARK_SET_PRINT,
  SOC_PPC_EG_QOS_PARAMS_REMARK_SET_UNSAFE,
  SOC_PPC_EG_QOS_PARAMS_REMARK_SET_VERIFY,
  SOC_PPC_EG_QOS_PARAMS_REMARK_GET,
  SOC_PPC_EG_QOS_PARAMS_REMARK_GET_PRINT,
  SOC_PPC_EG_QOS_PARAMS_REMARK_GET_VERIFY,
  SOC_PPC_EG_QOS_PARAMS_REMARK_GET_UNSAFE,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_SET,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_SET_PRINT,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_SET_UNSAFE,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_SET_VERIFY,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_GET,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_GET_PRINT,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_GET_VERIFY,
  SOC_PPC_EG_ENCAP_QOS_PARAMS_REMARK_GET_UNSAFE,
  ARAD_PP_EG_QOS_GET_PROCS_PTR,
  ARAD_PP_EG_QOS_GET_ERRS_PTR,
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_QOS_PROCEDURE_DESC_LAST
} ARAD_PP_EG_QOS_PROCEDURE_DESC;

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  ARAD_PP_EG_QOS_DSCP_EXP_OUT_OF_RANGE_ERR = ARAD_PP_ERR_DESC_BASE_EG_QOS_FIRST,
  ARAD_PP_EG_QOS_IN_DSCP_EXP_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_QOS_EXP_MAP_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_QOS_PHP_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_PKT_HDR_OUT_OF_RANGE_ERR,
  ARAD_PP_REMARK_PROFILE_OUT_OF_RANGE_ERR,
  ARAD_PP_EG_ENCAP_QOS_PKT_TYPE_OUT_OF_RANGE_ERR,
  ARAD_PP_ECN_CAPABLE_OUT_OF_RANGE_ERR,
  /*
   * } Auto generated. Do not edit previous section.
   */


  /*
   * Last element. Do no touch.
   */
  ARAD_PP_EG_QOS_ERR_LAST
} ARAD_PP_EG_QOS_ERR;

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
  arad_pp_eg_qos_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_qos_port_info_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets port information for egress QoS setting, including
 *   profiles used for QoS remarking.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Out Local port ID.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info -
 *     The port information for QoS remark
 * REMARKS:
 *   Arad-B only API, error is returned if called for T20E.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_qos_port_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  );

uint32
  arad_pp_eg_qos_port_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  );

uint32
  arad_pp_eg_qos_port_info_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_qos_port_info_set_unsafe" API.
 *     Refer to "arad_pp_eg_qos_port_info_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_qos_port_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  int                                 core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_qos_params_php_remark_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets how to remark QoS parameters upon PHP operation.
 *   When uniform pop performed the dscp_exp value is
 *   remarked.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key -
 *     The key used for mapping including exp of the poped
 *     header
 *   SOC_SAND_IN  uint32                                  dscp_exp -
 *     The new value for dscp_exp, this overwrites the dscp_exp
 *     value calculated by the ingress pipe. This value will be
 *     mapped and used for the QoS fileds in the transmitted
 *     packet see soc_ppd_eg_qos_params_remark_set()
 * REMARKS:
 *   - Arad-B only API, error is returned if called for
 *   T20E. - When pop type is pipe then the dscp_exp value is
 *   taken from the TOS/TC/EXP field in the networking header
 *   above the popped MPLS shim - When pop into MPLS the
 *   dscp_exp value not modified
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_qos_params_php_remark_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_IN  uint32                                  dscp_exp
  );

uint32
  arad_pp_eg_qos_params_php_remark_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_IN  uint32                                  dscp_exp
  );

uint32
  arad_pp_eg_qos_params_php_remark_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_qos_params_php_remark_set_unsafe" API.
 *     Refer to "arad_pp_eg_qos_params_php_remark_set_unsafe" API
 *     for details.
*********************************************************************/
uint32
  arad_pp_eg_qos_params_php_remark_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_OUT uint32                                  *dscp_exp
  );

#ifdef BCM_88660
uint32
  arad_pp_eg_qos_params_marking_set_verify(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  );

uint32
  arad_pp_eg_qos_params_marking_set_unsafe(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  );

uint32
  arad_pp_eg_qos_params_marking_get_verify(
    SOC_SAND_IN     int unit,
    SOC_SAND_IN     SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_OUT    SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  );
uint32
  arad_pp_eg_qos_params_marking_get_unsafe(
    SOC_SAND_IN     int unit,
    SOC_SAND_IN     SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_OUT    SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  );

uint32
  arad_pp_eg_qos_global_info_set_verify(
   SOC_SAND_IN int unit,
   SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );

uint32
  arad_pp_eg_qos_global_info_set_unsafe(
   SOC_SAND_IN int unit,
   SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );

uint32 
  arad_pp_eg_qos_global_info_get_verify(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );

uint32 
  arad_pp_eg_qos_global_info_get_unsafe(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );

#endif /* BCM_88660 */
/*********************************************************************
* NAME:
 *   arad_pp_eg_qos_params_remark_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remark QoS parameters, i.e. map in-dscp/exp and DP to
 *   out-dscp/exp in order to be set in forwarding headers.
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key -
 *     Incomming qos value (in-dscp-exp) as set by ingress
 *     pipe, or overwritten by POP functionalty see
 *     soc_ppd_eg_qos_params_php_remark_set()
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params -
 *     This mapped values, used to set the Qos fields (TOS, TC,
 *     EXP) of the outer header(forwarded header and if PHP was
 *     performed then the network header above the
 *     forwarded/poped header)
 * REMARKS:
 *   Arad/Petra-B only API, error is returned if called for T20E.
 *   - The out_qos_params used for - Setting the Qos of the
 *   forwarded header- Setting the Qos encapsulated header in
 *   case the encapsulation of type uniform - When tunnel
 *   encapsulation pushed to the packet and the encapsulation
 *   type is uniform, then the EXP of the pushed tunnel is
 *   taken from out_qos_params
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_qos_params_remark_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  );

uint32
  arad_pp_eg_qos_params_remark_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  );

uint32
  arad_pp_eg_qos_params_remark_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_qos_params_remark_set_unsafe" API.
 *     Refer to "arad_pp_eg_qos_params_remark_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_pp_eg_qos_params_remark_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_OUT SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_encap_qos_params_remark_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *     Remark QoS parameters, i.e. map in-dscp/exp, remark_profile and 
 *     header pkt type to out-dscp/exp in order to be set in outgoing
 *     packet encapsulated headers.
 *     Details: in the H file. (search for prototype)
 *    Valid only for ARAD.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key -
 *     Incomming pkt header type, remark profile and
 *     qos value (in-dscp-exp) as set by ingress
 *     pipe, or overwritten by POP functionalty see
 *     soc_ppd_eg_qos_params_php_remark_set()
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params -
 *     This mapped values, used to set the Qos fields (TOS, TC,
 *     EXP) of the outer header(forwarded header and if PHP was
 *     performed then the network header above the
 *     forwarded/poped header)
 * REMARKS:
 *   ARAD only API, error is returned if called for T20E/PetraB.
 *   - The out_qos_params used for -  Setting the Qos
 *    encapsulated header in case the encapsulation of type uniform -
 *   When tunnel encapsulation pushed to the packet and the encapsulation
 *   type is uniform, then the EXP of the pushed tunnel is
 *   taken from out_qos_params.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_eg_encap_qos_params_remark_set_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  );

uint32
  arad_pp_eg_encap_qos_params_remark_set_verify(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  );

uint32
  arad_pp_eg_encap_qos_params_remark_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_pp_eg_qos_params_remark_set_unsafe" API.
 *     Refer to "arad_pp_eg_qos_params_remark_set_unsafe" API for
 *     details.
 *   Valid only for ARAD
*********************************************************************/
uint32
  arad_pp_eg_encap_qos_params_remark_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  );

/*********************************************************************
* NAME:
 *   arad_pp_eg_qos_get_procs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of procedures of the
 *   arad_pp_api_eg_qos module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_eg_qos_get_procs_ptr(void);

/*********************************************************************
* NAME:
 *   arad_pp_eg_qos_get_errs_ptr
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get the pointer to the list of errors of the
 *   arad_pp_api_eg_qos module.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_eg_qos_get_errs_ptr(void);

uint32
  SOC_PPC_EG_QOS_MAP_KEY_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY *info
  );

uint32
  SOC_PPC_EG_ENCAP_QOS_MAP_KEY_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY *info
  );

uint32
  SOC_PPC_EG_QOS_PARAMS_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS *info
  );

uint32
  SOC_PPC_EG_ENCAP_QOS_PARAMS_verify(
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS *info
  );

uint32
  SOC_PPC_EG_QOS_PHP_REMARK_KEY_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY *info
  );

uint32
  SOC_PPC_EG_QOS_PORT_INFO_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO *info
  );

#ifdef BCM_88660
uint32
  SOC_PPC_EG_QOS_MARKING_KEY_verify(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MARKING_KEY *info
  );
uint32
  SOC_PPC_EG_QOS_MARKING_PARAMS_verify(
    SOC_SAND_IN  SOC_PPC_EG_QOS_MARKING_PARAMS *info
  );
uint32 SOC_PPC_EG_QOS_GLOBAL_INFO_verify(SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info);
#endif

uint32 arad_pp_port_eg_qos_marking_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN bcm_port_t port,
   SOC_SAND_IN int enable
  );
uint32   arad_pp_port_eg_qos_marking_get(
   SOC_SAND_IN int unit,
   SOC_SAND_IN bcm_port_t port,
   SOC_SAND_OUT int *enable
  );

uint32
  arad_pp_port_eg_ttl_inheritance_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN uint64 outlif_profiles
  );
uint32 
  arad_pp_port_eg_ttl_inheritance_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT uint64 *outlif_profiles
   );
uint32
  arad_pp_port_eg_qos_inheritance_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN uint64 outlif_profiles
  );
uint32 
  arad_pp_port_eg_qos_inheritance_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT uint64 *outlif_profiles
  );
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_EG_QOS_INCLUDED__*/
#endif



