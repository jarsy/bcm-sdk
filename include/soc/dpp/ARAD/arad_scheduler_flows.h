/* $Id: arad_scheduler_flows.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_SCHEDULER_FLOWS_H_INCLUDED__
/* { */
#define __ARAD_SCHEDULER_FLOWS_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_end2end_scheduler.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

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

/*********************************************************************
 *     Gets the configuration set by the
 *     "arad_sch_flow_ipf_config_mode_set_unsafe" API.
 *     Refer to "arad_sch_flow_ipf_config_mode_set_unsafe" API
 *     for details.
 *********************************************************************/
uint32
  arad_sch_flow_ipf_config_mode_set_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  );


uint32
  arad_sch_flow_ipf_config_mode_set_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_IPF_CONFIG_MODE mode
  );

uint32
  arad_sch_flow_ipf_config_mode_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_OUT ARAD_SCH_FLOW_IPF_CONFIG_MODE *mode
  );

/*****************************************************
* NAME
*   arad_sch_flow_nof_subflows_get
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Gets the number of subflows of a given flow from the device.
* INPUT:
*   SOC_SAND_IN     int             unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int             core -
*     Identifier of core on device to access.
*   SOC_SAND_IN     ARAD_SCH_FLOW_ID          base_flow_id -
*     Flow index of the base flow (the lower index of the two subflows)
*   SOC_SAND_OUT    uint32                   *nof_subflows -
*     The number of subflows (1 or 2)
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_flow_nof_subflows_get(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID          base_flow_id,
    SOC_SAND_OUT    uint32                   *nof_subflows
  );

/*****************************************************
* NAME
*   arad_sch_flow_nof_subflows_set
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Sets the number of subflows of a given flow to the device.
* INPUT:
*   SOC_SAND_IN     int             unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int             core -
*     Identifier of core on device to access.
*   SOC_SAND_IN     ARAD_SCH_FLOW_ID          base_flow_id -
*     Flow index of the base flow (the lower index of the two subflows)
*   SOC_SAND_OUT    uint32                   *nof_subflows -
*     The number of subflows (1 or 2)
*   SOC_SAND_IN     uint8                   is_odd_even -
*     TRUE if OddEven (0-1) per-1K configuration
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_flow_nof_subflows_set(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID          base_flow_id,
    SOC_SAND_IN     uint32                    nof_subflows,
    SOC_SAND_IN     uint8                   is_odd_even
  );

/*****************************************************
* NAME
*   arad_sch_flow_slow_enable_set
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Gets slow enable indicantion for a given flow from the device.
* INPUT:
*   SOC_SAND_IN     ARAD_SCH_FLOW_ID          flow_ndx -
*     Flow index. Range: 0 - 56K-1.
*   SOC_SAND_IN     uint8                   *is_slow_enabled -
*     True if the flow is slow-enabled (meaning that it can be
*     in a slow state, with maximal rate according to the
*     configured slow rate.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_flow_slow_enable_set(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID           flow_ndx,
    SOC_SAND_IN     uint8                   is_slow_enabled
  );


/*****************************************************
* NAME
*   arad_sch_flow_slow_enable_get
* TYPE:
*   PROC
* DATE:
*   11/11/2007
* FUNCTION:
*   Gets slow enable indicantion for a given flow from the device.
* INPUT:
*   SOC_SAND_IN     int             unit -
*     Identifier of device to access.
*   SOC_SAND_IN     int             core -
*     Identifier of core on device to access.
*   SOC_SAND_IN     ARAD_SCH_FLOW_ID          flow_ndx -
*     Flow index. Range: 0 - 56K-1.
*   SOC_SAND_OUT    uint8                   *is_slow_enabled -
*     True if the flow is slow-enabled (meaning that it can be
*     in a slow state, with maximal rate according to the
*     configured slow rate.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*    None.
*****************************************************/
uint32
  arad_sch_flow_slow_enable_get(
    SOC_SAND_IN     int                   unit,
    SOC_SAND_IN     int                   core,
    SOC_SAND_IN     ARAD_SCH_FLOW_ID          flow_ndx,
    SOC_SAND_OUT    uint8                   *is_slow_enabled
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_verify_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_flow_verify_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx -
*     Flow index to configure. This index must be identical to
*     the subflow index of the first (even) subflow in the
*     flow structure.
*  SOC_SAND_IN  ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_verify_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW           *flow
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Sets a scheduler flow, from a scheduling element (or
*     elements) another element . The driver writes to the
*     following tables: Scheduler Enable Memory (SEM), Shaper
*     Descriptor Memory Static(SHDS) Flow Sub-Flow (FSF) Flow
*     Descriptor Memory Static (FDMS)
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx -
*     Flow index to configure. This index must be identical to
*     the subflow index of the first (even) subflow in the
*     flow structure.
*  SOC_SAND_IN  ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   1. When OddEven is FALSE (1-3 configuration),
*     composite configuration (flow with two subflows)
*     is only applicable for aggregates - not for simple flows.
*   2. If a sub-flow is declared invalid (flow->sub_flow.is_valid == FALSE),
*      it is deleted. The only relevant configuration for such a sub-flow
*      is the sub-flow 'id' field.
*   3. The sub-flow id-s are validated by the driver according to the flow_ndx
*      and the per-1K configuration. The index of the first sub-flow must be
*      equal to flow_ndx, and the index of the second sub-flow is according to
*      the odd-even configuration.
*********************************************************************/
uint32
  arad_sch_flow_set_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  int                  core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID     flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW        *flow
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_flow_set_unsafe
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx -
*     Flow index to configure. This index must be identical to
*     the subflow index of the first (even) subflow in the
*     flow structure.
*  SOC_SAND_OUT ARAD_SCH_FLOW           *flow -
*     Flow parameters to set
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_OUT ARAD_SCH_FLOW           *flow
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_status_verify
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_flow_status_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx -
*     The flow id (0-56K) of the requested flow.
*  SOC_SAND_IN  ARAD_SCH_FLOW_STATUS    state -
*     The requested state.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_status_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW_STATUS    state
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_max_burst_save_and_set
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     See arad_sch_se_state_set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access
*  SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_id -
*     The flow id (0-56K) of the requested flow.
*  SOC_SAND_IN uint32              new_max_burst,-
*     new MAX burst we want to set in shds
*  SOC_SAND_OUT uint32             old_max_burst,-
*     old MAX burst we want to save for later use
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_max_burst_save_and_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID    flow_id,
    SOC_SAND_IN  uint32              new_max_burst,
    SOC_SAND_OUT uint32              *old_max_burst
  );

/*********************************************************************
* NAME:
*     arad_sch_flow_status_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct 18 2007
* FUNCTION:
*     Set flow state to off/on. The state of the flow will be
*     updated, unless was configured otherwise. Note: useful
*     for virtual flows, for which the flow state must be
*     explicitly set
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of device to access.
*  SOC_SAND_IN  int                 core -
*     Identifier of core on device to access.
*  SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx -
*     The flow id (0-56K) of the requested flow.
*  SOC_SAND_IN  ARAD_SCH_FLOW_STATUS    state -
*     The requested state.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_sch_flow_status_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  ARAD_SCH_FLOW_ID        flow_ndx,
    SOC_SAND_IN  ARAD_SCH_FLOW_STATUS    state
  );


/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_SCHEDULER_FLOWS_H_INCLUDED__*/
#endif


