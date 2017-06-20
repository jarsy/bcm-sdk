/*
 * $Id: policer.c,v 1.38.6.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/*
 * All Rights Reserved.$
 *
 * Module: Service Meter(global meter)
 *
 * Purpose:
 *     These routines manage the handling of service meter(global meter)
 *      functionality
 *
 *
 */
#include <shared/bsl.h>

#include <shared/idxres_afl.h>

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/scache.h>

#include <bcm/policer.h>
#include <bcm/module.h>
#include <bcm_int/esw/policer.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/vlan.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/switch.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/xgs4.h>

#if defined(BCM_GREYHOUND2_SUPPORT)
#include <bcm_int/esw/greyhound2.h>
#endif /* BCM_GREYHOUND2_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_VERSION_1_1                SOC_SCACHE_VERSION(1,1)
#define BCM_WB_VERSION_1_2                SOC_SCACHE_VERSION(1,2)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_2
#endif

#define _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER _BCM_XGS_METER_FLAG_FP_POLICER


#define UNKNOWN_PKT                 0x00
#define CONTROL_PKT                 0x01
#define OAM_PKT                     0x02
#define BFD_PKT                     0x03
#define BPDU_PKT                    0x04
#define ICNM_PKT                    0x05
#define PKT_IS_1588                 0x06
#define KNOWN_L2UC_PKT              0x07

#define UNKNOWN_L2UC_PKT            0x08
#define KNOWN_L2MC_PKT              0x09
#define UNKNOWN_L2MC_PKT            0x0a
#define L2BC_PKT                    0x0b
#define KNOWN_L3UC_PKT              0x0c
#define UNKNOWN_L3UC_PKT            0x0d
#define KNOWN_IPMC_PKT              0x0e
#define UNKNOWN_IPMC_PKT            0x0f

#define KNOWN_MPLS_L2_PKT           0x10
#define UNKNOWN_MPLS_PKT            0x11
#define KNOWN_MPLS_L3_PKT           0x12
#define KNOWN_MPLS_PKT              0x13
#define KNOWN_MPLS_MULTICAST_PKT    0x14
#define KNOWN_MIM_PKT               0x15
#define UNKNOWN_MIM_PKT             0x16
#define KNOWN_TRILL_PKT             0x17

#define UNKNOWN_TRILL_PKT           0x18
#define KNOWN_NIV_PKT               0x19
#define UNKNOWN_NIV_PKT             0x1a
#define	KNOWN_L2GRE_PKT             0x1b
#define KNOWN_VDL2_PKT              0x1c
#define KNOWN_FCOE_PKT              0x1d
#define UNKNOWN_FCOE_PKT            0x1e
#define SAT_DN_SAMP_RX              0x1f
#define FP_RESOLUTION_MAX           0x20  /* Max value */

#define METER_REFRESH_INTERVAL      8     /* 7.8125usec */
#define MIN_BURST_MULTIPLE          2

#define CONVERT_SECOND_TO_MICROSECOND 1000000

static soc_reg_t _pkt_attr_sel_key_reg[4] = {
                ING_SVM_PKT_ATTR_SELECTOR_KEY_0r,
                ING_SVM_PKT_ATTR_SELECTOR_KEY_1r,
                ING_SVM_PKT_ATTR_SELECTOR_KEY_2r,
                ING_SVM_PKT_ATTR_SELECTOR_KEY_3r
            };

STATIC sal_mutex_t global_meter_mutex[BCM_MAX_NUM_UNITS] = {NULL};

static uint8 kt_pkt_res[FP_RESOLUTION_MAX] =
                              {
                                   0x0, 0x1, 0x0, 0x0, 0x2, 0x0, 0x0, 0x4,
                                   0x5, 0x8, 0x9, 0x3, 0xa, 0xb, 0x7, 0x6,
                                   0xe, 0xf, 0xd, 0xc, 0x12, 0x10, 0x11, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0
                               };
#ifdef BCM_TRIUMPH3_SUPPORT
static uint8 tr3_pkt_res[FP_RESOLUTION_MAX] =
                          {
                               0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x8,
                               0x9, 0xa, 0xb, 0xc, 0x10, 0x11, 0x12, 0x13,
                               0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x20, 0x21, 0x28,
                               0x29, 0x30, 0x31, 0, 0, 0, 0, 0
                          };
#endif
#ifdef BCM_KATANA2_SUPPORT
static uint8 kt2_pkt_res[FP_RESOLUTION_MAX] =
                          {
                               0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,   0x8,
                               0x9,  0xa,  0xb,  0xc,  0x10, 0x11, 0x12,  0x13,
                               0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x20, 0x21,  0x28,
                               0x29, 0x30, 0x31, 0,    0,    0,    0,     0
                          };
#endif
#ifdef BCM_SABER2_SUPPORT
static uint8 sb2_pkt_res[FP_RESOLUTION_MAX] =
                          {
                               0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,   0x8,
                               0x9,  0xa,  0xb,  0xc,  0x10, 0x11, 0x12,  0x13,
                               0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x20, 0x21,  0x28,
                               0x29, 0x30, 0x31, 0,    0,    0,    0,     0x32
                          };
#endif
#ifdef BCM_APACHE_SUPPORT
static uint8 ap_pkt_res[FP_RESOLUTION_MAX] =
                          {
                               0x0,  0x1,  0x2,  0x3,  0x4,  0x5,  0x6,   0x8,
                               0x9,  0xa,  0xb,  0xc,  0x10, 0x11, 0x12,  0x13,
                               0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x20, 0x21,  0x28,
                               0x29, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,  0x00
                          };
#endif

static uint32 refresh_max[] =  { 100, 125, 150, 175,
                                 200, 225, 250, 275,
                                 300, 325, 350, 375,
                                 400, 425, 450, 475,
                                 500, 525, 550, 575,
                                 600, 625, 650, 675,
                                 700, 725, 750, 775,
                                 800, 0 };
/*
 * Global meter policer control data, one per device.
*/
static _global_meter_policer_control_t
                    **global_meter_policer_bookkeep[BCM_MAX_NUM_UNITS];
/*
 * Global meter horizontal alloc management data, one per device.
 */
static _global_meter_horizontal_alloc_t
              *global_meter_hz_alloc_bookkeep[BCM_MAX_NUM_UNITS];

static bcm_policer_svc_meter_bookkeep_mode_t
      global_meter_offset_mode[BCM_MAX_NUM_UNITS]\
                              [BCM_POLICER_SVC_METER_MAX_MODE];

static _bcm_policer_global_meter_control_t
      *bcm_esw_svm_control[BCM_MAX_NUM_UNITS];

#define BCM_SVM_DEV_ATTR(unit)      (&(bcm_esw_svm_control[(unit)]->svm_dev_attr_info))
#define BCM_SVM_SOURCE(unit, index) (bcm_esw_svm_control[(unit)]->svm_source[(index)])

static bcm_policer_global_meter_action_bookkeep_t
          *global_meter_action_bookkeep[BCM_MAX_NUM_UNITS];

/* Global meter module status - initialised or not initilised */
static bcm_policer_global_meter_init_status_t
                        global_meter_status[BCM_MAX_NUM_UNITS] = {{0}};
/*
 * Global meter resource lock
 */
#define GLOBAL_METER_LOCK(unit) \
        sal_mutex_take(global_meter_mutex[unit], sal_mutex_FOREVER);

#define GLOBAL_METER_UNLOCK(unit) \
        sal_mutex_give(global_meter_mutex[unit]);

/* Handles for indexed allocation - one for meter action id allocation
   and the other for policer id alloaction  */
shr_aidxres_list_handle_t
          meter_action_list_handle[BCM_UNITS_MAX] = {NULL};
shr_aidxres_list_handle_t  meter_alloc_list_handle[BCM_UNITS_MAX]\
          [BCM_POLICER_GLOBAL_METER_MAX_POOL * \
          BCM_POLICER_GLOBAL_METER_MAX_BANKS_PER_POOL] = {{NULL}};

#define CHECK_GLOBAL_METER_INIT(unit)                           \
        if (!global_meter_status[unit].initialised)             \
            return BCM_E_INIT

/* Global Meter platform-specific information */
STATIC const bcmi_global_meter_dev_info_t
    *bcmi_global_meter_dev_info[BCM_UNITS_MAX];
STATIC int bcmi_global_meter_dev_info_initialized = FALSE;

#define CHECK_GLOBAL_METER_PF_INIT(_u_)                        \
    do {                                                       \
        if (!soc_feature(unit, soc_feature_global_meter_v2)) { \
            return BCM_E_UNAVAIL;                              \
        }                                                      \
        if (!bcmi_global_meter_dev_info[(_u_)]) {              \
            return BCM_E_INIT;                                 \
        }                                                      \
    } while (0)

/*
 * Function:
 *      bcmi_global_meter_dev_info_init
 * Purpose:
 *      register Global Meter platform specific information & functions
 * Parameters:
 *      unit - (IN) Unit number
 * Returns:
 *      BCM_E_xxx
 */
STATIC int
bcmi_global_meter_dev_info_init(int unit)
{
    bcm_error_t rv = BCM_E_UNAVAIL;

    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "bcmi_global_meter_dev_info_init\n")));

#if defined(BCM_GREYHOUND2_SUPPORT)
    if (SOC_IS_GREYHOUND2(unit)) {
        rv = bcmi_gh2_global_meter_dev_info_init(unit,
                &(bcmi_global_meter_dev_info[unit]));
    }
    else
#endif /* BCM_GREYHOUND2_SUPPORT */
    if (BCM_FAILURE(rv)) {
        /* No chip specific information initialized */
        LOG_WARN(BSL_LS_BCM_SWITCH,
                  (BSL_META_U(unit,
                              "Global Meter not supported:\n"
                              "dev_info_init failed.\n")));
    }
    return rv;
}

STATIC
int _bcm_esw_svc_meter_offset_mode_id_check (uint32 unit, uint32 flags,
                bcm_policer_group_mode_type_t type, uint32 total_policers,
                uint32 num_selectors,
                bcm_policer_group_mode_attr_selector_t * attr_selectors,
                uint32 *mode_id);

typedef int (*_bcm_esw_svm_mem_entry_traverse_cb)(int unit,
    soc_mem_t mem, uint32 index, void *entry, void *user_data, uint32 *dirty);

/*
 * Function:
 *      check_global_meter_init
 * Purpose:
 *     To check whether the global meter module is initialized or not
 * Parameters:
 *     Unit                  - (IN) unit number
 * Returns:
 *     BCM_E_INIT
 */
int _check_global_meter_init(int unit)
{
    if (!global_meter_status[unit].initialised) {
            return BCM_E_INIT;
    } else {
        return BCM_E_NONE;
    }
}

/*
 * Function:
 *      convert_to_bitmask
 * Purpose:
 *
 * Parameters:
 *     num - number to be converted to bistmask
 * Returns:
 *     bit mask for the number
 */
static int convert_to_bitmask(int num)
{
    int bit_mask = 0;
    num = num & 0xf;
    while (num > 0) {
        bit_mask = bit_mask | (1 << (num-1));
        num--;
    }
    return bit_mask;
}

/*
 * Function:
 *      convert_to_mask
 * Purpose:
 *     Creates a mask with all the bit positon other then the positon
 *     represented by num
 * Parameters:
 *     num -  number
 * Returns:
 *     BCM_E_XXX
 */
static int convert_to_mask(int num)
{
    int bit_mask = 0;
    num = num & 0xf;
    bit_mask =  ~(1 << num);
    return bit_mask;
}

/*
 * Function:
 *     _bcm_attr_selectors_copy_to_wb_attr_selectors
 * Purpose:
 *     Copy the content of attr_selectors from
 *     bcm_policer_group_mode_attr_selector_t to
 *     bcm_policer_group_mode_attr_selector_wb_t
 * Parameters:
 *     attr_selectors (IN) -  bcm API attr_selectors
 *     attr_selectors_wb (OUT)-  attr_selectors for KT2/TR3/FB4 WB structure
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_attr_selectors_copy_to_wb_attr_selectors(
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    bcm_policer_group_mode_attr_selector_wb_t *attr_selectors_wb)
{
    if (NULL == attr_selectors || NULL == attr_selectors_wb) {
        return BCM_E_PARAM;
    }
    attr_selectors_wb->flags =
        attr_selectors->flags;
    attr_selectors_wb->policer_offset =
        attr_selectors->policer_offset;
    attr_selectors_wb->attr =
        attr_selectors->attr;
    attr_selectors_wb->attr_value =
        attr_selectors->attr_value;
    attr_selectors_wb->udf_id =
        attr_selectors->udf_id;
    attr_selectors_wb->offset =
        attr_selectors->offset;
    attr_selectors_wb->width =
        attr_selectors->width;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_wb_attr_selectors_copy_to_attr_selectors
 * Purpose:
 *     Copy the content of attr_selectors from
 *     bcm_policer_group_mode_attr_selector_wb_t to
 *     bcm_policer_group_mode_attr_selector_t
 * Parameters:
 *     attr_selectors_wb (IN) - attr_selectors for KT2/TR3/FB4 WB structure
 *     attr_selectors (OUT)- bcm API attr_selectors
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_wb_attr_selectors_copy_to_attr_selectors(
    bcm_policer_group_mode_attr_selector_wb_t *attr_selectors_wb,
    bcm_policer_group_mode_attr_selector_t *attr_selectors)
{
    if (NULL == attr_selectors_wb || NULL == attr_selectors) {
        return BCM_E_PARAM;
    }
    attr_selectors->flags =
        attr_selectors_wb->flags;
    attr_selectors->policer_offset =
        attr_selectors_wb->policer_offset;
    attr_selectors->attr =
        attr_selectors_wb->attr;
    attr_selectors->attr_value =
        attr_selectors_wb->attr_value;
    attr_selectors->udf_id =
        attr_selectors_wb->udf_id;
    attr_selectors->offset =
        attr_selectors_wb->offset;
    attr_selectors->width =
        attr_selectors_wb->width;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_attr_selectors_wb_attr_selectors_cmp
 * Purpose:
 *     compare the content of attr_selectors between
 *     bcm_policer_group_mode_attr_selector_wb_t and
 *     bcm_policer_group_mode_attr_selector_t
 * Parameters:
 *     attr_selectors (IN) -  bcm API attr_selectors
 *     attr_selectors_wb (IN) -  attr_selectors for KT2/TR3/FB4 WB structure
 *     is_same (OUT) -  return if contents are all the same or not
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_attr_selectors_wb_attr_selectors_cmp(
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    bcm_policer_group_mode_attr_selector_wb_t *attr_selectors_wb,
    int *is_same)
{
    if (NULL == attr_selectors_wb || NULL == attr_selectors ||
        NULL == is_same) {
        return BCM_E_PARAM;
    }

    *is_same = TRUE;
    if ((attr_selectors->flags != attr_selectors_wb->flags) ||
        (attr_selectors->policer_offset != attr_selectors_wb->policer_offset) ||
        (attr_selectors->attr != attr_selectors_wb->attr) ||
        (attr_selectors->attr_value != attr_selectors_wb->attr_value) ||
        (attr_selectors->udf_id != attr_selectors_wb->udf_id) ||
        (attr_selectors->offset != attr_selectors_wb->offset) ||
        (attr_selectors->width != attr_selectors_wb->width)) {
        *is_same = FALSE;
        return BCM_E_NONE;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      get_max_banks_in_a_pool
 * Purpose:
 *     Returns the number of banks per pool
 * Parameters:
 *     unit - unit number
 * Returns:
 *     num_banks
 */
STATIC
int get_max_banks_in_a_pool(int unit)
{
    return BCM_SVM_DEV_ATTR(unit)->banks_per_pool;
}

/*
 * Function:
 *      _bcm_policer_flow_info_t_init
 * Purpose:
 *     Init bcm_policer_flow_info_t
 * Parameters:
 *     flow_info : pointer of type _bcm_policer_flow_info_t
 * Returns:
 *     BCM_E_XXX
 */
STATIC
int _bcm_policer_flow_info_t_init(_bcm_policer_flow_info_t *flow_info)
{
    flow_info->flow_type = bcmPolicerFlowTypeNormal;
    flow_info->skip_pool = -1;
    flow_info->skip_bank = -1;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_update_udf_selector_keys
 * Purpose:
 *     Internal function for updating udf selector keys
 * Parameters:
 *     Unit                  - (IN) unit number
 *     pkt_attr_selector_key - (IN) packet attribute selector key
 *     udf_pkt_attr          - (IN) UDF packet attributes
 *     total_udf_bits        - (OUT) Total number of udf bits used
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_update_udf_selector_keys(
    int                        unit,
    soc_reg_t                  pkt_attr_selector_key,
    udf_pkt_attr_selectors_t   *udf_pkt_attr,
    uint32                     *total_udf_bits)
{
    uint64    pkt_attr_selector_key_value;
    uint32    udf_valid_bits = 0;
    uint8     key_bit_selector_position = 0;
    uint32    index = 0;

    COMPILER_64_ZERO(pkt_attr_selector_key_value);
    (*total_udf_bits) = 0;
    if (!(pkt_attr_selector_key >= ING_SVM_PKT_ATTR_SELECTOR_KEY_0r) &&
         (pkt_attr_selector_key <= ING_SVM_PKT_ATTR_SELECTOR_KEY_3r)) {
      /* Valid mem value go ahead for setting
                       ING_FLEX_CTR_PKT_ATTR_SELECTOR_KEY_?r: */
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Invalid memory for packet attribute selector \n")));
        return BCM_E_PARAM;
    }

    /* First Get complete value of ING_SVM_PKT_ATTR_SELECTOR_KEY_?r value */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit,
            pkt_attr_selector_key,
            REG_PORT_ANY,
            0,
            &pkt_attr_selector_key_value));

    /* Next set field value for
                    ING_SVM_PKT_ATTR_SELECTOR_KEY_?r:SELECTOR_KEY field*/
    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USE_UDF_KEYf,
            1);
    udf_valid_bits |= SVC_METER_UDF1_VALID;
    udf_valid_bits |= SVC_METER_UDF2_VALID;

    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USER_SPECIFIED_UDF_VALIDf,
            udf_valid_bits);

    /* Now update selector keys
     * In udf based offset generation, bit-0 is for drop.
     * bit-1 to bit-32 is used for 2 16-bits udf-chunks
     */
    if (udf_pkt_attr->drop == 1) {
        BCM_IF_ERROR_RETURN(
            _bcm_policer_svc_meter_update_selector_keys_enable_fields(
                        unit,
                        pkt_attr_selector_key,
                        &pkt_attr_selector_key_value,
                        0, /* Start bit of drop */
                        1, /* width of drop */
                        &key_bit_selector_position));
    }
    for (index = 0; index < udf_pkt_attr->total_subdiv; index++) {
        BCM_IF_ERROR_RETURN(
            _bcm_policer_svc_meter_update_selector_keys_enable_fields(
                        unit,
                        pkt_attr_selector_key,
                        &pkt_attr_selector_key_value,
                        udf_pkt_attr->udf_subdiv[index].offset + 1, /* +1 to adjust drop bit */
                        udf_pkt_attr->udf_subdiv[index].width,
                        &key_bit_selector_position));
    }

    /* Finally set value for ING_SVM_PKT_ATTR_SELECTOR_KEY_?r */
    SOC_IF_ERROR_RETURN(soc_reg_set(unit,
            pkt_attr_selector_key,
            REG_PORT_ANY,
            0,
            pkt_attr_selector_key_value));
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_esw_policer_get_offset_table_policer_count
 * Purpose:
 *    set group mode and number of policers in offset table at
 *              offsets 254 and 255 - for use during warmboot
 *
 * Parameters:
 *     Unit                  - (IN) unit number
 *     offset_mode           - (IN) Offset mode
 *     mode                  - (OUT) Policer group mode
 *     npolicers             - (OUT) Total number of policer created
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_esw_policer_get_offset_table_policer_count(
    int                                unit,
    bcm_policer_svc_meter_mode_t       offset_mode,
    bcm_policer_group_mode_t           *mode,
    int                                *npolicers)
{
    uint32            offset_table_entry = 0;
    int               max_index = 0;
    int               meter_enable;

    max_index = BCM_POLICER_SVC_METER_MAX_OFFSET;
    /* First Get complete value of ING_SVM_METER_OFFSET_TABLE_?m value */
    SOC_IF_ERROR_RETURN(soc_mem_read(unit,
         SVM_OFFSET_TABLEm, MEM_BLOCK_ANY,
         ((offset_mode << BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) |
                                                        (max_index  - 1)),
         &offset_table_entry));

    /* Read meter_enable first, if meter_enable=1, offsets 255,254 have
        not stored npolicers and group_mode.
        Also, Warmboot vesion BCM_WB_VERSION_1_1 onwards, npolices and group_mode
        are stored in scache. */
    soc_mem_field_get( unit, SVM_OFFSET_TABLEm, &offset_table_entry,
                                            METER_ENABLEf, (uint32 *)&meter_enable);
    if (meter_enable == 1)  {
        *npolicers = 0;
        *mode = 0;
        return BCM_E_NONE;
    }

    soc_mem_field_get( unit, SVM_OFFSET_TABLEm, &offset_table_entry,
                                            OFFSETf, (uint32 *)npolicers);

    SOC_IF_ERROR_RETURN(soc_mem_read(unit,
         SVM_OFFSET_TABLEm, MEM_BLOCK_ANY,
         ((offset_mode << BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) |
                                                        (max_index  - 2)),
         &offset_table_entry));

    /* Read meter_enable first, if meter_enable=1, offsets 255,254 have
        not stored npolicers and group_mode.
        Also, Warmboot vesion BCM_WB_VERSION_1_1 onwards, npolices and group_mode
        are stored in scache. */
    soc_mem_field_get( unit, SVM_OFFSET_TABLEm, &offset_table_entry,
                                            METER_ENABLEf, (uint32 *)&meter_enable);
    if (meter_enable == 1)  {
        *npolicers = 0;
        *mode = 0;
        return BCM_E_NONE;
    }
    soc_mem_field_get( unit, SVM_OFFSET_TABLEm, &offset_table_entry,
                                                 OFFSETf, (uint32 *)mode);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_update_offset_table
 * Purpose:
 *     Internal function to update the SVM offset table
 * Parameters:
 *     Unit                  - (IN) unit number
 *     offset_table_mem      - (IN) Offset table memory
 *     svc_meter_mode        - (IN) Meter offset mode
 *     offset_map            - (IN) Offset table entry map
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_update_offset_table(
    int                                unit,
    soc_mem_t                          offset_table_mem,
    bcm_policer_svc_meter_mode_t       svc_meter_mode,
    offset_table_entry_t  *offset_map)
{
    uint32            offset_table_entry = 0;
    uint32            index = 0;
    uint32            zero = 0;
    uint32            meter_enable = 0;
    uint32            offset_value = 0, pool = 0;
    if (offset_table_mem != SVM_OFFSET_TABLEm) {
        /* Valid mem value go ahead for setting SVM_OFFSET_TABLEm */
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid table specified \n")));
        return BCM_E_PARAM;
    }
    if(NULL == offset_map) {
        /* Clear all the entries */
        for (; index < BCM_POLICER_SVC_METER_MAX_OFFSET; index++) {
            /* First Get complete value of ING_SVM_METER_OFFSET_TABLE_?m value */
                SOC_IF_ERROR_RETURN(soc_mem_read(unit,
                    offset_table_mem,
                    MEM_BLOCK_ANY,
                    ((svc_meter_mode <<
                        BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) | index),
                    &offset_table_entry));
            /* Set POOL_OFFSETf */
            soc_mem_field_set(
                    unit,
                    offset_table_mem,
                    &offset_table_entry,
                    POOL_OFFSETf,
                    &zero);
            /* Set OFFSETf=zero */
            soc_mem_field_set(
                    unit,
                    offset_table_mem,
                    &offset_table_entry,
                    OFFSETf,
                    &zero);
            /* Set METER_ENABLEf=zero */
            soc_mem_field_set(
                    unit,
                    offset_table_mem,
                    &offset_table_entry,
                    METER_ENABLEf,
                    &zero);
            /* Finally set value for ING_SVM_METER_OFFSET_TABLE_?m?r */
            soc_mem_write(unit,
                    offset_table_mem,
                    MEM_BLOCK_ALL,
                    ((svc_meter_mode <<
                     BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) | index),
                    &offset_table_entry);

        }
    } else {
        for (index = 0; index < 256; index++) {
            offset_value = offset_map[index].offset;
            meter_enable = offset_map[index].meter_enable;
            pool = offset_map[index].pool;
            /* First Get complete value of SVM_OFFSET_TABLE_?m value */
            SOC_IF_ERROR_RETURN(soc_mem_read(unit,
                offset_table_mem,
                MEM_BLOCK_ANY,
                ((svc_meter_mode <<
                 BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) | index),
                &offset_table_entry));
            /* Set POOL_OFFSETf */
            soc_mem_field_set(
                unit,
                offset_table_mem,
                &offset_table_entry,
                POOL_OFFSETf,
                &pool);
            /* Set OFFSETf */
            soc_mem_field_set(
                unit,
                offset_table_mem,
                &offset_table_entry,
                OFFSETf,
                &offset_value);
           /* Set METER_ENABLEf */
            soc_mem_field_set(
                unit,
                offset_table_mem,
                &offset_table_entry,
                METER_ENABLEf,
                &meter_enable);
            /* Finally set value for ING_SVM_OFFSET_TABLE_?m?r */
            soc_mem_write(unit,
                offset_table_mem,
                MEM_BLOCK_ALL,
                ((svc_meter_mode <<
                   BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) | index),
                &offset_table_entry);
        }
    }
    if(svc_meter_mode == 0) {
        index = 0;
        meter_enable  = 1;
        /*  SVM_OFFSET_TABLE_?m value */
        SOC_IF_ERROR_RETURN(soc_mem_read(unit,
            offset_table_mem,
            MEM_BLOCK_ANY,
            ((svc_meter_mode << BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE)),
            &offset_table_entry));
        /* Set OFFSETf */
        soc_mem_field_set(
            unit,
            offset_table_mem,
            &offset_table_entry,
            OFFSETf,
            &zero);
       /* Set METER_ENABLEf */
        soc_mem_field_set(
            unit,
            offset_table_mem,
            &offset_table_entry,
            METER_ENABLEf,
            &meter_enable);
        /* Finally set value for ING_SVM_OFFSET_TABLE_?m?r */
        soc_mem_write(unit,
            offset_table_mem,
            MEM_BLOCK_ALL,
            ((svc_meter_mode << BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) | index),
            &offset_table_entry);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_update_selector_keys_enable_fields
 * Purpose:
 *     Internal function to set selector key enabled fields
 * Parameters:
 *     Unit                  - (IN) unit number
 *     pkt_attr_selector_key - (IN) Pkt attribute selector key
 *     pkt_attr_selector_key_value - (IN) Value of selector key
 *     pkt_attr_bit_position - (IN) Packet attribute bit position
 *     pkt_attr_total_bits   - (IN) total number of packet attribute bits
 *     current_bit_selector_position - (IN) Current bit position
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_update_selector_keys_enable_fields(
    int         unit,
    soc_reg_t   pkt_attr_selector_key,
    uint64      *pkt_attr_selector_key_value,
    uint32      pkt_attr_bit_position,
    uint32      pkt_attr_total_bits,
    uint8       *current_bit_selector_position)
{
    uint32    index = 0;
    uint32    selector_x_en_field_name[8]= {
        SELECTOR_0_ENf,
        SELECTOR_1_ENf,
        SELECTOR_2_ENf,
        SELECTOR_3_ENf,
        SELECTOR_4_ENf,
        SELECTOR_5_ENf,
        SELECTOR_6_ENf,
        SELECTOR_7_ENf,
    };
    uint32    selector_for_bit_x_field_name[8]={
        SELECTOR_FOR_BIT_0f,
        SELECTOR_FOR_BIT_1f,
        SELECTOR_FOR_BIT_2f,
        SELECTOR_FOR_BIT_3f,
        SELECTOR_FOR_BIT_4f,
        SELECTOR_FOR_BIT_5f,
        SELECTOR_FOR_BIT_6f,
        SELECTOR_FOR_BIT_7f,
    };

    if ((*current_bit_selector_position) +
        pkt_attr_total_bits > BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Number of slector bits exceeds max allowed \n")));
        return BCM_E_INTERNAL;
    }

    for (index = 0; index < pkt_attr_total_bits; index++) {
        soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            pkt_attr_selector_key_value,
            selector_x_en_field_name[*current_bit_selector_position],
            1);
        soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            pkt_attr_selector_key_value,
            selector_for_bit_x_field_name[*current_bit_selector_position],
            pkt_attr_bit_position + index);
        (*current_bit_selector_position) += 1;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_policer_svc_meter_dec_mode_reference_count
 * Purpose:
 *     Internal function to decrement meter offset mode usage count
 * Parameters:
 *     Unit                  - (IN) unit number
 *     svc_meter_mode        - (IN) Meter offset mode
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t bcm_policer_svc_meter_dec_mode_reference_count(
    uint32 unit,
    bcm_policer_svc_meter_mode_t svc_meter_mode)
{
    int rv = BCM_E_NONE;
    if (!((svc_meter_mode >= 1) && (svc_meter_mode <= 3))) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Wrong offset mode specified \n")));
        return BCM_E_PARAM;
    }
    if ( global_meter_offset_mode[unit][svc_meter_mode].reference_count > 0) {
       global_meter_offset_mode[unit][svc_meter_mode].reference_count--;
    }
    if (global_meter_offset_mode[unit][svc_meter_mode].type != -1) {
        /* Deletion of the mode is done through
           bcm_policer_group_mode_id_destroy */
        return rv;
    }
    if (global_meter_offset_mode[unit][svc_meter_mode].reference_count == 0) {
        rv = _bcm_esw_policer_svc_meter_delete_mode(unit, svc_meter_mode);
    }
    return rv;
}

/*
 * Function:
 *      bcm_policer_svc_meter_inc_mode_reference_count
 * Purpose:
 *     Internal function to increment meter offset mode usage count
 * Parameters:
 *     Unit                  - (IN) unit number
 *     svc_meter_mode        - (IN) Meter offset mode
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t bcm_policer_svc_meter_inc_mode_reference_count(
    uint32 unit,
    bcm_policer_svc_meter_mode_t svc_meter_mode)
{
    if (!((svc_meter_mode >= 1) && (svc_meter_mode <= 3))) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Wrong offset mode specified \n")));
        return BCM_E_PARAM;
    }
    global_meter_offset_mode[unit][svc_meter_mode].reference_count++;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_reserve_mode
 * Purpose:
 *     Internal function to allocate/reserve meter offset mode
 * Parameters:
 *     Unit                  - (IN) unit number
 *     svc_meter_mode        - (IN) Meter offset mode
 *     total_bits            - (IN) Total number of bits used
 *     group_mode            - (IN) Policer group mode
 *     meter_attr            - (IN) Meter attributes
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_reserve_mode(
    uint32                               unit,
    bcm_policer_svc_meter_mode_t         svc_meter_mode,
    bcm_policer_group_mode_t             group_mode,
    bcm_policer_svc_meter_attr_t         *meter_attr)
{
    if (!((svc_meter_mode >= 1) && (svc_meter_mode <= 3))) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Wrong offset mode specified \n")));
        return BCM_E_PARAM;
    }
    global_meter_offset_mode[unit][svc_meter_mode].used = 1;
    global_meter_offset_mode[unit][svc_meter_mode].group_mode = group_mode;
    global_meter_offset_mode[unit][svc_meter_mode].type = -1;
    global_meter_offset_mode[unit][svc_meter_mode].meter_attr = *meter_attr;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_unreserve_mode
 * Purpose:
 *     Internal function to unreserve meter offset mode
 * Parameters:
 *     Unit                  - (IN) unit number
 *     svc_meter_mode        - (IN) Meter offset mode
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_unreserve_mode(
    uint32                                  unit,
    bcm_policer_svc_meter_mode_t            svc_meter_mode)
{
    uint64    pkt_attr_selector_key_value;
    bcm_policer_svc_meter_mode_type_t       mode_type_v;

    COMPILER_64_ZERO(pkt_attr_selector_key_value);
    if (!((svc_meter_mode >= 1) && (svc_meter_mode <= 3))) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Wrong offset mode specified \n")));
        return BCM_E_PARAM;
    }
    if ( global_meter_offset_mode[unit][svc_meter_mode].used == 0) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Wrong offset mode: Mode is not in use\n")));
        return BCM_E_NOT_FOUND;
    }
    if ( global_meter_offset_mode[unit][svc_meter_mode].reference_count != 0) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Mode is still in use\n")));
        return BCM_E_INTERNAL;
    }

    /* Step1:  Reset selector keys */
    SOC_IF_ERROR_RETURN(soc_reg_set(unit,
        _pkt_attr_sel_key_reg[svc_meter_mode],
        REG_PORT_ANY,
        0,
        pkt_attr_selector_key_value));

     /* Step2: Reset Offset table filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
        SVM_OFFSET_TABLEm,
        svc_meter_mode,
        NULL));
    /* Step3: Cleanup based on mode */
    mode_type_v = global_meter_offset_mode[unit][svc_meter_mode].\
                                                    meter_attr.mode_type_v;
    switch (mode_type_v) {
       case uncompressed_mode:
           break;
       case udf_mode:
           break;
       case cascade_mode:
           break;
       case compressed_mode:
       {
            compressed_attr_selectors_t  compressed_attr_selectors_v =
                           global_meter_offset_mode[unit][svc_meter_mode].\
                                  meter_attr.compressed_attr_selectors_v;
            pkt_attr_bits_t  pkt_attr_bits_v = compressed_attr_selectors_v.\
                                                            pkt_attr_bits_v;

           /* Step3: Cleanup map array */
           if ((pkt_attr_bits_v.cng != 0) || (pkt_attr_bits_v.int_pri) || (pkt_attr_bits_v.short_int_pri)) {
               SOC_IF_ERROR_RETURN(
                   soc_mem_clear(unit, ING_SVM_PRI_CNG_MAPm, MEM_BLOCK_ALL, TRUE));
           }
           if ((pkt_attr_bits_v.vlan_format != 0) ||
               (pkt_attr_bits_v.outer_dot1p) ||
               (pkt_attr_bits_v.inner_dot1p)) {
               SOC_IF_ERROR_RETURN(
                   soc_mem_clear(unit, ING_SVM_PKT_PRI_MAPm, MEM_BLOCK_ALL, TRUE));
           }
           if (pkt_attr_bits_v.ing_port != 0){
               SOC_IF_ERROR_RETURN(
                   soc_mem_clear(unit, ING_SVM_PORT_MAPm, MEM_BLOCK_ALL, TRUE));
           }
           if (pkt_attr_bits_v.tos != 0) {
               SOC_IF_ERROR_RETURN(
                   soc_mem_clear(unit, ING_SVM_TOS_MAPm, MEM_BLOCK_ALL, TRUE));
           }
           if ((pkt_attr_bits_v.pkt_resolution != 0) ||
                (pkt_attr_bits_v.svp_type) ||
                (pkt_attr_bits_v.drop)) {
               SOC_IF_ERROR_RETURN(
                  soc_mem_clear(unit, ING_SVM_PKT_RES_MAPm, MEM_BLOCK_ALL, TRUE));
           }
           break;
       }
       case udf_cascade_mode:
       case udf_cascade_with_coupling_mode:
           break;
       default:
           LOG_DEBUG(BSL_LS_BCM_POLICER,
                     (BSL_META_U(unit,
                                 "Invalid offset mode\n")));
           return BCM_E_PARAM;
    }
    /* attr_selectors is allocated memory when mode_id is created explicitly
     * via _mode_id_create() API. This is the best place to free this
     * memory as after this call mode is set to un-used and can be reused */
    if (global_meter_offset_mode[unit][svc_meter_mode].attr_selectors != NULL) {
        sal_free (global_meter_offset_mode[unit][svc_meter_mode].attr_selectors);
        global_meter_offset_mode[unit][svc_meter_mode].attr_selectors = NULL;
    }
    sal_memset (&global_meter_offset_mode[unit][svc_meter_mode], 0,
            sizeof(global_meter_offset_mode[unit][svc_meter_mode]));

    global_meter_offset_mode[unit][svc_meter_mode].used = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_get_available_mode
 * Purpose:
 *      Internal function to get a free meter offset table entry
 * Parameters:
 *     Unit                  - (IN) unit number
 *     svc_meter_mode        - (IN) Meter offset mode
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_get_available_mode(uint32 unit,
    bcm_policer_svc_meter_mode_t        *svc_meter_mode)
{
    uint32    mode_index = 1;
    for (; mode_index < BCM_POLICER_SVC_METER_MAX_MODE; mode_index++) {
        if (global_meter_offset_mode[unit][mode_index].used == 0) {
            *svc_meter_mode = mode_index;
            return BCM_E_NONE;
        }
    }
    return BCM_E_FULL;
}

typedef enum pkt_attrs_e {
    pkt_attr_ip_pkt,
    pkt_attr_drop,
    pkt_attr_svp_group,
    pkt_attr_pkt_resolution,
    pkt_attr_tos,
    pkt_attr_ing_port,
    pkt_attr_inner_dot1p,
    pkt_attr_outer_dot1p,
    pkt_attr_vlan_format,
    pkt_attr_int_pri,
    pkt_attr_cng,
    pkt_attr_short_int_pri,
    pkt_attr_tos_ecn,
    pkt_attr_count
} pkt_attrs_t;

static _bcm_policer_pkt_attr_bit_pos_t  kt_pkt_attr_bit_pos[pkt_attr_count] =  {
                                       {0, 0}, {1, 1}, {2, 2}, {3, 8},
                                       {11,16}, {17,22}, {23,25}, {26,28},
                                       {29,30}, {31,34}, {35,36}, {31,33},
                                       {9,10}
                                  };
#ifdef BCM_KATANA2_SUPPORT
static _bcm_policer_pkt_attr_bit_pos_t kt2_pkt_attr_bit_pos[pkt_attr_count] =  {
                                       {0, 0}, {1, 1}, {2, 4}, {5, 10},
                                       {13,18}, {19, 26}, {27, 29}, {30,32},
                                       {33,34}, {35,38}, {39, 40}, {35,37},
                                       {11,12}
                                  };
#endif
#if defined (BCM_APACHE_SUPPORT) || defined (BCM_SABER2_SUPPORT)
static _bcm_policer_pkt_attr_bit_pos_t sb2_pkt_attr_bit_pos[pkt_attr_count] =  {
                                       {0, 0}, {1, 1}, {2, 4}, {5, 10},
                                       {13,18}, {19, 25}, {27, 29}, {30,32},
                                       {33,34}, {35,38}, {39, 40}, {35,37},
                                       {11,12}
                                  };
#endif

/*
 * Function:
 *      _bcm_policer_svc_meter_update_selector_keys
 * Purpose:
 *      Update the selector keys
 * Parameters:
 *     Unit                  - (IN) unit number
 *     mode_type_v           - (IN) Meter offset mode
 *     pkt_attr_selector_key - (IN) Selector key
 *     pkt_attr_bits_v       - (IN) Packet attribute bits
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_update_selector_keys(
    int           unit,
    bcm_policer_svc_meter_mode_type_t    mode_type_v,
    soc_reg_t          pkt_attr_selector_key,
    pkt_attr_bits_t    pkt_attr_bits_v)
{
    uint64   pkt_attr_selector_key_value;
    uint8    current_bit_position=0;
    _bcm_policer_pkt_attr_bit_pos_t *pkt_attr_bit_pos = NULL;

    pkt_attr_bit_pos  =
            BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos;

    COMPILER_64_ZERO(pkt_attr_selector_key_value);
    if (!((pkt_attr_selector_key >= ING_SVM_PKT_ATTR_SELECTOR_KEY_0r) &&
          (pkt_attr_selector_key <= ING_SVM_PKT_ATTR_SELECTOR_KEY_3r))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid Key for packet attribute selector\n")));
            return BCM_E_PARAM;
    }
    /* BCM_POLICER_SVC_METER_UDF_MODE not supported here */
    if (!((mode_type_v == uncompressed_mode) ||
          (mode_type_v == compressed_mode))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "UDF mode not supported \n")));
            return BCM_E_PARAM;
    }
    /* First Get complete value of ING_SVM_PKT_ATTR_SELECTOR_KEY_?r value */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit,
            pkt_attr_selector_key,
            REG_PORT_ANY,
            0,
            &pkt_attr_selector_key_value));

    /* Next set field value for
                 ING_SVM_PKT_ATTR_SELECTOR_KEY_?r:SELECTOR_KEY field*/
    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USER_SPECIFIED_UDF_VALIDf,
            0);
    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USE_UDF_KEYf,
            0);
    if (mode_type_v == compressed_mode) {
        soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USE_COMPRESSED_PKT_KEYf,
            1);
    } else {
        soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USE_COMPRESSED_PKT_KEYf,
            0);
    }
    if (pkt_attr_bits_v.ip_pkt != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_ip_pkt].start_bit, /*IP_PKT bit position*/
            pkt_attr_bits_v.ip_pkt,/*1:IP_PKT total bit*/
            &current_bit_position));
    }
    if (pkt_attr_bits_v.drop != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_drop].start_bit, /*DROP bit position */
            pkt_attr_bits_v.drop, /*1:DROP total bits */
            &current_bit_position));
    }
    if (pkt_attr_bits_v.svp_type != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_svp_group].start_bit, /* SVP_TYPE bit position */
            pkt_attr_bits_v.svp_type,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.pkt_resolution != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_pkt_resolution].start_bit, /* PKT_RESOLUTION bit position */
            pkt_attr_bits_v.pkt_resolution,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.tos != 0) {
        BCM_IF_ERROR_RETURN(
           _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_tos].start_bit, /* TOS DSCP bit position */
            pkt_attr_bits_v.tos,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.tos_ecn != 0) {
        BCM_IF_ERROR_RETURN(
           _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_tos_ecn].start_bit, /* TOS ECN bit position */
            pkt_attr_bits_v.tos_ecn,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.ing_port != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_ing_port].start_bit, /* ING_PORT bit position */
            pkt_attr_bits_v.ing_port,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.inner_dot1p != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_inner_dot1p].start_bit, /* INNER_DOT1P bit position */
            pkt_attr_bits_v.inner_dot1p,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.outer_dot1p != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_outer_dot1p].start_bit, /* OUTER_DOT1P bit position */
            pkt_attr_bits_v.outer_dot1p,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.vlan_format != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_vlan_format].start_bit, /* VLAN_FORMAT bit position*/
            pkt_attr_bits_v.vlan_format,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.int_pri != 0) {
        int start_bit = 0;
        if (mode_type_v == compressed_mode) {
            start_bit = BCM_SVM_DEV_ATTR(unit)->compressed_int_pri_bit_pos;
        } else {
            start_bit = pkt_attr_bit_pos[pkt_attr_int_pri].start_bit; /*INT_PRI bit position*/
        }
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            start_bit,
            pkt_attr_bits_v.int_pri,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.cng != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_cng].start_bit, /* CNG bit position */
            pkt_attr_bits_v.cng,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.short_int_pri != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_bit_pos[pkt_attr_short_int_pri].start_bit,
            pkt_attr_bits_v.short_int_pri,
            &current_bit_position));
    }

    /* Finally set value for ING_SVM_PKT_ATTR_SELECTOR_KEY_?r */
    SOC_IF_ERROR_RETURN(soc_reg_set(unit,
            pkt_attr_selector_key,
            REG_PORT_ANY,
            0,
            pkt_attr_selector_key_value));
    return BCM_E_NONE;
}

typedef enum pkt_cmprsd_attr_map_e {
    pkt_attr_pri_cng_map,
    pkt_attr_pkt_pri_map,
    pkt_attr_port_map,
    pkt_attr_tos_map,
    pkt_attr_pkt_res_map,
    pkt_attr_ip_pkt_map, /* no map for IP_PKT in h/w */
    pkt_attr_map_count
} pkt_cmprsd_attrs_map_t;

static _bcm_policer_pkt_attr_bit_pos_t  kt_pkt_cmprsd_attr_map_bit_pos[] =  {
                                       {32,37}, {24,31}, {17,23}, {9,16},
                                       {1,8}, {0,0}
                                  };
#ifdef BCM_KATANA2_SUPPORT
static _bcm_policer_pkt_attr_bit_pos_t kt2_pkt_cmprsd_attr_map_bit_pos[] =  {
                                       {32,37}, {24,31}, {17,23}, {9,16},
                                       {1,8}, {0,0}
                                  };
#endif
#if defined (BCM_APACHE_SUPPORT) || defined (BCM_SABER2_SUPPORT)
static _bcm_policer_pkt_attr_bit_pos_t sb2_pkt_cmprsd_attr_map_bit_pos[] =  {
                                       {32,37}, {24,31}, {17,23}, {9,16},
                                       {1,8}, {0,0}
                                  };
#endif

/*
 * Function:
 *      _bcm_policer_svc_meter_update_selector_keys2
 * Purpose:
 *      Update the selector keys for compressed mode
 *      taking H/w map format in account
 * Parameters:
 *     Unit                  - (IN) unit number
 *     mode_type_v           - (IN) Meter offset mode
 *     pkt_attr_selector_key - (IN) Selector key
 *     pkt_attr_bits_v       - (IN) Packet attribute bits
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_update_selector_keys2(
    int           unit,
    bcm_policer_svc_meter_mode_type_t    mode_type_v,
    soc_reg_t          pkt_attr_selector_key,
    pkt_attr_bits_t    pkt_attr_bits_v)
{
    uint64   pkt_attr_selector_key_value;
    uint8    current_bit_position=0;
    _bcm_policer_pkt_attr_bit_pos_t *pkt_attr_map_bit_pos = NULL;
    uint32 total_bits_to_set = 0;

    pkt_attr_map_bit_pos =
            BCM_SVM_DEV_ATTR(unit)->compressed_attr_map_bit_pos;

    COMPILER_64_ZERO(pkt_attr_selector_key_value);
    if (!((pkt_attr_selector_key >= ING_SVM_PKT_ATTR_SELECTOR_KEY_0r) &&
          (pkt_attr_selector_key <= ING_SVM_PKT_ATTR_SELECTOR_KEY_3r))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid Key for packet attribute selector\n")));
            return BCM_E_PARAM;
    }
    /* BCM_POLICER_SVC_METER_UDF_MODE not supported here */
    if (!(mode_type_v == compressed_mode)) {
            return BCM_E_PARAM;
    }
    /* First Get complete value of ING_SVM_PKT_ATTR_SELECTOR_KEY_?r value */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit,
            pkt_attr_selector_key,
            REG_PORT_ANY,
            0,
            &pkt_attr_selector_key_value));

    /* Next set field value for
                 ING_SVM_PKT_ATTR_SELECTOR_KEY_?r:SELECTOR_KEY field*/
    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USER_SPECIFIED_UDF_VALIDf,
            0);
    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USE_UDF_KEYf,
            0);
    soc_reg64_field32_set(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            USE_COMPRESSED_PKT_KEYf,
            1);

    if (pkt_attr_bits_v.ip_pkt != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_map_bit_pos[pkt_attr_ip_pkt_map].start_bit,
            pkt_attr_bits_v.ip_pkt,
            &current_bit_position));
    }
    if ((pkt_attr_bits_v.drop != 0) ||
        (pkt_attr_bits_v.svp_type != 0) ||
        (pkt_attr_bits_v.pkt_resolution != 0)) {
        total_bits_to_set = pkt_attr_bits_v.drop +
                            pkt_attr_bits_v.svp_type +
                            pkt_attr_bits_v.pkt_resolution;
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_map_bit_pos[pkt_attr_pkt_res_map].start_bit,
            total_bits_to_set,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.tos != 0) {
        BCM_IF_ERROR_RETURN(
           _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_map_bit_pos[pkt_attr_tos_map].start_bit,
            pkt_attr_bits_v.tos,
            &current_bit_position));
    }
    if (pkt_attr_bits_v.ing_port != 0) {
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_map_bit_pos[pkt_attr_port_map].start_bit,
            pkt_attr_bits_v.ing_port,
            &current_bit_position));
    }
    if ((pkt_attr_bits_v.inner_dot1p != 0) ||
        (pkt_attr_bits_v.outer_dot1p != 0) ||
        (pkt_attr_bits_v.vlan_format != 0)) {
        total_bits_to_set = pkt_attr_bits_v.inner_dot1p +
                            pkt_attr_bits_v.outer_dot1p +
                            pkt_attr_bits_v.vlan_format;
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_map_bit_pos[pkt_attr_pkt_pri_map].start_bit,
            total_bits_to_set,
            &current_bit_position));
    }
    if ((pkt_attr_bits_v.int_pri != 0) ||
        (pkt_attr_bits_v.cng != 0)) {
        total_bits_to_set = pkt_attr_bits_v.int_pri + pkt_attr_bits_v.cng;
        BCM_IF_ERROR_RETURN(
          _bcm_policer_svc_meter_update_selector_keys_enable_fields(
            unit,
            pkt_attr_selector_key,
            &pkt_attr_selector_key_value,
            pkt_attr_map_bit_pos[pkt_attr_pri_cng_map].start_bit,
            total_bits_to_set,
            &current_bit_position));
    }

    /* Finally set value for ING_SVM_PKT_ATTR_SELECTOR_KEY_?r */
    SOC_IF_ERROR_RETURN(soc_reg_set(unit,
            pkt_attr_selector_key,
            REG_PORT_ANY,
            0,
            pkt_attr_selector_key_value));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_create_cascade_mode
 * Purpose:
 *     Function to create an offset mode with uncompressed packet attributes
 * Parameters:
 *     Unit                  - (IN) unit number
 *     uncompressed_attr_selectors_v - (IN) Packet attribute selectors
 *     group_mode            - (IN) Policer group mode
 *     num_pol               - (IN) Number of policers
 *     svc_meter_mode        - (OUT) Allocated meter offset mode
 *     total_bits            - (OUT) Total number of packet attribute bits
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_create_cascade_mode(
    int unit,
    uncompressed_attr_selectors_t *uncompressed_attr_selectors_v,
    bcm_policer_group_mode_t     group_mode,
    int                          num_pol,
    bcm_policer_svc_meter_mode_t *svc_meter_mode)
{
    bcm_error_t                         bcm_error_v = BCM_E_NONE;
    bcm_policer_svc_meter_mode_t        bcm_policer_svc_meter_mode = 0;
    pkt_attr_bits_t     pkt_attr_bits_v = {0};
    uint32              total_bits_used = 0;
    uint32              index = 0;
    bcm_policer_svc_meter_bookkeep_mode_t   mode_info;
    uncompressed_attr_selectors_t *un_attr;
    int                 port_attr_size = BCM_POLICER_SVC_METER_ING_PORT_ATTR_SIZE;
    int                 svp_type_attr_size = 0;

    /*check if a mode is already configured with these attributes */
    for (index = 1; index < BCM_POLICER_SVC_METER_MAX_MODE; index++) {
        if (_bcm_policer_svc_meter_get_mode_info(unit,index,&mode_info)
                                                         == BCM_E_NONE) {
            if ((mode_info.meter_attr.mode_type_v != cascade_mode) ||
                                 (group_mode != mode_info.group_mode )) {
                continue;
            }
            if (mode_info.no_of_policers != num_pol) {
                continue;
            }
            un_attr = &mode_info.meter_attr.uncompressed_attr_selectors_v;
            if (un_attr->uncompressed_attr_bits_selector ==
                uncompressed_attr_selectors_v->uncompressed_attr_bits_selector) {
                *svc_meter_mode = index;
                return BCM_E_EXISTS;
            }
        }
    }

    bcm_error_v = _bcm_policer_svc_meter_get_available_mode(unit,
                                                &bcm_policer_svc_meter_mode);
    if (bcm_error_v != BCM_E_NONE) {
        LOG_WARN(BSL_LS_BCM_POLICER,
                 (BSL_META_U(unit, " Offset Table is full\n")));
        return bcm_error_v;
    }

    /* Step1: Packet Attribute selection */
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_CNG_ATTR_BITS) {
        pkt_attr_bits_v.cng = BCM_POLICER_SVC_METER_CNG_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_CNG_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS) {
        pkt_attr_bits_v.int_pri = BCM_POLICER_SVC_METER_INT_PRI_ATTR_SIZE ;
        total_bits_used += BCM_POLICER_SVC_METER_INT_PRI_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SHORT_INT_PRI_ATTR_BITS) {
        pkt_attr_bits_v.short_int_pri = BCM_POLICER_SVC_METER_SHORT_INT_PRI_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_SHORT_INT_PRI_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
         BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS) {
        pkt_attr_bits_v.vlan_format =
                            BCM_POLICER_SVC_METER_VLAN_FORMAT_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_VLAN_FORMAT_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS) {
        pkt_attr_bits_v.outer_dot1p =
                               BCM_POLICER_SVC_METER_OUTER_DOT1P_ATTR_SIZE ;
        total_bits_used += BCM_POLICER_SVC_METER_OUTER_DOT1P_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS) {
            pkt_attr_bits_v.inner_dot1p =
                               BCM_POLICER_SVC_METER_INNER_DOT1P_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_INNER_DOT1P_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INGRESS_PORT_ATTR_BITS) {
        port_attr_size =
                BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size;
        pkt_attr_bits_v.ing_port = port_attr_size;
        total_bits_used += port_attr_size;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_TOS_ATTR_BITS) {
        pkt_attr_bits_v.tos = BCM_POLICER_SVC_METER_TOS_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_TOS_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
        pkt_attr_bits_v.pkt_resolution =
                             BCM_POLICER_SVC_METER_PKT_REOLUTION_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_PKT_REOLUTION_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS) {
        svp_type_attr_size =
                BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size;
        total_bits_used += svp_type_attr_size;
        pkt_attr_bits_v.svp_type = svp_type_attr_size;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_DROP_ATTR_BITS) {
        pkt_attr_bits_v.drop = BCM_POLICER_SVC_METER_DROP_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_DROP_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS) {
        pkt_attr_bits_v.ip_pkt = BCM_POLICER_SVC_METER_IP_PKT_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_IP_PKT_ATTR_SIZE;
    }
    if (total_bits_used > BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Key size exceeds max allowed size \n")));
        return BCM_E_PARAM;
    }
    /* Step2: Packet Attribute filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_selector_keys(unit,
               uncompressed_mode,
               _pkt_attr_sel_key_reg[bcm_policer_svc_meter_mode],
               pkt_attr_bits_v));

    /* Step3: Offset table filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
                           SVM_OFFSET_TABLEm,
                           bcm_policer_svc_meter_mode,
                           &uncompressed_attr_selectors_v->offset_map[0]));

    /* Step4: Final: reserve mode and return */
    *svc_meter_mode = bcm_policer_svc_meter_mode;
    return BCM_E_NONE;
}
/*
 * Function:
 *      _bcm_policer_svc_meter_create_uncompress_mode
 * Purpose:
 *     Function to create an offset mode with uncompressed packet attributes
 * Parameters:
 *     Unit                  - (IN) unit number
 *     uncompressed_attr_selectors_v - (IN) Packet attribute selectors
 *     group_mode            - (IN) Policer group mode
 *     svc_meter_mode        - (OUT) Allocated meter offset mode
 *     total_bits            - (OUT) Total number of packet attribute bits
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_create_uncompress_mode(
    int unit,
    uncompressed_attr_selectors_t *uncompressed_attr_selectors_v,
    bcm_policer_group_mode_t     group_mode,
    bcm_policer_svc_meter_mode_t *svc_meter_mode)
{
    bcm_error_t                         bcm_error_v = BCM_E_NONE;
    bcm_policer_svc_meter_mode_t        bcm_policer_svc_meter_mode = 0;
    pkt_attr_bits_t     pkt_attr_bits_v = {0};
    uint32              total_bits_used = 0;
    uint32              index = 0;
    bcm_policer_svc_meter_bookkeep_mode_t   mode_info;
    uncompressed_attr_selectors_t *un_attr;
    int                 port_attr_size = BCM_POLICER_SVC_METER_ING_PORT_ATTR_SIZE;
    int                 svp_type_attr_size = 0;

    /*check if a mode is already configured with these attributes */
    for (index = 1; index < BCM_POLICER_SVC_METER_MAX_MODE; index++) {
        if (_bcm_policer_svc_meter_get_mode_info(unit,index,&mode_info)
                                                         == BCM_E_NONE) {
            if ((mode_info.meter_attr.mode_type_v != uncompressed_mode) ||
                                 (group_mode != mode_info.group_mode )) {
                continue;
            }
            un_attr = &mode_info.meter_attr.uncompressed_attr_selectors_v;
            if (un_attr->uncompressed_attr_bits_selector ==
                uncompressed_attr_selectors_v->uncompressed_attr_bits_selector) {
                *svc_meter_mode = index;
                return BCM_E_EXISTS;
            }
        }
    }

    bcm_error_v = _bcm_policer_svc_meter_get_available_mode(unit,
                                                &bcm_policer_svc_meter_mode);
    if (bcm_error_v != BCM_E_NONE) {
        LOG_WARN(BSL_LS_BCM_POLICER,
                 (BSL_META_U(unit,
                             " Offset Table is full\n")));
        return bcm_error_v;
    }

    /* Step1: Packet Attribute selection */
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_CNG_ATTR_BITS) {
        pkt_attr_bits_v.cng = BCM_POLICER_SVC_METER_CNG_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_CNG_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS) {
        pkt_attr_bits_v.int_pri = BCM_POLICER_SVC_METER_INT_PRI_ATTR_SIZE ;
        total_bits_used += BCM_POLICER_SVC_METER_INT_PRI_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SHORT_INT_PRI_ATTR_BITS) {
        pkt_attr_bits_v.short_int_pri = BCM_POLICER_SVC_METER_SHORT_INT_PRI_ATTR_SIZE ;
        total_bits_used += BCM_POLICER_SVC_METER_SHORT_INT_PRI_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
         BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS) {
        pkt_attr_bits_v.vlan_format =
                            BCM_POLICER_SVC_METER_VLAN_FORMAT_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_VLAN_FORMAT_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS) {
        pkt_attr_bits_v.outer_dot1p =
                               BCM_POLICER_SVC_METER_OUTER_DOT1P_ATTR_SIZE ;
        total_bits_used += BCM_POLICER_SVC_METER_OUTER_DOT1P_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS) {
            pkt_attr_bits_v.inner_dot1p =
                               BCM_POLICER_SVC_METER_INNER_DOT1P_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_INNER_DOT1P_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INGRESS_PORT_ATTR_BITS) {
        port_attr_size =
                BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size;
        pkt_attr_bits_v.ing_port = port_attr_size;
        total_bits_used += port_attr_size;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_TOS_ATTR_BITS) {
        pkt_attr_bits_v.tos = BCM_POLICER_SVC_METER_TOS_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_TOS_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_TOS_ECN_ATTR_BITS) {
        pkt_attr_bits_v.tos_ecn = BCM_POLICER_SVC_METER_TOS_ECN_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_TOS_ECN_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS) {
        pkt_attr_bits_v.pkt_resolution =
                             BCM_POLICER_SVC_METER_PKT_REOLUTION_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_PKT_REOLUTION_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS) {
        svp_type_attr_size =
                BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size;
        total_bits_used += svp_type_attr_size;
        pkt_attr_bits_v.svp_type = svp_type_attr_size;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_DROP_ATTR_BITS) {
        pkt_attr_bits_v.drop = BCM_POLICER_SVC_METER_DROP_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_DROP_ATTR_SIZE;
    }
    if (uncompressed_attr_selectors_v->uncompressed_attr_bits_selector &
        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS) {
        pkt_attr_bits_v.ip_pkt = BCM_POLICER_SVC_METER_IP_PKT_ATTR_SIZE;
        total_bits_used += BCM_POLICER_SVC_METER_IP_PKT_ATTR_SIZE;
    }
    if (total_bits_used > BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Key size exceeds max allowed size \n")));
        return BCM_E_PARAM;
    }
    /* Step2: Packet Attribute filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_selector_keys(unit,
               uncompressed_mode,
               _pkt_attr_sel_key_reg[bcm_policer_svc_meter_mode],
               pkt_attr_bits_v));

    /* Step3: Offset table filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
                           SVM_OFFSET_TABLEm,
                           bcm_policer_svc_meter_mode,
                           &uncompressed_attr_selectors_v->offset_map[0]));

    /* Step4: Final: reserve mode and return */
    *svc_meter_mode = bcm_policer_svc_meter_mode;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_create_compress_mode
 * Purpose:
 *     Function to create an offset mode with compressed packet attributes
 * Parameters:
 *     Unit                  - (IN) unit number
 *     compressed_attr_selectors_v - (IN) Packet attribute selectors
 *     group_mode            - (IN) Policer group mode
 *     svc_meter_mode        - (OUT) Allocated meter offset mode
 *     total_bits            - (OUT) Total number of packet attribute bits
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_create_compress_mode(
    int unit,
    compressed_attr_selectors_t *compressed_attr_selectors_v,
    bcm_policer_group_mode_t group_mode,
    int8 type,
    bcm_policer_svc_meter_mode_t *svc_meter_mode)
{
    bcm_error_t                     bcm_error_v=BCM_E_NONE;
    bcm_policer_svc_meter_mode_t    bcm_policer_svc_meter_mode = 0;
    pkt_attr_bits_t                 *pkt_attr_bits_v = NULL;
    pkt_attr_bits_t                 *config_pkt_attr = NULL;
    uint32                          total_bits_used = 0;
    uint32                          index = 0;
    bcm_policer_svc_meter_bookkeep_mode_t   *mode_info = NULL;
    pkt_attr_bits_t                 *pkt_attr_bits = NULL;
    uint32                           map_entry = 0;
    int                             index_max = 0;
    int                 port_attr_size = BCM_POLICER_SVC_METER_ING_PORT_ATTR_SIZE;
    uint32                          pri_cng_map_in_use = 0;
    uint32                          pkt_pri_map_in_use = 0;
    uint32                          port_map_in_use = 0;
    uint32                          tos_map_in_use = 0;
    uint32                          pkt_res_map_in_use = 0;
    int                             svp_type_attr_size = 0;

    mode_info = sal_alloc(sizeof(bcm_policer_svc_meter_bookkeep_mode_t),
                                                   "mode bookkeep info");
    if ( mode_info == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Failed to allocate memory for mode bookkeep "
                                "info\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(mode_info, 0, sizeof(bcm_policer_svc_meter_bookkeep_mode_t));

    config_pkt_attr = &compressed_attr_selectors_v->pkt_attr_bits_v;
    /*check if a mode is already configured with these attributes */
    for (index = 1; index < BCM_POLICER_SVC_METER_MAX_MODE; index++) {
        if (_bcm_policer_svc_meter_get_mode_info(
                             unit, index, mode_info) == BCM_E_NONE) {
            if ((mode_info->meter_attr.mode_type_v != compressed_mode) ||
               (group_mode != mode_info->group_mode )) {
                if (mode_info->meter_attr.mode_type_v == compressed_mode) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Some other policer group is already "
                                          "using this resource\n")));
                    sal_free(mode_info);
                    return BCM_E_PARAM;
                }
                continue;
            }
            pkt_attr_bits = &mode_info->meter_attr.compressed_attr_selectors_v.\
                                                                pkt_attr_bits_v;
            if ((pkt_attr_bits->cng == config_pkt_attr->cng) &&
                (pkt_attr_bits->int_pri == config_pkt_attr->int_pri) &&
                (pkt_attr_bits->vlan_format == config_pkt_attr->vlan_format) &&
                (pkt_attr_bits->outer_dot1p == config_pkt_attr->outer_dot1p) &&
                (pkt_attr_bits->inner_dot1p == config_pkt_attr->inner_dot1p) &&
                (pkt_attr_bits->ing_port == config_pkt_attr->ing_port) &&
                (pkt_attr_bits->tos == config_pkt_attr->tos) &&
                (pkt_attr_bits->tos_ecn == config_pkt_attr->tos_ecn) &&
                (pkt_attr_bits->pkt_resolution == config_pkt_attr->pkt_resolution) &&
                (pkt_attr_bits->svp_type == config_pkt_attr->svp_type) &&
                (pkt_attr_bits->drop == config_pkt_attr->drop) &&
                (pkt_attr_bits->ip_pkt == config_pkt_attr->ip_pkt)) {
                    *svc_meter_mode = index;
                    sal_free(mode_info);
                    return BCM_E_EXISTS;
            }
            if ((pkt_attr_bits->cng != 0) ||
                (pkt_attr_bits->int_pri != 0)) {
                pri_cng_map_in_use = 1;
            }
            if ((pkt_attr_bits->vlan_format != 0) ||
                (pkt_attr_bits->outer_dot1p != 0) ||
                (pkt_attr_bits->inner_dot1p != 0)) {
                pkt_pri_map_in_use = 1;
            }
            if (pkt_attr_bits->ing_port != 0) {
                port_map_in_use = 1;
            }
            if ((pkt_attr_bits->tos != 0) ||
                (pkt_attr_bits->tos_ecn != 0)) {
                tos_map_in_use = 1;
            }
            if ((pkt_attr_bits->pkt_resolution != 0) ||
                (pkt_attr_bits->svp_type != 0) ||
                (pkt_attr_bits->drop != 0)) {
                pkt_res_map_in_use = 1;
            }
        }
    } /* end of for */
    sal_free(mode_info);
    bcm_error_v = _bcm_policer_svc_meter_get_available_mode(unit,
                                                &bcm_policer_svc_meter_mode);
    if (bcm_error_v != BCM_E_NONE) {
        return bcm_error_v;
    }
    /* Step1: Packet Attribute selection */
    if (config_pkt_attr->cng != 0) {
        if (config_pkt_attr->cng > BCM_POLICER_SVC_METER_CNG_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "CNG attribute size exceeds max allowed size\n")));
            return BCM_E_PARAM;
        }
        if (pri_cng_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->cng;
    }
    if (config_pkt_attr->int_pri != 0) {
        if (config_pkt_attr->int_pri > BCM_POLICER_SVC_METER_INT_PRI_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "int_pri attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (pri_cng_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->int_pri;
    }
    if (config_pkt_attr->vlan_format != 0) {
        if (config_pkt_attr->vlan_format >
                                BCM_POLICER_SVC_METER_VLAN_FORMAT_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Vlan attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (pkt_pri_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->vlan_format;
    }
    if (config_pkt_attr->outer_dot1p != 0) {
        if (config_pkt_attr->outer_dot1p >
                                BCM_POLICER_SVC_METER_OUTER_DOT1P_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Outer DOT1P attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (pkt_pri_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->outer_dot1p;
    }
    if (config_pkt_attr->inner_dot1p != 0) {
        if (config_pkt_attr->inner_dot1p >
                                BCM_POLICER_SVC_METER_INNER_DOT1P_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Inner DOT1P attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (pkt_pri_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->inner_dot1p;
    }
    if (config_pkt_attr->ing_port != 0) {
        port_attr_size =
                BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size;
        if (config_pkt_attr->ing_port > port_attr_size) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "port attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (port_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->ing_port;
    }
    if (config_pkt_attr->tos != 0) {
        if (config_pkt_attr->tos > BCM_POLICER_SVC_METER_TOS_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "TOS attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (tos_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->tos;
    }
    if (config_pkt_attr->tos_ecn != 0) {
        if (config_pkt_attr->tos_ecn > BCM_POLICER_SVC_METER_TOS_ECN_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "TOS ecn attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (tos_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->tos_ecn;
    }
    if (config_pkt_attr->pkt_resolution != 0) {
        if (config_pkt_attr->pkt_resolution >
                              BCM_POLICER_SVC_METER_PKT_REOLUTION_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Pkt resolution attribute size exceeds max "
                                  "allowed size\n")));
            return BCM_E_PARAM;
        }
        if (pkt_res_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->pkt_resolution;
    }
    if (config_pkt_attr->svp_type != 0) {
        svp_type_attr_size =
                BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size;
        if (config_pkt_attr->svp_type > svp_type_attr_size) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                    "SVP type attribute size exceeds max allowed "
                    "size\n")));
            return BCM_E_PARAM;
        }
        if (pkt_res_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->svp_type;
    }
    if (config_pkt_attr->drop != 0) {
        if (config_pkt_attr->drop > BCM_POLICER_SVC_METER_DROP_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "DROP attribute size exceeds max allowed  "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        if (pkt_res_map_in_use == 1) {
            bcm_error_v = BCM_E_RESOURCE;
        }
        total_bits_used += config_pkt_attr->drop;
    }
    if (config_pkt_attr->ip_pkt != 0) {
        if (config_pkt_attr->ip_pkt > BCM_POLICER_SVC_METER_IP_PKT_ATTR_SIZE) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "IP pkt attribute size exceeds max allowed "
                                  "size\n")));
            return BCM_E_PARAM;
        }
        total_bits_used += config_pkt_attr->ip_pkt;
    }
    if (bcm_error_v == BCM_E_RESOURCE) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
            (BSL_META_U(unit, "Resource not available\n")));
        return bcm_error_v;
    }

    if (total_bits_used > BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Key size exceeds max allowed  "
                              "size\n")));
        return BCM_E_PARAM;
    }
    pkt_attr_bits_v = &compressed_attr_selectors_v->pkt_attr_bits_v;

    /* Step2: Fill up map array */
    if ((pkt_attr_bits_v->cng != 0) || (pkt_attr_bits_v->int_pri)) {
        index_max = soc_mem_index_max(unit, ING_SVM_PRI_CNG_MAPm);
        for (index = 0; index <= index_max; index++) {
            map_entry =
               compressed_attr_selectors_v->compressed_pri_cnf_attr_map_v[index];
            soc_mem_write(
             unit,
             ING_SVM_PRI_CNG_MAPm,
             MEM_BLOCK_ALL,
             index,
             &map_entry);
        }
    }
    if ((pkt_attr_bits_v->vlan_format != 0) || (pkt_attr_bits_v->outer_dot1p) ||
                                              (pkt_attr_bits_v->inner_dot1p)) {
        index_max = soc_mem_index_max(unit, ING_SVM_PKT_PRI_MAPm);
         for (index = 0; index < BCM_SVC_METER_MAP_SIZE_256; index++) {
            map_entry =
               compressed_attr_selectors_v->compressed_pkt_pri_attr_map_v[index];
            soc_mem_write(
             unit,
             ING_SVM_PKT_PRI_MAPm,
             MEM_BLOCK_ALL,
             index,
             &map_entry);
        }

    }
    if (pkt_attr_bits_v->ing_port != 0) {
        index_max = soc_mem_index_max(unit, ING_SVM_PORT_MAPm);
        for (index = 0; index <= index_max; index++) {
            map_entry =
               compressed_attr_selectors_v->compressed_port_attr_map_v[index];
            soc_mem_write(
             unit,
             ING_SVM_PORT_MAPm,
             MEM_BLOCK_ALL,
             index,
             &map_entry);
        }
    }
    if ((pkt_attr_bits_v->tos != 0) || (pkt_attr_bits_v->tos_ecn != 0)) {
        index_max = soc_mem_index_max(unit, ING_SVM_TOS_MAPm);
        for (index = 0; index < BCM_SVC_METER_MAP_SIZE_256; index++) {
            map_entry =
               compressed_attr_selectors_v->compressed_tos_attr_map_v[index];
            soc_mem_write(
                unit,
                ING_SVM_TOS_MAPm,
                MEM_BLOCK_ALL,
                index,
                &map_entry);
        }
    }
    if ((pkt_attr_bits_v->pkt_resolution != 0) || (pkt_attr_bits_v->svp_type) ||
                                                 (pkt_attr_bits_v->drop))   {
        index_max = soc_mem_index_max(unit, ING_SVM_PKT_RES_MAPm);
        for (index = 0; index < BCM_SVC_METER_MAP_SIZE_256; index++) {
            map_entry =
               compressed_attr_selectors_v->compressed_pkt_res_attr_map_v[index];
            soc_mem_write(
             unit,
             ING_SVM_PKT_RES_MAPm,
             MEM_BLOCK_ALL,
             index,
             &map_entry);
        }
    }

    /* Step3: Packet Attribute filling */
    if (type == -1) {
        BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_selector_keys(unit,
                 compressed_mode,
                 _pkt_attr_sel_key_reg[bcm_policer_svc_meter_mode],
                 *pkt_attr_bits_v));
    } else {
        BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_selector_keys2(unit,
                 compressed_mode,
                 _pkt_attr_sel_key_reg[bcm_policer_svc_meter_mode],
                 *pkt_attr_bits_v));
    }

    /* Step4: Offset table filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
                            SVM_OFFSET_TABLEm,
                            bcm_policer_svc_meter_mode,
                            &compressed_attr_selectors_v->offset_map[0]));

    /* Step5: Final: reserve mode and return */
    *svc_meter_mode = bcm_policer_svc_meter_mode;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_create_udf_mode
 * Purpose:
 *     Function to create an offset mode with udf packet attributes
 * Parameters:
 *     Unit                  - (IN) unit number
 *     udf_pkt_attr          - (IN) Packet attribute selectors
 *     group_mode            - (IN) Policer group mode
 *     svc_meter_mode        - (OUT) Allocated meter offset mode
 *     total_bits            - (OUT) Total number of packet attribute bits
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t _bcm_policer_svc_meter_create_udf_mode(
    int unit,
    udf_pkt_attr_selectors_t     *udf_pkt_attr,
    bcm_policer_group_mode_t     group_mode,
    int8                         type,
    bcm_policer_svc_meter_mode_t *svc_meter_mode,
    uint32                       *total_bits)
{
    bcm_error_t                  bcm_error_v = BCM_E_NONE;
    bcm_policer_svc_meter_mode_t bcm_policer_svc_meter_mode = 0;
    uint32                       total_udf_bits = 0;
    bcm_policer_svc_meter_bookkeep_mode_t   mode_info;
    udf_pkt_attr_selectors_t     *udf_attr = NULL;
    uint32                       index = 0;
    bcm_policer_svc_meter_mode_type_t mode_type = 0;
    if (type == bcmPolicerGroupModeTypeNormal) {
        mode_type = udf_mode;
    } else if (type == bcmPolicerGroupModeTypeCascade) {
        mode_type = udf_cascade_mode;
    } else {
        mode_type = udf_cascade_with_coupling_mode;
    }

    /*check if a mode is already configured with these attributes */
    for (index = 1;index < BCM_POLICER_SVC_METER_MAX_MODE; index++) {
        if (_bcm_policer_svc_meter_get_mode_info(unit, index, &mode_info) ==
                                                                 BCM_E_NONE) {
            if ((mode_info.meter_attr.mode_type_v != mode_type) ||
               (group_mode != mode_info.group_mode )) {
                continue;
            }
            udf_attr = &mode_info.meter_attr.udf_pkt_attr_selectors_v;
            if ((udf_attr->drop == udf_pkt_attr->drop) &&
                (udf_attr->total_subdiv == udf_pkt_attr->total_subdiv) &&
                (sal_memcmp(&udf_attr->udf_subdiv[0], &udf_pkt_attr->udf_subdiv[0],
                    sizeof(bcm_policer_udf_sub_div_t) * SVC_METER_UDF_MAX_SUB_DIVISIONS) == 0) &&
                (udf_attr->num_selectors == udf_pkt_attr->num_selectors) &&
                (sal_memcmp(&udf_attr->offset_map[0],
                            &udf_pkt_attr->offset_map[0],
                            (BCM_SVC_METER_MAP_SIZE_256 * sizeof(offset_table_entry_t))) == 0)) {
                *svc_meter_mode = index;
                return BCM_E_EXISTS;
            }
        }
    }

    bcm_error_v = _bcm_policer_svc_meter_get_available_mode(unit,
                                           &bcm_policer_svc_meter_mode);
    if (bcm_error_v != BCM_E_NONE) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "offset table is full \n")));
        return bcm_error_v;
    }
    /* Step1: Packet Attribute filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_udf_selector_keys(
                unit,
                _pkt_attr_sel_key_reg[bcm_policer_svc_meter_mode],
                udf_pkt_attr,
                &total_udf_bits));

    /* Step2: Offset table filling */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
                        SVM_OFFSET_TABLEm,
                        bcm_policer_svc_meter_mode,
                        &udf_pkt_attr->offset_map[0]));

    /* Step3: Final: reserve mode and return */
    *svc_meter_mode = bcm_policer_svc_meter_mode;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_policer_svc_meter_create_mode
 * Purpose:
 *     Function to create an Meter offset mode
 * Parameters:
 *     Unit                  - (IN) unit number
 *     meter_attr            - (IN) Packet attribute selectors
 *     group_mode            - (IN) Policer group mode
 *     num_pol               - (IN) Number of policers
 *     svc_meter_mode        - (OUT) Allocated meter offset mode
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_svc_meter_create_mode (
    int unit,
    bcm_policer_svc_meter_attr_t *meter_attr,
    bcm_policer_group_mode_t group_mode,
    int8 type,
    int num_pol,
    bcm_policer_svc_meter_mode_t *svc_meter_mode)
{
    uint32    total_bits;
    int rv = BCM_E_NONE;

    switch (meter_attr->mode_type_v)
    {
        case uncompressed_mode:
            rv = _bcm_policer_svc_meter_create_uncompress_mode(
                    unit,
                    &meter_attr->uncompressed_attr_selectors_v,
                    group_mode, svc_meter_mode);
            if (BCM_FAILURE(rv) && (rv == BCM_E_EXISTS)) {
                return rv;
            }
            break;
        case compressed_mode:
            BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_create_compress_mode(
                    unit,
                    &meter_attr->compressed_attr_selectors_v,
                    group_mode, type, svc_meter_mode));
            break;
        case udf_mode:
        case udf_cascade_mode:
        case udf_cascade_with_coupling_mode:
            BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_create_udf_mode(
                    unit,
                    &meter_attr->udf_pkt_attr_selectors_v,
                    group_mode, type, svc_meter_mode, &total_bits));
            break;
        case cascade_mode:
            BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_create_cascade_mode(
                    unit,
                    &meter_attr->uncompressed_attr_selectors_v,
                    group_mode, num_pol, svc_meter_mode));
            break;

        default:
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid offset mode\n")));
            return BCM_E_PARAM;
    }
    if (BCM_FAILURE(rv) && ((rv == BCM_E_FULL) || (rv == BCM_E_PARAM))) {
        return rv;
    }
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_reserve_mode(unit,
                                    *svc_meter_mode,
                                    group_mode,
                                    meter_attr));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_policer_svc_meter_delete_mode
 * Purpose:
 *      Delete a meter offset mode
 * Parameters:
 *     Unit                  - (IN) unit number
 *     svc_meter_mode        - (IN) offset mode
 * Returns:
 *     BCM_E_XXX
 */
int  _bcm_esw_policer_svc_meter_delete_mode(
        int                                  unit,
        bcm_policer_svc_meter_mode_t         svc_meter_mode)
{
    if (svc_meter_mode == 0) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid mode passed: %d \n"),
                   svc_meter_mode));
        return BCM_E_PARAM; /*0 is Reserved */
    }
    if (svc_meter_mode > (BCM_POLICER_SVC_METER_MAX_MODE - 1)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid mode passed: %d \n"),
                   svc_meter_mode));
        return BCM_E_PARAM; /* Exceeding max allowed value */
    }
    return _bcm_policer_svc_meter_unreserve_mode(unit, svc_meter_mode);
}

/*
 * Function:
 *      _bcm_esw_get_policer_id_from_index_offset
 * Purpose:
 *      Get policer id, given the HW policer index and offset_mode
 * Parameters:
 *     Unit                  - (IN) unit number
 *     index                 - (IN) HW policer index
 *     offset_mode           - (IN) offset mode
 *     pid                   - (OUT) policer Id
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_get_policer_id_from_index_offset(int unit, int index,
                                              int offset_mode,
                                              bcm_policer_t *pid)
{
    int pool=0, size_of_pool=0;
    size_of_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    if (index > 0) {
       pool = index / size_of_pool;
       index = index % size_of_pool;
       *pid = (((offset_mode + 1) << BCM_POLICER_GLOBAL_METER_MODE_SHIFT) +
                    (pool << _shr_popcount(size_of_pool - 1)) + index);
    } else {
       *pid = 0;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_get_policer_table_index
 * Purpose:
 *     Get the HW policer index, given a policer id.
 * Parameters:
 *     Unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 *     index                 - (OUT) HW policer index
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_get_policer_table_index(int unit, bcm_policer_t policer,
                                                         int *index)
{
    int rv = BCM_E_NONE;
    int size_pool = 0, num_pools = 0;
    int pool=0;
    int offset_mask = 0;
    int pool_mask = 0, pool_offset = 0;
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;
    pool = ((policer & pool_mask) >> pool_offset);
    *index = ((policer & offset_mask) + (pool * size_pool));
    return rv;
}

/*
 * Function:
 *      _bcm_esw_get_corres_policer_for_pool
 * Purpose:
 *     Get the policer Id, corresponding to a given policer id and specified pool.
 * Parameters:
 *     Unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 *     new_pool              - (IN) new pool for which policer Id required
 *     new_policer           - (OUT) New Policer Id for given pool in policer Id octuple
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_get_corres_policer_for_pool(int unit, bcm_policer_t policer, int new_pool,
                                                         bcm_policer_t *new_policer)
{
    int size_pool, pool_offset, offset_val;
    int num_pools;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    pool_offset = _shr_popcount(size_pool - 1);
    offset_val = policer & BCM_POLICER_GLOBAL_METER_MODE_MASK;
    num_pools =  SOC_INFO(unit).global_meter_pools;

    if ((new_pool < 0) || (new_pool >= num_pools)) {
        return BCM_E_INTERNAL;
    }

    *new_policer = offset_val + (new_pool << pool_offset) + (policer & (size_pool - 1));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_svm_source_traverse
 * Purpose:
 *     Traverse through all mem entires for a mem and call callback function
 * Parameters:
 *     Unit                  - (IN) unit number
 *     mem                   - (IN) table
 *     start_idx             - (IN) Start index where table traversal to begin
 *     end_idx               - (IN) Last index where table traversal to stop
 *     cb                    - (IN) Callback function
 *     user_data             - (IN) Argument to call back function
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t
_bcm_esw_svm_source_traverse(int unit, soc_mem_t mem, int start_idx,
            int end_idx, _bcm_esw_svm_mem_entry_traverse_cb cb, void *user_data)
{
    int rv = BCM_E_NONE;
    int8 *buffer = NULL;
    void *entry = NULL;
    uint32 idx = 0, entry_idx = 0;
    uint32 mem_min = 0, mem_max = 0;
    uint32 entry_dirty = 0, blk_dirty = 0;
    uint32 chunk_size = 0, mem_entry_size = 0;

    mem_min = soc_mem_index_min(unit, mem);
    mem_max = soc_mem_index_max(unit, mem);

    end_idx = (end_idx < 0) ? mem_max : end_idx;
    start_idx = (start_idx < 0) ? mem_min : start_idx;

    if ((start_idx > mem_max) ||
            (end_idx > mem_max) ||
            (start_idx > end_idx)) {
        return BCM_E_PARAM;
    }

    chunk_size = ((end_idx - start_idx + 1) > BCM_SVM_DMA_CHUNK_SIZE) ?
                BCM_SVM_DMA_CHUNK_SIZE : (end_idx - start_idx + 1);
    mem_entry_size = SOC_MEM_WORDS(unit, mem) * sizeof(uint32);

    buffer = soc_cm_salloc(unit, mem_entry_size * chunk_size, "svm mem buffer");
    if (buffer == NULL)  {
        return BCM_E_MEMORY;
    }
    for (idx = start_idx; idx <= end_idx; idx += chunk_size) {
        chunk_size = ((end_idx - idx + 1) > BCM_SVM_DMA_CHUNK_SIZE) ?
                    BCM_SVM_DMA_CHUNK_SIZE : (end_idx - idx + 1);

        if ((rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ALL,
                    idx, idx + chunk_size - 1, buffer)) != SOC_E_NONE) {
            break;
        }
        for (entry_idx = 0; entry_idx < chunk_size; entry_idx++) {
            entry = soc_mem_table_idx_to_pointer(unit, mem, void *,
                            buffer, entry_idx);
            entry_dirty = 0;
            rv = (*cb)(unit, mem, idx + entry_idx, entry, user_data, &entry_dirty);
            if (BCM_FAILURE(rv)) {
                break;
            }
            if (entry_dirty) {
                blk_dirty = 1;
            }
        }
        if (BCM_SUCCESS(rv) && blk_dirty) {
            rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL,
                    idx, idx + chunk_size - 1, buffer);
        }
        if (BCM_FAILURE(rv)) {
            break;
        }
    }

    if (buffer != NULL) {
        soc_cm_sfree(unit, buffer);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_cleanup
 * Purpose:
 *       Cleanup the global meter(service meter) internal data structure on
 *       init failure.
 * Parameters:
 *     Unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_esw_global_meter_cleanup(int unit)
{
    int rv = BCM_E_NONE;
    int mode = 0;
    int pool_id = 0, num_pools = 0;
    int num_banks_per_pool = 1, bank_id = 0, handle_id = 0;
    uint64 pkt_attr_selector_key_value;
    uint32 svc_meter_mode;

    if (0 == global_meter_status[unit].initialised) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Global meter feature not initialized\n")));
        return rv;
    }
    /* mutex destroy */
    if (NULL != global_meter_mutex[unit]) {
        sal_mutex_destroy(global_meter_mutex[unit]);
        global_meter_mutex[unit] = NULL;
    }
    /* Free handles of index management for meter action */
    if (NULL != meter_action_list_handle[unit]) {
        shr_aidxres_list_destroy(meter_action_list_handle[unit]);
        meter_action_list_handle[unit] = NULL;
    }
    /* Free handles of index management for meter table */
    num_pools = SOC_INFO(unit).global_meter_pools;
    num_banks_per_pool = get_max_banks_in_a_pool(unit);

    for (pool_id = 0; pool_id < num_pools; pool_id++) {
        for (bank_id = 0; bank_id < num_banks_per_pool; bank_id++) {
            handle_id = pool_id * num_banks_per_pool + bank_id;
            if (NULL != meter_alloc_list_handle[unit][handle_id]) {
                shr_aidxres_list_destroy(meter_alloc_list_handle[unit][handle_id]);
                meter_alloc_list_handle[unit][handle_id] = NULL;
            }
        }
    }
    if (NULL != global_meter_policer_bookkeep[unit]) {
        sal_free(global_meter_policer_bookkeep[unit]);
        global_meter_policer_bookkeep[unit] = NULL;
    }
    if (NULL != global_meter_hz_alloc_bookkeep[unit]) {
        sal_free(global_meter_hz_alloc_bookkeep[unit]);
        global_meter_hz_alloc_bookkeep[unit] = NULL;
    }
    if (NULL != global_meter_action_bookkeep[unit]) {
        sal_free(global_meter_action_bookkeep[unit]);
        global_meter_action_bookkeep[unit] = NULL;
    }

    /* global_meter_offset_mode[][].attr_selectors needs to be freed */
    for (mode = 1; mode < BCM_POLICER_SVC_METER_MAX_MODE; mode++) {
        if (global_meter_offset_mode[unit][mode].used == 1) {
            if (global_meter_offset_mode[unit][mode].attr_selectors != NULL) {
                sal_free(global_meter_offset_mode[unit][mode].attr_selectors);
                global_meter_offset_mode[unit][mode].attr_selectors = NULL;
            }
        }
    }
    if (bcm_esw_svm_control[unit] != NULL) {
        sal_free(bcm_esw_svm_control[unit]);
        bcm_esw_svm_control[unit] = NULL;
    }

    global_meter_status[unit].initialised = 0;
    SOC_IF_ERROR_RETURN(
           soc_mem_clear(unit, SVM_OFFSET_TABLEm, MEM_BLOCK_ALL, TRUE));
    SOC_IF_ERROR_RETURN(
           soc_mem_clear(unit, SVM_METER_TABLEm, MEM_BLOCK_ALL, TRUE));
    SOC_IF_ERROR_RETURN(
            soc_mem_clear(unit, SVM_MACROFLOW_INDEX_TABLEm, MEM_BLOCK_ALL, TRUE));
    SOC_IF_ERROR_RETURN(
            soc_mem_clear(unit, SVM_POLICY_TABLEm, MEM_BLOCK_ALL, TRUE));

    COMPILER_64_ZERO(pkt_attr_selector_key_value);
    for (svc_meter_mode = 1;
        svc_meter_mode < BCM_POLICER_SVC_METER_MAX_MODE;
        svc_meter_mode++) {
        SOC_IF_ERROR_RETURN(soc_reg_set(unit,
            _pkt_attr_sel_key_reg[svc_meter_mode],
            REG_PORT_ANY,
            0,
            pkt_attr_selector_key_value));
        BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
                        SVM_OFFSET_TABLEm, svc_meter_mode, NULL));
    }
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "Clening up global meter config\n")));
    return rv;
}

#ifdef BCM_APACHE_SUPPORT
static svm_dbg_01_entry_t ap_clr_tr_tbl[32] = \
{{{0x0000}}, {{0x0000}}, {{0x0000}}, {{0x0000}},
 {{0x4d22}}, {{0x4444}}, {{0x4d22}}, {{0x4d4d}},
 {{0x44e3}}, {{0x4444}}, {{0x44e3}}, {{0x44ee}},
 {{0x4d22}}, {{0x4444}}, {{0x4d22}}, {{0x4d4d}},
 {{0x4d22}}, {{0x4444}}, {{0x4d22}}, {{0x4d4d}},
 {{0x0000}}, {{0x0000}}, {{0x0000}}, {{0x0000}},
 {{0x0000}}, {{0x0000}}, {{0x0000}}, {{0x0000}},
 {{0x0000}}, {{0x0000}}, {{0x0000}}, {{0x0000}}};

static svm_dbg_02_entry_t ap_clr_hier_tr_tbl[4] = {
{{0x10290435, 0x04104104, 0x29040929}},
{{0x10274435, 0x04354104, 0x28840929}},
{{0x10090435, 0x04104104, 0x29043941}},
{{0x00000000, 0x00000000, 0x00000000}},
};

static svm_dbg_03_entry_t ap_hier_cmtd_updt_tr_tbl[32] = \
{{{0x00}}, {{0x00}}, {{0x00}}, {{0x00}}, {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}},
 {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}},
 {{0x13}}, {{0x13}}, {{0x13}}, {{0x13}}, {{0x00}}, {{0x00}}, {{0x00}}, {{0x00}},
 {{0x00}}, {{0x00}}, {{0x00}}, {{0x00}}, {{0x00}}, {{0x00}}, {{0x00}}, {{0x00}}};

/*
 * Function:
 *     _bcm_svm_dbg_clr_tr_tbls
 * Purpose:
 *    Initialize color truth table
 * Parameters:
 *     Unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
STATIC
int _bcm_svm_dbg_clr_tr_tbls (int unit) {
    int  idx = 0, rv = BCM_E_NONE, max_idx = 0;

    max_idx = soc_mem_index_max(unit,SVM_DBG_01m);
    for (idx = 0; idx < max_idx; idx++) {
        rv = WRITE_SVM_DBG_01m(unit,MEM_BLOCK_ANY,idx,&ap_clr_tr_tbl[idx]);
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
    }

    max_idx = soc_mem_index_max(unit,SVM_DBG_02m);
    for (idx = 0; idx < max_idx; idx++) {
        rv = WRITE_SVM_DBG_02m(unit,MEM_BLOCK_ANY,idx,&ap_clr_hier_tr_tbl[idx]);
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
    }

    max_idx = soc_mem_index_max(unit,SVM_DBG_03m);
    for (idx = 0; idx < max_idx; idx++) {
        rv = WRITE_SVM_DBG_03m(unit,MEM_BLOCK_ANY,idx,
                               &ap_hier_cmtd_updt_tr_tbl[idx]);
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
    }

    return rv;
}
#endif /* BCM_APACHE_SUPPORT */

/*
 * Function:
 *     _bcm_policer_svm_dev_attr_info_initialize
 * Purpose:
 *    Initialize parmas which varies across devices for
 *    global service meters
 * Parameters:
 *     Unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_policer_svm_dev_attr_info_initialize (int unit)
{
#ifdef BCM_APACHE_SUPPORT
    if (SOC_IS_APACHE(unit)) {
        BCM_SVM_DEV_ATTR(unit)->banks_per_pool = 2;
        BCM_SVM_DEV_ATTR(unit)->compressed_int_pri_bit_pos = 32;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size =
                                BCM_POLICER_SVC_METER_SB2_ING_PORT_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size =
                                BCM_POLICER_SVC_METER_SVP_NETWORK_GROUP_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->pkt_resolution = &ap_pkt_res[0];
        BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos =
                                &sb2_pkt_attr_bit_pos[0];
        BCM_SVM_DEV_ATTR(unit)->compressed_attr_map_bit_pos =
                                &sb2_pkt_cmprsd_attr_map_bit_pos[0];
    } else
#endif
#ifdef BCM_SABER2_SUPPORT
    if (SOC_IS_SABER2(unit)) {
        BCM_SVM_DEV_ATTR(unit)->banks_per_pool = 1;
        BCM_SVM_DEV_ATTR(unit)->compressed_int_pri_bit_pos = 32;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size =
                                BCM_POLICER_SVC_METER_SB2_ING_PORT_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size =
                                BCM_POLICER_SVC_METER_SVP_NETWORK_GROUP_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->pkt_resolution = &sb2_pkt_res[0];
        BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos =
                                &sb2_pkt_attr_bit_pos[0];
        BCM_SVM_DEV_ATTR(unit)->compressed_attr_map_bit_pos =
                                &sb2_pkt_cmprsd_attr_map_bit_pos[0];
    } else
#endif
#ifdef BCM_KATANA2_SUPPORT
    if (SOC_IS_KATANA2(unit)) {
        BCM_SVM_DEV_ATTR(unit)->banks_per_pool = 1;
        BCM_SVM_DEV_ATTR(unit)->compressed_int_pri_bit_pos = 33;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size =
                                BCM_POLICER_SVC_METER_KT2_ING_PORT_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size =
                                BCM_POLICER_SVC_METER_SVP_NETWORK_GROUP_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->pkt_resolution = &kt2_pkt_res[0];
        BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos =
                                &kt2_pkt_attr_bit_pos[0];
        BCM_SVM_DEV_ATTR(unit)->compressed_attr_map_bit_pos =
                                &kt2_pkt_cmprsd_attr_map_bit_pos[0];
    } else
#endif
    { /* Default. KT/TR3 */
        BCM_SVM_DEV_ATTR(unit)->banks_per_pool = 1;
        BCM_SVM_DEV_ATTR(unit)->compressed_int_pri_bit_pos =
                                kt_pkt_attr_bit_pos[pkt_attr_int_pri].start_bit;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size =
                                BCM_POLICER_SVC_METER_ING_PORT_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size =
                                BCM_POLICER_SVC_METER_SVP_TYPE_ATTR_SIZE;
        BCM_SVM_DEV_ATTR(unit)->pkt_resolution = &kt_pkt_res[0];
        BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos =
                                &kt_pkt_attr_bit_pos[0];
        BCM_SVM_DEV_ATTR(unit)->compressed_attr_map_bit_pos =
                                &kt_pkt_cmprsd_attr_map_bit_pos[0];
    }

#ifdef BCM_TRIUMPH3_SUPPORT
    if (SOC_IS_TRIUMPH3(unit)) {
        BCM_SVM_DEV_ATTR(unit)->pkt_resolution = &tr3_pkt_res[0];
    }
#endif
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_policer_svm_source_initialize
 * Purpose:
 *    Initialize service metering source from various tables
 * Parameters:
 *     Unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
STATIC bcm_error_t
_bcm_policer_svm_source_initialize(int unit)
{
    soc_mem_t table;
    _bcm_policer_svm_source_type_t mem_index;

    for (mem_index = 0; mem_index < _BCM_SVM_MEM_COUNT; mem_index++) {
        BCM_SVM_SOURCE(unit, mem_index).table = INVALIDm;
        BCM_SVM_SOURCE(unit, mem_index).offset_mode = INVALIDf;
        BCM_SVM_SOURCE(unit, mem_index).meter_index = INVALIDf;
        table = INVALIDm;
        switch(mem_index) {
            case _BCM_SVM_MEM_PORT:
                table = PORT_TABm;
                break;
            case _BCM_SVM_MEM_VLAN:
                table = VLAN_TABm;
                break;
            case _BCM_SVM_MEM_VFI:
                table = VFIm;
                break;
            case _BCM_SVM_MEM_SOURCE_VP:
                table = SOURCE_VPm;
                break;
            case _BCM_SVM_MEM_VLAN_XLATE:
                table = VLAN_XLATEm;
#ifdef BCM_TRIUMPH3_SUPPORT
                if (SOC_IS_TRIUMPH3(unit)) {
                    table = VLAN_XLATE_EXTDm;
                }
#endif
                break;
            case _BCM_SVM_MEM_VFP_POLICY_TABLE:
                table = VFP_POLICY_TABLEm;
                break;
            /* coverity[dead_error_begin : FALSE] */
            default :
                break;
        }
        BCM_SVM_SOURCE(unit, mem_index).table = table;
#ifdef BCM_TRIUMPH3_SUPPORT
        if (SOC_IS_TRIUMPH3(unit) && (mem_index == _BCM_SVM_MEM_VLAN_XLATE)) {
            if (SOC_MEM_FIELD_VALID(unit, table,
                    XLATE__SVC_METER_OFFSET_MODEf)) {
                BCM_SVM_SOURCE(unit, mem_index).offset_mode =
                        XLATE__SVC_METER_OFFSET_MODEf;
            }
            if (SOC_MEM_FIELD_VALID(unit, table,
                    XLATE__SVC_METER_INDEXf)) {
                BCM_SVM_SOURCE(unit, mem_index).meter_index =
                        XLATE__SVC_METER_INDEXf;
            }
            continue;
        }
#endif
        if (SOC_MEM_FIELD_VALID(unit, table, SVC_METER_OFFSET_MODEf)) {
            BCM_SVM_SOURCE(unit, mem_index).offset_mode =
                    SVC_METER_OFFSET_MODEf;
        }
        if (SOC_MEM_FIELD_VALID(unit, table, SVC_METER_INDEXf)) {
            BCM_SVM_SOURCE(unit, mem_index).meter_index =
                    SVC_METER_INDEXf;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_global_meter_init
 * Purpose:
 *     Initialise service meter data-structure
 * Parameters:
 *     Unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_global_meter_init(int unit)
{
    int rv = BCM_E_NONE;
    uint32 service_meter_control = 0;
    uint32 ing_svm_control = 0;
    int mode = 0, i = 0;
    uint32 num_pools = 0, size_pool = 0, total_size = 0, action_size = 0;
    uint32 num_banks_per_pool = 1, handle_id = 0, bank_id = 0;
    uint32 size_bank = 0, block_factor = 0;
    uint32 max_size_pool = 0;
    uint32 pool_id = 0, index = 0;
    soc_reg_t arb_reg;
    soc_field_t fields[2];
    uint32      vals[2] = {1,1};
    int         no_fields = 0;
#ifdef BCM_WARM_BOOT_SUPPORT
    int size = 0;
    soc_scache_handle_t scache_handle;
    uint8  *policer_state;
#endif

    /* Using driver function */
    if (soc_feature(unit, soc_feature_global_meter_v2)) {
        /* Initial all unit with default value FALSE. */
        if (FALSE == bcmi_global_meter_dev_info_initialized) {
            for (i = 0; i < BCM_MAX_NUM_UNITS; i++) {
                bcmi_global_meter_dev_info[unit] = NULL;
            }
            bcmi_global_meter_dev_info_initialized = TRUE;
        }

        /* Register driver functions */
        rv = bcmi_global_meter_dev_info_init(unit);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Device info init failed (rv = %d)\n"),
                                  rv));
        }
        return rv;
    }

    if (1 == global_meter_status[unit].initialised) {
        _bcm_esw_global_meter_cleanup(unit);
    }

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    if (!soc_property_get(unit, spn_GLOBAL_METER_CONTROL, 1)) {
        service_meter_control = BCM_POLICER_GLOBAL_METER_DISABLE;
    }
    SOC_IF_ERROR_RETURN(soc_reg32_get( unit,
        ING_SVM_CONTROLr,
        REG_PORT_ANY,
        0,
        &ing_svm_control));

    soc_reg_field_set(unit,
        ING_SVM_CONTROLr,
        &ing_svm_control,
        BLOCK_DISABLEf,
        service_meter_control);

    SOC_IF_ERROR_RETURN(soc_reg32_set(unit,
        ING_SVM_CONTROLr,
        REG_PORT_ANY,
        0,
        ing_svm_control));

    if ( BCM_POLICER_GLOBAL_METER_DISABLE == service_meter_control) {
        global_meter_status[unit].initialised = 0;
        LOG_WARN(BSL_LS_BCM_POLICER,
                 (BSL_META_U(unit,
                             "GLOBAL METER FEATURE disabled "
                             "at HW for unit %d\n"), unit));
        return rv;
    }

    _GLOBAL_METER_XGS3_ALLOC(bcm_esw_svm_control[unit],
        sizeof(_bcm_policer_global_meter_control_t),
        "global service meter control");

    rv = _bcm_policer_svm_dev_attr_info_initialize(unit);
    if (BCM_FAILURE(rv))    {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                "Chip Specific init failed\n")));
            return rv;
    }
    rv = _bcm_policer_svm_source_initialize(unit);
    if (BCM_FAILURE(rv))    {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                "SVM source init failed\n")));
            return rv;
    }

    num_pools =  SOC_INFO(unit).global_meter_pools;
    size_pool =  SOC_INFO(unit).global_meter_size_of_pool;
    max_size_pool = SOC_INFO(unit).global_meter_max_size_of_pool;
    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    size_bank = size_pool / num_banks_per_pool;
    total_size = num_pools * max_size_pool;
    if ((total_size) > (soc_mem_index_max(unit, SVM_METER_TABLEm) + 1))
    {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Number of global meters exceeding "
                              "its max value\n")));
        return BCM_E_INTERNAL;
    }

    action_size = SOC_INFO(unit).global_meter_action_size;
    if ((action_size) > (soc_mem_index_max(unit, SVM_POLICY_TABLEm) + 1))
    {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Global meter actions exceeding "
                              "its max value\n")));
        return BCM_E_INTERNAL;
    }

    /* Init the offset table 0 such that it can be used for non-offset
      based metering */
    BCM_IF_ERROR_RETURN(_bcm_policer_svc_meter_update_offset_table(unit,
                        SVM_OFFSET_TABLEm, 0, NULL));

    /* Initialize bookkeeping data structure */
    for (mode=1; mode < BCM_POLICER_SVC_METER_MAX_MODE; mode++) {
        sal_memset(&global_meter_offset_mode[unit][mode],
                     0, sizeof(bcm_policer_svc_meter_bookkeep_mode_t));
    }

     /* Allocate policer lookup hash. */
    _GLOBAL_METER_XGS3_ALLOC(global_meter_policer_bookkeep[unit],
               _GLOBAL_METER_HASH_SIZE * \
               sizeof(_global_meter_policer_control_t *), "Global meter hash");
    if (NULL == global_meter_policer_bookkeep[unit]) {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to allocate memory for policer bookkeep "
                              "data structure\n")));
        return (BCM_E_MEMORY);
    }
     /* Allocate horizontal allocation management data structure */
    _GLOBAL_METER_XGS3_ALLOC(global_meter_hz_alloc_bookkeep[unit],
                      size_pool * sizeof(_global_meter_horizontal_alloc_t),
                                            "Global meter horizontal alloc");
    if (NULL == global_meter_hz_alloc_bookkeep[unit]) {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to allocate memory for hz alloc bookkeep "
                              "data structure\n")));
        return (BCM_E_MEMORY);
    }

    for (index = 0; index < size_pool; index++) {
        global_meter_hz_alloc_bookkeep[unit][index].alloc_bit_map = 0xff;
    }

     /* Allocate meter action management data structure */
    _GLOBAL_METER_XGS3_ALLOC(global_meter_action_bookkeep[unit],
                   action_size * sizeof(bcm_policer_global_meter_action_bookkeep_t),
                                            "Global meter action alloc");
    if (NULL == global_meter_action_bookkeep[unit]) {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to allocate memory for meter action bookkeep "
                              "data structure\n")));
        return (BCM_E_MEMORY);
    }
    if (global_meter_mutex[unit] == NULL) {
        global_meter_mutex[unit] = sal_mutex_create("Global meter mutex");
        if (global_meter_mutex[unit] == NULL) {
            _bcm_esw_global_meter_cleanup(unit);
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to create mutex\n")));
            return BCM_E_MEMORY;
        }
    }

    /* Index management for meter action */
    if ( NULL != meter_action_list_handle[unit]) {
        shr_aidxres_list_destroy(meter_action_list_handle[unit]);
        meter_action_list_handle[unit] = NULL;
    }
    /* Create it */
    if (shr_aidxres_list_create(
                      &meter_action_list_handle[unit],
                      0,(action_size - 1),
                      0,(action_size - 1),
                      8,
                      "global_meter_action") != BCM_E_NONE) {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Couldn't create alligned list "
                              "for global meter action\n")));
        return BCM_E_INTERNAL;
    }
    /* Reserve the first entry in action table */
    rv = shr_aidxres_list_reserve_block(
                          meter_action_list_handle[unit],0,1);
    if (BCM_FAILURE(rv)) {
        _bcm_esw_global_meter_cleanup(unit);
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Not able to reserve first entry "
                              "in action table\n")));
        return (rv);
    }

    block_factor = _shr_popcount(size_bank - 1) - 1;

    /* Index management for policer */
    for (pool_id = 0; pool_id < num_pools; pool_id++) {
        for (bank_id = 0; bank_id < num_banks_per_pool; bank_id++) {
            handle_id = pool_id * num_banks_per_pool + bank_id;
            if ( NULL != meter_alloc_list_handle[unit][handle_id]) {
                shr_aidxres_list_destroy(
                               meter_alloc_list_handle[unit][handle_id]);
                meter_alloc_list_handle[unit][handle_id] = NULL;
            }
            /* Create it */
            if (shr_aidxres_list_create(
                  &meter_alloc_list_handle[unit][handle_id],
                  0, (size_bank - 1), 0, (size_bank - 1),
                  block_factor, "global_meter_policer") != BCM_E_NONE)
            {
                _bcm_esw_global_meter_cleanup(unit);
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Couldn't create alligned list for "
                                      "global meter policer\n")));
                return BCM_E_INTERNAL;
            }
            if (bank_id == 0) {
                /* Reserve the first entry in policer table */
                rv = shr_aidxres_list_reserve_block(
                        meter_alloc_list_handle[unit][handle_id], 0, 1);
                if (BCM_FAILURE(rv)) {
                    _bcm_esw_global_meter_cleanup(unit);
                    LOG_ERROR(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Not able to reserve first entry "
                                        "in policer table\n")));
                    return (rv);
                }
                rv = _bcm_global_meter_reserve_bloc_horizontally(unit,pool_id, 0);
                if (!BCM_SUCCESS(rv)) {
                    rv = shr_aidxres_list_free(meter_alloc_list_handle[unit]\
                            [handle_id], 0);
                    if (!BCM_SUCCESS(rv)) {
                        _bcm_esw_global_meter_cleanup(unit);
                        LOG_DEBUG(BSL_LS_BCM_POLICER,
                                (BSL_META_U(unit,
                                            "Not able to reserve first entry "
                                            "in policer table\n")));
                        return BCM_E_INTERNAL;
                    }
                }
            }
        }
    }
#ifdef BCM_APACHE_SUPPORT
    if (SOC_IS_APACHE(unit)) {
        rv = _bcm_svm_dbg_clr_tr_tbls(unit);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Failed to set color truth table\n")));
            return rv;
        }
    }
#endif /* BCM_APACHE_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        rv = _bcm_esw_global_meter_reinit(unit);
        if (!BCM_SUCCESS(rv)) {
            _bcm_esw_global_meter_cleanup(unit);
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Warm boot init failed\n")));
            return BCM_E_INTERNAL;
        }
    } else { /* cold boot */
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_POLICER, 0);
        size = sizeof(int32) * BCM_GLOBAL_METER_MAX_SCACHE_ENTRIES;
        if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_1)   {
            /* Following fields of global_meter_offset_mode needs to be saved in scache.
             * no_of_policer, group_mode and meter_attr.udf_pkt_attr_selectors_v.udf_id
             * Rest all fields can be recovered from hardware */
            size += (sizeof(uint32) * BCM_POLICER_SVC_METER_MAX_MODE * 3);
        }
        if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_2) {
            /* Now, we support non-continuous bit selection in Udf mode. So, we need
             * to store attr_selectors information for each group offset mode */
            size += (sizeof(bcm_policer_group_mode_attr_selectors_info_t) *
                                        BCM_GLOBAL_METER_MAX_SCACHEABLE_GROUP_MODE);
        }
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, TRUE,
                                     size, &policer_state,
                                     BCM_WB_DEFAULT_VERSION, NULL);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Scache Memory not available\n")));
            return rv;
        }
        rv = BCM_E_NONE;
    }
#endif
    global_meter_status[unit].initialised = 1;
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "GLOBAL METER FEATURE initialised\n")));
    /* coverity[result_independent_of_operands] */
    arb_reg = (SOC_REG_IS_VALID(unit, AUX_ARB_CONTROL_2_64r) ?
               AUX_ARB_CONTROL_2_64r : AUX_ARB_CONTROL_2r);

    if (SOC_IS_TRIUMPH3(unit)) {
        fields[0] = SVC_MTR_REFRESH_ENABLEf;
        no_fields = 1;
    } else {
        fields[0] = SVM_MASTER_REFRESH_RATEf;
        fields[1] = SVM_MASTER_REFRESH_ENABLEf;
        no_fields = 2;
    }

    SOC_IF_ERROR_RETURN(soc_reg_fields32_modify(unit,arb_reg,REG_PORT_ANY,\
                                                no_fields,fields,vals));

    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_validate
 * Purpose:
 *      Validate the given policer id
 * Parameters:
 *     Unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 * Returns:
 *     BCM_E_XXX
 */
int   _bcm_esw_policer_validate(int unit, bcm_policer_t *policer)
{
    int rv = BCM_E_NONE;
    int index = 0;
    uint32 offset_mode = 0;
    if( *policer == 0) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Policer id passed is 0 \n")));
        return rv;
    }
    _bcm_esw_get_policer_table_index(unit, *policer, &index);
    if (index > (soc_mem_index_max(unit, SVM_METER_TABLEm)))
    {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid table index for SVM_POLICY_TABLE\n")));
        return BCM_E_PARAM;
    }
    offset_mode = ((*policer & BCM_POLICER_GLOBAL_METER_MODE_MASK) >>
                                  BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
    if (offset_mode >= 1) {
        offset_mode = offset_mode - 1;
    }
    if (offset_mode > BCM_POLICER_SVC_METER_MAX_MODE)
    {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Invalid Offset mode \n")));
        return BCM_E_PARAM;

    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_increment_ref_count
 * Purpose:
 *      Increment the policer usage reference count
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 * Returns:
 *     BCM_E_XXX
 */
int  _bcm_esw_policer_increment_ref_count(int unit, bcm_policer_t policer)
{
    int rv = BCM_E_NONE;
    int offset_mode = 0;
    _global_meter_policer_control_t *policer_control = NULL;
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Invalid policer id passed: %x \n"),
                     policer));
        return (rv);
    }
    offset_mode = ((policer & BCM_POLICER_GLOBAL_METER_MODE_MASK) >>
                   BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
    if (offset_mode >= 1) {
        offset_mode = offset_mode - 1;
    }
    GLOBAL_METER_LOCK(unit);
    if ((global_meter_offset_mode[unit][offset_mode].meter_attr.mode_type_v ==
        cascade_mode) ||
        (global_meter_offset_mode[unit][offset_mode].type ==
                bcmPolicerGroupModeTypeCascade) ||
        (global_meter_offset_mode[unit][offset_mode].type ==
                bcmPolicerGroupModeTypeCascadeWithCoupling)) {
        /*
         * SDK alway allocates 8 policer IDs in cascaded mode,
         * and each policer is configurable for the purpose of
         * bandwidth sharing.
         */
        rv = _bcm_global_meter_policer_get(unit, policer, &policer_control);
    } else {
        rv = _bcm_global_meter_base_policer_get(unit, policer, &policer_control);
    }
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to get policer control for policer "
                                "id %d\n"), policer));
        return (rv);
    }
    policer_control->ref_count++;
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_decrement_ref_count
 * Purpose:
 *     Decrement the policer usage refernce count
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 * Returns:
 *     BCM_E_XXX
 */
int  _bcm_esw_policer_decrement_ref_count(int unit, bcm_policer_t policer)
{
    int rv = BCM_E_NONE;
    int offset_mode = 0;
    _global_meter_policer_control_t *policer_control = NULL;
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Invalid policer id passed: %x \n"),
                     policer));
        return (rv);
    }
    offset_mode = ((policer & BCM_POLICER_GLOBAL_METER_MODE_MASK) >>
                   BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
    if (offset_mode >= 1) {
        offset_mode = offset_mode - 1;
    }
    GLOBAL_METER_LOCK(unit);
    if ((global_meter_offset_mode[unit][offset_mode].meter_attr.mode_type_v ==
        cascade_mode) ||
        (global_meter_offset_mode[unit][offset_mode].type ==
                bcmPolicerGroupModeTypeCascade) ||
        (global_meter_offset_mode[unit][offset_mode].type ==
                bcmPolicerGroupModeTypeCascadeWithCoupling)) {
        /*
         * SDK alway allocates 8 policer IDs in cascaded mode,
         * and each policer is configurable for the purpose of
         * bandwidth sharing.
         */
        rv = _bcm_global_meter_policer_get(unit, policer, &policer_control);
    } else {
        rv = _bcm_global_meter_base_policer_get(unit, policer, &policer_control);
    }
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to get policer control for policer "
                                "id %d \n"), policer));
        return (rv);
    }
    if (policer_control->ref_count > 0) {
        policer_control->ref_count--;
    }
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_policer_svm_source_index_get
 * Purpose:
 *     Get the Enum value corresponding to memory table
 * Parameters:
 *     unit                  - (IN) unit number
 *     table                 - (IN) Memory table
 *     mem_index             - (OUT) Corresponding Enum
 *                              in _bcm_policer_svm_source_type_t
 * Returns:
 *     BCM_E_XXX
 */
bcm_error_t
_bcm_policer_svm_source_index_get(int unit, soc_mem_t table,
                                _bcm_policer_svm_source_type_t *mem_index)
{
    switch (table) {
        case PORT_TABm:
            *mem_index = _BCM_SVM_MEM_PORT;
            break;
        case VLAN_TABm:
            *mem_index = _BCM_SVM_MEM_VLAN;
            break;
        case VFIm:
            *mem_index = _BCM_SVM_MEM_VFI;
            break;
        case SOURCE_VPm:
            *mem_index = _BCM_SVM_MEM_SOURCE_VP;
            break;
#ifdef BCM_TRIUMPH3_SUPPORT
        case VLAN_XLATE_EXTDm:
            /* passthru */
            /* coverity[fallthrough: FALSE] */
#endif
        case VLAN_XLATEm:
            *mem_index = _BCM_SVM_MEM_VLAN_XLATE;
            break;
        case VFP_POLICY_TABLEm :
            *mem_index = _BCM_SVM_MEM_VFP_POLICY_TABLE;
            break;
        default :
            return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_add_policer_to_table
 * Purpose:
 *      Add policer id to the table entry specified
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 *     table                 - (IN) Table to which the entry needs to be added
 *     index                 - (IN) index in the table
 *     data                  - (IN) Data to be written
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_add_policer_to_table(int unit,bcm_policer_t policer,
                               soc_mem_t table, int index, void *data)
{
    int offset_mode = 0;
    int rv = BCM_E_NONE;
    bcm_policer_t current_policer = 0;
    _bcm_policer_svm_source_type_t mem_index = 0;
    if (soc_feature(unit, soc_feature_global_meter)) {
         /* validate policer id */
        rv = _bcm_esw_policer_validate(unit, &policer);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Invalid policer id passed: %x \n"),
                         policer));
            return (rv);
        }
        rv = _bcm_policer_svm_source_index_get(unit, table, &mem_index);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to read the table entry %d at "
                                "index %d \n"), table, index));
            return (rv);
        }
        /* updating VFP_POLICY_TABLEm is handled in field module */
        if (mem_index != _BCM_SVM_MEM_VFP_POLICY_TABLE) {
            if (BCM_SVM_SOURCE(unit, mem_index).table == INVALIDm) {
                return BCM_E_INTERNAL;
            }
            rv = _bcm_esw_get_policer_from_table(unit, table, index, data,
                    &current_policer, 1);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Unable to read the table entry %d at "
                                    "index %d \n"), table, index));
                return (rv);
            }
            if (policer == current_policer) {
                /* Nothing to do */
                return BCM_E_NONE;
            }
            /* Set policer id */
            if ((policer & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0) {
                offset_mode =
                    ((policer & BCM_POLICER_GLOBAL_METER_MODE_MASK) >>
                     BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
                if (offset_mode >= 1) {
                    offset_mode = offset_mode - 1;
                }
                if (offset_mode >=
                        (soc_mem_index_count(unit,SVM_OFFSET_TABLEm) >>
                         BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE))  {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Offset mode for the policer "
                                        "exceeds max allowed value \n")));
                    return BCM_E_PARAM;
                }
                _bcm_esw_get_policer_table_index(unit, policer, &index);
            } else {
                index = 0;
                offset_mode = 0;
            }
            if (index >= soc_mem_index_count(unit, SVM_METER_TABLEm))  {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Invalid table index passed for "
                                    "SVM_METER_TABLE\n")));
                return BCM_E_PARAM;
            }
            soc_mem_field32_set(unit,table, data,
                    BCM_SVM_SOURCE(unit, mem_index).offset_mode, offset_mode);
            soc_mem_field32_set(unit,table, data,
                    BCM_SVM_SOURCE(unit, mem_index).meter_index, index);

        }
        /* decrement current policer if any */
        if ((current_policer & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0) {
            rv = _bcm_esw_policer_decrement_ref_count(unit, current_policer);
            BCM_IF_ERROR_RETURN(rv);
        }
        /* increment new policer  */
        if ((policer & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0) {
            rv = _bcm_esw_policer_increment_ref_count(unit, policer);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_get_policer_from_table
 * Purpose:
 *      Read the policer id configured in the given table
 * Parameters:
 *     unit                  - (IN) unit number
 *     table                 - (IN) Table from which the entry needs to be got
 *     index                 - (IN) index in the table
 *     data                  - (IN) Data to be written
 *     policer               - (OUT) policer Id
 *     skip_read             - (IN) Skip reading the policer entry
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_get_policer_from_table(int unit, soc_mem_t table, int index,
                                    void *data, bcm_policer_t *policer,
                                                         int skip_read)
{
    int rv = BCM_E_NONE;
    int offset = 0;
    _bcm_policer_svm_source_type_t mem_index;
    if (soc_feature(unit, soc_feature_global_meter)) {
        
        if (!global_meter_status[unit].initialised && SOC_WARM_BOOT(unit)) {
            return rv;
        }
        if (!skip_read) {
            rv = soc_mem_read(unit, table, MEM_BLOCK_ANY, index, data);
            BCM_IF_ERROR_RETURN(rv);
        }
        if (table == SVM_MACROFLOW_INDEX_TABLEm) {
            if (SOC_MEM_FIELD_VALID(unit, table, MACROFLOW_INDEXf)) {
                index = soc_mem_field32_get(unit, table, data,
                        MACROFLOW_INDEXf);
            }
        } else {
            rv = _bcm_policer_svm_source_index_get(unit, table, &mem_index);
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                "Unable to read the table entry %d at "
                                "index %d \n"), table, index));
                return (rv);
            }
            if (BCM_SVM_SOURCE(unit, mem_index).table == INVALIDm) {
                return BCM_E_INTERNAL;
            }
            offset = soc_mem_field32_get(unit, table, data,
                        BCM_SVM_SOURCE(unit,mem_index).offset_mode);
            index = soc_mem_field32_get(unit, table, data,
                        BCM_SVM_SOURCE(unit,mem_index).meter_index);
        }
        _bcm_esw_get_policer_id_from_index_offset(unit, index,
                                                          offset, policer);
    } else {
        return BCM_E_UNAVAIL;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_reset_policer_from_table
 * Purpose:
 *      Remove the policer id configured from a table and re-adjust
 *      policer usage reference count.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 *     table                 - (IN) Table in which an entry needs to be reset
 *     index                 - (IN) index in the table
 *     data                  - (IN) Data to be written
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_reset_policer_from_table(int unit, bcm_policer_t policer,
                                      int table, int index, void *data)
{

    int rv = BCM_E_NONE;
    rv = _bcm_esw_delete_policer_from_table(unit, policer, table, index, data);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to delete policer from table %d at "
                              "index %d \n"), table, index));
        return (rv);
    }
    soc_mem_write(unit, table, MEM_BLOCK_ANY, index, data);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_delete_policer_from_table
 * Purpose:
 *      Remove the policer id configured from a table and re-adjust
 *      policer usage reference count
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer               - (IN) policer Id
 *     table                 - (IN) Table in which an entry needs to be deleted
 *     index                 - (IN) index in the table
 *     data                  - (IN) Data to be written
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_delete_policer_from_table(int unit, bcm_policer_t policer,
                                    soc_mem_t table, int index, void *data)
{
    int rv = BCM_E_NONE;
    bcm_policer_t current_policer = 0;
    int offset_mode = 0;
    _bcm_policer_svm_source_type_t mem_index;
    if (soc_feature(unit, soc_feature_global_meter)) {
         /* validate policer id */
        rv = _bcm_esw_policer_validate(unit, &policer);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid policer id passed: %x \n"),
                       policer));
            return (rv);
        }
        rv = _bcm_policer_svm_source_index_get(unit, table, &mem_index);
        if (BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to read the table entry %d at "
                                "index %d \n"), table, index));
            return (rv);
        }
        /* Deleting VFP_POLICY_TABLEm is handled in field module */
        if (mem_index != _BCM_SVM_MEM_VFP_POLICY_TABLE) {
            if (BCM_SVM_SOURCE(unit, mem_index).table == INVALIDm) {
                return BCM_E_INTERNAL;
            }
            rv =  _bcm_esw_get_policer_from_table(unit, table, index, data,
                    &current_policer, 0);
            if (BCM_FAILURE(rv)) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Unable to read the policer from table "
                                    "%d at index %d\n"), table, index));
                return (rv);
            }
            if (current_policer != policer) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Policer Id passed is different from the "
                                    "one that is configured in the table. configured value is \
                                    %d \n"), current_policer));
                return BCM_E_INTERNAL;
            }
            offset_mode = index = 0;
            soc_mem_field32_set(unit,table, data,
                    BCM_SVM_SOURCE(unit, mem_index).offset_mode, offset_mode);
            soc_mem_field32_set(unit,table, data,
                    BCM_SVM_SOURCE(unit, mem_index).meter_index, index);
        }
        /* decrement current policer if any */
        if ((policer & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0) {
            rv = _bcm_esw_policer_decrement_ref_count(unit, policer);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_get_mode_info
 * Purpose:
 *      Get the info related to a configured offset mode
 * Parameters:
 *     unit                  - (IN) unit number
 *     meter_mode_v          - (IN) Meter offset mode
 *     mode_info             - (OUT) Info of the configured mode
 * Returns:
 *     BCM_E_XXX
 */

bcm_error_t _bcm_policer_svc_meter_get_mode_info(
                int                                     unit,
                bcm_policer_svc_meter_mode_t            meter_mode_v,
                bcm_policer_svc_meter_bookkeep_mode_t   *mode_info
                )
{
    if (!((meter_mode_v >= 1) &&
                    (meter_mode_v < BCM_POLICER_SVC_METER_MAX_MODE))) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid offset mode %d  \n"), meter_mode_v));
        return BCM_E_PARAM;
    }
    if (global_meter_offset_mode[unit][meter_mode_v].used == 0) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Passed mode is not used \n")));
        return BCM_E_NOT_FOUND;
    }
    *mode_info = global_meter_offset_mode[unit][meter_mode_v];
    return BCM_E_NONE;
}


/* ACTION API's */

/*
 * Function:
 *      bcm_esw_policer_action_create
 * Purpose:
 *       Create a new action entry in meter action table
 * Parameters:
 *     unit                  - (IN) unit number
 *     action_id             - (OUT) Action Id created
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_action_create(
    int unit,
    uint32 *action_id)
{
    int rv = BCM_E_NONE;
    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    if (action_id == NULL)  {
        return (BCM_E_PARAM);
    }

    CHECK_GLOBAL_METER_INIT(unit);
    rv = shr_aidxres_list_alloc_block(
                 meter_action_list_handle[unit], 1, (uint32 *)action_id);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to allocate a handle for new "
                              "action\n")));
        return (rv);
    }
    global_meter_action_bookkeep[unit][*action_id].used = 1;
    return rv;
}

/*
 * Function:
 *     bcm_esw_policer_action_add
 * Purpose:
 *     Add an entry to meter action table
 * Parameters:
 *     unit                  - (IN) unit number
 *     action_id             - (IN) Action Id created
 *     action                - (IN) Action to be added
 *     action_id             - (IN) Parameter associated with action
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_action_add(
    int unit,
    uint32 action_id,
    bcm_policer_action_t action,
    uint32 param0)
{
    int rv = BCM_E_NONE;
    svm_policy_table_entry_t meter_action;
    uint32 green_action[1] = {0}, yellow_action[1] = {0}, red_action[1] = {0};

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    if (global_meter_action_bookkeep[unit][action_id].used != 1)
    {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Action id is not created \n")));
        return BCM_E_PARAM;
    }

    GLOBAL_METER_LOCK(unit);
    /* read action table entry */
    rv = READ_SVM_POLICY_TABLEm(unit, MEM_BLOCK_ANY, action_id, &meter_action);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM_POLICY_TABLE for given "
                              "action id \n")));
        return (rv);
    }
    soc_SVM_POLICY_TABLEm_field_get(unit, &meter_action, G_ACTIONSf,
                                                      &green_action[0]);
    soc_SVM_POLICY_TABLEm_field_get(unit, &meter_action, Y_ACTIONSf,
                                                     &yellow_action[0]);
    soc_SVM_POLICY_TABLEm_field_get(unit, &meter_action, R_ACTIONSf,
                                                        &red_action[0]);


    switch (action) {
       case bcmPolicerActionGpDrop :
           SHR_BITSET(green_action,
                              BCM_POLICER_GLOBAL_METER_ACTION_DROP_BIT_POS);
           break;

       case bcmPolicerActionYpDrop :
           SHR_BITSET(yellow_action,
                             BCM_POLICER_GLOBAL_METER_ACTION_DROP_BIT_POS);
           break;

       case bcmPolicerActionRpDrop :
           SHR_BITSET(red_action,
                            BCM_POLICER_GLOBAL_METER_ACTION_DROP_BIT_POS);
           break;

       case bcmPolicerActionGpEcnNew :
           if (param0 > 0x3) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for ECN  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(green_action,
                            BCM_POLICER_GLOBAL_METER_ACTION_ECN_BIT_POS, 2);
           green_action[0] = green_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_ECN_BIT_POS);
           SHR_BITSET(green_action,
                          BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_ECN_BIT_POS);
           break;

       case bcmPolicerActionYpEcnNew :
           if (param0 > 0x3) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for ECN  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(yellow_action,
                            BCM_POLICER_GLOBAL_METER_ACTION_ECN_BIT_POS, 2);
           yellow_action[0] = yellow_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_ECN_BIT_POS);
           SHR_BITSET(yellow_action,
                          BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_ECN_BIT_POS);
           break;

       case bcmPolicerActionRpEcnNew :
           if (param0 > 0x3) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for ECN  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(red_action,
                            BCM_POLICER_GLOBAL_METER_ACTION_ECN_BIT_POS, 2);
           red_action[0] = red_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_ECN_BIT_POS);
           SHR_BITSET(red_action,
                         BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_ECN_BIT_POS);
           break;

       case bcmPolicerActionGpCngNew :
           if (param0 > 0x3) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for CNG  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(green_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS, 2);
           green_action[0] = green_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS);
           break;

       case bcmPolicerActionYpCngNew :
           if (param0 > 0x3) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for CNG  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(yellow_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS, 2);
           yellow_action[0] = yellow_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS);
           break;

       case bcmPolicerActionRpCngNew :
           if (param0 > 0x3) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for CNG  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(red_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS, 2);
           red_action[0] = red_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS);
           break;

       case bcmPolicerActionGpDscpNew:
           if (param0 > 0x3f) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for DSCP  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(green_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_DSCP_BIT_POS, 6);
           green_action[0] = green_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_DSCP_BIT_POS);
           SHR_BITSET(green_action,
                         BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_DSCP_BIT_POS);
           break;

       case bcmPolicerActionYpDscpNew:
           if (param0 > 0x3f) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for DSCP  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(yellow_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_DSCP_BIT_POS, 6);
           yellow_action[0] = yellow_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_DSCP_BIT_POS);
           SHR_BITSET(yellow_action,
                          BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_DSCP_BIT_POS);
           break;

       case bcmPolicerActionRpDscpNew:
           if (param0 > 0x3f) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for DSCP  \n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(red_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_DSCP_BIT_POS, 6);
           red_action[0] = red_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_DSCP_BIT_POS);
           SHR_BITSET(red_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_DSCP_BIT_POS);
           break;

       case bcmPolicerActionGpPrioIntNew :
           if (param0 > 0xf) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for int pri\n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(green_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS, 4);
           green_action[0] = green_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS);
           SHR_BITSET(green_action,
                       BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_INT_PRI_BIT_POS);
           break;

       case bcmPolicerActionYpPrioIntNew :
           if (param0 > 0xf) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for int pri\n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(yellow_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS, 4);
           yellow_action[0] = yellow_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS);
           SHR_BITSET(yellow_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_INT_PRI_BIT_POS);
           break;

       case bcmPolicerActionRpPrioIntNew :
           if (param0 > 0xf) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for int pri\n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(red_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS, 4);
           red_action[0] = red_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS);
           SHR_BITSET(red_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_INT_PRI_BIT_POS);
           break;


       case bcmPolicerActionGpVlanPrioNew :
           if (param0 > 0x7) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for vlan pri\n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(green_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS, 3);
           green_action[0] = green_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS);
           SHR_BITSET(green_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_DOT1P_BIT_POS);
           break;

       case bcmPolicerActionYpVlanPrioNew :
           if (param0 > 0x7) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for vlan pri\n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(yellow_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS, 3);
           yellow_action[0] = yellow_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS);
           SHR_BITSET(yellow_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_DOT1P_BIT_POS);
           break;

       case bcmPolicerActionRpVlanPrioNew :
           if (param0 > 0x7) {
               GLOBAL_METER_UNLOCK(unit);
               LOG_ERROR(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Invalid value for vlan pri\n")));
               return BCM_E_PARAM;
           }
           SHR_BITCLR_RANGE(red_action,
                      BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS, 3);
           red_action[0] = red_action[0] |
                   (param0 << BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS);
           SHR_BITSET(red_action,
                        BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_DOT1P_BIT_POS);
           break;

       default:
          GLOBAL_METER_UNLOCK(unit);
          LOG_ERROR(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unsupported Action specified\n")));
          return BCM_E_PARAM;
    }

    soc_SVM_POLICY_TABLEm_field_set(unit, &meter_action, G_ACTIONSf,
                                                    &green_action[0]);
    soc_SVM_POLICY_TABLEm_field_set(unit, &meter_action, Y_ACTIONSf,
                                                   &yellow_action[0]);
    soc_SVM_POLICY_TABLEm_field_set(unit, &meter_action, R_ACTIONSf,
                                                      &red_action[0]);

    rv = WRITE_SVM_POLICY_TABLEm(unit,MEM_BLOCK_ANY,action_id, &meter_action);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to write to SVM_POLICY_TABLE at location "
                              "specified by action_id \n")));
        return (rv);
    }
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *     bcm_esw_policer_action_destroy
 * Purpose:
 *     Delete an entry from meter action table
 * Parameters:
 *     unit                  - (IN) unit number
 *     action_id             - (IN) Action Id to be destroyed
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_action_destroy(
    int unit,
    uint32 action_id)
{
    int rv = BCM_E_NONE;
    uint32 reset=0;
    svm_policy_table_entry_t meter_action;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    if (global_meter_action_bookkeep[unit][action_id].used != 1)
    {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Action Id specified doesn't exist\n")));
        return BCM_E_PARAM;
    }
    if (global_meter_action_bookkeep[unit]\
                                  [action_id].reference_count != 0)
    {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Action Id specified still being used \n")));
        return BCM_E_BUSY;
    }
    GLOBAL_METER_LOCK(unit);
    /* read action table entry */
    rv = READ_SVM_POLICY_TABLEm(unit, MEM_BLOCK_ANY, action_id, &meter_action);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM_POLICY_TABLE at location "
                              "specified by action_id \n")));
        return (rv);
    }

    soc_SVM_POLICY_TABLEm_field_set(unit, &meter_action, G_ACTIONSf, &reset);
    soc_SVM_POLICY_TABLEm_field_set(unit, &meter_action, Y_ACTIONSf, &reset);
    soc_SVM_POLICY_TABLEm_field_set(unit, &meter_action, R_ACTIONSf, &reset);

    rv = WRITE_SVM_POLICY_TABLEm(unit, MEM_BLOCK_ANY, action_id, &meter_action);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to write to SVM_POLICY_TABLE at location "
                              "specified by action_id \n")));
        return (rv);
    }

    rv = shr_aidxres_list_free(meter_action_list_handle[unit], action_id);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to free action handle \n")));
        return (rv);
    }
    GLOBAL_METER_UNLOCK(unit);
    global_meter_action_bookkeep[unit][action_id].used = 0;
    return rv;

}

/*
 * Function:
 *      bcm_esw_policer_action_get
 * Purpose:
 *     get action parameter for a given action
 * Parameters:
 *     unit                  - (IN) unit number
 *     action_id             - (IN) Action Id
 *     action                - (IN) Action for which param is needed
 *     param0                - (OUT) Parameter associated with action id
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_action_get(
    int unit,
    uint32 action_id,
    bcm_policer_action_t action,
    uint32   *param0)
{
    int rv = BCM_E_NONE;
    svm_policy_table_entry_t meter_action;
    uint32 green_action = 0, yellow_action = 0, red_action = 0;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    if (global_meter_action_bookkeep[unit][action_id].used != 1)
    {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Action Id specified doesn't exist\n")));
        return BCM_E_PARAM;
    }
    GLOBAL_METER_LOCK(unit);
    /* read action table entry */
    rv = READ_SVM_POLICY_TABLEm(unit, MEM_BLOCK_ANY, action_id, &meter_action);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM_POLICY_TABLE at location "
                              "specified by action_id \n")));
        return (rv);
    }
    soc_SVM_POLICY_TABLEm_field_get(unit, &meter_action, G_ACTIONSf,
                                                         &green_action);
    soc_SVM_POLICY_TABLEm_field_get(unit, &meter_action, Y_ACTIONSf,
                                                         &yellow_action);
    soc_SVM_POLICY_TABLEm_field_get(unit, &meter_action, R_ACTIONSf,
                                                           &red_action);

    switch (action) {
        case bcmPolicerActionGpEcnNew :
            *param0 = (green_action & BCM_POLICER_GLOBAL_METER_ACTION_ECN_MASK);
            break;

        case bcmPolicerActionGpDscpNew:
            *param0 = (green_action &
                            BCM_POLICER_GLOBAL_METER_ACTION_DSCP_MASK) >> 2;
            break;

        case bcmPolicerActionGpVlanPrioNew:
            *param0 = (green_action &
                              BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_MASK) >>
                              BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS;
            break;

        case bcmPolicerActionGpPrioIntNew:
            *param0 = (green_action &
                             BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_MASK) >>
                             BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS ;
            break;

       case bcmPolicerActionGpCngNew :
            *param0 = (green_action &
                             BCM_POLICER_GLOBAL_METER_ACTION_CNG_MASK) >>
                             BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS ;
            break;

        case bcmPolicerActionYpEcnNew :
            *param0 = (yellow_action &
                                  BCM_POLICER_GLOBAL_METER_ACTION_ECN_MASK);
            break;

        case bcmPolicerActionYpDscpNew:
            *param0 = (yellow_action &
                            BCM_POLICER_GLOBAL_METER_ACTION_DSCP_MASK) >> 2;
            break;

        case bcmPolicerActionYpVlanPrioNew:
            *param0 = (yellow_action &
                               BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_MASK) >>
                                 BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS;
            break;

        case bcmPolicerActionYpPrioIntNew:
            *param0 = (yellow_action &
                              BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_MASK) >>
                              BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS ;
            break;

       case bcmPolicerActionYpCngNew :
            *param0 = (yellow_action &
                             BCM_POLICER_GLOBAL_METER_ACTION_CNG_MASK) >>
                             BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS ;
            break;

        case bcmPolicerActionRpEcnNew :
            *param0 = (red_action & BCM_POLICER_GLOBAL_METER_ACTION_ECN_MASK);
            break;

        case bcmPolicerActionRpDscpNew:
            *param0 = (red_action &
                             BCM_POLICER_GLOBAL_METER_ACTION_DSCP_MASK) >> 2;
            break;

        case bcmPolicerActionRpVlanPrioNew:
            *param0 = (red_action &
                               BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_MASK) >>
                               BCM_POLICER_GLOBAL_METER_ACTION_DOT1P_BIT_POS;
            break;

        case bcmPolicerActionRpPrioIntNew:
            *param0 = (red_action &
                             BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_MASK) >>
                             BCM_POLICER_GLOBAL_METER_ACTION_INT_PRI_BIT_POS ;
            break;

       case bcmPolicerActionRpCngNew :
            *param0 = (red_action &
                             BCM_POLICER_GLOBAL_METER_ACTION_CNG_MASK) >>
                             BCM_POLICER_GLOBAL_METER_ACTION_CHANGE_CNG_BIT_POS ;
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unsupported Action specified\n")));
            rv = BCM_E_PARAM;
            break;
    }
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/* end Action API's */


/* POLICER API's */
/*
 * Function:
 *      _bcm_global_meter_free_allocated_policer_on_error
 * Purpose:
 *      Clean up the internal data structure in case of error.
 * Parameters:
 *     unit                  - (IN) unit number
 *     numbers               - (IN) Number of policer to be freed
 *     offset                - (IN) Offset w.r.t base policer
 *     policer_index         - (IN) Base policer id
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_global_meter_free_allocated_policer_on_error(int unit,
                                           uint32 numbers, uint8 *offset,
                                                  uint32 policer_index)
{

    int rv = BCM_E_NONE;
    int index    =0;
    int pool  =  0;
    int size_pool;
    int num_banks_per_pool = 1, index_bank = 0;
    int handle_id = 0, bank_id = 0;

    size_pool = SOC_INFO(unit).global_meter_max_size_of_pool;
    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    bank_id = (policer_index & (size_pool - 1)) / num_banks_per_pool;
    index_bank = (policer_index & (size_pool - 1)) % num_banks_per_pool;

    for (index = 0;index < numbers; index++) {
        if (index) {
            pool = offset[0] + offset[index];
        } else {
            pool = offset[0];
        }

        handle_id = pool * num_banks_per_pool + bank_id;
        rv = shr_aidxres_list_free(
                         meter_alloc_list_handle[unit][handle_id], index_bank);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to free policer handle \n")));
            return BCM_E_INTERNAL;
        }

        rv = _bcm_gloabl_meter_unreserve_bloc_horizontally(unit, pool, policer_index);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to free policer handle \n")));
            return BCM_E_INTERNAL;
        }
    }
    return rv;
}

static int
max_groups_per_octuple (int unit)
{
    return (soc_feature(unit, soc_feature_global_meter_mef_10dot3) ? 4 : 2);
}

/*
 * Function:
 *      _bcm_gloabl_meter_alloc_horizontally
 * Purpose:
 *      Allocate a set of policers such that each of the policers
 *      created have the same index but differnt pool number.
 * Parameters:
 *     unit                  - (IN) unit number
 *     numbers               - (IN) Number of policer to be allocated
 *     pid                   - (OUT) Base policer id
 *     offset                - (OUT) Offset w.r.t base policer
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_gloabl_meter_alloc_horizontally(int unit, int *numbers,
                                         bcm_policer_t *pid,
                                         uint8 *offset)
{
    int rv = BCM_E_NONE;
     _global_meter_horizontal_alloc_t *alloc_control;
    uint32 allocated_policer = 0;
    int max_index = 0, max_pool = 0, index = 0, map = 0, pool = 0;
    int policers_available = 0;
    int skip_index = 0;
    int bank_size = 0, handle_id = 0, bank_index = 0;
    int num_banks_per_pool = get_max_banks_in_a_pool(unit);

    max_index =  SOC_INFO(unit).global_meter_size_of_pool;
    max_pool = SOC_INFO(unit).global_meter_pools;
    bank_size = max_index / num_banks_per_pool;

    alloc_control = global_meter_hz_alloc_bookkeep[unit];
    for (index = 1; index < max_index; index++) {
        skip_index = 0;
        allocated_policer = 0;
        if (alloc_control[index].no_of_groups_allocated ==
            max_groups_per_octuple(unit)) {
            continue;
        } else if (alloc_control[index].no_of_groups_allocated > 0) {
        /* check if we have enough free policers to allocate */
            map = alloc_control[index].alloc_bit_map;
            if (alloc_control[index].first_bit_to_use > 0) {
                map = map &
                (convert_to_bitmask(alloc_control[index].first_bit_to_use - 1));
                policers_available = _shr_popcount(map);
            } else {
                policers_available = 0;
            }
            if (policers_available >= *numbers) {
                offset[0] = 0;
            /* Use shared aligned index management to allocate policers */
                for (pool = 0; pool <= alloc_control[index].first_bit_to_use;
                                                                    pool++) {
                    handle_id = pool * num_banks_per_pool + index / bank_size;
                    bank_index = index % bank_size;

                    rv = shr_aidxres_list_reserve_block(
                      meter_alloc_list_handle[unit][handle_id], bank_index, 1);
                    if (!BCM_SUCCESS(rv)) {
                        if (allocated_policer) {
                       /* free all the allocated policers */
                           rv = _bcm_global_meter_free_allocated_policer_on_error(unit,
                                              allocated_policer, offset, index);

                           skip_index = 1;
                           allocated_policer = 0;
                        }
                        continue;
                    }
                    rv = _bcm_global_meter_reserve_bloc_horizontally(unit,
                                                              pool, index);
                    if (!BCM_SUCCESS(rv)) {
                        rv = shr_aidxres_list_free(
                            meter_alloc_list_handle[unit][handle_id], bank_index);
                        if (!BCM_SUCCESS(rv)) {
                            LOG_DEBUG(BSL_LS_BCM_POLICER,
                                      (BSL_META_U(unit,
                                                  "Unable to free policer "
                                                  "handle \n")));
                            return BCM_E_INTERNAL;
                        }
                        continue;
                    }
                    if (allocated_policer) {
                        offset[allocated_policer] = pool - offset[0];
                    } else {
                        offset[allocated_policer] = pool;
                    }
                    allocated_policer++;
                    if (allocated_policer == *numbers) {
                       alloc_control[index].last_bit_to_use =
                                         alloc_control[index].first_bit_to_use;
                       alloc_control[index].first_bit_to_use = pool;
                       break;
                    }

                }
                if (allocated_policer != *numbers) {
                     /* free all the allocated policers */
                    rv = _bcm_global_meter_free_allocated_policer_on_error(
                                     unit, allocated_policer, offset, index);
                    if (skip_index == 0 ) {
                        LOG_DEBUG(BSL_LS_BCM_POLICER,
                                  (BSL_META_U(unit,
                                              "Unable to free policer "
                                              "handle \n")));
                        return BCM_E_INTERNAL;
                    }
                } else if( allocated_policer == *numbers) {
                    *pid = index;
                    alloc_control[index].no_of_groups_allocated++;
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Allocated base policer with "
                                          "index %x \n"), index));
                    return BCM_E_NONE;
                }
            }
            map = alloc_control[index].alloc_bit_map;
            map = map &
                   ~(convert_to_bitmask(alloc_control[index].last_bit_to_use));
            policers_available = _shr_popcount(map);
            if (policers_available >= *numbers) {
            /* Use shared aligned index management to allocate policers */
                for (pool = alloc_control[index].last_bit_to_use + 1;
                                              pool < max_pool; pool++) {
                    handle_id = pool * num_banks_per_pool + index / bank_size;
                    bank_index = index % bank_size;

                    rv = shr_aidxres_list_reserve_block(
                     meter_alloc_list_handle[unit][handle_id], bank_index, 1);
                    if (!BCM_SUCCESS(rv)) {
                        if (allocated_policer) {
                       /* free all the allocated policers */
                           rv = _bcm_global_meter_free_allocated_policer_on_error(unit,
                                              allocated_policer, offset, index);

                           skip_index = 1;
                           allocated_policer = 0;
                        }
                        continue;
                    }
                    rv = _bcm_global_meter_reserve_bloc_horizontally(unit,
                                                               pool, index);
                    if (!BCM_SUCCESS(rv)) {
                        rv = shr_aidxres_list_free(
                            meter_alloc_list_handle[unit][handle_id], bank_index);
                        if (!BCM_SUCCESS(rv)) {
                            LOG_DEBUG(BSL_LS_BCM_POLICER,
                                      (BSL_META_U(unit,
                                                  "Unable to free policer "
                                                  "handle \n")));
                            return BCM_E_INTERNAL;
                        }
                        continue;
                    }
                    if (allocated_policer) {
                        offset[allocated_policer] = pool - offset[0];
                    } else {
                        offset[allocated_policer] = pool;
                    }
                    allocated_policer++;
                    if (allocated_policer == *numbers) {
                       alloc_control[index].first_bit_to_use =
                                         alloc_control[index].last_bit_to_use;
                       alloc_control[index].last_bit_to_use = offset[0];
                       break;
                    }
                }
                if (allocated_policer != *numbers) {
                     /* free all the allocated policers */
                    rv = _bcm_global_meter_free_allocated_policer_on_error(
                                     unit, allocated_policer, offset, index);
                    if (skip_index == 0 ) {
                        LOG_DEBUG(BSL_LS_BCM_POLICER,
                                  (BSL_META_U(unit,
                                              "Unable to free policer "
                                              "handle \n")));
                        return BCM_E_INTERNAL;
                    }
                } else if( allocated_policer == *numbers) {
                    *pid = index;
                    alloc_control[index].no_of_groups_allocated++;
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Allocated base policer with "
                                          "index %x \n"), index));
                    return BCM_E_NONE;
                }
            }
        } else {   /* No Cascade group assigned */
            map = alloc_control[index].alloc_bit_map;
            policers_available = _shr_popcount(map);
            if (policers_available < *numbers) {
                continue;
            }
            offset[0] = 0;
            /* Use shared aligned index management to allocate policers */
            for (pool = 0; pool < max_pool; pool++) {
                handle_id = pool * num_banks_per_pool + index / bank_size;
                bank_index = index % bank_size;

                rv = shr_aidxres_list_reserve_block(
                     meter_alloc_list_handle[unit][handle_id], bank_index, 1);
                if (!BCM_SUCCESS(rv)) {
                    if (allocated_policer) {
                       /* free all the allocated policers */
                       rv = _bcm_global_meter_free_allocated_policer_on_error(unit,
                                              allocated_policer, offset, index);

                       skip_index = 1;
                       allocated_policer = 0;
                    }
                    continue;
                }
                rv = _bcm_global_meter_reserve_bloc_horizontally(unit, pool,
                                                                      index);
                if (!BCM_SUCCESS(rv)) {
                    rv = shr_aidxres_list_free(
                            meter_alloc_list_handle[unit][handle_id], bank_index);
                    if (!BCM_SUCCESS(rv)) {
                        LOG_DEBUG(BSL_LS_BCM_POLICER,
                                  (BSL_META_U(unit,
                                              "Unable to free policer "
                                              "handle \n")));
                        return BCM_E_INTERNAL;
                    }
                    continue;
                }
                if (allocated_policer == 0) {
                    alloc_control[index].first_bit_to_use = pool;
                    offset[allocated_policer]=pool;
                } else {
                    offset[allocated_policer] = pool - offset[0];
                }
                allocated_policer++;
                if (allocated_policer == *numbers) {
                   alloc_control[index].last_bit_to_use = pool;
                   break;
                }
            }
            if (allocated_policer != *numbers) {
                 /* free all the allocated policers */
                rv = _bcm_global_meter_free_allocated_policer_on_error(unit,
                                              allocated_policer, offset, index);
                if (skip_index == 0) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Unable to free policer "
                                          "handle \n")));
                    return BCM_E_INTERNAL;
                }
            } else if( allocated_policer == *numbers) {
                *pid = index;
                alloc_control[index].no_of_groups_allocated++;
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Allocated base policer with index %x \n"),
                           index));
                return BCM_E_NONE;
            }
        }
    }
    LOG_DEBUG(BSL_LS_BCM_POLICER,
              (BSL_META_U(unit,
                          "Unable to allocate policer as table is full  \n")));
    return BCM_E_FULL;
}

/*
 * Function:
 *      _bcm_global_meter_reserve_bloc_horizontally
 * Purpose:
 *      reset the bit in the inernal data structure corresponding
 *      to the policer that is being reserved.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pool_id               - (IN) polier pool number
 *     pid                   - (IN) policer id
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_global_meter_reserve_bloc_horizontally(int unit, int pool_id,
                                           bcm_policer_t pid)
{
    int rv = BCM_E_NONE;
    global_meter_hz_alloc_bookkeep[unit][pid].alloc_bit_map \
                       &= convert_to_mask(pool_id);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_get_policer_control
 * Purpose:
 *     Get the policer control data sructure for a given pool, offset mode
 *     and index in the pool.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pid                   - (IN) policer id
 *     pool                  - (IN) polier pool number
 *     offset_mode           - (IN) meter offset mode
 *     policer_control       - (OUT) polier control info
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_esw_get_policer_control(int unit, bcm_policer_t pid,
                                 int pool, int offset_mode,
              _global_meter_policer_control_t **policer_control)
{
    int rv = BCM_E_NONE;
    bcm_policer_t policer_id = 0;
    int size_pool = 0;
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    policer_id = (((offset_mode + 1) << BCM_POLICER_GLOBAL_METER_MODE_SHIFT) +
                    (pool << _shr_popcount(size_pool - 1)) + pid);
    rv = _bcm_global_meter_policer_get(unit, policer_id, policer_control);
    return rv;
}

/*
 * Function:
 *      _bcm_gloabl_meter_free_horizontally
 * Purpose:
 *      Free the bits in the inernal data structure corresponding
 *      to set of policer that is being destroyed.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pool_id               - (IN) polier pool number
 *     pid                   - (IN) policer id
 *     numbers               - (IN) Number of policers to be freed
 *     offset                - (IN) offset w.r.t pid
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_gloabl_meter_free_horizontally(int unit, int pool_id,
                                         bcm_policer_t pid,
                                         int numbers, uint8 offset[8])
{
    int rv = BCM_E_NONE;
    int index = 0;
     _global_meter_horizontal_alloc_t *alloc_control;
    int num_pol = 0;
    int actual_pool = 0;
    _global_meter_policer_control_t *policer_control = NULL;
    alloc_control = global_meter_hz_alloc_bookkeep[unit];
    actual_pool = pool_id;
    for (index = 0; index < numbers; index++)
    {
        if (index > 0) {
           actual_pool = pool_id + offset[index];
        }
        alloc_control[pid].alloc_bit_map |=
                             ((~convert_to_mask(actual_pool)) & 0xFF);
    }
    if ((numbers > 1) && (alloc_control[pid].no_of_groups_allocated)) {
        alloc_control[pid].no_of_groups_allocated--;
        if (alloc_control[pid].no_of_groups_allocated == 1) {
            if (pool_id + offset[index - 1] ==
                                 alloc_control[pid].first_bit_to_use) {
                rv = _bcm_esw_get_policer_control(unit, pid,
                                     alloc_control[pid].last_bit_to_use,
                                         0, &policer_control);
                if (!BCM_SUCCESS(rv)) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Unable to get policer control for pid "
                                          "%x\n"), pid));
                    return rv;
                }
                alloc_control[pid].first_bit_to_use =
                                        alloc_control[pid].last_bit_to_use;
                num_pol = policer_control->no_of_policers;
                if (num_pol > 0 ) {
                    pool_id = policer_control->offset[0] +
                          policer_control->offset[num_pol-1];
                    alloc_control[pid].last_bit_to_use = pool_id;
                } else {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Number of policers in policer control "
                                          "structure is 0\n")));
                    return BCM_E_INTERNAL;
                }
            } else if (pool_id == alloc_control[pid].last_bit_to_use) {
                alloc_control[pid].last_bit_to_use =
                                       alloc_control[pid].first_bit_to_use;
                rv = _bcm_esw_get_policer_control(unit, pid,
                                     alloc_control[pid].first_bit_to_use,
                                         0, &policer_control);
                if (!BCM_SUCCESS(rv)) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Unable to get policer control for pid "
                                          "%x\n"), pid));
                    return rv;
                }
                num_pol = policer_control->no_of_policers;
                if (num_pol > 0 ) {
                    pool_id = policer_control->offset[0];
                    alloc_control[pid].first_bit_to_use = pool_id;
                } else {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Number of policers in policer control "
                                          "structure is 0\n")));
                    return BCM_E_INTERNAL;
                }
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_gloabl_meter_unreserve_bloc_horizontally
 * Purpose:
 *      Free the bit in the inernal data structure corresponding
 *      to the policer that is being destroyed.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pool_id               - (IN) polier pool number
 *     pid                   - (IN) policer id
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_gloabl_meter_unreserve_bloc_horizontally(int unit, int pool_id,
                                           bcm_policer_t pid)
{
    int rv = BCM_E_NONE;

    global_meter_hz_alloc_bookkeep[unit][pid].alloc_bit_map \
        |= (~convert_to_mask(pool_id) & 0xFF);
    return rv;
}

/*
 * Function:
 *      _global_meter_reserve_policer_id
 * Purpose:
 *      Reserve a given policer id.
 * Parameters:
 *     unit                  - (IN) unit number
 *     direction             - (IN) Horizontal/Vertical
 *     numbers               - (IN) Number of policers to be reserved
 *     pid                   - (IN) base policer id
 *     pid_offset            - (IN) Offset w.r.t base policer
 * Returns:
 * Returns:
 * Returns:
 *     BCM_E_XXX
 */
int _global_meter_reserve_policer_id(int unit, int direction, int numbers,
                                             bcm_policer_t pid,
                                             uint8 *pid_offset)
{
    int rv = BCM_E_NONE;
    uint32 index = 0, free_index = 0;
    int pool = 0, pol_index = 0, pool_id = 0;
    int offset_mask = 0;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    int bank_size  = 0, bank_index = 0, num_banks_per_pool = 1;
    int handle_id = 0;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;

    pool = (pid & pool_mask) >> pool_offset;
    pol_index = pid & offset_mask;

    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    bank_size = size_pool / num_banks_per_pool;
    handle_id = pool * num_banks_per_pool + pol_index / bank_size;
    bank_index = pol_index % bank_size;

    if (direction == GLOBAL_METER_ALLOC_VERTICAL) {
        rv = shr_aidxres_list_reserve_block(
                    meter_alloc_list_handle[unit][handle_id], bank_index, numbers);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to reserve policer in shared index "
                                  "management\n")));
            return BCM_E_INTERNAL;
        }
        for (index = 0; index < numbers; index++) {
            rv = _bcm_global_meter_reserve_bloc_horizontally(unit, pool,
                                                            pol_index + index);
            if (!BCM_SUCCESS(rv)) {
                rv = shr_aidxres_list_free(
                     meter_alloc_list_handle[unit][handle_id], bank_index);
                if (!BCM_SUCCESS(rv)) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Unable to free policer in shared "
                                          "index management\n")));
                    return BCM_E_INTERNAL;
                }
                for (free_index = 0; free_index < index; free_index++) {
                    rv = _bcm_gloabl_meter_unreserve_bloc_horizontally(unit,
                                                      pool, pol_index+index);
                    if (!BCM_SUCCESS(rv)) {
                        LOG_DEBUG(BSL_LS_BCM_POLICER,
                                  (BSL_META_U(unit,
                                              "Unable to free policer in hz "
                                              "index management\n")));
                        return BCM_E_INTERNAL;
                    }
                }
            }
        }

    } else if (direction == GLOBAL_METER_ALLOC_HORIZONTAL) {
        for (index = 0; index < numbers; index++) {
            if (index > 0) {
                pool_id = pool + pid_offset[index];
            }
            handle_id = pool_id * num_banks_per_pool + pol_index / bank_size;
            bank_index = pol_index % bank_size;

            rv = shr_aidxres_list_reserve_block(
                   meter_alloc_list_handle[unit][handle_id], bank_index, 1);
            if (!BCM_SUCCESS(rv)) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Unable to reserve policer in shared "
                                      "index management\n")));
                return BCM_E_INTERNAL;
            }
            rv = _bcm_global_meter_reserve_bloc_horizontally(unit,
                             pool_id, pol_index);
            if (!BCM_SUCCESS(rv)) {
                rv = shr_aidxres_list_free(
                        meter_alloc_list_handle[unit][handle_id], bank_index);
                if (!BCM_SUCCESS(rv)) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                              (BSL_META_U(unit,
                                          "Unable to free policer in "
                                          "index management\n")));
                    return BCM_E_INTERNAL;
                }
                for (free_index=0; free_index < index; free_index++) {
                    rv = _bcm_gloabl_meter_unreserve_bloc_horizontally(unit,
                                                     pool_id, pol_index);
                    if (!BCM_SUCCESS(rv)) {
                        LOG_DEBUG(BSL_LS_BCM_POLICER,
                                  (BSL_META_U(unit,
                                              "Unable to free policer in hz "
                                              "index management\n")));
                        return BCM_E_INTERNAL;
                    }
                }
            }
        }
    } else {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid direction for policer allocation \n")));
        return BCM_E_INTERNAL;
    }
    return rv;
}

/*
 * Function:
 *      _global_meter_policer_id_alloc
 * Purpose:
 *     Allocate a new policer.
 * Parameters:
 *     unit                  - (IN) unit number
 *     direction             - (IN) Horizontal/Vertical
 *     numbers               - (IN/OUT) Number of policers
 *     pid                   - (OUT) base policer id
 *     flow_info             - (IN) information about flow type and skip pool
 *     pid_offset            - (OUT) Offset w.r.t base policer
 * Returns:
 *     BCM_E_XXX
 */
int _global_meter_policer_id_alloc(int unit, int direction, int *numbers,
                                    bcm_policer_t *pid,
                                    _bcm_policer_flow_info_t *flow_info,
                                    uint8 *pid_offset)
{
    int rv = BCM_E_NONE;
    uint32 index = 0, no_of_pools_checked = 0;
    static uint32 pool = 0;
    int size_pool = 0, num_pools = 0;
    int bank_size = 0, bank_index = 0;
    int meter_allocated = 0;
    _bcm_policer_flow_type_t flow_type;
    int skip_pool, skip_bank, bank_id = 0;
    int num_banks_per_pool = 1, handle_id = 0;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;

    flow_type = flow_info->flow_type;
    skip_pool = flow_info->skip_pool;
    skip_bank = flow_info->skip_bank;

    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    bank_size = size_pool / num_banks_per_pool;

    if (direction == GLOBAL_METER_ALLOC_VERTICAL) {
        if ((flow_type == bcmPolicerFlowTypeMacro) || (flow_type == bcmPolicerFlowTypeNormal)) {
            no_of_pools_checked = 0;
            for ( ;no_of_pools_checked < num_pools; pool++) {
                if (pool >= num_pools) {
                    pool = 0;
                }
                for (bank_id = 0; bank_id < num_banks_per_pool; bank_id++) {

                    /* For Macro flow use only one bank */
                    if (flow_type == bcmPolicerFlowTypeMacro) {
                        if (bank_id > 0) {
                            break;
                        }
                    }
                    handle_id = pool * num_banks_per_pool + bank_id;
                    rv = shr_aidxres_list_alloc_block(
                            (meter_alloc_list_handle[unit][handle_id]),
                            *numbers, (uint32 *)&bank_index);

                    if (BCM_SUCCESS(rv)) {
                        meter_allocated = 1;
                        (*pid) = bank_size * bank_id + bank_index;
                        break;
                    }
                }
                if (meter_allocated == 1) {
                    break;
                }
                no_of_pools_checked++;
            }
        } else {
            /* flow_type == bcmPolicerFlowTypeMicro */
            if (soc_feature(unit, soc_feature_global_meter_macro_micro_same_pool)) {
                /* If this soc feature is defined, it means that micro and macro
                 * meter must reside in same pool but different bank. A pool is
                 * divided into 2 banks.*/

                for (bank_id = 0; bank_id < num_banks_per_pool; bank_id++) {
                    if (skip_bank == bank_id) {
                        continue;
                    }

                    /* micro flow must be in the same pool as Macro flow (skip_pool) */
                    pool = skip_pool;
                    handle_id = skip_pool * num_banks_per_pool + bank_id;
                    rv = shr_aidxres_list_alloc_block(
                            (meter_alloc_list_handle[unit][handle_id]),
                            *numbers, (uint32 *)&bank_index);

                    if (BCM_SUCCESS(rv)) {
                        meter_allocated = 1;
                        (*pid) = bank_size * bank_id + bank_index;
                        break;
                    }
                }
            } else {
                /* macro-micro meter must reside in different pools */
                no_of_pools_checked = 0;
                for ( ;no_of_pools_checked < num_pools; pool++) {
                    for (bank_id =0; bank_id < num_banks_per_pool; bank_id++) {

                        if (pool >= num_pools) {
                            pool = 0;
                        }

                        if (pool == skip_pool) {
                            no_of_pools_checked++;
                            continue;
                        }

                        handle_id = pool * num_banks_per_pool + bank_id;
                        rv = shr_aidxres_list_alloc_block(
                                (meter_alloc_list_handle[unit][handle_id]),
                                *numbers, (uint32 *)&bank_index);

                        if (BCM_SUCCESS(rv)) {
                            meter_allocated = 1;
                            (*pid) = bank_size * bank_id + bank_index;
                            break;
                        }
                    }
                    if (meter_allocated == 1) {
                        break;
                    }
                    no_of_pools_checked++;
                }
            }
        }

        if (meter_allocated == 0) {
            return BCM_E_FULL;
        } else {
            /* add code to set bits in horizontal allocation datastruture */
            for (index = 0; index < *numbers; index++) {
                rv = _bcm_global_meter_reserve_bloc_horizontally(unit, pool,
                        ((*pid)+index));
                if (!BCM_SUCCESS(rv)) {
                    for (index = 0; index < *numbers; index++) {
                        rv = shr_aidxres_list_free(
                                meter_alloc_list_handle[unit][handle_id],
                                bank_index + index);
                    }
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Unable to free policer in "
                                        "index management\n")));
                    return BCM_E_INTERNAL;
                }
            }
            /* Add pool id to make complete policer id */
            *pid = *pid | (pool << _shr_popcount(size_pool - 1));
            pool++;
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "allocated policer with pid %x \n"),
                     *pid));

            return rv;
        }
    } else if (direction == GLOBAL_METER_ALLOC_HORIZONTAL) {

        /* alloc horizontally */
        rv = _bcm_gloabl_meter_alloc_horizontally(unit, numbers, pid,
                pid_offset);
        if (!BCM_SUCCESS(rv)) {
            return BCM_E_FULL;
        }
        *pid |= (*pid_offset << _shr_popcount(size_pool - 1));
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "allocated policer with pid %x \n"),
                 *pid));
    }
    return rv;
}
/*
 * Function:
 *      _bcm_global_meter_max_rate_set
 * Purpose:
 *      get CIR/EIR max value.
 * Parameters:
 *     rate                - (IN) CIR/EIR
 *     refreshmax          - (IN) refresh_max value
 *     max_rate            - (OUT) CIR/EIR max value
 * Returns:
 *     BCM_E_XXX
 */
void _bcm_global_meter_max_rate_set(uint32 rate,
                                   uint32 refreshmax, uint32 *max_rate) {
    if (rate == 0) {
        *max_rate = 0;
    }
    if (refreshmax == GLOBAL_METER_REFRESH_MAX_DISABLE_LIMIT) {
        *max_rate = 0xffffffff;
    } else {
        *max_rate = (rate * refresh_max[refreshmax]) / 100;
    }
}

/*
 * Function:
 *      _bcm_global_meter_read_config_from_hw
 * Purpose:
 *       Read policer configuration form the HW table entry.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     pol_cfg               - (OUT) policer configuration
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_global_meter_read_config_from_hw(int unit,
                                          bcm_policer_t policer_id,
                                          bcm_policer_config_t *pol_cfg)
{
#if defined(BCM_KATANA_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
    svm_meter_table_entry_t data;
    int rv = BCM_E_NONE;
    uint32    refresh_rate = 0;    /* Policer refresh rate.    */
    uint32    granularity = 0;     /* Policer granularity.     */
    uint32  mode = 0, mode_modifier = 0, coupling = 0;
    uint32 bucket_count = 0, bucket_size = 0;
    int index = 0, cascade_index = 0;
    uint32 refreshmax = 0;
    uint32    chain = 0;
    uint32 pkt_bytes = 0;
    _global_meter_policer_control_t *policer_control = NULL;
     uint32 flags=0;

    /* read HW register */
    _bcm_esw_get_policer_table_index(unit, policer_id, &index);
    rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,index, &data);
    if (!BCM_SUCCESS(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM METER TABLE at index %d "
                              "\n"), index));
        return rv;
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COUPLING_FLAGf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, COUPLING_FLAGf, &coupling);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODIFIERf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, METER_MODIFIERf,
                                                     &mode_modifier);
        if (mode_modifier == 0) {
            pol_cfg->flags = BCM_POLICER_COLOR_BLIND;
        } else {
            pol_cfg->flags = 0; /* color aware */
        }
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, PKT_BYTESf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, PKT_BYTESf,
                                                     &pkt_bytes);
        if (pkt_bytes == 0) {
            pol_cfg->flags |= BCM_POLICER_MODE_BYTES;
        } else {
            pol_cfg->flags |= BCM_POLICER_MODE_PACKETS;
        }

    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, METER_MODEf, &mode);
        switch (mode) {

            case GLOBAL_METER_MODE_DEFAULT:
                if (mode_modifier) {
                    pol_cfg->mode = bcmPolicerModePassThrough;
                } else {
                    pol_cfg->mode = bcmPolicerModeGreen;
                }
                break;

            case GLOBAL_METER_MODE_SR_TCM:
                pol_cfg->mode = bcmPolicerModeSrTcm;
                break;

            case GLOBAL_METER_MODE_SR_TCM_MODIFIED:
                pol_cfg->mode = bcmPolicerModeSrTcmModified;
                break;

            case GLOBAL_METER_MODE_TR_TCM:
                pol_cfg->mode = bcmPolicerModeTrTcm;
                break;

            case GLOBAL_METER_MODE_TR_TCM_MODIFIED:
                pol_cfg->mode = bcmPolicerModeTrTcmDs;
                if (coupling) {
                   pol_cfg->mode = bcmPolicerModeCoupledTrTcmDs;
                }
                break;

            case GLOBAL_METER_MODE_CASCADE:
                pol_cfg->mode = bcmPolicerModeCascade;
                if (coupling) {
                   pol_cfg->mode = bcmPolicerModeCoupledCascade;
                }
                break;

            default:
                break;
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COMMITTED_BUCKETCOUNTf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_BUCKETCOUNTf,
                                                           &bucket_count);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COMMITTED_REFRESHCOUNTf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_REFRESHCOUNTf,
                                                           &refresh_rate);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COMMITTED_BUCKETSIZEf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_BUCKETSIZEf,
                                                             &bucket_size);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_GRANf)) {
       soc_SVM_METER_TABLEm_field_get(unit, &data, METER_GRANf,&granularity);
    }
    soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_REFRESHMAXf,
                                                               &refreshmax);
    flags =  _BCM_XGS_METER_FLAG_GRANULARITY |
                        _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER;

    if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
        flags |= _BCM_XGS_METER_FLAG_PACKET_MODE;
    }
    if (pol_cfg->mode == bcmPolicerModeCoupledCascade) {
        rv = _bcm_xgs_bucket_encoding_to_kbits
            (refresh_rate, bucket_size, granularity,
                    flags,
             &pol_cfg->pkbits_sec, &pol_cfg->pkbits_burst);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to translate rate in kbps to bucket size "
                                  "and granularity \n")));
            return rv;
        }
        _bcm_global_meter_max_rate_set(pol_cfg->pkbits_sec,
                                        refreshmax, &pol_cfg->max_pkbits_sec);
    } else {
        rv = _bcm_xgs_bucket_encoding_to_kbits
            (refresh_rate, bucket_size, granularity,
                    flags,
             &pol_cfg->ckbits_sec, &pol_cfg->ckbits_burst);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to translate rate in kbps to bucket size "
                                  "and granularity \n")));
            return rv;
       }
        _bcm_global_meter_max_rate_set(pol_cfg->ckbits_sec,
                                        refreshmax, &pol_cfg->max_ckbits_sec);
    }

    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_SHARING_MODEf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data,
                         METER_SHARING_MODEf, &pol_cfg->sharing_mode);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, POLICY_TABLE_INDEXf)) {
        soc_SVM_METER_TABLEm_field_get(unit,
              &data, POLICY_TABLE_INDEXf, &pol_cfg->action_id);
    }

    if (mode == GLOBAL_METER_MODE_CASCADE) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, START_OF_CHAINf, &chain);
        if (chain) {
            pol_cfg->flags |= BCM_POLICER_BW_SHARING_GROUP_START;
        }
        soc_SVM_METER_TABLEm_field_get(unit, &data, END_OF_CHAINf, &chain);
        if (chain) {
            pol_cfg->flags |= BCM_POLICER_BW_SHARING_GROUP_END;
        }
    }

    if (pol_cfg->mode == bcmPolicerModeCoupledCascade) {
        rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to get policer control for the policer Id "
                                  "passed  \n")));
            return (rv);
        }

        rv =_bcm_global_meter_get_coupled_cascade_policer_index(unit,
                                    policer_id, policer_control, &cascade_index);
        BCM_IF_ERROR_RETURN(rv);
        rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                                      cascade_index, &data);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read SVM METER TABLE at index %d "
                                  "\n"), cascade_index));
           return rv;
        }

        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_REFRESHMAXf,
                                                               &refreshmax);
        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_BUCKETCOUNTf,
                                                        &bucket_count);
        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_REFRESHCOUNTf,
                                                        &refresh_rate);
        soc_SVM_METER_TABLEm_field_get(unit, &data, COMMITTED_BUCKETSIZEf,
                                                        &bucket_size);
        rv = _bcm_xgs_bucket_encoding_to_kbits
            (refresh_rate, bucket_size, granularity,
             _BCM_XGS_METER_FLAG_GRANULARITY |
                  _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER,
             &pol_cfg->ckbits_sec, &pol_cfg->ckbits_burst);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to translate rate in kbps to bucket size "
                                  "and granularity \n")));
            return rv;
        }
        _bcm_global_meter_max_rate_set(pol_cfg->ckbits_sec,
                                        refreshmax, &pol_cfg->max_ckbits_sec);
    } else {
        soc_SVM_METER_TABLEm_field_get(unit, &data, EXCESS_REFRESHMAXf,
                                                               &refreshmax);
        soc_SVM_METER_TABLEm_field_get(unit, &data, EXCESS_BUCKETCOUNTf,
                                                        &bucket_count);
        soc_SVM_METER_TABLEm_field_get(unit, &data, EXCESS_REFRESHCOUNTf,
                                                        &refresh_rate);
        soc_SVM_METER_TABLEm_field_get(unit, &data, EXCESS_BUCKETSIZEf,
                                                        &bucket_size);
        rv = _bcm_xgs_bucket_encoding_to_kbits
            (refresh_rate, bucket_size, granularity,
             _BCM_XGS_METER_FLAG_GRANULARITY |
                  _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER,
             &pol_cfg->pkbits_sec, &pol_cfg->pkbits_burst);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to translate rate in kbps to bucket size "
                                  "and granularity \n")));
            return rv;
        }
        _bcm_global_meter_max_rate_set(pol_cfg->pkbits_sec,
                                        refreshmax, &pol_cfg->max_pkbits_sec);
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

#if defined(BCM_APACHE_SUPPORT)
STATIC int
_bcm_global_meter_get_pool_from_policerId (int unit,
                                           bcm_policer_t policer_id)
{
    int size_pool = 0, num_pools = 0;
    int offset_mask = 0;
    int pool_mask = 0, pool_offset = 0;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;

    return ((policer_id & pool_mask) >> pool_offset);
}
#endif

/*
 * Function:
 *      _bcm_global_meter_read_config_from_hw_mef_10dot3
 * Purpose:
 *       Read policer configuration form the HW table entry.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     pol_cfg               - (OUT) policer configuration
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_global_meter_read_config_from_hw_mef_10dot3 (int unit,
                                                  bcm_policer_t policer_id,
                                                  bcm_policer_config_t *pol_cfg)
{
#if defined(BCM_APACHE_SUPPORT)
    svm_meter_table_entry_t svm_entry;
    int                     rv = BCM_E_NONE;
    uint32                  refresh_rate = 0;    /* Policer refresh rate.    */
    uint32                  granularity = 0;     /* Policer granularity.     */
    uint32                  mode = 0, mode_modifier = 0, coupling = 0;
    uint32                  bucket_count = 0, bucket_size = 0;
    int                     index = 0, cascade_index = 0;
    uint32                  refreshmax = 0, c_e = 0;
    uint32                  pkt_bytes = 0, flags = 0;
    uint32                  nxt_c = 0, nxt_e = 0, pair_e = 0;
    uint32                  nxt_c_valid = 0, nxt_e_valid = 0, pair_e_valid = 0;
    int                     gcf_mode = 0;
    bcm_policer_t           corres_policer = 0;
    _global_meter_policer_control_t *policer_control = NULL;

    rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to get policer control for the policer Id "
                              "passed  \n")));
        return (rv);
    }

    /* read HW register */
    _bcm_esw_get_policer_table_index(unit, policer_id, &index);
    rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,index, &svm_entry);
    if (!BCM_SUCCESS(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM METER TABLE at index %d "
                              "\n"), index));
        return rv;
    }

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, COUPLING_FLAGf, &coupling);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, METER_MODIFIERf,
                                   &mode_modifier);
    if (mode_modifier == 0) {
        pol_cfg->flags = BCM_POLICER_COLOR_BLIND;
    } else {
        pol_cfg->flags = 0; /* color aware */
    }

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, PKT_BYTESf, &pkt_bytes);
    if (pkt_bytes == 0) {
        pol_cfg->flags |= BCM_POLICER_MODE_BYTES;
    } else {
        pol_cfg->flags |= BCM_POLICER_MODE_PACKETS;
    }

    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, METER_MODEf, &mode);
        switch (mode) {

            case GLOBAL_METER_MODE_DEFAULT:
                if (mode_modifier) {
                    pol_cfg->mode = bcmPolicerModePassThrough;
                } else {
                    pol_cfg->mode = bcmPolicerModeGreen;
                }
                break;

            case GLOBAL_METER_MODE_SR_TCM:
                pol_cfg->mode = bcmPolicerModeSrTcm;
                break;

            case GLOBAL_METER_MODE_SR_TCM_MODIFIED:
                pol_cfg->mode = bcmPolicerModeSrTcmModified;
                break;

            case GLOBAL_METER_MODE_TR_TCM:
                pol_cfg->mode = bcmPolicerModeTrTcm;
                break;

            case GLOBAL_METER_MODE_TR_TCM_MODIFIED:
                pol_cfg->mode = bcmPolicerModeTrTcmDs;
                if (coupling) {
                   pol_cfg->mode = bcmPolicerModeCoupledTrTcmDs;
                }
                break;

            case GLOBAL_METER_MODE_CASCADE:
                pol_cfg->mode = bcmPolicerModeCascade;
                if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm,
                                        METER_PAIR_EXCESS_BUCKET_VALIDf)) {
                    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                                   METER_PAIR_EXCESS_BUCKET_VALIDf,
                                                   &pair_e_valid);
                }
                if (pair_e_valid == 1) {
                    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm,
                                            METER_PAIR_EXCESS_BUCKETf)) {
                        soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                                       METER_PAIR_EXCESS_BUCKETf,
                                                       &pair_e);
                    }

                    gcf_mode = 0;
                    if ((policer_control->grp_mode == bcmPolicerGroupModeCascade) ||
                            (policer_control->grp_mode == bcmPolicerGroupModeIntPriCascade)) {
                        /* Group is non-gcf mode */
                        gcf_mode = 0;
                        soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                        COMMITTED_EXCESSf, &c_e);
                        if (c_e == 1) {
                            /* this policer is coupled cascade */
                            pol_cfg->mode = bcmPolicerModeCoupledCascade;
                        }
                    } else if ((policer_control->grp_mode == bcmPolicerGroupModeCascadeWithCoupling) ||
                            (policer_control->grp_mode == bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
                        /* Group is GCF mode */
                        gcf_mode = 1;
                        pol_cfg->mode = bcmPolicerModeCoupledCascade;
                        soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                        COMMITTED_EXCESSf, &c_e);
                    }
                }
                break;

            default:
                break;
        }
    }

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, COMMITTED_BUCKETCOUNTf,
                                   &bucket_count);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, COMMITTED_REFRESHCOUNTf,
                                   &refresh_rate);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, COMMITTED_BUCKETSIZEf,
                                   &bucket_size);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, METER_GRANf,&granularity);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, COMMITTED_REFRESHMAXf,
                                   &refreshmax);

    flags =  _BCM_XGS_METER_FLAG_GRANULARITY |
             _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER;
    if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
        flags |= _BCM_XGS_METER_FLAG_PACKET_MODE;
    }

    rv = _bcm_xgs_bucket_encoding_to_kbits(refresh_rate, bucket_size,
                                           granularity, flags,
                                           &pol_cfg->ckbits_sec,
                                           &pol_cfg->ckbits_burst);
    if (!BCM_SUCCESS(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to translate rate in kbps to bucket size "
                              "and granularity \n")));
        return rv;
    }
    _bcm_global_meter_max_rate_set(pol_cfg->ckbits_sec,
                                   refreshmax, &pol_cfg->max_ckbits_sec);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                   METER_SHARING_MODEf,
                                   &pol_cfg->sharing_mode);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, POLICY_TABLE_INDEXf,
                                   &pol_cfg->action_id);

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                   NXT_EXCESS_BUCKET_VALIDf,
                                   &nxt_e_valid);
    if ((gcf_mode == 0) && (nxt_e_valid == 1)) {
        soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                       NXT_EXCESS_BUCKETf,
                                       &nxt_e);
        rv = _bcm_esw_get_corres_policer_for_pool(unit,  policer_id, nxt_e, &corres_policer);
        BCM_IF_ERROR_RETURN(rv);
        pol_cfg->npoflow_policer_id = corres_policer;
    }

    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                   NXT_COMMITTED_BUCKET_VALIDf,
                                   &nxt_c_valid);
    if ((gcf_mode == 0) && (nxt_c_valid == 1)) {
        soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                       NXT_COMMITTED_BUCKETf,
                                       &nxt_c);
        rv = _bcm_esw_get_corres_policer_for_pool(unit,  policer_id, nxt_c, &corres_policer);
        BCM_IF_ERROR_RETURN(rv);
        pol_cfg->ncoflow_policer_id = corres_policer;
    }

    if (pol_cfg->mode == bcmPolicerModeCoupledCascade) {
        if (gcf_mode == 1) {
            /* Get the next Committed bucket / Excess bucket */
            if (c_e == 0) {
                /* One of the pools from 0-2. Next Commit pool is set in nxt_c
                   and Next Excess pool will be same */
                if (nxt_c_valid == 1) {
                    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry,
                                    NXT_COMMITTED_BUCKETf,
                                    &nxt_c);
                    rv = _bcm_esw_get_corres_policer_for_pool(unit,  policer_id, nxt_c, &corres_policer);
                    BCM_IF_ERROR_RETURN(rv);
                    pol_cfg->ncoflow_policer_id = corres_policer;
                    pol_cfg->npoflow_policer_id = corres_policer;
                }
            }
            rv = _bcm_global_meter_get_coupled_cascade_policer_index(unit,
                                    policer_id, policer_control, &cascade_index);
        } else {
            /* Group is configured as cascade but this policer mode is coupled cascade */
            /* Excess bucket of this policer is stored in pool of nxt_c */
            rv = _bcm_esw_get_corres_policer_for_pool(unit,  policer_id, nxt_c, &corres_policer);
            BCM_IF_ERROR_RETURN(rv);
            rv = _bcm_esw_get_policer_table_index(unit, corres_policer, &cascade_index);
        }
        BCM_IF_ERROR_RETURN(rv);
        rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                          cascade_index, &svm_entry);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read SVM METER TABLE at index %d "
                                  "\n"), cascade_index));
           return rv;
        }
    }
    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, EXCESS_REFRESHMAXf,
                                   &refreshmax);
    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, EXCESS_BUCKETCOUNTf,
                                   &bucket_count);
    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, EXCESS_REFRESHCOUNTf,
                                   &refresh_rate);
    soc_SVM_METER_TABLEm_field_get(unit, &svm_entry, EXCESS_BUCKETSIZEf,
                                   &bucket_size);
    rv = _bcm_xgs_bucket_encoding_to_kbits(refresh_rate, bucket_size, granularity,
     _BCM_XGS_METER_FLAG_GRANULARITY | _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER,
                                           &pol_cfg->pkbits_sec,
                                           &pol_cfg->pkbits_burst);
    if (!BCM_SUCCESS(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to translate rate in kbps to bucket size "
                              "and granularity \n")));
        return rv;
    }
    _bcm_global_meter_max_rate_set(pol_cfg->pkbits_sec,
                                   refreshmax, &pol_cfg->max_pkbits_sec);

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      _bcm_esw_policer_action_detach
 * Purpose:
 *     Internal function to Disassociate a policer action from a given
 *     policer id and readjust reference count.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     action_id             - (IN) Action ID
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_action_detach(int unit, bcm_policer_t policer_id,
                                uint32 action_id)
{
    int rv = BCM_E_NONE;
    svm_meter_table_entry_t meter_entry;
    _global_meter_policer_control_t *policer_control = NULL;
    int policer_index = 0;
    uint32 policy_index = 0;
    int index = 0;
    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    CHECK_GLOBAL_METER_INIT(unit);
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer_id);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid Policer Id \n")));
        return (rv);
    }
    /* validate action id */
    if (action_id > soc_mem_index_max(unit, SVM_POLICY_TABLEm)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid Action Id \n")));
        return BCM_E_PARAM;
    }
    GLOBAL_METER_LOCK(unit);

    rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to get policer control for the policer Id "
                              "passed  \n")));
        return (rv);
    }
    /*  read meter table and remove action_id*/
    _bcm_esw_get_policer_table_index(unit, policer_id, &policer_index);
    rv = READ_SVM_METER_TABLEm(unit, MEM_BLOCK_ANY, policer_index,
                                                          &meter_entry);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM_METER_TABLE entry \n")));
        return (rv);
    }
    /* set action to 0 */
    soc_SVM_METER_TABLEm_field_set(unit, &meter_entry, POLICY_TABLE_INDEXf,
                                                           &policy_index);

    /* Write to HW*/
    rv = WRITE_SVM_METER_TABLEm(unit,MEM_BLOCK_ANY, policer_index,
                                                            &meter_entry);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to write SVM_METER_TABLE entry \n")));
        return (rv);
    }
    /* in case of cascade with coupling we need to configure the second set
       as well */
    if ((policer_control->grp_mode ==                           \
         bcmPolicerGroupModeIntPriCascadeWithCoupling) ||
        (policer_control->grp_mode == bcmPolicerGroupModeCascadeWithCoupling)) {

        rv =_bcm_global_meter_get_coupled_cascade_policer_index(unit,
                                      policer_id, policer_control, &index);
        /*  read meter table and remove action_id*/
        rv = READ_SVM_METER_TABLEm(unit, MEM_BLOCK_ANY, index,
                                   &meter_entry);
        if (BCM_FAILURE(rv)) {
            GLOBAL_METER_UNLOCK(unit);
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read SVM_METER_TABLE entry \n")));
            return (rv);
        }
        /* set action to 0 */
        soc_SVM_METER_TABLEm_field_set(unit, &meter_entry, POLICY_TABLE_INDEXf,
                                       &policy_index);
        /* write to hW */
        rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY, index,
                           &meter_entry);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to write SVM_METER_TABLE entry \n")));
            GLOBAL_METER_UNLOCK(unit);
            return (rv);
        }
    }

   /* decrement action usage reference count */
    if (global_meter_action_bookkeep[unit][action_id].reference_count > 0) {
        global_meter_action_bookkeep[unit][action_id].reference_count--;
    }
    /* reset action id in policer control */
    policer_control->action_id = 0;
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}


/*
 * Function:
 *      _bcm_global_meter_refresh_max_get
 * Purpose:
 *      get refresh max value.
 * Parameters:
 *     rate                - (IN) CIR/EIR
 *     max_rate            - (IN) CIRmax/EIRmax
 *     refreshmax          - (OUT) REFRESH_MAX value
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_global_meter_refresh_max_get(uint32 rate,
                                      uint32 max_rate, uint32 *refreshmax) {
    int found = 0;
    int multiplier = 0, i = 0;
    if (rate == 0 && max_rate > 0) {
        *refreshmax = GLOBAL_METER_REFRESH_MAX_DISABLE_LIMIT;
    } else if (rate >= max_rate) {
        *refreshmax = 0;
    } else {
        multiplier = ((max_rate * 100) / rate);
        while ((refresh_max[i] != 0) && (found == 0)) {
            if (refresh_max[i] == multiplier) {
                *refreshmax = i;
                found = 1;
            } else if ((refresh_max[i] >  multiplier) && (i > 0)) {
                if (((refresh_max[i - 1] + refresh_max[i]) / 2) >= multiplier) {
                    *refreshmax = (i - 1);
                } else {
                    *refreshmax = i;
                }
                found = 1;
            }
            i++;
        }
        if (found == 0) {
            *refreshmax = GLOBAL_METER_REFRESH_MAX_DISABLE_LIMIT;
        }
    }
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_global_meter_write_config_to_hw
 * Purpose:
 *      Write policer configuraton to meter table.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pol_cfg               - (IN) policer configuration
 *     policer_id            - (IN) policer id
 *     policer_control       - (IN) Policer control info
 * Returns:
 *     BCM_E_XXX
 */
int  _bcm_global_meter_write_config_to_hw(int unit,
                                          bcm_policer_config_t *pol_cfg,
                                          bcm_policer_t policer_id,
                  _global_meter_policer_control_t *policer_control)
{
#if defined(BCM_KATANA_SUPPORT) || defined(BCM_TRIUMPH3_SUPPORT)
    int rv = BCM_E_NONE;
    svm_meter_table_entry_t data;
    uint32 flags=0;
    uint32    c_bucketsize = 0, e_bucketsize = 0;  /* Bucket size.       */
    uint32    c_refresh_rate = 0, e_refresh_rate = 0; /* Policer refresh rate.*/
    uint32    granularity = 0;     /* Policer granularity.     */
    uint32    refresh_bitsize;     /* Number of bits for the
                                      refresh rate field.      */
    uint32    bucket_max_bitsize;  /* Number of bits for the
                                      bucket max field.        */
    uint32 c_bucketcount = 0, e_bucketcount=0,  bucket_cnt_bitsize = 0;
    uint32 mode = 0, mode_modifier = 0;
    uint32 coupling = 0, policy_index = 0;
    uint32  c_refreshmax = 0, e_refreshmax = 0;
    int index = 0;
    uint32 current_action_id = 0;
    bcm_policer_t  macro_meter_policer = 0;
    int macro_flow_meter_index = 0;
    svm_macroflow_index_table_entry_t macro_flow_entry;
    uint32 start_of_chain = 0, end_of_chain = 0;
    uint32 pkt_bytes = 0;
    int incr_reqd = 0;
    /* read HW register */
    _bcm_esw_get_policer_table_index(unit, policer_id, &index);
    rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY, index, &data);
    BCM_IF_ERROR_RETURN(rv);
    policy_index = pol_cfg->action_id;

    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, POLICY_TABLE_INDEXf)) {
        soc_SVM_METER_TABLEm_field_get(unit,
              &data, POLICY_TABLE_INDEXf, &current_action_id);
        if( policy_index != current_action_id ) {
            if(current_action_id != 0) {
                /* detach the existing action and
                 * decrement reference count
                 */
                _bcm_esw_policer_action_detach(unit, policer_id,
                                               current_action_id);
            }
            soc_SVM_METER_TABLEm_field_set(unit, &data,
                                           POLICY_TABLE_INDEXf, &policy_index);
            if (policy_index != 0) {
                incr_reqd = 1;
            }
        }
    }

    flags = _BCM_XGS_METER_FLAG_GRANULARITY |
                                 _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER;

    if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
        flags |= _BCM_XGS_METER_FLAG_PACKET_MODE;
    } else {
        flags &= ~_BCM_XGS_METER_FLAG_PACKET_MODE;
    }
    refresh_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                                     COMMITTED_REFRESHCOUNTf);
    bucket_max_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                                     COMMITTED_BUCKETSIZEf);

    /* Calculate policer bucket size/refresh_rate/granularity. */
    if (pol_cfg->pkbits_sec > pol_cfg->ckbits_sec) {
        rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->pkbits_sec,
                                            pol_cfg->pkbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &c_refresh_rate, &c_bucketsize,
                                            &granularity);
        BCM_IF_ERROR_RETURN(rv);
        rv = _bcm_xgs_kbits_to_bucket_encoding_with_granularity(
                                            pol_cfg->ckbits_sec,
                                             pol_cfg->ckbits_burst,
                                             flags, refresh_bitsize,
                                             bucket_max_bitsize,
                                             &c_refresh_rate, &c_bucketsize,
                                             &granularity);
        BCM_IF_ERROR_RETURN(rv);
    }  else {
         rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->ckbits_sec,
                                            pol_cfg->ckbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &c_refresh_rate, &c_bucketsize,
                                            &granularity);
        BCM_IF_ERROR_RETURN(rv);
    }

     /* Calculate initial backet count.
     * bucket size << (bit count diff - 1(sign) -1 (overflow))  - 1
     */
    bucket_cnt_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                              COMMITTED_BUCKETCOUNTf);
    if (c_bucketsize == 0 && c_refresh_rate == 0) {
        /* Committed should always be out of profile */
        c_bucketcount = (1 << bucket_cnt_bitsize) - 1;
    } else if (c_bucketsize) {
        c_bucketcount =
            (c_bucketsize << (bucket_cnt_bitsize - bucket_max_bitsize - 2))  - 1;
        c_bucketcount &= ((1 << bucket_cnt_bitsize) - 1);
    } else {
        c_bucketcount = 0;
    }
    rv =_bcm_global_meter_refresh_max_get(pol_cfg->ckbits_sec,
                                        pol_cfg->max_ckbits_sec, &c_refreshmax);
    BCM_IF_ERROR_RETURN(rv);

    if (pol_cfg->mode == bcmPolicerModeCoupledCascade) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_REFRESHMAXf,
                                                             &e_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_BUCKETCOUNTf,
                                                            &e_bucketcount);
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_REFRESHCOUNTf,
                                                             &e_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_BUCKETSIZEf,
                                                               &e_bucketsize);
    } else {
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_REFRESHMAXf,
                                                             &c_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_BUCKETCOUNTf,
                                                             &c_bucketcount);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_REFRESHCOUNTf,
                                                             &c_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_BUCKETSIZEf,
                                                               &c_bucketsize);
    }

    refresh_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                                         EXCESS_REFRESHCOUNTf);
    bucket_max_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                                           EXCESS_BUCKETSIZEf);

    if ((pol_cfg->ckbits_sec == 0) && (pol_cfg->ckbits_burst == 0)) {
        rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->pkbits_sec,
                                            pol_cfg->pkbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &e_refresh_rate, &e_bucketsize,
                                            &granularity);

    } else {
        rv = _bcm_xgs_kbits_to_bucket_encoding_with_granularity(
                                            pol_cfg->pkbits_sec,
                                            pol_cfg->pkbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &e_refresh_rate, &e_bucketsize,
                                            &granularity);
    }
    BCM_IF_ERROR_RETURN(rv);
     /* Calculate initial backet count.
     * bucket size << (bit count diff - 1(sign) -1 (overflow))  - 1
     */
    bucket_cnt_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                              EXCESS_BUCKETCOUNTf);
    if (e_bucketsize == 0 && e_refresh_rate == 0) {
        /* Excess should always be out of profile */
        e_bucketcount = (1 << bucket_cnt_bitsize) - 1;
    } else if (e_bucketsize) {
        e_bucketcount =
            (e_bucketsize << (bucket_cnt_bitsize - bucket_max_bitsize - 2))  - 1;
        e_bucketcount &= ((1 << bucket_cnt_bitsize) - 1);
    } else {
        e_bucketcount = 0;
    }
    /* Set HW register */

    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_GRANf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, METER_GRANf, &granularity);
    }

    rv = _bcm_global_meter_refresh_max_get(pol_cfg->pkbits_sec,
                                      pol_cfg->max_pkbits_sec, &e_refreshmax);
    BCM_IF_ERROR_RETURN(rv);

    if (pol_cfg->mode == bcmPolicerModeCoupledCascade) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_REFRESHMAXf,
                                                             &e_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_BUCKETCOUNTf,
                                                          &e_bucketcount);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_REFRESHCOUNTf,
                                                             &e_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_BUCKETSIZEf,
                                                                 &e_bucketsize);
    } else {
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_REFRESHMAXf,
                                                             &e_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_BUCKETCOUNTf,
                                                          &e_bucketcount);
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_REFRESHCOUNTf,
                                                             &e_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &data, EXCESS_BUCKETSIZEf,
                                                                 &e_bucketsize);
    }
    /* PKT/BYTE - by default BYTE counter */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, PKT_BYTESf)) {
        if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
            pkt_bytes = 1;
            soc_SVM_METER_TABLEm_field_set(unit, &data, PKT_BYTESf, &pkt_bytes);
        }
    }
    /* MIN/MAX/MIN_MAX */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_SHARING_MODEf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, METER_SHARING_MODEf,
                                                 &pol_cfg->sharing_mode);
    }
    if (pol_cfg->flags & BCM_POLICER_COLOR_BLIND) {
        mode_modifier = 0; /* color blind */
    } else {
        mode_modifier = 1; /* color aware */
    }

    switch (pol_cfg->mode) {
        case bcmPolicerModeGreen:
            mode = GLOBAL_METER_MODE_DEFAULT;
            mode_modifier = 0;
            break;

        case bcmPolicerModePassThrough:
            mode = GLOBAL_METER_MODE_DEFAULT;
            mode_modifier = 1;
            break;

        case bcmPolicerModeSrTcm:
            mode = GLOBAL_METER_MODE_SR_TCM;
            break;

        case bcmPolicerModeSrTcmModified:
            mode = GLOBAL_METER_MODE_SR_TCM_MODIFIED;
            break;

        case bcmPolicerModeTrTcm:
            mode = GLOBAL_METER_MODE_TR_TCM;
            break;

        case bcmPolicerModeTrTcmDs:
            mode = GLOBAL_METER_MODE_TR_TCM_MODIFIED;
            break;

        case bcmPolicerModeCoupledTrTcmDs:
            mode = GLOBAL_METER_MODE_TR_TCM_MODIFIED;
            coupling = 1;
            break;

        case bcmPolicerModeCascade:
            mode = GLOBAL_METER_MODE_CASCADE;
            coupling = 0;
            break;

        case bcmPolicerModeCoupledCascade:
            mode = GLOBAL_METER_MODE_CASCADE;
            coupling = 1;
            break;

        default:
           LOG_DEBUG(BSL_LS_BCM_POLICER,
                     (BSL_META_U(unit,
                                 "Invalid policer mode \n")));
           return BCM_E_PARAM;
           break;
     }
    /* SRTCM TrTcm Modified SrTcm, Modified TrTcm, cascade, default */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, METER_MODEf, &mode);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODIFIERf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, METER_MODIFIERf,
                                                        &mode_modifier);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COUPLING_FLAGf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, COUPLING_FLAGf, &coupling);
    }
    /* Read START/END of the chain info and set table accordingly */
    if (mode == GLOBAL_METER_MODE_CASCADE) {
        if (pol_cfg->flags & BCM_POLICER_BW_SHARING_GROUP_START) {
            start_of_chain = 1;
        }
        if (pol_cfg->flags & BCM_POLICER_BW_SHARING_GROUP_END) {
            end_of_chain = 1;
        }
    }
    soc_SVM_METER_TABLEm_field_set(unit, &data,
                                            START_OF_CHAINf, &start_of_chain);
    soc_SVM_METER_TABLEm_field_set(unit, &data, END_OF_CHAINf, &end_of_chain);

    /* write to hW */
    rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY, index, &data);
    BCM_IF_ERROR_RETURN(rv);
    /* increment action usage reference count */
    if (incr_reqd) {
        global_meter_action_bookkeep[unit][policy_index].reference_count++;
    }
    /* in case of cascade with coupling we need to configure the second set
       as well */
    if ((coupling == 1) && (mode == GLOBAL_METER_MODE_CASCADE)) {
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_REFRESHMAXf,
                                                             &c_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_BUCKETCOUNTf,
                                                          &c_bucketcount);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_REFRESHCOUNTf,
                                                             &c_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &data, COMMITTED_BUCKETSIZEf,
                                                                  &c_bucketsize);
        rv =_bcm_global_meter_get_coupled_cascade_policer_index(unit,
                                      policer_id, policer_control, &index);
        /* write to hW */
        rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                                             index, &data);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to configure the cascaded pair of "
                                  "policers \n")));
            return rv;
        }
    }
    if (pol_cfg->flags & BCM_POLICER_MIXED_MICRO_MACRO) {
        rv = READ_SVM_MACROFLOW_INDEX_TABLEm(unit, MEM_BLOCK_ANY,
                                         index, &macro_flow_entry);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unable to access macro flow table at the index provided\n")));
            return rv;
        }

        soc_mem_field_get(unit, SVM_MACROFLOW_INDEX_TABLEm, (void *) &macro_flow_entry,
                           MACROFLOW_INDEXf, (uint32 *)&macro_flow_meter_index);
        if (macro_flow_meter_index > 0) {
            _bcm_esw_get_policer_id_from_index_offset(unit,
                               macro_flow_meter_index, 0, &macro_meter_policer);
            rv = _bcm_esw_policer_decrement_ref_count(unit,
                                                      macro_meter_policer);
            if (BCM_FAILURE(rv)) {
                LOG_DEBUG(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%u : Unable to decrement ref count"),
                           macro_flow_meter_index));
                return rv;
            }
            macro_flow_meter_index = 0; /* clear macro flow index entry */
            soc_SVM_MACROFLOW_INDEX_TABLEm_field_set(unit, &macro_flow_entry,
                           MACROFLOW_INDEXf, (uint32 *)&macro_flow_meter_index);
            rv = WRITE_SVM_MACROFLOW_INDEX_TABLEm(unit, MEM_BLOCK_ANY,
                                          index, &macro_flow_entry);
            if (!(BCM_SUCCESS(rv))) {
                LOG_DEBUG(BSL_LS_BCM_COMMON, \
                          (BSL_META_U(unit,
                                      "Unable to write to macro flow table at index \
                                      provided\n")));
                return rv;
            }
        }
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      _bcm_global_meter_write_config_mef_10dot3_to_hw
 * Purpose:
 *      Write policer configuraton to meter table.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pol_cfg               - (IN) policer configuration
 *     policer_id            - (IN) policer id
 *     policer_control       - (IN) Policer control info
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_global_meter_write_config_mef_10dot3_to_hw (int unit,
                                                 bcm_policer_config_t *pol_cfg,
                                                 bcm_policer_t policer_id,
                              _global_meter_policer_control_t *policer_control)
{
#if defined(BCM_APACHE_SUPPORT)
    int       rv = BCM_E_NONE;
    uint32    flags=0;
    uint32    c_bucketsize = 0, e_bucketsize = 0;  /* Bucket size.       */
    uint32    c_refresh_rate = 0, e_refresh_rate = 0; /* Policer refresh rate */
    uint32    granularity = 0;     /* Policer granularity.     */
    uint32    refresh_bitsize;     /* Number of bits for the
                                      refresh rate field.      */
    uint32    bucket_max_bitsize;  /* Number of bits for the
                                      bucket max field.        */
    uint32    mode = 0, mode_modifier = 0;
    uint32    coupling = 0, policy_index = 0;
    uint32    c_refreshmax = 0, e_refreshmax = 0;
    int       index = 0, pool = 0, pair_index = 0;
    uint32    current_action_id = 0;
    uint32    pkt_bytes = 0;
    uint32    nc_pool_valid = 0, nc_pool = 0, ne_pool_valid = 0, ne_pool = 0;
    uint32    c_e = 0, pair_exc = 0, pair_exc_valid = 0;
    bcm_policer_t                     macro_meter_policer = 0;
    int                               macro_flow_meter_index = 0;
    svm_macroflow_index_table_entry_t macro_flow_entry;
    svm_meter_table_entry_t           svm_entry;
    int incr_reqd = 0;

    pool = _bcm_global_meter_get_pool_from_policerId(unit, policer_id);
    /* read HW register */
    _bcm_esw_get_policer_table_index(unit, policer_id, &index);
    rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY, index, &svm_entry);
    BCM_IF_ERROR_RETURN(rv);

    policy_index = pol_cfg->action_id;
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, POLICY_TABLE_INDEXf)) {
        soc_SVM_METER_TABLEm_field_get(unit,
              &svm_entry, POLICY_TABLE_INDEXf, &current_action_id);
        if( policy_index != current_action_id ) {
            if(current_action_id != 0) {
                /* detach the existing action and
                 * decrement reference count
                 */
                _bcm_esw_policer_action_detach(unit, policer_id,
                                               current_action_id);
            }
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                           POLICY_TABLE_INDEXf, &policy_index);
            if(policy_index != 0) {
                incr_reqd = 1;
            }
        }
    }

    flags = _BCM_XGS_METER_FLAG_GRANULARITY |
                                 _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER;

    if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
        flags |= _BCM_XGS_METER_FLAG_PACKET_MODE;
    } else {
        flags &= ~_BCM_XGS_METER_FLAG_PACKET_MODE;
    }

    refresh_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                           COMMITTED_REFRESHCOUNTf);
    bucket_max_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                              COMMITTED_BUCKETSIZEf);

    if (pol_cfg->mode == bcmPolicerModeTrTcm) {
        /* SW-WAR: In this mode CIR and PIR need swapping */
        uint32 tmp;
        tmp = pol_cfg->pkbits_sec;
        pol_cfg->pkbits_sec = pol_cfg->ckbits_sec;
        pol_cfg->ckbits_sec = tmp;
    }

    /* Calculate policer bucket size/refresh_rate/granularity. */
    if (pol_cfg->pkbits_sec > pol_cfg->ckbits_sec) {
        rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->pkbits_sec,
                                            pol_cfg->pkbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &c_refresh_rate, &c_bucketsize,
                                            &granularity);
        BCM_IF_ERROR_RETURN(rv);

        rv = _bcm_xgs_kbits_to_bucket_encoding_with_granularity(
                                             pol_cfg->ckbits_sec,
                                             pol_cfg->ckbits_burst,
                                             flags, refresh_bitsize,
                                             bucket_max_bitsize,
                                             &c_refresh_rate, &c_bucketsize,
                                             &granularity);
        BCM_IF_ERROR_RETURN(rv);
    }  else {
         rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->ckbits_sec,
                                                pol_cfg->ckbits_burst,
                                                flags, refresh_bitsize,
                                                bucket_max_bitsize,
                                                &c_refresh_rate, &c_bucketsize,
                                                &granularity);
        BCM_IF_ERROR_RETURN(rv);
    }

    rv =_bcm_global_meter_refresh_max_get(pol_cfg->ckbits_sec,
                                          pol_cfg->max_ckbits_sec,
                                          &c_refreshmax);
    BCM_IF_ERROR_RETURN(rv);


    soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, COMMITTED_REFRESHMAXf,
                                   &c_refreshmax);
    soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, COMMITTED_REFRESHCOUNTf,
                                   &c_refresh_rate);
    soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, COMMITTED_BUCKETSIZEf,
                                   &c_bucketsize);


    refresh_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                           EXCESS_REFRESHCOUNTf);

    bucket_max_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                              EXCESS_BUCKETSIZEf);

    if ((pol_cfg->ckbits_sec == 0) && (pol_cfg->ckbits_burst == 0)) {
        rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->pkbits_sec,
                                            pol_cfg->pkbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &e_refresh_rate, &e_bucketsize,
                                            &granularity);

    } else {
        rv = _bcm_xgs_kbits_to_bucket_encoding_with_granularity(
                                            pol_cfg->pkbits_sec,
                                            pol_cfg->pkbits_burst,
                                            flags, refresh_bitsize,
                                            bucket_max_bitsize,
                                            &e_refresh_rate, &e_bucketsize,
                                            &granularity);
    }
    BCM_IF_ERROR_RETURN(rv);

    /* Set HW register */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_GRANf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_GRANf, &granularity);
    }

    rv = _bcm_global_meter_refresh_max_get(pol_cfg->pkbits_sec,
                                           pol_cfg->max_pkbits_sec,
                                           &e_refreshmax);
    BCM_IF_ERROR_RETURN(rv);

    if ((policer_control->grp_mode == \
         bcmPolicerGroupModeIntPriCascadeWithCoupling) ||
        (policer_control->grp_mode == bcmPolicerGroupModeCascadeWithCoupling)) {
        
    } else { /* non-GCF mode */
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, EXCESS_REFRESHMAXf,
                                       &e_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, EXCESS_REFRESHCOUNTf,
                                       &e_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, EXCESS_BUCKETSIZEf,
                                       &e_bucketsize);
    }

    /* PKT/BYTE - by default BYTE counter */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, PKT_BYTESf)) {
        if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
            pkt_bytes = 1;
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, PKT_BYTESf, &pkt_bytes);
        }
    }
    /* MIN/MAX/MIN_MAX */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_SHARING_MODEf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_SHARING_MODEf,
                                                 &pol_cfg->sharing_mode);
    }
    if (pol_cfg->flags & BCM_POLICER_COLOR_BLIND) {
        mode_modifier = 0; /* color blind */
    } else {
        mode_modifier = 1; /* color aware */
    }

    switch (pol_cfg->mode) {
        case bcmPolicerModeGreen:
            mode = GLOBAL_METER_MODE_DEFAULT;
            mode_modifier = 0;
            break;

        case bcmPolicerModePassThrough:
            mode = GLOBAL_METER_MODE_DEFAULT;
            mode_modifier = 1;
            break;

        case bcmPolicerModeSrTcm:
            mode = GLOBAL_METER_MODE_SR_TCM;
            break;

        case bcmPolicerModeSrTcmModified:
            /* This mode is not supported in APACHE */
            return BCM_E_PARAM;

        case bcmPolicerModeTrTcm:
            mode = GLOBAL_METER_MODE_TR_TCM;
            break;

        case bcmPolicerModeTrTcmDs:
            mode = GLOBAL_METER_MODE_TR_TCM_MODIFIED;
            break;

        case bcmPolicerModeCoupledTrTcmDs:
            mode = GLOBAL_METER_MODE_TR_TCM_MODIFIED;
            coupling = 1;
            break;

        case bcmPolicerModeCascade:
            mode = GLOBAL_METER_MODE_CASCADE;
            /* If policer is part of non-GCF chain, adjust overflow params */
            if ((policer_control->grp_mode == bcmPolicerGroupModeIntPriCascade)
                || (policer_control->grp_mode == bcmPolicerGroupModeCascade)) {
                /* Check if next committed pool is supplied */
                if (pol_cfg->ncoflow_policer_id != 0) {
                    nc_pool = _bcm_global_meter_get_pool_from_policerId(unit,
                                                   pol_cfg->ncoflow_policer_id);
                } else {
                    nc_pool =  pool + 1;
                }

                /* Check if next excess pool is supplied */
                if (pol_cfg->npoflow_policer_id != 0) {
                    ne_pool = _bcm_global_meter_get_pool_from_policerId(unit,
                                                  pol_cfg->npoflow_policer_id);
                } else {
                    ne_pool = pool + 1;
                }

                if (ne_pool < nc_pool) {
                    /* Excess pool is high priority than commit pool
                       Shouldn't happen */
                    return BCM_E_PARAM;
                }

                if ((pol_cfg->flags & BCM_POLICER_BW_SHARING_GROUP_END) ||
                    ((SOC_INFO(unit).global_meter_pools -1 ) == pool)) {
                    /* Last policer in chain, make next committed and
                       excess pools invalid */
                    nc_pool_valid = nc_pool = 0;
                    ne_pool_valid = ne_pool = 0;
                    soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                                   NXT_COMMITTED_BUCKET_VALIDf,
                                                   &nc_pool_valid);
                    soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                                   NXT_EXCESS_BUCKET_VALIDf,
                                                   &ne_pool_valid);
                }

                soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                               NXT_COMMITTED_BUCKETf,
                                               &nc_pool);
                soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                               NXT_EXCESS_BUCKETf,
                                               &ne_pool);
            } /* group mode check */
            break;

        case bcmPolicerModeCoupledCascade:
            mode = GLOBAL_METER_MODE_CASCADE;
            /* If policer is part of non-GCF chain, adjust overflow params */
            if ((policer_control->grp_mode == bcmPolicerGroupModeIntPriCascade)
                || (policer_control->grp_mode == bcmPolicerGroupModeCascade)) {
                /* Enable COMMITEED_EXCESS in HW */
                c_e = 1;
                soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                               COMMITTED_EXCESSf, &c_e);
                nc_pool_valid = 1; /* Already in HW during group config */
                nc_pool = pool;    /* Implicit requirement */
                soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                               NXT_COMMITTED_BUCKETf,
                                               &nc_pool);

                /* Check if next excess pool is supplied */
                if (pol_cfg->npoflow_policer_id != 0) {
                    ne_pool = _bcm_global_meter_get_pool_from_policerId(unit,
                                                  pol_cfg->npoflow_policer_id);
                } else {
                    ne_pool = pool + 1;
                }

                if ((pol_cfg->flags & BCM_POLICER_BW_SHARING_GROUP_END) ||
                    ((SOC_INFO(unit).global_meter_pools -1 ) == pool)) {
                    /* Last policer is chain, disable excess pool validity */
                    ne_pool = 0;
                    ne_pool_valid = 0;
                    soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                                   NXT_EXCESS_BUCKET_VALIDf,
                                                   &ne_pool_valid);
                }
                soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                               NXT_EXCESS_BUCKETf,
                                               &ne_pool);
            } /* End - "if" group mode check */
            break;

        default:
           LOG_DEBUG(BSL_LS_BCM_POLICER,
                     (BSL_META_U(unit,
                                 "Invalid policer mode \n")));
           return BCM_E_PARAM;
           break;
     }
    /* SRTCM TrTcm Modified SrTcm, Modified TrTcm, cascade, default */
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_MODEf, &mode);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODIFIERf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_MODIFIERf,
                                                        &mode_modifier);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COUPLING_FLAGf)) {
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, COUPLING_FLAGf, &coupling);
    }

    if ((policer_control->grp_mode != \
         bcmPolicerGroupModeIntPriCascadeWithCoupling) &&
        (policer_control->grp_mode != bcmPolicerGroupModeCascadeWithCoupling)) {
        /* Set METER_PAIR_EXCESS for all other modes here */
        pair_exc_valid = 1;
        pair_exc = pool;
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       METER_PAIR_EXCESS_BUCKETf,
                                       &pair_exc);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       METER_PAIR_EXCESS_BUCKET_VALIDf,
                                       &pair_exc_valid);
    }

    /* write to hW */
    rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY, index, &svm_entry);
    BCM_IF_ERROR_RETURN(rv);
    /* increment action usage reference count */
    if (incr_reqd) {
        global_meter_action_bookkeep[unit][policy_index].reference_count++;
    }
    /* in case of cascade with coupling we need to configure the second set
       as well - GCF Mode */
    if ((policer_control->grp_mode == \
         bcmPolicerGroupModeIntPriCascadeWithCoupling) ||
        (policer_control->grp_mode == bcmPolicerGroupModeCascadeWithCoupling)) {
        rv = _bcm_global_meter_get_coupled_cascade_policer_index(unit,
                                      policer_id, policer_control, &pair_index);
        BCM_IF_ERROR_RETURN(rv);

        rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                          pair_index, &svm_entry);
        BCM_IF_ERROR_RETURN(rv);

        policy_index = pol_cfg->action_id;
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, POLICY_TABLE_INDEXf)) {
            soc_SVM_METER_TABLEm_field_get(unit,
              &svm_entry, POLICY_TABLE_INDEXf, &current_action_id);
            /* detach the existing action and decrement reference count */
            if (current_action_id) {
                _bcm_esw_policer_action_detach(unit, policer_id,
                                               current_action_id);
            }
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, POLICY_TABLE_INDEXf,
                                           &policy_index);
        }

        flags = _BCM_XGS_METER_FLAG_GRANULARITY |
                                 _BCM_XGS_METER_FLAG_GLOBAL_METER_POLICER;

        refresh_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                               EXCESS_REFRESHCOUNTf);

        bucket_max_bitsize = soc_mem_field_length(unit, SVM_METER_TABLEm,
                                                  EXCESS_BUCKETSIZEf);

        if ((pol_cfg->ckbits_sec == 0) && (pol_cfg->ckbits_burst == 0)) {
            rv = _bcm_xgs_kbits_to_bucket_encoding(pol_cfg->pkbits_sec,
                                                   pol_cfg->pkbits_burst,
                                                   flags, refresh_bitsize,
                                                   bucket_max_bitsize,
                                                   &e_refresh_rate,
                                                   &e_bucketsize,
                                                   &granularity);

        } else {
            rv = _bcm_xgs_kbits_to_bucket_encoding_with_granularity(
                pol_cfg->pkbits_sec,
                pol_cfg->pkbits_burst,
                flags, refresh_bitsize,
                bucket_max_bitsize,
                &e_refresh_rate, &e_bucketsize,
                &granularity);
        }
        BCM_IF_ERROR_RETURN(rv);

        /* Set HW register */
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_GRANf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_GRANf, &granularity);
        }

        rv = _bcm_global_meter_refresh_max_get(pol_cfg->pkbits_sec,
                                               pol_cfg->max_pkbits_sec,
                                               &e_refreshmax);
        BCM_IF_ERROR_RETURN(rv);

        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, EXCESS_REFRESHMAXf,
                                       &e_refreshmax);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, EXCESS_REFRESHCOUNTf,
                                       &e_refresh_rate);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, EXCESS_BUCKETSIZEf,
                                       &e_bucketsize);
        /* PKT/BYTE - by default BYTE counter */
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, PKT_BYTESf)) {
            if (pol_cfg->flags & BCM_POLICER_MODE_PACKETS) {
                pkt_bytes = 1;
                soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, PKT_BYTESf, &pkt_bytes);
            }
        }

        mode = GLOBAL_METER_MODE_CASCADE;

        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_MODEf, &mode);
        }
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODIFIERf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry, METER_MODIFIERf,
                                           &mode_modifier);
        }

        /* write to hW */
        rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                           pair_index, &svm_entry);
        if (!BCM_SUCCESS(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to configure the cascaded pair of "
                                  "policers \n")));
            return rv;
        }
    } /* end if for GCF mode check */

    if (pol_cfg->flags & BCM_POLICER_MIXED_MICRO_MACRO) {
        rv = READ_SVM_MACROFLOW_INDEX_TABLEm(unit, MEM_BLOCK_ANY,
                                             index, &macro_flow_entry);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Unable to access macro flow table at the index provided\n")));
            return rv;
        }

        soc_mem_field_get(unit, SVM_MACROFLOW_INDEX_TABLEm, (void *) &macro_flow_entry,
                           MACROFLOW_INDEXf, (uint32 *)&macro_flow_meter_index);
        if (macro_flow_meter_index > 0) {
            _bcm_esw_get_policer_id_from_index_offset(unit,
                               macro_flow_meter_index, 0, &macro_meter_policer);
            rv = _bcm_esw_policer_decrement_ref_count(unit,
                                                      macro_meter_policer);
            if (BCM_FAILURE(rv)) {
                LOG_DEBUG(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%u : Unable to decrement ref count"),
                           macro_flow_meter_index));
                return rv;
            }
            macro_flow_meter_index = 0; /* clear macro flow index entry */
            soc_SVM_MACROFLOW_INDEX_TABLEm_field_set(unit, &macro_flow_entry,
                           MACROFLOW_INDEXf, (uint32 *)&macro_flow_meter_index);
            rv = WRITE_SVM_MACROFLOW_INDEX_TABLEm(unit, MEM_BLOCK_ANY,
                                          index, &macro_flow_entry);
            if (!(BCM_SUCCESS(rv))) {
                LOG_DEBUG(BSL_LS_BCM_COMMON, \
                          (BSL_META_U(unit,
                                      "Unable to write to macro flow table at index \
                                      provided\n")));
                return rv;
            }
        }
    }

    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}
/*
 * Function:
 *      _bcm_global_meter_get_coupled_cascade_policer_index
 * Purpose:
 *      In case of cascade with coupling get the index of the coupled
 *      pair
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     policer_control       - (IN) Policer control info
 *     new_index             - (OUT) Policer index
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_global_meter_get_coupled_cascade_policer_index(int unit,
                                          bcm_policer_t policer_id,
                  _global_meter_policer_control_t *policer_control,
                   int *new_index)
{

    int rv = BCM_E_NONE;
    int offset1 = 0, pool1 = 0, pool2 = 0;
    int new_offset = 0;
    int size_pool = 0, num_pools = 0;
    int index = 0, index_max = 0;
    int offset_mask = 0;
    int pool_mask = 0, pool_offset = 0;
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;

    offset1 = policer_control->pid & offset_mask;

    pool1 = (policer_control->pid & pool_mask) >> pool_offset;
    pool2 = (policer_id & pool_mask) >> pool_offset;

    if (!soc_feature(0,soc_feature_global_meter_mef_10dot3)) {
        index_max = policer_control->no_of_policers/2;
        if (pool1 == pool2) {
            new_offset = policer_control->offset[index_max];
            *new_index = offset1 + (new_offset * size_pool);
            return rv;
        }
        for (index = 1; index < index_max; index++) {
            if ((pool1 + policer_control->offset[index]) == pool2) {
                new_offset = policer_control->offset[index_max + index];
                *new_index = offset1 + (new_offset * size_pool);
            }
        }
    } else { /* MEF 10.3 onward device */
        new_offset = pool2 + 4;
        /* In GCF mode, SDK assumes its always chain of 4 policers.
         * Not much choice left but to carry on this legacy assumption
         * as no GCF mode functionality change is expected in MEF10.3
         */
        *new_index = offset1 + (new_offset * size_pool);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_global_meter_base_policer_get
 * Purpose:
 *      Get the internal data sructure for a given polier id if
 *      it is base policer.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pid                   - (IN) policer id
 *     policer_p             - (OUT) Policer control info
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_global_meter_base_policer_get(int unit, bcm_policer_t pid,
                         _global_meter_policer_control_t **policer_p)
{
    _global_meter_policer_control_t *global_meter_pl = NULL;
    uint32 hash_index;      /* Entry hash.  */
    /* Input parameters check. */
    if (NULL == policer_p) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Policer control is null \n")));
        return (BCM_E_PARAM);
    }
    hash_index = pid & _GLOBAL_METER_HASH_MASK;
    global_meter_pl  =  global_meter_policer_bookkeep[unit][hash_index];
    while (NULL != global_meter_pl) {
        /* Match entry id. */
        if (global_meter_pl->pid == pid) {
            *policer_p = global_meter_pl;
            return (BCM_E_NONE);
        }
        global_meter_pl = global_meter_pl->next;
    }
    /* Policer with pid == pid was not found. */
    return (BCM_E_NOT_FOUND);
}

/*
 * Function:
 *      _bcm_global_meter_policer_get
 * Purpose:
 *      Get the internal control data structure for a given policer id.
 * Parameters:
 *     unit                  - (IN) unit number
 *     pid                   - (IN) policer id
 *     policer_p             - (OUT) Policer control info
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_global_meter_policer_get(int unit, bcm_policer_t pid,
                         _global_meter_policer_control_t **policer_p)
{
    _global_meter_policer_control_t *global_meter_pl = NULL;
    uint32 hash_index;      /* Entry hash */
    int offset1 = 0, offset2 = 0;
    int pool1 = 0, pool2 = 0;
    uint32 index = 0, index_max = 0;
    int rv = BCM_E_NONE;
    int hash_max = 0;
    int offset_mask = 0;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    /* Input parameters check. */
    if (NULL == policer_p) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Policer control is null \n")));
        return (BCM_E_PARAM);
    }
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;

    rv = _bcm_global_meter_base_policer_get(unit, pid, policer_p);
    if (BCM_E_NOT_FOUND == rv) {
        hash_max = pid & _GLOBAL_METER_HASH_MASK;
        for (hash_index=0; hash_index <= hash_max; hash_index++) {
            global_meter_pl  =
                      global_meter_policer_bookkeep[unit][hash_index];
            while (NULL != global_meter_pl) {
                offset1 = global_meter_pl->pid & offset_mask;
                offset2 = pid & offset_mask;

                pool1 = (global_meter_pl->pid & pool_mask) >> pool_offset;
                pool2 = (pid & pool_mask) >> pool_offset;

                /* in case of cascade policers, only pool number is diferent */
                if (global_meter_pl->direction ==
                                               GLOBAL_METER_ALLOC_HORIZONTAL) {
                    if (offset1 == offset2) {
                        index_max = global_meter_pl->no_of_policers;
                        for (index = 1; index < index_max; index++) {
                            if ((pool1 + global_meter_pl->offset[index]) ==
                                                                     pool2) {
                                *policer_p = global_meter_pl;
                                 return (BCM_E_NONE);
                            }
                        }
                    }
                } else {
                    if (pool1 == pool2) {
                        if (offset2 > offset1 && offset2 <
                                (offset1 + global_meter_pl->no_of_policers)) {
                            *policer_p = global_meter_pl;
                             return (BCM_E_NONE);
                        }
                    }

                }
                global_meter_pl = global_meter_pl->next;
            }
        }
        /* Policer with pid == pid was not found. */
        return (BCM_E_NOT_FOUND);
    } else {
        return rv;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_global_meter_policer_get
 * Purpose:
 *      Get the configuration info for a given policer id
 * Parameters:
 *     unit                  - (IN) unit number
 *     pid                   - (IN) policer id
 *     policer_p             - (OUT) Policer control info
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_policer_get(int unit, bcm_policer_t policer_id,
                   bcm_policer_config_t *pol_cfg)
{
    int rv = BCM_E_NONE;
    _global_meter_policer_control_t *policer_control = NULL;
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer_id);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid policer id %x  \n"),
                   policer_id));
        return (rv);
    }
    GLOBAL_METER_LOCK(unit);
   /* we come here from bcm_esw_policer_get,
      only when policer id indicates : global meter */
    rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
    if (BCM_SUCCESS(rv)) {
        rv = (soc_feature(unit, soc_feature_global_meter_mef_10dot3)) ?
            _bcm_global_meter_read_config_from_hw_mef_10dot3(unit, policer_id,
                                                             pol_cfg) :
            _bcm_global_meter_read_config_from_hw(unit, policer_id, pol_cfg);

        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read policer config from hw %x\n"),
                       policer_id));
            GLOBAL_METER_UNLOCK(unit);
            return (rv);
        }
    }
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_global_meter_min_burst_size_set
 * Purpose:
 *     If the passed burst size is less than the minimum required, set it to
 *     minimum required value.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     pol_cfg               - (IN) Policer control info
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_global_meter_min_burst_size_set(bcm_policer_config_t *pol_cfg)
{
    uint32 min_burst = 0;
    /* min burst should be
       max (MTU size + metering rate * refresh interval,
                                    2 * metering rate * refresh interval) */

    min_burst = (MIN_BURST_MULTIPLE *
                 ((pol_cfg->ckbits_sec * METER_REFRESH_INTERVAL) /
                                    CONVERT_SECOND_TO_MICROSECOND));
    if (min_burst > pol_cfg->ckbits_burst) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META("Commited burst is less than the minimum required "
                            "value.  \n")));
        return (BCM_E_PARAM);
    }
    min_burst = (MIN_BURST_MULTIPLE *
                 ((pol_cfg->pkbits_sec * METER_REFRESH_INTERVAL) /
                                    CONVERT_SECOND_TO_MICROSECOND));
    if (min_burst > pol_cfg->pkbits_burst) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META("Peak burst is less than the minimum required "
                            "value.  \n")));
        return (BCM_E_PARAM);
    }
    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_esw_global_meter_policer_set
 * Purpose:
 *     Configure the parmeters for a policer already created.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     pol_cfg               - (IN) Policer control info
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_esw_global_meter_policer_set(int unit, bcm_policer_t policer_id,
                   bcm_policer_config_t *pol_cfg)
{
    int rv = BCM_E_NONE;
    _global_meter_policer_control_t *policer_control = NULL;
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer_id);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid policer id %x  \n"),
                   policer_id));
        return (rv);
    }
    GLOBAL_METER_LOCK(unit);

   /* we come here from bcm_esw_policer_get,
      only when policer id indicates : global meter */
    rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
    if (BCM_SUCCESS(rv)) {
       /* Check whether the burst size is >= min required. If not, set it
          to minimum required value */
        rv = _bcm_global_meter_min_burst_size_set(pol_cfg);

       /* Add code to write this to HW */
        if (soc_feature(unit, soc_feature_global_meter_mef_10dot3)) {
            rv =  _bcm_global_meter_write_config_mef_10dot3_to_hw(unit, pol_cfg,
                                            policer_id, policer_control);
        } else {
            rv = _bcm_global_meter_write_config_to_hw(unit, pol_cfg,
                                                  policer_id, policer_control);
        }
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to write policer config to hw %x\n"),
                       policer_id));
            GLOBAL_METER_UNLOCK(unit);
            return (rv);
        }

    }
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_policer_destroy_all
 * Purpose:
 *      Destroy all the policers.
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_policer_destroy_all(int unit)
{

    int rv = BCM_E_NONE;
    int idx = 0;
    _global_meter_policer_control_t *policer_control = NULL;
    if (global_meter_status[unit].initialised == 0) {
        return rv;
    }
    if (global_meter_mutex[unit] != NULL) {
        GLOBAL_METER_LOCK(unit);
    }
    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _GLOBAL_METER_HASH_SIZE; idx++) {
        policer_control = global_meter_policer_bookkeep[unit][idx];
        /* Destroy entries in each bucket. */
        while (NULL != policer_control) {
            rv = _bcm_esw_global_meter_policer_destroy2(unit, policer_control);
            if (BCM_FAILURE(rv)) {
                break;
            }
            policer_control = global_meter_policer_bookkeep[unit][idx];
        }
        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    if (global_meter_mutex[unit] != NULL) {
        GLOBAL_METER_UNLOCK(unit);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_policer_traverse
 * Purpose:
 *     Traverse through all the policers that are configured and
 *     call the given call back function with policer configuration.
 * Parameters:
 *     unit                  - (IN) unit number
 *     cb                    - (IN) Call back function
 *     user_data             - (OUT) Data for callback function
 * Returns:
 *     BCM_E_XXX
 */
int  _bcm_esw_global_meter_policer_traverse(int unit,
               bcm_policer_traverse_cb cb,  void *user_data)
{
    int rv = BCM_E_NONE;
    int idx = 0;
    bcm_policer_config_t    cfg;      /* Policer configuration.         */
    _global_meter_policer_control_t *policer_control = NULL;
    /* Iterate over all hash buckets. */
    for (idx = 0; idx < _GLOBAL_METER_HASH_SIZE; idx++) {
        policer_control = global_meter_policer_bookkeep[unit][idx];
        /* Destroy entries in each bucket. */
        while (NULL != policer_control) {
            rv = _bcm_esw_global_meter_policer_get(unit, policer_control->pid,
                                                                         &cfg);
            if (BCM_FAILURE(rv)) {
                break;
            }
            rv = (*cb)(unit, policer_control->pid, &cfg, user_data);
            if (BCM_FAILURE(rv)) {
                break;
            }
            policer_control = policer_control->next;
        }
        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_policer_destroy
 * Purpose:
 *     Destroys a policer and readjusts action
 *     usage reference count and policer allocation management data struture.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_policer_destroy(int unit, bcm_policer_t policer_id)
{
    int rv = BCM_E_NONE;
    _global_meter_policer_control_t *policer_control = NULL;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer_id);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Invalid policer id %x  \n"),
                     policer_id));
        return (rv);
    }
    GLOBAL_METER_LOCK(unit);
    rv = _bcm_global_meter_base_policer_get(unit,
                                   policer_id, &policer_control);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to get policer control for policer id %x\n"),
                     policer_id));
        return (rv);
    }
    rv = _bcm_esw_global_meter_policer_destroy2(unit, policer_control);
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_policer_destroy2
 * Purpose:
 *     Internal function that destroys a policer and readjusts action
 *     usage reference count and policer allocation management data struture.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) policer id
 *     policer_control       - (IN) policer control info
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_policer_destroy2(int unit,
               _global_meter_policer_control_t *policer_control)
{

    int index = 0;
    uint32 pool = 0, pid = 0, numbers = 0;
    int rv=BCM_E_NONE;
    int offset_mode = 0;
    int pool_id = 0;
    int offset_mask = 0;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    int bank_size = 0, num_banks_per_pool = 1, bank_index = 0;
    int handle_id = 0;
    bcm_policer_t  macro_meter_policer = 0;
    int macro_flow_meter_index = 0;
    svm_macroflow_index_table_entry_t   *buf;
    svm_macroflow_index_table_entry_t   *entry;
    int entry_mem_size;    /* Size of table entry. */
    int entry_index = 0;
    int entry_index_max = 0;
    int entry_modified = 0;
    svm_meter_table_entry_t svm_meter_entry;
    svm_meter_table_entry_t *svm_meter_buf;
    int base_index = 0;

    /* Make sure policer is not attached to any entry. */
    if (0 != policer_control->ref_count) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Policer is still in use  \n")));
        return (BCM_E_BUSY);
    }
    if (global_meter_action_bookkeep[unit]\
                             [policer_control->action_id].reference_count > 0)
    {
        global_meter_action_bookkeep[unit]\
                               [policer_control->action_id].reference_count--;
    }
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;

    pid = policer_control->pid & offset_mask;
    pool = (policer_control->pid & pool_mask) >> pool_offset;
    numbers = policer_control->no_of_policers;
    bank_size = size_pool / num_banks_per_pool;

    offset_mode =
             ((policer_control->pid & BCM_POLICER_GLOBAL_METER_MODE_MASK) >>
                                    BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
    if (offset_mode >= 1) {
        offset_mode = offset_mode - 1;
    }

    /*  get the HW index for the policer to be deleted */
    rv = _bcm_esw_get_policer_table_index(unit, policer_control->pid,
                                                     &base_index);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to Get policer index for "
                              "policer \n")));
        return rv;
    }

    /* check horizontal or vertical allocation and then free the policer */
    if (policer_control->direction == GLOBAL_METER_ALLOC_VERTICAL) {

        handle_id = (pool * num_banks_per_pool) + (pid / bank_size);
        bank_index = pid % bank_size;

        rv = shr_aidxres_list_free(meter_alloc_list_handle[unit][handle_id], bank_index);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to free policer handle\n")));
            return (rv);
        }
        for (index = 0; index < numbers; index++) {
            rv = _bcm_gloabl_meter_unreserve_bloc_horizontally(unit,
                                                    pool, pid+index);
            if (!BCM_SUCCESS(rv)) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Unable to free policer handle in hz "
                                      "index management\n")));
                return BCM_E_INTERNAL;
            }
        }

        entry_mem_size = sizeof(svm_meter_table_entry_t);
        entry_index_max = base_index + numbers - 1;
        svm_meter_buf = soc_cm_salloc(unit, entry_mem_size * numbers,
                            "svm meter table entry");
        if (svm_meter_buf == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(svm_meter_buf, 0, entry_mem_size * numbers);

        rv = soc_mem_write_range(unit, SVM_METER_TABLEm,
                                  MEM_BLOCK_ALL, base_index,
                                  entry_index_max, svm_meter_buf);
        if (BCM_FAILURE(rv)) {
            if (svm_meter_buf != NULL) {
                soc_cm_sfree(unit, svm_meter_buf);
            }
            return rv;
        }
    } else if (policer_control->direction == GLOBAL_METER_ALLOC_HORIZONTAL) {
        pool_id = pool;
        sal_memset(&svm_meter_entry, 0, sizeof(svm_meter_entry));
        for (index = 0;index < numbers; index++) {
            if (index > 0) {
                pool_id = pool + policer_control->offset[index];
            }
            handle_id = (pool_id * num_banks_per_pool) + (pid / bank_size);
            bank_index = pid % bank_size;
            rv = shr_aidxres_list_free(
                   meter_alloc_list_handle[unit][handle_id], bank_index);
            if (BCM_FAILURE(rv)) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Unable to free policer handle\n")));
                return (rv);
            }
            WRITE_SVM_METER_TABLEm (unit, MEM_BLOCK_ALL,
                                    base_index + (pool_id * size_pool),
                                    &svm_meter_entry);
        }
        rv = _bcm_gloabl_meter_free_horizontally(unit, pool, pid, numbers,
                                                      policer_control->offset);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to free policer handle in hz "
                                  "index management\n")));
            return (rv);
        }
    }

   /* Delete policer from policers hash. */
    _GLOBAL_METER_HASH_REMOVE(global_meter_policer_bookkeep[unit],
                   _global_meter_policer_control_t,
                   policer_control, (pid & _GLOBAL_METER_HASH_MASK));
    /* decrement mode reference count  */
    if (offset_mode) {
        bcm_policer_svc_meter_dec_mode_reference_count(unit, offset_mode);
    }

    index = base_index;

    /* Write the index of the macro-meter at "micro_flow_index" location
           of SVM_MACROFLOW_INDEX_TABLE */
    if (policer_control->direction == GLOBAL_METER_ALLOC_VERTICAL) {

        entry_mem_size = sizeof(svm_macroflow_index_table_entry_t);
        entry_index_max = index + numbers - 1;

        /* Allocate buffer to store the DMAed table entries. */
        buf = soc_cm_salloc(unit, entry_mem_size * numbers,
                              "svm macro flow index table entry buffer");
        if (NULL == buf) {
            return (BCM_E_MEMORY);
        }
        /* Initialize the entry buffer. */
        sal_memset(buf, 0, sizeof(entry_mem_size) * numbers);

        /* Read the table entries into the buffer. */
        rv = soc_mem_read_range(unit, SVM_MACROFLOW_INDEX_TABLEm,
                                MEM_BLOCK_ALL, index,
                                entry_index_max, buf);
        if (BCM_FAILURE(rv)) {
            if (buf) {
                soc_cm_sfree(unit, buf);
            }
            return rv;
        }

        /* Iterate over the table entries. */
        for (entry_index = 0; entry_index < numbers; entry_index++) {
            entry = soc_mem_table_idx_to_pointer(unit,
                              SVM_MACROFLOW_INDEX_TABLEm,
                              svm_macroflow_index_table_entry_t *,
                              buf, entry_index);
            soc_mem_field_get(unit, SVM_MACROFLOW_INDEX_TABLEm, (void *)entry,
                           MACROFLOW_INDEXf, (uint32 *)&macro_flow_meter_index);
            if (macro_flow_meter_index > 0) {
                _bcm_esw_get_policer_id_from_index_offset(unit,
                               macro_flow_meter_index, 0, &macro_meter_policer);
                rv = _bcm_esw_policer_decrement_ref_count(unit,
                                                      macro_meter_policer);
                if (BCM_FAILURE(rv)) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                                (BSL_META_U(unit,
                                            "Unable to decrement ref count "
                                            "for macro meter provided\n")));
                    if (buf) {
                        soc_cm_sfree(unit, buf);
                    }
                    return rv;
                }
                macro_flow_meter_index = 0; /* clear macro flow index entry */
                soc_mem_field_set(unit, SVM_MACROFLOW_INDEX_TABLEm,
                                     (void *)entry, MACROFLOW_INDEXf,
                                     (uint32 *)&macro_flow_meter_index);
                entry_modified = 1;
            }
        }
        if (entry_modified) {
            if ((rv = soc_mem_write_range(unit, SVM_MACROFLOW_INDEX_TABLEm,
                                  MEM_BLOCK_ALL, index,
                                  entry_index_max, buf)) < 0) {
                if (BCM_FAILURE(rv)) {
                         LOG_VERBOSE(BSL_LS_BCM_POLICER,
                                     (BSL_META_U(unit,
                                                 "Unable to write to macro flow "
                                                 "index table at index provided\n")));
                    if (buf) {
                        soc_cm_sfree(unit, buf);
                    }
                    return rv;
                }
            }
        }
        if (buf) {
            soc_cm_sfree(unit, buf);
        }
        return rv;
    }
    /* De-allocate policer descriptor. */
    sal_free(policer_control);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_set_cascade_info_to_hw
 * Purpose:
 *      In case of cascade meter, write configuration info that is
 *      specific to cascade meters to HW
 * Parameters:
 *     unit                  - (IN) unit number
 *     numbers               - (IN) Number of policers
 *     policer_id            - (IN) base policer id
 *     mode                  - (IN) policer group mode
 *     pid_offset            - (IN) Offset w.r.t base policer
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_set_cascade_info_to_hw(int unit,int  numbers,
                          bcm_policer_t policer_id,
                          bcm_policer_group_mode_t mode, uint8 *pid_offset)
{
    int rv = BCM_E_NONE;
    svm_meter_table_entry_t data;
    int index = 0;
    uint32 start_of_chain = 0, end_of_chain = 0, meter_mode = 0;
    uint32 coupling_flag = 0;
    int size_pool;
    int policer_index = 0;
    int table_offset = 0;
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;

    _bcm_esw_get_policer_table_index(unit, policer_id, &policer_index);

    for (index=0; index < numbers; index++) {
        if (index > 0) {
            table_offset = policer_index + (pid_offset[index] * size_pool);
        } else {
            table_offset = policer_index;
        }
        /* read HW register */
        rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                                  table_offset,  &data);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read SVC METER TABLE at "
                                  "offset %d\n"), table_offset));
            return (rv);
        }
        switch (mode) {
            case bcmPolicerGroupModeCascade:
            case bcmPolicerGroupModeIntPriCascade:
                if (index == 0) {
                    end_of_chain = 1;
                } else {
                    end_of_chain = 0;
                }
                if (index == (numbers - 1)) {
                    start_of_chain = 1;
                } else {
                    start_of_chain = 0;
                }
                meter_mode = 1;
                coupling_flag = 0;
                break;


            case bcmPolicerGroupModeCascadeWithCoupling:
            case bcmPolicerGroupModeIntPriCascadeWithCoupling:
                if ((index == 0) || (index == (numbers / 2))) {
                    end_of_chain = 1;
                } else {
                    end_of_chain = 0;
                }
                if ((index == (numbers - 1)) || (index == ((numbers/2) - 1))) {
                    start_of_chain = 1;
                } else {
                    start_of_chain = 0;
                }
                meter_mode = 1;
                coupling_flag = 1;
                break;

            default:
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Invalid mode passed \n")));
                return BCM_E_NONE;
                break;
        }
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, START_OF_CHAINf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &data,
                                            START_OF_CHAINf, &start_of_chain);
        }
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, END_OF_CHAINf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &data,
                                               END_OF_CHAINf, &end_of_chain);
        }
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &data,
                                                    METER_MODEf, &meter_mode);
        }
        if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COUPLING_FLAGf)) {
            soc_SVM_METER_TABLEm_field_set(unit, &data,
                                             COUPLING_FLAGf, &coupling_flag);
        }
        /* write to hW */
        rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                                         table_offset, &data);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to write SVC METER TABLE at "
                                  "offset %d\n"), table_offset));
            return (rv);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_set_cascade_info_mef_10dot3_to_hw
 * Purpose:
 *      In case of MEF 10.3 cascade meter, write configuration info that is
 *      specific to cascade meters to HW
 * Parameters:
 *     unit                  - (IN) unit number
 *     numbers               - (IN) Number of policers
 *     policer_id            - (IN) base policer id
 *     mode                  - (IN) policer group mode
 *     pid_offset            - (IN) Offset w.r.t base policer
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_esw_global_meter_set_cascade_info_mef_10dot3_to_hw (int unit,
                                                         int  numbers,
                                                       bcm_policer_t policer_id,
                                                  bcm_policer_group_mode_t mode,
                                                         uint8 *pid_offset)
{
    int     rv = BCM_E_NONE;
    int     pool = 0;
    uint32  nxt_c = 0, nxt_e = 0, c_e = 0, pair_e = 0;
    uint32  nxt_c_valid = 0, nxt_e_valid = 0, pair_e_valid = 0;
    uint32  meter_mode = 0;
    int     size_pool;
    int     policer_index = 0;
    int     table_offset = 0;
    svm_meter_table_entry_t svm_entry;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    _bcm_esw_get_policer_table_index(unit, policer_id, &policer_index);

    for (pool = 0; pool < numbers; pool++) {
        if (pool > 0) {
            table_offset = policer_index + (pid_offset[pool] * size_pool);
        } else {
            table_offset = policer_index;
        }

        /* read HW register */
        rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                          table_offset,  &svm_entry);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read SVC METER TABLE at "
                                  "offset %d\n"), table_offset));
            return (rv);
        }

        meter_mode = 1;

        switch (mode) {
        case bcmPolicerGroupModeCascade:
        case bcmPolicerGroupModeIntPriCascade:
        {
            /* For non GCF mode, overflow fields need to be over written
             * during individual policer_set if policer mode is passed
             * as CoupledCascaded
             */
            c_e  = 0;
            nxt_c_valid  = 1; /* Should the last C_Valid always be true? */
            nxt_e_valid = (pool == (numbers -1)) ? 0 : 1;
            nxt_c  = nxt_e = (pool == (numbers -1)) ? 0 : pool + 1;
        }
        break;

        case bcmPolicerGroupModeCascadeWithCoupling: /* Pass through */
        case bcmPolicerGroupModeIntPriCascadeWithCoupling:
        {
            /* Set GCF configuration, it is 2 stage process
             *     Stage 1 - Set committed cascaded chain
             *     Stage 2 - Set excess cascaded chain
             */

            if (pool < 4) {
                /* Stage 1 - committed chain */
                c_e          =  (pool == (numbers/2 -1)) ? 1 : 0;
                nxt_c_valid  = 1;
                nxt_c        = pool + 1;
                nxt_e_valid  = 0;
                nxt_e        = 0;
                pair_e_valid = 1;
                pair_e       = pool + 4;
            } else {
                /* Stage 2 - excess chain */
                c_e          = 0;
                nxt_c_valid  = 0;
                nxt_c        = 0;
                nxt_e_valid  = (pool == (numbers -1)) ? 0 : 1;
                nxt_e        = (pool == (numbers -1)) ? 0 : pool + 1;
                pair_e_valid = 0;
                pair_e       = 0;
            }

            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                           METER_PAIR_EXCESS_BUCKETf,
                                           &pair_e);
            soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                           METER_PAIR_EXCESS_BUCKET_VALIDf,
                                           &pair_e_valid);
        }
        break;

        default:
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid mode passed \n")));
            return BCM_E_NONE;
        }

        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       NXT_EXCESS_BUCKETf,
                                       &nxt_e);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       NXT_EXCESS_BUCKET_VALIDf,
                                       &nxt_e_valid);

        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       NXT_COMMITTED_BUCKETf,
                                       &nxt_c);
        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       NXT_COMMITTED_BUCKET_VALIDf,
                                       &nxt_c_valid);

        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       COMMITTED_EXCESSf,
                                       &c_e);

        soc_SVM_METER_TABLEm_field_set(unit, &svm_entry,
                                       METER_MODEf, &meter_mode);

        /* write to hW */
        rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                           table_offset, &svm_entry);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to write SVC METER TABLE at "
                                  "offset %d\n"), table_offset));
            return (rv);
        }
    } /* End for loop */

    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_set_offset_table_map_to_increasing_value
 * Purpose:
 *     Set first n entires of the offset table with a incremental value.
 *     starting from 0
 * Parameters:
 *     num_offsets           - (IN) Number of offsets
 *     offset_map            - (IN/OUT) Offset Map
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_set_offset_table_map_to_increasing_value(uint32 num_offsets,
                               offset_table_entry_t *offset_map)
{
    int rv = BCM_E_NONE;
    int i = 0;
    if (num_offsets >= BCM_SVC_METER_MAP_SIZE_256) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Number of offsets passed is more than map table "
                            "size %d\n"), num_offsets));
        return BCM_E_INTERNAL;
    }
    for(i = 0; i < num_offsets; i++) {
        offset_map[i].offset= i;
        offset_map[i].meter_enable = 1;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_set_offset_table_map_to_a_value_with_pool
 * Purpose:
 *     Set first n entires of the offset table with a specified value.
 * Parameters:
 *     Coupling              - (IN) Coupling flag
 *     num_offsets           - (IN) Number of offsets
 *     value                 - (IN) Value to be set
 *     offset_map            - (IN/OUT) Offset Map
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_set_offset_table_map_to_a_value_with_pool(int coupling,
                               uint32 num_offsets,
                               uint32 value,
                               offset_table_entry_t *offset_map)
{
    int rv = BCM_E_NONE;
    int i = 0, j = 0;
    int pool_offset_limit;

    pool_offset_limit = (coupling) ? BCM_POLICER_GLOBAL_METER_MAX_POOL/2 :
                                        BCM_POLICER_GLOBAL_METER_MAX_POOL;
    if (num_offsets > BCM_SVC_METER_MAP_SIZE_256) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Number of offsets passed is more than map table "
                            "size %d\n"), num_offsets));
        return BCM_E_INTERNAL;
    }
    if (num_offsets) {
        if (!soc_feature(0,soc_feature_global_meter_mef_10dot3)) {
            /* pre MEF 10.3 device */
            i = num_offsets - 1;
            for(; i >= 0; i--) {
                if (coupling) {
                    offset_map[j].pool = j + 4;
                } else {
                    offset_map[j].pool = j;
                }
                offset_map[j].offset= value;
                offset_map[j].meter_enable = 1;
                j++;
            }
        } else {
            /* MEF 10.3 onward device */
            for (j = 0; j < num_offsets; j++) {
                if (soc_feature(0,soc_feature_global_meter_pool_priority_descending))  {
                    offset_map[j].pool = pool_offset_limit - j - 1;
                } else {
                    offset_map[j].pool = j;
                }
                offset_map[j].offset= value;
                offset_map[j].meter_enable = 1;
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_set_offset_table_map_flex_pool
 * Purpose:
 *     Set first n entires of the offset table with a specified value.
 * Parameters:
 *     Coupling              - (IN) Coupling flag
 *     num_offsets           - (IN) Number of offsets
 *     pool_offset           - (IN) Values to be set as offsets
 *     offset_map            - (IN/OUT) Offset Map
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_set_offset_table_map_flex_pool(int coupling,
                               uint32 num_offsets,
                               uint32 *pool_offset,
                               offset_table_entry_t *offset_map)
{
    int rv = BCM_E_NONE;
    int i = 0, j = 0;
    int pool_offset_limit;

    if (num_offsets > BCM_SVC_METER_MAP_SIZE_256) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Number of offsets passed is more than map table "
                            "size %d\n"), num_offsets));
        return BCM_E_INTERNAL;
    }

    pool_offset_limit = (coupling) ? BCM_POLICER_GLOBAL_METER_MAX_POOL/2 :
                                        BCM_POLICER_GLOBAL_METER_MAX_POOL;

    if (num_offsets) {
        if (!soc_feature(0,soc_feature_global_meter_mef_10dot3)) {
            i = num_offsets - 1;
            for(; i >= 0; i--) {
                if (coupling) {
                    offset_map[j].pool = pool_offset[j] + 4;
                } else {
                    offset_map[j].pool = pool_offset[j];
                }
                offset_map[j].offset= 0;
                offset_map[j].meter_enable = 1;
                j++;
            }
        } else {
            /* MEF 10.3 onward devices */
            for (j = 0; j < num_offsets; j++) {
                if (soc_feature(0,soc_feature_global_meter_pool_priority_descending))  {
                    offset_map[j].pool = pool_offset_limit - pool_offset[j] - 1;
                } else {
                    offset_map[j].pool = pool_offset[j];
                }
                offset_map[j].offset = 0;
                offset_map[j].meter_enable = 1;
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_set_offset_table_map_to_a_value
 * Purpose:
 *     Set first n entires of the offset table with a specified value.
 * Parameters:
 *     num_offsets           - (IN) Number of offsets
 *     value                 - (IN) Value to be set
 *     offset_map            - (IN/OUT) Offset Map
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_set_offset_table_map_to_a_value(uint32 num_offsets,
                               uint32 value,
                               offset_table_entry_t *offset_map)
{
    int rv = BCM_E_NONE;
    int i = 0;
    if (num_offsets >= BCM_SVC_METER_MAP_SIZE_256) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Number of offsets passed is more than map table "
                            "size %d\n"), num_offsets));
        return BCM_E_INTERNAL;
    }
    for(i = 0; i < num_offsets; i++) {
        offset_map[i].offset= value;
        offset_map[i].meter_enable = 1;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_set_offset_table_map_at_offsets_with_flex_pool
 * Purpose:
 *     Create the map of offset table with varying pool of policers that needs
 *     to be written to different offsets
 * Parameters:
 *     Coupling              - (IN) Coupling flag
 *     num_offsets           - (IN) Number of offsets
 *     table_offset          - (IN) Offset positons
 *     pool_offset           - (IN) Values to be set as offsets
 *     offset_map            - (IN/OUT) Offset Map
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_esw_policer_set_offset_table_map_at_offsets_with_flex_pool(int coupling,
                               uint32 num_offsets,
                               uint32 *table_offset,
                               uint32 *pool_offset,
                               offset_table_entry_t *offset_map)
{
    int rv = BCM_E_NONE;
    int i = 0, j = 0;
    int pool_offset_limit;

    if (num_offsets > BCM_SVC_METER_MAP_SIZE_256) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META("Number of offsets passed is more than map table "
                            "size %d\n"), num_offsets));
        return BCM_E_INTERNAL;
    }

    pool_offset_limit = (coupling) ? BCM_POLICER_GLOBAL_METER_MAX_POOL/2 :
                                        BCM_POLICER_GLOBAL_METER_MAX_POOL;

    if (num_offsets) {
        if (!soc_feature(0,soc_feature_global_meter_mef_10dot3)) {
            i = num_offsets - 1;
            for(j = 0; j <= i; j++) {
                if ((table_offset[j] >= BCM_SVC_METER_MAP_SIZE_256) ||
                        (pool_offset[j] >= pool_offset_limit)) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                            (BSL_META("Offset/pool passed is out of range "
                                      "size %d %d\n"), table_offset[j], pool_offset[j]));
                    return BCM_E_PARAM;
                }

                if (coupling) {
                    offset_map[table_offset[j]].pool = pool_offset[j] + 4;
                } else {
                    offset_map[table_offset[j]].pool = pool_offset[j];
                }
                offset_map[table_offset[j]].offset= 0;
                offset_map[table_offset[j]].meter_enable = 1;
            }
        } else {
            /* MEF 10.3 onward devices */
            for (j = 0; j < num_offsets; j++) {
                if ((table_offset[j] >= BCM_SVC_METER_MAP_SIZE_256) ||
                        (pool_offset[j] >= pool_offset_limit)) {
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                            (BSL_META("Offset/pool passed is out of range "
                                      "size %d %d\n"), table_offset[j], pool_offset[j]));
                    return BCM_E_PARAM;
                }

                if (soc_feature(0,soc_feature_global_meter_pool_priority_descending))  {
                    offset_map[table_offset[j]].pool = pool_offset_limit - pool_offset[j] - 1;
                } else {
                    offset_map[table_offset[j]].pool = pool_offset[j];
                }
                offset_map[table_offset[j]].offset = 0;
                offset_map[table_offset[j]].meter_enable = 1;
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_set_offset_table_map
 * Purpose:
 *     Create the map of policer offsets that needs to be written to offset
 *     table
 * Parameters:
 *     num_offsets           - (IN) Number of offsets
 *     table_offset          - (IN) Offset positons
 *     value                 - (IN) Values to be set
 *     offset_map            - (IN/OUT) Offset Map
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_set_offset_table_map(uint32 num_offsets,
                               uint32 *table_offset,
                               uint32 *value,
                               offset_table_entry_t *offset_map)
{
    int rv = BCM_E_NONE;
    int i = 0;
    for(i = 0; i < num_offsets; i++) {
        if ((table_offset[i] >= BCM_SVC_METER_MAP_SIZE_256) ||
               (value[i] >= BCM_SVC_METER_MAP_SIZE_256)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META("Offset/value passed is more than map table "
                                "size %d %d\n"), table_offset[i], value[i]));
            return BCM_E_INTERNAL;
        }
        offset_map[table_offset[i]].offset= value[i];
        offset_map[table_offset[i]].meter_enable = 1;
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_group_set_mode_and_map
 * Purpose:
 *
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) Policer group mode
 *     npolicers             - (IN) Number of policers
 *     direction             - (IN) Horizontal/Vertical
 *     mode_attr             - (IN) Offset mode attributes
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_group_set_mode_and_map( int unit,
                               bcm_policer_group_mode_t mode,
                               int *npolicers, bcm_policer_map_t *mapping,
                               uint32 *direction,
                               bcm_policer_svc_meter_attr_t *mode_attr)
{
    int rv = BCM_E_NONE;
    int num_pools = 0;
    uint32 table_offset[16] = { 0 };
    uint32 value[16] = { 0 };
    pkt_attr_bits_t  *pkt_attr = NULL;
    uint32                   map_index = 0;
    compressed_pkt_res_attr_map_t *pkt_res_map = NULL;
    compressed_pri_cnf_attr_map_t  *pri_cnf_map = NULL;
    offset_table_entry_t   *offset_map = NULL;
    uint32                  type = 0;
    uint32                  offset = 0;
    uint8                   *pkt_res = NULL;
    int                     no_entries_to_populate = 0;
    int                     shift_bits = 0;
    int                     index_max = 0;
    int                     i = 0;
    uint8 *val = NULL;

    mode_attr->mode_type_v = uncompressed_mode;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    if (mapping != NULL) {
        val  = mapping->offset_map;
    }

    if (BCM_SVM_DEV_ATTR(unit)->pkt_resolution != NULL) {
        pkt_res = BCM_SVM_DEV_ATTR(unit)->pkt_resolution;
    } else {
        /*
         * Other devices do not support service meters, return error.
         */
        return (BCM_E_UNAVAIL);
    }

    switch (mode) {
        /* A single policer used for all traffic types */
        case bcmPolicerGroupModeSingle:
        {
            *npolicers = 1;
            rv =  _bcm_esw_policer_set_offset_table_map_to_a_value(
                      FP_RESOLUTION_MAX, 0,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* ********************************************************************/
         /* A dedicated policer per traffic type Unicast,multicast,broadcast  */
         /* 1) L2UC_PKT | KNOWN_L3UC_PKT | UNKNOWN_L3UC_PKT                   */
         /* 2) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_IPMC_PKT|UNKNOWN_IPMC_PKT*/
         /* 3) L2BC_PKT|                                                      */
         /* *******************************************************************/
        case bcmPolicerGroupModeTrafficType:
        {
            *npolicers = 3;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
                table_offset[0] = pkt_res[KNOWN_L2UC_PKT];
                table_offset[1] = pkt_res[KNOWN_L3UC_PKT];
                table_offset[2] = pkt_res[UNKNOWN_L3UC_PKT];
                table_offset[3] = pkt_res[KNOWN_L2MC_PKT];
                table_offset[4] = pkt_res[UNKNOWN_L2MC_PKT];
                table_offset[5] = pkt_res[L2BC_PKT];
                table_offset[6] = pkt_res[UNKNOWN_L2UC_PKT];
                table_offset[7] = pkt_res[UNKNOWN_IPMC_PKT];
                table_offset[8] = pkt_res[KNOWN_IPMC_PKT];
                value[3] = 1;
                value[4] = 1;
                value[5] = 2;
                value[7] = 1;
                value[8] = 1;

            rv =  _bcm_esw_policer_set_offset_table_map(9, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /************************************************************** */
         /* A pair of policers where the base policer is used for dlf and */
         /* the other is used for all traffic types                       */
         /* 1) L2DLF_PKT(UNKNOWN_L2UC_PKT)                                */
         /* 2) UNKNOWN_PKT | CONTROL_PKT|BPDU_PKT|L2BC_PKT|L2UC_PKT|      */
         /*    UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|            */
         /*    UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|          */
         /*    UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|         */
         /*    KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|          */
         /*    UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                   */
         /* ************************************************************* */

        case bcmPolicerGroupModeDlfAll:
        {
            *npolicers = 2;
            mode_attr->uncompressed_attr_selectors_v.\
                     uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            rv =  _bcm_esw_policer_set_offset_table_map_to_a_value(
                      FP_RESOLUTION_MAX, 1,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            table_offset[0] = pkt_res[UNKNOWN_L2UC_PKT];
            value[0] = 0;
            rv =  _bcm_esw_policer_set_offset_table_map(1, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ******************************************************* */
         /* A dedicated policer for unknown unicast, known unicast, */
         /* multicast, broadcast                                    */
         /* 1) UNKNOWN_L3UC_PKT                                     */
         /* 2) L2UC_PKT | KNOWN_L3UC_PKT                            */
         /* 3) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT |                    */
         /*    KNOWN_IPMC_PKT | UNKNOWN_IPMC_PKT                    */
         /* 4) L2BC_PKT                                             */
         /* ******************************************************* */
        case bcmPolicerGroupModeTyped:
        {
            *npolicers = 4;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            table_offset[0] = pkt_res[UNKNOWN_L3UC_PKT];
            table_offset[1] = pkt_res[KNOWN_L3UC_PKT];
            table_offset[2] = pkt_res[KNOWN_L2UC_PKT];
            table_offset[3] = pkt_res[KNOWN_L2MC_PKT];
            table_offset[4] = pkt_res[UNKNOWN_L2MC_PKT];
            table_offset[5] = pkt_res[L2BC_PKT];
            table_offset[6] = pkt_res[UNKNOWN_L2UC_PKT];
            table_offset[7] = pkt_res[UNKNOWN_IPMC_PKT];
            table_offset[8] = pkt_res[KNOWN_IPMC_PKT];
            value[1] = 1;
            value[2] = 1;
            value[3] = 2;
            value[4] = 2;
            value[5] = 3;
            value[7] = 2;
            value[8] = 2;
            rv =  _bcm_esw_policer_set_offset_table_map(9, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ******************************************************* */
         /* A dedicated policer for unknown unicast, known unicast, */
         /* multicast, broadcast and one for all traffic(not already*/
         /* counted)                                                */
         /* 1) UNKNOWN_L3UC_PKT                                     */
         /* 2) L2UC_PKT | KNOWN_L3UC_PKT                            */
         /* 3) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                      */
         /*    KNOWN_IPMC_PKT | UNKNOWN_IPMC_PKT                    */
         /* 4) L2BC_PKT                                             */
         /* 5) UNKNOWN_PKT|CONTROL_PKT|BPDU_PKT|L2DLF_PKT|          */
         /*    UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_MPLS_PKT |     */
         /*    KNOWN_MPLS_L3_PKT|KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT */
         /*    KNOWN_MIM_PKT|UNKNOWN_MIM_PKT|                       */
         /*    KNOWN_MPLS_MULTICAST_PKT                             */
         /* ******************************************************* */
        case bcmPolicerGroupModeTypedAll:
        {
            *npolicers = 5;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            rv =  _bcm_esw_policer_set_offset_table_map_to_a_value(
                      FP_RESOLUTION_MAX, 4,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            table_offset[0] = pkt_res[UNKNOWN_L3UC_PKT];
            table_offset[1] = pkt_res[KNOWN_L3UC_PKT];
            table_offset[2] = pkt_res[KNOWN_L2UC_PKT];
            table_offset[3] = pkt_res[KNOWN_L2MC_PKT];
            table_offset[4] = pkt_res[UNKNOWN_L2MC_PKT];
            table_offset[5] = pkt_res[L2BC_PKT];
            table_offset[6] = pkt_res[UNKNOWN_L2UC_PKT];
            table_offset[7] = pkt_res[KNOWN_IPMC_PKT];
            table_offset[8] = pkt_res[UNKNOWN_IPMC_PKT];
            value[1] = 1;
            value[2] = 1;
            value[3] = 2;
            value[4] = 2;
            value[5] = 3;
            value[7] = 2;
            value[8] = 2;
            rv =  _bcm_esw_policer_set_offset_table_map(9, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* **************************************************************   */
         /* A single policer used for all traffic types with an additional   */
         /* policer for control traffic                                      */
         /* 1) UNKNOWN_PKT|L2BC_PKT|L2UC_PKT|L2DLF_PKT|                      */
         /*    UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|               */
         /*    UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|             */
         /*    UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|            */
         /*    KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|             */
         /*    UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                      */
         /* 2) CONTROL_PKT|BPDU_PKT                                          */
         /* **************************************************************   */
        case bcmPolicerGroupModeSingleWithControl:
        {
            *npolicers = 2;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            rv =  _bcm_esw_policer_set_offset_table_map_to_a_value(
                      FP_RESOLUTION_MAX, 0,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            table_offset[0] = pkt_res[CONTROL_PKT];
            table_offset[1] = pkt_res[BPDU_PKT];
            value[0] = 1;
            value[1] = 1;
            rv =  _bcm_esw_policer_set_offset_table_map(2, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ********************************************************  */
         /* A dedicated policer per traffic type unicast, multicast,  */
         /* broadcast with an additional counter for control traffic  */
         /* 1) L2UC_PKT | KNOWN_L3UC_PKT | UNKNOWN_L3UC_PKT           */
         /* 2) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|                       */
         /*    KNOWN_IPMC_PKT | UNKNOWN_IPMC_PKT                      */
         /* 3) L2BC_PKT|                                              */
         /* 4) CONTROL_PKT|BPDU_PKT                                   */
         /* ********************************************************  */
        case bcmPolicerGroupModeTrafficTypeWithControl:
        {
            *npolicers = 4;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            table_offset[0] = pkt_res[UNKNOWN_L3UC_PKT];
            table_offset[1] = pkt_res[KNOWN_L3UC_PKT];
            table_offset[2] = pkt_res[KNOWN_L2UC_PKT];
            table_offset[3] = pkt_res[KNOWN_L2MC_PKT];
            table_offset[4] = pkt_res[UNKNOWN_L2MC_PKT];
            table_offset[5] = pkt_res[L2BC_PKT];
            table_offset[6] = pkt_res[CONTROL_PKT];
            table_offset[7] = pkt_res[BPDU_PKT];
            table_offset[8] = pkt_res[UNKNOWN_L2UC_PKT];
            table_offset[9] = pkt_res[UNKNOWN_IPMC_PKT];
            table_offset[10] = pkt_res[KNOWN_IPMC_PKT];
            value[3] = 1;
            value[4] = 1;
            value[5] = 2;
            value[6] = 3;
            value[7] = 3;
            value[9] = 1;
            value[10] = 1;
            rv =  _bcm_esw_policer_set_offset_table_map(11, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ************************************************************** */
         /* A set of 3 policers where the base policer is used for control, */
         /* the next one for dlf and the other counter is used for all     */
         /* traffic types                                                  */
         /* 1) CONTROL_PKT|BPDU_PKT                                        */
         /* 2) L2DLF_PKT(UNKNOWN_L2UC_PKT)                                 */
         /* 3)UNKNOWN_PKT | CONTROL_PKT|BPDU_PKT|L2BC_PKT|L2UC_PKT|        */
         /*   UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT|KNOWN_L2MC_PKT|              */
         /*   UNKNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_L3UC_PKT|            */
         /*   UNKNOWN_L3UC_PKT|KNOWN_MPLS_PKT|KNOWN_MPLS_L3_PKT|           */
         /*   KNOWN_MPLS_L2_PKT|UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|            */
         /*   UNKNOWN_MIM_PKT|KNOWN_MPLS_MULTICAST_PKT                     */
         /* ************************************************************** */
        case bcmPolicerGroupModeDlfAllWithControl:
        {
            *npolicers = 3;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            rv =  _bcm_esw_policer_set_offset_table_map_to_a_value(
                      FP_RESOLUTION_MAX, 2,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            table_offset[0] = pkt_res[CONTROL_PKT];
            table_offset[1] = pkt_res[BPDU_PKT];
            table_offset[2] = pkt_res[UNKNOWN_L2UC_PKT];
            value[2] = 1;
            rv =  _bcm_esw_policer_set_offset_table_map(3, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* **************************************************************** */
         /* A dedicated policer for control, unknown unicast, known unicast, */
         /* multicast, broadcast                                             */
         /* 1) CONTROL_PKT|BPDU_PKT                                          */
         /* 2) UNKNOWN_L3UC_PKT                                              */
         /* 3) KNOWN_L2UC_PKT | KNOWN_L3UC_PKT                               */
         /* 4) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|                              */
         /*    KNOWN_IPMC_PKT| UNKNOWN_IPMC_PKT                              */
         /* 5) L2BC_PKT                                                      */
         /* **************************************************************** */
        case bcmPolicerGroupModeTypedWithControl:
        {
            *npolicers = 5;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            table_offset[0] = pkt_res[CONTROL_PKT];
            table_offset[1] = pkt_res[BPDU_PKT];
            table_offset[2] = pkt_res[UNKNOWN_L3UC_PKT];
            table_offset[3] = pkt_res[KNOWN_L2UC_PKT];
            table_offset[4] = pkt_res[KNOWN_L3UC_PKT];
            table_offset[5] = pkt_res[KNOWN_L2MC_PKT];
            table_offset[6] = pkt_res[UNKNOWN_L2MC_PKT];
            table_offset[7] = pkt_res[L2BC_PKT];
            table_offset[8] = pkt_res[UNKNOWN_L2UC_PKT];
            table_offset[9] = pkt_res[UNKNOWN_IPMC_PKT];
            table_offset[10] = pkt_res[KNOWN_IPMC_PKT];
            value[2] = 1;
            value[3] = 2;
            value[4] = 2;
            value[5] = 3;
            value[6] = 3;
            value[7] = 4;
            value[8] = 1;
            value[9] = 3;
            value[10] = 3;
            rv =  _bcm_esw_policer_set_offset_table_map(11, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* ***************************************************************** */
         /* A dedicated policer for control, unknown unicast, known unicast,  */
         /* multicast, broadcast and one for all traffic (not already policed)*/
         /* 1) CONTROL_PKT|BPDU_PKT                                           */
         /* 2) UNKNOWN_L3UC_PKT                                               */
         /* 3) KNOWN_L2UC_PKT | KNOWN_L3UC_PKT                                */
         /* 4) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT|KNOWN_IPMC_PKT|UNKNOWN_IPMC_PKT*/
         /* 5) L2BC_PKT                                                       */
         /* 6) UNKNOWN_PKT|L2DLF_PKT|UNKNOWN_IPMC_PKT|KNOWN_IPMC_PKT          */
         /*    KNOWN_MPLS_PKT | KNOWN_MPLS_L3_PKT|KNOWN_MPLS_L2_PKT|          */
         /*    UNKNOWN_MPLS_PKT|KNOWN_MIM_PKT|UNKNOWN_MIM_PKT|                */
         /*    KNOWN_MPLS_MULTICAST_PKT                                       */
         /* ***************************************************************** */
        case bcmPolicerGroupModeTypedAllWithControl:
        {
            *npolicers = 6;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
               BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
            rv =  _bcm_esw_policer_set_offset_table_map_to_a_value(
                      FP_RESOLUTION_MAX, 5,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            table_offset[0] = pkt_res[CONTROL_PKT];
            table_offset[1] = pkt_res[BPDU_PKT];
            table_offset[2] = pkt_res[UNKNOWN_L3UC_PKT];
            table_offset[3] = pkt_res[KNOWN_L2UC_PKT];
            table_offset[4] = pkt_res[KNOWN_L3UC_PKT];
            table_offset[5] = pkt_res[KNOWN_L2MC_PKT];
            table_offset[6] = pkt_res[UNKNOWN_L2MC_PKT];
            table_offset[7] = pkt_res[L2BC_PKT];
            table_offset[8] = pkt_res[UNKNOWN_L2UC_PKT];
            table_offset[9] = pkt_res[UNKNOWN_IPMC_PKT];
            table_offset[10] = pkt_res[KNOWN_IPMC_PKT];
            value[2] = 1;
            value[3] = 2;
            value[4] = 2;
            value[5] = 3;
            value[6] = 3;
            value[7] = 4;
            value[8] = 1;
            value[9] = 3;
            value[10] = 3;
            rv =  _bcm_esw_policer_set_offset_table_map(11, &table_offset[0],
                      &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* ******************************************************** */
        /* A set of 8(2^3) policers selected based on outer Vlan pri*/
        /* outer_dot1p; 3 bits 1..8                                 */
        /* ******************************************************** */

        case bcmPolicerGroupModeDot1P:
        {
            *npolicers = 8;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                        *npolicers,
                       &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                                            &table_offset[0],
                                                            &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }

            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* ******************************************************** */
        /* A set of 8(2^3) policers selected based on inner Vlan pri*/
        /* inner_dot1p; 3 bits 1..8                                 */
        /* ******************************************************** */

        case bcmPolicerGroupModeInnerDot1P:
        {
            *npolicers = 8;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                       *npolicers,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                                            &table_offset[0],
                                                            &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }

            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* **************************************************** */
        /* A set of 16(2^4) policers based on internal priority */
        /* 1..16 INT_PRI bits: 4bits                            */
        /* **************************************************** */
        case bcmPolicerGroupModeIntPri:
        {
            *npolicers = 16;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                        *npolicers,
                       &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                                            &table_offset[0],
                                                            &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* ********************************************************** */
        /* set of 64 policers(2^(4+2)) based on Internal priority+CNG */
        /* 1..64 (INT_PRI bits: 4bits + CNG 2 bits                    */
        /* 1..64 (INT_PRI bits: 4bits + CNG 2 bits                    */
        /* ********************************************************** */
        case bcmPolicerGroupModeIntPriCng:
        {
            *npolicers = 64;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_CNG_ATTR_BITS |
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                       *npolicers,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                                            &table_offset[0],
                                                            &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
        /* ********************************************************** */
        /* set of 8 policers(2^3) based on Internal priority          */
        /* 1..8 INT_PRI bits: 3bits                                  */
        /* ********************************************************** */
        case bcmPolicerGroupModeShortIntPri:
        {
            *npolicers = 8;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SHORT_INT_PRI_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                      8,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                                            &table_offset[0],
                                                            &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }

            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ****************************************** */
         /* A set of 2 policers(2^1) based on SVP type */
         /* 1..2 (SVP 1 bit). In case of KT2, SVP is 3 */
         /* 3 bits and hence no of policers is 8       */
         /* ********************************************/
        case bcmPolicerGroupModeSvpType:
        {
            *npolicers = (1 << BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size);
            mode_attr->uncompressed_attr_selectors_v.\
                   uncompressed_attr_bits_selector =
                     BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                       *npolicers,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                &table_offset[0], &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }
            BCM_IF_ERROR_RETURN(rv);
            break;
        }

         /* ******************************************** */
         /* A set of 64 policers(2^6) based on DSCP bits */
         /* 1..64 (6 bits from TOS 8 bits)               */
         /* ******************************************** */
        case bcmPolicerGroupModeDscp:

        {
            *npolicers = 64;
            mode_attr->uncompressed_attr_selectors_v.\
                        uncompressed_attr_bits_selector =
                      BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_TOS_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_increasing_value(
                       *npolicers,
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map(*npolicers,
                                &table_offset[0], &value[0],
                      &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }
            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ******************************************** */
         /* A set of n policers based on user input */
         /* ******************************************** */
        case bcmPolicerGroupModeCascade:
            if ((*npolicers > num_pools) || (*npolicers == 0 )) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Invalid number of Policers \n")));
                return BCM_E_PARAM;
            }
            *npolicers = 8;  /* Numbers of meters in a cascade chain is 8 */
            *direction = GLOBAL_METER_ALLOC_HORIZONTAL;
            mode_attr->mode_type_v = cascade_mode;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_a_value_with_pool(
                        0, *npolicers, 0,
                        &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map_flex_pool(
                        0, *npolicers, &value[0] ,
                       &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
           }
           BCM_IF_ERROR_RETURN(rv);
            break;
           /* ******************************************** */
         /* A set of n policers based on user input based on IntPri*/
         /* ******************************************** */
        case bcmPolicerGroupModeIntPriCascade:
        {
            if ((*npolicers > num_pools) || (*npolicers == 0 )) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Invalid number of Policers \n")));
                return BCM_E_PARAM;
            }
            *npolicers = 8;  /* Numbers of meters in a cascade chain is 8 */
            *direction = GLOBAL_METER_ALLOC_HORIZONTAL;
            mode_attr->mode_type_v = cascade_mode;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                      BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SHORT_INT_PRI_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_a_value_with_pool(
                          0, *npolicers, 0,
                          &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map_flex_pool(
                        0, *npolicers, &value[0] ,
                       &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }

            BCM_IF_ERROR_RETURN(rv);
            break;
        }

         /* ******************************************** */
         /* A set of n policers based on user input */
         /* ******************************************** */
        case bcmPolicerGroupModeCascadeWithCoupling:
        {
            if ((*npolicers > 0 ) &&
                 ((*npolicers * 2) <= num_pools)) {
                *direction = GLOBAL_METER_ALLOC_HORIZONTAL;
                *npolicers = 8; /* Allocate all the 8 policers */
            } else {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Invalid number of Policers \n")));
                return BCM_E_PARAM;
            }
            mode_attr->mode_type_v = cascade_mode;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                  BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_a_value_with_pool(
                        1, *npolicers/2, 0,
                        &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map_flex_pool(
                       1, *npolicers/2, &value[0] ,
                       &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }

            BCM_IF_ERROR_RETURN(rv);
            break;
        }
         /* ******************************************** */
         /* A set of n policers based on user input */
         /* ******************************************** */
        case bcmPolicerGroupModeIntPriCascadeWithCoupling:
        {
            if ((*npolicers > 0 ) &&
                 ((*npolicers * 2) <= num_pools)) {
                *direction = GLOBAL_METER_ALLOC_HORIZONTAL;
                *npolicers = 8;
            } else {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Invalid number of Policers \n")));
                return BCM_E_PARAM;
            }
            mode_attr->mode_type_v = cascade_mode;
            mode_attr->uncompressed_attr_selectors_v.\
                      uncompressed_attr_bits_selector =
                      BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SHORT_INT_PRI_ATTR_BITS;
            if (mapping == NULL) {
                rv =  _bcm_esw_policer_set_offset_table_map_to_a_value_with_pool(
                        1, *npolicers/2, 0,
                        &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            } else {
                for (i = 0; i < mapping->count; i++) {
                    table_offset[i] = i;
                    value[i] = *val;
                    val++;
                }
                rv =  _bcm_esw_policer_set_offset_table_map_flex_pool(
                       1, *npolicers/2, &value[0] ,
                       &mode_attr->uncompressed_attr_selectors_v.offset_map[0]);
            }

            BCM_IF_ERROR_RETURN(rv);
            break;
        }

         /* ************************************************************** */
         /* N+1 policers where in the base counter is used for dlf and next N */
         /* are used as per Cos                                            */
         /* 1) L2_DLF(UNKNOWN_L2UC_PKT)                                    */
         /* 2..17) INT_PRI bits: 4bits                                     */
         /* ************************************************************** */
        case bcmPolicerGroupModeDlfIntPri:
        {
            *npolicers = 17;
            mode_attr->mode_type_v = compressed_mode;
            pkt_attr = &mode_attr->compressed_attr_selectors_v.pkt_attr_bits_v;
            pkt_attr->pkt_resolution = 1;
            pkt_attr->int_pri = 4;

            /* Reset pkt_resolution map */
            pkt_res_map =  &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pkt_res_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PKT_RES_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pkt_res_map[map_index] = 0;
            }
            /* set pkt_resoltion map for DLF to 1.
             * PKT_RES_MAPm == [ pkt_res | svp_type | drop ]
             * So, Don't care for bits corresponding to svp_type and drop */
            shift_bits = BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size + 1;
            no_entries_to_populate = (1 << shift_bits);

            for (offset = 0; offset < no_entries_to_populate; offset++) {
                pkt_res_map[(pkt_res[UNKNOWN_L2UC_PKT] << shift_bits) + \
                                                      offset] = 1 << shift_bits;
            }
            /* Reset pri_cnf map */
            pri_cnf_map = &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pri_cnf_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PRI_CNG_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
               pri_cnf_map[map_index] = 0;
            }
            /* set pri_cnf map for  16 policers */
            for (offset = 0; offset < 4; offset++) {
                for (map_index = 0; map_index < 16; map_index++) {
                    pri_cnf_map[offset << 4 | map_index] = map_index;
                }
            }

            /* Reset all Offset table fields */
            offset_map = &mode_attr->compressed_attr_selectors_v.offset_map[0];
            for (map_index = 0; map_index < BCM_SVC_METER_MAP_SIZE_256;
                                                                map_index++) {
                offset_map[map_index].offset = 0;
                offset_map[map_index].meter_enable = 0;
            }
             /* Set DLF policer indexes considering INT_PRI bits don't care */
            for(map_index = 0; map_index < 16; map_index++) {
                offset_map[(map_index << 1) | 1].offset = 0;
                offset_map[(map_index << 1) | 1].meter_enable = 1;
            }
            /* Set Int pri policer indexes considering DLF=0 */
            for(map_index = 0; map_index < 16; map_index++) {
                offset_map[map_index << 1].offset = map_index + 1;
                offset_map[map_index << 1].meter_enable = 1;
            }
            break;
        }
         /* *************************************************************** */
         /* A dedicated policer for unknown unicast, known unicast,         */
         /* multicast,broadcast and N internal priority policers for traffic*/
         /* (not already policed)                                           */
         /* 1) UNKNOWN_L3UC_PKT                                             */
         /* 2) KNOWN_L2UC_PKT | KNOWN_L3UC_PKT                              */
         /* 3) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                              */
         /* 4) L2BC_PKT                                                     */
         /* 5..20) INT_PRI bits: 4bits                                      */
         /* *************************************************************** */
        case bcmPolicerGroupModeTypedIntPri:
        {
            *npolicers = 20;
            mode_attr->mode_type_v = compressed_mode;
            pkt_attr = &mode_attr->compressed_attr_selectors_v.pkt_attr_bits_v;
            pkt_attr->pkt_resolution = 3;
            pkt_attr->int_pri = 4;
            /* Reset pkt_resolution map */
            pkt_res_map =  &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pkt_res_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PKT_RES_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pkt_res_map[map_index] = 0;
            }
            /* PKT_RES_MAPm == [ pkt_res | svp_type | drop ]
             * So, Don't care for bits corresponding to svp_type and drop */
            shift_bits = BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size + 1;
            no_entries_to_populate = (1 << shift_bits);

            for (offset = 0; offset < no_entries_to_populate; offset++) {
                pkt_res_map[(pkt_res[UNKNOWN_L3UC_PKT] << shift_bits) + offset]
                                                              = 1 << shift_bits;
                pkt_res_map[(pkt_res[KNOWN_L2UC_PKT] << shift_bits) + offset]
                                                              = 2 << shift_bits;
                pkt_res_map[(pkt_res[KNOWN_L3UC_PKT] << shift_bits) + offset]
                                                              = 2 << shift_bits;
                pkt_res_map[(pkt_res[KNOWN_L2MC_PKT] << shift_bits) + offset]
                                                              = 3 << shift_bits;
                pkt_res_map[(pkt_res[UNKNOWN_L2MC_PKT] << shift_bits) + offset]
                                                              = 3 << shift_bits;
                pkt_res_map[(pkt_res[L2BC_PKT] << shift_bits) + offset]
                                                              = 4 << shift_bits;
                pkt_res_map[(pkt_res[UNKNOWN_L2UC_PKT] << shift_bits) + offset]
                                                              = 1 << shift_bits;
            }
            /* Reset pri_cng map */
            pri_cnf_map = &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pri_cnf_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PRI_CNG_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pri_cnf_map[map_index] = 0;
            }
            /* set pri_cnf map for  16 policers */
            for (map_index = 0; map_index < 16; map_index++) {
                pri_cnf_map[map_index] = map_index;
            }

            /* Reset all Offset table fields */
            offset_map = &mode_attr->compressed_attr_selectors_v.offset_map[0];
            for (map_index = 0; map_index < BCM_SVC_METER_MAP_SIZE_256;
                                                                map_index++) {
                offset_map[map_index].offset = 0;
                offset_map[map_index].meter_enable = 0;
            }
            /* Set Int pri policer indexes considering Type fields = 0 */
            for(map_index = 0; map_index < 16; map_index++) {
                for(type = 0; type < 8; type++) {
                   offset_map[type + (8 * map_index)].offset = map_index + 4;
                   offset_map[type + (8 * map_index)].meter_enable = 1;
               }
            }
             /* Set Type based indexes considering INT_PRI bits don't care */
            for(type = 0; type < 4; type++) {
                for(map_index = 0; map_index < 16; map_index++) {
                    offset_map[(map_index << 3) | (type + 1)].offset = type;
                    offset_map[(map_index << 3) | (type + 1)].meter_enable = 1;
                }
            }
            break;
        }
         /* ************************************************************* */
         /* N+2 policers where the base policers is used for control, the  */
         /* next one for dlf and next N are used per Cos                  */
         /* 1) CONTROL_PKT|BPDU_PKT                                       */
         /* 2) L2_DLF                                                     */
         /* 3..18) INT_PRI bits: 4bits                                    */
         /* ************************************************************* */
        case bcmPolicerGroupModeDlfIntPriWithControl:
        {
            *npolicers = 18;
            mode_attr->mode_type_v = compressed_mode;
            pkt_attr = &mode_attr->compressed_attr_selectors_v.pkt_attr_bits_v;
            pkt_attr->pkt_resolution = 2;
            pkt_attr->int_pri = 4;
            /* Reset pkt_resolution map */
            pkt_res_map =  &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pkt_res_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PKT_RES_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pkt_res_map[map_index] = 0;
            }
            /* PKT_RES_MAPm == [ pkt_res | svp_type | drop ]
             * So, Don't care for bits corresponding to svp_type and drop */
            shift_bits = BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size + 1;
            no_entries_to_populate = (1 << shift_bits);

            /* set pkt_resolution map */
            for (offset = 0; offset < no_entries_to_populate; offset++) {
                pkt_res_map[(pkt_res[CONTROL_PKT] << shift_bits) + offset]
                                                           = 1 << shift_bits;
                pkt_res_map[(pkt_res[BPDU_PKT] << shift_bits) + offset]
                                                           = 1 << shift_bits;
                pkt_res_map[(pkt_res[UNKNOWN_L2UC_PKT] << shift_bits) + offset]
                                                           = 2 << shift_bits;
            }
            /* Reset pri_cng map */
            pri_cnf_map = &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pri_cnf_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PRI_CNG_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pri_cnf_map[map_index] = 0;
            }
            /* set pri_cnf map for 16 policers */
            for (map_index = 0; map_index < 16; map_index++) {
                pri_cnf_map[map_index] = map_index;
            }

            /* Reset all Offset table fields */
            offset_map = &mode_attr->compressed_attr_selectors_v.offset_map[0];
            for (map_index = 0; map_index < BCM_SVC_METER_MAP_SIZE_256;
                                                                map_index++) {
                offset_map[map_index].offset = 0;
                offset_map[map_index].meter_enable = 0;
            }
            /* Set Int pri policer indexes considering Type fields = 0 */
            for(map_index = 0; map_index < 16; map_index++) {
               for(type = 0; type < 4; type++) {
                   offset_map[type + (4 * map_index)].offset = map_index + 2;
                   offset_map[type + (4 * map_index)].meter_enable = 1;
               }
            }
             /* Set Type based indexes considering INT_PRI bits don't care */
            for(type = 0; type < 2; type++) {
                for(map_index = 0; map_index < 16; map_index++) {
                    offset_map[(map_index << 2) | (type + 1)].offset = type;
                    offset_map[(map_index << 2) | (type + 1)].meter_enable = 1;
                }
            }
            break;
        }
        /* *************************************************************** */
         /* A dedicated counter for control, unknown unicast, known unicast */
         /* , multicast, broadcast and N internal priority counters for     */
         /* traffic (not already counted)                                   */
         /* 1) CONTROL_PKT|BPDU_PKT                                         */
         /* 2) UNKNOWN_L3UC_PKT                                             */
         /* 3) L2UC_PKT | KNOWN_L3UC_PKT                                    */
         /* 4) KNOWN_L2MC_PKT|UNKNOWN_L2MC_PKT                              */
         /* 5) L2BC_PKT                                                     */
         /* 6..21) INT_PRI bits: 4bits                                      */
         /* *************************************************************** */
        case bcmPolicerGroupModeTypedIntPriWithControl:
        {
            *npolicers = 21;
            mode_attr->mode_type_v = compressed_mode;
            pkt_attr = &mode_attr->compressed_attr_selectors_v.pkt_attr_bits_v;
            pkt_attr->pkt_resolution = 3;
            pkt_attr->int_pri = 4;
            /* Reset pkt_resolution map */
            pkt_res_map =  &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pkt_res_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PKT_RES_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pkt_res_map[map_index] = 0;
            }
            /* PKT_RES_MAPm == [ pkt_res | svp_type | drop ]
             * So, Don't care for bits corresponding to svp_type and drop */
            shift_bits = BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size + 1;
            no_entries_to_populate = (1 << shift_bits);

            /* set pkt_resolution map */
            for (offset = 0; offset < no_entries_to_populate; offset++) {
                pkt_res_map[(pkt_res[CONTROL_PKT] << shift_bits) + offset]
                                                             = 1 << shift_bits;
                pkt_res_map[(pkt_res[BPDU_PKT] << shift_bits) + offset]
                                                             = 1 << shift_bits;
                pkt_res_map[(pkt_res[UNKNOWN_L3UC_PKT] << shift_bits)+ offset]
                                                             = 2 << shift_bits;
                pkt_res_map[(pkt_res[KNOWN_L2UC_PKT] << shift_bits) + offset]
                                                             = 3 << shift_bits;
                pkt_res_map[(pkt_res[KNOWN_L3UC_PKT] << shift_bits) + offset]
                                                             = 3 << shift_bits;
                pkt_res_map[(pkt_res[KNOWN_L2MC_PKT] << shift_bits) + offset]
                                                             = 4 << shift_bits;
                pkt_res_map[(pkt_res[UNKNOWN_L2MC_PKT] << shift_bits) + offset]
                                                             = 4 << shift_bits;
                pkt_res_map[(pkt_res[L2BC_PKT] << shift_bits) + offset]
                                                             = 5 << shift_bits;
                pkt_res_map[(pkt_res[UNKNOWN_L2UC_PKT] << shift_bits)+ offset]
                                                             = 2 << shift_bits;
            }
            /* Reset pri_cnf map */
            pri_cnf_map = &mode_attr->compressed_attr_selectors_v.\
                                             compressed_pri_cnf_attr_map_v[0];
            index_max = soc_mem_index_max(unit, ING_SVM_PRI_CNG_MAPm);
            for (map_index = 0; map_index <= index_max; map_index++) {
                pri_cnf_map[map_index] = 0;
            }
            /* set pri_cnf map for  16 policers */
            for (map_index = 0; map_index < 16; map_index++) {
                pri_cnf_map[map_index] = map_index;
            }

            /* Reset all Offset table fields */
            offset_map = &mode_attr->compressed_attr_selectors_v.offset_map[0];
            for (map_index = 0; map_index < BCM_SVC_METER_MAP_SIZE_256;
                                                                map_index++) {
                offset_map[map_index].offset = 0;
                offset_map[map_index].meter_enable = 0;
            }
            /* Set Int pri policer indexes considering Type fields = 0 */
            for(map_index = 0; map_index < 16; map_index++) {
                for(type = 0; type < 8; type++) {
                    offset_map[type + (8 * map_index)].offset = map_index + 5;
                    offset_map[type + (8 * map_index)].meter_enable = 1;
                }
            }
             /* Set Type based indexes considering INT_PRI bits don't care */
            for(type = 0; type < 5; type++) {
                for(map_index = 0; map_index < 16; map_index++) {
                    offset_map[(map_index << 3) | (type + 1)].offset = type;
                    offset_map[(map_index << 3) | (type + 1)].meter_enable = 1;
                }
            }
            break;
        }

        default:
            LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Invalid policer group mode\n")));
            return BCM_E_PARAM;

    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_group_create
 * Purpose:
 *      Create a group of policers based on the mode.
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) Policer group mode
 *     flow_info             - (IN) Information regarding flow type and skip pool
 *     offset_map            - (IN) Attr_value to policer_offset mapping
 *     policer_id            - (OUT) Base policer id
 *     npolicers             - (OUT) Number of policers
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_group_create(int unit, bcm_policer_group_mode_t mode,
                                   _bcm_policer_flow_info_t *flow_info,
                                   bcm_policer_map_t *offset_map,
                                   bcm_policer_t *policer_id, int *npolicers)
{
    int rv = BCM_E_NONE;
    uint32 direction = 0;
    int index = 0;
    bcm_policer_svc_meter_mode_t offset_mode = 0;
    uint8 pid_offset[BCM_POLICER_GLOBAL_METER_MAX_POOL] = {0};
    _global_meter_policer_control_t *policer_control = NULL;
    bcm_policer_svc_meter_attr_t *mode_attr = NULL;
    int offset_mask = 0;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    mode_attr = sal_alloc(sizeof(bcm_policer_svc_meter_attr_t),"meter mode attr");
    if ( mode_attr == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Failed to allocate memory for svc meter attr \n")));
        return BCM_E_MEMORY;
    }
    sal_memset(mode_attr, 0, sizeof(bcm_policer_svc_meter_attr_t));

    direction = GLOBAL_METER_ALLOC_VERTICAL;

    rv = _bcm_esw_policer_group_set_mode_and_map(unit, mode, npolicers, offset_map,
                                                  &direction, mode_attr);
    if (!(BCM_SUCCESS(rv))) {
        sal_free(mode_attr);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Failed to set mode and map\n")));
        return rv;
    }

    GLOBAL_METER_LOCK(unit);

    if (((direction == GLOBAL_METER_ALLOC_VERTICAL) && (*npolicers > 1)) ||
        ((direction == GLOBAL_METER_ALLOC_HORIZONTAL))) {
        rv = _bcm_esw_policer_svc_meter_create_mode(unit, mode_attr, mode,
                                        -1, *npolicers, &offset_mode);
        if (BCM_FAILURE(rv) && (rv != BCM_E_EXISTS)) {
            GLOBAL_METER_UNLOCK(unit);
            sal_free(mode_attr);
            return rv;
        } else {
             global_meter_offset_mode[unit][offset_mode].no_of_policers =
                                                               *npolicers;
        }
    }
    sal_free(mode_attr);
    rv = _global_meter_policer_id_alloc(unit, direction, npolicers,
                            policer_id, flow_info, &pid_offset[0]);
    if (!(BCM_SUCCESS(rv))) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Failed to allocate policer\n")));
        return rv;
    }
    offset_mask = SOC_INFO(unit).global_meter_max_size_of_pool - 1;

    /* Allocate policer descriptor. */
    _GLOBAL_METER_XGS3_ALLOC(policer_control,
             sizeof (_global_meter_policer_control_t), "Global meter policer");
    if (NULL == policer_control) {
        _bcm_global_meter_free_allocated_policer_on_error(unit, *npolicers,
                             &pid_offset[0], (*policer_id & offset_mask));
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to allocate memeory for policer control \n")));
        return (BCM_E_MEMORY);
    }

    /* Add mode info to the policer index */
    *policer_id |= (offset_mode + 1) << BCM_POLICER_GLOBAL_METER_MODE_SHIFT;

    /* Set policer configuration */
    policer_control->direction = direction;
    policer_control->pid = *policer_id;
    policer_control->grp_mode = mode;
    policer_control->no_of_policers =  *npolicers;

    if (direction == GLOBAL_METER_ALLOC_HORIZONTAL) {
        index = 0;
        do {
            policer_control->offset[index] = pid_offset[index];
            index++;
        } while (index < (*npolicers));

        if (soc_feature(unit, soc_feature_global_meter_mef_10dot3)) {
            rv = _bcm_esw_global_meter_set_cascade_info_mef_10dot3_to_hw(unit,
                               *npolicers, *policer_id, mode, &pid_offset[0]);
        } else {
            rv = _bcm_esw_global_meter_set_cascade_info_to_hw(unit,
                               *npolicers, *policer_id, mode, &pid_offset[0]);
        }
        if (!(BCM_SUCCESS(rv))) {
            /* free all the allocated policers */
            _bcm_global_meter_free_allocated_policer_on_error(unit, *npolicers,
                             &pid_offset[0], (*policer_id & offset_mask));
            /* De-allocate policer descriptor. */
            sal_free(policer_control);
            GLOBAL_METER_UNLOCK(unit);
            return rv;
        }
    }
    if ((mode == bcmPolicerGroupModeCascadeWithCoupling) ||
        (mode == bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
        *npolicers = *npolicers / 2;
    }
    /* increment mode reference count for new policer  */
    if (offset_mode) {
        rv = bcm_policer_svc_meter_inc_mode_reference_count(unit, offset_mode);
        if (!(BCM_SUCCESS(rv))) {
            /* free all the allocated policers */
            _bcm_global_meter_free_allocated_policer_on_error(unit,*npolicers,
                             &pid_offset[0], (*policer_id & offset_mask));
            /* De-allocate policer descriptor. */
            sal_free(policer_control);
            GLOBAL_METER_UNLOCK(unit);
            return rv;
        }
    }


    /* Insert policer into policers hash. */
    _GLOBAL_METER_HASH_INSERT(global_meter_policer_bookkeep[unit],
                              policer_control,
                              (*policer_id & _GLOBAL_METER_HASH_MASK));

    GLOBAL_METER_UNLOCK(unit);
    LOG_DEBUG(BSL_LS_BCM_POLICER,
              (BSL_META_U(unit,
                          "create policer with id %x\n"),
               *policer_id));

    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_envelop_create
 * Purpose:
 *      Create a single policer in Envelop mode. The policer created is
 *      micro or a macro meter depending on the configuration.
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) Indicates micro/macro flow policer
 *     macro_flow_policer_id - (IN) macro flow policer id
 *     policer_id            - (OUT) policer Id
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_envelop_create(int unit, uint32 flags,
                                   bcm_policer_t macro_flow_policer_id,
                                   bcm_policer_t *policer_id)
{
    int rv = BCM_E_NONE;
    int index = 0;
    int micro_flow_index = 0;
    int pool = 0;
    int numbers = 1;
    svm_macroflow_index_table_entry_t macro_flow_entry;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    _bcm_policer_flow_info_t flow_info;
    int bank_size = 0, num_banks_per_pool = 1;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    pool_offset = _shr_popcount(size_pool - 1);
    pool_mask = (num_pools - 1) << pool_offset;
    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    bank_size = size_pool / num_banks_per_pool;

    _bcm_policer_flow_info_t_init(&flow_info);
    if (flags == BCM_POLICER_GLOBAL_METER_ENVELOP_MACRO_FLOW) {
        flow_info.flow_type = bcmPolicerFlowTypeMacro;
        rv = _bcm_esw_policer_group_create(unit, bcmPolicerGroupModeSingle,
                   &flow_info, NULL, policer_id, &numbers);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to create macro flow policer\n")));
            return rv;
        }
    } else if (flags == BCM_POLICER_GLOBAL_METER_ENVELOP_MICRO_FLOW) {
        /* validate macroflow policer id */
        rv = _bcm_esw_policer_validate(unit, &macro_flow_policer_id);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid policer Id passed\n")));
            return rv;
        }
        rv = _bcm_esw_get_policer_table_index(unit, macro_flow_policer_id,
                                                                   &index);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to Get policer index for macro flow "
                                  "policer \n")));
            return rv;
        }
        pool = (macro_flow_policer_id & pool_mask) >> pool_offset;
        flow_info.flow_type = bcmPolicerFlowTypeMicro;
        flow_info.skip_pool = pool;
        flow_info.skip_bank = (macro_flow_policer_id & (size_pool - 1)) / bank_size;
        rv = _bcm_esw_policer_group_create(unit, bcmPolicerGroupModeSingle, &flow_info,
                                                NULL, policer_id, &numbers);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to create micro flow policer\n")));
            return rv;
        }
        rv = _bcm_esw_policer_increment_ref_count(unit, macro_flow_policer_id);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to increment ref count for  micro flow "
                                  "policer\n")));
            return rv;
        }
        /*  get the HW index for the micro flow policer created */
        rv = _bcm_esw_get_policer_table_index(unit, *policer_id,
                                                           &micro_flow_index);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to Get policer index for micro flow "
                                  "policer \n")));
            return rv;
        }
        /* Write the index of the macro-meter at "micro_flow_index" location
           of SVM_MACROFLOW_INDEX_TABLE */
        rv = READ_SVM_MACROFLOW_INDEX_TABLEm(unit, MEM_BLOCK_ANY,
                                         micro_flow_index, &macro_flow_entry);
        if (!(BCM_SUCCESS(rv))) {
            LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Unable to access macro flow table at the index "
                                    "provided\n")));
            return rv;
        }
        if (SOC_MEM_FIELD_VALID(unit, SVM_MACROFLOW_INDEX_TABLEm,
                                                           MACROFLOW_INDEXf)) {
            soc_SVM_MACROFLOW_INDEX_TABLEm_field_set(unit, &macro_flow_entry,
                                           MACROFLOW_INDEXf, (uint32 *)&index);
        }
        rv = WRITE_SVM_MACROFLOW_INDEX_TABLEm(unit, MEM_BLOCK_ANY,
                                          micro_flow_index, &macro_flow_entry);
        if (!(BCM_SUCCESS(rv))) {
            LOG_VERBOSE(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Unable to write to macro flow table at index "
                                    "provided\n")));
            return rv;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid flag passed \n")));
        return BCM_E_PARAM;
    }
    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_group_create
 * Purpose:
 *      Create a group of policers based on the mode.
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) policer group mode
 *     policer_id            - (OUT)Base policer Id
 *     npolicers             - (OUT) Number of policers created
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_group_create(int unit, bcm_policer_group_mode_t mode,
                                   bcm_policer_t *policer_id, int *npolicers)
{
    int rv = BCM_E_NONE;
    int num_pools = 0;
    _bcm_policer_flow_info_t flow_info;
    num_pools =  SOC_INFO(unit).global_meter_pools;

    _bcm_policer_flow_info_t_init(&flow_info);
    flow_info.flow_type = bcmPolicerFlowTypeNormal;
    flow_info.skip_pool = num_pools;
    rv = _bcm_esw_policer_group_create(unit, mode, &flow_info, NULL,
                                            policer_id, npolicers);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to create policer for given mode %d \n"),
                     mode));
        return (rv);
    }
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "Created policer group of %d with base policer %x \n"),
                 *npolicers, *policer_id));
    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_group_create
 * Purpose:
 *      Create a group of policers based on the mode.
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) policer group mode
 *     policer_id            - (OUT)Base policer Id
 *     npolicers             - (OUT) Number of policers created
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_group_create_with_map(int unit,
                                          bcm_policer_group_mode_t mode,
                                          bcm_policer_map_t *offset_map,
                                          bcm_policer_t *policer_id,
                                          int *num_policers)
{
    int rv = BCM_E_NONE;
    int num_pools = 0;
    _bcm_policer_flow_info_t flow_info;
    num_pools =  SOC_INFO(unit).global_meter_pools;

    _bcm_policer_flow_info_t_init(&flow_info);
    flow_info.flow_type = bcmPolicerFlowTypeNormal;
    flow_info.skip_pool = num_pools;
    rv = _bcm_esw_policer_group_create(unit, mode, &flow_info, offset_map,
                                            policer_id, num_policers);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to create policer for given mode %d \n"),
                     mode));
        return (rv);
    }
    LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "Created policer group of %d with base policer %x \n"),
                 *num_policers, *policer_id));
    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_envelop_group_create
 * Purpose:
 *      Create a group of policers based on the mode.
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) Indicates micro/macro flow policer
 *     mode                  - (IN) policer group mode
 *     macro_flow_policer_id - (IN) macro flow policer id
 *     policer_id            - (OUT)Base policer Id
 *     npolicers             - (OUT) Number of policers created
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_envelop_group_create(int unit, uint32 flag,
                                     bcm_policer_group_mode_t mode,
                                     bcm_policer_t macro_flow_policer_id,
                                     bcm_policer_t *policer_id,
                                     int *npolicers)
{
    int rv = BCM_E_NONE;
    int index = 0;
    int micro_flow_index = 0;
    int pool = 0;
    int numbers = 1;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    svm_macroflow_index_table_entry_t   *buf;
    svm_macroflow_index_table_entry_t   *entry;
    int entry_mem_size;    /* Size of table entry. */
    int entry_index = 0;
    int entry_index_max = 0;
    _bcm_policer_flow_info_t flow_info;
    int bank_size, num_banks_per_pool = 1;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    pool_offset = _shr_popcount(size_pool - 1);
    pool_mask = (num_pools - 1) << pool_offset;
    num_banks_per_pool = get_max_banks_in_a_pool(unit);
    bank_size = size_pool / num_banks_per_pool;
    _bcm_policer_flow_info_t_init(&flow_info);

    if (flag == BCM_POLICER_GLOBAL_METER_ENVELOP_MACRO_FLOW) {
        flow_info.flow_type = bcmPolicerFlowTypeMacro;
        flow_info.skip_pool = num_pools;
        rv = _bcm_esw_policer_group_create(unit, bcmPolicerGroupModeSingle,
                    &flow_info,  NULL, policer_id, &numbers);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to create macro flow policer\n")));
            return rv;
        }
    } else if (flag == BCM_POLICER_GLOBAL_METER_ENVELOP_MICRO_FLOW) {

        if ((mode == bcmPolicerGroupModeCascade) ||
            (mode == bcmPolicerGroupModeCascadeWithCoupling) ||
            (mode == bcmPolicerGroupModeIntPriCascade) ||
            (mode == bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
            LOG_ERROR(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to create micro flow policers "
                                  "due to unsupported mode\n")));
            return (BCM_E_PARAM);
        }
        /* validate macroflow policer id */
        rv = _bcm_esw_policer_validate(unit, &macro_flow_policer_id);
        if (!(BCM_SUCCESS(rv))) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Invalid policer Id passed\n")));
            return rv;
        }
        rv = _bcm_esw_get_policer_table_index(unit, macro_flow_policer_id,
                                                                   &index);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to Get policer index for macro flow "
                                  "policer \n")));
            return rv;
        }

        pool = (macro_flow_policer_id & pool_mask) >> pool_offset;

        flow_info.flow_type = bcmPolicerFlowTypeMicro;
        flow_info.skip_pool = pool;
        flow_info.skip_bank = (macro_flow_policer_id & (size_pool - 1)) / bank_size;
        rv = _bcm_esw_policer_group_create(unit, mode, &flow_info,
                                           NULL, policer_id, &numbers);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to create micro flow policers\n")));
            return rv;
        }
        /*  get the HW index for the micro flow policer created */
        rv = _bcm_esw_get_policer_table_index(unit, *policer_id,
                                                           &micro_flow_index);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to Get policer index for micro flow "
                                  "policer \n")));
            return rv;
        }
        /* Write the index of the macro-meter at "micro_flow_index" location
           of SVM_MACROFLOW_INDEX_TABLE */

        entry_mem_size = sizeof(svm_macroflow_index_table_entry_t);
        entry_index_max = micro_flow_index + numbers - 1;
        /* Allocate buffer to store the DMAed table entries. */
        buf = soc_cm_salloc(unit, entry_mem_size * numbers,
                              "svm macro flow index table entry buffer");
        if (NULL == buf) {
            return (BCM_E_MEMORY);
        }
        /* Initialize the entry buffer. */
        sal_memset(buf, 0, sizeof(entry_mem_size) * numbers);

        /* Read the table entries into the buffer. */
        rv = soc_mem_read_range(unit, SVM_MACROFLOW_INDEX_TABLEm,
                                MEM_BLOCK_ALL, micro_flow_index,
                                entry_index_max, buf);
        if (BCM_FAILURE(rv)) {
            if (buf) {
                soc_cm_sfree(unit, buf);
            }
            return rv;
        }

        /* Iterate over the table entries. */
        for (entry_index = 0; entry_index < numbers; entry_index++) {
            entry = soc_mem_table_idx_to_pointer(unit,
                                  SVM_MACROFLOW_INDEX_TABLEm,
                                  svm_macroflow_index_table_entry_t *,
                                  buf, entry_index);
            soc_mem_field_set(unit, SVM_MACROFLOW_INDEX_TABLEm,
                           (void *)entry, MACROFLOW_INDEXf, (uint32 *)&index);
            rv = _bcm_esw_policer_increment_ref_count(unit,
                                                      macro_flow_policer_id);
            if (BCM_FAILURE(rv)) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Unable to increment ref count for micro flow "
                                      "policer\n")));
                if (buf) {
                    soc_cm_sfree(unit, buf);
                }
                return rv;
            }
        }
        if ((rv = soc_mem_write_range(unit, SVM_MACROFLOW_INDEX_TABLEm,
                                      MEM_BLOCK_ALL, micro_flow_index,
                                      entry_index_max, buf)) < 0) {
            if (BCM_FAILURE(rv)) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER,
                            (BSL_META_U(unit,
                                        "Unable to write to macro flow table at index "
                                        "provided\n")));
                if (buf) {
                    soc_cm_sfree(unit, buf);
                }
                return rv;
            }
        }
        if (buf) {
            soc_cm_sfree(unit, buf);
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid flag passed \n")));
        return BCM_E_PARAM;
    }
    *npolicers = numbers;
    return rv;
}



int
_bcm_esw_policer_udf_group_set_offset_map(
   int unit,
   bcm_policer_group_mode_type_t type,
   uint32 npolicers,
   uint32 num_selectors,
   bcm_policer_group_mode_attr_selector_t *attr_selectors,
   bcm_policer_svc_meter_attr_t *mode_attr)
{
    int rv = BCM_E_NONE;
    int num_pools = 0;
    offset_table_entry_t   *offset_map = NULL;
    int i = 0;
    uint32 value[256] = { 0 };
    uint32 table_offset[256] = { 0 };
    num_pools =  SOC_INFO(unit).global_meter_pools;

    offset_map =  &mode_attr->udf_pkt_attr_selectors_v.offset_map[0];

    switch (type) {
        case bcmPolicerGroupModeTypeNormal:
            for (i = 0; i < num_selectors; i++) {
               offset_map[attr_selectors[i].attr_value].offset
                                       = attr_selectors[i].policer_offset;
               offset_map[attr_selectors[i].attr_value].meter_enable = 1;
            }
            break;

        case bcmPolicerGroupModeTypeCascade:
            for (i = 0; i < num_selectors; i++) {
                if (attr_selectors[i].policer_offset >= num_pools) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "Invalid policer offset \n")));
                    return BCM_E_PARAM;
                }
                value[i] = attr_selectors[i].policer_offset;
                table_offset[i] = attr_selectors[i].attr_value;
            }
            rv = _bcm_esw_policer_set_offset_table_map_at_offsets_with_flex_pool(
                                             0, num_selectors, &table_offset[0],
                                             &value[0], offset_map);
            BCM_IF_ERROR_RETURN(rv);
            break;

        case bcmPolicerGroupModeTypeCascadeWithCoupling:
            for (i = 0; i < num_selectors; i++) {
                if (attr_selectors[i].policer_offset >= num_pools/2) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Invalid policer offset \n")));
                    return BCM_E_PARAM;
                }
                value[i] = attr_selectors[i].policer_offset;
                table_offset[i] = attr_selectors[i].attr_value;
            }
            rv = _bcm_esw_policer_set_offset_table_map_at_offsets_with_flex_pool(
                                             1, num_selectors, &table_offset[0],
                                             &value[0], offset_map);
            BCM_IF_ERROR_RETURN(rv);
            break;

        default :
            rv =  BCM_E_PARAM;

    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_svc_meter_offset_map_update
 * Purpose:
 *      Update offset map based on attributes
 * Parameters:
 *     unit                  - (IN) unit number
 *     attr                  - (IN/OUT) Offset mode attributes
 *     offset                - (IN) Index in Offset Map
 *     policer_offset        - (IN) value to be updated
 * Returns:
 *     NONE
 */
STATIC void
 _bcm_esw_svc_meter_offset_map_update(
     uint32 unit,
     bcm_policer_svc_meter_attr_t *attr,
     bcm_policer_group_mode_t group_mode,
     uint32 offset,
     uint32 policer_offset)
{
    uint8 pool_offset_limit = 0;
    uint8 dir = 0;
    uint8 coupling_offset = 0;
    offset_table_entry_t *offset_map_ptr = NULL;

    switch (attr->mode_type_v) {
        case udf_mode:
        case udf_cascade_mode:
        case udf_cascade_with_coupling_mode:
            offset_map_ptr =
                &(attr->udf_pkt_attr_selectors_v.offset_map[0]);
            break;
        case compressed_mode:
            offset_map_ptr =
                &(attr->compressed_attr_selectors_v.offset_map[0]);
            break;
        case uncompressed_mode:
            offset_map_ptr =
                &(attr->uncompressed_attr_selectors_v.offset_map[0]);
            break;
        default :
            break;
    }
    if (offset_map_ptr != NULL) {
        if (group_mode == bcmPolicerGroupModeSingle) {
            offset_map_ptr[offset].offset = policer_offset;
            offset_map_ptr[offset].pool = 0;
            offset_map_ptr[offset].meter_enable = 1;
        } else {
            if (soc_feature(0, soc_feature_global_meter_mef_10dot3)) {
                coupling_offset = 0;
            } else {
                coupling_offset =
                    (group_mode == bcmPolicerGroupModeCascade) ? 0 : 4;
            }
            if (soc_feature(unit,
                        soc_feature_global_meter_pool_priority_descending)) {
                pool_offset_limit = (group_mode == bcmPolicerGroupModeCascade) ?
                    (BCM_POLICER_GLOBAL_METER_MAX_POOL - 1) :
                    (BCM_POLICER_GLOBAL_METER_MAX_POOL/2 - 1);
                dir = -1;
            } else {
                pool_offset_limit = 0;
                dir = 1;
            }
            offset_map_ptr[offset].offset = 0;
            offset_map_ptr[offset].pool =
                pool_offset_limit + dir * policer_offset + coupling_offset;
            offset_map_ptr[offset].meter_enable = 1;
        }
    }
}

/*
 * Function:
 *      _bcm_esw_svc_meter_offset_mode_id_check
 * Purpose:
 *      Check if given attribute selectors list is already
 *      configured or not
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) type of group mode
 *     total_policers        - (IN) total policers
 *     num_selectors         - (IN) number of selectors
 *     attr_selectors        - (IN) list of attributes selectors
 *     mode_id               - (OUT) pointer to offset mode index
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_svc_meter_offset_mode_id_check(
    uint32 unit,
    uint32 flags,
    bcm_policer_group_mode_type_t type,
    uint32 total_policers,
    uint32 num_selectors,
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    uint32 *mode_id)
{
    uint32 index = 0;
    int rv;
    int i, is_same;
    bcm_policer_svc_meter_bookkeep_mode_t  mode_info;

    /* Loop through all offset mode index and find match */
    for (index = 1; index < BCM_POLICER_SVC_METER_MAX_MODE; index++) {
        if (_bcm_policer_svc_meter_get_mode_info(unit, index, &mode_info)
                                                         == BCM_E_NONE) {
            if (mode_info.type != type) {
                continue;
            }

            /* check num selectors */
            if (mode_info.no_of_selectors == num_selectors) {
                if (mode_info.attr_selectors != NULL) {
                    for (i = 0; i < num_selectors; i++) {
                        rv = _bcm_attr_selectors_wb_attr_selectors_cmp(
                                &attr_selectors[i],
                                &mode_info.attr_selectors[i],
                                &is_same);
                        BCM_IF_ERROR_RETURN(rv);
                        if (is_same == FALSE) {
                            break;
                        }
                    }
                    if (i == num_selectors) {
                        /* all the selectors are the same */
                        *mode_id = index;
                        return BCM_E_EXISTS;
                    }
                }
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_svc_meter_group_offset_mode_info_update
 * Purpose:
 *      update offset mode info
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) offset mode index
 *     num_selectors         - (IN) number of selectors
 *     attr_selectors        - (IN) list of attributes selectors
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_policer_svc_meter_group_offset_mode_info_update (
            int unit,
            bcm_policer_svc_meter_mode_t mode,
            int8 type,
            int num_selectors,
            bcm_policer_group_mode_attr_selector_t *attr_selectors)
{
    int rv;
    int i;
    if ((num_selectors == 0) || (attr_selectors == NULL)) {
        /* Free if allocated */
        if (global_meter_offset_mode[unit][mode].attr_selectors !=
                NULL) {
            sal_free(global_meter_offset_mode[unit][mode].attr_selectors);
        }
        global_meter_offset_mode[unit][mode].attr_selectors = NULL;
    } else {
        global_meter_offset_mode[unit][mode].type = type;
        global_meter_offset_mode[unit][mode].no_of_selectors = num_selectors;

        global_meter_offset_mode[unit][mode].attr_selectors =
            sal_alloc(num_selectors *
                    sizeof(bcm_policer_group_mode_attr_selector_t),
                    "attr selectors");
        if (NULL == global_meter_offset_mode[unit][mode].attr_selectors) {
            return BCM_E_MEMORY;
        }
        for (i = 0; i < num_selectors; i++) {
            rv = _bcm_attr_selectors_copy_to_wb_attr_selectors(
                    &attr_selectors[i],
                    &global_meter_offset_mode[unit][mode].attr_selectors[i]);
            BCM_IF_ERROR_RETURN(rv);
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_policer_group_udf_mode_fillup_values
 * Purpose:
 *      Fill up temporary structures in appropriate order for efficient
 *      management of user input
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) type of group mode
 *     total_policers        - (IN) total policers
 *     num_selectors         - (IN) number of selectors
 *     attr_selectors        - (IN) list of attributes selectors
 *     udf_attr_selectors    - (OUT) fill up this structure
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_policer_group_udf_mode_fillup_values (
            int unit,
            uint32 flags,
            bcm_policer_group_mode_type_t type,
            uint32 total_policers,
            uint32 num_selectors,
            bcm_policer_group_mode_attr_selector_t *attr_selectors,
            bcm_policer_udf_attr_selectors_t *udf_attr_selectors)
{
    int rv = BCM_E_NONE;
    uint32 sel = 0;
    uint32 index = 0;
    uint32 drop =0;
    int udf_id = 0;
    uint32 offset = 0, width = 0, attr_value = 0, policer_offset = 0;
    uint32 is_subdiv_new = 0, subdiv_first_bit = 0, subdiv_last_bit = 0;
    uint32 selectors = 0, udf_subdiv_present = 0;
    bcm_policer_group_mode_attr_t attr;
    bcm_policer_udf_pkt_combine_attr_t *udf_combine_attr_data = NULL;
#if defined(BCM_KATANA2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
    bcmi_xgs4_udf_offset_info_t *udf_offset_node = NULL;
    int grp_udf_id = -1;
#endif

    for (sel = 0; sel < num_selectors; sel++) {
        policer_offset = attr_selectors[sel].policer_offset;
        attr = attr_selectors[sel].attr;
        attr_value = attr_selectors[sel].attr_value;
        udf_id = attr_selectors[sel].udf_id;
        offset = attr_selectors[sel].offset;
        width = attr_selectors[sel].width;

        if (policer_offset >= total_policers) {
            return BCM_E_PARAM;
        }

        switch(attr) {
            case bcmPolicerGroupModeAttrUdf :
            {
                if (!(attr_selectors[sel].flags &
                                BCM_POLICER_ATTR_WIDTH_OFFSET)) {
                    return (BCM_E_PARAM);
                }
#if defined(BCM_KATANA2_SUPPORT) || defined(BCM_APACHE_SUPPORT)
                /* Validate UDF Id passed */
                if (grp_udf_id == -1) {
                    rv = bcmi_xgs4_udf_offset_node_get(unit,
                                attr_selectors[sel].udf_id, &udf_offset_node);
                    if (BCM_FAILURE(rv)) {
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "Failed to get info related to UDF Id \n")));
                        return rv;
                    }

                    /* Matching node not found */
                    if (udf_offset_node == NULL) {
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "Failed to get info related to UDF Id \n")));
                        return BCM_E_NOT_FOUND;
                    }

                    if (!((udf_offset_node->hw_bmap & 0x1) ||
                                    (udf_offset_node->hw_bmap & 0x2))) {
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                            "UDF is not properly configured for policer use\n")));
                        return BCM_E_PARAM;
                    }
                    grp_udf_id = udf_id;
                } else if (udf_id != grp_udf_id) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                        "Mismatch of UDFs in two attribute selectors\n")));
                    return BCM_E_PARAM;
                }
#endif
                /* width = 0 means ING_SVM_PKT_ATTR_SELECTOR_KEY_Xr is not
                 * updated with required values, makes no sense */
                if ((width > 8) || (width <= 0)) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                        "Invalid width passed for UDF \n")));
                    return BCM_E_PARAM;
                }
                /* offset value range from 0 to 31 */
                if (offset >= SVC_METER_UDF1_MAX_BIT_POSITION) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                        "Invalid offset passed for UDF \n")));
                    return (BCM_E_PARAM);
                }
                /* offset+width should not exceed 32 - size of (UDF0 + UDF1)
                 * For eg. if offset=31, width can be 1 (selecting msb
                 * of udf1) making sum 32 */
                if ((offset + width) > SVC_METER_UDF1_MAX_BIT_POSITION) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                        "Invalid offset and width pair passed for UDF \n")));
                    return (BCM_E_PARAM);
                }
                /* Verify that given offset-width does not overlap
                 * 0-not used, 1-start offset in udf, 2-rest bits upto width */
                if (udf_attr_selectors->udf_bits[offset] == 0) {
                    is_subdiv_new = 1;
                }
                subdiv_first_bit = offset;
                subdiv_last_bit = offset + width - 1;
                if (subdiv_last_bit >= SVC_METER_UDF1_MAX_BIT_POSITION) {
                    subdiv_last_bit = SVC_METER_UDF1_MAX_BIT_POSITION - 1;
                }
                for (index = subdiv_first_bit; index <= subdiv_last_bit; index++) {
                    if (index == subdiv_first_bit) {
                        if (is_subdiv_new == 1) {
                            udf_attr_selectors->udf_bits[index] = 1;
                        } else if (udf_attr_selectors->udf_bits[index] != 1) {
                            rv = BCM_E_PARAM;
                            break;
                        }
                    } else {
                        if (((is_subdiv_new == 1) &&
                                (udf_attr_selectors->udf_bits[index] != 0)) ||
                                ((is_subdiv_new == 0) &&
                                (udf_attr_selectors->udf_bits[index] != 2))) {
                            rv = BCM_E_PARAM;
                            break;
                        } else {
                            udf_attr_selectors->udf_bits[index] = 2;
                        }
                    }
                }
                if ((rv == BCM_E_NONE) &&
                    (index != SVC_METER_UDF1_MAX_BIT_POSITION) &&
                    (udf_attr_selectors->udf_bits[index] == 2)) {
                    rv = BCM_E_PARAM;
                }
                if (BCM_FAILURE(rv)) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                        "Overlapped offset/width pair for UDF not allowed\n")));
                    return rv;
                }
                /* Attribute value should not exceed size as given in width */
                if (attr_value >= (1 << width)) {
                    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                        "Attribute value passed exceeds range of 'width' bits\n")));
                    return BCM_E_PARAM;
                }

                if (is_subdiv_new == 1) {
                    index = udf_attr_selectors->total_subdiv;
                    udf_attr_selectors->udf_subdiv[index].udf_id = udf_id;
                    udf_attr_selectors->udf_subdiv[index].offset = offset;
                    udf_attr_selectors->udf_subdiv[index].width = width;
                    udf_attr_selectors->total_subdiv = index + 1;
                    is_subdiv_new = 0;
                }
                udf_combine_attr_data =
                    &(udf_attr_selectors->combine_attr_data[policer_offset]);
                selectors = udf_combine_attr_data->selectors;
                udf_combine_attr_data->udf_subdiv[selectors].udf_id = udf_id;
                udf_combine_attr_data->udf_subdiv[selectors].offset = offset;
                udf_combine_attr_data->udf_subdiv[selectors].width = width;
                udf_combine_attr_data->attr_value[selectors] = attr_value;
                udf_combine_attr_data->selectors = selectors + 1;
                break;
            }
            case bcmPolicerGroupModeAttrDrop :
            {
                udf_attr_selectors->drop = 1;
                udf_combine_attr_data =
                    &(udf_attr_selectors->combine_attr_data[policer_offset]);
                drop = attr_selectors[sel].attr_value;
                switch(drop) {
                    case 0:
                        udf_combine_attr_data->drop_flags =
                                            _BCM_POLICER_DROP_DISABLE;
                        break;
                    case 1:
                        udf_combine_attr_data->drop_flags =
                                            _BCM_POLICER_DROP_ENABLE;
                        break;
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        udf_combine_attr_data->drop_flags =
                                            _BCM_POLICER_DROP_DISABLE |
                                            _BCM_POLICER_DROP_ENABLE;
                        break;
                    default :
                        return BCM_E_PARAM;
                }
                break;
            }
            default :
                return BCM_E_PARAM;
        }
    }

    /* All policer offset must have atleast 1 selector for each sub-division*/
    for (policer_offset = 0; policer_offset < total_policers; policer_offset++) {
        udf_combine_attr_data = &(udf_attr_selectors->combine_attr_data[policer_offset]);
        if (udf_attr_selectors->drop) {
            if (udf_combine_attr_data->drop_flags == 0) {
                return BCM_E_PARAM;
            }
        }
        for (index = 0; index < udf_attr_selectors->total_subdiv; index++) {
            udf_subdiv_present = 0;
            for (selectors = 0; udf_combine_attr_data->selectors; selectors++) {
                if (udf_attr_selectors->udf_subdiv[index].offset == \
                        udf_combine_attr_data->udf_subdiv[selectors].offset) {
                    udf_subdiv_present = 1;
                    break;
                }
            }
            if (udf_subdiv_present == 0) {
                return BCM_E_PARAM;
            }
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_policer_group_mode_fillup_values
 * Purpose:
 *      Fill up temporary structures in appropriate order for efficient
 *      management of user input
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) type of group mode
 *     total_policers        - (IN) total policers
 *     num_selectors         - (IN) number of selectors
 *     attr_selectors        - (IN) list of attributes selectors
 *     pkt_attr_selectors    - (OUT) fill up this structure
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_policer_group_mode_fillup_values (
            int unit,
            uint32 flags,
            bcm_policer_group_mode_type_t type,
            uint32 total_policers,
            uint32 num_selectors,
            bcm_policer_group_mode_attr_selector_t *attr_selectors,
            bcm_policer_attr_selectors_t *pkt_attr_selectors)
{
    uint32 sel = 0, pol_offset = 0;
    uint32 attr_value = 0;
    uint32 value = 0, max_value = 0;
    uint32 attr_size = 0;
    uint32 index = 0;
    uint32 prio_range_first = 0, prio_range_last = 0;
    bcm_policer_group_mode_attr_t attr;
    bcm_policer_combine_attr_t *combine_attr_data = NULL;

    for (sel = 0; sel < num_selectors; sel++) {
        pol_offset = attr_selectors[sel].policer_offset;
        attr = attr_selectors[sel].attr;
        attr_value = attr_selectors[sel].attr_value;
        if (pol_offset >= total_policers) {
            LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                            "Bad Param : sel=%d : offset (%d) more than"
                            " total_policer=%d \n"),
                        sel, pol_offset, total_policers));
            return BCM_E_PARAM;
        }
        combine_attr_data =
            &pkt_attr_selectors->combine_attr_data[pol_offset];
        switch(attr) {
            case bcmPolicerGroupModeAttrFieldIngressColor :
                pkt_attr_selectors->cng = 1;
                switch(attr_value) {
                    case bcmColorGreen :
                        combine_attr_data->cng_flags |=
                            _BCM_POLICER_COLOR_GREEN;
                        break;
                    case bcmColorYellow :
                        combine_attr_data->cng_flags |=
                            _BCM_POLICER_COLOR_YELLOW;
                        break;
                    case bcmColorRed :
                        combine_attr_data->cng_flags |=
                            _BCM_POLICER_COLOR_RED;
                        break;
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        combine_attr_data->cng_flags |=
                            (_BCM_POLICER_COLOR_GREEN |
                             _BCM_POLICER_COLOR_YELLOW |
                             _BCM_POLICER_COLOR_RED);
                        break;
                    default :
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(cng) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                pkt_attr_selectors->combine_cng_flags |=
                        combine_attr_data->cng_flags;
                break;

            case bcmPolicerGroupModeAttrIntPri :
                pkt_attr_selectors->int_pri = 1;
                if (attr_value == BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES) {
                    prio_range_first = _BCM_POLICER_PKT_PRI0;
                    prio_range_last = _BCM_POLICER_PKT_PRI15;
                } else if (attr_value <= _BCM_POLICER_PKT_PRI15) {
                    prio_range_first = prio_range_last = attr_value;
                } else {
                       LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(int_pri) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                for (index = prio_range_first; index <= prio_range_last; index++) {
                    combine_attr_data->int_pri_flags |= (1 << index);
                }
                pkt_attr_selectors->combine_int_pri_flags |=
                        combine_attr_data->int_pri_flags;
                break;

            case bcmPolicerGroupModeAttrVlan:
                pkt_attr_selectors->vlan_format = 1;
                switch(attr_value) {
                    case bcmPolicerGroupModeAttrVlanUnTagged :
                        combine_attr_data->vlan_format_flags |=
                            _BCM_POLICER_VLAN_FORMAT_UNTAGGED;
                        break;
                    case bcmPolicerGroupModeAttrVlanInnerTag :
                        combine_attr_data->vlan_format_flags |=
                            _BCM_POLICER_VLAN_FORMAT_INNER;
                        break;
                    case bcmPolicerGroupModeAttrVlanOuterTag :
                        combine_attr_data->vlan_format_flags |=
                            _BCM_POLICER_VLAN_FORMAT_OUTER;
                        break;
                    case bcmPolicerGroupModeAttrVlanStackedTag :
                        combine_attr_data->vlan_format_flags |=
                            _BCM_POLICER_VLAN_FORMAT_BOTH;
                        break;
                    case bcmPolicerGroupModeAttrVlanAll :
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        combine_attr_data->vlan_format_flags |=
                            (_BCM_POLICER_VLAN_FORMAT_UNTAGGED |
                             _BCM_POLICER_VLAN_FORMAT_INNER |
                             _BCM_POLICER_VLAN_FORMAT_OUTER |
                             _BCM_POLICER_VLAN_FORMAT_BOTH);
                        break;
                    default :
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(vlan_format) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                pkt_attr_selectors->combine_vlan_format_flags |=
                        combine_attr_data->vlan_format_flags;
                break;

            case bcmPolicerGroupModeAttrOuterPri:
                pkt_attr_selectors->outer_dot1p = 1;
                if (attr_value == BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES) {
                    prio_range_first = _BCM_POLICER_PKT_PRI0;
                    prio_range_last = _BCM_POLICER_PKT_PRI7;
                } else if (attr_value <= _BCM_POLICER_PKT_PRI7) {
                    prio_range_first = prio_range_last = attr_value;
                } else {
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(outer_dot1p) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                for (index = prio_range_first; index <= prio_range_last; index++) {
                    combine_attr_data->outer_dot1p_flags |= (1 << index);
                }
                pkt_attr_selectors->combine_outer_dot1p_flags |=
                        combine_attr_data->outer_dot1p_flags;
                break;

            case bcmPolicerGroupModeAttrInnerPri :
                pkt_attr_selectors->inner_dot1p = 1;
                if (attr_value == BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES) {
                    prio_range_first = _BCM_POLICER_PKT_PRI0;
                    prio_range_last = _BCM_POLICER_PKT_PRI7;
                } else if (attr_value <= _BCM_POLICER_PKT_PRI7) {
                    prio_range_first = prio_range_last = attr_value;
                } else {
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(inner_dot1p) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                for (index = prio_range_first; index <= prio_range_last; index++) {
                    combine_attr_data->inner_dot1p_flags |= (1 << index);
                }
                pkt_attr_selectors->combine_inner_dot1p_flags |=
                        combine_attr_data->inner_dot1p_flags;
                break;

            case bcmPolicerGroupModeAttrPort :
                pkt_attr_selectors->port = 1;
                max_value =  soc_mem_index_max(unit, ING_SVM_PORT_MAPm);
                switch (attr_value) {
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        for (value = 0; value <= max_value; value++) {
                            _BCM_POLICER_VALUE_SET(combine_attr_data->
                                    value_array[_bcmPolicerAttrPort], value);
                            _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                    combine_value_array[_bcmPolicerAttrPort], value);
                        }
                        break;
                    default :
                        if (attr_value > max_value) {
                            LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                            "BAD PARAM(port) : sel=%d "
                                            "offset=%d, attr=%d value=%d \n"),
                                        sel, pol_offset, attr, attr_value));
                            return BCM_E_PARAM;
                        }
                        _BCM_POLICER_VALUE_SET (combine_attr_data->
                                value_array[_bcmPolicerAttrPort], attr_value);
                        _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                combine_value_array[_bcmPolicerAttrPort], attr_value);
                }
                break;
            case bcmPolicerGroupModeAttrTosDscp :
                pkt_attr_selectors->tos_dscp = 1;
                attr_size = BCM_POLICER_SVC_METER_TOS_ATTR_SIZE;
                max_value = (1 << attr_size) - 1;
                switch (attr_value) {
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        for (value = 0; value <= max_value; value++) {
                            _BCM_POLICER_VALUE_SET (combine_attr_data->
                                    value_array[_bcmPolicerAttrTosDscp], value);
                            _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                    combine_value_array[_bcmPolicerAttrTosDscp], value);
                        }
                        break;
                    default :
                        if (attr_value > max_value) {
                            LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                            "BAD PARAM(tos_dscp) : sel=%d "
                                            "offset=%d, attr=%d value=%d \n"),
                                        sel, pol_offset, attr, attr_value));
                            return BCM_E_PARAM;
                        }
                        _BCM_POLICER_VALUE_SET (combine_attr_data->
                                value_array[_bcmPolicerAttrTosDscp], attr_value);
                        _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                combine_value_array[_bcmPolicerAttrTosDscp], attr_value);
                }
                break;
            case bcmPolicerGroupModeAttrTosEcn :
                pkt_attr_selectors->tos_ecn = 1;
                attr_size = BCM_POLICER_SVC_METER_TOS_ECN_ATTR_SIZE;
                max_value = (1 << attr_size) - 1;
                switch (attr_value) {
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        for (value = 0; value <= max_value; value++) {
                            _BCM_POLICER_VALUE_SET (combine_attr_data->
                                    value_array[_bcmPolicerAttrTosEcn], value);
                            _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                    combine_value_array[_bcmPolicerAttrTosEcn], value);
                        }
                        break;
                    default :
                        if (attr_value > max_value) {
                            LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                            "BAD PARAM(tos_ecn) : sel=%d "
                                            "offset=%d, attr=%d value=%d \n"),
                                        sel, pol_offset, attr, attr_value));
                            return BCM_E_PARAM;
                        }
                        _BCM_POLICER_VALUE_SET(combine_attr_data->
                                value_array[_bcmPolicerAttrTosEcn], attr_value);
                        _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                combine_value_array[_bcmPolicerAttrTosEcn], attr_value);
                }
                break;
            case bcmPolicerGroupModeAttrPktType :
                pkt_attr_selectors->pkt_resolution = 1;
                switch(attr_value) {
                    case bcmPolicerGroupModeAttrPktTypeUnknown :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeControl :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_CONTROL_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeOAM :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_OAM_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeBFD :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_BFD_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeBPDU :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_BPDU_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeICNM :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_ICNM_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktType1588 :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_PKT_IS_1588;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownL2UC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_L2UC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownL2UC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_L2UC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeL2BC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_L2BC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownL2MC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_L2MC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownL2MC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_L2MC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownL3UC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_L3UC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownL3UC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_L3UC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownIPMC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_IPMC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownIPMC :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_IPMC_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownMplsL2 :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L2_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownMplsL3 :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L3_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownMpls :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownMpls :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_MPLS_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownMplsMulticast :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_MULTICAST_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownMim :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_MIM_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownMim :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_MIM_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownTrill :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_TRILL_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownTrill :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_TRILL_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeKnownNiv :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_KNOWN_NIV_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeUnknownNiv :
                        combine_attr_data->pkt_resolution_flags |=
                            _BCM_POLICER_PKT_TYPE_UNKNOWN_NIV_PKT;
                        break;
                    case bcmPolicerGroupModeAttrPktTypeAll :
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        combine_attr_data->pkt_resolution_flags |=
                            (_BCM_POLICER_PKT_TYPE_UNKNOWN_PKT |
                             _BCM_POLICER_PKT_TYPE_CONTROL_PKT |
                             _BCM_POLICER_PKT_TYPE_OAM_PKT |
                             _BCM_POLICER_PKT_TYPE_BFD_PKT |
                             _BCM_POLICER_PKT_TYPE_BPDU_PKT |
                             _BCM_POLICER_PKT_TYPE_ICNM_PKT |
                             _BCM_POLICER_PKT_TYPE_PKT_IS_1588 |
                             _BCM_POLICER_PKT_TYPE_KNOWN_L2UC_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_L2UC_PKT |
                             _BCM_POLICER_PKT_TYPE_L2BC_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_L2MC_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_L2MC_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_L3UC_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_L3UC_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_IPMC_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_IPMC_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L2_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L3_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_MPLS_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_MULTICAST_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_MIM_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_MIM_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_TRILL_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_TRILL_PKT |
                             _BCM_POLICER_PKT_TYPE_KNOWN_NIV_PKT |
                             _BCM_POLICER_PKT_TYPE_UNKNOWN_NIV_PKT);
                        break;
                    default :
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(port) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                pkt_attr_selectors->combine_pkt_res_flags |=
                        combine_attr_data->pkt_resolution_flags;
                break;

            case bcmPolicerGroupModeAttrIngNetworkGroup :
                pkt_attr_selectors->svp_type = 1;
                attr_size = BCM_SVM_DEV_ATTR(unit)->uncompressed_svp_type_size;
                max_value = (1 << attr_size) - 1;
                switch (attr_value) {
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        for (value = 0; value <= max_value; value++) {
                            _BCM_POLICER_VALUE_SET (combine_attr_data->
                                    value_array[_bcmPolicerAttrSvpType], value);
                            _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                    combine_value_array[_bcmPolicerAttrSvpType], value);
                        }
                        break;
                    default :
                        if (attr_value > max_value) {
                            LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                            "BAD PARAM(svm_group) : sel=%d "
                                            "offset=%d, attr=%d value=%d \n"),
                                        sel, pol_offset, attr, attr_value));
                            return BCM_E_PARAM;
                        }
                        _BCM_POLICER_VALUE_SET (combine_attr_data->
                                value_array[_bcmPolicerAttrSvpType], attr_value);
                        _BCM_POLICER_VALUE_SET(pkt_attr_selectors->
                                combine_value_array[_bcmPolicerAttrSvpType], attr_value);
                }
                break;

            case bcmPolicerGroupModeAttrDrop :
                pkt_attr_selectors->drop = 1;
                switch(attr_value) {
                    case 0 :
                        combine_attr_data->drop_flags |=
                            _BCM_POLICER_DROP_DISABLE;
                        break;
                    case 1 :
                        combine_attr_data->drop_flags |=
                            _BCM_POLICER_DROP_ENABLE;
                        break;
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        combine_attr_data->drop_flags |=
                            (_BCM_POLICER_DROP_DISABLE |
                             _BCM_POLICER_DROP_ENABLE);
                        break;
                    default :
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(drop) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                pkt_attr_selectors->combine_drop_flags |=
                        combine_attr_data->drop_flags;
                break;

            case bcmPolicerGroupModeAttrPacketTypeIp :
                pkt_attr_selectors->ip_pkt = 1;
                switch(attr_value) {
                    case 0 :
                        combine_attr_data->ip_pkt_flags |=
                            _BCM_POLICER_IP_PKT_DISABLE;
                        break;
                    case 1 :
                        combine_attr_data->ip_pkt_flags |=
                            _BCM_POLICER_IP_PKT_ENABLE;
                        break;
                    case BCM_POLICER_GROUP_MODE_ATTR_ALL_VALUES :
                        combine_attr_data->ip_pkt_flags |=
                            (_BCM_POLICER_IP_PKT_DISABLE |
                             _BCM_POLICER_IP_PKT_ENABLE);
                        break;
                    default :
                        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                        "BAD PARAM(ip_pkt) : sel=%d "
                                        "offset=%d, attr=%d value=%d \n"),
                                    sel, pol_offset, attr, attr_value));
                        return BCM_E_PARAM;
                }
                pkt_attr_selectors->combine_ip_pkt_flags |=
                        combine_attr_data->ip_pkt_flags;
                break;

            default :
                return BCM_E_PARAM;
        }
        combine_attr_data = NULL;
    }

    /* Check that all policer offset have all configured attributes */
    for (pol_offset = 0; pol_offset < total_policers; pol_offset++) {
        if (pkt_attr_selectors->cng) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    cng_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "color : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->int_pri) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    int_pri_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "int_pri : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->vlan_format) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    vlan_format_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "vlan_format : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->outer_dot1p) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    outer_dot1p_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "outer_dot1p : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->inner_dot1p) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    inner_dot1p_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "inner_dot1p : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->port) {
            for (index = 0; index < _BCM_POLICER_BIT_ARRAY_SIZE; index++) {
                if (pkt_attr_selectors->combine_attr_data[pol_offset].
                        value_array[_bcmPolicerAttrPort][index] != 0) {
                    break;
                }
            }
            if (index == _BCM_POLICER_BIT_ARRAY_SIZE) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "ing_port : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->tos_dscp) {
            for (index = 0; index < _BCM_POLICER_BIT_ARRAY_SIZE; index++) {
                if (pkt_attr_selectors->combine_attr_data[pol_offset].
                        value_array[_bcmPolicerAttrTosDscp][index] != 0) {
                    break;
                }
            }
            if (index == _BCM_POLICER_BIT_ARRAY_SIZE) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "tos_dscp : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->tos_ecn) {
            for (index = 0; index < _BCM_POLICER_BIT_ARRAY_SIZE; index++) {
                if (pkt_attr_selectors->combine_attr_data[pol_offset].
                        value_array[_bcmPolicerAttrTosEcn][index] != 0) {
                    break;
                }
            }
            if (index == _BCM_POLICER_BIT_ARRAY_SIZE) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "tos_ecn : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->pkt_resolution) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    pkt_resolution_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "pkt_res : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->svp_type) {
            for (index = 0; index < _BCM_POLICER_BIT_ARRAY_SIZE; index++) {
                if (pkt_attr_selectors->combine_attr_data[pol_offset].
                        value_array[_bcmPolicerAttrSvpType][index] != 0) {
                    break;
                }
            }
            if (index == _BCM_POLICER_BIT_ARRAY_SIZE) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "svp_type : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->drop) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    drop_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "drop : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
        if (pkt_attr_selectors->ip_pkt) {
            if (pkt_attr_selectors->combine_attr_data[pol_offset].
                    ip_pkt_flags == 0) {
                LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                                "ip_pkt : combination issue... Check"
                                "params and assign dummy value\n")));
                return BCM_E_PARAM;
            }
        }
    }
    pkt_attr_selectors->total_policers = total_policers;
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_group_mode_offset_map_generate
 * Purpose:
 *      This internal function is used to calculate final offset locations
 *      in offset_map where policer_offset needs to be put
 * Parameters:
 *     unit                  - (IN) unit number
 *     meter_attr            - (IN/OUT) Meter attributes
 *     group_mode            - (IN) group mode of policer group
 *     policer_policers      - (IN) policer offset
 *     offset_array          - (IN) 2D array which stores shifted attr_values
 * Returns:
 *     NONE
 */
STATIC
void _bcm_policer_group_mode_offset_map_generate(
    int unit,
    bcm_policer_svc_meter_attr_t *meter_attr,
    bcm_policer_group_mode_t group_mode,
    int pol_offset,
    uint8 **offset_array)
{
    uint32 offset = 0, final_offset = 0;
    uint32 count = 0, row = 0, index = 0;
    uint32 stack_offset[SVC_METER_MAX_SELECTOR_BITS] = {0};
    uint32 stack_index[SVC_METER_MAX_SELECTOR_BITS] = {0};
    uint32 top = 0;

    /* To generate final offset index at which policer_offset value
     * needs to be written in SVM_OFFSET_TABLE, attr_value belonging to
     * different sub-groups have AND relationship.
     * Following code block generate final offset using OR operator as
     * the caller function already left-shifted values by required number
     * of bits.
     * At offset_array[X][0], count of (left-shifted) attr_values
     * is stored. Actual attr_value is stored from index 1.
     * 'top' is index in stack at which next data is inserted.
     * 'row' denotes moving row-wise in offset_array[][]. Each row in
     * offset_array[][] denotes different sub-group for attr_value
     * stack_offset[row] stores ORed offset computed at till [row-1] (from 0)
     * stack_index[row] stores the index in offset_array[X] upto
     * which, we have used is used to calculate offset in this row.
     */
    /* Example : For offset_array
     * offset_array[0] = {2,   0, 1}
     * offset_array[1] = {3,   2, 3, 4}
     * offset_array[2] = {1,   5}
     * offset_array[3] = {2,   6, 7}
     * offset_array[4] = {0}
     * Final_offsets calculated
     *               =  0|2|5|6, 0|2|5|7, 0|3|5|6, 0|3|5|7,
     *                  0|4|5|6, 0|4|5|7, 1|2|5|6, 1|2|5|7,
     *                  1|3|5|6, 1|3|5|7, 1|4|5|6, 1|4|5|7,
     */
    /* Insert default value in stack, both stack_offset[] and stack_index[]
     * is initialized with 0 as none of the index is used to compute offset
     */
    if (offset_array[0][0] != 0) {
        stack_offset[top] = 0;
        stack_index[top] = 0;
        top++;
    }

    while (top != 0) {
        row = top - 1;
        /* Get data from Stack */
        index = stack_index[row];
        offset = stack_offset[row];

        if (index == offset_array[row][0]) {
            /* All attr_values at this row is used up to calculate
             * offset, go back one row */
            /* Intentionally left blank */
            ;
        }
        else if ((row < (SVC_METER_MAX_SELECTOR_BITS - 1))
                            && (offset_array[row + 1][0] != 0)) {
            /* Next row have data, push current row to stack */
            stack_offset[top] = offset |
                        offset_array[row][index + 1];
            stack_index[top] = 0;
            top++;
            continue;
        } else {
            /* This is last row which have data. Need to compute final
             * index and populate offset_map */
            for (count = 0;
                    count < offset_array[row][0];
                    count++) {
                final_offset = offset | offset_array[row][ count + 1];
                _bcm_esw_svc_meter_offset_map_update(unit,
                        meter_attr, group_mode, final_offset, pol_offset);
            }
        }
        /* Pop last row */
        top--;
        if (top != 0) {
            /* Increment index for this row */
            row = top - 1;
            stack_index[row] = stack_index[row] + 1;
        }
    }
}

/*
 * Function:
 *      _bcm_policer_svm_pkt_attr_bits_get
 * Purpose:
 *      This internal function is used to get the number of
 *      bits required for attribute in uncompressed mode
 * Parameters:
 *     unit                  - (IN) unit number
 *     pkt_attr              - (IN) packet attribute
 * Returns:
 *     Number of bits in uncompressed mode
 */

STATIC
int _bcm_policer_svm_pkt_attr_bits_get(
    int unit,
    pkt_attrs_t pkt_attr)
{
    _bcm_policer_pkt_attr_bit_pos_t *pkt_attr_bit_pos = NULL;

    pkt_attr_bit_pos =
            BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos;

    return (pkt_attr_bit_pos[pkt_attr].end_bit -
            pkt_attr_bit_pos[pkt_attr].start_bit + 1);
}

/*
 * Function:
 *      _bcm_policer_num_of_bits_get
 * Purpose:
 *      This internal function is used to determine number
 *      of bits required for the given value
 * Parameters:
 *     value                  - (IN) number
 * Returns:
 *     Number of bits
 */
STATIC
uint8 _bcm_policer_num_of_bits_get(uint8 value)
{
    uint8 index = 0;
    for (index = 0; index < 8; index++) {
        if ((value >> index) == 0) {
            break;
        }
    }
    return index;
}

/*
 * Function:
 *      _bcm_esw_policer_group_mode_id_associate
 * Purpose:
 *      This function is used to
 *      1. Create offset map for compressed or uncompressed mode
 *       based on number of bits required for the given input selectors
 *      2. Reserve svm meter mode and update h/w register/table
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) type of group mode
 *     pkt_attr_selectors    - (IN) stores data about selectors
 *     mode_id               - (OUT) mode which used by policer group
 * Returns:
 *     BCM_E_XXX
 */
STATIC
bcm_error_t _bcm_esw_policer_group_mode_id_associate(
    int unit,
    uint32 flags,
    uint32 type,
    bcm_policer_attr_selectors_t *pkt_attr_selectors,
    uint32 *mode_id)
{
    int rv = BCM_E_NONE;
    uint32 pol_offset = 0;
    uint32 total_bits = 0;
    uint32 shift_by_bits = 0;
    uint32 cng_bits = 0;
    uint32 int_pri_bits = 0;
    uint32 vlan_format_bits = 0;
    uint32 outer_dot1p_bits = 0;
    uint32 inner_dot1p_bits = 0;
    uint32 ing_port_bits = 0;
    uint32 pkt_res_bits = 0;
    uint32 svp_type_bits = 0;
    uint32 drop_bits = 0;
    uint32 ip_pkt_bits = 0;
    uint32 tos_dscp_bits = 0;
    uint32 tos_ecn_bits = 0;
    uint32 loop = 0;
    uint32 temp_count = 0;
    uint32 value = 0;
    uint32 index0 = 0, index1 = 0, index2 = 0;
    uint32 final_index = 0, final_mapped_value = 0;
    uint32 mapped_value0 = 0, mapped_value1 = 0, mapped_value2 = 0;
    uint32 cng_cmprsd_bits = 0, cng_attrs = 0;
    uint32 int_pri_cmprsd_bits = 0, int_pri_attrs = 0;
    uint32 vlan_format_cmprsd_bits = 0, vlan_format_attrs = 0;
    uint32 outer_dot1p_cmprsd_bits = 0, outer_dot1p_attrs = 0;
    uint32 inner_dot1p_cmprsd_bits = 0, inner_dot1p_attrs = 0;
    uint32 ing_port_cmprsd_bits = 0, ing_port_attrs = 0;
    uint32 pkt_res_cmprsd_bits = 0, pkt_res_attrs = 0;
    uint32 svp_type_cmprsd_bits = 0, svp_type_attrs = 0;
    uint32 drop_cmprsd_bits = 0, drop_attrs = 0;
    uint32 ip_pkt_cmprsd_bits = 0, ip_pkt_attrs = 0;
    uint32 tos_dscp_cmprsd_bits = 0, tos_dscp_attrs = 0;
    uint32 tos_ecn_cmprsd_bits = 0, tos_ecn_attrs = 0;
    uint8 *pkt_res = NULL;
    uint8 *map_array[bcmPolicerGroupModeAttrCount] = {NULL};
    uint8 *offset_array[SVC_METER_MAX_SELECTOR_BITS] = {NULL};
    uncompressed_attr_selectors_t *uncmprsd_attr_selectors = NULL;
    compressed_attr_selectors_t *cmprsd_attr_selectors = NULL;
    bcm_policer_svc_meter_attr_t *meter_attr = NULL;
    bcm_policer_group_mode_t group_mode;
    bcm_policer_combine_attr_t  *combine_attr_data = NULL;

    if (BCM_SVM_DEV_ATTR(unit)->pkt_resolution != NULL) {
        pkt_res = BCM_SVM_DEV_ATTR(unit)->pkt_resolution;
    } else {
        return (BCM_E_UNAVAIL);
    }

    meter_attr = sal_alloc(sizeof(bcm_policer_svc_meter_attr_t),
            "meter mode attr");
    if (meter_attr == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                (BSL_META_U(unit,
                            "Failed to allocate memory for meter attr\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(meter_attr, 0, sizeof(bcm_policer_svc_meter_attr_t));

    /* bcm_policer_group_mode_t should be based on type */
    if (type == bcmPolicerGroupModeTypeNormal) {
        group_mode = bcmPolicerGroupModeSingle;
    } else if (type == bcmPolicerGroupModeTypeCascade) {
        group_mode = bcmPolicerGroupModeCascade;
    } else {  /* type == bcmPolicerGroupModeTypeCascadeWithCoupling */
        group_mode = bcmPolicerGroupModeCascadeWithCoupling;
    }

    /* Calcuate total bits req. to determine compressed/uncompressed mode */
    if (pkt_attr_selectors->cng == 1) {
        total_bits += (cng_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_cng));
    }
    if (pkt_attr_selectors->int_pri == 1) {
        total_bits += (int_pri_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_int_pri));
    }
    if (pkt_attr_selectors->vlan_format == 1) {
        total_bits += (vlan_format_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_vlan_format));
    }
    if (pkt_attr_selectors->outer_dot1p == 1) {
        total_bits += (outer_dot1p_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_outer_dot1p));
    }
    if (pkt_attr_selectors->inner_dot1p == 1) {
        total_bits += (inner_dot1p_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_inner_dot1p));
    }
    if (pkt_attr_selectors->port == 1) {
        total_bits += (ing_port_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_ing_port));
    }
    if (pkt_attr_selectors->tos_dscp == 1) {
        total_bits += (tos_dscp_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_tos));
    }
    if (pkt_attr_selectors->tos_ecn == 1) {
        total_bits += (tos_ecn_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_tos_ecn));
    }
    if (pkt_attr_selectors->pkt_resolution == 1) {
        total_bits += (pkt_res_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_pkt_resolution));
    }
    if (pkt_attr_selectors->svp_type == 1) {
        total_bits += (svp_type_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_drop));
    }
    if (pkt_attr_selectors->drop == 1) {
        total_bits += (drop_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_drop));
    }
    if (pkt_attr_selectors->ip_pkt == 1) {
        total_bits += (ip_pkt_bits = _bcm_policer_svm_pkt_attr_bits_get(
                    unit, pkt_attr_ip_pkt));
    }

    if (total_bits <= BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
        /* Mode : Uncompressed */
        uncmprsd_attr_selectors = &(meter_attr->uncompressed_attr_selectors_v);
        meter_attr->mode_type_v = uncompressed_mode;

        /* Fill uncompressed_attr_bits_selector */
        if (pkt_attr_selectors->cng == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_CNG_ATTR_BITS;
        }
        if (pkt_attr_selectors->int_pri == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INT_PRI_ATTR_BITS;
        }
        if (pkt_attr_selectors->vlan_format == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_VLAN_FORMAT_ATTR_BITS;
        }
        if (pkt_attr_selectors->outer_dot1p == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_OUTER_DOT1P_ATTR_BITS;
        }
        if (pkt_attr_selectors->inner_dot1p == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INNER_DOT1P_ATTR_BITS;
        }
        if (pkt_attr_selectors->port == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_INGRESS_PORT_ATTR_BITS;
        }
        if (pkt_attr_selectors->tos_dscp == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_TOS_ATTR_BITS;
        }
        if (pkt_attr_selectors->tos_ecn == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_TOS_ECN_ATTR_BITS;
        }
        if (pkt_attr_selectors->pkt_resolution == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_PKT_RESOLUTION_ATTR_BITS;
        }
        if (pkt_attr_selectors->svp_type == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS;
        }
        if (pkt_attr_selectors->drop == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_DROP_ATTR_BITS;
        }
        if (pkt_attr_selectors->ip_pkt == 1) {
            uncmprsd_attr_selectors->uncompressed_attr_bits_selector |=
                BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS;
        }

        /* Fill offset_map field of uncompressed_attr_selectors_v */
        /* Step 1 Reset all values in offset_map */
        sal_memset(uncmprsd_attr_selectors->offset_map, 0,
                sizeof (uncmprsd_attr_selectors->offset_map));

        for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
            offset_array[loop] = (uint8 *) sal_alloc(
                    BCM_POLICER_SVC_METER_MAX_ATTR_SELECTORS + 1,"offset_array");
            if (offset_array[loop] == NULL) {
                rv = BCM_E_MEMORY;
                break;
            }
        }
        if (BCM_FAILURE(rv)) {
            /* Free offset_array */
            for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
                if (offset_array[loop] != NULL) {
                    sal_free(offset_array[loop]);
                }
            }
            sal_free(meter_attr);
            return rv;
        }

        /* Step 2 Calculate offset value for each policer_offset  */
        for (pol_offset = 0;
                pol_offset < pkt_attr_selectors->total_policers;
                pol_offset++ ) {
            combine_attr_data =
                &pkt_attr_selectors->combine_attr_data[pol_offset];
            /* Step 2.1 : Reset offset_array */
            for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
                sal_memset(offset_array[loop], 0,
                        BCM_POLICER_SVC_METER_MAX_ATTR_SELECTORS + 1);
            }
            loop = 0;

            /* Step 2.2 : Parse through each attr type and save values */
            if ((pkt_attr_selectors->cng == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = int_pri_bits +
                    vlan_format_bits +
                    outer_dot1p_bits +
                    inner_dot1p_bits +
                    ing_port_bits +
                    tos_dscp_bits +
                    tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;

                temp_count = offset_array[loop][0];
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_GREEN) {
                    offset_array[loop][temp_count + 1] = (0 << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_YELLOW) {
                    offset_array[loop][temp_count + 1] = (1 << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_RED) {
                    offset_array[loop][temp_count + 1] = (3 << shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->int_pri == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = vlan_format_bits +
                    outer_dot1p_bits +
                    inner_dot1p_bits +
                    ing_port_bits +
                    tos_dscp_bits +
                    tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;

                temp_count = offset_array[loop][0];
                for(index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI15; index0++) {
                    if (combine_attr_data->int_pri_flags & (1 << index0)) {
                        offset_array[loop][temp_count + 1] =
                            (index0 << shift_by_bits);
                        temp_count++;
                    }
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->vlan_format == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = outer_dot1p_bits +
                    inner_dot1p_bits +
                    ing_port_bits +
                    tos_dscp_bits +
                    tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                temp_count = offset_array[loop][0];
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_UNTAGGED) {
                    offset_array[loop][temp_count + 1] = (0 << shift_by_bits);
                    temp_count++;
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_INNER) {
                    offset_array[loop][temp_count + 1] = (1 << shift_by_bits);
                    temp_count++;
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_OUTER) {
                    offset_array[loop][temp_count + 1] = (2 << shift_by_bits);
                    temp_count++;
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_BOTH) {
                    offset_array[loop][temp_count + 1] = (3 << shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->outer_dot1p == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = inner_dot1p_bits +
                    ing_port_bits +
                    tos_dscp_bits +
                    tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                temp_count = offset_array[loop][0];
                for(index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI7; index0++) {
                    if (combine_attr_data->outer_dot1p_flags & (1 << index0)) {
                        offset_array[loop][temp_count + 1] = (index0 << shift_by_bits);
                        temp_count++;
                    }
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->inner_dot1p == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = ing_port_bits +
                    tos_dscp_bits +
                    tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                temp_count = offset_array[loop][0];
                for(index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI7; index0++) {
                    if (combine_attr_data->inner_dot1p_flags & (1 << index0)) {
                        offset_array[loop][temp_count + 1] = (index0 << shift_by_bits);
                        temp_count++;
                    }
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->port == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = tos_dscp_bits +
                    tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                ing_port_attrs = soc_mem_index_max(unit, ING_SVM_PORT_MAPm) + 1;
                for (index0 = 0; index0 < ing_port_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrPort], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] = index0 << shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->tos_dscp == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = tos_ecn_bits +
                    pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                tos_dscp_attrs = 1 << tos_dscp_attrs;
                for (index0 = 0; index0 < tos_dscp_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrTosDscp], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] = index0 << shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->tos_ecn == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = pkt_res_bits +
                    svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                tos_ecn_attrs = 1 << tos_ecn_attrs;
                for (index0 = 0; index0 < tos_ecn_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrTosEcn], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] = index0 << shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->pkt_resolution == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = svp_type_bits +
                    drop_bits +
                    ip_pkt_bits;
                temp_count = offset_array[loop][0];
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_CONTROL_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[CONTROL_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_OAM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[OAM_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_BFD_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[BFD_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_BPDU_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[BPDU_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_ICNM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[ICNM_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_PKT_IS_1588) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[PKT_IS_1588] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L2UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_L2UC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L2UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_L2UC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_L2BC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[L2BC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L2MC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_L2MC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L2MC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_L2MC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L3UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_L3UC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L3UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_L3UC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_IPMC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_IPMC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_IPMC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_IPMC_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L2_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_MPLS_L2_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L3_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_MPLS_L3_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_MPLS_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_MPLS_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_MPLS_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_MULTICAST_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_MPLS_MULTICAST_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MIM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_MIM_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_MIM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_MIM_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_TRILL_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_TRILL_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_TRILL_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_TRILL_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_NIV_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[KNOWN_NIV_PKT] << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_NIV_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (pkt_res[UNKNOWN_NIV_PKT] << shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->svp_type == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = drop_bits +
                    ip_pkt_bits;
                svp_type_attrs = 1 << svp_type_bits;
                for (index0 = 0; index0 < svp_type_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrSvpType], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] = index0 <<
                            shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->drop == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = ip_pkt_bits;
                temp_count = offset_array[loop][0];
                if (combine_attr_data->drop_flags & _BCM_POLICER_DROP_DISABLE) {
                    offset_array[loop][temp_count + 1] = (0 << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->drop_flags & _BCM_POLICER_DROP_ENABLE) {
                    offset_array[loop][temp_count + 1] = (1 << shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->ip_pkt == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = 0;
                temp_count = offset_array[loop][0];
                if (combine_attr_data->ip_pkt_flags &
                        _BCM_POLICER_IP_PKT_DISABLE) {
                    offset_array[loop][temp_count + 1] = (0 << shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->ip_pkt_flags &
                        _BCM_POLICER_IP_PKT_ENABLE) {
                    offset_array[loop][temp_count + 1] = (1 << shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }

            /* Step 2.3 : compute final offset value */
            _bcm_policer_group_mode_offset_map_generate(unit,
                    meter_attr, group_mode, pol_offset, offset_array);
        }
    } else {
        /* Mode : Compressed */
        meter_attr->mode_type_v = compressed_mode;
        cmprsd_attr_selectors = &(meter_attr->compressed_attr_selectors_v);

        cng_bits = _bcm_policer_svm_pkt_attr_bits_get(unit, pkt_attr_cng);
        cng_attrs = 1 << cng_bits;

        int_pri_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_int_pri);
        int_pri_attrs = 1 << int_pri_bits;

        vlan_format_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_vlan_format);
        vlan_format_attrs = 1 << vlan_format_bits;

        outer_dot1p_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_outer_dot1p);
        outer_dot1p_attrs = 1 << outer_dot1p_bits;

        inner_dot1p_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_inner_dot1p);
        inner_dot1p_attrs = 1 << inner_dot1p_bits;

        tos_dscp_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_tos);
        tos_dscp_attrs = 1 << tos_dscp_bits;

        tos_ecn_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_tos_ecn);
        tos_ecn_attrs = 1 << tos_ecn_bits;

        pkt_res_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_pkt_resolution);
        pkt_res_attrs = 1 << pkt_res_bits;

        svp_type_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_svp_group);
        svp_type_attrs = 1 << svp_type_bits;

        drop_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_drop);
        drop_attrs = 1 << drop_bits;

        ip_pkt_bits = _bcm_policer_svm_pkt_attr_bits_get(unit,
                pkt_attr_ip_pkt);
        ip_pkt_attrs = 1 << ip_pkt_bits;

        ing_port_bits = _bcm_policer_svm_pkt_attr_bits_get(
                unit, pkt_attr_ing_port);
        /* Number of port is device dependent. So, get the size from h/w */
        ing_port_attrs = soc_mem_index_max(unit, ING_SVM_PORT_MAPm) + 1;


        /* Step 1 : Create a separate map for each attribute type */
        /* 0XFF means that index is not in use */
        for (pol_offset = 0;
                pol_offset < pkt_attr_selectors->total_policers;
                pol_offset++) {
            combine_attr_data =
                &pkt_attr_selectors->combine_attr_data[pol_offset];
            if (pkt_attr_selectors->cng == 1) {
                if (map_array[bcmPolicerGroupModeAttrFieldIngressColor] == NULL) {
                    map_array[bcmPolicerGroupModeAttrFieldIngressColor] =
                        (uint8 *) sal_alloc(cng_attrs + 1, "cng map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrFieldIngressColor],
                            0xFF, cng_attrs);
                    map_array[bcmPolicerGroupModeAttrFieldIngressColor][cng_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrFieldIngressColor][cng_attrs];
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_GREEN) {
                    if (map_array[bcmPolicerGroupModeAttrFieldIngressColor][0] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrFieldIngressColor][0] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_YELLOW) {
                    if (map_array[bcmPolicerGroupModeAttrFieldIngressColor][1] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrFieldIngressColor][1] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_RED) {
                    if (map_array[bcmPolicerGroupModeAttrFieldIngressColor][3] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrFieldIngressColor][3] = temp_count;
                        temp_count++;
                    }
                }
                map_array[bcmPolicerGroupModeAttrFieldIngressColor][cng_attrs] = temp_count;
            }
            if (pkt_attr_selectors->int_pri == 1) {
                if (map_array[bcmPolicerGroupModeAttrIntPri] == NULL) {
                    map_array[bcmPolicerGroupModeAttrIntPri] =
                        (uint8 *) sal_alloc(int_pri_attrs + 1,"int_pri map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrIntPri], 0xFF, int_pri_attrs);
                    map_array[bcmPolicerGroupModeAttrIntPri][int_pri_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrIntPri][int_pri_attrs];
                for (index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI15; index0++) {
                    if (combine_attr_data->int_pri_flags & (1 << index0)) {
                        if (map_array[bcmPolicerGroupModeAttrIntPri][index0] == 0xFF) {
                            map_array[bcmPolicerGroupModeAttrIntPri][index0] = temp_count;
                            temp_count++;
                        }
                    }
                }
                map_array[bcmPolicerGroupModeAttrIntPri][int_pri_attrs] = temp_count;
            }
            if (pkt_attr_selectors->vlan_format == 1) {
                if (map_array[bcmPolicerGroupModeAttrVlan] == NULL) {
                    map_array[bcmPolicerGroupModeAttrVlan] =
                        (uint8 *) sal_alloc(vlan_format_attrs + 1,"vlan_format map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrVlan], 0xFF, vlan_format_attrs);
                    map_array[bcmPolicerGroupModeAttrVlan][vlan_format_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrVlan][vlan_format_attrs];
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_UNTAGGED) {
                    if (map_array[bcmPolicerGroupModeAttrVlan][0] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrVlan][0] = temp_count;
                        temp_count++;
                    }
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_INNER) {
                    if (map_array[bcmPolicerGroupModeAttrVlan][1] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrVlan][1] = temp_count;
                        temp_count++;
                    }
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_OUTER) {
                    if (map_array[bcmPolicerGroupModeAttrVlan][2] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrVlan][2] = temp_count;
                        temp_count++;
                    }
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_BOTH) {
                    if (map_array[bcmPolicerGroupModeAttrVlan][3] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrVlan][3] = temp_count;
                        temp_count++;
                    }
                }
                map_array[bcmPolicerGroupModeAttrVlan][vlan_format_attrs] = temp_count;
            }
            if (pkt_attr_selectors->outer_dot1p == 1) {
                if (map_array[bcmPolicerGroupModeAttrOuterPri] == NULL) {
                    map_array[bcmPolicerGroupModeAttrOuterPri] =
                        (uint8 *) sal_alloc(outer_dot1p_attrs + 1,"outer_dot1p map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrOuterPri], 0xFF, outer_dot1p_attrs);
                    map_array[bcmPolicerGroupModeAttrOuterPri][outer_dot1p_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrOuterPri][outer_dot1p_attrs];
                for (index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI7; index0++) {
                    if (combine_attr_data->outer_dot1p_flags & (1 << index0)) {
                        if (map_array[bcmPolicerGroupModeAttrOuterPri][index0] == 0xFF) {
                            map_array[bcmPolicerGroupModeAttrOuterPri][index0] = temp_count;
                            temp_count++;
                        }
                    }
                }
                map_array[bcmPolicerGroupModeAttrOuterPri][outer_dot1p_attrs] = temp_count;
            }
            if (pkt_attr_selectors->inner_dot1p == 1) {
                if (map_array[bcmPolicerGroupModeAttrInnerPri] == NULL) {
                    map_array[bcmPolicerGroupModeAttrInnerPri] =
                        (uint8 *) sal_alloc(inner_dot1p_attrs + 1,"inner_dot1p map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrInnerPri], 0xFF, inner_dot1p_attrs);
                    map_array[bcmPolicerGroupModeAttrInnerPri][inner_dot1p_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrInnerPri][inner_dot1p_attrs];
                for (index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI7; index0++) {
                    if (combine_attr_data->inner_dot1p_flags & (1 << index0)) {
                        if (map_array[bcmPolicerGroupModeAttrInnerPri][index0] == 0xFF) {
                            map_array[bcmPolicerGroupModeAttrInnerPri][index0] = temp_count;
                            temp_count++;
                        }
                    }
                }
                map_array[bcmPolicerGroupModeAttrInnerPri][inner_dot1p_attrs] = temp_count;
            }
            if (pkt_attr_selectors->port == 1) {
                if (map_array[bcmPolicerGroupModeAttrPort] == NULL) {
                    map_array[bcmPolicerGroupModeAttrPort] =
                        (uint8 *) sal_alloc(ing_port_attrs + 1,"ing_port map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrPort], 0xFF, ing_port_attrs);
                    map_array[bcmPolicerGroupModeAttrPort][ing_port_attrs] = 0;
                }
                for (value = 0; value < ing_port_attrs; value++) {
                    if (_BCM_POLICER_VALUE_GET (combine_attr_data->
                                value_array[_bcmPolicerAttrPort], value)) {
                        if (map_array[bcmPolicerGroupModeAttrPort][value] == 0xFF) {
                            temp_count = map_array[bcmPolicerGroupModeAttrPort][ing_port_attrs];
                            map_array[bcmPolicerGroupModeAttrPort][value] = temp_count;
                            map_array[bcmPolicerGroupModeAttrPort][ing_port_attrs] = temp_count + 1;
                        }
                    }
                }
            }
            if (pkt_attr_selectors->tos_dscp == 1) {
                if (map_array[bcmPolicerGroupModeAttrTosDscp] == NULL) {
                    map_array[bcmPolicerGroupModeAttrTosDscp] =
                        (uint8 *) sal_alloc(tos_dscp_attrs + 1,"tos dscp map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrTosDscp], 0xFF, tos_dscp_attrs);
                    map_array[bcmPolicerGroupModeAttrTosDscp][tos_dscp_attrs] = 0;
                }
                for (value = 0; value < tos_dscp_attrs; value++) {
                    if (_BCM_POLICER_VALUE_GET (combine_attr_data->
                                value_array[_bcmPolicerAttrTosDscp], value)) {
                        if (map_array[bcmPolicerGroupModeAttrTosDscp][value] == 0xFF) {
                            temp_count = map_array[bcmPolicerGroupModeAttrTosDscp][tos_dscp_attrs];
                            map_array[bcmPolicerGroupModeAttrTosDscp][value] = temp_count;
                            map_array[bcmPolicerGroupModeAttrTosDscp][tos_dscp_attrs] = temp_count + 1;
                        }
                    }
                }
            }
            if (pkt_attr_selectors->tos_ecn == 1) {
                if (map_array[bcmPolicerGroupModeAttrTosEcn] == NULL) {
                    map_array[bcmPolicerGroupModeAttrTosEcn] =
                        (uint8 *) sal_alloc(tos_ecn_attrs + 1,"tos ecn map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrTosEcn], 0xFF, tos_ecn_attrs);
                    map_array[bcmPolicerGroupModeAttrTosEcn][tos_ecn_attrs] = 0;
                }
                for (value = 0; value < tos_ecn_attrs; value++) {
                    if (_BCM_POLICER_VALUE_GET (combine_attr_data->
                                value_array[_bcmPolicerAttrTosEcn], value)) {
                        if (map_array[bcmPolicerGroupModeAttrTosEcn][value] == 0xFF) {
                            temp_count = map_array[bcmPolicerGroupModeAttrTosEcn][tos_ecn_attrs];
                            map_array[bcmPolicerGroupModeAttrTosEcn][value] = temp_count;
                            map_array[bcmPolicerGroupModeAttrTosEcn][tos_ecn_attrs] = temp_count + 1;
                        }
                    }
                }
            }
            if (pkt_attr_selectors->pkt_resolution == 1) {
                if (map_array[bcmPolicerGroupModeAttrPktType] == NULL) {
                    map_array[bcmPolicerGroupModeAttrPktType] =
                        (uint8 *) sal_alloc(pkt_res_attrs + 1,"pkt_res map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrPktType], 0xFF, pkt_res_attrs);
                    map_array[bcmPolicerGroupModeAttrPktType][pkt_res_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrPktType][pkt_res_attrs];
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_CONTROL_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[CONTROL_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[CONTROL_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_OAM_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[OAM_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[OAM_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_BFD_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[BFD_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[BFD_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_BPDU_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[BPDU_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[BPDU_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_ICNM_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[ICNM_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[ICNM_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_PKT_IS_1588) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[PKT_IS_1588]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[PKT_IS_1588]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L2UC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L2UC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L2UC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L2UC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L2UC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L2UC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_L2BC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[L2BC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[L2BC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L2MC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L2MC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L2MC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L2MC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L2MC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L2MC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L3UC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L3UC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L3UC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L3UC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L3UC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L3UC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_IPMC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_IPMC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_IPMC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_IPMC_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_IPMC_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_IPMC_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L2_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_L2_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_L2_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L3_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_L3_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_L3_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_MPLS_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_MPLS_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_MPLS_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_MULTICAST_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_MULTICAST_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_MULTICAST_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MIM_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MIM_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MIM_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_MIM_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_MIM_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_MIM_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_TRILL_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_TRILL_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_TRILL_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_TRILL_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_TRILL_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_TRILL_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_NIV_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_NIV_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_NIV_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_NIV_PKT) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_NIV_PKT]] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_NIV_PKT]] = temp_count;
                        temp_count++;
                    }
                }
                map_array[bcmPolicerGroupModeAttrPktType][pkt_res_attrs] = temp_count;
            }
            if (pkt_attr_selectors->svp_type == 1) {
                if (map_array[bcmPolicerGroupModeAttrIngNetworkGroup] == NULL) {
                    map_array[bcmPolicerGroupModeAttrIngNetworkGroup] =
                        (uint8 *) sal_alloc(svp_type_attrs + 1,"svp type map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrIngNetworkGroup], 0xFF, svp_type_attrs);
                    map_array[bcmPolicerGroupModeAttrIngNetworkGroup][svp_type_attrs] = 0;
                }

                for (value = 0; value < svp_type_attrs; value++) {
                    if (_BCM_POLICER_VALUE_GET (combine_attr_data->
                                value_array[_bcmPolicerAttrSvpType], value)) {
                        if (map_array[bcmPolicerGroupModeAttrIngNetworkGroup][value] == 0xFF) {
                            temp_count = map_array[bcmPolicerGroupModeAttrIngNetworkGroup][svp_type_attrs];
                            map_array[bcmPolicerGroupModeAttrIngNetworkGroup][value] = temp_count;
                            map_array[bcmPolicerGroupModeAttrIngNetworkGroup][svp_type_attrs] = temp_count + 1;
                        }
                    }
                }
            }
            if (pkt_attr_selectors->drop == 1) {
                if (map_array[bcmPolicerGroupModeAttrDrop] == NULL) {
                    map_array[bcmPolicerGroupModeAttrDrop] =
                        (uint8 *) sal_alloc(drop_attrs + 1,"drop map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrDrop], 0xFF, drop_attrs);
                    map_array[bcmPolicerGroupModeAttrDrop][drop_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrDrop][drop_attrs];
                if (combine_attr_data->drop_flags & _BCM_POLICER_DROP_DISABLE) {
                    if (map_array[bcmPolicerGroupModeAttrDrop][0] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrDrop][0] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->drop_flags & _BCM_POLICER_DROP_ENABLE) {
                    if (map_array[bcmPolicerGroupModeAttrDrop][1] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrDrop][1] = temp_count;
                        temp_count++;
                    }
                }
                map_array[bcmPolicerGroupModeAttrDrop][drop_attrs] = temp_count;
            }
            if (pkt_attr_selectors->ip_pkt == 1) {
                if (map_array[bcmPolicerGroupModeAttrPacketTypeIp] == NULL) {
                    map_array[bcmPolicerGroupModeAttrPacketTypeIp] =
                        (uint8 *) sal_alloc(ip_pkt_attrs + 1,"ip_pkt map_array");
                    sal_memset(map_array[bcmPolicerGroupModeAttrPacketTypeIp], 0xFF, ip_pkt_attrs);
                    map_array[bcmPolicerGroupModeAttrPacketTypeIp][ip_pkt_attrs] = 0;
                }
                temp_count = map_array[bcmPolicerGroupModeAttrPacketTypeIp][ip_pkt_attrs];
                if (combine_attr_data->ip_pkt_flags & _BCM_POLICER_IP_PKT_DISABLE) {
                    if (map_array[bcmPolicerGroupModeAttrPacketTypeIp][0] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPacketTypeIp][0] = temp_count;
                        temp_count++;
                    }
                }
                if (combine_attr_data->ip_pkt_flags & _BCM_POLICER_IP_PKT_ENABLE) {
                    if (map_array[bcmPolicerGroupModeAttrPacketTypeIp][1] == 0xFF) {
                        map_array[bcmPolicerGroupModeAttrPacketTypeIp][1] = temp_count;
                        temp_count++;
                    }
                }
                map_array[bcmPolicerGroupModeAttrPacketTypeIp][ip_pkt_attrs] = temp_count;
            }
        }

        /* Step 2: Calculate total bits required.
         * If Exceed 8, then we cannot use even compressed mode */
        if (map_array[bcmPolicerGroupModeAttrFieldIngressColor] != NULL) {
            cng_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrFieldIngressColor][cng_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrIntPri] != NULL) {
            int_pri_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrIntPri][int_pri_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrVlan] != NULL) {
            vlan_format_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrVlan][vlan_format_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrOuterPri] != NULL) {
            outer_dot1p_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrOuterPri][outer_dot1p_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrInnerPri] != NULL) {
            inner_dot1p_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrInnerPri][inner_dot1p_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrPort] != NULL) {
            ing_port_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrPort][ing_port_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrTosDscp] != NULL) {
            tos_dscp_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrTosDscp][tos_dscp_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrTosEcn] != NULL) {
            tos_ecn_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrTosEcn][tos_ecn_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrPktType] != NULL) {
            pkt_res_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrPktType][pkt_res_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrIngNetworkGroup] != NULL) {
            svp_type_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrIngNetworkGroup][svp_type_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrDrop] != NULL) {
            drop_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrDrop][drop_attrs]);
        }
        if (map_array[bcmPolicerGroupModeAttrPacketTypeIp] != NULL) {
            ip_pkt_cmprsd_bits = _bcm_policer_num_of_bits_get(
                    map_array[bcmPolicerGroupModeAttrPacketTypeIp][ip_pkt_attrs]);
        }

        total_bits = cng_cmprsd_bits + int_pri_cmprsd_bits +
            vlan_format_cmprsd_bits + outer_dot1p_cmprsd_bits +
            inner_dot1p_cmprsd_bits + ing_port_cmprsd_bits +
            tos_dscp_cmprsd_bits + tos_ecn_cmprsd_bits +
            pkt_res_cmprsd_bits + svp_type_cmprsd_bits +
            drop_cmprsd_bits + ip_pkt_cmprsd_bits;

        if (total_bits > BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
            /* Free map_array */
            for (loop = 0; loop < bcmPolicerGroupModeAttrCount; loop++) {
                if (map_array[loop] != NULL) {
                    sal_free(map_array[loop]);
                }
            }
            sal_free(meter_attr);
            LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit, "Number of bits"
                            " required is more than 8 even for compressed mode\n")));
            return BCM_E_PARAM;
        }

        /* Step 3 : Filling up Each Logical MAP */
        /* Logical Group-1 int_pri / cng */
        if ((pkt_attr_selectors->cng == 1) ||
                (pkt_attr_selectors->int_pri == 1)) {
            cmprsd_attr_selectors->pkt_attr_bits_v.cng = cng_cmprsd_bits;
            cmprsd_attr_selectors->pkt_attr_bits_v.int_pri = int_pri_cmprsd_bits;

            mapped_value0 = mapped_value1 = 0;

            for (index0 = 0; index0 < (1 << cng_bits); index0++) {
                if (map_array[bcmPolicerGroupModeAttrFieldIngressColor] != NULL) {
                    if (map_array[bcmPolicerGroupModeAttrFieldIngressColor]
                            [index0] == 0xFF) {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrFieldIngressColor][cng_attrs];
                    } else {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrFieldIngressColor][index0];
                    }
                }
                for (index1 = 0; index1 < (1 << int_pri_bits); index1++) {
                    if (map_array[bcmPolicerGroupModeAttrIntPri] != NULL) {
                        if (map_array[bcmPolicerGroupModeAttrIntPri]
                                [index1] == 0xFF) {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrIntPri][int_pri_attrs];

                        } else {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrIntPri][index1];
                        }
                    }
                    final_index = (index0 << int_pri_bits) + index1;
                    final_mapped_value = (mapped_value0 << int_pri_cmprsd_bits) +
                        mapped_value1;
                    cmprsd_attr_selectors->compressed_pri_cnf_attr_map_v[final_index] =
                        final_mapped_value;
                }
            }
        }

        /* Logical Group-2 vlan format / outer_dot1p /inner_dot1p */
        if ((pkt_attr_selectors->vlan_format == 1) ||
                (pkt_attr_selectors->outer_dot1p == 1) ||
                (pkt_attr_selectors->inner_dot1p == 1)) {
            cmprsd_attr_selectors->pkt_attr_bits_v.vlan_format = vlan_format_cmprsd_bits;
            cmprsd_attr_selectors->pkt_attr_bits_v.outer_dot1p = outer_dot1p_cmprsd_bits;
            cmprsd_attr_selectors->pkt_attr_bits_v.inner_dot1p = inner_dot1p_cmprsd_bits;

            mapped_value0 = mapped_value1 = mapped_value2 = 0;

            for (index0 = 0; index0 < (1 << vlan_format_bits); index0++) {
                if (map_array[bcmPolicerGroupModeAttrVlan] != NULL) {
                    if (map_array[bcmPolicerGroupModeAttrVlan][index0] == 0xFF) {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrVlan][vlan_format_attrs];
                    } else {
                        mapped_value0 =
                            map_array[bcmPolicerGroupModeAttrVlan][index0];
                    }
                }
                for (index1 = 0; index1 < (1 << outer_dot1p_bits); index1++) {
                    if (map_array[bcmPolicerGroupModeAttrOuterPri] != NULL) {
                        if (map_array[bcmPolicerGroupModeAttrOuterPri][index1] == 0xFF) {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrOuterPri][outer_dot1p_attrs];
                        } else {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrOuterPri][index1];
                        }
                    }
                    for (index2 = 0; index2 < (1 << inner_dot1p_bits); index2++) {
                        if (map_array[bcmPolicerGroupModeAttrInnerPri] != NULL) {
                            if (map_array[bcmPolicerGroupModeAttrInnerPri]
                                    [index2] == 0xFF) {
                                mapped_value2 = map_array
                                    [bcmPolicerGroupModeAttrInnerPri][inner_dot1p_attrs];
                            } else {
                                mapped_value2 = map_array
                                    [bcmPolicerGroupModeAttrInnerPri][index2];
                            }
                        }
                        final_index = (index0 << (outer_dot1p_bits + inner_dot1p_bits)) +
                            (index1 << inner_dot1p_bits) + index2;
                        final_mapped_value = (mapped_value0 << (outer_dot1p_cmprsd_bits +
                                    inner_dot1p_cmprsd_bits)) +
                            (mapped_value1 << inner_dot1p_cmprsd_bits) +
                            mapped_value2;
                        cmprsd_attr_selectors->compressed_pkt_pri_attr_map_v[final_index] =
                            final_mapped_value;
                    }
                }
            }
        }

        /* Logical Group-3 port */
        if (pkt_attr_selectors->port == 1) {
            mapped_value0 = 0;
            cmprsd_attr_selectors->pkt_attr_bits_v.ing_port = ing_port_cmprsd_bits;

            if (map_array[bcmPolicerGroupModeAttrPort] != NULL) {
                /* Use ing_port_attrs as it is device dependent */
                for (index0 =0; index0 < ing_port_attrs; index0++ ) {
                    if (map_array[bcmPolicerGroupModeAttrPort][index0] == 0xFF) {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrPort][ing_port_attrs];
                    } else {
                        mapped_value0 =
                            map_array[bcmPolicerGroupModeAttrPort][index0];
                    }
                    final_index = index0;
                    final_mapped_value = mapped_value0;
                    cmprsd_attr_selectors->compressed_port_attr_map_v[final_index] =
                        final_mapped_value;
                }
            }
        }

        /* Logical Group-4 tos_dscp / tos_ecn */
        if ((pkt_attr_selectors->tos_dscp == 1) ||
                (pkt_attr_selectors->tos_ecn == 1)) {
            mapped_value0 = mapped_value1 = 0;
            cmprsd_attr_selectors->pkt_attr_bits_v.tos = tos_dscp_cmprsd_bits;
            cmprsd_attr_selectors->pkt_attr_bits_v.tos_ecn = tos_ecn_cmprsd_bits;

            for (index0 = 0; index0 < (1 << tos_dscp_bits); index0++) {
                if (map_array[bcmPolicerGroupModeAttrTosDscp] != NULL) {
                    if (map_array[bcmPolicerGroupModeAttrTosDscp][index0] == 0xFF) {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrTosDscp][tos_dscp_attrs];
                    } else {
                        mapped_value0 =
                            map_array[bcmPolicerGroupModeAttrTosDscp][index0];
                    }
                }
                for (index1 = 0; index1 < (1 << tos_ecn_bits); index1++) {
                    if (map_array[bcmPolicerGroupModeAttrTosEcn] != NULL) {
                        if (map_array[bcmPolicerGroupModeAttrTosEcn][index1] == 0xFF) {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrTosEcn][tos_ecn_attrs];
                        } else {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrTosEcn][index1];
                        }
                    }
                    final_index = (index0 << tos_ecn_bits) + index1;
                    final_mapped_value = (mapped_value0 << tos_ecn_cmprsd_bits) + mapped_value1;
                    cmprsd_attr_selectors->compressed_tos_attr_map_v[final_index] =
                        final_mapped_value;
                }
            }
        }

        /* Logical Group-5 pkt_resolution / network group(svp type) / drop */
        if ((pkt_attr_selectors->pkt_resolution == 1) ||
                (pkt_attr_selectors->svp_type == 1) ||
                (pkt_attr_selectors->drop ==1)) {
            cmprsd_attr_selectors->pkt_attr_bits_v.pkt_resolution = pkt_res_cmprsd_bits;
            cmprsd_attr_selectors->pkt_attr_bits_v.svp_type = svp_type_cmprsd_bits;
            cmprsd_attr_selectors->pkt_attr_bits_v.drop = drop_cmprsd_bits;

            mapped_value0 = mapped_value1 = mapped_value2 = 0;

            for (index0 = 0; index0 < (1 << pkt_res_bits); index0++) {
                if (map_array[bcmPolicerGroupModeAttrPktType] != NULL) {
                    if (map_array[bcmPolicerGroupModeAttrPktType][index0] == 0xFF) {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrPktType][pkt_res_attrs];
                    } else {
                        mapped_value0 = map_array
                            [bcmPolicerGroupModeAttrPktType][index0];
                    }
                }
                for (index1 = 0; index1 < (1 << svp_type_bits); index1++) {
                    if (map_array[bcmPolicerGroupModeAttrIngNetworkGroup] != NULL) {
                        if (map_array[bcmPolicerGroupModeAttrIngNetworkGroup][index1] == 0xFF) {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrIngNetworkGroup][svp_type_attrs];
                        } else {
                            mapped_value1 = map_array
                                [bcmPolicerGroupModeAttrIngNetworkGroup][index1];
                        }
                    }
                    for (index2 = 0; index2 < (1 << drop_bits); index2++) {
                        if (map_array[bcmPolicerGroupModeAttrDrop] != NULL) {
                            if (map_array[bcmPolicerGroupModeAttrDrop][index2] == 0xFF) {
                                mapped_value2 = map_array
                                    [bcmPolicerGroupModeAttrDrop][drop_attrs];
                            } else {
                                mapped_value2 = map_array
                                    [bcmPolicerGroupModeAttrDrop][index2];
                            }
                        }
                        final_index = (index0 << (svp_type_bits + drop_bits)) +
                            (index1 << drop_bits) + index2;
                        final_mapped_value = (mapped_value0 << (svp_type_cmprsd_bits +
                                    drop_cmprsd_bits)) + (mapped_value1 << drop_cmprsd_bits) +
                            mapped_value2;
                        cmprsd_attr_selectors->compressed_pkt_res_attr_map_v[final_index] =
                            final_mapped_value;
                    }
                }
            }
        }

        if (pkt_attr_selectors->ip_pkt == 1) {
            cmprsd_attr_selectors->pkt_attr_bits_v.ip_pkt = ip_pkt_cmprsd_bits;
        }

        /* Step 4 : Compute offset_map */
        /* Fill offset_map field of uncompressed_attr_selectors_v */
        /* Step 4.1 : Reset all values in offset_map */
        sal_memset(cmprsd_attr_selectors->offset_map, 0,
                sizeof (cmprsd_attr_selectors->offset_map));

        for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
            offset_array[loop] =
                (uint8 *) sal_alloc(BCM_SVC_METER_MAP_SIZE_256 + 1,
                        "offset_array");
            if (offset_array[loop] == NULL) {
                rv = BCM_E_MEMORY;
                break;
            }
        }
        if (BCM_FAILURE(rv)) {
            /* Free offset_array */
            for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
                if (offset_array[loop] != NULL) {
                    sal_free(offset_array[loop]);
                }
            }
            /* free map_array */
            for (loop = 0; loop < bcmPolicerGroupModeAttrCount; loop++) {
                if (map_array[loop] != NULL) {
                    sal_free(map_array[loop]);
                }
            }
            sal_free(meter_attr);
            return rv;
        }

        /* Step 4.2 : Calculate offset value for each pol_offset  */
        for (pol_offset = 0;
                pol_offset < pkt_attr_selectors->total_policers ;
                pol_offset++ ) {
            combine_attr_data =
                &pkt_attr_selectors->combine_attr_data[pol_offset];
            /* Step 4.2.1 : Reset offset_array */
            for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
                sal_memset(offset_array[loop], 0,
                        BCM_POLICER_SVC_METER_MAX_ATTR_SELECTORS + 1);
            }
            loop = 0;

            /* Step 4.2.2 : Parse each attr types and save values */
            if ((pkt_attr_selectors->cng == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = int_pri_cmprsd_bits +
                    vlan_format_cmprsd_bits +
                    outer_dot1p_cmprsd_bits +
                    inner_dot1p_cmprsd_bits +
                    ing_port_cmprsd_bits +
                    tos_dscp_cmprsd_bits +
                    tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_GREEN) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrFieldIngressColor][0] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_YELLOW) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrFieldIngressColor][1] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->cng_flags & _BCM_POLICER_COLOR_RED) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrFieldIngressColor][3] <<
                         shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->int_pri == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = vlan_format_cmprsd_bits +
                    outer_dot1p_cmprsd_bits +
                    inner_dot1p_cmprsd_bits +
                    ing_port_cmprsd_bits +
                    tos_dscp_cmprsd_bits +
                    tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                for (index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI15; index0++) {
                    if(combine_attr_data->int_pri_flags & (1 << index0)) {
                        offset_array[loop][temp_count + 1] =
                            (map_array[bcmPolicerGroupModeAttrIntPri][index0] <<
                             shift_by_bits);
                        temp_count++;
                    }
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->vlan_format == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = outer_dot1p_cmprsd_bits +
                    inner_dot1p_cmprsd_bits +
                    ing_port_cmprsd_bits +
                    tos_dscp_cmprsd_bits +
                    tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_UNTAGGED) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrVlan][0] <<
                         shift_by_bits);
                    temp_count++;
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_INNER) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrVlan][1] <<
                         shift_by_bits);
                    temp_count++;
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_OUTER) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrVlan][2] <<
                         shift_by_bits);
                    temp_count++;
                }
                if(combine_attr_data->vlan_format_flags &
                        _BCM_POLICER_VLAN_FORMAT_BOTH) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrVlan][3] <<
                         shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->outer_dot1p == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = inner_dot1p_cmprsd_bits +
                    ing_port_cmprsd_bits +
                    tos_dscp_cmprsd_bits +
                    tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                for (index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI7; index0++) {
                    if(combine_attr_data->outer_dot1p_flags & (1 << index0)) {
                        offset_array[loop][temp_count + 1] =
                            (map_array[bcmPolicerGroupModeAttrOuterPri][index0] <<
                             shift_by_bits);
                        temp_count++;
                    }
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->inner_dot1p == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = ing_port_cmprsd_bits +
                    tos_dscp_cmprsd_bits +
                    tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                for (index0 = _BCM_POLICER_PKT_PRI0;
                        index0 <= _BCM_POLICER_PKT_PRI7; index0++) {
                    if(combine_attr_data->inner_dot1p_flags & (1 << index0)) {
                        offset_array[loop][temp_count + 1] =
                            (map_array[bcmPolicerGroupModeAttrInnerPri][index0] <<
                             shift_by_bits);
                        temp_count++;
                    }
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->port == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = tos_dscp_cmprsd_bits +
                    tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                for (index0 = 0; index0 < ing_port_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrPort], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] =
                            map_array[bcmPolicerGroupModeAttrPort][index0] <<
                            shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->tos_dscp == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = tos_ecn_cmprsd_bits +
                    pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                for (index0 = 0; index0 < tos_dscp_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrTosDscp], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] =
                            map_array[bcmPolicerGroupModeAttrTosDscp][index0] <<
                            shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->tos_ecn == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = pkt_res_cmprsd_bits +
                    svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                for (index0 = 0; index0 < tos_ecn_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrTosEcn], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] =
                            map_array[bcmPolicerGroupModeAttrTosEcn][index0] <<
                            shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->pkt_resolution == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = svp_type_cmprsd_bits +
                    drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_CONTROL_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[CONTROL_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_OAM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[OAM_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_BFD_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[BFD_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_BPDU_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[BPDU_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_ICNM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[ICNM_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_PKT_IS_1588) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[PKT_IS_1588]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L2UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L2UC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L2UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L2UC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_L2BC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[L2BC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L2MC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L2MC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L2MC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L2MC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_L3UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_L3UC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_L3UC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_L3UC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_IPMC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_IPMC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_IPMC_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_IPMC_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L2_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_L2_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_L3_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_L3_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_MPLS_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_MPLS_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MPLS_MULTICAST_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MPLS_MULTICAST_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_MIM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_MIM_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_MIM_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_MIM_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_TRILL_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_TRILL_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_TRILL_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_TRILL_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_KNOWN_NIV_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[KNOWN_NIV_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->pkt_resolution_flags &
                        _BCM_POLICER_PKT_TYPE_UNKNOWN_NIV_PKT) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPktType][pkt_res[UNKNOWN_NIV_PKT]] <<
                         shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->svp_type == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = drop_cmprsd_bits +
                    ip_pkt_cmprsd_bits;

                for (index0 = 0; index0 < svp_type_attrs; index0++) {
                    if (_BCM_POLICER_VALUE_GET(combine_attr_data->
                                value_array[_bcmPolicerAttrSvpType], index0)) {
                        temp_count = offset_array[loop][0];
                        offset_array[loop][temp_count + 1] =
                            map_array[bcmPolicerGroupModeAttrIngNetworkGroup][index0] <<
                            shift_by_bits;
                        offset_array[loop][0] =  temp_count + 1;
                    }
                }
                loop++;
            }
            if ((pkt_attr_selectors->drop == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = ip_pkt_cmprsd_bits;

                temp_count = offset_array[loop][0];
                if (combine_attr_data->drop_flags & _BCM_POLICER_DROP_DISABLE) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrDrop][0] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->drop_flags & _BCM_POLICER_DROP_ENABLE) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrDrop][1] <<
                         shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }
            if ((pkt_attr_selectors->ip_pkt == 1) &&
                    (loop <= (SVC_METER_MAX_SELECTOR_BITS - 2))) {
                shift_by_bits = 0;
                temp_count = offset_array[loop][0];
                if (combine_attr_data->ip_pkt_flags & _BCM_POLICER_IP_PKT_DISABLE) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPacketTypeIp][0] <<
                         shift_by_bits);
                    temp_count++;
                }
                if (combine_attr_data->ip_pkt_flags & _BCM_POLICER_IP_PKT_ENABLE) {
                    offset_array[loop][temp_count + 1] =
                        (map_array[bcmPolicerGroupModeAttrPacketTypeIp][1] <<
                         shift_by_bits);
                    temp_count++;
                }
                offset_array[loop][0] = temp_count;
                loop++;
            }

            /* Step 4.2.3 : compute final offset value */
            _bcm_policer_group_mode_offset_map_generate(unit,
                    meter_attr, group_mode, pol_offset, offset_array);
        }
    }

    /* free map_array */
    for (loop = 0; loop < bcmPolicerGroupModeAttrCount; loop++) {
        if (map_array[loop] != NULL) {
            sal_free(map_array[loop]);
        }
    }
    /* Free offset_array */
    for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
        if (offset_array[loop] != NULL) {
            sal_free(offset_array[loop]);
        }
    }
    GLOBAL_METER_LOCK(unit);
    rv = _bcm_esw_policer_svc_meter_create_mode (unit, meter_attr, group_mode,
            type, pkt_attr_selectors->total_policers, mode_id);
    if (BCM_FAILURE(rv) && (rv != BCM_E_EXISTS)) {
        GLOBAL_METER_UNLOCK(unit);
        sal_free(meter_attr);
        return rv;
    } else if (rv == BCM_E_EXISTS) {
        GLOBAL_METER_UNLOCK(unit);
        sal_free(meter_attr);
        return BCM_E_NONE;
    }
    GLOBAL_METER_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_policer_custom_group_mode_id_create
 * Purpose:
 *      This function is used to configure customnized group mode
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) type of group mode
 *     total_policers        - (IN) total policers
 *     num_selectors         - (IN) number of selectors
 *     attr_selectors        - (IN) list of attributes selectors
 *     mode_id               - (OUT) pointer to offset mode index
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_policer_custom_group_mode_id_create(
    int unit,
    uint32 flags,
    bcm_policer_group_mode_type_t type,
    uint32 total_policers,
    uint32 num_selectors,
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    uint32 *mode_id)
{
    int rv = BCM_E_NONE;
    bcm_policer_attr_selectors_t pkt_attr_selectors;
    uint32 npolicers = 0;

    npolicers = total_policers;
    if (type == bcmPolicerGroupModeTypeCascadeWithCoupling) {
        npolicers = total_policers/2;
    }

    /* Check if mode info already exists */
    rv = _bcm_esw_svc_meter_offset_mode_id_check(unit, flags, type,
            total_policers, num_selectors, attr_selectors, mode_id);
    BCM_IF_ERROR_RETURN(rv);

    sal_memset(&pkt_attr_selectors, 0, sizeof(bcm_policer_attr_selectors_t));
    pkt_attr_selectors.combine_attr_data =
            sal_alloc(sizeof(bcm_policer_combine_attr_t) * npolicers,
            "combine attributes");
    if (pkt_attr_selectors.combine_attr_data == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(pkt_attr_selectors.combine_attr_data, 0,
            sizeof(bcm_policer_combine_attr_t) * npolicers);

    rv = _bcm_esw_policer_group_mode_fillup_values (unit, flags, type,
                npolicers, num_selectors, attr_selectors, &pkt_attr_selectors);
    if (rv != BCM_E_NONE) {
        sal_free(pkt_attr_selectors.combine_attr_data);
        return rv;
    }

    rv = _bcm_esw_policer_group_mode_id_associate(unit, flags,
                type, &pkt_attr_selectors, mode_id);
    if (rv != BCM_E_NONE) {
        sal_free(pkt_attr_selectors.combine_attr_data);
        return rv;
    }

    GLOBAL_METER_LOCK(unit);
    /* In case of group type is cascade/cascadewithcoupling, total_policers
     * must be set to 8. This requirement is due to the fact that in these
     * two cases, meter index allocation is one complete octuple */
    if ((type == bcmPolicerGroupModeTypeCascade) ||
            (type == bcmPolicerGroupModeTypeCascadeWithCoupling)) {
        global_meter_offset_mode[unit][*mode_id].no_of_policers = 8;
    } else {
        global_meter_offset_mode[unit][*mode_id].no_of_policers =
                                                        total_policers;
    }
    rv = _bcm_policer_svc_meter_group_offset_mode_info_update (unit, *mode_id,
                    type, num_selectors, attr_selectors);
    if (BCM_FAILURE(rv)) {
        _bcm_policer_svc_meter_unreserve_mode (unit, *mode_id);
        GLOBAL_METER_UNLOCK(unit);
        return rv;
    }
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_udf_custom_group_create
 * Purpose:
 *      This function is used to configure udf based
 *      group mode
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) type of group mode
 *     total_policers        - (IN) total policers
 *     num_selectors         - (IN) number of selectors
 *     attr_selectors        - (IN) list of attributes selectors
 *     mode_id               - (OUT) pointer to offset mode index
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_esw_policer_udf_custom_group_create(
    int unit,
    uint32 flags,
    bcm_policer_group_mode_type_t type,
    uint32 total_policers,
    uint32 num_selectors,
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    uint32 *mode_id)
{
    int rv = BCM_E_NONE;
    bcm_policer_udf_attr_selectors_t udf_attr_selectors;
    bcm_policer_udf_pkt_combine_attr_t *udf_combine_data = NULL;
    bcm_policer_udf_sub_div_t dummy_data;

    bcm_policer_svc_meter_attr_t *meter_attr = NULL;
    bcm_policer_group_mode_t group_mode;
    bcm_policer_svc_meter_mode_type_t mode_type_v;
    uint32 npolicers = 0;

    uint8 shift_by_bits_array[SVC_METER_UDF1_MAX_BIT_POSITION] = {0};
    int8 loop_index_array[SVC_METER_UDF1_MAX_BIT_POSITION] = {-1};
    uint8 *offset_array[SVC_METER_MAX_SELECTOR_BITS] = {NULL};
    uint32 sorted = 0, total_bits = 0, shift_by_bits = 0;
    uint32 index0 = 0, index1 = 0;
    uint32 loop_index = 0, loop =0;
    uint32 offset = 0, total_offsets = 0;
    uint32 pol_offset = 0;

    npolicers = total_policers;
    if (type == bcmPolicerGroupModeTypeCascadeWithCoupling) {
        npolicers = total_policers/2;
    }

    /* Check if mode info already exists */
    rv = _bcm_esw_svc_meter_offset_mode_id_check(unit, flags, type,
                total_policers, num_selectors, attr_selectors, mode_id);
    BCM_IF_ERROR_RETURN(rv);

    sal_memset (&udf_attr_selectors, 0,
            sizeof(bcm_policer_udf_attr_selectors_t));
    udf_attr_selectors.combine_attr_data =
            sal_alloc(sizeof(bcm_policer_udf_pkt_combine_attr_t) * npolicers,
            "combine attributes");

    if (udf_attr_selectors.combine_attr_data == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(udf_attr_selectors.combine_attr_data, 0,
            sizeof(bcm_policer_udf_pkt_combine_attr_t) * npolicers);

    rv = _bcm_esw_policer_group_udf_mode_fillup_values (unit, flags, type,
                npolicers, num_selectors, attr_selectors, &udf_attr_selectors);
    if (rv != BCM_E_NONE) {
        sal_free(udf_attr_selectors.combine_attr_data);
        return rv;
    }

    /* Below block of code is intended to sort the input offset/width
     * in ascending order of udf offsets numerically.
     * 'total_bits' required is also calculated to check if creating mode
     * with given attr_selectors is possible or not.
     * Also, shift bits needs to be calculated which is later used to
     * generate offset to be indexed for SVM_OFFSET_TABLE
     */
    sorted = total_bits = shift_by_bits = 0;
    if (udf_attr_selectors.drop) {
        total_bits++;
        shift_by_bits = 1;
    }
    /* Loop through lower offset to higher offset index to calculate shift */
    for (index0 = 0; index0 < SVC_METER_UDF1_MAX_BIT_POSITION; index0++) {
        if (udf_attr_selectors.udf_bits[index0] == 1) {
            for (index1 = sorted; index1 < udf_attr_selectors.total_subdiv; index1++) {
                if (udf_attr_selectors.udf_subdiv[index1].offset == index0) {
                    /* Swap to rearrange */
                    sal_memcpy(&dummy_data, &udf_attr_selectors.udf_subdiv[index1],
                                sizeof (bcm_policer_udf_sub_div_t));
                    sal_memcpy(&udf_attr_selectors.udf_subdiv[index1],
                                &udf_attr_selectors.udf_subdiv[sorted],
                                sizeof(bcm_policer_udf_sub_div_t));
                    sal_memcpy(&udf_attr_selectors.udf_subdiv[sorted], &dummy_data,
                                        sizeof (bcm_policer_udf_sub_div_t));
                    total_bits += udf_attr_selectors.udf_subdiv[sorted].width;
                    shift_by_bits_array[index0] = shift_by_bits;
                    shift_by_bits += udf_attr_selectors.udf_subdiv[sorted].width;
                    sorted++;
                    break;
                }
            }
        }
    }

    if (total_bits > BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) {
        sal_free(udf_attr_selectors.combine_attr_data);
        return BCM_E_PARAM;
    }

    meter_attr = sal_alloc(sizeof(bcm_policer_svc_meter_attr_t),"meter mode attr");
    if ( meter_attr == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Failed to allocate memory for svc meter attr \n")));
        sal_free(udf_attr_selectors.combine_attr_data);
        return BCM_E_MEMORY;
    }
    sal_memset (meter_attr, 0, sizeof(bcm_policer_svc_meter_attr_t));

    /* bcm_policer_group_mode_t should be based on type */
    if (type == bcmPolicerGroupModeTypeNormal) {
        mode_type_v = udf_mode;
        group_mode = bcmPolicerGroupModeSingle;
    } else if (type == bcmPolicerGroupModeTypeCascade) {
        mode_type_v = udf_cascade_mode;
        group_mode = bcmPolicerGroupModeCascade;
    } else {  /* type == bcmPolicerGroupModeTypeCascadeWithCoupling */
        mode_type_v = udf_cascade_with_coupling_mode;
        group_mode = bcmPolicerGroupModeCascadeWithCoupling;
    }
    meter_attr->udf_pkt_attr_selectors_v.num_selectors = num_selectors;
    meter_attr->udf_pkt_attr_selectors_v.udf_id = attr_selectors[0].udf_id;
    meter_attr->mode_type_v = mode_type_v;

    if (udf_attr_selectors.drop == 1) {
        meter_attr->udf_pkt_attr_selectors_v.drop = 1;
    }
    if (udf_attr_selectors.total_subdiv > 0) {
        meter_attr->udf_pkt_attr_selectors_v.total_subdiv =
                                        udf_attr_selectors.total_subdiv;
        sal_memcpy(&meter_attr->udf_pkt_attr_selectors_v.udf_subdiv[0],
                    &udf_attr_selectors.udf_subdiv[0],
                    sizeof(udf_attr_selectors.udf_subdiv));
    }

    for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
        offset_array[loop] =
                    (uint8 *) sal_alloc(BCM_POLICER_SVC_METER_MAX_ATTR_SELECTORS + 1,
                                "offset_array");
        if (offset_array[loop] == NULL) {
            rv = BCM_E_MEMORY;
            break;
        }
    }
    if (BCM_FAILURE(rv)) {
        for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
            if (offset_array[loop] != NULL) {
                sal_free(offset_array[loop]);
            }
        }
        sal_free(meter_attr);
        sal_free(udf_attr_selectors.combine_attr_data);
        return rv;
    }
    /* Loop through all policer offset and compute offset map */
    for (pol_offset = 0; pol_offset < npolicers; pol_offset++) {
        sal_memset(&loop_index_array[0], -1, sizeof (loop_index_array));
        for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
            sal_memset(offset_array[loop], 0,
                    BCM_POLICER_SVC_METER_MAX_ATTR_SELECTORS + 1);
        }
        loop = loop_index = 0;
        udf_combine_data = &udf_attr_selectors.combine_attr_data[pol_offset];
        /* Loop through all udf sub-division per policer offset */
        for (index1 = 0; index1 < udf_combine_data->selectors; index1++) {
            offset = udf_combine_data->udf_subdiv[index1].offset;
            /* Get loop_index */
            if (loop_index_array[offset] == -1) {
                loop_index_array[offset] = loop_index;
                loop_index++;
            }
            loop = loop_index_array[offset];

            /* get current count */
            total_offsets = offset_array[loop][0];
            /* Save the value */
            offset_array[loop][total_offsets + 1] =
                udf_combine_data->attr_value[index1] << shift_by_bits_array[offset];
            offset_array[loop][0] = total_offsets + 1;
        }
        if (udf_attr_selectors.drop == 1) {
            loop = loop_index;
            total_offsets = offset_array[loop][0];
            if ((udf_combine_data->drop_flags & _BCM_POLICER_DROP_DISABLE) != 0) {
                offset_array[loop][total_offsets + 1] = 0;
                total_offsets++;
            }
            if ((udf_combine_data->drop_flags & _BCM_POLICER_DROP_ENABLE) != 0) {
                offset_array[loop][total_offsets + 1] = 1;
                total_offsets++;
            }
            offset_array[loop][0] = total_offsets;
        }

        /* Make final offset calculation */
        _bcm_policer_group_mode_offset_map_generate(unit, meter_attr, group_mode,
                pol_offset, offset_array);
    }

    for (loop = 0; loop < SVC_METER_MAX_SELECTOR_BITS; loop++) {
        if (offset_array[loop] != NULL) {
            sal_free(offset_array[loop]);
        }
    }
    if (udf_attr_selectors.combine_attr_data != NULL) {
        sal_free(udf_attr_selectors.combine_attr_data);
    }

    GLOBAL_METER_LOCK(unit);
    rv = _bcm_esw_policer_svc_meter_create_mode (unit, meter_attr, group_mode,
               type, total_policers, mode_id);
    if (BCM_FAILURE(rv) && (rv != BCM_E_EXISTS)) {
        GLOBAL_METER_UNLOCK(unit);
        sal_free(meter_attr);
        return rv;
    } else if (rv == BCM_E_EXISTS) {
        GLOBAL_METER_UNLOCK(unit);
        sal_free(meter_attr);
        return BCM_E_NONE;
    } else {
        /* In case of group type is cascade/cascadewithcoupling, total_policers
         * must be set to 8. This requirement is due to the fact that in these
         * two cases, meter index allocation is whole octuple */
        if ((type == bcmPolicerGroupModeTypeCascade) ||
                (type == bcmPolicerGroupModeTypeCascadeWithCoupling)) {
            global_meter_offset_mode[unit][*mode_id].no_of_policers = 8;
        } else {
            global_meter_offset_mode[unit][*mode_id].no_of_policers =
                    total_policers;
        }

        rv = _bcm_policer_svc_meter_group_offset_mode_info_update (unit,
                *mode_id, type, num_selectors, attr_selectors);
        if (BCM_FAILURE(rv)) {
            _bcm_policer_svc_meter_unreserve_mode (unit, *mode_id);
            GLOBAL_METER_UNLOCK(unit);
            sal_free(meter_attr);
            return rv;
        }
    }
    GLOBAL_METER_UNLOCK(unit);
    sal_free(meter_attr);
    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_group_mode_id_create
 * Purpose:
 *     Create Customized Policer Group mode for given Policer Attributes
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     type                  - (IN) modes of policer group creation -
 *                                  normal/cascade/cascade with coupling.
 *     total_policers        - (IN) Total Policers in Policer Group Mode
 *     num_selectors         - (IN) Number of Selectors for policer Group Mode
 *     attr_selectors        - (IN) Attribute Selectors for Policer Group Mode
 *     mode_id               - (OUT) Created mode Id for Policer Group Mode
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_policer_group_mode_id_create(
    int unit,
    uint32 flags,
    bcm_policer_group_mode_type_t type,
    uint32 total_policers,
    uint32 num_selectors,
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    uint32 *mode_id)
{
    int rv = BCM_E_NONE;
    int num_pools = 0;
    uint32 index = 0;
    int8 custom_group_udf_type = -1;

    num_pools =  SOC_INFO(unit).global_meter_pools;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    if (type >= bcmPolicerGroupModeTypeCount) {
        return (BCM_E_PARAM);
    }

    /* Attribute selectors are used for selecting one of the policers created in
     * policer group. For total_policer=1 there is just one policer.
     * Hence, attribute selectors have no significance.
     * All other IN parameters are set fixed to avoid any confusion
     * while fetching group policer config via bcm_policer_group_mode_id_get()
     */
    if (total_policers == 1) {
        if ((type == bcmPolicerGroupModeTypeNormal) && (attr_selectors == NULL)
                    && (num_selectors == 0)) {
            *mode_id = 1;
            return (BCM_E_NONE);
        } else {
            return (BCM_E_PARAM);
        }
    }

    if ((total_policers == 0) || (num_selectors == 0)) {
        return (BCM_E_PARAM);
    }
    if ((type == bcmPolicerGroupModeTypeNormal) && (total_policers > 256)) {
        return (BCM_E_PARAM);
    }
    if ((type == bcmPolicerGroupModeTypeCascade)
                    && (total_policers > num_pools)) {
        return (BCM_E_PARAM);
    }
    if (type == bcmPolicerGroupModeTypeCascadeWithCoupling) {
        if (total_policers > (num_pools/2)) {
            return (BCM_E_PARAM);
        }
        total_policers = 8;
    }

    if (attr_selectors == NULL) {
        return (BCM_E_PARAM);
    }

    /* loop through attr_selectors[x].attr to determine correct internal API call */
    for (index = 0; index < num_selectors; index++) {
        if (attr_selectors[index].attr == bcmPolicerGroupModeAttrUdf) {
            if (custom_group_udf_type == 0) {
                rv = BCM_E_PARAM;
                break;
            }
            custom_group_udf_type = 1;
        } else if (attr_selectors[index].attr != bcmPolicerGroupModeAttrDrop) {
            if (custom_group_udf_type == 1) {
                rv = BCM_E_PARAM;
                break;
            }
            custom_group_udf_type = 0;
        }
    }
    if (rv == BCM_E_PARAM) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                  "Mix of UDF and other attributes not allowed\n")));
        return rv;
    }

    if (custom_group_udf_type == 0) {
        rv = _bcm_policer_custom_group_mode_id_create(unit, flags, type,
                                        total_policers, num_selectors,
                                        attr_selectors, mode_id);
    } else {
        rv = _bcm_esw_policer_udf_custom_group_create(unit, flags, type,
                                        total_policers, num_selectors,
                                        attr_selectors, mode_id);
    }
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                  "Unable to create policer group  \n")));
         return (rv);
    }
    *mode_id = *mode_id + 1;
    LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Created policer group with mode id 0x%x \n"), *mode_id));
    return rv;

}


/*
 * Function:
 *      bcm_esw_policer_group_mode_id_destroy
 * Purpose:
 *     Destroy Customized Policer Group mode with given mode_id.
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode_id               - (IN) Created mode Id for Policer Group Mode
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_policer_group_mode_id_destroy(
    int unit,
    uint32 mode_id)
{
    bcm_policer_svc_meter_bookkeep_mode_t   mode_info;
    int rv = BCM_E_NONE;
    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    if ((mode_id == 0) || (mode_id > BCM_POLICER_SVC_METER_MAX_MODE)) {
        return (BCM_E_PARAM);
    }
    mode_id -= 1;
    if (mode_id == 0) {
        return (BCM_E_NONE);
    }

    sal_memset(&mode_info, 0, sizeof(mode_info));
    rv = _bcm_policer_svc_meter_get_mode_info(unit, mode_id, &mode_info);
    BCM_IF_ERROR_RETURN(rv);

    if (mode_info.no_of_policers == 1) {
        return (BCM_E_NONE);
    }
    if (mode_info.reference_count > 0) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Policer group mode is still in use   \n")));
        return (BCM_E_BUSY);
    } else {
        rv = _bcm_esw_policer_svc_meter_delete_mode(unit, mode_id);
    }
    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_group_mode_id_get
 * Purpose:
 *     Retrieves Customized Policer Group mode Attributes for given mode_id.
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode_id               - (IN) Created mode Id for Policer Group Mode
 *     num_selectors         - (IN) Number of Selectors for policer Group Mode
 *     flags                 - (OUT) flags
 *     type                  - (OUT) modes of policer group creation -
 *                                   normal/cascade/cascade with coupling.
 *     total_policers        - (OUT) Total Policers in Policer Group Mode
 *     attr_selectors        - (OUT) Attribute Selectors for Policer Group Mode
 *     actual_num_selectors  - (OUT) Actual Number of Selectors for Policer
 *                                   Group Mode
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_policer_group_mode_id_get(
    int unit,
    uint32 mode_id,
    uint32 num_selectors,
    uint32 *flags,
    bcm_policer_group_mode_type_t *type,
    uint32 *total_policers,
    bcm_policer_group_mode_attr_selector_t *attr_selectors,
    uint32 *actual_num_selectors)
{
    int rv = BCM_E_NONE;
    bcm_policer_svc_meter_bookkeep_mode_t   mode_info;
    udf_pkt_attr_selectors_t     *udf_attr = NULL;
    int  i, j;
    int pool_offset_limit;
    bcm_policer_svc_meter_mode_type_t mode_type_v;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    if ((mode_id == 0) || (mode_id > BCM_POLICER_SVC_METER_MAX_MODE)) {
        return (BCM_E_PARAM);
    }
    if (mode_id == 1) {
        *flags = 0;
        *type = bcmPolicerGroupModeTypeNormal;
        *actual_num_selectors = 0;
        *total_policers = 1;
        return BCM_E_NONE;
    }

    sal_memset (&mode_info, 0, sizeof(mode_info));
    rv = _bcm_policer_svc_meter_get_mode_info(unit, mode_id - 1, &mode_info);
    BCM_IF_ERROR_RETURN(rv);

    *total_policers = mode_info.no_of_policers;
    if (mode_info.group_mode == bcmPolicerGroupModeCascadeWithCoupling) {
        *total_policers = mode_info.no_of_policers / 2;
    }

    *flags =  0;
    mode_type_v = mode_info.meter_attr.mode_type_v;
    if ((mode_info.type != -1) &&
        (mode_info.attr_selectors != NULL)) {
        *type = mode_info.type;
        *actual_num_selectors = mode_info.no_of_selectors;

        if (mode_info.attr_selectors != NULL) {
            for (i = 0; i < mode_info.no_of_selectors; i++) {
                rv = _bcm_wb_attr_selectors_copy_to_attr_selectors(
                        &mode_info.attr_selectors[i],
                        &attr_selectors[i]);
                BCM_IF_ERROR_RETURN(rv);
            }
        }
        return BCM_E_NONE;
    }

    switch (mode_type_v) {
        case udf_mode:
        {
            udf_attr = &mode_info.meter_attr.udf_pkt_attr_selectors_v;
            *type = bcmPolicerGroupModeTypeNormal;
            *actual_num_selectors = mode_info.no_of_selectors;
            if (num_selectors > *actual_num_selectors) {
                 num_selectors = *actual_num_selectors;
            }
            j = 0;
            for (i = 0; i < BCM_SVC_METER_MAP_SIZE_256; i++)   {
                if (udf_attr->offset_map[i].meter_enable == 1)  {
                    attr_selectors[j].attr_value = i;
                    attr_selectors[j].policer_offset =
                            udf_attr->offset_map[i].offset;
                    attr_selectors[j].udf_id = udf_attr->udf_id;
                    attr_selectors[j].width = udf_attr->udf_subdiv[0].width;
                    attr_selectors[j].flags |= BCM_POLICER_ATTR_WIDTH_OFFSET;
                    attr_selectors[j].attr = bcmPolicerGroupModeAttrUdf;
                    attr_selectors[j].offset = udf_attr->udf_subdiv[0].offset;

                    j++;
                }
            }
            break;
        }
        case udf_cascade_mode:
        case udf_cascade_with_coupling_mode:
        {
            udf_attr = &mode_info.meter_attr.udf_pkt_attr_selectors_v;
            pool_offset_limit =
                    (mode_info.group_mode == bcmPolicerGroupModeCascade) ?
                    BCM_POLICER_GLOBAL_METER_MAX_POOL :
                    BCM_POLICER_GLOBAL_METER_MAX_POOL/2;
            *type = (mode_type_v == udf_cascade_mode) ?
                    bcmPolicerGroupModeTypeCascade :
                    bcmPolicerGroupModeTypeCascadeWithCoupling;
            *actual_num_selectors = mode_info.no_of_selectors;
            if (num_selectors > *actual_num_selectors) {
                 num_selectors = *actual_num_selectors;
            }
            j = 0;
            for (i = 0; i < BCM_SVC_METER_MAP_SIZE_256; i++)   {
                if (udf_attr->offset_map[i].meter_enable == 1)  {
                    attr_selectors[j].attr_value = i;
                    attr_selectors[j].udf_id = udf_attr->udf_id;
                    attr_selectors[j].width = udf_attr->udf_subdiv[0].width;
                    attr_selectors[j].flags |= BCM_POLICER_ATTR_WIDTH_OFFSET;
                    attr_selectors[j].attr = bcmPolicerGroupModeAttrUdf;
                    attr_selectors[j].offset = udf_attr->udf_subdiv[0].offset;

                    if (soc_feature(unit,
                            soc_feature_global_meter_pool_priority_descending)) {
                        attr_selectors[j].policer_offset = (pool_offset_limit -
                                udf_attr->offset_map[i].pool - 1);
                    } else {
                        attr_selectors[j].policer_offset =
                                udf_attr->offset_map[i].pool;
                    }
                    j++;
                }
            }
            break;
        }
        default:
            return BCM_E_UNAVAIL;
            break;
    }
    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_esw_policer_custom_group_create
 * Purpose:
 *      Create a group of policers based on the mode.
 * Parameters:
 *     unit                  - (IN) unit number
 *     flags                 - (IN) flags
 *     mode_id               - (IN) Created Mode Id for Policer Group Mode
 *     macro_flow_policer_id - (IN) Is an optional parameter and needs to be
 *                                  passed only while creating the micro flow
 *                                  policers of a hierarchical groupi
 *                                  (2 stage policers).
 *     policer_id            - (OUT)Base policer Id
 *     npolicers             - (OUT) Number of policers created
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_policer_custom_group_create(
    int unit,
    uint32 flags,
    uint32 mode_id,
    bcm_policer_t macro_flow_policer_id,
    bcm_policer_t *policer_id,
    uint32 *npolicers)
{
    bcm_policer_svc_meter_bookkeep_mode_t   mode_info;
    _global_meter_policer_control_t *policer_control = NULL;
    uint32 direction = 0;
    int rv = BCM_E_NONE;
    uint8 pid_offset[BCM_POLICER_GLOBAL_METER_MAX_POOL] = {0};
    int skip_pool = 0;
    bcm_policer_group_mode_t mode = 0;
    _bcm_policer_flow_type_t flow_type = bcmPolicerFlowTypeNormal;
    int offset_mask = 0;
    int index = 0;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    _bcm_policer_flow_info_t flow_info;
    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;
    _bcm_policer_flow_info_t_init(&flow_info);

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    if ((mode_id == 0) || (mode_id > BCM_POLICER_SVC_METER_MAX_MODE)) {
        return (BCM_E_PARAM);
    }
    mode_id -= 1;
    if (mode_id == 0) {
        *npolicers = 1;
    } else {
        rv = _bcm_policer_svc_meter_get_mode_info(unit, mode_id, &mode_info);
        BCM_IF_ERROR_RETURN(rv);

        /* Allow only when Group mode Id is created explicitly */
        if (mode_info.type == -1) {
            return BCM_E_PARAM;
        }
        *npolicers = mode_info.no_of_policers;
    }

    direction = GLOBAL_METER_ALLOC_VERTICAL;
    mode = bcmPolicerGroupModeSingle;
    if (*npolicers !=  1) {
        if (mode_info.group_mode == bcmPolicerGroupModeCascade) {
            direction = GLOBAL_METER_ALLOC_HORIZONTAL;
            mode = bcmPolicerGroupModeCascade;
        } else if (mode_info.group_mode ==
                bcmPolicerGroupModeCascadeWithCoupling) {
            direction = GLOBAL_METER_ALLOC_HORIZONTAL;
            mode = bcmPolicerGroupModeCascadeWithCoupling;
        }
    }

    if (macro_flow_policer_id > 0) {
        skip_pool = (macro_flow_policer_id & pool_mask) >> pool_offset;
    } else {
        skip_pool = num_pools;
    }
    flow_info.flow_type = flow_type;
    flow_info.skip_pool = skip_pool;
    GLOBAL_METER_LOCK(unit);
    rv = _global_meter_policer_id_alloc(unit, direction,
                            (int *)npolicers, policer_id,
                            &flow_info, &pid_offset[0]);
    if (!(BCM_SUCCESS(rv))) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Failed to allocate policer   \n")));
        return rv;
    }
    offset_mask = SOC_INFO(unit).global_meter_max_size_of_pool - 1;

    /* Allocate policer descriptor. */
    _GLOBAL_METER_XGS3_ALLOC(policer_control,
             sizeof (_global_meter_policer_control_t), "Global meter policer");

    if (NULL == policer_control) {
        _bcm_global_meter_free_allocated_policer_on_error(unit, *npolicers,
                             &pid_offset[0], (*policer_id & offset_mask));
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                      "Unable to allocate memeory for policer control   \n")));
        return (BCM_E_MEMORY);
    }

    /* Add mode info to the policer index */
    *policer_id |= (mode_id + 1) << BCM_POLICER_GLOBAL_METER_MODE_SHIFT;

    /* Set policer configuration */
    policer_control->direction = direction;
    policer_control->pid = *policer_id;
    policer_control->no_of_policers =  *npolicers;
    policer_control->grp_mode = mode;

    if (direction == GLOBAL_METER_ALLOC_HORIZONTAL) {
        index = 0;
        do {
            policer_control->offset[index] = pid_offset[index];
            index++;
        } while (index < (*npolicers));

        if (soc_feature(unit, soc_feature_global_meter_mef_10dot3)) {
            rv = _bcm_esw_global_meter_set_cascade_info_mef_10dot3_to_hw(unit,
                               *npolicers, *policer_id, mode, &pid_offset[0]);
        } else {
            rv = _bcm_esw_global_meter_set_cascade_info_to_hw(unit,
                               *npolicers, *policer_id, mode, &pid_offset[0]);
        }

        if (!(BCM_SUCCESS(rv))) {
            /* free all the allocated policers */
            _bcm_global_meter_free_allocated_policer_on_error(unit, *npolicers,
                             &pid_offset[0], (*policer_id & offset_mask));
            /* De-allocate policer descriptor. */
            sal_free(policer_control);
            GLOBAL_METER_UNLOCK(unit);
            return rv;
        }
    }

    /* increment mode reference count for new policer  */
    if (mode_id) {
        rv = bcm_policer_svc_meter_inc_mode_reference_count(unit, mode_id);
        if (!(BCM_SUCCESS(rv))) {
            /* free all the allocated policers */
            _bcm_global_meter_free_allocated_policer_on_error(unit,*npolicers,
                             &pid_offset[0], (*policer_id & offset_mask));
            /* De-allocate policer descriptor. */
            sal_free(policer_control);
            GLOBAL_METER_UNLOCK(unit);
            return rv;
        }
    }
    if (mode == bcmPolicerGroupModeCascadeWithCoupling) {
        *npolicers = *npolicers / 2;
    }
   /* Insert policer into policers hash. */
    _GLOBAL_METER_HASH_INSERT(global_meter_policer_bookkeep[unit],
                   policer_control, (*policer_id & _GLOBAL_METER_HASH_MASK));

    GLOBAL_METER_UNLOCK(unit);
    LOG_DEBUG(BSL_LS_BCM_POLICER, (BSL_META_U(unit,
                              "create policer with id %x \n"), *policer_id));
    return rv;
}

/*
 * Function:
 *     bcm_esw_policer_group_get
 * Purpose:
 *     Get the list of policer members for a given policer group represented
 *     by its base policer id
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) Base policer Id
 *     member_max            - (IN) Maximum number of policers to return in
 *                              the member_array parameter
 *     member_array          - (OUT) Place to store policer members of the
 *                              given policer group represented by its base_policer_id.
 *                              Memory need to be allocated by the end user before
 *                              calling the API
 *     member_count          - (OUT) Place to store total number of policers
 *                              configured in the given policer group represented
 *                              by base_policer_id

 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_group_get (
    int unit,
    bcm_policer_t base_policer_id,
    int member_max,
    bcm_policer_t *member_array,
    int *member_count)
{
    int rv = BCM_E_NONE;
    int index, policer_offset, indices_to_fill;
    _global_meter_policer_control_t *policer_control = NULL;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    CHECK_GLOBAL_METER_INIT(unit);

    /* verify arguments */
    if ((base_policer_id == 0) || (member_count == NULL)) {
        return BCM_E_PARAM;
    }
    if ((member_max > 0) && (member_array == NULL)) {
        return BCM_E_PARAM;
    }

    rv = _bcm_esw_policer_validate(unit, &base_policer_id);
    BCM_IF_ERROR_RETURN(rv);

    GLOBAL_METER_LOCK(unit);
    rv = _bcm_global_meter_base_policer_get(unit, base_policer_id, &policer_control);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to get policer control for the policer Id "
                              "passed  \n")));
        return (rv);
    }

    if ((policer_control->grp_mode == bcmPolicerGroupModeCascadeWithCoupling) ||
        (policer_control->grp_mode == bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
        *member_count = policer_control->no_of_policers/2;
    } else {
        *member_count = policer_control->no_of_policers;
    }

    if (member_max <= 0) {
        GLOBAL_METER_UNLOCK(unit);
        return rv;
    }

    indices_to_fill = (member_max < *member_count) ? member_max : *member_count;

    switch(policer_control->grp_mode) {
        case bcmPolicerGroupModeCascade:
        case bcmPolicerGroupModeCascadeWithCoupling:
        case bcmPolicerGroupModeIntPriCascade:
        case bcmPolicerGroupModeIntPriCascadeWithCoupling:
        {
            for (index = 0; index < indices_to_fill; index++) {
                /* policer_control->offset[] stores offset/pool from 0 to 7 */
                if (soc_feature(unit,
                        soc_feature_global_meter_pool_priority_descending)) {
                    policer_offset = *member_count - index - 1;
                } else {
                    policer_offset = index;
                }

                member_array[index] = base_policer_id +
                            (policer_control->offset[policer_offset] *
                            SOC_INFO(unit).global_meter_max_size_of_pool);
            }
            break;
        }
        default:
        {
            /* For every other modes, member policers are in same pool */
            for (index = 0; index < indices_to_fill; index++) {
                member_array[index] = base_policer_id + index;
            }
            break;
        }
    }
    GLOBAL_METER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_policer_action_attach_get
 * Purpose:
 *     Get the action id associated with a given policer.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) Base policer Id
 *     action_id             - (OUT) action id
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_action_attach_get(int unit, bcm_policer_t policer_id,
                                uint32 *action_id)
{
    int rv = BCM_E_NONE;
    svm_meter_table_entry_t meter_entry;
    _global_meter_policer_control_t *policer_control = NULL;
    int policer_index = 0;
    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    CHECK_GLOBAL_METER_INIT(unit);
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer_id);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    GLOBAL_METER_LOCK(unit);
    rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to get policer control for the policer Id "
                              "passed  \n")));
        return (rv);
    }
    /*  read meter table action_id*/
    _bcm_esw_get_policer_table_index(unit, policer_id, &policer_index);
    rv = READ_SVM_METER_TABLEm(unit, MEM_BLOCK_ANY, policer_index,
                                                            &meter_entry);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM_METER_TABLE entry \n")));
        return (rv);
    }
    /* get action id */
    soc_SVM_METER_TABLEm_field_get(unit, &meter_entry, POLICY_TABLE_INDEXf,
                                                                 action_id);
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcm_esw_policer_action_detach
 * Purpose:
 *     Disassociate a policer action from a given policer id and
 *     readjust reference count.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) Base policer Id
 *     action_id             - (IN) action id
 * Returns:
 *     BCM_E_XXX
 */
int bcm_esw_policer_action_detach(int unit, bcm_policer_t policer_id,
                                uint32 action_id)
{
    return _bcm_esw_policer_action_detach(unit, policer_id, action_id);
}

/*
 * Function:
 *      bcm_esw_policer_action_attach
 * Purpose:
 *     Associate a policer action with a given policer id and
 *     increment action usage reference count.
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer_id            - (IN) Base policer Id
 *     action_id             - (IN) action id
 * Returns:
 *     BCM_E_XXX
 */

int bcm_esw_policer_action_attach(int unit, bcm_policer_t policer_id,
                                uint32 action_id)
{
    int rv = BCM_E_NONE;
    svm_meter_table_entry_t meter_entry;
    _global_meter_policer_control_t *policer_control = NULL;
    int policer_index = 0;
    int index = 0;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }
    CHECK_GLOBAL_METER_INIT(unit);
    /* validate policer id */
    rv = _bcm_esw_policer_validate(unit, &policer_id);
    if (BCM_FAILURE(rv)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid Policer Id \n")));
        return (rv);
    }
    /* validate action id */
    if (action_id > soc_mem_index_max(unit, SVM_POLICY_TABLEm)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Invalid Action Id \n")));
        return BCM_E_PARAM;
    }

    GLOBAL_METER_LOCK(unit);
    rv = _bcm_global_meter_policer_get(unit, policer_id, &policer_control);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to get policer control for the policer Id "
                              "passed  \n")));
        return (rv);
    }

    if (policer_control->action_id == action_id) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Action Id passed is different from the one in "
                                "policer control-%x\n"), policer_control->action_id));
        return (BCM_E_NONE);
    }
    /* check if action id is allocated */
    if (global_meter_action_bookkeep[unit][action_id].used !=1)
    {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Action Id is not created \n")));
        return BCM_E_PARAM;
    }
    /*  read meter table and add action_id*/
    _bcm_esw_get_policer_table_index(unit, policer_id, &policer_index);
    rv = READ_SVM_METER_TABLEm(unit, MEM_BLOCK_ANY, policer_index,
                                                             &meter_entry);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to read SVM_METER_TABLE entry \n")));
        return (rv);
    }
    soc_SVM_METER_TABLEm_field_set(unit, &meter_entry, POLICY_TABLE_INDEXf,
                                                                 &action_id);
    /* detach the existing action and decrement reference count */
    if (policer_control->action_id) {
        _bcm_esw_policer_action_detach(unit, policer_id,
                                                policer_control->action_id);
    }
    /* Write to HW*/
    rv = WRITE_SVM_METER_TABLEm(unit, MEM_BLOCK_ANY, policer_index,
                                                                 &meter_entry);
    if (BCM_FAILURE(rv)) {
        GLOBAL_METER_UNLOCK(unit);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to write SVM_METER_TABLE entry \n")));
        return (rv);
    }
    /* in case of cascade with coupling we need to configure the second set
       as well */
    if ((policer_control->grp_mode == \
         bcmPolicerGroupModeIntPriCascadeWithCoupling) ||
        (policer_control->grp_mode == bcmPolicerGroupModeCascadeWithCoupling)) {
        rv =_bcm_global_meter_get_coupled_cascade_policer_index(unit,
                                     policer_id, policer_control, &index);
        rv = READ_SVM_METER_TABLEm(unit, MEM_BLOCK_ANY, index,
                                   &meter_entry);
        if (BCM_FAILURE(rv)) {
            GLOBAL_METER_UNLOCK(unit);
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to read SVM_METER_TABLE entry \n")));
            return (rv);
        }
        soc_SVM_METER_TABLEm_field_set(unit, &meter_entry, POLICY_TABLE_INDEXf,
                                       &action_id);
        /* write to hW */
        rv = soc_mem_write(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY, index,
                                                            &meter_entry);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "Unable to write SVM_METER_TABLE entry \n")));
            GLOBAL_METER_UNLOCK(unit);
        return (rv);
       }
    }
   /* increment action usage reference count */
    global_meter_action_bookkeep[unit][action_id].reference_count++;
    policer_control->action_id = action_id;
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      bcmi_global_meter_dump_config
 * Purpose:
 *      dump the config of Global Meter
 * Parameters:
 *      unit - (IN) Unit number
 *      prefix - (IN) Prefix string for dump message
 *      config - (IN) Global Meter Config
 * Returns:
 *      BCM_E_xxx
 */
STATIC void
bcmi_global_meter_dump_config(
    int unit,
    char *prefix,
    bcm_policer_global_meter_config_t *config)
{
    int i;

    if ((NULL == prefix) || (NULL == config)) {
        return;
    }

    if (bsl_check(bslLayerBcm, bslSourcePolicer, bslSeverityDebug, unit)) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "bcmi_global_meter_dump_config:\n")));
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "%s map_type %d.\n"),
                              prefix,
                              config->ifg_enable
                              ));

        for (i = 0; i < bcmPolicerGlobalMeterSourceCount; i++) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                      (BSL_META_U(unit,
                                  "%s source_order[%d]: %d.\n"),
                                  prefix,
                                  i,
                                  config->source_order[i]
                                  ));
        }
    }
}

/*
 * Function:
 *      bcm_esw_policer_global_meter_config_set
 * Purpose:
 *      Set the Global Meter Configuration
 * Parameters:
 *      unit - (IN) Unit number
 *      config - (IN) Global Meter Configuration
 * Returns:
 *      BCM_E_xxx
 */
int bcm_esw_policer_global_meter_config_set(
    int unit,
    bcm_policer_global_meter_config_t *config)
{
    int rv = BCM_E_UNAVAIL;
    const bcmi_global_meter_dev_info_t *gm_dev_info = NULL;

    /* Initialization check */
    CHECK_GLOBAL_METER_INIT(unit);
    CHECK_GLOBAL_METER_PF_INIT(unit);

    /* Parameter NULL error handing */
    if (NULL == config) {
        return BCM_E_PARAM;
    }
    bcmi_global_meter_dump_config(unit, "config set", config);

    GLOBAL_METER_LOCK(unit);
    gm_dev_info = bcmi_global_meter_dev_info[unit];
    rv = gm_dev_info->ifg_set(unit, config->ifg_enable);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "HW Operation failed(rv = %d)\n"),
                              rv));
        GLOBAL_METER_UNLOCK(unit);
        return rv;
    }

    rv = gm_dev_info->source_order_set(unit,
                                       config->source_order,
                                       config->source_order_count);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "HW Operation failed(rv = %d)\n"),
                              rv));
        GLOBAL_METER_UNLOCK(unit);
        return rv;
    }
    GLOBAL_METER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_policer_global_meter_config_get
 * Purpose:
 *      Get the Global Meter Configuration
 * Parameters:
 *      unit - (IN) Unit number
 *      config - (OUT) Global Meter Configuration
 * Returns:
 *      BCM_E_xxx
 */
int bcm_esw_policer_global_meter_config_get(
    int unit,
    bcm_policer_global_meter_config_t *config)
{
    int rv = BCM_E_UNAVAIL;
    const bcmi_global_meter_dev_info_t *gm_dev_info = NULL;

    /* Initialization check */
    CHECK_GLOBAL_METER_INIT(unit);
    CHECK_GLOBAL_METER_PF_INIT(unit);

    /* Parameter NULL error handing */
    if (NULL == config) {
        return BCM_E_PARAM;
    }

    GLOBAL_METER_LOCK(unit);
    gm_dev_info = bcmi_global_meter_dev_info[unit];

    rv = gm_dev_info->ifg_get(unit, &config->ifg_enable);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "HW Operation failed(rv = %d)\n"),
                              rv));
        GLOBAL_METER_UNLOCK(unit);
        return rv;
    }

    rv = gm_dev_info->source_order_get(unit,
                                       config->source_order,
                                       config->source_order_count);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "HW Operation failed(rv = %d)\n"),
                              rv));
        GLOBAL_METER_UNLOCK(unit);
        return rv;
    }

    bcmi_global_meter_dump_config(unit, "config get", config);
    GLOBAL_METER_UNLOCK(unit);

    return BCM_E_NONE;
}
/* END POLICER API's */

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_esw_policer_config_from_meter_table
 * Purpose:
 *     For a given policer, read the configuraton info from meter table
 * Parameters:
 *     unit                  - (IN) unit number
 *     policer               - (IN) Base policer Id
 *     policer_control       - (OUT) Policer control info
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_policer_config_from_meter_table(int unit, bcm_policer_t policer,
                           _global_meter_policer_control_t *policer_control)
{
    svm_meter_table_entry_t data;
    int index = 0, numbers = 0;
    uint32 mode = 0, coupling_flag = 0, pool = 0;
    bcm_policer_svc_meter_mode_t meter_mode;
    uint32 pol_index = 0, offset_mode = 0;
    uint32 end_of_chain = 0, first_end_of_chain = 0;
    int rv = BCM_E_NONE;
    int policer_index = 0;
    int size_pool = 0, num_pools = 0;
    int pool_mask = 0, pool_offset = 0;
    int offset_mask = 0;
    int multi_chain = 0;
    uint32 action_id = 0;

    size_pool =  SOC_INFO(unit).global_meter_max_size_of_pool;
    num_pools =  SOC_INFO(unit).global_meter_pools;
    offset_mask = size_pool - 1;
    pool_offset = _shr_popcount(offset_mask);
    pool_mask = (num_pools - 1) << pool_offset;

    offset_mode = ((policer & BCM_POLICER_GLOBAL_METER_MODE_MASK) >>
                   BCM_POLICER_GLOBAL_METER_MODE_SHIFT);
    if (offset_mode >= 1) {
        offset_mode = offset_mode - 1;
        if ((offset_mode) &&
           (global_meter_offset_mode[unit][offset_mode].no_of_policers == 0)) {
            return rv;
        }
    }
    if ((global_meter_offset_mode[unit][offset_mode].meter_attr.mode_type_v ==
        cascade_mode) ||
        (global_meter_offset_mode[unit][offset_mode].type ==
                bcmPolicerGroupModeTypeCascade) ||
        (global_meter_offset_mode[unit][offset_mode].type ==
                bcmPolicerGroupModeTypeCascadeWithCoupling)) {
        /*
         * Get the base policer ID for cascaded mode, then SDK will
         * recover policer_control stuct according to the base policer ID
         */
        policer = (policer & BCM_POLICER_GLOBAL_METER_MODE_MASK) |
                  (policer & offset_mask);
        multi_chain = 1;
    }
   /* Lets first try to get policer_control */
    if (multi_chain == 1) {
        rv = _bcm_global_meter_policer_get(unit, policer, &policer_control);
    } else {
        rv = _bcm_global_meter_base_policer_get(unit, policer, &policer_control);
    }
    if (BCM_SUCCESS(rv)) {
        return rv;
    }
   /* read meter table to get policer configuration */
    _bcm_esw_get_policer_table_index(unit, policer, &policer_index);
    rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                                    policer_index, &data);
    BCM_IF_ERROR_RETURN(rv);
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, METER_MODEf)) {
        soc_SVM_METER_TABLEm_field_get(unit, &data, METER_MODEf, &mode);
    }
    if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, COUPLING_FLAGf)) {
       soc_SVM_METER_TABLEm_field_get(unit, &data, COUPLING_FLAGf,
                                                         &coupling_flag);
   }
   pool = (policer & pool_mask) >> pool_offset;
   /* Allocate policer descriptor. */
    _GLOBAL_METER_XGS3_ALLOC(policer_control,
       sizeof (_global_meter_policer_control_t), "Global meter policer");
   if (NULL == policer_control) {
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to allocate memory for Policer control data "
                              "structure\n")));
       return (BCM_E_MEMORY);
   }

   if (SOC_MEM_FIELD_VALID(unit, SVM_METER_TABLEm, POLICY_TABLE_INDEXf)) {
       soc_SVM_METER_TABLEm_field_get(unit,
                      &data, POLICY_TABLE_INDEXf, &action_id);
        /* increment action usage reference count */
       if (action_id) {
            policer_control->action_id = action_id;
            global_meter_action_bookkeep[unit]\
                       [policer_control->action_id].reference_count++;
       }
   }

    /* Set the grp_mode for policer control. In case offset_mode=0,
     * global_meter_offset_mode[0][0].group_mode will have by
     * default 0 (bcmPolicerGroupModeSingle) */
    policer_control->grp_mode =
            global_meter_offset_mode[unit][offset_mode].group_mode;

    if((soc_feature(unit, soc_feature_global_meter_mef_10dot3)) &&
            (mode == GLOBAL_METER_MODE_CASCADE)) {
        /* Handling recovery of Cascade/Coupledcascade Mode in Apache
         * octuple is allocated for cascade/coupledcascade group mode */
        policer_control->direction = GLOBAL_METER_ALLOC_HORIZONTAL;
        policer_control->pid = policer;
        policer_control->offset[0] = pool;
        if ((policer_control->grp_mode ==
                bcmPolicerGroupModeCascadeWithCoupling) ||
            (policer_control->grp_mode ==
                bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
            end_of_chain = 4;
        }
        pol_index = policer_index;
        for (index = pool + 1; index < num_pools; index++) {
            policer_control->offset[index] = index;

            if (index == end_of_chain) {
                first_end_of_chain = 1;
            }
            /* increment action usage reference count */
            /* In case of cascade coupled, use only one chain */
            if (first_end_of_chain == 0) {
                /* Read index to get policy_index, if any */
                policer_index = pol_index + (index << _shr_popcount(size_pool - 1));
                rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                    policer_index, &data);
                if (!BCM_SUCCESS(rv)) {
                    sal_free(policer_control);
                    LOG_DEBUG(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "Unable to read SVM meter table at "
                                    "index %x\n"), policer_index));
                    return rv;
                }
                action_id = 0;
                soc_SVM_METER_TABLEm_field_get(unit, &data,
                                POLICY_TABLE_INDEXf, &action_id);
                if (action_id != 0) {
                    global_meter_action_bookkeep[unit]\
                            [action_id].reference_count++;
                }
            }
        }
        policer_control->no_of_policers = index;
    } else if (!soc_feature(unit, soc_feature_global_meter_mef_10dot3) &&
            (mode == GLOBAL_METER_MODE_CASCADE)) {
       policer_control->direction = GLOBAL_METER_ALLOC_HORIZONTAL;
       policer_control->pid = policer;
       policer_control->offset[numbers] = 0;
       numbers++;
       pol_index = policer_index;
       for (index = pool + 1; index < num_pools; index++) {
           policer_index = pol_index + (index << _shr_popcount(size_pool - 1));
           rv = soc_mem_read(unit, SVM_METER_TABLEm, MEM_BLOCK_ANY,
                                                   policer_index, &data);
           if (!BCM_SUCCESS(rv)) {
               sal_free(policer_control);
               LOG_DEBUG(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Unable to read SVM meter table at "
                                     "index %x\n"), policer_index));
               return rv;
           }
           /* increment action usage reference count */
           /* In case of cascade coupled, use only one chain */
           if (first_end_of_chain == 0) {
                action_id = 0;
                soc_SVM_METER_TABLEm_field_get(unit, &data,
                            POLICY_TABLE_INDEXf, &action_id);
                if (action_id != 0) {
                    global_meter_action_bookkeep[unit]\
                            [action_id].reference_count++;
                }
           }
           soc_SVM_METER_TABLEm_field_get(unit, &data,
                                        END_OF_CHAINf, &end_of_chain);
           soc_SVM_METER_TABLEm_field_get(unit, &data,
                                              METER_MODEf, &meter_mode);
           if ((meter_mode == GLOBAL_METER_MODE_CASCADE) &&
                                                   (end_of_chain != 1)) {
               policer_control->offset[numbers] = index - pool;
           }
           if (!(coupling_flag) &&
                  (meter_mode == GLOBAL_METER_MODE_CASCADE) &&
                  (end_of_chain == 1)) {
               policer_control->offset[numbers] = index - pool;
               if (!multi_chain) {
                   /*
                    * In cascaded mode there may be multi chains in one policer
                    * group, and each policer group contains 8 policer IDs. SDK
                    * should recover all these 8 policer IDs.
                    */
                   index = num_pools;
               }
           }
           if ((coupling_flag) &&
                           (meter_mode == GLOBAL_METER_MODE_CASCADE) &&
                           (end_of_chain) && (first_end_of_chain)) {
               policer_control->offset[numbers] = index - pool;
               index = num_pools;
           }
           if ((coupling_flag) &&
                       (meter_mode == GLOBAL_METER_MODE_CASCADE) &&
                       (end_of_chain) && (first_end_of_chain == 0)) {
               policer_control->offset[numbers] = index - pool;
               first_end_of_chain = 1;
           }
           numbers++;
       } /* end of for */
       policer_control->no_of_policers = numbers;
    } else if (offset_mode >= 1) {
       policer_control->direction = GLOBAL_METER_ALLOC_VERTICAL;
       policer_control->pid = policer;
       /* add code to get the number of policers for this mode */
       policer_control->no_of_policers =
       global_meter_offset_mode[unit][offset_mode].no_of_policers;
    } else {
       policer_control->direction = GLOBAL_METER_ALLOC_VERTICAL;
       policer_control->pid = policer;
       policer_control->no_of_policers = 1;
    }
    rv =  _global_meter_reserve_policer_id(unit, policer_control->direction,
                                            policer_control->no_of_policers,
                                        policer, &policer_control->offset[0]);
    if (!BCM_SUCCESS(rv)) {
         /* De-allocate policer descriptor. */
        sal_free(policer_control);
        LOG_DEBUG(BSL_LS_BCM_POLICER,
                  (BSL_META_U(unit,
                              "Unable to allocate policers with base id %x\n"),
                   policer));
        return BCM_E_INTERNAL;
    }
    /* Insert policer into policers hash. */
    _GLOBAL_METER_HASH_INSERT(global_meter_policer_bookkeep[unit],
                 policer_control,(policer & _GLOBAL_METER_HASH_MASK));
    /* increment mode reference count for new policer  */
    if (offset_mode) {
        rv = bcm_policer_svc_meter_inc_mode_reference_count(unit, offset_mode);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_policer_config_reinit_from_table
 * Purpose:
 *     Read entry from table and re-init policer if present
 * Parameters:
 *     unit                  - (IN) unit number
 *     mem                   - (IN) table
 *     index                 - (IN) Index in table for entry
 *     entry                 - (IN) one entry in table
 *     user_data             - (IN) data passed by calling function
 *     dirty                 - (OUT) Set if entry is modified
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_policer_config_reinit_from_table(int unit, soc_mem_t mem, uint32 index,
                                        void *entry, void *user_data, uint32 *dirty)
{
    int rv = BCM_E_NONE;
    bcm_policer_t policer;
    uint32 offset_mode = 0, meter_index = 0;
    _bcm_policer_svm_source_type_t mem_index;
    _global_meter_policer_control_t *policer_control = NULL;

    if (entry == NULL) {
        return BCM_E_INTERNAL;
    }
    _bcm_policer_svm_source_index_get(unit, mem, &mem_index);
    if (mem == SVM_MACROFLOW_INDEX_TABLEm) {
        offset_mode = meter_index = 0;
        if (SOC_MEM_FIELD_VALID(unit, mem, MACROFLOW_INDEXf)) {
            meter_index = soc_mem_field32_get(unit, mem, entry,
                    MACROFLOW_INDEXf);
        }
    } else {
        offset_mode = soc_mem_field32_get(unit, mem, entry,
                BCM_SVM_SOURCE(unit, mem_index).offset_mode);
        meter_index = soc_mem_field32_get(unit, mem, entry,
                BCM_SVM_SOURCE(unit, mem_index).meter_index);
    }
    _bcm_esw_get_policer_id_from_index_offset(unit, meter_index,
            offset_mode, &policer);
    if (policer != 0) {
        policer_control = NULL;
        rv = _bcm_esw_policer_config_from_meter_table(unit, policer,
                policer_control);
        if (BCM_SUCCESS(rv)) {
            rv = _bcm_esw_policer_increment_ref_count(unit, policer);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_svm_action_reinit_from_table
 * Purpose:
 *       Called via a svm table traversal function to reinit action if present
 * Parameters:
 *     unit                  - (IN) unit number
 *     mem                   - (IN) memory
 *     index                 - (IN) Index in Memory
 *     entry                 - (IN) Buffer of mem entry at index in Memory
 *     user_data             - (IN) user specific info
 *     dirty                 - (OUT) Is entry modified
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_esw_svm_action_reinit_from_table(int unit, soc_mem_t mem, uint32 index,
        void *entry, void *user_data, uint32 *dirty)
{
    int rv = BCM_E_NONE;
    uint32 green_action[1] = {0}, yellow_action[1] = {0}, red_action[1] = {0};

    soc_SVM_POLICY_TABLEm_field_get(unit, entry, G_ACTIONSf, &green_action[0]);
    soc_SVM_POLICY_TABLEm_field_get(unit, entry, Y_ACTIONSf, &yellow_action[0]);
    soc_SVM_POLICY_TABLEm_field_get(unit, entry, R_ACTIONSf, &red_action[0]);

    if ((green_action[0] > 0) || (yellow_action[0] > 0) ||
            (red_action[0] > 0) ||
            (global_meter_action_bookkeep[unit][index].reference_count != 0))
    {
        global_meter_action_bookkeep[unit][index].used = 1;
        /* Reserve the the action table entry */
        rv = shr_aidxres_list_reserve_block(
                meter_action_list_handle[unit], index, 1);
        if (BCM_FAILURE(rv)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to reserve action id %d in "
                                "index management list\n"), index));
            return (rv);
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_policer_config_reinit
 * Purpose:
 *     go through all the table entries that support service meter
 *     and get policer configuration for those policers that are
 *     configured in HW to rebuild SDK internal data strutcure on
 *     warm boot.
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_policer_config_reinit(int unit)
{
    int rv = BCM_E_NONE;
    soc_mem_t table;
    _bcm_policer_svm_source_type_t mem_index;

    for (mem_index = 0; mem_index < _BCM_SVM_MEM_COUNT; mem_index++) {
        table = BCM_SVM_SOURCE(unit, mem_index).table;
        if (table == INVALIDm) {
            continue;
        }
        rv = _bcm_esw_svm_source_traverse (unit, table, -1, -1,
                _bcm_esw_policer_config_reinit_from_table, NULL);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (BCM_SUCCESS(rv)) {
        table = SVM_MACROFLOW_INDEX_TABLEm;
        rv = _bcm_esw_svm_source_traverse (unit, table, -1, -1,
                _bcm_esw_policer_config_reinit_from_table, NULL);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_uncompressed_offset_mode_reinit
 * Purpose:
 *      Recover un-coompessed offset mode information from HW and re-configure
 *      SDK data structures on warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) meter offset mode
 *     selector_count        - (IN) Number of selectors
 *     selector_for_bit_x_en_field_value - (IN) selector field enable
 *     selector_for_bit_x_field_value    - (IN) selector field
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_uncompressed_offset_mode_reinit(
                                 int unit,
                                 int mode,
                                 uint32 selector_count,
                                 uint32  selector_for_bit_x_en_field_value[8],
                                 uint32  selector_for_bit_x_field_value[8])
{
    uint32  uncompressed_attr_bits_selector = 0;
    uint32  index = 0;
    int rv = BCM_E_NONE;
    _bcm_policer_pkt_attr_bit_pos_t *pkt_attr_bit_pos = NULL;
    int                 port_attr_size = BCM_POLICER_SVC_METER_ING_PORT_ATTR_SIZE;

    pkt_attr_bit_pos = BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos;

    global_meter_offset_mode[unit][mode].used = 1;
    global_meter_offset_mode[unit][mode].reference_count = 0;
    global_meter_offset_mode[unit][mode].meter_attr.mode_type_v =
                                                  uncompressed_mode;
    for (uncompressed_attr_bits_selector = 0, index = 0; index < 8; index++)
    {
        if (selector_for_bit_x_en_field_value[index] != 0)
        {
            if ( selector_for_bit_x_field_value[index] ==
                               pkt_attr_bit_pos[pkt_attr_ip_pkt].start_bit)
            {
                 uncompressed_attr_bits_selector |=
                       BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_IP_PKT_ATTR_BITS;
                 continue;
            }
            if ( selector_for_bit_x_field_value[index] ==
                               pkt_attr_bit_pos[pkt_attr_drop].start_bit)
            {
                 uncompressed_attr_bits_selector |=
                        BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_DROP_ATTR_BITS;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] ==
                        pkt_attr_bit_pos[pkt_attr_svp_group].start_bit)  &&
               ( selector_for_bit_x_field_value[index] <=
                        pkt_attr_bit_pos[pkt_attr_svp_group].end_bit))
            {
                 uncompressed_attr_bits_selector |=
                    BCM_POLICER_SVC_METER_UNCOMPRESSED_USE_SVP_TYPE_ATTR_BITS;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                        pkt_attr_bit_pos[pkt_attr_pkt_resolution].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                        pkt_attr_bit_pos[pkt_attr_pkt_resolution].end_bit))

            {
                 uncompressed_attr_bits_selector |=
                               BCM_POLICER_SVC_METER_PKT_REOLUTION_ATTR_SIZE;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_tos].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_tos].end_bit))
            {
                 uncompressed_attr_bits_selector |=
                                        BCM_POLICER_SVC_METER_TOS_ATTR_SIZE;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                             pkt_attr_bit_pos[pkt_attr_ing_port].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                             pkt_attr_bit_pos[pkt_attr_ing_port].end_bit))
            {
                port_attr_size =
                        BCM_SVM_DEV_ATTR(unit)->uncompressed_ing_port_size;
                uncompressed_attr_bits_selector |= port_attr_size;
                continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                          pkt_attr_bit_pos[pkt_attr_inner_dot1p].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                          pkt_attr_bit_pos[pkt_attr_inner_dot1p].end_bit))
            {
                uncompressed_attr_bits_selector |=
                                BCM_POLICER_SVC_METER_INNER_DOT1P_ATTR_SIZE;
                continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                           pkt_attr_bit_pos[pkt_attr_outer_dot1p].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                           pkt_attr_bit_pos[pkt_attr_outer_dot1p].end_bit))
            {
                uncompressed_attr_bits_selector |=
                                BCM_POLICER_SVC_METER_OUTER_DOT1P_ATTR_SIZE;
                continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                           pkt_attr_bit_pos[pkt_attr_vlan_format].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                           pkt_attr_bit_pos[pkt_attr_vlan_format].end_bit))
            {
                uncompressed_attr_bits_selector |=
                                BCM_POLICER_SVC_METER_VLAN_FORMAT_ATTR_SIZE;
                continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_int_pri].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_int_pri].end_bit))
            {
                uncompressed_attr_bits_selector |=
                                   BCM_POLICER_SVC_METER_INT_PRI_ATTR_SIZE;
                continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_cng].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_cng].end_bit))
            {
                uncompressed_attr_bits_selector |=
                                          BCM_POLICER_SVC_METER_CNG_ATTR_SIZE;
                continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_short_int_pri].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_short_int_pri].end_bit))
            {
                uncompressed_attr_bits_selector |=
                                   BCM_POLICER_SVC_METER_SHORT_INT_PRI_ATTR_SIZE;
                continue;
            }

        }
    }
    global_meter_offset_mode[unit][mode].meter_attr.\
          uncompressed_attr_selectors_v.uncompressed_attr_bits_selector =
                                              uncompressed_attr_bits_selector;
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_compressed_offset_mode_reinit
 * Purpose:
 *      Recover Compessed offset mode information from HW and re-configure SDK
 *      data structures on warm boot
 *
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) meter offset mode
 *     selector_count        - (IN) Number of selectors
 *     selector_for_bit_x_en_field_value - (IN) selector field enable
 *     selector_for_bit_x_field_value    - (IN) selector field
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_compressed_offset_mode_reinit(
                                 int unit,
                                 int mode,
                                 uint32 selector_count,
                                 uint32  selector_for_bit_x_en_field_value[8],
                                 uint32  selector_for_bit_x_field_value[8])
{
    uint32  index = 0;
    int rv = BCM_E_NONE;
    compressed_attr_selectors_t *comp_pkt_attr_sel;
    pkt_attr_bits_t *pkt_attr_bits;
    _bcm_policer_pkt_attr_bit_pos_t *pkt_attr_bit_pos = NULL;
    int                     index_max = 0;

    pkt_attr_bit_pos =
            BCM_SVM_DEV_ATTR(unit)->uncompressed_pkt_attr_bit_pos;

    global_meter_offset_mode[unit][mode].used = 1;
    global_meter_offset_mode[unit][mode].reference_count = 0;
    global_meter_offset_mode[unit][mode].meter_attr.mode_type_v =
                                                     compressed_mode;
    pkt_attr_bits = &(global_meter_offset_mode[unit][mode].meter_attr.\
                       compressed_attr_selectors_v.pkt_attr_bits_v);
    comp_pkt_attr_sel = &(global_meter_offset_mode[unit][mode].meter_attr.\
                                               compressed_attr_selectors_v);
    for (index = 0; index < 8; index++)
    {
        if (selector_for_bit_x_en_field_value[index] != 0)
        {
            if ( selector_for_bit_x_field_value[index] ==
                               pkt_attr_bit_pos[pkt_attr_ip_pkt].start_bit)
            {
                 pkt_attr_bits->ip_pkt = 1;
                 continue;
            }
            if ( selector_for_bit_x_field_value[index] ==
                               pkt_attr_bit_pos[pkt_attr_drop].start_bit)
            {
                 pkt_attr_bits->drop = 1;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                        pkt_attr_bit_pos[pkt_attr_svp_group].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                        pkt_attr_bit_pos[pkt_attr_svp_group].end_bit))
            {
                 pkt_attr_bits->svp_type++ ;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                        pkt_attr_bit_pos[pkt_attr_pkt_resolution].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                        pkt_attr_bit_pos[pkt_attr_pkt_resolution].end_bit))

            {
                 pkt_attr_bits->pkt_resolution++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_tos].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_tos].end_bit))
            {
                 pkt_attr_bits->tos++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                             pkt_attr_bit_pos[pkt_attr_ing_port].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                             pkt_attr_bit_pos[pkt_attr_ing_port].end_bit))
            {
                 pkt_attr_bits->ing_port++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                          pkt_attr_bit_pos[pkt_attr_inner_dot1p].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                          pkt_attr_bit_pos[pkt_attr_inner_dot1p].end_bit))
            {
                 pkt_attr_bits->inner_dot1p++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                           pkt_attr_bit_pos[pkt_attr_outer_dot1p].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                           pkt_attr_bit_pos[pkt_attr_outer_dot1p].end_bit))
            {
                 pkt_attr_bits->outer_dot1p++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                           pkt_attr_bit_pos[pkt_attr_vlan_format].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                           pkt_attr_bit_pos[pkt_attr_vlan_format].end_bit))
            {
                 pkt_attr_bits->vlan_format++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_int_pri].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_int_pri].end_bit))
            {
                 pkt_attr_bits->int_pri++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_cng].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_cng].end_bit))
            {
                 pkt_attr_bits->cng++;
                 continue;
            }
            if (( selector_for_bit_x_field_value[index] >=
                               pkt_attr_bit_pos[pkt_attr_short_int_pri].start_bit) &&
               ( selector_for_bit_x_field_value[index] <=
                               pkt_attr_bit_pos[pkt_attr_short_int_pri].end_bit))
            {
                 pkt_attr_bits->short_int_pri++;
                 continue;
            }

        }
    }

    if ((pkt_attr_bits->cng) || (pkt_attr_bits->int_pri))
    {
        index_max = soc_mem_index_max(unit, ING_SVM_PRI_CNG_MAPm);
        for (index = 0; index <= index_max; index++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(
                unit,
                ING_SVM_PRI_CNG_MAPm,
                MEM_BLOCK_ANY,
                index,
                &comp_pkt_attr_sel->compressed_pri_cnf_attr_map_v[index]));
        }
    }
    if ((pkt_attr_bits->vlan_format) || (pkt_attr_bits->outer_dot1p) ||
                                               (pkt_attr_bits->inner_dot1p))
    {
        index_max = soc_mem_index_max(unit, ING_SVM_PKT_PRI_MAPm);
        for (index = 0; index < BCM_SVC_METER_MAP_SIZE_256; index++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(
                unit,
                ING_SVM_PKT_PRI_MAPm,
                MEM_BLOCK_ANY,
                index,
                &comp_pkt_attr_sel->compressed_pkt_pri_attr_map_v[index]));
        }
    }

    if ((pkt_attr_bits->ing_port))
    {
        index_max = soc_mem_index_max(unit, ING_SVM_PORT_MAPm);
        for (index = 0; index <= index_max; index++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(
                unit,
                ING_SVM_PORT_MAPm,
                MEM_BLOCK_ANY,
                index,
                &comp_pkt_attr_sel->compressed_port_attr_map_v[index]));
        }
    }

    if (pkt_attr_bits->tos)
    {
        index_max = soc_mem_index_max(unit, ING_SVM_TOS_MAPm);
        for (index = 0; index < BCM_SVC_METER_MAP_SIZE_256; index++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(
                unit,
                ING_SVM_TOS_MAPm,
                MEM_BLOCK_ANY,
                index,
                &comp_pkt_attr_sel->compressed_tos_attr_map_v[index]));
        }
    }

    if ((pkt_attr_bits->pkt_resolution) || (pkt_attr_bits->svp_type) ||
                                                (pkt_attr_bits->drop))
    {
        index_max = soc_mem_index_max(unit, ING_SVM_PKT_RES_MAPm);
        for (index = 0; index < BCM_SVC_METER_MAP_SIZE_256; index++) {
            SOC_IF_ERROR_RETURN(soc_mem_read(
                unit,
                ING_SVM_PKT_RES_MAPm,
                MEM_BLOCK_ANY,
                index,
                &comp_pkt_attr_sel->compressed_pkt_res_attr_map_v[index]));
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_udf_offset_mode_reinit
 * Purpose:
 *      Recover UDF offset mode information from HW and re-configure SDK
 *      data structures on warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) meter offset mode
 *     selector_count        - (IN) Number of selectors
 *     selector_for_bit_x_en_field_value - (IN) selector field enable
 *     selector_for_bit_x_field_value    - (IN) selector field
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_udf_offset_mode_reinit(
                                 int unit,
                                 int mode,
                                 uint32 selector_count,
                                 uint32  selector_for_bit_x_en_field_value[8],
                                 uint32  selector_for_bit_x_field_value[8])
{
    int rv = BCM_E_NONE;
    uint16  udf0 = 0;
    uint16  udf1 = 0;
    uint32  index = 0;
    udf_pkt_attr_selectors_t *udf_attr = NULL;
    svm_offset_table_entry_t   *buf;
    svm_offset_table_entry_t   *entry;
    int     entry_index = 0;
    uint32 offset, pool_offset, meter_enable;

    udf_attr =
      &global_meter_offset_mode[unit][mode].meter_attr.udf_pkt_attr_selectors_v;
    if (selector_for_bit_x_field_value[0] == 0) {
        udf_attr->drop = 1;
        index++;
    }
    for (udf0 = 0, udf1 = 0; index < 8; index++) {
        if (selector_for_bit_x_field_value[index] != 0) {
            udf_attr->udf_subdiv[0].width += 1;
            if ( selector_for_bit_x_field_value[index] <= 16) {
                 udf0 |= (1 << (selector_for_bit_x_field_value[index] - 1));
            } else {
                 udf1 |= (1 << (selector_for_bit_x_field_value[index]- 16 - 1));
            }
        }
    }

    global_meter_offset_mode[unit][mode].used = 1;
    global_meter_offset_mode[unit][mode].reference_count = 0;
    global_meter_offset_mode[unit][mode].meter_attr.mode_type_v = udf_mode;
    global_meter_offset_mode[unit][mode].meter_attr.udf_pkt_attr_selectors_v.\
                                               udf_pkt_attr_bits_v.udf0 = udf0;
    global_meter_offset_mode[unit][mode].meter_attr.udf_pkt_attr_selectors_v.\
                                               udf_pkt_attr_bits_v.udf1 = udf1;

    udf_attr->total_subdiv = 1;
    offset = (udf_attr->drop == 1) ?
            selector_for_bit_x_field_value[1] : selector_for_bit_x_field_value[0];
    /* Since udf0 bits start from bit-1 in the selector bit vector */
    udf_attr->udf_subdiv[0].offset = offset - 1;

    /* Allocate buffer to store the DMAed table entries. */
    buf = soc_cm_salloc(unit, sizeof(svm_offset_table_entry_t) * 256,
                              "svm macro flow index table entry buffer");
    if (NULL == buf) {
        return (BCM_E_MEMORY);
    }
    /* Initialize the entry buffer. */
    sal_memset(buf, 0, sizeof(svm_offset_table_entry_t) * 256);

    /* Read the table entries into the buffer. */
    rv = soc_mem_read_range(unit, SVM_OFFSET_TABLEm,
                                MEM_BLOCK_ALL,
                    ((mode << BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE)),
                    ((mode << BCM_POLICER_SVC_METER_OFFSET_TABLE_KEY_SIZE) + 256),
                    buf);
    if (BCM_FAILURE(rv)) {
        if (buf) {
             soc_cm_sfree(unit, buf);
        }
        return rv;
    }

    /* Iterate over the table entries. */
    for (entry_index = 0; entry_index < 256; entry_index++) {
        entry = soc_mem_table_idx_to_pointer(unit,
                              SVM_OFFSET_TABLEm,
                              svm_offset_table_entry_t *,
                              buf, entry_index);
        soc_SVM_OFFSET_TABLEm_field_get (unit, (void *)entry, OFFSETf, &offset);
        soc_SVM_OFFSET_TABLEm_field_get (unit, (void *)entry, POOL_OFFSETf, &pool_offset);
        soc_SVM_OFFSET_TABLEm_field_get (unit, (void *)entry, METER_ENABLEf, &meter_enable);

        udf_attr->offset_map[entry_index].offset = offset;
        udf_attr->offset_map[entry_index].pool = pool_offset;
        udf_attr->offset_map[entry_index].meter_enable = meter_enable;

        if(udf_attr->offset_map[entry_index].meter_enable == 1) {
            udf_attr->num_selectors += 1;
        }
    }
    if (buf) {
        soc_cm_sfree(unit, buf);
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_policer_offset_mode_update
 * Purpose:
 *      Update global_meter_offset_mode information based on values retrieve
 *      from scache on warmboot
 * Parameters:
 *     unit                  - (IN) unit number
 *     mode                  - (IN) meter offset mode
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_policer_offset_mode_update(int unit, uint32 mode)
{
    int rv;
    int i;
    uint32 offset;
    uint32 width;
    int udf_id;
    uint32 num_selectors =0;
    uint32 total_subdiv=0, sel, index=0, index1=0, sorted;
    udf_pkt_attr_selectors_t *udf_pkt_attr = NULL;
    bcm_policer_group_mode_attr_selector_t *attr_selectors = NULL;
    bcm_policer_group_mode_attr_selector_wb_t *attr_selectors_wb  = NULL;
    uint8 udf_set_bits[SVC_METER_UDF1_MAX_BIT_POSITION] = {0};
    bcm_policer_udf_sub_div_t dummy_data;
    bcm_policer_udf_sub_div_t udf_subdiv[SVC_METER_UDF_MAX_SUB_DIVISIONS] = {{0,0,0}};
    compressed_attr_selectors_t *cmprsd_attr_selectors = NULL;
    bcm_policer_attr_selectors_t pkt_attr_selectors;
    uint32 attr_count = 0;
    uint32 total_policers = 0;
    uint32 npolicers = 0;
    uint32 port_attr_count = 0;
    uint32 tos_dscp_attr_count = 0;
    uint32 tos_ecn_attr_count = 0;
    uint32 svp_type_attr_count = 0;

    num_selectors = global_meter_offset_mode[unit][mode].no_of_selectors;
    attr_selectors_wb = global_meter_offset_mode[unit][mode].attr_selectors;
    total_policers = global_meter_offset_mode[unit][mode].no_of_policers;
    if ((global_meter_offset_mode[unit][mode].meter_attr.mode_type_v == \
                        udf_mode) ||
            (global_meter_offset_mode[unit][mode].meter_attr.mode_type_v == \
                        udf_cascade_mode) ||
            (global_meter_offset_mode[unit][mode].meter_attr.mode_type_v == \
                        udf_cascade_with_coupling_mode)) {

        /* Re-populate udf_subdiv based on attr_selectors */
        udf_pkt_attr = &(global_meter_offset_mode[unit][mode].meter_attr.
                                udf_pkt_attr_selectors_v);
        for (sel = 0; sel < num_selectors; sel++) {
            udf_id = attr_selectors_wb[sel].udf_id;
            offset = attr_selectors_wb[sel].offset;
            width = attr_selectors_wb[sel].width;

            if (udf_set_bits[offset] == 0) {
                index = total_subdiv;
                udf_subdiv[index].udf_id = udf_id;
                udf_subdiv[index].offset = offset;
                udf_subdiv[index].width = width;
                total_subdiv++;
                udf_set_bits[offset] = 1;
            }
        }

        /* Sort */
        sorted = 0;
        for (index = 0; index < SVC_METER_UDF1_MAX_BIT_POSITION; index++) {
            if (udf_set_bits[index] == 1) {
                for (index1 = sorted; index1 < total_subdiv; index1++) {
                    if (udf_subdiv[index1].offset == index) {
                        /* Swap to rearrange */
                        sal_memcpy(&dummy_data, &udf_subdiv[index1],
                                sizeof (bcm_policer_udf_sub_div_t));
                        sal_memcpy(&udf_subdiv[index1],
                                &udf_subdiv[sorted],
                                sizeof(bcm_policer_udf_sub_div_t));
                        sal_memcpy(&udf_subdiv[sorted], &dummy_data,
                                sizeof (bcm_policer_udf_sub_div_t));
                        sorted++;
                        break;
                    }
                }
            }
        }
        /* Save */
        memcpy (&(udf_pkt_attr->udf_subdiv[0]), &udf_subdiv[0], sizeof(udf_subdiv));
        udf_pkt_attr->total_subdiv = total_subdiv;
    } else if (global_meter_offset_mode[unit][mode].meter_attr.mode_type_v ==
            compressed_mode) {

        npolicers = total_policers;
        if (global_meter_offset_mode[unit][mode].type ==
                bcmPolicerGroupModeTypeCascadeWithCoupling) {
            npolicers = total_policers/2;
        }
        sal_memset(&pkt_attr_selectors, 0, sizeof(bcm_policer_attr_selectors_t));
        pkt_attr_selectors.combine_attr_data =
            sal_alloc(sizeof(bcm_policer_combine_attr_t) * npolicers,
                    "combine attributes");
        if (pkt_attr_selectors.combine_attr_data == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(pkt_attr_selectors.combine_attr_data, 0,
                sizeof(bcm_policer_combine_attr_t) * npolicers);

        /*
         * allocate temp memory for convert attr_selectors_wb to attr_selectors
         * _bcm_esw_policer_group_mode_fillup_values
         */
        attr_selectors = sal_alloc(
                sizeof(bcm_policer_group_mode_attr_selector_t) * num_selectors,
                "bcm_policer_group_mode_attr_selector_t");
        if (attr_selectors == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(attr_selectors, 0,
            sizeof(bcm_policer_group_mode_attr_selector_t) * num_selectors);

        for (i = 0; i < num_selectors; i++) {
            rv = _bcm_wb_attr_selectors_copy_to_attr_selectors(
                    attr_selectors_wb,
                    attr_selectors);
            if (BCM_FAILURE(rv)) {
               sal_free(attr_selectors);
               return (rv);
            }
        }

        _bcm_esw_policer_group_mode_fillup_values (unit, 0,
                global_meter_offset_mode[unit][mode].type,
                npolicers, num_selectors, attr_selectors, &pkt_attr_selectors);
        sal_free(attr_selectors);

        cmprsd_attr_selectors = &(global_meter_offset_mode[unit][mode].
                meter_attr.compressed_attr_selectors_v);
        sal_memset (&cmprsd_attr_selectors->pkt_attr_bits_v, 0,
                sizeof(cmprsd_attr_selectors->pkt_attr_bits_v));
        if (pkt_attr_selectors.cng != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_cng_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.cng =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.int_pri != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_int_pri_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.int_pri =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.vlan_format != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_vlan_format_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.vlan_format =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.outer_dot1p != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_outer_dot1p_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.outer_dot1p =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.inner_dot1p != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_inner_dot1p_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.inner_dot1p =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.pkt_resolution != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_pkt_res_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.pkt_resolution =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.drop != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_drop_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.drop =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        if (pkt_attr_selectors.int_pri != 0) {
            attr_count = _shr_popcount(pkt_attr_selectors.combine_ip_pkt_flags);
            cmprsd_attr_selectors->pkt_attr_bits_v.int_pri =
                _bcm_policer_num_of_bits_get(attr_count);
        }
        for (index = 0; index < _BCM_POLICER_BIT_ARRAY_SIZE; index++) {
            port_attr_count += _shr_popcount(pkt_attr_selectors.combine_value_array
                    [_bcmPolicerAttrPort][index]);
            tos_dscp_attr_count += _shr_popcount(pkt_attr_selectors.combine_value_array
                    [_bcmPolicerAttrTosDscp][index]);
            tos_ecn_attr_count += _shr_popcount(pkt_attr_selectors.combine_value_array
                    [_bcmPolicerAttrTosEcn][index]);
            svp_type_attr_count += _shr_popcount(pkt_attr_selectors.combine_value_array
                    [_bcmPolicerAttrSvpType][index]);
        }
        cmprsd_attr_selectors->pkt_attr_bits_v.ing_port =
            _bcm_policer_num_of_bits_get(port_attr_count);
        cmprsd_attr_selectors->pkt_attr_bits_v.tos =
            _bcm_policer_num_of_bits_get(tos_dscp_attr_count);
        cmprsd_attr_selectors->pkt_attr_bits_v.tos_ecn =
            _bcm_policer_num_of_bits_get(tos_ecn_attr_count);
        cmprsd_attr_selectors->pkt_attr_bits_v.svp_type =
            _bcm_policer_num_of_bits_get(svp_type_attr_count);

        sal_free(pkt_attr_selectors.combine_attr_data);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_global_meter_offset_mode_reinit
 * Purpose:
 *      Recover the service meter offset mode configuration information
 *      from HW on warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_offset_mode_reinit(int unit)
{
    int rv = BCM_E_NONE;
    int mode = 0;
    bcm_policer_group_mode_t          group_mode = 0;
    int                               npolicers = 0;
    uint64 selector_key_value;
    uint32 selector_count = 0, index = 0;
    uint32 selector_for_bit_x_en_field_value[8] = {0};
    uint32 selector_for_bit_x_en_field_name[8] = {
                SELECTOR_0_ENf,
                SELECTOR_1_ENf,
                SELECTOR_2_ENf,
                SELECTOR_3_ENf,
                SELECTOR_4_ENf,
                SELECTOR_5_ENf,
                SELECTOR_6_ENf,
                SELECTOR_7_ENf,
    };
    uint32  selector_for_bit_x_field_value[8] = {0};
    uint32  selector_for_bit_x_field_name[8] = {
                SELECTOR_FOR_BIT_0f,
                SELECTOR_FOR_BIT_1f,
                SELECTOR_FOR_BIT_2f,
                SELECTOR_FOR_BIT_3f,
                SELECTOR_FOR_BIT_4f,
                SELECTOR_FOR_BIT_5f,
                SELECTOR_FOR_BIT_6f,
                SELECTOR_FOR_BIT_7f,
    };
    bcm_policer_svc_meter_mode_type_t mode_type = 0;

    COMPILER_64_ZERO(selector_key_value);
    for (mode = 1; mode < BCM_POLICER_SVC_METER_MAX_MODE; mode++) {
        BCM_IF_ERROR_RETURN(soc_reg64_get(unit,
                                          _pkt_attr_sel_key_reg[mode],
                                          REG_PORT_ANY,
                                          0,
                                          &selector_key_value)
                            );
        if (!COMPILER_64_IS_ZERO(selector_key_value))
        {
            for (selector_count = 0, index = 0; index < 8; index++)
            {
                selector_for_bit_x_en_field_value[index] = soc_reg64_field32_get(
                        unit,
                        _pkt_attr_sel_key_reg[mode],
                        selector_key_value,
                        selector_for_bit_x_en_field_name[index]);
                selector_count += selector_for_bit_x_en_field_value[index];
                selector_for_bit_x_field_value[index] = 0;
                if (selector_for_bit_x_en_field_value[index] != 0)
                {
                          selector_for_bit_x_field_value[index] =
                                  soc_reg64_field32_get(
                                     unit,
                                     _pkt_attr_sel_key_reg[mode],
                                     selector_key_value,
                                     selector_for_bit_x_field_name[index]);
                }
            }

            if (soc_reg64_field32_get(unit,
                                     _pkt_attr_sel_key_reg[mode],
                                      selector_key_value, USE_UDF_KEYf))
            {
                mode_type = udf_mode;
                rv =  _bcm_esw_global_meter_udf_offset_mode_reinit(unit, mode,
                             selector_count, selector_for_bit_x_en_field_value,
                                              selector_for_bit_x_field_value);
                if (BCM_FAILURE(rv)) {
                   LOG_DEBUG(BSL_LS_BCM_POLICER,
                             (BSL_META_U(unit,
                                         "Unable to re-init UDF offset mode\n")));
                   return (rv);
                }
            } else if (soc_reg64_field32_get(unit,
                                   _pkt_attr_sel_key_reg[mode],
                                   selector_key_value,
                                   USE_COMPRESSED_PKT_KEYf))
            {
                rv =  _bcm_esw_global_meter_compressed_offset_mode_reinit(unit,
                                          mode, selector_count,
                                          selector_for_bit_x_en_field_value,
                                          selector_for_bit_x_field_value);
                if (BCM_FAILURE(rv)) {
                   LOG_DEBUG(BSL_LS_BCM_POLICER,
                             (BSL_META_U(unit,
                                         "Unable to re-init compressed offset "
                                         "mode\n")));
                   return (rv);
                }
            } else {
                rv = _bcm_esw_global_meter_uncompressed_offset_mode_reinit(
                         unit, mode, selector_count,
                         selector_for_bit_x_en_field_value,
                         selector_for_bit_x_field_value);
                if (BCM_FAILURE(rv)) {
                   LOG_VERBOSE(BSL_LS_BCM_POLICER,
                               (BSL_META_U(unit,
                                           "Unable to re-init uncompressed offset "
                                           "mode\n")));
                   return (rv);
                }
            }
            /* get the number of policers allocated for this mode */
            /* Warmboot version BCM_WB_VERSION_1_1 onwards, npolicers and
               group_mode are stored in scache. This function call is still
               left here for backward compatibility */
            rv = _bcm_esw_policer_get_offset_table_policer_count(unit,
                                              mode, &group_mode, &npolicers);
            if (BCM_FAILURE(rv)) {
               LOG_DEBUG(BSL_LS_BCM_POLICER,
                         (BSL_META_U(unit,
                                     "Unable to re-init number of policers "
                                     "to be allcated in this mode\n")));
               return (rv);
            }

            /* If npolicers is ZERO, group mode is also not stored  */
            if (npolicers == 0)  {
                continue;
            }
            global_meter_offset_mode[unit][mode].no_of_policers = npolicers;
            global_meter_offset_mode[unit][mode].group_mode = group_mode;
            if ((group_mode == bcmPolicerGroupModeCascade) ||
                (group_mode == bcmPolicerGroupModeCascadeWithCoupling) ||
                (group_mode == bcmPolicerGroupModeIntPriCascade) ||
                (group_mode == bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
                global_meter_offset_mode[unit][mode].meter_attr.mode_type_v =
                                                               cascade_mode;
            }
            if (mode_type == udf_cascade_mode) {
                global_meter_offset_mode[unit][mode].meter_attr.mode_type_v =
                                                               udf_cascade_mode;
            }
            if (mode_type == udf_cascade_with_coupling_mode) {
                global_meter_offset_mode[unit][mode].meter_attr.mode_type_v =
                                                 udf_cascade_with_coupling_mode;
            }
        }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_action_reinit
 * Purpose:
 *      Recover the policer action table entries on warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_action_reinit(int unit)
{
    int rv = BCM_E_NONE;
    int total_size = SOC_INFO(unit).global_meter_action_size;

    GLOBAL_METER_LOCK(unit);
        rv = _bcm_esw_svm_source_traverse (unit, SVM_POLICY_TABLEm, -1, total_size - 1,
                _bcm_esw_svm_action_reinit_from_table, NULL);
    GLOBAL_METER_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_esw_global_meter_policer_sync
 * Purpose:
 *      Write poliers which are created but not yet used in any
 *      table to scache so that we can recover the configuration
 *      on warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_esw_global_meter_policer_sync(int unit)
{
    int    rv = BCM_E_NONE;
    soc_scache_handle_t scache_handle;
    _global_meter_policer_control_t *global_meter_pl = NULL;
    uint32 hash_index;      /* Entry hash  */
    int policer_id = 0;
    uint8  *policer_state = NULL;
    uint32 npolicers, group, udf_id;
    uint8 type;
    int mode;
    int size = 0, count = 0;
    bcm_policer_svc_meter_attr_t *meter_attr = NULL;
    bcm_policer_group_mode_attr_selectors_info_t attr_selectors_info;

    if (!soc_feature(unit, soc_feature_global_meter)) {
        return BCM_E_UNAVAIL;
    }

    memset(&attr_selectors_info, 0, sizeof(attr_selectors_info));

    CHECK_GLOBAL_METER_INIT(unit);
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_POLICER, 0);

    size = sizeof(int) * BCM_GLOBAL_METER_MAX_SCACHE_ENTRIES;
    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_1)   {
        size += (sizeof(uint32) * BCM_POLICER_SVC_METER_MAX_MODE * 3);
    }
    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_2)   {
        size += (sizeof(bcm_policer_group_mode_attr_selectors_info_t) *
                          BCM_GLOBAL_METER_MAX_SCACHEABLE_GROUP_MODE);
    }
    SOC_IF_ERROR_RETURN(_bcm_esw_scache_ptr_get(unit, scache_handle,
                          FALSE, size, &policer_state,
                          BCM_WB_DEFAULT_VERSION, NULL));

    if (NULL == policer_state) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "SCACHE Memory not available \n")));
        return BCM_E_MEMORY;
    }

    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_1)   {
        /* Iterate through global_meter_offset_mode to store
           npolicers,group_mode,udf_id */
        for (mode = 0; mode < BCM_POLICER_SVC_METER_MAX_MODE; mode++)   {
            if (global_meter_offset_mode[unit][mode].used == 1) {
                npolicers = global_meter_offset_mode[unit][mode].no_of_policers;
                sal_memcpy(policer_state, &npolicers, sizeof(uint32));
                policer_state += sizeof(uint32);

                /* Using same 32-bit for saving group mode and type.
                 * group mode (0-23), type (24-31) */
                group = (uint32)global_meter_offset_mode[unit][mode].group_mode;
                type = (global_meter_offset_mode[unit][mode].type == -1) ?
                        0 : (global_meter_offset_mode[unit][mode].type + 1);
                group |= (type << BCM_POLICER_GLOBAL_METER_GROUP_MODE_TYPE_SHIFT);
                sal_memcpy(policer_state, &group, sizeof(uint32));
                policer_state += sizeof(uint32);

                meter_attr = &global_meter_offset_mode[unit][mode].meter_attr;
                udf_id = meter_attr->udf_pkt_attr_selectors_v.udf_id;
                sal_memcpy(policer_state, &udf_id, sizeof(uint32));
                policer_state += sizeof(uint32);

                if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_2) {
                    if (global_meter_offset_mode[unit][mode].type == -1) {
                        sal_memset(policer_state, 0, sizeof(attr_selectors_info));
                        policer_state += sizeof(attr_selectors_info);
                        continue;
                    }

                    /*Save attr_selectors info */
                    if (global_meter_offset_mode[unit][mode].attr_selectors != NULL) {
                        attr_selectors_info.flags = 0;
                        attr_selectors_info.num_selectors =
                            global_meter_offset_mode[unit][mode].no_of_selectors;
                        sal_memcpy(&attr_selectors_info.attr_selectors[0],
                                &global_meter_offset_mode[unit][mode].attr_selectors[0],
                                sizeof(bcm_policer_group_mode_attr_selector_wb_t) * \
                                attr_selectors_info.num_selectors);
                        sal_memcpy(policer_state, &attr_selectors_info,
                                sizeof(attr_selectors_info));
                    }
                    policer_state += sizeof(attr_selectors_info);
                }
            }
        }
    }

    /* go through policer control bookkeep structure and get policer
       entries with ref_count=0 */
    for (hash_index = 0; hash_index < _GLOBAL_METER_HASH_SIZE; hash_index++) {
        global_meter_pl  =  global_meter_policer_bookkeep[unit][hash_index];
        while ((NULL != global_meter_pl) &&
                      (count < BCM_GLOBAL_METER_MAX_SCACHE_ENTRIES)) {
            /* Match entry id. */
            if (global_meter_pl->ref_count == 0) {
                policer_id  = global_meter_pl->pid;
                sal_memcpy(policer_state, &policer_id, sizeof(bcm_policer_t));
                policer_state += sizeof(bcm_policer_t);
                count++;
            }
            global_meter_pl = global_meter_pl->next;
        }
        if (count == BCM_GLOBAL_METER_MAX_SCACHE_ENTRIES) {
            return rv;
        }
    }
    return rv;
}
/*
 * Function:
 *      _bcm_esw_global_meter_policer_recover_from_scache
 * Purpose:
 *     Recover policers which are not configured in HW from
 *     Scache as part of warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_policer_recover_from_scache(int unit)
{
    int    rv = BCM_E_NONE;
    int    stable_size;
    uint16 recovered_ver;
    int policer_id = 0;
    uint8  *policer_state = NULL;
    uint32 npolicers, udf_id;
    uint32 group = 0;
    uint8 type = 0;
    int mode=0;
    int size = 0, scache_index = 0;
    uint32 num_selectors = 0;
    soc_scache_handle_t scache_handle;
    _global_meter_policer_control_t *policer_control = NULL;
    bcm_policer_svc_meter_attr_t *meter_attr = NULL;
    bcm_policer_group_mode_attr_selectors_info_t *attr_sel_info;
    bcm_policer_group_mode_t group_mode;

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_POLICER, 0);

    size = sizeof(int32) * BCM_GLOBAL_METER_MAX_SCACHE_ENTRIES;
    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_1)   {
        size += (sizeof(uint32) * BCM_POLICER_SVC_METER_MAX_MODE * 3);
    }
    if (BCM_WB_DEFAULT_VERSION >= BCM_WB_VERSION_1_2)   {
        size += (sizeof(bcm_policer_group_mode_attr_selectors_info_t) *
                BCM_GLOBAL_METER_MAX_SCACHEABLE_GROUP_MODE);
    }
    if (SOC_WARM_BOOT(unit)) {
        /* Warm Boot */
        SOC_IF_ERROR_RETURN(soc_stable_size_get(unit, &stable_size));
        if (stable_size > size) {
            SOC_IF_ERROR_RETURN(_bcm_esw_scache_ptr_get(unit, scache_handle,
                        FALSE, size, &policer_state,
                        BCM_WB_DEFAULT_VERSION, &recovered_ver));

            if (NULL == policer_state) {
                LOG_DEBUG(BSL_LS_BCM_POLICER,
                        (BSL_META_U(unit,
                                    "SCACHE Memory not available \n")));
                return BCM_E_MEMORY;
            }

            /* Read from scache and populate fields for global_meter_offset_mode first */
            if (recovered_ver >= BCM_WB_VERSION_1_1)  {
                for (mode = 0; mode < BCM_POLICER_SVC_METER_MAX_MODE; mode++)   {
                    if (global_meter_offset_mode[unit][mode].used == 1) {
                        sal_memcpy(&npolicers, policer_state, sizeof(uint32));
                        global_meter_offset_mode[unit][mode].no_of_policers = npolicers;
                        policer_state += (sizeof(uint32));

                        sal_memcpy(&group, policer_state, sizeof(uint32));
                        group_mode = (group & (~(BCM_POLICER_GLOBAL_METER_GROUP_MODE_TYPE_MASK)));
                        global_meter_offset_mode[unit][mode].group_mode = group_mode;
                        type = (group & BCM_POLICER_GLOBAL_METER_GROUP_MODE_TYPE_MASK) >>
                                    BCM_POLICER_GLOBAL_METER_GROUP_MODE_TYPE_SHIFT;
                        if (type == 0) {
                            global_meter_offset_mode[unit][mode].type = -1;
                        } else {
                            global_meter_offset_mode[unit][mode].type = type - 1;
                        }
                        policer_state+=(sizeof(uint32));

                        meter_attr = &global_meter_offset_mode[unit][mode].meter_attr;
                        sal_memcpy(&udf_id, policer_state, sizeof(uint32));
                        meter_attr->udf_pkt_attr_selectors_v.udf_id = udf_id;
                        policer_state += (sizeof(uint32));

                        if (recovered_ver >= BCM_WB_VERSION_1_2) {
                            if (global_meter_offset_mode[unit][mode].type != -1) {
                                attr_sel_info = (bcm_policer_group_mode_attr_selectors_info_t *) policer_state;
                                num_selectors = attr_sel_info->num_selectors;
                                global_meter_offset_mode[unit][mode].no_of_selectors = num_selectors;

                                if (num_selectors != 0) {
                                    global_meter_offset_mode[unit][mode].attr_selectors =
                                        sal_alloc(sizeof(bcm_policer_group_mode_attr_selector_wb_t) * num_selectors,
                                                "attr selector info");
                                    if (global_meter_offset_mode[unit][mode].attr_selectors == NULL) {
                                        _bcm_esw_global_meter_cleanup(unit);
                                        return BCM_E_MEMORY;
                                    }
                                    sal_memcpy(&(global_meter_offset_mode[unit][mode].attr_selectors[0]),
                                            &(attr_sel_info->attr_selectors[0]),
                                            sizeof(bcm_policer_group_mode_attr_selector_wb_t) * num_selectors);
                                }
                                _bcm_esw_global_policer_offset_mode_update(unit, mode);
                            }
                            policer_state += sizeof(bcm_policer_group_mode_attr_selectors_info_t);
                        }
                        /* handling mode_type_v/type for udf based policers */
                        if (meter_attr->mode_type_v == udf_mode) {
                            global_meter_offset_mode[unit][mode].type = bcmPolicerGroupModeTypeNormal;
                            if (group_mode == bcmPolicerGroupModeCascade) {
                                meter_attr->mode_type_v = udf_cascade_mode;
                                global_meter_offset_mode[unit][mode].type =
                                        bcmPolicerGroupModeTypeCascade;
                            } else if (group_mode == bcmPolicerGroupModeCascadeWithCoupling) {
                                meter_attr->mode_type_v = udf_cascade_with_coupling_mode;
                                global_meter_offset_mode[unit][mode].type =
                                        bcmPolicerGroupModeTypeCascadeWithCoupling;
                            }
                        } else if (global_meter_offset_mode[unit][mode].type != -1) {
                            /* Here, for group type Cascade/CascadeWithCoupling, there is
                             * no need to modfy mode_type_v. Lets keep it compressed/uncompressed. */
                            ;
                        } else {
                            if ((group_mode == bcmPolicerGroupModeCascade) ||
                                    (group_mode == bcmPolicerGroupModeCascadeWithCoupling) ||
                                    (group_mode == bcmPolicerGroupModeIntPriCascade) ||
                                    (group_mode == bcmPolicerGroupModeIntPriCascadeWithCoupling)) {
                                meter_attr->mode_type_v = cascade_mode;
                            }
                        }
                    }
                }
            }

            /*  read from scache and create policer bookkeep data structure */
            for (scache_index = 0; scache_index <
                    BCM_GLOBAL_METER_MAX_SCACHE_ENTRIES; scache_index++) {
                sal_memcpy(&policer_id,
                        &policer_state[scache_index * sizeof(int)],
                        sizeof(int));
                if ((policer_id & BCM_POLICER_GLOBAL_METER_INDEX_MASK) > 0) {
                    policer_control = NULL;
                    rv = _bcm_esw_policer_config_from_meter_table(unit,
                            policer_id, policer_control);
                }
            }
        }
    } else {
        /* Cold Boot */
        rv = _bcm_esw_scache_ptr_get(unit, scache_handle, TRUE,
                size, &policer_state,
                BCM_WB_DEFAULT_VERSION, NULL);
        if (BCM_FAILURE(rv) && (rv != BCM_E_NOT_FOUND)) {
            LOG_DEBUG(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Scache Memory not available\n")));
            return rv;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_global_meter_reinit
 * Purpose:
 *      Reinit all the global meter internal data structures on warm boot
 * Parameters:
 *     unit                  - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_esw_global_meter_reinit(int unit)
{
    int rv = BCM_E_NONE;
    rv =  _bcm_esw_global_meter_offset_mode_reinit(unit);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to reinit offset modes\n")));
        return (rv);
    }
    /* go through scache and recover policer configuration */
    rv = _bcm_esw_global_meter_policer_recover_from_scache(unit);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to reinit policer configuration\n")));
        return (rv);
    }
    /* go through all policer entries to increment action reference count
       and set policer configuration parameter */
    rv = _bcm_policer_config_reinit(unit);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to reinit policer configuration\n")));
        return (rv);
    }
    /* reinit action table bookkeep data structure */
    rv =  _bcm_esw_global_meter_action_reinit(unit);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_POLICER,
                    (BSL_META_U(unit,
                                "Unable to reinit meter action configuration\n")));
        return (rv);
    }
    return BCM_E_NONE;
}

#endif /* BCM_WARM_BOOT_SUPPORT */
