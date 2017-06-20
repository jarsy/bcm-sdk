/*
 * $Id: sec.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

#define BCM_HARRIER_EAP_PORT_BLOCK_NONE    0
#define BCM_HARRIER_EAP_PORT_BLOCK_INGRESS    1
#define BCM_HARRIER_EAP_PORT_BLOCK_INGRESS_EGRESS   2 

STATIC int
_drv_harrier_security_init(int unit)
{
    uint32 reg_value, temp;
    int rv = SOC_E_NONE;

    /* pass special fream while block mode is set  */
    if ((rv = REG_READ_EAP_GLO_CONr(unit, &reg_value)) < 0) {
        return rv;
    }
    
    /* bit11(EN_DHCP) in register profile is defined but not sync with 
     *    SW source definition. 
     */
    temp = 1;
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_DHCPf, &temp);

    /*  set bit 10 should program EAP destination IP 0/1 registers also 
     *  especially at DIP_MASK. So here we clear this bit always.
     */
    temp = 0;
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_DIP_0f, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_DIP_1f, &temp);
    
    temp = 1;
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_ARPf, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_22_2Ff, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_21f, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_20f, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_11_1Ff, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_10f, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_02_04_0Ff, &temp);
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_MAC_BPDUf, &temp);

    /* bit0(EN_EAP_PT_CHKf) should be reset 
       to force bcm5324 not to check EAPOL frame type. 
       - Current setting will force EAPOL frame in chip is tagged.
         (for 1Q + 1S enable and force reserve BPDU been tagged) 
    */

    temp = 0; /* force bit0 been reset */
    soc_EAP_GLO_CONr_field_set(unit, &reg_value,
        EN_EAP_PT_CHKf, &temp);
        
    if ((rv = REG_WRITE_EAP_GLO_CONr(unit, &reg_value)) < 0) {
        return rv;
    }

    return rv;
}


/*
 *  Function : drv_harrier_security_set
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
drv_harrier_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask)
{
    int             rv = SOC_E_NONE;
    uint32          reg_value, temp, flag = 0;
    uint32          port, sa_num = 0, trap = 0;
    l2_arl_sw_entry_t   arl_entry;
    uint32 sec_mask;
    
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_harrier_security_set: unit = %d, bmp= 0x%x, state = %d, mask = 0x%x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), state, mask));
    switch (state) {
        case DRV_SECURITY_PORT_UNCONTROLLED:
            /* 1. EAP Port Block (port is not blocked)
                2. set security mode = NONE (default), 
                3. enble l2 learning
            */                
            /* disable 802.1x */
            PBMP_ITER(bmp, port) {
                if ((rv = REG_READ_PORT_SEC_CONr(
                    unit, port, &reg_value)) < 0) {
                    return rv;
                }
                /* port is not blocked */
                temp = BCM_HARRIER_EAP_PORT_BLOCK_NONE;
                soc_PORT_SEC_CONr_field_set(unit, &reg_value, 
                    EAP_PORT_BLKf, &temp);
                if ((rv = REG_WRITE_PORT_SEC_CONr(
                    unit, port, &reg_value)) < 0) {
                    return rv;
                }
            }
            /* set no security function (Basic mode)*/
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                (unit, bmp, DRV_PORT_PROP_SEC_MAC_MODE_NONE, 0));
            
            /* enable l2 learning */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                (unit, bmp, DRV_PORT_HW_LEARN));
            
            break;

        case DRV_SECURITY_PORT_UNAUTHENTICATE:
            /* 1. disbale l2 learnning
                2. set security mode
                3. remove mac address associted with this port
                4. if (BLOCK_INOUT) -> port blocked in both ingress and egress side
            */
            SOC_IF_ERROR_RETURN(_drv_harrier_security_init(unit));

            /* disable l2 learning */
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                (unit, bmp, DRV_PORT_DISABLE_LEARN));
            
            /* set security mode */
            if (mask & DRV_SECURITY_VIOLATION_NONE) {
                temp = DRV_PORT_PROP_SEC_MAC_MODE_NONE;
            } else if (mask & DRV_SECURITY_VIOLATION_SA_NUM) {
                temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM;
            } else if (mask & DRV_SECURITY_VIOLATION_SA_MATCH) {
                temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH;
            } else {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                (unit, bmp, temp, 0));

            PBMP_ITER(bmp, port) {
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
                
                if ((rv = REG_READ_PORT_SEC_CONr(
                    unit, port, &reg_value)) < 0) {
                    return rv;
                }
                /* blocked in ingress or both ingress and egress side*/
                temp = (mask & DRV_SECURITY_BLOCK_INOUT) ? 
                            BCM_HARRIER_EAP_PORT_BLOCK_INGRESS_EGRESS : 
                            BCM_HARRIER_EAP_PORT_BLOCK_INGRESS;
                soc_PORT_SEC_CONr_field_set(unit, &reg_value, 
                    EAP_PORT_BLKf, &temp);
                if ((rv = REG_WRITE_PORT_SEC_CONr(
                    unit, port, &reg_value)) < 0) {
                    return rv;
                }
            }
            break;

        case DRV_SECURITY_PORT_AUTHENTICATED:
            /* 1. set security mode = NONE (default), 
                    it's depends on customer's definition.
                2. EAP Port Block (port is not blocked)
                3. enable l2 learning mode !
            */
            SOC_IF_ERROR_RETURN(_drv_harrier_security_init(unit));
                
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->dev_prop_get)
                    (unit, DRV_DEV_PROP_AUTH_SEC_MODE, &sec_mask));         

            if (!(mask & sec_mask)) {
                    return SOC_E_UNAVAIL;                
            }

            /* set security mode */

            if (mask & MAC_SEC_FLAGS) {
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
                temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_MATCH;               
            }

            if (mask & DRV_SECURITY_VIOLATION_SA_NUM) {
                if (!(mask & DRV_SECURITY_LEARN)) {                    
                /* for dynamic sa num, should always enable learning*/
                    return SOC_E_PARAM;
                }
                temp = DRV_PORT_PROP_SEC_MAC_MODE_DYNAMIC_SA_NUM;
                sa_num = 0x1fff;
            } 

             /* SA violation will be drop or trap*/
            if (mask & DRV_SECURITY_EAP_MODE_EXTEND){
                trap = DRV_PORT_PROP_SEC_MAC_MODE_EXTEND;
            } else if (mask & DRV_SECURITY_EAP_MODE_SIMPLIFIED){
                trap = DRV_PORT_PROP_SEC_MAC_MODE_SIMPLIFY;
            }


            if (trap) {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, bmp, trap, sa_num));  
            }
            if (temp) {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_set)
                    (unit, bmp, temp, sa_num));
            }        

            PBMP_ITER(bmp, port) {
                if ((rv = REG_READ_PORT_SEC_CONr(
                    unit, port, &reg_value)) < 0) {
                    return rv;
                }
                /* port is not blocked */
                temp =BCM_HARRIER_EAP_PORT_BLOCK_NONE;
                soc_PORT_SEC_CONr_field_set(unit, &reg_value, 
                    EAP_PORT_BLKf, &temp);
                if ((rv = REG_WRITE_PORT_SEC_CONr(
                    unit, port, &reg_value)) < 0) {
                    return rv;
                }
                
            }
                
            if (mask & DRV_SECURITY_IGNORE_LINK) {
                /* not implemented yet */
            } 
                
            if (mask & DRV_SECURITY_IGNORE_VIOLATION) {
                /* not implemented yet */
            }

            /* enable l2 learning */
            if (mask & DRV_SECURITY_LEARN) {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                    (unit, bmp, DRV_PORT_HW_LEARN));
            } else {
                SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->arl_learn_enable_set)
                    (unit, bmp, DRV_PORT_DISABLE_LEARN));
            }
            break;
        default:
            return SOC_E_PARAM;
    }
    
    return rv;
}

/*
 *  Function : drv_harrier_security_get
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
drv_harrier_security_get(int unit, uint32 port, uint32 *state, uint32 *mask)
{
    uint32          reg_value, temp;
    int             rv = SOC_E_NONE;
            
    /* set no security function (Basic mode)*/
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_get)
          (unit, port, DRV_PORT_PROP_SEC_MAC_MODE_NONE, &temp));
    if (temp) {
        *state = DRV_SECURITY_PORT_UNCONTROLLED;
    } else {
        if ((rv = REG_READ_PORT_SEC_CONr(
            unit, port, &reg_value)) < 0) {
            return rv;
        }
        soc_PORT_SEC_CONr_field_get(unit, &reg_value, 
            EAP_PORT_BLKf, &temp);

        if(temp) {
            /* Authenticated */
            *state = DRV_SECURITY_PORT_UNAUTHENTICATE;
        } else {
            /* Unauthenticate */
            *state = DRV_SECURITY_PORT_AUTHENTICATED;
        }
    }
                    
    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "drv_harrier_security_get: unit = %d, port= %d, state = %d, mask = 0x%x\n"),
              unit, port, *state, *mask));

    return rv;
}

int
drv_harrier_security_egress_set(int unit, soc_pbmp_t bmp, int enable)
{
    int             rv = SOC_E_NONE;
    uint32          reg_value, temp;
    int             port;

    PBMP_ITER(bmp, port) {
        if ((rv = REG_READ_PORT_SEC_CONr(
            unit, port, &reg_value)) < 0) {
            return rv;
        }
        /* port is not blocked */
        if (enable) {
            /* 
             * This function should only be called when UNAUTH mode. 
             * Port block modes are only block tx/rx and block tx.
             */
            temp = BCM_HARRIER_EAP_PORT_BLOCK_INGRESS;
        } else {
            temp = BCM_HARRIER_EAP_PORT_BLOCK_INGRESS_EGRESS;
        }
        soc_PORT_SEC_CONr_field_set(unit, &reg_value, 
            EAP_PORT_BLKf, &temp);
        if ((rv = REG_WRITE_PORT_SEC_CONr(
            unit, port, &reg_value)) < 0) {
            return rv;
        }
    }

    return rv;
}

int
drv_harrier_security_egress_get(int unit, int port, int *enable)
{
    int             rv = SOC_E_NONE;
    uint32          reg_value, temp = 0;

    if ((rv = REG_READ_PORT_SEC_CONr(
        unit, port, &reg_value)) < 0) {
        return rv;
    }

    soc_PORT_SEC_CONr_field_get(unit, &reg_value, 
        EAP_PORT_BLKf, &temp);

    if (temp == BCM_HARRIER_EAP_PORT_BLOCK_INGRESS_EGRESS) {
        *enable = 0;
    } else {
        *enable = 1;
    }

    return rv;
}


