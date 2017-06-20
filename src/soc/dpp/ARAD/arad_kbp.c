/* $Id: arad_kbp.c,v 1.50 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TCAM

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/drv.h>

#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/arad_kbp_rop.h>
#include <soc/dpp/ARAD/arad_kbp_xpt.h>
#include <soc/dpp/ARAD/arad_kbp_recover.h>

#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/ARAD/arad_api_nif.h>
#include <soc/dpp/ARAD/arad_api_ports.h>

#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_ce_instruction.h>


#include <soc/dpp/port_sw_db.h>

#include <soc/dpp/SAND/Management/sand_low_level.h>

#include <soc/scache.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/ha.h>
#endif

/* } */

/*************
 * DEFINES   *
 *************/
/* { */
#define arad_kbp_alloc(s,x) sal_alloc((s*x),__FILE__)
#define arad_kbp_free(s) sal_free(s)

/* KBP mdio */
#define ARAD_KBP_MDIO_CLAUSE 45

/* KBP ROP test */
#define ARAD_KBP_ROP_TEST_REG_ADDR       0x102 
#define ARAD_KBP_ROP_TEST_DATA          {0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x35}

#ifdef CRASH_RECOVERY_SUPPORT
#define KBP_VERSION_1_0 1
#define KBP_STRUCT_SIG 0
typedef enum {
    HA_KBP_SUB_ID_0 = 0
} HA_KBP_sub_id_tl;
#endif

/*
 *    ELK CPU/ROP Defines
 */






/* 
 *  Arad NLM Application
 */ 

 /* Number of devices */

/* 
 * ARAD frwarding NLM Application Defines
 */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */


void alg_kbp_callback(void *handle, struct kbp_db *db,struct kbp_entry *entry, int32_t old_index, int32_t new_index);


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
    ARAD_KBP_TABLE_CONFIG
        Arad_kbp_dummy_table_config_info_static =  {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_DUMMY_0, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{ /* entry_key_parsing */
					0, 
					{}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
		};

static int Arad_kbp_default_result_sizes_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] 	= {
	ARAD_KBP_LUT_AD_TRANSFER_6B,
	ARAD_KBP_LUT_AD_TRANSFER_2B,
	ARAD_KBP_LUT_AD_TRANSFER_3B,
	ARAD_KBP_LUT_AD_TRANSFER_4B
};

static int Arad_kbp_default_result_formats_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] 	= {
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD
};

static
    ARAD_KBP_TABLE_CONFIG
        Arad_kbp_table_config_info_static[ARAD_KBP_FRWRD_IP_NOF_TABLES_ARAD] = {

            /* TABLE 0 (IPv4 + RPF): 80b, <4'b 0s> <12b VRF> <32b DIP> <32b don't care> */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{ /* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 1 (IPv4 + RPF): 80b, <4'b 0s> <12b VRF> <32b SIP> <32b don't care> */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            }, 

            /* TABLE 2 (IPv4 Multicast + RPF): 80b, <12b IN-RIF> <32b SIP> <28b DIP> */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					4, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 3 (IPv6 Unicast) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                128, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 16, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },
            
            /* TABLE 4 (IPv6 Unicast RPF) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                128, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 16, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 5 (IPv6 Multicast) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV6_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_320,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					4, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 16, KBP_KEY_FIELD_PREFIX},{"SIP", 16, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 6 (MPLS) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_LSR, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"MPLS-KEY", 6, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 7 (TRILL UC) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_TRILL_UC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"EGRESS-NICK", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 8 (TRILL MC) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_TRILL_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"TRILL-MC-KEY", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },
        };  

static 
    ARAD_KBP_LTR_CONFIG
        Arad_kbp_ltr_config_static[ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD] = {

            /* DB 0: IPv4 UC (without RPF) */
            {
                FALSE, /* valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_UC, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_UC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 1: IPv4 UC with RPF */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF, /* ltr_id */
                0x3, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 2: IPv4 MC with RPF */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_MC_COMP, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF, /* ltr_id */
                0x3, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_MC, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_ACL, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 4, KBP_KEY_FIELD_PREFIX}, {"SIP", 4, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}},
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}, {"SIP", 4, KBP_KEY_FIELD_PREFIX}, {"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 3: IPv6 UC (without RPF) */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_UC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },

            /* DB 4: IPv6 UC (with RPF in 2 Pass - for Pass 1 SIP) */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF_2PASS, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF_2PASS, /* ltr_id */
                0x0, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                    {2, {{"VRF", 2}, {"SIP", 16}}},
                    {2, {{"VRF", 2}, {"SIP", 16}}}                    
                },
                /* master key */
                {2, {{"VRF", 2}, {"SIP", 16}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },

            /* DB 5: IPv6 UC with RPF */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF, /* ltr_id */
                0x0, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                    {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                    {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 6: IPv6 MC with RPF */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_MC_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_MC_RPF, /* ltr_id */
                0x3, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_MC, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_ACL, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                    {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}, {"SIP", 16, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}},
                    {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 7: MPLS */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_LSR, /* opcode */
                ARAD_KBP_FRWRD_LTR_LSR, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_LSR}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"MPLS-KEY", 6, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"MPLS-KEY", 6, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 8: TRILL UC */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_UC, /* opcode */
                ARAD_KBP_FRWRD_LTR_TRILL_UC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_TRILL_UC}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"EGRESS-NICK", 2, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"EGRESS-NICK", 2, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 9: TRILL MC */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_MC, /* opcode */
                ARAD_KBP_FRWRD_LTR_TRILL_MC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_TRILL_MC}, /* tbl_id's*/ 
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"TRILL-MC-KEY", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"TRILL-MC-KEY", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }
        };

static int Arad_plus_kbp_default_result_sizes_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] 	= {
	ARAD_KBP_LUT_AD_TRANSFER_6B,
	ARAD_KBP_LUT_AD_TRANSFER_4B,
	ARAD_KBP_LUT_AD_TRANSFER_2B,
	ARAD_KBP_LUT_AD_TRANSFER_3B
};

static int Arad_plus_kbp_default_result_formats_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] 	= {
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD
};

#ifdef BCM_88660_A0
static
    ARAD_KBP_TABLE_CONFIG
        Arad_plus_kbp_table_config_info_static[ARAD_KBP_FRWRD_IP_NOF_TABLES_ARAD_PLUS] = {

            /* TABLE 0 (IPv4 + RPF): 80b, <4'b 0s> <12b VRF> <32b DIP> <32b don't care> */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{ /* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 1 (IPv4 + RPF): 80b, <4'b 0s> <12b VRF> <32b SIP> <32b don't care> */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0 /* Clone table ID */
            }, 

            /* TABLE 2 (IPv4 Multicast + RPF): 80b, <12b IN-RIF> <32b SIP> <28b DIP> */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					4, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 3 (IPv6 Unicast) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                128, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 16, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },
            
            /* TABLE 4 (IPv6 Unicast RPF) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                128, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 16, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0 /* Clone table ID */
            },

            /* TABLE 5 (IPv6 Multicast) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV6_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_320,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					4, 
					{{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 16, KBP_KEY_FIELD_PREFIX},{"SIP", 16, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 6 (MPLS) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_LSR, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"MPLS-KEY", 6, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 7 (TRILL UC) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_TRILL_UC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"EGRESS-NICK", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 8 (TRILL MC) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_TRILL_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_64B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                0, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"TRILL-MC-KEY", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 9 (IP LSR SHARED table) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_128B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					3, 
					{{"MPLS-EXT-KEY", 3, KBP_KEY_FIELD_PREFIX},{"VRF", 2, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 10 (IP LSR SHARED for IP + RPF) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_IP, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_128B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					3, 
					{{"HOLE", 3, KBP_KEY_FIELD_PREFIX},{"VRF", 2, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED /* Clone table ID */
            },

            /* TABLE 11 (IP LSR SHARED for LSR) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_LSR, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_128B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					3, 
					{{"MPLS-EXT-KEY", 3, KBP_KEY_FIELD_PREFIX},{"HOLE", 2, KBP_KEY_FIELD_PREFIX},{"HOLE", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED /* Clone table ID */
            },

            /* TABLE 12 (IPv6 extended table) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_160,   /* Table Width */
                NLM_TBL_ADLEN_128B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF", 2, KBP_KEY_FIELD_PREFIX},{"DIP", 16, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 13 (p2p extended table) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,    /* Table Width */
                NLM_TBL_ADLEN_128B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"IN-LIF-ID", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* TABLE 14 (inrif mapping table) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,    /* Table Width */
                NLM_TBL_ADLEN_128B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

			/* TABLE 15 (IPV4_DC) */ 
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_IPV4_DC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_48B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					2, 
					{{"VRF_8B", 1, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

			/* TABLE 16 (DUMMY_IPV4_MC) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                -1, /* Min Priority */
				{/* entry_key_parsing */
					0,
					{}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

			/* TABLE 17 (DUMMY_IPV6_MC) */ 
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,   /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                -1, /* Min Priority */
				{/* entry_key_parsing */
					0,
					{}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },

            /* This table is only put to use when no forwarding tables are initialized
               to ensure the successful execution of kbp_device_lock */
			/* TABLE 18 (DUMMY_FRWRD) */
            {   
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TBL_ID_DUMMY_FRWRD, /* Table ID */ 
                0, /* Table Size */
                NLM_TBL_WIDTH_80,    /* Table Width */
                NLM_TBL_ADLEN_32B,  /* Associated Data Width */ 
                0,  /* Group ID Start */
                20, /* Group ID End */
                NLMDEV_BANK_0, /* Bank Number */
                32, /* Min Priority */
				{/* entry_key_parsing */
					1, 
					{{"DUMMY", 1, KBP_KEY_FIELD_PREFIX}}
				},
				ARAD_KBP_FRWRD_IP_NOF_TABLES /* Clone table ID */
            },
        };  

ARAD_KBP_LTR_SINGLE_SEARCH ipv4_mc_extended_search = 
       {4, {{"VRF", 2, KBP_KEY_FIELD_TERNARY}, {"IN-RIF", 2, KBP_KEY_FIELD_TERNARY}, {"DIP", 4, KBP_KEY_FIELD_TERNARY}, {"SIP", 4, KBP_KEY_FIELD_TERNARY}}} ;

ARAD_KBP_LTR_SINGLE_SEARCH inrif_mapping_search = 
       {1, { {"IN-RIF", 2, KBP_KEY_FIELD_TERNARY}}} ;

ARAD_KBP_LTR_SINGLE_SEARCH ipv4_mc_extended_search_1 = 
       {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 4, KBP_KEY_FIELD_PREFIX}}};

ARAD_KBP_LTR_SINGLE_SEARCH ipv6_dip_sip_sharing_forwarding_header_search =
       {3, {{"UNUSED_HEADER_PART", 8, KBP_KEY_FIELD_PREFIX}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}};

static 
    ARAD_KBP_LTR_CONFIG
        Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD_PLUS] = {

            /* DB 0: IPv4 UC (without RPF) */
            {
                FALSE, /* valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_UC, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_UC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}}                
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 1: IPv4 UC with RPF */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF, /* ltr_id */
                0x5, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmTRUE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 2: IPv4 MC with RPF */
            {
				/* In order to set the MC_RPF on the same lookup as the UC_RPF (lookup#2),
				 * a dummy lookup is need to be added on lookup#1. 
				 * this dummy lookup can be overwritten by ACL.
				 * the difference between the MC_RPF and the UC_RPF configuration is caused because of 
				 * UC_RPF using cmp3_search=TRUE which binding lookup#0 and lookup #2 to the same DB  
				 */ 
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_MC_COMP, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF, /* ltr_id */
                0x7, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_MC, ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_ACL, ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 4, KBP_KEY_FIELD_PREFIX}, {"SIP", 4, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}},
                {1, {{"VRF", 2, KBP_KEY_FIELD_EM}}}, /* Dummy lookup - can be overwritten by ACL */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}, {"SIP", 4, KBP_KEY_FIELD_PREFIX}, {"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 3: IPv6 UC (without RPF) */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_UC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },
            /* DB 4: IPv6 UC (with RPF in 2 Pass - for Pass 1 SIP) */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF_2PASS, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF_2PASS, /* ltr_id */
                0x0, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                    {2, {{"VRF", 2}, {"SIP", 16}}},
                    {2, {{"VRF", 2}, {"SIP", 16}}}                    
                },
                /* master key */
                {2, {{"VRF", 2}, {"SIP", 16}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },
            /* DB 5: IPv6 UC with RPF */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF, /* ltr_id */
                0x5, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0,ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING,ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmTRUE, /* is_cmp3_search */
                { /* ltr searches */
                    {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                    {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {3, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 6: IPv6 MC with RPF */
            {
				/* In order to set the MC_RPF on the same lookup as the UC_RPF (lookup#2),
				 * a dummy lookup is need to be added on lookup#1. 
				 * this dummy lookup can be overwritten by ACL.
				 * the difference between the MC_RPF and the UC_RPF configuration is caused because of 
				 * UC_RPF using cmp3_search=TRUE which binding lookup#0 and lookup #2 to the same DB  
				 */ 
				FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_MC_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV6_MC_RPF, /* ltr_id */
                0x7, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV6_MC, ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC, ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_ACL, ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                    {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}, {"SIP", 16, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}},
                    {1, {{"VRF", 2, KBP_KEY_FIELD_EM}}}, /* Dummy lookup - can be overwritten by ACL */
                    {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}, {"SIP", 16, KBP_KEY_FIELD_PREFIX}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 7: MPLS */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_LSR, /* opcode */
                ARAD_KBP_FRWRD_LTR_LSR, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_LSR}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"MPLS-KEY", 6, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"MPLS-KEY", 6, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 8: TRILL UC */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_UC, /* opcode */
                ARAD_KBP_FRWRD_LTR_TRILL_UC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_TRILL_UC}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"EGRESS-NICK", 2, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"EGRESS-NICK", 2, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            }, 

            /* DB 9: TRILL MC */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_MC, /* opcode */
                ARAD_KBP_FRWRD_LTR_TRILL_MC, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_TRILL_MC}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"TRILL-MC-KEY", 4, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"TRILL-MC-KEY", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },

			/* DB 10: IPV4_DC */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_DC, /* opcode */
                ARAD_KBP_FRWRD_LTR_IPV4_DC, /* ltr_id */
                0x5, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_IPV4_DC, ARAD_KBP_FRWRD_TBL_ID_IPV4_DC}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmTRUE, /* is_cmp3_search */
                { /* ltr searches */
                    {2, {{"VRF_8B", 1, KBP_KEY_FIELD_EM}, {"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                    {2, {{"VRF_8B", 1, KBP_KEY_FIELD_EM}, {"DIP", 4, KBP_KEY_FIELD_PREFIX}}}
				},
                /* master key */
                {3, {{"VRF_8B", 1, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },

            /* DB 11: Shared IPv4 LSR primary (not used for any search) */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_NONE, /* opcode */                
                ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {3, {{"MPLS-EXT-KEY", 3, KBP_KEY_FIELD_TERNARY},{"VRF", 2, KBP_KEY_FIELD_TERNARY},{"DIP", 4, KBP_KEY_FIELD_TERNARY}}}
                },
                /* master key */
                {3, {{"MPLS-EXT-KEY", 3, KBP_KEY_FIELD_TERNARY},{"VRF", 2, KBP_KEY_FIELD_TERNARY},{"DIP", 4, KBP_KEY_FIELD_TERNARY}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },  

            /* DB 12: Shared IPv4 LSR Shared for IP */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_IP, /* opcode */
                ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_IP, /* ltr_id */
                0x3, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_IP, ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_ACL}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {3, {{"HOLE", 3, KBP_KEY_FIELD_HOLE},{"VRF", 2, KBP_KEY_FIELD_TERNARY},{"DIP", 4, KBP_KEY_FIELD_TERNARY}}},
                {1, {{"IN-RIF", 2, KBP_KEY_FIELD_TERNARY}}}
                },
                /* master key */
                {4, {{"VRF", 2, KBP_KEY_FIELD_TERNARY},{"SIP", 4, KBP_KEY_FIELD_TERNARY},{"DIP", 4, KBP_KEY_FIELD_TERNARY},{"IN-RIF", 2, KBP_KEY_FIELD_TERNARY}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },  

            /* DB 13: IPv4 LSR Shared for IP with RPF*/
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_IP_WITH_RPF, /* opcode */
                ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_IP_WITH_RPF, /* ltr_id */
                0x7, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_IP, ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1, ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_FORWARDING, ARAD_KBP_DB_TYPE_ACL}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {3, {{"HOLE", 3, KBP_KEY_FIELD_HOLE},{"VRF", 2, KBP_KEY_FIELD_TERNARY},{"DIP", 4, KBP_KEY_FIELD_TERNARY}}},
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"SIP", 4, KBP_KEY_FIELD_PREFIX}}},
                {1, {{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {4, {{"VRF", 2, KBP_KEY_FIELD_EM},{"SIP", 4, KBP_KEY_FIELD_PREFIX},{"DIP", 4, KBP_KEY_FIELD_PREFIX},{"IN-RIF", 2, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },  

            /* DB 14: Shared IPv4 LSR Shared for LSR */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_LSR, /* opcode */
                ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_LSR, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_LSR}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {3, {{"MPLS-EXT-KEY", 3, KBP_KEY_FIELD_TERNARY}, {"HOLE", 2, KBP_KEY_FIELD_HOLE}, {"HOLE", 4, KBP_KEY_FIELD_HOLE}}}
                },
                /* master key */
                {1, {{"MPLS-EXT-KEY", 3, KBP_KEY_FIELD_TERNARY}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },
            
            /* DB 15: extended IPv6 */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_IPV6, /* opcode */
                ARAD_KBP_FRWRD_LTR_IP_LSR_EXTENDED_IPV6, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM},{"DIP", 16, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {2, {{"VRF", 2, KBP_KEY_FIELD_EM}, {"DIP", 16, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },

            /* DB 16: extended p2p */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_P2P, /* opcode */
                ARAD_KBP_FRWRD_LTR_IP_LSR_EXTENDED_P2P, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"IN-LIF-ID", 2, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"IN-LIF-ID", 2, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },

            /* DB 17: Dummy forwarding */
            {
                FALSE, /* Valid */
                ARAD_KBP_FRWRD_TABLE_OPCODE_NONE, /* opcode */
                ARAD_KBP_FRWRD_LTR_DUMMY_FRWRD, /* ltr_id */
                0x1, /* parallel_srches_bmp */
                {ARAD_KBP_FRWRD_TBL_ID_DUMMY_FRWRD}, /* tbl_id's*/
                {ARAD_KBP_DB_TYPE_FORWARDING}, /* Search Types*/ 
                NlmFALSE, /* is_cmp3_search */
                { /* ltr searches */
                {1, {{"DUMMY", 1, KBP_KEY_FIELD_PREFIX}}}
                },
                /* master key */
                {1, {{"DUMMY", 1, KBP_KEY_FIELD_PREFIX}}},
                NULL, /* ARAD_KBP_INSTRUCTION */
                NULL, /* ARAD_KBP_KEY */
            },
        };

static int Jer_kbp_default_result_sizes_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] = {
	ARAD_KBP_LUT_AD_TRANSFER_6B,
	ARAD_KBP_LUT_AD_TRANSFER_4B,
	ARAD_KBP_LUT_AD_TRANSFER_3B,
	ARAD_KBP_LUT_AD_TRANSFER_2B,
	ARAD_KBP_LUT_AD_TRANSFER_4B,
	ARAD_KBP_LUT_AD_TRANSFER_4B
};

static int Jer_kbp_default_result_formats_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES]= {
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD
};

static int Jer_kbp_default_result_sizes_ipv4_dc_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] = {
	ARAD_KBP_LUT_AD_TRANSFER_6B,
	ARAD_KBP_LUT_AD_TRANSFER_1B,
	ARAD_KBP_LUT_AD_TRANSFER_12B,
	ARAD_KBP_LUT_AD_TRANSFER_4B,
	ARAD_KBP_LUT_AD_TRANSFER_4B,
	0
};

static int Jer_kbp_default_result_formats_ipv4_dc_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES]= {
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_128B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_NO_INDEX_NO_AD
};

static int Jer_kbp_default_result_sizes_ipv4_dc_24_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES] = {
    ARAD_KBP_LUT_AD_TRANSFER_6B,
    ARAD_KBP_LUT_AD_TRANSFER_3B,
    ARAD_KBP_LUT_AD_TRANSFER_6B,
    ARAD_KBP_LUT_AD_TRANSFER_5B,
    ARAD_KBP_LUT_AD_TRANSFER_4B,
    ARAD_KBP_LUT_AD_TRANSFER_4B
};

static int Jer_kbp_default_result_formats_ipv4_dc_24_static[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES]= {
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_64B_AD,
	NLM_ARAD_INDEX_AND_32B_AD,
	NLM_ARAD_INDEX_AND_32B_AD
};

#endif /* BCM_88660_A0 */

/* mapping DB type to LTR id */
static
    ARAD_KBP_FRWRD_IP_LTR
        Arad_kbp_db_type_to_ltr[ARAD_KBP_MAX_NUM_OF_FRWRD_DBS] = {
            ARAD_KBP_FRWRD_LTR_IPV4_UC,
            ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF,
            ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF, 
            ARAD_KBP_FRWRD_LTR_IPV6_UC,
            ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF_2PASS,
			ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF,
            ARAD_KBP_FRWRD_LTR_IPV6_MC_RPF,
            ARAD_KBP_FRWRD_LTR_LSR, 
            ARAD_KBP_FRWRD_LTR_TRILL_UC,
            ARAD_KBP_FRWRD_LTR_TRILL_MC,
            ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED,
            ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_IP,
            ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_IP_WITH_RPF,
            ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_LSR,
            ARAD_KBP_FRWRD_LTR_IP_LSR_EXTENDED_IPV6,
            ARAD_KBP_FRWRD_LTR_IP_LSR_EXTENDED_P2P
        };

static kbp_warmboot_t kbp_warmboot_data[SOC_SAND_MAX_DEVICE];

static 
    genericTblMgrAradAppData 
        *AradAppData[SOC_SAND_MAX_DEVICE];

uint8 hit_bit_result_for_do_search = 0;

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/* 
 *  Utility functions
 */


/* 
 *  Test functions
 */

/* This function takes unsigned integer, puts each byte into array pointed by 'data' */
STATIC void WriteValueToBitVector4( uint8 *data, uint32 value )
{
      data[ 0 ] = ( uint8 )( 0xFF & ( value >> 24 ) );
      data[ 1 ] = ( uint8 )( 0xFF & ( value >> 16 ) );
      data[ 2 ] = ( uint8 )( 0xFF & ( value >> 8 ) );
      data[ 3 ] = ( uint8 )( 0xFF & ( value >> 0 ) );
}

void FillRecordPatternFor_80B0_Table(uint8 *data_p, uint8 *mask_p, uint32 iter)
{
    /* 0.0.32b <source/dest_IP>.X.X.X.X */

    sal_memset( data_p, 0, 10 );
    sal_memset( mask_p, 0, 10 );

    /* first 2B are 0, second 4B are source/dest IP, remaining 4B don't cares. MS 48b valid,
     * so mask the LS 32b
     */
    WriteValueToBitVector4(&data_p[2], iter);

    /* Mask off last 4 bytes */
    sal_memset(&mask_p[6], 0xFF, 4 );

    return;
}

/* This function fills the internal data strcture for tables with records */
STATIC void CreateRecordsInTables_nosmt(genericTblMgrAradAppData *refAppData_p,
    uint32 num_entries,
    uint32 record_base_tbl[4],
    uint32 ad_val_tbl[4] )
{
    tableInfo *tblInfo_p;
    tableRecordInfo *tblRecordInfo_p;
    uint32 alloc_size;
    uint8  iter_tbl, tblWidth_inBytes;
    uint8  *rec_data, *rec_mask;
    uint16  start, end, iter_group, iter_priority;
    uint32 loop = 0, start_value = 0, advlaue = 0;

    /* First allocate memory for storing records within the data structures.
     * Each table structure has start groupId and end groupId. Records of
     * all groupIds including the start-end groupId are added. With each
     * groupId, priorities of the records would range from [0 TO groupId-1].
     */
     for( iter_tbl = 0; iter_tbl < ARAD_KBP_FRWRD_IP_NOF_TABLES; iter_tbl++ )
     {
         if (NULL == refAppData_p->g_gtmInfo[iter_tbl].tblInfo.db_p) {
            /* This table is not initialized, continue to next table */
            continue;
        }

        tblInfo_p = &refAppData_p->g_gtmInfo[iter_tbl].tblInfo;

        start = tblInfo_p->groupId_start;
        end = tblInfo_p->groupId_end;

        tblInfo_p->tbl_size = num_entries;
        alloc_size = tblInfo_p->tbl_size;
        tblInfo_p->tblRecordInfo_p = arad_kbp_alloc(alloc_size, sizeof( tableRecordInfo ) );

        tblInfo_p->max_recCount    = alloc_size;
         tblWidth_inBytes = (uint8)(tblInfo_p->tbl_width / 8);

        rec_data = arad_kbp_alloc(1, tblWidth_inBytes );
        rec_mask = arad_kbp_alloc(1, tblWidth_inBytes );

        start_value = record_base_tbl[iter_tbl]; /* dest  Ip: table-0 */
        advlaue = ad_val_tbl[iter_tbl];      /* 64b  ad:         */

        /* store the records */
        iter_group = start;
        iter_priority = 0;
        tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;

        for(loop = 0; loop < tblInfo_p->tbl_size; loop++)
        {
            tblRecordInfo_p->groupId  = iter_group;
            tblRecordInfo_p->priority = iter_priority;

            tblRecordInfo_p->record.m_data = arad_kbp_alloc(1, tblWidth_inBytes );
            tblRecordInfo_p->record.m_mask = arad_kbp_alloc(1, tblWidth_inBytes );
            tblRecordInfo_p->record.m_len  = tblInfo_p->tbl_width;

            /* Generate the ACL record here */
            FillRecordPatternFor_80B0_Table(rec_data, rec_mask, (start_value + loop));

            /* associated data : msb32b */
            WriteValueToBitVector4(&tblRecordInfo_p->assoData[0], (advlaue + loop));

            if( tblInfo_p->tbl_id  == 0) /* lsb 32b associated data */
                WriteValueToBitVector4(&tblRecordInfo_p->assoData[4], (advlaue + loop));

            sal_memcpy( tblRecordInfo_p->record.m_data, rec_data, tblWidth_inBytes );
            sal_memcpy( tblRecordInfo_p->record.m_mask, rec_mask, tblWidth_inBytes );

            tblRecordInfo_p++;

#ifdef WITH_INDEX_SHUFFLE
            iter_priority++;

            /* loop back priority */
            if( (iter_priority % end) == 0)
            {
                iter_priority = start;
                iter_group++;
            }

            /* loop back group */
            if( iter_group && ((iter_group % end) == 0))
            {
                iter_group = start;
            }
#else
            iter_group++;
            iter_priority++;

            /* loop back priority */
            if( (iter_priority % end) == 0)
            {
                iter_priority = start;
            }

            /* loop back group */
            if( (iter_group % end) == 0)
            {
                iter_group = start;
            }
#endif
        }

        /* Free the memory allocated for temp data and mask. A fresh memory will be
         * based on the table width
         */
        arad_kbp_free(rec_data );
        arad_kbp_free(rec_mask );

     } /* end of table's loop */
}


/* Adds records into various tables */
STATIC int AddRecordsIntoTables(
                genericTblMgrAradAppData *refAppData_p,
                uint8 instance
                )
{
    int res;
    tableInfo *tblInfo_p;
    tableRecordInfo    *tblRecordInfo_p;
    uint32 iter_rec, num_recs;
    struct kbp_ad_db *ad_db_p;
    struct kbp_entry *dummy_entry;

    tblInfo_p = &refAppData_p->g_gtmInfo[instance].tblInfo;
    tblRecordInfo_p = tblInfo_p->tblRecordInfo_p;
    num_recs  = tblInfo_p->max_recCount;

    LOG_INFO(BSL_LS_SOC_TCAM,
             (BSL_META("\n Adding records into table = %d\n"),instance));
    for(iter_rec = 0; iter_rec < num_recs; iter_rec++ )
    {
        struct kbp_ad *ad_entry;

        /* Add record now */
        res = kbp_db_add_ace(tblInfo_p->db_p, tblRecordInfo_p->record.m_data, tblRecordInfo_p->record.m_mask,
                             tblRecordInfo_p->priority, &dummy_entry);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                    (BSL_META("Error in %s(): kbp_db_add_ace failed: %s\n"), FUNCTION_NAME(), kbp_get_status_string(res)));
        }

        /* Add the AD */
        if(tblInfo_p->tbl_assoWidth){
            ad_db_p = refAppData_p->g_gtmInfo[instance].tblInfo.ad_db_p;
            res = kbp_ad_db_add_entry(ad_db_p,tblRecordInfo_p->assoData,&ad_entry);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                        (BSL_META("Error in %s(): kbp_ad_db_add_entry failed: %s\n"), FUNCTION_NAME(), kbp_get_status_string(res)));
            }
            res = kbp_entry_add_ad(tblInfo_p->db_p,dummy_entry,ad_entry);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                        (BSL_META("Error in %s(): kbp_entry_add_ad failed: %s\n"), FUNCTION_NAME(), kbp_get_status_string(res)));
            }
        }

        /* install the entry */
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
soc_sand_ll_timer_set("ARAD_KBP_TIMERS_ADD_RECORDS_INTO_TABLES", ARAD_KBP_TIMERS_ADD_RECORDS_INTO_TABLES);
#endif 
        res = kbp_db_install(tblInfo_p->db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                    (BSL_META("Error in %s(): kbp_db_install failed: %s\n"), FUNCTION_NAME(), kbp_get_status_string(res)));
        }
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
soc_sand_ll_timer_stop(ARAD_KBP_TIMERS_ADD_RECORDS_INTO_TABLES);
#endif 


        /* Advance the record pointer */
        tblRecordInfo_p++;
        tblInfo_p->rec_count++;

        if( (tblInfo_p->rec_count % 500) == 0 ) {
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META("\n\t    - %u records added to table \n"),tblInfo_p->rec_count));
        }

    } /* end of for loop */

    LOG_INFO(BSL_LS_SOC_TCAM,
             (BSL_META("\t   Total number of records added [%u] to table\n"), tblInfo_p->rec_count));

    return SOC_SAND_OK;
}

#define SRCH_ENABLE

#ifdef SRCH_ENABLE
/* 2x parallel search of SrcIP(table-0) and DstIP(table-1) tables using LTR 0 */
STATIC uint32 Perform_LTR_Searches(int unit,
                                genericTblMgrAradAppData *refAppData_p,
                                uint32 nof_searches,
                                uint32 tbl_id,
                                uint32 db_type,
                                uint8 ltr_num)
{
    tableInfo   *tblInfo_p[4];
    tableRecordInfo   *tblRecordInfo_p[4];
    alg_kbp_rec *tblRecord_p[4];
    uint32 iter, numRecs, res;
    uint8  i=0;
    int32_t index1 = -1,priority1 = -1,index2 = -1,priority2 = -1;
    tableRecordInfo *tblRecHoldInfo_p[4]; /* reference for random prefixes */
    struct kbp_entry *entry_in_db1 = NULL,*entry_in_db2 = NULL;
    uint8_t db_key[80] = {0,};
    arad_kbp_rslt exp_result;

    /* Device Manager declarations */
    struct kbp_search_result cmp_rslt;
    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};
    uint8_t mkey[80] = { 0, };
	
	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    sal_memset(tblInfo_p, 0x0, sizeof(tableInfo*) * 4);
    sal_memset(tblRecordInfo_p, 0x0, sizeof(tableRecordInfo*) * 4);
    sal_memset(tblRecord_p, 0x0, sizeof(alg_kbp_rec*) * 4);
    sal_memset(tblRecHoldInfo_p, 0x0, sizeof(tableRecordInfo*) * 4);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (i=0; i < nof_searches; i++) {

        tblInfo_p[i] = &refAppData_p->g_gtmInfo[tbl_id+i].tblInfo;

        /* expected search results */
        tblRecHoldInfo_p[i] = tblRecordInfo_p[i] = tblInfo_p[i]->tblRecordInfo_p;
    }

    numRecs = tblInfo_p[0]->max_recCount; /* all records count is same */

    LOG_INFO(BSL_LS_SOC_TCAM,
             (BSL_META_U(unit,
                         "\n\tPerforming LTR#%d searches\n"), ltr_num));

    iter = 0;

    /* MasterKey[79:0] is constructed from table0/1 records */
    for( iter = 0; iter < numRecs; iter++ )
    {
        /* NlmCmFile__printf("\n\tStart     - %u keys are searched ", iter); */
        sal_memset(&cmp_rslt,0,sizeof(struct kbp_search_result));
        sal_memset(&exp_result,0,sizeof(arad_kbp_rslt));
        sal_memset(mkey,0,80);

        /* Expected results */
        for(i = 0; i < nof_searches; i++)
        {
            exp_result.m_resultValid[i] = 1;
            exp_result.m_hitOrMiss[i]   = 1;
            exp_result.m_hitDevId[i]    = 0;
            exp_result.m_hitIndex[i]    = tblRecordInfo_p[i]->index;
        }

        /* construct the master key 79:0  <16B_0's><32b_sourceIP><32b_destinationIP> */
        switch (ltr_num) {
        case ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF:

            exp_result.m_respType[0] = 0x2;
            exp_result.m_respType[1] = 0x1;

            /* using 64bit AD XPT format, but KBP LUT configuratoin is only for 48 bit. then copy 48bit from bit 16 */
            sal_memcpy(&exp_result.m_AssocData[0][2], &tblRecordInfo_p[0]->assoData[2], 6);

            /* using 32bit AD XPT format, but KBP LUT configuratoin is only for 16 bit. then copy 16bit from bit 16 */
            sal_memcpy(&exp_result.m_AssocData[1][2], &tblRecordInfo_p[1]->assoData[2], 2);

            tblRecord_p[0] = &(tblRecordInfo_p[0]->record);
            sal_memcpy(&mkey[0],tblRecord_p[0]->m_data, 10);

            tblRecord_p[1] = &(tblRecordInfo_p[1]->record);
            sal_memcpy(&mkey[6],tblRecord_p[1]->m_data, 4);
            break;

        case ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF:

            exp_result.m_respType[0] = 0x1;

            /* using 32bit AD XPT format, but KBP LUT configuratoin is only for 16 bit. then copy 16bit from bit 16 */
            sal_memcpy(&exp_result.m_AssocData[0][1], &tblRecordInfo_p[0]->assoData[1], 3);

            tblRecord_p[0] = &(tblRecordInfo_p[0]->record);
            sal_memcpy(&mkey[0],tblRecord_p[0]->m_data, 9);
            break;

        default:
            break;
        }

        /* Search the S/W*/
         {
             i = 0;
             sal_memcpy(db_key,tblRecord_p[i]->m_data,10);
             res = kbp_db_search(refAppData_p->g_gtmInfo[i].tblInfo.db_p,db_key, &entry_in_db1, &index1, &priority1);
             if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                 LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                 "Error in %s(): kbp_db_search failed index %d: %s\n"), FUNCTION_NAME(), i+1, kbp_get_status_string(res)));
             }
             LOG_INFO(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit,
                                  "\n\t- Search result for db = %d,index = %x\n"), i,index1));

             ++i;
             sal_memset(db_key,0,10);
             sal_memcpy(db_key,tblRecord_p[i]->m_data,10);
             res = kbp_db_search(refAppData_p->g_gtmInfo[i].tblInfo.db_p,db_key, &entry_in_db2, &index2, &priority2);
             if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                 LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                 "Error in %s(): kbp_db_search failed index %d: %s\n"), FUNCTION_NAME(), i+1, kbp_get_status_string(res)));
             }

             LOG_INFO(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit,
                                  "\n\t- Search result for db = %d,index = %x\n"), i,index2));
         }

        /*fire searches */
        KBP_STRY(kbp_instruction_search(ltr_config_info.inst_p, mkey, 0, &cmp_rslt));

        for(i = 0; i < nof_searches; i++) {
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "\n Hit/miss of search = %d is %x, index  = %x\n"),i,cmp_rslt.hit_or_miss[i],cmp_rslt.hit_index[i]));
        }

        for(i = 0; i<nof_searches;i++)
        {
            uint16_t j = 0;
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "\n Actual AD for Search = %d\n"),i));
            for(j = 0;j<16;j++)
            {
                LOG_INFO(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "%d_"),cmp_rslt.assoc_data[i][j]));
            }
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "\n")));
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "\n Expected AD for Search = %d\n"),i));
            for(j = 0;j<16;j++)
            {
                LOG_INFO(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "%d_"),exp_result.m_AssocData[i][j]));
            }
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "\n")));
        }

        if( iter && ((iter % 500) == 0 ) )
            LOG_INFO(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "\n\t     - %u keys are searched "), iter));


        /* NlmCmFile__printf("\n\tEnd     - %u keys are searched ", iter); */
    }

    LOG_INFO(BSL_LS_SOC_TCAM,
             (BSL_META_U(unit,
                         "\n\t   Total number of keys searched [%u]\n"), iter));
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in Perform_LTR_Searches()", 0, 0);
}
#endif

uint32 arad_kbp_test_ip4_rpf_NlmGenericTableManager(
    int unit,
    uint32 num_entries,
    uint32 record_base_tbl[4],
    uint32 ad_val_tbl[4]
    )
{
    uint8  iter_tbl;
    genericTblMgrAradAppData *refAppData_p = AradAppData[unit];

    /* Create records to be inserted into various tables. */
    LOG_INFO(BSL_LS_SOC_TCAM,
             (BSL_META_U(unit,
                         "\n Creating the records to add to table\n")));
    CreateRecordsInTables_nosmt(
        refAppData_p,
        num_entries,
        record_base_tbl,
        ad_val_tbl);

    /* Add records to tables now */
    for( iter_tbl = 0; iter_tbl < ARAD_KBP_FRWRD_IP_NOF_TABLES; iter_tbl++ )
    {
        /* Verify this table is initialized before adding records */
        if (NULL != refAppData_p->g_gtmInfo[iter_tbl].tblInfo.db_p) {
           AddRecordsIntoTables( refAppData_p,iter_tbl);
        }
    }

    /* perform searches */
#ifdef  SRCH_ENABLE
    if (NULL != refAppData_p->g_gtmInfo[ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0].tblInfo.db_p)
    {
        Perform_LTR_Searches(
           unit,
           refAppData_p,
           2,
           ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0,
           ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC_RPF ,
           ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF
        );
    }

    if (NULL != refAppData_p->g_gtmInfo[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].tblInfo.db_p)
    {
        Perform_LTR_Searches(
           unit,
           refAppData_p, 
           1, 
           ARAD_KBP_FRWRD_TBL_ID_IPV4_MC, 
           ARAD_KBP_FRWRD_DB_TYPE_IPV4_MC_RPF,
           ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF
        );
    }
#endif
    return 0x0;
}

/* Test ROP packet write and read */
int arad_kbp_init_rop_test(int unit, uint32 core){
    uint32
       res,  
       addr, nbo_addr;
    uint8 data[10] = ARAD_KBP_ROP_TEST_DATA;
    arad_kbp_rop_write_t wr_data;
    arad_kbp_rop_read_t rd_data;

    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311)
        addr = 0xE; /* scratch pad register address */
    else
        addr = ARAD_KBP_ROP_TEST_REG_ADDR;

    nbo_addr = soc_htonl(addr);
    sal_memcpy(wr_data.addr, &nbo_addr, sizeof(uint32));
    sal_memset(wr_data.mask, 0x0, NLM_DATA_WIDTH_BYTES);
    sal_memset(wr_data.addr_short, 0x0, NLMDEV_REG_ADDR_LEN_IN_BYTES);
    wr_data.writeMode = NLM_ARAD_WRITE_MODE_DATABASE_DM;
    
    /* Set wr_data */
    ARAD_KBP_ROP_REVERSE_DATA(data, wr_data.data, 10);
    
    /* Set rd_data */
    sal_memcpy(rd_data.addr, &nbo_addr, sizeof(uint32));
    rd_data.dataType = NLM_ARAD_READ_MODE_DATA_X;
    
    /* ROP write and ROP read */
    res = arad_kbp_rop_write(unit, core, &wr_data);
    if (res != 0) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "%s(): arad_kbp_rop_write failed\n"), FUNCTION_NAME()));
        return SOC_E_FAIL;
    }

    res = arad_kbp_rop_write(unit, core, &wr_data);
    if (res != 0) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "%s(): arad_kbp_rop_write failed\n"), FUNCTION_NAME()));
        return SOC_E_FAIL;
    }

    res = arad_kbp_rop_read(unit, core, &rd_data);
    if (res!=0) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "%s(): arad_kbp_rop_read failed\n"), FUNCTION_NAME()));
        return SOC_E_FAIL;
    }
    /* Comparing Read data with Written data */
    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        /* Scratch PD is only 8 bytes in Optimus Prime */
        res = sal_memcmp(rd_data.data+3 ,wr_data.data + 2, 8);
    } else {
    res = sal_memcmp(rd_data.data+1 ,wr_data.data, 10);
    }

    if (res != 0) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "%s(): arad_kbp_rop write-read failed. read data does no match expected data\n"), 
                              FUNCTION_NAME()));
        return SOC_E_FAIL;
    }
    return SOC_E_NONE;
}

STATIC uint32
	arad_kbp_init_static_tables(int unit)
{
	int i,tbl_idx;
	int res;
    int num_tbl_valid = 0;

	uint32 table_size;
    ARAD_KBP_TABLE_CONFIG Arad_kbp_table_config_info = Arad_kbp_dummy_table_config_info_static;
	
	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	/* set the frwrd tables */
	if (SOC_IS_ARADPLUS(unit)) {
		for (i = 0; i < ARAD_KBP_FRWRD_IP_NOF_TABLES_ARAD_PLUS; i++) {           
            if(ARAD_KBP_IPV4DC_24BIT_FWD) {
                /* In case of IPv4 DC 24bit forwarding, change the table AD width to 24 bits */
                Arad_plus_kbp_table_config_info_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_DC].tbl_asso_width = NLM_TBL_ADLEN_24B;
            }
			res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.set(unit, i, &Arad_plus_kbp_table_config_info_static[i]); 
			SOC_SAND_CHECK_FUNC_RESULT(res, 200+i, exit);
		}
	}else{
		for (i = 0; i < ARAD_KBP_FRWRD_IP_NOF_TABLES_ARAD; i++) {
			res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.set(unit, i, &Arad_kbp_table_config_info_static[i]); 
			SOC_SAND_CHECK_FUNC_RESULT(res, 200+i, exit);
		}
	}

	/* set the dummy tables */
	for (i = 0 ; i < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; i++) {  
		Arad_kbp_table_config_info.tbl_id = ARAD_KBP_FRWRD_TBL_ID_DUMMY_0+i;
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.set(unit, ARAD_KBP_FRWRD_TBL_ID_DUMMY_0+i, &Arad_kbp_table_config_info); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 200+i, exit);
	}

	/* First set table sizes according to configuration */
	for ( tbl_idx = 0;tbl_idx < ARAD_KBP_FRWRD_IP_NOF_TABLES; tbl_idx++ )
	{
        /* skip dummy forwarding table */
        if(tbl_idx == ARAD_KBP_FRWRD_TBL_ID_DUMMY_FRWRD) {
            continue;
        }

		table_size = SOC_DPP_CONFIG(unit)->arad->init.elk.fwd_table_size[tbl_idx];
        if (table_size > 0x0) 
        {
			/* support dynamic allocation*/
			if (table_size == 1) {
				table_size = 0;
			}

			/* exceptional cases:
			 * IPV4MC/IPV6MC - use dummy tables to support RPF on lookup#2
			 */
            if (SOC_IS_ARADPLUS(unit)) {
                if (tbl_idx == ARAD_KBP_FRWRD_TBL_ID_IPV4_MC) {
    				res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit,ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC, TRUE); 
    				SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
    			}
    			if (tbl_idx == ARAD_KBP_FRWRD_TBL_ID_IPV6_MC) {
    				res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit,ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC, TRUE); 
    				SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
    			}
            }
			/* set the table size and validity*/
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit,tbl_idx, TRUE); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_size.set(unit, tbl_idx, table_size); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 260, exit);

            /* Disable the IPv4 RPF table for some combination of SOC property */
            if (tbl_idx == ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1) {
                if (soc_property_get(unit, spn_EXT_IP4_FWD_TABLE_SIZE, 0x0) 
                    && (soc_property_get(unit, spn_EXT_IP4_UC_RPF_FWD_TABLE_SIZE, 0x0) == 0)) {
                    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_size.set(unit, tbl_idx, 0); 
                    SOC_SAND_CHECK_FUNC_RESULT(res, 280, exit);
                }
            }
            num_tbl_valid++;
		}
	}

    /* no valid tables; enable DUMMY_FRWRD table */
    if(num_tbl_valid == 0) {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit, ARAD_KBP_FRWRD_TBL_ID_DUMMY_FRWRD, TRUE); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
    }

	/* Init Acls empty tables*/
    ARAD_KBP_TABLE_CONFIG_clear(&Arad_kbp_table_config_info);
	for ( tbl_idx = ARAD_KBP_ACL_TABLE_ID_OFFSET; 
		  tbl_idx < ARAD_KBP_MAX_NUM_OF_TABLES; 
		  tbl_idx++) 
	{
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.set(unit, tbl_idx, &Arad_kbp_table_config_info); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 50+tbl_idx, exit);
	}

exit:
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_init_static_tables()", 0, 0);

}

STATIC uint32
	arad_kbp_init_program_db_default_fields(
	   int 								unit,
	   arad_kbp_lut_data_t				*gtm_lut_info,
	   ARAD_KBP_GTM_OPCODE_CONFIG_INFO 	*gtm_opcode_config_info)
{

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);
	SOC_SAND_CHECK_NULL_INPUT(gtm_lut_info);
	SOC_SAND_CHECK_NULL_INPUT(gtm_opcode_config_info);

	/* GTM opcode config */
	gtm_opcode_config_info->rx_data_type = GTM_OPCODE_CONFIG_DATA_TYPE_RX_REPLY;
	gtm_opcode_config_info->tx_data_type = GTM_OPCODE_CONFIG_DATA_TYPE_TX_REQUEST;

	/* GTM LUT info */
	gtm_lut_info->rec_type 			= ARAD_KBP_LUT_REC_TYPE_REQUEST;    
	gtm_lut_info->rec_is_valid 		= 0x1;
	gtm_lut_info->mode 				= ARAD_KBP_LUT_INSTR_LUT1_CONTEXID_SEQ_NUM_MODE;
	gtm_lut_info->key_config 		= ARAD_KBP_LUT_KEY_CONFIG_SEND_INCOMING_DATA;
	gtm_lut_info->lut_key_data 		= 0;
	gtm_lut_info->key_w_cpd_gt_80 	= 0;
	gtm_lut_info->copy_data_cfg 	= 0;

exit:
	SOC_SAND_EXIT_AND_SEND_ERROR("error in kbp_update_db_all_default_fields()", 0,0);

}

STATIC int
	arad_kbp_init_gtm_lut_result_by_index(
	   int 	unit,
	   arad_kbp_lut_data_t  *gtm_lut_info,
	   int *res_sizes,
	   int index,
	   uint8 valid)
{
	switch (index) {
	case 0:
		gtm_lut_info->result0_idx_or_ad  = valid == TRUE ? ARAD_KBP_LUT_TRANSFER_AD_ONLY : ARAD_KBP_LUT_TRANSFER_INDX_ONLY;
		gtm_lut_info->result0_idx_ad_cfg = valid == TRUE ? res_sizes[index] : ARAD_KBP_LUT_AD_TRANSFER_1B;
		break;
	case 1:
		gtm_lut_info->result1_idx_or_ad  = valid == TRUE ? ARAD_KBP_LUT_TRANSFER_AD_ONLY : ARAD_KBP_LUT_TRANSFER_INDX_ONLY;
		gtm_lut_info->result1_idx_ad_cfg = valid == TRUE ? res_sizes[index] : ARAD_KBP_LUT_AD_TRANSFER_1B;
		break;
	case 2:
		gtm_lut_info->result2_idx_or_ad  = valid == TRUE ? ARAD_KBP_LUT_TRANSFER_AD_ONLY : ARAD_KBP_LUT_TRANSFER_INDX_ONLY;
		gtm_lut_info->result2_idx_ad_cfg = valid == TRUE ? res_sizes[index] : ARAD_KBP_LUT_AD_TRANSFER_1B;
		break;
	case 3:
		gtm_lut_info->result3_idx_or_ad  = valid == TRUE ? ARAD_KBP_LUT_TRANSFER_AD_ONLY : ARAD_KBP_LUT_TRANSFER_INDX_ONLY;
		gtm_lut_info->result3_idx_ad_cfg = valid == TRUE ? res_sizes[index] : ARAD_KBP_LUT_AD_TRANSFER_1B;
		break;
	case 4:
		gtm_lut_info->result4_idx_or_ad  = valid == TRUE ? ARAD_KBP_LUT_TRANSFER_AD_ONLY : ARAD_KBP_LUT_TRANSFER_INDX_ONLY;
		gtm_lut_info->result4_idx_ad_cfg = valid == TRUE ? res_sizes[index] : ARAD_KBP_LUT_AD_TRANSFER_1B;
		break;
	case 5:
		gtm_lut_info->result5_idx_or_ad  = valid == TRUE ? ARAD_KBP_LUT_TRANSFER_AD_ONLY : ARAD_KBP_LUT_TRANSFER_INDX_ONLY;
		gtm_lut_info->result5_idx_ad_cfg = valid == TRUE ? res_sizes[index] : ARAD_KBP_LUT_AD_TRANSFER_1B;
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

STATIC uint32
	arad_kbp_init_static_ltr_db(int unit)
{
	int res;
	int db_ndx, nof_db, search_ndx;
	int search_to_copy;
	uint8 ltr_valid,tbl_valid;
	
	ARAD_KBP_LTR_CONFIG  kbp_ltr_config = {0};
	ARAD_KBP_LTR_CONFIG  *kbp_ltr_config_ptr;

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	if (SOC_IS_ARADPLUS(unit)) {
		kbp_ltr_config_ptr = Arad_plus_kbp_ltr_config_static;
		nof_db = ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD_PLUS;
	}else{
		kbp_ltr_config_ptr = Arad_kbp_ltr_config_static;
		nof_db = ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD;
	}

	for (db_ndx = 0; db_ndx < nof_db; db_ndx++) {
		kbp_ltr_config = *(kbp_ltr_config_ptr + db_ndx);

		search_to_copy = 0;
		ltr_valid = 1;
		tbl_valid = 0;
		for (search_ndx = 0; search_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; search_ndx++) {
			/* Per lookup configuration*/
			if (!SHR_BITGET(&(kbp_ltr_config.parallel_srches_bmp), search_ndx)) {
				/* LTR config info*/
				kbp_ltr_config.ltr_search[search_ndx].nof_key_segments = 0;
				kbp_ltr_config.search_type[search_ndx] 	= ARAD_KBP_NOF_DB_TYPES;
				kbp_ltr_config.tbl_id[search_ndx]		= ARAD_KBP_FRWRD_TBL_ID_DUMMY_0+search_ndx;
			}else{
				/* LTR config info*/
				if ((search_ndx >= ARAD_KBP_CMPR3_FIRST_ACL) && !(SOC_IS_JERICHO(unit))) {
					SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit);
				}
				kbp_ltr_config.ltr_search[search_ndx]  	= kbp_ltr_config_ptr[db_ndx].ltr_search[search_to_copy];
				kbp_ltr_config.search_type[search_ndx] 	= kbp_ltr_config_ptr[db_ndx].search_type[search_to_copy];
				kbp_ltr_config.tbl_id[search_ndx]		= kbp_ltr_config_ptr[db_ndx].tbl_id[search_to_copy];

				res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.get(unit, kbp_ltr_config_ptr[db_ndx].tbl_id[search_to_copy], &tbl_valid); 
                SOC_SAND_CHECK_FUNC_RESULT(res, 290+search_ndx, exit);
				ltr_valid &= tbl_valid;

				search_to_copy++;
			}
		}

		/* Per DB configuration*/

		/* LTR config info*/
		if (kbp_ltr_config.parallel_srches_bmp == 0) {
			ltr_valid = 0;
		}
		kbp_ltr_config.valid = ltr_valid;

		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_ndx, &kbp_ltr_config);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);
	}

	/* Init ACLs DB with default init values*/
	ARAD_KBP_LTR_CONFIG_clear(&kbp_ltr_config);

	for ( db_ndx = ARAD_KBP_FRWRD_DB_ACL_OFFSET; db_ndx < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; db_ndx++) 
	{ 
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_ndx, &kbp_ltr_config); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 50+db_ndx, exit);
	}

exit:
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_init_static_db()", 0, 0);
}

STATIC uint32
	arad_kbp_set_all_db_by_ltr(int unit, int ltr_ndx)
{
	int i, res;
	int db_ndx,search_ndx;

	int *result_default_size;
	int *result_default_format;

	int *result_size;
	int *result_format;
	
	arad_kbp_lut_data_t				kbp_gtm_lut_info = {0};
	ARAD_KBP_LTR_CONFIG             kbp_ltr_config = {0};
	arad_kbp_frwd_ltr_db_t          kbp_gtm_ltr_info = {0};
	ARAD_KBP_GTM_OPCODE_CONFIG_INFO kbp_gtm_opcode_config_info = {0};

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	if (SOC_IS_JERICHO(unit)) {
		result_size   = Jer_kbp_default_result_sizes_static;
		result_format = Jer_kbp_default_result_formats_static;
	}else if (SOC_IS_ARADPLUS(unit)) {
		result_size = Arad_plus_kbp_default_result_sizes_static;
		result_format = Arad_plus_kbp_default_result_formats_static;
	} else {
      result_size = Arad_kbp_default_result_sizes_static;
      result_format = Arad_kbp_default_result_formats_static;
    }

	for (db_ndx = 0 ; db_ndx < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS ; db_ndx++ ) {
		if ((ltr_ndx >= 0) && (db_ndx != ltr_ndx)) {
			continue;
		}
		kbp_gtm_opcode_config_info.rx_data_size = 0;
		kbp_gtm_ltr_info.res_total_data_len = 0;

		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_ndx ,&kbp_ltr_config);
		SOC_SAND_CHECK_FUNC_RESULT(res, 290+search_ndx, exit);
		if (kbp_ltr_config.valid == FALSE) {
			if (ltr_ndx == -1) {
				continue;
			}else{
				EXIT_AND_ERR_PRINT("arad_kbp_set_all_db_by_ltr: Trying to set an invalid LTR DB\n",ltr_ndx,0);
			}
		}

		if (db_ndx != ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) {
			result_default_size = result_size;
			result_default_format = result_format;
		}else if (ARAD_KBP_IPV4DC_24BIT_FWD){
			result_default_size = Jer_kbp_default_result_sizes_ipv4_dc_24_static;
			result_default_format = Jer_kbp_default_result_formats_ipv4_dc_24_static;
        }else{
			result_default_size = Jer_kbp_default_result_sizes_ipv4_dc_static;
			result_default_format = Jer_kbp_default_result_formats_ipv4_dc_static;
		}

		for (search_ndx = 0; search_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; search_ndx++) {
			/* Per lookup configuration*/
			if (!SHR_BITGET(&(kbp_ltr_config.parallel_srches_bmp), search_ndx)) {
				/* GTM LTR info*/
				kbp_gtm_ltr_info.res_data_len[search_ndx]= SOC_IS_JERICHO(unit) ? result_default_size[search_ndx] : 0;
				kbp_gtm_ltr_info.res_format[search_ndx]  = NLM_ARAD_NO_INDEX_NO_AD;

				/* GTM LUT info*/
				res = arad_kbp_init_gtm_lut_result_by_index(unit, &kbp_gtm_lut_info, result_default_size, search_ndx, FALSE);
				SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

			}else{
				/* GTM LTR info*/
				kbp_gtm_ltr_info.res_data_len[search_ndx]= result_default_size[search_ndx];
				kbp_gtm_ltr_info.res_format[search_ndx]  = result_default_format[search_ndx];
				kbp_gtm_ltr_info.res_total_data_len	+= result_default_size[search_ndx];

				/* GTM LUT info*/
				res = arad_kbp_init_gtm_lut_result_by_index(unit, &kbp_gtm_lut_info, result_default_size, search_ndx, TRUE);
				SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

				/* GTM opcode config info*/
				kbp_gtm_opcode_config_info.rx_data_size += result_default_size[search_ndx];
			}
		}
		/* Per DB configuration*/

		/* GTM LTR info*/
		kbp_gtm_ltr_info.opcode = kbp_ltr_config.opcode;
		if (kbp_ltr_config.is_cmp3_search == NlmTRUE) {
			kbp_gtm_opcode_config_info.rx_data_size += result_default_size[ARAD_KBP_CMPR3_SKIPPED_SEARCH];
			res = arad_kbp_init_gtm_lut_result_by_index(unit, &kbp_gtm_lut_info, result_default_size, ARAD_KBP_CMPR3_SKIPPED_SEARCH, TRUE);
			SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);
		}
		if (kbp_gtm_ltr_info.res_total_data_len > 0) {
			kbp_gtm_ltr_info.res_total_data_len += 1;
		}

		/* GTM opcode config info*/
		kbp_gtm_opcode_config_info.tx_data_size = 0;
		for (i=0;i<kbp_ltr_config.master_key_fields.nof_key_segments;i++) {
			kbp_gtm_opcode_config_info.tx_data_size += kbp_ltr_config.master_key_fields.key_segment[i].nof_bytes;
		}
		if (kbp_gtm_opcode_config_info.tx_data_size > 0) {
			kbp_gtm_opcode_config_info.tx_data_size -= 1;
		}

		/* GTM LUT info*/
		kbp_gtm_lut_info.instr = kbp_ltr_config.is_cmp3_search == NlmTRUE 
															? ARAD_KBP_ROP_LUT_INSTR_CMP3_GET(kbp_ltr_config.ltr_id) 
															: ARAD_KBP_ROP_LUT_INSTR_CMP1_GET(kbp_ltr_config.ltr_id) ;

		kbp_gtm_lut_info.rec_size = kbp_gtm_opcode_config_info.tx_data_size + 1;

		res = arad_kbp_init_program_db_default_fields(unit,&kbp_gtm_lut_info,&kbp_gtm_opcode_config_info);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.set(unit, db_ndx, &kbp_gtm_lut_info); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 50+db_ndx, exit);
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.set(unit, db_ndx, &kbp_gtm_ltr_info); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 100+db_ndx, exit);
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.set(unit, db_ndx, &kbp_gtm_opcode_config_info); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 150+db_ndx, exit);
	}

exit:
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_set_all_db_ltr()", 0, 0);
}

/* 
 *  KBP db (SW) management functions
 */
uint32 
    arad_kbp_sw_init(
       int unit
    )
{
	int res;
    uint32 db_type,res_idx;
	
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

#ifdef BCM_88660
    if (SOC_IS_ARADPLUS(unit) && (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_rpf_fwd_parallel", 0) == 0)){
        if (ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED) {/* in this case the RPF table is primary table. */
            Arad_plus_kbp_table_config_info_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1].clone_of_tbl_id = ARAD_KBP_FRWRD_IP_NOF_TABLES;
        }
        if (ARAD_KBP_ENABLE_IPV6_EXTENDED){/* in this case the RPF table is primary table. */
            Arad_plus_kbp_table_config_info_static[ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1].clone_of_tbl_id = ARAD_KBP_FRWRD_IP_NOF_TABLES;            
            Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC].parallel_srches_bmp = 0;
            Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC_RPF].parallel_srches_bmp = 0;
            Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC_RPF_2PASS].parallel_srches_bmp = 0x3;
        }

		if (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_ipv4_mc_flexible_fwd_table", 0)) {
			Arad_plus_kbp_table_config_info_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].tbl_width = NLM_TBL_WIDTH_160;
            Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].ltr_search[0] = ipv4_mc_extended_search;

			/* adding inrif mapping search */
            Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].parallel_srches_bmp = 0x7;
            Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].ltr_search[2] = inrif_mapping_search; /* inrif mapping serach */
			Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].tbl_id[2] = ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING;	
			Arad_plus_kbp_ltr_config_static[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC].search_type[2] = ARAD_KBP_DB_TYPE_FORWARDING;
		}
    }
#endif /* BCM_88660 */

	res = arad_kbp_init_static_tables(unit);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

	res = arad_kbp_init_static_ltr_db(unit);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

	res = arad_kbp_set_all_db_by_ltr(unit, -1);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    sal_memset(arad_kbp_frwd_ltr_db, 0x0, sizeof(arad_kbp_frwd_ltr_db_t) * ARAD_KBP_ROP_LTR_NUM_MAX);

    for ( db_type = 0; db_type < ARAD_KBP_ROP_LTR_NUM_MAX; db_type++) {
        /* Clear LTR (KBP-instruction), LUT and ROP configurations */
        for ( res_idx = 0; res_idx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; res_idx++) {
            arad_kbp_frwd_ltr_db[db_type].res_format[res_idx] = NLM_ARAD_NO_INDEX_NO_AD;
        }
    }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_sw_init()", 0, 0);
}

STATIC uint8
	arad_kbp_result_sizes_is_default(int unit, int *result_actual_sizes, ARAD_KBP_FRWRD_IP_DB_TYPE db_type)
{
	int i;
	int *result_default_size; 

    if (db_type == ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) {
        if(ARAD_KBP_IPV4DC_24BIT_FWD) {
            result_default_size = Jer_kbp_default_result_sizes_ipv4_dc_24_static;
        } else {
            result_default_size = Jer_kbp_default_result_sizes_ipv4_dc_static;
        }
    } else {
        result_default_size = Jer_kbp_default_result_sizes_static;
    }

	for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
		if (result_actual_sizes[i] != result_default_size[i]) {
			return FALSE;
		}
	}
	return TRUE;
}

/* 
    Function: arad_kbp_result_sizes_configurations_init
 
    results_size should be equal to the actual number of bytes that is used to this lookup.
    if an ACL lookup is not exists but there is a reserved place of 24bits for it the value should be 3
    the valid bits indicates the lookups that is already configured in the ltr (Arad_kbp_ltr_config) FWD and RPF lookup
*/
uint32
    arad_kbp_result_sizes_configurations_init(int unit, ARAD_KBP_FRWRD_IP_DB_TYPE db_type, int results[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS])
{
    int i, res;
    int total_size_valid_lookups = 0;
	uint8 is_default, parallel_bitmap;
    NlmBool is_cmp3;
	int results_size[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS];

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (db_type > ARAD_KBP_FRWRD_DB_NOF_TYPES) {
        LOG_ERROR(BSL_LS_SOC_TCAM,(BSL_META_U(unit,"Error in %s(): invalid db_type %d\n"), 
                                   FUNCTION_NAME(), db_type));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
    }

	is_default = arad_kbp_result_sizes_is_default(unit, results, db_type);
	if (is_default) {
		ARAD_DO_NOTHING_AND_EXIT;
	}

	for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
        if (results[i] > 15) {
            LOG_ERROR(BSL_LS_SOC_TCAM,(BSL_META_U(unit,"Error in %s(): result_size not valid for ID %d size %d\n"), 
                       FUNCTION_NAME(), i, results[i]));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
        }
    }
    
	for (i=0;i<ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS;i++) {
		results_size[i] = results[i];
	}

	if (ARAD_KBP_ENABLE_IPV4_DC) {
        if(db_type == ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) {
            results_size[1] += ARAD_KBP_IPV4DC_RES1_PAD_BYTES;
            results_size[2] += ARAD_KBP_IPV4DC_RES3_PAD_BYTES;
        } else {
            results_size[0] += ARAD_KBP_IPV4DC_RES1_PAD_BYTES;
            results_size[3] += ARAD_KBP_IPV4DC_RES3_PAD_BYTES;
        }
	}

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.parallel_srches_bmp.get(unit,db_type,&parallel_bitmap);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* here we update the result size also for the ACLs that in between the FWD and the RPF */
	if (SHR_BITGET(&parallel_bitmap, 0)) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result0_idx_ad_cfg.set(unit, db_type, results_size[0]);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result0_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
	if (SHR_BITGET(&parallel_bitmap, 1)) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result1_idx_ad_cfg.set(unit, db_type, results_size[1]); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result1_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
	if (SHR_BITGET(&parallel_bitmap, 2)) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result2_idx_ad_cfg.set(unit, db_type, results_size[2]); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result2_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
	if (SHR_BITGET(&parallel_bitmap, 3)) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result3_idx_ad_cfg.set(unit, db_type, results_size[3]); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result3_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
	if (SHR_BITGET(&parallel_bitmap, 4)) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result4_idx_ad_cfg.set(unit, db_type, results_size[4]); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result4_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}
	if (SHR_BITGET(&parallel_bitmap, 5)) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result5_idx_ad_cfg.set(unit, db_type, results_size[5]); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result5_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}

    for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_data_len.set(unit, db_type, i, results_size[i]);
        SOC_SAND_CHECK_FUNC_RESULT(res, 100+i, exit);
		if (SHR_BITGET(&parallel_bitmap, i)) {
            total_size_valid_lookups += results_size[i];
            if (results_size[i] <= 4) {
                res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_format.set(unit, db_type, i, NLM_ARAD_INDEX_AND_32B_AD);
                SOC_SAND_CHECK_FUNC_RESULT(res, 100+i, exit);
            } else if (results_size[i] <= 8) {
                res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_format.set(unit, db_type, i, NLM_ARAD_INDEX_AND_64B_AD);
                SOC_SAND_CHECK_FUNC_RESULT(res, 100+i, exit);
            } else {
                res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_format.set(unit, db_type, i, NLM_ARAD_INDEX_AND_128B_AD);
                SOC_SAND_CHECK_FUNC_RESULT(res, 100+i, exit);
            }
        } else {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_format.set(unit, db_type, i, NLM_ARAD_ONLY_INDEX_NO_AD);
            SOC_SAND_CHECK_FUNC_RESULT(res, 100+i, exit);
        }
	}

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_total_data_len.set(unit, db_type, (1 + total_size_valid_lookups)); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.is_cmp3_search.get(unit,db_type,&is_cmp3);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

	if (is_cmp3 == NlmTRUE) {
		total_size_valid_lookups += results_size[ARAD_KBP_CMPR3_SKIPPED_SEARCH];
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result1_idx_ad_cfg.set(unit, db_type, results_size[ARAD_KBP_CMPR3_SKIPPED_SEARCH]); 
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.result1_idx_or_ad.set(unit, db_type, ARAD_KBP_LUT_TRANSFER_AD_ONLY);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
	}

	/* here we need to update the result size also for the ACLs that in between the FWD and the RPF */
    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.rx_data_size.set(unit, db_type, total_size_valid_lookups); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_result_sizes_configurations_init", 0, db_type);
}

STATIC uint32 
    arad_kbp_db_create_verify(
       SOC_SAND_IN  ARAD_KBP_TABLE_CONFIG   *sw_table
   )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);

    SOC_SAND_CHECK_NULL_INPUT(sw_table);

    /* Verify table attributes */
    SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(sw_table->bankNum, NLMDEV_BANK_0, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
    SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(sw_table->group_id_start, 0, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit);
    SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(sw_table->group_id_end, 0, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 30, exit);

    /* Table width must be one of the following */
    if(sw_table->tbl_width != NLM_TBL_WIDTH_80 &&
       sw_table->tbl_width != NLM_TBL_WIDTH_160 &&
       sw_table->tbl_width != NLM_TBL_WIDTH_320 &&
       sw_table->tbl_width != NLM_TBL_WIDTH_640 )
    {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 40, exit);
    }

    /* Associated width must be one of the following */
    if(sw_table->tbl_asso_width != NLM_TBL_ADLEN_ZERO &&
       sw_table->tbl_asso_width != NLM_TBL_ADLEN_24B &&
       sw_table->tbl_asso_width != NLM_TBL_ADLEN_32B &&
       sw_table->tbl_asso_width != NLM_TBL_ADLEN_48B &&
       sw_table->tbl_asso_width != NLM_TBL_ADLEN_64B &&
       sw_table->tbl_asso_width != NLM_TBL_ADLEN_128B )
    {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 50, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_db_create_verify()", 0, 0);
}

STATIC
uint32 
    arad_kbp_db_create(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint32                      table_id,
       SOC_SAND_IN  ARAD_KBP_TABLE_CONFIG       *sw_table,
       SOC_SAND_IN  uint32                      flags,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
   )
{
    uint32 
        res = SOC_SAND_OK;

    nlm_u8 valid = FALSE;

    uint8 
        check_only;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(sw_table);

    check_only = (flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY) ? TRUE : FALSE;
    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
    
    /* Verify table attributes */
    res = arad_kbp_db_create_verify(sw_table);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Search for the next valid table */
    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.get(unit, table_id, &valid); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    if(valid == FALSE)
    {
        if (!check_only) 
        {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.set(unit, table_id, sw_table); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit, table_id, TRUE); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_id.set(unit, table_id, table_id); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

        }
        *success = SOC_SAND_SUCCESS;
    }
    else
    {
        /* Table already configured */
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 30, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_db_create()", 0, 0);
}

/*
* Converting qualifier to string. 
* If not a KBP known qualifier, use the fp qualifiers names 
* IPV6 and IPV4 SIP/DIP habe to be "SIP" "DIP" in order to support sip dip sharing  
*/
STATIC
 void ARAD_KBP_QUAL_TYPE_to_string(
	SOC_SAND_IN int 					unit,
	SOC_SAND_IN SOC_PPC_FP_QUAL_TYPE 	qual_type,
	SOC_SAND_OUT  char*					qual_name
  )
{
	switch(qual_type){
	case SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP:
		sal_strcpy(qual_name, "DIP");
		break;
	case SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP:
		sal_strcpy(qual_name, "SIP");
		break;
	case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_HIGH:
	case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_DIP_LOW:
		sal_strcpy(qual_name, "DIP");
		break;
	case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_HIGH:
	case SOC_PPC_FP_QUAL_HDR_FWD_IPV6_SIP_LOW:
		sal_strcpy(qual_name, "SIP");
		break;
	case SOC_PPC_FP_QUAL_IRPP_VRF:
		sal_strcpy(qual_name, "VRF");
		break;
	case SOC_PPC_FP_QUAL_IRPP_IN_RIF:
		sal_strcpy(qual_name, "IN-RIF");
		break;
	case SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD:
		sal_strcpy(qual_name, "MPLS-KEY");
		break;
	default:
		sal_strcpy(qual_name, SOC_PPC_FP_QUAL_TYPE_to_string(qual_type));
		break;
	}
}

/*
* Converting qualifier array to LTR single search structure. 
* Used for master key and lookup keys (LTR and tables) 
*/
STATIC
 uint32 arad_kbp_convert_quals_to_segments(
	SOC_SAND_IN int							unit,
	SOC_SAND_IN SOC_DPP_DBAL_QUAL_INFO		qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
	SOC_SAND_IN int							nof_qualifiers,
	SOC_SAND_OUT ARAD_KBP_LTR_SINGLE_SEARCH *master_key)
{
	int qual_ndx;
	int segment_ndx = -1;
	int qual_length_in_bits=0;
	SOC_PPC_FP_QUAL_TYPE qual_type;
	char next_segment_name[20];
	char segment_name[20];
	int segment_length_in_bits=0;
	int zeroes_ones_padding_length = 0;

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	sal_memset(master_key, 0x0, sizeof(ARAD_KBP_LTR_SINGLE_SEARCH));
	sal_strcpy(segment_name,"");
	sal_strcpy(next_segment_name,"");

	for (qual_ndx = nof_qualifiers-1; qual_ndx >= 0 ; qual_ndx--) {
		qual_length_in_bits = qual_info[qual_ndx].qual_nof_bits == 0 ? qual_info[qual_ndx].qual_full_size : qual_info[qual_ndx].qual_nof_bits;
		qual_type = qual_info[qual_ndx].qual_type;
		if ((qual_type == SOC_PPC_FP_QUAL_IRPP_ALL_ZEROES)||(qual_type == SOC_PPC_FP_QUAL_IRPP_ALL_ONES)) {
			zeroes_ones_padding_length = qual_length_in_bits;
			continue;
		}
		ARAD_KBP_QUAL_TYPE_to_string(unit, qual_type, next_segment_name);
		if (sal_strcmp(segment_name,next_segment_name) != 0) {
			/* New segment start*/
			if (segment_ndx != -1) {
				/* write the previous segment to the master key/lookup key*/
				master_key->nof_key_segments++;
				sal_strcpy(master_key->key_segment[segment_ndx].name,segment_name);
				if ((segment_length_in_bits & 0x7) != 0) {
					SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 3, exit);
				}
				master_key->key_segment[segment_ndx].nof_bytes = (segment_length_in_bits)/8;
                if((SOC_DPP_CONFIG(unit)->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) && !(sal_strcmp("VRF", segment_name))) {
                    master_key->key_segment[segment_ndx].type = KBP_KEY_FIELD_EM;
                } else {
                    master_key->key_segment[segment_ndx].type = KBP_KEY_FIELD_PREFIX;
                }
			}
			segment_ndx++;
			segment_length_in_bits = qual_length_in_bits+zeroes_ones_padding_length;
            zeroes_ones_padding_length=0;
			sal_strcpy(segment_name,next_segment_name);
		} else {
			segment_length_in_bits += qual_length_in_bits;
		}
	}
	master_key->nof_key_segments++;
	sal_strcpy(master_key->key_segment[segment_ndx].name,segment_name);
	if ((segment_length_in_bits & 0x7) != 0) {
		SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 3, exit);
	}
	master_key->key_segment[segment_ndx].nof_bytes = (segment_length_in_bits)/8;
    if((SOC_DPP_CONFIG(unit)->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) && !(sal_strcmp("VRF", segment_name))) {
        master_key->key_segment[segment_ndx].type = KBP_KEY_FIELD_EM;
    } else {
        master_key->key_segment[segment_ndx].type = KBP_KEY_FIELD_PREFIX;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_convert_quals_to_segments", segment_length_in_bits, 0);
}

/*
* Update the master key according to program_id 
* called by the dbal 
*/
uint32
  arad_kbp_update_master_key(	
	SOC_SAND_IN int						unit,
	SOC_SAND_IN int						prog_id,
	SOC_SAND_IN SOC_DPP_DBAL_QUAL_INFO	qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
	SOC_SAND_IN int						nof_qualifiers)
{
	int i,res;
	uint8 opcode;
    uint8 sw_prog_id=0xFF;
	uint32 db_ndx,ltr_id;
    ARAD_KBP_LTR_SINGLE_SEARCH master_key;
	ARAD_KBP_LTR_CONFIG kbp_ltr_config;
	SOC_SAND_SUCCESS_FAILURE success;

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	/* build the new master key*/
	res = arad_kbp_convert_quals_to_segments(unit,qual_info,nof_qualifiers,&master_key);
	SOC_SAND_CHECK_FUNC_RESULT(res, 116, exit);

	/* find the relevant LTR*/
    arad_pp_prog_index_to_flp_app_get(unit,prog_id,&sw_prog_id);
    opcode = ARAD_KBP_FLP_PROG_TO_OPCODE(sw_prog_id);
	res = arad_kbp_opcode_to_db_type(
			  unit,
			  opcode,
			  &db_ndx,
			  &ltr_id,
			  &success
		  );
	SOC_SAND_CHECK_FUNC_RESULT(res, 116, exit);

	if (success != SOC_SAND_SUCCESS) {
		SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 2, exit);
	}

	/* update the LTR with the new master key*/
	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_ndx, &kbp_ltr_config);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

	if (kbp_ltr_config.valid == FALSE) {
		kbp_ltr_config.valid = TRUE;
		kbp_ltr_config.ltr_id = ltr_id;
		kbp_ltr_config.opcode = opcode;
	}

	kbp_ltr_config.master_key_fields.nof_key_segments = master_key.nof_key_segments;
	for (i=0;i<master_key.nof_key_segments;i++) {
		kbp_ltr_config.master_key_fields.key_segment[i].nof_bytes = master_key.key_segment[i].nof_bytes;
		kbp_ltr_config.master_key_fields.key_segment[i].type = master_key.key_segment[i].type;
		sal_strcpy(kbp_ltr_config.master_key_fields.key_segment[i].name,master_key.key_segment[i].name);
	}

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_ndx, &kbp_ltr_config);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_update_master_key", 0,0);
}

/*
* Update the lookup key according to program_id and table id 
* called by the dbal 
*/
uint32
  arad_kbp_update_lookup(
	SOC_SAND_IN int						unit,
	SOC_SAND_IN int 					table_id,
	SOC_SAND_IN int						prog_id,
	SOC_SAND_IN int 					search_id,
	SOC_SAND_IN SOC_DPP_DBAL_QUAL_INFO	qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
	SOC_SAND_IN int						nof_qualifiers)
{
	int i,res;
	int key_length=0;
	uint8 opcode;
    uint8 sw_prog_id=0xFF;
	uint32 db_ndx,ltr_id;
    ARAD_KBP_LTR_SINGLE_SEARCH lookup_key;
	ARAD_KBP_LTR_CONFIG kbp_ltr_config;
	ARAD_KBP_TABLE_CONFIG kbp_table_config,kbp_table;
	SOC_SAND_SUCCESS_FAILURE success;
	int tbl_id;

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	/* build the new lookup key*/
	res = arad_kbp_convert_quals_to_segments(unit,qual_info,nof_qualifiers,&lookup_key);
	SOC_SAND_CHECK_FUNC_RESULT(res, 116, exit);

	/* find the relevant LTR*/
    arad_pp_prog_index_to_flp_app_get(unit,prog_id,&sw_prog_id);
    opcode = ARAD_KBP_FLP_PROG_TO_OPCODE(sw_prog_id);
	res = arad_kbp_opcode_to_db_type(
			  unit,
			  opcode,
			  &db_ndx,
			  &ltr_id,
			  &success
		  );
	SOC_SAND_CHECK_FUNC_RESULT(res, 116, exit);

	if (success != SOC_SAND_SUCCESS) {
		SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 2, exit);
	}

	/* update the LTR with the new lookup*/
	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_ndx, &kbp_ltr_config);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

	/* cannot update lookup of an invalid LTR - arad_kbp_update_master_key() suppose to initialize it */
	if (kbp_ltr_config.valid == FALSE) {
		EXIT_AND_ERR_PRINT("arad_kbp_update_lookup() - invalid LTR\n",prog_id,db_ndx);
	}

	kbp_ltr_config.ltr_search[search_id].nof_key_segments = lookup_key.nof_key_segments;
	for (i=0;i<lookup_key.nof_key_segments;i++) {
		key_length += lookup_key.key_segment[i].nof_bytes;
		kbp_ltr_config.ltr_search[search_id].key_segment[i].nof_bytes = lookup_key.key_segment[i].nof_bytes;
		kbp_ltr_config.ltr_search[search_id].key_segment[i].type = lookup_key.key_segment[i].type;
		sal_strcpy(kbp_ltr_config.ltr_search[search_id].key_segment[i].name,lookup_key.key_segment[i].name);
	}

	if(table_id == -1){
		/* Init new table*/
		SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE  success;
		ARAD_KBP_TABLE_CONFIG_clear(&kbp_table);
		kbp_table.tbl_id = ARAD_KBP_MAX_NUM_OF_TABLES; 

		/* Set table width according to key-calc result */
		if (key_length <= 10) {
			kbp_table.tbl_width = NLM_TBL_WIDTH_80; 
		} 
		else if (key_length <= 20) {
			kbp_table.tbl_width = NLM_TBL_WIDTH_160; 
		} 
		else if (key_length <= 40) {
			kbp_table.tbl_width = NLM_TBL_WIDTH_320; 
		} else {
			/* Undefined table width */
			SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 3, exit);
		}

		kbp_table.tbl_asso_width = NLM_TBL_ADLEN_64B;
		kbp_table.bankNum = NLMDEV_BANK_0;
		kbp_table.group_id_start = 0;
		kbp_table.group_id_end = 20;
		kbp_table.min_priority = 32;
		kbp_table.clone_of_tbl_id = ARAD_KBP_FRWRD_IP_NOF_TABLES;
		kbp_table.tbl_size = 0; 

		res = arad_kbp_db_create(unit,
								 table_id,
								 &kbp_table, 
								 0,
								 &success);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10000, exit);
	}

	tbl_id = (table_id != -1) ? table_id : kbp_table_config.tbl_id;
	/* update the table with the new lookup (entry structure, used basically in diagnostics)*/
	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.get(unit, tbl_id, &kbp_table_config);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);
	if (kbp_table_config.valid == FALSE) {
		kbp_table_config.valid = TRUE;
	}

	kbp_table_config.entry_key_parsing.nof_segments = lookup_key.nof_key_segments;
	for (i=0;i<lookup_key.nof_key_segments;i++) {
		kbp_table_config.entry_key_parsing.key_segment[i].nof_bytes = lookup_key.key_segment[i].nof_bytes;
		kbp_table_config.entry_key_parsing.key_segment[i].type = lookup_key.key_segment[i].type;
		sal_strcpy(kbp_table_config.entry_key_parsing.key_segment[i].name,lookup_key.key_segment[i].name);
	}

	kbp_ltr_config.search_type[search_id] = ARAD_KBP_DB_TYPE_FORWARDING;
	kbp_ltr_config.tbl_id[search_id] = (ARAD_KBP_FRWRD_IP_TBL_ID)tbl_id;
	SHR_BITSET(&(kbp_ltr_config.parallel_srches_bmp), search_id);
	if (SHR_BITGET(&(kbp_ltr_config.parallel_srches_bmp), 0)&& 
       !SHR_BITGET(&(kbp_ltr_config.parallel_srches_bmp), 1)&&
        SHR_BITGET(&(kbp_ltr_config.parallel_srches_bmp), 2)) {
		kbp_ltr_config.is_cmp3_search = NlmTRUE;
	}else{
		kbp_ltr_config.is_cmp3_search = NlmFALSE;
	}

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_ndx, &kbp_ltr_config);
	SOC_SAND_CHECK_FUNC_RESULT(res, 20+db_ndx, exit);

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.set(unit, table_id, &kbp_table_config);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

	/* update all other DB (GTM,LUT) */
	res = arad_kbp_set_all_db_by_ltr(unit, db_ndx);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+db_ndx, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_update_lookup", 0, 0);
}

/* 
 * input: ltr_id 
 * outputs: master key structure (only nof_bytes of each segment) 
 * 			nof forwarding segments 
 *  
 */
STATIC uint32 
	arad_kbp_get_master_key_info_by_ltr_id(
       SOC_SAND_IN  int                      	unit,
	   SOC_SAND_IN  uint32                   	ltr_id,
	   SOC_SAND_OUT uint32                   	*num_of_frwd_seg,
	   SOC_SAND_OUT	ARAD_KBP_LTR_SINGLE_SEARCH 	*master_key
	)
{
	int res;
	int i;
	ARAD_KBP_LTR_CONFIG ltr_config_info = {0};
	uint32 ltr_idx;

	SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

	/* The LTR ID is correlated with the LTR location index */
	if (ltr_id < ARAD_KBP_ACL_LTR_ID_OFFSET) {
		for(ltr_idx = ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC; ltr_idx < ARAD_KBP_FRWRD_DB_NOF_TYPES; ltr_idx++) {
			if (Arad_kbp_db_type_to_ltr[ltr_idx] == ltr_id) {
				*num_of_frwd_seg = Arad_plus_kbp_ltr_config_static[ltr_idx].master_key_fields.nof_key_segments;
				break;
			}
		}
	}
	else {
		ltr_idx = (ltr_id - ARAD_KBP_ACL_LTR_ID_OFFSET)/2 + ARAD_KBP_FRWRD_DB_NOF_TYPES;
		*num_of_frwd_seg = 0;
	}

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, ltr_idx, &ltr_config_info); 
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

	master_key->nof_key_segments = ltr_config_info.master_key_fields.nof_key_segments;
	for (i=0;i<master_key->nof_key_segments;i++) {
		master_key->key_segment[i].nof_bytes = ltr_config_info.master_key_fields.key_segment[i].nof_bytes;
	}

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_get_masker_key_by_ltr_id()", 0, ltr_id);

}

/*
 *  return the fist segment index that is part of the ACL LSB part
 */
STATIC int 
	arad_kbp_acl_get_lsb_boundry_index(
       SOC_SAND_IN  int                      	unit,
	   SOC_SAND_IN	ARAD_KBP_LTR_SINGLE_SEARCH 	master_key
	)
{
	int i;
	uint32 lsb_size = 0;

	/* empty maskter-key - start form index 0*/
	if (master_key.nof_key_segments == 0) {
		return 0;
	}

	/* run on all segments in reverse order to find:
	*  1. ACLS in total length of 10 bytes - return index
	*  2. segment of non-ACL  - return former index (index+1)
	*  3. end of segments - return index 0
	*/
	for(i = master_key.nof_key_segments-1; i >= 0; i--) {
		lsb_size += master_key.key_segment[i].nof_bytes;

		/*if we get to non-acl segment than return the former checked segment */
		if (strstr(master_key.key_segment[i].name, "ACL") == NULL ) {
			return (i+1);
		}
		if (lsb_size == 10) {
			return i;
		}
	}
	return 0;
}

/*
 *  Function used only in case IPv6 DIP SIP sharing for programs what don't use KBP for IPv6 forwarding is enabled.
 *  It removes the IPv6 forwarding header segments from the end of the master key of the given LTR, if they are present there,
 *  and updates the LUT record size and the GTM opcode tx_data_size accordingly.
 *  The ACL searches will use the DIP and SIP from the forwarding header instead of the FLP keys.
 */
STATIC int
    arad_kbp_forwarding_header_segments_remove_from_ltr_master_key(
        int unit,
        uint32 db_type
    )
{
    uint32
        res;
    int
        master_key_field_index,
        header_key_field_index,
        is_header_present_in_master_key,
        ipv6_header_size_in_bytes = 40;
    ARAD_KBP_LTR_SINGLE_SEARCH
        header_search = ipv6_dip_sip_sharing_forwarding_header_search;
    ARAD_KBP_LTR_CONFIG
        ltr_config_info;
    arad_kbp_lut_data_t
        lut_info;
    ARAD_KBP_GTM_OPCODE_CONFIG_INFO
        opcode_config_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.get(unit, db_type, &lut_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.get(unit, db_type, &opcode_config_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    if(ltr_config_info.master_key_fields.nof_key_segments >= header_search.nof_key_segments) {
        /* the presumption is that the last 3 key segments are "UNUSED_HEADER_PART", "SIP" and "DIP"; need to verify that */
        master_key_field_index = ltr_config_info.master_key_fields.nof_key_segments - 1;
        header_key_field_index = header_search.nof_key_segments - 1;
        is_header_present_in_master_key = 1;

        while(master_key_field_index >= 0 && header_key_field_index >= 0) {
            /* compare forwarding header key fileds to the last master key fields */
            if(sal_strcmp(ltr_config_info.master_key_fields.key_segment[master_key_field_index].name, header_search.key_segment[header_key_field_index].name) != 0) {
                is_header_present_in_master_key = 0;
                break;
            }

            master_key_field_index--;
            header_key_field_index--;
        }

        if(is_header_present_in_master_key != 0) {
            /* forwarding header key segments are found in the master key; remove them */
            ltr_config_info.master_key_fields.nof_key_segments -= header_search.nof_key_segments;
            /* decrease the record size */
            lut_info.rec_size -= ipv6_header_size_in_bytes;
            /* decrease the data size */
            if(ipv6_header_size_in_bytes > opcode_config_info.tx_data_size) {
                opcode_config_info.tx_data_size = 0;
            } else {
                opcode_config_info.tx_data_size -= ipv6_header_size_in_bytes;
            }

            /* update all info */
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_type, &ltr_config_info);
            SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.set(unit, db_type, &lut_info);
            SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.set(unit, db_type, &opcode_config_info);
            SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_forwarding_header_segments_remove_from_ltr_master_key()", 0, 0);
}

/*
 *  Function used only in case IPv6 DIP SIP sharing for programs what don't use KBP for IPv6 forwarding is enabled.
 *  It adds the IPv6 forwarding header segments at the end of the master key of the given LTR
 *  and updates the LUT record size and the GTM opcode tx_data_size accordingly.
 *  The ACL searches will use the DIP and SIP from the forwarding header instead of the FLP keys.
 */
STATIC int
    arad_kbp_forwarding_header_segments_append_to_ltr_master_key(
        int unit,
        uint32 db_type
    )
{
    uint32
        res;
    int
        master_key_field_index,
        header_key_field_index,
        ipv6_header_size_in_bytes = 40;
    ARAD_KBP_LTR_SINGLE_SEARCH
        header_search = ipv6_dip_sip_sharing_forwarding_header_search;
    ARAD_KBP_LTR_CONFIG
        ltr_config_info;
    arad_kbp_lut_data_t
        lut_info;
    ARAD_KBP_GTM_OPCODE_CONFIG_INFO
        opcode_config_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.get(unit, db_type, &lut_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.get(unit, db_type, &opcode_config_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    if(ltr_config_info.master_key_fields.nof_key_segments + header_search.nof_key_segments > ARAD_KBP_MAX_NOF_KEY_SEGMENTS) {
        /* master key segments will exceed the allowed size */
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in $s(): Allowed number of segments in the master key exceeded\n")));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 3, exit);
    }

    master_key_field_index = ltr_config_info.master_key_fields.nof_key_segments;
    header_key_field_index = 0;

    while(header_key_field_index < header_search.nof_key_segments) {
        /* append the forwarding header key fields to the master key */
        sal_strcpy(ltr_config_info.master_key_fields.key_segment[master_key_field_index].name, header_search.key_segment[header_key_field_index].name);
        ltr_config_info.master_key_fields.key_segment[master_key_field_index].nof_bytes = header_search.key_segment[header_key_field_index].nof_bytes;
        ltr_config_info.master_key_fields.key_segment[master_key_field_index].type = header_search.key_segment[header_key_field_index].type;

        master_key_field_index++;
        header_key_field_index++;
    }

    ltr_config_info.master_key_fields.nof_key_segments += header_search.nof_key_segments;
    /* increase the record size */
    lut_info.rec_size += ipv6_header_size_in_bytes;
    /* increase the data size */
    opcode_config_info.tx_data_size += ipv6_header_size_in_bytes;

    /* update all info */
    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_type, &ltr_config_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.set(unit, db_type, &lut_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.set(unit, db_type, &opcode_config_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_forwarding_header_segments_append_to_ltr_master_key()", 0, 0);
}

STATIC
uint32 
    arad_kbp_ltr_lookup_add_verify(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint32                      ltr_id,
       SOC_SAND_IN  uint32                      flags,
       SOC_SAND_IN  uint32                      table_id,
       SOC_SAND_IN  ARAD_KBP_DB_TYPE            db_type,
       SOC_SAND_IN  ARAD_KBP_LTR_SINGLE_SEARCH  *search_format
   )
{
    uint32 
        field_idx, res;

    uint8 is_valid = TRUE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_ERR_IF_OUT_OF_RANGE(ltr_id, ARAD_KBP_LTR_ID_FIRST, ARAD_KBP_MAX_ACL_LTR_ID - 1, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);    
    SOC_SAND_ERR_IF_OUT_OF_RANGE(table_id, ARAD_KBP_ACL_TABLE_ID_OFFSET, ARAD_KBP_MAX_NUM_OF_TABLES - 1, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit); 

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.get(unit, table_id, &is_valid); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    /* Verify the table is valid (already created) iff not check only */
    if (((flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY) && (is_valid))
        || ((!(flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY)) && (!is_valid)))
    {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DB_DOESNT_EXIST_ERR, 30, exit);
    }

    SOC_SAND_ERR_IF_ABOVE_NOF(db_type, ARAD_KBP_NOF_DB_TYPES, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 40, exit);    
    SOC_SAND_ERR_IF_ABOVE_MAX(search_format->nof_key_segments, ARAD_KBP_MAX_NOF_KEY_SEGMENTS, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 50, exit);

    /* Verify total number of segments in Master key will not exceed max */
    {
        uint32 nof_key_segments = 0;
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.master_key_fields.nof_key_segments.get(unit, db_type, &nof_key_segments); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        SOC_SAND_ERR_IF_ABOVE_MAX((search_format->nof_key_segments + nof_key_segments), 
                                  ARAD_KBP_MAX_NOF_KEY_SEGMENTS, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 60, exit);
    }

    /* Verify search format */
    for (field_idx = 0; field_idx < search_format->nof_key_segments; field_idx++) 
    {
        SOC_SAND_ERR_IF_ABOVE_MAX(search_format->key_segment[field_idx].nof_bytes, ARAD_KBP_MAX_SEGMENT_LENGTH_BYTES, 
                                  ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 70, exit);
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_ltr_lookup_add_verify()", 0, 0);
}

STATIC
uint32 
    arad_kbp_ltr_lookup_add(
       SOC_SAND_IN  int                      		unit,
       SOC_SAND_IN  uint32                      	search_id,
       SOC_SAND_IN  uint8                       	opcode,
       SOC_SAND_IN  uint32                      	ltr_id,
       SOC_SAND_IN  uint32                      	flags,
       SOC_SAND_IN  uint32                      	table_id,
       SOC_SAND_IN  ARAD_KBP_DB_TYPE            	db_type,
	   SOC_SAND_IN  ARAD_KBP_ACL_IN_MASTER_KEY_TYPE acl_type,
       SOC_SAND_IN  ARAD_KBP_LTR_SINGLE_SEARCH  	*search_format,
       uint32                                   	nof_shared_quals,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    	*success
   )
{
    uint32 
        ltr_idx,
        nof_segments,
        res = SOC_SAND_OK;
    uint8
        check_only;

    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(success);    

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    check_only = (flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY) ? TRUE : FALSE;
    *success = SOC_SAND_FAILURE_INTERNAL_ERR;

    res = arad_kbp_ltr_lookup_add_verify(
            unit,
            ltr_id, 
            flags,
            table_id, 
            db_type, 
            search_format
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* The LTR ID is correlated with the LTR location index */
    if (ltr_id < ARAD_KBP_ACL_LTR_ID_OFFSET) {
        for(ltr_idx = ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC; ltr_idx < ARAD_KBP_FRWRD_DB_NOF_TYPES; ltr_idx++) {
            if (Arad_kbp_db_type_to_ltr[ltr_idx] == ltr_id) {
                break;
            }
        }
    }
    else {
		ltr_idx = (ltr_id - ARAD_KBP_ACL_LTR_ID_OFFSET)/2 + ARAD_KBP_FRWRD_DB_NOF_TYPES;
    }

    /* Verify that there is an available lookup/search in LTR.
     * (Each LTR supports up to 4 parallel DB lookups).
     */

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, ltr_idx, &ltr_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

	/* If an ACL was assign to a MC dummy table,
	 * its search bit was already set and the ACL is overwrite a dummy lookup 
	 * the DUMMY tables are configure only on search #1 - validate it! 
	 */
	if (ltr_config_info.tbl_id[search_id] == ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC || 
		ltr_config_info.tbl_id[search_id] == ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC) {
		if (search_id != 1) {
			SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit);
		}
	}
	else {
		if (SHR_BITGET(&ltr_config_info.parallel_srches_bmp, search_id)) {
				SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit);
		}
	}

	/* Searches 4,5 are available only for Jericho and above*/
	if ((search_id >= ARAD_KBP_CMPR3_FIRST_ACL) && !(SOC_IS_JERICHO(unit))) {
		SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit);
	}

    /* Verify that no other ACL Database is configured for this LTR.
     * (Currently only one ACL per LTR is supported, in addition to 
     * Forwarding tables).
     */
    if (ltr_config_info.search_type[search_id] == ARAD_KBP_DB_TYPE_ACL) {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_DATABASE_ALREADY_EXISTS_ERR, 30, exit);
    }

    if (!check_only) 
    {
		uint32 index_of_first_lsb_segment;
		ARAD_KBP_LTR_SINGLE_SEARCH copy_of_lsb_master_key;
		int rv;
        /* In case verification passed and configuration is allowed, then add new
         * db lookup to this LTR, when the search index is the next one available.
         */
        SHR_BITSET(&ltr_config_info.parallel_srches_bmp, search_id);
        ltr_config_info.tbl_id[search_id] = table_id;
        ltr_config_info.search_type[search_id] = db_type;

        /* Search Format was already verified */
        sal_memcpy(&ltr_config_info.ltr_search[search_id],
                   search_format, 
                   sizeof(ARAD_KBP_LTR_SINGLE_SEARCH));

        /* New search fields need to be added to master key as well */
        nof_segments = ltr_config_info.master_key_fields.nof_key_segments;


		/*
		*   On Jericho, Key c (used for ACLs) is splited into two parts (LSB and MSB) - 80 bits each.
		*   FLP builds the master key in {MSB,LSB} order.
		*  
		*   On Arad+ key C was one chunk of 160 bits and the master key was equivalent to {LSB,MSB} order.
		*  
		*   there are 3 different types of Acls, this is how we handle each one of them:
		*   ARAD_KBP_ACL_IN_MASTER_KEY_LSB_ONLY -
		*   	same as Arad+, just adding the ACL at the end of the master-key (last segment)
		*  
		*   ARAD_KBP_ACL_IN_MASTER_KEY_MSB_ONLY -
		*   	need to be added before the LSB segments, but after the former MSB segments (last msb segment)
		*  	
		*   ARAD_KBP_ACL_IN_MASTER_KEY_LSB_MSB -
		*   	saparte the acl into teo segments: segment[0] - LSB, segment[1] - MSB.	
		*  		segment[0] - handle as ARAD_KBP_ACL_IN_MASTER_KEY_LSB_ONLY
		* 		segment[1] - handle as ARAD_KBP_ACL_IN_MASTER_KEY_MSB_ONLY
		*/
		if (SOC_IS_JERICHO(unit)) {
			if (acl_type == ARAD_KBP_ACL_IN_MASTER_KEY_LSB_ONLY) {
				sal_memcpy(&ltr_config_info.master_key_fields.key_segment[nof_segments],
						   &search_format->key_segment[0+nof_shared_quals], 
						   sizeof(ARAD_KBP_KEY_SEGMENT) * (search_format->nof_key_segments - nof_shared_quals));
			}else if (acl_type == ARAD_KBP_ACL_IN_MASTER_KEY_MSB_ONLY) {
				rv = arad_kbp_acl_get_lsb_boundry_index(unit, ltr_config_info.master_key_fields);
				index_of_first_lsb_segment = rv;

				sal_memcpy(& copy_of_lsb_master_key.key_segment[0],
						   &search_format->key_segment[0+nof_shared_quals], 
						   sizeof(ARAD_KBP_KEY_SEGMENT));

				sal_memcpy(&copy_of_lsb_master_key.key_segment[1],
						   &ltr_config_info.master_key_fields.key_segment[index_of_first_lsb_segment], 
						   sizeof(ARAD_KBP_KEY_SEGMENT) * (nof_segments - index_of_first_lsb_segment));

				sal_memcpy(&ltr_config_info.master_key_fields.key_segment[index_of_first_lsb_segment],
						   &copy_of_lsb_master_key.key_segment[0], 
						   sizeof(ARAD_KBP_KEY_SEGMENT) * (nof_segments - index_of_first_lsb_segment + 1));
			}else{
				rv = arad_kbp_acl_get_lsb_boundry_index(unit, ltr_config_info.master_key_fields);
				index_of_first_lsb_segment = rv;


				sal_memcpy(&ltr_config_info.master_key_fields.key_segment[nof_segments],
						   &search_format->key_segment[0+nof_shared_quals],
						   sizeof(ARAD_KBP_KEY_SEGMENT));

				nof_segments++;

				sal_memcpy(&copy_of_lsb_master_key.key_segment[0],
						   &search_format->key_segment[1+nof_shared_quals], 
						   sizeof(ARAD_KBP_KEY_SEGMENT));

				sal_memcpy(&copy_of_lsb_master_key.key_segment[1],
						   &ltr_config_info.master_key_fields.key_segment[index_of_first_lsb_segment], 
						   sizeof(ARAD_KBP_KEY_SEGMENT) * (nof_segments - index_of_first_lsb_segment));

				sal_memcpy(&ltr_config_info.master_key_fields.key_segment[index_of_first_lsb_segment],
						   &copy_of_lsb_master_key.key_segment[0], 
						   sizeof(ARAD_KBP_KEY_SEGMENT) * (nof_segments - index_of_first_lsb_segment + 1));
			}
		}else{
        	sal_memcpy(&ltr_config_info.master_key_fields.key_segment[nof_segments],
            		   &search_format->key_segment[0+nof_shared_quals], 
                       sizeof(ARAD_KBP_KEY_SEGMENT) * (search_format->nof_key_segments - nof_shared_quals));
		}
       
        ltr_config_info.master_key_fields.nof_key_segments += 
            (search_format->nof_key_segments - nof_shared_quals);

        /* Validate LTR (instruction) */
        ltr_config_info.valid = TRUE;
        ltr_config_info.opcode = opcode;
        ltr_config_info.ltr_id = ltr_id;
		if (search_id >= ARAD_KBP_CMPR3_FIRST_ACL) {
			ltr_config_info.is_cmp3_search = NlmTRUE;
		}

        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, ltr_idx, &ltr_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
    }

    *success = SOC_SAND_SUCCESS;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_ltr_lookup_add()", 0, 0);
}

uint32 
    arad_kbp_opcode_to_db_type(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint8                       opcode,
       SOC_SAND_OUT uint32                      *db_type,
       SOC_SAND_OUT uint32                      *ltr_id,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
   )
{
    uint32 
        db_type_idx, res,
        found_idx = 0;

    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(ltr_id);
    SOC_SAND_CHECK_NULL_INPUT(success);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;

    /* Check if opcode has tables already */
    for (db_type_idx = 0; (db_type_idx < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS) && (SOC_SAND_SUCCESS != *success); db_type_idx++) 
    {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type_idx, &ltr_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 10+ db_type_idx, exit);
        if (ltr_config_info.valid) 
        {
            if (ltr_config_info.opcode == opcode) 
            {
                found_idx = db_type_idx;
                *success = SOC_SAND_SUCCESS;
                break;
            }
        }

    }
    
    /* If no LTR was matched, find an unused LTR for the given opcode */
    if (*success != SOC_SAND_SUCCESS) 
    {
        for (db_type_idx = ARAD_KBP_FRWRD_DB_NOF_TYPES; (db_type_idx < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS) && (SOC_SAND_SUCCESS != *success); db_type_idx++) 
        {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type_idx, &ltr_config_info); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 100+ db_type_idx, exit);
            if(ltr_config_info.valid == FALSE)
            {
                found_idx = db_type_idx;
                *success = SOC_SAND_SUCCESS;
            }
        }
    }
   
    /* If a matching or available LTR was found, update return values */
    if (*success == SOC_SAND_SUCCESS) 
    {
        if (found_idx < ARAD_KBP_FRWRD_DB_NOF_TYPES) {
            *ltr_id = Arad_kbp_db_type_to_ltr[found_idx];
        }
        else {
	       *ltr_id = ARAD_KBP_ACL_LTR_ID_OFFSET + 2*(found_idx - ARAD_KBP_FRWRD_DB_NOF_TYPES);
			if (*ltr_id >= ARAD_KBP_MAX_ACL_LTR_ID) {
				SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_ENTRY_ID_OUT_OF_RANGE_ERR, 20, exit);
			}
        }

        *db_type = found_idx;
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_opcode_to_db_type()", 0, 0);
}

STATIC
uint32 
    arad_kbp_add_ltr_to_opcode_verify(
       SOC_SAND_IN  int                         unit,
       SOC_SAND_IN  ARAD_KBP_FRWRD_IP_DB_TYPE   db_type,
       SOC_SAND_IN  uint8                       opcode,
       SOC_SAND_IN  uint32                      ltr_id,
       SOC_SAND_IN  uint32                      key_size_in_bits,
       SOC_SAND_IN  uint32                      search_id
   )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_ERR_IF_ABOVE_NOF(db_type, ARAD_KBP_MAX_NUM_OF_FRWRD_DBS, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 10, exit);    
    SOC_SAND_ERR_IF_OUT_OF_RANGE(ltr_id, ARAD_KBP_LTR_ID_FIRST, ARAD_KBP_MAX_ACL_LTR_ID - 1, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 30, exit);    
    SOC_SAND_ERR_IF_ABOVE_NOF(key_size_in_bits, ARAD_KBP_MASTER_KEY_MAX_LENGTH, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 40, exit);
    SOC_SAND_ERR_IF_ABOVE_NOF(search_id, ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES, ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 50, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_add_ltr_to_opcode_verify()", 0, 0);
}


STATIC
uint32 
    arad_kbp_add_ltr_search_to_opcode(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  ARAD_KBP_FRWRD_IP_DB_TYPE   db_type,
       SOC_SAND_IN  uint8                       opcode,
       SOC_SAND_IN  uint32                      ltr_id,
       SOC_SAND_IN  uint32                      key_size_in_bytes,
       SOC_SAND_IN  uint32                      search_id,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
   )
{
    uint32
        result_idx, res;

    arad_kbp_lut_data_t lut_info = {0};
    arad_kbp_frwd_ltr_db_t ltr_db_info;
    ARAD_KBP_GTM_OPCODE_CONFIG_INFO opcode_info = {0};

    ARAD_PMF_CE_IRPP_QUALIFIER_INFO irpp_qual_info;
    uint8 found;
    NlmBool is_cmp3_search = NlmTRUE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(success);

    arad_kbp_frwd_ltr_db_clear(&ltr_db_info);

    /* Configure LUT:
     * Includes encoding of LTR, record size and type, result size and type.
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.get(unit, db_type, &lut_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    lut_info.rec_size += key_size_in_bytes;
    lut_info.rec_type = ARAD_KBP_LUT_REC_TYPE_REQUEST;
    lut_info.rec_is_valid = 1;
    lut_info.mode = ARAD_KBP_LUT_INSTR_LUT1_CONTEXID_SEQ_NUM_MODE;
    lut_info.key_config = ARAD_KBP_LUT_KEY_CONFIG_SEND_INCOMING_DATA;
    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.is_cmp3_search.get(unit, db_type, &is_cmp3_search); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10 + db_type, exit);
    if (is_cmp3_search) {
        lut_info.instr = ARAD_KBP_ROP_LUT_INSTR_CMP3_GET(ltr_id);
    }
    else {
        lut_info.instr = ARAD_KBP_ROP_LUT_INSTR_CMP1_GET(ltr_id);
    }

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.get(unit, db_type, &ltr_db_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
    ltr_db_info.opcode = opcode;

    arad_pmf_ce_internal_field_info_find(unit, SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_0 + search_id, 
                                                                     SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, 0, &found, &irpp_qual_info);    

    if (!found) {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_ACTION_OUT_OF_RANGE_ERR, 20, exit);
    }
	
	switch (search_id) {
        case 0:            
            lut_info.result0_idx_ad_cfg = irpp_qual_info.info.qual_nof_bits/8;
            lut_info.result0_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY;
            break;
        case 1:
            lut_info.result1_idx_ad_cfg = irpp_qual_info.info.qual_nof_bits/8;
            lut_info.result1_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY;
			if ((db_type == ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) && (ARAD_KBP_ENABLE_IPV4_DC)) {
				 lut_info.result1_idx_ad_cfg += ARAD_KBP_IPV4DC_RES1_PAD_BYTES;
			}
            break;
        case 2:
            lut_info.result2_idx_ad_cfg = irpp_qual_info.info.qual_nof_bits/8;
            lut_info.result2_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY;
            break;
        case 3:
			lut_info.result3_idx_ad_cfg = irpp_qual_info.info.qual_nof_bits/8;
			lut_info.result3_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY;
			if ((db_type != ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) && (ARAD_KBP_ENABLE_IPV4_DC)) {
				 lut_info.result3_idx_ad_cfg += ARAD_KBP_IPV4DC_RES3_PAD_BYTES;
			}
            break;
		case 4:
            lut_info.result4_idx_ad_cfg = irpp_qual_info.info.qual_nof_bits/8;            
            lut_info.result4_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY;
            break;
		case 5:
            lut_info.result5_idx_ad_cfg = irpp_qual_info.info.qual_nof_bits/8;            
            lut_info.result5_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY;
            break;
        default:
            SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_MGMT_TCAM_DB_ID_OUT_OF_RANGE_ERR, 20, exit);
        }

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.set(unit, db_type, &lut_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    
    ltr_db_info.res_data_len[search_id] = irpp_qual_info.info.qual_nof_bits/8;

    if ((db_type == ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) && (search_id == 1) && (ARAD_KBP_ENABLE_IPV4_DC)) {
        ltr_db_info.res_data_len[search_id] += ARAD_KBP_IPV4DC_RES1_PAD_BYTES;
    }

    if ((db_type != ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC) && (search_id == 3) && (ARAD_KBP_ENABLE_IPV4_DC)) {
        ltr_db_info.res_data_len[search_id] += ARAD_KBP_IPV4DC_RES3_PAD_BYTES;
    }

    if (ltr_db_info.res_data_len[search_id] <= 4) {
        ltr_db_info.res_format[search_id] = NLM_ARAD_INDEX_AND_32B_AD;
    }else if (ltr_db_info.res_data_len[search_id] <= 8) {
        ltr_db_info.res_format[search_id] = NLM_ARAD_INDEX_AND_64B_AD;
    }else{
        ltr_db_info.res_format[search_id] = NLM_ARAD_INDEX_AND_128B_AD;
    }
	
    /* Configure LTR Database Info:
     * Includes opcode, result encoding and length.
     */
    ltr_db_info.res_total_data_len = 1;
    for (result_idx = 0;
         result_idx < NLM_ARAD_MAX_NUM_RESULTS_PER_INST; 
         result_idx++ ) 
    {
		if (ltr_db_info.res_format[result_idx] != NLM_ARAD_NO_INDEX_NO_AD) 
		{
			ltr_db_info.res_total_data_len += ltr_db_info.res_data_len[result_idx];
		}
    }

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.set(unit, db_type, &ltr_db_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);


    /* Configure opcode Info:
     * Includes ROP TX and RX definitions per opcode.
     */

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.get(unit, db_type, &opcode_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    opcode_info.tx_data_size = lut_info.rec_size - 1;
    opcode_info.tx_data_type = 1; /* REQUEST */
	if ((search_id != ARAD_KBP_CMPR3_SKIPPED_SEARCH) || (is_cmp3_search == NlmFALSE)) {
		opcode_info.rx_data_size += ltr_db_info.res_data_len[search_id];
	}
    opcode_info.rx_data_type = 3; /* REPLY */

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.set(unit, db_type, &opcode_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    /* Add bytes to total result according to result index according to work mode */
        
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_add_ltr_search_to_opcode()", 0, 0);
}

STATIC
uint32 
    arad_kbp_add_ltr_to_opcode(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  ARAD_KBP_FRWRD_IP_DB_TYPE   db_type,
       SOC_SAND_IN  uint32                      flags,
       SOC_SAND_IN  uint8                       opcode,
       SOC_SAND_IN  uint32                      ltr_id,
       SOC_SAND_IN  uint32                      key_size_in_bytes,
       SOC_SAND_IN  uint32                      search_id,
       SOC_SAND_IN  ARAD_KBP_LTR_SINGLE_SEARCH  *search_format,
       uint32                                   nof_shared_segments,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
   )
{
    uint32
        srch_idx,
        ltr_key_size_in_bytes,
        res;

    int i;

    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    /* First Verify */
    res = arad_kbp_add_ltr_to_opcode_verify(
            unit,
            db_type,
            opcode,
            ltr_id,
            (key_size_in_bytes * 8),
            search_id
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if(!(flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY))
    {
        for(srch_idx = 0; srch_idx <= search_id; srch_idx++)
        {            
            if ((srch_idx != search_id) && 
                SHR_BITGET(&(ltr_config_info.parallel_srches_bmp), srch_idx)) {
                continue;
            }

            ltr_key_size_in_bytes = (srch_idx == search_id) ? key_size_in_bytes : 0;

            /* Add LTR to Opcode */
            res = arad_kbp_add_ltr_search_to_opcode(
                    unit,
                    db_type,
                    opcode,
                    ltr_id,
                    ltr_key_size_in_bytes,
                    srch_idx,
                    success
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            if (srch_idx != search_id)
            {
                /* After setting some LTR search index, make sure to
                 * configure bitmap even if it's DUMMY table, so that 
                 * if will be configured in table_init and search_init 
                 *  
                 * Define the Key segment to be a dummy field after the master key 
                 */
                res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit, ARAD_KBP_TABLE_INDX_TO_DUMMY_TABLE_ID(srch_idx), TRUE); 
                SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
                
                SHR_BITSET(&ltr_config_info.parallel_srches_bmp, srch_idx);
                ltr_config_info.search_type[srch_idx] = ARAD_KBP_DB_TYPE_FORWARDING;
                ltr_config_info.ltr_search[srch_idx].nof_key_segments = 1 + nof_shared_segments;
                for (i = 0; i < 1 + nof_shared_segments; i++) {
                    ltr_config_info.ltr_search[srch_idx].key_segment[i] = search_format->key_segment[i];
                }

                res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, db_type, &ltr_config_info); 
                SOC_SAND_CHECK_FUNC_RESULT(res, 100+srch_idx, exit);
            }
        }
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_add_ltr_to_opcode()", 0, 0);
}


uint32
    arad_kbp_shrink_shared_qualifiers(SOC_SAND_IN  int                      unit,
                                      ARAD_PP_FP_SHARED_QUAL_INFO*          shared_quals_info,
                                      uint32                                nof_shared_quals,
                                      int*                                  nof_shared_segments,
                                      int*                                  total_shared_size)
{
    int i;
    int ip6_sip = 0, ip6_dip = 0;
    ARAD_PP_FP_SHARED_QUAL_INFO          shared_quals_info_lcl[MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM];

    (*total_shared_size) = 0;
    (*nof_shared_segments) = 0;

    for(i = 0; i < nof_shared_quals; i++){
        (*total_shared_size) += shared_quals_info[i].size;
        if((shared_quals_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW) || (shared_quals_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH)){        
            ip6_sip++;
            if (ip6_sip == 2) {
                memcpy(&shared_quals_info_lcl[(*nof_shared_segments)], &shared_quals_info[i], sizeof(ARAD_PP_FP_SHARED_QUAL_INFO));
                shared_quals_info_lcl[(*nof_shared_segments)].size = shared_quals_info[i].size*2;
                (*nof_shared_segments)++;
            }
        }

        if((shared_quals_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH) || (shared_quals_info[i].qual_type == SOC_PPC_FP_QUAL_HDR_IPV6_DIP_LOW)){        
            ip6_dip++;
            if (ip6_dip == 2) {
                memcpy(&shared_quals_info_lcl[(*nof_shared_segments)], &shared_quals_info[i], sizeof(ARAD_PP_FP_SHARED_QUAL_INFO));
                shared_quals_info_lcl[(*nof_shared_segments)].size = shared_quals_info[i].size*2;
                (*nof_shared_segments)++;
            }
        }
    }

    if (ip6_dip == 2 || ip6_sip == 2) {
        memcpy(shared_quals_info, &shared_quals_info_lcl, sizeof(ARAD_PP_FP_SHARED_QUAL_INFO)* MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM);
    }

    (*nof_shared_segments) = nof_shared_quals - (*nof_shared_segments);
    
        
return 0;
}
/* 
 * For ACL, the key-size gives the table size.
 * Then, all the computations are done according to the table 
 * size, assuming the whole table size is the logical key 
 */
uint32 
    arad_kbp_add_acl(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint32                      table_id,
       SOC_SAND_IN  uint32                      search_id,
       SOC_SAND_IN  uint32                      pgm_bmp_used,
       SOC_SAND_IN  uint32                      pgm_ndx_min,
       SOC_SAND_IN  uint32                      pgm_ndx_max,
       SOC_SAND_IN  uint32                      key_size_in_bytes,   
       SOC_SAND_IN  uint32                      flags, 
       SOC_SAND_IN  uint32                      min_priority,
       uint32                                   nof_shared_quals,
       ARAD_PP_FP_SHARED_QUAL_INFO              shared_quals_info[MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM],
       SOC_SAND_OUT uint32                      *pgm_bmp_new,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
    )
{
    uint32 
        res,
        db_type,
        pgm_ndx,
        ltr_id,
        segment_idx;
	uint8 prog_id;
    ARAD_KBP_TABLE_CONFIG 
        kbp_table_config;
    ARAD_KBP_LTR_SINGLE_SEARCH 
        search_format;
    ARAD_KBP_FRWRD_IP_OPCODE
        opcode;

    int i, total_shared_size = 0, nof_shared_segments = nof_shared_quals;

    ARAD_PMF_CE_IRPP_QUALIFIER_INFO irpp_qual_info;
    uint8 found;

#ifdef BCM_88675_A0
    ARAD_INIT_ELK* elk = &SOC_DPP_CONFIG(unit)->arad->init.elk;
#endif

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

     ARAD_KBP_TABLE_CONFIG_clear(&kbp_table_config);
     ARAD_KBP_LTR_SINGLE_SEARCH_clear(&search_format);

     if(nof_shared_quals > 0){
         arad_kbp_shrink_shared_qualifiers(unit, shared_quals_info, nof_shared_quals, &nof_shared_segments, &total_shared_size);
     }
    /* Initialize to invalid Table ID */
    kbp_table_config.tbl_id = ARAD_KBP_MAX_NUM_OF_TABLES; 

    /* Set table width according to key-calc result */
    if (key_size_in_bytes + total_shared_size <= 10) {
        kbp_table_config.tbl_width = NLM_TBL_WIDTH_80; 
    } 
    else if (key_size_in_bytes + total_shared_size <= 20) {
        kbp_table_config.tbl_width = NLM_TBL_WIDTH_160; 
    } 
    else if (key_size_in_bytes + total_shared_size <= 40) {
        kbp_table_config.tbl_width = NLM_TBL_WIDTH_320; 
    } 
    else {
        /* Undefined table width */
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 3, exit);
    }


    arad_pmf_ce_internal_field_info_find(unit, SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_0 + search_id, 
                                                                     SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, 0, &found, &irpp_qual_info);

    if (!found) {
        SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_ACTION_OUT_OF_RANGE_ERR, 20, exit);
    }

    if (irpp_qual_info.info.qual_nof_bits <= 24) {
        kbp_table_config.tbl_asso_width = NLM_TBL_ADLEN_24B;
    }else if (irpp_qual_info.info.qual_nof_bits <= 32) {
        kbp_table_config.tbl_asso_width = NLM_TBL_ADLEN_32B;
    }else if (irpp_qual_info.info.qual_nof_bits <= 48) {
        kbp_table_config.tbl_asso_width = NLM_TBL_ADLEN_48B;
    }else if (irpp_qual_info.info.qual_nof_bits <= 64) {
        kbp_table_config.tbl_asso_width = NLM_TBL_ADLEN_64B;
    }else{
        kbp_table_config.tbl_asso_width = NLM_TBL_ADLEN_128B;
    }

    kbp_table_config.bankNum = NLMDEV_BANK_0;
    kbp_table_config.group_id_start = 0;
    kbp_table_config.group_id_end = 0;
    kbp_table_config.min_priority = min_priority;
    kbp_table_config.clone_of_tbl_id = ARAD_KBP_FRWRD_IP_NOF_TABLES;

    /* Configurable table size acocrding to SOC property */
    switch (kbp_table_config.tbl_width) {
    case NLM_TBL_WIDTH_80:
        kbp_table_config.tbl_size = soc_property_get(unit, spn_EXT_ACL80_TABLE_SIZE, (64*1024)); 
        break;
    case NLM_TBL_WIDTH_160:
        kbp_table_config.tbl_size = soc_property_get(unit, spn_EXT_ACL160_TABLE_SIZE, (64*1024)); 
        break;
    case NLM_TBL_WIDTH_320:
        kbp_table_config.tbl_size = soc_property_get(unit, spn_EXT_ACL320_TABLE_SIZE, (64*1024)); 
        break;
    default:
        /* Undefined table width */
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 2, exit);
    }

	if(!SOC_IS_JERICHO(unit)){
	    /* Initialize search format */

	    /* The key segments will be in 80 bit chunks.
	     * Currently there is no reason to break the ACL to its qualifiers 
	     * since ACL in ELK uses only key C and only one ACL is currently 
	     * supported (lookup key can be taken as a whole instead of separate 
	     * pieces of the master key).
	     * The reason to break down to segments is the max key field size 
	     * in ELK is 16 bytes.  
	     */
	    memset(&search_format, 0, sizeof(search_format));
	    search_format.nof_key_segments = (key_size_in_bytes + 9) / 10;
	    search_format.nof_key_segments = search_format.nof_key_segments+nof_shared_segments;

	    for (i = 0; i < nof_shared_segments; i++) {        
	        search_format.key_segment[i].nof_bytes = shared_quals_info[i].size;
	        sal_memcpy(search_format.key_segment[i].name, shared_quals_info[i].name, 20);
	        search_format.key_segment[i].type = KBP_KEY_FIELD_TERNARY;
	    }
    
	    for (segment_idx = 0; segment_idx < search_format.nof_key_segments - nof_shared_segments; segment_idx++) 
	    {
	      /* the last segment should be the remaining
	       * of the bytes, else a full 10 byte segment 
	       * For the last segment the nof_bytes should be between 1B and 10B.
	       */
	      search_format.key_segment[segment_idx+nof_shared_segments].nof_bytes = 
	        (segment_idx+nof_shared_segments == (search_format.nof_key_segments-1)) ? (((key_size_in_bytes - 1) % 10) + 1) : 10;

	      sal_sprintf(search_format.key_segment[segment_idx+nof_shared_segments].name, "ACL%d-s%d", table_id, segment_idx);
	      search_format.key_segment[segment_idx+nof_shared_segments].type = KBP_KEY_FIELD_TERNARY;
	    }
	}

    /* KBP config SW is configured in two phases:
     * Phase 1 -> check physibility 
     * Phase 2 -> allocate resources (databases and instructions) 
     *  
     * Actual configuration of KBP device is done at a later time 
     * according to this configation.
     */

    res = arad_kbp_db_create(
              unit,
              table_id,
              &kbp_table_config, 
              flags,
              success
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 115, exit);

    if(SOC_SAND_SUCCESS != *success) {
        /* Break configuration loop */
        ARAD_DO_NOTHING_AND_EXIT;
    }

    for (pgm_ndx = pgm_ndx_min; pgm_ndx < pgm_ndx_max; ++pgm_ndx)
    {
		ARAD_KBP_LTR_SINGLE_SEARCH master_key_fields;
		ARAD_KBP_ACL_IN_MASTER_KEY_TYPE acl_type = ARAD_KBP_ACL_IN_MASTER_KEY_LSB_ONLY;
		uint32 master_key_len = 0;
		uint32 num_of_frwd_segments = 0;
		uint32 lsb_free_in_bytes = 0;

        if (SOC_SAND_GET_BIT(pgm_bmp_used, pgm_ndx) != 0x1){
            /* PMF Program not used */
            continue;
        }

        /* Get DB type and LTR ID from program or allocate
         * new program if necessary
         */
		arad_pp_prog_index_to_flp_app_get(unit,pgm_ndx,&prog_id);
        opcode = ARAD_KBP_FLP_PROG_TO_OPCODE(prog_id);
        if (ARAD_KBP_ENABLE_IPV4_DC && (search_id == 5) && !ARAD_KBP_IPV4DC_24BIT_FWD) {
            /* lookup 5 is not supported with Double Capacity */
            SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_ACTION_OUT_OF_RANGE_ERR, 20, exit);
        }
        
        res = arad_kbp_opcode_to_db_type(
                  unit,
                  opcode,
                  &db_type,
                  &ltr_id,
                  success
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 116, exit);

        if(SOC_SAND_SUCCESS != *success) {
            ARAD_DO_NOTHING_AND_EXIT;
        }

        sal_memset(&master_key_fields, 0x0, sizeof(master_key_fields));
		if(SOC_IS_JERICHO(unit)){
            /* In case IPv6 forwarding is not performed via KBP and the forwarding program is one of the supported IPv6 programs,
             * the IPv6 forwarding header segments, that reside at the end of the master key, should be removed
             * and added back after the ACL key.
             * Additionally the LUT record size and the GTM opcode tx_data_size are updated accordingly.
             *
             * The IPv6 header segments have to be in the format "UNUSED_HEADER_PART" "SIP" "DIP".
             * If they are not found at the end of the master key, nothing is removed.
             */
            if(elk->kbp_no_fwd_ipv6_dip_sip_sharing_disable == 0) {
                if(ARAD_PP_FLP_IPV6_DIP_SIP_SHARING_IS_PROG_VALID(opcode)) {
                    arad_kbp_forwarding_header_segments_remove_from_ltr_master_key(unit, db_type);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 117, exit);
                }
            }

			/* Initialize search format */
			/* A. Get the maskter-key status (its current length and segment's division).
			 *    Why is it important? 
			 *    1. If the current ACL will cross the 80 bits length of the master-key:
			 *  	- Need to split it into two parts, LSB and MSB parts (under 80bits and above 80 bits of the total master-key)
			 *    2. If not, the size of the ACL is less than or equal to 10 bytes. 
			 */

			arad_kbp_get_master_key_info_by_ltr_id(unit, ltr_id, &num_of_frwd_segments, &master_key_fields);

			/*
			*    calculate the current length of the master key - only ACLs part!!
			*    i - running index, start from num_of_frwd_segments
			*/
			for (i=num_of_frwd_segments;i<master_key_fields.nof_key_segments;i++) {
				master_key_len += master_key_fields.key_segment[i].nof_bytes;
			}

			if(master_key_len >= 10){
				acl_type = ARAD_KBP_ACL_IN_MASTER_KEY_MSB_ONLY;
			}
			else if ((master_key_len < 10) && (key_size_in_bytes+master_key_len > 10)){
				acl_type = ARAD_KBP_ACL_IN_MASTER_KEY_LSB_MSB;
				lsb_free_in_bytes = 10 - master_key_len;
			}
			else{
				acl_type = ARAD_KBP_ACL_IN_MASTER_KEY_LSB_ONLY;
			}
		
			memset(&search_format, 0, sizeof(search_format));
			search_format.nof_key_segments = (acl_type == ARAD_KBP_ACL_IN_MASTER_KEY_LSB_MSB) ? 2 : 1;
			search_format.nof_key_segments = search_format.nof_key_segments+nof_shared_segments;

			for (i = 0; i < nof_shared_segments; i++) {        
				search_format.key_segment[i].nof_bytes = shared_quals_info[i].size;
				sal_memcpy(search_format.key_segment[i].name, shared_quals_info[i].name, 20);
				search_format.key_segment[i].type = KBP_KEY_FIELD_TERNARY;
			}

			segment_idx = i;

			/*
			*    ARAD_KBP_ACL_IN_MASTER_KEY_LSB_MSB
			*    - two segments for acl - LSB (ACL_s0) and MSB (ACL_s1)
			*   LSBONLY/ MSB_ONLY
			*    - one segment for acl  - (ACL_s0)
			*/
			if(acl_type == ARAD_KBP_ACL_IN_MASTER_KEY_LSB_MSB){
				search_format.key_segment[segment_idx].nof_bytes = lsb_free_in_bytes;
				sal_sprintf(search_format.key_segment[segment_idx].name, "ACL%d-s0", table_id);
				search_format.key_segment[segment_idx].type = KBP_KEY_FIELD_TERNARY;
				segment_idx++;
				search_format.key_segment[segment_idx].nof_bytes = key_size_in_bytes - lsb_free_in_bytes;
				sal_sprintf(search_format.key_segment[segment_idx].name, "ACL%d-s1", table_id);
				search_format.key_segment[segment_idx].type = KBP_KEY_FIELD_TERNARY;
			}else{
				search_format.key_segment[segment_idx].nof_bytes = key_size_in_bytes;
				sal_sprintf(search_format.key_segment[segment_idx].name, "ACL%d-s0", table_id);
				search_format.key_segment[segment_idx].type = KBP_KEY_FIELD_TERNARY;
			}
		}
		
        /* Add table to KBP instruction (LTR search) */
        res = arad_kbp_ltr_lookup_add(
                   unit,
                   search_id,
                   opcode,
                   ltr_id,
                   flags,
                   table_id,
                   ARAD_KBP_DB_TYPE_ACL, /* database type */
				   acl_type,
                   &search_format,
                   nof_shared_segments,
                   success
               );
        SOC_SAND_CHECK_FUNC_RESULT(res, 115, exit);

        /* In case of IPv6 forwarding is not performed via KBP and the forwarding program is one of the supported IPv6 programs,
         * the IPv6 forwarding header segments should be added at the end of the master key.
         * Additionally the LUT record size and the GTM opcode tx_data_size are updated accordingly.
         */
        if(SOC_IS_JERICHO(unit)) {
            if(elk->kbp_no_fwd_ipv6_dip_sip_sharing_disable == 0) {
                if(ARAD_PP_FLP_IPV6_DIP_SIP_SHARING_IS_PROG_VALID(opcode)) {
                    arad_kbp_forwarding_header_segments_append_to_ltr_master_key(unit, db_type);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 118, exit);
                }
            }
        }

        if(SOC_SAND_SUCCESS != *success) {
            ARAD_DO_NOTHING_AND_EXIT;
        }

        /* Add LTR to Opcode */
        res = arad_kbp_add_ltr_to_opcode(
                unit,
                db_type,
                flags,
                opcode,
                ltr_id,
                key_size_in_bytes,
                search_id,
                &search_format,
                nof_shared_segments,
                success
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 115, exit);

        if(SOC_SAND_SUCCESS != *success) {
            ARAD_DO_NOTHING_AND_EXIT;
        }

		/* If an ACL was assign to a MC dummy table-> (MC + search#1)
		 * it was ovwrwitten a dummy lookup. so we need to invalid its dummy table. 
		 * the KBP cannot be loaded with valid table that non of the LTRs are using it 
		 */
		if ((pgm_ndx == PROG_FLP_IPV6MC) && (search_id == 1) && !(flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY)) {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit,ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC, FALSE); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
		}
		if ((pgm_ndx == PROG_FLP_IPV4COMPMC_WITH_RPF) && (search_id == 1) && !(flags & ARAD_KBP_TABLE_ALLOC_CHECK_ONLY)) {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.set(unit,ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC, FALSE); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
		}

        /* update the program if configuration was successfull */
        SHR_BITSET(pgm_bmp_new, pgm_ndx);
    }
   
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_add_acl()", 0, 0);
}

void 
    arad_kbp_sw_config_print(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  int  advanced
    )
{
    uint32 
        idx;

    ARAD_KBP_TABLE_CONFIG           Arad_kbp_table_config_info = {0};
    arad_kbp_frwd_ltr_db_t          Arad_kbp_gtm_ltr_info = {0};
    ARAD_KBP_LTR_CONFIG             ltr_config_info = {0};
	arad_kbp_lut_data_t 			lut_data = {0};
	ARAD_KBP_GTM_OPCODE_CONFIG_INFO gtm_config_info = {0};

    ARAD_KBP_TABLE_CONFIG_clear(&Arad_kbp_table_config_info);
    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);
    arad_kbp_frwd_ltr_db_clear(&Arad_kbp_gtm_ltr_info);
	ARAD_KBP_GTM_OPCODE_CONFIG_INFO_clear(&gtm_config_info);

    if(NULL == AradAppData[unit])
    {
        LOG_CLI((BSL_META_U(unit,
                            "\n***** KBP Device is not yet installed with the following configuration!\n\n")));
        return;
    }

    LOG_CLI((BSL_META_U(unit,
                        "\nTable Configuration\n\n")));
    LOG_CLI((BSL_META_U(unit,
                        "%-10s"),"Tbl-ID"));
    LOG_CLI((BSL_META_U(unit,
                        "%-21s"),"Tbl-Name"));
    LOG_CLI((BSL_META_U(unit,
                        "%-10s"),"Size"));
    LOG_CLI((BSL_META_U(unit,
                        "%-10s"),"Width"));
    LOG_CLI((BSL_META_U(unit,
                        "%-10s"),"AD Width"));
    LOG_CLI((BSL_META_U(unit,
                        "%-10s"),"Num ent."));
    LOG_CLI((BSL_META_U(unit,
                        "%-10s\n"),"~Capacity"));
    LOG_CLI((BSL_META_U(unit,
                        "--------------------------------------------------------------------------------\n")));

    for ( idx = 0; 
          idx < ARAD_KBP_MAX_NUM_OF_TABLES; 
          idx++) 
    {
        if(advanced == 0 && /* don't show dummy tables when using basic print */
                (idx >= ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC && idx <= ARAD_KBP_FRWRD_TBL_ID_DUMMY_5))
        {
            continue;
        }
        sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.get(unit, idx, &Arad_kbp_table_config_info);         
        if (Arad_kbp_table_config_info.valid) 
        {
            ARAD_KBP_TABLE_CONFIG_print(unit, &Arad_kbp_table_config_info);
        }
    }

    LOG_CLI((BSL_META_U(unit,
                        "\n\nApplication Configuration\n")));

    for ( idx = 0; 
          idx < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; 
          idx++) 
    {
        sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, idx, &ltr_config_info);
        if((advanced == 0) && (idx == ARAD_KBP_FRWRD_DB_TYPE_DUMMY_FRWRD))
            continue;
        if (ltr_config_info.valid) 
        {
            LOG_CLI((BSL_META_U(unit,
                                "--------------------------------------------------------------------------------\n")));
            LOG_CLI((BSL_META_U(unit,
                                "LTR Index: %d, program: %s\n"), idx, ARAD_KBP_FRWRD_IP_DB_TYPE_to_string((ARAD_KBP_FRWRD_IP_DB_TYPE)idx)));
            ARAD_KBP_LTR_CONFIG_print(&ltr_config_info, advanced);
            LOG_CLI((BSL_META_U(unit,
                                "\n")));

            sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.get(unit, idx, &Arad_kbp_gtm_ltr_info);
            arad_kbp_frwd_ltr_db_print(&Arad_kbp_gtm_ltr_info, advanced);
            LOG_CLI((BSL_META_U(unit,
                                "\n")));

            if (advanced == 2) {
                           LOG_CLI((BSL_META_U(unit,"  Advanced debug\n")));
                           sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.get(unit, idx, &gtm_config_info);
                           ARAD_KBP_GTM_OPCODE_CONFIG_INFO_print(&gtm_config_info);

                           sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.get(unit, idx, &lut_data); 
                           arad_kbp_frwd_lut_db_print(&lut_data);

            }
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }
}

void
    arad_kbp_device_print(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  char*   file_name
    )
{
    FILE *kbp_file;
    uint core = 0;
    int res;

    if (AradAppData[unit]->device_p == NULL ||
        file_name == NULL) 
    {
        return;
    }

    kbp_file = fopen(file_name,"w");

    if (kbp_file != NULL) {
        res = kbp_device_print_html(AradAppData[unit]->device_p[core], kbp_file);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_device_print_html failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
        }
        fclose(kbp_file);
    }
}

/* 
 *  Table Get functions
 */
uint32 arad_kbp_alg_kbp_db_get(
   int unit,
   uint32 frwrd_tbl_id,
   struct kbp_db **db_p)
{
    uint32 res = SOC_SAND_OK;
    *db_p = AradAppData[unit]->g_gtmInfo[frwrd_tbl_id].tblInfo.db_p;
    return res;
}

uint32 arad_kbp_alg_kbp_ad_db_get(
   int unit,
   uint32 frwrd_tbl_id,
   struct kbp_ad_db **ad_db_p)
{
    uint32 
        res = SOC_SAND_OK;
    *ad_db_p = AradAppData[unit]->g_gtmInfo[frwrd_tbl_id].tblInfo.ad_db_p;
    return res;
}

uint32 
  arad_kbp_ltr_get_inst_pointer(
    SOC_SAND_IN  int                     	unit,
    SOC_SAND_IN  uint32      				ltr_idx,
    SOC_SAND_OUT struct  kbp_instruction    **inst_p
  )
{
    uint32 res;
    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, ltr_idx, &ltr_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if(ltr_config_info.valid){
        *inst_p = ltr_config_info.inst_p;
    } else {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_ltr_get_inst_pointer()", 0, 0);
}

uint32 
    arad_kbp_gtm_table_info_get(
       int unit,
       uint32 frwrd_tbl_id,
       tableInfo *tblInfo_p
    )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(tblInfo_p);

    if (AradAppData[unit] == NULL) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }
    else {
        sal_memcpy(tblInfo_p, 
                   &(AradAppData[unit]->g_gtmInfo[frwrd_tbl_id].tblInfo), 
                   sizeof(tableInfo));
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_gtm_table_info_get()", frwrd_tbl_id, 0);
}


uint32 
    arad_kbp_get_device_pointer(
        SOC_SAND_IN  int                unit,
        SOC_SAND_OUT struct kbp_device  **device_p
    )
{
    uint32 core = 0;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(device_p);

    if(AradAppData[unit]->device_p[core] != NULL) {
        *device_p = AradAppData[unit]->device_p[core];
    }
    else {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
    }
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_get_device_pointer()", 0, 0);
}

uint32 arad_kbp_gtm_ltr_num_get(
   int unit,
   uint32 frwrd_tbl_id,
   uint8 *ltr_num)
{
    uint32 res = SOC_SAND_OK;
    
    *ltr_num = AradAppData[unit]->g_gtmInfo[frwrd_tbl_id].ltr_num;

    return res;
}


int32_t arad_kbp_callback_mdio_read(void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t *value){
    uint32 kbp_reg_addr;
    kbp_init_user_data_t *user_data = handle;
    int rv;
    
    if(handle == NULL){
        return KBP_INVALID_ARGUMENT;
    }
    kbp_reg_addr = reg + (dev << 16);
    rv = soc_dcmn_miim_read(user_data->device, ARAD_KBP_MDIO_CLAUSE, user_data->kbp_mdio_id, kbp_reg_addr, value);
    if(SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META("%s(): soc_dcmn_miim_read() Failed\n"), FUNCTION_NAME()));
        return KBP_INTERNAL_ERROR;
    }
    return KBP_OK;
}

int32_t arad_kbp_callback_mdio_write(void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t value){
    uint32 kbp_reg_addr;
    kbp_init_user_data_t *user_data = handle;
    int rv;
    
    if(handle == NULL){
        return KBP_INVALID_ARGUMENT;
    }
    kbp_reg_addr = reg + (dev << 16);
    rv = soc_dcmn_miim_write(user_data->device, ARAD_KBP_MDIO_CLAUSE, user_data->kbp_mdio_id, kbp_reg_addr, value);
    if(SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META("%s(): arad_kbp_callback_mdio_write() Failed\n"), FUNCTION_NAME()));
        return KBP_INTERNAL_ERROR;
    }
    return KBP_OK;   
}


STATIC
int32_t arad_kbp_usleep_callback(void *handle, uint32_t usec){
    sal_usleep(usec);
    return KBP_OK;
}

/*
 *    Initialization functions functions
 */
uint32 arad_kbp_init_kbp_ilkn_port_set(
    int unit,
    uint32 core,
    uint32 kbp_mdio_id,
    int ilkn_num_lanes, 
    int ilkn_rate, 
    int ilkn_meta_data,
    uint32 kbp_ilkn_rev,
    kbp_reset_f kbp_reset)
{
    int rc;
    struct kbp_device_config kbp_config = KBP_DEVICE_CONFIG_DEFAULT;
    enum kbp_device_type kbp_type;
    kbp_init_user_data_t handle;
    uint32_t flags = KBP_DEVICE_DEFAULT;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* KBP mdio ID format (kbp_mdio_id):
     * bits [5:6,8:9] - bus ID. Arad has 8 external buses (0-7).
     * bit [7] - Internal select. Set to 0 for external phy access.
     * bits [0:4] - phy/kbp id 
     *  
     * Adress format (according to kbp register spec): 
     * Mdio address =  [15:0] 
     * Mdio Device ID = [23:16] 
     */

    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        if (core == 1) {
            /*
             * On OP ignore the serdes setting for core 1, as we connect to single chip
             */
            return 0;
        }
        kbp_type = KBP_DEVICE_OP;
    }
    else
        kbp_type = KBP_DEVICE_12K;

   LOG_VERBOSE(BSL_LS_SOC_TCAM,
               (BSL_META_U(unit,
                           "%s(): Start. kbp_mdio_id=0x%x, kbp_ilkn_rev=%d.\n"), FUNCTION_NAME(), kbp_mdio_id, kbp_ilkn_rev));

    switch(ilkn_rate){
    case 6250:
        kbp_config.speed = KBP_INIT_LANE_6_25;
        break;
    case 10312:
        kbp_config.speed = KBP_INIT_LANE_10_3;
        break;
    case 12500:
        kbp_config.speed = KBP_INIT_LANE_12_5;
        break;
    case 25781:
        kbp_config.speed = KBP_INIT_LANE_25_78125;
        break;
    case 27343:
        kbp_config.speed = KBP_INIT_LANE_27_34375;
        break;
    default:
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in %s(): Error, unsupported ilkn_rate=%d.\n"), FUNCTION_NAME(), ilkn_rate));
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 70, exit);  
    }

    if (kbp_type == KBP_DEVICE_OP) {
        flags |= KBP_DEVICE_DUAL_PORT;
    }
    if((ilkn_num_lanes == 4) || (ilkn_num_lanes == 8) || (ilkn_num_lanes == 12) ||
       ((ilkn_num_lanes == 6) && (kbp_type == KBP_DEVICE_OP))){
        kbp_config.port_map[0].num_lanes = ilkn_num_lanes;
        if (kbp_type == KBP_DEVICE_OP) {
            kbp_config.port_map[1].start_lane = 20;
            kbp_config.port_map[1].num_lanes = ilkn_num_lanes;
        }
    }
    else{
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in %s(): Unsupported ilkn_num_lanes=%d.\n"), FUNCTION_NAME(), ilkn_num_lanes));
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 80, exit);
    }
    kbp_config.burst = KBP_INIT_BURST_SHORT_16_BYTES;
    /*metframe length set*/
    if (ilkn_meta_data == 64 || ilkn_meta_data == 2048) {
        kbp_config.meta_frame_len = ilkn_meta_data;
    }
    else{
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in %s(): Unsupported ilkn_meta_data=%d.\n"), FUNCTION_NAME(), ilkn_meta_data));
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 90, exit);
    }
    handle.device = unit;
    handle.kbp_mdio_id = kbp_mdio_id;
    kbp_config.handle = &handle;

    
    /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

    /* clause 22 disable */
    arad_kbp_callback_mdio_write(&handle, 0, 1, 0xF0FD, 0x0001);
    arad_kbp_callback_mdio_write(&handle, 0, 1, 0xF0FF, 0xFFFF);
    arad_kbp_callback_mdio_write(&handle, 0, 1, 0xFFDD, 0x004C);
    arad_kbp_callback_mdio_write(&handle, 0, 1, 0xF0FD, 0x0000);

    /* Override ROP mode */
    arad_kbp_callback_mdio_write(&handle, 0, 0, 0x0054, 0x8004);

    /* PCIE workaround */
    arad_kbp_callback_mdio_write(&handle, 0, 0x11, 0x6109, 0x2080);

    
    /* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

    /*callbacks set*/
    kbp_config.mdio_read = arad_kbp_callback_mdio_read;
    kbp_config.mdio_write = arad_kbp_callback_mdio_write;
    kbp_config.assert_kbp_resets = kbp_reset;
    kbp_config.usleep = arad_kbp_usleep_callback;
    kbp_config.pre_enable_link_callback = NULL;
    kbp_config.reverse_lanes = kbp_ilkn_rev;

    /*config the kbp*/
    rc = kbp_device_interface_init(kbp_type, flags, &kbp_config);
    if(ARAD_KBP_TO_SOC_RESULT(rc) != SOC_SAND_OK){
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in %s(): configuring elk device. %s\n"), FUNCTION_NAME(), kbp_get_status_string(rc)));
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 100, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_kbp_ilkn_port_set()", ilkn_num_lanes, ilkn_rate);
}


uint32 arad_kbp_init_egw_config_set(
    int unit,
    uint32 core,
    int ilkn_num_lanes, 
    int ilkn_rate)
{
    uint32
        res,
        reg_val = 0x0,
        fld_val = 0x0,
        ilkn_total_mbits_rate,
        ilkn_total_burst_rate,
        core_clk_ticks,
        spr_dly_mega,
        spr_dly_fld,
        spr_dly_fraction_fld;
    uint64
        reg64_val = 0x0;
    soc_reg_above_64_val_t
        fld_above64_val,
        reg_above64_val;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above64_val);
    SOC_REG_ABOVE_64_CLEAR(fld_above64_val);

    if(!SAL_BOOT_PLISIM){

    /* Minimum/Maximum packet size in B, up to which packet not closed */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_PACKET_SIZEr(unit, core, &reg_val));
    fld_val = 64;
    soc_reg_field_set(unit, IHB_PACKET_SIZEr, &reg_val, MIN_PKT_SIZEf, fld_val);
    fld_val = 256;
    soc_reg_field_set(unit, IHB_PACKET_SIZEr, &reg_val, MAX_PKT_SIZEf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_PACKET_SIZEr(unit, core, reg_val));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_TRANSMIT_CFGSr(unit, core, &reg64_val));
    /* EGW will not transmit FLP records if FIFO exceed this threshold */
    fld_val = 0x190;
    soc_reg64_field_set(unit, IHB_TRANSMIT_CFGSr, &reg64_val, FLP_TRANSMITING_THRESHOLDf, fld_val);
    /* congestion identified when FIFO exceed this threshold (enables ROP style packing and/or MaxPktSize reaching) */
    fld_val = 0x14;
    soc_reg64_field_set(unit, IHB_TRANSMIT_CFGSr, &reg64_val, CONGESTION_THRESHOLDf, fld_val);
    /* Enable ROP style packing */
    fld_val = 0x1;
    soc_reg64_field_set(unit, IHB_TRANSMIT_CFGSr, &reg64_val, PACKING_ENABLEf, fld_val);
#ifdef BCM_88660_A0
    if (SOC_IS_ARADPLUS(unit)){
        /* set CPU priority over traffic lookups */
        fld_val = 0x1;
        soc_reg64_field_set(unit, IHB_TRANSMIT_CFGSr, &reg64_val, CPU_RECORD_PRIOf, fld_val);
    }
#endif /* BCM_88660_A0 */
    if (SOC_DPP_CONFIG(unit)->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        fld_val = 0x1;
        soc_reg64_field_set(unit, IHB_TRANSMIT_CFGSr, &reg64_val, TIMER_MODEf, fld_val);
        fld_val = 0x10;
        soc_reg64_field_set(unit, IHB_TRANSMIT_CFGSr, &reg64_val, TIMER_DELAYf, fld_val);
    }
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_TRANSMIT_CFGSr(unit, core, reg64_val));

    /* Enables Attaching application prefix in packet transmition/reciving */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_APP_PREFIX_CONTROLr(unit, core, &reg_val));
    fld_val = 0x0;
    soc_reg_field_set(unit, IHB_APP_PREFIX_CONTROLr, &reg_val, TX_APP_PREFIX_ENABLEf, fld_val);
    fld_val = 0x0;
    soc_reg_field_set(unit, IHB_APP_PREFIX_CONTROLr, &reg_val, RX_APP_PREFIX_ENABLEf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_APP_PREFIX_CONTROLr(unit, core, reg_val));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_PHY_CHANNEL_CFGSr(unit, core, &reg_val));
    fld_val = 0x0;
    soc_reg_field_set(unit, IHB_PHY_CHANNEL_CFGSr, &reg_val, TX_CHANNEL_NUMf, fld_val);
    fld_val = 0x0;
    soc_reg_field_set(unit, IHB_PHY_CHANNEL_CFGSr, &reg_val, RX_CHANNEL_NUMf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_PHY_CHANNEL_CFGSr(unit, core, reg_val));

    /* 
     * Shaper configuration
     */
    /* ilkn_num_lanes = 12; */
    ilkn_total_mbits_rate = ilkn_num_lanes * ilkn_rate;
    ilkn_total_burst_rate = ilkn_total_mbits_rate / (32 * 8);
    core_clk_ticks = arad_chip_ticks_per_sec_get(unit);
    spr_dly_mega = core_clk_ticks / ilkn_total_burst_rate;

    spr_dly_fld = spr_dly_mega / 1000000; 
    /* Need to find the fraction = ((64 * 1024 * dly_mega) / 1000000).
     * since the fraction is = x/64K. 64/1000000 = 1/15625 
     */ 
    spr_dly_fraction_fld = ((spr_dly_mega % 1000000) * 1024) / 15625;
   LOG_VERBOSE(BSL_LS_SOC_TCAM,
               (BSL_META_U(unit,
                           "%s(): Shaper configuration ilkn_total_mbits_rate=%d, ilkn_total_burst_rate=%d, core_clk_ticks=%d\n"), 
                           FUNCTION_NAME(), ilkn_total_mbits_rate, ilkn_total_burst_rate, core_clk_ticks)); 
   LOG_VERBOSE(BSL_LS_SOC_TCAM,
               (BSL_META_U(unit,
                           "%s(): Shaper configuration spr_dly_fld=%d, spr_dly_fraction_fld=%d\n"), 
                           FUNCTION_NAME(), spr_dly_fld, spr_dly_fraction_fld));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_SPR_DLY_CFGSr(unit, core, &reg64_val));
    soc_reg64_field_set(unit, IHB_SPR_DLY_CFGSr, &reg64_val, SPR_DLYf, spr_dly_fld);
    soc_reg64_field_set(unit, IHB_SPR_DLY_CFGSr, &reg64_val, SPR_DLY_FRACTIONf, spr_dly_fraction_fld);
    fld_val = 0x1ff;
    soc_reg64_field_set(unit, IHB_SPR_DLY_CFGSr, &reg64_val, SPR_MAX_BURSTf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_SPR_DLY_CFGSr(unit, core, reg64_val));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_SPR_CFGSr(unit, core, &reg_val));
    fld_val = 0x3;
    soc_reg_field_set(unit, IHB_SPR_CFGSr, &reg_val, WORD_ALIGNMENTf, fld_val);
    fld_val = 0x0;
    soc_reg_field_set(unit, IHB_SPR_CFGSr, &reg_val, PACKET_GAPf, fld_val);
    fld_val = 0x8;
    soc_reg_field_set(unit, IHB_SPR_CFGSr, &reg_val, PACKET_OVERHEADf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_SPR_CFGSr(unit, core, reg_val));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_INTERLAKEN_CFGSr(unit, core, &reg_val));
    fld_val = 0x1;
    soc_reg_field_set(unit, IHB_INTERLAKEN_CFGSr, &reg_val, ILKN_MODEf, fld_val);
    fld_val = 0x1;
    soc_reg_field_set(unit, IHB_INTERLAKEN_CFGSr, &reg_val, ILKN_MIN_BURSTf, fld_val);
    fld_val = 0x7;
    soc_reg_field_set(unit, IHB_INTERLAKEN_CFGSr, &reg_val, ILKN_MAX_BURSTf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_INTERLAKEN_CFGSr(unit, core, reg_val));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_GENERAL_CFGSr(unit, core, &reg_val));
    /* ***** Enables lookups to ELK ***** */
    fld_val = 0x1;
    soc_reg_field_set(unit, IHB_GENERAL_CFGSr, &reg_val, ENABLE_ELK_LOOKUPf, fld_val);
    fld_val = 0x0;
    /* PB leftover should be 0x0*/
    soc_reg_field_set(unit, IHB_GENERAL_CFGSr, &reg_val, NIF_TX_INIT_CREDITSf, fld_val);
    /* Error recovery time of ROP packet transmission, Jericho Plus has higher core clock, this vaule should be larger */
    fld_val = SOC_IS_JERICHO_PLUS_ONLY(unit) ? 0x514 : 0x3E8;
    soc_reg_field_set(unit, IHB_GENERAL_CFGSr, &reg_val, TIMEOUT_DLYf, fld_val);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_GENERAL_CFGSr(unit, core, reg_val));
    if (!SOC_IS_JERICHO_PLUS(unit)) {
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHP_FLP_GENERAL_CFGr_REG64(unit, core, &reg64_val));
        /* ***** Enables lookups to ELK ***** */
        fld_val = 0x1;
        soc_reg64_field_set(unit, IHP_FLP_GENERAL_CFGr, &reg64_val, ELK_ENABLEf, fld_val);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHP_FLP_GENERAL_CFGr_REG64(unit, core, reg64_val));
    } else {
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHP_FLP_GENERAL_CFGr(unit, core, reg_above64_val));
        /* ***** Enables lookups to ELK ***** */
        fld_above64_val[0] = 0x1;
        soc_reg_above_64_field_set(unit, IHP_FLP_GENERAL_CFGr, reg_above64_val, ELK_ENABLEf, fld_above64_val);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHP_FLP_GENERAL_CFGr(unit, core, reg_above64_val));
    }
    /* ***** Enables ELK on ilkn port 10/16 ***** */
    fld_val = 0x1; /*ILKN port 16 val*/
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)){
#ifdef BCM_88660_A0
        if (SOC_IS_ARADPLUS(unit)) {
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_NBI_ELK_CFG_ON_ILKN_PORT_10r(unit, &reg_val));
            if(SOC_DPP_CONFIG(unit)->arad->init.elk.ext_interface_mode){
                soc_reg_field_set(unit, NBI_ELK_CFG_ON_ILKN_PORT_10r, &reg_val, ELK_ENABLE_ON_ILKN_PORT_10f, 1);
                fld_val = 0; /*change the value of ELK on ILKN port 16*/
                switch(ilkn_num_lanes){
                case 4:
                    soc_reg_field_set(unit, NBI_ELK_CFG_ON_ILKN_PORT_10r, &reg_val, ELK_ENABLE_ON_ILKN_PORT_10_USING_FOUR_LANESf, 1);
                    break;
                case 8:
                    soc_reg_field_set(unit, NBI_ELK_CFG_ON_ILKN_PORT_10r, &reg_val, ELK_ENABLE_ON_ILKN_PORT_10_USING_FOUR_LANESf, 1);
                    soc_reg_field_set(unit, NBI_ELK_CFG_ON_ILKN_PORT_10r, &reg_val, ELK_ENABLE_ON_ILKN_PORT_10_USING_EIGHT_LANESf, 1);
                    break;
                default:
                     LOG_ERROR(BSL_LS_SOC_TCAM,
                               (BSL_META_U(unit,
                                           "Error in %s(): External interface mode unsupported ilkn_num_lanes=%d. (supported num of lanes 4/8)\n"), FUNCTION_NAME(), ilkn_num_lanes));
                     SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 11, exit);
                }
            }
            else{
                soc_reg_field_set(unit, NBI_ELK_CFG_ON_ILKN_PORT_10r, &reg_val, ELK_ENABLE_ON_ILKN_PORT_10f, 0);
            }
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_NBI_ELK_CFG_ON_ILKN_PORT_10r(unit, reg_val));
        }
#endif
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_NBI_ELK_CFGr(unit, &reg_val));
        soc_reg_field_set(unit, NBI_ELK_CFGr, &reg_val, ELK_ENABLE_ON_ILKN_PORT_16f, fld_val);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_NBI_ELK_CFGr(unit, reg_val));
    }
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_egw_config_set()", ilkn_num_lanes, ilkn_rate);
}

uint32 arad_kbp_init_egw_default_opcode_set(
    int unit)
{
    uint32
        res,
        mem_filed = 0x0;
    uint32
        table_entry[1];
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Configure in the EGW the used OPCODES */
    /* LUT_WR. Opcode ARAD_KBP_CPU_WR_LUT_OPCODE = 255 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_WR_LUT_OPCODE, table_entry));
    /* Opcode 255 is 10B data (80 bit) */
    mem_filed = 9;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 255 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_WR_LUT_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_WR_LUT_OPCODE, table_entry));
    /* Reply is 1B */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_WR_LUT_OPCODE, table_entry));

    /* LUT_RD. Opcode ARAD_KBP_CPU_RD_LUT_OPCODE = 254 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_RD_LUT_OPCODE, table_entry));
    /* Opcode 254 is 1B data (8 bit) */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 254 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_RD_LUT_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_RD_LUT_OPCODE, table_entry));
    /* Reply is 10B */
    mem_filed = 9;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_RD_LUT_OPCODE, table_entry));

    /* PIOWR. Opcode ARAD_KBP_CPU_PIOWR_OPCODE = 253 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIOWR_OPCODE, table_entry));
    /* Opcode 253 is 24B data */
    mem_filed = 23;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 253 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIOWR_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIOWR_OPCODE, table_entry));
    /* Reply is 1B */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIOWR_OPCODE, table_entry));

    /* PIORD-X. Opcode ARAD_KBP_CPU_PIORDX_OPCODE = 252 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDX_OPCODE, table_entry));
    /* Opcode 252 is 4B data */
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 252 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDX_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDX_OPCODE, table_entry));
    /* Reply is 11B */
    mem_filed = 10;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDX_OPCODE, table_entry));

    /* PIORD-Y. Opcode ARAD_KBP_CPU_PIORDY_OPCODE = 251 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
    /* Opcode 251 is 4B data */
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 251 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
    /* Reply is 11B */
    mem_filed = 10;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
  
    /* PIORD-Y. Opcode ARAD_KBP_CPU_CTX_BUFF_WRITE_OPCODE = 251 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
    /* Opcode 251 is 4B data */
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 251 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));
    /* Reply is 11B */
    mem_filed = 10;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_PIORDY_OPCODE, table_entry));

    /* Block Copy. Opcode ARAD_KBP_CPU_BLK_COPY_OPCODE = 249 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_COPY_OPCODE, table_entry));
    /* Opcode 249 is 8B data */
    mem_filed = 7;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 249 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_COPY_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_COPY_OPCODE, table_entry));
    /* Reply is 1B */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_COPY_OPCODE, table_entry));

   /* Block move. Opcode ARAD_KBP_CPU_BLK_MOVE_OPCODE = 248 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_MOVE_OPCODE, table_entry));
    /* Opcode 248 is 8B data */
    mem_filed = 7;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 248 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_MOVE_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_MOVE_OPCODE, table_entry));
    /* Reply is 1B */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_MOVE_OPCODE, table_entry));

   /* Block clear. Opcode ARAD_KBP_CPU_BLK_CLR_OPCODE = 247 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_CLR_OPCODE, table_entry));
    /* Opcode 247 is 5B data */
    mem_filed = 4;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 247 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_CLR_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_CLR_OPCODE, table_entry));
    /* Reply is 1B */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 115, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_CLR_OPCODE, table_entry));

   /* Block invalidate. Opcode ARAD_KBP_CPU_BLK_CLR_OPCODE = 246 */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 120, exit, READ_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_EV_OPCODE, table_entry));
    /* Opcode 246 is 5B data */
    mem_filed = 4;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, mem_filed);
    /* Opcode 246 is Type request (not info) */
    mem_filed = 1;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 125, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_EV_OPCODE, table_entry));
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 130, exit, READ_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_EV_OPCODE, table_entry));
    /* Reply is 1B */
    mem_filed = 0;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, mem_filed);
    mem_filed = 3;
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, mem_filed);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 135, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, SOC_CORE_ALL, ARAD_KBP_CPU_BLK_EV_OPCODE, table_entry));
        
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_egw_default_opcode_set()", 0x0, 0x0);
}


uint32 arad_kbp_ilkn_interface_param_get( 
    int         unit,
    uint32      core,
    soc_port_t *ilkn_port,
    uint32     *ilkn_num_lanes,
    int        *ilkn_rate,
    uint32     *ilkn_metaframe)
{
    uint32
        res = SOC_SAND_OK,
        offset;
    soc_error_t
        rv;
    soc_port_t
        port;
    soc_pbmp_t 
        ports_bm;
    soc_port_if_t 
        interface_type;
    ARAD_PORTS_ILKN_CONFIG 
        *ilkn_config;
    uint32 flags;
    int core_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if(!SAL_BOOT_PLISIM){

    /* Find ILKN1 port number */
    *ilkn_port = -1;
    rv = soc_port_sw_db_valid_ports_get(unit, 0x0, &ports_bm);
    if(SOC_FAILURE(rv)) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 2, exit);
    }
    SOC_PBMP_ITER(ports_bm, port){
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 3, exit, soc_port_sw_db_core_get(unit, port, &core_id));

        if (core != core_id) {
            continue;
        }
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 3, exit, soc_port_sw_db_interface_type_get(unit, port, &interface_type)); 
        if (SOC_PORT_IF_ILKN == interface_type) {
            rv = soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset); 
            ilkn_config = &SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn[offset];
            if(SOC_FAILURE(rv)) {
                SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 4, exit);
            }
            if (SOC_IS_JERICHO(unit)) {
                rv = soc_port_sw_db_flags_get(unit, port, &flags);
                if(SOC_FAILURE(rv)) {
                    SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 4, exit);
                }
                if (SOC_PORT_IS_ELK_INTERFACE(flags)) {
                    if (*ilkn_port != -1) {
                        LOG_ERROR(BSL_LS_SOC_TCAM, (BSL_META_U(unit, "Error in %s(): More than 1 ILKN KBP port found.\n"), FUNCTION_NAME()));    
                         SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 5, exit);
                    }
                    *ilkn_port = port;
                }
            } else {
                if (0x1 == offset) {
                    *ilkn_port = port;
                    break;
                }
            }

        }
    }
    if (*ilkn_port == -1) {
        if (!SOC_IS_JERICHO(unit)) {
           LOG_ERROR(BSL_LS_SOC_TCAM, (BSL_META_U(unit, "Error in %s(): No ILKN1 port found.\n"), FUNCTION_NAME()));    
           SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 5, exit);
        } else {
           LOG_VERBOSE(BSL_LS_SOC_TCAM, (BSL_META_U(unit, "%s(): No ILKN port found for core %u\n"), FUNCTION_NAME(), core));    
           soc_sand_set_error_code_into_error_word(SOC_SAND_GEN_ERR, &ex);
           goto exit_wo_print;
        }
    }

    /* get ILKN configuration */
    rv = soc_port_sw_db_num_lanes_get(unit, *ilkn_port, ilkn_num_lanes);
    if(SOC_FAILURE(rv)) {
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 10, exit);
    }
    rv = soc_port_sw_db_speed_get(unit, *ilkn_port, ilkn_rate);
    if(SOC_FAILURE(rv)) {
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 20, exit);
    }
    rv = soc_port_sw_db_protocol_offset_get(unit, *ilkn_port, 0, &offset);
    ilkn_config = &SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn[offset];
    if(SOC_FAILURE(rv)) {
        SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_GEN_ERR, 30, exit);
    }
    *ilkn_metaframe = ilkn_config->metaframe_sync_period;
    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META_U(unit,
                            "%s(): ilkn_port=%d, ilkn_num_lanes=%d, ilkn_rate=%d, ilkn_metaframe=%d\n"), 
                            FUNCTION_NAME(), *ilkn_port, *ilkn_num_lanes, *ilkn_rate, *ilkn_metaframe));    
    }

exit_wo_print:
    return ex;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_ilkn_interface_param_get()", *ilkn_num_lanes, *ilkn_rate);
}

uint32 arad_kbp_init_kbp_interface( 
    int unit,
    uint32 core,
    uint32 kbp_mdio_id,
    uint32 kbp_ilkn_rev,
    kbp_reset_f kbp_reset)
{
    uint32
        res,
        ilkn_num_lanes,
        ilkn_metaframe,
        offset;
    soc_port_t
        ilkn_port;
    int
        ilkn_rate;
   
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if(!SAL_BOOT_PLISIM){

    SOC_DPP_CONFIG(unit)->arad->init.elk.kbp_mdio_id[core] = kbp_mdio_id;

    if (SOC_DPP_CONFIG(unit)->arad->init.elk.enable == 0x0) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): ELK disabled (ext_tcam_dev_type might be NONE)!!!\n"), FUNCTION_NAME()));    
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }
    if(kbp_reset == NULL){
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): Invalid reset function\n"), FUNCTION_NAME()));    
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
    }

    res = arad_kbp_ilkn_interface_param_get(unit, core, &ilkn_port, &ilkn_num_lanes, &ilkn_rate, &ilkn_metaframe);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    LOG_VERBOSE(BSL_LS_SOC_TCAM,
                (BSL_META_U(unit,
                            "%s(): kbp_mdio_id=0x%x, ilkn_port=%d, ilkn_num_lanes=%d, ilkn_rate=%d, ilkn_metaframe=%d\n"), 
                            FUNCTION_NAME(), kbp_mdio_id, ilkn_port, ilkn_num_lanes, ilkn_rate, ilkn_metaframe));    

    /* KBP ILKN config needed */
    /* using the ElkIlknRev mode from soc properties if exists */
    res = soc_port_sw_db_protocol_offset_get(unit, ilkn_port, 0, &offset);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    kbp_ilkn_rev = soc_property_port_get(unit, offset, spn_EXT_ILKN_REVERSE, kbp_ilkn_rev);
    res = arad_kbp_init_kbp_ilkn_port_set(unit, core, kbp_mdio_id, ilkn_num_lanes, ilkn_rate, ilkn_metaframe, kbp_ilkn_rev, kbp_reset);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_kbp_interface()", ilkn_num_lanes, ilkn_rate);
}

uint32 arad_kbp_blk_lut_set(int unit, uint32 core)
{
    uint32
        res = SOC_SAND_OK;
    arad_kbp_lut_data_t 
        lut_data;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    sal_memset(&lut_data, 0, sizeof(lut_data));

    /* Block copy instruction */
    lut_data.instr = ARAD_KBP_CPU_BLK_COPY_INSTRUCTION;
    lut_data.rec_size = ARAD_KBP_CPU_BLK_COPY_REC_SIZE;
    lut_data.rec_is_valid = 1;   
    res = arad_kbp_lut_write(unit, core, ARAD_KBP_CPU_BLK_COPY_OPCODE, &lut_data, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Block move instruction */
    lut_data.instr = ARAD_KBP_CPU_BLK_MOVE_INSTRUCTION;
    lut_data.rec_size = ARAD_KBP_CPU_BLK_MOVE_REC_SIZE;
    lut_data.rec_is_valid = 1;   
    res = arad_kbp_lut_write(unit, core, ARAD_KBP_CPU_BLK_MOVE_OPCODE, &lut_data, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* Block clear instruction */
    lut_data.instr = ARAD_KBP_CPU_BLK_CLR_INSTRUCTION;
    lut_data.rec_size = ARAD_KBP_CPU_BLK_CLR_REC_SIZE;
    lut_data.rec_is_valid = 1;   
    res = arad_kbp_lut_write(unit, core, ARAD_KBP_CPU_BLK_CLR_OPCODE, &lut_data, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* Block invalidate instruction */
    lut_data.instr = ARAD_KBP_CPU_BLK_EV_INSTRUCTION;
    lut_data.rec_size = ARAD_KBP_CPU_BLK_EV_REC_SIZE;
    lut_data.rec_is_valid = 1;   
    res = arad_kbp_lut_write(unit, core, ARAD_KBP_CPU_BLK_EV_OPCODE, &lut_data, NULL);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_blk_lut_set()", 0, 0);
}

uint32 arad_kbp_init_arad_interface( 
    int unit)
{
    uint32
        res = SOC_SAND_OK,
        ilkn_num_lanes,
        ilkn_metaframe;
    soc_port_t
        ilkn_port;
    int
        ilkn_rate;
    uint64
        reg_val;
    uint32 core;
    uint32 one_kbp_ilkn_at_least = 0;
   
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_DPP_CORES_ITER(_SHR_CORE_ALL, core) {
        res = arad_kbp_ilkn_interface_param_get(unit, core, &ilkn_port, &ilkn_num_lanes, &ilkn_rate, &ilkn_metaframe);
        if (res == 0) {
            one_kbp_ilkn_at_least = 1;
           LOG_VERBOSE(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "%s(): ilkn_port=%d, ilkn_num_lanes=%d, ilkn_rate=%d, ilkn_metaframe=%d\n"), FUNCTION_NAME(), ilkn_port, ilkn_num_lanes, ilkn_rate, ilkn_metaframe));
       

           if (!SOC_IS_JERICHO(unit)) {
			    uint32 tx_1_burst_short_ilkn_config = soc_property_suffix_num_get(unit, 0, "tx_1_burst_short_ilkn_segment", "", 0);
                SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, READ_NBI_TX_1_ILKN_CONTROLr(unit, &reg_val));
                soc_reg64_field_set(unit, NBI_TX_1_ILKN_CONTROLr, &reg_val, TX_1_BURSTSHORTf, tx_1_burst_short_ilkn_config);
                SOC_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, WRITE_NBI_TX_1_ILKN_CONTROLr(unit, reg_val));
           }

           /* EGW config needed, might be according to ilkn configuration */
            res = arad_kbp_init_egw_config_set(unit, core, ilkn_num_lanes, ilkn_rate);
            SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
        }
    }

    if (one_kbp_ilkn_at_least == 0) {
       SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }

    res = arad_kbp_init_egw_default_opcode_set(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);


	if ( soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_ipv4_mc_flexible_fwd_table", 0) &&
		!soc_property_get(unit, spn_EXT_IP4_MC_FWD_TABLE_SIZE, 0x0) ) {
		res=-1;
        _bsl_error (_BSL_SOCDNX_MSG("soc property custom_feature_ext_ipv4_mc_flexible_fwd_table is on! make sure also ext_ip4_mc_fwd_table_size is defined")) ;  
		SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
	}
      
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_arad_interface()", ilkn_num_lanes, ilkn_rate);
}

/* This function performs Device Manager related Inits */
STATIC int arad_kbp_init_nlm_dev_mgr( 
   int unit, 
   int core)
{
    uint32 flags, is_12k;
    kbp_warmboot_t *warmboot_data;
    enum kbp_device_type kbp_type;
    int32 res;

#ifdef CRASH_RECOVERY_SUPPORT
    uint8 *nv_mem_ptr = NULL;
    unsigned int nv_mem_size = ARAD_KBP_NV_MEMORY_SIZE;
#endif
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    warmboot_data = &kbp_warmboot_data[unit];

    flags = KBP_DEVICE_DEFAULT | KBP_DEVICE_ISSU;
    if (SOC_WARM_BOOT(unit)) {
        flags |= KBP_DEVICE_SKIP_INIT;
#ifdef CRASH_RECOVERY_SUPPORT
        if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
            flags &= ~KBP_DEVICE_ISSU;
        }
#endif
    }
    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        if (core == 1) {
            /*
             * On OP ignore the device init for core 1
             */
            return 0;
        }
        kbp_type = KBP_DEVICE_OP;
        flags |= KBP_DEVICE_DUAL_PORT;
        is_12k = 0;
    }
    else {
        kbp_type = KBP_DEVICE_12K;
        is_12k = 1;
    }

    if(!SAL_BOOT_PLISIM){
        res = kbp_device_init(AradAppData[unit]->dalloc_p[core],
                            kbp_type, 
                            flags, 
                            AradAppData[unit]->alg_kbp_xpt_p[core], 
                            NULL,
                            &AradAppData[unit]->device_p[core]);
    } else{
        res = kbp_device_init(AradAppData[unit]->dalloc_p[core], 
                                kbp_type, 
                                flags, 
                                NULL, 
                                NULL,
                                &AradAppData[unit]->device_p[core]);
    }
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
        LOG_ERROR(BSL_LS_SOC_TCAM,
                  (BSL_META_U(unit,
                              "Error in %s(): kbp_device_init failed %s\n"), FUNCTION_NAME(), kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
    }

    /* This was added for ARAD as flow control was causing some issues with block operations on */
    if (is_12k && SOC_IS_ARADPLUS_AND_BELOW(unit)){
        res = kbp_device_set_property(AradAppData[unit]->device_p[core], KBP_DEVICE_PROP_INST_LATENCY, 1);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_device_set_property KBP_DEVICE_PROP_INST_LATENCY failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }
    }

    if (is_12k && (core > 0)) {
        res = kbp_device_set_property(AradAppData[unit]->device_p[0], 
                                        KBP_DEVICE_ADD_BROADCAST, 
                                        1, 
                                        AradAppData[unit]->device_p[core]);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_device_set_property KBP_DEVICE_ADD_BROADCAST failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
        }
        /* Enable kbp broadcast at xpt layer */
        if (soc_property_get(unit, spn_KBP_MESSAGE_BROADCAST_ENABLE, 0)) {
            res = kbp_device_set_property(AradAppData[unit]->device_p[0],
                                            KBP_DEVICE_BROADCAST_AT_XPT, 1);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_device_set_property KBP_DEVICE_ADD_BROADCAST failed: %s\n"),
                          FUNCTION_NAME(), kbp_get_status_string(res)));
            }
        }
    }

#ifdef CRASH_RECOVERY_SUPPORT
    if (ARAD_KBP_IS_CR_MODE(unit) == 1) {
        /* Get allocation HA */
        if(core == 0) {
            nv_mem_ptr = ha_mem_alloc(unit, HA_KBP_Mem_Pool, HA_KBP_SUB_ID_0, KBP_VERSION_1_0, KBP_STRUCT_SIG, &nv_mem_size);
            SOC_SAND_CHECK_NULL_PTR(nv_mem_ptr,20,exit);

            if (SOC_WARM_BOOT(unit)){
                res = kbp_device_set_property(AradAppData[unit]->device_p[core], KBP_DEVICE_PROP_CRASH_RECOVERY, 0, nv_mem_ptr, ARAD_KBP_NV_MEMORY_SIZE);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_device_set_property KBP_DEVICE_PROP_CRASH_RECOVERY 0 failed: %s\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
                }
                res = kbp_device_restore_state(AradAppData[unit]->device_p[core], NULL, NULL, NULL);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_device_restore_state failed: %s\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
                }
            } else {
                res = kbp_device_set_property(AradAppData[unit]->device_p[core], KBP_DEVICE_PROP_CRASH_RECOVERY, 1, nv_mem_ptr, ARAD_KBP_NV_MEMORY_SIZE);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_device_set_property KBP_DEVICE_PROP_CRASH_RECOVERY 1 failed: %s\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
                }
            }
        }
    } else
#endif
    {
        if (SOC_WARM_BOOT(unit) && (core == 0)) {
           res = kbp_device_restore_state(AradAppData[unit]->device_p[core], warmboot_data->kbp_file_read , warmboot_data->kbp_file_write, warmboot_data->kbp_file_fp);
           if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
               LOG_ERROR(BSL_LS_SOC_TCAM,
                         (BSL_META_U(unit,
                                     "Error in %s(): kbp_device_restore_state failed: %s!\n"), 
                                     FUNCTION_NAME(),
                          kbp_get_status_string(res)));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
           }
       }
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_nlm_dev_mgr()", 0, 0);
}

uint32 
    arad_kbp_init_nlm_app_set(
       SOC_SAND_IN int unit,
       SOC_SAND_IN uint32 core
    )
{
    uint32
        res, is_12k;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        if (core == 1) {
            /*
             * On OP ignore the device init for core 1
             */
            return 0;
        }
        is_12k = 0;
    }
    else {
        is_12k = 1;
    }
    /* Currently Device Manager is flushing out each request immediately. */
    AradAppData[unit]->request_queue_len = 1;
    AradAppData[unit]->result_queue_len  = 1;

    /* Create the default allocator */
    res = default_allocator_create(&AradAppData[unit]->dalloc_p[core]);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): default_allocator_create failed: %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
    }

    AradAppData[unit]->alloc_p[core] = NlmCmAllocator__ctor(&AradAppData[unit]->alloc_bdy[core]);
    if(!AradAppData[unit]->alloc_p[core]){
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): Allocator creation Failed!\n"), FUNCTION_NAME()));
        exit(0);
    }

    /* Create transport interface */
   LOG_VERBOSE(BSL_LS_SOC_TCAM,
               (BSL_META_U(unit,
                           "%s(): Create transport interface.\n"), FUNCTION_NAME()));

   if (is_12k) {
    AradAppData[unit]->alg_kbp_xpt_p[core] = arad_kbp_xpt_init(
        unit,
        core,
        AradAppData[unit]->alloc_p[core],            /* General purpose memory allocator */
        AradAppData[unit]->request_queue_len,  /* Max request count */
        AradAppData[unit]->result_queue_len,   /* Max result count*/
        AradAppData[unit]->channel_id);
    } else {
        kbp_status status;
        status = kbp_pcie_init(KBP_DEVICE_OP, KBP_DEVICE_DEFAULT, unit,
                               AradAppData[unit]->dalloc_p[core],
                               NULL, 
                               NULL,
                               &AradAppData[unit]->alg_kbp_xpt_p[core]);
        if (status != KBP_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "%s: Error in PCIE device driver init: %s\n"),
                                  FUNCTION_NAME(),
                                  kbp_get_status_string(status)));
        }
        if (1) {
            /* All update and control path compare operations will go through
             * PCIE. We override the PCIE here to send compares only through
             * ROP path. This is optional
             */
            void *handle;

            handle = arad_kbp_op_xpt_init(unit, core, AradAppData[unit]->dalloc_p[core]);
            status = kbp_pcie_set_property(AradAppData[unit]->alg_kbp_xpt_p[core], KBP_PCIE_REPLACE_SEARCH,
                                          handle, arad_op_search);
            if (status != KBP_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "%s: Error in overriding PCIE compare function: %s\n"),
                          FUNCTION_NAME(),
                          kbp_get_status_string(status)));
            }
        }
#if 0
        status = kbp_pcie_set_property(AradAppData[unit]->alg_kbp_xpt_p[core], KBP_PCIE_VERBOSE_LEVEL, 2, stdout);
        if (status != KBP_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
            (BSL_META_U(unit, "%s: PCIE set property failed: %s\n"),
            FUNCTION_NAME(),
            kbp_get_status_string(status)));
        }
#endif

    }
    if (AradAppData[unit]->alg_kbp_xpt_p[core] == NULL) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): arad_kbp_xpt_init() failed!\n"), FUNCTION_NAME()));
        exit(0);
    }

    /* Initialize Device Manager now */
    res = arad_kbp_init_nlm_dev_mgr(unit, core);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_nlm_app_set()", 
                                  AradAppData[unit]->request_queue_len, 
                                  AradAppData[unit]->result_queue_len);
}

STATIC 
void
    arad_kbp_deinit_nlm_app_set(
       SOC_SAND_IN int unit,
       SOC_SAND_IN uint32 second_kbp_supported
    )
{
    uint32 core = second_kbp_supported ? 2 : 1;
    int res, is_12k;
    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        is_12k = 0;
    } else {
        is_12k = 1;
    }

   /* destroy kbp_dm , was created in arad_kbp_deinit_nlm_dev_mgr() */
    while(core--) {
        /* The init proccess has ignored NLM app init on core 1 */
        if ((!is_12k) && (core == 1)) {
            continue;
        }
        res = kbp_device_destroy(AradAppData[unit]->device_p[core]);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): kbp_device_destroy failed: %s!\n"), 
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res)));
        }
        if (is_12k){
            arad_kbp_xpt_destroy(AradAppData[unit]->alg_kbp_xpt_p[core]);
        }
    }
}

STATIC
    int arad_kbp_device_lock_config(
        SOC_SAND_IN  int unit
    )
{
    uint32 res = SOC_SAND_OK;
    uint32 core = 0;

	res = kbp_device_set_property(AradAppData[unit]->device_p[core], KBP_DEVICE_PRE_CLEAR_ABS, 1);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_device_set_property KBP_DEVICE_PRE_CLEAR_ABS failed: %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));
    }
    res = kbp_device_lock(AradAppData[unit]->device_p[core]);
    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp_device_lock failed: %s!\n"), 
                             FUNCTION_NAME(),
                  kbp_get_status_string(res)));
    }
    return res;
}

STATIC
  uint32 
    arad_kbp_key_init(
        SOC_SAND_IN int                      unit,
        SOC_SAND_IN ARAD_KBP_FRWRD_IP_DB_TYPE   gtm_db_type
    )
{
    uint32
        srch_ndx,
        table_idx,
        key_ndx,
        table_key_offset;

    uint32_t res;
    uint32 core=0;

    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, gtm_db_type, &ltr_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (!SOC_WARM_BOOT(unit)) {
        /*Master key Creation*/
        res = kbp_key_init(AradAppData[unit]->device_p[core], &ltr_config_info.master_key);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): kbp_key_init failed: %s!\n"), 
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 90, exit);
        }
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, gtm_db_type, &ltr_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        for(srch_ndx = 0; srch_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; srch_ndx++)
        {
            if (SHR_BITGET(&ltr_config_info.parallel_srches_bmp, srch_ndx)) {
                table_idx = ltr_config_info.tbl_id[srch_ndx];

                for(key_ndx = 0;
                     key_ndx < ltr_config_info.ltr_search[srch_ndx].nof_key_segments;
                     key_ndx++)
                {
                    /*Create a unique field name*/
                    if(ltr_config_info.ltr_search[srch_ndx].key_segment[key_ndx].nof_bytes){
                        if(!(AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.is_key_adde_to_db)){
                            int type;
                            if(SOC_DPP_CONFIG(unit)->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
                                type = sal_strcmp("VRF", ltr_config_info.ltr_search[srch_ndx].key_segment[key_ndx].name) ? ltr_config_info.ltr_search[srch_ndx].key_segment[key_ndx].type : KBP_KEY_FIELD_EM;
                            } else {
                                type = ltr_config_info.ltr_search[srch_ndx].key_segment[key_ndx].type == KBP_KEY_FIELD_HOLE ? KBP_KEY_FIELD_HOLE : KBP_KEY_FIELD_PREFIX;
                            }
                            res = kbp_key_add_field(
                                    AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.key,
                                    ltr_config_info.ltr_search[srch_ndx].key_segment[key_ndx].name,
                                    (ltr_config_info.ltr_search[srch_ndx].key_segment[key_ndx].nof_bytes * 8),
                                    type
                                  );

                            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                                if(res != KBP_DUPLICATE_KEY_FIELD) {
                                   LOG_ERROR(BSL_LS_SOC_TCAM,
                                             (BSL_META_U(unit,
                                                         "Error in %s(): DB Key : kbp_key_add_field with failed: %s, ltr = %d srch_ndx=%d!\n"), 
                                                         FUNCTION_NAME(),
                                              kbp_get_status_string(res), gtm_db_type, srch_ndx));

                                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
                                }
                            }
                        }
                    }
                }

                if(!(AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.is_key_adde_to_db)){
                    res = kbp_db_set_key(
                            AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.db_p,
                            AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.key
                          );
                    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                       LOG_ERROR(BSL_LS_SOC_TCAM,
                                 (BSL_META_U(unit,
                                             "Error in %s(): DB Key : kbp_db_set_key with failed: %s, srch_ndx = %d table_idx = %d gtm_db_type = %d!\n"), 
                                             FUNCTION_NAME(),
                                  kbp_get_status_string(res), srch_ndx, table_idx, gtm_db_type));

                        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
                    }

                    AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.is_key_adde_to_db = 1;
                    AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.dummy_segment.nof_bytes = ltr_config_info.ltr_search[srch_ndx].key_segment[0].nof_bytes;
                    sal_memcpy(
                        AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.dummy_segment.name, 
                        ltr_config_info.ltr_search[srch_ndx].key_segment[0].name, 
                       sizeof(ltr_config_info.ltr_search[srch_ndx].key_segment[0].name));
                }
            }
        }

        table_key_offset = 0;
        for(key_ndx = 0;
             key_ndx < ltr_config_info.master_key_fields.nof_key_segments;
             key_ndx++)
        {
            /*Create a unique field name*/
            if(ltr_config_info.master_key_fields.key_segment[key_ndx].nof_bytes){
                int type;
                if(SOC_DPP_CONFIG(unit)->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
                    type = sal_strcmp("VRF", ltr_config_info.master_key_fields.key_segment[key_ndx].name) ? ltr_config_info.master_key_fields.key_segment[key_ndx].type : KBP_KEY_FIELD_EM;
                } else {
                    type = ltr_config_info.master_key_fields.key_segment[key_ndx].type == KBP_KEY_FIELD_HOLE ? KBP_KEY_FIELD_HOLE : KBP_KEY_FIELD_PREFIX;
                }
                res = kbp_key_add_field(
                        ltr_config_info.master_key, 
                        ltr_config_info.master_key_fields.key_segment[key_ndx].name,
                        (ltr_config_info.master_key_fields.key_segment[key_ndx].nof_bytes*8), 
                        type
                      );

                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    if(res != KBP_DUPLICATE_KEY_FIELD) {
                       LOG_ERROR(BSL_LS_SOC_TCAM,
                                 (BSL_META_U(unit,
                                             "Error in %s(): DB Master Key Resp Code = %d ,kbp_key_add_field failed: %s!\n"), 
                                             FUNCTION_NAME(),
                                  res,
                                  kbp_get_status_string(res)));

                        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
                    }
                }

                table_key_offset += ltr_config_info.master_key_fields.key_segment[key_ndx].nof_bytes;
            }
        }

        if ((table_key_offset % 10) != 0) 
        {
            table_key_offset = (10 - (table_key_offset % 10));
            res = kbp_key_add_field(
                    ltr_config_info.master_key, 
                    "DUMMY",
                    table_key_offset*8, 
                    KBP_KEY_FIELD_PREFIX
                  );

            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                if(res != KBP_DUPLICATE_KEY_FIELD) {
                   LOG_ERROR(BSL_LS_SOC_TCAM,
                             (BSL_META_U(unit,
                                         "Error in %s(): DB Master Key Resp Code = %d ,kbp_key_add_field failed: %s!\n"), 
                                         FUNCTION_NAME(),
                              res,
                              kbp_get_status_string(res)));

                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 30, exit);
                }
            }
        }

        for(srch_ndx = 0; srch_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; srch_ndx++)
        {
            if (SHR_BITGET(&ltr_config_info.parallel_srches_bmp, srch_ndx)) {
                table_idx = ltr_config_info.tbl_id[srch_ndx];
                if ((table_idx >= ARAD_KBP_FRWRD_TBL_ID_DUMMY_0) && (table_idx <= ARAD_KBP_FRWRD_TBL_ID_DUMMY_3)) {
                    /* Add the dummy fields of the others */
                    res = kbp_key_add_field(
                            ltr_config_info.master_key, 
                            AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.dummy_segment.name,
                            AradAppData[unit]->g_gtmInfo[table_idx].tblInfo.dummy_segment.nof_bytes*8, 
                            KBP_KEY_FIELD_PREFIX
                          );

                    if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                        if(res != KBP_DUPLICATE_KEY_FIELD) {
                           LOG_ERROR(BSL_LS_SOC_TCAM,
                                     (BSL_META_U(unit,
                                                 "Error in %s(): DB Master Key Resp Code = %d ,kbp_key_add_field failed: %s!\n"), 
                                                 FUNCTION_NAME(),
                                      res,
                                      kbp_get_status_string(res)));

                            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 33, exit);
                        }
                    }
                }
            }
        }
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_key_init()", gtm_db_type, 0x0);
}

STATIC
  uint32 
    arad_kbp_search_init(
        SOC_SAND_IN int                      unit,
        SOC_SAND_IN ARAD_KBP_FRWRD_IP_DB_TYPE   gtm_db_type
    )
{
    uint32
        srch_ndx, res;
    uint32 core=0;

    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    ARAD_KBP_LTR_HANDLES
        ltr_handles_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, gtm_db_type, &ltr_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (!SOC_WARM_BOOT(unit)) {
        /* create a new instruction */
        LOG_VERBOSE(BSL_LS_SOC_TCAM,
                    (BSL_META_U(unit,
                                "%s(): DB Type [%d] : create a new instruction, LTR ID %d.\n"),
                                FUNCTION_NAME(),
                     gtm_db_type,
                     ltr_config_info.ltr_id));

        res = kbp_instruction_init(
                    AradAppData[unit]->device_p[core], 
                    ltr_config_info.ltr_id,
                    ltr_config_info.ltr_id, 
                    &ltr_config_info.inst_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_instruction_init failed with : %s, LTR ID = %d\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res), ltr_config_info.ltr_id));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }

		/* save instruction handle */
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.set(unit, gtm_db_type, &ltr_config_info); 
    	SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        ltr_handles_info.inst_p = ltr_config_info.inst_p;
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.ltr_info.set(unit,
                                        gtm_db_type,
                                        &ltr_handles_info); 

        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

        res = kbp_instruction_set_key(ltr_config_info.inst_p, ltr_config_info.master_key);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): kbp_instruction_set_key failed with : %s, db_type = %d\n"), 
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res), gtm_db_type));
        }

        for(srch_ndx = 0; srch_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; srch_ndx++){
            if (SHR_BITGET(&ltr_config_info.parallel_srches_bmp, srch_ndx)) {
                res = kbp_instruction_add_db(ltr_config_info.inst_p,
                                                AradAppData[unit]->g_gtmInfo[ltr_config_info.tbl_id[srch_ndx]].tblInfo.db_p,
                                                srch_ndx);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                   LOG_ERROR(BSL_LS_SOC_TCAM,
                             (BSL_META_U(unit,
                                         "Error in %s(): kbp_instruction_add_db failed with : %s, db_type = %d srch_ndx = %d!\n"), 
                                         FUNCTION_NAME(),
                              kbp_get_status_string(res), gtm_db_type, srch_ndx));

                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
                }
            }
        }

        res = kbp_instruction_install(ltr_config_info.inst_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
           LOG_ERROR(BSL_LS_SOC_TCAM,
                     (BSL_META_U(unit,
                                 "Error in %s(): kbp_instruction_install failed with : %s, db_type = %d!\n"), 
                                 FUNCTION_NAME(),
                      kbp_get_status_string(res), gtm_db_type));

            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
        }

    }
    else { 
        /* refresh the instruction handle */
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.ltr_info.get(unit,
                                       gtm_db_type,
                                       &ltr_handles_info); 

        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

        res = kbp_instruction_refresh_handle(AradAppData[unit]->device_p[core], ltr_handles_info.inst_p, &ltr_handles_info.inst_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_instruction_refresh_handle failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }

        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.ltr_info.set(unit,
                                        gtm_db_type,
                                        &ltr_handles_info); 

        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_search_init()", gtm_db_type, 0x0);
}

/* index change callback function */
void alg_kbp_callback(void *handle,
                      struct kbp_db *db,
                      struct kbp_entry *entry,
                      int32_t old_index,
                      int32_t new_index){
    (void)handle;
    (void)db;
    (void)entry;
    (void)old_index;
    (void)new_index;

    /*LOG_INFO(BSL_LS_SOC_TCAM,
               (BSL_META("\n Index change callback called: oldIndex  = %x,New Index = %x\n"),old_index,new_index));*/
    return;
}

/* Get table size in bytes */
uint32
  arad_kbp_table_size_get(
    SOC_SAND_IN int                   unit, 
    SOC_SAND_IN ARAD_KBP_FRWRD_IP_TBL_ID table_id,
    SOC_SAND_OUT uint32                  *table_size_in_bytes,
    SOC_SAND_OUT uint32                  *table_payload_in_bytes
  )
{
    uint32
        res,
        tbl_width_in_bits = 0,
        tbl_payload_in_bits = 0;

    uint8 is_valid;

    ARAD_KBP_DB_HANDLES db_handles_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Check if table was already created */
    if(table_id >= ARAD_KBP_MAX_NUM_OF_TABLES) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }
    if(table_id >= ARAD_KBP_ACL_TABLE_ID_OFFSET) {
        /* 
         * ACL case 
         * Retrieve the table size by: 
         * 1. Get the used FLP program bitmap and the lookup-id 
         * 2. Get the KBP table id, and its size
         */
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.db_info.get(unit, table_id,&db_handles_info); 
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
        is_valid = db_handles_info.is_valid;
        if (!is_valid) {
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 60, exit);
        }
 	    tbl_width_in_bits = db_handles_info.table_width;
 	 	tbl_payload_in_bits = db_handles_info.table_asso_width;
       
    }
    else {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_width.get(unit, table_id, &tbl_width_in_bits); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_asso_width.get(unit, table_id, &tbl_payload_in_bits); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
    }
    *table_size_in_bytes = SOC_SAND_DIV_ROUND_UP(tbl_width_in_bits, SOC_SAND_NOF_BITS_IN_CHAR); /* Key size in bytes */
    *table_payload_in_bytes = SOC_SAND_DIV_ROUND_UP(tbl_payload_in_bits, SOC_SAND_NOF_BITS_IN_CHAR); /* Payload size in bytes */

    ARAD_DO_NOTHING_AND_EXIT;
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_table_size_get()", table_id, 0x0);
}

uint32
  arad_kbp_static_table_size_get(
    SOC_SAND_IN int                   unit, 
    SOC_SAND_IN ARAD_KBP_FRWRD_IP_TBL_ID table_id,
    SOC_SAND_OUT uint32                  *table_size_in_bytes,
    SOC_SAND_OUT uint32                  *table_payload_in_bytes
  )
{
    uint32
        tbl_width_in_bits = 0,
        tbl_payload_in_bits = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Check if table was already created */
    if(table_id >= ARAD_KBP_FRWRD_IP_NOF_TABLES) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

    tbl_width_in_bits = Arad_kbp_table_config_info_static[table_id].tbl_width;
    tbl_payload_in_bits = Arad_kbp_table_config_info_static[table_id].tbl_asso_width;
    
    *table_size_in_bytes = SOC_SAND_DIV_ROUND_UP(tbl_width_in_bits, SOC_SAND_NOF_BITS_IN_CHAR); /* Key size in bytes */
    *table_payload_in_bytes = SOC_SAND_DIV_ROUND_UP(tbl_payload_in_bits, SOC_SAND_NOF_BITS_IN_CHAR); /* Payload size in bytes */

    ARAD_DO_NOTHING_AND_EXIT;
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_static_table_size_get()", table_id, 0x0);
}

/* Get LTR-ID per table id */
uint32
  arad_kbp_table_ltr_id_get(
    SOC_SAND_IN int                   unit, 
    SOC_SAND_IN ARAD_KBP_FRWRD_IP_TBL_ID table_id,
    SOC_SAND_OUT ARAD_KBP_FRWRD_IP_LTR   *ltr_id
  )
{
    ARAD_KBP_FRWRD_IP_DB_TYPE 
        gtm_db_type_ndx;
    uint32
        srch_ndx, res;
    uint8
        found;

    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);   
    
    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info); 

    found = FALSE;
    for(gtm_db_type_ndx = 0; (gtm_db_type_ndx < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS) && (!found); gtm_db_type_ndx++) 
    {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, gtm_db_type_ndx, &ltr_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        for(srch_ndx = 0; (srch_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES) && (!found); srch_ndx++) 
        {
            if (SHR_BITGET(&(ltr_config_info.parallel_srches_bmp), srch_ndx)) {
                if (ltr_config_info.tbl_id[srch_ndx] == table_id) {
                    found = TRUE;
                    *ltr_id = ltr_config_info.ltr_id;
                }
            }
        }
    }

    /* Assumption always found */
    if (!found) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

    ARAD_DO_NOTHING_AND_EXIT;
    
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_table_ltr_id_get()", table_id, 0x0);
}

/* This function performs Generic Table Manager related Inits */
STATIC
  uint32 
    arad_kbp_table_init(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  uint32 table_size,
        SOC_SAND_IN  uint32 table_id
    )
{
    uint32 
        res = SOC_SAND_OK, is_12k;

    ARAD_KBP_TABLE_CONFIG table_config_info = {0};
    uint32
        db_type = KBP_DB_LPM;
    ARAD_KBP_DB_HANDLES
        db_handles_info = {0};

    uint32 core = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_TABLE_CONFIG_clear(&table_config_info);

    if(NULL != AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p) {
        ARAD_DO_NOTHING_AND_EXIT;
    }

    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311) {
        is_12k = 0;
    } else {
        is_12k = 1;
    }

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.get(unit, table_id, &table_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

    /* Create tables according to GTM database type */
    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id        = table_config_info.tbl_id;
    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_width     = table_config_info.tbl_width;
    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_size      = table_size;
    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_assoWidth = table_config_info.tbl_asso_width;
    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.groupId_start = table_config_info.group_id_start;
    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.groupId_end   = table_config_info.group_id_end;

    if (!SOC_WARM_BOOT(unit)) {
        /* Call the CreateTable API to create each of the tables */
       LOG_VERBOSE(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "%s(): Table [%d] : create DB with width=%d, table-size=%d.\n"),
                               FUNCTION_NAME(),
                    table_config_info.tbl_id,
                    table_config_info.tbl_width,
                    table_config_info.tbl_size));

        if (table_config_info.clone_of_tbl_id == ARAD_KBP_FRWRD_IP_NOF_TABLES) {
            /* Not a clone. Create the DB */
            if (table_config_info.tbl_id >= ARAD_KBP_FRWRD_IP_NOF_TABLES || ((table_config_info.tbl_id >= ARAD_KBP_FRWRD_TBL_ID_DUMMY_0) && (table_config_info.tbl_id <= ARAD_KBP_FRWRD_TBL_ID_DUMMY_5))) {
                db_type = KBP_DB_ACL; 
            }

			if (table_config_info.tbl_id == ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED) {
                db_type = KBP_DB_ACL;
			}

			if (table_config_info.tbl_id == ARAD_KBP_FRWRD_TBL_ID_IPV4_MC &&
			soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_ipv4_mc_flexible_fwd_table", 0)) {
                db_type = KBP_DB_ACL;
			}

			res = kbp_db_init(
                    AradAppData[unit]->device_p[core], 
                    db_type, 
                    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id,
                    AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_size, 
                    &AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p
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
            if ((AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_assoWidth != NLM_TBL_ADLEN_ZERO)) {
                LOG_VERBOSE(BSL_LS_SOC_TCAM,
                            (BSL_META_U(unit,
                                        "%s(): Table [%d] : create AD DB with asso-data=%d.\n"),
                                        FUNCTION_NAME(),
                             table_id,
                             ARAD_KBP_AD_WIDTH_TYPE_TO_BITS(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_assoWidth)));

                res = kbp_ad_db_init(
                            AradAppData[unit]->device_p[core], 
                            AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id,
                            AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_size, 
                            ARAD_KBP_AD_WIDTH_TYPE_TO_BITS(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_assoWidth),
                            &AradAppData[unit]->g_gtmInfo[table_id].tblInfo.ad_db_p);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_ad_db_init failed: %s, TBL ID %d\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res),
                              AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
                }

                res = kbp_db_set_ad(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p,
                                      AradAppData[unit]->g_gtmInfo[table_id].tblInfo.ad_db_p);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_db_set_ad failed: %s\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
                }

				/* save DB handles */
                db_handles_info.db_p    = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p;
                db_handles_info.ad_db_p = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.ad_db_p;
                db_handles_info.is_valid = TRUE;
                db_handles_info.table_size = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_size;
                db_handles_info.table_id = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id;
                db_handles_info.table_width = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_width;
                db_handles_info.table_asso_width = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_assoWidth;
                res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.db_info.set(unit,
                                                table_id,
                                                &db_handles_info); 

                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
            }

           LOG_VERBOSE(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "%s(): Table [%d] : Create new key for each database.\n"),
                                   FUNCTION_NAME(),
                        table_config_info.tbl_id));
        }
        else {
            res = kbp_db_clone(AradAppData[unit]->g_gtmInfo[table_config_info.clone_of_tbl_id].tblInfo.db_p,
                         AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id,
                         &(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p));
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_db_clone failed: %s, TBL ID %d\n"),
                          FUNCTION_NAME(),
                          kbp_get_status_string(res),
                          AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
            }

            /* save Db handle */
            db_handles_info.db_p    = AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p;
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.db_info.set(unit,
                                            table_id,
                                            &db_handles_info); 

            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

        }
        res = kbp_key_init(AradAppData[unit]->device_p[core], 
                             &AradAppData[unit]->g_gtmInfo[table_id].tblInfo.key);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_key_init failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }

       LOG_VERBOSE(BSL_LS_SOC_TCAM,
                   (BSL_META_U(unit,
                               "%s(): Table [%d] : Setting ACL database to massively parallel mode.\n"),
                               FUNCTION_NAME(), table_config_info.tbl_id));

        if (is_12k || (db_type == KBP_DB_ACL)) {
        res = kbp_db_set_property(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p, 
                                    KBP_PROP_ALGORITHMIC, 0);
        }
        
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_set_property KBP_PROP_ALGORITHMIC failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }

        /*res = kbp_db_set_property(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p, 
                                    KBP_PROP_DEFER_DELETES, 1);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_set_property KBP_PROP_DEFER_DELETES failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }*/

		if (is_12k && table_id == ARAD_KBP_FRWRD_TBL_ID_IPV4_DC) {
			res = kbp_db_set_property(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p,
										KBP_PROP_SCALE_UP_CAPACITY, 1);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_db_set_property KBP_PROP_SCALE_UP_CAPACITY failed: %s\n"),
        				  FUNCTION_NAME(), kbp_get_status_string(res)));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
            }
		}

        if (is_12k && table_config_info.min_priority >= 0) {
            res = kbp_db_set_property(AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p, 
                                        KBP_PROP_MIN_PRIORITY, table_config_info.min_priority);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_db_set_property KBP_PROP_MIN_PRIORITY failed: %s\n"),
        				  FUNCTION_NAME(), kbp_get_status_string(res)));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
            }
        }

        /*
         *  Set UPD level. Enables all HW DBA resources.
         *  property PROG_UPD_LEVEL (109) can be se if the DB won't be
         *  allocated dynamically.
         *  It will increase the performance for static DBs
         */ 
         
        ARAD_DO_NOTHING_AND_EXIT;
    }
    else {
        /* refresh the DB handles */
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.db_info.get(unit,
                                       table_id, 
                                       &db_handles_info);

        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

        res = kbp_db_refresh_handle(AradAppData[unit]->device_p[core], db_handles_info.db_p, &db_handles_info.db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_refresh_handle failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }
        res = kbp_ad_db_refresh_handle(AradAppData[unit]->device_p[core], db_handles_info.ad_db_p, &db_handles_info.ad_db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_ad_db_refresh_handle failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }

        AradAppData[unit]->g_gtmInfo[table_id].tblInfo.db_p = db_handles_info.db_p;
        AradAppData[unit]->g_gtmInfo[table_id].tblInfo.ad_db_p = db_handles_info.ad_db_p;
        AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_size = db_handles_info.table_size;
        AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_id = db_handles_info.table_id; 
        AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_width = db_handles_info.table_width;
        AradAppData[unit]->g_gtmInfo[table_id].tblInfo.tbl_assoWidth = db_handles_info.table_asso_width;

        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.db_info.set(unit,
                                        table_id,
                                        &db_handles_info);

        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);        
    }
    

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_table_init()", table_id, table_size);
}


uint32 arad_kbp_frwrd_opcode_set(
    int unit,
    uint8 opcode,
    uint32 tx_data_size, 
    uint32 tx_data_type, 
    uint32 rx_data_size, 
    uint32 rx_data_type)
{
    uint32
        res;
    uint32
        table_entry[1];
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Configure in the EGW OPCODES */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IHB_OPCODE_MAP_TXm(unit, MEM_BLOCK_ANY, opcode, table_entry));
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, SIZEf, tx_data_size);
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_TXm, table_entry, TYPEf, tx_data_type);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHB_OPCODE_MAP_TXm(unit, MEM_BLOCK_ANY, opcode, table_entry));

    SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_OPCODE_MAP_RXm(unit, MEM_BLOCK_ANY, opcode, table_entry));
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, SIZEf, rx_data_size);
    soc_mem_field32_set(unit, IHB_OPCODE_MAP_RXm, table_entry, TYPEf, rx_data_type);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_OPCODE_MAP_RXm(unit, MEM_BLOCK_ANY, opcode, table_entry));


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_frwrd_opcode_set()", opcode, tx_data_size);
}

uint32 arad_kbp_frwrd_ip_rpf_lut_set(
    int unit,
    uint8 opcode,
    uint32 ltr,
    uint32 gtm_table_id,
    uint32 second_kbp_supported)
{
    uint32
        res = SOC_SAND_OK;
    arad_kbp_lut_data_t lut_data = {0};
    uint32 core = 0;
    arad_kbp_frwd_ltr_db_t  Arad_kbp_gtm_ltr_info = {0};
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    arad_kbp_frwd_ltr_db_clear(&Arad_kbp_gtm_ltr_info);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.get(unit, gtm_table_id, &lut_data); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.get(unit, gtm_table_id, &Arad_kbp_gtm_ltr_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* Setting LTR to opcode mapping DB
     * res_format - rounding up AD size.
     */
    
    sal_memcpy(
	&(arad_kbp_frwd_ltr_db[ltr]),
	&(Arad_kbp_gtm_ltr_info), sizeof(arad_kbp_frwd_ltr_db_t));
	
    
    if (!SOC_WARM_BOOT(unit)) {
        SOC_DPP_CORES_ITER(_SHR_CORE_ALL, core) {
            if ((core > 0) && (!second_kbp_supported)) {
                goto exit;
            }
            res = arad_kbp_lut_write(unit, core, opcode, &lut_data, NULL);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
			res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.set(unit, gtm_table_id, &lut_data); 
    		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_frwrd_ip_rpf_lut_set()", opcode, ltr);
}

uint32 arad_kbp_frwrd_ip_opcode_set(
    int unit,
    uint8 opcode,
    uint32 gtm_table_id)
{
    uint32
        res;

    ARAD_KBP_GTM_OPCODE_CONFIG_INFO Arad_kbp_gtm_opcode_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_opcode_config_info.get(unit, gtm_table_id, &Arad_kbp_gtm_opcode_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (!SOC_WARM_BOOT(unit)) {
        /* Setting Arad opcode */
        res = arad_kbp_frwrd_opcode_set(
            unit, 
            opcode,
            Arad_kbp_gtm_opcode_config_info.tx_data_size,
            Arad_kbp_gtm_opcode_config_info.tx_data_type,
            Arad_kbp_gtm_opcode_config_info.rx_data_size,
            Arad_kbp_gtm_opcode_config_info.rx_data_type
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_frwrd_ip4_rpf_opcode_set()", opcode, gtm_table_id);
}

uint32 arad_kbp_init_db_set(
    int unit,
    uint32 second_kbp_supported,
    ARAD_INIT_ELK *elk)
{
    uint32
        tbl_idx,
        db_type,
        res,
        tbl_size;

    nlm_u8 is_valid, tbl_id;

    ARAD_KBP_DB_HANDLES db_handles_info;
    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

    /* Configure DBs */
    for (tbl_idx = 0; tbl_idx < ARAD_KBP_MAX_NUM_OF_TABLES; tbl_idx++) 
    {
        if (SOC_WARM_BOOT(unit)) {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.db_info.get(unit,
                                           tbl_idx, 
                                           &db_handles_info);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
            is_valid = db_handles_info.is_valid;
            /*tbl_id = db_handles_info.table_id;*/
            tbl_id = tbl_idx;
            tbl_size = db_handles_info.table_size;
        } 
        else {
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.get(unit, tbl_idx, &is_valid); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_id.get(unit, tbl_idx, &tbl_id); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.tbl_size.get(unit, tbl_idx, &tbl_size); 
            SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);            
        }

        if (is_valid) 
        {
            res = arad_kbp_table_init(
                    unit, 
                    tbl_size, 
                    tbl_id
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
        }
    }

    /* Configure Tables keys in KBP */
    for(db_type = 0; db_type < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; db_type++) 
    {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 500+db_type, exit);
        if (ltr_config_info.valid) 
        {
            /* Configure Search */
            res = arad_kbp_key_init(
                    unit, 
                    db_type
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
        }
    }

    /* Configure Tables and LTRs in KBP */
    for(db_type = 0; db_type < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; db_type++) 
    {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 600+db_type, exit);
        if (ltr_config_info.valid) 
        {
           LOG_VERBOSE(BSL_LS_SOC_TCAM,
                       (BSL_META_U(unit,
                                   "%s(): Configure DB Type %s, LTR=%d(0x%x), Opcode = %d\n"), 
                                   FUNCTION_NAME(),
                        ARAD_KBP_FRWRD_IP_DB_TYPE_to_string(db_type),
                        ltr_config_info.ltr_id,
                        ltr_config_info.ltr_id,
                        ltr_config_info.opcode));

            /* Configure Search */
            res = arad_kbp_search_init(
                    unit, 
                    db_type
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);

            /* Configure KBP Opcode/LUT table */
            res = arad_kbp_frwrd_ip_rpf_lut_set(
                    unit, 
                    ltr_config_info.opcode,
                    ltr_config_info.ltr_id,
                    db_type, 
                    second_kbp_supported
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

            /* Configure Arad Opcode/LUT table */
            res = arad_kbp_frwrd_ip_opcode_set(
                    unit, 
                    ltr_config_info.opcode,
                    db_type
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
        }
    }
    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311
        && !SOC_WARM_BOOT(unit)) {
        for(db_type = 0; db_type < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; db_type++) {
            uint32 srch_ndx;
            arad_kbp_lut_data_t kbp_gtm_lut_info = {0};
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_lut_info.get(unit, db_type, &kbp_gtm_lut_info);
            SOC_SAND_CHECK_FUNC_RESULT(res, 600+db_type, exit);
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info);
            SOC_SAND_CHECK_FUNC_RESULT(res, 600+db_type, exit);
            if (ltr_config_info.valid) {
               LOG_VERBOSE(BSL_LS_SOC_TCAM,
                           (BSL_META_U(unit,
                                       "%s(): Configure DB Type %s, LTR=%d(0x%x), Opcode = %d\n"),
                                       FUNCTION_NAME(),
                            ARAD_KBP_FRWRD_IP_DB_TYPE_to_string(db_type),
                            ltr_config_info.ltr_id,
                            ltr_config_info.ltr_id,
                            ltr_config_info.opcode));
                for(srch_ndx = 0; srch_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; srch_ndx++) {
                    if (SHR_BITGET(&ltr_config_info.parallel_srches_bmp, srch_ndx)) {
                        uint32 ad_size = 0;
                        switch(srch_ndx) {
                        case 0:
                            ad_size = kbp_gtm_lut_info.result0_idx_ad_cfg;
                            break;
                        case 1:
                            ad_size = kbp_gtm_lut_info.result1_idx_ad_cfg;
                            break;
                        case 2:
                            ad_size = kbp_gtm_lut_info.result2_idx_ad_cfg;
                            break;
                        case 3:
                            ad_size = kbp_gtm_lut_info.result3_idx_ad_cfg;
                            break;
                        case 4:
                            ad_size = kbp_gtm_lut_info.result4_idx_ad_cfg;
                            break;
                        case 5:
                            ad_size = kbp_gtm_lut_info.result5_idx_ad_cfg;
                            break;
                        default:
                            SOC_SAND_CHECK_FUNC_RESULT(res, 600+db_type, exit);
                        }
                        res = kbp_instruction_set_property(ltr_config_info.inst_p, KBP_INST_PROP_RESULT_DATA_LEN, srch_ndx, 0, ad_size);
                        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                            LOG_ERROR(BSL_LS_SOC_TCAM,
                                      (BSL_META_U(unit, "Error in %s(): kbp_instruction_set_property failed: %s\n"),
                                      FUNCTION_NAME(), kbp_get_status_string(res)));
                            ARAD_KBP_CHECK_FUNC_RESULT(res, 70, exit);
                        }
                    }
                }
            }
        }
    }

    res = arad_kbp_device_lock_config(unit);
    if(res != 0) {
        LOG_ERROR(BSL_LS_SOC_TCAM,
             (BSL_META_U(unit,
                         "Error in %s(): %s!\n"), 
                         FUNCTION_NAME(),
              kbp_get_status_string(res)));
        SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    }
    if (SOC_DPP_CONFIG( unit )->arad->init.elk.tcam_dev_type == ARAD_NIF_ELK_TCAM_DEV_TYPE_BCM52311
        && !SOC_WARM_BOOT(unit)) {
        for(db_type = 0; db_type < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; db_type++) {
            uint32 srch_ndx;
            res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info);
            SOC_SAND_CHECK_FUNC_RESULT(res, 600+db_type, exit);
            if (ltr_config_info.valid) {
               LOG_VERBOSE(BSL_LS_SOC_TCAM,
                           (BSL_META_U(unit,
                                       "%s(): Configure DB Type %s, LTR=%d(0x%x), Opcode = %d\n"),
                                       FUNCTION_NAME(),
                            ARAD_KBP_FRWRD_IP_DB_TYPE_to_string(db_type),
                            ltr_config_info.ltr_id,
                            ltr_config_info.ltr_id,
                            ltr_config_info.opcode));
                for(srch_ndx = 0; srch_ndx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; srch_ndx++) {
                    if (SHR_BITGET(&ltr_config_info.parallel_srches_bmp, srch_ndx)) {
                        if (AradAppData[unit]->g_gtmInfo[ltr_config_info.tbl_id[srch_ndx]].tblInfo.tbl_assoWidth == 64)
                            KBP_TRY(kbp_instruction_set_property(ltr_config_info.inst_p, KBP_INST_PROP_RESULT_DATA_LEN, srch_ndx, 0, 6));
                        else if (AradAppData[unit]->g_gtmInfo[ltr_config_info.tbl_id[srch_ndx]].tblInfo.tbl_assoWidth == 32)
                            KBP_TRY(kbp_instruction_set_property(ltr_config_info.inst_p, KBP_INST_PROP_RESULT_DATA_LEN, srch_ndx, 0, 3));
                    }
                }
            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_db_set()", 0, 0);
}

#ifdef CRASH_RECOVERY_SUPPORT
uint32 arad_kbp_cr_transaction_cmd(
   int unit,
   uint8 is_start)
{
    int core = 0;
    int res = SOC_SAND_OK;
    
    if (AradAppData[unit]->device_p[core] != NULL) {
        if (is_start) {
            res = kbp_device_start_transaction(AradAppData[unit]->device_p[core]);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_device_start_transaction failed: %s\n"),
                          FUNCTION_NAME(), kbp_get_status_string(res)));
            }
        } else { 
            res = kbp_device_end_transaction(AradAppData[unit]->device_p[core]);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_device_end_transaction failed: %s\n"),
                          FUNCTION_NAME(), kbp_get_status_string(res)));
            }
        }
    }
    return res;
}

uint8 arad_kbp_cr_query_restore_status(int unit)
{
    int core = 0;
    int res;
    uint8 result=TRUE;

    enum kbp_restore_status res_status;

    if(AradAppData[unit]->device_p[core] != NULL){
        res = kbp_device_query_restore_status(AradAppData[unit]->device_p[core], &res_status);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_device_query_restore_status failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
        }
    }

    LOG_CLI((BSL_META_U(unit,"KBP status=%d\n"),res_status));
    if (res_status == KBP_RESTORE_CHANGES_ABORTED) {
        result = FALSE;
    }

    return result;
}

uint32 arad_kbp_cr_clear_restore_status(int unit)
{
    int core=0;
    int res = SOC_SAND_OK;

    if(AradAppData[unit]->device_p[core] != NULL){
        res = kbp_device_clear_restore_status(AradAppData[unit]->device_p[core]);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_device_clear_restore_status failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
        }
    }
    return res;
}

uint32 arad_kbp_cr_db_commit(int unit,
                             uint32 tbl_id)
{
    struct kbp_db *db_p = NULL;
    uint32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = arad_kbp_alg_kbp_db_get(unit, tbl_id, &db_p);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (db_p != NULL) {
        res = kbp_db_install(db_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_install failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_db_commit()", 0, 0);
}
#endif

uint32 arad_kbp_db_commit(
   int unit)
{
    struct kbp_db 
        *db_p = NULL;
    uint32
        tbl_idx,        
        res;

    nlm_u8 is_valid = FALSE;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* commit DBs */
    for (tbl_idx = 0; tbl_idx < ARAD_KBP_MAX_NUM_OF_TABLES; tbl_idx++) 
    {
        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.valid.get(unit, tbl_idx, &is_valid); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
        if (is_valid) 
        {
            res = arad_kbp_alg_kbp_db_get(unit, tbl_idx, &db_p);
            SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
			soc_sand_ll_timer_set("ARAD_KBP_TIMERS_ROUTE_ADD_COMMIT", ARAD_KBP_TIMERS_ROUTE_ADD_COMMIT);
#endif 
            res = kbp_db_install(db_p);
#ifdef ARAD_PP_KBP_TIME_MEASUREMENTS
			soc_sand_ll_timer_stop(ARAD_KBP_TIMERS_ROUTE_ADD_COMMIT);
#endif 
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_db_install failed: %s\n"),
                          FUNCTION_NAME(), kbp_get_status_string(res)));
                SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 20, exit);
            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_db_commit()", 0, 0);
}

/* 
 * Arad KBP application init function. 
 * Called from PP Mgmt. 
 */
uint32 arad_kbp_init_app(
    int unit,
    uint32 second_kbp_supported,
    ARAD_INIT_ELK *elk_ptr)
{
    uint32
        res;
    ARAD_INIT_ELK *elk;
    uint32 core;
    uint32 iter_count = (second_kbp_supported ? 2:1);
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (elk_ptr == NULL) {
        elk = &SOC_DPP_CONFIG(unit)->arad->init.elk;
    } else {
        elk = elk_ptr;
    }

    if (elk->enable == 0x0) {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): ELK disabled (ext_tcam_dev_type might be NONE)!!!\n"), FUNCTION_NAME()));    
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
    }

    AradAppData[unit] = NULL;
    ARAD_ALLOC_ANY_SIZE(AradAppData[unit], genericTblMgrAradAppData, 1,"AradAppData[unit][core]");
    sal_memset(AradAppData[unit], 0x0, sizeof(genericTblMgrAradAppData));

    for (core = 0; core < iter_count; core++) {
        /* ROP Write-Read Test */
        if(!SAL_BOOT_PLISIM){
            res = arad_kbp_init_rop_test(unit, core);
            /* KBP Recovery */
            if (res != 0 && elk->kbp_recover_enable) {
                res = arad_kbp_recover_run_recovery_sequence(unit, core, elk->kbp_mdio_id[core], elk->kbp_recover_iter, NULL, 1);
            }
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
        }    

        if (!SOC_WARM_BOOT(unit)) {
           res = arad_kbp_blk_lut_set(unit, core);
           if (res != 0 && elk->kbp_recover_enable) {
               res = arad_kbp_recover_run_recovery_sequence(unit, core, elk->kbp_mdio_id[core], elk->kbp_recover_iter, NULL, 2);
           }
           SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        }
        
        /* NLM Application init */
        res = arad_kbp_init_nlm_app_set(unit, core);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    }


    /* Create KBP DB, config KBP searches, config KBP LUT, config Arad opcode */
    res = arad_kbp_init_db_set(unit, second_kbp_supported, elk);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_init_app()", elk->enable, elk->tcam_dev_type);
}

uint32 
    arad_kbp_deinit_app(
        int unit,
        uint32 second_kbp_supported
    )
{
    int core=0;
    int res;
    uint32 iter_count;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    iter_count = second_kbp_supported?2:1;

    if(NULL == AradAppData[unit])
    {
       LOG_ERROR(BSL_LS_SOC_TCAM,
                 (BSL_META_U(unit,
                             "Error in %s(): kbp device not initialized.\n"), FUNCTION_NAME()));
        ARAD_DO_NOTHING_AND_EXIT;
    }else{
        arad_kbp_deinit_nlm_app_set(unit, second_kbp_supported);
        for (core = 0; core < iter_count; core++) {
            if (AradAppData[unit]->dalloc_p[core] != NULL) {
                res = default_allocator_destroy(AradAppData[unit]->dalloc_p[core]);
                if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): default_allocator_destroy failed: %s\n"),
                              FUNCTION_NAME(), kbp_get_status_string(res)));
                    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
                }
            }
        }

        ARAD_FREE(AradAppData[unit]);
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_deinit_app()", unit, 0x0);
}

void
  arad_kbp_frwd_ltr_db_clear(
    SOC_SAND_OUT arad_kbp_frwd_ltr_db_t *info
  )
{
    uint32 
        res_idx;

    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(arad_kbp_frwd_ltr_db_t));
    for( res_idx = 0; 
         res_idx < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; 
         res_idx++ )
    {
        info->res_format[res_idx] = NLM_ARAD_NO_INDEX_NO_AD;
    }

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_GTM_OPCODE_CONFIG_INFO_clear(
    SOC_SAND_OUT ARAD_KBP_GTM_OPCODE_CONFIG_INFO *info
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(ARAD_KBP_GTM_OPCODE_CONFIG_INFO));

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_GTM_LUT_clear(
    SOC_SAND_OUT 	arad_kbp_lut_data_t	*info
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(arad_kbp_lut_data_t));

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_LTR_CONFIG_clear(
    SOC_SAND_OUT ARAD_KBP_LTR_CONFIG *info
  )
{
    uint32 
        index;

    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(ARAD_KBP_LTR_CONFIG));
    info->valid = FALSE;
    
    for (index = 0; index < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; index++) 
    {
        info->tbl_id[index] = ARAD_KBP_TABLE_INDX_TO_DUMMY_TABLE_ID(index);
        info->search_type[index] = ARAD_KBP_NOF_DB_TYPES;
        ARAD_KBP_LTR_SINGLE_SEARCH_clear(info->ltr_search);
    }

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_LTR_SINGLE_SEARCH_clear(
    SOC_SAND_OUT ARAD_KBP_LTR_SINGLE_SEARCH *info
  )
{
    uint32 
        index;

    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(ARAD_KBP_LTR_SINGLE_SEARCH));

    for (index = 0; index < ARAD_KBP_MAX_NOF_KEY_SEGMENTS; index++) 
    {
        ARAD_KBP_KEY_SEGMENT_clear(&info->key_segment[index]);
    }

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_TABLE_CONFIG_clear(
    SOC_SAND_OUT ARAD_KBP_TABLE_CONFIG *info
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(ARAD_KBP_TABLE_CONFIG));
    info->valid = FALSE;
    info->tbl_width = NLM_TBL_WIDTH_END;
    info->tbl_asso_width = NLM_TBL_ADLEN_END;
    
exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_KEY_SEGMENT_clear(
    SOC_SAND_OUT ARAD_KBP_KEY_SEGMENT *info
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    sal_memset(info, 0x0, sizeof(ARAD_KBP_KEY_SEGMENT));

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

const char*
  NlmAradCompareResponseFormat_to_string(
    SOC_SAND_IN  NlmAradCompareResponseFormat enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case NLM_ARAD_ONLY_INDEX_NO_AD:
    str = "Index-No-AD";
  break;
  case NLM_ARAD_INDEX_AND_32B_AD:
    str = "Index,32bAD";
  break;
  case NLM_ARAD_INDEX_AND_64B_AD:
    str = "Index,64bAD";
  break;
  case NLM_ARAD_INDEX_AND_128B_AD:
    str = "Index,128bAD";
  break;
  case NLM_ARAD_NO_INDEX_NO_AD:
    str = "No-Index-No-AD";
  break;
  default:
    str = " Unknown";
  }
  return str;
}

const char*
  ARAD_KBP_FRWRD_IP_DB_TYPE_to_string(
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_DB_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC:
    str = "IPv4 Unicast";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC_RPF:
    str = "IPv4 Unicast RPF";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IPV4_MC_RPF:
    str = "IPv4 Multicast RPF";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC:
    str = "IPv6 Unicast";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC_RPF:
    str = "IPv6 Unicast RPF";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IPV6_MC_RPF:
    str = "IPv6 Multicast RPF";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_LSR:
    str = "MPLS";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_TRILL_UC:
    str = "TRILL Unicast";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_TRILL_MC:
    str = "TRILL Multicast";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC:
    str = "IPV4 Double Capacity";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED_FOR_IP:
    str = "IP LSR SHARED FOR IP";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED_FOR_IP_WITH_RPF:
    str = "IP LSR SHARED FOR IP WITH RPF";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED_FOR_LSR:
    str = "IP LSR SHARED FOR LSR";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_EXTENDED_IPv6:
    str = "EXTENDED IPv6";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_EXTENDED_P2P:
    str = "EXTENDED P2P";
  break;
  case ARAD_KBP_FRWRD_DB_TYPE_DUMMY_FRWRD:
    str = "DUMMY FRWRD";
  break;
  default:
    str = "ACL";
  }
  return str;
}

const char*
  ARAD_KBP_FRWRD_IP_TBL_ID_to_string(
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_TBL_ID enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0:
    str = "IPv4 UC";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1:
    str = "IPv4 RPF";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_IPV4_MC:
    str = "IPv4 MC";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0:
    str = "IPv6 UC";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1:
    str = "IPv6 RPF";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_IPV6_MC:
    str = "IPv6 MC";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_LSR:
    str = "MPLS";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_TRILL_UC:
    str = "TRILL UC";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_TRILL_MC:
    str = "TRILL MC";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC:
    str = "IPV4 MC DUMMY";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV6_MC:
    str = "IPV6 MC DUMMY";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_FRWRD:
    str = "DUMMY FRWRD";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_0:
    str = "DUMMY 0";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_1:
    str = "DUMMY 1";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_2:
    str = "DUMMY 2";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_3:
    str = "DUMMY 3";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_4:
    str = "DUMMY 4";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_DUMMY_5:
    str = "DUMMY 5";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED:
    str = "LSR IP SHARED";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_IP:
    str = "LSR IP SHARED FOR IP";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED_FOR_LSR:
    str = "LSR IP SHARED FOR LSR";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6:
    str = "EXTENDED IPV6";
  break;
  case ARAD_KBP_FRWRD_TBL_ID_IPV4_DC:
      str = "IPV4 DC";
  break;
  default:
    str = "ACL";
  }
  return str;
}

const char*
  ARAD_KBP_DB_TYPE_to_string(
    SOC_SAND_IN  ARAD_KBP_DB_TYPE enum_val
  )
{
  const char* str = NULL;
  switch(enum_val)
  {
  case ARAD_KBP_DB_TYPE_ACL:
    str = "ACL";
  break;
  case ARAD_KBP_DB_TYPE_FORWARDING:
    str = "Forwarding";
  break;
  default:
    str = "Unknown";
  }
  return str;
}

/* Print Definitions */
void
  arad_kbp_frwd_lut_db_print(
    SOC_SAND_IN  arad_kbp_lut_data_t *info
  )
{
	SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

	LOG_CLI((BSL_META_U(unit,"  \tLUT DB:\n")));
	LOG_CLI((BSL_META_U(unit,
						"  \t\trec_is_valid=%d, rec_size=%d, rec_type=%d\n"
						"  \t\tmode=%d, key_config=%d, lut_key_data=%d\n"
						"  \t\tinstr=%d, key_w_cpd_gt_80=%d, copy_data_cfg=%d\n"),
			 info->rec_is_valid,info->rec_size,info->rec_type,
			 info->mode,info->key_config,info->lut_key_data,
			 info->instr,info->key_w_cpd_gt_80,info->copy_data_cfg));
	LOG_CLI((BSL_META_U(unit,
						"  \t\tresult0_idx_ad_cfg=%d, result0_idx_or_ad=%d\n"
						"  \t\tresult1_idx_ad_cfg=%d, result1_idx_or_ad=%d\n"
						"  \t\tresult2_idx_ad_cfg=%d, result2_idx_or_ad=%d\n"
						"  \t\tresult3_idx_ad_cfg=%d, result3_idx_or_ad=%d\n"
						"  \t\tresult4_idx_ad_cfg=%d, result4_idx_or_ad=%d\n"
						"  \t\tresult5_idx_ad_cfg=%d, result5_idx_or_ad=%d\n"),
			 info->result0_idx_ad_cfg,info->result0_idx_or_ad,
			 info->result1_idx_ad_cfg,info->result1_idx_or_ad,
			 info->result2_idx_ad_cfg,info->result2_idx_or_ad,
			 info->result3_idx_ad_cfg,info->result3_idx_or_ad,
			 info->result4_idx_ad_cfg,info->result4_idx_or_ad,
			 info->result5_idx_ad_cfg,info->result5_idx_or_ad));
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  arad_kbp_frwd_ltr_db_print(
    SOC_SAND_IN  arad_kbp_frwd_ltr_db_t *info,
    SOC_SAND_IN  int advanced
  )
{
    uint32
        index;

    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

	LOG_CLI((BSL_META_U(unit,
						"  Total Result Length=%d\n"), 
			 info->res_total_data_len));
    if(advanced) { /* show each result only when using advanced or debug print */
        for (index = 0; index < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; index++) 
        {
            LOG_CLI((BSL_META_U(unit,
                            "  Result %d: length=%d  format=<%s>\n"), 
                    index, 
                    info->res_data_len[index], 
                    NlmAradCompareResponseFormat_to_string(info->res_format[index])));
        }
    }
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_GTM_OPCODE_CONFIG_INFO_print(
    SOC_SAND_IN  ARAD_KBP_GTM_OPCODE_CONFIG_INFO *info
  )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

	LOG_CLI((BSL_META_U(unit,"  \tGTM Config Info:\n")));
	LOG_CLI((BSL_META_U(unit,
                        "  \t\tTX Data Size: %d\n"), info->tx_data_size));
    LOG_CLI((BSL_META_U(unit,
                        "  \t\tTX Data Type: %d\n"), info->tx_data_type));
    LOG_CLI((BSL_META_U(unit,
                        "  \t\tRX Data Size: %d\n"), info->rx_data_size));
    LOG_CLI((BSL_META_U(unit,
                        "  \t\tRX Data Type: %d\n"), info->rx_data_type));

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_LTR_CONFIG_print(
    SOC_SAND_IN  ARAD_KBP_LTR_CONFIG *info,
    SOC_SAND_IN  int advanced
  )
{
    uint32
        index;

    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    if (info->valid) 
    {
        LOG_CLI((BSL_META_U(unit,
                            "  Opcode: %d, LTR ID: %d, Parallel Searches bitmap: %d, Compare 3 search: %d\n"), 
                 info->opcode, 
                 info->ltr_id,
                 info->parallel_srches_bmp,
				 info->is_cmp3_search));

        LOG_CLI((BSL_META_U(unit,
                            "                                            Master Key ")));
        for (index = 0; index < info->master_key_fields.nof_key_segments; index++) 
        {
            ARAD_KBP_KEY_SEGMENT_print(&info->master_key_fields.key_segment[index]);
        }
        LOG_CLI((BSL_META_U(unit,
                            "\n")));
        
        for (index = 0; index < ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES; index++) 
        {
            if(advanced == 0 && /* don't show searches in dummy tables when using bacis print */
                info->tbl_id[index] >= ARAD_KBP_FRWRD_TBL_ID_DUMMY_IPV4_MC &&
                info->tbl_id[index] <= ARAD_KBP_FRWRD_TBL_ID_DUMMY_5)
            {
                continue;
            }
			LOG_CLI((BSL_META_U(unit,
							"  Search %d: Tbl-ID %3d %-15s type %-10s "), index, info->tbl_id[index],
                            ARAD_KBP_FRWRD_IP_TBL_ID_to_string((ARAD_KBP_FRWRD_IP_DB_TYPE)info->tbl_id[index]),
                         	ARAD_KBP_DB_TYPE_to_string(info->search_type[index])));
            ARAD_KBP_LTR_SINGLE_SEARCH_print(&info->ltr_search[index]);
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    }

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_LTR_SINGLE_SEARCH_print(
    SOC_SAND_IN  ARAD_KBP_LTR_SINGLE_SEARCH *info
    )
{
    uint32 
        index;

    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);
/*
    LOG_CLI((BSL_META_U(unit,
                        "Segments(name,bytes): ")));*/
    for (index = 0; index < info->nof_key_segments; index++) 
    {
        ARAD_KBP_KEY_SEGMENT_print(&info->key_segment[index]);
    }
    
exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

void
  ARAD_KBP_TABLE_CONFIG_print(
      SOC_SAND_IN  int  unit,
      SOC_SAND_IN  ARAD_KBP_TABLE_CONFIG *info
    )
{
    struct kbp_db_stats stats;
    uint32 res;

    if (info->valid) 
    {
        LOG_CLI((BSL_META_U(unit,
                            " %-9d"), info->tbl_id));
        LOG_CLI((BSL_META_U(unit,
                            "%-21s"), ARAD_KBP_FRWRD_IP_TBL_ID_to_string(info->tbl_id)));
        LOG_CLI((BSL_META_U(unit,
                            "%-10d"), info->tbl_size));
        LOG_CLI((BSL_META_U(unit,
                            "%-10d"), info->tbl_width));
        LOG_CLI((BSL_META_U(unit,
                            "%-10d"), ARAD_KBP_AD_WIDTH_TYPE_TO_BITS(info->tbl_asso_width)));
        /* 
        LOG_CLI((BSL_META_U(unit,
                            "%-10d\n"), info->group_id_start)); 
        LOG_CLI((BSL_META_U(unit,
                            "%-10d\n"), info->group_id_end));
        LOG_CLI((BSL_META_U(unit,
                            "%-10d\n"), info->bankNum));
        */ 
        res = kbp_db_stats(AradAppData[unit]->g_gtmInfo[info->tbl_id].tblInfo.db_p, &stats);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit,
                                  "Error in %s(): DB: kbp_db_stats with failed: %s!\n"), 
                                  FUNCTION_NAME(),
                       kbp_get_status_string(res)));
        }
        LOG_CLI((BSL_META_U(unit,
                            "%-10d"), stats.num_entries));
        LOG_CLI((BSL_META_U(unit,
                            "%-10d\n"), stats.capacity_estimate));
    }
}

void
  ARAD_KBP_KEY_SEGMENT_print(
    SOC_SAND_IN  ARAD_KBP_KEY_SEGMENT *info
    )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
    SOC_SAND_CHECK_NULL_INPUT(info);

    LOG_CLI((BSL_META_U(unit,
                        "<%s,%d> "), info->name, info->nof_bytes));

exit:
    SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32
    arad_kbp_ltr_config_get(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  uint32 flp_program,
        SOC_SAND_OUT ARAD_KBP_LTR_CONFIG *config
    )
{
    uint32 prog_id;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);
    SOC_SAND_CHECK_NULL_INPUT(config);

    LOG_CLI((BSL_META_U(unit, "<UnAvailable> ")));
    return 0;

    for(prog_id = 0; prog_id < ARAD_KBP_MAX_NUM_OF_FRWRD_DBS; ++prog_id) {
        /*if(Arad_kbp_ltr_config[unit][prog_id].opcode == flp_program && Arad_kbp_ltr_config[unit][prog_id].valid) {
            *config = Arad_kbp_ltr_config[unit][prog_id];
            ARAD_DO_NOTHING_AND_EXIT;
        }*/
    }

    /* no configuration found */
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_ltr_config_get()", 0, 0);
}

int
arad_kbp_autosync_set(int unit, int enable)
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    
    ARAD_DO_NOTHING_AND_EXIT;
           
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in kbp_sync()", 0, 0);
}
int
 arad_kbp_sync(int unit)
{
    kbp_warmboot_t *warmboot_data;
    uint32 core=0;
    int res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

/* Do not perform sync in CR mode */
#ifdef CRASH_RECOVERY_SUPPORT
    if(!(ARAD_KBP_IS_CR_MODE(unit)))
#endif
    {
        warmboot_data = &kbp_warmboot_data[unit];

        res = kbp_device_save_state(AradAppData[unit]->device_p[core], warmboot_data->kbp_file_read , warmboot_data->kbp_file_write, warmboot_data->kbp_file_fp);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_device_save_state failed: %s\n"),
                      FUNCTION_NAME(), kbp_get_status_string(res)));
            SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
        }
    }
   
    ARAD_DO_NOTHING_AND_EXIT;
           
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in kbp_sync()", 0, 0);
}

void 
  arad_kbp_warmboot_register(int unit,
                             FILE *file_fp,
                             kbp_device_issu_read_fn read_fn, 
                             kbp_device_issu_write_fn write_fn)
{
    kbp_warmboot_data[unit].kbp_file_fp = file_fp;
    kbp_warmboot_data[unit].kbp_file_read = read_fn;
    kbp_warmboot_data[unit].kbp_file_write = write_fn;
}

static void
  arad_kbp_parse_and_print_entry(
    SOC_SAND_IN	int 					unit,
    SOC_SAND_IN	uint8					acl_indication,
	SOC_SAND_IN	uint8 					*data,
	SOC_SAND_IN uint8   				*mask,
	SOC_SAND_IN int 					data_len_bytes,
	SOC_SAND_IN uint32					prefix_length,
	SOC_SAND_IN	uint8 					*ad_data,
	SOC_SAND_IN int 					ad_len_bytes,
    SOC_SAND_IN int  					nof_segments,
    SOC_SAND_IN ARAD_KBP_KEY_SEGMENT 	segment[ARAD_KBP_MAX_NOF_KEY_SEGMENTS],
	SOC_SAND_IN	int 					entry_index)
{
	int i,j,k=0;

	if(entry_index == -1){
		LOG_CLI((BSL_META_U(unit,"| Row Data=0x")));
	}else{
		LOG_CLI((BSL_META_U(unit,"| E.%02d - Row Data=0x"),entry_index));
	}
	/* In all cases, print entry as row of data*/
	for(i=0;i<data_len_bytes;i++) {
		LOG_CLI((BSL_META_U(unit,"%02x"),(data[i])));
	}
	
	if(acl_indication == TRUE){
		/* In case of ACL, print the mask*/
		LOG_CLI((BSL_META_U(unit,"\tMask=0x")));
        for(i=0;i<data_len_bytes;i++) {
            LOG_CLI((BSL_META_U(unit,"%02x"),(mask[i])));
        }
	}else{
		/* In case of frwd, print the parsed fields*/
		LOG_CLI((BSL_META_U(unit,"/%d\t"),prefix_length));

		if(entry_index == -1){
			LOG_CLI((BSL_META_U(unit,"\n| ")));
		}
		for (i=0;i<nof_segments;i++) {
			LOG_CLI((BSL_META_U(unit,"%s"), segment[i].name));
			if ((strcmp(segment[i].name,"SIP")) == 0 || (strcmp(segment[i].name,"DIP") == 0)) {
				if (segment[i].nof_bytes == 4) {
					/* IPV4 IP*/
					LOG_CLI((BSL_META_U(unit,"(dec)=")));
					for (j=0;j<4;j++) {
						LOG_CLI((BSL_META_U(unit,"%d"),(data[k++])));
						if(j != 3) {
							LOG_CLI((BSL_META_U(unit,".")));
						}
					}
				}else if(segment[i].nof_bytes == 16) {
					/*IPV6 IP*/
					LOG_CLI((BSL_META_U(unit,"(hex)=")));
					for (j=0;j<8;j++) {
						LOG_CLI((BSL_META_U(unit,"%02x%02x"),data[k],data[k+1]));
						k=k+2;
						if(j != 7) {
							LOG_CLI((BSL_META_U(unit,".")));
						}
					}
				}
			}else{
				LOG_CLI((BSL_META_U(unit,"=0x")));
				for (j=0;j<segment[i].nof_bytes;j++) {
					LOG_CLI((BSL_META_U(unit,"%02x"),(data[k++])));
				}
			}
			LOG_CLI((BSL_META_U(unit,"\t")));
		}
	}
	LOG_CLI((BSL_META_U(unit,"\n")));

	/* print associate data*/
	if (ad_len_bytes>0) {
		LOG_CLI((BSL_META_U(unit, "| Associate Data (result)=0x")));
		for (i=0;i<ad_len_bytes;i++) {
			LOG_CLI((BSL_META_U(unit,"%02x"),(ad_data[i])));
		}
	}
	LOG_CLI((BSL_META_U(unit,"\n")));
}

static uint32
  arad_kbp_print_searches(
    SOC_SAND_IN	int 					unit,
    SOC_SAND_IN	uint32					ltr_indx,
    SOC_SAND_IN	uint8					*result)
{
	int i,j;
	uint32 result_length;
    uint32 res;
	int result_start_offset = 1; /* the result start offset is 1 byte in the result array 
								   first 2 bytes are the hit bits*/

    ARAD_KBP_LTR_CONFIG             Arad_kbp_ltr_config = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_KBP_LTR_CONFIG_clear(&Arad_kbp_ltr_config);

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, ltr_indx, &Arad_kbp_ltr_config);
    SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

	for (i=0;i<ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES;i++) {
		LOG_CLI((BSL_META_U(unit,"| search index %d\t"),i));

		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_data_len.get(unit, ltr_indx, i, &result_length);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

		if ((Arad_kbp_ltr_config.tbl_id[i] >= ARAD_KBP_FRWRD_TBL_ID_DUMMY_0) &&
			(Arad_kbp_ltr_config.tbl_id[i] < (ARAD_KBP_FRWRD_TBL_ID_DUMMY_0+ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS))) 
		{   /* one of the dummy table. */
			LOG_CLI((BSL_META_U(unit,"Empty\n")));
		}else{
			LOG_CLI((BSL_META_U(unit,"Table ID=%d, %s\t"),Arad_kbp_ltr_config.tbl_id[i], ARAD_KBP_FRWRD_IP_TBL_ID_to_string(Arad_kbp_ltr_config.tbl_id[i])));
			LOG_CLI((BSL_META_U(unit,"{")));
			for (j=0;j<Arad_kbp_ltr_config.ltr_search[i].nof_key_segments;j++) {
				LOG_CLI((BSL_META_U(unit,"%s\\%d bytes"),Arad_kbp_ltr_config.ltr_search[i].key_segment[j].name,Arad_kbp_ltr_config.ltr_search[i].key_segment[j].nof_bytes));
				if (j!=Arad_kbp_ltr_config.ltr_search[i].nof_key_segments-1) {
					LOG_CLI((BSL_META_U(unit,", ")));
				}
			}
			LOG_CLI((BSL_META_U(unit,"}\t")));
			/* add the result of the specific search*/

			if (result[0]& 1<<(7-i)) {
				LOG_CLI((BSL_META_U(unit, "Found\tresult=0x")));
				for(j=0;j<result_length;j++){
					LOG_CLI((BSL_META_U(unit,"%02x"),result[result_start_offset+j]));
				}
				LOG_CLI((BSL_META_U(unit,"\n")));
			}else{
				LOG_CLI((BSL_META_U(unit, "Not Found\n")));
			}
		}
		result_start_offset += result_length;
	}

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_print_searches()", 0, 0);
}

static void
  arad_kbp_print_result(
    SOC_SAND_IN	int 					unit,
	SOC_SAND_IN	uint8 					*data,
	SOC_SAND_IN int 					data_len_bytes,
	SOC_SAND_IN int 					hit_bits_len_bytes)

{
	int indx=0;
	LOG_CLI((BSL_META_U(unit,"| Hit bits=0x")));
	for (indx=0;indx<hit_bits_len_bytes;indx++) {
		LOG_CLI((BSL_META_U(unit,"%02x"),data[indx]));
	}
	LOG_CLI((BSL_META_U(unit,"\t")));
	LOG_CLI((BSL_META_U(unit,"Data=0x")));
	for (;indx<data_len_bytes;indx++) {
		LOG_CLI((BSL_META_U(unit,"%02x"),data[indx]));
	}
	LOG_CLI((BSL_META_U(unit,"\n")));
}

int
  arad_kbp_print_diag_entry_added(
	 SOC_SAND_IN	int unit,
	 SOC_SAND_IN	tableInfo *tbl_info,
     SOC_SAND_IN	uint8     *data,
     SOC_SAND_IN	uint32    prefix_len,
     SOC_SAND_IN    uint8     *mask,
     SOC_SAND_IN	uint8     *ad_data)
{
	int table_id;
	int tbl_width_bytes;
	uint8 is_acl = FALSE;
    ARAD_KBP_TABLE_CONFIG kbp_table_config_info = {0};
    uint32 res;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(tbl_info);
    SOC_SAND_CHECK_NULL_INPUT(data);

    ARAD_KBP_TABLE_CONFIG_clear(&kbp_table_config_info);

	table_id = tbl_info->tbl_id;
	tbl_width_bytes = tbl_info->tbl_width >> 3; /* divide by 8 to get it in bytes*/
	LOG_CLI((BSL_META_U(unit,"|------------------------------------------------------------------------------------------------------------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"| New entry was added to KBP, Table ID = %d, %s\n"),table_id, ARAD_KBP_FRWRD_IP_TBL_ID_to_string(table_id)));

	if (table_id >= ARAD_KBP_FRWRD_IP_NOF_TABLES) {
		is_acl = TRUE;
	}

    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.get(unit, table_id, &kbp_table_config_info); 
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

	arad_kbp_parse_and_print_entry(unit, is_acl,
								   data, mask, tbl_width_bytes,prefix_len,
								   ad_data, kbp_table_config_info.tbl_asso_width/8,
								   kbp_table_config_info.entry_key_parsing.nof_segments,
								   kbp_table_config_info.entry_key_parsing.key_segment,
								   -1);

	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------------------------------------------------------\n\n")));

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_print_diag_entry_added()", 0, 0);
}

int
  arad_kbp_print_diag_all_entries(int unit, const char *print_table){

    struct kbp_db 
        *db_p = NULL;
    struct kbp_entry_iter
        *iter_p = NULL;
    struct kbp_entry
        *db_entry_p = NULL;
	struct kbp_entry_info
		info;
    struct kbp_ad_db 
        *ad_db_p = NULL;

	uint32 table_id;
	uint32 res;
	uint8 is_acl = FALSE;
	int i;
	uint8 assoData[256];
	uint8 valid_ad = 0;
	int half[3];
        int iterate_start = 0;
        int iterate_end = ARAD_KBP_MAX_NUM_OF_TABLES;

        ARAD_KBP_TABLE_CONFIG kbp_table_config_info = {0};

        SOC_SAND_INIT_ERROR_DEFINITIONS(0);

        ARAD_KBP_TABLE_CONFIG_clear(&kbp_table_config_info);


        if(NULL == AradAppData[unit])
        {
           LOG_CLI((BSL_META_U(unit, "\n***** KBP Device is not yet installed with configuration!\n\n")));
           return 0;
        }

        /* setting start and end table IDs to print */
        /* all */
        if(!sal_strncasecmp(print_table, "all", strlen(print_table))) {
           iterate_start = 0;
           iterate_end = ARAD_KBP_MAX_NUM_OF_TABLES;
        }
        /* ipv4 */
        else if (!sal_strncasecmp(print_table, "ipv4", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV4_MC + 1;
        }
        else if (!sal_strncasecmp(print_table, "ipv4_uc", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0 + 1;
        }
        else if (!sal_strncasecmp(print_table, "ipv4_uc_rpf", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1 + 1;
        }
        else if (!sal_strncasecmp(print_table, "ipv4_mc", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV4_MC;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV4_MC + 1;
        }
        /* ipv6 */
        else if (!sal_strncasecmp(print_table, "ipv6", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV6_MC + 1;
        }
        else if (!sal_strncasecmp(print_table, "ipv6_uc", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0 + 1;
        }
        else if (!sal_strncasecmp(print_table, "ipv6_uc_rpf", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1 + 1;
        }
        else if (!sal_strncasecmp(print_table, "ipv6_mc", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_IPV6_MC;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_IPV6_MC + 1;
        }
        /* mpls */
        else if (!sal_strncasecmp(print_table, "mpls", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_LSR;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_LSR + 1;
        }
        /* trill */
        else if (!sal_strncasecmp(print_table, "trill", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_TRILL_UC;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_TRILL_MC + 1;
        }
        else if (!sal_strncasecmp(print_table, "trill_uc", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_TRILL_UC;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_TRILL_UC + 1;
        }
        else if (!sal_strncasecmp(print_table, "trill_mc", strlen(print_table))) {
           iterate_start = ARAD_KBP_FRWRD_TBL_ID_TRILL_MC;
           iterate_end = ARAD_KBP_FRWRD_TBL_ID_TRILL_MC + 1;
        }
        /* ACL */
        else if (!sal_strncasecmp(print_table, "acl", strlen(print_table))) {
           iterate_start = ARAD_KBP_ACL_TABLE_ID_OFFSET;
           iterate_end = ARAD_KBP_MAX_NUM_OF_TABLES;
        }
        /* default: even if input is wrong, print all tables */



	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|---------------------------------------  KBP Entries  -------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------\n")));

	for (table_id = iterate_start; table_id < iterate_end; table_id++) {
		i=0;
		half[0] = 0;
		half[1] = 0;
		half[2] = 0;

		/*Get kbp_db*/
		res =  arad_kbp_alg_kbp_db_get(
				unit,
				table_id, 
				&db_p
			);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
		if(db_p == NULL){
			continue;
		}
		res =  arad_kbp_alg_kbp_ad_db_get(
				unit,
				table_id, 
				&ad_db_p
			);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

		valid_ad = (ad_db_p == NULL) ? 0 : 1;

        res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_table_config_info.get(unit, table_id, &kbp_table_config_info); 
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

		/*Get iterator for first entry in kbp table*/
		res = kbp_db_entry_iter_init(db_p, &iter_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_init failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
		    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
        }
		SOC_SAND_CHECK_NULL_PTR(iter_p, 20, exit);

		res = kbp_db_entry_iter_next(db_p, iter_p, &db_entry_p);
        if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
            LOG_ERROR(BSL_LS_SOC_TCAM,
                      (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_next failed: %s\n"),
    				  FUNCTION_NAME(), kbp_get_status_string(res)));
		    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
        }

		while (db_entry_p != NULL) {
			if (i==0) {
				LOG_CLI((BSL_META_U(unit,"|\n")));
				LOG_CLI((BSL_META_U(unit,"| Table ID = %d, %s:\n"), table_id,ARAD_KBP_FRWRD_IP_TBL_ID_to_string(table_id)));
			}
			res = kbp_entry_get_info(db_p, db_entry_p, &info);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_entry_get_info failed: %s\n"),
        				  FUNCTION_NAME(), kbp_get_status_string(res)));
			    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
            }

			sal_memset(assoData, 0x0, sizeof(uint8) * SOC_DPP_TCAM_ACTION_ELK_KBP_GET_LEN_BYTES(unit));

			if (valid_ad) {
				res = kbp_ad_db_get(ad_db_p, info.ad_handle, assoData);
				if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK) {
                    LOG_ERROR(BSL_LS_SOC_TCAM,
                              (BSL_META_U(unit, "Error in %s(): kbp_ad_db_get failed: %s\n"),
            				  FUNCTION_NAME(), kbp_get_status_string(res)));
					SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 100, exit);
				}
			}

			/* print entry*/
			if (table_id >= ARAD_KBP_FRWRD_IP_NOF_TABLES) {
				is_acl = TRUE;
			}

			arad_kbp_parse_and_print_entry(unit, is_acl,
										   info.data, info.mask, info.width_8,info.prio_len,
										   assoData, (valid_ad == 1)?kbp_table_config_info.tbl_asso_width/8:0,
										   kbp_table_config_info.entry_key_parsing.nof_segments,
										   kbp_table_config_info.entry_key_parsing.key_segment,
										   i++);

			half[info.which_half]++;
			res = kbp_db_entry_iter_next(db_p, iter_p, &db_entry_p);
            if(ARAD_KBP_TO_SOC_RESULT(res) != SOC_SAND_OK){
                LOG_ERROR(BSL_LS_SOC_TCAM,
                          (BSL_META_U(unit, "Error in %s(): kbp_db_entry_iter_next failed: %s\n"),
        				  FUNCTION_NAME(), kbp_get_status_string(res)));
			    ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
            }
		}
		if (i != 0) {
			if (table_id == ARAD_KBP_FRWRD_TBL_ID_IPV4_DC) {
				LOG_CLI((BSL_META_U(unit, "|half[0] = %d, half[1]=%d, half[2]=%d, total=%d\n"),half[0],half[1],half[2],half[0] + half[1] + half[2]));
			}
			LOG_CLI((BSL_META_U(unit,"|___________________________________________________________________________________________\n")));
		}
	}
	LOG_CLI((BSL_META_U(unit,"|                                                                                           \n")));
	LOG_CLI((BSL_META_U(unit,"|-----------------------------------  End Of KBP Entries  ----------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|___________________________________________________________________________________________\n\n")));
exit:
	if (iter_p != NULL) {
		kbp_db_entry_iter_destroy(db_p, iter_p);
	}
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_print_diag_all_entries()", 0, 0);
}

int
  arad_kbp_print_diag_last_packet(int unit, int core, int flp_program){

	uint32 res;
	uint32 signal_value[24];
	uint8 prog_opcode, signal_opcode, sw_prog_id=0xff;
	int core_id = core;
	SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE signal_id = SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_FLP_2_KBP_KEY_PART5;
	ARAD_KBP_LTR_SINGLE_SEARCH master_key;
	int signal_nof_bits, signal_nof_bytes, signal_nof_32bits;
	int master_key_len_bytes=0,master_key_bytes_counter;
	int i,indx=0;
	uint8 *signal_byte_ptr;
	uint8 master_key_value[ARAD_KBP_MASTER_KEY_MAX_LENGTH_BYTES];
	uint8 result_value[ARAD_KBP_RESULT_MAX_LENGTH_BYTES]={0};
    uint32   db_type, ltr_id;
	SOC_SAND_SUCCESS_FAILURE    success;
    int prog_id, num_of_progs;
    ARAD_KBP_LTR_CONFIG ltr_config_info = {0};

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

	master_key.nof_key_segments=0;

    ARAD_KBP_LTR_CONFIG_clear(&ltr_config_info);

	if (flp_program == -1) {
		/* get last program data */
		res = arad_pp_flp_access_print_last_programs_data(unit, core_id, 0, &prog_id, &num_of_progs);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
	}else{
		prog_id = flp_program;
	}

    /* find the kbp opcode associated with that program*/
    arad_pp_prog_index_to_flp_app_get(unit,prog_id,&sw_prog_id);
    prog_opcode = ARAD_KBP_FLP_PROG_TO_OPCODE(sw_prog_id);

	/* get last packet opcode from signal - opcode is the FLP program ID */
	res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_FLP_2_KBP_OPCODE, signal_value, &signal_nof_bits);
	ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

	/* opcode is only one byte length - so need to take its 8 LSB from the first 32-bit array cell. */
	signal_opcode = signal_value[0]&0xFF;
	if (prog_opcode != signal_opcode) {
        uint8 last_prog_id;
        arad_pp_flp_app_to_prog_index_get(unit,ARAD_KBP_OPCODE_TO_FLP_PROG(signal_opcode),&last_prog_id);
		if (flp_program == -1) {
			LOG_CLI((BSL_META_U(unit, "Last FLP program (ID=%d) did not perform a KBP lookup on core=%d\nLast FLP program with KBP lookup is ID=%d\n"), prog_id, core_id, last_prog_id));
		}else{
			LOG_CLI((BSL_META_U(unit, "The required FLP program (ID=%d) did not perform a KBP lookup on core=%d\nLast FLP program with KBP lookup is ID=%d\n"), prog_id, core_id, last_prog_id));
		}
		return 0;
	}

	/* find the matched LTR by the opcode */
    res = arad_kbp_opcode_to_db_type(unit, signal_opcode, &db_type, &ltr_id, &success);
	ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
    res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, db_type, &ltr_config_info);

	master_key = ltr_config_info.master_key_fields;

	/* compute the master key len from its segmennts length */
	for (i=0;i<master_key.nof_key_segments;i++) {
		master_key_len_bytes += master_key.key_segment[i].nof_bytes;
	}
	master_key_bytes_counter = master_key_len_bytes;

	/* an error if master key length is still zero*/
	if (master_key_len_bytes == 0) {
		SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_kbp_print_diag_last_key()\nmaster key length is zero", signal_opcode, 0);
	}

	/* 	according to the master key length in bytes,
		decide which parts (5-1) of the signal SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_FLP_2_KBP_KEY
		we need to read. master key max length is 640 bits=80bytes
		part 5 - 1  byte  (  8 bit )  
		part 4 - 32 bytes (256 bits)
		part 3 - 32 bytes (256 bits)
		part 2 - 32 bytes (256 bits)
		part 1 - headers and payload
	 
	    In addition, copy the signal to "master_key_value" - parsing will done after reading the entire signal
	 
	    explanation for the reading order:
	    part 5 is only one byte length - so need to take its 8 LSB from the first 32-bit array cell.
	    for all other parts of hte signal:
	    	start read the signal from the last 32-bit array cell, and each 32-bit cell need to be read as:
					MSB |	8bit	|	8bit	|	8bit	|	8bit	| LSB
	    				|	1st		|	2nd		|	3rd		|	4th		|
	    	NOTE: when a 8bit ptr is set to a casted 32-bit cell,
				  it start points to its 8 MSB bits  
	*/

	/* always read part 5 - first and only byte*/
	res = arad_pp_signal_mngr_signal_get(unit, core_id, signal_id, signal_value, &signal_nof_bits);
	ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

	master_key_value[indx++] = signal_value[0]&0xFF;
	master_key_bytes_counter -= 1; /* reducte 1 byte from the counter */
	signal_id--; /* SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_FLP_2_KBP_KEY_PART4 */

	while (master_key_bytes_counter > 0) {
		/* read parts 4,3,2 (according to length) according to the above explanation */
		res = arad_pp_signal_mngr_signal_get(unit, core_id, signal_id, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

		signal_nof_bytes  = (signal_nof_bits+7)/8;
		signal_nof_32bits = (signal_nof_bits+31)/32;

		while (signal_nof_32bits > 0) {
			/* place the signal by a simple order in the master key array */
			signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
			master_key_value[indx++] = *signal_byte_ptr++;
			master_key_value[indx++] = *signal_byte_ptr++;
			master_key_value[indx++] = *signal_byte_ptr++;
			master_key_value[indx++] = *signal_byte_ptr++;
			signal_nof_32bits--;
		}
		master_key_bytes_counter -= signal_nof_bytes;
		signal_id--;
	}

	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|------------------------------------  KBP Last Packet  ------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"| Core = %d, FLP program = %s (opcode=%d), LTR index = %d\n"), core_id, ARAD_KBP_FRWRD_IP_DB_TYPE_to_string(db_type), signal_opcode, db_type));

	/* print master key*/
	LOG_CLI((BSL_META_U(unit,"|\n")));
	LOG_CLI((BSL_META_U(unit,"| -Master key \\ %d bytes:\n"),master_key_len_bytes));
	arad_kbp_parse_and_print_entry(unit, FALSE,
								   master_key_value, NULL, master_key_len_bytes,0,
								   NULL, 0,
								   master_key.nof_key_segments,
								   master_key.key_segment,
								   -1);

	/* read result signal kbp2flp */
	res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES, signal_value, &signal_nof_bits);
	ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

	signal_nof_bytes  = (signal_nof_bits+7)/8;
	signal_nof_32bits = (signal_nof_bits+31)/32;
	indx = 0;
	while (signal_nof_32bits > 0) {
		/* place the signal by a simple order in the result array */
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
		result_value[indx++] = *signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		signal_nof_32bits--;
	}

    if (SOC_IS_QAX(unit)) {
		/* read result signal kbp2flp LSB PART2*/
		res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES_LSB_PART2, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);
        /* part 2 of the result is 104b in QAX, so:
            1. read the last 1 byte from 32b register
            2. read the rest 12 bytes
        */
		signal_nof_32bits = (signal_nof_bits+31)/32;
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
        signal_byte_ptr += 3;
        result_value[indx++] = *signal_byte_ptr++;
        signal_nof_32bits--;

        while (signal_nof_32bits > 0) {
            /* place the signal by a simple order in the result array */
            signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
            result_value[indx++] = *signal_byte_ptr++;
            result_value[indx++] = *signal_byte_ptr++;
            result_value[indx++] = *signal_byte_ptr++;
            result_value[indx++] = *signal_byte_ptr++;
            signal_nof_32bits--;
        }

		/* read result signal kbp2flp LSB PART1*/
		res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES_LSB_PART1, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

		/* part 1 of the result is 24b in QAX, so:
            1. read the last 3 bytes from the 32b register
		*/
		signal_nof_32bits = (signal_nof_bits+31)/32;
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
        signal_byte_ptr++;
        result_value[indx++] = *signal_byte_ptr++;
        result_value[indx++] = *signal_byte_ptr++;
        result_value[indx++] = *signal_byte_ptr++;
		signal_nof_32bits--;
    }
	else if (SOC_IS_JERICHO_PLUS(unit)) {
		/* read result signal kbp2flp LSB PART2*/
		res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES_LSB_PART2, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

		/* part 2 of the result is 109b in jericho plus,so:
			1. read the last 32b register, skip the high 2 bytes
			2. start shifting the 5 relevant bits and add to them the 3 msb from the next byte register
		*/
		signal_nof_32bits = (signal_nof_bits+31)/32;
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
		signal_byte_ptr++;
		signal_byte_ptr++;
		result_value[indx] = (*signal_byte_ptr&0x1F)<<3;
		signal_byte_ptr++;
		result_value[indx++] |= (*signal_byte_ptr&0xE0)>>5;
		result_value[indx] = (*signal_byte_ptr&0x1F)<<3;
		signal_byte_ptr++;
		signal_nof_32bits--;

		while (signal_nof_32bits > 0) {
			/* place the signal by a simple order in the result array */
			signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
    		result_value[indx++] |= (*signal_byte_ptr&0xE0)>>5;
    		result_value[indx] = (*signal_byte_ptr&0x1F)<<3;
    		signal_byte_ptr++;
    		result_value[indx++] |= (*signal_byte_ptr&0xE0)>>5;
    		result_value[indx] = (*signal_byte_ptr&0x1F)<<3;
    		signal_byte_ptr++;
    		result_value[indx++] |= (*signal_byte_ptr&0xE0)>>5;
    		result_value[indx] = (*signal_byte_ptr&0x1F)<<3;
    		signal_byte_ptr++;
    		result_value[indx++] |= (*signal_byte_ptr&0xE0)>>5;
    		result_value[indx] = (*signal_byte_ptr&0x1F)<<3;
    		signal_byte_ptr++;
			signal_nof_32bits--;
		}

		/* read result signal kbp2flp LSB PART1*/
		res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES_LSB_PART1, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

		/* part 1 of the result is 19b in jericho plus,so:
			1. read the high 3 bits to complete the 5 bits from part 1
			2. continue as usual, byte by byte
		*/
		signal_nof_32bits = (signal_nof_bits+31)/32;
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];

		signal_byte_ptr++;
		result_value[indx++] |= (*signal_byte_ptr&0x07);
		signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		signal_nof_32bits--;
	}
	else if (SOC_IS_JERICHO(unit)) {
		/* read result signal kbp2flp LSB PART2*/
		res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES_LSB_PART2, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

		/* part 2 of the result is 39b in jericho,so:
			1. read the high 7 bits
			2. than, in the last 32b register, read the data without byte aligning:
				a. read high bit to complete the previos 7 bits
				b. read the extra 7 bits to the next byte register...
		*/
		signal_nof_32bits = (signal_nof_bits+31)/32;
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
		signal_byte_ptr++;
		signal_byte_ptr++;
		signal_byte_ptr++;
		result_value[indx] = ((*signal_byte_ptr&0x7F))<<1;
		signal_nof_32bits--;

		/* place the signal by a simple order in the result array */
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
		result_value[indx++] |= (*signal_byte_ptr&0x80)>>7;
		result_value[indx]    = (*signal_byte_ptr&0x7F)<<1;
		signal_byte_ptr++;
		result_value[indx++] |= (*signal_byte_ptr&0x80)>>7;
		result_value[indx]    = (*signal_byte_ptr&0x7F)<<1;
		signal_byte_ptr++;
		result_value[indx++] |= (*signal_byte_ptr&0x80)>>7;
		result_value[indx]    = (*signal_byte_ptr&0x7F)<<1;
		signal_byte_ptr++;
		result_value[indx++] |= (*signal_byte_ptr&0x80)>>7;
		result_value[indx]    = (*signal_byte_ptr&0x7F)<<1;

		/* read result signal kbp2flp LSB PART1*/
		res = arad_pp_signal_mngr_signal_get(unit, core_id, SOC_DPP_SIGNAL_MNGR_SIGNAL_TYPE_KBP_2_FLP_RES_LSB_PART1, signal_value, &signal_nof_bits);
		ARAD_KBP_CHECK_FUNC_RESULT(res, 10, exit);

		/* part 1 of the result is 89b in jericho,so:
			1. read the high 1 bit to complete the 7 bits from part 1
			2. continue as usual, byte by byte
		*/
		signal_nof_32bits = (signal_nof_bits+31)/32;
		signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];

		result_value[indx++] |= (*signal_byte_ptr&0x1);
		signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		result_value[indx++] = *signal_byte_ptr++;
		signal_nof_32bits--;

		while (signal_nof_32bits > 0) {
			/* place the signal by a simple order in the result array */
			signal_byte_ptr = (uint8*)&signal_value[signal_nof_32bits-1];
			result_value[indx++] = *signal_byte_ptr++;
			result_value[indx++] = *signal_byte_ptr++;
			result_value[indx++] = *signal_byte_ptr++;
			result_value[indx++] = *signal_byte_ptr++;
			signal_nof_32bits--;
		}
	}
	/* print lookups*/
	LOG_CLI((BSL_META_U(unit,"|\n")));
	LOG_CLI((BSL_META_U(unit,"| -Look Ups associated with the LTR:\n")));
	arad_kbp_print_searches(unit, db_type, result_value);

	/* print result*/
	LOG_CLI((BSL_META_U(unit,"|\n")));
	LOG_CLI((BSL_META_U(unit,"| -Result (Row of Data):\n")));
	arad_kbp_print_result(unit, result_value, 32, 1);


	LOG_CLI((BSL_META_U(unit,"|                                                                                           \n")));
	LOG_CLI((BSL_META_U(unit,"|-----------------------------------  End Of KBP packet ------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|___________________________________________________________________________________________\n\n")));

exit:
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_print_diag_last_key()", 0, 0);
}

int
  arad_kbp_print_search_result(int unit, uint32 ltr_idx, uint8 *master_key_buffer, struct kbp_search_result search_rslt){

	int res, i, j;
	uint8 result[32] = {0};
	uint32 result_width_byte, result_length_byte, result_offset_byte = 1;
	ARAD_KBP_LTR_SINGLE_SEARCH master_key;
	ARAD_KBP_LTR_CONFIG ltr_config_info;
	int master_key_len_bytes = 0;
	NlmAradCompareResponseFormat configured_result_type;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    sal_memset(result, 0x0, sizeof(uint8) * 32);

	for (i=0;i<ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS;i++) {
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_data_len.get(unit, ltr_idx, i, &result_length_byte);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
		res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_gtm_ltr_info.res_format.get(unit, ltr_idx, i, &configured_result_type);
		SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

		
		if(hit_bit_result_for_do_search & (0x1 << (7 - i))){
			if ((search_rslt.resp_type[i] > KBP_ONLY_AD_128B) || (search_rslt.resp_type[i] < KBP_ONLY_AD_24B)) {
				SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 10, exit);
			}
			/* write the hit bit */
			result[0] |= 0x1<<(7-i);

			/* translate res_format to length in bytes:
			 * bytes = 2^(res_format -1)*4 = 2^(res_format+1) = 0x1<<(res_format+1)
			 */
			result_width_byte = (0x1<<((int)configured_result_type + 1));

			for (j=0;j<result_length_byte;j++) {
				result[result_offset_byte + j] = search_rslt.assoc_data[i][result_width_byte-(result_length_byte-j)];
			}

		}
		result_offset_byte += result_length_byte;
	}

	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|-------------------------------------  KBP Do search  -------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|-------------------------------------------------------------------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|LTR index = %d\n"),ltr_idx));
	LOG_CLI((BSL_META_U(unit,"|NOTE!! The result value of R2 in compare3 LTR is invalid, but hit indication is valid.\n")));

	res = sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_kbp_ltr_config.get(unit, ltr_idx, &ltr_config_info);
	SOC_SAND_CHECK_FUNC_RESULT(res, 10+ltr_idx, exit);

	master_key = ltr_config_info.master_key_fields;

	/* compute the master key len from its segmennts length */
	for (i=0;i<master_key.nof_key_segments;i++) {
		master_key_len_bytes += master_key.key_segment[i].nof_bytes;
	}

	/* print master key*/
	LOG_CLI((BSL_META_U(unit,"|\n")));
	LOG_CLI((BSL_META_U(unit,"| -Master key \\ %d bytes:\n"),master_key_len_bytes));
	arad_kbp_parse_and_print_entry(unit, FALSE,
								   master_key_buffer, NULL, master_key_len_bytes,0,
								   NULL, 0,
								   master_key.nof_key_segments,
								   master_key.key_segment,
								   -1);
	/* print lookups*/
	LOG_CLI((BSL_META_U(unit,"|\n")));
	LOG_CLI((BSL_META_U(unit,"| -Look Ups associated with the LTR:\n")));
	arad_kbp_print_searches(unit, ltr_idx, result);

	LOG_CLI((BSL_META_U(unit,"|                                                                                           \n")));
	LOG_CLI((BSL_META_U(unit,"|---------------------------------  End Of KBP Do search -----------------------------------\n")));
	LOG_CLI((BSL_META_U(unit,"|___________________________________________________________________________________________\n\n")));

exit:
	SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_kbp_print_search_result()", 0, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* #if defined(BCM_88650_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */

