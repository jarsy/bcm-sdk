/* $Id: arad_api_debug.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_API_DEBUG_INCLUDED__
/* { */
#define __ARAD_API_DEBUG_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/TMC/tmc_api_debug.h>

#include <soc/dpp/ARAD/arad_chip_regs.h>
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
   *  Device reset - ingress pass only
   */
  ARAD_DBG_RST_DOMAIN_INGR = 0,
  /*
   *  Device reset - egress pass only
   */
  ARAD_DBG_RST_DOMAIN_EGR = 1,
  /*
   *  Device reset - scheduler only
   */
  ARAD_DBG_RST_DOMAIN_SCH = 2,
  /*
   *  Full device reset (all supported domains except fabric)
   */
  ARAD_DBG_RST_DOMAIN_FULL= 3,
    /*
   *  Device reset - ingress + fabric pass only
   */
  ARAD_DBG_RST_DOMAIN_INGR_AND_FABRIC = 4,
  /*
   *  Device reset - egress + fabricpass only
   */
  ARAD_DBG_RST_DOMAIN_EGR_AND_FABRIC = 5,
  /*
   *  Full device reset (all supported domains and fabric)
   */
  ARAD_DBG_RST_DOMAIN_FULL_AND_FABRIC = 6,
  /*
   *  Total number of device reset domains.
   */
  ARAD_DBG_NOF_RST_DOMAINS = 7
}ARAD_DBG_RST_DOMAIN;

typedef SOC_TMC_DBG_AUTOCREDIT_INFO                            ARAD_DBG_AUTOCREDIT_INFO;

/*********************************************************************
* NAME:
*     arad_dbg_autocredit_set
* TYPE:
*   PROC
* FUNCTION:
*     Configure the Scheduler AutoCredit parameters.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_DBG_AUTOCREDIT_INFO *info -
*     Scheduler AutoCredit parameters.
*  SOC_SAND_OUT uint32                  *exact_rate -
*     May vary due to rounding.
* REMARKS:
*     Note: if first_queue > last_queue, then the AutoCredit
*     is set for all the queues.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_autocredit_set(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_DBG_AUTOCREDIT_INFO *info,
    SOC_SAND_OUT uint32                  *exact_rate
  );

/*********************************************************************
* NAME:
*     arad_dbg_autocredit_get
* TYPE:
*   PROC
* FUNCTION:
*     Configure the Scheduler AutoCredit parameters.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_DBG_AUTOCREDIT_INFO *info -
*     Scheduler AutoCredit parameters.
* REMARKS:
*     Note: if first_queue > last_queue, then the AutoCredit
*     is set for all the queues.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_autocredit_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_DBG_AUTOCREDIT_INFO *info
  );

/*********************************************************************
* NAME:
*   arad_dbg_ingr_reset
* TYPE:
*   PROC
* FUNCTION:
*   Resets the ingress pass. The following blocks are
*   soft-reset (running soft-init): IPS, IQM, IPT, MMU,
*   DPRC, IRE, IHP, IDR, IRR. As part of the reset sequence,
*   traffic is stopped, and re-started (according to the
*   original condition).
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* REMARKS:
 *   1. This debug function is traffic-affecting (traffic is
 *   stopped internally, all queues are emptied)2. Depricated
 *   - the functionality is covered by dbg_dev_reset API
 *   using DBG_RST_DOMAIN_INGR
 * RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dbg_ingr_reset(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
 *   arad_dbg_dev_reset
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Soft-resets the device. As part of the reset sequence,
 *   traffic is stopped, and re-started (according to the
 *   original condition).
 * INPUT:
 *   SOC_SAND_IN  ARAD_DBG_RST_DOMAIN      rst_domain -
 *     SOC_SAND_IN ARAD_DBG_RST_DOMAIN rst_domain
 * REMARKS:
 *   1. This debug function is traffic-affecting (traffic is
 *   stopped internally, all queues are emptied)
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dbg_dev_reset(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_DBG_RST_DOMAIN      rst_domain
  );

void
  arad_ARAD_DBG_AUTOCREDIT_INFO_clear(
    SOC_SAND_OUT ARAD_DBG_AUTOCREDIT_INFO *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_DEBUG_INCLUDED__*/
#endif



