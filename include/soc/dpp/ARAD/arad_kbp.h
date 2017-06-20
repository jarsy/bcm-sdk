/* $Id: arad_kbp.h,v 1.31 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#if defined(INCLUDE_KBP) && !defined(BCM_88030) && !defined(__ARAD_KBP_INCLUDED__)
/* { */
#define __ARAD_KBP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>
/* uncomment this flag to enable time measurments */
/* #define ARAD_KBP_ROP_TIME_MEASUREMENTS */

#include <shared/swstate/sw_state_workarounds.h>

#include <soc/kbp/alg_kbp/include/db.h>
#include <soc/kbp/alg_kbp/include/default_allocator.h>
#include <soc/kbp/alg_kbp/include/device.h>
#include <soc/kbp/alg_kbp/include/key.h>
#include <soc/kbp/alg_kbp/include/instruction.h>
#include <soc/kbp/alg_kbp/include/errors.h>
#include <soc/kbp/alg_kbp/include/ad.h>
#include <soc/kbp/alg_kbp/include/kbp_legacy.h>
#include <soc/kbp/alg_kbp/include/init.h>
#include <soc/kbp/alg_kbp/include/kbp_pcie.h>
#include <soc/hwstate/hw_log.h>

#include <soc/dpp/ARAD/arad_api_nif.h>
#include <soc/dpp/ARAD/arad_kbp_rop.h>
#include <soc/dpp/ARAD/arad_sw_db_tcam_mgmt.h>

#include <soc/dpp/TMC/tmc_api_tcam.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_key.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>
#include <soc/dpp/PPC/ppc_api_fp.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/* 
 * Global defines
 */

/* Max number of entries: 1 million entries of 80b for KBP NL88650 */
#define ARAD_KBP_NL_88650_MAX_NOF_ENTRIES (SOC_TMC_TCAM_NL_88650_MAX_NOF_ENTRIES)

/* Timeout for checking stability of KBP ILKN port */
#define KBP_PORT_STABLE_TIMEOUT 2000000

#define ARAD_KBP_ENABLE_FRWRD_TABLE(frwrd_table)     \
    (SOC_DPP_CONFIG(unit)->arad->init.elk.enable   \
     && (0 != SOC_DPP_CONFIG(unit)->arad->init.elk.frwrd_table)) 

#define ARAD_KBP_ENABLE_IPV4_UC             ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_0])
#define ARAD_KBP_ENABLE_IPV4_MC             ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV4_MC])
#define ARAD_KBP_ENABLE_IPV6_UC             ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_0])
#define ARAD_KBP_ENABLE_IPV6_MC             ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV6_MC])
#define ARAD_KBP_ENABLE_TRILL_UC            ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_TRILL_UC])
#define ARAD_KBP_ENABLE_TRILL_MC            ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_TRILL_MC])
#define ARAD_KBP_ENABLE_MPLS                ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_LSR])
#define ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED  ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_LSR_IP_SHARED])
#define ARAD_KBP_ENABLE_IPV6_EXTENDED       ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6])
#define ARAD_KBP_ENABLE_P2P_EXTENDED        ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P])
#define ARAD_KBP_ENABLE_INRIF_MAPPING       ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING])

#define ARAD_KBP_ENABLE_IPV4_RPF            ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV4_UC_RPF_1])
#define ARAD_KBP_ENABLE_IPV6_RPF            ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV6_UC_RPF_1])
#define ARAD_KBP_ENABLE_IPV4_DC             ARAD_KBP_ENABLE_FRWRD_TABLE(fwd_table_size[ARAD_KBP_FRWRD_TBL_ID_IPV4_DC])

#define ARAD_KBP_ENABLE_ANY_IPV4UC_PROGRAM  (ARAD_KBP_ENABLE_IPV4_UC || ARAD_KBP_ENABLE_IPV4_MPLS_EXTENDED || ARAD_KBP_ENABLE_IPV4_DC)

#define ARAD_KBP_MASTER_KEY_MAX_LENGTH          (NLM_TBL_WIDTH_640)
#define ARAD_KBP_MASTER_KEY_MAX_LENGTH_BYTES    (ARAD_KBP_MASTER_KEY_MAX_LENGTH>>3)
#define ARAD_KBP_RESULT_MAX_LENGTH_BYTES        (32)

#define ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES   (ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS)
#define NLM_ARAD_MAX_NUM_RESULTS_PER_INST       ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES
#define ARAD_KBP_CMPR3_SKIPPED_SEARCH	   (1)
#define ARAD_KBP_CMPR3_FIRST_ACL		   (4)

#define ARAD_KBP_IPV4DC_24BIT_FWD          (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ext_tcam_dc_24bit_result", 0) ? 1 : 0)
#define ARAD_KBP_IPV4DC_24BIT_FWD_RES_SIZE (24)
#define ARAD_KBP_IPV4DC_RES1_PAD_BYTES	   (ARAD_KBP_IPV4DC_24BIT_FWD ? 3 : 0)
#define ARAD_KBP_IPV4DC_RES3_PAD_BYTES	   (ARAD_KBP_IPV4DC_24BIT_FWD ? 3 : 6)

/* Currently there is a direct mapping between program to opcode */
#define ARAD_KBP_FLP_PROG_TO_OPCODE(program)    (program)
#define ARAD_KBP_OPCODE_TO_FLP_PROG(opcode)     (opcode)

#define ARAD_KBP_TABLE_INDX_TO_DUMMY_TABLE_ID(_index)    (ARAD_KBP_FRWRD_TBL_ID_DUMMY_0 + _index)


#ifdef CRASH_RECOVERY_SUPPORT
/* crash recovery parameters*/
#define ARAD_KBP_IS_CR_MODE(unit) (SOC_UNIT_VALID(unit) && SOC_IS_JERICHO(unit) && (DCMN_CR_JOURNALING_MODE_DISABLED != soc_dcmn_cr_journaling_mode_get(unit)))
#define ARAD_KBP_NV_MEMORY_SIZE (50*1024*1024)
#endif
/* 
 * Opcde
 */
#define ARAD_KBP_OPCODE_NUM_MAX 256

#define ARAD_KBP_CPU_WR_LUT_OPCODE 255
#define ARAD_KBP_CPU_RD_LUT_OPCODE 254
#define ARAD_KBP_CPU_PIOWR_OPCODE 253
#define ARAD_KBP_CPU_PIORDX_OPCODE 252
#define ARAD_KBP_CPU_PIORDY_OPCODE 251
#define ARAD_KBP_CPU_ERR_OPCODE 250
#define ARAD_KBP_CPU_BLK_COPY_OPCODE 249
#define ARAD_KBP_CPU_BLK_MOVE_OPCODE 248
#define ARAD_KBP_CPU_BLK_CLR_OPCODE 247
#define ARAD_KBP_CPU_BLK_EV_OPCODE 246
#define ARAD_KBP_CPU_CTX_BUFF_WRITE_OPCODE 10
#define ARAD_KBP_CPU_PAD_OPCODE 0

/* 
 * LTR
 */
#define ARAD_KBP_ROP_LTR_NUM_MAX            256
#define ARAD_KBP_MAX_NOF_KEY_SEGMENTS       10
#define ARAD_KBP_MAX_SEGMENT_LENGTH_BYTES   16

/*
 * Instruction
 */
#define ARAD_KBP_CPU_BLK_COPY_INSTRUCTION 8
#define ARAD_KBP_CPU_BLK_MOVE_INSTRUCTION 9
#define ARAD_KBP_CPU_BLK_CLR_INSTRUCTION 10
#define ARAD_KBP_CPU_BLK_EV_INSTRUCTION 11

/*
 * Record Size
 */
#define ARAD_KBP_CPU_BLK_COPY_REC_SIZE 8
#define ARAD_KBP_CPU_BLK_MOVE_REC_SIZE 8
#define ARAD_KBP_CPU_BLK_CLR_REC_SIZE 5
#define ARAD_KBP_CPU_BLK_EV_REC_SIZE 5

/* Default CTX_BUFF_WRITE and CMP1/2/3 LTR*/
#define ARAD_KBP_CPU_CTX_BUFF_WRITE_LTR 11

/* Arad Application */
#define ARAD_KBP_APP_NUM_OF_FRWRD_TABLES 2


/* Flags */
#define ARAD_KBP_TABLE_ALLOC_CHECK_ONLY     ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY

/* Translate KBP result to SOC result */
#define ARAD_KBP_TO_SOC_RESULT(result)     ((result == 0) ? SOC_SAND_OK : SOC_SAND_ERR)
#define ARAD_KBP_CHECK_FUNC_RESULT(f_res,err_num,err_exit_label)\
  if(soc_sand_update_error_code(ARAD_KBP_TO_SOC_RESULT(f_res), &ex ) != no_err)         \
  {                                                         \
    exit_place = err_num;                                   \
    FUNC_RESULT_PRINT   \
    goto err_exit_label;                                    \
  }

/* GTM opcode flgs */
#define GTM_OPCODE_CONFIG_DATA_TYPE_TX_REQUEST (1)
#define GTM_OPCODE_CONFIG_DATA_TYPE_RX_REPLY   (3)

/* } */

/*************
 * ENUMS     *
 *************/
/* { */

typedef enum
{
    ARAD_KBP_ACL_IN_MASTER_KEY_LSB_ONLY  = 0,/* ALL ACL will be in the LSB part*/
	ARAD_KBP_ACL_IN_MASTER_KEY_MSB_ONLY  = 1,/* ALL ACL will be in the MSB part*/
	ARAD_KBP_ACL_IN_MASTER_KEY_LSB_MSB   = 2 /* ACL will be splited into two parts*/
} ARAD_KBP_ACL_IN_MASTER_KEY_TYPE;

typedef enum
{
    /* IPV4 */
    ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC,
    ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC_RPF,
    ARAD_KBP_FRWRD_DB_TYPE_IPV4_MC_RPF,
    /* IPV6 */
    ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC,
    ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC_RPF_2PASS,
    ARAD_KBP_FRWRD_DB_TYPE_IPV6_UC_RPF,
    ARAD_KBP_FRWRD_DB_TYPE_IPV6_MC_RPF,
    /* TRILL */
    ARAD_KBP_FRWRD_DB_TYPE_LSR,
    ARAD_KBP_FRWRD_DB_TYPE_TRILL_UC,
    ARAD_KBP_FRWRD_DB_TYPE_TRILL_MC,
    ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD,

    /* Exclusive*/
    ARAD_KBP_FRWRD_DB_TYPE_IPV4_DC = ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD,
    ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED,
    ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED_FOR_IP,
    ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED_FOR_IP_WITH_RPF,
    ARAD_KBP_FRWRD_DB_TYPE_IP_LSR_SHARED_FOR_LSR,
    ARAD_KBP_FRWRD_DB_TYPE_EXTENDED_IPv6,
    ARAD_KBP_FRWRD_DB_TYPE_EXTENDED_P2P,
    ARAD_KBP_FRWRD_DB_TYPE_DUMMY_FRWRD,
    ARAD_KBP_FRWRD_DB_TYPE_INRIF_MAPPING,
    ARAD_KBP_FRWRD_DB_NOF_TYPE_ARAD_PLUS,

    ARAD_KBP_FRWRD_DB_NOF_TYPES,

    ARAD_KBP_FRWRD_DB_ACL_OFFSET	= ARAD_KBP_FRWRD_DB_NOF_TYPES,
    /*ARAD_KBP_MAX_NUM_OF_FRWRD_DBS   = ARAD_KBP_FRWRD_DB_ACL_OFFSET + SOC_DPP_DEFS_MAX(NOF_FLP_PROGRAMS),*/
	ARAD_KBP_MAX_NUM_OF_FRWRD_DBS   = SOC_DPP_DEFS_MAX(NOF_FLP_PROGRAMS),

} ARAD_KBP_FRWRD_IP_DB_TYPE;

typedef enum
{
    ARAD_KBP_LTR_ID_FIRST           		= 0,
    /* IPV4 */
    ARAD_KBP_FRWRD_LTR_IPV4_UC      		= ARAD_KBP_LTR_ID_FIRST + ARAD_KBP_FRWRD_DB_TYPE_IPV4_UC,
    ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF  		= ARAD_KBP_FRWRD_LTR_IPV4_UC 			+ 2,
    ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF  		= ARAD_KBP_FRWRD_LTR_IPV4_UC_RPF 		+ 2,
    /* IPV6 */
    ARAD_KBP_FRWRD_LTR_IPV6_UC      		= ARAD_KBP_FRWRD_LTR_IPV4_MC_RPF 		+ 2,
    ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF_2PASS  	= ARAD_KBP_FRWRD_LTR_IPV6_UC  			+ 2,
    ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF 		= ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF_2PASS          + 2,
    ARAD_KBP_FRWRD_LTR_IPV6_MC_RPF  		= ARAD_KBP_FRWRD_LTR_IPV6_UC_RPF 		+ 2,
    /* TRILL */
    ARAD_KBP_FRWRD_LTR_LSR          		= ARAD_KBP_FRWRD_LTR_IPV6_MC_RPF 		+ 2, 
    ARAD_KBP_FRWRD_LTR_TRILL_UC     		= ARAD_KBP_FRWRD_LTR_LSR                        + 2,
    ARAD_KBP_FRWRD_LTR_TRILL_MC     		= ARAD_KBP_FRWRD_LTR_TRILL_UC 			+ 2,
    /* Exclusive */
    ARAD_KBP_FRWRD_LTR_IPV4_DC              = ARAD_KBP_FRWRD_LTR_TRILL_MC 			+ 2,
    ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED 		= ARAD_KBP_FRWRD_LTR_IPV4_DC 			+ 2,
    ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_IP, 
    ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_IP_WITH_RPF,
    ARAD_KBP_FRWRD_LTR_IP_LSR_SHARED_FOR_LSR,
    ARAD_KBP_FRWRD_LTR_IP_LSR_EXTENDED_IPV6,
    ARAD_KBP_FRWRD_LTR_IP_LSR_EXTENDED_P2P,

    /* Dummy forwarding */
    ARAD_KBP_FRWRD_LTR_DUMMY_FRWRD,

    ARAD_KBP_MAX_FRWRD_LTR_ID,
    ARAD_KBP_ACL_LTR_ID_OFFSET      		= ARAD_KBP_MAX_FRWRD_LTR_ID,

    /* MAX must be lower than ARAD_KBP_ROP_LTR_NUM_MAX */
    ARAD_KBP_MAX_ACL_LTR_ID         = ARAD_KBP_LTR_ID_FIRST + 2*ARAD_KBP_MAX_NUM_OF_FRWRD_DBS,

} ARAD_KBP_FRWRD_IP_LTR;

typedef enum
{
    ARAD_KBP_FRWRD_TABLE_OPCODE_NONE          = 0, 

    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_RPF     = PROG_FLP_IPV4UC_RPF,         /*  3 */
    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_UC      = PROG_FLP_IPV4UC,             /*  4 */
    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_MC_COMP = PROG_FLP_IPV4COMPMC_WITH_RPF,/* 18 */

    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC      = PROG_FLP_IPV6UC,             /* 5 */
    ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_P2P = PROG_FLP_P2P,                /* 6 */
    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_MC_RPF  = PROG_FLP_IPV6MC,             /* 7 */

    ARAD_KBP_FRWRD_TABLE_OPCODE_LSR          = PROG_FLP_LSR,                /* 8 */
    ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_UC     = PROG_FLP_TRILL_UC,           /* 10 */
    ARAD_KBP_FRWRD_TABLE_OPCODE_TRILL_MC     = PROG_FLP_TRILL_MC_ONE_TAG,   /* 11 */

    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV4_DC      = PROG_FLP_IPV4_DC,            /* 19 */

    ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_IP           = 12,
    ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_IP_WITH_RPF  = 13,
    ARAD_KBP_FRWRD_TABLE_OPCODE_SHARED_IP_LSR_FOR_LSR          = 14,     
    ARAD_KBP_FRWRD_TABLE_OPCODE_EXTENDED_IPV6                  = 15,

    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF  = PROG_FLP_IPV6UC_RPF, /* 32, program is dynamically allocated */
    ARAD_KBP_FRWRD_TABLE_OPCODE_IPV6_UC_RPF_2PASS = PROG_FLP_IPV6UC_WITH_RPF_2PASS, /* 46, program is dynamically allocated */

} ARAD_KBP_FRWRD_IP_OPCODE;

typedef enum
{
    ARAD_KBP_MAX_AD_TABLE_SIZE_64,
    ARAD_KBP_MAX_AD_TABLE_SIZE_48,
    ARAD_KBP_MAX_AD_TABLE_SIZE_32,
    ARAD_KBP_MAX_AD_TABLE_SIZE_24,
    
    ARAD_KBP_MAX_NUM_OF_AD_TABLES,

} ARAD_KBP_AD_TABLE_INDEX;

typedef enum
{
    ARAD_KBP_DB_TYPE_ACL            = 0,
    ARAD_KBP_DB_TYPE_FORWARDING     = 1,

    ARAD_KBP_NOF_DB_TYPES,

} ARAD_KBP_DB_TYPE;

typedef enum alg_kbp_tbl_width
{
    NLM_TBL_WIDTH_80  = 80,
    NLM_TBL_WIDTH_160 = 160,
    NLM_TBL_WIDTH_320 = 320,
    NLM_TBL_WIDTH_640 = 640,

    NLM_TBL_WIDTH_END
}alg_kbp_tbl_width;

typedef enum alg_kbp_ad_width
{
    NLM_TBL_ADLEN_ZERO = 0,
    NLM_TBL_ADLEN_24B  = 24,
    NLM_TBL_ADLEN_32B  = 32,
    NLM_TBL_ADLEN_48B  = 48,
    NLM_TBL_ADLEN_64B  = 64,
    NLM_TBL_ADLEN_128B = 128,
    NLM_TBL_ADLEN_256B = 256,

    NLM_TBL_ADLEN_END

} alg_kbp_ad_width;

#define ARAD_KBP_AD_WIDTH_TYPE_TO_BITS(width)    \
     ((width == NLM_TBL_ADLEN_24B)  ? 24  : \
     ((width == NLM_TBL_ADLEN_32B)  ? 32  : \
     ((width == NLM_TBL_ADLEN_48B)  ? 48  : \
     ((width == NLM_TBL_ADLEN_64B)  ? 64  : \
     ((width == NLM_TBL_ADLEN_128B) ? 128 : \
     0)))))

#define ARAD_KBP_AD_WIDTH_TO_AD_TABLE_IDX(width)    \
     ((width == NLM_TBL_ADLEN_24B) ? ARAD_KBP_MAX_AD_TABLE_SIZE_24 : \
     ((width == NLM_TBL_ADLEN_32B) ? ARAD_KBP_MAX_AD_TABLE_SIZE_32 : \
     ((width == NLM_TBL_ADLEN_48B) ? ARAD_KBP_MAX_AD_TABLE_SIZE_48 : \
     ((width == NLM_TBL_ADLEN_64B) ? ARAD_KBP_MAX_AD_TABLE_SIZE_64 : \
     ARAD_KBP_MAX_NUM_OF_AD_TABLES))))

#define ARAD_KBP_AD_TABLE_IDX_TO_AD_WIDTH(_index)    \
     ((_index == ARAD_KBP_MAX_AD_TABLE_SIZE_24) ? NLM_TBL_ADLEN_24B : \
     ((_index == ARAD_KBP_MAX_AD_TABLE_SIZE_32) ? NLM_TBL_ADLEN_32B : \
     ((_index == ARAD_KBP_MAX_AD_TABLE_SIZE_48) ? NLM_TBL_ADLEN_48B : \
     ((_index == ARAD_KBP_MAX_AD_TABLE_SIZE_64) ? NLM_TBL_ADLEN_64B : \
     0))))

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

typedef kbp_instruction_t*      ARAD_KBP_INSTRUCTION;
typedef kbp_key_t*              ARAD_KBP_KEY;


/* Represents one segment in the master key */
typedef struct
{
    /* A logical name to be recognized with this segment */
    char name[20];

    /* Number of bytes to take */
    uint8 nof_bytes;

    /* A type (kbp_key_field_type)to be recognized with this segment */
    int type;

} ARAD_KBP_KEY_SEGMENT;

typedef struct
{
    int     nof_segments;
    ARAD_KBP_KEY_SEGMENT key_segment[ARAD_KBP_MAX_NOF_KEY_SEGMENTS];

}ARAD_KBP_DIAG_ENTRY_PARSING_TABLE;

/* Configuration information needed for one lookup table */
typedef struct 
{
    /* This table configuration is valid */
    uint8 valid;

    /* Table ID - each lookup table has a unique ID,
     * several tables can belong to the same LTR
     */ 
    nlm_u8 tbl_id;

    /* Table size - number of entries to allocate for
     * this table
     */ 
    uint32 tbl_size;

    /* Table Width - the width in bits of each line
     * in this table
     */
    alg_kbp_tbl_width tbl_width;

    /* Association Width - the association data is the
     * result of a lookup in htis table. The width represents
     * The number of bits the result line can contain
     */
    alg_kbp_ad_width tbl_asso_width;

    /* Group ID start - one table can contain several groups
     * of lines - for better management. The group ID start
     * is the lowest ID a group id this table can have.
     */
    uint16 group_id_start;

    /* Group ID End - one table can contain several groups
     * of lines - for better management. The group ID end
     * is the highest ID a group id this table can have.
     */
    uint16 group_id_end;

    /* Bank Number (0/1) - the bank associated with this table.
     * The bank ID is relevant when working in SMT mode, which
     * means that multithreading is supported with two threads, 
     * in which case the database arrays are divided into two 
     * partitions (banks). 
     */
    NlmBankNum bankNum;

    /* Min Priority - the minimal priority (logically, not numerically)
	 * value that shall be set for an entry in this table.
	 * -1 indicates no set value. In this case the entries in this table
	 * may be set with priorities from 0 to 2^22.
	 */
    int32 min_priority;

	/* entry_key_parsing - the keys the used for search in this table.
	 */
	ARAD_KBP_DIAG_ENTRY_PARSING_TABLE entry_key_parsing;

    /* Clone of Table ID - this table is a clone of tbl_id.
     * if ARAD_KBP_FRWRD_IP_NOF_TABLES then not a clone.
     */
    nlm_u8 clone_of_tbl_id;

} ARAD_KBP_TABLE_CONFIG;

/* Configuration information needed for one lookup table */
typedef struct 
{
    /* The number of key segments - the number of fields that
     * are looked up in the table.
     */
    uint32 nof_key_segments;

    /* Key Segment - one of several fields that are being searched
     * in the table. The key segment should correspond to the D/M
     * structure when adding a record.
     */
    ARAD_KBP_KEY_SEGMENT key_segment[ARAD_KBP_MAX_NOF_KEY_SEGMENTS];

} ARAD_KBP_LTR_SINGLE_SEARCH;

/* Configuration information needed for one lookup table */
typedef struct 
{
    /* This LTR configuration is valid */
    uint8 valid;

    /* The OPCODE - is the ID of this table's GTM in a lower level */
    uint8 opcode;

    /* LTR id - one LTR represents a "program" -
     * each packet is assigned with an LTR, and lookups 
     * are performed according to it. 
     * Several LTRs can search in the same table. 
     */
    ARAD_KBP_FRWRD_IP_LTR ltr_id;

    /* The number of parallel serahces - the number of searches
     * that can be performed at the same time, which include
     * search in this LTR.
     */
    uint8 parallel_srches_bmp;

    /* The Tables that should be searched - up to 4.
     */
    ARAD_KBP_FRWRD_IP_TBL_ID tbl_id[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES];

    /* For each lookup indicate the type of Database:
     * Forwarding or ACL.
     */
    ARAD_KBP_DB_TYPE search_type[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES];

    /* is cmp3 search - there are 3 types of compare (1,2,3),
     * which perform different number of searches in different 
     * lengths. The cmp3 performs up to 6 searches (in two clocks) 
     * and can pass 640 bits to context buffer. 
     */
    NlmBool is_cmp3_search;

    /* LTR - All the information required forthis search:
     * which fields of the master key to search and in 
     * which order.
     */
    ARAD_KBP_LTR_SINGLE_SEARCH ltr_search[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES];

    /* LTR - All the information required forthis search:
     * which fields of the master key to search and in
     * which order.
     */
    ARAD_KBP_LTR_SINGLE_SEARCH master_key_fields;


    /* An instruction represents a line in the LTR table.
     * All configuration for a specific LTR is performed on 
     * the relevant instruction. 
     */
    ARAD_KBP_INSTRUCTION inst_p;

    /* A pointer to the master key. 
     */
    ARAD_KBP_KEY master_key;

} ARAD_KBP_LTR_CONFIG;

typedef struct
{
    /* The size of the Data (-1) */
    uint32 tx_data_size;

    /* The Data type (usually INFO/REQUEST) */
    uint32 tx_data_type;

    /* The Reply size (-1) */
    uint32 rx_data_size;

    /* The Data Type (usually REPLY) */
    uint32 rx_data_type;

} ARAD_KBP_GTM_OPCODE_CONFIG_INFO;

typedef struct alg_kbp_rec
{
    uint8    *m_data; /* Data portion of the record */
    uint8    *m_mask; /* Mask portion of the record */
    uint16    m_len;  /* Record length in bits */
} alg_kbp_rec;

typedef struct tableRecordInfo
{
    alg_kbp_rec   record;
    uint16        groupId;
    uint16        priority;
    uint32        index;
    uint8         assoData[8]; /* max associcated data 64b*/
} tableRecordInfo;

typedef struct tableInfo
{
    uint8               tbl_id;
    alg_kbp_tbl_width   tbl_width;
    alg_kbp_ad_width    tbl_assoWidth;
    uint32              tbl_size;
    uint32              max_recCount;
    uint32              rec_count;
    uint16              groupId_start;
    uint16              groupId_end;
    tableRecordInfo     *tblRecordInfo_p;

    /*ARAD SPEC */
    struct kbp_db       *db_p;
    struct kbp_ad_db    *ad_db_p;
    struct kbp_key      *key;
    uint8               is_key_adde_to_db;
    ARAD_KBP_KEY_SEGMENT dummy_segment;
} tableInfo;

typedef struct globalGTMInfo_s
{
    tableInfo tblInfo;

    /* Parallel Search Attributes */
    uint8 ltr_num;

} globalGTMInfo;


typedef struct arad_kbp_rslt_s
{
    uint8   m_resultValid[NLM_MAX_NUM_RESULTS];
    uint8   m_hitOrMiss[NLM_MAX_NUM_RESULTS];
    uint8   m_respType[NLM_MAX_NUM_RESULTS];

    uint8   m_hitDevId[NLM_MAX_NUM_RESULTS];
    uint32  m_hitIndex[NLM_MAX_NUM_RESULTS];

    uint8   m_AssocData[NLM_MAX_NUM_RESULTS][NLMDEV_MAX_AD_LEN_IN_BYTES];

} arad_kbp_rslt;

typedef struct ad_info
{
    struct kbp_ad_db *ad_db_p;
    uint32 ad_table_size;

} ad_info;

typedef struct genericTblMgrAradAppData
{
    uint32  channel_id;
    uint32  request_queue_len;
    uint32  result_queue_len;

    globalGTMInfo g_gtmInfo[ARAD_KBP_MAX_NUM_OF_TABLES];
    
    NlmCmAllocator  alloc_bdy[SOC_DPP_DEFS_MAX(NOF_CORES)];
    NlmCmAllocator *alloc_p[SOC_DPP_DEFS_MAX(NOF_CORES)];

    /*ALG KBP SPEC*/
    struct kbp_allocator *dalloc_p[SOC_DPP_DEFS_MAX(NOF_CORES)];
    void *alg_kbp_xpt_p[SOC_DPP_DEFS_MAX(NOF_CORES)];
    struct kbp_device *device_p[SOC_DPP_DEFS_MAX(NOF_CORES)];

} genericTblMgrAradAppData;


typedef struct arad_kbp_frwd_ltr_db_s
{
    uint32 opcode;
    uint32 res_data_len[NLM_ARAD_MAX_NUM_RESULTS_PER_INST];
    NlmAradCompareResponseFormat res_format[NLM_ARAD_MAX_NUM_RESULTS_PER_INST];
    uint32 res_total_data_len;
} arad_kbp_frwd_ltr_db_t;

/* Table handles for WB purposes */
typedef kbp_db_t*  kbp_db_handle;
typedef kbp_ad_db_t* kbp_ad_handle;
typedef struct 
{
    kbp_db_handle       db_p;
    kbp_ad_handle       ad_db_p;
    uint8               is_valid;
    uint32              table_size;
    uint32              table_id;
    uint32              table_width;
    uint32              table_asso_width;

} ARAD_KBP_DB_HANDLES;

/* Table handles for WB purposes */
typedef struct 
{
    ARAD_KBP_INSTRUCTION inst_p;

} ARAD_KBP_LTR_HANDLES;



/*kbp reset function. 
  A value of one means they are asserted low 0V, zero means de-asserted.
  Function returns zero on success or treated as an error
  */
typedef  int32_t (*kbp_reset_f)(void *handle, int32_t s_reset_low, int32_t c_reset_low);

/* kbp device warmboot info */
typedef struct kbp_warmboot_s{
    FILE *kbp_file_fp;
    kbp_device_issu_read_fn  kbp_file_read;
    kbp_device_issu_write_fn kbp_file_write;
}kbp_warmboot_t;

typedef struct {
    ARAD_KBP_DB_HANDLES db_info[ARAD_KBP_MAX_NUM_OF_TABLES];
    ARAD_KBP_LTR_HANDLES ltr_info[ARAD_KBP_MAX_NUM_OF_FRWRD_DBS];


    ARAD_KBP_TABLE_CONFIG           Arad_kbp_table_config_info[ARAD_KBP_MAX_NUM_OF_TABLES];
    ARAD_KBP_LTR_CONFIG             Arad_kbp_ltr_config[ARAD_KBP_MAX_NUM_OF_FRWRD_DBS];
    arad_kbp_lut_data_t             Arad_kbp_gtm_lut_info[ARAD_KBP_MAX_NUM_OF_FRWRD_DBS];
    arad_kbp_frwd_ltr_db_t          Arad_kbp_gtm_ltr_info[ARAD_KBP_MAX_NUM_OF_FRWRD_DBS];
    ARAD_KBP_GTM_OPCODE_CONFIG_INFO Arad_kbp_gtm_opcode_config_info[ARAD_KBP_MAX_NUM_OF_FRWRD_DBS];

	/* dynamic KBP result size info */
    uint8                                 Arad_pmf_ce_dynamic_kbp_qualifiers_enabled;
    ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES Arad_pmf_ce_dynamic_kbp_qualifiers_values[ARAD_KBP_MAX_NUM_OF_PARALLEL_SEARCHES];

} ARAD_KBP_INFO;

typedef struct kbp_init_user_data_s{
    int device;
    uint32 kbp_mdio_id;
}kbp_init_user_data_t;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */
    
arad_kbp_frwd_ltr_db_t 
    arad_kbp_frwd_ltr_db[ARAD_KBP_ROP_LTR_NUM_MAX];

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

/* 
 * Test Functions
 */
uint32 arad_kbp_test_ip4_rpf_NlmGenericTableManager(
    int unit,
    uint32 num_entries,
    uint32 record_base_tbl[4],
    uint32 ad_val_tbl[4]);

/*get the database pointer*/
uint32 arad_kbp_alg_kbp_db_get(
   int unit,
   uint32 frwrd_tbl_id,
   struct kbp_db **db_p);

/* GTM table info get */
uint32 arad_kbp_gtm_table_info_get(
   int unit,
   uint32 frwrd_tbl_id,
   tableInfo *tblInfo_p);

uint32 arad_kbp_alg_kbp_ad_db_get(
   int unit,
   uint32 frwrd_tbl_id,
   struct kbp_ad_db **db_p);

/* GTM LTR number get */
uint32 arad_kbp_gtm_ltr_num_get(
   int unit,
   uint32 frwrd_tbl_id,
   uint8 *ltr_num);

uint32 
    arad_kbp_get_device_pointer(
        SOC_SAND_IN  int                unit,
        SOC_SAND_OUT struct kbp_device  **device_p
    );
uint32 
    arad_kbp_ltr_get_inst_pointer(
        SOC_SAND_IN  int                     	unit,
        SOC_SAND_IN  uint32      				ltr_idx,
        SOC_SAND_OUT struct  kbp_instruction    **inst_p
    );
uint32 
    arad_kbp_opcode_to_db_type(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint8                       opcode,
       SOC_SAND_OUT uint32                      *db_type,
       SOC_SAND_OUT uint32                      *ltr_id,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
   );

/* Init functions */
uint32 arad_kbp_init_kbp_interface( 
    int unit,
    uint32 core,
    uint32 kbp_mdio_id,
    uint32 kbp_ilkn_rev,
    kbp_reset_f kbp_reset);

uint32 arad_kbp_init_arad_interface( 
    int unit);

uint32 arad_kbp_init_app(
    int unit,
    uint32 second_kbp_supported,
    ARAD_INIT_ELK *elk);

uint32 arad_kbp_deinit_app(
    int unit,
    uint32 second_kbp_supported);

/* 
 *  KBP db (SW) management functions
 */
void
  ARAD_KBP_GTM_LUT_clear(
    SOC_SAND_OUT arad_kbp_lut_data_t *info
  );
void
  arad_kbp_frwd_ltr_db_clear(
    SOC_SAND_OUT arad_kbp_frwd_ltr_db_t *info
  );
void
  ARAD_KBP_GTM_OPCODE_CONFIG_INFO_clear(
    SOC_SAND_OUT ARAD_KBP_GTM_OPCODE_CONFIG_INFO *info
  );
void
  ARAD_KBP_LTR_CONFIG_clear(
    SOC_SAND_OUT ARAD_KBP_LTR_CONFIG *info
  );
void
  ARAD_KBP_LTR_SINGLE_SEARCH_clear(
    SOC_SAND_OUT ARAD_KBP_LTR_SINGLE_SEARCH *info
  );
void
  ARAD_KBP_TABLE_CONFIG_clear(
    SOC_SAND_OUT ARAD_KBP_TABLE_CONFIG *info
  );
void
  ARAD_KBP_KEY_SEGMENT_clear(
    SOC_SAND_OUT ARAD_KBP_KEY_SEGMENT *info
  );


void
  arad_kbp_frwd_lut_db_print(
    SOC_SAND_IN  arad_kbp_lut_data_t *info
  );
void
  arad_kbp_frwd_ltr_db_print(
    SOC_SAND_IN  arad_kbp_frwd_ltr_db_t *info,
    SOC_SAND_IN  int advanced
  );
void
  ARAD_KBP_GTM_OPCODE_CONFIG_INFO_print(
    SOC_SAND_IN  ARAD_KBP_GTM_OPCODE_CONFIG_INFO *info
  );
void
  ARAD_KBP_LTR_CONFIG_print(
    SOC_SAND_IN  ARAD_KBP_LTR_CONFIG *info,
    SOC_SAND_IN  int advanced
  );
void
  ARAD_KBP_LTR_SINGLE_SEARCH_print(
    SOC_SAND_IN  ARAD_KBP_LTR_SINGLE_SEARCH *info
  );
void
  ARAD_KBP_TABLE_CONFIG_print(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  ARAD_KBP_TABLE_CONFIG *info
  );
void
  ARAD_KBP_KEY_SEGMENT_print(
    SOC_SAND_IN  ARAD_KBP_KEY_SEGMENT *info
  );

const char*
  NlmAradCompareResponseFormat_to_string(
    SOC_SAND_IN  NlmAradCompareResponseFormat enum_val
  );
const char*
  ARAD_KBP_FRWRD_IP_DB_TYPE_to_string(
    SOC_SAND_IN  ARAD_KBP_FRWRD_IP_DB_TYPE enum_val
  );

const char*
  ARAD_KBP_DB_TYPE_to_string(
    SOC_SAND_IN  ARAD_KBP_DB_TYPE enum_val
  );

uint32
    arad_kbp_result_sizes_configurations_init(int unit, ARAD_KBP_FRWRD_IP_DB_TYPE db_type, int results[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS]);

uint32
  arad_kbp_update_master_key(	
	SOC_SAND_IN int						unit,
	SOC_SAND_IN int						prog_id,
	SOC_SAND_IN SOC_DPP_DBAL_QUAL_INFO	qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
	SOC_SAND_IN int						nof_qualifiers);

uint32
  arad_kbp_update_lookup(
	SOC_SAND_IN int						unit,
	SOC_SAND_IN int 					table_id,
	SOC_SAND_IN int						prog_id,
	SOC_SAND_IN int 					search_id,
	SOC_SAND_IN SOC_DPP_DBAL_QUAL_INFO	qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
	SOC_SAND_IN int						nof_qualifiers);

uint32
    arad_kbp_sw_init(
       SOC_SAND_IN  int  unit
    );

uint32 
    arad_kbp_add_acl(
       SOC_SAND_IN  int                      unit,
       SOC_SAND_IN  uint32                      table_id,
       SOC_SAND_IN  uint32                      search_id,
       SOC_SAND_IN  uint32                      pgm_bmp_used,
       SOC_SAND_IN  uint32                      pgm_ndx_min,
       SOC_SAND_IN  uint32                      pgm_ndx_max,
       SOC_SAND_IN  uint32                      key_size_in_bits,   
       SOC_SAND_IN  uint32                      flags,
       SOC_SAND_IN  uint32                      min_priority,   
       uint32                                   shared_quals,   
       ARAD_PP_FP_SHARED_QUAL_INFO              shared_quals_info[MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM],
       SOC_SAND_OUT uint32                      *pgm_bmp_new,
       SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE    *success
    );

/*
 *    ELK CPU/ROP access functions
 */
uint32 arad_kbp_cpu_ctxb_write_cmp(
    int unit,
    uint8 opcode,
    soc_reg_above_64_val_t data);

/* Get table size in bytes */
uint32
  arad_kbp_table_size_get(
    SOC_SAND_IN int                   unit, 
    SOC_SAND_IN ARAD_KBP_FRWRD_IP_TBL_ID table_id,
    SOC_SAND_OUT uint32                  *table_size_in_bytes,
    SOC_SAND_OUT uint32                  *table_payload_in_bytes
  );

/* Get static configuration of table size in bytes - only for static Forwarding tables
 */
uint32
  arad_kbp_static_table_size_get(
    SOC_SAND_IN int                   unit, 
    SOC_SAND_IN ARAD_KBP_FRWRD_IP_TBL_ID table_id,
    SOC_SAND_OUT uint32                  *table_size_in_bytes,
    SOC_SAND_OUT uint32                  *table_payload_in_bytes
  );


/* Get LTR-ID per table id */
uint32
  arad_kbp_table_ltr_id_get(
    SOC_SAND_IN int                   unit, 
    SOC_SAND_IN ARAD_KBP_FRWRD_IP_TBL_ID table_id,
    SOC_SAND_OUT ARAD_KBP_FRWRD_IP_LTR   *ltr_id
  );

/* retrieves ILKN parameters: port, num of lanes, rate, and metaframe */
uint32 arad_kbp_ilkn_interface_param_get( 
    int         unit,
    uint32      core,
    soc_port_t *ilkn_port,
    uint32     *ilkn_num_lanes,
    int        *ilkn_rate,
    uint32     *ilkn_metaframe);

/* given the flp program, return the adequate ARAD_KBP_LTR_CONFIG struct from the global array Arad_kbp_ltr_config */
uint32
    arad_kbp_ltr_config_get(
        SOC_SAND_IN  int unit,
        SOC_SAND_IN  uint32 flp_program,
        SOC_SAND_OUT ARAD_KBP_LTR_CONFIG *config
    );

#ifdef CRASH_RECOVERY_SUPPORT
/* set the start/end command for CR transaction */
uint32 arad_kbp_cr_transaction_cmd(int unit,
                                   uint8 is_start);
uint8 arad_kbp_cr_query_restore_status(int unit);
uint32 arad_kbp_cr_clear_restore_status(int unit);
uint32 arad_kbp_cr_db_commit(int unit,
                             uint32 tbl_id);
#endif
/* commit all cached KBP configuration */
uint32 arad_kbp_db_commit(
   int unit);

/* enable/disable autosync for KBP SW state */
int arad_kbp_autosync_set(int unit, int enable);

/* sync KBP SW state */
int arad_kbp_sync(int unit);

/* register file and read/write functions for KBP warmboot usage*/
void 
  arad_kbp_warmboot_register(int unit,
                             FILE *file_fp,
                             kbp_device_issu_read_fn read_fn, 
                             kbp_device_issu_write_fn write_fn);

void 
    arad_kbp_sw_config_print(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  int  advanced
    );

void
    arad_kbp_device_print(
       SOC_SAND_IN  int  unit,
       SOC_SAND_IN  char*   file_name
    );

int
  arad_kbp_print_diag_entry_added(
     SOC_SAND_IN	int unit,
     SOC_SAND_IN	tableInfo *tbl_info,
     SOC_SAND_IN	uint8     *data,
     SOC_SAND_IN	uint32    prefix_len,
	 SOC_SAND_IN    uint8     *mask,
     SOC_SAND_IN	uint8     *ad_data);

int
  arad_kbp_print_diag_all_entries(int unit, const char *print_table);

int
  arad_kbp_print_diag_last_packet(int unit, int core, int flp_program);

int
  arad_kbp_print_search_result(int unit, uint32 ltr_idx, uint8 *master_key_buffer, struct kbp_search_result search_rslt);

int32_t
  arad_kbp_callback_mdio_write(void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t value);

int32_t
  arad_kbp_callback_mdio_read(void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t *value);

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __ARAD_KBP_INCLUDED__ */

#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) !defined(__ARAD_KBP_INCLUDED__) */
