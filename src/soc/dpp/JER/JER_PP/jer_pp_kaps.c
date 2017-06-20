/* $Id: jer_kaps.c, hagayco Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#include <soc/mcm/memregs.h>

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TCAM

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <appl/diag/shell.h>
#include <shared/bsl.h>
#include <soc/mem.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/ARAD/arad_kbp.h>

#include <shared/swstate/access/sw_state_access.h>
#include <shared/swstate/sw_state.h>

#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_xpt.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_entry_mgmt.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_diag.h>

#include <soc/scache.h>

#ifdef USE_MODEL
#include <soc/kbp/alg_kbp/include/model.h>
#endif

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/ha.h>
#endif

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* 
 * Offset for table IDs.
 * Defined to differentiate from DB IDs which start from ID 0. */
#define TABLE_ID_OFFSET             10

#define JER_KAPS_DB_IS_PRIVATE(db_id) ((db_id == JER_KAPS_IP_CORE_0_PRIVATE_DB_ID) || (db_id == JER_KAPS_IP_CORE_1_PRIVATE_DB_ID))

#define KAPS_ACCESS sw_state_access[unit].dpp.soc.arad.pp.kaps_db

#ifdef CRASH_RECOVERY_SUPPORT
#define KAPS_VERSION_1_0 1
#define KAPS_STRUCT_SIG 0
typedef enum {
    HA_KAPS_SUB_ID_0 = 0
} HA_KAPS_sub_id_tl;
#endif


/* } */

/*************
 *  MACROS   *
 *************/
/* { */


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


static
    JER_KAPS_DB_CONFIG
        jer_kaps_db_config_info_static[JER_KAPS_IP_NOF_DB] = {

            /* DB 0 (private CORE_0) */ 
            {   
                FALSE, /* Valid */
                0, /* DB Size */
                JER_KAPS_IP_NOF_DB, /* Clone DB ID */
                NULL, /* DB handle */
                NULL /* AD DB handle */
            },

            /* DB 1 (private CORE_1) */ 
            {   
                FALSE, /* Valid */
                0, /* DB Size */
                JER_KAPS_IP_CORE_0_PRIVATE_DB_ID, /* Clone DB ID */
                NULL, /* DB handle */
                NULL /* AD DB handle */
            }, 

            /* DB 2 (public CORE_0) */ 
            {   
                FALSE, /* Valid */
                0, /* DB Size */
                JER_KAPS_IP_NOF_DB, /* Clone DB ID */
                NULL, /* DB handle */
                NULL /* AD DB handle */
            },

            /* DB 4 (public CORE_1) */ 
            {   
                FALSE, /* Valid */
                0, /* DB Size */
                JER_KAPS_IP_CORE_0_PUBLIC_DB_ID, /* Clone DB ID */
                NULL, /* DB handle */
                NULL /* AD DB handle */
            },
        };  



static
    JER_KAPS_TABLE_CONFIG
        jer_kaps_table_config_info_static[JER_KAPS_IP_NOF_TABLES] = {

            /* Table 0 (IPv4 UC private CORE_0) + Non-IP */
            {   
                JER_KAPS_IP_CORE_0_PRIVATE_DB_ID, /* DB ID */ 
                JER_KAPS_TABLE_USE_DB_HANDLE, /* Clone table ID */
                {3, {{"TBL_ID_0", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"IPV4: VRF_0 + DIP_0", 14 + 32 + (112 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 1 (IPv4 UC private CORE_1)  + Non-IP */
            {   
                JER_KAPS_IP_CORE_1_PRIVATE_DB_ID, /* DB ID */ 
                JER_KAPS_TABLE_USE_DB_HANDLE, /* Clone table ID */
                {3, {{"TBL_ID_1", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"IPV4: VRF_1 + SIP_1", 14 + 32 + (112 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            }, 

            /* Table 2 (IPv6 UC private CORE_0) */ 
            {   
                JER_KAPS_IP_CORE_0_PRIVATE_DB_ID, /* DB ID */ 
                JER_KAPS_IP_NOF_TABLES, /* Clone table ID */
                {3, {{"TBL_ID_2", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_2", 14, KBP_KEY_FIELD_EM}, {"DIP_2", 128+ (16 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 3 (IPv6 UC private CORE_1) */ 
            {   
                JER_KAPS_IP_CORE_1_PRIVATE_DB_ID, /* DB ID */ 
                JER_KAPS_IP_CORE_0_PRIVATE_IPV6_UC_TBL_ID, /* Clone table ID */
                {3, {{"TBL_ID_3", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_3", 14, KBP_KEY_FIELD_EM}, {"SIP_3", 128+ (16 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            }, 

            /* Table 4 (IPv4 MC private CORE_0) */ 
            {   
                JER_KAPS_IP_CORE_0_PRIVATE_DB_ID, /* DB ID */ 
                JER_KAPS_IP_NOF_TABLES, /* Clone table ID                                                         MC-GROUP + SIP + IN-RIF */
                {3, {{"TBL_ID_4", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_4", 14, KBP_KEY_FIELD_EM}, {"MC-G-SIP-INRIF_4", 28 + 32 + 15 + (69 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 5 (IPv6 MC private CORE_0) */ 
            {   
                JER_KAPS_IP_CORE_0_PRIVATE_DB_ID, /* DB ID */ 
                JER_KAPS_IP_NOF_TABLES, /* Clone table ID                                                                                      INRIF  */
                {4, {{"TBL_ID_5", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_5", 14, KBP_KEY_FIELD_EM}, {"MC-GROUP_5", 128, KBP_KEY_FIELD_EM}, {"INRIF_5", 15 + (1 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 6 (IPv4 UC public CORE_0)  + Non-IP */
            {   
                JER_KAPS_IP_CORE_0_PUBLIC_DB_ID, /* DB ID */ 
                JER_KAPS_TABLE_USE_DB_HANDLE, /* Clone table ID */
                {3, {{"TBL_ID_6", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"IPV4: VRF_6 + DIP_6", 14 + 32 + (112 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 7 (IPv4 UC public CORE_1)  + Non-IP */
            {   
                JER_KAPS_IP_CORE_1_PUBLIC_DB_ID, /* DB ID */ 
                JER_KAPS_TABLE_USE_DB_HANDLE, /* Clone table ID */
                {3, {{"TBL_ID_7", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"IPV4: VRF_7 + SIP_7", 14 + 32 + (112 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            }, 

            /* Table 8 (IPv6 UC public CORE_0) */ 
            {   
                JER_KAPS_IP_CORE_0_PUBLIC_DB_ID, /* DB ID */ 
                JER_KAPS_IP_NOF_TABLES, /* Clone table ID */
                {3, {{"TBL_ID_8", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_8", 14, KBP_KEY_FIELD_EM}, {"DIP_8", 128 + (16 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 9 (IPv6 UC public CORE_1) */ 
            {   
                JER_KAPS_IP_CORE_1_PUBLIC_DB_ID, /* DB ID */ 
                JER_KAPS_IP_CORE_0_PUBLIC_IPV6_UC_TBL_ID, /* Clone table ID */
                {3, {{"TBL_ID_9", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_9", 14, KBP_KEY_FIELD_EM}, {"SIP_9", 128 + (16 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            }, 

            /* Table 10 (IPv4 MC public CORE_0) */ 
            {   
                JER_KAPS_IP_CORE_0_PUBLIC_DB_ID, /* DB ID */ 
                JER_KAPS_IP_NOF_TABLES, /* Clone table ID                                                           MC-GROUP + SIP + VRF */
                {3, {{"TBL_ID_10", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_10", 14, KBP_KEY_FIELD_EM}, {"MC-G-SIP-INRIF_10", 28 + 32 + 15 + (69 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },

            /* Table 11 (IPv6 MC public CORE_0) */ 
            {   
                JER_KAPS_IP_CORE_0_PUBLIC_DB_ID, /* DB ID */ 
                JER_KAPS_IP_NOF_TABLES, /* Clone table ID                                                                                          INRIF */
                {4, {{"TBL_ID_11", JER_KAPS_TABLE_PREFIX_LENGTH, KBP_KEY_FIELD_TABLE_ID}, {"VRF_11", 14, KBP_KEY_FIELD_EM}, {"MC-GROUP_11", 128, KBP_KEY_FIELD_EM}, {"INRIF_11", 15 + (1 /*padding to 160b*/), KBP_KEY_FIELD_PREFIX}}}, /* key_fields */
                NULL /* table handle */
            },
        };  

static 
    JER_KAPS_SEARCH_CONFIG
        jer_kaps_search_config_info_static[JER_KAPS_NOF_SEARCHES] = {

            /* Search 0: IPv4 UC */
            {
                FALSE, /* valid_tables_num */
                FALSE, /* max_tables_num */
                {JER_KAPS_IP_CORE_1_PRIVATE_IPV4_UC_TBL_ID, JER_KAPS_IP_CORE_1_PUBLIC_IPV4_UC_TBL_ID, JER_KAPS_IP_CORE_0_PRIVATE_IPV4_UC_TBL_ID, JER_KAPS_IP_CORE_0_PUBLIC_IPV4_UC_TBL_ID}, /* tbl_id's*/

                NULL /* JER_KAPS_INSTRUCTION */
            }, 

            /* Search 1: IPv6 UC */
            {
                FALSE, /* valid_tables_num */
                FALSE, /* max_tables_num */
                {JER_KAPS_IP_CORE_1_PRIVATE_IPV6_UC_TBL_ID, JER_KAPS_IP_CORE_1_PUBLIC_IPV6_UC_TBL_ID, JER_KAPS_IP_CORE_0_PRIVATE_IPV6_UC_TBL_ID, JER_KAPS_IP_CORE_0_PUBLIC_IPV6_UC_TBL_ID}, /* tbl_id's*/
                NULL /* JER_KAPS_INSTRUCTION */
            }, 

            /* Search 2: IPv4 MC */
            {
                FALSE, /* valid_tables_num */
                FALSE, /* max_tables_num */
                {JER_KAPS_IP_CORE_1_PRIVATE_IPV4_UC_TBL_ID, JER_KAPS_IP_CORE_1_PUBLIC_IPV4_UC_TBL_ID, JER_KAPS_IP_CORE_0_PRIVATE_IPV4_MC_TBL_ID, JER_KAPS_IP_CORE_0_PUBLIC_IPV4_MC_TBL_ID}, /* tbl_id's*/
                NULL /* JER_KAPS_INSTRUCTION */
            },  

            /* Search 3: IPv6 MC */
            {
                FALSE, /* valid_tables_num */
                FALSE, /* max_tables_num */
                {JER_KAPS_IP_CORE_1_PRIVATE_IPV6_UC_TBL_ID, JER_KAPS_IP_CORE_1_PUBLIC_IPV6_UC_TBL_ID, JER_KAPS_IP_CORE_0_PRIVATE_IPV6_MC_TBL_ID, JER_KAPS_IP_CORE_0_PUBLIC_IPV6_MC_TBL_ID}, /* tbl_id's*/
                NULL /* JER_KAPS_INSTRUCTION */
            }, 
        };

static
    JER_KAPS_DB_CONFIG
        jer_kaps_db_config_info[SOC_SAND_MAX_DEVICE][JER_KAPS_IP_NOF_DB];

static
    JER_KAPS_TABLE_CONFIG
        jer_kaps_table_config_info[SOC_SAND_MAX_DEVICE][JER_KAPS_IP_NOF_TABLES];

static 
    JER_KAPS_SEARCH_CONFIG
        jer_kaps_search_config_info[SOC_SAND_MAX_DEVICE][JER_KAPS_NOF_SEARCHES];
static 
    genericJerAppData 
        *JerAppData[SOC_SAND_MAX_DEVICE] = {NULL};

static
    JER_KAPS_DB_CONFIG
        jer_kaps_dma_db_config_info[SOC_SAND_MAX_DEVICE][JER_KAPS_NOF_MAX_DMA_DB];

static kbp_warmboot_t kaps_warmboot_data[SOC_SAND_MAX_DEVICE];


const char* JER_KAPS_DB_NAMES[] = {"PRIVATE_CORE_0", "PRIVATE_CORE_1", "PUBLIC_CORE_0", "PUBLIC_CORE_1"};

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */


void jer_kaps_sw_init(int unit)
{
    uint32
        db_id,
        db_idx,
        tbl_id,
        tbl_idx,
        search_idx,
        db_size;
    uint8 
        valid_tables_num;
    soc_dpp_config_jer_pp_t *jer_pp_config = &(SOC_DPP_JER_CONFIG(unit)->pp);

    /* Initialize data structures with static configuration */
    sal_memcpy(&jer_kaps_db_config_info[unit][0], &jer_kaps_db_config_info_static[0], sizeof(JER_KAPS_DB_CONFIG)*JER_KAPS_IP_NOF_DB);
    sal_memcpy(&jer_kaps_table_config_info[unit][0], &jer_kaps_table_config_info_static[0], sizeof(JER_KAPS_TABLE_CONFIG)*JER_KAPS_IP_NOF_TABLES);
    sal_memcpy(&jer_kaps_search_config_info[unit][0], &jer_kaps_search_config_info_static[0], sizeof(JER_KAPS_SEARCH_CONFIG)*JER_KAPS_NOF_SEARCHES);
    sal_memset(&jer_kaps_dma_db_config_info[unit][0], 0x0, sizeof(JER_KAPS_DB_CONFIG)*JER_KAPS_NOF_MAX_DMA_DB);


    if (SOC_IS_JERICHO_PLUS(unit) && !SOC_IS_QAX(unit)) {
        /*
         * RPB indices 2 and 3 have switched for Jericho Plus
         *
         * KBPSDK to ASIC RPB blk_ids:
         * 1->2
         * 3->3
         * 0->0
         * 2->1
         *
         * ASIC mapping:
         * 0 - private core 0
         * 1 - private core 1
         * 2 - public  core 0
         * 3 - public  core 1
         */
        for(search_idx = 0; search_idx < JER_KAPS_NOF_SEARCHES; search_idx++) {
            jer_kaps_search_config_info[unit][search_idx].tbl_id[1] = jer_kaps_search_config_info_static[search_idx].tbl_id[2];
            jer_kaps_search_config_info[unit][search_idx].tbl_id[2] = jer_kaps_search_config_info_static[search_idx].tbl_id[1];
        }
    }

    /* First set table sizes according to configuration */
    for ( db_idx = 0; 
          db_idx < JER_KAPS_IP_NOF_DB; 
          db_idx++ ) 
    {
        if (JER_KAPS_DB_IS_PRIVATE(db_idx))
        {
            db_size = jer_pp_config->kaps_private_ip_frwrd_table_size;
        }
        else
        {
            db_size = jer_pp_config->kaps_public_ip_frwrd_table_size;
        }

        if (db_size > 0) 
        {
            if (SOC_IS_QAX(unit) && jer_kaps_db_config_info[unit][db_idx].clone_of_db_id != JER_KAPS_IP_NOF_DB) {
                /* DB clones are only necessary for additional pipelines */
                jer_kaps_db_config_info[unit][db_idx].valid = FALSE;
            } else {
                jer_kaps_db_config_info[unit][db_idx].valid = TRUE;
                jer_kaps_db_config_info[unit][db_idx].db_size = db_size;
            }
        }
    }

    /* Now verify all tables used by each search are valid */
    for(search_idx = 0; search_idx < JER_KAPS_NOF_SEARCHES; search_idx++) 
    {
        valid_tables_num = 0;
        /* Verify instruction has tables configured */
        for ( tbl_idx = 0; tbl_idx < JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES; tbl_idx++) 
        {
            /* If all of the tables are not valid, then the instruction is not valid */
            tbl_id = jer_kaps_search_config_info[unit][search_idx].tbl_id[tbl_idx];
            db_id = jer_kaps_table_config_info[unit][tbl_id].db_id;
            if (jer_kaps_db_config_info[unit][db_id].valid) {
                /* Shift the valid tables to the beginning of the table array */
                if (valid_tables_num != tbl_idx) {
                    jer_kaps_search_config_info[unit][search_idx].tbl_id[valid_tables_num] = jer_kaps_search_config_info[unit][search_idx].tbl_id[tbl_idx];
                }
                valid_tables_num++;
            }
        }
        jer_kaps_search_config_info[unit][search_idx].valid_tables_num = valid_tables_num;
        /*  Mapping of searches to tables:
         *  In Jericho the mapping of tables to searches is:
         *  srch_0 srch_1 srch_2  srch_3
         *    1      7      0       6
         *    3      9      2       8
         *    1      7      4       10
         *    3      9      5       11
         *  When the public DB is disabled we would like to perform only the following searches:
         *  srch_0        srch_2
         *    1             0
         *    3             2
         *    1             4
         *    3             5
         *
         *  The KBPSDK required that the master_key will have a number of key fields that is equal or greater to the highest search
         *  In this case, 2.
         *
         *  In QAX the mapping of tables to searches is:
         *  srch_0  srch_1
         *    0       6
         *    2       8
         *    4       10
         *    5       11
         *  When the public DB is disabled we would like to perform only the following searches:
         *  srch_0
         *    0
         *    2
         *    4
         *    5
         *
         *  Since this is the first search, we do not need to create dummy key fields in the master_key in QAX
         */
        jer_kaps_search_config_info[unit][search_idx].max_tables_num = SOC_IS_JERICHO_PLUS(unit) ? jer_kaps_search_config_info[unit][search_idx].valid_tables_num : JER_KAPS_MAX_NUM_OF_PARALLEL_SEARCHES;
    }

    /* Set number of Direct access DB*/
    if (jer_pp_config->kaps_large_db_size) {
        for ( db_idx = 0; 
              db_idx < ( jer_pp_config->kaps_large_db_size / JER_KAPS_DMA_DB_NOF_ENTRIES ); 
              db_idx++ ) 
        {
              jer_kaps_dma_db_config_info[unit][db_idx].valid = TRUE;
              jer_kaps_dma_db_config_info[unit][db_idx].db_size = JER_KAPS_DMA_DB_NOF_ENTRIES;
        }
    }
}

/* This function performs Device Inits */
STATIC int jer_kaps_init_device(int unit)
{
    uint32 flags;
    kbp_warmboot_t *warmboot_data;
    int32 res;
    uint8 is_allocated;

#ifdef CRASH_RECOVERY_SUPPORT
    uint8 *nv_mem_ptr = NULL;
    unsigned int nv_mem_size = JER_KAPS_NV_MEMORY_SIZE; 
#endif
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    JerAppData[unit]->kaps_xpt_p = NULL;
#ifdef USE_MODEL
    /* Initialize the C-Model */
    res = kbp_sw_model_init(JerAppData[unit]->dalloc_p, KBP_DEVICE_KAPS, KBP_DEVICE_DEFAULT,
                              NULL, &(JerAppData[unit]->kaps_xpt_p));
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_sw_model_init with failed: %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));
    }
#else
#ifndef BLACKHOLE_MODE
    res = jer_pp_xpt_init(unit, &(JerAppData[unit]->kaps_xpt_p));
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
#endif
#endif

    warmboot_data = &kaps_warmboot_data[unit];

    flags = KBP_DEVICE_DEFAULT | KBP_DEVICE_ISSU;
    if (SOC_WARM_BOOT(unit)) {
        flags |= KBP_DEVICE_SKIP_INIT;
#ifdef CRASH_RECOVERY_SUPPORT
        if (JER_KAPS_IS_CR_MODE(unit) == 1) {
            flags &= ~KBP_DEVICE_ISSU;
        }
#endif

    }
    KBP_TRY(kbp_device_init(JerAppData[unit]->dalloc_p, 
                            KBP_DEVICE_KAPS, 
                            flags, 
                            (struct kaps_xpt*)JerAppData[unit]->kaps_xpt_p,
                            NULL,
                            &JerAppData[unit]->kaps_device_p));

#ifdef CRASH_RECOVERY_SUPPORT
    if (JER_KAPS_IS_CR_MODE(unit) == 1) {
        /* Get allocation HA */
        nv_mem_ptr = ha_mem_alloc(unit, HA_KAPS_Mem_Pool, HA_KAPS_SUB_ID_0, KAPS_VERSION_1_0, KAPS_STRUCT_SIG, &nv_mem_size);
        SOC_SAND_CHECK_NULL_PTR(nv_mem_ptr,20,exit);

        if (SOC_WARM_BOOT(unit)){
            KBP_TRY(kbp_device_set_property(JerAppData[unit]->kaps_device_p, KBP_DEVICE_PROP_CRASH_RECOVERY, 0, nv_mem_ptr, nv_mem_size)); 
            KBP_TRY(kbp_device_restore_state(JerAppData[unit]->kaps_device_p, NULL, NULL, NULL));
        } else {
            KBP_TRY(kbp_device_set_property(JerAppData[unit]->kaps_device_p, KBP_DEVICE_PROP_CRASH_RECOVERY, 1, nv_mem_ptr, nv_mem_size)); 
            res = KAPS_ACCESS.is_allocated(unit, &is_allocated);
            SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
            if(!is_allocated) {
                res = KAPS_ACCESS.alloc(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
            }
        }
    } else
#endif
    {
        if (SOC_WARM_BOOT(unit)) {
            res = kbp_device_restore_state(JerAppData[unit]->kaps_device_p, warmboot_data->kbp_file_read , warmboot_data->kbp_file_write, warmboot_data->kbp_file_fp);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit,
                           "Error in %s(): failed with error: %s!\n"), 
                           FUNCTION_NAME(),
                           kbp_get_status_string(res)));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
            }
        } else {
            res = KAPS_ACCESS.is_allocated(unit, &is_allocated);
            SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
            if(!is_allocated) {
                res = KAPS_ACCESS.alloc(unit);
                SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
            }
        }
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_init_nlm_dev_mgr()", 0, 0);
}


STATIC uint32 jer_kaps_db_init(int unit, uint32 db_id)
{
    uint32 
        res = SOC_SAND_OK;

    JER_KAPS_DB_CONFIG
        *db_config_info = &(jer_kaps_db_config_info[unit][db_id]);

    JER_KAPS_DB_HANDLES
        db_handles_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    memset(&db_handles_info, 0, sizeof(db_handles_info));

    /* Check if table was already created, or if it is invalid */
    if((NULL != db_config_info->db_p) || !(db_config_info->valid)) {
        ARAD_DO_NOTHING_AND_EXIT;
    }

    if (!SOC_WARM_BOOT(unit)) {
       LOG_VERBOSE(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "%s(): DB [%d] : create DB with db-size=%d.\n"),
                               FUNCTION_NAME(),
                    db_id,
                    db_config_info->db_size));

        if (db_config_info->clone_of_db_id == JER_KAPS_IP_NOF_DB) {
            /* Not a clone. Create the DB */
            res = kbp_db_init(
                    JerAppData[unit]->kaps_device_p, 
                    KBP_DB_LPM, 
                    db_id,
                    db_config_info->db_size, 
                    &db_config_info->db_p
                  );
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): kbp_db_init failed with : %s!\n"), 
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res)));

                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }

            /* Associate DB with AD table */
            LOG_VERBOSE(BSL_LS_SOC_TCAM,
                        (BSL_META_U(unit,
                                    "%s(): DB [%d] : create AD DB with asso-data=%d.\n"),
                                    FUNCTION_NAME(),
                         db_id,
                         JER_KAPS_AD_WIDTH_IN_BITS));

            KBP_TRY(kbp_ad_db_init(
                        JerAppData[unit]->kaps_device_p, 
                        db_id,
                        db_config_info->db_size, 
                        JER_KAPS_AD_WIDTH_IN_BITS,
                        &db_config_info->ad_db_p)
                    );

            KBP_TRY(kbp_db_set_ad(db_config_info->db_p,
                                  db_config_info->ad_db_p));

            /* save AD DB handle in case of not clone */
            db_handles_info.ad_db_p = db_config_info->ad_db_p;
        }
        else {
            /* DB clones are only necessary for additional pipelines */
            KBP_TRY(kbp_db_clone(jer_kaps_db_config_info[unit][db_config_info->clone_of_db_id].db_p,
                                 db_id,
                                 &(db_config_info->db_p))
                    );
        }

        /*KBP_TRY(kbp_db_set_property(db_config_info->db_p, KBP_PROP_DEFER_DELETES, 1));*/

        /* save DB handles */
        db_handles_info.db_p    = db_config_info->db_p;
        db_handles_info.is_valid = db_config_info->valid;
        res = KAPS_ACCESS.db_info.set(unit, db_id, db_handles_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        ARAD_DO_NOTHING_AND_EXIT;
    }
    else {
        res = KAPS_ACCESS.db_info.get(unit, db_id, &db_handles_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        res = kbp_db_refresh_handle(JerAppData[unit]->kaps_device_p, db_handles_info.db_p, &db_handles_info.db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): Table Key : kbp_db_refresh_handle failed: %s, db_id = %d!\n"),
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res), db_id));

            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 31, exit);
        }

        if (db_config_info->clone_of_db_id == JER_KAPS_IP_NOF_DB) {
            res = kbp_ad_db_refresh_handle(JerAppData[unit]->kaps_device_p, db_handles_info.ad_db_p, &db_handles_info.ad_db_p);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): Table Key : kbp_ad_db_refresh_handle failed: %s, db_id = %d!\n"),
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res), db_id));

                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 31, exit);
            }
            jer_kaps_db_config_info[unit][db_id].ad_db_p = db_handles_info.ad_db_p;
        }

        jer_kaps_db_config_info[unit][db_id].db_p = db_handles_info.db_p;

        res = KAPS_ACCESS.db_info.set(unit, db_id, db_handles_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }
    

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_db_init()", db_id, 0);
}


STATIC uint32 jer_kaps_dma_db_init(int unit, uint32 db_id)
{
    uint32 
        res = SOC_SAND_OK;

    JER_KAPS_DB_CONFIG
        *db_config_info = &(jer_kaps_dma_db_config_info[unit][db_id]);


    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Check if table was already created */
    if(NULL != db_config_info->dma_db_p) {
        ARAD_DO_NOTHING_AND_EXIT;
    }

    res = kbp_dma_db_init(
            JerAppData[unit]->kaps_device_p, 
            db_id,
            db_config_info->db_size,
            JER_KAPS_DMA_DB_WIDTH(unit),
            &db_config_info->dma_db_p
          );
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_db_init failed with : %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));

        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_db_init()", db_id, 0);
}


STATIC uint32 jer_kaps_table_init(int unit, uint32 tbl_id)
{
    uint32 
        key_ndx,
        db_id,
        res = SOC_SAND_OK;

    JER_KAPS_KEY *key;

    JER_KAPS_TABLE_CONFIG
        *tbl_config_info = &(jer_kaps_table_config_info[unit][tbl_id]);

    JER_KAPS_TABLE_HANDLES
        table_handle;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    memset(&table_handle, 0, sizeof(table_handle));

    /* Check if table was already created */
    if(NULL != tbl_config_info->tbl_p) {
        ARAD_DO_NOTHING_AND_EXIT;
    }

    if (!SOC_WARM_BOOT(unit)) 
    {
        /* Call the Add Table API to create each of the tables */
        LOG_VERBOSE(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "%s(): Table [%d] : create table.\n"),
                               FUNCTION_NAME(),
                    tbl_id));

        /* Only init valid (belong to valid DBs) tables */
        if (!jer_kaps_db_config_info[unit][tbl_config_info->db_id].valid) {
            return SOC_SAND_OK;
        }
        if (tbl_config_info->clone_of_tbl_id == JER_KAPS_TABLE_USE_DB_HANDLE) 
        {
            /* Use DB handle. Don't add a table in this case. */
            db_id = tbl_config_info->db_id;
            tbl_config_info->tbl_p = jer_kaps_db_config_info[unit][db_id].db_p;
        }
        else if (tbl_config_info->clone_of_tbl_id == JER_KAPS_IP_NOF_TABLES) 
        {
            /* Not a clone. Create the table */
            db_id = tbl_config_info->db_id;
            res = kbp_db_add_table(
                    jer_kaps_db_config_info[unit][db_id].db_p, 
                    tbl_id + TABLE_ID_OFFSET, 
                    &(tbl_config_info->tbl_p)
                  );
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): kbp_db_add_table failed with : %s!\n"), 
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res)));
           
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }
        }
        else
        {
            res = kbp_db_clone(jer_kaps_table_config_info[unit][tbl_config_info->clone_of_tbl_id].tbl_p,
                                 tbl_id + TABLE_ID_OFFSET,
                                 &(tbl_config_info->tbl_p));
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): kbp_db_clone failed with : %s!\n"),
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res)));

                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }
        }

        /* key creation */
        KBP_TRY(kbp_key_init(JerAppData[unit]->kaps_device_p, &key));

        /* Add fields to key */
        for(key_ndx = 0;
             key_ndx < tbl_config_info->key_fields.nof_key_segments;
             key_ndx++)
        {
            /*Create a unique field name*/
            if(tbl_config_info->key_fields.key_segment[key_ndx].nof_bits){

                res = kbp_key_add_field(
                        key, 
                        tbl_config_info->key_fields.key_segment[key_ndx].name,
                        tbl_config_info->key_fields.key_segment[key_ndx].nof_bits, 
                        tbl_config_info->key_fields.key_segment[key_ndx].type
                      );

                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    if(res != KBP_DUPLICATE_KEY_FIELD) {
                       LOG_ERROR(BSL_LS_SOC_TCAM,
                                 (BSL_META_U(unit,
                                             "Error in %s(): Key Resp Code = %d ,kbp_key_add_field failed: %s!\n"), 
                                             FUNCTION_NAME(),
                                  res,
                                  kbp_get_status_string(res)));

                        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
                    }
                }
            }
        }

        res = kbp_db_set_key(
                tbl_config_info->tbl_p,
                key
              );
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): Table Key : kbp_db_set_key with failed: %s, table_id = %d!\n"),
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res), tbl_id));

            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
        }

        table_handle.tbl_p = tbl_config_info->tbl_p;
        res = KAPS_ACCESS.table_handles.set(unit, tbl_id, table_handle);
        SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

    } else {
        if (tbl_config_info->clone_of_tbl_id == JER_KAPS_TABLE_USE_DB_HANDLE)
        {
            /* Use DB handle. Don't add a table in this case. */
            db_id = tbl_config_info->db_id;
            table_handle.tbl_p = jer_kaps_db_config_info[unit][db_id].db_p;
        }
        else
        {
            /* Not a clone. Refresh the table handle */
            res = KAPS_ACCESS.table_handles.get(unit, tbl_id, &table_handle);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            res = kbp_db_refresh_handle(JerAppData[unit]->kaps_device_p, table_handle.tbl_p, &(table_handle.tbl_p));
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): Table Key : kbp_db_refresh_handle failed: %s, table_id = %d!\n"),
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res), tbl_id));

                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 31, exit);
            }
        }
        tbl_config_info->tbl_p = table_handle.tbl_p;

        res = KAPS_ACCESS.table_handles.set(unit, tbl_id, table_handle);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_table_init()", tbl_id, 0);
}

STATIC uint32 jer_kaps_search_init(int unit, uint32 search_id)
{
    uint32
        key_ndx,
        tbl_idx,
        tbl_id,
        res = SOC_SAND_OK;

    JER_KAPS_KEY *master_key;

    JER_KAPS_TABLE_CONFIG
        *tbl_config_info = NULL;

    JER_KAPS_SEARCH_CONFIG
        *search_config_info = &(jer_kaps_search_config_info[unit][search_id]);

    JER_KAPS_INSTRUCTION_HANDLES
        instruction_handles_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (!SOC_WARM_BOOT(unit)) {
        /* create a new instruction */
        LOG_VERBOSE(BSL_LS_SOC_TCAM,
                    (BSL_META_U(unit,
                                "%s() : create a new instruction, search ID %d.\n"),
                                FUNCTION_NAME(),
                     search_id));

        KBP_TRY(kbp_instruction_init(
                    JerAppData[unit]->kaps_device_p, 
                    search_id,
                    0, 
                    &(search_config_info->inst_p))
                );

        /* Master key creation */
        KBP_TRY(kbp_key_init(JerAppData[unit]->kaps_device_p, &master_key));

        /* Add fields to the master key */
        /* Even if a DB is disabled, we still need to build the full master key */
        for (tbl_idx = 0; tbl_idx < search_config_info->max_tables_num; tbl_idx++)
        {
            tbl_id = search_config_info->tbl_id[tbl_idx];
            tbl_config_info = &(jer_kaps_table_config_info[unit][tbl_id]);

            /* Add fields to key */
            for(key_ndx = 0;
                 key_ndx < tbl_config_info->key_fields.nof_key_segments;
                 key_ndx++)
            {
                /*Create a unique field name*/
                if(tbl_config_info->key_fields.key_segment[key_ndx].nof_bits){

                    res = kbp_key_add_field(
                            master_key, 
                            tbl_config_info->key_fields.key_segment[key_ndx].name,
                            tbl_config_info->key_fields.key_segment[key_ndx].nof_bits, 
                            tbl_config_info->key_fields.key_segment[key_ndx].type
                          );

                    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                        if(res != KBP_DUPLICATE_KEY_FIELD) {
                           LOG_ERROR(BSL_LS_SOC_TCAM,
                                     (BSL_META_U(unit,
                                                 "Error in %s(): Key Resp Code = %d ,kbp_key_add_field failed: %s!\n"), 
                                                 FUNCTION_NAME(),
                                      res,
                                      kbp_get_status_string(res)));

                            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
                        }
                    }
                }
            }
        }

        KBP_TRY(kbp_instruction_set_key(search_config_info->inst_p, master_key));

        for (tbl_idx = 0; tbl_idx < search_config_info->valid_tables_num; tbl_idx++)
        {
            tbl_id = search_config_info->tbl_id[tbl_idx];
            tbl_config_info = &(jer_kaps_table_config_info[unit][tbl_id]);

            if (SOC_IS_JERICHO_PLUS(unit) && !SOC_IS_QAX(unit)) {
                res = kbp_instruction_add_db(search_config_info->inst_p,
                                             jer_kaps_table_config_info[unit][tbl_id].tbl_p,
                                             /* In Jericho plus we have switched the public and private searches, so the multiplication is no longer needed */
                                             tbl_idx);
            } else {
                res = kbp_instruction_add_db(search_config_info->inst_p,
                                             jer_kaps_table_config_info[unit][tbl_id].tbl_p,
                                             /* We would like to skip the public searches, but still maintain the mapping of searches to tables */
                                             /* longer explanation at the assignment to max_tables_num */
                                             tbl_idx * search_config_info->max_tables_num / search_config_info->valid_tables_num);
            }
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): kbp_instruction_add_db failed with : %s, tbl_id = %d result = %d!\n"),
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res), tbl_id, tbl_idx));

                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
            }
        }

        res = kbp_instruction_install(search_config_info->inst_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): kbp_instruction_install failed with : %s, search_id = %d!\n"), 
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res), search_id));

            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
        }

        instruction_handles_info.inst_p = search_config_info->inst_p;

        res = KAPS_ACCESS.search_instruction_info.set(unit, search_id, instruction_handles_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    } else {
        res = KAPS_ACCESS.search_instruction_info.get(unit, search_id, &instruction_handles_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        KBP_TRY(kbp_instruction_refresh_handle(JerAppData[unit]->kaps_device_p, instruction_handles_info.inst_p, &instruction_handles_info.inst_p));

        search_config_info->inst_p = instruction_handles_info.inst_p;

        res = KAPS_ACCESS.search_instruction_info.set(unit, search_id, instruction_handles_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_search_init()", search_id, 0);
}

STATIC
    int jer_kaps_device_lock_config(
        SOC_SAND_IN  int unit
    )
{
    uint32 res = SOC_SAND_OK;

    res = kbp_device_lock(JerAppData[unit]->kaps_device_p);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_device_lock with failed: %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));
    }

    return res;
}

uint32 jer_kaps_init_db_set(int unit)
{
    uint32
        tbl_idx,
        search_idx,
        db_idx,
        db_id,
        res;

    uint8 is_valid;

    JER_KAPS_DB_HANDLES
        db_handles_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (JER_KAPS_ENABLE_PRIVATE_DB(unit) || JER_KAPS_ENABLE_PUBLIC_DB(unit)) {
        /* Configure DBs */
        for (db_idx = 0; db_idx < JER_KAPS_IP_NOF_DB; db_idx++) 
        {
            if (SOC_WARM_BOOT(unit)) {
                res = KAPS_ACCESS.db_info.get(unit, db_idx, &db_handles_info);
                SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

                is_valid = db_handles_info.is_valid;
            } 
            else 
            {
                is_valid = jer_kaps_db_config_info[unit][db_idx].valid; 
            }

            if (is_valid) 
            {
                res = jer_kaps_db_init(
                        unit,  
                        db_idx
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
            }
        }

        /* Configure tables */
        for (tbl_idx = 0; tbl_idx < JER_KAPS_IP_NOF_TABLES; tbl_idx++) 
        {
            db_id = jer_kaps_table_config_info[unit][tbl_idx].db_id;
            is_valid = jer_kaps_db_config_info[unit][db_id].valid;
            if (is_valid)
            {
                res = jer_kaps_table_init(unit, tbl_idx);
                SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
            }
        }

        /* Configure search keys in KAPS */
        for(search_idx = 0; search_idx < JER_KAPS_NOF_SEARCHES; search_idx++)
        {
            if (jer_kaps_search_config_info[unit][search_idx].valid_tables_num)
            {
                /* Configure Search */
                res = jer_kaps_search_init(
                        unit, 
                        search_idx
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
            }
        }
    }

    if (JER_KAPS_ENABLE_DMA(unit)) {
        /* Configure direct Aceess DBs*/
        for (db_idx = 0; db_idx < JER_KAPS_NOF_MAX_DMA_DB; db_idx++) 
        {
             is_valid = jer_kaps_dma_db_config_info[unit][db_idx].valid; 

             if (is_valid) 
             {
                res = jer_kaps_dma_db_init(
                        unit,  
                        db_idx
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
             }

        }
    }

    res = jer_kaps_device_lock_config(unit);
    if(res != 0) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
             (BSL_META_U(unit,
                         "Error in %s(): %s!\n"), 
                         FUNCTION_NAME(),
              kbp_get_status_string(res)));
    }
    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    for (db_idx = 0; db_idx < JER_KAPS_NOF_MAX_DMA_DB; db_idx++) 
    {
         is_valid = jer_kaps_dma_db_config_info[unit][db_idx].valid; 

         if (is_valid) 
         {
            res = jer_pp_kaps_dma_clear_db(
                    unit,  
                    db_idx
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
         }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_init_db_set()", 0, 0);
}

/* 
 * KAPS application init function. 
 * Called from PP Mgmt. 
 */
uint32 jer_kaps_init_app(int unit)
{
    uint32 res;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (JerAppData[unit] == NULL) {
        ARAD_ALLOC_ANY_SIZE(JerAppData[unit], genericJerAppData, 1,"JerAppData[unit]");
        sal_memset(JerAppData[unit], 0x0, sizeof(genericJerAppData));
    }
    else {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kaps is already initialized.\n"), FUNCTION_NAME()));
        ARAD_DO_NOTHING_AND_EXIT;
    }

    /* Create the default allocator */
    KBP_TRY(default_allocator_create(&JerAppData[unit]->dalloc_p));

    /* Initialize Device now */
    res = jer_kaps_init_device(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    jer_kaps_sw_init(unit);

    /* Create KAPS DB, config searches */
    res = jer_kaps_init_db_set(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in jer_kaps_init_app()", 0, 0);
}

STATIC
uint32 jer_kaps_deinit_db_set(
    int unit)
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Set all configuration table to 0 */
    sal_memset(&jer_kaps_db_config_info[unit][0], 0, sizeof(JER_KAPS_DB_CONFIG)*JER_KAPS_IP_NOF_DB);
    sal_memset(&jer_kaps_table_config_info[unit][0], 0, sizeof(JER_KAPS_TABLE_CONFIG)*JER_KAPS_IP_NOF_TABLES);
    sal_memset(&jer_kaps_search_config_info[unit][0], 0, sizeof(JER_KAPS_SEARCH_CONFIG)*JER_KAPS_NOF_SEARCHES);

    jer_kaps_sw_init(unit);

    SOC_SAND_EXIT_AND_SEND_ERROR( "error in  jer_kaps_deinit_db_set()", 0, 0);

}

STATIC 
int
    jer_kaps_deinit_device(
       SOC_SAND_IN int unit
    )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    KBP_TRY(kbp_device_destroy(JerAppData[unit]->kaps_device_p));

#ifdef USE_MODEL
    /* Cleanup the model */
    KBP_TRY(kbp_sw_model_destroy(JerAppData[unit]->kaps_xpt_p));
#else
#ifndef BLACKHOLE_MODE
    jer_pp_xpt_deinit(unit, JerAppData[unit]->kaps_xpt_p);
#endif
#endif

    ARAD_DO_NOTHING_AND_EXIT;
           
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_kaps_deinit_device()", 0, 0);
}

uint32 
    jer_kaps_deinit_app(
        int unit
    )
{
    uint32 res;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if(NULL == JerAppData[unit])
    {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kaps not initialized.\n"), FUNCTION_NAME()));
        ARAD_DO_NOTHING_AND_EXIT;
    }

    res = jer_kaps_deinit_db_set(unit); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = jer_kaps_deinit_device(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* Free the default allocator */
    KBP_TRY(default_allocator_destroy(JerAppData[unit]->dalloc_p));

    ARAD_FREE(JerAppData[unit]);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_deinit_app()", unit, 0x0);
}

int
jer_kaps_autosync_set(int unit, int enable)
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    
    ARAD_DO_NOTHING_AND_EXIT;
           
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in kbp_sync()", 0, 0);
}

int
 jer_kaps_sync(int unit)
{
    kbp_warmboot_t *warmboot_data;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

/* Do not perform sync in CR mode */
#ifdef CRASH_RECOVERY_SUPPORT
    if(!(JER_KAPS_IS_CR_MODE(unit)))
#endif
    {
        warmboot_data = &kaps_warmboot_data[unit];

        if (NULL == JerAppData[unit]) ARAD_DO_NOTHING_AND_EXIT;

        KBP_TRY(kbp_device_save_state(JerAppData[unit]->kaps_device_p, warmboot_data->kbp_file_read , warmboot_data->kbp_file_write, warmboot_data->kbp_file_fp));
    }
   
    ARAD_DO_NOTHING_AND_EXIT;
           
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in kbp_sync()", 0, 0);
}

#ifdef CRASH_RECOVERY_SUPPORT
uint32 jer_kaps_cr_transaction_cmd(int unit,
                                   uint8 is_start)
{
    if (JerAppData[unit]->kaps_device_p != NULL) {
        if (is_start) {
            KBP_TRY(kbp_device_start_transaction(JerAppData[unit]->kaps_device_p));
        } else { 
            KBP_TRY(kbp_device_end_transaction(JerAppData[unit]->kaps_device_p));
        }
    }
    return 0;
}

uint8 jer_kaps_cr_query_restore_status(int unit)
{
    uint8 result=TRUE;

    enum kbp_restore_status res_status;
    if(JerAppData[unit]->kaps_device_p != NULL){
        KBP_TRY(kbp_device_query_restore_status(JerAppData[unit]->kaps_device_p, &res_status));
    }

    if (res_status == KBP_RESTORE_CHANGES_ABORTED) {
        result = FALSE;
    }

    return result;
}

uint32 jer_kaps_cr_clear_restore_status(int unit)
{
    if(JerAppData[unit]->kaps_device_p != NULL){
        KBP_TRY(kbp_device_clear_restore_status(JerAppData[unit]->kaps_device_p));
    }
    return 0;
}

uint32 jer_kaps_cr_db_commit(int unit,
                             uint32 tbl_idx)
{
    struct kbp_db *db_p = NULL;
    uint32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    jer_kaps_db_get(unit, tbl_idx, &db_p);

    if (db_p != NULL) {
        res = kbp_db_install(db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_db_commit()", 0, 0);
}
#endif

void 
  jer_kaps_warmboot_register(int unit,
                             FILE *file_fp,
                             kbp_device_issu_read_fn read_fn, 
                             kbp_device_issu_write_fn write_fn)
{
    kaps_warmboot_data[unit].kbp_file_fp = file_fp;
    kaps_warmboot_data[unit].kbp_file_read = read_fn;
    kaps_warmboot_data[unit].kbp_file_write = write_fn;
}

/* 
 *  Table Get functions
 */
void jer_kaps_db_get(
   int unit,
   uint32 tbl_id,
   struct kbp_db **db_p)
{
    *db_p = jer_kaps_table_config_info[unit][tbl_id].tbl_p;
}

void jer_kaps_ad_db_get(
   int unit,
   uint32 tbl_id,
   struct kbp_ad_db **ad_db_p)
{
    uint32 db_id;

    db_id = jer_kaps_table_config_info[unit][tbl_id].db_id;
    *ad_db_p = jer_kaps_db_config_info[unit][db_id].ad_db_p;
}

void jer_kaps_dma_db_get(
   int unit,
   uint32 db_id,
   struct kbp_dma_db **db_p)
{

    *db_p = jer_kaps_dma_db_config_info[unit][db_id].dma_db_p;
}


void jer_kaps_search_config_get(
   int unit,
   uint32 search_id,
   JER_KAPS_SEARCH_CONFIG *search_cfg_p)
{
    *search_cfg_p = jer_kaps_search_config_info[unit][search_id];
}

void jer_kaps_table_config_get(
   int unit,
   uint32 tbl_id,
   JER_KAPS_TABLE_CONFIG *table_cfg_p)
{
    *table_cfg_p = jer_kaps_table_config_info[unit][tbl_id];
}

void jer_kaps_ad_db_by_db_id_get(
    int unit,
    uint32 db_id,
    JER_KAPS_AD_DB **ad_db_p)
{
    *ad_db_p = jer_kaps_db_config_info[unit][db_id].ad_db_p;
}


uint8 jer_kaps_clone_of_db_id_get(
    int unit,
    uint32 db_id)
{
    return jer_kaps_db_config_info[unit][db_id].clone_of_db_id;
}

void* jer_kaps_kaps_xpt_p_get(
    int unit)
{
    return JerAppData[unit]->kaps_xpt_p;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */
