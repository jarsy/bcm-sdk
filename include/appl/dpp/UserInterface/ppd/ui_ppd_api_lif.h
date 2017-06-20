/* $Id: ui_ppd_api_lif.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#if LINK_PPD_LIBRARIES

#include <appl/dpp/UserInterface/ui_pure_defi_ppd_api.h>
#include <soc/dpp/PPD/ppd_api_general.h>

#ifdef UI_LIF/* { lif*/
/******************************************************************** 
 *  Section handler: lif
 ********************************************************************/ 
int 
  ui_ppd_api_lif( 
    CURRENT_LINE *current_line 
  );


int
ui_ppd_frwrd_decision_set(
  CURRENT_LINE *current_line,
  uint32 occur,
  SOC_PPC_FRWRD_DECISION_INFO *fwd_decision
);

#endif /* UI_LIF */
#endif /* LINK_PPD_LIBRARIES */ 

