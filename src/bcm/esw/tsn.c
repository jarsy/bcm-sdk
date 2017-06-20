/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <soc/drv.h>
#include <bcm/tsn.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <shared/bsl.h>
#include <bcm_int/esw/tsn.h>
#if defined(BCM_GREYHOUND2_SUPPORT)
#include <bcm_int/esw/greyhound2.h>
#endif /* BCM_GREYHOUND2_SUPPORT */

#if defined(BCM_TSN_SUPPORT)
/* Initialization flag for TSN bookkeeping info structure */
static int tsn_bk_info_initialized = 0;

/*
 * Structure definition for TSN bookkeeping info,
 * including bookkeeping and chip specific info
 */
typedef struct bcmi_esw_tsn_bk_info_s {
    /* bookkeeping structure defined below */

    /* device specific info */
    const bcmi_esw_tsn_dev_info_t *tsn_dev_info;
} bcmi_esw_tsn_bk_info_t;

/* TSN bookkeeping info */
STATIC bcmi_esw_tsn_bk_info_t *tsn_bk_info[BCM_MAX_NUM_UNITS] = { NULL };

/* Get the tsn_bk_info[unit] pointer */
STATIC int
tsn_bk_info_instance_get(int unit, bcmi_esw_tsn_bk_info_t **bk_info)
{
    /* Input parameters check. */
    if (NULL == bk_info) {
        return (BCM_E_PARAM);
    }
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Check if TSN feature supported and initialized */
    if (!soc_feature(unit, soc_feature_tsn)) {
        return (BCM_E_UNAVAIL);
    }
    if (tsn_bk_info[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_TSN,
                  (BSL_META_U(unit,
                              "TSN Error: bookkeeping info not "
                              "initialized\n")));
        return (BCM_E_INIT);
    }

    /* Fill info structure. */
    *bk_info = tsn_bk_info[unit];

    return (BCM_E_NONE);
}

#if defined(BCM_TSN_SR_SUPPORT)

/*
 * Function:
 *     bcmi_esw_tsn_sr_flowset_ref_inc
 * Purpose:
 *     Increase reference count for the specified SR flowset
 * Parameters:
 *     unit             - (IN) unit number
 *     flowset          - (IN) flowset ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_sr_flowset_ref_inc(int unit, bcm_tsn_sr_flowset_t flowset)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_sr_flowset_ref_dec
 * Purpose:
 *     Decrease reference count for the specified SR flowset
 * Parameters:
 *     unit             - (IN) unit number
 *     flowset          - (IN) flowset ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_sr_flowset_ref_dec(int unit, bcm_tsn_sr_flowset_t flowset)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_sr_hw_flow_id_get
 * Purpose:
 *     Convert software SR flow ID to hardware SR flow ID
 * Parameters:
 *     unit             - (IN) unit number
 *     flow_id          - (IN) SW flow ID
 *     hw_id            - (OUT) HW flow ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_sr_hw_flow_id_get(
    int unit,
    bcm_tsn_sr_flow_t flow_id,
    uint32 *hw_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_sr_sw_flow_id_get
 * Purpose:
 *     Convert hardware SR flow ID to software SR flow ID
 * Parameters:
 *     unit             - (IN) unit number
 *     hw_id            - (IN) HW flow ID
 *     flow_id          - (OUT) SW flow ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_sr_sw_flow_id_get(
    int unit,
    uint32 hw_id,
    bcm_tsn_sr_flow_t *flow_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_sr_flowset_identify
 * Purpose:
 *     SR flow to flowset converstion (identify the flowset based on a flow)
 * Parameters:
 *     unit             - (IN) unit number
 *     flow_id          - (IN) flow ID
 *     flowset          - (OUT) flowset ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_sr_flowset_identify(
    int unit,
    bcm_tsn_sr_flow_t flow_id,
    bcm_tsn_sr_flowset_t *flowset)
{
    return BCM_E_UNAVAIL;
}

#endif /* BCM_TSN_SR_SUPPORT */

/*
 * Function:
 *     bcmi_esw_tsn_flowset_ref_inc
 * Purpose:
 *     Increase reference count for the specified TSN flowset
 * Parameters:
 *     unit             - (IN) unit number
 *     flowset          - (IN) flowset ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_flowset_ref_inc(int unit, bcm_tsn_flowset_t flowset)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_flowset_ref_dec
 * Purpose:
 *     Decrease reference count for the specified TSN flowset
 * Parameters:
 *     unit             - (IN) unit number
 *     flowset          - (IN) flowset ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_flowset_ref_dec(int unit, bcm_tsn_flowset_t flowset)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_hw_flow_id_get
 * Purpose:
 *     Convert software TSN flow ID to hardware TSN flow ID
 * Parameters:
 *     unit             - (IN) unit number
 *     flow_id          - (IN) SW flow ID
 *     hw_id            - (OUT) HW flow ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_hw_flow_id_get(int unit, bcm_tsn_flow_t flow_id, uint32 *hw_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_sw_flow_id_get
 * Purpose:
 *     Convert hardware TSN flow ID to software TSN flow ID
 * Parameters:
 *     unit             - (IN) unit number
 *     hw_id            - (IN) HW flow ID
 *     flow_id          - (OUT) SW flow ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_sw_flow_id_get(int unit, uint32 hw_id, bcm_tsn_flow_t *flow_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_flowset_identify
 * Purpose:
 *     TSN flow to flowset converstion (identify the flowset based on a flow)
 * Parameters:
 *     unit             - (IN) unit number
 *     flow_id          - (IN) flow ID
 *     flowset          - (OUT) flowset ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_flowset_identify(
    int unit,
    bcm_tsn_flow_t flow_id,
    bcm_tsn_flowset_t *flowset)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcmi_esw_tsn_pri_map_hw_index_get
 * Purpose:
 *      Get HW memory base index with given map_id
 * Parameters:
 *      unit     - (IN) Unit number
 *      map_id   - (IN) Map ID
 *      hw_index - (OUT) HW memory base index
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_pri_map_hw_index_get(
    int unit,
    bcm_tsn_pri_map_t map_id,
    uint32 *hw_index)
{
    bcm_error_t rv = BCM_E_UNAVAIL;

    /* Input parameters check. */
    if (NULL == hw_index) {
        return (BCM_E_PARAM);
    }
    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_pri_map_hw_index_get\n")));

    return rv;
}

/*
 * Function:
 *      bcmi_esw_tsn_pri_map_map_id_get
 * Purpose:
 *      Get map_id with given HW memory base index
 * Parameters:
 *      unit     - (IN) Unit number
 *      hw_index - (IN) HW memory base index
 *      map_id   - (OUT) Map ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_pri_map_map_id_get(
    int unit,
    uint32 hw_index,
    bcm_tsn_pri_map_t *map_id)
{
    bcm_error_t rv = BCM_E_UNAVAIL;

    /* Input parameters check. */
    if (NULL == map_id) {
        return (BCM_E_PARAM);
    }
    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_pri_map_map_id_get\n")));

    return rv;
}

/*
 * Function:
 *      bcmi_esw_tsn_pri_map_ref_cnt_dec
 * Purpose:
 *      Decrease reference cnt by 1 with given map_id
 * Parameters:
 *      unit   - (IN) Unit number
 *      map_id - (IN) Map ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_pri_map_ref_cnt_dec(
    int unit,
    bcm_tsn_pri_map_t map_id)
{
    bcm_error_t rv = BCM_E_UNAVAIL;
    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_pri_map_ref_cnt_dec\n")));

    return rv;
}

/*
 * Function:
 *      bcmi_esw_tsn_pri_map_ref_cnt_inc
 * Purpose:
 *      Increase reference cnt by 1 with given map_id
 * Parameters:
 *      unit   - (IN) Unit number
 *      map_id - (IN) Map ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_pri_map_ref_cnt_inc(
    int unit,
    bcm_tsn_pri_map_t map_id)
{
    bcm_error_t rv = BCM_E_UNAVAIL;

    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_pri_map_ref_cnt_inc\n")));

    return rv;
}

/*
 * Function:
 *      bcmi_esw_tsn_pri_map_ref_cnt_get
 * Purpose:
 *      Get current reference cnt with given map_id
 * Parameters:
 *      unit    - (IN) Unit number
 *      map_id  - (IN) Map ID
 *      ref_cnt - (OUT) Reference cnt
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_pri_map_ref_cnt_get(
    int unit,
    bcm_tsn_pri_map_t map_id,
    uint32 *ref_cnt)
{
    bcm_error_t rv = BCM_E_UNAVAIL;

    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_pri_map_ref_cnt_get\n")));

    /* Parameter NULL error handing */
    if (NULL == ref_cnt) {
        return BCM_E_PARAM;
    }
    return rv;
}

#if defined(BCM_WARM_BOOT_SUPPORT)
/*
 * Function:
 *      bcmi_esw_tsn_pri_map_mtu_stu_wb_set
 * Purpose:
 *      Warmboot function provided for mtu/stu module doing warmboot recovery
 *      MTU/STU modules should have config and map_id when doing warmboot
 *      and this API is for MTU/STU modules to do WB recovery.
 * Parameters:
 *      unit   - (IN) Unit number
 *      map_id - (IN) Map ID
 *      config - (IN) Priority Map configuration
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_pri_map_mtu_stu_wb_set(
    int unit,
    bcm_tsn_pri_map_t map_id,
    bcm_tsn_pri_map_config_t *config)
{
    /* Input parameters check. */
    if (NULL == config) {
        return (BCM_E_PARAM);
    }
    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_pri_map_mtu_stu_wb_set\n")));

    if (!SOC_WARM_BOOT(unit)) {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *     bcmi_esw_tsn_bk_info_cleanup
 * Purpose:
 *     Internal function to clear the resource of the TSN module.
 * Parameters:
 *     unit             - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 *
 */
STATIC int
bcmi_esw_tsn_bk_info_cleanup(int unit)
{
    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_bk_info_cleanup\n")));

    /* Free and clear the bookkeeping info structure */
    if (tsn_bk_info[unit] != NULL) {
        TSN_FREE(unit, tsn_bk_info[unit], 0);
        tsn_bk_info[unit] = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcmi_esw_tsn_init
 * Purpose:
 *     Internal function to initialize the resource and setup device info.
 * Parameters:
 *     unit - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 *
 */
STATIC int
bcmi_esw_tsn_init(int unit)
{
    bcmi_esw_tsn_bk_info_t *bk_info = NULL;
    int rv = BCM_E_NONE;
    int i;

    LOG_VERBOSE(BSL_LS_BCM_TSN,
                (BSL_META_U(unit,
                            "bcmi_esw_tsn_init\n")));

    if (!soc_feature(unit, soc_feature_tsn)) {
        return BCM_E_UNAVAIL;
    }

    /* Initialize structure */
    if (!tsn_bk_info_initialized) {
        for (i = 0; i < BCM_MAX_NUM_UNITS; i++) {
            tsn_bk_info[i] = NULL;
        }
        tsn_bk_info_initialized = 1;
    }

    /* clean up the bookkeeping resource */
    if (tsn_bk_info[unit] != NULL) {
        rv = bcmi_esw_tsn_bk_info_cleanup(unit);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    /* Allocate TSN bookkeeping resource for this unit. */
    TSN_ALLOC(unit, bk_info, bcmi_esw_tsn_bk_info_t,
              sizeof(bcmi_esw_tsn_bk_info_t),
              "TSN bookkeeping info", 0, rv);
    if (BCM_FAILURE(rv)) {
        LOG_VERBOSE(BSL_LS_BCM_TSN,
                    (BSL_META_U(unit,
                                "bcmi_esw_tsn_init: failed to "
                                "allocate memory\n")));
        return rv;
    }

    /* Get the chip dev_info structure. */
    tsn_bk_info[unit] = bk_info;
#ifdef BCM_GREYHOUND2_SUPPORT
    if (SOC_IS_GREYHOUND2(unit)) {
        rv = bcmi_gh2_tsn_info_init(unit, &bk_info->tsn_dev_info);
    } else
#endif
    {
        rv = BCM_E_UNAVAIL;
    }

    /* Release resource if any error occurred. */
    if (BCM_FAILURE(rv) || bk_info->tsn_dev_info == NULL) {
        bcmi_esw_tsn_bk_info_cleanup(unit);
        return (rv);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcmi_esw_tsn_detach
 * Purpose:
 *     Clear the resource of the TSN module.
 * Parameters:
 *     unit - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 *
 */
STATIC int
bcmi_esw_tsn_detach(int unit)
{
    if (!soc_feature(unit, soc_feature_tsn)) {
        return BCM_E_UNAVAIL;
    }

    /* chip specific detach */
#ifdef BCM_GREYHOUND2_SUPPORT
    if (SOC_IS_GREYHOUND2(unit)) {
        (void)bcmi_gh2_tsn_info_detach(unit);
    }
#endif

    /* clean up bookkeeping info resource */
    return bcmi_esw_tsn_bk_info_cleanup(unit);
}

/*
 * Function:
 *     bcmi_esw_tsn_control_set
 * Description:
 *     Internal function for configure device-based operation modes.
 * Parameters:
 *     unit - (IN) Unit number.
 *     type - (IN) Control type of bcm_tsn_control_t.
 *     arg  - (IN) argument to be set
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
bcmi_esw_tsn_control_set(int unit, bcm_tsn_control_t type, uint32 arg)
{
    bcmi_esw_tsn_bk_info_t *bk_info = NULL;
    const bcmi_esw_tsn_dev_info_t *dev_info = NULL;
    const tsn_chip_ctrl_info_t *ctrl_info = NULL;

    BCM_IF_ERROR_RETURN(tsn_bk_info_instance_get(unit, &bk_info));
    dev_info = bk_info->tsn_dev_info;

    ctrl_info = dev_info->tsn_chip_ctrl_info;
    if ((ctrl_info != NULL) &&
        (ctrl_info->tsn_chip_ctrl_set != NULL)) {
        return ctrl_info->tsn_chip_ctrl_set(unit, type, arg);
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_control_get
 * Description:
 *     Internal function for get the configure of device-based operation modes.
 * Parameters:
 *     unit - (IN) Unit number.
 *     type - (IN) Control type of bcm_tsn_control_t.
 *     arg  - (OUT) argument got
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
bcmi_esw_tsn_control_get(int unit, bcm_tsn_control_t type, uint32 *arg)
{
    bcmi_esw_tsn_bk_info_t *bk_info = NULL;
    const bcmi_esw_tsn_dev_info_t *dev_info = NULL;
    const tsn_chip_ctrl_info_t *ctrl_info = NULL;

    BCM_IF_ERROR_RETURN(tsn_bk_info_instance_get(unit, &bk_info));
    dev_info = bk_info->tsn_dev_info;

    ctrl_info = dev_info->tsn_chip_ctrl_info;
    if ((ctrl_info != NULL) &&
        (ctrl_info->tsn_chip_ctrl_get != NULL)) {
        return ctrl_info->tsn_chip_ctrl_get(unit, type, arg);
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcmi_esw_tsn_mtu_profile_ref_inc
 * Purpose:
 *      Increase the reference count for the
 *      specific mtu profile ID
 * Parameters:
 *      unit - (IN) Unit number
 *      mtu_profile_id - (IN) profile ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_mtu_profile_ref_inc(
    int unit,
    int mtu_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcmi_esw_tsn_mtu_profile_ref_dec
 * Purpose:
 *      Decrease the reference count of the
 *      specific mtu profile ID
 * Parameters:
 *      unit - (IN) Unit number
 *      mtu_profile_id - (IN) profile ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_mtu_profile_ref_dec(
    int unit,
    int mtu_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcmi_esw_tsn_stu_profile_ref_inc
 * Purpose:
 *      Increase the reference count for the
 *      specific stu profile ID
 * Parameters:
 *      unit - (IN) Unit number
 *      stu_profile_id - (IN) profile ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_stu_profile_ref_inc(
    int unit,
    int stu_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcmi_esw_tsn_stu_profile_ref_dec
 * Purpose:
 *      Decrease the reference count for the
 *      specific stu profile ID
 * Parameters:
 *      unit - (IN) Unit number
 *      stu_profile_id - (IN) profile ID
 * Returns:
 *      BCM_E_xxx
 */
int
bcmi_esw_tsn_stu_profile_ref_dec(
    int unit,
    int stu_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_mtu_hw_profile_id_get
 * Purpose:
 *     Convert software mtu profile ID to hardware mtu profile ID
 * Parameters:
 *     unit - (IN) unit number
 *     mtu_profile_id - (IN) SW profile ID
 *     hw_profile_id - (OUT) HW profile ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_mtu_hw_profile_id_get(
    int unit,
    int mtu_profile_id,
    int *hw_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_mtu_sw_profile_id_get
 * Purpose:
 *     Convert hardware mtu profile ID to software mtu profile ID
 * Parameters:
 *     unit - (IN) unit number
 *     hw_profile_id - (IN) HW profile ID
 *     mtu_profile_id - (OUT) SW profile ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_mtu_sw_profile_id_get(
    int unit,
    int hw_profile_id,
    int *mtu_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_stu_hw_profile_id_get
 * Purpose:
 *     Convert software stu profile ID to hardware stu profile ID
 * Parameters:
 *     unit - (IN) unit number
 *     stu_profile_id - (IN) SW profile ID
 *     hw_profile_id - (OUT) HW profile ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_stu_hw_profile_id_get(
    int unit,
    int stu_profile_id,
    int *hw_profile_id)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmi_esw_tsn_stu_sw_profile_id_get
 * Purpose:
 *     Convert hardware stu profile ID to software stu profile ID
 * Parameters:
 *     unit - (IN) unit number
 *     hw_profile_id - (IN) HW profile ID
 *     stu_profile_id - (OUT) SW profile ID
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
bcmi_esw_tsn_stu_sw_profile_id_get(
    int unit,
    int hw_profile_id,
    int *stu_profile_id)
{
    return BCM_E_UNAVAIL;
}
#endif /* BCM_TSN_SUPPORT */

/*
 * Function:
 *     bcm_esw_tsn_init
 * Purpose:
 *     Initialize the TSN module.
 * Parameters:
 *     unit - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 *
 */
int
bcm_esw_tsn_init(int unit)
{
#if defined(BCM_TSN_SUPPORT)
    /* initialize TSN info */
    BCM_IF_ERROR_RETURN(bcmi_esw_tsn_init(unit));

    return BCM_E_NONE;
#else /* BCM_TSN_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TSN_SUPPORT */
}

/*
 * Function:
 *     bcm_esw_tsn_detach
 * Purpose:
 *     Clear the resource of the TSN module.
 * Parameters:
 *     unit - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 *
 */
int
bcm_esw_tsn_detach(int unit)
{
#if defined(BCM_TSN_SUPPORT)
    BCM_IF_ERROR_RETURN(bcmi_esw_tsn_detach(unit));

    return BCM_E_NONE;
#else /* BCM_TSN_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* BCM_TSN_SUPPORT */
}

int bcm_esw_tsn_sr_port_config_set(int unit, bcm_gport_t port,
                                   bcm_tsn_sr_port_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_port_config_get(int unit, bcm_gport_t port,
                                   bcm_tsn_sr_port_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_pri_map_create(int unit, bcm_tsn_pri_map_config_t *config,
                               bcm_tsn_pri_map_t *map_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_pri_map_set(int unit, bcm_tsn_pri_map_t map_id,
                            bcm_tsn_pri_map_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_pri_map_get(int unit, bcm_tsn_pri_map_t map_id,
                            bcm_tsn_pri_map_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_pri_map_destroy(int unit, bcm_tsn_pri_map_t map_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_pri_map_traverse(int unit, bcm_tsn_pri_map_traverse_cb cb,
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_tx_flowset_create(int unit, bcm_tsn_pri_map_t pri_map,
                                     bcm_tsn_sr_tx_flow_config_t *default_config,
                                     bcm_tsn_sr_flowset_t *flowset)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_tx_flowset_config_get(int unit, bcm_tsn_sr_flowset_t flowset,
                                         bcm_tsn_pri_map_t *pri_map,
                                         bcm_tsn_sr_tx_flow_config_t *default_config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flowset_create(int unit, bcm_tsn_pri_map_t pri_map,
                                     bcm_tsn_sr_rx_flow_config_t *default_config,
                                     bcm_tsn_sr_flowset_t *flowset)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flowset_config_get(int unit, bcm_tsn_sr_flowset_t flowset,
                                         bcm_tsn_pri_map_t *pri_map,
                                         bcm_tsn_sr_rx_flow_config_t *default_config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flowset_status_get(int unit, bcm_tsn_sr_flowset_t flowset,
                                      bcm_tsn_sr_flowset_status_t *status)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flowset_traverse(int unit,
                                    int is_rx /* 1 for RX flowsets; 0 for TX flowsets */,
                                    bcm_tsn_sr_flowset_traverse_cb cb, void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flowset_destroy(int unit, bcm_tsn_sr_flowset_t flowset)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flowset_flow_get(int unit, bcm_tsn_sr_flowset_t flowset, int index,
                                    bcm_tsn_sr_flow_t *flow_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_tx_flow_config_get(int unit, bcm_tsn_sr_flow_t flow_id,
                                      bcm_tsn_sr_tx_flow_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_tx_flow_config_set(int unit, bcm_tsn_sr_flow_t flow_id,
                                      bcm_tsn_sr_tx_flow_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_tx_flow_status_get(int unit, bcm_tsn_sr_flow_t flow_id,
                                      bcm_tsn_sr_tx_flow_status_t *status)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flow_config_get(int unit, bcm_tsn_sr_flow_t flow_id,
                                      bcm_tsn_sr_rx_flow_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flow_config_set(int unit, bcm_tsn_sr_flow_t flow_id,
                                      bcm_tsn_sr_rx_flow_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flow_status_get(int unit, bcm_tsn_sr_flow_t flow_id,
                                      bcm_tsn_sr_rx_flow_status_t *status)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flow_reset(int unit, uint32 flags, bcm_tsn_sr_flow_t flow_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flow_seqnum_history_set(int unit, bcm_tsn_sr_flow_t flow_id,
                                              int offset_in_bits, int size_in_bits,
                                              uint8 *history_bits)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_rx_flow_seqnum_history_get(int unit, bcm_tsn_sr_flow_t flow_id,
                                              int offset_in_bits, int max_size_in_bits,
                                              uint8 *history_bits,
                                              int *actual_size_in_bits)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flowset_create(int unit, bcm_tsn_pri_map_t pri_map,
                               bcm_tsn_flow_config_t *default_config,
                               bcm_tsn_flowset_t *flowset)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flowset_status_get(int unit, bcm_tsn_flowset_t flowset,
                                   bcm_tsn_flowset_status_t *status)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flowset_flow_get(int unit, bcm_tsn_flowset_t flowset, int index,
                                 bcm_tsn_flow_t *flow_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flowset_config_get(int unit, bcm_tsn_flowset_t flowset,
                                   bcm_tsn_pri_map_t *pri_map,
                                   bcm_tsn_flow_config_t *default_config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flowset_traverse(int unit, bcm_tsn_flowset_traverse_cb cb,
                                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flowset_destroy(int unit, bcm_tsn_flowset_t flowset)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flow_config_get(int unit, bcm_tsn_flow_t flow_id,
                                bcm_tsn_flow_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_flow_config_set(int unit, bcm_tsn_flow_t flow_id,
                                bcm_tsn_flow_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_group_create(int unit,
                                           bcm_tsn_sr_auto_learn_group_config_t *config,
                                           int *group_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_group_get(int unit, int group_id,
                                        bcm_tsn_sr_auto_learn_group_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_group_set(int unit, int group_id,
                                        bcm_tsn_sr_auto_learn_group_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_group_destroy(int unit, int group_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_group_traverse(int unit,
                                             bcm_tsn_sr_auto_learn_group_traverse_cb cb,
                                             void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_enable(int unit, int enable,
                                     bcm_tsn_sr_auto_learn_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_auto_learn_enable_get(int unit, int *enabled,
                                         bcm_tsn_sr_auto_learn_config_t *config)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_esw_tsn_control_set
 * Description:
 *     Configure device operation modes for TSN.
 * Parameters:
 *     unit - (IN) Unit number.
 *     type - (IN) Control type of bcm_tsn_control_t.
 *     arg  - (IN) argument to be set
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_tsn_control_set(int unit, bcm_tsn_control_t type, uint32 arg)
{
#if defined(BCM_TSN_SUPPORT)
    if (soc_feature(unit, soc_feature_tsn)) {
        return bcmi_esw_tsn_control_set(unit, type, arg);
    }
#endif /* BCM_TSN_SUPPORT */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_esw_tsn_control_get
 * Description:
 *     Get the configure of device TSN operation modes.
 * Parameters:
 *     unit - (IN) Unit number.
 *     type - (IN) Control type of bcm_tsn_control_t.
 *     arg  - (OUT) argument got
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_esw_tsn_control_get(int unit, bcm_tsn_control_t type, uint32 *arg)
{
#if defined(BCM_TSN_SUPPORT)
    if (soc_feature(unit, soc_feature_tsn)) {
        return bcmi_esw_tsn_control_get(unit, type, arg);
    }
#endif /* BCM_TSN_SUPPORT */
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_control_set(int unit, bcm_gport_t port, bcm_tsn_control_t type,
                                 uint32 arg)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_control_get(int unit, bcm_gport_t port, bcm_tsn_control_t type,
                                 uint32 *arg)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_set(int unit, bcm_gport_t port, bcm_tsn_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_set32(int unit, bcm_gport_t port, bcm_tsn_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_get(int unit, bcm_gport_t port, bcm_tsn_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_get32(int unit, bcm_gport_t port, bcm_tsn_stat_t stat,
                                uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_multi_set(int unit, bcm_gport_t port, int nstat,
                                    bcm_tsn_stat_t *stat_arr, uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_multi_set32(int unit, bcm_gport_t port, int nstat,
                                      bcm_tsn_stat_t *stat_arr, uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_multi_get(int unit, bcm_gport_t port, int nstat,
                                    bcm_tsn_stat_t *stat_arr, uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_multi_get32(int unit, bcm_gport_t port, int nstat,
                                      bcm_tsn_stat_t *stat_arr, uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_sync_get(int unit, bcm_gport_t port, bcm_tsn_stat_t stat,
                                   uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_sync_get32(int unit, bcm_gport_t port, bcm_tsn_stat_t stat,
                                     uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_sync_multi_get(int unit, bcm_gport_t port, int nstat,
                                         bcm_tsn_stat_t *stat_arr, uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_port_stat_sync_multi_get32(int unit, bcm_gport_t port, int nstat,
                                           bcm_tsn_stat_t *stat_arr, uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_group_create(int unit, bcm_tsn_stat_group_type_t group_type,
                                  int count, bcm_tsn_stat_t *stat_arr,
                                  bcm_tsn_stat_group_t *id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_group_get(int unit, bcm_tsn_stat_group_t id,
                               bcm_tsn_stat_group_type_t *group_type, int max,
                               bcm_tsn_stat_t *stat_arr, int *count)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_group_set(int unit, bcm_tsn_stat_group_t id, int count,
                               bcm_tsn_stat_t *stat_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_group_destroy(int unit, bcm_tsn_stat_group_t id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_group_traverse(int unit, bcm_tsn_stat_group_traverse_cb cb,
                                    void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_group_set(int unit, bcm_tsn_sr_flow_t flow,
                                       bcm_tsn_stat_group_type_t group_type,
                                       bcm_tsn_stat_group_t stat_group)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_group_get(int unit, bcm_tsn_sr_flow_t flow,
                                       bcm_tsn_stat_group_type_t group_type,
                                       bcm_tsn_stat_group_t *stat_group)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_set(int unit, bcm_tsn_sr_flow_t flow, bcm_tsn_stat_t stat,
                                 uint64 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_set32(int unit, bcm_tsn_sr_flow_t flow, bcm_tsn_stat_t stat,
                                   uint32 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_get(int unit, bcm_tsn_sr_flow_t flow, bcm_tsn_stat_t stat,
                                 uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_get32(int unit, bcm_tsn_sr_flow_t flow, bcm_tsn_stat_t stat,
                                   uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_multi_set(int unit, bcm_tsn_sr_flow_t flow, int nstat,
                                       bcm_tsn_stat_t *stat_arr, uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_multi_set32(int unit, bcm_tsn_sr_flow_t flow, int nstat,
                                         bcm_tsn_stat_t *stat_arr, uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_multi_get(int unit, bcm_tsn_sr_flow_t flow, int nstat,
                                       bcm_tsn_stat_t *stat_arr, uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_multi_get32(int unit, bcm_tsn_sr_flow_t flow, int nstat,
                                         bcm_tsn_stat_t *stat_arr, uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_sync_get(int unit, bcm_tsn_sr_flow_t flow,
                                      bcm_tsn_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_sync_get32(int unit, bcm_tsn_sr_flow_t flow,
                                        bcm_tsn_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_sync_multi_get(int unit, bcm_tsn_sr_flow_t flow, int nstat,
                                            bcm_tsn_stat_t *stat_arr, uint64 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_sr_flow_stat_sync_multi_get32(int unit, bcm_tsn_sr_flow_t flow, int nstat,
                                              bcm_tsn_stat_t *stat_arr,
                                              uint32 *value_arr)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_threshold_set(int unit, bcm_tsn_stat_threshold_source_t source,
                                   bcm_tsn_stat_t stat,
                                   bcm_tsn_stat_threshold_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stat_threshold_get(int unit, bcm_tsn_stat_threshold_source_t source,
                                   bcm_tsn_stat_t stat,
                                   bcm_tsn_stat_threshold_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_event_register(int unit, bcm_tsn_event_type_t event,
                               bcm_tsn_event_source_t *src, bcm_tsn_event_cb cb,
                               void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_event_unregister(int unit, bcm_tsn_event_type_t event,
                                 bcm_tsn_event_source_t *src, bcm_tsn_event_cb cb)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_event_notification_traverse(int unit,
                                            bcm_tsn_event_notification_traverse_cb cb,
                                            void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_mtu_profile_create(int unit, bcm_tsn_mtu_profile_type_t type,
                                   bcm_tsn_mtu_config_t *config, int *mtu_profile_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_mtu_profile_get(int unit, int mtu_profile_id,
                                bcm_tsn_mtu_profile_type_t *type,
                                bcm_tsn_mtu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_mtu_profile_set(int unit, int mtu_profile_id,
                                bcm_tsn_mtu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_mtu_profile_destroy(int unit, int mtu_profile_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_mtu_profile_traverse(int unit, bcm_tsn_mtu_profile_traverse_cb cb,
                                     void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_ingress_mtu_config_set(int unit, bcm_tsn_ingress_mtu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_ingress_mtu_config_get(int unit, bcm_tsn_ingress_mtu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stu_profile_create(int unit, bcm_tsn_stu_profile_type_t type,
                                   bcm_tsn_stu_config_t *config, int *stu_profile_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stu_profile_get(int unit, int stu_profile_id,
                                bcm_tsn_stu_profile_type_t *type,
                                bcm_tsn_stu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stu_profile_set(int unit, int stu_profile_id,
                                bcm_tsn_stu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stu_profile_destroy(int unit, int stu_profile_id)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_stu_profile_traverse(int unit, bcm_tsn_stu_profile_traverse_cb cb,
                                     void *user_data)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_ingress_stu_config_set(int unit, bcm_tsn_ingress_stu_config_t *config)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_tsn_ingress_stu_config_get(int unit, bcm_tsn_ingress_stu_config_t *config)
{
    return BCM_E_UNAVAIL;
}
