/*
 * $Id: mcast.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/types.h>
#include <soc/mcast.h>
#include "robo_tbx.h"

/*
 *  Function : drv_tbx_mcast_bmp_get
 *
 *  Purpose :
 *      Get the multicast member ports from multicast entry
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer 
 *      bmp     :   group port member
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int  
drv_tbx_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp)
{
    int rv = SOC_E_NONE;
    uint32  entry_index = 0, fld_v32 = 0;
    marl_pbmp_entry_t  mcast_entry;

    assert(entry);
    assert(bmp);

    /* get the multicast id */
    rv = soc_L2_MARL_SWm_field_get(unit, entry, MGIDf, &entry_index);
    if (rv){
        LOG_INFO(BSL_LS_SOC_L2TABLE,
                 (BSL_META_U(unit,
                             "%s: faield on get the l2 table\n"), FUNCTION_NAME()));
        return rv;
    }
    
    /* get multicast group entry */
    rv = MEM_READ_MARL_PBMPm(unit, entry_index, (uint32 *)&mcast_entry);
    if (rv){
        LOG_INFO(BSL_LS_SOC_L2TABLE,
                 (BSL_META_U(unit,
                             "%s: faield on get the mcast_bmp table\n"), FUNCTION_NAME()));
        return rv;
    }
    rv = soc_MARL_PBMPm_field_get(unit, (uint32 *)&mcast_entry, PBMPf, &fld_v32);
    if (rv){
        LOG_CLI((BSL_META_U(unit,
                            "[DEBUG]%s,%d,FAILED!\n"), FUNCTION_NAME(),__LINE__));
        return rv;
    }
    SOC_PBMP_WORD_SET(*bmp, 0, fld_v32);

    LOG_INFO(BSL_LS_SOC_L2TABLE,
             (BSL_META_U(unit,
                         "%s: unit %d, bmp = 0x%x\n"), FUNCTION_NAME(), unit, 
              SOC_PBMP_WORD_GET(*bmp, 0)));
    return rv;
}

 /*
 *  Function : drv_tbx_mcast_bmp_set
 *
 *  Purpose :
 *      Set the multicast member ports from multicast entry
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer 
 *      bmp     :   group port member
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. index could be -1 in ESW to indicate none L2MC_id. This is used to 
 *      request SW to choose one free id to user.
 *
 */
int  
drv_tbx_mcast_bmp_set(int unit, uint32 *entry, 
        soc_pbmp_t bmp, uint32 flag)
{
    int rv = SOC_E_NONE, loop, mcindex = -1;
    uint32  index, entry_index = 0;
    marl_pbmp_entry_t  mcast_entry;
    uint32  fld_value = 0, request_autoid = 0;
    
    assert(entry);
    LOG_INFO(BSL_LS_SOC_L2TABLE,
             (BSL_META_U(unit,
                         "%s: unit %d, bmp = 0x%x flag %x\n"), FUNCTION_NAME(), unit, 
              SOC_PBMP_WORD_GET(bmp, 0), flag));
    
    rv = soc_L2_MARL_SWm_field_get(unit, entry, MGIDf, &index);
    SOC_IF_ERROR_RETURN(rv);

    /* check if user is requesting AUTO select ID or request force 
     *  ID at id=0. And clear the USER filed value after fld_value is 
     *  retrieved.
     */
    rv = soc_L2_MARL_SWm_field_get(unit, entry, USERf, &request_autoid);
    SOC_IF_ERROR_RETURN(rv);    
    fld_value = 0;
    rv = soc_L2_MARL_SWm_field_set(unit, entry, USERf, &fld_value);
    SOC_IF_ERROR_RETURN(rv);    
    
    if( flag == DRV_MCAST_INDEX_ADD) {
        
        /* index at dummy id means rquest a free index. */
        if ((index == _TB_MCAST_PBMP_DUMMY_ID) && 
                (request_autoid == _TB_MCAST_USER_FLD_AUTOID)){
            
            /* get a free index for creating */
            for (loop = DRV_TBX_MAX_MCREP_MCAST_GROUP_NUM; 
                    loop < L2MC_SIZE(unit); loop++) {
                /* avoid 0-255 L2MC_ID for normal free L2MC_ID requesting 
                 *  - 0-255 in TB may be reserved for the usage of Mcast 
                 *      replication solution.
                 */
                if (!L2MC_USED_ISSET(unit, loop)) {
                    mcindex = loop;
                    break;
                }
            }
    
            if (mcindex == -1){
                return SOC_E_FULL;
            }
            
        } else {
            /* L2MC_ID verification: 
             *  - 0-255 is valid for Mcast Replication group.
             *  - 256-4095 is valid for normal Mcast group.
             */
            if (index >= L2MC_SIZE(unit)){
                LOG_CLI((BSL_META_U(unit,
                                    "%s: invalid L2MC_ID(%d) is requested!\n"), 
                         FUNCTION_NAME(), index));
            }
            mcindex = index;
            
        }

        /* set mc-index field */
        fld_value = (uint32)mcindex;
        /* calculate entry index */
        rv = soc_L2_MARL_SWm_field_set(unit, entry, MGIDf, &fld_value);
        SOC_IF_ERROR_RETURN(rv);

        /* Insert this address into arl table. */
        SOC_IF_ERROR_RETURN(DRV_MEM_INSERT(unit, DRV_MEM_MARL, entry, 
                (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID
                | DRV_MEM_OP_REPLACE)));

        index = mcindex;
    }
    entry_index = index;

    /* get multicast group entry */
    rv = MEM_READ_MARL_PBMPm(unit, entry_index, (uint32 *)&mcast_entry);
    if (rv){
        LOG_INFO(BSL_LS_SOC_L2TABLE,
                 (BSL_META_U(unit,
                             "%s: faield on get the mcast_bmp table\n"), FUNCTION_NAME()));
        return rv;
    }
    
    fld_value = SOC_PBMP_WORD_GET(bmp, 0);
    rv = soc_MARL_PBMPm_field_set(unit, (uint32 *)&mcast_entry, 
        PBMPf, &fld_value);
    if (rv){
        LOG_CLI((BSL_META_U(unit,
                            "[DEBUG]%s,%d,FAILED!\n"), FUNCTION_NAME(),__LINE__));
        return rv;
    }

    rv = MEM_WRITE_MARL_PBMPm(unit, entry_index,(uint32 *)&mcast_entry);
    if (rv){
        LOG_INFO(BSL_LS_SOC_L2TABLE,
                 (BSL_META_U(unit,
                             "%s: faield on get the mcast_bmp table\n"), FUNCTION_NAME()));
        return rv;
    }
    
    if (flag == DRV_MCAST_INDEX_ADD) {
        /* set MC used info-base */
        L2MC_USED_SET(unit, index);
        
    }else if (flag == DRV_MCAST_INDEX_REMOVE) {
        /* clear MC used info-base */
        L2MC_USED_CLR(unit, index);
    }

    return rv;
}
