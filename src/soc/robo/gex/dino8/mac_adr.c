/*
 * $Id: mac_adr.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_dino8_mac_set
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
 */
int 
drv_dino8_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
    uint8* mac, uint32 bpdu_idx)
{
    int     rv = SOC_E_NONE;
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
                                "drv_dino8_mac_set: unit %d, bmp = %x, type = %d,  \
                                mac =%02x-%02x-%02x-%02x-%02x-%02x\n"),
                     unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type, 
                     *mac, *(mac+1), *(mac+2), *(mac+3), *(mac+4), *(mac+5)));
        SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "drv_dino8_mac_set: unit %d, bmp = %x, type = %d"),
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
        case DRV_MAC_SECURITY_REMOVE:            
        case DRV_MAC_SECURITY_CLEAR:            
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
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
        (unit, reg_index, (uint32 *)&reg_v64, fld_index, (uint32 *)&mac_field));
    if ((rv = (DRV_SERVICES(unit)->reg_write)
        (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
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

        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bmp_index);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bmp_index, 0, 0);

        val32 = SOC_PBMP_WORD_GET(pbmp, 0);
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_set)
            (unit, bmp_index, &reg_v32, bmp_fld, &val32));
        if ((rv = (DRV_SERVICES(unit)->reg_write)
            (unit, reg_addr, &reg_v32, reg_len)) < 0) {
            return rv;
        }
   } 

    return rv;
}

/*
 *  Function : drv_dino8_mac_get
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
drv_dino8_mac_get(int unit, uint32 port, uint32 mac_type, 
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
            return rv;
        default :
            rv = SOC_E_PARAM;
            return rv;
    }
    reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, reg_index);
    reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, reg_index, 0, 0);
    if ((rv = (DRV_SERVICES(unit)->reg_read)
        (unit, reg_addr, (uint32 *)&reg_v64, reg_len)) < 0) {
        return rv;
    }
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
        (unit, reg_index, (uint32 *)&reg_v64, fld_index, (uint32 *)&mac_field));
  
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    if ((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1)) {
        reg_len = (DRV_SERVICES(unit)->reg_length_get)(unit, bmp_index);
        reg_addr = (DRV_SERVICES(unit)->reg_addr)(unit, bmp_index, 0, 0);
        if ((rv = (DRV_SERVICES(unit)->reg_read)
            (unit, reg_addr, &reg_v32, reg_len)) < 0) {
            return rv;
        }
        SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->reg_field_get)
            (unit, bmp_index, &reg_v32, bmp_fld, &fld_v32));  
        SOC_PBMP_WORD_SET(*bmp, 0, fld_v32);
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_dino8_mac_get: unit %d, port = %d, type = %d,  \
                            mac =%02x-%02x-%02x-%02x-%02x-%02x\n"),
                 unit, port, mac_type, *mac, *(mac+1), *(mac+2),
                 *(mac+3), *(mac+4), *(mac+5)));

    return rv;
}

