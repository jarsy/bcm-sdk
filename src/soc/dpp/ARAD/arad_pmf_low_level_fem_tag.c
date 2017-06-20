#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pmf_low_level_fem_tag.c,v 1.57 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FP

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>

#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_fem_tag.h>

#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
/* #include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h> */

#include <soc/dpp/drv.h>
#include <soc/dpp/dpp_config_defs.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PMF_FEM_MAP_TABLE_TBL_DATA_LENGTH_MAX ((SOC_SAND_MAX(139, 173)/32) + 1)

#define ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX ((SOC_SAND_MAX(99, 430) / 32) + 1)

#define ARAD_PMF_FEM_NOF_TCAM_RESULTS (4)

#define ARAD_PMF_FEM_KEY_LSB_MAX             (128)
#define ARAD_PMF_FEM_KEY_LSB_MULTIPLE         (16)
#define ARAD_PMF_FEM_KEY_LSB_FIRST_IN_MSB     (80)
#define ARAD_PMF_FEM_KEY_LSB_FORBIDDEN         (64)

#define ARAD_PMF_FEM_INSTR_KEY_SRC_MSB                  (4)
#define ARAD_PMF_FEM_INSTR_KEY_SRC_LSB                  (3)
#define ARAD_PMF_FEM_INSTR_KEY_SRC_IS_MSB_BIT         (2)
#define ARAD_PMF_FEM_INSTR_KEY_LSB_MSB                  (1)
#define ARAD_PMF_FEM_INSTR_KEY_LSB_LSB                  (0)
#define ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_MSB    (5)
#define ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_LSB    (4)
#define ARAD_PMF_FEM_INSTR_SRC_TYPE_TCAM            (2) /* '10' */
#define ARAD_PMF_FEM_INSTR_SRC_TYPE_NOP                (63) /* '11' */
#define ARAD_PMF_FEM_INSTR_TCAM_LOOKUP_ID_BIT         (3)
#define ARAD_PMF_FEM_INSTR_TCAM_SRC_MSB                  (2)
#define ARAD_PMF_FEM_INSTR_TCAM_SRC_LSB                  (1)
#define ARAD_PMF_FEM_INSTR_TCAM_32_MSB_RESULT_BIT     (0)
#define ARAD_PMF_FEM_INSTR_FES_FEM_COMPARE_VAL        (60)


#define ARAD_PMF_FES_SHIFT_MAX                         (31)

#define ARAD_PMF_FEM_TABLE_LENGTH_IN_UINT32S            (20)


/* NOF User-header-ndx: 0-UH0 for PMF, 1-UH1 for PMF, 2-UH0 for Egress Editor, 3-UH1 for Egress Editor */
#define ARAD_PMF_FEM_NOF_USER_HEADER_NDXS                (4)

#ifndef SOC_DPP_NOF_EGRESS_PMF_ACTIONS_JERICHO
/* Compilation error in arad-only compilation - Number of Egress PMF actions in Jericho */
#define SOC_DPP_NOF_EGRESS_PMF_ACTIONS_JERICHO (9)
#endif

/* In Jericho, 3 options from the TCAM result (31:0, 39:8, 47:16) */
#define ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS (3)

/* First value for the Key-Select-TCAM: 32 */
#define ARAD_PMF_FEM_NOF_KEY_SELECT_TCAM_START (SOC_SAND_SET_BITS_RANGE(ARAD_PMF_FEM_INSTR_SRC_TYPE_TCAM, ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_MSB, ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_LSB))

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* 
 * Encoding in Jericho of the FES_KEY_SELECT field, e.g.:
 * 0-2 are for first cycle, first key, with
 *  0 - Key[31:0]
 *  1 - Key[39:8] 
 *  2 - Key[47:16] (key_lsb = 1) - not exposed in Driver
 */
#define ARAD_PMF_KEY_SELECT_TCAM_JERICHO(lookup_cycle_id, key_tcam_id, key_lsb) \
    ((((lookup_cycle_id * ARAD_PMF_FEM_NOF_TCAM_RESULTS) + key_tcam_id) * ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS) + key_lsb)

#define ARAD_PMF_KEY_SELECT_TCAM_JERICHO_LOOKUP_CYCLE_GET(fes_key_select) \
    ((fes_key_select - ARAD_PMF_FEM_NOF_KEY_SELECT_TCAM_START) / (ARAD_PMF_FEM_NOF_TCAM_RESULTS * ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS))

#define ARAD_PMF_KEY_SELECT_TCAM_JERICHO_TCAM_LOOKUP_GET(fes_key_select, lookup_cycle_id) \
    ((fes_key_select - ARAD_PMF_FEM_NOF_KEY_SELECT_TCAM_START - (lookup_cycle_id * ARAD_PMF_FEM_NOF_TCAM_RESULTS * ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS)) / ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS)

#define ARAD_PMF_KEY_SELECT_TCAM_JERICHO_KEY_LSB_GET(fes_key_select) \
    ((fes_key_select - ARAD_PMF_FEM_NOF_KEY_SELECT_TCAM_START) % ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS)

#define ARAD_PMF_KEY_SELECT_TCAM_JERICHO_USE_KAPS(lookup_cycle_id,key_lsb) \
    (SOC_PPC_FP_NOF_CYCLES*ARAD_PMF_FEM_NOF_TCAM_RESULTS*ARAD_PMF_FEM_NOF_TCAM_LSB_OPTIONS + ((lookup_cycle_id * SOC_PPC_FP_NOF_CYCLES) + key_lsb))

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
/*
 * Arrays containing ations and their widhts - default and runtime.
 */
/* { */
/*
 * Four arrays containing the INITIAL (default) values for the width per action.
 * Arrays are for Arad, Arad+, Jericho and QAX.
 * For each action, there are three parameters:
 * Action identifier, Internal (HW) identifier, no. of bits assigned (width).
 * Number of entries is 'SOC_PPC_NOF_FP_ACTION_TYPES' - See SOC_PPC_FP_ACTION_TYPE
 * Number of meaningful entries is assumed smaller than that.
 * Last entry is marked by a line of '-1's
 */
/* { */
static 
	uint32  /* action name, hw-id, size*/
		Arad_pmf_fem_action_type_encoding[SOC_PPC_NOF_FP_ACTION_TYPES][NUM_ACTION_ELEMENTS] = {
            {SOC_PPC_FP_ACTION_TYPE_DEST                     ,0 ,  19       },
            {SOC_PPC_FP_ACTION_TYPE_TC                       ,1 ,  3        },
            {SOC_PPC_FP_ACTION_TYPE_DP                       ,2 ,  2        },
            {SOC_PPC_FP_ACTION_TYPE_EEI                      ,3 ,  24       },
            {SOC_PPC_FP_ACTION_TYPE_OUTLIF                   ,4 ,  16       },
            {SOC_PPC_FP_ACTION_TYPE_TRAP                     ,5 ,  8+3+16   }, /* Trap qualifier + Trap strength + Trap code.*/
                                                                   /* If SOC property custom_feature_reduced_trap_action=1, size is 11 bits: Trap strength + Trap code */
            {SOC_PPC_FP_ACTION_TYPE_SNP                      ,6 ,  8+2      },
            {SOC_PPC_FP_ACTION_TYPE_IN_PORT                  ,7 ,  8        },
            {SOC_PPC_FP_ACTION_TYPE_STAT                     ,8 ,  8        },
            {SOC_PPC_FP_ACTION_TYPE_VSQ_PTR                  ,9 ,  8        },
            {SOC_PPC_FP_ACTION_TYPE_MIRROR                   ,10,  4        },
            {SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY            ,11,  3        },
            {SOC_PPC_FP_ACTION_TYPE_MIR_DIS                  ,12,  1        },
            {SOC_PPC_FP_ACTION_TYPE_EXC_SRC                  ,13,  1        },
            {SOC_PPC_FP_ACTION_TYPE_IS                       ,14,  19       },
            {SOC_PPC_FP_ACTION_TYPE_METER_A                  ,15,  16       },
            {SOC_PPC_FP_ACTION_TYPE_METER_B                  ,16,  16       },
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_A                ,17,  16+1 /* 21+1(valid bit) only for Statistic-Record */ },
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_B                ,18,  16+1 /* 21+1(valid bit) only for Statistic-Record */ },
            {SOC_PPC_FP_ACTION_TYPE_DP_METER_COMMAND         ,19,  2        },
            {SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT            ,20,  16       },
            {SOC_PPC_FP_ACTION_TYPE_FWD_CODE                 ,21,  4        },
            {SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET               ,22,  3+6      },
            {SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE          ,23,  2+6      },
            {SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID ,24,  4        },
            {SOC_PPC_FP_ACTION_TYPE_VSI                      ,25,  16       },
            {SOC_PPC_FP_ACTION_TYPE_ORIENTATION_IS_HUB       ,26,  1        },
            {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_COMMAND        ,27,  6        },
            {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_1          ,28,  12       },
            {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_2          ,29,  12       },
            {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_PCP_DEI        ,30,  4        },
            {SOC_PPC_FP_ACTION_TYPE_IN_RIF                   ,31,  12       },
            {SOC_PPC_FP_ACTION_TYPE_VRF                      ,32,  12       },
            {SOC_PPC_FP_ACTION_TYPE_IN_TTL                   ,33,  8        },
            {SOC_PPC_FP_ACTION_TYPE_IN_DSCP_EXP              ,34,  8        },
            {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION_VALID    ,35,  1        },
            {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION          ,36,  19       },
            {SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE     ,37,  1        },
            {SOC_PPC_FP_ACTION_TYPE_EGRESS_LEARN_ENABLE      ,38,  1        },
            {SOC_PPC_FP_ACTION_TYPE_LEARN_FID                ,39,  15       },
            {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_0_TO_15         ,40,  16       },
            {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_16_TO_47        ,41,  32       },
            {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_0_TO_15       ,42,  16       },
            {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39      ,43,  24       },
            {SOC_PPC_FP_ACTION_TYPE_LEARN_OR_TRANSPLANT      ,44,  1        },
            {SOC_PPC_FP_ACTION_TYPE_IN_LIF                   ,45,  16       },
            {SOC_PPC_FP_ACTION_TYPE_ECMP_LB                  ,46,  20       },
            {SOC_PPC_FP_ACTION_TYPE_LAG_LB                   ,47,  20       },
            {SOC_PPC_FP_ACTION_TYPE_SEQUENCE_NUMBER_TAG      ,48,  16       },
            {SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST            ,49,  16       },
            {SOC_PPC_FP_ACTION_TYPE_IGNORE_CP                ,50,  1        },
            {SOC_PPC_FP_ACTION_TYPE_PPH_TYPE                 ,51,  2        },
            {SOC_PPC_FP_ACTION_TYPE_PACKET_IS_BOOTP_DHCP     ,52,  1        },
            {SOC_PPC_FP_ACTION_TYPE_UNKNOWN_ADDR             ,53,  1        },
            {SOC_PPC_FP_ACTION_TYPE_FWD_HDR_ENCAPSULATION    ,54,  2        },
            {SOC_PPC_FP_ACTION_TYPE_IEEE_1588                ,55,  1+1+1+2+7},
            {SOC_PPC_FP_ACTION_TYPE_OAM                      ,56,  1+3+7+7  },
            {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1            ,57,  32       }, /* Size set according to the SOC property in Bytes */
            {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2            ,58,  32       }, /* Size set according to the SOC property in Bytes */
            {SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT             ,59,  0        },
            {SOC_PPC_FP_ACTION_TYPE_NOP                      ,60,  0        },
            {SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED             ,5 ,  8+3      }, /* Trap strength + Trap code No Trap qualifier */
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER        ,-1,  18       },
            {SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP           ,-1,  13       },
            {SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY               ,-1,  20       }, /* 4b + cascaded-width encoding */
            {-1                                              ,-1,  -1       }  /* This is an indication of the last entry. DO NOT REMOVE! */
   };

static 
   uint32  /* action name, hw-id, size*/
       Arad_plus_pmf_fem_action_type_encoding[SOC_PPC_NOF_FP_ACTION_TYPES][NUM_ACTION_ELEMENTS] = {
           {SOC_PPC_FP_ACTION_TYPE_DEST                     ,0 ,  19       },
           {SOC_PPC_FP_ACTION_TYPE_TC                       ,1 ,  3        },
           {SOC_PPC_FP_ACTION_TYPE_DP                       ,2 ,  2        },
           {SOC_PPC_FP_ACTION_TYPE_EEI                      ,3 ,  24       },
           {SOC_PPC_FP_ACTION_TYPE_OUTLIF                   ,4 ,  16       },
           {SOC_PPC_FP_ACTION_TYPE_TRAP                     ,5 ,  8+3+16   }, /* Trap qualifier + Trap strength + Trap code */
                                                                  /* If SOC property custom_feature_reduced_trap_action=1, size is 11 bits: Trap strength + Trap code */
           {SOC_PPC_FP_ACTION_TYPE_SNP                      ,6 ,  8+2      },
           {SOC_PPC_FP_ACTION_TYPE_IN_PORT                  ,7 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_STAT                     ,8 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_VSQ_PTR                  ,9 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_MIRROR                   ,10,  4        },
           {SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY            ,11,  3        },
           {SOC_PPC_FP_ACTION_TYPE_MIR_DIS                  ,12,  1        },
           {SOC_PPC_FP_ACTION_TYPE_EXC_SRC                  ,13,  1        },
           {SOC_PPC_FP_ACTION_TYPE_IS                       ,14,  19       },
           {SOC_PPC_FP_ACTION_TYPE_METER_A                  ,15,  16       },
           {SOC_PPC_FP_ACTION_TYPE_METER_B                  ,16,  16       },
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_A                ,17,  16+1 /* 22+1(valid bit) only for Statistic-Record */ },
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_B                ,18,  16+1 /* 22+1(valid bit) only for Statistic-Record */ },
           {SOC_PPC_FP_ACTION_TYPE_DP_METER_COMMAND         ,19,  2        },
           {SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT            ,20,  16       },
           {SOC_PPC_FP_ACTION_TYPE_FWD_CODE                 ,21,  4        },
           {SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET               ,22,  3+6      },
           {SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE          ,23,  2+6      },
           {SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID ,24,  4        },
           {SOC_PPC_FP_ACTION_TYPE_VSI                      ,25,  16       },
           {SOC_PPC_FP_ACTION_TYPE_ORIENTATION_IS_HUB       ,26,  1        },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_COMMAND        ,27,  6        },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_1          ,28,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_2          ,29,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_PCP_DEI        ,30,  4        },
           {SOC_PPC_FP_ACTION_TYPE_IN_RIF                   ,31,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VRF                      ,32,  12       },
           {SOC_PPC_FP_ACTION_TYPE_IN_TTL                   ,33,  8        },
           {SOC_PPC_FP_ACTION_TYPE_IN_DSCP_EXP              ,34,  8        },
           {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION_VALID    ,35,  1        },
           {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION          ,36,  19       },
           {SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE     ,37,  1        },
           {SOC_PPC_FP_ACTION_TYPE_EGRESS_LEARN_ENABLE      ,38,  1        },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_FID                ,39,  15       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_0_TO_15         ,40,  16       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_16_TO_47        ,41,  32       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_0_TO_15       ,42,  16       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39      ,43,  24       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_OR_TRANSPLANT      ,44,  1        },
           {SOC_PPC_FP_ACTION_TYPE_IN_LIF                   ,45,  16       },
           {SOC_PPC_FP_ACTION_TYPE_ECMP_LB                  ,46,  20       },
           {SOC_PPC_FP_ACTION_TYPE_LAG_LB                   ,47,  20       },
           {SOC_PPC_FP_ACTION_TYPE_SEQUENCE_NUMBER_TAG      ,48,  16       },
           {SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST            ,49,  16       },
           {SOC_PPC_FP_ACTION_TYPE_IGNORE_CP                ,50,  1        },
           {SOC_PPC_FP_ACTION_TYPE_PPH_TYPE                 ,51,  2        },
           {SOC_PPC_FP_ACTION_TYPE_PACKET_IS_BOOTP_DHCP     ,52,  1        },
           {SOC_PPC_FP_ACTION_TYPE_UNKNOWN_ADDR             ,53,  1        },
           {SOC_PPC_FP_ACTION_TYPE_FWD_HDR_ENCAPSULATION    ,54,  2        },
           {SOC_PPC_FP_ACTION_TYPE_IEEE_1588                ,55,  1+1+1+2+7},
           {SOC_PPC_FP_ACTION_TYPE_OAM                      ,56,  1+3+7+7  },
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1            ,57,  32       }, /* Size set according to the SOC property in Bytes */
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2            ,58,  32       }, /* Size set according to the SOC property in Bytes */
           {SOC_PPC_FP_ACTION_TYPE_NATIVE_VSI               ,59,  1+12     },
           {SOC_PPC_FP_ACTION_TYPE_IN_LIF_PROFILE           ,60,  4        },
           {SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT             ,61,  0        },
           {SOC_PPC_FP_ACTION_TYPE_NOP                      ,62,  0        },
           {SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED             ,5 ,  8+3      }, /* Trap strength + Trap code No Trap qualifier */
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER        ,-1,  18       },
           {SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP           ,-1,  13       },
           {SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY               ,-1,  20       }, /* 4b + cascaded-width encoding */
           {-1                                              ,-1,  -1       }  /* This is an indication of the last entry. DO NOT REMOVE! */
  };

static 
   uint32  /* action name, hw-id, size*/
       Jericho_pmf_fem_action_type_encoding[SOC_PPC_NOF_FP_ACTION_TYPES][NUM_ACTION_ELEMENTS] = {
           {SOC_PPC_FP_ACTION_TYPE_DEST                     ,0 ,  19       },
           {SOC_PPC_FP_ACTION_TYPE_TC                       ,1 ,  3        },
           {SOC_PPC_FP_ACTION_TYPE_DP                       ,2 ,  2        },
           {SOC_PPC_FP_ACTION_TYPE_EEI                      ,3 ,  24       },
           {SOC_PPC_FP_ACTION_TYPE_OUTLIF                   ,4 ,  18       },
           {SOC_PPC_FP_ACTION_TYPE_TRAP                     ,5 ,  8+3+16   }, /* Trap qualifier + Trap strength + Trap code */
                                                                  /* If SOC property custom_feature_reduced_trap_action=1, size is 11 bits: Trap strength + Trap code */
           {SOC_PPC_FP_ACTION_TYPE_SNP                      ,6 ,  8+2      },
           {SOC_PPC_FP_ACTION_TYPE_IN_PORT                  ,7 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_STAT                     ,8 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_VSQ_PTR                  ,9 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_MIRROR                   ,10,  4        },
           {SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY            ,11,  3        },
           {SOC_PPC_FP_ACTION_TYPE_MIR_DIS                  ,12,  1        },
           {SOC_PPC_FP_ACTION_TYPE_EXC_SRC                  ,13,  1        },
           {SOC_PPC_FP_ACTION_TYPE_IS                       ,14,  19       },
           {SOC_PPC_FP_ACTION_TYPE_METER_A                  ,15,  18       },
           {SOC_PPC_FP_ACTION_TYPE_METER_B                  ,16,  18       },
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_A                ,17,  20+1 /* 22+1(valid bit) only for Statistic-Record */ },
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_B                ,18,  20+1 /* 22+1(valid bit) only for Statistic-Record */ },
           {SOC_PPC_FP_ACTION_TYPE_DP_METER_COMMAND         ,19,  2        },
           {SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT            ,20,  16       },
           {SOC_PPC_FP_ACTION_TYPE_FWD_CODE                 ,21,  4        },
           {SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET               ,22,  3+6      },
           {SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE          ,23,  2+6      },
           {SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID ,24,  4        },
           {SOC_PPC_FP_ACTION_TYPE_VSI                      ,25,  16       },
           {SOC_PPC_FP_ACTION_TYPE_ORIENTATION_IS_HUB       ,26,  1        },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_COMMAND        ,27,  6        },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_1          ,28,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_2          ,29,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_PCP_DEI        ,30,  4        },
           {SOC_PPC_FP_ACTION_TYPE_IN_RIF                   ,31,  15       },
           {SOC_PPC_FP_ACTION_TYPE_VRF                      ,32,  14       },
           {SOC_PPC_FP_ACTION_TYPE_IN_TTL                   ,33,  8        },
           {SOC_PPC_FP_ACTION_TYPE_IN_DSCP_EXP              ,34,  8        },
           {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION_VALID    ,35,  1        },
           {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION          ,36,  19       },
           {SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE     ,37,  1        },
           {SOC_PPC_FP_ACTION_TYPE_EGRESS_LEARN_ENABLE      ,38,  1        },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_FID                ,39,  15       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_0_TO_15         ,40,  16       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_16_TO_47        ,41,  32       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_0_TO_15       ,42,  16       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39      ,43,  24       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_OR_TRANSPLANT      ,44,  1        },
           {SOC_PPC_FP_ACTION_TYPE_IN_LIF                   ,45,  18       },
           {SOC_PPC_FP_ACTION_TYPE_ECMP_LB                  ,46,  20       },
           {SOC_PPC_FP_ACTION_TYPE_LAG_LB                   ,47,  20       },
           {SOC_PPC_FP_ACTION_TYPE_SEQUENCE_NUMBER_TAG      ,48,  16       },
           {SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST            ,49,  16       },
           {SOC_PPC_FP_ACTION_TYPE_IGNORE_CP                ,50,  1        },
           {SOC_PPC_FP_ACTION_TYPE_PPH_TYPE                 ,51,  2        },
           {SOC_PPC_FP_ACTION_TYPE_PACKET_IS_BOOTP_DHCP     ,52,  1        },
           {SOC_PPC_FP_ACTION_TYPE_UNKNOWN_ADDR             ,53,  1        },
           {SOC_PPC_FP_ACTION_TYPE_FWD_HDR_ENCAPSULATION    ,54,  2        },
           {SOC_PPC_FP_ACTION_TYPE_IEEE_1588                ,55,  1+1+1+2+7},
           {SOC_PPC_FP_ACTION_TYPE_OAM                      ,56,  1+3+7+7  },
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1            ,57,  32       }, /* Size set according to the SOC property in Bytes */
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2            ,58,  32       }, /* Size set according to the SOC property in Bytes */
           {SOC_PPC_FP_ACTION_TYPE_NATIVE_VSI               ,59,  1+12     },
           {SOC_PPC_FP_ACTION_TYPE_IN_LIF_PROFILE           ,60,  4        },
           {SOC_PPC_FP_ACTION_TYPE_IN_RIF_PROFILE           ,61,  4        },
           {SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT             ,62,  0        },
           {SOC_PPC_FP_ACTION_TYPE_NOP                      ,63,  0        },
           {SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED             ,5 ,  8+3      }, /* Trap strength + Trap code No Trap qualifier */
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER        ,-1,  22       },
           {SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP           ,-1,  13       },
           {SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY               ,-1,  24       }, /* 4b + cascaded-width encoding */
           {-1                                              ,-1,  -1       }  /* This is an indication of the last entry. DO NOT REMOVE! */
  };

static 
   uint32  /* action name, hw-id, size*/
       Qax_pmf_fem_action_type_encoding[SOC_PPC_NOF_FP_ACTION_TYPES][NUM_ACTION_ELEMENTS] = {
           {SOC_PPC_FP_ACTION_TYPE_DEST                     ,0 ,  19       },
           {SOC_PPC_FP_ACTION_TYPE_DEST_DROP                ,1 ,  1        },
           {SOC_PPC_FP_ACTION_TYPE_TC                       ,2 ,  3        },
           {SOC_PPC_FP_ACTION_TYPE_DP                       ,3 ,  2        },
           {SOC_PPC_FP_ACTION_TYPE_EEI                      ,4 ,  24       },
           {SOC_PPC_FP_ACTION_TYPE_OUTLIF                   ,5 ,  18       },
           {SOC_PPC_FP_ACTION_TYPE_TRAP                     ,6 ,  8+3+16   }, /* Trap qualifier + Trap strength + Trap code */
                                                                  /* If SOC property custom_feature_reduced_trap_action=1, size is 11 bits: Trap strength + Trap code */
           {SOC_PPC_FP_ACTION_TYPE_SNP                      ,7 ,  8+2      },
           {SOC_PPC_FP_ACTION_TYPE_IN_PORT                  ,8 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_STAT                     ,9 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_VSQ_PTR                  ,10 ,  8        },
           {SOC_PPC_FP_ACTION_TYPE_MIRROR                   ,11,  4        },
           {SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY            ,-1,  3        }, /* Not valid */
           {SOC_PPC_FP_ACTION_TYPE_MIR_DIS                  ,12,  1        },
           {SOC_PPC_FP_ACTION_TYPE_EXC_SRC                  ,13,  1        },
           {SOC_PPC_FP_ACTION_TYPE_IS                       ,14,  19       },
           {SOC_PPC_FP_ACTION_TYPE_METER_A                  ,15,  18       },
           {SOC_PPC_FP_ACTION_TYPE_METER_B                  ,16,  18       },
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_A                ,17,  20+1 /* 22+1(valid bit) only for Statistic-Record */ },
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_B                ,18,  20+1 /* 22+1(valid bit) only for Statistic-Record */ },
           {SOC_PPC_FP_ACTION_TYPE_DP_METER_COMMAND         ,19,  2        },
           {SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT            ,20,  16       },
           {SOC_PPC_FP_ACTION_TYPE_FWD_CODE                 ,21,  4        },
           {SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET               ,22,  3+6      },
           {SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE          ,23,  2+6      },
           {SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID ,24,  4        },
           {SOC_PPC_FP_ACTION_TYPE_VSI                      ,25,  16       },
           {SOC_PPC_FP_ACTION_TYPE_ORIENTATION_IS_HUB       ,26,  1        },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_COMMAND        ,27,  6        },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_1          ,28,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_2          ,29,  12       },
           {SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_PCP_DEI        ,30,  4        },
           {SOC_PPC_FP_ACTION_TYPE_IN_RIF                   ,31,  15       },
           {SOC_PPC_FP_ACTION_TYPE_VRF                      ,32,  14       },
           {SOC_PPC_FP_ACTION_TYPE_IN_TTL                   ,33,  8        },
           {SOC_PPC_FP_ACTION_TYPE_IN_DSCP_EXP              ,34,  8        },
           {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION_VALID    ,35,  1        },
           {SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION          ,36,  19       },
           {SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE     ,37,  1        },
           {SOC_PPC_FP_ACTION_TYPE_EGRESS_LEARN_ENABLE      ,38,  1        },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_FID                ,39,  15       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_0_TO_15         ,40,  16       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_SA_16_TO_47        ,41,  32       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_0_TO_15       ,42,  16       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39      ,43,  24       },
           {SOC_PPC_FP_ACTION_TYPE_LEARN_OR_TRANSPLANT      ,44,  1        },
           {SOC_PPC_FP_ACTION_TYPE_IN_LIF                   ,45,  18       },
           {SOC_PPC_FP_ACTION_TYPE_ECMP_LB                  ,46,  20       },
           {SOC_PPC_FP_ACTION_TYPE_LAG_LB                   ,47,  20       },
           {SOC_PPC_FP_ACTION_TYPE_SEQUENCE_NUMBER_TAG      ,-1,  16       }, /* Not valid */
           {SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST            ,48,  16       },
           {SOC_PPC_FP_ACTION_TYPE_IGNORE_CP                ,49,  1        },
           {SOC_PPC_FP_ACTION_TYPE_PPH_TYPE                 ,50,  2        },
           {SOC_PPC_FP_ACTION_TYPE_PACKET_IS_BOOTP_DHCP     ,51,  1        },
           {SOC_PPC_FP_ACTION_TYPE_UNKNOWN_ADDR             ,52,  1        },
           {SOC_PPC_FP_ACTION_TYPE_FWD_HDR_ENCAPSULATION    ,53,  2        },
           {SOC_PPC_FP_ACTION_TYPE_IEEE_1588                ,54,  1+1+1+2+7},
           {SOC_PPC_FP_ACTION_TYPE_OAM                      ,55,  1+3+7+7  },
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1            ,56,  32       }, /* Size set according to the SOC property in Bytes */
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2            ,57,  32       }, /* Size set according to the SOC property in Bytes */
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_3            ,58 , 32       }, 
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_4            ,59 , 32       }, 
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1_TYPE       ,60 , 2        }, 
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2_TYPE       ,61 , 2        }, 
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_3_TYPE       ,62 , 2        }, 
           {SOC_PPC_FP_ACTION_TYPE_USER_HEADER_4_TYPE       ,63 , 2        }, 
           {SOC_PPC_FP_ACTION_TYPE_NATIVE_VSI               ,64,  1+15     },
           {SOC_PPC_FP_ACTION_TYPE_IN_LIF_PROFILE           ,65,  4        },
           {SOC_PPC_FP_ACTION_TYPE_IN_RIF_PROFILE           ,66,  4        },        
           {SOC_PPC_FP_ACTION_TYPE_ITPP_DELTA               ,67 , 8+1      }, /* +1 is for valid bit */
           {SOC_PPC_FP_ACTION_TYPE_STATISTICS_POINTER_0     ,68 , 21       }, 
           {SOC_PPC_FP_ACTION_TYPE_STATISTICS_POINTER_1     ,69 , 21       }, 
           {SOC_PPC_FP_ACTION_TYPE_LATENCY_FLOW_ID          ,70 , 19       }, 
           {SOC_PPC_FP_ACTION_TYPE_ADMIT_PROFILE            ,71 , 3        },         
           {SOC_PPC_FP_ACTION_TYPE_PEM_CONTEXT              ,72 , 0        }, 
           {SOC_PPC_FP_ACTION_TYPE_PEM_GENERAL_DATA_0       ,73 , 0        }, 
           {SOC_PPC_FP_ACTION_TYPE_PEM_GENERAL_DATA_1       ,74 , 0        }, 
           {SOC_PPC_FP_ACTION_TYPE_PPH_RESERVE_VALUE        ,75 , 2+1      }, /* +1 is for valid bit */
           {SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT             ,76,  0        }, 
           {SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED             ,6 ,  8+3      }, /* Trap strength + Trap code No Trap qualifier */
           {SOC_PPC_FP_ACTION_TYPE_NOP                      ,127,  0       }, /* Not in use */
           {SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER        ,-1,  22       },
           {SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP           ,-1,  13       },
           {SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY               ,-1,  24       }, /* 4b + cascaded-width encoding */
           {SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_0,-1,  2        }, /* 2 LSBs of action result 0*/
           {SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_1,-1,  2        }, /* 2 LSBs of action result 1*/
           {SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_2,-1,  2        }, /* 2 LSBs of action result 2*/
           {SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_3,-1,  2        }, /* 2 LSBs of action result 3*/
           {SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_KAPS,-1,  2     }, /* 2 LSBs of action KAPS payload*/
           {-1                                              ,-1,  -1       }  /* This is an indication of the last entry. DO NOT REMOVE! */
  };
/* } */
/* The following
 * four arrays contain the RUNTIME (current) values for the width per action.
 * Arrays are for Arad, Arad+ and Jericho.
 * For each action, there are three parameters:
 * Action identifier, Internal (HW) identifier, no. of bits currently assigned (width).
 * Number of entries is 'SOC_PPC_NOF_FP_ACTION_TYPES' - See SOC_PPC_FP_ACTION_TYPE
 * These arrays are initialized from the arrays above (same name, without the 'runtime' suffix)
 *   uint32 Arad_pmf_fem_action_type_encoding_runtime[SOC_PPC_NOF_FP_ACTION_TYPES][3] ;
 *   uint32 Arad_plus_pmf_fem_action_type_encoding_runtime[SOC_PPC_NOF_FP_ACTION_TYPES][3] ;
 *   uint32 Jericho_pmf_fem_action_type_encoding_runtime[SOC_PPC_NOF_FP_ACTION_TYPES][3] ;
 *   uint32 Qax_pmf_fem_action_type_encoding_runtime[SOC_PPC_NOF_FP_ACTION_TYPES][3] ;
 *
 * Each array is assumed to have at least three action entries (first index).
 *
 * Regarding ISSU:
 *   If the first three 'action' elements are '0' then we assume table has not
 *   been initialized.
 *
 * See
 *   bcm_dpp_field_info_t, bcm_petra_field_action_width_get(), bcm_petra_field_action_width_set(),
 *   arad_pmf_fem_action_type_array_size_get_unsafe(),arad_pmf_fem_action_type_array_element_get_unsafe()
 *
 * To access those tables, use, for example:
 *   Set, on ARAD, the runtime width of action 'actionIndex', to 11:
 *     FIELD_ACCESS_ARAD_ACTION_RUNTIME.set(unit,actionIndex,2,11)
 *   Same for
 *     FIELD_ACCESS_ARADP_ACTION_RUNTIME
 *     FIELD_ACCESS_JERICHO_ACTION_RUNTIME
 */

/* } */

CONST soc_mem_t 
     Arad_pmf_fem_map_tbl[ARAD_PMF_LOW_LEVEL_NOF_FEMS] = {
       IHB_FEM_0_4B_MAP_TABLEm, IHB_FEM_1_4B_MAP_TABLEm, 
       IHB_FEM_2_19B_MAP_TABLEm, IHB_FEM_3_19B_MAP_TABLEm, IHB_FEM_4_19B_MAP_TABLEm, 
       IHB_FEM_5_19B_MAP_TABLEm, IHB_FEM_6_24B_MAP_TABLEm, IHB_FEM_7_24B_MAP_TABLEm, 
       IHB_FEM_8_4B_MAP_TABLEm, IHB_FEM_9_4B_MAP_TABLEm, 
       IHB_FEM_10_19B_MAP_TABLEm, IHB_FEM_11_19B_MAP_TABLEm, IHB_FEM_12_19B_MAP_TABLEm, 
       IHB_FEM_13_19B_MAP_TABLEm, IHB_FEM_14_24B_MAP_TABLEm, IHB_FEM_15_24B_MAP_TABLEm};


CONST soc_field_t 
     Arad_pmf_fem_map_field_select_field[] = {
       FIELD_SELECT_MAP_0f, FIELD_SELECT_MAP_1f, FIELD_SELECT_MAP_2f, FIELD_SELECT_MAP_3f, FIELD_SELECT_MAP_4f, 
       FIELD_SELECT_MAP_5f, FIELD_SELECT_MAP_6f, FIELD_SELECT_MAP_7f, FIELD_SELECT_MAP_8f, FIELD_SELECT_MAP_9f,
       FIELD_SELECT_MAP_10f, FIELD_SELECT_MAP_11f, FIELD_SELECT_MAP_12f, FIELD_SELECT_MAP_13f, FIELD_SELECT_MAP_14f, 
       FIELD_SELECT_MAP_15f, FIELD_SELECT_MAP_16f, FIELD_SELECT_MAP_17f, FIELD_SELECT_MAP_18f, FIELD_SELECT_MAP_19f,
       FIELD_SELECT_MAP_20f, FIELD_SELECT_MAP_21f, FIELD_SELECT_MAP_22f, FIELD_SELECT_MAP_23f
     };

static
soc_field_t 
     Arad_pmf_fem_instruction_field[ARAD_PMF_LOW_LEVEL_NOF_FEMS] = {
         FEM_0_INSTRUCTIONf, FEM_1_INSTRUCTIONf, FEM_2_INSTRUCTIONf,  FEM_3_INSTRUCTIONf,  FEM_4_INSTRUCTIONf,  
         FEM_5_INSTRUCTIONf, FEM_6_INSTRUCTIONf, FEM_7_INSTRUCTIONf, FEM_8_INSTRUCTIONf, FEM_9_INSTRUCTIONf, 
         FEM_10_INSTRUCTIONf, FEM_11_INSTRUCTIONf, FEM_12_INSTRUCTIONf, 
         FEM_13_INSTRUCTIONf, FEM_14_INSTRUCTIONf, FEM_15_INSTRUCTIONf
     };

static
soc_field_t 
     Arad_pmf_fes_instruction_field[ARAD_PMF_LOW_LEVEL_NOF_FESS] = {
         FES_0_INSTRUCTIONf, FES_1_INSTRUCTIONf, FES_2_INSTRUCTIONf, FES_3_INSTRUCTIONf, FES_4_INSTRUCTIONf, FES_5_INSTRUCTIONf, 
         FES_6_INSTRUCTIONf, FES_7_INSTRUCTIONf, FES_8_INSTRUCTIONf, FES_9_INSTRUCTIONf, 
         FES_10_INSTRUCTIONf,FES_11_INSTRUCTIONf,FES_12_INSTRUCTIONf,FES_13_INSTRUCTIONf,FES_14_INSTRUCTIONf,FES_15_INSTRUCTIONf,
         FES_16_INSTRUCTIONf,FES_17_INSTRUCTIONf,FES_18_INSTRUCTIONf,FES_19_INSTRUCTIONf,
         FES_20_INSTRUCTIONf,FES_21_INSTRUCTIONf,FES_22_INSTRUCTIONf,FES_23_INSTRUCTIONf,FES_24_INSTRUCTIONf,FES_25_INSTRUCTIONf,
         FES_26_INSTRUCTIONf,FES_27_INSTRUCTIONf,FES_28_INSTRUCTIONf,FES_29_INSTRUCTIONf,
         FES_30_INSTRUCTIONf,FES_31_INSTRUCTIONf
     };

CONST STATIC 
    ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL
        Arad_pmf_fem_action_egress_pmf_signal[SOC_DPP_NOF_EGRESS_PMF_ACTIONS_ARAD] = {
            {SOC_PPC_FP_ACTION_TYPE_EGR_TRAP,        129, 126, 2, 0, 35,  4}, /* Action-Index, i.e. Trap */
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_PROFILE, 121, 119, 2, 0, 32,  3}, /* Counter-Profile */
            {SOC_PPC_FP_ACTION_TYPE_COS_PROFILE,     125, 122, 2, 0, 4,   4}, /* Cos-Profile */
            {SOC_PPC_FP_ACTION_TYPE_ACE_POINTER,     141, 130, 2, 0, 8,  12}, /* ACE-Pointer */
            {SOC_PPC_FP_ACTION_TYPE_EGR_OFP,          53,  45, 2, 0, 20,  8}, /* Change the TM-Port */
            {SOC_PPC_FP_ACTION_TYPE_TC,               43,  41, 2, 0, 28,  4}, /* Traffic-Class - LSB is valid */
            {SOC_PPC_FP_ACTION_TYPE_MIRROR,           65,  62, 2, 0, 0,   4}, /* Mirror-Profile */
        };

CONST STATIC 
    ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL
        Jericho_pmf_fem_action_egress_pmf_signal[SOC_DPP_NOF_EGRESS_PMF_ACTIONS_JERICHO] = {
            {SOC_PPC_FP_ACTION_TYPE_EGR_TRAP,        139, 136, 2, 0, 41,  4}, /* Action-Index, i.e. Trap */
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_PROFILE, 131, 129, 2, 0, 38,  3}, /* Counter-Profile */
            {SOC_PPC_FP_ACTION_TYPE_COS_PROFILE,     135, 132, 2, 0, 4,   4}, /* Cos-Profile */
            {SOC_PPC_FP_ACTION_TYPE_ACE_TYPE,        141, 140, 2, 0, 8,   2}, /* ACE-type */
            {SOC_PPC_FP_ACTION_TYPE_ACE_POINTER,     153, 142, 2, 0, 10, 12}, /* ACE-Pointer */
            {SOC_PPC_FP_ACTION_TYPE_EGR_OFP,           9,   1, 2, 0, 22,  9}, /* Change the PP-DSP (was Out-TM-Port in Arad)  - LSB is valid */
            {SOC_PPC_FP_ACTION_TYPE_TC,               45,  43, 2, 0, 31,  4}, /* Traffic-Class - LSB is valid */
            {SOC_PPC_FP_ACTION_TYPE_DP,                0,   0, 2, 0, 35,  3},   /* Drop-precedence - LSB is valid - no signal for it */
            {SOC_PPC_FP_ACTION_TYPE_MIRROR,           67,  64, 2, 0, 0,   4}, /* Mirror-Profile */
        };

/* Data Structures serving "diag field res" representation */
STATIC
    ARAD_PMF_FEM_ACTION_EGRESS_INFO
        Arad_pmf_fem_action_egress_pmf_info[] = {
            {SOC_PPC_FP_ACTION_TYPE_EGR_TRAP,        35,  4, -1}, /* Action-Index, i.e. Trap */
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_PROFILE, 32,  3, -1}, /* Counter-Profile */
            {SOC_PPC_FP_ACTION_TYPE_COS_PROFILE,     4,   4, -1}, /* Cos-Profile */
            {SOC_PPC_FP_ACTION_TYPE_ACE_POINTER,     8,  12, -1}, /* ACE-Pointer */
            {SOC_PPC_FP_ACTION_TYPE_EGR_OFP,         20,  8, -1}, /* Change the TM-Port */
            {SOC_PPC_FP_ACTION_TYPE_TC,              29,  3, 28}, /* Traffic-Class - LSB is valid */
            {SOC_PPC_FP_ACTION_TYPE_MIRROR,          0,   4, -1}, /* Mirror-Profile */
            {SOC_PPC_NOF_FP_ACTION_TYPES,            0,   0, -1}
        };

STATIC
    ARAD_PMF_FEM_ACTION_EGRESS_INFO
        Jericho_pmf_fem_action_egress_pmf_info[] = {
            {SOC_PPC_FP_ACTION_TYPE_EGR_TRAP,        41,  4, -1}, /* Action-Index, i.e. Trap */
            {SOC_PPC_FP_ACTION_TYPE_COUNTER_PROFILE, 38,  3, -1}, /* Counter-Profile */
            {SOC_PPC_FP_ACTION_TYPE_COS_PROFILE,     4,   4, -1}, /* Cos-Profile */
            {SOC_PPC_FP_ACTION_TYPE_ACE_TYPE,        8,   2, -1}, /* ACE-type */
            {SOC_PPC_FP_ACTION_TYPE_ACE_POINTER,     10, 12, -1}, /* ACE-Pointer */
            {SOC_PPC_FP_ACTION_TYPE_EGR_OFP,         23,  8, 22}, /* Change the PP-DSP (was Out-TM-Port in Arad)  - LSB is valid */
            {SOC_PPC_FP_ACTION_TYPE_TC,              32,  3, 31}, /* Traffic-Class - LSB is valid */
            {SOC_PPC_FP_ACTION_TYPE_DP,           36,  2, 35},  /* Drop-precedence - LSB is valid - no signal for it */
            {SOC_PPC_FP_ACTION_TYPE_MIRROR,          0,   4, -1}, /* Mirror-Profile */
            {SOC_PPC_NOF_FP_ACTION_TYPES,            0,   0, -1}
        };

/* Synchronize the PMF qualifiers ELK-Lookup-Result locations in the internal buffer according to the FLP actions sizes */
static 
    ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL
        Arad_pmf_fem_action_ingress_flp_signal[SOC_PPC_FP_NOF_INGRESS_FLP_ACTIONS] = {
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_0,    0, 0, 0, 0 , (64+64+24+32+16)	, 48}, 
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_1,    0, 0, 0, 0 , (64+64+24+32)		, 16}, 
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_2,    0, 0, 0, 0 , (64+64+24)		, 32},
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_3,    0, 0, 0, 0 , (64+64)			, 24},
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_4,    0, 0, 0, 0 , (64)				, 64},
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_5,    0, 0, 0, 0 , (0)				, 64},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_1ST_RESULT_0,    0, 0, 0, 0 , 0, 32},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_1ST_RESULT_1,    0, 0, 0, 0 , 32, 16},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_0,    0, 0, 0, 0 , 0, 32},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_1,    0, 0, 0, 0 , 32, 16},
        };

static 
    ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL
        Arad_plus_pmf_fem_action_ingress_flp_signal[SOC_PPC_FP_NOF_INGRESS_FLP_ACTIONS] = {
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_0,    0, 0, 0, 0 , (64+64+24+32+16)	, 48}, 
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_1,    0, 0, 0, 0 , (64+64+24+32)		, 32}, 
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_2,    0, 0, 0, 0 , (64+64+24)		, 24},
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_3,    0, 0, 0, 0 , (64+64)			, 16},
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_4,    0, 0, 0, 0 , (64)				, 64},
            {SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_5,    0, 0, 0, 0 , (0)				, 64},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_1ST_RESULT_0,    0, 0, 0, 0 , 0, 32},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_1ST_RESULT_1,    0, 0, 0, 0 , 32, 16},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_0,    0, 0, 0, 0 , 0, 32},
            {SOC_PPC_FP_ACTION_TYPE_FLP_LEM_2ND_RESULT_1,    0, 0, 0, 0 , 32, 16},
        };

CONST STATIC 
    ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL
        Arad_pmf_fem_action_ingress_slb_signal[SOC_PPC_FP_NOF_INGRESS_SLB_ACTIONS] = {
            {SOC_PPC_FP_ACTION_TYPE_SLB_HASH_VALUE,    0, 0, 0, 0, 0, 0}, /* Pre-hashing Action type */
            {SOC_PPC_FP_ACTION_TYPE_LAG_LB,    0, 0, 0, 0, 0, 0}, /* Post-hashing Action type */
        };

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* Reference table for fields filling */
uint32 
     arad_pmf_fem_map_tbl_reference(
        SOC_SAND_IN  int                         unit,
        SOC_SAND_IN  uint32                      fem_type,
        SOC_SAND_OUT soc_mem_t                   *mem
     )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    SOC_SAND_CHECK_NULL_INPUT(mem);
    
    switch(fem_type) {
        case 0:
            *mem = IHB_FEM_0_4B_MAP_TABLEm;
            break;
        case 1:
            *mem = IHB_FEM_2_19B_MAP_TABLEm;
            break;
        case 2:
            if(SOC_IS_JERICHO(unit)) {
                *mem = IHB_FEM_6_24B_MAP_TABLEm;
            } else {
                *mem = IHB_FEM_5_19B_MAP_TABLEm;
            }
            break;
        default:
            SOC_SAND_SET_ERROR_CODE(ARAD_PMF_FEM_TYPE_OUT_OF_RANGE_ERR, 10, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_map_tbl_reference()", 0, 0);
}

/* Get the attributes of the egress actions */
uint32
  arad_pmf_fem_action_egress_info_get(
      SOC_SAND_IN  int                           unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE               action_type,
      SOC_SAND_OUT uint8                             *is_found,
      SOC_SAND_OUT ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL *action_egress_info
  )
{
    uint32
        table_line;
    uint8
        found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  found = FALSE;
  for (table_line = 0; (table_line < SOC_DPP_DEFS_GET(unit, nof_egress_pmf_actions)) && (!found); ++table_line) {
      if (action_type == 
            (SOC_IS_JERICHO(unit)? Jericho_pmf_fem_action_egress_pmf_signal[table_line].action_type: 
                                    Arad_pmf_fem_action_egress_pmf_signal[table_line].action_type)) {
          found = TRUE;
          break;
      }
  }

  *is_found = found;
  if (found) {
      if (SOC_IS_JERICHO(unit)) {
          sal_memcpy(action_egress_info, &(Jericho_pmf_fem_action_egress_pmf_signal[table_line]), sizeof(ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL));
      } else {
          sal_memcpy(action_egress_info, &(Arad_pmf_fem_action_egress_pmf_signal[table_line]), sizeof(ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL));
      }
  }

  ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_egress_info_get()", 0, 0);
}



uint32
  arad_pmf_low_level_fem_tag_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
    uint32
        tbl_data[ARAD_PMF_FEM_TABLE_LENGTH_IN_UINT32S],
        pmf_pgm_indx =0,
        cycle = 0,
        offset = 0,
        instruction,
        instruction_idx,
        instruction_idx_from,
        instruction_idx_to,
      res = SOC_SAND_OK;
    soc_field_t
      instruction_field;
    uint32 nof_ingress_pmf_pgms = SOC_DPP_DEFS_GET(unit, nof_ingress_pmf_programs);
    uint32 key_select_nop = SOC_DPP_DEFS_GET(unit, fem_key_select_nop);

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*
   *    Do not disable any actions by default
   */


  /*
   *    Init the FEMs - Assumption that all the tables are zeroed 
    *               except IHB_PMF_FEM_PROGRAM  - KE_SELECT field should be set to nop according to device type
   */
  for(cycle = 0 ; cycle < ARAD_PMF_NOF_CYCLES ; cycle++)
  {
      offset = cycle*nof_ingress_pmf_pgms; 
      instruction_idx_from = cycle*ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP;
      instruction_idx_to = cycle*ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP + ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP;
      for(pmf_pgm_indx =0 ; pmf_pgm_indx < nof_ingress_pmf_pgms; pmf_pgm_indx++)
      {
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PMF_FEM_PROGRAMm(unit, MEM_BLOCK_ANY, pmf_pgm_indx+offset, tbl_data));
        for(instruction_idx = instruction_idx_from ; instruction_idx < instruction_idx_to ;instruction_idx++)
        {
            instruction_field = Arad_pmf_fem_instruction_field[instruction_idx];
            instruction = soc_mem_field32_get(unit,IHB_PMF_FEM_PROGRAMm,tbl_data,instruction_field);
            soc_mem_field32_set(unit, IHB_PMF_FEM_PROGRAMm, &instruction, FEM_KEY_SELECTf,key_select_nop);
            soc_mem_field32_set(unit, IHB_PMF_FEM_PROGRAMm, tbl_data, instruction_field, instruction);
        }  
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 402, exit, WRITE_IHB_PMF_FEM_PROGRAMm(unit, MEM_BLOCK_ANY, pmf_pgm_indx+offset, tbl_data));
      }      
  }

  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_TABLE_LENGTH_IN_UINT32S * sizeof(uint32));
  /* zero all the tables */
  res = arad_fill_table_with_entry(unit, IHB_PMF_PROGRAM_GENERALm, MEM_BLOCK_ANY, tbl_data); /* fill memory with the entry in data */
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);



  ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_low_level_fem_tag_init_unsafe()", 0, 0);
}

/*
 *    Return the FEM output size (4, 14, 17 bits)
 */
uint32
  arad_pmf_fem_output_size_get(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX      *fem_ndx,
    SOC_SAND_OUT uint32            *output_size_in_bits
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_OUTPUT_SIZE_GET);

  switch(fem_ndx->id)
  {
  case 0:
  case 1:
  case 8:
  case 9:
    *output_size_in_bits = ARAD_PMF_FEM_SIZE_1ST_SIZE;
    break;
  
  case 2:
  case 3:
  case 4:
  case 10:
  case 11:
  case 12:
    *output_size_in_bits = ARAD_PMF_FEM_SIZE_2ND_SIZE;
    break;

  case 5:
  case 13:
      *output_size_in_bits = SOC_IS_JERICHO(unit)? ARAD_PMF_FEM_SIZE_2ND_SIZE: ARAD_PMF_FEM_SIZE_3RD_SIZE;
      break;

  case 6:
  case 7:
  case 14:
  case 15:
    *output_size_in_bits = ARAD_PMF_FEM_SIZE_3RD_SIZE;
    break;

  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_output_size_get()", 0, 0);
}


/*****************************************************************************
* Function:  arad_pmf_fem_pgm_duplicate
* Purpose:  Since different PMF program can haave different FEM programs when duplicating a PMF program
                    FEM program might need to be changed also
* Params:
* unit (IN)                             - Device Number
* stage (IN)                            - PMF stage
* pmf_pgm_from  (IN)           - PMF program to copy from
* pmf_pgm_to  (IN)           - PMF program to copy from
* mem_offset  (IN)           - offset in memory, FEM 8-15 are copied to lines 32-63 ,
                                            Hence offset can be 0 or 32
* Return:    (uint32)  Error in case of error
*******************************************************************************/

uint32
    arad_pmf_fem_pgm_duplicate(
          SOC_SAND_IN  int                   unit,
          SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
          SOC_SAND_IN  uint32                   pmf_pgm_from,
          SOC_SAND_IN  uint32                   pmf_pgm_to,
          SOC_SAND_IN  uint32                   mem_offset
      )
{
    uint32 
        res = SOC_SAND_OK,
        table_entry[20] = {0};
    uint8
        fem_pgm_id_from;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);


    /*Duplicate the FEM PROGRAM TABLE*/
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_PMF_FEM_PROGRAMm(unit, MEM_BLOCK_ANY, pmf_pgm_from+mem_offset, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 402, exit, WRITE_IHB_PMF_FEM_PROGRAMm(unit, MEM_BLOCK_ANY, pmf_pgm_to+mem_offset, table_entry));

    /*Get the FEM PGM of duplicated program*/
    res = arad_pp_fp_fem_pgm_per_pmf_pgm_get(unit,stage, pmf_pgm_from,&fem_pgm_id_from);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    /*Duplicate the FEM program also if its configured*/
    if(fem_pgm_id_from != ARAD_PMF_FEM_PGM_INVALID)
    {
        res = arad_pp_fp_fem_pgm_per_pmf_pgm_set(unit,stage,pmf_pgm_to,fem_pgm_id_from);
        SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);
    }

    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_pgm_duplicate()", pmf_pgm_from, pmf_pgm_to); 
}


/*********************************************************************
*     Select the input for each Field Extraction MACRO. The FEM
 *     processes the PCL results, and extracts possible actions
 *     to be applied on the packet. INPUT SOC_SAND_IN
 *     SOC_PPD_PMF_LKP_PROFILE *lkp_profile_ndx - Lookup-Profile
 *     information (id and cycle). SOC_SAND_IN uint32 *fem_ndx -
 *     FEM (Field Extraction Macro) Index. Range: 0 - 7.
 *     (Arad-B) SOC_SAND_IN SOC_PPD_PMF_FEM_INPUT_INFO *info - FEM
 *     input parameters: the FEM-Program-Id and the FEM-Input
 *     source. RETURNS OK or Error indicationREMARKS None.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_db_fem_input_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint8                    is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO             *info
  )
{
  uint32
      group_idx =  (is_fes)? ((fem_fes_ndx >= 16)? 0x1:0x0) : ((fem_fes_ndx >= 8)? 0x1:0x0), 
      is_src_above_80 = (info->src_arad.key_lsb >= ARAD_PMF_FEM_KEY_LSB_FIRST_IN_MSB)? 0x1:0x0,
      instruction,
    fld_val,
    res = SOC_SAND_OK,
    tbl_data[ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX];
  soc_mem_t
      fem_input_table;
  soc_field_t
      instruction_field;
  SOC_PPC_FP_DATABASE_STAGE            
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FEM_INPUT_SET_UNSAFE);


  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "     "
                        "Action: FEM/FES set, "
                        "FES/FEM-ID: %d, "
                        "PMF-Program: %d, "
                        "1-FES (0-FEM): %d, \n\r"),
             fem_fes_ndx, pmf_pgm_ndx, is_fes));
      
  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "     "
                        "FES info: "
                        "FEM-Program: %d, "
                        "source: "
                        "is_key_src: %d, "
                        "key_tcam_id: %d, "
                        "key_lsb: %d, "
                        "lookup_cycle_id: %d, "
                        "is_16_lsb_overridden: %d, \n\r"), info->pgm_id, info->src_arad.is_key_src, info->src_arad.key_tcam_id, info->src_arad.key_lsb, info->src_arad.lookup_cycle_id, info->is_16_lsb_overridden));
  if (info->is_16_lsb_overridden) {
      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "  source: "
                            "is_key_src: %d, "
                            "key_tcam_id: %d, "
                            "key_lsb: %d, "
                            "lookup_cycle_id: %d, "
                            "db_id: %d\n\r"),
                 info->lsb16_src.is_key_src, info->lsb16_src.key_tcam_id, info->lsb16_src.key_lsb, info->lsb16_src.lookup_cycle_id, info->db_id));
  } else {
      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "  db_id: %d\n\r"),info->db_id));
  }
  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX * sizeof(uint32));

  SOC_SAND_CHECK_NULL_INPUT(info);

  res = arad_pmf_db_fem_input_set_verify(
          unit,
          pmf_pgm_ndx,
          is_fes,
          fem_fes_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /* Get the correct table */
  fem_input_table = (is_fes)? IHB_PMF_FES_PROGRAMm: IHB_PMF_FEM_PROGRAMm;

  /* Read-Modify-Write due to the FES table other params */
  res = soc_mem_read(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (group_idx * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* Get the instruction  */
  instruction_field = (is_fes)? Arad_pmf_fes_instruction_field[fem_fes_ndx]: Arad_pmf_fem_instruction_field[fem_fes_ndx];
  instruction = soc_mem_field32_get(unit, fem_input_table, tbl_data, instruction_field);


  /*
   *    Set the table info
   */
  if (!is_fes) {
      /* set the pgm_id for that FEM. Pgm_id=0 means not managed FEM. */
      soc_mem_field32_set(unit, fem_input_table, &instruction, FEM_PROGRAMf, ((info->src_arad.is_nop == TRUE) ? 0: info->pgm_id));
  }


  /* Set the Source Type */

  fld_val = 0;
  if (info->src_arad.is_nop == TRUE) {
      /* Set the NOP action,Jericho the acceptable NOP values are 60-63.
       * In Jericho_Plus, NOP values are 127 */
      fld_val |= SOC_DPP_DEFS_GET(unit, fem_key_select_nop);
  } else if (info->src_arad.is_key_src == TRUE) { /* Key source (Direct Extraction) */
      fld_val |= SOC_SAND_SET_BITS_RANGE(info->src_arad.key_tcam_id, ARAD_PMF_FEM_INSTR_KEY_SRC_MSB, ARAD_PMF_FEM_INSTR_KEY_SRC_LSB);
      fld_val |= SOC_SAND_SET_BITS_RANGE(is_src_above_80, ARAD_PMF_FEM_INSTR_KEY_SRC_IS_MSB_BIT, ARAD_PMF_FEM_INSTR_KEY_SRC_IS_MSB_BIT);
      fld_val |= SOC_SAND_SET_BITS_RANGE(((info->src_arad.key_lsb - (is_src_above_80 * ARAD_PMF_FEM_KEY_LSB_FIRST_IN_MSB)) / ARAD_PMF_FEM_KEY_LSB_MULTIPLE), 
                                         ARAD_PMF_FEM_INSTR_KEY_LSB_MSB, ARAD_PMF_FEM_INSTR_KEY_LSB_LSB);
  } else if (SOC_IS_JERICHO_PLUS(unit) && info->src_arad.is_compare) {
          /* FES/FEM compare value - starting Jericho_Plus - is a fixed value. */
          fld_val = ARAD_PMF_FEM_INSTR_FES_FEM_COMPARE_VAL;
  } else { /* TCAM-Direct Table lookup results */
      fld_val |= SOC_SAND_SET_BITS_RANGE(ARAD_PMF_FEM_INSTR_SRC_TYPE_TCAM, ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_MSB, ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_LSB);
      if (SOC_IS_JERICHO(unit)) {
          fld_val |= ( info->src_arad.use_kaps ) ? ARAD_PMF_KEY_SELECT_TCAM_JERICHO_USE_KAPS(info->src_arad.lookup_cycle_id,info->src_arad.key_lsb) : 
               ARAD_PMF_KEY_SELECT_TCAM_JERICHO(info->src_arad.lookup_cycle_id, info->src_arad.key_tcam_id, info->src_arad.key_lsb);
      } else {
          /* Arad encoding */
          fld_val |= SOC_SAND_SET_BITS_RANGE(SOC_SAND_BOOL2NUM(info->src_arad.lookup_cycle_id), ARAD_PMF_FEM_INSTR_TCAM_LOOKUP_ID_BIT, ARAD_PMF_FEM_INSTR_TCAM_LOOKUP_ID_BIT);
          fld_val |= SOC_SAND_SET_BITS_RANGE(info->src_arad.key_tcam_id, ARAD_PMF_FEM_INSTR_TCAM_SRC_MSB, ARAD_PMF_FEM_INSTR_TCAM_SRC_LSB);
          fld_val |= SOC_SAND_SET_BITS_RANGE(info->src_arad.key_lsb, ARAD_PMF_FEM_INSTR_TCAM_32_MSB_RESULT_BIT, ARAD_PMF_FEM_INSTR_TCAM_32_MSB_RESULT_BIT);
      }
  }
  /* tbl_data.key_select = fld_val; */
  soc_mem_field32_set(unit, fem_input_table, &instruction, (is_fes)?FES_KEY_SELECTf:FEM_KEY_SELECTf, fld_val);

  fld_val = 0;
  if (!is_fes) {
      soc_mem_field32_set(unit, fem_input_table, &instruction, REPLACE_LSBf, SOC_SAND_BOOL2NUM(info->is_16_lsb_overridden));
      SOC_SAND_SET_BIT(fld_val, SOC_SAND_BOOL2NUM(info->lsb16_src.lookup_cycle_id), ARAD_PMF_FEM_INSTR_TCAM_LOOKUP_ID_BIT - 1);
      fld_val |= SOC_SAND_SET_BITS_RANGE(info->lsb16_src.key_tcam_id, ARAD_PMF_FEM_INSTR_TCAM_SRC_MSB - 1, ARAD_PMF_FEM_INSTR_TCAM_SRC_LSB - 1);
      soc_mem_field32_set(unit, fem_input_table, &instruction, REPLACE_LSB_SELECTf, fld_val);
  }

  /* Set the instruction back */
  soc_mem_field32_set(unit, fem_input_table, tbl_data, instruction_field, instruction);

  /*
   *    Set the table data
   */
  res = soc_mem_write(   
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
        pmf_pgm_ndx + (group_idx * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fem_input_set_unsafe()", pmf_pgm_ndx, fem_fes_ndx);
}

uint32
  arad_pmf_db_fem_input_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint8                    is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO             *info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_STAGE            
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FEM_INPUT_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(pmf_pgm_ndx, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_OUT_OF_RANGE_ERR, 10, exit);
  if (is_fes == FALSE) {
      SOC_SAND_ERR_IF_ABOVE_MAX(fem_fes_ndx, ARAD_PMF_LOW_LEVEL_FEM_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR, 20, exit);
  }
  else {
      SOC_SAND_ERR_IF_ABOVE_MAX(fem_fes_ndx, ARAD_PMF_LOW_LEVEL_FES_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR, 30, exit);
  }

  res = ARAD_PMF_FEM_INPUT_INFO_verify(unit, info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);

  if (is_fes) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->pgm_id, 0, ARAD_PMF_LOW_LEVEL_PGM_ID_OUT_OF_RANGE_ERR, 40, exit);
  }


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fem_input_set_verify()", pmf_pgm_ndx, fem_fes_ndx);
}

uint32
  arad_pmf_db_fem_input_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint8                    is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx
  )
{
    SOC_PPC_FP_DATABASE_STAGE            
        stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FEM_INPUT_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(pmf_pgm_ndx, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_OUT_OF_RANGE_ERR, 10, exit);
  if (is_fes == FALSE) {
      SOC_SAND_ERR_IF_ABOVE_MAX(fem_fes_ndx, ARAD_PMF_LOW_LEVEL_FEM_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR, 20, exit);
  }
  else {
      SOC_SAND_ERR_IF_ABOVE_MAX(fem_fes_ndx, ARAD_PMF_LOW_LEVEL_FES_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR, 30, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fem_input_get_verify()", pmf_pgm_ndx, fem_fes_ndx);
}

/*********************************************************************
*     Select the input for each Field Extraction MACRO. The FEM
 *     processes the PCL results, and extracts possible actions
 *     to be applied on the packet. INPUT SOC_SAND_IN
 *     SOC_PPD_PMF_LKP_PROFILE *lkp_profile_ndx - Lookup-Profile
 *     information (id and cycle). SOC_SAND_IN uint32 *fem_ndx -
 *     FEM (Field Extraction Macro) Index. Range: 0 - 7.
 *     (Arad-B) SOC_SAND_IN SOC_PPD_PMF_FEM_INPUT_INFO *info - FEM
 *     input parameters: the FEM-Program-Id and the FEM-Input
 *     source. RETURNS OK or Error indicationREMARKS None.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_db_fem_input_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint8                    is_fes,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_OUT ARAD_PMF_FEM_INPUT_INFO             *info
  )
{
    uint32
        is_src_above_80 = 0,
        group_idx =  (is_fes)? ((fem_fes_ndx >= 16)? 0x1:0x0) : ((fem_fes_ndx >= 8)? 0x1:0x0), 
        instruction,
        fem_key_select,
        replace_lsb_select,
      fld_val,
      res = SOC_SAND_OK,
      tbl_data[ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX];
    soc_mem_t
        fem_input_table;
    soc_field_t
        instruction_field;
    SOC_PPC_FP_DATABASE_STAGE            
        stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FEM_INPUT_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PMF_FEM_INPUT_INFO_clear(info);

  res = arad_pmf_db_fem_input_get_verify(
          unit,
          pmf_pgm_ndx,
          is_fes,
          fem_fes_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX * sizeof(uint32));


  /* Get the correct table */
  fem_input_table = (is_fes)? IHB_PMF_FES_PROGRAMm: IHB_PMF_FEM_PROGRAMm;

  /* Read-Modify-Write due to the FES table other params */
  res = soc_mem_read(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (group_idx * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* Get the instruction  */
  instruction_field = (is_fes)? Arad_pmf_fes_instruction_field[fem_fes_ndx]: Arad_pmf_fem_instruction_field[fem_fes_ndx];
  instruction = soc_mem_field32_get(unit, fem_input_table, tbl_data, instruction_field);


  /*
   *    Set the table info
   */
  if (!is_fes) {
      /* tbl_data.program = info->pgm_id; */
      info->pgm_id = soc_mem_field32_get(unit, fem_input_table, &instruction, FEM_PROGRAMf);
  }


  /* Set the Source Type */
  /* tbl_data.key_select = fld_val; */
  fem_key_select = soc_mem_field32_get(unit, fem_input_table, &instruction, (is_fes)?FES_KEY_SELECTf:FEM_KEY_SELECTf);

  fld_val = SOC_SAND_GET_BITS_RANGE(fem_key_select, ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_MSB, ARAD_PMF_FEM_INSTR_SRC_TYPE_INDICATION_LSB);

  if (SOC_SAND_GET_BIT(fem_key_select, 5) == 0) {
      info->src_arad.is_key_src = TRUE;
      info->src_arad.key_tcam_id = SOC_SAND_GET_BITS_RANGE(fem_key_select, ARAD_PMF_FEM_INSTR_KEY_SRC_MSB, ARAD_PMF_FEM_INSTR_KEY_SRC_LSB);
      is_src_above_80 = SOC_SAND_GET_BITS_RANGE(fem_key_select, ARAD_PMF_FEM_INSTR_KEY_SRC_IS_MSB_BIT, ARAD_PMF_FEM_INSTR_KEY_SRC_IS_MSB_BIT);
      info->src_arad.key_lsb = ARAD_PMF_FEM_KEY_LSB_FIRST_IN_MSB * is_src_above_80; 
      info->src_arad.key_lsb += ARAD_PMF_FEM_KEY_LSB_MULTIPLE * 
          (SOC_SAND_GET_BITS_RANGE(fem_key_select, ARAD_PMF_FEM_INSTR_KEY_LSB_MSB, ARAD_PMF_FEM_INSTR_KEY_LSB_LSB));
  }
  
  else if (fld_val == ARAD_PMF_FEM_INSTR_SRC_TYPE_TCAM) {
      info->src_arad.is_key_src = FALSE;
      if (SOC_IS_JERICHO(unit)) {
          info->src_arad.lookup_cycle_id = ARAD_PMF_KEY_SELECT_TCAM_JERICHO_LOOKUP_CYCLE_GET(fem_key_select);
          info->src_arad.key_tcam_id = ARAD_PMF_KEY_SELECT_TCAM_JERICHO_TCAM_LOOKUP_GET(fem_key_select, info->src_arad.lookup_cycle_id);
          info->src_arad.key_lsb = ARAD_PMF_KEY_SELECT_TCAM_JERICHO_KEY_LSB_GET(fem_key_select);
      } else {
          info->src_arad.lookup_cycle_id = SOC_SAND_NUM2BOOL(SOC_SAND_GET_BIT(fem_key_select, ARAD_PMF_FEM_INSTR_TCAM_LOOKUP_ID_BIT));
          info->src_arad.key_tcam_id = SOC_SAND_GET_BITS_RANGE(fem_key_select, ARAD_PMF_FEM_INSTR_TCAM_SRC_MSB, ARAD_PMF_FEM_INSTR_TCAM_SRC_LSB);
          info->src_arad.key_lsb = SOC_SAND_GET_BIT(fem_key_select, ARAD_PMF_FEM_INSTR_TCAM_32_MSB_RESULT_BIT);
      }
  }
  else {
      info->src_arad.is_nop = TRUE;
  }

  if (!is_fes) {
      replace_lsb_select = soc_mem_field32_get(unit, fem_input_table, &instruction, REPLACE_LSB_SELECTf);
      info->is_16_lsb_overridden = SOC_SAND_NUM2BOOL(soc_mem_field32_get(unit, fem_input_table, &instruction, REPLACE_LSBf));
      info->lsb16_src.is_key_src = FALSE;
      info->lsb16_src.lookup_cycle_id = SOC_SAND_GET_BIT(replace_lsb_select, ARAD_PMF_FEM_INSTR_TCAM_LOOKUP_ID_BIT - 1);
      info->lsb16_src.key_tcam_id = SOC_SAND_GET_BITS_RANGE(replace_lsb_select, ARAD_PMF_FEM_INSTR_TCAM_SRC_MSB - 1, ARAD_PMF_FEM_INSTR_TCAM_SRC_LSB - 1);
  }

  


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fem_input_get_unsafe()", 0, fem_fes_ndx);
}

/* Get the user-header sizes in bits */
STATIC uint32
  arad_pmf_db_fes_user_header_single_size_get(
      SOC_SAND_IN  int                  unit,
      SOC_SAND_IN  uint32                  user_header_ndx,
      SOC_SAND_OUT uint32                 *user_header_size
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(user_header_size);

  /* Set the User-Header size as used for Egress PMF */
  *user_header_size = 0;
  switch (soc_property_port_get((unit), user_header_ndx, spn_FIELD_CLASS_ID_SIZE, 0)) {
  case 0:
      *user_header_size = 0;
      break;
  case 8:
      *user_header_size = 8;
      break;
  case 16:
      *user_header_size = 16;
      break;
  case 24:
      *user_header_size = 24;
      break;
  case 32:
      *user_header_size = 32;
      break;
  default:
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d User-Header size not valid.\n\r"), unit));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_user_header_single_size_get()", 0, 0);
}

/* Get the user-header sizes in bits */
uint32
  arad_pmf_db_fes_user_header_sizes_get(
      SOC_SAND_IN  int                  unit,
      SOC_SAND_OUT uint32                 *user_header_0_size,
      SOC_SAND_OUT uint32                 *user_header_1_size,
      SOC_SAND_OUT uint32                 *user_header_egress_pmf_offset_0,
      SOC_SAND_OUT uint32                 *user_header_egress_pmf_offset_1
  )
{
    uint32
        res,
        user_header_ndx,
        user_header_size_lcl[ARAD_PMF_FEM_NOF_USER_HEADER_NDXS];

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(user_header_0_size);
  SOC_SAND_CHECK_NULL_INPUT(user_header_1_size);

  /* Set the User-Header size as used for Egress PMF */
  for (user_header_ndx = 0; user_header_ndx < ARAD_PMF_FEM_NOF_USER_HEADER_NDXS; user_header_ndx++) {
    /* Get the user-header sizes in bits */
      res = arad_pmf_db_fes_user_header_single_size_get(
                unit,
                user_header_ndx,
                &user_header_size_lcl[user_header_ndx]
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }

  *user_header_0_size = user_header_size_lcl[0] + user_header_size_lcl[2];
  *user_header_1_size = user_header_size_lcl[1] + user_header_size_lcl[3];

  /* Verify the consistency */
  if ((*user_header_0_size > 32) || (*user_header_1_size > 32)) {
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d User-Header size not valid.\n\r"), unit));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 20, exit);
  }

  /* Compute the Egress PMF user-header locations */
  *user_header_egress_pmf_offset_0 = 0;
  *user_header_egress_pmf_offset_1 = 0;
  if (user_header_size_lcl[0] == 0) {
      *user_header_egress_pmf_offset_0 = (*user_header_0_size);
      *user_header_egress_pmf_offset_1 = (*user_header_0_size) + 8;
  }
  else if (user_header_size_lcl[0] == 8) {
      *user_header_egress_pmf_offset_0 = 0;
      *user_header_egress_pmf_offset_1 = (*user_header_0_size);
  }
  else /* (user_header_size_lcl[0] == 16) */ {
      *user_header_egress_pmf_offset_0 = 0;
      *user_header_egress_pmf_offset_1 = 8;
  }
  


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_user_header_sizes_get()", 0, 0);
}
/*
 * Search input double-indexed array over first index (up to 'first_index_limit')
 * and over second index (up to 'second_index_limit').
 *
 * If some error is detected then return '-1'. Otherwise, return '0'.
 *
 * If a line with all elements equal to 'last_indicator' is found then
 * return (on '*num_actions_per_table_p') the number of lines until (not
 * including) this line. Otherwise, return 'first_index_limit'
 *
 * Also, while searching, copy the values read from input 'arr' to
 * a sw state element (corresponding to 'runtime' array)
 *
 */
static
  uint32
    arad_pmf_fem_action_get_array_len_and_copy(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32         *arr,
      SOC_SAND_IN  uint32         first_index_limit,
      SOC_SAND_IN  uint32         second_index_limit,
      SOC_SAND_IN  uint32         last_indicator,
      SOC_SAND_OUT uint32         *num_actions_per_table_p
  )
{
    uint32
        err,
        is_last,
        quit,
        ii,
        jj,
        val,
        ret ;
    ret = -1 ;
    quit = 0 ;
    for (ii = 0 ; ii < first_index_limit ; ii++) {
        is_last = 0 ;
        for (jj = 0 ; jj < second_index_limit ; jj++) {
            val = arr[(ii*second_index_limit) + jj] ;
            if (SOC_IS_JERICHO_PLUS(unit)) {
                err = FIELD_ACCESS_QAX_ACTION_RUNTIME.set(unit,ii,jj,val) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else if (SOC_IS_JERICHO(unit)) {
                err = FIELD_ACCESS_JERICHO_ACTION_RUNTIME.set(unit,ii,jj,val) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
                err = FIELD_ACCESS_ARAD_PLUS_ACTION_RUNTIME.set(unit,ii,jj,val) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else {
                err = FIELD_ACCESS_ARAD_ACTION_RUNTIME.set(unit,ii,jj,val) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            }
            if (val == last_indicator) {
                is_last++ ;
                /*
                 * If 'second_index_limit' consecutive 'last_indicator's
                 * are found then this is the element beyond the last meaningful one.
                 */
                if (is_last == second_index_limit) {
                    quit = 1 ;
                    *num_actions_per_table_p = ii ;
                    break ;
                }
            }
        }
        if (quit) {
            break ;
        }
    }
    ret = 0 ;
exit:
    return (ret) ;
}
/*
 * Initialize RUNTIME array of actions
 *
 * Check whether *_pmf_fem_action_type_encoding_runtime has been initialized.
 * If not, initialize it from *_pmf_fem_action_type_encoding (default values)
 * This procedure loads *num_actions_per_table_p by the number of actions
 * This procedure loads *num_elements_per_action_p by the number of elements per each action
 * (range of numbers of first index) on the relevant table.
 * If some error is encountered, then return 'ACTION_INIT_IF_REQUIRED_FAIL'
 * Important:
 *   If this procedure is called before 'field' has been initialized then use
 *   the default tables.
 */
static
  uint32
    arad_pmf_fem_action_init_if_required(
      SOC_SAND_IN  int         unit,
      SOC_SAND_OUT uint32      *num_actions_per_table_p,
      SOC_SAND_OUT uint32      *num_elements_per_action_p
    )
{
    uint32
        ret,
        err,
        num_elements_per_action,
        num_actions_per_table,
        last_indicator ;
    uint8
        field_is_initialized ;
    ret = ACTION_INIT_IF_REQUIRED_FAIL ;
    /*
     * If a whole line in the table is made out of this value
     * then this is the last line (one after the last meanigful line)
     */
    last_indicator = -1 ;
    /*
     * Just good practice: Make sure return values are all zero even
     * in case of error.
     */
    num_elements_per_action = 0 ;
    num_actions_per_table = 0 ;
    *num_elements_per_action_p = num_elements_per_action ;
    *num_actions_per_table_p = num_actions_per_table ;
    err = FIELD_ACCESS.is_allocated(unit,&field_is_initialized) ;
    if (err != SOC_E_NONE) {
        goto exit ;
    }
    if (!field_is_initialized) {
        *num_actions_per_table_p =
            arad_pmf_fem_action_type_array_size_default_get_unsafe(unit) ;
    }

    if (SOC_IS_JERICHO_PLUS(unit)) {
        if (!field_is_initialized) {
            /*
             * 'Field' is not initialized. Use 'default'.
             */
            *num_elements_per_action_p = num_elements_per_action =
                sizeof(Qax_pmf_fem_action_type_encoding[0]) /
                            sizeof(Qax_pmf_fem_action_type_encoding[0][0]) ;
        } else {
            err = FIELD_ACCESS_QAX_NUM_ACTION_RUNTIME.get(unit,&num_actions_per_table) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
            *num_elements_per_action_p = num_elements_per_action =
                SAL_SIZEOF(bcm_dpp_field_info_t,Qax_pmf_fem_action_type_encoding_runtime[0]) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Qax_pmf_fem_action_type_encoding_runtime[0][0]) ;
            if (!num_actions_per_table) {
                num_actions_per_table =
                    SAL_SIZEOF(bcm_dpp_field_info_t,Qax_pmf_fem_action_type_encoding_runtime) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Qax_pmf_fem_action_type_encoding_runtime[0]) ;
                err =
                    arad_pmf_fem_action_get_array_len_and_copy(
                        unit,&Qax_pmf_fem_action_type_encoding[0][0],
                        num_actions_per_table,num_elements_per_action,
                        last_indicator,num_actions_per_table_p
                    ) ;
                err = FIELD_ACCESS_QAX_NUM_ACTION_RUNTIME.set(unit,*num_actions_per_table_p) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else {
                *num_actions_per_table_p = num_actions_per_table ;
            }
        }
    } else if (SOC_IS_JERICHO(unit)) {
        if (!field_is_initialized) {
            /*
             * 'Field' is not initialized. Use 'default'.
             */
            *num_elements_per_action_p = num_elements_per_action =
                sizeof(Jericho_pmf_fem_action_type_encoding[0]) /
                            sizeof(Jericho_pmf_fem_action_type_encoding[0][0]) ;
        } else {
            err = FIELD_ACCESS_JERICHO_NUM_ACTION_RUNTIME.get(unit,&num_actions_per_table) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
            *num_elements_per_action_p = num_elements_per_action =
                SAL_SIZEOF(bcm_dpp_field_info_t,Jericho_pmf_fem_action_type_encoding_runtime[0]) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Jericho_pmf_fem_action_type_encoding_runtime[0][0]) ;
            if (!num_actions_per_table) {
                num_actions_per_table =
                    SAL_SIZEOF(bcm_dpp_field_info_t,Jericho_pmf_fem_action_type_encoding_runtime) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Jericho_pmf_fem_action_type_encoding_runtime[0]) ;
                err =
                    arad_pmf_fem_action_get_array_len_and_copy(
                        unit,&Jericho_pmf_fem_action_type_encoding[0][0],
                        num_actions_per_table,num_elements_per_action,
                        last_indicator,num_actions_per_table_p
                    ) ;
                err = FIELD_ACCESS_JERICHO_NUM_ACTION_RUNTIME.set(unit,*num_actions_per_table_p) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else {
                *num_actions_per_table_p = num_actions_per_table ;
            }
        }
    } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
        if (!field_is_initialized) {
            /*
             * 'Field' is not initialized. Use 'default'.
             */
            *num_elements_per_action_p = num_elements_per_action =
                sizeof(Arad_plus_pmf_fem_action_type_encoding[0]) /
                            sizeof(Arad_plus_pmf_fem_action_type_encoding[0][0]) ;
        } else {
            err = FIELD_ACCESS_ARAD_PLUS_NUM_ACTION_RUNTIME.get(unit,&num_actions_per_table) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
            *num_elements_per_action_p = num_elements_per_action =
                SAL_SIZEOF(bcm_dpp_field_info_t,Arad_plus_pmf_fem_action_type_encoding_runtime[0]) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Arad_plus_pmf_fem_action_type_encoding_runtime[0][0]) ;
            if (!num_actions_per_table) {
                num_actions_per_table =
                    SAL_SIZEOF(bcm_dpp_field_info_t,Arad_plus_pmf_fem_action_type_encoding_runtime) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Arad_plus_pmf_fem_action_type_encoding_runtime[0]) ;
                err =
                    arad_pmf_fem_action_get_array_len_and_copy(
                        unit,&Arad_plus_pmf_fem_action_type_encoding[0][0],
                        num_actions_per_table,num_elements_per_action,
                        last_indicator,num_actions_per_table_p
                    ) ;
                err = FIELD_ACCESS_ARAD_PLUS_NUM_ACTION_RUNTIME.set(unit,*num_actions_per_table_p) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else {
                *num_actions_per_table_p = num_actions_per_table ;
            }
        }
    } else {
        if (!field_is_initialized) {
            /*
             * 'Field' is not initialized. Use 'default'.
             */
            *num_elements_per_action_p = num_elements_per_action =
                sizeof(Arad_pmf_fem_action_type_encoding[0]) /
                            sizeof(Arad_pmf_fem_action_type_encoding[0][0]) ;
        } else {
            err = FIELD_ACCESS_ARAD_NUM_ACTION_RUNTIME.get(unit,&num_actions_per_table) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
            *num_elements_per_action_p = num_elements_per_action =
                SAL_SIZEOF(bcm_dpp_field_info_t,Arad_pmf_fem_action_type_encoding_runtime[0]) /
                                SAL_SIZEOF(bcm_dpp_field_info_t,Arad_pmf_fem_action_type_encoding_runtime[0][0]) ;
            if (!num_actions_per_table) {
                num_actions_per_table =
                    SAL_SIZEOF(bcm_dpp_field_info_t,Arad_pmf_fem_action_type_encoding_runtime) /
                                        SAL_SIZEOF(bcm_dpp_field_info_t,Arad_pmf_fem_action_type_encoding_runtime[0]) ;
                err =
                    arad_pmf_fem_action_get_array_len_and_copy(
                        unit,&Arad_pmf_fem_action_type_encoding[0][0],
                        num_actions_per_table,num_elements_per_action,
                        last_indicator,num_actions_per_table_p
                    ) ;
                err = FIELD_ACCESS_ARAD_NUM_ACTION_RUNTIME.set(unit,*num_actions_per_table_p) ;
                if (err != SOC_E_NONE) {
                    goto exit ;
                }
            } else {
                *num_actions_per_table_p = num_actions_per_table ;
            }
        }
    }
    ret = 0 ;
exit:
    return (ret) ;
}
/*
 * Return the number of actions on the RUNTIME array.
 * If some error is encountered, then return '0'
 * If runtime array is found to not have yet been initialized, then
 * this procedure initializes it with the default values (although
 * this is not required for getting the return value).
 */
uint32
  arad_pmf_fem_action_type_array_size_get_unsafe(
      SOC_SAND_IN  int                            unit
  )
{
    uint32
        ret,
        num_elements_per_action,
        num_actions_per_table ;
    ret = ACTION_INIT_IF_REQUIRED_FAIL ;
    ret = arad_pmf_fem_action_init_if_required(unit,&num_actions_per_table,&num_elements_per_action) ;
    if (ret == (uint32)(ACTION_INIT_IF_REQUIRED_FAIL)) {
        ret = 0 ;
        goto exit ;
    }
    ret = num_actions_per_table ;
exit:
    return (ret) ;
}
/*
 * Search input double-indexed array over first index (up to 'first_index_limit')
 * and over second index (up to 'second_index_limit').
 *
 * If a line with all elements equal to 'last_indicator' is found then
 * return the number of lines until (not including) this line.
 * Otherwise, return 'first_index_limit'
 */
static
  uint32
    arad_pmf_fem_action_get_array_len(
      SOC_SAND_IN  int            unit,
      SOC_SAND_IN  uint32         *arr,
      SOC_SAND_IN  uint32         first_index_limit,
      SOC_SAND_IN  uint32         second_index_limit,
      SOC_SAND_IN  uint32         last_indicator
      
  )
{
    uint32
        is_last,
        quit,
        ii,
        jj,
        val,
        ret ;
    quit = 0 ;
    for (ii = 0 ; ii < first_index_limit ; ii++) {
        is_last = 0 ;
        for (jj = 0 ; jj < second_index_limit ; jj++) {
            val = arr[(ii*second_index_limit) + jj] ;
            if (val == last_indicator) {
                is_last++ ;
                /*
                 * If 'second_index_limit' consecutive 'last_indicator's
                 * are found then this is the element beyond the last meaningful one.
                 */
                if (is_last == second_index_limit) {
                    quit = 1 ;
                    break ;
                }
            }
        }
        if (quit) {
            break ;
        }
    }
    ret = ii ;
    return (ret) ;
}
/*
 * Return the number of actions on the DEFAULT array:
 *   *_pmf_fem_action_type_encoding
 * This number of valid elements should be the same as that
 * number on the RUNTIME array.
 * (See arad_pmf_fem_action_type_array_size_get_unsafe())
 */
uint32
  arad_pmf_fem_action_type_array_size_default_get_unsafe(
      SOC_SAND_IN  int                            unit
  )
{
    uint32
        num_elements_per_action,
        num_actions_per_table,
        last_indicator ;
    /*
     * If a whole line in the table is made out of this value ('last_indicator')
     * then this is the last line (one after the last meanigful line)
     */
    last_indicator = -1 ;
    if (SOC_IS_JERICHO_PLUS(unit)) {
        num_actions_per_table =
            sizeof(Qax_pmf_fem_action_type_encoding) /
                        sizeof(Qax_pmf_fem_action_type_encoding[0]) ;
        num_elements_per_action =
            sizeof(Qax_pmf_fem_action_type_encoding[0]) /
                        sizeof(Qax_pmf_fem_action_type_encoding[0][0]) ;
        num_actions_per_table =
            arad_pmf_fem_action_get_array_len(
                unit,&Qax_pmf_fem_action_type_encoding[0][0],
                num_actions_per_table,num_elements_per_action,last_indicator) ;
    } else if (SOC_IS_JERICHO(unit)) {
        num_actions_per_table =
            sizeof(Jericho_pmf_fem_action_type_encoding) /
                        sizeof(Jericho_pmf_fem_action_type_encoding[0]) ;
        num_elements_per_action =
            sizeof(Jericho_pmf_fem_action_type_encoding[0]) /
                        sizeof(Jericho_pmf_fem_action_type_encoding[0][0]) ;
        num_actions_per_table =
            arad_pmf_fem_action_get_array_len(
                unit,&Jericho_pmf_fem_action_type_encoding[0][0],
                num_actions_per_table,num_elements_per_action,last_indicator) ;
    } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
        num_actions_per_table =
            sizeof(Arad_plus_pmf_fem_action_type_encoding) /
                        sizeof(Arad_plus_pmf_fem_action_type_encoding[0]) ;
        num_elements_per_action =
            sizeof(Arad_plus_pmf_fem_action_type_encoding[0]) /
                        sizeof(Arad_plus_pmf_fem_action_type_encoding[0][0]) ;
        num_actions_per_table =
            arad_pmf_fem_action_get_array_len(
                unit,&Arad_plus_pmf_fem_action_type_encoding[0][0],
                num_actions_per_table,num_elements_per_action,last_indicator) ;
    } else {
        num_actions_per_table =
            sizeof(Arad_pmf_fem_action_type_encoding) /
                        sizeof(Arad_pmf_fem_action_type_encoding[0]) ;
        num_elements_per_action =
            sizeof(Arad_pmf_fem_action_type_encoding[0]) /
                        sizeof(Arad_pmf_fem_action_type_encoding[0][0]) ;
        num_actions_per_table =
            arad_pmf_fem_action_get_array_len(
                unit,&Arad_pmf_fem_action_type_encoding[0][0],
                num_actions_per_table,num_elements_per_action,last_indicator) ;
    }
    return (num_actions_per_table) ;
}
/*
 * Return the number of elements per action on the
 * DEFAULT array:
 *   *_pmf_fem_action_type_encoding
 * This number of elements should be the same as that
 * number on the RUNTIME array.
 * (See arad_pmf_fem_action_type_array_size_get_unsafe())
 */
static
  uint32
    arad_pmf_fem_action_type_num_elements_default_get_unsafe(
      SOC_SAND_IN  int                            unit
    )
{
    uint32
        num_elements_per_action ;
    if (SOC_IS_JERICHO_PLUS(unit)) {
        num_elements_per_action =
            sizeof(Qax_pmf_fem_action_type_encoding[0]) /
                        sizeof(Qax_pmf_fem_action_type_encoding[0][0]) ;
    } else if (SOC_IS_JERICHO(unit)) {
        num_elements_per_action =
            sizeof(Jericho_pmf_fem_action_type_encoding[0]) /
                        sizeof(Jericho_pmf_fem_action_type_encoding[0][0]) ;
    } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
        num_elements_per_action =
            sizeof(Arad_plus_pmf_fem_action_type_encoding[0]) /
                        sizeof(Arad_plus_pmf_fem_action_type_encoding[0][0]) ;
    } else {
        num_elements_per_action =
            sizeof(Arad_pmf_fem_action_type_encoding[0]) /
                        sizeof(Arad_pmf_fem_action_type_encoding[0][0]) ;
    }
    return (num_elements_per_action) ;
}
/*
 * Return the pointer to the relevant action table (per chip) on the
 * DEFAULT array:
 *   *_pmf_fem_action_type_encoding
 * (See arad_pmf_fem_action_type_array_size_get_unsafe())
 */
static
  uint32
    *arad_pmf_fem_action_type_ptr_default_get_unsafe(
      SOC_SAND_IN  int                            unit
    )
{
    uint32
        *ptr_to_table ;
    if (SOC_IS_JERICHO_PLUS(unit)) {
        ptr_to_table = (uint32 *)&(Qax_pmf_fem_action_type_encoding[0][0]) ;
    } else if (SOC_IS_JERICHO(unit)) {
        ptr_to_table = (uint32 *)&(Jericho_pmf_fem_action_type_encoding[0][0]) ;
    } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
        ptr_to_table = (uint32 *)&(Arad_plus_pmf_fem_action_type_encoding[0][0]) ;
    } else {
        ptr_to_table = (uint32 *)&(Arad_pmf_fem_action_type_encoding[0][0]) ;
    }
    return (ptr_to_table) ;
}
/*
 * Return an element from DEFAULT array of actions:
 *   *_pmf_fem_action_type_encoding
 * If some error is encountered, then return '-2'
 *
 */
uint32
  arad_pmf_fem_action_type_array_element_default_get_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                table_line,
      SOC_SAND_IN  uint32                sub_index
  )
{
    uint32
        ret,
        num_elements_per_action,
        num_actions_per_table ;
    uint32
        *ptr_to_table ;
    ret = -2 ;
    num_actions_per_table = arad_pmf_fem_action_type_array_size_default_get_unsafe(unit) ;
    if (table_line >= num_actions_per_table) {
        goto exit ;
    }
    num_elements_per_action = arad_pmf_fem_action_type_num_elements_default_get_unsafe(unit) ;
    if (sub_index >= num_elements_per_action) {
        goto exit ;
    }
    ptr_to_table = arad_pmf_fem_action_type_ptr_default_get_unsafe(unit) ;
    ret = ptr_to_table[(table_line*num_elements_per_action) + sub_index] ;
exit:
    return (ret) ;
}

/*
 * Return an element from RUNTIME array of actions:
 *   *_pmf_fem_action_type_encoding_runtime
 * If some error is encountered, then return 'ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL'
 *
 * If runtime array is found to not have yet been initialized, then
 * this procedure initializes it with the default values.
 *
 * See bcm_petra_field_action_width_get()
 */
uint32
  arad_pmf_fem_action_type_array_element_get_unsafe(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  uint32                table_line,
      SOC_SAND_IN  uint32                sub_index
  )
{
    uint32
        ret,
        err,
        array_element,
        num_elements_per_action,
        num_actions_per_table ;
    uint8
        field_is_initialized ;
    ret = arad_pmf_fem_action_init_if_required(unit,&num_actions_per_table,&num_elements_per_action) ;
    if (ret == (uint32)(ACTION_INIT_IF_REQUIRED_FAIL)) {
        ret = ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL ;
        goto exit ;
    }
    ret = ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL ;
    if (table_line >= num_actions_per_table) {
        sal_printf(
            "%s(): table_line (%d) is too large for size of actions table (%d)\r\n",
            __FUNCTION__,table_line,num_actions_per_table
        ) ;
        goto exit ;
    }
    if (sub_index >= num_elements_per_action) {
        sal_printf(
            "%s(): sub_index (%d) is too large for number of elements on actions table (%d)\r\n",
            __FUNCTION__,sub_index,num_elements_per_action
        ) ;
        goto exit ;
    }
    err = FIELD_ACCESS.is_allocated(unit,&field_is_initialized) ;
    if (err != SOC_E_NONE) {
        goto exit ;
    }
    if (field_is_initialized) {
        if (SOC_IS_JERICHO_PLUS(unit)) {
            err = FIELD_ACCESS_QAX_ACTION_RUNTIME.get(unit,table_line,sub_index,&array_element) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
        } else if (SOC_IS_JERICHO(unit)) {
            err = FIELD_ACCESS_JERICHO_ACTION_RUNTIME.get(unit,table_line,sub_index,&array_element) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
        } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
            err = FIELD_ACCESS_ARAD_PLUS_ACTION_RUNTIME.get(unit,table_line,sub_index,&array_element) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
        } else {
            err = FIELD_ACCESS_ARAD_ACTION_RUNTIME.get(unit,table_line,sub_index,&array_element) ;
            if (err != SOC_E_NONE) {
                goto exit ;
            }
        }
        ret = array_element ;
    } else {
        ret = arad_pmf_fem_action_type_array_element_default_get_unsafe(unit,table_line,sub_index) ;
        if (ret == -2) {
            sal_printf(
                "%s(): arad_pmf_fem_action_type_array_element_default_get_unsafe() has failed.\r\n",
                __FUNCTION__
            ) ;
            goto exit ;
        }
    }
exit:
    return (ret) ;
}
/*
 *   Function
 *      arad_pmf_fem_action_width_default_get
 *   Purpose
 *      Given 'action', get 'width from DEFAULT array of actions).
 *   Parameters
 *      (in)  int unit =
 *              The unit number
 *      (in)  SOC_PPC_FP_ACTION_TYPE in_action_type =
 *              The action for which to get the DEFAULT width. It is
 *              converted to index on corresponding DEFAULT array at
 *              which to get the width. (This index effectively
 *              identified the 'action' for which to update the width
 *              since each line also contains 'action identifier'.)
 *              Note that this is this is the low level action type
 *              and not the higher BCM level action type which is not
 *              directly related to HW.
 *      (out) uint32 *out_action_width =
 *              Loaded by DEFAULT width (no. of bits) assigned to specified
 *              'in_action_type'. If specified action is not found on
 *              the device-matching "Default array" below, the this
 *              procedure returns *out_action_width=INTERNAL_TO_BCM_ACTION_NO_SPACE.
 *              This is not an error!
 *   Returns
 *      int (SOCDNX error)
 *   Notes
 *     No direct hardware changes.
 *
 *     Default arrays (in sw state):
 *       Jericho_pmf_fem_action_type_encoding
 *       Arad_plus_pmf_fem_action_type_encoding
 *       Arad_pmf_fem_action_type_encoding
 *
 *     See:
 *       bcm_petra_field_action_width_set(), INDEX_OF_ACTION_WIDTH,
 *       NUM_ACTION_ELEMENTS, INDEX_OF_ACTION,SOC_PPC_FP_ACTION_TYPE,
 *       bcm_field_action_t
 */
uint32
  arad_pmf_fem_action_width_default_get(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type,
      SOC_SAND_OUT uint32                  *out_action_width
  )
{
    uint32
        found,
        table_line,
        loc_action,
        num_actions_per_table ;

    SOCDNX_INIT_FUNC_DEFS ;
    num_actions_per_table = arad_pmf_fem_action_type_array_size_default_get_unsafe(unit) ;
    found = FALSE ;
    *out_action_width = INTERNAL_TO_BCM_ACTION_NO_SPACE ;
    for (table_line = 0 ; table_line < num_actions_per_table ; table_line++) {
        loc_action = arad_pmf_fem_action_type_array_element_default_get_unsafe(unit,table_line,INDEX_OF_ACTION) ;
        if ((SOC_PPC_FP_ACTION_TYPE)loc_action == in_action_type) {
            found = TRUE ;
            /*
             * At this point, 'table_line' is the corresponding table index
             */
            break ;
        }
    }
    if (!found) {
        LOG_VERBOSE(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "\n\r"
                "Unit %d action %d (%s) num_actions_per_table %d :\r\n"
                "  ==> Could not find action on HW table.\n\r"),
            unit, in_action_type, SOC_PPC_FP_ACTION_TYPE_to_string(in_action_type), num_actions_per_table)
        ) ;
        goto exit ;
    }
    /*
     * Action was found on the DEFAULT table. Its index is 'table_line'.
     * Now get width.
     */
    *out_action_width = arad_pmf_fem_action_type_array_element_default_get_unsafe(unit,table_line,INDEX_OF_ACTION_WIDTH) ;
exit:
    SOCDNX_FUNC_RETURN ;
}
/*
 *   Function
 *      arad_pmf_is_field_initialized
 *   Purpose
 *      Indicate whether 'field' has been initialized on sw state.
 *   Parameters
 *      (in)  int unit =
 *             The unit number
 *      (out) uint8 *field_is_initialized_p -
 *              If 'field' has been initialized, then return *field_is_initialized_p
 *              set to non-zero. Otherwise, set *field_is_initialized_p
 *              to zero.
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *
 *     See:
 *       arad_pmf_fem_action_width_set_unsafe
 */
uint32
  arad_pmf_is_field_initialized(
      SOC_SAND_IN  int   unit,
      SOC_SAND_OUT uint8 *field_is_initialized_p
  )
{
    uint32
        err ;
    uint8
        field_is_initialized ;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
    *field_is_initialized_p = 0 ;
    err = FIELD_ACCESS.is_allocated(unit,&field_is_initialized) ;
    if (err != SOC_E_NONE) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d: Failed to access FIELD.\n\r"),
            unit)
        ) ;
        SOC_SAND_SET_ERROR_CODE(ARAD_TBLS_NOT_INITIALIZED, 10, exit) ;
    }
    *field_is_initialized_p = field_is_initialized ;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_is_field_initialized()",0,0);
}
/*
 *   Function
 *      arad_pmf_fem_action_width_set_unsafe
 *   Purpose
 *      Set the action 'width', in bits, for action on RUNTIME
 *      array of actions on the the line corresponding to specified 'action'.
 *   Parameters
 *      (in)   int unit =
 *             The unit number
 *      (in)  SOC_PPC_FP_ACTION_TYPE in_action_type =
 *              The action for which to set the new width. It is
 *              converted to index on corresponding RUNTIME array at
 *              which to update the width. (This index effectively
 *              identified the 'action' for which to update the width
 *              since each line also contains 'action identifier'.)
 *              Note that this is this is the low level action type
 *              and not the higher BCM level action type which is not
 *              directly related to HW.
 *      (in)  uint32 in_action_width =
 *              New width (no. of bits) to assign to specified 'in_action_type'.
 *              This procedure assumes caller has verified that 'in_action_width'
 *              is NOT larger than the default width assigned to this action.
 *      (out) uint32 *db_identifier_p -
 *              If input action is NOT found on any DB, then return
 *              *db_identifier_p set to NO_DB_FOUND. Otherwise, set *db_identifier_p
 *              to the index of containing DB. Note that this is true even
 *              when return value indicates 'error'.
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     No direct hardware changes.
 *
 *     If whole 'field' is found to not have yet been initialized, then
 *     this procedure returns with an error!
 *
 *     If runtime array is found to not have yet been initialized, then
 *     this procedure initializes it with the default values.
 *
 *     Runtime arrays (in sw state):
 *       Jericho_pmf_fem_action_type_encoding_runtime
 *       Arad_plus_pmf_fem_action_type_encoding_runtime
 *       Arad_pmf_fem_action_type_encoding_runtime
 *
 *     If specified action (on entry 'table_line') is in use on any
 *     currently active data base then this operation is refused
 *     (unless the new width is equal to the one currently being
 *     used). Data base must be destroyed first.
 *
 *     See:
 *       bcm_petra_field_action_width_set(), INDEX_OF_ACTION_WIDTH,
 *       NUM_ACTION_ELEMENTS, INDEX_OF_ACTION,SOC_PPC_FP_ACTION_TYPE,
 *       bcm_field_action_t
 */
uint32
  arad_pmf_fem_action_width_set_unsafe(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type,
      SOC_SAND_IN  uint32                  in_action_width,
      SOC_SAND_OUT uint32                  *db_identifier_p
  )
{
    uint32
        err,
        found,
        table_line,
        loc_action,
        loc_action_width,
        num_elements_per_action,
        num_actions_per_table ;
    uint8
        field_is_initialized ;

    SOCDNX_INIT_FUNC_DEFS ;
    *db_identifier_p = NO_DB_FOUND ;
    /*
     * Field must have been initialized for this procedure to perform properly.
     */
    err = FIELD_ACCESS.is_allocated(unit,&field_is_initialized) ;
    if (err != SOC_E_NONE) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d: Failed to access FIELD.\n\r"),
            unit)
        ) ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    if (!field_is_initialized) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d: FIELD has not yet neen initialized!.\n\r"),
            unit)
        ) ;
        err = SOC_E_INIT ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    err = arad_pmf_fem_action_init_if_required(unit,&num_actions_per_table,&num_elements_per_action) ;
    if (err == (uint32)(ACTION_INIT_IF_REQUIRED_FAIL)) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d: Failed to access runtime table.\n\r"),
            unit)
        ) ;
        err = SOC_E_FAIL ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    found = FALSE ;
    for (table_line = 0 ; table_line < num_actions_per_table ; table_line++) {
        loc_action = arad_pmf_fem_action_type_array_element_get_unsafe(unit,table_line,INDEX_OF_ACTION) ;
        if (loc_action == (uint32)(ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL)) {
            LOG_ERROR(
                BSL_LS_SOC_FP,
                (BSL_META_U(
                    unit,
                    "Unit %d table_line %d : Failed to access runtime table.\n\r"),
                unit, table_line)
            ) ;
            err = SOC_E_FAIL ;
            SOCDNX_IF_ERR_EXIT(err) ;
        }
        if ((SOC_PPC_FP_ACTION_TYPE)loc_action == in_action_type) {
            found = TRUE ;
            /*
             * At this point, 'table_line' is the corresponding table index
             */
            break ;
        }
    }
    if (!found) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d action %d (%s) num_actions_per_table %d : Could not find action on table.\n\r"),
            unit, in_action_type, SOC_PPC_FP_ACTION_TYPE_to_string(in_action_type), num_actions_per_table)
        ) ;
        err = SOC_E_NOT_FOUND ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    /*
     * Action was found on the table (runtime or default). Its index is 'table_line'.
     * Now get current width. If it is equal to requested one then return OK.
     */
    loc_action_width = arad_pmf_fem_action_type_array_element_get_unsafe(unit,table_line,INDEX_OF_ACTION_WIDTH) ;
    if (loc_action_width == (uint32)(ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL)) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d table_line %d : Failed to access runtime table for read.\n\r"),
            unit, table_line)
        ) ;
        err = SOC_E_FAIL ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    if (loc_action_width == in_action_width) {
        SOC_EXIT ;
    }
    {
        /*
         * If input action is in use on any DB then quit.
         */
        err = arad_pp_fp_is_action_on_any_db(unit,in_action_type,db_identifier_p) ;
        if (err != SOC_E_NONE) {
            LOG_ERROR(
                BSL_LS_SOC_FP,
                (BSL_META_U(
                    unit,
                    "Unit %d in_action_type %d (%s) : arad_pp_fp_is_action_on_any_db() reports an error.\n\r"),
                unit, in_action_type, SOC_PPC_FP_ACTION_TYPE_to_string(in_action_type))
            ) ;
            err = SOC_E_FAIL ;
            SOCDNX_IF_ERR_EXIT(err) ;
        }
        if (*db_identifier_p != (NO_DB_FOUND)) {
            LOG_ERROR(
                BSL_LS_SOC_FP,
                (BSL_META_U(
                    unit,
                    "\r\n"
                    " ==> Unit %d in_action_type %d (%s) :\r\n"
                    " ==> This action is in use on DB %d. Destroy this DB first.\n\r"),
                unit, in_action_type, SOC_PPC_FP_ACTION_TYPE_to_string(in_action_type),*db_identifier_p)
            ) ;
            err = SOC_E_BUSY ;
            SOCDNX_IF_ERR_EXIT(err) ;
        }
    }
    /*
     * Make sure requested new value is not higher than default value.
     */
    loc_action_width = arad_pmf_fem_action_type_array_size_default_get_unsafe(unit) ;
    if (in_action_width > loc_action_width) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "\r\n"
                " ==> Unit %d Required width (%d) is higher than allowed (%d) :\r\n"
                " ==> Reject request.\n\r"),
            unit, in_action_width, loc_action_width )
        ) ;
        err = SOC_E_PARAM ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    /*
     * At this point, this 'action_width' is not used by any data base and
     * is within allowed range (0 -> default_value).
     * Update it on RUNTIME table (on 'table_line').
     */
    if (SOC_IS_JERICHO_PLUS(unit)) {
        err = FIELD_ACCESS_QAX_ACTION_RUNTIME.set(unit,table_line,INDEX_OF_ACTION_WIDTH,in_action_width) ;
    } else if (SOC_IS_JERICHO(unit)) {
        err = FIELD_ACCESS_JERICHO_ACTION_RUNTIME.set(unit,table_line,INDEX_OF_ACTION_WIDTH,in_action_width) ;
    } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
        err = FIELD_ACCESS_ARAD_PLUS_ACTION_RUNTIME.set(unit,table_line,INDEX_OF_ACTION_WIDTH,in_action_width) ;
    } else {
        err = FIELD_ACCESS_ARAD_ACTION_RUNTIME.set(unit,table_line,INDEX_OF_ACTION_WIDTH,in_action_width) ;
    }
    if (err != SOC_E_NONE) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d table_line %d : Failed to access runtime table for write.\n\r"),
            unit, table_line)
        ) ;
        err = SOC_E_FAIL ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
exit:
    SOCDNX_FUNC_RETURN ;
}
/*
 *   Function
 *      arad_pmf_fem_action_width_get_unsafe
 *   Purpose
 *      Get the action 'width', in bits, for action on RUNTIME
 *      array of actions on the the line corresponding to specified 'action'.
 *      Also get the 'hardware id' of that specified action.
 *   Parameters
 *      (in) int unit =
 *             The unit number
 *      (in) SOC_PPC_FP_ACTION_TYPE in_action_type =
 *             The action for which to get the width. It is
 *             converted to index on corresponding RUNTIME array at
 *             which to update the width. (This index effectively
 *             identified the 'action' for which to update the width
 *             since each line also contains 'action identifier'.)
 *             Note that this is this is the low level action type
 *             and not the higher BCM level action type which is not
 *             directly related to HW.
 *      (out)uint32 *out_action_width =
 *             Loaded by width (no. of bits) assigned to specified 'in_action_type'.
 *             If 'in_action_type' is not found on the device-matching
 *             runtime array (e.g., Jericho_pmf_fem_action_type_encoding_runtime)
 *             then this procedure returns *out_action_width=NOT_ON_RUNTIME_ARRAY.
 *             This is not an error!
 *      (out)uint32 *out_hw_id =
 *             Loaded by HW identifier assigned to specified 'in_action_type'.
 *             If 'in_action_type' is not found on the device-matching
 *             runtime array (e.g., Jericho_pmf_fem_action_type_encoding_runtime)
 *             then this procedure returns *out_hw_id=NOT_ON_RUNTIME_ARRAY.
 *             This is not an error!
 *   Returns
 *      int (SOCDNX errors)
 *   Notes
 *     No direct hardware changes.
 *
 *     If runtime array is found to not have yet been initialized, then
 *     this procedure initializes it with the default values.
 *
 *     Runtime arrays (in sw state):
 *       Jericho_pmf_fem_action_type_encoding_runtime
 *       Arad_plus_pmf_fem_action_type_encoding_runtime
 *       Arad_pmf_fem_action_type_encoding_runtime
 *
 *     See:
 *       bcm_petra_field_action_width_set(), INDEX_OF_ACTION_WIDTH,
 *       NUM_ACTION_ELEMENTS, INDEX_OF_ACTION,SOC_PPC_FP_ACTION_TYPE,
 *       bcm_field_action_t
 */
uint32
  arad_pmf_fem_action_width_get_unsafe(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type,
      SOC_SAND_OUT uint32                  *out_action_width,
      SOC_SAND_OUT uint32                  *out_hw_id
  )
{
    uint32
        err,
        found,
        table_line,
        loc_action,
        loc_action_width,
        loc_hw_id,
        num_elements_per_action,
        num_actions_per_table ;

    SOCDNX_INIT_FUNC_DEFS ;
    *out_action_width = NOT_ON_RUNTIME_ARRAY ;
    *out_hw_id = NOT_ON_RUNTIME_ARRAY ;
    err = arad_pmf_fem_action_init_if_required(unit,&num_actions_per_table,&num_elements_per_action) ;
    if (err == (uint32)(ACTION_INIT_IF_REQUIRED_FAIL)) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d: Failed to access runtime table.\n\r"),
            unit)
        ) ;
        err = SOC_E_FAIL ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    found = FALSE ;
    for (table_line = 0 ; table_line < num_actions_per_table ; table_line++) {
        loc_action = arad_pmf_fem_action_type_array_element_get_unsafe(unit,table_line,INDEX_OF_ACTION) ;
        if (loc_action == (uint32)(ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL)) {
            LOG_ERROR(
                BSL_LS_SOC_FP,
                (BSL_META_U(
                    unit,
                    "Unit %d table_line %d : Failed to access runtime table.\n\r"),
                unit, table_line)
            ) ;
            err = SOC_E_FAIL ;
            SOCDNX_IF_ERR_EXIT(err) ;
        }
        if ((SOC_PPC_FP_ACTION_TYPE)loc_action == in_action_type) {
            found = TRUE ;
            /*
             * At this point, 'table_line' is the corresponding table index
             */
            break ;
        }
    }
    if (!found) {
        LOG_VERBOSE(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "\n\r"
                "Unit %d action %d (%s) num_actions_per_table %d :\r\n"
                " ==> Could not find action on table.\n\r"),
            unit, in_action_type, SOC_PPC_FP_ACTION_TYPE_to_string(in_action_type), num_actions_per_table)
        ) ;
        goto exit ;
    }
    /*
     * Action was found on the table (runtime or default). Its index is 'table_line'.
     */
    /*
     * Now get current width.
     */
    loc_action_width = arad_pmf_fem_action_type_array_element_get_unsafe(unit,table_line,INDEX_OF_ACTION_WIDTH) ;
    if (loc_action_width == (uint32)(ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL)) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d table_line %d : Failed to access runtime table for read.\n\r"),
            unit, table_line)
        ) ;
        err = SOC_E_FAIL ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    *out_action_width = loc_action_width ;
    /*
     * Now get current HW identifier.
     */
    loc_hw_id = arad_pmf_fem_action_type_array_element_get_unsafe(unit,table_line,INDEX_OF_HW_ID) ;
    if (loc_hw_id == (uint32)(ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL)) {
        LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(
                unit,
                "Unit %d table_line %d : Failed to access runtime table for read.\n\r"),
            unit, table_line)
        ) ;
        err = SOC_E_FAIL ;
        SOCDNX_IF_ERR_EXIT(err) ;
    }
    *out_hw_id = loc_hw_id ;
exit:
    SOCDNX_FUNC_RETURN ;
}
uint32
    arad_pmf_fem_action_elk_result_size_update(int unit, int res_sizes[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS])
{
    int i;
    int total_size = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS+ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_SIZE_IN_BITS;

    SOCDNX_INIT_FUNC_DEFS;

    for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {

        total_size -= res_sizes[i];
        Arad_pmf_fem_action_ingress_flp_signal[i].lsb_hw = total_size;
        Arad_pmf_fem_action_ingress_flp_signal[i].size = res_sizes[i];

        Arad_plus_pmf_fem_action_ingress_flp_signal[i].lsb_hw = total_size;
        Arad_plus_pmf_fem_action_ingress_flp_signal[i].size = res_sizes[i];
    }    

    SOCDNX_FUNC_RETURN; 
}

uint32
  arad_egress_pmf_db_action_get_unsafe(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  action_type,
      SOC_SAND_OUT uint32                 *action_lsb,
      SOC_SAND_OUT uint32                 *action_size,
      SOC_SAND_OUT int                    *action_valid
  )
{
    uint8 found = FALSE;
    ARAD_PMF_FEM_ACTION_EGRESS_INFO *action_data;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(action_size);

    if (SOC_IS_JERICHO(unit))
        action_data = &Jericho_pmf_fem_action_egress_pmf_info[0];
    else
        action_data = &Arad_pmf_fem_action_egress_pmf_info[0];

    while(action_data->action_type != SOC_PPC_NOF_FP_ACTION_TYPES) {
       if(action_data->action_type == action_type) {
          *action_lsb   = action_data->lsb_hw;
          *action_size  = action_data->size;
          *action_valid = action_data->valid;
          found = TRUE;
          break;
       }
       action_data++;
    }

    if (!found) {
        LOG_ERROR(BSL_LS_SOC_FP, (BSL_META_U(unit, "Unit %d Egress Action type %s : Failed to get\n\r"),
                                                           unit, SOC_PPC_FP_ACTION_TYPE_to_string(action_type)));
        SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 40, exit);/* action type is not supported in ARAD */
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_action_size_get_unsafe()",action_type,0);
}

uint32
  arad_pmf_db_fes_action_size_get_unsafe(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE     action_type,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_OUT uint32                 *action_size,
      SOC_SAND_OUT uint32                 *action_lsb_egress
  )
{
    uint8
        found = FALSE,
        fp_key_changed_size;
    uint32
        user_header_0_size,
        user_header_1_size,
        user_header_egress_pmf_offset_0,
        user_header_egress_pmf_offset_1,
        table_nof_lines,
        res,
        table_line;
    SOC_PPC_FP_ACTION_TYPE
        action_type_line;
	int do_not_update_in_billing_mode;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(action_size);

  found = FALSE;
  *action_size = 0;
  *action_lsb_egress = 0;
  switch (stage) {
  case SOC_PPC_FP_DATABASE_STAGE_EGRESS:
      table_nof_lines = SOC_DPP_DEFS_GET(unit, nof_egress_pmf_actions);
      break;
  case SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP:
      table_nof_lines = SOC_PPC_FP_NOF_ELK_ACTIONS;
      break;
  case SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB:
      table_nof_lines = SOC_PPC_FP_NOF_INGRESS_SLB_ACTIONS;
      break;
  case SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF:
      table_nof_lines = arad_pmf_fem_action_type_array_size_get_unsafe(unit);
      break;
  case SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT:
  case SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT:
  default:
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 10, exit);
  }
  for (table_line = 0; (table_line < table_nof_lines) && (!found); ++table_line) {
      switch (stage) {
      case SOC_PPC_FP_DATABASE_STAGE_EGRESS:
          if (SOC_IS_JERICHO(unit)) {
              action_type_line = Jericho_pmf_fem_action_egress_pmf_signal[table_line].action_type;
          } else {
              action_type_line = Arad_pmf_fem_action_egress_pmf_signal[table_line].action_type;
          }
          break;
      case SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP:
          if (SOC_IS_ARADPLUS(unit) && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_rpf_fwd_parallel", 0) == 0)){
              action_type_line = Arad_plus_pmf_fem_action_ingress_flp_signal[table_line].action_type;
          } else {
              action_type_line = Arad_pmf_fem_action_ingress_flp_signal[table_line].action_type;
          }
          break;
#ifdef BCM_88660_A0
      case SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB:
          if (SOC_IS_ARADPLUS(unit)) {
              action_type_line = Arad_pmf_fem_action_ingress_slb_signal[table_line].action_type;
          }
          else {
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 20, exit);
          }
          break;
#endif /* BCM_88660_A0 */
      case SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF:
          action_type_line = arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION);
          if (action_type_line == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 25, exit);
          }
          break;
      case SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT:
      case SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT:
      default:
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 20, exit);
      }
      if (action_type == action_type_line) {
          found = TRUE;
          switch (stage) {
          case SOC_PPC_FP_DATABASE_STAGE_EGRESS:
              if (SOC_IS_JERICHO(unit)) {
                  *action_size = Jericho_pmf_fem_action_egress_pmf_signal[table_line].size;
                  *action_lsb_egress = Jericho_pmf_fem_action_egress_pmf_signal[table_line].lsb_hw;
              } else {
                  *action_size = Arad_pmf_fem_action_egress_pmf_signal[table_line].size;
                  *action_lsb_egress = Arad_pmf_fem_action_egress_pmf_signal[table_line].lsb_hw;
              }
              break;
          case SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP:
              if (SOC_IS_ARADPLUS(unit) && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_rpf_fwd_parallel", 0) == 0)){
                  *action_size = Arad_plus_pmf_fem_action_ingress_flp_signal[table_line].size;
                  *action_lsb_egress = Arad_plus_pmf_fem_action_ingress_flp_signal[table_line].lsb_hw;
              } else {
                  *action_size = Arad_pmf_fem_action_ingress_flp_signal[table_line].size;
                  *action_lsb_egress = Arad_pmf_fem_action_ingress_flp_signal[table_line].lsb_hw;
              }
              break;
#ifdef BCM_88660_A0
          case SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB:
              if (SOC_IS_ARADPLUS(unit)) {
                  *action_size = Arad_pmf_fem_action_ingress_slb_signal[table_line].size;
                  *action_lsb_egress = Arad_pmf_fem_action_ingress_slb_signal[table_line].lsb_hw;
              }
              else {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 20, exit);
              }
              break;
#endif /* BCM_88660_A0 */
          case SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF:
              *action_size = arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION_WIDTH);
              if (*action_size == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 25, exit);
              }
              break;
          /* must default. Otherwise, compilation error*/
          /* coverity[dead_error_begin:FALSE] */
          case SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT:
          case SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT:
          default:
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 30, exit);
          }
          /* Exit the for loop */
          break;
      }
  }

  do_not_update_in_billing_mode = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "do_not_update_action_size_in_billing_mode", 0);

  /* 
   * Particular cases: 
   * 1. the counter action has a regular size of 16 bits (for the Counter Processor), 
   * and will have a size of 22 bits only if the Statistic-Interface is set in the Billing mode 
   * 2. the user-headers have a size set by SOC property 
   * 3. SOC property custom_feature_reduced_trap_action=1. Indiactes not to use the 16 bits 
   * trap qualifier. Only use trap code (8 bits) and trap strength (3 bits).
   */
  if (found 
	  && (!do_not_update_in_billing_mode)
      && ((action_type == SOC_PPC_FP_ACTION_TYPE_COUNTER_A) || (action_type == SOC_PPC_FP_ACTION_TYPE_COUNTER_B))
      && (SOC_DPP_CONFIG(unit)->arad->init.stat_if.stat_if_enable)
      && ((SOC_DPP_CONFIG(unit)->arad->init.stat_if.stat_if_info[0].mode == SOC_TMC_STAT_IF_REPORT_MODE_BILLING)
          || (SOC_DPP_CONFIG(unit)->arad->init.stat_if.stat_if_info[0].mode == SOC_TMC_STAT_IF_REPORT_MODE_BILLING_QUEUE_NUMBER))) {
      *action_size = 22; /* Instead of 17 */ 
  }
  else if (found && ((action_type == SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1) || (action_type == SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2))) {
      res = arad_pmf_db_fes_user_header_sizes_get(
                unit,
                &user_header_0_size,
                &user_header_1_size,
                &user_header_egress_pmf_offset_0,
                &user_header_egress_pmf_offset_1
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      /*In case of ITMH programmable mode, if action is user_header, at least 18 bytes should be taken for class set as action argument.*/
      *action_size = (action_type == SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1)? 
         ( (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) || (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0)))
          ? SOC_SAND_MAX(18,user_header_0_size): user_header_0_size ) : 
         ( (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) || (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0))) 
         ? SOC_SAND_MAX(18,user_header_1_size): user_header_1_size ) ; 
  }
  else if(found
          && (action_type == SOC_PPC_FP_ACTION_TYPE_TRAP)
          && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "reduced_trap_action", 0))) {
      *action_size = 11;
  }
  else if (found  && action_type == SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED) {
            *action_size = 11;
  } 
  else if(found
          && (action_type == SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY)
          && (!(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "increased_cascaded_action", 0)))) {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.key_change_size.get(unit, stage, &fp_key_changed_size);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      *action_size = 4 /* select (2b) and operation (2b) fields mandatory */ + fp_key_changed_size;
  }
  else if (found
           && (action_type == SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39)
           && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "reduced_learn_data_action", 0))) {
      *action_size = 8;
  }

  if (!found) {
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Stage %s Action type %s : Failed to get action size. No encoding found.\n\r"),
                 unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), SOC_PPC_FP_ACTION_TYPE_to_string(action_type)));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 40, exit);/* action type is not supported in ARAD */
  }

  /* Do not allow action size 0 (e.g. user header) - return -1 instead */
 if (*action_size == 0) {
     *action_size = SOC_SAND_U32_MAX;
 }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_action_size_get_unsafe()",action_type,0);
}

/* Set the FES attributes except the input */
uint32
  arad_pmf_db_fes_set_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FES_INPUT_INFO   *info
  )
{
  uint32
      instruction,
      table_line,
    fld_val = 0,
    res = SOC_SAND_OK,
    tbl_data[ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX];
  soc_mem_t
      fem_input_table;
  soc_field_t
      instruction_field;
  uint8
      is_fes_above_16 = (fem_fes_ndx >= 16)? 0x1:0x0,
      found;
  SOC_PPC_FP_DATABASE_STAGE            
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FES_SET_UNSAFE);
  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "     "
                        "Action: FES set, "
                        "FES/FEM-ID: %d, "
                        "PMF-Program: %d, "
                        "     "
                        "FES info: "
                        "Action_type: %s, "
                        "Always_valid: %d, "
                        "Shift: %d\n\r"),
             fem_fes_ndx, pmf_pgm_ndx, SOC_PPC_FP_ACTION_TYPE_to_string(info->action_type), info->is_action_always_valid, info->shift));

  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX * sizeof(uint32));

  SOC_SAND_CHECK_NULL_INPUT(info);

  res = arad_pmf_db_fes_set_verify(
          unit,
          pmf_pgm_ndx,
          fem_fes_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /* Get the correct table */
  fem_input_table = IHB_PMF_FES_PROGRAMm;

  /* Read-Modify-Write due to the FES table other params */
  res = soc_mem_read(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (is_fes_above_16 * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* Get the instruction  */
  instruction_field = Arad_pmf_fes_instruction_field[fem_fes_ndx];
  instruction = soc_mem_field32_get(unit, fem_input_table, tbl_data, instruction_field);


  /* Set the Shift */
  soc_mem_field32_set(unit, fem_input_table, &instruction, FES_SHIFTf, info->shift);

  /* Set the Action type */
  found = FALSE;
  for (table_line = 0; (table_line < arad_pmf_fem_action_type_array_size_get_unsafe(unit)) && (!found); ++table_line) {
      if (info->action_type == arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION)) {
          if (info->action_type == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          fld_val = arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_HW_ID);
          if (fld_val == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          found = TRUE;
      }
  }
  if (!found) {
      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Fes index %d Action type %s : Failed to set fes. No encoding found.\n\r"),
                 unit, fem_fes_ndx, SOC_PPC_FP_ACTION_TYPE_to_string(info->action_type)));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_SUB_HEADER_OUT_OF_RANGE_ERR, 25, exit);
  }
  soc_mem_field32_set(unit, fem_input_table, &instruction, FES_ACTION_TYPEf, fld_val);

  if (SOC_IS_JERICHO_PLUS(unit)) {
      soc_mem_field32_set(unit, fem_input_table, &instruction, FES_VALID_BITSf, info->valid_bits);
  }

  /* Set the FES type */
  soc_mem_field32_set(unit, fem_input_table, &instruction, FES_TYPEf, SOC_SAND_BOOL2NUM(info->is_action_always_valid));

#ifdef BCM_88660_A0
  /* In Arad+, new option of LSB valid polarity always set to have same behaviour as Arad */
  if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
      fld_val = 0x1;
      soc_mem_field32_set(unit, fem_input_table, &instruction, FES_LSB_VALID_POLARITYf, fld_val);
  }
#endif /* BCM_88660_A0 */

  /* Set the instruction back */
  soc_mem_field32_set(unit, fem_input_table, tbl_data, instruction_field, instruction);

  /*
   *    Set the table data
   */
  res = soc_mem_write(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (is_fes_above_16 * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_set_unsafe()", pmf_pgm_ndx, fem_fes_ndx);
}

uint32
  arad_pmf_db_fes_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FES_INPUT_INFO             *info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_STAGE            
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FES_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(pmf_pgm_ndx, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_fes_ndx, ARAD_PMF_LOW_LEVEL_FES_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR, 20, exit);
  res = ARAD_PMF_FES_INPUT_INFO_verify(unit, info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_set_verify()", pmf_pgm_ndx, fem_fes_ndx);
}

uint32
  arad_pmf_db_fes_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx
  )
{
    SOC_PPC_FP_DATABASE_STAGE            
        stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FES_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(pmf_pgm_ndx, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_fes_ndx, ARAD_PMF_LOW_LEVEL_FES_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_NDX_OUT_OF_RANGE_ERR, 30, exit);


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_get_verify()", pmf_pgm_ndx, fem_fes_ndx);
}

/* Retrieve the SW action from the HW value */
uint32
  arad_pmf_db_action_type_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 action_type_hw,
    SOC_SAND_OUT  uint8                    *is_found,
    SOC_SAND_OUT SOC_PPC_FP_ACTION_TYPE *action_type_sw
  )
{
  uint32
      table_line = 0,
    fld_val;
  uint8
      found = FALSE;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(is_found);
  SOC_SAND_CHECK_NULL_INPUT(action_type_sw);

  /* Get the Action type */
  fld_val = action_type_hw;
  found = FALSE;
  for (table_line = 0; (table_line < arad_pmf_fem_action_type_array_size_get_unsafe(unit)) && (!found); ++table_line) {
      if (fld_val == arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_HW_ID)) {
          if (fld_val == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_FEM_ID_OUT_OF_RANGE_ERR, 10, exit);
          }
          *action_type_sw = arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION);
          if (*action_type_sw == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_FEM_ID_OUT_OF_RANGE_ERR, 20, exit);
          }
          found = TRUE;
      }
  }

  *is_found = found;
  if (!found) {
      /* If not found, the number is more than number of actions and does nothing */
      *action_type_sw = SOC_PPC_FP_ACTION_TYPE_NOP;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_action_type_get_unsafe()", action_type_hw, table_line);
}

uint32
  arad_pmf_db_fes_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  uint32                    pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  fem_fes_ndx,
    SOC_SAND_OUT ARAD_PMF_FES_INPUT_INFO             *info
  )
{
  uint32
      instruction,
    fld_val,
    res = SOC_SAND_OK,
    tbl_data[ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX];
  soc_mem_t
      fem_input_table;
  soc_field_t
      instruction_field;
  uint8
      is_fes_above_16 = (fem_fes_ndx >= 16)? 0x1:0x0,
      found;
  SOC_PPC_FP_DATABASE_STAGE            
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FES_GET_UNSAFE);

  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX * sizeof(uint32));

  SOC_SAND_CHECK_NULL_INPUT(info);

  res = arad_pmf_db_fes_get_verify(
          unit,
          pmf_pgm_ndx,
          fem_fes_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  ARAD_PMF_FES_INPUT_INFO_clear(info);

  /* Get the correct table */
  fem_input_table = IHB_PMF_FES_PROGRAMm;

  /* Read-Modify-Write due to the FES table other params */
  res = soc_mem_read(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (is_fes_above_16 * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* Get the instruction  */
  instruction_field = Arad_pmf_fes_instruction_field[fem_fes_ndx];
  instruction = soc_mem_field32_get(unit, fem_input_table, tbl_data, instruction_field);


  /* Get the Shift */
  info->shift = soc_mem_field32_get(unit, fem_input_table, &instruction, FES_SHIFTf);

  /* Get the Action type */
  fld_val = soc_mem_field32_get(unit, fem_input_table, &instruction, FES_ACTION_TYPEf);
  res = arad_pmf_db_action_type_get_unsafe(
            unit,
            fld_val, /* action_type_hw */
            &found,
            &(info->action_type)
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 59, exit);
  if (SOC_IS_JERICHO_PLUS(unit)) {
      info->valid_bits = soc_mem_field32_get(unit, fem_input_table, &instruction, FES_VALID_BITSf);
  }

  /* Get the FES type */
  info->is_action_always_valid = SOC_SAND_NUM2BOOL(soc_mem_field32_get(unit, fem_input_table, &instruction, FES_TYPEf));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fem_input_get_unsafe()", pmf_pgm_ndx, fem_fes_ndx);
}


/* move FES info from FES x to FES y */
uint32
  arad_pmf_db_fes_move_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32			         pmf_pgm_ndx,
    SOC_SAND_IN  uint32                  from_fem_fes_ndx,
    SOC_SAND_IN  uint32                  to_fem_fes_ndx,
    SOC_SAND_IN  ARAD_PMF_FES            *fes_info
  )
{
  uint32
      group_idx_from,
      group_idx_to,
      instruction,
    res = SOC_SAND_OK,
      tbl_data_from[ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX],
    tbl_data_to[ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX];
  soc_mem_t
      fem_input_table;
  soc_field_t
      instruction_field;
  SOC_PPC_FP_DATABASE_STAGE            
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_DB_FES_MOVE_UNSAFE);

  sal_memset(tbl_data_from, 0x0, ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX * sizeof(uint32));
  sal_memset(tbl_data_to, 0x0, ARAD_PMF_FEM_INPUT_TABLE_DATA_LENGTH_MAX * sizeof(uint32));

  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "    "
                        "Move FES-id:%d to FES-id:%d\n\r"), from_fem_fes_ndx, to_fem_fes_ndx));

  /* Get the correct table */
  fem_input_table = IHB_PMF_FES_PROGRAMm;

  group_idx_from =  (from_fem_fes_ndx >= 16)? 0x1:0x0; 
  group_idx_to =  (to_fem_fes_ndx >= 16)? 0x1:0x0; 

  /* Read-Modify-Write due to the FES table other params */
  res = soc_mem_read(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (group_idx_from * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data_from
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
  res = soc_mem_read(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (group_idx_to * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data_to
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* Get the instruction  */
  instruction_field = Arad_pmf_fes_instruction_field[from_fem_fes_ndx];
  instruction = soc_mem_field32_get(unit, fem_input_table, tbl_data_from, instruction_field);

  /* new field */
  instruction_field = Arad_pmf_fes_instruction_field[to_fem_fes_ndx];
  /* Set the instruction back */
  soc_mem_field32_set(unit, fem_input_table, tbl_data_to, instruction_field, instruction);

  /*
   * Set the table data
   */
  res = soc_mem_write(
          unit,
          fem_input_table,
          MEM_BLOCK_ANY,
          pmf_pgm_ndx + (group_idx_to * (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1)),
          tbl_data_to
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.set(
              unit,
              stage,
              pmf_pgm_ndx,
              to_fem_fes_ndx,
              fes_info
            );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_db_fes_move_unsafe()", pmf_pgm_ndx, from_fem_fes_ndx);
}

/*********************************************************************
*     Set the location of 4 bits from the FEM-key that select
 *     the performed action format for this key (configure the
 *     Select-4-bits table).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_fem_select_bits_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO     *info
  )
{
  uint32
    tbl_ndx,
    res = SOC_SAND_OK,
    tbl_data = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_SELECT_BITS_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(fem_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "     "
                        "Action: FEM select bits set, "
                        "FEM-ID: %d, "
                        "FEM-Program: %d, "
                        "     "
                        "FES info: "
                        "Select MSB: %d\n\r"),
             fem_ndx->id, fem_pgm_ndx, info->sel_bit_msb));

  res = arad_pmf_fem_select_bits_set_verify(
          unit,
          fem_ndx,
          fem_pgm_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *    Select the MSB of the four bits
   */
  tbl_ndx = fem_ndx->id;
  soc_mem_field32_set(unit, IHB_FEM_BIT_SELECTm, &tbl_data, BIT_SELECTf, info->sel_bit_msb);
  res = soc_mem_array_write(
          unit,
          IHB_FEM_BIT_SELECTm,
          tbl_ndx,
          MEM_BLOCK_ANY,
          fem_pgm_ndx,
          &tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_select_bits_set_unsafe()", 0, fem_pgm_ndx);
}

uint32
  arad_pmf_fem_select_bits_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO     *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_SELECT_BITS_SET_VERIFY);

  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_NDX, fem_ndx, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_pgm_ndx, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_OUT_OF_RANGE_ERR, 20, exit);
  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_SELECTED_BITS_INFO, info, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_select_bits_set_verify()", 0, fem_pgm_ndx);
}

uint32
  arad_pmf_fem_select_bits_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_SELECT_BITS_GET_VERIFY);

  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_NDX, fem_ndx, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_pgm_ndx, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_select_bits_get_verify()", 0, fem_pgm_ndx);
}

/*********************************************************************
*     Set the location of 4 bits from the FEM-key that select
 *     the performed action format for this key (configure the
 *     Select-4-bits table).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_fem_select_bits_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_OUT ARAD_PMF_FEM_SELECTED_BITS_INFO     *info
  )
{
  uint32
    tbl_ndx,
    res = SOC_SAND_OK,
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_SELECT_BITS_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(fem_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PMF_FEM_SELECTED_BITS_INFO_clear(info);

  res = arad_pmf_fem_select_bits_get_verify(
          unit,
          fem_ndx,
          fem_pgm_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  tbl_ndx = fem_ndx->id;
  res = soc_mem_array_read(
          unit,
          IHB_FEM_BIT_SELECTm,
          tbl_ndx,
          MEM_BLOCK_ANY,
          fem_pgm_ndx,
          &tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);


  info->sel_bit_msb = soc_mem_field32_get(unit, IHB_FEM_BIT_SELECTm, &tbl_data, BIT_SELECTf);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_select_bits_get_unsafe()", 0, fem_pgm_ndx);
}

/*********************************************************************
*     Set the location of 4 bits from the FEM-key that select
 *     the performed action format for this key (configure the
 *     Select-4-bits table).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_fem_action_format_map_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  )
{
  uint32
    tbl_ndx,
    res = SOC_SAND_OK,
    entry_offset,
    tbl_data = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_MAP_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(fem_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "     "
                        "Action: FEM action format map set, "
                        "FEM-ID: %d, "
                        "FEM-Program: %d, "
                        "Selected-bit: %d, "
                        "     "
                        "FEM info: "
                        "Is-Action-Valid: %d, "
                        "Map-Data: %d, "
                        "Action format ID: %d\n\r"),
             fem_ndx->id, fem_pgm_ndx, selected_bits_ndx, 
             info->is_action_valid, info->map_data, info->action_fomat_id));

  res = arad_pmf_fem_action_format_map_set_verify(
          unit,
          fem_ndx,
          fem_pgm_ndx,
          selected_bits_ndx,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *    Set the entry offset and write to the table
   */
  entry_offset = selected_bits_ndx + (fem_pgm_ndx << ARAD_PMF_FEM_SEL_BITS_SIZE_IN_BITS);
  tbl_ndx = fem_ndx->id;
  soc_mem_field32_set(unit, IHB_FEM_MAP_INDEX_TABLEm, &tbl_data, MAP_INDEXf, info->action_fomat_id);
  soc_mem_field32_set(unit, IHB_FEM_MAP_INDEX_TABLEm, &tbl_data, MAP_DATAf, info->map_data);
  soc_mem_field32_set(unit, IHB_FEM_MAP_INDEX_TABLEm, &tbl_data, VALIDf, SOC_SAND_BOOL2NUM(info->is_action_valid));
  res = soc_mem_array_write(
          unit,
          IHB_FEM_MAP_INDEX_TABLEm,
          tbl_ndx,
          MEM_BLOCK_ANY,
          entry_offset,
          &tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_map_set_unsafe()", 0, fem_pgm_ndx);
}

uint32
  arad_pmf_fem_action_format_map_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_MAP_SET_VERIFY);

  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_NDX, fem_ndx, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_pgm_ndx, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(selected_bits_ndx, ARAD_PMF_LOW_LEVEL_SELECTED_BITS_NDX_MAX, ARAD_PMF_LOW_LEVEL_SELECTED_BITS_NDX_OUT_OF_RANGE_ERR, 30, exit);
  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO, info, 40, exit);

exit:  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_map_set_verify()", 0, fem_pgm_ndx);
}

uint32
  arad_pmf_fem_action_format_map_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_MAP_GET_VERIFY);

  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_NDX, fem_ndx, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_pgm_ndx, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(selected_bits_ndx, ARAD_PMF_LOW_LEVEL_SELECTED_BITS_NDX_MAX, ARAD_PMF_LOW_LEVEL_SELECTED_BITS_NDX_OUT_OF_RANGE_ERR, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_map_get_verify()", 0, fem_pgm_ndx);
}

/*********************************************************************
*     Set the location of 4 bits from the FEM-key that select
 *     the performed action format for this key (configure the
 *     Select-4-bits table).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_fem_action_format_map_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint32                            fem_pgm_ndx,
    SOC_SAND_IN  uint32                            selected_bits_ndx,
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK,
    entry_offset,
    tbl_ndx,
    tbl_data;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_MAP_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(fem_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_clear(info);

  res = arad_pmf_fem_action_format_map_get_verify(
          unit,
          fem_ndx,
          fem_pgm_ndx,
          selected_bits_ndx
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);


  /*
   *    Get the entry offset and read from the table
   */
  entry_offset = selected_bits_ndx + (fem_pgm_ndx << ARAD_PMF_FEM_SEL_BITS_SIZE_IN_BITS);
  tbl_ndx = fem_ndx->id;
  res = soc_mem_array_read(
          unit,
          IHB_FEM_MAP_INDEX_TABLEm,
          tbl_ndx,
          MEM_BLOCK_ANY,
          entry_offset,
          &tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  info->action_fomat_id = soc_mem_field32_get(unit, IHB_FEM_MAP_INDEX_TABLEm, &tbl_data, MAP_INDEXf);
  info->map_data = soc_mem_field32_get(unit, IHB_FEM_MAP_INDEX_TABLEm, &tbl_data, MAP_DATAf);
  info->is_action_valid = SOC_SAND_NUM2BOOL(soc_mem_field32_get(unit, IHB_FEM_MAP_INDEX_TABLEm, &tbl_data, VALIDf));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_map_get_unsafe()", 0, fem_pgm_ndx);
}

/*********************************************************************
*     Configure the format of the each action done by the
 *     Field Extraction Macro. Each FEM can perform up to four
 *     different actions.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_fem_action_format_set_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info
  )
{
  uint32
    fld_extr_encoded,
    tbl_ndx ,
    table_line,
    field_select_map,
    fem_map_tbl_reference_index,
    type_fld_val = 0,
    res = SOC_SAND_OK,
    output_size_in_bits,
    tbl_data[ARAD_PMF_FEM_MAP_TABLE_TBL_DATA_LENGTH_MAX];
  uint32
    bit_ndx;
  soc_mem_t
      fem_map_table_reference,
      fem_map_table;
  uint8
    found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(fem_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_MAP_TABLE_TBL_DATA_LENGTH_MAX * sizeof(uint32));

  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "     "
                        "Action: FEM action format map set, "
                        "FEM-ID: %d, "
                        "     "
                        "FEM info: "
                        "Action type: %s, "
                        "Bit location[0]: type %s, val %d,"
                        "Bit location[1]: type %s, val %d,..."
                        "Number of bits to set: %d, "
                        "Base value: %d\n\r"),
             fem_ndx->id, SOC_PPC_FP_ACTION_TYPE_to_string(info->type), 
             SOC_TMC_PMF_FEM_BIT_LOC_TYPE_to_string(info->bit_loc[0].type), info->bit_loc[0].val, 
             SOC_TMC_PMF_FEM_BIT_LOC_TYPE_to_string(info->bit_loc[1].type), info->bit_loc[1].val, 
             info->size, info->base_value));

  res = arad_pmf_fem_action_format_set_verify(
          unit,
          fem_ndx,
          fem_pgm_id,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  res = arad_pmf_fem_output_size_get(
          unit,
          fem_ndx,
          &output_size_in_bits
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /*
   *    Init the table data - set all the values to '0'
   */
  tbl_ndx = fem_ndx->id;
  fem_map_table = Arad_pmf_fem_map_tbl[tbl_ndx];
  fem_map_tbl_reference_index = (output_size_in_bits == ARAD_PMF_FEM_SIZE_1ST_SIZE)? 0:((output_size_in_bits == ARAD_PMF_FEM_SIZE_2ND_SIZE)? 1:2);
  res = arad_pmf_fem_map_tbl_reference(
          unit,
          fem_map_tbl_reference_index,
          &fem_map_table_reference
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  
  for (bit_ndx = 0; bit_ndx < output_size_in_bits; ++bit_ndx)
  {
    field_select_map = 0;
    field_select_map |= SOC_SAND_SET_BITS_RANGE(ARAD_PMF_FEM_CST_ENCODED_BIT,
      ARAD_PMF_FEM_CST_ENCODED_BIT_MSB, ARAD_PMF_FEM_CST_ENCODED_BIT_LSB);
    /* field_select_map[bit_ndx] */
    soc_mem_field32_set(unit, fem_map_table_reference, tbl_data, Arad_pmf_fem_map_field_select_field[bit_ndx], field_select_map);
  }

  /*
   *    Set the table entry data
   */
  found = FALSE;
  for (table_line = 0; (table_line < arad_pmf_fem_action_type_array_size_get_unsafe(unit)) && (!found); ++table_line) {
      if (info->type == arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION)) {
          if (info->type == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          type_fld_val = arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_HW_ID);
          if (type_fld_val == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          found = TRUE;
      }
  }
  if (!found) {
      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Fem index %d Action type %s : Failed to set action format. No encoding found.\n\r"),
                 unit, fem_ndx->id, SOC_PPC_FP_ACTION_TYPE_to_string(info->type)));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_SUB_HEADER_OUT_OF_RANGE_ERR, 20, exit);
  }
  /* tbl_data.action_type = type_fld_val; */
  soc_mem_field32_set(unit, fem_map_table_reference, tbl_data, ACTION_TYPEf, type_fld_val);

  if (output_size_in_bits != ARAD_PMF_FEM_SIZE_1ST_SIZE)
  {
    /* offset_data.offset = info->base_value; */
    soc_mem_field32_set(unit, fem_map_table_reference, tbl_data, OFFSETf, info->base_value);
  }

  /*
   *    Build the encoded Bit location
   */
  for (bit_ndx = 0; bit_ndx < info->size; ++bit_ndx)
  {
    fld_extr_encoded = 0;
    switch(info->bit_loc[bit_ndx].type)
    {
    case ARAD_PMF_FEM_BIT_LOC_TYPE_CST:
      fld_extr_encoded = info->bit_loc[bit_ndx].val;
      fld_extr_encoded |= SOC_SAND_SET_BITS_RANGE(ARAD_PMF_FEM_CST_ENCODED_BIT,
        ARAD_PMF_FEM_CST_ENCODED_BIT_MSB, ARAD_PMF_FEM_CST_ENCODED_BIT_LSB);
      break;

    case ARAD_PMF_FEM_BIT_LOC_TYPE_KEY:
      fld_extr_encoded = info->bit_loc[bit_ndx].val;
      break;

    case ARAD_PMF_FEM_BIT_LOC_TYPE_MAP_DATA:
      fld_extr_encoded = info->bit_loc[bit_ndx].val;
      SOC_SAND_SET_BIT(fld_extr_encoded, 0x1, ARAD_PMF_FEM_MAP_DATA_ENCODED_BIT);
      break;
    
    default:
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Fem index %d Action type %s : Failed to set action format. No encoding found.\n\r"),
                 unit, fem_ndx->id, SOC_PPC_FP_ACTION_TYPE_to_string(info->bit_loc[bit_ndx].type)));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 30, exit);
    }
    /* tbl_data.field_select_map[bit_ndx] = fld_extr_encoded; */
    soc_mem_field32_set(unit, fem_map_table_reference, tbl_data, Arad_pmf_fem_map_field_select_field[bit_ndx], fld_extr_encoded);
  }

  /*
   *    Write into the tables
   */
  res = soc_mem_write(
          unit,
          fem_map_table,
          MEM_BLOCK_ANY,
          fem_pgm_id,
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_set_unsafe()", 0, fem_pgm_id);
}


uint32
  arad_pmf_fem_action_format_set_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info
  )
{
  uint32
    res = SOC_SAND_OK,
    table_line,
    output_size_in_bits;
  uint8
    found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_SET_VERIFY);

  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_NDX, fem_ndx, 10, exit);
  /* Check the action is supported in Arad */
  found = FALSE;
  for (table_line = 0; (table_line < arad_pmf_fem_action_type_array_size_get_unsafe(unit)) && (!found); ++table_line) {
      if (info->type == arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION)) {
          if (info->type == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          found = TRUE;
      }
  }
  if (!found) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 20, exit);
  }

  res = ARAD_PMF_FEM_ACTION_FORMAT_INFO_verify(unit, info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /*
   *    Check if this FEM has an offset option
   */
  if ((fem_ndx->id < ARAD_PMF_FEM_FOR_OFFSET_MIN) && (info->base_value != 0))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PMF_FEM_INVALID_FOR_OFFSET_ERR, 40, exit);
  }

  /*
   *    Check if the size is coherent with the FEM type
   */
  res = arad_pmf_fem_output_size_get(
          unit,
          fem_ndx,
          &output_size_in_bits
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if (info->size > output_size_in_bits)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PMF_FEM_OUTPUT_SIZE_ERR, 60, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_set_verify()", 0, fem_pgm_id);
}

uint32
  arad_pmf_fem_action_format_get_verify(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_GET_VERIFY);

  ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_NDX, fem_ndx, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(fem_pgm_id, ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_MAX, ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_get_verify()", 0, fem_pgm_id);
}

/*********************************************************************
*     Configure the format of the each action done by the
 *     Field Extraction Macro. Each FEM can perform up to four
 *     different actions.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pmf_fem_action_format_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_NDX                    *fem_ndx,
    SOC_SAND_IN  uint8                            fem_pgm_id,
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_INFO     *info
  )
{
  uint32
    type_fld_val,
    res = SOC_SAND_OK,
    output_size_in_bits,
    field_select_map,
    fem_map_tbl_reference_index,
    table_line,
    tbl_ndx,
    tbl_data[ARAD_PMF_FEM_MAP_TABLE_TBL_DATA_LENGTH_MAX];
  uint32
    bit_ndx;
  soc_mem_t
      fem_map_table_reference,
      fem_map_table;
  uint8
    found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PMF_FEM_ACTION_FORMAT_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(fem_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PMF_FEM_ACTION_FORMAT_INFO_clear(info);

  sal_memset(tbl_data, 0x0, ARAD_PMF_FEM_MAP_TABLE_TBL_DATA_LENGTH_MAX * sizeof(uint32));

  res = arad_pmf_fem_action_format_get_verify(
          unit,
          fem_ndx,
          fem_pgm_id
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  /*
   *    Get the FEM size according to the FEM id
   */
  res = arad_pmf_fem_output_size_get(
          unit,
          fem_ndx,
          &output_size_in_bits
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  /*
   *    Get the data from the tables
   */
  tbl_ndx = fem_ndx->id;
  fem_map_table = Arad_pmf_fem_map_tbl[tbl_ndx];
  res = soc_mem_read(
          unit,
          fem_map_table,
          MEM_BLOCK_ANY,
          fem_pgm_id,
          tbl_data
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /*
   *    Take the offset only if the FEM-ID is superior to 2
   *  (i.e. the option of an offset exists)
   */
  fem_map_tbl_reference_index = (output_size_in_bits == ARAD_PMF_FEM_SIZE_1ST_SIZE)? 0:((output_size_in_bits == ARAD_PMF_FEM_SIZE_2ND_SIZE)? 1:2);
  res = arad_pmf_fem_map_tbl_reference(
          unit,
          fem_map_tbl_reference_index,
          &fem_map_table_reference
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

  if (fem_ndx->id >= ARAD_PMF_FEM_FOR_OFFSET_MIN)
  {
    info->base_value = soc_mem_field32_get(unit, fem_map_table_reference, tbl_data, OFFSETf);
  }
  else
  {
    info->base_value = 0;
  }


  /*
   *    Set the action type
   */
  type_fld_val = soc_mem_field32_get(unit, fem_map_table_reference, tbl_data, ACTION_TYPEf);
  found = FALSE;
  for (table_line = 0; (table_line < arad_pmf_fem_action_type_array_size_get_unsafe(unit)) && (!found); ++table_line) {
      if (type_fld_val == arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_HW_ID)) {
          if (type_fld_val == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          info->type = arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION);
          if (info->type == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          found = TRUE;
      }
  }
  if (!found) {
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Fem index %d Action code %d - Faile dto get action format. The action is not supported.\n\r"), unit, fem_ndx->id, type_fld_val));
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 30, exit);
  }

  info->size = output_size_in_bits;


  /*
   *    Build the encoded Bit location
   */
  for (bit_ndx = 0; bit_ndx < info->size; ++bit_ndx)
  {
    field_select_map = soc_mem_field32_get(unit, fem_map_table_reference, tbl_data, Arad_pmf_fem_map_field_select_field[bit_ndx]);
    if (SOC_SAND_GET_BIT(field_select_map, 5) == 0)
    {
      /*
       *    FEM-Key source
       */
      info->bit_loc[bit_ndx].type = ARAD_PMF_FEM_BIT_LOC_TYPE_KEY;
      info->bit_loc[bit_ndx].val = SOC_SAND_GET_BITS_RANGE(field_select_map, 4, 0);
    }
    else if (SOC_SAND_GET_BITS_RANGE(field_select_map, 5, 2) == ARAD_PMF_FEM_MAP_DATA_ENCODING)
    {
      /*
       *    Map-Data source
       */
      info->bit_loc[bit_ndx].type = ARAD_PMF_FEM_BIT_LOC_TYPE_MAP_DATA;
      info->bit_loc[bit_ndx].val = SOC_SAND_GET_BITS_RANGE(field_select_map, 1, 0);
    }
    else
    {
      /*
       *    Constant source
       */
      info->bit_loc[bit_ndx].type = ARAD_PMF_FEM_BIT_LOC_TYPE_CST;
      info->bit_loc[bit_ndx].val = SOC_SAND_GET_BIT(field_select_map, 0);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pmf_fem_action_format_get_unsafe()", 0, fem_pgm_id);
}



uint32
  ARAD_PMF_FEM_INPUT_INFO_verify(
     SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO *info
  )
{
    SOC_PPC_FP_DATABASE_STAGE            
        stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->pgm_id, ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX, ARAD_PMF_LOW_LEVEL_PGM_ID_OUT_OF_RANGE_ERR, 10, exit);

  if (info->src != SOC_TMC_NOF_PMF_FEM_INPUT_SRCS) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR, 20, exit);
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->db_id, ARAD_PMF_LOW_LEVEL_NOF_DATABASES, ARAD_PMF_LOW_LEVEL_DB_ID_OUT_OF_RANGE_ERR, 22, exit);
  if (info->is_16_lsb_overridden) {
      if (info->lsb16_src.is_key_src != FALSE) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR, 25, exit);
      }
      SOC_SAND_ERR_IF_ABOVE_MAX(info->lsb16_src.key_tcam_id, ARAD_PMF_FEM_NOF_TCAM_RESULTS, ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR, 26, exit);
  }

  SOC_SAND_ERR_IF_ABOVE_MAX(info->src_arad.key_tcam_id, ARAD_PMF_FEM_NOF_TCAM_RESULTS, ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR, 27, exit);
  if (info->src_arad.is_key_src) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->src_arad.key_lsb, ARAD_PMF_FEM_KEY_LSB_MAX, ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR, 30, exit);
      if (((info->src_arad.key_lsb % ARAD_PMF_FEM_KEY_LSB_MULTIPLE)!= 0) 
          || (info->src_arad.key_lsb == ARAD_PMF_FEM_KEY_LSB_FORBIDDEN)){
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_SRC_OUT_OF_RANGE_ERR, 35, exit);
      }
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FEM_INPUT_INFO_verify()",0,0);
}

uint32
  ARAD_PMF_FES_INPUT_INFO_verify(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  ARAD_PMF_FES_INPUT_INFO *info
  )
{
    uint8
        found = FALSE;
    uint32
        table_line;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->shift, ARAD_PMF_FES_SHIFT_MAX, ARAD_PMF_FES_SHIFT_OUT_OF_RANGE_ERR, 10, exit);

  found = FALSE;
  for (table_line = 0; (table_line < arad_pmf_fem_action_type_array_size_get_unsafe(unit)) && (!found); ++table_line) {
      if (info->action_type == arad_pmf_fem_action_type_array_element_get_unsafe(unit, table_line, INDEX_OF_ACTION)) {
          if (info->action_type == ACTION_TYPE_ARRAY_ELEMENT_GET_FAIL) {
              break ;
          }
          found = TRUE;
      }
  }
  if (!found) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_OUT_OF_RANGE_ERR, 20, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FES_INPUT_INFO_verify()",0,0);
}

uint32
  ARAD_PMF_FEM_NDX_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_NDX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->id, ARAD_PMF_LOW_LEVEL_FEM_ID_MAX, ARAD_PMF_LOW_LEVEL_FEM_ID_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FEM_NDX_verify()",0,0);
}

uint32
  ARAD_PMF_FEM_SELECTED_BITS_INFO_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_OUT_OF_RANGE(info->sel_bit_msb, ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_MIN, ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_MAX, ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_OUT_OF_RANGE_ERR, 10, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FEM_SELECTED_BITS_INFO_verify()",0,0);
}

uint32
  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->action_fomat_id, ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_ID_MAX, ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->map_data, ARAD_PMF_LOW_LEVEL_MAP_DATA_MAX, ARAD_PMF_LOW_LEVEL_MAP_DATA_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_verify()",0,0);
}

uint32
  ARAD_PMF_FEM_BIT_LOC_verify(
    SOC_SAND_IN  ARAD_PMF_FEM_BIT_LOC *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->type, ARAD_PMF_LOW_LEVEL_TYPE_MAX, ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 10, exit);
  switch(info->type)
  {
  case ARAD_PMF_FEM_BIT_LOC_TYPE_CST:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val, TRUE, ARAD_PMF_LOW_LEVEL_BIT_LOC_VAL_OUT_OF_RANGE_ERR, 11, exit);
    break;
  
  case ARAD_PMF_FEM_BIT_LOC_TYPE_KEY:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val, ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_MAX, ARAD_PMF_LOW_LEVEL_BIT_LOC_VAL_OUT_OF_RANGE_ERR, 12, exit);
    break;

  case ARAD_PMF_FEM_BIT_LOC_TYPE_MAP_DATA:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val, ARAD_PMF_FEM_SEL_BITS_SIZE_IN_BITS-1, ARAD_PMF_LOW_LEVEL_BIT_LOC_VAL_OUT_OF_RANGE_ERR, 13, exit);
    break;

  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_TYPE_OUT_OF_RANGE_ERR, 20, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FEM_BIT_LOC_verify()",0,0);
}

uint32
  ARAD_PMF_FEM_ACTION_FORMAT_INFO_verify(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->type, SOC_PPC_NOF_FP_ACTION_TYPES_ARAD, ARAD_PMF_LOW_LEVEL_AF_TYPE_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->size, SOC_DPP_DEFS_GET(unit, fem_max_action_size_nof_bits), ARAD_PMF_LOW_LEVEL_SIZE_OUT_OF_RANGE_ERR, 11, exit);
  for (ind = 0; ind < info->size; ++ind)
  {
    ARAD_STRUCT_VERIFY(ARAD_PMF_FEM_BIT_LOC, &(info->bit_loc[ind]), 12, exit);
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->base_value, ARAD_PMF_LOW_LEVEL_BASE_VALUE_MAX, ARAD_PMF_LOW_LEVEL_BASE_VALUE_OUT_OF_RANGE_ERR, 13, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in ARAD_PMF_FEM_ACTION_FORMAT_INFO_verify()",0,0);
}

void
  ARAD_PMF_FEM_INPUT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_INPUT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_INPUT_INFO_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FES_INPUT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FES_INPUT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FES_INPUT_INFO_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_NDX_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_NDX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_NDX_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_SELECTED_BITS_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_SELECTED_BITS_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_SELECTED_BITS_INFO_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_BIT_LOC_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_BIT_LOC *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_BIT_LOC_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_ACTION_FORMAT_INFO_clear(
    SOC_SAND_OUT ARAD_PMF_FEM_ACTION_FORMAT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO_clear(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

#if ARAD_DEBUG_IS_LVL1

const char*
  ARAD_PMF_FEM_BIT_LOC_TYPE_to_string(
    SOC_SAND_IN  ARAD_PMF_FEM_BIT_LOC_TYPE enum_val
  )
{
  return SOC_TMC_PMF_FEM_BIT_LOC_TYPE_to_string(enum_val);
}

void
  ARAD_PMF_FEM_INPUT_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_INPUT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  LOG_CLI((BSL_META_U(unit,
                      "FEM-Program: %u, "),info->pgm_id));
  LOG_CLI((BSL_META_U(unit,
                      "source: ")));
  SOC_TMC_PMF_FEM_INPUT_SRC_ARAD_print(&(info->src_arad));
  LOG_CLI((BSL_META_U(unit,
                      "is_16_lsb_overridden: %u, "),info->is_16_lsb_overridden));
  if (info->is_16_lsb_overridden) {
      LOG_CLI((BSL_META_U(unit,
                          "source: ")));
      SOC_TMC_PMF_FEM_INPUT_SRC_ARAD_print(&(info->lsb16_src));
  }
  LOG_CLI((BSL_META_U(unit,
                      "db_id: %u\n\r"),info->db_id));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_NDX_print(
    SOC_SAND_IN  ARAD_PMF_FEM_NDX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_NDX_print(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_SELECTED_BITS_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_SELECTED_BITS_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_SELECTED_BITS_INFO_print(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_MAP_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_ACTION_FORMAT_MAP_INFO_print(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_BIT_LOC_print(
    SOC_SAND_IN  ARAD_PMF_FEM_BIT_LOC *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_BIT_LOC_print(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_PMF_FEM_ACTION_FORMAT_INFO_print(
    SOC_SAND_IN  ARAD_PMF_FEM_ACTION_FORMAT_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_TMC_PMF_FEM_ACTION_FORMAT_INFO_print(info);
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
#endif /* ARAD_DEBUG_IS_LVL1 */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#undef _ERR_MSG_MODULE_NAME

#endif /* of #if defined(BCM_88650_A0) */

