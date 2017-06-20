/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>


static  uint32 port_sec_state[SOC_MAX_NUM_PORTS];

/*
 *  Function : drv_dino16_security_set
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
drv_dino16_security_set(int unit, soc_pbmp_t bmp, 
    uint32 state, uint32 mask)
{
    int     rv = SOC_E_NONE;
    uint32  temp = 0, flag = 0;
    uint32  reg_value;
    uint32  port = 0;
    l2_arl_sw_entry_t  arl_entry;
    soc_pbmp_t  temp_pbmp;
    uint32  sec_mask;

    LOG_INFO(BSL_LS_SOC_PORT, \
             (BSL_META_U(unit, \
                         "drv_dino16_security_set: \
                         unit = %d, bmp= 0x%x, state = %d, mask = 0x%x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), state, mask));

    switch (state) {
        case DRV_SECURITY_PORT_UNCONTROLLED:
            /*  1. disable 802.1x
                2. enable Tx/Rx
            */
            /* disable 802.1x */
            if ((rv = REG_READ_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_AUTH_8021X_CTL2r_field_get(unit, &reg_value, 
                PORT_BLOCKf, &temp);

            temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
            soc_AUTH_8021X_CTL2r_field_set(unit, &reg_value, 
                PORT_BLOCKf, &temp);
                
            if ((rv = REG_WRITE_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                return rv;
            }

            PBMP_ITER(bmp, port) {
                /* enable Tx/Rx */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
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

            /* enable 802.1x */
            if ((rv = REG_READ_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_AUTH_8021X_CTL2r_field_get(unit, &reg_value, 
                PORT_BLOCKf, &temp);

            temp |= SOC_PBMP_WORD_GET(bmp, 0);
            soc_AUTH_8021X_CTL2r_field_set(unit, &reg_value, 
                PORT_BLOCKf, &temp);
                
            if ((rv = REG_WRITE_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                return rv;
            }

            PBMP_ITER(bmp, port) {                
                /* remove the l2 mac address associated with this port */
                flag |= DRV_MEM_OP_DELETE_BY_PORT | 
                    DRV_MEM_OP_DELETE_BY_STATIC;
                sal_memset(&arl_entry, 0, sizeof(l2_arl_sw_entry_t));
                temp = port;
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)&arl_entry, &temp)) < 0) {
                    return rv;
                }
                if ((rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, 
                        (uint32 *)&arl_entry, flag)) < 0) {
                    return rv;
                }
                
                /* disable TX if block in-out or enable Tx anyway */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                if ((rv = DRV_PORT_SET(unit, temp_pbmp, 
                        DRV_PORT_PROP_ENABLE_TX, 
                        ((mask & DRV_SECURITY_BLOCK_INOUT) ? 
                                FALSE : TRUE))) < 0) {
                    return rv;
                }
                port_sec_state[port] = DRV_SECURITY_PORT_UNAUTHENTICATE;
            }

            break;
            
        case DRV_SECURITY_PORT_AUTHENTICATED:
            /* 1. enable 802.1x (but reset BLK_mode)
               2. set security mode = NONE (default), 
                    it depends on the definiton of customer.
               3. enable Tx when port been AUTHENTICATED and enable 
                  learning/not-learning mode !
                  - not learning mode need to clear all existed L2 entries by 
                    this port. 
               4. Limitation on bcm5396 :
                  a. the security mode allowed below two security mode only :
                    - DRV_SECURITY_VIOLATION_NONE (learning mode)
                    - DRV_SECURITY_VIOLATION_STATIC_ACCEPT(not learning mode,
                       CPU have to help to add the valid L2 entry on this port)
                  b. other the SEC_MODE will got the return value as "UNAVAIL"
               5. For bcm5396, the 802.1x mode will be always been set as zero.
                  (Mode = 802.1x mode[page0x00,addr0xx77,bit0])
                  (PBlock = PortBlock[page0x00,addr0x78,bit0:16])
                         Uncontrol    UnAuth         Auth-learn  Auth-notlearn
                         ======================================================
                  Mode      [0/1]  [0/1]+L2_Clear        [0/1]     0+L2_lear
                  PBlock      0         1                  0          1
                         ======================================================
            */
            SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
                (unit, DRV_DEV_PROP_AUTH_SEC_MODE, &sec_mask));         

            if (!(mask & sec_mask)) {
                return SOC_E_UNAVAIL;                
            }            

            /* check valid configuration on learning mode */
            if (mask & DRV_SECURITY_LEARN) {
                if (!(mask & DRV_SECURITY_VIOLATION_NONE)) {
                    return SOC_E_UNAVAIL;
                }
            } else {
                if (!(mask & DRV_SECURITY_VIOLATION_STATIC_ACCEPT)) {
                    return SOC_E_UNAVAIL;
                }
            }
            
            if ((mask & DRV_SECURITY_IGNORE_LINK) ||
                (mask & DRV_SECURITY_IGNORE_VIOLATION)) {
                /* not implemented yet */
                return SOC_E_UNAVAIL;
            } 
            
            /* enable 802.1x */
            if ((rv = REG_READ_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_AUTH_8021X_CTL2r_field_get(unit, &reg_value, 
                PORT_BLOCKf, &temp);

            /* set security learning mode :
             *  - free port block
             */
            if (mask & DRV_SECURITY_LEARN) {
                temp &= ~(SOC_PBMP_WORD_GET(bmp, 0));
                soc_AUTH_8021X_CTL2r_field_set(unit, &reg_value, 
                    PORT_BLOCKf, &temp);
                
                if ((rv = REG_WRITE_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                    return rv;
                }

            /* set security not learning mode 
             *  1. set 802.1x mode(0: drop if SA miss and not 802.1x special 
             *     frame) 
             *  2. set port block
             *  3. remove learned l2 entries by port.
             */
            } else {         
                temp |= SOC_PBMP_WORD_GET(bmp, 0);
                soc_AUTH_8021X_CTL2r_field_set(unit, &reg_value, 
                    PORT_BLOCKf, &temp);
                
                if ((rv = REG_WRITE_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
                    return rv;
                }

                if ((rv = REG_READ_AUTH_8021X_CTL1r(unit, &reg_value)) < 0) {
                    return rv;
                }
                temp = 0;                                                         
                soc_AUTH_8021X_CTL1r_field_get(unit, &reg_value, 
                    MODE_8021Xf, &temp);

                if ((rv = REG_WRITE_AUTH_8021X_CTL1r(unit, &reg_value)) < 0) {
                    return rv;
                }

                /* remove the l2 mac address associated with port */
                PBMP_ITER(bmp, port) {                
                    flag |= DRV_MEM_OP_DELETE_BY_PORT | 
                        DRV_MEM_OP_DELETE_BY_STATIC;
                    sal_memset(&arl_entry, 0, sizeof(l2_arl_sw_entry_t));
                    temp = port;
                    if ((rv = DRV_MEM_FIELD_SET(unit, 
                            DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                            (uint32 *)&arl_entry, &temp)) < 0) {
                        return rv;
                    }
                    if ((rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, 
                            (uint32 *)&arl_entry, flag)) < 0) {
                        return rv;
                    }
                }
            }

            PBMP_ITER(bmp, port) {
                /* enable Tx in learning mode */
                SOC_PBMP_CLEAR(temp_pbmp);
                SOC_PBMP_PORT_ADD(temp_pbmp, port);
                if ((rv = DRV_PORT_SET(unit, temp_pbmp, 
                        DRV_PORT_PROP_ENABLE_TX, TRUE)) < 0) {
                    return rv;
                }
                port_sec_state[port] = DRV_SECURITY_PORT_AUTHENTICATED;
            }         
            break;
        default:
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_dino16_security_get
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
 *
 */
int 
drv_dino16_security_get(int unit, uint32 port, 
    uint32 *state, uint32 *mask)
{
    int      rv = SOC_E_NONE;
    uint32   temp = 0;
    uint32   reg_value;
    
    *state = port_sec_state[port];
   
    if (*state != DRV_SECURITY_PORT_AUTHENTICATED) {
        *mask = 0x0;
    } else {
        if ((rv = REG_READ_AUTH_8021X_CTL2r(unit, &reg_value)) < 0) {
            return rv;
        }
        soc_AUTH_8021X_CTL2r_field_get(unit, &reg_value, 
            PORT_BLOCKf, &temp);
        
        if (temp & (0x1 << port)) {
            *mask = DRV_SECURITY_VIOLATION_STATIC_ACCEPT;
        } else {
            *mask = DRV_SECURITY_LEARN | DRV_SECURITY_VIOLATION_NONE;
        }
    }
    
    return rv;
}

int
drv_dino16_security_egress_set(int unit, soc_pbmp_t bmp, int enable)
{
    SOC_IF_ERROR_RETURN(DRV_PORT_SET
        (unit, bmp, DRV_PORT_PROP_ENABLE_TX, enable));

    return SOC_E_NONE;
}

int
drv_dino16_security_egress_get(int unit, int port, int *enable)
{
    SOC_IF_ERROR_RETURN(DRV_PORT_GET
        (unit, port, DRV_PORT_PROP_ENABLE_TX, (uint32 *)enable));

    return SOC_E_NONE;
}
