
/* $Id: qax_pp_lif.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __QAX_PP_LIF_INCLUDED__
/* { */
#define __QAX_PP_LIF_INCLUDED__


/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>


/*********************************************************************
* NAME:
 *   qax_pp_lif_default_native_ac_outlif_init
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Initialize default native ac outlifs with invalid value.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t qax_pp_lif_default_native_ac_outlif_init(int unit); 

/*********************************************************************
* NAME:
 *   qax_pp_lif_default_native_ac_outlif_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   set default native ac outlif with outlif.
 *   We support only 1 default native ac outlif 
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t qax_pp_lif_default_native_ac_outlif_set(int unit, uint32 local_out_lif_id); 

/*********************************************************************
* NAME:
 *   qax_pp_lif_default_native_ac_outlif_get
 * TYPE:
 *   PROC
 * FUNCTION:
 *   return default native ac outlif with outlif.
 *   We support only 1 default native ac outlif 
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
soc_error_t qax_pp_lif_default_native_ac_outlif_get(int unit, uint32* local_out_lif_id); 


#endif /* __QAX_PP_LIF_INCLUDED__ */


