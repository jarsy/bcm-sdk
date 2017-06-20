/* $Id: ppc_api_metering.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
/******************************************************************
*
* FILENAME:       DuneDriver/ppc/include/soc_ppc_api_metering.h
*
* MODULE PREFIX:  soc_ppc_metering
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

#ifndef __SOC_PPC_API_METERING_INCLUDED__
/* { */
#define __SOC_PPC_API_METERING_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <bcm/policer.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     indicates not to assign meter to the traffic            */
#define  SOC_PPC_MTR_ASSIGN_NO_METER (0xFFFFFFFF)
#define SOC_PPC_BW_PROFILE_IR_MAX_UNLIMITED 0xffffffff
#define SOC_PPC_BW_PROFILE_MAX_NUMBER_OF_RATES (24)
#define ARAD_PP_INVALID_IR_REV_EXP (0xFFFFFFFF)


/* 
 * returns the SOC_PPC_MTR_RES_USE according to the type needed (ingress/egress/both/none)
 * ingress: Metering result overwrite the DP (Drop Precedence) in ingress
 * egress: Metering result overwrite the DE (Drop Eligibility), used by egress 
 */ 
#define  SOC_PPC_MTR_RES_USE_GET_BY_TYPE(_ingress, _egress) \
  ((_ingress) && (_egress)) ? SOC_PPC_MTR_RES_USE_OW_DP_DE : \
  (((_ingress) && !(_egress)) ? SOC_PPC_MTR_RES_USE_OW_DP: \
  ((!(_ingress) && (_egress)) ? SOC_PPC_MTR_RES_USE_OW_DE : \
  SOC_PPC_MTR_RES_USE_NONE))

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
   *  Unknown Unicast traffic
   */
  SOC_PPC_MTR_ETH_TYPE_UNKNOW_UC = 0,
  /*
   *  Known Unicast traffic
   */
  SOC_PPC_MTR_ETH_TYPE_KNOW_UC = 1,
  /*
   *  Unknown Multicast traffic
   */
  SOC_PPC_MTR_ETH_TYPE_UNKNOW_MC = 2,
  /*
   *  Known Multicast traffic
   */
  SOC_PPC_MTR_ETH_TYPE_KNOW_MC = 3,
  /*
   *  Broadcast traffic
   */
  SOC_PPC_MTR_ETH_TYPE_BC = 4,
  /*
   *  Number of types in SOC_PPC_MTR_ETH_TYPE
   */
  SOC_PPC_NOF_MTR_ETH_TYPES = 5
}SOC_PPC_MTR_ETH_TYPE;

typedef enum
{
  /*
   *  Ignore the incoming packet color (if any) when
   *  determining the (new) color to apply to the packet. (all
   *  packets are assumed Green)
   */
  SOC_PPC_MTR_COLOR_MODE_BLIND = 0,
  /*
   *  Consider the incoming packet color (if any) when
   *  determining the (new) color to apply to the packet.
   */
  SOC_PPC_MTR_COLOR_MODE_AWARE = 1,
  /*
   *  Number of types in SOC_PPC_MTR_COLOR_MODE
   */
  SOC_PPC_NOF_MTR_COLOR_MODES = 2
}SOC_PPC_MTR_COLOR_MODE;

typedef enum
{
  /*
   *  Single meter per packet, no sharing
   */
  SOC_PPC_MTR_SHARING_MODE_NONE = 0,
  /*
   *  Two meters per packet, the output of the first in the input of the second
   */
  SOC_PPC_MTR_SHARING_MODE_SERIAL = 1,
  /*
   *  Two meters per packet, each meter determines the output color independently
   */
  SOC_PPC_MTR_SHARING_MODE_PARALLEL = 2,
  /*
   *  Number of types in SOC_PPC_MTR_SHARING_MODE
   */
  SOC_PPC_NOF_MTR_SHARING_MODES = 3
}SOC_PPC_MTR_SHARING_MODE;

typedef enum
{
  /*
   *  Default resolution, both ethernet policer and metering Red are 3
   */
  SOC_PPC_MTR_COLOR_RESOLUTION_MODE_DEFAULT = 0,
   /*
   *  Red diffrentiate resolution, Ethernet Policer Red is 3, Metering
   *  Red is 2
   */
  SOC_PPC_MTR_COLOR_RESOLUTION_MODE_RED_DIFF = 1,
  /*
   *  Number of types in SOC_PPC_MTR_COLOR_RESOLUTION_MODE
   */
  SOC_PPC_NOF_MTR_COLOR_RESOLUTION_MODES = 2
}SOC_PPC_MTR_COLOR_RESOLUTION_MODE;

typedef enum
{
  /*
   *  Metering result overwrite both DE and DP. (See below)
   */
  SOC_PPC_MTR_RES_USE_OW_DP_DE = 0,
  /*
   *  Metering result overwrite the DP (Drop Precedence) in
   *  ingress, used for ingress packet queuing.
   */
  SOC_PPC_MTR_RES_USE_OW_DP = 1,
  /*
   *  Metering result overwrite the DE (Drop Eligibility),
   *  used by egress to build the packet UP/PCP.
   */
  SOC_PPC_MTR_RES_USE_OW_DE = 2,
  /*
   *  Metering has no effect. Mainly for debug purpose.
   */
  SOC_PPC_MTR_RES_USE_NONE = 3,
  /*
   *  Number of types in SOC_PPC_MTR_RES_USE
   */
  SOC_PPC_NOF_MTR_RES_USES = 4
}SOC_PPC_MTR_RES_USE;

typedef enum
{
  /*
   *  Packets are metered according to the incoming logical
   *  interface they attached to
   */
  SOC_PPC_MTR_TYPE_INLIF = 0,
  /*
   *  Packets are metered according to the Outgoing AC they
   *  are sent from
   */
  SOC_PPC_MTR_TYPE_OUTLIF = 1,
  /*
   *  Packets are metered according to the Ingress AC
   */
  SOC_PPC_MTR_TYPE_IN_AC = 2,
  /*
   *  Packets are metered according to the Egress AC they are
   *  sent from
   */
  SOC_PPC_MTR_TYPE_OUT_AC = 3,
  /*
   *  Packets are metered according to the VSID they are
   *  attached to
   */
  SOC_PPC_MTR_TYPE_VSID = 4,
  /*
   *  Packets are metered according to the PWE incoming MPLS
   *  label ID
   */
  SOC_PPC_MTR_TYPE_IN_PWE = 5,
  /*
   *  Packets are metered according to the Ingress terminated
   *  label (Tunnel/LSR label)
   */
  SOC_PPC_MTR_TYPE_IN_LABEL = 6,
  /*
   *  Packets are metered according to the Egress label they
   *  sent with
   */
  SOC_PPC_MTR_TYPE_OUT_LABEL = 7,
  /*
   *  Soc_petra-B only, Packets are metered according to the FEC
   *  entry they were forwarded by.
   */
  SOC_PPC_MTR_TYPE_FEC_ID = 8,
  /*
   *  Packets are metered according to the Ingress AC and
   *  Traffic Class. Each AC is attached to 8 Meter
   *  instances:Meter-ID = AC-ID * 8 + TC
   */
  SOC_PPC_MTR_TYPE_IN_AC_AND_TC = 9,
  /*
   *  Number of types in SOC_PPC_MTR_TYPE
   */
  SOC_PPC_NOF_MTR_TYPES = 10
}SOC_PPC_MTR_TYPE;

typedef enum
{
  /*
   *  IPG disabled
   */
  SOC_PPC_MTR_IPG_COMPENSATION_DISABLED_SIZE = 0,
  /*
   *  IPG enabled, compensation size of 20 bytes
   */
  SOC_PPC_MTR_IPG_COMPENSATION_ENABLED_SIZE = 20,
  /*
   *  Number of types in SOC_PPC_MTR_TYPE
   */
  SOC_PPC_NOF_MTR_IPG_COMPENSATION_SIZE = 2
}SOC_PPC_MTR_IPG_COMPENSATION_SIZE_TYPE;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Group selection. There are two groups of meters, and
   *  each packet may be assigned one meter from each group.
   *  Soc_petra-B only. In T20E has to be zero. Note: In T20E
   *  packet is assigned two meters according to Tunnel and
   *  PWE
   */
  uint32 group;
  /*
   *  number that Identifies the meter in the group
   */
  uint32 id;

} SOC_PPC_MTR_METER_ID;


typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the High-rate metering is enabled Soc_petra-B
   *  only. in T20E has to be FALSE. if High-rate metering is
   *  enabled then - 0-447 are use for normal profiles -
   *  448-511 used for high rate profile.if High-rate metering
   *  is disabled then- 0-511 are use for normal profiles In
   *  Normal Profile: Information Rates (CIR and EIR) are
   *  comprised between 64 Kbps and 19 Gbps. The burst sizes
   *  (CBS and EBS) are comprised between 64B and
   *  1,040,384B. In High-rate Profile: Information Rates (CIR
   *  and EIR) are between 9.6 Gbps and 120 Gbps. The burst
   *  sizes (CBS and EBS) are comprised between 64B and
   *  4,161,536B. In addition, all the rates and burst sizes
   *  are given with a worst case resolution of 1.56%.
   *  Soc_petra-B only.
   */
  uint8 is_hr_enabled;
  /*
   *  Maximal packet size for metering. Used (only) for
   *  fairness mode, in fairness mode packets receiving grants
   *  regardless their size, and all packets considered with
   *  this size. Units: Bytes. Range: 0 - 16K.
   *  Soc_petra-B only.
   */
  uint32 max_packet_size;
  /*
   *  In case two streams (or more) are associated to same policer,
   *  one stream may be starved as another stream consumes all the credits.
   *  To avoid this starvation, there is a global mode to randomize the bucket level. 
   *  If True, a packet is considered in profile if bucket-level plus random value
   *  are above the packet's size. Otherwise, a packet is consider in profile if
   *  bucket-level is above the packet's size. 
   *  ARAD only.
   */
  uint8 rand_mode_enable;

} SOC_PPC_MTR_GROUP_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /* When set, disable the CIR. i.e.,
   * the Committed Leaky-Bucket will never give credit.
   * The actual rate will be zero. Relevant only to meter APIs;
   * not relevant for Ethernet policers (has to be FALSE).
  */
  uint8 disable_cir;
   /* When set, disable the EIR. i.e.,
   * the Excess Leaky-Bucket will never give credit.
   * The actual rate will be zero. Relevant only to meter APIs;
   * not relevant for Ethernet policers (has to be FALSE).
  */
  uint8 disable_eir;
  /*
   *  Committed Information Rate, in Kbps
   */
  uint32 cir;
  /*
   *  Excess Information Rate, in Kbps
   */
  uint32 eir;
  /*
   *  Max Committed Information Rate, in Kbps
   *  Used only when sharing is enabled
   *  ARAD only.
   */
  uint32 max_cir;
  /*
   *  Max Excess Information Rate, in Kbps
   *  ARAD only.
   */
  uint32 max_eir;
  /*
   *  Committed Burst Size, in bytes
   */
  uint32 cbs;
  /*
   *  Excess Burst Size, in bytes
   */
  uint32 ebs;
  /*
   *  Indicates whether the color-aware or color-blind mode is
   *  employed at the metered traffic. In T20E has to be color
   *  blind, error is returned otherwise.
   */

  SOC_PPC_MTR_COLOR_MODE color_mode;
  /*
   *  When TRUE, work in fairness mode. In fairness mode a
   *  packet is granted (colored) without dependence on its
   *  size. In non-fairness mode packet is colored depending
   *  on it size against the available credits in the bucket
   *  (standard behavior).
   *  In T20E and ARAD has to be FALSE, error is returned otherwise.
   */
  uint8 is_fairness_enabled;
  /*
   *  When TRUE, work in coupling mode. In coupling mode
   *  excess credits from the Committed Leaky Bucket is passed
   *  to the Excess Leaky Bucket. In non- coupling mode excess
   *  credits from the Committed Leaky Bucket are discarded
   *  (standard behavior). In T20E has to be FALSE, error is
   *  returned otherwise. Remarks:1. In coupling mode, the long
   *  term average bit rate of bytes in yellow service frames
   *  admitted to the network is bounded by EIR. 2. In
   *  non-coupling mode, the long term average bit rate of
   *  bytes in yellow Service Frames admitted to the network
   *  is bounded by CIR + EIR depending on volume of the
   *  offered green Service Frames.3. SrTcm (Single Rate Three
   *  color meter) is achieved by enable the coupling mode and
   *  set EIR to zero.
   */
  uint8 is_coupling_enabled;
  /*
   *  When TRUE, work in sharing mode. In sharing mode, meters
   *  are part of a hierarchical quad meters. Bucket will get
   *  credits only from higher hierarchy bucket with SharingFlag set.
   *  When FALSE, all extra credit from all shared buckets will be
   *  summed and spread according priority.
   *  ARAD only.
   */
  uint8 is_sharing_enabled;

  /*
   *  Enable this metering profile,
   *  If TRUE then packet associated to this profile is
   *  policed according to the profile rate. If FALSE then
   *  packet is not policed.
   *  Relevant only for Ethernet poilcers, in Meters has to be TRUE
  */
  uint8 is_mtr_enabled;

  /*
   *  Enable packet mode,
   *  If TRUE then policing according to packet profile rate 
   *  If FALSE then policing according to bytes profile rate.
   *  Relevant only for Ethernet poilcers.
  */
  uint8 is_packet_mode;

    /*
   *  Enable packet adjust with header truncate,
   *  If TRUE then packet size will be considere of the header truncate size in irpp
   *  If FALSE then policing without header truncate size
   *  Relevant only for Ethernet poilcers and Meters.
  */
  uint8 is_pkt_truncate;

/*
    for cascade mode (sharing enable TRUE), need to have same rev exp for all buckets.
    therefore, we calculate the rev_exp in advance according to all buckets in the cascade and send it to the soc level.
    for other cases, it will be 0xFF.
*/
  uint32 ir_rev_exp;

  /* define if the color of the meter will be black or red in case no committed or excess credits left (relevant for QUX only)*/
  uint32 mark_black; 
  
} SOC_PPC_MTR_BW_PROFILE_INFO;

typedef struct
{
	SOC_SAND_MAGIC_NUM_VAR

	/* color decision flags */
	uint32 flags;

	/* policer-0 color decision */
    bcm_policer_color_t policer0_color; 

	/* if policer0_color is yellow,
	   indicate if green bucket has credits,
	   otherwise if yellow bucket has credits
	   */
    uint32 policer0_other_bucket_has_credits; 

	/* policer-1 color decision */
    bcm_policer_color_t policer1_color;

	/* if policer1_color is yellow,
	   indicate if green bucket has credits,
	   otherwise if yellow bucket has credits */
    uint32 policer1_other_bucket_has_credits;

    /* defines if drop will marked as black or red. relevant for QUX only */
    uint32 policer0_mark_black; 
    uint32 policer1_mark_black; 

	/* the policer-0 bucket to update */
    bcm_policer_color_t policer0_update_bucket; 

	/* the policer-1 bucket to update */
    bcm_policer_color_t policer1_update_bucket;

	/* policer color decision */
    bcm_color_t policer_color;          

} SOC_PPC_MTR_COLOR_DECISION_INFO;

typedef struct
{
	SOC_SAND_MAGIC_NUM_VAR

	/* color resolution flags */
	uint32 flags;
	
	/* policer command */              
    uint32 policer_action;
	
	/* incoming color */         
    bcm_color_t incoming_color;
	
	/* ethernet policer color, Jericho only*/ 
    bcm_color_t ethernet_policer_color;

	/* policer color */
    bcm_color_t policer_color;

	/* ingress color */
    bcm_color_t ingress_color;

	/* egress color */
    bcm_color_t egress_color;

} SOC_PPC_MTR_COLOR_RESOLUTION_INFO;


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
  SOC_PPC_MTR_METER_ID_clear(
    SOC_SAND_OUT SOC_PPC_MTR_METER_ID *info
  );

void
  SOC_PPC_MTR_GROUP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MTR_GROUP_INFO *info
  );

void
  SOC_PPC_MTR_BW_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_PPC_MTR_BW_PROFILE_INFO *info
  );

void
  SOC_PPC_MTR_COLOR_DECISION_INFO_clear(
	SOC_SAND_OUT SOC_PPC_MTR_COLOR_DECISION_INFO *info
  );

void
  SOC_PPC_MTR_COLOR_RESOLUTION_INFO_clear(
	SOC_SAND_OUT SOC_PPC_MTR_COLOR_RESOLUTION_INFO *info
  );


#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_MTR_ETH_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_MTR_ETH_TYPE enum_val
  );

const char*
  SOC_PPC_MTR_COLOR_MODE_to_string(
    SOC_SAND_IN  SOC_PPC_MTR_COLOR_MODE enum_val
  );

const char*
  SOC_PPC_MTR_RES_USE_to_string(
    SOC_SAND_IN  SOC_PPC_MTR_RES_USE enum_val
  );

const char*
  SOC_PPC_MTR_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_MTR_TYPE enum_val
  );

const char*
  SOC_PPC_MTR_MODE_to_string(
    SOC_SAND_IN  bcm_policer_mode_t enum_val
  );

void
  SOC_PPC_MTR_METER_ID_print(
    SOC_SAND_IN  SOC_PPC_MTR_METER_ID *info
  );

void
  SOC_PPC_MTR_GROUP_INFO_print(
    SOC_SAND_IN  SOC_PPC_MTR_GROUP_INFO *info
  );

void
  SOC_PPC_MTR_BW_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_PPC_MTR_BW_PROFILE_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_METERING_INCLUDED__*/
#endif

