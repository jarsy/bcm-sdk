/* $Id: tmc_api_ingress_scheduler.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_INGRESS_SCHEDULER_INCLUDED__
/* { */
#define __SOC_TMC_API_INGRESS_SCHEDULER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_general.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define SOC_TMC_ING_SCH_DONT_TOUCH                (0xFFFFFFFF)
#define SOC_TMC_ING_NUMBER_OF_HIGHEST_LATENCY_PACKETS (8)
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

/* $Id: tmc_api_ingress_scheduler.h,v 1.7 Broadcom SDK $
 *  In the ingress scheduler of type Mesh,
 *  this enumerator describes the link in the mesh
 *  topology that is being configured.
 */
typedef enum
{
  /*
   *  Local destination
   */
  SOC_TMC_ING_SCH_MESH_LOCAL=0,
  /*
   *  FAP-1 destination
   */
  SOC_TMC_ING_SCH_MESH_CON1=1,
  /*
   *  FAP-2 destination
   */
  SOC_TMC_ING_SCH_MESH_CON2=2,
  /*
   *  FAP-3 destination
   */
  SOC_TMC_ING_SCH_MESH_CON3=3,
  /*
   *  FAP-4 destination
   */
  SOC_TMC_ING_SCH_MESH_CON4=4,
  /*
   *  FAP-5 destination
   */
  SOC_TMC_ING_SCH_MESH_CON5=5,
  /*
   *  FAP-6 destination
   */
  SOC_TMC_ING_SCH_MESH_CON6=6,
  /*
   *  FAP-7 destination
   */
  SOC_TMC_ING_SCH_MESH_CON7=7,
  /*
   *  Last value of enumerator
   */
  SOC_TMC_ING_SCH_MESH_LAST=8
}SOC_TMC_ING_SCH_MESH_CONTEXTS;

#define   SOC_TMC_ING_NOF_SCH_MESH_CONTEXTSS SOC_TMC_ING_SCH_MESH_LAST

/*
 *  This structure contains two parameters that define each shaper:
 *  shaper_max_rate (maximum rate in the flow in kbps),
 *  shaper_max_burst (maximum bytes for a bursty flow).
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Maximum rate in the flow in kbps. Range- [1 : 0xFFFFFFFF]
   *  Value 0 is means context blocked
   */
  uint32 max_rate;
  /*
   *  Maximum bytes for a bursty flow. Range- [1 : 65,535]
   *  Value 0 is means context blocked.
   */
  uint32 max_burst;
  /*
   * Slow start mechanism  - available for multicast queues 
   * When a packet arrive: 
   *  First phase - the initial rate will be shaped to slow_rate_phase_0 percent for a random period of time.
   *  Second phase - the rate will be shaped to slow_rate_phase_1 percent for a random period of time.
   *  Last phase - Normal shaper
   *  
   *  Enable bit - slow_rate_enable.
   */
  int     slow_start_enable;
  uint32  slow_start_rate_phase_0;
  uint32  slow_start_rate_phase_1;

}SOC_TMC_ING_SCH_SHAPER;

/*
 * This structure defines the single mesh flow with weight,
 * shaper and identifier.
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shaper structure.
   */
  SOC_TMC_ING_SCH_SHAPER shaper;
  /*
   *  Weight, compared directly to the rest of the mesh
   *  contexts weights. Range- [1 : 63] Value 0 is means
   *  context blocked.
   */
  uint32 weight;
  /*
   *  Identifier.
   */
  SOC_TMC_ING_SCH_MESH_CONTEXTS id;
}SOC_TMC_ING_SCH_MESH_CONTEXT_INFO;

/*
 * This structure defines the entire scheduler module by
 * consisting of 3 variables: an array of single context
 * information, nof_entries (the number of entries that
 * were set at the current API call, for easier implementation),
 * total_rate_shaper (gates total read request bandwidth).
 *
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  An array of all the contexts information.
   */
  SOC_TMC_ING_SCH_MESH_CONTEXT_INFO contexts[SOC_TMC_ING_SCH_MESH_LAST];
  /*
   *  The number of entries to be set in the current API call,
   *  for easier implementation. (N/A for GET function).
   *  Range: 0 - 8.
   */
  uint32 nof_entries;
  /*
   *  Gates total read request bandwidth.
   */
  SOC_TMC_ING_SCH_SHAPER total_rate_shaper;
}SOC_TMC_ING_SCH_MESH_INFO;

/*
 * This structure contains the values of 2 weights
 * competing on a WFQ for two values.
 * This structure is used for Clos Configuration
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The 1st weight competing in the 2-flow-WFQ. Range- [1 :
   *  63] Value 0 is means context blocked.
   */
  uint32 weight1;
  /*
   *  The 2nd weight competing in the 2-flow-WFQ. Range- [1 :
   *  63] Value 0 is means context blocked.
   */
  uint32 weight2;
}SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT;

/*
 * This structure contains the 4 WFQs of the Clos Configuration:
 * fabric_hp (high priority), fabric_lp (low priority),
 * global_hp and global_lp.
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Weight1 - Weight for fabric Unicast high priority
   *  traffic. Weight2 - Weight for fabric multicast high
   *  priority traffic (GFMC). A 1st level WFQ (refer to the
   *  figure describing the ING SCH Clos configuration in
   *  Section 2.3.9 of the UM).
   */
  SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT fabric_hp;
  /*
   *  Weight1 - Weight for fabric Unicast low priority
   *  traffic. Weight2 - Weight for fabric multicast low
   *  priority traffic. A 1st level WFQ (refer to the figure
   *  describing the ING SCH Clos configuration in Section
   *  2.3.9 of the UM).
   */
  SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT fabric_lp;
  /*
   *  Weight1 - Weight for high priority traffic for local
   *  route. Weight2 - weight for fabric high priority
   *  traffic. A 2nd level WFQ (refer to the figure describing
   *  the ING SCH Clos configuration in Section 2.3.9 of the
   *  UM).
   */
  SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT global_hp;
  /*
   *  Weight1 - Weight for low priority traffic for local
   *  route. Weight2 - weight for fabric low priority traffic. A
   *  2nd level WFQ (refer to the figure describing the ING
   *  SCH Clos configuration in Section 2.3.9 of the UM).
   */
  SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT global_lp;
}SOC_TMC_ING_SCH_CLOS_WFQS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shaper of the high priority traffic for local route.
   */
  SOC_TMC_ING_SCH_SHAPER local;
  /*
   *  Shaper of the fabric unicast high priority traffic.
   */
  SOC_TMC_ING_SCH_SHAPER fabric_unicast;
  /*
   *  Shaper of the fabric multicast high priority traffic
   *  (GFMC).
   */
  SOC_TMC_ING_SCH_SHAPER fabric_multicast;

} SOC_TMC_ING_SCH_CLOS_HP_SHAPERS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shaper of the fabric unicast low priority traffic.
   */  
  SOC_TMC_ING_SCH_SHAPER fabric_unicast;
  /*
   *  Shaper of the fabric multicast low priority traffic
   *  (GFMC).
   */
  SOC_TMC_ING_SCH_SHAPER fabric_multicast;   
    
} SOC_TMC_ING_SCH_CLOS_LP_SHAPERS;

/*
 * This structure contains the 3 categorized shapers of
 * the Clos configuration:
 * 1. hp (high priority) - contains 3 shapers for all the hp flows.
 * 2. (global) local - for all local shapers.
 * 3. (global) fabric - for all fabric shapers.
 */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Shaper of the local traffic.
   */
  SOC_TMC_ING_SCH_SHAPER local;
  /*
   *  Shaper of the fabric traffic.
   */
  SOC_TMC_ING_SCH_SHAPER fabric;
  /*
   * The two low priority shapesr, for fabric unicast and 
   * multicast traffic. Not valid for Soc_petra-A
   */
  SOC_TMC_ING_SCH_CLOS_LP_SHAPERS lp;
  /*
   *  The three high priority shapers, for local hp, and for
   *  fabric unicast and multicast traffic. Not valid for
   *  Soc_petra-A.
   */
  SOC_TMC_ING_SCH_CLOS_HP_SHAPERS hp;

}SOC_TMC_ING_SCH_CLOS_SHAPERS;

/*
 * This structure contains the entire clos configuration
 * parameters in two categories : shapers and weights.
 */

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Holds the entire shapers in the clos scheduler.
   */
  SOC_TMC_ING_SCH_CLOS_SHAPERS shapers;
  /*
   *  Holds the entire weights to be compared in 4 WFQ of two
   *  levels.
   */
  SOC_TMC_ING_SCH_CLOS_WFQS weights;
}SOC_TMC_ING_SCH_CLOS_INFO;

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
  SOC_TMC_ING_SCH_SHAPER_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_SHAPER *info
  );

void
  SOC_TMC_ING_SCH_MESH_CONTEXT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_MESH_CONTEXT_INFO *info
  );

void
  SOC_TMC_ING_SCH_MESH_INFO_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_MESH_INFO *info
  );

void
  SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT *info
  );

void
  SOC_TMC_ING_SCH_CLOS_WFQS_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_WFQS *info
  );

void
  SOC_TMC_ING_SCH_CLOS_HP_SHAPERS_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_HP_SHAPERS *info
  );

void
  SOC_TMC_ING_SCH_CLOS_SHAPERS_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_SHAPERS *info
  );

void
  SOC_TMC_ING_SCH_CLOS_INFO_clear(
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_ING_SCH_MESH_CONTEXTS_to_string(
    SOC_SAND_IN SOC_TMC_ING_SCH_MESH_CONTEXTS enum_val
  );

void
  SOC_TMC_ING_SCH_SHAPER_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_SHAPER *info
  );

void
  SOC_TMC_ING_SCH_MESH_CONTEXT_INFO_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_MESH_CONTEXT_INFO *info
  );

void
  SOC_TMC_ING_SCH_MESH_INFO_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_MESH_INFO *info
  );

void
  SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_WFQ_ELEMENT *info
  );

void
  SOC_TMC_ING_SCH_CLOS_WFQS_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_WFQS *info
  );

void
  SOC_TMC_ING_SCH_CLOS_HP_SHAPERS_print(
    SOC_SAND_IN  SOC_TMC_ING_SCH_CLOS_HP_SHAPERS *info
  );

void
  SOC_TMC_ING_SCH_CLOS_SHAPERS_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_SHAPERS *info
  );

void
  SOC_TMC_ING_SCH_CLOS_INFO_print(
    SOC_SAND_IN SOC_TMC_ING_SCH_CLOS_INFO *info
  );

void
  SOC_TMC_ING_SCH_CLOS_INFO_SHAPERS_dont_touch(
    SOC_SAND_OUT SOC_TMC_ING_SCH_CLOS_INFO *info
  );

void
  SOC_TMC_ING_SCH_MESH_INFO_SHAPERS_dont_touch(
    SOC_SAND_OUT SOC_TMC_ING_SCH_MESH_INFO *info
  );
#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_INGRESS_SCHEDULER_INCLUDED__*/
#endif
