/*
 * $Id: sec.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

static  uint32 port_sec_state[SOC_MAX_NUM_PORTS];

#define BCM_GEX_EAP_PORT_BLOCK_NONE  0 
#define BCM_GEX_EAP_PORT_BLOCK_INGRESS  0x1
#define BCM_GEX_EAP_PORT_BLOCK_INGRESS_EGRESS  0x3

STATIC int
_drv_gex_security_init(int unit)
{
    uint32 reg_value, temp;
    int rv = SOC_E_NONE;


    /* pass special fream while block mode is set  */
    if ((rv = REG_READ_EAP_GLO_CONr(unit, &reg_value)) < 0) {
        return rv;
    }

    temp = 0;
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_RARPf, &temp);

    temp = 1;
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_ARPf, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_DHCPf, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_RMCf, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_BPDUf, &temp);
    
    /*  set EN_2_DIPf should program EAP destination IP 0/1 registers also 
     *  especially at DIP_MASK. So here we clear this bit always.
     */
    temp = 0;
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_2_DIPf, &temp);

    if ((rv = REG_WRITE_EAP_GLO_CONr(unit, &reg_value)) < 0) {
        return rv;
    }
    return rv;
}

/*
 *  Function : drv_gex_security_set
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
drv_gex_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask)
{
    int             rv = SOC_E_NONE;
    uint32          temp, flag = 0;
    uint64          reg_value64;
    uint32          port;
    l2_arl_sw_entry_t   arl_entry;
    soc_pbmp_t      temp_pbmp;
    uint32 sec_mask;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT */

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_security_set: unit = %d, bmp= 0x%x, state = %d, mask = 0x%x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), state, mask));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
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
    switch (state) {
        case DRV_SECURITY_PORT_UNCONTROLLED:
            /*  1. disable 802.1x
                2. enable Tx/Rx
                 3. enble l2 learning
            */
            /* disable 802.1x */
            PBMP_ITER(bmp, port) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
                if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)|| 
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
                temp = 0;
                soc_PORT_EAP_CONr_field_set(unit, (uint32 *)&reg_value64,
                    EAP_BLK_MODEf, &temp);
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
		        /* enable l2 learning */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_ENABLE_SET
                    (unit, temp_pbmp, DRV_PORT_HW_LEARN));
                
                /* set no security function */
                SOC_IF_ERROR_RETURN(DRV_PORT_SET
                    (unit, temp_pbmp, DRV_PORT_PROP_SEC_MAC_MODE_NONE, 0));
                /* enable Tx/Rx */
                if ((rv = DRV_PORT_SET(unit, temp_pbmp, 
					          DRV_PORT_PROP_ENABLE_TXRX, TRUE)) < 0) {
                    return rv;
                }
                port_sec_state[port] = DRV_SECURITY_PORT_UNCONTROLLED;
            }
            break;
        case DRV_SECURITY_PORT_UNAUTHENTICATE:
            /*  1. enable 802.1x (BLK_mode == 1)
                2. disbale L2 learnning (set security mode)
                3. remove mac address associted with this port
                4. if (BLOCK_INOUT) -> disable Tx
            */			
            SOC_IF_ERROR_RETURN(
                _drv_gex_security_init(unit));
            PBMP_ITER(bmp, port) {
                /* enable 802.1x */
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
                /* allow only EAPOL packets */

                if (mask & DRV_SECURITY_BLOCK_INOUT) {
                    temp = BCM_GEX_EAP_PORT_BLOCK_INGRESS_EGRESS;
                } else if (mask & DRV_SECURITY_BLOCK_IN){
                    temp = BCM_GEX_EAP_PORT_BLOCK_INGRESS;
                }
                soc_PORT_EAP_CONr_field_set(unit, (uint32 *)&reg_value64,
                    EAP_BLK_MODEf, &temp);
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

                /* disable l2 learning */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                SOC_IF_ERROR_RETURN(DRV_ARL_LEARN_ENABLE_SET
                    (unit, temp_pbmp, DRV_PORT_DISABLE_LEARN));	
                    
                /* remove the l2 mac address associated with this port */
                flag |= DRV_MEM_OP_DELETE_BY_PORT | 
                    DRV_MEM_OP_DELETE_BY_STATIC;
                sal_memset(&arl_entry, 0, sizeof(l2_arl_sw_entry_t));
                temp = port;
                if ((rv = (DRV_SERVICES(unit)->mem_field_set)
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&arl_entry, &temp)) < 0) {
                    return rv;
                }
                if ((rv = (DRV_SERVICES(unit)->mem_delete)
                    (unit, DRV_MEM_ARL, (uint32 *)&arl_entry, flag)) < 0) {
                    return rv;
                }
                              
                port_sec_state[port] = DRV_SECURITY_PORT_UNCONTROLLED;
           }
            break;
        case DRV_SECURITY_PORT_AUTHENTICATED:
            /* 1. enable 802.1x (but reset BLK_mode)
               2. set security mode = NONE (default), 
                    it's depends on customer's definition.
               3. enable Tx when port been AUTHENTICATED and enable 
                  learning mode !
            */
            SOC_IF_ERROR_RETURN(
                _drv_gex_security_init(unit));
            PBMP_ITER(bmp, port) {
                /* enable 802.1x */
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
                temp = 0;
                /* enable TX/Rx */
                soc_PORT_EAP_CONr_field_set(unit, (uint32 *)&reg_value64,
                    EAP_BLK_MODEf, &temp);
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

                SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                    (unit, DRV_DEV_PROP_AUTH_SEC_MODE, &sec_mask));         

                if (!(mask & sec_mask)) {
                    return SOC_E_UNAVAIL;                
                }
                /* set security mode */
                temp = DRV_PORT_PROP_SEC_MAC_MODE_NONE; ; 
                if (mask & DRV_SECURITY_EAP_MODE_EXTEND) {
                    /* unknow SA drop */
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_EXTEND;
                    if (mask & DRV_SECURITY_LEARN) {
                        return SOC_E_PARAM;
                    }
                } else if (mask & DRV_SECURITY_EAP_MODE_SIMPLIFIED) {
                    /* unknow SA trap */
                    temp = DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY;
                    if (mask & DRV_SECURITY_LEARN) {
                        return SOC_E_PARAM;
                    }
                }

                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                SOC_IF_ERROR_RETURN(DRV_PORT_SET
                    (unit, temp_pbmp, temp, 0));

                
                if (mask & DRV_SECURITY_IGNORE_LINK) {
                    /* not implemented yet */
                } 
                
                if (mask & DRV_SECURITY_IGNORE_VIOLATION) {
                    /* not implemented yet */
                }
                port_sec_state[port] = DRV_SECURITY_PORT_UNAUTHENTICATE;
            }

            /* enable l2 learning */
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
 *  Function : drv_gex_security_get
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
drv_gex_security_get(int unit, uint32 port, uint32 *state, uint32 *mask)
{
    int             rv = SOC_E_NONE;
    uint32          temp = 0;
    uint64          reg_value64;
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
    soc_PORT_EAP_CONr_field_get(unit, (uint32 *)&reg_value64,
        EAP_MODEf, &temp);
    if (temp) {
        soc_PORT_EAP_CONr_field_get(unit, (uint32 *)&reg_value64,
            EAP_BLK_MODEf, &temp);
        if (temp) {
        	*state = DRV_SECURITY_PORT_UNAUTHENTICATE;
        } else {
            *state = DRV_SECURITY_PORT_AUTHENTICATED;
        }
    } else {
        *state = DRV_SECURITY_PORT_UNCONTROLLED;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_gex_security_get: unit = %d, port= %d, state = %d, mask = 0x%x\n"),
              unit, port, *state, *mask));

    return rv;
}

int
drv_gex_security_egress_set(int unit, soc_pbmp_t bmp, int enable)
{
    int             rv = SOC_E_NONE;
    uint32          temp;
    uint64          reg_value64;
    int             port;
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

    PBMP_ITER(bmp, port) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT)|| \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit)) 
            && (port == specified_port_num)) {
            SOC_IF_ERROR_RETURN(
                REG_READ_PORT_EAP_CON_P7r(unit, (uint32 *)&reg_value64));
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
        {
            SOC_IF_ERROR_RETURN(
                REG_READ_PORT_EAP_CONr(unit, port, (uint32 *)&reg_value64));
        }

        /* port is not blocked */
        if (enable) {
            /* 
                       * This function should only be called when UNAUTH mode. 
                       * Port block modes are only block tx/rx and block tx.
                       */
            temp = BCM_GEX_EAP_PORT_BLOCK_INGRESS;
        } else {
            temp = BCM_GEX_EAP_PORT_BLOCK_NONE;
        }

        SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set
            (unit, (uint32 *)&reg_value64, EAP_BLK_MODEf, &temp));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit)) 
            && (port == specified_port_num)) {
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PORT_EAP_CON_P7r(unit, (uint32 *)&reg_value64));
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
        {
            SOC_IF_ERROR_RETURN(
                REG_WRITE_PORT_EAP_CONr(unit, port, (uint32 *)&reg_value64));
        }
    }

    return rv;
}

int
drv_gex_security_egress_get(int unit, int port, int *enable)
{
    int             rv = SOC_E_NONE;
    uint32          temp;
    uint64          reg_value64;
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

    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_EAP_CON_P7r(unit, (uint32 *)&reg_value64));
    } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
    {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_EAP_CONr(unit, port, (uint32 *)&reg_value64));
    }

    SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_get
        (unit, (uint32 *)&reg_value64, EAP_BLK_MODEf, &temp));

    if (temp == BCM_GEX_EAP_PORT_BLOCK_INGRESS_EGRESS) {
        *enable = FALSE;
    } else {
        *enable = TRUE;
    }

    return rv;
}

