/* $Id: ppc_api_oam.h,v 1.42 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_PPC_API_OAM_INCLUDED__
/* { */
#define __SOC_PPC_API_OAM_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>
#include <soc/register.h>
#include <bcm/oam.h>

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

#define SOC_PPC_OAM_IS_MEP_TYPE_BFD(mep_type) \
  ((mep_type==SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP) || (mep_type==SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP) || \
   (mep_type==SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS) || (mep_type==SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE)              || \
   (mep_type==SOC_PPC_OAM_MEP_TYPE_BFDCC_O_MPLSTP) || (mep_type==SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE_GAL))

#define SOC_PPC_OAM_IS_MEP_TYPE_Y1731(mep_type) \
  ((mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP) || (mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE) || \
   (mep_type==SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL))

/* icc map data is bytes 5:0 of ma_name */
#define SOC_PPC_OAM_GROUP_NAME_TO_ICC_MAP_DATA(ma_name, data)              \
  COMPILER_64_SET(data, ma_name[1]+(ma_name[0]<<8),                        \
    ma_name[5]+(ma_name[4]<<8)+(ma_name[3]<<16)+(ma_name[2]<<24))

/*    CCM Interval (field)      Transmission Period
*        0                            -
*        1                         3.333ms
*        2                         10ms
*        3                         100ms
*        4                         1s
*        5                         10s
*        6                         1min
*        7                         10min 
*/
#define SOC_PPC_OAM_CCM_PERIOD_TO_CCM_INTERVAL_FIELD(ccm_period_ms, ccm_period_micro_s, ccm_interval) \
do {                                                                                               \
      if (ccm_period_ms==0) {                \
        ccm_interval = 0;\
      }\
      else if (ccm_period_ms > 5*60*1000 /*5 min*/) {                                                   \
        ccm_interval = 0x7;                                                                        \
      }                                                                                            \
      else if (ccm_period_ms > 35*1000 /*35 sec*/) {                                               \
        ccm_interval = 0x6;                                                                        \
      }                                                                                            \
      else if (ccm_period_ms > 5 *1500 /*5.5 sec*/) {                                            \
        ccm_interval = 0x5;                                                                        \
      }                                                                                            \
      else if (ccm_period_ms > 550 /*550 ms*/) {                                                   \
        ccm_interval = 0x4;                                                                        \
      }                                                                                            \
      else if (ccm_period_ms > 55 /*55 ms*/) {                                                     \
        ccm_interval = 0x3;                                                                        \
      }                                                                                            \
      else if (ccm_period_ms > 7 || (ccm_period_ms > 6 && ccm_period_micro_s > 660) /*6.66 ms*/) { \
        ccm_interval = 0x2;                                                                        \
      }                                                                                            \
      else {                                                                                       \
        ccm_interval = 0x1;                                                                        \
      }                                                                                            \
} while (0)

#define SOC_PPC_OAM_CCM_PERIOD_FROM_CCM_INTERVAL_FIELD(ccm_period_ms, ccm_period_micro_s, ccm_interval)   \
do {                                                                               \
      ccm_period_micro_s = 0;                                                      \
      switch (ccm_interval) {                                                      \
      case 0x0: /*0ms*/                                                            \
          ccm_period_ms = 0;                                                       \
          break;                                                                   \
      case 0x1: /*3.333ms*/                                                        \
          ccm_period_ms = 3;                                                       \
          ccm_period_micro_s = 333;                                                \
          break;                                                                   \
      case 0x2: /*10ms*/                                                           \
          ccm_period_ms = 10;                                                      \
          break;                                                                   \
      case 0x3: /*100ms*/                                                          \
          ccm_period_ms = 100;                                                     \
          break;                                                                   \
      case 0x4: /*1s*/                                                             \
          ccm_period_ms = 1*1000/*ms*/;                                            \
          break;                                                                   \
      case 0x5: /*10s*/                                                            \
          ccm_period_ms = 10*1000/*ms*/;                                           \
          break;                                                                   \
      case 0x6: /*1min*/                                                           \
          ccm_period_ms = 60*1000/*ms*/;                                           \
          break;                                                                   \
      default: /*10min*/                                                           \
          ccm_period_ms = 10*60*1000/*ms*/;                                        \
      }                                                                            \
} while (0)


#define PPC_API_OAM_STORE_LOCAL_PORT_IN_MEP_DB(mep_type)       \
  ((mep_type == SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP) ||       \
   (mep_type == SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP) ||       \
   (mep_type == SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS) ||             \
   (mep_type == SOC_PPC_OAM_MEP_TYPE_ETH_OAM))

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef uint32   SOC_PPC_OAM_ETH_ACC_MEP_ID;

typedef enum
{
  /*
   *  MIP
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_0 = 0,
  /*
   *  Non accelerated MEP
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_1 = 1,
  /*
   *  Accelerated MEP (CCM)
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_2 = 2,
  /*
   *  Accelerated MEP (DLM)
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_3 = 3,
  /*
   *  Accelerated MEP (TST)
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_4 = 4,
  /*
   *  Accelerated MEP (CCM, DLM)
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_5 = 5,
  /*
   *  Number of types in SOC_PPC_OAM_ETH_ACC_FUNC_TYPE
   */
  SOC_PPC_OAM_NOF_ETH_ACC_FUNC_TYPES = 6
}SOC_PPC_OAM_ETH_ACC_FUNC_TYPE;

typedef enum
{
  /*
   *  Maintenance point level 0
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_0 = 0,
  /*
   *  Maintenance point level 1
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_1 = 1,
  /*
   *  Maintenance point level 2
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_2 = 2,
  /*
   *  Maintenance point level 3
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_3 = 3,
  /*
   *  Maintenance point level 4
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_4 = 4,
  /*
   *  Maintenance point level 5
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_5 = 5,
  /*
   *  Maintenance point level 6
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_6 = 6,
  /*
   *  Maintenance point level 7
   */
  SOC_PPC_OAM_ETH_MP_LEVEL_7 = 7,
  /*
   *  Number of types in SOC_PPC_OAM_ETH_MP_LEVEL
   */
  SOC_PPC_OAM_NOF_ETH_MP_LEVELS = 8
}SOC_PPC_OAM_ETH_MP_LEVEL;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
  /*
   *  T - Enable MP for this ACF - Disable MP for this AC
   */
  uint8 valid;
  /*
   *  MP type. Determines the way an OAM packet is handled
   *  when being received through the specified attachment
   *  circuit
   */
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE func_type;
  /*
   *  The accelerated MEP handle to associate with the
   *  specified attachment circuit and level. Relevant only to
   *  APIs of accelerated MEPs. (E.g. CCM). Range: 0-4K
   */
  SOC_PPC_OAM_ETH_ACC_MEP_ID acc_mep_id;

} SOC_PPC_OAM_ETH_MP_INFO;



/* BFD opcode */
#define SOC_PPC_BFD_PDU_OPCODE  0

/* Ethernet 1731 opcodes */
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_CCM  1
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LBR  2 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LBM  3 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LTR  4
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LTM  5
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_AIS  33
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LCK  35
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_TST  37
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LINEAR_APS  39
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LMR  42
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_LMM  43 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_1DM  45 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_DMR  46
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_DMM  47 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_EXR  48 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_EXM  49 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_CSF  52 
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_SLR  54
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_SLM  55
#define SOC_PPC_OAM_ETHERNET_PDU_OPCODE_COUNT 256

/*Update below macro when a new OPCODE is added to above macros*/
/*Below macro is used for dump usage purpose - to indicate valid ETH OAM OPCODE*/
#define SOC_PPC_OAM_ETHERNET_PDU_DUMMY_MAX_DIAG_OPCODE SOC_PPC_OAM_ETHERNET_PDU_OPCODE_SLM

/* internal 4-bit opcodes map */
typedef enum
{

    SOC_PPC_OAM_OPCODE_MAP_BFD        = 0,
    SOC_PPC_OAM_OPCODE_MAP_CCM        = 1,
    SOC_PPC_OAM_OPCODE_MAP_LBR        = 2,
    SOC_PPC_OAM_OPCODE_MAP_LBM        = 3,
    SOC_PPC_OAM_OPCODE_MAP_LTR        = 4,
    SOC_PPC_OAM_OPCODE_MAP_LTM        = 5,
    SOC_PPC_OAM_OPCODE_MAP_LMR        = 6,
    SOC_PPC_OAM_OPCODE_MAP_LMM        = 7,
    SOC_PPC_OAM_OPCODE_MAP_DMR        = 8,
    SOC_PPC_OAM_OPCODE_MAP_DMM        = 9,
    SOC_PPC_OAM_OPCODE_MAP_1DM        = 10,
    SOC_PPC_OAM_OPCODE_MAP_SLM_SLR    = 11,
    SOC_PPC_OAM_OPCODE_MAP_SLM        = 11,
    SOC_PPC_OAM_OPCODE_MAP_SLR        = 11,
    SOC_PPC_OAM_OPCODE_MAP_AIS        = 12,
    SOC_PPC_OAM_OPCODE_MAP_LCK        = 13,
    SOC_PPC_OAM_OPCODE_MAP_LINEAR_APS = 14,
    SOC_PPC_OAM_OPCODE_MAP_DEFAULT    = 15,
    SOC_PPC_OAM_OPCODE_MAP_COUNT      = 16/* number of ethernet internal opcodes */

}SOC_PPC_OAM_INTERNAL_OPCODE;

#define SOC_PPC_OAM_NON_ACC_PROFILES_NUM 4
#define SOC_PPC_OAM_NON_ACC_PROFILES_ARAD_PLUS_NUM 16
#define SOC_PPC_OAM_ACC_PROFILES_NUM     16
#define SOC_PPC_OAM_PROFILE_DEFAULT 0

#define SOC_PPC_OAM_MAX_PROTECTION_HEADER_SIZE 80
#define SOC_PPC_OAM_FULL_PROTECTION_HEADER_SIZE 120

#define SOC_PPC_OAM_MAX_NUMBER_OF_MAS(unit) (SOC_DPP_DEFS_GET(unit,oamp_number_of_meps)) /* assuming at least one mep per ma */
#define SOC_PPC_OAM_MAX_NUMBER_OF_LOCAL_MEPS(unit) (SOC_DPP_DEFS_GET(unit,oamp_number_of_meps)) /* the maximal number of local meps is 8k/16k for Jer */
#define SOC_PPC_OAM_MAX_NUMBER_OF_REMOTE_MEPS(unit) (SOC_DPP_DEFS_GET(unit,oamp_number_of_rmeps)) /* the maximal number of remote meps is 16k/32k for Jer */
#define SOC_PPC_OAM_MAX_NUMBER_OF_ETH1731_MEP_PROFILES(unit) (SOC_DPP_DEFS_GET(unit,oamp_number_of_eth_y1731_mep_profiles)) /*16 for Arad, 128 for Jericho.*/
#define SOC_PPC_OAM_SIZE_OF_UMC_TABLE(unit) (SOC_DPP_DEFS_GET(unit,oamp_umc_table_size)) /*Number of meps that may use lond MA format.*/
#define SOC_PPC_OAM_SIZE_OF_LOCAL_PORT_2_SYS_PORT_TABLE(unit) (SOC_DPP_DEFS_GET(unit,oamp_local_port_2_sys_port_size))

#define SOC_PPC_OAM_SIZE_OF_OAM_KEY_IN_BITS(unit)              (SOC_DPP_DEFS_GET(unit,size_of_oam_key))



#define SOC_PPC_OAM_ICC_MAP_DATA_NOF_BITS (48) /* number of bits of ICC MAP REG field*/


/* Jericho and above. Due to a HW bug this must be 0 in Jericho A0/QMX. */
#define SOC_PPC_OAM_NUMBER_OF_OUTLIF_BITS_USED_BY_PCP               (SOC_DPP_CONFIG(unit)->pp.oam_pcp_egress_prof_num_bits)
#define SOC_PPC_OAM_NUMBER_OF_OUTLIF_BITS_USED_BY_DEFAULT           (SOC_DPP_CONFIG(unit)->pp.oam_default_egress_prof_num_bits)

#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_ACCELERATED                0x1
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_ENDPOINT                   0x2
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_PCP                        0x4
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_DM_1588                    0x8
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_UPMEP                      0x10
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_HAS_LOOPBACK           0x20
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_HAS_LOOPBACK_REPLY     0x40
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_SERVER                     0x80
/* When there is an active on demand DMM session but no proactive session we need a SW indication.*/
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_ONLY_ON_DEMAND_DMM_ACTIVE  0x100
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_USING_ASYMETRIC_MPLS_LIF   0x200
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_BFD_TIMEOUT_SET_EXPLICITLY 0x400
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_DOWNMEP_TX_GPORT_IS_LAG    0x400
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_LOOPBACK_PERIOD_IN_PPS     0x800
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_LOOPBACK_JER           0x1000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_TST_TX_JER             0x2000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_TST_RX_JER             0x4000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_EGRESS_ONLY            0x8000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_SD_SF                  0x10000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MEP_1711_ALARM             0x20000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_TTL_1                      0x40000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_RA                         0x80000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_MPLS_OUTLIF_EGRESS_ONLY    0x100000
#define SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_FLAG_USE_DOUBLE_OUTLIF_INJ      0x200000
/*
 * Y.1711 LM function type
 */
#define SOC_PPC_OAM_Y1711_LM_MPLS          0x01        /* Y1711 LM base on mpls lsp */
#define SOC_PPC_OAM_Y1711_LM_PWE           0x02         /* Y1711 LM base on pwe */


/*
 * Flags on the lif profile (8 bits available)
 */
#define SOC_PPC_OAM_LIF_PROFILE_FLAG_COUNTED                0x01        /* If set, indicates that counter should be stamped on LM packet */
#define SOC_PPC_OAM_LIF_PROFILE_FLAG_SAT_LOOPBACK           0x02        /* If set, use LBM subtype to trap upmep TST/LBR to the SAT */

typedef enum
{
    /* Packet arrived to the OAMP with illegal trap code. */
    SOC_PPC_OAM_OAMP_TRAP_ERR,
    /* Packet arrived to the OAMP with illegal mep type. */
    SOC_PPC_OAM_OAMP_TYPE_ERR,
    /* Packet arrived to the OAMP with RMEP index miss. */
    SOC_PPC_OAM_OAMP_RMEP_ERR,
    /* Packet arrived to the OAMP with MAID miss. */
    SOC_PPC_OAM_OAMP_MAID_ERR,
    /* Packet arrived to the OAMP with MDL miss. */
    SOC_PPC_OAM_OAMP_MDL_ERR,
    /* Packet arrived to the OAMP with CCM interval miss. */
    SOC_PPC_OAM_OAMP_CCM_INTERVAL_ERR,
    /* Packet arrived to the OAMP with My Discriminator miss (BFD). */
    SOC_PPC_OAM_OAMP_MY_DISC_ERR,
    /* Packet arrived to the OAMP with source IP address miss (BFD). */
    SOC_PPC_OAM_OAMP_SRC_IP_ERR,
    /* Packet arrived to the OAMP with Your Discriminator miss (BFD). */
    SOC_PPC_OAM_OAMP_YOUR_DISC_ERR,
    /* Packet arrived to the OAMP with UDP source port miss (BFD). */
    SOC_PPC_OAM_OAMP_SRC_PORT_ERR,
    /* BFD packet arrived that does not match the current RMEP state. */
    SOC_PPC_OAM_OAMP_RMEP_STATE_CHANGE,
    /* Parity error occurred in the OAMP. */
    SOC_PPC_OAM_OAMP_PARITY_ERR,
    /* Packet arrived to the OAMP with timestamp miss. */
    SOC_PPC_OAM_OAMP_TIMESTAMP_ERR,
    /* OAMP protection packet. */
    SOC_PPC_OAM_OAMP_PROTECTION,
    /* OAMP control channel miss match packet. */
    SOC_PPC_OAM_OAMP_CHANNEL_TYPE_ERR,
    /* OAMP Flex CRC miss match packet. */
    SOC_PPC_OAM_OAMP_FLEX_CRC_ERR,
    SOC_PPC_OAM_OAMP_TRAP_TYPE_COUNT
} SOC_PPC_OAM_OAMP_TRAP_TYPE;

typedef enum
{
    /* No such event */
    SOC_PPC_OAM_EVENT_NULL=0,
    /* Loss of continuity detected */
    SOC_PPC_OAM_EVENT_LOC_SET=1,
    /* Loss of continuity not detected yet but expected */
    SOC_PPC_OAM_EVENT_ALMOST_LOC_SET=2,
    /* Loss of continuity clear - signal is back */
    SOC_PPC_OAM_EVENT_LOC_CLR=3,
    /* Remote defect indication set */
    SOC_PPC_OAM_EVENT_RDI_SET=4,
    /* Remote defect indication clear */
    SOC_PPC_OAM_EVENT_RDI_CLR=5,
    /* Remote state change */
    SOC_PPC_OAM_EVENT_RMEP_STATE_CHANGE=6,
    /* Report received - Loss Measurement */
    SOC_PPC_OAM_EVENT_REPORT_RX_LM=7,
    /* Report received - Delay Measurement */
    SOC_PPC_OAM_EVENT_REPORT_RX_DM=8,
    /* SD set */
    SOC_PPC_OAM_EVENT_SD_SET=9,
    /* SD clear */
    SOC_PPC_OAM_EVENT_SD_CLR=10,
    /* SF set */
    SOC_PPC_OAM_EVENT_SF_SET=11,
    /* SF clear */
    SOC_PPC_OAM_EVENT_SF_CLR=12,
    /* dExcess set */
    SOC_PPC_OAM_EVENT_DEXCESS_SET=13,
    /* dMissmatch */
    SOC_PPC_OAM_EVENT_DMISSMATCH=14,
    /* dMissmerge */
    SOC_PPC_OAM_EVENT_DMISSMERGE=15,
    /* dAll clear */
    SOC_PPC_OAM_EVENT_DALL_CLR=16,

    SOC_PPC_OAM_EVENT_COUNT
} SOC_PPC_OAM_EVENT;


/* ARAD Only */
typedef enum {
  SOC_PPC_OAM_MP_TYPE_MATCH = 0,
  SOC_PPC_OAM_MP_TYPE_ABOVE,
  SOC_PPC_OAM_MP_TYPE_BELOW,
  SOC_PPC_OAM_MP_TYPE_BETWEEN,
  SOC_PPC_OAM_MP_TYPE_COUNT
} SOC_PPC_OAM_MP_TYPE;

/* ARAD + Only */
typedef enum {
  SOC_PPC_OAM_MP_TYPE_MEP_OR_ACTIVE_MATCH_PLUS = 0,
  SOC_PPC_OAM_MP_TYPE_BELOW_PLUS,
  SOC_PPC_OAM_MP_TYPE_MIP_OR_PASSIVE_MATCH_PLUS,
  SOC_PPC_OAM_MP_TYPE_ABOVE_PLUS,
  SOC_PPC_OAM_MP_TYPE_COUNT_PLUS
} SOC_PPC_OAM_MP_TYPE_PLUS;

/* Jericho Only */
typedef enum {
    SOC_PPC_OAM_MP_TYPE_JERICHO_MIP_MATCH = 0,
    SOC_PPC_OAM_MP_TYPE_JERICHO_ACTIVE_MATCH = 1,
    SOC_PPC_OAM_MP_TYPE_JERICHO_PASSIVE_MATCH = 2,
    SOC_PPC_OAM_MP_TYPE_JERICHO_BELLOW_HIGHEST_MEP = 3,
    SOC_PPC_OAM_MP_TYPE_JERICHO_ABOVE_ALL = 4,
    SOC_PPC_OAM_MP_TYPE_JERICHO_BFD = 5,
    SOC_PPC_OAM_MP_TYPE_JERICHO_COUNT = 6 
} SOC_PPC_OAM_MP_TYPE_JERICHO; 

/* QAX / Jer+ Only */
typedef enum {
    SOC_PPC_OAM_MP_TYPE_QAX_MIP_MATCH = 0,
    SOC_PPC_OAM_MP_TYPE_QAX_ACTIVE_MATCH = 1,
    SOC_PPC_OAM_MP_TYPE_QAX_PASSIVE_MATCH = 2,
    SOC_PPC_OAM_MP_TYPE_QAX_BETWEEN_MEPS = 3,
    SOC_PPC_OAM_MP_TYPE_QAX_BELOW_ALL = 4,
    SOC_PPC_OAM_MP_TYPE_QAX_ABOVE_ALL = 5,
    SOC_PPC_OAM_MP_TYPE_QAX_BFD = 6,

    SOC_PPC_OAM_MP_TYPE_QAX_COUNT
} SOC_PPC_OAM_MP_TYPE_QAX;


/* Jericho Only 
 * Default endpoints are using the same values as Jericho's MP-Type to mean different things 
 */
typedef enum {
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_INGRESS_MATCH = 0,
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_EGRESS_MATCH = 1,
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_INGRESS_EGRESS_MATCH = 2,
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_PASS = 3,
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_ABOVE_ALL = 4,
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_INVALID = 5,
    SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO_COUNT = 6
} SOC_PPC_OAM_MP_TYPE_FOR_DEFAULT_EP_JERICHO;


typedef enum {
  SOC_PPC_OAM_MEP_TYPE_ETH_OAM = 0,
  SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP = 1,
  SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE = 2,
  SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_1_HOP = 3,
  SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP = 4,
  SOC_PPC_OAM_MEP_TYPE_BFD_O_MPLS = 5,
  SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE = 6,
  SOC_PPC_OAM_MEP_TYPE_BFDCC_O_MPLSTP = 7,
  SOC_PPC_OAM_MEP_TYPE_LM = 8,
  SOC_PPC_OAM_MEP_TYPE_LM_STAT = 9,
  SOC_PPC_OAM_MEP_TYPE_DM = 10,
  SOC_PPC_OAM_MEP_TYPE_EXT_DATA_HDR = 11, /* QAX Only */
  SOC_PPC_OAM_MEP_TYPE_BFD_O_PWE_GAL = 12,  /* Supported only in Arad+.*/
  SOC_PPC_OAM_MEP_TYPE_EXT_DATA_PLD = 12, /* QAX Only */
  SOC_PPC_OAM_MEP_TYPE_DM_ONE_WAY = 13, /* QAX Only */


  /* Note that intentionally gap has been left so that to accomodate any future
   * MEP_DB types that directly correspond to HW key type in MEP_DB.
   * The ones below are used purely a SW MEP_TYPE and doesnt directly correspond
   * to a new key type in MEP_DB.
   */
  SOC_PPC_OAM_MEP_TYPE_Y1731_O_PWE_GAL = 16,
  SOC_PPC_OAM_MEP_TYPE_Y1711_MPLS = 17, /* QAX Only for Y1711 LM,which actually use ..Y1731_O_MPLSTP */
  SOC_PPC_OAM_MEP_TYPE_Y1711_PWE = 18,  /* QAX Only for Y1711 LM ,which actually use ..Y1731_O_PWE */
  /*Even though Y1731_O_PWE MEP_DB type is being used for SECTION OAM, for SW differentiation, this type will be used */
  SOC_PPC_OAM_MEP_TYPE_Y1731_O_MPLSTP_SECTION = 19,
  SOC_PPC_OAM_MEP_TYPE_COUNT /*Should never be greater than 15 
                               for MEP_DB types corresponding 
                               directly to key type in MEP_DB.*/
} SOC_PPC_OAM_MEP_TYPE;

/* As far as the classifier is concerned this may as welll be an IPv4 entry.*/
#define SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV6_M_HOP SOC_PPC_OAM_MEP_TYPE_BFD_O_IPV4_M_HOP

typedef enum {
  SOC_PPC_OAM_LM_DM_ENTRY_TYPE_LM = 0,
  SOC_PPC_OAM_LM_DM_ENTRY_TYPE_LM_STAT,
    SOC_PPC_OAM_LM_DM_ENTRY_TYPE_DM,
  SOC_PPC_OAM_LM_DM_ENTRY_TYPE_NONE,
  SOC_PPC_OAM_LM_DM_ENTRY_TYPE_COUNT
} SOC_PPC_OAM_LM_DM_ENTRY_TYPE;

typedef enum {
  SOC_PPC_OAM_TRAP_ID_OAMP = 0,
  SOC_PPC_OAM_TRAP_ID_CPU,
  SOC_PPC_OAM_TRAP_ID_RECYCLE,
  SOC_PPC_OAM_TRAP_ID_SNOOP,
  SOC_PPC_OAM_TRAP_ID_ERR_LEVEL,
  SOC_PPC_OAM_TRAP_ID_ERR_PASSIVE,
  SOC_PPC_OAM_TRAP_ID_OAMP_Y1731_MPLS,
  SOC_PPC_OAM_TRAP_ID_OAMP_Y1731_PWE,
  SOC_PPC_OAM_TRAP_ID_SAT0_TST,
  SOC_PPC_OAM_TRAP_ID_SAT0_LB,
  SOC_PPC_OAM_TRAP_ID_RECYCLE_LBM,
  SOC_PPC_OAM_TRAP_ID_Y1711_MPLS,
  SOC_PPC_OAM_TRAP_ID_Y1711_PWE,
  SOC_PPC_OAM_TRAP_ID_COUNT
} SOC_PPC_OAM_TRAP_ID;

typedef enum {
  SOC_PPC_OAM_UPMEP_TRAP_ID_OAMP = 0,
  SOC_PPC_OAM_UPMEP_TRAP_ID_CPU,
  SOC_PPC_OAM_UPMEP_TRAP_ID_RECYCLE,
  SOC_PPC_OAM_UPMEP_TRAP_ID_SNOOP,
  SOC_PPC_OAM_UPMEP_TRAP_ID_ERR_LEVEL,
  SOC_PPC_OAM_UPMEP_TRAP_ID_ERR_PASSIVE,
  SOC_PPC_OAM_UPMEP_TRAP_ID_COUNT
} SOC_PPC_OAM_UPMEP_TRAP_ID;

typedef enum {
  SOC_PPC_OAM_MIRROR_ID_OAMP = 0,
  SOC_PPC_OAM_MIRROR_ID_CPU,
  SOC_PPC_OAM_MIRROR_ID_RECYCLE,
  SOC_PPC_OAM_MIRROR_ID_SNOOP,
  SOC_PPC_OAM_MIRROR_ID_CPU_RAW,
  SOC_PPC_OAM_MIRROR_ID_ERR_LEVEL,
  SOC_PPC_OAM_MIRROR_ID_ERR_PASSIVE,
  SOC_PPC_OAM_MIRROR_ID_RECYCLE_LBM,
  SOC_PPC_OAM_MIRROR_ID_COUNT
} SOC_PPC_OAM_MIRROR_ID;

typedef enum {
  SOC_PPC_BFD_TRAP_ID_CPU = 0,
  SOC_PPC_BFD_TRAP_ID_OAMP_IPV4,
  SOC_PPC_BFD_TRAP_ID_OAMP_MPLS,
  SOC_PPC_BFD_TRAP_ID_OAMP_PWE,
  SOC_PPC_BFD_TRAP_ID_OAMP_CC_MPLS_TP,
  SOC_PPC_BFD_TRAP_ID_UC_IPV6,
  SOC_PPC_BFD_TRAP_ID_COUNT
} SOC_PPC_BFD_TRAP_ID;


typedef enum {
    SOC_PPC_OAM_REPORT_MODE_NORMAL = 0,
    SOC_PPC_OAM_REPORT_MODE_COMPACT = 1,
    SOC_PPC_OAM_REPORT_MODE_RAW = 2,

    SOC_PPC_OAM_REPORT_MODE_COUNT
} SOC_PPC_OAM_REPORT_MODE;

typedef uint64 SOC_PPC_OAM_ICC_MAP_DATA;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 loc;
    uint8 rdi_set;
    uint8 rdi_clear;
    uint8 rmep_state_change;
    uint8 sd_set;
    uint8 sd_clear;
    uint8 sf_set;
    uint8 sf_clear;
    uint8 dExcess_set;
    uint8 dMissmatch;
    uint8 dMissmerge;
    uint8 dAll_clear;
    uint32 rmeb_db_ndx;
    uint32 rmep_state;
} SOC_PPC_OAM_EVENT_DATA;

/*Struct preserving all "global" data associated with an interrupt.*/
typedef struct {
    /* Indicates which of the 20 entries in interrupt_message is valid.*/
   int  last_interrupt_message_num; 
   /* Can't use soc_reg_above_64_val_t in struct. Field represents the freshest copy of the interrupt message. Only this should be processed directly.*/
   uint32 interrupt_message[SOC_REG_ABOVE_64_MAX_SIZE_U32]; 

   /** Used only when using DMA interrupts */
   /* Upon entering an interrupt a "chunk of memory should be copy from the "host memeory" (shared by the DMA) to this memory */ 
   uint32    * buffer_copied_from_dma_host_memory; 
   uint8 internal_buffer_is_allocated;
   /* Represents amount of valid entries in buffer_copied_from_dma_host_memory */
   int num_entries_available_in_local_buffer;
   /*Pointer to current location in buffer_copied_from_dma_host_memory*/
   int num_entries_read_in_local_buffer;


} SOC_PPC_OAM_INTERRUPT_GLOBAL_DATA; 


typedef enum {
    SOC_PPC_OAM_DMA_EVENT_TYPE_EVENT,
    SOC_PPC_OAM_DMA_EVENT_TYPE_STAT_EVENT,

    SOC_PPC_OAM_DMA_EVENT_TYPE_NOF
} SOC_PPC_OAM_DMA_EVENT_TYPE;

/* This enumerator defines the strategy
   for flexible OAMP CRC calculcation
   for QAX: instead of dynamic allocation,
   each TCAM entry has a fixed purpose */
typedef enum {
    QAX_OAMP_FLEX_CRC_TCAM_48_BYTE_MAID = 0,
    QAX_OAMP_FLEX_CRC_TCAM_48_BYTE_MAID_CCM_COUNT = 1,
    QAX_OAMP_FLEX_CRC_TCAM_48_BYTE_MAID_EGRESS_INJ = 2,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV3 = 3,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV4 = 4,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV5 = 5,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV6 = 6,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV7 = 7,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV8 = 8,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV9 = 9,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV10 = 10,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV11 = 11,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV12 = 12,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV13 = 13,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV14 = 14,
    QAX_OAMP_FLEX_CRC_TCAM_RSRV15 = 15
} QAX_OAMP_FLEX_CRC_TCAM_t;

/* This enumerator defines the strategy
   for flexible OAMP CRC calculcation
   for QAX: instead of dynamic allocation,
   each CRC mask entry has a fixed purpose */
typedef enum {
    QAX_OAMP_FLEX_VER_MASK_TEMP_48_BYTE_MAID = 0,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV1 = 1,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV2 = 2,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV3 = 3,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV4 = 4,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV5 = 5,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV6 = 6,
    QAX_OAMP_FLEX_VER_MASK_TEMP_RSRV7 = 7
} QAX_OAMP_FLEX_VER_MASK_TEMP_t;

/* Callback type for event handlers */
typedef int (*dma_event_handler_cb_t)(int, SOC_PPC_OAM_DMA_EVENT_TYPE, SOC_PPC_OAM_INTERRUPT_GLOBAL_DATA *) ;


/* Used for diag oam action_key (diag oam CLassification) */
typedef struct {
    int lif;
    int level;
    int opcode;
    int ing;
    int inj;
    int mymac;
    int bfd;

    int your_disc; /* for Jericho only */

} SOC_PPC_OAM_ACTION_KEY_PARAMS;

/* Used for diag oam Key_Select */
typedef struct {
    int ing; /*Ingress*/
    int olo; /*OAM-LIF-Outer-Valid*/
    int oli; /*OAM-LIF-Inner-Valid*/
    /*Egress only*/
    int inj; /*Is inject bit is on?*/
    int pio; /*Packet-Is-OAM?*/
    int cp;  /*Counter-Ptr-Valid?*/
    /*Ingress only*/
    int ydv; /*Your-Disc-Valid?*/
    int leo; /*LIF-Equal-To-OAM-LIF-Outer?*/
    int lei; /*LIF-Equal-To-OAM-LIF-Inner?*/
} SOC_PPC_OAM_KEY_SELECT_PARAMS;


/* Jericho Only */
/* LM/DM rx report data from interrupt */
typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint32 event_size; /* Measured in consecutive events */
    uint8 event_type; /* 0 - LM, 1 - DM */
} SOC_PPC_OAM_RX_REPORT_EVENT_DATA;

/* ARAD+ Only */
typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    /* value of Punt Counter on which the packet is punt */
    uint32             punt_rate;  
    /* 
     * '0' disables punt of packet in case of state/RDI change or validity check fail;
     * '1' enables the punt according to Punt Rate
     */
    uint32             punt_enable;
    /* 
     * '00' don't update RMEP DB state/RDI on rx packet, but generate an event
     * '01' update RMEP DB state/RDI on rx packet, but don't generate an event
     * '10' update RMEP DB state/RDI on rx packet only if event fifo is not full generate an event
     * '11' update RMEP DB state/RDI on rx packet even if event fifo is full then dont generate an event, else generate an event
     */
    uint32             rx_state_update_enable;
    /* 
     * '00' don't update RMEP DB state on scan, but generate an event
     * '01' update RMEP DB state on scan, but don't generate an event
     * '10' update RMEP DB state on scan only if event fifo is not full, generate an event
     * '11' update RMEP DB state on scan even if event fifo is full then dont generate an event else generate an event
     */      
    uint32             scan_state_update_enable;  
    /* Enable the scanner machine to update MEP DB RDI indication in case LoC (timeout) event detected */
    uint32             mep_rdi_update_loc_enable; 
    /* Enable the scanner machine to update MEP DB RDI indication in case LoC clear (time-in) event detected */
    uint32             mep_rdi_update_loc_clear_enable; 
    /* Enables the RX machine to update the MEP DB RDI indication by copy the RDI indication from the received valid CCM packet */
    uint32             mep_rdi_update_rx_enable;  
} SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    SHR_BITDCL             counter_disable[_SHR_BITDCLSIZE(SOC_PPC_OAM_ETHERNET_PDU_OPCODE_COUNT)];
    SHR_BITDCL             meter_disable[_SHR_BITDCLSIZE(SOC_PPC_OAM_ETHERNET_PDU_OPCODE_COUNT)];
    uint32                 opcode_to_trap_code_unicast_map[SOC_PPC_OAM_OPCODE_MAP_COUNT];
    uint32                 opcode_to_trap_code_multicast_map[SOC_PPC_OAM_OPCODE_MAP_COUNT];
    uint8                  opcode_to_trap_strength_unicast_map[SOC_PPC_OAM_OPCODE_MAP_COUNT];
    uint8                  opcode_to_trap_strength_multicast_map[SOC_PPC_OAM_OPCODE_MAP_COUNT];
    uint8                  opcode_to_snoop_strength_unicast_map[SOC_PPC_OAM_OPCODE_MAP_COUNT];
    uint8                  opcode_to_snoop_strength_multicast_map[SOC_PPC_OAM_OPCODE_MAP_COUNT];
} SOC_PPC_OAM_MEP_PROFILE_DATA;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8                  is_1588;
    SOC_PPC_OAM_MEP_PROFILE_DATA                  mep_profile_data;
    SOC_PPC_OAM_MEP_PROFILE_DATA                  mip_profile_data;           
    uint8                  flags;
    /* Used only in ARAD+ */
    uint8                  mp_type_passive_active_mix;
    uint8                  is_piggybacked;
    uint8                  is_slm;
    /* Used only in ARAD */
    uint8                  is_default;
} SOC_PPC_OAM_LIF_PROFILE_DATA;

typedef uint8 SOC_PPC_OAM_CPU_TRAP_CODE_TO_MIRROR_PROFILE_MAP[SOC_PPC_NOF_TRAP_CODES];

typedef uint8 SOC_PPC_OAM_MA_NAME[13];

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint16             rmep_id;
    uint32             mep_index;           
} SOC_PPC_OAM_RMEP_INFO_DATA;

typedef struct {
    uint32 trap_ids[SOC_PPC_OAM_TRAP_ID_COUNT];
    uint32 upmep_trap_ids[SOC_PPC_OAM_UPMEP_TRAP_ID_COUNT];
    uint32 mirror_ids[SOC_PPC_OAM_MIRROR_ID_COUNT];
} SOC_PPC_OAM_INIT_TRAP_INFO;

typedef struct {
    uint32 trap_ids[SOC_PPC_BFD_TRAP_ID_COUNT];
} SOC_PPC_BFD_INIT_TRAP_INFO;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8              rdi_received;     /* read only */
    uint8              loc; /* R/O*/
    uint8              is_state_auto_handle;    /* Arad only */
    uint8              is_event_mask;    /* Arad only */        
    uint8              punt_profile;    /* Arad+ only */
    uint32             ccm_period;      /* in microseconds*/
    uint32                loc_clear_threshold;  
    uint32                rmep_state;
    uint8               last_ccm_lifetime_valid_on_create; /* For event generation immediately after RMEP creation */
} SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8              last_prd_pkt_cnt_1731;     /* count the number of1731 packets received during the last period */
    uint8              last_prd_pkt_cnt_1711;     /* count the number of1711 packets received during the last period  */
    uint8              rx_err;                    /* error packet received */
    uint8              loc;                        /* pointer to the RMEP DB  */
} SOC_PPC_OAM_OAMP_RMEP_DB_EXT_ENTRY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8              ccm_tx_rate;             /* tx rate of the appropriate MEP */
    uint8              entry_format;             /* Format 1731 is equal to 2'b00 and format 1711 is equal to 2'b01 2 bits*/
    uint8              thresh_profile;          /* thresholds profile   */
    uint8              sf;                      /*signal failure indication */
    uint8              sd;                      /* segnal degragate indication */
    uint16             rmep_db_ptr;             /* pointer to the RMEP DB */
    uint16             sum_cnt;                  /* Sum of the sliding window packets  */
    uint32             sliding_wnd_cntr[BCM_OAM_MAX_NUM_SLINDING_WINDOWS];      /*256 2-bit counters of per-period packets received */
} SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8            prd_pkt_cnt_0;       /* period 0 packet counter  3 bits*/
    uint8            prd_pkt_cnt_1;       /* period 1 packet counter  3 bits*/
    uint8            prd_pkt_cnt_2;       /* period 2 packet counter  3 bits*/
    uint8            prd_err_ind_0;       /* period 0 err packet indicator */
    uint8            prd_err_ind_1;       /* period 1 err packet indicator*/
    uint8            prd_err_ind_2;       /* period 2 err packet indicator*/
    uint8            d_excess ;           /* dexcess indication 1 bit */
    uint8            d_mismerge;          /* dMismerge indication   1 bit */
    uint8            d_mismatch;          /* dMismatch indication   1 bit */
    uint8            ccm_tx_rate;         /* rate of the appropriate CCM 3 bits*/
    uint8            allert_method ;      /* if allert_method 1, issue event on mismerge, mismatch or excess indication set, 
                                         otherwise isue event on state change 1 bit*/
   uint16            rmep_db_ptr;        /* pointer to rmep db entry 16 bits */
} SOC_PPC_OAM_OAMP_SD_SF_Y_1711_DB_ENTRY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8           wnd_lngth;          /* size of the sliding window (1-256) */
    uint8           alert_method;       /*  if set, issue event on sd/sf set, otherwise issue allert on state change */
    uint8           supress_alerts;     /*  supress alert event generation otherwise isue event on state change 1 bit*/
    uint16         sd_set_thresh;      /*  sd set threshold */
    uint16         sd_clr_thresh;      /*  sf clear threshold */
    uint16         sf_set_thresh;      /*  sf set threshold */
    uint16         sf_clr_thresh;      /*  sf clear threshold */
} SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB;

typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
  uint8                                 d_excess_thresh;
  uint8                                 clr_low_thresh;
  uint8                                 clr_high_thresh;
  uint8                                 num_entry;
} SOC_PPC_OAM_OAMP_SD_SF_1711_config;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 do_not_set_interval;
    uint8 ccm_interval; /* 3 bits */
    uint8 is_upmep; /* 1 bits */
    uint8 priority; /* 3 bits */
    uint32 system_port;
    /* Local port: in Arad, Local port is taken  from the  src-mac lsb.
       For up-MEPs The local port also goes on the PTCH.
       For down MEPs (similarly BFD) on the ITMH via the local_port_2_sys_port register.
       In Arad A0 Local port goes directly on the packet either way.*/
    /* Value of - 1 signifies to the HW not to update.*/
    uint32 local_port; 
    SOC_PPC_OAM_MEP_TYPE  mep_type;
    uint16 remote_recycle_port; /* Used by up-MEP server only. If this is !=0 then the server program will also be set in the mep-pe profile.*/

    /*oam*/
    SOC_SAND_PP_MAC_ADDRESS   src_mac_address; /* Arad*/
    uint8 mdl; /* 3 bits */
    uint8 icc_ndx; /* 4 bits */
    uint16 mep_id; /* 13 bits */
    uint8 inner_tpid; /* 2 bits */
    uint16 inner_vid_dei_pcp; /* 16 bits */
    uint8 outer_tpid; /* 2 bits */
    uint16 outer_vid_dei_pcp; /* 16 bits */
    uint8 rdi; /* 1 bits */
    uint8 tags_num;
    /* Arad+ OAM*/
    uint16 counter_pointer; /* Counter pointer for TXfcf stapming by LMM.*/
    /* Jericho OAM*/
    uint8 port_status_tlv_en;
    uint8 port_status_tlv_val; /* applicable only when tlv_en==1. If tlv_val==0, value on outgoing packets will be BCM_OAM_PORT_TLV_BLOCKED (1), 
                        if tlv_val==1 value will be BCM_OAM_PORT_TLV_UP (2)*/
    uint8 interface_status_tlv_control; /* Legal values: 1- 7 when interface status is to be used, 0 otherwise.*/
    uint8 src_mac_lsb; /* in arad this is taken from the local port. In Jericho from a dedicated field.*/
    uint8 src_mac_msb_profile; /* One of two profiles available.*/

    /*bfd, MPLS, PWE*/
    uint32 dst_ip_add;
    uint32 egress_if;       /* egress interface */
    uint32 label;           /* encapsulation label */
    uint32 remote_discr;    /* remote discriminator */
    uint8  src_ip_add_ptr;
    uint8  push_profile;
    uint8  local_detect_mult; /* local detection multiplier */
    uint8  min_rx_interval_ptr;    /* required local rx interval */
    uint8  min_tx_interval_ptr;    /* required local tx interval */
    uint8  tunnel_is_mpls;
    uint8  tos_ttl_profile;
    uint8 ip_subnet_len; /* Jericho only*/
    uint8 micro_bfd; /* jericho only. used for signifying mep-pe-profile*/

    /* ARAD only - BFD */
    uint8  pbit;
    uint8  fbit;

    /* ARAD+ only - BFD */
    uint8  sta;
    uint8  diag_profile;
    uint8  flags_profile;
    uint8  bfd_pwe_router_alert; /* indication whether PWE RA is set*/
    uint8  bfd_pwe_ach; /* indication whether PWE has CW */
    uint8  bfd_pwe_gal; /* indication whether PWE has GAL, Jr only */

    /* ARAD+ only - string based 11 bytes MAID */
    uint8 is_11b_maid;

    uint8 maid_48_index; /* index to the oamp_pe_gen_mem */
    uint8 is_maid_48;    /* flexible maid 48 Byte */
    uint8 is_mc;         /* ITMH with mc destination*/

    /* QAX only - pointers */
    uint32 lm_dm_ptr; /* pointer to LM/DM data, 0=disable, {Bank(4),entry(10)} */
    uint32 flex_data_ptr; /* pointer to extra PE data, 0=disable, {Bank(4),entry(10)} */
    uint32 out_lif; /* Used in ETH OAM, PWE + LSP CCM Counting */

} SOC_PPC_OAM_OAMP_MEP_DB_ENTRY;

/*ARAD+  only. Used for LM, Dm accelerated packets.*/
typedef struct {
    /* Shared*/
    SOC_SAND_MAGIC_NUM_VAR
    SOC_PPC_OAM_LM_DM_ENTRY_TYPE entry_type; 
    uint32 mep_id; /*ID of the endpoint. (ccm entry)*/
    uint32 allocated_id;  /*The (first) index of the entry allocated for DM/LM. */
    uint8 is_update; /* Set to 1 if loss/delay_add() is called with the update flag*/

    uint8 is_1DM; /* offer 1DM functionality */
    uint8 is_piggyback_down; /* For down piggy back the mep-pe profile must be changed to 0*/

    uint8   is_slm; /* for assigning new OAMP_PE program for downMEP SLM */
    uint32 lm_dm_id; /* QAX MEP-DB bank entry */
} SOC_PPC_OAM_OAMP_LM_DM_MEP_DB_ENTRY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
        /*basic statistics*/
    uint32 entry_id; /*index of the MEP DB entry*/
    uint32 my_tx;
    uint32 my_rx;
    uint32 peer_tx;
    uint32 peer_rx;
        /*extended statistics*/
    uint8 is_extended;
    uint32 last_lm_far;
    uint32 last_lm_near;
    uint32 acc_lm_far;
    uint32 acc_lm_near;
    uint32 max_lm_far;
    uint32 max_lm_near;

} SOC_PPC_OAM_OAMP_LM_INFO_GET;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint32 entry_id; /*index of the MEP DB entry*/
    uint32 last_delay_sub_seconds; /* 30 bits. What unit?? */
    uint32 last_delay_second; /*12 bits.*/
    uint32 max_delay_sub_seconds; 
    uint32 max_delay_second;
    uint32 min_delay_sub_seconds; 
    uint32 min_delay_second;

} SOC_PPC_OAM_OAMP_DM_INFO_GET;

/* CCM_CNT is 20 bits in JERICHO. So the highest is 0xfffff.
 * CCM_CNT is 19 bits in QAX. So the highest is 0x7ffff.
 * Keeping the lowest of them as MAX, since that number itself is
 * sufficiently large for any platform. */
#define SOC_PPC_OAM_OAMP_ETH1731_MEP_PROFILE_MAX_CCM_CNT 0x7ffff

typedef struct {
    unsigned int piggy_back_lm : 1;
    unsigned int slm_lm :1;
	
    /* For QAX Flexible 48 byte MAID - disable verification of message by comparing MAID to field in MEP-DB entry */
    unsigned int maid_check_dis:1;
	
    /* The report_mode is 2 bit bitmap that indicate which report is active:
            00 - None    01 - LM     10 - DM     11-Both
       In Jericho the reports are global for the profile(once report_mode is set, it's always = 11)
       In QAX the reports are separated and all of the cases are available */
    unsigned int report_mode :2; 
    unsigned int rdi_gen_method :2;
    unsigned int lmm_da_oui_prof :3;
    unsigned int dmm_rate : 3;
    unsigned int lmm_rate : 3;
    /* Jericho: only opcode 0, 1 used at this stage (0-7 available)*/
    unsigned int opcode_0_rate : 3;
    unsigned int opcode_1_rate : 3;
    /* Not really part ot the profile in Arad+, but uses same pointer as MEP profile..*/
    unsigned int lmm_offset :8;
    unsigned int lmr_offset :8;
    unsigned int dmm_offset :8;
    unsigned int dmr_offset :8;
    /* Phase profile (Jericho). Strictly speaking not part of MEP profile but uses the same pointer*/
    unsigned int ccm_cnt : 20;
    unsigned int dmm_cnt : 20;
    unsigned int lmm_cnt : 20;
    unsigned int op_0_cnt : 20;
    unsigned int op_1_cnt : 20;

}SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY;



typedef struct {
    uint32  *buffer;/*Memory to be allocated*/
    uint8 use_dma; /* determines how the memory is to be allocated/freed*/
    uint8 is_allocated;
} _oam_oam_a_b_table_buffer_t;


typedef struct {
    /* No MAGIC_NUM to save WB space */
    SOC_PPC_OAM_MEP_TYPE mep_type;
    SOC_PPC_LIF_ID lif;
    SOC_PPC_LIF_ID passive_side_lif; /* Should be different than lif only in the case of BHH endpoints */
    uint32 tx_gport;                 /* Used to store tx_gport for BFD endpoint*/
    uint32 your_discriminator;
    uint32 port; /* represents MEP on which endpoint resides, used for my-cfm-mac configuration. */
    uint8 port_core; /* The core on which this port is on. */
    uint8 non_acc_profile; /* For MIPs this represents the ingress */
    uint8 non_acc_profile_passive; /* For MIPs this represents the egress */
    uint8 acc_profile;
    uint32 flags; /* is_accelerated, is_mep, is_pcp, is_dm_1588, is_upmep, and some more */
    uint8 src_mac_lsb; /* Used to signify the original "local_port" for OAM.
    This is used when the OAMP is required to TX with CCM period >0.
    Stores the src_mac LSB*/
    /*oam*/
    uint8 md_level;
    SOC_SAND_PP_MAC_ADDRESS dst_mac_address;
    uint32 ma_index; /*For bfd single hop this is used as local discriminator (key for the LEM)*/
    uint32 counter; /* For BFD endpoints of type MPLS this is used as the key for the TCAM index.*/
    /*bfd*/
    uint32 dip_profile_ndx; /* Also Used by OAM to signify the original "system_port" for Down MEP/MPLS.
    This is used when the OAMP is required to TX with CCM period >0.*/
    uint32 remote_gport; /* only used for 'get' api in BFD 
         Also used when storing OAM server entry.
            Down MEP: Representing trap code.
            Up MEP: Representing given tx_gport.*/
    uint32 session_map_flag;  /* session map */
    uint16 sd_sf_id;  /* sd/sf DB index */
    uint16 y1711_sd_sf_id;  /* Y.1711 sd/sf DB index */
} SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY;

typedef struct {
    uint8 flag;  /*1:lb enalbe / 2:tst enable */
    bcm_oam_endpoint_t mepid;              /* Endpoint ID of Local MEP. */
    int lb_tlv_num;
    bcm_oam_tlv_t tlv; /*flag1:tlv for LB, flag 2 tlv for tst tx*/
    uint8                     pkt_pri;              /* Optional priority value to be set on Ethernet outer/sole VLAN tag PDP and DEI fields */
    uint8                     inner_pkt_pri;        /* Optional priority value to be set on Ethernet inner VLAN tag PDP and DEI fields */
    bcm_cos_t                 int_pri;              /* Optional priority fields on ITMH header */
} SOC_PPC_OAM_SAT_GTF_ENTRY;

typedef struct {
    uint8 flag;  /*1:lb enalbe / 2:tst enable */
    bcm_oam_endpoint_t mepid;              /* Endpoint ID of Local MEP. */
    uint32 identifier_tc;          /* Class of service */
    uint32 identifier_color;       /* Color */
    uint32 identifier_trap_id;     /* Trap ID */
    uint32 identifier_session_id;  /* SAT session ID */
    uint32 session_map_flag;  /* session map */
    bcm_oam_tlv_t tlv; 
} SOC_PPC_OAM_SAT_CTF_ENTRY;


typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 ingress; 
    uint8 your_discr; /* Jericho only*/
     uint32 oam_lif;
} SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 mep_bitmap;  /* Arad only*/
    uint8 mip_bitmap;  /* Arad only*/
    uint8 mp_profile;
    uint16 mp_type_vector; /* Jericho only*/
    uint32 counter_ndx;
} SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 ingress; 
    uint8 mdl;
    uint8 your_disc;
    uint32 oam_lif;
} SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_KEY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 mp_profile;
    uint16 oam_id;
} SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 my_cfm_mac; 
    uint8 is_bfd;
    uint8 opcode;
    /* Used in ARAD only */
    SOC_PPC_OAM_MP_TYPE mip_type;
    /* Used in ARAD only */
    SOC_PPC_OAM_MP_TYPE mep_type;
    /* Used in ARAD+ only */
    SOC_PPC_OAM_MP_TYPE_PLUS mp_type;
    /* Jericho*/
    SOC_PPC_OAM_MP_TYPE_JERICHO mp_type_jr;
    uint8 mp_profile;
    uint8 inject;
    uint8 ingress;
} SOC_PPC_OAM_CLASSIFIER_OAM1_ENTRY_KEY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 my_cfm_mac; 
    uint8 is_bfd;
    uint8 opcode;
    uint8 mp_profile;
    uint8 inject;
    uint8 ingress;
} SOC_PPC_OAM_CLASSIFIER_OAM2_ENTRY_KEY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 snoop_strength;/* Ingress*/
    uint8 forward_disable; /*Egress*/
    uint8 mirror_profile; /*Egress*/
    uint8 mirror_strength; /* Jericho only. Egress. Currently ignored 
    (mirror strength is always set to highest value)*/
    uint8 mirror_enable; /* Jericho only. Egress*/
    uint32 cpu_trap_code; /* Ingress*/
    uint8 forwarding_strength;/* Ingress + Egress*/
    uint8 up_map;/* Ingress*/
    uint8 sub_type;/* Ingress + Egress*/
    uint8 meter_disable; /* Ingress*/
    uint8 counter_disable;/* Ingress + Egress*/
} SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    SOC_PPC_PKT_TERM_TYPE ttc;
    uint32 fwd_code;
    uint32 mpls_label;
    uint32 channel_type;
    uint32 mdl;
    uint32 opcode;
    uint32 your_disc;
} SOC_PPC_OAM_TCAM_ENTRY_KEY;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint32 type;
    uint32 is_oam;
    uint32 is_bfd;
    uint32 opcode;
    uint32 mdl;
    uint32 your_discr;
    uint32 oam_lif_tcam_result;
    uint32 oam_lif_tcam_result_valid;
    uint32 my_cfm_mac;
    uint32 oam_offset;
    uint32 oam_stamp_offset;
    uint32 oam_pcp;
} SOC_PPC_OAM_TCAM_ENTRY_ACTION;


typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 data[80]; 
    int len;
} SOC_PPC_OAM_OAMP_PROTECTION_HEADER;


/*bfd*/
#define SOC_PPC_OAMP_LENGTH_TTL                             8
#define SOC_PPC_OAMP_IPV4_TOS_TTL_LENGTH_TOS                 8
#define SOC_PPC_OAMP_IPV4_TOS_TTL_DATA_NOF_BITS             16
#define SOC_PPC_OAMP_IPV4_SRC_ADDR_DATA_NOF_BITS             32
#define SOC_PPC_OAMP_BFD_REQ_INTERVAL_DATA_NOF_BITS         32
#define SOC_PPC_OAMP_BFD_DIAG_PROFILE_NOF_BITS                5       
#define SOC_PPC_OAMP_BFD_FLAGS_PROFILE_NOF_BITS                6
#define SOC_PPC_OAMP_MPLS_PWE_PROFILE_NOF_BITS                 11
#define SOC_PPC_OAMP_MPLS_PWE_PROFILE_LENGTH_EXP             3
/* First N bits of the TX my-discr are taken from the MEP-DB index, where N is log(|MEP_DB|). */
#define SOC_PPC_BFD_TX_MY_DISCRIMINATOR_RANGE_BIT_START           (13 + SOC_IS_JERICHO(unit) ) 
/* RX your-discr range defined by configuration of 16/14 MSBs. In Jericho the OAM-LIF is 2 bits larger which meas that this can be 2 bits smaller.*/
#define SOC_PPC_BFD_RX_YOUR_DISCRIMINATOR_RANGE_BIT_START           ( 16 + 2 *SOC_IS_JERICHO(unit)  )
#define SOC_PPC_BFD_DISCRIMINATOR_TO_LIF_START_MASK        ((1 <<SOC_PPC_OAM_SIZE_OF_OAM_KEY_IN_BITS(unit)) -1)
#define SOC_PPC_BFD_DISCRIMINATOR_TO_ACC_MEP_ID_START_MASK         ((1<<SOC_DPP_DEFS_GET(unit,oam_2_id_nof_bits)) -1) /* Should be mapped to a legal MEP-DB entry*/

#define SOC_PPC_BFD_PDU_LENGTH  0x18
#define SOC_PPC_BFD_PDU_VERSION 0x1

/* ARAD+ */
#define SOC_PPC_OAM_OAMP_NUMBER_OF_PUNT_PROFILES(_unit)  SOC_DPP_DEFS_GET(_unit,oamp_number_of_punt_profiles)

/* Jericho */

/* Determine the index used in the OPCODE_N registers (global configuration).*/
#define SOC_PPC_OAM_AIS_PERIOD_ONE_SECOND_OPCODE_ENTRY 0
#define SOC_PPC_OAM_AIS_PERIOD_ONE_MINUTE_OPCODE_ENTRY 1

/* ACH channel type selection */
#define SOC_PPC_BFD_ACH_TYPE_PWE_CW     1
#define SOC_PPC_BFD_ACH_TYPE_GACH_CC    2
typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 bfd_vers;
    uint8 bfd_diag;
    uint8 bfd_sta;
    uint8 bfd_flags;
    uint8 bfd_length;
    uint32 bfd_req_min_echo_rx_interval;
} SOC_PPC_BFD_PDU_STATIC_REGISTER;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    SOC_PPC_BFD_PDU_STATIC_REGISTER bfd_static_reg_fields;
    uint32 bfd_my_discr;
    uint32 bfd_your_discr;
} SOC_PPC_BFD_CC_PACKET_STATIC_REGISTER;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 ttl;
    uint8 exp;
} SOC_PPC_MPLS_PWE_PROFILE_DATA;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 tos;
    uint8 ttl;
} SOC_PPC_BFD_IP_MULTI_HOP_TOS_TTL_DATA;

typedef struct {
    SOC_SAND_MAGIC_NUM_VAR
    uint8 tc;
    uint8 dp;
} SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES;


#define SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BIT_SIZE  (64)
#define SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BYTE_SIZE (SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BIT_SIZE / SOC_SAND_NOF_BITS_IN_BYTE)
#define SOC_PPC_OAM_OAMP_CRC_MASK_LSB_BYTE_SIZE (120)

/* In the msb mask, each bit represents a bit to mask (or not) when
   calculating the CRC
   In the lsb mask, each BIT represents an entire BYTE to be masked
   when calculating the CRC */
typedef struct {
    uint8       msb_mask[SOC_PPC_OAM_OAMP_CRC_MASK_MSB_BYTE_SIZE];
    SHR_BITDCL  lsbyte_mask[_SHR_BITDCLSIZE(SOC_PPC_OAM_OAMP_CRC_MASK_LSB_BYTE_SIZE)];
} SOC_PPC_OAM_OAMP_CRC_MASK;

typedef struct {
    uint16                      crc16_val1;      /* Calculated crc val #1. */
    uint16                      crc16_val2;      /* Calculated crc val #2. */
    SOC_PPC_OAM_OAMP_CRC_MASK   mask;           /* Mask for the packet's crc. */
} SOC_PPC_OAM_OAMP_CRC_INFO;

typedef struct {
    int                         opcode_bmp;          /* gach/opcode. */
    int                         opcode_bmp_mask;     /* Which bits to mask, if any? */
    int                         mep_pe_profile;      /* PE profile of the packet. */
    int                         mep_pe_profile_mask; /* Needed when bits in the profile are used as arguments*/
    uint8                       oam_bfd;             /* 0: Oam. 1: Bfd. */
    uint8                       oam_bfd_mask;        /* Just in case a profile applied to OAM and BFD */
    int                         mask_tbl_index;      /* The index in the mask table to point to. */
    uint8                       crc_select;          /* Which crc to use. 0: CRC1. 1: CRC2. */
} SOC_PPC_CRC_SELECT_INFO;



typedef struct {
    int                           mep_idx;        /* MEP DB index where the calculated crc val is stored. */
    QAX_OAMP_FLEX_VER_MASK_TEMP_t mask_tbl_index; /* The index in the mask table where the mask will be stored. */
    QAX_OAMP_FLEX_CRC_TCAM_t      crc_tcam_index; /* The index in the flex crc tcam */
    SOC_PPC_OAM_OAMP_CRC_INFO     crc_info;       /* Info for the crc (mask + calculated value). */
    SOC_PPC_CRC_SELECT_INFO       crc_tcam_info;  /* Info for mask + crc select. */
} SOC_PPC_OAM_BFD_FLEXIBLE_VERIFICATION_INFO;


#define SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS     (124)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS   (172)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_NOF_EXTRA_ENTRIES   (3)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_BITS       \
        (SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_FIRST_HDR_SIZE_BITS \
         + SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_EXTRA_ENTRY_SIZE_BITS * SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_NOF_EXTRA_ENTRIES)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_UINT32 (SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_BITS / SOC_SAND_NOF_BITS_IN_UINT32 + 1)

/*
 * The opcode bmp of the mep db extension can be a combination of these flags:
 */
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_CCM_CCM_LM         (1 << 0)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_SLM_LMM            (1 << 1)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_DMM                (1 << 2)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_0   (1 << 3)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_1   (1 << 4)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_2   (1 << 5)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_3   (1 << 6)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_4   (1 << 7)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_5   (1 << 8)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_6   (1 << 9)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_GEN_PDU_OPCODE_7   (1 << 10)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_BFD                (1 << 11)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_RFC_LM             (1 << 12)
#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_RFC_DM             (1 << 13)

#define SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_LEGAL_OPCODE       ((1 << 14) - 1)

typedef struct {
    int         mep_idx;
    int         extension_idx;
    uint32      data[SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_MAX_SIZE_UINT32];
    int         data_size_in_bits;
    int         opcode_bmp; /* SOC_PPC_OAM_BFD_MEP_DB_EXT_OPCODE_BITMAP_* */
} SOC_PPC_OAM_BFD_MEP_DB_EXT_DATA_INFO;

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
  SOC_PPC_OAM_ETH_MP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_OAM_ETH_MP_INFO *info
  );

#if SOC_PPC_DEBUG_IS_LVL1

const char*
  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE_to_string(
    SOC_SAND_IN  SOC_PPC_OAM_ETH_ACC_FUNC_TYPE enum_val
  );

const char*
  SOC_PPC_OAM_ETH_MP_LEVEL_to_string(
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_LEVEL enum_val
  );

void
  SOC_PPC_OAM_ETH_MP_INFO_print(
    SOC_SAND_IN  SOC_PPC_OAM_ETH_MP_INFO *info
  );

#endif /* SOC_PPC_DEBUG_IS_LVL1 */
/* } */


void
  SOC_PPC_OAM_INIT_TRAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_OAM_INIT_TRAP_INFO *init_trap_info
  );

void
  SOC_PPC_OAM_MEP_PROFILE_DATA_clear(
    SOC_SAND_OUT SOC_PPC_OAM_MEP_PROFILE_DATA *profile_data
  );

void
  SOC_PPC_OAM_LIF_PROFILE_DATA_clear(
    SOC_SAND_OUT SOC_PPC_OAM_LIF_PROFILE_DATA *profile_data
  );

void
  SOC_PPC_OAM_RMEP_INFO_DATA_clear(
    SOC_SAND_OUT SOC_PPC_OAM_RMEP_INFO_DATA *rmep_info_data
  );

void
  SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY *rmep_db_entry
  );

void
  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_MEP_DB_ENTRY *mep_db_entry
  );

void
  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY *classifier_entry
  );
void
  SOC_PPC_OAM_SAT_CTF_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_SAT_CTF_ENTRY *info
  );

void
  SOC_PPC_OAM_SAT_GTF_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_SAT_GTF_ENTRY *info
  );

void
  SOC_PPC_OAM_OAMP_LM_DM_MEP_DB_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_LM_DM_MEP_DB_ENTRY *mep_db_entry
  );

void
  SOC_PPC_OAM_OAMP_DM_INFO_GET_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_DM_INFO_GET *mep_db_entry
  );

void
  SOC_PPC_OAM_OAMP_LM_INFO_GET_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_LM_INFO_GET *mep_db_entry
  );

void
  SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_ETH1731_MEP_PROFILE_ENTRY *entry
  );


void
  SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD *oem1_payload
  );

void
  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD *oem2_payload
  );

void
  SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_KEY *oem1_key
  );

void
  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_KEY *oem2_key
  );

void
  SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD *oam1_payload
  );

void
  SOC_PPC_OAM_CLASSIFIER_OAM1_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OAM1_ENTRY_KEY *oam1_key
  );

void
  SOC_PPC_OAM_CLASSIFIER_OAM2_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_CLASSIFIER_OAM2_ENTRY_KEY *oam2_key
  );

void
  SOC_PPC_OAM_TCAM_ENTRY_KEY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_TCAM_ENTRY_KEY *oam_tcam_key
  );

void
  SOC_PPC_OAM_TCAM_ENTRY_ACTION_clear(
    SOC_SAND_OUT SOC_PPC_OAM_TCAM_ENTRY_ACTION *oam_tcam_action
  );

/*bfd*/
void
  SOC_PPC_BFD_INIT_TRAP_INFO_clear(
    SOC_SAND_OUT SOC_PPC_BFD_INIT_TRAP_INFO *init_trap_info
  );

void
  SOC_PPC_BFD_PDU_STATIC_REGISTER_clear(
    SOC_SAND_OUT SOC_PPC_BFD_PDU_STATIC_REGISTER *bfd_pdu
  );

void
  SOC_PPC_BFD_CC_PACKET_STATIC_REGISTER_clear(
    SOC_SAND_OUT SOC_PPC_BFD_CC_PACKET_STATIC_REGISTER *bfd_cc_packet
  );

void
  SOC_PPC_MPLS_PWE_PROFILE_DATA_clear(
    SOC_SAND_OUT SOC_PPC_MPLS_PWE_PROFILE_DATA *mpls_pwe_profile
  );

void
  SOC_PPC_BFD_IP_MULTI_HOP_TOS_TTL_DATA_clear(
    SOC_SAND_OUT SOC_PPC_BFD_IP_MULTI_HOP_TOS_TTL_DATA *tos_ttl_data
  );

void
  SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES_clear(
    SOC_SAND_OUT SOC_PPC_OAMP_TX_ITMH_ATTRIBUTES *itmh_attr
  );

void
  SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_PUNT_PROFILE_DATA *info
  );

void
  SOC_PPC_OAM_EVENT_DATA_clear(
    SOC_SAND_OUT SOC_PPC_OAM_EVENT_DATA *info
  );

void
  SOC_PPC_OAM_RX_REPORT_EVENT_DATA_clear(
    SOC_SAND_OUT SOC_PPC_OAM_RX_REPORT_EVENT_DATA *info
  );

/***************************/

void
  SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY_print(
    SOC_SAND_IN SOC_PPC_OAM_OAMP_RMEP_DB_ENTRY *rmep_db_entry
  );

void
  SOC_PPC_OAM_OAMP_MEP_DB_ENTRY_print(
    SOC_SAND_IN SOC_PPC_OAM_OAMP_MEP_DB_ENTRY *mep_db_entry
  );

void
  SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY_print(
    SOC_SAND_IN SOC_PPC_OAM_CLASSIFIER_MEP_ENTRY *classifier_entry
  );

void
  SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD_print(
    SOC_SAND_IN SOC_PPC_OAM_CLASSIFIER_OEM1_ENTRY_PAYLOAD *oem1_payload
  );

void
  SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD_print(
    SOC_SAND_IN SOC_PPC_OAM_CLASSIFIER_OEM2_ENTRY_PAYLOAD *oem2_payload
  );

void
  SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD_print(
    SOC_SAND_IN SOC_PPC_OAM_CLASSIFIER_OAM_ENTRY_PAYLOAD *oam1_payload
  );

void
  SOC_PPC_OAM_OAMP_RMEP_DB_EXT_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_RMEP_DB_EXT_ENTRY *info
  );

void
  SOC_PPC_OAM_OAMP_RMEP_DB_EXT_ENTRY_print(
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_RMEP_DB_EXT_ENTRY *rmep_ext_entry
  );

void
  SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY *info
  );

void
  SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY_print(
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_SD_SF_DB_ENTRY *info
  );

void
  SOC_PPC_OAM_OAMP_SD_SF_Y_1711_DB_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_SD_SF_Y_1711_DB_ENTRY *info
  );

void
  SOC_PPC_OAM_OAMP_SD_SF_Y_1711_DB_ENTRY_print(
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_SD_SF_Y_1711_DB_ENTRY *info
  );

void
  SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB_ENTRY_clear(
    SOC_SAND_OUT SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB *info
  );

void
  SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB_ENTRY_print(
    SOC_SAND_IN  SOC_PPC_OAM_OAMP_SD_SF_PROFILE_DB *info
  );

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPC_API_OAM_INCLUDED__*/
#endif
