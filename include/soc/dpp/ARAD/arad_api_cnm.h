/*
 * $Id: arad_api_cnm.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __ARAD_API_CNM_INCLUDED__
/* { */
#define __ARAD_API_CNM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_general.h>
#include <soc/dpp/TMC/tmc_api_cnm.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of sampling bases in a CP profile entry          */
#define  ARAD_CNM_NOF_SAMPLING_BASES  (SOC_TMC_CNM_NOF_SAMPLING_BASES)

/*     Number of CP profiles                                   */
#define  ARAD_CNM_NOF_PROFILES        (SOC_TMC_CNM_NOF_PROFILES)

/*     Maximum number of CP Queues                             */
#define  ARAD_CNM_MAX_CP_QUEUES      SOC_TM_CNM_MAX_CP_QUEUES

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

typedef SOC_TMC_CNM_CP_INFO             ARAD_CNM_CP_INFO;
typedef SOC_TMC_CNM_CPQ_INFO            ARAD_CNM_CPQ_INFO;
typedef SOC_TMC_CNM_CP_PROFILE_INFO     ARAD_CNM_CP_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  First CP Queue - max number of queues is 8K
   */
  uint32 cp_queue_low;
  /*
   *  Last CP Queue - max number of queues is 8K
   */
  uint32 cp_queue_high;
  /*
   *  The types of queue sets (8 or 4).
   */
  SOC_TMC_CNM_Q_SET q_set;
  /*
   *  If set - selects the odd queues from the range configured.
   *  If not - selects the even queues from the range configured.
   *  Relevant only when q_set is SOC_TMC_CNM_Q_SET_4_CPS.
   *  Ignored if q_set is SOC_TMC_CNM_Q_SET_8_CPS.
   */
  uint8 odd_queues_select;

} ARAD_CNM_Q_MAPPING_INFO;

typedef struct
{
  /* The source port value in the FTMH */
  /* Set to -1 to use original source port */
  uint32 ftmh_src_port;

  /* The traffic class value in the FTMH */
  uint32 ftmh_tc;

  /* The DP value in the FTMH */
  uint32 ftmh_dp;

  /* MAC Source Address of the CNM Packet */
  SOC_SAND_PP_MAC_ADDRESS mac_sa;

  /* The ether type of the CNM packet */
  uint32 ether_type;

  /* CNM Packet's PDU Version */
  uint32 pdu_version;

  /* CNM Packet's PDU Reserved Version */
  uint32 pdu_reserved_v;

  /* OTM Port */
  /* Set to -1 if OTM is taken from IFP mapping */
  uint32 ftmh_otm_port;

  /* VLAN */
  uint32 vlan_edit_command_with_cn_tag;
  uint32 vlan_edit_command_without_cn_tag;
  uint32 has_vlan_config;
  uint32 vlan_id;
  uint32 vlan_cfi;
  uint32 vlan_priority;

}ARAD_CNM_CP_PACKET_INFO;


typedef struct
{
  /* Disables CN TAG in the generated CNM packet, when there is Cn-Tag in the dram. */
  uint8 disable_cn_tag_if_exist_in_dram;

  /* If set, then the IPT generates packet when there is no CN Tag in the original packet.
     Otherwise, CNM packet is not generated. */
  uint8 gen_if_no_cn_tag;

  /* If set, the IPT adds the cfg cn-tag from the "CN Tag Info" below, 
     when there is no cn-tag in the dram, but gen_if_no_cn_tag is set, 
     or if CnTag is in the dram, but disable_cn_tag_if_exist_in_dram is set. */
  uint8 add_cn_tag;

  /* This is relevant only in Full mode.
     If set, then the IPT generates packet when there is no PPH in the original packet. 
     Otherwise, CNM packet is not generated. */
  uint8 gen_if_no_pph;

  /* CN Tag info (used if CN Tag is generated) */
  uint32 cn_tag_ethertype;
  uint32 cn_tag_flow_id;

  /* If set then IQM will generate CNM for ingress replication multicast packets, 
     else those packets are ignored in CNM logic. */
  uint8 gen_if_replications;

#if defined(BCM_88650_B0)
  /* --- ARAD B0 FIXES --- */

  /* If set, fabric CRC for CNM packets will not be calculated and will be stamped to constant value */
  uint8 disable_fabric_crc_calculation;

  /* If set and CNM sampled packet size is less than configured 
     then DRAM CRC will be copied into the packet payload. */
  uint8 disable_crc_removal;

  /* If set then IQM will generate CNM for packets using OCB buffer, 
     else, those packets are ignored in CNM logic. */
  uint8 enable_cnm_ocb;
#endif

}ARAD_CNM_CP_OPTIONS;

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
 *   arad_cnm_cp_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Congestion Point functionality, in particular
 *   the packet generation mode and the fields of the CNM
 *   packet.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_CP_INFO                *info -
 *     Attributes of the congestion point functionality
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  ARAD_CNM_CP_INFO                    *info
  );

uint32
  arad_cnm_cp_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT ARAD_CNM_CP_INFO                    *info
  );


/*********************************************************************
* NAME:
 *   arad_cnm_queue_mapping_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the mapping from the pair (destination, traffic
 *   class) to the CP Queue.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_Q_MAPPING_INFO                  *info -
 *     Attributes of the CP Queue mapping
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_queue_mapping_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  ARAD_CNM_Q_MAPPING_INFO             *info
  );

uint32
  arad_cnm_queue_mapping_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT ARAD_CNM_Q_MAPPING_INFO             *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cpq_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the CP Queue attributes into an entry of the
 *   Congestion Point Table.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                              cpq_ndx -
 *     Index of the CP Queue. Range: 0 - 4K-1.
 *   SOC_SAND_IN  ARAD_CNM_CPQ_INFO                        *info -
 *     Attributes of the congestion point profile entry
 * REMARKS:
 *   Relevant only if not in a sampling mode.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cpq_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              cpq_ndx,
    SOC_SAND_IN  ARAD_CNM_CPQ_INFO                   *info
  );

uint32
  arad_cnm_cpq_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  uint32                              cpq_ndx,
    SOC_SAND_OUT ARAD_CNM_CPQ_INFO                   *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cp_profile_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure an entry into the Congestion Point Profile
 *   Table.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                   profile_ndx -
 *     Index of the profile in the Congestion Point profile
 *     table. Range: 0 - 7.
 *   SOC_SAND_IN  ARAD_CNM_CP_PROFILE_INFO      *info -
 *     Attributes of the congestion point profile entry.
 * REMARKS:
 *   Relevant only if not in a sampling mode (set in
 *   cnm_cp_set API). The terminology used in this API
 *   corresponds to the 802.1 Qau standard of Congestion
 *   Notification.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_profile_set(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                         profile_ndx,
    SOC_SAND_IN  ARAD_CNM_CP_PROFILE_INFO       *info
  );

uint32
  arad_cnm_cp_profile_get(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                         profile_ndx,
    SOC_SAND_OUT ARAD_CNM_CP_PROFILE_INFO       *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cp_packet_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Configure the CNM packet properties (such as FTMH, MAC SA, etc)
 *   Table.
 * INPUT:
 *   SOC_SAND_IN  int                   unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_CP_PACKET_INFO      *info -
 *     Attributes of the congestion point profile entry.
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_packet_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  ARAD_CNM_CP_PACKET_INFO       *info
  );

uint32
  arad_cnm_cp_packet_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_OUT ARAD_CNM_CP_PACKET_INFO       *info
  );

/*********************************************************************
* NAME:
 *   arad_cnm_cp_options_set
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Set the Congestion Point generation options.
 * INPUT:
 *   SOC_SAND_IN  int                              unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  ARAD_CNM_CP_OPTIONS                *info -
 *     Options for the congestion point functionality
 * REMARKS:
 *   None.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  arad_cnm_cp_options_set(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_IN  ARAD_CNM_CP_OPTIONS                 *info
  );

uint32
  arad_cnm_cp_options_get(
    SOC_SAND_IN  int                              unit,
    SOC_SAND_OUT ARAD_CNM_CP_OPTIONS                 *info
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_API_CNM_INCLUDED__*/
#endif
