/*
 * $Id: arad_pp_flp_init.c,v 1.119 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>
#include <soc/dcmn/utils.h>

#include <soc/dpp/drv.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_isem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/arad_tcam.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <soc/dpp/ARAD/arad_pmf_low_level.h>
#include <soc/dpp/ARAD/arad_parser.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ilm.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_bmact.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_dbal.h>

#include <soc/register.h>

#include <soc/dpp/ARAD/arad_general.h>

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#endif

/* } */
/*************
 * DEFINES   *
 *************/
/* { */


#define ARAD_PP_FLP_INIT_PRINT_ADVANCE                                                               soc_sand_os_printf

#define ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP         (0x060)
#define ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID    (0x018)
#define ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP     (0x004)
#define ARAD_PP_FLP_QLFR_ETH_MASK_INNER_TPID    (0x003)
#define ARAD_PP_FLP_QLFR_ETH_MASK_NEXT_PROTOCOL (0x780)

/* Next protocol */
#define ARAD_PP_FLP_QLFR_ETH_NEXT_PROTOCOL_IPV4 (0xD << 7)


#define ARAD_PP_FLP_LKP_KEY_SELECT_A_KEY_HW_VAL  (0x0)
#define ARAD_PP_FLP_LKP_KEY_SELECT_B_KEY_HW_VAL  (0x1)

#define ARAD_PP_FLP_INSTRUCTION_0_16B 0
#define ARAD_PP_FLP_INSTRUCTION_1_16B 1
#define ARAD_PP_FLP_INSTRUCTION_2_16B 2
#define ARAD_PP_FLP_INSTRUCTION_3_32B 3
#define ARAD_PP_FLP_INSTRUCTION_4_32B 4
#define ARAD_PP_FLP_INSTRUCTION_5_32B 5

#define ARAD_PP_FLP_INSTRUCTION_0_TO_5 0
#define ARAD_PP_FLP_INSTRUCTION_6_TO_11 1

#define ARAD_PP_FLP_SERVICE_TYPE_AC_P2P_AC2AC  0
#define ARAD_PP_FLP_SERVICE_TYPE_AC_P2P_AC2PWE 1
#define ARAD_PP_FLP_SERVICE_TYPE_AC_P2P_AC2PBB 2
#define ARAD_PP_FLP_SERVICE_TYPE_AC_MP 3
#define ARAD_PP_FLP_SERVICE_TYPE_ISID_P2P 4
#define ARAD_PP_FLP_SERVICE_TYPE_ISID_MP 5
#define ARAD_PP_FLP_SERVICE_TYPE_TRILL_IP_PWE_VRF_LSP_MP 6
#define ARAD_PP_FLP_SERVICE_TYPE_PWE_P2P 7

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(bitmap,index) \
  { \
    if(!COMPILER_64_BITTEST(bitmap,index) && (index<SOC_DPP_DEFS_GET(unit, nof_flp_programs))) { \
        COMPILER_64_BITSET(bitmap,index); \
    } \
    else { \
        SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 5500, exit); \
    } \
  }

#define ARAD_PP_FLP_SW_DB_FLP_PROGRAM_ID_SET(unit, prog_id, app_id) \
  { \
    if ((prog_id) >= (SOC_DPP_DEFS_GET(unit, nof_flp_programs))) \
    { \
      SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 8100, exit); \
    } \
    res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.set((unit), (prog_id), (app_id)); \
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit); \
  }

#define SEPARATE_IPV4_IPV6_RPF_ENABLE ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "separate_ipv4_ipv6_rpf_enable", 0)))

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef struct
{
    /* FLP instructions */
    uint32 instruction;

    /* Instruction ID. Range: 0 - 11 (ARAD). */
    uint8 is_inst_6_11;

    /* Instruction ID. Range: 0 - 5 (ARAD). */
    uint8 id;

} ARAD_FLP_CUSTOM_LPM_INSTRUCTION_INFO;

typedef struct
{
    /* key index: 0 - key A; 1 - key B; 2 - key C */
    uint8 key;

    /* key OR value, AND value is always 0 */
    uint8 key_or_value;

    /* number of instructions */
    uint8 inst_num;

    /* instructions */
    ARAD_FLP_CUSTOM_LPM_INSTRUCTION_INFO inst[12];

} ARAD_FLP_CUSTOM_LPM_KEY_CONSTR;

typedef struct
{
    /* app ID */
    uint8 app_id;

    /* number of lookups, up to two lookups in LPM */
    uint8 lookup_num;

    /* if only one lookup, specify which lookup to use */
    uint8 lookup_to_use;

    /* instructions */
    ARAD_FLP_CUSTOM_LPM_KEY_CONSTR key[2];

} ARAD_FLP_CUSTOM_LPM_LOOKUP_INFO;


/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

STATIC
    ARAD_FLP_CUSTOM_LPM_LOOKUP_INFO
        Arad_flp_custom_lpm_lookup_info[] = {
#if 0
        {
          PROG_FLP_ETHERNET_MAC_IN_MAC,
          /* lookup_num, lookup_to_use, {, {}, ... } */
          2, 0,
              {
                  { 2, 0, 2, /*  key C, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
        },
        {
          PROG_FLP_MAC_IN_MAC_AFTER_TERMINATIOM,
          /* lookup_num, lookup_to_use, {, {}, ... } */
          2, 0,
              {
                  { 2, 0, 2, /*  key C, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
        },
#endif
        {
          PROG_FLP_ETHERNET_ING_LEARN,
          /* lookup_num, {, {}, ... } */
          2, 0,
              {
                  { 2, 0, 2, /*  key index, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
        },
        {
          PROG_FLP_IPV4UC_RPF,
          /* lookup_num, {, {}, ... } */
          2, 0,
              {
                  { 2, 0, 2, /*  key index, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } 
              } 
        },
        {
          PROG_FLP_IPV4UC,
          /* lookup_num, {, {}, ... } */
          2, 0,
              {
                  { 2, 0, 2, /*  key index, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  }
              }
        },
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
        {
          PROG_FLP_IPV6UC_RPF,
          /* lookup_num, {, {}, ... } */
          2, 0,
              {
                  { 1, 0, 2, /*  key B, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 1, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } 
              } 
         },
#endif
        {
          PROG_FLP_IPV6UC,
          /* lookup_num, {, {}, ... } */
          2, 0,
              {
                  { 1, 0, 2, /*  key B, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 1, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } 
              } 
         },
         {
           PROG_FLP_LSR,
           /* lookup_num, lookup_to_use, {, {}, ... } */
           2, 0,
              {
                  { 1, 0, 2, /*  key index, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 1, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
          },
          {
            PROG_FLP_IPV4COMPMC_WITH_RPF,
            /* lookup_num, lookup_to_use, {, {}, ... } */
            2, 0,
              {
                  { 2, 0, 2, /*  key C, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
          },
          {
            PROG_FLP_IPV6MC,
            /* lookup_num, {, {}, ... } */
            2, 0,
              {
                  { 1, 0, 2, /*  key B, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 1, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } 
              } 
          },
          {
            PROG_FLP_P2P,
            /* lookup_num, {, {}, ... } */
            2, 0,
              {
                  { 1, 0, 2, /*  key B, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } ,
                  { 1, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_2_16B},
                      }
                  } 
              } 
          },
          {
            PROG_FLP_IPV4UC_PUBLIC,
            /* lookup_num, {, {}, ... } */
            2, 0,
              {
                  { 2, 0, 2, /*  key C, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_0_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_0_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_0_TO_5, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
          },
          {
            PROG_FLP_IPV4MC_BRIDGE,
            /* lookup_num, lookup_to_use, {, {}, ... } */
            2, 0,
              {
                 { 2, 0, 2, /*  key C, key OR val, number of instructions */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B}, /* instruction, is_inst_6_11, id */
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } ,
                  { 2, 2, 2, /* if the 2nd lookup uses the same key as first lookup, 2nd lookup instructions are ignored */
                      {
                          { ARAD_PP_FLP_16B_INST_P6_VT_LOOKUP0_PAYLOAD_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_0_16B},
                          { ARAD_PP_FLP_16B_INST_P6_FORWARDING_ACTION_TC_D, ARAD_PP_FLP_INSTRUCTION_6_TO_11, ARAD_PP_FLP_INSTRUCTION_1_16B},
                      }
                  } 
              } 
          },
    };
/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*
* function: first_lem_lkup_sa_fid_search_set 
* get as input a sw object of type ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA, 
* configures only the lem 1st lookup to be (sa,fid) lookup for learning 
*/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
STATIC uint32 first_lem_lkup_sa_fid_search_set(int unit, ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA* flp_lookups_tbl){
	if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "l3_learn_force_in_fwd", 0)) {
	   /* enable fid,sa lookup into the lem, using constant key*/
	   flp_lookups_tbl->lem_1st_lkp_valid     = 1;
	   flp_lookups_tbl->lem_1st_lkp_key_select = 3; /* special constant key */
	   flp_lookups_tbl->lem_1st_lkp_key_type   = 0x1; /* 1 source lookup */
	   flp_lookups_tbl->lem_1st_lkp_and_value  = 0x0;
	   flp_lookups_tbl->lem_1st_lkp_or_value   = 0x0;
	}
	return BCM_E_NONE;
}
#else
#define first_lem_lkup_sa_fid_search_set(unit,flp_lookups_tbl) BCM_E_NONE
#endif

/*
* function: process_tbl_learn_enable_set 
* sets learn enable bit in the process table entry of the program.  
*/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
STATIC uint32 process_tbl_learn_enable_set(int unit, ARAD_PP_IHB_FLP_PROCESS_TBL_DATA *flp_process_tbl) {
	if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "l3_learn_force_in_fwd", 0)) {
		flp_process_tbl->learn_enable = 0x1;
	}
	return BCM_E_NONE;
}
#else
#define process_tbl_learn_enable_set(unit,flp_process_tbl) BCM_E_NONE
#endif

STATIC uint32
   arad_pp_flp_pwe_gre_progs_init(
     int unit
   );

STATIC uint32
   arad_pp_flp_fcoe_progs_init(
     int unit
   );

STATIC uint32
   arad_pp_flp_ipmc_bidir_progs_init(
     int unit
   );

STATIC uint32
   arad_pp_flp_vmac_progs_init(
     int unit,
    uint8 sa_auth_enabled,
    uint8 slb_enabled
   );

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
STATIC uint32
   arad_pp_flp_ipv6uc_with_rpf_prog_init(
     int unit
   );
#endif

STATIC uint32
  arad_pp_flp_pon_arp_prog_init(
    int unit,
    uint8 sa_auth_enabled,
    uint8 slb_enabled
  );

STATIC uint32
  arad_pp_flp_lpm_custom_lookup_enable(
     int unit,
     uint8 app_id
   );

STATIC uint32 arad_pp_flp_program_disable(
        int unit,
        uint32 prog_id
   )
{
  uint32
    res,
    indx;
    
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA    prog_selection_cam_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (indx = 0; indx < SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines); ++indx) {
      res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, indx, &prog_selection_cam_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
            
      if (prog_selection_cam_tbl.program == prog_id) {
          if (prog_selection_cam_tbl.valid != 0) {
              prog_selection_cam_tbl.valid = 0;
              res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, indx, &prog_selection_cam_tbl);
              SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
              break;
          }
      }
  }
  if (indx == SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines)) {
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
  }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_program_disable", 0, 0);
}

/*
*   function: arad_pp_flp_set_app_id_and_get_cam_sel(...)
*   inputs:
*   sw_prog_id - prod_id (PROG_FLP_XXX)
*   is_static_prog - indicates if hw_prog_id = sw_prog_id
*   update_hw_prog_id - indicate if new hw_prog_id should be allocated
*                        hw_prog_id: one of SOC_DPP_DEFS_GET(unit, nof_flp_programs)
*   outputs:
*   cam_sel_id - selection ID
*   hw_prog_id - prog_id, used only if (is_static_prog=FALSE && update_hw_prog_id = TRUE)    
*/
STATIC uint32 arad_pp_flp_set_app_id_and_get_cam_sel(
   int    unit,
   uint32 sw_prog_id,
   int8   is_static_prog,
   int8   update_hw_prog_id,
   int32 *cam_sel_id,
   int32 *hw_prog_id)
{
  int i;
  uint32 
      res;
  int nof_flp_hw_programs = SOC_DPP_DEFS_GET(unit, nof_flp_programs);
  int nof_flp_selections  = SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines);

  uint64 hw_prog_bitmap;
  uint64 prog_sel_bitmap;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(cam_sel_id);

  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_hw_prog_id_bitmap.get(unit,&hw_prog_bitmap);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_prog_select_id_bitmap.get(unit,&prog_sel_bitmap);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* Allocate HW prog id:
   * In case of static sw prog. - verify its bit is set
   * In case of dynamic sw prog.- allocate new hw using the flp_hw_prog_id_bitmap
   */
  if (update_hw_prog_id) {   
      if (is_static_prog) {
          if (!COMPILER_64_BITTEST(hw_prog_bitmap, sw_prog_id)) {
              /* error - static prog has not been set*/
              SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 5500, exit);
          }
          ARAD_PP_FLP_SW_DB_FLP_PROGRAM_ID_SET(unit, sw_prog_id, sw_prog_id);
      }
      else {
          SOC_SAND_CHECK_NULL_INPUT(hw_prog_id);
          for (i = 0; i < nof_flp_hw_programs; i++) {
              if (!COMPILER_64_BITTEST(hw_prog_bitmap,i)) {
                  COMPILER_64_BITSET(hw_prog_bitmap,i);
                  *hw_prog_id = i;
                  break;
              }
              if (i == nof_flp_hw_programs-1) {
                  /* error - can not allocate hw_prog*/
                  SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 5500, exit);
              }
          }
          ARAD_PP_FLP_SW_DB_FLP_PROGRAM_ID_SET(unit, *hw_prog_id, sw_prog_id);
      }
  }

  /* Allocate prog selection id:
   * dynamic allocation for all sw prog.- allocate new hw using the flp_prog_select_id_bitmap
   */
  for (i = 0; i < nof_flp_selections; i++) {
      if (!COMPILER_64_BITTEST(prog_sel_bitmap,i)) {
          COMPILER_64_BITSET(prog_sel_bitmap,i);
          *cam_sel_id = i;
          break;
      }
      if (i == nof_flp_selections-1) {
          /* error - can not allocate prog selection id*/
          SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 5500, exit);
      }
  }

  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_hw_prog_id_bitmap.set(unit,hw_prog_bitmap);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_prog_select_id_bitmap.set(unit,prog_sel_bitmap);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_set_app_id_and_get_cam_sel", sw_prog_id, 0);
}


STATIC uint32 arad_pp_flp_get_cam_sel_list_by_prog_id(
   int    unit,
   uint32 prog_id,
   uint64 *cam_sel_list,
   int    *nof_selections)
{
  int i, counter;
  uint32 
      res;
  int nof_flp_selections  = SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines);
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA prog_selection_cam_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(cam_sel_list);
  SOC_SAND_CHECK_NULL_INPUT(nof_selections);

  counter = 0;
  for (i = 0; i < nof_flp_selections; i++) {
      res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, i, &prog_selection_cam_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      if ((prog_selection_cam_tbl.program == prog_id)&&(prog_selection_cam_tbl.valid == 1)) {
          COMPILER_64_BITSET(*cam_sel_list, i);
          counter++;
      }
  }

  *nof_selections = counter;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_get_cam_sel_list_by_prog_id", prog_id, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_key_program_tm(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  if (SOC_DPP_PP_ENABLE(unit)) {
      res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_TM,TRUE,TRUE,&cam_sel_id,NULL);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  } else {
      cam_sel_id = PROG_FLP_TM;
  }

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.packet_format_code  = 0x20; /* 6'b10_0000;*/
  prog_selection_cam_tbl.packet_format_code_mask  = 0x1F; /* 6'b01_1111;*/

  prog_selection_cam_tbl.program = PROG_FLP_TM;
  prog_selection_cam_tbl.valid = 1;

  if (SOC_IS_ARADPLUS_A0(unit) && SOC_DPP_CONFIG(unit)->pp.oam_statistics){
      prog_selection_cam_tbl.vt_lookup_0_found =0;
      prog_selection_cam_tbl.vt_lookup_0_found_mask =0;
      prog_selection_cam_tbl.vt_lookup_1_found =0;
      prog_selection_cam_tbl.vt_lookup_1_found_mask =0;
  }


  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_key_program_tm", 0, 0);
}

/*
 * Program selection for VPWS tagged mode 
 * There are two programs here - one for single tag and one for double tag 
 * Selection is mainly according to: 
 *      service type (PWE P2P is used for both PWE2AC and PWE2PWE)
 *      forwarding_header_qualifier (8 LSBs are Ethernet Tag Format: {Outer Tag[2], IsPriority[1], Inner Tag[2]}
 * These programs must be loaded after the P2P program as they rely on no match n the P2P selection
*/
uint32
   arad_pp_flp_prog_sel_cam_vpws_tagged(
     int unit
   )
{
    uint32 res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA prog_selection_cam_tbl;
    int prog_id;
    uint8 is_single_tag;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Single tag must be first because its program selection is more inclusive */
    for (is_single_tag = 1; is_single_tag!=(uint8)(-1); is_single_tag--) {
        /* For static, program id and app id is same */
        prog_id = is_single_tag ? PROG_FLP_VPWS_TAGGED_SINGLE_TAG:PROG_FLP_VPWS_TAGGED_DOUBLE_TAG;
        res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_id,TRUE,TRUE,&cam_sel_id,NULL);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        prog_selection_cam_tbl.service_type = ARAD_PP_FLP_SERVICE_TYPE_PWE_P2P;
        prog_selection_cam_tbl.service_type_mask = 0;
        prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
        prog_selection_cam_tbl.port_profile = 0;
        prog_selection_cam_tbl.ptc_profile  = 0;
        prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
        prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
        prog_selection_cam_tbl.ptc_profile_mask  = 0x03;
        prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
        prog_selection_cam_tbl.forwarding_code_mask = 0x00;
        prog_selection_cam_tbl.cos_profile_mask = 0x3F;
        prog_selection_cam_tbl.valid = 1;

        if (is_single_tag) {
            /* PWE Un-Tagged is handled in P2P program selection with higher priority */
            /* So in case Inner-Tag is 0 it must be Single Tagged packet */
            /* In case Inner-Tag is not 0 only option left is Double-Tagged */
            prog_selection_cam_tbl.forwarding_header_qualifier = 0; /*Outer Tag (2 MSBs) is dont care, Inner tag (2 LSBs) is None, Outer tag is priority is masked */
            prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID |
                                                                   ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_NEXT_PROTOCOL);
        }

        prog_selection_cam_tbl.program = prog_id;

        res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit,cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit); 
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_vpws_tagged", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_p2p(
     int unit
   )
{
    uint32 res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA prog_selection_cam_tbl;
    uint32 i = 0;
    int service_type[4]      = {ARAD_PP_FLP_SERVICE_TYPE_AC_P2P_AC2AC, /*AC to PWE & AC to AC for b00X*/
                                ARAD_PP_FLP_SERVICE_TYPE_AC_P2P_AC2PBB, 
                                ARAD_PP_FLP_SERVICE_TYPE_ISID_P2P, 
                                ARAD_PP_FLP_SERVICE_TYPE_PWE_P2P}; 
    int service_type_mask[4] = {1, 0, 0, 0};
    /* Only need AC2AC for P2P service in PON application */
    int num_of_selections = (SOC_DPP_CONFIG(unit)->pp.pon_application_enable ? 1 : 4);
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* when ARAD_KBP_ENABLE_P2P_EXTENDED is enabled, we will use only service type 000 and 001 (AC to PWE & AC to AC) */
    if (ARAD_KBP_ENABLE_P2P_EXTENDED) {
        num_of_selections = 1;
    }
#endif

    for (i = 0; i < num_of_selections; i++) {
        if (i == 0) {
            res = arad_pp_flp_set_app_id_and_get_cam_sel(unit, PROG_FLP_P2P, TRUE, TRUE, &cam_sel_id, NULL);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
        else {
            res = arad_pp_flp_set_app_id_and_get_cam_sel(unit, PROG_FLP_P2P+i, FALSE, FALSE, &cam_sel_id, NULL);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
        
        res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        prog_selection_cam_tbl.service_type = service_type[i];
        prog_selection_cam_tbl.service_type_mask = service_type_mask[i];    
        prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
        prog_selection_cam_tbl.port_profile = 0;
        prog_selection_cam_tbl.ptc_profile  = 0;
        prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
        prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
        prog_selection_cam_tbl.ptc_profile_mask  = 0x03;    
        prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
        prog_selection_cam_tbl.forwarding_code_mask = 0x00;
        prog_selection_cam_tbl.cos_profile_mask = 0x3F;
        prog_selection_cam_tbl.program = PROG_FLP_P2P;
        prog_selection_cam_tbl.valid = 1;

        if ((service_type[i] == ARAD_PP_FLP_SERVICE_TYPE_PWE_P2P) && soc_property_get(unit, spn_VPWS_TAGGED_MODE, 0)) { 
            /* PWE Tagged is handled in separate program selection with lower priority */
            prog_selection_cam_tbl.forwarding_header_qualifier = 0; /* Outer Tag (2 MSBs) and Inner tag (2 LSBs) are None, Outer tag is priority is masked */
            prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_NEXT_PROTOCOL);
        }
        res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_p2p", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ethernet_tk_epon_uni_v4(
     int unit
   )
{
  uint32
    res,
    program_id;
  int 
    i;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (i = 0; i < 2; i++) { /* DHCP, static */
    /* Don't install SAV DHCP because prefix value is conflict */
    if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable && (i==0)) {
      continue;
    }
    program_id = (i == 0) ? PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP:PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC; 
         
    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,program_id,TRUE,TRUE,&cam_sel_id,NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    prog_selection_cam_tbl.packet_format_code = ARAD_PARSER_PFC_IPV4_ETH;
    prog_selection_cam_tbl.packet_format_code_mask = 0x18 /* 6'b011000 */;
    prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
    prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
    prog_selection_cam_tbl.port_profile = 0x0;
    prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
    prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_PON;
    prog_selection_cam_tbl.ptc_profile_mask = 0x0;
    prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;  
    prog_selection_cam_tbl.forwarding_code_mask = 0x00;    
    prog_selection_cam_tbl.cos_profile = SOC_PPC_FLP_COS_PROFILE_ANTI_SPOOFING;
    prog_selection_cam_tbl.cos_profile_mask = 0x1F; /* 6'b011111 */
    prog_selection_cam_tbl.tt_lookup_0_found = (program_id == PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP) ? 0x1:0x0;
    prog_selection_cam_tbl.tt_lookup_0_found_mask = 0x0;
    prog_selection_cam_tbl.program = program_id;
    prog_selection_cam_tbl.valid = 1;
    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_tk_epon_uni_v4", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ethernet_tk_epon_uni_v6(
     int unit
   )
{
  uint32
    res,
    program_id;
  int 
    i;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  for (i = 0; i < 2; i++) { /* DHCP, static */
    /* Don't install SAV DHCP because prefix value is conflict */
    if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable && (i==0)) {
      continue;
    }
    program_id = (i == 0) ? PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP:PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC; 
         
    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,program_id,TRUE,TRUE,&cam_sel_id, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    prog_selection_cam_tbl.packet_format_code = ARAD_PARSER_PFC_IPV6_ETH;
    prog_selection_cam_tbl.packet_format_code_mask = 0x18 /* 6'b011000 */;
    prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
    prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
    prog_selection_cam_tbl.port_profile = 0x0;
    prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
    prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_PON;
    prog_selection_cam_tbl.ptc_profile_mask = 0x0;
    prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;  
    prog_selection_cam_tbl.forwarding_code_mask = 0x00;
    prog_selection_cam_tbl.cos_profile = SOC_PPC_FLP_COS_PROFILE_ANTI_SPOOFING;
    prog_selection_cam_tbl.cos_profile_mask = 0x1F; /* 6'b011111 */
    if (program_id == PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP){
      prog_selection_cam_tbl.tt_lookup_0_found = 0x1;
      prog_selection_cam_tbl.tt_lookup_0_found_mask = 0x0;
    } else {
      if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
        prog_selection_cam_tbl.tt_lookup_1_found = 0x1;
        prog_selection_cam_tbl.tt_lookup_1_found_mask = 0x0;
      } else {
        prog_selection_cam_tbl.tt_lookup_0_found = 0x0;
        prog_selection_cam_tbl.tt_lookup_0_found_mask = 0x0;
      }
    }

    prog_selection_cam_tbl.program = program_id;
    prog_selection_cam_tbl.valid = 1;
    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_tk_epon_uni_v6", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_pon_arp_downstream(
    int unit,
    int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_PON_ARP_DOWNSTREAM,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
   
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    
  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_NONE;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.ll_is_arp        = 1;
  prog_selection_cam_tbl.cos_profile      = 0x0;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask = 0x00;
  prog_selection_cam_tbl.ptc_profile_mask = 0x0;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.ll_is_arp_mask        = 0;
  prog_selection_cam_tbl.cos_profile_mask = 0x3F; /* 6'b011111 */
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.port_profile = 0x0;
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_pon_arp_downstream", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_pon_arp_upstream(
    int unit,
    int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_PON_ARP_UPSTREAM,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
   
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    
  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_PON;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.ll_is_arp        = 1;
  prog_selection_cam_tbl.cos_profile = SOC_PPC_FLP_COS_PROFILE_ANTI_SPOOFING;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask = 0x00;
  prog_selection_cam_tbl.ptc_profile_mask = 0x0;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.ll_is_arp_mask        = 0;
  prog_selection_cam_tbl.cos_profile_mask = 0x1F; /* 6'b011111 */
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.port_profile = 0x0;
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;


  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_pon_arp_upstream", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_ing_learn(
     int unit,
     uint8 mac_in_mac_enabled,
     int32* mac_in_mac_prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_LEARN,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = 0;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  /* In case of Spoof enabled, msb of cos profile will indicate to use this program or not */
  prog_selection_cam_tbl.cos_profile_mask = 0x00;
  prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.program = PROG_FLP_ETHERNET_ING_LEARN;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (mac_in_mac_enabled) {

      res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_MAC_IN_MAC_AFTER_TERMINATIOM,FALSE,TRUE,&cam_sel_id, mac_in_mac_prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

      prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
      prog_selection_cam_tbl.port_profile = ARAD_PP_FLP_PORT_PROFILE_PBP; /* PBP */
      prog_selection_cam_tbl.ptc_profile  = 0;
      prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
      prog_selection_cam_tbl.forwarding_offset_index = 2;
      prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
      prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
      prog_selection_cam_tbl.ptc_profile_mask = 0x03;
      prog_selection_cam_tbl.forwarding_code_mask = 0x00;
      prog_selection_cam_tbl.forwarding_offset_index_mask = 0;
      prog_selection_cam_tbl.program = (*mac_in_mac_prog_id);
      prog_selection_cam_tbl.valid = 1;
      res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_ing_learn", 0, 0);
}

/* function utility:
   from Occupation Manager in-lif-profile in advanced mode mask
   return the mask that is set with the b'1 lsb
 * @param (in) unit 
 * @param (out) o_mask - the mask of the ivl application 1b lsb
 *  
 * @return soc_error_t
*/
uint32
   arad_pp_flp_prog_mask_map_ivl_learn(
       int unit,
       uint32 *o_mask,
       uint32 *o_val
   )
{
    int res = SOC_E_NONE;
    SHR_BITDCL occ_mask = 0, occ_mask_flipped = 0;
    uint32 val = *o_val;
	uint32 
            int_o_val[1] = {0};
			
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *o_val = 0;
    *o_mask = 0;
    res = arad_pp_occ_mgmt_tcam_args_get(unit,SOC_OCC_MGMT_TYPE_INLIF, SOC_OCC_MGMT_APP_USER, val, int_o_val, &occ_mask, &occ_mask_flipped);
    SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit);

    *o_mask = occ_mask_flipped;
	*o_val = int_o_val[0];
/*  sal_printf ("orig app_val is = %x, hw_val is = %x\n", val, *o_val);*/
/*  sal_printf ("set o_mask = %d\n", *o_mask);                         */
    return res;

    exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_mask_map_ivl_learn", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_learn(
     int unit,
     int32 *prog_id
   )
{
    uint32
    res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
    uint32 in_lif_profile_mask, in_lif_profile_val;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_IVL_LEARN,FALSE,TRUE,&cam_sel_id, prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

    in_lif_profile_val = SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile;
    res = arad_pp_flp_prog_mask_map_ivl_learn(unit, &in_lif_profile_mask, &in_lif_profile_val);

    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "[%d ; %x] in_lif_profile_mask, [%d ; %x] in_lif_profile_val\n"),
                         in_lif_profile_mask, in_lif_profile_mask, in_lif_profile_val, in_lif_profile_val));

    prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
    prog_selection_cam_tbl.port_profile = 0;
    prog_selection_cam_tbl.ptc_profile  = 0;
    prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
    prog_selection_cam_tbl.in_lif_profile = in_lif_profile_val;  /* As assigned bu OCC Mng ; HW value accosiated with the application IVL in-lif profile */
    prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
    /* In case of Spoof enabled, msb of cos profile will indicate to use this program or not */
    prog_selection_cam_tbl.cos_profile_mask = 0x00;
    prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
    prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
    prog_selection_cam_tbl.ptc_profile_mask = 0x03;
    prog_selection_cam_tbl.forwarding_code_mask = 0x00;
    prog_selection_cam_tbl.in_lif_profile_mask = in_lif_profile_mask;   /* As assigned bu OCC Mng */
    prog_selection_cam_tbl.program = *prog_id ; /*PROG_FLP_ETHERNET_ING_IVL_LEARN;*/
    prog_selection_cam_tbl.valid = 1;
    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_learn", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_learn(
     int unit,
     int32 prog_id   /* prog_id - must be call after arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_learn 
                        to get id from uc IVL cam_selection init */
   )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    uint32 in_lif_profile_mask, in_lif_profile_val;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    in_lif_profile_val = SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile;
    res = arad_pp_flp_prog_mask_map_ivl_learn(unit, &in_lif_profile_mask, &in_lif_profile_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit); 

    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "[%d ; %x] in_lif_profile_mask, [%d ; %x] in_lif_profile_val\n"),
                         in_lif_profile_mask, in_lif_profile_mask, in_lif_profile_val, in_lif_profile_val));

    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_id,FALSE,FALSE,&cam_sel_id, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

    prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_PP;
    prog_selection_cam_tbl.port_profile                   = 0;
    prog_selection_cam_tbl.ptc_profile                    = 0;
    prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_ETHERNET;
    prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x0;
    prog_selection_cam_tbl.packet_is_compatible_mc        = 1;

    /* Next protocol is IPV4. PFC checking is not good enough for VPLS and Overlay packets such as VXLAN */
    prog_selection_cam_tbl.forwarding_header_qualifier      = ARAD_PP_FLP_QLFR_ETH_NEXT_PROTOCOL_IPV4;
    prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID |
                                                                   ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_INNER_TPID);

    prog_selection_cam_tbl.parser_leaf_context_mask       = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
    prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
    prog_selection_cam_tbl.ptc_profile_mask               = 0x03;
    prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
    prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1; /* ignore rpf */
    prog_selection_cam_tbl.packet_is_compatible_mc_mask   = 0x0; /* compatible MC only*/
    prog_selection_cam_tbl.packet_format_code_mask        = 0x3F;

    /* IVL Inner program selection options */
    prog_selection_cam_tbl.in_lif_profile = in_lif_profile_val;  /* As assigned bu OCC Mng ; HW value accosiated with the application IVL in-lif profile */
    prog_selection_cam_tbl.in_lif_profile_mask = in_lif_profile_mask;   /* As assigned bu OCC Mng */

    prog_selection_cam_tbl.program                        = prog_id; /*PROG_FLP_ETHERNET_ING_IVL_LEARN;*/
    prog_selection_cam_tbl.valid                          = 1;

    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_learn", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_inner_learn(
     int unit,
     int32 *prog_id
   )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    int idx;
    uint32 in_lif_profile_mask, in_lif_profile_val;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    in_lif_profile_val = SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile;
    res = arad_pp_flp_prog_mask_map_ivl_learn(unit, &in_lif_profile_mask, &in_lif_profile_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "[%d ; %x] in_lif_profile_mask, [%d ; %x] in_lif_profile_val\n"),
                         in_lif_profile_mask, in_lif_profile_mask, in_lif_profile_val, in_lif_profile_val));

    for (idx=0;idx<2;idx++) {
        /*
            idx == 0: After_term MPLS: Ethernet after MPLS termination, fwd-offset-index=3
            idx == 1: After_term S-TAG: Ethernet S-C-TAG, LLVP Tag structure is Stag+Ctag
        */

        if (idx == 0) {
            res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN,FALSE,TRUE,&cam_sel_id, prog_id);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
        else {
            res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN,FALSE,FALSE,&cam_sel_id, NULL);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
        res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

        prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
        prog_selection_cam_tbl.port_profile = 0;
        prog_selection_cam_tbl.ptc_profile  = 0;
        prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
        prog_selection_cam_tbl.in_lif_profile = in_lif_profile_val;  /* As assigned bu OCC Mng ; HW value accosiated with the application IVL in-lif profile */
        prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
        /* In case of Spoof enabled, msb of cos profile will indicate to use this program or not */
        prog_selection_cam_tbl.cos_profile_mask = 0x00;
        prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
        prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
        prog_selection_cam_tbl.ptc_profile_mask = 0x03;
        prog_selection_cam_tbl.forwarding_code_mask = 0x00;
        prog_selection_cam_tbl.in_lif_profile_mask = in_lif_profile_mask;   /* As assigned bu OCC Mng */

        prog_selection_cam_tbl.llvp_incoming_tag_structure = (idx==1 ? 6: prog_selection_cam_tbl.llvp_incoming_tag_structure);
        prog_selection_cam_tbl.llvp_incoming_tag_structure_mask = (idx==1 ? 0x0: prog_selection_cam_tbl.llvp_incoming_tag_structure_mask);

        prog_selection_cam_tbl.forwarding_offset_index = (idx==0 ? 3: prog_selection_cam_tbl.forwarding_offset_index);
        prog_selection_cam_tbl.forwarding_offset_index_mask = (idx==0 ? 0x0: prog_selection_cam_tbl.forwarding_offset_index_mask);

        prog_selection_cam_tbl.program = *prog_id ; /*PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN;*/
        prog_selection_cam_tbl.valid = 1;

        res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_inner_learn", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_inner_learn(
     int unit,
     int32 prog_id   /* prog_id - must call after arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_inner_learn 
                        to get id from uc IVL cam_selection init */
   )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    int idx;
    uint32 in_lif_profile_mask, in_lif_profile_val;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    in_lif_profile_val = SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile;
    res = arad_pp_flp_prog_mask_map_ivl_learn(unit, &in_lif_profile_mask, &in_lif_profile_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "[%d ; %x] in_lif_profile_mask, [%d ; %x] in_lif_profile_val\n"),
                         in_lif_profile_mask, in_lif_profile_mask, in_lif_profile_val, in_lif_profile_val));

    for (idx = 0; idx < 2; idx++) {
        /*
            idx == 0: After_term MPLS: Ethernet after MPLS termination, fwd-offset-index=3
            idx == 1: After_term S-TAG: Ethernet S-C-TAG, LLVP Tag structure is Stag+Ctag
        */
        res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_id,FALSE,FALSE,&cam_sel_id, NULL);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

        prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_PP;
        prog_selection_cam_tbl.port_profile                   = 0;
        prog_selection_cam_tbl.ptc_profile                    = 0;
        prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_ETHERNET;
        prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x0;
        prog_selection_cam_tbl.packet_is_compatible_mc        = 1;

        /* Next protocol is IPV4. PFC checking is not good enough for VPLS and Overlay packets such as VXLAN */
        prog_selection_cam_tbl.forwarding_header_qualifier      = ARAD_PP_FLP_QLFR_ETH_NEXT_PROTOCOL_IPV4;
        prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID |
                                                                       ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_INNER_TPID);

        prog_selection_cam_tbl.parser_leaf_context_mask       = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
        prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
        prog_selection_cam_tbl.ptc_profile_mask               = 0x03;
        prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
        prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1; /* ignore rpf */
        prog_selection_cam_tbl.packet_is_compatible_mc_mask   = 0x0; /* compatible MC only*/
        prog_selection_cam_tbl.packet_format_code_mask        = 0x3F;

        /* IVL Inner program selection options */
        prog_selection_cam_tbl.llvp_incoming_tag_structure = (idx==1 ? 6: prog_selection_cam_tbl.llvp_incoming_tag_structure);
        prog_selection_cam_tbl.llvp_incoming_tag_structure_mask = (idx==1 ? 0x0: prog_selection_cam_tbl.llvp_incoming_tag_structure_mask);

        prog_selection_cam_tbl.forwarding_offset_index = (idx==0 ? 3: prog_selection_cam_tbl.forwarding_offset_index);
        prog_selection_cam_tbl.forwarding_offset_index_mask = (idx==0 ? 0x0: prog_selection_cam_tbl.forwarding_offset_index_mask);

        prog_selection_cam_tbl.in_lif_profile = in_lif_profile_val;  /* As assigned bu OCC Mng ; HW value accosiated with the application IVL in-lif profile */
        prog_selection_cam_tbl.in_lif_profile_mask = in_lif_profile_mask;   /* As assigned bu OCC Mng */

        prog_selection_cam_tbl.program                        = prog_id; /*PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN;*/
        prog_selection_cam_tbl.valid                          = 1;

        res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_inner_learn", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_fwd_outer_learn(
     int unit,
     int32 *prog_id
   )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    int idx;
    uint32 in_lif_profile_mask, in_lif_profile_val;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    in_lif_profile_val = SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile;
    res = arad_pp_flp_prog_mask_map_ivl_learn(unit, &in_lif_profile_mask, &in_lif_profile_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "[%d ; %x] in_lif_profile_mask, [%d ; %x] in_lif_profile_val\n"),
                         in_lif_profile_mask, in_lif_profile_mask, in_lif_profile_val, in_lif_profile_val));

    for (idx=0;idx<2;idx++) {
        /* 
            either: 
            idx == 0: CoS-Profile[msb] is 1 AND FORWARDING_HEADER_QUALIFIER is UNTAGGED (bit 0 == 1)
            or
            idx == 1: CoS-Profile[msb] is 1 AND FORWARDING_HEADER_QUALIFIER is UNTAGGED (bit 1 == 1)
        */

        if (idx == 0) {
            /* First Program Selection setting - get a program_id pointer */
            res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_IVL_FWD_OUTER_LEARN,FALSE,TRUE,&cam_sel_id, prog_id);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
        else {
            res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_IVL_FWD_OUTER_LEARN,FALSE,FALSE,&cam_sel_id, NULL);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
        res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

        prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
        prog_selection_cam_tbl.port_profile = 0;
        prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
        prog_selection_cam_tbl.ptc_profile  = 0;
        prog_selection_cam_tbl.ptc_profile_mask = 0x03;

        prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
        prog_selection_cam_tbl.forwarding_code_mask = 0x00;

        prog_selection_cam_tbl.in_lif_profile = in_lif_profile_val;  /* As assigned bu OCC Mng ; HW value accosiated with the application IVL in-lif profile */
        prog_selection_cam_tbl.in_lif_profile_mask = in_lif_profile_mask;   /* As assigned bu OCC Mng */
        prog_selection_cam_tbl.cos_profile = 0x20;   /* COS Profile msb is set ==> IVL tagged format packets */
        prog_selection_cam_tbl.cos_profile_mask = 0x1F; /* Match 1 msb only */


        prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
        /* In case of Spoof enabled, msb of cos profile will indicate to use this program or not */

        prog_selection_cam_tbl.forwarding_header_qualifier = (idx==0 ? 0x001: 0x002);       /* Index = outer-tag exists - tpid1 or tpid3 */
        prog_selection_cam_tbl.forwarding_header_qualifier_mask = (idx==0 ? 0x7FE: 0x7FC);  /* Index = outer-tag exists - tpid2 or tpid3 */

        prog_selection_cam_tbl.program = *prog_id ; /*PROG_FLP_ETHERNET_ING_IVL_FWD_OUTER_LEARN;*/
        prog_selection_cam_tbl.valid = 1;

        res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_fwd_outer_learn", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_fwd_outer_learn(
     int unit,
     int32 prog_id   /* prog_id - must call after arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_fwd_outer_learn 
                        to get id from uc IVL cam_selection init */
   )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    int idx;
    uint32 in_lif_profile_mask, in_lif_profile_val;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    in_lif_profile_val = SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile;
    res = arad_pp_flp_prog_mask_map_ivl_learn(unit, &in_lif_profile_mask, &in_lif_profile_val);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "[%d ; %x] in_lif_profile_mask, [%d ; %x] in_lif_profile_val\n"),
                         in_lif_profile_mask, in_lif_profile_mask, in_lif_profile_val, in_lif_profile_val));

    for (idx = 0; idx < 2; idx++) {
        /*
            idx == 0: After_term MPLS: Ethernet after MPLS termination, fwd-offset-index=3
            idx == 1: After_term S-TAG: Ethernet S-C-TAG, LLVP Tag structure is Stag+Ctag
        */
        res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_id,FALSE,FALSE,&cam_sel_id, NULL);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

        prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_PP;
        prog_selection_cam_tbl.port_profile                   = 0;
        prog_selection_cam_tbl.ptc_profile                    = 0;
        prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_ETHERNET;
        prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x0;
        prog_selection_cam_tbl.packet_is_compatible_mc        = 1;

        /* Next protocol is IPV4. PFC checking is not good enough for VPLS and Overlay packets such as VXLAN */
        prog_selection_cam_tbl.forwarding_header_qualifier      = ARAD_PP_FLP_QLFR_ETH_NEXT_PROTOCOL_IPV4;
        prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID |
                                                                       ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_INNER_TPID);

        prog_selection_cam_tbl.parser_leaf_context_mask       = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
        prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
        prog_selection_cam_tbl.ptc_profile_mask               = 0x03;
        prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
        prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1; /* ignore rpf */
        prog_selection_cam_tbl.packet_is_compatible_mc_mask   = 0x0; /* compatible MC only*/
        prog_selection_cam_tbl.packet_format_code_mask        = 0x3F;

        /* IVL Inner program selection options */
        prog_selection_cam_tbl.llvp_incoming_tag_structure = (idx==1 ? 6: prog_selection_cam_tbl.llvp_incoming_tag_structure);
        prog_selection_cam_tbl.llvp_incoming_tag_structure_mask = (idx==1 ? 0x0: prog_selection_cam_tbl.llvp_incoming_tag_structure_mask);

        prog_selection_cam_tbl.forwarding_offset_index = (idx==0 ? 3: prog_selection_cam_tbl.forwarding_offset_index);
        prog_selection_cam_tbl.forwarding_offset_index_mask = (idx==0 ? 0x0: prog_selection_cam_tbl.forwarding_offset_index_mask);

        prog_selection_cam_tbl.in_lif_profile = in_lif_profile_val;  /* As assigned bu OCC Mng ; HW value accosiated with the application IVL in-lif profile */
        prog_selection_cam_tbl.in_lif_profile_mask = in_lif_profile_mask;   /* As assigned bu OCC Mng */

        prog_selection_cam_tbl.program                        = prog_id; /*PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN;*/
        prog_selection_cam_tbl.valid                          = 1;

        res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_fwd_outer_learn", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_oam_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_OAM_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);


  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask = 0x0;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_oam_statistics", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_oam_down_untagged_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_OAM_DOWN_UNTAGGED_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_TM;
  prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask = 0x0;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.tt_lookup_0_found =1;
  prog_selection_cam_tbl.tt_lookup_0_found_mask = 0;
  prog_selection_cam_tbl.vt_lookup_0_found = 1;
  prog_selection_cam_tbl.vt_lookup_0_found_mask = 0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_oam_statistics", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_oam_single_tag_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_OAM_SINGLE_TAG_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_TM;
  prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask = 0x0;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.tt_lookup_1_found =1;
  prog_selection_cam_tbl.tt_lookup_1_found_mask = 0;
  prog_selection_cam_tbl.tt_lookup_0_found =0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask = 0;
  prog_selection_cam_tbl.vt_lookup_0_found = 1;
  prog_selection_cam_tbl.vt_lookup_0_found_mask = 0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_oam_statistics", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_oam_double_tag_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_OAM_DOUBLE_TAG_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_TM;
  prog_selection_cam_tbl.cos_profile_mask = 0x3F; 
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask = 0x0;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.tt_lookup_1_found =0;
  prog_selection_cam_tbl.tt_lookup_1_found_mask = 0;
  prog_selection_cam_tbl.tt_lookup_0_found =0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask = 0;
  prog_selection_cam_tbl.vt_lookup_0_found = 1;
  prog_selection_cam_tbl.vt_lookup_0_found_mask = 0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_oam_statistics", 0, 0);
}




uint32
   arad_pp_flp_prog_sel_cam_bfd_mpls_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_BFD_MPLS_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);



  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_TM; 
  prog_selection_cam_tbl.ptc_profile_mask = 0x0; 
  prog_selection_cam_tbl.forwarding_code_mask = 0x00; 
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.vt_lookup_1_found=1;
  prog_selection_cam_tbl.vt_lookup_1_found_mask=0;
  prog_selection_cam_tbl.tt_lookup_0_found=0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask=0;
  prog_selection_cam_tbl.tt_lookup_1_found=1;
  prog_selection_cam_tbl.tt_lookup_1_found_mask=0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_bfd_statistics", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_bfd_pwe_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_BFD_PWE_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_TM; 
  prog_selection_cam_tbl.ptc_profile_mask = 0x0; 
  prog_selection_cam_tbl.forwarding_code_mask = 0x00; 
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.vt_lookup_1_found=1;
  prog_selection_cam_tbl.vt_lookup_1_found_mask=0;
  prog_selection_cam_tbl.tt_lookup_0_found=0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask=0;
  prog_selection_cam_tbl.tt_lookup_1_found=0;
  prog_selection_cam_tbl.tt_lookup_1_found_mask=0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_bfd_statistics", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_bfd_statistics(
     int unit,
     int32* prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_BFD_STATISTICS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_OAMP;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_TM; 
  prog_selection_cam_tbl.ptc_profile_mask = 0x0; 
  prog_selection_cam_tbl.forwarding_code_mask = 0x00; 
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  prog_selection_cam_tbl.vt_lookup_1_found=1;
  prog_selection_cam_tbl.vt_lookup_1_found_mask=0;
  prog_selection_cam_tbl.tt_lookup_0_found=1;
  prog_selection_cam_tbl.tt_lookup_0_found_mask=0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_bfd_statistics", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_pon_default_upstream(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_PON_DEFAULT_UPSTREAM,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile = 0;  
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_PON;  
  prog_selection_cam_tbl.ptc_profile_mask = 0;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;  
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.cos_profile = 0;
  /* For vmac/local switch, cos_profile 0x20 is presenting vmac/local switch
   * AC. if the AC isn't mac/local switch AC, use this default program.
   * 
   * For anti-spoofing, expect none-IP packet can pass through.
   * So if anti-spoofing program doesn't selected because of
   * packet format isn't IP, we want the default program will be selected.
   *
   */
  if (SOC_DPP_CONFIG(unit)->pp.vmac_enable ||
      SOC_DPP_CONFIG(unit)->pp.local_switching_enable) {    
      prog_selection_cam_tbl.cos_profile_mask = 0x1F; 
  } else {
      prog_selection_cam_tbl.cos_profile_mask = 0x3F;
  }
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_pon_default_upstream", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ethernet_pon_default_downstream(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_PON_DEFAULT_DOWNSTREAM,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile = 0;  
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile  = SOC_TMC_PORTS_FLP_PROFILE_NONE;  
  prog_selection_cam_tbl.ptc_profile_mask = 0;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;  
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.cos_profile = 0;
  if ((SOC_DPP_CONFIG(unit)->pp.l3_source_bind_mode != SOC_DPP_L3_SOURCE_BIND_MODE_DISABLE) ||
      SOC_DPP_CONFIG(unit)->pp.vmac_enable ||
      SOC_DPP_CONFIG(unit)->pp.local_switching_enable) {    
      prog_selection_cam_tbl.cos_profile_mask = 0x1F; 
  } else {
      prog_selection_cam_tbl.cos_profile_mask = 0x3F;
  }
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_pon_default_downstream", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_pon_local_route(
     int unit,
     uint8 mac_in_mac_enabled,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_PON_LOCAL_ROUTE,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile = 0;  
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile  = 0;  
  prog_selection_cam_tbl.ptc_profile_mask = 0x03;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;  
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.cos_profile = SOC_PPC_LIF_AC_LOCAL_SWITCHING_COS_PROFILE;
  prog_selection_cam_tbl.cos_profile_mask = 0x1F; 
  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if (mac_in_mac_enabled) {
      int32 mim_prog_id;

      res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_MAC_IN_MAC_AFTER_TERMINATIOM,FALSE,TRUE,&cam_sel_id, &mim_prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

      prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
      prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
      prog_selection_cam_tbl.port_profile = ARAD_PP_FLP_PORT_PROFILE_PBP; /* PBP */
      prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
      prog_selection_cam_tbl.ptc_profile  = 0;
      prog_selection_cam_tbl.ptc_profile_mask = 0x03;
      prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
      prog_selection_cam_tbl.forwarding_code_mask = 0x00;
      prog_selection_cam_tbl.forwarding_offset_index = 2;      
      prog_selection_cam_tbl.forwarding_offset_index_mask = 0;
      prog_selection_cam_tbl.program = mim_prog_id;
      prog_selection_cam_tbl.valid = 1;
      res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_pon_local_route", 0, 0);
}


/* do ethernet for FCoE packet (when not doing FCF) */
uint32
   arad_pp_flp_prog_sel_cam_fcoe_ethernet_ing_learn(
     int    unit,
     int32  *prog_id,
     int    is_fcoe_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);  
    
  if(is_fcoe_enabled){      
      res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_FC_TRANSIT,FALSE,TRUE,&cam_sel_id, prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }else{
      (*prog_id) = PROG_FLP_ETHERNET_ING_LEARN;
      res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_LEARN,FALSE,FALSE,&cam_sel_id, NULL);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* pfc is FCoE*/
  /* ARAD_PARSER_PFC_FC_STD_ETH or ARAD_PARSER_PFC_FC_ENCAP_ETH*/
  prog_selection_cam_tbl.packet_format_code  = ARAD_PARSER_PFC_FC_STD_ETH; /* 6'b000100;*/
  prog_selection_cam_tbl.packet_format_code_mask  = 0x1; /* 6'b00_0100;*/

  /* PLC is ARAD_PARSER_PLC_FCOE_VFT or ARAD_PARSER_PLC_FCoE*/
  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_FCOE_VFT; /* b0100*/
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x1; /* b0001*/

  /* forwarding is Ethernet */
  prog_selection_cam_tbl.port_profile                   = 0;
  prog_selection_cam_tbl.ptc_profile                    = 0;
  prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.forwarding_offset_index        = 1;

  prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask               = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.forwarding_offset_index_mask   = 0;

  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_fcoe_ethernet_ing_learn", 0, 0);
}



uint32
   arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft(
     int unit,
     int32  *prog_id,
     uint32 is_npv
     )
{
  uint32
      prog_usage,
      res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  if (is_npv == 1) {
      prog_usage = PROG_FLP_FC_WITH_VFT_N_PORT;
  }else{
      prog_usage = PROG_FLP_FC_WITH_VFT;
  }

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_usage,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_CUSTOM1;
  prog_selection_cam_tbl.forwarding_offset_index = 2;

  prog_selection_cam_tbl.tt_lookup_0_found       = 0x1; /* local */
  prog_selection_cam_tbl.parser_leaf_context     = ARAD_PARSER_PLC_FCOE_VFT; /* with-vft*/
  prog_selection_cam_tbl.tt_processing_profile   = ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_FC; /* 3 */
  prog_selection_cam_tbl.packet_format_code      = ARAD_PARSER_PFC_FC_STD_ETH;

  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.forwarding_offset_index_mask   = 0;
  prog_selection_cam_tbl.parser_leaf_context_mask       = 0x0; 
  prog_selection_cam_tbl.tt_processing_profile_mask     = 0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask         = is_npv; /* is_npv is always local... so the tt_lookup not metter */
  prog_selection_cam_tbl.packet_format_code_mask        = 0;
  prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NPV;
  
  if (is_npv) {      
      prog_selection_cam_tbl.port_profile               = ARAD_PP_FLP_PORT_PROFILE_FC_N_PORT;
  }else{
      prog_selection_cam_tbl.port_profile               = ARAD_PP_FLP_PORT_PROFILE_DEFAULT;
  }

  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);  
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_fcoe_fcf(
     int unit,
     int32  *prog_id,
     uint32 is_npv
   )
{
  uint32
      prog_usage,
      res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  if (is_npv == 1) {
      prog_usage = PROG_FLP_FC_N_PORT;
  }else{
      prog_usage = PROG_FLP_FC;
  }

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_usage,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  
  prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_CUSTOM1;
  prog_selection_cam_tbl.forwarding_offset_index        = 2;
  prog_selection_cam_tbl.tt_processing_profile          = ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_FC; /* 3 */
  prog_selection_cam_tbl.tt_lookup_0_found              = 0x1; /* local */
  prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_FCOE_VFT; /* with-vft*/
  prog_selection_cam_tbl.packet_format_code             = ARAD_PARSER_PFC_FC_ENCAP_ETH; /* both header according to mask*/

  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.forwarding_offset_index_mask   = 0;
  prog_selection_cam_tbl.tt_processing_profile_mask     = 0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask         = is_npv; /* is_npv is alwas local... so the tt_lookup not metter */
  prog_selection_cam_tbl.parser_leaf_context_mask       = 0x1;
  prog_selection_cam_tbl.packet_format_code_mask        = 0x1;

  if (is_npv) {
      prog_selection_cam_tbl.port_profile_mask          = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NPV;
      prog_selection_cam_tbl.port_profile               = ARAD_PP_FLP_PORT_PROFILE_FC_N_PORT;
  }else {
      prog_selection_cam_tbl.port_profile_mask          = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NPV;
      prog_selection_cam_tbl.port_profile               = ARAD_PP_FLP_PORT_PROFILE_DEFAULT;
  }

  prog_selection_cam_tbl.program                        = *prog_id;
  prog_selection_cam_tbl.valid                          = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_fcoe_fcf", 0, 0);
}



uint32
   arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft_remote(
     int unit,
     int32  *prog_id
   )
{
  uint32
     res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_FC_WITH_VFT_REMOTE,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_CUSTOM1;
  prog_selection_cam_tbl.forwarding_offset_index        = 2;
  prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_FCOE_VFT; /* with-vft */
  prog_selection_cam_tbl.tt_processing_profile          = ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_FC; /* 3 */
  prog_selection_cam_tbl.tt_lookup_0_found              = 0x0; /* remote */
  prog_selection_cam_tbl.packet_format_code             = ARAD_PARSER_PFC_FC_STD_ETH;

  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.forwarding_offset_index_mask   = 0;
  prog_selection_cam_tbl.parser_leaf_context_mask       = 0x0; /* b0001*/
  prog_selection_cam_tbl.tt_processing_profile_mask     = 0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask         = 0;
  prog_selection_cam_tbl.packet_format_code_mask        = 0x0;

  prog_selection_cam_tbl.port_profile                   = ARAD_PP_FLP_PORT_PROFILE_DEFAULT;
  prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NPV;

  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft_remote", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_fcoe_fcf_remote(
     int unit,
     int32  *prog_id
   )
{
  uint32      
      res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);  

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_FC_REMOTE,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  
  prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_CUSTOM1;
  prog_selection_cam_tbl.forwarding_offset_index        = 2;
  prog_selection_cam_tbl.tt_processing_profile          = ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_FC; /* 3 */
  prog_selection_cam_tbl.tt_lookup_0_found              = 0x0;  /* remote */
  prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_FCOE_VFT; /* with-vft */
  prog_selection_cam_tbl.packet_format_code             = ARAD_PARSER_PFC_FC_ENCAP_ETH;

  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.forwarding_offset_index_mask   = 0;
  prog_selection_cam_tbl.tt_processing_profile_mask     = 0;
  prog_selection_cam_tbl.tt_lookup_0_found_mask         = 0;
  prog_selection_cam_tbl.parser_leaf_context_mask       = 0x1; /* both except what caught in prev program */
  prog_selection_cam_tbl.packet_format_code_mask        = 0x1; /* both encap + standard except what caught in prev program */

  prog_selection_cam_tbl.port_profile                   = ARAD_PP_FLP_PORT_PROFILE_DEFAULT;
  prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NPV;

  prog_selection_cam_tbl.program = *prog_id;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_fcoe_fcf_remote", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ethernet_mac_in_mac(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_MAC_IN_MAC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile = ARAD_PP_FLP_PORT_PROFILE_PBP; /* PBP */
  prog_selection_cam_tbl.ptc_profile  = 0;
  prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.forwarding_offset_index = 1;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask = 0x00;
  prog_selection_cam_tbl.forwarding_offset_index_mask = 0;
  prog_selection_cam_tbl.program = PROG_FLP_ETHERNET_MAC_IN_MAC;
  prog_selection_cam_tbl.valid = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ethernet_mac_in_mac", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn_rpf(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(prog_id);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4UC_PUBLIC_RPF,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV4_UC;
  prog_selection_cam_tbl.l_3_vpn_default_routing     = 0x1;
  prog_selection_cam_tbl.l_3_vpn_default_routing_mask = 0x00;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
         /* format code for TM, this makes sure this tcam selection is never chosen */
         prog_selection_cam_tbl.packet_format_code  = 0x20; /* 6'b10_0000;*/
         prog_selection_cam_tbl.packet_format_code_mask  = 0x1F; /* 6'b01_1111;*/
  }
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x1;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4uc_with_l3vpn", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv6uc_l3vpn(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(prog_id);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC_PUBLIC,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_UC;
  prog_selection_cam_tbl.l_3_vpn_default_routing     = 0x1;
  prog_selection_cam_tbl.l_3_vpn_default_routing_mask = 0x00;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
         /* format code for TM, this makes sure this tcam selection is never chosen */
         prog_selection_cam_tbl.packet_format_code  = 0x20; /* 6'b10_0000;*/
         prog_selection_cam_tbl.packet_format_code_mask  = 0x1F; /* 6'b01_1111;*/
  }
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6uc_l3vpn", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ipv6uc_l3vpn_rpf(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(prog_id);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC_PUBLIC_RPF,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_UC;
  prog_selection_cam_tbl.l_3_vpn_default_routing     = 0x1;
  prog_selection_cam_tbl.l_3_vpn_default_routing_mask = 0x00;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
         /* format code for TM, this makes sure this tcam selection is never chosen */
         prog_selection_cam_tbl.packet_format_code  = 0x20; /* 6'b10_0000;*/
         prog_selection_cam_tbl.packet_format_code_mask  = 0x1F; /* 6'b01_1111;*/
  }
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x1;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x0;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6uc_l3vpn_rpf", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4UC_PUBLIC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV4_UC;
  prog_selection_cam_tbl.l_3_vpn_default_routing     = 0x1;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.l_3_vpn_default_routing_mask= 0x00;
  prog_selection_cam_tbl.program                     = PROG_FLP_IPV4UC_PUBLIC;
  prog_selection_cam_tbl.valid                       = 1;
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
         /* format code for TM, this makes sure this tcam selection is never chosen */
         prog_selection_cam_tbl.packet_format_code  = 0x20; /* 6'b10_0000;*/
         prog_selection_cam_tbl.packet_format_code_mask  = 0x1F; /* 6'b01_1111;*/
  }
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (JER_KAPS_ENABLE(unit)) {
      /* In Jericho the KAPS is able to perform both RPF and public searches */
      prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
      prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x0;
  }
#endif
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4uc_with_l3vpn", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn_custom_prgrm(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IP4UC_CUSTOM_ROUTE,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.port_profile = ARAD_PP_FLP_PORT_PROFILE_PBP; /* PBP */
  prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;

  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV4_UC;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;

  prog_selection_cam_tbl.packet_format_code = ARAD_PARSER_PFC_IPV4_ETH;
  prog_selection_cam_tbl.packet_format_code_mask = 0x18; /* 6'b011000 */
 
  prog_selection_cam_tbl.forwarding_offset_index = 2;
  prog_selection_cam_tbl.forwarding_offset_index_mask = 0x0;

  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;


  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn_custom_prgrm", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf_2pass(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC_WITH_RPF_2PASS,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 211, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_UC;   /* IPv6-UC */
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x1;                        /* inRIF.enable_rpf_check is set */
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;                        /* mask set to validate forwarding code + */
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x00;                     /* mask for inRIF.enable_rpf_check is set */
  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
	  prog_selection_cam_tbl.l_3_vpn_default_routing        = 0x1;
	  prog_selection_cam_tbl.l_3_vpn_default_routing_mask   = 0x0;
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1; 
  }
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 212, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf_2pass", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv4uc_with_rpf(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4UC_RPF,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV4_UC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x1;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x00;
  prog_selection_cam_tbl.program                   = PROG_FLP_IPV4UC_RPF;
  prog_selection_cam_tbl.valid                     = 1;
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x1;
	  prog_selection_cam_tbl.l_3_vpn_default_routing_mask   = 0x1;
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x0; 
  }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (JER_KAPS_ENABLE(unit) && !ARAD_KBP_ENABLE_IPV4_UC) {
      prog_selection_cam_tbl.l_3_vpn_default_routing     = 0x0;
      prog_selection_cam_tbl.l_3_vpn_default_routing_mask = 0x00;
  }
#endif
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4uc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv4uc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4UC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV4_UC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x00;
  prog_selection_cam_tbl.program                   = PROG_FLP_IPV4UC;
  prog_selection_cam_tbl.valid                     = 1;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (SOC_IS_JERICHO(unit)) {
          if(ARAD_KBP_ENABLE_IPV4_DC){ /* incase of double capacity use according to inrif profile */              
              prog_selection_cam_tbl.custom_rif_profile_bit      = 0x0;
              prog_selection_cam_tbl.custom_rif_profile_bit_mask = 0x0;             
          }
      }
#endif
      
  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x0;
	  prog_selection_cam_tbl.l_3_vpn_default_routing_mask   = 0x1;
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x0; 
  }
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4uc", 0, 0);
}


uint32
   arad_pp_flp_prog_sel_cam_ipv4_dc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4_DC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV4_UC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x00;

  prog_selection_cam_tbl.custom_rif_profile_bit_mask = 0x0;
  prog_selection_cam_tbl.custom_rif_profile_bit      = 0x1;

  prog_selection_cam_tbl.program                     = PROG_FLP_IPV4_DC;
  prog_selection_cam_tbl.valid                       = 1;

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4_dc", 0, 0);
}



/* bfd ipv4/ipv6 single hop is selected if Parser-Leaf-Context = x (FIX)
   The parser sets the Parser-Leaf-Context to X if UDP.Dst-Port = 3784*/
uint32
   arad_pp_flp_prog_sel_cam_bfd_single_hop(
     int unit,
     int is_ipv6,
	 int32  *prog_id
   )
{
  uint32
    res;
  uint32 prog_usage;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  prog_usage = (is_ipv6 ? PROG_FLP_BFD_IPV6_SINGLE_HOP : PROG_FLP_BFD_IPV4_SINGLE_HOP);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,prog_usage,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_BFD_SINGLE_HOP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = (is_ipv6 ? ARAD_PP_FWD_CODE_IPV6_UC : ARAD_PP_FWD_CODE_IPV4_UC);
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x00;
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;
 /* next protocol is UDP*/
  prog_selection_cam_tbl.forwarding_header_qualifier = 0x480;
  prog_selection_cam_tbl.forwarding_header_qualifier_mask = 0x7F; 
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id,&prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_bfd_single_hop", 0, 0);
}

/* bfd ipv4/ipv6 single hop bridge is selected if Parser-Leaf-Context = x (FIX) and forwarding code is Ethernet
   The parser sets the Parser-Leaf-Context to X if UDP.Dst-Port = 3784
   This is the case when packet should be bridged*/
uint32 arad_pp_flp_prog_sel_cam_bfd_single_hop_bridge( int unit)
{
    uint32 res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA prog_selection_cam_tbl;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_LEARN,FALSE,FALSE,&cam_sel_id, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    prog_selection_cam_tbl.parser_leaf_context = ARAD_PARSER_PLC_BFD_SINGLE_HOP;
    prog_selection_cam_tbl.port_profile = 0;
    prog_selection_cam_tbl.ptc_profile  = 0;
    prog_selection_cam_tbl.forwarding_code = ARAD_PP_FWD_CODE_ETHERNET;
    prog_selection_cam_tbl.parser_leaf_context_mask = 0x0; /*Should be ARAD_PARSER_PLC_BFD_SINGLE_HOP */
    prog_selection_cam_tbl.cos_profile_mask = 0x00;
    prog_selection_cam_tbl.cos_profile_mask = 0x3F;
    prog_selection_cam_tbl.port_profile_mask = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
    prog_selection_cam_tbl.ptc_profile_mask = 0x03;
    prog_selection_cam_tbl.forwarding_code_mask = 0x00;
    prog_selection_cam_tbl.program = PROG_FLP_ETHERNET_ING_LEARN;
    prog_selection_cam_tbl.valid = 1;
    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
     SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_bfd_single_hop_bridge", 0, 0);
}
/* 
 *  IPv6 FLP program Selection for IPv6 FWD in the second pass
 */
uint32
   arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf_2pass_fwd(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC,FALSE,FALSE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* CAM selection for second pass (IPv6 with RPF RCY port) must be higher than first pass */
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0; 
  prog_selection_cam_tbl.ptc_profile                 = ARAD_PORTS_FLP_PROFILE_OVERLAY_RCY; /* IPV6 UC FWD second pass (rcy port) PTC */
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_UC;   /* IPv6-UC */
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x1;                        /* inRIF.enable_rpf_check is set */
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x00;                        /* mask set to validate PTC profile and */
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;                        /* mask set to validate forwarding code and */
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x00;                     /* mask for inRIF.enable_rpf_check is set */

  prog_selection_cam_tbl.program                   = PROG_FLP_IPV6UC;
  prog_selection_cam_tbl.valid                     = 1;

  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
	  prog_selection_cam_tbl.l_3_vpn_default_routing        = 0x1;
	  prog_selection_cam_tbl.l_3_vpn_default_routing_mask   = 0x0;
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1; 
  }
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf_2pass_fwd", 0, 0);
}


/*
 *  IPv6 FLP program Selection with RPF
 */
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

uint32
   arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(prog_id);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC_RPF,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_UC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x1;
  if (JER_KAPS_ENABLE(unit) && !ARAD_KBP_ENABLE_IPV6_RPF) {
      prog_selection_cam_tbl.l_3_vpn_default_routing     = 0x0;
      prog_selection_cam_tbl.l_3_vpn_default_routing_mask= 0x0;
  }
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x00;
  prog_selection_cam_tbl.program                   = *prog_id;
  prog_selection_cam_tbl.valid                     = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf", 0, 0);
}
#endif

/* 
 *  IPv6 FLP program Selection when RPF check is not enabled
 */
uint32
   arad_pp_flp_prog_sel_cam_ipv6uc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_UC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x01; /* ignore rpf */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (JER_KAPS_ENABLE(unit) && !ARAD_KBP_ENABLE_IPV6_UC) {
      prog_selection_cam_tbl.l_3_vpn_default_routing        = 0x0;
      prog_selection_cam_tbl.l_3_vpn_default_routing_mask   = 0x0;
      prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
      prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = 0x0;
  }
  if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_EXTENDED){
      /* 
       * In KBP, another FLP program for IPv6 UC with RPF to differentiate from this one: 
       * - cannot assume the location of the program selection in the table due to the 
       * program management 
       * - the single difference in program selection is the in_rif_uc_rpf_enable value
       */
      prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x00; /* only if not rpf */
  } else if (SOC_IS_JERICHO(unit)) {
#else
if (SOC_IS_JERICHO(unit)) {
#endif
    if(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "l3_ipv6_uc_use_tcam", 0)) {
      /* Currently when the IPV6 UC is using the TCAM the IPV6 UC RPF is disabled. */
      prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x01;
    } else {
      /*KAPS - rpf should not be ignored in jericho unless TCAM is used*/
      prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x00; /* only if not rpf */
    }
}


  prog_selection_cam_tbl.program                   = PROG_FLP_IPV6UC;
  prog_selection_cam_tbl.valid                     = 1;

  if (SEPARATE_IPV4_IPV6_RPF_ENABLE) {
	  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1; 
  }

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6uc", 0, 0);
}

STATIC
  uint32
    arad_pp_flp_instruction_rsrc_set(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  uint32  prog_id
    )
{
    uint32
      ce_rsrc = 0;
    soc_error_t
      rv;    

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    arad_pp_ihb_flp_ce_resources_per_program_get_unsafe(unit, prog_id, &ce_rsrc);


    rv = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.set(
            unit, 
            SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 
            prog_id,
            0, /* Cycle Index */
            ce_rsrc
        );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(rv, 41, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_elk_instruction_rsrc_set", 0, 0);
}

STATIC
  uint32
    arad_pp_flp_all_progs_instruction_set(
       SOC_SAND_IN  int  unit
    )
{
  uint32
    prog_idx,
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (prog_idx = 0; prog_idx < SOC_DPP_DEFS_GET(unit, nof_flp_programs); prog_idx++) 
  {
      res = arad_pp_flp_instruction_rsrc_set(unit, prog_idx);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_all_progs_instruction_set", 0, 0);
}


#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
    arad_pp_flp_elk_prog_config(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  uint32  prog_id,
       SOC_SAND_IN  uint32  opcode,
       SOC_SAND_IN  uint32  key_size_msb,
       SOC_SAND_IN  uint32  key_size
    )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
      flp_lookups_tbl;

#ifdef BCM_88675_A0
    ARAD_INIT_ELK* elk = &SOC_DPP_CONFIG(unit)->arad->init.elk;
#endif

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* 1. Configure Lookup table:
     *    Set ELK as a valid lookup, and configure the
     *    which bytes to use for the master key.
     */
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(
            unit, 
            prog_id, 
            &flp_lookups_tbl
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    if (SOC_IS_JERICHO(unit)) {
#ifdef BCM_88675_A0
        if(elk->kbp_no_fwd_ipv6_dip_sip_sharing_disable == 0) {
            if(ARAD_PP_FLP_IPV6_DIP_SIP_SHARING_IS_PROG_VALID(opcode)) {
                flp_lookups_tbl.elk_packet_data_select = 0x6; /* select forwarding header */
            }
        }
#endif
        flp_lookups_tbl.elk_key_c_valid_bytes += key_size;
        flp_lookups_tbl.elk_key_c_msb_valid_bytes += key_size_msb;

    }else{
        flp_lookups_tbl.elk_key_c_valid_bytes += key_size;
    }


    flp_lookups_tbl.elk_lkp_valid = 0x1;
    flp_lookups_tbl.elk_wait_for_reply = 0x1;
    flp_lookups_tbl.elk_opcode = opcode;    

    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(
            unit, 
            prog_id, 
            &flp_lookups_tbl
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_elk_prog_config", 0, 0);
}

uint32
    arad_pp_flp_elk_prog_config_max_key_size_get(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  uint32  prog_id,
       SOC_SAND_IN  uint32  zone_id,
       SOC_SAND_OUT uint32   *max_key_size_in_bits
    )
{
    uint32
      res;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
      flp_lookups_tbl;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Get current Key-C size*/
   res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(
            unit, 
            prog_id, 
            &flp_lookups_tbl
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

	if(SOC_IS_JERICHO(unit)){
		if (zone_id == 0) {
	        *max_key_size_in_bits = ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS - (flp_lookups_tbl.elk_key_c_valid_bytes * SOC_SAND_NOF_BITS_IN_CHAR);
		}else{
	        *max_key_size_in_bits = ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS - (flp_lookups_tbl.elk_key_c_msb_valid_bytes * SOC_SAND_NOF_BITS_IN_CHAR);
	    }
	}else{
    	*max_key_size_in_bits = ARAD_PP_FLP_KEY_C_ZONE_SIZE_ARAD_PLUS_BITS - (flp_lookups_tbl.elk_key_c_valid_bytes * SOC_SAND_NOF_BITS_IN_CHAR);
	}

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_elk_prog_config_max_key_size_get", 0, 0);
}

STATIC
  uint32
   arad_pp_flp_ipv6uc_with_rpf_prog_init(
     int unit
   )
{
    uint32 vrf_ce_inst = 0,
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
      flp_key_construction_tbl;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
      flp_lookups_tbl;
    ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
      flp_process_tbl;
    uint32
      tmp;
    soc_reg_above_64_val_t
      reg_val;
    int32  
      prog_id;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);

    /* program selection */
    /* allocate resources */
    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6UC_RPF,FALSE,TRUE, &cam_sel_id, &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* configure program selection */
    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    prog_selection_cam_tbl.parser_leaf_context       = ARAD_PARSER_PLC_PP;
    prog_selection_cam_tbl.port_profile              = 0;
    prog_selection_cam_tbl.ptc_profile               = 0;
    prog_selection_cam_tbl.forwarding_code           = ARAD_PP_FWD_CODE_IPV6_UC;
    prog_selection_cam_tbl.in_rif_uc_rpf_enable      = 0x1;
    prog_selection_cam_tbl.parser_leaf_context_mask  = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
    prog_selection_cam_tbl.port_profile_mask         = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
    prog_selection_cam_tbl.ptc_profile_mask          = 0x03;
    prog_selection_cam_tbl.forwarding_code_mask      = 0x00;
    prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask = 0x0;
    prog_selection_cam_tbl.program                   = prog_id;
    prog_selection_cam_tbl.valid                     = 1;

    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* key construction */
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    flp_key_construction_tbl.instruction_0_16b        = vrf_ce_inst;
    flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_63_48
    flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_47_32
    flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_127_96
    flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_95_64
    flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_31_0
    flp_key_construction_tbl.key_a_inst_0_to_5_valid  = 0x19 /*hex 6'b011001*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid  = 0x26 /*6'b100110*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid  = 0x0  /*6'b000000*/;

    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    flp_key_construction_tbl.instruction_0_16b       = vrf_ce_inst;
    flp_key_construction_tbl.instruction_1_16b       = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_127_112
    flp_key_construction_tbl.instruction_2_16b       = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_111_96
    flp_key_construction_tbl.instruction_3_32b       = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_95_64
    flp_key_construction_tbl.instruction_4_32b       = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_63_32
    flp_key_construction_tbl.instruction_5_32b       = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_31_0
    flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid = SOC_IS_JERICHO(unit)? 0x3806 /*MSB: 6'b111000, LSB: 6'b000110*/: 0x3e /*hex 6'b111110*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

    /* reference: PROG_FLP_IPV4UC_WITH_RPF */
    /* flp lookups */
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    flp_lookups_tbl.elk_lkp_valid = 1;
    flp_lookups_tbl.elk_wait_for_reply = 1;
    flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF;

    flp_lookups_tbl.tcam_lkp_db_profile    = ARAD_TCAM_ACCESS_PROFILE_INVALID;
    flp_lookups_tbl.tcam_lkp_key_select    = ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL; /* Key-C */
    flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;
    flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;
    flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

    flp_lookups_tbl.elk_key_a_valid_bytes = 10; 
    flp_lookups_tbl.elk_key_b_valid_bytes = 8;  
    flp_lookups_tbl.elk_key_c_valid_bytes = 16; 

    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

    /* flp process */
    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    sal_memset(&flp_process_tbl, 0x0, sizeof(ARAD_PP_IHB_FLP_PROCESS_TBL_DATA));
    /*flp_process_tbl.include_tcam_in_result_a    = 1;*/
    flp_process_tbl.result_a_format             = 0;
    flp_process_tbl.result_b_format             = 0;
    flp_process_tbl.procedure_ethernet_default  = 0;
    flp_process_tbl.enable_hair_pin_filter      = 1;
    flp_process_tbl.enable_rpf_check            = 1;
    flp_process_tbl.sa_lkp_process_enable       = 0;
    flp_process_tbl.apply_fwd_result_a          = 1;
    /* take VRF default destination */
    flp_process_tbl.not_found_trap_strength     = 0;
    flp_process_tbl.not_found_trap_code         = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6;
    /* 0x0 - Use VRF Default Unicast
       0x1 - Use VRF Default Multicast
       Else: Use NotFoundTrapCode from program */
    flp_process_tbl.select_default_result_a     = 0; 
    flp_process_tbl.select_default_result_b     = 0;

    flp_process_tbl.elk_result_format = 1;
    flp_process_tbl.include_elk_fwd_in_result_a = 1;
    flp_process_tbl.include_elk_ext_in_result_a = 0;
    flp_process_tbl.include_elk_fwd_in_result_b = 0;
    flp_process_tbl.include_elk_ext_in_result_b = 1;

    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);


    /* 0: Apply Ethernet traps on forwarding-header
       1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
       2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
       3: Apply MPLS traps on forwarding-header
       4: Apply FC traps on forwarding-header
       Else: Don't apply any protocol traps */
    tmp = 2; 
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV6UC,&tmp,0,3);
    res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

  exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_ipv6uc_with_rpf_prog_init", 0, 0);
}
#endif

uint32
   arad_pp_flp_prog_sel_cam_ipv6mc(
     int unit,
     int32 *ipv6mc_sel_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(ipv6mc_sel_id);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV6MC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  *ipv6mc_sel_id = cam_sel_id;
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_IPV6_MC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable        = 0x0;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask   = (SOC_IS_JERICHO(unit)) ? 0x01 : 0x0;
  prog_selection_cam_tbl.program                     = PROG_FLP_IPV6MC;
  prog_selection_cam_tbl.valid                       = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv6mc", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_pwe_gre(
     int unit,
     int32  *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_VPLSOGRE,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  /* In case of PWEoGRE the forwarding code is not as expected, need special program selection */
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_ETHERNET; /* Ethernet processing instead of MPLS */
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.tt_processing_profile       = ARAD_PP_IHP_VTT_TT_PROCESSING_PROFILE_PWEoGRE;
  prog_selection_cam_tbl.tt_processing_profile_mask  = 0x0;
  prog_selection_cam_tbl.program                     = *prog_id;
  prog_selection_cam_tbl.valid                       = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_pwe_gre", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_lsr(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_LSR,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context         = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                = 0;
  prog_selection_cam_tbl.ptc_profile                 = 0;
  prog_selection_cam_tbl.forwarding_code             = ARAD_PP_FWD_CODE_MPLS;
  prog_selection_cam_tbl.parser_leaf_context_mask    = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask            = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask        = 0x00;
  prog_selection_cam_tbl.program                     = PROG_FLP_LSR;
  prog_selection_cam_tbl.valid                       = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_lsr", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_TRILL_uc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_TRILL_UC,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context     = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile            = 0;
  prog_selection_cam_tbl.ptc_profile             = 0;
  prog_selection_cam_tbl.forwarding_code         = ARAD_PP_FWD_CODE_TRILL;
  prog_selection_cam_tbl.trill_mc                = 0x0;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0xf;
  prog_selection_cam_tbl.port_profile_mask       = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask        = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask    = 0x00;
  prog_selection_cam_tbl.trill_mc_mask           = 0x0;  
  prog_selection_cam_tbl.program                 = PROG_FLP_TRILL_UC;
  prog_selection_cam_tbl.valid                   = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_TRILL", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_TRILL_mc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_TRILL_MC_ONE_TAG,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* One tag (VL) */
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile             = 0;
  prog_selection_cam_tbl.ptc_profile              = 0;
  prog_selection_cam_tbl.forwarding_code          = ARAD_PP_FWD_CODE_TRILL;
  prog_selection_cam_tbl.trill_mc                 = 0x1;
  prog_selection_cam_tbl.vt_lookup_1_found        = 0x0; /* VT1 looks up FGL TPID in ISEM. Indicates 1/2 tags */
  prog_selection_cam_tbl.parser_leaf_context_mask = 0xf;
  prog_selection_cam_tbl.port_profile_mask        = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask         = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask     = 0x00;
  prog_selection_cam_tbl.trill_mc_mask            = 0x0;  
  prog_selection_cam_tbl.vt_lookup_1_found_mask   = 0x0;
  prog_selection_cam_tbl.program                  = PROG_FLP_TRILL_MC_ONE_TAG;
  prog_selection_cam_tbl.valid                    = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* TWO tags (FGL) */
  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_TRILL_MC_TWO_TAGS,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    
  prog_selection_cam_tbl.parser_leaf_context      = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile             = 0;
  prog_selection_cam_tbl.ptc_profile              = 0;
  prog_selection_cam_tbl.forwarding_code          = ARAD_PP_FWD_CODE_TRILL;
  prog_selection_cam_tbl.trill_mc                 = 0x1;
  prog_selection_cam_tbl.vt_lookup_1_found        = 0x1; /* VT1 looks up FGL TPID in ISEM. Indicates 1/2 tags */
  prog_selection_cam_tbl.parser_leaf_context_mask = 0xf;
  prog_selection_cam_tbl.port_profile_mask        = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask         = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask     = 0x00;
  prog_selection_cam_tbl.trill_mc_mask            = 0x0;  
  prog_selection_cam_tbl.vt_lookup_1_found_mask   = 0x0;
  prog_selection_cam_tbl.program                  = PROG_FLP_TRILL_MC_TWO_TAGS;
  prog_selection_cam_tbl.valid                    = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_TRILL_mc", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_TRILL_mc_after_recycle_overlay(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_TRILL_AFTER_TERMINATION,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  prog_selection_cam_tbl.parser_leaf_context     = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile            = 0;
  prog_selection_cam_tbl.ptc_profile             = ARAD_PORTS_FLP_PROFILE_OVERLAY_RCY;
  prog_selection_cam_tbl.ptc_profile_mask        = 0x0;
  prog_selection_cam_tbl.forwarding_code         = ARAD_PP_FWD_CODE_TRILL;
  prog_selection_cam_tbl.trill_mc                = 0x1;
  prog_selection_cam_tbl.parser_leaf_context_mask = 0xf;
  prog_selection_cam_tbl.port_profile_mask       = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask        = 0x00;
  prog_selection_cam_tbl.forwarding_code_mask    = 0x00;
  prog_selection_cam_tbl.trill_mc_mask           = 0x0;  
  prog_selection_cam_tbl.program                 = PROG_FLP_TRILL_AFTER_TERMINATION;
  prog_selection_cam_tbl.valid                   = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_TRILL_mc_after_recycle_overlay", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_pon_vmac_upstream(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_VMAC_UPSTREAM,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
   
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prog_selection_cam_tbl.parser_leaf_context        = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.parser_leaf_context_mask   = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile               = 0;
  prog_selection_cam_tbl.port_profile_mask          = 0x00;
  prog_selection_cam_tbl.ptc_profile                = SOC_TMC_PORTS_FLP_PROFILE_PON;
  prog_selection_cam_tbl.ptc_profile_mask           = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.forwarding_code            = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.forwarding_code_mask       = 0x00;
  prog_selection_cam_tbl.cos_profile                = SOC_PPC_FLP_COS_PROFILE_VMAC;
  prog_selection_cam_tbl.cos_profile_mask           = 0x1F;
  prog_selection_cam_tbl.program                    = *prog_id;
  prog_selection_cam_tbl.valid                      = 1;

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_pon_vmac_upstream", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_pon_vmac_downstream(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_VMAC_DOWNSTREAM,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prog_selection_cam_tbl.parser_leaf_context        = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.parser_leaf_context_mask   = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile = 0;
  prog_selection_cam_tbl.port_profile_mask          = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile                = SOC_TMC_PORTS_FLP_PROFILE_NONE;
  prog_selection_cam_tbl.ptc_profile_mask           = 0x0;
  prog_selection_cam_tbl.forwarding_code            = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.forwarding_code_mask       = 0x00;
  prog_selection_cam_tbl.cos_profile                = SOC_PPC_FLP_COS_PROFILE_VMAC;
  prog_selection_cam_tbl.cos_profile_mask           = 0x1F;
  prog_selection_cam_tbl.program                    = *prog_id;
  prog_selection_cam_tbl.valid                      = 1;

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_pon_vmac_downstream", 0, 0);
}

uint32
   arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_dhcp(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
    uint32
        res,
        flp_prog_sel;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
        flp_lookups_tbl;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);      

    flp_prog_sel = PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP;
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

    flp_lookups_tbl.lem_1st_lkp_valid      = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
    flp_lookups_tbl.lem_1st_lkp_key_select = 1; /* Key B */
    flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
    flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
    flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);

    flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
    flp_lookups_tbl.lem_2nd_lkp_key_select = 0;    
    flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x7;
    flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK;
    flp_lookups_tbl.learn_key_select       = 1;

    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_dhcp", 0, 0);
}

uint32
   arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_static(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
    uint32
        res,
        flp_prog_sel;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
        flp_lookups_tbl;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);      
  
    flp_prog_sel = PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC;
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

    flp_lookups_tbl.lem_1st_lkp_valid      = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
    flp_lookups_tbl.lem_1st_lkp_key_select = 1; /* Key B */
    flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
    flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
    flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);

    flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
    flp_lookups_tbl.lem_2nd_lkp_key_select = 0;    
    flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
    flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK;

    if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_ENABLE(unit)) {
        flp_lookups_tbl.lpm_1st_lkp_valid      = 1;
        flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
        flp_lookups_tbl.lpm_1st_lkp_and_value  = 3;
        flp_lookups_tbl.lpm_1st_lkp_or_value   = 0;
        flp_lookups_tbl.learn_key_select       = 1;
    }
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_static", 0, 0);
}

uint32
   arad_pp_flp_lookups_ethernet_tk_epon_uni_v6(
     int unit,
     uint32 tcam_access_profile_id,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res,
    flp_prog_sel;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);      

  if (!SOC_IS_JERICHO(unit)){
    flp_prog_sel = PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC;
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
    flp_lookups_tbl.lem_1st_lkp_valid      = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
    flp_lookups_tbl.lem_1st_lkp_key_select = 1; /* Key B */
    flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
    flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
    flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
    
    if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
      flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
      flp_lookups_tbl.lem_2nd_lkp_key_select = 0; /* Key A */ 
      
      flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x7;
      flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK;
    } else {
      flp_lookups_tbl.lem_2nd_lkp_valid      = 0;
      flp_lookups_tbl.tcam_lkp_key_select    = ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL;
      flp_lookups_tbl.tcam_lkp_db_profile    = tcam_access_profile_id;
    }
    flp_lookups_tbl.learn_key_select   = 1;
    
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  }
  
  /* Don't install SAV DHCP because prefix value is conflict */
  if (!SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
    flp_prog_sel = PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP;
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

    flp_lookups_tbl.lem_1st_lkp_valid      = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
    flp_lookups_tbl.lem_1st_lkp_key_select = 1; /* Key B */
    flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
    flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
    flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);

    flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
    flp_lookups_tbl.lem_2nd_lkp_key_select = 0;    
    flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x7;
    flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK;

    flp_lookups_tbl.learn_key_select   = 1;

    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, flp_prog_sel, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_tk_epon_uni_v6", 0, 0);
}

uint32
   arad_pp_flp_lookups_pon_arp_downstream(
     int unit,
     int prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK;
  flp_lookups_tbl.lpm_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lpm_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lpm_2nd_lkp_and_value = 3;
  flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_pon_arp_downstream", 0, 0);
}

uint32
   arad_pp_flp_lookups_pon_arp_upstream(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_lookups_tbl.lem_1st_lkp_valid      = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_lookups_tbl.lem_1st_lkp_key_select = 1; /* Key B */
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0;    
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK;
  flp_lookups_tbl.lpm_1st_lkp_valid     = 1;
  flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
  flp_lookups_tbl.lpm_1st_lkp_and_value = 3;
  flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
  flp_lookups_tbl.learn_key_select   = 1;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_pon_arp_upstream", 0, 0);
}


/* Get ingress learn mode of anti-spoofing V6*/
uint32
   arad_pp_flp_tk_epon_uni_v6_ing_learn_get(
     int unit,
     uint8 *ingress_learn_enable,
     uint8 *ingress_learn_oppurtunistic
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  if (flp_process_tbl.lem_1st_lkp_valid)
  {
    *ingress_learn_enable = 1;
    *ingress_learn_oppurtunistic = 0;
  }
  else
  {
    *ingress_learn_enable = 0;
    *ingress_learn_oppurtunistic = 0;
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_tk_epon_uni_v6_ing_learn_get", 0, 0);
}

/* Get tcam_lkp_db_profile of static anti-spoofing V6*/
uint32
   arad_pp_flp_tk_epon_uni_v6_tcam_profile_get(
     int unit,
     uint32 *tcam_access_profile_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  *tcam_access_profile_id = flp_process_tbl.tcam_lkp_db_profile;


exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_tk_epon_uni_v6_tcam_profile_get", 0, 0);
}

uint32
   arad_pp_flp_lookups_ethernet_ing_learn(
     int unit,
     uint8 ingress_learn_enable,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_lookups_ethernet_ing_learn ingress_learn_oppurtunistic: %h",ingress_learn_oppurtunistic);*/
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = (ingress_learn_enable &&(!sa_auth_enabled && !slb_enabled))? 1 : 0;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.learn_key_select   = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (ARAD_KBP_ENABLE_P2P_EXTENDED) {
	  flp_lookups_tbl.elk_lkp_valid = 0x1;
	  flp_lookups_tbl.elk_wait_for_reply = 0x1;
	  flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_P2P; /* to acquire inlif mapping both eth,p2p access the same table */

	  flp_lookups_tbl.elk_key_a_valid_bytes = 0;
	  flp_lookups_tbl.elk_key_b_valid_bytes = 2;
	  flp_lookups_tbl.elk_key_c_valid_bytes = 0; 
  }
#endif

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_ing_learn", 0, 0);
}


uint32
   arad_pp_flp_lookups_ethernet_ing_ivl_learn(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_lookups_ethernet_ing_learn ingress_learn_oppurtunistic: %h",ingress_learn_oppurtunistic);*/
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 204, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;
  flp_lookups_tbl.lem_1st_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_A_KEY_HW_VAL;   /* Key A */
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_B_KEY_HW_VAL; /* Key B */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.learn_key_select   = 0; /* Select Learned Key 63 lsbs */

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 204, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_ing_ivl_learn", 0, 0);
}

uint32
   arad_pp_flp_lookups_ethernet_pon_default_upstream(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);    
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 0;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = 0;
  flp_lookups_tbl.learn_key_select   = 0;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_pon_default_upstream", 0, 0);
}


uint32
   arad_pp_flp_lookups_ethernet_pon_default_downstream(
     int unit,
     uint32 tcam_access_profile_id,
     uint8 ingress_learn_enable, /* = 1,*/
     uint8 ingress_learn_oppurtunistic, /* = 0*/
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);    
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (SOC_DPP_CONFIG(unit)->pp.compression_ip6_enable) {
    flp_lookups_tbl.lem_1st_lkp_valid      =  1;
    flp_lookups_tbl.lem_1st_lkp_key_select = 0; /* Key A */
    flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
    flp_lookups_tbl.lem_1st_lkp_and_value  = 0;
    res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_IP6_COMPRESSION_DIP_KEY_OR_MASK,&flp_lookups_tbl.lem_1st_lkp_or_value);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    flp_lookups_tbl.tcam_lkp_key_select    = ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL;
    flp_lookups_tbl.tcam_lkp_db_profile    = tcam_access_profile_id;
  }
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.learn_key_select   = 0;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_pon_default_downstream", 0, 0);
}


uint32
   arad_pp_flp_lookups_ethernet_pon_local_route(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_lookups_ethernet_ing_learn ingress_learn_oppurtunistic: %h",ingress_learn_oppurtunistic);*/
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.learn_key_select   = 0;


  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_pon_local_route", 0, 0);
}


uint32
   arad_pp_flp_lookups_TRILL_mc_after_recycle_overlay(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_lookups_ethernet_ing_learn ingress_learn_oppurtunistic: %h",ingress_learn_oppurtunistic);*/
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit,PROG_FLP_TRILL_AFTER_TERMINATION , &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.learn_key_select   = 0;


  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit,PROG_FLP_TRILL_AFTER_TERMINATION , &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_TRILL_mc_after_recycle_overlay", 0, 0);
}

uint32
   arad_pp_flp_lookups_ethernet_mac_in_mac(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_MAC_IN_MAC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; /* key for LEM search: {11b'0, FID, Forwarding-DA} */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_B_ETH_KEY_OR_MASK(unit);

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_MAC_IN_MAC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ethernet_mac_in_mac", 0, 0);
}

uint32
   arad_pp_flp_key_const_mac_in_mac_after_termination(
     int unit,
     int32 prog_id
   )
{
  uint32
      res, vsi_ce_inst = 0, pp_port_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI,0, &vsi_ce_inst);

  if (SOC_IS_JERICHO(unit)) {
      arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT, 0, &pp_port_ce_inst);
  }

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  
  if (SOC_DPP_CONFIG(unit)->pp.test2) {
      flp_key_construction_tbl.instruction_0_16b        = vsi_ce_inst;
      flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x19 /*6'b011001*/;
  } else {
      flp_key_construction_tbl.instruction_0_16b        =  (SOC_IS_JERICHO(unit) ? pp_port_ce_inst : ARAD_PP_FLP_16B_INST_P6_IN_PORT_D);
      flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_ETH_HEADER_OUTER_TAG;
      flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x1B /*6'b011011*/;
  }
  
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_24MSB;  
  flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SA_24LSB;  
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_mac_in_mac_after_termination", 0, 0);
}

uint32
   arad_pp_flp_lookups_mac_in_mac_after_termination(
     int unit,
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid      = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_MAC_IN_MAC_TUNNEL_KEY_OR_MASK; /* MAC Tunnel Learn DB */
  flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit); /* CMACT DB */
  flp_lookups_tbl.learn_key_select       = 0;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_mac_in_mac_after_termination", 0, 0);
}

uint32
   arad_pp_flp_process_mac_in_mac_after_termination(
     int unit,
     uint8 learn_enable,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.sa_lkp_result_select           = 0;
  flp_process_tbl.sa_lkp_process_enable          = 0;
  flp_process_tbl.procedure_ethernet_default     = 3;
  flp_process_tbl.enable_hair_pin_filter         = 1;
  flp_process_tbl.learn_enable                   = learn_enable;
  flp_process_tbl.not_found_trap_strength        = 0;
  flp_process_tbl.unknown_address                = 3;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_mac_in_mac_after_termination", 0, 0);
}


uint32
   arad_pp_flp_process_key_program_tm(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;

  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_tm");*/

  ARAD_CLEAR(&flp_process_tbl, ARAD_PP_IHB_FLP_PROCESS_TBL_DATA, 1);
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_TM, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 0;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_TM,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);


exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_key_program_tm", 0, 0);
}


uint32
   arad_pp_flp_process_tm(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_tm");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_TM, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 0;
  flp_process_tbl.include_lem_1st_in_result_b    = 0;
  flp_process_tbl.include_lpm_2nd_in_result_a    = 0;
  flp_process_tbl.include_lpm_1st_in_result_b    = 0;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.procedure_ethernet_default  = 0;
  flp_process_tbl.enable_hair_pin_filter       = 0;
  flp_process_tbl.enable_rpf_check            = 0;
  /* only thing matter, use program trap which is disabled, prevent use IPv4 default routing action */
  flp_process_tbl.select_default_result_b = 2;
  flp_process_tbl.select_default_result_a = 2;
  flp_process_tbl.not_found_trap_strength = 0;
  flp_process_tbl.not_found_snoop_strength = 0;
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_TM, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 7; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_TM,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_tm", 0, 0);
}


uint32
   arad_pp_flp_lookups_p2p(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_P2P, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (ARAD_KBP_ENABLE_P2P_EXTENDED) {
      flp_lookups_tbl.elk_lkp_valid = 0x1;
      flp_lookups_tbl.elk_wait_for_reply = 0x1;
      flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_P2P;

      flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

      flp_lookups_tbl.elk_key_a_valid_bytes = 2;
      flp_lookups_tbl.elk_key_b_valid_bytes = 0;
      flp_lookups_tbl.elk_key_c_valid_bytes = 0; 
  }
#endif

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_P2P, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 243, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_p2p", 0, 0);
}

uint32
   arad_pp_flp_process_p2p(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_P2P, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  
  flp_process_tbl.procedure_ethernet_default   = 0;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.apply_fwd_result_a           = 0;
  flp_process_tbl.sa_lkp_process_enable        = 0;
  flp_process_tbl.learn_enable                 = 0;
  flp_process_tbl.not_found_trap_strength      = 0;

  flp_process_tbl.unknown_address = 0; /* in this case the unknown_address will not be set */

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (ARAD_KBP_ENABLE_P2P_EXTENDED) {
    flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_P2P_MISCONFIGURATION;
    flp_process_tbl.elk_result_format = 1;
    /* ignoring the elk result in the FLP (only used in the PMF) */
    flp_process_tbl.include_elk_fwd_in_result_a = 0;
    flp_process_tbl.include_elk_ext_in_result_a = 0;
    flp_process_tbl.include_elk_fwd_in_result_b = 0;
    flp_process_tbl.include_elk_ext_in_result_b = 0;
  }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_P2P, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 0; 
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_P2P,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_p2p", 0, 0);
}

uint32
   arad_pp_flp_key_const_p2p(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_P2P, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  #if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (ARAD_KBP_ENABLE_P2P_EXTENDED) {        
      flp_key_construction_tbl.instruction_0_16b       = ARAD_PP_FLP_16B_INST_P6_IN_LIF_D;
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x1 /*hex 6'b000001 */;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_P2P, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_P2P+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;      
  }
  #endif

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_P2P+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_p2p", 0, 0);
}


uint32
   arad_pp_flp_process_ethernet_tk_epon_uni_v4(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  uint32
    tmp,
    flp_prog_sel;
  int
    i;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (i = 0; i < 2; i++) { /* DHCP, static */
    /* Don't install SAV DHCP because prefix value is conflict */
    if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable && (i==0)) {
      continue;
    }

    flp_prog_sel = (i == 0) ? PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP:PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC;
    
    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, flp_prog_sel, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    flp_process_tbl.include_lem_1st_in_result_b    = 1;
    flp_process_tbl.result_a_format                = 0;
    flp_process_tbl.result_b_format                = 0;
    flp_process_tbl.sa_lkp_result_select           = 1;
    flp_process_tbl.sa_lkp_process_enable          = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
    flp_process_tbl.procedure_ethernet_default     = 3;
    flp_process_tbl.enable_hair_pin_filter         = 1;
    flp_process_tbl.learn_enable                   = learn_enable;
    flp_process_tbl.select_default_result_b        = 3;
    flp_process_tbl.not_found_trap_code            = 0;
    flp_process_tbl.not_found_trap_strength        = 0;
    flp_process_tbl.not_found_snoop_strength       = 0;

    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, flp_prog_sel, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    SHR_BITCOPY_RANGE(reg_val,3*flp_prog_sel,&tmp,0,3);
    res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_tk_epon_uni_v4", 0, 0);
}

uint32
   arad_pp_flp_process_ethernet_tk_epon_uni_v6(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  uint32
    tmp,
    flp_prog_sel;
  int
    i;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  flp_prog_sel = PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP;



  for (i = 0; i < 2; i++) { /* DHCP, static */
    /* Don't install SAV DHCP because prefix value is conflict */
    if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable && (i==0)) {
      continue;
    }

    flp_prog_sel = (i == 0) ? PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP:PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC;
    
    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, flp_prog_sel, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    
    flp_process_tbl.include_lem_1st_in_result_b    = 1;        
    flp_process_tbl.result_a_format                = 0;
    flp_process_tbl.result_b_format                = 0;
    flp_process_tbl.sa_lkp_result_select           = 1;
    flp_process_tbl.sa_lkp_process_enable          = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
    flp_process_tbl.procedure_ethernet_default     = 3;
    flp_process_tbl.enable_hair_pin_filter         = 1;
    flp_process_tbl.learn_enable                   = learn_enable;
    flp_process_tbl.select_default_result_b        = 3;
    flp_process_tbl.not_found_trap_code            = 0;
    flp_process_tbl.not_found_trap_strength        = 0;
    flp_process_tbl.not_found_snoop_strength       = 0;

    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, flp_prog_sel, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    SHR_BITCOPY_RANGE(reg_val,3*flp_prog_sel,&tmp,0,3);
    res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_tk_epon_uni_v6", 0, 0);
}

uint32
   arad_pp_flp_process_pon_arp_downstream(
     int unit,
     int prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
  flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.procedure_ethernet_default     = 3;
  flp_process_tbl.enable_hair_pin_filter         = 1;
  flp_process_tbl.enable_rpf_check               = 0;
  flp_process_tbl.not_found_trap_code            = 0;
  flp_process_tbl.not_found_trap_strength        = 0;
  flp_process_tbl.not_found_snoop_strength       = 0;
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_pon_arp_downstream", 0, 0);
}

uint32
   arad_pp_flp_process_pon_arp_upstream(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int prog_id
   )
{
  uint32
    res;
  uint32
    tmp;

  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.sa_lkp_result_select           = 1;
  flp_process_tbl.sa_lkp_process_enable          = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default     = 3;
  flp_process_tbl.enable_hair_pin_filter         = 1;
  flp_process_tbl.learn_enable                   = learn_enable;
  flp_process_tbl.select_default_result_b        = 3;
  flp_process_tbl.not_found_trap_code            = 0;
  flp_process_tbl.not_found_trap_strength        = 0;
  flp_process_tbl.not_found_snoop_strength       = 0;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_pon_arp_upstream", 0, 0);
}



uint32
   arad_pp_flp_process_oam_statistics(
     int unit,
     uint32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = 1;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = 0;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; 
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_oam_statistics", 0, 0);
}

uint32
   arad_pp_flp_process_bfd_statistics(
     int unit,
     uint32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  ARAD_CLEAR(&flp_process_tbl, ARAD_PP_IHB_FLP_PROCESS_TBL_DATA, 1);

 /* flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0; */

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 0;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_oam_statistics", 0, 0);
}

uint32
   arad_pp_flp_process_vpws_tagged(int unit)
{
  uint32 res;
  uint32 tmp;
  soc_reg_above_64_val_t reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA flp_process_tbl;
  uint8 is_single_tag;
  uint32 flp_prog;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  for (is_single_tag=0; is_single_tag<=1; is_single_tag++) {
      flp_prog = is_single_tag ? PROG_FLP_VPWS_TAGGED_SINGLE_TAG:PROG_FLP_VPWS_TAGGED_DOUBLE_TAG;
      res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, flp_prog, &flp_process_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

      flp_process_tbl.include_lem_1st_in_result_a   = 1;
      flp_process_tbl.include_lem_2nd_in_result_a   = 1;
      flp_process_tbl.result_a_format               = 0;
      flp_process_tbl.result_b_format               = 0;
      flp_process_tbl.sa_lkp_result_select          = 0;
      flp_process_tbl.sa_lkp_process_enable         = 0;
      flp_process_tbl.procedure_ethernet_default    = 3;
      flp_process_tbl.enable_hair_pin_filter        = 1;
      flp_process_tbl.learn_enable                  = 0;
      flp_process_tbl.not_found_trap_strength       = 0;
      flp_process_tbl.unknown_address               = 0;

      res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, flp_prog, &flp_process_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

      tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
      SOC_REG_ABOVE_64_CLEAR(reg_val);
      res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
      SHR_BITCOPY_RANGE(reg_val,3*flp_prog,&tmp,0,3);
      res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_vpws_tagged", 0, 0);
}


uint32
   arad_pp_flp_process_ethernet_ing_learn(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ethernet_ing_learn ingress_learn_enable: %h ingress_learn_oppurtunistic: %h",
                          ingress_learn_enable,ingress_learn_oppurtunistic);*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (ARAD_KBP_ENABLE_P2P_EXTENDED) {
	  flp_process_tbl.elk_result_format = 1;
	  /* ignoring the elk result in the FLP (only used in the PMF) */
	  flp_process_tbl.include_elk_fwd_in_result_a = 0;
	  flp_process_tbl.include_elk_ext_in_result_a = 0;
	  flp_process_tbl.include_elk_fwd_in_result_b = 0;
	  flp_process_tbl.include_elk_ext_in_result_b = 0;
  }
#endif
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_ETHERNET_ING_LEARN,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_ing_learn", 0, 0);
}


uint32
   arad_pp_flp_process_ethernet_ing_all_ivl_learn(
     int unit,
     uint8 learn_enable, /* ? == TRUE*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ethernet_ing_learn ingress_learn_enable: %h ingress_learn_oppurtunistic: %h",
                          ingress_learn_enable,ingress_learn_oppurtunistic);*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 401, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1; /* Result B for SA Lookup */
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (ARAD_KBP_ENABLE_P2P_EXTENDED) {
	  flp_process_tbl.elk_result_format = 1;
	  /* ignoring the elk result in the FLP (only used in the PMF) */
	  flp_process_tbl.include_elk_fwd_in_result_a = 0;
	  flp_process_tbl.include_elk_ext_in_result_a = 0;
	  flp_process_tbl.include_elk_fwd_in_result_b = 0;
	  flp_process_tbl.include_elk_ext_in_result_b = 0;
  }
#endif
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 401, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 401, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 401, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_ing_all_ivl_learn", 0, 0);
}



uint32
   arad_pp_flp_process_ethernet_pon_default_upstream(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 0;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_pon_default_upstream", 0, 0);
}


uint32
   arad_pp_flp_process_ethernet_pon_default_downstream(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 ingress_learn_enable, /* = 1*/
     uint8 ingress_learn_oppurtunistic,/* = 0*/
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 0;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = 0;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;
  if (SOC_DPP_CONFIG(unit)->pp.compression_ip6_enable) {
    flp_process_tbl.include_lem_1st_in_result_b    = 1;
    flp_process_tbl.include_tcam_in_result_a       = 0;
  }

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_pon_default_downstream", 0, 0);
}


uint32
   arad_pp_flp_process_ethernet_pon_local_route(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_pon_local_route", 0, 0);
}


uint32
   arad_pp_flp_process_TRILL_mc_after_recycle_overlay(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ethernet_ing_learn ingress_learn_enable: %h ingress_learn_oppurtunistic: %h",
                          ingress_learn_enable,ingress_learn_oppurtunistic);*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_TRILL_AFTER_TERMINATION, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_TRILL_AFTER_TERMINATION, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_TRILL_AFTER_TERMINATION,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_TRILL_mc_after_recycle_overlay", 0, 0);
}


uint32
   arad_pp_flp_process_pon_vmac_upstream(
     int unit,
     uint32 prog_id,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.include_lem_1st_in_result_a    = 0;
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_2nd_in_result_b    = 0;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.sa_lkp_result_select           = 1;
  flp_process_tbl.sa_lkp_process_enable          = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default     = 2;
  flp_process_tbl.enable_hair_pin_filter         = 1;
  flp_process_tbl.learn_enable                   = learn_enable;
  flp_process_tbl.select_default_result_b        = 3;
  flp_process_tbl.select_default_result_a        = 3;
  flp_process_tbl.not_found_trap_code            = 0;
  flp_process_tbl.not_found_trap_strength        = 0;
  flp_process_tbl.not_found_snoop_strength       = 0;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_pon_vmac_upstream", 0, 0);
}

uint32
   arad_pp_flp_process_pon_vmac_downstream(
     int unit,
     uint32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.sa_lkp_result_select           = 1;
  flp_process_tbl.sa_lkp_process_enable          = 0;
  flp_process_tbl.procedure_ethernet_default     = 0;
  flp_process_tbl.enable_hair_pin_filter         = 1;
  flp_process_tbl.learn_enable                   = 0;
  flp_process_tbl.select_default_result_b        = 3;
  flp_process_tbl.select_default_result_a        = 3;
  flp_process_tbl.not_found_trap_code            = 0;
  flp_process_tbl.not_found_trap_strength        = 0;
  flp_process_tbl.not_found_snoop_strength       = 0;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_pon_vmac_upstream", 0, 0);
}


uint32
   arad_pp_flp_ethernet_prog_learn_get(
     int unit,
     uint8 *learn_enable
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ethernet_ing_learn ingress_learn_enable: %h ingress_learn_oppurtunistic: %h",
                          ingress_learn_enable,ingress_learn_oppurtunistic);*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  *learn_enable = flp_process_tbl.learn_enable;


exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_ethernet_prog_learn_get", 0, 0);
}


uint32
   arad_pp_flp_process_ethernet_mac_in_mac(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_MAC_IN_MAC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.result_a_format                = 1;
  flp_process_tbl.sa_lkp_result_select           = 0;
  flp_process_tbl.sa_lkp_process_enable          = 1;
  flp_process_tbl.procedure_ethernet_default     = 3;
  flp_process_tbl.enable_hair_pin_filter         = 1;
  flp_process_tbl.learn_enable                   = 0;
  flp_process_tbl.not_found_trap_strength        = 0;


  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_MAC_IN_MAC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);

  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  SHR_BITCOPY_RANGE(reg_val, 3*PROG_FLP_ETHERNET_MAC_IN_MAC, &tmp, 0, 3);

  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ethernet_ing_learn", 0, 0);
}

uint32
   arad_pp_flp_key_const_ethernet_tk_epon_uni_v4_dhcp(
     int unit
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  if (!SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
    /* DHCP */
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

    flp_key_construction_tbl.instruction_0_16b        = ARAD_PP_FLP_16B_INST_P6_TT_LOOKUP0_PAYLOAD_D; /* spoof id (SOC_IS_JERICHO(unit)? 0xfcac : 0xf068)*/
    flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_CE_SA_FWD_HEADER_40_32_CB16; /* SA 32:40 */
    flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB; /* SA 0:31 */
    if (SOC_IS_JERICHO(unit)){
        flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_FLP_32B_INST_P6_GLOBAL_IN_LIF_D; /* In LIF */
        flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x35 /*6'b110101*/;
    }
    else{
       flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_FLP_16B_INST_P6_IN_LIF_D;
       flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x17 /*6'b010111*/;
    }    
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
    flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
    flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
    flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0xB /*6'b001011*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  }


exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_tk_epon_uni_v4_dhcp", 0, 0);
}


uint32
   arad_pp_flp_key_const_ethernet_tk_epon_uni_v4_static(
     int unit
   )
{
    uint32
        res, fid_ce_inst = 0;
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
        flp_key_construction_tbl;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

    /* Static */
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 143, exit);

    if (SOC_IS_JERICHO(unit)){
        flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_IPV4_SIP_HDR2_HEADER;
    }
    else{
        flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_IPV4_SIP_HDR2_HEADER;
    }

    if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit)) {
        flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
        flp_key_construction_tbl.key_a_inst_0_to_5_valid  = (SOC_IS_JERICHO(unit))?0x21:0x9 ; /* Jericho 6'b100001 , Arad 6'b001001*/
    } else {
        if (SOC_IS_JERICHO(unit)){
            flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_FLP_32B_INST_P6_GLOBAL_IN_LIF_D; /* In LIF */
            flp_key_construction_tbl.key_a_inst_0_to_5_valid  = 0x30 /*6'b011000*/;
        }
        else{
            flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_FLP_16B_INST_P6_IN_LIF_D;
            flp_key_construction_tbl.key_a_inst_0_to_5_valid  = 0xC /*6'b001100*/;
        }
    }

    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 144, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);
    flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
    flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
    flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0xB /*6'b001011*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 146, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_tk_epon_uni_v4_static", 0, 0);
}

uint32
   arad_pp_flp_key_const_ethernet_tk_epon_uni_v6(
     int unit
   )
{
  uint32
    res, fid_ce_inst = 0, tt9b_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 9, SOC_PPC_FP_QUAL_TT_LOOKUP1_PAYLOAD,0, &tt9b_ce_inst);

  if (!SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
    /* DHCP */
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

    flp_key_construction_tbl.instruction_0_16b        = ARAD_PP_FLP_16B_INST_P6_TT_LOOKUP0_PAYLOAD_D; /* Spoof-ID */
    flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_CE_SA_FWD_HEADER_40_32_CB16; /* SA 40:32*/
    flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  /* SA */
    if (SOC_IS_JERICHO(unit)){
       flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_FLP_32B_INST_P6_GLOBAL_IN_LIF_D; /* In LIF */
       flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x35 /*6'b110101*/;
    }
    else{
       flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_FLP_16B_INST_P6_IN_LIF_D; /* In LIF */
       flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x17 /*6'b010111*/;
    }
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
    flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
    flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
    flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0xB /*6'b001011*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;  
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);
  }
  
  if (!SOC_IS_JERICHO(unit)){
      /* static */
      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 143, exit);
      if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
         flp_key_construction_tbl.instruction_0_16b        = tt9b_ce_inst; /* TCAM lookup result */
         flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_63_32; /* SIP 31:0 */
         flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_31_0; /* SIP 63:32 */
         flp_key_construction_tbl.key_a_inst_0_to_5_valid    = SOC_IS_JERICHO(unit)? 0x1801 /*MSB: 6'b011000, LSB: 6'b000001*/: 0x19 /*6'b011001*/;
         flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
         flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
      } else {
         flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_FLP_16B_INST_P6_IN_LIF_D; /* In AC */
         flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_127_96;  
         flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_95_64;  
         flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_63_32;  
         flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
         flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
         flp_key_construction_tbl.key_c_inst_0_to_5_valid    = SOC_IS_JERICHO(unit)? 0x3804 /*MSB: 6'b111000, LSB: 6'b000100*/: 0x3C /*6'b111100*/;
      }
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 144, exit);
      
      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);
      flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_31_0;
      flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
      flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
      flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;
      flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x13 /*6'b010011*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid    = (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable ? 0x0: 0x8) /*6'b001000*/;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 146, exit);
   }
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_tk_epon_uni_v6", 0, 0);
}

uint32
   arad_pp_flp_key_const_pon_arp_downstream(
     int unit,
     int prog_id
   )
{
  uint32
    res, fid_ce_inst;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_ARP_TPA_HEADER_2;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x9 /*6'b001001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_pon_arp_downstream", 0, 0);
}


uint32
   arad_pp_flp_key_const_pon_arp_upstream(
     int unit,
     int prog_id
   )
{
  uint32
    res, fid_ce_inst;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  /* Static */
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_key_construction_tbl.instruction_0_16b          = fid_ce_inst;
  flp_key_construction_tbl.instruction_3_32b          = ARAD_PP_CE_ARP_SPA_HEADER_2;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x9 /*6'b001001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0xB /*6'b001011*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_pon_arp_upstream", 0, 0);
}


uint32
   arad_pp_flp_key_const_ethernet_ing_learn(
     int unit
   )
{
  uint32
    res, fid_ce_inst;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*
   * 1. Build the Source-Address lookup explictly because the default one is LL-SA not 
   * usable when termination (different from forwarding header) 
   * 2. Use the default DA lookup (forwarding header), no need for instructions (key-B) 
   * 3. Use the instructions 0, 3, 3 for SA lookup to be identical to instructions taken in 
   * IPv4 UC KBP lookup and maximize the shared ELK ACL definition on these programs
   */

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB_CB32;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x9 /*6'b001001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
	if (ARAD_KBP_ENABLE_P2P_EXTENDED) { /* put IN-LIF on key-b*/
        flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_FLP_16B_INST_P6_IN_LIF_D;
		flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x2 /*6'b000010*/;
	}
#endif

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x8 /*6'b001000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_ETHERNET_ING_LEARN+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_ing_learn", 0, 0);
}


uint32
   arad_pp_flp_key_const_ethernet_pon_default_upstream(
     int unit,
     int32 prog_id
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0xB /*6'b001011*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_pon_default_upstream", 0, 0);
}


uint32
   arad_pp_flp_key_const_ethernet_pon_default_downstream(
     int unit,
     int32 prog_id
   )
{
  uint32
    res, tt12b_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 12, SOC_PPC_FP_QUAL_TT_LOOKUP1_PAYLOAD,0, &tt12b_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if (SOC_DPP_CONFIG(unit)->pp.compression_ip6_enable) {
    flp_key_construction_tbl.instruction_0_16b        = tt12b_ce_inst; /* TCAM lookup result */
    flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_DIP_IPV6_HDR2_HEADER_55_32; /* DIP 55:32 */
    flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_DIP_IPV6_HDR2_HEADER_31_0; /* DIP 31:0 */
    if (SOC_IS_JERICHO(unit)) {
        flp_key_construction_tbl.instruction_1_16b    = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_127_112; 
        flp_key_construction_tbl.instruction_5_32b    = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_111_80;
    } else { 
        flp_key_construction_tbl.instruction_5_32b    = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_127_96; /* SIP 127:96 */
    }
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x19 /*6'b011001*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = SOC_IS_JERICHO(unit) ? 0x2200 : 0x20/*6'b100000*/;
  } else {
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b001011*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  if (SOC_DPP_CONFIG(unit)->pp.compression_ip6_enable) {
    if (SOC_IS_JERICHO(unit)) {
        flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_79_64;
    } else {
        flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_95_64;
    }
    flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_63_32;  
    flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_IPV6_SIP_HDR2_HEADER_31_0;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid  = 0x38;
  } else {
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;   
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_pon_default_downstream", 0, 0);
}


uint32
   arad_pp_flp_key_const_ethernet_pon_local_route(
     int unit,
     int32 prog_id
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
    
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0xB /*6'b001011*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ethernet_pon_local_route", 0, 0);
}


uint32
   arad_pp_flp_key_const_TRILL_mc_after_recycle_overlay(
     int unit
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_TRILL_AFTER_TERMINATION, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  /* Take FID+SA from native header */
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_TRILL_NATIVE_DA_32MSB;
  flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_TRILL_NATIVE_DA_16LSB;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0  /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x31 /*6'b011001*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0  /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_TRILL_AFTER_TERMINATION, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_TRILL_AFTER_TERMINATION+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_TRILL_NATIVE_SA_32MSB;
  flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_TRILL_NATIVE_SA_16LSB; 
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x19 /*6'b011001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0  /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0  /*6'b000000*/;    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_TRILL_AFTER_TERMINATION+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_TRILL_mc_after_recycle_overlay", 0, 0);
}

uint32
   arad_pp_flp_key_const_ipv4uc_l3vpn(
     int unit
   )
{
  uint32
    res, vrf_ce_inst;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_PUBLIC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_DIP_FWD_HEADER;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x8 /*6'b001000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_PUBLIC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_PUBLIC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_CE_DIP_FWD_HEADER;
  flp_key_construction_tbl.instruction_0_16b     = vrf_ce_inst;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x9 /*6'b001001*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_PUBLIC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv4uc_with_l3vpn", 0, 0);
}

uint32
   arad_pp_flp_key_const_ipv6uc_with_rpf_2pass(
     int unit,
     int32 prog_id
   )
{
  uint32
    res, vrf_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* key construction */
/*    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV6UC_WITH_RPF_2PASS, &flp_key_construction_tbl); */
    res = arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 240, exit);

    flp_key_construction_tbl.instruction_0_16b        = vrf_ce_inst;
    flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_63_48
    flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_47_32
    flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_127_96
    flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_95_64
    flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_31_0
    flp_key_construction_tbl.key_a_inst_0_to_5_valid  = 0x19 /*hex 6'b011001*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid  = 0x26 /*hex 6'b100110*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid  = 0x0  /*hex 6'b000000*/;

/*    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV6UC_WITH_RPF_2PASS, &flp_key_construction_tbl); */
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 241, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv6uc_with_rpf_2pass", 0, 0);
}

uint32
   arad_pp_flp_key_const_ipv4uc_rpf(
     int unit
   )
{
  uint32
    res, vrf_ce_inst = 0, rif_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 1, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_RPF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b        = vrf_ce_inst;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SIP_FWD_HEADER;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x9 /*6'b001001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_RPF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_RPF+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_CE_DIP_FWD_HEADER;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x8; /*6'b001000 */
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0; 
	  if(ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){		
		flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_N_ZEROS(4);
		flp_key_construction_tbl.instruction_5_32b     = rif_ce_inst;
		flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x38; /*6'b111000 */
	}
  }else 
#endif
  {
      flp_key_construction_tbl.instruction_0_16b     = vrf_ce_inst;
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x9 /*6'b001001*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_RPF+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv4uc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_key_const_ipv4uc(
     int unit
   )
{
  uint32
    res, vrf_ce_inst, rif_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 1, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);
    

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4UC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b        = vrf_ce_inst;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SIP_FWD_HEADER;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x9 /*6'b001001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if( ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED & 
     (SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1)){
      flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b001001*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x9 /*6'b000000*/;
  }
  #endif
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4UC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4UC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_CE_DIP_FWD_HEADER;
  
  
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x8; /*6'b001000 */
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0; 
	  if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
		flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x38; /*6'b111000 */
		flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_N_ZEROS(4);
		flp_key_construction_tbl.instruction_5_32b     = rif_ce_inst;
		if (SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1) {
              flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0;
              flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0; /*6'b001000 */
              flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x38;
          }
	  }
  }
  else
#endif
  {
      flp_key_construction_tbl.instruction_0_16b     = vrf_ce_inst;
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x9 /*6'b001001*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4UC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv4uc", 0, 0);
}

uint32
   arad_pp_flp_key_const_bfd_single_hop(
     int unit,
	 int32  prog
   )
{
  uint32
    res, vrf_ce_inst = 0, header4_ce_inst = 0, ttl_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_HEADER_OFFSET4,0, &header4_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_TTL,0, &ttl_ce_inst);
    

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b        = ttl_ce_inst;

  flp_key_construction_tbl.instruction_1_16b        = header4_ce_inst;
  
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_YOUR_DESCRIMINATOR;

  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x9  /*6'b001001*/  ; /* Use instructions 0, 3. Inst.*/
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x2 /*6'b000010*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_bfd_single_hop", 0, 0);
}

uint32
   arad_pp_flp_key_const_ipv6uc(
     int unit
   )
{
  uint32
    res, vrf_ce_inst;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV6UC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b        = vrf_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_63_48
  flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_47_32
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_127_96
  flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_95_64
  flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_31_0

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_EXTENDED){
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x19;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x26; 
      if(ARAD_KBP_ENABLE_IPV6_EXTENDED){
          flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x1; /* No SIP */
          flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0; 
      }
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0; 
  }
  else
#endif
  {
      flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x3F /*hex 6'b111111*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV6UC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV6UC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b     = vrf_ce_inst;
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_127_112
  flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_111_96
  flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_95_64
  flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_63_32
  flp_key_construction_tbl.instruction_5_32b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_31_0

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_EXTENDED){
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0; 
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0; 
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3e; /*hex 6'b111110*/
  }
  else
#endif
  {
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV6UC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv6uc", 0, 0);
}


uint32
   arad_pp_flp_key_const_ipv6mc(
     int unit
   )
{
  uint32
    res, vrf_ce_inst = 0, rif_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);
    

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV6MC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_MC){
      /* Key A */
      flp_key_construction_tbl.instruction_0_16b = vrf_ce_inst | 0xf000; 
      flp_key_construction_tbl.instruction_1_16b = rif_ce_inst | 0xf000;
      flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_127_96;

      /* Key B */
      flp_key_construction_tbl.instruction_2_16b = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_95_80;
      flp_key_construction_tbl.instruction_4_32b = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_79_48;

      /* Key C */
      flp_key_construction_tbl.instruction_5_32b = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_31_0;

      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0xb;  /* hex 0'b001011 */
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x14; /* hex 0'b010100 */
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x20; /* hex 0'b100000 */
  }
  else
#endif
  {
      flp_key_construction_tbl.instruction_0_16b        = rif_ce_inst;
      flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_119_112
      flp_key_construction_tbl.instruction_2_16b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_111_96
      flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_95_64
      flp_key_construction_tbl.instruction_4_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_63_32
      flp_key_construction_tbl.instruction_5_32b        = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_31_0
      flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x3F /*hex 6'b111111*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV6MC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV6MC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_MC){
      /* Key B */
      flp_key_construction_tbl.instruction_0_16b = ARAD_PP_CE_SIP_IPV6_FWD_HEADER_47_32;

      /* Key C */
      flp_key_construction_tbl.instruction_1_16b = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_127_112;
      flp_key_construction_tbl.instruction_2_16b = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_111_96;
      flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_95_64;
      flp_key_construction_tbl.instruction_4_32b = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_63_32;
      flp_key_construction_tbl.instruction_5_32b = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_31_0;

      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0;  
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x1;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3e; /* hex 6'b111110 */
  }
  else
#endif
  {
      flp_key_construction_tbl.instruction_0_16b     = rif_ce_inst;
      flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_119_112
      flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_111_96
      flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_95_64
      flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_63_32
      flp_key_construction_tbl.instruction_5_32b     = ARAD_PP_CE_DIP_IPV6_FWD_HEADER_31_0
      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3F /*hex 6'b111111*/;
  }
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV6MC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv6mc", 0, 0);
}

uint32
   arad_pp_flp_key_const_pwe_gre(
     int    unit,     
     uint8  in_rif,
     uint8  in_exp
   )
{
  uint32
    res, mpls_exp_ce_inst = 0, rif_ce_inst = 0;
  uint32
    prog;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_MPLS_EXP,0, &mpls_exp_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);
    
  res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_VPLSOGRE, &prog);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b     = (in_rif)  ? rif_ce_inst:ARAD_PP_FLP_16B_INST_12_ZEROS;
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_8_ZEROS;
  flp_key_construction_tbl.instruction_2_16b     =             mpls_exp_ce_inst;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x7 /*hex 6'b000111*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_CE_MPLS_FWD_HEADER_15_0; /* 0 is the MSB of the packet */
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_CE_MPLS_FWD_HEADER_19_16;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x3 /*6'b000011*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_pwe_gre", 0, 0);
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_key_const_lsr(
     int unit,
     uint8  in_port,
     uint8  in_rif,
     uint8  in_exp
   )
{
  uint32
      pp_port_ce_inst = 0;
  uint32
    res, mpls_exp_ce_inst = 0, rif_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_MPLS_EXP,0, &mpls_exp_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);
    
  if (SOC_IS_JERICHO(unit)) {
      arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT, 0, &pp_port_ce_inst);
  }

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_LSR, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b     =  ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_3_BITS; /* MPLS name space configures in the class_id*/
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x1 /*hex 6'b000001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_LSR, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_LSR+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_CE_MPLS_FWD_HEADER_15_0; /* 0 is the MSB of the packet */
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_CE_MPLS_FWD_HEADER_19_16;
  flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_CE_MPLS_FWD_HEADER_BOS_BIT_23;

  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x7 /*6'b000111*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_LSR+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  /* have to put it here due to that arad_pp_flp_key_const_lsr is called twice, 
     * one in arad_pp_flp_lookups_initial_programs and the other in arad_pp_lem_ilm_key_build_set,
     * which will override any changes done in arad_pp_flp_init.
     */
  res = arad_pp_flp_lpm_custom_lookup_enable(unit, PROG_FLP_LSR);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_lsr", 0, 0);
}
#endif

uint32
   arad_pp_flp_lsr_stat_init(int unit)
{
    uint32 res=0;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    /*Jericho need to support 1st lookup for LSR counting*/
    res = arad_pp_flp_instruction_rsrc_set(unit, PROG_FLP_LSR);
    SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);
    arad_pp_flp_dbal_mpls_lsr_stat_table_init(unit);
      
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_lsr", 0, 0);
}

/* key const for all FCF NPV programs */

uint32
   arad_pp_flp_key_const_fcoe_fcf_npv(
     int unit,
     int32  progs[2],
     int is_vsan_from_vsi
   )
{
    uint32
        res;
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
        flp_key_construction_tbl;
    uint32
        vrf_bits;
    uint32
        vsi_vft_instuction,
        key_gen_var;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (is_vsan_from_vsi == 0) {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_13_BITS;
    } else {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_VSI(12);
    }

    vrf_bits = soc_sand_log2_round_up(((SOC_DPP_CONFIG(unit))->l3.max_nof_vrfs));

    /* NPV no-vft */
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[0], &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
    flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_0_PROGRAM_KEY_GEN_VAR(vrf_bits); /* ones for VRF-id: 11111 */
    flp_key_construction_tbl.instruction_1_16b     = vsi_vft_instuction;    
    flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
    flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_S_ID;
    flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0  /*6'b000000*/;
    if (is_vsan_from_vsi == 0) {
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x12 /*6'b010010 - fwd*/;
        flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3  /*6'b000011*/;/* LPM-remote: ones,zeros,DOMAIN*/
    } else {
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x16 /*6'b010110 - fwd*/;
        flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x7  /*6'b000111*/;/* LPM-remote: ones,zeros,DOMAIN*/
    }        

    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[0], &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[0]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
    flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_ARAD_FC_S_ID_8_MSB;/* FC_S_ID[23:16]*/
    flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(32-ARAD_PP_FLP_VFT_NOF_BITS-8);/* make "IP address" 32 bit*/
    flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3 /*6'b000011*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[0]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);

    /* 2 NPV - with VFT*/
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[1], &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
    flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_0_PROGRAM_KEY_GEN_VAR(vrf_bits); /* ones for VRF-id: 11111 */    

    if (is_vsan_from_vsi == 0) {
      flp_key_construction_tbl.instruction_1_16b = ARAD_PP_FLP_16B_INST_ARAD_FC_WITH_VFT_VFT_ID;/* FC_VFT_ID */
    } else {
      flp_key_construction_tbl.instruction_1_16b = ARAD_PP_FLP_16B_INST_P6_VSI(12);}

    flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
    flp_key_construction_tbl.instruction_5_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_S_ID;
    flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0  /*6'b000000*/;
    if (is_vsan_from_vsi == 0) {
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x22 /*6'b100010 - fwd*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3  /*6'b000011 - fwd-LPM: ones,VFT,DOMAIN*/;
    } else {
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x26 /*6'b100110 - fwd*/;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x7  /*6'b000111 - fwd-LPM: ones,0,DOMAIN*/;
    }    
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[1], &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);
    
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[1]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
    flp_key_construction_tbl.instruction_0_16b     = ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_S_ID_8_MSB; /* FC_S_ID-msb*/
    flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(32-ARAD_PP_FLP_VFT_NOF_BITS-8);/* make "IP address" 32 bit*/
    flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3 /*6'b000011 */;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[1]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);
    
    /* key gen var used to set VRF used for FCF forwarding */
    if (!SOC_IS_JERICHO(unit)) {
        key_gen_var = (((SOC_DPP_CONFIG(unit))->pp.fcoe_reserved_vrf) + 1);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, WRITE_IHP_FLP_PROGRAM_KEY_GEN_VARm(unit,MEM_BLOCK_ANY,progs[0],&key_gen_var));
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, WRITE_IHP_FLP_PROGRAM_KEY_GEN_VARm(unit,MEM_BLOCK_ANY,progs[1],&key_gen_var));
    }

    if (SOC_IS_JERICHO(unit)) {
        /* in Jericho, we are using DBAL for allocating resources to the LPM table,
         so we deallocate the resources that was manually allocated here (for arad) and not needed */

        /* 3 REMOTE no VFT */
      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[0], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_0_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[0], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[0]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_0_16b = 0;
      flp_key_construction_tbl.instruction_1_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[0]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
            
      /* 4 REMOTE with VFT/VSI */
      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[1], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_0_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[1], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[1]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_0_16b = 0;
      flp_key_construction_tbl.instruction_1_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[1]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

      /* update SW about instruction usage */
      res = arad_pp_flp_instruction_rsrc_set(unit, progs[0]);
      SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
      res = arad_pp_flp_instruction_rsrc_set(unit, progs[1]);
      SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_fcoe_fcf_npv", 0, 0);
}

/* key const for all FCF programs */
uint32
   arad_pp_flp_key_const_fcoe_fcf(
     int unit,
     int32  progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS],
     int is_vsan_from_vsi
   )
{
  uint32
    res;
  uint32
    vrf_bits;
  uint32
      vsi_vft_instuction,
      key_gen_var;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;  

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (is_vsan_from_vsi == 0) {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_13_BITS;
    } else {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_VSI(12);
    }

  /* local no-vft (use default vft or vsi according to vsi_vft_instuction) */
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[0], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b     = vsi_vft_instuction;
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
  flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_S_ID;
  flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_D_ID;
  if (is_vsan_from_vsi == 0) {
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x19 /*6'b011001 - Zoning*/;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x11 /*hex 6'b010001 - fwd*/;
  } else {/* adding one more zero for the reserve bit in the VFT*/
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x1B /*6'b011011 - Zoning*/;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x13 /*hex 6'b010011 - fwd*/;
  }  
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[0], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[0]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[0]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);

  /* 2 LOCAL - with VFT/VSI according to vsi_vft_instuction */
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[1], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  if (is_vsan_from_vsi == 0) {
      flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_ARAD_FC_WITH_VFT_VFT_ID;
  } else {
      flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_P6_VSI(12);}
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
  flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_S_ID;
  flp_key_construction_tbl.instruction_5_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_D_ID;
  if (is_vsan_from_vsi == 0) {
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x31 /*6'b110001 - Zoning*/;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x21 /*hex 6'b100001 - fwd*/;
  } else {/* adding one more zero for the reserve bit in the VFT*/
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x33 /*6'b110011 - Zoning*/;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x23 /*hex 6'b100011 - fwd*/;
  }
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[1], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[1]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[1]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);

    /* 3 REMOTE no- VFT use port default VFT or VSI*/
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[2], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  vrf_bits = soc_sand_log2_round_up(((SOC_DPP_CONFIG(unit))->l3.max_nof_vrfs));
  
  flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_0_PROGRAM_KEY_GEN_VAR(vrf_bits); /* ones for VRF-id: 11111 */
  flp_key_construction_tbl.instruction_1_16b     = vsi_vft_instuction;
  flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
  flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_S_ID;/* FC_S_ID */
  flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_D_ID;/* FC_D_ID*/
  if (is_vsan_from_vsi == 0) {
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x1A /*6'b011010 - Zoning:S_ID, D_ID*/;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x2 /* 6'b000010 - fwd:DOMAIN*/;
        flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3 /* 6'b000011*/; /* LPM-remote: ones,zeros,DOMAIN*/
  } else {/* adding one more zero for the reserve bit in the VFT*/
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x1E /*6'b011110 - Zoning:S_ID, D_ID*/;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x6 /* 6'b000110 - fwd:DOMAIN*/;
        flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x7 /* 6'b000111*/; /* LPM-remote: ones,zeros,DOMAIN*/
  }    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[2], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);
  
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[2]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_ARAD_FC_D_ID_8_MSB;/* FC_D_ID[23:16]*/
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(32-ARAD_PP_FLP_VFT_NOF_BITS-8);/* make "IP address" 32 bit*/
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x1 /*6'b000001*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3 /*6'b000011*/; /* push more zeros*/
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[2]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);
  
  /* 4 REMOTE with VFT/VSI */
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[3], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b     = ARAD_PP_FLP_16B_INST_0_PROGRAM_KEY_GEN_VAR(vrf_bits); /* ones for VRF-id: 11111 */

  if (is_vsan_from_vsi == 0) {
      flp_key_construction_tbl.instruction_1_16b = ARAD_PP_FLP_16B_INST_ARAD_FC_WITH_VFT_VFT_ID; /* FC_VFT_ID */
  } else {
      flp_key_construction_tbl.instruction_1_16b = ARAD_PP_FLP_16B_INST_P6_VSI(12);}

  flp_key_construction_tbl.instruction_2_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
  flp_key_construction_tbl.instruction_4_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_S_ID; /* FC_S_ID */
  flp_key_construction_tbl.instruction_5_32b     = ARAD_PP_FLP_32B_INST_ARAD_FC_WITH_VFT_D_ID; /* FC_D_ID*/
  if (is_vsan_from_vsi == 0) {
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x32 /*6'b110010 - Zoning: VFT, S_ID, D_ID */;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x2 /* 6'b000010 - fwd : VFT + FC_ID_MSB*/;
        flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3; /*6'b000011 - fwd-LPM: ones,VFT,DOMAIN */
  } else {/* adding one more zero for the reserve bit in the VFT*/
        flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x36 /*6'b110110 - Zoning: VFT, S_ID, D_ID */;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x6 /* 6'b000110 - fwd : VFT + FC_ID_MSB*/;
        flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x7; /*6'b000111 - fwd-LPM: ones,VFT,DOMAIN */
  }    

  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[3], &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[3]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  flp_key_construction_tbl.instruction_0_16b     = ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_D_ID_8_MSB; /* FC_D_ID-msb*/
  flp_key_construction_tbl.instruction_1_16b     = ARAD_PP_FLP_16B_INST_N_ZEROS(32-ARAD_PP_FLP_VFT_NOF_BITS-8);/* make "IP address" 32 bit*/
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x1 /*6'b000001*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x3 /*6'b000011*/; /* push more zeros*/
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[3]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);

  if (!SOC_IS_JERICHO(unit)) { /* this is done for ARAD+ only, not needed in Jericho*/
      /* key gen var used to set VRF used for FCF forwarding */
      key_gen_var = (SOC_DPP_CONFIG(unit))->pp.fcoe_reserved_vrf;
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, WRITE_IHP_FLP_PROGRAM_KEY_GEN_VARm(unit,MEM_BLOCK_ANY,progs[0],&key_gen_var));
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, WRITE_IHP_FLP_PROGRAM_KEY_GEN_VARm(unit,MEM_BLOCK_ANY,progs[1],&key_gen_var));
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, WRITE_IHP_FLP_PROGRAM_KEY_GEN_VARm(unit,MEM_BLOCK_ANY,progs[2],&key_gen_var));
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, WRITE_IHP_FLP_PROGRAM_KEY_GEN_VARm(unit,MEM_BLOCK_ANY,progs[3],&key_gen_var));
  }

  if (SOC_IS_JERICHO(unit)) {      
      /* we are using DBAL for allocating resources to the LPM table,
         so we deallocate the resources that was manually allocated here (in arad) and not needed */

      /* 3 REMOTE no VFT */
      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[2], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_0_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[2], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[2]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_1_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[2]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
            
      /* 4 REMOTE with VFT/VSI */
      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[3], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_0_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[3], &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, progs[3]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
      flp_key_construction_tbl.instruction_1_16b = 0;
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, progs[3]+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

      /* update SW about instruction usage */
      res = arad_pp_flp_instruction_rsrc_set(unit, progs[2]);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      res = arad_pp_flp_instruction_rsrc_set(unit, progs[3]);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      res = arad_pp_flp_dbal_fcoe_program_tables_init(unit, is_vsan_from_vsi, progs[2], progs[3]);
      SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
#endif
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_fcoe_fcf", 0, 0);
}


uint32
   arad_pp_flp_key_const_TRILL_uc(
     int unit
   )
{
  uint32
    res, src_port_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_SRC_SYST_PORT,0, &src_port_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_TRILL_UC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b = ARAD_PP_CE_TRILL_EGRESS_NICK;
  flp_key_construction_tbl.instruction_1_16b = src_port_ce_inst;
  flp_key_construction_tbl.instruction_2_16b = ARAD_PP_CE_TRILL_SA_16MSB;
  flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_TRILL_SA_32LSB;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x1 /*hex 6'b000001*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0xe /*6'b001110*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_TRILL_UC, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_TRILL_UC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_TRILL_UC+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  res = arad_pp_flp_instruction_rsrc_set(unit, PROG_FLP_TRILL_UC);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_TRILL", 0, 0);
}

uint32
   arad_pp_flp_key_const_TRILL_mc(
     int unit)
{
  uint32
    res, fid_ce_inst = 0, esadi_ce_inst, src_port_ce_inst = 0,
    prog /* one tag or two tags */;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;
  uint8 is_two_tags;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_ESADI,0, &esadi_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_SRC_SYST_PORT,0, &src_port_ce_inst);

  /* The only different between the single/two tags prorgams is whether param3 is all zeros, or inner vid */
  for (is_two_tags = 0; is_two_tags <= 1; ++is_two_tags) {
      prog = is_two_tags ?
          PROG_FLP_TRILL_MC_TWO_TAGS :
          PROG_FLP_TRILL_MC_ONE_TAG;

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_TRILL_MC && 
         (prog == PROG_FLP_TRILL_MC_ONE_TAG))
      {
          flp_key_construction_tbl.instruction_0_16b = esadi_ce_inst;
          flp_key_construction_tbl.instruction_1_16b = SOC_DPP_CONFIG(unit)->trill.mc_prune_mode ? fid_ce_inst : ARAD_PP_FLP_16B_INST_15_ZEROS;
          flp_key_construction_tbl.instruction_2_16b = ARAD_PP_CE_TRILL_DIST_TREE_NICK_16;

          flp_key_construction_tbl.key_a_inst_0_to_5_valid  = 0x7; /*6'b000111*/
          flp_key_construction_tbl.key_b_inst_0_to_5_valid  = 0x0; /*6'b000000*/
          flp_key_construction_tbl.key_c_inst_0_to_5_valid  = 0x0; /*6'b000000*/
      }
      else
#endif
      {
          flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0;
          flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0xb;
          flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0;
          /* Take Port+SA from native header */
          flp_key_construction_tbl.instruction_0_16b = src_port_ce_inst;
          flp_key_construction_tbl.instruction_1_16b = ARAD_PP_CE_TRILL_SA_16MSB;
          flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_TRILL_SA_32LSB;
      }
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);

      res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 47, exit);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_TRILL_MC && (prog == PROG_FLP_TRILL_MC_ONE_TAG))
      {
          flp_key_construction_tbl.key_a_inst_0_to_5_valid  = 0x0; /*6'b000000*/
          flp_key_construction_tbl.key_b_inst_0_to_5_valid  = 0xb; /*6'b001011*/
          flp_key_construction_tbl.key_c_inst_0_to_5_valid  = 0x0; /*6'b000000*/
		  flp_key_construction_tbl.instruction_0_16b = src_port_ce_inst;
          flp_key_construction_tbl.instruction_1_16b = ARAD_PP_CE_TRILL_SA_16MSB;
          flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_TRILL_SA_32LSB;
      }
      else
#endif
      {
          if (SOC_IS_DPP_TRILL_FGL(unit) && SOC_DPP_CONFIG(unit)->trill.mc_prune_mode) {
              /* FGL key without prunning - <Inner-vid,Outer-vid,ESADI,DIST-TREE> */
              if (is_two_tags) {
                flp_key_construction_tbl.instruction_0_16b = ARAD_PP_CE_TRILL_NATIVE_INNER_TAG_CE16;
              } else {
                flp_key_construction_tbl.instruction_0_16b = ARAD_PP_FLP_16B_INST_N_ZEROS(12);
              }
              flp_key_construction_tbl.instruction_1_16b = ARAD_PP_CE_TRILL_NATIVE_OUTER_TAG_CE16;
              flp_key_construction_tbl.instruction_2_16b = esadi_ce_inst;
              flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_TRILL_DIST_TREE_NICK_32;
              flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0xf /*6'b001111*/;
              flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
              flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b001110*/;
          } else {
            /* We behave in the same manner in VL mode (VSI is a part of the key) or if we don't vlan prune */
            /* VL key: <FID or 0x0,ESADI,DIST-TREE> */
              flp_key_construction_tbl.instruction_0_16b = SOC_DPP_CONFIG(unit)->trill.mc_prune_mode ? fid_ce_inst :ARAD_PP_FLP_16B_INST_15_ZEROS;
              flp_key_construction_tbl.instruction_1_16b = esadi_ce_inst;
              flp_key_construction_tbl.instruction_2_16b = ARAD_PP_CE_TRILL_DIST_TREE_NICK_16;
              flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x7 /*6'b000111*/;
              flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
              flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
          }
      }
      
      res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 51, exit);
      res = arad_pp_flp_instruction_rsrc_set(unit, prog);
      SOC_SAND_CHECK_FUNC_RESULT(res, 51, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_TRILL_mc", 0, 0);
}


uint32
   arad_pp_flp_key_const_pon_vmac_upstream(
     int unit,
     uint32 prog_id
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0xB /*6'b001011*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_pon_vmac_upstream", 0, 0);
}

uint32
   arad_pp_flp_key_const_pon_vmac_downstream(
     int unit,
     uint32 prog_id
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  /* DHCP */
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_DA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_DA_FWD_HEADER_32_LSB;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0xB /*6'b001011*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_pon_vmac_downstream", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv4uc_l3vpn(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_PUBLIC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  /* WARNING: This lookup will not be available when working with SLB. */    
  flp_lookups_tbl.lem_1st_lkp_valid     = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
  }
  else
#endif
  {
      flp_lookups_tbl.lpm_1st_lkp_valid	  = 1;
      flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
      flp_lookups_tbl.lpm_1st_lkp_and_value = 3;
      flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
      flp_lookups_tbl.lpm_2nd_lkp_valid	  = 1;
      flp_lookups_tbl.lpm_2nd_lkp_key_select = 1;
      flp_lookups_tbl.lpm_2nd_lkp_and_value = 3;
      flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;
  }

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      
  }  
#endif

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_PUBLIC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv4uc_with_l3vpn", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv6uc_with_rpf_2pass(
     int unit,
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
/*   res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV6UC_WITH_RPF_2PASS, &flp_lookups_tbl); */
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 242, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  flp_lookups_tbl.elk_lkp_valid = 0x1;
  flp_lookups_tbl.elk_wait_for_reply = 0x1;
  flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF_2PASS;

  flp_lookups_tbl.tcam_lkp_db_profile    = ARAD_TCAM_ACCESS_PROFILE_INVALID;
  flp_lookups_tbl.tcam_lkp_key_select    = ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL; /* Key-C */
  flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;
  flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

  /* Key A hex 6'b011001: 16b + 32b + 32b = 10B */
  flp_lookups_tbl.elk_key_a_valid_bytes = 10; 
  /* Key B hex 6'b100110: 16b + 16b + 32b = 8B */
  flp_lookups_tbl.elk_key_b_valid_bytes = 8;  
  flp_lookups_tbl.elk_key_c_valid_bytes = 0; 

  res = first_lem_lkup_sa_fid_search_set(unit,&flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
#endif

/*  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV6UC_WITH_RPF_2PASS, &flp_lookups_tbl); */
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 243, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv6uc_with_rpf_2pass", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv4uc_rpf(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_RPF, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
  flp_lookups_tbl.lpm_1st_lkp_valid     = 1;
  flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
  flp_lookups_tbl.lpm_1st_lkp_and_value = 3; /* b`40011 */
  flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_lookups_tbl.elk_lkp_valid = 0x1;
      flp_lookups_tbl.elk_wait_for_reply = 0x1;
      flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_RPF;
      flp_lookups_tbl.elk_key_a_valid_bytes = 0x6;
      flp_lookups_tbl.elk_key_b_valid_bytes = 0x4;
      flp_lookups_tbl.elk_key_c_valid_bytes = 0x0;
      if(ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
          flp_lookups_tbl.elk_key_b_valid_bytes = 0x6;
          flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_IP_WITH_RPF;

		  res = first_lem_lkup_sa_fid_search_set(unit,&flp_lookups_tbl);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
      }
  }
  else
#endif
  {
      flp_lookups_tbl.lpm_2nd_lkp_valid     = 1;
      flp_lookups_tbl.lpm_2nd_lkp_key_select = 1;
      flp_lookups_tbl.lpm_2nd_lkp_and_value = 3; /* b`40011 */
      flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;
  }

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_RPF, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv4uc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv4uc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV4UC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  /* WARNING: This lookup will not be available when working with SLB. */    
  flp_lookups_tbl.lem_1st_lkp_valid     = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
  }
  else
#endif
  {
      flp_lookups_tbl.lpm_1st_lkp_valid     = 1;
      flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
      flp_lookups_tbl.lpm_1st_lkp_and_value = 3;
      flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
      flp_lookups_tbl.lpm_2nd_lkp_valid     = 1;
      flp_lookups_tbl.lpm_2nd_lkp_key_select = 1;
      flp_lookups_tbl.lpm_2nd_lkp_and_value = 3;
      flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;
  }

  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_lookups_tbl.elk_lkp_valid = 0x1;
      flp_lookups_tbl.elk_wait_for_reply = 0x1;
      flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_UC;
      flp_lookups_tbl.elk_key_a_valid_bytes = 0x6;
      flp_lookups_tbl.elk_key_b_valid_bytes = 0x4;
      flp_lookups_tbl.elk_key_c_valid_bytes = 0x0;
	  if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
		  flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_IP;
          flp_lookups_tbl.elk_key_b_valid_bytes = 0x6;
		  res = first_lem_lkup_sa_fid_search_set(unit, &flp_lookups_tbl);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
	  }

      if (SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1) {
          flp_lookups_tbl.elk_key_a_valid_bytes = 0x0;
          flp_lookups_tbl.elk_key_b_valid_bytes = 0x0;
          flp_lookups_tbl.elk_key_c_valid_bytes = 0xa;
          if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
              flp_lookups_tbl.elk_key_c_valid_bytes = 0xc;
		  }
          }
  }  
#endif

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV4UC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv4uc", 0, 0);
}


uint32
   arad_pp_flp_lookups_bfd_single_hop(
     int unit,
	 int prog
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  /* WARNING: This lookup will not be available when working with SLB. */    
  flp_lookups_tbl.lem_1st_lkp_valid     = 0;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = 0;
  flp_lookups_tbl.lpm_1st_lkp_valid     = 0;
  flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
  flp_lookups_tbl.lpm_1st_lkp_and_value = 0;
  flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lpm_2nd_lkp_valid     = 0;
  flp_lookups_tbl.lpm_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value =0x0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_SINGLE_HOP,&flp_lookups_tbl.lem_2nd_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  flp_lookups_tbl.lpm_2nd_lkp_and_value = 0;
  flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;
  flp_lookups_tbl.learn_key_select      = 1;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_bfd_single_hop", 0, 0);
}


uint32
   arad_pp_flp_lookups_tcam_profile_set(
     int unit,
     uint32 tcam_access_profile_ndx,
     uint32 tcam_access_profile_id,
     uint32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  if (tcam_access_profile_ndx == 0) {
      flp_lookups_tbl.tcam_lkp_db_profile    = tcam_access_profile_id;
      flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;
  } else {
      flp_lookups_tbl.tcam_lkp_db_profile_1    = tcam_access_profile_id;
      flp_lookups_tbl.tcam_lkp_key_select_1    = 1; 
      flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;
  }

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_tcam_profile_set", 0, 0);
}

uint32
   arad_pp_flp_process_ipv6uc_with_rpf_2pass(
     int unit,
     int32 prog_id
   )
{
  uint32
      res;
  uint32
      fld_val,
      mem_val=0;
  uint32
      tmp;
  soc_reg_above_64_val_t
      reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
      flp_process_tbl;
  SOC_SAND_OUT ARAD_SOC_REG_FIELD   strength_fld_fwd,
                                    strength_fld_snp;
  SOC_SAND_OUT SOC_PPC_TRAP_CODE_INTERNAL   trap_code_internal;
  

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Used the pre-allocated User_Defined_Trap code and map it to the HW Code */
    res = arad_pp_trap_mgmt_trap_code_to_internal(unit, SOC_PPC_TRAP_CODE_IPV6_UC_RPF_2PASS, &trap_code_internal, &strength_fld_fwd, &strength_fld_snp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 240, exit);  
 
    /* RPF check expect ELK_LOOKUP_B results and not in ELK_LOOKUP_A resilts which is FWD results
    * Since using ELK opcode of RPF the FWD is always failed.
    */
/*    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV6UC_WITH_RPF_2PASS, &flp_process_tbl); */
    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 241, exit);

    flp_process_tbl.include_tcam_in_result_a      = 0x0;
    flp_process_tbl.result_a_format               = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
    flp_process_tbl.result_b_format               = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
    flp_process_tbl.procedure_ethernet_default    = 0x0; /* disabled */
    flp_process_tbl.enable_hair_pin_filter        = 0x0; /* not relevant since not expecting result in Result_A */
    flp_process_tbl.enable_rpf_check              = 0x1;
    flp_process_tbl.sa_lkp_process_enable         = 0x0;
    flp_process_tbl.apply_fwd_result_a            = 0x0; /* Apply only result A (FWD) not found */

    /* take VRF default destination */
    flp_process_tbl.not_found_trap_strength       = 0x6;
    flp_process_tbl.not_found_trap_code           = trap_code_internal;
    /* 0x0 - Use VRF Default Unicast
       0x1 - Use VRF Default Multicast
       Else: Use NotFoundTrapCode from program */
    flp_process_tbl.select_default_result_a     = 0x2;  /* Use NotFoundTrapCode from program */
    flp_process_tbl.select_default_result_b     = 0x0;

    flp_process_tbl.elk_result_format = 1;
    flp_process_tbl.include_elk_fwd_in_result_a = 0;     /* Always signal FWD not Fount -> results with RCY trap */
    flp_process_tbl.include_elk_ext_in_result_a = 0;
    flp_process_tbl.include_elk_fwd_in_result_b = 0;
    flp_process_tbl.include_elk_ext_in_result_b = 1;

    flp_process_tbl.fwd_processing_profile        = ARAD_PP_FLP_PROGRAM_FWD_PROCESS_PROFILE_REPLACE_FWD_CODE;

	res = process_tbl_learn_enable_set(unit,&flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);

/*    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV6UC_WITH_RPF_2PASS, &flp_process_tbl); */
    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 244, exit);

  /* 0: Apply Ethernet traps on forwarding-header
     1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
     2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
     3: Apply MPLS traps on forwarding-header
     4: Apply FC traps on forwarding-header
     Else: Don't apply any protocol traps */
    tmp = 2; 
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 245, exit);
/*    SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV6UC_WITH_RPF_2PASS,&tmp,0,3); */
    SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
    res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 246, exit);

    fld_val = 0x6; /* bit 6 is lookup-found-1 */
    soc_mem_field_set(unit, IHB_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val);
    fld_val = 0x5; /* bits 109:56 is lookup-result-1 */
    soc_mem_field_set(unit, IHB_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val);
    fld_val = 0x2; /*FEC format in lookup-result-1*/
    soc_mem_field_set(unit, IHB_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_FORMATf, &fld_val);
    res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, prog_id, &mem_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101, exit);


exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv6uc_with_rpf_2pass", 0, 0);
}

uint32
   arad_pp_flp_process_ipv4uc_rpf(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv4uc_with_rpf");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV4UC_RPF, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;

  flp_process_tbl.result_a_format                = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.result_b_format                = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 1;
  /* take VRF default destination */
  flp_process_tbl.select_default_result_a = 0;
  flp_process_tbl.select_default_result_b = 0;
  flp_process_tbl.sa_lkp_process_enable = 0;
  flp_process_tbl.not_found_trap_strength = 3;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_process_tbl.elk_result_format = 1;
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
      flp_process_tbl.include_elk_ext_in_result_a = 0;
      flp_process_tbl.include_elk_fwd_in_result_b = 0;
      flp_process_tbl.include_elk_ext_in_result_b = 1;
      if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
          res = process_tbl_learn_enable_set(unit, &flp_process_tbl);
          SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
      }else{
          if (SOC_IS_JERICHO(unit)) {/* in Jericho with KBP the LEM has higher priority */
              flp_process_tbl.include_lem_2nd_in_result_a    = 2;
              flp_process_tbl.include_lem_1st_in_result_b    = 2;
          }
      }
  }
  else 
#endif
  {
      flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
      flp_process_tbl.include_lpm_1st_in_result_b    = 1;
      flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(JER_KAPS_ENABLE(unit)) {
          flp_process_tbl.lpm_2nd_lkp_enable_default = 1;
      }
#endif
  }


  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV4UC_RPF, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 1; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV4UC_RPF,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4uc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv6uc(
     int unit,
     uint32 tcam_access_profile_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV6UC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  {
      flp_lookups_tbl.tcam_lkp_db_profile    = tcam_access_profile_id;
      flp_lookups_tbl.tcam_lkp_key_select    = 0;
      flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;
      flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;
      flp_lookups_tbl.learn_key_select      = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_EXTENDED){
          flp_lookups_tbl.elk_lkp_valid = 0x1;
          flp_lookups_tbl.elk_wait_for_reply = 0x1;
          flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC;

          if(ARAD_KBP_ENABLE_IPV6_EXTENDED) {
              flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_IPV6;
		  res = first_lem_lkup_sa_fid_search_set(unit, &flp_lookups_tbl);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);    
		  }
          
          flp_lookups_tbl.elk_key_a_valid_bytes = 10; 
          flp_lookups_tbl.elk_key_b_valid_bytes = 8;  
          if(ARAD_KBP_ENABLE_IPV6_EXTENDED) {
              flp_lookups_tbl.elk_key_a_valid_bytes = 2; /* No SIP */
              flp_lookups_tbl.elk_key_b_valid_bytes = 0;  
          }
          flp_lookups_tbl.elk_key_c_valid_bytes = 16; 
      }
#endif
  }

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV6UC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv6uc", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv6mc(
     int unit,
     uint32 tcam_access_profile_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV6MC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  {
      flp_lookups_tbl.tcam_lkp_db_profile    = tcam_access_profile_id;
      flp_lookups_tbl.tcam_lkp_key_select    = ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL; /* Key-C */
      flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;
      flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;
      flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

    #if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_IPV6_MC){
          flp_lookups_tbl.elk_lkp_valid = 0x1;
          flp_lookups_tbl.elk_wait_for_reply = 0x1;
          flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_MC_RPF;

          flp_lookups_tbl.elk_key_a_valid_bytes = 8;  /* take 8 bytes from key A */
          flp_lookups_tbl.elk_key_b_valid_bytes = 8;  /* take 6 bytes from key B */
          flp_lookups_tbl.elk_key_c_valid_bytes = 20; /* take 21 bytes from key C */
      }
    #endif
  }

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV6MC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv6mc", 0, 0);
}

uint32
   arad_pp_flp_lookups_oam(
     int unit,
     uint32 tcam_access_profile_id_0,
     uint32 tcam_access_profile_id_1,
     uint32 flp_key_program
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_key_program, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = tcam_access_profile_id_0;
  flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = tcam_access_profile_id_1;
  flp_lookups_tbl.enable_tcam_identification_oam = 1;
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, flp_key_program, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_oam", 0, 0);
}

uint32
   arad_pp_flp_lookups_fcf_npv(
     int    unit,
     int32  progs[2]
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

   /* FlpLookups_fcf */ 
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[0], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  /* 2nd */  
  flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1; /* B: forwarding */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_FC_N_PORT_KEY_OR_MASK,&flp_lookups_tbl.lem_2nd_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

  flp_lookups_tbl.learn_key_select      = 0;

  /* general */
  flp_lookups_tbl.elk_lkp_valid      = 0;
  flp_lookups_tbl.lpm_1st_lkp_valid  = 0;
  if (!SOC_IS_JERICHO(unit)) {
      /* in Jericho, done by DBAL */
      flp_lookups_tbl.lpm_2nd_lkp_valid  = 1;
      flp_lookups_tbl.lpm_2nd_lkp_key_select = 2; /* key C */
  }
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[0], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  /* FlpLookups_fcf with vft */ 
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[1], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_fcf_npv", 0, 0);
}


uint32
   arad_pp_flp_lookups_fcf(
     int    unit,
     int32  progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS],
     int    is_zoning_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* FlpLookups_fcf */ 
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[0], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  /* 1st */  
  flp_lookups_tbl.lem_1st_lkp_valid      = is_zoning_enabled;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0; /* A: 1 zoning */
  flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;

  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_FC_ZONING_KEY_OR_MASK,&flp_lookups_tbl.lem_1st_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  /* 2nd */  
  flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1; /* B: forwarding */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_FC_KEY_OR_MASK,&flp_lookups_tbl.lem_2nd_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

  flp_lookups_tbl.learn_key_select      = 0;

  /* general */
  flp_lookups_tbl.elk_lkp_valid      = 0;
  flp_lookups_tbl.lpm_1st_lkp_valid  = 0;
  flp_lookups_tbl.lpm_2nd_lkp_valid  = 0;
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[0], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  /* FlpLookups_fcf with vft */ 
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[1], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  /* FlpLookups_fcf_remote */ 

  /* remote prefix + lookup in LPM */
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_FC_REMOTE_KEY_OR_MASK,&flp_lookups_tbl.lem_2nd_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
  if (!SOC_IS_JERICHO(unit)) { /* in jericho the LPM enable is done by DBAL */
      flp_lookups_tbl.lpm_2nd_lkp_valid  = 1;
      flp_lookups_tbl.lpm_2nd_lkp_key_select = 2; /* key C */
  }
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[2], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);
  /* FlpLookups_fcf with vft remote */ 
  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[3], &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_fcf", 0, 0);
}



#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_lookups_lsr(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_LSR, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_LSR_KEY_OR_MASK;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;
  flp_lookups_tbl.elk_lkp_valid = 0x1;
  flp_lookups_tbl.elk_wait_for_reply = 0x1;      

  flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_LSR;
  flp_lookups_tbl.elk_key_a_valid_bytes = 3;

  res = first_lem_lkup_sa_fid_search_set(unit, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);

  flp_lookups_tbl.elk_key_b_valid_bytes = 0;
  flp_lookups_tbl.elk_key_c_valid_bytes = 0;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_LSR, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_lsr", 0, 0);
}
#endif


uint32
   arad_pp_flp_lookups_pwe_gre(
     int unit,
     int prog
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_LSR_KEY_OR_MASK;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_pwe_gre", 0, 0);
}

uint32
   arad_pp_flp_lookups_TRILL_uc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_TRILL_UC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_TRILL_ADJ_KEY_OR_MASK,&flp_lookups_tbl.lem_1st_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_TRILL_KEY_OR_MASK;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_TRILL_UC){
      flp_lookups_tbl.elk_lkp_valid = 0x1;
      flp_lookups_tbl.elk_wait_for_reply = 0x1;
      flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_UC;

      flp_lookups_tbl.elk_key_a_valid_bytes = 2; 
      flp_lookups_tbl.elk_key_b_valid_bytes = 0;  
      flp_lookups_tbl.elk_key_c_valid_bytes = 0; 
  }
#endif

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_TRILL_UC, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_TRILL", 0, 0);
}


uint32
    arad_pp_flp_lookups_TRILL_mc_update(int unit,
     uint32 is_ingress_learn)
{
    uint32 res, prog /* one tag or two tags */;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;
  uint8 is_two_tags;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (is_two_tags = 0; is_two_tags <= 1; ++is_two_tags) {
      prog = is_two_tags ?PROG_FLP_TRILL_MC_TWO_TAGS :PROG_FLP_TRILL_MC_ONE_TAG;
      res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog, &flp_lookups_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);      
      flp_lookups_tbl.learn_key_select = (is_ingress_learn ? 0x1 : 0x0); /* For ingress learning, learn the key. For egress, take from forwarding header */
      
      res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog, &flp_lookups_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_TRILL_mc", 0, 0);

}
uint32
   arad_pp_flp_lookups_TRILL_mc(
     int unit,
     uint32 is_ingress_learn,
     uint32 tcam_access_profile_id
   )
{
   uint32
    res,
    prog /* one tag or two tags */;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;
  uint8 is_two_tags;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* The only different between the single/two tags prorgams is whether param3 is all zeros, or inner vid */
  for (is_two_tags = 0; is_two_tags <= 1; ++is_two_tags) {
      prog = is_two_tags ?
          PROG_FLP_TRILL_MC_TWO_TAGS :
          PROG_FLP_TRILL_MC_ONE_TAG;
      res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog, &flp_lookups_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

      flp_lookups_tbl.lem_1st_lkp_valid     = 1;
      flp_lookups_tbl.lem_1st_lkp_key_select = 1;
      flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
      res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_TRILL_ADJ_KEY_OR_MASK,&flp_lookups_tbl.lem_1st_lkp_or_value);
      SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
      
      flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
      flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
      flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
      flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_TRILL_KEY_OR_MASK_MC;
      flp_lookups_tbl.learn_key_select = (is_ingress_learn ? 0x1 : 0x0); /* For ingress learning, learn the key. For egress, take from forwarding header */
      
	  if (is_two_tags) {
          flp_lookups_tbl.tcam_lkp_key_select = ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL;
          flp_lookups_tbl.tcam_lkp_db_profile = tcam_access_profile_id;      
          flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;
          flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;
	  }

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_TRILL_MC &&
         (prog == PROG_FLP_TRILL_MC_ONE_TAG)){
          flp_lookups_tbl.elk_lkp_valid = 0x1;
          flp_lookups_tbl.elk_wait_for_reply = 0x1;
          flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_MC;

          flp_lookups_tbl.elk_key_a_valid_bytes = 4; 
          flp_lookups_tbl.elk_key_b_valid_bytes = 0;  
          flp_lookups_tbl.elk_key_c_valid_bytes = 0; 
      }
#endif
      res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog, &flp_lookups_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_TRILL_mc", 0, 0);
}


uint32
   arad_pp_flp_lookups_pon_vmac_upstream(
     int unit,
     int32  prog_id,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* FlpLookups_vmac_upstream */ 
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* 1st */  
  flp_lookups_tbl.lem_1st_lkp_valid      = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0; /* A: learning */
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);

  /* 2nd */  
  flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 0; /* A: vmac llid index */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_OMAC_2_VMAC_KEY_OR_MASK,&flp_lookups_tbl.lem_2nd_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  flp_lookups_tbl.learn_key_select       = 0;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_pon_vmac_upstream", 0, 0);
}

uint32
   arad_pp_flp_lookups_pon_vmac_downstream(
     int unit,
     uint32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* FlpLookups_vmac_downstream */ 
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  /* 1st */  
  flp_lookups_tbl.lem_1st_lkp_valid      = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0; /* key A: vmac->omac */
  flp_lookups_tbl.lem_1st_lkp_key_type   = 0; 
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_VMAC_2_OMAC_KEY_OR_MASK,&flp_lookups_tbl.lem_1st_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* 2nd */  
  flp_lookups_tbl.lem_2nd_lkp_valid      = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; /* key A: FID forward DA omac[0:4]*/
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0;
  res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_VMAC_KEY_OR_MASK,&flp_lookups_tbl.lem_2nd_lkp_or_value);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  flp_lookups_tbl.learn_key_select       = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_pon_vmac_downstream", 0, 0);
}

uint32
   arad_pp_flp_process_ipv4uc_l3vpn_rpf(
     int unit,
     int prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv4uc_l3vpn_rpf");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
  flp_process_tbl.include_lpm_1st_in_result_b    = 1;
  flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
  flp_process_tbl.result_a_format            = 2;
  flp_process_tbl.result_b_format            = 2;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 1;
  /* take VRF default destination */
  flp_process_tbl.select_default_result_a = 0;
  flp_process_tbl.select_default_result_b = 0;
  flp_process_tbl.sa_lkp_process_enable = 0;
  flp_process_tbl.not_found_trap_strength = 3;

  flp_process_tbl.lpm_2nd_lkp_enable_default = 1;
  flp_process_tbl.lpm_public_2nd_lkp_enable_default = 1;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 1; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4uc_l3vpn_rpf", 0, 0);
}

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_process_ipv6uc_l3vpn(
     int unit,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv6uc");*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lpm_2nd_in_result_a     = 1;
  flp_process_tbl.result_a_format            = 2;
  flp_process_tbl.procedure_ethernet_default  = 0;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 0;
  flp_process_tbl.sa_lkp_process_enable       = 0;
  /* take VRF default destination */
  flp_process_tbl.not_found_trap_strength       = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6;
  /* 0x0 - Use VRF Default Unicast
     0x1 - Use VRF Default Multicast
     Else: Use NotFoundTrapCode from program */
  flp_process_tbl.select_default_result_a = 0;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(JER_KAPS_ENABLE(unit)) {
        flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
        flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
    }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  /* 0: Apply Ethernet traps on forwarding-header
     1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
     2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
     3: Apply MPLS traps on forwarding-header
     4: Apply FC traps on forwarding-header
     Else: Don't apply any protocol traps */
  tmp = 2;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv6uc", 0, 0);
}
#endif

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_process_ipv6uc_l3vpn_rpf(
     int unit,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv6uc");*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lpm_1st_in_result_b     = 1;
  flp_process_tbl.include_lpm_2nd_in_result_a     = 1;

  flp_process_tbl.result_a_format            = 2;
  flp_process_tbl.result_b_format            = 2;
  flp_process_tbl.procedure_ethernet_default  = 0;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 1;
  flp_process_tbl.sa_lkp_process_enable       = 0;
  /* take VRF default destination */
  flp_process_tbl.not_found_trap_strength       = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6;
  /* 0x0 - Use VRF Default Unicast
     0x1 - Use VRF Default Multicast
     Else: Use NotFoundTrapCode from program */
  flp_process_tbl.select_default_result_a = 0;
  flp_process_tbl.select_default_result_b = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(JER_KAPS_ENABLE(unit)) {
        flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
        flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
        flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
        flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
    }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  /* 0: Apply Ethernet traps on forwarding-header
     1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
     2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
     3: Apply MPLS traps on forwarding-header
     4: Apply FC traps on forwarding-header
     Else: Don't apply any protocol traps */
  tmp = 2;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv6uc", 0, 0);
}
#endif


uint32
   arad_pp_flp_process_ipv4uc_l3vpn(
     int unit,
     int custom_prgrm
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  int prog_id;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  if (custom_prgrm) {
      prog_id = custom_prgrm;
  } else {
      prog_id = PROG_FLP_IPV4UC_PUBLIC;
  }

  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv4uc_with_rpf");*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl); 
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  if (SOC_IS_ARADPLUS(unit)) {
      flp_process_tbl.include_lem_2nd_in_result_a    = 4;
      flp_process_tbl.include_lem_1st_in_result_a    = 2; /* VPN entry lower priority */
  } else {
      /* Arad B0 and below doesn't support priority on the result */
      flp_process_tbl.include_lem_2nd_in_result_a    = 1;
      flp_process_tbl.include_lem_1st_in_result_b    = 1;
  }
  flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
  if(SOC_IS_JERICHO(unit)){
      flp_process_tbl.include_lpm_1st_in_result_a    = 3;
  }
  flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
  flp_process_tbl.result_a_format            = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.result_b_format            = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 0;
  /* take VRF default destination */
  flp_process_tbl.select_default_result_a = 0;
  flp_process_tbl.select_default_result_b = 0;
  flp_process_tbl.sa_lkp_process_enable = 0;
  flp_process_tbl.not_found_trap_strength = 3;


  if (custom_prgrm) {
      flp_process_tbl.include_lem_1st_in_result_b    = 0;
      flp_process_tbl.include_lpm_1st_in_result_b    = 0;

      flp_process_tbl.include_lem_2nd_in_result_a    = 0; /* 2nd result is for PMF use in this case */
      flp_process_tbl.include_lem_1st_in_result_a    = 1;
      flp_process_tbl.include_lpm_2nd_in_result_a    = 1; 
  }

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      
  }
  else if(JER_KAPS_ENABLE(unit)) {
      flp_process_tbl.lpm_2nd_lkp_enable_default = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default = 1;
  }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 1; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_l3vpn", 0, 0);
}



uint32
   arad_pp_flp_process_ipv4uc(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv4uc_with_rpf");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV4UC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;

  if (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) {
      flp_process_tbl.include_lem_2nd_in_result_a    = 1;
      flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
      flp_process_tbl.include_lem_1st_in_result_a    = 2;
      flp_process_tbl.include_lpm_1st_in_result_a    = 2;
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
  }

  flp_process_tbl.result_a_format            = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.result_b_format            = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 0;
  /* take VRF default destination */
  flp_process_tbl.select_default_result_a = 0;
  flp_process_tbl.select_default_result_b = 0;
  flp_process_tbl.sa_lkp_process_enable = 0;
  flp_process_tbl.not_found_trap_strength = 3;

  
  if (SOC_IS_JERICHO(unit)) {
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(JER_KAPS_ENABLE(unit)) {
          flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
          flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
          flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
          flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
      }
      else 
#endif
      {
          flp_process_tbl.include_tcam_in_result_a    = 1;
          flp_process_tbl.include_tcam_1_in_result_b  = 1;
      }
  }


#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_process_tbl.elk_result_format = 1;
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
      flp_process_tbl.include_elk_ext_in_result_a = 0;
      flp_process_tbl.include_elk_fwd_in_result_b = 0;
      flp_process_tbl.include_elk_ext_in_result_b = 1;
	  if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
		  res = process_tbl_learn_enable_set(unit, &flp_process_tbl);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
	  }
      if(SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1){
          flp_process_tbl.include_lem_2nd_in_result_a    = 7;
      }
      if (SOC_IS_JERICHO(unit)) {/* in Jericho with KBP the LEM has higher priority */
          flp_process_tbl.include_lem_2nd_in_result_a    = 2;
          flp_process_tbl.include_lem_1st_in_result_b    = 2;
      }
      
  }
  else
#endif
  {
      flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
      flp_process_tbl.include_lpm_1st_in_result_b    = 1;
  }

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV4UC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 1; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV4UC,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4uc", 0, 0);
}


uint32
   arad_pp_flp_process_ipv4_dc(
     int unit
   )
{
  uint32 res;
  uint32 tmp;
  soc_reg_above_64_val_t reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV4_DC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.result_a_format               = 2;
  flp_process_tbl.result_b_format               = 2;
  flp_process_tbl.procedure_ethernet_default    = 3;
  flp_process_tbl.enable_hair_pin_filter        = 1;
  flp_process_tbl.enable_rpf_check              = 0;
  /* take VRF default destination */
  flp_process_tbl.select_default_result_a       = 0;
  flp_process_tbl.select_default_result_b       = 0;
  flp_process_tbl.sa_lkp_process_enable         = 0;
  flp_process_tbl.not_found_trap_strength       = 3;

  flp_process_tbl.elk_result_format = 1;
  flp_process_tbl.include_elk_fwd_in_result_a = 1;
  flp_process_tbl.include_elk_ext_in_result_a = 1;
  flp_process_tbl.include_elk_fwd_in_result_b = 0;
  flp_process_tbl.include_elk_ext_in_result_b = 0;

  /* in Jericho with KBP the LEM has higher priority */
  flp_process_tbl.include_lem_2nd_in_result_a    = 3;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV4_DC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 1; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV4_DC,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

  exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4_dc", 0, 0);
}

uint32
   arad_pp_flp_process_bfd_single_hop(
     int unit,
     int is_ipv6,
	 int prog
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  SOC_PPC_TRAP_CODE
    trap_code;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_PPC_TRAP_CODE_INTERNAL   trap_code_internal;
  ARAD_SOC_REG_FIELD   strength_fld_fwd,
                       strength_fld_snp;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "common_bfd_discr_not_found_trap", 0)) {
        /* It is possible (according to soc prop) to use the same trap for IPv6 and IPv4 your discr not found.*/
        trap_code =SOC_PPC_TRAP_CODE_BFD_IPV4_IPV6_YOUR_DISCR_NOT_FOUND; 
    } else {
        trap_code = (is_ipv6 ? SOC_PPC_TRAP_CODE_BFD_IPV6_YOUR_DISCR_NOT_FOUND : SOC_PPC_TRAP_CODE_BFD_IPV4_YOUR_DISCR_NOT_FOUND); 
    }

    /* Used the pre-allocated User_Defined_Trap code and map it to the HW Code */
    res = arad_pp_trap_mgmt_trap_code_to_internal(unit, trap_code, &trap_code_internal, &strength_fld_fwd, &strength_fld_snp);
    SOC_SAND_CHECK_FUNC_RESULT(res, 240, exit);  

    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    flp_process_tbl.include_lem_2nd_in_result_a    = 1;
    flp_process_tbl.include_lem_1st_in_result_a    = 0;
    flp_process_tbl.include_lpm_2nd_in_result_a    = 0;
    flp_process_tbl.include_lem_1st_in_result_b    = 0;
    flp_process_tbl.include_lem_2nd_in_result_b    = 1;
    flp_process_tbl.result_a_format            = 2;
    flp_process_tbl.result_b_format            = 2;
    flp_process_tbl.procedure_ethernet_default  = 3;
    flp_process_tbl.enable_hair_pin_filter       = 1;
    flp_process_tbl.enable_rpf_check            = 0;
    flp_process_tbl.not_found_trap_code         = trap_code_internal;
    flp_process_tbl.not_found_trap_strength     = 7;

    flp_process_tbl.select_default_result_a = 3;
    flp_process_tbl.select_default_result_b = 3;

    if (SOC_IS_JERICHO(unit)) {
      flp_process_tbl.include_tcam_in_result_a    = 1;
      flp_process_tbl.include_tcam_1_in_result_b  = 1;
    }

    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    tmp = (is_ipv6 ? 2 : 1); /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    SHR_BITCOPY_RANGE(reg_val,3*prog,&tmp,0,3);
    res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
    exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4uc", 0, 0);
}


uint32
   arad_pp_flp_process_TRILL_uc(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("PROG_FLP_TRILL_UC");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_TRILL_UC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lpm_2nd_in_result_a    = 0;
  flp_process_tbl.apply_fwd_result_a           = 1; /*  apply alwayes, even if not found, i.e. do Trap if not found */
  flp_process_tbl.not_found_trap_code          = SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_UC;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.procedure_ethernet_default  = 2; /*  if found do procedure_ethernet_default -i.e. BC */
  flp_process_tbl.sa_lkp_process_enable       = 0;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_TRILL_UC){
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
  }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_TRILL_UC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 5; /* >= ARAD-B0: Apply Ethernet traps on forwarding-header + 1  
              ARAD-A0: no traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);

  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_TRILL_UC,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_TRILL", 0, 0);
}

uint32
   arad_pp_flp_process_TRILL_mc(
     int unit,
     uint8 ingress_learn_enable
   )
{
  uint32
    res;
  uint32
    tmp,
    prog /* one tag or two tags */;
  uint8 is_two_tags;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /* The only different between the single/two tags prorgams is whether param3 is all zeros, or inner vid */
  for (is_two_tags = 0; is_two_tags <= 1; ++is_two_tags) {
      prog = is_two_tags ?
          PROG_FLP_TRILL_MC_TWO_TAGS :
          PROG_FLP_TRILL_MC_ONE_TAG;

      res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog, &flp_process_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

      flp_process_tbl.include_lem_1st_in_result_b    = 1;
      flp_process_tbl.include_lem_2nd_in_result_a    = 1;
      flp_process_tbl.include_lpm_2nd_in_result_a    = 0;
      flp_process_tbl.include_tcam_in_result_a       = 1;
      flp_process_tbl.apply_fwd_result_a           = 1; /*  apply alwayes, even if not found, i.e. do Trap if not found */
      flp_process_tbl.not_found_trap_code          = SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_MC;
      flp_process_tbl.result_a_format            = 0;
      flp_process_tbl.result_b_format            = 0;
      flp_process_tbl.procedure_ethernet_default  = 2; /*  if found do procedure_ethernet_default -i.e. BC */
      flp_process_tbl.enable_hair_pin_filter       = 1;
      flp_process_tbl.enable_rpf_check            = 0;
      /* 
       * if egress learning, we learn the native SA.
       * if ingress learning, we learn the link layer SA, which is undesired and so, disabled. 
       */ 
      flp_process_tbl.learn_enable                 = (ingress_learn_enable) ? 0 : 1;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_TRILL_MC &&
         (prog == PROG_FLP_TRILL_MC_ONE_TAG)){
          flp_process_tbl.include_elk_fwd_in_result_a = 1;
      }
#endif

      res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog, &flp_process_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

      tmp = 5; /* >= ARAD-B0: Apply Ethernet traps on forwarding-header + 1  
                 ARAD-A0: no traps  */
      SOC_REG_ABOVE_64_CLEAR(reg_val);
      res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
      SHR_BITCOPY_RANGE(reg_val,3*prog,&tmp,0,3);
      res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_TRILL_mc", 0, 0);
}

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_process_ipv6uc_with_rpf(
     int unit,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv6uc");*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lpm_1st_in_result_b     = 1;
  flp_process_tbl.include_lpm_2nd_in_result_a     = 1;

  flp_process_tbl.result_a_format            = 2;
  flp_process_tbl.result_b_format            = 2;
  flp_process_tbl.procedure_ethernet_default  = 0;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.enable_rpf_check            = 1;
  flp_process_tbl.sa_lkp_process_enable       = 0;
  /* take VRF default destination */
  flp_process_tbl.not_found_trap_strength       = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6;
  /* 0x0 - Use VRF Default Unicast
     0x1 - Use VRF Default Multicast
     Else: Use NotFoundTrapCode from program */
  flp_process_tbl.select_default_result_a = 0;
  flp_process_tbl.select_default_result_b = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_EXTENDED){      
      flp_process_tbl.elk_result_format = 1;
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
      flp_process_tbl.include_elk_ext_in_result_a = 0;
      flp_process_tbl.include_elk_fwd_in_result_b = 0;
      flp_process_tbl.include_elk_ext_in_result_b = 1;
  } else if(JER_KAPS_ENABLE(unit)) {
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
      flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
  }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  /* 0: Apply Ethernet traps on forwarding-header
     1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
     2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
     3: Apply MPLS traps on forwarding-header
     4: Apply FC traps on forwarding-header
     Else: Don't apply any protocol traps */
  tmp = 2;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv6uc", 0, 0);
}
#endif

uint32
   arad_pp_flp_process_ipv6uc(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv6uc");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV6UC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  if (SOC_IS_JERICHO(unit) && !(soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "l3_ipv6_uc_use_tcam", 0)) &&
                              !((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long))) {
      flp_process_tbl.include_lpm_2nd_in_result_a         = 1;
      flp_process_tbl.lpm_2nd_lkp_enable_default          = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default   = 1;
  } else if ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) {
      /* Long KAPS > Long LEM > Short LEM > Short KAPS */
      flp_process_tbl.include_lem_2nd_in_result_a    = 1;
      flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
      flp_process_tbl.include_lem_1st_in_result_a    = 2;
      flp_process_tbl.include_lpm_1st_in_result_a    = 3;
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
  } else {
      flp_process_tbl.include_tcam_in_result_a      = 1;
  }
  flp_process_tbl.result_a_format             = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.result_b_format             = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0 : 2;
  flp_process_tbl.procedure_ethernet_default  = 0;
  flp_process_tbl.enable_hair_pin_filter      = 1;
  flp_process_tbl.enable_rpf_check            = 0;
  flp_process_tbl.sa_lkp_process_enable       = 0;
  /* take VRF default destination */
  flp_process_tbl.not_found_trap_strength     = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6;
  /* 0x0 - Use VRF Default Unicast
     0x1 - Use VRF Default Multicast
     Else: Use NotFoundTrapCode from program */
  flp_process_tbl.select_default_result_a = 0; 
  flp_process_tbl.select_default_result_b = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_EXTENDED){
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
  }
  if (ARAD_KBP_ENABLE_IPV6_EXTENDED) {
	  res = process_tbl_learn_enable_set(unit, &flp_process_tbl);
	  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
  }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV6UC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  /* 0: Apply Ethernet traps on forwarding-header
     1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
     2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
     3: Apply MPLS traps on forwarding-header
     4: Apply FC traps on forwarding-header
     Else: Don't apply any protocol traps */
  tmp = 2; 
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV6UC,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv6uc", 0, 0);
}

uint32
   arad_pp_flp_process_ipv6mc(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV6MC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);


  if (SOC_IS_JERICHO(unit) && (SOC_DPP_CONFIG(unit)->pp.l3_mc_use_tcam == ARAD_PP_FLP_L3_MC_USE_TCAM_DISABLE)) {
      flp_process_tbl.include_lpm_1st_in_result_b        = 1;
      flp_process_tbl.lpm_1st_lkp_enable_default         = 1;
      flp_process_tbl.lpm_public_1st_lkp_enable_default  = 1;
      flp_process_tbl.include_lpm_2nd_in_result_a        = 1;
      flp_process_tbl.lpm_2nd_lkp_enable_default         = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default  = 1;
  } else {
      flp_process_tbl.include_tcam_in_result_a           = 1;
  }
  flp_process_tbl.result_a_format               = 0;
  flp_process_tbl.result_b_format               = 0;
  flp_process_tbl.procedure_ethernet_default    = 3;
  flp_process_tbl.enable_hair_pin_filter        = 1;
  flp_process_tbl.enable_rpf_check              = (SOC_IS_JERICHO(unit)) ? 1 : 0;
  flp_process_tbl.compatible_mc_bridge_fallback = 1;
  flp_process_tbl.sa_lkp_process_enable         = 0;
  /* take VRF default destination */
  flp_process_tbl.not_found_trap_strength       = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_MCV6;
  /* 0x0 - Use VRF Default Unicast
     0x1 - Use VRF Default Multicast
     Else: Use NotFoundTrapCode from program */
  flp_process_tbl.select_default_result_a = 1; 
  flp_process_tbl.select_default_result_b = 1;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV6_MC){
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
      flp_process_tbl.include_elk_ext_in_result_b = 1;
  }
#endif

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV6MC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  /* 0: Apply Ethernet traps on forwarding-header
     1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header
     2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header
     3: Apply MPLS traps on forwarding-header
     4: Apply FC traps on forwarding-header
     Else: Don't apply any protocol traps */
  tmp = 2; 
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV6MC,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv6mc", 0, 0);
}

uint32
   arad_pp_flp_process_lsr(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_lsr");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_LSR, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_MPLS || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
	  if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
		  res = process_tbl_learn_enable_set(unit, &flp_process_tbl);
		  SOC_SAND_CHECK_FUNC_RESULT(res, 247, exit);
	  }
  }
  else
#endif
  {
      flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
      flp_process_tbl.include_lpm_1st_in_result_b    = 1;
      flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;

      flp_process_tbl.include_lem_2nd_in_result_a    = 1;
      flp_process_tbl.include_lem_1st_in_result_b    = 1;
  }

  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.procedure_ethernet_default     = 3;
  flp_process_tbl.enable_hair_pin_filter         = 0;
  flp_process_tbl.enable_rpf_check               = 0;
  flp_process_tbl.enable_lsr_p2p_service         = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_UNKNOWN_LABEL;


  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_LSR, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 3; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_LSR,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_lsr", 0, 0);
}

uint32
   arad_pp_flp_process_pwe_gre(
     int unit,
     int prog
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_lsr");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
  flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
  flp_process_tbl.include_lpm_1st_in_result_b    = 1;
  flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.procedure_ethernet_default     = 3;
  flp_process_tbl.enable_hair_pin_filter         = 0;
  flp_process_tbl.enable_rpf_check               = 0;
  flp_process_tbl.enable_lsr_p2p_service         = 0;
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_UNKNOWN_LABEL;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  /* We can't apply any traps becuase initial processing think header is Ethernet even tough it is MPLS. */
  tmp = 5; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_pwe_gre", 0, 0);
}

uint32
   arad_pp_flp_process_fcoe_fcf_npv(
     int unit,
     int32  progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS]
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);  
  
  /* FlpProcess_fcf */
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, progs[0], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_1st_in_result_a    = 0; /* frwrd */
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.include_lem_1st_in_result_b    = 0;
  flp_process_tbl.include_lpm_2nd_in_result_b    = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.procedure_ethernet_default     = 0;
  flp_process_tbl.enable_hair_pin_filter         = 0;
  flp_process_tbl.learn_enable                   = 0;

 /* forwarding not found */
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_FCF;
  flp_process_tbl.not_found_trap_strength = 7;
  flp_process_tbl.not_found_snoop_strength = 0;

  /* lpm in forwarding for NPV */
  flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
  flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, progs[0], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, progs[1], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  
  tmp = 4; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);  
  SHR_BITCOPY_RANGE(reg_val,3*progs[0],&tmp,0,3);
  SHR_BITCOPY_RANGE(reg_val,3*progs[1],&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_fcoe_fcf_npv", 0, 0);

}


uint32
   arad_pp_flp_process_fcoe_fcf(
     int unit,
     int32  progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS]
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);  
  
  /* FlpProcess_fcf */
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, progs[0], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_1st_in_result_a    = 0; /* frwrd */
  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.result_a_format                = 0;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.include_lpm_2nd_in_result_b    = 0;
  flp_process_tbl.result_b_format                = 0;
  flp_process_tbl.procedure_ethernet_default     = 0;
  flp_process_tbl.enable_hair_pin_filter         = 0;
  flp_process_tbl.learn_enable                   = 0;

  /* forwarding not found */
  flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_FCF;
  flp_process_tbl.not_found_trap_strength = 7;
  flp_process_tbl.not_found_snoop_strength = 0;

  /* zonning not found: point to not found trap as well */
  flp_process_tbl.select_default_result_b = 2;
  flp_process_tbl.apply_fwd_result_a = 1;


  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, progs[0], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, progs[1], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  /* lpm in forwarding for remote */
  flp_process_tbl.include_lpm_2nd_in_result_a    = 1;
  flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, progs[2], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 44, exit);

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, progs[3], &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);


  tmp = 4; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit);
  SHR_BITCOPY_RANGE(reg_val,3*progs[0],&tmp,0,3);
  SHR_BITCOPY_RANGE(reg_val,3*progs[1],&tmp,0,3);
  SHR_BITCOPY_RANGE(reg_val,3*progs[2],&tmp,0,3);
  SHR_BITCOPY_RANGE(reg_val,3*progs[3],&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 47, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_fcoe_fcf", 0, 0);
}

uint32
    arad_pp_flp_prog_sel_cam_ipv4mc_bridge_v4mc(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_ETHERNET_ING_LEARN,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_ipv4mc_bridge_v4mc_cam_sel_id.set(unit,cam_sel_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                   = 0;
  prog_selection_cam_tbl.ptc_profile                    = 0;
  prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x0;
  prog_selection_cam_tbl.custom_rif_profile_bit        = 0x0;
  prog_selection_cam_tbl.packet_is_compatible_mc        = 1;

  /* Next protocol is IPV4. PFC checking is not good enough for VPLS and Overlay packets such as VXLAN */
  prog_selection_cam_tbl.forwarding_header_qualifier      = ARAD_PP_FLP_QLFR_ETH_NEXT_PROTOCOL_IPV4; 
  prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID |  
                                                            ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_INNER_TPID);
                                                            

  prog_selection_cam_tbl.parser_leaf_context_mask       = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask               = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1;/* ignore rpf */
  /* In case theipmc_l3mcastl2_mode SOC is on the 3rd bit should be enabled by this program selection*/
  prog_selection_cam_tbl.custom_rif_profile_bit_mask   = (SOC_DPP_CONFIG(unit)->pp.ipmc_l3mcastl2_mode == 1) ?0x0 : 0x1/*ignore*/;
  prog_selection_cam_tbl.packet_is_compatible_mc_mask   = 0x0; /* compatible MC only*/
  prog_selection_cam_tbl.packet_format_code_mask        = 0x3F; 
  prog_selection_cam_tbl.program                        = PROG_FLP_ETHERNET_ING_LEARN;
  prog_selection_cam_tbl.valid                          = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4mc_bridge", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_ipv4mc_bridge(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4MC_BRIDGE,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  prog_selection_cam_tbl.parser_leaf_context            = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                   = 0;
  prog_selection_cam_tbl.ptc_profile                    = 0;
  prog_selection_cam_tbl.forwarding_code                = ARAD_PP_FWD_CODE_ETHERNET;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable           = 0x0;
  prog_selection_cam_tbl.custom_rif_profile_bit        = 0x1;
  prog_selection_cam_tbl.packet_is_compatible_mc        = 1;

  /* Next protocol is IPV4. PFC checking is not good enough for VPLS and Overlay packets such as VXLAN */
  prog_selection_cam_tbl.forwarding_header_qualifier      = ARAD_PP_FLP_QLFR_ETH_NEXT_PROTOCOL_IPV4;
  prog_selection_cam_tbl.forwarding_header_qualifier_mask = (ARAD_PP_FLP_QLFR_ETH_MASK_ENCAP | ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_TPID |
                                                            ARAD_PP_FLP_QLFR_ETH_MASK_OUTER_PCP | ARAD_PP_FLP_QLFR_ETH_MASK_INNER_TPID);


  prog_selection_cam_tbl.parser_leaf_context_mask       = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask              = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask               = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask           = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask      = 0x1;/* ignore rpf */
  prog_selection_cam_tbl.custom_rif_profile_bit_mask   = 0x0;
  prog_selection_cam_tbl.packet_is_compatible_mc_mask   = 0x0; /* compatible MC only*/
  prog_selection_cam_tbl.packet_format_code_mask        = 0x3F;
  prog_selection_cam_tbl.program                        = PROG_FLP_IPV4MC_BRIDGE;
  prog_selection_cam_tbl.valid                          = 1;

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4mc_bridge_v4mc", 0, 0);
}



uint32
   arad_pp_flp_key_const_ipv4mc_bridge(
     int unit
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b = 0;
  flp_key_construction_tbl.instruction_2_16b = 0;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
  flp_key_construction_tbl.instruction_5_32b = arad_pp_ce_instruction_composer(28,ARAD_PP_CE_HEADER_AFTER_FWD,128,ARAD_PP_CE_IS_CE32);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0xB /*6'b001011*/; /* FID, SA, for learning */
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x21 /*hex 6'b100001*/; /*  FID,DIP[27:0] for forwarding */
  
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.instruction_0_16b = 0;
  flp_key_construction_tbl.instruction_1_16b = 0;
  flp_key_construction_tbl.instruction_2_16b = 0;
  flp_key_construction_tbl.instruction_3_32b = 0;
  flp_key_construction_tbl.instruction_4_32b = 0;
  flp_key_construction_tbl.instruction_5_32b = 0;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv4mc_bridge", 0, 0);
}


uint32
   arad_pp_flp_lookups_ipv4mc_bridge_update(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  /* learning lookup on SA lem1-keyb*/   
  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv4mc_bridge", 0, 0);
}

uint32
   arad_pp_flp_lookups_ipv4mc_bridge(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  /* learning lookup on SA lem1-keyb*/   
  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);


  /* LEM2 (forward): {RIF,SIP} - key-B */
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1; /* key-b: RIF,DIP */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_COMP_KEY_OR_MASK;
  flp_lookups_tbl.learn_key_select      = 0;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv4mc_bridge", 0, 0);
}


uint32
   arad_pp_flp_process_ipv4mc_bridge(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv4uc_with_rpf");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 0;
  flp_process_tbl.unknown_address              = 3;

  if((SOC_IS_JERICHO(unit)) && (SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode == BCM_IPMC_SSM_KAPS_LPM)) {
      flp_process_tbl.include_lpm_2nd_in_result_a = 2;
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
  } else if ((SOC_IS_ARADPLUS(unit)) && (SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode == BCM_IPMC_SSM_TCAM_LPM)) {
      flp_process_tbl.include_tcam_in_result_a    = 2;
  }

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV4MC_BRIDGE, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  
  tmp = 0; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV4MC_BRIDGE,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4mc_bridge", 0, 0);
}



/*  IPv4 compatible multicast */
/*  ------------------------- */

uint32
   arad_pp_flp_prog_sel_cam_ipv4compmc_with_rpf(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_IPV4COMPMC_WITH_RPF,TRUE,TRUE,&cam_sel_id, NULL);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    
  prog_selection_cam_tbl.parser_leaf_context          = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                 = 0;
  prog_selection_cam_tbl.ptc_profile                  = 0;
  prog_selection_cam_tbl.forwarding_code              = ARAD_PP_FWD_CODE_IPV4_MC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable         = 0x0;
  prog_selection_cam_tbl.packet_is_compatible_mc      = 1;

  prog_selection_cam_tbl.parser_leaf_context_mask     = 0x03; /* ignore 2 LSBs because 4 higher Parser-Leaf-Contexts are considered regular PP for FLP */
  prog_selection_cam_tbl.port_profile_mask            = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask             = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask         = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask    = 0x1;
  prog_selection_cam_tbl.packet_is_compatible_mc_mask = 0x1; /* ignore compatible if forward code is ipmc then ipmc*/

  prog_selection_cam_tbl.program                   = PROG_FLP_IPV4COMPMC_WITH_RPF;
  prog_selection_cam_tbl.valid                     = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_ipv4compmc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_key_const_ipv4compmc_with_rpf(
     int unit
   )
{
  uint32
    res, vrf_ce_inst = 0, rif_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_2_16b = 0;
  flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_SIP_FWD_HEADER;
  flp_key_construction_tbl.instruction_4_32b = 0;
  flp_key_construction_tbl.instruction_5_32b = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_MC){
      flp_key_construction_tbl.instruction_5_32b = ARAD_PP_CE_DIP_FWD_HEADER;
      flp_key_construction_tbl.instruction_0_16b = vrf_ce_inst;
	  if (SOC_IS_JERICHO(unit)) {
          flp_key_construction_tbl.instruction_1_16b = ARAD_PP_FLP_16B_INST_N_ZEROS(1);
      } else {
          flp_key_construction_tbl.instruction_1_16b = ARAD_PP_FLP_16B_INST_N_ZEROS(4);
      }
      flp_key_construction_tbl.instruction_2_16b = rif_ce_inst;

      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0xf;  /*  VRF + In-RIF + SIP*/
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x20; /*  DIP[27:0] */
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0;
  }
  else
#endif
  {
      flp_key_construction_tbl.instruction_0_16b = rif_ce_inst;
      flp_key_construction_tbl.instruction_1_16b = vrf_ce_inst;
      flp_key_construction_tbl.instruction_4_32b = ARAD_PP_CE_DIP_FWD_HEADER;

      flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0xA /*hex 6'b001010*/; /*  VRF,SIP */
      flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x12 /*hex 6'b010010*/; /*  VRF,DIP */
      flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0;
  }


  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  flp_key_construction_tbl.instruction_0_16b = 0;
  flp_key_construction_tbl.instruction_1_16b = 0;
  flp_key_construction_tbl.instruction_2_16b = 0;
  flp_key_construction_tbl.instruction_3_32b = 0;
  flp_key_construction_tbl.instruction_4_32b = 0;
  flp_key_construction_tbl.instruction_5_32b = 0;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_ipv4compmc_with_rpf", 0, 0);
}


uint32
   arad_pp_flp_lookups_ipv4compmc_with_rpf(
     int unit,
     uint32 tcam_access_profile_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  {
      /* LEM1: {VRF,DIP} - key-B */
      flp_lookups_tbl.lem_1st_lkp_valid     = 1;
      flp_lookups_tbl.lem_1st_lkp_key_select = 1;
      flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
      flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;


      /* TCAM: {RIF,SIP,DIP} - key-C by DBAL*/



      /* LPM2/LEM2(RPF): {VRF,SIP} - key-A */
      flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
      flp_lookups_tbl.lem_2nd_lkp_key_select = 0;
      flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
      flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;

      flp_lookups_tbl.lpm_2nd_lkp_valid     = 1;
      flp_lookups_tbl.lpm_2nd_lkp_key_select = 0;
      flp_lookups_tbl.lpm_2nd_lkp_and_value = 3;
      flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;

      flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

    #if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_IPV4_MC){
          flp_lookups_tbl.elk_lkp_valid = 0x1;
          flp_lookups_tbl.elk_wait_for_reply = 0x1;
          flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_MC_COMP;

          flp_lookups_tbl.elk_key_a_valid_bytes = 8;
          flp_lookups_tbl.elk_key_b_valid_bytes = 4;
          flp_lookups_tbl.elk_key_c_valid_bytes = 0;
      }
    #endif
  }

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_ipv4compmc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_process_ipv4compmc_with_rpf(
     int unit
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ipv4uc_with_rpf");*/
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  if (SOC_IS_JERICHO(unit)) {
      flp_process_tbl.include_lpm_1st_in_result_b     = 1;
      flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;
      flp_process_tbl.include_lpm_2nd_in_result_a     = 2;
      flp_process_tbl.lpm_2nd_lkp_enable_default     = 1;
      flp_process_tbl.lpm_public_2nd_lkp_enable_default     = 1;
	  flp_process_tbl.include_lem_1st_in_result_b     = 1;
	  flp_process_tbl.include_lem_2nd_in_result_a     = 1;
  }
  else {
      flp_process_tbl.include_tcam_in_result_a       = 1;
      flp_process_tbl.include_lem_1st_in_result_a     = 1;

      flp_process_tbl.include_lem_2nd_in_result_b     = 1;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_IPV4_MC) {
      }
	  else
#endif
      {
          flp_process_tbl.include_lpm_2nd_in_result_b     = 1;
      }
  }

  if (SOC_DPP_CONFIG(unit)->pp.l3_mc_use_tcam != ARAD_PP_FLP_L3_MC_USE_TCAM_DISABLE) {
      flp_process_tbl.include_tcam_in_result_a       = 1;
  }
  flp_process_tbl.result_a_format             = 0;
  flp_process_tbl.result_b_format             = 0;
  flp_process_tbl.compatible_mc_bridge_fallback = 1;
  flp_process_tbl.procedure_ethernet_default   = 3;
  flp_process_tbl.enable_hair_pin_filter        = 1;
  flp_process_tbl.enable_rpf_check             = 1;
  flp_process_tbl.not_found_trap_strength       = 0; /*  if not found dont trap */
    /* take VRF default destination */
  flp_process_tbl.select_default_result_a = 1;
  flp_process_tbl.select_default_result_b = 1;
  flp_process_tbl.sa_lkp_process_enable = 0;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_MC){
      flp_process_tbl.elk_result_format = 1;
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
      flp_process_tbl.include_elk_ext_in_result_a = 0;
      flp_process_tbl.include_elk_fwd_in_result_b = 0;
      flp_process_tbl.include_elk_ext_in_result_b = 1;
  }
#endif


  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  tmp = 1;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*PROG_FLP_IPV4COMPMC_WITH_RPF,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_ipv4compmc_with_rpf", 0, 0);
}


/*  Global IPv4 compatible multicast */
/*  ------------------------- */
uint32
   arad_pp_flp_prog_sel_cam_global_ipv4compmc_with_rpf(
     int unit,
     int32 *prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;

  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_GLOBAL_IPV4COMPMC_WITH_RPF,FALSE,TRUE,&cam_sel_id, prog_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
   
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  prog_selection_cam_tbl.parser_leaf_context          = ARAD_PARSER_PLC_PP;
  prog_selection_cam_tbl.port_profile                 = 0;
  prog_selection_cam_tbl.ptc_profile                  = 0;
  prog_selection_cam_tbl.forwarding_code              = ARAD_PP_FWD_CODE_IPV4_MC;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable         = 0x0;
  prog_selection_cam_tbl.packet_is_compatible_mc      = 1;

  prog_selection_cam_tbl.parser_leaf_context_mask     = 0x01;
  prog_selection_cam_tbl.port_profile_mask            = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_PBP;
  prog_selection_cam_tbl.ptc_profile_mask             = 0x03;
  prog_selection_cam_tbl.forwarding_code_mask         = 0x00;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask    = 0x1;
  prog_selection_cam_tbl.packet_is_compatible_mc_mask = 0x1; /* ignore compatible if forward code is ipmc then ipmc*/

  prog_selection_cam_tbl.program                      = *prog_id;
  prog_selection_cam_tbl.valid                        = 1;
  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_global_ipv4compmc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_key_const_global_ipv4compmc_with_rpf(
     int    unit,
     int32 prog_id
   )
{
  uint32
    res, vrf_ce_inst = 0, rif_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_key_construction_tbl.instruction_2_16b = 0;
  flp_key_construction_tbl.instruction_3_32b = ARAD_PP_CE_SIP_FWD_HEADER;
  flp_key_construction_tbl.instruction_4_32b = 0;
  flp_key_construction_tbl.instruction_5_32b = ARAD_PP_CE_DIP_FWD_HEADER_27_0;

  flp_key_construction_tbl.instruction_0_16b = rif_ce_inst;
  flp_key_construction_tbl.instruction_1_16b = vrf_ce_inst;
  flp_key_construction_tbl.instruction_4_32b = ARAD_PP_CE_DIP_FWD_HEADER;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0xA  /*hex 6'b001010*/; /*  VRF,SIP */
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x29 /*hex 6'b101001*/; /*  RIF,SIP,DIP[27:0] */
  
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  
  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  flp_key_construction_tbl.instruction_0_16b = 0;
  flp_key_construction_tbl.instruction_1_16b = 0;
  flp_key_construction_tbl.instruction_2_16b = 0;
  flp_key_construction_tbl.instruction_3_32b = 0;
  flp_key_construction_tbl.instruction_4_32b = 0;
  flp_key_construction_tbl.instruction_5_32b = 0;
  flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_global_ipv4compmc_with_rpf", 0, 0);
}


uint32
   arad_pp_flp_lookups_global_ipv4compmc_with_rpf(
     int    unit,
     int prog_id,
     uint32 tcam_access_profile_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;
  uint32 prefix = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res =  arad_pp_lem_access_app_to_prefix_get(unit, ARAD_PP_FLP_GLOBAL_IPV4_KEY_OR_MASK,&prefix);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
   
  /* LEM2: {RIF,SIP,DIP} - key-B */
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = 1;
  /* Because key size is 72 bits:RIF,SIP,DIP[27:0], have to take 2 bits from prefix
   * so the prefix value will be 11xx. */
  flp_lookups_tbl.lem_2nd_lkp_and_value  = ((~prefix) & 0xF); 
  flp_lookups_tbl.lem_2nd_lkp_or_value   = prefix;

  /* TCAM: {RIF,SIP,DIP} - key-C configured by DBAL */

  /* LPM1/LEM1(RPF): {VRF,SIP} - key-A */
  flp_lookups_tbl.lem_1st_lkp_valid     = 1;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK;
  flp_lookups_tbl.lpm_1st_lkp_valid     = 1;
  flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
  flp_lookups_tbl.lpm_1st_lkp_and_value = 3;
  flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
  flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_global_ipv4compmc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_process_global_ipv4compmc_with_rpf(
     int    unit,
     int prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  flp_process_tbl.include_tcam_in_result_a       = 1;
  flp_process_tbl.include_lem_2nd_in_result_a     = 1;

  flp_process_tbl.include_lem_1st_in_result_b     = 1;
  flp_process_tbl.include_lpm_1st_in_result_b     = 1;
  flp_process_tbl.lpm_1st_lkp_enable_default     = 1;
  flp_process_tbl.lpm_public_1st_lkp_enable_default     = 1;

  flp_process_tbl.result_a_format             = 0;
  flp_process_tbl.result_b_format             = 0;
  flp_process_tbl.compatible_mc_bridge_fallback = 1;
  flp_process_tbl.procedure_ethernet_default   = 3;
  flp_process_tbl.enable_hair_pin_filter        = 1;
  flp_process_tbl.enable_rpf_check             = 1;
  flp_process_tbl.not_found_trap_strength       = 0; /*  if not found dont trap */
    /* take VRF default destination */
  flp_process_tbl.select_default_result_a = 1;
  flp_process_tbl.select_default_result_b = 1;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  tmp = 1;
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_global_ipv4compmc_with_rpf", 0, 0);
}

uint32
   arad_pp_flp_static_programs_init(
     int unit)
{
  uint32 res;
  uint8  mac_in_mac_enabled=0;
#if defined(INCLUDE_L3)
  uint8  is_routing_enabled=0;
#endif
  uint64 hw_prog_bitmap;
  uint64 prog_sel_bitmap;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  COMPILER_64_ZERO(hw_prog_bitmap);
  COMPILER_64_ZERO(prog_sel_bitmap);
  
  /* set HW prog id bitmap for static programs */
  /* ***************************************** */

  /* TM */
  if (SOC_DPP_PP_ENABLE(unit)) {
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_TM);
  }

  /* ethernet_tk_epon_uni_v4 */
  if (SOC_DPP_L3_SRC_BIND_IPV4_ENABLE(unit)) {
      if (!SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP);
      }
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC);
  }

  /* ethernet_tk_epon_uni_v6 */
  if (SOC_DPP_L3_SRC_BIND_IPV6_ENABLE(unit)) {
      if (!SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP);
      }
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC);
  }

  /* ethernet_mac_in_mac */
  res = arad_pp_is_mac_in_mac_enabled(unit,&mac_in_mac_enabled);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  if (mac_in_mac_enabled) {
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_ETHERNET_MAC_IN_MAC);
  }

  /* P2P */
  ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_P2P);

  /* vpws_tagged */
  if (!(SOC_DPP_CONFIG(unit)->pp.pon_application_enable)) {
      /* VPWS tagged mode */
      if (soc_property_get(unit, spn_VPWS_TAGGED_MODE, 0)) {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_VPWS_TAGGED_SINGLE_TAG);
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_VPWS_TAGGED_DOUBLE_TAG);
      }

      /* Ethernet_ing_learn */
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_ETHERNET_ING_LEARN);
  }

#if defined(INCLUDE_L3)
  
  res = arad_pp_sw_db_ipv4_is_routing_enabled(unit, &is_routing_enabled);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
  if (is_routing_enabled) {

      /* IPV4 UC PUBLIC */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (!ARAD_KBP_ENABLE_IPV4_UC)
#endif
      {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV4UC_PUBLIC);
      }

      /* IPV4 UC */
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV4UC);

      /* IPV4 UC RPF */
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV4UC_RPF);

      /* IPV4 UC DC */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (SOC_IS_JERICHO(unit) && ARAD_KBP_ENABLE_IPV4_DC) {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV4_DC);
      }
#endif

      /* ipv4mc_bridge */
      if (!SOC_DPP_CONFIG(unit)->pp.pon_application_enable){
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV4MC_BRIDGE);
      }

      /* ipv4mc + RPF */
      if (SOC_DPP_CONFIG(unit)->l3.ipmc_vpn_lookup_enable) {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV4COMPMC_WITH_RPF);
      }

      /* IPV6 UC */
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV6UC);

      /* IPV6 MC */
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_IPV6MC);

      /* LSR */
      ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_LSR);

      /* TRILL */
      if (SOC_DPP_CONFIG(unit)->trill.mode)
      {
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_TRILL_UC);
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_TRILL_MC_ONE_TAG);
          ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_TRILL_MC_TWO_TAGS);
          if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
              ARAD_PP_FLP_COMPILER_64_BITSET_WITH_VALIDATION(hw_prog_bitmap, PROG_FLP_TRILL_AFTER_TERMINATION);
          }
      }
  }
#endif /* INCLUDE_L3*/

  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_hw_prog_id_bitmap.set(unit,hw_prog_bitmap);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_prog_select_id_bitmap.set(unit,prog_sel_bitmap);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_init", 0, 0);
}

uint32
   arad_pp_flp_prog_sel_cam_init(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  uint32
    addr;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  ARAD_CLEAR(&prog_selection_cam_tbl, ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA, 1);
  
  prog_selection_cam_tbl.valid = 0;
  prog_selection_cam_tbl.parser_leaf_context_mask         = 0xF;
  prog_selection_cam_tbl.port_profile_mask                = ARAD_PP_FLP_PORT_PROFILE_MASK_CONSIDER_NONE;
  prog_selection_cam_tbl.ptc_profile_mask                 = 0x3;
  prog_selection_cam_tbl.packet_format_code_mask          = 0x3F;
  prog_selection_cam_tbl.forwarding_header_qualifier_mask = 0x7FF;
  prog_selection_cam_tbl.forwarding_code_mask            = 0xF;
  prog_selection_cam_tbl.forwarding_offset_index_mask     = SOC_IS_ARAD_B1_AND_BELOW(unit)? 0x3 : 0x7;
  prog_selection_cam_tbl.l_3_vpn_default_routing_mask       = 0x1;
  prog_selection_cam_tbl.trill_mc_mask                   = 0x1;
  prog_selection_cam_tbl.packet_is_compatible_mc_mask      = 0x1;
  prog_selection_cam_tbl.in_rif_uc_rpf_enable_mask          = 0x1;
  prog_selection_cam_tbl.ll_is_arp_mask                   = 0x1;
  prog_selection_cam_tbl.elk_status_mask                 = 0x1;
  prog_selection_cam_tbl.cos_profile_mask                = 0x3F;
  prog_selection_cam_tbl.service_type_mask               = 0x7;
  prog_selection_cam_tbl.vt_processing_profile_mask       = 0x7;
  prog_selection_cam_tbl.vt_lookup_0_found_mask            = 0x1;
  prog_selection_cam_tbl.vt_lookup_1_found_mask            = 0x1;
  prog_selection_cam_tbl.tt_processing_profile_mask       = 0x7;
  prog_selection_cam_tbl.tt_lookup_0_found_mask            = 0x1;
  prog_selection_cam_tbl.tt_lookup_1_found_mask           = 0x1;  

  if (SOC_IS_JERICHO(unit)) {
      prog_selection_cam_tbl.forwarding_offset_index_ext_mask           = 0x3;  
      prog_selection_cam_tbl.cpu_trap_code_mask                         = 0xFF;  
      prog_selection_cam_tbl.in_lif_profile_mask                        = 0xF;  
      prog_selection_cam_tbl.llvp_incoming_tag_structure_mask           = 0xF;  
      prog_selection_cam_tbl.forwarding_plus_1_header_qualifier_mask    = 0x7FF;  
      prog_selection_cam_tbl.tunnel_termination_code_mask               = 0xF;  
      prog_selection_cam_tbl.qualifier_0_mask                           = 0x7;  
      prog_selection_cam_tbl.in_lif_data_index_mask                     = 0x3;  
      prog_selection_cam_tbl.in_lif_data_msb_mask                       = 0x1F;
      prog_selection_cam_tbl.custom_rif_profile_bit_mask                = 0x1;
  }

  for(addr = 0; addr < SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines); ++addr)
  {
    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, addr, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_sel_cam_init", 0, 0);
}

uint32
   arad_pp_flp_lookups_init(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;
  uint32
    addr;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  ARAD_CLEAR(&flp_lookups_tbl, ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA, 1);
    
  flp_lookups_tbl.elk_lkp_valid      = 0;
  flp_lookups_tbl.lem_1st_lkp_valid  = 0;
  flp_lookups_tbl.lem_2nd_lkp_valid  = 0;
  flp_lookups_tbl.lpm_1st_lkp_valid  = 0;
  flp_lookups_tbl.lpm_2nd_lkp_valid  = 0;
  flp_lookups_tbl.enable_tcam_identification_ieee_1588 = 0;
  flp_lookups_tbl.enable_tcam_identification_oam      = 0;
  flp_lookups_tbl.tcam_lkp_db_profile = 0x3F;
  flp_lookups_tbl.tcam_traps_lkp_db_profile_0 = 0x3F;/* no tcam access for OAM by default */
  flp_lookups_tbl.tcam_traps_lkp_db_profile_1 = 0x3F;

  if (SOC_IS_JERICHO(unit)) {
      flp_lookups_tbl.tcam_lkp_db_profile_1 = 0x3F;
  }

  for(addr = 0; addr < 0x18; ++addr)
  {
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, addr, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_init", 0, 0);
}



/* 
   function:arad_pp_flp_process_elk_result_configure
 
   ELK received data is 128 bit.
   bits 120-127: hit bit indication for each result.
   all other 120 bits are for the results.
   this function configures the FLP program to read the ELK result..                  
                                                                     
   FWD:                                                            
                                                                             
   fwd_res_size             -  fwd size is always 48 bits (any other value returns an error)
   fwd_res_data_start       -  first bit of the fwd result legal values: 72, 64, 56, 48, 40, 32, 24, 16, 8, 0 - at the moment always 120 - fwd_res_size
   fwd_res_found_bit_offset -  which of the hit bits indicates the fwd res hit bit legal values: 120 - 127 (default configuration 127)
                                                                         
   FEC:
 
   fec_res_size             -  indicates if fec exists (0 not exists)
   fec_res_data_start       -  first bit of the fwd result legal values: 56, 48, 40, 32, 24, 16, 8, 0
   fec_res_found_bit_offset -  which of the hit bits indicates the fwd res hit bit legal values: 120 - 127 (default configuration arad 126)
 
   */

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_elk_result_configure(int unit, int prog_id, int fwd_res_size, int fwd_res_found_bit_offset,
                                                           int fec_res_size, int fec_res_data_start, int fec_res_found_bit_offset )
{
  uint32 res, mem_val=0;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA flp_process_tbl;

  int fwd_res_data_start = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS - fwd_res_size;

  uint32 fld_val_fwd_res_found_offset = 0, fld_val_fwd_res_data_offset = 0, fld_val_fwd_data_type = 0;
  uint32 fld_val_fec_res_found_offset = 0, fld_val_fec_res_data_offset = 0, fld_val_fec_data_type = 0;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_CLEAR(&flp_process_tbl, ARAD_PP_IHB_FLP_PROCESS_TBL_DATA, 1);

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* FWD configurations */

  if ((fwd_res_size != ARAD_PP_FLP_DEFAULT_FWD_RES_SIZE) && (fwd_res_size != 0)) {
      LOG_ERROR(BSL_LS_SOC_TCAM,(BSL_META_U(unit,"Error in %s(): fwd_res_size not valid size %d\n"), 
                       FUNCTION_NAME(), fwd_res_size));
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
  }

  fld_val_fwd_data_type = 0x7;
  /* 0:sys_port, 1:flow, 2:FEC, 3:MC, 4:action, 7:mact format*/

  if (prog_id == PROG_FLP_IPV4_DC && ARAD_KBP_IPV4DC_24BIT_FWD) {
      /* double capacity 24bit FWD both lookups are FEC lookups */
      fld_val_fwd_data_type = 0x2;
      /* set result size and recalculate data start */
      fwd_res_size = ARAD_KBP_IPV4DC_24BIT_FWD_RES_SIZE;
      fwd_res_data_start = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS - fwd_res_size;
  }

  if (fwd_res_found_bit_offset > ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS+7 || fwd_res_found_bit_offset < ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS) {
      /* ERROR illegal value */
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 40, exit);
  }

  fld_val_fwd_res_found_offset = fwd_res_found_bit_offset - ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS;

  if (fwd_res_data_start%8 > 0) {
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 50, exit);
  }else{
      fld_val_fwd_res_data_offset = fwd_res_data_start/8;
      if(prog_id == PROG_FLP_IPV4_DC && ARAD_KBP_IPV4DC_24BIT_FWD) {
          /* for example fld_val_fwd_res_found_offset = 0xC to use bits 96::119*/
          if (fld_val_fwd_res_data_offset != 12) {
              SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 60, exit);
          }
      } else {
          /* for example fld_val_fwd_res_found_offset = 0x9 to use bits 72::119*/
          if (fld_val_fwd_res_data_offset != 9) {
              SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 60, exit);
          }
      }
  }

  soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_FOUND_OFFSETf, &fld_val_fwd_res_found_offset);
  soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_DATA_OFFSETf, &fld_val_fwd_res_data_offset);
  soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_DATA_FORMATf, &fld_val_fwd_data_type);

/* FEC configurations */

  if (fec_res_size == 0) {
      goto exit;
  }
  /* fec_size configurations */

  if (fec_res_found_bit_offset > ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS+7 || fec_res_found_bit_offset < ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS) {
      /* ERROR illegal value */
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 70, exit);
  }

  fld_val_fec_data_type = 0x2;
  /* 0:sys_port, 1:flow, 2:FEC, 3:MC, 4:action, 5:SA-drop, 6:?, 7:mact format*/

  if (prog_id == PROG_FLP_IPV4_DC && !ARAD_KBP_IPV4DC_24BIT_FWD) {
      /* double capacity both lookups are FWD lookups */
      fld_val_fec_data_type = 0x7;
  }  

  fld_val_fec_res_found_offset = fec_res_found_bit_offset - ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS;

  if (fec_res_data_start%8 > 0) {
      SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 80, exit);
  }else{
      fld_val_fec_res_data_offset = fec_res_data_start/8;
      /* for example fld_val_fec_res_found_offset = 0x7 to use bits 56::103*/
      if (fld_val_fec_res_data_offset > 7) {
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
      }
  }

  soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val_fec_res_found_offset);
  soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val_fec_res_data_offset);
  soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_FORMATf, &fld_val_fec_data_type);

  res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, prog_id, &mem_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_elk_result_configure", prog_id, 0);
}
#endif /*INCLUDE_KBP*/

uint32
   arad_pp_flp_process_init(
     int unit
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  uint32
    addr;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_CLEAR(&flp_process_tbl, ARAD_PP_IHB_FLP_PROCESS_TBL_DATA, 1);

  flp_process_tbl.include_lem_2nd_in_result_a     = 0;
  flp_process_tbl.include_tcam_in_result_a       = 0;
  flp_process_tbl.include_lpm_2nd_in_result_a     = 0;
  flp_process_tbl.include_lem_1st_in_result_a     = 0;
  flp_process_tbl.include_lpm_1st_in_result_a     = 0;
  flp_process_tbl.select_default_result_a       = 3;
  flp_process_tbl.include_elk_ext_in_result_b     = 0;
  flp_process_tbl.include_tcam_in_result_b       = 0;
  flp_process_tbl.include_lpm_1st_in_result_b     = 0;
  flp_process_tbl.include_lem_2nd_in_result_b     = 0;
  flp_process_tbl.include_lpm_2nd_in_result_b     = 0;
  flp_process_tbl.select_default_result_b       = 3;
  flp_process_tbl.apply_fwd_result_a            = 1;
  flp_process_tbl.not_found_snoop_strength      = 0;
  flp_process_tbl.not_found_trap_strength       = 7;
  flp_process_tbl.not_found_trap_code           = 0;
  flp_process_tbl.result_b_format             = 0;
  flp_process_tbl.result_a_format             = 0;
  flp_process_tbl.include_lem_1st_in_result_b     = 0;
  flp_process_tbl.include_elk_fwd_in_result_b     = 0;
  flp_process_tbl.include_elk_ext_in_result_a     = 0;
  flp_process_tbl.include_elk_fwd_in_result_a     = 0;
  flp_process_tbl.elk_result_format            = 0;
  flp_process_tbl.sa_lkp_result_select          = 0;
  flp_process_tbl.sa_lkp_process_enable         = 1;
  flp_process_tbl.procedure_ethernet_default   = 0;
  flp_process_tbl.unknown_address             = 0;
  flp_process_tbl.enable_hair_pin_filter        = 0;
  flp_process_tbl.enable_rpf_check             = 0;
  flp_process_tbl.compatible_mc_bridge_fallback = 0;
  flp_process_tbl.enable_lsr_p2p_service        = 0;
  flp_process_tbl.learn_enable                = 0;
  flp_process_tbl.fwd_processing_profile       = 0;    
  for(addr = 0; addr < SOC_DPP_DEFS_GET(unit, nof_flp_programs); ++addr)
  {
    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, addr, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
  }

#ifdef BCM_88660_A0
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if (SOC_IS_ARADPLUS(unit)) { 
        uint32
            fld_val,
            mem_val=0;

        /* In Arad+, if KBP is supported, initialized how the results are parsed */
        fld_val = 0x7; /* bit 7 is lookup-found-0 */
        soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_FOUND_OFFSETf, &fld_val);
        fld_val = 0x9; /* bits 119:72 is lookup-result-0 */
        soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_DATA_OFFSETf, &fld_val);
        fld_val = 0x7; /* MACT result format in lookup-result-0 */
        soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_DATA_FORMATf, &fld_val);
        if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_rpf_fwd_parallel", 0)) {
            /* KBP compare3 is disabled - RPF result is located in ELK result 1*/
			fld_val = 0x6; /* bit 6 is lookup-found-1 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val);
            fld_val = 0x7; /* bits 103:56 is lookup-result-1 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val);
        }
        else {
            /* KBP compare3 is enabled - RPF result is located in ELK result 2*/
            fld_val = 0x5; /* bit 5 is lookup-found-2 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val);
            fld_val = 0x3; /* bits 71:24 is lookup-result-2 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val);
        }
        fld_val = 0x2; /* FEC format in lookup-result-1 */
        soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_FORMATf, &fld_val);

        for(addr = 0; addr < SOC_DPP_DEFS_GET(unit, nof_flp_programs); ++addr)
        {
            res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, addr, &mem_val);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101 + addr, exit);
        }

		if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
			/* for ipv4 mc program we always work in comp1 (lookup in different tables) */
			fld_val = 0x6; /* bit 6 is lookup-found-1 */
			soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val);
			fld_val = 0x7; /* bits 103:56 is lookup-result-1 */
			soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val);
			res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, PROG_FLP_IPV4COMPMC_WITH_RPF, &mem_val);
			SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101 + addr, exit);
		}

        if(ARAD_KBP_ENABLE_IPV6_EXTENDED) {
            /* In Arad+, if KBP is supported, initialized how the results are parsed */
            fld_val = 0x7; /* bit 7 is lookup-found-0 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_FOUND_OFFSETf, &fld_val);
            fld_val = 0x9; /* bits 119:72 is lookup-result-0 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_DATA_OFFSETf, &fld_val);
            fld_val = 0x7; /* MACT result format in lookup-result-0 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, FWD_DATA_FORMATf, &fld_val);

            fld_val = 0x6; /* bit 5 is lookup-found-1 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val);
            fld_val = 0x0; /* bits 47:00 is lookup-result-4 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val);

            fld_val = 0x2; /* FEC format in lookup-result-1 */
            soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_FORMATf, &fld_val);

            res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, PROG_FLP_IPV6UC, &mem_val);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101 + addr, exit);
		}

		if(ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED)
		{                
			/* configure where to take rpf 16 bits from */
			fld_val = 0x2; /* bits 63:16 is lookup-result-1 */
			soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_DATA_OFFSETf, &fld_val);
			fld_val = 0x6; /* bit 6 is lookup-found-1 */
			soc_mem_field_set(unit, IHP_ELK_PAYLOAD_FORMATm, &mem_val, EXT_FOUND_OFFSETf, &fld_val);

			res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, PROG_FLP_LSR, &mem_val);
			SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101 + addr, exit);

			res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, PROG_FLP_IPV4UC_RPF, &mem_val);
			SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101 + addr, exit);

			res = WRITE_IHP_ELK_PAYLOAD_FORMATm(unit, MEM_BLOCK_ANY, PROG_FLP_IPV4UC, &mem_val);
			SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 101 + addr, exit);
		}
    }

#endif /*INCLUDE_KBP*/
#endif /* BCM_88660_A0 */

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_init", 0, 0);
}
uint32
   arad_pp_flp_lookups_initial_programs(
     int unit,
     uint8 ingress_learn_enable,
     uint8 ingress_learn_oppurtunistic
   )
{
    uint32 res=0;
    uint8
        mac_in_mac_enabled,
        slb_enabled = 0,
        sa_auth_enabled = 0;

#if defined(INCLUDE_L3)
    uint8 is_routing_enabled;
    int32 ipv6mc_cam_sel_id;
#endif /* INCLUDE_L3 */

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  if (SOC_IS_ARADPLUS(unit) && soc_property_get(unit, spn_RESILIENT_HASH_ENABLE, 0)) {
    slb_enabled = 1;
  }

  if (soc_property_get(unit, spn_SA_AUTH_ENABLED, 0)) {
    sa_auth_enabled = 1;
  }

  /* if mac in mac is enabled, set ethernet_mac_in_mac program and ethernet_pbp (inside ethernet_ing_learn) */  
  res = arad_pp_is_mac_in_mac_enabled(
      unit,
      &mac_in_mac_enabled
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

#ifdef BCM_88660_A0
  /* SLB uses the first lookup, and therefore cannot be used in combination with any other programs that use the 1st LEM lookup. */
  if (SOC_IS_ARADPLUS(unit) && soc_property_get(unit, spn_RESILIENT_HASH_ENABLE, 0) && 
      (
       (mac_in_mac_enabled) ||
       (SOC_DPP_CONFIG(unit)->pp.fcoe_enable) ||
       (SOC_DPP_CONFIG(unit)->l3.nof_rps != 0) ||
       (SOC_DPP_CONFIG(unit)->pp.pon_application_enable) ||
       (SOC_DPP_CONFIG(unit)->trill.mode)
      )) {
            const char *program = NULL;
            if (mac_in_mac_enabled) {
              program = "mac-in-mac";
            } else if (SOC_DPP_CONFIG(unit)->pp.fcoe_enable) {
              program = "FCoE";
            } else if (SOC_DPP_CONFIG(unit)->l3.nof_rps != 0) {
              program = "bidirectional";
            } else if (SOC_DPP_CONFIG(unit)->pp.pon_application_enable) {
              program = "PON";
            } else if (SOC_DPP_CONFIG(unit)->trill.mode) {
              program = "TRILL";
            } else {
              SOC_SAND_VERIFY(FALSE);
            }

            SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_MSG_STR("%s cannot be used with SLB.\n"), program));
    
  }
#endif /* BCM_88660_A0 */

  res = arad_pp_flp_prog_sel_cam_key_program_tm(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  res = arad_pp_flp_process_key_program_tm(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);


  res = arad_pp_flp_process_tm(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

  /* Exit here in case of TM - Enable the Management unit since the MACT is not enabled */
    if (SOC_DPP_CONFIG(unit)->pp.pon_application_enable) {
        if (SOC_DPP_L3_SRC_BIND_IPV4_ENABLE(unit)) {
            /* Spoof v4 */
            res = arad_pp_flp_prog_sel_cam_ethernet_tk_epon_uni_v4(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
            res = arad_pp_flp_key_const_ethernet_tk_epon_uni_v4_dhcp(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
            res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_dhcp(unit, sa_auth_enabled, slb_enabled);
            SOC_SAND_CHECK_FUNC_RESULT(res, 36, exit);

            if (SOC_IS_JERICHO(unit) && SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit)) {
                res = arad_pp_flp_dbal_pon_ipv4_sav_static_program_tables_init(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 37, exit);
            } else {
                res = arad_pp_flp_key_const_ethernet_tk_epon_uni_v4_static(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

                res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_static(unit, sa_auth_enabled, slb_enabled);
                SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
            }
            res = arad_pp_flp_process_ethernet_tk_epon_uni_v4(unit, TRUE,sa_auth_enabled, slb_enabled);
            SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
        }

        if (SOC_DPP_L3_SRC_BIND_IPV6_ENABLE(unit)) {
            /* Spoof v6 */
            res = arad_pp_flp_prog_sel_cam_ethernet_tk_epon_uni_v6(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
            if (SOC_IS_JERICHO(unit)){
                res = arad_pp_flp_dbal_pon_ipv6_sav_static_program_tables_init(unit,sa_auth_enabled, slb_enabled,ARAD_TCAM_ACCESS_PROFILE_INVALID);
                SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);
            }
            res = arad_pp_flp_key_const_ethernet_tk_epon_uni_v6(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
            res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v6(unit, ARAD_TCAM_ACCESS_PROFILE_INVALID, sa_auth_enabled, slb_enabled);
            SOC_SAND_CHECK_FUNC_RESULT(res, 65, exit);
            res = arad_pp_flp_process_ethernet_tk_epon_uni_v6(unit, TRUE,sa_auth_enabled, slb_enabled);
            SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
        }

        if (SOC_DPP_L3_SRC_BIND_ARP_RELAY_ENABLE(unit)) {
            res = arad_pp_flp_pon_arp_prog_init(unit, sa_auth_enabled, slb_enabled);
            SOC_SAND_CHECK_FUNC_RESULT(res, 75, exit);
        }
    }

  if (mac_in_mac_enabled) {
  
      res = arad_pp_flp_prog_sel_cam_ethernet_mac_in_mac(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
      /* key construction is not needed because lookup key selection is configured to use {FID,DA} as key */
      res = arad_pp_flp_lookups_ethernet_mac_in_mac(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 85, exit);
      res = arad_pp_flp_process_ethernet_mac_in_mac(unit); 
      SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
  }   

  res = arad_pp_flp_prog_sel_cam_p2p(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 95, exit);
  res = arad_pp_flp_key_const_p2p(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
  res = arad_pp_flp_lookups_p2p(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 105, exit);
  res = arad_pp_flp_process_p2p(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

  if (!(SOC_DPP_CONFIG(unit)->pp.pon_application_enable)) {
      /* VPWS tagged mode */
      if (soc_property_get(unit, spn_VPWS_TAGGED_MODE, 0)) {
          res = arad_pp_flp_prog_sel_cam_vpws_tagged(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 115, exit);
          res = arad_pp_flp_dbal_vpws_tagged_program_tables_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
          res = arad_pp_flp_process_vpws_tagged(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 125, exit);
      } 		  
  }

#if defined(INCLUDE_L3)
  
  res = arad_pp_sw_db_ipv4_is_routing_enabled(unit, &is_routing_enabled);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 130, exit);
  if(is_routing_enabled)
  {      
      /* -----------------------------------------*/
      /* ----------- custom ip route program -----*/
      /* -----------------------------------------*/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (!ARAD_KBP_ENABLE_IPV4_UC) {
#else
      {
#endif

    #if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
          if(JER_KAPS_ENABLE(unit) && SOC_DPP_CONFIG(unit)->pp.custom_ip_route){
              int prog_id = 0;
              res = arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn_custom_prgrm(unit,&prog_id); /* this will update prog_id*/
              SOC_SAND_CHECK_FUNC_RESULT(res, 135, exit);
              res = arad_pp_flp_dbal_ipv4uc_l3vpn_program_tables_init(unit,prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);
              res = arad_pp_flp_process_ipv4uc_l3vpn(unit,prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 145, exit);
          }
    #endif
      }

      /* -----------------------------------------*/
      /* ----------- IPv4 UC + L3VPN -------------*/
      /* -----------------------------------------*/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (!ARAD_KBP_ENABLE_IPV4_UC)
#endif
      {
          res = arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 165, exit);
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
          if(JER_KAPS_ENABLE(unit)){
              res = arad_pp_flp_dbal_ipv4uc_l3vpn_program_tables_init(unit,0);
              SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);
          }
          else
#endif
          {
              res = arad_pp_flp_key_const_ipv4uc_l3vpn(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 175, exit);
              res = arad_pp_flp_lookups_ipv4uc_l3vpn(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);
          }
          res = arad_pp_flp_process_ipv4uc_l3vpn(unit,0);
          SOC_SAND_CHECK_FUNC_RESULT(res, 185, exit);
      }

      /* ---------------------------------------*/
      /* ----------- IPv4 UC + RPF -------------*/
      /* ---------------------------------------*/
      res = arad_pp_flp_prog_sel_cam_ipv4uc_with_rpf(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 205, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if((JER_KAPS_ENABLE(unit)) || (ARAD_KBP_ENABLE_IPV4_RPF && SOC_IS_JERICHO(unit))) {
          res = arad_pp_flp_dbal_ipv4uc_rpf_program_tables_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 210, exit);
      }
      else 
#endif
      {
          res = arad_pp_flp_key_const_ipv4uc_rpf(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 215, exit);
          res = arad_pp_flp_lookups_ipv4uc_rpf(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit);
      }
      res = arad_pp_flp_process_ipv4uc_rpf(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 225, exit);

      /* ---------------------------------*/
      /* ----------- IPv4 UC -------------*/
      /* ---------------------------------*/
      res = arad_pp_flp_prog_sel_cam_ipv4uc(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 245, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(JER_KAPS_ENABLE(unit) || (ARAD_KBP_ENABLE_IPV4_UC && SOC_IS_JERICHO(unit))){
          res = arad_pp_flp_dbal_ipv4uc_program_tables_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
      }
      else 
#endif 
      {
          res = arad_pp_flp_key_const_ipv4uc(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 255, exit);
          res = arad_pp_flp_lookups_ipv4uc(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 260, exit);
      }      
      res = arad_pp_flp_process_ipv4uc(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 265, exit);

      /* ---------------------------------*/
      /* ---- IPv4 UC Double Capacity ----*/
      /* ---------------------------------*/
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (SOC_IS_JERICHO(unit) && ARAD_KBP_ENABLE_IPV4_DC) {
          res = arad_pp_flp_prog_sel_cam_ipv4_dc(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 270, exit);
          res = arad_pp_flp_dbal_ipv4_dc_program_tables_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 275, exit);
          res = arad_pp_flp_process_ipv4_dc(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 280, exit);
      }
#endif

      /* Load IVL Programs & Cam Selection */
      if ((SOC_DPP_CONFIG(unit)->pp.ivl_inlif_profile == 1)) {
          /* Load IVL L2 Inner Most V-TAG
             MUST be first, higher priority than rest of IVL L2 */
          int32 ivl_prog_id;
          /* Set Program Selection for L2 UC IVL using Inner-most VID */
          res = arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_inner_learn(unit, &ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 285, exit);
          /* Set Program Selection for L2 MC IVL using Inner-most VID - Must be called after UC */
          res = arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_inner_learn(unit, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 290, exit);
          /* Set database creation for L2 UC and MC IVL Inner-most VID KEY */
          res = arad_pp_dbal_flp_ethernet_ing_ivl_inner_learn_tables_create(unit, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 295, exit);
          /* Define FLP program process for all L2 UC and MC IVL databases */
          res = arad_pp_flp_process_ethernet_ing_all_ivl_learn(unit, TRUE, sa_auth_enabled, slb_enabled, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 300, exit);

          /* Load IVL L2 FWD OUTER V-TAG
             MUST be second, higher priority than untagged IVL L2 */
          ivl_prog_id = 0;
          /* Set Program Selection for L2 UC IVL using Inner-most VID */
          res = arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_fwd_outer_learn(unit, &ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 305, exit);
          /* Set Program Selection for L2 MC IVL using Inner-most VID - Must be called after UC */
          
          res = arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_fwd_outer_learn(unit, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 310, exit);
          /* Set database creation for L2 UC and MC IVL Inner-most VID KEY */
          res = arad_pp_dbal_flp_ethernet_ing_ivl_fwd_outer_learn_tables_create(unit, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 315, exit);
          /* Define FLP program process for all L2 UC and MC IVL databases */
          res = arad_pp_flp_process_ethernet_ing_all_ivl_learn(unit, TRUE, sa_auth_enabled, slb_enabled, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 320, exit);

          /* Load Untagged IVL L2 */
          ivl_prog_id = 0;
          /* Set Program Selection for L2 UC IVL using IVEC VID */
          res = arad_pp_flp_prog_sel_cam_ethernet_ing_ivl_learn(unit, &ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 325, exit);
          /* Set Program Selection for L2 MC IVL using IVEC VID - Must be called after UC */
          res = arad_pp_flp_prog_sel_cam_ethernet_mc_ing_ivl_learn(unit, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 330, exit);
          /* Set database creation for L2 UC and MC IVL IVEC VID KEY */
          res = arad_pp_dbal_flp_ethernet_ing_ivl_learn_tables_create(unit, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res,335, exit);

          /* Define FLP program process for all L2 UC and MC IVL databases */
          res = arad_pp_flp_process_ethernet_ing_all_ivl_learn(unit, TRUE, sa_auth_enabled, slb_enabled, ivl_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 340, exit);
      }


    if (SOC_IS_ARADPLUS_A0(unit) && SOC_DPP_CONFIG(unit)->pp.oam_statistics) {
          int oam_statistics_prog_id;
          int bfd_statistics_prog_id;
          int oam_down_statistics_prog_id;
          int oam_single_tag_statistics_prog_id;
          int oam_double_tag_statistics_prog_id;
          int bfd_mpls_statistics_prog_id;
          int bfd_pwe_statistics_prog_id;

          /* OAM up mep*/
          res = arad_pp_flp_prog_sel_cam_oam_statistics(unit, &oam_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 345, exit);
          res = arad_pp_flp_dbal_oam_statistics_program_tables_init(unit, oam_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);
          res = arad_pp_flp_process_oam_statistics(unit, oam_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 355, exit);

          /* BFD o IPV4*/
          res = arad_pp_flp_prog_sel_cam_bfd_statistics(unit, &bfd_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 360, exit);
          res = arad_pp_flp_dbal_bfd_statistics_program_tables_init(unit, bfd_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 365, exit);
          res = arad_pp_flp_process_bfd_statistics(unit, bfd_statistics_prog_id); 
          SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

          /* OAM down mep untagged*/
          res = arad_pp_flp_prog_sel_cam_oam_down_untagged_statistics(unit, &oam_down_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 375, exit);
          res = arad_pp_flp_dbal_oam_down_untagged_statistics_program_tables_init(unit, oam_down_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 380, exit);
          res = arad_pp_flp_process_oam_statistics(unit, oam_down_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 385, exit);

          /* OAM down mep single tag*/
          res = arad_pp_flp_prog_sel_cam_oam_single_tag_statistics(unit, &oam_single_tag_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 390, exit);
          res = arad_pp_flp_dbal_oam_single_tag_statistics_program_tables_init(unit, oam_single_tag_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 395, exit);
          res = arad_pp_flp_process_oam_statistics(unit, oam_single_tag_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 400, exit);

          /* OAM down mep double tag*/
          res = arad_pp_flp_prog_sel_cam_oam_double_tag_statistics(unit, &oam_double_tag_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 405, exit);
          res = arad_pp_flp_dbal_oam_double_tag_statistics_program_tables_init(unit, oam_double_tag_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 410, exit);
          res = arad_pp_flp_process_oam_statistics(unit, oam_double_tag_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 415, exit);

          /* BFD o MPLS*/
          res = arad_pp_flp_prog_sel_cam_bfd_mpls_statistics(unit, &bfd_mpls_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 420, exit);
          res = arad_pp_flp_dbal_bfd_mpls_statistics_program_tables_init(unit, bfd_mpls_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 425, exit);
          res = arad_pp_flp_process_oam_statistics(unit, bfd_mpls_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 430, exit);
          
          /* BFD o PWE*/
          res = arad_pp_flp_prog_sel_cam_bfd_pwe_statistics(unit, &bfd_pwe_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 435, exit);
          res = arad_pp_flp_dbal_bfd_pwe_statistics_program_tables_init(unit, bfd_pwe_statistics_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 440, exit);
          res = arad_pp_flp_process_oam_statistics(unit, bfd_pwe_statistics_prog_id); 
          SOC_SAND_CHECK_FUNC_RESULT(res, 445, exit);
      }


      /* IPV4MC program selection conflict with PON program selection for IPv4 MC packet.
       * Forwarding_code is used to identify L2 and L3 lookup, but In IPV4MC program selection the forwarding_code is ETHERNET.
       */
      if (!SOC_DPP_CONFIG(unit)->pp.pon_application_enable) {

          res = arad_pp_flp_prog_sel_cam_ipv4mc_bridge_v4mc(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 450, exit);

          if(SOC_DPP_CONFIG(unit)->pp.ipmc_l3mcastl2_mode == 1){
              res = arad_pp_flp_prog_sel_cam_ipv4mc_bridge(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 455, exit);
          }
		
          if(SOC_IS_ARADPLUS(unit)) {
              res = arad_pp_flp_dbal_ipv4mc_bridge_program_tables_init(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 460, exit);
          } else {
              res = arad_pp_flp_key_const_ipv4mc_bridge(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 465, exit);
              res = arad_pp_flp_lookups_ipv4mc_bridge(unit, sa_auth_enabled, slb_enabled);
              SOC_SAND_CHECK_FUNC_RESULT(res, 470, exit);
          }
          res = arad_pp_flp_process_ipv4mc_bridge(unit,TRUE,sa_auth_enabled, slb_enabled);
          SOC_SAND_CHECK_FUNC_RESULT(res, 475, exit);
      }


      /* --------------------------------------------------------------------------*/
      /* ----------- BIDIR support - must be above IPv4 MC COMP + RPF -------------*/
      /* --------------------------------------------------------------------------*/
      if(SOC_DPP_CONFIG(unit)->l3.nof_rps != 0){
          res = arad_pp_flp_ipmc_bidir_progs_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 480, exit);
      }

      /* --------------------------------------------*/
      /* ----------- IPv4 MC COMP + RPF -------------*/
      /* --------------------------------------------*/
      if (SOC_DPP_CONFIG(unit)->l3.ipmc_vpn_lookup_enable) {
          /* for IPMC packet with VRF !=0  (VPN) forwarding is according to VRF, G */
          res = arad_pp_flp_prog_sel_cam_ipv4compmc_with_rpf(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 485, exit);

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
          if((JER_KAPS_ENABLE(unit)) || (ARAD_KBP_ENABLE_IPV4_MC && SOC_IS_JERICHO(unit))){
              res = arad_pp_flp_dbal_ipv4compmc_with_rpf_program_tables_init(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 490, exit);
          }
          else 
#endif 
          {
              res = arad_pp_flp_key_const_ipv4compmc_with_rpf(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 495, exit);
              res = arad_pp_flp_lookups_ipv4compmc_with_rpf(unit, ARAD_TCAM_ACCESS_PROFILE_INVALID);
              SOC_SAND_CHECK_FUNC_RESULT(res, 500, exit);
              /* this function update the SW state about CE usage*/
              res = arad_pp_flp_instruction_rsrc_set(unit, PROG_FLP_IPV4COMPMC_WITH_RPF);
              SOC_SAND_CHECK_FUNC_RESULT(res, 505, exit);
              res = arad_pp_flp_dbal_ipv4mc_tcam_tables_init(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 510, exit);
          }      
          res = arad_pp_flp_process_ipv4compmc_with_rpf(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 520, exit);
      }

      /* ---------------------------------*/
      /* ----------- IPv6 UC -------------*/
      /* ---------------------------------*/
      res = arad_pp_flp_prog_sel_cam_ipv6uc(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 540, exit);

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_IPV6_UC){
          if (SOC_IS_JERICHO(unit)) {
              res = arad_pp_flp_dbal_ipv6uc_program_tables_init(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 545, exit);
          }
          else{
              res = arad_pp_flp_key_const_ipv6uc(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 550, exit);
              res = arad_pp_flp_lookups_ipv6uc(unit,ARAD_TCAM_ACCESS_PROFILE_INVALID);
              SOC_SAND_CHECK_FUNC_RESULT(res, 555, exit);
          }
      }
      else
#endif
      {
          res = arad_pp_flp_dbal_ipv6uc_program_tables_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 560, exit);
      }
      res = arad_pp_flp_process_ipv6uc(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 565, exit);

      /* ---------------------------------*/
      /* ----------- IPv6 MC -------------*/
      /* ---------------------------------*/
      res = arad_pp_flp_prog_sel_cam_ipv6mc(unit, &ipv6mc_cam_sel_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 570, exit);
      if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ipv6_mc_forwarding_disable", 0))
      {          
          ARAD_PP_IHB_FLP_PROCESS_TBL_DATA flp_process_tbl;
          res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, ipv6mc_cam_sel_id, &flp_process_tbl);
          SOC_SAND_CHECK_FUNC_RESULT(res, 575, exit);
          flp_process_tbl.not_found_trap_strength = 0;
          flp_process_tbl.not_found_trap_code = SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_MCV6;
          res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, ipv6mc_cam_sel_id, &flp_process_tbl);
          SOC_SAND_CHECK_FUNC_RESULT(res, 580, exit);
      }
      else {
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
          if(ARAD_KBP_ENABLE_IPV6_MC){
              if (SOC_IS_JERICHO(unit)) {
                  res = arad_pp_flp_dbal_ipv6mc_program_tables_init(unit);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 585, exit);
              }
              else{
                  res = arad_pp_flp_key_const_ipv6mc(unit);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 590, exit);
                  res = arad_pp_flp_lookups_ipv6mc(unit,ARAD_TCAM_ACCESS_PROFILE_INVALID);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 595, exit);
              }
          }
          else
#endif
          {
              res = arad_pp_flp_dbal_ipv6mc_program_tables_init(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 560, exit);
          }
          res = arad_pp_flp_process_ipv6mc(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 565, exit);
      }

      /* ---------------------------------*/
      /* ------------- MPLS --------------*/
      /* ---------------------------------*/
      res = arad_pp_flp_prog_sel_cam_lsr(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 570, exit);
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if(ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {
          res = arad_pp_flp_key_const_lsr(unit, FALSE, FALSE, FALSE);
          SOC_SAND_CHECK_FUNC_RESULT(res, 575, exit);
          res = arad_pp_flp_lookups_lsr(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 580, exit);
      } 
#endif
      res = arad_pp_flp_dbal_lsr_program_tables_init(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 585, exit);
      res = arad_pp_flp_process_lsr(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 590, exit);

      /* Configure Tables for trill.*/
      if (SOC_DPP_CONFIG(unit)->trill.mode)
      {
            res = arad_pp_flp_prog_sel_cam_TRILL_uc(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 575, exit);
            res = arad_pp_flp_key_const_TRILL_uc(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 580, exit);
            res = arad_pp_flp_lookups_TRILL_uc(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 585, exit);
            res = arad_pp_flp_process_TRILL_uc(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 590, exit);

          if (SOC_IS_ARADPLUS_AND_BELOW(unit))
          {
                res = arad_pp_flp_prog_sel_cam_TRILL_mc_after_recycle_overlay(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 595, exit);
                res = arad_pp_flp_key_const_TRILL_mc_after_recycle_overlay(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 600, exit);
                res = arad_pp_flp_lookups_TRILL_mc_after_recycle_overlay(unit, sa_auth_enabled, slb_enabled);
                SOC_SAND_CHECK_FUNC_RESULT(res, 605, exit);
                res = arad_pp_flp_process_TRILL_mc_after_recycle_overlay(unit,TRUE,sa_auth_enabled, slb_enabled);
                SOC_SAND_CHECK_FUNC_RESULT(res, 610, exit);
          }

            res = arad_pp_flp_prog_sel_cam_TRILL_mc(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 615, exit);
            res = arad_pp_flp_key_const_TRILL_mc(unit);        
            SOC_SAND_CHECK_FUNC_RESULT(res, 620, exit);
            res = arad_pp_flp_lookups_TRILL_mc(unit,ingress_learn_enable,ARAD_TCAM_ACCESS_PROFILE_INVALID);
            SOC_SAND_CHECK_FUNC_RESULT(res, 625, exit);
          if (SOC_DPP_CONFIG(unit)->trill.transparent_service){
                res = arad_pp_flp_dbal_trill_program_tcam_tables_init(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 630, exit);
          }
            res = arad_pp_flp_process_TRILL_mc(unit,ingress_learn_enable);
            SOC_SAND_CHECK_FUNC_RESULT(res, 635, exit);
      }

      /* load VPLSoGRE if needed */
      
      if (SOC_IS_ARADPLUS_AND_BELOW(unit) && (SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & SOC_DPP_IP_TUNNEL_TERM_DB_NVGRE)) {
          res = arad_pp_flp_pwe_gre_progs_init(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 1573, exit);
      }

      /* ---------------------------------------*/
      /* ----------- IPv6 UC + RPF -------------*/
      /* ---------------------------------------*/
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      if (SOC_IS_JERICHO(unit)) {
          /*Using the "l3_ipv6_uc_use_tcam" SOC for using the  TCAM for IPV6UC disable the use of IPV6UC-RPF*/
          if(!soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "l3_ipv6_uc_use_tcam", 0)) {
              int prog_id;
              res = arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf(unit,&prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 660, exit);

              if (JER_KAPS_ENABLE(unit) || ARAD_KBP_ENABLE_IPV6_UC) {
                  res = arad_pp_flp_dbal_ipv6uc_with_rpf_program_tables_init(unit,prog_id);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 665, exit);
              }
              res = arad_pp_flp_process_ipv6uc_with_rpf(unit, prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 670, exit);
          }
      } else {
          if(ARAD_KBP_ENABLE_IPV6_UC){
              /* If System is configue with ELK with up to 144b ROP, use 2PASS RPF FLP process */
              if (!SOC_DPP_CONFIG(unit)->pp.ipv6_with_rpf_2pass_exists) 
              {
                  /* probably this code does nothing in Arad+ */
                  res = arad_pp_flp_ipv6uc_with_rpf_prog_init(unit);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 33, exit);
              }
          }
      }
#endif
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      /* -----------------------------------------*/
      /* --------- IPv4 UC + L3VPN + RPF ---------*/
      /* -----------------------------------------*/
      if (JER_KAPS_ENABLE(unit) && !ARAD_KBP_ENABLE_IPV4_UC) {
          int prog_id;
          res = arad_pp_flp_prog_sel_cam_ipv4uc_l3vpn_rpf(unit, &prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 695, exit);
          res = arad_pp_flp_dbal_ipv4uc_l3vpn_rpf_program_tables_init(unit, prog_id);  
          SOC_SAND_CHECK_FUNC_RESULT(res, 700, exit);
          res = arad_pp_flp_process_ipv4uc_l3vpn_rpf(unit, prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 705, exit);
      }
#endif

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      /* -----------------------------------------*/
      /* ----------- IPv6 UC + L3VPN -------------*/
      /* -----------------------------------------*/
      if (JER_KAPS_ENABLE(unit) && !ARAD_KBP_ENABLE_IPV6_UC) {
          int prog_id;
          arad_pp_flp_prog_sel_cam_ipv6uc_l3vpn(unit,&prog_id);
          arad_pp_flp_dbal_ipv6uc_l3vpn_program_tables_init(unit,prog_id);
          arad_pp_flp_process_ipv6uc_l3vpn(unit,prog_id);
      }
#endif

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
      /* -----------------------------------------*/
      /* --------- IPv6 UC + L3VPN + RPF ---------*/
      /* -----------------------------------------*/
      if (JER_KAPS_ENABLE(unit) && !ARAD_KBP_ENABLE_IPV6_RPF) {
          int prog_id;
          arad_pp_flp_prog_sel_cam_ipv6uc_l3vpn_rpf(unit, &prog_id);
          arad_pp_flp_dbal_ipv6uc_l3vpn_rpf_program_tables_init(unit, prog_id);
          arad_pp_flp_process_ipv6uc_l3vpn_rpf(unit, prog_id);
      }
#endif

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      /* IPv6 support in ELK */
      if(ARAD_KBP_ENABLE_IPV6_EXTENDED){
          /* load IPv6 (L3) FWD with RPF in 2 passes if configured */
          if (SOC_DPP_CONFIG(unit)->pp.ipv6_with_rpf_2pass_exists) 
          {
              int32 prog_id;
              res = arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf_2pass(unit,&prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 710, exit);
              res = arad_pp_flp_key_const_ipv6uc_with_rpf_2pass(unit,prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 715, exit);
              res = arad_pp_flp_lookups_ipv6uc_with_rpf_2pass(unit,prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 720, exit);
              res = arad_pp_flp_process_ipv6uc_with_rpf_2pass(unit,prog_id);
              SOC_SAND_CHECK_FUNC_RESULT(res, 725, exit);
              res = arad_pp_flp_prog_sel_cam_ipv6uc_with_rpf_2pass_fwd(unit);
              SOC_SAND_CHECK_FUNC_RESULT(res, 730, exit);
          }
      }
#endif /* INCLUDE_KBP & !defined(BCM_88030) */

  }
#endif /* INCLUDE_L3*/  

  if (!(SOC_DPP_CONFIG(unit)->l3.ipmc_vpn_lookup_enable)) {
      int program_id = 0;
      /* for IPMC packet forwarding is according to <RIF,G,SIP> regardless the VRF value */
      res = arad_pp_flp_prog_sel_cam_global_ipv4compmc_with_rpf(unit, &program_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 735, exit);
      res = arad_pp_flp_key_const_global_ipv4compmc_with_rpf(unit, program_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 740, exit);
      res = arad_pp_flp_lookups_global_ipv4compmc_with_rpf(unit, program_id, ARAD_TCAM_ACCESS_PROFILE_INVALID);
      SOC_SAND_CHECK_FUNC_RESULT(res, 745, exit);
      /* this function update the SW state about CE usage*/
      res = arad_pp_flp_instruction_rsrc_set(unit, program_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 750, exit);
      res = arad_pp_flp_dbal_ipv4mc_tcam_tables_init(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 755, exit);
      res = arad_pp_flp_process_global_ipv4compmc_with_rpf(unit, program_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 760, exit);
  }

  if (SOC_DPP_CONFIG(unit)->pp.pon_application_enable) {
      int32 prog_id;

      if (SOC_DPP_CONFIG(unit)->pp.local_switching_enable) {
          /* ARAD FLP for local route upstream: DA+SA+COS */
          res = arad_pp_flp_prog_sel_cam_ethernet_pon_local_route(unit, mac_in_mac_enabled, &prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 765, exit);
          res = arad_pp_flp_key_const_ethernet_pon_local_route(unit, prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 770, exit);
          res = arad_pp_flp_lookups_ethernet_pon_local_route(unit, sa_auth_enabled, slb_enabled, prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 775, exit);
          res = arad_pp_flp_process_ethernet_pon_local_route(unit, TRUE,sa_auth_enabled, slb_enabled, prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 780, exit);
      } 

      /* ARAD FLP for non local route upstream: SA+PTC */
      res = arad_pp_flp_prog_sel_cam_ethernet_pon_default_upstream(unit, &prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 785, exit);
      res = arad_pp_flp_key_const_ethernet_pon_default_upstream(unit, prog_id);  
      SOC_SAND_CHECK_FUNC_RESULT(res, 790, exit);
      res = arad_pp_flp_lookups_ethernet_pon_default_upstream(unit, sa_auth_enabled, slb_enabled, prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 795, exit);
      res = arad_pp_flp_process_ethernet_pon_default_upstream(unit, TRUE,sa_auth_enabled, slb_enabled, prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 800, exit);
      /* ARAD FLP for non local route downstream: DA */
      res = arad_pp_flp_prog_sel_cam_ethernet_pon_default_downstream(unit, &prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 805, exit);
      res = arad_pp_flp_key_const_ethernet_pon_default_downstream(unit, prog_id);  
      SOC_SAND_CHECK_FUNC_RESULT(res, 810, exit);
      res = arad_pp_flp_lookups_ethernet_pon_default_downstream(unit, ARAD_TCAM_ACCESS_PROFILE_INVALID, sa_auth_enabled, slb_enabled, prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 815, exit);
      res = arad_pp_flp_process_ethernet_pon_default_downstream(unit, TRUE,sa_auth_enabled, slb_enabled, prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 820, exit);
  } else {
      int32 mac_in_mac_prog_id;

      res = arad_pp_flp_prog_sel_cam_ethernet_ing_learn(unit, mac_in_mac_enabled, &mac_in_mac_prog_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 825, exit);
      res = arad_pp_flp_key_const_ethernet_ing_learn(unit);  
      SOC_SAND_CHECK_FUNC_RESULT(res, 830, exit);
      res = arad_pp_flp_lookups_ethernet_ing_learn(unit, ingress_learn_enable, sa_auth_enabled, slb_enabled);
      SOC_SAND_CHECK_FUNC_RESULT(res, 835, exit);
      res = arad_pp_flp_process_ethernet_ing_learn(unit, TRUE,sa_auth_enabled, slb_enabled);
      SOC_SAND_CHECK_FUNC_RESULT(res, 840, exit);

      if (mac_in_mac_enabled) {
          /* MAC termination */      
          res = arad_pp_flp_key_const_mac_in_mac_after_termination(unit, mac_in_mac_prog_id);      
          SOC_SAND_CHECK_FUNC_RESULT(res, 845, exit);
          res = arad_pp_flp_lookups_mac_in_mac_after_termination(unit, mac_in_mac_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 850, exit);
          res = arad_pp_flp_process_mac_in_mac_after_termination(unit,TRUE, mac_in_mac_prog_id);      
          SOC_SAND_CHECK_FUNC_RESULT(res, 855, exit);
      }
  }

  /* load all FC programs */
  if(SOC_DPP_CONFIG(unit)->pp.fcoe_enable){
      res = arad_pp_flp_fcoe_progs_init(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
  }else
  {
      int32 prog_id;
      res = arad_pp_flp_prog_sel_cam_fcoe_ethernet_ing_learn(unit, &prog_id, FALSE);
      SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
  }

  if (SOC_DPP_CONFIG(unit)->pp.vmac_enable) {
    res = arad_pp_flp_vmac_progs_init(unit, sa_auth_enabled, slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);
  }
  if (SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop || (SOC_DPP_CONFIG(unit)->pp.bfd_ipv6_enable==SOC_DPP_ARAD_BFD_IPV6_SUPPORT_WITH_LEM)) {

      res = arad_pp_flp_prog_sel_cam_bfd_single_hop_bridge(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 875, exit);

      if (SOC_DPP_CONFIG(unit)->pp.bfd_ipv4_single_hop) {
          int bfd_ipv4_single_hop_prog_id;
          res = arad_pp_flp_prog_sel_cam_bfd_single_hop(unit, 0, &bfd_ipv4_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 880, exit);
          res = arad_pp_flp_key_const_bfd_single_hop(unit, bfd_ipv4_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 885, exit);
          res = arad_pp_flp_lookups_bfd_single_hop(unit, bfd_ipv4_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 890, exit);
          res = arad_pp_flp_process_bfd_single_hop(unit, 0, bfd_ipv4_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 895, exit);
      }

      if ((SOC_DPP_CONFIG(unit)->pp.bfd_ipv6_enable==SOC_DPP_ARAD_BFD_IPV6_SUPPORT_WITH_LEM)) {
          int bfd_ipv6_single_hop_prog_id;
          res = arad_pp_flp_prog_sel_cam_bfd_single_hop(unit, 1, &bfd_ipv6_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 900, exit);
          res = arad_pp_flp_key_const_bfd_single_hop(unit, bfd_ipv6_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 905, exit);
          res = arad_pp_flp_lookups_bfd_single_hop(unit, bfd_ipv6_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 910, exit);
          res = arad_pp_flp_process_bfd_single_hop(unit, 1, bfd_ipv6_single_hop_prog_id);
          SOC_SAND_CHECK_FUNC_RESULT(res, 915, exit);
      }
  }
   

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_initial_programs", 0, 0);
}

/*
  * FLP key construction for LPM custom lookups.
  * This function must be called before arad_pp_flp_lookup_lpm_custom_lookup and
  * arad_pp_flp_process_lpm_custom_lookup
  *
  */
STATIC uint32
  arad_pp_flp_key_const_lpm_custom_lookup(
     int unit,
     int32 prog_id,
     uint32 db_indx
   )
{
    uint32
      res,
      indx,
      inst,
      inst_0_to_5_valid,
      inst_6_to_11_valid,
      *inst_valid;

    ARAD_FLP_CUSTOM_LPM_LOOKUP_INFO *lpm_lookup;  
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
      flp_key_construction_tbl_0_5,
      flp_key_construction_tbl_6_11,
      *flp_key_construction;

    ARAD_FLP_CUSTOM_LPM_KEY_CONSTR *key;
    ARAD_FLP_CUSTOM_LPM_INSTRUCTION_INFO *inst_info;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    lpm_lookup = &Arad_flp_custom_lpm_lookup_info[db_indx];

    /*use key a instead of key c*/
    if (SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1){
        if (lpm_lookup->app_id == PROG_FLP_IPV4UC) {
            lpm_lookup->key[0].key =0;
            lpm_lookup->key[1].key = 0;
        }
    }


    for (indx=0; indx<lpm_lookup->lookup_num; ++indx) {
        key = &lpm_lookup->key[indx];

        if ( (indx == 0) || (key->key != lpm_lookup->key[0].key) ) { /* do key construction once if we are using the same key */
            res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl_0_5);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl_6_11);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            inst_0_to_5_valid = 0;
            inst_6_to_11_valid = 0;

            /* key construction */
            for (inst=0; inst<key->inst_num; ++inst) {
                inst_info = &key->inst[inst];

                if (inst_info->is_inst_6_11) {
                    flp_key_construction = &flp_key_construction_tbl_6_11;
                    inst_valid = &inst_6_to_11_valid;
                }
                else {
                    flp_key_construction = &flp_key_construction_tbl_0_5;
                    inst_valid = &inst_0_to_5_valid;
                }
                
                switch (inst_info->id) {
                    case 0:
                        flp_key_construction->instruction_0_16b = inst_info->instruction;
                        break;
                    case 1:
                        flp_key_construction->instruction_1_16b = inst_info->instruction;
                        break;
                    case 2:
                        flp_key_construction->instruction_2_16b = inst_info->instruction;
                        break;
                    case 3:
                        flp_key_construction->instruction_3_32b = inst_info->instruction;
                        break;
                    case 4:
                        flp_key_construction->instruction_4_32b = inst_info->instruction;
                        break;
                    case 5:
                        flp_key_construction->instruction_5_32b = inst_info->instruction;
                        break;
                    default:
                        break;
                }

                *inst_valid |= (0x1 << inst_info->id);
            }

            if (inst_0_to_5_valid) {
                switch (key->key) {
                    case 0:
                        flp_key_construction_tbl_0_5.key_a_inst_0_to_5_valid = inst_0_to_5_valid;

                        if ((flp_key_construction_tbl_0_5.key_b_inst_0_to_5_valid & inst_0_to_5_valid) ||
                            (flp_key_construction_tbl_0_5.key_c_inst_0_to_5_valid & inst_0_to_5_valid)) {                           
                            uint8 app_id;
                            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

                            LOG_ERROR(BSL_LS_SOC_INIT,
                                     (BSL_META_U(unit,
                                                 "[%s] trying to override instructions used by key b/key c\n"),
                                      arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                            SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 41, exit);
                            }
                        break;
                    case 1:
                        flp_key_construction_tbl_0_5.key_b_inst_0_to_5_valid = inst_0_to_5_valid;

                        if ((flp_key_construction_tbl_0_5.key_a_inst_0_to_5_valid & inst_0_to_5_valid) ||
                            (flp_key_construction_tbl_0_5.key_c_inst_0_to_5_valid & inst_0_to_5_valid)) {                           
                            uint8 app_id;
                            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 42, exit);

                            LOG_ERROR(BSL_LS_SOC_INIT,
                                     (BSL_META_U(unit,
                                                 "[%s] trying to override instructions used by key a/key c\n"),
                                      arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                            SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 43, exit);
                            }
                        break;
                    case 2:
                        flp_key_construction_tbl_0_5.key_c_inst_0_to_5_valid = inst_0_to_5_valid;

                        if ((flp_key_construction_tbl_0_5.key_a_inst_0_to_5_valid & inst_0_to_5_valid) ||
                            (flp_key_construction_tbl_0_5.key_b_inst_0_to_5_valid & inst_0_to_5_valid)) {                           
                            uint8 app_id;
                            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 44, exit);

                            LOG_ERROR(BSL_LS_SOC_INIT,
                                     (BSL_META_U(unit,
                                                 "[%s] trying to override instructions used by key a/key b\n"),
                                      arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                            SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 45, exit);
                            }
                        break;
                    default:
                         break;
                }

                res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl_0_5);
                SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
            }

            if (inst_6_to_11_valid) {
                switch (key->key) {
                    case 0:
                        flp_key_construction_tbl_6_11.key_a_inst_0_to_5_valid = inst_6_to_11_valid;

                        if ((flp_key_construction_tbl_6_11.key_b_inst_0_to_5_valid & inst_0_to_5_valid) ||
                            (flp_key_construction_tbl_6_11.key_c_inst_0_to_5_valid & inst_0_to_5_valid)) {                           
                            uint8 app_id;
                            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 46, exit);

                            LOG_ERROR(BSL_LS_SOC_INIT,
                                     (BSL_META_U(unit,
                                                 "[%s] trying to override instructions used by key b/key c\n"),
                                      arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                            SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 47, exit);
                            }
                        break;
                    case 1:
                        flp_key_construction_tbl_6_11.key_b_inst_0_to_5_valid = inst_6_to_11_valid;

                        if ((flp_key_construction_tbl_6_11.key_a_inst_0_to_5_valid & inst_0_to_5_valid) ||
                            (flp_key_construction_tbl_6_11.key_c_inst_0_to_5_valid & inst_0_to_5_valid)) {                           
                            uint8 app_id;
                            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit);

                            LOG_ERROR(BSL_LS_SOC_INIT,
                                     (BSL_META_U(unit,
                                                 "[%s] trying to override instructions used by key b/key c\n"),
                                      arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                            SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 49, exit);
                            }
                        break;
                    case 2:
                        flp_key_construction_tbl_6_11.key_c_inst_0_to_5_valid = inst_6_to_11_valid;

                        if ((flp_key_construction_tbl_6_11.key_a_inst_0_to_5_valid & inst_0_to_5_valid) ||
                            (flp_key_construction_tbl_6_11.key_b_inst_0_to_5_valid & inst_0_to_5_valid)) {                           
                            uint8 app_id;
                            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

                            LOG_ERROR(BSL_LS_SOC_INIT,
                                     (BSL_META_U(unit,
                                                 "[%s] trying to override instructions used by key b/key c\n"),
                                      arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                            SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_OUT_OF_RESOURCES, 51, exit);
                            }
                        break;
                    default:
                         break;
                }

                res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl_6_11);
                SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
            }
        }
    }

exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_key_const_lpm_custom_lookup", 0, 0);
}

/*
  * FLP lookup for LPM custom lookups.
  *
  */
STATIC uint32
  arad_pp_flp_lookup_lpm_custom_lookup(
     int unit,
     int32 prog_id,
     uint32 db_indx
   )
{
    uint32
      res,
      indx,
      lookup;
      
    ARAD_FLP_CUSTOM_LPM_LOOKUP_INFO *lpm_lookup;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;
    ARAD_FLP_CUSTOM_LPM_KEY_CONSTR *key;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	lpm_lookup = &Arad_flp_custom_lpm_lookup_info[db_indx];

    /*use key a instead of key c*/
    if (SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1){
        if (lpm_lookup->app_id == PROG_FLP_IPV4UC) {
            lpm_lookup->key[0].key =0;
            lpm_lookup->key[1].key =0;
        }
    }
	
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (indx=0; indx<lpm_lookup->lookup_num; ++indx) {
        key = &lpm_lookup->key[indx];
        lookup = (lpm_lookup->lookup_num>1)?indx:lpm_lookup->lookup_to_use;

        /* flp lookup */
        if (lookup == 0) {
            flp_lookups_tbl.lpm_1st_lkp_valid = 1;
            flp_lookups_tbl.lpm_1st_lkp_key_select = key->key;
            flp_lookups_tbl.lpm_1st_lkp_and_value = 0;
            flp_lookups_tbl.lpm_1st_lkp_or_value = key->key_or_value;
        }
        else {
            flp_lookups_tbl.lpm_2nd_lkp_valid = 1;
            flp_lookups_tbl.lpm_2nd_lkp_key_select = key->key;
            flp_lookups_tbl.lpm_2nd_lkp_and_value = 0;
            flp_lookups_tbl.lpm_2nd_lkp_or_value = key->key_or_value;
        }
        
    }

	res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
	SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_lookup_lpm_custom_lookup", 0, 0);
}

/*
  * FLP lookup for LPM custom lookups.
  *
  */
STATIC uint32
  arad_pp_flp_process_lpm_custom_lookup(
     int unit,
     int32 prog_id,
     uint32 db_indx
   )
{
    uint32
      res,
      indx,
      lookup,
      update = 0;
      
    ARAD_FLP_CUSTOM_LPM_LOOKUP_INFO *lpm_lookup;
    ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
        flp_process_tbl;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	lpm_lookup = &Arad_flp_custom_lpm_lookup_info[db_indx];
	
    /*use key a instead of key c*/
    if (SOC_DPP_CONFIG(unit)->pp.bfd_echo_with_lem == 1){
        if (lpm_lookup->app_id == PROG_FLP_IPV4UC) {
            lpm_lookup->key[0].key =0;
            lpm_lookup->key[1].key =0;
        }
    }

    /* flp process */
    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (indx=0; indx<lpm_lookup->lookup_num; ++indx) {
        lookup = (lpm_lookup->lookup_num>1)?indx:lpm_lookup->lookup_to_use;
        
        /* flp lookup */
        if (lookup == 0) {
            
            if (flp_process_tbl.include_lpm_1st_in_result_a || flp_process_tbl.include_lpm_1st_in_result_b) {
                uint8 app_id;
                res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
                
                /* if LPM 1st lookup is used for custom lookup, FLP should not include LPM 1st lookup in result */
                #if 0
                LOG_INFO(BSL_LS_SOC_INIT,
                         (BSL_META_U(unit,
                                     "[%s] LPM 1st lookup is used for customized lookup,"
                                     " but the LPM 1st lookup result is included in FLP result.\n"),
                          arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                #endif
                flp_process_tbl.include_lpm_1st_in_result_a = 0;
                flp_process_tbl.include_lpm_1st_in_result_b = 0;
                update = 1;
            }
        }
        else {
            if (flp_process_tbl.include_lpm_2nd_in_result_a || flp_process_tbl.include_lpm_2nd_in_result_b) {
                uint8 app_id;
                res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit, prog_id, &app_id);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);
                
                /* if LPM 2nd lookup is used for custom lookup, FLP should not include LPM 2nd lookup in result */
                #if 0
                LOG_INFO(BSL_LS_SOC_INIT,
                         (BSL_META_U(unit,
                                     "[%s] LPM 2nd lookup is used for customized lookup,"
                                     " but the LPM 2nd lookup result is included in FLP result.\n"),
                          arad_pp_flp_prog_id_to_prog_name(unit, app_id)));
                #endif
                flp_process_tbl.include_lpm_2nd_in_result_a = 0;
                flp_process_tbl.include_lpm_2nd_in_result_b = 0;
                update = 1;
            }
        }
        
    }

    if (update) {
        res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_process_lpm_custom_lookup", 0, 0);
}

STATIC uint32
  arad_pp_flp_lpm_custom_lookup_enable(
     int unit,
     uint8 app_id
   )
{
    uint32
      res,
      prog_id,
      indx;
    int found = 0;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (!ARAD_LPM_CUSTOM_LOOKUP_ENABLED(unit)) {
        goto exit; /* OK */
    }

    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, app_id, &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (prog_id == -1) {
        goto exit; /* No such program */
    }

    for (indx=0; indx<sizeof(Arad_flp_custom_lpm_lookup_info)/sizeof(Arad_flp_custom_lpm_lookup_info[0]); ++indx) {
        if (Arad_flp_custom_lpm_lookup_info[indx].app_id == app_id) {
            found = 1;
            break;
            }
        }

    if (!found) {
        goto exit;
        }

    res = arad_pp_flp_key_const_lpm_custom_lookup(unit, prog_id, indx);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    res = arad_pp_flp_lookup_lpm_custom_lookup(unit, prog_id, indx);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    res = arad_pp_flp_process_lpm_custom_lookup(unit, prog_id, indx);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_lpm_custom_lookup_enable", 0, 0);
}

STATIC uint32
  arad_pp_flp_lpm_custom_lookup_init(
     int unit
   )
{
    uint32
      res,
      indx;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    for (indx=0; indx<sizeof(Arad_flp_custom_lpm_lookup_info)/sizeof(Arad_flp_custom_lpm_lookup_info[0]); ++indx) {
        res = arad_pp_flp_lpm_custom_lookup_enable(unit, Arad_flp_custom_lpm_lookup_info[indx].app_id);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_lpm_custom_lookup_init", 0, 0);
}

/* initialize FLP programs */
uint32
  arad_pp_flp_init(
     int unit,
     uint8 ingress_learn_enable, /* = 1*/
     uint8 ingress_learn_oppurtunistic, /* = 0 */
     uint32  sa_lookup_type /*hex 2'b10*/
   )
{
  uint32 res;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_init ingress_learn_enable: %h ingress_learn_enable %h sa_lookup_type %h",
                          ingress_learn_enable,ingress_learn_oppurtunistic,sa_lookup_type);*/

  res = arad_pp_flp_static_programs_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 1, exit);
  res = arad_pp_flp_prog_sel_cam_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 1, exit);
  res = arad_pp_flp_lookups_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 2, exit);
  res = arad_pp_flp_process_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 3, exit);
  res = arad_pp_flp_lookups_initial_programs(unit, ingress_learn_enable,ingress_learn_oppurtunistic);
  SOC_SAND_CHECK_FUNC_RESULT(res, 4, exit);

  if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "lpm_l2_mc", 0)) {
      res = arad_pp_ipv4mc_bridge_lookup_update(unit,1);
      SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  }

  res = arad_pp_flp_lpm_custom_lookup_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
  
  /* Indicate the instructions taken for forwarding - must be at the end of the function */
  res = arad_pp_flp_all_progs_instruction_set(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_init", 0, 0);
}

/* update ethernet program to perform ingress/egress learning */
uint32
  arad_pp_flp_ethernet_prog_update(
     int unit,
     uint8 learn_enable,
     uint8 ingress_learn_enable, /* = 1*/
     uint8 ingress_learn_oppurtunistic, /* = 0 */
     uint32  sa_lookup_type /*hex 2'b10*/
   )
{
  uint32
    res;
  uint8
    mac_in_mac_enabled = 0;
  uint32
    tcam_access_profile_id;
  uint8
    prog_id = 0,
    slb_enabled = 0,
    sa_auth_enabled = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  /* In PON application, ethernet ingress learn program was replaced by pon upsteam/downstream programs.
     Doesn't set ethernet ingress learn program */
  if (!SOC_DPP_CONFIG(unit)->pp.pon_application_enable) {
    res = arad_pp_flp_lookups_ethernet_ing_learn(unit,ingress_learn_enable,sa_auth_enabled,slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_flp_process_ethernet_ing_learn(unit,learn_enable,sa_auth_enabled,slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  }

  /* if mac in mac is enabled, set ethernet_mac_in_mac program and ethernet_pbp (inside ethernet_ing_learn) */  
  res = arad_pp_is_mac_in_mac_enabled(
      unit,
      &mac_in_mac_enabled
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);

  if (mac_in_mac_enabled) 
  {
    uint32 prog_id;
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_MAC_IN_MAC_AFTER_TERMINATIOM, &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    res = arad_pp_flp_process_mac_in_mac_after_termination(unit, learn_enable, prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);
  }

  res = arad_pp_flp_lookups_ipv4mc_bridge_update(unit, sa_auth_enabled,slb_enabled);
  SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

  res = arad_pp_flp_process_ipv4mc_bridge(unit, learn_enable, sa_auth_enabled,slb_enabled);
  SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);


  if (SOC_DPP_CONFIG(unit)->trill.mode) 
  {
    res = arad_pp_flp_lookups_TRILL_mc_update(unit, ingress_learn_enable);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    res = arad_pp_flp_process_TRILL_mc(unit,ingress_learn_enable);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
      res = arad_pp_flp_lookups_TRILL_mc_after_recycle_overlay(unit,sa_auth_enabled,slb_enabled);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      res = arad_pp_flp_process_TRILL_mc_after_recycle_overlay(unit,learn_enable,sa_auth_enabled,slb_enabled);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
  }

  if (SOC_DPP_L3_SRC_BIND_IPV4_ENABLE(unit) && SOC_IS_ARADPLUS_AND_BELOW(unit))
  {
    /* ingress learning for Spoof v4 */
    res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_dhcp(unit, sa_auth_enabled, slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v4_static(unit, sa_auth_enabled, slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    res = arad_pp_flp_process_ethernet_tk_epon_uni_v4(unit,learn_enable,sa_auth_enabled,slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
  }

  if (SOC_DPP_L3_SRC_BIND_IPV6_ENABLE(unit))
  {
    /* ingress learning for Spoof v6 */
    res = arad_pp_flp_tk_epon_uni_v6_tcam_profile_get(unit,&tcam_access_profile_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);

    if(!SOC_IS_JERICHO(unit)){
       res = arad_pp_flp_lookups_ethernet_tk_epon_uni_v6(unit,tcam_access_profile_id,sa_auth_enabled,slb_enabled);
       SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }
    res = arad_pp_flp_process_ethernet_tk_epon_uni_v6(unit,learn_enable,sa_auth_enabled,slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 65, exit); 
  }
  if (SOC_DPP_CONFIG(unit)->pp.l3_src_bind_arp_relay & SOC_DPP_L3_SRC_BIND_FOR_ARP_RELAY_UP) {
    res = arad_pp_flp_app_to_prog_index_get(unit,
                                            PROG_FLP_PON_ARP_UPSTREAM,
                                            &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    /* ingress learning for Spoof v4 */
    res = arad_pp_flp_lookups_pon_arp_upstream(unit,sa_auth_enabled,slb_enabled, prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

    res = arad_pp_flp_process_pon_arp_upstream(unit,learn_enable,sa_auth_enabled,slb_enabled, prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_ethernet_prog_update", 0, 0);
}

/* Retrieve the FLP-Program according to the application type */
uint32 arad_pp_flp_app_to_prog_index_get(
   int unit,
   uint32 app_id,
   uint8  *prog_index
){

    uint8
        cur_prog_id;
    uint32
      res;
    uint32 
        indx;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);


    for (indx = 0; indx < SOC_DPP_DEFS_GET(unit, nof_flp_programs); ++indx) {
        res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit,indx,&cur_prog_id);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
        if (app_id == cur_prog_id) {
            *prog_index = indx;
            goto exit;
        }
    }
    SOC_SAND_SET_ERROR_CODE(SOC_PPD_ERR_NOT_EXIST, 20, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_app_to_prog_index_get", app_id, 0);
}

/* Retrieve the FLP-Program according to the application type */
uint32 arad_pp_prog_index_to_flp_app_get(
   int unit,
   uint32 prog_index,
   uint8 *app_id)
{

    uint32 res;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit,prog_index,app_id);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_prog_index_to_flp_app_get", prog_index, 0);
}

/* update trap configuration */
uint32
  arad_pp_flp_trap_config_update(
     int unit,
     SOC_PPC_TRAP_CODE_INTERNAL trap_code_internal, 
     int trap_strength, 
     int  snoop_strength
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  int line = -1;
  int indx;
  uint8 act_line=0;
  uint8 nof_lines = 1;
  uint8 map_id=0;
  uint8 use_lines_array = 0;
  int lines[4]; /* current max number of lines, used when programs are not consecutive */

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch(trap_code_internal) {
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_UNKNOWN_LABEL:
      line = PROG_FLP_LSR;
      if (SOC_DPP_CONFIG(unit)->pp.ipv4_tunnel_term_bitmap_enable & SOC_DPP_IP_TUNNEL_TERM_DB_NVGRE)
      {
        map_id = 1;
        nof_lines = 2;
        use_lines_array = 1;
        lines[0] = PROG_FLP_LSR;
        lines[1] = PROG_FLP_VPLSOGRE;
      }
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_P2P_MISCONFIGURATION:
      line = PROG_FLP_P2P;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_UC:
      line = PROG_FLP_TRILL_UC;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_MC:
      line = PROG_FLP_TRILL_MC_ONE_TAG;
      nof_lines = 2; /* for consecutive progs */

      map_id = 1; 
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6:
      map_id = 1;
      nof_lines = 4;
      use_lines_array = 1;
      lines[0] = PROG_FLP_IPV6UC;
      lines[1] = PROG_FLP_IPV6UC_RPF;
      lines[2] = PROG_FLP_IPV6UC_PUBLIC;
      lines[3] = PROG_FLP_IPV6UC_PUBLIC_RPF;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_MCV6:
      line = PROG_FLP_IPV6MC;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_FCF:
      line = PROG_FLP_FC_REMOTE;
      nof_lines = 4; /* for consecutive progs */
      map_id = 1;
      break;
  default:
      break;
  }

  /* no match trap in FLP */
  if((line == - 1) && (use_lines_array == 0)) {
      goto exit;
  }

  for (indx = 0;indx < nof_lines; line++,indx++) {
      if (use_lines_array) {
          line = lines[indx];
      }

      if (map_id != 0) {
          uint8 indx;
          uint8 cur_prog_id;
          /* Do not use arad_pp_flp_app_to_prog_index_get to avoid error prints when not found */
          for (indx = 0; indx < SOC_DPP_DEFS_GET(unit, nof_flp_programs); ++indx) {
              res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit,indx,&cur_prog_id);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);
              if ((uint32)line == cur_prog_id) {
                  act_line = indx;
                  break;
              }
          }

          /* Program not found, does not exist in this setting */
          if (indx == SOC_DPP_DEFS_GET(unit, nof_flp_programs)) {
              continue;
          }
      }
      else {
          act_line = (uint8)line;
      }
      res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, act_line, &flp_process_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 141, exit);

      if (trap_strength != -1) {
          flp_process_tbl.not_found_trap_strength      = trap_strength;
      }
      if (snoop_strength != -1) {
          flp_process_tbl.not_found_snoop_strength      = snoop_strength;
      }
      if ((trap_code_internal == SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6) || (act_line == PROG_FLP_IPV6MC)) {
          /* 0x0 - Use VRF Default Unicast
             0x1 - Use VRF Default Multicast
             Else: Use NotFoundTrapCode from program */

          if (flp_process_tbl.not_found_trap_strength) {
              /* take Trap default destination */
              flp_process_tbl.select_default_result_a = 2; 
              flp_process_tbl.select_default_result_b = 2;
          }
          else {
              /* take VRF default destination */
              if (trap_code_internal == SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6) {
                  flp_process_tbl.select_default_result_a = 0; 
                  flp_process_tbl.select_default_result_b = 0;
              }
              else { /* MC */
                  flp_process_tbl.select_default_result_a = 1; 
                  flp_process_tbl.select_default_result_b = 1;
              }
          }
      }

      res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, act_line, &flp_process_tbl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 142, exit);
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_trap_config_update", trap_code_internal, trap_strength);
}


uint32
  arad_pp_flp_trap_config_get(
     int unit,
     SOC_PPC_TRAP_CODE_INTERNAL trap_code_internal, 
     uint32 *trap_strength, 
     uint32  *snoop_strength
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  int line = -1;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  switch(trap_code_internal) {
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_MPLS_UNKNOWN_LABEL:
      line = PROG_FLP_LSR;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_P2P_MISCONFIGURATION:
      line = PROG_FLP_P2P;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_UC:
      line = PROG_FLP_TRILL_UC;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_TRILL_UNKNOWN_MC:
      line = PROG_FLP_TRILL_MC_ONE_TAG;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_UCV6:
      line = PROG_FLP_IPV6UC;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_DEFAULT_MCV6:
      line = PROG_FLP_IPV6MC;
      break;
  case SOC_PPC_TRAP_CODE_INTERNAL_FLP_FCF:
      line = PROG_FLP_FC_REMOTE;
      break;
  default:
      break;
  }
  /* no match trap in FLP */
  if(line == - 1) {
      goto exit;
  }
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, line, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 141, exit);

  if (trap_strength) {
      *trap_strength = flp_process_tbl.not_found_trap_strength;
  }
  if (snoop_strength) {
      *snoop_strength = flp_process_tbl.not_found_snoop_strength;
  }
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_trap_config_update", trap_code_internal, 0);
}


uint32
  arad_pp_flp_prog_learn_set(
     int unit,
     int32  fwd_code,
     uint8  learn_enable
  )
{
  uint32 res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    flp_cam_tbl_data;
  uint32 indx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (indx = 0; indx < SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines); indx++) {
      res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, indx, &flp_cam_tbl_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      /* Do not update learn_key_select for BFD single hop. The learn key is used to carry the BFD PDU offset to the PMF */
      if (flp_cam_tbl_data.parser_leaf_context != ARAD_PARSER_PLC_BFD_SINGLE_HOP) {

          if ((flp_cam_tbl_data.valid == 1) && (flp_cam_tbl_data.forwarding_code == fwd_code)) {
              res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, flp_cam_tbl_data.program, &flp_process_tbl);
              SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
              if(learn_enable)
              {
                  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, flp_cam_tbl_data.program, &flp_lookups_tbl);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

                  flp_lookups_tbl.learn_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;

                  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, flp_cam_tbl_data.program, &flp_lookups_tbl);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
              }

              flp_process_tbl.learn_enable = learn_enable;
              res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, flp_cam_tbl_data.program, &flp_process_tbl);
              SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
          }
      }
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_learn_set", 0, 0);
}


uint32
  arad_pp_flp_prog_learn_get(
     int unit,
     int32  fwd_code,
     uint8  *learn_enable
  )
{
  uint32 res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    flp_cam_tbl_data;
  uint32 indx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for (indx = 0; indx < SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines); indx++) {
      res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, indx, &flp_cam_tbl_data);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      /* Learn enable is irrelevant for BFD single hop.
       * The learn key is used to carry the BFD PDU offset to the PMF */
      if (flp_cam_tbl_data.parser_leaf_context != ARAD_PARSER_PLC_BFD_SINGLE_HOP) {

          if ((flp_cam_tbl_data.valid == 1) && (flp_cam_tbl_data.forwarding_code == fwd_code)) {
              res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, flp_cam_tbl_data.program, &flp_process_tbl);
              SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

              *learn_enable = flp_process_tbl.learn_enable;
              break;
          }
      }
  }

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_prog_learn_get", 0, 0);
}

uint32
   arad_pp_ipmc_not_found_proc_update(
     int unit,
     uint8  flood
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  uint8 program_id = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  /* ipv6 mc */  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV6MC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);
  flp_process_tbl.compatible_mc_bridge_fallback = flood;  
  flp_process_tbl.select_default_result_a = (flood)?2:1; /* 2: for flood, 1:default MC*/
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, PROG_FLP_IPV6MC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 21, exit);

  /* this is IPv4 mc bridging so it can be only flooding on vsi */
  if (SOC_DPP_CONFIG(unit)->l3.ipmc_vpn_lookup_enable) {
    /* ipv4 mc comp */  
    program_id = PROG_FLP_IPV4COMPMC_WITH_RPF;
  } else {
    /* Global ipv4 mc comp */
    res = arad_pp_flp_app_to_prog_index_get(unit,
                                            PROG_FLP_GLOBAL_IPV4COMPMC_WITH_RPF,
                                            &program_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  }
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, program_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 51, exit);
  flp_process_tbl.compatible_mc_bridge_fallback = flood;
  flp_process_tbl.select_default_result_a = (flood)?2:1; /* 2: for flood, 1:default MC*/
  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, program_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 61, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ipmc_not_found_proc_update", 0, 0);
}


uint32
   arad_pp_ipmc_not_found_proc_get(
     int unit,
     uint8  *flood
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  /* ip mc */  
  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, PROG_FLP_IPV6MC, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);
  *flood = flp_process_tbl.compatible_mc_bridge_fallback;   
  
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ipmc_not_found_proc_update", 0, 0);
}



uint32
   arad_pp_ipv4mc_bridge_lookup_update(
     int unit,
     uint8  mode /* 0:<FID,DA>, 1:<FID,DIP>*/
   )
{
  uint32
    res,
    program_id;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
 
  /* For static, program id and app id is same */
  program_id = (mode == 0)?PROG_FLP_ETHERNET_ING_LEARN:PROG_FLP_IPV4MC_BRIDGE;
  
  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_ipv4mc_bridge_v4mc_cam_sel_id.get(unit,&cam_sel_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* update program */
  prog_selection_cam_tbl.program = program_id;

  res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ipv4mc_bridge_lookup_update", mode, 0);
}

uint32
   arad_pp_ipv4mc_bridge_lookup_get(
     int unit,
     uint8  *mode/* 0:<FID,DA>, 1:<FID,DIP>*/
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
    prog_selection_cam_tbl;
  int32 cam_sel_id;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_ipv4mc_bridge_v4mc_cam_sel_id.get(unit,&cam_sel_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  *mode  = (prog_selection_cam_tbl.program == PROG_FLP_ETHERNET_ING_LEARN)?0:1;

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ipv4mc_bridge_lookup_get", 0, 0);
}

/* programs for FCoE transit, to enable FC traps */

uint32
   arad_pp_flp_key_const_fcf_transit(
     int unit,
     int32  prog_id
   )
{
  uint32
    res, fid_ce_inst = 0;
  ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
    flp_key_construction_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_FID,0, &fid_ce_inst);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
    
  flp_key_construction_tbl.instruction_0_16b        = fid_ce_inst;
  flp_key_construction_tbl.instruction_1_16b        = ARAD_PP_CE_SA_FWD_HEADER_16_MSB;
  flp_key_construction_tbl.instruction_3_32b        = ARAD_PP_CE_SA_FWD_HEADER_32_LSB;  
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0xB /*6'b001011*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
  flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;    
  res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_key_const_fcf_transit_learn", 0, 0);
}



uint32
   arad_pp_flp_lookups_fcf_transit(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_lookups_ethernet_ing_learn ingress_learn_oppurtunistic: %h",ingress_learn_oppurtunistic);*/
  
  res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

  flp_lookups_tbl.lem_1st_lkp_valid     = (!sa_auth_enabled && !slb_enabled)? 1 : 0 ;
  flp_lookups_tbl.lem_1st_lkp_key_select = 0;
  flp_lookups_tbl.lem_1st_lkp_key_type   = 1;
  flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_1st_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
  flp_lookups_tbl.lem_2nd_lkp_key_select = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL; 
  flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
  flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_ETH_KEY_OR_MASK(unit);
  flp_lookups_tbl.learn_key_select   = 0;


  res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_lookups_fcf_transit", 0, 0);
}


uint32
   arad_pp_flp_process_fcf_transit(
     int unit,
     uint8 learn_enable, /* = 1*/
     uint8 sa_auth_enabled,
     uint8 slb_enabled,
     int32 prog_id
   )
{
  uint32
    res;
  uint32
    tmp;
  soc_reg_above_64_val_t
    reg_val;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
  /*ARAD_PP_FLP_INIT_PRINT_ADVANCE("arad_pp_flp_process_ethernet_ing_learn ingress_learn_enable: %h ingress_learn_oppurtunistic: %h",
                          ingress_learn_enable,ingress_learn_oppurtunistic);*/

  res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  flp_process_tbl.include_lem_2nd_in_result_a    = 1;
  flp_process_tbl.include_lem_1st_in_result_b    = 1;
  flp_process_tbl.result_a_format            = 0;
  flp_process_tbl.result_b_format            = 0;
  flp_process_tbl.sa_lkp_result_select         = 0;
  flp_process_tbl.sa_lkp_process_enable        = (!sa_auth_enabled && !slb_enabled)? 1 : 0;
  flp_process_tbl.procedure_ethernet_default  = 3;
  flp_process_tbl.enable_hair_pin_filter       = 1;
  flp_process_tbl.learn_enable               = learn_enable;
  flp_process_tbl.not_found_trap_strength      = 7;
  flp_process_tbl.unknown_address              = 3;
  flp_process_tbl.not_found_trap_code          = SOC_PPC_TRAP_CODE_INTERNAL_LLR_ACCEPTABLE_FRAME_TYPE0;

  res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
  SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

  tmp = 4; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
  SOC_REG_ABOVE_64_CLEAR(reg_val);
  res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
  SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
  res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

exit:
 SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_process_fcf_transit", 0, 0);
}


/* init VPLSoGRE program */

STATIC uint32
   arad_pp_flp_pwe_gre_progs_init(
     int unit
   )
{
  uint32
    res;
  int prog;
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);


  res = arad_pp_flp_prog_sel_cam_pwe_gre(unit,&prog);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  res = arad_pp_flp_key_const_pwe_gre(unit, FALSE, FALSE);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  res = arad_pp_flp_lookups_pwe_gre(unit,prog);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  res = arad_pp_flp_process_pwe_gre(unit,prog);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_pwe_gre_progs_init", 0, 0);    
}

/* init all programs related to FCoE */
STATIC uint32
   arad_pp_flp_fcoe_progs_init(
     int unit
   )
{
    uint32
      res;
    int progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS];
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);


    /*has to be in this order: with vft[1] no vft[0] remote[2] remote with vft[3] */ 
    res = arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft(unit,&progs[1], FALSE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 3, exit);
    res = arad_pp_flp_prog_sel_cam_fcoe_fcf(unit,&progs[0],FALSE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 2, exit);
    res = arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft_remote(unit,&progs[3]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
    res = arad_pp_flp_prog_sel_cam_fcoe_fcf_remote(unit,&progs[2]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 4, exit);

    /* Lookup has to be before key construction because we are updating LPM lookup in jericho */
    res = arad_pp_flp_lookups_fcf(unit,progs, TRUE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);

    /* key const for all FCF programs */    
    res = arad_pp_flp_key_const_fcoe_fcf(unit,progs, FALSE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 6, exit);    
    
    res = arad_pp_flp_process_fcoe_fcf(unit,progs);
    SOC_SAND_CHECK_FUNC_RESULT(res, 8, exit);

    /* transit switch/bridging*/
    /* do ethernet for FCoE packet (when not doing FCF) */
    res = arad_pp_flp_prog_sel_cam_fcoe_ethernet_ing_learn(unit,&progs[0], TRUE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    /* similar to bridging: except enable FC traps and disable learning */
    res = arad_pp_flp_key_const_fcf_transit(unit,progs[0]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);
    /* no learning in this program */
    res = arad_pp_flp_lookups_fcf_transit(unit, 0,0,progs[0]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
    res = arad_pp_flp_process_fcf_transit(unit, 0,0,0,progs[0]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);

  exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_fcoe_progs_init", 0, 0);
}


uint32
   arad_pp_flp_npv_programs_init(
     int unit
   )
{
    uint32
      res;
    int progs[2];
    int is_vsan_from_vsi;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);    

    res = arad_pp_flp_fcoe_is_vsan_from_vsi_mode(unit, &is_vsan_from_vsi);
    SOC_SAND_CHECK_FUNC_RESULT(res, 1, exit);

    /* program selection for NPV has to be in this order: with vft[1] no vft[0]*/
    res = arad_pp_flp_prog_sel_cam_fcoe_fcf_with_vft(unit, &progs[1], TRUE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 2, exit);

    res = arad_pp_flp_prog_sel_cam_fcoe_fcf(unit,&progs[0], TRUE);
    SOC_SAND_CHECK_FUNC_RESULT(res, 3, exit);

    /* key const for all NPV programs */
    res = arad_pp_flp_key_const_fcoe_fcf_npv(unit,progs, is_vsan_from_vsi);
    SOC_SAND_CHECK_FUNC_RESULT(res, 6, exit);

    res = arad_pp_flp_lookups_fcf_npv(unit,progs); /* change the key mask */
    SOC_SAND_CHECK_FUNC_RESULT(res, 7, exit);

    if (SOC_IS_JERICHO(unit)) {
        int is_table_initiated = 0;
        res = arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, &is_table_initiated);
        SOC_SAND_CHECK_FUNC_RESULT(res, 8, exit);

        if (!is_table_initiated) {
            res = arad_pp_flp_dbal_fcoe_npv_program_tables_init(unit, is_vsan_from_vsi, progs[0], progs[1]);
            SOC_SAND_CHECK_FUNC_RESULT(res, 9, exit);
        }        
    }

    res = arad_pp_flp_process_fcoe_fcf_npv(unit,progs);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_n_port_programs_init", 0, 0);
}

uint32
   arad_pp_flp_n_port_programs_disable(
     int unit
   )
{
    uint32
        prog_id,
        res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_WITH_VFT_N_PORT, &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
    res = arad_pp_flp_program_disable(unit,prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_N_PORT, &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);
    res = arad_pp_flp_program_disable(unit,prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_n_port_programs_init", 0, 0);
}

uint32
arad_pp_flp_fcoe_zoning_set(
     int unit,
     int is_npv
   )
{
    uint32
      res;
    int progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS];
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA   flp_lookups_tbl;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /*has to be in this order: with vft[1] no vft[0] remote[2] remote with vft[3] */ 

    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC, (uint32*)&(progs[0]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_WITH_VFT, (uint32*)&(progs[1]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_REMOTE, (uint32*)&(progs[2]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_WITH_VFT_REMOTE, (uint32*)&(progs[3]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);    

    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[0], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    flp_lookups_tbl.lem_1st_lkp_valid      = (!is_npv);
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[0], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    
  	res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[1], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    flp_lookups_tbl.lem_1st_lkp_valid      = (!is_npv);
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[1], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[2], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    flp_lookups_tbl.lem_1st_lkp_valid      = (!is_npv);
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[2], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[3], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    flp_lookups_tbl.lem_1st_lkp_valid      = (!is_npv);
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, progs[3], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_fcoe_zoning_set", 0, 0);
}


uint32
arad_pp_flp_fcoe_vsan_mode_set(
     int unit,
     int is_vsan_from_vsi
   )
{
    uint32
      res;
    int progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS];
    int is_zoning_enabled;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /*has to be in this order: with vft[1] no vft[0] remote[2] remote with vft[3] */ 

    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC, (uint32*)&(progs[0]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_WITH_VFT, (uint32*)&(progs[1]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_REMOTE, (uint32*)&(progs[2]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_WITH_VFT_REMOTE, (uint32*)&(progs[3]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = arad_pp_flp_key_const_fcoe_fcf(unit,progs, is_vsan_from_vsi);
    SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);
    /* if zoning is disabled than it means we are in NPV mode */
    res = arad_pp_flp_fcoe_is_zoning_enabled(unit, &is_zoning_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    if (!is_zoning_enabled) {
        res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_N_PORT, (uint32*)&(progs[0]));
        SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
        res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC_WITH_VFT_N_PORT, (uint32*)&(progs[1]));
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

        res = arad_pp_flp_key_const_fcoe_fcf_npv(unit,progs, is_vsan_from_vsi);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);        
    }

    if (SOC_IS_JERICHO(unit)) {
        int is_table_initiated = 0;
        res = arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, &is_table_initiated);
        SOC_SAND_CHECK_FUNC_RESULT(res, 8, exit);

        /* if the table exists, we need to update the mode of the table */
        if (is_table_initiated) {
            res = arad_pp_flp_dbal_fcoe_npv_program_tables_init(unit, is_vsan_from_vsi, progs[0], progs[1]);
            SOC_SAND_CHECK_FUNC_RESULT(res, 9, exit);
        }        
    }    

    exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_fcoe_vsan_mode_set", 0, 0);
}

uint32
arad_pp_flp_fcoe_is_zoning_enabled(
     int unit,
     int* is_enabled
   )
{
    uint32
      res;
    int progs[ARAD_PP_FLP_NUMBER_OF_FCOE_FCF_PROGRAMS];
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA    flp_lookups_tbl;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC, (uint32*)&(progs[0]));
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, progs[0], &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    (*is_enabled) = flp_lookups_tbl.lem_1st_lkp_valid;

    exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_fcoe_is_zoning_enabled", 0, 0);

}


uint32
arad_pp_flp_fcoe_is_vsan_from_vsi_mode(
     int unit,
     int* is_vsan_from_vsi
   )
{
    uint32
        prog,
        res;
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA flp_key_construction_tbl;  

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_FC, &prog);
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    if(flp_key_construction_tbl.instruction_0_16b == ARAD_PP_FLP_16B_INST_P6_VSI(12)){
        (*is_vsan_from_vsi) = TRUE;
    }else{
        (*is_vsan_from_vsi) = FALSE;
    }

    exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_fcoe_is_vsan_from_vsi_mode", 0, 0);
}




/* init all programs related to IPMC-BIDIR */
STATIC uint32
   arad_pp_flp_ipmc_bidir_progs_init(
     int unit
   )
{
    uint32 vrf_ce_inst = 0, rif_ce_inst = 0,
      res;
    ARAD_PP_IHB_FLP_PROGRAM_SELECTION_CAM_TBL_DATA
      prog_selection_cam_tbl;
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA
      flp_key_construction_tbl;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
      flp_lookups_tbl;
    ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
      flp_process_tbl;
    uint32
      tmp;
    soc_reg_above_64_val_t
      reg_val;
    int32  prog_id;
    int32 cam_sel_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF,0, &vrf_ce_inst);
    arad_pp_dbal_qualifier_to_instruction(unit,SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF,0, &rif_ce_inst);

    /* program selection */
    /* allocate resources */
    res = arad_pp_flp_set_app_id_and_get_cam_sel(unit,PROG_FLP_BIDIR,FALSE,TRUE,&cam_sel_id, &prog_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* configure program selection */
    res = arad_pp_ihb_flp_program_selection_cam_tbl_get_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    prog_selection_cam_tbl.tt_lookup_0_found = 0x1;
    prog_selection_cam_tbl.tt_lookup_0_found_mask = 0x0;
    prog_selection_cam_tbl.vt_lookup_1_found = 0x0;
    prog_selection_cam_tbl.tt_lookup_0_found_mask = 0x0;
    prog_selection_cam_tbl.forwarding_code  = ARAD_PP_FWD_CODE_IPV4_MC;
    prog_selection_cam_tbl.forwarding_code_mask  = 0x0;
    prog_selection_cam_tbl.program = prog_id;
    prog_selection_cam_tbl.valid = 1;

    res = arad_pp_ihb_flp_program_selection_cam_tbl_set_unsafe(unit, cam_sel_id, &prog_selection_cam_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* key construction */
    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    flp_key_construction_tbl.instruction_0_16b        = ARAD_PP_FLP_16B_INST_P6_TT_LOOKUP0_PAYLOAD_D_BITS(8); /* RPA-ID */
    flp_key_construction_tbl.instruction_1_16b        = rif_ce_inst; /* in-RIF */
    flp_key_construction_tbl.key_a_inst_0_to_5_valid    = 0x3 /*6'b000011*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid    = 0x0 /*6'b000000*/;
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);

    res = arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);
    flp_key_construction_tbl.instruction_0_16b     = vrf_ce_inst;
    flp_key_construction_tbl.instruction_3_32b     = ARAD_PP_CE_DIP_FWD_HEADER;
    flp_key_construction_tbl.key_a_inst_0_to_5_valid = 0x0 /*6'b000000*/;
    flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x9 /*6'b001001*/;
    flp_key_construction_tbl.key_c_inst_0_to_5_valid = 0x0 /*6'b000000*/;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
        /* Insert also the SIP in Key-B to get the same opcode than IPv4 UC without RPF */
        flp_key_construction_tbl.instruction_3_32b  = ARAD_PP_CE_SIP_FWD_HEADER;
        flp_key_construction_tbl.instruction_4_32b  = ARAD_PP_CE_DIP_FWD_HEADER;
        flp_key_construction_tbl.key_b_inst_0_to_5_valid = 0x19 /*6'b011001*/;
    }
#endif
    res = arad_pp_ihb_flp_key_construction_tbl_set_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_construction_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

    /* reference: PROG_FLP_IPV4UC_WITH_RPF */
    /* flp lookups */
    res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    flp_lookups_tbl.lem_1st_lkp_valid     = 1;
    flp_lookups_tbl.lem_1st_lkp_key_select = 0;
    flp_lookups_tbl.lem_1st_lkp_key_type   = 0;
    flp_lookups_tbl.lpm_1st_lkp_valid     = 0;
    flp_lookups_tbl.lpm_1st_lkp_key_select = 0;
    flp_lookups_tbl.lpm_1st_lkp_and_value = 0;
    flp_lookups_tbl.lpm_1st_lkp_or_value = 0;
    flp_lookups_tbl.lem_1st_lkp_and_value  = 0x0;
    /* allocate prefix */
    res =  arad_pp_lem_access_app_to_prefix_get(unit,ARAD_PP_FLP_IPMC_BIDIR_KEY_OR_MASK,&flp_lookups_tbl.lem_1st_lkp_or_value);
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
    flp_lookups_tbl.lem_2nd_lkp_valid     = 1;
    flp_lookups_tbl.lem_2nd_lkp_key_select = 1;
    flp_lookups_tbl.lem_2nd_lkp_and_value  = 0x0;
    flp_lookups_tbl.lem_2nd_lkp_or_value   = ARAD_PP_FLP_IPV4_KEY_OR_MASK; /* forwarding is with IP prefix, but DIP value is MC --> no conflict*/
    flp_lookups_tbl.lpm_2nd_lkp_valid     = 0;
    flp_lookups_tbl.lpm_2nd_lkp_key_select = 0;
    flp_lookups_tbl.lpm_2nd_lkp_and_value = 0;
    flp_lookups_tbl.lpm_2nd_lkp_or_value = 0;
    flp_lookups_tbl.learn_key_select      = ARAD_PP_FLP_LKP_KEY_SELECT_FID_FWD_MAC_KEY_HW_VAL;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
        flp_lookups_tbl.elk_lkp_valid = 0x1;
        flp_lookups_tbl.elk_wait_for_reply = 0x1;
        flp_lookups_tbl.elk_opcode = ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_UC;

        flp_lookups_tbl.elk_key_a_valid_bytes = 0x0;
        flp_lookups_tbl.elk_key_b_valid_bytes = 10; /* VRF, SIP, DIP */
        flp_lookups_tbl.elk_key_c_valid_bytes = 0x0;
    }
#endif
    res = arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

    /* flp process */


    res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    flp_process_tbl.include_lem_2nd_in_result_a    = 1;
    flp_process_tbl.include_lem_1st_in_result_b    = 1;
    flp_process_tbl.include_lpm_2nd_in_result_a    = 0;
    flp_process_tbl.include_lpm_1st_in_result_b    = 0;
    flp_process_tbl.result_a_format            = 0;
    flp_process_tbl.result_b_format            = 0;
    flp_process_tbl.procedure_ethernet_default  = 3;
    flp_process_tbl.enable_hair_pin_filter       = 1;
    flp_process_tbl.enable_rpf_check            = 0;
    flp_process_tbl.learn_enable = 1;
    /* take VRF default destination */
    flp_process_tbl.select_default_result_a = 1;
    flp_process_tbl.compatible_mc_bridge_fallback = 1;
    flp_process_tbl.select_default_result_b = 0;
    flp_process_tbl.apply_fwd_result_a           = 1; /*  apply alwayes, even if not found, i.e. do Trap if not found */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if(ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED){
      flp_process_tbl.elk_result_format = 1;
      flp_process_tbl.include_elk_fwd_in_result_a = 1;
      flp_process_tbl.include_elk_ext_in_result_a = 0;
      flp_process_tbl.include_elk_fwd_in_result_b = 0;
      flp_process_tbl.include_elk_ext_in_result_b = 0;
  }
#endif
    res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, prog_id, &flp_process_tbl);
    SOC_SAND_CHECK_FUNC_RESULT(res, 41, exit);
    tmp = 1; /* 0: Apply Ethernet traps on forwarding-header 1: Apply IPv4 traps on forwarding-header and L4 Traps on the following header 2: Apply IPv6 traps on forwarding-header and L4 Traps on the following header 3: Apply MPLS traps on forwarding-header 4: Apply FC traps on forwarding-header Else: Don't apply any protocol traps */
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    res = READ_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, 0, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    SHR_BITCOPY_RANGE(reg_val,3*prog_id,&tmp,0,3);
    res = WRITE_IHP_PROTOCOL_TRAPS_PROGRAM_SELECTr(unit, SOC_CORE_ALL, reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

  exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_ipmc_bidir_progs_init", 0, 0);
}

/* init all programs related to vmac */
STATIC uint32
   arad_pp_flp_vmac_progs_init(
     int unit,
     uint8  sa_auth_enabled,
     uint8  slb_enabled
   )
{
    uint32
      res;
    int progs[2];
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_pp_flp_prog_sel_cam_pon_vmac_upstream(unit, &progs[0]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    res = arad_pp_flp_key_const_pon_vmac_upstream(unit, progs[0]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    res = arad_pp_flp_lookups_pon_vmac_upstream(unit, 
                                                progs[0],
                                                sa_auth_enabled,
                                                slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    res = arad_pp_flp_process_pon_vmac_upstream(unit, 
                                                progs[0],
                                                TRUE,
                                                sa_auth_enabled,
                                                slb_enabled);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    
    res = arad_pp_flp_prog_sel_cam_pon_vmac_downstream(unit, &progs[1]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    res = arad_pp_flp_key_const_pon_vmac_downstream(unit, progs[1]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    res = arad_pp_flp_lookups_pon_vmac_downstream(unit, progs[1]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

    res = arad_pp_flp_process_pon_vmac_downstream(unit, progs[1]);
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_vmac_progs_init", 0, 0);
}

STATIC uint32
   arad_pp_flp_pon_arp_prog_init(
     int unit,
     uint8 sa_auth_enabled,
     uint8 slb_enabled
   )
{
    uint32
        res;
        int prog_id;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (SOC_DPP_CONFIG(unit)->pp.l3_src_bind_arp_relay & SOC_DPP_L3_SRC_BIND_FOR_ARP_RELAY_DOWN) {
        res = arad_pp_flp_prog_sel_cam_pon_arp_downstream(unit, &prog_id);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if(SOC_IS_JERICHO(unit)){
            res = arad_pp_flp_dbal_pon_sav_arp_program_tables_init(unit, FALSE);
            SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);
        } else {
            res = arad_pp_flp_key_const_pon_arp_downstream(unit, prog_id);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
            res = arad_pp_flp_lookups_pon_arp_downstream(unit, prog_id);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
        }
        res = arad_pp_flp_process_pon_arp_downstream(unit, prog_id);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }

    if (SOC_DPP_CONFIG(unit)->pp.l3_src_bind_arp_relay & SOC_DPP_L3_SRC_BIND_FOR_ARP_RELAY_UP) {
        prog_id = 0;
        res = arad_pp_flp_prog_sel_cam_pon_arp_upstream(unit, &prog_id);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        if(SOC_IS_JERICHO(unit)){
            res = arad_pp_flp_dbal_pon_sav_arp_program_tables_init(unit, TRUE);
            SOC_SAND_CHECK_FUNC_RESULT(res, 51, exit);
        } else {
            res = arad_pp_flp_key_const_pon_arp_upstream(unit, prog_id);
            SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
            res = arad_pp_flp_lookups_pon_arp_upstream(unit, sa_auth_enabled,slb_enabled, prog_id);
            SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
        }

        res = arad_pp_flp_process_pon_arp_upstream(unit, TRUE, sa_auth_enabled,slb_enabled, prog_id);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }


exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_pon_arp_prog_init", 0, 0);
}



char*
  arad_pp_flp_prog_id_to_prog_name(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint8 prog_id
)
{
    switch(prog_id)
    {
    case PROG_FLP_TM:
      return "PROG_FLP_TM";
    case PROG_FLP_ETHERNET_ING_LEARN:
      return "PROG_FLP_ETHERNET_ING_LEARN";
    case PROG_FLP_IPV4UC_PUBLIC_RPF:
      return "PROG_FLP_IPV4UC_PUBLIC_RPF";
    case PROG_FLP_IPV6UC_PUBLIC_RPF:
      return "PROG_FLP_IPV6UC_PUBLIC_RPF";
    case PROG_FLP_IPV6UC_PUBLIC:
      return "PROG_FLP_IPV6UC_PUBLIC";
    case PROG_FLP_IPV4UC_PUBLIC:
      return "PROG_FLP_IPV4UC_PUBLIC";
    case PROG_FLP_IPV4UC_RPF:
      return "PROG_FLP_IPV4UC_RPF";
    case PROG_FLP_IPV4UC:
      return "PROG_FLP_IPV4UC";
    case PROG_FLP_IPV6UC:
      return "PROG_FLP_IPV6UC";
    case PROG_FLP_P2P:
      return "PROG_FLP_P2P";
    case PROG_FLP_IPV6MC:
      return "PROG_FLP_IPV6MC";
    case PROG_FLP_LSR:
      return "PROG_FLP_LSR";
    case PROG_FLP_TRILL_UC:
      if (SOC_DPP_CONFIG(unit)->trill.mode) { 
        return "PROG_FLP_TRILL_UC";
      } else {
        return "PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC";
      }
    case PROG_FLP_TRILL_MC_ONE_TAG:
      if (SOC_DPP_CONFIG(unit)->trill.mode) { 
        return "PROG_FLP_TRILL_MC_ONE_TAG";
      } else {
        return "PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC";
      }
#if 1
    case PROG_FLP_TRILL_MC_TWO_TAGS:
      if (SOC_DPP_CONFIG(unit)->trill.mode) { 
        return "PROG_FLP_TRILL_MC_TWO_TAGS";
      } else {
        return "PROG_FLP_MAC_IN_MAC_AFTER_TERMINATIOM";
      }
#endif
    case PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP /*or PROG_FLP_VPWS_TAGGED_SINGLE_TAG*/:
        return (SOC_DPP_CONFIG(unit)->pp.pon_application_enable ? "PROG_FLP_ETHERNET_TK_EPON_UNI_V4_DHCP" : "PROG_FLP_VPWS_TAGGED_SINGLE_TAG");
    case PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP /*or PROG_FLP_VPWS_TAGGED_DOUBLE_TAG*/:
        return (SOC_DPP_CONFIG(unit)->pp.pon_application_enable ? "PROG_FLP_ETHERNET_TK_EPON_UNI_V6_DHCP" : "PROG_FLP_VPWS_TAGGED_DOUBLE_TAG");
    case PROG_FLP_ETHERNET_MAC_IN_MAC:
      return "PROG_FLP_ETHERNET_MAC_IN_MAC";
    case PROG_FLP_TRILL_AFTER_TERMINATION:
        if (SOC_DPP_CONFIG(unit)->pp.custom_ip_route) {
            return "PROG_FLP_IP4UC_CUSTOM_ROUTE"; 
        } else {
            return "PROG_FLP_TRILL_AFTER_TERMINATION"; 
        }
    case PROG_FLP_IPV4MC_BRIDGE:
      return "PROG_FLP_IPV4MC_BRIDGE";
    case PROG_FLP_IPV4COMPMC_WITH_RPF:
      return "PROG_FLP_IPV4COMPMC_WITH_RPF";
    case PROG_FLP_FC_WITH_VFT_N_PORT:
      return "PROG_FLP_FC_WITH_VFT_N_PORT";
    case PROG_FLP_FC_N_PORT:
      return "PROG_FLP_FC_N_PORT";
    case PROG_FLP_FC_REMOTE:
      return "PROG_FLP_FC_REMOTE";
    case PROG_FLP_FC_WITH_VFT_REMOTE:
      return "PROG_FLP_FC_WITH_VFT_REMOTE";
    case PROG_FLP_FC:
      return "PROG_FLP_FC";
    case PROG_FLP_FC_WITH_VFT:
      return "PROG_FLP_FC_WITH_VFT";
    case PROG_FLP_BIDIR:
      return "PROG_FLP_BIDIR";
    case PROG_FLP_IPV6UC_RPF:
      return "PROG_FLP_IPV6UC_RPF";
    case PROG_FLP_VMAC_UPSTREAM:
      return "PROG_FLP_VMAC_UPSTREAM";
    case PROG_FLP_VMAC_DOWNSTREAM:
      return "PROG_FLP_VMAC_DOWNSTREAM";
    case PROG_FLP_VPLSOGRE:
      return "PROG_FLP_VPLSOGRE";
    case PROG_FLP_ETHERNET_PON_LOCAL_ROUTE:
      return "PROG_FLP_ETHERNET_PON_LOCAL_ROUTE";
    case PROG_FLP_ETHERNET_PON_DEFAULT_UPSTREAM:
      return "PROG_FLP_ETHERNET_PON_DEFAULT_UPSTREAM";
    case PROG_FLP_ETHERNET_PON_DEFAULT_DOWNSTREAM:
      return "PROG_FLP_ETHERNET_PON_DEFAULT_DOWNSTREAM";
    case PROG_FLP_GLOBAL_IPV4COMPMC_WITH_RPF:
      return "PROG_FLP_GLOBAL_IPV4COMPMC_WITH_RPF";
    case PROG_FLP_PON_ARP_DOWNSTREAM:
      return "PROG_FLP_PON_ARP_DOWNSTREAM";
    case PROG_FLP_PON_ARP_UPSTREAM:
      return "PROG_FLP_PON_ARP_UPSTREAM";
    case PROG_FLP_FC_TRANSIT:
        return "PROG_FLP_FC_TRANSIT";
    case PROG_FLP_IPV6UC_WITH_RPF_2PASS:
        return "PROG_FLP_IPV6UC_WITH_RPF_2PASS";
    case PROG_FLP_ETHERNET_ING_IVL_LEARN:
        return "PROG_FLP_ETHERNET_ING_IVL_LEARN";
    case PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN:
        return "PROG_FLP_ETHERNET_ING_IVL_INNER_LEARN";
    case PROG_FLP_ETHERNET_ING_IVL_FWD_OUTER_LEARN:
        return "PROG_FLP_ETHERNET_ING_IVL_FWD_OUTER_LEARN";
    case PROG_FLP_BFD_IPV4_SINGLE_HOP:
        return "PROG_FLP_BFD_IPV4_SINGLE_HOP";
	case PROG_FLP_BFD_IPV6_SINGLE_HOP:
        return "PROG_FLP_BFD_IPV6_SINGLE_HOP";
	case PROG_FLP_OAM_STATISTICS:
        return "PROG_FLP_OAM_STATISTICS";
    case PROG_FLP_OAM_DOWN_UNTAGGED_STATISTICS:
        return "PROG_FLP_OAM_DOWN_UNTAGGED_STATISTICS";
    case PROG_FLP_OAM_SINGLE_TAG_STATISTICS:
        return "PROG_FLP_OAM_SINGLE_TAG_STATISTICS";
	case PROG_FLP_OAM_DOUBLE_TAG_STATISTICS:
        return "PROG_FLP_OAM_DOUBLE_TAG_STATISTICS";
    case PROG_FLP_BFD_STATISTICS:
        return "PROG_FLP_BFD_STATISTICS";
    case PROG_FLP_BFD_MPLS_STATISTICS:
        return "PROG_FLP_BFD_MPLS_STATISTICS";
    case PROG_FLP_BFD_PWE_STATISTICS:
        return "PROG_FLP_BFD_PWE_STATISTICS";
    case PROG_FLP_IPV4_DC:
        return "PROG_FLP_IPV4_DOUBLE_CAPACITY";
    default:
      return "Unknown program";
    }
}

STATIC uint32
  arad_pp_flp_program_info_cam_selection_dump(
    SOC_SAND_IN int unit,
    SOC_SAND_IN int32 prog_id
  )
{
    int i;
    uint32 res;
    uint64 cam_sel_list;
    int nof_cam_sel=0;
    int nof_flp_selections  = SOC_DPP_DEFS_GET(unit, nof_flp_program_selection_lines);
    uint8 first_printed = FALSE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    COMPILER_64_ZERO(cam_sel_list);

    res = arad_pp_flp_get_cam_sel_list_by_prog_id(unit,prog_id,&cam_sel_list,&nof_cam_sel);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    LOG_CLI((BSL_META_U(unit,"Prog. Selections IDs [%d selection(s)]: "),nof_cam_sel));

    if (nof_cam_sel > 0) {
        for (i=0 ; i < nof_flp_selections ; i++) {
            if (COMPILER_64_BITTEST(cam_sel_list,i)) {
                if (first_printed) {
                    LOG_CLI((BSL_META_U(unit, ", ")));
                }
                LOG_CLI((BSL_META_U(unit, "%d"),i));
                first_printed=TRUE;
            }
        }
    }
    else{
        LOG_CLI((BSL_META_U(unit, "None")));
    }
    LOG_CLI((BSL_META_U(unit, "\n")));

exit:
   SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_program_info_cam_selection_dump", prog_id, 0);
}

uint32
  arad_pp_flp_access_print_all_programs_data(
    SOC_SAND_IN  int                                 unit
  )
{
    uint8  cur_prog_id;
    uint32 prog_indx;
    char*  prog_name;
    uint32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    LOG_CLI((BSL_META_U(unit,
                        "\n\tVALID FLP PROGRAMS:\n")));
    LOG_CLI((BSL_META("\n----------------------------------------------------------------------------------------------------------------------------------------------------------\n\n\n")));
    for (prog_indx = 0; prog_indx < SOC_DPP_DEFS_GET(unit, nof_flp_programs); prog_indx++) {
        res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit,prog_indx,&cur_prog_id);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

        prog_name = arad_pp_flp_prog_id_to_prog_name(unit, cur_prog_id);
        
        if (cur_prog_id != ARAD_PP_FLP_MAP_PROG_NOT_SET) {        
          LOG_CLI((BSL_META_U(unit,"Program %s ID (%d)\n"), prog_name, prog_indx));
          arad_pp_flp_program_info_cam_selection_dump(unit, prog_indx);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
          arad_pp_flp_dbal_program_info_dump(unit, prog_indx);
        }
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("arad_pp_flp_access_print_all_programs_data", 0, 0);
}

/*********************************************************************
*     Dump last FLP program invoked.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_flp_access_print_last_programs_data(
    SOC_SAND_IN   int                 unit,
    SOC_SAND_IN   int                 core_id,
    SOC_SAND_IN   int                 to_print,
    SOC_SAND_OUT  int                 *prog_id,
    SOC_SAND_OUT  int                 *num_of_progs

  )
{
    uint32 fld_val, progs_bmp, pgm_ndx;
    uint32 res = SOC_SAND_OK;
    uint8  cur_prog_id;
    char*  prog_name;
    uint8  found = FALSE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* 
     * 1. Read and print all invoked programs
     * 2. Clear the register
     */
    *prog_id = -1;
    (*num_of_progs) = 0;
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHP_DBG_FLP_SELECTED_PROGRAMr, core_id, 0, DBG_FLP_SELECTED_PROGRAMf, &progs_bmp));
    if (to_print) {
        LOG_CLI((BSL_META_U(unit,
                            "Last invoked FLP program:\n")));
    }

    for (pgm_ndx = 0; pgm_ndx < SOC_DPP_DEFS_GET(unit, nof_flp_programs); pgm_ndx++)
    {
        if (SOC_SAND_GET_BIT(progs_bmp, pgm_ndx))
        {
            /* Program found */
            res = sw_state_access[unit].dpp.soc.arad.pp.fec.flp_progs_mapping.get(unit,pgm_ndx,&cur_prog_id);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

            *prog_id = pgm_ndx;
            (*num_of_progs) ++;

            prog_name = arad_pp_flp_prog_id_to_prog_name(unit, cur_prog_id);

            if (cur_prog_id != ARAD_PP_FLP_MAP_PROG_NOT_SET)
            {
                found = TRUE;
                if(to_print){
                    LOG_CLI((BSL_META_U(unit,"Program %s ID: %d\n"), prog_name, pgm_ndx));    
                }
            }
        }
    }

    if (!found) 
    {
        if (to_print) {
            LOG_CLI((BSL_META_U(unit,"No program invoked\n")));
        }
    }

    /* for the future. If the length of the field DBG_FLP_SELECTED_PROGRAMf will be more then 32 bit - this check will force to change the code */
    if(SOC_DPP_DEFS_GET(unit, nof_flp_programs) > 32) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 15, exit);
    }

    /* Set all bits as ones to clear it.
     */
    if(SOC_DPP_DEFS_GET(unit, nof_flp_programs) < 32) {
        fld_val = (1 << SOC_DPP_DEFS_GET(unit, nof_flp_programs)) - 1;
    } else {
        fld_val = 0xffffffff;
    }
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_DBG_FLP_SELECTED_PROGRAMr, core_id, 0, DBG_FLP_SELECTED_PROGRAMf,  fld_val));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_flp_access_print_last_programs_data()", 0, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

