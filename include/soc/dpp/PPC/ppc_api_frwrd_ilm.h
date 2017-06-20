/* $Id: ppc_api_frwrd_ilm.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_ilm.h
*
* MODULE PREFIX:  soc_ppc_frwrd
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

#ifndef __SOC_PPC_API_FRWRD_ILM_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_ILM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

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

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If FALSE, then the key of the ILM includes the local
   *  port the packet came from. Otherwise it is masked.
   */
  uint8 mask_port;
  /*
   *  If FALSE, then the key of the ILM includes the incoming
   *  router interface the packet came from. Otherwise it is
   *  masked. Always masked in T20E
   */
  uint8 mask_inrif;

} SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  For labels in this range, the ILM key includes the EXP
   *  value. In order to provide QoS treatment according to
   *  the EXP bits in the MPLS header.
   */
  SOC_SAND_U32_RANGE labels_range;
  /*
   *  For labels in the range: map the EXP (from the MPLS
   *  header) to internal values. For labels out of the range,
   *  internal value is masked and set to 0. exp_map_tbl[x] =
   *  y; maps EXP x to internal value y. Note that the mapped
   *  values should be smaller than 8.
   */
  SOC_SAND_PP_MPLS_EXP exp_map_tbl[SOC_SAND_PP_NOF_BITS_IN_EXP];

} SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Incoming label.
   */
  SOC_SAND_PP_MPLS_LABEL in_label;
  /*
   *  Incoming label second.
   *  Used for Coupling.
   *  Second label after forwarding label.
   *  Invalid for Soc_petra-B.
   */
  SOC_SAND_PP_MPLS_LABEL in_label_second;
  /*
   *  Internal EXP. Relevant only for labels in the ELSP
   *  range. In this case, this is the value after mapping the
   *  header EXP with 'exp_map_tbl table'. Otherwise, this is
   *  ignored (set to zero)
   */
  SOC_SAND_PP_MPLS_EXP mapped_exp;
  /*
   *  The local port the packet enters from. Note: If by the
   *  global setting soc_ppd_frwrd_ilm_glbl_info_set, the port is
   *  masked, then this value has to be zero.
   */
  SOC_PPC_PORT in_local_port;
  /*
   *  The incoming interface the packet associated with. Note:
   *  If by the global setting soc_ppd_frwrd_ilm_glbl_info_set,
   *  the in-RIF is masked, then this value has to be
   *  zero. Always masked in T20E.
   */
  SOC_PPC_RIF_ID inrif;
  /* 
   *  FRWRD ILM Key flags
   *  See SOC_PPC_FRWRD_ILM_KEY_XXX
   */
  uint32 flags;
  /*
   *  The core of the local port the packet enters from. Note: If by the
   *  global setting soc_ppd_frwrd_ilm_glbl_info_set, the port is
   *  masked, then this value has to be zero.
   */
  uint32 in_core;
  /*
   *for VRF scoped ilm lookup
   */
  uint16 vrf;
} SOC_PPC_FRWRD_ILM_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  How to build the key to be used for the forwarding
   *  lookup.
   */
  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO key_info;
  /*
   *  Defines range of the labels as an E-LSP
   *  (Exp-Inferred-LSP) label that the QoS treatment for MPLS
   *  packet derived from the EXP bits in the MPLS header
   */
  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO elsp_info;

  /*
   * Enable short pipe mode for MPLS tunnel.
   */
  uint8 short_pipe_enable;

} SOC_PPC_FRWRD_ILM_GLBL_INFO;


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

void
  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO *info
  );

void
  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO *info
  );

void
  SOC_PPC_FRWRD_ILM_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY *info
  );

void
  SOC_PPC_FRWRD_ILM_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_GLBL_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO *info
  );

void
  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO *info
  );

void
  SOC_PPC_FRWRD_ILM_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY *info
  );

void
  SOC_PPC_FRWRD_ILM_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_ILM_INCLUDED__*/
#endif

