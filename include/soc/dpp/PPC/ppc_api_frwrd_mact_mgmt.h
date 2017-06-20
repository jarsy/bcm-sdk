/* $Id: ppc_api_frwrd_mact_mgmt.h,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_frwrd_mact_mgmt.h
*
* MODULE PREFIX:  soc_ppc_frwrd
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

#ifndef __SOC_PPC_API_FRWRD_MACT_MGMT_INCLUDED__
/* { */
#define __SOC_PPC_API_FRWRD_MACT_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/dpp/PPC/ppc_api_frwrd_mact.h>
#include <soc/dpp/TMC/tmc_api_packet.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of longs in the message header (DSP header).     */
#define  SOC_PPC_FRWRD_MACT_MSG_HDR_SIZE (8)

/*     Maximum size of the event buffer in longs               */
#define  SOC_PPC_FRWRD_MACT_EVENT_BUFF_MAX_SIZE (5)

/* use to indicate no limit on numnber MACT entries see glbl_limit    */
#define  SOC_PPC_FRWRD_MACT_NO_GLOBAL_LIMIT (0xFFFFFFFF)

/* Max number of LIF ranges that can be mapped for LIF Learn Limit enforcing */
#define  SOC_PPC_MAX_NOF_MACT_LIMIT_LIF_RANGES          (4)

/* Max number of LIF ranges that are mapped for LIF Learn Limit enforcing */
#define  SOC_PPC_MAX_NOF_MACT_LIMIT_MAPPED_LIF_RANGES   (SOC_DPP_MAX_NOF_MACT_LIMIT_MAPPED_LIF_RANGES)

/*  L2 MACT Learn Limit Mode */
#define  SOC_PPC_FRWRD_MACT_LEARN_LIMIT_MODE            ((SOC_DPP_CONFIG(unit))->l2.learn_limit_mode)

/* Identify whether is VMAC limit or MAC limit by bit 15 */
#define  SOC_PPC_FRWRD_MACT_LEARN_VMAC_LIMIT            (0x1 << 15)

/* Max number of VMAC limit */
#define  SOC_PPC_FRWRD_MACT_LEARN_MAX_VMAC_LIMIT        (16)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/*  L2 MACT Learn Limit LIF Range number (SOC_PPC_MAX_NOF_MACT_LIMIT_MAPPED_LIF_RANGES) for one of the mapped ranges */
#define  SOC_PPC_FRWRD_MACT_LEARN_LIF_RANGE_BASE(range_num)     ((SOC_DPP_CONFIG(unit))->l2.learn_limit_lif_range_base[range_num])

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  The learning occurs in the ingress device, distributed
   *  mode.
   */
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_INGRESS_DISTRIBUTED = 0,
  /*
   *  The learning occurs in the ingress device, centralized
   *  (managed) mode.
   */
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_INGRESS_CENTRALIZED = 1,
  /*
   *  The learning occurs in the egress device, distributed
   *  mode.
   */
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_EGRESS_DISTRIBUTED = 2,
  /*
   *  The learning occurs in the egress device, centralized
   *  (managed) mode.
   */
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_EGRESS_CENTRALIZED = 3,
  /*
   *  The learning occurs in the egress device, independent
   *  mode.
   */
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_EGRESS_INDEPENDENT = 4,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_LEARNING_MODE
   */
  SOC_PPC_NOF_FRWRD_MACT_LEARNING_MODES= 5
}SOC_PPC_FRWRD_MACT_LEARNING_MODE;

typedef enum
{
  /*
   *  No shadow is maintained by the CPU, and the CPU is not
   *  informed of changes in the MACT.
   */
  SOC_PPC_FRWRD_MACT_SHADOW_MODE_NONE = 0,
  /*
   *  The shadow maintained by the CPU is for ARP management
   *  (for routing); in this case the CPU is informed upon
   *  age-out and transplant events, but not upon Learn
   *  events. Learn events are here not relevant because the
   *  new entries ae inserted to the MACT by the control-plan
   *  upon ARP requests.
   */
  SOC_PPC_FRWRD_MACT_SHADOW_MODE_ARP = 1,
  /*
   *  The shadow maintained by the CPU is for LAG management.
   *  In this case, the CPU is informed upon learn/transplant
   *  and aged out events that were generated due to packets
   *  whose Destination port is a LAG.
   */
  SOC_PPC_FRWRD_MACT_SHADOW_MODE_LAG = 2,
  /*
   *  The shadow maintained by the CPU is fully synchronized
   *  with the MACT of the hardware. In this case CPU is
   *  informed upon age-out, transplant and learn events. This
   *  mode may be used when the CPU supervises the Hardware
   *  MACT.
   */
  SOC_PPC_FRWRD_MACT_SHADOW_MODE_ALL = (int)0xFFFFFFFF,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_SHADOW_MODE
   */
  SOC_PPC_NOF_FRWRD_MACT_SHADOW_MODES = 4
}SOC_PPC_FRWRD_MACT_SHADOW_MODE;

typedef enum
{
  /*
   *  Learning messages are injected into the local device as
   *  raw packets. The packet is forwarded according to port
   *  configuration (according to the ITMH header associated
   *  with the port). The learn message leaves the device as a
   *  raw packet that includes the learn records.in this mode
   *  both the shadow messages and learn messages will have
   *  the same header/destination.
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_RAW = 0,
  /*
   *  Learning messages are injected into the local device
   *  with ITMH header the packet is processed/forwarded in
   *  the device as TM packet (according to the associated
   *  ITMH header). The learn message leaves the device as a
   *  raw packet that includes the learn records.in order to
   *  build ITMH header use tmd_hpu_itmh_build()
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_ITMH = 1,
  /*
   *  Learning messages are injected into the local device
   *  with ITMH header the packet is processed/forwarded in
   *  the device as a TM packet (according to the associated
   *  ITMH header). The learn message leaves the device as an
   *  Ethernet packet that includes the learn records.
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_ETH_O_ITMH = 2,
  /*
   *  Messages towards the OLP might include an OTMH header that the OLP ignores
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_OTMH = 3,
  /*
   *  Messages towards the OLP. Ethernet over OTMH
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_ETH_O_OTMH = 4,
  /*
   *  Messages towards the OLP. Ethernet over raw payload
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_ETH_O_RAW = 5,
  /*
   *  Like SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_ETH_O_ITMH just with a vlan in the ethernet header
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_ETH_VLAN_O_ITMH = 6,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_MSG_HDR_TYPES = 7
}SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE;

typedef enum
{
  /*
   *  No notification is performed.
   */
  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE_NONE = 0,
  /*
   *  Raise an interrupt. Relevant only for Soc_petra-B.
   */
  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE_INTERRUPT = 1,
  /*
   *  Send a message. Relevant only for Soc_petra-B.
   */
  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE_MSG = 2,
  /*
   *  Generate a special event and add it to the MACT event
   *  FIFO. Use
   *  soc_ppd_frwrd_mact_event_get()/soc_ppd_frwrd_mact_event_parse()
   *  to catch this event. Relevant only for T20E.
   */
  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE_EVENT = 4,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPES = 4
}SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE;

typedef enum
{
  /*
   *  Trap performed for SA with drop_known.
   */
  SOC_PPC_FRWRD_MACT_TRAP_TYPE_SA_DROP = 0,
  /*
   *  Trap performed for unknown SA.
   */
  SOC_PPC_FRWRD_MACT_TRAP_TYPE_SA_UNKNOWN = 1,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_TRAP_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_TRAP_TYPES = 2
}SOC_PPC_FRWRD_MACT_TRAP_TYPE;

typedef enum
{
  /*
   *  Not valid event
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_NONE = 0x0,
  /*
   *  Event indicates that the entry is subject to be aged
   *  out. Note: using
   *  soc_ppd_frwrd_mact_aging_events_handle_info_set the user can
   *  define whether the aging process: - generate aged out
   *  events, - handle internally the aged-out entry and
   *  delete it,- both.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_AGED_OUT = 0x1,
  /*
   *  This event indicates that the entry is subject to be
   *  learned.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_LEARN = 0x2,
  /*
   *  This event indicates that the entry is subject to be
   *  transplanted, i.e. its key already exists in the MACT
   *  but the packet arrived on a different interface.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_TRANSPLANT = 0x4,
  /*
   *  This event indicates that the entry is subject to be
   *  refreshed, i.e. its key and its value already exist in
   *  the MACT but its age status has been reset due to the
   *  arrival of a new packet from the same interface. Note:
   *  using soc_ppd_frwrd_mact_aging_events_handle_info_set the
   *  user can define whether the aging process generates
   *  refresh events or not.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_REFRESH = 0x8,
  /*
   *  MACT entry returned upon CPU request using
   *  soc_ppd_frwrd_mact_traverse() with action type RETRIEVE. T20E
   *  only.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_RETRIEVE = 0x10,
  /*
   *  Insertion to MACT failed due to exceeding the limit
   *  assigned to the particular VSI. T20E only.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_LIMIT_EXCEED = 0x20,
  /*
   *  Acknowledgment event upon CPU request
   *  (insert/learn/delete).
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_ACK = 0x40,
  /*
   *  Request to the MACT has failed.
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_REQ_FAIL = 0x80,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_EVENT_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_EVENT_TYPES = 9
}SOC_PPC_FRWRD_MACT_EVENT_TYPE;

typedef enum
{
  /*
   *  Learn information transmitted through OLP messages
   *  composed of single learned information. Small packets
   *  are sent frequently. In this case, the header information
   *  of the messages must be set.
   */
  SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE_MSG_SINGLE = 0,
  /*
   *  Learn information transmitted through OLP messages
   *  composed of aggregated learned information. For
   *  aggregated messages, larger packets are sent less
   *  frequently. In this case, the header information of the
   *  messages must be set.
   */
  SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE_MSG_AGGR = 1,
  /*
   *  Learn information obtained through direct access to
   *  device using soc_ppd_frwrd_mact_event_get(). Can be used
   *  only in a Centralized learning mode.
   */
  SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE_DIRECT_ACCESS = 2,
  /*
   *  Number of types in SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE
   */
  SOC_PPC_NOF_FRWRD_MACT_EVENT_PATH_TYPES = 3
}SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE;

typedef enum
{
    /*
     * SA lookup type = Mac-in-Mac 
     * Invalid for ARAD. 
     */
    SOC_PPC_FRWRD_MACT_LOOKUP_TYPE_MIM = 0,

    /*
     * SA lookup type = SA authentication
     */
    SOC_PPC_FRWRD_MACT_LOOKUP_TYPE_SA_AUTH = 1,

    /*
     * SA lookup type = SA lookup 
     */
    SOC_PPC_FRWRD_MACT_LOOKUP_TYPE_SA_LOOKUP = 2,
#ifdef BCM_88660_A0
    /*
     * SLB lookup
     */
    SOC_PPC_FRWRD_MACT_LOOKUP_TYPE_SLB_LOOKUP = 3,
#endif /* BCM_88660_A0 */
    /*
     * Number of types in SOC_PPC_FRWRD_MACT_LOOKUP_TYPE
     */
    SOC_PPC_NOF_FRWRD_MACT_LOOKUP_TYPES
} SOC_PPC_FRWRD_MACT_LOOKUP_TYPE;


typedef enum
{
    /*
     * MACT Learn limit type = VSI
     */
    SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE_VSI = 0,
    /*
     * MACT Learn limit type = LIF
     */
    SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE_LIF = 1,
    /*
     * MACT Learn limit type = TUNNEL
     */
    SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE_TUNNEL = 2,
    /*
     *  Number of types in SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE
     */
    SOC_PPC_NOF_FRWRD_MACT_LEARN_LIMIT_TYPES = 3
} SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE;


typedef enum
{
  /*
   * Enable MACT learning for IPv4 UC packets
   */
  SOC_PPC_FRWRD_MACT_L3_LEARN_IPV4_UC = 0x1,
  /*
   * Enable MACT learning for IPv4 MC packets
   */
  SOC_PPC_FRWRD_MACT_L3_LEARN_IPV4_MC = 0x2,
  /*
   * Enable MACT learning for IPv6 UC packets
   */
  SOC_PPC_FRWRD_MACT_L3_LEARN_IPV6_UC = 0x4,
  /*
   * Enable MACT learning for IPv6 MC packets
   */
  SOC_PPC_FRWRD_MACT_L3_LEARN_IPV6_MC = 0x8,
  /*
   * Enable MACT learning for MPLS packets
   */
  SOC_PPC_FRWRD_MACT_L3_LEARN_MPLS = 0x10
} SOC_PPC_FRWRD_MACT_ROUTED_LEARNING_APP_TYPE;
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  When (1) IPv4 MC packet over Ethernet is identified as
   *  compatible MC, i.e the DA is in the range of compatible
   *  MC addresses (see below),(2) IP Multicast Routing is
   *  disabled on the in-RIF (see soc_ppd_rif_vsid_map_set,
   *  SOC_PPC_RIF_INFO) Then - If this field is set to TRUE then
   *  the MACT is accessed with the packet DIP and FID (FID
   *  may be masked) (i.e., the packet is bridged)- If this
   *  field is set to FALSE then the MACT is accessed with the
   *  MAC address and FID (a normal bridging occurs).
   *  Remarks:- The compatible MC MAC addresses range is
   *  01:00:5e:00:00:00 - 01:00:5e:7f:ff:ff.- on (2) above, if
   *  the Multicast routing is enabled then the packet is
   *  routed 'normally' - for IPv6 compatible packets, if the
   *  IP Multicast Routing is enabled on the in-RIF then the
   *  packet is routed, otherwise the packet is bridged
   *  according to its MAC address and its FID (normal
   *  bridging)
   */
  uint8 enable_ipv4_mc_compatible;
  /*
   *  If TRUE, then the lookup key (in the MACT) is DIP. If
   *  FALSE, then the lookup key is (FID,DIP).
   */
  uint8 mask_fid;

} SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, then enable the aging process of the device
   *  (aging machine).
   */
  uint8 enable_aging;
  /*
   *  Dynamic entries that were not refreshed for this amount
   *  of time will be aged-out from the MACT. Note: in a
   *  distributed mode, an entry inserted by the same device
   *  (and not by a learn message from another device) will be
   *  aged-out after half 'aging_time'.
   */
  SOC_PPC_FRWRD_MACT_TIME aging_time;

} SOC_PPC_FRWRD_MACT_AGING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

} SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, then enable the aging process of the device
   *  (aging machine). User can handle in one mapping more
   *  than one event-type: for
   *  examplePPD_FRWRD_MACT_EVENT_TYPE_LEARN |
   *  SOC_PPC_FRWRD_MACT_EVENT_TYPE_REFRESH
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE event_type;
  /*
   *  Indicates that this Event was generated from a packet
   *  whose Destination port is a LAG. Relevant only for egress
   *  learning, in ingress learning has to be FALSE.
   */
  uint8 is_lag;
  /*
   *  VSI Profile that is used to set different event handling
   *  according to VSI. Use
   *  soc_ppd_frwrd_mact_event_handle_profile_set() to set the fid
   *  event handle profile.
   *  Soc_petra-B: Range: 0 - 1.
   *  Arad: Range:0 - 3.
   */
  uint32 vsi_event_handle_profile;

} SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If set to TRUE then a 'copy' of the event is looped-back
   *  into the device and handled internally i.e
   *  learned/deleted/refreshed... according to the event type
   */
  uint8 self_learning;
  /*
   *  If set to TRUE then a 'copy' of the event is sent to the
   *  learning FIFO. Generally this FIFO is used to handle
   *  learning events. In centralized mode, this fifo is
   *  expected to send events to the Cental CPU. To set how to
   *  distribute events arriving to this FIFO, use:
   *  soc_ppd_frwrd_mact_sys_learn_msgs_distribution_info_set()
   */
  uint8 send_to_learning_fifo;
  /*
   *  If set to TRUE then a 'copy' of the event is sent to
   *  shadow FIFO. Generally this FIFO is used to inform the
   *  control plane upon 'interesting' events for monitoring.
   *  See SOC_PPC_FRWRD_MACT_SHADOW_MODE. However the user can use
   *  this FIFO to perform an auto system learning. To set how
   *  to distribute events arriving to this FIFO, use:
   *  soc_ppd_frwrd_mact_shadow_msgs_distribution_info_set()
   */
  uint8 send_to_shadow_fifo;
  /*
   *  If set to TRUE then a 'copy' of the event is sent to
   *  dma FIFO. From this FIFO a DMA copies the events to the
   *  host's memory. From the host's memory the driver parses
   *  the events and calls the handling callback of the user
   */
  uint8 send_to_cpu_dma_fifo;

} SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE then the aging process internally deletes aged
   *  out entries.
   */
  uint8 delete_internally;
  /*
   *  If TRUE then the aging process generates events for aged
   *  out entries. Should be set to TRUE for centralized mode
   */
  uint8 event_when_aged_out;
  /*
   *  If TRUE then the aging process generates events for
   *  entries refreshed in the last traverse of the MACT.
   */
  uint8 event_when_refreshed;
  /*
   *  If TRUE then when entry is readen clear the hit bit
   *  Arad only.
   */
  uint8 clear_hit_bit;

} SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Type of header coming from the program editor to the OLP.
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE olp_receive_header_type;
  /*
   *  Type of the header. According to this header the OLP
   *  sends the learning messages to other devices/CPU. I.e.
   *  this header is attached to the Learning messages. Not
   *  Relevant for T20E.
   */
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE header_type;
  /*
   *  Header that this device uses in order to send messages
   *  to other devices or the central CPU. Network Order:
   *  First uint32 of the header is header[0] and first bit of
   *  the header is the lsb of header[0]. If not all the uint32
   *  is used, then start from the lsb bits and reset the rest
   *  of the bits of the uint32. The content of this header is
   *  up to the header_type as follows:- raw: content of
   *  header is not considered. - OLP port type has to be Raw
   *  - TM: ITMH header - OLP port type has be TM - Eth:
   *  Ethernet header. In this case, the Ethernet type is not
   *  considered and is replaced with dsp_type.- OLP port type
   *  has to be Ethernet, and it has to be member in the VLAN
   *  which is equal to its VID. Not Relevant for T20E.
   */
  uint32 header[SOC_PPC_FRWRD_MACT_MSG_HDR_SIZE];

} SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Transmission type of the learning information. Can be
   *  through OLP messages or direct access. The shadow
   *  information must have the same type as the learning
   *  information. Not Relevant for T20E.
   */
  SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE type;
  /*
   *  OLP Message header parameters. Relevant only if the type
   *  is 'MSG_SINGLE' or 'MSG_AGG'. Not Relevant for T20E.
   */
  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO info;

} SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Mode of learning (ingress/egress,
   *  centralized/distributed, independent)
   */
  SOC_PPC_FRWRD_MACT_LEARNING_MODE learning_mode;
  /*
   *  If TRUE, then the MACT operates in compatible mode for
   *  Soc_petra-A. In this case, egress learning is not supported.
   */
  uint8 soc_petra_a_compatible;
  /*
   *  Specifies how the device will handle/distribute learning
   *  information:In Distributed Mode: Specifies how the
   *  device will share the MACT information with other
   *  devices through OLP messages. In Centralized Mode:
   *  Specifies how the managing device will get the learning
   *  information - OLP messages, or - direct register access
   *  only applicable for CPU management)). Not Relevant for
   *  T20E.
   */
  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO learn_msgs_info;
  /*
   *  The mode to build the shadow of the MACT by the CPU. In
   *  other words the subset of events to inform the CPU.
   */
  SOC_PPC_FRWRD_MACT_SHADOW_MODE shadow_mode;
  /*
   *  Specifies how the device will send events (for shadow)
   *  according to shadow mode to central CPU or other
   *  devicesNot Relevant for T20E.
   */
  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO shadow_msgs_info;
  /*
   *  If enabled, learning is done in the ELK. Internal
   *  MACT may still be used for insertion of static MAC
   *  addresses. If disabled, learning is done in the MACT.
   *  ELK may still be used for insertion of static MAC
   *  addresses.
   */
  uint8 learn_in_elk;
  /*
   *  Disable learning.
   *    - Disable learning lookups for Ethernet packets.
   *    - Thus no generation of learn/transplant/delete events.
   *    - Age-out event still may generate if aging is enabled                                                      .
   *  Arad only.
   */
  uint8 disable_learning;

} SOC_PPC_FRWRD_MACT_OPER_MODE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, a limitation is applied on this FID according
   *  to its profile.
   *  ARAD: has to be TRUE, as no per FID disable limit,
   *  limit can be enabled/disable only globally.
   */
  uint8 is_limited;
  /*
   *  Maximum number of entries that can be learned for each
   *  FID using this profile. Whether the limitation includes
   *  static entries or not is a global configuration.
   *  Soc_petra-B: Range: 0 - 0x7FFE.
   *  Arad: Range:0 - 64K
   */
  uint32 nof_entries;
  /*
   *  The action to perform when trying to exceed the limit
   *  set to this FID.
   */
  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE action_when_exceed;

} SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The FID of the last request that failed due to the FID
   *  entry limit.
   */
  SOC_PPC_FID fid_fail;
  /*
   *  The FID of the last request with a static payload that
   *  was accepted even if the FID entry limit was exceeded.
   *  The MACT may be configured to allow static entries to
   *  exceed limit, see
   *  soc_ppd_frwrd_mact_mac_limit_glbl_info_set().
   */
  SOC_PPC_FID fid_allowed;

} SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, then limit the number of MAC addresses learned
   *  by the FID. Otherwise, no limitation is enforced. This
   *  is a global Flag for all the MAC limit mechanism. The
   *  user can use the MACT limitation per FID/VSI subsets,
   *  see soc_ppd_frwrd_mact_mac_limit_profile_info_set().
   */
  uint8 enable;
  /*
   *  If TRUE, then adding a static entry to the MAC Table is
   *  allowed, even if the limit of the FID it belongs to is
   *  exceeded.
   *  in Arad: this also means to exceed global limit
   */
  uint8 static_may_exceed;
  /*
   *  If TRUE, then when the MACT limit is exceeded an event
   *  is generated. Relevant only for T20E. in Soc_petra-B this
   *  may be configured per limit profile, see
   *  soc_ppd_frwrd_mact_mac_limit_profile_info_set()
   */
  uint8 generate_event;
  /*
   *  limit on the total number of MAC addresses.
   *  use SOC_PPC_FRWRD_MACT_NO_GLOBAL_LIMIT to indicate no limt.
   *  Range: 0 - 256K                                                      .
   *  Arad only.
   */
  uint32 glbl_limit;
  /*
   *  If TRUE, then adding an entry from CPU is
   *  allowed, even if the limit of the FID it belongs to is
   *  exceeded. or if global limit is exceeded
   *  Arad only.
   */
  uint8 cpu_may_exceed;
  /*
   *  For T20E only, FIDs in the range [fid-base,
   *  fid-base+16k] may be limited and assigned to a limited
   *  profile. Other entries may not be limited. For Soc_petra-B,
   *  has to be zero.
   */
  SOC_PPC_FID fid_base;

} SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   *  The number of MSBs to reset in the mapped value.
   */
  int8  zero_msbs;
  /* 
   *  The number of bits to shift right the mapped value after the
   *  MSBs reset.
   */
  int8  shift_right_bits;
  /* 
   *  The base value to add, after the MSBs reset and the shift right.
   */
  int16 base_add_val; 
} SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   *  A pointer to an entry in the Limit table, that is used as default
   *  for all the objects that don't have valid MACT Limit configuration.
   */
  uint32 invalid_ptr;
  /* 
   *  Range limits to distinguish between mapping ranges.
   *  An object value smaller than the first range end value belongs to the first range,
   *  else an object value smaller than the second range end value belongs to the second
   *  range, etc.
   */
  int32 range_end[SOC_PPC_MAX_NOF_MACT_LIMIT_LIF_RANGES - 1];
} SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Set the profile per port for known SA MACs when
   *  drop_when_sa_is_known is set to TRUE. This profile is
   *  mapped to the action profile by
   *  soc_ppd_frwrd_mact_trap_info_set(). Range: 0 - 3.
   */
  uint32 sa_drop_action_profile;
  /*
   *  Set the profile per port for unknown SA MACs. This
   *  profile is mapped to the action profile by
   *  soc_ppd_frwrd_mact_trap_info_set(). Range: 0 - 3.
   */
  uint32 sa_unknown_action_profile;
  /*
   *  Set the profile per port for unknown DA MACs. This
   *  profile is mapped to the action profile by
   *  soc_ppd_frwrd_mact_trap_info_set(). Range: 0 - 3.
   */
  uint32 da_unknown_action_profile;

} SOC_PPC_FRWRD_MACT_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the original packet destination is part of
   *  LAG. The other devices member of this LAG should learn
   *  also this entry.
   */
  uint8 is_lag;
  /*
   *  Destination System-LAG Port id of the destination (DA)
   *  of the packet whose source (SA) has been learnt. Range: 0
   *  - 255.
   */
  uint32 id;

} SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Event type
   */
  SOC_PPC_FRWRD_MACT_EVENT_TYPE type;
  /*
   *  Event key information
   */
  SOC_PPC_FRWRD_MACT_ENTRY_KEY key;
  /*
   *  Event entry value (payload and age information)
   */
  SOC_PPC_FRWRD_MACT_ENTRY_VALUE value;
  /*
   *  LAG information
   */
  SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO lag;

} SOC_PPC_FRWRD_MACT_EVENT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Buffer to copy the Event to. Event is copied to buff
   *  starting from buff[0] lsb.
   */
  uint32 buff[SOC_PPC_FRWRD_MACT_EVENT_BUFF_MAX_SIZE];
  /*
   *  the actual length of the returned buffer (in longs)
   */
  uint32 buff_len;

} SOC_PPC_FRWRD_MACT_EVENT_BUFFER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * learn message content, network order
   */
  uint8  *msg_buffer;
  /* 
   * number of byte
   */
  uint32   msg_len;
  /* 
   * maximal number of events to parse. PB: maximum number of events per message is 8
   */
  uint8 max_nof_events;

} SOC_PPC_FRWRD_MACT_LEARN_MSG;


  /* 
   * This structure is in used in per device API, *_frwrd_mact_learn_msg_parse()
   * maybe filled/calculated either by calling *_frwrd_mact_learn_msg_conf_get
   * or according to caller logic.
   */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * size of message header ,including system headers, Ethernet,...
   * till the start of learning message.
   * Note: in case learning messages are over Eth header then EtherType
   * is consider as part of the learning message and not header.
   */
  uint32 header_size;
  /* 
   * the mode the learn message was read from the device
   * byte/byte reception, msb to lsb, or lsb to msb, 
   */
  SOC_TMC_PKT_PACKET_RECV_MODE recv_mode; 

  /* 
   * information needed to parse the learning messages
   */
} SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* 
   * event information
   */
  SOC_PPC_FRWRD_MACT_EVENT_INFO* events;
  /* 
   * number of events parsed and returned in events[]
   */
  uint32 nof_parsed_events;
  /* 
   * total number of events in message
   */
  uint32 nof_events_in_msg;
  /* 
   * the FAP id that generate the event
   */
  uint32 source_fap;
  
} SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO;


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
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_AGING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE *info
  );

void
  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_OPER_MODE_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_PORT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_PORT_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_BUFFER_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_EVENT_BUFFER *info
  );

void
  SOC_PPC_FRWRD_MACT_LEARN_MSG_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_LEARN_MSG *info
  );

void
  SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF *info
  );

void
  SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_FRWRD_MACT_LEARNING_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_LEARNING_MODE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_SHADOW_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_SHADOW_MODE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_HDR_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_LIMIT_EXCEED_NOTIFY_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_TRAP_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_TRAP_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_EVENT_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_TYPE enum_val
  );

const char*
  SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_PATH_TYPE enum_val
  );

void
  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_IP_COMPATIBLE_MC_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_AGING_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_ONE_PASS_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_KEY *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_HANDLE_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_AGING_EVENTS_HANDLE *info
  );

void
  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MSG_DISTR_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_PROCESSING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_OPER_MODE_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_EXCEEDED_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_GLBL_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_RANGE_MAP_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_MAC_LIMIT_MAPPING_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_PORT_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_PORT_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_LAG_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_INFO_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_INFO *info
  );

void
  SOC_PPC_FRWRD_MACT_EVENT_BUFFER_print(
    SOC_SAND_IN  SOC_PPC_FRWRD_MACT_EVENT_BUFFER *info
  );

void
  SOC_PPC_FRWRD_MACT_LEARN_MSG_print(
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_LEARN_MSG *info
  );

void
  SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF_print(
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_LEARN_MSG_CONF *info
  );

void
  SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO_print(
    SOC_SAND_IN SOC_PPC_FRWRD_MACT_LEARN_MSG_PARSE_INFO *info
  );
#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_FRWRD_MACT_MGMT_INCLUDED__*/
#endif
