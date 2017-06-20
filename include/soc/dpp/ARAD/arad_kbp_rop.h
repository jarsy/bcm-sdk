/* $Id: arad_kbp_rop.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __ARAD_KBP_ROP_INCLUDED__
/* { */
#define __ARAD_KBP_ROP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/kbp/nlm3/arch/nlmarch.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* #define ARAD_KBP_ROP_DEBUG_PRINTS 1 */

#define ARAD_KBP_CPU_DATA_REGISTER_BYTE_LEN   (64)
#define ARAD_KBP_CPU_LOOKUP_REPLY_DATA_REGISTER_BYTE_LEN   (32)


#define ARAD_KBP_LUT_REC_TYPE_INFO 0x1 
#define ARAD_KBP_LUT_REC_TYPE_REQUEST 0x0 

#define ARAD_KBP_LUT_INSTR_LUT1_CONTEXID_SEQ_NUM_MODE  0x0
#define ARAD_KBP_LUT_INSTR_LUT1_REC_DATA_CONTEXID_SEQ_NUM_MODE 0x1
#define ARAD_KBP_LUT_INSTR_REC_DATA_CONTEXID_SEQ_NUM_MODE 0x2
#define ARAD_KBP_LUT_INSTR_LUT1_CONTEXID_REC_DATA_MODE 0x3
#define ARAD_KBP_LUT_INSTR_REC_DATA_CONTEXID_REC_DATA_MODE 0x4

#define ARAD_KBP_LUT_KEY_CONFIG_APPEND_INCOMING_DATA 1
#define ARAD_KBP_LUT_KEY_CONFIG_SEND_INCOMING_DATA 0


#define ARAD_KBP_LUT_KEY_SIZE 0x1 
#define ARAD_KBP_LUT_80b_KEY_SIZE 0x0 

#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_16b 0x1
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_32b 0x2
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_48b 0x3
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_64b 0x4
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_80b 0x5
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_96b 0x6
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_112b 0x7
#define ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_128b 0x8

#define ARAD_KBP_LUT_TRANSFER_INDX_ONLY 0x0
#define ARAD_KBP_LUT_TRANSFER_AD_ONLY 0x1

#define ARAD_KBP_LUT_INDX_TRANSFER_1B 0x1
#define ARAD_KBP_LUT_INDX_TRANSFER_2B 0x2
#define ARAD_KBP_LUT_INDX_TRANSFER_3B 0x3
 
#define ARAD_KBP_LUT_AD_TRANSFER_1B 0x1
#define ARAD_KBP_LUT_AD_TRANSFER_2B 0x2
#define ARAD_KBP_LUT_AD_TRANSFER_3B 0x3
#define ARAD_KBP_LUT_AD_TRANSFER_4B 0x4
#define ARAD_KBP_LUT_AD_TRANSFER_5B 0x5
#define ARAD_KBP_LUT_AD_TRANSFER_6B 0x6
#define ARAD_KBP_LUT_AD_TRANSFER_7B 0x7
#define ARAD_KBP_LUT_AD_TRANSFER_8B 0x8
#define ARAD_KBP_LUT_AD_TRANSFER_9B 0x9
#define ARAD_KBP_LUT_AD_TRANSFER_10B 0xa
#define ARAD_KBP_LUT_AD_TRANSFER_11B 0xb
#define ARAD_KBP_LUT_AD_TRANSFER_12B 0xc
#define ARAD_KBP_LUT_AD_TRANSFER_13B 0xd
#define ARAD_KBP_LUT_AD_TRANSFER_14B 0xe
#define ARAD_KBP_LUT_AD_TRANSFER_15B 0xf
#define ARAD_KBP_LUT_AD_TRANSFER_16B 0x0

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_KBP_ROP_REVERSE_DATA(data_in, data_out, len) \
    { \
        int i; \
        for(i = 0; i < len; i++) { \
                   data_out[i] = data_in[len - 1 - i]; \
        } \
    }

#define ARAD_KBP_ROP_LUT_INSTR_CMP1_GET(ltr) \
    ((ltr & 0x3f) | (0x001 << 6) | ((ltr & 0x40) << 9))

#define ARAD_KBP_ROP_LUT_INSTR_CMP3_GET(ltr) \
    ((ltr & 0x3f) | (0x3 << 6) | ((ltr & 0x40) << 9))

#define ARAD_KBP_ROP_LUT_INSTR_CMP1_BUILD(instr, ltr) \
    instr = (ltr & 0x3f) | (0x001 << 6) | ((ltr & 0x40) << 9);

#define ARAD_KBP_ROP_LUT_INSTR_CMP2_BUILD(instr, ltr) \
    instr = (ltr & 0x3f) | (0x2 << 6) | ((ltr & 0x40) << 9);

#define ARAD_KBP_ROP_LUT_INSTR_CMP3_BUILD(instr, ltr) \
    instr = (ltr & 0x3f) | (0x3 << 6) | ((ltr & 0x40) << 9);

/* } */

/*************
 * ENUMS     *
 *************/
/* { */
 
typedef enum
{
    NLM_ARAD_CB_INST_WRITE, /* CB Write */
    NLM_ARAD_CB_INST_CMP1,  /* CB Write and Compare 1 */
    NLM_ARAD_CB_INST_CMP2,  /* CB Write and Compare 2 */
    NLM_ARAD_CB_INST_CMP3,  /* CB Write and Compare 3 */ 
    NLM_ARAD_CB_INST_LPM,   /* CB Write and LPM */
    NLM_ARAD_CB_INST_NONE   /* This does not represent any instruction and is used for marking end of the enum */    
} NlmAradCBInstType;

typedef enum NlmAradReadMode
{
    NLM_ARAD_READ_MODE_DATA_X, /* The read of a database X value is performed*/
    NLM_ARAD_READ_MODE_DATA_Y  /* The read of a database Y value is performed*/
}NlmAradReadMode;

typedef enum NlmAradWriteMode
{
    NLM_ARAD_WRITE_MODE_DATABASE_DM, /* The write mode is database-data mask mode*/
    NLM_ARAD_WRITE_MODE_DATABASE_XY /* The write mode is database-XY mode*/
}NlmAradWriteMode;  

typedef enum NlmAradCompareResponseFormat
{
    NLM_ARAD_ONLY_INDEX_NO_AD = 0x0,  /* Return only index without Associated Data in the result */
    NLM_ARAD_INDEX_AND_32B_AD = 0x1,  /* Return index + 32b Associated Data in the result */
    NLM_ARAD_INDEX_AND_64B_AD = 0x2,  /* Return index + 64b Associated Data in the result */
    NLM_ARAD_INDEX_AND_128B_AD = 0x3, /* Return index + 128b Associated Data in the result */
    NLM_ARAD_INDEX_AND_256B_AD = 0x4,  /* Return index + 256b Associated Data in the result */
    NLM_ARAD_NO_INDEX_NO_AD = 0xffffffff  /* No result */
}NlmAradCompareResponseFormat;
 
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */


/*
 * Request Decoder 
 */
typedef struct arad_kbp_lut_data_t {
    /*
     * Record Size
     *  (max upto 672b=84B)
     *  640b of data + 16b of instr + 16b of contextid
     */
    uint32 rec_size;  

    /*
     * Record Type 
     * Options: 
     *    ARAD_KBP_LUT_REC_TYPE_INFO  
     *    ARAD_KBP_LUT_REC_TYPE_REQUEST 
    */
    uint32 rec_type;    

    /*
     * Record Is Valid
     */
    uint32 rec_is_valid;


/*
 * Request Control
 */

    
/*
 * Mode - To be used to construct the instr, contextId and Key to core based on table 1
 * Options:
 *    ARAD_KBP_LUT_INSTR_LUT1_CONTEXID_SEQ_NUM_MODE  
 *    ARAD_KBP_LUT_INSTR_LUT1_REC_DATA_CONTEXID_SEQ_NUM_MODE 
 *    ARAD_KBP_LUT_INSTR_REC_DATA_CONTEXID_SEQ_NUM_MODE 
 *    ARAD_KBP_LUT_INSTR_LUT1_CONTEXID_REC_DATA_MODE 
 *    ARAD_KBP_LUT_INSTR_REC_DATA_CONTEXID_REC_DATA_MODE 
 */   
    uint32 mode;
    
/*
 * Key_config
 * Options:
 *   ARAD_KBP_LUT_KEY_CONFIG_APPEND_INCOMING_DATA - This feature makes sense only for Compare kind of instructions
 *   ARAD_KBP_LUT_KEY_CONFIG_SEND_INCOMING_DATA
*/
  uint32 key_config;

/*
 * Lut Key Data
 *  Data to be used for final key construction.
 *  max value - 0xff
 */
  uint32 lut_key_data;

/*
 * Instruction Bits
 *    max_value - 0x3ff
*/
  uint32 instr;

/*
 * key_w_cpd_gt_80 
 *    Length of key if record data is between 80b and 128b and copy_data > 80b
 * Options:
 *    ARAD_KBP_LUT_KEY_SIZE - 
 *    ARAD_KBP_LUT_80b_KEY_SIZE - 80b  
 */    
  uint32 key_w_cpd_gt_80;

/*
 * Copy Data Config
 *    Number of bits of Copy Data to be extracted from MSB portion of Record Data (after instr/ctxtId have been extracted 
 *    if applicable but before lut_key_data has been appended to form the final key) and attached to reply.
 * Options:
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_16b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_32b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_48b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_64b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_80b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_96b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_112b 
 *     ARAD_KBP_LUT_EXTR_MSB_PORTION_DATA_128b 
 */
  uint32 copy_data_cfg;


/*
 * Reply Format
 */

/*
 * Resultx Index or AD
 * Options:
 *     ARAD_KBP_LUT_TRANSFER_INDX_ONLY
 *     ARAD_KBP_LUT_TRANSFER_AD_ONLY    
 */


/*
 * Resultx Index Ad Cfg
 * Optiond: 
 *     If Index is to be transferred ( resultx_idx_or_ad = ARAD_KBP_LUT_TRANSFER_INDX_ONLY):
 *         ARAD_KBP_LUT_INDX_TRANSFER_1B 
 *         ARAD_KBP_LUT_INDX_TRANSFER_2B
 *         ARAD_KBP_LUT_INDX_TRANSFER_2B  
 *     If AD is to be transferred  ( resultx_idx_or_ad = ARAD_KBP_LUT_TRANSFER_AD_ONLY): 
 *         ARAD_KBP_LUT_AD_TRANSFER_1B, ARAD_KBP_LUT_AD_TRANSFER_2B... ARAD_KBP_LUT_AD_TRANSFER_16B 
 */


    uint32  result0_idx_ad_cfg;
    uint32  result0_idx_or_ad;

    uint32  result1_idx_ad_cfg;
    uint32  result1_idx_or_ad;

    uint32  result2_idx_ad_cfg;
    uint32  result2_idx_or_ad;

    uint32  result3_idx_ad_cfg;
    uint32  result3_idx_or_ad;

    uint32  result4_idx_ad_cfg;
    uint32  result4_idx_or_ad;

    uint32  result5_idx_ad_cfg;
    uint32  result5_idx_or_ad;
    
}arad_kbp_lut_data_t;


typedef struct arad_kbp_rop_write_s {
    uint8            addr[NLMDEV_REG_ADDR_LEN_IN_BYTES];       /* Address of location where data should be written. includes all :vbit, wrmode ...*/
    uint8            data[NLM_DATA_WIDTH_BYTES];               /* The data that should be written */
    uint8            mask[NLM_DATA_WIDTH_BYTES];               /* The mask that should be written if it is a database entry */
    uint8            addr_short[NLMDEV_REG_ADDR_LEN_IN_BYTES]; /*  Address of location where data should be written. includes nothing */
    uint8            vBit;                                     /* The valid bit which indicates if the database entry should be enabled or disabled */
    NlmAradWriteMode writeMode;                                /* Write mode which indicates if it is Database X or Y*/
    uint32          loop;
    
} arad_kbp_rop_write_t;

typedef struct arad_kbp_rop_read_s {
    uint8  addr[NLMDEV_REG_ADDR_LEN_IN_BYTES];    /* Address of the memory location to be read */
    uint8  vBit;                                  /* valid bit for database read */
    NlmAradReadMode dataType;                     /* reading data either X or Y  */
    uint8  data[NLM_DATA_WIDTH_BYTES + 1];        /* data read will be stored here */
} arad_kbp_rop_read_t;

typedef struct arad_kbp_rop_cbw_s {
        
    uint8 data[NLM_MAX_CMP_DATA_LEN_BYTES];              /* max 640-bit data valid data specified by datalen */
    uint32 data_len;   /*  data length in Bytes (should be multiple of 10, max value = 80) */

} arad_kbp_rop_cbw_t;


typedef struct NlmAradCompareResult
{
    uint8 isResultValid[NLM_MAX_NUM_RESULTS]; /* Indicates if the hit or miss are valid or not 
                                                 if no blocks have been configured for an output port, 
                                                 then the hit/miss returned is not valid*/
    uint8 hitOrMiss[NLM_MAX_NUM_RESULTS];     /* Indicates if a hit or miss occured */
    uint32 hitIndex[NLM_MAX_NUM_RESULTS];     /* Hit indexes indicating DBA/UDA address*/
    NlmAradCompareResponseFormat responseFormat[NLM_MAX_NUM_RESULTS]; /* Indicates the size of AD in the result*/

    /*Associated data. If result 0 has 16b associated data, then it will be stored in
    m_assocData[0][0] and m_assocData[0][1]*/
    uint8 assocData[NLM_MAX_NUM_RESULTS][NLM_MAX_SRAM_SB_WIDTH_IN_BYTES]; 

    uint8 data_raw[NLMDEV_MAX_RESP_LEN_IN_BYTES];


}NlmAradCompareResult;

typedef struct arad_kbp_rop_cbw_cmp_s {
        
    NlmAradCBInstType type;
    uint32 opcode;
    uint32 ltr;
    arad_kbp_rop_cbw_t cbw_data;  
    NlmAradCompareResult result;

} arad_kbp_rop_cbw_cmp_t;

typedef struct arad_kbp_rop_blk_s {
        
    uint32 opcode;
    uint16 loop_cnt;
    uint32 src_addr;	/* First DB address location */
    uint32 dst_addr;	/* destination address for copy/move instructions */
    uint8  set_not_clr; /* validate or invalidate for clear/invalidate instructions */
    uint8  copy_dir;	/* specifies if address is incremented or decremented in the loop */
    uint32 data_len;    /* data length in Bytes */

} arad_kbp_rop_blk_t;

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
/* 
 *  Utility functions
 */

void arad_kbp_format_long_integer(char *buf, uint32 *val, int nval);
void arad_kbp_array_print_debug(uint8 *data, int len);

/*
 *    ELK CPU/ROP access functions
 */

uint32 arad_kbp_cpu_lookup_reply(
    int unit,
    uint32 core,
    soc_reg_above_64_val_t data);

uint32 arad_kbp_cpu_record_send(
    int unit,
    uint32 opcode,
    soc_reg_above_64_val_t msb_data,
    soc_reg_above_64_val_t lsb_data,
    int lsb_enable);

#ifdef BCM_88660
uint32 aradplus_kbp_cpu_record_send(
    int unit,
    uint32 core,
    uint32 opcode,
    soc_reg_above_64_val_t msb_data,
    soc_reg_above_64_val_t lsb_data,
    int lsb_enable,
    soc_reg_above_64_val_t read_data);
#endif

/*
 * arad_kbp_lut_write:
 *    lut_data or lut_data_row can be NULL (but not both).
 *    if lut_data_row == NULL, than lut_data will be used.
 */

uint32 arad_kbp_lut_write(int unit, uint32 core, uint8 addr, arad_kbp_lut_data_t* lut_data, soc_reg_above_64_val_t lut_data_raw); 

uint32 arad_kbp_lut_read(int unit, uint32 core, uint8 addr,arad_kbp_lut_data_t* lut_data, soc_reg_above_64_val_t lut_raw_data);

uint32 arad_kbp_rop_write(
        int unit,
        uint32 core, 
        arad_kbp_rop_write_t *wr_data);

uint32 arad_kbp_rop_read(
        int unit,
        uint32 core, 
        arad_kbp_rop_read_t *rd_data);

uint32 arad_kbp_rop_cbw_cmp(
        int unit, 
        uint32 core,
        arad_kbp_rop_cbw_cmp_t *cbw_cmp_data);

uint32 arad_kbp_rop_blk_write(
        int unit, 
        uint32 core,
        arad_kbp_rop_blk_t *wr_data);

/* } __ARAD_KBP_ROP_INCLUDED__ */


#endif
