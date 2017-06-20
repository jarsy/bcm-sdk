/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __DNX_TMC_API_DEBUG_INCLUDED__
/* { */
#define __DNX_TMC_API_DEBUG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
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
  DNX_TMC_DBG_FORCE_MODE_NONE=0,
  /*
   *  Traffic route forced to the local route (never routed
   *  through the fabric).
   */
  DNX_TMC_DBG_FORCE_MODE_LOCAL=1,
  /*
   *  Traffic route forced to the fabric route (always routed
   *  through the fabric).
   */
  DNX_TMC_DBG_FORCE_MODE_FABRIC=2,
  /*
   *  Total number of traffic routing force modes.
   */
  DNX_TMC_DBG_NOF_FORCE_MODES=3
}DNX_TMC_DBG_FORCE_MODE;

typedef enum
{
  /*
   *  The packets in the queue are dequeued and go to their
   *  destination.
   */
  DNX_TMC_DBG_FLUSH_MODE_DEQUEUE=0,
  /*
   *  The packets in the queue are deleted.
   */
  DNX_TMC_DBG_FLUSH_MODE_DELETE=1,
  /*
   *  Total number of flushing modes.
   */
  DNX_TMC_DBG_NOF_FLUSH_MODES=2
}DNX_TMC_DBG_FLUSH_MODE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
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
}DNX_TMC_DBG_AUTOCREDIT_INFO;

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
  DNX_TMC_DBG_AUTOCREDIT_INFO_clear(
    DNX_SAND_OUT DNX_TMC_DBG_AUTOCREDIT_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_DBG_FORCE_MODE_to_string(
    DNX_SAND_IN DNX_TMC_DBG_FORCE_MODE enum_val
  );

const char*
  DNX_TMC_DBG_FLUSH_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_DBG_FLUSH_MODE enum_val
  );

void
  DNX_TMC_DBG_AUTOCREDIT_INFO_print(
    DNX_SAND_IN DNX_TMC_DBG_AUTOCREDIT_INFO *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_DEBUG_INCLUDED__*/
#endif
