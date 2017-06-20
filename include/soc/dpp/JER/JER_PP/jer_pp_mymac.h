/* $Id: arad_pp_mpls_term.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_JER_PP_MYMAC_INCLUDED__
/* { */
#define __SOC_JER_PP_MYMAC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/PPC/ppc_api_mymac.h>


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
soc_jer_mymac_2nd_mymac_init(int unit);

/* Full documentation available in the .c file. */
soc_error_t 
soc_jer_mymac_protocol_group_set(int unit, uint32 protocols, uint32 group);

/* Full documentation available in the .c file. */
soc_error_t 
soc_jer_mymac_protocol_group_get_protocol_by_group(int unit, uint8 group, uint32 *protocols);

/* Full documentation available in the .c file. */
soc_error_t 
soc_jer_mymac_protocol_group_get_group_by_protocols(int unit, uint32 protocols, uint8 *group);

soc_error_t
soc_jer_mymac_vrrp_tcam_info_set(int unit,SOC_PPC_VRRP_CAM_INFO *info);

soc_error_t
soc_jer_mymac_vrrp_tcam_info_delete(int unit, uint8 cam_index);


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_JER_PP_MYMAC_INCLUDED__*/
#endif

