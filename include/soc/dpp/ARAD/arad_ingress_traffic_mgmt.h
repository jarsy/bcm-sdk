/* $Id: arad_ingress_traffic_mgmt.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_INGRESS_TRAFFIC_MGMT_INCLUDED__
/* { */
#define __ARAD_INGRESS_TRAFFIC_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>


#include <soc/dpp/ARAD/arad_api_ingress_traffic_mgmt.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_ITM_HUNGRY_TH_MULTIPLIER_OFFSET  4

  /* Maximum ITM-Connection-Class: Value 31   */
#define  ARAD_ITM_QT_CC_CLS_MAX              31

  /* Maximum ITM-Credit-Class: Value 15 */
#define ARAD_ITM_IPS_QT_MAX(unit)            (SOC_DPP_DEFS_GET(unit, nof_credit_request_profiles) - 1)

  /* Maximum ITM-Credit-Class: Value 15  */
#define  ARAD_ITM_QT_CR_CLS_MAX              (ARAD_ITM_NOF_CR_DISCNT_CLS_NDXS - 1)

  /*  Maximum ITM-Rate-Class: Value 63 */
#define  ARAD_ITM_QT_RT_CLS_MAX              63

/* Maximum ITM-WRED exponent maximal value */
#define ARAD_ITM_WQ_MAX                      31

#define ARAD_MIN_SCAN_CYCLE_PERIOD_MICRO 33333 /* calculated for a deletion threshold with the max field value of 15 to be around 500ms */
#define ARAD_MIN_SCAN_CYCLE_PERIOD_MICRO_AGGRESSIVE_WD_STATUS_MSG 1000 /* calculated for a message threshold with the min field value of 1 to be around 1ms */
/* credit watchdog scan time per mode (the default one for the common FSM mode):
 * normal mode - calculated for a deletion threshold with the max field value of 15 to be around 500ms - always used when not disabled.
 * aggressive status message mode -calculated for a min message time of around 1ms - always used when not disabled.
 * common status message - The default scan time is the maximum allowed, supporting the most scanned queues.
 */
#define ARAD_MIN_SCAN_CYCLE_PERIOD_MICRO_PER_MODE { \
  ARAD_MIN_SCAN_CYCLE_PERIOD_MICRO, \
  ARAD_MIN_SCAN_CYCLE_PERIOD_MICRO, \
  ARAD_MIN_SCAN_CYCLE_PERIOD_MICRO_AGGRESSIVE_WD_STATUS_MSG, \
  ARAD_CREDIT_WATCHDOG_COMMON_MAX_SCAN_TIME_NS / 2000}

/*  Maximum Discard-DP: Value 4 */
#define  ARAD_ITM_DISCARD_DP_MAX              4

#define ARAD_ITM_GRNT_BYTES_MAX                                  (256*1024*1024)
#define ARAD_ITM_GRNT_BDS_MAX                                    (2*1024*1024)
#define ARAD_ITM_GRNT_BDS_OR_DBS_DISABLED                        0xffffff
#define ARAD_ITM_GRNT_BDS_OR_DBS_MANTISSA_BITS                   8
#define ARAD_ITM_GRNT_BDS_OR_DBS_EXPONENT_BITS                   4
#define ARAD_ITM_GLOB_RCS_DROP_EXCESS_MEM_SIZE_MAX               (128*1024*1024)

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
  ARAD_ITM_FWD_ACTION_TYPE_FWD = 0,
  ARAD_ITM_FWD_ACTION_TYPE_SNOOP = 1,
  ARAD_ITM_FWD_ACTION_TYPE_INBND_MIRR = 2,
  ARAD_ITM_FWD_ACTION_TYPE_OUTBND_MIRR = 3,
  ARAD_ITM_NOF_FWD_ACTION_TYPES = 4
}ARAD_ITM_FWD_ACTION_TYPE;

typedef struct soc_dpp_guaranteed_q_resource_s {
    soc_dpp_guaranteed_pair_t dram;
    soc_dpp_guaranteed_pair_t ocb;
} soc_dpp_guaranteed_q_resource_t;

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
*     arad_itm_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  arad_itm_init(
    SOC_SAND_IN  int  unit
  );

/*********************************************************************
* NAME:
*     arad_ingress_traffic_mgmt_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  arad_ingress_traffic_mgmt_init(
    SOC_SAND_IN  int                 unit
  );

/*********************************************************************
* NAME:
*     arad_itm_dram_buffs_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     DRAM buffers are used to store packets at the ingress.
*     This is a resource shared between Unicast,
*     Full-Multicast and Mini-Multicast packets. There are 2M
*     buffers available. This function sets the buffers share
*     dedicated for Unicast, Full-Multicast and Mini-Multicast
*     packets. This function also sets the size of a single
*     buffer. See remarks below for limitations.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_DRAM_BUFFERS_INFO *dram_buffs -
*     DRAM buffers configuration - size and distribution.
* REMARKS:
*     1. Total number of DRAM buffers for Unicast,
*     Full-Multicast and Mini-Multicast packets must not
*     exceed 2M. 2. Total number of DRAM buffers, multiplied
*     by buffer size, must not exceed total available DRAM
*     size.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_dram_buffs_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_DRAM_BUFFERS_INFO *dram_buffs
  );

/*********************************************************************
* NAME:
*     arad_itm_dram_buffs_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Apr 21 2008
* FUNCTION:
*     DRAM buffers are used to store packets at the ingress.
*     This is a resource shared between Unicast,
*     Full-Multicast and Mini-Multicast packets. There are 2M
*     buffers available. This function sets the buffers share
*     dedicated for Unicast, Full-Multicast and Mini-Multicast
*     packets. This function also sets the size of a single
*     buffer. See remarks below for limitations.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_DRAM_BUFFERS_INFO *dram_buffs -
*     DRAM buffers configuration - size and distribution.
* REMARKS:
*     1. Total number of DRAM buffers for Unicast,
*     Full-Multicast and Mini-Multicast packets must not
*     exceed 2M. 2. Total number of DRAM buffers, multiplied
*     by buffer size, must not exceed total available DRAM
*     size.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_dram_buffs_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_DRAM_BUFFERS_INFO *dram_buffs
  );

/*********************************************************************
* NAME:
*     arad_itm_glob_rcs_fc_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the Thresholds to trigger/clear the Flow Control
*     Indication. For the different kinds of general resources
*     (bds, unicast, multicast).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_GLOB_RCS_FC_TH  *info -
*     The thresholds for setting/clearing Flow Control High
*     Priority and Low Priority.
 *   SOC_SAND_OUT ARAD_ITM_GLOB_RCS_FC_TH                  *exact_info -
 *     Pointer to the exact set info.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_glob_rcs_fc_set_unsafe(
    SOC_SAND_IN   int                 unit,
    SOC_SAND_IN   ARAD_ITM_GLOB_RCS_FC_TH  *info,
    SOC_SAND_OUT  ARAD_ITM_GLOB_RCS_FC_TH  *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_glob_rcs_fc_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the Thresholds to trigger/clear the Flow Control
*     Indication. For the different kinds of general resources
*     (bds, unicast, multicast).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_GLOB_RCS_FC_TH  *info -
*     The thresholds for setting/clearing Flow Control High
*     Priority and Low Priority.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_glob_rcs_fc_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_GLOB_RCS_FC_TH  *info
  );

/*********************************************************************
* NAME:
*     arad_itm_glob_rcs_fc_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the Thresholds to trigger/clear the Flow Control
*     Indication. For the different kinds of general resources
*     (bds, unicast, multicast).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_GLOB_RCS_FC_TH  *info -
*     The thresholds for setting/clearing Flow Control High
*     Priority and Low Priority.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_glob_rcs_fc_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_GLOB_RCS_FC_TH  *info
  );

/*********************************************************************
* NAME:
*     arad_itm_glob_rcs_drop_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the Thresholds to trigger/clear the Drop mechanism,
*     in which packets are dropped if the buffers of the
*     different kinds have passed their hysteresis thresholds.
*     For the different kinds of general resources (bds,
*     unicast, full-multicast, mini-multicast).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_GLOB_RCS_DROP_TH *info -
*     The thresholds for setting/clearing the Drop mechanism.
 *   SOC_SAND_OUT ARAD_ITM_GLOB_RCS_DROP_TH                *exact_info -
 *     The exact values for the thresholds to the Drop
 *     mechanism.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_glob_rcs_drop_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_GLOB_RCS_DROP_TH *info,
    SOC_SAND_OUT ARAD_ITM_GLOB_RCS_DROP_TH *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_glob_rcs_drop_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the Thresholds to trigger/clear the Drop mechanism,
*     in which packets are dropped if the buffers of the
*     different kinds have passed their hysteresis thresholds.
*     For the different kinds of general resources (bds,
*     unicast, full-multicast, mini-multicast).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_GLOB_RCS_DROP_TH *info -
*     The thresholds for setting/clearing the Drop mechanism.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_glob_rcs_drop_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_GLOB_RCS_DROP_TH *info
  );

/*********************************************************************
* NAME:
*     arad_itm_glob_rcs_drop_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the Thresholds to trigger/clear the Drop mechanism,
*     in which packets are dropped if the buffers of the
*     different kinds have passed their hysteresis thresholds.
*     For the different kinds of general resources (bds,
*     unicast, full-multicast, mini-multicast).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_GLOB_RCS_DROP_TH *info -
*     The thresholds for setting/clearing the Drop mechanism.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_glob_rcs_drop_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_GLOB_RCS_DROP_TH *info
  );

/*********************************************************************
* NAME:
*     arad_itm_category_rngs_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Defines packet queues categories - in contiguous blocks.
*     IQM queues are divided to 4 categories in contiguous
*     blocks. Category-4 from 'category-end-3' till the last
*     queue (32K).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CATEGORY_RNGS *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_category_rngs_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_CATEGORY_RNGS *info
  );

/*********************************************************************
* NAME:
*     arad_itm_category_rngs_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Defines packet queues categories - in contiguous blocks.
*     IQM queues are divided to 4 categories in contiguous
*     blocks. Category-4 from 'category-end-3' till the last
*     queue (32K).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CATEGORY_RNGS *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_category_rngs_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_CATEGORY_RNGS *info
  );

/*********************************************************************
* NAME:
*     arad_itm_category_rngs_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Defines packet queues categories - in contiguous blocks.
*     IQM queues are divided to 4 categories in contiguous
*     blocks. Category-4 from 'category-end-3' till the last
*     queue (32K).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_CATEGORY_RNGS *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_category_rngs_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_CATEGORY_RNGS *info
  );

/*********************************************************************
* NAME:
*     arad_itm_admit_test_tmplt_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     In order to admit a packet to a queue, the packet must
*     pass the admit-test-logic. The packet belogs to some VSQs
*     out of the 4 types of VSQs. For each VSQ which the packet
*     belongs to it encounters WRED and Tail-Drop mechanisms.
*     The admit-test-template determines which, if at all, of
*     the VSQ groups and their reject mechanisms must the packet
*     consider. A test template consists of two optional combinations
*     of VSQ groups to consider (testA, testB).
*     Each queue (VOQ) is assigned with a test template.
*     Notice that in a queue, is a packet is chosen to be rejected
*     normally, the admit test logic will not affect it.
*     From the Data Sheet:
*     The Packet Queue Rate Class is used to select one of four
*     Admission Logic Templates. Each template is an 8-bit variable
*     {a1,b1,c1,d1,a2,b2,c2,d2} applied as detailed below:
*
*     Final-Admit =
*       GL-Admit & PQ-Admit &
*       ((a1 | CT-Admit) & (b1 | CTTC-Admit) &
*             (c1 | CTCC-Admit) & (d1 |STF-Admit ) OR
*         (a2 | CT-Admit) & (b2 | CTTC-Admit)  &
*             (c2 | CTCC-Admit) & (d2 |STF-Admit)) &
*       (!PQ-Sys-Red-Ena | SR-Admit)
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 admt_tst_ndx -
*     There are 4 possible VSQ admission tests. Range 0 to 3.
*     With this procedure, one can set the tests.
*  SOC_SAND_IN  ARAD_ITM_ADMIT_TEST_TMPLT_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_admit_test_tmplt_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 admt_tst_ndx,
    SOC_SAND_IN  ARAD_ITM_ADMIT_TEST_TMPLT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_admit_test_tmplt_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     In order to admit a packet to a queue, the packet must
*     pass the admit-test-logic. The packet belogs to some VSQs
*     out of the 4 types of VSQs. For each VSQ which the packet
*     belongs to it encounters WRED and Tail-Drop mechanisms.
*     The admit-test-template determines which, if at all, of
*     the VSQ groups and their reject mechanisms must the packet
*     consider. A test template consists of two optional combinations
*     of VSQ groups to consider (testA, testB).
*     Each queue (VOQ) is assigned with a test template.
*     Notice that in a queue, is a packet is chosen to be rejected
*     normally, the admit test logic will not affect it.
*     From the Data Sheet:
*     The Packet Queue Rate Class is used to select one of four
*     Admission Logic Templates. Each template is an 8-bit variable
*     {a1,b1,c1,d1,a2,b2,c2,d2} applied as detailed below:
*
*     Final-Admit =
*       GL-Admit & PQ-Admit &
*       ((a1 | CT-Admit) & (b1 | CTTC-Admit) &
*             (c1 | CTCC-Admit) & (d1 |STF-Admit ) OR
*         (a2 | CT-Admit) & (b2 | CTTC-Admit)  &
*             (c2 | CTCC-Admit) & (d2 |STF-Admit)) &
*       (!PQ-Sys-Red-Ena | SR-Admit)
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 admt_tst_ndx -
*     There are 4 possible VSQ admission tests. Range 0 to 3.
*     With this procedure, one can set the tests.
*  SOC_SAND_IN  ARAD_ITM_ADMIT_TEST_TMPLT_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_admit_test_tmplt_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 admt_tst_ndx,
    SOC_SAND_IN  ARAD_ITM_ADMIT_TEST_TMPLT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_admit_test_tmplt_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     In order to admit a packet to a queue, the packet must
*     pass the admit-test-logic. The packet belogs to some VSQs
*     out of the 4 types of VSQs. For each VSQ which the packet
*     belongs to it encounters WRED and Tail-Drop mechanisms.
*     The admit-test-template determines which, if at all, of
*     the VSQ groups and their reject mechanisms must the packet
*     consider. A test template consists of two optional combinations
*     of VSQ groups to consider (testA, testB).
*     Each queue (VOQ) is assigned with a test template.
*     Notice that in a queue, is a packet is chosen to be rejected
*     normally, the admit test logic will not affect it.
*     From the Data Sheet:
*     The Packet Queue Rate Class is used to select one of four
*     Admission Logic Templates. Each template is an 8-bit variable
*     {a1,b1,c1,d1,a2,b2,c2,d2} applied as detailed below:
*
*     Final-Admit =
*       GL-Admit & PQ-Admit &
*       ((a1 | CT-Admit) & (b1 | CTTC-Admit) &
*             (c1 | CTCC-Admit) & (d1 |STF-Admit ) OR
*         (a2 | CT-Admit) & (b2 | CTTC-Admit)  &
*             (c2 | CTCC-Admit) & (d2 |STF-Admit)) &
*       (!PQ-Sys-Red-Ena | SR-Admit)
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 admt_tst_ndx -
*     There are 4 possible VSQ admission tests. Range 0 to 3.
*     With this procedure, one can set the tests.
*  SOC_SAND_OUT ARAD_ITM_ADMIT_TEST_TMPLT_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_admit_test_tmplt_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 admt_tst_ndx,
    SOC_SAND_OUT ARAD_ITM_ADMIT_TEST_TMPLT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_request_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Per queue the device maintains an Off/Normal/Slow Credit
*     Request State. The device has 16 'Credit Request
*     Configurations', one per Credit-Class. Sets the (1)
*     Queue-Size-Thresholds (2) Credit-Balance-Thresholds (3)
*     Empty-Queue-Thresholds (4) Credit-Watchdog
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_QT_NDX          qt_ndx -
*     Ingress Queue Type IPS. Range: 0 to 15.
*  SOC_SAND_IN  ARAD_ITM_CR_REQUEST_INFO *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_CR_REQUEST_INFO *exact_info -
*     pointer to configurations structure. Will be filled with
*     exact values.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_request_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_QT_NDX          qt_ndx,
    SOC_SAND_IN  ARAD_ITM_CR_REQUEST_INFO *info,
    SOC_SAND_OUT ARAD_ITM_CR_REQUEST_INFO *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_request_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Per queue the device maintains an Off/Normal/Slow Credit
*     Request State. The device has 16 'Credit Request
*     Configurations', one per Credit-Class. Sets the (1)
*     Queue-Size-Thresholds (2) Credit-Balance-Thresholds (3)
*     Empty-Queue-Thresholds (4) Credit-Watchdog
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_QT_NDX          qt_ndx -
*     Ingress Queue Type IPS. Range: 0 to 15.
*  SOC_SAND_IN  ARAD_ITM_CR_REQUEST_INFO *info -
*     pointer to infoiguration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_request_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_QT_NDX          qt_ndx,
    SOC_SAND_IN  ARAD_ITM_CR_REQUEST_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_request_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Per queue the device maintains an Off/Normal/Slow Credit
*     Request State. The device has 16 'Credit Request
*     Configurations', one per Credit-Class. Sets the (1)
*     Queue-Size-Thresholds (2) Credit-Balance-Thresholds (3)
*     Empty-Queue-Thresholds (4) Credit-Watchdog
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_QT_NDX          qt_ndx -
*     Ingress Queue Type IPS. Range: 0 to 15.
*  SOC_SAND_OUT ARAD_ITM_CR_REQUEST_INFO *info -
*     pointer to infoiguration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_request_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_QT_NDX          qt_ndx,
    SOC_SAND_OUT ARAD_ITM_CR_REQUEST_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_discount_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     The device has 16 possible credit-discount
*     possibilities. This procedure sets the 16 options.
*     The Credit Discount value should be calculated as following:
*     Credit-Discount =
*     -(IPG (20B)+ CRC (size of CRC field only if it is not removed by NP)) +
*     Dune_H (size of FTMH + FTMH extension (if exists)) +
*     NP_H (size of Network Processor Header, or Dune PP Header) + DRAM CRC size.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CR_DISCNT_CLS_NDX cr_cls_ndx -
*     Per queue user can select 1 credit-discount class out of
*     16 different credit-discount classes.
*  SOC_SAND_IN  ARAD_ITM_CR_DISCOUNT_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_discount_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_CR_DISCNT_CLS_NDX cr_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_discount_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     There are 16 possible credit-discount classes.
*     Each Credit Class is configured with a value that
*     is to be added/subtracted from the credit counter at each
*     dequeue of packet. This procedure sets the 16
*     credit-discount values per credit class.
*     The Credit Discount value should be calculated as following:
*     Credit-Discount =
*     -IPG (20B)+ CRC (size of CRC field only if it is not removed by NP) +
*     NP_H (size of Network Processor Header) + Dune_H (size of ITMH+FTMH).
*     Note that this functionality will take affect only when working with
*     small packet sizes.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CR_DISCNT_CLS_NDX cr_cls_ndx -
*     Per queue user can select 1 credit-discount class out of
*     16 different credit-discount classes.
*  SOC_SAND_IN  ARAD_ITM_CR_DISCOUNT_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_discount_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_CR_DISCNT_CLS_NDX cr_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_discount_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     There are 16 possible credit-discount classes.
*     Each Credit Class is configured with a value that
*     is to be added/subtracted from the credit counter at each
*     dequeue of packet. This procedure sets the 16
*     credit-discount values per credit class.
*     The Credit Discount value should be calculated as following:
*     Credit-Discount =
*     -(IPG (20B)+ CRC (size of CRC field only if it is not removed by NP)) +
*     Dune_H (size of FTMH + FTMH extension (if exists)) +
*     NP_H (size of Network Processor Header, or Dune PP Header) + DRAM CRC size.
*     Note that this functionality will take affect only when working with
*     small packet sizes.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CR_DISCNT_CLS_NDX cr_cls_ndx -
*     Per queue user can select 1 credit-discount class out of
*     16 different credit-discount classes.
*  SOC_SAND_OUT ARAD_ITM_CR_DISCOUNT_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_discount_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_CR_DISCNT_CLS_NDX cr_cls_ndx,
    SOC_SAND_OUT ARAD_ITM_CR_DISCOUNT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_queue_test_tmplt_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Each queue (VOQ) is assigned with a test template.
*     This Function sets the admit logic test of the queue
*     per rate-class and drop-precedence (there are 4
*     pre-configured by 'arad_itm_admit_test_tmplt_set'
*     options for test types).
*     Notice that in a queue, is a packet is chosen to be
*     rejected normally, the admit test logic will not affect it.
*     For more information about the admit test template refer to
*     the description of 'arad_itm_admit_test_tmplt_set'.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3.
*  SOC_SAND_IN  ARAD_ITM_ADMIT_TSTS      test_tmplt -
*     Enumerator indicating the test-template index.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_test_tmplt_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_ADMIT_TSTS      test_tmplt
  );

/*********************************************************************
* NAME:
*     arad_itm_queue_test_tmplt_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Each queue (VOQ) is assigned with a test template.
*     This Function sets the admit logic test of the queue
*     per rate-class and drop-precedence (there are 4
*     pre-configured by 'arad_itm_admit_test_tmplt_set'
*     options for test types).
*     Notice that in a queue, is a packet is chosen to be
*     rejected normally, the admit test logic will not affect it.
*     For more information about the admit test template refer to
*     the description of 'arad_itm_admit_test_tmplt_set'.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3.
*  SOC_SAND_IN  ARAD_ITM_ADMIT_TSTS      test_tmplt -
*     Enumerator indicating the test-template index.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_test_tmplt_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_ADMIT_TSTS      test_tmplt
  );

/*********************************************************************
* NAME:
*     arad_itm_queue_test_tmplt_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Each queue (VOQ) is assigned with a test template.
*     This Function sets the admit logic test of the queue
*     per rate-class and drop-precedence (there are 4
*     pre-configured by 'arad_itm_admit_test_tmplt_set'
*     options for test types).
*     Notice that in a queue, is a packet is chosen to be
*     rejected normally, the admit test logic will not affect it.
*     For more information about the admit test template refer to
*     the description of 'arad_itm_admit_test_tmplt_set'.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3.
*  SOC_SAND_OUT ARAD_ITM_ADMIT_TSTS      *test_tmplt -
*     Enumerator indicating the test-template index.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_test_tmplt_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_ADMIT_TSTS      *test_tmplt
  );

/*********************************************************************
* NAME:
*     arad_itm_wred_exp_wq_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets exponential weighted queue per rate-class. The
*     calculation the device does: Average_constant =
*     2^(-RED_exp_weight)if (Instantaneous-Queue-size <
*     Average-queue-size) Average-queue-size =
*     Instantaneous-Queue-sizeelse Average-queue-size =
*     (1-Average_constant)*Average-queue-size +
*     Average_constant*Instantaneous-Queue-size To configure
*     WRED configuration which are per queue-type and dp, use
*     the functionarad_itm_wred_info_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                  exp_wq -
*     Constant for average queue size calculation. Range:
*     0-31. I.e., make the average factor from 1 to 2^(-31).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_wred_exp_wq_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                  exp_wq,
    SOC_SAND_IN  uint8                   enable
  );

/*********************************************************************
* NAME:
*     arad_itm_wred_exp_wq_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets exponential weighted queue per rate-class. The
*     calculation the device does: Average_constant =
*     2^(-RED_exp_weight)if (Instantaneous-Queue-size <
*     Average-queue-size) Average-queue-size =
*     Instantaneous-Queue-sizeelse Average-queue-size =
*     (1-Average_constant)*Average-queue-size +
*     Average_constant*Instantaneous-Queue-size To configure
*     WRED configuration which are per queue-type and dp, use
*     the functionarad_itm_wred_info_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                  exp_wq -
*     Constant for average queue size calculation. Range:
*     0-31. I.e., make the average factor from 1 to 2^(-31).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_wred_exp_wq_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                  exp_wq
  );

/*********************************************************************
* NAME:
*     arad_itm_wred_exp_wq_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets exponential weighted queue per rate-class. The
*     calculation the device does: Average_constant =
*     2^(-RED_exp_weight)if (Instantaneous-Queue-size <
*     Average-queue-size) Average-queue-size =
*     Instantaneous-Queue-sizeelse Average-queue-size =
*     (1-Average_constant)*Average-queue-size +
*     Average_constant*Instantaneous-Queue-size To configure
*     WRED configuration which are per queue-type and dp, use
*     the functionarad_itm_wred_info_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_OUT uint32                  *exp_wq -
*     Constant for average queue size calculation. Range:
*     0-31. I.e., make the average factor from 1 to 2^(-31).
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_wred_exp_wq_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_OUT uint32                  *exp_wq
  );

/*********************************************************************
* NAME:
*     arad_itm_wred_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     arad_itm_wred_exp_wq_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3, or SOC_TMC_NOF_DROP_PRECEDENCE which means ECN and not a real drop precedence.
*  SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *exact_info -
*     Loaded with the actual WRED parameters (difference due
*     to rounding). Note: if 'min_avrg_th' & 'max_avrg_th'
*     fields are the same, no real WRED is activates. Since
*     below all are admitted and above all are discarded
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_wred_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info,
    SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_wred_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     arad_itm_wred_exp_wq_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3, or SOC_TMC_NOF_DROP_PRECEDENCE which means ECN and not a real drop precedence.
*  SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_wred_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_wred_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets WRED parameters per rate-class and drop precedence,
*     including wred-enable and the admit test logic. Also, as
*     part of the WRED parameters; max-queue,
*     WRED-thresholds/probability. To configure WRED
*     Configuration that is per queue-type only (exponential
*     weight queue), use the function
*     arad_itm_wred_exp_wq_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3, or SOC_TMC_NOF_DROP_PRECEDENCE which means ECN and not a real drop precedence.
*  SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_wred_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_tail_drop_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets tail drop parameter - max-queue-size per rate-class
*     and drop precedence. The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3, or SOC_TMC_NOF_DROP_PRECEDENCE which means ECN and not a real drop precedence.
*  SOC_SAND_IN  ARAD_ITM_TAIL_DROP_INFO  *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_TAIL_DROP_INFO  *exact_info -
*     pointer to returned exact data.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_tail_drop_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_TAIL_DROP_INFO  *info,
    SOC_SAND_OUT ARAD_ITM_TAIL_DROP_INFO  *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_tail_drop_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets tail drop parameter - max-queue-size per rate-class
*     and drop precedence. The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3, or SOC_TMC_NOF_DROP_PRECEDENCE which means ECN and not a real drop precedence.
*  SOC_SAND_IN  ARAD_ITM_TAIL_DROP_INFO  *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_tail_drop_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_TAIL_DROP_INFO  *info
  );

/*********************************************************************
* NAME:
*     arad_itm_tail_drop_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets tail drop parameter - max-queue-size per rate-class
*     and drop precedence. The tail drop mechanism drops
*     packets that are mapped to queues that exceed thresholds
*     of this structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3, or SOC_TMC_NOF_DROP_PRECEDENCE which means ECN and not a real drop precedence.
*  SOC_SAND_OUT ARAD_ITM_TAIL_DROP_INFO  *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_tail_drop_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_TAIL_DROP_INFO  *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_wd_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets ingress-queue credit Watchdog thresholds and
*     configuration. includes: start-queue, end-queue and
*     wd-rates.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CR_WD_INFO      *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_CR_WD_INFO      *exact_info -
*     Loaded with actual watchdog parameters.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_wd_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_IN  ARAD_ITM_CR_WD_INFO      *info,
    SOC_SAND_OUT ARAD_ITM_CR_WD_INFO      *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_wd_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets ingress-queue credit Watchdog thresholds and
*     configuration. includes: start-queue, end-queue and
*     wd-rates.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_CR_WD_INFO      *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_wd_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_IN  ARAD_ITM_CR_WD_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_itm_cr_wd_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets ingress-queue credit Watchdog thresholds and
*     configuration. includes: start-queue, end-queue and
*     wd-rates.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_CR_WD_INFO      *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_cr_wd_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id, 
    SOC_SAND_OUT ARAD_ITM_CR_WD_INFO      *info
  );


/*********************************************************************
*     Set ECN as enabled or disabled for the device
*********************************************************************/
uint32
  arad_itm_enable_ecn_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_IN  uint32   enabled /* ECN will be enabled/disabled for non zero/zero values */
  );

/*********************************************************************
*     Return if ECN is enabled for the device
*********************************************************************/
uint32
  arad_itm_get_ecn_enabled_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_OUT uint32   *enabled /* will return non zero if /ECN is enabled */
  );


/*********************************************************************
* NAME:
*     arad_itm_vsq_qt_rt_cls_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Each Virtual Statistics Queue has a VSQ-Rate-Class.
*     This function assigns a VSQ with its Rate Class.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx -
*     Vsq in group index to which to configure rate class. the
*     index shoud be according the group it is, as following:
*     vsq_group 0 (category): 0-3 vsq_group 1 (cat + traffic
*     class): 0-32 vsq_group 2 (cat2/3 + connection class):
*     0-64 vsq_group 3 (statistics tag): 0-255
*  SOC_SAND_IN  uint32                 vsq_rt_cls -
*     Vsq rate class to configure, range: 0-15.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_qt_rt_cls_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_ocb_only,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_qt_rt_cls_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets Virtual Statistics Queue Rate-Class.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx -
*     Vsq in group index to which to configure rate class. the
*     index shoud be according the group it is, as following:
*     vsq_group 0 (category): 0-3 vsq_group 1 (cat + traffic
*     class): 0-32 vsq_group 2 (cat2/3 + connection class):
*     0-64 vsq_group 3 (statistics tag): 0-255
*  SOC_SAND_IN  uint32                 vsq_rt_cls -
*     Vsq rate class to configure, range: 0-15.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_qt_rt_cls_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_ocb_only,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_qt_rt_cls_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets Virtual Statistics Queue Rate-Class.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx -
*     Vsq in group index to which to configure rate class. the
*     index shoud be according the group it is, as following:
*     vsq_group 0 (category): 0-3 vsq_group 1 (cat + traffic
*     class): 0-32 vsq_group 2 (cat2/3 + connection class):
*     0-64 vsq_group 3 (statistics tag): 0-255
*  SOC_SAND_OUT uint32                 *vsq_rt_cls -
*     Vsq rate class to configure, range: 0-15.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_qt_rt_cls_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_ocb_only,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx,
    SOC_SAND_OUT uint32                 *vsq_rt_cls
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_fc_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets Virtual Statistics Queue, includes: vsq-id,
*     rate-class
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class index. Range: 0-63
*  SOC_SAND_IN  ARAD_ITM_VSQ_FC_INFO     *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_VSQ_FC_INFO     *exact_info -
*     May vary due to rounding.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_fc_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_FC_INFO     *info,
    SOC_SAND_OUT ARAD_ITM_VSQ_FC_INFO     *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_fc_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets Virtual Statistics Queue, includes: vsq-id,
*     rate-class
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class index. Range: 0-63
*  SOC_SAND_IN  ARAD_ITM_VSQ_FC_INFO     *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_fc_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_FC_INFO     *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_fc_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets Virtual Statistics Queue, includes: vsq-id,
*     rate-class
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class index. Range: 0-63
*  SOC_SAND_OUT ARAD_ITM_VSQ_FC_INFO     *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_fc_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_OUT ARAD_ITM_VSQ_FC_INFO     *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_tail_drop_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets tail drop parameter on the VSQ - max-queue-size in
*     words and in buffer-descriptors per vsq-rate-class and
*     drop precedence. The tail drop mechanism drops packets
*     that are mapped to queues that exceed thresholds of this
*     structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     Ingress VSQ group index. Range: 0 to 3.
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     Ingress rate class. Range: 0 to 15.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3.
*  SOC_SAND_IN  ARAD_ITM_VSQ_TAIL_DROP_INFO *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_VSQ_TAIL_DROP_INFO *exact_info -
*     Pointer to returned exact data.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_tail_drop_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_TAIL_DROP_INFO *info,
    SOC_SAND_OUT ARAD_ITM_VSQ_TAIL_DROP_INFO *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_tail_drop_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets tail drop parameter on the VSQ - max-queue-size in
*     words and in buffer-descriptors per vsq-rate-class and
*     drop precedence. The tail drop mechanism drops packets
*     that are mapped to queues that exceed thresholds of this
*     structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     Ingress VSQ group index. Range: 0 to 3.
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     Ingress rate class. Range: 0 to 15.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3.
*  SOC_SAND_IN  ARAD_ITM_VSQ_TAIL_DROP_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_tail_drop_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_TAIL_DROP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_tail_drop_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets tail drop parameter on the VSQ - max-queue-size in
*     words and in buffer-descriptors per vsq-rate-class and
*     drop precedence. The tail drop mechanism drops packets
*     that are mapped to queues that exceed thresholds of this
*     structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     Ingress VSQ group index. Range: 0 to 3.
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     Ingress rate class. Range: 0 to 15.
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     The drop precedence to set. Range: 0-3.
*  SOC_SAND_OUT ARAD_ITM_VSQ_TAIL_DROP_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_tail_drop_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_VSQ_TAIL_DROP_INFO *info
  );

/********************************************************************* 
* NAME:
*     arad_itm_vsq_tail_drop_get_default_unsafe 
*     Get tail drop default parameters on the VSQ - max-queue-size in
*     words and in buffer-descriptors per vsq-rate-class.
*     The tail drop mechanism drops packets
*     that are mapped to queues that exceed thresholds of this
*     structure.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_VSQ_TAIL_DROP_INFO *info -
*     pointer to configuration structure.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_tail_drop_get_default_unsafe(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_OUT ARAD_ITM_VSQ_TAIL_DROP_INFO  *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_wred_gen_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This procedure sets VSQ WRED general configurations,
*     includes: WRED-enable and exponential-weight-queue (for
*     the WRED algorithm).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class. Range: 0-15
*  SOC_SAND_IN  ARAD_ITM_VSQ_WRED_GEN_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_wred_gen_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_WRED_GEN_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_wred_gen_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This procedure sets VSQ WRED general configurations,
*     includes: WRED-enable and exponential-weight-queue (for
*     the WRED algorithm).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class. Range: 0-15
*  SOC_SAND_IN  ARAD_ITM_VSQ_WRED_GEN_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_wred_gen_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_WRED_GEN_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_wred_gen_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     This procedure sets VSQ WRED general configurations,
*     includes: WRED-enable and exponential-weight-queue (for
*     the WRED algorithm).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class. Range: 0-15
*  SOC_SAND_OUT ARAD_ITM_VSQ_WRED_GEN_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_wred_gen_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_OUT ARAD_ITM_VSQ_WRED_GEN_INFO *info
  );
/*********************************************************************
* NAME:
*     arad_itm_vsq_wred_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     A WRED test for each packet versus the packet queue or
*     VSQ that the packet is mapped to is performed. This
*     procedure sets Virtual Statistics Queue WRED, includes:
*     WRED-thresholds/probability.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class. Range: 0-15
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     drop-precedence. Range: 0-3.
*  SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *exact_info -
*     May vary due to rounding.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_wred_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info,
    SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_wred_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     A WRED test for each packet versus the packet queue or
*     VSQ that the packet is mapped to is performed. This
*     procedure sets Virtual Statistics Queue WRED, includes:
*     WRED-thresholds/probability.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class. Range: 0-15
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     drop-precedence. Range: 0-3.
*  SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_wred_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_IN  ARAD_ITM_WRED_QT_DP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_wred_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     A WRED test for each packet versus the packet queue or
*     VSQ that the packet is mapped to is performed. This
*     procedure sets Virtual Statistics Queue WRED, includes:
*     WRED-thresholds/probability.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     There are 4 groups of vsqs (0-3).
*  SOC_SAND_IN  uint32                 vsq_rt_cls_ndx -
*     VSQ rate class. Range: 0-15
*  SOC_SAND_IN  uint32                 drop_precedence_ndx -
*     drop-precedence. Range: 0-3.
*  SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_vsq_wred_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  uint32                 vsq_rt_cls_ndx,
    SOC_SAND_IN  uint32                 drop_precedence_ndx,
    SOC_SAND_OUT ARAD_ITM_WRED_QT_DP_INFO *info
  );
uint32
  arad_itm_man_exp_buffer_set(
    SOC_SAND_IN  int32  value,
    SOC_SAND_IN  uint32 mnt_lsb,
    SOC_SAND_IN  uint32 mnt_nof_bits,
    SOC_SAND_IN  uint32 exp_lsb,
    SOC_SAND_IN  uint32 exp_nof_bits,
    SOC_SAND_IN  uint8 is_signed,
    SOC_SAND_OUT uint32  *output_field,
    SOC_SAND_OUT int32  *exact_value
  );
uint32
  arad_itm_man_exp_buffer_get(
    SOC_SAND_IN  uint32  buffer,
    SOC_SAND_IN  uint32 mnt_lsb,
    SOC_SAND_IN  uint32 mnt_nof_bits,
    SOC_SAND_IN  uint32 exp_lsb,
    SOC_SAND_IN  uint32 exp_nof_bits,
    SOC_SAND_IN  uint8 is_signed,
    SOC_SAND_OUT int32  *value
  );
uint32
    arad_itm_vsq_in_group_size_get(
       SOC_SAND_IN  int                      unit, 
       SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx, 
       SOC_SAND_OUT uint32                   *vsq_in_group_size);
uint32
  arad_itm_vsq_idx_verify(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint8                    is_cob_only,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx
  );
/*********************************************************************
* NAME:
*     arad_itm_vsq_counter_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Select VSQ for monitoring. The selected VSQ counter can
*     be further read, indicating the number of packets
*     enqueued to the VSQ.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     VSQ group: 0 - Group A, VSQs 0->3, Category. 1 - Group
*     B, VSQs 4->35, Category & TC. 2 - Group C, VSQs
*     36->99,Category & CC 2/3. 3 - Group D, VSQs 100 -> 355,
*     STAG.
*  SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx -
*     VSQ in-group index:Group A (Category) Range: 0-3. Group
*     B (Category + TC) Range: 0-32. Group C (Category 2/3 +
*     CC) Range: 0-64. Group D (STAG) Range: 0-255. SOC_SAND_OUT
*     uint32 *pckt_count - Number of packets enqueued to the
*     specified VSQ
* REMARKS:
*     1. The counter is read using the vsq_counter_read API,
*        or any counter read/print API of arad_stat module.
*     2. This API is considered deprecated.
*        It is replaced by the arad_stat_vsq_cnt_select_set API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_itm_vsq_counter_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_cob_only,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_counter_verify
* TYPE:
*   PROC
* FUNCTION:
*     Select VSQ for monitoring. The selected VSQ counter can
*     be further read, indicating the number of packets
*     enqueued to the VSQ.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx -
*     VSQ group: 0 - Group A, VSQs 0->3, Category. 1 - Group
*     B, VSQs 4->35, Category & TC. 2 - Group C, VSQs
*     36->99,Category & CC 2/3. 3 - Group D, VSQs 100 -> 355,
*     STAG.
*  SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx -
*     VSQ in-group index:Group A (Category) Range: 0-3. Group
*     B (Category + TC) Range: 0-32. Group C (Category 2/3 +
*     CC) Range: 0-64. Group D (STAG) Range: 0-255. SOC_SAND_OUT
*     uint32 *pckt_count - Number of packets enqueued to the
*     specified VSQ
* REMARKS:
*     1. The counter is read using the vsq_counter_read API,
*        or any counter read/print API of arad_stat module.
*     2. This API is considered deprecated.
*        It is replaced by the arad_stat_vsq_cnt_select_set API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_itm_vsq_counter_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_IN  uint8               is_cob_only,
    SOC_SAND_IN  ARAD_ITM_VSQ_GROUP       vsq_group_ndx,
    SOC_SAND_IN  ARAD_ITM_VSQ_NDX         vsq_in_group_ndx
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_counter_get
* TYPE:
*   PROC
* FUNCTION:
*     Select VSQ for monitoring. The selected VSQ counter can
*     be further read, indicating the number of packets
*     enqueued to the VSQ.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT  ARAD_ITM_VSQ_GROUP       *vsq_group_ndx -
*     VSQ group: 0 - Group A, VSQs 0->3, Category. 1 - Group
*     B, VSQs 4->35, Category & TC. 2 - Group C, VSQs
*     36->99,Category & CC 2/3. 3 - Group D, VSQs 100 -> 355,
*     STAG.
*  SOC_SAND_OUT  ARAD_ITM_VSQ_NDX         *vsq_in_group_ndx -
*     VSQ in-group index:Group A (Category) Range: 0-3. Group
*     B (Category + TC) Range: 0-32. Group C (Category 2/3 +
*     CC) Range: 0-64. Group D (STAG) Range: 0-255. SOC_SAND_OUT
*     uint32 *pckt_count - Number of packets enqueued to the
*     specified VSQ
* REMARKS:
*     1. The counter is read using the vsq_counter_read API,
*        or any counter read/print API of arad_stat module.
*     2. This API is considered deprecated.
*        It is replaced by the arad_stat_vsq_cnt_select_set API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_itm_vsq_counter_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_OUT uint8                    *is_cob_only,
    SOC_SAND_OUT ARAD_ITM_VSQ_GROUP       *vsq_group_ndx,
    SOC_SAND_OUT ARAD_ITM_VSQ_NDX         *vsq_in_group_ndx
  );

/*********************************************************************
* NAME:
*     arad_itm_vsq_counter_read_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Indicates the number of packets enqueued to the
*     monitored VSQ.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT uint32                  *pckt_count -
*     Number of packets enqueued to the VSQ
* REMARKS:
*  1. The counter is selected using the vsq_counter_set API.
*  2. Maximal value (0xFFFFFFFF) indicates counter overflow.
*  3. Can be also read using counter read/print APIs of arad_stat module.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/

uint32
  arad_itm_vsq_counter_read_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_OUT uint32                  *pckt_count
  );



uint32
  arad_itm_queue_is_ocb_only_get(
     SOC_SAND_IN  int                 unit,
     SOC_SAND_IN  int                 core,
     SOC_SAND_IN  uint32              queue_ndx,
     SOC_SAND_OUT uint8               *enable
     );
/*********************************************************************
* NAME:
*     arad_itm_queue_info_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the queue types of a queue
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  queue_ndx -
*     The ID of the queue to be set (Range: 0 - 32K-1).
*  SOC_SAND_IN  ARAD_ITM_QUEUE_INFO      *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_info_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                  queue_ndx,
    SOC_SAND_IN  ARAD_ITM_QUEUE_INFO      *old_info,
    SOC_SAND_IN  ARAD_ITM_QUEUE_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_itm_queue_info_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the queue types of a queue
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  queue_ndx -
*     The ID of the queue to be set (Range: 0 - 32768).
*  SOC_SAND_IN  ARAD_ITM_QUEUE_INFO      *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_info_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                  queue_ndx,
    SOC_SAND_IN  ARAD_ITM_QUEUE_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_itm_queue_info_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Sets the queue types of a queue
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  queue_ndx -
*     The ID of the queue to be set (Range: 0 - 32768).
*  SOC_SAND_OUT ARAD_ITM_QUEUE_INFO      *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_info_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core,
    SOC_SAND_IN  uint32                  queue_ndx,
    SOC_SAND_OUT ARAD_ITM_QUEUE_INFO      *info
  );

/*********************************************************************
* NAME:
*     arad_itm_queue_dyn_info_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Gets the dynamic info of a queue
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                  queue_ndx -
*     The ID of the queue to be set (Range: 0 - 32768).
*  SOC_SAND_OUT ARAD_ITM_QUEUE_DYN_INFO      *info -
*     pointer to dynamic info structure.
* REMARKS:
*     None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_queue_dyn_info_get_unsafe(
	SOC_SAND_IN  int 				unit,
    SOC_SAND_IN  int                 core,
	SOC_SAND_IN  uint32 				 queue_ndx,
	SOC_SAND_OUT ARAD_ITM_QUEUE_DYN_INFO	  *info
  );

int
  arad_ingress_drop_status(
    SOC_SAND_IN int   unit,
    SOC_SAND_OUT uint32 *is_max_size 
  );

/*
 * set the dynamic queue thresholds for the guaranteed resource.
 * The threshold is used to achieve the resource guarantee for queues,
 * ensuring that not to much of the resource is allocated bound the total of guarantees.
 */
uint32
  arad_itm_dyn_total_thresh_set_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_IN  int32    reservation_increase /* the (signed) amount in which the thresholds should decrease (according to 100% as will be set for DP 0) */
  );

/*********************************************************************
* NAME:
*     arad_itm_ingress_shape_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     The device may have one continues range of queues that
*     belongs to ingress shaping. And set the Ingress shaping
*     credit generator.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_INGRESS_SHAPE_INFO *info -
*     Ingress shaping configuration.
* REMARKS:
*     Base Queue number and add/subtract mode must be set
*     prior to calling this API. To set base-q configuration,
*     use ipq_explicit_mapping_mode_info_set API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_ingress_shape_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_INGRESS_SHAPE_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_ingress_shape_verify
* TYPE:
*   PROC
* FUNCTION:
*     Sets ingress shaping configuration. This includes
*     ingress shaping queues range, and credit generation
*     configuration.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_INGRESS_SHAPE_INFO *info -
*     Ingress shaping configuration.
* REMARKS:
*     Base Queue number and add/subtract mode must be set
*     prior to calling this API. To set base-q configuration,
*     use ipq_explicit_mapping_mode_info_set API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_ingress_shape_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_INGRESS_SHAPE_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_ingress_shape_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Sets ingress shaping configuration. This includes
*     ingress shaping queues range, and credit generation
*     configuration.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_INGRESS_SHAPE_INFO *info -
*     Ingress shaping configuration.
* REMARKS:
*     Base Queue number and add/subtract mode must be set
*     prior to calling this API. To set base-q configuration,
*     use ipq_explicit_mapping_mode_info_set API.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_ingress_shape_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_INGRESS_SHAPE_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_priority_map_tmplt_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Four sets of queues-priorities maps are held in the
*     device. Per map: describes a segment of 64 contiguous
*     queues. Each queue is either high or low priority.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                 map_ndx -
*  SOC_SAND_IN  ARAD_ITM_PRIORITY_MAP_TMPLT *info -
*     info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_priority_map_tmplt_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 map_ndx,
    SOC_SAND_IN  ARAD_ITM_PRIORITY_MAP_TMPLT *info
  );

/*********************************************************************
* NAME:
*     arad_itm_priority_map_tmplt_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Four sets of queues-priorities maps are held in the
*     device. Per map: describes a segment of 64 contiguous
*     queues. Each queue is either high or low priority.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                 map_ndx -
*   Index of map to set. Range: 0 - 3.
*  SOC_SAND_IN  ARAD_ITM_PRIORITY_MAP_TMPLT *info -
*     info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_priority_map_tmplt_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 map_ndx,
    SOC_SAND_IN  ARAD_ITM_PRIORITY_MAP_TMPLT *info
  );

/*********************************************************************
* NAME:
*     arad_itm_priority_map_tmplt_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Four sets of queues-priorities maps are held in the
*     device. Per map: describes a segment of 64 contiguous
*     queues. Each queue is either high or low priority.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  uint32                 map_ndx -
*   Index of map to set. Range: 0 - 3.
*  SOC_SAND_OUT ARAD_ITM_PRIORITY_MAP_TMPLT *info -
*     info pointer to configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_priority_map_tmplt_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 map_ndx,
    SOC_SAND_OUT ARAD_ITM_PRIORITY_MAP_TMPLT *info
  );

/*********************************************************************
* NAME:
*     arad_itm_priority_map_tmplt_select_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     The 32K ingress-queues range is segmented into 512
*     segments of 64 contiguous queues, that is, queues 64N to
*     64N+63 that all have the same map-id (one of four).
* INPUT:
*  SOC_SAND_IN  uint32                  queue_64_ndx -
*   The index to the contiguous 64 queues. Range: 0 - 511.
*  SOC_SAND_IN  uint32                 priority_map -
*   The map-id. Range: 0 - 3.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_priority_map_tmplt_select_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  queue_64_ndx,
    SOC_SAND_IN  uint32                 priority_map
  );

/*********************************************************************
* NAME:
*     arad_itm_priority_map_tmplt_select_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     The 32K ingress-queues range is segmented into 512
*     segments of 64 contiguous queues, that is, queues 64N to
*     64N+63 that all have the same map-id (one of four).
* INPUT:
*  SOC_SAND_IN  uint32                  queue_64_ndx -
*   The index to the contiguous 64 queues. Range: 0 - 511.
*  SOC_SAND_IN  uint32                 priority_map -
*   The map-id. Range: 0 - 3.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_priority_map_tmplt_select_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  queue_64_ndx,
    SOC_SAND_IN  uint32                 priority_map
  );

/*********************************************************************
* NAME:
*     arad_itm_priority_map_tmplt_select_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     The 32K ingress-queues range is segmented into 512
*     segments of 64 contiguous queues, that is, queues 64N to
*     64N+63 that all have the same map-id (one of four).
* INPUT:
*  SOC_SAND_IN  uint32                  queue_64_ndx -
*   The index to the contiguous 64 queues. Range: 0 - 511.
*  SOC_SAND_IN  uint32                 priority_map -
*   The map-id. Range: 0 - 3.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_priority_map_tmplt_select_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                  queue_64_ndx,
    SOC_SAND_OUT uint32                 *priority_map
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_drop_prob_verify
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     System Red drop probabilities table fill in. The system
*     Red mechanism uses a table of 16 probabilities. The
*     table is used by indexes which choose 1 out of the 16
*     options.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_DROP_PROB *info -
*     info A pointer to the system red queue type info
*     configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_drop_prob_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_DROP_PROB *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_drop_prob_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     System Red drop probabilities table fill in. The system
*     Red mechanism uses a table of 16 probabilities. The
*     table is used by indexes which choose 1 out of the 16
*     options.
*     Note that the System-Red mechanism is a system-wide
*     attribute and it should be configured homogeneously
*     in all FAPs.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_DROP_PROB *info -
*     info A pointer to the system red queue type info
*     configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_drop_prob_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_DROP_PROB *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_drop_prob_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     System Red drop probabilities table fill in. The system
*     Red mechanism uses a table of 16 probabilities. The
*     table is used by indexes which choose 1 out of the 16
*     options.
*     Note that the System-Red mechanism is a system-wide
*     attribute and it should be configured homogeneously
*     in all FAPs.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_DROP_PROB *info -
*     info A pointer to the system red queue type info
*     configuration.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_drop_prob_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_DROP_PROB *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_queue_size_boundaries_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     System Red queue size boundaries, per queue type - rate
*     class. The queue size ranges table is set. For each
*     queue type and drop-precedence,
*     drop/pass/drop-with-probability parameters are set using
*     the function arad_itm_sys_red_qt_dp_info_set.
*     Note that the System-Red mechanism is a system-wide
*     attribute and it should be configured homogeneously
*     in all FAPs.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class Queue Type. Range: 0 to 63.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_INFO *info -
*     A pointer to the system red queue type info
*     configuration.
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *exact_info -
*     May vary due to rounding.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_queue_size_boundaries_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_INFO *info,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *exact_info
  );

uint32
  x_itm_sys_red_queue_size_boundaries_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_INFO *info,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *exact_info,
    SOC_SAND_IN int mnt_lsb,
    SOC_SAND_IN int mnt_nof_bits,
    SOC_SAND_IN int exp_lsb,
    SOC_SAND_IN int exp_nof_bits
  );
/*********************************************************************
* NAME:
*     arad_itm_sys_red_queue_size_boundaries_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     System Red queue size boundaries, per queue type - rate
*     class. The queue size ranges table is set. For each
*     queue type and drop-precedence,
*     drop/pass/drop-with-probability parameters are set using
*     the function arad_itm_sys_red_qt_dp_info_set.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class Queue Type. Range: 0 to 63.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_INFO *info -
*     A pointer to the system red queue type info
*     configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_queue_size_boundaries_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_queue_size_boundaries_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     System Red queue size boundaries, per queue type - rate
*     class. The queue size ranges table is set. For each
*     queue type and drop-precedence,
*     drop/pass/drop-with-probability parameters are set using
*     the function arad_itm_sys_red_qt_dp_info_set.
*     Note that the System-Red mechanism is a system-wide
*     attribute and it should be configured homogeneously
*     in all FAPs.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress rate class Queue Type. Range: 0 to 63.
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *info -
*     A pointer to the system red queue type info
*     configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_queue_size_boundaries_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *info
  );
uint32
  x_itm_sys_red_queue_size_boundaries_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_INFO *info,
    SOC_SAND_IN int mnt_lsb,
    SOC_SAND_IN int mnt_nof_bits,
    SOC_SAND_IN int exp_lsb,
    SOC_SAND_IN int exp_nof_bits
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_q_based_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Configures the ingress system red parameters per q-type
*     - rate class and drop-precedence. This includes the
*     thresholds and drop probability, which determine the
*     behavior of the algorithm according to the queue size
*     index.
*     Note that the System-Red mechanism is a system-wide
*     attribute and it should be configured homogeneously
*     in all FAPs.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress QDP Queue Type. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 sys_red_dp_ndx -
*     The drop precedence to be affected by this
*     configuration. Range: 0 - 3.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_DP_INFO *info -
*     Pointer to configuration structure. SOC_SAND_OUT
*     ARAD_ITM_SYS_RED_QT_DP_INFO *exact_info - To be filled
*     with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_q_based_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 sys_red_dp_ndx,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_DP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_q_based_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Configures the ingress system red parameters per q-type
*     - rate class and drop-precedence. This includes the
*     thresholds and drop probability, which determine the
*     behavior of the algorithm according to the queue size
*     index.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress QDP Queue Type. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 sys_red_dp_ndx -
*     The drop precedence to be affected by this
*     configuration. Range: 0 - 3.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_DP_INFO *info -
*     Pointer to configuration structure. SOC_SAND_OUT
*     ARAD_ITM_SYS_RED_QT_DP_INFO *exact_info - To be filled
*     with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_q_based_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 sys_red_dp_ndx,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_QT_DP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_q_based_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     Configures the ingress system red parameters per q-type
*     - rate class and drop-precedence. This includes the
*     thresholds and drop probability, which determine the
*     behavior of the algorithm according to the queue size
*     index.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                 rt_cls_ndx -
*     Ingress QDP Queue Type. Range: 0 to 63.
*  SOC_SAND_IN  uint32                 sys_red_dp_ndx -
*     The drop precedence to be affected by this
*     configuration. Range: 0 - 3.
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_DP_INFO *info -
*     Pointer to configuration structure. SOC_SAND_OUT
*     ARAD_ITM_SYS_RED_QT_DP_INFO *exact_info - To be filled
*     with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_q_based_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 rt_cls_ndx,
    SOC_SAND_IN  uint32                 sys_red_dp_ndx,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_QT_DP_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_eg_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     At the outgoing FAP port, a System-Queue-Size is
*     maintained. Per a configurable aging-period time the
*     queue is aged. System-Queue-Size has two again models
*     (when aging time arrived): reset or decrement. Reset
*     sets the System-Queue-Size to zero, decrement decrease
*     the size of the OFP System-Queue-Size with one. Note:
*     though this function is not an ITM function, it resides
*     here due to relevance to other System RED functions.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_EG_INFO *info -
*     pointer to configuration structure.
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_EG_INFO *exact_info -
*     To be filled with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_eg_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_EG_INFO *info,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_EG_INFO *exact_info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_eg_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     At the outgoing FAP port, a System-Queue-Size is
*     maintained. Per a configurable aging-period time the
*     queue is aged. System-Queue-Size has two again models
*     (when aging time arrived): reset or decrement. Reset
*     sets the System-Queue-Size to zero, decrement decrease
*     the size of the OFP System-Queue-Size with one. Note:
*     though this function is not an ITM function, it resides
*     here due to relevance to other System RED functions.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_EG_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_eg_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_EG_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_eg_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     At the outgoing FAP port, a System-Queue-Size is
*     maintained. Per a configurable aging-period time the
*     queue is aged. System-Queue-Size has two again models
*     (when aging time arrived): reset or decrement. Reset
*     sets the System-Queue-Size to zero, decrement decrease
*     the size of the OFP System-Queue-Size with one. Note:
*     though this function is not an ITM function, it resides
*     here due to relevance to other System RED functions.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_EG_INFO *info -
*     pointer to configuration structure.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_eg_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_EG_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_glob_rcs_set_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     In the System Red mechanism there is an aspect of
*     Consumed Resources. This mechanism gives the queues a
*     value that is compared with the value of the queue size
*     index - the maximum of the 2 is sent to the threshold
*     tests. The queues are divided to 4 ranges. In 3 types:
*     Free Unicast Data buffers Thresholds, Free Multicast
*     Data buffers Thresholds, Free BD buffers Thresholds.
*     This function determines the thresholds of the ranges
*     and the values of the ranges (0-15).
*     Note that the value of the queue is attributed to the
*     consumed resources (as opposed to the free resources).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info -
*     pointer to configuration structure. SOC_SAND_OUT
*     ARAD_ITM_SYS_RED_GLOB_RCS_VAL_INFO *exact_info - To be
*     filled with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_glob_rcs_set_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_glob_rcs_verify
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     In the System Red mechanism there is an aspect of
*     Consumed Resources. This mechanism gives the queues a
*     value that is compared with the value of the queue size
*     index - the maximum of the 2 is sent to the threshold
*     tests. The queues are divided to 4 ranges. In 3 types:
*     Free Unicast Data buffers Thresholds, Free Multicast
*     Data buffers Thresholds, Free BD buffers Thresholds.
*     This function determines the thresholds of the ranges
*     and the values of the ranges (0-15).
*     Note that the value of the queue is attributed to the
*     consumed resources (as opposed to the free resources).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info -
*     pointer to configuration structure. SOC_SAND_OUT
*     ARAD_ITM_SYS_RED_GLOB_RCS_VAL_INFO *exact_info - To be
*     filled with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_glob_rcs_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );

/*********************************************************************
* NAME:
*     arad_itm_sys_red_glob_rcs_get_unsafe
* TYPE:
*   PROC
* DATE:
*   May 12 2008
* FUNCTION:
*     In the System Red mechanism there is an aspect of
*     Consumed Resources. This mechanism gives the queues a
*     value that is compared with the value of the queue size
*     index - the maximum of the 2 is sent to the threshold
*     tests. The queues are divided to 4 ranges. In 3 types:
*     Free Unicast Data buffers Thresholds, Free Multicast
*     Data buffers Thresholds, Free BD buffers Thresholds.
*     This function determines the thresholds of the ranges
*     and the values of the ranges (0-15).
*     Note that the value of the queue is attributed to the
*     consumed resources (as opposed to the free resources).
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info -
*     pointer to configuration structure. SOC_SAND_OUT
*     ARAD_ITM_SYS_RED_GLOB_RCS_VAL_INFO *exact_info - To be
*     filled with exact configuration.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_sys_red_glob_rcs_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_OUT ARAD_ITM_SYS_RED_GLOB_RCS_INFO *info
  );



/*********************************************************************
* NAME:
*     arad_itm_convert_admit_one_test_tmplt_to_u32
* TYPE:
*   PROC
* DATE:
*   Nov 18 2007
* FUNCTION:
*     Convert the type ARAD_ITM_ADMIT_ONE_TEST_TMPLT to uint32
*     in order to be written to the register field.
* INPUT:
*  SOC_SAND_IN ARAD_ITM_ADMIT_ONE_TEST_TMPLT test -
*     the ARAD_ITM_ADMIT_ONE_TEST_TMPLT type consists of 4 Booleans,
*     to be converted to a 4 bits
*  SOC_SAND_OUT uint32                       *test_in_sand_u32 -
*     pointer to the converted value.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_convert_admit_one_test_tmplt_to_u32(
    SOC_SAND_IN ARAD_ITM_ADMIT_ONE_TEST_TMPLT test,
    SOC_SAND_OUT uint32                       *test_in_sand_u32
  );
/*********************************************************************
* NAME:
*     arad_itm_convert_u32_to_admit_one_test_tmplt
* TYPE:
*   PROC
* DATE:
*   Nov 18 2007
* FUNCTION:
*     Convert the type uint32 to ARAD_ITM_ADMIT_ONE_TEST_TMPLT
*     in order to be received from the register field.
* INPUT:
*  SOC_SAND_IN  uint32 test_in_sand_u32 -
*    read from the register.
*  SOC_SAND_OUT ARAD_ITM_ADMIT_ONE_TEST_TMPLT *test -
*     pointer to the converted value.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_convert_u32_to_admit_one_test_tmplt(
    SOC_SAND_IN  uint32                       test_in_sand_u32,
    SOC_SAND_OUT ARAD_ITM_ADMIT_ONE_TEST_TMPLT *test
  );

uint32
  arad_itm_dbuff_internal2size(
    SOC_SAND_IN  uint32                   dbuff_size_internal,
    SOC_SAND_OUT ARAD_ITM_DBUFF_SIZE_BYTES *dbuff_size
  );

uint32
  arad_itm_dbuff_size2internal(
    SOC_SAND_IN  ARAD_ITM_DBUFF_SIZE_BYTES dbuff_size,
    SOC_SAND_OUT uint32                   *dbuff_size_internal
  );

/*
 *	Extension to the Arad API
 */
uint32
  arad_b_itm_glob_rcs_drop_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_ITM_GLOB_RCS_DROP_TH *info
  );

uint32
    arad_b_itm_glob_rcs_drop_set_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  ARAD_THRESH_WITH_HYST_INFO mem_size[ARAD_NOF_DROP_PRECEDENCE],
      SOC_SAND_OUT ARAD_THRESH_WITH_HYST_INFO exact_mem_size[ARAD_NOF_DROP_PRECEDENCE]
    );

uint32
  arad_b_itm_glob_rcs_drop_get_unsafe(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_OUT ARAD_THRESH_WITH_HYST_INFO *mem_size
  );

/*********************************************************************
* NAME:
 *   arad_itm_committed_q_size_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the size of committed queue size (i.e., the
 *   guaranteed memory) for each VOQ, even in the case that a
 *   set of queues consume most of the memory resources.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    grnt_bytes -
 *     Value of the guaranteed memory size. Units: Bytes. Range:
 *     0 - 256M.
 *   SOC_SAND_OUT uint32                    *exact_grnt_bytes -
 *     Exact value of the guaranteed memory size. Units: Bytes.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_itm_committed_q_size_set_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  rt_cls_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_GUARANTEED_INFO *info, 
    SOC_SAND_OUT SOC_TMC_ITM_GUARANTEED_INFO *exact_info 
  );

uint32
  arad_itm_committed_q_size_set_verify(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  rt_cls_ndx,
    SOC_SAND_IN  SOC_TMC_ITM_GUARANTEED_INFO *info
  );

uint32
  arad_itm_committed_q_size_get_verify(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  uint32        rt_cls_ndx
  );
/*********************************************************************
*     Gets the configuration set by the
 *     "arad_itm_committed_q_size_set_unsafe" API.
 *     Refer to "arad_itm_committed_q_size_set_unsafe" API for
 *     details.
*********************************************************************/
uint32
  arad_itm_committed_q_size_get_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  rt_cls_ndx,
    SOC_SAND_OUT SOC_TMC_ITM_GUARANTEED_INFO *exact_info 
  );

/* Maps the VOQ TC to the VSQ TC for the new VSQ types per interface */
soc_error_t
arad_itm_pfc_tc_map_set_unsafe(const int unit, const int tc_in, const int port_id, const int tc_out);

soc_error_t
arad_itm_pfc_tc_map_get_unsafe(const int unit, const int tc_in, const int port_id, int *tc_out); 

/*********************************************************************
* NAME:
 *   arad_itm_dp_discard_set_unsafe
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the drop precedence value above which 
 *     all packets will always be discarded.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                    discard_dp -
 *     Value of DP above which packets
 *     will de discarded. Range: 0-4.
 *     4 means disables the discard.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
int
  arad_itm_dp_discard_set_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  discard_dp
  );

uint32
  arad_itm_dp_discard_set_verify(
    SOC_SAND_IN  int        unit,
    SOC_SAND_IN  uint32        discard_dp
  );

/*********************************************************************
*     Gets the configuration set by the
 *     "arad_itm_dp_discard_set_unsafe" API.
 *     Refer to "arad_itm_dp_discard_set_unsafe" API for
 *     details.
*********************************************************************/
int
  arad_itm_dp_discard_get_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_OUT uint32                  *discard_dp
  );

/**
 * Initialize the Drop Precedence Mapping: 
 * <dp-meter-cmd,incoming-dp,meter-processor-dp> -> <ingress-dp,egress-dp> 
 * The (auto-detected) mode (PP or TM) determines the mapping (for TM we ignore the meter and the 
 * ingress-dp is passed through). 
 * 
 * @param unit 
 * 
 * @return uint32 
 */
uint32 
  arad_itm_setup_dp_map(
    SOC_SAND_IN  int unit
  );

/*
 * Set the alpha value of fair adaptive tail drop for the given rate class and DP.
 * Arad+ only.
 */
uint32
  arad_plus_itm_alpha_set_unsafe(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint32       rt_cls_ndx,
    SOC_SAND_IN  uint32       drop_precedence_ndx,
    SOC_SAND_IN  int32        alpha 
  );

/*
 * Get the alpha value of fair adaptive tail drop for the given rate class and DP.
 * Arad+ only.
 */
uint32
  arad_plus_itm_alpha_get_unsafe(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint32       rt_cls_ndx,
    SOC_SAND_IN  uint32       drop_precedence_ndx,
    SOC_SAND_OUT int32        *alpha 
  );

/*
 * Arad+ only: enable/disable fair adaptive tail drop (Free BDs dynamic MAX queue size)
 */
uint32
  arad_plus_itm_fair_adaptive_tail_drop_enable_set_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_IN  uint8    enabled /* 0=disabled, non zero=enabled */
  );

/*
 * Arad+ only: Check if fair adaptive tail drop (Free BDs dynamic MAX queue size) is enabled.
 */
uint32
  arad_plus_itm_fair_adaptive_tail_drop_enable_get_unsafe(
    SOC_SAND_IN  int   unit,
    SOC_SAND_OUT uint8    *enabled /* return value: 0=disabled, 1=enabled */
  );

/*
Get Arad ingress congestion statistics.
*/
int arad_itm_congestion_statistics_get(
  SOC_SAND_IN int unit,
  SOC_SAND_IN int core,
  SOC_SAND_OUT ARAD_ITM_CGM_CONGENSTION_STATS *stats /* place current statistics output here */
  );

/* } */


#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_INGRESS_TRAFFIC_MGMT_INCLUDED__*/
#endif

