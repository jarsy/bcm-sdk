/*
 * $Id: port.c,v 1.15 Broadcom SDK $
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
#include "../common/robo_common.h"



/* The loopback set on ROBO's internal FE PHY can't work at speed=10.
 *  - tested on bcm5347 and found 2 cases with speed=10 but loopback still 
 *      working properly.
 *      a. set loopback at speed=10 and Auto-Neg is on.
 *      b. set loopback at speed=100 with AN=on and than set loopback to 
 *          speed=10 with AN=off
 */
#define ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED   0


/*
 *  Function : _drv_harrier_set_MAC_auto_negotiation
 *
 *  Purpose :
 *      Set PHY auto-nego through MAC register.
 *      (This is not suitable for ROBO)
 *
 *  Parameters :
 *      unit        :   unit id
 *      port    :   port id.
 *      enable  :   enable/disable MAC auto-negotiation.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
STATIC int 
_drv_harrier_set_MAC_auto_negotiation(int unit, int port, int enable)
{

  LOG_INFO(BSL_LS_SOC_PORT,
           (BSL_META_U(unit,
                       "_drv_harrier_set_MAC_auto_negotiation.\n")));
  
  return SOC_E_UNAVAIL;
}

/*
 *  Function : _drv_harrier_port_property_enable_set
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
_drv_harrier_port_property_enable_set(int unit, int port, int property, uint32 enable)
{
    uint32    addr = 0, temp = 0;
    int        rv = SOC_E_NONE;
    int         length = 0;
    uint32    reg_index = 0, fld_index = 0;
    uint32    reg_value;
    uint64    vlan_ctrl3_64, reg_value64;
    soc_pbmp_t pbmp;

    switch (property) {
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port)) {
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
            } else if (IS_GE_PORT(unit, port)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }

                soc_G_PCTLr_field_set(unit, &reg_value, 
                    MIRX_DISf, &temp);
                if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TX:
            if (IS_FE_PORT(unit, port)) {
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
            } else if (IS_GE_PORT(unit, port)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }
                soc_G_PCTLr_field_set(unit, &reg_value, 
                    MITX_DISf, &temp);
                if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }

            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_TXRX:
            if (IS_FE_PORT(unit, port)) {
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
            } else if (IS_GE_PORT(unit, port))  {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                if (enable) {
                    temp = 0;
                } else {
                    temp = 1;
                }

                soc_G_PCTLr_field_set(unit, &reg_value, 
                    MITX_DISf, &temp);
                soc_G_PCTLr_field_set(unit, &reg_value, 
                    MIRX_DISf, &temp);
                if ((rv = REG_WRITE_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
            if ((rv = REG_READ_VLAN_CTRL3r(
                unit, (uint32 *)&vlan_ctrl3_64)) < 0) {
                return rv;
            }
            soc_VLAN_CTRL3r_field_get(unit, (uint32 *)&vlan_ctrl3_64, 
                EN_DROP_NON1Qf, &temp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (enable) {
                SOC_PBMP_PORT_ADD(pbmp, port);
            } else {
                SOC_PBMP_PORT_REMOVE(pbmp, port);
            }
            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            soc_VLAN_CTRL3r_field_set(unit, (uint32 *)&vlan_ctrl3_64, 
                EN_DROP_NON1Qf, &temp);
            if ((rv = REG_WRITE_VLAN_CTRL3r(
                unit, (uint32 *)&vlan_ctrl3_64)) < 0) {
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
                (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (enable) {
                SOC_PBMP_PORT_ADD(pbmp, port);
            } else {
                SOC_PBMP_PORT_REMOVE(pbmp, port);
            }
            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);

            (DRV_SERVICES(unit)->reg_field_set)
                (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
            if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            if ((rv = REG_READ_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_DTAG_GLO_CTLr_field_set(unit, &reg_value, 
                EN_DTAG_ISPf, &enable);
            if ((rv = REG_WRITE_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            if ((rv = REG_READ_ISP_SEL_PORTMAPr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }

            soc_ISP_SEL_PORTMAPr_field_get(unit, (uint32 *)&reg_value64,
                ISP_PORT_PBMPf, &temp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            
            if (enable) {
                SOC_PBMP_PORT_ADD(pbmp, port);
            } else {
                SOC_PBMP_PORT_REMOVE(pbmp, port);
            }

            /* CPU is suggested to be ISP port (ASIC suggest it) */
            /* remarked : still allow user to set CPU to none-ISP for the
             * regression test have test item to set CPU to none-ISP.
            SOC_PBMP_PORT_ADD(pbmp, CMIC_PORT(unit));
            */

            temp = SOC_PBMP_WORD_GET(pbmp, 0);
            soc_ISP_SEL_PORTMAPr_field_set(unit, (uint32 *)&reg_value64,
                ISP_PORT_PBMPf, &temp);

            if ((rv = REG_WRITE_ISP_SEL_PORTMAPr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }
            break;
            
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            return SOC_E_UNAVAIL;
            break;
            
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            return SOC_E_UNAVAIL;
            break;
            
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            return SOC_E_UNAVAIL;
            break;
            
        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

/*
 *  Function : _drv_harrier_port_property_enable_get
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
_drv_harrier_port_property_enable_get(int unit, int port, int property, uint32 *enable)
{
    uint32    addr, temp = 0;
    int        rv = SOC_E_NONE;
    int        length;
    uint32    reg_index = 0, fld_index = 0;
    uint32    reg_value;
    uint64    vlan_ctrl3_64, reg_value64;
    soc_pbmp_t pbmp;

    switch (property) {
        case DRV_PORT_PROP_ENABLE_RX:
            if (IS_FE_PORT(unit, port)) {
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
            } else if (IS_GE_PORT(unit, port)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_G_PCTLr_field_get(unit, &reg_value, 
                    MIRX_DISf, &temp);
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
            if (IS_FE_PORT(unit, port)) {
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
            } else if (IS_GE_PORT(unit, port)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_G_PCTLr_field_get(unit, &reg_value, 
                    MITX_DISf, &temp);
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
            if (IS_FE_PORT(unit, port)) {
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
            } else if (IS_GE_PORT(unit, port)) {
                if ((rv = REG_READ_G_PCTLr(unit, port, &reg_value)) < 0) {
                    return rv;
                }
                soc_G_PCTLr_field_get(unit, &reg_value, 
                    MIRX_DISf, &temp);
                if (temp) {
                    *enable = FALSE;
                    return rv;
                }
                soc_G_PCTLr_field_get(unit, &reg_value, 
                    MITX_DISf, &temp);
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
                unit, (uint32 *)&vlan_ctrl3_64)) < 0) {
                return rv;
            }
            soc_VLAN_CTRL3r_field_get(unit, (uint32 *)&vlan_ctrl3_64, 
                EN_DROP_NON1Qf, &temp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
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
                (unit, addr, (uint32 *)&reg_value64, length)) < 0) {
                return rv;
            }
            (DRV_SERVICES(unit)->reg_field_get)
                (unit, reg_index, (uint32 *)&reg_value64, fld_index, &temp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);
            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            if ((rv = REG_READ_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_DTAG_GLO_CTLr_field_get(unit, &reg_value, 
                EN_DTAG_ISPf, &temp);
            *enable = temp;
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            if ((rv = REG_READ_ISP_SEL_PORTMAPr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }

            soc_ISP_SEL_PORTMAPr_field_get(unit, (uint32 *)&reg_value64,
                ISP_PORT_PBMPf, &temp);
            SOC_PBMP_WORD_SET(pbmp, 0, temp);

            if (SOC_PBMP_MEMBER(pbmp, port)) {
                *enable = TRUE;
            } else {
                *enable = FALSE;
            }
            break;

        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
            return SOC_E_UNAVAIL;
            break;
            
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
            return SOC_E_UNAVAIL;
            break;

        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            return SOC_E_UNAVAIL;
            break;

        default:
            return SOC_E_PARAM;
            break;
    }
    return SOC_E_NONE;
}

STATIC int
_drv_harrier_port_security_mode_set(int unit, uint32 port, uint32 mode, uint32 sa_num)
{
    uint32 temp = 0, trap = 0;
    uint32 reg_value;
    int rv = SOC_E_NONE;
    int change_eap = 0, change_trap = 0;

    /* the security mode */
    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            temp = 0;
            change_eap = 1;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            temp = 2;
            change_eap = 1;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            temp = 1;
            change_eap = 1;
            break;            
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            trap = 0; 
            change_trap = 1;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            trap = 1;
            change_trap = 1;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* write mode to chip */
    SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, port, &reg_value));
    if (change_eap) {
        SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set
                (unit, &reg_value, EAP_MODEf, &temp));
    }
    if (change_trap) {
        SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set
                (unit, &reg_value, SA_VIO_OPTf, &trap));
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SEC_CONr(unit, port, &reg_value));

    if (mode != DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM) {
        /* Config Port x Dynamic Learning Threshold register 
         *  - set SA_NO for MAC security (default value is 0x0000)
         */
        SOC_IF_ERROR_RETURN(REG_READ_PORT_MAX_LEARNr(unit, port, &reg_value));

        temp = 0;
        SOC_IF_ERROR_RETURN(soc_PORT_MAX_LEARNr_field_set
                (unit, &reg_value, DYN_MAX_MAC_NOf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_MAX_LEARNr(unit, port, &reg_value));
    }

    return rv;
}

STATIC int
_drv_harrier_port_security_mode_get(int unit, uint32 port, uint32 mode, 
    uint32 *prop_val)
{
    uint32 temp_mode, temp;
    uint32 reg_value;
    int rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, port, &reg_value));
    SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get
                (unit, &reg_value, EAP_MODEf, &temp_mode));
    SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get
                (unit, &reg_value, SA_VIO_OPTf, &temp));

    switch (mode) {
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            *prop_val = (temp_mode == 0) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            *prop_val = (temp_mode == 2) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            *prop_val = (temp_mode == 1) ? TRUE : FALSE;
            break;            
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            *prop_val = (temp == 0) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            *prop_val = (temp == 1) ? TRUE : FALSE;
            break;
        default:
            rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_harrier_port_set
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
drv_harrier_port_set(int unit, soc_pbmp_t bmp, uint32 prop_type, uint32 prop_val)
{
    uint32 reg_value, temp = 0;
    int port, conf_port = 0;
    uint64  reg_value64;
    soc_pbmp_t tmp_pbmp;
    int rv = SOC_E_NONE;
    uint8 cosq = 0;
#if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED
    int no_fe10_loopback = 0;   /* special limitation for ROBO fe phy */
#endif  /* #if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED */
    mac_driver_t *p_mac = NULL;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_harrier_port_set: unit=%d bmp=%x %x\n"), \
              unit, SOC_PBMP_WORD_GET(bmp, 0), SOC_PBMP_WORD_GET(bmp, 1)));

    SOC_ROBO_PORT_INIT(unit);

    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SPEED\n")));

            /* BCM53222 is 24FE+2FE */
            if (bcm53222_attached) {
                PBMP_ITER(bmp, port) {
                    if ((port == 25) || (port == 26)) {
                        if (prop_val == DRV_PORT_STATUS_SPEED_1G) {
                            return SOC_E_PARAM;
                        }
                    }
                }
            }

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
                default:
                    return SOC_E_PARAM;
            }
            PBMP_ITER(bmp, port) {
                /* set PHY and MAC auto-negotiation OFF */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
             
                /* set MAC auto-negotiation OFF */
                _drv_harrier_set_MAC_auto_negotiation(unit, port, FALSE);

                /* Set PHY registers anyway. */
                rv = soc_phyctrl_speed_set(unit, port, temp);

                /* if auto-negotiation is OFF, */
                /* MAC register(s) should be set also */
                p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
                if (p_mac != NULL) {
                    /* Set PHY registers anyway. */
                    if (p_mac->md_speed_set != NULL) {
                        rv = MAC_SPEED_SET(
                            p_mac, 
                            unit, port, temp);
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
                                 "drv_harrier_port_set: PROP_DUPLEX\n")));
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

                /* set MAC auto-negotiation OFF */
                _drv_harrier_set_MAC_auto_negotiation(unit, port, FALSE);

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
        case DRV_PORT_PROP_AUTONEG:
        case DRV_PORT_PROP_RESTART_AUTONEG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_AUTONEG\n")));
            /* RE_AN in PHY driver to set AN will be executed also */
            PBMP_ITER(bmp, port) {
                /* set PHY auto-negotiation */
                rv = soc_phyctrl_auto_negotiate_set(unit, port, prop_val);

                /* set MAC auto-negotiation */
                _drv_harrier_set_MAC_auto_negotiation(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_TX_PAUSE\n")));
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
                                 "drv_harrier_port_set: PROP_RX_PAUSE\n")));
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
                                 "drv_harrier_port_set: LOCAL_ADVER\n")));
            PBMP_ITER(bmp, port) {
                /* set advertise to PHY accordingly */
                rv = soc_phyctrl_adv_local_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: REMOTE_ADVER not support\n")));
            /* can not set remote advert */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PORT_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_MAC_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_PHY_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_ABILITY not support\n")));
            /* can not be set */
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_INTERFACE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_INTERFACE\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_interface_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_MAC_ENABLE:
            /* This case is called for _bcm_robo_port_update() only. */
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_MAC_ENABLE\n")));
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
                                 "drv_harrier_port_set: PROP_ENABLE\n")));
            PBMP_ITER(bmp, port) {
                /* for enable, set MAC first and than PHY.
                 * for disable, set PHY first and than MAC
                 */
                if (prop_val) {
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
                        rv = SOC_E_UNAVAIL;
                    }
                    SOC_IF_ERROR_RETURN(rv);
                    /* Set PHY registers anyway. */
                    rv = soc_phyctrl_enable_set(unit, port, prop_val);
                } else {
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
            }
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_ENABLE_SET\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_IPG_FE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_IPG_FE\n")));
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) { /* per system */
                if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val > 92) {
                    temp = 3;
                } else if (prop_val > 88) {
                    temp = 2;
                } else if (prop_val > 84) {
                    temp = 1;
                } else {
                    temp = 0;
                }
                soc_SWMODEr_field_set(unit, &reg_value,
                    IPGf, &temp);
                if ((rv = REG_WRITE_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_IPG_GE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_IPG_GE\n")));
            if (SOC_PBMP_EQ(bmp, PBMP_ALL(unit))) { /* per system */
                if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
                if (prop_val > 88) {
                    temp = 3;
                } else {
                    temp = 0;
                }
                soc_SWMODEr_field_set(unit, &reg_value,
                    IPGf, &temp);
                if ((rv = REG_WRITE_SWMODEr(unit, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;
        case DRV_PORT_PROP_JAM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_JAM\n")));
            
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
                                 "drv_harrier_port_set: PROP_BPDU_RX\n")));
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
                                 "drv_harrier_port_set: PROP_MAC_LOOPBACK\n")));
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
                                 "drv_harrier_port_set: PROP_PHY_LOOPBACK\n")));
            PBMP_ITER(bmp, port) {
                /* Special case :
                 *  - Design team confirmed that integrated FE PHY after 
                 *      robo5348 device will be limited to serve phy loopback
                 *      at 10MB speed.
                 */
#if ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED
                if (IS_FE_PORT(unit, port)){
                    no_fe10_loopback = (port < 24) ? 1 : 0;
                    if (no_fe10_loopback){
                        SOC_IF_ERROR_RETURN(
                                REG_READ_MIICTLr(unit, port, &reg_value));
                        soc_MIICTLr_field_get(unit, &reg_value, F_SPD_SELf, 
                                &temp);
                        if (!temp) {    /* F_SPD_SEL == b0 means speed=10 */
                            LOG_WARN(BSL_LS_SOC_COMMON,
                                     (BSL_META_U(unit,
                                                 "No loopback on port%d for speed=10!\n"),
                                      port));
                            return SOC_E_UNAVAIL;
                        }
                    }
                }
#endif  /* ROBO_INTFE_NO_FE10_LOOPBACK_CONFIRMED */
                rv = soc_phyctrl_loopback_set(unit, port, prop_val, TRUE);
            }
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_MEDIUM not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_MDIX\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_mdix_set(unit, port, prop_val); 
            }
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_MDIX_STATUS not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_MS not support\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_master_set(unit, port, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MODE_NONE\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MODE_STATIC_ACCEPT\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MODE_STATIC_REJECT\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MODE_DYNAMIC_SA_NUM\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_security_mode_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MODE_DYNAMIC_SA_MATCH\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_EXTEND:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MAC_MODE_EXTEND\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;

        case DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_SEC_MAC_MODE_SIMPLIFY\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_security_mode_set(unit, port, prop_type, 0);
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP\n")));
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, TRUE));
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, TRUE));
               temp = ((int)prop_val < 0) ? 0 : prop_val;
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET(unit, 
                        port, DRV_PORT_SA_LRN_CNT_LIMIT, (int)temp));
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU\n")));
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY, TRUE));
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, TRUE));
                temp = ((int)prop_val < 0) ? 0 : prop_val;
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET(unit, 
                        port, DRV_PORT_SA_LRN_CNT_LIMIT, (int)temp));
            }
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            PBMP_ITER(bmp, port) {
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_NONE, TRUE));
                /* clear learn limit */
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                        (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, 0));
            }
            break;

        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION\n")));
            /* harrier can be at action in one of NONE, CPU-ONLY and DROP */
            temp = (prop_val & DRV_PORT_LEARN_LIMIT_ACTION_MASK);
            if (temp == DRV_PORT_LEARN_LIMIT_ACTION_COPY2CPU) {
               return SOC_E_UNAVAIL;
            }
            PBMP_ITER(bmp, port) {
                if (temp == DRV_PORT_LEARN_LIMIT_ACTION_NONE) {
                    SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                           (unit, port,
                            DRV_PORT_PROP_SEC_MAC_MODE_NONE, TRUE));
                    /* clear learn limit : 
                    *  - set limit to 0 to ensure the action-none can works properly.
                    */
                    SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_SET
                            (unit, port, DRV_PORT_SA_LRN_CNT_LIMIT, 0));
               } else {
                    /* harrier in this section can only be drop or CPU-ONLY */
                    if (temp == DRV_PORT_LEARN_LIMIT_ACTION_DROP) {
                        SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                                (unit, port,
                                DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, TRUE));
                    } else {
                        SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_set
                                (unit, port,
                                DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY, TRUE));
                    }
                }
            }
            break; 

        case DRV_PORT_PROP_PHY_LINKUP_EVT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_LINKUP_EVT\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkup_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_LINKDN_EVT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_LINKDN_EVT\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phyctrl_linkdn_evt(unit, port);
                if (SOC_FAILURE(rv) && (rv != SOC_E_UNAVAIL)) {
                    return rv;
                }
                rv = SOC_E_NONE;
            }
            break;
        case DRV_PORT_PROP_PHY_RESET:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_PHY_RESET\n")));
            PBMP_ITER(bmp, port) {
                rv = soc_phy_reset(unit, port);
            }
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_DTAG_MODE\n")));
            rv = _drv_harrier_port_property_enable_set(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_DTAG_ISP_PORT\n")));
            PBMP_ITER(bmp, port) {
                rv = _drv_harrier_port_property_enable_set(
                    unit, port, prop_type, prop_val);
            }
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_DTAG_TPID\n")));
            if ((rv = REG_READ_ISP_VIDr(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_ISP_VIDr_field_set(unit, &reg_value,
                ISP_VLAN_DELIMITERf, &prop_val);
            if ((rv = REG_WRITE_ISP_VIDr(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_802_1X_MODE :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_802_1X_MODE\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_802_1X_BLK_RX :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_set: PROP_802_1X_BLK_RX\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            rv = ((DRV_SERVICES(unit)->vlan_prop_port_enable_set)
                (unit, DRV_VLAN_PROP_MAC2V_PORT, bmp,
                prop_val));
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
                                 "drv_harrier_port_set: PROP_INGRESS_VLAN_CHK\n")));
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                INGR_VID_CHKf, &prop_val);
            if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
            /*
             * The following 2 steps are implemented for sFlow configuration.
             * Step 1. Enable or disable the port's ingress sFlow.
             * Step 2. Set sFlow rate if it's not a disabled port.
             */
            /* 
             * Step 1. enable or disable port's sFlow.
             */
            if ((rv = REG_READ_INGRESS_RMONr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }
            soc_INGRESS_RMONr_field_get(unit, (uint32 *)&reg_value64,
                EN_INGRESS_PORTMAPf, &temp);
            SOC_PBMP_WORD_SET(tmp_pbmp, 0, temp);

            if (!prop_val) {
                /* disable the port's sFlow */
                SOC_PBMP_REMOVE(tmp_pbmp, bmp);
            } else {
                /* enable the port's sFlow */
                SOC_PBMP_OR(tmp_pbmp, bmp);
            }

            temp = SOC_PBMP_WORD_GET(tmp_pbmp, 0);
            soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64,
                EN_INGRESS_PORTMAPf, &temp);

            /* 
             * Step 2. Set sample rate. 
             * If the port is going to be disabled, 
             * no sample rate configuration needed.
             */
            if (prop_val) {
                temp = drv_robo_port_sample_rate_get(unit, prop_val);
                if (!temp) {
                    /* 
                     * For harrier, 
                     * sample rate 1/1 is not supported.
                     */
                    return SOC_E_PARAM;
                } else {
                    temp -= 1;
                }
                soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64,
                    INGRESS_CFGf, &temp);

                /* update software copy of enabled ports. */
                PBMP_ITER(bmp, port) {
                    SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = 
                        prop_val;
                }
                /* 
                 * Then, update software copy of other enabled ports. 
                 * since all enabled ports share one configured value.
                 */
                PBMP_ITER(PBMP_ALL(unit), port) {
                    if (SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate) {
                        SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = 
                            prop_val;
                    }
                }
            } else {
                /* update software copy of disabled ports. */
                PBMP_ITER(bmp, port) {
                    SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate = 
                        prop_val;
                }
            }

            if ((rv = REG_WRITE_INGRESS_RMONr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
            if ((rv = REG_READ_EGRESS_RMONr(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_EGRESS_RMONr_field_get(unit, &reg_value, 
                EGRESS_Pf, &temp);
            port = (int)temp;
            if (SOC_PORT_VALID(unit, port)) {
                /* 
                 * If the port that desired to disable is not 
                 * the previous enabled port, 
                 * do nothing.
                 */
                if ((!prop_val) && !SOC_PBMP_MEMBER(bmp, port)) {
                    return SOC_E_NONE;
                }
            } else {
                /* 
                 * If the value of field EGRESS_Pf is not a valid port,
                 * the field is not initialized or configured yet. Only
                 * the parameter "prop_val" needs to be checked.
                 */
                if (!prop_val) {
                    return SOC_E_NONE;
                }
            }

            if (!prop_val) {
                /* disable the port's sFlow */
                temp = 0;
            } else {
                /* enable the port's sFlow */
                temp = 1;
            }
            soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                EN_EGRESS_RMONf, &temp);

            PBMP_ITER(bmp, port) {
                soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                    EGRESS_Pf, (uint32 *)&port);
                conf_port = port;
            }

            if (prop_val) {
                temp = drv_robo_port_sample_rate_get(unit, prop_val);
                if (!temp) {
                    /* 
                     * For harrier, 
                     * sample rate 1/1 is not supported.
                     */
                    return SOC_E_PARAM;
                } else {
                    temp -= 1;
                }
                soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                    EGRESS_CFGf, &temp);

                /* 
                 * Clear software copy of all ports. 
                 * Since there is only one port can be 
                 * egress sample port at one time.
                 */
                PBMP_ITER(PBMP_ALL(unit), port) {
                    SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate = 0;
                }

                /* Only update software copy of the configured port. */
                SOC_ROBO_PORT_INFO(unit, conf_port).eg_sample_rate = prop_val;
            } else {
                /* 
                 * Clear software copy of all ports. 
                 * Since there is only one port can be 
                 * egress sample port at one time.
                 */
                PBMP_ITER(PBMP_ALL(unit), port) {
                    SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate = 0;
                }
            }
            if ((rv = REG_WRITE_EGRESS_RMONr(unit, &reg_value)) < 0) {
                return rv;
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
            if ((rv = REG_READ_INGRESS_RMONr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->queue_prio_get)
                                (unit, -1, prop_val, &cosq));

            temp = cosq;
            soc_INGRESS_RMONr_field_set(unit, (uint32 *)&reg_value64, 
                INGRESS_PRIf, &temp);

            if ((rv = REG_WRITE_INGRESS_RMONr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }

            /* 
             * Priority of sFlow packets is a global configuration, 
             * save the configured value at port #0.
             */
            port = 0;
            SOC_ROBO_PORT_INFO(unit, port).ing_sample_prio = prop_val;
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:

            if ((rv = REG_READ_EGRESS_RMONr(unit, &reg_value)) < 0) {
                return rv;
            }

            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->queue_prio_get)
                                (unit, -1, prop_val, &cosq));
            temp = cosq;
            soc_EGRESS_RMONr_field_set(unit, &reg_value, 
                EGRESS_PRIf, &temp);

            if ((rv = REG_WRITE_EGRESS_RMONr(unit, &reg_value)) < 0) {
                return rv;
            }

            /* 
             * Priority of sFlow packets is a global configuration, 
             * save the configured value at port #0.
             */
            port = 0;
            SOC_ROBO_PORT_INFO(unit, port).eg_sample_prio = prop_val;
            break;
        case DRV_PORT_PROP_ROAMING_OPT:
            if (prop_val & ~((uint32)DRV_SA_MOVE_ARL)){
                /* harrier support DRV_SA_MOVE_ARL only */
                rv = SOC_E_UNAVAIL;
            } else {
                PBMP_ITER(bmp, port) {
                    SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, 
                            port, &reg_value));
                    temp = (prop_val & DRV_SA_MOVE_ARL) ? 1 : 0;
                    SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_set(unit, 
                            &reg_value, ROAMING_OPTf, &temp));
                    SOC_IF_ERROR_RETURN(REG_WRITE_PORT_SEC_CONr(unit, 
                            port, &reg_value));
                }
            }
            break;
        /* not supported list */
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
        case DRV_PORT_PROP_MIB_CLEAR:
        case DRV_PORT_PROP_PPPOE_PARSE_EN:
            rv = SOC_E_UNAVAIL;
            break;
        default: 
            rv = SOC_E_PARAM; 
            break;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_harrier_port_set: Exit\n")));
    return rv;
}

/*
 *  Function : drv_harrier_port_get
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
int drv_harrier_port_get(int unit, int port, uint32 prop_type, uint32 *prop_val)
{
    int         rv = SOC_E_NONE;
    uint32     reg_value, temp = 0;
    uint32     mac_ability = 0, phy_ability;
    int        pause_tx, pause_rx;
    int        autoneg, done;
    soc_pbmp_t tmp_pbmp;
    uint64  reg_value64;
    mac_driver_t *p_mac = NULL;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_harrier_port_get: unit=%d port=%d\n"), unit, port));

    SOC_ROBO_PORT_INIT(unit);
    p_mac = SOC_ROBO_PORT_MAC_DRIVER(unit, port);
    switch (prop_type) {
        case DRV_PORT_PROP_SPEED:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: Speed\n")));
            temp = 0;

            rv = soc_phyctrl_speed_get(unit, port, (int *) &temp);
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
                default:
                    *prop_val = 0;
                    break;
            }
            break;
        case DRV_PORT_PROP_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: Duplex\n")));
            temp = 0;

            rv = soc_phyctrl_duplex_get(unit, port, (int *) &temp);
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
                                 "drv_harrier_port_get: Autoneg\n")));
            
            rv = soc_phyctrl_auto_negotiate_get(unit, port,
                                                    &autoneg, &done);
            *prop_val = (autoneg) ? DRV_PORT_STATUS_AUTONEG_ENABLE :
                        DRV_PORT_STATUS_AUTONEG_DISABLED;
            break;
        case DRV_PORT_PROP_TX_PAUSE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: TX Pause\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac, 
                        unit, port, (int *) prop_val, &pause_rx);
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
                                 "drv_harrier_port_get: RX Pause\n")));
            if (p_mac != NULL) {
                if (p_mac->md_pause_get != NULL) {
                    rv = MAC_PAUSE_GET(p_mac,
                        unit, port, &pause_tx, (int *) prop_val);
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
                                 "drv_harrier_port_get: Local Advertise\n")));
            rv = soc_phyctrl_adv_local_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_REMOTE_ADVERTISE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: Remote Advertise\n")));
            /* if auto-negotiation is ON and negotiation is completed */
            /*   get remote advertisement from PHY */
            rv = soc_phyctrl_adv_remote_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PORT_ABILITY:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: Port Ability\n")));
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
                                 "drv_harrier_port_get: MAC Ability\n")));
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
                                 "drv_harrier_port_get: PHY Ability\n")));
            rv = soc_phyctrl_ability_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_INTERFACE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: Interface\n")));
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
                                 "drv_harrier_port_get: Enable\n")));
            rv = soc_phyctrl_enable_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_ENABLE_DROP_1Q:
        case DRV_PORT_PROP_ENABLE_DROP_NON1Q:
        case DRV_PORT_PROP_ENABLE_RX:
        case DRV_PORT_PROP_ENABLE_TX:
        case DRV_PORT_PROP_ENABLE_TXRX:
        case DRV_PORT_PROP_EGRESS_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_CFI_REMARK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: Enable Get\n")));
            rv = _drv_harrier_port_property_enable_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_IPG_FE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: IPG FE\n")));
            if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_SWMODEr_field_get(unit, &reg_value,
                IPGf, &temp);
            switch (temp) {
                case 0:
                    *prop_val = 84;
                    break;
                case 1:
                    *prop_val = 88;
                    break;
                case 2:
                    *prop_val = 92;
                    break;
                case 3:
                    *prop_val = 96;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }
            break;
        case DRV_PORT_PROP_IPG_GE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: IPG GE\n")));
            if ((rv = REG_READ_SWMODEr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_SWMODEr_field_get(unit, &reg_value,
                IPGf, &temp);
            switch (temp) {
                case 0:
                case 1:
                case 2:
                    *prop_val = 88;
                    break;
                case 3:
                    *prop_val = 96;
                    break;
                default:
                    rv = SOC_E_INTERNAL;
                    break;
            }
            break;
        case DRV_PORT_PROP_JAM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: JAM\n")));
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
                                 "drv_harrier_port_get: BPDU RX\n")));
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
                                 "drv_harrier_port_get: PROP_RESTART_AUTONEG not support\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_MAC_LOOPBACK:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_MAC_LOOPBACK\n")));
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
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_PHY_LOOPBACK\n")));
            rv = soc_phyctrl_loopback_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_PHY_MEDIUM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_PHY_MEDIUM\n")));
            rv = soc_phyctrl_medium_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_PHY_MDIX\n")));
            rv = soc_phyctrl_mdix_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_PHY_MDIX_STATUS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_PHY_MDIX_STATUS\n")));
            rv = soc_phyctrl_mdix_status_get(unit, port, prop_val);
            break;
        case DRV_PORT_PROP_MS:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_MS\n")));
            rv = soc_phyctrl_master_get(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_NONE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_SEC_MODE_NONE\n")));
            rv = _drv_harrier_port_security_mode_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_ACCEPT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_SEC_MODE_STATIC_ACCEPT\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_STATIC_REJECT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_SEC_MODE_STATIC_REJECT\n")));
            rv = SOC_E_UNAVAIL;
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_SEC_MODE_DYNAMIC_SA_NUM\n")));
            /* worse case
             * While the prop_val = 0, it has two meannings. 
             * One is the SA_NUM mode is not enabling.
             * The other is the SA_NUM is enabling and the number of SA is zero.
             * We assume that the BCM layer has software 
             * database to distinguish them.
             */
             rv = _drv_harrier_port_security_mode_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_SEC_MODE_DYNAMIC_SA_MATCH\n")));
            rv = _drv_harrier_port_security_mode_get(
                unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP\n")));

            temp = 0;
            SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                    (unit, port, 
                    DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            if (temp) {
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, &temp));
                /* temp=0 is Drop; temp=1 is Trap */
                temp = (temp == 0) ? TRUE : FALSE;
            }

            *prop_val = (temp) ? TRUE : FALSE;
            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU\n")));
            temp = 0;
            SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                    (unit, port, 
                    DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            if (temp) {
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                        (unit, port, 
                        DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, &temp));
            }
            *prop_val = (temp) ? TRUE : FALSE;

            break;
        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE:
            temp = FALSE;
            SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                    (unit, port, 
                    DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));

            *prop_val = (!temp) ? TRUE : FALSE;
            break;

        case DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION:
            /* harrier can be at action in one of NONE, CPU-ONLY and DROP */
            
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION\n")));
            temp = 0;
            SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                   (unit, port, 
                   DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM, &temp));
            if (temp) {
                SOC_IF_ERROR_RETURN(_drv_harrier_port_security_mode_get
                        (unit, port,
                         DRV_PORT_PROP_SEC_MAC_MODE_EXTEND, &temp));
                if (temp) {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_DROP; 
                } else {
                    *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_REDIRECT2CPU; 
                }
            } else {
                *prop_val = DRV_PORT_LEARN_LIMIT_ACTION_NONE;
            }

            break;

        case DRV_PORT_PROP_PHY_CABLE_DIAG:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_PHY_CABLE_DIAG\n")));
            rv = soc_phyctrl_cable_diag(unit, port, 
                                        (soc_port_cable_diag_t *)prop_val);
            break;
        case DRV_PORT_PROP_PHY_LINK_CHANGE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_PHY_LINKCHANGE\n")));
            rv = soc_phyctrl_link_change(unit, port, (int *) prop_val);
            break;
        case DRV_PORT_PROP_DTAG_MODE:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_DTAG_MODE\n")));
            rv = _drv_harrier_port_property_enable_get(unit, 0, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_ISP_PORT:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_DTAG_ISP_PORT\n")));
            rv = _drv_harrier_port_property_enable_get(unit, port, prop_type, prop_val);
            break;
        case DRV_PORT_PROP_DTAG_TPID:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_DTAG_TPID\n")));
            if ((rv = REG_READ_ISP_VIDr(unit, &reg_value)) < 0) {
                return rv;
            }

            soc_ISP_VIDr_field_get(unit, &reg_value,
                ISP_VLAN_DELIMITERf, &temp);
            *prop_val = temp;
            break;
        case DRV_PORT_PROP_802_1X_MODE :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_802_1X_MODE\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_802_1X_BLK_RX :
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_get: PROP_802_1X_BLK_RX\n")));
            rv = SOC_E_PARAM;
            break;
        case DRV_PORT_PROP_MAC_BASE_VLAN:
            rv = ((DRV_SERVICES(unit)->vlan_prop_port_enable_get)
                (unit, DRV_VLAN_PROP_MAC2V_PORT, port, &temp));
            *prop_val = temp;
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
                                 "drv_harrier_port_get: PROP_INGRESS_VLAN_CHK\n")));
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            
            soc_VLAN_CTRL4r_field_get(unit, &reg_value, 
                INGR_VID_CHKf, prop_val);
            
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_RATE:
            if ((rv = REG_READ_INGRESS_RMONr(
                unit,(uint32 *)&reg_value64)) < 0) {
                return rv;
            }
            soc_INGRESS_RMONr_field_get(unit, (uint32 *)&reg_value64,
                EN_INGRESS_PORTMAPf, &temp);
            SOC_PBMP_WORD_SET(tmp_pbmp, 0, temp);

            if (SOC_PBMP_MEMBER(tmp_pbmp, port)) {
                *prop_val = SOC_ROBO_PORT_INFO(unit, port).ing_sample_rate;
            } else {
                *prop_val = 0;
            }
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_RATE:
            if ((rv = REG_READ_EGRESS_RMONr(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_EGRESS_RMONr_field_get(unit, &reg_value, 
                EGRESS_Pf, &temp);
            if (temp != port) {
                *prop_val = 0;
            } else {
                soc_EGRESS_RMONr_field_get(unit, &reg_value, 
                    EN_EGRESS_RMONf, &temp);
                if (!temp) {
                    *prop_val = 0;
                } else {
                    *prop_val = SOC_ROBO_PORT_INFO(unit, port).eg_sample_rate;
                }
            }
            break;
        case DRV_PORT_PROP_SFLOW_INGRESS_PRIO:
            *prop_val = SOC_ROBO_PORT_INFO(unit, port).ing_sample_prio;
            break;
        case DRV_PORT_PROP_SFLOW_EGRESS_PRIO:
            *prop_val = SOC_ROBO_PORT_INFO(unit, port).eg_sample_prio;
            break;
        case DRV_PORT_PROP_ROAMING_OPT:
            SOC_IF_ERROR_RETURN(REG_READ_PORT_SEC_CONr(unit, 
                    port, &reg_value));
            SOC_IF_ERROR_RETURN(soc_PORT_SEC_CONr_field_get(unit, 
                    &reg_value, ROAMING_OPTf, &temp));
            *prop_val = (temp) ? DRV_SA_MOVE_ARL : 0;
            break;
            
        /* not supported list */
        case DRV_PORT_PROP_EGRESS_DSCP_REMARK:
        case DRV_PORT_PROP_DEFAULT_TC_PRIO:
        case DRV_PORT_PROP_DEFAULT_DROP_PRECEDENCE:
        case DRV_PORT_PROP_UNTAG_DEFAULT_TC:
        case DRV_PORT_PROP_EGRESS_ECN_REMARK:
        case DRV_PORT_PROP_EGRESS_C_PCP_REMARK:
        case DRV_PORT_PROP_EGRESS_S_PCP_REMARK:
        case DRV_PORT_PROP_MIB_CLEAR:
        case DRV_PORT_PROP_PPPOE_PARSE_EN:
            rv = SOC_E_UNAVAIL;
            break;
        default: 
            rv = SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_harrier_port_status_get
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
drv_harrier_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr = 0, reg_len = 0, temp = 0;
    int     okay = 0;
    uint32  phy_medium = SOC_PORT_MEDIUM_COPPER;
    uint32  port_lb_phy = 0;
    uint64  reg_value64;
    soc_pbmp_t pbmp;
    int up = 0;
    int speed = 0;
    int duplex = 0;
    
    /* int_pd and pd used to prevent the runpacket issue on the GE port.
     * (with Internal SerDes bounded)
     */
    phy_ctrl_t      *int_pc = NULL, *ext_pc = NULL;  
    
    COMPILER_64_ZERO(reg_value64);
    
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
                             "drv_harrier_port_status_get: DETACH %s\n"),
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

                if ((rv = DRV_REG_READ(unit, reg_addr, 
                        (uint32 *)&reg_value64, reg_len)) < 0) {
                    return rv;
                }
                DRV_REG_FIELD_GET(unit, INDEX(LNKSTSr), 
                        (uint32 *)&reg_value64, INDEX(LNK_STSf), 
                        &temp);
                SOC_PBMP_WORD_SET(pbmp, 0, temp);
                if (SOC_PBMP_MEMBER(pbmp, port)) {
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
                    (speed == 1000) ? DRV_PORT_STATUS_SPEED_1G :
                    (speed == 100) ? DRV_PORT_STATUS_SPEED_100M :
                                    DRV_PORT_STATUS_SPEED_10M;
                
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_hrrier_port_status_get: SPEED = %d\n"),
                      *val));
            break;
        case DRV_PORT_STATUS_LINK_DUPLEX:
            LOG_INFO(BSL_LS_SOC_PORT,
                     (BSL_META_U(unit,
                                 "drv_harrier_port_status_get: DUPLEX\n")));
            
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
                                 "drv_harrier_port_status_get: PROBE %s\n"),
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
                                 "drv_harrier_port_status_get: PHY_DRV_NAME\n")));
            SOC_ROBO_PORT_INIT(unit);            
            
            *val = PTR_TO_INT(soc_phyctrl_drv_name(unit, port));
            break;
        default:
            rv = SOC_E_PARAM;
    }
    return rv;
}

/*
 *  Function : drv_harrier_port_pri_mapop_set
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration set
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      op_type     :   operation type
 *      pri_old     :   old priority.
 *      pri_new     :   new priority.
 *      cfi_new     :   new cfi.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_harrier_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new)
{
    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
        case DRV_PORT_OP_NORMAL_TC2PCP:
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            return SOC_E_UNAVAIL;
        default:
            break;
    }
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_port_pri_mapop_get
 *
 *  Purpose :
 *      Port basis priority mapping operation configuration get
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port id.
 *      pri_old     :   (in)old priority.
 *      cfi_old     :   (in)old cfi (No used).
 *      pri_new     :   (out)new priority.
 *      cfi_new     :   (out)new cfi.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. This driver service is designed for priority operation exchange.
 *  2. Priority type could be dot1p, DSCP, port based.
 *
 */
int 
drv_harrier_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new)
{
    switch (op_type) {
        case DRV_PORT_OP_PCP2TC :
        case DRV_PORT_OP_NORMAL_TC2PCP:
        case DRV_PORT_OP_OUTBAND_TC2PCP:
            return SOC_E_UNAVAIL;
        default:
            break;
    }

    return SOC_E_NONE;
}


