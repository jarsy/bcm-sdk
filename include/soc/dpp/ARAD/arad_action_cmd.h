/* $Id: arad_action_cmd.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_ACTION_CMD_INCLUDED__
/* { */
#define __ARAD_ACTION_CMD_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_action_cmd.h>
#include <soc/dpp/ARAD/arad_framework.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_ACTION_NDX_MIN                                      (1)
#define ARAD_ACTION_NDX_MAX                                      (15)

/* 
 *     the following macro used to access the destination encoding/decoding genericly used the define constant mechanizam(tools/autocoder/DNXDEFINES)
 *     foreach destination type we have  a per chip constant in the following format:
 *     bits 0-7  the encoding value for each dest type
 *     bits 8-15  the encoding start bit position  for each  dest type
 *     bits 16-23  the encoding end bit position  for each  dest type
 *     when we encode for specific type we  do: dest_id  |   value<<start
 *     here is short description for the macros
 *     ENCODING_VALUE(dpp_const) return the value  from the const
 *     ENCODING_START(dpp_const) return the start position  from the const
 *     ENCODING_END(dpp_const) return the end position  from the const
 *     DESTINATION_ENCODING(dpp_const)  return the value shifted to it start pos
 *     DESTINATION_ENCODING_MASK(dpp_const)   return mask  from start to end
*/
#define ENCODING_VALUE(mask) ((SOC_DPP_IMP_DEFS_GET(unit,mask)) & 0xff)
#define ENCODING_START(mask) ((SOC_DPP_IMP_DEFS_GET(unit,mask)>>8) & 0xff)
#define ENCODING_END(mask) ((SOC_DPP_IMP_DEFS_GET(unit,mask))>>16)
#define DESTINATION_ENCODING(mask) (ENCODING_VALUE(mask) << ENCODING_START(mask))
#define DESTINATION_ENCODING_MASK(mask) ((1<<ENCODING_END(mask) - 1<<ENCODING_START(mask)) + 1<<ENCODING_END(mask))

#define INGRESS_DESTINATION_TYPE_QUEUE   (DESTINATION_ENCODING(mirror_snoop_destination_queue_encoding))
#define INGRESS_DESTINATION_TYPE_MULTICAST  (DESTINATION_ENCODING(mirror_snoop_destination_multicast_encoding))
#define INGRESS_DESTINATION_TYPE_SYS_PHY_PORT   (DESTINATION_ENCODING(mirror_snoop_destination_sys_phy_port_encoding))
#define INGRESS_DESTINATION_TYPE_LAG  (DESTINATION_ENCODING(mirror_snoop_destination_lag_encoding))

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
 *   arad_action_cmd_snoop_set_unsafe
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
  arad_action_cmd_snoop_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO               *info
  );

uint32
  arad_action_cmd_snoop_set_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO               *info
  );

uint32
  arad_action_cmd_snoop_get_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_action_cmd_snoop_set_unsafe" API.
 *     Refer to "arad_action_cmd_snoop_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_action_cmd_snoop_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_OUT ARAD_ACTION_CMD_SNOOP_MIRROR_INFO               *info,
    SOC_SAND_IN  SOC_TMC_CMD_TYPE                                     cmd_type
  );


uint32
dpp_snoop_mirror_stamping_config_get(
   SOC_SAND_IN  int             unit,
   SOC_SAND_IN  SOC_TMC_CMD_TYPE cmnd_type,
   SOC_SAND_IN  int              cmd,
   SOC_SAND_OUT  SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO  *info
   );

uint32
dpp_snoop_mirror_stamping_config_set(
   SOC_SAND_IN  int              unit,
   SOC_SAND_IN  SOC_TMC_CMD_TYPE cmnd_type,
   SOC_SAND_IN  int              cmd,
   SOC_SAND_IN  SOC_TMC_ACTION_CMD_SNOOP_MIRROR_STAMPING_INFO  *info
   );
/*********************************************************************
* NAME:
 *   arad_action_cmd_mirror_set_unsafe
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
  arad_action_cmd_mirror_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  SOC_TMC_CMD_TYPE                   cmnd_type,
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO              *info
  );

uint32
  arad_action_cmd_mirror_set_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO              *info
  );

uint32
  arad_action_cmd_mirror_get_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_action_cmd_mirror_set_unsafe" API.
 *     Refer to "arad_action_cmd_mirror_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_action_cmd_mirror_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              action_ndx,
    SOC_SAND_IN  SOC_TMC_CMD_TYPE                               cmnd_type,
    SOC_SAND_OUT ARAD_ACTION_CMD_SNOOP_MIRROR_INFO              *info
  );

#if ARAD_DEBUG_IS_LVL1

uint32
  ARAD_ACTION_CMD_OVERRIDE_verify(
    SOC_SAND_IN  ARAD_ACTION_CMD_OVERRIDE *info
  );

uint32
  ARAD_ACTION_CMD_verify(
    SOC_SAND_IN  ARAD_ACTION_CMD *info
  );

uint32
  ARAD_ACTION_CMD_SNOOP_INFO_verify(
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

uint32
  ARAD_ACTION_CMD_MIRROR_INFO_verify(
    SOC_SAND_IN  ARAD_ACTION_CMD_SNOOP_MIRROR_INFO *info
  );

#endif /* ARAD_DEBUG_IS_LVL1 */
/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_ACTION_CMD_INCLUDED__*/
#endif


