/* $Id: arad_diagnostics.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_DIAGNOSTICS_INCLUDED__
/* { */
#define __ARAD_DIAGNOSTICS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_api_diagnostics.h>
#include <soc/dpp/ARAD/arad_chip_defines.h>
#include <soc/dpp/ARAD/arad_framework.h>
#include <soc/dpp/ARAD/arad_general.h>

#include <soc/dpp/SAND/SAND_FM/sand_user_callback.h>

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
 *   arad_diag_last_packet_info_get_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Returns diagnostics information regarding the last
 *   packet: the incoming TM port and the corresponding PP
 *   port, port header processing type, packet headers and
 *   payload (first 128 Bytes). In case of TM port, the ITMH,
 *   which is part of that buffer, is parsed.
 * INPUT:
 *   SOC_SAND_IN  int                unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  int                core -
 *     Core number
 *   SOC_SAND_OUT ARAD_DIAG_LAST_PACKET_INFO *last_packet -
 *     Fields of the last packet.
 * REMARKS:
 *   1. If the packet is processed with ingress shaping, then
 *   the returned ITMH corresponds to the one of the ingress
 *   shaping2. This API does not retrieve PP-related
 *   information and does not parse PP-headers, e.g. Ethernet
 *   header in the case of Ethernet port. For this reason,
 *   this API targets mainly TM ports diagnostics. For PP
 *   diagnostics, use the PPD API 'received_packet_info_get'
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_diag_last_packet_info_get_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  int                core,
    SOC_SAND_OUT ARAD_DIAG_LAST_PACKET_INFO *last_packet
  );

uint32
  arad_diag_last_packet_info_get_verify(
    SOC_SAND_IN  int                unit
  );


/*********************************************************************
* NAME:
 *   arad_diag_sample_enable_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Enable/disable diagnostic APIs.affects only APIs with
 *   type: need_sample
 * INPUT:
 *   SOC_SAND_IN  int                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint8                               enable -
 *     TRUE: diag APIs are enabled, FALSE diag APIs are
 *     disabled.
 * REMARKS:
 *   - when enabled will affect device power consuming
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_diag_sample_enable_set_unsafe(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint8                               enable
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_diag_sample_enable_get_unsafe" API.
 *     Refer to "arad_diag_sample_enable_get_unsafe" API for details.
*********************************************************************/
uint32
  arad_diag_sample_enable_get_unsafe(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_OUT uint8                                *enable
  );

#if ARAD_DEBUG

/*********************************************************************
* NAME:
*     arad_diag_signals_dump_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     dump signals from the device for last packet
* INPUT:
*  SOC_SAND_IN  uint32   flags -
* REMARKS:
*  has to call to arad_diag_sample_enable_set_unsafe() with enable = true.
*  before calling this API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_diag_signals_dump_unsafe(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  int      core,
    SOC_SAND_IN  uint32   flags
  );

#endif /* ARAD_DEBUG */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_DIAGNOSTICS_INCLUDED__*/
#endif



