/* $Id: tmc_api_cnm.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_TMC_API_CNM_INCLUDED__
/* { */
#define __SOC_TMC_API_CNM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_pp_mac.h>

#include <soc/dpp/TMC/tmc_api_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Number of sampling bases in a CP profile entry          */
#define  SOC_TMC_CNM_NOF_SAMPLING_BASES (8)

/*     Number of uint32s in the CP Id field in the PDU header    */
#define  SOC_TMC_CNM_NOF_UINT32S_IN_CP_ID (2)

/*     Number of CP profiles                                   */
#define  SOC_TMC_CNM_NOF_PROFILES (8)

/*     Maximum number of CP Queues                             */
#define  SOC_TM_CNM_MAX_CP_QUEUES      (8*1024)
/* $Id: tmc_api_cnm.h,v 1.7 Broadcom SDK $
 * Can be used as dest_tm_port (CNM_PACKET structure).
 * In this case, the destination port will be the same port as
 * the incoming port for CNM message
 */
#define  SOC_TCM_CNM_DEST_TM_PORT_AS_INCOMING  (0xffffffff)

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
   *  Generate CNM information packet including in the FTMH
   *  header all the data to allow an external egress PP to
   *  create a CNM packet.
   */
  SOC_TMC_CNM_GEN_MODE_EXT_PP = 0,
  /*
   *  CNM information packet including in the FTMH header all
   *  the data to allow the Dune PP to create a CNM packet.
   */
  SOC_TMC_CNM_GEN_MODE_PETRA_B_PP = 1,
  /*
   *  Generate a CNM information packet according to the
   *  sampling rate defined per CP Queue. The CNM information
   *  packet is sent to an external egress PP which decides if
   *  a CNM packet is generated.
   */
  SOC_TMC_CNM_GEN_MODE_SAMPLING = 2,

  /* Same as Petra-B PP */
  SOC_TMC_CNM_GEN_MODE_DUNE_PP = 3,

  /* HiGig Mode  - ARAD ONLY */
  SOC_TMC_CNM_GEN_MODE_HIGIG = 4,

  /*
   *  Number of types in SOC_TMC_CNM_GEN_MODE
   */
  SOC_TMC_CNM_NOF_GEN_MODES
}SOC_TMC_CNM_GEN_MODE;

typedef enum
{
  /*
   *  For each destination system port, the CP queues are
   *  mapped the 8 consecutive VOQs (one per traffic class).
   */
  SOC_TMC_CNM_Q_SET_8_CPS = 0,
  /*
   *  For each destination system port, the CP queues are
   *  mapped to the 4 even queues from 8 consecutive VOQs (0,
   *  2, 4 and 6). This set must be chosen if the CP
   *  functionality is supposed to be supported by more than
   *  512 OFPs (and less than 1K).
   */
  SOC_TMC_CNM_Q_SET_4_CPS = 1,
  /*
   *  Number of types in SOC_TMC_CNM_Q_SET
   */
  SOC_TMC_CNM_NOF_Q_SETS = 2
}SOC_TMC_CNM_Q_SET;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Congestion point queue base in the mapping.
   */
  uint32 q_base;
  /*
   *  The three types of queue sets.
   */
  SOC_TMC_CNM_Q_SET q_set;
  /*
   *  Number of CP queues. If the packet generation mode is
   *  'SAMPLING' and the type of queue sets is '8_CPS', then
   *  the number of CP queues must be inferior to 4K.
   *  Otherwise, Range: 0 - 8K.
   */
  uint32 nof_queues;

} SOC_TMC_CNM_Q_MAPPING_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then CNM packets are generated also for packets
   *  with MAC-DA of type multicast or broadcast.
   */
  uint8 is_mc_also;
  /*
   *  If True, then CNM packets are generated also for ingress
   *  replication multicast packets.
   */
  uint8 is_ingr_rep_also;
  /*
   *  If True, then CNM packets are generated also for snooped
   *  copies.
   */
  uint8 is_snoop_also;
  /*
   *  If True, then CNM packets are generated also for
   *  mirrored copies.
   */
  uint8 is_mirr_also;

} SOC_TMC_CNM_CONGESTION_TEST_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The Equilibrium (target) size of the CP queue. Must be a
   *  multiple of 128 bytes. Default value: 26000 Bytes.
   *  Units: Bytes. Range: 0 - (4K-1)*128.
   */
  uint32 q_eq;
  /*
   *  The signed power of the Qdelta parameter during the
   *  computation of the feedback value - CpW = 2 ^ cpw_power.
   *  Default value: 1. Units: Bytes. Range: -15 - 15.
   */
  int32 cpw_power;
  /*
   *  The minimum number of enqueued bytes between two
   *  generations of a CNM packet. Must be a multiple of 64
   *  bytes. Default value: [150,000; 75,000; 50,000; 37,500;
   *  30,000; 25,000; 21,500; 18,500]. Units: Bytes. Range: 0 -
   *  (8K-1)*64.
   */
  uint32 sampling_base[SOC_TMC_CNM_NOF_SAMPLING_BASES];
  /*
   *  The maximum negative value allowed to the CP feedback.
   *  Must be a multiple of 128 bytes. Units: Bytes. Range: 0 -
   *  (16K-1)*128.
   */
  uint32 max_neg_fb_value;
  /*
   *  The unsigned division factor between the calculated
   *  feedback and the quantized feedback sent on the CNM
   *  packet. Range: 0 - 15.
   */
  uint32 quant_div;
  /*
   *  If True, then the CP sampling threshold is modified
   *  according to a random factor located between 0.85 and
   *  1.15.
   */
  uint8 is_sampling_th_random;

} SOC_TMC_CNM_CP_PROFILE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, then the Congestion Point enables the
   *  generation of CNM packets on this CP queue.
   */
  uint8 is_cp_enabled;
  /*
   *  Congestion Point profile. Range: 0 - 7.
   */
  uint32 profile;

} SOC_TMC_CNM_CPQ_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Ingress VLAN Edit Command for all the CNM packets.
   *  Range: 0 - 63.
   */
  uint32 ing_vlan_edit_cmd;

} SOC_TMC_CNM_PPH;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The Ethernet type of the packet to indicate that the
   *  packet is a CNM packet.
   */
  uint32 ether_type;
  /*
   *  The Version field of the PDU header. Range: 0 - 15.
   */
  uint32 version;
  /*
   *  The Reserved-V field of the PDU header. Range: 0 - 63.
   */
  uint32 res_v;
  /*
   *  The 6 MSB bytes of the CP-Id field of the PDU header.
   */
  uint32 cp_id_6_msb[SOC_TMC_CNM_NOF_UINT32S_IN_CP_ID];

} SOC_TMC_CNM_PDU;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  The source MAC address to be encapsulated in the CNM
   *  packets.
   */
  SOC_SAND_PP_MAC_ADDRESS mac_sa;
  /*
   *  Fields of the PPH header in the generated CNM Packet
   */
  SOC_TMC_CNM_PPH pph;
  /*
   *  Fields of the PDU header in the generated CNM Packet
   */
  SOC_TMC_CNM_PDU pdu;
  /*
   *  The Ethernet type used to tag packets with a CN-TAG
   *  (i.e., CN-TAG Ethernet type).
   */
  uint32 ether_type;

} SOC_TMC_CNM_PETRA_B_PP;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  Destination TM-Port.
   *  The CNM packet will be sent through this port.
   *  Range: 0 - 79, or SOC_TCM_CNM_DEST_TM_PORT_AS_INCOMING.
   */
  uint32 dest_tm_port;
  /*
   *  Traffic class of the generated CNM packet (not dependent
   *  on the original traffic class). Range: 0 - 7.
   */
  uint32 tc;
  /*
   *  Value of the 4 MSB bits in the CP-Id field. Range: 0 -
   *  15. In the 'EXT_PP' mode, the CP-Id field is in the CNM
   *  extension header and CP-Id = 1 << (cp_id_4_msb + 12) +
   *  CP-Index (received from the CP Queue). In the 'SOC_PETRA_B_PP'
   *  mode, the CP-Id field is in the PDU header and CP-Id =
   *  PDU.cp_id_6_msb + 1 << (cp_id_4_msb + 12) + CP-Index.
   */
  uint32 cp_id_4_msb;

  /* Drop Precedence of the packet */
  uint32 dp;

  /* The signature of the CNM Packet */
  /* Range: 0-3                      */
  uint32 qsig;

} SOC_TMC_CNM_PACKET;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  If True, the Congestion Point functionality is enabled.
   *  The ingress shaping must be then disabled.
   */
  uint8 is_cp_enabled;
  /*
   *  CNM Packet generation mode.
   */
  SOC_TMC_CNM_GEN_MODE pkt_gen_mode;
  /*
   *  CNM Packet fields - Must be set according to the CNM
   *  Packet generation mode.
   *  PETRA B ONLY
   */
  SOC_TMC_CNM_PACKET pckt;
  /*
   *  Fields of the CNM in the Dune PP header extensions. Relevant
   *  only if 'pkt_gen_mode' is 'Dune PP'.
   *  PETRA B ONLY
   */
  SOC_TMC_CNM_PETRA_B_PP pp;

} SOC_TMC_CNM_CP_INFO;

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
  SOC_TMC_CNM_Q_MAPPING_INFO_clear(
    SOC_SAND_OUT SOC_TMC_CNM_Q_MAPPING_INFO *info
  );

void
  SOC_TMC_CNM_CONGESTION_TEST_INFO_clear(
    SOC_SAND_OUT SOC_TMC_CNM_CONGESTION_TEST_INFO *info
  );

void
  SOC_TMC_CNM_CP_PROFILE_INFO_clear(
    SOC_SAND_OUT SOC_TMC_CNM_CP_PROFILE_INFO *info
  );

void
  SOC_TMC_CNM_CPQ_INFO_clear(
    SOC_SAND_OUT SOC_TMC_CNM_CPQ_INFO *info
  );

void
  SOC_TMC_CNM_PPH_clear(
    SOC_SAND_OUT SOC_TMC_CNM_PPH *info
  );

void
  SOC_TMC_CNM_PDU_clear(
    SOC_SAND_OUT SOC_TMC_CNM_PDU *info
  );

void
  SOC_TMC_CNM_PETRA_B_PP_clear(
    SOC_SAND_OUT SOC_TMC_CNM_PETRA_B_PP *info
  );

void
  SOC_TMC_CNM_PACKET_clear(
    SOC_SAND_OUT SOC_TMC_CNM_PACKET *info
  );

void
  SOC_TMC_CNM_CP_INFO_clear(
    SOC_SAND_OUT SOC_TMC_CNM_CP_INFO *info
  );

#if SOC_TMC_DEBUG_IS_LVL1

const char*
  SOC_TMC_CNM_GEN_MODE_to_string(
    SOC_SAND_IN  SOC_TMC_CNM_GEN_MODE enum_val
  );

const char*
  SOC_TMC_CNM_Q_SET_to_string(
    SOC_SAND_IN  SOC_TMC_CNM_Q_SET enum_val
  );

void
  SOC_TMC_CNM_Q_MAPPING_INFO_print(
    SOC_SAND_IN  SOC_TMC_CNM_Q_MAPPING_INFO *info
  );

void
  SOC_TMC_CNM_CONGESTION_TEST_INFO_print(
    SOC_SAND_IN  SOC_TMC_CNM_CONGESTION_TEST_INFO *info
  );

void
  SOC_TMC_CNM_CP_PROFILE_INFO_print(
    SOC_SAND_IN  SOC_TMC_CNM_CP_PROFILE_INFO *info
  );

void
  SOC_TMC_CNM_CPQ_INFO_print(
    SOC_SAND_IN  SOC_TMC_CNM_CPQ_INFO *info
  );

void
  SOC_TMC_CNM_PPH_print(
    SOC_SAND_IN  SOC_TMC_CNM_PPH *info
  );

void
  SOC_TMC_CNM_PDU_print(
    SOC_SAND_IN  SOC_TMC_CNM_PDU *info
  );

void
  SOC_TMC_CNM_PETRA_B_PP_print(
    SOC_SAND_IN  SOC_TMC_CNM_PETRA_B_PP *info
  );

void
  SOC_TMC_CNM_PACKET_print(
    SOC_SAND_IN  SOC_TMC_CNM_PACKET *info
  );

void
  SOC_TMC_CNM_CP_INFO_print(
    SOC_SAND_IN  SOC_TMC_CNM_CP_INFO *info
  );

#endif /* SOC_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_TMC_API_CNM_INCLUDED__*/
#endif
