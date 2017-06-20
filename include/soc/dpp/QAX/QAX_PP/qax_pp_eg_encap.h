
/* $Id: qax_pp_eg_encap.h,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __QAX_PP_EG_ENCAP_INCLUDED__
/* { */
#define __QAX_PP_EG_ENCAP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap.h>
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


/* Set Reserved label profile table in QAX and above. */
soc_error_t soc_qax_eg_encap_additional_label_profile_set(int unit, SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO *additional_label_profile_info, int profile_index);
/* Get Reserved label profile table information in QAX and above. */
soc_error_t soc_qax_eg_encap_additional_label_profile_get(int unit, SOC_PPC_EG_ENCAP_ADDITIONAL_LABEL_PROFILE_INFO *additional_label_profile_info, int profile_index);


/*
 * Function:
 *      soc_qax_eg_encap_header_compensation_per_cud_set/_get
 * Purpose:
 *  Set a header compensation per packet according to 5 MSBs of the CUD.
 *  This header is IN ADDITION to the header compensation per port.
 * 
 * Parameters:
 *      unit    - (IN) Unit number.
 *      cud_msb - (IN) 5 MSBs of the CUD.
 *      value   - (IN(set) or OUT(get)) Header compensation for these packets.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
uint32 soc_qax_eg_encap_header_compensation_per_cud_set(int unit, int cud_msb, int value);

uint32 soc_qax_eg_encap_header_compensation_per_cud_get(int unit, int cud_msb, int *value);


#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __QAX_PP_EG_ENCAP_INCLUDED__*/
#endif


