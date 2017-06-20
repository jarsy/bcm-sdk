/* $Id: tmc_api_framework.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_framework.h
*
* MODULE PREFIX:  soc_tmcframework
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

#ifndef __SOC_TMC_API_FRAMEWORK_INCLUDED__
/* { */
#define __SOC_TMC_API_FRAMEWORK_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define SOC_TMC_FUNCS_OFFSET1                                                     (100|SOC_TMC_PROC_BITS)
#define SOC_TMC_TOPOLOGY_STATUS_CONNECTIVITY_PRINT                                (SOC_TMC_FUNCS_OFFSET1 +    1)

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

typedef enum SOC_TMC_ERR_LIST
{
  SOC_TMC_NO_ERR                     = SOC_SAND_OK,
  SOC_TMC_GEN_ERR                    = SOC_SAND_ERR,

  SOC_TMC_START_ERR_LIST_NUMBER      = SOC_SAND_TMC_START_ERR_NUMBER,
  SOC_TMC_REGS_FIELD_VAL_OUT_OF_RANGE_ERR,

  SOC_TMC_INPUT_OUT_OF_RANGE,
  SOC_TMC_CONFIG_ERR,

  SOC_TMC_LAST_ERR
} SOC_TMC_ERR;

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

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_FRAMEWORK_INCLUDED__*/
#endif
