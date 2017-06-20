/*
 * $Id: dev.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        robo.c
 * Purpose:
 * Requires:
 */
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/robo/mcm/driver.h>
#include <soc/error.h>
#include <soc/debug.h>
#ifdef BCM_NORTHSTARPLUS_SUPPORT
#include "../gex/northstarplus/robo_northstarplus.h"
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
#ifdef BCM_STARFIGHTER3_SUPPORT
#include "../gex/starfighter3/robo_sf3.h"
#endif /* BCM_STARFIGHTER3_SUPPORT */

/*
 * Function: 
 *	    drv_dev_control_set
 * Purpose:
 *	    Set the system basis management or control functions. Especially for 
 *      those functions which provides a simple on/off or enable/disable 
 *      option to control device's working feature.
 *      Also, this driver interface provide some kinds of flexibility for 
 *      designer on implementing the device management mode configuration 
 *      instead of creating a individual driver interface.
 * Parameters:
 *	    ctrl_cnt    - (IN/OUT) the number count to indicate how many pairs of 
 *                      control types and values.
 *                      >> Output the number of proceeded control set items.
 *      type_list   - (IN) control type list
 *      value_list  - (IN) control value list
 * Returns:
 *	    
 * Notes:
 *  1. the contorl type and value of the same list index must be matched as 
 *      a pair.
 */

int
drv_dev_control_set(int unit,uint32 *ctrl_cnt,
                uint32 *type_list,int *value_list)
{
    uint32  op_cnt = 0;
    int     i, rv = SOC_E_NONE;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_HARRIER_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_DINO8_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 reg_value = 0;
    uint32 fld_valud = 0;
#endif
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    soc_pbmp_t  pbmp;
    uint32      temp;
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

    /* Null check */
    if (op_cnt){
        assert(type_list && value_list);
    }
    
    for (i = 0; i < *ctrl_cnt; i++){

#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_HARRIER_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_DINO8_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        fld_valud = (value_list[i] == 0) ? 0 : 1;
#endif

        if (type_list[i] == DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN){
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_DINO16_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO(unit)) {

                SOC_IF_ERROR_RETURN(
                    REG_READ_RSV_MCAST_CTRLr(unit, &reg_value));
                soc_RSV_MCAST_CTRLr_field_set(unit, &reg_value, 
                    EN_RES_MUL_LEARNf, &fld_valud);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_RSV_MCAST_CTRLr(unit, &reg_value));

            } else {
                rv = SOC_E_UNAVAIL;
            }
#else 
            rv = SOC_E_UNAVAIL;
#endif
#if defined(BCM_HARRIER_SUPPORT)
        } else if (type_list[i] == DRV_DEV_CTRL_L2_USERADDR) {
            if (SOC_IS_HARRIER(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_GARLCFGr(unit, &reg_value));
                soc_GARLCFGr_field_set(unit, &reg_value, 
                        MPADDR_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_GARLCFGr(unit, &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
#endif  /* BCM_HARRIER_SUPPORT  */
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXULF){
            if (SOC_IS_HARRIER(unit) || SOC_IS_DINO(unit)) {
#if defined(BCM_HARRIER_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_set(unit, &reg_value, 
                        MIRX_UC_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
#endif /* BCM_HARRIER_SUPPORT | BCM_DINO8_SUPPORT | BCM_DINO16_SUPPORT */                
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_set(unit, &reg_value, 
                        RX_UCST_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
#endif /* VULCAN || BLACKBIRD || ... */               
                
            } else {
                rv = SOC_E_UNAVAIL;
            }
        
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXMLF){
            if (SOC_IS_HARRIER(unit) || SOC_IS_DINO(unit)) {
#if defined(BCM_HARRIER_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_set(unit, &reg_value, 
                        MIRX_MC_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
#endif /* BCM_HARRIER_SUPPORT | BCM_DINO8_SUPPORT | BCM_DINO16_SUPPORT */               
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_set(unit, &reg_value, 
                        RX_MCST_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(REG_WRITE_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        } else if (type_list[i] == DRV_DEV_CTRL_RATE_METER_PLUS_IPG) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_PBMP_CLEAR(pbmp);
                if (SOC_IS_NORTHSTARPLUS(unit)) {
#ifdef BCM_NORTHSTARPLUS_SUPPORT
                    SOC_PBMP_WORD_SET(pbmp, 0,
                        DRV_NORTHSTARPLUS_RATE_CONTROL_SUPPORT_PBMP);
#endif /* BCM_NORTHSTARPLUS_SUPPORT */
                } else if (SOC_IS_STARFIGHTER3(unit)) {
#ifdef BCM_STARFIGHTER3_SUPPORT
                    SOC_PBMP_WORD_SET(pbmp, 0,
                        DRV_SF3_RATE_CONTROL_SUPPORT_PBMP);
#endif /* BCM_STARFIGHTER3_SUPPORT */
                }
                temp = value_list[i];
                rv = DRV_RATE_CONFIG_SET(unit, pbmp, 
                    DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE, temp);
            } else {
                return SOC_E_UNAVAIL;
            }
        } else if (type_list[i] == DRV_DEV_CTRL_ARLBINFULL_TOCPU) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
                SOC_IF_ERROR_RETURN(
                        REG_READ_ARL_BIN_FULL_FWDr(unit, &reg_value));
                fld_valud = (value_list[i]) ? 1 : 0;
                soc_ARL_BIN_FULL_FWDr_field_set(unit, &reg_value, 
                        ARL_BIN_FULL_FWD_ENf , &fld_valud);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_ARL_BIN_FULL_FWDr(unit, &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else if (type_list[i] == DRV_DEV_CTRL_ARLBINFULL_CNT) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
                SOC_IF_ERROR_RETURN(
                        REG_READ_ARL_BIN_FULL_CNTRr(unit, &reg_value));
                fld_valud = value_list[i];
                soc_ARL_BIN_FULL_CNTRr_field_set(unit, &reg_value, 
                        ARL_BIN_FUL_CNTRf, &fld_valud);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_ARL_BIN_FULL_CNTRr(unit, &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else if (type_list[i] == DRV_DEV_CTRL_EGRESS_PPPOEDSCP_REMARK) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
                SOC_IF_ERROR_RETURN(
                        REG_READ_TRREG_CTRL1r(unit, &reg_value));
                fld_valud = value_list[i];
                soc_TRREG_CTRL1r_field_set(unit, &reg_value, 
                        PPPOE_DSCP_RMK_ENf, &fld_valud);
                SOC_IF_ERROR_RETURN(
                        REG_WRITE_TRREG_CTRL1r(unit, &reg_value));
            } else {
                rv = SOC_E_UNAVAIL;
            }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            if (type_list[i] < DRV_DEV_CTRL_CNT) {
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        }
        
        if (rv) {
            *ctrl_cnt = op_cnt;
            break;
            
        }
        op_cnt++;
    }
    
    return rv;

}

/*
 * Function: 
 *	    drv_dev_control_get
 * Purpose:
 *	    Get the system basis management or control functions. Especially for 
 *      those functions which provides a simple on/off or enable/disable 
 *      option to control device's working feature.
 *      Also, this driver interface provide some kinds of flexibility for 
 *      designer on implementing the device management mode configuration 
 *      instead of creating a individual driver interface.
 * Parameters:
 *	    ctrl_cnt    - (IN/OUT) the number count to indicate how many pairs of 
 *                      control types and values.
 *                      >> Output the number of proceeded control set items.
 *      type_list   - (IN) control type list
 *      value_list  - (OUT) control value list
 * Returns:
 *	    
 * Notes:
 *  1. the contorl type and value of the same list index must be matched as 
 *      a pair.
 */

int
drv_dev_control_get(int unit,uint32 *ctrl_cnt,
                uint32 *type_list,int *value_list)
{
    uint32  op_cnt = 0;
    uint32  field_val = 0;
    int     i, rv = SOC_E_NONE;
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_HARRIER_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_DINO8_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 reg_value = 0;
#endif

    /* Null check */
    if (op_cnt){
        assert(type_list && value_list);
    }
    
    for (i = 0; i < *ctrl_cnt; i++){
        if (type_list[i] == DRV_DEV_CTRL_RESERVED_MCAST_SA_LEARN){
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_DINO16_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_ROBO_ARCH_VULCAN(unit) || SOC_IS_DINO(unit)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_RSV_MCAST_CTRLr(unit, &reg_value));
                soc_RSV_MCAST_CTRLr_field_get(unit, &reg_value, 
                    EN_RES_MUL_LEARNf, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }

#else
            rv = SOC_E_UNAVAIL;
#endif
#if defined(BCM_HARRIER_SUPPORT)
        } else if (type_list[i] == DRV_DEV_CTRL_L2_USERADDR) {
            if (SOC_IS_HARRIER(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_GARLCFGr(unit, &reg_value));
                soc_GARLCFGr_field_get(unit, &reg_value, 
                        MPADDR_ENf, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
#endif  /* BCM_HARRIER_SUPPORT  */
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXULF){
            if (SOC_IS_HARRIER(unit) || SOC_IS_DINO(unit)) {
#if defined(BCM_HARRIER_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_get(unit, &reg_value, 
                        MIRX_UC_ENf, &field_val);
#endif /* BCM_HARRIER_SUPPORT | BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_get(unit, &reg_value, 
                        RX_UCST_ENf, &field_val);
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
        
        } else if (type_list[i] == DRV_DEV_CTRL_CPU_RXMLF){
            if (SOC_IS_HARRIER(unit) || SOC_IS_DINO(unit)) {
#if defined(BCM_HARRIER_SUPPORT) || defined(BCM_DINO8_SUPPORT) || \
    defined(BCM_DINO16_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_MII_PCTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_MII_PCTLr_field_get(unit, &reg_value, 
                        MIRX_MC_ENf, &field_val);
#endif /* BCM_HARRIER_SUPPORT | BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */              
            } else if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#if defined(BCM_VULCAN_SUPPORT) || defined(BCM_BLACKBIRD_SUPPORT) || \
    defined(BCM_LOTUS_SUPPORT) || defined(BCM_STARFIGHTER_SUPPORT) || \
    defined(BCM_BLACKBIRD2_SUPPORT) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT) || \
    defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(REG_READ_IMP_CTLr(unit, 
                        CMIC_PORT(unit), &reg_value));
                soc_IMP_CTLr_field_get(unit, &reg_value, 
                        RX_MCST_ENf, &field_val);
#endif                
            } else {
                rv = SOC_E_UNAVAIL;
            }
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        } else if (type_list[i] == DRV_DEV_CTRL_RATE_METER_PLUS_IPG) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                /* Get the configuration of port 0 */
                rv = DRV_RATE_CONFIG_GET(unit, 0, 
                    DRV_RATE_CONFIG_INGRESS_IPG_INCLUDE, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else if (type_list[i] == DRV_DEV_CTRL_ARLBINFULL_TOCPU) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
                SOC_IF_ERROR_RETURN(
                        REG_READ_ARL_BIN_FULL_FWDr(unit, &reg_value));
                soc_ARL_BIN_FULL_FWDr_field_get(unit, &reg_value, 
                        ARL_BIN_FULL_FWD_ENf , &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else if (type_list[i] == DRV_DEV_CTRL_ARLBINFULL_CNT) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
                SOC_IF_ERROR_RETURN(
                        REG_READ_ARL_BIN_FULL_CNTRr(unit, &reg_value));
                soc_ARL_BIN_FULL_CNTRr_field_get(unit, &reg_value, 
                        ARL_BIN_FUL_CNTRf , &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
        } else if (type_list[i] == DRV_DEV_CTRL_EGRESS_PPPOEDSCP_REMARK) {
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
                SOC_IF_ERROR_RETURN(
                        REG_READ_TRREG_CTRL1r(unit, &reg_value));
                soc_TRREG_CTRL1r_field_get(unit, &reg_value, 
                        PPPOE_DSCP_RMK_ENf, &field_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            if (type_list[i] < DRV_DEV_CTRL_CNT){
                rv = SOC_E_UNAVAIL;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        }
        value_list[i] = field_val;    

        if (rv){
            *ctrl_cnt = op_cnt;
            break;
            
        }
        op_cnt++;
    }

    return rv;
}


