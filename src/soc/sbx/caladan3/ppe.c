/*
 * $Id: ppe.c,v 1.41 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    ppe.c
 * Purpose: Caladan3 Packet Parsing Engine drivers
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/ppe.h>
#include <soc/sbx/caladan3/wb_db_ppe.h>
#include <soc/sbx/caladan3/util.h>

#ifdef BCM_CALADAN3_SUPPORT

/*
 * PPE Driver
 */

/* TCAM */
#define SOC_SBX_CALADAN3_PPE_TCAM_LEN_BITS 234
#define SOC_SBX_CALADAN3_PPE_TCAM_LENGTH   29   /* Note 29 bytes and 2 bits actually */
#define SOC_SBX_CALADAN3_PPE_TCAM_LENGTH_WORDS  \
         ((SOC_SBX_CALADAN3_PPE_TCAM_LENGTH + 3) >> 2)

#define SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH   25
#define SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH_WORDS \
         ((SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH + 3) >> 2)

#define SOC_SBX_CALADAN3_PPE_TCAM_REP_DATA_SIZE 6
#define SOC_SBX_CALADAN3_PPE_TCAM_REP_DATA_OFFSET 0
#define SOC_SBX_CALADAN3_PPE_TCAM_REP_DATA_MASK \
            ((1 << SOC_SBX_CALADAN3_PPE_TCAM_REP_DATA_SIZE)-1)

#define SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_SIZE 2
#define SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_OFFSET 6
#define SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_MASK \
            ((1 << SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_SIZE)-1)

/* Property table */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_DEFINED_ROWS    (32 * 1024)
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG0_ENTRIES  32   /* per row */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG1_ENTRIES  16   /* per row */
#define SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_CONFIG2_ENTRIES   8   /* per row */



typedef struct {
    uint8 inuse;                 /* ref count */
    uint8 index;
    uint8 entry_width;
    uint32 base;
    uint32 max_entries;
    uint32 user_type;
    uint8  *data;
    void   *usercb;
} soc_sbx_caladan3_ppe_prop_segment_t;

typedef struct {
    uint32 num_rows;
    uint32 num_entries_per_row;
    uint8  config_type;
    uint8  segments_inuse;    /* bitmap */
    soc_sbx_caladan3_ppe_prop_segment_t segment[SOC_SBX_CALADAN3_PPE_PROPERTY_TABLE_SEGMENT_MAX];
} soc_sbx_caladan3_ppe_prop_mem_t;

typedef struct {
    uint32 num_entries;
    uint32 entry_width;
    uint32 table_base;
    uint32 index_start;
    uint32 index_width;
} soc_sbx_caladan3_ppe_prop_table_t;
 
typedef struct {
    uint32 key0[SOC_SBX_CALADAN3_PPE_TCAM_LENGTH_WORDS];
    uint32 key1[SOC_SBX_CALADAN3_PPE_TCAM_LENGTH_WORDS];
} soc_sbx_caladan3_ppe_enc_tcamdata_t;

struct soc_sbx_caladan3_ppe_config_s {

    sal_mutex_t lock;     /* Serialize access ** must be 1st element for warm boot signature *** */
    uint32 unit_active;   /* Set when PPE HW init is completed */

#if 0
    soc_sbx_caladan3_ppe_header_type_desc_t hdesc[SOC_SBX_CALADAN3_PPE_HTYPE_MAX];
#endif

    /* Hash template control block */
    soc_sbx_caladan3_ppe_hash_template_t hashtemp[SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX];

    /* Initial queue state */
    soc_sbx_caladan3_ppe_iqsm_t iqsm[SOC_SBX_CALADAN3_PPE_IQSM_INDEX_MAX];
    int iqsm_push;        /* commit field update immediately */

    /* Source Queue data */
    soc_sbx_caladan3_ppe_sqdm_t sqdm[SOC_SBX_CALADAN3_PPE_SQDM_INDEX_MAX];

    /* Cam Ram and Cam entry */
    struct {
        soc_sbx_caladan3_ppe_camram_t camram[SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX];
        soc_sbx_caladan3_ppe_tcamdata_t tcam[SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX];
    } cam_cfg[SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX];

    /* User config */
    soc_sbx_caladan3_ppe_userdata_t assembler_config;

    /* Capture manager *|
    soc_sbx_caladan3_ppe_header_capture_t  cap_mgr;
 
    |* Test counter info *|
    soc_sbx_caladan3_ppe_debug_counter_t   counter;
    */

};

static soc_sbx_caladan3_ppe_config_t ppe_config[SOC_MAX_NUM_DEVICES];

#define C3_PPE(unit) (&ppe_config[(unit)])

#define CALADAN3_PPE_READY(unit) ((C3_PPE(unit) && C3_PPE(unit)->unit_active) ? 1 : 0)
#define CALADAN3_PPE_LOCK(unit) sal_mutex_take((C3_PPE(unit))->lock, sal_mutex_FOREVER)
#define CALADAN3_PPE_UNLOCK(unit) sal_mutex_give((C3_PPE(unit))->lock)

/* 
 * PPE cam capture
 */
static const soc_reg_t soc_sbx_caladan3_ppe_cam_cap_regs[] = {
        PP_CAM_CAPTURE_P0_STATUSr,
        PP_CAM_CAPTURE_P1_STATUSr,
        PP_CAM_CAPTURE_P2_STATUSr,
        PP_CAM_CAPTURE_P3_STATUSr,
        PP_CAM_CAPTURE_P4_STATUSr,
        PP_CAM_CAPTURE_P5_STATUSr,
        PP_CAM_CAPTURE_P6_STATUSr
};
    
int ppe_config_size_get(int unit)
{
    return sizeof(soc_sbx_caladan3_ppe_config_t);
}

int ppe_signature_size_get(int unit)
{
    /* returns the size of the ppe_config structure the selected unit
     * not counting the first element (lock)
     */
    return sizeof(soc_sbx_caladan3_ppe_config_t) - sizeof(ppe_config[unit].lock);
}

soc_sbx_caladan3_ppe_config_t *ppe_config_get(int unit)
{
    return &ppe_config[unit];
}

unsigned char *ppe_signature_get(int unit)
{
    /* returns the size of the ppe_config structure the selected unit
     * not counting the first element (lock)
     */

    unsigned char *ret = (unsigned char *)&ppe_config[unit] + sizeof(ppe_config[unit].lock);
    return ret;
}


#if 0


/*
 *  Routine: soc_sbx_caladan3_pkt_header_type_alloc
 *  Description:
 *     Allocates and saves header type info.
 *     Specific purpose is to have a database for validations.
 *  Inputs:
 *     type - ucode header type
 *     hlen - length of the header
 *     flag - behaviour specifier for the type
 *     desc - user defined discription of the type
 *  Outputs: one of SOC_E* status types
 */
int soc_sbx_caladan3_pkt_header_type_alloc(int unit, uint8 type, uint8 len, soc_sbx_caladan3_htype_specifier_t flag, char* desc) {

    soc_sbx_caladan3_ppe_config_t *ppe;

    if (!CALADAN3_PPE_READY(unit)) return SOC_E_INIT;
    
    ppe = C3_PPE(unit);
    if (type >= SOC_SBX_CALADAN3_PPE_HTYPE_MAX) {
        return SOC_E_PARAM;
    }
    if (ppe->hdesc[type].inuse) {
        return SOC_E_EXISTS;
    }
    CALADAN3_PPE_LOCK(unit);
    ppe->hdesc[type].htype.header_type = type;
    ppe->hdesc[type].htype.header_len = len;
    ppe->hdesc[type].htype.flag = flag;
    sal_strncpy(ppe->hdesc[type].name, desc, sizeof(ppe->hdesc[type].name));
    ppe->hdesc[type].inuse++;
    CALADAN3_PPE_UNLOCK(unit);

    return SOC_E_NONE;
     
}

/*
 *  Routine: soc_sbx_caladan3_pkt_header_type_free
 *  Description:
 *     Undefine a header type
 *  Inputs:
 *     type - ucode header type
 *     hlen - length of the header
 *  Outputs: one of SOC_E* status types
 */
int soc_sbx_caladan3_pkt_header_type_free(int unit, uint8 type, soc_sbx_caladan3_htype_specifier_t flag) {

    soc_sbx_caladan3_ppe_config_t *ppe;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (type >= SOC_SBX_CALADAN3_PPE_HTYPE_MAX) {
        return SOC_E_PARAM;
    }
   
    ppe = C3_PPE(unit);
    if (!ppe->hdesc[type].inuse) {
        return SOC_E_NOT_FOUND;
    }
    
    CALADAN3_PPE_LOCK(unit);
    ppe->hdesc[type].inuse--;

    if (!ppe->hdesc[type].inuse) {
        sal_memset(&ppe->hdesc[type], 0, sizeof(soc_sbx_caladan3_ppe_header_type_desc_t));
    }
    CALADAN3_PPE_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_pkt_header_checker_set
 *  Description:
 *     Associate a checker to a type
 *  Inputs:
 *     type - ucode header type
 *     checkerid - checker id
 *  Outputs: one of SOC_E* status types
 */
int soc_sbx_caladan3_pkt_header_checker_set(int unit, uint8 type, soc_sbx_caladan3_ppe_checker_t checkerid) {

    soc_sbx_caladan3_ppe_config_t *ppe;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (type >= SOC_SBX_CALADAN3_PPE_HTYPE_MAX) {
        return SOC_E_PARAM;
    }
   
    ppe = C3_PPE(unit);
    if (!ppe->hdesc[type].inuse) {
        return SOC_E_NOT_FOUND;
    }
    if (checkerid >= SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_MAX) {
        return SOC_E_PARAM;
    }
    CALADAN3_PPE_LOCK(unit);
    ppe->hdesc[type].checker = checkerid;
    CALADAN3_PPE_UNLOCK(unit);
    return SOC_E_NONE;   
}

/*
 *  Routine: soc_sbx_caladan3_pkt_header_checker_clear
 *  Description:
 *     Disaccociate a checker from a type
 *  Inputs:
 *     type - ucode header type
 *  Outputs: one of SOC_E* status types
 */
int soc_sbx_caladan3_pkt_header_checker_clear(int unit, uint8 type) {

    soc_sbx_caladan3_ppe_config_t *ppe;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (type >= SOC_SBX_CALADAN3_PPE_HTYPE_MAX) {
        return SOC_E_PARAM;
    }
   
    ppe = C3_PPE(unit);
    if (!ppe->hdesc[type].inuse) {
        return SOC_E_NOT_FOUND;
    }

    CALADAN3_PPE_LOCK(unit);
    ppe->hdesc[type].checker = SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_NONE;
    CALADAN3_PPE_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_pkt_header_hasher_set
 *  Description:
 *     accociate a hash to a type
 *  Inputs:
 *     type - ucode header type
 *     hid - hash template id
 *  Outputs: one of SOC_E* status types
 */
int soc_sbx_caladan3_pkt_header_hasher_set(int unit, uint8 type, soc_sbx_caladan3_ppe_hash_id_t hid) {

    soc_sbx_caladan3_ppe_config_t *ppe;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (type >= SOC_SBX_CALADAN3_PPE_HTYPE_MAX) {
        return SOC_E_PARAM;
    }
   
    ppe = C3_PPE(unit);
    if (!ppe->hdesc[type].inuse) {
        return SOC_E_NOT_FOUND;
    }
    CALADAN3_PPE_LOCK(unit);
    ppe->hdesc[type].hasher = hid;
    CALADAN3_PPE_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_pkt_header_checker_set
 *  Description:
 *     Disaccociate a hash from a type
 *  Inputs:
 *     type - ucode header type
 *  Outputs: one of SOC_E* status types
 */
int soc_sbx_caladan3_pkt_header_hasher_clear(int unit, uint8 type) {

    soc_sbx_caladan3_ppe_config_t *ppe;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (type >= SOC_SBX_CALADAN3_PPE_HTYPE_MAX) {
        return SOC_E_PARAM;
    }
   
    ppe = C3_PPE(unit);
    if (ppe->hdesc[type].inuse) {
        return SOC_E_NOT_FOUND;
    }
    CALADAN3_PPE_LOCK(unit);
    ppe->hdesc[type].hasher = 0;
    CALADAN3_PPE_UNLOCK(unit);
    return SOC_E_NONE;
}

#endif

/*
 * Hash management
 *    Hash template id 0-3 are bit mapped
 *    Hash template id 4-31 are byte mapped
 *    The routines allow user to allocate any specific type and 
 *    program the hash masks as required
 */

/*
 *  Routine: soc_sbx_caladan3_ppe_hash_template_alloc
 *  Description:
 *     Allocate a free hash template id based on type
 *  Inputs:
 *     type - hash type bit/byte
 *  Outputs: 
 *      one of SOC_E* status types
 *      id - hash template id allocated when return is SOC_E_NONE
 */
int soc_sbx_caladan3_ppe_hash_template_alloc(int unit, soc_sbx_caladan3_ppe_hash_type_t type, soc_sbx_caladan3_ppe_hash_id_t *id) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int i, start, end;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
   
    if (type >= SOC_SBX_CALADAN3_PPE_HASH_TYPE_MAX) {
        return SOC_E_PARAM;
    }

    ppe = C3_PPE(unit);

    CALADAN3_PPE_LOCK(unit);
    if (*id >= SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX) {
        if (type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT) {
            start = SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_START;
            end = start + SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_ENTRIES;
        } else {
            start = SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START;
            end = start + SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_ENTRIES;
        }

        for (i = start; i < end; i++) {
            if (!ppe->hashtemp[i].inuse) {
                *id = i;
                break;
            }
        }   
    } else {
#ifdef BCM_WARM_BOOT_SUPPORT
        if (!SOC_WARM_BOOT(unit)) {
#endif
            if (ppe->hashtemp[*id].inuse) {
                CALADAN3_PPE_UNLOCK(unit);
                return SOC_E_EXISTS;
            }
#ifdef BCM_WARM_BOOT_SUPPORT
        }
#endif
        i = *id;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
#endif
        ppe->hashtemp[i].inuse++;
        ppe->hashtemp[i].id = i;
        ppe->hashtemp[i].type = type;
#ifdef BCM_WARM_BOOT_SUPPORT
    }
#endif

    CALADAN3_PPE_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_hash_template_alloc_by_pattern
 *  Description:
 *     Allocate a hash template id based on type and pattern
 *  Inputs:
 *     type - hash type bit/byte
 *     hash_mask_array - pattern 
 *  Outputs: 
 *      one of SOC_E* status types
 *      id - hash template id allocated when return is SOC_E_NONE
 */
int soc_sbx_caladan3_ppe_hash_template_alloc_by_pattern(int unit, soc_sbx_caladan3_ppe_hash_type_t type, uint32 *hash_mask_array, soc_sbx_caladan3_ppe_hash_id_t *id) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_hash_id_t tmp_id = 0;
    int i, start, end;
    int found = 0;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (hash_mask_array == NULL) {
        return SOC_E_PARAM;
    }
   
    if (type >= SOC_SBX_CALADAN3_PPE_HASH_TYPE_MAX) {
        return SOC_E_PARAM;
    }

    ppe = C3_PPE(unit);

    CALADAN3_PPE_LOCK(unit);
    if (type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT) {
        start = SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_START;
        end = start + SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_ENTRIES;
    } else {
        start = SOC_SBX_CALADAN3_PPE_HASH_BIT_TEMPLATE_START;
        end = start + SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_ENTRIES;
    }

    for (i = start; i < end; i++) {
        if (ppe->hashtemp[i].inuse) {
            if ((type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT) &&
                (sal_memcmp(hash_mask_array, &ppe->hashtemp[i].hash.bitmem, sizeof(ppe->hashtemp[i].hash.bitmem)) == 0)) {
                *id = i;
                found = 1;
                break;
            } else if ((type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BYTE) &&
                (sal_memcmp(hash_mask_array, &ppe->hashtemp[i].hash.bytemem, sizeof(ppe->hashtemp[i].hash.bytemem)) == 0)) {
                *id = i;
                found = 1;
                break;
            }
        } else {
            tmp_id = i;
        }
    }
    if (!found) {
        if (tmp_id) {
            *id = tmp_id;
            result = SOC_E_NOT_FOUND;
        } else {
            result = SOC_E_RESOURCE;
        }
    }

    CALADAN3_PPE_UNLOCK(unit);
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_hash_template_free
 *  Description:
 *     Free a hash template id 
 *  Inputs:
 *      id - hash template id allocated earlier
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_hash_template_free(int unit, soc_sbx_caladan3_ppe_hash_id_t id) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_hash_bit_template_entry_t bitm;
    pp_hash_byte_template_entry_t bytem;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    ppe = C3_PPE(unit);

    if (id < SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX) {
        if (ppe->hashtemp[id].inuse) {
            CALADAN3_PPE_LOCK(unit);
            ppe->hashtemp[id].inuse--;
            if (ppe->hashtemp[id].inuse == 0) {
                if (ppe->hashtemp[id].type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT) {
                    sal_memset(&bitm, 0, sizeof(pp_hash_bit_template_entry_t));
                    soc_mem_lock(unit, PP_HASH_BIT_TEMPLATEm);
                    WRITE_PP_HASH_BIT_TEMPLATEm(unit, MEM_BLOCK_ALL, id, &bitm);
                    soc_mem_unlock(unit, PP_HASH_BIT_TEMPLATEm);
                } else {
                    sal_memset(&bytem, 0, sizeof(pp_hash_byte_template_entry_t));
                    soc_mem_lock(unit, PP_HASH_BYTE_TEMPLATEm);
                    WRITE_PP_HASH_BYTE_TEMPLATEm(unit, MEM_BLOCK_ALL,
                                                 id-SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START, &bytem);
                    soc_mem_unlock(unit, PP_HASH_BYTE_TEMPLATEm);
                }
            }
            CALADAN3_PPE_UNLOCK(unit);
        }
    } else {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_hash_template_set
 *  Description:
 *     Program a hash template
 *  Inputs:
 *      id - hash template id allocated earlier
 *      hash_mask_array - template
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_hash_template_set(int unit, soc_sbx_caladan3_ppe_hash_id_t id, uint32 *hash_mask_array) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_hash_bit_template_entry_t bitmem;
    pp_hash_byte_template_entry_t bytemem;
    int result = SOC_E_NONE;
    soc_sbx_caladan3_ppe_hash_type_t type;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (hash_mask_array == NULL) {
        return SOC_E_PARAM;
    }
      
#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        return SOC_E_NONE;
    }
#endif

    ppe = C3_PPE(unit);
    sal_memset(&bitmem, 0, sizeof(pp_hash_bit_template_entry_t));
    sal_memset(&bytemem, 0, sizeof(pp_hash_byte_template_entry_t));

    type = ppe->hashtemp[id].type;
    if ((type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT) && 
         (id < SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START)) {
        sal_memcpy(&(ppe->hashtemp[id].hash.bitmem), hash_mask_array, sizeof(soc_sbx_caladan3_ppe_hash_bitmem_t));
        soc_mem_lock(unit, PP_HASH_BIT_TEMPLATEm);
        result = WRITE_PP_HASH_BIT_TEMPLATEm(unit, MEM_BLOCK_ALL, id, hash_mask_array);
        soc_mem_unlock(unit, PP_HASH_BIT_TEMPLATEm);
    } else if ((type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BYTE) &&
              (id < SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX) &&
              (id >= SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START)) {
        sal_memcpy(&(ppe->hashtemp[id].hash.bytemem), hash_mask_array, sizeof(soc_sbx_caladan3_ppe_hash_bytemem_t));
        soc_mem_lock(unit, PP_HASH_BYTE_TEMPLATEm);
        result = WRITE_PP_HASH_BYTE_TEMPLATEm(unit, MEM_BLOCK_ALL, 
                                              id-SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START, hash_mask_array);
        soc_mem_unlock(unit, PP_HASH_BYTE_TEMPLATEm);
    } else {
        return SOC_E_PARAM;
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_hash_template_get
 *  Description:
 *     fetch a hash template
 *  Inputs:
 *      id - hash template id allocated earlier
 *      hash_mask_array - template
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_hash_template_get(int unit, soc_sbx_caladan3_ppe_hash_id_t id, uint32 *hash_mask_array) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_hash_bit_template_entry_t bitmem;
    pp_hash_byte_template_entry_t bytemem;
    int result = SOC_E_NONE;
    int type;

    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (hash_mask_array == NULL) {
        return SOC_E_PARAM;
    }
   
    ppe = C3_PPE(unit);
    sal_memset(&bitmem, 0, sizeof(pp_hash_bit_template_entry_t));
    sal_memset(&bytemem, 0, sizeof(pp_hash_byte_template_entry_t));

    type = ppe->hashtemp[id].type;
    if ((type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BIT) && 
         (id < SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START)) {
        soc_mem_lock(unit, PP_HASH_BIT_TEMPLATEm);
        result = READ_PP_HASH_BIT_TEMPLATEm(unit, MEM_BLOCK_ANY, id, hash_mask_array);
        soc_mem_unlock(unit, PP_HASH_BIT_TEMPLATEm);
    } else if ((type == SOC_SBX_CALADAN3_PPE_HASH_TYPE_BYTE) &&
              (id < SOC_SBX_CALADAN3_PPE_HASH_TEMPLATE_MAX) &&
              (id >= SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START)) {
        soc_mem_lock(unit, PP_HASH_BYTE_TEMPLATEm);
        result = READ_PP_HASH_BYTE_TEMPLATEm(unit, MEM_BLOCK_ANY,
                                             id-SOC_SBX_CALADAN3_PPE_HASH_BYTE_TEMPLATE_START, hash_mask_array);
        soc_mem_unlock(unit, PP_HASH_BYTE_TEMPLATEm);
    } else {
        return SOC_E_PARAM;
    }
    
    return result;
}



/*
 * Initial Queue State
 *     There is one entry per queue, totally 128 entries
 *     Each field of any entry can be programmed directly by controlling iqsm_push
 *     If iqsm_push is not set, then the user will have to invoke the commit routine 
 *     to commit into iqsm memory
 */


/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_type_set
 *  Description:
 *     Set first header type
 *  Inputs:
 *      queueid - queue number
 *      type - ucode aware htype 
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_type_set(int unit, uint8 queueid, uint8 type) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].initial_type = type;
    if (ppe->iqsm_push) {
        result = soc_mem_field32_modify(unit, PP_IQSMm, queueid,
                                        INITIAL_TYPEf, type);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_type_get
 *  Description:
 *     Get the first header type for this queue
 *  Inputs:
 *      queueid - queue number
 *      type - ucode aware htype 
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_type_get(int unit, uint8 queueid, uint8 *type) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (type == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        *type = soc_PP_IQSMm_field32_get(unit, &iqsm, INITIAL_TYPEf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_offset_set
 *  Description:
 *     set the offset of first header type for this queue
 *  Inputs:
 *      queueid - queue number
 *      off - number of bytes to shift to meet the header
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_offset_set(int unit, uint8 queueid, uint8 off) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    if (off > SOC_SBX_CALADAN3_PPE_SHIFT_MAX) {
        return SOC_E_PARAM;
    }
    ppe->iqsm[queueid].initial_shift = off;
    if (ppe->iqsm_push) {
        result = soc_mem_field32_modify(unit, PP_IQSMm, queueid,
                                        INITIAL_SHIFTf, off);
    }
    
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_offset_get
 *  Description:
 *     Get the first header type for this queue
 *  Inputs:
 *      queueid - queue number
 *      off - number of bytes to shift to meet the header
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_offset_get(int unit, uint8 queueid, uint8 *off) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (off == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        *off = soc_PP_IQSMm_field32_get(unit, &iqsm, INITIAL_SHIFTf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_len_set
 *  Description:
 *     Set the length of first header type for this queue
 *  Inputs:
 *      queueid - queue number
 *      len - length of header
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_len_set(int unit, uint8 queueid, uint8 len) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (len > SOC_SBX_CALADAN3_PPE_SHIFT_MAX) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].shift = len;
    if (ppe->iqsm_push) {
        result = soc_mem_field32_modify(unit, PP_IQSMm, queueid, SHIFTf, len);
    }
    COMPILER_REFERENCE(result);
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_len_get
 *  Description:
 *     Get the first header type for this queue
 *  Inputs:
 *      queueid - queue number
 *      len - length of header
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_first_hdr_len_get(int unit, uint8 queueid, uint8 *len) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (len == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        *len = soc_PP_IQSMm_field32_get(unit, &iqsm, SHIFTf);
    }
    return result;
    
}
/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_variable_hdr_len_set
 *  Description:
 *     set the variable header parsing params for first stage
 *  Inputs:
 *      queueid - queue number (0-127)
 *      var_hdr - variable len parsing params
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_variable_hdr_len_set(int unit, uint8 queueid, 
                                                 soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    ppe = C3_PPE(unit);
    sal_memcpy(&ppe->iqsm[queueid].var_len_hdr, var_hdr, sizeof(soc_sbx_caladan3_ppe_var_len_hdr_t));

    if (ppe->iqsm_push) {
        sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
        soc_mem_lock(unit, PP_IQSMm);
        result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
        if (result == SOC_E_NONE) {
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                                     LENGTH_UNITSf, var_hdr->units);
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                                     LENGTH_MASKf, var_hdr->mask);
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                                     LENGTH_SHIFT_PTRf, var_hdr->shift_bytes);
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                                     LENGTH_SHIFT_BITSf, var_hdr->shift_bits);
            result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
        }
        soc_mem_unlock(unit, PP_IQSMm);
    }
    return result;
    
}
/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_variable_hdr_len_get
 *  Description:
 *     get the variable header parsing params for first stage
 *  Inputs:
 *      queueid - queue number (0-127)
 *      var_hdr - variable len parsing params
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_variable_hdr_len_get(int unit, uint8 queueid, 
                                                 soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (var_hdr == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        var_hdr->units =
            soc_PP_IQSMm_field32_get(unit, &iqsm, LENGTH_UNITSf);
        var_hdr->mask =
            soc_PP_IQSMm_field32_get(unit, &iqsm, LENGTH_MASKf);
        var_hdr->shift_bytes =
            soc_PP_IQSMm_field32_get(unit, &iqsm, LENGTH_SHIFT_PTRf);
        var_hdr->shift_bits =
            soc_PP_IQSMm_field32_get(unit, &iqsm, LENGTH_SHIFT_BITSf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_initial_state_set
 *  Description:
 *     set the state for packets entrying this queue
 *  Inputs:
 *      queueid - queue number
 *      state - default value of state
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_initial_state_set(int unit, uint8 queueid, 
                                              soc_sbx_caladan3_ppe_state_t state) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;
    uint32 state_data;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    sal_memcpy(&ppe->iqsm[queueid].istate, state, sizeof(soc_sbx_caladan3_ppe_state_t));
    if (ppe->iqsm_push) {
        sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
        state_data = ((state[0] << 16) | (state[1] << 8) | state[2]);
        soc_mem_lock(unit, PP_IQSMm);
        result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
        if (result == SOC_E_NONE) {
            soc_PP_IQSMm_field32_set(unit, &iqsm, STATEf, state_data);
            result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
        }
        soc_mem_unlock(unit, PP_IQSMm);
    }
    return result;
    
}
/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_initial_state_get
 *  Description:
 *     Get the default state for packets entrying this queue
 *  Inputs:
 *      queueid - queue number (0-127)
 *      state - default value of state
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_initial_state_get(int unit, uint8 queueid, 
                                              soc_sbx_caladan3_ppe_state_t state) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;
    uint32 state_data;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (state == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        state_data = soc_PP_IQSMm_field32_get(unit, &iqsm, STATEf);
        state[0] = (state_data >> 16) & 0xFF;
        state[1] = (state_data >> 8) & 0xFF;
        state[2] = (state_data  & 0xFF);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_initial_variable_set
 *  Description:
 *     Set the default variable for the packets on this queue
 *  Inputs:
 *      queueid - queue number (0-127)
 *      variable - default variable
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_initial_variable_set(int unit, uint8 queueid, soc_sbx_caladan3_ppe_variable_t *var) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    sal_memcpy(&ppe->iqsm[queueid].initial_variable, var, sizeof(soc_sbx_caladan3_ppe_variable_t));
    if (ppe->iqsm_push) {
        sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
        soc_mem_lock(unit, PP_IQSMm);
        result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
        if (result == SOC_E_NONE) {
            soc_PP_IQSMm_field32_set(unit, &iqsm, INITIAL_VARIABLEf, *var);
            result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
        }
        soc_mem_unlock(unit, PP_IQSMm);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_initial_variable_get
 *  Description:
 *     Get the default variable for the packets on this queue
 *  Inputs:
 *      queueid - queue number (0-127)
 *      variable - default variable
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_initial_variable_get(int unit, uint8 queueid, soc_sbx_caladan3_ppe_variable_t *var) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (var == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        soc_PP_IQSMm_field_get(unit, &iqsm, INITIAL_VARIABLEf, var);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_initial_stream_set
 *  Description:
 *     Set the default stream for the packets on this queue
 *  Inputs:
 *      queueid - queue number (0-127)
 *      stream - stream on which pkts land
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_initial_stream_set(int unit, uint8 queueid, uint8 stream) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].initial_stream = stream;
    if (ppe->iqsm_push) {
        result = soc_mem_field32_modify(unit, PP_IQSMm, queueid, INITIAL_STREAMf, stream);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_initial_stream_get
 *  Description:
 *     Get the default stream for the packets on this queue
 *  Inputs:
 *      queueid - queue number (0-127)
 *      stream - stream on which pkts land
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_iqsm_entry_initial_stream_get(int unit, uint8 queueid, uint8 *stream) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (stream == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        *stream = soc_PP_IQSMm_field32_get(unit, &iqsm, INITIAL_STREAMf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_load_sq_control_set
 *  Description:
 *     Set the initial value of load_sq_data control word
 *     this can be overridden by camram entry keep* fields
 *     this controls if user prefers source queue data be
 *     made available to ucode.
 *  Inputs:
 *      queueid - queue id (0-127)
 *      load_sq_data - load sq data 
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_load_sq_control_set(int unit, uint8 queueid, uint32 load_sq_data) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].initial_load_sq_data = load_sq_data;
    if (ppe->iqsm_push) {
        result = soc_mem_field32_modify(unit, PP_IQSMm, queueid, 
                                        INITIAL_LOAD_SQ_DATAf, load_sq_data);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_load_sq_control_get
 *  Description:
 *     get the initial value of load_sq_data control word
 *     this can be overridden by camram entry keep* fields
 *     this controls if user prefers source queue data be
 *     made available to ucode.
 *  Inputs:
 *      queueid - queue id (0-127)
 *      load_sq_data - load sq data 
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_load_sq_control_get(int unit, uint8 queueid, uint32 *load_sq_data) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (load_sq_data == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        *load_sq_data = 
            soc_PP_IQSMm_field32_get(unit, &iqsm, INITIAL_LOAD_SQ_DATAf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_hash_template_disable_set
 *  Description:
 *     Set the masked out hash templates
 *  Inputs:
 *      queueid - queue id (0-127)
 *      disabled_mask - hash template mask
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_hash_template_disable_set(int unit, uint8 queueid, uint32 disable_mask) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].hash_template_disable = disable_mask;
    if (ppe->iqsm_push) {
        result = soc_mem_field32_modify(unit, PP_IQSMm, queueid, 
                                        HASH_TEMPLATE_DISABLEf, disable_mask);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_hash_template_disable_get
 *  Description:
 *     Get the masked out hash templates
 *  Inputs:
 *      queueid - queue id (0-127)
 *      disabled_mask - hash template mask
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_hash_template_disable_get(int unit, uint8 queueid, uint32 *disabled_mask) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (disabled_mask == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        *disabled_mask =
            soc_PP_IQSMm_field32_get(unit, &iqsm, HASH_TEMPLATE_DISABLEf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_checker_set
 *  Description:
 *     Set the header checker info associated with this entry
 *  Inputs:
 *      queueid - queue id (0-127)
 *      checker - header checker info
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_checker_set(int unit, uint8 queueid, soc_sbx_caladan3_ppe_hdr_checker_t *checker) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;
    int disable = 0;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].checker = *checker;
    if (ppe->iqsm_push) {
        sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
        soc_mem_lock(unit, PP_IQSMm);
        result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
        if (result == SOC_E_NONE) {
            soc_PP_IQSMm_field32_set(unit, &iqsm, CHECKERf, checker->id);
            soc_PP_IQSMm_field32_set(unit, &iqsm, CHECKER_OFFSETf, checker->offset);
            disable = (checker->enable ? 0 : 1);
            soc_PP_IQSMm_field32_set(unit, &iqsm, CHECKER_DISABLEf, disable);
            result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
        }
        soc_mem_unlock(unit, PP_IQSMm);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_checker_get
 *  Description:
 *     Get the header checker info associated with this entry
 *  Inputs:
 *      queueid - queue id (0-127)
 *      checker - header checker info
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_checker_get(int unit, uint8 queueid, soc_sbx_caladan3_ppe_hdr_checker_t *checker) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (checker == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        checker->id = soc_PP_IQSMm_field32_get(unit, &iqsm, CHECKERf);
        checker->offset =
              soc_PP_IQSMm_field32_get(unit, &iqsm, CHECKER_OFFSETf);
        checker->enable = 
              ((soc_PP_IQSMm_field32_get(unit, &iqsm, CHECKER_DISABLEf)) ? 0 : 1);

    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_hasher_set
 *  Description:
 *     Set the hash template parameters associated with this entry
 *  Inputs:
 *      queueid - queue id (0-127)
 *      hasher - hash tempate info
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_hasher_set(int unit, uint8 queueid, soc_sbx_caladan3_ppe_hdr_hash_t *hasher) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].hasher = *hasher;
    if (ppe->iqsm_push) {
        sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
        soc_mem_lock(unit, PP_IQSMm);
        result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
        if (result == SOC_E_NONE) {
            soc_PP_IQSMm_field32_set(unit, &iqsm,
               HASH_TEMPLATEf, hasher->hash_id);
            soc_PP_IQSMm_field32_set(unit, &iqsm, 
                 HASH_ENABLEf, hasher->enable);
            result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
        }
        soc_mem_unlock(unit, PP_IQSMm);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_hasher_get
 *  Description:
 *     Get the hash template parameters associated with this entry
 *  Inputs:
 *      queueid - queue id (0-127)
 *      hasher - hash tempate info
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_hasher_get(int unit, uint8 queueid, soc_sbx_caladan3_ppe_hdr_hash_t *hasher) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (hasher == NULL) {
        return SOC_E_PARAM;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        hasher->hash_id =
            soc_PP_IQSMm_field32_get(unit, &iqsm, HASH_TEMPLATEf);
        hasher->enable =
            soc_PP_IQSMm_field32_get(unit, &iqsm, HASH_ENABLEf);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_property_set
 *  Description:
 *     Set the property table parameters associated with this entry
 *  Inputs:
 *      queueid - queue id (0-127)
 *      prop - property table entry
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_property_set(int unit, uint8 queueid, soc_sbx_caladan3_ppe_prop_entry_t *prop) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->iqsm[queueid].prop = *prop;
    if (ppe->iqsm_push) {
        sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
        soc_mem_lock(unit, PP_IQSMm);
        result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
        if (result == SOC_E_NONE) {
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                    PROPERTY_ENABLEf, prop->property_enable);
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                    PROPERTY_SEGMENTf, prop->property_segment);
            soc_PP_IQSMm_field32_set(unit, &iqsm,
                    PROPERTY_INDEX_STARTf, prop->property_index_start);
            soc_PP_IQSMm_field32_set(unit, &iqsm, 
                    PROPERTY_INDEX_SIZEf, prop->property_index_size);

            result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
        }
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_property_get
 *  Description:
 *     Get the property table parameters associated with this entry
 *  Inputs:
 *      queueid - queue id (0-127)
 *      prop - property table entry
 *  Outputs:
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_property_get(int unit, uint8 queueid, soc_sbx_caladan3_ppe_prop_entry_t *prop) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));

    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ANY, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);
 
    if (result == SOC_E_NONE) {
        prop->property_enable =
            soc_PP_IQSMm_field32_get(unit, &iqsm, PROPERTY_ENABLEf);
        prop->property_segment =
            soc_PP_IQSMm_field32_get(unit, &iqsm, PROPERTY_SEGMENTf);
        prop->property_index_start =
            soc_PP_IQSMm_field32_get(unit, &iqsm, PROPERTY_INDEX_STARTf);
        prop->property_index_size =
            soc_PP_IQSMm_field32_get(unit, &iqsm, PROPERTY_INDEX_SIZEf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_write
 *  Description:
 *     Set the initial source queue data for given queue
 *  Inputs:
 *     queueid - source queue id (0-127)
 *     iqsm - initial source queue data 
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_write(int unit, uint8 queueid, soc_sbx_caladan3_ppe_iqsm_t *iqsm ) {

    soc_sbx_caladan3_ppe_config_t *ppe __attribute__((unused));
    pp_iqsm_entry_t iqsm_ent;
    int result = SOC_E_NONE;
    int disable = 0;
    int state = 0;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);

#ifdef BCM_WARM_BOOT_SUPPORT
    if (!SOC_WARM_BOOT(unit)) {
        *(&ppe->iqsm[queueid]) = *iqsm;
    }
#endif
    sal_memset(&iqsm_ent, 0, sizeof(pp_iqsm_entry_t));
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, INITIAL_TYPEf, iqsm->initial_type);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, INITIAL_SHIFTf, iqsm->initial_shift);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, SHIFTf, iqsm->shift);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, 
             LENGTH_UNITSf, iqsm->var_len_hdr.units);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             LENGTH_MASKf, iqsm->var_len_hdr.mask);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             LENGTH_SHIFT_PTRf, iqsm->var_len_hdr.shift_bytes);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             LENGTH_SHIFT_BITSf, iqsm->var_len_hdr.shift_bits);
    state = ((iqsm->istate[0] << 16) | (iqsm->istate[1] << 8) | iqsm->istate[2]);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, STATEf, state);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             INITIAL_VARIABLEf, iqsm->initial_variable);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, INITIAL_STREAMf, iqsm->initial_stream);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             INITIAL_LOAD_SQ_DATAf, iqsm->initial_load_sq_data);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             HASH_TEMPLATE_DISABLEf, iqsm->hash_template_disable);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, CHECKERf, iqsm->checker.id);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, 
             CHECKER_OFFSETf, iqsm->checker.offset);
    disable = (iqsm->checker.enable ? 0 : 1);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, CHECKER_DISABLEf, disable);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, HASH_TEMPLATEf, iqsm->hasher.hash_id);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, HASH_ENABLEf, iqsm->hasher.enable);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             PROPERTY_ENABLEf, iqsm->prop.property_enable);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             PROPERTY_SEGMENTf, iqsm->prop.property_segment);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent,
             PROPERTY_INDEX_STARTf, iqsm->prop.property_index_start);
    soc_PP_IQSMm_field32_set(unit, &iqsm_ent, 
             PROPERTY_INDEX_SIZEf, iqsm->prop.property_index_size);

    soc_mem_lock(unit, PP_IQSMm);
    result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm_ent);
    soc_mem_unlock(unit, PP_IQSMm);

    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_read
 *  Description:
 *     Get the initial source queue data from memory for given queue
 *  Inputs:
 *     queueid - source queue id (0-127)
 *     iqsm - initial source queue data 
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_read(int unit, uint8 queueid, 
                                 soc_sbx_caladan3_ppe_iqsm_t *iqsm ) {

    pp_iqsm_entry_t iqsm_ent;
    int result = SOC_E_NONE;
    int state;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    sal_memset(&iqsm_ent, 0, sizeof(pp_iqsm_entry_t));
    soc_mem_lock(unit, PP_IQSMm);
    result = READ_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm_ent);
    soc_mem_unlock(unit, PP_IQSMm);
    if (result == SOC_E_NONE) {
        iqsm->initial_type = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, INITIAL_TYPEf);
        iqsm->initial_shift = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, INITIAL_SHIFTf);
        iqsm->shift = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, SHIFTf);
        iqsm->var_len_hdr.units = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, LENGTH_UNITSf);
        iqsm->var_len_hdr.mask = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, LENGTH_MASKf);
        iqsm->var_len_hdr.shift_bytes = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, LENGTH_SHIFT_PTRf);
        iqsm->var_len_hdr.shift_bits = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, LENGTH_SHIFT_BITSf);
        state = soc_PP_IQSMm_field32_get(unit, &iqsm_ent, STATEf);
        iqsm->istate[0] = (state >> 16) & 0xFF;
        iqsm->istate[1] = (state >> 8) & 0xFF;
        iqsm->istate[2] = (state & 0xFF);
        iqsm->initial_variable = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, INITIAL_VARIABLEf);
        iqsm->initial_stream = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, INITIAL_STREAMf);
        iqsm->initial_load_sq_data = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, INITIAL_LOAD_SQ_DATAf);
        iqsm->hash_template_disable =
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, HASH_TEMPLATE_DISABLEf);
        iqsm->checker.id =
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, CHECKERf);
        iqsm->checker.offset =
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, CHECKER_OFFSETf);
        iqsm->checker.enable = 
            (soc_PP_IQSMm_field32_get(unit, &iqsm_ent, CHECKER_DISABLEf)) ? 0 : 1; 
        iqsm->hasher.hash_id = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, HASH_TEMPLATEf);
        iqsm->hasher.enable = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, HASH_ENABLEf);
        iqsm->prop.property_enable =
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, PROPERTY_ENABLEf);
        iqsm->prop.property_segment =
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, PROPERTY_SEGMENTf);
        iqsm->prop.property_index_start = 
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, PROPERTY_INDEX_STARTf);
        iqsm->prop.property_index_size =
            soc_PP_IQSMm_field32_get(unit, &iqsm_ent, PROPERTY_INDEX_SIZEf);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_clear
 *  Description:
 *     Clear the Initial Source queue data associated with a queue in mwmory
 *  Inputs:
 *     queueid - source queue id (0-127)
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_clear(int unit, uint8 queueid) {

    pp_iqsm_entry_t iqsm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    sal_memset(&iqsm, 0, sizeof(pp_iqsm_entry_t));
    soc_mem_lock(unit, PP_IQSMm);
    result = WRITE_PP_IQSMm(unit, MEM_BLOCK_ALL, queueid, &iqsm);
    soc_mem_unlock(unit, PP_IQSMm);

    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_iqsm_entry_commit
 *  Description:
 *     Commit the Initial source queue data for the given queue
 *     This is used to write the entire IQSM data in one shot.
 *     Entries are written immediately if ppe->push_iqsm control 
 *     is set and this routine is redundant in this case.
 *  Inputs:
 *     queueid - source queue id (0-127)
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_iqsm_entry_commit(int unit, uint8 queueid) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    result = soc_sbx_caladan3_ppe_iqsm_entry_write(unit, queueid, &ppe->iqsm[queueid]);
    return result;
}   

/***
 *** Source Queue Data
 ***/

/*
 *  Routine: soc_sbx_caladan3_ppe_sqdm_entry_clear
 *  Description:
 *     Clear the source queue data associated with a queue in mwmory
 *  Inputs:
 *     queueid - source queue id (0-127)
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_sqdm_entry_clear(int unit, uint8 queueid) {

    pp_sqdm_entry_t sqdm;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    sal_memset(&sqdm, 0, sizeof(pp_sqdm_entry_t));
    soc_mem_lock(unit, PP_SQDMm);
    result = WRITE_PP_SQDMm(unit, MEM_BLOCK_ALL, queueid, &sqdm);
    soc_mem_unlock(unit, PP_SQDMm);
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_sqdm_entry_write
 *  Description:
 *     Set the source queue data associated with a queue
 *  Inputs:
 *     queueid - source queue id (0-127)
 *     sqdm - source queue data
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_sqdm_entry_write(int unit, uint8 queueid,
                                   soc_sbx_caladan3_ppe_sqdm_t *sqdm) {

    pp_sqdm_entry_t sqdmdata;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    soc_PP_SQDMm_field_set(unit, &sqdmdata, SQUEUE_DATAf, (uint32*)sqdm);
    soc_mem_lock(unit, PP_SQDMm);
    result = WRITE_PP_SQDMm(unit, MEM_BLOCK_ALL, queueid, &sqdmdata);
    soc_mem_unlock(unit, PP_SQDMm);
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_sqdm_entry_read
 *  Description:
 *     Get the source queue data associated with a queue
 *  Inputs:
 *     queueid - source queue id (0-127)
 *     sqdm - source queue data
 *  Outputs: 
 *     one of SOC_E*
 */
int soc_sbx_caladan3_ppe_sqdm_entry_read(int unit, uint8 queueid,
                                   soc_sbx_caladan3_ppe_sqdm_t *sqdm) {

    pp_sqdm_entry_t sqdmdata;

    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    sal_memset(sqdm, 0, sizeof(soc_sbx_caladan3_ppe_sqdm_t));
    soc_mem_lock(unit, PP_SQDMm);
    result = READ_PP_SQDMm(unit, MEM_BLOCK_ANY, queueid, &sqdmdata);
    soc_mem_unlock(unit, PP_SQDMm);
    if (result == SOC_E_NONE) {
        soc_PP_SQDMm_field_get(unit, &sqdmdata, SQUEUE_DATAf, (uint32*)sqdm);
    }
    return result;
    
}

/***
 ***  PPE Property Table
 ***/

/*
 *   Function
 *     sbx_caladan3_ppe_property_table_iaccess
 *   Purpose
 *      access ppe property table interface
 *   Parameters
 *      (IN) unit      : unit number of the device
 *      (IN) operation : operation type (read=1/write=0)
 *      (IN) address   : address - register address
 *      (IN/OUT)data   : 2 words data
 *   Returns
 *       SOC_E_NONE    - success
 *       SOC_E_TIMEOUT - command timed out
 */
int soc_sbx_caladan3_ppe_property_table_iaccess(int unit,
                                        int operation,
                                        uint32 address,
                                        uint32 *data)
{
    int    result = SOC_E_NONE;
    int    nDone;
    uint32 uRegValue = 0;

    sal_usecs_t    sTimeout = 10 * MILLISECOND_USEC;
    soc_timeout_t  sTo;

    /* sanity checks */
    if (data == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s ppe property table - data = NULL on unit %d\n"),
                   FUNCTION_NAME(), unit));
        return SOC_E_PARAM;
    }

    CALADAN3_PPE_LOCK(unit);

    /* initiate the operation */
    uRegValue = 0;
    soc_reg_field_set(unit, PP_PROPERTY_TABLE_IACCESSr, &uRegValue, REQf, 1);
    soc_reg_field_set(unit, PP_PROPERTY_TABLE_IACCESSr, &uRegValue, ADDRf, address);
    switch (operation) {
        case 0:
            WRITE_PP_PROPERTY_TABLE_IACCESS_DATA0r(unit, data[0]);
            WRITE_PP_PROPERTY_TABLE_IACCESS_DATA1r(unit, data[1]);
            WRITE_PP_PROPERTY_TABLE_IACCESS_DATA2r(unit, 0); /* ecc */

            soc_reg_field_set(unit, PP_PROPERTY_TABLE_IACCESSr, &uRegValue, RD_WR_Nf, 0);
            break;
        case 1:
            WRITE_PP_PROPERTY_TABLE_IACCESS_DATA0r(unit, 0);
            WRITE_PP_PROPERTY_TABLE_IACCESS_DATA1r(unit, 0);
            soc_reg_field_set(unit, PP_PROPERTY_TABLE_IACCESSr, &uRegValue, RD_WR_Nf, 1);
            break;
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "%s ppe unsupported operation %d on unit %d\n"),
                       FUNCTION_NAME(), operation, unit));
            CALADAN3_PPE_UNLOCK(unit);
            return SOC_E_PARAM;
    }
    result = WRITE_PP_PROPERTY_TABLE_IACCESSr(unit, uRegValue);
    if (SOC_FAILURE(result)) {
        CALADAN3_PPE_UNLOCK(unit);
        return result;
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "%s ppe property table access 0x%x, data 0x%8x 0x%8x on ppe unit %d.\n"),
                 FUNCTION_NAME(), uRegValue, data[0], data[1], unit));

    /* poll for done */
    if (SAL_BOOT_QUICKTURN) {
        /* quickturn is much more slower */
        sTimeout *= 100;
    }

    nDone = FALSE;
    soc_timeout_init(&sTo, sTimeout, 0);
    while(!soc_timeout_check(&sTo)) {
        result = READ_PP_PROPERTY_TABLE_IACCESSr(unit, &uRegValue);
        if (SOC_FAILURE(result)) {
            break;
        }
        nDone = soc_reg_field_get(unit, PP_PROPERTY_TABLE_IACCESSr, uRegValue, ACKf);
        if (nDone) {
            WRITE_PP_PROPERTY_TABLE_IACCESSr(unit, uRegValue);
            break;
        }
    }

    if (nDone || SAL_BOOT_PLISIM) {
        /* return data */
        if (data != NULL) {
            READ_PP_PROPERTY_TABLE_IACCESS_DATA0r(unit, &data[0]);
            READ_PP_PROPERTY_TABLE_IACCESS_DATA1r(unit, &data[1]);
        }
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: ppe sbus injected command timed out on unit %d\n"),
                   FUNCTION_NAME(), unit));
        result = SOC_E_TIMEOUT;
    }

    CALADAN3_PPE_UNLOCK(unit);
    return result;
}

/**
 * Property table init
 */
int soc_sbx_caladan3_ppe_property_table_init(int unit, int mode, int portA, int portB, int portC, int portD)
{
  uint32 config = 0;

  if (((mode & 0x3) != mode) || (mode == 0x3))
    return SOC_E_PARAM;
  if ((portA & 0x7) != portA)
    return SOC_E_PARAM;
  if ((portB & 0x7) != portB)
    return SOC_E_PARAM;
  if ((portC & 0x7) != portC)
    return SOC_E_PARAM;
  if ((portD & 0x7) != portD)
    return SOC_E_PARAM;

  /* Property Table Mem Config. 0=1Mx2b , 1=512Kx4b , 2=256Kx8b , 3=Invalid(do not use) */
  config |= (mode << 12);
  config |= (portA << 0);
  config |= (portB << 3);
  config |= (portC << 6);
  config |= (portD << 9);

  WRITE_PP_PROPERTY_TABLE_CONFIGr(unit, config);
  return SOC_E_NONE;
}

int soc_sbx_caladan3_ppe_property_table_segment_init(int unit, int segment, int seg_id, int start)
{
  if ((start & 0xFFFFF) != start)
    return SOC_E_PARAM;
  if ((segment & 0x7) != segment)
    return SOC_E_PARAM;

  switch (segment)
  {
   case 0:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY0_CONFIGr(unit, start);
    break;
   case 1:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY1_CONFIGr(unit, start);
    break;
   case 2:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY2_CONFIGr(unit, start);
    break;
   case 3:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY3_CONFIGr(unit, start);
    break;
   case 4:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY4_CONFIGr(unit, start);
    break;
   case 5:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY5_CONFIGr(unit, start);
    break;
   case 6:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY6_CONFIGr(unit, start);
    break;
   case 7:
    WRITE_PP_PROPERTY_SEGMENT_ENTRY7_CONFIGr(unit, start);
    break;
  }
  return SOC_E_NONE;
}


/***
 ***  PPE Cam Ram Entry
 ***/

/*
 *  Routine: soc_sbx_caladan3_ppe_camid_to_index
 *  Description:
 *     Convert camid,entry tuple to an index into memory
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index within the cam entries  (0-126)
 *  Outputs: 
 *      index - index into memory
 */
STATIC
int soc_sbx_caladan3_ppe_camid_to_index(uint8 camid, uint8 entry) {
    return (entry + (camid * SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX));
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_type_set
 *  Description:
 *     Set the header type of the header associated with the cam ram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      type - header type
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_type_set(int unit, uint8 camid, uint8 entry, uint8 type) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "\nCam id %d (max %d) Entry %d max(%d) exceeds limits"), camid, SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX, 
                   entry, SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX));
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);

    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->hdr_type = type;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_type_get
 *  Description:
 *     get the header type of the header associated with the cam ram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      type - header type
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_type_get(int unit, uint8 camid, uint8 entry, uint8 *type) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (type == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
 
    if (result == SOC_E_NONE) {
        *type = soc_PP_CAM_RAMm_field32_get(unit, &camram, NXT_HDR_TYPEf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_len_set
 *  Description:
 *     Set the length of the header associated with the cam ram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      len - length of header
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_len_set(int unit, uint8 camid, uint8 entry, uint8 len) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (len > SOC_SBX_CALADAN3_PPE_SHIFT_MAX) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->shift = len;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_len_get
 *  Description:
 *     Get the length of the header associated with the cam ram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      len - length of header
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_len_get(int unit, uint8 camid, uint8 entry, uint8 *len) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (len == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        *len = soc_PP_CAM_RAMm_field32_get(unit, &camram, SHIFTf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_variable_hdr_len_set
 *  Description:
 *     Set the variable header parsing parameters for the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      var_hdr - variable header parsing params
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_variable_hdr_len_set(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr ) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->var_len_hdr = *var_hdr;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_variable_hdr_len_get
 *  Description:
 *     get the variable header parsing parameters for the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      var_hdr - variable header parsing params
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_variable_hdr_len_get(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_var_len_hdr_t *var_hdr) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (var_hdr == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));
    sal_memset(&var_hdr, 0, sizeof(soc_sbx_caladan3_ppe_var_len_hdr_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
 
    if (result == SOC_E_NONE) {
        var_hdr->units = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, LENGTH_UNITSf);
        var_hdr->mask = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, LENGTH_MASKf);
        var_hdr->shift_bytes = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, LENGTH_SHIFT_PTRf);
        var_hdr->shift_bits =
            soc_PP_CAM_RAMm_field32_get(unit, &camram, LENGTH_SHIFT_BITSf);
    }
   
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_state_set
 *  Description:
 *     set the current value of state associated with the camram entry
 *     state is only 3 bytes long, expected in big endian format (nw order)
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      state - current state value
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_state_set(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_state_t state, soc_sbx_caladan3_ppe_state_t mask) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    sal_memcpy(camram->state, state, sizeof(soc_sbx_caladan3_ppe_state_t));
    sal_memcpy(camram->state_mask, mask, sizeof(soc_sbx_caladan3_ppe_state_t));
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_state_get
 *  Description:
 *     get the current value of state associated with the camram entry
 *     state is only 3 bytes long, expected in big endian format (nw order)
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      state - current state value
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_state_get(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_state_t state, soc_sbx_caladan3_ppe_state_t mask ) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;
    uint32 data;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((state == NULL) || (mask==NULL)) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        data = soc_PP_CAM_RAMm_field32_get(unit, &camram, STATEf);
        state[0] = (data >> 16) & 0xff;
        state[1] = (data >> 8) & 0xff;
        state[2] = (data & 0xff);
        data = soc_PP_CAM_RAMm_field32_get(unit, &camram, STATE_MASKf);
        mask[0] = (data >> 16) & 0xff;
        mask[1] = (data >> 8) & 0xff;
        mask[2] = (data & 0xff);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_variable_set
 *  Description:
 *     Set the current value of variable associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      var - current variable value
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_variable_set(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_variable_t *var, soc_sbx_caladan3_ppe_variable_t *mask ) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->variable = *var;
    camram->variable_mask = *mask;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_variable_get
 *  Description:
 *     Get the current value of variable associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      var - current variable value
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_variable_get(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_variable_t *var, soc_sbx_caladan3_ppe_variable_t *mask) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((var == NULL) || (mask == NULL)) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        soc_PP_CAM_RAMm_field_get(unit, &camram, VARIABLE_VALUEf, var);
        soc_PP_CAM_RAMm_field_get(unit, &camram, VARIABLE_MASKf, mask);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_stream_set
 *  Description:
 *     Associate an instruction stream to this camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      stream - stream to set
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_stream_set(int unit, uint8 camid, uint8 entry, uint8 stream, uint8 mask) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->stream = stream;
    camram->stream_mask = mask;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_stream_get
 *  Description:
 *     Current value of stream and mask associated with the camram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      stream - current value of stream
 *      mask - current value of stream mask
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_stream_get(int unit, uint8 camid, uint8 entry, uint8 *stream, uint8 *mask) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((stream == NULL) || (mask == NULL)) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        *stream = soc_PP_CAM_RAMm_field32_get(unit, &camram, STREAM_VALUEf);
        *mask = soc_PP_CAM_RAMm_field32_get(unit, &camram, STREAM_MASKf);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_keeptimestamp_set
 *  Description:
 *     Override the source queue data control word if this entry is hit
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      keep - 1 to override load_sq_data, 0 to ignore
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_keeptimestamp_enable(int unit, uint8 camid, uint8 entry, uint8 keep) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->keep_timestamp = keep;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_keepprocreg_get
 *  Description:
 *     get the header checker parameters associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      keep - current value
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_keeptimestamp_get(int unit, uint8 camid, uint8 entry, uint8 *keep) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (keep == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        *keep = soc_PP_CAM_RAMm_field32_get(unit, &camram, KEEP_TIMESTAMPf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_keepprocreg_set
 *  Description:
 *     Override the source queue data control word if this entry is hit
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      keep - 1 to override load_sq_data, 0 to ignore
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_keepprocreg_enable(int unit, uint8 camid, uint8 entry, uint8 keep) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->keep_procreg = keep;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_keepprocreg_get
 *  Description:
 *     get the header checker parameters associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      keep - current value
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_keepprocreg_get(int unit, uint8 camid, uint8 entry, uint8 *keep) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (keep == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        *keep = soc_PP_CAM_RAMm_field32_get(unit, &camram, KEEP_PROCREGf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_keeprepdata_enable
 *  Description:
 *     Override the source queue data control word if this entry is hit
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      keep - 1 to override load_sq_data, 0 to ignore
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_keeprepdata_enable(int unit, uint8 camid, uint8 entry, uint8 keep) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->keep_repdata = keep;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_keeprepdata_get
 *  Description:
 *     get the header checker parameters associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      keep - current value
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_keeprepdata_get(int unit, uint8 camid, uint8 entry, uint8 *keep) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (keep == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        *keep = soc_PP_CAM_RAMm_field32_get(unit, &camram, KEEP_REPDATA2f);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_checker_set
 *  Description:
 *     set the header checker parameters associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      checker - header checker parameters
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_checker_set(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_hdr_checker_t *checker) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->checker = *checker;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_checker_get
 *  Description:
 *     get the header checker parameters associated with the camram entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      checker - header checker parameters
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_checker_get(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_hdr_checker_t *checker) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (checker == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));
    sal_memset(checker, 0, sizeof(soc_sbx_caladan3_ppe_hdr_checker_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
         checker->id = soc_PP_CAM_RAMm_field32_get(unit, &camram, CHECKERf);
         checker->offset =
              soc_PP_CAM_RAMm_field32_get(unit, &camram, CHECKER_OFFSETf);
         if (checker->id != SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_NONE) {
             checker->enable = 1;
         }
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_hasher_set
 *  Description:
 *     set the hash template id associated with the cam ram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      hasher - hash entry 
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_hasher_set(int unit, uint8 camid, uint8 entry,
                                             soc_sbx_caladan3_ppe_hdr_hash_t *hasher ) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->hasher = *hasher;
    return SOC_E_NONE;
    
}
/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_hdr_hasher_get
 *  Description:
 *     Get the hash template id associated with the cam ram
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      hasher - hash entry 
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_hdr_hasher_get(int unit, uint8 camid,
                                        uint8 entry,
                                        soc_sbx_caladan3_ppe_hdr_hash_t *hasher ) {


    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (hasher == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));
    sal_memset(hasher, 0, sizeof(soc_sbx_caladan3_ppe_hdr_hash_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        hasher->hash_id =
            soc_PP_CAM_RAMm_field32_get(unit, &camram, HASH_TEMPLATEf);
        hasher->enable = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, HASH_ENABLEf);
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_property_set
 *  Description:
 *     set the property table parameters associated with a entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      prop - property table params
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_property_set(int unit, uint8 camid, uint8 entry,
                                           soc_sbx_caladan3_ppe_prop_entry_t *prop ) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_camram_t *camram;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    camram = &ppe->cam_cfg[camid].camram[entry];
    camram->prop = *prop;
    return SOC_E_NONE;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_property_get
 *  Description:
 *     Get the property table parameters associated with this entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      prop - property table entry
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_property_get(int unit, uint8 camid, uint8 entry,
                                           soc_sbx_caladan3_ppe_prop_entry_t *prop) {

    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (prop == NULL) {
        return SOC_E_PARAM;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));
    sal_memset(prop, 0, sizeof(soc_sbx_caladan3_ppe_prop_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ANY, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    if (result == SOC_E_NONE) {
        prop->property_enable = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, PROPERTY_ENABLEf);
        prop->property_segment = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, PROPERTY_SEGMENTf);
        prop->property_index_start = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, PROPERTY_INDEX_STARTf);
        prop->property_index_size = 
            soc_PP_CAM_RAMm_field32_get(unit, &camram, PROPERTY_INDEX_SIZEf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_clear
 *  Description:
 *     Flush a camram entry from HW
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_clear(int unit, uint8 camid, uint8 entry) {

    pp_cam_ram_entry_t camram;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&camram, 0, sizeof(pp_cam_ram_entry_t));
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = WRITE_PP_CAM_RAMm(unit, MEM_BLOCK_ALL, index, &camram);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_read
 *  Description:
 *     Read camram enty from HW
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      camram - entry to read into 
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_read(int unit, uint8 camid, uint8 entry,
                                soc_sbx_caladan3_ppe_camram_t *camram) {
    pp_cam_ram_entry_t ram_entry;
    int result = SOC_E_NONE;
    int index;
    int data;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    sal_memset(&ram_entry, 0, sizeof(pp_cam_ram_entry_t));
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = READ_PP_CAM_RAMm(unit, MEM_BLOCK_ALL, index, &ram_entry);
    soc_mem_unlock(unit, PP_CAM_RAMm);

    if (result == SOC_E_NONE) {
        /* header type and len */
        camram->hdr_type = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, NXT_HDR_TYPEf);
        camram->shift = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, SHIFTf);

        /* state */
        data = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, STATEf);
        camram->state[0] = (data >> 16) & 0xff;
        camram->state[1] = (data >> 8) & 0xff;
        camram->state[2] = (data & 0xff);

        data = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, STATE_MASKf);
        camram->state_mask[0] = (data >> 16) & 0xff;
        camram->state_mask[1] = (data >> 8) & 0xff;
        camram->state_mask[2] = (data & 0xff);
 
        /* Variable len header */
        camram->var_len_hdr.units = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, LENGTH_UNITSf);
        camram->var_len_hdr.mask = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, LENGTH_MASKf);
        camram->var_len_hdr.shift_bytes = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, LENGTH_SHIFT_PTRf);
        camram->var_len_hdr.shift_bits =
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, LENGTH_SHIFT_BITSf);

        /* stream */
        camram->stream = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, STREAM_VALUEf);
        camram->stream_mask = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, STREAM_MASKf);

        /* variable */
        camram->variable = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, VARIABLE_VALUEf);
        camram->variable_mask = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, VARIABLE_MASKf);

        /* load sq control */
        camram->keep_procreg = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, KEEP_PROCREGf);
        camram->keep_timestamp = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, KEEP_TIMESTAMPf);
        camram->keep_repdata = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, KEEP_REPDATA2f);

        /* checker */
        camram->checker.id = soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, CHECKERf);
        camram->checker.offset =
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, CHECKER_OFFSETf);
        if (camram->checker.id != SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_TYPE_NONE) {
            camram->checker.enable = 1;
        }

        /* hash */
        camram->hasher.hash_id =
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, HASH_TEMPLATEf);
        camram->hasher.enable = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, HASH_ENABLEf);

        /* prop */
        camram->prop.property_enable = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, PROPERTY_ENABLEf);
        camram->prop.property_segment = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, PROPERTY_SEGMENTf);
        camram->prop.property_index_start = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, PROPERTY_INDEX_STARTf);
        camram->prop.property_index_size = 
            soc_PP_CAM_RAMm_field32_get(unit, &ram_entry, PROPERTY_INDEX_SIZEf);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_camram_entry_write
 *  Description:
 *     Write camram enty to HW
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      camram - entry to write 
 *  Outputs: 
 *      one of SOC_E*
 */
int soc_sbx_caladan3_ppe_camram_entry_write(int unit, uint8 camid, uint8 entry,
                                soc_sbx_caladan3_ppe_camram_t *camram) {
    pp_cam_ram_entry_t ram_entry;
    int result = SOC_E_NONE;
    int index;
    uint32 state, mask;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&ram_entry, 0, sizeof(pp_cam_ram_entry_t));
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, NXT_HDR_TYPEf, camram->hdr_type);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, SHIFTf, camram->shift);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, LENGTH_SHIFT_BITSf, camram->var_len_hdr.shift_bits);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, LENGTH_SHIFT_PTRf, camram->var_len_hdr.shift_bytes);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, LENGTH_MASKf, camram->var_len_hdr.mask);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, LENGTH_UNITSf, camram->var_len_hdr.units);

    state = ((camram->state[0] << 16) | (camram->state[1] << 8) | camram->state[2]);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, STATEf, state);

    mask = ((camram->state_mask[0] << 16) | (camram->state_mask[1] << 8) | camram->state_mask[2]);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, STATE_MASKf, mask);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, VARIABLE_MASKf, camram->variable_mask);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, VARIABLE_VALUEf, camram->variable);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, STREAM_VALUEf, camram->stream);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, STREAM_MASKf, camram->stream_mask);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, KEEP_TIMESTAMPf, camram->keep_timestamp);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, KEEP_PROCREGf, camram->keep_procreg);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, KEEP_REPDATA2f, camram->keep_repdata);
    if (camram->checker.enable) {
        soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, CHECKERf, camram->checker.id);
        soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, CHECKER_OFFSETf, camram->checker.offset);
    }
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, HASH_TEMPLATEf, camram->hasher.hash_id);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, HASH_ENABLEf, camram->hasher.enable);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, PROPERTY_INDEX_SIZEf, camram->prop.property_index_size);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, PROPERTY_INDEX_STARTf, camram->prop.property_index_start);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, PROPERTY_SEGMENTf, camram->prop.property_segment);
    soc_PP_CAM_RAMm_field32_set(unit, &ram_entry, PROPERTY_ENABLEf, camram->prop.property_enable);
    soc_mem_lock(unit, PP_CAM_RAMm);
    result = WRITE_PP_CAM_RAMm(unit, MEM_BLOCK_ALL, index, &ram_entry);
    soc_mem_unlock(unit, PP_CAM_RAMm);
    return result;
    
}


/***
 ***  PPE TCam Entry
 ***/

/*
 *  Routine: sbx_caladan3_cam_entry_encode
 *  Description:
 *     Convert from user type to HW encoded type
 *  Inputs:
 *      enc_cam - packed entry
 *      cam - user programmable type
 *  Outputs: 
 *      one of SOC_E*
 */
int sbx_caladan3_cam_entry_encode(soc_sbx_caladan3_ppe_enc_tcamdata_t *enc_cam,
                                  soc_sbx_caladan3_ppe_tcamdata_t *cam) {
    uint32 data;
    uint32 mask;
    int i, j = 0, n, max;

    sal_memset(enc_cam, 0, sizeof(soc_sbx_caladan3_ppe_enc_tcamdata_t));
    max = SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH;
    n = SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH % 4;
    for (i=max-1, j=0; i > n; i-=4, j++) {
        data = ((cam->data[i] ) | (cam->data[i-1] << 8) | 
                (cam->data[i-2] << 16) | (cam->data[i-3] << 24));
        mask = ((cam->mask[i] ) | (cam->mask[i-1] << 8) | 
                (cam->mask[i-2] << 16) | (cam->mask[i-3] << 24));
        enc_cam->key0[j] = data;
        enc_cam->key1[j] = mask;
    }
    data = cam->data[i];
    mask = cam->mask[i];
    data |= (cam->state[2] << 8) |
               (cam->state[1] << 16) | (cam->state[0] << 24);
    mask |= (cam->state_mask[2] << 8) |
               (cam->state_mask[1] << 16) | (cam->state_mask[0] << 24);
    enc_cam->key0[j] = data;
    enc_cam->key1[j] = mask;
    j++;
    data = cam->property;
    mask = cam->property_mask;
    enc_cam->key0[j] = data;
    enc_cam->key1[j] = mask;

#ifdef PPE_DEBUG
    LOG_CLI((BSL_META("\nData: ")));
    for (i=0; i<=j; i++) {
        LOG_CLI((BSL_META("0x%08x "), enc_cam->key0[i]));
    }
    LOG_CLI((BSL_META("\n")));
    LOG_CLI((BSL_META("%2x "), cam->property));
    LOG_CLI((BSL_META("%02x%02x%02x "), cam->state[2], cam->state[1], cam->state[0]));
    for (i=0; i<=j-2; i++) {
        LOG_CLI((BSL_META("%02x%02x%02x%02x "), cam->data[i*4 + 0], cam->data[i*4 + 1], cam->data[i*4 + 2], cam->data[i*4 + 3]));
    }
    LOG_CLI((BSL_META("\nMask: ")));
    for (i=0; i<=j; i++) {
        LOG_CLI((BSL_META("0x%08x "), enc_cam->key1[i]));
    }
    LOG_CLI((BSL_META("\n")));
    LOG_CLI((BSL_META("%2x "), cam->property_mask));
    LOG_CLI((BSL_META("%02x%02x%02x "), cam->state_mask[2], cam->state_mask[1], cam->state_mask[0]));
    for (i=0; i<=j-2; i++) {
        LOG_CLI((BSL_META("%02x%02x%02x%02x "), cam->mask[i*4 + 0], cam->mask[i*4 + 1], cam->mask[i*4 + 2], cam->mask[i*4 + 3]));
    }
#endif

    return SOC_E_NONE;
}

/*
 *  Routine: sbx_caladan3_cam_entry_decode
 *  Description:
 *     Unpack entry, decoding if required
 *  Inputs:
 *      enc_cam - packed entry
 *      cam - user programmable type
 *  Outputs: 
 *      one of SOC_E*
 */
int sbx_caladan3_cam_entry_decode(soc_sbx_caladan3_ppe_enc_tcamdata_t *enc_cam,
                                  soc_sbx_caladan3_ppe_tcamdata_t *cam) {
    uint32 data;
    uint32 mask;
    int i, j, n;

    /* Index of the property byte */
    n = SOC_SBX_CALADAN3_PPE_TCAM_DATA_LENGTH_WORDS;
    data = enc_cam->key0[n];
    mask = enc_cam->key1[n];
    cam->property = data  & 0xFF;
    cam->property_mask = mask  & 0xff;
    n--;

    /* Index of the Cam state */
    data = enc_cam->key0[n];
    mask = enc_cam->key1[n];
    cam->state[0] = (data >> 24) & 0xff;
    cam->state_mask[0] = (mask >> 24) & 0xff;
    cam->state[1] = (data >> 16) & 0xff;
    cam->state_mask[1] = (mask >> 16) & 0xff;
    cam->state[2] = (data >> 8) & 0xff;
    cam->state_mask[2] = (mask >> 8) & 0xff;
    cam->data[0] = (data & 0xFF);
    cam->mask[0] = (mask & 0xFF);

    for (i=n-1, j=1; i >= 0; i--, j+=4) {
        data = enc_cam->key0[i];
        mask = enc_cam->key1[i];
        cam->data[j]   = (data >> 24) & 0xFF;
        cam->data[j+1] = (data >> 16) & 0xFF;
        cam->data[j+2] = (data >>  8) & 0xFF;
        cam->data[j+3] = (data >>  0) & 0xFF;
        cam->mask[j]   = (mask >> 24) & 0xFF;
        cam->mask[j+1] = (mask >> 16) & 0xFF;
        cam->mask[j+2] = (mask >>  8) & 0xFF;
        cam->mask[j+3] = (mask >>  0) & 0xFF;
    }

#ifdef PPE_DEBUG
    LOG_CLI((BSL_META("\nDecode Data: ")));
    for (i=0; i<=n+1; i++) {
        LOG_CLI((BSL_META("0x%08x "), enc_cam->key0[i]));
    }
    LOG_CLI((BSL_META("\n")));
    LOG_CLI((BSL_META("%2x "), cam->property));
    LOG_CLI((BSL_META("%02x%02x%02x "), cam->state[2], cam->state[1],  cam->state[0]));
    for (i=0; i<=n-1; i++) {
        LOG_CLI((BSL_META("%02x%02x%02x%02x "), cam->data[i*4 + 0], cam->data[i*4 + 1], cam->data[i*4 + 2], cam->data[i*4 + 3]));
    }
    LOG_CLI((BSL_META("\nDecode Mask: ")));
    for (i=0; i<=n+1; i++) {
        LOG_CLI((BSL_META("0x%08x "), enc_cam->key1[i]));
    }
    LOG_CLI((BSL_META("\n")));
    LOG_CLI((BSL_META("%2x "), cam->property_mask));
    LOG_CLI((BSL_META("%02x%02x%02x "), cam->state_mask[2], cam->state_mask[1], cam->state_mask[0]));
    for (i=0; i<=n-1; i++) {
        LOG_CLI((BSL_META("%02x%02x%02x%02x "), cam->mask[i*4 + 0], cam->mask[i*4 + 1], cam->mask[i*4 + 2], cam->mask[i*4 + 3]));
    }
#endif

    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_src_rep_set
 *  Description:
 *     Get the SRC_TYPE and REP_DATA associated with the tcam entry
 *     Note this is applicable only for the first lookup
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      src_type - SRC_TYPE to match
 *      rep_data - REP_DATA[5:0] to match
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_src_rep_set(int unit, uint8 camid, uint8 entry, uint8 src_type, uint8 rep_data) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_tcamdata_t *tcam;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    tcam = &ppe->cam_cfg[camid].tcam[entry];
    tcam->property = 
       (((src_type & SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_MASK) << 
            SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_OFFSET)       |
         (rep_data & SOC_SBX_CALADAN3_PPE_TCAM_REP_DATA_MASK));
    tcam->property_mask = 0xFF;
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_src_rep_get
 *  Description:
 *     Get the SRC_TYPE and REP_DATA associated with the tcam entry
 *     Note this is applicable only for the first lookup
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      src_type - current src_type associated with the tcam
 *      rep_data - current rep_data associated with the tcam
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_src_rep_get(int unit, uint8 camid, uint8 entry, uint8 *src_type, uint8 *rep_data) {
    pp_tcam_entry_t tcam_entry;
    soc_sbx_caladan3_ppe_tcamdata_t tcam;
    soc_sbx_caladan3_ppe_enc_tcamdata_t enc_cam;
    
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&tcam_entry, 0, sizeof(pp_tcam_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_TCAMm);
    result = READ_PP_TCAMm(unit, MEM_BLOCK_ANY, index, &tcam_entry);
    soc_mem_unlock(unit, PP_TCAMm);
    if (result == SOC_E_NONE) {
         soc_PP_TCAMm_field_get(unit, &tcam_entry, DATAf, enc_cam.key0);
         soc_PP_TCAMm_field_get(unit, &tcam_entry, MASKf, enc_cam.key1);
         sbx_caladan3_cam_entry_decode(&enc_cam, &tcam);
         *src_type = ((tcam.property >> SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_OFFSET) & SOC_SBX_CALADAN3_PPE_TCAM_SRC_TYPE_MASK);
         *rep_data = (tcam.property & SOC_SBX_CALADAN3_PPE_TCAM_REP_DATA_MASK);
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_property_set
 *  Description:
 *    Set the property byte and mask for the tcam entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      property - property value associated with the entry
 *      property_mask - property mask value associated with the entry
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_property_set(int unit, uint8 camid, uint8 entry, uint8 property) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_tcamdata_t *tcam;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    tcam = &ppe->cam_cfg[camid].tcam[entry];
    tcam->property = property;
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_property_get
 *  Description:
 *     Get the propety byte and its associted mask
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      property - property value associated with the entry
 *      property_mask - property mask value associated with the entry
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_property_get(int unit, uint8 camid, uint8 entry, uint8 *property, uint8 *property_mask) {

    pp_tcam_entry_t tcam_entry;
    soc_sbx_caladan3_ppe_tcamdata_t tcam;
    soc_sbx_caladan3_ppe_enc_tcamdata_t enc_cam;
    
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&tcam_entry, 0, sizeof(pp_tcam_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_TCAMm);
    result = READ_PP_TCAMm(unit, MEM_BLOCK_ANY, index, &tcam_entry);
    soc_mem_unlock(unit, PP_TCAMm);
    if (result == SOC_E_NONE) {
         soc_PP_TCAMm_field_get(unit, &tcam_entry, DATAf, enc_cam.key0);
         soc_PP_TCAMm_field_get(unit, &tcam_entry, MASKf, enc_cam.key1);
         sbx_caladan3_cam_entry_decode(&enc_cam, &tcam);
         *property = tcam.property;
         *property_mask = tcam.property_mask;
    }
    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_state_set
 *  Description:
 *     Set the state and mask of the tcam entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      state - state associated with the tcam
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_state_set(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_state_t state, soc_sbx_caladan3_ppe_state_t mask) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    soc_sbx_caladan3_ppe_tcamdata_t *tcam;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    tcam = &ppe->cam_cfg[camid].tcam[entry];
    sal_memcpy(tcam->state, state, sizeof(soc_sbx_caladan3_ppe_state_t));
    sal_memcpy(tcam->state_mask, mask, sizeof(soc_sbx_caladan3_ppe_state_t));
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_state_get
 *  Description:
 *     Get the State and Mask from the TCAM entry
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      state - state associated with the tcam
 *      mask - associated mask
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_state_get(int unit, uint8 camid, uint8 entry, soc_sbx_caladan3_ppe_state_t state, soc_sbx_caladan3_ppe_state_t mask) {

    pp_tcam_entry_t tcam_entry;
    soc_sbx_caladan3_ppe_tcamdata_t tcam;
    soc_sbx_caladan3_ppe_enc_tcamdata_t enc_cam;
    
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&tcam_entry, 0, sizeof(pp_tcam_entry_t));

    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);
    soc_mem_lock(unit, PP_TCAMm);
    result = READ_PP_TCAMm(unit, MEM_BLOCK_ANY, index, &tcam_entry);
    soc_mem_unlock(unit, PP_TCAMm);
    if (result == SOC_E_NONE) {
         soc_PP_TCAMm_field_get(unit, &tcam_entry, DATAf, enc_cam.key0);
         soc_PP_TCAMm_field_get(unit, &tcam_entry, MASKf, enc_cam.key1);
         sbx_caladan3_cam_entry_decode(&enc_cam, &tcam);
         sal_memcpy(state, tcam.state, sizeof(soc_sbx_caladan3_ppe_state_t));
         sal_memcpy(mask, tcam.state_mask, sizeof(soc_sbx_caladan3_ppe_state_t));
    }
    return result;
    
}


/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_clear
 *  Description:
 *     Delete an entry from hw and clear it
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_clear(int unit, uint8 camid, uint8 entry) {

    pp_tcam_entry_t tcam;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }

    sal_memset(&tcam, 0, sizeof(pp_tcam_entry_t));
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    soc_mem_lock(unit, PP_TCAMm);
    result = WRITE_PP_TCAMm(unit, MEM_BLOCK_ALL, index, &tcam);
    soc_mem_unlock(unit, PP_TCAMm);

    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_read
 *  Description:
 *     Get the tcam entry from hw
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_read(int unit, uint8 camid, uint8 entry,
                                soc_sbx_caladan3_ppe_tcamdata_t *cam, uint8 *valid) {

    soc_sbx_caladan3_ppe_enc_tcamdata_t enc_cam;
    pp_tcam_entry_t tcam;
    int index;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&tcam, 0, sizeof(pp_tcam_entry_t));
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    soc_mem_lock(unit, PP_TCAMm);
    result = READ_PP_TCAMm(unit, MEM_BLOCK_ANY, index, &tcam);
    soc_mem_unlock(unit, PP_TCAMm);

    if (result == SOC_E_NONE) {
        soc_PP_TCAMm_field_get(unit, &tcam, DATAf, enc_cam.key0);
        soc_PP_TCAMm_field_get(unit, &tcam, MASKf, enc_cam.key1);
        sbx_caladan3_cam_entry_decode(&enc_cam, cam);
        *valid = soc_PP_TCAMm_field32_get(unit, &tcam, VALIDf);
    }

    return result;
    
}
/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_write
 *  Description:
 *     Write a tcam entry to Hw and set valid state
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *      cam - pointer to tcam entry
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_write(int unit, uint8 camid, uint8 entry,
                                 soc_sbx_caladan3_ppe_tcamdata_t *cam, uint8 valid) {

    soc_sbx_caladan3_ppe_enc_tcamdata_t enc_cam;
    pp_tcam_entry_t tcam;
    int index;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    if ((valid != SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_VALID) &&
       (valid != SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID)) {
        return SOC_E_PARAM;
    }

    /*sal_memset(&tcam, 0, sizeof(pp_tcam_entry_t));*/
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    sbx_caladan3_cam_entry_encode(&enc_cam, cam);

    soc_PP_TCAMm_field_set(unit, &tcam, DATAf, enc_cam.key0);
    soc_PP_TCAMm_field_set(unit, &tcam, MASKf, enc_cam.key1);
    soc_PP_TCAMm_field32_set(unit, &tcam, VALIDf, valid);
    soc_mem_lock(unit, PP_TCAMm);
    result = WRITE_PP_TCAMm(unit, MEM_BLOCK_ALL, index, &tcam);
    soc_mem_unlock(unit, PP_TCAMm);

    return result;
    
}
    

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_valid_set
 *  Description:
 *     Set valid bits of an tcam entry in hw
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_valid_set(int unit, uint8 camid, uint8 entry, uint8 valid) {

    pp_tcam_entry_t tcam;
    int result = SOC_E_NONE;
    int index;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&tcam, 0, sizeof(pp_tcam_entry_t));
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    soc_mem_lock(unit, PP_TCAMm);
    result = READ_PP_TCAMm(unit, MEM_BLOCK_ANY, index, &tcam);
    if (result == SOC_E_NONE) {
        soc_PP_TCAMm_field32_set(unit, &tcam, VALIDf, valid);
        result = WRITE_PP_TCAMm(unit, MEM_BLOCK_ALL, index, &tcam);
    }
    soc_mem_unlock(unit, PP_TCAMm);

    return result;
    
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_valid_get
 *  Description:
 *     Get the valid bits of a tcam entry from hw
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_valid_get(int unit, uint8 camid, uint8 entry, uint8 *valid) {

    pp_tcam_entry_t tcam;
    int index;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (valid == NULL) {
        return SOC_E_NONE;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&tcam, 0, sizeof(pp_tcam_entry_t));
    index = soc_sbx_caladan3_ppe_camid_to_index(camid, entry);

    soc_mem_lock(unit, PP_TCAMm);
    result = READ_PP_TCAMm(unit, MEM_BLOCK_ANY, index, &tcam);
    soc_mem_unlock(unit, PP_TCAMm);
    if (result == SOC_E_NONE) {
        *valid = soc_PP_TCAMm_field32_get(unit, &tcam, VALIDf);
    }

    return result;
}

/***
 *** CAM programming interface
 ***/

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_program
 *  Description:
 *     Program an tcam entry an associted cam ram
 *     Follows: Invalidate, update camram, update tcam rule
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_program(int unit, uint8 camid, uint8 entry) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }
    ppe = C3_PPE(unit);
    result = soc_sbx_caladan3_ppe_tcam_entry_valid_set(unit, camid, entry, 
                                                   SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_INVALID);
    if (result == SOC_E_NONE) {
        result = soc_sbx_caladan3_ppe_camram_entry_write(unit, camid, entry, 
                                                     &ppe->cam_cfg[camid].camram[entry]);
        if (result == SOC_E_NONE) {
            result = soc_sbx_caladan3_ppe_tcam_entry_write(unit, camid, entry, 
                                                       &ppe->cam_cfg[camid].tcam[entry],
                                                       SOC_SBX_CALADAN3_PPE_TCAM_ENTRY_VALID);
        }
    }
    return result;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_tcam_entry_destrory
 *  Description:
 *     Clear a tcam entry and associated camram entry from hw
 *  Inputs:
 *      camid - cam state (0-13)
 *      entry - index of the entry (0-126)
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_tcam_entry_destrory(int unit, uint8 camid, uint8 entry) {
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((camid >= SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX) ||
       (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX)) {
        return SOC_E_PARAM;
    }

    result = soc_sbx_caladan3_ppe_tcam_entry_clear(unit, camid, entry);
    if (result == SOC_E_NONE) {
        result = soc_sbx_caladan3_ppe_camram_entry_clear(unit, camid, entry);
    }
    return result;
}

/*
 * Assembler config
 */

/*
 *  Routine: soc_sbx_caladan3_ppe_exception_stream_set
 *  Description:
 *     Set the stream id for packets taking a exception at the ppe
 *  Inputs:
 *      stream - stream id
 *      enable - enable/disable pushing the packet to the stream on exception
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_exception_stream_set(int unit, uint8 stream, uint8 enable) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    if (stream > SOC_SBX_CALADAN3_PPE_STREAM_MAX) {
        return SOC_E_PARAM;
    }
    ppe->assembler_config.exception_stream = stream;
    ppe->assembler_config.exception_str_enable = (enable > 0) ? 1 : 0;

    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        soc_reg_field_set(unit, PP_ASSEMBLER_CONFIGr, &rval, STR_VALUEf,
                      ppe->assembler_config.exception_stream);
        soc_reg_field_set(unit, PP_ASSEMBLER_CONFIGr, &rval, STR_EXCEPTION_ENf,
                      ppe->assembler_config.exception_str_enable);
        result = WRITE_PP_ASSEMBLER_CONFIGr(unit, rval);
    }
    return result;
}
/*
 *  Routine: int soc_sbx_caladan3_ppe_exception_stream_get
 *  Description:
 *     Get the target stream id for the exception packets and if the feature is enabled
 *  Inputs:
 *  Outputs: 
 *      stream - stream id
 *      enable - enable/disable pushing the packet to the stream on exception
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_exception_stream_get(int unit, uint8 *stream, uint8 *enable) {
    int result = SOC_E_NONE;
    uint32 rval;
    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if ((stream == NULL) || (enable == NULL)) {
        return SOC_E_PARAM;
    }
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        *stream = soc_reg_field_get(unit, PP_ASSEMBLER_CONFIGr, rval, STR_VALUEf);
        *enable = soc_reg_field_get(unit, PP_ASSEMBLER_CONFIGr, rval, STR_EXCEPTION_ENf);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_exception_stream_enable_set
 *  Description:
 *     Enable pushing exception packet to a specified stream
 *  Inputs:
 *      enable - enabled/disable
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_exception_stream_enable_set(int unit, uint8 enable) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->assembler_config.exception_str_enable = (enable > 0) ? 1 : 0;
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        soc_reg_field_set(unit, PP_ASSEMBLER_CONFIGr, &rval, STR_EXCEPTION_ENf,
                      ppe->assembler_config.exception_str_enable);
        result = WRITE_PP_ASSEMBLER_CONFIGr(unit, rval);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_exception_stream_enable_get
 *  Description:
 *     Get the current state of the enable flag
 *  Inputs:
 *  Outputs: 
 *      enable - current state enabled/disabled
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_exception_stream_enable_get(int unit, uint8 *enable) {
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (enable == NULL) {
        return SOC_E_PARAM;
    }
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        *enable = soc_reg_field_get(unit, PP_ASSEMBLER_CONFIGr, rval, STR_EXCEPTION_ENf);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_keep_replication_for_copies_set
 *  Description:
 *     Force ppe to ignore load_sq_data control for replicant packets
 *     This affects only the REPLICANT_DATA part of the header
 *  Inputs:
 *      enable - enable/disable the feature
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_keep_replication_for_copies_set(int unit, uint8 enable) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->assembler_config.keep_rep_for_copies = (enable > 0) ? 1 : 0;
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        soc_reg_field_set(unit, PP_ASSEMBLER_CONFIGr, &rval, KEEP_REPDATA_FOR_COPIESf,
                      ppe->assembler_config.keep_rep_for_copies);
        result = WRITE_PP_ASSEMBLER_CONFIGr(unit, rval);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_keep_replication_for_copies_get
 *  Description:
 *     Get if the PPE is set to ignore load_sq_data control for replicant packets
 *     This affects only the REPLICANT_DATA part of the header
 *  Inputs:
 *  Outputs: 
 *      enable - current state enabled/disabled
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_keep_replication_for_copies_get(int unit, uint8 *enable) {

    int result = SOC_E_NONE;
    uint32 rval;
    if (!CALADAN3_PPE_READY(unit)) {
        return SOC_E_INIT;
    }
    if (enable == NULL) {
        return SOC_E_PARAM;
    }
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        *enable = soc_reg_field_get(unit, PP_ASSEMBLER_CONFIGr, rval, KEEP_REPDATA_FOR_COPIESf);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_load_seq_number_set
 *  Description:
 *     Load the 16bit sequence number instead of proc register value in the PROC_REG field
 *     of the header discriptor
 *  Inputs:
 *      enable - enable/disable the feature
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_load_seq_number_set(int unit, uint8 enable) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->assembler_config.load_seq_number = (enable>0) ? 1: 0;
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        soc_reg_field_set(unit, PP_ASSEMBLER_CONFIGr, &rval, LOAD_SEQNUMf,
                      ppe->assembler_config.load_seq_number);
        result = WRITE_PP_ASSEMBLER_CONFIGr(unit, rval);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_load_seq_number_get
 *  Description:
 *     Get the state of load_seq_number setting
 *  Inputs:
 *  Outputs: 
 *      enable - current state enabled/disabled
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_load_seq_number_get(int unit, uint8 *enable) {
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (enable == NULL) {
        return SOC_E_PARAM;
    }
    result = READ_PP_ASSEMBLER_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        *enable = soc_reg_field_get(unit, PP_ASSEMBLER_CONFIGr, rval, LOAD_SEQNUMf);
    }
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_proc_reg_set
 *  Description:
 *     Set the proc register value
 *  Inputs:
 *     proc_reg - value to set
 *  Outputs: 
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_proc_reg_set(int unit, uint32 proc_reg) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    ppe = C3_PPE(unit);
    ppe->assembler_config.proc_reg_value = proc_reg;
    result = WRITE_PP_ASSEMBLER_PROC_REG_CONFIGr(unit, proc_reg);
    return result;
}
/*
 *  Routine: soc_sbx_caladan3_ppe_proc_reg_get
 *  Description:
 *     Get the proc register value
 *  Inputs:
 *     -
 *  Outputs: 
 *      proc_reg value
 *      one of SOC_E* status types
 */
int soc_sbx_caladan3_ppe_proc_reg_get(int unit, uint32 *proc_reg) {
    int result = SOC_E_NONE;
    uint32 rval;

    if (!CALADAN3_PPE_READY(unit)) {
         return SOC_E_INIT;
    }
    if (proc_reg == NULL) {
        return SOC_E_PARAM;
    }
    
    result = READ_PP_ASSEMBLER_PROC_REG_CONFIGr(unit, &rval);
    if (result == SOC_E_NONE) {
        *proc_reg = rval;
    }
    return result;
}

/*
 * PPE Event handling
 */

/*
 * PPE initialization
 */
int sbx_caldan3_ppe_hw_self_init (int unit) {
    uint32 rval;
    int result = SOC_E_NONE;
    rval = 0;
    soc_reg_field_set(unit, PP_GLOBAL_CONFIGr, &rval, SOFT_RESET_Nf, 1);
    WRITE_PP_GLOBAL_CONFIGr(unit, rval);

    soc_reg_field_set(unit, PP_GLOBAL_CONFIGr, &rval, INITf, 1);
    result = WRITE_PP_GLOBAL_CONFIGr(unit, rval);
    if (result == SOC_E_NONE) {
        result = soc_sbx_caladan3_reg32_expect_field_timeout(unit, PP_GLOBAL_EVENTr, -1,
                                                             0, -1, INIT_DONEf, 1, -1/*default*/);
        if (result == SOC_E_NONE) {
            /* turn on tcam scruber, single bit correct, multi-bit invalidate */
            SOC_IF_ERROR_RETURN(READ_PP_TCAM_ECC_DEBUG0r(unit, &rval));
            soc_reg_field_set(unit, PP_TCAM_ECC_DEBUG0r, &rval, TCAM_SCAN_SEC_CORRECTf, 1);
            soc_reg_field_set(unit, PP_TCAM_ECC_DEBUG0r, &rval, TCAM_SCAN_SEC_INVALIDf, 0);
            soc_reg_field_set(unit, PP_TCAM_ECC_DEBUG0r, &rval, TCAM_SCAN_DEC_INVALIDf, 0);
            SOC_IF_ERROR_RETURN(WRITE_PP_TCAM_ECC_DEBUG0r(unit, rval));

            SOC_IF_ERROR_RETURN(READ_PP_TCAM_ECC_DEBUG1r(unit, &rval));
            soc_reg_field_set(unit, PP_TCAM_ECC_DEBUG1r, &rval, TCAM_SCAN_WINDOWf, 0x7FFFFFFF);
            SOC_IF_ERROR_RETURN(WRITE_PP_TCAM_ECC_DEBUG1r(unit, rval));    
        }
    }
    return result;
}

int soc_sbx_caladan3_ppe_driver_init(int unit) {

    soc_sbx_caladan3_ppe_config_t *ppe;
    int result = SOC_E_NONE;
    sal_mutex_t ppe_lock;

#ifdef 	BCM_WARM_BOOT_SUPPORT
        result = soc_sbx_ppe_wb_state_init(unit);
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        ppe_lock = sal_mutex_create("C3PPE_MUTEX");
        if (ppe_lock == NULL) {
            return SOC_E_RESOURCE;
        } else {
            ppe = C3_PPE(unit);
            ppe->lock = ppe_lock;
        }
    } else
#endif
    {
        if (CALADAN3_PPE_READY(unit)) {
            result = soc_sbx_caladan3_ppe_driver_uninit(unit);
            if (result != SOC_E_NONE) {
                return result;
            }
        }

        ppe_lock = sal_mutex_create("C3PPE_MUTEX");
        if (ppe_lock == NULL) {
            return SOC_E_RESOURCE;
        }

        sal_mutex_take(ppe_lock, sal_mutex_FOREVER);
        result = sbx_caldan3_ppe_hw_self_init(unit);
        
        if (result == SOC_E_NONE) {
            ppe = C3_PPE(unit);
            sal_memset(ppe, 0, sizeof(soc_sbx_caladan3_ppe_config_t));
            ppe->unit_active = 1;
            ppe->lock = ppe_lock;
            sal_mutex_give(ppe_lock);
        } else {
            sal_mutex_give(ppe_lock);
            sal_mutex_destroy(ppe_lock);
        }
    }
    return result;
}

int soc_sbx_caladan3_ppe_driver_uninit(int unit) {
    soc_sbx_caladan3_ppe_config_t *ppe;
    sal_mutex_t ppe_lock;


    /* Free up PPE resources */
    ppe = C3_PPE(unit);
    if (ppe != NULL) {
        /* Check if the mutex is active and if so destroy it */
        ppe_lock = ppe->lock;
        if (ppe_lock != NULL) {
            sal_mutex_destroy(ppe_lock);
            ppe_lock = NULL;
        }
    }

    return SOC_E_NONE;
}


int 
soc_sbx_caladan3_ppe_hc_control(int unit, int on, int clear,
                                int queue, int str, int var, int varmask)
{
    uint32 regval, old_regval;
    uint32 copy_enable_mask = 0;
    int result = SOC_E_NONE;

    result = READ_PP_HDR_COPY_CONTROL0r(unit, &regval);
    if (result == SOC_E_NONE) {
        copy_enable_mask = soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r,
                                             regval, COPY_MASKf);
    }

    if (clear) {
        /* Clear the capture buffer */
        regval = 0;
        old_regval = 0;
        result = READ_PP_HDR_COPY_CONTROL0r(unit, &old_regval);
        if (result == SOC_E_NONE) {
            soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_MASKf, 0);
            soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_ENABLEf, 0);
            WRITE_PP_HDR_COPY_CONTROL0r(unit, regval);
            soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_ENABLEf, 1);
            WRITE_PP_HDR_COPY_CONTROL0r(unit, regval);
            WRITE_PP_HDR_COPY_CONTROL0r(unit, old_regval);
        }
    } else {
        if (queue > -1) {
            soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, 
                              COPY_SQUEUEf, queue);
            copy_enable_mask &= 
                    ~SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_SQUEUE;
        } else {
            copy_enable_mask |= 
                    SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_SQUEUE;
        }
        if (str > -1) {
            soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, 
                              COPY_STREAMf, str);
            copy_enable_mask &= 
                    ~SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_STREAM;
        } else {
            copy_enable_mask |= 
                     SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_STREAM;
        }
        if (varmask != 0) {
            WRITE_PP_HDR_COPY_CONTROL1r(unit, var);
            WRITE_PP_HDR_COPY_CONTROL2r(unit, varmask);
            copy_enable_mask &= 
                  ~SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_VARIABLE;
        } else {
            copy_enable_mask |= 
                  SOC_SBX_CALADAN3_PPE_HDR_CAPTURE_IGNORE_VARIABLE;
        }
        soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_ENABLEf,
                          on);
        soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_MASKf, 
                          copy_enable_mask);
        WRITE_PP_HDR_COPY_CONTROL0r(unit, regval);
    }
    return SOC_E_NONE;
}

static soc_sbx_caladan3_hdesc_format_t ppe_hdesc_format[] = {
{  1, "cont ",          24,  8},  
{  1, "hdr0 ",          16,  6}, 
{  1, "hdr1 ",           8,  6}, 
{  1, "hdr2 ",           0,  6}, 
{  2, "hdr3 ",          24,  6}, 
{  2, "hdr4 ",          16,  6}, 
{  2, "hdr5 ",           8,  6}, 
{  2, "hdr6 ",           0,  6}, 
{  3, "hdr7 ",          24,  6}, 
{  3, "hdr8 ",          16,  6}, 
{  3, "hdr9 ",           8,  6}, 
{  3, "hdr10",          0,  6}, 
{  4, "hdr11",         24,  6}, 
{  4, "hdr12",         16,  6}, 
{  4, "hdr13",          8,  6}, 
{  4, "hdr14",          0,  6}, 
{  5, "loc0 ",          24,  8}, 
{  5, "loc1 ",          16,  8}, 
{  5, "loc2 ",           8,  8}, 
{  5, "loc3 ",           0,  8}, 
{  6, "loc4 ",          24,  8}, 
{  6, "loc5 ",          16,  8}, 
{  6, "loc6 ",           8,  8}, 
{  6, "loc7 ",           0,  8}, 
{  7, "loc8 ",          24,  8}, 
{  7, "loc9 ",          16,  8}, 
{  7, "loc10",          8,  8}, 
{  7, "loc11",          0,  8}, 
{  8, "loc12",         24,  8}, 
{  8, "loc13",         16,  8}, 
{  8, "loc14",          8,  8}, 
{  8, "loc15",          0,  8}, 
{  9, "ccde ",         30,  2}, 
{  9, "sbuf ",         16, 14}, 
{  9, "dque ",          8,  8}, 
{  9, "sque ",          0,  7}, 
{  9, "egress",         7,  1}, 
{ 10, "flen ",         16, 14}, 
{ 10, "cnum ",          0, 12}, 
{ 11, "str  ",         28,  4}, 
{ 11, "drop ",         27,  1}, 
{ 11, "err  ",         26,  1}, 
{ 11, "src  ",         22,  2}, 
{ 11, "ccdp ",         16,  2}, 
{ 11, "excid",          0,  8}, 
{ 12, "variable ",      0, 32}, 
{ 13, "repdata  ",      0, 32}, 
{ 14, "procreg  ",      0, 32}, 
{ 14, "seqnum   ",     16, 16}, 
{ 15, "aggrhash ",      0, 32}, 
{ 16, "timestamp",      0, 32}, 
{ 0 }
};

int 
soc_sbx_caladan3_ppe_hd_parsed(uint32 *rawdata, int max)
{
    int i, j, s, data, maxdesc;
    uint32 m;
    soc_sbx_caladan3_hdesc_format_t *element;

    s = i = 0; maxdesc = COUNTOF(ppe_hdesc_format);
    for (j = 0; j < maxdesc; ) {
        element = &ppe_hdesc_format[j];
        if (!element->word) {
            break;
        }
        /*if ((s == 0) || (s >=50)) {*/
        if (s == i) {
            LOG_CLI((BSL_META("\nPPEDesc> 0x%02x"), i<<2));
            s++;
        }
        if (element->word-1 == i) {
            m = (1 << element->size) - 1;
            data = (rawdata[i] >> element->pos) & m;
            LOG_CLI((BSL_META(" %s:%0*x"), element->desc,
                     (element->size+3)/4, data));
            j++;
        } else {
            i++; 
        }
    }
    for (i++; i < max; i++) {
        if (i%4==0) {
            LOG_CLI((BSL_META("\nPPEData> 0x%02x"), i<<2));
        }
        LOG_CLI((BSL_META(" %08x"), rawdata[i]));
    }
    return SOC_E_NONE;
}

int 
soc_sbx_caladan3_ppe_hd(int unit, int parsed, uint32 *header)
{
    uint32 regval, old_regval;
    uint32 numpkts;
    int rv, i, j, p;
    pp_cpdm_entry_t pktdata;
    uint32 data[SOC_SBX_CALADAN3_PPE_CPDM_MAX_WORDS];
    uint32 *rp, rawdata[256];

    READ_PP_HDR_COPY_CONTROL0r(unit, &regval);
    numpkts = soc_reg_field_get(unit, PP_HDR_COPY_CONTROL0r,
                                regval, COPY_COUNTf);
    for (p =0; p < numpkts; p++) {
            if (header)
                rp = header + (64 * p);
            else {
                rp = &rawdata[0];
                LOG_CLI((BSL_META_U(unit,
                                    "\nPPE Header Descriptor for packet %d %s"), p, parsed ? "" : "(raw)" ));
            }

            soc_mem_lock(unit, PP_CPDMm);

            /* Data is spread over SOC_SBX_CALADAN3_PPE_CPDM_NUM_ENT_PER_PACKET entries */
            for (j=0; j<SOC_SBX_CALADAN3_PPE_CPDM_NUM_ENT_PER_PACKET; j++) {
                rv = READ_PP_CPDMm(unit, MEM_BLOCK_ALL, 
                                   (p * SOC_SBX_CALADAN3_PPE_CPDM_NUM_ENT_PER_PACKET + j), 
                                   &pktdata);
                if (SOC_SUCCESS(rv)) {
                    soc_PP_CPDMm_field_get(unit, &pktdata, DATAf, data);
                    for (i=0; i < SOC_SBX_CALADAN3_PPE_CPDM_MAX_WORDS; i++) {
                        if ((!parsed) && (i%4 == 0) && !header) {
                            LOG_CLI((BSL_META_U(unit,
                                                "\n0x%02x"), (j*SOC_SBX_CALADAN3_PPE_CPDM_MAX_BYTES) + (i*4)));
                        }
                        /* Data is in inverted word order */
                        if (parsed || header) {
                            *rp++ = data[SOC_SBX_CALADAN3_PPE_CPDM_MAX_WORDS -i -1];
                        } else {
                            LOG_CLI((BSL_META_U(unit,
                                                " %08x"), data[SOC_SBX_CALADAN3_PPE_CPDM_MAX_WORDS -i -1]));
                        }
                    }
                } else {
                    LOG_CLI((BSL_META_U(unit,
                                        "soc_sbx_caladan3_ppe_hd: Read failed (%d)\n"), rv));
                    break;
                }
            }
            soc_mem_unlock(unit, PP_CPDMm);
            if (parsed && !header) {
                soc_sbx_caladan3_ppe_hd_parsed(&rawdata[0], 64);
            }
            if (!header)
                LOG_CLI((BSL_META_U(unit,
                                    "\n")));
    }

    regval = 0;
    old_regval = 0;
    rv = READ_PP_HDR_COPY_CONTROL0r(unit, &old_regval);
    if (rv == SOC_E_NONE) {
        soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_MASKf, 0);
        soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_ENABLEf, 0);
        WRITE_PP_HDR_COPY_CONTROL0r(unit, regval);
        soc_reg_field_set(unit, PP_HDR_COPY_CONTROL0r, &regval, COPY_ENABLEf, 1);
        WRITE_PP_HDR_COPY_CONTROL0r(unit, regval);
        WRITE_PP_HDR_COPY_CONTROL0r(unit, old_regval);
    }

    return SOC_E_NONE;
}

/*
 * PPE CAM trace
 */

int 
soc_sbx_caladan3_ppe_cam_trace_set(int unit, int enable, int queue, int mode) 
{
    uint32 regval;

    if ((queue < 0) || (queue >= 128)) {
        return SOC_E_PARAM;
    }
    if ((queue > 63) && (queue < 128)) {
        queue = 0x80 | queue;
    }
    READ_PP_CAM_CAPTURE_CONTROLr(unit, &regval);
    soc_reg_field_set(unit, 
        PP_CAM_CAPTURE_CONTROLr, &regval, TRACE_ARMf, enable);
    soc_reg_field_set(unit, 
        PP_CAM_CAPTURE_CONTROLr, &regval, TRACE_CAPTUREDf, 1); /* to clear */
    soc_reg_field_set(unit, 
        PP_CAM_CAPTURE_CONTROLr, &regval, CAPTURE_TRACE_PKTf, 0); /* 2do mode */
    soc_reg_field_set(unit, 
        PP_CAM_CAPTURE_CONTROLr, &regval, SQUEUEf, queue); 
    WRITE_PP_CAM_CAPTURE_CONTROLr(unit, regval);

    if (enable) {
        LOG_CLI((BSL_META_U(unit,
                            "PPE Cam Capture enabled on queue 0x%x\n"), (queue & 0x7f)));
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "PPE Cam Capture disabled\n")));
    }
    return SOC_E_NONE; 
}

int 
soc_sbx_caladan3_ppe_cam_trace_get(void)
{
    return SOC_E_NONE; 
}

void 
soc_sbx_caladan3_ppe_cam_dump(int unit, int cam, int entry) {
    if (entry >= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX) {
        entry -= SOC_SBX_CALADAN3_PPE_CAM_PER_STAGE_ENTRIES_MAX;
    }
    LOG_CLI((BSL_META_U(unit,
                        "\nCam: %d entry: %d"), cam, entry ));
}

int 
soc_sbx_caladan3_ppe_cam_trace_dump(int unit)
{
    uint32 regval;
    uint32 status, hit, entry, cam;
    READ_PP_CAM_CAPTURE_CONTROLr(unit, &regval);
    status = soc_reg_field_get(unit, 
        PP_CAM_CAPTURE_CONTROLr, regval, TRACE_CAPTUREDf); 
    if (status) {
        for (cam=0; cam<(SOC_SBX_CALADAN3_PPE_CAM_NUM_STAGES_MAX/2); cam++) {
            soc_reg32_get(unit,soc_sbx_caladan3_ppe_cam_cap_regs[cam],
                          REG_PORT_ANY, 0, &regval);
            hit = soc_reg_field_get(unit, 
                     PP_CAM_CAPTURE_P0_STATUSr, regval, CAM0_HITf);
            entry = soc_reg_field_get(unit, 
                     PP_CAM_CAPTURE_P0_STATUSr, regval, CAM0_ENTRYf);
            if (hit) {
                soc_sbx_caladan3_ppe_cam_dump(unit, cam*2, entry);
            }
            hit = soc_reg_field_get(unit, 
                     PP_CAM_CAPTURE_P0_STATUSr, regval, CAM1_HITf);
            entry = soc_reg_field_get(unit, 
                     PP_CAM_CAPTURE_P0_STATUSr, regval, CAM1_ENTRYf);
            if (hit) {
                soc_sbx_caladan3_ppe_cam_dump(unit, (cam*2)+1, entry);
            }
        }
    } else {
        LOG_CLI((BSL_META_U(unit,
                            "\nNo Cam Trace")));
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));
    return SOC_E_NONE;
}

/*
 *  Routine: soc_sbx_caladan3_ppe_broad_shield_check_set_get
 *  Description:
 *     
 *  Inputs:
 *      type:   type, see "Header checker exceptions" in ppe.h
 *      enable: TRUE for enable, FALSE for disable
 *      set:    TRUE to set, FALSE to get
 *  Outputs: 
 *      one of SOC_E*
 */
static int
soc_sbx_caladan3_ppe_broad_shield_check_set_get(int unit, int type, uint8 *enable, uint8 set)
{
    uint32 status = SOC_E_NONE;
    uint32 uRegValue = 0;
    soc_field_t field;
    soc_reg_t reg;

    if (enable == NULL) {
        return SOC_E_PARAM;
    }

    switch (type) {
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_INV_PPP_ADDR_CTL:
            reg = PP_HC_CONFIG0r;
            field = INV_PPP_ADDR_CTLf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_INV_PPP_PID:
            reg = PP_HC_CONFIG0r;
            field = INV_PPP_PIDf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN0_FFF:
            reg = PP_HC_CONFIG0r;
            field = ENET_VLAN0_EQ_3FFf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN1_FFF:
            reg = PP_HC_CONFIG0r;
            field = ENET_VLAN1_EQ_3FFf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN2_FFF:
            reg = PP_HC_CONFIG0r;
            field = ENET_VLAN2_EQ_3FFf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_SMAC_EQ_DMAC:
            reg = PP_HC_CONFIG0r;
            field = ENET_SMAC_EQ_DMACf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_GRE_INV_RES0:
            reg = PP_HC_CONFIG0r;
            field = INV_GRE_RES0f;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_MAC_ZERO:
            reg = PP_HC_CONFIG0r;
            field = ENET_MAC_EQ_ZEROf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_TYPE_VALUE:
            reg = PP_HC_CONFIG0r;
            field = ENET_TYPE_VALUEf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_SMAC_MCAST:
            reg = PP_HC_CONFIG0r;
            field = ENET_SMAC_MCASTf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_RUNT_PKT:
            reg = PP_HC_CONFIG0r;
            field = IPV4_RUNT_PKTf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_OPTIONS:
            reg = PP_HC_CONFIG0r;
            field = IPV4_OPTIONSf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_CHECKSUM:
            reg = PP_HC_CONFIG0r;
            field = INV_IPV4_CHECKSUMf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_VER:
            reg = PP_HC_CONFIG0r;
            field = INV_IPV4_VERf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_RUNT_HDR:
            reg = PP_HC_CONFIG0r;
            field = IPV4_RUNT_HDRf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_LEN_ERR:
            reg = PP_HC_CONFIG0r;
            field = IPV4_LEN_ERRf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_PKT_LEN_ERR:
            reg = PP_HC_CONFIG0r;
            field = IPV4_PKT_LEN_ERRf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_SA:
            reg = PP_HC_CONFIG0r;
            field = IPV4_INV_SAf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_DA:
            reg = PP_HC_CONFIG0r;
            field = IPV4_INV_DAf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_SA_EQ_DA:
            reg = PP_HC_CONFIG0r;
            field = IPV4_SA_EQ_DAf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_LOOPBACK:
            reg = PP_HC_CONFIG0r;
            field = IPV4_SA_OR_DA_LPBKf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_MARTIAN_ADDR:
            reg = PP_HC_CONFIG0r;
            field = IPV4_SA_OR_DA_MARTIANf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER0:
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER1:
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER2:
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_USR_FILTER3:
            reg = PP_HC_CONFIG0r;
            field = IPV4_SA_OR_DA_MATCHf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_ICMP_FRAG:
            reg = PP_HC_CONFIG0r;
            field = IPV4_ICMP_FRAGf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_RUNT:
            reg = PP_HC_CONFIG0r;
            field = IPV6_RUNT_PKTf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_VER:
            reg = PP_HC_CONFIG0r;
            field = INV_IPV6_VERf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_PKT_LEN_ERR:
            reg = PP_HC_CONFIG0r;
            field = IPV6_PKT_LEN_ERRf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_SA:
            reg = PP_HC_CONFIG0r;
            field = IPV6_INV_SAf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_DA:
            reg = PP_HC_CONFIG0r;
            field = IPV6_INV_DAf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_SA_EQ_DA:
            reg = PP_HC_CONFIG0r;
            field = IPV6_SA_EQ_DAf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_LOOPBACK:
            reg = PP_HC_CONFIG0r;
            field = IPV6_SA_OR_DA_LPBKf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_USR_FILTER0:
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_USR_FILTER1:
            reg = PP_HC_CONFIG0r;
            field = IPV6_SA_OR_DA_MATCHf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SP_EQ_DP:
            reg = PP_HC_CONFIG1r;
            field = TCP_SP_EQ_DPf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_NULL_SCAN:
            reg = PP_HC_CONFIG1r;
            field = TCP_NULL_SCANf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_XMAS_SCAN:
            reg = PP_HC_CONFIG1r;
            field = TCP_XMAS_SCANf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SYN_FIN:
            reg = PP_HC_CONFIG1r;
            field = TCP_SYN_FINf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_TINY_FRAG:
            reg = PP_HC_CONFIG1r;
            field = TCP_TINY_FRAGf;
            break;
        case SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SYN_SPORT_LT_1024:
            reg = PP_HC_CONFIG1r;
            field = SYN_SPORT_LT_1024f;
            break;
        default:
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unsupported caladan3 ppe checker type %d\n"), type));
            return SOC_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &uRegValue));
    
    if (set) {
        /* set */
      soc_reg_field_set(unit, reg, &uRegValue, field, (*enable)?0:1);
        if (field == TCP_SP_EQ_DPf) {
            field = UDP_SP_EQ_DPf;
            soc_reg_field_set(unit, reg, &uRegValue, field, (*enable)?0:1);
        }
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, uRegValue));
    } else {
        /* get */
        *enable = soc_reg_field_get(unit, reg, uRegValue, field)?FALSE:TRUE;
    }

    return status;
}

int soc_sbx_caladan3_ppe_broad_shield_check_set(int unit, int type, uint8 enable)
{
    return soc_sbx_caladan3_ppe_broad_shield_check_set_get(unit, type, &enable, TRUE);
}

int soc_sbx_caladan3_ppe_broad_shield_check_get(int unit, int type, uint8 *enable)
{
    return soc_sbx_caladan3_ppe_broad_shield_check_set_get(unit, type, enable, FALSE);
}

#endif /* CALADAN3_SUPPORT */

