/*
 * $Id: dev.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <shared/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/robo/mcm/driver.h>
#include "robo_polar.h"

#define DEV_POLAR_SINGAL_DETECTION_RETRY            2000
#define DEV_POLAR_FIRST_PORT_NUM_OF_QUAD_BR_PHY     0
#define DRV_POLAR_SINGLE_BR_PHY_PORT_NUM            4

/*
 *  Function : drv_polar_dev_prop_get
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
drv_polar_dev_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32 reg_val = 0, temp = 0;
    uint32  imp1_port_num;
    
    switch (prop_type) {
        case DRV_DEV_PROP_MCAST_NUM:
            *prop_val = DRV_POLAR_MCAST_GROUP_NUM;
            break;
        case DRV_DEV_PROP_AGE_TIMER_MAX_S:
            *prop_val = DRV_POLAR_AGE_TIMER_MAX;
            break;
        case DRV_DEV_PROP_TRUNK_NUM:
            *prop_val = DRV_POLAR_TRUNK_GROUP_NUM;
            break;
        case DRV_DEV_PROP_TRUNK_MAX_PORT_NUM:
            *prop_val = DRV_POLAR_TRUNK_MAX_PORT_NUM;
            break;
        case DRV_DEV_PROP_COSQ_NUM:
            *prop_val = DRV_POLAR_COS_QUEUE_NUM;
            break;
        case DRV_DEV_PROP_MSTP_NUM:
            *prop_val = DRV_POLAR_MSTP_GROUP_NUM;
            break;
        case DRV_DEV_PROP_SEC_MAC_NUM_PER_PORT:
            *prop_val = DRV_POLAR_SEC_MAC_NUM_PER_PORT;
            break;
        case DRV_DEV_PROP_COSQ_MAX_WEIGHT_VALUE:
            *prop_val = DRV_POLAR_COS_QUEUE_MAX_WEIGHT_VALUE;
            break;
        case DRV_DEV_PROP_AUTH_PBMP:
            *prop_val = DRV_POLAR_AUTH_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_RATE_CONTROL_PBMP:
            *prop_val = DRV_POLAR_RATE_CONTROL_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_VLAN_ENTRY_NUM:
            *prop_val = DRV_POLAR_VLAN_ENTRY_NUM;
            break;
        case DRV_DEV_PROP_BPDU_NUM:
            *prop_val = DRV_POLAR_BPDU_NUM;
            break;
        case DRV_DEV_PROP_CFP_TCAM_SIZE:
            *prop_val = DRV_POLAR_CFP_TCAM_SIZE;
            break;
        case DRV_DEV_PROP_CFP_UDFS_NUM:
            *prop_val = DRV_POLAR_CFP_UDFS_NUM;
            break;
         case DRV_DEV_PROP_CFP_UDFS_OFFSET_MAX:
            *prop_val = DRV_POLAR_CFP_UDFS_OFFSET_MAX;
            break;
        case DRV_DEV_PROP_AUTH_SEC_MODE:
            *prop_val = DRV_POLAR_AUTH_SEC_MODE;
            break;            
        case DRV_DEV_PROP_AGE_HIT_VALUE:
            *prop_val = 0x1;
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
            *prop_val = DRV_LED_FUNC_ALL_MASK & (~DRV_LED_FUNC_LNK_ACT_SP);
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
                    case 3:
                        *prop_val = 3125;
                        break;
                    default:
                        return SOC_E_INTERNAL;
                }
            }
            break;
        case DRV_DEV_PROP_LOW_POWER_SUPPORT_PBMP:
            *prop_val = DRV_POLAR_MAC_LOW_POWER_SUPPORT_PBMP;
            break;
        case DRV_DEV_PROP_LOW_POWER_PHY_CFG_RESTORE_PBMP:
            *prop_val = 0;
            SOC_IF_ERROR_RETURN(REG_READ_MODEL_IDr(unit, &reg_val));
            if (reg_val != BCM89500_MODEL_ID) {
                *prop_val |= (0x1 << DRV_POLAR_SINGLE_BR_PHY_PORT_NUM);
            }
            SOC_IF_ERROR_RETURN(REG_READ_STRAP_PIN_STATUSr(unit, &reg_val));
            if ((reg_val & 0x180) == 0x180) {
                SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));
                *prop_val |= (0x1 << imp1_port_num);
            }
            break;
        case DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM:
            /* Port 7 connects to ARM processor */
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
 *  Function : drv_polar_dev_prop_set
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
drv_polar_dev_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32 reg_val = 0, temp = 0;
    int rv = SOC_E_NONE, i;
    uint8   phy_addr;
    uint16  phy_data;
    uint32  imp1_port_num;
    
    switch (prop_type) {
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

        case DRV_DEV_PROP_ULTRA_LOW_POWER:
            /* Start power off sequence */
            reg_val = 0x8000;
            SOC_IF_ERROR_RETURN(REG_WRITE_IDDQ_CTRLr(unit, &reg_val));

            /* Set the power down register (bit 12 of expansion register at 0x0e) in the BR-PHY*/
            phy_addr = PORT_TO_PHY_ADDR(unit, 
                               DEV_POLAR_FIRST_PORT_NUM_OF_QUAD_BR_PHY);
            SOC_IF_ERROR_RETURN(
                soc_miim_read(unit, phy_addr, 0x17, &phy_data));
            phy_data = (phy_data & 0xF000) | 0xF0E;
            SOC_IF_ERROR_RETURN(
                soc_miim_write(unit, phy_addr, 0x17, phy_data));

            SOC_IF_ERROR_RETURN(
                soc_miim_read(unit, phy_addr, 0x15, &phy_data));
            phy_data = phy_data | 0x1000;      
            SOC_IF_ERROR_RETURN(
                soc_miim_write(unit, phy_addr, 0x15, phy_data));

            SOC_IF_ERROR_RETURN(REG_READ_MODEL_IDr(unit, &reg_val));
            if (reg_val != BCM89500_MODEL_ID) {
                phy_addr = PORT_TO_PHY_ADDR(unit, 
                                DRV_POLAR_SINGLE_BR_PHY_PORT_NUM);
                SOC_IF_ERROR_RETURN(
                    soc_miim_read(unit, phy_addr, 0x17, &phy_data));
                phy_data = (phy_data & 0xF000) | 0xF0E;
                SOC_IF_ERROR_RETURN(
                    soc_miim_write(unit, phy_addr, 0x17, phy_data));
                
                SOC_IF_ERROR_RETURN(
                    soc_miim_read(unit, phy_addr, 0x15, &phy_data));
                phy_data = phy_data | 0x1000;      
                SOC_IF_ERROR_RETURN(
                    soc_miim_write(unit, phy_addr, 0x15, phy_data));
            }

            /* Finish power off sequence */
            reg_val = 0x8000;
            SOC_IF_ERROR_RETURN(REG_WRITE_IDDQ_CTRLr(unit, &reg_val));
            break;
        case DRV_DEV_PROP_LOW_POWER_ENABLE:
            if (prop_val) {
                /* 1. Check the signal detection should be clear */
                for (i=0; i < DEV_POLAR_SINGAL_DETECTION_RETRY; i++) {
                    rv = REG_READ_ENG_DET_STSr(unit, &reg_val);
                    if (!reg_val) {
                        break;
                    }
                    /* Check link status if any pulg-in */
                    SOC_IF_ERROR_RETURN(
                        REG_READ_LNKSTSr(unit, &reg_val));
                    if (reg_val & DRV_POLAR_MAC_LOW_POWER_SUPPORT_PBMP) {
                        /* There is a link-up come from one of front ports.  */
                        /* Return it, didn't set to low power mode */
                        return SOC_E_FAIL;
                    }
                    sal_usleep(1000);
                }
                if (i == DEV_POLAR_SINGAL_DETECTION_RETRY) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "The signal detection is not clear.\n")));
                    return SOC_E_TIMEOUT;
                }

                /* 1. Set single BR-PHY to IDDQ_SD mode (not for BCM89500)  */
                SOC_IF_ERROR_RETURN(REG_READ_MODEL_IDr(unit, &reg_val));
                if (reg_val != BCM89500_MODEL_ID) {
                    /* Enable passive termination for single BR-PHY */
                    phy_addr = PORT_TO_PHY_ADDR(unit, 
                                       DRV_POLAR_SINGLE_BR_PHY_PORT_NUM);
                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x17, &phy_data));
                    phy_data = (phy_data & 0xF000) | 0xFFC;
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x17, phy_data));

                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x15, &phy_data));
                    phy_data = phy_data | 0xBC00;      
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x15, phy_data));

                    /* Set P4 to IDDQ_SD */
                    SOC_IF_ERROR_RETURN(
                        REG_READ_PHY_PWR_DOWNr(unit, &reg_val));
                    temp = 0x10;
                    soc_PHY_PWR_DOWNr_field_set(unit, &reg_val, 
                                                EXT_PWR_DOWN_PHY_ENf, &temp);
                    
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));
                }

                /* 2. Set Quad BR-PHYs to standby mode */
                SOC_IF_ERROR_RETURN(REG_READ_PHY_PWR_DOWNr(unit, &reg_val));

                soc_PHY_PWR_DOWNr_field_get(unit, &reg_val, 
                                            EXT_PWR_DOWN_PHY_ENf, &temp);
                temp |= 0xf;
                soc_PHY_PWR_DOWNr_field_set(unit, &reg_val, 
                                            EXT_PWR_DOWN_PHY_ENf, &temp);
                temp = 0xf;
                soc_PHY_PWR_DOWNr_field_set(unit, &reg_val, 
                                            EN_AUTO_RECOVER_STDf, &temp);

                SOC_IF_ERROR_RETURN(REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));

                /* 3. Set serdes to power down  mode (for SGMII mode only) */
                SOC_IF_ERROR_RETURN(
                    REG_READ_STRAP_PIN_STATUSr(unit, &reg_val));
                if ((reg_val & 0x180) == 0x180) {
                    /*  Port 5 is SGMII interface */
                    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                        (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));

                    /* Set serdes block address  to 3 */
                    phy_addr = PORT_TO_PHY_ADDR_INT(unit, imp1_port_num);
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x1f, 0x3));

                    /* Power down analog TX channel */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x10, &phy_data));
                    phy_data = phy_data | 0x1;
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x10, phy_data));
                    
                    /* Power down analog RX channel */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x13, &phy_data));
                    phy_data = phy_data | 0x1;
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x13, phy_data));

                    /* Power down PLL */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x18, &phy_data));
                    phy_data = phy_data | 0x1;
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x18, phy_data));

                    /* Set serdes block address  to 0 */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x1f, 0x0));

                    /* Power down digital circuit */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x0, &phy_data));
                    phy_data = phy_data | 0x800;
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x0, phy_data));
                }

                /* 4. Set MIIx interfaces to sleep mode & Set switch core to lower power speed */
                SOC_IF_ERROR_RETURN(REG_READ_LOW_POWER_CTRLr(unit, &reg_val));
                temp = 0x1;
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                                                EN_LOW_POWERf, &temp);
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                                                SLEEP_P4f, &temp);
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val,
                                                SLEEP_P5f, &temp);
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val,
                                                SLEEP_P8f, &temp);
                switch (prop_val) {
                    case 12500:
                        temp = 0;
                        break;
                    case 6250:
                        temp = 1;
                        break;
                    case 3125:
                        temp = 3;
                        break;
                    default:
                        return SOC_E_PARAM;
                }
                soc_LOW_POWER_CTRLr_field_set(unit, &reg_val, 
                    LOW_POWER_DIVIDERf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_LOW_POWER_CTRLr(unit, &reg_val));

                /* 5. Power down switch PLL */
                SOC_IF_ERROR_RETURN(REG_READ_PLL_CTRLr(unit, &reg_val));
                temp = 0x1f;
                soc_PLL_CTRLr_field_set(unit, &reg_val, PLL_LP_CTRLf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_PLL_CTRLr(unit, &reg_val));
            } else {
                /* 1. Enable switch PLL */
                reg_val = 0x0;
                temp = 0x1;
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_ARESETf, &temp);
                soc_PLL_CTRLr_field_set(unit, &reg_val, 
                    PLL_DRESETf, &temp);
                SOC_IF_ERROR_RETURN(REG_WRITE_PLL_CTRLr(unit, &reg_val));
                
                reg_val = 0x0;
                SOC_IF_ERROR_RETURN(REG_WRITE_PLL_CTRLr(unit, &reg_val));
                
                /* 2. Set switch core to full speed state & Bring back MIIx interfaces */
                reg_val = 0;
                SOC_IF_ERROR_RETURN(REG_WRITE_LOW_POWER_CTRLr(unit, &reg_val));

                /* 3. Bring back single BR-PHY (P4) (not for BCM89500) */
                SOC_IF_ERROR_RETURN(REG_READ_MODEL_IDr(unit, &reg_val));
                if (reg_val != BCM89500_MODEL_ID) {

                    /* Disable PHY IDDQ_SD mode */
                    reg_val = 0;
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_PHY_PWR_DOWNr(unit, &reg_val));

                    /* Reset single BR-PHY */
                    reg_val = 0;
                    temp = 0x1;
                    soc_PHY_CTRLr_field_set(unit, &reg_val, 
                        SPHY_RESETf, &temp);
                    SOC_IF_ERROR_RETURN(REG_WRITE_PHY_CTRLr(unit, &reg_val));

                    /* Clear BR-PHY reset */
                    reg_val = 0;
                    SOC_IF_ERROR_RETURN(REG_WRITE_PHY_CTRLr(unit, &reg_val));
                }

                /* 4. Bring back serdes and Re-initialization (for SGMII mode only) */
                SOC_IF_ERROR_RETURN(
                    REG_READ_STRAP_PIN_STATUSr(unit, &reg_val));
                if ((reg_val & 0x180) == 0x180) {
                    /*  Port 5 is SGMII interface */
                    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                        (unit, DRV_DEV_PROP_IMP1_PORT_NUM, &imp1_port_num));

                    phy_addr = PORT_TO_PHY_ADDR_INT(unit, imp1_port_num);
                    /* Set serdes block address to 0 */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x1f, 0x0));

                    /* Toggle self-clear software reset */
                    SOC_IF_ERROR_RETURN(
                        soc_miim_read(unit, phy_addr, 0x0, &phy_data));
                    phy_data = phy_data | 0x8000;
                    SOC_IF_ERROR_RETURN(
                        soc_miim_write(unit, phy_addr, 0x0, phy_data));
                }

                MAC_LOW_POWER_LOCK(unit);
                /* Reset the timestamp */
                SOC_CONTROL(unit)->all_link_down_detected = 0;
                SOC_CONTROL(unit)->ultra_low_power_detected = 0;
                MAC_LOW_POWER_UNLOCK(unit);
            }
            break;
        default:
            return SOC_E_UNAVAIL;
    }
    return SOC_E_NONE;
}

