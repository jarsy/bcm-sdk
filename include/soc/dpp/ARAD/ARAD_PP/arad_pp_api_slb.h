/* $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


#ifndef __ARAD_PP_API_SLB_INCLUDED__
#define __ARAD_PP_API_SLB_INCLUDED__

/*************
 * INCLUDES  *
 *************/

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_slb.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

#define PPD_API_SLB_INTERNAL_STD_IMPL(suffix, args) ;
#define PPD_API_SLB_INTERNAL_PREFIX(suffix) arad_pp_api_impl_ ## suffix
#define PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY

/* Add arad_pp_api declarations. */
#include <soc/dpp/PPD/ppd_api_slb.h>

#undef PPD_API_SLB_INTERNAL_FUNCTIONS_ONLY
#undef PPD_API_SLB_INTERNAL_PREFIX
#undef PPD_API_SLB_INTERNAL_STD_IMPL

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* __ARAD_PP_API_SLB_INCLUDED__ */

