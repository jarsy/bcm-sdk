#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_fp_key.c,v 1.105 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_FP

#include <shared/bsl.h>
/*************
 * INCLUDES  *
 *************/
/* { */

#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>


#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_key.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_fem.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/ARAD/arad_tcam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#endif

/* } */
/*************
 * DEFINES   *
 *************/
/* { */





#define ARAD_PP_FP_KEY_SWAP_PRIORITY_FIRST                  (100)
#define ARAD_PP_FP_KEY_SWAP_PRIORITY_SECOND                 (90)
#define ARAD_PP_FP_KEY_SWAP_PRIORITY_THIRD                  (80)
#define ARAD_PP_FP_KEY_SWAP_PRIORITY_FOURTH                 (70)
#define ARAD_PP_FP_KEY_SWAP_PRIORITY_FIFTH                  (60)

/* 8 categories of Copy Engines: 16/32, LSB/MSB, sub-group 0/1*/
#define ARAD_PP_FP_KEY_NOF_CATEGORIES (8)
#define ARAD_PP_FP_KEY_CATEGORY_GET(is_32b, is_msb, is_high_group) (((is_32b) << 2) + ((is_msb) << 1) + (is_high_group))


/* Cascaded Key <-> real key translation. 0 is reserved to non-assigned */
#define ARAD_PP_FP_KEY_FROM_CASCADED_KEY_GET(cascaded_key) (cascaded_key - 1)
#define ARAD_PP_FP_KEY_TO_CASCADED_KEY_GET(real_key) ((uint8) (real_key + 1))

/* Number of 80b parts in 320b key */
#define ARAD_PP_FP_KEY_NUMBER_80_PARTS_IN_320B (4)
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

/* CE can be 16 bits or 32 bits */
typedef enum 
{
    ARAD_PP_FP_QUAL_CE_16_BITS = 0,
    ARAD_PP_FP_QUAL_CE_32_BITS = 1,
    ARAD_PP_FP_QUAL_NOF_CE_SIZES = 2
} ARAD_PP_FP_QUAL_CE_SIZES;

typedef enum 
{
    ARAD_PP_FP_KEY_BUFF_LSB = 0,
    ARAD_PP_FP_KEY_BUFF_MSB = 1,
    ARAD_PP_FP_KEY_NOF_BUFFERS = 2
} ARAD_PP_FP_KEY_BUFF_TYPES;

typedef struct
{
    uint8   is_header;
    uint8   is_msb;
    uint8   is_lsb;
    uint8   nof_ce;
    uint8   place_cons[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint8   group_cons[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint8   size_cons[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint32  ce_zone_id[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint32  ce_nof_bits[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint32  ce_qual_lsb[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint32  ce_lost_bits[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS];
    uint8   qual_size; /* Real number of bits */
    uint8   lost_bits; /* Lost bits in best case */
    uint8   lost_bits_worst; /* Lost bits in worst case, in condition no split is performed, in which case more lest bits can be added */

} ARAD_PP_FP_QUAL_CE_INFO;


typedef enum 
{
    ARAD_PP_FP_VT_MATCH_MODE_IPV4 = 0,
    ARAD_PP_FP_VT_MATCH_MODE_EFP = 1,
    ARAD_PP_FP_QUAL_NOF_MATCH_MODES = 2
} ARAD_PP_FP_VT_MATCH_MODES;


typedef struct
{
    uint8 balance_enabled;
    uint8 balance_lsb_msb;

} ARAD_PP_FP_ALLOC_ALG_ARGS;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC
  SOC_PROCEDURE_DESC_ELEMENT
    Arad_pp_procedure_desc_element_fp_key[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_LENGTH_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_LENGTH_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_LENGTH_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_LENGTH_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_ALLOC),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_ALLOC_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_ALLOC_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_ALLOC_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_FREE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_FREE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_FREE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_SPACE_FREE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_DESC_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_DESC_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_DESC_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_DESC_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_HEADER_NDX_FIND),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_NOF_VLAN_TAGS_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_TOTAL_SIZE_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_CE_INSTR_BOUNDS_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_CE_INSTR_INSTALL),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_NEW_DB_POSSIBLE_CONFS_GET_UNSAFE),



  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC
SOC_ERROR_DESC_ELEMENT
    Arad_pp_error_desc_element_fp_key[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'pfg' is out of range. \n\r "
    "The range is: No min - SOC_SAND_UINT_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_DB_ID_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_KEY_DB_ID_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'db_id_ndx' is out of range. \n\r "
    "The range is: No min - SOC_SAND_UINT_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_CYCLE_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_KEY_CYCLE_OUT_OF_RANGE_ERR",
    "The parameter 'cycle' is out of range. \n\r "
    "The range is: No min - SOC_SAND_UINT_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_LSB_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_KEY_LSB_OUT_OF_RANGE_ERR",
    "The parameter 'lsb' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_UINT_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_LENGTH_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_KEY_LENGTH_OUT_OF_RANGE_ERR",
    "The parameter 'length' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_UINT_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_KEY_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_KEY_KEY_OUT_OF_RANGE_ERR",
    "The parameter 'key' is out of range. \n\r "
    "The range is: 0 - ARAD_PP_NOF_FP_PMF_KEYS-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR,
    "ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR",
    "The requested qualifier is not supported.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_BAD_PADDING_ERR,
    "ARAD_PP_FP_KEY_BAD_PADDING_ERR",
    "The hardware cannot add the requested amount of padding bits.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_COPY_OVERFLOW_ERR,
    "ARAD_PP_FP_KEY_COPY_OVERFLOW_ERR",
    "Attempted read beyond the end of the requested qualifier.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_KEY_TOO_MANY_BITS_ERR,
    "ARAD_PP_FP_KEY_TOO_MANY_BITS_ERR",
    "The hardware cannot copy the requested amount of bits.\n\r"
    "The maximum number of bits per copy instruction is 32.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */



  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};


static 
    uint32
        Arad_pp_fp_place_const_convert[ARAD_PP_FP_NOF_KEY_CE_PLACES][6] = {
            /* SW Flag                      Number of keys   Key size               Number of 80s  Is-for-TCAM  Is-for-Direct-Extraction */
            {ARAD_PP_FP_KEY_CE_LOW                    , 1, ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS,   1, 1,               1},
            {ARAD_PP_FP_KEY_CE_HIGH                   , 1, ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS,  1, 0,               1},
            {ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE   , 1, ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS,  2, 1,               0},
            {ARAD_PP_FP_KEY_CE_PLACE_ANY              , 2, ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS,  4, 1,               0}
        };

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

STATIC
uint32 arad_pp_fp_kbp_match_key_ces_get(      
          SOC_SAND_IN  int                    unit,
          SOC_SAND_IN  uint32                 table_id,
          SOC_SAND_OUT ARAD_PMF_CE      *ce_array,
          SOC_SAND_OUT uint8                  *nof_ces
       )
{
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    const ARAD_PMF_CE ce_array_kbp_ipv6_match[] = {
                {1,   0, 0, 15,  0, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF},
                {1,   0, 0, 31, 0, 0, 32, SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH},               
                {1,   0, 0, 31, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH},               
                {1,   0, 0, 31, 0, 0, 32, SOC_PPC_FP_QUAL_HDR_IPV6_DIP_LOW},
                {1,   0, 0, 31, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_IPV6_DIP_LOW}
               };

    const ARAD_PMF_CE ce_array_kbp_match[] = {
                {1,   0, 0, 2, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_MPLS_EXP_FWD},
                {1,   0, 0, 19, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD},
                {1,   0, 0, 0, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_MPLS_BOS_FWD},
                {1,   0, 0, 15,  0, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF},
                {1,   0, 0, 31, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_IPV4_DIP}           
               };

    const ARAD_PMF_CE ce_array_kbp_p2p_match[] = {
                {1,   0, 0, 15, 0, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_LIF}
               };

    const ARAD_PMF_CE ce_array_kbp_rif_match[] = {
                {1,   0, 0, 15, 0, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF}
               };
	
	const ARAD_PMF_CE ce_array_kbp_ipv4_mc_match[] = {
		{1,   0, 0, 15, 0, 0, 0, SOC_PPC_FP_QUAL_IRPP_VRF},
		{1,   0, 0, 15, 0, 0, 0, SOC_PPC_FP_QUAL_IRPP_IN_RIF},
		{1,   0, 0, 31, 0, 0, 0, SOC_PPC_FP_QUAL_HDR_IPV4_DIP},
		{1,   0, 0, 31,  0, 0, 0, SOC_PPC_FP_QUAL_HDR_IPV4_SIP}
	};
	    

    switch (table_id) {
        case ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED:
            (*nof_ces) = sizeof(ce_array_kbp_match)/sizeof(ARAD_PMF_CE);
            sal_memcpy(ce_array,&ce_array_kbp_match,sizeof(ARAD_PMF_CE)* (*nof_ces) );
            break;

        case ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6:
            (*nof_ces) = sizeof(ce_array_kbp_ipv6_match)/sizeof(ARAD_PMF_CE);
            sal_memcpy(ce_array,&ce_array_kbp_ipv6_match,sizeof(ARAD_PMF_CE)* (*nof_ces) );        
            break;

        case ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P:
            (*nof_ces) = sizeof(ce_array_kbp_p2p_match)/sizeof(ARAD_PMF_CE);
            sal_memcpy(ce_array,&ce_array_kbp_p2p_match,sizeof(ARAD_PMF_CE)* (*nof_ces) );
            break;

        case ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING:
            (*nof_ces) = sizeof(ce_array_kbp_rif_match)/sizeof(ARAD_PMF_CE);
            sal_memcpy(ce_array,&ce_array_kbp_rif_match,sizeof(ARAD_PMF_CE)* (*nof_ces) );
            break;

        case ARAD_KBP_FRWRD_TBL_ID_IPV4_MC:
            (*nof_ces) = sizeof(ce_array_kbp_ipv4_mc_match)/sizeof(ARAD_PMF_CE);
            sal_memcpy(ce_array,&ce_array_kbp_ipv4_mc_match,sizeof(ARAD_PMF_CE)* (*nof_ces) );
            break;
    default:
        return -1;
    }



    return SOC_SAND_OK;
#else
    return -1;
#endif
}

STATIC
    uint32
        arad_pp_fp_key_access_profile_hw_set(
            SOC_SAND_IN  int                       unit,
            SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
            SOC_SAND_IN  uint32                       prog_ndx,
            SOC_SAND_IN  uint32                       access_profile_array_id,
            SOC_SAND_IN  uint32                       access_profile_id,
            SOC_SAND_IN  ARAD_PP_FP_KEY_ALLOC_INFO   *alloc_info
        )
{
    uint32
      tbl_data[2];
    soc_mem_t
        ce_table;
    uint8
        cycle;
    soc_field_t
        fields[] = {TCAM_DB_PROFILE_KEY_Af,TCAM_DB_PROFILE_KEY_Bf,TCAM_DB_PROFILE_KEY_Cf,TCAM_DB_PROFILE_KEY_Df};
    soc_field_t
        qax_fields[] = {TCAM_DB_PROFILE_KEY_0f,TCAM_DB_PROFILE_KEY_1f,TCAM_DB_PROFILE_KEY_2f,TCAM_DB_PROFILE_KEY_3f};
    uint32
        access_profile_id_lcl,
      res = SOC_SAND_OK;
    soc_reg_t
        egq_key_data_base_profile[2] = {EGQ_KEYA_DATA_BASE_PROFILEr, EGQ_KEYB_DATA_BASE_PROFILEr};
    soc_reg_above_64_val_t
            reg_above_64;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64);

    /* Set in the HW the access profile - according to the stage  - can be a separate function */
    if (stage ==  SOC_PPC_FP_DATABASE_STAGE_EGRESS) {
        access_profile_id_lcl = access_profile_id;

#ifdef ARAD_TCAM_EGQ_DUMMY_ACCESS_PROFILE_WA_ENABLE
        /* Write the dummy access profile instead of the disable one */
        if(!soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "egress_pmf_lookups_always_valid_disable", 0))
        {
            if (access_profile_id_lcl == ARAD_TCAM_NOF_ACCESS_PROFILE_IDS) {
                access_profile_id_lcl = ARAD_TCAM_EGQ_DUMMY_ACCESS_PROFILE_NO_LOOKUP;
            }
        }
#endif /* ARAD_TCAM_EGQ_DUMMY_ACCESS_PROFILE_WA_ENABLE */

        SOC_SAND_SOC_IF_ERROR_RETURN(res, 122, exit, soc_reg_above_64_get(unit, egq_key_data_base_profile[alloc_info->key_id[access_profile_array_id]], REG_PORT_ANY, 0, reg_above_64));
        SHR_BITCOPY_RANGE(reg_above_64, (ARAD_PMF_KEY_TCAM_DB_PROFILE_NOF_BITS * prog_ndx), &access_profile_id_lcl, 0, ARAD_PMF_KEY_TCAM_DB_PROFILE_NOF_BITS);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 124, exit, soc_reg_above_64_set(unit, egq_key_data_base_profile[alloc_info->key_id[access_profile_array_id]], REG_PORT_ANY, 0, reg_above_64));
    }
    else if (stage ==  SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
        cycle = (alloc_info->cycle == ARAD_PP_FP_KEY_CYCLE_EVEN)?0:1;
        ce_table = (cycle == 0)? IHB_PMF_PASS_1_LOOKUPm : IHB_PMF_PASS_2_LOOKUPm;
        res = soc_mem_read(
                unit,
                ce_table,
                MEM_BLOCK_ANY,
                prog_ndx, /* line */
                tbl_data
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 130, exit);
        if (alloc_info->use_kaps)
        {
            soc_mem_field32_set(unit, ce_table, tbl_data, KAPS_LOOKUP_SELECTf, alloc_info->key_id[access_profile_array_id]);
        } 
        else {
            if (SOC_IS_JERICHO_PLUS(unit)) {
                soc_mem_field32_set(unit, ce_table, tbl_data, qax_fields[alloc_info->key_id[access_profile_array_id]], access_profile_id); 
            } else {
                soc_mem_field32_set(unit, ce_table, tbl_data, fields[alloc_info->key_id[access_profile_array_id]], access_profile_id); 
            }
        }

        res = soc_mem_write(
                unit,
                ce_table,
                MEM_BLOCK_ANY,
                prog_ndx, /* line */
                tbl_data
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 140, exit);
    }
    else {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 150, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_access_profile_hw_set()", 0, 0);
}


/* get resource/information used for DB in given program
 * used only after DB is already created and commited in program
 */
uint32
  arad_pp_fp_db_prog_info_get(
      SOC_SAND_IN  int                           unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                           db_ndx,
      SOC_SAND_IN  uint32                           prog_ndx,
      SOC_SAND_OUT  ARAD_PP_FP_KEY_DP_PROG_INFO     *db_prog_info
  )
{
    ARAD_PMF_DB_INFO
      db_info;
    uint32
        ce_indx;
    ARAD_PMF_CE
      sw_db_ce;
    uint32
      prog_used_cycle_bmp_lcl[1];
    uint32
      res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    db_prog_info->nof_ces = 0;

    /* get DB info from SW state */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
            unit,
            stage,
            db_ndx,
            &db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
    /* check if this program already include this DB */
    if(!SHR_BITGET(db_info.progs,prog_ndx)){
        db_prog_info->used = 0;
        LOG_ERROR(BSL_LS_SOC_FP,
                  (BSL_META_U(unit,
                              "Unit %d, Stage %s, Data Base Id %d, Program %d - Failed to get the information; The Data Base doen\'t include The program.\n\r"),
                   unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), db_ndx, prog_ndx));
        goto exit;
    }

    prog_used_cycle_bmp_lcl[0] = db_info.prog_used_cycle_bmp;
    db_prog_info->cycle = SHR_BITGET(prog_used_cycle_bmp_lcl, prog_ndx) ? 1 : 0;
    db_prog_info->is_320b = db_info.is_320b;
    db_prog_info->alloc_place = db_info.alloc_place;
    db_prog_info->key_id[0] = db_info.used_key[prog_ndx][0];
    db_prog_info->key_id[1] = db_info.used_key[prog_ndx][1];

    /* go over all instructio in this program/cycle check if this used from my-DB*/
    for(ce_indx = 0; ce_indx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG; ++ce_indx) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(
                unit,
                stage,
                prog_ndx,
                db_prog_info->cycle,
                ce_indx,
                &sw_db_ce
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
        if(sw_db_ce.is_used && sw_db_ce.db_id == db_ndx) {
            db_prog_info->ces[db_prog_info->nof_ces] = ce_indx;
            db_prog_info->nof_ces++;
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_db_prog_info_get()", 0, 0);
}

soc_error_t
  arad_pp_fp_is_qual_in_qual_arr(
      SOC_SAND_IN  int                   unit,
      SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE  *qual_types,
      SOC_SAND_IN  uint32                nof_qual_types,
      SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE  search_qual,
      SOC_SAND_OUT uint8                 *found
  )
{
    int qual_idx;
    SOCDNX_INIT_FUNC_DEFS;

    *found = FALSE;

    for (qual_idx = 0; qual_idx < nof_qual_types; qual_idx++, qual_types++) {
        if (*qual_types == search_qual) {
            *found = TRUE;
            SOC_EXIT;
        }
        
        /* If UDF then get the base of the UDF. */
        if (SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(*qual_types)) {
            SOC_PPC_FP_CONTROL_INDEX index;
            SOC_PPC_FP_CONTROL_INFO info;
            uint32 sand_res;

            SOC_PPC_FP_CONTROL_INDEX_clear(&index);
            SOC_PPC_FP_CONTROL_INFO_clear(&info);

            index.type = SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF;
            index.val_ndx = *qual_types - SOC_PPC_FP_QUAL_HDR_USER_DEF_0;
            sand_res = soc_ppd_fp_control_get(unit, SOC_CORE_INVALID, &index, &info);
            SOCDNX_SAND_IF_ERR_EXIT(sand_res);

            /* info.val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX] - base qualifier for the UDF. */
            if ((info.val[0] == SOC_TMC_NOF_PMF_CE_SUB_HEADERS) && (info.val[3 /* SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX */] == search_qual)) {
                *found = TRUE;
                SOC_EXIT;
            }
        }
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

void
    arad_pp_fp_key_qual_to_ce_const(
        SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
        SOC_SAND_IN ARAD_PP_FP_QUAL_CE_INFO   *ce_info,
        SOC_SAND_IN  uint32                    ce_indx,
        SOC_SAND_OUT ARAD_PP_FP_CE_CONSTRAINT *ce_const
    )
{
    ARAD_PP_FP_CE_CONSTRAINT_clear(ce_const);
    ce_const->place_cons = ce_info->place_cons[ce_indx];
    ce_const->cycle_cons = ARAD_PP_FP_KEY_CYCLE_ANY;
    ce_const->group_cons = ce_info->group_cons[ce_indx];
    ce_const->qual_lsb = ce_info->ce_qual_lsb[ce_indx];
    ce_const->lost_bits = ce_info->ce_lost_bits[ce_indx];
    ce_const->nof_bits = ce_info->ce_nof_bits[ce_indx];
    ce_const->size_cons = ce_info->size_cons[ce_indx];
}


void
    arad_pp_fp_key_ce_to_ce_const(
       SOC_SAND_IN  int                            unit,
        SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
        SOC_SAND_IN ARAD_PP_KEY_CE_ID         ce_id,
        SOC_SAND_OUT ARAD_PP_FP_CE_CONSTRAINT *ce_const
    )
{
    ARAD_PP_FP_CE_CONSTRAINT_clear(ce_const);
    ce_const->place_cons = (ce_id >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB)?ARAD_PP_FP_KEY_CE_HIGH:ARAD_PP_FP_KEY_CE_LOW;
    ce_const->size_cons =  arad_pmf_low_level_ce_is_32b_ce(unit, stage, ce_id)?ARAD_PP_FP_KEY_CE_SIZE_32:ARAD_PP_FP_KEY_CE_SIZE_16;
    ce_const->cycle_cons = ARAD_PP_FP_KEY_CYCLE_ANY;
    ce_const->group_cons = (ce_id % ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP)?ARAD_PP_FP_KEY_CE_HIGH:ARAD_PP_FP_KEY_CE_LOW;
}


/*
 * For a specific CE, get the minimal number of lost bits for all scenarios 
 * (buffer type - LSB/MSB, and CE-size - 16/32 bits) 
 */ 
STATIC
uint32
  arad_pp_fp_qual_ce_min_max_lost_bits_get(
      SOC_SAND_IN uint32 nof_buffs,
      SOC_SAND_IN uint8 is_msb, /* relevant if nof_buffs = 1 */
      SOC_SAND_IN uint8 want_max,
      SOC_SAND_INOUT uint32 ce_lost_bits_options[ARAD_PP_FP_KEY_NOF_BUFFERS][ARAD_PP_FP_QUAL_NOF_CE_SIZES]
  )
{
    uint32 
        min_max_lost_bits,
        buff_idx, 
        ce_size_idx;

    buff_idx = ((nof_buffs < 2) && is_msb) ? 1 : 0;
    min_max_lost_bits = ce_lost_bits_options[buff_idx][0];

    for(; buff_idx < 2; buff_idx++)
    {
        for(ce_size_idx = 0; ce_size_idx < ARAD_PP_FP_QUAL_NOF_CE_SIZES; ce_size_idx++)
        {
            min_max_lost_bits = want_max ? SOC_SAND_MAX(min_max_lost_bits, ce_lost_bits_options[buff_idx][ce_size_idx]) :
                SOC_SAND_MIN(min_max_lost_bits, ce_lost_bits_options[buff_idx][ce_size_idx]);
        }
    }
    return min_max_lost_bits;
}

STATIC
  uint32
  arad_pp_fp_key_next_zone_get(
    SOC_SAND_IN uint8  zone_overloaded[ARAD_PP_FP_KEY_NOF_ZONES],
    SOC_SAND_IN uint8  ce_is_msb
    )
{
    uint32 zone_id;

    /* Choose which zone to use (which key 0/1, lsb/msb) */
    if(!ce_is_msb)
    {
        /* For LSB - Choose the zone which uses the lowest number of CEs, unless it is not used yet (if second key isn't used yet) */
        zone_id = !zone_overloaded[ARAD_PP_FP_KEY_ZONE_LSB_0] ? ARAD_PP_FP_KEY_ZONE_LSB_0 : ARAD_PP_FP_KEY_ZONE_LSB_1;
    }
    else
    {
        /* For MSB - Choose the zone which uses the lowest number of CEs, unless it is not used yet (if second key isn't used yet) */
        zone_id = !zone_overloaded[ARAD_PP_FP_KEY_ZONE_MSB_0] ? ARAD_PP_FP_KEY_ZONE_MSB_0 : ARAD_PP_FP_KEY_ZONE_MSB_1;
    }

    return zone_id;
}

STATIC
    uint8
    arad_pp_fp_replace_options_get(
        SOC_SAND_IN     int     unit,
        SOC_SAND_IN     uint8   balance_enabled,
        SOC_SAND_IN     uint8   balance_msb,
        SOC_SAND_IN     uint32  ce_idx,
        SOC_SAND_IN     uint8   ce_is_32[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS],
        SOC_SAND_IN     uint32  qual_nof_bits[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS],
        SOC_SAND_IN     uint32  remaining_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
        SOC_SAND_INOUT  uint32  qual_idx_sorted[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS]
    )
{
    uint8 replace_val = FALSE;
    uint32 
        zone_idx,
        zone_id,
        next_available_zone[2] = {ARAD_PP_FP_KEY_NOF_ZONES,ARAD_PP_FP_KEY_NOF_ZONES},
        idx = qual_idx_sorted[ce_idx], 
        idx_1 = qual_idx_sorted[ce_idx + 1];

    /* Calculate the first zone which is available for both options */
    for(zone_idx = 0; zone_idx < ARAD_PP_FP_KEY_NOF_ZONES; zone_idx++)
    {
        if (SOC_IS_JERICHO(unit) && 
            balance_enabled && 
            soc_property_get(unit, spn_FIELD_KEY_ALLOCATION_MSB_BALANCE_ENABLE, FALSE)) 
        {
            /* check zone availability in the following order:
               if balance_msb == 0 (prefer LSB first), then 0, 1, 2, 3
               if balance_msb == 1 (prefer MSB first), then 1, 0, 3, 2
            */
            zone_id = (zone_idx % 2) ? (zone_idx - balance_msb) : (zone_idx + balance_msb);
        }
        else
        {
            zone_id = zone_idx;
        }

        if(next_available_zone[0] == ARAD_PP_FP_KEY_NOF_ZONES && 
           remaining_bits_in_zone[zone_id] >= qual_nof_bits[idx])
        {
            next_available_zone[0] = zone_id;
        }
        if(next_available_zone[1] == ARAD_PP_FP_KEY_NOF_ZONES && 
           remaining_bits_in_zone[zone_id] >= qual_nof_bits[idx_1])
        {
            next_available_zone[1] = zone_id;
        }
    }
    
    /* First test if the next available zone is the same for both options.
     * If it is, give priority to the options which have the same buffer. 
     * This condition covers mainly the case where a new zone needs to be 
     * started and we want it to be the first available. e.g. if zone 0 is 
     * all used we want the next one to be zone 1 (rather than 2), and 
     * since it is MSB we will try to allocate the MSB options first. 
     */
    if(next_available_zone[1] == next_available_zone[0] &&
       (idx_1 / 2) == (next_available_zone[1] % 2) &&
       (idx / 2) != (next_available_zone[0] % 2))
    {
        replace_val = TRUE;
    }
    /* Test that the last condition is not replaces back */
    else if(!(next_available_zone[1] == next_available_zone[0] &&
            (idx / 2) == (next_available_zone[0] % 2) &&
            (idx_1 / 2) != (next_available_zone[1] % 2)))
    {
        /* 
         * If the first option is better replace, unless:
         * if both are bigger than 16 bits, give priority to 32-bit CE
         */
        if(qual_nof_bits[idx] > qual_nof_bits[idx_1] && 
           !(ce_is_32[idx] && ce_is_32[idx_1] && 
           (idx % 2) && !(idx_1 % 2)))
        {
            replace_val = TRUE;
        }
        /* 
         * If both options are equal AND both are smaller or equal 16 bits:
         * give priority to 16-bit CE
         */
        else if(qual_nof_bits[idx] == qual_nof_bits[idx_1] && 
                !ce_is_32[idx] && !ce_is_32[idx_1] && 
                (idx % 2) && !(idx_1 % 2))
        {
            replace_val = TRUE;
        }
        /* 
         * If both options are equal AND both are bigger than 16 bits:
         * give priority to 32-bit CE
         */
        else if(qual_nof_bits[idx] == qual_nof_bits[idx_1] && 
                ce_is_32[idx] && ce_is_32[idx_1] && 
                !(idx % 2) && (idx_1 % 2))
        {
            replace_val = TRUE;
        }
    }

    return replace_val;
}

STATIC
    void 
    arad_pp_fp_qual_size_sort_idx(
       SOC_SAND_IN  int                            unit,
        SOC_SAND_IN     SOC_PPC_FP_DATABASE_STAGE  stage,
        SOC_SAND_IN     uint32                  nof_options,
        SOC_SAND_IN     uint8                   ce_is_msb,
        SOC_SAND_IN     ARAD_PP_FP_ALLOC_ALG_ARGS  algorithm_args,
        SOC_SAND_IN     uint32                  qual_nof_bits[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS],
        SOC_SAND_IN     uint32                  total_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
        SOC_SAND_IN     uint32                  nof_free_ces_in_zone[ARAD_PP_FP_KEY_NOF_CATEGORIES],
        SOC_SAND_OUT    uint32                  qual_idx_sorted[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS]
    )
{
    uint8 
        swaped, 
        ce_is_32[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS];
    uint32 
        ce_idx, 
        ce_idx_tmp,
        zone_idx,
        remaining_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
        nof_free_ces_bits; /* 0-16b, 1-32b */

    sal_memset(ce_is_32, 0x0, sizeof(uint8) * ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS);

    /* First sort zones according to size and their usage status:
     * - sort already used zone in decreasing available-bits order 
     * - unused zone will not be sorted
     */
    for(zone_idx = 0; zone_idx < ARAD_PP_FP_KEY_NOF_ZONES; zone_idx++)
    {
        remaining_bits_in_zone[zone_idx] = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE - total_bits_in_zone[zone_idx];

        
        nof_free_ces_bits = ((nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(0, (zone_idx % 2), 0)] + 
                              nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(0, (zone_idx % 2), 1)]) * 16) 
                            + ((nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(1, (zone_idx % 2), 0)] + 
                               nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(1, (zone_idx % 2), 1)]) * 32)
                            - ((zone_idx >= 2)? remaining_bits_in_zone[zone_idx - 2] : 0);
        remaining_bits_in_zone[zone_idx] = SOC_SAND_MIN(nof_free_ces_bits, remaining_bits_in_zone[zone_idx]);  
    }

    /* Initialize needed information - ce_is_32 and qual_idx_sorted */
    for(ce_idx = 0; ce_idx < nof_options; ce_idx++)
    {
        /* In case the qualifier is MSB only, the two last elements of
         * qual_nof_bits are the relevant to the calculation
         */
        ce_idx_tmp = (ce_is_msb && nof_options <=2) ? ce_idx + 2 : ce_idx;
        qual_idx_sorted[ce_idx] = ce_idx_tmp;

        /* 
         * If qualifier is larger than 16 - first try to allocate a 32-bit CE,
         * only if failed - try to allocate two of 16
         */
        if(qual_nof_bits[ce_idx_tmp] > 16)
        {
            ce_is_32[ce_idx_tmp] = TRUE;
        }
    }

    do {
        swaped = FALSE;
        for(ce_idx = 0; ce_idx < (nof_options - 1); ce_idx++)
        {
            /* 
             * Test if two options should be replaced according to the number 
             * of bits including the lost bits.
             * In case the qualifier is larger than 16 bits, 32-bit CEs should 
             * be allocated first, even if the number of lost bits is greater. 
             * This is done in order to "save" 16 bit CEs for smaller qualifiers 
             * that will be allocated later on. 
             */
            if(arad_pp_fp_replace_options_get(
                unit,
                algorithm_args.balance_enabled, /* is-balance-enabled */
                algorithm_args.balance_lsb_msb, /* if enabled, which to prefer - LSB or MSB */
                ce_idx,
                ce_is_32,
                qual_nof_bits,
                remaining_bits_in_zone,
                qual_idx_sorted))
            {
                ce_idx_tmp = qual_idx_sorted[ce_idx];
                qual_idx_sorted[ce_idx] = qual_idx_sorted[ce_idx+1];
                qual_idx_sorted[ce_idx+1] = ce_idx_tmp;
                swaped = TRUE;
            }
        }
    } while(swaped); 
}

STATIC
    uint8 
    arad_pp_fp_free_ces_in_zone_get(
        SOC_SAND_IN     uint32  flags,
        SOC_SAND_IN     uint8   modify,
        SOC_SAND_IN     uint8   ce_is_msb,
        SOC_SAND_INOUT  uint8   *ce_is_32,
        SOC_SAND_INOUT  uint8   *ce_is_high_group,
        SOC_SAND_INOUT  uint32  nof_free_ces_in_zone[ARAD_PP_FP_KEY_NOF_CATEGORIES]
    )
{
    uint8 success = TRUE;

    if (*ce_is_32) {
        /* Check if there is a 32-bit instruction according to LSB/MSB */
        if (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(1, ce_is_msb, 0)] + nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(1, ce_is_msb, 1)]) { 
            *ce_is_32 = 1;
            /* In general, load balance between groups except if the order is set outside */
            if (flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER) {
                /* Prefer high-group first since it is in MSB */
                *ce_is_high_group = (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 1)])? 1: 0;
            }
            else if (flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) {
                /* Prefer lowest-group first since for FLP since it is in LSB to reduce the master-key size */
                *ce_is_high_group = (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 0)])? 0: 1;
            }
            else {
                *ce_is_high_group = (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 0)] < nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 1)])? 1: 0;
            }
            if(modify){
                nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, *ce_is_high_group)] --;
            }
        }
        /* Check if there are two 16-bit instructions according to LSB/MSB*/
        else 
        {
          /* Can't make this allocation, try next option if there is */
          success = FALSE;
        }
    }
    else {
        /* Check if there is a 16-bit instruction according to LSB/MSB */
        if (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(0, ce_is_msb, 0)] + nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(0, ce_is_msb, 1)]) { 
            *ce_is_32 = 0;
        }
        /* Check if there is a 32-bit instruction according to LSB/MSB */
        else if (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(1, ce_is_msb, 0)] + nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(1, ce_is_msb, 1)]) { 
            *ce_is_32 = 1;
        }
        else 
        {
          /* Can't make this allocation, try next option if there is */
          success = FALSE;
        }
        /* In general, load balance between groups except if the order is set outside */
        if (flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER) {
            /* Prefer high-group first since it is in MSB */
            *ce_is_high_group = (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 1)])? 1: 0;
        }
        else if (flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) {
            /* Prefer lowest-group first since for FLP since it is in LSB to reduce the master-key size */
            *ce_is_high_group = (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 0)])? 0: 1;
        }
        else {
            *ce_is_high_group = (nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 0)] < nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, 1)])? 1: 0;
        }
        if(modify){
            nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(*ce_is_32, ce_is_msb, *ce_is_high_group)] --;
        }
    }
    return success;
}
/* 
 * Compute if there are enough instructions for this qualifier 
 * size. Used when qualifier not broken. 
 */
STATIC
  uint8
  arad_pp_fp_key_not_enough_free_instr_get(
      SOC_SAND_IN  uint32   nof_free_ces_in_zone[ARAD_PP_FP_KEY_NOF_CATEGORIES],
      SOC_SAND_IN  uint32   qual_nof_bits, 
      SOC_SAND_IN  uint8    is_msb /* LSB/MSB looked for */
    )
{
  uint32 
      tmp_size,
      nof_ce_32b_free[2] = {0, 0}, /* 0-16b, 1-32b */
      nof_ce_32b_needed[2] = {0, 0}; /* 0-16b, 1-32b */
  uint8
      is_overloaded;
  int8
      is_high_group,
      is_32b;

  /* 
   * Compute first the number of 16b/32b needed for this qualifier size: 
   * - Remove 32b as long as needed 
   * - Then use 1*16b or 1*32b for the last instruction 
   */
  tmp_size = qual_nof_bits;
  while(tmp_size > 0) 
  {
      if (tmp_size > 32) {
          nof_ce_32b_needed[1] ++;
          tmp_size -= 32;
      }
      else {
          nof_ce_32b_needed[tmp_size > 16] ++;
          break;
      }
  }

  /* 
   * Compute if there are enough instructions: 
   * - any 32b CE takes 1*32b instruction or 2*16b instructions 
   * - any 16b ce takes 1*32b instruction or 1*16b instruction
   */
  for(is_32b = 0; is_32b <= 1; ++is_32b) {
      for(is_high_group = 0; is_high_group <= 1; ++is_high_group) {
          nof_ce_32b_free[is_32b] += nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(is_32b, is_msb, is_high_group)]; 
      }
  }

  /* 
   *  Remove all the 32b instructions first, then see if enough place                                      
   */   
  is_overloaded = FALSE;                                                                                                  
  for(is_32b = 1; (is_32b >= 0) && (!is_overloaded); is_32b--) {
      if (is_32b) {
          if (nof_ce_32b_free[is_32b] >= nof_ce_32b_needed[is_32b]) {
              nof_ce_32b_free[is_32b] -= nof_ce_32b_needed[is_32b];
          }
          else {
              nof_ce_32b_needed[is_32b] -= nof_ce_32b_free[is_32b];
              /* See if enough 16b instructions */
              if (nof_ce_32b_needed[is_32b] <= (nof_ce_32b_free[0] / 2)) {
                  nof_ce_32b_free[0] -= (nof_ce_32b_needed[is_32b] * 2);
              }
              else {
                  is_overloaded = TRUE;
              }
          }
      }
      else {
          /* 16b instructions needed */
          if (nof_ce_32b_needed[is_32b] > (nof_ce_32b_free[0] + nof_ce_32b_free[1])) {
              is_overloaded = TRUE;
          }
      }
  }

  return is_overloaded;
}

/*
 * Attempt an allocation of a single CE in key zone. 
 * Based on available bits, available CEs and the CE size and constraints 
 * try to find the best location or any. 
 *  
 * return Value:
 *  uint32 - zone ID - if allocation suceeded return the zone ID allocated,
 *           else return ARAD_PP_FP_KEY_NOF_ZONES
 *  
 * SOC_SAND_IN: 
 *  total_bits_in_zone - array of free bits per zone
 *  is_lsb - indicates if LSB zone is allowed
 *  is_msb - indicates if MSB zone is allowed
 *  nof_options - how many options of CE size and constaints to choose from
 *  qual_nof_bits - the actual size options to choose from (which options are
 *                  determined according to nof_options and ce_is_msb).
 *  nof_free_ces_in_zone - array of free CEs (16-bit LSB, 32-bit LSB, 16-bit MSB, 32-bit MSB)
 *  
 * SOC_SAND_OUT: 
 *  nof_free_ces_in_zone - array of free CEs as described above after allocation
 *  ce_is_msb - in case of success - allocation of LSB/MSB buffer (different meaning
 *              from is_lsb/is_msb) - to determine which element in qual_nof_bits
 *  ce_is_32 - in case of success - allocation of 32/16 bits CE
 *  curr_size - in case of success - the number of bits allocated,
 *              in case of failure - the maximum number of bits that can be
 *              allocated in any zone
 */
STATIC
  uint32
  arad_pp_fp_key_next_zone_probe(
     SOC_SAND_IN  int                            unit,
    SOC_SAND_IN     SOC_PPC_FP_DATABASE_STAGE  stage,
    SOC_SAND_IN     uint32  flags,
    SOC_SAND_IN     ARAD_PP_FP_ALLOC_ALG_ARGS  algorithm_args,
    SOC_SAND_IN     uint32  total_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
    SOC_SAND_IN     uint8   is_lsb,
    SOC_SAND_IN     uint8   is_msb,
    SOC_SAND_IN     uint32  nof_options,
    SOC_SAND_IN     uint32  qual_nof_bits[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS],
    SOC_SAND_INOUT  uint32  nof_free_ces_in_zone[ARAD_PP_FP_KEY_NOF_CATEGORIES],
    SOC_SAND_OUT    uint8   *ce_is_msb,
    SOC_SAND_OUT    uint8   *ce_is_32,
    SOC_SAND_OUT    uint8   *ce_is_high_group,
    SOC_SAND_OUT    uint32  *curr_size
    )
{
    uint32 
        zone_id = 0,
        zone_idx,
        max_remaining_size,
        qual_id,
        qual_idx,
        qual_idx_sorted[ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS];
    uint8 
        success = FALSE,
        zone_overloaded[ARAD_PP_FP_KEY_NOF_ZONES];

    /* 
     * First choose which CE size to use according to availability -
     * Sort the CE allocation options according to their size and buffer. 
     * Then for each option (best first) check if allocation can be found. 
     * If not move to the next option. 
     * In case no allocation can be done return ARAD_PP_FP_KEY_NOF_ZONES 
     * as an indication that no zone ID was matched. 
     */
    arad_pp_fp_qual_size_sort_idx(
        unit,
        stage,
        nof_options, 
        (!is_lsb && is_msb), 
        algorithm_args,
        qual_nof_bits, 
        total_bits_in_zone,
        nof_free_ces_in_zone,
        qual_idx_sorted);

    max_remaining_size = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE - total_bits_in_zone[0];

    for(qual_idx = 0; qual_idx < nof_options; qual_idx++)
    {
        qual_id = qual_idx_sorted[qual_idx];
        
        /* according to qual index fix ce_info so that it would indicate is_lsb/is_msb */
        *ce_is_msb = (qual_id < 2) ? FALSE : TRUE;
        *ce_is_32 = (qual_id % 2) ? TRUE : FALSE;
        
        for(zone_idx = 0; zone_idx < ARAD_PP_FP_KEY_NOF_ZONES; zone_idx++)
        {
            if((ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE - total_bits_in_zone[zone_idx]) > max_remaining_size) {
                max_remaining_size = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE - total_bits_in_zone[zone_idx];
            }

            /* Check if there is enough available bits in each zone */
            zone_overloaded[zone_idx] = 
                ((total_bits_in_zone[zone_idx] + qual_nof_bits[qual_id]) > ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE)? 
                TRUE: FALSE;

            /* Take account of the number of free CE instructions to indicate an overload */
            zone_overloaded[zone_idx] |= 
                arad_pp_fp_key_not_enough_free_instr_get(
                    nof_free_ces_in_zone,
                    qual_nof_bits[qual_id], 
                    *ce_is_msb
                );

            /* consider the LSB/MSB constraints of the qualifier */
            if(!zone_overloaded[zone_idx] && (*ce_is_msb == (zone_idx % 2))){
                success = TRUE;
            }
        }

        /* Cannot peform allocation according
           to the current constraints */
        if(!success) {
            continue;
        }

        /* Check number of free CEs, do not modify */
        success = arad_pp_fp_free_ces_in_zone_get(
                    flags,
                    FALSE, /* do not modify */
                    *ce_is_msb,
                    ce_is_32, 
                    ce_is_high_group,
                    nof_free_ces_in_zone);
        
        if(success){

            /* 
             * If requested CE size is not free OR
             * if the actual size does not match the CE size 
             * continue search 
             */
            if(((qual_id % 2) != *ce_is_32) || 
               (qual_nof_bits[qual_id] > 16 && !(*ce_is_32)))
            {
                success = FALSE;
                continue;
            }

            /* Find a Zone ID to match current CE */
            zone_id = arad_pp_fp_key_next_zone_get(
                        zone_overloaded, 
                        *ce_is_msb
                      );

            /* Update number of free CEs*/
            success = arad_pp_fp_free_ces_in_zone_get(
                        flags,
                        TRUE, 
                        *ce_is_msb,
                        ce_is_32, 
                        ce_is_high_group,
                        nof_free_ces_in_zone);

            /* indicate the actual number of bits allocated */
            *curr_size = qual_nof_bits[qual_id];

            break;
        }
    }

    /* If the allocation was NOT successful - zone ID
     * indicated the failure, and curr_size indicated 
     * the maximum number of bits that CAN be allocated 
     */
    if(!success) {
        zone_id = ARAD_PP_FP_KEY_NOF_ZONES;
        *curr_size = max_remaining_size;
    }
    return zone_id;
}

/*
 * Given a qualifier type, attempt to allocate space for its construction 
 * in an already existing key, according to given key information.
 *  
 * return Value:
 *  res - OK or ERROR.
 *  
 * SOC_SAND_IN: 
 *  unit - the device ID
 *  stage - Ingress or Egress
 *  break_uneven - FALSE if only check is needed, TRUE for optimized allocation
 *   
 * SOC_SAND_OUT: 
 *  total_bits_in_zone - array of free bits per zone
 *  nof_free_ces_in_zone - array of free CEs (16-bit LSB, 32-bit LSB, 16-bit MSB, 32-bit MSB)
 *  zone_used - which zones were used for this qualifier in case of success
 *  qual_ce_info - information of all CEs for this qualifier
 *  success - indication if allocation was successful
 */
STATIC
uint32
  arad_pp_fp_qual_ce_info_uneven_get(
      SOC_SAND_IN       int                         unit,
      SOC_SAND_IN       SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN       uint8                       break_uneven,
      SOC_SAND_IN       uint32                      flags,
      SOC_SAND_IN       SOC_PPC_FP_QUAL_TYPE        qual_type,
      SOC_SAND_INOUT    ARAD_PP_FP_ALLOC_ALG_ARGS   *algorithm_args,
      SOC_SAND_INOUT    uint32                      total_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
      SOC_SAND_INOUT    uint32                      nof_free_ces_in_zone[ARAD_PP_FP_KEY_NOF_CATEGORIES],
      SOC_SAND_OUT      uint8                       zone_used[ARAD_PP_FP_KEY_NOF_ZONES],
      SOC_SAND_OUT      ARAD_PP_FP_QUAL_CE_INFO     *qual_ce_info, 
      SOC_SAND_OUT      uint8                       *success 
      )
{
    uint8
        found = 0xff, /* putting extrene value for case of using uninitialized variable */
        found_internal[2],
        is_header,
        ce_is_high_group = 0,
        ce_is_msb = 0,
        ce_is_32 = 0,
        is_msb;
    uint32
        res = SOC_SAND_OK,
        bits_for_ce,
        hw_buffer_jericho,
        qual_lsb,
        ce_offset,
        tmp_size = 0,
        break_idx,
        max_break_idx,
        found_indx,
        ce_size_idx,
        nof_found = 0,
        ce_cur_size = 32,
        alloc_size = 0,
        qual_cur_size = 0,
        zone_id = 0,
        lost_bits,
        qual_nof_bits[ARAD_PP_FP_KEY_NOF_BUFFERS * ARAD_PP_FP_QUAL_NOF_CE_SIZES],
        ce_lost_bits_options[ARAD_PP_FP_KEY_NOF_BUFFERS][ARAD_PP_FP_QUAL_NOF_CE_SIZES],
        found_place[2] = {0,0};
    ARAD_PMF_CE_IRPP_QUALIFIER_INFO
        int_qual_info[2];
    ARAD_PMF_CE_HEADER_QUAL_INFO
        hdr_qual_info;
    ARAD_PMF_CE_PACKET_HEADER_INFO
        hdr_offset;

    /* Initialization */

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(qual_ce_info);
    SOC_SAND_CHECK_NULL_INPUT(success);

    /* these arrays are not expected for resource check only */
    if(break_uneven)
    {
        SOC_SAND_CHECK_NULL_INPUT(total_bits_in_zone);
        SOC_SAND_CHECK_NULL_INPUT(nof_free_ces_in_zone);
        SOC_SAND_CHECK_NULL_INPUT(zone_used);
    }
    
    qual_ce_info->is_header = 0;
    qual_ce_info->place_cons[0] = 0;
    qual_ce_info->nof_ce = 0;
    qual_ce_info->qual_size = 0;
    qual_ce_info->lost_bits = 0;
    qual_ce_info->lost_bits_worst = 0;
    
    *success = TRUE;

    if (SOC_IS_ARADPLUS(unit))
    {  
        /* No need to allocate CEs for IsEqual qualifier */
        if(qual_type == SOC_PPC_FP_QUAL_IS_EQUAL)
        {
            if (break_uneven) {
                zone_used[ARAD_PP_FP_KEY_ZONE_MSB_0] = TRUE;
            }
            goto exit;
        }
    }

    /* 
     * First try to find out if qualifier is of header type 
     */
    res = arad_pmf_ce_header_info_find(
             unit,
             qual_type,
             stage,
             &is_header,
             &hdr_qual_info
             );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if(is_header) 
    {
        qual_ce_info->is_header = 1;

        /* header can be in LSB or MSB */
        qual_ce_info->is_lsb = 1;
        qual_ce_info->is_msb = 1;
        qual_ce_info->place_cons[0] = ARAD_PP_FP_KEY_CE_PLACE_ANY;

        qual_ce_info->qual_size = hdr_qual_info.lsb - hdr_qual_info.msb + 1;
        hdr_offset.nof_bits = qual_ce_info->qual_size;
        hdr_offset.offset = hdr_qual_info.msb;

        
        tmp_size = qual_ce_info->qual_size;
    }
    else
    {
        /* In case the qualifier is not of header type - try to find
         * it in the internal table types, done twice since there are 
         * two buffers for internal qualifiers 
         */
        for(is_msb = 0; is_msb <=1; is_msb++)
        {
            /* check if it's qualifier for internal value */
            res = arad_pmf_ce_internal_field_info_find(
                    unit,
                    qual_type,
                    stage,
                    is_msb,
                    &(found_internal[is_msb]),
                    &(int_qual_info[is_msb])
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            if(is_msb)
                qual_ce_info->is_msb = found_internal[is_msb];
            else
                qual_ce_info->is_lsb = found_internal[is_msb];

            /* If qualifer was found update constraints */
            if(found_internal[is_msb]) {
                found_place[nof_found++] = is_msb;
                qual_ce_info->place_cons[0] |= is_msb ? ARAD_PP_FP_KEY_CE_HIGH : ARAD_PP_FP_KEY_CE_LOW;
            }
        }

        /* 
         * Validate the results match expectations
         */
        if(!qual_ce_info->is_msb && !qual_ce_info->is_lsb) {
           LOG_ERROR(BSL_LS_SOC_FP,
                     (BSL_META_U(unit,
                                 "Unit %d Stage %s Qualifier %s : The qualifier does not exist in no buffer.\n\r"),
                      unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), SOC_PPC_FP_QUAL_TYPE_to_string(qual_type)));
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 50, exit);
        }
        if((nof_found == 2) && (int_qual_info[0].info.qual_nof_bits != int_qual_info[1].info.qual_nof_bits)) {
           LOG_ERROR(BSL_LS_SOC_FP,
                     (BSL_META_U(unit,
                                 "Unit %d Stage %s Qualifier %s : Number of bits differs between lsb to msb.\n\r"
                                 "Number of bits in lsb %d, Munber of bits in msb %d.\n\r"),
                      unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), SOC_PPC_FP_QUAL_TYPE_to_string(qual_type), int_qual_info[0].info.qual_nof_bits, int_qual_info[1].info.qual_nof_bits));
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 51, exit); /* not expected different sizes */
        }

        /* In case the qualifier was found, update its size */
        if (nof_found > 0) {
            qual_ce_info->qual_size = (found_internal[1])? int_qual_info[1].info.qual_nof_bits : int_qual_info[0].info.qual_nof_bits;
            /* 
             * Special case for FLP: 
             * in case of zero qualifier, reduce the qualifier size to the key to 
             * be bit-aligned 
             */
            if ((flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) && (qual_type == SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES)) {
                qual_ce_info->qual_size = SOC_SAND_ALIGN_UP(total_bits_in_zone[0], SOC_SAND_NOF_BITS_IN_CHAR) - total_bits_in_zone[0];
            }
            tmp_size = qual_ce_info->qual_size;
        }
    }

    qual_lsb = 0;

    /* 
     * Break the header/internal qualifier into several CEs according to its size 
     * and free space in the key.
     * Then for each CE calculate the lost bits for several scenarios (the amount 
     * of lost bits may differ if it is calculated as 32/16 bit CE and for internal 
     * qualifier the buffer it is read from). 
     * Then allocate the key zone where it would be places (first/second key, 
     * and LSB/MSB). 
     */
    while(tmp_size > 0) 
    {
        if ((flags & ARAD_PP_FP_KEY_ALLOC_CE_SINGLE_MAPPING) && (qual_ce_info->nof_ce != 0)) {
                /* No more Space - Error */
                *success = FALSE;
                LOG_DEBUG(BSL_LS_SOC_FP,
                          (BSL_META_U(unit,
                                      SOC_DPP_MSG("    "
                                      "Key: fail to split qualifier because the key zones are too small \n\r"))));
                goto exit; /* not enough place for the qualifiers */
        }
        sal_memset(ce_lost_bits_options, 0x0, sizeof(uint32) * ARAD_PP_FP_QUAL_NOF_CE_SIZES * ARAD_PP_FP_KEY_NOF_BUFFERS);

        /* Calculate the lost bits for each CE according to the CE size (16-bit CEs are in
         * nibble resolution and 32-bit CEs are in byte resolution), and according to the 
         * buffer type in case of internal. 
         * In case of header there is no significance to the LSB/MSB buffer and thus there 
         * are considered the same size. For internal qualifier, there could be a difference 
         * in the number of lost bits in different buffers as well.
         */
        if(is_header) 
        {
            /* All CE sizes are okay, as well as LSB and MSB */
            nof_found = ARAD_PP_FP_KEY_NOF_BUFFERS;
            found_place[0] = 0;
            found_place[1] = 1;

            for(ce_size_idx = 0; ce_size_idx < ARAD_PP_FP_QUAL_NOF_CE_SIZES; ce_size_idx++)
            {
                /* Compute the actual number of bits needed from this offset (including lost bits) */
                res = arad_pmf_ce_nof_real_bits_compute_header(
                        unit,
                        &hdr_offset,
                        0,     /* is_msb doesn't matter */
                        ce_size_idx,    /* is_32b_ce - 0 - CE-16, 1 - CE-32*/
                        &bits_for_ce 
                        );
                SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

                /* In case of header the CE could be 16 or 32 bits, but there is only one buffer */
                ce_lost_bits_options[ARAD_PP_FP_KEY_BUFF_LSB][ce_size_idx] = bits_for_ce - hdr_offset.nof_bits;
                ce_lost_bits_options[ARAD_PP_FP_KEY_BUFF_MSB][ce_size_idx] = bits_for_ce - hdr_offset.nof_bits;
            }
        }
        else
        {
            /* 
             * Go over the places we found this qualifier (LSB/MSB) 
             * In case the qualifier is found at both LSB and MSB, 
             * the lost bits are calculated for both buffers, and also 
             * for both possible sizes (16/32) 
             */ 
            for(found_indx = 0; found_indx < nof_found; ++found_indx) 
            {
                for(ce_size_idx = 0; ce_size_idx < ARAD_PP_FP_QUAL_NOF_CE_SIZES; ce_size_idx++)
                {
                    res = arad_pmf_ce_internal_field_offset_compute(
                            unit,
                            qual_type,
                            stage,
                            found_place[found_indx],    /* is_msb */
                            qual_ce_info->qual_size - qual_lsb,       
                            qual_lsb,
                            0,              /* lost_bits */
                            ce_size_idx,    /* is_32b_ce - 0 - CE-16, 1 - CE-32*/
                            &found,
                            &ce_offset,
                            &bits_for_ce,
                            &hw_buffer_jericho
                            );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

                    if ((flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) && (qual_type == SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES)) {
                        ce_lost_bits_options[found_place[found_indx]][ce_size_idx] = 0; /* no lost bits in special FLP case byte-aligned */
                    }
                    else {
                        ce_lost_bits_options[found_place[found_indx]][ce_size_idx] = bits_for_ce - (qual_ce_info->qual_size - qual_lsb);
                    }
                }
            }
        }
      
        /* Break the qualifier into CEs as necessary */
        if(break_uneven) 
        {
            /* break_idx : first try to allocate CE regularly.
             * Only if first attemp fails, then try to break qualifier according to free 
             * space as indicated with return value of arad_pp_fp_key_next_zone_probe
             */
            max_break_idx =  (((flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER) ||
                               (flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT)) ? 
                              1/* no split */: 2);

            for(break_idx = 0; break_idx < max_break_idx; break_idx++)
            {
                /* Fill the qual_nof_bits array to indicate the possible sizes for allocation */
                for(found_indx = 0; found_indx < nof_found; ++found_indx) 
                {
                    for(ce_size_idx = 0; ce_size_idx < ARAD_PP_FP_QUAL_NOF_CE_SIZES; ce_size_idx++)
                    {
                        /* qual_nof_bits is a flat array which corresponds to a two dimension array.
                         * Its calculation is done according to the number of lost bits expected and 
                         * the results of the first round of the allocation attempt (if break_idx >0).
                         */
                        lost_bits = ce_lost_bits_options[found_place[found_indx]][ce_size_idx];

                        /* Test if this is the second round and the maximum size in a zone is big enough */
                        if(break_idx > 0 && alloc_size > lost_bits && (tmp_size + lost_bits) >= alloc_size) {
                            ce_cur_size = (alloc_size < 32) ? alloc_size : 32;
                        }
                        else {
                            ce_cur_size = (tmp_size + lost_bits) < 32 ? (tmp_size + lost_bits) : 32;
                        }
                        qual_nof_bits[found_place[found_indx] * 2 + ce_size_idx] = ce_cur_size;
                    }
                }

                zone_id =  arad_pp_fp_key_next_zone_probe(
                                unit,
                                stage,
                                flags,
                                *algorithm_args,
                                total_bits_in_zone,
                                qual_ce_info->is_lsb,
                                qual_ce_info->is_msb,
                                nof_found * 2, /* number of options : 2 or 4 (lsb-16, lsb-32 and optional msb-16, msb-32) */
                                qual_nof_bits,
                                nof_free_ces_in_zone,
                                &ce_is_msb,
                                &ce_is_32,
                                &ce_is_high_group,
                                &alloc_size
                                );
                                
                /* for balance optimization flip lsb/msb indication */
                if (algorithm_args->balance_enabled) {
                    algorithm_args->balance_lsb_msb ^= 1;
                }

                /* Allocation could not be completed */
                if(zone_id < ARAD_PP_FP_KEY_NOF_ZONES || alloc_size <= 0)
                    break;
            }

            /* Allocation Failed */
            if(zone_id >= ARAD_PP_FP_KEY_NOF_ZONES)
            {
                /* No more Space - Error */
                *success = FALSE;
                LOG_DEBUG(BSL_LS_SOC_FP,
                          (BSL_META_U(unit,
                                      "    "
                                      "Key: fail to split qualifier because the key zones are too small \n\r")));
                goto exit; /* not enough place for the qualifiers */
            }

            zone_used[zone_id] = TRUE;

            /* update lost bits according to chosen zone and CE size */
            ce_cur_size = alloc_size;
            qual_ce_info->ce_lost_bits[qual_ce_info->nof_ce] = ce_lost_bits_options[ce_is_msb][ce_is_32];
            qual_ce_info->place_cons[qual_ce_info->nof_ce] = ce_is_msb ? ARAD_PP_FP_KEY_CE_HIGH : ARAD_PP_FP_KEY_CE_LOW;
            qual_ce_info->group_cons[qual_ce_info->nof_ce] = ce_is_high_group? ARAD_PP_FP_KEY_CE_HIGH : ARAD_PP_FP_KEY_CE_LOW;
            qual_ce_info->size_cons[qual_ce_info->nof_ce] = ce_is_32 ? ARAD_PP_FP_KEY_CE_SIZE_32 : ARAD_PP_FP_KEY_CE_SIZE_16;

            /* Update the total_bits_in_zone so that the next CE allocation would be accurate */ 
            total_bits_in_zone[zone_id] += ce_cur_size;
        }
        else /* Only estimate the size according to the best case scenario */
        {
            /* The lost_bits field should save the minimal aggregated lost bits for all CEs of each qualifier */
            qual_ce_info->ce_lost_bits[qual_ce_info->nof_ce] = arad_pp_fp_qual_ce_min_max_lost_bits_get(nof_found, found_internal[1], FALSE, ce_lost_bits_options);
            qual_ce_info->lost_bits_worst = arad_pp_fp_qual_ce_min_max_lost_bits_get(nof_found, found_internal[1], TRUE, ce_lost_bits_options);

            qual_ce_info->place_cons[qual_ce_info->nof_ce] = qual_ce_info->place_cons[0];
            qual_ce_info->group_cons[qual_ce_info->nof_ce] = ARAD_PP_FP_KEY_CE_PLACE_ANY;
			qual_ce_info->size_cons[qual_ce_info->nof_ce] = ARAD_PP_FP_KEY_CE_SIZE_ANY;
            ce_cur_size = ((tmp_size + qual_ce_info->ce_lost_bits[qual_ce_info->nof_ce]) < 32) ? 
                (tmp_size + qual_ce_info->ce_lost_bits[qual_ce_info->nof_ce]) : 32;

        }

        qual_ce_info->ce_nof_bits[qual_ce_info->nof_ce] = ce_cur_size;
        qual_ce_info->lost_bits += qual_ce_info->ce_lost_bits[qual_ce_info->nof_ce];

        /* The number of bits from the qualifier that can be taken considering the lost bits */
        qual_cur_size = ce_cur_size - qual_ce_info->ce_lost_bits[qual_ce_info->nof_ce];
        qual_ce_info->ce_qual_lsb[qual_ce_info->nof_ce] = qual_lsb;
        qual_lsb += qual_cur_size;

        tmp_size -= qual_cur_size;
        hdr_offset.nof_bits -= qual_cur_size;

        if(break_uneven)
        {
            qual_ce_info->ce_zone_id[qual_ce_info->nof_ce] = zone_id;
            zone_used[zone_id] = TRUE;
        }

        qual_ce_info->nof_ce++;
    }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_ce_info_uneven_get()", qual_type, 0);
}

uint32
  arad_pp_fp_qual_ce_info_get(
	  SOC_SAND_IN  int                              unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
	  SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE                qual_type,
      SOC_SAND_OUT  ARAD_PP_FP_QUAL_CE_INFO          *qual_ce_info 
  )
{
    uint8 
        success,
        dummy_u8[ARAD_PP_FP_KEY_NOF_ZONES];
    uint32 
        res, 
        dummy_u32[ARAD_PP_FP_KEY_NOF_ZONES];
    ARAD_PP_FP_ALLOC_ALG_ARGS
        algorithm_args;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  algorithm_args.balance_enabled = 0;
  algorithm_args.balance_lsb_msb = 0;

  res = arad_pp_fp_qual_ce_info_uneven_get(
          unit,
          stage,
          FALSE,
          0,
          qual_type,
          &algorithm_args,
          dummy_u32, /* total bits in zone[] - not to be used */
          dummy_u32, /* free ces in zone[] - not to be used */
          dummy_u8,   /* zone_used[] - not to be used */
          qual_ce_info,
          &success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_ce_info_get()", qual_type, 0);
}

/* given constrain place/size, return u32 bitmap, where 1: CE is fit constrain, 0 otherwise */
uint32
  arad_pp_fp_key_get_available_ce_under_constraint(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 place_cons,/* lsb, msb, any*/
      SOC_SAND_IN  uint32                 size_cons,  /* 16, 32, any*/
      SOC_SAND_IN  uint32                 group_cons  /* 0, 1, any*/
  )
{
  uint32
    avail_bmp_ce = 0,
    avail_bmp_place = 0,
    avail_bmp_group = 0,
    avail_bmp_size = 0;

  if(place_cons == ARAD_PP_FP_KEY_CE_PLACE_ANY) {
      avail_bmp_place = 0xffffffff;
  }
  else if(place_cons == 0) {
      return 0;
  }
  /* set 0-15 as ones, if can allocate from low*/
  else
  {
      SHR_BITSET_RANGE(&avail_bmp_place,0,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB);
  /* set 16-31 as ones, if can allocate from high*/
      if(place_cons == ARAD_PP_FP_KEY_CE_HIGH) 
      {
          avail_bmp_place = ~avail_bmp_place;
      }
  }

  if(size_cons == ARAD_PP_FP_KEY_CE_SIZE_ANY) {
      avail_bmp_size = 0xffffffff;
  }
  else if(size_cons == 0) {
      return 0;
  }
  else{
      /* 
       * Both at ingress PMF, SLB, FLP, and egress PMF, we have 4 groups of 16b/32b CEs 
       * (except SLB with 4 groups of 32b CEs) 
       */ 
      if (ARAD_PMF_LOW_LEVEL_PROG_GROUP_IS_ALL_32) {
          avail_bmp_size = 0;
      }
      else {
          /* for 16 size */
          /* set 0-3*/
          SHR_BITSET_RANGE(&avail_bmp_size,0,ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP);

          /* set 8-11*/
          SHR_BITSET_RANGE(&avail_bmp_size,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP,ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP);
          
          /* set 16-19*/
          SHR_BITSET_RANGE(&avail_bmp_size,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB,ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP);
          
          /* set 24-27*/
          SHR_BITSET_RANGE(&avail_bmp_size,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB+ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP,ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP);
      }
      
      /* if size is 32, inverse bitmap*/
      if((size_cons == ARAD_PP_FP_KEY_CE_SIZE_32)){
          avail_bmp_size = ~avail_bmp_size;
      }
  }


  if(group_cons == ARAD_PP_FP_KEY_CE_PLACE_ANY) {
      avail_bmp_group = 0xffffffff;
  }
  else if(group_cons == 0) {
      return 0;
  }
  else{
      /* for group 0 */
      /* set 0-8*/
    SHR_BITSET_RANGE(&avail_bmp_group,0,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP);
      
      /* set 16-24*/
	SHR_BITSET_RANGE(&avail_bmp_group,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP);
      
      /* if high group, inverse bitmap*/
      if((group_cons == ARAD_PP_FP_KEY_CE_HIGH)){
          avail_bmp_group = ~avail_bmp_group;
      }
  }

  /* 
   * Get the number of CEs per Program to filter the ~ 
   * when number of CEs is not 32                                                ~
   */
  avail_bmp_ce = 0xffffffff;
  if (ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG != 32) {
      /* coverity explaination:the value of index will never beyond 32*/
      /* coverity[large_shift:FALSE] */
      avail_bmp_ce = (1 << ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG) - 1;
  }
  

  /* bitwise all */
  return avail_bmp_place & avail_bmp_group & avail_bmp_size & avail_bmp_ce;
}



/* 
 *   given program, Cycle, CE-index,
 *   return if it in use or not
 *   used only if ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP flag  not presents
 */
uint32
  arad_pp_fp_key_ce_is_in_use(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 prog_ndx,
      SOC_SAND_IN  uint32                 cycle_ndx, /* 0, 1*/
      SOC_SAND_IN  ARAD_PP_KEY_CE_ID      ce_ndx,
      SOC_SAND_OUT  uint8                 *in_use

  )
{
    ARAD_PMF_CE
      sw_db_ce;
    uint32
      res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(
            unit,
            stage,
            prog_ndx,
            cycle_ndx,
            ce_ndx,
            &sw_db_ce
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    *in_use = (sw_db_ce.is_used == TRUE);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_ce_is_in_use()", 0, 0);
}



/* 
 *   given program, Cycle, CE-index,
 *   mark CE in use 
 */
uint32
  arad_pp_fp_key_ce_set_use(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 prog_ndx,
      SOC_SAND_IN  uint32                 cycle_ndx, /* 0, 1*/
      SOC_SAND_IN  ARAD_PP_KEY_CE_ID      ce_ndx,
      SOC_SAND_IN  uint8                  in_use

  )
{
    ARAD_PMF_CE
      sw_db_ce;
    uint32
      res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(
            unit,
            stage,
            prog_ndx,
            cycle_ndx,
            ce_ndx,
            &sw_db_ce
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    sw_db_ce.is_used = in_use;

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.set(
            unit,
            stage,
            prog_ndx,
            cycle_ndx,
            ce_ndx,
            &sw_db_ce
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_ce_set_use()", 0, 0);
}

/* 
 * See if this Database: 
 * - is cascaded 
 * - if so, if 2nd cycle 
 * - if so, if it has already a Key-ID in some Program 
 * - if so, give this Key-ID 
 */
uint32
    arad_pp_fp_key_alloc_key_cascaded_key_get(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32  db_id_ndx,
      SOC_SAND_OUT uint8  *is_key_fixed,
      SOC_SAND_OUT uint8  *key_id, 
      SOC_SAND_OUT uint32  *key_bitmap_constraint /* [Key-D-MSB][Key-D-LSB]..[Key-A-MSB][Key-A-LSB] */
    )
{
  uint32
      pmf_pgm_ndx_min,
      pmf_pgm_ndx_max,
      pmf_pgm_ndx,
      keys_bmp,
    res = SOC_SAND_OK;
  uint8
      is_cascaded,
      lookup_id;
  ARAD_PMF_DB_INFO
      db_info;
  SOC_PPC_FP_DATABASE_STAGE
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *is_key_fixed = FALSE;
  *key_id = 0;
  *key_bitmap_constraint = 0;

  /* 
   * Take any stage since the DB info is set for all stages during the create API
   */
  /* Get the correct stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  /* Check if there is a key-change action or qualifier */
  res = arad_pp_fp_db_cascaded_cycle_get(
          unit,
          db_id_ndx,
          &is_cascaded,
          &lookup_id /* 1st or 2nd cycle */
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 23, exit);
  if (is_cascaded && (lookup_id == 1)) {
      /* Find a PMF-Program where the Database has already a Key */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
              unit,
              stage,
              db_id_ndx,
              &db_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 150, exit);

      /* 
       * See if the cascaded key is allocated: 
       * 1. Explicitly in the SW DB 
       * 2. If not, see if there is a possible key between all the PMF-Programs 
       * 3. If not, return an error 
       */ 
      if (db_info.cascaded_key) {
          *is_key_fixed = TRUE;
          *key_id = ARAD_PP_FP_KEY_FROM_CASCADED_KEY_GET(db_info.cascaded_key);
      }
      else {
          *is_key_fixed = FALSE;
          /* intersection of the keys */
          res = arad_pmf_prog_select_pmf_pgm_borders_get(
                    unit,
                    stage,
                    0 /* is_for_tm */, 
                    &pmf_pgm_ndx_min,
                    &pmf_pgm_ndx_max
                  );
          SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);

          for (pmf_pgm_ndx = pmf_pgm_ndx_min; pmf_pgm_ndx < pmf_pgm_ndx_max; ++pmf_pgm_ndx)
          {
              if (SHR_BITGET(db_info.progs, pmf_pgm_ndx)) {
                  /* Relevant program. OR on the taken keys */
                  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(unit, stage, pmf_pgm_ndx, lookup_id /* 1 */, &keys_bmp);
                  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
                  *key_bitmap_constraint |= keys_bmp;
              }
          }
      }
  }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_key_cascaded_key_get()", 0, 0);
}

/* 
 * See if this Database: 
 * - is compare 
 * - if so, if 2nd cycle 
 * - if so, try to allocate Key-D 
 */
uint32
    arad_pp_fp_key_alloc_key_is_equal_get(
      SOC_SAND_IN int    unit,
      SOC_SAND_IN uint32    db_id_ndx,
      SOC_SAND_OUT uint8    *is_key_fixed,
      SOC_SAND_OUT uint8    *key_id,
      SOC_SAND_OUT uint32   *key_place,
      SOC_SAND_OUT uint32   *key_bitmap_constraint /* [Key-D-MSB][Key-D-LSB]..[Key-A-MSB][Key-A-LSB] */
    )
{
    uint32
        pmf_pgm_ndx_min,
        pmf_pgm_ndx_max,
        pmf_pgm_ndx,
        keys_bmp,
        place_cons,
        res = SOC_SAND_OK;
    uint8
        is_equal;
    ARAD_PMF_DB_INFO
        db_info;
    SOC_PPC_FP_DATABASE_STAGE
        stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *is_key_fixed = FALSE;
    *key_id = 0;
    *key_place = 0;
    *key_bitmap_constraint = 0;

    /* 
    * Take any stage since the DB info is set for all stages during the create API
    */
    /* Get the correct stage */
    res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 19, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
            unit,
            stage,
            db_id_ndx,
            &db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 150, exit);

    /* Check if there is a key-change action or qualifier */
    res = arad_pp_fp_db_is_equal_place_get(
            unit,
            db_id_ndx,
            &is_equal,
            &place_cons /* LSB or MSB */
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 29, exit);

    /* 
    * 1. Check if Key-D (second cycle) is available in all relevant PMF-Programs 
    * 2. If not, return an error 
    */ 
    if (is_equal) 
    {
      *is_key_fixed = TRUE;
      *key_id = 3;
      *key_place = place_cons;

      /* intersection of the keys */
      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                unit,
                stage,
                0 /* is_for_tm */, 
                &pmf_pgm_ndx_min,
                &pmf_pgm_ndx_max
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 39, exit);

      for (pmf_pgm_ndx = pmf_pgm_ndx_min; pmf_pgm_ndx < pmf_pgm_ndx_max; ++pmf_pgm_ndx)
      {
          if (SHR_BITGET(db_info.progs, pmf_pgm_ndx)) 
          {
              /* Relevant program. OR on the taken keys */
              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(
                        unit, 
                        stage, 
                        pmf_pgm_ndx, 
                        1 /* cycle */, 
                        &keys_bmp
                    );
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 49, exit);

              *key_bitmap_constraint |= keys_bmp;
          }
      }
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_key_is_equal_get()", 0, 0);
}



/* Do not use the SW DB: the bitmap is given explicitly */

/* 
 * Set / unset the 80b's part bitmap: 
 * [Key-D-MSB][Key-D-LSB]..[Key-A-MSB][Key-A-LSB] 
 */
uint32
  arad_pp_fp_key_alloc_key_bitmap_configure(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 db_id, 
      SOC_SAND_IN  uint32                 alloc_place, /* 1, 2, 3, 4 half-parts to take */
      SOC_SAND_IN  uint8                  is_to_set, /* 1 for set, 0 for unset */
      SOC_SAND_INOUT  uint32                 *keys_bmp, 
      SOC_SAND_INOUT  uint32                *key_id, /* IN if is_to_set = FALSE, OUT otherwise */
      SOC_SAND_OUT  uint8                 *found
  )
{
    uint32
        nof_bits_option, /* number of possible valid bits to consider for this configuration */
        nof_bits_to_set = 0, /* number of valid bits to set for this configuration */
        bit_id = 0, /* valid bit id */
        bit_ndx, /* bit id for loop */
        place_ndx,
        key_bitmap_constraint_cascaded = 0,
        key_bitmap_constraint_compare = 0,
        nof_80s = 0,
        key_place_compare,
        keys_bmp_tmp,
        key_bmp_ndx = 0,
        res,
      index; /* loop on all the combinations */
    uint8
        key_ndx_cascaded,
        key_ndx_compare,
        is_key_fixed_cascaded,
        is_key_fixed_compare,
        key_bitmap_constraint_dir_ext = FALSE,
        key_ndx_dir_ext,
        is_for_upper_80,
        set_config;
    SOC_PPC_FP_DATABASE_INFO
    fp_database_info; 

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

      /* Get the DB info */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
              unit,
              SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, /* All stages equivalent in use */
              db_id,
              &fp_database_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    /* Translate reversely from start-place */
    for(place_ndx = 0; place_ndx < ARAD_PP_FP_NOF_KEY_CE_PLACES_STAGE; place_ndx++) {
      if (Arad_pp_fp_place_const_convert[place_ndx][0] == alloc_place) {
          /* Take its attributes */
          nof_80s = Arad_pp_fp_place_const_convert[place_ndx][3];
      }
    }

    nof_bits_option = (nof_80s == ARAD_PP_FP_KEY_NUMBER_80_PARTS_IN_320B)? 
        (ARAD_PMF_LOW_LEVEL_NOF_KEYS / 2) /* (0,1) or (2,3) */: ARAD_PMF_LOW_LEVEL_NOF_KEYS /* 4 keys */;
    is_for_upper_80 = (alloc_place == ARAD_PP_FP_KEY_CE_HIGH);
    for(index = 0; index < nof_bits_option; ++index)
    {
      /* if this bit is available according to constraint */
        if (nof_80s == 1) {
            bit_id =  (2 * index) + is_for_upper_80; 
            nof_bits_to_set = 1;
            if (is_to_set) {
              *key_id = index;
            }
            else if (*key_id != index) {
                /* Find the good index of this key */
                continue;
            }
        }
        else if (nof_80s == 2) {
            bit_id =  2 * index;  /* key Id */
            nof_bits_to_set = 2;
            if (is_to_set) {
                *key_id = index;
            }
            else if (*key_id != index) {
                continue;
            }
        }
        /* In case of 2 keys, they must be adjacent and (0,1) or (2,3) */
        else if (nof_80s == 4) {
          bit_id = index * 4; /* 0 or 2*/
          nof_bits_to_set = 4;
          if (is_to_set) {
              *key_id = index * 2;
          }
          else if (*key_id != (index * 2)) {
              continue;
          }
        }

        /* See if the configuration can be set: all the parts are free */
        set_config = TRUE;
        key_bitmap_constraint_dir_ext = FALSE;

        /* If optimized direct extraction allocatyion mode, check which key index to choose: choose key which the other 80-bit half is already allocated */
        if (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION  && soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "optimized_de_allocation", 0)) {
            keys_bmp_tmp = *keys_bmp;
            for (key_bmp_ndx = 0 ; key_bmp_ndx < ARAD_PMF_LOW_LEVEL_NOF_KEYS ; key_bmp_ndx++)
            {
                if ( (is_for_upper_80 && (keys_bmp_tmp == 0x1) ) || (!is_for_upper_80 && (keys_bmp_tmp == 0x2)) )
                {
                    key_ndx_dir_ext = key_bmp_ndx;
                    key_bitmap_constraint_dir_ext = TRUE;
                    break;
                }
                keys_bmp_tmp = keys_bmp_tmp >> 2;

            }
            /* If a key bitmap constraint is found and it is not the index - continue until the right index is found*/
            if (key_bitmap_constraint_dir_ext && key_ndx_dir_ext != *key_id) 
            {
                set_config = FALSE;
                continue;
            }
        }

        /* in case of cascaded 2nd lookup, force the Key-ID since it is written in the TCAM action */
        res = arad_pp_fp_key_alloc_key_cascaded_key_get(
                unit,
                db_id,
                &is_key_fixed_cascaded,
                &key_ndx_cascaded,
                &key_bitmap_constraint_cascaded /* 0 if not cascaded */
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if (SOC_IS_ARADPLUS(unit))
        {
            /* Compare requires allocation of a specific key
             * (Key D, second cycle). All programs are checked 
             * to verify it is available.  
             */
            res = arad_pp_fp_key_alloc_key_is_equal_get(
                    unit,
                    db_id,
                    &is_key_fixed_compare,
                    &key_ndx_compare,
                    &key_place_compare,
                    &key_bitmap_constraint_compare /* 0 if not compare */
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        
            /* Continue until key-D is checked. */
            if (is_key_fixed_compare) 
            {
                if (*key_id != key_ndx_compare)
                {
                    set_config = FALSE;
                    continue;
                }
            }
        }
        /* Cascaded database requires allocation of a specific key
         * if it is already in the TCAM  and it should be the same 
         * key for all programs.
         */
        if (is_key_fixed_cascaded) 
        {
            if (*key_id != key_ndx_cascaded)
            {
                set_config = FALSE;
                continue;
            }

            if (SOC_IS_ARADPLUS(unit))
            {
                if (is_key_fixed_compare 
                    && (key_ndx_cascaded != key_ndx_compare)) 
                {
                    set_config = FALSE;
                    break;
                }
            }
        }
        else {
            *keys_bmp |= key_bitmap_constraint_cascaded | key_bitmap_constraint_compare;
            for(bit_ndx = 0; bit_ndx < nof_bits_to_set; ++bit_ndx)
            {
                if(SHR_BITGET(keys_bmp, bit_id + bit_ndx)){
                    set_config = FALSE;
                    break;
                }
            }
        }

        if (set_config || (!is_to_set)) {
            *found = 1;
            for(bit_ndx = 0; bit_ndx < nof_bits_to_set; ++bit_ndx)
            {
                if (is_to_set) {
                    SHR_BITSET(keys_bmp, bit_id + bit_ndx);
                }
                else {
                    SHR_BITCLR(keys_bmp, bit_id + bit_ndx);
                }
            }
            break;
        }

    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_key_bitmap_configure()", 0, 0);
}



/* 
 * Get a Bitmap of the 80b's part to see what is free: 
 * [Key-D-MSB][Key-D-LSB]..[Key-A-MSB][Key-A-LSB] 
 */
uint32
  arad_pp_fp_key_alloc_key_id(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 prog_ndx,
      SOC_SAND_IN  uint32                 db_id,
      SOC_SAND_IN  uint32                 cycle_ndx, /* 0, 1*/
      SOC_SAND_IN  uint32                 alloc_place, /* 1, 2, 3, 4 half-parts to take */
      SOC_SAND_IN  uint32                 flags,
      SOC_SAND_OUT  uint32                *key_id, 
      SOC_SAND_OUT  uint8                 *found
  )
{
    uint32
      key_id_lcl = 0,
      keys_bmp = 0,
      tmp_key_a_bmp = 0,
      res = SOC_SAND_OK;
    ARAD_PMF_DB_INFO
       db_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *found = 0;

    /* allocate according to SW DB */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(
            unit,
            stage,
            prog_ndx,
            cycle_ndx,
            &keys_bmp  
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    /* 
     * Arad: Never allocate Key-A cycle-0 in Ingress PMF because not support of interleaved PD 
     * Jericho: the HW has been changed - key-A cycle-0  has a regular TCAM lookup in bank mode
     */
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) && (cycle_ndx == 0) && !(flags & ARAD_PP_FP_KEY_ALLOC_USE_KEY_A))  {
            tmp_key_a_bmp = ( keys_bmp & 0x3 ); /*store key-A bitmap*/
            keys_bmp |= 0x3; /* Key-A taken */
        }
    }

    res = arad_pp_fp_key_alloc_key_bitmap_configure(
            unit,
            stage,
            db_id,
            alloc_place,
            TRUE, /* is_to_set */
            &keys_bmp, 
            &key_id_lcl, 
            found
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
    *key_id = key_id_lcl;

    /* if found update allocated bit */
    if(*found) {
        if (!(flags & ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY)) {
            if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
                if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) && (cycle_ndx == 0) && !(flags & ARAD_PP_FP_KEY_ALLOC_USE_KEY_A))  
                {
                   keys_bmp = (keys_bmp & ~(0x3) ) | tmp_key_a_bmp ;  /*restore original key-A bitmap */
                }
            }
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.set(
                    unit,
                    stage,
                    prog_ndx,
                    cycle_ndx,
                    keys_bmp  
                  );
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

            /* Set the cascaded Key */
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(unit, stage, db_id, &db_info);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
            db_info.cascaded_key = ARAD_PP_FP_KEY_TO_CASCADED_KEY_GET((*key_id));
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.set(unit, stage, db_id, &db_info);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

        }
    }


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_key_id()", 0, 0);
}


uint32
  arad_pp_fp_key_alloc_ce(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                 prog_ndx,
      SOC_SAND_IN  uint32                 cycle_ndx, /* 0, 1*/
      SOC_SAND_IN  uint32                 place_cons,/* low, high, any*/
      SOC_SAND_IN  uint32                 size_cons, /* 16, 32, any*/
      SOC_SAND_IN  uint32                 group_cons, /* 0, 1, any*/
      SOC_SAND_IN  uint32                 flags, /* see ARAD_PP_FP_KEY_ALLOC_CE_xxx */
      SOC_SAND_INOUT  uint32              *ce_rsrc_bmp, /* see ARAD_PP_FP_KEY_ALLOC_CE_xxx */
      SOC_SAND_INOUT  ARAD_PP_KEY_CE_ID   *cur_ce,
      SOC_SAND_OUT  uint8                 *found
  )
{
    uint32
      fit_ce[1] = {0},
      ce_rsrc_lcl[1],
      ce_ndx,
      index;
    uint8
      in_use;
    uint32
      res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    *found = FALSE;
    *fit_ce = arad_pp_fp_key_get_available_ce_under_constraint(unit, stage, place_cons,size_cons,group_cons);
    if(flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP) {
        *fit_ce = *fit_ce & (~(*ce_rsrc_bmp)); /* only not allocated */
    }

    /* if need to allocate given CE, check it's possible in constrains and available */
    if(flags & ARAD_PP_FP_KEY_ALLOC_CE_WITH_ID) {
        index = *cur_ce;
        if(!(SHR_BITGET(fit_ce,index))){
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "Unit %d CE %d : cannot allocate given CE.\n\r"), unit, index));
            goto exit;
        }

        /* bit is fit so can be used */
        if(flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP) {
            in_use = FALSE;
        }
        else{
            /* check if entry in use */
            res = arad_pp_fp_key_ce_is_in_use(
                    unit,
                    stage,
                    prog_ndx,
                    cycle_ndx,
                    index,
                    &in_use
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }

        if(!in_use) {
            *found = TRUE;
        }
    }
    else{ /* in case CE is not given try to find first available */
        for(ce_ndx = 0; ce_ndx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG; ++ce_ndx)
        {

            if (flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) {
                /* In FLP, start from the LSB copy engines */
                index = ce_ndx;
            }
            else {
                /* Allocate the first qualifiers in the LSBs - a must for the cascaded qualifier*/
                index = ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG - ce_ndx - 1;
            }

          /* if this bit is available according to constraint */
          /* coverity explaination:the value of index will never beyond 32*/
          /* coverity[overrun-local:FALSE] */
          if(!SHR_BITGET(fit_ce,index)){
              continue;
          }

          if(flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP) {
              in_use = FALSE;
          }
          else{
              res = arad_pp_fp_key_ce_is_in_use(
                      unit,
                      stage,
                      prog_ndx,
                      cycle_ndx,
                      index,
                      &in_use
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
          }

          /* not in use, use it */
          if(!in_use) {
              *cur_ce = index;
              *found = TRUE;
              break;
          }
        }
    }

    /* if found CE, and flags indicate to use rsrc-bitmp then update it */
    if(*found && (flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP)) {
        SHR_BITSET(ce_rsrc_bmp,*cur_ce);
    }
    /* if found CE, and flags indicate to mark in use update use status */
    if(*found == TRUE && !(flags & ARAD_PP_FP_KEY_ALLOC_CE_CHECK_ONLY)) {
        res = arad_pp_fp_key_ce_set_use(
                unit,
                stage,
                prog_ndx,
                cycle_ndx,
                *cur_ce,
                in_use
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        /* Get - set the SW DB because it may be different from ce_rsrc_bmp (no 32b CE for ex.) */
        /* get CE resource status */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(
                  unit,
                  stage,
                  prog_ndx,
                  cycle_ndx,
                  ce_rsrc_lcl
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
        /* coverity explaination:the value of index will never beyond 32*/
        /* coverity[overrun-local:FALSE] */
        SHR_BITSET(ce_rsrc_lcl,*cur_ce);
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.set(
                  unit,
                  stage,
                  prog_ndx,
                  cycle_ndx,
                  *ce_rsrc_lcl
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
    }
     

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_ce()", 0, 0);
}

/* sort programs to optimize allocation */
/* currently do  nothing */


/*
 * write CE change to HW/SW upon successful allocation
 */
STATIC
 uint32
  arad_pp_fp_key_alloc_in_prog_commit(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
    SOC_SAND_IN  uint32                       prog_ndx,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX], /* qualifiers*/
    SOC_SAND_INOUT  ARAD_PP_FP_KEY_ALLOC_INFO   *alloc_info
  )
{
    uint32
      res = SOC_SAND_OK,
      ce_indx=0,
      cascaded_key_bitmap_constraint, /* Here used as dummy argument */
      access_profile_array_id,
      nof_access_profiles,
      offset_start=0,
      access_profile_id,
      place_cons,
      nof_bits_zone[ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX][2],
      prog_used_cycle_bmp_lcl[1];
    uint8
        is_cascaded,
        lookup_id,
        cascaded_is_key_fixed,
        cascaded_key_id, 
        is_equal = 0,
        found = 0xff, /* Assigning all 1s, in case we accidently access it without assigning real value */
        is_header = FALSE,
        ce_msb,
        is_320b=0,
        is_slb_hash_in_quals = FALSE;
    ARAD_PMF_CE_HEADER_QUAL_INFO
        hdr_qual_info;
    ARAD_PMF_CE_PACKET_HEADER_INFO
        cur_hdr_offset;
    ARAD_PMF_CE_IRPP_QUALIFIER_INFO
        inter_qual_info,
        cur_inter_qual_info;
    SOC_PPC_FP_QUAL_TYPE  
        cascaded_qual_type,              
        qual_type;
    ARAD_PMF_CE
        sw_db_ce;
    ARAD_PMF_DB_INFO /* update db info */
        pmf_db_info; 
    ARAD_TCAM_BANK_ENTRY_SIZE
        entry_size;
    ARAD_PP_IHB_PMF_PASS_2_KEY_UPDATE_TBL_DATA
        pass_2_key_update_tbl_data;
    SOC_PPC_FP_DATABASE_INFO
        db_info;
    soc_error_t soc_res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_os_memset(nof_bits_zone, 0x0, ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX * 2 * sizeof(uint32));
    ARAD_PMF_CE_PACKET_HEADER_INFO_clear(&cur_hdr_offset);

    /* Get the DB info */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
            unit,
            stage, 
            db_id,
            &db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    /* write all CEs*/
    for(ce_indx = 0; ce_indx < alloc_info->nof_ce; ++ce_indx) {

        qual_type = alloc_info->qual_type[ce_indx];
        ce_msb = alloc_info->ce[ce_indx] >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB;

        /* get CE info, if contain different qual-type */
        /* 
         * Get the qualifier information only at the first Copy engine 
         * of this qualifier. 
         * For header: compute offset and nof_bits 
         * For internal:  compute nof_bits 
         */
        res = arad_pmf_ce_header_info_find(
                 unit,
                 qual_type,
                 stage,
                 &found,
                 &hdr_qual_info
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

        /* calc header placement */
        if(found) {
            is_header = TRUE;
            
            /*decide which header index */
             cur_hdr_offset.sub_header = ((flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_HEADER_SELECTION) && 
                                          ((hdr_qual_info.qual_type == SOC_PPC_FP_QUAL_HDR_INNER_SA) || (hdr_qual_info.qual_type == SOC_PPC_FP_QUAL_HDR_INNER_DA) ) ) ? 
                                          hdr_qual_info.header_ndx_1 : hdr_qual_info.header_ndx_0;
        }
        /* calc internal placement */
        else{
            is_header = FALSE;
            res = arad_pmf_ce_internal_field_info_find(
                     unit,
                     qual_type,
                     stage,
                     ce_msb,
                     &found,
                     &inter_qual_info
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
            if(found == 0) {
               LOG_ERROR(BSL_LS_SOC_FP,
                         (BSL_META_U(unit,
                                     "   "
                                     "CE HW commit: "
                                     "qual_type %s unsupported \n\r"), SOC_PPC_FP_QUAL_TYPE_to_string(qual_type)));
                SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 60, exit); /* unexpected !*/
            }

            sal_memcpy(&cur_inter_qual_info,&inter_qual_info,sizeof(ARAD_PMF_CE_IRPP_QUALIFIER_INFO));
        }

        /* 
         * Main steps of the computation: 
         * - see if all the bits can be inserted in this Copy Engine 
         * - see if there are lost bits 
         * - if so: 
         *      - see if they can be included inside
         *      - otherwise, reduce the number of copied bits, all relevant
         * -  update the SW DB CE, where: 
         *      - LSB is the start of intersting bits in the Copy Engine
         *      - MSB is the size-1 of the Copy Engine
         *      - Qual-lsb is the offset inside the qualifier to see bits in the Copy Engine
         *  
         * Copy from the MSB of the value (header and internal) and get closer to the LSB 
         * with the iterations. 
         */
        if(is_header) {

          /* fill cur header offset */
            cur_hdr_offset.nof_bits = alloc_info->act_ce_const[ce_indx].nof_bits;
            /* Compute the offset from the bit 0 (LSB) which is the qual_lsb = 0 */
            cur_hdr_offset.offset = hdr_qual_info.lsb - alloc_info->act_ce_const[ce_indx].qual_lsb - 
                (alloc_info->act_ce_const[ce_indx].nof_bits - alloc_info->act_ce_const[ce_indx].lost_bits - 1);
            offset_start = cur_hdr_offset.offset;

            sw_db_ce.lsb = alloc_info->act_ce_const[ce_indx].lost_bits;
            sw_db_ce.qual_lsb = alloc_info->act_ce_const[ce_indx].qual_lsb;

            if (!(flags&ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY)) {
                /* write to CE HW*/
                res = arad_pmf_ce_packet_header_entry_set_unsafe(
                        unit,
                        stage,
                        prog_ndx,
                        alloc_info->key_id[alloc_info->ce_key[ce_indx]],
                        alloc_info->ce[ce_indx]%ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB, /* ce in lsb/msb*/
                        ce_msb, /* is msb*/
                        (alloc_info->cycle == ARAD_PP_FP_KEY_CYCLE_EVEN)?0:1, /* lookup num*/
                        &cur_hdr_offset
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
            }

            /* write to CE SW*/
            /* qual_lsb is how much I wanted to copy - how I copy */ 
            sw_db_ce.db_id = db_id;
            sw_db_ce.is_used = 1;
            sw_db_ce.msb = cur_hdr_offset.nof_bits-1; /* number of used in CE or max CE size*/
            sw_db_ce.qual_type = qual_type;
            sw_db_ce.is_msb = ce_msb;
            sw_db_ce.is_second_key = alloc_info->ce_key[ce_indx];
            nof_bits_zone[sw_db_ce.is_second_key][ce_msb] += cur_hdr_offset.nof_bits;
            /* Simple way to know if this is 320b DB */
            if (sw_db_ce.is_second_key) {
                is_320b = 1;
            }
            if (!(flags&ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY)) {
                LOG_DEBUG(BSL_LS_SOC_FP,
                          (BSL_META_U(unit,
                                      "   "
                                      "CE HW commit: "
                                      "qual-lsb %d, "
                                      "qual-size %d, ce-size:%d, already-coppied:%d \n\r"), 
                           sw_db_ce.qual_lsb, cur_hdr_offset.nof_bits,cur_hdr_offset.nof_bits,(cur_hdr_offset.offset - offset_start)));
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.set(
                        unit,
                        stage,
                        prog_ndx,
                        (alloc_info->cycle == ARAD_PP_FP_KEY_CYCLE_EVEN)?0:1, /* lookup num*/
                        alloc_info->ce[ce_indx], /* ce in lsb/msb*/
                        &sw_db_ce
                      );
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);
            }
        }
        else{ /* internal qual */

            /* fill cur internal info */
            cur_inter_qual_info.info.qual_nof_bits = alloc_info->act_ce_const[ce_indx].nof_bits;

            /* Start by the qualifier MSBs */
            cur_inter_qual_info.info.buffer_lsb = alloc_info->act_ce_const[ce_indx].qual_lsb; /* Qual-lsb in fact */

            /* Where the relevant bits begin */
            sw_db_ce.lsb = alloc_info->act_ce_const[ce_indx].lost_bits; 

            if (!(flags&ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY)) {
                if (!(SOC_IS_JERICHO_PLUS(unit) && qual_type == SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED)) {
                    /* In JER+/QAX, there's no need to allocate CE for cascaded, since
                     * the update_key memory takes care of allocating a CE for it */

                    /* write to CE HW*/
                    res = arad_pmf_ce_internal_info_entry_set_unsafe(
                            unit,
                            stage,
                            prog_ndx,
                            alloc_info->key_id[alloc_info->ce_key[ce_indx]],
                            alloc_info->ce[ce_indx]%ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB, /* ce in lsb/msb*/
                            ce_msb, /* is msb*/
                            (alloc_info->cycle == ARAD_PP_FP_KEY_CYCLE_EVEN)?0:1, /* lookup num*/
                            FALSE, /* is_update_key */
                            cur_inter_qual_info.info.buffer_lsb,
                            alloc_info->act_ce_const[ce_indx].lost_bits,
                            cur_inter_qual_info.info.qual_nof_bits,
                            qual_type
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
                }
            }

            /* write to CE SW*/
            sw_db_ce.db_id = db_id;
            sw_db_ce.is_used = 1;
            sw_db_ce.qual_lsb = cur_inter_qual_info.info.buffer_lsb;
            sw_db_ce.msb = cur_inter_qual_info.info.qual_nof_bits-1; /* number  used in CE or max CE size*/
            sw_db_ce.qual_type = qual_type;
            sw_db_ce.is_msb = ce_msb;
            sw_db_ce.is_second_key = alloc_info->ce_key[ce_indx];
            nof_bits_zone[sw_db_ce.is_second_key][ce_msb] += cur_inter_qual_info.info.qual_nof_bits;

            /* Simple way to know if this is 320b DB */
            if (sw_db_ce.is_second_key) {
                is_320b = 1;
            }

            if (!(flags&ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY)) {
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.set(
                        unit,
                        stage,
                        prog_ndx,
                        (alloc_info->cycle == ARAD_PP_FP_KEY_CYCLE_EVEN)?0:1, /* lookup num*/
                        alloc_info->ce[ce_indx], /* ce in lsb/msb*/
                        &sw_db_ce
                      );
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 110, exit);
            }
        }
    }

    if (!(flags&ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY)) {
        /* Compute the number of access profiles: 320b key or large action size */
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit,ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id), &entry_size);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 115, exit);
        nof_access_profiles = 0;
        if ((db_info.db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) 
            && (db_info.db_type != SOC_PPC_FP_DB_TYPE_FLP)
            && (db_info.db_type != SOC_PPC_FP_DB_TYPE_SLB)) {
            nof_access_profiles = (is_320b || (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS))? 2 : 1; /* Number of keys */
        }
        alloc_info->use_kaps = ( db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) ? TRUE : FALSE ;
        for (access_profile_array_id = 0; access_profile_array_id < nof_access_profiles; access_profile_array_id++) {
            /* map DB = <program x key> to access-profile */
            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(
                    unit,
                    ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id),
                    access_profile_array_id,
                    &access_profile_id
                  ); 
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 120, exit);

            res = arad_pp_fp_key_access_profile_hw_set(
                    unit,
                    stage,
                    prog_ndx,
                    access_profile_array_id,
                    access_profile_id,
                    alloc_info
                  );
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 145, exit);
        }


        /* 
         * For 1st cycle cascaded Databases, set the source Database 
         * of this PMF-Program to take the action 
         */
        /* Check if there is a key-change action or qualifier */
        res = arad_pp_fp_db_cascaded_cycle_get(
                unit,
                db_id,
                &is_cascaded,
                &lookup_id /* 1st or 2nd cycle */
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);

        if (SOC_IS_ARADPLUS(unit))
        {
            /* Check if there is a key-change action or qualifier */
            res = arad_pp_fp_db_is_equal_place_get(
                    unit,
                    db_id,
                    &is_equal,  /* is compare db */
                    &place_cons /* LSB/MSB */
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);

            /* Check whether there is a hash value qualifier (data from SLB 74b LEM key). */
            soc_res = arad_pp_fp_is_qual_in_qual_arr(unit, 
                                                     qual_types, 
                                                     ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS, 
                                                     SOC_PPC_FP_QUAL_KEY_AFTER_HASHING, 
                                                     &is_slb_hash_in_quals);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(soc_res, 151, exit);
        }

        if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) &&
            ((is_cascaded && (lookup_id == 0)) || is_equal || is_slb_hash_in_quals))
        {
            

            if (!SOC_IS_JERICHO_PLUS(unit))
            {
                res = arad_pp_ihb_pmf_pass_2_key_update_tbl_get_unsafe(
                        unit,
                        prog_ndx,
                        &pass_2_key_update_tbl_data
                  );
                SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);

            }


            if (is_cascaded && (lookup_id == 0)) 
            {
                /* For Jericho+ and QAX, the update key HW has changed.
                 * In order to support Legacy cascading, as it is performed 
                 * in Arad and Jericho devices, it is necessary to use a CE 
                 * from update key (an additional key that was added specifically 
                 * for rewriting the second pass key, with access to inputs such 
                 * as keys and results from first pass.
                 */
                if (SOC_IS_JERICHO_PLUS(unit)) 
                {
                    res = arad_pp_fp_key_alloc_key_cascaded_key_get(
                            unit,
                            db_info.cascaded_coupled_db_id,
                            &cascaded_is_key_fixed,
                            &cascaded_key_id,
                            &cascaded_key_bitmap_constraint
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

                    if (!cascaded_is_key_fixed) {
                      LOG_ERROR(BSL_LS_SOC_FP,
                                (BSL_META_U(unit,
                                            "   Error in cascaded configuration: "
                                            "For database %d, stage %s, the database is indicated to be cascaded with DB-Id %d."
                                            "This latest Database is not set as cascaded. \n\r"), 
                                 db_id, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), db_info.cascaded_coupled_db_id));
                        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 50, exit); /* must have active program */
                    }

                    if(db_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE && 
                       (db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS)) 
                    {
                        cascaded_qual_type = SOC_PPC_FP_QUAL_IRPP_KAPS_PASS1_PAYLOAD;
                    }
                    else
                    {
                        cascaded_qual_type = SOC_PPC_FP_QUAL_IRPP_TCAM_0_RESULT + alloc_info->key_id[0];
                    }

                    /* write to CE HW*/
                    res = arad_pmf_ce_internal_info_entry_set_unsafe(
                            unit,
                            stage,
                            prog_ndx,
                            cascaded_key_id,
                            4, 
                            FALSE, /* is msb*/
                            1, /* lookup num*/
                            TRUE, /* is_update_key */
                            0,
                            0,
                            0,
                            cascaded_qual_type
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

                } else {
                    pass_2_key_update_tbl_data.enable_update_key = 0x1;
                    pass_2_key_update_tbl_data.action_select = alloc_info->key_id[0]; /* First Key-ID */
                }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
                if(SOC_IS_JERICHO(unit) && ( db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) )
                {
                    pass_2_key_update_tbl_data.action_select = ARAD_PP_IHB_PMF_PASS_2_KEY_UPDATE_ACTION_SELECT_KAPS;
                }
#endif 
            }
            if (SOC_IS_ARADPLUS(unit))
            {
                if (is_equal) 
                {
                    soc_reg_above_64_val_t reg_above_64, field_above_64;
                    uint32 xor_mask, xor_input_selection;

                    if (SOC_IS_JERICHO_PLUS(unit))
                    {
                        SOC_REG_ABOVE_64_CLEAR(reg_above_64);
                        SOC_REG_ABOVE_64_CLEAR(field_above_64);
                        SOC_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, READ_IHB_XOR_MASKSr(unit, reg_above_64));
                        SHR_BITSET_RANGE(field_above_64, 0, 80);
                        soc_reg_above_64_field_set(unit, IHB_XOR_MASKSr, reg_above_64, XOR_MASK_3f, field_above_64);
                        SOC_SAND_SOC_IF_ERROR_RETURN(res, 166, exit, WRITE_IHB_XOR_MASKSr(unit, reg_above_64));

                        SOC_REG_ABOVE_64_CLEAR(reg_above_64);
                        SOC_REG_ABOVE_64_CLEAR(field_above_64);
                        SOC_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, READ_IHB_FEM_FES_COMPARE_SELECTIONr(unit, reg_above_64));
                        soc_reg_above_64_field_get(unit, IHB_FEM_FES_COMPARE_SELECTIONr, reg_above_64, XOR_MASK_SELECTf, field_above_64);
                        xor_mask = 0x3;
                        SHR_BITCOPY_RANGE(field_above_64,(prog_ndx*2),&xor_mask,0,2);
                        soc_reg_above_64_field_set(unit, IHB_FEM_FES_COMPARE_SELECTIONr, reg_above_64, XOR_MASK_SELECTf, field_above_64);
                        SOC_SAND_SOC_IF_ERROR_RETURN(res, 166, exit, WRITE_IHB_FEM_FES_COMPARE_SELECTIONr(unit, reg_above_64));

                        SOC_REG_ABOVE_64_CLEAR(reg_above_64);
                        SOC_REG_ABOVE_64_CLEAR(field_above_64);
                        SOC_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, READ_IHB_FEM_FES_COMPARE_SELECTIONr(unit, reg_above_64));
                        soc_reg_above_64_field_get(unit, IHB_FEM_FES_COMPARE_SELECTIONr, reg_above_64, XOR_INPUT_SELECTf, field_above_64);
                        xor_input_selection = 0x2;
                        SHR_BITCOPY_RANGE(field_above_64,(prog_ndx*2),&xor_input_selection,0,2);
                        soc_reg_above_64_field_set(unit, IHB_FEM_FES_COMPARE_SELECTIONr, reg_above_64, XOR_INPUT_SELECTf, field_above_64);
                        SOC_SAND_SOC_IF_ERROR_RETURN(res, 166, exit, WRITE_IHB_FEM_FES_COMPARE_SELECTIONr(unit, reg_above_64));  
                    }
                    else
                    {
                        int core;

                        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
                            SOC_REG_ABOVE_64_CLEAR(reg_above_64);
                            SOC_REG_ABOVE_64_CLEAR(field_above_64);
                            SOC_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, READ_IHB_KEY_D_XOR_MASKSr(unit, core, reg_above_64));
                            SHR_BITSET_RANGE(field_above_64, 0, 80);
                            soc_reg_above_64_field_set(unit, IHB_KEY_D_XOR_MASKSr, reg_above_64, KEY_D_XOR_MASK_0f, field_above_64);
                            SOC_SAND_SOC_IF_ERROR_RETURN(res, 166, exit, WRITE_IHB_KEY_D_XOR_MASKSr(unit, core, reg_above_64));
                        }

                        pass_2_key_update_tbl_data.key_d_use_compare_result = 0x1;
                        pass_2_key_update_tbl_data.key_d_mask_select = 0x0;
                        pass_2_key_update_tbl_data.key_d_xor_enable = 0x1;
                    }
                }

                if (is_slb_hash_in_quals) {
                    if (SOC_IS_JERICHO_PLUS(unit)) {
                        uint32 reg;
                        /* Set 32bit CEs 4,5,6 to slb_hash offset */
                        uint32 data[5] = { 0, 0, 0xfe087dc8, 0xfe48, 0};
                        int key_idx = alloc_info->key_id[0];
                        int core;

                        /* set KEY_INST_VALID for CEs 4,5,6 and the relevant key
                         * Each key has 8 bits for valid CEs allocated for it
                         * 0x70 represents CEs 4,5,6*/
                        data[4] = 0x70 << (key_idx * 8);

                        SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) {
                            SOC_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, READ_IHB_PMF_GENERALr(unit, core, &reg));
                            soc_reg_field_set(unit, IHB_PMF_GENERALr, &reg, DISABLE_2ND_PASS_KEY_UPDATEf, 0);
                            SOC_SAND_SOC_IF_ERROR_RETURN(res, 165, exit, WRITE_IHB_PMF_GENERALr(unit, core, reg));
                        }
                        res = soc_mem_write(unit, IHB_PMF_UPDATE_KEY_GEN_MSBm, MEM_BLOCK_ANY, prog_ndx, data);
                        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 171, exit);

                    } else {
                        /* SLB hash key means the SLB 74b key must be copied to the high bits of the key. */
                        /* The key is always the first. */
                        /* This is controlled per program. */
                        uint32 *key_fields[4];
                        int i = 0;
                        int key_idx = alloc_info->key_id[0];

                        key_fields[i++] = &pass_2_key_update_tbl_data.key_a_lem_operation_select;
                        key_fields[i++] = &pass_2_key_update_tbl_data.key_b_lem_operation_select;
                        key_fields[i++] = &pass_2_key_update_tbl_data.key_c_lem_operation_select;
                        key_fields[i++] = &pass_2_key_update_tbl_data.key_d_lem_operation_select;

                        /* SOCDNX_VERIFY(0 <= key_idx && key_idx <= sizeof(key_fields) / sizeof(key_fields[0])); */
                        *(key_fields[key_idx]) = 1;
                    }
                 }
            }

            if (!SOC_IS_JERICHO_PLUS(unit))
            {
                res = arad_pp_ihb_pmf_pass_2_key_update_tbl_set_unsafe(
                        unit,
                        prog_ndx,
                        &pass_2_key_update_tbl_data
                  );
                SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);
            }
        }
    }

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
            unit,
            stage,
            db_id,
            &pmf_db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 180, exit);

    *prog_used_cycle_bmp_lcl = pmf_db_info.prog_used_cycle_bmp;
    if(alloc_info->cycle) {
        SHR_BITSET(prog_used_cycle_bmp_lcl, prog_ndx);
    }
    else{
        SHR_BITCLR(prog_used_cycle_bmp_lcl, prog_ndx);
    }
    pmf_db_info.prog_used_cycle_bmp = *prog_used_cycle_bmp_lcl;
    pmf_db_info.used_key[prog_ndx][0] = alloc_info->key_id[0];
    pmf_db_info.used_key[prog_ndx][1] = alloc_info->key_id[1];
    pmf_db_info.is_320b = is_320b;
    pmf_db_info.alloc_place = alloc_info->alloc_place;
    pmf_db_info.nof_bits_zone[0][0] = nof_bits_zone[0][0];
    pmf_db_info.nof_bits_zone[0][1] = nof_bits_zone[0][1];
    pmf_db_info.nof_bits_zone[1][0] = nof_bits_zone[1][0];
    pmf_db_info.nof_bits_zone[1][1] = nof_bits_zone[1][1];
    
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.set(
            unit,
            stage,
            db_id,
            &pmf_db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 190, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_in_prog_commit()", prog_ndx, ce_indx);
}

STATIC
 uint32
  arad_pp_fp_key_ce_rsrc_normalize(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                       flags,
      SOC_SAND_IN  uint32                       cur_rsrc
    )
{
    uint32
        tmp_rsrc[1],
        new_val,
        indx,
        new_rsrc_bmp=0;
    int
        set_count;

    *tmp_rsrc = cur_rsrc;
    /* found in each quarter number of set bits */
    
    for(indx = 0; indx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG; indx+=ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP) {
        /* how many set bit*/
        shr_bitop_range_count(tmp_rsrc, indx, ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP, &set_count);

        /* In case of exact order, skip the 32b CEs if not SLB */
        if ((stage != SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)
            && (flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER) 
            && arad_pmf_low_level_ce_is_32b_ce(unit, stage, indx)) {
            set_count = ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP;
        }

        /* write them in the beginning*/
        new_val = (1 << set_count) - 1;
        SHR_BITCOPY_RANGE(&new_rsrc_bmp, indx, &new_val, 0, ARAD_PMF_LOW_LEVEL_NOF_CE_16_IN_PROG_GROUP);  
    }
    return new_rsrc_bmp;
}


uint32
  arad_pp_fp_key_alloc_in_prog(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
    SOC_SAND_IN  uint32                       prog_ndx,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX], /* qualifiers*/
    SOC_SAND_IN  uint32                       cycle_cons, /* 0, 1, any*/
    SOC_SAND_IN  uint32                       place_cons,/* low, any: only low, or any*/
    SOC_SAND_IN  uint32                       start_place,/* low, any: try this first then any*/
    SOC_SAND_IN ARAD_PP_FP_CE_CONSTRAINT      ce_cons[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS], /* CE constraints*/
    SOC_SAND_IN  uint32                       nof_ce_const,
   /* out */
    SOC_SAND_OUT  ARAD_PP_FP_KEY_ALLOC_INFO   *alloc_info,
    SOC_SAND_OUT  uint8                       *key_alloced
  )
{
  int32
    alloc_exact = 1;
  uint32
      size_cons,
      group_cons,
      place_ndx,
      start_place_ndx = 0,
      end_place_ndx = 0,
      alloc_place_ndx,
      alloc_place = 0,
      cycle_first,
      cycle_last,
      cycle=0,
      ce_rsrc,
      key_id[2],
      nof_keys,
      ce_index;
  uint8
      success = FALSE,
      is_slb_hash_in_quals = FALSE;
  ARAD_PP_KEY_CE_ID
      ce_id;
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info; 
  soc_error_t 
    soc_res;
     
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *key_alloced = 0;
  ARAD_PP_FP_KEY_ALLOC_INFO_clear(alloc_info);

  /* Get the DB info */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
          unit,
          SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, /* All stages equivalent in use */
          db_id,
          &fp_database_info
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);


  /* 
   * if no constrain then from cycle 0 --> 1
   * if with constraint then from cycle x --> x (one cycle)
   */
  cycle_first = (cycle_cons != ARAD_PP_FP_KEY_CYCLE_ANY)? cycle_cons : ARAD_PP_FP_KEY_CYCLE_EVEN;
  cycle_last = (cycle_cons != ARAD_PP_FP_KEY_CYCLE_ANY)? cycle_cons : ARAD_PP_FP_KEY_CYCLE_ODD;

  /* Translate reversely from start-place */
  for(place_ndx = 0; place_ndx < ARAD_PP_FP_NOF_KEY_CE_PLACES_STAGE; place_ndx++) {
      if (Arad_pp_fp_place_const_convert[place_ndx][0] == start_place) {
          /* Good start_place_ndx found */
          start_place_ndx = place_ndx;
      }
      if (Arad_pp_fp_place_const_convert[place_ndx][0] == place_cons) {
          /* Good end_place_ndx found */
          end_place_ndx = place_ndx;
      }
  }

  /* start allocate in lsb,{lsb,msb}, if fail try to allocate on {lsb,msb} */
  for(alloc_place_ndx = start_place_ndx; alloc_place_ndx <= end_place_ndx; alloc_place_ndx++) {
      alloc_place = Arad_pp_fp_place_const_convert[alloc_place_ndx][0];
      nof_keys = Arad_pp_fp_place_const_convert[alloc_place_ndx][1];

      /* In case of Direct Extraction, try high then low and that's it */
      if (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) {
          /* For Direct extraction, do not allow a >16b qualifier to be split into 2 16b CEs */
          if (alloc_place_ndx == end_place_ndx) {
              alloc_place = ARAD_PP_FP_KEY_CE_LOW;
          }
      }

      if (SOC_IS_ARADPLUS(unit))
      {
          /* For TCAM or Direct Extraction databases, override
           * allocation place according to flag (LSB or MSB).
           */
          if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_LSB) {
              alloc_place = ARAD_PP_FP_KEY_CE_LOW;
          }
          else if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_MSB) {
              alloc_place = ARAD_PP_FP_KEY_CE_HIGH;
          }


      }

      
      for(alloc_exact = 1; alloc_exact >= 0; --alloc_exact) {
          for (cycle = cycle_first; cycle <= cycle_last; ++cycle) {

              LOG_DEBUG(BSL_LS_SOC_FP,
                        (BSL_META_U(unit,
                                    "   "
                                    "allocating key + CE for db: %d program: %d, "
                                    "alloc_place(high|low|high|low): %d alloc_exact (no, yes): %d cycle(0,1,..):%d \n\r"),
                         db_id, prog_ndx, alloc_place, alloc_exact, cycle));

              /* get CE resource status */
              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(
                        unit,
                        stage,
                        prog_ndx,
                        cycle,
                        &ce_rsrc
                    );
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
              /* Insert the CE constraints, e.g. when using only 16b CEs */
              ce_rsrc |= arad_pp_fp_key_ce_rsrc_normalize(unit, stage, flags, 0 /*cur_rsrc1*/);

              /* In SLB, set to 0 since always one Field group per pre/post-hash key */
              if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB) {
                  ce_rsrc = 0;
              }

              /* Check whether there is a hash value qualifier (data from SLB 74b LEM key). */
              soc_res = arad_pp_fp_is_qual_in_qual_arr(unit, 
                                                       qual_types, 
                                                       SOC_PPC_FP_NOF_QUALS_PER_DB_MAX, 
                                                       SOC_PPC_FP_QUAL_KEY_AFTER_HASHING, 
                                                       &is_slb_hash_in_quals);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(soc_res, 151, exit);

              if (is_slb_hash_in_quals) {
                  alloc_place = ARAD_PP_FP_KEY_CE_HIGH;
              }

			  /* allocate key in program */
              res = arad_pp_fp_key_alloc_key_id(
                       unit,
                       stage,
                       prog_ndx,
                       db_id,
                       cycle,
                       alloc_place, /* Number of half keys taken: 1, 2, 3, 4 */
                       flags|ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY, /* Check-only: commit in case of success */
                       &key_id[0],
                       &success
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
              if(!success) { /* if didn't find try in next cycle */
                  continue;
              }
              else if (nof_keys > 1) {
                  key_id[1] = key_id[0] + 1; /* The 2 keys must be adjacent */ 
              }
              if (nof_keys == 2) {
                  LOG_DEBUG(BSL_LS_SOC_FP,
                            (BSL_META_U(unit,
                                        "   "
                                        "PMF-Program: %d, cycle: %d, "
                                        "keys:%d and %d "
                                        "allocated for db:%d \n\r"), prog_ndx, cycle, key_id[0], key_id[1], db_id));
              }
              else { /* 1 key */
                  LOG_DEBUG(BSL_LS_SOC_FP,
                            (BSL_META_U(unit,
                                        "   "
                                        "PMF-Program: %d, cycle: %d, "
                                        "key:%d "
                                        "allocated for db:%d \n\r"), prog_ndx, cycle, key_id[0], db_id));
              }
              alloc_info->nof_ce = 0;
              /* For Each qualifier Allocate CE */
              for(ce_index = 0; ce_index  < nof_ce_const; ++ce_index) {

                  /* 
                   * Override for the cascaded qualifiers: try 32b first (highest), then 16b if not found 
                   * Special case for Qual-order: override the key-changed decision 
                   */
                  size_cons = ((ce_cons[ce_index].qual_type == SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED) 
                                && (!(flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER)))?
                      ARAD_PP_FP_KEY_CE_SIZE_32: ce_cons[ce_index].size_cons;

                  LOG_DEBUG(BSL_LS_SOC_FP,
                            (BSL_META_U(unit,
                                        "   "
                                        "PMF-Program: %d, cycle: %d, "
                                        "key:%d "
                                        "try to allocate qual %15s for db:%d size_cons %d\n\r"), prog_ndx, cycle, key_id[0], SOC_PPC_FP_QUAL_TYPE_to_string(ce_cons[ce_index].qual_type),db_id, size_cons));
                  group_cons = ((ce_cons[ce_index].qual_type == SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED) 
                                && (!(flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER)))?
                      ARAD_PP_FP_KEY_CE_HIGH: ce_cons[ce_index].group_cons;

				  /* allocate with minimum possible size */
                  res = arad_pp_fp_key_alloc_ce(
                           unit,
                           stage,
                           prog_ndx,
                           cycle,
                           ce_cons[ce_index].place_cons & alloc_place,/* qual constraint & my constraint */
                           size_cons, /* try qual constraint: 16 or 32 */
                           group_cons, 
                           flags|ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP,
                           &ce_rsrc,
                           &ce_id,
                           &success
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

				  if(success) {
                      arad_pp_fp_key_ce_to_ce_const(unit, stage, ce_id, &alloc_info->act_ce_const[alloc_info->nof_ce]);
                      alloc_info->act_ce_const[alloc_info->nof_ce].lost_bits = ce_cons[alloc_info->nof_ce].lost_bits;
                      alloc_info->act_ce_const[alloc_info->nof_ce].nof_bits = ce_cons[alloc_info->nof_ce].nof_bits;
                      alloc_info->act_ce_const[alloc_info->nof_ce].qual_lsb = ce_cons[alloc_info->nof_ce].qual_lsb;
                      alloc_info->ce[alloc_info->nof_ce] = ce_id;
                      alloc_info->ce_key[alloc_info->nof_ce] = ce_cons[ce_index].is_second_key;
                      alloc_info->qual_type[alloc_info->nof_ce] = ce_cons[ce_index].qual_type;
                      ++alloc_info->nof_ce;
                      LOG_DEBUG(BSL_LS_SOC_FP,
                                (BSL_META_U(unit,
                                            "   "
                                            "program: %d cycle: %d CE-ID:%d allocated for qualifier:%s \n\r"), prog_ndx, cycle, ce_id, SOC_PPC_FP_QUAL_TYPE_to_string(ce_cons[ce_index].qual_type)));
                  }

                  if(!success) {
                      LOG_DEBUG(BSL_LS_SOC_FP,
                                (BSL_META_U(unit,
                                            "   "
                                            "program: %d cycle: %d CE-ID:%d allocation not succeeded for qualifier:%s \n\r"), prog_ndx, cycle, ce_id, SOC_PPC_FP_QUAL_TYPE_to_string(ce_cons[ce_index].qual_type)));
                      break; /* if didn't succeeded, to allocate, then try next cycle,...*/
                  }
              }

              if(success && ce_index == nof_ce_const) {
                  *key_alloced = 1;  /* done */
                  alloc_info->key_id[0] = key_id[0];
                  alloc_info->key_id[1] = key_id[1];
                  alloc_info->cycle = cycle;
                  alloc_info->alloc_place = alloc_place;
                  /* Get the Key size according to the current alloc_place */
                  alloc_info->key_size = Arad_pp_fp_place_const_convert[alloc_place_ndx][2];

                  /* allocate key in program */
                  res = arad_pp_fp_key_alloc_key_id(
                           unit,
                           stage,
                           prog_ndx,
                           db_id,
                           cycle,
                           alloc_place, /* Number of half keys taken: 1, 2, 3, 4 */
                           flags, /* Commit because success */
                           &key_id[0],
                           &success
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

                  goto exit;
              }
              else{
                  LOG_DEBUG(BSL_LS_SOC_FP,
                            (BSL_META_U(unit,
                                        "   "
                                        "program: %d cycle: %d allocation not succeeded %d, nof_ce_const %d \n\r"), prog_ndx, cycle, success, nof_ce_const));
              }
          } /* for cycle */
          /* if fail to allocate in all cycle with given sizes/places then done with fail */
          if(flags & ARAD_PP_FP_KEY_ALLOC_USE_CE_CONS) {
             LOG_ERROR(BSL_LS_SOC_FP,
                       (BSL_META_U(unit,
                                   "    "
                                   "Key: fail to allocate in all cycle with given sizes/places  \n\r")));
              goto exit;
          }

      } /* exact alloc */
  } /* alloc place */

  
exit:
  /* success and need to update HW - 1st round do write */
  if(success) {

      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "   "
                            "commit allocation to hw, nof CE used:%d , prog_ndx %d, flags %d\n\r"), alloc_info->nof_ce, prog_ndx, flags));

      res = arad_pp_fp_key_alloc_in_prog_commit(
                unit,
                stage,
                prog_ndx,
                db_id,
                flags,
                qual_types,
                alloc_info
             );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit2);
  }

  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "   "
                        "allocation succeeded:%d  \n\r"), *key_alloced));

exit2:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_in_prog()", alloc_place, cycle);
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
  arad_pp_fp_elk_key_alloc_in_prog(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
    SOC_SAND_IN  uint32                       prog_ndx,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX], /* qualifiers*/
    SOC_SAND_IN ARAD_PP_FP_CE_CONSTRAINT      ce_cons[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS], /* CE constraints*/
    SOC_SAND_IN  uint32                       nof_ce_const,
   /* out */
    SOC_SAND_OUT  ARAD_PP_FP_KEY_ALLOC_INFO   *alloc_info,
    SOC_SAND_OUT  uint8                       *key_alloced
  )
{
  uint32
      cycle,
      ce_rsrc,
      ce_index,
      size_cons,
      alloc_ndx,
      res = SOC_SAND_OK;
  uint8
      success = FALSE;
  ARAD_PP_KEY_CE_ID
      ce_id_highest = 0,
      ce_id;

  SOC_PPC_FP_DATABASE_INFO
    fp_database_info; 
     
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  cycle = 0,

  *key_alloced = 0;
  ARAD_PP_FP_KEY_ALLOC_INFO_clear(alloc_info);

  /* Get the DB info */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
          unit,
          SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, /* All stages equivalent in use */
          db_id,
          &fp_database_info
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  /* get CE resource status */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(
            unit,
            stage,
            prog_ndx,
            cycle,
            &ce_rsrc
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  /* For Each qualifier Allocate CE */
  for(ce_index = 0; ce_index  < nof_ce_const; ++ce_index) 
  {
      /* There are two attempts for instruction allocation:
       * if first fails - try again for 32-bit instruction
       */
      for (alloc_ndx = 0; alloc_ndx < 2; alloc_ndx++) 
      {
          size_cons = (alloc_ndx == 0) ? ce_cons[ce_index].size_cons : ARAD_PP_FP_KEY_CE_SIZE_32;

          /* allocate with minimum possible size */
          res = arad_pp_fp_key_alloc_ce(
                   unit,
                   stage,
                   prog_ndx,
                   cycle,
                   ce_cons[ce_index].place_cons,/* qual constraint & my constraint */
                   size_cons, /* try qual constraint: 16 or 32 */
                   ce_cons[ce_index].group_cons, 
                   flags|ARAD_PP_FP_KEY_ALLOC_CE_USE_RSRC_BMP,
                   &ce_rsrc,
                   &ce_id,
                   &success
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

          /* Break upon success */
          if(success) {
              break;
          }
      }

      if(success) {
          arad_pp_fp_key_ce_to_ce_const(unit, stage, ce_id, &alloc_info->act_ce_const[alloc_info->nof_ce]);
          alloc_info->act_ce_const[alloc_info->nof_ce].lost_bits = ce_cons[alloc_info->nof_ce].lost_bits;
          alloc_info->act_ce_const[alloc_info->nof_ce].nof_bits = ce_cons[alloc_info->nof_ce].nof_bits;
          alloc_info->act_ce_const[alloc_info->nof_ce].qual_lsb = ce_cons[alloc_info->nof_ce].qual_lsb;
          alloc_info->ce[alloc_info->nof_ce] = ce_id;
          alloc_info->ce_key[alloc_info->nof_ce] = ce_cons[ce_index].is_second_key;
          alloc_info->qual_type[alloc_info->nof_ce] = ce_cons[ce_index].qual_type;
          ++alloc_info->nof_ce;
          ce_id_highest = SOC_SAND_MAX(ce_id_highest, ce_id);

          LOG_DEBUG(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "   "
                                "Stage: FLP, program: %d cycle: %d CE-ID:%d alloced for qualifier:%s \n\r"), 
                     prog_ndx, cycle, ce_id, SOC_PPC_FP_QUAL_TYPE_to_string(ce_cons[ce_index].qual_type)));
      }
  }

  /* All CEs have been allocated with instructions */
  if(success && ce_index == nof_ce_const) {
      *key_alloced = 1;  /* done */
      alloc_info->key_id[0] = 2; /* Key C */
      alloc_info->cycle = cycle;
      alloc_info->alloc_place = ARAD_PP_FP_KEY_CE_LOW;
      alloc_info->key_size = ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS;
  }
  else{
      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "   "
                            "program: %d cycle: %d allocation not succeeded %d, nof_ce_const %d \n\r"), prog_ndx, cycle, success, nof_ce_const));
  }
  
  /* 
   * In multiple ELK ACLs, do not allow to intermingle ACL instructions: 
   * once the instructions of an ACL are set, all the instructions higher to the lowest instructions 
   * are set to be taken. Thus, the next ACL will have only lower instructions and will be in the 
   * Key-C MSB 
   */
  if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) && (!(flags & ARAD_PP_FP_KEY_ALLOC_CE_CHECK_ONLY))) {
      SHR_BITSET_RANGE(&ce_rsrc, 0, ce_id_highest); /* At least one Copy-Engine */

      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.set(unit, stage, prog_ndx, cycle, ce_rsrc);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 55, exit);
  }


exit:
  /* success and need to update HW - 1st round do write */
  if(success) {

      LOG_DEBUG(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "   "
                            "commit allocation to hw, nof CE used:%d \n\r"), alloc_info->nof_ce));

      res = arad_pp_fp_key_alloc_in_prog_commit(
                unit,
                stage,
                prog_ndx,
                db_id,
                flags,
                qual_types,
                alloc_info
             );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit2);
  }

  LOG_DEBUG(BSL_LS_SOC_FP,
            (BSL_META_U(unit,
                        "   "
                        "allocation succeeded:%d  \n\r"), *key_alloced));

exit2:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_elk_key_alloc_in_prog()", 0, cycle);
}
#endif


/*
 * return better bitmap (which set on less bits in cur_rsrs
 */

STATIC
 uint32
  arad_pp_fp_key_ce_rsrc_select(
     SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
      SOC_SAND_IN  uint32                       rsrc1,
      SOC_SAND_IN  uint32                       rsrc2
    )
{
    uint32
        new_rsrc_bmp[1];
    int
        nof_set_bits1,
        nof_set_bits2;

    *new_rsrc_bmp = rsrc1;
    shr_bitop_range_count(new_rsrc_bmp,0,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG,&nof_set_bits1);


    *new_rsrc_bmp = rsrc2;
    shr_bitop_range_count(new_rsrc_bmp,0,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG,&nof_set_bits2);

    if(nof_set_bits1 <= nof_set_bits2) {
        return rsrc1;
    }

    return rsrc2;
}

/*
 * Given a PMF-Program, is there a cycle with no possible 320b 
 * keys. 
 * In practice, only for Ingress-PMF (single stage with 2 cycles) 
 */
uint32
  arad_pp_fp_key_cycle_no_320b_key_get(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE    stage,
      SOC_SAND_IN  uint32                    cur_prog,
      SOC_SAND_OUT uint8                    *no_320b_cycle_found, 
      SOC_SAND_OUT uint32                   *selected_cycle
  )
{
    uint32 
        key_ndx,
        keys_bmp, /* [Key-D-MSB][Key-D-LSB]..[Key-A-MSB][Key-A-LSB] */
        cycle_ndx,
        fld_val,
        res;
    uint8
        is_key_found_for_320b[SOC_PPC_FP_NOF_CYCLES];

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  for(cycle_ndx = 0; cycle_ndx < SOC_PPC_FP_NOF_CYCLES; cycle_ndx++) {
      /* 
       * Get the key resource bitmap: in the first run, 
       * the Key-A is not disabled yet but no influence since there 
       *  is always possibly 320b keys
       */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(
                unit,
                stage,
                cur_prog,
                cycle_ndx,
                &keys_bmp
              );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

      /* 
       * See if there is a 320b in this cycle
       */
      is_key_found_for_320b[cycle_ndx] = FALSE;
      for (key_ndx = 0; (key_ndx < (ARAD_PMF_LOW_LEVEL_NOF_KEYS / 2 /* 2 keys for 320b */)) && (!is_key_found_for_320b[cycle_ndx]); key_ndx++) {
          fld_val = 0;
          /* Extract the 4 bits */
          SHR_BITCOPY_RANGE(&fld_val, 0, &keys_bmp, (ARAD_PP_FP_KEY_NUMBER_80_PARTS_IN_320B * key_ndx), ARAD_PP_FP_KEY_NUMBER_80_PARTS_IN_320B);
          if(fld_val == 0 /* No 80b part alreay allocated */) {
              is_key_found_for_320b[cycle_ndx] = TRUE;
          }
      }
  }

  *no_320b_cycle_found = FALSE; 
  *selected_cycle = 0;
  /* 
   * Summarize the results: select cycle only if one of the two 
   * cycles has no posible 320b key 
   */
  if(is_key_found_for_320b[0] && (!is_key_found_for_320b[1])) {
      *no_320b_cycle_found = TRUE; 
      *selected_cycle = 1;
  }
  else if(is_key_found_for_320b[1] && (!is_key_found_for_320b[0])) {
      *no_320b_cycle_found = TRUE; 
      *selected_cycle = 0;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_cycle_no_320b_key_get()", cur_prog, 0);
}

/*
 *  given list of program return bmp of CE resources
 *  cycles 0,1, ANY: which cycle to use of each program 
 */
STATIC
 uint32
  arad_pp_fp_key_progs_rsrc_stat(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  uint32                       db_id, 
    SOC_SAND_IN  uint32                       progs_bmp,
    SOC_SAND_IN  uint32                       cycles,
    SOC_SAND_IN  uint8                        is_for_direct_extraction,
    SOC_SAND_OUT  uint8                       cycle_needed[SOC_PPC_FP_NOF_CYCLES],
    SOC_SAND_OUT  uint32                      ce_rsrc[SOC_PPC_FP_NOF_CYCLES],
    SOC_SAND_OUT  uint32                      selected_cycle[ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES],
    SOC_SAND_OUT  uint8                       key_zone_enough[ARAD_PP_FP_NOF_KEY_CE_PLACES][SOC_PPC_FP_NOF_CYCLES],
    SOC_SAND_OUT  uint32                      *flp_max_key_size_in_bits,
    SOC_SAND_OUT  uint32                      *dir_ext_start_place
  )
{
  uint32   
      key_id[2],
      place_ndx,
      cur_prog,
      prog_result,
      cur_rsrc1,
      cur_rsrc2,
      cur_rsrc1_normalized,
      cur_rsrc2_normalized=0,
      max_rsrc1_used_ces=0,
      max_rsrc2_used_ces=0,
      sel_rsrc,
      cycle_ndx,
      ce_indx,
      bmp_index,
      nof_cycles=0,
      cyc_1 = 0,
      selected_cycle_320b,
      progs_bmp_lcl[1],
      keys_bmp,/* [Key-D-MSB][Key-D-LSB]..[Key-A-MSB][Key-A-LSB] */
      res = SOC_SAND_OK,
      key_masks[2] = {0x55,0xaa};
  uint8
      is_high_and_low_diff_fgs_allowed,
      bit_to_set = FALSE,
      no_320b_cycle_found,
      key_index = 0,
      success = FALSE;
  int
      cur_rsrc1_count,
      cur_rsrc2_count;

      
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  if (((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)&&(SOC_IS_JERICHO(unit)))) {
      flp_max_key_size_in_bits[0] = ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS; /* Used only in FLP stage */
      flp_max_key_size_in_bits[1] = ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS; /* Used only in FLP stage */
  }else{
      *flp_max_key_size_in_bits = ARAD_PP_FLP_KEY_C_ZONE_SIZE_ARAD_PLUS_BITS; /* Used only in FLP stage */
  }
  /* init the structure */
  sal_memset(key_zone_enough, 0x0, sizeof(uint8) * ARAD_PP_FP_NOF_KEY_CE_PLACES * SOC_PPC_FP_NOF_CYCLES);
  
  for(cycle_ndx = 0; cycle_ndx < SOC_PPC_FP_NOF_CYCLES; cycle_ndx++) {
      for(place_ndx = 0; place_ndx < ARAD_PP_FP_NOF_KEY_CE_PLACES_STAGE; place_ndx++) {
          /* By default always a place */
          key_zone_enough[place_ndx][cycle_ndx] = TRUE;
      }
  }

  ce_rsrc[0] = 0;
  ce_rsrc[1] = 0;
  cycle_needed[0] = FALSE;
  cycle_needed[1] = FALSE;

  if(cycles == ARAD_PP_FP_KEY_CYCLE_ANY) {
      nof_cycles = 2;
      cyc_1 = 0;
  }
  else if(cycles == ARAD_PP_FP_KEY_CYCLE_EVEN) {
      nof_cycles = 1;
      cyc_1 = 0;
  }
  else if(cycles == ARAD_PP_FP_KEY_CYCLE_ODD) {
      nof_cycles = 1;
      cyc_1 = 1;
  }

  cur_prog = 0;
  *progs_bmp_lcl = progs_bmp;
  ARAD_PP_FP_KEY_FIRST_SET_BIT(progs_bmp_lcl,cur_prog,ARAD_PMF_LOW_LEVEL_NOF_PROGS-cur_prog,ARAD_PMF_LOW_LEVEL_NOF_PROGS,FALSE,prog_result);
  if(prog_result == 0) {
     LOG_ERROR(BSL_LS_SOC_FP,
               (BSL_META_U(unit,
                           "Unit %d Stage %s No programss available.\n\r"),
                unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 5, exit); /* unexpected !*/
  }
  while(prog_result != 0) {
      selected_cycle[cur_prog] = (cycles == ARAD_PP_FP_KEY_CYCLE_ODD) ? 1 : 0;

      /* get CE resource status for 1st cycle */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(
              unit,
              stage,
              cur_prog,
              cyc_1,
              &cur_rsrc1
          );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    /* 
     * When multiple programs are allowed, the CE repartition 
     * should be normalized to know number of copy engines free in each group 
     * Besides, when allocating the CE, a free CE in this group should be found 
     */
    cur_rsrc1_normalized = arad_pp_fp_key_ce_rsrc_normalize(unit, stage, flags, cur_rsrc1); 
    sel_rsrc = cur_rsrc1_normalized;

    /* get for second cycle */
    if(nof_cycles == 2) {
        /* 
         * In case of two possible cycles, select the cycle according to: 
         * 1. the cycle with zero 320b possible keys. This way, a 160b database 
         * will not take the place of a possible 320b in the other cycle 
         * 2. if there is not, with maximum free copy engines. 
         * Since the loop is on all programs, a decision taken by a program 
         * can be reverted by a future program 
         */
        res = arad_pp_fp_key_cycle_no_320b_key_get(
                unit,
                stage,
                cur_prog,
                &no_320b_cycle_found, 
                &selected_cycle_320b
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

        /* Compute the normalized CE resources */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(
                unit,
                stage,
                cur_prog,
                1 - cyc_1,
                &cur_rsrc2
            );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

        cur_rsrc2_normalized = arad_pp_fp_key_ce_rsrc_normalize(unit, stage, flags, cur_rsrc2); 

        /* Sum the number of used CEs for each cycle and retrieve the maximum value for all programs. */
        shr_bitop_range_count(&cur_rsrc1_normalized,0,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG,&cur_rsrc1_count);
        shr_bitop_range_count(&cur_rsrc2_normalized,0,ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG,&cur_rsrc2_count);
        if (max_rsrc1_used_ces < cur_rsrc1_count) {
            max_rsrc1_used_ces = cur_rsrc1_count;
        }
        if (max_rsrc2_used_ces < cur_rsrc2_count) {
            max_rsrc2_used_ces = cur_rsrc2_count;
        }

       /* Select either according to number of 320b keyss left or number of free instructions.
          Perform this optimization only if SOC property custom_feature_pmf_320b_key_opt_disable is no set.
          This SOC property should be used when running field_scene regression. */
        if((!soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "pmf_320b_key_opt_disable", 0))
            && (no_320b_cycle_found)) {
            selected_cycle[cur_prog] = selected_cycle_320b;
            sel_rsrc = (selected_cycle[cur_prog] == 0)? cur_rsrc1_normalized: cur_rsrc2_normalized;
        }
        else {
            sel_rsrc = arad_pp_fp_key_ce_rsrc_select(unit, stage, cur_rsrc1_normalized, cur_rsrc2_normalized);
            selected_cycle[cur_prog] = (sel_rsrc == cur_rsrc1_normalized)?0:1;

            /* If KEY_A (SINGLE_BANK) then try to allocate the cycle 0 first. Only in case of failure to allocate - try to allocate from cycle 1.*/
            if (SOC_IS_ARADPLUS_AND_BELOW(unit))
            {
                if (flags & ARAD_PP_FP_KEY_ALLOC_USE_KEY_A)
                {
                    selected_cycle[cur_prog] = 0;       
                    sel_rsrc = cur_rsrc1_normalized;
                }
            }

        }
    }

    cycle_needed[selected_cycle[cur_prog]] = TRUE;
    ce_rsrc[selected_cycle[cur_prog]] |= sel_rsrc;
    if(nof_cycles == 2) {
        ce_rsrc[1 - selected_cycle[cur_prog]] |= (sel_rsrc == cur_rsrc1_normalized)? cur_rsrc2_normalized: cur_rsrc1_normalized;
        cycle_needed[selected_cycle[cur_prog]] = TRUE;
        cycle_needed[1 - selected_cycle[cur_prog]] = FALSE;
    }
    
    /* 
     * Compute for each key configuration (80b LSB, 80b MSB, 160b and 320b) 
     * if there is key for this 
     */
    is_high_and_low_diff_fgs_allowed = is_for_direct_extraction || (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB);
    for(cycle_ndx = 0; cycle_ndx < SOC_PPC_FP_NOF_CYCLES; cycle_ndx++) {
        for(place_ndx = 0; place_ndx < ARAD_PP_FP_NOF_KEY_CE_PLACES_STAGE; place_ndx++) {
            /* See if relevant: always found until now */
            if (!key_zone_enough[place_ndx][cycle_ndx])
            {
                continue;
            }
            /* Set to True only if success */
            key_zone_enough[place_ndx][cycle_ndx] = FALSE;

            /* See if relevant: correct place index */
            if ((((!is_high_and_low_diff_fgs_allowed) && Arad_pp_fp_place_const_convert[place_ndx][4]) 
                 || (is_high_and_low_diff_fgs_allowed && Arad_pp_fp_place_const_convert[place_ndx][5])))
            {
                if(stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)
                {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
                    uint32
                        max_key_size_in_bits;

                    /* For FLP Databases, only Key C in FLP can be used for ACLs,
                     * and Places are not relevant
                     */
					if (SOC_IS_JERICHO(unit)) {
                        uint32 i;
						for (i=0;i<ARAD_PP_FLP_KEY_C_NUM_OF_ZONES_JERICHO;i++) {
                            res = arad_pp_flp_elk_prog_config_max_key_size_get(unit, cur_prog, i, &max_key_size_in_bits);
                            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 192, exit);
                            flp_max_key_size_in_bits[i] = SOC_SAND_MIN(flp_max_key_size_in_bits[i], max_key_size_in_bits);
						}
					}else{
                        res = arad_pp_flp_elk_prog_config_max_key_size_get(unit, cur_prog, 0, &max_key_size_in_bits);
                        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 192, exit);
                        *flp_max_key_size_in_bits = SOC_SAND_MIN(*flp_max_key_size_in_bits, max_key_size_in_bits);
                    }
                    success = TRUE;
#else
                    success = FALSE;
#endif
                }
                else
                {
                    /* Go over the Key until such a case is found */
                    /* Simulate allocation of a key in program */
                    res = arad_pp_fp_key_alloc_key_id(
                             unit,
                             stage,
                             cur_prog,
                             db_id,
                             cycle_ndx,
                             Arad_pp_fp_place_const_convert[place_ndx][0], /* Number of half keys taken: 1, 2, 3, 4 */
                             ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY | (flags & ARAD_PP_FP_KEY_ALLOC_USE_KEY_A ), /* Check-only */
                             &key_id[0],
                             &success
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
                }

                if (success) {
                    key_zone_enough[place_ndx][cycle_ndx] = TRUE;
                    continue; /* Go to next place */
                }
            }
        }

        /* Get available LSB/MSB Keys */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(
                unit,
                stage,
                cur_prog,
                cycle_ndx,
                &keys_bmp 
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

        /* Choose the correct start place according to the key bitmap */
        if (is_for_direct_extraction && soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "optimized_de_allocation", 0)) 
        {
            for (key_index = 0  ; key_index < ARAD_PMF_LOW_LEVEL_NOF_KEYS ; key_index ++) 
            {
                if ( ((keys_bmp >> (key_index * 2)) == 0x1) )  
                {
                    *dir_ext_start_place = ARAD_PP_FP_KEY_CE_HIGH;
                    break;
                }
                else if ( ((keys_bmp >> (key_index * 2)) == 0x2) ) 
                {
                    *dir_ext_start_place = ARAD_PP_FP_KEY_CE_LOW;
                    break;
                }
            }
        }

        for (bmp_index = 0 ; bmp_index < 2 ; bmp_index ++) { /*for LSBs (bmp_index = 0) / MSBs (bmp_index = 1) */

            /*check that  all LSB/MSB bits are set - if yes: disregard CE instruction occupancy for LSB/MSB */
            if ( (keys_bmp & key_masks[bmp_index]) == key_masks[bmp_index] ) 
            {
                  for(ce_indx = 0; ce_indx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG; ++ce_indx) {
                     bit_to_set = (bmp_index == 0 ) ? (ce_indx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB) : ( ce_indx >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB );
                     if (bit_to_set)
                     {
                         SHR_BITSET(&ce_rsrc[cycle_ndx], ce_indx);
                     }
                  }/*ce index iteration*/
             }
        }/*bitmap iteration*/
    }

    ARAD_PP_FP_KEY_FIRST_SET_BIT(progs_bmp_lcl,cur_prog,ARAD_PMF_LOW_LEVEL_NOF_PROGS-cur_prog,ARAD_PMF_LOW_LEVEL_NOF_PROGS,TRUE,prog_result);
  }
  if (nof_cycles == 2) {
      /* Choose the cycle with the most free resources.*/
      if (max_rsrc1_used_ces <= max_rsrc2_used_ces) {
          cycle_needed[0] = TRUE;
          cycle_needed[1] = FALSE;
      } else {
          cycle_needed[0] = FALSE;
          cycle_needed[1] = TRUE;
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_progs_rsrc_stat()", 0, 0);
}

STATIC
uint32
  arad_pp_fp_qual_ce_mltpl_info_get(
      SOC_SAND_IN  int                       unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
	  SOC_SAND_IN  uint32                       flags,
      SOC_SAND_INOUT SOC_PPC_FP_QUAL_TYPE       qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
      SOC_SAND_OUT uint32                       *nof_quals,
      SOC_SAND_OUT ARAD_PP_FP_QUAL_CE_INFO      qual_ce_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX]
  )
{
  uint8 
      success;
  uint32 
      res, 
      qual_indx;

  ARAD_PP_FP_ALLOC_ALG_ARGS
      algorithm_args;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *nof_quals = 0;
  algorithm_args.balance_enabled = 0;
  algorithm_args.balance_lsb_msb = 0;

  for(qual_indx = 0; (qual_indx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX) && (qual_types[qual_indx]!= SOC_PPC_NOF_FP_QUAL_TYPES) && (qual_types[qual_indx]!= BCM_FIELD_ENTRY_INVALID); ++qual_indx)
  {
    /*
        res = arad_pp_fp_qual_ce_info_get(
              unit,
              stage,
              qual_types[qual_indx],
              &(qual_ce_info[qual_indx])
          );
    */
    res = arad_pp_fp_qual_ce_info_uneven_get(
            unit,
            stage,
            FALSE, /* break_uneven */
            0,
            qual_types[qual_indx],
            &algorithm_args,
            NULL, /* total_bits_in_zone[] - not to be used */
            NULL, /* nof_free_ces_in_zone[] - not to be used */
            NULL, /* zone_used[] - not to be used */
            &(qual_ce_info[(*nof_quals)]),
            &success
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10 + qual_indx, exit);

    (*nof_quals)++;
  }

  /*
   * For FLP, insert an empty qualifier at the end of the list 
   * to allow the byte round-up per ACL: 
   * - each FLP ELK ACL must have a key size byte-aligned 
   * - the different FLP ACLs will not be superposed 
   */
  if (flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) {
      res = arad_pp_fp_qual_ce_info_uneven_get(
              unit,
              stage,
              FALSE, /* break_uneven */
              0,
              SOC_PPC_FP_QUAL_FWD_PRCESSING_PROFILE, /* Zero qualifier */
              &algorithm_args,
              NULL, /* total_bits_in_zone[] - not to be used */
              NULL, /* nof_free_ces_in_zone[] - not to be used */
              NULL, /* zone_used[] - not to be used */
              &(qual_ce_info[(*nof_quals)]),
              &success
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      /* No bits to add for the moment */
      qual_ce_info[(*nof_quals)].qual_size = 0;
      qual_ce_info[(*nof_quals)].ce_nof_bits[0] = 0;
      qual_types[(*nof_quals)] = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;

      (*nof_quals)++;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_ce_mltpl_info_get()", 0, 0);
}

/* 
 * Sort the qualifiers according to their size for better utilization of 
 * the key sizes: 
 * 1. The Key-Changed qualifier must be at LSB 
 * 2. The SLB hash key must be put in the MSB of the first key.
 * 3. Sort first the LSB-only and MSB-only qualifiers, since their location is known 
 * 4. Then sort per qualifier size (regardless the lost bits) 
 *  
 * Special case for FLP: 
 * - all the qualifiers are both LSB and MSB 
 * - sort the User-Defined fields to be the first to give possibility of control to 
 * the user on the instructions order 
 */
STATIC
uint32 
	arad_pp_fp_qual_ce_info_sort(
	SOC_SAND_IN    uint32                   flags,
    SOC_SAND_IN    uint32                   nof_quals,
    SOC_SAND_INOUT SOC_PPC_FP_QUAL_TYPE     qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
	SOC_SAND_INOUT ARAD_PP_FP_QUAL_CE_INFO  qual_ce_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX]
	)
{
  uint32 
      qual_idx,
      swap_idx,
      next_qual,
      qual_nof_bits,
      qual_nof_bits_next,
      swap_priority,
      no_swap_priority;
  uint8 
    same_category,
	  swap_needed = FALSE,
    swaped = FALSE;

  ARAD_PP_FP_QUAL_CE_INFO ce_info_tmp;
  SOC_PPC_FP_QUAL_TYPE qual_tmp;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

  for(swap_idx = 0; swap_idx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; swap_idx++) {
    swaped = FALSE;  
    for (qual_idx = 0; qual_idx < nof_quals-1; qual_idx++) {
        swap_priority = 0;
        no_swap_priority = 0;
        if ((!qual_ce_info[qual_idx].is_lsb) && (!qual_ce_info[qual_idx].is_msb)) {
           LOG_ERROR(BSL_LS_SOC_FP,
                     (BSL_META_U(unit,
                                 "Qualifier %s : Qualifier is not allowed in neither lsb nor msb.\n\r"),
                      SOC_PPC_FP_QUAL_TYPE_to_string(qual_types[qual_idx])));
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 10, exit);
        }

        next_qual = qual_idx + 1;

        qual_nof_bits = qual_ce_info[qual_idx].lost_bits + qual_ce_info[qual_idx].qual_size;
        qual_nof_bits_next = qual_ce_info[next_qual].lost_bits + qual_ce_info[next_qual].qual_size;
        same_category = (qual_ce_info[qual_idx].is_lsb == qual_ce_info[next_qual].is_lsb) && 
            (qual_ce_info[qual_idx].is_msb == qual_ce_info[next_qual].is_msb) ? TRUE : FALSE;
        swap_needed = FALSE;

        /* check conditions to swap */
        if (SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED == qual_types[next_qual]) {
            swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FIRST;
        } else if (SOC_PPC_FP_QUAL_KEY_AFTER_HASHING == qual_types[next_qual]) {
            swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FIRST;
        }
        /* Swap in FLP if current qual is not UDF and next one is UDF */
        else if ((flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) 
                 && SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(qual_types[next_qual])
                 && (!SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(qual_types[qual_idx]))) {
            swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_SECOND;
        }
        else if (!qual_ce_info[qual_idx].is_lsb && !qual_ce_info[next_qual].is_msb) {
            swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_THIRD;
        }
        else if (qual_ce_info[qual_idx].is_lsb && qual_ce_info[qual_idx].is_msb && (!qual_ce_info[next_qual].is_lsb || !qual_ce_info[next_qual].is_msb)) {
            swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FOURTH;
        }
        else if ((qual_nof_bits < qual_nof_bits_next) && same_category) {
            swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FIFTH;
        }

        /* check conditions NOT to swap */
        if (SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED == qual_types[qual_idx]) {
            no_swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FIRST;
        } 
        else if (SOC_PPC_FP_QUAL_KEY_AFTER_HASHING == qual_types[next_qual]) {
            no_swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FIRST;
        }
        else if ((flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) 
                 && SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(qual_types[qual_idx])) {
            no_swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_SECOND;
        }
        else if (!qual_ce_info[qual_idx].is_msb) {
            no_swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_THIRD;
        }      
        else if ((!qual_ce_info[qual_idx].is_lsb) || (!qual_ce_info[qual_idx].is_msb)) {
            no_swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FOURTH;
        }
        else if ((qual_nof_bits >= qual_nof_bits_next) && same_category) {
            no_swap_priority = ARAD_PP_FP_KEY_SWAP_PRIORITY_FIFTH;
        }

        /* Decide whether to swap */
        if (swap_priority > no_swap_priority) {
            swap_needed = TRUE;
        }

        if (swap_needed) {
            /* Swap */
            sal_memcpy(&ce_info_tmp, &(qual_ce_info[qual_idx]), sizeof(ARAD_PP_FP_QUAL_CE_INFO));
            qual_tmp = qual_types[qual_idx];

            sal_memcpy(&(qual_ce_info[qual_idx]), &(qual_ce_info[next_qual]), sizeof(ARAD_PP_FP_QUAL_CE_INFO));
            qual_types[qual_idx] = qual_types[next_qual];

            sal_memcpy(&(qual_ce_info[next_qual]), &ce_info_tmp, sizeof(ARAD_PP_FP_QUAL_CE_INFO));
            qual_types[next_qual] = qual_tmp;

            /*
            ce_info_tmp = qual_ce_info[qual_idx];
            qual_ce_info[qual_idx] = qual_ce_info[next_qual];
            qual_ce_info[next_qual] = ce_info_tmp; 
            */ 
            swaped = TRUE;
        }
      } /* for (qual_idx = 0; qual_idx < nof_quals-1; qual_idx++)  */
      if (swaped == FALSE) {
          break;
      }
    } 

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_ce_info_sort()", 0, 0);
}

STATIC
uint32 
    arad_pp_fp_qual_sum(
    SOC_SAND_IN    ARAD_PP_FP_QUAL_CE_INFO  qual_ce_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
    SOC_SAND_IN    uint32                   nof_quals
    )
{
    uint32 qual_idx, qual_sum = 0;
  
    for(qual_idx = 0; qual_idx < nof_quals; qual_idx++) 
    {
        qual_sum += qual_ce_info[qual_idx].lost_bits + qual_ce_info[qual_idx].qual_size;
    }

    return qual_sum;
}


/* 
 * Compute the number of free Copy Engines per stage, LSB/MSB, 16/32
 */
uint32
  arad_pp_fp_key_nof_free_instr_get(
      SOC_SAND_IN  int                            unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
      SOC_SAND_IN  uint32                       ce_rsrc_bmp_glbl,
      SOC_SAND_IN  uint8                        is_32b,
      SOC_SAND_IN  uint8                        is_msb,
      SOC_SAND_IN  uint8                        is_high_group
    )
{
  uint32 
      nof_free_instr,
      ce_indx,
      ce_rsrc_bmp_glbl_lcl[1];
  uint8
      high_group,
      ce_32b,
      ce_msb;

  *ce_rsrc_bmp_glbl_lcl = ce_rsrc_bmp_glbl;
  nof_free_instr = 0;
  for(ce_indx = 0; ce_indx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG; ++ce_indx) {
      ce_32b = arad_pmf_low_level_ce_is_32b_ce(unit, stage, ce_indx);
      ce_msb = (ce_indx >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB);
      high_group = arad_pmf_low_level_ce_is_second_group(unit, stage, (ce_indx % ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB));


      /* Verify first it is in the correct group */
      if ((is_32b == ce_32b) && (is_msb == ce_msb) && (high_group == is_high_group)) {
          /* Verify second it is not used */
          if (!(SHR_BITGET(ce_rsrc_bmp_glbl_lcl, ce_indx))) {
              nof_free_instr ++;
          }
      }
  }

  return nof_free_instr;
}

/* get constrain of CE allocation
 * check if DB already defined over some program 
 * - then get constraints from exist allocation
 * - otherwise get constraints from qualifiers list 
 */
uint32
  arad_pp_fp_key_alloc_constrain_calc(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  uint32                       flags,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX], /* qualifiers*/
    SOC_SAND_IN  uint32                       cycle_cons, /* 0, 1, any*/
    SOC_SAND_IN  uint32                       place_cons,/* low, any*/
    SOC_SAND_IN  uint8                        is_for_direct_extraction,
    SOC_SAND_OUT ARAD_PP_FP_CE_CONSTRAINT     ce_const[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS],
    SOC_SAND_OUT  uint32                      *nof_consts,
    SOC_SAND_INOUT  uint32                      *place_start,/* low, any*/
    SOC_SAND_OUT  uint32                      selected_cycle[ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES],
    SOC_SAND_OUT  uint32                      total_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
    SOC_SAND_OUT  uint8                       *found
  )
{
  uint32   
    res = SOC_SAND_OK,
    place_ndx,
    zone_idx,
    zone_id = 0,
      zone_id_first =0 ,
      zone_id_second = 0,
    direct_ext_loop_indx, 
    qual_indx,
    nof_quals,
    qual_sum,
    exist_progs,
    exist_progs_array[1],
    nof_cycles,
    cycle_idx,
    cycle_id,
    cycle_base,
    new_cycle_cons,
    ce_indx,
    total_ce_indx = 0,
    prog_ndx,
      flp_max_key_size_in_bits[ARAD_PP_FP_KEY_NOF_ZONES]={0},
    place_start_lcl, tmp_start_place,
    dir_ext_start_place = 0,
    ce_rsrc_bmp_glbl_lcl[SOC_PPC_FP_NOF_CYCLES], 
    nof_free_ces_in_zone[ARAD_PP_FP_KEY_NOF_CATEGORIES]; /* Not same interpretation of zone */
  uint8
      success,
      is_32b,
      is_msb,
      is_high_group,
      is_key_not_found,
      is_slb_hash_in_quals,
      key_zone_enough[ARAD_PP_FP_NOF_KEY_CE_PLACES][SOC_PPC_FP_NOF_CYCLES],
      direct_ext_2_needed, 
      cycle_needed[SOC_PPC_FP_NOF_CYCLES], 
      lookup_id,
      is_cascaded,
      zone_used[ARAD_PP_FP_KEY_NOF_ZONES];
  SOC_PPC_FP_QUAL_TYPE          
      new_qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  ARAD_PP_FP_QUAL_CE_INFO       
      *ce_info = NULL;
  soc_error_t 
    soc_res;
  ARAD_PP_FP_ALLOC_ALG_ARGS
      algorithm_args;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  sal_memcpy(new_qual_types, qual_types, sizeof(SOC_PPC_FP_QUAL_TYPE) * SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
  ARAD_ALLOC(ce_info, ARAD_PP_FP_QUAL_CE_INFO, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX, "arad_pp_fp_key_alloc_constrain_calc.ce_info");
  sal_memset(ce_info, 0x0, sizeof(ARAD_PP_FP_QUAL_CE_INFO) * SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

  /* get relevant programs for the DB */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(
          unit,
          stage,
          db_id,
          0,
          &exist_progs
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  if (  (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)) {
      uint32 dip_sip_sharing_enabled = 0, prog_result = 0;
	  int i,j;
  	  int actual_nof_quals = 0;
  	  int nof_shared_quals = 0;
  	  int is_exists = 0;
      int cluster_id = -1;

      arad_pp_fp_dip_sip_sharing_is_sharing_enabled_for_db(unit, stage, db_id, &dip_sip_sharing_enabled, &cluster_id);
      
	  exist_progs_array[0] = exist_progs;      
      ARAD_PP_FP_KEY_FIRST_SET_BIT(exist_progs_array, prog_result, ARAD_PMF_LOW_LEVEL_NOF_PROGS, ARAD_PMF_LOW_LEVEL_NOF_PROGS, FALSE, res);

      if (dip_sip_sharing_enabled) {
          for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
              if((qual_types[i] == SOC_PPC_NOF_FP_QUAL_TYPES) || (qual_types[i] == BCM_FIELD_ENTRY_INVALID)){
                  break;
              }
              actual_nof_quals++;

              dip_sip_sharing_qual_info_get(unit, prog_result, new_qual_types[i], &is_exists, NULL);
              if (is_exists) {              
                  nof_shared_quals++;
                  for (j = i; j < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX - nof_shared_quals; j++) {
                      if (j < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX) {/* this is done for coverity */
                          new_qual_types[j] = qual_types[j+nof_shared_quals];
                      }else{
                          assert(0);
                      }
                  }
                  i--;
              }
          }
      }
  }

  /* get status of resources */
  *found = TRUE;
  nof_cycles = (cycle_cons == ARAD_PP_FP_KEY_CYCLE_ANY)? 2 :1;
  cycle_base = (cycle_cons == ARAD_PP_FP_KEY_CYCLE_ODD)? 1 :0;
  new_cycle_cons = cycle_cons;
  tmp_start_place = *place_start;
  dir_ext_start_place = *place_start;

  res = arad_pp_fp_key_progs_rsrc_stat(
          unit, 
          stage, 
          flags,
          db_id, 
          exist_progs, 
          new_cycle_cons, 
          is_for_direct_extraction, 
          cycle_needed,
          ce_rsrc_bmp_glbl_lcl, 
          selected_cycle, 
          key_zone_enough,
          &flp_max_key_size_in_bits[0],
          &dir_ext_start_place
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_fp_db_cascaded_cycle_get(
      unit,
     db_id,
      &is_cascaded,
      &lookup_id /* 1st or 2nd cycle */
  );
  SOC_SAND_CHECK_FUNC_RESULT(res, 21, exit);

  for (cycle_idx = cycle_base; cycle_idx < (cycle_base + nof_cycles); cycle_idx++) {

      /*If direct extraction and patch is enabled, find the first available 80bit key*/
      tmp_start_place = (is_for_direct_extraction && soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "optimized_de_allocation", 0)) ? dir_ext_start_place  : *place_start;

      cycle_id = cycle_idx;
      if (nof_cycles == 2)
      {
          if (cycle_idx == 0)
          {
              cycle_id = cycle_needed[0]? 0: 1;
          }
          else {
              cycle_id = cycle_needed[0]? 1: 0;
          }
      }

   
        /*
         * All qualifiers are broken into CEs and key construction is according to Qualifiers 
         * (qualifier may be parted according to size constraints in order to maximize the key capacity). 
         * Then qualifiers are sorted according to constraints and size: LSB qualifiers come first, 
         * followed by MSB qualifiers and last come the qualifiers with no special demand. 
         * All sub-categories are sorted according to their size (largest first). 
         * Qualifiers are places by key such that a construction of 80 bit key will be attempted
         * first, then 160 bit and then 320 bit - as a last resort.
         */

        /*
         * For all qualifiers get CE information
         */ 
        res = arad_pp_fp_qual_ce_mltpl_info_get(
                    unit,
                    stage,
                    flags,
                    new_qual_types,
                    &nof_quals,
                    ce_info
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        /* 
         * Sort all qualifiers, so that LSBs will be first, then MSBs and then the rest.  
         * Also, for each sub-category, the qualifiers will be sorted according to their                     .
         * size (biggest first).
         */
        if (!(flags & ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER)) {
            /* Do not reorder in case of large DE DBs */
            res = arad_pp_fp_qual_ce_info_sort(
                    flags,
                    nof_quals, 
                    new_qual_types, 
                    ce_info
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        }
        
        /* In Jericho devices and above, an optimization of the resource allocation
         * can be used. The regular allocation sorts qualifiers in decreasing size
         * order, and allocates instructions and keys from LSB to MSB.
         * This means that bigger qualifiers are allocated in LSB, and require less
         * instructions to fill up LSB key. The small qualiifers are then allocated
         * in MSB and require more MSB instructions to fill up the key.
         * This optimization allocates instructions in a balanced manner.
         * If it is known that key cannot be constructed in LSB part only (MSB half
         * must also be used), then after sort (same sort), allocate once in LSB
         * and once in MSB and so on. That will cause the CE resources to be more
         * balanced, and next keys that are allocated will not run into
         * out-of-resources in MSB. 
         * */ 

        /* Init */
        algorithm_args.balance_enabled = FALSE;
        algorithm_args.balance_lsb_msb = 0;

        if (SOC_IS_JERICHO(unit) &&
            soc_property_get(unit, spn_FIELD_KEY_ALLOCATION_MSB_BALANCE_ENABLE, FALSE) &&
            stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF)
        {
            /* calculate sum of all qualifiers in database */
            qual_sum = arad_pp_fp_qual_sum(ce_info, nof_quals);
            
            if (qual_sum > ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE) 
            {
                /* For keys over 80 bit enable balance optimization */
                algorithm_args.balance_enabled = TRUE;

                /* For cascaded qualifier only LSB can be used and thus balance should
                 * prefer LSB for this specific qualifier. Since it must be the first
                 * qualifier in after sort, then initializing to 0 suffices. */ 
                algorithm_args.balance_lsb_msb = 0;
            }
        }
        
        direct_ext_2_needed = TRUE; 
        total_ce_indx = 0;
        for (direct_ext_loop_indx = 0; (direct_ext_loop_indx < (1 + is_for_direct_extraction)) && direct_ext_2_needed; direct_ext_loop_indx++) {
            *found = TRUE;
            direct_ext_2_needed = FALSE; /* Set to False unless error */

            /* For Each qualifier get what is the constraint */
            for(zone_idx = 0; zone_idx < ARAD_PP_FP_KEY_NOF_ZONES; zone_idx++)
            {
              total_bits_in_zone[zone_idx] = 0;
              
              if ( is_for_direct_extraction ) 
              {
                  /* Optimized direct extraction allocation: start from Low to High - always single key.
                   * If cascaded, start with LSB first, if LSB is full and MSB comes into turn - it will
                   * fail at a later stage (because of the lsb only constraint on cascaded databases).
                   */
                  if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "optimized_de_allocation", 0)
                      || is_cascaded)
                  {
                      zone_id_first  = ( tmp_start_place == ARAD_PP_FP_KEY_CE_LOW ) ?  ARAD_PP_FP_KEY_ZONE_LSB_0 : ARAD_PP_FP_KEY_ZONE_MSB_0;
                      zone_id_second = ( tmp_start_place == ARAD_PP_FP_KEY_CE_LOW ) ?  ARAD_PP_FP_KEY_ZONE_MSB_0 : ARAD_PP_FP_KEY_ZONE_LSB_0;
                       if (((direct_ext_loop_indx == 0) && (zone_idx != zone_id_first))
                          || ((direct_ext_loop_indx == 1) && (zone_idx != zone_id_second))) {
                          total_bits_in_zone[zone_idx] = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE;
                      }

                  }
                  else 
                  {
                      if (((direct_ext_loop_indx == 0) && (zone_idx != ARAD_PP_FP_KEY_ZONE_MSB_0))
                          || ((direct_ext_loop_indx == 1) && (zone_idx != ARAD_PP_FP_KEY_ZONE_LSB_0))) {
                          total_bits_in_zone[zone_idx] = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE;
                      }
                  }
              }
              if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB) {
                  if (((place_cons == ARAD_PP_FP_KEY_CE_HIGH) && (zone_idx != ARAD_PP_FP_KEY_ZONE_MSB_0))
                      || ((place_cons == ARAD_PP_FP_KEY_CE_LOW) && (zone_idx != ARAD_PP_FP_KEY_ZONE_LSB_0))) {
                      total_bits_in_zone[zone_idx] = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE;
                  }
              }
            }
            /* For Each qualifier get what is the constraint */
            for(is_32b = FALSE; is_32b <= TRUE; is_32b++)
            {
                for(is_msb = FALSE; is_msb <= TRUE; is_msb++)
                {
                    for(is_high_group = FALSE; is_high_group <= TRUE; is_high_group++)
                    {
                      nof_free_ces_in_zone[ARAD_PP_FP_KEY_CATEGORY_GET(is_32b, is_msb, is_high_group)] = 
                          arad_pp_fp_key_nof_free_instr_get(
                              unit,
                              stage, 
                              ce_rsrc_bmp_glbl_lcl[cycle_id],
                              is_32b,
                              is_msb,
                              is_high_group);
                    }
                }
            }

            if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)&&SOC_IS_JERICHO(unit)) {
                total_bits_in_zone[0] = ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS - flp_max_key_size_in_bits[0];
                total_bits_in_zone[1] = ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS - flp_max_key_size_in_bits[1];
            }

            for(qual_indx = 0;  (qual_indx < nof_quals) && (*found); ++qual_indx) 
            {
              /* 
               * calculation is optimized by breaking the qualifiers into CEs according to space left
               * in zones in arad_pp_fp_qual_ce_info_uneven_get()
               */
              sal_memset(&(ce_info[qual_indx]), 0x0, sizeof(ARAD_PP_FP_QUAL_CE_INFO));
              sal_memset(zone_used, 0x0, sizeof(uint8) * ARAD_PP_FP_KEY_NOF_ZONES);

              /* Special case FLP: skip the last qualifier if alreayd byte-aligned */ 
              if ((flags & ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT) && (new_qual_types[qual_indx] == SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES)) {
                  if (SOC_SAND_DIV_ROUND_UP(total_bits_in_zone[0], SOC_SAND_NOF_BITS_IN_CHAR) == 0) {
                      /* Byte aligned, skip this not-mandatory qualifier continue */
                      continue;
                  }
              }

              soc_res = arad_pp_fp_is_qual_in_qual_arr(unit, 
                                                       qual_types, 
                                                       SOC_PPC_FP_NOF_QUALS_PER_DB_MAX, 
                                                       SOC_PPC_FP_QUAL_KEY_AFTER_HASHING, 
                                                       &is_slb_hash_in_quals);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(soc_res, 151, exit);

              /* In SLB hash key copy to PMF key no CE is used. */
              if (is_slb_hash_in_quals && stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
                  total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_MSB_0] = 0;
                  continue;
              }

              res = arad_pp_fp_qual_ce_info_uneven_get(
                      unit,
                      stage,
                      TRUE, /* Optimize */
                      flags,
                      new_qual_types[qual_indx],
                      &algorithm_args, 
                      total_bits_in_zone,
                      nof_free_ces_in_zone,
                      zone_used,
                      &(ce_info[qual_indx]),
                      &success
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);


              /* Key construction cannot be completed in any way */
              if(!success)
              {
                LOG_DEBUG(BSL_LS_SOC_FP,
                          (BSL_META_U(unit,
                                      "    "
                                      "Key: fail to allocate Database %d because not enough space to split qualifiers\n\r"), db_id));
                *found = FALSE;
                total_ce_indx = 0;
                direct_ext_2_needed = TRUE;
                continue; /* not enough place for the qualifiers */
              }

              /* 
               * Verify that constraints are met: for each zone that was used,
               * verify it does not contradict the constraints 
               */
              for(zone_idx = 0; zone_idx < ARAD_PP_FP_KEY_NOF_ZONES; zone_idx++)
              {
                  int max_bits_for_zone;
                  /* Zone ID changes */
                  if(!zone_used[zone_idx])
                      continue;

                  zone_id = zone_idx;
                  max_bits_for_zone = ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)&&(SOC_IS_JERICHO(unit)))? ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS : flp_max_key_size_in_bits[0];
                /* 
                 * here the constraint is to use low only, however it couldn't be 
                 * reached in construction 
                 */
                if ((place_cons == ARAD_PP_FP_KEY_CE_LOW) && 
                    (zone_id > ARAD_PP_FP_KEY_ZONE_MSB_0 || 
                     zone_id == ARAD_PP_FP_KEY_ZONE_MSB_0 || 
                     zone_id == ARAD_PP_FP_KEY_ZONE_MSB_1)) 
                {
                        LOG_DEBUG(BSL_LS_SOC_FP,
                                  (BSL_META_U(unit,
                                              "    "
                                              "Key: fail to allocate Database %d because zone tested %d is not acceptable with place_cons %d \n\r"), 
                                   db_id, zone_id, place_cons));
                        *found = FALSE;
                        total_ce_indx = 0;
                        direct_ext_2_needed = TRUE;
                        continue; /* not enough place for the qualifiers */
                }
                /* 
                * here the constraint is to use one key, however the key construction 
                * could only complete with two keys 
                */
                else if ((place_cons == ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE) && (zone_id > ARAD_PP_FP_KEY_ZONE_MSB_0)) 
                {
                        LOG_DEBUG(BSL_LS_SOC_FP,
                                  (BSL_META_U(unit,
                                              "    "
                                              "Key: fail to allocate Database %d because zone tested %d is not acceptable with place_cons %d \n\r"), 
                                   db_id, zone_id, place_cons));
                        *found = FALSE;
                        total_ce_indx = 0;
                        direct_ext_2_needed = TRUE;
                        continue; /* not enough place for the qualifiers */
                }
                else if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) && ( max_bits_for_zone < total_bits_in_zone[zone_id])) {
                    /* ACL too large for FLP */
                    LOG_DEBUG(BSL_LS_SOC_FP,
                              (BSL_META_U(unit,
                                          "    "
                                          "Key: fail to allocate Database %d because FLP Key-C size remaining is %d bits less than ACL current key size %d \n\r"), 
                               db_id, flp_max_key_size_in_bits[zone_id], total_bits_in_zone[zone_id]));
                    *found = FALSE;
                    total_ce_indx = 0;
                    direct_ext_2_needed = FALSE;
                    ARAD_DO_NOTHING_AND_EXIT; /* not enough place for the qualifiers */
                }
              }
              
              /* ce info to constraints */
              for(ce_indx = 0; ce_indx < ce_info[qual_indx].nof_ce; ++ce_indx) 
              {
                if(total_ce_indx >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG) 
                {
                 LOG_ERROR(BSL_LS_SOC_FP,
                           (BSL_META_U(unit,
                                       "Unit: %d too much qualifiers\n\r"
                                       "Total qualifiers %d, Maximum qualifiers allowed %d.\n\r"),
                            unit, total_ce_indx, ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG));
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 60, exit);
                }
                arad_pp_fp_key_qual_to_ce_const(stage, &(ce_info[qual_indx]),ce_indx,&(ce_const[total_ce_indx]));
                ce_const[total_ce_indx].qual_type = new_qual_types[qual_indx];

                zone_id = ce_info[qual_indx].ce_zone_id[ce_indx];
                
                
                if((zone_id == ARAD_PP_FP_KEY_ZONE_MSB_0) || (zone_id == ARAD_PP_FP_KEY_ZONE_MSB_1)) {
                  ce_const[total_ce_indx].place_cons = ARAD_PP_FP_KEY_CE_HIGH;
                }
                else {
                  ce_const[total_ce_indx].place_cons = ARAD_PP_FP_KEY_CE_LOW;
                }

                if (zone_id > ARAD_PP_FP_KEY_ZONE_MSB_0) {
                  ce_const[total_ce_indx].is_second_key = TRUE;
                }

                total_ce_indx++;
              }
            } /* for qualifiers */
        } /* direct extraction loop */

        if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)&&(SOC_IS_JERICHO(unit))) {
            total_bits_in_zone[0] -= (ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS - flp_max_key_size_in_bits[0]);
            total_bits_in_zone[1] -= (ARAD_PP_FLP_KEY_C_ZONE_SIZE_JERICHO_BITS - flp_max_key_size_in_bits[1]);
        }

        if(is_for_direct_extraction || (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)) {
            place_start_lcl = (zone_id ==0) ? ARAD_PP_FP_KEY_CE_LOW : ARAD_PP_FP_KEY_CE_HIGH;
        }
        else {
            /* If second key was used */
            if (total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_MSB_1] > 0 || total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_LSB_1] > 0) 
            {
              /* Double key */
              place_start_lcl = ARAD_PP_FP_KEY_CE_PLACE_ANY; 
            }
            /* If MSB was used - for TCAM / DT, usage of MSB implies using reserving also LSB */
            else if(total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_MSB_0] > 0) 
            {
              /* start allocate from low/high, we already know we will use msb, so no constrain to use also lsb  */
              place_start_lcl = ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE;
            }
            else 
            {
              place_start_lcl = ARAD_PP_FP_KEY_CE_LOW;
            }
        }

        /* 
         * Validate if there is enough place for this place_start in the Keys
         */
        is_key_not_found = FALSE;

        /* In direct extraction take calculated start place,
         * Otherwise take the hardest constraint if necessary
         */

        tmp_start_place = is_for_direct_extraction 
            ? place_start_lcl : SOC_SAND_MAX(place_start_lcl, tmp_start_place);
        
        for(place_ndx = 0; place_ndx < ARAD_PP_FP_NOF_KEY_CE_PLACES_STAGE; place_ndx++) {
            /* Find correct place index */
            if (Arad_pp_fp_place_const_convert[place_ndx][0] == tmp_start_place) {
                if (!key_zone_enough[place_ndx][cycle_id]) {
                    /* A PMF-Program does not have enough place for this Key size */
                    LOG_DEBUG(BSL_LS_SOC_FP,
                              (BSL_META_U(unit,
                                          "    "
                                          "Key: fail to allocate Database %d because the no key available at this size %d \n\r"), 
                               db_id, tmp_start_place));
                    *found = FALSE;
                    is_key_not_found = TRUE;
                    break; 
                }
            }
        } /*        for(place_ndx = 0; place_ndx < ARAD_PP_FP_NOF_KEY_CE_PLACES_STAGE; place_ndx++) */
        if(is_key_not_found || (*found == FALSE)) {
            /* Try the second cycle */
            continue;
        }

      *nof_consts = total_ce_indx;
      /* The allocation in this cycle succeeds, thus  */

      /* The macro SOC_DPP_DEFS_MAX can cause same_on_both_sides defects in case that there is a same value 
       * in both of the chips. Ignore such defects */
      /* coverity[same_on_both_sides] */
      for(prog_ndx = 0; prog_ndx < ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES; prog_ndx++) {
          selected_cycle[prog_ndx] = cycle_id;
      }
      /* Success - stop here (no need for another cycle to consider)*/
      break;
  } /*  for (cycle_idx = cycle_base; cycle_idx < (cycle_base + nof_cycles); cycle_idx++) */

  *place_start = tmp_start_place;

exit:
  ARAD_FREE(ce_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_alloc_constrain_calc()", 0, 0);
}


STATIC
 uint32
  arad_pp_fp_key_dealloc_in_prog(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE       stage,
    SOC_SAND_IN  uint32                       prog_ndx,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX] /* qualifiers*/
  )
{
    uint32
        key_id,
        ce_indx=0,
        ce_rsrc_bmp[1],
        keys_bmp[1],
        access_profile_array_id,
        nof_access_profiles;
    uint8
        is_cascaded,
        lookup_id,
        found,
        ce_msb;
    ARAD_PMF_CE
        sw_db_ce;
    uint32
      res = SOC_SAND_OK;
    ARAD_PP_FP_KEY_DP_PROG_INFO
        db_prog_info;
    ARAD_PP_FP_KEY_ALLOC_INFO   
        alloc_info;
    ARAD_TCAM_BANK_ENTRY_SIZE
        entry_size;
    ARAD_PP_IHB_PMF_PASS_2_KEY_UPDATE_TBL_DATA
        pass_2_key_update_tbl_data;
    SOC_PPC_FP_DATABASE_INFO
        db_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_PP_FP_KEY_ALLOC_INFO_clear(&alloc_info);

    /* get CE used for this DB */
    res = arad_pp_fp_db_prog_info_get(
            unit,
            stage,
            db_id,
            prog_ndx,
            &db_prog_info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
            unit,
            stage,
            db_id,
            &db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);

    /* Invalid all the CEs */
    for(ce_indx = 0; ce_indx < db_prog_info.nof_ces; ++ce_indx) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(
                unit,
                stage,
                prog_ndx,
                db_prog_info.cycle,
                db_prog_info.ces[ce_indx],
                &sw_db_ce
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

        ce_msb = db_prog_info.ces[ce_indx] >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB;
        res = arad_pmf_ce_nop_entry_set_unsafe(
                unit,
                stage,
                prog_ndx,
                db_prog_info.key_id[sw_db_ce.is_second_key],
                db_prog_info.ces[ce_indx]%ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB, /* ce in lsb/msb*/
                ce_msb,
                db_prog_info.cycle,
                TRUE /*is_ce_not_valid*/
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10+ce_indx, exit);

        /* Disable this Copy Engine */
        ARAD_CLEAR(&sw_db_ce, ARAD_PMF_CE, 1);
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.set(
                unit,
                stage,
                prog_ndx,
                db_prog_info.cycle,
                db_prog_info.ces[ce_indx],
                &sw_db_ce
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

        /* Remove it from the CE resource bitmap */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(
                  unit,
                  stage,
                  prog_ndx,
                  db_prog_info.cycle,
                  ce_rsrc_bmp
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
        SHR_BITCLR(ce_rsrc_bmp, db_prog_info.ces[ce_indx]);
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.set(
                  unit,
                  stage,
                  prog_ndx,
                  db_prog_info.cycle,
                  *ce_rsrc_bmp
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 33, exit);
    }

    /* Only Key id and cycle are needed for the Access profile deallocate */
    alloc_info.key_id[0] = db_prog_info.key_id[0];
    alloc_info.key_id[1] = db_prog_info.key_id[1];
    alloc_info.cycle = db_prog_info.cycle;

    /* Compute the number of access profiles: 320b key or large action size */
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id), &entry_size);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);

    /* Set nof_access_profiles to 0 if Direct-Extraction */
    nof_access_profiles = 0;
    if ((db_info.db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) && (db_info.db_type != SOC_PPC_FP_DB_TYPE_SLB)) {
        nof_access_profiles = (db_prog_info.is_320b || (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS))? 2 : 1; /* Number of keys */
    }
    for (access_profile_array_id = 0; access_profile_array_id < nof_access_profiles; access_profile_array_id++) {
        /* Set an invalid access-profile */
        /* Set in the HW the access profile - according to the stage  - can be a separate function */
        res = arad_pp_fp_key_access_profile_hw_set(
                unit,
                stage,
                prog_ndx,
                access_profile_array_id,
                ARAD_TCAM_NOF_ACCESS_PROFILE_IDS, /* access_profile_id */ 
                &alloc_info
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    }

    /* Disable the cascaded action if the database is of type cascaded */
    /* Check if there is a key-change action or qualifier */
    res = arad_pp_fp_db_cascaded_cycle_get(
            unit,
            db_id,
            &is_cascaded,
            &lookup_id /* 1st or 2nd cycle */
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    if (is_cascaded && (lookup_id == 0)) {
        if (!SOC_IS_JERICHO_PLUS(unit)) {
            res = arad_pp_ihb_pmf_pass_2_key_update_tbl_get_unsafe(
                    unit,
                    prog_ndx,
                    &pass_2_key_update_tbl_data
              );
            SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

            pass_2_key_update_tbl_data.enable_update_key = 0;
            pass_2_key_update_tbl_data.action_select = 0;

            if (SOC_IS_ARADPLUS(unit))
            {
                uint8 is_equal;
                uint32 place_unused;
                uint32 *key_bits[4];
                int i = 0;
                key_bits[i++] = &pass_2_key_update_tbl_data.key_a_lem_operation_select;
                key_bits[i++] = &pass_2_key_update_tbl_data.key_b_lem_operation_select;
                key_bits[i++] = &pass_2_key_update_tbl_data.key_c_lem_operation_select;
                key_bits[i++] = &pass_2_key_update_tbl_data.key_d_lem_operation_select;
             
                res = arad_pp_fp_db_is_equal_place_get(
                    unit,
                    db_id,
                    &is_equal,
                    &place_unused
                );
                SOC_SAND_CHECK_FUNC_RESULT(res, 75, exit);

                if (is_equal) {
                    pass_2_key_update_tbl_data.key_d_use_compare_result = 0; 
                    pass_2_key_update_tbl_data.key_d_mask_select = 0; 
                    pass_2_key_update_tbl_data.key_d_xor_enable = 0;
                }

                /* Reset the SLB LEM key copy bit. */
                *(key_bits[db_prog_info.key_id[0]]) = 0;
            }
            
            res = arad_pp_ihb_pmf_pass_2_key_update_tbl_set_unsafe(
                    unit,
                    prog_ndx,
                    &pass_2_key_update_tbl_data
              );
            SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
        }
    }

    
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(
                unit,
                stage,
                prog_ndx,
                db_prog_info.cycle,
                keys_bmp  
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);

    /* Unset from the SW DB 80s bitmap the DB */
    key_id = db_prog_info.key_id[0];
    res = arad_pp_fp_key_alloc_key_bitmap_configure(
            unit,
            stage,
            db_id,
            db_prog_info.alloc_place,
            FALSE, /* is_to_set */
            keys_bmp, 
            &key_id, 
            &found
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.set(
            unit,
            stage,
            prog_ndx,
            db_prog_info.cycle,
            *keys_bmp  
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_dealloc_in_prog()", 0, 0);
}


/* Dealloc the key(s) from the Database in all its PMF-Programs */
uint32
  arad_pp_fp_key_dealloc(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE            stage,
    SOC_SAND_IN  uint32                       db_id,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE         qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX] /* qualifiers*/
  )
{
  uint32   
      cur_prog = 0;
  uint32
      exist_progs;
  uint32
    res = SOC_SAND_OK;
  
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* get relevant programs for the DB */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(
            unit,
            stage,
            db_id,
            0,
            &exist_progs
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  for (cur_prog = 0; cur_prog < ARAD_PMF_LOW_LEVEL_NOF_PROGS; cur_prog++) {
      if (exist_progs & (1 << cur_prog)) {
          res = arad_pp_fp_key_dealloc_in_prog(
                    unit,
                    stage,
                    cur_prog,
                    db_id,
                    qual_types
                 );
          SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
      }
  }
          
 
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_dealloc()", 0, 0);
}


uint32 arad_pp_fp_ce_info_get(      
          SOC_SAND_IN  int                    unit,
          SOC_SAND_IN  uint32                 db_id,
          SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE stage,
          SOC_SAND_OUT ARAD_PMF_CE      *ce_array,
          SOC_SAND_OUT uint8                  *nof_ces,
          SOC_SAND_OUT uint8                  *is_key_320b,
          SOC_SAND_OUT uint8                  *ces_ids
       )
{

    ARAD_PP_FP_KEY_DP_PROG_INFO db_prog_info;
    SOC_PPC_FP_DATABASE_INFO *fp_database_info = NULL;
    uint32 res = SOC_SAND_OK, db_prog, prog_result, exist_progs[1];
    int i;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_ALLOC(fp_database_info, SOC_PPC_FP_DATABASE_INFO, 1, "arad_pp_fp_ce_info_get.fp_database_info");
    /* Get the database Info */
     SOC_PPC_FP_DATABASE_INFO_clear(fp_database_info);

     res = arad_pp_fp_database_get_unsafe(unit, db_id, fp_database_info);
     SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

     if (fp_database_info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL) {

         res = arad_pp_dbal_ce_info_get(unit, db_id, stage, ce_array, nof_ces, is_key_320b, ces_ids);              
         SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
     }else if (fp_database_info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) {
        res = arad_pp_fp_kbp_match_key_ces_get(unit, fp_database_info->internal_table_id, ce_array, nof_ces);
        SOC_SAND_CHECK_FUNC_RESULT(res, 8, exit);
        (*is_key_320b) = 0;
        for(i = 0; i < (*nof_ces); i++){
            ces_ids[i] = i;
        }
     } else {        
        /* get relevant programs for the DB */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(unit, stage, db_id, 0, exist_progs);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

        db_prog = 0;
        ARAD_PP_FP_KEY_FIRST_SET_BIT(exist_progs,db_prog,ARAD_PMF_LOW_LEVEL_NOF_PROGS,ARAD_PMF_LOW_LEVEL_NOF_PROGS,FALSE,prog_result);
        if(prog_result == 0) {
            LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"Unit %d Stage %s No programss available.\n\r"),unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 20, exit);
        }

        /* get CE used for this DB */
        res = arad_pp_fp_db_prog_info_get( unit, stage, db_id, db_prog, &db_prog_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

        (*nof_ces) = db_prog_info.nof_ces;
        (*is_key_320b) = db_prog_info.is_320b;

        for(i = 0; i < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS; ++i) {
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(unit, stage, db_prog, db_prog_info.cycle, i, &ce_array[i]);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);
        }
        sal_memcpy(ces_ids, &(db_prog_info.ces[0]), ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS);
    }

exit:
    ARAD_FREE(fp_database_info);
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_ce_info_get()", db_id, 0);
}

/*
 * given qual values of DB, 
 * build TCAM value and mask 
 */
uint32
  arad_pp_fp_key_value_buffer_mapping(
      SOC_SAND_IN  int                              unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE           stage,
      SOC_SAND_IN  uint32                           db_id,
      SOC_SAND_IN  ARAD_PP_FP_KEY_BUFFER_DIRECTION  direction, /* 0 - value to buffer, 1 - buffer to value*/
      SOC_SAND_IN  uint8                            is_for_kbp,
      SOC_SAND_IN  uint8                            is_from_dbal,
      SOC_SAND_INOUT  SOC_PPC_FP_QUAL_VAL      qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
      SOC_SAND_INOUT  uint32                   value_out[ARAD_PP_FP_TCAM_ENTRY_SIZE], /* In/Out according to the direction */
      SOC_SAND_INOUT  uint32                   mask_out[ARAD_PP_FP_TCAM_ENTRY_SIZE] /* In/Out according to the direction */
  )
{    
    int32
        key_ndx,   
        ce_indx=0;
    uint32 
        qual_index,  
        curr_msb,
        buffer_to_key_nof_lsb[2] = {0, 0},
        buffer_to_key_nof_msb[2] = {0, 0},
        qual_type = -1,
        nof_lsb[2]={0, 0}, /* nof bits in lsb part */
        nof_msb[2]={0, 0},
        moved_to_msb=0, /* first move to msb */
        nof_copy_bits,
        skip_lsb = 0,
        first_key_size,
        qual_lsb[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX+1], /* for each qualifier how many bits was eaten */
        buf_msb=ARAD_PP_FP_TCAM_ENTRY_SIZE*32,
        flp_lsb = 0,
        res = SOC_SAND_OK;
    int msb_padding_bits = 0;

    ARAD_PMF_CE sw_db_ce;
    uint32                   
        value[ARAD_PP_FP_TCAM_ENTRY_SIZE],
        mask[ARAD_PP_FP_TCAM_ENTRY_SIZE];

    SOC_PPC_FP_QUAL_TYPE qual_type_lcl = SOC_PPC_FP_QUAL_IRPP_INVALID;
    
    uint8 qual_found;
    uint8 ce_ids[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS] = {0};
    uint8 is_key_320b = 0;
    uint8 nof_ces_to_handle;
    ARAD_PMF_CE ce_array[ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS] = {{0}};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* 
     * The value and mask are represented like this: 
     *  
     * |LSB-CE0...LSB-CE15|MSB-CE0...MSB-CE15|0..0| ... [twice if double key] 
     *  
     *  i.e. aligned to the MSB of the buffer. At the end, we copy the LSB part in
     *  the LSB of the buffer and the MSB part after 80 bits
     *  
     *  In the other direction, we copy directly the LSB and MSB parts as described above
     */

    /* init masking */
    sal_memset(value,0x0, sizeof(uint32) * ARAD_PP_FP_TCAM_ENTRY_SIZE);
    sal_memset(mask,0x0, sizeof(uint32) * ARAD_PP_FP_TCAM_ENTRY_SIZE);
    sal_memset(qual_lsb,0x0, sizeof(uint32) * SOC_PPC_FP_NOF_QUALS_PER_DB_MAX+1);

    if(is_from_dbal)
    {
        res = arad_pp_dbal_ce_info_get(unit, db_id, stage, &ce_array[0], &nof_ces_to_handle, &is_key_320b, &ce_ids[0]);              
        SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);
    }else{
        res = arad_pp_fp_ce_info_get(unit, db_id, stage, &ce_array[0], &nof_ces_to_handle, &is_key_320b, &ce_ids[0]);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }    

#ifdef ARAD_PP_FP_DEBUG_PRINTS
    {
        int i;

        LOG_CLI((BSL_META("   ""\nTable qualifiers:\n")));        
        for (i = 0; i < nof_ces_to_handle; i++) {
            LOG_CLI((BSL_META("   ""\tqual_type = %s, qual_lsb=%d\n"), SOC_PPC_FP_QUAL_TYPE_to_string(ce_array[i].qual_type), ce_array[i].qual_lsb));        
        }

        LOG_CLI((BSL_META("   ""\nInput qualifiers:\n")));
        for (i = 0; i < nof_ces_to_handle+5; i++) {
            LOG_CLI((BSL_META("   ""\tqual_type = %s (%d), val = %d\n"), SOC_PPC_FP_QUAL_TYPE_to_string(qual_vals[i].type), qual_vals[i].type, qual_vals[i].val.arr[0]));                
        }
    }
#endif

    
    /* 
     * Build the buffer back 
     * Compute in the buffer->key direction the number of relevant bits
     */
    if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_BUFFER_TO_KEY) {
        for(ce_indx = 0; ce_indx < nof_ces_to_handle; ++ce_indx) {

            if(ce_ids[ce_indx] >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB) {
                buffer_to_key_nof_msb[ce_array[ce_ids[ce_indx]].is_second_key] += ce_array[ce_ids[ce_indx]].msb + 1;
            }else {
                buffer_to_key_nof_lsb[ce_array[ce_ids[ce_indx]].is_second_key] += ce_array[ce_ids[ce_indx]].msb + 1;
            }
        }
        
        /* Copy the local buffer similarly to the other direction */
        curr_msb = ARAD_PP_FP_TCAM_ENTRY_SIZE*32;
        for (key_ndx = 0; key_ndx <= is_key_320b; key_ndx++) {
            /* Copy the LSB part */
            curr_msb -= buffer_to_key_nof_lsb[key_ndx];
            nof_copy_bits = buffer_to_key_nof_lsb[key_ndx];
            SHR_BITCOPY_RANGE(mask, curr_msb, mask_out, (160 * key_ndx), nof_copy_bits);
            SHR_BITCOPY_RANGE(value, curr_msb, value_out, (160 * key_ndx), nof_copy_bits);
            LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"LSB  Buffer-->Key:  ce-id:%d, type:%s, curr_msb %d, nof_copy_bits %d val %X %X %X mask %X\n\r"),
                       ce_ids[ce_indx], SOC_PPC_FP_QUAL_TYPE_to_string(ce_array[ce_ids[ce_indx]].qual_type), curr_msb, nof_copy_bits, value_out[160 * key_ndx - 1], value_out[160 * key_ndx], value_out[160 * key_ndx + 1], mask_out[160 * key_ndx]));

            /* Copy the MSB part */
            curr_msb -= buffer_to_key_nof_msb[key_ndx];
            nof_copy_bits = buffer_to_key_nof_msb[key_ndx];
            /* Start from 0 in SLB since it is another key in fact */
            skip_lsb = (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)? 0: 80;
            SHR_BITCOPY_RANGE(mask, curr_msb, mask_out, skip_lsb + (160 * key_ndx), nof_copy_bits);
            SHR_BITCOPY_RANGE(value, curr_msb, value_out, skip_lsb + (160 * key_ndx), nof_copy_bits);
            LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"MSB  Buffer-->Key:  ce-id:%d, type:%s, curr_msb %d, nof_copy_bits %d skip_lsb %d val %X mask %X\n\r"),
                       ce_ids[ce_indx], SOC_PPC_FP_QUAL_TYPE_to_string(ce_array[ce_ids[ce_indx]].qual_type), curr_msb, nof_copy_bits, skip_lsb, value_out[(skip_lsb+160 * key_ndx)/32], mask_out[(skip_lsb+ 160 * key_ndx)/32]));

        }
    }

    /* 
     *  Shared step: copy the bits in the correct place
     *  between the local buffer (as described above) and
     *  the qualifiers type-value-mask
     */ 
    /* Run on the key order */
    for (key_ndx = 0; key_ndx <= is_key_320b; key_ndx++) {
        first_key_size = nof_lsb[0] + nof_msb[0]; /* 0 at the beginning, 1st key size at the second round */
        moved_to_msb = 0;
        /* for CE, get which bits of qualifier it writes and where */
        for(ce_indx = 0; ce_indx < nof_ces_to_handle; ++ce_indx) {

            sw_db_ce = ce_array[ce_ids[ce_indx]];
            
            if (sw_db_ce.is_second_key != key_ndx) {
                /* Take for this key only the corresponding Copy Engines */
                continue;
            }
            LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"   Key value-buffer: direction:%d, ce-id:%d, type:%s \n\r"), direction, 
                       ce_ids[ce_indx], SOC_PPC_FP_QUAL_TYPE_to_string(sw_db_ce.qual_type)));

            if(ce_ids[ce_indx] >= ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB && moved_to_msb == 0) {
                nof_lsb[key_ndx]= (ARAD_PP_FP_TCAM_ENTRY_SIZE*32 ) - buf_msb - first_key_size;
                moved_to_msb = 1;
            }
            
            /* check which qualifier is built by this CE */
            qual_found = 0;
            for(qual_index = 0; qual_index < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX ; ++qual_index) {
                /* Take the list of qualifier types from different places per direction */
                qual_type_lcl = qual_vals[qual_index].type;
                if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_KEY_TO_BUFFER) {
                    /* Continue the input qualifier list if not existing */
                    if(qual_type_lcl == BCM_FIELD_ENTRY_INVALID) {
                        continue;
                    }
                }
                else if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_BUFFER_TO_KEY) {
                    /* 
                     * Loop on the qual_index to find if this qualifier already 
                     * appears. If so, use this qual_index to fill the same qualifier 
                     * Otherwise, the first that is empty 
                     */
                    if(qual_type_lcl == BCM_FIELD_ENTRY_INVALID) {
                        /* Empty spot to insert this qualifier */
                        qual_found = 1;
                        qual_type = qual_type_lcl;
                        break;
                    }
                }

                /* 
                 * In buffer -> key, qualifier already present found
                 *  In key -> buffer, the input qualifier found in the list of the
                 *  Database instruction qualifiers
                 */
                if(sw_db_ce.qual_type == qual_type_lcl){
                    qual_found = 1;
                    qual_type = qual_type_lcl;
                    break;
                }
            }
            if(!qual_found) {
                /* it's OK, qualifier could be masked*/
                buf_msb -= (sw_db_ce.msb + 1); /* sw_db_ce.lsb + sw_db_ce.msb - sw_db_ce.lsb + 1;*/
                if (moved_to_msb == 0) {
                    nof_lsb[key_ndx] = (ARAD_PP_FP_TCAM_ENTRY_SIZE*32) - buf_msb - first_key_size;
                }
                LOG_DEBUG(BSL_LS_SOC_FP,
                          (BSL_META_U(unit," db_id %d  qual:%s  qual is masked. skip to %d (%d bits)\n\r"), db_id, SOC_PPC_FP_QUAL_TYPE_to_string(qual_type_lcl), buf_msb, sw_db_ce.msb + 1));
                continue;
            }


            /* write valid bits */
            nof_copy_bits = sw_db_ce.msb - sw_db_ce.lsb + 1;
            LOG_DEBUG(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "   db_id %d, "
                                  "skip lsb:%d, "
                                  "buf_msb:%d, "
                                  "qual-lsb bits:%d, "
                                  "qual-lsb bits sw db:%d, "
                                  "# copied bits:%d,"
                                  " nof_lsb[key_ndx %d] %d\n\r"), db_id, sw_db_ce.lsb, buf_msb, qual_lsb[qual_index], sw_db_ce.qual_lsb, nof_copy_bits, key_ndx, nof_lsb[key_ndx]));

            {
                uint32
                    copied_val=0,
                    copied_maks=0;

                if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_KEY_TO_BUFFER) {
                    SHR_BITCOPY_RANGE(&copied_val, 0, qual_vals[qual_index].val.arr, sw_db_ce.qual_lsb, nof_copy_bits);
                    SHR_BITCOPY_RANGE(&copied_maks, 0, qual_vals[qual_index].is_valid.arr, sw_db_ce.qual_lsb, nof_copy_bits);
                    LOG_DEBUG(BSL_LS_SOC_FP,
                              (BSL_META_U(unit,
                                          "   db_id %d, "
                                          "qual:%s, "
                                          "copied val:%08x, "
                                          "copied mask:%08x \n\r"
                                          "qual_vals[qual_index].val.arr:%08x %08x "
                                          "qual_vals[qual_index].is_valid.arr:%08x %08x \n\r"), db_id, SOC_PPC_FP_QUAL_TYPE_to_string(qual_type), copied_val, copied_maks,
                                      qual_vals[qual_index].val.arr[1],qual_vals[qual_index].val.arr[0],
                                      qual_vals[qual_index].is_valid.arr[1], qual_vals[qual_index].is_valid.arr[0]));
                }
                else if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_BUFFER_TO_KEY) {
                    SHR_BITCOPY_RANGE(&copied_val, 0, value, buf_msb-nof_copy_bits, nof_copy_bits);
                    SHR_BITCOPY_RANGE(&copied_maks, 0, mask, buf_msb-nof_copy_bits, nof_copy_bits);
                    LOG_DEBUG(BSL_LS_SOC_FP,
                              (BSL_META_U(unit,
                                          "   db_id %d, "
                                          "qual:%s, "
                                          "copied val:%08x, "
                                          "copied mask:%08x \n\r"
                                          "value[buf_msb - nof_copy_bits %d]:%08x"
                                          "mask[buf_msb - nof_copy_bits %d]:%08x \n\r"), db_id, SOC_PPC_FP_QUAL_TYPE_to_string(sw_db_ce.qual_type), copied_val, copied_maks,
                                      buf_msb-nof_copy_bits, value[nof_copy_bits],
                                      buf_msb-nof_copy_bits, mask[nof_copy_bits]));
                }

            }
            /* write from qual to value/mask buffer */
            if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_KEY_TO_BUFFER) {
                SHR_BITCOPY_RANGE(value, buf_msb-nof_copy_bits, qual_vals[qual_index].val.arr, sw_db_ce.qual_lsb, nof_copy_bits);
                SHR_BITCOPY_RANGE(mask, buf_msb-nof_copy_bits, qual_vals[qual_index].is_valid.arr, sw_db_ce.qual_lsb, nof_copy_bits);
            }
            else if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_BUFFER_TO_KEY) {
                /* Opposite: write from the buffer to the key and mask*/
                SHR_BITCOPY_RANGE(qual_vals[qual_index].val.arr, sw_db_ce.qual_lsb, value, buf_msb-nof_copy_bits, nof_copy_bits);
                SHR_BITCOPY_RANGE(qual_vals[qual_index].is_valid.arr, sw_db_ce.qual_lsb, mask, buf_msb-nof_copy_bits, nof_copy_bits);
                qual_vals[qual_index].type = sw_db_ce.qual_type;
            }
            buf_msb -= nof_copy_bits;
            /* skip junk */
            buf_msb -= sw_db_ce.lsb;
            /* hanlded from qualifiers */
            qual_lsb[qual_index] += nof_copy_bits;

            /* Align the nof_lsb to buf_msb until a MSB CE is allocated */
            if (moved_to_msb == 0) {
                nof_lsb[key_ndx] = (ARAD_PP_FP_TCAM_ENTRY_SIZE*32) - buf_msb - first_key_size;
            }
        } /* for ce */

        /* 
         *  For the Key to buffer direction, set the final buffer from the local
         *  one: copy the LSB part at LSB and the MSB part at MSB
         */ 
        /* now that we done, shift bits down */
        if (direction == ARAD_PP_FP_KEY_BUFFER_DIRECTION_KEY_TO_BUFFER) {
            /* check there is something to copy, otherwise zeros is ok.*/
            if(buf_msb != ARAD_PP_FP_TCAM_ENTRY_SIZE*32) {
                 
                /* copy lsb */
                if(moved_to_msb) {
                    nof_msb[key_ndx] = (ARAD_PP_FP_TCAM_ENTRY_SIZE*32-buf_msb) - nof_lsb[key_ndx] - first_key_size;
                }
                else{
                    nof_msb[key_ndx] = 0;
                }

                /* In KBP, need to stick to MSB (159:X) because the LSBs are not transmitted in the master key */
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
                if (is_for_kbp) {
                    uint32
                        frwrd_table_id, table_size_in_bytes, table_payload_in_bytes, table_size_in_bits;

                    SOC_PPC_FP_DATABASE_INFO fp_database_info;

                    if (is_from_dbal) {
                         arad_pp_dbal_db_predfix_get(unit, db_id, &frwrd_table_id);
                    } else {
                        res = arad_pp_fp_database_get_unsafe(unit, db_id, &fp_database_info);
                        SOC_SAND_CHECK_FUNC_RESULT(res, 25, exit);

                        if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) {
                            frwrd_table_id = fp_database_info.internal_table_id; /* the table ID is updated in the BCM layer according to the table in the KBP*/
                        } else {
                            frwrd_table_id = (ARAD_KBP_ACL_TABLE_ID_OFFSET + db_id);
                        }
                    }

                    res = arad_kbp_table_size_get(unit, frwrd_table_id, &table_size_in_bytes, &table_payload_in_bytes); /* For ACL, entry-size = table-size */
                    SOC_SAND_CHECK_FUNC_RESULT(res,  71, exit);

                    table_size_in_bits = table_size_in_bytes * 8;

                    /* Round up to bytes */
                    nof_lsb[key_ndx] = SOC_SAND_ALIGN_UP(nof_lsb[key_ndx], 8);
                    flp_lsb = table_size_in_bits - nof_lsb[key_ndx];

                }
#endif

                if(nof_lsb[key_ndx] > 0) 
                {
                    msb_padding_bits = (((nof_msb[key_ndx] + 7)/8)*8)-nof_msb[key_ndx];
                    LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META("msb_padding_bits=%d\n"),msb_padding_bits));

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
                    if (!is_for_kbp) {
                        flp_lsb = 0;
                        msb_padding_bits = 0;
                    }
#else
                    flp_lsb = 0;
                    msb_padding_bits = 0;
#endif
                    LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META("key_ndx=%d, flp_lsb=%d, buf_msb=%d, nof_msb[key_ndx]=%d, nof_lsb[key_ndx]=%d\n"),
                             key_ndx, flp_lsb, buf_msb, nof_msb[key_ndx], nof_lsb[key_ndx]));

                    SHR_BITCOPY_RANGE(mask_out,(160 * key_ndx) + flp_lsb, mask, buf_msb+nof_msb[key_ndx], nof_lsb[key_ndx]);
                    SHR_BITCOPY_RANGE(value_out,(160 * key_ndx) + flp_lsb, value, buf_msb+nof_msb[key_ndx], nof_lsb[key_ndx]);
                    LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"LSB  Key-->Buffer: (160 * key_ndx) + flp_lsb %d  buf_msb+nof_msb[key_ndx]:%d, key_ndx %d, nof_lsb[key_ndx] %d val %X mask %X\n\r"),
                            (160 * key_ndx) + flp_lsb, buf_msb+nof_msb[key_ndx], key_ndx, nof_lsb[key_ndx], value_out[(160 * key_ndx) + flp_lsb], mask_out[(160 * key_ndx) + flp_lsb]));
                }

                /* there is lsb so skip it */
                /* Key of 80b only in high not allowed as long as only TCAM Databases */
                if((is_for_kbp && SOC_IS_JERICHO(unit))){
                    skip_lsb = flp_lsb - nof_msb[key_ndx];
				}else{
                	skip_lsb = 80;
				}

                LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META("moved_to_msb=%d, key_ndx=%d, skip_lsb=%d, buf_msb=%d, nof_msb[key_ndx]=%d, msb_padding_bits=%d\n"),
                         moved_to_msb, key_ndx, skip_lsb, buf_msb, nof_msb[key_ndx], msb_padding_bits));

                if(moved_to_msb) {
                    SHR_BITCOPY_RANGE(mask_out, skip_lsb + (160 * key_ndx)- msb_padding_bits, mask, buf_msb, nof_msb[key_ndx]);
                    SHR_BITCOPY_RANGE(value_out, skip_lsb + (160 * key_ndx)- msb_padding_bits, value, buf_msb, nof_msb[key_ndx]);
                }
            }

#ifdef ARAD_PP_FP_DEBUG_PRINTS
            {
                uint32 print_idx;                
                char string_to_print[20*ARAD_PP_FP_TCAM_ENTRY_SIZE + 10] = "";

                LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"   ""Buffer after shifting:\n\r")));

                for(print_idx = 0; print_idx < ARAD_PP_FP_TCAM_ENTRY_SIZE; ++print_idx) {
                    sal_sprintf(string_to_print + sal_strlen(string_to_print), "%08x:", value_out[ARAD_PP_FP_TCAM_ENTRY_SIZE-print_idx-1]);
                }

                LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"    ""%s\n\r"), string_to_print));

                sal_sprintf(string_to_print, "    ");

                for(print_idx = 0; print_idx < ARAD_PP_FP_TCAM_ENTRY_SIZE; ++print_idx) {
                    sal_sprintf(string_to_print + sal_strlen(string_to_print), "%08x:", mask_out[ARAD_PP_FP_TCAM_ENTRY_SIZE-print_idx-1]);
                }
                LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"%s\n\r"), string_to_print));
            }
#endif
        } 
    } /* for key */

    /* for each qual/CEs parse qual value overed used bits in the CE */
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_value_buffer_mapping()", db_id, 0);
}


/*
 * For a specific Database, retrieve the qualifiers types and values 
 * from a buffer (the final HW Key) 
 */
uint32
  arad_pp_fp_key_buffer_to_value(
      SOC_SAND_IN  int                      unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN  uint32                   db_id,
      SOC_SAND_IN  uint32                   value_in[ARAD_PP_FP_TCAM_ENTRY_SIZE],
      SOC_SAND_IN  uint32                   mask_in[ARAD_PP_FP_TCAM_ENTRY_SIZE],
      SOC_SAND_OUT  SOC_PPC_FP_QUAL_VAL     qual_vals_out[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX]
  )
{
    uint32
      res = SOC_SAND_OK;
    uint32        
        qual_val_ndx,           
        value_in_lcl[ARAD_PP_FP_TCAM_ENTRY_SIZE],
        mask_in_lcl[ARAD_PP_FP_TCAM_ENTRY_SIZE];
    uint32 is_for_kbp = FALSE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Init the Qualifier out */
    for(qual_val_ndx = 0; qual_val_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++qual_val_ndx) {
        SOC_PPC_FP_QUAL_VAL_clear(&qual_vals_out[qual_val_ndx]);
    }


    sal_memcpy(value_in_lcl, value_in, ARAD_PP_FP_TCAM_ENTRY_SIZE * sizeof(uint32));
    sal_memcpy(mask_in_lcl, mask_in, ARAD_PP_FP_TCAM_ENTRY_SIZE * sizeof(uint32));

    /* in this case the KBP uses the mapping so extra functionality is needed */
    if(stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
        is_for_kbp = TRUE;

    }

    res = arad_pp_fp_key_value_buffer_mapping(
            unit,
            stage,
            db_id,
            ARAD_PP_FP_KEY_BUFFER_DIRECTION_BUFFER_TO_KEY,
            is_for_kbp,
            0, 
            qual_vals_out,
            value_in_lcl, 
            mask_in_lcl
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_buffer_to_value()", db_id, 0);
}



/*
 * given qual values of DB, 
 * build TCAM value and mask 
 */
uint32
  arad_pp_fp_key_value_to_buffer(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE stage,
      SOC_SAND_IN  SOC_PPC_FP_QUAL_VAL    qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
      SOC_SAND_IN  uint32                 db_id,
      SOC_SAND_OUT  uint32                value_out[ARAD_PP_FP_TCAM_ENTRY_SIZE],
      SOC_SAND_OUT  uint32                mask_out[ARAD_PP_FP_TCAM_ENTRY_SIZE]
  )
{
    uint32
      res = SOC_SAND_OK;
    SOC_PPC_FP_QUAL_VAL      
        qual_vals_lcl[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint8 is_for_kbp = FALSE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Init */
    sal_memset(value_out,0x0, sizeof(uint32) * ARAD_PP_FP_TCAM_ENTRY_SIZE);
    sal_memset(mask_out,0x0, sizeof(uint32) * ARAD_PP_FP_TCAM_ENTRY_SIZE);
    sal_memcpy(qual_vals_lcl, qual_vals, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX * sizeof(SOC_PPC_FP_QUAL_VAL));


    /* in this case the KBP uses the mapping so extra functionality is needed */
    if(stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
        is_for_kbp = TRUE;

    }

    res = arad_pp_fp_key_value_buffer_mapping(
            unit,
            stage,
            db_id,
            ARAD_PP_FP_KEY_BUFFER_DIRECTION_KEY_TO_BUFFER,
            is_for_kbp,
            0, 
            qual_vals_lcl,
            value_out, 
            mask_out
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_value_to_buffer()", db_id, 0);
}


/* Get qualifier length */
uint32
  arad_pp_fp_qual_length_get(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE     stage,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_TYPE       qual_type,
    SOC_SAND_OUT uint32                     *found,
    SOC_SAND_OUT uint32                     *length_padded_best_case,
    SOC_SAND_OUT uint32                     *length_padded_worst_case,
    SOC_SAND_OUT uint32                     *length_logical
  )
{
  uint32
    res;
  ARAD_PP_FP_QUAL_CE_INFO            
      ce_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(length_padded_best_case);
  SOC_SAND_CHECK_NULL_INPUT(length_padded_worst_case);
  SOC_SAND_CHECK_NULL_INPUT(length_logical);
  SOC_SAND_CHECK_NULL_INPUT(found);

  if (SOC_IS_ARADPLUS(unit))
  {  
      /* No need to allocate CEs for IsEqual qualifier */
      if(qual_type == SOC_PPC_FP_QUAL_IS_EQUAL)
      {
          *found = TRUE;
          *length_logical = 4;
          *length_padded_best_case = 4;
          *length_padded_worst_case = 4;
          ARAD_DO_NOTHING_AND_EXIT;
      }
  }
  
  /* Find this qualifier in the non-predfined qualifiers */
  res = arad_pp_fp_qual_ce_info_get(
            unit,
            stage,
            qual_type,
            &ce_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  /* See if this qualifier exists */
  if (ce_info.is_lsb || ce_info.is_msb)
  {
      *found = TRUE;
      *length_logical = ce_info.qual_size;
      *length_padded_best_case = ce_info.lost_bits + ce_info.qual_size;
      *length_padded_worst_case = ce_info.lost_bits_worst + ce_info.qual_size;
      ARAD_DO_NOTHING_AND_EXIT;
 }

    *found = FALSE;
    *length_logical = 0;
    *length_padded_best_case = 0;
    *length_padded_worst_case = 0;
    
    ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_length_get()", 0, 0);
}



/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_fp_key module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_fp_key_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_fp_key;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_fp_key module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_fp_key_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_fp_key;
}

void
  ARAD_PP_FP_CE_CONSTRAINT_clear(
    SOC_SAND_OUT ARAD_PP_FP_CE_CONSTRAINT *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_PP_FP_CE_CONSTRAINT));
  info->cycle_cons = ARAD_PP_FP_KEY_CYCLE_ANY;
  info->place_cons = ARAD_PP_FP_KEY_CE_PLACE_ANY;
  info->size_cons = ARAD_PP_FP_KEY_CE_SIZE_ANY;
  info->group_cons = ARAD_PP_FP_KEY_CE_PLACE_ANY;
  info->is_second_key = FALSE;
  info->lost_bits = 0;
  info->qual_lsb = 0;
  info->qual_type = BCM_FIELD_ENTRY_INVALID;
  
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
void
  ARAD_PP_FP_KEY_ALLOC_INFO_clear(
    SOC_SAND_OUT ARAD_PP_FP_KEY_ALLOC_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_PP_FP_KEY_ALLOC_INFO));
  info->key_size =  ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS;

exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void
  ARAD_PP_FP_KEY_DESC_clear(
    SOC_SAND_OUT ARAD_PP_FP_KEY_DESC *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_PP_FP_KEY_DESC));

exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* test */


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#undef _ERR_MSG_MODULE_NAME

#endif /* of #if defined(BCM_88650_A0) */
