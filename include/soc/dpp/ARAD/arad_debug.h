/* $Id: arad_debug.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_DEBUG_INCLUDED__
/* { */
#define __ARAD_DEBUG_INCLUDED__


/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_debug.h>

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
*     arad_dbg_autocredit_set_unsafe
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
  arad_dbg_autocredit_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_DBG_AUTOCREDIT_INFO *info,
    SOC_SAND_OUT uint32                  *exact_rate
  );

/*********************************************************************
* NAME:
*     arad_dbg_autocredit_verify
* TYPE:
*   PROC
* FUNCTION:
*     Configure the Scheduler AutoCredit parameters.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_DBG_AUTOCREDIT_INFO *info -
*     Scheduler AutoCredit parameters.
* REMARKS:
*     Note: if first_queue > last_queue, then the AutoCredit
*     is set for all the queues.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_autocredit_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_DBG_AUTOCREDIT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_dbg_autocredit_get_unsafe
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
  arad_dbg_autocredit_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_DBG_AUTOCREDIT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_dbg_egress_shaping_enable_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Enable/disable the egress shaping.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint8                 enable -
*     If TRUE, the egress shaping is enabled. Otherwise, it is
*     disabled.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_egress_shaping_enable_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 enable
  );

/*********************************************************************
* NAME:
*     arad_dbg_egress_shaping_enable_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Enable/disable the egress shaping.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT uint8                 *enable -
*     If TRUE, the egress shaping is enabled. Otherwise, it is
*     disabled.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_egress_shaping_enable_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT uint8                 *enable
  );

/*********************************************************************
* NAME:
*     arad_dbg_flow_control_enable_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Enable/disable device-level flow control.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint8                 enable -
*     Flow control mode: enabled or disabled.
* REMARKS:
*     Note: Enable/disable device-level flow control. This
*     does not affect the flow control threshold
*     configuration, and internal per-block flow control
*     configuration (e.g. SCH, NIF). If disabled, it disables
*     inter-block flow control (e.g. EGQ to SCH, NIF to
*     EGQ/SCH, etc.). For more details on device-level flow
*     control, please refer to the user manual.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_flow_control_enable_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 enable
  );

/*********************************************************************
* NAME:
*     arad_dbg_flow_control_enable_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Enable/disable device-level flow control.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT uint8                 *enable -
*     Flow control mode: enabled or disabled.
* REMARKS:
*     Note: Enable/disable device-level flow control. This
*     does not affect the flow control threshold
*     configuration, and internal per-block flow control
*     configuration (e.g. SCH, NIF). If disabled, it disables
*     inter-block flow control (e.g. EGQ to SCH, NIF to
*     EGQ/SCH, etc.). For more details on device-level flow
*     control, please refer to the user manual.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_dbg_flow_control_enable_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT uint8                 *enable
  );

/*********************************************************************
* NAME:
*   arad_dbg_ingr_reset_unsafe
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
  arad_dbg_ingr_reset_unsafe(
    SOC_SAND_IN  int                 unit
  );

 /*********************************************************************
 * NAME:
 *   arad_dbg_dev_reset_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Soft-resets the device. As part of the reset sequence,
 *   traffic is stopped, and re-started (according to the
 *   original condition).
 * INPUT:
 *   SOC_SAND_IN    ARAD_DBG_RST_DOMAIN    rst_domain -
 *   Reset domain. Typical domain is "Full device reset".
 *   Can be also ingress-only, egress-only etc.
 * REMARKS:
 *   1. This debug function is traffic-affecting (traffic is
 *   stopped internally, all queues are emptied)
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_dbg_dev_reset_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_DBG_RST_DOMAIN      rst_domain
  );

uint32
  arad_dbg_dev_reset_verify(
    SOC_SAND_IN  ARAD_DBG_RST_DOMAIN      rst_domain
  );
  
/*********************************************************************
* NAME:
*   arad_dbg_eci_access_tst_unsafe
* TYPE:
*   PROC
* FUNCTION:
*   This function tests the ECI access to Arad.
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32                 nof_k_iters -
*     Number of iterations, in 1000 units. For example,
*     setting '100' runs 100000 iterations.
*   SOC_SAND_IN  uint8                 use_dflt_tst_reg -
*     If TRUE, the default test register is used. This
*     register inverts the data written to it. If this option
*     is selected, the following register addresses are not
*     required.
*   SOC_SAND_IN  uint32                  reg1_addr_longs -
*     Optional (only if use_dflt_tst_reg is FALSE). If not
*     using the default test register, the data is written to
*     intermittently to the two registers supplied. The
*     register address is in longs (i.e. the same way it is
*     described in the Data Sheet.)
*   SOC_SAND_IN  uint32                  reg2_addr_longs -
*     Optional (only if use_dflt_tst_reg is FALSE) If not
*     using the default test register, the data is written to
*     intermittently to the two registers supplied. The
*     register address is in longs (i.e. the same way it is
*     described in the Data Sheet.)
*   SOC_SAND_OUT uint8                 *is_valid -
*     The returned value. If TRUE, no problems in the ECI
*     access were found. Otherwise, some problems were found
*     in the ECI access.
* REMARKS:
*   If the ARAD_DBG level is set to SOC_SAND_DBG_LVL2 or above,
*   the test will print all addresses for which the ECI
*   access failed.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dbg_eci_access_tst_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 nof_k_iters,
    SOC_SAND_IN  uint8                 use_dflt_tst_reg,
    SOC_SAND_IN  uint32                  reg1_addr_longs,
    SOC_SAND_IN  uint32                  reg2_addr_longs,
    SOC_SAND_OUT uint8                 *is_valid
  );

/*********************************************************************
* NAME:
 *   arad_dbg_sch_reset_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Resets the end-to-end scheduler. The reset is performed
 *   by clearing the internal scheduler pipes, and then
 *   performing soft-reset.
 * INPUT:
 *   SOC_SAND_IN  int unit -
 *     Identifier of the device to access.
 * REMARKS:
 *   1. This debug function is traffic-affecting, on a system
 *   level, since all credit generation of the device is
 *   stopped until the internal scheduler initialization
 *   completes after reset. 2. Depricated - the functionality
 *   is covered by dbg_dev_reset API using DBG_RST_DOMAIN_SCH
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_dbg_sch_reset_unsafe(
    SOC_SAND_IN  int unit
  );
#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif
