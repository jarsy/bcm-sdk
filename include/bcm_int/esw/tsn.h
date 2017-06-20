/*
 * $Id: tsn.h $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        tsn.h
 * Purpose:     Definitions for TSN.
 */

#ifndef _BCM_INT_TSN_H_
#define _BCM_INT_TSN_H_

#include <bcm/tsn.h>

#if defined(BCM_TSN_SUPPORT)

/*
 * Macro:
 *     TSN_ALLOC
 * Purpose:
 *      Generic memory allocation routine.
 * Parameters:
 *    _unit_    - Unit.
 *    _ptr_     - Pointer to allocated memory.
 *    _ptype_   - Pointer type.
 *    _size_    - Size of heap memory to be allocated.
 *    _descr_   - Information about this memory allocation.
 *    _dma_     - use sal_alloc or soc_cm_alloc.
 *    _rv_      - return value
 */
#define TSN_ALLOC(_unit_, _ptr_, _ptype_, _size_, _descr_, _dma_, _rv_) \
            do { \
                if (NULL == (_ptr_)) { \
                    if (0 == (_dma_)) { \
                        (_ptr_) = (_ptype_ *)sal_alloc((_size_), (_descr_)); \
                    } else { \
                        (_ptr_) = (_ptype_ *)soc_cm_salloc((_unit_), (_size_), \
                                                            (_descr_)); \
                    } \
                } \
                if ((_ptr_) != NULL) { \
                    sal_memset((_ptr_), 0, (_size_)); \
                    (_rv_) = BCM_E_NONE; \
                }  else { \
                    LOG_ERROR(BSL_LS_BCM_TSN, \
                              (BSL_META("TSN Error: Allocation failure %s\n"), \
                               (_descr_))); \
                    (_rv_) = BCM_E_MEMORY; \
                } \
            } while (0)

/*
 * Macro:
 *     TSN_FREE
 * Purpose:
 *      Generic memory free routine.
 * Parameters:
 *    _unit_    - Unit.
 *    _ptr_     - Memory pointer to be free
 *    _dma_     - use sal_free or soc_cm_free.
 */
#define TSN_FREE(_unit_, _ptr_, _dma_) \
            do { \
                if (0 == (_dma_)) { \
                    sal_free((_ptr_)); \
                } else { \
                    soc_cm_sfree((_unit_), (_ptr_)); \
                } \
            } while (0)

#if defined(BCM_TSN_SR_SUPPORT)

/*
 * SR flow ID conversion between software flow ID and hardware flow ID
 */
extern int
bcmi_esw_tsn_sr_hw_flow_id_get(
    int unit,
    bcm_tsn_sr_flow_t flow_id,
    uint32 *hw_id);
extern int
bcmi_esw_tsn_sr_sw_flow_id_get(
    int unit, uint32
    hw_id,
    bcm_tsn_sr_flow_t *flow_id);

/*
 * SR flowset reference count control (increase or decrease reference count)
 */
extern int
bcmi_esw_tsn_sr_flowset_ref_inc(int unit, bcm_tsn_sr_flowset_t flowset);
extern int
bcmi_esw_tsn_sr_flowset_ref_dec(int unit, bcm_tsn_sr_flowset_t flowset);

/*
 * SR flow to flowset converstion (identify the flowset based on a flow)
 */
extern int
bcmi_esw_tsn_sr_flowset_identify(
    int unit,
    bcm_tsn_sr_flow_t flow_id,
    bcm_tsn_sr_flowset_t *flowset);

#endif /* BCM_TSN_SR_SUPPORT */

/*
 * TSN flow ID conversion between software flow ID and hardware flow ID
 */
extern int
bcmi_esw_tsn_hw_flow_id_get(int unit, bcm_tsn_flow_t flow_id, uint32 *hw_id);
extern int
bcmi_esw_tsn_sw_flow_id_get(int unit, uint32 hw_id, bcm_tsn_flow_t *flow_id);

/*
 * TSN flowset reference count control (increase or decrease reference count)
 */
extern int
bcmi_esw_tsn_flowset_ref_inc(int unit, bcm_tsn_flowset_t flowset);
extern int
bcmi_esw_tsn_flowset_ref_dec(int unit, bcm_tsn_flowset_t flowset);

/*
 * TSN flow to flowset converstion (identify the flowset based on a flow)
 */
extern int
bcmi_esw_tsn_flowset_identify(
    int unit,
    bcm_tsn_flow_t flow_id,
    bcm_tsn_flowset_t *flowset);

/*
 * TSN MTU reference count control (increase or decrease reference count)
 */
extern int
bcmi_esw_tsn_mtu_profile_ref_inc(
    int unit,
    int mtu_profile_id);
extern int
bcmi_esw_tsn_mtu_profile_ref_dec(
    int unit,
    int mtu_profile_id);

/*
 * TSN STU reference count control (increase or decrease reference count)
 */
extern int
bcmi_esw_tsn_stu_profile_ref_inc(
    int unit,
    int stu_profile_id);
extern int
bcmi_esw_tsn_stu_profile_ref_dec(
    int unit,
    int stu_profile_id);

/*
 * TSN MTU profile ID conversion between
 * software profile ID and hardware profile ID
 */
extern int
bcmi_esw_tsn_mtu_hw_profile_id_get(
    int unit,
    int mtu_profile_id,
    int *hw_profile_id);
extern int
bcmi_esw_tsn_mtu_sw_profile_id_get(
    int unit,
    int hw_profile_id,
    int *mtu_profile_id);

/*
 * TSN STU profile ID conversion between
 * software profile ID and hardware profile ID
 */
extern int
bcmi_esw_tsn_stu_hw_profile_id_get(
    int unit,
    int stu_profile_id,
    int *hw_profile_id);
extern int
bcmi_esw_tsn_stu_sw_profile_id_get(
    int unit,
    int hw_profile_id,
    int *stu_profile_id);

/* per-chip control drivers */
typedef struct tsn_chip_ctrl_info_s {
    int (*tsn_chip_ctrl_set)(int, bcm_tsn_control_t, uint32);
    int (*tsn_chip_ctrl_get)(int, bcm_tsn_control_t, uint32 *);
} tsn_chip_ctrl_info_t;

/* structure for device specific info */
typedef struct bcmi_esw_tsn_dev_info_s {
    /* per-chip control drivers */
    const tsn_chip_ctrl_info_t *tsn_chip_ctrl_info;
} bcmi_esw_tsn_dev_info_t;

/*
 * TSN Priority Map conversion between software Map ID and hardware Index
 */
extern int
bcmi_esw_tsn_pri_map_hw_index_get(
    int unit,
    bcm_tsn_pri_map_t map_id,
    uint32 *hw_index);

extern int
bcmi_esw_tsn_pri_map_map_id_get(
    int unit,
    uint32 hw_index,
    bcm_tsn_pri_map_t *map_id);

/*
 * TSN Priority Map reference count control (increase or decrease reference
 * count)
 */
extern int
bcmi_esw_tsn_pri_map_ref_cnt_dec(
    int unit,
    bcm_tsn_pri_map_t map_id);

extern int
bcmi_esw_tsn_pri_map_ref_cnt_inc(
    int unit,
    bcm_tsn_pri_map_t map_id);

extern int
bcmi_esw_tsn_pri_map_ref_cnt_get(
    int unit,
    bcm_tsn_pri_map_t map_id,
    uint32 *ref_cnt);

#if defined(BCM_WARM_BOOT_SUPPORT)
/*
 * TSN Priority Map warmboot recovery function provided for mtu/stu module
 */
extern int
bcmi_esw_tsn_pri_map_mtu_stu_wb_set(
    int unit,
    bcm_tsn_pri_map_t map_id,
    bcm_tsn_pri_map_config_t *config);
#endif /* BCM_WARM_BOOT_SUPPORT */

#endif /* BCM_TSN_SUPPORT */

#endif /* _BCM_INT_TSN_H_ */
