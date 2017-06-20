/* $Id: jer_pp_eg_protection.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_JER_PP_EG_PROTECTION_INCLUDED__
/* { */
#define __SOC_JER_PP_EG_PROTECTION_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>


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
soc_error_t soc_jer_pp_eg_protection_init(
   int unit);

soc_error_t soc_jer_egress_protection_state_set(
   int unit,
   uint32 protection_ndx,
   uint8 path_state);

soc_error_t soc_jer_egress_protection_state_get(
   int unit,
   uint32 protection_ndx,
   uint8 *path_state);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_JER_PP_EG_PROTECTION_INCLUDED__*/
#endif

