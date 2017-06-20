/* $Id: arad_stat_if.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_STAT_IF_INCLUDED__
/* { */
#define __ARAD_STAT_IF_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_stat_if.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_STAT_IF_REPORT_THRESHOLD_LINES                      15
#define ARAD_STAT_IF_REPORT_SCRUBBER_QUEUE_MIN                   ((96*1024)-1)
#define ARAD_STAT_IF_REPORT_SCRUBBER_QUEUE_MAX                   ((96*1024)-1) 
#define ARAD_STAT_IF_REPORT_THRESHOLD_IGNORED                    0xFFFFFFFF
#define ARAD_STAT_IF_REPORT_SCRUBBER_DISABLE                     0
#define ARAD_STAT_IF_REPORT_WC_ID                                7

#ifdef BCM_88650_B0
#define ARAD_STAT_IF_REPORT_QSIZE_QUEUE_MIN                   (0)
#define ARAD_STAT_IF_REPORT_QSIZE_QUEUE_MAX                   ((96*1024)-1) 
#endif

#define JER_STAT_IF_REPORT_QSIZE_QUEUE_MAX                      ((64*1024)-1)
#define QAX_STAT_IF_REPORT_QSIZE_QUEUE_MAX                      ((32*1024)-1)
#define ARAD_STAT_IF_REPORT_BILLING_NO_IDLE_PERIOD               0
#define ARAD_STAT_IF_REPORT_BILLING_MAX_IDLE_PERIOD              0x0FF
#define ARAD_STAT_IF_REPORT_QSIZE_NO_IDLE_PERIOD                 0
#define ARAD_STAT_IF_REPORT_QSIZE_MAX_IDLE_PERIOD                0x03F

#define ARAD_STAT_IF_REPORT_DESC_THRESHOLD_MAX                   0x3fffff
#define ARAD_STAT_IF_REPORT_BDB_THRESHOLD_MAX                    0x7ffff
#define ARAD_STAT_IF_REPORT_UC_DRAM_THRESHOLD_MAX                0x3fffff

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
* arad_mgmt_functional_init     
* FUNCTION:
*   Initialization of the Arad blocks configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  arad_stat_if_init(
    SOC_SAND_IN  int                 unit
  );



/*********************************************************************
* NAME:
*     arad_stat_if_info_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This function configures the working mode of the
*     statistics interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_STAT_IF_INFO        *info -
*     Statistics interface info
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_stat_if_info_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_STAT_IF_INFO        *info
  );

/*********************************************************************
* NAME:
*     arad_stat_if_info_verify
* TYPE:
*   PROC
* FUNCTION:
*     This function configures the working mode of the
*     statistics interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_STAT_IF_INFO        *info -
*     Statistics interface info
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_stat_if_info_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_STAT_IF_INFO        *info
  );

/*********************************************************************
* NAME:
*     arad_stat_if_info_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This function configures the working mode of the
*     statistics interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_STAT_IF_INFO        *info -
*     Statistics interface info
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_stat_if_info_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_STAT_IF_INFO        *info
  );

/*********************************************************************
* NAME:
*     arad_stat_if_report_info_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This function configures the format of the report sent
*     through the statistics interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_STAT_IF_REPORT_INFO *info -
*     Statistics report format info
* REMARKS:
*     For Arad-B, the API soc_pb_stat_if_report_set must be used instead.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_stat_if_report_info_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_STAT_IF_REPORT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_stat_if_report_info_verify
* TYPE:
*   PROC
* FUNCTION:
*     This function configures the format of the report sent
*     through the statistics interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_STAT_IF_REPORT_INFO *info -
*     Statistics report format info
* REMARKS:
*     For Arad-B, the API soc_pb_stat_if_report_set must be used instead.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_stat_if_report_info_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_STAT_IF_REPORT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_stat_if_report_info_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This function configures the format of the report sent
*     through the statistics interface
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_STAT_IF_REPORT_INFO *info -
*     Statistics report format info
* REMARKS:
*     For Arad-B, the API soc_pb_stat_if_report_set must be used instead.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_stat_if_report_info_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_STAT_IF_REPORT_INFO *info
  );

uint32
 arad_stat_if_rate_limit_prd_get(
     SOC_SAND_IN  int                  unit,
     SOC_SAND_IN  ARAD_STAT_IF_REPORT_INFO   *info,
     SOC_SAND_OUT uint32                   *limit_prd_val
  );


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_STAT_IF_INCLUDED__*/
#endif

