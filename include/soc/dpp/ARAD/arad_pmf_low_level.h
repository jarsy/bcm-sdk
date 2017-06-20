
/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifndef __ARAD_PMF_LOW_LEVEL_INCLUDED__
/* { */
#define __ARAD_PMF_LOW_LEVEL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/arad_api_framework.h>
#include <soc/dpp/ARAD/arad_api_ports.h>
#include <soc/dpp/ARAD/arad_general.h>

#include <soc/dpp/TMC/tmc_api_pmf_low_level_ce.h>
#include <soc/dpp/TMC/tmc_api_pmf_low_level_db.h>
#include <soc/dpp/TMC/tmc_api_pmf_low_level_fem_tag.h>
#include <soc/dpp/TMC/tmc_api_pmf_low_level_pgm.h>

#include <soc/dpp/PPC/ppc_api_fp_fem.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximum number of banks per TCAM.                       */
#define  ARAD_PMF_TCAM_NOF_BANKS (SOC_TMC_PMF_TCAM_NOF_BANKS)

/*     Number of final TCAM lookup results.                    */
#define  ARAD_PMF_TCAM_NOF_LKP_RESULTS (SOC_TMC_PMF_TCAM_NOF_LKP_RESULTS)

/*     Number of FEM selected bits.                            */
#define  ARAD_PMF_FEM_NOF_SELECTED_BITS (SOC_TMC_PMF_FEM_NOF_SELECTED_BITS)

/*     Maximal number of bits in the FEM output result (action
*     value).                                                 */
#define  ARAD_PMF_FEM_MAX_OUTPUT_SIZE_IN_BITS (SOC_TMC_PMF_FEM_MAX_OUTPUT_SIZE_IN_BITS)

/*     Number of PMF Programs.                                 */
#define  ARAD_PMF_NOF_PGMS (SOC_TMC_PMF_NOF_PGMS)

/*     Number of cycles in the PMF process.                    */
#define  ARAD_PMF_NOF_CYCLES (ARAD_PMF_LOW_LEVEL_NOF_CYCLES_MAX_ALL_LEVELS)

/*     Number of lookup profiles in the PMF process.           */
#define ARAD_PMF_LOW_LEVEL_NOF_DBS_PER_LOOKUP_PROFILE_MAX      (4)

#define ARAD_PMF_LOW_LEVEL_NOF_DATABASES                       (SOC_TMC_PMF_LOW_LEVEL_NOF_DATABASES)


#define ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(_ingress_pmf, _egress_pmf, _ingress_flp, _ingress_slb, _ingress_vt, _ingress_tt) \
    ((stage == SOC_PPC_FP_DATABASE_STAGE_EGRESS)? SOC_DPP_DEFS_GET(unit, _egress_pmf): \
     (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)? SOC_DPP_DEFS_GET(unit, _ingress_flp): \
     (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)? SOC_DPP_DEFS_GET(unit, _ingress_slb): \
     (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF)? SOC_DPP_DEFS_GET(unit, _ingress_pmf): \
     (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT) ? SOC_DPP_DEFS_GET(unit, _ingress_vt): \
     (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT) ? SOC_DPP_DEFS_GET(unit, _ingress_tt): 31/* This number has no meaning */)


#define ARAD_PMF_LOW_LEVEL_NOF_PROGS                          (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX+1) /* 16/32 */
#define ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES               (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX_ALL_STAGES+1) /* 32 */
#define ARAD_PMF_LOW_LEVEL_NOF_CYCLES                         ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_cycles, \
                                                                 nof_egress_pmf_cycles, nof_flp_cycles, nof_slb_cycles, \
                                                                 nof_vt_cycles, nof_tt_cycles )
#define ARAD_PMF_LOW_LEVEL_NOF_CYCLES_MAX_ALL_LEVELS          SOC_DPP_DEFS_MAX(NOF_INGRESS_PMF_CYCLES)
#define ARAD_PMF_LOW_LEVEL_NOF_LSB_MSB                        ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_80b_zones, \
                                                                 nof_egress_pmf_80b_zones, nof_flp_80b_zones, nof_slb_80b_zones, \
                                                                 nof_vt_80b_zones, nof_tt_80b_zones)
#define ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG                     ((ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB)*ARAD_PMF_LOW_LEVEL_NOF_LSB_MSB)
#define ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS      (32)/* 16*2 = 32 in ingress PMF */ /* in cycle */
#define ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP               (ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_instructions_lsb, \
                                                                 nof_egress_pmf_instructions_lsb, nof_flp_instructions_lsb, nof_slb_instructions_lsb, \
                                                                 nof_vt_instructions_lsb, nof_tt_instructions_lsb) / 2)
#define ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB                 (ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP * 2) /* 8/16: 2 groups in LSB */
#define ARAD_PMF_LOW_LEVEL_PROG_GROUP_IS_ALL_32               ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)? 1: 0)
#define ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP            (ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP / 2)  /* 2/4 always same number of 16b and 32b in a group */
#define ARAD_PMF_LOW_LEVEL_NOF_KEYS                           (ARAD_PMF_LOW_LEVEL_PMF_KEY_MAX + 1) /* 2 / 4 */

/* size of zones for key */
#define ARAD_PMF_LOW_LEVEL_NOF_ZONES_PER_STAGE                ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_key_zones, \
                                                                 nof_egress_pmf_key_zones, nof_flp_key_zones, nof_slb_key_zones, \
                                                                 nof_vt_key_zones, nof_tt_key_zones)
#define ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE                ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_key_zone_bits, \
                                                                 nof_egress_pmf_key_zone_bits, nof_flp_key_zone_bits, nof_slb_key_zone_bits, \
                                                                 nof_vt_key_zone_bits, nof_tt_key_zone_bits)

/* Number of databases for TCAM lookup, may be extended */
#define ARAD_PMF_LOW_LEVEL_NOF_DBS                            (SOC_TMC_PMF_LOW_LEVEL_NOF_DATABASES) /* 128 */
#define ARAD_PMF_LOW_LEVEL_NOF_DBS_BMP                        ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS)

/*     Number of User-Defined Fields (UDF)                      */
#define ARAD_PP_FP_NOF_HDR_USER_DEFS                             (SOC_PPC_FP_QUAL_HDR_USER_DEF_LAST - SOC_PPC_FP_QUAL_HDR_USER_DEF_0 + 1)

#define ARAD_PMF_LOW_LEVEL_NOF_FEMS               (16)
#define ARAD_PMF_LOW_LEVEL_NOF_FESS               (32)

#define ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX                    (ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_programs, \
                                                                 nof_egress_pmf_programs, nof_flp_programs, nof_slb_programs, \
                                                                 nof_vt_programs, nof_tt_programs ) - 1)
#define ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX_ALL_STAGES         (SOC_DPP_DEFS_MAX(NOF_INGRESS_PMF_PROGRAMS) - 1)
                                                               /* 4 keys in Arad, 2 in Petra-B, 2 at Arad egress */
#define ARAD_PMF_LOW_LEVEL_PMF_KEY_MAX                        (ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_keys, \
                                                                 nof_egress_pmf_keys, nof_flp_keys, nof_slb_keys, \
                                                                 nof_vt_keys, nof_tt_keys) - 1)
#define ARAD_PMF_LOW_LEVEL_PMF_KEY_MAX_ALL_STAGES             (SOC_DPP_DEFS_MAX(NOF_INGRESS_PMF_KEYS) - 1)
#define ARAD_PMF_LOW_LEVEL_CE_NDX_MAX                         (ARAD_PMF_LOW_LEVEL_PMF_KLEAR_STAGE_INFO_GET(nof_ingress_pmf_instructions_lsb, \
                                                                 nof_egress_pmf_instructions_lsb, nof_flp_instructions_lsb, nof_slb_instructions_lsb, \
                                                                 nof_vt_instructions_lsb, nof_tt_instructions_lsb) - 1)
#define ARAD_PMF_LOW_LEVEL_CE_NDX_MAX_ALL_STAGES              (SOC_DPP_DEFS_MAX(NOF_INGRESS_PMF_INSTRUCTIONS_LSB) - 1) 
#define ARAD_PMF_LOW_LEVEL_IRPP_FIELD_MAX                     (ARAD_NOF_PMF_IRPP_INFO_FIELDS-1)
#define ARAD_PMF_LOW_LEVEL_IS_CE_NOT_VALID_MAX                (7)
#define ARAD_PMF_LOW_LEVEL_DB_ID_NDX_MAX                      (ARAD_PMF_LOW_LEVEL_NOF_DATABASES-1)
#define ARAD_PMF_LOW_LEVEL_KEY_FORMAT_MAX                     (ARAD_NOF_PMF_TCAM_KEY_SRCS-1)
#define ARAD_PMF_LOW_LEVEL_KEY_SRC_MAX                        (ARAD_NOF_PMF_DIRECT_TBL_KEY_SRCS-1)
#define ARAD_PMF_LOW_LEVEL_ENTRY_NDX_MAX                      (1023)
#define ARAD_PMF_LOW_LEVEL_TAG_PROFILE_NDX_MAX                (ARAD_PMF_NOF_TAG_PROFILES - 1)
#define ARAD_PMF_LOW_LEVEL_TAG_TYPE_NDX_MAX                   (ARAD_NOF_PMF_TAG_TYPES-1)
#define ARAD_PMF_LOW_LEVEL_FEM_PGM_NDX_MAX                    (3)
#define ARAD_PMF_LOW_LEVEL_SELECTED_BITS_NDX_MAX              (15)
#define ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_NDX_MAX               (3)
#define ARAD_PMF_LOW_LEVEL_SUB_HEADER_MAX                     (ARAD_NOF_PMF_CE_SUB_HEADERS-1)
#define ARAD_PMF_LOW_LEVEL_OFFSET_MIN                         (-512)
#define ARAD_PMF_LOW_LEVEL_OFFSET_MAX                         (508)
#define ARAD_PMF_LOW_LEVEL_NOF_BITS_MIN                       (1)
#define ARAD_PMF_LOW_LEVEL_NOF_BITS_CE_16B_MAX                (16)
#define ARAD_PMF_LOW_LEVEL_NOF_BITS_CE_32B_MAX                (32)
#define ARAD_PMF_LOW_LEVEL_DB_ID_MAX                          (ARAD_PMF_LOW_LEVEL_NOF_DATABASES-1)
#define ARAD_PMF_LOW_LEVEL_ENTRY_ID_MAX                       (16*1024-1)
#define ARAD_PMF_LOW_LEVEL_PRIORITY_MAX                       (16*1024-1)
#define ARAD_PMF_LOW_LEVEL_ID_MAX                             (7)
#define ARAD_PMF_LOW_LEVEL_FEM_ID_MAX                         (ARAD_PMF_LOW_LEVEL_NOF_FEMS-1)
#define ARAD_PMF_LOW_LEVEL_FEM_ID_MAX_PER_GROUP               (ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP-1)
#define ARAD_PMF_LOW_LEVEL_FES_ID_MAX                         (ARAD_PMF_LOW_LEVEL_NOF_FESS-1)
#define ARAD_PMF_LOW_LEVEL_SRC_MAX                            (ARAD_NOF_PMF_FEM_INPUT_SRCS-1)
#define ARAD_PMF_LOW_LEVEL_VAL_SRC_MAX                        (ARAD_NOF_PMF_TAG_VAL_SRCS-1)
#define ARAD_PMF_LOW_LEVEL_STAT_TAG_LSB_POSITION_MAX          ((1<<5)-1)
#define ARAD_PMF_LOW_LEVEL_SIZE_MAX                           (SOC_TMC_NOF_TCAM_BANK_ENTRY_SIZES-1)
#define ARAD_PMF_LOW_LEVEL_FORMAT_MAX                         (ARAD_NOF_PMF_TCAM_KEY_SRCS-1)
#define ARAD_PMF_LOW_LEVEL_MODE_MAX                           (ARAD_NOF_PMF_TCAM_FLD_L3_MODES-1)
#define ARAD_PMF_LOW_LEVEL_IPV4_FLD_MAX                       (ARAD_NOF_PMF_FLD_IPV4S-1)
#define ARAD_PMF_LOW_LEVEL_IPV6_FLD_MAX                       (ARAD_NOF_PMF_FLD_IPV6S-1)
#define ARAD_PMF_LOW_LEVEL_LOC_MAX                            (ARAD_NOF_PMF_TCAM_FLD_A_B_LOCS-1)
#define ARAD_PMF_LOW_LEVEL_IRPP_FLD_MAX                       (ARAD_NOF_PMF_IRPP_INFO_FIELDS-1)
#define ARAD_PMF_LOW_LEVEL_SUB_HEADER_OFFSET_MAX              (SOC_SAND_U32_MAX)
/*#define ARAD_PMF_LOW_LEVEL_PMF_PGM_ID_MAX                     (ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX)*/
#define ARAD_PMF_LOW_LEVEL_L2_MAX                             (ARAD_NOF_PMF_TCAM_FLD_L2S-1)
#define ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_MIN                    (3)
#define ARAD_PMF_LOW_LEVEL_SEL_BIT_MSB_MAX                    (31)
#define ARAD_PMF_LOW_LEVEL_ACTION_FOMAT_ID_MAX                (3)
#define ARAD_PMF_LOW_LEVEL_MAP_DATA_MAX                       (15)
#define ARAD_PMF_LOW_LEVEL_TYPE_MAX                           (ARAD_NOF_PMF_FEM_BIT_LOC_TYPES-1)
#define ARAD_PMF_LOW_LEVEL_BASE_VALUE_MAX                     (SOC_SAND_U32_MAX)
#define ARAD_PMF_LOW_LEVEL_PFQ_MAX                            (3)
#define ARAD_PMF_LOW_LEVEL_SEM_13_8_NDX_MAX                   (63)
#define ARAD_PMF_LOW_LEVEL_FWD_MAX                            (SOC_TMC_NOF_PKT_FRWRD_TYPES-1)
#define ARAD_PMF_LOW_LEVEL_TTC_MAX                            (SOC_TMC_NOF_TUNNEL_TERM_CODES-1)
#define ARAD_PMF_LOW_LEVEL_PRSR_MAX                           (15)
#define ARAD_PMF_LOW_LEVEL_PORT_PMF_MAX                       (7)
#define ARAD_PMF_LOW_LEVEL_LLVP_MAX                           (15)
#define ARAD_PMF_LOW_LEVEL_PMF_PRO_MAX                        (7)
#define ARAD_PMF_LOW_LEVEL_EEI_OUTLIF_15_8_MAX                (255)
#define ARAD_PMF_LOW_LEVEL_EEI_OUTLIF_7_0_MAX                 (255)
#define ARAD_PMF_LOW_LEVEL_SEM_7_0_NDX_MAX                    (255)
#define ARAD_PMF_LOW_LEVEL_HEADER_TYPE_MAX                    (ARAD_NOF_PMF_PGM_BYTES_TO_RMV_HDRS-1)
#define ARAD_PMF_LOW_LEVEL_NOF_BYTES_MAX                      (31)
#define ARAD_PMF_LOW_LEVEL_LKP_PROFILE_ID_MAX                 (SOC_PPC_FP_NOF_PFGS_ARAD - 1)
#define ARAD_PMF_LOW_LEVEL_TAG_PROFILE_ID_MAX                 (7)
#define ARAD_PMF_LOW_LEVEL_COPY_PGM_VAR_MAX                   (0xFFFFFFFF)
#define ARAD_PMF_LOW_LEVEL_FC_TYPE_MAX                        (ARAD_PORTS_NOF_FC_TYPES-1)

#define ARAD_PMF_LOW_LEVEL_AF_TYPE_MAX                        (SOC_PPC_NOF_FP_ACTION_TYPES_ARAD-1)
#define ARAD_PMF_LOW_LEVEL_PGM_TYPE_MAX                       (ARAD_NOF_PMF_PGM_SEL_TYPES-1)
#define ARAD_PMF_FEM_SEL_BITS_SIZE_IN_BITS                    (ARAD_PP_IHP_FEM_SEL_BITS_SIZE_IN_BITS)
#define ARAD_PMF_FEM_FOR_OFFSET_MIN                           (2)
/* Encoded bit declare constant value */
#define ARAD_PMF_FEM_MAP_DATA_ENCODED_BIT                     (5)
#define ARAD_PMF_FEM_CST_ENCODED_BIT                          (0x1F)
#define ARAD_PMF_FEM_CST_ENCODED_BIT_MSB                      (5)
#define ARAD_PMF_FEM_CST_ENCODED_BIT_LSB                      (1)
#define ARAD_PMF_FEM_MAP_DATA_ENCODING                        (0x8)
#define ARAD_PMF_PGM_SEL_PORT_PMF_SIZE_IN_BITS                (3)
#define ARAD_PMF_PGM_SEL_PMF_PROFILE_SIZE_IN_BITS             (3)
#define ARAD_PMF_LOW_LEVEL_DIRECT_TBL_VAL_MAX                 ((1<<20)-1)
#define ARAD_PMF_DIRECT_TBL_BITWISE_AND_VAL                   (0x3)
#define ARAD_PMF_CE_OFFSET_RESOLUTION                         (4)
#define ARAD_PMF_CE_OFFSET_NIBBLE_VAL_SIZE_IN_BITS            (7)
#define ARAD_PMF_CE_NIBBLE_IN_BITS                            (4)
#define ARAD_PMF_TCAM_PREFIX_MSB                              (3)
#define ARAD_PMF_TCAM_PREFIX_SIZE_IN_BITS                     (4)
#define ARAD_PMF_KEY_FLD_ALIGNMENT                            (4)
#define ARAD_PMF_PGM_SEL_VALUE_INIT                           (0x7FFFFFFF)
#define ARAD_PMF_CE_INSTRUCTION_NOF_BITS_MAX                  (32)
#define ARAD_PMF_LOW_LEVEL_FEM_SIZE_MAX                       (19)

#define ARAD_PMF_PGM_PMF_PORT_PROFILE_BASE_VAL_RAW            (0)
#define ARAD_PMF_PGM_PMF_PORT_PROFILE_BASE_VAL_TM             (2)
#define ARAD_PMF_PGM_PMF_PORT_PROFILE_BASE_VAL_TM_FAT_PIPE    (3)
#define ARAD_PMF_PGM_PMF_PORT_PROFILE_BASE_VAL_TDM            (4)
#define ARAD_PMF_PGM_PMF_PORT_PROFILE_BASE_VAL_ETH_FAT_PIPE   (5)

#define ARAD_TCAM_BANK_ENTRY_SIZE_72_BITS_SIZE_IN_BITS              (72)
#define ARAD_TCAM_BANK_ENTRY_SIZE_144_BITS_SIZE_IN_BITS             (144)
#define ARAD_TCAM_BANK_ENTRY_SIZE_288_BITS_SIZE_IN_BITS             (288)

#define ARAD_PMF_PGM_NDX_TM(fat_pipe, pph, ingress_shape)     (31 - (fat_pipe) - ((pph) << 1) - ((ingress_shape) << 2))
#define ARAD_PMF_ETH_DFLT_PGM                                 (1)
#define ARAD_PMF_PGM_ETH_FAT_PIPE                             (2)
#define ARAD_PMF_PGM_NDX_STACK                                (14)
#define ARAD_PMF_PGM_NDX_TM_FAT_PIPE_PPH_PRESENT              ARAD_PMF_PGM_NDX_TM(TRUE,  TRUE,  FALSE)
#define ARAD_PMF_PGM_NDX_TM_REGULAR_PPH_PRESENT               ARAD_PMF_PGM_NDX_TM(FALSE, TRUE,  FALSE)
#define ARAD_PMF_PGM_NDX_TM_FAT_PIPE                          ARAD_PMF_PGM_NDX_TM(TRUE,  FALSE, FALSE)
#define ARAD_PMF_PGM_NDX_TM_REGULAR                           ARAD_PMF_PGM_NDX_TM(FALSE, FALSE, FALSE)
#define ARAD_PMF_PGM_NDX_RAW                                  (0)

#define ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_ETH               (0) /* In fact, from 0 to 4 according to the PFG (FP) */
#define ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_RAW               (5)
#define ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_TM                (6)
#define ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_STACKING          (7)


/* Tag-Profile = Lookup-Profile */
#define ARAD_PMF_PGM_TAG_PROFILE_NDX_FOR_ETH                  (ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_ETH)
#define ARAD_PMF_PGM_TAG_PROFILE_NDX_FOR_RAW                  (ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_RAW)
#define ARAD_PMF_PGM_TAG_PROFILE_NDX_FOR_TM                   (ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_TM)
#define ARAD_PMF_PGM_TAG_PROFILE_NDX_FOR_STACKING             (ARAD_PMF_PGM_LOOKUP_PROFILE_NDX_FOR_STACKING)

#define ARAD_PMF_FEM_PGM_FOR_TM                               (3)
#define ARAD_PMF_FEM_PGM_FOR_ETH_0                            (0)
#define ARAD_PMF_FEM_PGM_FOR_ETH_1                            (1)
#define ARAD_PMF_FEM_PGM_FOR_ETH_2                            (2)
#define ARAD_PMF_FEM_PGM_INVALID                              (0xFF)

#define ARAD_PMF_NOF_FEM_PGMS                                 (4)

#define ARAD_PMF_CE_PACKET_HEADER_FLD_VAL                     (0x0)
#define ARAD_PMF_CE_IRPP_INFO_FLD_VAL                         (0x1)

#define SOC_PPC_FP_ACTION_TYPE_DEST_FLD_VAL                 (0)
#define SOC_PPC_FP_ACTION_TYPE_DP_FLD_VAL                   (1)
#define SOC_PPC_FP_ACTION_TYPE_TC_FLD_VAL                   (2)
#define SOC_PPC_FP_ACTION_TYPE_TRAP_FLD_VAL                 (3)
#define SOC_PPC_FP_ACTION_TYPE_SNP_FLD_VAL                  (4)
#define SOC_PPC_FP_ACTION_TYPE_MIRROR_FLD_VAL               (5)
#define SOC_PPC_FP_ACTION_TYPE_MIR_DIS_FLD_VAL              (6)
#define SOC_PPC_FP_ACTION_TYPE_EXC_SRC_FLD_VAL              (7)
#define SOC_PPC_FP_ACTION_TYPE_IS_FLD_VAL                   (8)
#define SOC_PPC_FP_ACTION_TYPE_METER_FLD_VAL                (9)
#define SOC_PPC_FP_ACTION_TYPE_COUNTER_FLD_VAL              (10)
#define SOC_PPC_FP_ACTION_TYPE_VSQ_PTR_FLD_VAL                 (11)
#define SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY_FLD_VAL        (13)
#define SOC_PPC_FP_ACTION_TYPE_2ND_PASS_PGM_FLD_VAL         (12)
#define SOC_PPC_FP_ACTION_TYPE_OUTLIF_FLD_VAL               (14)
#define SOC_PPC_FP_ACTION_TYPE_NOP_FLD_VAL                  (15)

#define ARAD_PMF_PGM_BYTES_TO_RMV_HDR_START_FLD_VAL           (0)
#define ARAD_PMF_PGM_BYTES_TO_RMV_HDR_1ST_FLD_VAL             (1)
#define ARAD_PMF_PGM_BYTES_TO_RMV_HDR_FWDING_FLD_VAL          (2)
#define ARAD_PMF_PGM_BYTES_TO_RMV_HDR_POST_FWDING_FLD_VAL     (3)

#define ARAD_PMF_PGM_HEADER_PRO_TM_PPH_PRESENT_FLD_VAL        (6)
#define ARAD_PMF_PGM_HEADER_PRO_TM_FLD_VAL                    (3)
#define ARAD_PMF_PGM_HEADER_PRO_PP_FLD_VAL                    (0)
#define ARAD_PMF_PGM_HEADER_PRO_FTMH_FLD_VAL                  (7)

#define ARAD_PMF_TAG_VAL_SRC_A_29_0_FLD_VAL                   (0)
#define ARAD_PMF_TAG_VAL_SRC_A_61_32_FLD_VAL                  (1)
#define ARAD_PMF_TAG_VAL_SRC_B_29_0_FLD_VAL                   (2)
#define ARAD_PMF_TAG_VAL_SRC_B_61_32_FLD_VAL                  (3)
#define ARAD_PMF_TAG_VAL_SRC_TCAM_0_FLD_VAL                   (4)
#define ARAD_PMF_TAG_VAL_SRC_TCAM_1_FLD_VAL                   (5)
#define ARAD_PMF_TAG_VAL_SRC_TCAM_2_FLD_VAL                   (6)
#define ARAD_PMF_TAG_VAL_SRC_DIR_TBL_FLD_VAL                  (7)

#define ARAD_PMF_FEM_INPUT_SRC_A_31_0_FLD_VAL                 (5)
#define ARAD_PMF_FEM_INPUT_SRC_A_47_16_FLD_VAL                (6)
#define ARAD_PMF_FEM_INPUT_SRC_A_63_32_FLD_VAL                (7)
#define ARAD_PMF_FEM_INPUT_SRC_A_79_48_FLD_VAL                (8)
#define ARAD_PMF_FEM_INPUT_SRC_A_95_64_FLD_VAL                (9)
#define ARAD_PMF_FEM_INPUT_SRC_B_31_0_FLD_VAL                 (10)
#define ARAD_PMF_FEM_INPUT_SRC_B_47_16_FLD_VAL                (11)
#define ARAD_PMF_FEM_INPUT_SRC_B_63_32_FLD_VAL                (12)
#define ARAD_PMF_FEM_INPUT_SRC_B_79_48_FLD_VAL                (13)
#define ARAD_PMF_FEM_INPUT_SRC_B_95_64_FLD_VAL                (14)
#define ARAD_PMF_FEM_INPUT_SRC_TCAM_0_FLD_VAL                 (0)
#define ARAD_PMF_FEM_INPUT_SRC_TCAM_1_FLD_VAL                 (1)
#define ARAD_PMF_FEM_INPUT_SRC_TCAM_2_FLD_VAL                 (2)
#define ARAD_PMF_FEM_INPUT_SRC_TCAM_3_FLD_VAL                 (3)
#define ARAD_PMF_FEM_INPUT_SRC_DIR_TBL_FLD_VAL                (4)
#define ARAD_PMF_FEM_INPUT_SRC_NOP_FLD_VAL                    (15)

#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_9_0_FLD_VAL             (0)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_19_10_FLD_VAL           (1)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_29_20_FLD_VAL           (2)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_A_39_30_FLD_VAL           (3)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_9_0_FLD_VAL             (4)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_19_10_FLD_VAL           (5)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_29_20_FLD_VAL           (6)
#define ARAD_PMF_DIRECT_TBL_KEY_SRC_B_39_30_FLD_VAL           (7)


#define ARAD_PMF_CE_SUB_HEADER_0_FLD_VAL                      (0)
#define ARAD_PMF_CE_SUB_HEADER_1_FLD_VAL                      (1)
#define ARAD_PMF_CE_SUB_HEADER_2_FLD_VAL                      (2)
#define ARAD_PMF_CE_SUB_HEADER_3_FLD_VAL                      (3)
#define ARAD_PMF_CE_SUB_HEADER_4_FLD_VAL                      (4)
#define ARAD_PMF_CE_SUB_HEADER_5_FLD_VAL                      (5)
#define ARAD_PMF_CE_SUB_HEADER_FWD_FLD_VAL                    (6)
#define ARAD_PMF_CE_SUB_HEADER_FWD_POST_FLD_VAL               (7)

#define ARAD_PMF_TCAM_KEY_SRC_UNDEF_FLD_VAL                   (0)
#define ARAD_PMF_TCAM_KEY_SRC_L2_FLD_VAL                      (1)
#define ARAD_PMF_TCAM_KEY_SRC_L3_IPV4_FLD_VAL                 (2)
#define ARAD_PMF_TCAM_KEY_SRC_L3_IPV6_FLD_VAL                 (3)
#define ARAD_PMF_TCAM_KEY_SRC_B_A_FLD_VAL                     (7)
#define ARAD_PMF_TCAM_KEY_SRC_A_71_0_FLD_VAL                  (8)
#define ARAD_PMF_TCAM_KEY_SRC_A_103_32_FLD_VAL                (9)
#define ARAD_PMF_TCAM_KEY_SRC_A_143_0_FLD_VAL                 (10)
#define ARAD_PMF_TCAM_KEY_SRC_A_175_32_FLD_VAL                (11)
#define ARAD_PMF_TCAM_KEY_SRC_B_71_0_FLD_VAL                  (12)
#define ARAD_PMF_TCAM_KEY_SRC_B_103_32_FLD_VAL                (13)
#define ARAD_PMF_TCAM_KEY_SRC_B_143_0_FLD_VAL                 (14)
#define ARAD_PMF_TCAM_KEY_SRC_B_175_32_FLD_VAL                (15)

#define ARAD_PMF_TCAM_FLD_L2_LLVP_LSB 											  (138)
#define ARAD_PMF_TCAM_FLD_L2_STAG_LSB                         (122)
#define ARAD_PMF_TCAM_FLD_L2_CTAG_IN_AC_LSB                   (106)
#define ARAD_PMF_TCAM_FLD_L2_SA_LSB                           (58)
#define ARAD_PMF_TCAM_FLD_L2_DA_LSB                           (10)
#define ARAD_PMF_TCAM_FLD_L2_ETHERTYPE_LSB                    (6)
#define ARAD_PMF_TCAM_FLD_L2_IN_PP_PORT_LSB                   (0)

#define ARAD_PMF_TCAM_FLD_L2_LLVP_SIZE 											  (4)
#define ARAD_PMF_TCAM_FLD_L2_STAG_SIZE                        (16)
#define ARAD_PMF_TCAM_FLD_L2_CTAG_IN_AC_SIZE                  (16)
#define ARAD_PMF_TCAM_FLD_L2_SA_SIZE                          (48)
#define ARAD_PMF_TCAM_FLD_L2_DA_SIZE                          (48)
#define ARAD_PMF_TCAM_FLD_L2_ETHERTYPE_SIZE                   (4)
#define ARAD_PMF_TCAM_FLD_L2_IN_PP_PORT_SIZE                  (6)

#define ARAD_PMF_FLD_IPV4_L4OPS_LSB                           (136)
#define ARAD_PMF_FLD_IPV4_NEXT_PRTCL_LSB                      (132)
#define ARAD_PMF_FLD_IPV4_DF_LSB                              (131)
#define ARAD_PMF_FLD_IPV4_MF_LSB                              (130)
#define ARAD_PMF_FLD_IPV4_FRAG_NON_0_LSB                      (129)
#define ARAD_PMF_FLD_IPV4_L4OPS_FLAG_LSB                      (128)
#define ARAD_PMF_FLD_IPV4_SIP_LSB                             (96)
#define ARAD_PMF_FLD_IPV4_DIP_LSB                             (64)
#define ARAD_PMF_FLD_IPV4_L4OPS_OPT_LSB                       (32)
#define ARAD_PMF_FLD_IPV4_SRC_PORT_LSB                        (48)
#define ARAD_PMF_FLD_IPV4_DEST_PORT_LSB                       (32)
#define ARAD_PMF_FLD_IPV4_TOS_LSB                             (24)
#define ARAD_PMF_FLD_IPV4_TCP_CTL_LSB                         (18)
#define ARAD_PMF_FLD_IPV4_IN_AC_VRF_LSB                       (0)
#define ARAD_PMF_FLD_IPV4_IN_PP_PORT_LSB                      (12)
#define ARAD_PMF_FLD_IPV4_IN_VID_LSB                          (0)

#define ARAD_PMF_FLD_IPV4_L4OPS_SIZE                          (7)
#define ARAD_PMF_FLD_IPV4_NEXT_PRTCL_SIZE                     (4)
#define ARAD_PMF_FLD_IPV4_DF_SIZE                             (1)
#define ARAD_PMF_FLD_IPV4_MF_SIZE                             (1)
#define ARAD_PMF_FLD_IPV4_FRAG_NON_0_SIZE                     (1)
#define ARAD_PMF_FLD_IPV4_L4OPS_FLAG_SIZE                     (1)
#define ARAD_PMF_FLD_IPV4_SIP_SIZE                            (32)
#define ARAD_PMF_FLD_IPV4_DIP_SIZE                            (32)
#define ARAD_PMF_FLD_IPV4_L4OPS_OPT_SIZE                      (16)
#define ARAD_PMF_FLD_IPV4_SRC_PORT_SIZE                       (16)
#define ARAD_PMF_FLD_IPV4_DEST_PORT_SIZE                      (16)
#define ARAD_PMF_FLD_IPV4_TOS_SIZE                            (8)
#define ARAD_PMF_FLD_IPV4_TCP_CTL_SIZE                        (6)
#define ARAD_PMF_FLD_IPV4_IN_AC_VRF_SIZE                      (16)
#define ARAD_PMF_FLD_IPV4_IN_PP_PORT_SIZE                     (6)
#define ARAD_PMF_FLD_IPV4_IN_VID_SIZE                         (12)

#define ARAD_PMF_FLD_IPV6_L4OPS_LSB                           (280)
#define ARAD_PMF_FLD_IPV6_SIP_HIGH_LSB                        (152 + 64)
#define ARAD_PMF_FLD_IPV6_SIP_LOW_LSB                         (152)
#define ARAD_PMF_FLD_IPV6_DIP_HIGH_LSB                        (24 + 64)
#define ARAD_PMF_FLD_IPV6_DIP_LOW_LSB                         (24)
#define ARAD_PMF_FLD_IPV6_NEXT_PRTCL_LSB                      (20)
#define ARAD_PMF_FLD_IPV6_PP_PORT_TCP_CTL_LSB                 (14)
#define ARAD_PMF_FLD_IPV6_IN_AC_VRF_LSB                       (0)

#define ARAD_PMF_FLD_IPV6_L4OPS_SIZE                          (8)
#define ARAD_PMF_FLD_IPV6_SIP_HIGH_SIZE                       (64)
#define ARAD_PMF_FLD_IPV6_SIP_LOW_SIZE                        (64)
#define ARAD_PMF_FLD_IPV6_DIP_HIGH_SIZE                       (64)
#define ARAD_PMF_FLD_IPV6_DIP_LOW_SIZE                        (64)
#define ARAD_PMF_FLD_IPV6_NEXT_PRTCL_SIZE                     (4)
#define ARAD_PMF_FLD_IPV6_PP_PORT_TCP_CTL_SIZE                (6)
#define ARAD_PMF_FLD_IPV6_IN_AC_VRF_SIZE                      (14)

/* 
 * Due to the TM application, do not set any TM port profile 
 * The TM programs are very low 
 */ 
#define ARAD_PMF_PORT_PROFILE_TM    (0) /* Cannot be used due to injected PTCH: TM processing on non-ITMH-ports */
#define ARAD_PMF_PORT_PROFILE_FTMH  (1)
#define ARAD_PMF_PORT_PROFILE_RAW   (2)
#define ARAD_PMF_PORT_PROFILE_TM_OAM_TEST   (4) /* Only for the PMF key selection */
#define ARAD_PMF_PORT_PROFILE_XGS_TM      (5)
#define ARAD_PMF_PORT_PROFILE_MH      (6)
#define ARAD_PMF_PORT_PROFILE_MIRROR_RAW      (3)



/* 
 * CPU to CPU stacking Walk-around: 
 * 1. The user sends a packet with Payload o ITMH o PTCH-2, 
 * where the destination of the ITMH is the specific stacking port, 
 * and the PTCH has to have PFQ0[0] (called 
 * Opaque-Attributes) set 
 * 2. In the ITMH PMF Program, the MSB of FTMH-Stacking-Extension 
 * is set 
 * 3. In the stacking program, if FTMH.Stacking-Extension.MSB is set, 
 * then add an additional FTMH with Out-LIF = Local-PTC
 */
#define ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED (1)

#ifdef ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED
#define ARAD_PMF_HEADER_PROFILE_CPU_TO_CPU_FTMH_RESERVED (11) /* use the default header profile of XGS, ITMH*/
#endif /* ARAD_PMF_CPU_TO_CPU_WA_STACKING_ENABLED */

/* Minimum border of the TM PMF-Program space */
#define ARAD_PMF_PROG_SELECT_TM_PMF_PGM_MIN (0)

/* 
 * OAM Mirror Walk-around: 
 * multiple identical packet types may arrive to the same port, 
 * defined as 
 * - FTMH packets. The destination is done according to the trap, 
 * but the bytes-to-remove and the LBP header profile must be 
 * the one of the stacking-TDM, so prefer redirect to the 
 * static PMF-Program 
 * - Ethernet, nothing to do. 
 * Solution: 
 * 0. All is done in case number of OAM Procesors is not Null 
 * 1. Differentiate betweent these packets according to the 
 * Mirror-Profile, mapped in LLR to different trap codes 
 * 2. The mapping is static at init, fixed trap codes 
 * 3. In PMF Program selection, insert in the high line
 * selection according to Trap code to redirect to the 
 * default FTMH program 
 */
#define ARAD_PMF_OAM_MIRROR_WA_ENABLED (1)
/* 
 * For OAM mirror Walk-around, for snooped packets, 
 * the user may want on the packet the original Out-LIF 
 * and the DSP-Extension. 
 * Due to the IPT stamping of the snoop code on the FTMH. 
 * Out-LIF field on snooped packet, the egress editor inverts 
 * the meaning between the DSP-Extension and the Out-LIF. 
 * At ingress PMF, the Out-LIF signal is extracted from the 
 * FTMH.DSP-Ext, and the DSP-Extension is extracted from 
 * the FTMH.Out-LIF. This DSP-Extension value (located in 
 * FTMH.Out-LIF) will be overridden by the snoop code in IPT, 
 * but it is not needed afterwards. 
 * This feature is enable by the SOC property 
 * custom_feature_egress_snooping_advanced=1
 */
#define ARAD_PMF_OAM_MIRROR_WA_SNOOP_OUT_LIF_IN_DSP_EXT_ENABLED (1)


/* 
 * 1588 OAM-TS over ITMH with User-Header Walk-around: 
 * 1. The user sends a packet with Payload o User-Header o 
 * OAM-TS o ITMH o PTCH-2, 
 * where the destination of the ITMH is of type Unicast, 
 * and the PTCH has to have its opaque attribute LSB set 
 * 2. In the ITMH PMF Program, another Header-Profile is 
 * selected (profile 1, not used except in custom52), 
 * where the User-Header is not built 
 */
#define ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED (1)
#ifdef ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED
#define ARAD_PMF_HEADER_PROFILE_OAM_TS_ITMH_USER_HEADER_WA (1) /* unused except for Multicast 17b */
#endif /* ARAD_PMF_OAM_TS_ITMH_USER_HEADER_WA_ENABLED */

/* number of programs */
#define ARAD_PMF_NOF_PROGS                (ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES)
/* number of data bases */
#define ARAD_PMF_NOF_DBS                  (ARAD_PMF_LOW_LEVEL_NOF_DBS)

/* Max number of keys per Database  */
#define ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX       (2)

/* number of FES per program */
#define SOC_DPP_DEFS_MAX_TCAM_NOF_LINES_IN_LONGS           ARAD_BIT_TO_U32(SOC_DPP_DEFS_MAX(NOF_TCAM_BIG_BANK_LINES))
#define ARAD_PMF_NOF_FES                  (ARAD_PMF_LOW_LEVEL_NOF_FESS)

#define ARAD_PMF_NOF_LINES_MAX_ALL_STAGES       SOC_DPP_DEFS_MAX(NOF_INGRESS_PMF_PROGRAM_SELECTION_LINES)
#define ARAD_PMF_GROUP_LEN                      (ARAD_BIT_TO_U32(SOC_PPC_FP_NOF_PFGS_ARAD))
#define ARAD_PMF_NOF_LINES_IN_LEVEL_MAX_ALL_STAGES      (ARAD_PMF_NOF_LINES_MAX_ALL_STAGES)
#define ARAD_PMF_NOF_LEVELS_MAX_ALL_STAGES              (ARAD_PMF_NOF_LINES_MAX_ALL_STAGES)

#define ARAD_PMF_SEL_LINE_NOF_BITS       (SOC_SAND_MAX(SOC_DPP_DEFS_MAX(IHB_FLP_PROGRAM_SELECTION_CAM_MASK_NOF_BITS), SOC_DPP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_MASK_NOF_BITS)))
#define ARAD_PMF_SEL_LINE_LEN                   ARAD_BIT_TO_U32(ARAD_PMF_SEL_LINE_NOF_BITS)

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
  *  Selector PD1.
  */
  ARAD_PMF_BANK_SEL_PD1 = 0,
  /*
   *  Selector PD2.
   */
  ARAD_PMF_BANK_SEL_PD2 = 1,
  /*
   *  Selector Sel3.
   */
  ARAD_PMF_BANK_SEL_SEL3 = 2,
  /*
   *  Selector Sel4.
   */
  ARAD_PMF_BANK_SEL_SEL4 = 3,
  /*
   *  Number of types in ARAD_PMF_BANK_SEL
   */
  ARAD_NOF_PMF_BANK_SELS = 4
}ARAD_PMF_BANK_SEL;

typedef enum
{
 /*
  *  Key-A.
  */
  ARAD_PMF_TCAM_KEY_TYPE_A = 0,
 /*
  *  Key-B.
  */
  ARAD_PMF_TCAM_KEY_TYPE_B = 1,
 /*
  *  Key-A and Key-B.
  */
  ARAD_PMF_TCAM_KEY_TYPE_A_B = 2,
  /*
   *  Number of types in ARAD_PMF_TCAM_KEY_TYPE
   */
   ARAD_NOF_PMF_TCAM_KEY_TYPES = 3
}ARAD_PMF_TCAM_KEY_TYPE;

typedef enum
{
 /*
  *  Key-A.
  */
  ARAD_PMF_TCAM_KEY_NDX_A = 0,
 /*
  *  Key-B.
  */
  ARAD_PMF_TCAM_KEY_NDX_B = 1,
  /*
   *  Number of types in ARAD_PMF_TCAM_KEY_NDX
   */
   ARAD_NOF_PMF_TCAM_KEY_NDXS = 2
}ARAD_PMF_TCAM_KEY_NDX;

/* Enumeration of the PMF Program types */
typedef enum
{
    ARAD_PMF_PGM_TYPE_RAW = 0,
	ARAD_PMF_PGM_TYPE_MIRROR_RAW = 1,  
    ARAD_PMF_PGM_TYPE_STACK = 2,
    ARAD_PMF_PGM_TYPE_MH = 3,
    ARAD_PMF_PGM_TYPE_XGS = 4,
    ARAD_PMF_PGM_TYPE_TM = 5,
    ARAD_PMF_PGM_TYPE_ETH = 6, /* Set as always last since it is not allocated like the others */
    ARAD_NOF_PMF_PGM_TYPES = ARAD_PMF_PGM_TYPE_ETH

} ARAD_PMF_PGM_TYPE;

/* Enumeration of the PMF Program selection groups */
typedef enum
{
    ARAD_PMF_PSL_TYPE_ETH = 0, 
    ARAD_PMF_PSL_TYPE_TM = 1,
    ARAD_NOF_PMF_PSL_TYPES = 2
} ARAD_PMF_PSL_TYPE;

typedef struct
{
  /*
   *  Used by instruction allocation code
   */
  uint8 is_used;

 /*
  * in which DB this CE is ued
  */
  uint8 db_id;

 /*
  * Number of lost bits in this CE
  */
  uint8 lsb;
 /*
  * MSB indicates the number of copied bits in this CE minus one
  */
  uint8 msb;

  /* Indicates the location inside the key - LSB or MSB */
  uint8 is_msb;

  /* In case of 320b, differentiate between the 2 keys */
  uint8 is_second_key;

  /* For this qualifier type, first bit (i.e., LSB) taken inside the qualifier (offst inside the qualifier)*/
  uint8 qual_lsb;
  /*
   * qual-type set by this CE
   */
  SOC_PPC_FP_QUAL_TYPE                qual_type;

} ARAD_PMF_CE;

typedef struct
{
  /*
   *  is in use
   */
  uint8 is_used;

 /*
  * in which DB this FES is used
  */
  uint8 db_id;
   /*
  * which entry uses this FES
  */
  uint8 entry_id;
  /*
   * action is performe by this FES
   */
  SOC_PPC_FP_ACTION_TYPE action_type;

} ARAD_PMF_FES;

typedef struct
{
  /*
   *  Used by instruction allocation code
   */
  uint32 ce[ARAD_PMF_NOF_PROGS][ARAD_PMF_NOF_CYCLES];

  uint32 key[ARAD_PMF_NOF_PROGS][ARAD_PMF_NOF_CYCLES];

  uint32 progs[ARAD_BIT_TO_U32(ARAD_PMF_NOF_PROGS)];

  uint8  fem_pgm_per_pmf_prog[ARAD_PMF_NOF_PROGS];

} ARAD_PMF_RESOURCE;

typedef struct
{
  /*
   *  relevant programs to update for the DB, bitmap
   */
  uint32 progs[ARAD_BIT_TO_U32(ARAD_PMF_NOF_PROGS)]; /* ok */

  /* which cycle is used for this DB - same cycle for any program due to the deterministic allocation */
  uint32 prog_used_cycle_bmp;

  /* which key is used in each program for this DB */
  uint8 used_key[ARAD_PMF_NOF_PROGS][ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX];

  /* priority of this Database */
  uint32 prio;

  /* is-320b  */
  uint32 is_320b;

  /* Which half-keys are taken  */
  uint32 alloc_place;

 /* valid Database */
  uint8 valid; 

    /* which key is used in each program for this DB */
  uint32 nof_bits_zone[ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX][2];

  /* 
   * Cascaded key: assign it before committing to HW and SW 
   * Not assigned if equal to 0 
   * Otherwise: Key+1 (1..4 in Arad) 
   * Relevant only for 2nd cycle cascaded DBs 
   */
  uint8 cascaded_key; 

  /*relevant only in case of direct table in kaps*/
  uint32 kaps_db_id; 

} ARAD_PMF_DB_INFO;

typedef struct
{
  /*
   *	Indicates if the number of entries already created
   */
  uint32 nof_db_entries;

} ARAD_FP_ENTRY;


typedef struct
{
  /*
   *  IRPP field
   */
  SOC_PPC_FP_QUAL_TYPE            irpp_field;

  /* Is-LSB group */
  uint32 is_lsb;

  /* Is-MSB group */
  uint32 is_msb;

  /* Qualifier LSB in the buffer */
  uint32 buffer_lsb;

  /* Qualifier number of bits by default */
  uint32 qual_nof_bits;

} ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES;

typedef struct
{
    /*
     *  IRPP field
     */
    SOC_PPC_FP_QUAL_TYPE            irpp_field;

    uint32 msb;
    uint32 lsb;
    uint32 base0;
    uint32 base1;

} ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL;

typedef struct
{
  /*
   *  Qualifier info
   */
  ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES  info;
  /* 
   * Qualifer signal
   */
  ARAD_PMF_CE_IRPP_QUALIFIER_SIGNAL      signal;

  /* Valid from Jericho: LSB or MSB buffer for HW configuration only */
  uint32  hw_buffer_jericho; 

} ARAD_PMF_CE_IRPP_QUALIFIER_INFO;

#define ARAD_PMF_CE_SUB_HEADER_0                          SOC_TMC_PMF_CE_SUB_HEADER_0
#define ARAD_PMF_CE_SUB_HEADER_1                          SOC_TMC_PMF_CE_SUB_HEADER_1
#define ARAD_PMF_CE_SUB_HEADER_2                          SOC_TMC_PMF_CE_SUB_HEADER_2
#define ARAD_PMF_CE_SUB_HEADER_3                          SOC_TMC_PMF_CE_SUB_HEADER_3
#define ARAD_PMF_CE_SUB_HEADER_4                          SOC_TMC_PMF_CE_SUB_HEADER_4
#define ARAD_PMF_CE_SUB_HEADER_5                          SOC_TMC_PMF_CE_SUB_HEADER_5
#define ARAD_PMF_CE_SUB_HEADER_FWD                        SOC_TMC_PMF_CE_SUB_HEADER_FWD
#define ARAD_PMF_CE_SUB_HEADER_FWD_POST                   SOC_TMC_PMF_CE_SUB_HEADER_FWD_POST
#define ARAD_NOF_PMF_CE_SUB_HEADERS                       SOC_TMC_NOF_PMF_CE_SUB_HEADERS
typedef SOC_TMC_PMF_CE_SUB_HEADER                           	ARAD_PMF_CE_SUB_HEADER;

typedef struct
{
  SOC_PPC_FP_QUAL_TYPE   qual_type;
  int32            msb;
  int32            lsb;
  ARAD_PMF_CE_SUB_HEADER header_ndx_0; /* Possible header indices */
  ARAD_PMF_CE_SUB_HEADER header_ndx_1;
} ARAD_PMF_CE_HEADER_QUAL_INFO;

/* 
 * this struct is used for user defined qualifiers based on predefined qualifiers 
 * (may be from headers of from internal fields)
 */
typedef struct
{
	uint8 is_header_qual;
	ARAD_PMF_CE_HEADER_QUAL_INFO header_qual_info;
	ARAD_PMF_CE_IRPP_QUALIFIER_INFO irpp_qual_info[2]; /* one for msb instruction, one for lsb instruction */
	SOC_PPC_FP_DATABASE_STAGE stage;
} ARAD_PMF_CE_QUAL_INFO;

typedef struct
{
  /*
   *  Indicate the Database type (TCAM, Direct Table, Direct Extraction, or not exist)
   */
  SOC_PPC_FP_DATABASE_INFO db_info[SOC_PPC_FP_NOF_DBS];

  /*
   *  Indicates if the number of entries already created
   */
  ARAD_FP_ENTRY db_entries[ARAD_PMF_NOF_DBS];
  /*
   * Packet Format Groups
   */
  SOC_PPC_PMF_PFG_INFO pfg_info[SOC_PPC_FP_NOF_PFGS_ARAD];

  /*
   * User-Defined Fields
   */
  ARAD_PMF_CE_QUAL_INFO  udf[ARAD_PP_FP_NOF_HDR_USER_DEFS];

  /*
   * Number of VLAN Tags in the inner Ethernet header per PFG
   */
  uint32 inner_eth_nof_tags[SOC_PPC_FP_NOF_PFGS_ARAD];
  /*
   * Key change size in bits. Default: 12 bits
   */
  uint8 key_change_size;

  SHR_BITDCL entry_bitmap[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS][SOC_DPP_DEFS_MAX_TCAM_NOF_LINES_IN_LONGS];

} ARAD_FP_INFO;




typedef struct
{
  /*
   *  number of resereved lines, in Program selection DB
   *  0 - Ehternet, 1 - TM
   */
   uint32 nof_reserved_lines[ARAD_NOF_PMF_PSL_TYPES];
  /*
   *  Default Ethernet PMF-Program.
   *  It can change at each DB creation / destroy
   */
   uint32 pmf_pgm_default[ARAD_NOF_PMF_PSL_TYPES];
} ARAD_PMF_SEL_INIT_INFO;


/*
 * PSL info
 */
typedef struct
{
  /*
   *  bitmap mask, 1 to mask
   */
  uint32 mask[ARAD_PMF_SEL_LINE_LEN];
  /*
   *  bitmap value, if masked value has to be zero.
   */
  uint32 value[ARAD_PMF_SEL_LINE_LEN];

} ARAD_PMF_SEL_GROUP;

typedef struct ARAD_PMF_PSL_LINE_INFO_s
{
  /*
   *  group represented by this line:
   *  buffer itself (value and mask)
   */
  ARAD_PMF_SEL_GROUP group; 
  /*
   *  Bitmap of groups that intersected in this line
   */
  uint32 groups[ARAD_PMF_GROUP_LEN];
  /*
   *  Program ID this line associated to.
   *  If ARAD_PMF_PROG_INVALID, then program invalid
   */
  uint32 prog_id;
  /* Define line index in case of presel mgmt advanced mode*/
  uint32 line_id;
  /* Define the action according to ARAD_PMF_SEL_LINE_ACTION */
  uint32 flags;

} ARAD_PMF_PSL_LINE_INFO;

typedef struct ARAD_PMF_PSL_LEVEL_INFO_s
{
  /*
   *  index of first line belongs to this level
   *  In practice, the lowest (high value) line (closest to static non-Eth lines)
   */
  uint32 first_index;
  /*
   *  index of last line belongs to this level
   *  In practice, the highest (lowest value) line (closest to 0)
   */
  uint32 last_index;

  ARAD_PMF_PSL_LINE_INFO lines[ARAD_PMF_NOF_LINES_IN_LEVEL_MAX_ALL_STAGES];
  /*
   *  number of lines in this level not include new lines
   */
  uint32 nof_lines;
  /*
   *  number of new lines in this level from last calculation
   */
  uint32 nof_new_lines;
  /*
   *  number of new lines in this level from last calculation
   */
  uint32 nof_removed_lines;
  /*
   *  level index
   */
  uint32 level_index;

} ARAD_PMF_PSL_LEVEL_INFO;



/* } */

uint32
  arad_pmf_low_level_init_unsafe(
    SOC_SAND_IN  int                                 unit
  );

uint32
  arad_pmf_low_level_pgm_set(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  uint32                          pmf_pgm_ndx,
    SOC_SAND_IN  ARAD_PMF_PGM_TYPE              header_type
   );

uint32 arad_pmf_low_level_mirror_raw_pgm_update(int unit, int nof_reserved_lines);

uint32
  arad_pmf_low_level_stack_pgm_set_unsafe(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32                pmf_pgm_ndx,
    SOC_SAND_IN  uint8              is_for_oam_snoop_was
  );

uint32
 arad_pmf_low_level_pgm_counter_set_unsafe(
   SOC_SAND_IN  int                      unit,
   SOC_SAND_IN  int                      command_id,
   SOC_SAND_IN  SOC_TMC_CNT_MODE_EG_TYPE eg_type,
   SOC_SAND_IN  int                      counter_base);
uint32
 arad_pmf_low_level_pgm_counter_get_unsafe(
   SOC_SAND_IN  int                         unit,
   SOC_SAND_IN  int                         command_id,
   SOC_SAND_OUT SOC_TMC_CNT_MODE_EG_TYPE    *eg_type,
   SOC_SAND_OUT int                         *counter_base);
#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_PMF_LOW_LEVEL_INCLUDED__*/
#endif


