
/* $Id: jer_pp_eg_encap.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER_PP_EG_FILTER_INCLUDED__
/* { */
#define __JER_PP_EG_FILTER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>

/* } */
/*************
 * DEFINES   *
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

/* 
 * Enables or disables split horizon filtering of packets sent from source_network_group_id to dest_network_group_id, 
 * according to is_egress_prune_enable 
 */ 
int 
soc_jer_pp_network_group_config_set(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN uint32 source_network_group_id,
    SOC_SAND_IN uint32 dest_network_group_id, 
    SOC_SAND_IN uint32 is_filter);

/* 
 * If split horizon filtering is enabled for packets sent from source_network_group_id to dest_network_group_id, 
 * is_egress_prune_enable will be set to 1. (otherwise 0)
 */ 
int 
soc_jer_pp_network_group_config_get(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN uint32 source_network_group_id,
    SOC_SAND_IN uint32 dest_network_group_id,
    SOC_SAND_OUT uint32 *is_filter);

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __JER_PP_EG_FILTER_INCLUDED__*/
#endif

