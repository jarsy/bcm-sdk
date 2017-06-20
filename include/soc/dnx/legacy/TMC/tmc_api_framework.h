/* $Id: jer2_jer2_jer2_tmc_api_framework.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_framework.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmcframework
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __DNX_TMC_API_FRAMEWORK_INCLUDED__
/* { */
#define __DNX_TMC_API_FRAMEWORK_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define DNX_TMC_FUNCS_OFFSET1                                                     (100|DNX_TMC_PROC_BITS)
#define DNX_TMC_TOPOLOGY_STATUS_CONNECTIVITY_PRINT                                (DNX_TMC_FUNCS_OFFSET1 +    1)

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

typedef enum DNX_TMC_ERR_LIST
{
  DNX_TMC_NO_ERR                     = DNX_SAND_OK,
  DNX_TMC_GEN_ERR                    = DNX_SAND_ERR,

  DNX_TMC_START_ERR_LIST_NUMBER      = DNX_SAND_TMC_START_ERR_NUMBER,
  DNX_TMC_REGS_FIELD_VAL_OUT_OF_RANGE_ERR,

  DNX_TMC_INPUT_OUT_OF_RANGE,
  DNX_TMC_CONFIG_ERR,

  DNX_TMC_LAST_ERR
} DNX_TMC_ERR;

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

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_FRAMEWORK_INCLUDED__*/
#endif
