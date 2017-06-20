/*
 * $Id: dev.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/drv.h> 
#include <soc/feature.h>
#include <soc/robo/mcm/driver.h>
#include "robo_sf3.h"

/* EEE default values */
#define DEFAULT_EEE_BCM53134_SLEEP_DELAY_TIMER_G 4
#define DEFAULT_EEE_BCM53134_SLEEP_DELAY_TIMER_H 40
#define DEFAULT_EEE_BCM53134_WAKE_TRANS_TIMER_G 17
#define DEFAULT_EEE_BCM53134_WAKE_TRANS_TIMER_H 36
#define DEFAULT_EEE_BCM53134_MIN_LPI_TIMER_G 50
#define DEFAULT_EEE_BCM53134_MIN_LPI_TIMER_H 500
#define DEFAULT_EEE_BCM53134_PIPELINE_TIMER 32
#define DEFAULT_EEE_BCM53134_GLOBAL_CONG_THRESH 256
#define DEFAULT_EEE_BCM53134_TXQ02_CONG_THRESH 58
#define DEFAULT_EEE_BCM53134_TXQ35_CONG_THRESH 1

#define DEFAULT_EEE_BCM53134_TXQ6_CONG_THRESH 1
#define DEFAULT_EEE_BCM53134_TXQ7_CONG_THRESH 1

#define DEV_BCM53134_NORMAL_SPI_FREQ     20000000 /* MAX SPI freq support */

/*
 *  Function : drv_starfighter3_dev_prop_get
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
drv_starfighter3_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32 reg_val = 0, temp = 0;
    
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_SF3_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_SF3_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_SF3_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_SF3_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_SF3_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_SF3_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_SF3_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_SF3_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_SF3_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_SF3_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_SF3_BPDU_NUM;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            *prop_val = DRV_SF3_CFP_TCAM_SIZE;
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_SF3_CFP_UDFS_NUM;
            break;
         case DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX:
            *prop_val = DRV_SF3_CFP_UDFS_OFFSET_MAX;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_SF3_AUTH_SEC_MODE;
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
            *prop_val = DRV_SF3_MAC_LOW_POWER_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM:
            /* Port 8 connects to ARM processor */
            *prop_val = 8;
            break;
        case DRV_DEV_PROP_IMP1_PORT_NUM:
            /* Port 5 can be used for IMP1 */
            *prop_val = 5;
            break;
        case DRV_DEV_PROP_PPPOE_SESSION_ETYPE:
            SOC_IF_ERROR_RETURN(
                REG_READ_PPPOE_SESSION_PARSE_ENr(unit, &reg_val));
            soc_PPPOE_SESSION_PARSE_ENr_field_get(unit, &reg_val,
                PPPOE_SESSION_ETYPEf , &temp);
            *prop_val = temp;
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_starfighter3_dev_prop_set
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
drv_starfighter3_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32 reg_val = 0, temp = 0;
    soc_pbmp_t  pbmp;
    int i, rv = SOC_E_NONE;
    
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
        case DRV_DEV_PROP_EEE_INIT:
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_ASSIGN(pbmp, PBMP_GE_ALL(unit));
            /* per port */
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G, 
                DEFAULT_EEE_BCM53134_SLEEP_DELAY_TIMER_G));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H, 
                DEFAULT_EEE_BCM53134_SLEEP_DELAY_TIMER_H));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G, 
                DEFAULT_EEE_BCM53134_WAKE_TRANS_TIMER_G));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H, 
                DEFAULT_EEE_BCM53134_WAKE_TRANS_TIMER_H));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G, 
                DEFAULT_EEE_BCM53134_MIN_LPI_TIMER_G));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H, 
                DEFAULT_EEE_BCM53134_MIN_LPI_TIMER_H));
            /* per device */
            SOC_IF_ERROR_RETURN(
                drv_starfighter3_dev_prop_set(unit, 
                DRV_DEV_PROP_EEE_PIPELINE_TIMER, 
                DEFAULT_EEE_BCM53134_PIPELINE_TIMER));
            SOC_IF_ERROR_RETURN(
                drv_starfighter3_dev_prop_set(unit, 
                DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH, 
                DEFAULT_EEE_BCM53134_GLOBAL_CONG_THRESH));
            
            /* per queue */
            for (i = 0; i < 6; i++) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_TXQ_CONG_THr(unit, i, &reg_val));
                if ( i < 3) {
                    /* Queue 0 ~ 2 */
                    temp = DEFAULT_EEE_BCM53134_TXQ02_CONG_THRESH;
                } else {
                    /* Queue 3 ~ 5 */
                    temp = DEFAULT_EEE_BCM53134_TXQ35_CONG_THRESH;
                }
                soc_EEE_TXQ_CONG_THr_field_set(unit, &reg_val, 
                    TXQ_CONG_THf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_TXQ_CONG_THr(unit, i, &reg_val));
            }
            /* queue 6 */
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_TXQ_CONG_TH6r(unit, &reg_val));
            temp = DEFAULT_EEE_BCM53134_TXQ6_CONG_THRESH;
            soc_EEE_TXQ_CONG_TH6r_field_set(unit, &reg_val, 
                TXQ_CONG_THf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EEE_TXQ_CONG_TH6r(unit, &reg_val));
            
            /* queue 7 */
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_TXQ_CONG_TH7r(unit, &reg_val));
            temp = DEFAULT_EEE_BCM53134_TXQ7_CONG_THRESH;
            soc_EEE_TXQ_CONG_TH7r_field_set(unit, &reg_val, 
                TXQ_CONG_THf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EEE_TXQ_CONG_TH7r(unit, &reg_val));

            break;
        case DRV_DEV_PROP_PPPOE_SESSION_ETYPE:
            SOC_IF_ERROR_RETURN(
                REG_READ_PPPOE_SESSION_PARSE_ENr(unit, &reg_val));
            temp = prop_val;
            soc_PPPOE_SESSION_PARSE_ENr_field_set(unit, &reg_val,
                PPPOE_SESSION_ETYPEf , &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PPPOE_SESSION_PARSE_ENr(unit, &reg_val));
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

