/*
 * $Id: policer.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/scache.h>
#include <soc/mem.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/policer.h>
#if defined(BCM_GREYHOUND2_SUPPORT)
#include <bcm_int/esw/greyhound2.h>

/*
 * Function:
 *   bcmi_gh2_global_meter_ifg_set
 * Purpose:
 *   Enable/Disable Inter Frame Gap(IFG)
 * Parameters:
 *   unit - (IN) unit number
 *   ifg_enable - (IN) Enable/disable
 * Returns:
 *   BCM_E_XXX
 */
int bcmi_gh2_global_meter_ifg_set(
    int unit,
    uint8 ifg_enable)
{
    uint32 rval = 0;
    SOC_IF_ERROR_RETURN(READ_SVM_TSN_CONTROLr(unit, &rval));
    soc_reg_field_set(unit,
                      SVM_TSN_CONTROLr,
                      &rval,
                      SVM_PACKET_IFG_ENf,
                      ifg_enable);
    SOC_IF_ERROR_RETURN(WRITE_SVM_TSN_CONTROLr(unit, rval));
    return BCM_E_NONE;
}

/*
 * Function:
 *   bcmi_gh2_global_meter_ifg_get
 * Purpose:
 *   Get Enable/Disable Inter Frame Gap(IFG)
 * Parameters:
 *   unit - (IN) unit number
 *   ifg_enable - (OUT) Enable/disable
 * Returns:
 *   BCM_E_XXX
 */
int bcmi_gh2_global_meter_ifg_get(
    int unit,
    uint8 *ifg_enable)
{
    uint32 rval = 0;

    if (NULL == ifg_enable) {
        return BCM_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(READ_SVM_TSN_CONTROLr(unit, &rval));
    *ifg_enable = (uint8)soc_reg_field_get(unit,
                                           SVM_TSN_CONTROLr,
                                           rval,
                                           SVM_PACKET_IFG_ENf);
    return BCM_E_NONE;
}

/*
 * Function:
 *   bcmi_gh2_global_meter_source_order_set
 * Purpose:
 *   Configure the priority of VFP/L2/VXLT/VLAN/Port in the case of
 *   conflict source happened.
 * Parameters:
 *   unit - (IN) unit number
 *   config - (IN) Service Metering Source Table Priority Config
 *       The lower index of meter source in config array has higher priority
 *   source_order_count - (IN) Valid index count in source_order
 * Returns:
 *   BCM_E_XXX
 */
int bcmi_gh2_global_meter_source_order_set(
    int unit,
    bcm_policer_global_meter_source_t *source_order,
    uint32 source_order_count)
{
    int priority;
    bcm_policer_global_meter_source_t source;
    soc_field_t source_field = INVALIDf;

    if (NULL == source_order) {
        return BCM_E_PARAM;
    }

    for (source = 0; source < source_order_count; source++) {
        /* highest value will have the highest priority */
        priority = source_order_count - source - 1;
        switch (source) {
            case bcmPolicerGlobalMeterSourceFieldStageLookup:
                source_field = VFP_TABLEf;
                break;
            case bcmPolicerGlobalMeterSourceDstMac:
                source_field = L2_TABLE_DA_LOOKUPf;
                break;
            case bcmPolicerGlobalMeterSourceSrcMac:
                source_field = L2_TABLE_SA_LOOKUPf;
                break;
            case bcmPolicerGlobalMeterSourceVlanTranslateFirst:
                source_field = VXLT_TABLE_KEY1f;
                break;
            case bcmPolicerGlobalMeterSourceVlanTranslateSecond:
                source_field = VXLT_TABLE_KEY2f;
                break;
            case bcmPolicerGlobalMeterSourceVlan:
                source_field = VLAN_TABLEf;
                break;
            case bcmPolicerGlobalMeterSourcePort:
                source_field = LPORT_TABLEf;
                /* need to write LPORT_TABLEf and PORT_TABLEf */
                SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit,
                    METER_SOURCE_PRIORITYr,
                    REG_PORT_ANY,
                    PORT_TABLEf,
                    priority));
                break;
            default:
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Invalid source(%d).\n"),
                                      source));
                return BCM_E_PARAM;
        }
        SOC_IF_ERROR_RETURN(soc_reg_field32_modify(unit,
            METER_SOURCE_PRIORITYr,
            REG_PORT_ANY,
            source_field,
            priority));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *    bcmi_gh2_global_meter_source_order_get
 * Purpose:
 *   Get the priority of VFP/L2/VXLT/VLAN/Port in the case of
 *   conflict source happened.
 * Parameters:
 *   unit - (IN) unit number
 *   source_order_count - (IN) Valid index count in source_order
 *   config - (OUT) Service Metering Source Table Priority Config
 * Returns:
 *   BCM_E_XXX
 */
int bcmi_gh2_global_meter_source_order_get(
    int unit,
    bcm_policer_global_meter_source_t *source_order,
    uint32 source_order_count)
{
    uint32 rval = 0;
    soc_field_t source_field = INVALIDf;
    int priority;
    bcm_policer_global_meter_source_t source;

    if (NULL == source_order) {
        return BCM_E_PARAM;
    }

    for (source = 0; source < source_order_count; source++) {
        switch (source) {
            case bcmPolicerGlobalMeterSourceFieldStageLookup:
                source_field = VFP_TABLEf;
                break;
            case bcmPolicerGlobalMeterSourceDstMac:
                source_field = L2_TABLE_DA_LOOKUPf;
                break;
            case bcmPolicerGlobalMeterSourceSrcMac:
                source_field = L2_TABLE_SA_LOOKUPf;
                break;
            case bcmPolicerGlobalMeterSourceVlanTranslateFirst:
                source_field = VXLT_TABLE_KEY1f;
                break;
            case bcmPolicerGlobalMeterSourceVlanTranslateSecond:
                source_field = VXLT_TABLE_KEY2f;
                break;
            case bcmPolicerGlobalMeterSourceVlan:
                source_field = VLAN_TABLEf;
                break;
            case bcmPolicerGlobalMeterSourcePort:
                source_field = PORT_TABLEf;
                break;
            default:
                LOG_ERROR(BSL_LS_BCM_POLICER,
                          (BSL_META_U(unit,
                                      "Invalid source(%d).\n"),
                                      source));
                return BCM_E_PARAM;
        }
        SOC_IF_ERROR_RETURN(READ_METER_SOURCE_PRIORITYr(unit, &rval));
        priority = soc_reg_field_get(unit,
                                     METER_SOURCE_PRIORITYr,
                                     rval,
                                     source_field);
        source_order[source] = priority;
    }
    return BCM_E_NONE;
}

STATIC const bcmi_global_meter_dev_info_t bcmi_gh2_global_meter_dev_info = {
    "gh2_global_meter_dev_info",
    bcmi_gh2_global_meter_ifg_set,
    bcmi_gh2_global_meter_ifg_get,
    bcmi_gh2_global_meter_source_order_set,
    bcmi_gh2_global_meter_source_order_get,
};

/*
 * Function:
 *     bcmi_gh2_global_meter_dev_info_init
 * Description:
 *     Setup the chip specific info.
 * Parameters:
 *     unit    - (IN) Unit number.
 *     devinfo - (OUT) device info structure.
 * Returns:
 *     BCM_E_XXX
 */
int
bcmi_gh2_global_meter_dev_info_init(
    int unit,
    const bcmi_global_meter_dev_info_t **dev_info)
{
    if (dev_info == NULL) {
        return BCM_E_PARAM;
    }

    /* Return the chip info structure */
    *dev_info = &bcmi_gh2_global_meter_dev_info;
    return BCM_E_NONE;
}
#endif /* BCM_GREYHOUND2_SUPPORT */

