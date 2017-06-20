/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_TMC_API_DEBUG_INCLUDED__
/* { */
#define __SOC_TMC_API_DEBUG_INCLUDED__

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

typedef enum
{
  /*
   *  Traffic route not forced.
   */
  SOC_TMC_DBG_FORCE_MODE_NONE=0,
  /*
   *  Traffic route forced to the local route (never routed
   *  through the fabric).
   */
  SOC_TMC_DBG_FORCE_MODE_LOCAL=1,
  /*
   *  Traffic route forced to the fabric route (always routed
   *  through the fabric).
   */
  SOC_TMC_DBG_FORCE_MODE_FABRIC=2,
  /*
   *  Total number of traffic routing force modes.
   */
  SOC_TMC_DBG_NOF_FORCE_MODES=3
}SOC_TMC_DBG_FORCE_MODE;

typedef enum
{
  /*
   *  The packets in the queue are dequeued and go to their
   *  destination.
   */
  SOC_TMC_DBG_FLUSH_MODE_DEQUEUE=0,
  /*
   *  The packets in the queue are deleted.
   */
  SOC_TMC_DBG_FLUSH_MODE_DELETE=1,
  /*
   *  Total number of flushing modes.
   */
  SOC_TMC_DBG_NOF_FLUSH_MODES=2
}SOC_TMC_DBG_FLUSH_MODE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The index of the first queue for the generation of auto
   *  credits. Range: 0 - 32767. Note: if last_queue <
   *  first_queue, then all the queues are selected.
   */
  uint32 first_queue;
  /*
   *  The index of the last queue for the generation of auto
   *  credits. Range: 0 - 32767. Note: if last_queue <
   *  first_queue, then all the queues are selected.
   */
  uint32 last_queue;
  /*
   *  The rate for the generation of auto credits. Units: Mbps.
   */
  uint32 rate;
}SOC_TMC_DBG_AUTOCREDIT_INFO;

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
  SOC_TMC_DBG_AUTOCREDIT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_DBG_AUTOCREDIT_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_DBG_FORCE_MODE_to_string(
    SOC_SAND_IN SOC_TMC_DBG_FORCE_MODE enum_val
  );

const char*
  SOC_TMC_DBG_FLUSH_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_DBG_FLUSH_MODE enum_val
  );

void
  SOC_TMC_DBG_AUTOCREDIT_INFO_print(
    SOC_SAND_IN SOC_TMC_DBG_AUTOCREDIT_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_DEBUG_INCLUDED__*/
#endif
