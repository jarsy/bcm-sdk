/*
 * $Id: mac_adr.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>


int
_drv_gex_sec_mac_set(int unit, uint32 bmp, uint32 *mac, int mcast)
{
    int         rv = SOC_E_NONE;
    soc_pbmp_t  pbmp;
    uint32      port, temp, vid, prio;
    l2_arl_sw_entry_t    arl_entry;
    
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        sal_memset(&arl_entry, 0, sizeof (arl_entry));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_vlan_pvid_get)
            (unit, port, &vid, &prio));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            (uint32 *)&arl_entry, mac)); 

        temp = vid;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
            (uint32 *)&arl_entry, &temp)); 
        temp = prio;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
            (uint32 *)&arl_entry, &temp));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
            (uint32 *)&arl_entry, &temp));    
    
        if(mcast) {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&arl_entry, &temp));

        } else {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)&arl_entry, &temp));                
            temp = 1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)&arl_entry, &temp));

        }

        /* Insert this address into arl table. */
        rv = (DRV_SERVICES(unit)->mem_insert)
                (unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
                (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID));

    }

    return rv;
}

int
_drv_gex_sec_mac_remove(int unit, uint32 bmp, uint32 *mac, int mcast)
{
    int         rv = SOC_E_NONE;
    soc_pbmp_t  pbmp;
    uint32      port, temp, vid, prio;
    l2_arl_sw_entry_t    arl_entry;
    
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        sal_memset(&arl_entry, 0, sizeof (arl_entry));
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->port_vlan_pvid_get)
            (unit, port, &vid, &prio));

        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            (uint32 *)&arl_entry, mac)); 

        temp = vid;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
            (uint32 *)&arl_entry, &temp)); 
        temp = prio;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
            (uint32 *)&arl_entry, &temp));
        temp = 1;
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
            (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
            (uint32 *)&arl_entry, &temp));    
    
        if(mcast) {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, 
                (uint32 *)&arl_entry, &temp));

        } else {
            temp = port;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
                (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                 (uint32 *)&arl_entry, &temp));                
            temp = 1;
            SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->mem_field_set)
              (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                 (uint32 *)&arl_entry, &temp));

        }

        /* Insert this address into arl table. */
       rv = (DRV_SERVICES(unit)->mem_delete)
                (unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
                (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID
                | DRV_MEM_OP_DELETE_BY_STATIC));

    }
    return rv;
}

int
_drv_gex_sec_mac_clear(int unit, uint32 bmp)
{
    int         rv = SOC_E_NONE;
    uint32      reg_value, temp, port;
    int         count;
    soc_pbmp_t  pbmp;

    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, bmp);
    PBMP_ITER(pbmp, port) {
        
        reg_value = 0;
        if ((rv = REG_WRITE_FAST_AGE_PORTr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        } 
        temp = port;
        soc_FAST_AGE_PORTr_field_set(unit, &reg_value, 
            AGE_PORTf, &temp);
        
        if ((rv = REG_WRITE_FAST_AGE_PORTr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }
        
        /* start fast aging process */
        if ((rv = REG_READ_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }

        temp = 1;
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
            EN_FAST_AGE_STATICf, &temp);

        temp = 1;
        soc_FAST_AGE_CTRLr_field_set(unit, &reg_value, 
            FAST_AGE_STR_DONEf, &temp);

        if ((rv = REG_WRITE_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
            goto sec_mac_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_FAST_AGE_CTRLr(unit, &reg_value)) < 0) {
                goto sec_mac_exit;
            }
            soc_FAST_AGE_CTRLr_field_get(unit, &reg_value, 
            FAST_AGE_STR_DONEf, &temp);
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
 *  Function : drv_gex_mac_set
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
drv_gex_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, uint8* mac, uint32 bpdu_idx)
{
    int     rv = SOC_E_NONE, mcast = 0;
    uint32  reg_addr, port;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32  reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0;
    uint32 mport_ctrl_fld = 0;
    uint32  reg_v32;
    uint32  val32;
    int     customeap_en = 0;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32 specified_port_num;
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT  || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */

    COMPILER_64_ZERO(reg_v64);
    COMPILER_64_ZERO(mac_field);
    if (mac_type != DRV_MAC_SECURITY_CLEAR) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "drv_mac_set: unit %d, bmp = %x, type = %d,  \
                                mac =%02x-%02x-%02x-%02x-%02x-%02x\n"),
                     unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type, 
                     *mac, *(mac+1), *(mac+2),
                     *(mac+3), *(mac+4), *(mac+5)));
        SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
        if (mac[0] & 0x01) {
            mcast = 1;
        } else {
            mcast = 0;
        }

    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "drv_mac_set: unit %d, bmp = %x, type = %d"),
                     unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type));
    }       
    
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

    switch (mac_type) {
    case DRV_MAC_CUSTOM_BPDU:
        reg_index = INDEX(BPDU_MCADDRr);
        fld_index = INDEX(BPDU_MC_ADDRf);
        break;
    case DRV_MAC_MULTIPORT_0:
        reg_index = INDEX(MULTIPORT_ADDR0r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC0r);
        bmp_fld = INDEX(PORT_VCTRf);
        mport_ctrl_fld = INDEX(MPORT_CTRL0f);
        break;
    case DRV_MAC_MULTIPORT_1:
        reg_index = INDEX(MULTIPORT_ADDR1r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC1r);
        bmp_fld = INDEX(PORT_VCTRf);
        mport_ctrl_fld = INDEX(MPORT_CTRL1f);
        break;
    case DRV_MAC_MULTIPORT_2:
        reg_index = INDEX(MULTIPORT_ADDR2r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC2r);
        bmp_fld = INDEX(PORT_VCTRf);
        mport_ctrl_fld = INDEX(MPORT_CTRL2f);
        break;
    case DRV_MAC_MULTIPORT_3:
        reg_index = INDEX(MULTIPORT_ADDR3r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC3r);
        bmp_fld = INDEX(PORT_VCTRf);
        mport_ctrl_fld = INDEX(MPORT_CTRL3f);
        break;
    case DRV_MAC_MULTIPORT_4:
        reg_index = INDEX(MULTIPORT_ADDR4r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC4r);
        bmp_fld = INDEX(PORT_VCTRf);
        mport_ctrl_fld = INDEX(MPORT_CTRL4f);
        break;
    case DRV_MAC_MULTIPORT_5:
        reg_index = INDEX(MULTIPORT_ADDR5r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC5r);
        bmp_fld = INDEX(PORT_VCTRf);
        mport_ctrl_fld = INDEX(MPORT_CTRL5f);
        break;
    case DRV_MAC_CUSTOM_EAP:
        customeap_en = (COMPILER_64_IS_ZERO(mac_field)) ? 0 : 1;
        PBMP_ITER(pbmp, port) {
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
            if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                SOC_IS_NORTHSTARPLUS(unit)) 
                && (port == specified_port_num)) {
                SOC_IF_ERROR_RETURN(REG_READ_PORT_EAP_CON_P7r
                    (unit, (uint32 *)&reg_v64));
            } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */
            {
                SOC_IF_ERROR_RETURN(REG_READ_PORT_EAP_CONr(
                        unit, port, (uint32 *)&reg_v64 ));
            }
            /* set EAP_DA */
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set(
                    unit, (uint32 *)&reg_v64, EAP_UNI_DAf, 
                    (uint32 *)&mac_field));
                /* set enabling status :
                 *  - set EAP_DA to zero MAC will disable this feature. 
                 */
            SOC_IF_ERROR_RETURN(soc_PORT_EAP_CONr_field_set(
                    unit, (uint32 *)&reg_v64, EAP_EN_UNI_DAf, 
                    (uint32 *)&customeap_en));
            SOC_IF_ERROR_RETURN(REG_WRITE_PORT_EAP_CONr(
                    unit, port, (uint32 *)&reg_v64 ));
        }
        return SOC_E_NONE;
        break;
    case DRV_MAC_MIRROR_IN:
        reg_index = INDEX(IGMIRMACr);
        fld_index = INDEX(IN_MIR_MACf);
        break;
    case DRV_MAC_MIRROR_OUT:
        reg_index = INDEX(EGMIRMACr);
        fld_index = INDEX(OUT_MIR_MACf);
        break;
    case DRV_MAC_SECURITY_ADD:                                  
        rv = _drv_gex_sec_mac_set
            (unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field, mcast);
        return rv;
    case DRV_MAC_SECURITY_REMOVE:            
        rv = _drv_gex_sec_mac_remove
            (unit, SOC_PBMP_WORD_GET(pbmp, 0), (uint32 *)&mac_field, mcast);
        return rv;
    case DRV_MAC_SECURITY_CLEAR:            
        rv = _drv_gex_sec_mac_clear
            (unit, SOC_PBMP_WORD_GET(pbmp, 0));
        return rv;
    case DRV_MAC_IGMP_REPLACE:            
        rv = SOC_E_UNAVAIL;
        return rv;
    default :
        rv = SOC_E_PARAM;
        return rv;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, reg_index, (uint32 *)&reg_v64, 
            fld_index, (uint32 *)&mac_field));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
       return rv;
    }
    if((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1) ||
        (mac_type == DRV_MAC_MULTIPORT_2) ||
        (mac_type == DRV_MAC_MULTIPORT_3) ||
        (mac_type == DRV_MAC_MULTIPORT_4) ||
        (mac_type == DRV_MAC_MULTIPORT_5)) {
        /* Enable MPORT_CTRL(N) for Multiport Address(N) and Vector(N), N = 0 ~ 5
          * N = 0 : DRV_MAC_MULTIPORT_0,
          * N = 1 : DRV_MAC_MULTIPORT_1,
          * N = 2 : DRV_MAC_MULTIPORT_2,
          * N = 3 : DRV_MAC_MULTIPORT_3,
          * N = 4 : DRV_MAC_MULTIPORT_4,
          * N = 5 : DRV_MAC_MULTIPORT_5,
          */
        if ((rv = REG_READ_MULTI_PORT_CTLr(unit, &reg_v32)) < 0) {
            return rv;
        }

        /* Disable MPORT_CTRL(N) if Multiport Address and Vector are all zero */
        if ((ENET_CMP_MACADDR(mac, _soc_mac_all_zeroes) == 0) && 
            (SOC_PBMP_IS_NULL(pbmp))) {
            val32 = DRV_MULTIPORT_CTRL_DISABLE;
        } else {
            val32 = DRV_MULTIPORT_CTRL_MATCH_ADDR;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, INDEX(MULTI_PORT_CTLr), &reg_v32, 
                mport_ctrl_fld, &val32));

        if ((rv = REG_WRITE_MULTI_PORT_CTLr(unit, &reg_v32)) < 0) {
            return rv;
        }
        
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bmp_index);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bmp_index, 0, 0);

        val32 = SOC_PBMP_WORD_GET(pbmp, 0);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
                (unit, bmp_index, &reg_v32, 
                bmp_fld, &val32));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
                (unit, reg_addr, &reg_v32, reg_len)) < 0) {
            return rv;
        }
    } 

    return rv;
}

/*
 *  Function : drv_gex_mac_get
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
drv_gex_mac_get(int unit, uint32 port, uint32 mac_type, 
                                                soc_pbmp_t *bmp, uint8* mac)
{
    int     rv = SOC_E_NONE;
    uint32  reg_addr;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32  reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32, fld_v32;
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
    
    switch (mac_type) {
    case DRV_MAC_CUSTOM_BPDU:
        reg_index = INDEX(BPDU_MCADDRr);
        fld_index = INDEX(BPDU_MC_ADDRf);
        break;
    case DRV_MAC_MULTIPORT_0:
        reg_index = INDEX(MULTIPORT_ADDR0r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC0r);
        bmp_fld = INDEX(PORT_VCTRf);
        break;
    case DRV_MAC_MULTIPORT_1:
        reg_index = INDEX(MULTIPORT_ADDR1r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC1r);
        bmp_fld = INDEX(PORT_VCTRf);
        break;
    case DRV_MAC_MULTIPORT_2:
        reg_index = INDEX(MULTIPORT_ADDR2r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC2r);
        bmp_fld = INDEX(PORT_VCTRf);
        break;
    case DRV_MAC_MULTIPORT_3:
        reg_index = INDEX(MULTIPORT_ADDR3r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC3r);
        bmp_fld = INDEX(PORT_VCTRf);
        break;
    case DRV_MAC_MULTIPORT_4:
        reg_index = INDEX(MULTIPORT_ADDR4r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC4r);
        bmp_fld = INDEX(PORT_VCTRf);
        break;
    case DRV_MAC_MULTIPORT_5:
        reg_index = INDEX(MULTIPORT_ADDR5r);
        fld_index = INDEX(MPORT_ADDRf);
        bmp_index = INDEX(MPORTVEC5r);
        bmp_fld = INDEX(PORT_VCTRf);
        break;
    case DRV_MAC_CUSTOM_EAP:
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
        if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit)) 
            && (port == specified_port_num)) {
            reg_index = INDEX(PORT_EAP_CON_P7r);
        } else 
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */
        {
            reg_index = INDEX(PORT_EAP_CONr);
        }
        fld_index = INDEX(EAP_UNI_DAf);
        break;
    case DRV_MAC_MIRROR_IN:
        reg_index = INDEX(IGMIRMACr);
        fld_index = INDEX(IN_MIR_MACf);
        break;
    case DRV_MAC_MIRROR_OUT:
        reg_index = INDEX(EGMIRMACr);
        fld_index = INDEX(OUT_MIR_MACf);
        break;
    case DRV_MAC_IGMP_REPLACE:
    case DRV_MAC_SECURITY_ADD:
    case DRV_MAC_SECURITY_REMOVE:
    case DRV_MAC_SECURITY_CLEAR:
        rv = SOC_E_UNAVAIL;
        return rv;
    default :
        rv = SOC_E_PARAM;
        return rv;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    if (mac_type == DRV_MAC_CUSTOM_EAP) {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, port, 0);
    } else {
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    }
    if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, reg_index, (uint32 *)&reg_v64, 
            fld_index, (uint32 *)&mac_field));
  
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    if((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1) ||
        (mac_type == DRV_MAC_MULTIPORT_2) ||
        (mac_type == DRV_MAC_MULTIPORT_3) ||
        (mac_type == DRV_MAC_MULTIPORT_4) ||
        (mac_type == DRV_MAC_MULTIPORT_5)) {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bmp_index);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bmp_index, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
                (unit, reg_addr, &reg_v32, reg_len)) < 0) {
           return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
                (unit, bmp_index, &reg_v32, 
                bmp_fld, &fld_v32));  
        SOC_PBMP_WORD_SET(*bmp, 0, fld_v32);
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_mac_get: unit %d, port = %d, type = %d,  \
                            mac =%02x-%02x-%02x-%02x-%02x-%02x\n"),
                 unit, port, mac_type, *mac, *(mac+1), *(mac+2),
                 *(mac+3), *(mac+4), *(mac+5)));
    return rv;
}

