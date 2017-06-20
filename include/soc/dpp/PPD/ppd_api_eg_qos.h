/* $Id: ppd_api_eg_qos.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/include/soc_ppd_api_eg_qos.h
*
* MODULE PREFIX:  soc_ppd_eg
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

#ifndef __SOC_PPD_API_EG_QOS_INCLUDED__
/* { */
#define __SOC_PPD_API_EG_QOS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_eg_qos.h>

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
  SOC_PPD_EG_QOS_PORT_INFO_SET = SOC_PPD_PROC_DESC_BASE_EG_QOS_FIRST,
  SOC_PPD_EG_QOS_PORT_INFO_SET_PRINT,
  SOC_PPD_EG_QOS_PORT_INFO_GET,
  SOC_PPD_EG_QOS_PORT_INFO_GET_PRINT,
  SOC_PPD_EG_QOS_PARAMS_PHP_REMARK_SET,
  SOC_PPD_EG_QOS_PARAMS_PHP_REMARK_SET_PRINT,
  SOC_PPD_EG_QOS_PARAMS_PHP_REMARK_GET,
  SOC_PPD_EG_QOS_PARAMS_PHP_REMARK_GET_PRINT,
  SOC_PPD_EG_QOS_PARAMS_REMARK_SET,
  SOC_PPD_EG_QOS_PARAMS_REMARK_SET_PRINT,
  SOC_PPD_EG_QOS_PARAMS_REMARK_GET,
  SOC_PPD_EG_QOS_PARAMS_REMARK_GET_PRINT,
  SOC_PPD_EG_QOS_GET_PROCS_PTR,
  SOC_PPD_EG_ENCAP_QOS_PARAMS_REMARK_SET,
  SOC_PPD_EG_ENCAP_QOS_PARAMS_REMARK_SET_PRINT,
  SOC_PPD_EG_ENCAP_QOS_PARAMS_REMARK_GET,
  SOC_PPD_EG_ENCAP_QOS_PARAMS_REMARK_GET_PRINT,
  /*
   * } Auto generated. Do not edit previous section.
   */
  /*
   * Last element. Do no touch.
   */
  SOC_PPD_EG_QOS_PROCEDURE_DESC_LAST
} SOC_PPD_EG_QOS_PROCEDURE_DESC;

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
 *   soc_ppd_eg_qos_port_info_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets port information for egress QoS setting, including
 *   profiles used for QoS remarking.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx -
 *     Out Local port ID.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info -
 *     The port information for QoS remark
 * REMARKS:
 *   Soc_petra-B only API, error is returned if called for T20E.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_qos_port_info_set(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_qos_port_info_set" API.
 *     Refer to "soc_ppd_eg_qos_port_info_set" API for details.
*********************************************************************/
uint32
  soc_ppd_eg_qos_port_info_get(
    SOC_SAND_IN  int                               unit,
	SOC_SAND_IN  int                               core_id,
    SOC_SAND_IN  SOC_PPC_PORT                                local_port_ndx,
    SOC_SAND_OUT SOC_PPC_EG_QOS_PORT_INFO                    *port_qos_info
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_qos_params_php_remark_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Sets how to remark QoS parameters upon PHP operation.
 *   When uniform pop performed the dscp_exp value is
 *   remarked.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key -
 *     The key used for mapping including exp of the poped
 *     header
 *   SOC_SAND_IN  uint32                                dscp_exp -
 *     The new value for dscp_exp, this overwrites the dscp_exp
 *     value calculated by the ingress pipe. This value will be
 *     mapped and used for the QoS fileds in the transmitted
 *     packet see soc_ppd_eg_qos_params_remark_set()
 * REMARKS:
 *   - Soc_petra-B only API, error is returned if called for
 *   T20E. - When pop type is pipe then the dscp_exp value is
 *   taken from the TOS/TC/EXP field in the networking header
 *   above the popped MPLS shim - When pop into MPLS the
 *   dscp_exp value not modified
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_qos_params_php_remark_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_IN  uint32                                dscp_exp
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_qos_params_php_remark_set" API.
 *     Refer to "soc_ppd_eg_qos_params_php_remark_set" API for
 *     details.
*********************************************************************/
uint32
  soc_ppd_eg_qos_params_php_remark_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PHP_REMARK_KEY               *php_key,
    SOC_SAND_OUT uint32                                *dscp_exp
  );

#ifdef BCM_88660
/*********************************************************************
* NAME:
 *   soc_ppd_api_eg_qos_params_marking_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the mapping
 *    (TC,Resolved-DP,InLIF,Port Profile) -> (DSCP,EXP)
 *   The parameters will determine the DSCP/EXP that will be
 *   marked on the packet in egress (according to TC/DP and
 *   InLIF, port profile).
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_MARKING_KEY           *qos_key
 *     The key to the map.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_MARKING_PARAMS        *qos_params
 *     The value to set in the map.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_qos_params_marking_set(
    SOC_SAND_IN int unit,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_IN SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  );

/*********************************************************************
 *     Gets the map set by soc_ppd_api_eg_qos_params_marking_set.
 *     Refer to soc_ppd_api_eg_qos_params_marking_set for
 *     details.
 *********************************************************************/
uint32
  soc_ppd_eg_qos_params_marking_get(
    SOC_SAND_IN     int unit,
    SOC_SAND_IN     SOC_PPC_EG_QOS_MARKING_KEY *qos_key,
    SOC_SAND_OUT    SOC_PPC_EG_QOS_MARKING_PARAMS *qos_params
  );

/*********************************************************************
 * NAME:
 *   soc_ppd_api_eg_qos_global_info_set
 * TYPE:
 *   PROC
 * FUNCTION: 
 *   Set global setting for egress QoS. 
 *   Map DP to Resolved-DP and determine whether DSCP marking
 *   is enabled for an InLIF profile.
 *   If bit X in resolved_dp_bitmap is on, then dp X will have
 *   Resolved-DP 1. Otherwise it will have Resolved-DP 0.
 *   If bit X in in_lif_profile_bitmap is on then DSCP marking will
 *   be enabled for this profile.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_EG_QOS_GLOBAL_INFO *         info -
 *     Info to set.
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32 
  soc_ppd_eg_qos_global_info_set(
   SOC_SAND_IN int unit,
   SOC_SAND_IN SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );

/**
 * Gets info set by soc_ppd_api_eg_qos_global_info_set. 
 * See soc_ppd_api_eg_qos_global_info_set for details. 
 */
uint32
  soc_ppd_eg_qos_global_info_get(
   SOC_SAND_IN int unit,
   SOC_SAND_OUT SOC_PPC_EG_QOS_GLOBAL_INFO *info
  );

#endif /* BCM_88660 */

/*********************************************************************
* NAME:
 *   soc_ppd_eg_qos_params_remark_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Remark QoS parameters, i.e. map in-dscp/exp and DP to
 *   out-dscp/exp in order to be set in outgoing packet
 *   forwarding headers.
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
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
 *   Soc_petra-B only API, error is returned if called for T20E.
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
  soc_ppd_eg_qos_params_remark_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_qos_params_remark_set" API.
 *     Refer to "soc_ppd_eg_qos_params_remark_set" API for details.
*********************************************************************/
uint32
  soc_ppd_eg_qos_params_remark_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_QOS_MAP_KEY                      *in_qos_key,
    SOC_SAND_OUT SOC_PPC_EG_QOS_PARAMS                       *out_qos_params
  );

/*********************************************************************
* NAME:
 *   soc_ppd_eg_qos_encap_params_remark_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *     Remark QoS parameters, i.e. map in-dscp/exp, remark_profile and 
 *     header pkt type to out-dscp/exp in order to be set in outgoing
 *     packet forwarding headers.
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
 *   SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params -
 *     This mapped values, used to set the Qos fields (TOS, 
 *     EXP) of the outer header(forwarded header and if PHP was
 *     performed then the network header above the
 *     forwarded/poped header)
 * REMARKS:
 *   ARAD only API, error is returned if called for T20E/Soc_petraB.
 *   - The out_qos_params used for -  Setting the Qos
 *    encapsulated header in case the encapsulation of type uniform -
 *   When tunnel encapsulation pushed to the packet and the encapsulation
 *   type is uniform, then the EXP of the pushed tunnel is
 *   taken from out_qos_params.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_eg_encap_qos_params_remark_set(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  );    

/*********************************************************************
*     Gets the configuration set by the
 *     "soc_ppd_eg_encap_qos_params_remark_set" API.
 *     Refer to "soc_ppd_eg_encap_qos_params_remark_set" API for details.
 *   Valid only for ARAD.
*********************************************************************/
uint32
  soc_ppd_eg_encap_qos_params_remark_get(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  SOC_PPC_EG_ENCAP_QOS_MAP_KEY                *in_encap_qos_key,
    SOC_SAND_OUT SOC_PPC_EG_ENCAP_QOS_PARAMS                 *out_encap_qos_params
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_EG_QOS_INCLUDED__*/
#endif

