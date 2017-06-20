/*
 * $Id: dev.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <shared/types.h>
#include <shared/et/osl.h>
#include <shared/et/aiutils.h>
#include <soc/error.h>
#include <soc/drv_if.h> 
#include "robo_lotus.h"
#include <soc/drv.h>



#define DEV_BCM53101_SINGAL_DETECTION_RETRY     2000


#define DEV_BCM53101_LOW_POWER_FREQ     12500000 /* 12.5 Mhz */
#define DEV_BCM53101_NORMAL_SPI_FREQ     20000000 /* MAX SPI freq support */


/*
 *  Function : drv_dev_prop_get
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
drv_lotus_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32 reg_val =0, temp = 0;

    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_LOTUS_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_LOTUS_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_LOTUS_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_LOTUS_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = NUM_COS(unit);
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_LOTUS_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_LOTUS_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_LOTUS_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_LOTUS_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_LOTUS_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_LOTUS_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_LOTUS_BPDU_NUM;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_LOTUS_AUTH_SEC_MODE;
            break;            
        case DRV_DEV_PROP_AGE_HIT_VALUE:
            *prop_val = 0x1;
            break;
        case DRV_DEV_PROP_LOW_POWER_ENABLE:
            /* Check the LOW POWER mode is enabled or not */
            SOC_IF_ERROR_RETURN(
                REG_READ_LOW_POWER_CTRLr(unit, &reg_val));
            soc_LOW_POWER_CTRLr_field_get(unit, &reg_val, 
                SYS_LOW_POWER_ENf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP:
            *prop_val = DRV_LOTUS_MAC_LOW_POWER_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_SUPPORTED_LED_FUNCTIONS:
            *prop_val = DRV_LED_FUNC_ALL_MASK & 
                    ~(DRV_LED_FUNC_PHYLED4 | DRV_LED_FUNC_SP_100 |
                    DRV_LED_FUNC_100_ACT | DRV_LED_FUNC_10_100_ACT |
                    DRV_LED_FUNC_PHYLED3);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_lotus_dev_prop_set
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
drv_lotus_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    int rv = SOC_E_NONE, i;
    uint32  reg_val = 0, temp = 0;

    switch(prop_type) {
        case DRV_DEV_PROP_LOW_POWER_ENABLE:
            if (prop_val) {
                if (prop_val != (DEV_BCM53101_LOW_POWER_FREQ / 1000)) {
                    return SOC_E_PARAM;
                }
                /* Enable LOW POWER MODE */      
                
                /* 1. Check the signal detection should be clear */
                /* It will take 1.5s from link-down to signal-detect deassertion */
                for (i=0; i < DEV_BCM53101_SINGAL_DETECTION_RETRY; i++) {
                    SOC_IF_ERROR_RETURN(
                        REG_READ_PHY_STSr(unit, &reg_val));
                    soc_PHY_STSr_field_get(unit, &reg_val, PHY_ENERGY_DETf, &temp);
                    if (!temp) {
                        break;
                    }
                    /* Check link status if any pulg-in */
                    SOC_IF_ERROR_RETURN(
                        REG_READ_LNKSTSr(unit, &reg_val));
                    if (reg_val & DRV_LOTUS_MAC_LOW_POWER_SUPPORT_PBMP) {
                        /* There is a link-up come from one of front ports.  */
                        /* Return it, didn't set to low power mode */
                        return SOC_E_FAIL;
                    }
                    sal_usleep(1000);
                }
                if (i == DEV_BCM53101_SINGAL_DETECTION_RETRY) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "The signal detection is not clear.\n")));
                    return SOC_E_TIMEOUT;
                }

                /* Adjust the SPI freq to 1/4 slower than system clock */
#if defined(KEYSTONE)
                SPI_LOCK;
                ai_soc_spi_freq_set(NULL, (DEV_BCM53101_LOW_POWER_FREQ /4));
                SPI_UNLOCK;
#endif

                /* 2. Shut down PHY */
                SOC_IF_ERROR_RETURN(
                    REG_READ_PHY_PWR_DOWNr(unit, &reg_val));
                temp = DRV_LOTUS_MAC_LOW_POWER_SUPPORT_PBMP;
                
                /* 2.1 Enable PHY Power Down */
                soc_PHY_PWR_DOWNr_field_set(unit, &reg_val, 
                    PHY_PWR_DOWN_PHY_ENf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));
                
                /* 2.2 Power Down PHY TX */
                soc_PHY_PWR_DOWNr_field_set(unit, &reg_val, 
                    PHY_PWR_DOWN_PHY_TXf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));

                 /* 2.3 Power Down PHY RX */
                soc_PHY_PWR_DOWNr_field_set(unit, &reg_val, 
                    PHY_PWR_DOWN_PHY_RXf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));

                /* 
                 * 3. Slow down system clock to 12.5Mhz 
                 *     and shut down MAC clocks for port 5 and port 8 
                 */
                SOC_IF_ERROR_RETURN(
                    REG_READ_LOW_POWER_CTRLr(unit, &reg_val));
                temp = 0x1;
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                    SYS_LOW_POWER_ENf, &temp);
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                    SLEEP_MACf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_LOW_POWER_CTRLr(unit, &reg_val));

                /* 4. Show down PLL */
                SOC_IF_ERROR_RETURN(
                    REG_READ_PLL_CTRLr(unit, &reg_val));
                temp = 0x1f;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_POWER_DOWN_CTRLf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PLL_CTRLr(unit, &reg_val));
                
            } else {
                /* Disable Low Power Mode */

                /* 1. Bring back PLL */
                SOC_IF_ERROR_RETURN(
                    REG_READ_PLL_CTRLr(unit, &reg_val));
                temp = 0x1;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_ARESETf, &temp);
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_DRESETf, &temp);
                temp = 0;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_POWER_DOWN_CTRLf, &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PLL_CTRLr(unit, &reg_val));
                
                reg_val = 0x0;
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PLL_CTRLr(unit, &reg_val));
                
                /* 2. Bring back system and MAC clocks */
                reg_val = 0;
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_LOW_POWER_CTRLr(unit, &reg_val));

                /* 3. Bring back PHY */
                /* 3.1 Disable PHY power down */
                reg_val = 0;
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));
                /* 3.2 Reset PHY , need wait for at least 400ns */
                reg_val = DRV_LOTUS_MAC_LOW_POWER_SUPPORT_PBMP;
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PHY_CTRLr(unit, &reg_val));
                sal_usleep(1);
                /* 3.3 PHY reset finished */
                /* Need wait at least 100us */
                reg_val = 0;
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PHY_CTRLr(unit, &reg_val));
                sal_usleep(100);

                MAC_LOW_POWER_LOCK(unit);
                /* Reset the timestamp */
                SOC_CONTROL(unit)->all_link_down_detected = 0;
                MAC_LOW_POWER_UNLOCK(unit);
                
                /* Adjust the SPI freq to 20 Mhz */
#if defined(KEYSTONE)
                SPI_LOCK;
                ai_soc_spi_freq_set(NULL, DEV_BCM53101_NORMAL_SPI_FREQ);
                SPI_UNLOCK;
#endif
                
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

