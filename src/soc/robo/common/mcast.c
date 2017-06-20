/*
 * $Id: mcast.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/mcast.h>
#ifdef BCM_TB_SUPPORT
#include "../tbx/robo_tbx.h"
#endif

_soc_robo_mcast_t    robo_l2mc_info[SOC_MAX_NUM_DEVICES];
/*
 *  Function : drv_mcast_init
 *
 *  Purpose :
 *      Initialize multicast
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

int
drv_mcast_init(int unit)
{
    int rv = SOC_E_NONE;   

    if(SOC_IS_HARRIER(unit) || SOC_IS_TBX(unit)){
        /* clear the mcast table(mcast pbmap table) */
        SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_MCAST));
    }
    /* clear the mcast arl table */
    SOC_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_MARL));    
    return rv;
}


/*
 * Internal functions to convert between marl_entry_t and soc_mcast_addr_t
 */

int
drv_mcast_to_marl(int unit, uint32 *marl_entry,
			  uint32 *addr)
{
    uint32 fld_value = 0;
    uint64 mac_field;
    soc_mcast_addr_t *mcaddr = (soc_mcast_addr_t *)addr;

    COMPILER_64_ZERO(mac_field);

    sal_memset(marl_entry, 0, sizeof (l2_marl_sw_entry_t));

    SAL_MAC_ADDR_TO_UINT64(mcaddr->mac, mac_field);
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_MAC,
            (uint32 *)marl_entry, (void *)&mac_field));

    /* set VID field */
    fld_value = mcaddr->vid;
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_VLANID,
            (uint32 *)marl_entry, &fld_value));

    /* set static field - multicast entry should be static entry */
    /* exclude bcm5396, no such field */
    if (!SOC_IS_DINO16(unit)) {
        fld_value = 1;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_STATIC,
                (uint32 *)marl_entry, &fld_value));
    }

    /* multicast entry should be valid entry */
    fld_value = 1;
#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        fld_value = _TB_ARL_STATUS_VALID;
    }
#endif
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_VALID,
            (uint32 *)marl_entry, &fld_value));
            
    if (SOC_IS_GEX(unit)){
        fld_value = SOC_PBMP_WORD_GET(mcaddr->pbmp, 0);
    } else {
        /* set mc_index */
        if(mcaddr->l2mc_index == -1) {
            fld_value = 0;
#ifdef BCM_TB_SUPPORT
            if (SOC_IS_TBX(unit)){
                /* special process to indicate user's request on free ID
                 *  auto-select.
                 *  1. TB's mc_id=0 is special for mcast replication. this 
                 *      id will not be proper to indicate the ID auto-select 
                 *      process.
                 */
                fld_value = _TB_MCAST_USER_FLD_AUTOID;
                SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
                        unit, DRV_MEM_MARL, DRV_MEM_FIELD_USER,
                        (uint32 *)marl_entry, &fld_value));
                        
                /* assign the dummy id to indicate no id is assigned */
                fld_value = _TB_MCAST_PBMP_DUMMY_ID;
                
            }
#endif
        } else {
           fld_value = mcaddr->l2mc_index;
        }	   
    }
    
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP,
            (uint32 *)marl_entry, &fld_value));
    
    return SOC_E_NONE;
}


int
drv_mcast_from_marl(int unit, uint32 *addr,
			    uint32 *marl_entry)
{
    uint64 mac_field;
    vlan_1q_entry_t vEntry;
    uint32 fld_value[SOC_PBMP_WORD_MAX + 1] ;
    soc_mcast_addr_t *mcaddr = (soc_mcast_addr_t *)addr;
    soc_pbmp_t  t_pbmp;
    int i;

    sal_memset(mcaddr, 0, sizeof (*mcaddr));
    sal_memset(&t_pbmp, 0, sizeof (soc_pbmp_t));
    COMPILER_64_ZERO(mac_field);

    /* get MAC addr */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_MAC,
            (uint32 *)marl_entry, (void *)&mac_field));

    SAL_MAC_ADDR_FROM_UINT64(mcaddr->mac, mac_field);

    /* get VID field */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_MARL, DRV_MEM_FIELD_VLANID,
            (uint32 *)marl_entry, fld_value));
    mcaddr->vid = fld_value[0];

    /* Need to fill in the untagged port bitmap from the VLAN table */
    SOC_IF_ERROR_RETURN(DRV_MEM_READ(unit, DRV_MEM_VLAN, mcaddr->vid, 1, 
            (uint32 *)&vEntry));

    /* get pbmp field */
    SOC_IF_ERROR_RETURN(DRV_MCAST_BMP_GET(
            unit, (uint32 *)marl_entry, &t_pbmp));
    SOC_PBMP_ASSIGN(mcaddr->pbmp, t_pbmp);

    /* get ubmp field */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
            unit, DRV_MEM_VLAN, DRV_MEM_FIELD_OUTPUT_UNTAG, 
            (uint32 *)&vEntry, fld_value));

    SOC_PBMP_CLEAR(mcaddr->ubmp);
    for (i = 0; i < SOC_PBMP_WORD_MAX; i++){
        SOC_PBMP_WORD_SET(mcaddr->ubmp, 0, fld_value[i]);
    }
    
    if (!SOC_IS_GEX(unit)){
        /* get l2mc_index field */
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(
                unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP,
                (uint32 *)marl_entry, fld_value));
        mcaddr->l2mc_index = *fld_value;
    }
    return SOC_E_NONE;   
}
