/*
 * $Id: igmp.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * IGMP Snooping
 */

#include <soc/sbx/sbx_drv.h>


#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#endif

#include <bcm_int/sbx/error.h>
#include <bcm/vlan.h>

int
_bcm_caladan3_g3p1_igmp_snooping_init(int unit)
{
    bcm_vlan_data_t *listp = NULL;
    int              count = 0;
    int              rv = BCM_E_NONE;

#ifdef BCM_CALADAN3_G3P1_SUPPORT
    soc_sbx_g3p1_v2e_t v2e;
    soc_sbx_g3p1_xt_t  xt;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */

    /*
     * Turn on IGMP snooping on default VID
     */
    BCM_IF_ERROR_RETURN
       (bcm_vlan_list(unit, &listp, &count));

    if (count > 0) {
        int i;

        for (i = 0; i < count; i++) {
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
            case SOC_SBX_UCODE_TYPE_G3P1:
                soc_sbx_g3p1_v2e_t_init(&v2e);
                rv = soc_sbx_g3p1_v2e_get(unit, 
                                          SBX_VSI_FROM_VID(listp[i].vlan_tag),
                                          &v2e);
                if (rv == BCM_E_NONE) {
                    v2e.igmp = FALSE;
                    rv = soc_sbx_g3p1_v2e_set(unit, 
                                              SBX_VSI_FROM_VID(listp[i].vlan_tag),
                                              &v2e);
                }
                break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
            default:
                SBX_UNKNOWN_UCODE_WARN(unit);
                rv = BCM_E_INTERNAL;
                break;
            }
            
            if (rv != BCM_E_NONE) {
                BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy(unit, listp, count));
                return rv;
            }
        }
    }

    BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy(unit, listp, count));

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
    {
        uint32            exc_idx;
        
        BCM_IF_ERROR_RETURN(soc_sbx_g3p1_exc_igmp_idx_get(unit, &exc_idx));

        soc_sbx_g3p1_xt_t_init(&xt);
        rv = soc_sbx_g3p1_xt_get(unit, exc_idx, &xt);
        if (rv == BCM_E_NONE) {
            /*xt.qid = SBX_EXC_QID_BASE;*/
            xt.forward = TRUE;
            rv = soc_sbx_g3p1_xt_set(unit, exc_idx, &xt); 
        }
    }
    break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }
 
    return rv;
}

