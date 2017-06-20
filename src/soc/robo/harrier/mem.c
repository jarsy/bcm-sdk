/*
 * $Id: mem.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>

#include <soc/l2x.h>
#include <soc/arl.h>
#include <soc/error.h>

#define FIX_MEM_ORDER_E(v,m) (((m)->flags & SOC_MEM_FLAG_BE) ? \
    BYTES2WORDS((m)->bytes)-1-(v) : (v))
    
#define HARRIER_TXDSC_MEMORY 1 
#define HARRIER_ARL_MEMORY  0

#define HARRIER_MEM_OP_WRITE    0
#define HARRIER_MEM_OP_READ    1

#define VLAN_MEMORY 2 
#define ARL_MEMORY  3

/* Define Table Type for Access */
#define HARRIER_ARL_TABLE_ACCESS       0x0
#define HARRIER_VLAN_TABLE_ACCESS      0x1
#define HARRIER_MARL_PBMP_TABLE_ACCESS 0x2
#define HARRIER_MSPT_TABLE_ACCESS      0x3
#define HARRIER_VLAN2VLAN_TABLE_ACCESS       0x4
#define HARRIER_MAC2VLAN_TABLE_ACCESS      0x5
#define HARRIER_PROTOCOL2VLAN_TABLE_ACCESS 0x6
#define HARRIER_FLOW2VLAN_TABLE_ACCESS      0x7

/* ---- internal MASK ---- */
#define HARRIER_MASK_PROTOCOL2VLAN_INDEX   0xF

/* MAC2VLAN hash bucket id */
#define HARRIER_MACVLAN_BIN0       0
#define HARRIER_MACVLAN_BIN1       1
#define HARRIER_MACVLAN_UNAVAIL    -1

/*
 *  Function : _drv_harrier_mem_macvlan_locate
 *
 *  Purpose :
 *      special write procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit    :   unit id
 *      mac_addr:   MAC address (key) for s
 *      data0   :   (OUTPUT)entry data0 pointer
 *      data1   :   (OUTPUT)entry data1 pointer
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int 
_drv_harrier_mem_macvlan_locate(int unit, sal_mac_addr_t mac_addr, 
                uint64 *data0, uint64 *data1)
{
    uint32  reg_val32, count, temp;
    uint64  reg_val64, mac_field;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NONE;
    
    COMPILER_64_ZERO(reg_val64);
    COMPILER_64_ZERO(mac_field);

    MEM_LOCK(unit, INDEX(MAC2VLANm));
    /* 1. set arla_mac and arla_vid */
    if (mac_addr == NULL){
        MEM_UNLOCK(unit, INDEX(MAC2VLANm));
        return SOC_E_PARAM;
    }
    
    SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);

    soc_ARLA_MACr_field_set(unit, (uint32 *)&reg_val64,
        MAC_ADDR_INDXf, (uint32 *)&mac_field);

    rv = REG_WRITE_ARLA_MACr(unit, (uint32 *)&reg_val64);
    if (SOC_FAILURE(rv)) {
        MEM_UNLOCK(unit, INDEX(MAC2VLANm));
        return rv;
    }
 
   /* always set arla_vid to zero to prevent the improper MAC2VLAN table
     * hash result.
     */
    temp = 0;
    soc_ARLA_VIDr_field_set(unit, &reg_val32, 
        ARLA_VIDTAB_INDXf, &temp);

    rv = REG_WRITE_ARLA_VIDr(unit, &reg_val32);
    if (SOC_FAILURE(rv)) {
        MEM_UNLOCK(unit, INDEX(MAC2VLANm));
        return rv;
    }

    /* 2. set arla_rwctl(read), check for command DONE. */
    MEM_RWCTRL_REG_LOCK(soc);
    if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_locate_exit;
    }
    temp = MEM_TABLE_READ;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        TAB_RWf, &temp);
    temp = HARRIER_MAC2VLAN_TABLE_ACCESS;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        TAB_INDEXf, &temp);
    temp = 1;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        ARL_STRTDNf, &temp);
    if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_locate_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_locate_exit;
        }
        soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
        ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_locate_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

    /* 3. get othere_table_data0 , othere_table_data1 */
    if ((rv = REG_READ_OTHER_TABLE_DATA0r(unit, (uint32 *)data0)) < 0) {
        goto mem_locate_exit;
    }
    
    if ((rv = REG_READ_OTHER_TABLE_DATA1r(unit, (uint32 *)data1)) < 0) {
        goto mem_locate_exit;
    }

 mem_locate_exit:
    MEM_UNLOCK(unit, INDEX(MAC2VLANm));

    return SOC_E_NONE;
}

/*
 *  Function : _drv_harrier_mem_macvlan_search
 *
 *  Purpose :
 *      special search procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry       :   entry data pointer
 *      mv_entry    :   (OUTPUT)search return entry
 *      index       :   (OUTPUTindex key if exist.
 *                          here we used to indicate the bin ID
 *                          (MAC2VLAN is a hashed table).
 *              1). if exist, index is that existed entry bin_id
 *              2). if not_found, then
 *                      >> index = -1, bin0 & bin1 both unavailable.
 *                      >> index = 0, bin0 is avaliable. 
 *                      >> index = 1, bin1 is avaiable.
 *                      P.S. both available got index = 0 still.
 *
 *  Return :
 *      SOC_E_EXISTS | SOC_E_NOT_FOUND
 *
 *  Note :
 *      The proper design should integrates this routine into 
 *      formal mem_search(). --- TBD 
 *
 */
static int 
_drv_harrier_mem_macvlan_search(int unit, uint32 *entry, 
                mac2vlan_entry_t *mv_entry, uint32 *index)
{
    mac2vlan_entry_t    mv_entry0, mv_entry1;
    sal_mac_addr_t      ent_mac_addr, loc_mac_addr;
    uint32      loc_valid;
    uint64      mac_field;

    /* 1. get the hashed entrys */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                    (uint32 *)entry, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(ent_mac_addr, mac_field);       
    
    sal_memset(mv_entry, 0, sizeof(mac2vlan_entry_t));
    sal_memset(&mv_entry0, 0, sizeof(mac2vlan_entry_t));
    sal_memset(&mv_entry1, 0, sizeof(mac2vlan_entry_t));
    SOC_IF_ERROR_RETURN(_drv_harrier_mem_macvlan_locate(
                    unit, ent_mac_addr, (uint64 *)&mv_entry0, 
                    (uint64 *)&mv_entry1));
    
    /* get the data0 vlaid bit */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                    (uint32 *)&mv_entry0, (uint32 *)&loc_valid));
                    
    if (loc_valid){
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                        (uint32 *)&mv_entry0, (uint32 *)&mac_field));
        SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr, mac_field);
        if (!SAL_MAC_ADDR_CMP(loc_mac_addr, ent_mac_addr)){
            mv_entry = &mv_entry0;
            *index = HARRIER_MACVLAN_BIN0;
            return SOC_E_EXISTS;
        } else {
            /* assign to bin1 first for not found case */
            *index = HARRIER_MACVLAN_BIN1;     
        }
    } else {
        /* assign to bin0 for not found case */
        *index = HARRIER_MACVLAN_BIN0;     
    }
    
    /* get the data1 vlaid bit */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                    (uint32 *)&mv_entry1, (uint32 *)&loc_valid));
                    
    if (loc_valid){
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                        (uint32 *)&mv_entry1, (uint32 *)&mac_field));
        SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr, mac_field);
        if (!SAL_MAC_ADDR_CMP(loc_mac_addr, ent_mac_addr)){
            mv_entry = &mv_entry1;
            *index = HARRIER_MACVLAN_BIN1;
            return SOC_E_EXISTS;
        } else {
            if (*index == HARRIER_MACVLAN_BIN1){
                *index = HARRIER_MACVLAN_UNAVAIL; 
            }
        }
    }
   
    return SOC_E_NOT_FOUND;
}

/*
 *  Function : _drv_harrier_mem_macvlan_write
 *
 *  Purpose :
 *      special write procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer
 *      flags       :   flag control.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int 
_drv_harrier_mem_macvlan_write(int unit, uint32 *entry, uint32 flags)
{
    mac2vlan_entry_t    mv_entry0, mv_entry1;
    sal_mac_addr_t      ent_mac_addr, loc_mac_addr0, loc_mac_addr1;
    uint32              ent_valid, loc_valid0, loc_valid1;
    uint32      reg_val32, count, temp, reg_id;
    uint64      reg_val64, mac_field;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NONE;

    if (entry == NULL){
        return SOC_E_PARAM;
    }
    
    /* get valid field */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)entry, &ent_valid));
                    
    /* 1. get the hashed entrys */                 
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                (uint32 *)entry, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(ent_mac_addr, mac_field);       
    
    sal_memset(&mv_entry0, 0, sizeof(mac2vlan_entry_t));
    sal_memset(&mv_entry1, 0, sizeof(mac2vlan_entry_t));
    _drv_harrier_mem_macvlan_locate(unit, ent_mac_addr, 
                (uint64 *)&mv_entry0, (uint64 *)&mv_entry1);
    
    /* get MAC 0/1 */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                (uint32 *)&mv_entry0, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr0, mac_field);
    
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                (uint32 *)&mv_entry1, (uint32 *)&mac_field));
    SAL_MAC_ADDR_FROM_UINT64(loc_mac_addr1, mac_field);
    
    /* get valid 0/1 */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&mv_entry0, &loc_valid0));
    
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&mv_entry1, &loc_valid1));
    
    /* get the target entry to write :
     *  - default write to bin0 unless :
     *      1. valid1 + (MAC1 == writing_MAC).  #### bin1
     *      2. valid0 + (MAC0 != writing_MAC) and no valid1.    #### bin1
     */
    
    if ((SAL_MAC_ADDR_CMP(loc_mac_addr1, ent_mac_addr) == 0) && 
                    loc_valid1 == 1){
        reg_id = 1;
    } else if ((SAL_MAC_ADDR_CMP(loc_mac_addr0, ent_mac_addr) != 0) && 
                    (loc_valid0 == 1) && loc_valid1 == 0){
        reg_id = 1;
    } else {
        reg_id = 0;
    }
    
    /* start to write ------------- */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* write to target entry (data0 or data1) */
    if (reg_id == 0) {
        if ((rv = REG_WRITE_OTHER_TABLE_DATA0r(unit, 
            (uint32 *)entry)) < 0) {
            goto mv_mem_write_exit;
        }
    } else { /* reg_id == 1 */
        if ((rv = REG_WRITE_OTHER_TABLE_DATA1r(unit, 
            (uint32 *)entry)) < 0) {
            goto mv_mem_write_exit;
        }
    }

    /*  1. clear arla_vid */
    temp = 0;

    soc_ARLA_VIDr_field_set(unit, &reg_val32,
        ARLA_VIDTAB_INDXf, &temp);
    
    rv = REG_WRITE_ARLA_VIDr(unit, &reg_val32);
    if (SOC_FAILURE(rv)) {
        MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        return rv;
    }

    /*  2. set arla_mac */
    SAL_MAC_ADDR_TO_UINT64(ent_mac_addr, mac_field);
    soc_ARLA_MACr_field_set(unit, (uint32 *)&reg_val64,
        MAC_ADDR_INDXf, (uint32 *)&mac_field);

    rv = REG_WRITE_ARLA_MACr(unit, (uint32 *)&reg_val64);
    if (SOC_FAILURE(rv)) {
        MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        return rv;
    }
    
    /*  3. set arla_rwctl */
    MEM_RWCTRL_REG_LOCK(soc);
    if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mv_mem_write_exit;
    }
    temp = MEM_TABLE_WRITE;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        TAB_RWf, &temp);
    temp = HARRIER_MAC2VLAN_TABLE_ACCESS;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        TAB_INDEXf, &temp);
    temp = 1;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        ARL_STRTDNf, &temp);

    if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mv_mem_write_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_write_exit;
        }
        soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
            ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mv_mem_write_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

 mv_mem_write_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));    
        
    return rv;
}


/*
 *  Function : _drv_harrier_mem_macvlan_delete
 *
 *  Purpose :
 *      special delete procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry       :   entry data pointer
 *      flags       :   flag control.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int 
_drv_harrier_mem_macvlan_delete(int unit, uint32 *entry, uint32 flags)
{
    mac2vlan_entry_t    m2v_output;
    uint64  reg_val64, mac_field;
    uint32  reg_val32, count, temp, reg_id;
    uint32  bin_id;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int rv;

    if (entry == NULL){
        return SOC_E_PARAM;
    }
    sal_memset(&m2v_output, 0, sizeof(mac2vlan_entry_t));
    if ((rv = _drv_harrier_mem_macvlan_search(unit, 
                    entry, &m2v_output, &bin_id))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            return rv;;
        }
    }
    
    if (rv == SOC_E_EXISTS){

        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_MAC,
                    (uint32 *)entry, (uint32 *)&mac_field));
        
        /* start to write ------------- */
        MEM_LOCK(unit, INDEX(GEN_MEMORYm));
        reg_id = (bin_id) ? 1 : 0;

        /* write to target entry (data0 or data1) */
        sal_memset(&m2v_output, 0, sizeof(mac2vlan_entry_t));
        if (reg_id == 0) {
            if ((rv = REG_WRITE_OTHER_TABLE_DATA0r(unit, 
                (uint32 *)&m2v_output)) < 0) {
                goto mv_mem_delete_exit;
            }
        } else { /* reg_id == 1 */
            if ((rv = REG_WRITE_OTHER_TABLE_DATA1r(unit, 
                (uint32 *)&m2v_output)) < 0) {
                goto mv_mem_delete_exit;
            }
        }

        /*  1. set arla_mac */
        soc_ARLA_MACr_field_set(unit, (uint32 *)&reg_val64,
            MAC_ADDR_INDXf, (uint32 *)&mac_field);

        rv = REG_WRITE_ARLA_MACr(unit, (uint32 *)&reg_val64);
        if (SOC_FAILURE(rv)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return rv;
        }

        /*  2. set arla_rwctl */
        MEM_RWCTRL_REG_LOCK(soc);
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_delete_exit;
        }
        temp = MEM_TABLE_WRITE;
        soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
            TAB_RWf, &temp);
        temp = HARRIER_MAC2VLAN_TABLE_ACCESS;
        soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
            TAB_INDEXf, &temp);
        temp = 1;
        soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
            ARL_STRTDNf, &temp);

        if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_delete_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mv_mem_delete_exit;
            }
            soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
                ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mv_mem_delete_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);

 mv_mem_delete_exit:
        MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));   
    }
    
    return rv;
}

/*
 *  Function : _drv_harrier_mem_protocolvlan_search
 *
 *  Purpose :
 *      special search procedure to PROTOCOL2VLAN table
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer
 *      pv_entry    :   (OUTPUT)search return entry
 *      index       :   (OUTPUT)index for exist.
 *
 *  Return :
 *      SOC_E_EXISTS | SOC_E_NOT_FOUND
 *
 *  Note :
 *      The proper design should integrates this routine into 
 *      formal mem_search(). --- TBD 
 *
 */
static int 
_drv_harrier_mem_protocolvlan_search(int unit, uint32 *entry, 
                protocol2vlan_entry_t *pv_entry, uint32 *index)
{
    uint32  field_temp, ether_type, ent_id;

    *index =  -1;
    /* get target ether_type */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
            entry, &field_temp));
    
    ent_id = field_temp & HARRIER_MASK_PROTOCOL2VLAN_INDEX;

    SOC_IF_ERROR_RETURN(DRV_MEM_READ(unit, 
            DRV_MEM_PROTOCOLVLAN, ent_id, 1, (uint32 *)pv_entry));
    
    /* get current ether_type */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
            (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
            (uint32 *)pv_entry, &ether_type));
            
    if (ether_type == field_temp){
        
        /* get current valid bit */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)pv_entry, &field_temp));
        if (!field_temp) {
            return SOC_E_NOT_FOUND;
        }
    } else {
        return SOC_E_NOT_FOUND;
    }

    *index = ent_id;
    return SOC_E_EXISTS;
}

/*
 *  Function : _drv_harrier_mem_macvlan_delete
 *
 *  Purpose :
 *      special delete procedure to MAC2VLAN table
 *
 *  Parameters :
 *      unit        :   unit id
 *      entry       :   entry data pointer
 *      flags       :   flag control.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int 
_drv_harrier_mem_protococlvlan_delete(int unit, uint32 *entry, uint32 flags)
{
    protocol2vlan_entry_t    p2v_output;
    uint32  field_val32;
    uint32  ent_id;
    int rv;
    
    sal_memset(&p2v_output, 0, sizeof(protocol2vlan_entry_t));
    if ((rv = _drv_harrier_mem_protocolvlan_search(unit, 
                    entry, &p2v_output, &ent_id))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            return rv;;
        }
    }
    
    if (rv == SOC_E_EXISTS){

        /* set valid bit */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                (uint32 *)&p2v_output, &field_val32));
        /* set vid */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VLANID,
                (uint32 *)&p2v_output, &field_val32));
        /* set priority */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_PRIORITY,
                (uint32 *)&p2v_output, &field_val32));
                
        /* set priority */
        field_val32 = 0;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
                (uint32 *)&p2v_output, &field_val32));
        
        /* PROTOCOL2VLAN write */
        rv = DRV_MEM_WRITE(unit, 
                DRV_MEM_PROTOCOLVLAN,ent_id, 1, (uint32 *)&p2v_output);
                
    }
    
    return rv;
}

/*
 *  Function : _drv_harrier_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *      index   :   entry index in this memory bank (0 or 1).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. In the MAC+VID hash search section, there are two search operations 
 *      will be preformed.
 *    a. Match basis search : report the valid entry with MAC+VID matched.
 *    b. Conflict basis search : report all valid entries in the MAC+VID 
 *      hashed bucket.
 */
static int 
_drv_harrier_mem_search(int unit, uint32 mem, uint32 *key, 
                                    uint32 *entry, uint32 *entry_1, uint32 flags, int *index)
{
    int             rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    uint32          count, temp = 0, value;
    uint32          control = 0;
    uint8           mac_addr_rw[6], temp_mac_addr[6];
    uint64          rw_mac_field, temp_mac_field;
    uint64          entry0, entry1, *input;
    uint32          vid_rw, vid1 = 0, vid2 = 0; 
    int             binNum = -1, existed = 0;
    uint32          reg_value;
    int             i;
    uint32          process, search_valid = 0;
    uint32          src_port;
    uint32 mcast_index;

    COMPILER_64_ZERO(temp_mac_field);
    COMPILER_64_ZERO(rw_mac_field);
    if (flags & DRV_MEM_OP_SEARCH_DONE) {
        if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &reg_value)) < 0) {
            goto mem_search_done;
        }
        soc_ARLA_SRCH_CTLr_field_get(unit, &reg_value,
            ARLA_SRCH_STDNf, &temp);
        if (temp) {
            rv = SOC_E_BUSY;    
        }else {
            rv = SOC_E_NONE;
        }
mem_search_done:
        return rv;
            
    } else if (flags & DRV_MEM_OP_SEARCH_VALID_START) {
        /* Set ARL Search Control register */
        if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            goto mem_search_valid_start;
        }
        process = 1;
        soc_ARLA_SRCH_CTLr_field_set(unit, &control,
            ARLA_SRCH_STDNf, &process);
        if ((rv = REG_WRITE_ARLA_SRCH_CTLr(unit, &control)) < 0) {
            goto mem_search_valid_start;
        }
mem_search_valid_start:
        return rv;

    } else if (flags & DRV_MEM_OP_SEARCH_VALID_GET) {
        if (flags & DRV_MEM_OP_SEARCH_PORT) {
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    key, &src_port)) < 0) {
                goto mem_search_valid_get;
            }
        }
 
        process = 1;
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_ARLA_SRCH_CTLr(unit, &control)) < 0) {
                goto mem_search_valid_start;
            }
            soc_ARLA_SRCH_CTLr_field_get(unit, &control,
                ARLA_SRCH_STDNf, &process);
            soc_ARLA_SRCH_CTLr_field_get(unit, &control,
                ARLA_SRCH_VLIDf, &search_valid);
            /* ARL search operation was done */
            if (!process){
                break;
            }
            if (!search_valid){
                continue;
            }
            count = 0;
            temp = 1;
            /* Set VALID field */
            if ((rv = DRV_MEM_FIELD_SET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_VALID, entry, &temp))){
                goto mem_search_valid_get;
            }

            if (!(flags & DRV_MEM_OP_SEARCH_PORT)) {
                if ((rv = REG_READ_ARLA_SRCH_ADRr(unit, &reg_value)) < 0) {
                    goto mem_search_valid_get;
                }
                soc_ARLA_SRCH_ADRr_field_get(unit, &reg_value, 
                    SRCH_ADRf, key);
            }

            /* Read ARL Search Result VID Register */
            vid_rw = 0;
            if ((rv = REG_READ_ARLA_SRCH_RSLT_VIDr(unit, &reg_value)) < 0) {
                goto mem_search_valid_get;
            }
            soc_ARLA_SRCH_RSLT_VIDr_field_get(unit, &reg_value,
                ARLA_SRCH_RSLT_VIDf, &vid_rw);
            if ((rv = DRV_MEM_FIELD_SET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid_rw)) < 0) {
                goto mem_search_valid_get;
            }

            /* Read ARL Search Result MAC register */
            REG_READ_ARLA_SRCH_RSLTr(unit,(uint32 *)&entry0);
                    
            if (flags & DRV_MEM_OP_SEARCH_PORT) {
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                if (temp != src_port) {
                    continue;
                }
            }

            COMPILER_64_ZERO(temp_mac_field);
            /* MAC Address */
            soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                ARLA_SRCH_RSLT_ADDRf, (uint32 *)&temp_mac_field);
            if ((rv = DRV_MEM_FIELD_SET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&temp_mac_field)) < 0){
                goto mem_search_valid_get;
            }
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* mcast address */
                /* The the multicast format didn't define, we need 
                   collect 3 fields to get the multicast index value.
                   multicast index : bit 55~48
                   age : bit 55
                   priority : bit 54~53
                   port id : bit 52~48
                */
                mcast_index = 0;
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                mcast_index = temp;
                 
                /* Port number should add 24 for HARRIER */
                mcast_index += 24;

                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ENTRY_RSRV0f, &temp);
                mcast_index += (temp << 6);
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_AGEf, &temp);
                mcast_index += (temp << 11);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                        entry, (uint32 *)&mcast_index)) < 0){
                    goto mem_search_valid_get;
                }

                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_STATICf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        entry, &temp)) < 0){
                    goto mem_search_valid_get;
                }
                /* arl_control */
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARL_SRCH_RSLT_CONf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        entry, &temp)) < 0){
                    goto mem_search_valid_get;
                }

            } else { /* unicast address */
                /* Source Port Number */
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_PRIDf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        entry, &temp)) < 0){
                    goto mem_search_valid_get;
                }
            
                /* Static Bit */
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_STATICf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        entry, &temp)) < 0){
                    goto mem_search_valid_get;
                }

                /* Hit bit */
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARLA_SRCH_RSLT_AGEf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                        entry, &temp)) < 0){
                    goto mem_search_valid_get;
                }

                /* arl_control */
                soc_ARLA_SRCH_RSLTr_field_get(unit, (uint32 *)&entry0,
                    ARL_SRCH_RSLT_CONf, &temp);
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        entry, &temp)) < 0){
                    goto mem_search_valid_get;
                } 

            }
            rv = SOC_E_EXISTS;
            goto mem_search_valid_get;
        }
        rv = SOC_E_TIMEOUT;
mem_search_valid_get:
        return rv;

    } else if (flags & DRV_MEM_OP_BY_INDEX) {
        /*
         * Since no one calls this case, left it empty temporary.
         */
        LOG_CLI((BSL_META_U(unit,
                            "_drv_harrier_mem_search: flag = DRV_MEM_OP_BY_INDEX\n")));
        return SOC_E_UNAVAIL;
    /* delete by MAC */    
    } else if (flags & DRV_MEM_OP_BY_HASH_BY_MAC) {
        l2_arl_sw_entry_t   *rep_entry;
        int is_conflict[ROBO_HARRIER_L2_BUCKET_SIZE];

        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            for (i = 0; i < ROBO_HARRIER_L2_BUCKET_SIZE; i++){
                /* check the parameter for output entry buffer */
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
                if (rep_entry == NULL){
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s,entries buffer not allocated!\n"), 
                              FUNCTION_NAME()));
                    return SOC_E_PARAM;
                }

                sal_memset(rep_entry, 0, sizeof(l2_arl_sw_entry_t));
                is_conflict[i] = FALSE;
            }
        }
    
        ARL_MEM_SEARCH_LOCK(soc);
        /* enable 802.1Q and set VID+MAC to hash */
        if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
            goto mem_search_exit;
        }
        temp = 1;
        soc_VLAN_CTRL0r_field_set(unit, &reg_value,
            VLAN_ENf, &temp);
        
        /* check IVL or SVL */
        soc_VLAN_CTRL0r_field_get(unit, &reg_value,
            VLAN_LEARN_MODEf, &temp);
        if (temp == 3){     /* VLAN is at IVL mode */
            if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                temp = 3;
            } else {
                temp = 0;
            }
            soc_VLAN_CTRL0r_field_set(unit, &reg_value,
                VLAN_LEARN_MODEf, &temp);
        }
        if ((rv = REG_WRITE_VLAN_CTRL0r(unit, &reg_value)) < 0) {
            goto mem_search_exit;
        }
        /* Write MAC Address Index Register */
        if ((rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                key, (uint32 *)&rw_mac_field)) < 0) {
            goto mem_search_exit;
        }
        SAL_MAC_ADDR_FROM_UINT64(mac_addr_rw, rw_mac_field);
        if ((rv = REG_WRITE_ARLA_MACr(unit, (uint32 *)&rw_mac_field)) < 0) {
            goto mem_search_exit;
        }

        if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
            /* Write VID Table Index Register */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                    key, &vid_rw)) < 0) {
                goto mem_search_exit;
            }
            if ((rv = REG_WRITE_ARLA_VIDr(unit, &vid_rw)) < 0) {
                goto mem_search_exit;
            }
        }

        /* Write ARL Read/Write Control Register */
        /* Read Operation */
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
            goto mem_search_exit;
        }
        temp = MEM_TABLE_READ;
        soc_ARLA_RWCTLr_field_set(unit, &control, 
            TAB_RWf, &temp);

        temp = HARRIER_ARL_TABLE_ACCESS;
        soc_ARLA_RWCTLr_field_set(unit, &control, 
            TAB_INDEXf, &temp);

        temp = 1;
        soc_ARLA_RWCTLr_field_set(unit, &control, 
            ARL_STRTDNf, &temp);
        if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &control)) < 0) {
            goto mem_search_exit;
        }

        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
                goto mem_search_exit;
            }
            soc_ARLA_RWCTLr_field_get(unit, &control, 
                ARL_STRTDNf, &temp);
            if (!temp)
                break;
        }

        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_search_exit;
        }

        /* Read Operation sucessfully */
        /* Get the ARL Entry 0/1 Register */
        if ((rv = REG_READ_ARLA_ENTRY_0r(unit, (uint32 *)&entry0)) < 0) {
            goto mem_search_exit;
        }

        if ((rv = REG_READ_ARLA_ENTRY_1r(unit, (uint32 *)&entry1)) < 0) {
            goto mem_search_exit;
        }

        /* check DRV_MEM_OP_REPLACE */
        if (flags & DRV_MEM_OP_REPLACE) {
            uint32 temp_valid1 = 0;
            uint32 temp_valid2 = 0;
            uint32 temp_static1 = 0;
            uint32 temp_static2 = 0;
        
            LOG_INFO(BSL_LS_SOC_ARL,
                     (BSL_META_U(unit,
                                 "DRV_MEM_OP_REPLACE\n")));

            /* Check the ARL Entry 0 Register */
            soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)&entry0,
                ARL_VALIDf, &temp_valid1);

            /* Check the ARL Entry 0 Register */
            soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)&entry0,
                ARL_STATICf, &temp_static1);

            /* Check the ARL Entry 1 Register */
            soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)&entry1,
                ARL_VALIDf, &temp_valid2);

            /* Check the ARL Entry 1 Register */
            soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)&entry1,
                ARL_STATICf, &temp_static2);
            
            if (temp_valid1) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[0] = TRUE;
                }

                COMPILER_64_ZERO(temp_mac_field);
                /* bin 0 valid, check mac or mac+vid */
                /* get mac_addr */
                soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)&entry0,
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                /* get vid */
                REG_READ_ARLA_VID_ENTRY_0r(unit, &vid1);  

                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && (vid1 == vid_rw)) {
                        /* select bin 0 to overwrite it */
                            binNum = 0;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 0 to overwrite it */                
                            binNum = 0;
                    }                
                } 
            } 
        
            if (temp_valid2) {
                /* bin 1 valid */          

                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[1] = TRUE;
                }

                /* get mac_addr */
                soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)&entry1,
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                /* get vid */
                REG_READ_ARLA_VID_ENTRY_1r(unit, &vid2);
                
                /* check if we have to overwrite this valid bin0 */
                if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                    /* check mac + vid */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6) && (vid2 == vid_rw)) {
                        /* select bin 1 to overwrite it */                
                            binNum = 1;
                    }
                } else {
                    /* check mac */
                    if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                        /* select bin 1 to overwrite it */               
                            binNum = 1;
                    }                
                } 
            }
        
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                /* can't find a entry to overwrite based on same mac + vid */
                if (binNum == -1) {
                    if (temp_valid1 == 0) {
                        binNum = 0;
                    } else if (temp_valid2 == 0) {
                        binNum = 1;
                    } else {
                        /* both valid, pick non-static one */
                        if (temp_static1 == 0) {
                            binNum = 0;
                        } else if (temp_static2 == 0) {
                            binNum = 1;
                        } else {
                            /* table full */
                            rv = SOC_E_FULL;
                            goto mem_search_exit;                    
                        }
                    }
                }
            }
        /* Not DRV_MEM_OP_REPLACE */
        } else {
            /* Check the ARL Entry 0 Register */
            soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)&entry0,
                ARL_VALIDf, &temp);
            if (temp) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[0] = TRUE;
                }
                
                /* this entry if valid, check to see if this is the MAC */
                soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)&entry0,
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                REG_READ_ARLA_VID_ENTRY_0r(unit, &vid1);  
                if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                        if (vid1 == vid_rw) {
                            binNum = 0;
                            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                                existed = 1;
                            }
                        }
                    } else {
                        binNum = 0;
                        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                            existed = 1;
                        }
                    }
                }
            } else {
                binNum = 0;
            }

            /* Check the ARL Entry 1 Register */
            soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)&entry1,
                ARL_VALIDf, &temp);
            if (temp) {
                if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                    is_conflict[1] = TRUE;
                }

                /* this entry if valid, check to see if this is the MAC */
                soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)&entry1,
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
                REG_READ_ARLA_VID_ENTRY_1r(unit, &vid2);  
                if (!memcmp(temp_mac_addr, mac_addr_rw, 6)) {
                    if (flags & DRV_MEM_OP_BY_HASH_BY_VLANID) {
                        if (vid2 == vid_rw) {
                            binNum = 1;
                            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                                existed = 1;
                            }
                        }
                    } else {
                        binNum = 1;
                        if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                            existed = 1;
                        }
                    }
                }
            } else {
                if (binNum == -1) binNum = 1;
            }
            
            if (!(flags & DRV_MEM_OP_SEARCH_CONFLICT)){
                /* if no entry found, fail */
                if (binNum == -1) {
                    rv = SOC_E_FULL;
                    goto mem_search_exit;
                }
            }
        }
        
        for (i = 0; i < ROBO_HARRIER_L2_BUCKET_SIZE; i++){
            if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
                if (is_conflict[i] == FALSE){
                    continue;
                }

                existed = 1;
                rep_entry = (l2_arl_sw_entry_t *)entry + i;
            } else {
                /* match basis search */
                if (i != binNum){
                    continue;
                }

                rep_entry = (l2_arl_sw_entry_t *)entry;
            }

            /* assign the processing hw arl entry */
            if (i == 0) {
                input = &entry0;
                vid_rw = vid1;
            } else {
                input = &entry1;
                vid_rw = vid2;
            }
            *index = i;

            /* Only need write the selected Entry Register */
            soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)input,
                    ARL_MACADDRf, (uint32 *)&temp_mac_field);
            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            if (temp_mac_addr[0] & 0x01) { /* The input is the mcast address */
                if ((rv = DRV_MEM_FIELD_SET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field)) < 0){
                    goto mem_search_exit;
                }
                value = 0;
                soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)input,
                    ARL_PIDf, &temp);
                value = temp;
                
                /* Port number should add 24 for HARRIER */
                value += 24;

                soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)input,
                    ARL_ENTRY_RSRV0f, &temp);
                value |= (temp << 6);
                soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)input,
                    ARL_AGEf, &temp);
                value |= (temp << 9);
                if ((rv = DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
                        (uint32 *)rep_entry, &value)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &vid_rw)) < 0){
                    goto mem_search_exit;
                }
                soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)input,
                    ARL_CONf, &temp);
                if ((rv = DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
                temp = 1;
                if ((rv = DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((rv = DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
            } else { /* The input is the unicast address */
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)rep_entry, (uint32 *)&temp_mac_field)) < 0){
                    goto mem_search_exit;
                }
                soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)input,
                    ARL_PIDf, &temp);
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
                soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)input,
                    ARL_AGEf, &temp);
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)rep_entry, &vid_rw)) < 0){
                    goto mem_search_exit;
                }
                soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)input,
                    ARL_STATICf, &temp);
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
                soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)input,
                    ARL_CONf, &temp);
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
                temp = 1;
                if ((DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
                        (uint32 *)rep_entry, &temp)) < 0){
                    goto mem_search_exit;
                }
            }
            
        }

        if (flags & DRV_MEM_OP_SEARCH_CONFLICT){
            if (existed){
                rv = SOC_E_EXISTS;
            } else {
                rv = SOC_E_NOT_FOUND;
            }
        } else {
            if (flags & DRV_MEM_OP_REPLACE) {
                rv = SOC_E_NONE;
            } else {
                 if(existed)
                    rv = SOC_E_EXISTS;    
                 else
                    rv = SOC_E_NOT_FOUND;
            }
        }

mem_search_exit:
        ARL_MEM_SEARCH_UNLOCK(soc);
        return rv;

    } else {
        return SOC_E_PARAM;
    }

}

static int 
_drv_harrier_mem_arl_entry_delete(int unit, uint8 *mac_addr, uint32 vid, int index)
{
    uint32 reg_val32, count, temp;
    uint64 reg_val64, mac_field;
    int rv = SOC_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);

    /* write MAC Addr */
    if ((rv = REG_READ_ARLA_MACr(unit, (uint32 *)&reg_val64)) < 0) {
        goto marl_entry_delete_exit;
    }

    SAL_MAC_ADDR_TO_UINT64(mac_addr, mac_field);
    soc_ARLA_MACr_field_set(unit, (uint32 *)&reg_val64,
            MAC_ADDR_INDXf, (uint32 *)&mac_field);

    if ((rv = REG_WRITE_ARLA_MACr(unit, (uint32 *)&reg_val64)) < 0) {
        goto marl_entry_delete_exit;
    }

    /* write VID */
    if ((rv = REG_READ_ARLA_VIDr(unit, &reg_val32)) < 0) {
        goto marl_entry_delete_exit;
    }
    soc_ARLA_VIDr_field_set(unit, &reg_val32,
        ARLA_VIDTAB_INDXf, &vid);

    if ((rv = REG_WRITE_ARLA_VIDr(unit, &reg_val32)) < 0) {
        goto marl_entry_delete_exit;
    }

    if (index == 0) {
        /* entry 0 */
        /* Clear entire entry */
        COMPILER_64_ZERO(reg_val64);
        if ((rv = REG_WRITE_ARLA_ENTRY_0r(unit, (uint32 *)&reg_val64)) < 0) {
            goto marl_entry_delete_exit;
        }

        /* Clear VID entry */
        reg_val32 = 0;
        if ((rv = REG_WRITE_ARLA_VID_ENTRY_0r(unit, &reg_val32)) < 0) {
            goto marl_entry_delete_exit;
        }
    } else {
        /* entry 1 */
        /* Clear entire entry */
        COMPILER_64_ZERO(reg_val64);
        if ((rv = REG_WRITE_ARLA_ENTRY_1r(unit, (uint32 *)&reg_val64)) < 0) {
            goto marl_entry_delete_exit;
        }

        /* Clear VID entry */
        reg_val32 = 0;
        if ((rv = REG_WRITE_ARLA_VID_ENTRY_1r(unit, &reg_val32)) < 0) {
            goto marl_entry_delete_exit;
        }
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Write ARL Read/Write Control Register */
    if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }
    temp = MEM_TABLE_WRITE;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        TAB_RWf, &temp);
    temp = HARRIER_ARL_TABLE_ACCESS;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        TAB_INDEXf, &temp);
    temp = 1;
    soc_ARLA_RWCTLr_field_set(unit, &reg_val32, 
        ARL_STRTDNf, &temp);
    if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &reg_val32)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto marl_entry_delete_exit;
        }
        soc_ARLA_RWCTLr_field_get(unit, &reg_val32, 
            ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto marl_entry_delete_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);

marl_entry_delete_exit:
    return rv;
}   

/*
 *  Function : _drv_harrier_mem_table__reset
 *
 *  Purpose :
 *
 *  Parameters :
 *      unit        :   unit id
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
static int
_drv_harrier_mem_table_reset(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;
    uint32 retry;
    uint32  reg_len, reg_addr, temp, reg_value;
    uint32 fld_index, reg_index;
    soc_control_t           *soc = SOC_CONTROL(unit);
    
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
            reg_index = INDEX(RST_TABLE_MEMr);
            fld_index = INDEX(RST_ARLf);
            break;
        case DRV_MEM_MCAST:
            reg_index = INDEX(RST_TABLE_MEMr);
            fld_index = INDEX(RST_IPMCf);
            break;
        case DRV_MEM_VLAN:
            reg_index = INDEX(RST_TABLE_MEMr);
            fld_index = INDEX(RST_VTf);
            break;
        case DRV_MEM_MSTP:
            reg_index = INDEX(RST_TABLE_MEMr);
            fld_index = INDEX(RST_MSPTf);
            break;
        case DRV_MEM_VLANVLAN:
            reg_index = INDEX(RST_TABLE_MEM1r);
            fld_index = INDEX(RST_VLAN2VLANf);
            break;
        case DRV_MEM_MACVLAN:
            reg_index = INDEX(RST_TABLE_MEM1r);
            fld_index = INDEX(RST_MAC2VLANf);
            break;
        case DRV_MEM_PROTOCOLVLAN:
            reg_index = INDEX(RST_TABLE_MEM1r);
            fld_index = INDEX(RST_PROTOCOL2VLANf);
            break;
        case DRV_MEM_FLOWVLAN:
            reg_index = INDEX(RST_TABLE_MEM1r);
            fld_index = INDEX(RST_FLOW2VLANf);
            break;
        case DRV_MEM_MARL:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    MEM_RWCTRL_REG_LOCK(soc);    
    /* read control setting */                
    reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);

    if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }
    
    temp = 1;
    if ((rv = DRV_REG_FIELD_SET(unit, 
            reg_index, &reg_value, fld_index, &temp)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }
    
    if ((rv = DRV_REG_WRITE(unit, reg_addr, &reg_value, reg_len)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }
    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = DRV_REG_READ(unit, reg_addr, &reg_value, reg_len)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_table_reset_exit;
        }
        if ((rv = DRV_REG_FIELD_GET(unit, 
                reg_index, &reg_value, fld_index, &temp)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_table_reset_exit;
        }
        if (!temp) {
            break;
        }
    }

    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_table_reset_exit;
    }

    MEM_RWCTRL_REG_UNLOCK(soc);

    /* The SA learn count is observed not been clear to 0 after ARL table 
     *  reset. Thus we need to force port basis SA learn count here.
     */
    if (mem == DRV_MEM_ARL || DRV_MEM_ARL_HW){
        soc_port_t  port = 0;
        
        PBMP_E_ITER(unit, port) {
            rv = DRV_ARL_LEARN_COUNT_SET(unit, port, DRV_PORT_SA_LRN_CNT_RESET, 0);
            if (rv != SOC_E_NONE){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "Failed on reset ARL LRN_CNT on port %d\n"), port));
                
                return SOC_E_INTERNAL;
            }
        }
        
        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);

    }

mem_table_reset_exit:   
    return rv;
}

static int 
_drv_harrier_mem_marl_delete_all(int unit)
{
    int rv = SOC_E_NONE;
    soc_control_t   *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t output;
    l2_arl_sw_entry_t entry0, entry1;
    uint64 temp_mac_field;
    uint8  temp_mac_addr[6];
    int index_min, index_count;
    int32 idx, temp_vid;
    uint32 valid;
    int index;

    index_min = SOC_MEM_BASE(unit, INDEX(L2_ARLm));
    index_count = SOC_MEM_SIZE(unit, INDEX(L2_ARLm));
    if(soc->arl_table != NULL){
        ARL_SW_TABLE_LOCK(soc);
        for (idx = index_min; idx < index_count; idx++) {
            sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
            if(!ARL_ENTRY_NULL(&soc->arl_table[idx])) {
                sal_memcpy(&output, &soc->arl_table[idx], 
                    sizeof(l2_arl_sw_entry_t));
            } else {
                continue;
            }
            rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MARL, DRV_MEM_FIELD_VALID, 
                    (uint32 *)&output, &valid);
            if (rv != SOC_E_NONE){
                ARL_SW_TABLE_UNLOCK(soc);
                return rv;
            }
            if (valid){
                rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_MARL, DRV_MEM_FIELD_MAC, 
                        (uint32 *)&output, (uint32 *)&temp_mac_field);
                if (rv != SOC_E_NONE){
                    ARL_SW_TABLE_UNLOCK(soc);
                    return rv;
                }
                rv = DRV_MEM_FIELD_GET (unit, 
                        DRV_MEM_MARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)&output, (uint32 *)&temp_vid);
                if (rv != SOC_E_NONE){
                    ARL_SW_TABLE_UNLOCK(soc);
                    return rv;
                }
                SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);

                if (temp_mac_addr[0] & 0x01) { /* mcast address */
                    /* delete HW entry */
                    if ((rv =  _drv_harrier_mem_search
                            (unit, DRV_MEM_ARL, (uint32 *)&output, 
                            (uint32 *)&entry0, (uint32 *)&entry1, 
                            DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID, 
                            &index)) < 0) {
                        if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                            if (rv == SOC_E_FULL) {
                            /* For mem_delete if mem_search return SOC_E_FULL it means
                              * the entry is not found. */
                                rv = SOC_E_NOT_FOUND;
                            }
                        }
                    }
                    if (rv == SOC_E_EXISTS) {
                        rv = _drv_harrier_mem_arl_entry_delete(unit, 
                            temp_mac_addr, temp_vid, index);
                        if (rv < 0) {
                            ARL_SW_TABLE_UNLOCK(soc);
                            return rv;
                        }

                        /* Remove the entry from sw database */
                        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
                        sal_memcpy(&soc->arl_table[idx], &output,
                            sizeof(l2_arl_sw_entry_t));
                    }
                }
           } 
        }

        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);

        ARL_SW_TABLE_UNLOCK(soc);
    } else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc arl table not allocated")));
        rv = SOC_E_FAIL;
    }
    return rv;
}

static int
_drv_harrier_cfp_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            mem_id = INDEX(CFP_TCAM_S0m);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_ACT:
            mem_id = INDEX(CFP_ACT_POLm);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_METER:
            mem_id = INDEX(CFP_METERm);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            mem_id = INDEX(CFP_STAT_IBm);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
            case DRV_MEM_TCAM_DATA:
            case DRV_MEM_TCAM_MASK:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                        DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                if (mem == DRV_MEM_TCAM_DATA) {
                    sal_memcpy(data_ptr, cfp_entry.tcam_data, entry_len);
                } else {
                    sal_memcpy(data_ptr, cfp_entry.tcam_mask, entry_len);
                }
                break;
            case DRV_MEM_CFP_ACT:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                        DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                sal_memcpy(data_ptr, cfp_entry.act_data, entry_len);
                break;
            case DRV_MEM_CFP_METER:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                        DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                sal_memcpy(data_ptr, cfp_entry.meter_data, entry_len);
                break;
            case DRV_MEM_CFP_STAT_IB:
                sal_memset(&counter, 0, sizeof(uint32));
                if ((rv = DRV_CFP_STAT_GET(unit, DRV_CFP_STAT_INBAND, 
                        (entry_id + i), &counter)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                sal_memcpy(data_ptr, &counter, entry_len);
                break;
            case DRV_MEM_CFP_STAT_OB:
                sal_memset(&counter, 0, sizeof(uint32));
                if ((rv = DRV_CFP_STAT_GET(unit, DRV_CFP_STAT_OUTBAND, 
                        (entry_id + i), &counter)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                sal_memcpy(data_ptr, &counter, entry_len);
                break;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}

static int
_drv_harrier_cfp_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    drv_cfp_entry_t cfp_entry;
    int entry_len, rv = SOC_E_NONE;
    uint8 *data_ptr;
    uint32 mem_id, counter;
    uint32 i, index_min, index_max;
    soc_mem_info_t *meminfo;

    data_ptr = (uint8 *)entry;
    /* Get the length of entry */
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
            mem_id = INDEX(CFP_TCAM_S0m);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_ACT:
            mem_id = INDEX(CFP_ACT_POLm);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_METER:
            mem_id = INDEX(CFP_METERm);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            mem_id = INDEX(CFP_STAT_IBm);
            meminfo = &SOC_MEM_INFO(unit, mem_id);
            entry_len = meminfo->bytes;
            break;
        default:
            return SOC_E_PARAM;
    }

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        return rv;
    }

    /* check index */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);

    for (i = 0; i < count; i++) {
        if (((entry_id + i) < index_min) || 
            ((entry_id + i) > index_max)) {
            return SOC_E_PARAM;
        }

        switch (mem) {
            case DRV_MEM_TCAM_DATA:
            case DRV_MEM_TCAM_MASK:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                if ((rv = DRV_CFP_ENTRY_READ(unit, (entry_id + i), 
                        DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                if (mem == DRV_MEM_TCAM_DATA) {
                    sal_memcpy(cfp_entry.tcam_data, data_ptr, entry_len);
                } else {
                    sal_memcpy(cfp_entry.tcam_mask, data_ptr, entry_len);
                }
                if ((rv = DRV_CFP_ENTRY_WRITE(unit, (entry_id + i), 
                        DRV_CFP_RAM_TCAM, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_write(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                break;
            case DRV_MEM_CFP_ACT:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                sal_memcpy(cfp_entry.act_data, data_ptr, entry_len);
                if ((rv = DRV_CFP_ENTRY_WRITE(unit, (entry_id + i), 
                        DRV_CFP_RAM_ACT, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_write(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                break;
            case DRV_MEM_CFP_METER:
                sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
                sal_memcpy(cfp_entry.meter_data, data_ptr, entry_len);
                if ((rv = DRV_CFP_ENTRY_WRITE(unit, (entry_id + i), 
                        DRV_CFP_RAM_METER, &cfp_entry)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_write(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                break;
            case DRV_MEM_CFP_STAT_IB:
                sal_memset(&counter, 0, sizeof(uint32));
                sal_memcpy(&counter, data_ptr, entry_len);
                if ((rv = DRV_CFP_STAT_SET(unit, DRV_CFP_STAT_INBAND, 
                        (entry_id + i), counter)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                break;
            case DRV_MEM_CFP_STAT_OB:
                sal_memset(&counter, 0, sizeof(uint32));
                sal_memcpy(&counter, data_ptr, entry_len);
                if ((rv = DRV_CFP_STAT_SET(unit, DRV_CFP_STAT_OUTBAND, 
                        (entry_id + i), counter)) < 0){
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "_drv_harrier_cfp_read(mem=0x%x,entry_id=0x%x)\n"),
                               mem, entry_id + i));
                    return rv;
                }
                break;
        }
        data_ptr = data_ptr + entry_len;
    }

    return rv;
}

static int
_drv_harrier_cfp_field_get(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;
    
    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_S0m));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_GET(unit, 
                    DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            break;
        case DRV_MEM_TCAM_MASK:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_MASKm));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_GET(unit, 
                    DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            break;
        case DRV_MEM_CFP_ACT:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_ACT_POLm));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.act_data, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_GET(unit, 
                    DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            break;
        case DRV_MEM_CFP_METER:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_METERm));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.meter_data, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_GET(unit, 
                    DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_get(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    return rv;
}


static int
_drv_harrier_cfp_field_set(int unit, uint32 mem, uint32 field_index, 
        uint32 *entry, uint32 *fld_data)
{
    int rv = SOC_E_NONE;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    soc_mem_info_t *meminfo;

    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch (mem) {
        case DRV_MEM_TCAM_DATA:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_S0m));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_data, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_SET(unit, 
                    DRV_CFP_RAM_TCAM, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            sal_memcpy(entry, cfp_entry.tcam_data, entry_len);
            break;
        case DRV_MEM_TCAM_MASK:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_TCAM_MASKm));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.tcam_mask, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_SET(unit, 
                    DRV_CFP_RAM_TCAM_MASK, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            sal_memcpy(entry, cfp_entry.tcam_mask, entry_len);
            break;
        case DRV_MEM_CFP_ACT:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_ACT_POLm));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.act_data, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_SET(unit, 
                    DRV_CFP_RAM_ACT, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            sal_memcpy(entry, cfp_entry.act_data, entry_len);
            break;
        case DRV_MEM_CFP_METER:
            meminfo = &SOC_MEM_INFO(unit, INDEX(CFP_METERm));
            entry_len = meminfo->bytes;
            sal_memcpy(cfp_entry.meter_data, entry, entry_len);
            if ((rv = DRV_CFP_FIELD_SET(unit, 
                    DRV_CFP_RAM_METER, field_index, &cfp_entry, fld_data)) < 0){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "_drv_harrier_cfp_field_set(mem=0x%x,field=0x%x)\n"),
                           mem, field_index));
                return rv;
            }
            sal_memcpy(entry, cfp_entry.meter_data, entry_len);
            break;
        default:
            rv = SOC_E_PARAM;
            break;
    }

    return rv;
}

/*
 *  Function : drv_harrier_mem_length_get
 *
 *  Purpose :
 *      Get the number of entries of the selected memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number entries of this memory type.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_harrier_mem_length_get(int unit, uint32 mem, uint32 *data)
{

    soc_mem_info_t *meminfo;
    
    switch (mem)
    {
        case DRV_MEM_ARL:
            meminfo = &SOC_MEM_INFO(unit, INDEX(L2_ARLm));
            break;
        case DRV_MEM_MARL:
            meminfo = &SOC_MEM_INFO(unit, INDEX(L2_MARL_SWm));
            break;            
        case DRV_MEM_VLAN:
            meminfo = &SOC_MEM_INFO(unit, INDEX(VLAN_1Qm));
            break;
        case DRV_MEM_MSTP:
            meminfo = &SOC_MEM_INFO(unit, INDEX(MSPT_TABm));
            break;
        case DRV_MEM_MCAST:
            meminfo = &SOC_MEM_INFO(unit, INDEX(MARL_PBMPm));
            break;
        case DRV_MEM_GEN:
            meminfo = &SOC_MEM_INFO(unit, INDEX(GEN_MEMORYm));
            break;
        case DRV_MEM_VLANVLAN:
            meminfo = &SOC_MEM_INFO(unit, INDEX(VLAN2VLANm));
            break;
        case DRV_MEM_MACVLAN:
            meminfo = &SOC_MEM_INFO(unit, INDEX(MAC2VLANm));
            break;
        case DRV_MEM_PROTOCOLVLAN:
            meminfo = &SOC_MEM_INFO(unit, INDEX(PROTOCOL2VLANm));
            break;
        case DRV_MEM_FLOWVLAN:
            meminfo = &SOC_MEM_INFO(unit, INDEX(FLOW2VLANm));
            break;
        default:
            return SOC_E_PARAM;
    }

  *data = meminfo->index_max - meminfo->index_min + 1;

  return SOC_E_NONE;
}


/*
 *  Function : drv_harrier_mem_width_get
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      data   :   total number bits of entry.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int 
drv_harrier_mem_width_get(int unit, uint32 mem, uint32 *data)
{

    switch (mem)
    {
        case DRV_MEM_ARL:
            *data = sizeof(l2_arl_entry_t);
            break;
         case DRV_MEM_MARL:
            *data = sizeof(l2_marl_sw_entry_t);
            break;
        case DRV_MEM_VLAN:
            *data = sizeof(vlan_1q_entry_t);
            break;
        case DRV_MEM_MSTP:
            *data = sizeof(mspt_tab_entry_t);
            break;
        case DRV_MEM_MCAST:
            *data = sizeof(marl_pbmp_entry_t);
            break;
        case DRV_MEM_GEN:
            *data = sizeof(gen_memory_entry_t);
            break;
        case DRV_MEM_VLANVLAN:
            *data = sizeof(vlan2vlan_entry_t);
            break;
        case DRV_MEM_MACVLAN:
            *data = sizeof(mac2vlan_entry_t);
            break;
        case DRV_MEM_PROTOCOLVLAN:
            *data = sizeof(protocol2vlan_entry_t);
            break;
        case DRV_MEM_FLOWVLAN:
            *data = sizeof(flow2vlan_entry_t);
            break;
        default:
            return SOC_E_PARAM;
    }
    return SOC_E_NONE;
}

int
_drv_harrier_arl_read(int unit, uint32 entry_id, 
                               uint32 count, uint32 *entry)
{
    
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max, index;
    uint32 mem_id;
    uint32 acc_ctrl = 0, mem_addr = 0;
    uint32  temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "_drv_harrier_arl_read(entry_id=0x%x,count=%d)\n"),
              entry_id, count));

    mem_id = INDEX(L2_ARLm);

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process read action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* check count */
    if (count < 1) {
        MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
        return SOC_E_PARAM;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */

        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(gmem_entry, 
                cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        index = entry_id + i;
        temp = index / 2;
        /* Set memory entry address */
        soc_GENMEM_ADDRr_field_set(unit, &mem_addr,
            GENMEM_ADDRf, &temp);
        if ((rv = REG_WRITE_GENMEM_ADDRr(unit, &mem_addr)) < 0) {
            goto arl_read_exit;
        }

        /* Read memory control register */
        if ((rv = REG_READ_GENMEM_CTLr(unit, &acc_ctrl)) < 0) {
            goto arl_read_exit;
        }
        temp = HARRIER_MEM_OP_READ;
        soc_GENMEM_CTLr_field_set(unit, &acc_ctrl,
            GENMEM_RWf, &temp);
        temp = HARRIER_ARL_MEMORY;
        soc_GENMEM_CTLr_field_set(unit, &acc_ctrl,
            TXDSC_ARLf, &temp);

        temp = 1;
        soc_GENMEM_CTLr_field_set(unit, &acc_ctrl,
            GENMEM_STDNf, &temp);
        if ((rv = REG_WRITE_GENMEM_CTLr(unit, &acc_ctrl)) < 0) {
            goto arl_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_GENMEM_CTLr(unit, &acc_ctrl)) < 0) {
                goto arl_read_exit;
            }
            soc_GENMEM_CTLr_field_get(unit, &acc_ctrl,
                GENMEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto arl_read_exit;
        }

        /* Read the current generic memory entry */
        if ((index % 2) == 0) {
            /* Read bin 0 entry */
            if ((rv = REG_READ_GENMEM_DATA0r(unit, gmem_entry)) < 0) {
                goto arl_read_exit;
            }
            gmem_entry++;
            /* Read bin 1 entry */
            if (++i < count) {
                if ((rv = REG_READ_GENMEM_DATA1r(unit, gmem_entry)) < 0) {
                    goto arl_read_exit;
                }
                gmem_entry++;    
            }
        } else {
            if ((rv = REG_READ_GENMEM_DATA1r(unit, gmem_entry)) < 0) {
                goto arl_read_exit;
            }
            gmem_entry++;  
        }
        

    }

 arl_read_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;

}

int
_drv_harrier_arl_write(int unit, uint32 entry_id, 
                               uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0, mem_addr = 0;
    uint32  temp, index;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_write(entry_id=0x%x,count=%d)\n"),
              entry_id, count));
         
    mem_id = INDEX(L2_ARLm);

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    if (count < 1) {
        return SOC_E_PARAM;
    }
    if (((entry_id) < index_min) || 
        ((entry_id + count - 1) > index_max)) {
            return SOC_E_PARAM;
    }

    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));
    
    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* write data */
        index = entry_id + i;
        if ((index % 2) == 0) {
            if ((rv = REG_WRITE_GENMEM_DATA0r(unit, gmem_entry)) < 0) {
                goto arl_write_exit;
            }
            if (++i < count) {

                if ((rv = REG_WRITE_GENMEM_DATA1r(unit, ++gmem_entry)) < 0) {
                    goto arl_write_exit;
                }
            }
        } else {
            if ((rv = REG_WRITE_GENMEM_DATA1r(unit, gmem_entry)) < 0) {
                goto arl_write_exit;
            }
        }

        /* Set memory entry address */
        temp = index / 2;
        soc_GENMEM_ADDRr_field_set(unit, &mem_addr,
            GENMEM_ADDRf, &temp);
        if ((rv = REG_WRITE_GENMEM_ADDRr(unit, &mem_addr)) < 0) {
            goto arl_write_exit;
        }

        /* Read memory control register */
        if ((rv = REG_READ_GENMEM_CTLr(unit, &acc_ctrl)) < 0) {
            goto arl_write_exit;
        }
        temp = HARRIER_MEM_OP_WRITE;
        soc_GENMEM_CTLr_field_set(unit, &acc_ctrl,
            GENMEM_RWf, &temp);
        temp = HARRIER_ARL_MEMORY;
        soc_GENMEM_CTLr_field_set(unit, &acc_ctrl,
            TXDSC_ARLf, &temp);

        temp = 1;
        soc_GENMEM_CTLr_field_set(unit, &acc_ctrl,
            GENMEM_STDNf, &temp);
        if ((rv = REG_WRITE_GENMEM_CTLr(unit, &acc_ctrl)) < 0) {
            goto arl_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_GENMEM_CTLr(unit, &acc_ctrl)) < 0) {
                goto arl_write_exit;
            }
            soc_GENMEM_CTLr_field_get(unit, &acc_ctrl,
                GENMEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto arl_write_exit;
        }
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            if (((index % 2) == 0) && (i < count)) {
                sal_memcpy(cache + (index) * entry_size, 
                    --gmem_entry, entry_size * 2);
                CACHE_VMAP_SET(vmap, (index));
                CACHE_VMAP_SET(vmap, (index+1));
                gmem_entry++;
            } else {
                sal_memcpy(cache + (index) * entry_size, 
                    gmem_entry, entry_size);
                CACHE_VMAP_SET(vmap, (index));
            }
        }
        gmem_entry++;
    }

arl_write_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
    
}
 /*
 *  Function : drv_harrier_mem_read
 *
 *  Purpose :
 *      Get the width of selected memory. 
 *      The value returned in data is width in bits.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be read.
 *      count   :   one or more netries to be read.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the read result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_harrier_mem_read(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    int i;
    uint32 retry, index_min, index_max;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32 other_table_idx = 0;
    uint32  temp;

    /* Harrier uses 3 data register to read/write memories except 
     *  the ARL and CFP tables 
     *   - ARL table read are processed in other sub-routine.
     *   - CFP table read are not designed in this routine.
     *   - the gmem_entry access pointer is not referenced or used when the 
     *      target table is ARL or any one CFP related table.
     */
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint64 value64;
    uint32 *entry32;
    soc_control_t           *soc = SOC_CONTROL(unit);

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_read(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
             rv = _drv_harrier_arl_read(unit, entry_id, count, entry);
             return rv;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            break;
        case DRV_MEM_MCAST:
            mem_id = INDEX(MARL_PBMPm);
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = INDEX(VLAN2VLANm);
            break;
        case DRV_MEM_MACVLAN:
            mem_id = INDEX(MAC2VLANm);
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = INDEX(PROTOCOL2VLANm);
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = INDEX(FLOW2VLANm);
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            rv = _drv_harrier_cfp_read(
                unit, mem, entry_id, count, entry);
            return rv;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);

    /* process read action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    /* check count */
    if (count < 1) {
        rv = SOC_E_PARAM;
        goto mem_read_exit;
    }

    for (i = 0;i < count; i++ ) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* Return data from cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL && CACHE_VMAP_TST(vmap, (entry_id + i))) {
            sal_memcpy(gmem_entry, cache + (entry_id + i) * entry_size, entry_size);
            continue;
        }

        /* Clear memory data access registers */
        COMPILER_64_ZERO(value64);
        if ((rv = REG_WRITE_OTHER_TABLE_DATA0r(unit, 
            (uint32 *)&value64)) < 0) {
            goto mem_read_exit;
        }
        if ((rv = REG_WRITE_OTHER_TABLE_DATA1r(unit, 
            (uint32 *)&value64)) < 0) {
            goto mem_read_exit;
        }
        if ((rv = REG_WRITE_OTHER_TABLE_DATA2r(unit, 
            (uint32 *)&value64)) < 0) {
            goto mem_read_exit;
        }

        /* Set memory index */
        if ((rv = REG_READ_OTHER_TABLE_INDEXr(unit, &other_table_idx)) < 0) {
            goto mem_read_exit;
        }
        temp = entry_id + i;
        soc_OTHER_TABLE_INDEXr_field_set(unit, &other_table_idx,
            TABLE_INDEXf, &temp);
        if ((rv = REG_WRITE_OTHER_TABLE_INDEXr(unit, &other_table_idx)) < 0) {
            goto mem_read_exit;
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Read memory control register */
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &acc_ctrl)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }
        
        temp = MEM_TABLE_READ;
        soc_ARLA_RWCTLr_field_set(unit, &acc_ctrl, 
            TAB_RWf, &temp);

        /* Decide which table to be read */
        switch(mem) {
            case DRV_MEM_VLAN:
                temp = HARRIER_VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_MSTP:
                temp = HARRIER_MSPT_TABLE_ACCESS;
                break;
            case DRV_MEM_MCAST:
                temp = HARRIER_MARL_PBMP_TABLE_ACCESS;
                break;
            case DRV_MEM_VLANVLAN:
                temp = HARRIER_VLAN2VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_MACVLAN:
                temp = HARRIER_MAC2VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_PROTOCOLVLAN:
                temp = HARRIER_PROTOCOL2VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_FLOWVLAN:
                temp = HARRIER_FLOW2VLAN_TABLE_ACCESS;
                break;
            /*
             * COVERITY
             *
             * This default is unreachable. It is kept intentionally as a defensive 
             * default for future development. 
             */ 
            /* coverity[dead_error_begin] */
            default:
                rv = SOC_E_UNAVAIL;
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_read_exit;
                break;
        }
        soc_ARLA_RWCTLr_field_set(unit, &acc_ctrl, 
            TAB_INDEXf, &temp);

        /* Start Read Process */
        temp = 1;
        soc_ARLA_RWCTLr_field_set(unit, &acc_ctrl, 
            ARL_STRTDNf, &temp);
        if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &acc_ctrl)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &acc_ctrl)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_read_exit;
            }
            soc_ARLA_RWCTLr_field_get(unit, &acc_ctrl, 
                ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_read_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);

        /* Read the current generic memory entry */
        entry32 = (uint32 *)gmem_entry;
        switch(mem) {
            case DRV_MEM_VLANVLAN:
            case DRV_MEM_PROTOCOLVLAN:
            case DRV_MEM_FLOWVLAN:
                /* Multi-table memory entry_size = 4 bytes */
                if ((rv = REG_READ_OTHER_TABLE_DATA0r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[0] = COMPILER_64_LO(value64);
                break;
            case DRV_MEM_MACVLAN:
                /* Multi-table memory entry_size = 8 bytes */
                /* Need to return 8*2 = 16 bytes for MAC2VLAN with bin0 and bin1 */
                /* bin0 */
                if ((rv = REG_READ_OTHER_TABLE_DATA0r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[0] = COMPILER_64_LO(value64);
                entry32[1] = COMPILER_64_HI(value64);
                /* bin1 */
                if ((rv = REG_READ_OTHER_TABLE_DATA1r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[2] = COMPILER_64_LO(value64);
                entry32[3] = COMPILER_64_HI(value64);
                break;
            case DRV_MEM_VLAN:
            case DRV_MEM_MSTP:
            case DRV_MEM_MCAST:
                /* Multi-table memory entry_size = 24 bytes */
                if ((rv = REG_READ_OTHER_TABLE_DATA0r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[0] = COMPILER_64_LO(value64);
                entry32[1] = COMPILER_64_HI(value64);
                if ((rv = REG_READ_OTHER_TABLE_DATA1r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[2] = COMPILER_64_LO(value64);
                entry32[3] = COMPILER_64_HI(value64);
                if ((rv = REG_READ_OTHER_TABLE_DATA2r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_read_exit;
                }
                entry32[4] = COMPILER_64_LO(value64);
                entry32[5] = COMPILER_64_HI(value64);
                break;
            /*
             * COVERITY
             *
             * This default is unreachable. It is kept intentionally as a defensive 
             * default for future development. 
             */ 
            /* coverity[dead_error_begin] */
            default:
                rv = SOC_E_UNAVAIL;
                goto mem_read_exit;
                break;
        }
        gmem_entry++;
    }

 mem_read_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;

}

 /*
 *  Function : drv_harrier_mem_write
 *
 *  Purpose :
 *      Writes an internal memory.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   the memory type to access.
 *      entry_id    :  the entry's index of the memory to be written.
 *      count   :   one or more netries to be written.
 *      entry_data   :   pointer to a buffer of 32-bit words 
 *                              to contain the writting result.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int
drv_harrier_mem_write(int unit, uint32 mem, 
    uint32 entry_id, uint32 count, uint32 *entry)
{
    int rv = SOC_E_NONE;
    uint32 retry, index_min, index_max;
    uint32 i;
    uint32 mem_id;
    uint32 acc_ctrl = 0;
    uint32  temp;
    gen_memory_entry_t *gmem_entry = (gen_memory_entry_t *)entry;
    uint32 *cache;
    uint8 *vmap;
    int entry_size;
    uint32 other_table_idx = 0;
    uint64 value64;
    uint32 *entry32;
    soc_control_t           *soc = SOC_CONTROL(unit);

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_write(mem=0x%x,entry_id=0x%x,count=%d)\n"),
              mem, entry_id, count));
         
    switch (mem)
    {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MARL:
             rv = _drv_harrier_arl_write(unit, entry_id, count, entry);
             return rv;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            break;
        case DRV_MEM_MCAST:
            mem_id = INDEX(MARL_PBMPm);
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = INDEX(VLAN2VLANm);
            break;
        case DRV_MEM_MACVLAN:
            /* call MACVLAN specific routine */
            rv = _drv_harrier_mem_macvlan_write(unit, entry, 0);
            return rv;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = INDEX(PROTOCOL2VLANm);
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = INDEX(FLOW2VLANm);
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
        case DRV_MEM_CFP_STAT_IB:
        case DRV_MEM_CFP_STAT_OB:
            rv = _drv_harrier_cfp_write(
                unit, mem, entry_id, count, entry);
            return rv;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }


    /* add code here to check addr */
    index_min = soc_robo_mem_index_min(unit, mem_id);
    index_max = soc_robo_mem_index_max(unit, mem_id);
    entry_size = soc_mem_entry_bytes(unit, mem_id);
    
    /* process write action */
    MEM_LOCK(unit, INDEX(GEN_MEMORYm));

    if (count < 1) {
        rv = SOC_E_PARAM;
        goto mem_write_exit;
    }

    for (i = 0; i < count; i++) {
        if (((entry_id+i) < index_min) || ((entry_id+i) > index_max)) {
            MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
            return SOC_E_PARAM;
        }

        /* Clear memory data access registers */
        COMPILER_64_ZERO(value64);
        if ((rv = REG_WRITE_OTHER_TABLE_DATA0r(unit, 
            (uint32 *)&value64)) < 0) {
            goto mem_write_exit;
        }
        if ((rv = REG_WRITE_OTHER_TABLE_DATA1r(unit, 
            (uint32 *)&value64)) < 0) {
            goto mem_write_exit;
        }
        if ((rv = REG_WRITE_OTHER_TABLE_DATA2r(unit, 
            (uint32 *)&value64)) < 0) {
            goto mem_write_exit;
        }

        /* write data */
        entry32 = (uint32 *)gmem_entry;
        switch(mem) {
            case DRV_MEM_VLANVLAN:
            case DRV_MEM_PROTOCOLVLAN:
            case DRV_MEM_FLOWVLAN:
                /* Multi-table memory entry_size = 4 bytes */
                COMPILER_64_SET(value64, 0, entry32[0]);
                if ((rv = REG_WRITE_OTHER_TABLE_DATA0r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_write_exit;
                }
                break;
            case DRV_MEM_VLAN:
            case DRV_MEM_MSTP:
            case DRV_MEM_MCAST:
                /* Multi-table memory entry_size = 24 bytes */
                COMPILER_64_SET(value64, entry32[1], entry32[0]);
                if ((rv = REG_WRITE_OTHER_TABLE_DATA0r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_write_exit;
                }
                COMPILER_64_SET(value64, entry32[3], entry32[2]);
                if ((rv = REG_WRITE_OTHER_TABLE_DATA1r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_write_exit;
                }
                COMPILER_64_SET(value64, entry32[5], entry32[4]);
                if ((rv = REG_WRITE_OTHER_TABLE_DATA2r
                        (unit, (uint32 *)&value64)) < 0) {
                    goto mem_write_exit;
                }
                break;
            /*
             * COVERITY
             *
             * This default is unreachable. It is kept intentionally as a defensive 
             * default for future development. 
             */ 
            /* coverity[dead_error_begin] */
            default:
                rv = SOC_E_UNAVAIL;
                goto mem_write_exit;
                break;
        }

        /* Set memory index */
        if ((rv = REG_READ_OTHER_TABLE_INDEXr(unit, &other_table_idx)) < 0) {
            goto mem_write_exit;
        }
        temp = entry_id + i;
        soc_OTHER_TABLE_INDEXr_field_set(unit, &other_table_idx,
            TABLE_INDEXf, &temp);
        if ((rv = REG_WRITE_OTHER_TABLE_INDEXr(unit, &other_table_idx)) < 0) {
            goto mem_write_exit;
        }

        MEM_RWCTRL_REG_LOCK(soc);
        /* Read memory control register */
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &acc_ctrl)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }
        
        temp = MEM_TABLE_WRITE;
        soc_ARLA_RWCTLr_field_set(unit, &acc_ctrl, 
            TAB_RWf, &temp);

        /* Decide which table to be write */
        switch(mem) {
            case DRV_MEM_VLAN:
                temp = HARRIER_VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_MSTP:
                temp = HARRIER_MSPT_TABLE_ACCESS;
                break;
            case DRV_MEM_MCAST:
                temp = HARRIER_MARL_PBMP_TABLE_ACCESS;
                break;
            case DRV_MEM_VLANVLAN:
                temp = HARRIER_VLAN2VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_PROTOCOLVLAN:
                temp = HARRIER_PROTOCOL2VLAN_TABLE_ACCESS;
                break;
            case DRV_MEM_FLOWVLAN:
                temp =HARRIER_FLOW2VLAN_TABLE_ACCESS;
                break;
            /*
             * COVERITY
             *
             * This default is unreachable. It is kept intentionally as a defensive 
             * default for future development. 
             */ 
            /* coverity[dead_error_begin] */
            default:
                rv = SOC_E_UNAVAIL;
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_write_exit;
                break;
        }
        soc_ARLA_RWCTLr_field_set(unit, &acc_ctrl, 
            TAB_INDEXf, &temp);

        /* Start Read Process */
        temp = 1;
        soc_ARLA_RWCTLr_field_set(unit, &acc_ctrl, 
            ARL_STRTDNf, &temp);
        if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &acc_ctrl)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            if ((rv = REG_READ_ARLA_RWCTLr(unit, &acc_ctrl)) < 0) {
                MEM_RWCTRL_REG_UNLOCK(soc);
                goto mem_write_exit;
            }

            soc_ARLA_RWCTLr_field_get(unit, &acc_ctrl, 
                ARL_STRTDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_write_exit;
        }
        MEM_RWCTRL_REG_UNLOCK(soc);
        
        /* Write back to cache if active */
        cache = SOC_MEM_STATE(unit, mem_id).cache[0];
        vmap = SOC_MEM_STATE(unit, mem_id).vmap[0];

        if (cache != NULL) {
            sal_memcpy(cache + (entry_id + i) * entry_size, gmem_entry, entry_size);
            CACHE_VMAP_SET(vmap, (entry_id + i));
        }
        gmem_entry++;
    }

 mem_write_exit:
    MEM_UNLOCK(unit, INDEX(GEN_MEMORYm));
    return rv;
}

/*
 *  Function : drv_harrier_mem_field_get
 *
 *  Purpose :
 *      Extract the value of a field from a memory entry value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will return all the ports' state.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_harrier_mem_field_get(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32          mask = -1;
    uint32          mask_hi, mask_lo;
    int         mem_id = 0, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32              val32;
#endif

    switch (mem)
    {
        case DRV_MEM_ARL_HW:
            mem_id = INDEX(L2_ARLm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(RESERVED1_Rf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONTROLf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_ARL_SWm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                mem_id = INDEX(L2_MARL_SWm);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONTROLf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(UNTAG_MAPf);
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
                field_id = INDEX(FWD_MODEf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(fld_data, entry, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST:
            mem_id = INDEX(MARL_PBMPm);
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(PBMP_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = INDEX(VLAN2VLANm);
           if (field_index == DRV_MEM_FIELD_MAPPING_MODE) {
                field_id = INDEX(M_MODEf);
            }else if (field_index == DRV_MEM_FIELD_NEW_VLANID) {
                field_id = INDEX(NEW_VID_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
            mem_id = INDEX(MAC2VLANm);
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRI_Rf);
            }else if ((field_index == DRV_MEM_FIELD_NEW_VLANID)||
                        (field_index == DRV_MEM_FIELD_VLANID)) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = INDEX(PROTOCOL2VLANm);
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRI_Rf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ETHER_TYPE) {
                field_id = INDEX(ETHER_TYPEf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = INDEX(FLOW2VLANm);
            if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
            return _drv_harrier_cfp_field_get
                (unit, mem, field_index, entry, fld_data);
            break;
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(entry);
    assert(fld_data);

    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);
    bp = fieldinfo->bp;
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif

    wp = bp / 32;
    bp = bp & (32 - 1);
    len = fieldinfo->len;

    /* field is 1-bit wide */
    if (len == 1) {
        fld_data[0] = ((entry[FIX_MEM_ORDER_E(wp, meminfo)] >> bp) & 1);
    } else {

    if (fieldinfo->flags & SOCF_LE) {
        for (i = 0; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;
            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }
            /* get field value --- 32 bits each time */
            fld_data[i] = (entry[FIX_MEM_ORDER_E(wp++, meminfo)] 
                & mask_lo) >> bp;
            if (mask_hi) {
                fld_data[i] |= (entry[FIX_MEM_ORDER_E(wp, meminfo)] 
                    & mask_hi) << (32 - bp);
            }
            i++;
        }
    } else {
        i = (len - 1) / 32;

        while (len > 0) {
            assert(i >= 0);
            fld_data[i] = 0;
            do {
                fld_data[i] = (fld_data[i] << 1) |
                ((entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] >>
                (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));
            i--;
        }
    }
    }
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    if (field_id == INDEX(PORTID_Rf)) {
        *fld_data -= 24;
    }
    return SOC_E_NONE;
}

 /*
 *  Function : drv_harrier_mem_field_set
 *
 *  Purpose :
 *      Set the value of a field in a 8-, 16-, 32, and 64-bit memory value.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *      field_index    :  field type.
 *      entry   :   entry value pointer.
 *      fld_data   :   field value pointer.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1.For DRV_MEM_MSTP, because there is no port information 
 *          on the parameter list, so it will set the value on the 
 *          fld_data to memory entry.
 *      2. For DRV_MEM_ARL, the entry type will be l2_arl_sw_entry_t 
 *          and the mem_id is L2_ARL_SWm.
 */
int
drv_harrier_mem_field_set(int unit, uint32 mem, 
    uint32 field_index, uint32 *entry, uint32 *fld_data)
{
    soc_mem_info_t  *meminfo;
    soc_field_info_t    *fieldinfo;
    uint32                  mask, mask_hi, mask_lo;
    int         mem_id, field_id;
    int         i, wp, bp, len;
#ifdef BE_HOST
    uint32               val32;
#endif

    switch (mem)
    {
        case DRV_MEM_ARL_HW:
            mem_id = INDEX(L2_ARLm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(RESERVED1_Rf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONTROLf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            mem_id = INDEX(L2_ARL_SWm);
            if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else if (field_index == DRV_MEM_FIELD_SRC_PORT) {
                field_id = INDEX(PORTID_Rf);
            }else if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(MARL_PBMP_IDXf);
                mem_id = INDEX(L2_MARL_SWm);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRIORITY_Rf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_AGE) {
                field_id = INDEX(AGEf);
            }else if (field_index == DRV_MEM_FIELD_STATIC) {
                field_id = INDEX(STATICf);
            }else if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ARL_CONTROL) {
                field_id = INDEX(CONTROLf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLAN:
            mem_id = INDEX(VLAN_1Qm);
            if (field_index == DRV_MEM_FIELD_SPT_GROUP_ID) {
                field_id = INDEX(MSPT_IDf);
            }else if (field_index == DRV_MEM_FIELD_OUTPUT_UNTAG) {
                field_id = INDEX(UNTAG_MAPf);
            }else if (field_index == DRV_MEM_FIELD_PORT_BITMAP) {
                field_id = INDEX(FORWARD_MAPf);
            }else if (field_index == DRV_MEM_FIELD_FWD_MODE) {
                field_id = INDEX(FWD_MODEf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MSTP:
            mem_id = INDEX(MSPT_TABm);
            if (field_index == DRV_MEM_FIELD_MSTP_PORTST) {
                sal_memcpy(entry, fld_data, sizeof(mspt_tab_entry_t));
                return SOC_E_NONE;
            } else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MCAST:
            mem_id = INDEX(MARL_PBMPm);
            if (field_index == DRV_MEM_FIELD_DEST_BITMAP) {
                field_id = INDEX(PBMP_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_VLANVLAN:
            mem_id = INDEX(VLAN2VLANm);
            if (field_index == DRV_MEM_FIELD_MAPPING_MODE) {
                field_id = INDEX(M_MODEf);
            }else if (field_index == DRV_MEM_FIELD_NEW_VLANID) {
                field_id = INDEX(NEW_VID_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_MACVLAN:
            mem_id = INDEX(MAC2VLANm);
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRI_Rf);
            }else if ((field_index == DRV_MEM_FIELD_NEW_VLANID)||
                        (field_index == DRV_MEM_FIELD_VLANID)) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_MAC) {
                field_id = INDEX(MACADDRf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_PROTOCOLVLAN:
            mem_id = INDEX(PROTOCOL2VLANm);
            if (field_index == DRV_MEM_FIELD_VALID) {
                field_id = INDEX(VALID_Rf);
            }else if (field_index == DRV_MEM_FIELD_PRIORITY) {
                field_id = INDEX(PRI_Rf);
            }else if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else if (field_index == DRV_MEM_FIELD_ETHER_TYPE) {
                field_id = INDEX(ETHER_TYPEf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_FLOWVLAN:
            mem_id = INDEX(FLOW2VLANm);
            if (field_index == DRV_MEM_FIELD_VLANID) {
                field_id = INDEX(VID_Rf);
            }else {
                return SOC_E_PARAM;
            }
            break;
        case DRV_MEM_TCAM_DATA:
        case DRV_MEM_TCAM_MASK:
        case DRV_MEM_CFP_ACT:
        case DRV_MEM_CFP_METER:
            return _drv_harrier_cfp_field_set
                (unit, mem, field_index, entry, fld_data);
            break;
        case DRV_MEM_SECMAC:
        case DRV_MEM_GEN:
        default:
            return SOC_E_PARAM;
    }    
    
    assert(SOC_MEM_IS_VALID(unit, mem_id));
    meminfo = &SOC_MEM_INFO(unit, mem_id);

    assert(meminfo);
    assert(entry);
    assert(fld_data);
    SOC_FIND_FIELD(field_id,
         meminfo->fields,
         meminfo->nFields,
         fieldinfo);
    assert(fieldinfo);
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif

    /* Port number should add 24 for HARRIER */
    if (field_id == INDEX(PORTID_Rf)) {
        *fld_data += 24;
    }
    bp = fieldinfo->bp;
    if (fieldinfo->flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = fieldinfo->len; len > 0; len -= 32) {
            /* mask covers all bits in field. */
            /* if the field is wider than 32, takes 32 bits in each iteration */
            if (len >= 32) {
                mask = 0xffffffff;
            } else {
                mask = (1 << len) - 1;
            }
            /* the field may be splited across a 32-bit word boundary. */
            /* assume bp=0 to start with. Therefore, mask for higer word is 0 */
            mask_lo = mask;
            mask_hi = 0;

            /* if field is not aligned with 32-bit word boundary */
            /* adjust hi and lo masks accordingly. */
            if (bp) {
                mask_lo = mask << bp;
                mask_hi = mask >> (32 - bp);
            }

            /* set field value --- 32 bits each time */
            entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~mask_lo;
            entry[FIX_MEM_ORDER_E(wp++, meminfo)] |= 
                ((fld_data[i] << bp) & mask_lo);
            if (mask_hi) {
                entry[FIX_MEM_ORDER_E(wp, meminfo)] &= ~(mask_hi);
                entry[FIX_MEM_ORDER_E(wp, meminfo)] |= 
                    ((fld_data[i] >> (32 - bp)) & mask_hi);
            }

            i++;
        }
    } else {                   
        /* Big endian: swap bits */
        len = fieldinfo->len;

        while (len > 0) {
            len--;
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] &= ~(1 << (bp & (32-1)));
            entry[FIX_MEM_ORDER_E(bp / 32, meminfo)] |=
            (fld_data[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
#ifdef BE_HOST
    if ((fieldinfo->len > 32) && (fieldinfo->len <= 64)) {
        val32 = fld_data[0];
        fld_data[0] = fld_data[1];
        fld_data[1] = val32;
    }
#endif
    if (field_id == INDEX(PORTID_Rf)) {
        *fld_data -= 24;
    }
    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_mem_clear
 *
 *  Purpose :
 *      Clear whole memory entries.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory indication.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 *
 */
int 
drv_harrier_mem_clear(int unit, uint32 mem)
{
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_clear : mem=0x%x\n"), mem));
    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_ARL_HW:
        case DRV_MEM_MCAST:
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_FLOWVLAN:
            rv = _drv_harrier_mem_table_reset(unit, mem);
            return rv;
            break;
        case DRV_MEM_MARL:
            rv = DRV_MEM_DELETE(unit, DRV_MEM_MARL, NULL, DRV_MEM_OP_DELETE_ALL_ARL);
            return SOC_E_NONE;
            break;
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        default:
            return SOC_E_PARAM;
    }

    return rv;
}

/*
 *  Function : drv_harrier_mem_search
 *
 *  Purpose :
 *      Search selected memory for the key value
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      key   :   the pointer of the data to be search.
 *      entry     :   entry data pointer (if found).
 *      flags     :   search flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_harrier_mem_search(int unit, uint32 mem, 
    uint32 *key, uint32 *entry, uint32 *entry_1, uint32 flags)
{
    int                 rv = SOC_E_NONE;
    int                 index;

    switch(mem) {
        case DRV_MEM_ARL:
        case DRV_MEM_MARL:
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_MACVLAN:
        case DRV_MEM_PROTOCOLVLAN:
        case DRV_MEM_FLOWVLAN:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    rv = _drv_harrier_mem_search(unit, mem, key, 
        entry, entry_1, flags, &index);

    return rv;
}

#define _DRV_ST_OVERRIDE_NO_CHANGE  0
#define _DRV_ST_OVERRIDE_DYN2ST     1
#define _DRV_ST_OVERRIDE_ST2DYN     2
/*
 *  Function : drv_harrier_mem_insert
 *
 *  Purpose :
 *      Insert an entry to specific memory
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   insert flags (no use now).
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_harrier_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int                 rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t       output, output1;
    uint32          temp, count;
    uint64          entry_reg;
    uint8           mac_addr[6];
    uint64          mac_field, mac_field_output, mac_field_entry;
    uint32          vid, control, vid_output, vid_entry, st_output, st_entry;
    int             index;
    uint32          value;
    int             is_override = 0, is_dynamic = 0, is_ucast = 0;
    int             ori_st = 0, ori_port = -1, ori_ucast = 0;
    uint32          st_override_status = 0, src_port = 0 ;
    soc_control_t   *soc = SOC_CONTROL(unit);

    LOG_INFO(BSL_LS_SOC_MEM,
             (BSL_META_U(unit,
                         "drv_mem_insert : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    switch(mem) {
        case DRV_MEM_ARL:
            break;
        case DRV_MEM_MACVLAN:
            break;
        case DRV_MEM_PROTOCOLVLAN:
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_FLOWVLAN:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    /* section for inster the non-ARL table */
    if (mem != DRV_MEM_ARL){
        
        /* in Harrier, the non-ARL tables for intersting routine are 
         *  - designed on replace mode when entry conflict.
         *  - Full table insterion is not checked currently
         * 1. MAC2VLAN table 
         * 2. Protocol2VLAN table
         */
        if (entry == NULL){
            return SOC_E_MEMORY;
        }
        
        /* 1. MAC2VLAN table */
        if (mem == DRV_MEM_MACVLAN){
            mac2vlan_entry_t    m2v_output;
            uint32  field_val32;
            uint32     bin_id;
            
            sal_memset(&m2v_output, 0, sizeof(mac2vlan_entry_t));
            if ((rv = _drv_harrier_mem_macvlan_search(unit, 
                            entry, &m2v_output, &bin_id))< 0) {
                if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
                    return rv;
                }
            }

            if (((rv == SOC_E_EXISTS) && (flags == DRV_MEM_OP_REPLACE)) || 
                        ((rv == SOC_E_NOT_FOUND) && (bin_id != HARRIER_MACVLAN_UNAVAIL))){
            
                /* set valid bit */
                field_val32 = 1;
                SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_MACVLAN, DRV_MEM_FIELD_VALID,
                        entry, &field_val32));
                        
                /* MAC2VLAN write */
                rv = _drv_harrier_mem_macvlan_write(unit, entry, 0);
            }
            
        /* 2. PROTOCOL2VLAN */
        } else if (mem == DRV_MEM_PROTOCOLVLAN){
            protocol2vlan_entry_t   prot2v_output;
            uint32  ether_type, ent_id;
            uint32  field_val32;
            
            sal_memset(&prot2v_output, 0, sizeof(protocol2vlan_entry_t));
            if ((rv = _drv_harrier_mem_protocolvlan_search(unit, 
                            entry, &prot2v_output, (uint32 *) &index))< 0) {
                if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
                    return rv;
                }
            }

            if (((rv == SOC_E_EXISTS) && (flags == DRV_MEM_OP_REPLACE)) || 
                        (rv == SOC_E_NOT_FOUND)){
                /* get target ether_type */
                SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                        (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_ETHER_TYPE,
                        entry, &ether_type));
                
                ent_id = ether_type & HARRIER_MASK_PROTOCOL2VLAN_INDEX;

                /* set valid bit */
                field_val32 = 1;
                SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_PROTOCOLVLAN, DRV_MEM_FIELD_VALID,
                        entry, &field_val32));

                /* Protocol2VLAN write */
                rv = drv_harrier_mem_write(unit, DRV_MEM_PROTOCOLVLAN,
                        ent_id, 1, entry);
            }
        }

        return rv;
    }

    MEM_LOCK(unit, INDEX(L2_ARLm));
    /* search entry */
    sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
    is_override = FALSE;
    if ((rv =  _drv_harrier_mem_search
            (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
            (uint32 *)&output1, flags, &index))< 0) {
        if((rv!=SOC_E_NOT_FOUND)&&(rv!=SOC_E_EXISTS)) {
            goto mem_insert_exit;              
        }
    }
    /* Return SOC_E_NONE instead of SOC_E_EXISTS to fit DV test. */
    if (rv == SOC_E_EXISTS) {

        if (!sal_memcmp(&output, entry, 
                sizeof(l2_arl_sw_entry_t)) ){

            rv = SOC_E_NONE;
            goto mem_insert_exit;
        } else {
            /* MAC Address */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&mac_field_output)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    entry, (uint32 *)&mac_field_entry)) < 0) {
                goto mem_insert_exit;
            }
            if (!sal_memcmp(&mac_field_entry, &mac_field_output, 
                    sizeof(mac_field_output)) ){
                 /*  VLAN ID  */
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        (uint32 *)&output, &vid_output)) < 0) {
                    goto mem_insert_exit;
                }
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, 
                        entry, &vid_entry)) < 0) {
                    goto mem_insert_exit;
                }
                if (vid_output != vid_entry){
                    rv = SOC_E_NONE;
                    goto mem_insert_exit;
                }                
            } else {
               rv = SOC_E_NONE;
                goto mem_insert_exit;
            }

            is_override = TRUE;
            /* retrieve the original Uncast status */
            SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field_output);
            ori_ucast = (mac_addr[0] & 0x1) ? FALSE : TRUE;
            
            /* check static status change */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    (uint32 *)&output, &st_output)) < 0) {
                goto mem_insert_exit;
            }
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &st_entry)) < 0) {
                goto mem_insert_exit;
            }
            ori_st = st_output;
            if (st_output == st_entry){
                st_override_status = _DRV_ST_OVERRIDE_NO_CHANGE;
            } else {
                if (st_output){
                    st_override_status = _DRV_ST_OVERRIDE_ST2DYN;
                } else {
                    st_override_status = _DRV_ST_OVERRIDE_DYN2ST;
                }
            }

            /* retrieve source port */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                    (uint32 *)&output, (uint32 *)&ori_port)) < 0) {
                goto mem_insert_exit;
            }
            
        }
    }
    /* write entry */
    
    /* form entry */
    /* VLAN ID */
    if ((rv = DRV_MEM_FIELD_GET(unit, 
            DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &vid)) < 0) {
        goto mem_insert_exit;
    }

    /* MAC Address */
    if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
            entry, (uint32 *)&mac_field)) < 0) {
        goto mem_insert_exit;
    }
    SAL_MAC_ADDR_FROM_UINT64(mac_addr, mac_field);
    
    soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
        ARL_MACADDRf, (uint32 *)&mac_field);

    if (mac_addr[0] & 0x01) { /* The input is the mcast address */

        is_ucast = FALSE;
        
        /* multicast group index */
        rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &temp);
        if (rv != SOC_E_NONE){
            goto mem_insert_exit;
        }
        value = temp & 0x3f;
        
        /* Port number should subtract 24 for HARRIER */
        value -= 24;

        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_PIDf, &value);
        value = (temp >> 6) & 0x1f;
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_ENTRY_RSRV0f, &value);
        value = (temp >> 11) & 0x01;
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_AGEf, &value);
        /* arl_control */
        rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        if (rv != SOC_E_NONE){
            goto mem_insert_exit;
        }
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_CONf, &temp);
        /* static :
         *  - ROBO chip arl_control_mode at none-zero value cab't work 
         *    without static setting.
         */
        if (temp == 0 ) {
            rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
            if (rv != SOC_E_NONE){
                goto mem_insert_exit;
            }
        } else {
            temp = 1;
        }
        is_dynamic = (temp) ? FALSE : TRUE;
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_STATICf, &temp);

        /* valid bit */
        temp = 1;
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_VALIDf, &temp);
    } else { /* unicast address */

        is_ucast = TRUE;
        /* source port id */
        rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp);
        if (rv != SOC_E_NONE){
            goto mem_insert_exit;
        }
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
                ARL_PIDf, &temp);
        src_port = temp;

        /* age */
        rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_AGE, entry, &temp);
        if (rv != SOC_E_NONE){
            goto mem_insert_exit;
        }
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_AGEf, &temp);

        /* arl_control */
        rv = DRV_MEM_FIELD_GET(unit, 
                DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL, entry, &temp);
        if (rv != SOC_E_NONE){
            goto mem_insert_exit;
        }
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_CONf, &temp);

        /* static :
         *  - ROBO chip arl_control_mode at none-zero value can't work 
         *    without static setting.
         */
        if (temp == 0 ) {
            rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, entry, &temp);
            if (rv != SOC_E_NONE){
                goto mem_insert_exit;
            }
        } else {
            temp = 1;
        }
        is_dynamic = (temp) ? FALSE : TRUE;
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_STATICf, &temp);

        temp = 1;
        soc_ARLA_ENTRY_0r_field_set(unit, (uint32 *)&entry_reg,
            ARL_VALIDf, &temp);
    }


    /* write ARL and VID entry register*/
    if (index == 0){ /* entry 0 */
        if ((rv = REG_WRITE_ARLA_VID_ENTRY_0r(unit, &vid)) < 0) {
            goto mem_insert_exit;
        }
        if ((rv = REG_WRITE_ARLA_ENTRY_0r(unit, (uint32 *)&entry_reg)) < 0) {
            goto mem_insert_exit;
        }
    } else { /* entry 1 */
        if ((rv = REG_WRITE_ARLA_VID_ENTRY_1r(unit, &vid)) < 0) {
            goto mem_insert_exit;
        }
        if ((rv = REG_WRITE_ARLA_ENTRY_1r(unit, (uint32 *)&entry_reg)) < 0) {
            goto mem_insert_exit;
        }
    }

    MEM_RWCTRL_REG_LOCK(soc);
    /* Write ARL Read/Write Control Register */
    if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }
    temp = MEM_TABLE_WRITE;
    soc_ARLA_RWCTLr_field_set(unit, &control, 
        TAB_RWf, &temp);
    temp = HARRIER_ARL_TABLE_ACCESS;
    soc_ARLA_RWCTLr_field_set(unit, &control, 
        TAB_INDEXf, &temp);
    temp = 1;
    soc_ARLA_RWCTLr_field_set(unit, &control, 
        ARL_STRTDNf, &temp);

    if ((rv = REG_WRITE_ARLA_RWCTLr(unit, &control)) < 0) {
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        if ((rv = REG_READ_ARLA_RWCTLr(unit, &control)) < 0) {
            MEM_RWCTRL_REG_UNLOCK(soc);
            goto mem_insert_exit;
        }
        
        soc_ARLA_RWCTLr_field_get(unit, &control, 
            ARL_STRTDNf, &temp);
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        MEM_RWCTRL_REG_UNLOCK(soc);
        goto mem_insert_exit;
    }
    MEM_RWCTRL_REG_UNLOCK(soc);
    
    /* SA Learning Count handler :
     *  - increase one for the ARL insertion process
     *      (for Dynamic and Unicast entry only)
     *
     *  SW SA learn count handling process :
     *  ------------------------------------------
     *  If the L2 insert entry is new created on a empty entry space.
     *      1. Dynamic + Unicast 
     *          >> INCREASE one in SA_LRN_CNT
     *  else ( L2 entry insert to a existed entry)
     *      1. No port movement
     *          a. If both of original and this entries are uicast
     *              1). ST changed from Dynamic to Static
     *                  >> DECREASE one in SA_LRN_CNT
     *              2). ST changed from Static to Dynamic
     *                  >> INCREASE one in SA_LRN_CNT
     *          b. else if original entry is MCast and this entry is unicast
     *              1). This entry is Dynamic
     *                  >> INCREASE one in SA_LRN_CNT
     *          c. else if original entry is Unicast and this entry is MCast
     *              1). Original entry is Dynamic
     *                  >> DECREASE one in original port's SA_LRN_CNT
     *      2. Port movement
     *          a. Original entry is dynamic and unicast
     *              >> DECREASE one in original port's SA_LRN_CNT
     *          b. This entry is dynamic and unicast
     *              >> INCREASE entry in SA_LRN_CNT
     *
     */
    temp = 0;
    if (is_override == FALSE){
        if (is_dynamic && is_ucast){
            temp = DRV_PORT_SA_LRN_CNT_INCREASE;
        }
    } else {
        if (ori_port == src_port){
            if ((is_ucast == TRUE) && (ori_ucast == TRUE)){
                if (st_override_status == _DRV_ST_OVERRIDE_ST2DYN){
                    temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                } else if (st_override_status == _DRV_ST_OVERRIDE_DYN2ST){
                    temp = DRV_PORT_SA_LRN_CNT_DECREASE;
                }
            } else if ((is_ucast == TRUE) && (ori_ucast == FALSE)){
                if (is_dynamic){
                    temp = DRV_PORT_SA_LRN_CNT_INCREASE;
                }
            } else if ((is_ucast == FALSE) && (ori_ucast == TRUE)){
                if (ori_st == FALSE){
                    temp = DRV_PORT_SA_LRN_CNT_DECREASE;
                }
            } else {
                /* both not ucast >> No SA learn count handling action!! */
            }
        } else {
            /* station removment(port changed)! */
            if (is_ucast && is_dynamic){
                temp = DRV_PORT_SA_LRN_CNT_INCREASE;
            }

            if (ori_st == FALSE && ori_ucast){
                /* decrease original port's SA learn count  */
                rv = DRV_ARL_LEARN_COUNT_SET(unit, 
                        ori_port, DRV_PORT_SA_LRN_CNT_DECREASE, 0);
                if (SOC_FAILURE(rv)){
                    goto mem_insert_exit;
                }   
                LOG_INFO(BSL_LS_SOC_ARL,
                         (BSL_META_U(unit,
                                     "%s,port%d,SA_LRN_CNT decrease one!\n"),
                          FUNCTION_NAME(), ori_port));
            }
        }
    }
    /* performing SW SA learn count handling process on THIS PORT */
    if (temp){
        rv = DRV_ARL_LEARN_COUNT_SET(unit, src_port, temp, 0);
        if (SOC_FAILURE(rv)){
            goto mem_insert_exit;
        }   
        LOG_INFO(BSL_LS_SOC_ARL,
                 (BSL_META_U(unit,
                             "%s,port%d,SA_LRN_CNT %s one!\n"),
                  FUNCTION_NAME(), src_port, 
                  (temp == DRV_PORT_SA_LRN_CNT_INCREASE) ? 
                  "increase" : "decrease"));
    }

    sw_arl_update = 1;
    
mem_insert_exit:
    MEM_UNLOCK(unit,INDEX(L2_ARLm));

    if (sw_arl_update){
        /* Add the entry to sw database */
        _drv_arl_database_insert(unit, index, entry);
    }

    return rv;
}

/*
 *  Function : drv_harrier_mem_delete
 *
 *  Purpose :
 *      Remove an entry to specific memory or remove entries by flags 
 *
 *  Parameters :
 *      unit        :   unit id
 *      mem   :   memory entry type.
 *      entry     :   entry data pointer.
 *      flags     :   delete flags.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      Only for ARL memory now.
 *
 */
int 
drv_harrier_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags)
{
    int rv = SOC_E_NONE, sw_arl_update = 0;
    l2_arl_sw_entry_t output, output1;
    uint32 temp = 0, count;
    int index;
    uint32  reg_value;
    uint32 ag_port_mode = 0, ag_vlan_mode = 0;
    uint32  ag_static_mode = 0;
    uint32  mst_con, age_out_ctl;
    uint64 temp_mac_field;
    uint8  temp_mac_addr[6];
    uint32 temp_vid;
    uint64 entry_reg;
    uint32      src_port= 0, vlanid = 0;
    int         is_ucast = 0, is_dynamic = 0;
    
    LOG_INFO(BSL_LS_SOC_ARL,
             (BSL_META_U(unit,
                         "drv_mem_delete : mem=0x%x, flags = 0x%x)\n"),
              mem, flags));
    switch(mem) {
        case DRV_MEM_ARL:
            break;
        case DRV_MEM_MARL:
            /* MARL entries should do normal deletion, not fast aging. */
            flags &= ~DRV_MEM_OP_DELETE_BY_PORT;
            flags &= ~DRV_MEM_OP_DELETE_BY_VLANID;
            flags &= ~DRV_MEM_OP_DELETE_BY_SPT;
            flags &= ~DRV_MEM_OP_DELETE_BY_STATIC;
            break;
        case DRV_MEM_MACVLAN:

            rv = _drv_harrier_mem_macvlan_delete(unit, entry, 0);
            return rv;
            break;
        case DRV_MEM_PROTOCOLVLAN:
            
            rv = _drv_harrier_mem_protococlvlan_delete(unit, entry, 0);
            return rv;
            break;
        case DRV_MEM_VLAN:
        case DRV_MEM_MSTP:
        case DRV_MEM_MCAST:
        case DRV_MEM_GEN:
        case DRV_MEM_SECMAC:
        case DRV_MEM_VLANVLAN:
        case DRV_MEM_FLOWVLAN:
            return SOC_E_UNAVAIL;
        default:
            return SOC_E_PARAM;
    }

    if (flags & DRV_MEM_OP_DELETE_BY_PORT) {
        ag_port_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_VLANID) {
        ag_vlan_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
        ag_static_mode = 1;
    }
    if (flags & DRV_MEM_OP_DELETE_ALL_ARL) {
        if (mem == DRV_MEM_MARL) {
            rv = _drv_harrier_mem_marl_delete_all(unit);
            
            return rv;
        } else {
            rv = _drv_harrier_mem_table_reset(unit, mem);
            if (rv == SOC_E_NONE) {
                /* Remove entries from software table by port/vlan */
                _drv_arl_database_delete_by_fastage(unit, src_port,
                        vlanid, flags);
            }
            return rv;
        }
    }
    
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* 
         * aging port and vlan mode 
         */
        if ((rv = REG_READ_AGEOUT_CTLr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }

        /*
         * keep original age port id and vid
         */
        age_out_ctl = reg_value;

        /* aging port mode */
        if (ag_port_mode) {
            temp = 0;
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, entry, &temp)) < 0){
                goto mem_delete_exit;
            }
            src_port = temp;

            /* Port number should add 24 for HARRIER */
            temp = temp + 24;

            soc_AGEOUT_CTLr_field_set(unit, &reg_value, 
                AGE_EN_PORTf, &temp);
        }

        /* aging vlan mode */
        if (ag_vlan_mode) {
            temp = 0;
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp)) < 0){
                goto mem_delete_exit;
            }
            vlanid = temp;

            soc_AGEOUT_CTLr_field_set(unit, &reg_value, 
                AGE_EN_VIDf, &temp);
        } 

        if ((rv = REG_WRITE_AGEOUT_CTLr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }

        /* start fast aging process */
        if ((rv = REG_READ_MST_CONr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }
        mst_con = reg_value;
        soc_MST_CONr_field_set(unit, &reg_value,
            AGE_MODE_PRTf, &ag_port_mode);
        soc_MST_CONr_field_set(unit, &reg_value,
            AGE_MODE_VLANf, &ag_vlan_mode);
        soc_MST_CONr_field_set(unit, &reg_value,
            EN_AGE_STATICf, &ag_static_mode);
        temp = 1;
        soc_MST_CONr_field_set(unit, &reg_value,
            FAST_AGE_STDNf, &temp);
        if ((rv = REG_WRITE_MST_CONr(unit, &reg_value)) < 0) {
            goto mem_delete_exit;
        }
        /* wait for complete */
        for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
            if ((rv = REG_READ_MST_CONr(unit, &reg_value)) < 0) {
                goto mem_delete_exit;
            }
            soc_MST_CONr_field_get(unit, &reg_value,
                FAST_AGE_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (count >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto mem_delete_exit;
        }

        /* reset to init status while l2 thaw is proceeded. */
        soc_arl_frozen_sync_init(unit);

        /* Remove entries from software table by port/vlan */
        _drv_arl_database_delete_by_fastage(unit, src_port,
            vlanid, flags);
    } else { /* normal deletion */
        /* get static status for normal deletion */
        if (flags & DRV_MEM_OP_DELETE_BY_STATIC) {
            ag_static_mode = 1;
        } else {
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
                    (unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                    entry, &ag_static_mode));
        }

        MEM_LOCK(unit,INDEX(L2_ARLm));
        /* search entry */
        sal_memset(&output, 0, sizeof(l2_arl_sw_entry_t));
        if ((rv =  _drv_harrier_mem_search
                (unit, DRV_MEM_ARL, entry, (uint32 *)&output, 
                (uint32 *)&output1, flags, &index))< 0) {
            if((rv != SOC_E_NOT_FOUND) && (rv != SOC_E_EXISTS)) {
                if (rv == SOC_E_FULL) {
                /* For mem_delete if mem_search return SOC_E_FULL it means
                  * the entry is not found. */
                    rv = SOC_E_NOT_FOUND;
                }
                goto mem_delete_exit;              
            }
        }

        /* clear entry */
        if (rv == SOC_E_EXISTS) {
            if (index == 0) {
                /* entry 0 */
                if ((rv = REG_READ_ARLA_ENTRY_0r(
                    unit, (uint32 *)&entry_reg)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    soc_ARLA_ENTRY_0r_field_get(unit, (uint32 *)&entry_reg,
                        ARL_STATICf, &temp);
                    if (temp) {
                        LOG_INFO(BSL_LS_SOC_MEM,
                                 (BSL_META_U(unit,
                                             "\t Entry exist with static=%d\n"),
                                  temp));
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }
            } else {
                /* entry 1 */
                if ((rv = REG_READ_ARLA_ENTRY_1r(
                    unit, (uint32 *)&entry_reg)) < 0) {
                    goto mem_delete_exit;
                }
                /* check static bit if set */
                if (!ag_static_mode){
                    soc_ARLA_ENTRY_1r_field_get(unit, (uint32 *)&entry_reg,
                        ARL_STATICf, &temp);
                    if (temp) {
                        LOG_INFO(BSL_LS_SOC_MEM,
                                 (BSL_META_U(unit,
                                             "\t Entry exist with static=%d\n"),
                                  temp));
                        rv = SOC_E_NOT_FOUND;
                        goto mem_delete_exit;
                    }
                }
            }

            if ((rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC, 
                    (uint32 *)&output, (uint32 *)&temp_mac_field)) < 0) {
                goto mem_delete_exit;
            }

            SAL_MAC_ADDR_FROM_UINT64(temp_mac_addr, temp_mac_field);
            is_ucast = (temp_mac_addr[0] & 0x1) ? FALSE : TRUE;

            /* retrieve the static staus */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_STATIC,
                    (uint32 *)&output, &temp)) < 0) {
                goto mem_delete_exit;
            }
            is_dynamic = (temp) ? FALSE : TRUE;
            
            /* retrieve the src_port */
            if (is_ucast){
                if ((rv = DRV_MEM_FIELD_GET(unit, 
                        DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                        (uint32 *)&output, &temp)) < 0) {
                    goto mem_delete_exit;
                }
                src_port = temp;
            }
            
            /* Write VID Table Index Register */
            if ((rv = DRV_MEM_FIELD_GET(unit, 
                    DRV_MEM_ARL, DRV_MEM_FIELD_VLANID, entry, &temp_vid)) < 0){
                goto mem_delete_exit;
            }

            rv = _drv_harrier_mem_arl_entry_delete(unit, 
                    temp_mac_addr, temp_vid, index);
            if (rv == SOC_E_NONE) {
                if (is_dynamic && is_ucast){
                    rv = DRV_ARL_LEARN_COUNT_SET(unit, src_port, 
                            DRV_PORT_SA_LRN_CNT_DECREASE, 0);
                    if (SOC_FAILURE(rv)){
                        goto mem_delete_exit;
                    }
                    LOG_INFO(BSL_LS_SOC_ARL,
                             (BSL_META_U(unit,
                                         "%s,port%d, SA_LRN_CNT decreased one!\n"),
                              FUNCTION_NAME(), temp));
                }

                sw_arl_update = 1;
            }
        }
    }

mem_delete_exit:
    /* 
     * Restore MST Settings
     */
    if ((ag_port_mode) || (ag_vlan_mode)) {
        /* MST configuration */
        REG_WRITE_MST_CONr(unit, &mst_con);

        /* Age Ports and VIDs */
        REG_WRITE_AGEOUT_CTLr(unit, &age_out_ctl);
    }else {
        MEM_UNLOCK(unit,INDEX(L2_ARLm));
    }

    if (sw_arl_update){
        /* Remove the entry from sw database */
        _drv_arl_database_delete(unit, index, &output);
    }

    return rv;
}

