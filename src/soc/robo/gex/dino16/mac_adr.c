/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

static int
_drv_dino16_sec_mac_set(int unit, uint32 bmp, uint32 *mac, int mcast)
{
    soc_pbmp_t  pbmp;
    uint32      port, temp, vid, prio;
    l2_arl_sw_entry_t  arl_entry;
    
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        sal_memset(&arl_entry, 0, sizeof (arl_entry));
        SOC_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET
            (unit, port, &vid, &prio));

        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            (uint32 *)&arl_entry, mac)); 

        temp = vid;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
            (uint32 *)&arl_entry, &temp)); 

        temp = prio;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
            (uint32 *)&arl_entry, &temp));

        temp = 1;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
            (uint32 *)&arl_entry, &temp));    
    
        if (mcast) {
            temp = port;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&arl_entry, &temp));

        } else {
            temp = port;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)&arl_entry, &temp));                

            temp = 1;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&arl_entry, &temp));

        }

        /* Insert this address into arl table. */
        SOC_IF_ERROR_RETURN(DRV_MEM_INSERT
            (unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID)));
    }

    return SOC_E_NONE;
}

static int
_drv_dino16_sec_mac_remove(int unit, uint32 bmp, uint32 *mac, 
    int mcast)
{
    soc_pbmp_t  pbmp;
    uint32      port, temp, vid, prio;
    l2_arl_sw_entry_t  arl_entry;
   
    SOC_PBMP_CLEAR(pbmp); 
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        sal_memset(&arl_entry, 0, sizeof (arl_entry));
        SOC_IF_ERROR_RETURN(DRV_PORT_VLAN_PVID_GET
            (unit, port, &vid, &prio));

        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            (uint32 *)&arl_entry, mac)); 

        temp = vid;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
            (uint32 *)&arl_entry, &temp)); 

        temp = prio;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
            (uint32 *)&arl_entry, &temp));

        temp = 1;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
            (uint32 *)&arl_entry, &temp));    
    
        if (mcast) {
            temp = port;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&arl_entry, &temp));

        } else {
            temp = port;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)&arl_entry, &temp));                
            temp = 1;
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&arl_entry, &temp));
        }

        /* Insert this address into arl table. */
        SOC_IF_ERROR_RETURN(DRV_MEM_DELETE
            (unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID | 
             DRV_MEM_OP_DELETE_BY_STATIC)));
    }

    return SOC_E_NONE;
}

static int
_drv_dino16_sec_mac_clear(int unit, uint32 bmp)
{
    int     rv = SOC_E_NONE;
    uint32  reg_value, temp, port;
    int     count;
    soc_pbmp_t  pbmp;

    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        reg_value = 0;
        if ((rv = REG_WRITE_FAST_AGING_PORTr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        } 
        temp = port;
        soc_FAST_AGING_PORTr_field_set(unit, &reg_value, 
            AGE_SRC_PORTf, &temp);
        if ((rv = REG_WRITE_FAST_AGING_PORTr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }

        reg_value = 0;
        if ((rv = REG_READ_FAST_AGING_VIDr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }

        temp = 1;
        soc_FAST_AGING_VIDr_field_set(unit, &reg_value, 
            AGE_OUT_ALL_VIDSf, &temp);
        if ((rv = REG_WRITE_FAST_AGING_VIDr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }

        /* start fast aging process */
        if ((rv = REG_READ_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }

        temp = 1;    
        soc_FAST_AGE_CTLr_field_set(unit, &reg_value, 
            EN_FAST_AGE_STATICf, &temp);

        temp = 1;
        soc_FAST_AGE_CTLr_field_set(unit, &reg_value, 
            FAST_AGE_START_DONEf, &temp);

        if ((rv = REG_WRITE_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_FAST_AGE_CTLr(unit, &reg_value)) < 0) {
                goto sec_mac_exit;
            }
            soc_FAST_AGE_CTLr_field_get(unit, &reg_value, 
                FAST_AGE_START_DONEf, &temp);
            if (!temp) {
               break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto sec_mac_exit;
        }
    }

sec_mac_exit:
    return rv;
}

/*
 *  Function : drv_mac_set
 *
 *  Purpose :
 *      Set the MAC address base on its type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      val     :   port bitmap.
 *      mac_type   :   mac address type.
 *      mac     :   mac address.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */

int 
drv_dino16_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
    uint8* mac, uint32 bpdu_idx)
{
    int     rv = SOC_E_NONE, mcast = 0;
    uint32  reg_addr;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32  reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0;
    uint32  reg_v32;
    uint32  val32;

    COMPILER_64_ZERO(reg_v64);
    COMPILER_64_ZERO(mac_field);
    if (mac_type != DRV_MAC_SECURITY_CLEAR) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "drv_dino16_mac_set: unit %d, bmp = 0x%x, type = %d, \
                                mac = %02x-%02x-%02x-%02x-%02x-%02x\n"),
                     unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type, 
                     *mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5)));

        SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
        if (mac[0] & 0x01) {
            mcast = 1;
        } else {
            mcast = 0;
        }
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "drv_dino16_mac_set: unit %d, bmp = 0x%x, type = %d"),
                     unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type));
    }       

    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
            reg_index = INDEX(BPDU_MCADDRr);
            fld_index = INDEX(BPDU_MC_ADDRf);
            break;
        case DRV_MAC_MULTIPORT_0:
            reg_index = INDEX(GRPADDR1r);
            fld_index = INDEX(GRP_ADDRf);
            bmp_index = INDEX(PORTVEC1r);
            bmp_fld = INDEX(PORT_VCTRf);
            break;
        case DRV_MAC_MULTIPORT_1:
            reg_index = INDEX(GRPADDR2r);
            fld_index = INDEX(GRP_ADDRf);
            bmp_index = INDEX(PORTVEC2r);
            bmp_fld = INDEX(PORT_VCTRf);
            break;
        case DRV_MAC_SECURITY_ADD:                                  
            rv = _drv_dino16_sec_mac_set
                (unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field, mcast);
            return rv;
        case DRV_MAC_SECURITY_REMOVE:            
            rv = _drv_dino16_sec_mac_remove
                (unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field, mcast);
            return rv;
        case DRV_MAC_SECURITY_CLEAR:            
            rv = _drv_dino16_sec_mac_clear
                (unit, SOC_PBMP_WORD_GET(pbmp, 0));
            return rv;
        case DRV_MAC_CUSTOM_EAP:          
        case DRV_MAC_MIRROR_IN:       
        case DRV_MAC_MIRROR_OUT:           
        case DRV_MAC_IGMP_REPLACE:            
            rv = SOC_E_UNAVAIL;
            return rv;
        default :
            rv = SOC_E_PARAM;
            return rv;
    }

    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET
        (unit, reg_index, (uint32 *)&reg_v64, fld_index, (uint32 *)&mac_field));
    if ((rv = DRV_REG_WRITE(unit, reg_addr, 
            (uint32 *)&reg_v64, reg_len)) < 0) {
       return rv;
    }

    if ((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1)) {
        /* enable MULTIPORT Address 1 and 2 register
          * and their associated MULTIPORT Vector1 and 2 register
          */            
        if ((rv = REG_READ_GARLCFGr(unit, (uint32 *)&reg_v64)) < 0) {
            return rv;
        }
        reg_v32 = 1;
        soc_GARLCFGr_field_set(unit, (uint32 *)&reg_v64, 
            MPORT_ADDR_ENf, (uint32 *)&reg_v32);
        if ((rv = REG_WRITE_GARLCFGr(unit, (uint32 *)&reg_v64)) < 0) {
            return rv;
        }

        reg_len = DRV_REG_LENGTH_GET(unit, bmp_index);
        reg_addr = DRV_REG_ADDR(unit, bmp_index, 0, 0);

        val32 = SOC_PBMP_WORD_GET(pbmp, 0);
        SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET
            (unit, bmp_index, &reg_v32, bmp_fld, &val32));
        if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_v32, reg_len)) < 0) {
            return rv;
        }
    } 

    return rv;
}

/*
 *  Function : drv_mac_get
 *
 *  Purpose :
 *      Get the MAC address base on its type.
 *
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port number.
 *      mac_type   :   mac address type.
 *      mac     :   mac address.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      It didn't support to get the Secure MAC address for each port. 
 *      Using the mem_read to achieve this.
 *
 */
int 
drv_dino16_mac_get(int unit, uint32 port, uint32 mac_type, 
    soc_pbmp_t *bmp, uint8* mac)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32  reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32, fld_v32;
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
            reg_index = INDEX(BPDU_MCADDRr);
            fld_index = INDEX(BPDU_MC_ADDRf);
            break;
        case DRV_MAC_MULTIPORT_0:
            reg_index = INDEX(GRPADDR1r);
            fld_index = INDEX(GRP_ADDRf);
            bmp_index = INDEX(PORTVEC1r);
            bmp_fld = INDEX(PORT_VCTRf);
            break;
        case DRV_MAC_MULTIPORT_1:
            reg_index = INDEX(GRPADDR2r);
            fld_index = INDEX(GRP_ADDRf);
            bmp_index = INDEX(PORTVEC2r);
            bmp_fld = INDEX(PORT_VCTRf);
            break;
        case DRV_MAC_CUSTOM_EAP:        
        case DRV_MAC_MIRROR_IN:
        case DRV_MAC_MIRROR_OUT:          
        case DRV_MAC_IGMP_REPLACE:
        case DRV_MAC_SECURITY_ADD:
        case DRV_MAC_SECURITY_REMOVE:
        case DRV_MAC_SECURITY_CLEAR:
            rv = SOC_E_UNAVAIL;
            break;
        default :
            rv = SOC_E_PARAM;
            return rv;
    }

    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
    if ((rv = DRV_REG_READ(unit, reg_addr, 
            (uint32 *)&reg_v64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET
        (unit, reg_index, (uint32 *)&reg_v64, fld_index, (uint32 *)&mac_field));
  
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    if ((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1)) {
        reg_len = DRV_REG_LENGTH_GET(unit, bmp_index);
        reg_addr = DRV_REG_ADDR(unit, bmp_index, 0, 0);
        if ((rv = DRV_REG_READ
                (unit, reg_addr, &reg_v32, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET
            (unit, bmp_index, &reg_v32, bmp_fld, &fld_v32));  
        SOC_PBMP_WORD_SET(*bmp, 0, fld_v32);
    }

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_dino16_mac_get: unit %d, port = %d, type = %d, \
                            mac =%02x-%02x-%02x-%02x-%02x-%02x\n"),
                 unit, port, mac_type, 
                 *mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5)));

    return rv;
}
