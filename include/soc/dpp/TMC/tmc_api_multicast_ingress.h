/* $Id: tmc_api_multicast_ingress.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_multicast_ingress.h
*
* MODULE PREFIX:  soc_tmcmult_ing
*
* FILE DESCRIPTION: Ingress Multicast refers to the act of replication at
*                   the ingress.
*                   This file holds the API functions and Structures
*                   which implement the egress multicast.
*                   The file contains the standard get/set, clear and print
*                   for configuration. This file also contains dynamic
*                   configuration of multicast groups (open. close, update
*                   and more).
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_TMC_API_MULTICAST_INGRESS_INCLUDED__
/* { */
#define __SOC_TMC_API_MULTICAST_INGRESS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_general.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>
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
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  This array holds the mapping of one traffic class to
   *  another. The index of the array is the class, which is
   *  mapped to the class in the value of the array indicated
   *  by the certain index.
   */
  SOC_TMC_TR_CLS map[SOC_TMC_NOF_TRAFFIC_CLASSES];
}SOC_TMC_MULT_ING_TR_CLS_MAP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The destination queue id in which to store the
   *  replication copy of the packet. The input queues may be
   *  VOQs and/or Fabric Multicast Queues.
   */
  SOC_TMC_DEST_INFO destination;
  /*
   *  Copy-Unique-Data for replication copy of
   *  the packet. This is a local number of specific egress
   *  FAP. That is, user should first allocate the
   *  egress-cud in specific FAP, then add this value to
   *  the needed ingress-multicast groups.
   */
  uint32 cud;
}SOC_TMC_MULT_ING_ENTRY;

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
  SOC_TMC_MULT_ING_TR_CLS_MAP_clear(
    SOC_SAND_OUT SOC_TMC_MULT_ING_TR_CLS_MAP *info
  );

void
  SOC_TMC_MULT_ING_ENTRY_clear(
    SOC_SAND_OUT SOC_TMC_MULT_ING_ENTRY *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

void
  SOC_TMC_MULT_ING_TR_CLS_MAP_print(
    SOC_SAND_IN SOC_TMC_MULT_ING_TR_CLS_MAP *info
  );

void
  SOC_TMC_MULT_ING_ENTRY_print(
    SOC_SAND_IN SOC_TMC_MULT_ING_ENTRY *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_MULTICAST_INGRESS_INCLUDED__*/
#endif
