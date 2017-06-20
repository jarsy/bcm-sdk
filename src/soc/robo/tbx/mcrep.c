/*
 * $Id: mcrep.c,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/types.h>
#include <soc/robo/mcm/memregs.h>
#include <soc/debug.h>
#include <assert.h>
#include <soc/mem.h>
#include "robo_tbx.h"

/* Field name for Thunderbolt's Multicast Replication Tables */
static int _TB_VPVID_FIELD[]={
        INDEX(VPORT_VID_0f), INDEX(VPORT_VID_1f), INDEX(VPORT_VID_2f),
        INDEX(VPORT_VID_3f), INDEX(VPORT_VID_4f), INDEX(VPORT_VID_5f),
        INDEX(VPORT_VID_6f), INDEX(VPORT_VID_7f), INDEX(VPORT_VID_8f), 
        INDEX(VPORT_VID_9f), INDEX(VPORT_VID_10f), INDEX(VPORT_VID_11f),
        INDEX(VPORT_VID_12f), INDEX(VPORT_VID_13f), INDEX(VPORT_VID_14f), 
        INDEX(VPORT_VID_15f)
};

/*
 *  Function : drv_mcrep_vpgrp_vport_config_set
 *  Purpose :
 *      Set the multicast replication vport membership in a given group
 *  Parameters :
 *      unit        :   unit id
 *      mc_group    :   vPort group ID 
 *      port        :   port ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPGRP_OP_VPORT_MEMBER
 *              - DRV_MCREP_VPGRP_OP_VPGRP_RESET
 *      param       :   (In)parameter for OP.
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_tbx_mcrep_vpgrp_vport_config_set(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param)
{
    int         rv = SOC_E_NONE;
    uint32      vports = 0;
    int         mcrep_grp_id = -1;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "%s,%d,START! mcgrp=%x,port=%x,op=%d,param=%d\n"),
              FUNCTION_NAME(),__LINE__,mc_group,port,op,*param));

    /* valid section : assume the port id is verified already */
    assert(param);
    if (mc_group >= DRV_TBX_MAX_MCREP_MCAST_GROUP_NUM){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,MCast group_id is not vaild!\n"),FUNCTION_NAME()));
        return SOC_E_PARAM;
    }
    
    mcrep_grp_id = DRV_MCREP_GROUPID_GET(mc_group, port);
    
    switch(op){
        case DRV_MCREP_VPGRP_OP_VPORT_MEMBER :
            /* Operation :  DRV_MCREP_VPGRP_OP_VPORT_MEMBER 
             *  1. This OP is used to set the vport_bitmap in Mcast Rep. 
             *      table.
             *  2. The param for this OP is used to carry the vport_bmp.
             */
            vports = *((uint32 *)param) & DRV_MCREP_VPORTS_BMPMASK;
            break;
        case DRV_MCREP_VPGRP_OP_VPGRP_RESET :
            /* Operation :  DRV_MCREP_VPGRP_OP_VPGRP_RESET 
             *  1. This OP is used to reset the indicated Mcast Rep entry.
             *  2. The param is not used in this case.
             */
            vports = 0;
            break;
        default : 
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,invalid op %d!\n"), FUNCTION_NAME(), op));
            return SOC_E_PARAM;
    }
    rv = MEM_WRITE_MCAST_VPORT_MAPm(unit, (uint32)mcrep_grp_id, &vports);
    
    return rv;
}

/*
 *  Function : drv_mcrep_vpgrp_vport_config_get
 *  Purpose :
 *      Get the multicast replication vport membership in a given group
 *  Parameters :
 *      unit        :   unit id
 *      mc_group    :   vPort group ID 
 *      port        :   port ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPGRP_OP_VPORT_MEMBER
 *              - DRV_MCREP_VPGRP_OP_ENTRY_ID
 *      param       :   (Out)parameter for OP.
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_tbx_mcrep_vpgrp_vport_config_get(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param)
{
    int         rv = SOC_E_NONE;
    int         mcrep_grp_id = -1;
    uint32      vports = 0;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "%s,%d,START! mcgrp=%x,port=%x,op=%d,param=%x\n"),
              FUNCTION_NAME(),__LINE__,mc_group,port,op,*param));

    /* valid section : assume the port id is verified already */
    assert(param);
    if (mc_group >= DRV_TBX_MAX_MCREP_MCAST_GROUP_NUM){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,MCast group_id is not vaild!\n"),FUNCTION_NAME()));
        return SOC_E_PARAM;
    }
    
    mcrep_grp_id = DRV_MCREP_GROUPID_GET(mc_group, port);
    
    switch(op){
        case DRV_MCREP_VPGRP_OP_VPORT_MEMBER:
            /* Operation :  DRV_MCREP_VPGRP_OP_VPORT_MEMBER 
             *  1. This OP is used to get the vport bitmap on the Mcast Rep 
             *      table entry.
             */
            rv = MEM_READ_MCAST_VPORT_MAPm(unit, mcrep_grp_id, &vports);
            SOC_IF_ERROR_RETURN(rv);

            *param = vports & DRV_MCREP_VPORTS_BMPMASK;
            break;
        case DRV_MCREP_VPGRP_OP_ENTRY_ID:
            /* Operation :  DRV_MCREP_VPGRP_OP_ENTRY_ID 
             *  1. This OP is used to report the Mcast Replication table entry
             *      index from Mcast_ID and Port_ID.
             */
            *param = mcrep_grp_id;
            break;
        default :
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,invalid op %d!\n"), FUNCTION_NAME(), op));
            return SOC_E_PARAM;
    }

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "%s,%d,Done! param=%x\n"), 
              FUNCTION_NAME(),__LINE__, *param));
    return rv;
}

/*
 *  Function : drv_mcrep_vport_config_set
 *  Purpose :
 *      Set the vport related configuration.
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPORT_OP_VID
 *              - DRV_MCREP_VPORT_OP_UNTAG_VP
 *              - DRV_MCREP_VPORT_OP_UNTAG_RESET
 *      vport       :   vport_id
 *      vid         :   VID
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_tbx_mcrep_vport_config_set(int unit, uint32 port, 
        drv_mcrep_control_flag_t op, uint32 vport, uint32 vid)
{
    int     rv = SOC_E_NONE;
    uint32 fld_val32 = 0;
    vport_vid_map_entry_t *vpvid_map_entry = 0;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "%s,%d,START! port=%x,op=%d,vport%d,vid=%x\n"),
              FUNCTION_NAME(),__LINE__,port,op,vport,vid));

    /* valid section : assume the port id is verified already */
    if (vport >= DRV_TBX_MAX_MCREP_VPORT_NUM){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,invalid vPort ID %d!\n"), FUNCTION_NAME(), vport));
        return SOC_E_PARAM;
    }
    if (vid > DRV_MAX_VPORT_VID){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,invalid vPort VID %d!\n"), FUNCTION_NAME(), vid));
        return SOC_E_PARAM;
    }

    vpvid_map_entry = sal_alloc(sizeof (vport_vid_map_entry_t), "vpvid_map");
    sal_memset(vpvid_map_entry, 0, sizeof(vport_vid_map_entry_t));

    MEM_LOCK(unit, INDEX(VPORT_VID_MAPm));
    rv = MEM_READ_VPORT_VID_MAPm(unit, port, (uint32 *)vpvid_map_entry);
    if (rv) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,%d,mem_read failed!!\n"),
                  FUNCTION_NAME(), __LINE__));
        goto config_set_exit;
    }
    
    switch(op){
        case DRV_MCREP_VPORT_OP_VID:
            /* set vPort's VID */
            fld_val32 = vid;
            rv = DRV_MEM_FIELD_SET(unit, INDEX(VPORT_VID_MAPm),
                        _TB_VPVID_FIELD[vport],
                        (uint32 *)vpvid_map_entry, 
                        &fld_val32);
            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_set failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_set_exit;
            }
            break;

        case DRV_MCREP_VPORT_OP_VID_RESET:
            /* reset the VID from vPort0-vPort15 on this port :
             * keep untag vport bmp and reset all other field to 0 
             */
            rv = soc_VPORT_VID_MAPm_field_get(unit, (uint32 *)vpvid_map_entry, 
                VPORT_UNTAGf, &fld_val32);

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_get failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_set_exit;
            }

            sal_memset(vpvid_map_entry, 0, sizeof(vport_vid_map_entry_t));
            
            rv = soc_VPORT_VID_MAPm_field_set(unit, (uint32 *)vpvid_map_entry, 
                VPORT_UNTAGf, &fld_val32);

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_set failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_set_exit;
            }
            break;
        case DRV_MCREP_VPORT_OP_UNTAG_VP:
            /* set untag vPort :
             *  1. only one vPort is allowed in untag bitmap.
             *      - our design will override original untag vPort by user 
             *          requested vPort.
             *  2. vid in this OP is not used.
             */
            fld_val32 = 1 << vport;
            rv = soc_VPORT_VID_MAPm_field_set(unit, (uint32 *)vpvid_map_entry, 
                VPORT_UNTAGf, &fld_val32);

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_set failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_set_exit;
            }
            break;

        case DRV_MCREP_VPORT_OP_UNTAG_RESET:
            /* reset untag vPort bitmap on this port */
            fld_val32 = 0;
            rv = soc_VPORT_VID_MAPm_field_set(unit, (uint32 *)vpvid_map_entry, 
                VPORT_UNTAGf, &fld_val32);

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_set failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_set_exit;
            }
            break;
        case DRV_MCREP_VPORT_OP_VID_UNTAG_RESET:
            /* reset the vPort VID mapping table entry for this port */
            sal_memset(vpvid_map_entry, 0, sizeof(vport_vid_map_entry_t));
            break;
        default:
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,invalid op %d!\n"), FUNCTION_NAME(), op));
            rv = SOC_E_PARAM;
            goto config_set_exit;
    }
    
    rv = MEM_WRITE_VPORT_VID_MAPm(unit, port, (uint32 *)vpvid_map_entry);
    
config_set_exit :
    MEM_UNLOCK(unit, INDEX(VPORT_VID_MAPm));
    sal_free(vpvid_map_entry);
    return rv;
    

}

/*
 *  Function : drv_mcrep_vport_config_get
 *  Purpose :
 *      Get the vport related configuration.
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port ID
 *      op          :   Operation ID
 *              - DRV_MCREP_VPORT_OP_VID
 *              - DRV_MCREP_VPORT_OP_UNTAG_VP
 *      vport       :   (IN/OUT)vport_id
 *      vid         :   (OUT)VID
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int drv_tbx_mcrep_vport_config_get(int unit, uint32 port, 
        drv_mcrep_control_flag_t op, uint32 *vport, uint32 *vid)
{
    int     rv = SOC_E_NONE;
    int     i;
    uint32  fld_val32 = 0;
    vport_vid_map_entry_t *vpvid_map_entry = 0;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "%s,%d,START! port=%x,op=%d,vport%d,vid=%x\n"),
              FUNCTION_NAME(), __LINE__, port,op, *vport, *vid));

    /* valid section : assume the port id is verified already */
    assert(vport != NULL);
    assert(vid != NULL);
    if ((op == DRV_MCREP_VPORT_OP_VID) && 
            (*vport >= DRV_TBX_MAX_MCREP_VPORT_NUM)){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,invalid vPort ID %d!\n"), FUNCTION_NAME(), *vport));
        return SOC_E_PARAM;
    }

    vpvid_map_entry = sal_alloc(sizeof (vport_vid_map_entry_t), "vpvid_map");
    sal_memset(vpvid_map_entry, 0, sizeof(vport_vid_map_entry_t));

    rv = MEM_READ_VPORT_VID_MAPm(unit, port, (uint32 *)vpvid_map_entry);
    if (rv) {
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,%d,mem_read failed!!\n"),
                  FUNCTION_NAME(), __LINE__));
        goto config_get_exit;
    }
    
    switch(op){
        case DRV_MCREP_VPORT_OP_VID:
            /* get vPort's VID */
            rv = DRV_MEM_FIELD_GET(unit, INDEX(VPORT_VID_MAPm), 
                        _TB_VPVID_FIELD[*vport],
                        (uint32 *)vpvid_map_entry, 
                        &fld_val32);
            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_get failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_get_exit;
            }
            *vid = fld_val32;
            break;
            
        case DRV_MCREP_VPORT_OP_UNTAG_VP:
            /* get untag vPort :
             *  - for the untag vport is bitmap if the bitmap is 0 means there
             *      is no untag vport setting. And the vport return value must
             *      has a special value to reflect such condition.
             */
            rv = soc_VPORT_VID_MAPm_field_get(unit, (uint32 *)vpvid_map_entry,
                    VPORT_UNTAGf, &fld_val32);

            if (rv){
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s,%d,mem_field_set failed!!\n"),
                          FUNCTION_NAME(), __LINE__));
                goto config_get_exit;
            }
            if (fld_val32 == 0){
                /* vport=0 means vport0 is the untag vport. Here the pre 
                 *  defined symbol, "DRV_VPORT_NONE", is used to return the 
                 *  proper value on reporting no untag vport is assigned.
                 */
                *vport = DRV_TBX_VPORT_NONE;
            } else {
                for (i = 0; i < DRV_TBX_MAX_MCREP_VPORT_NUM; i++){
                    if (fld_val32 & (0x1 << i)){
                        break;
                    }
                }
                assert(i < DRV_TBX_MAX_MCREP_VPORT_NUM);
                *vport = i;
            }
            break;

        default:
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,invalid op %d!\n"), FUNCTION_NAME(), op));
            rv = SOC_E_PARAM;
            goto config_get_exit;
    }
    
config_get_exit :
    sal_free(vpvid_map_entry);
    return rv;
}

/*
 *  Function : drv_mcrep_vport_vid_search
 *  Purpose :
 *      Search the existed vport through a known VID.
 *  Parameters :
 *      unit        :   unit id
 *      port        :   port ID
 *      vport       :   (OUT)vport_id
 *      param       :   (In) Seached VID. (1-4095)
 *                      (OUT) Searched result
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      1. If there is two more vports have the same VID and the seach 
 *          request is targeted at this VID, the first seached one vport_id 
 *          will be reported as the vp_id and the result is FOUND.
 *          (include vid=0)
 */
int drv_tbx_mcrep_vport_vid_search(int unit, uint32 port, 
        uint32 *vport, int *param)
{
    int     rv = SOC_E_NONE;
    int     vid;
    int     i;
    uint32  fld_val32 = 0;
    vport_vid_map_entry_t *vpvid_map_entry = 0;

    LOG_INFO(BSL_LS_SOC_PORT,
             (BSL_META_U(unit,
                         "%s,%d,START! port=%x,param=%x\n"),
              FUNCTION_NAME(),__LINE__,port,*param));

    vid = *param;   /* get the search key */
    
    /* valid check : vid must be verified first (0-4095) */
    assert(param);
    if ((vid < 0) || (vid > DRV_MAX_VPORT_VID)){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,invalid vPort VID %d!\n"), FUNCTION_NAME(), vid));
        return SOC_E_PARAM;
    }
    
    vpvid_map_entry = sal_alloc(sizeof (vport_vid_map_entry_t), "vpvid_map");
    sal_memset(vpvid_map_entry, 0, sizeof(vport_vid_map_entry_t));

    rv = MEM_READ_VPORT_VID_MAPm(unit, port, (uint32 *)vpvid_map_entry);

    if (rv){
        LOG_WARN(BSL_LS_SOC_COMMON,
                 (BSL_META_U(unit,
                             "%s,%d,mem_read failed entry_id=1!!\n"),
                  FUNCTION_NAME(), __LINE__));
        goto search_exit;
    }
    
    /* Seach the VID for the first matched vport */
    for (i = 0; i < DRV_TBX_MAX_MCREP_VPORT_NUM; i++){
        rv = DRV_MEM_FIELD_GET(unit, INDEX(VPORT_VID_MAPm),
                    _TB_VPVID_FIELD[i],
                    (uint32 *)vpvid_map_entry, 
                    &fld_val32);
        if (rv){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,mem_field_get failed!!\n"),
                      FUNCTION_NAME(), __LINE__));
            goto search_exit;
        }
        if (fld_val32 == vid){   /* found the matched VID */
            break;
        }
    }
    
    if(i < DRV_TBX_MAX_MCREP_VPORT_NUM){    /* found */
        /* check if there is any conflict with untag vport */
        rv = soc_VPORT_VID_MAPm_field_get(unit, (uint32 *)vpvid_map_entry,
            VPORT_UNTAGf, &fld_val32);
        if (rv){
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s,%d,mem_field_get failed!!\n"),
                      FUNCTION_NAME(), __LINE__));
            goto search_exit;
        }
        
        *vport = i;
        *param = (fld_val32 & (0x1 << i)) ? 
                DRV_MCREP_VID_SEARCH_FOUND_BUT_UNTAG : 
                DRV_MCREP_VID_SEARCH_FOUND;
    } else {    /* not found */
        *vport = DRV_TBX_VPORT_NONE;
        *param = DRV_MCREP_VID_SEARCH_NOT_FOUND;
    }

search_exit :
    sal_free(vpvid_map_entry);
    return rv;
}

