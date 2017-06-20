/* $Id: jer2_arad_api_ingress_packet_queuing.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_API_INGRESS_PACKET_QUEUING_INCLUDED__
/* { */
#define __JER2_ARAD_API_INGRESS_PACKET_QUEUING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ingress_packet_queuing.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/cosq.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_IPQ_TR_CLS DNX_TMC_IPQ_TR_CLS

#define JER2_ARAD_IPQ_MAX_NOF_GLAG_PORTS 16

/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define JER2_ARAD_IPQ_Q_TO_QRTT_ID(que_id)                 DNX_TMC_IPQ_Q_TO_QRTT_ID(que_id)
#define JER2_ARAD_IPQ_QRTT_TO_Q_ID(q_que_id)               DNX_TMC_IPQ_QRTT_TO_Q_ID(q_que_id)
#define JER2_ARAD_IPQ_Q_TO_1K_ID(que_id)                   DNX_TMC_IPQ_Q_TO_1K_ID(que_id)
#define JER2_ARAD_IPQ_1K_TO_Q_ID(k_que_id)                 DNX_TMC_IPQ_1K_TO_Q_ID(k_que_id)

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

typedef DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO                 JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO;
typedef DNX_TMC_IPQ_BASEQ_MAP_INFO                             JER2_ARAD_IPQ_BASEQ_MAP_INFO;
typedef DNX_TMC_IPQ_QUARTET_MAP_INFO                           JER2_ARAD_IPQ_QUARTET_MAP_INFO;

#define JER2_ARAD_IPQ_TR_CLS_MIN        DNX_TMC_IPQ_TR_CLS_MIN
#define JER2_ARAD_IPQ_TR_CLS_MAX        DNX_TMC_IPQ_TR_CLS_MAX
typedef DNX_TMC_IPQ_TR_CLS_RNG          JER2_ARAD_IPQ_TR_CLS_RNG;

#define JER2_ARAD_IPQ_TR_CLS_RNG_LAST   DNX_TMC_IPQ_TR_CLS_RNG_LAST

/* Ingress TC Mapping Profiles (UC, Flow) */
#define JER2_ARAD_NOF_INGRESS_UC_TC_MAPPING_PROFILES    DNX_TMC_NOF_INGRESS_UC_TC_MAPPING_PROFILES
#define JER2_ARAD_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES  DNX_TMC_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES

/* Max & min values for Stak Lag and base_queue:      */
#define JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MIN     DNX_TMC_IPQ_STACK_LAG_DOMAIN_MIN
#define JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MAX     DNX_TMC_IPQ_STACK_LAG_DOMAIN_MAX

#define JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MIN     DNX_TMC_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MIN
#define JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MAX     DNX_TMC_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MAX
#define JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_ALL     DNX_TMC_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_ALL

#define JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MIN     DNX_TMC_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MIN
#define JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MAX     DNX_TMC_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MAX
#define JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL     DNX_TMC_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL

/* 
 * Max & min values for system port destination index (dest_ndx)
 * and base queues (base_queue)
 * These values are used for system port to base queue mapping.   
 */
#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_DEST_NDX_MIN     0

#define JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_BASE_QUEUE_MIN     0

#define JER2_ARAD_IPQ_DESTINATION_ID_STACKING_BASE_QUEUE_MIN           ((0x7ff) << 6)
#define JER2_ARAD_IPQ_DESTINATION_ID_STACKING_BASE_QUEUE_MAX(unit)     (SOC_IS_QUX(unit) ? 0x3fff :\
                                                                   (SOC_IS_QAX(unit) ? (0x7fff) : (0x1ffff)))
#define JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit) JER2_ARAD_IPQ_DESTINATION_ID_STACKING_BASE_QUEUE_MAX(unit)


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
*     jer2_arad_ipq_explicit_mapping_mode_info_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info -
*     Mapping information of packet with explicit flow header
*     to queues
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_ipq_explicit_mapping_mode_info_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_explicit_mapping_mode_info_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info -
*     Mapping information of packet with explicit flow header
*     to queues
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
int
  jer2_arad_ipq_explicit_mapping_mode_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_traffic_class_map_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx -
*     tr_cls_ndx-the traffic class index, for which to map a
*     new class. DNX_SAND_IN JER2_ARAD_IPQ_TR_CLS class - The new
*     class that is mapped to the tr_cls_ndx.
*  DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          class -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_map_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_traffic_class_map_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx -
*     tr_cls_ndx-the traffic class index, for which to map a
*     new class. DNX_SAND_IN JER2_ARAD_IPQ_TR_CLS class - The new
*     class that is mapped to the tr_cls_ndx.
*  DNX_SAND_OUT JER2_ARAD_IPQ_TR_CLS          *class -
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_OUT JER2_ARAD_IPQ_TR_CLS          *new_class
  );
/*********************************************************************
* NAME:
*     jer2_arad_ipq_traffic_class_multicast_priority_map_set
* TYPE:
*   PROC
* DATE:
*   Jan 27 2013
* FUNCTION:
*     Sets multicast packets traffic class mapping to strict priority between high-low priority.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 traffic_class - tc to be set to high-low priority.
*  DNX_SAND_IN  uint8                  enable - if TURE, set to high priority, else to low priority.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_IN  uint8                  enable
  );
/*********************************************************************
* NAME:
*     jer2_arad_ipq_traffic_class_multicast_priority_map_get
* TYPE:
*   PROC
* DATE:
*   Jan 27 2013
* FUNCTION:
*     Gets multicast packets traffic class mapping to strict priority between high-low priority.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                 traffic_class - tc to be set to high-low priority.
*  DNX_SAND_OUT uint8                  *enable - if TURE, then is set to high priority, else to low priority.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_OUT uint8                  *enable
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_destination_id_packets_base_queue_id_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Sets the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  dest_ndx -
*     dest_ndx - The destination from the packet
*     System-Port-Destination ID. DNX_SAND_IN
*  DNX_SAND_IN  uint8                 valid -
*     If set the specific queue is valid, Otherwise the queue
*     is set to be invalid and packets are not sent to it.
*  DNX_SAND_IN  uint8                 sw_only -
*     If set then the queue mapping is stored only in SW DB, not in HW.
*  DNX_SAND_IN  uint32                  base_queue -
*     base_queue - Packet is stored in queue: base_queue +
*     class. Valid values: 0 - 32K.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_destination_id_packets_base_queue_id_set(
     DNX_SAND_IN  int                 unit,
     DNX_SAND_IN  int                 core,
     DNX_SAND_IN  uint32              dest_ndx,
     DNX_SAND_IN  uint8               valid,
     DNX_SAND_IN  uint8               sw_only,
     DNX_SAND_IN  uint32              base_queue
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_destination_id_packets_base_queue_id_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Gets the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  dest_ndx -
*     dest_ndx - The destination from the packet
*     System-Port-Destination ID. DNX_SAND_IN
*  DNX_SAND_OUT uint8                 *valid -
*     If set the specific queue is valid, Otherwise the queue
*     is set to be invalid and packets are not sent to it.
*  DNX_SAND_OUT  uint8                 *sw_only -
*     If set then the queue mapping is stored only in SW DB, not in HW.
*  DNX_SAND_OUT uint32                  *base_queue -
*     base_queue - Packet is stored in queue: base_queue +
*     class. Valid values: 0 - 32K.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_destination_id_packets_base_queue_id_get(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32              dest_ndx,
   DNX_SAND_OUT uint8               *valid,
   DNX_SAND_OUT uint8               *sw_only,
   DNX_SAND_OUT uint32              *base_queue
  );


uint32
  jer2_arad_ipq_stack_lag_packets_base_queue_id_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  tmd,
    DNX_SAND_IN  uint32                  entry,
    DNX_SAND_IN  uint32                  base_queue
  );

uint32
  jer2_arad_ipq_stack_lag_packets_base_queue_id_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 tmd,
    DNX_SAND_IN  uint32                 entry,
    DNX_SAND_OUT uint32              *base_queue
  );

uint32
  jer2_arad_ipq_stack_fec_map_stack_lag_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  tmd,
    DNX_SAND_IN  uint32                  entry,
    DNX_SAND_IN  uint32                  stack_lag
  );

uint32
  jer2_arad_ipq_stack_fec_map_stack_lag_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  tmd,
    DNX_SAND_IN  uint32                  entry,
    DNX_SAND_OUT  uint32*                stack_lag
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_queue_interdigitated_mode_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  k_queue_ndx -
*     The K queues to configure are (k_queue_ndx * 1024) -
*     (k_queue_ndx * 1024 + 1023) DNX_SAND_IN
*  DNX_SAND_IN  uint8                 is_interdigitated -
*     If TRUE, the K queues which k_queue_index points to will
*     be set to interdigitated mode.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_queue_interdigitated_mode_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  k_queue_ndx,
    DNX_SAND_IN  uint8                 is_interdigitated
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_queue_interdigitated_mode_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  k_queue_ndx -
*     The K queues to configure are (k_queue_ndx * 1024) -
*     (k_queue_ndx * 1024 + 1023) DNX_SAND_IN
*  DNX_SAND_OUT uint8                 *is_interdigitated -
*     If TRUE, the K queues which k_queue_index points to will
*     be set to interdigitated mode.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_queue_interdigitated_mode_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  k_queue_ndx,
    DNX_SAND_OUT uint8                 *is_interdigitated
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_queue_to_flow_mapping_set
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  queue_quartet_ndx -
*     queue_quartet_ndx - The index of the quartet to be
*     configured, the range of queues in the quartet is:
*     (queue_quartet_ndx * 4) to (queue_quartet_ndx * 4 + 3)
*     DNX_SAND_IN
*  DNX_SAND_IN  JER2_ARAD_IPQ_QUARTET_MAP_INFO *info -
*     Pointer to mapping configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_queue_to_flow_mapping_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_quartet_ndx,
    DNX_SAND_IN  JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_queue_to_flow_mapping_get
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  queue_quartet_ndx -
*     queue_quartet_ndx - The index of the quartet to be
*     configured, the range of queues in the quartet is:
*     (queue_quartet_ndx * 4) to (queue_quartet_ndx * 4 + 3)
*     DNX_SAND_IN
*  DNX_SAND_OUT JER2_ARAD_IPQ_QUARTET_MAP_INFO *info -
*     Pointer to mapping configuration structure.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_queue_to_flow_mapping_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_quartet_ndx,
    DNX_SAND_OUT JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  );

/*********************************************************************
* NAME:
*   jer2_arad_ipq_queue_qrtt_unmap
* TYPE:
*   PROC
* FUNCTION:
*   Unmap a queues-quartet, by mapping it to invalid
*   destination. Also, flush all the queues in the quartet.
* INPUT:
*   DNX_SAND_IN  int unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  uint32  queue_quartet_ndx -
*     The index of the queue quartet, the range of queues in
*     the quartet is: (queue_quartet_ndx * 4) to
*     (queue_quartet_ndx * 4 + 3) Range: 0 - 8K-1.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_queue_qrtt_unmap(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core,
    DNX_SAND_IN  uint32  queue_quartet_ndx
  );

/*********************************************************************
* NAME:
*     jer2_arad_ipq_quartet_reset
* TYPE:
*   PROC
* DATE:
*   Oct  3 2007
* FUNCTION:
*     Resets a quartet to default values.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*  DNX_SAND_IN  uint32                  queue_quartet_ndx -
*     queue_quartet_ndx - The 4 queues currently configured
*     are: (queue_quartet_ndx * 4) - (queue_quartet_ndx * 4 +
*     3)
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_ipq_quartet_reset(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_quartet_ndx
  );

uint32
  jer2_arad_ipq_k_quartet_reset(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                  queue_k_quartet_ndx,
    DNX_SAND_IN uint32                  region_size
  );

void
  jer2_arad_JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  );

void
  jer2_arad_JER2_ARAD_IPQ_QUARTET_MAP_INFO_clear(
    DNX_SAND_OUT JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  );

uint32
  jer2_arad_ipq_attached_flow_port_get(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  int core,
    DNX_SAND_IN  uint32 queue_ndx,
    DNX_SAND_OUT uint32 *flow_id,
    DNX_SAND_OUT uint32 *sys_port
  );

uint32
  jer2_arad_ipq_tc_profile_set(
    DNX_SAND_IN	  int	  unit,
    DNX_SAND_IN   int     core,
    DNX_SAND_IN	  uint8	  is_flow_ndx,
    DNX_SAND_IN	  uint32	  dest_ndx,
    DNX_SAND_IN   uint32	  tc_profile
  );

uint32
  jer2_arad_ipq_tc_profile_get(
    DNX_SAND_IN	  int	  unit,
    DNX_SAND_IN   int     core,
    DNX_SAND_IN	  uint8	  is_flow_ndx,
    DNX_SAND_IN	  uint32	  dest_ndx,
    DNX_SAND_OUT  uint32	  *tc_profile
  );

uint32
  jer2_arad_ipq_tc_profile_map_set(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core_id,
    DNX_SAND_IN  uint32                profile_ndx,
    DNX_SAND_IN  uint8                is_flow_profile,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
  );
uint32
  jer2_arad_ipq_tc_profile_map_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                core_id,
    DNX_SAND_IN  uint32                profile_ndx,
    DNX_SAND_IN  uint8                is_flow_profile,
    DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
    DNX_SAND_OUT  JER2_ARAD_IPQ_TR_CLS         *new_class
  );

#if JER2_ARAD_DEBUG_IS_LVL1

const char*
  jer2_arad_JER2_ARAD_IPQ_TR_CLS_RNG_to_string(
    DNX_SAND_IN JER2_ARAD_IPQ_TR_CLS_RNG enum_val
  );


void
  jer2_arad_JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_print(
    DNX_SAND_IN JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  );



void
  jer2_arad_JER2_ARAD_IPQ_QUARTET_MAP_INFO_print(
    DNX_SAND_IN JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
  );

uint32
  jer2_arad_ips_non_empty_queues_info_get(
    DNX_SAND_IN  int                unit,
    DNX_SAND_IN  int                   core,
    DNX_SAND_IN  uint32                first_queue,
    DNX_SAND_IN  uint32                max_array_size,
    DNX_SAND_OUT dnx_soc_ips_queue_info_t* queues,
    DNX_SAND_OUT uint32*               nof_queues_filled,
    DNX_SAND_OUT uint32*               next_queue,
    DNX_SAND_OUT uint32*               reached_end
  );



#endif /* JER2_ARAD_DEBUG_IS_LVL1 */


/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_API_INGRESS_PACKET_QUEUING_INCLUDED__*/
#endif


