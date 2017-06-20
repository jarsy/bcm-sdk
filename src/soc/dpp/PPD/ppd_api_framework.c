/* $Id: ppd_api_framework.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppd/src/soc_ppd_api_framework.c
*
* MODULE PREFIX:  soc_ppd_api_framework
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


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PPD

#include <shared/bsl.h>
#include <soc/dpp/drv.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/PPD/ppd_api_framework.h>

#include <soc/dpp/PPD/ppd_api_general.h>
#include <soc/dpp/PPD/ppd_api_llp_parse.h>
#include <soc/dpp/PPD/ppd_api_llp_sa_auth.h>
#include <soc/dpp/PPD/ppd_api_llp_vid_assign.h>
#include <soc/dpp/PPD/ppd_api_llp_filter.h>
#include <soc/dpp/PPD/ppd_api_llp_cos.h>
#include <soc/dpp/PPD/ppd_api_llp_trap.h>
#include <soc/dpp/PPD/ppd_api_llp_mirror.h>
#include <soc/dpp/PPD/ppd_api_mymac.h>
#include <soc/dpp/PPD/ppd_api_mpls_term.h>
#include <soc/dpp/PPD/ppd_api_vsi.h>
#include <soc/dpp/PPD/ppd_api_lif.h>
#include <soc/dpp/PPD/ppd_api_rif.h>
#include <soc/dpp/PPD/ppd_api_lif_ing_vlan_edit.h>
#include <soc/dpp/PPD/ppd_api_lif_cos.h>
#include <soc/dpp/PPD/ppd_api_lif_table.h>
#include <soc/dpp/PPD/ppd_api_frwrd_mact.h>
#include <soc/dpp/PPD/ppd_api_frwrd_mact_mgmt.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ipv4.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ipv6.h>
#include <soc/dpp/PPD/ppd_api_frwrd_ilm.h>
#include <soc/dpp/PPD/ppd_api_frwrd_bmact.h>
#include <soc/dpp/PPD/ppd_api_frwrd_trill.h>
#include <soc/dpp/PPD/ppd_api_frwrd_fec.h>
#include <soc/dpp/PPD/ppd_api_eg_filter.h>
#include <soc/dpp/PPD/ppd_api_eg_qos.h>
#include <soc/dpp/PPD/ppd_api_eg_encap.h>
#include <soc/dpp/PPD/ppd_api_eg_ac.h>
#include <soc/dpp/PPD/ppd_api_eg_vlan_edit.h>
#include <soc/dpp/PPD/ppd_api_eg_mirror.h>
#include <soc/dpp/PPD/ppd_api_port.h>
#include <soc/dpp/PPD/ppd_api_lag.h>
#include <soc/dpp/PPD/ppd_api_trap_mgmt.h>
#include <soc/dpp/PPD/ppd_api_metering.h>
#include <soc/dpp/PPD/ppd_api_fp.h>
#include <soc/dpp/PPD/ppd_api_diag.h>

#include <soc/dpp/PPD/ppd_api_oam.h>
#include <soc/dpp/PPD/ppd_api_l3_src_bind.h>
#include <soc/dpp/PPD/ppd_api_ptp.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define SOC_PPD_PROCS_DESC_VALIDATE(module, err_num) \
  do { \
  if (SOC_PPD_PROC_DESC_BASE_##module##_LAST <= SOC_PPD_##module##_PROCEDURE_DESC_LAST) \
    { \
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_ERRORS_FOUND_DUPLICATES_ERR, err_num, exit); \
    } \
  } while (0)

/* } */
/*************
 *  MACROS   *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC
  SOC_PROCEDURE_DESC_ELEMENT
    Ppd_procedure_desc_element_framework[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPD_PROCEDURE_DESC_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPD_FRAMEWORK_GET_PROCS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     soc_ppd_api_framework module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  soc_ppd_framework_get_procs_ptr(
  )
{
  return Ppd_procedure_desc_element_framework;
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

