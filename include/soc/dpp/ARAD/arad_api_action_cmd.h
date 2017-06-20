/* $Id: arad_api_action_cmd.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_API_ACTION_CMD_INCLUDED__
/* { */
#define __ARAD_API_ACTION_CMD_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/SAND_FM/sand_pp_general.h>
#include <soc/dpp/TMC/tmc_api_action_cmd.h>

#include <soc/dpp/ARAD/arad_api_general.h>

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

#define ARAD_ACTION_CMD_SIZE_BYTES_64                        SOC_TMC_ACTION_CMD_SIZE_BYTES_64
#define ARAD_ACTION_CMD_SIZE_BYTES_128                       SOC_TMC_ACTION_CMD_SIZE_BYTES_128
#define ARAD_ACTION_CMD_SIZE_BYTES_192                       SOC_TMC_ACTION_CMD_SIZE_BYTES_192
#define ARAD_ACTION_CMD_SIZE_BYTES_ALL_PCKT                  SOC_TMC_ACTION_CMD_SIZE_BYTES_ALL_PCKT
#define ARAD_ACTION_NOF_CMD_SIZE_BYTESS                      SOC_TMC_ACTION_NOF_CMD_SIZE_BYTESS
typedef SOC_TMC_ACTION_CMD_SIZE_BYTES                          ARAD_ACTION_CMD_SIZE_BYTES;

typedef SOC_TMC_ACTION_CMD_OVERRIDE                            ARAD_ACTION_CMD_OVERRIDE;
typedef SOC_TMC_ACTION_CMD                                     ARAD_ACTION_CMD;

typedef SOC_TMC_ACTION_CMD_SNOOP_MIRROR_INFO                         ARAD_ACTION_CMD_SNOOP_MIRROR_INFO;


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
 *   arad_action_cmd_snoop_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a snoop action profile in the snoop action profile
 *   table.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                              action_ndx -
 *     Action profile index in the snoop action profile table.
 *     The action profile is relevant for packets with a snoop
 *     action field of the same value. Range: 0 - 15.
 *   SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO               *info -
 *     Snoop action profile parameters
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_action_cmd_snoop_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO               *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_action_cmd_snoop_set" API.
 *     Refer to "arad_action_cmd_snoop_set" API for details.
*********************************************************************/
uint32
  arad_action_cmd_snoop_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_OUT ARAD_ACTION_CMD_SNOOP_MIRROR_INFO               *info
  );

/*********************************************************************
* NAME:
 *   arad_action_cmd_mirror_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set a mirror action profile in the mirror action profile
 *   table.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                              action_ndx -
 *     Action profile index in the mirror action profile table.
 *     Relevant for packets with a mirror action field of the
 *     same value. Range: 0 - 15.
 *   SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO              *info -
 *     Mirror action profile parameters.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_action_cmd_mirror_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  SOC_TMC_CMD_TYPE                   cmnd_type,
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO              *info
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_action_cmd_mirror_set" API.
 *     Refer to "arad_action_cmd_mirror_set" API for details.
*********************************************************************/
uint32
  arad_action_cmd_mirror_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  SOC_TMC_CMD_TYPE                            cmnd_type,
    SOC_SAND_OUT ARAD_ACTION_CMD_SNOOP_MIRROR_INFO              *info
  );

void
  ARAD_ACTION_CMD_OVERRIDE_clear(
    SOC_SAND_OUT ARAD_ACTION_CMD_OVERRIDE *info
  );

void
  ARAD_ACTION_CMD_clear(
    SOC_SAND_OUT ARAD_ACTION_CMD *info
  );

void
  ARAD_ACTION_CMD_SNOOP_INFO_clear(
    SOC_SAND_OUT ARAD_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

void
  ARAD_ACTION_CMD_MIRROR_INFO_clear(
    SOC_SAND_OUT ARAD_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#if ARAD_DEBUG_IS_LVL1

const char*
  ARAD_ACTION_CMD_SIZE_BYTES_to_string(
    SOC_SAND_IN  ARAD_ACTION_CMD_SIZE_BYTES enum_val
  );

void
  ARAD_ACTION_CMD_OVERRIDE_print(
    SOC_SAND_IN  ARAD_ACTION_CMD_OVERRIDE *info
  );

void
  ARAD_ACTION_CMD_print(
    SOC_SAND_IN  ARAD_ACTION_CMD *info
  );

void
  ARAD_ACTION_CMD_SNOOP_INFO_print(
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

void
  ARAD_ACTION_CMD_MIRROR_INFO_print(
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_ACTION_CMD_INCLUDED__*/
#endif

