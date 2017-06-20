/*
 * $Id: port.c,v 1.23 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#include <shared/bsl.h>

#include <shared/error.h>
#include <soc/robo.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/robo/mcm/driver.h>
#include <soc/phy.h>
#include <soc/phy/phyctrl.h>
#include <soc/phyreg.h>
#include "robo_gex.h"
#include "../common/robo_common.h"


#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined (BCM_STARFIGHTER3_SUPPORT)
/* SA overlimit action in NSP are NONE/DROP/COPY2CPU/REDIRECT2CPU 
 *  
 *  The HW Spec. can be use two bits to indicate all action.
 *      - bit 0 for drop action : 01b(NONE) and 01b(DROP)
 *      - bit 1 for to CPU action : 10b(COPY2CPU) and 11b(REDIRECT2CPU)
 */
#define _DRV_SA_OVERLIMIT_ACTION_NONE   (0x0)
#define _DRV_SA_OVERLIMIT_ACTION_DROP   (0x1 << 0) 
#define _DRV_SA_OVERLIMIT_ACTION_CPU    (0x1 << 1) 
#define _DRV_SA_OVERLIMIT_ACTION_MASK    (0x3) 
#endif /* BCM_NORTHSTARPLUS_SUPPORT */

/*
 *  Function : _drv_gex_port_property_enable_set
 *
 *  Purpose :
 *      Set the port based properties enable/disable.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      enable  :   enable/disable
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_gex_port_property_enable_set(int unit, int port, int property, uint32 enable)
{
    uint32    addr = 0, temp = 0, vlan_ctrl3;
    int        rv = SOC_E_NONE;
    int         length = 0;
    uint32    reg_index = 0, fld_index = 0;
    uint32    reg_value;
    soc_pbmp_t t_bmp;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */

    switch (property) {
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
        case DRV_PORT_PROP_SA_OVERLIMIT_DROP:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
        case DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            /* those properties is for Learn limit action setting only */

            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (port == 7){
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_7_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else if (port == 8) {
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_8_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_N_SA_LIMIT_CTLr(unit, 
                            port, &reg_value));
                }
                /* 
                 *  request : original->result
                 *  ========================================
                 *  drop_true : (00b->01b / 01b->01b / 10b->11b / 11b->11b)
                 *  drop_false : (00b->00b / 01b->00b / 10b->10b / 11b->10b)
                 *  cpu_true : (00b->10b / 01b->11b / 10b->10b / 11b->11b)
                 *  cpu_false : (00b->00b / 01b->01b / 10b->00b / 11b->01b)
                 *  none : (NA->00b)
                 *  ========================================
                 */
                if (property == DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE) {
                    temp = 0;
                } else {
                    SOC_IF_ERROR_RETURN(
                            soc_PORT_N_SA_LIMIT_CTLr_field_get(unit, &reg_value, 
                            OVER_LIMIT_ACTIONSf, &temp));
                    if ((property == DRV_PORT_PROP_SA_OVERLIMIT_DROP) ||
                        (property == DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP)){
                        if (enable) {
                            temp |= _DRV_SA_OVERLIMIT_ACTION_DROP;
                        } else {
                            temp &= ~_DRV_SA_OVERLIMIT_ACTION_DROP;
                        }
                    } else {
                        /* to CPU action */
                        if (enable) {
                            temp |= _DRV_SA_OVERLIMIT_ACTION_CPU;
                        } else {
                            temp &= ~_DRV_SA_OVERLIMIT_ACTION_CPU;
                        }
                    }
                    temp &= _DRV_SA_OVERLIMIT_ACTION_MASK;
                    SOC_IF_ERROR_RETURN(
                            soc_PORT_N_SA_LIMIT_CTLr_field_set(unit, &reg_value, 
                            OVER_LIMIT_ACTIONSf, &temp));
                }
                if (port == 7){
                    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_7_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else if (port == 8) {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_8_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_N_SA_LIMIT_CTLr(unit, 
                            port, &reg_value));
                }
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_LOTUS(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MIRX_DISf, &temp);
                if ((rv = REG_WRITE_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_LOTUS(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit)) 
                    && (port == specified_port_num)) {
                    if ((rv = REG_READ_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }

                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MIRX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        RX_DISf, &temp);
                }
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit)) 
                    && (port == specified_port_num)) {
                    if ((rv = REG_WRITE_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_LOTUS(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MITX_DISf, &temp);
                if ((rv = REG_WRITE_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_LOTUS(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit)) 
                    && (port == specified_port_num)) {
                    if ((rv = REG_READ_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT 
        */
                {
                    if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MITX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        TX_DISf, &temp);
                }
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit))
                    && (port == specified_port_num)) {
                    if ((rv = REG_WRITE_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }

            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_LOTUS(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MITX_DISf, &temp);
                soc_TH_PCTLr_field_set(unit, &reg_value,
                    MIRX_DISf, &temp);
                if ((rv = REG_WRITE_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_LOTUS(unit))  {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit))
                    && (port == specified_port_num)) {
                    if ((rv = REG_READ_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }

                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MITX_DISf, &temp);
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        MIRX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        TX_DISf, &temp);
                    soc_G_PCTLr_field_set(unit, &reg_value, 
                        RX_DISf, &temp);
                }
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit))
                    && (port == specified_port_num)) {
                    if ((rv = REG_WRITE_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            if ((rv = REG_READ_VLAN_CTRL3r(
                unit, &vlan_ctrl3)) < 0) {
                return rv;
            }
            if (enable) {
                vlan_ctrl3 |= ( 1 << port);
            } else {
                vlan_ctrl3 &= ~( 1 << port);
            }
            if ((rv = REG_WRITE_VLAN_CTRL3r(
                unit, &vlan_ctrl3)) < 0) {
                return rv;
            }
            return rv;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
            return SOC_E_UNAVAIL;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            if (property == DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX) {
                reg_index = INDEX(RX_PAUSE_PASSr);
                fld_index = INDEX(RX_PAUSE_PASSf);
            } else {
                reg_index = INDEX(TX_PAUSE_PASSr);
                fld_index = INDEX(TX_PAUSE_PASSf);
            }
            addr =  (DRV_SERVICES(unit)->reg_addr)
                (unit, reg_index, 0, 0);
            length = (DRV_SERVICES(unit)->reg_length_get)
                (unit, reg_index);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, addr, &reg_value, length)) < 0) {
                return rv;
            }
            temp = 0;
            if (enable) {
                temp |= (1 << port);
            } else {
                temp &= ~(1 << port);
            }

            (DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, &reg_value, fld_index, &temp);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, addr, &reg_value, length)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            rv = DRV_VLAN_PROP_SET(unit, DRV_VLAN_PROP_DOUBLE_TAG_MODE, enable);
            if (rv < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            SOC_PBMP_CLEAR(t_bmp);
            SOC_PBMP_PORT_ADD(t_bmp, port);

            rv = DRV_VLAN_PROP_PORT_ENABLE_SET
                (unit, DRV_VLAN_PROP_ISP_PORT, t_bmp, enable);
            if (rv < 0) {
                return rv;
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                if ((rv = REG_READ_TRREG_CTRL1r(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_TRREG_CTRL1r_field_get(unit, &reg_value, 
                    DSCP_RMK_ENf, &temp);

                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
                soc_TRREG_CTRL1r_field_set(unit, &reg_value, 
                    DSCP_RMK_ENf, &temp);

                if ((rv = REG_WRITE_TRREG_CTRL1r(unit, &reg_value)) < 0) {
                    return rv;
                }
                
            } else 
 #endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */             
            return SOC_E_UNAVAIL;
            break;
            
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            /* This feature currently is supported in 53115/53118 robo
             *  chips.
             */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    if ((rv = REG_READ_TRREG_CTRL0r(unit, &reg_value)) < 0) {
                        return rv;
                    }
    
                    soc_TRREG_CTRL0r_field_get(unit, &reg_value, 
                        PCP_RMK_ENf, &temp);
                } else 
 #endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */           
                {
                    if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
    
                    soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                        PCP_RMK_ENf, &temp);
                }
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
    
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    soc_TRREG_CTRL0r_field_set(unit, &reg_value, 
                        PCP_RMK_ENf, &temp);
    
                    if ((rv = REG_WRITE_TRREG_CTRL0r(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
 #endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */
                {
                    soc_TRREG_CTRLr_field_set(unit, &reg_value, 
                        PCP_RMK_ENf, &temp);
    
                    if ((rv = REG_WRITE_TRREG_CTRLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) {
#if defined(BCM_53115) || defined(BCM_53125) || defined(BCM_POLAR_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT)
                if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                    CFI_RMK_ENf, &temp);
                            
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }

                soc_TRREG_CTRLr_field_set(unit, &reg_value, 
                    CFI_RMK_ENf, &temp);
                    
                if ((rv = REG_WRITE_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }
#endif /* BCM_53115 || BCM_53125 || POLAR || NORTHSTAR */

            } else if(SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                /* NSP's CFI_REMARK and DEI_REMARK can control this feature 
                 *   1. CFI_REMARK: backward compatible to earily ROBO device.
                 *      - TRREG_CTRL0r.CFI_RMK_ENf
                 *      - for CFI/DEI remark on outmost tag in egress frame.
                 *          P.S: in double tagged packet, the CFI bit in inner
                 *              Tag(C-Tag)won't be modified even this field 
                 *              is set.
                 *      
                 *   2. DEI_REMARK: new control in NSP.
                 *      - TRREG_CTRL1r.DEI_RMK_ENf
                 *      - for DEI remark on S-Tagged egress frame 
                 *      P.S. DEI_RMK_ENf will be inactive internally while 
                 *          CFI_RMK_ENf is enabled.
                 *
                 *  The design for such HW Spec. can be found in comment below.
                 *
                 *  Note :
                 *  - Once the DT mode or NNI/UNI mode been changed by user 
                 *      VLAN APIs this process must be proceeded to ensure 
                 *      the final setting can still been working properly.
                 */
                uint32  is_dt = 0, is_nni = 0; 
                uint32  request_cfi_rmk, request_dei_rmk;
                
                /* 1. check if double tagging mode(both DT/iDT) is set and 
                 *  the NNI(ServicePort)/UNI(UsedrPort) mode of this port.
                 */
                SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_GET(unit, 
                        DRV_VLAN_PROP_DOUBLE_TAG_MODE, &is_dt));
                SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, port, 
                        DRV_PORT_PROP_DTAG_ISP_PORT, &is_nni));
                
                /* 2. For enable case, the configuration of CFI_RMK_ENf and 
                 *  DEI_RMK_ENf are :
                 *  a. for CFI_RMK_ENf :
                 *      - Assert it at the port bit while
                 *          i. Double tagging mode is not enabled, or
                 *          ii. DT/iDT enabled but port is not at NNI mode.
                 *      - Reset it for all other case.
                 *  b. for DEI_RMK_ENf :
                 *      - Assert it at the port bit while
                 *          i. Double tagging(DT or iDT) is enabled, and 
                 *          ii. Port is at NNI mode(is ISP port).
                 *      - Reset it for all other case.
                 *
                 * 3. For disable case, 
                 *  a. both CFI/DEI remark fields of this port bit must be 
                 *      reset no matter the port is NNI or UNI.
                 */
                request_cfi_rmk = request_dei_rmk = 0;
                if (enable){
                    if (is_dt && is_nni){
                        request_dei_rmk = 1;
                    } else {
                        request_cfi_rmk = 1;
                    }
                }
                
                /* proceeding DEI remarking setting */
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL1r(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL1r_field_get(unit, 
                        &reg_value, DEI_RMK_ENf, &temp));
                if (request_dei_rmk){
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL1r_field_set(unit,
                        &reg_value, DEI_RMK_ENf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_TRREG_CTRL1r(unit, &reg_value));
                
                /* proceeding CFI remarking setting */
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL0r(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL0r_field_get(unit, 
                        &reg_value, CFI_RMK_ENf, &temp));
                if (request_cfi_rmk){
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL0r_field_set(unit,
                        &reg_value, CFI_RMK_ENf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_TRREG_CTRL0r(unit, &reg_value));
                
#endif /* NORTHSTARPLUS || SF3 */
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined (BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL2r(unit, &reg_value));

                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL2r_field_get(unit, 
                        &reg_value, C_PCP_RMK_ENf, &temp));
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL2r_field_set(unit, 
                        &reg_value, C_PCP_RMK_ENf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_TRREG_CTRL2r(unit, &reg_value));

            } else
#endif /* BCM_NORTHSTARPLUS_SUPPORT */   
            {
                return SOC_E_UNAVAIL;
            }
            break;

        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined (BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL2r(unit, &reg_value));

                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL2r_field_get(unit, 
                        &reg_value, S_PCP_RMK_ENf, &temp));
                if (enable) {
                    temp |= (1 << port);
                } else {
                    temp &= ~(1 << port);
                }
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL2r_field_set(unit, 
                        &reg_value, S_PCP_RMK_ENf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_TRREG_CTRL2r(unit, &reg_value));

            } else
#endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3*/   
            {
                return SOC_E_UNAVAIL;
            }
            break;

        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

/*
 *  Function : _drv_gex_port_property_enable_get
 *
 *  Purpose :
 *      Get the status of port related properties
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      property  :   port property type.
 *      enable  :   status of this property.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_gex_port_property_enable_get(int unit, int port, int property, uint32 *enable)
{
    uint32    addr, temp = 0, vlan_ctrl3;
    int        rv = SOC_E_NONE;
    int        length;
    uint32    reg_index = 0, fld_index = 0;
    uint32    reg_value;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */

    switch (property) {
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
        case DRV_PORT_PROP_SA_OVERLIMIT_DROP:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
        case DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (port == 7){
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_7_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else if (port == 8) {
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_8_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_N_SA_LIMIT_CTLr(unit, 
                            port, &reg_value));
                }

                SOC_IF_ERROR_RETURN(
                        soc_PORT_N_SA_LIMIT_CTLr_field_get(unit, &reg_value, 
                        OVER_LIMIT_ACTIONSf, &temp));

                if (property == DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE) {
                    *enable = (temp == _DRV_SA_OVERLIMIT_ACTION_NONE) ? 
                            TRUE : FALSE;
                } else {
                    if ((property == DRV_PORT_PROP_SA_OVERLIMIT_DROP) ||
                        (property == DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP)){

                        *enable = (temp & _DRV_SA_OVERLIMIT_ACTION_DROP) ? TRUE : FALSE;
                    } else {
                        /* to CPU action */
                        *enable = (temp & _DRV_SA_OVERLIMIT_ACTION_CPU) ? TRUE : FALSE;
                    }
                }
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT*/
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_LOTUS(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MIRX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_LOTUS(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit)) 
                    && (port == specified_port_num)) {
                    if ((rv = REG_READ_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MIRX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        RX_DISf, &temp);
                }
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_LOTUS(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MITX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_LOTUS(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit))
                    && (port == specified_port_num)) {
                    if ((rv = REG_READ_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MITX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        TX_DISf, &temp);
                }
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (IS_FE_PORT(unit, port) && (!SOC_IS_LOTUS(unit))) {
                if ((rv = REG_READ_TH_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MIRX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                    return rv;
                }
                soc_TH_PCTLr_field_get(unit, &reg_value,
                    MITX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else if (IS_GE_PORT(unit, port) || SOC_IS_LOTUS(unit)) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                    SOC_IS_NORTHSTARPLUS(unit))
                    && (port == specified_port_num)) {
                    if ((rv = REG_READ_P7_CTLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
                } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                        return rv;
                    }
                }
                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MIRX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        RX_DISf, &temp);
                }
                if (temp) {
                    *enable = FALSE;
                    return rv;
                }
                if (SOC_IS_DINO(unit)) {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        MITX_DISf, &temp);
                } else {
                    soc_G_PCTLr_field_get(unit, &reg_value, 
                        TX_DISf, &temp);
                }
                if (temp) {
                    *enable = FALSE;
                } else {
                    *enable = TRUE;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            if ((rv = REG_READ_VLAN_CTRL3r(
                unit, &vlan_ctrl3)) < 0) {
                return rv;
            }
            if (vlan_ctrl3 & ( 1 << port)) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }
            return rv;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
            return SOC_E_UNAVAIL;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            if (property == DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX) {
                reg_index = INDEX(RX_PAUSE_PASSr);
                fld_index = INDEX(RX_PAUSE_PASSf);
            } else {
                reg_index = INDEX(TX_PAUSE_PASSr);
                fld_index = INDEX(TX_PAUSE_PASSf);
            }
            addr =  (DRV_SERVICES(unit)->reg_addr)
                (unit, reg_index, 0, 0);
            length = (DRV_SERVICES(unit)->reg_length_get)
                (unit, reg_index);
            if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, addr, &reg_value, length)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, reg_index, &reg_value, fld_index, &temp);
            if (temp & ( 1 << port)) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            rv = DRV_VLAN_PROP_GET(unit, DRV_VLAN_PROP_DOUBLE_TAG_MODE, &temp);
            if (rv < 0) {
                return rv;
            }
            *enable = temp;
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            if ((rv = REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value,
                ISP_PORTMAPf, &temp);

            if (temp & (1 << port)) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }

            break;

        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if ((rv = REG_READ_TRREG_CTRL1r(unit, &reg_value)) < 0) {
                    return rv;
                }

                soc_TRREG_CTRL1r_field_get(unit, &reg_value, 
                    DSCP_RMK_ENf, &temp);
                
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
                return rv;
#endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */                       
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            /* This feature currently is supported in 5395/53115/53118 robo
             *  chips.
             */
            if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    if ((rv = REG_READ_TRREG_CTRL0r(unit, &reg_value)) < 0) {
                        return rv;
                    }
    
                    soc_TRREG_CTRL0r_field_get(unit, &reg_value, 
                        PCP_RMK_ENf, &temp);
                } else
#endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */   
                {
                    if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                        return rv;
                    }
    
                    soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                        PCP_RMK_ENf, &temp);
                }
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else {
                return SOC_E_UNAVAIL;
            }
            break;

        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) ||
                SOC_IS_NORTHSTAR(unit)) {
                if ((rv = REG_READ_TRREG_CTRLr(unit, &reg_value)) < 0) {
                    return rv;
                }
                
                soc_TRREG_CTRLr_field_get(unit, &reg_value, 
                    CFI_RMK_ENf, &temp);
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                /* NSP's CFI_REMARK and DEI_REMARK can control this feature 
                 * through CFI_RMK_ENf and DEI_RMK_ENf.(Please check the set 
                 * process for more detail description.)
                 *
                 *  The specific design for NS+, the reporting status is OR 
                 * from both fields setting.
                 */
                *enable = FALSE;
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL1r(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL1r_field_get(unit, 
                        &reg_value, DEI_RMK_ENf, &temp));
                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL0r(unit, &reg_value));
                    SOC_IF_ERROR_RETURN(soc_TRREG_CTRL0r_field_get(unit, 
                            &reg_value, CFI_RMK_ENf, &temp));
                    if (temp & (1 << port)) {
                        *enable = TRUE;
                    }
                }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */   
            } else {
                return SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL2r(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL2r_field_get(unit, 
                        &reg_value, C_PCP_RMK_ENf, &temp));

                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else
#endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */   
            {
                return SOC_E_UNAVAIL;
            }
            break;

        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(REG_READ_TRREG_CTRL2r(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_TRREG_CTRL2r_field_get(unit, 
                        &reg_value, S_PCP_RMK_ENf, &temp));

                if (temp & (1 << port)) {
                    *enable = TRUE;
                } else {
                    *enable = FALSE;
                }
            } else
#endif /* BCM_NORTHSTARPLUS_SUPPORT || SF3 */   
            {
                return SOC_E_UNAVAIL;
            }
            break;
        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

STATIC int
_drv_gex_port_security_mode_set(int unit, uint32 port, uint32 mode, uint32 sa_num)
{
    uint32 temp = 0;
    uint64  reg_value64;
    int rv = SOC_E_NONE;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            temp = 0;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            temp = 2;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            temp = 3;
            break;
        default:
            return SOC_E_PARAM;
    }
    /* write to chip */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        if ((rv = REG_READ_PORT_EAP_CON_P7r(
            unit, (uint32 *)&reg_value64)) < 0) {
            return rv;
        }
    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
    {
        if ((rv = REG_READ_PORT_EAP_CONr(
            unit, port, (uint32 *)&reg_value64)) < 0) {
            return rv;
        }
    }
    soc_PORT_EAP_CONr_field_set(unit, (uint32 *)&reg_value64,
        EAP_MODEf, &temp);

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit))
        && (port == specified_port_num)) {
        if ((rv = REG_WRITE_PORT_EAP_CON_P7r(
            unit, (uint32 *)&reg_value64)) < 0) {
            return rv;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
    {
        if ((rv = REG_WRITE_PORT_EAP_CONr(
            unit, port, (uint32 *)&reg_value64)) < 0) {
            return rv;
        }
    }

    return rv;
}

STATIC int
_drv_gex_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32 temp = 0;
    uint64  reg_value64;
    int rv = SOC_E_NONE;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit)|| SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }

    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        if ((rv = REG_READ_PORT_EAP_CON_P7r(
            unit, (uint32 *)&reg_value64)) < 0) {
            return rv;
        }
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORET
        */
    {
        if ((rv = REG_READ_PORT_EAP_CONr(
            unit, port, (uint32 *)&reg_value64)) < 0) {
            return rv;
        }
    }
    soc_PORT_EAP_CONr_field_get(unit, (uint32 *)&reg_value64,
        EAP_MODEf, &temp);
    switch (temp) {
        case 0:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_NONE) {
                *prop_val = TRUE;
            }
            break;
        case 2:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_EXTEND) {
                *prop_val = TRUE;
            }
            break;
        case 3:
            if (mode == DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY) {
                *prop_val = TRUE;
            }
            break;
        default:
            return SOC_E_INTERNAL;
    }
    return rv;
}

STATIC int 
_drv_gex_port_eee_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val)
{
#if defined(BCM_53125) || defined(BCM_53128) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    int rv = SOC_E_NONE, port;
    uint32 reg_val = 0, temp = 0;
    uint32 freq_khz = 0;
    
    switch(prop_type) {
        case DRV_PORT_PROP_EEE_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_EN_CTRLr(unit, &reg_val));
            soc_EEE_EN_CTRLr_field_get(unit, &reg_val, 
                EN_EEEf, &temp);
            if (prop_val) {
                /* Northstar don't support MAC LOW POWER Mode */
                if ((!SOC_IS_NORTHSTAR(unit)) && 
                                (!SOC_IS_NORTHSTARPLUS(unit)) &&
                                (!SOC_IS_STARFIGHTER3(unit) )) {
                    /* If current state is MAC LOW POWER Mode,
                     * don't allow to enable the EEE.
                     * It will casue the port hand or other abnormal behavior.
                     */
                    SOC_IF_ERROR_RETURN(
                        DRV_DEV_PROP_GET(unit, 
                        DRV_DEV_PROP_LOW_POWER_ENABLE, &freq_khz));
                    if (freq_khz) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META_U(unit,
                                              "Don't allow to enable EEE in Low Power Mode.\n")));
                        return SOC_E_FAIL;
                    }
                }
                temp |= (SOC_PBMP_WORD_GET(bmp, 0));
            } else {
                temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            }
            soc_EEE_EN_CTRLr_field_set(unit, &reg_val, 
                EN_EEEf, &temp);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_EEE_EN_CTRLr(unit, &reg_val));
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_SLEEP_TIMER_Gr(unit, port, &reg_val));
                soc_EEE_SLEEP_TIMER_Gr_field_set(unit, &reg_val, 
                    SLEEP_TIMER_Gf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_SLEEP_TIMER_Gr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_SLEEP_TIMER_Hr(unit, port, &reg_val));
                soc_EEE_SLEEP_TIMER_Hr_field_set(unit, &reg_val, 
                    SLEEP_TIMER_Hf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_SLEEP_TIMER_Hr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_MIN_LP_TIMER_Gr(unit, port, &reg_val));
                soc_EEE_MIN_LP_TIMER_Gr_field_set(unit, &reg_val, 
                    MIN_LP_TIMER_Gf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_MIN_LP_TIMER_Gr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
            /* 20-bit length */
            if (prop_val > 1048576) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_MIN_LP_TIMER_Hr(unit, port, &reg_val));
                soc_EEE_MIN_LP_TIMER_Hr_field_set(unit, &reg_val, 
                    MIN_LP_TIMER_Hf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_MIN_LP_TIMER_Hr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
            /* 10-bit length */
            if (prop_val > 1024) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_WAKE_TIMER_Gr(unit, port, &reg_val));
                soc_EEE_WAKE_TIMER_Gr_field_set(unit, &reg_val, 
                    WAKE_TIMER_Gf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_WAKE_TIMER_Gr(unit, port, &reg_val));
            }
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
            /* 10-bit length */
            if (prop_val > 1024) {
                return SOC_E_CONFIG;
            }
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_EEE_WAKE_TIMER_Hr(unit, port, &reg_val));
                soc_EEE_WAKE_TIMER_Hr_field_set(unit, &reg_val, 
                    WAKE_TIMER_Hf, &prop_val);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_EEE_WAKE_TIMER_Hr(unit, port, &reg_val));
            }
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
#else /* defined(BCM_53125) || defined(BCM_53128) || BCM_NORTHSTAR_SUPPORT ||
       * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
       */
    return SOC_E_UNAVAIL;
#endif /* defined(BCM_53125) || defined(BCM_53128) || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */
}

STATIC int 
_drv_gex_port_eee_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val)
{
#if defined(BCM_53125) || defined(BCM_53128) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    int rv = SOC_E_NONE;
    uint32 reg_val = 0, temp = 0;
    
    switch(prop_type) {
        case DRV_PORT_PROP_EEE_ENABLE:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_EN_CTRLr(unit, &reg_val));
            soc_EEE_EN_CTRLr_field_get(unit, &reg_val, 
                EN_EEEf, &temp);
            if (temp & (0x1 << port)) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_SLEEP_TIMER_Gr(unit, port, &reg_val));
            soc_EEE_SLEEP_TIMER_Gr_field_get(unit, &reg_val, 
                SLEEP_TIMER_Gf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_SLEEP_TIMER_Hr(unit, port, &reg_val));
            soc_EEE_SLEEP_TIMER_Hr_field_get(unit, &reg_val, 
                SLEEP_TIMER_Hf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_MIN_LP_TIMER_Gr(unit, port, &reg_val));
            soc_EEE_MIN_LP_TIMER_Gr_field_get(unit, &reg_val, 
                MIN_LP_TIMER_Gf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_MIN_LP_TIMER_Hr(unit, port, &reg_val));
            soc_EEE_MIN_LP_TIMER_Hr_field_get(unit, &reg_val, 
                MIN_LP_TIMER_Hf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_WAKE_TIMER_Gr(unit, port, &reg_val));
            soc_EEE_WAKE_TIMER_Gr_field_get(unit, &reg_val, 
                WAKE_TIMER_Gf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
            SOC_IF_ERROR_RETURN(
                REG_READ_EEE_WAKE_TIMER_Hr(unit, port, &reg_val));
            soc_EEE_WAKE_TIMER_Hr_field_get(unit, &reg_val, 
                WAKE_TIMER_Hf, &temp);
            *prop_val = temp;
            break;
        default:
            rv = SOC_E_UNAVAIL;
    }

    return rv;
#else /* defined(BCM_53125) || defined(BCM_53128) || BCM_NORTHSTAR_SUPPORT || NS+ ||
       * BCM_STARFIGHTER3_SUPPORT
       */
    return SOC_E_UNAVAIL;
#endif /* defined(BCM_53125) || defined(BCM_53128) || BCM_NORTHSTAR_SUPPORT || NS+ ||
        * BCM_STARFIGHTER3_SUPPORT
        */
}


/*
 *  Function : drv_gex_port_set
 *
 *  Purpose :
 *      Set the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val)
{
    uint32 reg_value, temp = 0;
    int port;
    int rv = SOC_E_NONE;
    mac_driver_t *p_mac = NULL;
    int br_mode = 0;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */    

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_port_set: unit=%d bmp=%x\n"), 
              unit, SOC_PBMP_WORD_GET(bmp, 0)));

    SOC_ROBO_PORT_INIT(unit); 
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_SPEED\n")));
            temp = 0;
            switch (prop_val) {
                case DRV_PORT_STATUS_SPEED_10M:
                    temp = 10;
                    break;
                case DRV_PORT_STATUS_SPEED_100M:
                    temp =100;
                    break;
                case DRV_PORT_STATUS_SPEED_1G:
                    temp = 1000;
                    break;
                case DRV_PORT_STATUS_SPEED_2G:
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                    if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
                        temp = 2000;
                    } else 
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
                    {
                        return SOC_E_PARAM;
                    }
                    break;
                case DRV_PORT_STATUS_SPEED_2500M:
#ifdef BCM_STARFIGHTER3_SUPPORT
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        temp = 2500;
                    } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
                    {
                        return SOC_E_PARAM;
                    }                      
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY and MAC auto-negotiation OFF */
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_speed_set(unit, port, temp);

                /* if auto-negotiation is OFF, */
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_speed_set != NULL) {
                        rv = MAC_SPEED_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;

        case DRV_PORT_PROP_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_DUPLEX\n")));
            switch (prop_val) {
                case DRV_PORT_STATUS_DUPLEX_HALF:
                    temp = FALSE;
                    break;
                case DRV_PORT_STATUS_DUPLEX_FULL:
                    temp =TRUE;
                    break;
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
             
                /* Set PHY registers anyway. */
                rv = soc_phyctrl_duplex_set(unit, port, temp);

                /* set MAC duplex */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);                
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_duplex_set != NULL) {
                        rv = MAC_DUPLEX_SET(
                            p_mac, unit, port, temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
        case DRV_PORT_PROP_AUTONEG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_AUTONEG\n")));
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_TX_PAUSE\n")));
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_RX_PAUSE\n")));
            PBMP_ITER(bmp, port) {
                 /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, -1, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: LOCAL_ADVER\n")));
            PBMP_ITER(bmp, port) {
                /* set advertise to PHY accordingly */
                rv = soc_phyctrl_adv_local_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: REMOTE_ADVER not support\n")));
            /* can not set remote advert */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PORT_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_MAC_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_INTERFACE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_INTERFACE %x %x\n"), 
                      SOC_PBMP_WORD_GET(bmp, 0), prop_val));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_interface_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_ENABLE:
            /* This case is called for _bcm_robo_port_update() only. */
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_MAC_ENABLE\n")));
            PBMP_ITER(bmp, port) {
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_ENABLE\n")));
            PBMP_ITER(bmp, port) {
                /* Set PHY registers anyway. */
                rv = soc_phyctrl_enable_set(unit, port, prop_val);
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_enable_set != NULL) {
                        rv = MAC_ENABLE_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s: PROP_ENABLE_SET\n"), FUNCTION_NAME()));
            PBMP_ITER(bmp, port) {
                rv = _drv_gex_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_IPG_FE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_IPG_FE\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_IPG_GE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_IPG_GE\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_JAM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_JAM\n")));
            
            /* Robo chip use binding Jamming/Pause, Jamming/Pause can't be set
             * independent.
             */
            PBMP_ITER(bmp, port) {
                /* set TX PAUSE */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_pause_set != NULL) {
                        rv = MAC_PAUSE_SET(
                            p_mac, unit, port, prop_val, -1);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_BPDU_RX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_BPDU_RX\n")));
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) {
                if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_GMNGCFGr_field_set(unit, &reg_value, 
                    RXBPDU_ENf, &temp);
                if ((rv = REG_WRITE_GMNGCFGr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_MAC_LOOPBACK\n")));
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_lb_set != NULL) {
                        rv = MAC_LOOPBACK_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_LOOPBACK\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_loopback_set(unit, port, prop_val, TRUE);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_ex_port_set: PROP_PHY_MEDIUM not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_MDIX\n")));
            PBMP_ITER(bmp, port) {
#ifdef BCM_POLAR_SUPPORT
                if (SOC_IS_POLAR(unit)) {
                    br_mode = soc_property_port_get(unit, port, 
                                           spn_PHY_LR_INITIAL_MODE, 0);
                }
#endif /* BCM_POLAR_SUPPORT */
                if (!br_mode) {
                    rv = soc_phyctrl_mdix_set(unit, port, prop_val);
                }
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_MDIX_STATUS not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_MS not support\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_master_set(unit, port, prop_val);
            }
            break;
            
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_SEC_MODE_NONE\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            PBMP_ITER(bmp, port) {
                rv = _drv_gex_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_SEC_MAC_MODE_EXTEND\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            PBMP_ITER(bmp, port) {
                rv = _drv_gex_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_SEC_MAC_MODE_SIMPLIFY\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            PBMP_ITER(bmp, port) {
                rv = _drv_gex_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_LINKUP_EVT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_LINKUP_EVT\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkup_evt(unit, port);
                if (SOC_FAILURE(rv) && (SOC_E_UNAVAIL != rv)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_LINKDN_EVT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_LINKDN_EVT\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkdn_evt(unit, port);
                if (SOC_FAILURE(rv) && (SOC_E_UNAVAIL != rv)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_RESET:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PHY_RESET\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phy_reset(unit, port);
            }
            break;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_PAUSE_FRAME_BYPASS\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            PBMP_ITER(bmp, port) {
                rv = _drv_gex_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_DTAG_MODE\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                if (prop_val == 1) {
                    /* bcm5389/5396 does not support dtag mode*/
                    rv = SOC_E_UNAVAIL;
                } else {
                    rv = SOC_E_NONE;
                }
            }
            SOC_IF_ERROR_RETURN(rv);
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_property_enable_set(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_DTAG_ISP_PORT\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            PBMP_ITER(bmp, port) {
                rv = _drv_gex_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_TPID:       
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_DTAG_TPID\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            if ((rv = REG_READ_DTAG_TPIDr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_DTAG_TPIDr_field_set(unit, &reg_value, ISP_TPIDf, &prop_val);
            if ((rv = REG_WRITE_DTAG_TPIDr(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_MAC_BASE_VLAN\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            PBMP_ITER(bmp, port) {
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    if (p_mac->md_frame_max_set != NULL) {
                        rv = MAC_FRAME_MAX_SET(
                            p_mac, unit, port, prop_val);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_set: PROP_INGRESS_VLAN_CHK\n")));
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_VLAN_CTRL4r_field_set(unit, &reg_value,
                INGR_VID_CHKf, &prop_val);
            if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_EEE_ENABLE:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
            if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit) ) {
#if defined(BCM_53125) || defined(BCM_53128) || defined(BCM_STARFIGHTER3_SUPPORT) ||\
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                rv = _drv_gex_port_eee_set(unit, bmp, 
                    prop_type, prop_val);
#endif
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT) 
            if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "drv_gex_port_set: PORT_PROP_DEFAULT_TC_PRIO\n")));

                SOC_IF_ERROR_RETURN(REG_READ_PID2TCr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_PID2TCr_field_get(unit, 
                                        &reg_value, PID2TCf, &temp));
                PBMP_ITER(bmp, port) {
                    if (port < SOC_ROBO_MAX_NUM_PORTS) {
                        /* Port number will never cross 9 for these devices. Adding
                           below check to avoid coverity error */
                        if (port > 10) {
                            continue; 
                        }    
                        /* 3 bits per port */
                        temp &= ~(7 << (port * 3));
                        temp |= (prop_val << (port * 3));
                    }
                }
                SOC_IF_ERROR_RETURN(soc_PID2TCr_field_set(unit, 
                                        &reg_value, PID2TCf, &temp));
                SOC_IF_ERROR_RETURN(REG_WRITE_PID2TCr(unit, &reg_value));              
            } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
                SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit) || SOC_IS_DINO(unit) ) {
                /* 
                 * The default TC value is the same for untagged packets and 
                 * port-based mapping.
                 */
                return DRV_PORT_SET(unit, bmp, 
                    DRV_PORT_PROP_UNTAG_DEFAULT_TC, prop_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
            if (SOC_IS_POLAR(unit)) {
                SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, 
                    &specified_port_num));
            } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
                SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, 
                    &specified_port_num));
            }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */                

            PBMP_ITER(bmp, port) {
                /* Read the default value */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit)) 
                    && (port == specified_port_num)) {
                    SOC_IF_ERROR_RETURN(
                        REG_READ_DEFAULT_1Q_TAG_P7r(unit, &reg_value));
                } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    SOC_IF_ERROR_RETURN(
                        REG_READ_DEFAULT_1Q_TAGr(unit, port, &reg_value));
                }
                reg_value &= ~(DOT1P_PRI_MASK << DOT1P_PRI_SHIFT);
                reg_value |= ((prop_val & DOT1P_PRI_MASK) << 
                    DOT1P_PRI_SHIFT);

                /* Write the default value */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)||
                    SOC_IS_NORTHSTARPLUS(unit)) 
                    && (port == specified_port_num)) {
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_DEFAULT_1Q_TAG_P7r(unit, &reg_value));
                } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
                {
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_DEFAULT_1Q_TAGr(unit, port, &reg_value));
                }
            }
            break;

        case DRV_PORT_PROP_MIB_CLEAR:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                /* Enable per-port based mib clear function */
                temp = SOC_PBMP_WORD_GET(bmp, 0);
                soc_RST_MIB_CNT_ENr_field_set(unit, &reg_value, 
                    RST_MIB_CNT_ENf , &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_RST_MIB_CNT_ENr(unit, &reg_value));
                
                /* Start to clear */
                SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
                     (unit, &reg_value));
         
                temp = 1;
                SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
                     (unit, &reg_value, RST_MIB_CNTf, &temp));
                 
                SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
                    (unit, &reg_value));
             
                for (port = 0; port < 10000; port++) {
                    temp = port;
                }
                temp = 0;
                SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
                    (unit, &reg_value, RST_MIB_CNTf, &temp));
                 
                SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
                    (unit, &reg_value));
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_PPPOE_PARSE_EN:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_PPPOE_SESSION_PARSE_ENr(unit, &reg_value));
                soc_PPPOE_SESSION_PARSE_ENr_field_get(unit, &reg_value, 
                    PPPOE_SESSION_PARSE_ENf , &temp);
                if (prop_val) {
                    temp |= SOC_PBMP_WORD_GET(bmp, 0);
                } else {
                    temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
                }
                soc_PPPOE_SESSION_PARSE_ENr_field_set(unit, &reg_value, 
                    PPPOE_SESSION_PARSE_ENf , &temp);
                SOC_IF_ERROR_RETURN(
                    REG_WRITE_PPPOE_SESSION_PARSE_ENr(unit, &reg_value));
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT*/
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
            
        case DRV_PORT_PROP_RAW_TC_MAP_MODE_SELECT:
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)||
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit))) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                PBMP_ITER(bmp, port) {
    
                    reg_value = prop_val;
                    if (port == 7){
                        SOC_IF_ERROR_RETURN(
                                REG_WRITE_P7_TC_SEL_TABLEr(unit, 
                                &reg_value));
                    } else if (IS_CPU_PORT(unit, port)) {
                        SOC_IF_ERROR_RETURN(
                                REG_WRITE_IMP_TC_SEL_TABLEr(unit, 
                                &reg_value));
                    } else {
                        SOC_IF_ERROR_RETURN(
                                REG_WRITE_TC_SEL_TABLEr(unit, 
                                port, &reg_value));
                    }
                }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */
            } else {
                rv = SOC_E_UNAVAIL;
            }

            break;

        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                PBMP_ITER(bmp, port) {
                    if (port == 7){
                        SOC_IF_ERROR_RETURN(
                                REG_READ_PORT_7_SA_LIMIT_CTLr(unit, 
                                &reg_value));
                    } else if (port == 8) {
                        SOC_IF_ERROR_RETURN(
                                REG_READ_PORT_8_SA_LIMIT_CTLr(unit, 
                                &reg_value));
                    } else {
                        SOC_IF_ERROR_RETURN(
                                REG_READ_PORT_N_SA_LIMIT_CTLr(unit, 
                                port, &reg_value));
                    }
                    temp = prop_val & DRV_PORT_LEARN_LIMIT_ACTION_MASK;
                    SOC_IF_ERROR_RETURN(
                            soc_PORT_N_SA_LIMIT_CTLr_field_set(unit, &reg_value, 
                            OVER_LIMIT_ACTIONSf, &temp));
                    if (port == 7){
                        SOC_IF_ERROR_RETURN(
                                REG_WRITE_PORT_7_SA_LIMIT_CTLr(unit, 
                                &reg_value));
                    } else if (port == 8) {
                        SOC_IF_ERROR_RETURN(
                                REG_WRITE_PORT_8_SA_LIMIT_CTLr(unit, 
                                &reg_value));
                    } else {
                        SOC_IF_ERROR_RETURN(
                                REG_WRITE_PORT_N_SA_LIMIT_CTLr(unit, 
                                port, &reg_value));
                    }
                }
                
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            } else {
                rv = SOC_E_UNAVAIL;
            }
            
            break;
            
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            /* prop_val here is the limit value only
             *  
             * Note : value can be 
             *  1. (-1) for disable learn limit
             *  2. (0) enable learn limit but no MAC can be learned.
             *  3. (>0) enable learn limit and only the limit count of MAC can be learned.
             */
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_gex_port_property_enable_set
                            (unit, port, prop_type, prop_val));
            }
            break;

        /* not supported list */
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_ROAMING_OPT:
            rv = SOC_E_UNAVAIL;
            break;
            
        default: 
            rv = SOC_E_PARAM; 
        break;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_port_set: Exit\n")));
    return rv;
}

/*
 *  Function : drv_gex_port_get
 *
 *  Purpose :
 *      Get the property to the specific ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port id.
 *      prop_type  :   port property type.
 *      prop_val    :   port property value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val)
{
    int         rv = SOC_E_NONE;
    uint32     reg_value, temp = 0;
    uint32     mac_ability = 0, phy_ability;
    int        pause_tx, pause_rx;
    int        autoneg, done;
    mac_driver_t *p_mac = NULL;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */       

    /* remarked for linkscan called each time
     *  (too many messages will be printed out)
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_get: unit=%d port=%d\n"), unit, port));
    */
    SOC_ROBO_PORT_INIT(unit);
    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Speed\n")));
            temp = 0;

            if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                /* redesigned for Northstar to report speed from MAC layer */
                if (p_mac != NULL) {
                    if (p_mac->md_speed_get != NULL) {
                        rv = MAC_SPEED_GET(p_mac, 
                            unit, port, (int *)&temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
            } else {
                rv = soc_phyctrl_speed_get(unit, port, (int *)&temp);
            }

            if (rv != 0){
                return rv;
            }
            switch(temp) {
                case 10:
                    *prop_val = DRV_PORT_STATUS_SPEED_10M;
                    break;
                case 100:
                    *prop_val = DRV_PORT_STATUS_SPEED_100M;
                    break;
                case 1000:
                    *prop_val = DRV_PORT_STATUS_SPEED_1G;
                    break;
                case 2000:
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                    if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
                        *prop_val = DRV_PORT_STATUS_SPEED_2G;
                    } else 
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
                    {
                        *prop_val = 0;
                    }
                    break;
                case 2500:
#ifdef BCM_STARFIGHTER3_SUPPORT
                    if (SOC_IS_STARFIGHTER3(unit)) {
                        *prop_val = DRV_PORT_STATUS_SPEED_2500M;
                    } else
#endif /* BCM_STARFIGHTER3_SUPPORT */
                    {
                        *prop_val = 0;
                    }                      
                    break;
                default:
                    *prop_val = 0;
                    break;
            }
            break;
        case DRV_PORT_PROP_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Duplex\n")));
            temp = 0;

            if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                /* redesigned for Northstar to report duplex from MAC layer */
                if (p_mac != NULL) {
                    if (p_mac->md_duplex_get != NULL) {
                        rv = MAC_DUPLEX_GET(p_mac, 
                            unit, port, (int *)&temp);
                    } else {
                        rv = SOC_E_UNAVAIL;
                    }
                } else {
                    rv = SOC_E_PARAM;
                }
#endif /* BCM_NORTHSTAR_SUPPORT || BCM_NORTHSTARPLUS_SUPPORT */
            } else {
                rv = soc_phyctrl_duplex_get(unit, port, (int *)&temp);
            }
            if (rv != 0){
                return rv;
            }

            switch(temp) {
                case 0:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_HALF;
                    break;
                case 1:
                    *prop_val = DRV_PORT_STATUS_DUPLEX_FULL;
                    break;
                default:
                    break;
            }
            break;
        case DRV_PORT_PROP_AUTONEG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Autoneg\n")));
            rv = soc_phyctrl_auto_negotiate_get(unit, port,
                                                &autoneg, &done);
            *prop_val = (autoneg) ? DRV_PORT_STATUS_AUTONEG_ENABLE :
                        DRV_PORT_STATUS_AUTONEG_DISABLED;
            
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: TX Pause\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, (int *)prop_val, &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_RX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: RX Pause\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, &pause_tx, (int *)prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_LOCAL_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Local Advertise\n")));
            rv = soc_phyctrl_adv_local_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Remote Advertise\n")));
            /* if auto-negotiation is ON and negotiation is completed */
            /*   get remote advertisement from PHY */
            rv = soc_phyctrl_adv_remote_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Port Ability\n")));
            rv = soc_phyctrl_ability_get(unit, port, &phy_ability);
            SOC_IF_ERROR_RETURN(rv);
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }

            *prop_val  = mac_ability & phy_ability;
            *prop_val |= phy_ability & SOC_PM_ABILITY_PHY;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: MAC Ability\n")));
            if (p_mac != NULL) {
                if (p_mac->md_ability_get != NULL) {
                    rv = MAC_ABILITY_GET(
                        p_mac, unit, port, &mac_ability);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            *prop_val = mac_ability;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PHY Ability\n")));
            rv = soc_phyctrl_ability_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_INTERFACE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Interface\n")));
            if (p_mac != NULL) {
                if (p_mac->md_interface_get != NULL) {
                    rv = MAC_INTERFACE_GET(
                        p_mac, unit, port, prop_val); 
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_ENABLE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: Enable\n")));
            rv = soc_phyctrl_enable_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "%s: Enable Get\n"), FUNCTION_NAME()));
            rv = _drv_gex_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_IPG_FE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: IPG FE\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_IPG_GE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: IPG GE\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_JAM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: JAM\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(
                        p_mac, unit, port, (int *) prop_val,
                        &pause_rx);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            } 
            break;
        case DRV_PORT_PROP_BPDU_RX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: BPDU RX\n")));
            if ((rv = REG_READ_GMNGCFGr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_GMNGCFGr_field_get(unit, &reg_value, 
                    RXBPDU_ENf, &temp);
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
            break;
        case DRV_PORT_PROP_RESTART_AUTONEG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_RESTART_AUTONEG not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_MAC_LOOPBACK\n")));
            if (p_mac != NULL) {
                if (p_mac->md_lb_get != NULL) {
                    rv = MAC_LOOPBACK_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_PHY_LOOPBACK:
            /* Remarked for avoid too many message when linkscan.
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_port_get: PROP_PHY_LOOPBACK\n")));
            */
            rv = soc_phyctrl_loopback_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            /* remarked for linkscan called each time
             *  (too many messages will be printed out)
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_port_get: PROP_PHY_MEDIUM\n")));
            */
            rv = soc_phyctrl_medium_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_PHY_MDIX\n")));
            rv = soc_phyctrl_mdix_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_PHY_MDIX_STATUS\n")));
            rv = soc_phyctrl_mdix_status_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_MS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_MS\n")));
            rv = soc_phyctrl_master_get(unit, port, (int *) prop_val);
            break;
            
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_SEC_MODE_NONE\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_security_mode_get(unit, port, prop_type, prop_val);
            break;

        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_SEC_MAC_MODE_EXTEND\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_security_mode_get(unit, port, prop_type, prop_val);
            break;

        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_security_mode_get(unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_CABLE_DIAG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_PHY_CABLE_DIAG\n")));
            rv = soc_phyctrl_cable_diag(unit, port,
                                        (soc_port_cable_diag_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_LINK_CHANGE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_PHY_LINKCHANGE\n")));
            rv = soc_phyctrl_link_change(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_RX:
        case DRV_PORT_PROP_PAUSE_FRAME_BYPASS_TX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_PAUSE_FRAME_BYPASS\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_DTAG_MODE\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                /* bcm5389/5396 do not support dtag mode*/
                *prop_val = 0;
                rv = SOC_E_NONE;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_property_enable_get(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_DTAG_ISP_PORT\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            rv = _drv_gex_port_property_enable_get(unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_TPID:            
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_DTAG_TPID\n")));
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
            if (SOC_IS_DINO(unit)) {
                return SOC_E_UNAVAIL;
            }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
            if ((rv = REG_READ_DTAG_TPIDr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_DTAG_TPIDr_field_get(unit, &reg_value, ISP_TPIDf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_MAC_BASE_VLAN\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAX_FRAME_SZ:
            if (p_mac != NULL) {
                if (p_mac->md_frame_max_get != NULL) {
                    rv = MAC_FRAME_MAX_GET(
                        p_mac, unit, port, (int *) prop_val);
                } else {
                    rv = SOC_E_UNAVAIL;
                }
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_INGRESS_VLAN_CHK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_get: PROP_INGRESS_VLAN_CHK\n")));
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_VLAN_CTRL4r_field_get(unit, &reg_value,
                INGR_VID_CHKf, prop_val);
            break;
        case DRV_PORT_PROP_EEE_ENABLE:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_G:
        case DRV_PORT_PROP_EEE_SLEEP_DELAY_TIMER_H:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_G:
        case DRV_PORT_PROP_EEE_MIN_LPI_TIMER_H:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_G:
        case DRV_PORT_PROP_EEE_WAKE_TRANS_TIMER_H:
            if (SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)||
                SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_53125) || defined(BCM_53128) || defined(BCM_STARFIGHTER3_SUPPORT) || \
    defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
                rv = _drv_gex_port_eee_get(unit, port, 
                    prop_type, prop_val);
#endif /* defined(BCM_53125) || defined(BCM_53128) || BCM_STARFIGHTER3_SUPPORT ||
        * BCM_NORTHSTAR_SUPPORT  || BCM_NORTHSTARPLUS_SUPPORT
        */
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            if(SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "drv_port_get: PORT_PROP_DEFAULT_TC_PRIO\n")));

                SOC_IF_ERROR_RETURN(REG_READ_PID2TCr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(soc_PID2TCr_field_get(unit, 
                                        &reg_value, PID2TCf, &temp));
                /* 3 bits per port */
                *prop_val = (temp & (7 << (port * 3))) >> (port * 3);
            } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */
            if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit) ||
                SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                SOC_IS_BLACKBIRD2(unit) || SOC_IS_DINO(unit)) {
                /* 
                 * The default TC value is the same for untagged packets and 
                 * port-based mapping.
                 */
                return DRV_PORT_GET(unit, port,
                    DRV_PORT_PROP_UNTAG_DEFAULT_TC, prop_val);
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
            
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
            if (SOC_IS_POLAR(unit)) {
                SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, 
                    &specified_port_num));
            } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
                SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, 
                    &specified_port_num));
            }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */                

            /* Read the default value */
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit)) 
                && (port == specified_port_num)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_DEFAULT_1Q_TAG_P7r(unit, &reg_value));
            } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
            {
                SOC_IF_ERROR_RETURN(
                    REG_READ_DEFAULT_1Q_TAGr(unit, port, &reg_value));
            }

            *prop_val = ((reg_value >> DOT1P_PRI_SHIFT) & DOT1P_PRI_MASK);
                
            break;

        case DRV_PORT_PROP_PPPOE_PARSE_EN:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                SOC_IF_ERROR_RETURN(
                    REG_READ_PPPOE_SESSION_PARSE_ENr(unit, &reg_value));
                soc_PPPOE_SESSION_PARSE_ENr_field_get(unit, &reg_value, 
                    PPPOE_SESSION_PARSE_ENf , &temp);
                if (temp * (0x1 << port)) {
                    *prop_val = TRUE;
                } else {
                    *prop_val = FALSE;
                }
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;

        case DRV_PORT_PROP_RAW_TC_MAP_MODE_SELECT:
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)||
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) ) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (port == 7){
                    SOC_IF_ERROR_RETURN(
                            REG_READ_P7_TC_SEL_TABLEr(unit, 
                            &reg_value));
                } else if (IS_CPU_PORT(unit, port)) {
                    SOC_IF_ERROR_RETURN(
                            REG_READ_IMP_TC_SEL_TABLEr(unit, 
                            &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(
                            REG_READ_TC_SEL_TABLEr(unit, 
                            port, &reg_value));
                }
                *prop_val = reg_value;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT
        */
            } else {
                rv = SOC_E_UNAVAIL;
            }

            break;

        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
                if (port == 7){
                    SOC_IF_ERROR_RETURN(
                            REG_READ_PORT_7_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else if (port == 8) {
                    SOC_IF_ERROR_RETURN(
                            REG_READ_PORT_8_SA_LIMIT_CTLr(unit, 
                            &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(
                            REG_READ_PORT_N_SA_LIMIT_CTLr(unit, 
                            port, &reg_value));
                }
                SOC_IF_ERROR_RETURN(
                        soc_PORT_N_SA_LIMIT_CTLr_field_get(unit, &reg_value, 
                        OVER_LIMIT_ACTIONSf, &temp));
                if (temp == 1) {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_DROP;
                } else if (temp == 2) {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_COPY2CPU;
                } else if (temp == 3) {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_REDIRECT2CPU;
                } else {
                    /* tmep ==0 */
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_NONE;
                }
                
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
            } else {
                rv = SOC_E_UNAVAIL;
            }
            
            break;

        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            SOC_IF_ERROR_RETURN(_drv_gex_port_property_enable_get
                    (unit, port, prop_type, prop_val));
            break;
        /* not supported list */
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_ROAMING_OPT:
            rv = SOC_E_UNAVAIL;
            break;
            
        default: 
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_gex_port_status_get
 *
 *  Purpose :
 *      Get the status of the port for selected status type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port   :   port number.
 *      status_type  :   port status type.
 *      vla     :   status value.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr = 0, reg_value = 0, reg_len = 0;
    int     okay = 0;
    
    uint32  phy_medium = SOC_PORT_MEDIUM_COPPER;
    
    uint32  port_lb_phy = 0;
    int up = 0;
    int speed = 0;
    int duplex = 0;
    
    /* int_pd and pd used to prevent the runpacket issue on the GE port.
     * (with Internal SerDes bounded)
     */
    phy_ctrl_t      *int_pc = NULL, *ext_pc = NULL;
    
    /* special process to detach port driver */
    if (status_type == DRV_PORT_STATUS_DETACH){
        *val = TRUE;
        rv = drv_robo_port_sw_detach(unit);
        if (rv < 0) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "Port detach failed!\n")));
            *val = FALSE;
        }
        LOG_INFO(BSL_LS_SOC_PORT,
                 (BSL_META_U(unit,
                             "drv_gex_port_status_get: DETACH %s\n"),
                  *val ? "OK" : "FAIL"));
        return SOC_E_NONE;
    }
    
    /* To prevent the runpacket issue on the GE port which bounded 
     *  with internal SerDes and connected to an external PHY through SGMII.
     *  The linked information report will be designed to retrieved from   
     *  internal SerDes instead of Ext_PHY.
     *  (original design check external PHY only) 
     */
    if (IS_GE_PORT(unit, port)){
        int_pc = INT_PHY_SW_STATE(unit, port);
        ext_pc = EXT_PHY_SW_STATE(unit, port);
    }
     

    /* remarked for performance issue when debugging LinkScan
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_port_status_get: unit = %d, port = %d\n"),
              unit, port));
    */
    switch (status_type)
    {
        case DRV_PORT_STATUS_LINK_UP:
            /* get the port loopback status (for Robo Chip)
             */
            rv = soc_phyctrl_loopback_get(unit, port, (int *) &port_lb_phy);

            if (port_lb_phy){
                LOG_INFO(BSL_LS_SOC_PORT,
                         (BSL_META_U(unit,
                                     "port%d at loopback status.\n"), port));
                reg_addr = DRV_REG_ADDR(unit, INDEX(LNKSTSr), 0, 0);
                reg_len = DRV_REG_LENGTH_GET(unit, INDEX(LNKSTSr));

                if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, 
                        reg_len)) < 0) {
                    return rv;
                }
                if (reg_value & (1 << port)) {
                    *val = TRUE;
                } else {
                    *val = FALSE;
                }
                
                return SOC_E_NONE;
            }
            
            rv = soc_phyctrl_link_get(unit, port, &up);
            
            /* Section to prevent the runpacket issue on the GE port */
            if (IS_GE_PORT(unit, port)){
                if ((int_pc != NULL) && (ext_pc != NULL)  && 
                        (ext_pc != int_pc)){
                    rv = PHY_LINK_GET(int_pc->pd, unit, port, (&up));
                }
            }

            if (rv < 0){
                *val = FALSE;
            } else {
                *val = (up) ? TRUE : FALSE;
            }  

           /* If link down, set default COPPER mode. */
            if (*val) {
                
                /* 1. GE port get the medium from PHY.
                 * 2. FE port on geting medium will got COPPER medium properly 
                 *    - for the PHY_MEDIUM_GET will direct to the mapping 
                 *      routine in phy.c
                 */
                (DRV_SERVICES(unit)->port_get) \
                    (unit, port, 
                    DRV_PORT_PROP_PHY_MEDIUM, &phy_medium);
            }
            else {
                phy_medium = SOC_PORT_MEDIUM_COPPER;
            }

            /* Program MAC if medium mode is changed. */
            if (phy_medium != SOC_ROBO_PORT_MEDIUM_MODE(unit, port)) {
                /* SW port medium update */
                SOC_ROBO_PORT_MEDIUM_MODE_SET(unit, port, phy_medium);
            }

            break;
        case DRV_PORT_STATUS_LINK_SPEED:
            /* Section to prevent the runpacket issue on the GE port */
            if (IS_GE_PORT(unit, port)){
                if ((int_pc != NULL) && (ext_pc != NULL) && 
                        (ext_pc != int_pc)){
                    rv = PHY_SPEED_GET(int_pc->pd, unit, port, (&speed));
                } else {
                    rv = soc_phyctrl_speed_get(unit, port, &speed);
                }
            }

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s, Can't get the PHY speed!\n"), FUNCTION_NAME())); 
                return rv;
            }
            
            *val = (speed == 2500) ? DRV_PORT_STATUS_SPEED_2500M :
                    (speed == 2000) ? DRV_PORT_STATUS_SPEED_2G :
                    (speed == 1000) ? DRV_PORT_STATUS_SPEED_1G :
                    (speed == 100) ? DRV_PORT_STATUS_SPEED_100M :
                                    DRV_PORT_STATUS_SPEED_10M;
            
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_status_get: SPEED = %d\n"),
                      *val));
            break;
        case DRV_PORT_STATUS_LINK_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_status_get: DUPLEX\n")));
            
            /* Section to prevent the runpacket issue on the GE port */
            if (IS_GE_PORT(unit, port)){
                if ((int_pc != NULL) && (ext_pc != NULL) && 
                        (ext_pc != int_pc)){
                    rv = PHY_DUPLEX_GET(int_pc->pd, unit, port, (&duplex));
                } else {
                    rv = soc_phyctrl_duplex_get(unit, port, &duplex);
                }
            }

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s, Can't get the PHY duplex!\n"), FUNCTION_NAME())); 
                return rv;
            }
            
            *val = (duplex == TRUE) ? DRV_PORT_STATUS_DUPLEX_FULL :
                                    DRV_PORT_STATUS_DUPLEX_HALF;
            
            break;
        case DRV_PORT_STATUS_PROBE:
            *val = 0;
            rv = drv_robo_port_probe(unit, port, &okay);
            *val = okay;
            if (rv < 0) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "Port probe failed on port %s\n"),
                          SOC_PORT_NAME(unit, port)));
            }
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_status_get: PROBE %s\n"),
                      *val ? "OK" : "FAIL"));
            break;
        case DRV_PORT_STATUS_INIT:
            rv = drv_robo_port_sw_init_status_get(unit);
            if (rv == SOC_E_INIT) {
                *val = FALSE;
            } else {
                *val = TRUE;
            }
            break;
        case DRV_PORT_STATUS_PHY_DRV_NAME:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_gex_port_status_get: PHY_DRV_NAME\n")));
            SOC_ROBO_PORT_INIT(unit);            
            
            *val = PTR_TO_INT(soc_phyctrl_drv_name(unit, port));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}


