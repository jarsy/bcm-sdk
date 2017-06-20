/*
 * $Id: sec.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>


#define BCM_TBX_EAP_PORT_BLOCK_NONE    0
#define BCM_TBX_EAP_PORT_BLOCK_INGRESS    1
#define BCM_TBX_EAP_PORT_BLOCK_INGRESS_EGRESS    2

#define BCM_TBX_EAP_PORT_BLOCK_MASK    0x3

STATIC int
_drv_tbx_security_init(int unit)
{
    uint32 reg_value32, temp;
    int rv = SOC_E_NONE;

    /* pass special frame while block mode is set  */
    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_BYPASS_CTLr
        (unit, &reg_value32));
    
    temp = 1;
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_L2_USER_ADDRf, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_ARPf, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_MAC_22_2Ff, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_MAC_21f, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_MAC_20f, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_MAC_11_1Ff, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_MAC_10f, &temp));
    SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
        (unit, &reg_value32, EAP_BYPASS_MAC_0Xf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_BYPASS_CTLr
        (unit, &reg_value32));

    return rv;
}


/*
 *  Function : drv_tbx_security_set
 *
 *  Purpose :
 *      Set the security mask to teh selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      state   :   port state.
 *      mask     :   security mask.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_tbx_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask)
{
    int  rv = SOC_E_NONE;
    uint64  reg_value64, temp64;
    uint32 reg_value32, temp, temp_mask, temp_lo, temp_hi, flag = 0;
    uint32  port;
    l2_arl_sw_entry_t   arl_entry;
    uint32  sec_mask;
    
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_tbx_security_set: unit = %d, bmp= 0x%x, state = %d, mask = 0x%x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), state, mask));
    switch (state) {
        case DRV_SECURITY_PORT_UNCONTROLLED:
            /* 
              * 1. EAP Port Block (port is not blocked)
              * 2. Enble l2 learning
              */

            /* Disable 802.1x */

            /* 
              * To indicate EAP PDU CPUCopy is enable (default) or 
              * not (drop EAP packet) for all ports (now only for TB)
              */
            if (mask & DRV_SECURITY_RX_EAP_DROP) {
                temp_mask = 0;
            } else {
                temp_mask = 1;
            }
            SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
                (unit, &reg_value32));
            SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
                (unit, &reg_value32, RX_EAP_ENf, &temp));
            /* check original setting */
            if (temp != temp_mask) {
                SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
                    (unit, &reg_value32, RX_EAP_ENf, &temp_mask));

                SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
                    (unit, &reg_value32));
            }

            /* EAP Port Control Registers (Global) */
            SOC_IF_ERROR_RETURN(REG_READ_EAP_PORT_CTLr
                (unit, (void *)&reg_value64));
            SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_get
                (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

            if (SOC_PBMP_EQ(PBMP_E_ALL(unit), bmp)) {
                COMPILER_64_ZERO(temp64);
            } else {
                COMPILER_64_TO_32_LO(temp_lo, temp64);
                COMPILER_64_TO_32_HI(temp_hi, temp64);

                PBMP_ITER(bmp, port) {
                    if (port < SOC_ROBO_MAX_NUM_PORTS) {
                        temp = BCM_TBX_EAP_PORT_BLOCK_NONE;
                        /* Port is not blocked */
                        temp_mask = BCM_TBX_EAP_PORT_BLOCK_MASK;
                        if (port > 15) {
                            temp_mask = ~(temp_mask << ((port - 16)*2));
                            temp_hi &= temp_mask;
                        } else {
                            temp_mask = ~(temp_mask << (port*2));
                            temp_lo &= temp_mask;
                        }
                    }
                }
                COMPILER_64_SET(temp64, temp_hi, temp_lo);
            }

            SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_set
                (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));
            SOC_IF_ERROR_RETURN(REG_WRITE_EAP_PORT_CTLr
                (unit, (void *)&reg_value64));

            /* Enable l2 learning */
            SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_ENABLE_SET
                (unit, bmp, DRV_PORT_HW_LEARN));
            
            break;

        case DRV_SECURITY_PORT_UNAUTHENTICATE:
            /* 1. Disbale l2 learnning
              * 2. Remove mac address associted with this port
              * 3. EAP Port Block (port is blocked Ingress side)
              *     if (BLOCK_INOUT) -> port blocked in both ingress and egress side
              */
            SOC_IF_ERROR_RETURN(_drv_tbx_security_init(unit));

            /* 
              * To indicate EAP PDU CPUCopy is enable (default) or 
              * not (drop EAP packet) for all ports (now only for TB)
              */
            if (mask & DRV_SECURITY_RX_EAP_DROP) {
                temp_mask = 0;
            } else {
                temp_mask = 1;
            }
            SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
                (unit, &reg_value32));
            SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
                (unit, &reg_value32, RX_EAP_ENf, &temp));
            /* check original setting */
            if (temp != temp_mask) {
                SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
                    (unit, &reg_value32, RX_EAP_ENf, &temp_mask));

                SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
                    (unit, &reg_value32));
            }

            /* Disable l2 learning */
            SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_ENABLE_SET
                (unit, bmp, DRV_PORT_DISABLE_LEARN));

            /* EAP Port Control Registers (Global) */
            SOC_IF_ERROR_RETURN(REG_READ_EAP_PORT_CTLr
                (unit, (void *)&reg_value64));
            SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_get
                (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

            COMPILER_64_TO_32_LO(temp_lo, temp64);
            COMPILER_64_TO_32_HI(temp_hi, temp64);

            PBMP_ITER(bmp, port) {
                if (port < SOC_ROBO_MAX_NUM_PORTS) {
                    /* Remove the l2 mac address associated with this port */
                    flag |= DRV_MEM_OP_DELETE_BY_PORT |
                        DRV_MEM_OP_DELETE_BY_STATIC;
                    sal_memset(&arl_entry, 0, sizeof(l2_arl_sw_entry_t));
                    temp = port;
                    rv = soc_L2_ARL_SWm_field_set(unit, (uint32 *)&arl_entry,
                        PORTIDf,  &temp);
                    SOC_IF_ERROR_RETURN(rv);
                    if ((rv = DRV_MEM_DELETE
                        (unit, DRV_MEM_ARL, (uint32 *)&arl_entry, flag)) < 0) {
                        return rv;
                    }

                    /* Blocked in ingress or both ingress and egress side*/
                    temp = (mask & DRV_SECURITY_BLOCK_INOUT) ?
                                BCM_TBX_EAP_PORT_BLOCK_INGRESS_EGRESS :
                                    BCM_TBX_EAP_PORT_BLOCK_INGRESS;

                    temp_mask = BCM_TBX_EAP_PORT_BLOCK_MASK;
                    if (port > 15) {
                        temp = (temp << ((port - 16)*2));
                        temp_mask = ~(temp_mask << ((port - 16)*2));
                        temp_hi &= temp_mask;
                        temp_hi |= temp;
                    } else {
                        temp = (temp << (port*2));
                        temp_mask = ~(temp_mask << (port*2));
                        temp_lo &= temp_mask;
                        temp_lo |= temp;
                    }
                }
            }
            COMPILER_64_SET(temp64, temp_hi, temp_lo);
            SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_set
                (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));
            SOC_IF_ERROR_RETURN(REG_WRITE_EAP_PORT_CTLr
                (unit, (void *)&reg_value64));

            break;

        case DRV_SECURITY_PORT_AUTHENTICATED:
            /* 1. Set security mode = NONE (default), 
                    it's depends on customer's definition.
                2. EAP Port Block (port is not blocked)
                3. Enable l2 learning mode !
            */
            SOC_IF_ERROR_RETURN(_drv_tbx_security_init(unit));
                
            SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_AUTH_SEC_MODE, &sec_mask));         

            if (!(mask & sec_mask)) {
                    return SOC_E_UNAVAIL;                
            }

            /* 
              * To indicate EAP PDU CPUCopy is enable (default) or 
              * not (drop EAP packet) for all ports (now only for TB)
              */
            if (mask & DRV_SECURITY_RX_EAP_DROP) {
                temp_mask = 0;
            } else {
                temp_mask = 1;
            }
            SOC_IF_ERROR_RETURN(REG_READ_SECURITY_GLOBAL_CTLr
                (unit, &reg_value32));
            SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_get
                (unit, &reg_value32, RX_EAP_ENf, &temp));
            /* check original setting */
            if (temp != temp_mask) {
                SOC_IF_ERROR_RETURN(soc_SECURITY_GLOBAL_CTLr_field_set
                    (unit, &reg_value32, RX_EAP_ENf, &temp_mask));

                SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_GLOBAL_CTLr
                    (unit, &reg_value32));
            }

            /* set security mode */
            if (mask & DRV_SECURITY_VIOLATION_SA_MATCH) {
                if (mask & DRV_SECURITY_LEARN){
                    /* if we have mac sec criterion,  should not be learnd*/ 
                    return SOC_E_PARAM;
                }
            }

             /* default at no secutiry mode */
             temp = 0;
            if (mask & DRV_SECURITY_VIOLATION_NONE) {
                temp = DRV_PORT_PROP_SEC_MAC_MODE_NONE;
            }
            
            if (mask & DRV_SECURITY_VIOLATION_SA_MATCH) {
                /* SA violation will be drop or trap*/
                if (mask & DRV_SECURITY_EAP_MODE_EXTEND) {
                    temp = DRV_PORT_PROP_SA_UNKNOWN_DROP;
                } else if (mask & DRV_SECURITY_EAP_MODE_SIMPLIFIED) {
                    temp = DRV_PORT_PROP_SA_UNKNOWN_CPUCOPY;
                } else {
                    return SOC_E_PARAM;
                }
            }

            if (mask & DRV_SECURITY_VIOLATION_SA_NUM) {
                if (!(mask & DRV_SECURITY_LEARN)) {                    
                /* for dynamic sa num, should always enable learning*/
                    return SOC_E_PARAM;
                }
                /* SA violation will be drop or trap*/
                if (mask & DRV_SECURITY_EAP_MODE_EXTEND) {
                    temp = DRV_PORT_PROP_SA_OVERLIMIT_DROP;
                } else if (mask & DRV_SECURITY_EAP_MODE_SIMPLIFIED) {
                    temp = DRV_PORT_PROP_SA_OVERLIMIT_CPUCOPY;
                } else {
                    return SOC_E_PARAM;
                }
            } 

            if (mask & DRV_SECURITY_VIOLATION_SA_MOVEMENT) {
                /* SA violation will be drop or trap*/
                if (mask & DRV_SECURITY_EAP_MODE_EXTEND) {
                    temp = DRV_PORT_PROP_SA_MOVE_DROP;
                } else if (mask & DRV_SECURITY_EAP_MODE_SIMPLIFIED) {
                    temp = DRV_PORT_PROP_SA_MOVE_CPUCOPY;
                } else {
                    return SOC_E_PARAM;
                }
            }

            if (temp) {
                SOC_IF_ERROR_RETURN(DRV_PORT_SET
                    (unit, bmp, temp, 1));
            }        

            /* EAP Port Control Registers (Global) */
            SOC_IF_ERROR_RETURN(REG_READ_EAP_PORT_CTLr
                (unit, (void *)&reg_value64));
            SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_get
                (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

            COMPILER_64_TO_32_LO(temp_lo, temp64);
            COMPILER_64_TO_32_HI(temp_hi, temp64);

            PBMP_ITER(bmp, port) {
                if (port < SOC_ROBO_MAX_NUM_PORTS) {
                    temp = BCM_TBX_EAP_PORT_BLOCK_NONE;

                    /* port is not blocked */
                    temp_mask = BCM_TBX_EAP_PORT_BLOCK_MASK;
                    if (port > 15) {
                        temp_mask = ~(temp_mask << ((port - 16)*2));
                        temp_hi &= temp_mask;
                        temp_hi |= temp;
                    } else {
                        temp_mask = ~(temp_mask << (port*2));
                        temp_lo &= temp_mask;
                        temp_lo |= temp;
                    }
                }
            }
            COMPILER_64_SET(temp64, temp_hi, temp_lo);
            SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_set
                (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));
            SOC_IF_ERROR_RETURN(REG_WRITE_EAP_PORT_CTLr
                (unit, (void *)&reg_value64));

            /* Enable l2 learning */
            if (mask & DRV_SECURITY_LEARN) {
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_ENABLE_SET
                    (unit, bmp, DRV_PORT_HW_LEARN));
            } else {
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_ENABLE_SET
                    (unit, bmp, DRV_PORT_DISABLE_LEARN));
            }
            break;
        default:
            return SOC_E_PARAM;
    }
    
    return rv;
}

/*
 *  Function : drv_tbx_security_get
 *
 *  Purpose :
 *      Get the security mask to the selected ports.
 *
 *  Parameters :
 *      unit        :   unit id
 *      bmp   :   port bitmap.
 *      state   :   port state.
 *      mask     :   security mask.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Now, didn't output the mask value.
 *
 */
int 
drv_tbx_security_get(int unit, uint32 port, uint32 *state, uint32 *mask)
{
    int  rv = SOC_E_NONE;
    uint64  reg_value64, temp64;
    uint32  temp, temp_mask, temp_lo, temp_hi; 
            
    /* Get no security function (Basic mode)*/
    SOC_IF_ERROR_RETURN(DRV_PORT_GET
          (unit, port, DRV_PORT_PROP_SEC_MAC_MODE_NONE, &temp));
    if (temp) {
        *state = DRV_SECURITY_PORT_UNCONTROLLED;
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_EAP_PORT_CTLr
            (unit, (void *)&reg_value64));
        SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_get
            (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

        COMPILER_64_TO_32_LO(temp_lo, temp64);
        COMPILER_64_TO_32_HI(temp_hi, temp64);

        temp_mask = BCM_TBX_EAP_PORT_BLOCK_MASK;
        if (port > 15) {
            temp_hi = (temp_hi >> ((port - 16)*2));
            temp = temp_hi & temp_mask;
        } else {
            temp_lo = (temp_lo >> (port*2));
            temp = temp_lo & temp_mask; 
        }  

        if(temp) {
            /* Unauthenticate */
            *state = DRV_SECURITY_PORT_UNAUTHENTICATE;
        } else {
            /* Authenticated */
            *state = DRV_SECURITY_PORT_AUTHENTICATED;
        }
    }
                    
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_tbx_security_get: unit = %d, port= %d, state = %d, mask = 0x%x\n"),
              unit, port, *state, *mask));

    return rv;
}

int
drv_tbx_security_egress_set(int unit, soc_pbmp_t bmp, int enable)
{
    int  rv = SOC_E_NONE;
    uint64  reg_value64, temp64;
    uint32  temp, temp_mask, temp_lo, temp_hi;
    int  port;

    SOC_IF_ERROR_RETURN(REG_READ_EAP_PORT_CTLr
        (unit, (void *)&reg_value64));
    SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_get
        (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

    COMPILER_64_TO_32_LO(temp_lo, temp64);
    COMPILER_64_TO_32_HI(temp_hi, temp64);

    PBMP_ITER(bmp, port) {
        /* blocked in ingress or both ingress and egress side*/
        /* port is not blocked */
        if (port < SOC_ROBO_MAX_NUM_PORTS) {
            if (enable) {
            /* 
             * This function should only be called when UNAUTH mode. 
             * Port block modes are only block tx/rx and block tx.
             */
                temp = BCM_TBX_EAP_PORT_BLOCK_INGRESS;
            } else {
                temp = BCM_TBX_EAP_PORT_BLOCK_INGRESS_EGRESS;
            }

            temp_mask = BCM_TBX_EAP_PORT_BLOCK_MASK;
            if (port > 15) {
                temp = (temp << ((port - 16)*2));
                temp_mask = ~(temp_mask << ((port - 16)*2));
                temp_hi &= temp_mask;
                temp_hi |= temp;
            } else {
                temp = (temp << (port*2));
                temp_mask = ~(temp_mask << (port*2));
                temp_lo &= temp_mask;
                temp_lo |= temp;
            }
        }
    }

    COMPILER_64_SET(temp64, temp_hi, temp_lo);
    SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_set
        (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

    SOC_IF_ERROR_RETURN(REG_WRITE_EAP_PORT_CTLr
        (unit, (void *)&reg_value64));

    return rv;
}

int
drv_tbx_security_egress_get(int unit, int port, int *enable)
{
    int  rv = SOC_E_NONE;
    uint64  reg_value64, temp64;
    uint32  temp, temp_mask, temp_lo, temp_hi;

    SOC_IF_ERROR_RETURN(REG_READ_EAP_PORT_CTLr
        (unit, (void *)&reg_value64));
    SOC_IF_ERROR_RETURN(soc_EAP_PORT_CTLr_field_get
        (unit, (void *)&reg_value64, EAP_PORT_BLKf, (void *)&temp64));

    COMPILER_64_TO_32_LO(temp_lo, temp64);
    COMPILER_64_TO_32_HI(temp_hi, temp64);

    temp_mask = BCM_TBX_EAP_PORT_BLOCK_MASK;
    if (port > 15) {
        temp_hi = (temp_hi >> ((port - 16)*2));
        temp = temp_hi & temp_mask;
    } else {
        temp_lo = (temp_lo >> (port*2));
        temp = temp_lo & temp_mask; 
    } 

    if (temp == BCM_TBX_EAP_PORT_BLOCK_INGRESS_EGRESS) {
        *enable = 0;
    } else {
        *enable = 1;
    }

    return rv;
}

/*
 *  Function : drv_tbx_security_eap_control_set
 *
 *  Purpose :
 *      Set to enable or disable bypass for the EAP mac control types.
 *
 *  Parameters :
 *      unit        :   unit id
 *      type   :   (IN)eap mac control type.
 *      value     :   (IN)enable or disable.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int
drv_tbx_security_eap_control_set(int unit, uint32 type, uint32 value)
{
    uint32 reg_value32 = 0, temp = 0;

    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_BYPASS_CTLr
        (unit, &reg_value32));
    
    temp = (value) ? 1 : 0;
    switch (type) {
        case DRV_DEV_CTRL_EAP_BYPASS_USERADDR:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_L2_USER_ADDRf, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_DHCP:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_DHCPf, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_ARP:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_ARPf, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_MAC_22_2Ff, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_21:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_MAC_21f, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_20:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_MAC_20f, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_MAC_11_1Ff, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_10:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_MAC_10f, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_0X:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_set
                (unit, &reg_value32, EAP_BYPASS_MAC_0Xf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }
    
    SOC_IF_ERROR_RETURN(REG_WRITE_SECURITY_BYPASS_CTLr
        (unit, &reg_value32));

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_security_eap_control_set
 *
 *  Purpose :
 *      Get the status of EAP mac control types.
 *
 *  Parameters :
 *      unit        :   unit id
 *      type   :   (IN)eap mac control type.
 *      value     :   (OUT)enable or disable.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int
drv_tbx_security_eap_control_get(int unit, uint32 type, uint32 *value)
{
    uint32 reg_value32 = 0, temp = 0;

    /* pass special frame while block mode is set  */
    SOC_IF_ERROR_RETURN(REG_READ_SECURITY_BYPASS_CTLr
        (unit, &reg_value32));
    
    switch (type) {
        case DRV_DEV_CTRL_EAP_BYPASS_USERADDR:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_L2_USER_ADDRf, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_DHCP:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_DHCPf, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_ARP:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_ARPf, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_22_2F:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_MAC_22_2Ff, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_21:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_MAC_21f, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_20:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_MAC_20f, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_11_1F:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_MAC_11_1Ff, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_10:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_MAC_10f, &temp));
            break;
        case DRV_DEV_CTRL_EAP_BYPASS_MAC_0X:
            SOC_IF_ERROR_RETURN(soc_SECURITY_BYPASS_CTLr_field_get
                (unit, &reg_value32, EAP_BYPASS_MAC_0Xf, &temp));
            break;
        default:
            return SOC_E_PARAM;
    }
    
    *value = (temp) ? 1 : 0;

    return SOC_E_NONE;
}


