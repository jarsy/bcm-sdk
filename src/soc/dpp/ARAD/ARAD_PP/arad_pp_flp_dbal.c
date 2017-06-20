/*
 * $Id: dpp_dbal.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MANAGEMENT

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_dbal.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/ARAD/arad_kbp.h>
#endif

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_entry_mgmt.h>
#endif


/********* FUNCTIONS *********/



#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
/* This function allocates prefixes to the tables in KAPS
   These prefixes are assigned dynamically in order to maximize the IPV4 UC capacity
   New dynamic tables support template is added in two places, demonstrated with SOC_DPP_DBAL_SW_TABLE_ID_NEW_TABLE_KAPS*/
STATIC uint32
arad_pp_flp_dbal_kaps_table_prefix_get(int unit, SOC_DPP_DBAL_SW_TABLE_IDS table_id, uint32 *table_prefix, uint32 *table_prefix_len)
{
    uint32 num_of_dynamic_tables = 0;
    uint32 dynamic_tables_additional_bits = 0;
    uint32 dynamic_table_prefix = 0; /*unique dynamic table prefix*/

    SOCDNX_INIT_FUNC_DEFS

    /* Increase num_of_dynamic_tables for each additional table */
    if (SOC_DPP_CONFIG(unit)->pp.fcoe_enable) {
        num_of_dynamic_tables++;
        num_of_dynamic_tables++; /* we have 2 tables for FCoE in LPM*/
    } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - kaps table_id is disabled")));
    }

    if (SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode == BCM_IPMC_SSM_KAPS_LPM) {
        num_of_dynamic_tables++;
    } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - kaps table_id is disabled")));
    }

    if (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) {
        num_of_dynamic_tables++;
        num_of_dynamic_tables++; /* we have 2 tables for FIB scale */
    } else if ((table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_SHORT_KAPS) || (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LONG_KAPS)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - kaps table_id is disabled")));
    }

    if ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) {
        num_of_dynamic_tables++;
        num_of_dynamic_tables++; /* we have 2 tables for IPv6 FIB scale */
    } else if ((table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS) || (table_id == SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_KAPS)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - kaps table_id is disabled")));
    }

    if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit)) {
        num_of_dynamic_tables++;
    } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - kaps table_id is disabled")));
    }

    /*
    if (new_table_enable_condition) {
        num_of_dynamic_tables++;
    } else if (table_id == SOC_DPP_DBAL_SW_TABLE_ID_NEW_TABLE_KAPS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - kaps table_id is disabled")));
    }
    */

    while (num_of_dynamic_tables > 0) {
        dynamic_tables_additional_bits++;
        num_of_dynamic_tables = num_of_dynamic_tables/2;
    }

    switch (table_id) {
    /* Static KAPS table prefix assignment */
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS:
        *table_prefix = JER_KAPS_IPV4_MC_TABLE_PREFIX;
        *table_prefix_len = JER_KAPS_TABLE_PREFIX_LENGTH;
        break;
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE:
        *table_prefix = JER_KAPS_IPV6_UC_TABLE_PREFIX;
        *table_prefix_len = JER_KAPS_TABLE_PREFIX_LENGTH;
        break;
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS:
        *table_prefix = JER_KAPS_IPV6_UC_TABLE_PREFIX;
        *table_prefix_len = JER_KAPS_TABLE_PREFIX_LENGTH;
        break;
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC:
        *table_prefix = JER_KAPS_IPV6_MC_TABLE_PREFIX;
        *table_prefix_len = JER_KAPS_TABLE_PREFIX_LENGTH;
        break;
    /* Dynamic KAPS table prefix assignment */
    /* Update the dynamic_table_prefix based on the allocated dynamic tables, do not add breaks between the cases */
    case SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS:
        if (SOC_DPP_CONFIG(unit)->pp.fcoe_enable) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS:
        if (SOC_DPP_CONFIG(unit)->pp.fcoe_enable) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID:
        if (SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode == BCM_IPMC_SSM_KAPS_LPM) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_SHORT_KAPS:
        if (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LONG_KAPS:
        if (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS:
        if ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_KAPS:
        if ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) {
            dynamic_table_prefix++;
        }
    case SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS:
        if (SOC_DPP_L3_SRC_BIND_IPV4_SUBNET_OR_ARP_ENABLE(unit)) {
            dynamic_table_prefix++;
        }
    /*
    case SOC_DPP_DBAL_SW_TABLE_ID_NEW_TABLE_KAPS:
        if (new_table_enable_condition) {
            dynamic_table_prefix++;
        }
    */
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS:     /* Always dynamic_table_prefix = 0 */
    case SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS: /* Always dynamic_table_prefix = 0 */
        if (dynamic_table_prefix > JER_KAPS_DYNAMIC_TABLE_PREFIX_MAX_NUM - 1) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - number of dynamic table prefixes exceeds the max")));
        }
        *table_prefix = (JER_KAPS_IPV4_UC_AND_NON_IP_TABLE_PREFIX << dynamic_tables_additional_bits) + dynamic_table_prefix;
        *table_prefix_len = 2 + dynamic_tables_additional_bits;
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_kaps_table_prefix_get - invalid kaps table_id")));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * This function returns whether or not the table_id is a dynamic KAPS table
 */
STATIC uint32
arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(int unit, SOC_DPP_DBAL_SW_TABLE_IDS table_id, uint8 *is_dynamic)
{
    uint32 table_prefix;
    uint32 table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, table_id, &table_prefix, &table_prefix_len));

    if (table_prefix_len == JER_KAPS_TABLE_PREFIX_LENGTH) {
        *is_dynamic = 0;
    } else {
        *is_dynamic = 1;
    }

exit:
    SOCDNX_FUNC_RETURN;
}
#endif



uint32 
    arad_pp_flp_dbal_source_lookup_with_aget_access_enable(int unit, int prog_id)
{
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
      flp_lookups_tbl;

    SOCDNX_INIT_FUNC_DEFS;    

    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl));

    flp_lookups_tbl.lem_1st_lkp_key_type   = 1;

    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl));
exit:
     SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_oam_statistics_program_tables_init(
     int unit,
	 int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;

    /* creating the table that related to the program */
    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_LIF;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_OAM_OPCODE;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_OAM_MD_LEVEL;


    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_OAM_STATISTICS, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 3, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP oam statistics LEM"));

    /* associating the tables to the program */
    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_OAM_STATISTICS;

    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, &keys_to_table_id, NULL, 1));

    /* updating extra look configuration for the program */  
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));  

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_oam_down_untagged_statistics_program_tables_init(
     int unit,
	 int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;


    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD; 
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_OAM_MD_LEVEL_UNTAGGED;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_OAM_DOWN_UNTAGGED_STATISTICS, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 2, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP OAM down untagged statistics LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_OAM_DOWN_UNTAGGED_STATISTICS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,&keys_to_table_id, NULL, 1));  

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));  

exit:
    SOCDNX_FUNC_RETURN;
}



uint32
arad_pp_flp_dbal_bfd_statistics_program_tables_init(
    int unit,
    int prog_id
    ) {
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = { 0 };
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 user_header_0_size;
    uint32 user_header_1_size;
    uint32 user_header_egress_pmf_offset_0;
    uint32 user_header_egress_pmf_offset_1; 
    

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_MY_DISCR_IPV4;

    SOCDNX_IF_ERR_EXIT(arad_pmf_db_fes_user_header_sizes_get(
                unit,
                &user_header_0_size,
                &user_header_1_size,
                &user_header_egress_pmf_offset_0,
                &user_header_egress_pmf_offset_1
              ));
   

    qual_info[0].ignore_qual_offset_for_entry_mngmnt = 1;
    qual_info[0].qual_offset = user_header_0_size + user_header_1_size;


    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_BFD_STATISTICS, ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_STATISTICS, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 1, ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_STATISTICS, qual_info, "FLP BFD statistics LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_BFD_STATISTICS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, &keys_to_table_id, NULL, 1));


    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));

exit:
    SOCDNX_FUNC_RETURN
}

uint32
    arad_pp_flp_dbal_oam_single_tag_statistics_program_tables_init(   
     int unit,
	 int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;
    
    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD; 
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_OAM_MD_LEVEL_SINGLE_TAG;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_TM_OUTER_TAG;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_OAM_SINGLE_TAG_STATISTICS, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 3, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP OAM sinngle tag statistics LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_OAM_SINGLE_TAG_STATISTICS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, &keys_to_table_id, NULL, 1));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));
    
exit:
    SOCDNX_FUNC_RETURN;
}


uint32
    arad_pp_flp_dbal_oam_double_tag_statistics_program_tables_init(
     int unit,
	 int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_ITMH_DEST_FWD; 
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_OAM_MD_LEVEL_DOUBLE_TAG;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_TM_OUTER_TAG;
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_TM_INNER_TAG;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_OAM_DOUBLE_TAG_STATISTICS, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP OAM double tag statistics LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_OAM_DOUBLE_TAG_STATISTICS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,&keys_to_table_id, NULL, 1));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));
  
exit:
    SOCDNX_FUNC_RETURN;
}



uint32
   arad_pp_flp_dbal_bfd_mpls_statistics_program_tables_init(
     int unit,
	 int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_MY_DISCR_MPLS;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_BFD_MPLS_STATISTICS, ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_STATISTICS, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 1, ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_STATISTICS, qual_info, "FLP BFD MPLS statistics LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_BFD_MPLS_STATISTICS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,&keys_to_table_id, NULL, 1));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_bfd_pwe_statistics_program_tables_init(
     int unit,
	 int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_MY_DISCR_PWE;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_BFD_PWE_STATISTICS, ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_STATISTICS, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 1, ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_STATISTICS, qual_info, "FLP BFD PWE statistics LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id.lookup_number = 1;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_BFD_PWE_STATISTICS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, &keys_to_table_id, NULL, 1));  

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEM_2ND));
  
exit:
    SOCDNX_FUNC_RETURN;
}

/*bfd echo - clasification in the LEM*/
soc_error_t
   arad_pp_flp_dbal_bfd_echo_program_tables_init(
     int unit
   )
{
    
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id = {0};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    
    
  /*  qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_DEST_PORT;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_TTL; 
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_NEXT_PRTCL;
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_DIP; */
    

    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_DEST_PORT;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_DIP;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_TTL; 
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_IPV4_NEXT_PRTCL;

    
   
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_BFD_ECHO_LEM, ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_BFD_ECHO, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_BFD_ECHO, qual_info, "FLP BFD ECHO LEM"));

    keys_to_table_id.key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id.lookup_number = 2;
    keys_to_table_id.sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_BFD_ECHO_LEM;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV4UC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, &keys_to_table_id, NULL, 1));

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
    arad_pp_dbal_flp_ethernet_ing_ivl_learn_tables_create(
        int unit,
        int prog_id)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};    

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_info[0].qual_offset = 16;
    qual_info[0].qual_nof_bits = 32;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_info[1].qual_offset = 0;
    qual_info[1].qual_nof_bits = 16;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
    qual_info[2].qual_offset = 0;
    qual_info[2].qual_nof_bits = 15;
/*  qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG_ID;*/
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_VLAN_EDIT_CMD_VID1;
    qual_info[3].qual_offset = 0;
    qual_info[3].qual_nof_bits = 12;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IVL_LEARN_LEM, (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)), ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN, qual_info, "FLP L2 Learn DB, IVL Learn Mode LEM"));

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_info[0].qual_offset = 16;
    qual_info[0].qual_nof_bits = 32;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_info[1].qual_offset = 0;
    qual_info[1].qual_nof_bits = 16;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
    qual_info[2].qual_offset = 0;
    qual_info[2].qual_nof_bits = 15;
/*  qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG_ID;*/
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_VLAN_EDIT_CMD_VID1;
    qual_info[3].qual_offset = 0;
    qual_info[3].qual_nof_bits = 12;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_LEM, (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)), 4,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN, qual_info, "FLP L2 Fwd DB,IVL Learn Mode"));

   /* Associate Key construction for IVL Programs */ 
    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IVL_LEARN_LEM;
    keys_to_table_id[0].lookup_number = 1;
    
    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_LEM;
    keys_to_table_id[1].lookup_number = 2;
    
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                   keys_to_table_id, NULL, 2));

    /* update the IVL MACT program lookup LEM 1st lookup type  (LEM_1ST_LKP_KEY_TYPE) */
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_source_lookup_with_aget_access_enable(unit, prog_id));
exit:
     SOCDNX_FUNC_RETURN;    
}

uint32
    arad_pp_dbal_flp_ethernet_ing_ivl_inner_learn_tables_create(
        int unit,
        int prog_id)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};    

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_info[0].qual_offset = 16;
    qual_info[0].qual_nof_bits = 32;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_info[1].qual_offset = 0;
    qual_info[1].qual_nof_bits = 16;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
    qual_info[2].qual_offset = 0;
    qual_info[2].qual_nof_bits = 15;
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_INNERMOST_VLAN_TAG_ID;
    qual_info[3].qual_offset = 0;
    qual_info[3].qual_nof_bits = 12;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IVL_INNER_LEARN_LEM, (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)), ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN, qual_info, "FLP L2 Learn DB, IVL INNER VLAN-TAG LEM Learn Mode"));

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_info[0].qual_offset = 16;
    qual_info[0].qual_nof_bits = 32;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_info[1].qual_offset = 0;
    qual_info[1].qual_nof_bits = 16;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
    qual_info[2].qual_offset = 0;
    qual_info[2].qual_nof_bits = 15;
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_INNERMOST_VLAN_TAG_ID;
    qual_info[3].qual_offset = 0;
    qual_info[3].qual_nof_bits = 12;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IVL_INNER_FWD_LEM, (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)), ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN, qual_info, "FLP L2 Fwd DB, IVL INNER VLAN-TAG LEM Learn Mode"));

   /* Associate Key construction for IVL Programs */ 
    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IVL_INNER_LEARN_LEM;
    keys_to_table_id[0].lookup_number = 1;
    
    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IVL_INNER_FWD_LEM;
    keys_to_table_id[1].lookup_number = 2;
    
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                   keys_to_table_id, NULL, 2));

    /* update the IVL MACT program lookup LEM 1st lookup type  (LEM_1ST_LKP_KEY_TYPE) */
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_source_lookup_with_aget_access_enable(unit, prog_id));


exit:
     SOCDNX_FUNC_RETURN;    
}


uint32
    arad_pp_dbal_flp_ethernet_ing_ivl_fwd_outer_learn_tables_create(
        int unit,
        int prog_id)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};    

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_info[0].qual_offset = 16;
    qual_info[0].qual_nof_bits = 32;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_info[1].qual_offset = 0;
    qual_info[1].qual_nof_bits = 16;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
    qual_info[2].qual_offset = 0;
    qual_info[2].qual_nof_bits = 15;
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG_ID;
    qual_info[3].qual_offset = 0;
    qual_info[3].qual_nof_bits = 12;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_OUTER_LEARN_LEM, (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)), ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN, qual_info, "FLP L2 Fwd DB, IVL FWD OUTER VLAN-TAG LEM Learn Mode"));

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
    qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_info[0].qual_offset = 16;
    qual_info[0].qual_nof_bits = 32;
    qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_info[1].qual_offset = 0;
    qual_info[1].qual_nof_bits = 16;
    qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
    qual_info[2].qual_offset = 0;
    qual_info[2].qual_nof_bits = 15;
    qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG_ID;
    qual_info[3].qual_offset = 0;
    qual_info[3].qual_nof_bits = 12;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_OUTER_FWD_LEM, (ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IVL_LEARN_LEM(unit)), 4,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_IVL_LEARN, qual_info, "FLP L2 Fwd DB, IVL FWD OUTER VLAN-TAG LEM Fwd Mode"));

   /* Associate Key construction for IVL Programs */ 
    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_OUTER_LEARN_LEM;
    keys_to_table_id[0].lookup_number = 1;
    
    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_OUTER_FWD_LEM;
    keys_to_table_id[1].lookup_number = 2;
    
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                   keys_to_table_id, NULL, 2));

    /* update the IVL MACT program lookup LEM 1st lookup type  (LEM_1ST_LKP_KEY_TYPE) */
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_source_lookup_with_aget_access_enable(unit, prog_id));
exit:
     SOCDNX_FUNC_RETURN;    
}

uint32 
    arad_pp_flp_dbal_ipv4uc_lem_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM, ARAD_PP_FLP_IPV4_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 2, ARAD_PP_LEM_ACCESS_KEY_TYPE_IP_HOST, qual_info, "FLP IPv4 UC LEM"));

    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_pon_ipv4_sav_lem_table_create(int unit, uint8 is_not_arp, uint8 is_arp_up)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = (is_not_arp ? SOC_PPC_FP_QUAL_HDR_IPV4_SIP : (is_arp_up ? SOC_PPC_FP_QUAL_ARP_SENDER_IP4 : SOC_PPC_FP_QUAL_ARP_TARGET_IP4));
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_FID;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_LEM, ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 2, ARAD_PP_FLP_IPV4_SPOOF_STATIC_KEY_OR_MASK, qual_info, "FLP IPv4 SAV STATIC LEM"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32 
    arad_pp_flp_dbal_ipv4uc_lem_custom_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_CUSTOM_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);             
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_SA;
        qual_info[0].qual_offset = 16;
        qual_info[0].qual_nof_bits = 32;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_SA;
        qual_info[1].qual_offset = 0;
        qual_info[1].qual_nof_bits = 16;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_VLAN_TAG;
        qual_info[2].qual_offset = 4;
        qual_info[2].qual_nof_bits = 12;  
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT;
        qual_info[3].qual_offset = 0;
        qual_info[3].qual_nof_bits = 12;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_CUSTOM_LEM, ARAD_PP_FLP_MAC_IN_MAC_TUNNEL_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 4, ARAD_PP_LEM_ACCESS_KEY_TYPE_MAC_IN_MAC_TUNNEL, qual_info, "FLP IPv4 UC LEM"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv4uc_lem_route_scale_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* Need To break it into two qualifiers for nibble resolution in over 16bit qualifier */
        if ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) > 16    &&
            (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) % 8 > 0) {
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[qualifiers_counter].qual_offset = 16;
            qual_info[qualifiers_counter++].qual_nof_bits = (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) - 16;
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[qualifiers_counter++].qual_nof_bits = 16;
        } else {
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[qualifiers_counter++].qual_nof_bits = (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length);
        }
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LEM, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, qualifiers_counter, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv4 UC SCALE ROUTE LEM"));

    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv6uc_lem_route_scale_long_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* Each CE can be at the most 32 bits */
        if (((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) > 32) {
            /* Need To break it into two qualifiers for nibble resolution in over 16bit qualifier */
            if (((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) - 32 > 16    &&
                ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) % 8 > 0) {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 16 + 32;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) - 32 - 16;
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 32;
                qual_info[qualifiers_counter++].qual_nof_bits = 16;
            } else {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 32;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) - 32;
            }
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
            qual_info[qualifiers_counter++].qual_nof_bits = 32;
        } else {
            /* Need To break it into two qualifiers for nibble resolution in over 16bit qualifier */
            if (((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) > 16    &&
                ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) % 8 > 0) {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 16;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) - 16;
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter++].qual_nof_bits = 16;
            } else {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long));
            }
        }
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_LEM, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, qualifiers_counter, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv6 UC SCALE ROUTE LEM LONG"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv6uc_lem_route_scale_short_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* Each CE can be at the most 32 bits */
        if (((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) > 32) {
            /* Need To break it into two qualifiers for nibble resolution in over 16bit qualifier */
            if (((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) - 32 > 16    &&
                ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) % 8 > 0) {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 16 + 32;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) - 32 - 16;
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 32;
                qual_info[qualifiers_counter++].qual_nof_bits = 16;
            } else {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 32;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) - 32;
            }
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
            qual_info[qualifiers_counter++].qual_nof_bits = 32;
        } else {
            /* Need To break it into two qualifiers for nibble resolution in over 16bit qualifier */
            if (((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) > 16    &&
                ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) % 8 > 0) {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter].qual_offset = 16;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short)) - 16;
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter++].qual_nof_bits = 16;
            } else {
                qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
                qual_info[qualifiers_counter++].qual_nof_bits = ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_short));
            }
        }
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_LEM, DBAL_PREFIX_NOT_DEFINED, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, qualifiers_counter, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv6 UC SCALE ROUTE LEM SHORT"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv4mc_bridge_lem_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_DIP;
        qual_info[0].qual_offset = 16;
        qual_info[0].qual_nof_bits = 16;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_DIP;
        qual_info[1].qual_offset = 4;
        qual_info[1].qual_nof_bits = 12;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_LEM, ARAD_PP_FLP_IPV4_COMP_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 3, ARAD_PP_LEM_ACCESS_KEY_TYPE_IPV4_MC, qual_info, "FLP IPv4 MC LEM"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv4mc_Learning_lem_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_LEARN_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
        qual_info[0].qual_offset = 32;
        qual_info[0].qual_nof_bits = 16;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
        qual_info[1].qual_offset = 0;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
        qual_info[2].qual_offset = 0;
        qual_info[2].qual_nof_bits = 15;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_LEARN_LEM,
        ARAD_PP_FLP_ETH_KEY_OR_MASK(unit), ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
        SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 3, 0, qual_info, "FLP: IPV4 MC Learning"));
    }
exit:
    SOCDNX_FUNC_RETURN;
}


uint32
    arad_pp_flp_dbal_ipv4uc_rpf_lem_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_LEM, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_LEM, ARAD_PP_FLP_IPV4_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 2, 0, qual_info, "FLP IPv4 UC RPF LEM"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_ipv4uc_default_lem_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM_DEFAULT, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM_DEFAULT, ARAD_PP_FLP_IPV4_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 1, 0, qual_info, "FLP IPv4 UC default LEM "));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv4mc_bridge_tcam_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix_len = 0;

    SOCDNX_INIT_FUNC_DEFS;


    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID, &is_table_initiated));
    if (!is_table_initiated) {


        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* MSB */
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_SIP;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_DIP;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID, DBAL_PREFIX_NOT_DEFINED, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM, 3, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv4 MC FID TCAM"));
    }



exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv6uc_tcam_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;


    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, &is_table_initiated));
    if (!is_table_initiated) {

        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;  /* DIP 31:0 */
        qual_info[0].qual_offset = 32;
        qual_info[0].qual_nof_bits = 32;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;  /* DIP 63:32 */
        qual_info[1].qual_offset = 0;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;  /* DIP 79:64 */
        qual_info[2].qual_offset = 48;
        qual_info[2].qual_nof_bits = 16;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;  /* DIP 95:80 */
        qual_info[3].qual_offset = 32;
        qual_info[3].qual_nof_bits = 16;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;  /* DIP 127:96 */
        qual_info[4].qual_offset = 0;
        qual_info[4].qual_nof_bits = 32;
        qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[5].qual_offset = 0;
        qual_info[5].qual_nof_bits = 8;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE,
                DBAL_PREFIX_NOT_DEFINED, 0, SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM,6, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv6 UC TCAM"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

uint32 
    arad_pp_flp_dbal_fcoe_kaps_table_create(int unit, int is_vsan_from_vsi)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS, &table_prefix, &table_prefix_len));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS, &is_table_initiated));
    if (is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_destroy(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS));
    }        
        
    if (is_vsan_from_vsi) {                

        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;        
        qual_info[0].qual_nof_bits = 27 - table_prefix_len; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[2].qual_nof_bits = 8;
        qual_info[2].qual_offset = 24;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[3].qual_nof_bits = 1;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI;
        qual_info[4].qual_nof_bits = 12;
        
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 5, 0, qual_info, "FLP FCoE VSI KAPS"));
    } else{        
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;        
        qual_info[0].qual_nof_bits = 27 - table_prefix_len; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[2].qual_nof_bits = 8;
        qual_info[2].qual_offset = 24;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI;
        qual_info[3].qual_nof_bits = 13;
        
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 4, 0, qual_info, "FLP FCoE VFT KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_fcoe_npv_kaps_table_create(int unit, int is_vsan_from_vsi)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, &table_prefix, &table_prefix_len));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, &is_table_initiated));
    if (is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_destroy(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS));
    }        
        
    if (is_vsan_from_vsi) {                

        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;        
        qual_info[0].qual_nof_bits = 27 - table_prefix_len; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
        qual_info[2].qual_nof_bits = 8;
        qual_info[2].qual_offset = 24;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[3].qual_nof_bits = 1;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI;
        qual_info[4].qual_nof_bits = 12;
        
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 5, 0, qual_info, "FLP FCoE NPV VSI KAPS"));
    } else{        
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;        
        qual_info[0].qual_nof_bits = 27 - table_prefix_len; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
        qual_info[2].qual_nof_bits = 8;
        qual_info[2].qual_offset = 24;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI;
        qual_info[3].qual_nof_bits = 13;
        
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 4, 0, qual_info, "FLP FCoE NPV VFT KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32 
    arad_pp_flp_dbal_ipv4uc_kaps_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = table_prefix_len == JER_KAPS_TABLE_PREFIX_LENGTH ? 32 : 28; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        if (table_prefix_len != JER_KAPS_TABLE_PREFIX_LENGTH) { /* vrf + table_prefix_len need to be nibble aligned for public key generation */
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = JER_KAPS_DYNAMIC_TABLE_PREFIX_LENGTH - table_prefix_len;
        }
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, qualifiers_counter, 0, qual_info, "FLP IPv4 UC KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_pon_ipv4_sav_kaps_table_create(int unit, uint8 is_not_arp, uint8 is_arp_up)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[0].qual_nof_bits = (33 - table_prefix_len);
        qual_info[1].qual_type = (is_not_arp ? SOC_PPC_FP_QUAL_HDR_IPV4_SIP : (is_arp_up ? SOC_PPC_FP_QUAL_ARP_SENDER_IP4 : SOC_PPC_FP_QUAL_ARP_TARGET_IP4));;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 3, 0, qual_info, "FLP IPV4 SAV STATIC KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
    arad_pp_flp_dbal_ipv4uc_kaps_route_scale_long_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LONG_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LONG_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = table_prefix_len == JER_KAPS_TABLE_PREFIX_LENGTH ? 32 : 28; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        if (table_prefix_len != JER_KAPS_TABLE_PREFIX_LENGTH) { /* vrf + table_prefix_len need to be nibble aligned for public key generation */
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = JER_KAPS_DYNAMIC_TABLE_PREFIX_LENGTH - table_prefix_len;
        }
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LONG_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, qualifiers_counter, 0, qual_info, "FLP IPv4 UC SCALE LONG KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv4uc_kaps_route_scale_short_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len, qualifiers_counter = 0;
    uint32 dip_qual_size = (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length);

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_SHORT_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_SHORT_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        if (dip_qual_size < 28) {
            /* Two zero qualifier paddings are needed to reach 80bits alignment */
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = 80 - 20 /*VRF+TBL_ID*/ - 32/*ZERO*/ - dip_qual_size;
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = 32;
        } else {
            /* A single zero qualifier padding is needed to reach 80bits alignment */
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = 80 - 20 /*VRF+TBL_ID*/ - dip_qual_size;
        }
        /* Need To break it into two qualifiers for nibble resolution in over 16bit qualifier */
        if ((dip_qual_size > 16) && (dip_qual_size % 8 > 0)) {
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[qualifiers_counter].qual_offset = 16;
            qual_info[qualifiers_counter++].qual_nof_bits = dip_qual_size - 16;
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[qualifiers_counter++].qual_nof_bits = 16;
        } else {
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[qualifiers_counter++].qual_nof_bits = dip_qual_size;
        }
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        /* vrf + table_prefix_len need to be nibble aligned for public key generation */
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = JER_KAPS_DYNAMIC_TABLE_PREFIX_LENGTH - table_prefix_len;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_SHORT_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, qualifiers_counter, 0, qual_info, "FLP IPv4 UC SCALE SHORT KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv6uc_kaps_route_scale_long_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* LSB */
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[qualifiers_counter].qual_nof_bits = 32;
        qual_info[qualifiers_counter++].qual_offset = 32;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[qualifiers_counter++].qual_nof_bits = 32;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[qualifiers_counter].qual_nof_bits = 16;
        qual_info[qualifiers_counter++].qual_offset = 48;

        /* MSB */
        /* Need To break 28bits DIP into two qualifiers for nibble resolution in over 16bit qualifier */
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = 12;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[qualifiers_counter].qual_offset = 32;
        qual_info[qualifiers_counter++].qual_nof_bits = 16;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[qualifiers_counter++].qual_nof_bits = 32;
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        if (table_prefix_len != JER_KAPS_TABLE_PREFIX_LENGTH) { /* vrf + table_prefix_len need to be nibble aligned for public key generation */
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = JER_KAPS_DYNAMIC_TABLE_PREFIX_LENGTH - table_prefix_len;
        }
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, qualifiers_counter, 0, qual_info, "FLP IPv6 UC SCALE LONG KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv6uc_kaps_route_scale_short_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* Always use 56bits of IPv6 prefix, 6(TID) + 14(VRF) + 56(DIP) +4(ZEROS) = 80 */
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = 4;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[qualifiers_counter].qual_offset = 32;
        qual_info[qualifiers_counter++].qual_nof_bits = 24;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[qualifiers_counter++].qual_nof_bits = 32;
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = JER_KAPS_DYNAMIC_TABLE_PREFIX_LENGTH - table_prefix_len;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, qualifiers_counter, 0, qual_info, "FLP IPv6 UC SCALE SHORT KAPS"));
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_stage_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, 0/*lookup_number*/, 0/*program_id*/ ));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv4mc_bridge_kaps_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID, &is_table_initiated));
    if (!is_table_initiated) {

        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* MSB */
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[0].qual_nof_bits = 5 - table_prefix_len; /*Align to 80bits*/
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_SIP;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_DIP;
        qual_info[2].qual_offset = 16;
        qual_info[2].qual_nof_bits = 16;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_AFTER_FWD_IPV4_DIP;
        qual_info[3].qual_offset = 4;
        qual_info[3].qual_nof_bits = 12;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_FID;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 5, 0, qual_info, "FLP IPv4 MC FID KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32 
    arad_pp_flp_dbal_ipv4uc_rpf_kaps_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len, qualifiers_counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[qualifiers_counter++].qual_nof_bits = table_prefix_len == JER_KAPS_TABLE_PREFIX_LENGTH ? 32 : 28; /* Align to 80bits according to the dynamic prefix len*/
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
        qual_info[qualifiers_counter++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        if (table_prefix_len != JER_KAPS_TABLE_PREFIX_LENGTH) { /* vrf + table_prefix_len need to be nibble aligned for public key generation */
            qual_info[qualifiers_counter].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[qualifiers_counter++].qual_nof_bits = JER_KAPS_DYNAMIC_TABLE_PREFIX_LENGTH - table_prefix_len;
        }
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, qualifiers_counter, 0, qual_info, "FLP IPv4 UC RPF KAPS"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_ipv4uc_kbp_table_create(int unit, int is_rpf)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_MASTER_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;        
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;        
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
		qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[3].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_MASTER_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 4, SOC_DPP_DBAL_ATI_KBP_MASTER_KEY_INDICATION, qual_info, "FLP IPv4 UC MASTER KBP"));
    }

    /* this table is used for entry managemant and lookup indication */
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_FWD_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[2].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_FWD_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 3, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv4 UC FWD KBP"));
    }

    if (is_rpf) {
        /*this table is used for entry managemant and lookup indication */
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KBP, &is_table_initiated));
        if (!is_table_initiated) {
            DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
            qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
            qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
		    qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
			qual_info[2].qual_nof_bits = 2;
            SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1, 0,
                                                         SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 3, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv4 UC RPF KBP"));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_ipv6uc_kbp_table_create(int unit, int is_rpf)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_MASTER_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* key A LSB - 80b */
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[0].qual_nof_bits = 32;
        qual_info[0].qual_offset = 32;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[2].qual_nof_bits = 16;
        qual_info[2].qual_offset = 48;
        /* key A MSB - 80b */
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[3].qual_nof_bits = 16;
        qual_info[3].qual_offset = 32;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[4].qual_nof_bits = 32;
        qual_info[5].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[5].qual_nof_bits = 32;
        qual_info[5].qual_offset = 32;
        /* key B LSB - 80b */
        qual_info[6].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[6].qual_nof_bits = 32;
        qual_info[7].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[7].qual_nof_bits = 32;
        qual_info[7].qual_offset = 32;
        qual_info[8].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[8].qual_nof_bits = 16;
        qual_info[8].qual_offset = 16;
        /* key B MSB - 32b */
        qual_info[9].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[9].qual_nof_bits = 16;
        qual_info[10].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
		qual_info[11].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[11].qual_nof_bits = 2;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_MASTER_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 12, SOC_DPP_DBAL_ATI_KBP_MASTER_KEY_INDICATION, qual_info, "FLP IPv6 UC MASTER KBP"));
    }

    /* this table is used for entry managemant and lookup indication */
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_FWD_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[0].qual_nof_bits = 32;
        qual_info[0].qual_offset = 32;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[2].qual_nof_bits = 32;
        qual_info[2].qual_offset = 32;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[3].qual_nof_bits = 32;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
		qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[5].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_FWD_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 6, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv6 UC FWD KBP"));
    }

    if (is_rpf) {
        /*this table is used for entry managemant and lookup indication */
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KBP, &is_table_initiated));
        if (!is_table_initiated) {
            DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
            qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
            qual_info[0].qual_nof_bits = 32;
            qual_info[0].qual_offset = 32;
            qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
            qual_info[1].qual_nof_bits = 32;
            qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
            qual_info[2].qual_nof_bits = 32;
            qual_info[2].qual_offset = 32;
            qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
            qual_info[3].qual_nof_bits = 32;
            qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
            qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[5].qual_nof_bits = 2;
            SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1, 0,
                                                         SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 6, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv6 UC RPF KBP"));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_ipv4mc_kbp_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_FWD_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
		qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[1].qual_nof_bits = 1;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;        
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
		qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[5].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_FWD_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV4_MC, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 6, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv4 MC FWD KBP"));
    }

    /*this table is used for entry managemant and lookup indication */
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_DUMMY_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[1].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_DUMMY_KBP, ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 2, SOC_DPP_DBAL_ATI_KBP_DUMMY_TABLE, qual_info, "FLP IPv4 MC DUMMY KBP"));
    }

    /*this table is used for entry managemant and lookup indication */
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_RPF_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[2].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_RPF_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 3, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv4 MC RPF KBP"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_ipv6mc_kbp_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_FWD_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* key A LSB - 80b */
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
		qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[1].qual_nof_bits = 1;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[2].qual_nof_bits = 32;
        qual_info[2].qual_offset = 32;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[3].qual_nof_bits = 32;
        /* key A MSB - 80b */
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[4].qual_nof_bits = 32;
        qual_info[4].qual_offset = 32;
        qual_info[5].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[5].qual_nof_bits = 32;
        qual_info[6].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[6].qual_nof_bits = 16;
        qual_info[6].qual_offset = 48;
        /* key B LSB - 80b */
        qual_info[7].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[7].qual_nof_bits = 16;
        qual_info[7].qual_offset = 32;
        qual_info[8].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[8].qual_nof_bits = 32;
        qual_info[9].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[9].qual_nof_bits = 32;
        qual_info[9].qual_offset = 32;
        /* key B LSB - 48b */
        qual_info[10].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[10].qual_nof_bits = 32;
        qual_info[11].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
		qual_info[12].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
		qual_info[12].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_FWD_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV6_MC, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 13, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv6 MC FWD KBP"));
    }

    /*this table is used for entry managemant and lookup indication */
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_DUMMY_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[1].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_DUMMY_KBP, ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 2, SOC_DPP_DBAL_ATI_KBP_DUMMY_TABLE, qual_info, "FLP IPv6 MC DUMMY KBP"));
    }

    /*this table is used for entry managemant and lookup indication */
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_RPF_KBP, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[3].qual_nof_bits = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_RPF_KBP, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 4, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv6 MC RPF KBP"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32 
    arad_pp_flp_dbal_lsr_kbp_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP, &is_table_initiated));
    if(!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
        qual_info[0].qual_offset = 4;
        qual_info[0].qual_nof_bits = 16;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
        qual_info[1].qual_offset = 0;
        qual_info[1].qual_nof_bits = 4;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[2].qual_nof_bits = 28;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP, ARAD_KBP_FRWRD_TBL_ID_LSR, 0,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 3, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP LSR KBP"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

#endif


uint32 
    arad_pp_flp_dbal_ipv4mc_kaps_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0, nof_qual;
    uint32 table_prefix = DBAL_PREFIX_NOT_DEFINED, table_prefix_len = 0;



    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, &is_table_initiated));
    if (!is_table_initiated) {

        if (SOC_DPP_CONFIG(unit)->pp.l3_mc_use_tcam != ARAD_PP_FLP_L3_MC_USE_TCAM_DISABLE) {

            nof_qual = (SOC_DPP_CONFIG(unit)->pp.l3_mc_use_tcam == ARAD_PP_FLP_L3_MC_USE_TCAM_NO_IPV4_VRF) ? 4 : 5;

            qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[0].qual_nof_bits = 1;
            qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
            qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
            qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
            qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;

            SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, table_prefix, table_prefix_len,
                                                         SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM, nof_qual, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv4 MC TCAM"));
        }else{
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
            if(JER_KAPS_ENABLE(unit)){

                SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, &table_prefix, &table_prefix_len));        
                /* LSB */
                qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
                qual_info[0].qual_nof_bits = 4;
                qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
                qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
                qual_info[3].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
                qual_info[3].qual_nof_bits = 12;
             
                /* MSB */
                qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
                qual_info[4].qual_offset = 12;
                qual_info[4].qual_nof_bits = 3;
                qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
                qual_info[5].qual_nof_bits = 1;
                qual_info[6].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
                qual_info[7].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
                qual_info[7].qual_offset = 16;
                qual_info[7].qual_nof_bits = 16;
                qual_info[8].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
                qual_info[8].qual_offset = 4;
                qual_info[8].qual_nof_bits = 12;
                qual_info[9].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;

                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, table_prefix, table_prefix_len,
                                                         SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 10, 0, qual_info, "FLP IPv4 MC KAPS"));
            }
#endif
        }

    }

exit:
    SOCDNX_FUNC_RETURN;
}


#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32 
    arad_pp_flp_dbal_ipv6uc_kaps_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, &table_prefix, &table_prefix_len));
         DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* LSB */
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[0].qual_nof_bits = 16;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[1].qual_nof_bits = 32;
        qual_info[1].qual_offset = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
        qual_info[2].qual_nof_bits = 32;

        /* MSB */
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[3].qual_nof_bits = 32;
        qual_info[3].qual_offset = 32;
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
        qual_info[4].qual_nof_bits = 32;
        qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 6, 0, qual_info, "FLP IPv6 UC KAPS"));

    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
    arad_pp_flp_dbal_ipv6uc_rpf_kaps_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint32 table_prefix, table_prefix_len;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS, &table_prefix, &table_prefix_len));
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* LSB */
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
        qual_info[0].qual_nof_bits = 16;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[1].qual_nof_bits = 32;
        qual_info[1].qual_offset = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
        qual_info[2].qual_nof_bits = 32;

        /* MSB */
        qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[3].qual_nof_bits = 32;
        qual_info[3].qual_offset = 32;        
        qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
        qual_info[4].qual_nof_bits = 32;
        qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS, table_prefix, table_prefix_len,
                                                     SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 6, 0, qual_info, "FLP IPv6 UC RPF KAPS"));


    }

exit:
    SOCDNX_FUNC_RETURN;
}

#endif /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/

uint32 
    arad_pp_flp_dbal_ipv6mc_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, &is_table_initiated));
    if (!is_table_initiated) {

        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

        if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {

            qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
            qual_info[0].qual_nof_bits = 32;
            qual_info[0].qual_offset = 32;
            qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
            qual_info[1].qual_nof_bits = 32;
            qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
            qual_info[2].qual_nof_bits = 32;
            qual_info[2].qual_offset = 32;
            qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
            qual_info[3].qual_nof_bits = 32;
            qual_info[4].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;

            SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, DBAL_PREFIX_NOT_DEFINED, 0,
                                                                    SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM, 5, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv6 MC TCAM"));

        } else {
            /* LSB */
            qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES;
            qual_info[0].qual_nof_bits = 1;
            qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
            qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
            qual_info[2].qual_nof_bits = 32;
            qual_info[2].qual_offset = 32;
            qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
            qual_info[3].qual_nof_bits = 32;

            /* MSB */
            qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
            qual_info[4].qual_nof_bits = 32;
            qual_info[4].qual_offset = 32;
            qual_info[5].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
            qual_info[5].qual_nof_bits = 32;
            qual_info[6].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;


             if (SOC_DPP_CONFIG(unit)->pp.l3_mc_use_tcam != ARAD_PP_FLP_L3_MC_USE_TCAM_DISABLE) {

                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, DBAL_PREFIX_NOT_DEFINED, 0,
                                                             SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM, 7, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv6 MC TCAM"));
            } else {
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

                uint32 table_prefix = 0, table_prefix_len = 0;
                SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_get(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, &table_prefix, &table_prefix_len));

                SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC, table_prefix, table_prefix_len,
                                                             SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KAPS, 7, 0, qual_info, "FLP IPv6 MC KAPS"));
#endif
            }
        }

    }

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_fcoe_program_tables_init(int unit, int is_vsan_from_vsi, int fcoe_no_vft_prog_id, int fcoe_vft_prog_id)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    uint32 vsi_vft_instuction = 0, did_instuction = 0;
    SOC_DPP_DBAL_TABLE_INFO table;
    int i, vft_pos, no_vft_pos, vsi_vft_inst_id, dip_inst_id;
    soc_mem_t flp_key_construction_mem;
    uint32 data[ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE];
    uint32 data2[ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE];
    SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;

    SOCDNX_INIT_FUNC_DEFS;

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_fcoe_kaps_table_create(unit, is_vsan_from_vsi));
#endif

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B_MSB;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS;
    keys_to_table_id[0].lookup_number = 2;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, fcoe_vft_prog_id , SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,keys_to_table_id, NULL, 1));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, fcoe_no_vft_prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,keys_to_table_id, NULL, 1));

    /*W.A for FCoE: 
      we use qualifiers VSI and DIP when creating the table. but we change them to VFT and DID here...
      1. update the VSI instruction to VFT or VSI
      2. update the DIP instruction to DID
      we are updating only the MSB part of the key, kaps is usung key b MSB */
    
    soc_sand_os_memset(data, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));
    soc_sand_os_memset(data2, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_KAPS, &table));

    if (is_vsan_from_vsi == 0) {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_13_BITS;
    } else {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_VSI(12);
    }    
    
    if(table.table_programs[0].program_id == fcoe_no_vft_prog_id){
        no_vft_pos = 0;
        vft_pos = 1;
    }else{
        no_vft_pos = 1;
        vft_pos = 0;
    }

    /* no VFT PROGRAM */

    vsi_vft_inst_id = -1;
    dip_inst_id = -1;
    for (i = 0; i < table.nof_qualifiers; i++) {
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP) {
            dip_inst_id = table.table_programs[no_vft_pos].ce_assigned[i];
        }
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI) {
            vsi_vft_inst_id = table.table_programs[no_vft_pos].ce_assigned[i];
        }
    }

    if (vsi_vft_inst_id == -1 || dip_inst_id == -1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_fcoe_program_tables_init - instruction not found 1")));
    }
    
    flp_key_construction_mem = IHP_FLP_KEY_CONSTRUCTION_MSBm;

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_no_vft_prog_id,data));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_no_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF,data2));

    /* update DID instruction */
    did_instuction = ARAD_PP_FLP_16B_INST_ARAD_FC_D_ID_8_MSB;    
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);

    /* update VFT/VSI instruction */
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);    

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_no_vft_prog_id, data ));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_no_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, data2 ));

    /* VFT PROGRAM */

     if (is_vsan_from_vsi == 0) {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_ARAD_FC_WITH_VFT_VFT_ID;
    }
    ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_D_ID_8_MSB

    soc_sand_os_memset(data, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));
    soc_sand_os_memset(data2, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));

    vsi_vft_inst_id = -1;
    dip_inst_id = -1;
    for (i = 0; i < table.nof_qualifiers; i++) {
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP) {
            dip_inst_id = table.table_programs[vft_pos].ce_assigned[i];
        }
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI) {
            vsi_vft_inst_id = table.table_programs[vft_pos].ce_assigned[i];
        }
    }

    if (vsi_vft_inst_id == -1 || dip_inst_id == -1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_fcoe_program_tables_init - instruction not found 1")));
    }
      
    flp_key_construction_mem = IHP_FLP_KEY_CONSTRUCTION_MSBm;

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_vft_prog_id,data));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF,data2));

    /* update DID instruction*/
    did_instuction = ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_D_ID_8_MSB;
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);

    /* update VFT/VSI instruction */
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_vft_prog_id, data ));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, data2 ));

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_fcoe_npv_program_tables_init(int unit, int is_vsan_from_vsi, int fcoe_no_vft_prog_id, int fcoe_vft_prog_id)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    uint32 vsi_vft_instuction = 0, did_instuction = 0;
    SOC_DPP_DBAL_TABLE_INFO table;
    int i, vft_pos, no_vft_pos, vsi_vft_inst_id, dip_inst_id;
    soc_mem_t flp_key_construction_mem;
    uint32 data[ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE];
    uint32 data2[ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE];
    SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;

    SOCDNX_INIT_FUNC_DEFS;

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_fcoe_npv_kaps_table_create(unit, is_vsan_from_vsi));
#endif

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B_MSB;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS;
    keys_to_table_id[0].lookup_number = 2;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, fcoe_vft_prog_id , SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,keys_to_table_id, NULL, 1));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, fcoe_no_vft_prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,keys_to_table_id, NULL, 1));

    /*W.A for FCoE: 
      we use qualifiers VSI and SIP when creating the table. but we change them to VFT and SID here...
      1. update the VSI instruction to VFT or VSI
      2. update the SIP instruction to SID
      we are updating only the MSB part of the key, kaps is usung key b MSB */
    
    soc_sand_os_memset(data, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));
    soc_sand_os_memset(data2, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));

    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.pp.dbal_info.dbal_tables.get(unit, SOC_DPP_DBAL_SW_TABLE_ID_FCOE_NPORT_KAPS, &table));

    if (is_vsan_from_vsi == 0) {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_IN_PORT_KEY_GEN_VAR_D_13_BITS;
    } else {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_P6_VSI(12);
    }    
    
    if(table.table_programs[0].program_id == fcoe_no_vft_prog_id){
        no_vft_pos = 0;
        vft_pos = 1;
    }else{
        no_vft_pos = 1;
        vft_pos = 0;
    }

    /* no VFT PROGRAM */

    vsi_vft_inst_id = -1;
    dip_inst_id = -1;
    for (i = 0; i < table.nof_qualifiers; i++) {
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP) {
            dip_inst_id = table.table_programs[no_vft_pos].ce_assigned[i];
        }
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI) {
            vsi_vft_inst_id = table.table_programs[no_vft_pos].ce_assigned[i];
        }
    }

    if (vsi_vft_inst_id == -1 || dip_inst_id == -1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_fcoe_program_tables_init - instruction not found 1")));
    }
    
    flp_key_construction_mem = IHP_FLP_KEY_CONSTRUCTION_MSBm;

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_no_vft_prog_id,data));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_no_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF,data2));

    /* update SID instruction */
    did_instuction = ARAD_PP_FLP_16B_INST_ARAD_FC_S_ID_8_MSB;    
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);

    /* update VFT/VSI instruction */
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);    

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_no_vft_prog_id, data ));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_no_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, data2 ));

    /* VFT PROGRAM */

     if (is_vsan_from_vsi == 0) {
        vsi_vft_instuction = ARAD_PP_FLP_16B_INST_ARAD_FC_WITH_VFT_VFT_ID;
    }
    ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_D_ID_8_MSB

    soc_sand_os_memset(data, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));
    soc_sand_os_memset(data2, 0x0, (ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_ENTRY_SIZE) * sizeof(uint32));

    vsi_vft_inst_id = -1;
    dip_inst_id = -1;
    for (i = 0; i < table.nof_qualifiers; i++) {
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP) {
            dip_inst_id = table.table_programs[vft_pos].ce_assigned[i];
        }
        if (table.qual_info[i].qual_type == SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI) {
            vsi_vft_inst_id = table.table_programs[vft_pos].ce_assigned[i];
        }
    }

    if (vsi_vft_inst_id == -1 || dip_inst_id == -1) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("arad_pp_flp_dbal_fcoe_program_tables_init - instruction not found 1")));
    }
      
    flp_key_construction_mem = IHP_FLP_KEY_CONSTRUCTION_MSBm;

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_vft_prog_id,data));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_read(unit,flp_key_construction_mem,MEM_BLOCK_ANY,fcoe_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF,data2));

    /* update SID instruction*/
    did_instuction = ARAD_IHP_FLP_16B_INST_ARAD_FC_WITH_VFT_S_ID_8_MSB;
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, dip_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), did_instuction);

    /* update VFT/VSI instruction */
    soc_mem_field32_set(unit, flp_key_construction_mem, data, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);
    soc_mem_field32_set(unit, flp_key_construction_mem, data2, arad_pmf_ce_instruction_fld_get(unit,stage, vsi_vft_inst_id - (ARAD_PMF_LOW_LEVEL_CE_NDX_MAX +1)), vsi_vft_instuction);

    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_vft_prog_id, data ));
    SOCDNX_SAND_IF_ERR_EXIT(soc_mem_write(unit, flp_key_construction_mem, MEM_BLOCK_ANY, fcoe_vft_prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, data2 ));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
arad_pp_flp_dbal_mpls_lsr_stat_table_create(int unit)  
{
     SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
     int is_table_initiated = 0;
     
     SOCDNX_INIT_FUNC_DEFS;
     SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_CNT_LEM, &is_table_initiated));
     
     if (!is_table_initiated) {
          DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

          qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
          qual_info[0].qual_offset = 16;
          qual_info[0].qual_nof_bits = 4;
          qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
          qual_info[1].qual_offset = 0;
          qual_info[1].qual_nof_bits = 16;
          SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_CNT_LEM,
          ARAD_PP_FLP_LSR_CNT_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
          SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 2, ARAD_PP_LEM_ACCESS_KEY_LSR_CUNT, qual_info, "FLP LSR STAT LEM"));
     }
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_mpls_lsr_stat_table_init(int unit)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id_static[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_mpls_lsr_stat_table_create(unit));     
    keys_to_table_id_static[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B; /* Key B */ 
    keys_to_table_id_static[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_CNT_LEM;
    keys_to_table_id_static[0].lookup_number =  1;  /* means 1st lookup*/

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_LSR, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
    keys_to_table_id_static, NULL, 1 ));
    
exit:
    SOCDNX_FUNC_RETURN;
}




uint32
   arad_pp_flp_dbal_ipv4uc_rpf_program_tables_init(
     int unit
   )
{
    int nof_tables = 4;
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_rpf_lem_table_create(unit));
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_table_create(unit));

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_IPV4_RPF){    /* in case of KBP is enabled */        
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kbp_table_create(unit,1));
    }else {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_rpf_kaps_table_create(unit));
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_table_create(unit));
    }
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_LEM;
    keys_to_table_id[0].lookup_number = 1;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
    keys_to_table_id[1].lookup_number = 2;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_IPV4_RPF){    /* in case of KBP is enabled */        
        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_MASTER_KBP;
        keys_to_table_id[2].lookup_number = 1;

        keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_FWD_KBP;
        keys_to_table_id[3].lookup_number = 0;

        keys_to_table_id[4].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[4].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KBP;
        keys_to_table_id[4].lookup_number = 2;
        nof_tables = 5;
    }else
#endif
    {
        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS;
        keys_to_table_id[2].lookup_number = 1;
        keys_to_table_id[2].public_lpm_lookup_size = 0;

        keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B_MSB;
        keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS;
        keys_to_table_id[3].lookup_number = 2;
        keys_to_table_id[3].public_lpm_lookup_size = 0;
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV4UC_RPF, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,keys_to_table_id, NULL, nof_tables));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_IPV4UC_RPF, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));

exit:
    SOCDNX_FUNC_RETURN;
}


/* 
 * For trill, DBAL mechanism is used to create the DB in Tcam.
 * This table is used for transparent service in FGL. Two different
 * program are used for two-tag and one-tag scenarios separately.
 * They are associated to the same table.
 */

uint32
    arad_pp_flp_dbal_trill_program_tcam_tables_init(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table[1] = {{ 0 }};
    SOC_PPC_FP_DATABASE_STAGE stage;
    uint8 qualifier_to_ce_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS][SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    int prog[2] = {PROG_FLP_TRILL_MC_TWO_TAGS, PROG_FLP_TRILL_MC_ONE_TAG};
    int iter, rv;

    SOCDNX_INIT_FUNC_DEFS;

    sal_memset(qualifier_to_ce_id, 0, sizeof(qualifier_to_ce_id));
    stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;

    SOCDNX_IF_ERR_EXIT(
        arad_pp_dbal_table_is_initiated(unit,
                                        SOC_DPP_DBAL_SW_TABLE_ID_TRILL_FLP_TCAM,
                                        &is_table_initiated));

    /* Create SOC_DPP_DBAL_SW_TABLE_ID_TRILL_FLP_TCAM */
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

        qual_info[0].qual_type = SOC_PPC_FP_QUAL_TRILL_EGRESS_NICK;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_ESADI;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_TRILL_NATIVE_VLAN_VSI;

        SOCDNX_IF_ERR_EXIT(
            arad_pp_dbal_table_create(unit,
                                      SOC_DPP_DBAL_SW_TABLE_ID_TRILL_FLP_TCAM,
                                      DBAL_PREFIX_NOT_DEFINED, 0,
                                      SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM, 3,
                                      SOC_DPP_DBAL_ATI_NONE, qual_info,
                                      "Trill Tcam"));
    }

    keys_to_table[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_C;
    keys_to_table[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_TRILL_FLP_TCAM;
    keys_to_table[0].lookup_number = 0;

    qualifier_to_ce_id[0][0] = 3;
    qualifier_to_ce_id[0][1] = 2;
    qualifier_to_ce_id[0][2] = 1;
    for (iter = 0; iter < 3; iter++) {
        qualifier_to_ce_id[0][iter] += ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_GROUP;
    }

    for (iter = 0; iter < 2; iter++) {
        rv = arad_pp_dbal_program_to_tables_associate_implicit(unit,
                prog[iter], stage, keys_to_table,
                qualifier_to_ce_id, 1);
        if (rv != SOC_E_NONE) {
            SOCDNX_EXIT_WITH_ERR(rv,
                (_BSL_SOCDNX_MSG("Error! Associating program %d to table %d"
                                 "failed while creating DBAL table for"
                                 "trill mc in stage %d"),
                                 prog[iter],
                                 SOC_DPP_DBAL_SW_TABLE_ID_TRILL_FLP_TCAM,
                                 SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP));
        }
    }

    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Init table for VPWS tagged mode 
 * There are two programs here - one for single tag and one for double tag 
 * Qualifiers are InLif (global) + VLAN tags (one or two according to the program) 
*/
uint32
    arad_pp_flp_dbal_vpws_tagged_program_tables_init(int unit)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint8 prog_id;
    int is_single_tagged, tid_key_index;
    SOC_DPP_DBAL_SW_TABLE_IDS table_id;
    uint32 nof_qual;

    SOCDNX_INIT_FUNC_DEFS;

    for (is_single_tagged=1; is_single_tagged>=0; is_single_tagged--) {
        tid_key_index = 0;
        prog_id = is_single_tagged ? PROG_FLP_VPWS_TAGGED_SINGLE_TAG : PROG_FLP_VPWS_TAGGED_DOUBLE_TAG;
        table_id = is_single_tagged ? SOC_DPP_DBAL_SW_TABLE_ID_VPWS_TAGGED_SINGLE_TAG : SOC_DPP_DBAL_SW_TABLE_ID_VPWS_TAGGED_DOUBLE_TAG;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, table_id, &is_table_initiated));
        if (!is_table_initiated) {
            DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);        
            qual_info[0].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_LIF;
            qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG_ID;
            if (is_single_tagged) {
                nof_qual = 2;
            }
            else {
                /* Double Tagged */
                qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_INNERMOST_VLAN_TAG_ID;
                nof_qual = 3;
            }

            SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, table_id,
                                                         DBAL_PREFIX_NOT_DEFINED, 0,
                                                         SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, nof_qual, SOC_DPP_DBAL_ATI_NONE, qual_info, 
                                                         is_single_tagged ? "FLP VPWS Tagged single tag" : "FLP VPWS Tagged double tag"));
        }

        keys_to_table_id[tid_key_index].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[tid_key_index].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_VPWS_TAGGED_SINGLE_TAG;
        keys_to_table_id[tid_key_index].lookup_number = 1;
        tid_key_index++;

        if (!is_single_tagged) {
            keys_to_table_id[tid_key_index].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
            keys_to_table_id[tid_key_index].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_VPWS_TAGGED_DOUBLE_TAG;
            keys_to_table_id[tid_key_index].lookup_number = 2;
            tid_key_index++;
        }

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, tid_key_index));
    }
  
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_lsr_lem_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    uint32 nof_qual = 0;
    int is_table_initiated = 0;
    int i = 0;
    char *propval;

    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM, &is_table_initiated));
    if(!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);

        qual_info[i].qual_type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
        qual_info[i].qual_offset = 4;
        qual_info[i++].qual_nof_bits = 16;
        qual_info[i].qual_type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
        qual_info[i].qual_offset = 0;
        qual_info[i++].qual_nof_bits = 4;
        qual_info[i++].qual_type = SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_MPLS_EXP;

        propval = soc_property_get_str(unit, spn_MPLS_CONTEXT);
        if (propval && sal_strcmp(propval, "port") == 0) {
            /* in_port defined */
            qual_info[i++].qual_type = SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT;
        } else if(propval && sal_strcmp(propval, "interface") == 0){
            /* inRIF defined */
            qual_info[i++].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
        } else if(propval && sal_strcmp(propval, "vrf") == 0){
            /* VRF defined */
            qual_info[i++].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        }
        
        nof_qual = i;

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM, ARAD_PP_FLP_LSR_KEY_OR_MASK, ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, nof_qual, ARAD_PP_LEM_ACCESS_KEY_TYPE_ILM, qual_info, "FLP LSR"));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_lsr_program_tables_init(int unit)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int nof_tables;
    uint8 prog_id;

    SOCDNX_INIT_FUNC_DEFS;
  
    prog_id = PROG_FLP_LSR;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_MPLS) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_lsr_kbp_table_create(unit));

        keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
        keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP;
        keys_to_table_id[0].lookup_number = 0;

        nof_tables = 1;
    }
    else
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_lsr_lem_table_create(unit));

        keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
        keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM;
        keys_to_table_id[0].lookup_number = 2;

        nof_tables = 1;
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, nof_tables));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));
  
exit:
    SOCDNX_FUNC_RETURN;
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
    arad_pp_flp_dbal_ipv4_dc_program_tables_init(int unit)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);        
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
		qual_info[1].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[1].qual_nof_bits = 8;
		SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC, ARAD_KBP_FRWRD_TBL_ID_IPV4_DC, 0,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 2, SOC_DPP_DBAL_ATI_KBP_LOOKUP_ONLY, qual_info, "FLP IPv4 DOUBLE CAPACITY"));
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC_MASTER, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);        
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
		qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
        qual_info[2].qual_nof_bits = 8;
		SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC_MASTER, ARAD_KBP_FRWRD_TBL_ID_IPV4_DC, 0,
                                                 SOC_DPP_DBAL_PHYSICAL_DB_TYPE_KBP, 3, SOC_DPP_DBAL_ATI_KBP_MASTER_KEY_INDICATION, qual_info, "FLP IPv4 DOUBLE CAPACITY MASTER"));
    }   

    /* LEM lookup */
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_table_create(unit));    

	keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC_MASTER;
    keys_to_table_id[0].lookup_number = 1;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC;
    keys_to_table_id[1].lookup_number = 0;

    keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4_DC;
    keys_to_table_id[2].lookup_number = 2;

    keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
    keys_to_table_id[3].lookup_number = 2;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV4_DC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, 4));
  
exit:
    SOCDNX_FUNC_RETURN;
}

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */


uint32
   arad_pp_flp_dbal_ipv4uc_program_tables_init(
     int unit
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int table_id = 0, num_of_tables = 2;
    
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_table_create(unit));

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_IPV4_UC)
    {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kbp_table_create(unit, 0));
        table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_MASTER_KBP;
        num_of_tables = 3;

        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_FWD_KBP;
        keys_to_table_id[2].lookup_number = 0;
    }else{
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_table_create(unit));
        table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS;
        if (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) {
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_route_scale_table_create(unit));
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_route_scale_short_table_create(unit));
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_route_scale_long_table_create(unit));
        } else {
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_table_create(unit));
        }
    }
#endif /*defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
    keys_to_table_id[0].lookup_number = 2;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
    keys_to_table_id[1].sw_table_id = table_id;
    keys_to_table_id[1].lookup_number = 2;
    keys_to_table_id[1].public_lpm_lookup_size = 0;

    if (SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length) {
        /* Search in the special KAPS and LEM tables */
        keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
        keys_to_table_id[0].lookup_number = 1;

        keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B_MSB;
        keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LONG_KAPS;
        keys_to_table_id[1].lookup_number = 1;
        keys_to_table_id[1].public_lpm_lookup_size = 0;

        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_LEM;
        keys_to_table_id[2].lookup_number = 2;

        keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_ROUTE_SCALE_SHORT_KAPS;
        keys_to_table_id[3].lookup_number = 2;
        keys_to_table_id[3].public_lpm_lookup_size = 0;

        num_of_tables = 4;
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV4UC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                keys_to_table_id, NULL, num_of_tables));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_IPV4UC, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));
  
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_pon_ipv4_sav_static_program_tables_init(
     int unit
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int num_of_tables = 2;
    
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_pon_ipv4_sav_lem_table_create(unit, TRUE, FALSE));

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_pon_ipv4_sav_kaps_table_create(unit, TRUE, FALSE));
#endif /*defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_LEM;
    keys_to_table_id[0].lookup_number = 2;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS;
    keys_to_table_id[1].lookup_number = 1;
    keys_to_table_id[1].public_lpm_lookup_size = 0;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                keys_to_table_id, NULL, num_of_tables));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));
  
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_pon_sav_arp_program_tables_init(
     int unit, int is_up
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int num_of_tables = 2;
    
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_pon_ipv4_sav_lem_table_create(unit, FALSE, is_up));

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_pon_ipv4_sav_kaps_table_create(unit, FALSE, is_up));
#endif /*defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_LEM;
    keys_to_table_id[0].lookup_number = 2;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V4_STATIC_KAPS;
    keys_to_table_id[1].lookup_number = 1;
    keys_to_table_id[1].public_lpm_lookup_size = 0;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                keys_to_table_id, NULL, num_of_tables));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V4_STATIC, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));
  
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_ipv4mc_bridge_program_tables_init(
     int unit
   )
{
    int ssm_lpm_key_id = 0;
    int nof_valid_keys = 2;
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};

    SOCDNX_INIT_FUNC_DEFS;
    /*LEM <FID,G>*/
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4mc_bridge_lem_table_create(unit));
    /*LEM <FID,SA>*/
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4mc_Learning_lem_table_create(unit));
    /*KAPS <FID,G,S>*/
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    if((JER_KAPS_ENABLE(unit)) && (SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode == BCM_IPMC_SSM_KAPS_LPM)) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4mc_bridge_kaps_table_create(unit));
        ssm_lpm_key_id = SOC_DPP_DBAL_PROGRAM_KEY_C_MSB;
    }
#endif
    if(SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode == BCM_IPMC_SSM_TCAM_LPM)
    {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4mc_bridge_tcam_table_create(unit));
        ssm_lpm_key_id = SOC_DPP_DBAL_PROGRAM_KEY_C;
    }

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_LEARN_LEM;
    keys_to_table_id[0].lookup_number = 1;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_LEM;
    keys_to_table_id[1].lookup_number = 2;

    if (SOC_DPP_CONFIG(unit)->pp.ipmc_l2_ssm_mode != BCM_IPMC_SSM_DISABLE) {
        keys_to_table_id[2].key_id = ssm_lpm_key_id;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_BRIDGE_FID;
        keys_to_table_id[2].lookup_number = 2;
        nof_valid_keys = 3;
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV4MC_BRIDGE, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                keys_to_table_id, NULL, nof_valid_keys));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_IPV4MC_BRIDGE, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_ipv4mc_tcam_tables_init(
     int unit
   )
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    uint8 program_id = 0;
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = { { 0 } };

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_DPP_CONFIG(unit)->l3.ipmc_vpn_lookup_enable) {
        program_id = PROG_FLP_IPV4COMPMC_WITH_RPF;
    } else {
        SOCDNX_SAND_IF_ERR_EXIT(arad_pp_flp_app_to_prog_index_get(unit, PROG_FLP_GLOBAL_IPV4COMPMC_WITH_RPF, &program_id));
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, &is_table_initiated));
    if (!is_table_initiated) {
      DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
                        
      qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
      qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
      qual_info[2].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;

      SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS, DBAL_PREFIX_NOT_DEFINED, 0,
                                      SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM, 3, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP IPv4 MC TCAM"));
    }

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_C;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS;
    keys_to_table_id[0].lookup_number = 2;    

    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, program_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, 1));

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_ipv4compmc_with_rpf_program_tables_init(
     int unit
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int nof_tables=0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_rpf_lem_table_create(unit));
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_table_create(unit));

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_LEM;
    keys_to_table_id[0].lookup_number = 1;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_D;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
    keys_to_table_id[1].lookup_number = 2;

    nof_tables = 2;

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_IPV4_MC){

        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4mc_kbp_table_create(unit));
        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_FWD_KBP;
        keys_to_table_id[2].lookup_number = 0;
        keys_to_table_id[2].public_lpm_lookup_size = 0;

        keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
        keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_DUMMY_KBP;
        keys_to_table_id[3].lookup_number = 1;
        keys_to_table_id[3].public_lpm_lookup_size = 0;

        keys_to_table_id[4].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
        keys_to_table_id[4].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_RPF_KBP;
        keys_to_table_id[4].lookup_number = 2;
        keys_to_table_id[4].public_lpm_lookup_size = 0;
        nof_tables = 5;
    }
    else{
        uint8 is_dynamic;

        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4mc_kaps_table_create(unit));
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_rpf_kaps_table_create(unit));

        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS;
        keys_to_table_id[2].lookup_number = 1;
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[2].sw_table_id, &is_dynamic));
        keys_to_table_id[2].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

        keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4MC_KAPS;
        keys_to_table_id[3].lookup_number = 2;
        if (!soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "tcam_l3_mc", 0)) {
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[3].sw_table_id, &is_dynamic));
            keys_to_table_id[3].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);
        }
        nof_tables = 4;
    }
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,keys_to_table_id, NULL, nof_tables));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_IPV4COMPMC_WITH_RPF, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_ipv4uc_l3vpn_rpf_program_tables_init(
     int unit,
     int prog_id
   )
{
    uint8 is_dynamic = 0;
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_rpf_lem_table_create(unit));
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_table_create(unit));

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_rpf_kaps_table_create(unit));
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_table_create(unit));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_LEM;
    keys_to_table_id[0].lookup_number = 1;

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
    keys_to_table_id[1].lookup_number = 2;

    keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
    keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_RPF_KAPS;
    keys_to_table_id[2].lookup_number = 1;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[2].sw_table_id, &is_dynamic));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[2].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

    keys_to_table_id[3].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B_MSB;
    keys_to_table_id[3].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS;
    keys_to_table_id[3].lookup_number = 2;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[3].sw_table_id, &is_dynamic));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[3].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                 keys_to_table_id, NULL, 4));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));

exit:
    SOCDNX_FUNC_RETURN;
}





uint32
   arad_pp_flp_dbal_ipv4uc_l3vpn_program_tables_init(
     int unit,
     int custom_prgrm
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int prog_id;
    uint8 is_dynamic = 0;
    int num_tables = 0;

    SOCDNX_INIT_FUNC_DEFS;
  
    if (custom_prgrm) {
        prog_id = custom_prgrm;
    } else {
        prog_id = PROG_FLP_IPV4UC_PUBLIC;
    }

    if (custom_prgrm) {
        /* custom lookup */
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_custom_table_create(unit));
    } else {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_lem_table_create(unit)); /* lem private lookup */            
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_default_lem_table_create(unit)); /* lem default lookup */
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv4uc_kaps_table_create(unit));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM_DEFAULT;
    keys_to_table_id[num_tables++].lookup_number = 1;

    if (custom_prgrm) {
        keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_CUSTOM_LEM;
        keys_to_table_id[num_tables++].lookup_number = 2;
        keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB; /*SOC_DPP_DBAL_PROGRAM_KEY_D;*/
        keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS;
        keys_to_table_id[num_tables].lookup_number = DBAL_KAPS_2ND_LKP_DIASBLE;
    #if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[num_tables].sw_table_id, &is_dynamic));
    #endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
        keys_to_table_id[num_tables++].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);
    }
    else {
        keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_LEM;
        keys_to_table_id[num_tables++].lookup_number = 2;

        keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB; /*SOC_DPP_DBAL_PROGRAM_KEY_D;*/
        keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS;
        keys_to_table_id[num_tables].lookup_number = DBAL_KAPS_2ND_LKP_DIASBLE;
    #if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[num_tables].sw_table_id, &is_dynamic));
    #endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
        keys_to_table_id[num_tables++].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

        keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B_MSB; /*SOC_DPP_DBAL_PROGRAM_KEY_D;*/
        keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV4UC_KAPS;
        keys_to_table_id[num_tables].lookup_number = 1;
        keys_to_table_id[num_tables++].public_lpm_lookup_size = 0;
    }


    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                 keys_to_table_id, NULL, num_tables));

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, prog_id, SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_ipv6uc_l3vpn_program_tables_init(
     int unit,
     int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    uint8 is_dynamic = 0;
    int num_tables = 0;

    SOCDNX_INIT_FUNC_DEFS;

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kaps_table_create(unit));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/


    keys_to_table_id[num_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B; /*SOC_DPP_DBAL_PROGRAM_KEY_D;*/
    keys_to_table_id[num_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE;
    keys_to_table_id[num_tables].lookup_number = 2;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[num_tables].sw_table_id, &is_dynamic));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[num_tables++].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                 keys_to_table_id, NULL, num_tables));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
   arad_pp_flp_dbal_ipv6uc_l3vpn_rpf_program_tables_init(
     int unit,
     int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int nof_tables = 0;
    uint8 is_dynamic = 0;

    SOCDNX_INIT_FUNC_DEFS;

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kaps_table_create(unit));
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_rpf_kaps_table_create(unit));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
    keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS;
    keys_to_table_id[0].lookup_number = 1;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[0].sw_table_id, &is_dynamic));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[0].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

    keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE;
    keys_to_table_id[1].lookup_number = 2;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[1].sw_table_id, &is_dynamic));
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    keys_to_table_id[1].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);
    nof_tables = 2;

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                 keys_to_table_id, NULL, nof_tables));
exit:
    SOCDNX_FUNC_RETURN;
}

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32
   arad_pp_flp_dbal_ipv6uc_with_rpf_program_tables_init(
     int unit,
     int prog_id
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int nof_tables=0;
    uint8 is_dynamic;

    SOCDNX_INIT_FUNC_DEFS;

    if (ARAD_KBP_ENABLE_IPV6_UC) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kbp_table_create(unit,1));

        keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KBP_MULTI_KEY;
        keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_MASTER_KBP;
        keys_to_table_id[0].lookup_number = 0;
        keys_to_table_id[0].public_lpm_lookup_size = 0;

        keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
        keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_FWD_KBP;
        keys_to_table_id[1].lookup_number = 0;
        keys_to_table_id[1].public_lpm_lookup_size = 0;

        keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
        keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KBP;
        keys_to_table_id[2].lookup_number = 2;
        keys_to_table_id[2].public_lpm_lookup_size = 0;

        nof_tables = 3;
    }
    else{
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kaps_table_create(unit));
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_rpf_kaps_table_create(unit));

        keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
        keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS;
        keys_to_table_id[0].lookup_number = 1;
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[0].sw_table_id, &is_dynamic));
        keys_to_table_id[0].public_lpm_lookup_size = 0;

        keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
        keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE;
        keys_to_table_id[1].lookup_number = 2;
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[1].sw_table_id, &is_dynamic));
        keys_to_table_id[1].public_lpm_lookup_size = 0;
        nof_tables = 2;
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, prog_id, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
                                                                 keys_to_table_id, NULL, nof_tables));
exit:
    SOCDNX_FUNC_RETURN;
}
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/


uint32
   arad_pp_flp_dbal_ipv6uc_program_tables_init(
     int unit
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    int num_of_tables = 0;

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_ARADPLUS_AND_BELOW(unit) || (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "l3_ipv6_uc_use_tcam", 0))) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_tcam_table_create(unit));
        keys_to_table_id[num_of_tables].lookup_number = 1;
        keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_C;
        keys_to_table_id[num_of_tables++].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE;
    } else {
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
        if (ARAD_KBP_ENABLE_IPV6_UC) {
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kbp_table_create(unit,0));
            keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_KBP_MULTI_KEY;
            keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_MASTER_KBP;
            keys_to_table_id[num_of_tables].lookup_number = 0;
            keys_to_table_id[num_of_tables++].public_lpm_lookup_size = 0;

            keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
            keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_FWD_KBP;
            keys_to_table_id[num_of_tables].lookup_number = 0;
            keys_to_table_id[num_of_tables++].public_lpm_lookup_size = 0;
        } else {
            uint8 is_dynamic;

            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kaps_table_create(unit));
            keys_to_table_id[num_of_tables].lookup_number = 2;
            keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE;
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[num_of_tables].sw_table_id, &is_dynamic));
            keys_to_table_id[num_of_tables].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);
            keys_to_table_id[num_of_tables++].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;

            if ((SOC_DPP_CONFIG(unit)->pp.enhanced_fib_scale_prefix_length_ipv6_long)) {
                SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_lem_route_scale_short_table_create(unit));
                SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_lem_route_scale_long_table_create(unit));
                /* The short table is not used for CEs, we use the long table for both KAPS searches to conserve CEs */
                SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kaps_route_scale_short_table_create(unit));
                SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_kaps_route_scale_long_table_create(unit));

                num_of_tables = 0;

                /* Search in the special KAPS and LEM tables */
                keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
                keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_LEM;
                keys_to_table_id[num_of_tables].lookup_number = 2;
                keys_to_table_id[num_of_tables++].public_lpm_lookup_size = 0;

                keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
                keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_LEM;
                keys_to_table_id[num_of_tables].lookup_number = 1;
                keys_to_table_id[num_of_tables++].public_lpm_lookup_size = 0;

                keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_D;
                keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_LONG_KAPS;
                keys_to_table_id[num_of_tables].lookup_number = 1;
                keys_to_table_id[num_of_tables++].public_lpm_lookup_size = 0;

                keys_to_table_id[num_of_tables].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A_MSB;
                keys_to_table_id[num_of_tables].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_ROUTE_SCALE_SHORT_KAPS;
                keys_to_table_id[num_of_tables].lookup_number = 2;
                keys_to_table_id[num_of_tables++].public_lpm_lookup_size = 0;
            }
        }
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/
    }

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV6UC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, num_of_tables));
  
exit:
    SOCDNX_FUNC_RETURN;
}


uint32
   arad_pp_flp_dbal_ipv6mc_program_tables_init(
     int unit
   )
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6mc_table_create(unit));

    if (SOC_IS_ARADPLUS_AND_BELOW(unit) || (SOC_DPP_CONFIG(unit)->pp.l3_mc_use_tcam != ARAD_PP_FLP_L3_MC_USE_TCAM_DISABLE)) {

        keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_C;
        keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC;
        keys_to_table_id[0].lookup_number = 1;        

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV6MC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, 1));
    }else 
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)    
    {        
        int nof_tables=0;
        if (ARAD_KBP_ENABLE_IPV6_MC) {
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6mc_kbp_table_create(unit));
            keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KBP_MULTI_KEY;
            keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_FWD_KBP;
            keys_to_table_id[0].lookup_number = 0;
            keys_to_table_id[0].public_lpm_lookup_size = 0;

            keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
            keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_DUMMY_KBP;
            keys_to_table_id[1].lookup_number = 1;
            keys_to_table_id[1].public_lpm_lookup_size = 0;

            keys_to_table_id[2].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS;
            keys_to_table_id[2].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC_RPF_KBP;
            keys_to_table_id[2].lookup_number = 2;
            keys_to_table_id[2].public_lpm_lookup_size = 0;
            nof_tables = 3;

        }
        else{
            uint8 is_dynamic;

            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_ipv6uc_rpf_kaps_table_create(unit));

            keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A;
            keys_to_table_id[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6UC_RPF_KAPS;
            keys_to_table_id[0].lookup_number = 1;
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[0].sw_table_id, &is_dynamic));
            keys_to_table_id[0].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);

            keys_to_table_id[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
            keys_to_table_id[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_IPV6MC;
            keys_to_table_id[1].lookup_number = 2;
            SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_kaps_table_prefix_is_dynamic(unit, keys_to_table_id[1].sw_table_id, &is_dynamic));
            keys_to_table_id[1].public_lpm_lookup_size = SOC_DPP_DBAL_ZERO_VRF_IN_KEY(is_dynamic);
            nof_tables = 2;
        }

        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_IPV6MC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, nof_tables));
    }
#else
    {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("ipv6mc Error no DB\n")));
    }
#endif /*#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)*/    
    

exit:
    SOCDNX_FUNC_RETURN;
}


uint32
    arad_pp_flp_dbal_ipv6uc_kbp_table_init(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO qual_info = {0};
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;
    SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;
    int msb_ce_offset = ARAD_PMF_LOW_LEVEL_CE_NDX_MAX + 1;

    SOCDNX_INIT_FUNC_DEFS;

    /*KEY A LSB */
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 4));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 5));

    /*KEY A MSB */      
    qual_info.qual_offset = 0;
    qual_info.qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
    arad_pp_dbal_qualifier_full_size_get(unit, stage, qual_info.qual_type, &(qual_info.qual_full_size), &(qual_info.qual_is_in_hdr));
    qual_info.qual_nof_bits = qual_info.qual_full_size;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+0));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+4));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+5));

    /*KEY B LSB*/
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 0, 6));
    
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 0, 7));

    /*KEY B MSB*/
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 1, msb_ce_offset+6));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6UC, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 1, msb_ce_offset+7));

    /* Lookup configurations */
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV6UC, &flp_lookups_tbl));
    flp_lookups_tbl.elk_lkp_valid = 0x1;
    flp_lookups_tbl.elk_wait_for_reply = 0x1;
    flp_lookups_tbl.elk_opcode = PROG_FLP_IPV6UC; /*ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC;*/
    flp_lookups_tbl.elk_key_a_valid_bytes = 8; 
    flp_lookups_tbl.elk_key_a_msb_valid_bytes = 10; 
    flp_lookups_tbl.elk_key_b_valid_bytes = 8;  
    flp_lookups_tbl.elk_key_b_msb_valid_bytes = 8; 
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV6UC, &flp_lookups_tbl));
    
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
    arad_pp_flp_dbal_ipv6mc_kbp_table_init(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO qual_info = {0};
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;
    SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;
    int msb_ce_offset = ARAD_PMF_LOW_LEVEL_CE_NDX_MAX + 1;

    SOCDNX_INIT_FUNC_DEFS;

    /*KEY A LSB */
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 32+16;
    qual_info.qual_nof_bits = 16; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 0));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 4));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 5));


    /*KEY A MSB */
    qual_info.qual_type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_qualifier_full_size_get(unit, stage,qual_info.qual_type, &(qual_info.qual_full_size), &(qual_info.qual_is_in_hdr)));
    qual_info.qual_nof_bits = 16;
    qual_info.qual_offset = 0;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+1));
    
    qual_info.qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_qualifier_full_size_get(unit, stage,qual_info.qual_type, &(qual_info.qual_full_size), &(qual_info.qual_is_in_hdr)));
    qual_info.qual_nof_bits = 16;
    qual_info.qual_offset = 0;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+0));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+4));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 16; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+5));

    /*KEY B LSB*/
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 0, 6));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage,SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 0, 7));

    /*KEY B MSB*/                  
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 1, msb_ce_offset+6));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, PROG_FLP_IPV6MC, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 1, msb_ce_offset+7));
    
    /* Lookup configurations */
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, PROG_FLP_IPV6MC, &flp_lookups_tbl));
    flp_lookups_tbl.elk_lkp_valid = 0x1;
    flp_lookups_tbl.elk_wait_for_reply = 0x1;
    flp_lookups_tbl.elk_opcode = PROG_FLP_IPV6MC; /*ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_MC_RPF;*/
    flp_lookups_tbl.elk_key_a_valid_bytes = 10; 
    flp_lookups_tbl.elk_key_a_msb_valid_bytes = 10; 
    flp_lookups_tbl.elk_key_b_valid_bytes = 8;  
    flp_lookups_tbl.elk_key_b_msb_valid_bytes = 8;
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, PROG_FLP_IPV6MC, &flp_lookups_tbl));

exit:
    SOCDNX_FUNC_RETURN;
}



uint32
    arad_pp_flp_dbal_ipv6uc_rpf_kbp_table_init(int unit, int prog_id)
{
    SOC_DPP_DBAL_QUAL_INFO qual_info = {0};
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;
    SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;
    int msb_ce_offset = ARAD_PMF_LOW_LEVEL_CE_NDX_MAX + 1;

    SOCDNX_INIT_FUNC_DEFS;

    /*KEY A LSB */
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 32+16;
    qual_info.qual_nof_bits = 16; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 0));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 4));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage, SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 0, 5));


    /*KEY A MSB */    
    
    qual_info.qual_type = SOC_PPC_FP_QUAL_IRPP_VRF;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_qualifier_full_size_get(unit, stage,qual_info.qual_type, &(qual_info.qual_full_size), &(qual_info.qual_is_in_hdr)));
    qual_info.qual_nof_bits = 16;
    qual_info.qual_offset = 0;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+0));    

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+4));    

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 16; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_A, qual_info, 1, msb_ce_offset+5));    

    /*KEY B LSB*/
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 0, 6));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage,SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 0, 7));

    /*KEY B MSB*/                  
    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
    qual_info.qual_offset = 0;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 1, msb_ce_offset+6));

    qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH;
    qual_info.qual_offset = 32;
    qual_info.qual_nof_bits = 32; 
    qual_info.qual_full_size = 32;              

    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_key_inst_set(unit, prog_id, stage, SOC_DPP_DBAL_PROGRAM_KEY_B, qual_info, 1, msb_ce_offset+7));
    
    /* Lookup configurations */
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &flp_lookups_tbl));
    flp_lookups_tbl.elk_lkp_valid = 0x1;
    flp_lookups_tbl.elk_wait_for_reply = 0x1;
    flp_lookups_tbl.elk_opcode = PROG_FLP_IPV6UC_RPF; /*ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF*/
    flp_lookups_tbl.elk_key_a_valid_bytes = 10; 
    flp_lookups_tbl.elk_key_a_msb_valid_bytes = 8; 
    flp_lookups_tbl.elk_key_b_valid_bytes = 8;  
    flp_lookups_tbl.elk_key_b_msb_valid_bytes = 8; 
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_set_unsafe(unit, prog_id, &flp_lookups_tbl));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 
arad_pp_flp_dbal_epon_uni_v6_static_lem_table_create(int unit)
{
     SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
     int is_table_initiated = 0;
     SOCDNX_INIT_FUNC_DEFS;
     SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_LEM, &is_table_initiated));
     if (!is_table_initiated) {
          DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
          /* LSB */
          qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW; /* SIP 31:0 */
          qual_info[0].qual_offset = 32;
          qual_info[0].qual_nof_bits = 32;
          qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW; /* SIP 63:32 */
          qual_info[1].qual_nof_bits = 32;
          qual_info[2].qual_type = SOC_PPC_FP_QUAL_TT_LOOKUP1_PAYLOAD; /* TCAM lookup result */
          qual_info[2].qual_nof_bits = 9;  
          SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_LEM,
          (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable ? ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_IP6_SPOOF_STATIC : ARAD_PP_FLP_IP_SPOOF_DHCP_KEY_OR_MASK),
          ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
          SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 3, ARAD_PP_LEM_ACCESS_KEY_TYPE_IP6_SPOOF_STATIC, qual_info, "FLP:TK_EPON_UNI_V6_STATIC_LEM"));
     }
exit:
    SOCDNX_FUNC_RETURN;
}

uint32
arad_pp_flp_dbal_epon_uni_v6_static_tcam_table_create(int unit,uint32 tcam_access_profile_id)
{
     SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
     int is_table_initiated = 0;
     
     SOCDNX_INIT_FUNC_DEFS;
     SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_TCM, &is_table_initiated));
     if (!is_table_initiated) {
          DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
         
          qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW;  /* SIP 31:0 */
          qual_info[0].qual_offset = 32;
          qual_info[0].qual_nof_bits = 32;
          qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW;  /* SIP 63:32 */
          qual_info[1].qual_offset = 0;
          qual_info[1].qual_nof_bits = 32;
          qual_info[2].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;  /* SIP 79:64 */
          qual_info[2].qual_offset = 48;
          qual_info[2].qual_nof_bits = 16;
                                  
          qual_info[3].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;  /* SIP 95:80 */
          qual_info[3].qual_offset = 32;
          qual_info[3].qual_nof_bits = 16;
          qual_info[4].qual_type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;  /* SIP 127:96 */
          qual_info[4].qual_offset = 0;
          qual_info[4].qual_nof_bits = 32;

          qual_info[5].qual_type = SOC_PPC_FP_QUAL_IRPP_IN_LIF; 
          qual_info[5].qual_nof_bits = 18;


          SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_TCM, 
          DBAL_PREFIX_NOT_DEFINED, 0,SOC_DPP_DBAL_PHYSICAL_DB_TYPE_TCAM,6, SOC_DPP_DBAL_ATI_NONE, qual_info, "FLP:TK_EPON_UNI_V6_STATIC_TCAM"));
     }
exit:
     SOCDNX_FUNC_RETURN;
}
uint32 
arad_pp_flp_dbal_epon_uni_v6_static_lem_default_table_create(int unit)
{
    SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    int is_table_initiated = 0;
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_is_initiated(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_LEM_DEFAULT, &is_table_initiated));
    if (!is_table_initiated) {
        DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        qual_info[0].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
        qual_info[0].qual_offset = 32;
        qual_info[0].qual_nof_bits = 16;
        qual_info[1].qual_type = SOC_PPC_FP_QUAL_HDR_FWD_SA;
        qual_info[1].qual_offset = 0;
        qual_info[1].qual_nof_bits = 32;
        qual_info[2].qual_type = SOC_PPC_FP_QUAL_FID;
        qual_info[2].qual_offset = 0;
        qual_info[2].qual_nof_bits = 15;
        SOCDNX_IF_ERR_EXIT(arad_pp_dbal_table_create(unit, SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_LEM_DEFAULT, 
        ARAD_PP_FLP_ETH_KEY_OR_MASK(unit), ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS,
        SOC_DPP_DBAL_PHYSICAL_DB_TYPE_LEM, 3, 0, qual_info, "FLP: TK_EPON_UNI_V6_STATIC_LEM_DEFAULT"));
    }
exit:
    SOCDNX_FUNC_RETURN;
}
uint32  
    arad_pp_flp_dbal_pon_ipv6_sav_static_program_tables_init(int unit,uint8 sa_auth_enabled,uint8 slb_enabled,uint32 tcam_access_profile_id)
{
    SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id_static[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
    SOCDNX_INIT_FUNC_DEFS;
    /* static */
    if (SOC_DPP_CONFIG(unit)->pp.compression_spoof_ip6_enable) {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_epon_uni_v6_static_lem_table_create(unit)); 
        keys_to_table_id_static[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_A; /* Key A */ 
        keys_to_table_id_static[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_LEM;
        keys_to_table_id_static[0].lookup_number = 2;
    }
    else {
        SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_epon_uni_v6_static_tcam_table_create(unit, tcam_access_profile_id));   
        keys_to_table_id_static[0].key_id = SOC_DPP_DBAL_PROGRAM_KEY_C;
        keys_to_table_id_static[0].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_TCM;
        keys_to_table_id_static[0].lookup_number = 1;
    }
    /*learning DB,don't need to add entry by code*/
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_epon_uni_v6_static_lem_default_table_create(unit));  
    keys_to_table_id_static[1].key_id = SOC_DPP_DBAL_PROGRAM_KEY_B;
    keys_to_table_id_static[1].sw_table_id = SOC_DPP_DBAL_SW_TABLE_ID_ETHERNET_TK_EPON_UNI_V6_STATIC_LEM_DEFAULT;

    keys_to_table_id_static[1].lookup_number = ((!sa_auth_enabled && !slb_enabled)? 1 : 0);
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_program_to_tables_associate(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC, SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP,
    keys_to_table_id_static, NULL, 2 ));
    SOCDNX_IF_ERR_EXIT(arad_pp_dbal_flp_hw_based_key_enable(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC,
       SOC_DPP_HW_KEY_LOOKUP_IN_LEARN_KEY));
    SOCDNX_IF_ERR_EXIT(arad_pp_flp_dbal_source_lookup_with_aget_access_enable(unit, PROG_FLP_ETHERNET_TK_EPON_UNI_V6_STATIC));
    
exit:
    SOCDNX_FUNC_RETURN;
} 

/********* DIAGNOSTIC FUNCTIONS *********/
uint32 
    arad_pp_flp_dbal_program_info_dump(int unit, uint32 prog_id)
{
    ARAD_PP_IHB_FLP_KEY_CONSTRUCTION_TBL_DATA flp_key_cons_lsb, flp_key_cons_msb;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA lookups_tbl;    

    SOCDNX_INIT_FUNC_DEFS;    

    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id, &flp_key_cons_lsb));
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_key_construction_tbl_get_unsafe(unit, prog_id+ARAD_PP_FLP_INSTRUCTIONS_NOF, &flp_key_cons_msb));
    SOCDNX_SAND_IF_ERR_EXIT(arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, prog_id, &lookups_tbl));
    /*LOG_CLI((BSL_META("-----------------------------------\n")));*/

    if (!SOC_IS_JERICHO(unit)) { /* values received in Jericho are not correct */
        LOG_CLI((BSL_META(" KEYS:")));

        if ((flp_key_cons_lsb.key_a_inst_0_to_5_valid) || (flp_key_cons_msb.key_a_inst_0_to_5_valid)) {
            LOG_CLI((BSL_META("\t Key A: LSB = 0x%x, MSB = 0x%x,"), flp_key_cons_lsb.key_a_inst_0_to_5_valid, flp_key_cons_msb.key_a_inst_0_to_5_valid));
        } else {
            LOG_CLI((BSL_META("\t Key A: no CE assigned,")));
        }

        if (flp_key_cons_lsb.key_b_inst_0_to_5_valid || (flp_key_cons_msb.key_b_inst_0_to_5_valid)) {
            LOG_CLI((BSL_META("\tKey B: LSB = 0x%x, MSB = 0x%x,"), flp_key_cons_lsb.key_b_inst_0_to_5_valid, flp_key_cons_msb.key_b_inst_0_to_5_valid));
        } else {
            LOG_CLI((BSL_META("\tKey B: no CE assigned,")));
        }

        if (flp_key_cons_lsb.key_c_inst_0_to_5_valid || (flp_key_cons_msb.key_c_inst_0_to_5_valid)) {
            LOG_CLI((BSL_META("\tKey C: LSB = 0x%02x, MSB = 0x%x"), flp_key_cons_lsb.key_c_inst_0_to_5_valid, flp_key_cons_msb.key_c_inst_0_to_5_valid));
        } else {
            LOG_CLI((BSL_META("\tKey C: no CE assigned ")));
        }

        if (SOC_IS_JERICHO(unit)) {
            if (flp_key_cons_lsb.key_d_inst_0_to_7_valid || (flp_key_cons_msb.key_d_inst_0_to_7_valid)) {
                LOG_CLI((BSL_META(",\tKey D: LSB = 0x%x, MSB = 0x%x"), flp_key_cons_lsb.key_d_inst_0_to_7_valid, flp_key_cons_msb.key_d_inst_0_to_7_valid));
            } else {
                LOG_CLI((BSL_META(",\tKey D: no CE assigned ")));
            }
        }
        LOG_CLI((BSL_META("\n")));
    }

    LOG_CLI((BSL_META("LOOKUPS: ")));

    if(lookups_tbl.lem_1st_lkp_valid == 1) {
        LOG_CLI((BSL_META("\tLEM 1st: %s"), arad_pp_dbal_key_id_to_string(unit, lookups_tbl.lem_1st_lkp_key_select) ));
    }else{
        LOG_CLI((BSL_META("\tLEM 1st: Non")));
    }
    if(lookups_tbl.lem_2nd_lkp_valid == 1) {
        LOG_CLI((BSL_META("\tLEM 2nd: %s"), arad_pp_dbal_key_id_to_string(unit, lookups_tbl.lem_2nd_lkp_key_select) ));
    }else{
        LOG_CLI((BSL_META("\tLEM 2nd: Non")));
    }    

    if(lookups_tbl.lpm_1st_lkp_valid == 1) {
        LOG_CLI((BSL_META("\tLPM 1st: %s"), arad_pp_dbal_key_id_to_string(unit, lookups_tbl.lpm_1st_lkp_key_select) ));
    }else{
        LOG_CLI((BSL_META("\tLPM 1st: Non")));
    }    

    if(lookups_tbl.lpm_2nd_lkp_valid == 1) {
        LOG_CLI((BSL_META("\tLPM 2nd: %s"),arad_pp_dbal_key_id_to_string(unit, lookups_tbl.lpm_2nd_lkp_key_select) ));
    }else{
        LOG_CLI((BSL_META("\tLPM 2nd: Non")));
    }    

    if (lookups_tbl.tcam_lkp_db_profile != ARAD_TCAM_ACCESS_PROFILE_INVALID ) {
        uint32 key_select = lookups_tbl.tcam_lkp_key_select;
        if (!SOC_IS_JERICHO(unit)) {
            if (lookups_tbl.tcam_lkp_key_select == ARAD_PP_FLP_TCAM_LKP_KEY_SELECT_KEY_C_HW_VAL) {
                key_select = SOC_DPP_DBAL_PROGRAM_KEY_C;
            }else{
                key_select = SOC_DPP_DBAL_PROGRAM_KEY_A;
            }
        }
        LOG_CLI((BSL_META("\tTCAM 1st: %s DB %d "), arad_pp_dbal_key_id_to_string(unit, key_select), lookups_tbl.tcam_lkp_db_profile ));        
    }else{
        LOG_CLI((BSL_META("\tTCAM 1st: not valid")));
    }    

    if (SOC_IS_JERICHO(unit)) {
        if (lookups_tbl.tcam_lkp_db_profile_1 != ARAD_TCAM_ACCESS_PROFILE_INVALID ) {
            LOG_CLI((BSL_META("\tTCAM 2nd: %s DB %d "), arad_pp_dbal_key_id_to_string(unit, lookups_tbl.tcam_lkp_key_select_1), lookups_tbl.tcam_lkp_db_profile_1 ));
        }else{
            LOG_CLI((BSL_META("\tTCAM 2nd: not valid")));
        }
    }

    if(lookups_tbl.elk_lkp_valid == 1){
        LOG_CLI((BSL_META("\tKBP: opcode %d "), lookups_tbl.elk_opcode));
        if (lookups_tbl.elk_key_a_valid_bytes != 0) {
            LOG_CLI((BSL_META("A LSB (%d) "), lookups_tbl.elk_key_a_valid_bytes));
        }
        if (SOC_IS_JERICHO(unit)) {
            if (lookups_tbl.elk_key_a_msb_valid_bytes != 0) {
                LOG_CLI((BSL_META("A MSB (%d) "), lookups_tbl.elk_key_a_msb_valid_bytes));
            }
        }
        if (lookups_tbl.elk_key_b_valid_bytes != 0) {
            LOG_CLI((BSL_META("B LSB (%d) "), lookups_tbl.elk_key_b_valid_bytes));
        }
        if (SOC_IS_JERICHO(unit)) {
            if (lookups_tbl.elk_key_b_msb_valid_bytes != 0) {
                LOG_CLI((BSL_META("B MSB (%d) "), lookups_tbl.elk_key_b_msb_valid_bytes));
            }
        }

        if (lookups_tbl.elk_key_c_valid_bytes != 0) {
            LOG_CLI((BSL_META("C LSB (%d) "), lookups_tbl.elk_key_c_valid_bytes));
        }
        if (SOC_IS_JERICHO(unit)) {
            if (lookups_tbl.elk_key_c_msb_valid_bytes != 0) {
                LOG_CLI((BSL_META("C MSB (%d) "), lookups_tbl.elk_key_c_msb_valid_bytes));
            }
        }

        if (SOC_IS_JERICHO(unit)) {
            if (lookups_tbl.elk_key_d_lsb_valid_bytes != 0) {
                LOG_CLI((BSL_META("D LSB (%d) "), lookups_tbl.elk_key_d_lsb_valid_bytes));
            }
            if (lookups_tbl.elk_key_d_msb_valid_bytes != 0) {
                LOG_CLI((BSL_META("D MSB (%d) "), lookups_tbl.elk_key_d_msb_valid_bytes));
            }
        }        
    }

    LOG_CLI((BSL_META("\n----------------------------------------------------------------------------------------------------------------------------------------------------------\n")));
    LOG_CLI((BSL_META("\n\n")));
exit:
     SOCDNX_FUNC_RETURN;
}
