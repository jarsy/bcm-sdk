/* $Id: tmc_api_ports.h,v 1.33 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_PORTS_INCLUDED__
/* { */
#define __SOC_TMC_API_PORTS_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/TMC/tmc_api_general.h>
#include <soc/dpp/TMC/tmc_api_cnt.h>
#include <shared/port.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximal System Physical Port index. Physical Port
*     uniquely identifies a FAP port in the system.           */
#define  SOC_TMC_PORTS_SYS_PHYS_PORT_ID_MAX 4095

/*     Maximal number of LAG-s in the system                  */
#define  SOC_TMC_PORT_LAGS_MAX 1024

/*     Maximal number of mamber ports in Out-LAG              */
#define  SOC_TMC_PORTS_LAG_OUT_MEMBERS_MAX 256

/* $Id: tmc_api_ports.h,v 1.33 Broadcom SDK $
 *   Maximal LAG member index.
 *   Note: though incoming LAG can contain up to 80 members,
 *   the maximal member-id is 15.
 *   If the LAG-member-id is not used by a higher protocol
 *   in the CPU, it's value is not significant (not used for LAG-based pruning)
 */
#define  SOC_TMC_PORTS_LAG_MEMBER_ID_MAX    (SOC_TMC_PORTS_LAG_OUT_MEMBERS_MAX-1)

/*     Maximal number of mamber ports in In-LAG               */
#define  SOC_TMC_PORTS_LAG_IN_MEMBERS_MAX SOC_TMC_NOF_FAP_PORTS

/*     Maximal number of mamber ports in In-LAG for ARAD      */
#define  SOC_TMC_PORTS_LAG_IN_MEMBERS_MAX_ARAD SOC_TMC_NOF_FAP_PORTS_ARAD

/*     Maximal number of mamber ports in LAG (incoming or outgoing) */
#define  SOC_TMC_PORTS_LAG_MEMBERS_MAX       SOC_TMC_PORTS_LAG_IN_MEMBERS_MAX
#define  SOC_TMC_PORTS_LAG_MEMBERS_MAX_ARAD  SOC_TMC_PORTS_LAG_IN_MEMBERS_MAX_ARAD

/* SOC_TMC_PORT_PP_PORT flags */
#define SOC_TMC_PORT_PP_PORT_RCY_OVERLAY_PTCH 0x1

/* SOC_TMC_PORT_PP_PORT flags */

#define SOC_TEST1_PORT_PP_PORT 0x2

/* SOC_TMC_PORT_PP_PORT flags - Preserving DSCP on a per out-port bases*/
#define SOC_PRESERVING_DSCP_PORT_PP_PORT 0x4

#define SOC_MIM_SPB_PORT 0x8

#define SOC_TMC_PORT_PRD_MAX_KEY_BUILD_OFFSETS 4

#define SOC_TMC_PORT_PRD_F_PORT_EXTERNDER 0x1
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
   *  Port direction - incoming (IFP).
   */
  SOC_TMC_PORT_DIRECTION_INCOMING=0,
  /*
   *  Port direction - outgoing (OFP).
   */
  SOC_TMC_PORT_DIRECTION_OUTGOING=1,
  /*
   *  Port direction - both incoming (IFP) and outgoing (OFP).
   */
  SOC_TMC_PORT_DIRECTION_BOTH=2,
  /*
   *  Total number of Port directions.
   */
  SOC_TMC_PORT_NOF_DIRECTIONS=3
}SOC_TMC_PORT_DIRECTION;

typedef enum
{
  /*
   *  Port header processing type: undefined.
   */
  SOC_TMC_PORT_HEADER_TYPE_NONE=0,
  /*
   *  Port header processing type: Ethernet. Supported
   *  direction: incoming / outgoing. Switching and TM
   *  functions are based on Ethernet packet
   *  processing. Incoming and outgoing outermost header is
   *  Ethernet.
   */
  SOC_TMC_PORT_HEADER_TYPE_ETH=1,
  /*
   *  Port header processing type: Raw. Supported direction:
   *  incoming / outgoing. Simple static switching; entire
   *  packet is payload. No header is assumed.
   */
  SOC_TMC_PORT_HEADER_TYPE_RAW=2,
  /*
   *  Port header processing type: TM. Supported direction:
   *  incoming / outgoing. Designed to enable use of the TM
   *  features of the Incoming/Outgoing packets have an
   *  outermost Incoming/Outgoing-TM-Header (ITMH/OTMH).
   */
  SOC_TMC_PORT_HEADER_TYPE_TM=3,
  /*
   *  Port header processing type: programmable. Supported
   *  direction: incoming. User programmable ingress
   *  processing. There are 4 programmable types that define
   *  the different starting program for classification of the
   *  packet.
   */
  SOC_TMC_PORT_HEADER_TYPE_PROG=4,
  /*
   *  Port header processing type: CPU. Supported direction:
   *  Outgoing. Designed to support CPU protocol
   *  processing. Outgoing packet has a Fabric-TM-Header
   *  (FTMH).
   */
  SOC_TMC_PORT_HEADER_TYPE_CPU=5,
  /*
   *  Port header processing type: Stacking. Supported direction:
   *  Incoming / Outgoing. Designed to support use of stacking ports
   *  with a Fabric-TM-Header(FTMH) format.
   */
  SOC_TMC_PORT_HEADER_TYPE_STACKING=6,
  /*
   *  Port header processing type: TDM. Supported direction:
   *  Incoming / Outgoing. Designed to support use of TDM processing
   *  with a regular Fabric-TM-Header(FTMH) format.
   */
  SOC_TMC_PORT_HEADER_TYPE_TDM=7,
  /*
   *  Port header processing type: TDM_RAW. Supported direction:
   *  Incoming. Designed to support use of TDM ports with
   *  simple static switching; entire packet is payload.
   *  No header is assumed.
   */
  SOC_TMC_PORT_HEADER_TYPE_TDM_RAW = 8,
  /*
   *  Port header processing type: Both. Supported direction:
   *  Incoming only. Designed to support use of injected packets with
   *  PTCH Header as a first header.
   *  Format in Arad - 24b: {Is-Eth(1b), PFQ0(3b), Ignored(4b), Source-System-Port(16b)}
   *  Output: FTMH (6B) + Out-LIF Extension (2B) + PPH.
   */
  SOC_TMC_PORT_HEADER_TYPE_INJECTED = 9,
    /* Same than SOC_TMC_PORT_HEADER_TYPE_INJECTED when the Port processing is PP */
  SOC_TMC_PORT_HEADER_TYPE_INJECTED_PP = 10,
  /*
   *  Port header processing type: Both. Supported direction:
   *  Incoming only. Designed to support use of injected packets with
   *  PTCH-2 Header as a first header.
   *  Format in Arad - 16b: {Is-Eth(1b), PFQ0(3b), Ignored(4b), In-PP-Port(8b)}
   *  Output: FTMH (6B) + Out-LIF Extension (2B) + PPH.
   */
  SOC_TMC_PORT_HEADER_TYPE_INJECTED_2 = 11,
    /* Same than SOC_TMC_PORT_HEADER_TYPE_INJECTED_2 when the Port processing is PP */
  SOC_TMC_PORT_HEADER_TYPE_INJECTED_2_PP = 12,
  /*
   *  Port header processing type: XGS_HQoS. Supported direction:
   *  incoming / outgoing. Designed to enable use of the Arad
   *  as a TM for XGS devices with HQoS queing model.
   *  features of the Incoming/Outgoing packets have an
   *  outermost Incoming/Outgoing FRC headers.
   */
  SOC_TMC_PORT_HEADER_TYPE_XGS_HQoS = 13,
  /*
   *  Port header processing type: XGS_DiffServ. Supported direction:
   *  incoming / outgoing. Designed to enable use of the Arad
   *  as a TM for XGS devices with Diffserv queing model.
   *  features of the Incoming/Outgoing packets have an
   *  outermost Incoming/Outgoing FRC headers.
   */
  SOC_TMC_PORT_HEADER_TYPE_XGS_DiffServ = 14,
  /*
   *  Port header processing type: XGS_MAC_EXT. Supported direction:
   *  incoming / outgoing. Designed to enable use XGS as
   *  a mac extension for Arad
   *  features of the Incoming/Outgoing packets have an
   *  outermost Incoming/Outgoing FRC headers.
   */
  SOC_TMC_PORT_HEADER_TYPE_XGS_MAC_EXT = 15,
  /*
   *  Port header processing type: TDM_PMM. Supported direction:
   *  Outgoing. Designed to support use of TDM processing
   *  with a stamping of PMM header (external header).
   */
  SOC_TMC_PORT_HEADER_TYPE_TDM_PMM=16,

  SOC_TMC_PORT_HEADER_TYPE_REMOTE_CPU=17,

  /*
   *  Port header processing type: Add User Defined Header
   *  (UDH) extension pre-concunitated to the standard packet.
   *  Supported direction: outgoing.
   *  Switching and TM functions are based on Ethernet packet
   *  processing. Outgoing outermost header is
   *  Application specific with leading UDH + Ethernet.
   */
  SOC_TMC_PORT_HEADER_TYPE_UDH_ETH=18,

    /*
   *  Port header processing type: MPLS_RAW.
   *  Supported direction: incoming / outgoing.
   *  Designed to enable ethernet header removal for  
   *  an MPLSoE packet.
   */
  SOC_TMC_PORT_HEADER_TYPE_MPLS_RAW=19,
  /*
   *  Port header processing type: L2_ENCAP_EXTERNAL_CPU_PORT.
   *  Supported direction: outgoing.
   *  Designed to enable ethernet header removal and  
   *  encapsulation for l2 external cpu port packet.
   */
 SOC_TMC_PORT_HEADER_TYPE_L2_ENCAP_EXTERNAL_CPU=20,

  /*
   *  Port header processing type: Add CoE Header
   *  extension pre-concunitated to the standard packet.
   *  Supported direction: Incoming/outgoing.
   *  Switching and TM functions are based on Ethernet packet
   *  processing. Outgoing outermost header is
   *  Application specific with leading CoE + Ethernet.
   */
  SOC_TMC_PORT_HEADER_TYPE_COE,
  /*
   *  Port header processing type: Mirror Raw. Supported direction:
   *  incoming / outgoing. Simple static switching; entire
   *  packet is payload. No header is assumed. Used for RCY interface
   *  Header Type. Perform ETH PMF program, Means adding PPH+PPH.
   */
  SOC_TMC_PORT_HEADER_TYPE_MIRROR_RAW,

  /*
   *  Total number of ingress port types
   */
  SOC_TMC_PORT_NOF_HEADER_TYPES
}SOC_TMC_PORT_HEADER_TYPE;

typedef enum
{
  /*
   *  Snoop the first 64 bytes of the packet.
   */
  SOC_TMC_PORTS_SNOOP_SIZE_BYTES_64=0,
  /*
   *  Snoop the first 192 bytes of the packet.
   */
  SOC_TMC_PORTS_SNOOP_SIZE_BYTES_192=2,
  /*
   *  Snoop the entire packet.
   */
  SOC_TMC_PORTS_SNOOP_SIZE_ALL=3,
  /*
   *  Total number of snoop sizes.
   */
  SOC_TMC_PORTS_NOF_SNOOP_SIZES=4
}SOC_TMC_PORTS_SNOOP_SIZE;

typedef enum
{
  /*
   *  Never add an FTMH or OTMH extension.
   */
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_NEVER=0,
  /*
   *  Add FTMH and OTMH extension only if multicast.
   */
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_IF_MC=1,
  /*
   *  Always add an FTMH and OTMH extension.
   */
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_ALWAYS=2,
  /*
   * Enlarge the outlif in the OTMH(using EEDB/EPE)
   */
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_ENLARGE=4,
  /*
   * Two hop scheduling. 
   */
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_DOUBLE_TAG=3,
  /*
   *  Number of option for the ftmh extension.
   */
  SOC_TMC_PORTS_FTMH_NOF_EXT_OUTLIFS
}SOC_TMC_PORTS_FTMH_EXT_OUTLIF;

typedef enum
{
  /*
   *  Egress header credit discount (AKA header compensation),
   *  type A.
   */
  SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_A=0,
  /*
   *  Egress header credit discount (AKA header compensation),
   *  type B.
   */
  SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_B=1,
  /*
   *  Total number of egress header credit discount types.
   */
  SOC_TMC_PORT_NOF_EGR_HDR_CR_DISCOUNT_TYPES=2
}SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE;

typedef enum
{
  /* 
   *  Default VT profile.
   */
  SOC_TMC_PORTS_VT_PROFILE_NONE=0,
  /* 
   *  Identify port as overlay recycle port.
   */
  SOC_TMC_PORTS_VT_PROFILE_OVERLAY_RCY=1,
  /* 
    *  Identify port as custom pp port.
    */
  SOC_TMC_PORTS_VT_PROFILE_CUSTOM_PP=2,
  /* 
   *  Total number of VT profiles.
   */
  SOC_TMC_PORTS_NOF_VT_PROFILES
} SOC_TMC_PORTS_VT_PROFILE;

typedef enum
{
  /* 
   *  Default TT profile.
   */
  SOC_TMC_PORTS_TT_PROFILE_NONE=0,
  /* 
   *  Identify port as overlay recycle port.
   */
  SOC_TMC_PORTS_TT_PROFILE_OVERLAY_RCY=1,
  /* 
   *  Identify port as PON port.
   */
  SOC_TMC_PORTS_TT_PROFILE_PON=2,
  /* 
   *  Total number of TT profiles.
   */
  SOC_TMC_PORTS_NOF_TT_PROFILES
} SOC_TMC_PORTS_TT_PROFILE;

typedef enum
{
  /* 
   *  Default FLP profile.
   */
  SOC_TMC_PORTS_FLP_PROFILE_NONE=0,
  /* 
   *  Identify port as overlay recycle port.
   */
  SOC_TMC_PORTS_FLP_PROFILE_OVERLAY_RCY=1,
  /* 
   *  Identify port as PON port(0-7).
   */
  SOC_TMC_PORTS_FLP_PROFILE_PON=2,
  /* 
   *  Identify port as OAMP.
   */
  SOC_TMC_PORTS_FLP_PROFILE_OAMP=3,
  /* 
   *  Total number of FLP profiles.
   */
  SOC_TMC_PORTS_NOF_FLP_PROFILES
} SOC_TMC_PORTS_FLP_PROFILE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Interface index of the interface to which the FAP port
   *  is mapped.
   */
  SOC_TMC_INTERFACE_ID if_id;
  /*
   *  Channel index. Zero for non-channelized interface. Range:
   *  Soc_petra: 0 - 63, Arad: 0 - 255.
   */
  uint32 channel_id;
}SOC_TMC_PORT2IF_MAPPING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  System Physical Port index of the port that is a member
   *  of the specified LAG. Range: 0 - 4095.
   */
  uint32 sys_port;
  /*
   *  The LAG member index. This index is not relevant for
   *  LAG-based pruning. It is embedded in the FTMH
   *  (SRC_SYS_PORT field), and can be used by the
   *  CPU. Outgoing LAG Range: 0 - 16. Incoming LAG Range: 0 -
   *  80.
   */
  uint32 member_id;

   /*  
    * Lag member flags
    */ 
  uint32 flags;
  
}  SOC_TMC_PORTS_LAG_MEMBER;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of lag members, as listed in lag_members. Outgoing
   *  LAG Range: 0 - 16. Incoming LAG Range: 0 - 80.
   */
  uint32 nof_entries;
  /*
   *  System Physical Port indexes of the ports that are
   *  members of the specified LAG.
   */
  SOC_TMC_PORTS_LAG_MEMBER lag_member_sys_ports[SOC_TMC_PORTS_LAG_MEMBERS_MAX];

}  SOC_TMC_PORTS_LAG_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Number of lag members, as listed in lag_members. Outgoing
   *  LAG Range: 0 - 16. Incoming LAG Range: 0 - 80.
   */
  uint32 nof_entries;
  /*
   *  System Physical Port indexes of the ports that are
   *  members of the specified LAG.
   */
  SOC_TMC_PORTS_LAG_MEMBER lag_member_sys_ports[SOC_TMC_PORTS_LAG_MEMBERS_MAX_ARAD];

}  SOC_TMC_PORTS_LAG_INFO_ARAD;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
 /*
  *  member in incoming LAG
  */
  uint8 in_member;
 /*
  *  the incoming LAG the sys port is member at
  */
  uint32 in_lag;
 /*
  *  number of out-lags this port is member in
  */
  uint8 nof_of_out_lags;
 /*
  *  the out coming LAG the sys port is member at
  */
  uint32 out_lags[SOC_TMC_MAX_NOF_LAG_MEMBER_IN_GROUP];
}SOC_TMC_PORT_LAG_SYS_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If TRUE, the requested field is overridden by the
   *  indicated value.
   */
  uint8 enable;
  /*
   *  The value with which the requested field is
   *  overridden. For the drop precedence, range: 0 - 3. For
   *  the traffic class, range: 0 - 7.
   */
  uint32 override_val;
}SOC_TMC_PORTS_OVERRIDE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable inbound mirroring for the specified port.
   */
  uint8 enable;
  /*
   *  Queue, multicast group or system port to send
   *  the mirrored packet to.
   *  Queue id. Range: 0 - 32K-1
   *  Multicast id. Range: 0 - 16K-1
   *  System Phy Port. Range: 0 - 4K-1
   *  Lag Id. Range: 0-255..
   */
  SOC_TMC_DEST_INFO destination;
  /*
   *  Override Drop Precedence - enable and value.
   */
  SOC_TMC_PORTS_OVERRIDE_INFO dp_override;
  /*
   *  Override Traffic Class - enable and value.
   */
  SOC_TMC_PORTS_OVERRIDE_INFO tc_override;
}SOC_TMC_PORT_INBOUND_MIRROR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable outbound mirroring for the specified
   *  port.
   */
  uint8 enable;
  /*
   *  The incoming FAP port id for mirroring. This FAP port
   *  must be mapped to a recycling channel, and there must be
   *  and outgoing FAP port to be mapped to the same recycling
   *  channel. Range: 0 - 79.
   */
  uint32 ifp_id;  
  /*
   * In case TRUE Skip configuration for port default mirror settings
   * Relevant only for Petra-B
   */
  uint8  skip_port_deafult_enable;
}SOC_TMC_PORT_OUTBOUND_MIRROR_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Enable/disable snooping for the specified port.
   */
  uint8 enable;
  /*
   *  Size in bytes, or all packet, to snoop.
   */
  SOC_TMC_PORTS_SNOOP_SIZE size_bytes;
  /*
   *  Queue, multicast group or system port the snooped packet
   *  is sent to.
   *  Queue id. Range: 0 - 32K-1
   *  Multicast id. Range: 0 - 16K-1
   *  System Port. Range: 0 - 4K-1
   *  LAG Id. Range: 0 - 255.
   */
  SOC_TMC_DEST_INFO destination;
  /*
   *  Override Drop Precedence - enable and value.
   */
  SOC_TMC_PORTS_OVERRIDE_INFO dp_override;
  /*
   *  Override Traffic Class - enable and value.
   */
  SOC_TMC_PORTS_OVERRIDE_INFO tc_override;
}SOC_TMC_PORT_SNOOP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates whether a packet-processor-header is above.
   */
  uint8 pp_header_present;
  /*
   *  Indicates whether port outbound mirroring is enabled
   *  /disabled.
   */
  uint8 out_mirror_dis;
  /*
   *  There are 15 types of 'Snoop Commands'. This field
   *  indicates which command should be used for this packet
   *  (snoop_cmd_ndx = 0, means snoop disabled). Range: 0-
   *  15.(* Optionally only 7 snoop commands are available -
   *  )
   */
  uint32 snoop_cmd_ndx;
  /*
   *  Indicates whether to filter the packet at the egress
   *  when it arrives with source system-port-id the same as
   *  destination system-port-id
   */
  uint8 exclude_src;
  /*
   *  Traffic class. Range: 0 - 7.
   */
  uint32 tr_cls;
  /*
   *  Drop Precedence. Range: 0 - 3.
   */
  uint32 dp;
  /*
   *  This field indicates the forwarding action destination
   *  data (Next address to be sent to - system wise) it can
   *  be system-port-id, unicast-flow-id, or
   *  multicast-id. Queue id. Range: 0 - 32K-1Multicast id.
   *  Range: 0 - 16K-1System Physical Port. Range: 0 - 4K-1LAG
   *  id. Range: 0 - 255.
   */
  SOC_TMC_DEST_INFO destination;
}SOC_TMC_PORTS_ITMH_BASE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Indicates whether the extension exists. If this value is
   *  FALSE the next fields in this structure are
   *  meaningless. If TRUE, the purpose of this extension is to
   *  override the value that is placed in the FTMH, thus
   *  allowing a packet to impersonate as entering from any
   *  sys-port when processed at the egress.
   */
  uint8 enable;
  /*
   *  This field indicated whether the 'src_port' value is a
   *  system-level port (system-port - [0..4095]), or is the
   *  value of the local source port (IFP - [0..79]).
   */
  uint8 is_src_sys_port;
  /*
   *  If the previous field 'is_src_sys_port' is true, this
   *  field identifies the system level source LAG or physical
   *  system port at the ingress (see DEST_SYS_PORT field
   *  description in the DS). Otherwise, this field identifies
   *  the local incoming fap source port.
   */
  SOC_TMC_DEST_SYS_PORT_INFO src_port;
  /* 
   * This field indicates the forwarding action destination
   * data.
   */ 
  SOC_TMC_DEST_INFO destination;
}SOC_TMC_PORTS_ITMH_EXT_SRC_PORT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The ITMH base fields information.
   */
  SOC_TMC_PORTS_ITMH_BASE base;
  /*
   *  The ITMH source port extension info.
   */
  SOC_TMC_PORTS_ITMH_EXT_SRC_PORT extension;
}SOC_TMC_PORTS_ITMH;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  enable/disable ingress shaping.
   */
  uint8 enable;
  /*
   *  The index of the destination ingress shaping queue.
   *  Range: 0 - 32K-1.
   */
  uint32 queue_id;
}SOC_TMC_PORTS_ISP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Customer Service ID (valid only for Ethernet type
   *  ports).
   */
  uint8 cid;
  /*
   *  Incoming FAP Port.
   */
  uint8 ifp;
  /*
   *  Traffic class.
   */
  uint8 tr_cls;
  /*
   *  Drop precedence.
   */
  uint8 dp;
  /*
   *  Data type. Unicast/ Multicast/ Broadcast. The indication
   *  is added to the packet in following form: (UC is 01; BC
   *  is 10; MC is 11)
   */
  uint8 data_type;
}SOC_TMC_PORTS_STAG_FIELDS;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The OTMH has 3 optional extensions: Outlif (always
   *  allow/ never allow/ allow only when the packet is
   *  multicast.)
   */
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF outlif_ext_en;
  /*
   *  Source Sys-Port enable.
   */
  uint8 src_ext_en;
  /*
   *  Destination Sys-Port enable. Not valid for Arad.
   */
  uint8 dest_ext_en;
}SOC_TMC_PORTS_OTMH_EXTENSIONS_EN;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Credit discount (AKA Header Compensation) for unicast
   *  packets, in Bytes. Range: -32 - 32.
   */
  int32 uc_credit_discount;
  /*
   *  Credit discount (AKA Header Compensation) for multicast
   *  packets, in Bytes. Range: -32 - 32.
   */
  int32 mc_credit_discount;
}SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO;

typedef enum
{
  /*
   *  No Flow Control.
   */
  SOC_TMC_PORTS_FC_TYPE_NONE = 0,
  /*
   *  Link-Level Flow Control.
   */
  SOC_TMC_PORTS_FC_TYPE_LL = 1,
  /*
   *  Class-Based Flow Control (2 classes).
   */
  SOC_TMC_PORTS_FC_TYPE_CB2 = 2,
  /*
   *  Class-Based Flow Control (8 classes).
   */
  SOC_TMC_PORTS_FC_TYPE_CB8 = 3,
  /*
   *  Number of types in SOC_TMC_PORTS_FC_TYPE
   */
  SOC_TMC_PORTS_NOF_FC_TYPES = 4
}SOC_TMC_PORTS_FC_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then Packets coming from this TM Port have a
   *  Statistic Tag header. The Statistic Header position is
   *  defined globally according to the soc_pb_itm_stag_set
   *  API. Relevant only if the header type is TM.
   */
  uint8 is_stag_enabled;
  /*
   *  If True, then Packets coming from this TM Port have a
   *  first header to strip before any processing. For
   *  example, in the Fat Pipe processing a Sequence Number
   *  header (2 Bytes) must be stripped.
   *  For injected packets, the PTCH Header must be
   *  removed (4 Bytes).
   *  Units: Bytes. Range: 0 - 63.
   */
  uint32 first_header_size;
  /*
   *  If True, then Packets coming from this TM Port have a
   *  Statistic Tag header
   */
  SOC_TMC_PORTS_FC_TYPE fc_type;
  /*
   *    Header type of the traffic going through this PP-Port.
   *  During the mapping between the TM-Ports and the PP-Ports,
   *  all the header types of the TM Ports and PP Ports
   *  must be identical.
   */
  SOC_TMC_PORT_HEADER_TYPE header_type;
  /*
   *  Header type of the traffic going out of this PP-Port (Arad only)
   */
  SOC_TMC_PORT_HEADER_TYPE header_type_out;
  /*
   *  If True, then Packets coming from this TM Port are snooped
   *  according to the ITMH. Snoop action command
   */
  uint8 is_snoop_enabled;
  /*
   *  Inbound mirroring action command (i.e., its profile) for this
   *  PP Port. Range: 0 - 15.
   *  Relevant only if the header type is not Ethernet.
   */
  uint32 mirror_profile;
  /*
   *  If True, then TM Packets can come with an Ingress Shaping header
   *  before the ITMH. Relevant only if the header type is TM.
   */
  uint8 is_tm_ing_shaping_enabled;
  /*
   *  If True, then TM Packets can come with a PPH header
   *  after the ITMH (the PPH-present bit in the ITMH can be set).
   *  Relevant only if the header type is TM and only for Petra-B
   */
  uint8 is_tm_pph_present_enabled;
  /*
   *  If True, then all TM Packets come with a Source-System-Port Extension
   *  after the ITMH.
   *  Relevant only if the header type is TM.
   */
  uint8 is_tm_src_syst_port_ext_present;
  /*
   *  Flags SOC_TMC_PORT_PP_PORT_XXX
   */
  uint32 flags;

} SOC_TMC_PORT_PP_PORT_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *    Counter Processor the instance 'id' is attached to.
   */
  SOC_TMC_CNT_PROCESSOR_ID processor_id;

  /*
   *  Counter instance index. Range: 0 - 8K-1.
   */
  uint32 id;

} SOC_TMC_PORT_COUNTER_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  This field indicates the forwarding action destination
   *  data (Next address to be sent to - system wise). It can
   *  be system-port-id, unicast-flow-id, or
   *  multicast-id. Queue id. Range: 0 - 32K-1Multicast id.
   *  Range: 0 - 16K-1System Physical Port. Range: 0 - 4K-1LAG
   *  id. Range: 0 - 255.
   */
  SOC_TMC_DEST_INFO destination;
  /*
   *  Traffic Class. Range: 0 - 7.
   */
  uint32 tr_cls;
  /*
   *  Drop Precedence. Range: 0 - 3.
   */
  uint32 dp;
  /*
   *  Snoop command index. Range: 0 - 15.
   */
  uint32 snoop_cmd_ndx;
  /*
   *  Counter instance to attach to the Packet.
   */
  SOC_TMC_PORT_COUNTER_INFO counter;

} SOC_TMC_PORTS_FORWARDING_HEADER_INFO;

typedef enum
{
  /*
   *  No first header must be processed for this TM-Port. In
   *  this case, this TM-Port is considered as a regular CPU
   *  port.
   */
  SOC_TMC_PORT_INJECTED_HDR_FORMAT_NONE = 0,
  /*
   *  Explicit PP-Port format header: all the Packets coming
   *  from this CPU TM-Port start with a PTCH-2 header
   *  indicating if the Packet is of type TM (then the
   *  following header is ITMH) or PP (in this case, the
   *  PP-Port is explicit and part of the header).
   */
  SOC_TMC_PORT_INJECTED_HDR_FORMAT_PP_PORT = 1,
  /*
   *  Number of types in SOC_TMC_PORT_INJECTED_HDR_FORMAT
   */
  SOC_TMC_PORT_NOF_INJECTED_HDR_FORMATS = 2
}SOC_TMC_PORT_INJECTED_HDR_FORMAT;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Format of the first packet header.
   */
  SOC_TMC_PORT_INJECTED_HDR_FORMAT hdr_format;
  /*
   *  PP-Port for the TM traffic coming from this TM-Port.
   *  Must be set only if the Header format 'hdr_format' is
   *  'PP_PORT'. This PP-Port must be previously defined via
   *  the soc_pb_port_pp_port_set API with a header type 'TM'.
   */
  uint32 pp_port_for_tm_traffic;

} SOC_TMC_PORT_INJECTED_CPU_HEADER_INFO;

typedef enum
{
    /* Arad only - Determines the number of LAGs in system */
    /* 1x1 mapping with HW */
  /*
   *   1K groups of 16
   */
  SOC_TMC_PORT_LAG_MODE_1K_16 = 0,
  /*
   *  512 groups of 32
   */
  SOC_TMC_PORT_LAG_MODE_512_32 = 1,
  /*
   *  256 groups of 64
   */
  SOC_TMC_PORT_LAG_MODE_256_64 = 2,
  /*
   *  128 groups of 128
   */
  SOC_TMC_PORT_LAG_MODE_128_128 = 3,
  /*
   *  64 groups of 256
   */
  SOC_TMC_PORT_LAG_MODE_64_256 = 4,
  /*
   *   32 groups of 64 for QUX
   */
  SOC_TMC_PORT_LAG_MODE_32_64 = 0,
  /*
   *  Total number of Port directions.
   */
  SOC_TMC_NOF_PORT_LAG_MODES = 6
}SOC_TMC_PORT_LAG_MODE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * PP-Port mapped from TM-Port and Tunnel-ID.
   * Used only in PON application.
   */
  uint32 pp_port;
}SOC_TMC_PORTS_PON_TUNNEL_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * SA, DA, EtherType and VLAN tag
   */
  uint16 tpid;
  uint16 vlan;
  uint16 eth_type;
  uint8 sa[6];
  uint8 da[6];
}SOC_TMC_L2_ENCAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * Enable swap operation on port.
   */
  uint8 enable;
} SOC_TMC_PORTS_SWAP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   * Set TM Port profile for VT programs.
   */
  SOC_TMC_PORTS_VT_PROFILE ptc_vt_profile;
  /* 
   * Set TM Port profile fot TT programs.
   */
  SOC_TMC_PORTS_TT_PROFILE ptc_tt_profile;
  /* 
   * Set TM Port profile for FLP programs.    
   */
  SOC_TMC_PORTS_FLP_PROFILE ptc_flp_profile;
  /* 
  * Set Trill multicast to terminate the Trill header .
  */
  uint32 en_trill_mc_in_two_path;
} SOC_TMC_PORTS_PROGRAMS_INFO;

/* ILKN Retransmit */
typedef struct {
    uint32 enable_rx;
    uint32 enable_tx;
    uint32 nof_seq_number_repetitions_rx;
    uint32 nof_seq_number_repetitions_tx;
    uint32 nof_requests_resent;
    uint32 rx_timeout_words;
    uint32 rx_timeout_sn;
    uint32 rx_ignore;
    uint32 rx_watchdog;
    uint32 buffer_size_entries;
    uint32 tx_wait_for_seq_num_change;
    uint32 tx_ignore_requests_when_fifo_almost_empty;
    uint32 rx_reset_when_error;
    uint32 rx_reset_when_watchdog_err;
    uint32 rx_reset_when_alligned_error;
    uint32 rx_reset_when_retry_error;
    uint32 rx_reset_when_wrap_after_disc_error;
    uint32 rx_reset_when_wrap_before_disc_error;
    uint32 rx_reset_when_timout_error;
    uint32 reserved_channel_id_rx;                 /* Relevant only for Jericho */
    uint32 reserved_channel_id_tx;                 /* Relevant only for Jericho */
    uint32 seq_number_bits_rx;                     /* Relevant only for Jericho */
    uint32 seq_number_bits_tx;                     /* Relevant only for Jericho */
    uint32 rx_discontinuity_event_timeout;         /* Relevant only for Jericho */
    uint32 peer_tx_buffer_size;                    /* Relevant only for Jericho */
} SOC_TMC_PORTS_ILKN_RETRANSMIT_CONFIG;

/* ILKN */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  uint32 metaframe_sync_period;
  uint32 interfcae_status_ignore;
  uint32 interfcae_status_oob_ignore;
  uint32 interleaved;
  uint32 mubits_tx_polarity;
  uint32 mubits_rx_polarity;
  uint32 fc_tx_polarity;
  uint32 fc_rx_polarity;
  uint32 dedicated_tdm_context;
  SOC_TMC_PORTS_ILKN_RETRANSMIT_CONFIG retransmit;

} SOC_TMC_PORTS_ILKN_CONFIG;

/* CAUI */
typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  uint8 rx_recovery_lane;

} SOC_TMC_PORTS_CAUI_CONFIG;

/* 
 * Application mapping type enum used in SOC_TMC_PORTS_APPLICATION_MAPPING_INFO
 */
typedef enum
{
  SOC_TMC_PORTS_APPLICATION_MAPPING_TYPE_XGS_MAC_EXTENDER = 0,
  SOC_TMC_PORTS_APPLICATION_MAPPING_TYPE_PP_PORT_EXTENDER = 1,
  SOC_TMC_PORTS_APPLICATION_MAPPING_NOF_TYPES
} SOC_TMC_PORTS_APPLICATION_MAPPING_TYPE;

typedef union
{
  /* XGS MAC Extender information */
  struct
  {
    /* 
     * HiGig header Modid field 
     * Required to map out-port to Higig Modid 
     */
    uint32 hg_modid;
    /* 
     * HiGig header port field      
     * Required to: 
     *   Ingress map incoming-port from Higig port and PTC. 
     *   Egress map out-port to Higig Port. 
     */
    uint32 hg_port;
    /* 
     * PP-Port mapped from PTC (TM-Port) 
     * and HiGig header port
     */
    uint32 pp_port;
  } xgs_mac_ext;
  /* PP Port Extender information */
  struct
  {
    /* 
     * Virtual port table index
     * To access virtual port table
     */
    uint32 index;
    /* 
     * PP-Port mapped from PON+Tunnel_id/VLAN/Source Board+Source Port
     */
    uint32 pp_port;
    /* 
     * Source system port
     * used to get the ingress source system port from "virtual port table"
     */
    uint32 sys_port;
  } pp_port_ext;
} SOC_TMC_PORTS_APPLICATION_MAPPING_VALUE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR

  /* 
   * Application type. 
   * Determines application mapping information to take from.
   */
  SOC_TMC_PORTS_APPLICATION_MAPPING_TYPE type;

  /* 
   * Value per application
   */
  SOC_TMC_PORTS_APPLICATION_MAPPING_VALUE value;
   

} SOC_TMC_PORTS_APPLICATION_MAPPING_INFO;


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
  SOC_TMC_PORT2IF_MAPPING_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT2IF_MAPPING_INFO *info
  );

void
SOC_TMC_PORTS_LAG_MEMBER_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_LAG_MEMBER *info
  );

void
  SOC_TMC_PORTS_LAG_INFO_ARAD_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_LAG_INFO_ARAD *info
  );

void
  SOC_TMC_PORTS_LAG_INFO_PETRAB_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_LAG_INFO *info
  );

void
  SOC_TMC_PORTS_OVERRIDE_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_OVERRIDE_INFO *info
  );

void
  SOC_TMC_PORT_INBOUND_MIRROR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_INBOUND_MIRROR_INFO *info
  );

void
  SOC_TMC_PORT_OUTBOUND_MIRROR_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_OUTBOUND_MIRROR_INFO *info
  );

void
  SOC_TMC_PORT_SNOOP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_SNOOP_INFO *info
  );

void
  SOC_TMC_PORTS_ITMH_BASE_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_ITMH_BASE *info
  );

void
  SOC_TMC_PORTS_ITMH_EXT_SRC_PORT_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_ITMH_EXT_SRC_PORT *info
  );

void
  SOC_TMC_PORTS_ITMH_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_ITMH *info
  );

void
  SOC_TMC_PORTS_ISP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_ISP_INFO *info
  );

void
  SOC_TMC_PORT_LAG_SYS_PORT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_LAG_SYS_PORT_INFO *info
  );

void
  SOC_TMC_PORTS_STAG_FIELDS_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_STAG_FIELDS   *info
  );

void
  SOC_TMC_PORTS_OTMH_EXTENSIONS_EN_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_OTMH_EXTENSIONS_EN *info
  );

void
  SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

void
  SOC_TMC_PORT_PP_PORT_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_PP_PORT_INFO *info
  );

void
  SOC_TMC_PORT_COUNTER_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_COUNTER_INFO *info
  );

void
  SOC_TMC_PORTS_FORWARDING_HEADER_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_FORWARDING_HEADER_INFO *info
  );

void
  SOC_TMC_PORT_INJECTED_CPU_HEADER_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORT_INJECTED_CPU_HEADER_INFO *info
  );

void
  SOC_TMC_PORTS_SWAP_INFO_clear(
    SOC_TMC_PORTS_SWAP_INFO *info
  );

void
  SOC_TMC_PORTS_PON_TUNNEL_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_PON_TUNNEL_INFO *info
  );

void
  SOC_TMC_PORTS_PROGRAMS_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_PROGRAMS_INFO *info
  );

void
  SOC_TMC_L2_ENCAP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_L2_ENCAP_INFO *info
  );

void
  SOC_TMC_PORTS_APPLICATION_MAPPING_INFO_clear(
    SOC_SAND_OUT SOC_TMC_PORTS_APPLICATION_MAPPING_INFO *info
  );


#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_PORT_DIRECTION_to_string(
    SOC_SAND_IN SOC_TMC_PORT_DIRECTION enum_val
  );

const char*
  SOC_TMC_PORT_HEADER_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_PORT_HEADER_TYPE enum_val
  );

const char*
  SOC_TMC_PORTS_SNOOP_SIZE_to_string(
    SOC_SAND_IN SOC_TMC_PORTS_SNOOP_SIZE enum_val
  );

const char*
  SOC_TMC_PORTS_FTMH_EXT_OUTLIF_to_string(
    SOC_SAND_IN SOC_TMC_PORTS_FTMH_EXT_OUTLIF enum_val
  );

const char*
  SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE_to_string(
    SOC_SAND_IN SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE enum_val
  );

const char*
  SOC_TMC_PORT_INJECTED_HDR_FORMAT_to_string(
    SOC_SAND_IN  SOC_TMC_PORT_INJECTED_HDR_FORMAT enum_val
  );

void
  SOC_TMC_PORT2IF_MAPPING_INFO_print(
    SOC_SAND_IN SOC_TMC_PORT2IF_MAPPING_INFO *info
  );

void
  SOC_TMC_PORTS_LAG_MEMBER_print(
    SOC_SAND_IN  SOC_TMC_PORTS_LAG_MEMBER *info
  );

void
  SOC_TMC_PORTS_LAG_INFO_print(
    SOC_SAND_IN SOC_TMC_PORTS_LAG_INFO *info
  );

void
  SOC_TMC_PORTS_OVERRIDE_INFO_print(
    SOC_SAND_IN SOC_TMC_PORTS_OVERRIDE_INFO *info
  );

void
  SOC_TMC_PORT_INBOUND_MIRROR_INFO_print(
    SOC_SAND_IN SOC_TMC_PORT_INBOUND_MIRROR_INFO *info
  );

void
  SOC_TMC_PORT_OUTBOUND_MIRROR_INFO_print(
    SOC_SAND_IN SOC_TMC_PORT_OUTBOUND_MIRROR_INFO *info
  );

void
  SOC_TMC_PORT_SNOOP_INFO_print(
    SOC_SAND_IN SOC_TMC_PORT_SNOOP_INFO *info
  );

void
  SOC_TMC_PORTS_ITMH_BASE_print(
    SOC_SAND_IN SOC_TMC_PORTS_ITMH_BASE *info
  );

void
  SOC_TMC_PORT_LAG_SYS_PORT_INFO_print(
    SOC_SAND_IN SOC_TMC_PORT_LAG_SYS_PORT_INFO *info
  );

void
  SOC_TMC_PORTS_ITMH_EXT_SRC_PORT_print(
    SOC_SAND_IN SOC_TMC_PORTS_ITMH_EXT_SRC_PORT *info
  );

void
  SOC_TMC_PORTS_ITMH_print(
    SOC_SAND_IN SOC_TMC_PORTS_ITMH *info
  );

void
  SOC_TMC_PORTS_ISP_INFO_print(
    SOC_SAND_IN SOC_TMC_PORTS_ISP_INFO *info
  );

void
  SOC_TMC_PORTS_STAG_FIELDS_print(
    SOC_SAND_IN SOC_TMC_PORTS_STAG_FIELDS   *info
  );

void
  SOC_TMC_PORTS_OTMH_EXTENSIONS_EN_print(
    SOC_SAND_IN SOC_TMC_PORTS_OTMH_EXTENSIONS_EN *info
  );

void
  SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO_print(
    SOC_SAND_IN SOC_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *info
  );

const char*
  SOC_TMC_PORTS_FC_TYPE_to_string(
    SOC_SAND_IN  SOC_TMC_PORTS_FC_TYPE enum_val
  );

void
  SOC_TMC_PORT_PP_PORT_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORT_PP_PORT_INFO *info
  );

void
  SOC_TMC_PORT_COUNTER_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORT_COUNTER_INFO *info
  );

void
  SOC_TMC_PORTS_FORWARDING_HEADER_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORTS_FORWARDING_HEADER_INFO *info
  );

void
  SOC_TMC_PORT_INJECTED_CPU_HEADER_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORT_INJECTED_CPU_HEADER_INFO *info
  );

void
  SOC_TMC_PORTS_SWAP_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORTS_SWAP_INFO *info
  );

void
  SOC_TMC_PORTS_PON_TUNNEL_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORTS_PON_TUNNEL_INFO *info
  );

void
  SOC_TMC_PORTS_PROGRAMS_INFO_print(
    SOC_SAND_IN  SOC_TMC_PORTS_PROGRAMS_INFO *info
  );


/* 
 * SOC_TMC_PORT_SWAP_GLOBAL_INFO 
 * Used to configure the global IRE swap definitions. 
 */
typedef struct {
    /* 
     *  Global size of tag to swap. 
     *  0 - 4B
     *  1 - 6B
     *  2 - 8B
     *  3 - 10B
     */
    uint8 global_tag_swap_n_size;
    /* 
     *  Tag size in case of tpid_0/1 match.
     *  Values are like the global size.
     */
    uint8 tpid_1_tag_swap_n_size;
    uint8 tpid_0_tag_swap_n_size;
    /* 
     *  Tpid_1/0 to match.
     */
    uint16 tag_swap_n_tpid_1;
    uint16 tag_swap_n_tpid_0;
    /* 
     *  Offest from packet start where tpid_1/0 is located.
     */
    uint8 tag_swap_n_offset_1;
    uint8 tag_swap_n_offset_0;
} SOC_TMC_PORT_SWAP_GLOBAL_INFO;

/* in PRD (priority drop) user can define destination address 
 * and Eth type code to be identified as control frame and get the highest priority
 */
typedef struct {
    uint64 mac_da_val; /*MAC DA*/
    uint64 mac_da_mask; /*MAC DA mask*/
    uint32 ether_type_code; /*ether code*/
    uint32 ether_type_code_mask; /*ether code mask*/
} SOC_TMC_PORT_PRD_CONTROL_PLANE;

/* in PRD there is a TCAM which allows the user to override
 *  priority if there is a hit in one of the entrys.
 * the user should build the TCAM using the following struct
 */
typedef struct {
    uint32 ether_code; /*ether code*/
    uint32 ether_code_mask; /*ether code mask*/
    uint32 num_key_fields; /*actual fields in array */             
    uint32 key_fields_values[SOC_TMC_PORT_PRD_MAX_KEY_BUILD_OFFSETS]; /*up to 4 segments of 8 bits each*/
    uint32 key_fields_masks[SOC_TMC_PORT_PRD_MAX_KEY_BUILD_OFFSETS]; /*key mask - up to 4 segments of 8 bits each*/
    uint32 priority; /*priority for this entry */                  
} SOC_TMC_PORT_PRD_FLEX_KEY_ENTRY;

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_PORTS_INCLUDED__*/
#endif

