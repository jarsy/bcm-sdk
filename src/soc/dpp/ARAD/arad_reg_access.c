#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_reg_access.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_REG

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/SAND/SAND_FM/sand_mem_access.h>
#include <soc/dpp/SAND/Utils/sand_bitstream.h>
#include <soc/dpp/SAND/Management/sand_low_level.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/*
 *	A value marking invalid register in the device
 */
/* } */

/*************
 *  MACROS   *
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


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88650_A0) */
