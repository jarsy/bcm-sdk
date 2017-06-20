
/* $Id: arad_pp_extended_p2p.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_FRWRD_EXT_P2P_INCLUDED__
/* { */
#define __ARAD_PP_FRWRD_EXT_P2P_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
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
 *   arad_pp_frwrd_extend_p2p_lem_entry_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Setting lem entry according to match info (key) and forward info (payload)
 * INPUT:
 *   SOC_SAND_IN  int                                 unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FRWRD_MATCH_INFO                *fwd_match_info -
 *     Key for LEM access
 *   SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO                *frwrd_info -
 *     Forwarding information for LEM payload 
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t
  arad_pp_frwrd_extend_p2p_lem_entry_add(int unit, 
                                         SOC_PPC_FRWRD_MATCH_INFO *fwd_match_info,
                                         SOC_PPC_FRWRD_DECISION_INFO *frwrd_info);


soc_error_t
  arad_pp_frwrd_extend_p2p_lem_entry_remove(int unit, 
                                         SOC_PPC_FRWRD_MATCH_INFO *fwd_match_info);

soc_error_t
  arad_pp_frwrd_extend_p2p_lem_entry_get(int unit, 
                                         SOC_PPC_FRWRD_MATCH_INFO *fwd_match_info,
                                         SOC_PPC_FRWRD_DECISION_INFO *frwrd_info,
                                         uint8 * found);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_FRWRD_EXT_P2P_INCLUDED__*/
#endif

