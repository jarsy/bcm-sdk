/* $Id: jer_pp_kaps_xpt.c, hagayco Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

#include <soc/mem.h>

#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FORWARD
#include <soc/mem.h>

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/ARAD/arad_general.h>
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>

#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_xpt.h>

#include <soc/dpp/QAX/QAX_PP/qax_pp_kaps_xpt.h>

#include <soc/dpp/JER/JER_PP/jer_pp_kaps_entry_mgmt.h>

#include <soc/dpp/SAND/Management/sand_low_level.h>

#include <soc/dpp/JER/jer_sbusdma_desc.h>

#include <soc/dpp/JER/JER_PP/jer_pp_kaps_arm_image.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/*#define JER_KAPS_XPT_PRINTS_ENABLED*/
/*#define JER_KAPS_XPT_REG_PRINTS_ENABLED*/

#define JER_KAPS_XPT_EXTENDED_FUNC_ENUM     KAPS_FUNC14
#define JER_KAPS_XPT_RPB_FUNC_KAPS_SEARCH   KAPS_FUNC3

/* Block ID info */
#define JER_KAPS_XPT_BLOCK_OFFSET_RPB   1
#define JER_KAPS_XPT_BLOCK_OFFSET_BB    5
#define JER_KAPS_XPT_BLOCK_OFFSET_BRR   37
#define JER_KAPS_XPT_NOF_BLOCKS         40

/* Response values */
#define JER_KAPS_XPT_RESPONSE_VALID             3
#define JER_KAPS_XPT_RESPONSE_EXTENDED_VALID    2
#define JER_KAPS_RESPONSE_STATUS_AOK            0

/*Equal to TCMB. Aligned on 1KB boundary.
 *There is a single command FIFO with 4K-1 entries, each 64bit wide.
 */
#define JER_KAPS_CONFIG_CFIFO_BASE                0x80
#define JER_JER_KAPS_CONFIG_CFIFO_NUM_ENTRIES     0xFFF
#define JER_KAPS_CONFIG_CFIFO_ENTRY_SIZE          0x0

#define JER_KAPS_CONFIG_RFIFO_BASE                0xc0
#define JER_JER_KAPS_CONFIG_RFIFO_NUM_ENTRIES     JER_JER_KAPS_CONFIG_CFIFO_NUM_ENTRIES
#define JER_KAPS_CONFIG_RFIFO_ENTRY_SIZE          JER_KAPS_CONFIG_CFIFO_ENTRY_SIZE

/*Convert from the 1KB alignment to 8bit alignment*/
#define JER_KAPS_CONFIG_CFIFO_BASE_8         1024*JER_KAPS_CONFIG_CFIFO_BASE

/*Calculate the max FIFO address in bytes.
 *Base address + entry size = 2**JER_KAPS_CONFIG_CFIFO_ENTRY_SIZE * 8 (64 bits to bytes) * (number of entries).
  */
#define JER_KAPS_CONFIG_CFIFO_MAX_ADD_8      (JER_KAPS_CONFIG_CFIFO_BASE_8 + 8*(JER_JER_KAPS_CONFIG_CFIFO_NUM_ENTRIES))

#define NOF_UINT8_IN_UINT64              8

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
typedef enum
{
    DATA_0,
    DATA_1,
    DATA_2,
    DATA_3,
    DATA_4,
    DATA_5,
    DATA_6,
    DATA_7,
    DATA_8,
    DATA_9,
    DATA_10,
    DATA_11,
    DATA_12,
    DATA_13,
    DATA_14,

    DATA_LAST


}JER_KAPS_DATA_REGS;

typedef struct
{
    soc_reg_t reg;
    uint8 index;
}KAPS_REG_ACCESS_INFO;

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

KAPS_REG_ACCESS_INFO offset_to_register_access_info[] = 
{
    /* register                               index       offset */   
    /*-----------------------------------------------------------*/      
    {KAPS_REVISIONr,                            0   },  /* 0x0 */
    {KAPS_CONFIGr,                              0   },  /* 0x1 */
    {KAPS_IBC_CONFIGr,                          0   },  /* 0x2 */
    {KAPS_GLOBAL_EVENTr,                        0   },  /* 0x3 */
    {KAPS_GLOBAL_EVENT_MASKr,                   0   },  /* 0x4 */
    {KAPS_R_5_FIFO_MON_EVENTr,                  0   },  /* 0x5 */
    {KAPS_R_5_FIFO_MON_EVENT_MASKr,             0   },  /* 0x6 */
    {KAPS_R_5_ATTNr,                            0   },  /* 0x7 */
    {KAPS_R_5_ATTNr,                            1   },  /* 0x8 */
    {KAPS_R_5_ATTNr,                            2   },  /* 0x9 */
    {KAPS_R_5_ATTNr,                            3   },  /* 0xa */
    {KAPS_R_5_COUNTERSr,                        0   },  /* 0xb */
    {KAPS_R_5_COUNTERSr,                        1   },  /* 0xc */
    {KAPS_R_5_COUNTERSr,                        2   },  /* 0xd */
    {KAPS_R_5_COUNTERSr,                        3   },  /* 0xe */
    {KAPS_R_5_COUNTERSr,                        4   },  /* 0xf */
    {KAPS_R_5_COUNTERSr,                        5   },  /* 0x10 */
    {KAPS_R_5_COUNTERSr,                        6   },  /* 0x11 */
    {KAPS_R_5_COUNTERSr,                        7   },  /* 0x12 */
    {KAPS_R_5_COUNTERSr,                        8   },  /* 0x13 */
    {KAPS_R_5_COUNTERSr,                        9   },  /* 0x14 */
    {KAPS_R_5_COUNTERSr,                        10  },  /* 0x15 */
    {KAPS_R_5_COUNTERSr,                        11  },  /* 0x16 */
    {KAPS_R_5_COUNTERSr,                        12  },  /* 0x17 */
    {KAPS_R_5_COUNTERSr,                        13  },  /* 0x18 */
    {KAPS_R_5_COUNTERSr,                        14  },  /* 0x19 */
    {KAPS_R_5_COUNTERSr,                        15  },  /* 0x1a */
    {KAPS_MEMORY_A_ERRORr,                      0   },  /* 0x1b */
    {KAPS_MEMORY_B_ERRORr,                      0   },  /* 0x1c */
    {KAPS_SEARCH_0A_ERRORr,                     0   },  /* 0x1d */
    {KAPS_MEMORY_A_ERROR_MASKr,                 0   },  /* 0x1e */
    {KAPS_MEMORY_B_ERROR_MASKr,                 0   },  /* 0x1f */
    {KAPS_SEARCH_0A_ERROR_MASKr,                0   },  /* 0x20 */
    {KAPS_SEARCH_1A_ERRORr,                     0   },  /* 0x21 */
    {KAPS_SEARCH_1A_ERROR_MASKr,                0   },  /* 0x22 */
    {KAPS_SEARCH_0B_ERRORr,                     0   },  /* 0x23 */
    {KAPS_SEARCH_0B_ERROR_MASKr,                0   },  /* 0x24 */
    {KAPS_SEARCH_1B_ERRORr,                     0   },  /* 0x25 */
    {KAPS_SEARCH_1B_ERROR_MASKr,                0   },  /* 0x26 */
    {KAPS_CONFIG_CFIFOr,                        0   },  /* 0x27 */
    {KAPS_CONFIG_CFIFOr,                        1   },  /* 0x28 */
    {KAPS_CONFIG_CFIFOr,                        2   },  /* 0x29 */
    {KAPS_CONFIG_CFIFOr,                        3   },  /* 0x2a */
    {KAPS_CONFIG_CFIFO_THRESHr,                 0   },  /* 0x2b */
    {KAPS_CONFIG_CFIFO_THRESHr,                 1   },  /* 0x2c */
    {KAPS_CONFIG_CFIFO_THRESHr,                 2   },  /* 0x2d */
    {KAPS_CONFIG_CFIFO_THRESHr,                 3   },  /* 0x2e */
    {KAPS_STATUS_CFIFOr,                        0   },  /* 0x2f */
    {KAPS_STATUS_CFIFOr,                        1   },  /* 0x30 */
    {KAPS_STATUS_CFIFOr,                        2   },  /* 0x31 */
    {KAPS_STATUS_CFIFOr,                        3   },  /* 0x32 */
    {KAPS_CONFIG_RFIFOr,                        0   },  /* 0x33 */
    {KAPS_CONFIG_RFIFOr,                        1   },  /* 0x34 */
    {KAPS_CONFIG_RFIFOr,                        2   },  /* 0x35 */
    {KAPS_CONFIG_RFIFOr,                        3   },  /* 0x36 */
    {KAPS_STATUS_RFIFOr,                        0   },  /* 0x37 */
    {KAPS_STATUS_RFIFOr,                        1   },  /* 0x38 */
    {KAPS_STATUS_RFIFOr,                        2   },  /* 0x39 */
    {KAPS_STATUS_RFIFOr,                        3   },  /* 0x3a */
    {KAPS_CONFIG_FIFO_MONr,                     0   },  /* 0x3b */
    {0,                                         0   },  /* 0x3c */ 
    {0,                                         0   },  /* 0x3d */ 
    {0,                                         0   },  /* 0x3e */ 
    {0,                                         0   },  /* 0x3f */                                                 
    {KAPS_IBC_COMMAND_DATAr,                    0   },  /* 0x40 */
    {KAPS_IBC_COMMAND_DATAr,                    1   },  /* 0x41 */
    {KAPS_IBC_COMMAND_DATAr,                    2   },  /* 0x42 */
    {KAPS_IBC_COMMAND_DATAr,                    3   },  /* 0x43 */
    {KAPS_IBC_COMMAND_DATAr,                    4   },  /* 0x44 */
    {KAPS_IBC_COMMAND_DATAr,                    5   },  /* 0x45 */
    {KAPS_IBC_COMMAND_DATAr,                    6   },  /* 0x46 */
    {KAPS_IBC_COMMAND_DATAr,                    7   },  /* 0x47 */
    {KAPS_IBC_COMMAND_DATAr,                    8   },  /* 0x48 */
    {KAPS_IBC_COMMAND_DATAr,                    9   },  /* 0x49 */
    {KAPS_IBC_COMMAND_DATAr,                    10  },  /* 0x4a */
    {KAPS_IBC_COMMAND_DATAr,                    11  },  /* 0x4b */
    {KAPS_IBC_COMMAND_DATAr,                    12  },  /* 0x4c */
    {KAPS_IBC_COMMAND_DATAr,                    13  },  /* 0x4d */
    {KAPS_IBC_COMMAND_DATAr,                    14  },  /* 0x4e */
    {KAPS_IBC_COMMAND_WORDr,                    0   },  /* 0x4f */
    {KAPS_IBC_RESPONSE_DATAr,                   0   },  /* 0x50 */
    {KAPS_IBC_RESPONSE_DATAr,                   1   },  /* 0x51 */
    {KAPS_IBC_RESPONSE_DATAr,                   2   },  /* 0x52 */
    {KAPS_IBC_RESPONSE_DATAr,                   3   },  /* 0x53 */
    {KAPS_IBC_RESPONSE_DATAr,                   4   },  /* 0x54 */
    {KAPS_IBC_RESPONSE_DATAr,                   5   },  /* 0x55 */
    {KAPS_IBC_RESPONSE_DATAr,                   6   },  /* 0x56 */
    {KAPS_IBC_RESPONSE_DATAr,                   7   },  /* 0x57 */
    {KAPS_IBC_RESPONSE_DATAr,                   8   },  /* 0x58 */
    {KAPS_IBC_RESPONSE_DATAr,                   9   },  /* 0x59 */
    {KAPS_IBC_RESPONSE_DATAr,                   10  },  /* 0x5a */
    {KAPS_IBC_RESPONSE_DATAr,                   11  },  /* 0x5b */
    {KAPS_IBC_RESPONSE_DATAr,                   12  },  /* 0x5c */
    {KAPS_IBC_RESPONSE_DATAr,                   13  },  /* 0x5d */
    {KAPS_IBC_RESPONSE_DATAr,                   14  },  /* 0x5e */
    {KAPS_IBC_RESPONSE_WORDr,                   0   },  /* 0x5f */
    {KAPS_IBC_QUEUE_STATUSr,                    0   },  /* 0x60 */
    {KAPS_IBC_FIFO_MEM_CONTROLr,                0   },  /* 0x61 */
    {KAPS_IBC_FIFO_ECC_DEBUGr,                  0   },  /* 0x62 */
    {KAPS_IBC_FIFO_CORRECTABLE_STATUSr,         0   },  /* 0x63 */
    {KAPS_IBC_FIFO_UNCORRECTABLE_STATUSr,       0   },  /* 0x64 */
    {KAPS_R_5_CORE_CONTROLr,                    0   },  /* 0x65 */
    {KAPS_R_5_CORE_MEM_CONTROLr,                0   },  /* 0x66 */
    {KAPS_R_5_ECC_ERR_WR_TCM_Ar,                0   },  /* 0x67 */
    {KAPS_R_5_ECC_ERR_WR_TCM_B_0r,              0   },  /* 0x68 */
    {KAPS_R_5_ECC_ERR_WR_TCM_B_1r,              0   },  /* 0x69 */
    {KAPS_R_5_ECC_ERR_RD_TCM_Ar,                0   },  /* 0x6a */
    {KAPS_R_5_ECC_ERR_RD_TCM_B_0r,              0   },  /* 0x6b */
    {KAPS_R_5_ECC_ERR_RD_TCM_B_1r,              0   },  /* 0x6c */
    {KAPS_R_5_CORE_CONFIGAr,                    0   },  /* 0x6d */
    {KAPS_R_5_CORE_CONFIGBr,                    0   },  /* 0x6e */
    {KAPS_R_5_CORE_CONFIGCr,                    0   },  /* 0x6f */
    {KAPS_R_5_CORE_CONFIGDr,                    0   },  /* 0x70 */
    {KAPS_R_5_CORE_CONFIGEr,                    0   },  /* 0x71 */
    {KAPS_R_5_AXI_CONFIGr,                      0   },  /* 0x72 */
    {KAPS_R_5_AXI_ERROR_STATUS_0r,              0   },  /* 0x73 */
    {KAPS_R_5_AXI_ERROR_STATUS_1r,              0   },  /* 0x74 */
    {KAPS_R_5_DEBUG_CONFIGAr,                   0   },  /* 0x75 */
    {KAPS_R_5_DEBUG_CONFIGBr,                   0   },  /* 0x76 */
    {KAPS_R_5_CONFIGAr,                         0   },  /* 0x77 */
    {KAPS_R_5_DEBUG_STATUSr,                    0   },  /* 0x78 */
    {KAPS_R_5_DEBUG_STATUS_EVENT_BUS_0r,        0   },  /* 0x79 */
    {KAPS_R_5_DEBUG_STATUS_EVENT_BUS_0_MASKr,   0   },  /* 0x7a */
    {KAPS_R_5_DEBUG_STATUS_EVENT_BUS_1r,        0   },  /* 0x7b */
    {KAPS_R_5_DEBUG_STATUS_EVENT_BUS_1_MASKr,   0   },  /* 0x7c */
    {KAPS_R_5_DEBUG_EBUS_PATTERN_0r,            0   },  /* 0x7d */
    {KAPS_R_5_DEBUG_EBUS_PATTERN_1r,            0   },  /* 0x7e */
    {KAPS_R_5_DEBUG_EBUS_PATTERN_0_MASKr,       0   },  /* 0x7f */
    {KAPS_R_5_DEBUG_EBUS_PATTERN_1_MASKr,       0   },  /* 0x80 */
    {KAPS_R_5_DAP_APB_CTRLr,                    0   },  /* 0x81 */
    {KAPS_R_5_DAP_APB_ADDRr,                    0   },  /* 0x82 */
    {KAPS_R_5_DAP_APB_WDATAr,                   0   },  /* 0x83 */
    {KAPS_R_5_DAP_APB_RDATAr,                   0   },  /* 0x84 */
    {KAPS_GLOBAL_DEBUGr,                        0   },  /* 0x85 */
    {KAPS_R_5_TM_TEST_CTRLr,                    0   },  /* 0x86 */
    {KAPS_IBC_FIFO_TM_TEST_CTRLr,               0   },  /* 0x87 */
    {KAPS_PD_ASSIST_DEBUGr,                     0   }   /* 0x88 */  
};

STATIC uint32 kaps_dma_enabled[SOC_SAND_MAX_DEVICE];

/*The kaps_tcm_write_p index is in bytes, points to an address in KAPS_TCM*/
STATIC uint32 kaps_tcm_write_p[SOC_SAND_MAX_DEVICE];
/*This is the pointer to the last memory the R5 read,
 *it needs to be updated each time we reach it with our write pointer.
 */
STATIC uint32 kaps_tcm_read_p[SOC_SAND_MAX_DEVICE];

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */

STATIC uint32 jer_pp_kaps_command_word_poll( int unit,
                                             uint32 expected_reg_val)
{
    int rv;
    uint32 reg_val;
    soc_timeout_t to;

    SOCDNX_INIT_FUNC_DEFS;

    if (!SAL_BOOT_PLISIM) {

        /* polling command word to validate it's ready */
        soc_timeout_init(&to, ARAD_TIMEOUT, ARAD_MIN_POLLS);
        for(;;){
            /* get command word register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
            {
                soc_reg_above_64_val_t above_64_val;
                above_64_val[0] = 0;
                above_64_val[1] = 0;
                rv = ARAD_FAST_REGISER_GET(ARAD_FAST_REG_IHP_KAPS_IBC_COMMAND_WORD, above_64_val);
                reg_val = above_64_val[0];
            }
#else
            rv = READ_KAPS_IBC_COMMAND_WORDr(unit, &reg_val);
#endif
            SOCDNX_IF_ERR_EXIT(rv);
            /* validate against expected value */        
            if(reg_val == expected_reg_val) {
                break;
            }

            if (soc_timeout_check(&to)) {
                SOCDNX_IF_ERR_EXIT_MSG(SOC_SAND_GEN_ERR, (_BSL_SOCDNX_MSG("Command word register not ready.")));
                break;
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC uint32 jer_pp_kaps_response_word_poll(int unit,
                                             uint32 blk_id,
                                             uint32 rsp,
                                             uint32 func,
                                             uint32 status)
{
    int rv;
    uint32 reg_val, field_val, expected_reg_val = 0;
    soc_timeout_t to;

    SOCDNX_INIT_FUNC_DEFS;

    if (!SAL_BOOT_PLISIM) {
        /* 
         * build expected register value
         */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
        field_val = blk_id;
        ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_RESPONSE_WORD_BLKID, &expected_reg_val, &field_val);

        field_val = rsp;
        ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_RESPONSE_WORD_RSP, &expected_reg_val, &field_val);

        field_val = func;
        ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_RESPONSE_WORD_FUNC, &expected_reg_val, &field_val);

        field_val = status;
        ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_RESPONSE_WORD_STATUS, &expected_reg_val, &field_val);
#else
        field_val = blk_id;
        soc_reg_field_set(unit, KAPS_IBC_RESPONSE_WORDr, &expected_reg_val, BLKIDf, field_val);

        field_val = rsp;
        soc_reg_field_set(unit, KAPS_IBC_RESPONSE_WORDr, &expected_reg_val, RSPf, field_val);

        field_val = func;
        soc_reg_field_set(unit, KAPS_IBC_RESPONSE_WORDr, &expected_reg_val, FUNCf, field_val);

        field_val = status;
        soc_reg_field_set(unit, KAPS_IBC_RESPONSE_WORDr, &expected_reg_val, STATUSf, field_val);
#endif /*ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS*/
        /* polling response word  */
        soc_timeout_init(&to, ARAD_TIMEOUT, ARAD_MIN_POLLS);
        for(;;){
            /* get response word register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
            {
                soc_reg_above_64_val_t above_64_val;
                above_64_val[0] = 0;
                above_64_val[1] = 0;
                rv = ARAD_FAST_REGISER_GET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_WORD, above_64_val);
                reg_val = above_64_val[0];
            }
#else
            rv = READ_KAPS_IBC_RESPONSE_WORDr(unit, &reg_val);
#endif
            SOCDNX_IF_ERR_EXIT(rv);
      
            /* validate blk_id, rsp, func, status */      
            if(reg_val == expected_reg_val) {
                break;
            }

            if (soc_timeout_check(&to)) {
                SOCDNX_IF_ERR_EXIT_MSG(SOC_SAND_GEN_ERR, (_BSL_SOCDNX_MSG("Response word register not ready.")));
                break;
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC uint32 jer_pp_kaps_command_word_write(int unit,
                                             uint32 blk_id,
                                             uint32 cmd,
                                             uint32 func,
                                             uint32 offset)
{
    int rv, res = KBP_OK;
    uint32 field_val, reg_val;

    /*
     * build register value
     */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    field_val = blk_id;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_BLKID, &reg_val, &field_val);

    field_val = cmd;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_CMD, &reg_val, &field_val);

    field_val = func;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_FUNC, &reg_val, &field_val);

    field_val = offset;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_OFFSET, &reg_val, &field_val);

    /* set command word register */
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = reg_val;
        above_64_val[1] = 0;
        rv = ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_COMMAND_WORD, above_64_val);
    }
#else
    field_val = blk_id;
    soc_reg_field_set(unit, KAPS_IBC_COMMAND_WORDr, &reg_val, BLKIDf, field_val);

    field_val = cmd;
    soc_reg_field_set(unit, KAPS_IBC_COMMAND_WORDr, &reg_val, CMDf, field_val);

    field_val = func;
    soc_reg_field_set(unit, KAPS_IBC_COMMAND_WORDr, &reg_val, FUNCf, field_val);

    field_val = offset;
    soc_reg_field_set(unit, KAPS_IBC_COMMAND_WORDr, &reg_val, OFFSETf, field_val);

    /* set command word register */
    rv = WRITE_KAPS_IBC_COMMAND_WORDr(unit, reg_val);
#endif /*ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS*/
    if (rv != SOC_E_NONE) {
        res = KBP_INTERNAL_ERROR;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0, "%s():  blk_id: %d, cmd: %d, func: %d, offset: %d, reg_val:0x%08X\n"),
                         FUNCTION_NAME(), blk_id, cmd, func, offset, reg_val));
#endif
    return res;
}

STATIC kbp_status jer_pp_kaps_search(void *xpt, 
                                     uint8_t *key, 
                                     enum kaps_search_interface search_interface, 
                                     struct kaps_search_result *kaps_result)
{
    int rv, unit;
    uint32  blk_id,
            func,
        tmp, val,i =0,j,k;
    JER_KAPS_DATA_REGS data_reg; 

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0, "%s():  search_interface: %d, key: 0x"),
                         FUNCTION_NAME(), search_interface));
    for (j = 0; j < JER_KAPS_KEY_BUFFER_NOF_BYTES; j++) {
        LOG_CLI((BSL_META_U(0,"%02X"),key[j]));
    }
    LOG_CLI((BSL_META_U(0,"\n")));
#endif

    unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;
    blk_id = search_interface + JER_KAPS_XPT_BLOCK_OFFSET_RPB;
    func = JER_KAPS_XPT_RPB_FUNC_KAPS_SEARCH;

    /* polling command word to validate it's ready */
    rv = jer_pp_kaps_command_word_poll(unit, 0);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* write the key to the data registers */
    data_reg = DATA_10;

    for (k = 0; k < JER_KAPS_KEY_BUFFER_NOF_BYTES/4; k++) {
        tmp = 0;
        for(j=0; j<4; j++) {
            val = key[i];
            tmp |= val << (3-j)*8;
            i++;
        }
        /* write tmp to data register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
        {
            soc_reg_above_64_val_t above_64_val;
            above_64_val[0] = tmp;
            above_64_val[1] = 0;
            ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_COMMAND_DATA_0 + data_reg, above_64_val);
        }
#else
        WRITE_KAPS_IBC_COMMAND_DATAr(unit, data_reg, tmp);
#endif
        data_reg++;
    }

    /* trigger search action by writing to command word register */
    rv = jer_pp_kaps_command_word_write(unit, blk_id, KAPS_CMD_READ, func, 0);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* poll response word until response is ready */
    rv = jer_pp_kaps_response_word_poll(unit, blk_id, JER_KAPS_XPT_RESPONSE_VALID, func, JER_KAPS_RESPONSE_STATUS_AOK);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* read response_data_14, bits 24-31 */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = val;
        above_64_val[1] = 0;
        ARAD_FAST_REGISER_GET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_DATA_0 + DATA_14, above_64_val);
    }
#else
    READ_KAPS_IBC_RESPONSE_DATAr(unit, DATA_14, &val);
#endif
    kaps_result->match_len = val >> 24;

    /* read response_data_13, bits 0-19 */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = 0;
        above_64_val[1] = 0;
        ARAD_FAST_REGISER_GET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_DATA_0 + DATA_13, above_64_val);
        val = above_64_val[0];
    }
#else
    READ_KAPS_IBC_RESPONSE_DATAr(unit, DATA_13, &val);
#endif
    kaps_result->ad_value[2]= (val << 4);
    kaps_result->ad_value[1]= (val >> 4);
    kaps_result->ad_value[0]= (val >> 12);

    /* zero response word to dequeue response fifo */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = 0;
        above_64_val[1] = 0;
        rv = ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_WORD, above_64_val);
    }
#else
    rv = WRITE_KAPS_IBC_RESPONSE_WORDr(unit, 0);
#endif
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0,"%s()\n"), FUNCTION_NAME()));
#endif
    return rv;
}

/*Retrieve the R5 read pointer in case we reach it with this write*/
STATIC uint32 jer_pp_kaps_retrieve_read_p(int unit,
                                          uint32 next_write_max_address)
{
    int rv = SOC_E_NONE;
    uint32 read_pointer_index;
    uint32 reg_val;
    uint32 counter = 1000;

    while ( /*Regular write, make sure we are behind the pointer before write, and ahead of it after write
          *This also applies to the case where the next write is going to wrap around and the read pointer has not wrapped around yet */
        (kaps_tcm_write_p[unit] < kaps_tcm_read_p[unit]) && (next_write_max_address >= kaps_tcm_read_p[unit]) && counter
         /*Does not check for wrap around since the function that calls this writes twice if it needs to wrap around*/)  {
            /* get the pointers register */
            rv = READ_KAPS_STATUS_CFIFOr(unit, 0/*array index*/, &reg_val);
            if (rv != SOC_E_NONE) {
                return KBP_INTERNAL_ERROR;
            }

            #ifdef JER_KAPS_XPT_REG_PRINTS_ENABLED
                LOG_CLI((BSL_META_U(0,"The old read pointer is %d. "),kaps_tcm_read_p[unit]));
            #endif

            /*The retrieved index is in 64bits, while the addressing is in 8bits*/
            read_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, READPf);
            kaps_tcm_read_p[unit] = JER_KAPS_CONFIG_CFIFO_BASE_8 + read_pointer_index * 8;

            #ifdef JER_KAPS_XPT_REG_PRINTS_ENABLED
                LOG_CLI((BSL_META_U(0,"The new retrieved pointer is %d."),kaps_tcm_read_p[unit]));
                LOG_CLI((BSL_META_U(0,"\n")));
            #endif

            counter--;
    }
    if (counter == 0) {
        rv = SOC_E_TIMEOUT;
    }

    return rv;
}


kbp_status jer_pp_kaps_write_command_arm(void *xpt,
                                            uint8 blk_id,
                                            uint32 cmd,
                                            uint32 func,
                                            uint32 offset,
                                            uint32 nbytes,
                                            uint8 *bytes)
{
    int rv, unit, i;
    uint32 temp_ndwords;
    uint8 new_descriptor = 0;

    /* Each TCM entry is 64bits, at most we utilize ceil(480/64)=8 + 1 (control) entries per command */
    uint32 tcm_entry[18];

    uint32 field_val, temp_nbytes;

    uint8 entry_size_counter;

    uint8 tcm_entry_counter = 0;

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    int j;
    LOG_CLI((BSL_META_U(0,"%s() start\n nbytes: %d, bytes: 0x"),FUNCTION_NAME(), nbytes));
    for (j = 0; j < nbytes; j++) {
        LOG_CLI((BSL_META_U(0,"%02X "),bytes[j]));
    }
    LOG_CLI((BSL_META_U(0,"\n")));
#endif

    rv = SOC_SAND_OK;

    unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;

    /* write to command data memory address */
    /*4 MSBs - 32 bit only contain the nbytes, 4 LSBs - 32 bits contain the command word register*/
    field_val = blk_id;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_BLKID, &tcm_entry[tcm_entry_counter], &field_val);

    field_val = cmd;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_CMD, &tcm_entry[tcm_entry_counter], &field_val);

    field_val = func;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_FUNC, &tcm_entry[tcm_entry_counter], &field_val);

    field_val = offset;
    ARAD_FAST_FIELD_SET(ARAD_FAST_FIELD_IHP_KAPS_IBC_COMMAND_WORD_OFFSET, &tcm_entry[tcm_entry_counter], &field_val);

    tcm_entry_counter++;
    tcm_entry[tcm_entry_counter++] = nbytes;

    if (nbytes == 60) {
        tcm_entry[tcm_entry_counter++] = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
        tcm_entry[tcm_entry_counter++] = 0;
    } else if (nbytes == 21) {
        tcm_entry[tcm_entry_counter++] = bytes[1] << 24 | bytes[2] << 16 | bytes[3] << 8 | bytes[4];
        tcm_entry[tcm_entry_counter++] = bytes[0];
    }

    temp_ndwords = nbytes/NOF_UINT8_IN_UINT64;
    temp_nbytes =  nbytes%8;
    for ( i = 0; i < temp_ndwords; i++) {
        tcm_entry[tcm_entry_counter++] = bytes[temp_nbytes+8*i+4] << 24 | bytes[temp_nbytes+8*i+5] << 16 | bytes[temp_nbytes+8*i+6] << 8 | bytes[temp_nbytes+8*i+7];
        tcm_entry[tcm_entry_counter++] = bytes[temp_nbytes+8*i] << 24 | bytes[temp_nbytes+8*i+1] << 16 | bytes[temp_nbytes+8*i+2] << 8 | bytes[temp_nbytes+8*i+3];
    }

    /* number of rows written to TCM for this entry */
    entry_size_counter = nbytes%8 ? nbytes/NOF_UINT8_IN_UINT64 + 2 : nbytes/NOF_UINT8_IN_UINT64 + 1;

    if (kaps_tcm_write_p[unit] + entry_size_counter * 8 > JER_KAPS_CONFIG_CFIFO_MAX_ADD_8) {
        /* Remaining writes before wrap-around */
        uint32 remaining_writes = 1 + (JER_KAPS_CONFIG_CFIFO_MAX_ADD_8 - kaps_tcm_write_p[unit]) / 8;
        /* Break the wrap around into two separate descriptors */
        rv = jer_pp_kaps_retrieve_read_p(unit, JER_KAPS_CONFIG_CFIFO_MAX_ADD_8);
        if (rv != SOC_SAND_OK) {
            LOG_CLI((BSL_META_U(0,"%s(), jer_pp_kaps_retrieve_read_p failed.\n"),FUNCTION_NAME()));
            return KBP_FATAL_TRANSPORT_ERROR;
        }

        rv = jer_sbusdma_desc_add_fifo_dma(unit, KAPS_TCMm, 0/*index*/, KAPS_BLOCK(unit), kaps_tcm_write_p[unit], &tcm_entry[0], remaining_writes, 3, 0);
        if (rv != SOC_SAND_OK) {
            LOG_CLI((BSL_META_U(0,"%s(), jer_sbusdma_desc_add failed.\n"),FUNCTION_NAME()));
            return KBP_FATAL_TRANSPORT_ERROR;
        }

        kaps_tcm_write_p[unit] = JER_KAPS_CONFIG_CFIFO_BASE_8;

        if ((entry_size_counter - remaining_writes) > 0) {
            rv = jer_pp_kaps_retrieve_read_p(unit, kaps_tcm_write_p[unit] + (entry_size_counter - remaining_writes) * 8);
            if (rv != SOC_SAND_OK) {
                LOG_CLI((BSL_META_U(0,"%s(), jer_pp_kaps_retrieve_read_p failed.\n"),FUNCTION_NAME()));
                return KBP_FATAL_TRANSPORT_ERROR;
            }
            rv = jer_sbusdma_desc_add_fifo_dma(unit, KAPS_TCMm, 0/*index*/, KAPS_BLOCK(unit), kaps_tcm_write_p[unit], &tcm_entry[remaining_writes*2], entry_size_counter - remaining_writes, 3, 1);
            if (rv != SOC_SAND_OK) {
                LOG_CLI((BSL_META_U(0,"%s(), jer_sbusdma_desc_add failed.\n"),FUNCTION_NAME()));
                return KBP_FATAL_TRANSPORT_ERROR;
            }

            kaps_tcm_write_p[unit] += (entry_size_counter - remaining_writes) * 8;
        }
    } else {
        rv = jer_pp_kaps_retrieve_read_p(unit, kaps_tcm_write_p[unit] + (entry_size_counter * 8));
        if (rv != SOC_SAND_OK) {
            LOG_CLI((BSL_META_U(0,"%s(), jer_pp_kaps_retrieve_read_p failed.\n"),FUNCTION_NAME()));
            return KBP_FATAL_TRANSPORT_ERROR;
        }

        if (kaps_tcm_write_p[unit] == JER_KAPS_CONFIG_CFIFO_BASE_8) {
            new_descriptor = 1;
        }

        rv = jer_sbusdma_desc_add_fifo_dma(unit, KAPS_TCMm, 0/*index*/, KAPS_BLOCK(unit), kaps_tcm_write_p[unit], &tcm_entry[0], entry_size_counter, 3, new_descriptor);
        if (rv != SOC_SAND_OK) {
            LOG_CLI((BSL_META_U(0,"%s(), jer_sbusdma_desc_add failed.\n"),FUNCTION_NAME()));
            return KBP_FATAL_TRANSPORT_ERROR;
        }

        kaps_tcm_write_p[unit] += entry_size_counter * 8;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0, "%s():  blk_id: %d, cmd: %d, func: %d, offset: %d\n"),
                         FUNCTION_NAME(), blk_id, cmd, func, offset));
    LOG_CLI((BSL_META_U(0,"%s() end\n"),FUNCTION_NAME()));
#endif

    return rv;
}

kbp_status jer_pp_kaps_write_command(void *xpt,  
                                            uint8 blk_id, 
                                            uint32 cmd,
                                            uint32 func,
                                            uint32 offset,
                                            uint32 nbytes, 
                                            uint8 *bytes)
{
    int rv, unit;
    uint32 tmp, val,i =0,j,k;
    JER_KAPS_DATA_REGS data_reg;

    unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0,"%s() start\n nbytes: %d, bytes: 0x"),FUNCTION_NAME(), nbytes));
    for (j = 0; j < nbytes; j++) {
        LOG_CLI((BSL_META_U(0,"%02X "),bytes[j]));
    }
    LOG_CLI((BSL_META_U(0,"\n")));
#endif



    /* polling command word to validate it's ready */
    rv = jer_pp_kaps_command_word_poll(unit, 0);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* write to command data registers */
    memset(&tmp, 0 , sizeof(uint32));
    data_reg = DATA_LAST - SOC_SAND_DIV_ROUND_UP(nbytes,4);

    if (nbytes%4 > 0) {
        tmp = 0;
        for (i = 0; i < nbytes%4; i++) {
            val = bytes[i];
            tmp |= val << (nbytes%4-1-i)*8;
        }
        /* write to data register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
        {
            soc_reg_above_64_val_t above_64_val;
            above_64_val[0] = tmp;
            above_64_val[1] = 0;
            ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_COMMAND_DATA_0 + data_reg, above_64_val);
        }
#else
        WRITE_KAPS_IBC_COMMAND_DATAr(unit, data_reg, tmp);
#endif
#ifdef JER_KAPS_XPT_PRINTS_ENABLED
        LOG_CLI((BSL_META_U(0,"data_reg%d: 0x%08X \n"), data_reg, tmp));
#endif
        data_reg++;
    }

    for (k = 0; k < nbytes/4; k++) {
        tmp = 0;
        for(j=0; j<4; j++) {
            val = bytes[i];
            tmp |= val << (3-j)*8;
            i++;
        }
        /* write to data register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
        {
            soc_reg_above_64_val_t above_64_val;
            above_64_val[0] = tmp;
            above_64_val[1] = 0;
            ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_COMMAND_DATA_0 + data_reg, above_64_val);
        }
#else
        WRITE_KAPS_IBC_COMMAND_DATAr(unit, data_reg, tmp);
#endif
#ifdef JER_KAPS_XPT_PRINTS_ENABLED
        LOG_CLI((BSL_META_U(0,"data_reg%d: 0x%08X \n"), data_reg, tmp));
#endif
        data_reg++;
    }

    /* trigger write action by writing to command word register */
    rv = jer_pp_kaps_command_word_write(unit, blk_id, cmd, func, offset);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* poll response word until response is ready */
    rv = jer_pp_kaps_response_word_poll(unit, blk_id, JER_KAPS_XPT_RESPONSE_VALID, func, JER_KAPS_RESPONSE_STATUS_AOK);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* zero response word to dequeue response fifo */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = 0;
        above_64_val[1] = 0;
        ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_WORD, above_64_val);
    }
#else
    rv = WRITE_KAPS_IBC_RESPONSE_WORDr(unit, 0);
#endif
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0,"%s() end\n"),FUNCTION_NAME()));
#endif

    return rv;

}

uint32 jer_pp_xpt_wait_dma_done(int unit)
{
    uint32 reg_val;
    uint32 read_pointer_index,
           write_pointer_index,
           response_depth,
           command_depth,
           counter = 0;

    SOCDNX_INIT_FUNC_DEFS;

    if (kaps_dma_enabled[unit]) {
        do {
            SOCDNX_SAND_IF_ERR_EXIT(READ_KAPS_STATUS_CFIFOr(unit, 0/*array index*/, &reg_val));

            read_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, READPf);
            write_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, WRITEPf);

            counter++;
            if (counter > 1000) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_SOCDNX_MSG("ARM is taking too long to finish operation.\n")));
            }
            SOCDNX_SAND_IF_ERR_EXIT(READ_KAPS_IBC_QUEUE_STATUSr(unit, &reg_val));
            response_depth = soc_reg_field_get(unit, KAPS_IBC_QUEUE_STATUSr, reg_val, RESPONSE_DEPTHf);
            command_depth = soc_reg_field_get(unit, KAPS_IBC_QUEUE_STATUSr, reg_val, COMMAND_DEPTHf);
            if (response_depth || command_depth) {
                LOG_CLI((BSL_META_U(0,"Response depth is %d. Command depth is %d.\n"), response_depth, command_depth));
            }
            SOCDNX_IF_ERR_EXIT(jer_sbusdma_desc_wait_done(unit));
        } while ((read_pointer_index != write_pointer_index) || response_depth || command_depth);
        /* Wait for host to end transmitting and the ARM to finish working */
    }

exit:
    SOCDNX_FUNC_RETURN;
}

kbp_status jer_pp_kaps_read_command(void *xpt,  
                                           uint32 blk_id, 
                                           uint32 cmd,
                                           uint32 func,
                                           uint32 offset,
                                           uint32 n_result_bytes, 
                                           uint8 *result_bytes)
{
    int rv, unit;
    uint32 tmp, val,i =0,j,k;
    JER_KAPS_DATA_REGS data_reg; 

    unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;

    /* polling command word to validate it's ready */
    rv = jer_pp_kaps_command_word_poll(unit, 0);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* trigger read action by writing to command word register */
    rv = jer_pp_kaps_command_word_write(unit, blk_id, cmd, func, offset);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* poll response word until response is ready */
    rv = jer_pp_kaps_response_word_poll(unit, blk_id, JER_KAPS_XPT_RESPONSE_VALID, func, JER_KAPS_RESPONSE_STATUS_AOK);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    /* copy response data to buffer */
    memset(&tmp, 0 , sizeof(uint32));
    data_reg = DATA_LAST - SOC_SAND_DIV_ROUND_UP(n_result_bytes,4);

    if (n_result_bytes%4 > 0) {
        /* read from response data register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
        {
            soc_reg_above_64_val_t above_64_val;
            above_64_val[0] = 0;
            above_64_val[1] = 0;
            ARAD_FAST_REGISER_GET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_DATA_0 + data_reg, above_64_val);
            val = above_64_val[0];
        }
#else
        READ_KAPS_IBC_RESPONSE_DATAr(unit, data_reg, &val);
#endif
        for (i = 0; i < n_result_bytes%4; i++) {
            result_bytes[i] = val >> ((n_result_bytes%4-1-i)*8);
        }
        /* decrement response data register */
        data_reg++;
    }

    for (k = 0; k < n_result_bytes/4; k++) {
        /* read from response data register */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
        {
            soc_reg_above_64_val_t above_64_val;
            above_64_val[0] = 0;
            above_64_val[1] = 0;
            ARAD_FAST_REGISER_GET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_DATA_0 + data_reg, above_64_val);
            val = above_64_val[0];
        }
#else
        READ_KAPS_IBC_RESPONSE_DATAr(unit, data_reg, &val);
#endif
        for(j=0; j<4; j++) {
            result_bytes[i] = val >> (3-j)*8;
            i++;
        }
        /* write to data register */
        data_reg++;
    }

    /* zero response word to dequeue response fifo */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = tmp;
        above_64_val[1] = 0;
        ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_WORD, above_64_val);
    }
#else
    rv = WRITE_KAPS_IBC_RESPONSE_WORDr(unit, 0);
#endif
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0, "%s()\n"), FUNCTION_NAME()));
#endif

    return rv;
}

uint32 jer_pp_xpt_extended_command(void *xpt,  
                                   uint32 blk_id, 
                                   uint32 cmd,
                                   uint32 func,
                                   uint32 offset)
{
    int rv, unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;


    SOCDNX_INIT_FUNC_DEFS;

    /* polling command word to validate it's ready */
    rv = jer_pp_kaps_command_word_poll(unit, 0);
    SOCDNX_IF_ERR_EXIT(rv);

    /* trigger enumeration command by writing to command word register */
    rv = jer_pp_kaps_command_word_write(unit, blk_id, cmd, func, offset);
    SOCDNX_IF_ERR_EXIT(rv);

    /* poll response word until response is ready. */
    rv = jer_pp_kaps_response_word_poll(unit, JER_KAPS_XPT_NOF_BLOCKS, JER_KAPS_XPT_RESPONSE_EXTENDED_VALID, func, JER_KAPS_RESPONSE_STATUS_AOK);
    SOCDNX_IF_ERR_EXIT(rv);

    /* zero response word to dequeue response fifo */
#ifdef ARAD_FAST_REGISTERS_AND_FIELDS_ACCESS
    {
        soc_reg_above_64_val_t above_64_val;
        above_64_val[0] = 0;
        above_64_val[1] = 0;
        rv = ARAD_FAST_REGISER_SET(ARAD_FAST_REG_IHP_KAPS_IBC_RESPONSE_WORD, above_64_val);
    }
#else
    rv = WRITE_KAPS_IBC_RESPONSE_WORDr(unit, 0);
#endif
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


STATIC kbp_status jer_pp_kaps_command(void *xpt,
                                      enum kaps_cmd cmd,
                                      enum kaps_func func,
                                      uint32_t blk_nr,
                                      uint32_t row_nr,
                                      uint32_t nbytes,
                                      uint8_t *bytes)
{
    int rv = KBP_OK;
    int unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;

    switch (cmd)
    {
       case KAPS_CMD_READ:
           rv = jer_pp_kaps_read_command(xpt, blk_nr, cmd, func, row_nr, nbytes, bytes);
           break;

       case KAPS_CMD_WRITE:
           if (kaps_dma_enabled[unit]) {
               rv = jer_pp_kaps_write_command_arm(xpt, blk_nr, cmd, func, row_nr, nbytes, bytes);
           } else {
               rv = jer_pp_kaps_write_command(xpt, blk_nr, cmd, func, row_nr, nbytes, bytes);
           }
           break;

       case KAPS_CMD_EXTENDED:
           if (!SOC_WARM_BOOT(((JER_KAPS_XPT*)xpt)->jer_data.unit)) {
               rv = jer_pp_xpt_extended_command(xpt, blk_nr, cmd, func, row_nr);
           }
           break;

       default:
           LOG_CLI((BSL_META_U(0, "%s():  unsupported cmd: %d\n"), 
                                FUNCTION_NAME(), cmd));
           rv = KBP_INTERNAL_ERROR;
           break;
    }

    return rv;
}

kbp_status kaps_register_read(void *xpt, uint32_t offset, uint32_t nbytes, uint8_t *bytes)
{
    int rv, unit;
    uint32 i, val;
    uint8 array_index;
    soc_reg_t reg;

    unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;
    reg  = offset_to_register_access_info[offset].reg;
    array_index = offset_to_register_access_info[offset].index;

    rv = soc_reg32_get(unit, reg, REG_PORT_ANY, array_index, &val);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

    for (i = 0; i < nbytes; i++) {
        bytes[i] = (val >> ((nbytes-1-i)*8)) & 0xff;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0, "%s()\n"), FUNCTION_NAME()));
#endif

    return rv;
}

kbp_status kaps_register_write(void *xpt, uint32_t offset, uint32_t nbytes, uint8_t *bytes)
{
    int rv, unit;
    uint32 reg_val, val, i;
    uint8 array_index;
    soc_reg_t reg;

    unit = ((JER_KAPS_XPT*)xpt)->jer_data.unit;
    reg  = offset_to_register_access_info[offset].reg;
    array_index = offset_to_register_access_info[offset].index;

    /* set data */
    reg_val = 0;
    for (i = 0; i < nbytes; i++) {
        val = bytes[i];
        reg_val |= val << (nbytes-1-i)*8;
    }

    rv = soc_reg32_set(unit, reg, REG_PORT_ANY, array_index, reg_val);
    if (rv != SOC_SAND_OK) {
        return KBP_INTERNAL_ERROR;
    }

#ifdef JER_KAPS_XPT_PRINTS_ENABLED
    LOG_CLI((BSL_META_U(0,"%s()\n"),FUNCTION_NAME()));
#endif

    return rv;

}

/*Initialize the KAPS TCM memories*/
uint32 jer_pp_xpt_arm_init(int unit)
{
    int rv;
    uint32 field_val, reg_val;
    soc_timeout_t to;
    uint32 a_tcm_init_done = 0,
           b_0_tcm_init_done = 0,
           b_1_tcm_init_done = 0;
    soc_field_t fields[9];/*9 is the max number of fields we need to set in a single register*/
    uint32 values[9];


    SOCDNX_INIT_FUNC_DEFS;

    /* First reset the R5 */
    SOC_ALLOW_WB_WRITE(unit, soc_reg_field32_modify(unit, KAPS_R_5_CORE_CONTROLr, KAPS_BLOCK(unit), CPU_HALT_Nf, 0), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    field_val = 0;
    fields[0] = RESET_Nf; values[0] = field_val;
    fields[1] = SYS_PORESET_Nf; values[1] = field_val;

    SOC_ALLOW_WB_WRITE(unit, soc_reg_fields32_modify(unit, KAPS_R_5_CORE_CONTROLr, KAPS_BLOCK(unit), 2, fields, values), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /* Enable ECC checking and generation inside the R5 */
    field_val = 0x7;
    SOC_ALLOW_WB_WRITE(unit, soc_reg_field32_modify(unit, KAPS_R_5_CORE_CONFIGAr, KAPS_BLOCK(unit), ECC_ENABLEf, field_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /* Reset the TCM memories */
    field_val = 1;
    fields[0] = A_POWERDOWNf; values[0] = field_val;
    fields[1] = B_0_POWERDOWNf; values[1] = field_val;
    fields[2] = B_1_POWERDOWNf; values[2] = field_val;

    fields[3] = A_STANDBYf; values[3] = field_val;
    fields[4] = B_0_STANDBYf; values[4] = field_val;
    fields[5] = B_1_STANDBYf; values[5] = field_val;

    /*Initialize the TCM memories*/
    field_val = 0;
    fields[6] = A_TCM_INITf; values[6] = field_val;
    fields[7] = B_0_TCM_INITf; values[7] = field_val;
    fields[8] = B_1_TCM_INITf; values[8] = field_val;

    SOC_ALLOW_WB_WRITE(unit, soc_reg_fields32_modify(unit, KAPS_R_5_CORE_MEM_CONTROLr, KAPS_BLOCK(unit), 9, fields, values), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /* Take the TCM memories out of the powerdown and standby states */
    field_val = 0;
    fields[0] = A_POWERDOWNf; values[0] = field_val;
    fields[1] = B_0_POWERDOWNf; values[1] = field_val;
    fields[2] = B_1_POWERDOWNf; values[2] = field_val;

    fields[3] = A_STANDBYf; values[3] = field_val;
    fields[4] = B_0_STANDBYf; values[4] = field_val;
    fields[5] = B_1_STANDBYf; values[5] = field_val;

    /*Initialize the TCM memories*/
    field_val = 1;
    fields[6] = A_TCM_INITf; values[6] = field_val;
    fields[7] = B_0_TCM_INITf; values[7] = field_val;
    fields[8] = B_1_TCM_INITf; values[8] = field_val;

    SOC_ALLOW_WB_WRITE(unit, soc_reg_fields32_modify(unit, KAPS_R_5_CORE_MEM_CONTROLr, KAPS_BLOCK(unit), 9, fields, values), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Wait for the initialization to complete*/
    if (!SAL_BOOT_PLISIM) {
        /* polling command word to validate it's ready */
        soc_timeout_init(&to, ARAD_TIMEOUT, ARAD_MIN_POLLS);
        for(;;){
            /* get command word register */
            SOC_ALLOW_WB_WRITE(unit, READ_KAPS_R_5_CORE_MEM_CONTROLr(unit, &reg_val), rv);
            if (rv != SOC_E_NONE) {
                return KBP_INTERNAL_ERROR;
            }
            /* validate against expected value */
            a_tcm_init_done = soc_reg_field_get(unit, KAPS_R_5_CORE_MEM_CONTROLr, reg_val, A_TCM_INIT_DONEf);
            b_0_tcm_init_done = soc_reg_field_get(unit, KAPS_R_5_CORE_MEM_CONTROLr, reg_val, B_0_TCM_INIT_DONEf);
            b_1_tcm_init_done = soc_reg_field_get(unit, KAPS_R_5_CORE_MEM_CONTROLr, reg_val, B_1_TCM_INIT_DONEf);

            if((a_tcm_init_done == 1) && (b_0_tcm_init_done == 1) && (b_1_tcm_init_done == 1)) {
                break;
            }

            if (soc_timeout_check(&to)) {
                SOCDNX_IF_ERR_EXIT_MSG(SOC_SAND_GEN_ERR, (_BSL_SOCDNX_MSG("Command word register not ready.")));
                break;
            }
        }
    }

    /*Take the R5 out of reset, but leave it halted*/
    field_val = 1;
    fields[0] = RESET_Nf; values[0] = field_val;
    fields[1] = SYS_PORESET_Nf; values[1] = field_val;

    SOC_ALLOW_WB_WRITE(unit, soc_reg_fields32_modify(unit, KAPS_R_5_CORE_CONTROLr, KAPS_BLOCK(unit), 2, fields, values), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize the KAPS FIFOs*/

    /*Initialize a single command fifo*/
    reg_val = 0;
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_CFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize a single command fifo*/
    reg_val = 0;
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, BASEf, JER_KAPS_CONFIG_CFIFO_BASE);
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, NUM_ENTRIESf, JER_JER_KAPS_CONFIG_CFIFO_NUM_ENTRIES);
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, ENTRY_SIZEf, JER_KAPS_CONFIG_CFIFO_ENTRY_SIZE);
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, ENABLEf, 1);
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_CFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize a single response fifo*/
    reg_val = 0;
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_RFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize a single response fifo*/
    reg_val = 0;
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, BASEf, JER_KAPS_CONFIG_RFIFO_BASE);
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, NUM_ENTRIESf, JER_JER_KAPS_CONFIG_RFIFO_NUM_ENTRIES);
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, ENTRY_SIZEf, JER_KAPS_CONFIG_RFIFO_ENTRY_SIZE);
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, ENABLEf, 1);
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_RFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 jer_pp_xpt_arm_deinit(int unit)
{
    int rv;
    uint32 field_val, reg_val;
    soc_field_t fields[9];/*9 is the max number of fields we need to set in a single register*/
    uint32 values[9];


    SOCDNX_INIT_FUNC_DEFS;

    /* reset the R5 */
    SOC_ALLOW_WB_WRITE(unit, soc_reg_field32_modify(unit, KAPS_R_5_CORE_CONTROLr, KAPS_BLOCK(unit), CPU_HALT_Nf, 0), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    field_val = 0;
    fields[0] = RESET_Nf; values[0] = field_val;
    fields[1] = SYS_PORESET_Nf; values[1] = field_val;

    SOC_ALLOW_WB_WRITE(unit, soc_reg_fields32_modify(unit, KAPS_R_5_CORE_CONTROLr, KAPS_BLOCK(unit), 2, fields, values), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize the KAPS FIFOs*/
    /*This will reset them*/
    /*Initialize a single command fifo*/
    reg_val = 0;
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_CFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize a single command fifo*/
    reg_val = 0;
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, BASEf, JER_KAPS_CONFIG_CFIFO_BASE);
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, NUM_ENTRIESf, JER_JER_KAPS_CONFIG_CFIFO_NUM_ENTRIES);
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, ENTRY_SIZEf, JER_KAPS_CONFIG_CFIFO_ENTRY_SIZE);
    soc_reg_field_set(unit, KAPS_CONFIG_CFIFOr, &reg_val, ENABLEf, 1);
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_CFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize a single response fifo*/
    reg_val = 0;
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_RFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*Initialize a single response fifo*/
    reg_val = 0;
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, BASEf, JER_KAPS_CONFIG_RFIFO_BASE);
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, NUM_ENTRIESf, JER_JER_KAPS_CONFIG_RFIFO_NUM_ENTRIES);
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, ENTRY_SIZEf, JER_KAPS_CONFIG_RFIFO_ENTRY_SIZE);
    soc_reg_field_set(unit, KAPS_CONFIG_RFIFOr, &reg_val, ENABLEf, 1);
    SOC_ALLOW_WB_WRITE(unit, WRITE_KAPS_CONFIG_RFIFOr(unit, 0/*Array Index*/, reg_val), rv);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    SOCDNX_FUNC_RETURN;
}

STATIC uint32 jer_pp_xpt_dma_thread_init(int unit)
{
    uint32 reg_val, rv;
    uint32 write_pointer_index, read_pointer_index;

    SOCDNX_INIT_FUNC_DEFS;

    /*Update the pointer*/
    rv = READ_KAPS_STATUS_CFIFOr(unit, 0/*array index*/, &reg_val);
    if (rv != SOC_E_NONE) {
        return KBP_INTERNAL_ERROR;
    }

    /*The retrieved index is in 64bits, while the addressing is in 8bits*/
    write_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, WRITEPf);
    kaps_tcm_write_p[unit] = JER_KAPS_CONFIG_CFIFO_BASE_8 + write_pointer_index * 8;

    /*The retrieved index is in 64bits, while the addressing is in 8bits*/
    read_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, READPf);
    kaps_tcm_read_p[unit] = JER_KAPS_CONFIG_CFIFO_BASE_8 + read_pointer_index * 8;

    SOCDNX_FUNC_RETURN;
}

uint32 jer_pp_xpt_init(int unit, void **xpt)
{
    JER_KAPS_XPT *xpt_p;

    SOCDNX_INIT_FUNC_DEFS;

    *xpt = soc_sand_os_malloc(sizeof(JER_KAPS_XPT), "kaps_xpt");
    if (*xpt == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Error, failed to allocate memory for KAPS XPT")));
    }

    xpt_p = (JER_KAPS_XPT*)*xpt;

    xpt_p->jer_kaps_xpt.device_type = KBP_DEVICE_KAPS;

    if (SOC_IS_JERICHO_PLUS(unit)) {
        xpt_p->jer_kaps_xpt.kaps_search = qax_pp_kaps_search;
        xpt_p->jer_kaps_xpt.kaps_register_read = qax_kaps_register_read;
        xpt_p->jer_kaps_xpt.kaps_command = qax_pp_kaps_command;
        xpt_p->jer_kaps_xpt.kaps_register_write = qax_kaps_register_write;
    } else if (SOC_IS_JERICHO(unit)) {
        xpt_p->jer_kaps_xpt.kaps_search = jer_pp_kaps_search;
        xpt_p->jer_kaps_xpt.kaps_register_read = kaps_register_read;
        xpt_p->jer_kaps_xpt.kaps_command = jer_pp_kaps_command;
        xpt_p->jer_kaps_xpt.kaps_register_write = kaps_register_write;
    }

    xpt_p->jer_data.unit = unit;

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC uint32 jer_pp_xpt_dma_thread_deinit(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_REPORT(jer_pp_xpt_wait_dma_done(unit));

    SOCDNX_FUNC_RETURN;
}

uint32 jer_pp_xpt_deinit(int unit, void *xpt)
{
    SOCDNX_INIT_FUNC_DEFS;

    soc_sand_os_free(xpt);

    if (kaps_dma_enabled[unit]) {
        SOCDNX_IF_ERR_EXIT(jer_pp_xpt_dma_thread_deinit(unit));
    }
    kaps_dma_enabled[unit] = 0;

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 jer_pp_xpt_kaps_arm_image_load_default(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*Initialize the TCM memories*/
    SOCDNX_IF_ERR_EXIT(jer_pp_xpt_arm_init(unit));

    /*Load default software image to ARM*/
    SOCDNX_IF_ERR_EXIT(jer_pp_kaps_arm_image_load_default(unit));

    /*Unhalt ARM*/
    SOCDNX_IF_ERR_EXIT(jer_pp_xpt_arm_start_halt(unit, 0 /*cpu_halt*/));

    /*Enable DMA thread*/
    SOCDNX_IF_ERR_EXIT(jer_pp_xpt_dma_state(unit, 0 /*print_status*/, 1 /*enable_dma_thread*/, 0 /*wait_arm*/));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 jer_pp_xpt_dma_state(int unit, uint32 print_status, uint32 enable_dma_thread, uint32 wait_arm)
{
    uint32 reg_val, rv;
    uint32 read_pointer_index, read_p;
    uint32 write_pointer_index, write_p;
    SOCDNX_INIT_FUNC_DEFS;

    if (enable_dma_thread != kaps_dma_enabled[unit]) {
        if (enable_dma_thread) {
            SOCDNX_IF_ERR_EXIT(jer_pp_xpt_dma_thread_init(unit));
        } else {
            SOCDNX_IF_ERR_EXIT(jer_pp_xpt_dma_thread_deinit(unit));
        }
        kaps_dma_enabled[unit] = enable_dma_thread;
    }

    if (wait_arm) {
        SOCDNX_IF_ERR_EXIT(jer_pp_xpt_wait_dma_done(unit));
    }

    if (print_status) {
        cli_out("DMA is enabled: %d.\n",
                kaps_dma_enabled[unit]);

        rv = READ_KAPS_STATUS_CFIFOr(unit, 0/*array index*/, &reg_val);
        if (rv != SOC_E_NONE) {
            return KBP_INTERNAL_ERROR;
        }

        /*The retrieved index is in 64bits, while the addressing is in 8bits*/
        read_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, READPf);
        read_p = JER_KAPS_CONFIG_CFIFO_BASE_8 + read_pointer_index * 8;

        /*The retrieved index is in 64bits, while the addressing is in 8bits*/
        write_pointer_index = soc_reg_field_get(unit, KAPS_STATUS_CFIFOr, reg_val, WRITEPf);
        write_p = JER_KAPS_CONFIG_CFIFO_BASE_8 + write_pointer_index * 8;

        cli_out("tcm_write_p: %d. tcm_read_p: %d.\n",
                 write_p,           read_p);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * KAPS ARM Functions
 */

/* Unhalt KAPS ARM */
uint32 jer_pp_xpt_arm_start_halt(int unit, int halt)
{
    uint32 rv;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_ALLOW_WB_WRITE(unit, soc_reg_field32_modify(unit, KAPS_R_5_CORE_CONTROLr, KAPS_BLOCK(unit), CPU_HALT_Nf, halt == 0 ? 0x1 : 0x0), rv);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/* Write to KAPS TCM table according to entry */
uint32 jer_pp_xpt_arm_load_file_entry(int unit, uint32 *input_32, int entry_num)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(WRITE_KAPS_TCMm(unit, KAPS_BLOCK(unit), (entry_num) * 8, input_32));

exit:
    SOCDNX_FUNC_RETURN;
}




/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */
