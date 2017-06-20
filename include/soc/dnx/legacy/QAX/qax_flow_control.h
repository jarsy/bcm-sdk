
/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_QAX_FLOW_CONTROL_INCLUDED__
/* { */
#define __JER2_QAX_FLOW_CONTROL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_traffic_mgmt.h>

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
soc_error_t
  jer2_qax_fc_pfc_generic_bitmap_set(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  );
soc_error_t
  jer2_qax_fc_pfc_generic_bitmap_get(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index,
    DNX_SAND_OUT  DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  );

soc_error_t
  jer2_qax_fc_glb_rcs_mask_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_IN uint32                        glb_res_src_bitmap
    );

soc_error_t
  jer2_qax_fc_glb_rcs_mask_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_OUT uint32                       *glb_res_src_bitmap
    );
/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __JER2_ARAD_FLOW_CONTROL_INCLUDED__*/
#endif


