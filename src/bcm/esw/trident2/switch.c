/*
 * $Id: switch.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module : Switch 
 * 
 * Purpose: 
 * This file has the flexible hashing apis and its accessory functions to configure
 * field match data and relative hash offsets for bucket 2 and 3.
 * 
 * The below shows the configurations of flexible hashing mechanism in UDF and Flex
 * tables. Data qualifier apis are used to configure the UDF table with relative
 * packet offsets from L4 header. UDF chunk4 - 7 is being used to configure hash
 * bucket 2 and 3. Flexible hashing offset mapping is done with functions
 * implemented in this file.
 * 
 * 
 * 
 * FLEXIBLE HASHING PACKET PARSER (with UDF)
 * 
 *     Packet            L4
 *     +--------+--------+------------+---------+
 *     |        |        | L4 header  | Payload |
 *     +--------+--------+-+-+--------+---------+
 *                 offset  2 2 ....                                      
 *                  ||                                            -----+ 
 *                  ||                                                 | 
 *                           +----Chunk4----+           PKT.field1(16b)| 
 *         PKT.offset1  -->  | +--Chunk5-----+                         | 
 *                           | | +-Chunk6------+    ==> PKT.field2(16b)|
 *     +   PKT.offset2  -->  | | | +-Chunk7------+                    -+ 
 *                           +-| | |             |      PKT.field3(16b)| 
 *         PKT.offset3  ---->  +-| |UDF_TCAM/OFF |                     | 
 *                               +-|  (4 x 16bit)|      PKT.field4(16b)| 
 *         PKT.offset4  ------>    +-------------+                     | 
 *                                                                -----+ 
 *                                                                      
 *                                                                                           
 * FLEXIBLE HASHING OFFSET MAPPING
 *                                                                                               * 
 *                        (64bx8)  [UDF_CONDITIONAL_CHECK_TABLE_CAMm]
 *                  -----+         +--0-------------+                                    
 *        PKT.field1(16b)|          +-|+-2--------------+                                
 *                       |           +-|+-3--------------+                               
 *        PKT.field2(16b)| 64bit       +| +-4--------------+    ==> offset 2
 *                      -+---- Key      +-| +-5--------------+                           
 *        PKT.field3(16b)|                +-| +-6--------------+   
 *                       |                  +-| +-7--------------+                       
 *        PKT.field4(16b)|                    +-|                |                       
 *                       |                      +----------------+                       
 *                  -----+                 [UDF_CONDITIONAL_CHECK_TABLE_RAMm]
 *   
 */

#include <soc/defs.h>

#if defined(BCM_TRIDENT2_SUPPORT)
#include <soc/mem.h>
#include <soc/mcm/memregs.h>
#include <soc/debug.h>
#include <soc/field.h>
#include <bcm/switch.h>

#include <bcm_int/esw/xgs4.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/switch.h>
#include <bcm_int/esw/trident2.h>


/* Flex enable regmem and fields */
static soc_reg_t _bcm_td2_flex_hash_control = RTAG7_HASH_CONTROL_3r;
static soc_field_t _bcm_td2_flex_hash_enable_fields[] = {
    ENABLE_FLEX_FIELD_1_Af,
    ENABLE_FLEX_FIELD_1_Bf,
    ENABLE_FLEX_FIELD_2_Af,
    ENABLE_FLEX_FIELD_2_Bf
};

#ifdef BCM_TOMAHAWK2_SUPPORT
static soc_reg_t _bcm_th2_flex_hash_control = RTAG7_HASH_CONTROL_3_64r;
static soc_field_t _bcm_th2_flex_hash_enable_fields[] = {
    ENABLE_FLEX_FIELD_1_Af,
    ENABLE_FLEX_FIELD_1_Bf,
    ENABLE_FLEX_FIELD_2_Af,
    ENABLE_FLEX_FIELD_2_Bf,
    ENABLE_FLEX_FIELD_3_Af,
    ENABLE_FLEX_FIELD_3_Bf,
    ENABLE_FLEX_FIELD_4_Af,
    ENABLE_FLEX_FIELD_4_Bf,
    ENABLE_FLEX_FIELD_5_Af,
    ENABLE_FLEX_FIELD_5_Bf,
    ENABLE_FLEX_FIELD_6_Af,
    ENABLE_FLEX_FIELD_6_Bf,
    ENABLE_FLEX_FIELD_7_Af,
    ENABLE_FLEX_FIELD_7_Bf,
    ENABLE_FLEX_FIELD_8_Af,
    ENABLE_FLEX_FIELD_8_Bf,
    ENABLE_FLEX_FIELD_9_Af,
    ENABLE_FLEX_FIELD_9_Bf,
    ENABLE_FLEX_FIELD_10_Af,
    ENABLE_FLEX_FIELD_10_Bf,
    ENABLE_FLEX_FIELD_11_Af,
    ENABLE_FLEX_FIELD_11_Bf,
    ENABLE_FLEX_FIELD_12_Af,
    ENABLE_FLEX_FIELD_12_Bf,
    ENABLE_FLEX_FIELD_13_Af,
    ENABLE_FLEX_FIELD_13_Bf
};
#endif /*BCM_TOMAHAWK2_SUPPORT*/

/* register to enable flex hasing scheme */
static soc_reg_t _bcm_td2_ing_hash_config = ING_HASH_CONFIG_0r; 
static soc_field_t _bcm_td2_ing_hash_config_fields[] = {
    ENABLE_FLEX_HASHINGf
};
/* 
 *   TCAM: FIELD_1_MASK, FIELD_2_MASK
 *   FIELD_1_DATA, FIELD_2_DATA

 *   RAM: FIELD_1_MASK, FIELD_2_MASK
 *   FIELD_1_OFFSET, FIELD_2_OFFSET
 */

static soc_field_t _bcm_td2_flex_hash_data[] = {
    FIELD_2_DATAf,
    FIELD_1_DATAf
};

static soc_field_t _bcm_td2_flex_hash_mask[] = {
    FIELD_2_MASKf,
    FIELD_1_MASKf,
    FIELD_3_MASKf,
    FIELD_4_MASKf,
    FIELD_5_MASKf,
    FIELD_6_MASKf,
    FIELD_7_MASKf,
    FIELD_8_MASKf,
    FIELD_9_MASKf,
    FIELD_10_MASKf,
    FIELD_11_MASKf,
    FIELD_12_MASKf,
    FIELD_13_MASKf
};

static soc_field_t _bcm_td2_flex_hash_offset[] = {
    FIELD_2_OFFSETf,
    FIELD_1_OFFSETf
};
#if defined(BCM_TOMAHAWK_SUPPORT)
static soc_field_t _bcm_th_flex_hash_offset_udf[] = {
    UDF_SEL_2f,
    UDF_CHUNK_ID_2f,
    UDF_SEL_1f,
    UDF_CHUNK_ID_1f,
    UDF_SEL_3f,
    UDF_CHUNK_ID_3f,
    UDF_SEL_4f,
    UDF_CHUNK_ID_4f,
    UDF_SEL_5f,
    UDF_CHUNK_ID_5f,
    UDF_SEL_6f,
    UDF_CHUNK_ID_6f,
    UDF_SEL_7f,
    UDF_CHUNK_ID_7f,
    UDF_SEL_8f,
    UDF_CHUNK_ID_8f,
    UDF_SEL_9f,
    UDF_CHUNK_ID_9f,
    UDF_SEL_10f,
    UDF_CHUNK_ID_10f,
    UDF_SEL_11f,
    UDF_CHUNK_ID_11f,
    UDF_SEL_12f,
    UDF_CHUNK_ID_12f,
    UDF_SEL_13f,
    UDF_CHUNK_ID_13f
};
static soc_field_t _bcm_th_flex_hash_offset_type[] = {
    FIELD_2_TYPEf,
    FIELD_1_TYPEf,
    FIELD_3_TYPEf,
    FIELD_4_TYPEf,
    FIELD_5_TYPEf,
    FIELD_6_TYPEf,
    FIELD_7_TYPEf,
    FIELD_8_TYPEf,
    FIELD_9_TYPEf,
    FIELD_10_TYPEf,
    FIELD_11_TYPEf,
    FIELD_12_TYPEf,
    FIELD_13_TYPEf
};
#endif /*BCM_TOMAHAWK_SUPPORT*/
static soc_mem_t _bcm_td2_flex_hash_table[] = {
    UDF_CONDITIONAL_CHECK_TABLE_CAMm,
    UDF_CONDITIONAL_CHECK_TABLE_RAMm
};

#define FLEX_HASH_TCAM (_bcm_td2_flex_hash_table[0])
#define FLEX_HASH_OFFSET (_bcm_td2_flex_hash_table[1])
#define FLEX_HASH_TCAM_ENTRY_T udf_conditional_check_table_cam_entry_t
#define FLEX_HASH_OFFSET_ENTRY_T udf_conditional_check_table_ram_entry_t
#define FLEX_HASH_OFFSET_MASK(_u_) (SOC_IS_TOMAHAWK2(_u_) ? \
                UDF_CONDITIONAL_CHECK_TABLE_RAM_1m : UDF_CONDITIONAL_CHECK_TABLE_RAMm)
#define FLEX_HASH_OFFSET_MASK_ENTRY_T udf_conditional_check_table_ram_1_entry_t

/*
 * Flex Index usage bitmap operations. The 8 indexes of flex tcam table are
 * being managed with index bitmap operations.
 */
#define _BCM_SWITCH_FLEX_IDX_USED_GET(_u_, _idx_) \
        SHR_BITGET(SWITCH_HASH_INFO(_u_)->hash_idx_bitmap, (_idx_))

#define _BCM_SWITCH_FLEX_IDX_USED_SET(_u_, _idx_) \
        SHR_BITSET(SWITCH_HASH_INFO((_u_))->hash_idx_bitmap, (_idx_))

#define _BCM_SWITCH_FLEX_IDX_USED_CLR(_u_, _idx_) \
        SHR_BITCLR(SWITCH_HASH_INFO((_u_))->hash_idx_bitmap, (_idx_))


#define FLEX_HASH_MAX_FIELDS 4
#define FLEX_HASH_MAX_OFFSET 13
#define FLEX_QUAL_BIT_PER_SIZE 8
#define FLEX_HASH_OFFSET_NUM(_u_) (SOC_IS_TOMAHAWK2(_u_) ? 13 : 2)
#define FLEX_HASH_CHECK_FIELDS_NUM 2
typedef struct _bcm_flex_hash_match_arr_s {
    int             qual_id;    /* Data Qualifier Id */
    int             qual_size;  /* Data qualifier size: xbytes, 2byte max */
    uint32          qual_match; /* Qualifier match data */
    uint32          qual_mask;  /* Qualifier mask */
} _bcm_flex_hash_match_arr_t;

typedef struct _bcm_flex_hash_entry_s {
    int             entry_id;   /* Logical eid for flex hash entry */
    int             hw_idx; /* Associated flex table index */
    int             entry_count;    /* Reference count of the entry */
    int             offset_bitmap;   /* Flex hash offset bitmap */
    int             offset_count;   /* Flex hash offset count */
    int             hash_offset[FLEX_HASH_MAX_OFFSET]; /* Flex hash offset */
    int             hash_mask[FLEX_HASH_MAX_OFFSET]; /* Flex hash mask */
    _bcm_flex_hash_match_arr_t match_arr[FLEX_HASH_MAX_FIELDS];
    struct _bcm_flex_hash_entry_s *next;    /* Next in flex hash list */
    int             type[FLEX_HASH_MAX_OFFSET];   /* field type,
                 currently there are two field types in flex hash ram table. 0: offset, 1: udf chunk */
    bcm_udf_id_t    udf_id[FLEX_HASH_MAX_OFFSET];  /*udf id when field type is udf*/
} flex_hash_entry_t, *flex_hash_entry_p;

/* Switch flex hash bookkeeping structure */
typedef struct _bcm_td2_switch_hash_bookeeping_s {
    int             init;
    int             api_ver;
    flex_hash_entry_p hash_entry_list;
    SHR_BITDCL     *hash_idx_bitmap;
} _bcm_td2_switch_hash_bookeeping_t;

STATIC          _bcm_td2_switch_hash_bookeeping_t
    _bcm_td2_switch_hash_bk_info[BCM_MAX_NUM_UNITS];

#define SWITCH_HASH_INFO(_unit_)   (&_bcm_td2_switch_hash_bk_info[_unit_])
#define SWITCH_HASH_INIT(_unit_) \
            (_bcm_td2_switch_hash_bk_info[_unit_].init)
#define SWITCH_HASH_ENTRY_LIST(_unit_) \
            (_bcm_td2_switch_hash_bk_info[_unit_].hash_entry_list)
#define SWITCH_HASH_IDX_BITMAP(_unit_) \
            (_bcm_td2_switch_hash_bk_info[_unit_].hash_idx_bitmap)

/* Global defines */
STATIC uint32   last_hash_entry_id = 0;

#if defined (BCM_WARM_BOOT_SUPPORT)
/* warmboot entry */
typedef struct wb_hash_entry_match_arr_s {
    uint32 qual_id;    
    uint32 qual_size; 
}  wb_hash_entry_match_arr_t;

typedef struct bcmi_xgs4_td2_switch_wb_hash_entry_s {
    uint32 entry_id;
    uint32 hw_idx;
    uint32 offset_bitmap;
    wb_hash_entry_match_arr_t match_arr[FLEX_HASH_MAX_FIELDS];
    bcm_udf_id_t udf_id[FLEX_HASH_MAX_OFFSET];
} bcmi_xgs4_td2_switch_wb_hash_entry_t;
#endif



typedef struct _bcm_td2_gtp_port_match_s {
    uint8 match_criteria;
    uint16 dst_port;
    uint16 src_port;
} _bcm_td2_gtp_port_match_t;
/*
 * Software book keeping for Switch Match related information
 */
typedef struct _bcm_td2_switch_match_bookkeeping_s {
    _bcm_td2_gtp_port_match_t **gtp_ptr_array; /* Array of pointers to gtp port table */
    sal_mutex_t match_mutex;
} _bcm_td2_switch_match_bookkeeping_t;

STATIC _bcm_td2_switch_match_bookkeeping_t
    _bcm_td2_switch_match_bk_info[BCM_MAX_NUM_UNITS];

STATIC int _bcm_td2_match_initialized[BCM_MAX_NUM_UNITS] = { 0 };

#define TD2_MATCH_INFO(_u_) (&_bcm_td2_switch_match_bk_info[_u_])

#define TD2_GTP_PORT_MATCH(_u_, id) \
    (TD2_MATCH_INFO(_u_)->gtp_ptr_array[id])

#define TD2_GTP_PORT_MATCH_IS_USED(_u_, id) \
    (TD2_MATCH_INFO(_u_)->gtp_ptr_array[id] != NULL)


/* Switch Match related definitions */
#define _BCM_TD2_SWITCH_MATCH_ID_MASK               (0xffff)
#define _BCM_TD2_SWITCH_MATCH_TYPE_SHIFT            (28)

#define TD2_MATCH_TYPE_GET(_id_) \
        (_id_ >> _BCM_TD2_SWITCH_MATCH_TYPE_SHIFT)
#define TD2_MATCH_ID_SET(_id_, _service_type_) \
        (_id_ | _service_type_ << _BCM_TD2_SWITCH_MATCH_TYPE_SHIFT)
#define TD2_MATCH_HW_ID_GET(_id_) \
        (_id_ & _BCM_TD2_SWITCH_MATCH_ID_MASK)

#define TD2_MATCH_LOCK(_u_) \
        sal_mutex_take(TD2_MATCH_INFO(_u_)->match_mutex, sal_mutex_FOREVER);
#define TD2_MATCH_UNLOCK(_u_) \
        sal_mutex_give(TD2_MATCH_INFO(_u_)->match_mutex);

#define TD2_MATCH_INIT(_u_)                               \
    do {                                                  \
        if ((_u_ < 0) || (_u_ >= BCM_MAX_NUM_UNITS)) {    \
            return BCM_E_UNIT;                            \
        }                                                 \
        if (!_bcm_td2_match_initialized[_u_]) {           \
            return BCM_E_INIT;                            \
        }                                                 \
    } while (0)


/* forward declarations */

STATIC int      _flex_hash_entry_get(int unit, bcm_hash_entry_t hash_entry_id,
                     flex_hash_entry_t ** hash_entry);

STATIC int      _bcm_hash_entry_delete(int unit, bcm_hash_entry_t entry_id);
STATIC int      _bcm_hash_entry_remove(int unit, bcm_hash_entry_t entry_id);

STATIC int      _flex_hash_entry_destroy(int unit, flex_hash_entry_t * entry);


#if defined (BCM_WARM_BOOT_SUPPORT)
STATIC int      _bcm_flex_hash_table_wb_read(int unit,
                                     flex_hash_entry_t * hash_entry,
                                     bcmi_xgs4_td2_switch_wb_hash_entry_t * wb_hash_entry);
#endif 

/*
 * The below listed flexible hashing support routines are modularized for table
 * and list management routines.  
 *  1. Flex hash TCAM table read/write routines
 *     _bcm_flex_hash_table_write
 *     _bcm_flex_hash_table_read
 *     _bcm_flex_hash_control_enable
 *     _bcm_flex_hash_control_disable
 *    
 *  2. Flex hash Entry list management routines 
 *     _flex_hash_entry_get 
 *     _flex_hash_entry_destroy
 *     _flex_hash_entry_destroy_all
 *     _flex_hash_entry_alloc
 *     _flex_hash_entry_add
 *     _flex_hash_entry_delete
 * 
 *  3. Flex hash management routines 
 *     _bcm_hash_entry_create
 *     _bcm_hash_entry_delete
 *     _bcm_hash_entry_qual_set
 *     _bcm_hash_entry_qual_get
 *     _bcm_hash_entry_qual_data_set
 *     _bcm_hash_entry_qual_data_get
 * 
 * 
 *  4. Flexible hash Switch API support routines
 *     bcm_td2_switch_hash_entry_init
 *     bcm_td2_switch_hash_entry_detach
 *     bcm_td2_switch_hash_entry_create
 *     bcm_td2_switch_hash_entry_destroy
 *     bcm_td2_switch_hash_qualify_data
 * 
 *     bcm_td2_switch_hash_entry_install
 *     bcm_td2_switch_hash_entry_reinstall
 *     bcm_td2_switch_hash_entry_remove
 */

/*
 * 
 *  Flex hash TCAM table read/write routines 
 *
 */
 #if defined(BCM_TOMAHAWK2_SUPPORT)
 STATIC int
_bcm_th2_flex_hash_table_write(int unit, flex_hash_entry_t * hash_entry, uint8 delete)
{
        int             fld, rv = BCM_E_NONE;
        FLEX_HASH_TCAM_ENTRY_T flex_tcam_entry;
        FLEX_HASH_OFFSET_ENTRY_T flex_offset_entry;
        FLEX_HASH_OFFSET_MASK_ENTRY_T flex_offset_mask_entry;
        uint32          data[2] = { 0, 0 };
        uint32          mask[2] = { 0, 0 };

        if (NULL == hash_entry) {
            return BCM_E_FAIL;
        }

    /*
        note :qual_size_range is 0~2. maximum 4 match field data in a flexkey.
        it is required to create a flexkey in a such order
        field_data2: 1st qual_data, 2nd qual_data
        field_data1: 3rd qual_data, 4th qual_data; thus
        first and third match field data is shifed to left for 16 bits insdie field data.
        in addition, if qual_size is 1, qual_data is shifed to left for 8 bits inside field data
        if qual_size is 0 or 2, there won't be any shift inside field data
    */
        data[0] = (hash_entry->match_arr[0].qual_match <<
                      ((hash_entry->match_arr[0].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
        data[0] |= (hash_entry->match_arr[1].qual_match <<
                      ((hash_entry->match_arr[1].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;
        data[1] = (hash_entry->match_arr[2].qual_match <<
                      ((hash_entry->match_arr[2].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
        data[1] |= (hash_entry->match_arr[3].qual_match <<
                      ((hash_entry->match_arr[3].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;

        mask[0] = (hash_entry->match_arr[0].qual_mask <<
                      ((hash_entry->match_arr[0].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
        mask[0] |= (hash_entry->match_arr[1].qual_mask <<
                      ((hash_entry->match_arr[1].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;
        mask[1] = (hash_entry->match_arr[2].qual_mask <<
                      ((hash_entry->match_arr[2].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
        mask[1] |= (hash_entry->match_arr[3].qual_mask <<
                      ((hash_entry->match_arr[3].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;

        sal_memset(&flex_tcam_entry, 0, sizeof(flex_tcam_entry));
        sal_memset(&flex_offset_entry, 0, sizeof(flex_offset_entry));
        sal_memset(&flex_offset_mask_entry, 0, sizeof(flex_offset_mask_entry));

        rv = soc_mem_read(unit, FLEX_HASH_TCAM, MEM_BLOCK_ANY,
                  hash_entry->hw_idx, &flex_tcam_entry);
        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }
        rv = soc_mem_read(unit, FLEX_HASH_OFFSET, MEM_BLOCK_ANY,
                  hash_entry->hw_idx, &flex_offset_entry);
        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }

        rv = soc_mem_read(unit, FLEX_HASH_OFFSET_MASK(unit), MEM_BLOCK_ANY,
                  hash_entry->hw_idx, &flex_offset_mask_entry);
        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }

        if (soc_mem_field_valid(unit, FLEX_HASH_TCAM, VALIDf)) {
            soc_mem_field32_set(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                        VALIDf, delete? 0:1);
        }

        for (fld = 0; fld < FLEX_HASH_CHECK_FIELDS_NUM; fld++) {
            soc_mem_field32_set(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                        _bcm_td2_flex_hash_data[fld], delete ? 0 : data[fld]);
            soc_mem_field32_set(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                        _bcm_td2_flex_hash_mask[fld], delete ? 0 : mask[fld]);
        }

        for (fld = 0; fld < FLEX_HASH_OFFSET_NUM(unit); fld++) {
            if (hash_entry->type[fld] & BCM_SWITCH_HASH_FIELD_TYPE_UDF) {
                if (hash_entry->offset_bitmap & (0x1 << fld)) {
                    if (hash_entry->hash_offset[fld] >= MAX_UDF_OFFSET_CHUNKS) {
                        return BCM_E_PARAM;
                    }
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_th_flex_hash_offset_udf[fld*2],
                                delete ? 0 : hash_entry->hash_offset[fld] / 8);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_th_flex_hash_offset_udf[fld*2 + 1],
                                delete ? 0 : hash_entry->hash_offset[fld] % 8);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET_MASK(unit),
                                &flex_offset_mask_entry,
                                _bcm_td2_flex_hash_mask[fld],
                                delete ? 0 : hash_entry->hash_mask[fld]);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_th_flex_hash_offset_type[fld],
                                delete ? 0 : 1);
               } else {
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_th_flex_hash_offset_udf[fld*2], 0);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_th_flex_hash_offset_udf[fld*2 + 1], 0);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET_MASK(unit),
                                &flex_offset_mask_entry,
                                _bcm_td2_flex_hash_mask[fld], 0);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_th_flex_hash_offset_type[fld], 0);
                    break;
                }
            } else {
                if (fld > bcmSwitchFlexHashField1) {
                        continue;
                }
                if (hash_entry->offset_bitmap & (0x1 << fld)) {
                    if (hash_entry->hash_offset[fld] >=
                        (FLEX_QUAL_BIT_PER_SIZE *  FLEX_HASH_CHECK_FIELDS_NUM)) {
                        return BCM_E_PARAM;
                    }
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_td2_flex_hash_offset[fld],
                                delete ? 0 : hash_entry->hash_offset[fld]);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET_MASK(unit),
                                &flex_offset_mask_entry,
                                _bcm_td2_flex_hash_mask[fld],
                                delete ? 0 : hash_entry->hash_mask[fld]);
                } else {
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                                &flex_offset_entry,
                                _bcm_td2_flex_hash_offset[fld], 0);
                    soc_mem_field32_set(unit, FLEX_HASH_OFFSET_MASK(unit),
                                &flex_offset_mask_entry,
                                _bcm_td2_flex_hash_mask[fld], 0);
                }
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_type[fld], 0);
            }
        }

        rv = soc_mem_write(unit, FLEX_HASH_OFFSET, MEM_BLOCK_ANY,
                   hash_entry->hw_idx, &flex_offset_entry);
        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }
        rv = soc_mem_write(unit, FLEX_HASH_OFFSET_MASK(unit), MEM_BLOCK_ANY,
                   hash_entry->hw_idx, &flex_offset_mask_entry);
        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }
        rv = soc_mem_write(unit, FLEX_HASH_TCAM, MEM_BLOCK_ALL,
                   hash_entry->hw_idx, &flex_tcam_entry);

        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }
        return BCM_E_NONE;
    }
#endif /*BCM_TOMAHAWK2_SUPPORT*/

STATIC int
_bcm_flex_hash_table_write(int unit, flex_hash_entry_t * hash_entry, uint8 delete)
{
    int             fld, rv = BCM_E_NONE;
    FLEX_HASH_TCAM_ENTRY_T flex_tcam_entry;
    FLEX_HASH_OFFSET_ENTRY_T flex_offset_entry;
    uint32          data[2] = { 0, 0 };
    uint32          mask[2] = { 0, 0 };

    if (NULL == hash_entry) {
        return BCM_E_FAIL;
    }

/*  
    note :qual_size_range is 0~2. maximum 4 match field data in a flexkey.
    it is required to create a flexkey in a such order
    field_data2: 1st qual_data, 2nd qual_data
    field_data1: 3rd qual_data, 4th qual_data; thus
    first and third match field data is shifed to left for 16 bits insdie field data.
    in addition, if qual_size is 1, qual_data is shifed to left for 8 bits inside field data 
    if qual_size is 0 or 2, there won't be any shift inside field data
*/
    data[0] = (hash_entry->match_arr[0].qual_match <<
                  ((hash_entry->match_arr[0].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
    data[0] |= (hash_entry->match_arr[1].qual_match <<
                  ((hash_entry->match_arr[1].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;
    data[1] = (hash_entry->match_arr[2].qual_match <<
                  ((hash_entry->match_arr[2].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
    data[1] |= (hash_entry->match_arr[3].qual_match <<
                  ((hash_entry->match_arr[3].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;

    mask[0] = (hash_entry->match_arr[0].qual_mask <<
                  ((hash_entry->match_arr[0].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
    mask[0] |= (hash_entry->match_arr[1].qual_mask <<
                  ((hash_entry->match_arr[1].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;
    mask[1] = (hash_entry->match_arr[2].qual_mask <<
                  ((hash_entry->match_arr[2].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 16;
    mask[1] |= (hash_entry->match_arr[3].qual_mask <<
                  ((hash_entry->match_arr[3].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE)) << 0;

    sal_memset(&flex_tcam_entry, 0, sizeof(flex_tcam_entry));
    sal_memset(&flex_offset_entry, 0, sizeof(flex_offset_entry));

    rv = soc_mem_read(unit, FLEX_HASH_TCAM, MEM_BLOCK_ANY,
              hash_entry->hw_idx, &flex_tcam_entry);
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }
    rv = soc_mem_read(unit, FLEX_HASH_OFFSET, MEM_BLOCK_ANY,
              hash_entry->hw_idx, &flex_offset_entry);
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }

    if (soc_mem_field_valid(unit, FLEX_HASH_TCAM, VALIDf)) { 
        soc_mem_field32_set(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                    VALIDf, delete? 0:1);
    }

    for (fld = 0; fld < FLEX_HASH_CHECK_FIELDS_NUM; fld++) {
        soc_mem_field32_set(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                    _bcm_td2_flex_hash_data[fld], delete ? 0 : data[fld]);
        soc_mem_field32_set(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                    _bcm_td2_flex_hash_mask[fld], delete ? 0 : mask[fld]);
    }

    for (fld = 0; fld < FLEX_HASH_OFFSET_NUM(unit); fld++) {
#if defined(BCM_TOMAHAWK_SUPPORT)
        if (hash_entry->type[fld] & BCM_SWITCH_HASH_FIELD_TYPE_UDF) {
            if (hash_entry->offset_bitmap & (0x1 << fld)) {
                if (hash_entry->hash_offset[fld] >= MAX_UDF_OFFSET_CHUNKS) {
                    return BCM_E_PARAM;
                }
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_udf[fld*2],
                            delete ? 0 : hash_entry->hash_offset[fld] / 8);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_udf[fld*2 + 1],
                            delete ? 0 : (7 - hash_entry->hash_offset[fld] % 8));
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_td2_flex_hash_mask[fld],
                            delete ? 0 : hash_entry->hash_mask[fld]);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_type[fld],
                            delete ? 0 : 1);
           } else {
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_udf[fld*2], 0);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_udf[fld*2 + 1], 0);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_td2_flex_hash_mask[fld], 0);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_type[fld], 0);
                break;
            }
        } else
#endif /*BCM_TOMAHAWK_SUPPORT*/
        {
            if (fld > bcmSwitchFlexHashField1) {
                    continue;
            }
            if (hash_entry->offset_bitmap & (0x1 << fld)) {
                if (hash_entry->hash_offset[fld] >=
                    (FLEX_QUAL_BIT_PER_SIZE *  FLEX_HASH_CHECK_FIELDS_NUM)) {
                    return BCM_E_PARAM;
                }
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_td2_flex_hash_offset[fld],
                            delete ? 0 : hash_entry->hash_offset[fld]);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_td2_flex_hash_mask[fld],
                            delete ? 0 : hash_entry->hash_mask[fld]);
            } else {
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_td2_flex_hash_offset[fld], 0);
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_td2_flex_hash_mask[fld], 0);
            }
#if defined(BCM_TOMAHAWK_SUPPORT)
            if (SOC_IS_TOMAHAWK(unit)) {
                soc_mem_field32_set(unit, FLEX_HASH_OFFSET,
                            &flex_offset_entry,
                            _bcm_th_flex_hash_offset_type[fld], 0);
            }
#endif /*BCM_TOMAHAWK_SUPPORT*/
        }
    }

    rv = soc_mem_write(unit, FLEX_HASH_OFFSET, MEM_BLOCK_ANY,
               hash_entry->hw_idx, &flex_offset_entry);
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }
    rv = soc_mem_write(unit, FLEX_HASH_TCAM, MEM_BLOCK_ALL,
               hash_entry->hw_idx, &flex_tcam_entry);
    
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }

    return BCM_E_NONE;
}

#if defined (BCM_WARM_BOOT_SUPPORT)
STATIC int
_bcm_flex_hash_table_wb_read(int unit,
                      flex_hash_entry_t * hash_entry,
                      bcmi_xgs4_td2_switch_wb_hash_entry_t * wb_hash_entry)
{
    int             fld, rv = BCM_E_NONE;
    uint32          data[2], mask[2];
    FLEX_HASH_TCAM_ENTRY_T flex_tcam_entry;
    FLEX_HASH_OFFSET_ENTRY_T flex_offset_entry;

#if defined(BCM_TOMAHAWK_SUPPORT)
    int udf_chunk_id, udf_sel;
#endif /*BCM_TOMAHAWK_SUPPORT*/

#ifdef BCM_TOMAHAWK2_SUPPORT
    FLEX_HASH_OFFSET_MASK_ENTRY_T flex_offset_mask_entry;
#endif /*BCM_TOMAHAWK2_SUPPORT*/

    sal_memset(&flex_tcam_entry, 0, sizeof(flex_tcam_entry));
    sal_memset(&flex_offset_entry, 0, sizeof(flex_offset_entry));
    
    hash_entry->offset_count = 0;

    rv = soc_mem_read(unit, FLEX_HASH_TCAM, MEM_BLOCK_ANY,
              hash_entry->hw_idx, &flex_tcam_entry);
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }

    rv = soc_mem_read(unit, FLEX_HASH_OFFSET, MEM_BLOCK_ANY,
              hash_entry->hw_idx, &flex_offset_entry);
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }
#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
       sal_memset(&flex_offset_mask_entry, 0, sizeof(flex_offset_mask_entry));
       rv = soc_mem_read(unit, FLEX_HASH_OFFSET_MASK(unit), MEM_BLOCK_ANY,
                hash_entry->hw_idx, &flex_offset_mask_entry);
        if (BCM_FAILURE(rv)) {
            return BCM_E_FAIL;
        }
    }
#endif

    for (fld = 0; fld < FLEX_HASH_CHECK_FIELDS_NUM; fld++) {
        data[fld] =
            soc_mem_field32_get(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                    _bcm_td2_flex_hash_data[fld]);
        mask[fld] =
            soc_mem_field32_get(unit, FLEX_HASH_TCAM, &flex_tcam_entry,
                    _bcm_td2_flex_hash_mask[fld]);
    }

    for (fld = 0; fld < FLEX_HASH_OFFSET_NUM(unit); fld++) {
#if defined(BCM_TOMAHAWK_SUPPORT)
        if (SOC_IS_TOMAHAWKX(unit)) {
            hash_entry->type[fld] =
                soc_mem_field32_get(unit, FLEX_HASH_OFFSET,
                        &flex_offset_entry,
                        _bcm_th_flex_hash_offset_type[fld]);
        }
        if (hash_entry->type[fld] & BCM_SWITCH_HASH_FIELD_TYPE_UDF) {
            udf_sel = soc_mem_field32_get(unit, FLEX_HASH_OFFSET,
                        &flex_offset_entry,
                        _bcm_th_flex_hash_offset_udf[fld*2]);
            udf_chunk_id= soc_mem_field32_get(unit, FLEX_HASH_OFFSET,
                        &flex_offset_entry,
                        _bcm_th_flex_hash_offset_udf[fld*2 + 1]);
            hash_entry->hash_offset[fld] = udf_sel * 8 + (7 - udf_chunk_id);
        } else
#endif /*BCM_TOMAHAWK_SUPPORT*/
        {
            if (fld > bcmSwitchFlexHashField1) {
                    continue;
            }
            hash_entry->hash_offset[fld] =
                soc_mem_field32_get(unit, FLEX_HASH_OFFSET,
                        &flex_offset_entry,
                        _bcm_td2_flex_hash_offset[fld]);
        }

#ifdef BCM_TOMAHAWK2_SUPPORT
        if (SOC_IS_TOMAHAWK2(unit)) {
            hash_entry->hash_mask[fld] =
                soc_mem_field32_get(unit, FLEX_HASH_OFFSET_MASK(unit),
                        &flex_offset_mask_entry,
                        _bcm_td2_flex_hash_mask[fld]);
        } else
#endif
        {
            hash_entry->hash_mask[fld] =
                soc_mem_field32_get(unit, FLEX_HASH_OFFSET,
                        &flex_offset_entry,
                        _bcm_td2_flex_hash_mask[fld]);
        }
        
        if (hash_entry->hash_offset[fld] > 0) {
            hash_entry->offset_count++;
        }
    }

    for (fld = 0; fld < FLEX_HASH_MAX_FIELDS; fld++) {
        hash_entry->match_arr[fld].qual_id= wb_hash_entry->match_arr[fld].qual_id;
        if (hash_entry->match_arr[fld].qual_id >= 0)
        {
            hash_entry->match_arr[fld].qual_size = wb_hash_entry->match_arr[fld].qual_size;

            hash_entry->match_arr[fld].qual_match = (data[fld / 2] >>((fld % 2) ? 0:16)) >>
                ((hash_entry->match_arr[0].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE);
            hash_entry->match_arr[fld].qual_mask = (mask[fld / 2] >> ((fld % 2) ? 0:16)) >>
                ((hash_entry->match_arr[0].qual_size & 1) * FLEX_QUAL_BIT_PER_SIZE);

        }
    }
#if defined(BCM_TOMAHAWK_SUPPORT)
    for (fld = 0; fld < FLEX_HASH_OFFSET_NUM(unit); fld++) {
        if (hash_entry->type[fld]) {
            hash_entry->udf_id[fld] = wb_hash_entry->udf_id[fld];
        }
    }
#endif /*BCM_TOMAHAWK_SUPPORT*/

    return BCM_E_NONE;
}
#endif

/*
    enable RTAG7_HASH_CONTROL and ING_HASH_CONFIG_0 registers
*/
STATIC int
_bcm_flex_hash_control_enable(int unit)
{

    int             cnt;
    soc_reg_t       reg;
    uint32          hash_control, hash_config;
#ifdef BCM_TOMAHAWK2_SUPPORT
    uint64          hash_control_64;
#endif

    /* 
     * Enable Flexible/UDF controls
     * Flex hash controls are enabled for Hash A and Hash B, which has 
     * high precedence over Flex UDF controls.
     */
#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        reg = _bcm_th2_flex_hash_control;
        BCM_IF_ERROR_RETURN(soc_reg64_get
                    (unit, reg, REG_PORT_ANY, 0, &hash_control_64));
        for (cnt = 0;
             cnt <
             (sizeof(_bcm_th2_flex_hash_enable_fields) /
              sizeof(_bcm_th2_flex_hash_enable_fields[0])); cnt++) {
            if (SOC_REG_FIELD_VALID
                (unit, reg, _bcm_th2_flex_hash_enable_fields[cnt])) {
                soc_reg64_field32_set(unit, reg, &hash_control_64,
                          _bcm_th2_flex_hash_enable_fields[cnt], 1);
            }
        }
        BCM_IF_ERROR_RETURN(soc_reg64_set
                    (unit, reg, REG_PORT_ANY, 0, hash_control_64));
    } else
#endif
    {
        reg = _bcm_td2_flex_hash_control;
        BCM_IF_ERROR_RETURN(soc_reg32_get
                    (unit, reg, REG_PORT_ANY, 0, &hash_control));
        for (cnt = 0;
             cnt <
             (sizeof(_bcm_td2_flex_hash_enable_fields) /
              sizeof(_bcm_td2_flex_hash_enable_fields[0])); cnt++) {
            if (SOC_REG_FIELD_VALID
                (unit, reg, _bcm_td2_flex_hash_enable_fields[cnt])) {
                soc_reg_field_set(unit, reg, &hash_control,
                          _bcm_td2_flex_hash_enable_fields[cnt],
                          1);
            }
        }
        BCM_IF_ERROR_RETURN(soc_reg32_set
                    (unit, reg, REG_PORT_ANY, 0, hash_control));
    }

    /* Enable Actual TCAM lookup using flex key*/
    reg = _bcm_td2_ing_hash_config;
    BCM_IF_ERROR_RETURN(READ_ING_HASH_CONFIG_0r(unit, &hash_config));

    if (SOC_REG_FIELD_VALID
        (unit, reg, _bcm_td2_ing_hash_config_fields[0])) {
        soc_reg_field_set(unit, reg, &hash_config,
            _bcm_td2_ing_hash_config_fields[0], 1);
    } 

    BCM_IF_ERROR_RETURN(WRITE_ING_HASH_CONFIG_0r(unit, hash_config));

    return BCM_E_NONE;
}

STATIC int
_bcm_flex_hash_control_disable(int unit)
{

    int             cnt;
    soc_reg_t       reg;
    uint32          hash_control, hash_config;
#ifdef BCM_TOMAHAWK2_SUPPORT
    uint64          hash_control_64;
#endif

    /* 
     * Disable Flexible/UDF controls
     * Flex hash controls are enabled for Hash A and Hash B, which has 
     * high precedence over Flex UDF controls.
     */
#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        reg = _bcm_th2_flex_hash_control;
        BCM_IF_ERROR_RETURN(soc_reg64_get
                    (unit, reg, REG_PORT_ANY, 0, &hash_control_64));
        for (cnt = 0;
             cnt <
             (sizeof(_bcm_th2_flex_hash_enable_fields) /
              _bcm_th2_flex_hash_enable_fields[0]); cnt++) {
            if (SOC_REG_FIELD_VALID
                (unit, reg, _bcm_th2_flex_hash_enable_fields[cnt])) {
                soc_reg64_field32_set(unit, reg, &hash_control_64,
                          _bcm_th2_flex_hash_enable_fields[cnt], 0);
            }
        }
        BCM_IF_ERROR_RETURN(soc_reg64_set
                    (unit, reg, REG_PORT_ANY, 0, hash_control_64));
    } else
#endif
    {
        reg = _bcm_td2_flex_hash_control;
        BCM_IF_ERROR_RETURN(soc_reg32_get
                    (unit, reg, REG_PORT_ANY, 0, &hash_control));
        for (cnt = 0;
             cnt <
             (sizeof(_bcm_td2_flex_hash_enable_fields) /
              _bcm_td2_flex_hash_enable_fields[0]); cnt++) {
            if (SOC_REG_FIELD_VALID
                (unit, reg, _bcm_td2_flex_hash_enable_fields[cnt])) {
                soc_reg_field_set(unit, reg, &hash_control,
                          _bcm_td2_flex_hash_enable_fields[cnt],
                          0);
            }
        }
        BCM_IF_ERROR_RETURN(soc_reg32_set
                    (unit, reg, REG_PORT_ANY, 0, hash_control));
    }

    /* Disable TCAM lookup using flex key*/
    reg = _bcm_td2_ing_hash_config;
    BCM_IF_ERROR_RETURN(READ_ING_HASH_CONFIG_0r(unit, &hash_config));

    if (SOC_REG_FIELD_VALID
        (unit, reg, _bcm_td2_ing_hash_config_fields[0])) {

        soc_reg_field_set(unit, reg, &hash_config,
            _bcm_td2_ing_hash_config_fields[0], 0);
    } 

    BCM_IF_ERROR_RETURN(WRITE_ING_HASH_CONFIG_0r(unit, hash_config));
    return BCM_E_NONE;
}

/*
 * 
 *  Flex hash Entry list management routines 
 *
 */

STATIC int
_flex_hash_entry_get(int unit, bcm_hash_entry_t hash_entry_id,
             flex_hash_entry_t ** hash_entry)
{
    *hash_entry = SWITCH_HASH_ENTRY_LIST(unit);
    while (*hash_entry != NULL) {
        if ((*hash_entry)->entry_id == hash_entry_id) {
            return BCM_E_NONE;
        }
        *hash_entry = (*hash_entry)->next;
    }
    return BCM_E_NOT_FOUND;
}

STATIC int
_flex_hash_entry_destroy(int unit, flex_hash_entry_t * entry)
{

    if (NULL == entry) {
        return BCM_E_NOT_FOUND;
    }

    if (entry->entry_count == 0) {
        sal_free(entry);
    }
    return BCM_E_NONE;
}

STATIC int
_flex_hash_entry_destroy_all(int unit)
{

    flex_hash_entry_t *hash_entry = NULL;
    flex_hash_entry_t *hash_curr = NULL;

    hash_entry = SWITCH_HASH_ENTRY_LIST(unit);

    SWITCH_HASH_ENTRY_LIST(unit) = NULL;
    if (NULL != hash_entry) {
        while (hash_entry != NULL) {
            hash_curr = hash_entry;
            hash_entry = hash_curr->next;
            _BCM_SWITCH_FLEX_IDX_USED_CLR(unit, hash_curr->hw_idx);
            sal_free(hash_curr);
        }
    }
    BCM_IF_ERROR_RETURN(_bcm_flex_hash_control_disable(unit));
    return BCM_E_NONE;
}

STATIC int
_flex_hash_entry_alloc(int unit, flex_hash_entry_t ** entry)
{
    int             entry_size;
    int             entry_wrap = 0;
    flex_hash_entry_p hash_entry = NULL;

    if (NULL == entry) {
        return BCM_E_FAIL;
    }

    entry_size = sizeof(flex_hash_entry_t);
    *entry = sal_alloc(entry_size, "flex hash entry");
    if (*entry == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(*entry, 0, entry_size);

    while (BCM_SUCCESS
           (_flex_hash_entry_get(unit, last_hash_entry_id, &hash_entry))) {
        if (last_hash_entry_id++ == 0) {
            if (entry_wrap++ >= 2) {
                sal_free(*entry);
                return BCM_E_RESOURCE;
            }
        }
    }
    (*entry)->entry_id = last_hash_entry_id;
    (*entry)->next = NULL;   

    return BCM_E_NONE;
}


STATIC int
_flex_hash_entry_add(int unit, flex_hash_entry_t * entry, int hw_index)
{
    flex_hash_entry_t *hash_entry = NULL;
    flex_hash_entry_t *hash_prev = NULL;

    if (NULL == entry) {
        return BCM_E_FAIL;
    }

    hash_entry = SWITCH_HASH_ENTRY_LIST(unit);
    if (NULL == hash_entry) {
        entry->hw_idx = hw_index;
        SWITCH_HASH_ENTRY_LIST(unit) = entry;
        if (!SOC_WARM_BOOT(unit)) {
            BCM_IF_ERROR_RETURN(_bcm_flex_hash_control_enable(unit));
        }
    } else {
        while (hash_entry != NULL) {
            if (hash_entry->entry_id == entry->entry_id) {
                entry->hw_idx = hw_index;
                /* Increment entry count */
                entry->entry_count++;
            } else if (hash_entry->entry_id < entry->entry_id) {
                if (NULL == hash_prev) {
                    SWITCH_HASH_ENTRY_LIST(unit) = entry;
                } else {
                    hash_prev->next = entry;
                }
                entry->next = hash_entry;
                entry->hw_idx = hw_index;
                /* Increment entry count */
                entry->entry_count++;
                break;
            } 
            hash_prev = hash_entry;
            hash_entry = hash_entry->next;
        }
    }
    return BCM_E_NONE;
}

STATIC int
_flex_hash_entry_delete(int unit, flex_hash_entry_t * entry)
{
    flex_hash_entry_t *hash_entry = NULL;
    flex_hash_entry_t *hash_prev = NULL;

    if (NULL == entry) {
        return BCM_E_FAIL;
    }

    hash_entry = SWITCH_HASH_ENTRY_LIST(unit);
    if (NULL != hash_entry) {
    
            while (hash_entry != NULL) {
            if (hash_entry->entry_id == entry->entry_id) {
                if (NULL == hash_prev) {
                    SWITCH_HASH_ENTRY_LIST(unit) = hash_entry->next;
                } else {
                    hash_prev->next = hash_entry->next;
                }
                if (SWITCH_HASH_ENTRY_LIST(unit) == NULL) {
                    BCM_IF_ERROR_RETURN
                        (_bcm_flex_hash_control_disable(unit));

                }
                break;  
        }
            hash_prev = hash_entry;
            hash_entry = hash_entry->next;
        }
    }
    return BCM_E_NONE;
}

/*
 * 
 *  Flexible Hashing management routines 
 *
 */

STATIC int
_bcm_hash_entry_create(int unit, bcm_hash_entry_t * entry)
{
    int             num_indexes, idx_cnt;
    int             hw_index = -1;
    flex_hash_entry_p hash_entry = NULL;
    int             rv = BCM_E_NONE;

    /* Allocate hardware index into flex tcam table. */
    num_indexes = soc_mem_index_count(unit, FLEX_HASH_TCAM);
    for (idx_cnt = 0; idx_cnt < num_indexes; idx_cnt++) {
        if (!_BCM_SWITCH_FLEX_IDX_USED_GET(unit, idx_cnt)) {
            hw_index = idx_cnt; /* free index */
            break;
        }
    }
    if (hw_index == -1) {
        return BCM_E_FULL;
    }
    _BCM_SWITCH_FLEX_IDX_USED_SET(unit, hw_index);

    /* Allocate entry id and associate with hardware index. */
    rv = _flex_hash_entry_alloc(unit, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    /* Add in hash entry list. 
       If RTAG7_HASH_CONTROL and ING_HASH_CONFIG_0 are not set,
       the registers will be programemed in this routine */
    rv = _flex_hash_entry_add(unit, hash_entry, hw_index);
    if (BCM_FAILURE(rv)) {
        _flex_hash_entry_destroy(unit, hash_entry);
        return rv;
    }

    /* return entry id */
    *entry = hash_entry->entry_id;
    return rv;
}

STATIC int
_bcm_hash_entry_delete(int unit, bcm_hash_entry_t entry_id)
{
    int             rv = BCM_E_NONE;
    flex_hash_entry_t *hash_entry;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    _BCM_SWITCH_FLEX_IDX_USED_CLR(unit, hash_entry->hw_idx);

    rv = _flex_hash_entry_delete(unit, hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        rv = _bcm_th2_flex_hash_table_write(unit, hash_entry, TRUE);
    } else
#endif
    {
        rv = _bcm_flex_hash_table_write(unit, hash_entry, TRUE);
    }
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    rv = _flex_hash_entry_destroy(unit, hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

   
    return rv;
}

STATIC int
_bcm_hash_entry_remove(int unit, bcm_hash_entry_t entry_id)
{
    int             rv = BCM_E_NONE;
    flex_hash_entry_t *hash_entry;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        return _bcm_th2_flex_hash_table_write(unit, hash_entry, TRUE);
    } else
#endif
    {
        return _bcm_flex_hash_table_write(unit, hash_entry, TRUE);
    }
}

STATIC int
_bcm_hash_entry_udf_set(int unit, bcm_hash_entry_t entry_id,
             int qual_count, int *qual_array)
{
    int             id = 0;
    int             rv = BCM_E_NONE;
    int             idx, chunk_id, chunk_base, chunk_max;
    flex_hash_entry_t *hash_entry;
    bcmi_xgs4_udf_offset_info_t *offset_info;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (qual_count > FLEX_HASH_MAX_FIELDS) {
        /* reset outbound size */
        qual_count = FLEX_HASH_MAX_FIELDS;
    }

    chunk_base = 12; /* UDF2.4 */
    chunk_max  = chunk_base + FLEX_HASH_MAX_FIELDS - 1;

    for (id = 0; id < qual_count; id++) {
        /* Get data qualifier info. */
        rv = bcmi_xgs4_udf_offset_node_get(unit, qual_array[id],
                                           &offset_info);
        BCM_IF_ERROR_RETURN(rv);

        if (!(BCMI_XGS4_UDF_OFFSET_FLEXHASH & offset_info->flags) ||
            !(offset_info->hw_bmap & (0xf << chunk_base))) {
            return BCM_E_PARAM;
        }

        for (chunk_id = chunk_base; chunk_id <= chunk_max; chunk_id++) {
            if (offset_info->hw_bmap & (0x1 << chunk_id)) {
                idx = chunk_id - chunk_base;
                hash_entry->match_arr[idx].qual_id = qual_array[id];
                hash_entry->match_arr[idx].qual_size = offset_info->width;
                break;
            }
        }
    }
    
    return BCM_E_NONE;
}

STATIC int
_bcm_hash_entry_qual_set(int unit, bcm_hash_entry_t entry_id,
             int qual_count, int *qual_array)
{
    int             id = 0;
    int             rv = BCM_E_NONE;
    int             idx, chunk_id, chunk_base, chunk_max;
    flex_hash_entry_t *hash_entry;
    _field_data_qualifier_t data_qual;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (qual_count > FLEX_HASH_MAX_FIELDS) {
        /* reset outbound size */
        qual_count = FLEX_HASH_MAX_FIELDS;
    }

    chunk_base = 12; /* UDF2.4 */
    chunk_max  = chunk_base + FLEX_HASH_MAX_FIELDS - 1;

    for (id = 0; id < qual_count; id++) {
        /* Get data qualifier info. */
        rv = _field_data_qualifier_get(unit, qual_array[id],
                              &data_qual);
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
        if (!
            (BCM_FIELD_DATA_QUALIFIER_OFFSET_FLEX_HASH ==
             data_qual.flags) ||
            !(data_qual.hw_bmap & (0xf << chunk_base))) {
            return BCM_E_FAIL;
        }

        for (chunk_id = chunk_base; chunk_id <= chunk_max; chunk_id++) {
            if (data_qual.hw_bmap & (0x1 << chunk_id)) {
                idx = chunk_id - chunk_base;
                hash_entry->match_arr[idx].qual_id = qual_array[id];
                hash_entry->match_arr[idx].qual_size = data_qual.length;
                break;
            }
        }
    }

    return BCM_E_NONE;
}

#if 0 /* Code required for warmboot when implemented */
STATIC int
_bcm_hash_entry_qual_get(int unit, bcm_hash_entry_t entry_id,
             int *qual_array, int *qual_count)
{
    int             id = 0;
    int             rv = BCM_E_NONE;
    flex_hash_entry_t *hash_entry;
    bcm_field_data_qualifier_t data_qual;

    *qual_count = 0;
    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    for (id = 0; id < FLEX_HASH_MAX_FIELDS; id++) {
        if (bcm_esw_field_data_qualifier_get(unit,
                         hash_entry->match_arr[id].
                         qual_id, &data_qual)) {
            qual_array[id] = hash_entry->match_arr[id].qual_id;
            *qual_count += 1;
            return BCM_E_NONE;
        }
    }
    return BCM_E_NOT_FOUND;
}
#endif

STATIC int
_bcm_hash_entry_qual_data_set(int unit, bcm_hash_entry_t entry_id, int qual_id,
                  uint32 data, uint32 mask)
{
    int             id = 0;
    int             rv = BCM_E_NONE;
    flex_hash_entry_t *hash_entry;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    for (id = 0; id < FLEX_HASH_MAX_FIELDS; id++) {
        if (hash_entry->match_arr[id].qual_id == qual_id) {
            hash_entry->match_arr[id].qual_match = data;
            hash_entry->match_arr[id].qual_mask = mask;

            return BCM_E_NONE;
        }
    }
    return BCM_E_NOT_FOUND;
}

#if 0 /* Code required for warmboot when implemented */
STATIC int
_bcm_hash_entry_qual_data_get(int unit, bcm_hash_entry_t entry_id, int qual_id,
                  uint32 * data, uint32 * mask)
{
    int             id = 0;
    int             rv = BCM_E_NONE;
    flex_hash_entry_t *hash_entry;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    for (id = 0; id < FLEX_HASH_MAX_FIELDS; id++) {
        if (hash_entry->match_arr[id].qual_id == qual_id) {
            *data = hash_entry->match_arr[id].qual_match;
            *mask = hash_entry->match_arr[id].qual_mask;
        }
    }

    if (FLEX_HASH_MAX_FIELDS == id) {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}
#endif

#if defined (BCM_WARM_BOOT_SUPPORT)
int 
bcmi_td2_switch_wb_hash_entry_reinit(int unit, uint8 **scache_ptr)
{
    int rv;
    int i;
    int num_hash_entry;
    int hw_index;
    int wb_last_hash_entry_id;
    bcmi_xgs4_td2_switch_wb_hash_entry_t *hash_entry_scache_p;
    uint32 *u32_scache_p;
    flex_hash_entry_p hash_entry = NULL;
    BCM_IF_ERROR_RETURN(bcm_td2_switch_hash_entry_init(unit));

    u32_scache_p = (uint32 *)(*scache_ptr);

    /* Number of UDFs created */
    num_hash_entry = *u32_scache_p;
    u32_scache_p++;

    _bcm_td2_switch_hash_bk_info[unit].api_ver = *u32_scache_p;
    u32_scache_p++;

    wb_last_hash_entry_id = *u32_scache_p;
    u32_scache_p++;

    hash_entry_scache_p =
        (bcmi_xgs4_td2_switch_wb_hash_entry_t *)u32_scache_p;

    for (i = 0; i < num_hash_entry; i++) {
        hw_index = hash_entry_scache_p->hw_idx;
        
        rv = _flex_hash_entry_alloc(unit, &hash_entry);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        hash_entry->entry_id = hash_entry_scache_p->entry_id;
        hash_entry->hw_idx = hash_entry_scache_p->hw_idx;
        hash_entry->offset_bitmap = hash_entry_scache_p->offset_bitmap;        
        _bcm_flex_hash_table_wb_read(unit, hash_entry, hash_entry_scache_p);
            
        _BCM_SWITCH_FLEX_IDX_USED_SET(unit, hw_index);

        rv = _flex_hash_entry_add(unit, hash_entry, hw_index);
        if (BCM_FAILURE(rv)) {
            _flex_hash_entry_destroy(unit, hash_entry);
            return rv;
        }

        hash_entry_scache_p++;
    }
    last_hash_entry_id = wb_last_hash_entry_id;
    *scache_ptr = (uint8 *)hash_entry_scache_p;

    return BCM_E_NONE; 
}
#if defined(BCM_TOMAHAWK_SUPPORT)
int
bcmi_th_switch_wb_hash_entry_reinit(int unit, uint8 **scache_ptr)
{
    flex_hash_entry_t *hash_entry = NULL;
    int fld;
    bcm_udf_id_t *udf_id_p = NULL;

    hash_entry = SWITCH_HASH_ENTRY_LIST(unit);
    udf_id_p = (bcm_udf_id_t *)(*scache_ptr);

    while (hash_entry != NULL) {
        for (fld = 0; fld < FLEX_HASH_MAX_OFFSET; fld++) {
            hash_entry->udf_id[fld] = *udf_id_p;
            udf_id_p++;
        }
        hash_entry = hash_entry->next;
    }

    return BCM_E_NONE;
}
#endif /*BCM_TOMAHAWK_SUPPORT*/


STATIC int
_flex_hash_entry_num(int unit)
{
    int num_hash_entry = 0;
   
    flex_hash_entry_t *hash_entry = SWITCH_HASH_ENTRY_LIST(unit);

    while (hash_entry != NULL) {
        hash_entry = hash_entry->next;
        num_hash_entry++;
    }
    return num_hash_entry;
}


int
bcmi_td2_switch_wb_hash_entry_sync(int unit, uint8 **scache_ptr)
{
    int fld;
    bcmi_xgs4_td2_switch_wb_hash_entry_t *hash_entry_scache_p;
    uint32 *u32_scache_p;
    flex_hash_entry_p hash_entry = NULL;
    int num_hash_entry = 0;
#if defined(BCM_TOMAHAWK_SUPPORT)
        bcm_udf_id_t *udf_id_p = NULL;
#endif

    num_hash_entry = _flex_hash_entry_num(unit);    
    hash_entry = SWITCH_HASH_ENTRY_LIST(unit);

    u32_scache_p = (uint32 *)(*scache_ptr);

    /* Number of hash entry created */
    *u32_scache_p = num_hash_entry;
    u32_scache_p++;

    *u32_scache_p = _bcm_td2_switch_hash_bk_info[unit].api_ver;
    u32_scache_p++;

    *u32_scache_p = last_hash_entry_id;
    u32_scache_p++;

    hash_entry_scache_p =
        (bcmi_xgs4_td2_switch_wb_hash_entry_t *)u32_scache_p;

    while (hash_entry != NULL) {
        hash_entry_scache_p->entry_id = hash_entry->entry_id;
        hash_entry_scache_p->hw_idx = hash_entry->hw_idx;
        hash_entry_scache_p->offset_bitmap = hash_entry->offset_bitmap;
        for (fld = 0; fld < FLEX_HASH_MAX_FIELDS; fld++) {
            hash_entry_scache_p->match_arr[fld].qual_id= hash_entry->match_arr[fld].qual_id;
            hash_entry_scache_p->match_arr[fld].qual_size= hash_entry->match_arr[fld].qual_size;
        }
        hash_entry_scache_p++;
        
        hash_entry = hash_entry->next;
    }
    *scache_ptr = (uint8 *)u32_scache_p;

#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        udf_id_p = (bcm_udf_id_t *)hash_entry_scache_p;
        hash_entry = SWITCH_HASH_ENTRY_LIST(unit);
        while (hash_entry != NULL) {
            for (fld = 0; fld < FLEX_HASH_MAX_OFFSET; fld++) {
                *udf_id_p = hash_entry->udf_id[fld];
                udf_id_p++;
            }
            hash_entry = hash_entry->next;
        }
        /* overwrite scache_ptr*/
        *scache_ptr = (uint8 *)udf_id_p;
    }
#endif /*BCM_TOMAHAWK_SUPPORT*/

    return BCM_E_NONE;
}

/* If this size be changed, please don't forget to change udf wb version simultaneously */
int
bcmi_td2_switch_wb_hash_entry_scache_size_get(int unit, int *req_scache_size)
{
    int alloc_size = 0;
    int hash_entries = 0;

    alloc_size += sizeof(hash_entries);
    alloc_size += sizeof(_bcm_td2_switch_hash_bk_info[unit].api_ver);
    alloc_size += sizeof(last_hash_entry_id);
    /* Number of  hash entries*/
    hash_entries = soc_mem_index_count(unit, FLEX_HASH_TCAM);
    
    alloc_size += (hash_entries *
                   sizeof(bcmi_xgs4_td2_switch_wb_hash_entry_t));

    *req_scache_size += alloc_size;

    return BCM_E_NONE;
}
#endif 

/*
 * 
 *  Flexible hash Switch API support routines 
 *
 */
int
bcm_td2_switch_hash_entry_detach(int unit)
{

    if (SWITCH_HASH_ENTRY_LIST(unit)) {
        _flex_hash_entry_destroy_all(unit);
    }

    if (SWITCH_HASH_IDX_BITMAP(unit)) {
        sal_free(SWITCH_HASH_IDX_BITMAP(unit));
        SWITCH_HASH_IDX_BITMAP(unit) = NULL;
    }

    SWITCH_HASH_INIT(unit) = FALSE;

    return BCM_E_NONE;
}

int
bcm_td2_switch_hash_entry_init(int unit)
{
    int             hash_entries;

    if (SWITCH_HASH_INIT(unit) == TRUE ) {
        bcm_td2_switch_hash_entry_detach(unit);
    }


    if (SWITCH_HASH_IDX_BITMAP(unit)) {

    }
    /* Initialize bookeeping */
    sal_memset(SWITCH_HASH_INFO(unit), 0, sizeof(*SWITCH_HASH_INFO(unit)));

    /* Initialize the hash entry list */
    SWITCH_HASH_ENTRY_LIST(unit) = NULL;

    hash_entries = soc_mem_index_count(unit, FLEX_HASH_TCAM);
    SWITCH_HASH_IDX_BITMAP(unit) = sal_alloc(SHR_BITALLOCSIZE(hash_entries),
                         "hash index bitmap");
    if (NULL == SWITCH_HASH_IDX_BITMAP(unit)) {
        bcm_td2_switch_hash_entry_detach(unit);
        return BCM_E_FAIL;
    }
    sal_memset(SWITCH_HASH_IDX_BITMAP(unit), 0, SHR_BITALLOCSIZE(hash_entries));

    SWITCH_HASH_INIT(unit) = TRUE;

    last_hash_entry_id = 0;

    return BCM_E_NONE;
}


/*
 * Function: 
 *      bcm_td2_switch_hash_entry_create
 * Purpose: 
 *      Create a blank flex hash entry.
 * Parameters: 
 *      unit       - (IN) bcm device
 *      group      - (IN)  BCM field group
 *      *entry     - (OUT) BCM hash entry
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_create(int unit,
                 bcm_field_group_t group,
                 bcm_hash_entry_t * entry_id)
{
    int             rv = BCM_E_NONE;
    bcm_field_qset_t qset;
    int             qual_array[FLEX_HASH_MAX_FIELDS];
    int             qual_count;


    /* Validate unit, group of qualifier Ids */
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    rv = bcm_esw_field_group_get(unit, group, &qset);
    if (BCM_FAILURE(rv)) {
        return BCM_E_BADID;
    }

    /* Either this API or the create_qset should be used
     * Users should not intermix the APIs.
     */
    if (SWITCH_HASH_INFO(unit)->api_ver == 0) {
        SWITCH_HASH_INFO(unit)->api_ver = 1;
    } else if (SWITCH_HASH_INFO(unit)->api_ver == 2) {
        return BCM_E_CONFIG;
    }

    /* 
     * Check for the required qualifier fields, the assumption is qual_array
     * has the same sequence of offsets from UDF2.4 .. UDF2.7 for flex hash
     */
    rv = bcm_esw_field_qset_data_qualifier_get(unit, qset,
                           FLEX_HASH_MAX_FIELDS,
                           qual_array, &qual_count);
    if (BCM_FAILURE(rv)) {
        return BCM_E_FAIL;
    }

    if (0 == qual_count) {
        return BCM_E_FAIL;
    }

    /* Create flexible hash entry */
    /* Get free index from the flex hash tcam */
    BCM_IF_ERROR_RETURN(_bcm_hash_entry_create(unit, entry_id));

    /* Add qualifier ids in the hash entry */
    BCM_IF_ERROR_RETURN(_bcm_hash_entry_qual_set
                (unit, *entry_id, qual_count, qual_array));
    return rv;
}

/*
 * Function: 
 *      bcm_td2_switch_hash_entry_create_qset
 * Purpose: 
 *      Create a blank flex hash entry.
 * Parameters: 
 *      unit       - (IN)  bcm device
 *      qset       - (IN)  Qset
 *      *entry     - (OUT) BCM hash entry
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_create_qset(int unit,
                                      bcm_field_qset_t qset,
                                      bcm_hash_entry_t * entry_id)
{
    int             rv = BCM_E_NONE;
    int             qual_array[FLEX_HASH_MAX_FIELDS];
    int             qual_count;

    /* Either this API or the create_qset should be used
     * Users should not intermix the APIs.
     */
    if (SWITCH_HASH_INFO(unit)->api_ver == 0) {
        SWITCH_HASH_INFO(unit)->api_ver = 2;
    } else if (SWITCH_HASH_INFO(unit)->api_ver == 1) {
        return BCM_E_CONFIG;
    }

    /*
     * Check for the required qualifier fields, the assumption is qual_array
     * has the same sequence of offsets from UDF2.4 .. UDF2.7 for flex hash
     */
    rv = bcm_esw_field_qset_id_multi_get(unit, qset, bcmFieldQualifyUdf,
                                    FLEX_HASH_MAX_FIELDS,
                                    qual_array, &qual_count);
    BCM_IF_ERROR_RETURN(rv);

    if (0 == qual_count) {
        return BCM_E_PARAM;
    }

    /* Create flexible hash entry */
    /* Get free index from the flex hash tcam */
    BCM_IF_ERROR_RETURN(_bcm_hash_entry_create(unit, entry_id));

    /* Add qualifier ids in the hash entry */
    BCM_IF_ERROR_RETURN(_bcm_hash_entry_udf_set
                        (unit, *entry_id, qual_count, qual_array));

    return rv;
}


/*
 * Function: 
 *      bcm_td2_switch_hash_entry_destroy
 * Purpose: 
 *      Destroy a flex hash entry.
 * Parameters: 
 *      unit       - (IN) bcm device
 *      entry_id   - (IN) BCM hash entry
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_destroy(int unit, bcm_hash_entry_t entry_id)
{

    return _bcm_hash_entry_delete(unit, entry_id);
}

/*
 * Function:
 *      bcm_td2_switch_hash_entry_set
 * Purpose:
 *      Set a flex hash entry configuration information.
 * Parameters:
 *      unit       - (IN) BCM device
 *      entry      - (IN) BCM hash entry
 *      config     - (IN) BCM hash configure information
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_td2_switch_hash_entry_set(int unit,
                              bcm_hash_entry_t entry, 
                              bcm_switch_hash_entry_config_t *config)
{
    int mask_bit_max;
    uint32 mask;
#ifdef LE_HOST
    uint32 mask_swap;
#endif /* LE_HOST */
    flex_hash_entry_t *hash_entry;
    uint8 *p;
#if defined(BCM_TOMAHAWK_SUPPORT)
    bcmi_xgs4_udf_offset_info_t *offset_info = NULL;
    int i, gran, pos, chunk_pos = 0;
#endif

    if (config == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _flex_hash_entry_get(unit, entry, &hash_entry));

#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        /* In TH2 bcmSwitchFlexHashField2~12 only support udf type. */
        if ((config->field >= bcmSwitchFlexHashFieldCount) ||
            ((config->field > bcmSwitchFlexHashField1) &&
             !(config->flags & BCM_SWITCH_HASH_FIELD_TYPE_UDF))) {
            return BCM_E_PARAM;
        }
    } else
#endif /*BCM_TOMAHAWK2_SUPPORT*/
    {
        /* In TH and TD2 only first two fields are supported. */
        if (config->field > bcmSwitchFlexHashField1) {
            return BCM_E_PARAM;
        }
    }

    /*Only TH series support UDF type flex Hash*/
    if (!SOC_IS_TOMAHAWKX(unit) &&
        config->flags & BCM_SWITCH_HASH_FIELD_TYPE_UDF) {
        return BCM_E_UNAVAIL;
    }
    mask_bit_max = soc_mem_field_length(unit, FLEX_HASH_OFFSET_MASK(unit),
                             _bcm_td2_flex_hash_mask[config->field]);

    if (config->mask_length > BITS2BYTES(mask_bit_max)
        || config->mask_length > sizeof(mask)) {
        return BCM_E_PARAM;
    }

    mask = 0;
    if (config->mask_length) {
        p = (uint8 *)&mask;
        p += sizeof(mask) - config->mask_length;
        sal_memcpy(p, config->mask, config->mask_length);
    }

#ifdef LE_HOST
    mask_swap = _shr_swap32(mask);
    mask = mask_swap;
#endif /* LE_HOST */

#if defined(BCM_TOMAHAWK_SUPPORT)
    /*Extract udf chunk id from udf_id*/
    if (config->flags & BCM_SWITCH_HASH_FIELD_TYPE_UDF) {
        if ((config->udf_id < BCMI_XGS4_UDF_ID_MIN) ||
            (config->udf_id > BCMI_XGS4_UDF_ID_MAX)) {
            return BCM_E_PARAM;
        }
        hash_entry->type[config->field] |= BCM_SWITCH_HASH_FIELD_TYPE_UDF;
        hash_entry->udf_id[config->field] = config->udf_id;
        gran = BCMI_XGS4_UDF_CTRL_OFFSET_GRAN(unit);
        BCM_IF_ERROR_RETURN
            (bcmi_xgs4_udf_offset_node_get(unit, config->udf_id, &offset_info));
        /*Sanity check, the offset should start at even boundary of a packet */
        if ((offset_info->width < config->offset / 8 + gran) ||
            (offset_info->start + config->offset / 8) % gran) {
            return BCM_E_PARAM;
        }
        /*get the chunk id which the offset points to*/
        chunk_pos = 0;
        pos = offset_info->start % gran + (config->offset / 8) / gran;
        for (i = 0, chunk_pos = 0; i < MAX_UDF_OFFSET_CHUNKS; i++) {
            if (offset_info->hw_bmap & (1 << i)) {
                if (chunk_pos == pos) {
                break;
            }
            chunk_pos++;
          }
        }
        hash_entry->hash_offset[config->field] = i;
    } else
#endif
    {
        hash_entry->hash_offset[config->field] = config->offset & 0xf;
    }
    hash_entry->hash_mask[config->field] = (int)mask;
    if (mask) {
        hash_entry->offset_bitmap |=  (0x1 << config->field);
    } else {
        hash_entry->offset_bitmap &= ~(0x1 << config->field);
    }
    hash_entry->offset_count = _shr_popcount(hash_entry->offset_bitmap);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td2_switch_hash_entry_get
 * Purpose:
 *      Get a flex hash entry configure information.
 * Parameters:
 *      unit       - (IN) BCM device
 *      entry      - (IN) BCM hash entry
 *      config     - (IN/OUT) BCM hash configure information
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_get(int unit,
                              bcm_hash_entry_t entry, 
                              bcm_switch_hash_entry_config_t *config)
{
    int i, mask_len;
    uint32 mask;
#ifdef LE_HOST
    uint32 mask_swap;
#endif /* LE_HOST */
    flex_hash_entry_t *hash_entry;
#if defined(BCM_TOMAHAWK_SUPPORT)
    int gran, position = 0;
    bcmi_xgs4_udf_offset_info_t *offset_info = NULL;
#endif /*BCM_TOMAHAWK_SUPPORT*/

    if (config == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _flex_hash_entry_get(unit, entry, &hash_entry));

#if defined(BCM_TOMAHAWK2_SUPPORT)
    if (SOC_IS_TOMAHAWK2(unit)) {
        if (config->field >= bcmSwitchFlexHashFieldCount) {
            return BCM_E_PARAM;
        }
    } else
#endif
    {
        if (config->field > bcmSwitchFlexHashField1) {
            return BCM_E_PARAM;
        }
    }

    mask_len = 0;
    mask = (uint32)hash_entry->hash_mask[config->field];
    for (i = 0; i < sizeof(mask); i++) {
        if (mask & 0xff000000) {
            mask_len = sizeof(mask) - i;
            break;
        }
        mask <<= 8;
    }

    if (mask_len == 0) {
        return BCM_E_NOT_FOUND;
    }

#ifdef LE_HOST
    mask_swap = _shr_swap32(mask);
    mask = mask_swap;
#endif /* LE_HOST */

#if defined(BCM_TOMAHAWK_SUPPORT)
    /*Extract udf chunk id from udf_id*/
    if (hash_entry->type[config->field] & BCM_SWITCH_HASH_FIELD_TYPE_UDF) {
        config->udf_id = hash_entry->udf_id[config->field];
        gran = BCMI_XGS4_UDF_CTRL_OFFSET_GRAN(unit);
        BCM_IF_ERROR_RETURN
            (bcmi_xgs4_udf_offset_node_get(unit, config->udf_id, &offset_info));
        position = 0;
        for (i = 0; i < hash_entry->hash_offset[config->field]; i++) {
            if (SHR_BITGET(&(offset_info->hw_bmap), i)) {
                position++;
            }
        }
        /* offset gran is in bit, not byte. */
        config->offset = (position * gran - offset_info->start % gran) * 8;
    } else

#endif
    {
        config->offset = hash_entry->hash_offset[config->field];
    }
    config->flags = hash_entry->type[config->field];
    config->mask_length = mask_len;
    if (config->mask_length > 0) {
        sal_memcpy(config->mask, &mask, mask_len);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td2_switch_hash_entry_traverse
 * Purpose:
 *      Traverse flex hash entry tables.
 * Parameters: 
 *      unit       - (IN) BCM device
 *      cb_fn      - (IN) BCM traverse call back function
 *      user_data  - (IN) BCM user data
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_traverse(int unit, int flags, 
                                   bcm_switch_hash_entry_traverse_cb cb_fn,
                                   void *user_data)
{
    int rv;
    flex_hash_entry_t *hash_entry = SWITCH_HASH_ENTRY_LIST(unit);

    if (cb_fn == NULL) {
        return BCM_E_PARAM;
    }

    while (hash_entry != NULL) {
        rv = cb_fn(unit, flags, hash_entry->entry_id, user_data);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
        hash_entry = hash_entry->next;
    }

    return BCM_E_NONE;
}

/*
 * Function: 
 *      bcm_td2_switch_hash_entry_install
 * Purpose: 
 *      Install a flex hash entry into hardware tables.
 * Parameters: 
 *      unit       - (IN) bcm device
 *      entry      - (IN) BCM hash entry
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_install(int unit, bcm_hash_entry_t entry_id,
                                  uint32 offset, uint32 mask)
{
    int             rv = BCM_E_NONE;
    flex_hash_entry_t *hash_entry;

    rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    if (offset != BCM_SWITCH_HASH_USE_CONFIG) {
        if (++hash_entry->offset_count > FLEX_HASH_OFFSET_NUM(unit)) {
            return BCM_E_RESOURCE;
        }
        hash_entry->hash_offset[hash_entry->offset_count - 1] = offset & 0xf;
        hash_entry->hash_mask[hash_entry->offset_count - 1] = mask & 0xffff;
        hash_entry->offset_bitmap |= 0x1 << (hash_entry->offset_count - 1);
    }

#ifdef BCM_TOMAHAWK2_SUPPORT
    if (SOC_IS_TOMAHAWK2(unit)) {
        rv = _bcm_th2_flex_hash_table_write(unit, hash_entry, FALSE);
    } else
#endif
    {
        rv = _bcm_flex_hash_table_write(unit, hash_entry, FALSE);
    }

    if (BCM_FAILURE(rv)) {
        return rv;
    }

    return rv;
}

/*
 * Function: 
 *      bcm_td2_switch_hash_entry_reinstall
 * Purpose: 
 *      Re-install a flex hash entry into hardware tables.
 * Parameters: 
 *      unit       - (IN) bcm device
 *      entry      - (IN) BCM hash entry
 *      offset     - (IN) BCM hash offset
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_reinstall(int unit,
                    bcm_hash_entry_t entry_id, uint32 offset, uint32 mask)
{
    int             cnt, rv = BCM_E_NONE, reinstall;
    flex_hash_entry_t *hash_entry;

    if (offset != BCM_SWITCH_HASH_USE_CONFIG) {
        rv = _flex_hash_entry_get(unit, entry_id, &hash_entry);
        if (BCM_FAILURE(rv)) {
            return rv;
        }

        reinstall = 1;
        for (cnt = 0; cnt < FLEX_HASH_OFFSET_NUM(unit); cnt++) {
            if (hash_entry->hash_offset[cnt] == -1) {
                reinstall = 0;
            }
        }
        if ((reinstall == 1) && (cnt == FLEX_HASH_OFFSET_NUM(unit))) {
            for (cnt = 0; cnt < FLEX_HASH_OFFSET_NUM(unit); cnt++) {
                hash_entry->hash_offset[cnt] = -1;
                hash_entry->offset_count = 0;
                hash_entry->offset_bitmap = 0;
            }
        }
    }

    return bcm_td2_switch_hash_entry_install(unit, entry_id, offset, mask);
}

/*
 * Function: 
 *      bcm_td2_switch_hash_entry_remove
 * Purpose: 
 *      Remove a flex hash entry from hardware tables.
 * Parameters: 
 *      unit       - (IN) bcm device
 *      entry      - (IN) BCM hash entry
 *
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td2_switch_hash_entry_remove(int unit, bcm_hash_entry_t entry_id)
{
    return _bcm_hash_entry_remove(unit, entry_id);;
}

/*
 * Function:
 *      bcm_td2_switch_hash_qualify_data
 * Purpose:
 *      Add flex hash field qualifiers into hardware tables.
 * Parameters:
 *      unit       - (IN) bcm device
 *      entry      - (IN) BCM hash entry
 *      qual_id    - (IN) BCM qualifier id
 *      data       - (IN) BCM hash field qualifier
 *      mask       - (IN) BCM hash field mask
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_td2_switch_hash_qualify_data(int unit, bcm_hash_entry_t entry_id,
                 int qual_id, uint32 data, uint32 mask)
{
    int             rv = BCM_E_NONE;

    rv = _bcm_hash_entry_qual_data_set(unit, entry_id, qual_id, data, mask);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    return rv;
}

/*
 * Function:
 *      bcm_td2_switch_hash_qualify_udf
 * Purpose:
 *      Add flex hash udf qualifiers into hardware tables.
 * Parameters: 
 *      unit       - (IN) bcm device
 *      entry      - (IN) BCM hash entry
 *      udf_id     - (IN) BCM qualifier id
 *      length     - (IN) BCM qualifier id
 *      data       - (IN) BCM hash field qualifier
 *      mask       - (IN) BCM hash field mask
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_td2_switch_hash_qualify_udf(int unit,
                                bcm_hash_entry_t entry, 
                                bcm_udf_id_t udf_id,
                                int length, 
                                uint8 *data, uint8 *mask)
{
    int i, idx, len;
    int id, chunk_base, chunk_size, chunk_cnt;
    flex_hash_entry_t *hash_entry;
    bcmi_xgs4_udf_offset_info_t *offset_info;
    uint8 *p;
    uint32 qual_match, qual_mask; 
#ifdef LE_HOST
    uint32 qual_match_swap, qual_mask_swap; 
#endif /* LE_HOST */

    if (data == NULL || mask == NULL) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(
        _flex_hash_entry_get(unit, entry, &hash_entry));
    chunk_base = 12; /* UDF2.4 */
    chunk_size = 2;
    /* Get data qualifier info. */
    BCM_IF_ERROR_RETURN(
        bcmi_xgs4_udf_offset_node_get(unit, udf_id, &offset_info));

    if (!(offset_info->flags & BCMI_XGS4_UDF_OFFSET_FLEXHASH) ||
        !(offset_info->hw_bmap & (0xf << chunk_base)) ||
        !(offset_info->width > 0)) {
        return BCM_E_PARAM;
    }
    if (length > offset_info->width) {
        return BCM_E_PARAM;
    }
    for (id = 0; id < FLEX_HASH_MAX_FIELDS; id++) {
        if (hash_entry->match_arr[id].qual_id == udf_id) {
            hash_entry->match_arr[id].qual_size = 0;
            break;
        }
    }
    if (id == FLEX_HASH_MAX_FIELDS) {
        return BCM_E_PARAM;
    }
    chunk_cnt  = (offset_info->width + chunk_size - 1) / chunk_size;
    if ((id + chunk_cnt) > FLEX_HASH_MAX_FIELDS) {
        return BCM_E_PARAM;
    }

    for (i = 0; i < length; i += chunk_size) {
        idx = id + i / chunk_size;
        len = ((length - i) >= chunk_size) ? chunk_size : (length - i);
        qual_match = qual_mask = 0;

        p = (uint8 *)&qual_match;
        p += sizeof(uint32) - len;
        sal_memcpy(p, &data[i], len);
        p = (uint8 *)&qual_mask;
        p += sizeof(uint32) - len;
        sal_memcpy(p, &mask[i], len);

#ifdef LE_HOST
        qual_match_swap = _shr_swap32(qual_match);
        qual_match      = qual_match_swap;
        qual_mask_swap  = _shr_swap32(qual_mask);
        qual_mask       = qual_mask_swap;
#endif /* LE_HOST */

        hash_entry->match_arr[idx].qual_match = qual_match;
        hash_entry->match_arr[idx].qual_mask  = qual_mask;
        hash_entry->match_arr[idx].qual_size  = len;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td2_switch_hash_qualify_udf_get
 * Purpose:
 *      Get flex hash udf qualifiers' length, data and mask.
 * Parameters:
 *      unit       - (IN) bcm device
 *      entry      - (IN) BCM hash entry
 *      udf_id     - (IN) BCM qualifier id
 *      length     - (OUT) BCM qualifier id
 *      data       - (OUT) BCM hash field qualifier
 *      mask       - (OUT) BCM hash field mask
 * Returns:
 *      BCM_E_NONE - Success
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_td2_switch_hash_qualify_udf_get(int unit, 
                                    bcm_hash_entry_t entry, 
                                    bcm_udf_id_t udf_id,
                                    int max_length, 
                                    uint8 *data, uint8 *mask,
                                    int *actual_length)
{
    int i, idx, len, len_tmp = 0;
    int id, chunk_base, chunk_size, chunk_cnt;
    flex_hash_entry_t *hash_entry;
    bcmi_xgs4_udf_offset_info_t *offset_info;
    uint8 *p;
    uint32 qual_match, qual_mask; 
#ifdef LE_HOST
    uint32 qual_match_swap, qual_mask_swap; 
#endif /* LE_HOST */

    if (data == NULL || mask == NULL || actual_length == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _flex_hash_entry_get(unit, entry, &hash_entry));

    chunk_base = 12; /* UDF2.4 */
    chunk_size = 2;
    /* Get data qualifier info. */
    BCM_IF_ERROR_RETURN(
        bcmi_xgs4_udf_offset_node_get(unit, udf_id, &offset_info));
    if (!(offset_info->flags & BCMI_XGS4_UDF_OFFSET_FLEXHASH) ||
        !(offset_info->hw_bmap & (0xf << chunk_base))) {
        return BCM_E_PARAM;
    }

    if (max_length < offset_info->width) {
        return BCM_E_PARAM;
    }

    for (id = 0; id < FLEX_HASH_MAX_FIELDS; id++) {
        if (hash_entry->match_arr[id].qual_id == udf_id) {
            break;
        }
    }
    if (id >= FLEX_HASH_MAX_FIELDS) {
        return BCM_E_NOT_FOUND;
    }

    chunk_cnt  = (offset_info->width + chunk_size - 1) / chunk_size;
    if ((id + chunk_cnt) > FLEX_HASH_MAX_FIELDS) {
        return BCM_E_INTERNAL;
    }

    for (i = 0; i < offset_info->width; i += chunk_size) {
        idx = id + i / chunk_size;
        len = hash_entry->match_arr[idx].qual_size;
        if (len) {
            qual_match = hash_entry->match_arr[idx].qual_match;
            qual_mask  = hash_entry->match_arr[idx].qual_mask;

#ifdef LE_HOST
            qual_match_swap = _shr_swap32(qual_match);
            qual_match      = qual_match_swap;
            qual_mask_swap  = _shr_swap32(qual_mask);
            qual_mask       = qual_mask_swap;
#endif /* LE_HOST */

            p = (uint8 *)&qual_match;
            p += sizeof(uint32) - len;
            sal_memcpy(&data[i], p, len);
            p = (uint8 *)&qual_mask;
            p += sizeof(uint32) - len;
            sal_memcpy(&mask[i], p, len);
            len_tmp += len;
        }
    }
    
    *actual_length = len_tmp;

    return BCM_E_NONE;
}

/*
 * Function:
 *   _bcm_td2_switch_match_free_resources
 * Purpose:
 *   Free match bookkeeping resources associated with a given unit.  This
 *   happens either during an error initializing or during a detach operation.
 * Parameters:
 *   unit - (IN) Unit number
 * Returns:
 *   BCM_E_XXX
 */
STATIC void
_bcm_td2_switch_match_free_resources(int unit)
{
    _bcm_td2_switch_match_bookkeeping_t *match_info = TD2_MATCH_INFO(unit);
    int i;

    if (match_info->gtp_ptr_array) {
        for (i = 0; i < soc_mem_index_count(unit, GTP_PORT_TABLEm); i++) {
            if (match_info->gtp_ptr_array[i]) {
                sal_free(match_info->gtp_ptr_array[i]);
            }
        }
        sal_free(match_info->gtp_ptr_array);
        match_info->gtp_ptr_array = NULL;
    }

    if (match_info->match_mutex) {
        sal_mutex_destroy(match_info->match_mutex);
        match_info->match_mutex = NULL;
    }
}

/*
 * Function:
 *   bcm_td2_switch_match_detach
 * Purpose:
 *   Detach the TD2 match resource. Called when the module should de-initialize.
 * Parameters:
 *   unit - (IN) Unit being detached.
 * Returns:
 *   BCM_E_XXX
 */
int
bcm_td2_switch_match_detach(int unit)
{
    if (_bcm_td2_match_initialized[unit]) {
        _bcm_td2_switch_match_free_resources(unit);
        _bcm_td2_match_initialized[unit] = 0;
    }

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT

/*
 * Function:
 *   _bcmi_td2_gtp_match_reinit
 * Purpose:
 *   Restore GTP match entries
 * Parameters:
 *   unit - (IN) Unit being initialized
 * Returns:
 *   BCM_E_XXX
 */
STATIC int
_bcm_td2_gtp_port_match_reinit(int unit)
{
    int hw_id, table_size;
    gtp_port_table_entry_t gtp_port_table_entry;
    int match_type;
    _bcm_td2_gtp_port_match_t *gtp_port_match;

    table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);

    for (hw_id = 0; hw_id < table_size; hw_id++) {
        sal_memset(&gtp_port_table_entry, 0, sizeof(gtp_port_table_entry_t));
        BCM_IF_ERROR_RETURN(
            soc_mem_read(unit, GTP_PORT_TABLEm, MEM_BLOCK_ANY,
                         hw_id, &gtp_port_table_entry));

        match_type = soc_mem_field32_get(unit, GTP_PORT_TABLEm,
                                         &gtp_port_table_entry, MATCH_ENABLEf);
        if (match_type == 0) {
            continue;
        }

        gtp_port_match = sal_alloc(sizeof(_bcm_td2_gtp_port_match_t),
                                   "gtp_port_match");
        if (gtp_port_match == NULL) {
            return BCM_E_MEMORY;
        }

        gtp_port_match->match_criteria = match_type;
        gtp_port_match->src_port = soc_mem_field32_get(unit, GTP_PORT_TABLEm,
                                                       &gtp_port_table_entry,
                                                       SRC_PORTf);
        gtp_port_match->dst_port = soc_mem_field32_get(unit, GTP_PORT_TABLEm,
                                                       &gtp_port_table_entry,
                                                       DST_PORTf);

        TD2_GTP_PORT_MATCH(unit, hw_id) = gtp_port_match;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *   bcm_td2_switch_match_reinit
 * Purpose:
 *   Re-initialize the TD2 match resourse. Restore bookkeeping information.
 * Parameters:
 *   unit - (IN) Unit being initialized
 * Returns:
 *   BCM_E_XXX
 */
STATIC int
_bcm_td2_switch_match_reinit(int unit)
{
    BCM_IF_ERROR_RETURN(_bcm_td2_gtp_port_match_reinit(unit));

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *   bcmi_td2_switch_match_init
 * Purpose:
 *   Initialize the TD2 match resourse.  Allocate bookkeeping information.
 * Parameters:
 *   unit - (IN) Unit being initialized
 * Returns:
 *   BCM_E_XXX
 */
int
bcm_td2_switch_match_init(int unit)
{
    _bcm_td2_switch_match_bookkeeping_t *match_info = TD2_MATCH_INFO(unit);
    int gtp_port_table_size;
#ifdef BCM_WARM_BOOT_SUPPORT
    int rv;
#endif /* BCM_WARM_BOOT_SUPPORT */

    if (_bcm_td2_match_initialized[unit]) {
        BCM_IF_ERROR_RETURN(bcm_td2_switch_match_detach(unit));
    }

    sal_memset(match_info, 0, sizeof(_bcm_td2_switch_match_bookkeeping_t));
    gtp_port_table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);

    match_info->gtp_ptr_array =
        sal_alloc(sizeof(_bcm_td2_gtp_port_match_t *) * gtp_port_table_size,
                  "gtp_ptr_array");
    if (match_info->gtp_ptr_array == NULL) {
        _bcm_td2_switch_match_free_resources(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(match_info->gtp_ptr_array, 0,
               sizeof(_bcm_td2_gtp_port_match_t *) * gtp_port_table_size);

    /* Create mutex */
    match_info->match_mutex = sal_mutex_create("match_mutex");
    if (match_info->match_mutex == NULL) {
        _bcm_td2_switch_match_free_resources(unit);
        return BCM_E_MEMORY;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        /* Warm Boot recovery */
        rv = _bcm_td2_switch_match_reinit(unit);
        if (BCM_FAILURE(rv)) {
            _bcm_td2_switch_match_free_resources(unit);
            return rv;
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    _bcm_td2_match_initialized[unit] = 1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td2_switch_match_id_check
 * Purpose:
 *      Check match_id is valid
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_bcm_td2_switch_match_id_check(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id)
{
    int match_type, table_size;
    uint32 hw_id = TD2_MATCH_HW_ID_GET(match_id);

    if (match_service != bcmSwitchMatchServiceGtp) {
        return BCM_E_PARAM;
    }
    table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);
    match_type = TD2_MATCH_TYPE_GET(match_id);

    if ((match_type <= BCM_SWITCH_GTP_MATCH_DST_PORT) ||
        (match_type > BCM_SWITCH_GTP_MATCH_SRC_AND_DST_PORT)) {
        return BCM_E_PARAM;
    }

    if (hw_id >= table_size) {
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_td2_gtp_port_match_config_find(
    int unit,
    bcm_switch_match_service_t match_service,
    bcm_switch_match_config_info_t config_info,
    int *match_id)
{
    int rv = BCM_E_NOT_FOUND;
    int i, table_size;

   table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);

    for (i = 0; i < table_size; i++) {
        if (!TD2_GTP_PORT_MATCH_IS_USED(unit, i)) {
            continue;
        }
        if (config_info.match_criteria !=
            TD2_GTP_PORT_MATCH(unit, i)->match_criteria) {
            continue;
        }
        switch (config_info.match_criteria) {
        case BCM_SWITCH_GTP_MATCH_DST_PORT:
            if (config_info.dst_port ==
                TD2_GTP_PORT_MATCH(unit, i)->dst_port) {
                *match_id = TD2_MATCH_ID_SET(i, bcmSwitchMatchServiceGtp);
                return BCM_E_NONE;
            }
            break;
        case BCM_SWITCH_GTP_MATCH_SRC_PORT:
            if (config_info.src_port ==
                TD2_GTP_PORT_MATCH(unit, i)->src_port) {
                *match_id = TD2_MATCH_ID_SET(i, bcmSwitchMatchServiceGtp);
                return BCM_E_NONE;
            }
            break;
        case BCM_SWITCH_GTP_MATCH_SRC_AND_DST_PORT:
        case BCM_SWITCH_GTP_MATCH_SRC_OR_DST_PORT:
            if ((config_info.dst_port ==
                 TD2_GTP_PORT_MATCH(unit, i)->dst_port) &&
                (config_info.src_port ==
                 TD2_GTP_PORT_MATCH(unit, i)->src_port)) {
                 *match_id = TD2_MATCH_ID_SET(i, bcmSwitchMatchServiceGtp);
                 return BCM_E_NONE;
            }
            break;
        default:
            return BCM_E_PARAM;
            break;
        }
    }

    return rv;
}


/*
 * Function:
 *      _bcm_td2_custom_hdr_match_config_add
 * Purpose:
 *      Add a match config, match_id will be returned after match
 *      config created.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      config_info   - (IN) Match configuration
 *      match_id      - (OUT) Match ID
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_bcm_td2_gtp_port_match_config_add(
    int unit,
    bcm_switch_match_service_t match_service,
    bcm_switch_match_config_info_t *config_info,
    int *match_id)
{
    int rv = BCM_E_NONE;
    int i, table_size;
    int exist_match_id;
    int src_port = 0, dst_port = 0;
    gtp_port_table_entry_t gtp_port_entry;

    if ((config_info->match_criteria < BCM_SWITCH_GTP_MATCH_DST_PORT) ||
        (config_info->match_criteria > BCM_SWITCH_GTP_MATCH_SRC_AND_DST_PORT)) {
        return BCM_E_PARAM;
    }

    rv = _bcm_td2_gtp_port_match_config_find(unit, bcmSwitchMatchServiceGtp,
                                             *config_info, &exist_match_id);
    if (rv == BCM_E_NONE) {
        return BCM_E_EXISTS;
    }

    table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);
    for (i = 0; i < table_size; i++) {
        if (!TD2_GTP_PORT_MATCH_IS_USED(unit, i)) {
            break;
        }
    }

    if (i == table_size) {
        return BCM_E_FULL;
    }

    switch (config_info->match_criteria) {
    case BCM_SWITCH_GTP_MATCH_DST_PORT:
        dst_port = config_info->dst_port;
        src_port = 0;
        break;
    case BCM_SWITCH_GTP_MATCH_SRC_PORT:
        src_port = config_info->src_port;
        dst_port = 0;
        break;
    case BCM_SWITCH_GTP_MATCH_SRC_AND_DST_PORT:
    case BCM_SWITCH_GTP_MATCH_SRC_OR_DST_PORT:
        src_port = config_info->src_port;
        dst_port = config_info->dst_port;
        break;
    default:
        return BCM_E_PARAM;
        break;
    }

    sal_memset(&gtp_port_entry, 0, sizeof(gtp_port_table_entry_t));
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        MATCH_ENABLEf, config_info->match_criteria);
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        SRC_PORTf, src_port);
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        DST_PORTf, dst_port);
    SOC_IF_ERROR_RETURN(
        soc_mem_write(unit, GTP_PORT_TABLEm, MEM_BLOCK_ALL, i, &gtp_port_entry));

    TD2_GTP_PORT_MATCH(unit, i) =
        sal_alloc(sizeof(_bcm_td2_gtp_port_match_t), "gtp_port_match");
    if (TD2_GTP_PORT_MATCH(unit, i) == NULL) {
        return BCM_E_MEMORY;
    }
    TD2_GTP_PORT_MATCH(unit, i)->match_criteria = config_info->match_criteria;
    TD2_GTP_PORT_MATCH(unit, i)->dst_port = dst_port;
    TD2_GTP_PORT_MATCH(unit, i)->src_port = src_port;
    *match_id = TD2_MATCH_ID_SET(i, bcmSwitchMatchServiceGtp);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td2_switch_match_config_add
 * Purpose:
 *      Add a match config to hardware.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      config_info   - (IN) Match configuration
 *      match_id      - (OUT) Match ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcm_td2_switch_match_config_add(
    int unit,
    bcm_switch_match_service_t match_service,
    bcm_switch_match_config_info_t *config_info,
    int *match_id)
{
    int rv = BCM_E_NONE;

    TD2_MATCH_INIT(unit);
    TD2_MATCH_LOCK(unit);

    if (match_service == bcmSwitchMatchServiceGtp) {
        rv = _bcm_td2_gtp_port_match_config_add(unit, match_service,
                                                config_info, match_id);
    }

    TD2_MATCH_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *     _bcm_td2_custom_hdr_match_config_delete
 * Purpose:
 *     Destroy a match ID by removing it from hardware.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_td2_gtp_port_match_config_delete(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id)
{
    int hw_id;
    gtp_port_table_entry_t gtp_port_entry;

    BCM_IF_ERROR_RETURN(
        _bcm_td2_switch_match_id_check(unit, match_service, match_id));

    hw_id = TD2_MATCH_HW_ID_GET(match_id);
    if (!TD2_GTP_PORT_MATCH_IS_USED(unit, hw_id)) {
        return BCM_E_NOT_FOUND;
    }

    sal_memset(&gtp_port_entry, 0, sizeof(gtp_port_table_entry_t));
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        MATCH_ENABLEf, 0);
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        SRC_PORTf, 0);
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        DST_PORTf, 0);
    SOC_IF_ERROR_RETURN(
        soc_mem_write(unit, GTP_PORT_TABLEm, MEM_BLOCK_ALL, hw_id,
                      &gtp_port_entry));

    sal_free(TD2_GTP_PORT_MATCH(unit, hw_id));
    TD2_GTP_PORT_MATCH(unit, hw_id) = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td2_switch_match_config_delete
 * Purpose:
 *      Delete a match config with the given match_id by removing it from
 *      hardware and deleting the associated software state info.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcm_td2_switch_match_config_delete(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id)
{
    int rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_td2_switch_match_id_check(unit, match_service, match_id));

    TD2_MATCH_INIT(unit);
    TD2_MATCH_LOCK(unit);

    if (match_service == bcmSwitchMatchServiceGtp) {
        rv = _bcm_td2_gtp_port_match_config_delete(
                unit, match_service, match_id);
    }

    TD2_MATCH_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_td2_switch_match_config_delete_all
 * Purpose:
 *      Delete all match config with the given match_service.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 * Returns:
 *      BCM_E_xxx
 */
int
bcm_td2_switch_match_config_delete_all(
    int unit,
    bcm_switch_match_service_t match_service)
{
    int rv = BCM_E_NONE;
    int i, table_size;

    TD2_MATCH_INIT(unit);
    TD2_MATCH_LOCK(unit);

    if (match_service == bcmSwitchMatchServiceGtp) {
        table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);
        rv = soc_mem_clear(unit, GTP_PORT_TABLEm, MEM_BLOCK_ANY, TRUE);
        if (SOC_FAILURE(rv)) {
            TD2_MATCH_UNLOCK(unit);
            return rv;
        }
        for (i = 0; i < table_size; i++) {
            if (TD2_GTP_PORT_MATCH_IS_USED(unit,i)) {
                sal_free(TD2_GTP_PORT_MATCH(unit,i));
                TD2_GTP_PORT_MATCH(unit,i) = NULL;
            }
        }
    }

    TD2_MATCH_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_td2_gtp_port_match_config_get
 * Purpose:
 *      Get the match configuration from hardware
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 *      config_info   - (OUT) Match configuration
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_bcm_td2_gtp_port_match_config_get(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id,
    bcm_switch_match_config_info_t *config_info)
{
    uint32 hw_id;

    hw_id = TD2_MATCH_HW_ID_GET(match_id);

    if (!TD2_GTP_PORT_MATCH_IS_USED(unit, hw_id)) {
        return BCM_E_NOT_FOUND;
    }

    bcm_switch_match_config_info_t_init(config_info);
    config_info->match_criteria = TD2_GTP_PORT_MATCH(unit, hw_id)->match_criteria;
    config_info->dst_port = TD2_GTP_PORT_MATCH(unit, hw_id)->dst_port;
    config_info->src_port = TD2_GTP_PORT_MATCH(unit, hw_id)->src_port;

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_td2_switch_match_config_get
 * Purpose:
 *      Get the configuration of the match with the given match_id.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 *      config_info   - (OUT) Match configuration
 * Returns:
 *      BCM_E_xxx
 */
int
bcm_td2_switch_match_config_get(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id,
    bcm_switch_match_config_info_t *config_info)
{
    int rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(
        _bcm_td2_switch_match_id_check(unit, match_service, match_id));

    TD2_MATCH_INIT(unit);
    TD2_MATCH_LOCK(unit);

    if (match_service == bcmSwitchMatchServiceGtp) {
        rv = _bcm_td2_gtp_port_match_config_get(
                unit, match_service, match_id, config_info);
    }

    TD2_MATCH_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_td2_gtp_port_match_config_set
 * Purpose:
 *      Set the match configuration from hardware
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 *      config_info   - (IN) Match configuration
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
_bcm_td2_gtp_port_match_config_set(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id,
    bcm_switch_match_config_info_t *config_info)
{
    int rv;
    uint32 hw_id;
    int exist_match_id;
    int src_port = 0, dst_port = 0;
    gtp_port_table_entry_t gtp_port_entry;

    hw_id = TD2_MATCH_HW_ID_GET(match_id);

    rv = _bcm_td2_gtp_port_match_config_find(unit, bcmSwitchMatchServiceGtp,
                                             *config_info, &exist_match_id);
    if (rv != BCM_E_NOT_FOUND) {
        if (rv == BCM_E_NONE) {
            if (exist_match_id != match_id) {
                return BCM_E_EXISTS;
            } else {
                return BCM_E_NONE;
            }
        } else {
            return rv;
        }
    }

    switch (config_info->match_criteria) {
    case BCM_SWITCH_GTP_MATCH_DST_PORT:
        dst_port = config_info->dst_port;
        src_port = 0;
        break;
    case BCM_SWITCH_GTP_MATCH_SRC_PORT:
        src_port = config_info->src_port;
        dst_port = 0;
        break;
    case BCM_SWITCH_GTP_MATCH_SRC_AND_DST_PORT:
    case BCM_SWITCH_GTP_MATCH_SRC_OR_DST_PORT:
        src_port = config_info->src_port;
        dst_port = config_info->dst_port;
        break;
    default:
        return BCM_E_PARAM;
        break;
    }

    sal_memset(&gtp_port_entry, 0, sizeof(gtp_port_table_entry_t));
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        MATCH_ENABLEf, config_info->match_criteria);
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        SRC_PORTf, src_port);
    soc_mem_field32_set(unit, GTP_PORT_TABLEm, &gtp_port_entry,
                        DST_PORTf, dst_port);
    SOC_IF_ERROR_RETURN(
        soc_mem_write(unit, GTP_PORT_TABLEm, MEM_BLOCK_ALL,
                      hw_id, &gtp_port_entry));

    if (!TD2_GTP_PORT_MATCH_IS_USED(unit, hw_id)) {
        TD2_GTP_PORT_MATCH(unit, hw_id) =
            sal_alloc(sizeof(_bcm_td2_gtp_port_match_t), "gtp_port_match");
        if (TD2_GTP_PORT_MATCH(unit, hw_id) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    TD2_GTP_PORT_MATCH(unit, hw_id)->match_criteria = config_info->match_criteria;
    TD2_GTP_PORT_MATCH(unit, hw_id)->dst_port = dst_port;
    TD2_GTP_PORT_MATCH(unit, hw_id)->src_port = src_port;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_td2_switch_match_config_set
 * Purpose:
 *      Set the configuration of the match with the given match_id.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      match_id      - (IN) Match ID
 *      config_info   - (IN) Match configuration
 * Returns:
 *      BCM_E_xxx
 */
int
bcm_td2_switch_match_config_set(
    int unit,
    bcm_switch_match_service_t match_service,
    int match_id,
    bcm_switch_match_config_info_t *config_info)
{
    int rv = BCM_E_NONE;

    if ((config_info->match_criteria < BCM_SWITCH_GTP_MATCH_DST_PORT) ||
        (config_info->match_criteria > BCM_SWITCH_GTP_MATCH_SRC_AND_DST_PORT)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(
        _bcm_td2_switch_match_id_check(unit, match_service, match_id));

    TD2_MATCH_INIT(unit);
    TD2_MATCH_LOCK(unit);

    if (match_service == bcmSwitchMatchServiceGtp) {
        rv = _bcm_td2_gtp_port_match_config_set(
                unit, match_service, match_id, config_info);
    }

    TD2_MATCH_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_td2_switch_match_config_traverse
 * Purpose:
 *      Traverse the created match config.
 * Parameters:
 *      unit          - (IN) Unit number
 *      match_service - (IN) Match service type
 *      cb_fn         - (IN) Traverse call back function
 *      user_data     - (IN) User data
 * Returns:
 *      BCM_E_xxx
 */
int
bcm_td2_switch_match_config_traverse(
    int unit,
    bcm_switch_match_service_t match_service,
    bcm_switch_match_config_traverse_cb cb_fn,
    void *user_data)
{
    uint32 match_id;
    int table_size = 0, i = 0;
    bcm_switch_match_config_info_t match_config;
    int rv = BCM_E_NONE;

    if (cb_fn == NULL) {
        return BCM_E_PARAM;
    }

    TD2_MATCH_INIT(unit);
    TD2_MATCH_LOCK(unit);

    table_size = soc_mem_index_count(unit, GTP_PORT_TABLEm);

    for (i = 0; i < table_size; i++) {
        if (TD2_GTP_PORT_MATCH_IS_USED(unit, i)) {
            match_id = TD2_MATCH_ID_SET(i, bcmSwitchMatchServiceGtp);
            rv = bcm_td2_switch_match_config_get(
                     unit, match_service, match_id, &match_config);
            if (BCM_FAILURE(rv)) {
                TD2_MATCH_UNLOCK(unit);
                return rv;
            }

            rv = cb_fn(unit, match_id, &match_config, user_data);
            if (BCM_FAILURE(rv)) {
                TD2_MATCH_UNLOCK(unit);
                return rv;
            }
        }
    }

    TD2_MATCH_UNLOCK(unit);
    return BCM_E_NONE;
}


#endif              /* defined(BCM_TRIDENT2_SUPPORT) */
