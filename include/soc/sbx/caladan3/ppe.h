/*
 * $Id: ppe.h,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    ppe.h
 * Purpose: Caladan3 Packet Parsing Engine drivers
 */

#include <soc/types.h>
#include <soc/drv.h>


#ifdef BCM_CALADAN3_SUPPORT

#ifndef _SOC_SBX_CALADAN3_PPE_DRIVER
#define _SOC_SBX_CALADAN3_PPE_DRIVER


#define SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_VALID     0x3
#define SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID   0x0

#define SOC_SBX_CALADAN3_PPE_QUEUE_DIRECTION  0x40

#define SOC_SBX_CALADAN3_PPE_IS_INGRESS_QUEUE(queueid) \
                (((queueid) & SOC_SBX_CALADAN3_PPE_QUEUE_DIRECTION) ? 1 : 0)
#define SOC_SBX_CALADAN3_PPE_IS_EGRESS_QUEUE(queueid) \
                (((queueid) & SOC_SBX_CALADAN3_PPE_QUEUE_DIRECTION) ? 0 : 1)


/* Barrel Shifter limit 0-63 bytes */
#define SOC_SBX_CALADAN3_PPE_SHIFT_MAX                       63


/* Capture buffer */
#define SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_MAX                 127
#define SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_VARIABLE     0x1
#define SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_SQUEUE       0x2
#define SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_STREAM       0x4
#define SOC_SBX_CALADAN3_PPE_CPDM_MAX_BYTES                  64
#define SOC_SBX_CALADAN3_PPE_CPDM_MAX_WORDS                  16
#define SOC_SBX_CALADAN3_PPE_CPDM_NUM_ENT_PER_PACKET 4


/* Stream */
#define SOC_SBX_CALADAN3_PPE_STREAM_MAX    16
#define SOC_SBX_CALADAN3_PPE_STREAM_MASK   0xF

#define SOC_SBX_CALADAN3_PPE_VARIABLE_LENGTH_BYTES   4
#define SOC_SBX_CALADAN3_PPE_TCAM_STATE_LENGTH_BYTES   3


/*
 * Hash Template management
 * --------------------------
 *    Hash template id 0-3 are bit mapped
 *    Hash template id 4-31 are byte mapped
 *    There are routines that allow user to allocate any specific type 
 *    and program the hash masks as required
 */
#define SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX               32
#define SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_START         0
#define SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_ENTRIES       4

#define SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START     \
           (SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_START + SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_ENTRIES)

#define SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_ENTRIES   \
           (SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX - SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_ENTRIES)


typedef uint8 soc_sbx_caladan3_ppe_hash_id_t;

typedef struct soc_sbx_caladan3_ppe_hdr_hash_s {
    uint32 enable;                       /* Enable/Disable hash */
    soc_sbx_caladan3_ppe_hash_id_t hash_id;  /* hash template to apply */
} soc_sbx_caladan3_ppe_hdr_hash_t;

typedef enum {
    SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT = 0,
    SOC_SBX_CALADAN3_PPE_HASH_TYPE_BYTE = 1,
    SOC_SBX_CALADAN3_PPE_HASH_TYPE_MAX = 2
} soc_sbx_caladan3_ppe_hash_type_t;

#define SOC_SBX_CALADAN3_PPE_HASH_BIT_LENGTH_BITS 320
#define SOC_SBX_CALADAN3_PPE_HASH_BIT_LENGTH \
           (SOC_SBX_CALADAN3_PPE_HASH_BIT_LENGTH_BITS >> 5)
#define SOC_SBX_CALADAN3_PPE_HASH_BYTE_LENGTH_BITS 40
#define SOC_SBX_CALADAN3_PPE_HASH_BYTE_LENGTH \
           ((SOC_SBX_CALADAN3_PPE_HASH_BYTE_LENGTH_BITS + 31) >> 5)

/* Hash template */
typedef struct {
    uint32 mem[SOC_SBX_CALADAN3_PPE_HASH_BIT_LENGTH];
} soc_sbx_caladan3_ppe_hash_bitmem_t;

typedef struct {
    uint32 mem[SOC_SBX_CALADAN3_PPE_HASH_BYTE_LENGTH];
} soc_sbx_caladan3_ppe_hash_bytemem_t;

typedef struct soc_sbx_caladan3_ppe_hash_template_s {
    soc_sbx_caladan3_ppe_hash_type_t type;          /* bit/byte hash */
    soc_sbx_caladan3_ppe_hash_id_t id; 
    uint32 inuse;
    union { 
        soc_sbx_caladan3_ppe_hash_bytemem_t bytemem; 
        soc_sbx_caladan3_ppe_hash_bitmem_t bitmem; 
    } hash;
} soc_sbx_caladan3_ppe_hash_template_t;



/*
 * Initial Queue State
 * -------------------
 *     There is one entry per queue, totally 128 entries
 *     Each field of any entry can be programmed directly by controlling 
 *     iqsm_push. If iqsm_push is not set, then the user will have to
 *     invoke the commit routine to commit into iqsm memory
 */
#define SOC_SBX_CALADAN3_PPE_IQSM_INDEX_MIN  0
#define SOC_SBX_CALADAN3_PPE_IQSM_INDEX_MAX  128

/*
 * SQDM
 * ------
 *    User can program Per source queue user data (5 words)
 *    for each of the 127 source queue. The load_sq_data control
 *    word of the IQSM entry and the keep* flags of the CAMRAM entries
 *    control which bytes/bits are finally visible to the ucode
 */
#define SOC_SBX_CALADAN3_PPE_SQDM_INDEX_MIN  0
#define SOC_SBX_CALADAN3_PPE_SQDM_INDEX_MAX  128
/* Length of sqdm memory in words */
#define SOC_SBX_CALADAN3_PPE_SQDM_LENGTH   5

typedef uint32 soc_sbx_caladan3_ppe_sqdm_t[SOC_SBX_CALADAN3_PPE_SQDM_LENGTH];


/*
 * CAM
 * ---
 *     There are 7 CAMs and and 7 parsers. Each cam is looked up twice
 *     leading to 14 stage lookup.
 *     256 entries per cam, 2 lookups per cam => 128 entries per cam lookup.
 */
#define SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX  14
#define SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX  128

/* 
 * Header Checker
 * --------------
 *   PPE has inbuilt checkers for various header types (10)
 *   When writing a cam rule the user can specify a header checker to apply.
 *   The result of the header checker could lead to an exception
 *   When there is a exception due to the Header checker, the PPE marks
 *   the event register
 */
/* Defined header checkers */
typedef enum {
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_PPP             =  0x0,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_GRE             =  0x1,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_IPV4            =  0x2,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_IPV6            =  0x3,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_ETHERNET        =  0x4,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_VLAN_ONE_TAG    =  0x5,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_VLAN_TWO_TAG    =  0x6,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_VLAN_THREE_TAG  =  0x7,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_ETYPE           =  0x8,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_TCP             =  0x9,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_UDP             =  0xA,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_NONE            =  0xB,
    SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_MAX             =  0xC
} soc_sbx_caladan3_ppe_checker_t;

/* Header checker exceptions */
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_INV_PPP_ADDR_CTL     0x10
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_INV_PPP_PID          0x11
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN0_FFF       0x15
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN1_FFF       0x16
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN2_FFF       0x17
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_SMAC_EQ_DMAC    0x18
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_GRE_INV_RES0         0x19
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_MAC_ZERO        0x1D
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_TYPE_VALUE      0x1E
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_SMAC_MCAST      0x1F
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_RUNT_PKT        0x20
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_OPTIONS         0x21
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_CHECKSUM    0x22
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_VER         0x23
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_RUNT_HDR        0x24
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_LEN_ERR         0x25
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_PKT_LEN_ERR     0x26
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_SA          0x27
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_DA          0x28
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_SA_EQ_DA        0x29
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_LOOPBACK        0x2A
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_MARTIAN_ADDR    0x2B
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER0     0x30
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER1     0x31
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER2     0x32
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER3     0x33
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_ICMP_FRAG       0x38
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_RUNT            0x40
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_VER         0x41
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_PKT_LEN_ERR     0x42
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_SA          0x43
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_DA          0x44
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_SA_EQ_DA        0x45
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_LOOPBACK        0x46
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_USR_FILTER0     0x48
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_USR_FILTER1     0x49
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SP_EQ_DP          0x50
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_NULL_SCAN         0x51
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_XMAS_SCAN         0x52
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SYN_FIN           0x53
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_TINY_FRAG         0x54
#define SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SYN_SPORT_LT_1024 0x55

typedef struct soc_sbx_caladan3_ppe_hdr_checker_s {
    uint32 enable;               /* Enable/Disable checker */
    uint32 id;                   /* Checker id */
    uint32 offset;               /* Checker offset */
} soc_sbx_caladan3_ppe_hdr_checker_t;



/* Property table */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_SEGMENT_MAX       8

#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG0      0x0  /* 1M x 2bit */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG1      0x1  /* 512K x 4bit */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG2      0x2  /* 256K x 8bit */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG_MAX   0x3


#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE0     0
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE1     1
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE2     2
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE3     3
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE4     4
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE5     5
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_STAGE7     6
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_ATTACH_NONE       7


/*
 * Variable length header
 * ----------------------
 *    PPE can parse header that have a field in the header that indicates
 *    the actual size of the header. For instance the IPv4 header uses hlen
 *    field to specify the length of header. The PPE allows user to identify 
 *    the offset within the header where the length field exists, and the 
 *    number of bits that make up the field. It also allows the user to 
 *    specify units of this variable len header. 
 */

/* Variable len header units */
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_UNITS_1B    0x00
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_UNITS_2B    0x01
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_UNITS_4B    0x10
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_UNITS_8B    0x11

/* Variable len header mask */
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_MAX_LEN   0x6 /* field max 6bits */
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_DISABLE   0x0

/* Variable len header shift */
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_SHIFT_PTR(offset)  (offset)
#define SOC_SBX_CALADAN3_PPE_VAR_LEN_FIELD_SHIFT_BITS(offset)  (8 - (offset))

typedef struct soc_sbx_caladan3_ppe_var_len_hdr_s {
    uint8 units;        /* variable length units */
    uint8 mask;         /* Num of bits that specifiy the variable length */
    uint8 shift_bits;   /* Num of bits to shift right in the byte */
    uint8 shift_bytes;  /* Offset of the var header byte */
} soc_sbx_caladan3_ppe_var_len_hdr_t;


typedef uint32 soc_sbx_caladan3_ppe_variable_t;

typedef uint8 soc_sbx_caladan3_ppe_state_t[SOC_SBX_CALADAN3_PPE_TCAM_STATE_LENGTH_BYTES];




/*
 *  Property Table params
 */
typedef struct {
    uint32 property_enable;
    uint32 property_segment;
    uint32 property_index_size;
    uint32 property_index_start;
} soc_sbx_caladan3_ppe_prop_entry_t;

/* PPE IQSM */
typedef struct soc_sbx_caladan3_ppe_initial_queue_state_s {
    uint8 initial_type;                               /* hdr0 type */
    uint8 initial_shift;                              /* loc0, +64 */
    uint8 shift;                                      /* len of hdr0 */
    soc_sbx_caladan3_ppe_var_len_hdr_t var_len_hdr;       /* hdr0 variable len */
    soc_sbx_caladan3_ppe_state_t istate;                  /* user defined state */
    soc_sbx_caladan3_ppe_variable_t initial_variable;     /* user defined var */
    uint8  initial_stream;                            
    uint32 initial_load_sq_data;                      /* ctrl word for disc */
    uint32 hash_template_disable;                     /* ctrl for hash */
    soc_sbx_caladan3_ppe_hdr_checker_t checker;           /* checker config */
    soc_sbx_caladan3_ppe_hdr_hash_t hasher;               /* hash config */
    soc_sbx_caladan3_ppe_prop_entry_t prop;               /* proptable lkup cfg */
} soc_sbx_caladan3_ppe_iqsm_t;
    

/* PPE CAM RAM */
typedef struct soc_sbx_caladan3_ppe_camram_s {
    uint8 hdr_type;                               /* hdrx */
    uint8 shift;                                  /* length of hdrx */
    soc_sbx_caladan3_ppe_var_len_hdr_t var_len_hdr;   /* variable len header */
    soc_sbx_caladan3_ppe_state_t state;               /* user defined state */
    soc_sbx_caladan3_ppe_state_t state_mask;          /* user defined statemask */
    soc_sbx_caladan3_ppe_variable_t variable;         /* user defined variable */
    soc_sbx_caladan3_ppe_variable_t variable_mask;    
    uint8 stream;                                 
    uint8 stream_mask;
    uint8 keep_timestamp;                         /* retain Timestamp */
    uint8 keep_procreg;                           /* retain Proc register */
    uint8 keep_repdata;                           /* retain Replicant data */
    soc_sbx_caladan3_ppe_hdr_checker_t checker;       /* checker config */
    soc_sbx_caladan3_ppe_hdr_hash_t hasher;           /* hash config */
    soc_sbx_caladan3_ppe_prop_entry_t prop;           /* prop table lkup cfg */
} soc_sbx_caladan3_ppe_camram_t;


/* PPE TCAM */
#define SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH   25
#define SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH_WORDS \
         ((SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH + 3) >> 2)

typedef struct {
    uint8 property;
    uint8 property_mask;
    soc_sbx_caladan3_ppe_state_t state;
    soc_sbx_caladan3_ppe_state_t state_mask;
    uint8 data[SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH];
    uint8 mask[SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH];
} soc_sbx_caladan3_ppe_tcamdata_t;


/*
 * Assembler config
 * -----------------
 *  Allows user to specify/configure ppe behavior
 */

typedef struct {
    uint8 exception_stream;  /* Stream id to use when PPE detects exceptions
                              * due to header checker
                              */
    uint8 exception_str_enable; /* Enable pushing the packet to that stream */

    uint8 keep_rep_for_copies; /* Applicable only to copies, retain the
                                * replication data in the header regardless
                                * of load_dq_data control word
                                */
    uint8 load_seq_number; /* set the current value of 16bit sequence number 
                            * instead of procreg[31:16] regardless of the 
                            * load_sq_data control word 
                            */
    uint32 proc_reg_value;  /* value to program into PROC_REGISTER */
} soc_sbx_caladan3_ppe_userdata_t;

/* Header format desc */
typedef struct {
    uint8 word;
    char *desc;
    uint8 pos;
    uint8 size;
} soc_sbx_caladan3_hdesc_format_t;


/* PPE Config */
typedef struct soc_sbx_caladan3_ppe_config_s soc_sbx_caladan3_ppe_config_t;

/* Prototypes */
int soc_sbx_caladan3_ppe_init(int unit);

/* Hash template */
int soc_sbx_caladan3_ppe_hash_template_alloc(int unit, 
                                             soc_sbx_caladan3_ppe_hash_type_t type,
                                             soc_sbx_caladan3_ppe_hash_id_t *id);

int soc_sbx_caladan3_ppe_hash_template_alloc_by_pattern(int unit, 
                                             soc_sbx_caladan3_ppe_hash_type_t type,
                                             uint32 *hash_mask_array,
                                             soc_sbx_caladan3_ppe_hash_id_t *id);

int soc_sbx_caladan3_ppe_hash_template_free(int unit, soc_sbx_caladan3_ppe_hash_id_t id);

int soc_sbx_caladan3_ppe_hash_template_set(int unit, 
                                           soc_sbx_caladan3_ppe_hash_id_t id,
                                           uint32 *hash_mask_array);

int soc_sbx_caladan3_ppe_hash_template_get(int unit,
                                           soc_sbx_caladan3_ppe_hash_id_t id,
                                           uint32 *hash_mask_array);

/* IQSM */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_type_set(int unit, uint8 queueid,
                                                       uint8 type);

int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_type_get(int unit, uint8 queueid,
                                                       uint8 *type);

int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_offset_set(int unit, uint8 queueid,
                                                       uint8 off);

int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_offset_get(int unit, uint8 queueid,
                                                       uint8 *off);

int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_len_set(int unit, uint8 queueid,
                                                       uint8 len);

int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_len_get(int unit, uint8 queueid,
                                                       uint8 *len);

int soc_sbx_caladan3_ppe_iqsm_entry_variable_hdr_len_set(int unit, uint8 queueid,
                                             soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr);

int soc_sbx_caladan3_ppe_iqsm_entry_variable_hdr_len_get(int unit, uint8 queueid,
                                             soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr);

int soc_sbx_caladan3_ppe_iqsm_entry_initial_state_set(int unit, uint8 queueid,
                                              soc_sbx_caladan3_ppe_state_t state);

int soc_sbx_caladan3_ppe_iqsm_entry_initial_state_get(int unit, uint8 queueid,
                                              soc_sbx_caladan3_ppe_state_t state);

int soc_sbx_caladan3_ppe_iqsm_entry_initial_variable_set(int unit, uint8 queueid,
                                             soc_sbx_caladan3_ppe_variable_t *var);

int soc_sbx_caladan3_ppe_iqsm_entry_initial_variable_get(int unit, uint8 queueid,
                                             soc_sbx_caladan3_ppe_variable_t *var);

int soc_sbx_caladan3_ppe_iqsm_entry_initial_stream_set(int unit, uint8 queueid,
                                                       uint8 stream);

int soc_sbx_caladan3_ppe_iqsm_entry_initial_stream_get(int unit, uint8 queueid,
                                                       uint8 *stream);

int soc_sbx_caladan3_ppe_iqsm_entry_load_sq_control_set(int unit, uint8 queueid,
                                                        uint32 load_sq_data);

int soc_sbx_caladan3_ppe_iqsm_entry_load_sq_control_get(int unit, uint8 queueid,
                                                        uint32 *load_sq_data);

int soc_sbx_caladan3_ppe_iqsm_entry_hash_template_disable_set(int unit, uint8 queueid,
                                                              uint32 disable_mask);

int soc_sbx_caladan3_ppe_iqsm_entry_hash_template_disable_get(int unit, uint8 queueid,
                                                              uint32 *disabled_mask);

int soc_sbx_caladan3_ppe_iqsm_entry_checker_set(int unit, uint8 queueid,
                                            soc_sbx_caladan3_ppe_hdr_checker_t *checker);

int soc_sbx_caladan3_ppe_iqsm_entry_checker_get(int unit, uint8 queueid,
                                            soc_sbx_caladan3_ppe_hdr_checker_t *checker);

int soc_sbx_caladan3_ppe_iqsm_entry_hasher_set(int unit, uint8 queueid,
                                            soc_sbx_caladan3_ppe_hdr_hash_t *hasher);

int soc_sbx_caladan3_ppe_iqsm_entry_hasher_get(int unit, uint8 queueid,
                                            soc_sbx_caladan3_ppe_hdr_hash_t *hasher);

int soc_sbx_caladan3_ppe_iqsm_entry_property_set(int unit, uint8 queueid,
                                            soc_sbx_caladan3_ppe_prop_entry_t *prop);

int soc_sbx_caladan3_ppe_iqsm_entry_property_get(int unit, uint8 queueid,
                                            soc_sbx_caladan3_ppe_prop_entry_t *prop);

int soc_sbx_caladan3_ppe_iqsm_entry_read(int unit, uint8 queueid, soc_sbx_caladan3_ppe_iqsm_t *iqsm);
int soc_sbx_caladan3_ppe_iqsm_entry_write(int unit, uint8 queueid, soc_sbx_caladan3_ppe_iqsm_t *iqsm);

int soc_sbx_caladan3_ppe_iqsm_entry_clear(int unit, uint8 queueid);

int soc_sbx_caladan3_ppe_iqsm_entry_commit(int unit, uint8 queueid);


/* SQDM */
int soc_sbx_caladan3_ppe_sqdm_entry_clear(int unit, uint8 queueid);

int soc_sbx_caladan3_ppe_sqdm_entry_write(int unit, uint8 queueid,
                                   soc_sbx_caladan3_ppe_sqdm_t *sqdm);

int soc_sbx_caladan3_ppe_sqdm_entry_read(int unit, uint8 queueid,
                                   soc_sbx_caladan3_ppe_sqdm_t *sqdm);


/* Property Table */
int soc_sbx_caladan3_ppe_property_table_init(int unit,
                                             int mode,
                                             int portA, int portB, int portC, int portD);

int soc_sbx_caladan3_ppe_property_table_segment_init(int unit, int segment, int seg_id, int start);

int soc_sbx_caladan3_ppe_property_table_iaccess(int unit,
                                                int operation,
                                                uint32 address,
                                                uint32 *data);

/* CAMRAM */
int soc_sbx_caladan3_ppe_camram_entry_hdr_type_set(int unit, uint8 camid,
                                             uint8 entry, uint8 type);

int soc_sbx_caladan3_ppe_camram_entry_hdr_type_get(int unit, uint8 camid,
                                             uint8 entry, uint8 *type);

int soc_sbx_caladan3_ppe_camram_entry_hdr_len_set(int unit, uint8 camid,
                                             uint8 entry, uint8 len);

int soc_sbx_caladan3_ppe_camram_entry_hdr_len_get(int unit, uint8 camid,
                                             uint8 entry, uint8 *len);

int soc_sbx_caladan3_ppe_camram_entry_variable_hdr_len_set(int unit, uint8 camid,
                                             uint8 entry,
                                             soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr);

int soc_sbx_caladan3_ppe_camram_entry_variable_hdr_len_get(int unit, uint8 camid, 
                                             uint8 entry,
                                             soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr);

int soc_sbx_caladan3_ppe_camram_entry_state_set(int unit, uint8 camid, uint8 entry,
                                             soc_sbx_caladan3_ppe_state_t state,
                                             soc_sbx_caladan3_ppe_state_t mask);
 
int soc_sbx_caladan3_ppe_camram_entry_state_get(int unit, uint8 camid, uint8 entry,
                                             soc_sbx_caladan3_ppe_state_t state,
                                             soc_sbx_caladan3_ppe_state_t mask);
 
int soc_sbx_caladan3_ppe_camram_entry_variable_set(int unit, uint8 camid, uint8 entry,
                                             soc_sbx_caladan3_ppe_variable_t *var,
                                             soc_sbx_caladan3_ppe_variable_t *mask);

int soc_sbx_caladan3_ppe_camram_entry_variable_get(int unit, uint8 camid, uint8 entry,
                                             soc_sbx_caladan3_ppe_variable_t *var,
                                             soc_sbx_caladan3_ppe_variable_t *mask);

int soc_sbx_caladan3_ppe_camram_entry_stream_set(int unit, uint8 camid, uint8 entry,
                                             uint8 stream, uint8 mask);

int soc_sbx_caladan3_ppe_camram_entry_stream_get(int unit, uint8 camid, uint8 entry,
                                             uint8 *stream, uint8 *mask);

int soc_sbx_caladan3_ppe_camram_entry_keeptimestamp_enable(int unit, uint8 camid,
                                             uint8 entry, uint8 keep);

int soc_sbx_caladan3_ppe_camram_entry_keeptimestamp_get(int unit, uint8 camid,
                                             uint8 entry, uint8 *keep);

int soc_sbx_caladan3_ppe_camram_entry_keepprocreg_enable(int unit, uint8 camid,
                                             uint8 entry, uint8 keep);

int soc_sbx_caladan3_ppe_camram_entry_keepprocreg_get(int unit, uint8 camid,
                                             uint8 entry, uint8 *keep);

int soc_sbx_caladan3_ppe_camram_entry_keeprepdata_enable(int unit, uint8 camid, 
                                             uint8 entry, uint8 keep);

int soc_sbx_caladan3_ppe_camram_entry_keeprepdata_get(int unit, uint8 camid,
                                             uint8 entry, uint8 *keep);

int soc_sbx_caladan3_ppe_camram_entry_hdr_checker_set(int unit, uint8 camid,
                                             uint8 entry,
                                             soc_sbx_caladan3_ppe_hdr_checker_t *checker);

int soc_sbx_caladan3_ppe_camram_entry_hdr_checker_get(int unit, uint8 camid,
                                             uint8 entry,
                                             soc_sbx_caladan3_ppe_hdr_checker_t *checker);

int soc_sbx_caladan3_ppe_camram_entry_hdr_hasher_set(int unit, uint8 camid, uint8 entry,
                                             soc_sbx_caladan3_ppe_hdr_hash_t *hasher);

int soc_sbx_caladan3_ppe_camram_entry_hdr_hasher_get(int unit, uint8 camid,
                                        uint8 entry,
                                        soc_sbx_caladan3_ppe_hdr_hash_t *hasher);

int soc_sbx_caladan3_ppe_camram_entry_property_set(int unit, uint8 camid, uint8 entry,
                                           soc_sbx_caladan3_ppe_prop_entry_t *prop);

int soc_sbx_caladan3_ppe_camram_entry_property_get(int unit, uint8 camid, uint8 entry,
                                           soc_sbx_caladan3_ppe_prop_entry_t *prop);

int soc_sbx_caladan3_ppe_camram_entry_clear(int unit, uint8 camid, uint8 entry);

int soc_sbx_caladan3_ppe_camram_entry_write(int unit, uint8 camid, uint8 entry,
                                            soc_sbx_caladan3_ppe_camram_t *camram);

int soc_sbx_caladan3_ppe_camram_entry_read(int unit, uint8 camid, uint8 entry,
                                soc_sbx_caladan3_ppe_camram_t *camram);

/* TCAM */
int soc_sbx_caladan3_ppe_tcam_entry_src_rep_set(int unit, uint8 camid, uint8 entry,
                                                uint8 src_type,
                                                uint8 rep_data);

int soc_sbx_caladan3_ppe_tcam_entry_src_rep_get(int unit, uint8 camid, uint8 entry,
                                                uint8 *src_type, 
                                                uint8 *rep_data);

int soc_sbx_caladan3_ppe_tcam_entry_property_set(int unit, uint8 camid, uint8 entry, 
                                                 uint8 property);

int soc_sbx_caladan3_ppe_tcam_entry_property_get(int unit, uint8 camid, uint8 entry, 
                                                 uint8 *property, uint8 *property_mask);

int soc_sbx_caladan3_ppe_tcam_entry_state_set(int unit, uint8 camid, uint8 entry,
                                              soc_sbx_caladan3_ppe_state_t state,
                                              soc_sbx_caladan3_ppe_state_t mask);

int soc_sbx_caladan3_ppe_tcam_entry_state_get(int unit, uint8 camid, uint8 entry,
                                              soc_sbx_caladan3_ppe_state_t state,
                                              soc_sbx_caladan3_ppe_state_t mask);

int soc_sbx_caladan3_ppe_tcam_entry_clear(int unit, uint8 camid, uint8 entry);


int soc_sbx_caladan3_ppe_tcam_entry_valid_set(int unit, uint8 camid, 
                                              uint8 entry, uint8 valid);

int soc_sbx_caladan3_ppe_tcam_entry_valid_get(int unit, uint8 camid,
                                              uint8 entry, uint8 *valid);

int soc_sbx_caladan3_ppe_tcam_entry_program(int unit, uint8 camid, uint8 entry);

int soc_sbx_caladan3_ppe_tcam_entry_destrory(int unit, uint8 camid, uint8 entry);

int soc_sbx_caladan3_ppe_tcam_entry_write(int unit, uint8 camid, uint8 entry,
                                          soc_sbx_caladan3_ppe_tcamdata_t *cam, uint8 valid);

int soc_sbx_caladan3_ppe_tcam_entry_read(int unit, uint8 camid, uint8 entry,
                                soc_sbx_caladan3_ppe_tcamdata_t *cam, uint8 *valid);

/* PPE Assembler Config */

int soc_sbx_caladan3_ppe_exception_stream_set(int unit, uint8 stream, uint8 enable);
int soc_sbx_caladan3_ppe_exception_stream_get(int unit, uint8 *stream, uint8 *enable);

int soc_sbx_caladan3_ppe_exception_stream_enable_set(int unit, uint8 enable);
int soc_sbx_caladan3_ppe_exception_stream_enable_get(int unit, uint8 *enable);

int soc_sbx_caladan3_ppe_keep_replication_for_copies_set(int unit, uint8 enable);
int soc_sbx_caladan3_ppe_keep_replication_for_copies_get(int unit, uint8 *enable);

int soc_sbx_caladan3_ppe_load_seq_number_set(int unit, uint8 enable);
int soc_sbx_caladan3_ppe_load_seq_number_get(int unit, uint8 *enable);

int soc_sbx_caladan3_ppe_proc_reg_set(int unit, uint32 proc_reg);
int soc_sbx_caladan3_ppe_proc_reg_get(int unit, uint32 *proc_reg);
int soc_sbx_caladan3_ppe_hc_control(int unit, int on, int clear,
                                int queue, int stream, int var, int varmask);
int soc_sbx_caladan3_ppe_hd(int unit, int parsed, uint32 *header);

int soc_sbx_caladan3_ppe_driver_init(int unit) ;
int soc_sbx_caladan3_ppe_driver_uninit(int unit);

int soc_sbx_caladan3_ppe_cam_trace_dump(int unit);
int soc_sbx_caladan3_ppe_cam_trace_set(int unit, int enable, 
                                       int queue, int mode);
void
soc_sbx_caladan3_ppe_cam_dump(int unit, int cam, int entry);

/* PPE broadshield */
extern int soc_sbx_caladan3_ppe_broad_shield_check_set(int unit, int type, uint8 enable);
extern int soc_sbx_caladan3_ppe_broad_shield_check_get(int unit, int type, uint8 *enable);

/* warm boot */
extern int ppe_config_size_get(int unit);
extern soc_sbx_caladan3_ppe_config_t *ppe_config_get(int unit);
extern int ppe_signature_size_get(int unit);
extern unsigned char  *ppe_signature_get(int unit);

#endif /* PPE_DRIVER */
#endif /* BCM_CALADAN3_SUPPORT */
