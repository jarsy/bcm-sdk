/* $Id: arad_pp_mpls_term.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_JER_PP_LIF_INCLUDED__
/* { */
#define __SOC_JER_PP_LIF_INCLUDED__

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

/* Full documentation available in the .c file. */
soc_error_t
soc_jer_lif_glem_access_entry_add(int unit, int global_lif_index, int egress_lif_index);

/* Full documentation available in the .c file. */
soc_error_t
soc_jer_lif_glem_access_entry_remove(int unit, int global_lif_index);

/* Full documentation available in the .c file. */
soc_error_t
soc_jer_lif_glem_access_entry_by_key_get(int unit, int global_lif_id, int *egress_lif_id, uint8 *accessed, uint8 *is_found);

soc_error_t
soc_jer_lif_init(int unit);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_JER_PP_LIF_INCLUDED__*/
#endif


