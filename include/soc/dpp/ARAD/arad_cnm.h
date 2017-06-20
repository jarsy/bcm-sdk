/*
 * $Id: arad_cnm.h,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __ARAD_CNM_INCLUDED__
/* { */
#define __ARAD_CNM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_cnm.h>
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
 *   arad_cnm_cp_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Congestion Point functionality, in particular
 *   the packet generation mode and the fields of the CNM
 *   packet.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_CP_INFO                *info -
 *     Attributes of the congestion point functionality
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_set_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  ARAD_CNM_CP_INFO                *info
  );

uint32
  arad_cnm_cp_get_unsafe(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_OUT ARAD_CNM_CP_INFO                *info
  );

uint32
  arad_cnm_cp_set_verify(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  ARAD_CNM_CP_INFO                *info
  );


/*********************************************************************
* NAME:
 *   arad_cnm_queue_mapping_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the mapping from the pair (destination, traffic
 *   class) to the CP Queue.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_Q_MAPPING_INFO                  *info -
 *     Attributes of the CP Queue mapping
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_queue_mapping_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  ARAD_CNM_Q_MAPPING_INFO             *info
  );

uint32
  arad_cnm_queue_mapping_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT ARAD_CNM_Q_MAPPING_INFO             *info
  );

uint32
  arad_cnm_queue_mapping_set_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  ARAD_CNM_Q_MAPPING_INFO             *info
  );


/*********************************************************************
* NAME:
 *   arad_cnm_cpq_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the CP Queue attributes into an entry of the
 *   Congestion Point Table.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                              cpq_ndx -
 *     Index of the CP Queue. Range: 0 - 4K-1.
 *   SOC_SAND_IN  ARAD_CNM_CPQ_INFO                        *info -
 *     Attributes of the congestion point profile entry
 * REMARKS:
 *   Relevant only if not in a sampling mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cpq_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              cpq_ndx,
    SOC_SAND_IN  ARAD_CNM_CPQ_INFO                   *info
  );

uint32
  arad_cnm_cpq_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              cpq_ndx,
    SOC_SAND_OUT ARAD_CNM_CPQ_INFO                   *info
  );

uint32
  arad_cnm_cpq_verify(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              cpq_ndx,
    SOC_SAND_IN  ARAD_CNM_CPQ_INFO                   *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cp_profile_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure an entry into the Congestion Point Profile
 *   Table.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   profile_ndx -
 *     Index of the profile in the Congestion Point profile
 *     table. Range: 0 - 7.
 *   SOC_SAND_IN  ARAD_CNM_CP_PROFILE_INFO      *info -
 *     Attributes of the congestion point profile entry.
 * REMARKS:
 *   Relevant only if not in a sampling mode (set in
 *   cnm_cp_set API). The terminology used in this API
 *   corresponds to the 802.1 Qau standard of Congestion
 *   Notification.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_profile_set_unsafe(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                         profile_ndx,
    SOC_SAND_IN  ARAD_CNM_CP_PROFILE_INFO       *info
  );

uint32
  arad_cnm_cp_profile_get_unsafe(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                         profile_ndx,
    SOC_SAND_OUT ARAD_CNM_CP_PROFILE_INFO       *info
  );

uint32
  arad_cnm_cp_profile_verify(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                         profile_ndx,
    SOC_SAND_IN  ARAD_CNM_CP_PROFILE_INFO       *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cp_packet_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the CNM packet properties (such as FTMH, MAC SA, etc)
 *   Table.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_CP_PACKET_INFO      *info -
 *     Attributes of the congestion point profile entry.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_packet_set_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_CNM_CP_PACKET_INFO       *info
  );

uint32
  arad_cnm_cp_packet_get_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_OUT ARAD_CNM_CP_PACKET_INFO       *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cp_options_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Congestion Point generation options.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_CP_OPTIONS                *info -
 *     Options for the congestion point functionality
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_options_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  ARAD_CNM_CP_OPTIONS                 *info
  );

uint32
  arad_cnm_cp_options_get_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT ARAD_CNM_CP_OPTIONS                 *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_CNT_INCLUDED__*/
#endif
