/* $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __ARAD_PP_SLB_INCLUDED__
#define __ARAD_PP_SLB_INCLUDED__

/*************
 * INCLUDES  *
 *************/

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

#include <soc/dpp/PPC/ppc_api_slb.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_api_slb.h>


typedef struct {
  uint32 age_time_in_seconds;
} ARAD_PP_SLB_CONFIG;

uint32
  arad_pp_slb_init_unsafe(
    SOC_SAND_IN  int                                         unit,
    SOC_SAND_IN  uint8                                          enable
  );

#define ARAD_PP_SLB_ECMP_HASH_KEY_1_OFFSET_DEFAULT  23
#define ARAD_PP_SLB_ECMP_HASH_KEY_0_OFFSET_DEFAULT  24

/* Verify a SOC_PPC_SLB_OBJECT (according to object type). */
uint32 
  ARAD_PP_SLB_verify(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN SOC_PPC_SLB_OBJECT *object
  );

#define PPD_API_SLB_INTERNAL_STD_IMPL(suffix, args) ;
#define PPD_API_SLB_INTERNAL_PREFIX(suffix) arad_pp_api_impl_ ## suffix ## _verify
#define PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY

/* Add arad_pp_xxx_verify declarations. */
#include <soc/dpp/PPD/ppd_api_slb.h>

#undef PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY
#undef PPD_API_SLB_INTERNAL_PREFIX
#undef PPD_API_SLB_INTERNAL_STD_IMPL


#define PPD_API_SLB_INTERNAL_STD_IMPL(suffix, args) ;
#define PPD_API_SLB_INTERNAL_PREFIX(suffix) arad_pp_api_impl_ ## suffix ## _unsafe
#define PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY

/* Add arad_pp_xxx_unsafe declarations. */
#include <soc/dpp/PPD/ppd_api_slb.h>

#undef PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY
#undef PPD_API_SLB_INTERNAL_PREFIX
#undef PPD_API_SLB_INTERNAL_STD_IMPL

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* __ARAD_PP_SLB_INCLUDED__ */

