/* $Id: jer2_jer2_jer2_tmc_api_ingress_packet_queuing.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/jer2_jer2_jer2_tmc/include/soc_jer2_jer2_jer2_tmcapi_ingress_packet_queuing.h
*
* MODULE PREFIX:  soc_jer2_jer2_jer2_tmcipq
*
* FILE DESCRIPTION:
*
* REMARKS:
* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
******************************************************************/

#ifndef __DNX_TMC_API_INGRESS_PACKET_QUEUING_INCLUDED__
/* { */
#define __DNX_TMC_API_INGRESS_PACKET_QUEUING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Utils/sand_integer_arithmetic.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define DNX_TMC_IPQ_INVALID_FLOW_QUARTET    0x3fff
/* } */

/*************
 * MACROS    *
 *************/
/* { */
#define DNX_TMC_IPQ_Q_TO_QRTT_ID(que_id) ((que_id)/4)
#define DNX_TMC_IPQ_QRTT_TO_Q_ID(q_que_id) ((q_que_id)*4)
#define DNX_TMC_IPQ_Q_TO_1K_ID(que_id) ((que_id)/1024)
#define DNX_TMC_IPQ_1K_TO_Q_ID(k_que_id) ((k_que_id)*1024)

/* Max & min values for Stak Lag and base_queue:      */
#define DNX_TMC_IPQ_STACK_LAG_DOMAIN_MIN     0
#define DNX_TMC_IPQ_STACK_LAG_DOMAIN_MAX     64

#define DNX_TMC_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MIN     0
#define DNX_TMC_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MAX     (SOC_IS_JERICHO(unit)? 256 : 64)
#define DNX_TMC_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_ALL     0xffffffff

#define DNX_TMC_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MIN     0
#define DNX_TMC_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MAX     4
#define DNX_TMC_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL     0xffffffff

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/*     IPQ Queue type - Traffic Class. Range: 0-7        */
typedef uint32 DNX_TMC_IPQ_TR_CLS;

typedef enum
{
  /*
   *  Ingress-Packet-traffic-class: Value 0
   */
  DNX_TMC_IPQ_TR_CLS_MIN=0,
  /*
   *  Ingress-Packet-traffic-class: Value 7
   */
  DNX_TMC_IPQ_TR_CLS_MAX=7,
  /*
   *  Must be the last value
   */
  DNX_TMC_IPQ_TR_CLS_RNG_LAST
}DNX_TMC_IPQ_TR_CLS_RNG;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Explicit queue ID packet Base queue number. Range 0 to
   *  nof_queues.
   */
  uint32 base_queue_id;
  /*
   *  TRUE - queue is 'base_queue_id' + Packet's explicit
   *  queue ID (add).
   *  FALSE - queue is 'base_queue_id' - Packet's
   *  explicit queue ID (subtract).
   */
  uint8 queue_id_add_not_decrement;
}DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO;
typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  If set the specific queue is valid, Otherwise the queue
   *  is set to be invalid and packets are not sent to it.
   */
  uint8 valid;
  /*
   *  Packet is stored in queue: base_queue + class. Range: 0
   *  - 32K-1.
   */
  uint32 base_queue;

} DNX_TMC_IPQ_BASEQ_MAP_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  The index of the first flow quartet to which the queue
   *  quartet points. A single queue quartet might point to 1
   *  quartets of flows, 2 quartets of flows or 4 quartets of
   *  flow, depending on the 'Interdigitated' and 'Composite'
   *  modes. Range: 0 - 14K-1.
   */
  uint32 flow_quartet_index;
  /*
   *  When TRUE, the flow quartets that this queue quartet
   *  points to, might use composite flows. The user might
   *  configure the flows to use only one sub-flow, but the
   *  second sub-flow is still reserved as far as the
   *  flow-queues mapping is concerned.
   */
  uint8 is_composite;
  /*
   *  Used only in direct base queue to mod x port mapping.
   *  Used as input to _ipq_queue_to_flow_mapping_set.
   *  On a Set the input will be in fap_id and in fap_port_id and not in system_physical_port. 
   *  On a Get the output will be in fap_id and in fap_port_id; and
   *  system_physical_port will be set to a real value if a mapping is available, or to -1.
   *  Boolean value, used only in JER2_ARAD.
   */
  uint8 is_modport;
  /*
   *  The system physical port to which the queue quartet is
   *  destined. Refer to function
   *  soc_jer2_jer2_jer2_tmcsys_phys_to_local_port_map_set for more info.
   *  _ipq_queue_to_flow_mapping_get called in direct mapping mode will return -1 here if a sysport mapping is not found
   *  Range: 0 - 4095.
   */
  uint32 system_physical_port;
  /*
   *  Not in use in Soc_petra, and used only in direct base queue to mod x port mapping.
   *  The FAP and FAP port to which the queue quartet is
   *  destined. Refer to function
   *  Range: 0 - 4095.
   */
  uint16 fap_id;
  uint16 fap_port_id;
}DNX_TMC_IPQ_QUARTET_MAP_INFO;

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
  DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  );

void
  DNX_TMC_IPQ_BASEQ_MAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_IPQ_BASEQ_MAP_INFO *info
  );

void
  DNX_TMC_IPQ_QUARTET_MAP_INFO_clear(
    DNX_SAND_OUT DNX_TMC_IPQ_QUARTET_MAP_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_IPQ_TR_CLS_RNG_to_string(
    DNX_SAND_IN DNX_TMC_IPQ_TR_CLS_RNG enum_val
  );

void
  DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO_print(
    DNX_SAND_IN DNX_TMC_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
  );

void
  DNX_TMC_IPQ_BASEQ_MAP_INFO_print(
    DNX_SAND_IN  DNX_TMC_IPQ_BASEQ_MAP_INFO *info
  );

void
  DNX_TMC_IPQ_QUARTET_MAP_INFO_print(
    DNX_SAND_IN DNX_TMC_IPQ_QUARTET_MAP_INFO *info
  );

void
  soc_jer2_jer2_jer2_tmcips_non_empty_queues_print(
    DNX_SAND_IN  int   unit,
    DNX_SAND_IN  uint32   print_first_local_flow
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_INGRESS_PACKET_QUEUING_INCLUDED__*/
#endif
