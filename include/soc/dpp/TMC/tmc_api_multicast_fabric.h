/* $Id: tmc_api_multicast_fabric.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/tmc/include/soc_tmcapi_multicast_fabric.h
*
* MODULE PREFIX:  soc_tmcmult_fabric
*
* FILE DESCRIPTION: In the Fabric-Multicast scheme,
*                   the packets/cells are replicated at the FE stage.
*                   This file holds the API functions and Structures
*                   which implement the Fabric Multicast.
*                   The file contains the standard get/set, clear and print
*                   for configuration.
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __SOC_TMC_API_MULTICAST_FABRIC_INCLUDED__
/* { */
#define __SOC_TMC_API_MULTICAST_FABRIC_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_general.h>

#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>

#include <soc/dpp/dpp_config_defs.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define SOC_TMC_MULT_FABRIC_NOF_BE_CLASSES      3

#define SOC_TMC_MULT_FABRIC_NOF_UINT32S_FOR_ACTIVE_MC_LINKS(unit) SOC_SAND_DIV_ROUND_UP(SOC_DPP_DEFS_GET(unit, nof_fabric_links),SOC_SAND_REG_SIZE_BITS)
#define SOC_TMC_MULT_FABRIC_NOF_UINT32S_FOR_ACTIVE_MC_LINKS_MAX SOC_SAND_DIV_ROUND_UP(SOC_DPP_DEFS_MAX(NOF_FABRIC_LINKS),SOC_SAND_REG_SIZE_BITS)
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

/* Soc_petra Egress Multicast Fabric Class: 0-3.    */
typedef uint32 SOC_TMC_MULT_FABRIC_CLS;

typedef enum
{
  /*
   *  Egress-Multicast-Class min: Value 0
   */
  SOC_TMC_MULT_FABRIC_CLS_MIN=0,
  /*
   *  Egress-Multicast-Class max: Value 3.
   */
  SOC_TMC_MULT_FABRIC_CLS_MAX=3,
  /*
   *  Must be the last value
   */
  SOC_TMC_MULT_FABRIC_NOF_CLS
}SOC_TMC_MULT_FABRIC_CLS_RNG;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Port Id mapped to the Multicast class in the scheduler.
   *  Range: 0 - 79.
   */
  SOC_TMC_FAP_PORT_ID mcast_class_port_id;
  /*
   *  If True, then the scheduler receives credits for the
   *  port mapped to this Multicast class. Otherwise, the
   *  credits are directly given to the queue.
   */
  uint8 multicast_class_valid;
}SOC_TMC_MULT_FABRIC_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The maximum generation credit rate for the best
   *  effort/guaranteed traffic queues. For a credit size of
   *  512 Bytes, and a core frequency of 250 MHZ, the range:
   *  Minimum - 123 Kbps, Maximum - 75 Gbps
   */
  uint32 rate;
  /*
   *  The maximum number of credits the generator can hold
   *  when credits are not needed. Range: 0 - 63.
   */
  uint32 max_burst;
}SOC_TMC_MULT_FABRIC_SHAPER_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Scheduling port configuration for a scheduling scheme
   *  (per MC class). Must be set only if the credits come to
   *  the FMQs via the scheduler.
   */
  SOC_TMC_MULT_FABRIC_PORT_INFO be_sch_port;
  /*
   *  The proportion in which credits are generated. Range: 0
   *  - 15. (Max to min).
   */
  uint32 weight;
}SOC_TMC_MULT_FABRIC_BE_CLASS_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shaper configuration of the credit generation.
   */
  SOC_TMC_MULT_FABRIC_SHAPER_INFO be_shaper;
  /*
   *  If True, then the credit will be distributed according
   *  to the weights per Multicast Class. Otherwise, they are
   *  distributed in a strict priority in the following order:
   *  MC2 > MC1 > MC0.
   */
  uint8 wfq_enable;
  /*
   *  Best effort scheduling port configuration (per MC class)
   */
  SOC_TMC_MULT_FABRIC_BE_CLASS_INFO be_sch_port[SOC_TMC_MULT_FABRIC_NOF_BE_CLASSES];
}SOC_TMC_MULT_FABRIC_BE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shaper configuration of the credit generation.
   */
  SOC_TMC_MULT_FABRIC_SHAPER_INFO gr_shaper;
  /*
   *  Scheduling port configuration for a scheduling scheme.
   *  Must be set only if the credits come to the queues via
   *  the scheduler.
   */
  SOC_TMC_MULT_FABRIC_PORT_INFO gr_sch_port;
}SOC_TMC_MULT_FABRIC_GR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Credit configuration for the guaranteed traffic (FMQ 3).
   */
  SOC_TMC_MULT_FABRIC_GR_INFO guaranteed;
  /*
   *  Credit configuration for the best effort traffic (FMQ 0
   *  - 2).
   */
  SOC_TMC_MULT_FABRIC_BE_INFO best_effort;
  /*
   *  The maximum rate in which credits are generated. Units:
   *  Kbps. For a credit size of 512 Bytes, and a core
   *  frequency of 250 MHZ, the range: Minimum - 123 Kbps,
   *  Maximum - 75 Gbps
   */
  uint32 max_rate;
  /*
   *  The maximum number of credits the generator can hold
   *  when credits are not needed. Range: 0 - 63.
   *  Valid for ARAD only.
   */
  uint32 max_burst;
  /*
   *  If True, then the credits come to the FMQs via
   *  scheduling schemes handled by the scheduler. Otherwise,
   *  the credits go directly to the FMQs and no scheme must
   *  be set in the scheduler. Must be set to True to enable
   *  the Enhanced configuration.
   */
  uint8 credits_via_sch;
}SOC_TMC_MULT_FABRIC_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  A bitmap of the device links. if bit (0 -
   *  SOC_TMC_NOF_LINKS) is up, then its corresponding link (0 -
   *  SOC_TMC_NOF_LINKS) is eligible for fabric multicast
   *  distribution. Range: 0 - 0xFFFFFFFF. (1st entry), 0 -
   *  0xF (2nd entry).
   */
  uint32 bitmap[SOC_TMC_MULT_FABRIC_NOF_UINT32S_FOR_ACTIVE_MC_LINKS_MAX];
}SOC_TMC_MULT_FABRIC_ACTIVE_LINKS;

#define SOC_TMC_MULT_FABRIC_FLOW_CONTROL_DONT_MAP 0xFFFFFFFF

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* 
   *  Which  flow control indications will stop the GFMC credit generator 
   */ 
  uint32 gfmc_lb_fc_map;

  /* 
   * Which  flow control indications will stop the BFMC0 credit generator 
   */ 
  uint32 bfmc0_lb_fc_map;

  /* 
   * Which  flow control indications will stop the BFMC1 credit generator 
   */ 
  uint32 bfmc1_lb_fc_map;

  /* 
   * Which  flow control indications will stop the BFMC2 credit generator 
   */ 
  uint32 bfmc2_lb_fc_map;


}SOC_TMC_MULT_FABRIC_FLOW_CONTROL_MAP;

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
  SOC_TMC_MULT_FABRIC_PORT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_PORT_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_SHAPER_INFO_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_SHAPER_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_BE_CLASS_INFO_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_BE_CLASS_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_BE_INFO_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_BE_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_GR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_GR_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_INFO_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_ACTIVE_LINKS_clear(
    SOC_SAND_IN uint32 unit,
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_ACTIVE_LINKS *info
  );

void
  SOC_TMC_MULT_FABRIC_FLOW_CONTROL_MAP_clear(
    SOC_SAND_OUT SOC_TMC_MULT_FABRIC_FLOW_CONTROL_MAP *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_MULT_FABRIC_CLS_RNG_to_string(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_CLS_RNG enum_val
  );

void
  SOC_TMC_MULT_FABRIC_PORT_INFO_print(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_PORT_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_SHAPER_INFO_print(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_SHAPER_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_BE_CLASS_INFO_print(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_BE_CLASS_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_BE_INFO_print(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_BE_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_GR_INFO_print(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_GR_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_INFO_print(
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_INFO *info
  );

void
  SOC_TMC_MULT_FABRIC_ACTIVE_LINKS_print(
    SOC_SAND_IN uint32 unit,
    SOC_SAND_IN SOC_TMC_MULT_FABRIC_ACTIVE_LINKS *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_MULTICAST_FABRIC_INCLUDED__*/
#endif
