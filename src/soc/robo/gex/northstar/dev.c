/*
 * $Id: dev.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/robo/mcm/driver.h>
#include "robo_northstar.h"

/*
 *  Function : drv_northstar_dev_prop_get
 *
 *  Purpose :
 *      Get the device property information
 *
 *  Parameters :
 *      unit        :   unit id
 *      prop_type   :   property type
 *      prop_val     :   property value of the property type
 *
 *  Return :
 *      SOC_E_NONE      :   success
 *      SOC_E_PARAM    :   parameter error
 *
 *  Note :
 *      This function is to get the device porperty information.
 *
 */
int 
drv_northstar_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32 reg_val = 0, temp = 0;
    
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_NORTHSTAR_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_NORTHSTAR_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_NORTHSTAR_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_NORTHSTAR_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = DRV_NORTHSTAR_COS_QUEUE_NUM;
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_NORTHSTAR_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_NORTHSTAR_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_NORTHSTAR_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_NORTHSTAR_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_NORTHSTAR_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_NORTHSTAR_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_NORTHSTAR_BPDU_NUM;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            *prop_val = DRV_NORTHSTAR_CFP_TCAM_SIZE;
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_NORTHSTAR_CFP_UDFS_NUM;
            break;
         case DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX:
            *prop_val = DRV_NORTHSTAR_CFP_UDFS_OFFSET_MAX;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_NORTHSTAR_AUTH_SEC_MODE;
            break;            
        case DRV_DEV_PROP_AGE_HIT_VALUE:
            *prop_val = 0x1;
            break;
        case DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_GLB_CONG_THr(unit, &reg_val));
            soc_EEE_GLB_CONG_THr_field_get(unit, &reg_val, 
                GLB_CONG_THf, &temp);
            *prop_val = temp;
            break;
        case DRV_DEV_PROP_EEE_PIPELINE_TIMER:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_PIPELINE_TIMERr(unit, &reg_val));
            soc_EEE_PIPELINE_TIMERr_field_get(unit, &reg_val, 
                PIPELINE_TIMERf, &temp);
            *prop_val = temp;
            break;
        case DRV_DEV_PROP_RESOURCE_ARBITER_REQ:
            SOC_IF_ERROR_RETURN(
                REG_READ_CPU_RESOURCE_ARBITERr(unit, &reg_val));
            soc_CPU_RESOURCE_ARBITERr_field_get(unit, &reg_val, 
                        EXT_CPU_REQf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS:
            *prop_val = DRV_LED_FUNC_ALL_MASK & 
                    ~(DRV_LED_FUNC_SP_100_200 |  DRV_LED_FUNC_100_200_ACT | 
                    DRV_LED_FUNC_LNK_ACT_SP);
            break;
        case DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP:
            *prop_val = DRV_NORTHSTAR_MAC_LOW_POWER_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM:
            /* Port 8 connects to ARM processor */
            *prop_val = 8;
            break;
        case DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM:
            /* Port 7 connects to SOC */
            *prop_val = 7;
            break;
        case DRV_DEV_PROP_IMP1_PORT_NUM:
            /* Port 5 can be used for IMP1 */
            *prop_val = 5;
            break;
        case DRV_DEV_PROP_SCH2_OUTPUT_COSQ:
            /* COSQ ID for output of SCH2 */
            *prop_val = 6;
            break;
        case DRV_DEV_PROP_SCH1_NUM_COSQ:
            /* Number of COSQ for SCH1 */
            *prop_val = 3;
            break;
        case DRV_DEV_PROP_SCH2_NUM_COSQ:
            /* Number of COSQ for SCH2 */
            *prop_val = 4;
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_northstar_dev_prop_set
 *
 *  Purpose :
 *     Set the device property information
 *
 *  Parameters :
 *      unit        :   unit id
 *      prop_type   :   property type
 *      prop_val     :   property value of the property type
 *
 *  Return :
 *      SOC_E_UNAVAIL 
 *
 *  Note :
 *      This function is to set the device porperty information.
 *
 */
int 
drv_northstar_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32 reg_val = 0, temp = 0;
    int rv = SOC_E_NONE;
    
    switch (prop_type) {
        case DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH:
            if (prop_val > 512) {
                return SOC_E_CONFIG;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_GLB_CONG_THr(unit, &reg_val));
            temp = prop_val;
            soc_EEE_GLB_CONG_THr_field_set(unit, &reg_val, 
                GLB_CONG_THf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EEE_GLB_CONG_THr(unit, &reg_val));
            break;
        case DRV_DEV_PROP_EEE_PIPELINE_TIMER:
            /* 16-bit length */
            if (prop_val > 65536) {
                return SOC_E_CONFIG;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_PIPELINE_TIMERr(unit, &reg_val));
            temp = prop_val;
            soc_EEE_PIPELINE_TIMERr_field_set(unit, &reg_val, 
                PIPELINE_TIMERf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EEE_PIPELINE_TIMERr(unit, &reg_val));
            break;
        case DRV_DEV_PROP_RESOURCE_ARBITER_REQ:
            if (!soc_feature(unit, soc_feature_int_cpu_arbiter)) {
                return SOC_E_UNAVAIL;
            }
            SOC_IF_ERROR_RETURN(
                REG_READ_CPU_RESOURCE_ARBITERr(unit, &reg_val));
            sal_mutex_take(
                SOC_CONTROL(unit)->arbiter_mutex, sal_mutex_FOREVER);
            if (prop_val) {
                if (SOC_CONTROL(unit)->arbiter_lock_count == 0) {
                    /* Require an arbiter grant */
                    temp = 1;
                    soc_CPU_RESOURCE_ARBITERr_field_set(unit, &reg_val, 
                        EXT_CPU_REQf, &temp);
                    rv = REG_WRITE_CPU_RESOURCE_ARBITERr(unit, &reg_val);
                    if (SOC_FAILURE(rv)) {
                        sal_mutex_give(SOC_CONTROL(unit)->arbiter_mutex);
                        return rv;
                    }
 
                    /* wait for grant */
                    do {
                        rv = REG_READ_CPU_RESOURCE_ARBITERr(unit, &reg_val);
                        if (SOC_FAILURE(rv)) {
                            sal_mutex_give(SOC_CONTROL(unit)->arbiter_mutex);
                            return rv;
                        }
                        soc_CPU_RESOURCE_ARBITERr_field_get(unit, &reg_val, 
                            EXT_CPU_GNTf, &temp);
                    } while (temp == 0);
                }
                SOC_CONTROL(unit)->arbiter_lock_count++;
                
            } else {
                /* Release the arbiter grant */
                SOC_CONTROL(unit)->arbiter_lock_count--;
                if (SOC_CONTROL(unit)->arbiter_lock_count == 0) {
                    temp = 0;
                    soc_CPU_RESOURCE_ARBITERr_field_set(unit, &reg_val, 
                        EXT_CPU_REQf, &temp);
                    rv = REG_WRITE_CPU_RESOURCE_ARBITERr(unit, &reg_val);
                    if (SOC_FAILURE(rv)) {
                        sal_mutex_give(SOC_CONTROL(unit)->arbiter_mutex);
                        return rv;
                    }
                }
            }
            sal_mutex_give(SOC_CONTROL(unit)->arbiter_mutex);
            break;
        case DRV_DEV_PROP_LOW_POWER_ENABLE:
        default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

