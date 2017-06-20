/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: cmd.h,v 1.34.16.1 Broadcom SDK $
 *
 * TMU CMD defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TMU_CMD_H_
#define _SBX_CALADN3_TMU_CMD_H_

#include <soc/sbx/caladan3/tmu/tmu_config.h>
#include <soc/sbx/sbDq.h>
#include <soc/sbx/caladan3/tmu/hash.h>

#define SOC_SBX_TMU_CMD_WORD_SIZE (64) /* bits */
#define SOC_SBX_TMU_CMD_OFFSET_WORDS (2) /* 32bit offsets of command word 64bit/2 */

typedef enum _soc_sbx_caladan3_tmu_cmd_type_e {
    SOC_SBX_TMU_CMD_NOP=0,
    SOC_SBX_TMU_CMD_XL_READ = 1,
    SOC_SBX_TMU_CMD_XL_WRITE = 2,
    SOC_SBX_TMU_CMD_RAW_READ = 3,
    SOC_SBX_TMU_CMD_RAW_WRITE = 4,
    SOC_SBX_TMU_CMD_EML_INS_BEGIN = 5,
    SOC_SBX_TMU_CMD_EML_INS_END = 6,
    SOC_SBX_TMU_CMD_EML_DELETE = 7,
    SOC_SBX_TMU_CMD_TAPS = 8,
    SOC_SBX_TMU_CMD_LOCK = 9,
    SOC_SBX_TMU_CMD_RELEASE = 10,
    SOC_SBX_TMU_CMD_TRAILER = 15,
    SOC_SBX_TMU_CMD_MAX
} soc_sbx_caladan3_tmu_cmd_type_t; 

typedef enum _soc_sbx_caladan3_tmu_resp_type_e {
    SOC_SBX_TMU_RESPONSE,
    SOC_SBX_TMU_TAPS_RESPONSE,
    SOC_SBX_TMU_RESP_MAX
} soc_sbx_caladan3_tmu_resp_type_t;

#define SOC_SBX_TMU_IS_TAPS_LOOKUP(lookup) \
    (((lookup) >= SOC_SBX_TMU_LKUP_TAPS_IPV4_SUB_KEY) && \
     ((lookup) <= SOC_SBX_TMU_LKUP_TAPS_IPV6_UNIFIED_KEY))

#define SOC_SBX_TMU_EXPECT_RESPONSE_DATA(opcode) \
 ((opcode) == SOC_SBX_TMU_CMD_XL_READ || \
  (opcode) == SOC_SBX_TMU_CMD_RAW_READ)

#define SOC_SBX_TMU_RESP_SUCCESS(errcode) \
  (TMU_ERR_CODE_NONE == errcode)

/* Error Codes */
typedef enum _soc_sbx_caladan3_tmu_error_code_e {
    TMU_ERR_CODE_NONE = 0,
    TMU_ERR_CODE_EML_NO_HARDWARE_CHAIN = 1,
    TMU_ERR_CODE_EML_NO_FREE_CHAIN = 2,
    TMU_ERR_CODE_EML_CHAIN0_FULL = 3,
    TMU_ERR_CODE_EML_CHAIN1_FULL = 4,
    TMU_ERR_CODE_EML_KEY_NOT_FOUND = 5,
    TMU_ERR_CODE_EML_LOCK_FAIL = 6,
    TMU_ERR_CODE_EML_FILTER_NO_UPDATE = 7,
    TMU_ERR_CODE_EML_NO_BD_MATCH = 8,
    TMU_ERR_CODE_RAW_STRADDELS_ROW = 0x10,
    TMU_ERR_CODE_BAD_WRITE_SIZE = 0x11,
    TMU_ERR_CODE_RAW_KV_PAIRS_IS_0 = 0x12,
    TMU_ERR_CODE_SRAM_ECC_ERROR = 0xfe,
    TMU_ERR_CODE_DRAM_ECC_ERROR = 0xff
} soc_sbx_caladan3_tmu_error_code_e_t;

typedef struct _soc_sbx_caladan3_tmu_nop_cmd_s {
    unsigned int echo;
} soc_sbx_caladan3_tmu_nop_cmd_t;

typedef struct _soc_sbx_caladan3_tmu_xl_read_cmd_s {
    unsigned int table;
    unsigned int lookup;
    unsigned int kv_pairs;
    unsigned int entry_num;
} soc_sbx_caladan3_tmu_xl_read_cmd_t;

typedef struct _soc_sbx_caladan3_tmu_xl_write_cmd_s {
    unsigned int offset;
    unsigned int lookup;
    unsigned int table;
    unsigned int size;
    unsigned int entry_num;
    uint32 value_data[64]; /* support upto 1KBits, max supported by XLWRITE */
#define SOC_SBX_TMU_DM_119b_SIZE  (128)
#define SOC_SBX_TMU_DM_247b_SIZE  (256)
#define SOC_SBX_TMU_DM_366b_SIZE  (384)
#define SOC_SBX_TMU_DM_494b_SIZE  (512)
    int   value_size;
} soc_sbx_caladan3_tmu_xl_write_cmd_t;

typedef struct _soc_sbx_caladan3_tmu_raw_cmd_s {
    unsigned int dram;
    unsigned int size;
    unsigned int row;
    unsigned int column;
    unsigned int bank;
} soc_sbx_caladan3_tmu_raw_cmd_t;

typedef struct _soc_sbx_caladan3_tmu_eml_cmd_s {
    unsigned int filter;
    uint32 *key;
    soc_sbx_tmu_hash_key_type_e_t key_type;
    uint32 *value;
    unsigned int table_id;
} soc_sbx_caladan3_tmu_eml_cmd_t;

typedef struct _soc_sbx_caladan3_tmu_lock_cmd_s {
    unsigned int gbl;
    unsigned int table;
    unsigned int entry_num;
} soc_sbx_caladan3_tmu_lock_cmd_t;

#define SOC_SBX_TMU_TAPS_0 (1<<0)
#define SOC_SBX_TMU_TAPS_1 (1<<1)
#define SOC_SBX_TMU_TAPS_ALL (SOC_SBX_TMU_TAPS_0 | SOC_SBX_TMU_TAPS_1)

#define SOC_SBX_TMU_TAPS_CMD_SIZE      (256)
#define SOC_SBX_TMU_TAPS_RESPONSE_SIZE (256)
#define SOC_SBX_TMU_TAPS_CMD_KEY_SIZE  (144)
#define SOC_SBX_TMU_TAPS_SUBCMD_MAX      10

typedef enum _soc_sbx_caladan3_tmu_taps_block_type_e {
    SOC_SBX_TMU_TAPS_BLOCK_MIN = 0,
    SOC_SBX_TMU_TAPS_BLOCK_RPB = 1,
    SOC_SBX_TMU_TAPS_BLOCK_BB  = 2,
    SOC_SBX_TMU_TAPS_BLOCK_BRR = 3,
    SOC_SBX_TMU_TAPS_BLOCK_MAX
} soc_sbx_caladan3_tmu_taps_block_type_t; 

#define SOC_SBX_TMU_TAPS_RPB_TCAM (1<<0)
#define SOC_SBX_TMU_TAPS_RPB_TDM (1<<1)
#define SOC_SBX_TMU_TAPS_RPB_ADS (1<<2)
#define SOC_SBX_TMU_TAPS_RPB_ALL \
    (SOC_SBX_TMU_TAPS_RPB_TCAM | SOC_SBX_TMU_TAPS_RPB_TDM | SOC_SBX_TMU_TAPS_RPB_ADS)

/* TAPS RPB sub-commands */
typedef enum _soc_sbx_caladan3_tmu_taps_rpb_subcmd_type_e {
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_NOP = 0,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_READ = 1,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_WRITE = 2,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_PROPAGATE = 4,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_REPLACE = 5,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_FIND_BUCKET = 6,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_RAW_READ = 8,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_RAW_WRITE = 9,
    SOC_SBX_TMU_TAPS_RPB_SUBCMD_MAX
} soc_sbx_caladan3_tmu_taps_rpb_subcmd_type_t; 

typedef struct _soc_sbx_caladan3_tmu_taps_rpb_subcmd_nop_s {
    unsigned int unused;
} soc_sbx_caladan3_tmu_taps_rpb_subcmd_nop_t;

typedef struct _soc_sbx_caladan3_tmu_taps_rpb_subcmd_write_s {
    unsigned int segment;
    unsigned int offset;
    unsigned int target;
    unsigned int bpm_length;
    unsigned int kshift;
    unsigned int bucket;
    unsigned int best_match;
    unsigned int align_right;
    unsigned int length;
    unsigned int key[5];
} soc_sbx_caladan3_tmu_taps_rpb_subcmd_write_t;

typedef struct _soc_sbx_caladan3_tmu_taps_rpb_subcmd_read_s {
    unsigned int segment;
    unsigned int offset;
    unsigned int target;
} soc_sbx_caladan3_tmu_taps_rpb_subcmd_read_t;

typedef struct _soc_sbx_caladan3_tmu_taps_rpb_subcmd_propagate_s {
    unsigned int segment;
    unsigned int best_match;
    unsigned int align_right;
    unsigned int length;
    unsigned int key[5];
} soc_sbx_caladan3_tmu_taps_rpb_subcmd_propagate_t;

typedef struct _soc_sbx_caladan3_tmu_taps_rpb_subcmd_replace_s {
    unsigned int segment;
    unsigned int bpm_length;
    unsigned int best_match;
    unsigned int align_right;
    unsigned int length;
    unsigned int key[5];
} soc_sbx_caladan3_tmu_taps_rpb_subcmd_replace_t;

/* TAPS BB sub-commands */
typedef enum _soc_sbx_caladan3_tmu_taps_bb_subcmd_type_e {
    SOC_SBX_TMU_TAPS_BB_SUBCMD_NOP = 0,
    SOC_SBX_TMU_TAPS_BB_SUBCMD_READ = 1,
    SOC_SBX_TMU_TAPS_BB_SUBCMD_WRITE = 2,
    SOC_SBX_TMU_TAPS_BB_SUBCMD_RAW_READ = 3,
    SOC_SBX_TMU_TAPS_BB_SUBCMD_RAW_WRITE = 4,
    SOC_SBX_TMU_TAPS_BB_SUBCMD_MAX
} soc_sbx_caladan3_tmu_taps_bb_subcmd_type_t; 

typedef struct _soc_sbx_caladan3_tmu_taps_bb_subcmd_nop_s {
    unsigned int unused;
} soc_sbx_caladan3_tmu_taps_bb_subcmd_nop_t;

typedef struct _soc_sbx_caladan3_tmu_taps_bb_subcmd_write_s {
    unsigned int format;
    unsigned int segment;
    unsigned int offset;
    unsigned int kshift;
    unsigned int prefix_id;
    unsigned int align_right;
    unsigned int length;
    unsigned int key[5];
} soc_sbx_caladan3_tmu_taps_bb_subcmd_write_t;

typedef struct _soc_sbx_caladan3_tmu_taps_bb_subcmd_read_s {
    unsigned int format;
    unsigned int segment;
    unsigned int offset;
    unsigned int kshift;
    unsigned int prefix_id;
    unsigned int align_right;
} soc_sbx_caladan3_tmu_taps_bb_subcmd_read_t;

/* TAPS BRR sub-commands */
typedef enum _soc_sbx_caladan3_tmu_taps_brr_subcmd_type_e {
    SOC_SBX_TMU_TAPS_BRR_SUBCMD_NOP = 0,
    SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ = 1,
    SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE = 2,
    SOC_SBX_TMU_TAPS_BRR_SUBCMD_READ_WIDE = 3,
    SOC_SBX_TMU_TAPS_BRR_SUBCMD_WRITE_WIDE = 4,
    SOC_SBX_TMU_TAPS_BRR_SUBCMD_MAX
} soc_sbx_caladan3_tmu_taps_brr_subcmd_type_t; 

typedef struct _soc_sbx_caladan3_tmu_taps_brr_subcmd_nop_s {
    unsigned int unused;
} soc_sbx_caladan3_tmu_taps_brr_subcmd_nop_t;

typedef struct _soc_sbx_caladan3_tmu_taps_brr_subcmd_write_s {
    unsigned int format;
    unsigned int segment;
    unsigned int offset;
    unsigned int prefix_id;
    unsigned int adata;
} soc_sbx_caladan3_tmu_taps_brr_subcmd_write_t;

typedef struct _soc_sbx_caladan3_tmu_taps_brr_subcmd_read_s {
    unsigned int format;
    unsigned int segment;
    unsigned int offset;
    unsigned int prefix_id;
} soc_sbx_caladan3_tmu_taps_brr_subcmd_read_t;

typedef struct _soc_sbx_caladan3_tmu_taps_brr_subcmd_wide_write_s {
    unsigned int format;
    unsigned int segment;
    unsigned int offset;
    unsigned int prefix_id;
    unsigned int adatas[12];  /* write only field */
} soc_sbx_caladan3_tmu_taps_brr_subcmd_wide_write_t;

typedef struct _soc_sbx_caladan3_tmu_taps_cmd_s {
    /* taps commmand */
    unsigned int instance;
    unsigned int blk;
    unsigned int opcode;
    unsigned int max_key_size;
    union {
	soc_sbx_caladan3_tmu_taps_rpb_subcmd_nop_t rpb_nop;
	soc_sbx_caladan3_tmu_taps_rpb_subcmd_write_t rpb_write;
	soc_sbx_caladan3_tmu_taps_rpb_subcmd_read_t rpb_read;
	soc_sbx_caladan3_tmu_taps_rpb_subcmd_propagate_t rpb_propagate;
	soc_sbx_caladan3_tmu_taps_rpb_subcmd_replace_t rpb_replace;
	soc_sbx_caladan3_tmu_taps_bb_subcmd_nop_t bb_nop;
	soc_sbx_caladan3_tmu_taps_bb_subcmd_write_t bb_write;
	soc_sbx_caladan3_tmu_taps_bb_subcmd_read_t bb_read;
	soc_sbx_caladan3_tmu_taps_brr_subcmd_nop_t brr_nop;
	soc_sbx_caladan3_tmu_taps_brr_subcmd_write_t brr_write;
	soc_sbx_caladan3_tmu_taps_brr_subcmd_read_t brr_read;
	soc_sbx_caladan3_tmu_taps_brr_subcmd_wide_write_t brr_wide_write;
	soc_sbx_caladan3_tmu_taps_brr_subcmd_wide_write_t brr_wide_read; /* read/write same format */
    } subcmd;
} soc_sbx_caladan3_tmu_taps_cmd_t;

typedef struct _soc_sbx_caladan3_tmu_cmd_resp_s {
    /* response */
    unsigned int size;
    unsigned int errcode;
} soc_sbx_caladan3_tmu_cmd_resp_t;

typedef struct _soc_sbx_caladan3_tmu_taps_cmd_resp_s {
    /* taps response */
    unsigned int err0;
    unsigned int err1;
    unsigned int valid0;
    unsigned int valid1;
} soc_sbx_caladan3_tmu_taps_cmd_resp_t;

#define SOC_SBX_TMU_CMD_FLAG_IPV6    (1<<0)

#define POST_PFX_CHECK_DISABLE 0xFF 

typedef struct _soc_sbx_caladan3_tmu_cmd_s {
    dq_t list_elem;
    dq_t wq_list_elem;
    dq_t tc_list_elem;
    dq_t dm_cache_list_elem;
    uint8 opcode;
    uint8 seqnum;
    uint8 flags;
    uint8 post_dpfx_index;
    uint8 slave_seqnum[1];
    union {
        soc_sbx_caladan3_tmu_nop_cmd_t nop;
        soc_sbx_caladan3_tmu_xl_read_cmd_t xlread;
        soc_sbx_caladan3_tmu_xl_write_cmd_t xlwrite;
        soc_sbx_caladan3_tmu_raw_cmd_t raw_read;
        soc_sbx_caladan3_tmu_raw_cmd_t raw_write;
        soc_sbx_caladan3_tmu_eml_cmd_t eml;
        soc_sbx_caladan3_tmu_lock_cmd_t table_lock;
        soc_sbx_caladan3_tmu_lock_cmd_t table_release;
        soc_sbx_caladan3_tmu_taps_cmd_t taps;
    } cmd; 
    soc_sbx_caladan3_tmu_resp_type_t response_type;
    union {
        soc_sbx_caladan3_tmu_cmd_resp_t resp;
        soc_sbx_caladan3_tmu_taps_cmd_resp_t taps;
    } resp_ctrl;
    unsigned int response_trailer; /* response is followed by trailer */
} soc_sbx_caladan3_tmu_cmd_t; 

#define _SOC_SBX_TMU_GET_CMD_LIST_ELEM_(e, var) \
            (var) = DQ_ELEMENT(soc_sbx_caladan3_tmu_cmd_t*,\
                               (e), (var), wq_list_elem)

extern void tmu_cmd_seqnum_set_in_buffer(int seq_num, uint32 *buffer);

extern int tmu_cmd_writeBuf(int unit,
                            soc_sbx_caladan3_tmu_cmd_t *cmd, 
                            uint32 *buffer,
                            unsigned int length);
extern int tmu_cmd_readBuf(int unit,
                           soc_sbx_caladan3_tmu_cmd_t *cmd, 
                           uint32 *buffer,
                           unsigned int length, 
                           uint8 response);
extern void tmu_cmd_printf(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd);
extern void tmu_dump_buffer(unsigned char *buf, int len);
extern int tmu_cmd_get_size_bits(soc_sbx_caladan3_tmu_cmd_t *cmd, int *bits);

extern int tmu_cmd_mgr_init(int unit);
extern int tmu_cmd_mgr_uninit(int unit);
extern int tmu_cmd_alloc(int unit, soc_sbx_caladan3_tmu_cmd_t **cmd);
extern int tmu_cmd_free(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd);
extern int tmu_cmd_enqueue(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd, int fifoid, uint8 head);
extern int tmu_cmd_dequeue(int unit, soc_sbx_caladan3_tmu_cmd_t **cmd, int fifoid);
extern int tmu_cmd_is_empty_response(int unit, soc_sbx_caladan3_tmu_cmd_t *cmd, uint8 *empty);
extern int tmu_cmd_init(soc_sbx_caladan3_tmu_cmd_t *cmd);
extern int tmu_cmd_destory(soc_sbx_caladan3_tmu_cmd_t *cmd);
extern void tmu_resp_error_print(int unit, soc_sbx_caladan3_tmu_cmd_t *resp);

extern int tmu_cmd_eml_kv_unpack(int unit,
                                 uint32 *buffer,
                                 uint32 *key, 
                                 uint32* value,
                                 soc_sbx_tmu_hash_key_type_e_t key_type);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_table_map_dump
 * Purpose:
 *     Dump TMU table allocation
 */
void soc_sbx_caladan3_cmd_mgr_dump(int unit, int fifoid);

/*
 *
 * Function:
 *     soc_sbx_caladan3_cmd_mgr_stats_dump
 * Purpose:
 *     Dump command manager statistics
 */
void soc_sbx_caladan3_cmd_mgr_stats_dump(int unit);



/* 
 *
 * Functions to keep track of command counts */
void tmu_cmd_count_clear(void);
void tmu_cmd_count_print(void);



#endif /* _SBX_CALADN3_TMU_CMD_H_ */
