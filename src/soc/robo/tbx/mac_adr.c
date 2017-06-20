/*
 * $Id: mac_adr.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

/* this MACRO is for valid MAC check and the "mac" must be translated to 
 *  mac[6] already.
 */
#define _TB_MAC_ADDR_IS_NULL(mac)   \
    ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0) && \
    (mac[3] == 0) && (mac[4] == 0) && (mac[5] == 0)) 
/*
 *  Function : drv_tbx_mac_set
 *
 *  Purpose :
 *      Set the MAC address base on its type.
 *
 *  Parameters :
 *      unit    :   unit id
 *      bmp     :   port bitmap.
 *      mac_type:   mac address type.
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
drv_tbx_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
        uint8* mac, uint32 bpdu_idx)
{
    int     rv = SOC_E_NONE;
    int     reg_len = 0, ctrl_val = 0, l2_useradd_en = 0;
    uint32  ctrl_cnt = 0, ctrl_type = 0;
    uint32  reg_addr;
    uint32  reg_index = 0, fld_index = 0;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32;
    uint32  val32 = 0;
    uint64  reg_v64, mac_field;
    uint8   temp_mac[6];

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_mac_set: unit %d, pbmp = %x, type = %d"),
                 unit, SOC_PBMP_WORD_GET(pbmp, 0), mac_type));
    
    COMPILER_64_ZERO(reg_v64);
    COMPILER_64_ZERO(mac_field);
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
        	if (bpdu_idx == 0) {
                reg_index = INDEX(BPDU_MCADDRr);
                fld_index = INDEX(BPDU_MC_ADDRf);
        	} else {
                return SOC_E_UNAVAIL;
        	}        	
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
            return SOC_E_UNAVAIL;
            break;
        default :
            return SOC_E_PARAM;
    }
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);

    SAL_MAC_ADDR_TO_UINT64(mac, mac_field);
    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET(unit, reg_index, 
            (void *)&reg_v64, fld_index, (void *)&mac_field));
    SOC_IF_ERROR_RETURN(DRV_REG_WRITE(unit, reg_addr, 
            (void *)&reg_v64, reg_len));
    if((mac_type == DRV_MAC_MULTIPORT_0) || 
        (mac_type == DRV_MAC_MULTIPORT_1))  {
        
        /* check if processing l2_useraddr is invalid */
        l2_useradd_en = TRUE;
        if (_TB_MAC_ADDR_IS_NULL(mac)){
            if (mac_type == DRV_MAC_MULTIPORT_0){
        	    SOC_IF_ERROR_RETURN(REG_READ_GRPADDR2r(unit, 
        	            (void *)&reg_v64));
        	    soc_GRPADDR2r_field_get(unit, (void *)&reg_v64, 
        	            GRP_ADDRf, (void *)&mac_field);
            } else {
        	    SOC_IF_ERROR_RETURN(REG_READ_GRPADDR1r(unit, 
        	            (void *)&reg_v64));
        	    soc_GRPADDR1r_field_get(unit, (void *)&reg_v64, 
        	            GRP_ADDRf, (void *)&mac_field);
            }
    	    SAL_MAC_ADDR_FROM_UINT64(temp_mac, reg_v64);
    	    
    	    if (_TB_MAC_ADDR_IS_NULL(temp_mac)){
                l2_useradd_en = FALSE;
            }
        }
	    
        /* global multiport vector(L2 user address) function enable handler :
         *  1. This is for TB specific only. (Earily ROBO device has different
         *      design when L2 user address is disabled)
         *  2. Once MAC_Addr all zero and PBMP is zero too, this mac_set is 
         *      treat as l2 user address delete action. otherwise the process
         *      is treat as insert/modify action.
         *      >> insert or modify can be decided by checking current 
         *          configuration is valid or not.
         */
         
        /* enable MULTIPORT Address 1 and 2 register
         * and their associated MULTIPORT Vector1 and 2 register
         */
        ctrl_cnt = 1;
        ctrl_type = DRV_DEV_CTRL_L2_USERADDR;
        ctrl_val = l2_useradd_en;
        rv = DRV_DEV_CONTROL_SET(unit, &ctrl_cnt, &ctrl_type, &ctrl_val);
        if (rv != SOC_E_NONE){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s, %d, SOC Error!\n"), 
                      FUNCTION_NAME(), __LINE__));
            return rv;
        }
        
        reg_len = DRV_REG_LENGTH_GET(unit, bmp_index);
        reg_addr = DRV_REG_ADDR(unit, bmp_index, 0, 0);

        val32 = SOC_PBMP_WORD_GET(pbmp, 0);
        SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET(unit, bmp_index, &reg_v32, 
                bmp_fld, &val32));
        SOC_IF_ERROR_RETURN(DRV_REG_WRITE(unit, reg_addr, 
                &reg_v32, reg_len));
        
    } 
    
    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_mac_get
 *
 *  Purpose :
 *      Get the MAC address base on its type.
 *
 *  Parameters :
 *      unit    :   unit id
 *      port    :   port number.
 *      mac_type:   mac address type.
 *      mac     :   mac address.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :

 *
 */
int 
drv_tbx_mac_get(int unit, uint32 val, uint32 mac_type, 
        soc_pbmp_t *bmp, uint8* mac)
{
    uint32  reg_addr;
    uint64  reg_v64, mac_field;
    int     reg_len;
    uint32  bmp_index = 0, bmp_fld = 0, reg_v32, fld_v32;
    uint32      reg_index = 0, fld_index = 0;
    
    switch (mac_type) {
        case DRV_MAC_CUSTOM_BPDU:
        	if (val == 0) {
                reg_index = INDEX(BPDU_MCADDRr);
                fld_index = INDEX(BPDU_MC_ADDRf);
        	} else {
        	    return SOC_E_UNAVAIL;
        	}        	
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
            return SOC_E_UNAVAIL;
            break;
        default :
            return SOC_E_PARAM;
    }
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    if (mac_type == DRV_MAC_CUSTOM_BPDU) {
        reg_addr =DRV_REG_ADDR(unit, reg_index, 0, 0);
    } else {
        reg_addr = DRV_REG_ADDR(unit, reg_index, val, 0);
    }
    SOC_IF_ERROR_RETURN(DRV_REG_READ(unit, reg_addr, 
            (void *)&reg_v64, reg_len));
    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET(unit, reg_index, 
            (void *)&reg_v64, fld_index, (void *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(mac, mac_field);

    if((mac_type == DRV_MAC_MULTIPORT_0) || 
    	(mac_type == DRV_MAC_MULTIPORT_1)) {
        reg_len = DRV_REG_LENGTH_GET(unit, bmp_index);
        reg_addr = DRV_REG_ADDR(unit, bmp_index, 0, 0);
        SOC_IF_ERROR_RETURN(DRV_REG_READ(unit, reg_addr, 
                &reg_v32, reg_len));
        SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET(unit, bmp_index, &reg_v32, 
                    bmp_fld, &fld_v32));  
        SOC_PBMP_WORD_SET(*bmp, 0, fld_v32);
    }
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "drv_mac_get: unit %d, port = %d, type = %d,  \
                            mac =%02x-%02x-%02x-%02x-%02x-%02x\n"),
                 unit, val, mac_type, *mac, *(mac+1), *(mac+2),
                 *(mac+3), *(mac+4), *(mac+5)));
        
    return SOC_E_NONE;
}
