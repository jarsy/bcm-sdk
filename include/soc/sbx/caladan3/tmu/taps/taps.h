/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: taps.h,v 1.54.14.6 Broadcom SDK $
 *
 * TAPS defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TMU_TAPS_H_
#define _SBX_CALADN3_TMU_TAPS_H_

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/tmu/taps/trie.h>
#include <soc/sbx/caladan3/tmu/taps/work_queue.h>
#include <soc/sbx/sbDq.h>
#include <shared/bitop.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/caladan3/hpcm.h>

typedef struct taps_tcam_s taps_tcam_t,  *taps_tcam_handle_t;
typedef struct taps_dbucket_s taps_dbucket_t, *taps_dbucket_handle_t;
typedef struct taps_sbucket_s taps_sbucket_t, *taps_sbucket_handle_t;
typedef struct taps_spivot_s taps_spivot_t, *taps_spivot_handle_t;
typedef struct taps_dprefix_s taps_dprefix_t, *taps_dprefix_handle_t;

extern int taps_used_as_em;

/* Every chunk of memory pool is 100K */
#define TAPS_MEM_POOL_CHUNK_SIZE (100 * 1024)

#define _TAPS_INV_ID_ (0xFFFFFFFF)
#define _TAPS_INVALIDATE_PFX_LEN_ (0xFF)
#define _TAPS_PAYLOAD_SIZE_BITS_ (119)
#define _TAPS_ONCHIP_MODE_PAYLOAD_SIZE_BITS_ (17)
#define _TAPS_PAYLOAD_WORDS_ BITS2WORDS(_TAPS_PAYLOAD_SIZE_BITS_)
#define _TAPS_MAX_SEGMENT_ (8)

/* loop up key size */
#define TAPS_IPV4_MAX_VRF_SIZE (16)
#define TAPS_IPV4_PFX_SIZE (32)
#define TAPS_IPV4_KEY_SIZE (TAPS_IPV4_MAX_VRF_SIZE + TAPS_IPV4_PFX_SIZE)
#define TAPS_IPV4_KEY_SIZE_WORDS (((TAPS_IPV4_KEY_SIZE)+31)/32)

#define TAPS_IPV6_MAX_VRF_SIZE (16)
#define TAPS_IPV6_PFX_SIZE (128)
#define TAPS_IPV6_KEY_SIZE (TAPS_IPV6_MAX_VRF_SIZE + TAPS_IPV6_PFX_SIZE)
#define TAPS_IPV6_KEY_SIZE_WORDS (((TAPS_IPV6_KEY_SIZE)+31)/32)
     
#define TAPS_MAX_KEY_SIZE       (TAPS_IPV6_KEY_SIZE)
#define TAPS_MAX_KEY_SIZE_WORDS (((TAPS_MAX_KEY_SIZE)+31)/32)

#define TAPS_32BITS_KEY_SIZE (32)
#define TAPS_64BITS_KEY_SIZE (64)
#define TAPS_96BITS_KEY_SIZE (96)

typedef enum taps_key_type_e {
    TAPS_IPV4_KEY_TYPE,
    TAPS_IPV6_KEY_TYPE,
    TAPS_MAX_KEY_TYPE
} taps_key_type_e_t;

#define _TAPS_SUPPORTED_KEY_TYPE_(type) \
  (((type) == TAPS_IPV4_KEY_TYPE) || ((type) == TAPS_IPV6_KEY_TYPE))

#define _TAPS_KEY_IPV4(taps) \
             ((taps)->param.key_attr.type == TAPS_IPV4_KEY_TYPE)

#define _TAPS_KEY_IPV6(taps) \
             ((taps)->param.key_attr.type == TAPS_IPV6_KEY_TYPE)

#define _TAPS_SUPPORTED_KEY_SIZE_(size) \
  (((size) == TAPS_IPV4_KEY_SIZE) || ((size) == TAPS_IPV6_KEY_SIZE))

#define TAPS_PADDING_BITS(taps) ((taps)->param.key_attr.lookup_length - (taps)->param.key_attr.length)

/* Return TRUE if unit is master unit && the taps table is shared*/
#define _IS_MASTER_SHARE_LPM_TABLE(unit, master_unit, is_share_table) \
    ((is_share_table) && (unit == master_unit))

/* Return TRUE if unit is slave unit && the taps table is shared*/
#define _IS_SLAVE_SHARE_LPM_TABLE(unit, master_unit, is_share_table) \
    ((is_share_table) && (master_unit >= 0) && (unit != master_unit))

/* Redist bucket if the prefix is less than 5% of the bucke size */
#define TAPS_SBUCKET_DIST_THRESHOLD(sbucket_size) (((sbucket_size)>=20)?((sbucket_size)/20):1)
#define TAPS_DBUCKET_DIST_THRESHOLD(dbucket_size) (((dbucket_size)>=20)?((dbucket_size)/20):1)

#define SOC_TAPS_SHARE_CACHE_MAX_UNIT_NUM (SOC_MAX_NUM_DEVICES + 1)

/* Define a unit number to cache cmds for share taps table 
     It should equal to the max units number */
#define SOC_TAPS_SHARE_CACHE_UNIT SOC_MAX_NUM_DEVICES

/* Currently, just support 2 units share taps table */
#define SOC_NUM_SHARE_TAPS 2

#define SOC_MAX_NUM_SLAVE (SOC_NUM_SHARE_TAPS - 1)

typedef struct taps_key_attr_s {
    taps_key_type_e_t type;
    /* length of look up key in hardware, 48bits or 144 bits */
    unsigned int      lookup_length;
    /* valid length of the whole key */
    unsigned int      length;
    /* for routing, length of vrf within key 
     * max VRF supported is 16bits */
    unsigned int      vrf_length;
} taps_key_attr_t;

typedef enum taps_instance_e {
    TAPS_INST_0, /* parallel mode using only taps instance 0 */
    TAPS_INST_1, /* parallel mode using only taps instance 1 */
    TAPS_INST_SEQ, /* sequential mode using both taps 0 & 1 instance - not supported */
    TAPS_INST_MAX
} taps_instance_e_t;

#define SW_INST_CONVERT_TO_HW_INST(instance) ((instance) + 1)

/* supported instance */
#define _TAPS_SUPPORTED_INSTANCE_(inst) \
  ((inst) == TAPS_INST_0 || (inst) == TAPS_INST_1 || (inst) == TAPS_INST_SEQ)

#define _TAPS_IS_PARALLEL_MODE_(inst) \
  ((inst) == TAPS_INST_0 || (inst) == TAPS_INST_1)

#define ceiling_ratio (10)

#define _TAPS_SEG_DIVIDE_RATIO_CHECK(divide_ratio) ((divide_ratio) <= ceiling_ratio)

typedef struct taps_instance_segment_attr_s {
    /* offset are based on TAPS_TCAM_SINGLE_ENTRY format.
     * minimum hardware requirement is 4-QUAD entry or 16 SINGLE entry */
    unsigned int  offset;
    unsigned int  num_entry;      
} taps_instance_segment_attr_t;

typedef struct taps_segment_attr_s {
    taps_instance_segment_attr_t seginfo[TAPS_INST_MAX];
} taps_segment_attr_t;

typedef enum taps_tcam_layout_e {
    TAPS_TCAM_SINGLE_ENTRY,
    TAPS_TCAM_DOUBLE_ENTRY,
    TAPS_TCAM_QUAD_ENTRY,
    TAPS_MAX_TCAM_ENTRY
} taps_tcam_layout_e_t;

#define _TAPS_SUPPORTED_TCAM_LAYOUT_(layout) \
  ((layout) == TAPS_TCAM_QUAD_ENTRY)

#define _TAPS_MIN_TCAM_ENTRY_SEG_ (16)

#define _MAX_SBUCKET_PIVOT_PER_BB_ (72)

#define _MAX_SBUCKET_PIVOT_ (_MAX_SBUCKET_PIVOT_PER_BB_ * (1 << TAPS_TCAM_QUAD_ENTRY))

/* To support up to 72 prefixes in a sbucket, need to key_length in SRAM have at lease 7 bits*/
#define _MIN_SBUCKET_KEY_LEN_ (7)

#define _BB_ENTRY_SIZE_ (16) 

#define _BASE_UNIT_BITS_ (_MAX_SBUCKET_PIVOT_PER_BB_ * TAPS_32BITS_KEY_SIZE)

#define _BASE_UNIT_NUM_PER_BB_ (6)
#define BB_PREFIX_NUMBER(format) (_BASE_UNIT_BITS_ / (_BB_ENTRY_SIZE_ * (format)))

#define BASE_UNIT_PREFIX_NUMBER(format) ((BB_PREFIX_NUMBER(format)) / (_BASE_UNIT_NUM_PER_BB_))
typedef enum _soc_sbx_caladan3_tmu_taps_bb_format_type_e {
    SOC_SBX_TMU_TAPS_BB_FORMAT_2ENTRIES = 2,
    SOC_SBX_TMU_TAPS_BB_FORMAT_3ENTRIES = 3,
    SOC_SBX_TMU_TAPS_BB_FORMAT_4ENTRIES = 4,
    SOC_SBX_TMU_TAPS_BB_FORMAT_6ENTRIES = 6,
    SOC_SBX_TMU_TAPS_BB_FORMAT_8ENTRIES = 8,
    SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES = 12
} soc_sbx_caladan3_tmu_taps_bb_format_type_t; 

#define _TAPS_SUPPORTED_BBX_FORMAT_(format)           \
  ((format) == SOC_SBX_TMU_TAPS_BB_FORMAT_2ENTRIES || \
   (format) == SOC_SBX_TMU_TAPS_BB_FORMAT_3ENTRIES || \
   (format) == SOC_SBX_TMU_TAPS_BB_FORMAT_4ENTRIES || \
   (format) == SOC_SBX_TMU_TAPS_BB_FORMAT_6ENTRIES || \
   (format) == SOC_SBX_TMU_TAPS_BB_FORMAT_8ENTRIES || \
   (format) == SOC_SBX_TMU_TAPS_BB_FORMAT_12ENTRIES)

typedef struct taps_sbucket_attr_s {
    soc_sbx_caladan3_tmu_taps_bb_format_type_t format;
    uint32 max_pivot_number;
} taps_sbucket_attr_t;

/* ipv4 supports 7n or 7n+3. allow up to 63 */
#define _TAPS_SUPPORTED_IPV4_NUM_DDR_PFX_(keytype,num) \
  (((keytype) == TAPS_IPV4_KEY_TYPE) && (((num)%7 == 0)  || ((num)%7 == 3))&& ((num) <= 63))


/* ipv6 supports 5n or 5n+2. allow up to 62 */
#define _TAPS_SUPPORTED_IPV6_NUM_DDR_PFX_(keytype,num) \
  (((keytype) == TAPS_IPV6_KEY_TYPE) && (((num)%5 == 0) || ((num)%5 == 2)) && ((num) <= 62))

typedef enum taps_ddr_table_type_e_s {
    TAPS_DDR_PAYLOAD_TABLE=0,
    TAPS_DDR_PREFIX_TABLE,
    TAPS_DDR_TABLE_MAX
} taps_ddr_table_type_e_t;

#define _TAPS_DBUCKET_IPV4_128BIT_DBKT_NUM_PFX (3)
#define _TAPS_DBUCKET_IPV4_256BIT_DBKT_NUM_PFX (7)
#define _TAPS_DBUCKET_IPV6_128BIT_DBKT_NUM_PFX (2)
#define _TAPS_DBUCKET_IPV6_256BIT_DBKT_NUM_PFX (5)

#define _TAPS_DDR_V4_PAYLOAD_SIZE (128)
#define _TAPS_DDR_V6_PAYLOAD_SIZE (256)
#define _TAPS_DBUCKET_BSEL_MULTIPLY_FACTOR (2) /* 0*&1* bucket */

typedef enum taps_error_watch_point_onchip_e_s {
    INS_ST_SBKT_PIVOT_ONCHIP_WP_1 = 0,
    INS_ST_SBKT_PIVOT_ONCHIP_WP_2,
    INS_ST_SBKT_PIVOT_ONCHIP_WP_3,
    INS_ST_SBKT_PIVOT_ONCHIP_WP_4,
    INS_ST_DOMAIN_SPLIT_ONCHIP_WP_1,
    INS_ST_DOMAIN_SPLIT_ONCHIP_WP_2,
    INS_ST_DOMAIN_PROPAGATE_SPLIT_ONCHIP_WP_1,
    INS_ST_DOMAIN_PROPAGATE_SPLIT_ONCHIP_WP_2,
    INS_ST_TCAM_PIVOT_ONCHIP_WP_1,
    INS_ST_TCAM_PIVOT_ONCHIP_WP_2,
    INS_ST_TCAM_PIVOT_ONCHIP_MODE0_WP_1,
    INS_ST_TCAM_PIVOT_ONCHIP_MODE0_WP_2,
    INS_ST_ONCHIP_WP_MAX
} taps_error_watch_point_onchip_e_t;

typedef enum taps_error_watch_point_e_s {
    INS_ST_DBKT_PFX_WP_1 = 0,
    INS_ST_DBKT_PFX_WP_2,
    INS_ST_DBKT_PROPAGATE_WP_1,
    INS_ST_DBKT_PROPAGATE_WP_2,
    INS_ST_DBKT_PROPAGATE_WP_3,
    INS_ST_DBKT_SPLIT_WP_1,
    INS_ST_DBKT_SPLIT_WP_2,
    INS_ST_DBKT_SPLIT_WP_3,
    INS_ST_SBKT_PIVOT_WP_1,
    INS_ST_SBKT_PIVOT_WP_2,
    INS_ST_DBKT_PROPAGATE_SPLIT_WP_1,
    INS_ST_DBKT_PROPAGATE_SPLIT_WP_2,
    INS_ST_DOMAIN_SPLIT_WP_1,
    INS_ST_DOMAIN_SPLIT_WP_2,
    INS_ST_DOMAIN_SPLIT_WP_3,
    INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_1,
    INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_2,
    INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_3,
    INS_ST_DOMAIN_PROPAGATE_SPLIT_WP_4,
    INS_ST_TCAM_PIVOT_WP_1,
    INS_ST_TCAM_PIVOT_WP_2,
    INS_ST_WP_MAX
} taps_error_watch_point_e_t;

/* #define TAPS_ERROR_HANDLING_UNIT_TEST */
#ifdef TAPS_ERROR_HANDLING_UNIT_TEST
#define TAPS_ERROR_WATCH_POINT_MAXNUM 100
extern int watch_point[TAPS_ERROR_WATCH_POINT_MAXNUM];
#define TAPS_ERROR_WATCH_POINT(num, rv) do { \
                                            if (watch_point[num]) rv = SOC_E_FAIL; \
                                        } while (0)
#else
#define TAPS_ERROR_WATCH_POINT(num, rv)  
#endif

/* hardware implementation notes on dram bucket:
 * IPV4:
 * hardware suports buckets of 128b (3-31bit prefix bucket) or 
 * 256b-(7-31bit prefix bucket). Additional combination are build
 * by concatinating these combination. 
 * The combination is strict. Units of 256b multiple must always use
 * 256bit bucket and only 128b bucket could be concatenated at end.
 * Eg., if 384bit bucket is required, the only legal combination is 
 * 2x256bit + 1x128bit
 * IPV6:
 * hardware supports (2-47 bit psig) or (5-47 bit psig_ buckets
 * Number of dram bucket#
 * Hardware Notes: Hardware supports 0* & 1* buckets. So logically
 * buckets are split into 0/1 buckets and bucket sel assumed from key
 * is used to select one of them. Number of DRAM bucket is implicitly 
 * assumed from SRAM pivot & TCAM domain size. */
typedef struct taps_dbucket_attr_s {
    /* max number of prefix on dram bucket.
     * must be buildable by combination of hardware supported sizes.
     * refer to notes above */
    int num_dbucket_pfx;
    /* capacity of ddr table is num_dbucket_pfx * num_dbucket */
    /* reserve the table with provided id */   
    /* tmu ddr dm table id's */
    uint8 flags[TAPS_DDR_TABLE_MAX];
    uint8 table_id[TAPS_DDR_TABLE_MAX];
} taps_dbucket_attr_t;

#define _TAPS_DDR_TBL_FLAG_NONE_   (0x0)
#define _TAPS_DDR_TBL_FLAG_WITH_ID (0x1)
#define _TAPS_SUPPORTED_DDR_TBL_FLAGS_(flag) \
 ((flag) == _TAPS_DDR_TBL_FLAG_WITH_ID || \
  (flag) == _TAPS_DDR_TBL_FLAG_NONE_)

typedef enum taps_search_mode_e {
    TAPS_ONCHIP_ALL, /* returns a FIB pointer from on chip lookup */
    TAPS_ONCHIP_SEARCH_OFFCHIP_ADS,/* returns ADS from DDR3 & searches on chip only */
    TAPS_OFFCHIP_ALL,  /* uses DDR3 bucket & does 3 level search */
    TAPS_MAX_SEARCH_MODE
} taps_search_mode_e_t;

/* supported instance */
#define _TAPS_SUPPORTED_SEARCH_MODE_(mode) \
  ((mode) < TAPS_MAX_SEARCH_MODE)

typedef struct taps_init_params_s {
    taps_instance_e_t    instance; /* taps instance 0|1 */
    /* key property */
    taps_key_attr_t      key_attr;
    /* segment properties */
    taps_segment_attr_t  seg_attr;
    /* tcam layout */
    taps_tcam_layout_e_t tcam_layout;
    taps_sbucket_attr_t  sbucket_attr;
    taps_dbucket_attr_t  dbucket_attr;
    taps_search_mode_e_t mode;
    /* divide_ratio just uses for unified mode,
        * It decides how many entries allocated in TAPS0, the max value is 10.
        * If set to 0, It means no entries in TAPS0,
       *  If set to 10, It means no entries in TAPS1.
       */
    uint8         divide_ratio; 
    uint8         host_share_table;
    int max_capacity_limit;
    /* default entry * nhop information */
    unsigned int *defpayload;
} taps_init_params_t;

#define _DISABLE_LIMIT_ (-1)
#define TAPS_CAPACITY_LIMIT_DISABLED(param) \
 ((param)->max_capacity_limit == _DISABLE_LIMIT_)

#define TAPS_ERROR_COUNT
#define TAPS_V6_COLLISION_COUNT

typedef struct taps_s {
    dq_t taps_list_node;
    taps_init_params_t param;  
    uint8 hwinstance;
#define _TAPS_DEFAULT_WGROUP_ (1)
    taps_wq_handle_t wqueue[SOC_MAX_NUM_DEVICES]; /* work queue manager */
    int capacity; /* number of prefix on this TAPS FIB instance */
    /* Tcam database handle */
    taps_tcam_handle_t tcam_hdl;
    /* domain allocator */
    SHR_BITDCL  *allocator;
    /* default entry * nhop information */
    unsigned int *defpayload;
    /* segment id -tbd verify if segment id can be different on 2 instance */
    unsigned int segment;
    int8         master_unit;
    uint8        num_slaves;
    int8         slave_units[2];
#ifdef TAPS_ERROR_COUNT
    uint32       error_count;
#endif
#ifdef TAPS_V6_COLLISION_COUNT
    uint32       v6_collision_count;
#endif

    /* add more .. */
    /* mutex?? */
    sal_mutex_t taps_mutex;    
} taps_t, *taps_handle_t;

typedef enum _taps_bucket_stat_e_s {
    _TAPS_BKT_EMPTY,
    _TAPS_BKT_FULL,
    _TAPS_BKT_VACANCY,
    _TAPS_BKT_WILD,        /* only wild pivot exists */
    _TAPS_BKT_ERROR,
    _TAPS_BKT_ALMOST_FULL, /* 1 from FULL */
    _TAPS_BKT_MAX
} taps_bucket_stat_e_t;


typedef struct _taps_container_s {
    dq_t taps_object_list;
    /* segment allocator */
    uint32 seg_allocator[TAPS_INST_MAX];
    /* mutex */
} taps_container_t; 

typedef struct _taps_collision_dbucket_find_s {
    int unit;
    taps_handle_t taps;
    taps_dbucket_handle_t dbh;
    uint32 *key;
    int domain_id;
    int dbucket_id;
    int dph_idx;
} taps_collision_dbucket_find_t; 

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_taps_driver_init
 * Purpose:
 *     Bring up TAPS drivers
 */
extern int soc_sbx_caladan3_tmu_taps_driver_init(int unit);

/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_taps_driver_uninit
 * Purpose:
 *     Cleanup
 */
extern int soc_sbx_caladan3_tmu_taps_driver_uninit(int unit);


/*
 *
 * Function:
 *     soc_sbx_caladan3_tmu_taps_hw_init
 * Purpose:
 *     Bring up TAPS drivers
 */
extern int soc_sbx_caladan3_tmu_taps_hw_init(int unit);

/*
 *
 * Function:
 *     taps_insert_default_entry_for_slave_unit
 * Purpose:
 *     Insert default entry(0/0) in the LPM table of slave unit. 
 *	   The taps handle have created by the master unit
 */
extern int taps_insert_default_entry_for_slave_unit(int unit, taps_handle_t taps);

/*
 *
 * Function:
 *     taps_create
 * Purpose:
 *     allocate a taps instance
 */
extern int taps_create(int unit, taps_init_params_t *param, taps_handle_t *handle);

/*
 *
 * Function:
 *     taps_destroy
 * Purpose:
 *     destroys taps instance
 */

extern int taps_destroy(int unit, taps_handle_t handle);

/*
 *
 * Function:
 *     taps_domain_alloc
 * Purpose:
 *     allocate a sram bucket or tcam domain 
 */
extern int taps_domain_alloc(int unit,
                             taps_handle_t handle, 
                             unsigned int *domain_id);

/*
 *
 * Function:
 *     taps_sbucket_free
 * Purpose:
 *     free a sram bucket or tcam domain 
 */
extern int taps_domain_free(int unit,
                            taps_handle_t handle,
                            unsigned int domain_id);

typedef struct taps_arg_s {
    taps_handle_t taps;
    unsigned int  *key;
    unsigned int  length;
    uint32        *payload;
    void          *cookie;
} taps_arg_t;

/*
 *
 * Function:
 *     taps_insert_route
 * Purpose:
 *     insert a route into taps
 */
extern int taps_insert_route(int unit, taps_arg_t *arg);

/*
 *
 * Function:
 *     taps_delete_route
 * Purpose:
 *     delete a route from taps
 */
extern int taps_delete_route(int unit, taps_arg_t *arg);

/*
 *
 * Function:
 *     taps_get_route
 * Purpose:
 *     get a route information
 */
extern int taps_get_route(int unit, taps_arg_t *arg);

/*
 *
 * Function:
 *     taps_update_route
 * Purpose:
 *     update a route in taps
 */
extern int taps_update_route(int unit, taps_arg_t *arg);

/*
 *
 * Function:
 *     taps_find_prefix_global_pointer
 * Purpose:
 *     given a prefix return the global bucket pointer
 */
extern int taps_find_prefix_global_pointer(int unit, 
                                           taps_handle_t taps,
                                           uint32 *key, uint32 length,
										   uint32 *bpm_global_index);

/*
 *
 * Function:
 *     taps_find_prefix_local_pointer
 * Purpose:
 *     given a prefix return the local bucket pointer
 */
extern int taps_find_prefix_local_pointer(int unit, 
                                          taps_handle_t taps,
                                          uint32 *key, uint32 length,
                                          uint32 *bpm_local_index);

/*
 *
 * Function:
 *     taps_calculate_prefix_local_pointer
 * Purpose:
 *     given sph/dph, calculate (without searching) the local bucket pointer
 */
extern int taps_calculate_prefix_local_pointer(int unit, 
                           taps_handle_t taps,
                           taps_spivot_handle_t sph,
                           taps_dprefix_handle_t dph,
                           uint32 *bpm_local_index);
/*
* Fuction : 
*        taps_get_lpm_route
* Purpose:
*        Get the lpm of the input route, return the lpm route's key & length & payload
*/
extern int taps_get_lpm_route(int unit, taps_arg_t *arg, 
                           uint32 *out_bpm_key, uint32 *out_bpm_length);

/*
 *
 * Function:
 *     taps_find_bpm
 * Purpose:
 *     find best prefix match for given route
 */
extern int taps_find_bpm(int unit, taps_arg_t *arg,
                         unsigned int *bpm_length);


typedef enum _taps_work_commit_model_e {
    _TAPS_SEQ_COMMIT=0,
    _TAPS_BULK_COMMIT=1,
    _TAPS_COMMIT_MAX
} taps_work_commit_model_e_t;

#define _TAPS_VALID_COMMIT(commit) \
    ((commit) >= _TAPS_SEQ_COMMIT && \
     (commit) < _TAPS_COMMIT_MAX)

/*
 *
 * Function:
 *     taps_work_commit
 * Purpose:
 *     issues bulk/sequential commit & process response all issued command
 */
extern int taps_work_commit(int unit, 
                            taps_wgroup_handle_t wgroup,
                            taps_work_type_e_t *work_type,
                            uint32 work_type_count,
                            taps_work_commit_model_e_t commit);

/*
 *
 * Function:
 *     taps_free_work_queue
 * Purpose:
 *     flushes out entire work queue. Does not commit it to hardware,
 *     rather frees up all heap resources
 */
extern int taps_free_work_queue(int unit, 
                                taps_wgroup_handle_t wgroup,
                                taps_work_type_e_t *work_type,
                                uint32 work_type_count);


/* Taps dump flags */
/* dump overview of tcam pivot bucket */
#define TAPS_DUMP_TCAM_SW_BKT         (1<<0)
/* dump overview of specified pivots within bucket */
#define TAPS_DUMP_TCAM_SW_PIVOT       (1<<1)
/* dump tcam entire pivot bucket from hardware */
#define TAPS_DUMP_TCAM_HW_BKT         (1<<2)
/* dump given pivot from hardware */
#define TAPS_DUMP_TCAM_HW_PIVOT       (1<<3)
/* provides verbose information when ored with any flags */
#define TAPS_DUMP_TCAM_VERB           (1<<4)
#define TAPS_DUMP_TCAM_ALL            (TAPS_DUMP_TCAM_SW_BKT   |\
                       TAPS_DUMP_TCAM_SW_PIVOT |\
                       TAPS_DUMP_TCAM_HW_BKT   |\
                       TAPS_DUMP_TCAM_HW_PIVOT |\
                       TAPS_DUMP_TCAM_VERB)
#define TAPS_DUMP_TCAM_FLAGS(flag) ((flag)&\
                                    (TAPS_DUMP_TCAM_SW_BKT|\
                                     TAPS_DUMP_TCAM_SW_PIVOT|\
                                     TAPS_DUMP_TCAM_HW_BKT|\
                                     TAPS_DUMP_TCAM_HW_PIVOT|\
                                     TAPS_DUMP_TCAM_VERB))

#define TAPS_DUMP_SRAM_SW_BKT         (1<<5)
#define TAPS_DUMP_SRAM_SW_PIVOT       (1<<6)
#define TAPS_DUMP_SRAM_HW_BKT         (1<<7)
#define TAPS_DUMP_SRAM_HW_PIVOT       (1<<8)
#define TAPS_DUMP_SRAM_VERB           (1<<9)
#define TAPS_DUMP_SRAM_ALL            (TAPS_DUMP_SRAM_SW_BKT  |\
                       TAPS_DUMP_SRAM_SW_PIVOT|\
                       TAPS_DUMP_SRAM_HW_BKT  |\
                       TAPS_DUMP_SRAM_HW_PIVOT|\
                       TAPS_DUMP_SRAM_VERB)
#define TAPS_DUMP_SRAM_FLAGS(flag) ((flag)&\
                                    (TAPS_DUMP_SRAM_SW_BKT|\
                                     TAPS_DUMP_SRAM_SW_PIVOT|\
                                     TAPS_DUMP_SRAM_HW_BKT|\
                                     TAPS_DUMP_SRAM_HW_PIVOT|\
                                     TAPS_DUMP_SRAM_VERB))

#define TAPS_DUMP_DRAM_SW_BKT         (1<<10)
#define TAPS_DUMP_DRAM_SW_PFX         (1<<11)
#define TAPS_DUMP_DRAM_HW_BKT         (1<<12)
#define TAPS_DUMP_DRAM_HW_PFX         (1<<13)
#define TAPS_DUMP_DRAM_VERB           (1<<14)
#define TAPS_DUMP_DRAM_ALL            (TAPS_DUMP_DRAM_SW_BKT |\
                       TAPS_DUMP_DRAM_SW_PFX |\
                       TAPS_DUMP_DRAM_HW_BKT |\
                       TAPS_DUMP_DRAM_HW_PFX |\
                       TAPS_DUMP_DRAM_VERB)
#define TAPS_DUMP_DRAM_FLAGS(flag) ((flag)&\
                                    (TAPS_DUMP_DRAM_SW_BKT|\
                                     TAPS_DUMP_DRAM_SW_PFX|\
                                     TAPS_DUMP_DRAM_HW_BKT|\
                                     TAPS_DUMP_DRAM_HW_PFX|\
                                     TAPS_DUMP_DRAM_VERB))

#define TAPS_DUMP_MAX_FLAG_BIT        (15)            
#define TAPS_DUMP_FLAG_VALID(flag)    ((flag) < (1<<TAPS_DUMP_MAX_FLAG_BIT))


extern int taps_dump(int unit, taps_handle_t handle, uint32 flags);

extern void taps_set_caching(int unit, int cache);
extern int taps_get_caching(int unit);

extern int taps_share_flush_cache(unsigned int unit) ;

extern int taps_flush_cache(unsigned int unit);

extern int taps_host_flush_cache(unsigned int unit, int host_share) ;

extern int taps_prefix_search_and_dump(int unit, taps_arg_t *arg);

extern int taps_bucket_next_node_get(trie_t *trie, trie_type_t trie_type,
                                        uint32 *bit_map, int start_index,
                                        int max_node_num, trie_node_t **next_node); 

extern int taps_iterator_first(int unit, taps_arg_t *arg, uint32 *key, uint32 *key_length);

extern int taps_iterator_next(int unit, taps_arg_t *arg, uint32 *key, uint32 *key_length); 

extern int taps_instance_and_entry_id_get(int unit, const taps_handle_t taps, 
                                int tcam_entry, int sbucket_id,
                                int *hwinstance, int *hw_tcam_entry, int *hw_sbucket_id);

extern int taps_domain_move(int unit, taps_tcam_handle_t handle, taps_wgroup_handle_t *wgroup, 
                        int old_tcam_entry, int new_tcam_entry);

/*
 *
 * Function:
 *   taps_work_group_create_for_all_devices
 * Purpose:
 *   creates a serial taps work group for all devices which share this taps
 */
extern int taps_work_group_create_for_all_devices(int unit, 
                           taps_handle_t taps,
                           taps_wq_handle_t *work_queue, 
                           unsigned int wgroup,
                           int host_share_table,
                           taps_wgroup_handle_t *work_group_handle);

/*
 *
 * Function:
 *   taps_work_group_destroy_for_all_devices
 * Purpose:
 *   destroy an array of taps work group
 */
extern int taps_work_group_destroy_for_all_devices(int unit, taps_handle_t taps,
                           taps_wgroup_handle_t *wghdl, int host_share_table);

/*
 *   Function
 *      taps_command_enqueue_for_slave_unit
 *   Purpose
 *      Command enqueue for slave unit. 
 */
extern int taps_command_enqueue_for_slave_unit(int unit, taps_handle_t taps,
                             const taps_wgroup_handle_t *wgroup, 
                             taps_work_type_e_t type,
                             soc_sbx_caladan3_tmu_cmd_t *master_cmd); 

/*
 *   Function
 *      taps_command_enqueue_for_all_devices
 *   Purpose
 *      Command enqueue for master and slave unit. 
 */
extern int taps_command_enqueue_for_all_devices(int unit, taps_handle_t taps,
                            const taps_wgroup_handle_t *wgroup, 
                            taps_work_type_e_t type,
                            soc_sbx_caladan3_tmu_cmd_t *cmd); 

/*
 *   Function
 *      taps_find_bpm_asso_data
 *   Purpose
 *      Find Best Prefix Match (BPM) length  for a given
 *      key/length in the tcam database. Return the corresponding bpm's payload
 *      Note: Just used by mode zero
 */
extern int taps_find_bpm_asso_data(int unit, taps_tcam_t *handle,
                            uint32 *key, uint32 length, uint32 *bpm_mask, unsigned int **asso_data);

extern int taps_command_destory(int unit, taps_handle_t taps, const taps_wgroup_handle_t *wgroup, taps_work_type_e_t *work_type,
                                uint32 work_type_count);

extern int taps_v6_collision_isr(int unit, int table_num, int entry_num, uint32 *key);

extern int taps_host_bucket_redist(int unit, taps_handle_t taps);

#endif /* _SBX_CALADN3_TMU_TAPS_H_ */
