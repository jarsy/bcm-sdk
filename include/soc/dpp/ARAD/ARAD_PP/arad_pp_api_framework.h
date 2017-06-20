/* $Id: arad_pp_api_framework.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_PP_API_FRAMEWORK_INCLUDED__
/* { */
#define __ARAD_PP_API_FRAMEWORK_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/SAND/Utils/sand_os_interface.h>

/*#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>*/

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
 *   arad_pp_procedure_desc_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add the pool of SOC_SAND_FAP20V procedure descriptors to the
 *   all-system sorted pool.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_procedure_desc_add(void);

/*********************************************************************
* NAME:
 *   arad_pp_errors_desc_add
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Add the pool of error descriptors to the all-system
 *   sorted pool.
 * INPUT:
 * REMARKS:
 *
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_pp_errors_desc_add(void);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PP_API_FRAMEWORK_INCLUDED__*/
#endif


