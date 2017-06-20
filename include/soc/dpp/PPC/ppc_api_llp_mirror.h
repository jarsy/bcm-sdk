/* $Id: ppc_api_llp_mirror.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_llp_mirror.h
*
* MODULE PREFIX:  soc_ppc_llp
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

#ifndef __SOC_PPC_API_LLP_MIRROR_INCLUDED__
/* { */
#define __SOC_PPC_API_LLP_MIRROR_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

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

/* miminum and maximum mirroring action indices */
#define DPP_MIRROR_ACTION_NDX_MIN 1
#define DPP_MIRROR_ACTION_NDX_MAX 15

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Default mirroring profile for tagged packets. Range
   *  0-15.
   */
  uint32 tagged_dflt;
  /*
   *  Default mirroring profile for untagged packets. Range
   *  0-15.
   */
  uint32 untagged_dflt;

  /*
   *  These indicate whether the port is mirrored
   *  for untagged traffic and for tagged default (tagged traffic other than configured VIDs)
   */
  uint8 is_tagged_dflt;
  uint8 is_untagged_only;

} SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO;


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

void
  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO_print(
    SOC_SAND_IN  SOC_PPC_LLP_MIRROR_PORT_DFLT_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_LLP_MIRROR_INCLUDED__*/
#endif

