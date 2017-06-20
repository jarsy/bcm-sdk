/*
 * $Id: dev.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h> 
#include "robo_bb2.h"
#include <soc/drv.h> 

/* EEE default values */
#define DEFAULT_EEE_BCM53128_SLEEP_DELAY_TIMER_G 4
#define DEFAULT_EEE_BCM53128_SLEEP_DELAY_TIMER_H 40
#define DEFAULT_EEE_BCM53128_WAKE_TRANS_TIMER_G 17
#define DEFAULT_EEE_BCM53128_WAKE_TRANS_TIMER_H 36
#define DEFAULT_EEE_BCM53128_MIN_LPI_TIMER_G 50
#define DEFAULT_EEE_BCM53128_MIN_LPI_TIMER_H 500
#define DEFAULT_EEE_BCM53128_PIPELINE_TIMER 32
#define DEFAULT_EEE_BCM53128_GLOBAL_CONG_THRESH 0x1A0
#define DEFAULT_EEE_BCM53128_TXQ02_CONG_THRESH 100
#define DEFAULT_EEE_BCM53128_TXQ35_CONG_THRESH 1

/* 
 * Save the EEE configuration before entering the MAC low power mode.
 * restore it when exiting the MAC low power mode
 */
static uint32   bcm53128_eee_enable_config[SOC_MAX_NUM_SWITCH_DEVICES]; 

int
_drv_blackbird2_station_move_drop_set(int unit, uint32 prop_val)
{
    int         rv = SOC_E_NONE;
    uint32      reg_value = 0, enable = 0;

    
    SOC_IF_ERROR_RETURN(
        REG_READ_GARLCFGr(unit, &reg_value));
    /* 
     * If the ARL control mode = 10 (BCM_L2_DISCARD_SRC), then
     * enable = 0 : Drop packet if SA match (Default, allow station movement) 
     * enable = 1 : Drop packet if SA match and port number not match 
     * (Not allow station movement)
     */
    if (prop_val) {
        enable = 1;
    } else {
        enable = 0;
    }
    soc_GARLCFGr_field_set(unit, &reg_value, SA_MOVE_DROPf, &enable);
    
    SOC_IF_ERROR_RETURN(
        REG_WRITE_GARLCFGr(unit, &reg_value));        

    return rv;
}

/*
 *  Function : drv_blackbird2_dev_prop_get
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
drv_blackbird2_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32 reg_val = 0, temp = 0;
    
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_BB2_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_BB2_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_BB2_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_BB2_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_BB2_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_BB2_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_BB2_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_BB2_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_BB2_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_BB2_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_BB2_BPDU_NUM;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            *prop_val = DRV_BB2_CFP_TCAM_SIZE;
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_BB2_CFP_UDFS_NUM;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_BB2_AUTH_SEC_MODE;
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
        case DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS:
            *prop_val = DRV_LED_FUNC_ALL_MASK & 
                    ~(DRV_LED_FUNC_SP_100_200 |  DRV_LED_FUNC_100_200_ACT | 
                    DRV_LED_FUNC_LNK_ACT_SP);
            break;
        case DRV_DEV_PROP_LOW_POWER_ENABLE:
            /* 
             * Slow down the system clock  
             *     and shut down MAC clocks for port 5 and port 8 
             */
            SOC_IF_ERROR_RETURN(
                REG_READ_LOW_POWER_CTRLr(unit, &reg_val));
            soc_LOW_POWER_CTRLr_field_get(unit, &reg_val, 
                EN_LOW_POWERf, &temp);
            if (temp == 0) {
                /* Disabled */
                *prop_val = 0;
            } else {
                soc_LOW_POWER_CTRLr_field_get(unit, &reg_val, 
                    LOW_POWER_DIVIDERf, &temp);
                switch (temp) {
                    case 0:
                        *prop_val = 12500;
                        break;
                    case 1:
                        *prop_val = 6250;
                        break;
                    case 2:
                        *prop_val = 4170;
                        break;
                    case 3:
                        *prop_val = 3125;
                        break;
                    default:
                        return SOC_E_INTERNAL;
                }
            }
            break;
        case DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP:
            if (SOC_IS_BLACKBIRD2_BOND_V(unit)) {
                *prop_val = DRV_BB2_MAC_LOW_POWER_SUPPORT_PBMP & 0x7f;
            } else {
                *prop_val = DRV_BB2_MAC_LOW_POWER_SUPPORT_PBMP;
            }
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_blackbird2_dev_prop_set
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
drv_blackbird2_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    int         rv = SOC_E_NONE;
    uint32 reg_val = 0, temp = 0;
    soc_pbmp_t  pbmp;
    int i;

    switch (prop_type) {
        case DRV_DEV_PROP_SA_STATION_MOVE_DROP:
            rv = _drv_blackbird2_station_move_drop_set(unit, prop_val);
            break;
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
        case DRV_DEV_PROP_EEE_INIT:
            SOC_PBMP_CLEAR(pbmp);
            SOC_PBMP_ASSIGN(pbmp, PBMP_GE_ALL(unit));
            /* per port */
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G, 
                DEFAULT_EEE_BCM53128_SLEEP_DELAY_TIMER_G));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H, 
                DEFAULT_EEE_BCM53128_SLEEP_DELAY_TIMER_H));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G, 
                DEFAULT_EEE_BCM53128_WAKE_TRANS_TIMER_G));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H, 
                DEFAULT_EEE_BCM53128_WAKE_TRANS_TIMER_H));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G, 
                DEFAULT_EEE_BCM53128_MIN_LPI_TIMER_G));
            SOC_IF_ERROR_RETURN(
                DRV_PORT_SET(unit, pbmp, 
                DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H, 
                DEFAULT_EEE_BCM53128_MIN_LPI_TIMER_H));
            /* per device */
            SOC_IF_ERROR_RETURN(
                drv_blackbird2_dev_prop_set(unit, 
                DRV_DEV_PROP_EEE_PIPELINE_TIMER, 
                DEFAULT_EEE_BCM53128_PIPELINE_TIMER));
            SOC_IF_ERROR_RETURN(
                drv_blackbird2_dev_prop_set(unit, 
                DRV_DEV_PROP_EEE_GLOBAL_CONG_THRESH, 
                DEFAULT_EEE_BCM53128_GLOBAL_CONG_THRESH));
            
            /* per queue */
            for (i = 0; i < 6; i++) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_TXQ_CONG_THr(unit, i, &reg_val));
                if ( i < 3) {
                    /* Queue 0 ~ 2 */
                    temp = DEFAULT_EEE_BCM53128_TXQ02_CONG_THRESH;
                } else {
                    /* Queue 3 ~ 5 */
                    temp = DEFAULT_EEE_BCM53128_TXQ35_CONG_THRESH;
                }
                soc_EEE_TXQ_CONG_THr_field_set(unit, &reg_val, 
                    TXQ_CONG_THf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_TXQ_CONG_THr(unit, i, &reg_val));
            }
            break;
        case DRV_DEV_PROP_LOW_POWER_ENABLE:

            INT_MCU_LOCK(unit);
            if (prop_val) {
                /* Save the current EEE configuration */
                rv = REG_READ_EEE_EN_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                soc_EEE_EN_CTRLr_field_get(unit, &reg_val, EN_EEEf, &temp);
                bcm53128_eee_enable_config[unit] = temp;
                /* Disable EEE */
                reg_val = 0;
                rv = REG_WRITE_EEE_EN_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }

                /* Disable IMP port */
                rv = REG_READ_IMP_CTLr(unit, CMIC_PORT(unit), &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }

                temp = 1;
                soc_IMP_CTLr_field_set(unit, &reg_val, TX_DISf, &temp);
                soc_IMP_CTLr_field_set(unit, &reg_val, RX_DISf, &temp);
                rv = REG_WRITE_IMP_CTLr(unit, CMIC_PORT(unit), &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                 /* 
                  * Slow down the system clock  
                  *     and shut down MAC clocks  
                  */
                rv = REG_READ_LOW_POWER_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                temp = 0x1;
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                    EN_LOW_POWERf, &temp);
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                    SLEEP_MAC_250f, &temp);
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val,
                    SLEEP_MAC_50f, &temp);
                switch (prop_val) {
                    case 12500:
                        temp = 0;
                        break;
                    case 6250:
                        temp = 1;
                        break;
                    case 4170:
                        temp = 2;
                        break;
                    case 3125:
                        temp = 3;
                        break;
                    default:
                        INT_MCU_UNLOCK(unit);
                        return SOC_E_PARAM;
                }
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                    LOW_POWER_DIVIDERf, &temp);
                rv = REG_WRITE_LOW_POWER_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }

                /* 4. Show down PLL */
                rv = REG_READ_PLL_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                temp = 0x1f;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_LP_CTRLf, &temp);
                rv = REG_WRITE_PLL_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
            } else {
                /* 1. Bring back PLL */
                rv = REG_READ_PLL_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                temp = 0x1;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_ARESETf, &temp);
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_DRESETf, &temp);
                temp = 0;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_LP_CTRLf, &temp);
                rv = REG_WRITE_PLL_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                
                reg_val = 0x0;
                rv = REG_WRITE_PLL_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                
                /* 2. Bring back system and MAC clocks */
                reg_val = 0;
                rv = REG_WRITE_LOW_POWER_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }

                /* Enable IMP port */
                rv = REG_READ_IMP_CTLr(unit, CMIC_PORT(unit), &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }

                temp = 0;
                soc_IMP_CTLr_field_set(unit, &reg_val, TX_DISf, &temp);
                soc_IMP_CTLr_field_set(unit, &reg_val, RX_DISf, &temp);
                rv = REG_WRITE_IMP_CTLr(unit, CMIC_PORT(unit), &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }

                /* Restore the configuration of EEE */
                reg_val = bcm53128_eee_enable_config[unit];
                /* if an is disabled, disable the EEE as well */
                PBMP_ITER(PBMP_GE_ALL(unit), i) {
                    if (i < SOC_ROBO_MAX_NUM_PORTS) {
                        rv = DRV_PORT_GET(unit, i,
                                DRV_PORT_PROP_AUTONEG, &temp);
                        if (SOC_FAILURE(rv)) {
                            INT_MCU_UNLOCK(unit);
                            return rv;
                        }
                        if (!temp) {
                            reg_val &= ~(0x1 << i);
                        }
                    }
                }
                
                rv = REG_WRITE_EEE_EN_CTRLr(unit, &reg_val);
                if (SOC_FAILURE(rv)) {
                    INT_MCU_UNLOCK(unit);
                    return rv;
                }
                
            }
            break;
            INT_MCU_UNLOCK(unit);
        default:
            rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

