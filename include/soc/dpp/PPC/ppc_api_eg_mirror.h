/* $Id: ppc_api_eg_mirror.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_eg_mirror.h
*
* MODULE PREFIX:  soc_ppc_eg
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

#ifndef __SOC_PPC_API_EG_MIRROR_INCLUDED__
/* { */
#define __SOC_PPC_API_EG_MIRROR_INCLUDED__

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

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct
{
    uint32 mirror_command;  /* mirror profile index*/
    uint8 forward_strength; /* forward strength value for forwarding competition with other apps */
    uint8 mirror_strength;  /* mirror strength  value for mirroring competition with other apps */
    uint8 forward_en;       /*enable forwarding flag*/
    uint8 mirror_en;        /*enable mirroring flag*/

} dpp_outbound_mirror_config_t;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The default of the port to enable/disable mirroring
   *  Relevant only for Soc_petra
   */
  uint8 enable_mirror;
  /*
   *  Default mirroring profile. Range 0-15. (0 means no mirroring)
   *  Not relevant for Soc_petra
   */
  uint32 dflt_profile;

} SOC_PPC_EG_MIRROR_PORT_DFLT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   *  Outbound mirror enable/disable
   *  Not relevant for Soc_petra
   */
  uint8 outbound_mirror_enable;
  /*
   *  The allocated internal port for outbound mirroring.
   *  Not relevant for Soc_petra
   */
  SOC_PPC_PORT outbound_port_ndx;  

} SOC_PPC_EG_MIRROR_PORT_INFO;


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
  SOC_PPC_EG_MIRROR_PORT_DFLT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_MIRROR_PORT_DFLT_INFO *info
  );

void
  SOC_PPC_EG_MIRROR_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_EG_MIRROR_PORT_INFO *info
  );
#if SOC_PPC_DEBUG_IS_LVL1

void
  SOC_PPC_EG_MIRROR_PORT_DFLT_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_MIRROR_PORT_DFLT_INFO *info
  );

void
  SOC_PPC_EG_MIRROR_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_EG_MIRROR_PORT_INFO *info
  );


#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_EG_MIRROR_INCLUDED__*/
#endif

