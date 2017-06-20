/* $Id: arad_scheduler_end2end.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_END2END_SCHEDULER_INCLUDED__
/* { */
#define __ARAD_END2END_SCHEDULER_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/cosq.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_api_end2end_scheduler.h>
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
*     arad_scheduler_end2end_init
* FUNCTION:
*     Initialization of the ARAD blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  arad_scheduler_end2end_init(
    SOC_SAND_IN  int                 unit
  );

void
arad_sch_deallocate_cache_for_per1k(int unit);

/*********************************************************************
* NAME:
*     arad_scheduler_end2end_regs_init
* FUNCTION:
*     Initialization of the ARAD registers this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/

uint32
  arad_scheduler_end2end_regs_init(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     arad_sch_slow_max_rates_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_slow_max_rates_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  ARAD_SCH_SLOW_RATE      slow_max_rates -
*     The rates in Kbit/s. SOC_SAND_IN uint32 slow_max_rate2 -
*     Loaded with the actual parameters given difference due
*     to rounding.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_slow_max_rates_verify(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     arad_sch_slow_max_rates_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     This function sets the slow rates. A flow might be in
*     slow state, and in that case lower rate is needed.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  ARAD_SCH_SLOW_RATE      slow_max_rates -
*     The rates in Kbit/s. SOC_SAND_IN uint32 slow_max_rate2 -
*     Loaded with the actual parameters given difference due
*     to rounding.
*  SOC_SAND_OUT ARAD_SCH_SLOW_RATE      exact_slow_max_rates -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_slow_max_rates_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 slow_rate_type,
    SOC_SAND_IN  int                 slow_rate_val
  );

/*********************************************************************
* NAME:
*     arad_sch_slow_max_rates_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_slow_max_rates_set_unsafe
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_OUT ARAD_SCH_SLOW_RATE      *slow_max_rates -
*     The rates in Kbit/s. SOC_SAND_IN uint32 slow_max_rate2 -
*     Loaded with the actual parameters given difference due
*     to rounding.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_slow_max_rates_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 slow_rate_type,
    SOC_SAND_OUT int      *slow_rate_val
  );

/*********************************************************************
* NAME:
*     arad_sch_aggregate_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_aggregate_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx -
*     Scheduler element index.
*  SOC_SAND_IN  ARAD_SCH_SE_INFO        *se -
*     Scheduling element parameters to set.
*  SOC_SAND_IN  ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_aggregate_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx,
    SOC_SAND_IN  ARAD_SCH_SE_INFO        *se,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  );

/*********************************************************************
* NAME:
*     arad_sch_aggregate_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets an aggregate scheduler. It configures an elementary
*     scheduler, and defines a credit flow to this scheduler
*     from a 'father' scheduler. The driver writes to the
*     following tables: Scheduler Enable Memory (SEM),
*     HR-Scheduler-Configuration (SHC), CL-Schedulers
*     Configuration (SCC), Flow Group Memory (FGM) Shaper
*     Descriptor Memory (SHD) Flow Sub-Flow (FSF) Flow
*     Descriptor Memory (FDM) Shaper Descriptor Memory
*     Static(SHDS) Flow Descriptor Memory Static (FDMS)
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx -
*     Scheduler element index.
*  SOC_SAND_IN  ARAD_SCH_SE_INFO        *se -
*     Scheduling element parameters to set.
*  SOC_SAND_IN  ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_aggregate_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_SE_ID      se_ndx,
    SOC_SAND_IN  ARAD_SCH_SE_INFO        *se,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  );

/*********************************************************************
* NAME:
*     arad_sch_aggregate_group_set_unsafe
* TYPE:
*   PROC
* DATE:
*   April 04 2012
* FUNCTION:
*     Sets agroup of an aggregate scheduler. 
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx -
*     Scheduler element index.
*  SOC_SAND_IN  ARAD_SCH_SE_INFO        *se -
*     Scheduling element parameters to set.
*  SOC_SAND_IN  ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_aggregate_group_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx,
    SOC_SAND_IN  ARAD_SCH_SE_INFO        *se,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  );

/*********************************************************************
* NAME:
*     arad_sch_aggregate_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_aggregate_set_unsafe
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx -
*     Scheduler element index.
*  SOC_SAND_OUT ARAD_SCH_SE_INFO        *se -
*     Scheduling element parameters to set.
*  SOC_SAND_OUT ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_aggregate_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_SE_ID          se_ndx,
    SOC_SAND_OUT ARAD_SCH_SE_INFO        *se,
    SOC_SAND_OUT ARAD_SCH_FLOW           *flow
  );


/*********************************************************************
* NAME:
*     arad_sch_per1k_info_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_per1k_info_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  uint32                 k_flow_ndx -
*     The index of the 1K flows to configure. Range: 24 - 55.
*     Corresponds to flow-id range of 24K - 56K-1
*  SOC_SAND_IN  ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info -
*     Per 1K flows configuration. Includes the following: -
*     interdigitated - odd/even - cir/eir
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_per1k_info_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 k_flow_ndx,
    SOC_SAND_IN  ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  );

/*********************************************************************
* NAME:
*     arad_sch_per1k_info_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets configuration for 1K flows/aggregates (256
*     quartets). Flows interdigitated mode configuration must
*     match the interdigitated mode configurations of the
*     queues they are mapped to. Note1: the following flow
*     configuration is not allowed: interdigitated = TRUE,
*     odd_even = FALSE. The reason for this is that
*     interdigitated configuration defines flow-queue mapping,
*     but a flow with odd_even configuration = FALSE cannot be
*     mapped to a queue. Note2: this configuration is only
*     relevant to flow_id-s in the range 24K - 56K.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on  device to access.
*  SOC_SAND_IN  uint32                 k_flow_ndx -
*     The index of the 1K flows to configure. Range: 24 - 55.
*     Corresponds to flow-id range of 24K - 56K-1
*  SOC_SAND_IN  ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info -
*     Per 1K flows configuration. Includes the following: -
*     interdigitated - odd/even - cir/eir
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_per1k_info_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                 k_flow_ndx,
    SOC_SAND_IN  ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  );

/*********************************************************************
* NAME:
*     arad_sch_per1k_info_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_per1k_info_set_unsafe
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  uint32                 k_flow_ndx -
*     The index of the 1K flows to configure. Range: 24 - 55.
*     Corresponds to flow-id range of 24K - 56K-1
*  SOC_SAND_OUT ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info -
*     Per 1K flows configuration. Includes the following: -
*     interdigitated - odd/even - cir/eir
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   For indexes in range 24-55, the configuration is read from the device.
*   For indexes in range 0-23, per-1K configuration is irrelevant,
*   and the values of per1k_info are constant. Those values are returned
*   by the API.
* REMARKS:
*   For indexes in range 24-55, the configuration is read from the device.
*   For indexes in range 0-23, per-1K configuration is irrelevant,
*   and the values of per1k_info are constant. Those values are returned
*   by the API.
*********************************************************************/
uint32
  arad_sch_per1k_info_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                 k_flow_ndx,
    SOC_SAND_OUT ARAD_SCH_GLOBAL_PER1K_INFO *per1k_info
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_to_queue_mapping_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_flow_to_queue_mapping_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  uint32                 quartet_ndx -
*     The index of the base quartet to configure. Range: 0 -
*    16K-1. Corresponds to flow-id range of 0 - 56K -1.
*  SOC_SAND_IN  uint32                 nof_quartets_to_map -
*     The number of quartets to map. This parameter is
*     required for validation purposes only - it can be
*     derived from the interdigitated mode configuration and
*     the composite mode configuration, as following:
*     -------------------------------------------------
*    |InterDigitated | Composite | nof_quartets_to_map |
*     -------------------------------------------------
*    |      0        |     0     |         1           |
*     -------------------------------------------------
*    |      1        |     0     |         2           |
*     -------------------------------------------------
*    |      0        |     1     |         2           |
*     -------------------------------------------------
*    |      1        |     1     |         4           |
*     -------------------------------------------------
*  SOC_SAND_IN  ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info -
*     per 4 flows configuration
*     Note:
*       If quartet_flow_info->fip_id == SOC_TMC_MAX_FAP_ID then this is
*       a 'remove entry' operation.
*
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_to_queue_mapping_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                 quartet_ndx,
    SOC_SAND_IN  uint32                 nof_quartets_to_map,
    SOC_SAND_IN  ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_to_queue_mapping_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets the mapping from flow to queue and to source fap.
*     The configuration is per quartet (up to 4 quartets). The
*     mapping depends on the following parameters:
*       - interdigitated mode
*       - composite mode
*     The driver writes
*     to the following tables:
*       Flow to Queue Mapping (FQM)
*       Flow to FIP Mapping (FFM)
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  uint32                 quartet_ndx -
*     The index of the base quartet to configure. Range: 0 -
*     16K-1. Corresponds to flow-id range of 0 - 56K -1.
*  SOC_SAND_IN  uint32                 nof_quartets_to_map -
*     The number of quartets to map. This parameter is
*     required for validation purposes only - it can be
*     derived from the interdigitated mode configuration and
*     the composite mode configuration, as following:
*     -------------------------------------------------
*    |InterDigitated | Composite | nof_quartets_to_map |
*     -------------------------------------------------
*    |      0        |     0     |         1           |
*     -------------------------------------------------
*    |      1        |     0     |         2           |
*     -------------------------------------------------
*    |      0        |     1     |         2           |
*     -------------------------------------------------
*    |      1        |     1     |         4           |
*     -------------------------------------------------
*
*  SOC_SAND_IN  ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info -
*     per 4 flows configuration
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   1. A single mapping configuration covers a range of
*      four consecutive queue id-s, and up to 16 consecutive flow id-s,
*      depending on the mode. The consecutive quartets configuration
*      will be validated with previous quartets.
*      This means the quartets must be configured in order -
*      first the lower indexes, then the higher ones.
*      The configuration order must be kept:
*      - per 2 quartets if InterDigitated = 0
*      - per 4 quartets if InterDigitated = 1
*   2. The source fap configuration is per - 8 flows,
*      and not per quartet.
*      The second quartet source fap configuration
*       will be validated with the first one.
*********************************************************************/
uint32
  arad_sch_flow_to_queue_mapping_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                 quartet_ndx,
    SOC_SAND_IN  uint32                 nof_quartets_to_map,
    SOC_SAND_IN  ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_to_queue_mapping_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_flow_to_queue_mapping_set_unsafe
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of device to access.
*  SOC_SAND_IN  uint32                 quartet_ndx -
*     The index of the base quartet to configure. Range: 0 -
*    16K-1. Corresponds to flow-id range of 0 - 56K -1.
*  SOC_SAND_OUT uint32                 *nof_quartets_to_map -
*     The number of quartets to map. This parameter is
*     required for validation purposes only - it can be
*     derived from the interdigitated mode configuration and
*     the composite mode configuration, as following:
*     -------------------------------------------------
*    |InterDigitated | Composite | nof_quartets_to_map |
*     -------------------------------------------------
*    |      0        |     0     |         1           |
*     -------------------------------------------------
*    |      1        |     0     |         2           |
*     -------------------------------------------------
*    |      0        |     1     |         2           |
*     -------------------------------------------------
*    |      1        |     1     |         4           |
*     -------------------------------------------------
*  SOC_SAND_OUT ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info -
*     per 4 flows configuration
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_to_queue_mapping_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                 quartet_ndx,
    SOC_SAND_OUT ARAD_SCH_QUARTET_MAPPING_INFO *quartet_flow_info
  );

/*****************************************************
* NAME
*   arad_sch_is_flow_id_se_id
* TYPE:
*   PROC
* DATE:
*   24/10/2007
* FUNCTION:
*   Checks is the flow id is in scheduling element id-s range
* INPUT:
*   SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_id -
*     flow id, 0 - 56K-1
* RETURNS:
*   TRUE if the flow id is in scheduling element id-s range
* REMARKS:
*    None.
*****************************************************/
uint8
  arad_sch_is_flow_id_se_id(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_id
  );

/*****************************************************
* NAME
*   arad_sch_se_group_get
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Get/set flow group from the device
* INPUT:
*   SOC_SAND_IN     int               unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int               core -
*     Identifier of core on device to access.
*   SOC_SAND_IN     ARAD_SCH_SE_ID        se_ndx -
*     Scheduler element index.
*   SOC_SAND_OUT    ARAD_SCH_GROUP*  group -
*     The flow group (one of three possible)
*     the schduling element belongs to
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_se_group_get(
    SOC_SAND_IN     int               unit,
    SOC_SAND_IN     int               core,
    SOC_SAND_IN     ARAD_SCH_SE_ID        se_ndx,
    SOC_SAND_OUT    ARAD_SCH_GROUP*         group
  );


/*****************************************************
* NAME
*   arad_sch_se_group_set
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Clear from the code.
* INPUT:
*   SOC_SAND_IN     int               unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int               core -
*     Identifier of core on device to access.
*   SOC_SAND_IN     ARAD_SCH_SE_ID        se_ndx -
*     Scheduler element index.
*   SOC_SAND_OUT    ARAD_SCH_GROUP*  group -
*     The flow group (one of three possible)
*     the schduling element belongs to
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_se_group_set(
    SOC_SAND_IN     int              unit,
    SOC_SAND_IN     int              core,
    SOC_SAND_IN     ARAD_SCH_SE_ID        se_ndx,
    SOC_SAND_IN     ARAD_SCH_GROUP        group
  );

/*****************************************************
* NAME
*   arad_sch_nof_quartets_to_map_get
* TYPE:
*   PROC
* DATE:
*   14/01/2008
* FUNCTION:
* Calculate number of quartets to map according to the following table:
*
*    |InterDigitated | Composite | nof_quartets_to_map |
*     -------------------------------------------------
*    |      0        |     0     |         1           |
*     -------------------------------------------------
*    |      1        |     0     |         2           |
*     -------------------------------------------------
*    |      0        |     1     |         2           |
*     -------------------------------------------------
*    |      1        |     1     |         4           |
*     -------------------------------------------------
*
* INPUT:
*   SOC_SAND_IN  uint8                 is_interdigitated -
*     Interdigitated mode per-1k configuration
*   SOC_SAND_IN  uint8                 is_composite -
*     Composite per-quartet configuration
*   SOC_SAND_OUT uint32                 *nof_quartets_to_map -
*     Number of quartets to map, according to the table above
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_nof_quartets_to_map_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 is_interdigitated,
    SOC_SAND_IN  uint8                 is_composite,
    SOC_SAND_OUT uint32                 *nof_quartets_to_map
  );

uint32
  arad_sch_flow_delete_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID         flow_ndx
  );

uint32
  arad_sch_flow_is_deleted_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID         flow_ndx,
    SOC_SAND_OUT uint8                 *flow_is_reset
  );

uint32
  arad_flow_and_up_info_get_unsafe(
    SOC_SAND_IN     int                      unit,
    SOC_SAND_IN     int                      core,
    SOC_SAND_IN     uint32                      flow_id,
    SOC_SAND_IN     uint32                      reterive_status,
    SOC_SAND_INOUT  ARAD_SCH_FLOW_AND_UP_INFO   *flow_and_up_info
  );
/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_END2END_SCHEDULER_INCLUDED__*/
#endif


