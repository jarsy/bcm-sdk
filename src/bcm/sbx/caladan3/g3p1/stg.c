/*
 * $Id: stg.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stg.c
 * Purpose:     Spanning tree group support implementation for g3p1
 *
 * Multiple spanning trees (MST) is supported on this chipset
 */
#ifdef BCM_CALADAN3_G3P1_SUPPORT

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_int.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stg.h>
#include <bcm_int/sbx/caladan3/vlan.h>


uint32
_bcm_caladan3_g3p1_stg_stp_translate(int unit, bcm_stg_stp_t bcm_state)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    if (bcm_state == BCM_STG_STP_FORWARD
        || bcm_state == BCM_STG_STP_DISABLE) {
        return sbx->stpforward;
    } else if (bcm_state == BCM_STG_STP_LEARN) {
        return sbx->stplearn;
    }

    return sbx->stpblock;
}

bcm_stg_stp_t
_bcm_caladan3_g3p1_stg_stp_translate_to_bcm(int unit, int stpstate)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    if (stpstate == sbx->stpforward) {
        return BCM_STG_STP_FORWARD;
    } else if (stpstate == sbx->stplearn) {
        return BCM_STG_STP_LEARN;
    }

    return BCM_STG_STP_BLOCK;
}

int
_bcm_caladan3_g3p1_stg_vid_stp_set(int unit, bcm_vlan_t vid, bcm_port_t port,
                                 int stp_state)
{
    soc_sbx_g3p1_pv2e_t   sbx_pvid;
    soc_sbx_g3p1_epv2e_t  sbx_egr_pvid;

    BCM_IF_ERROR_RETURN(SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &sbx_pvid));


    sbx_pvid.stpstate = _bcm_caladan3_g3p1_stg_stp_translate(unit, stp_state);

    BCM_IF_ERROR_RETURN(_soc_sbx_g3p1_pv2e_set(unit, port, vid, &sbx_pvid));

    BCM_IF_ERROR_RETURN(SOC_SBX_G3P1_EPV2E_GET(unit, port, vid,
                                               &sbx_egr_pvid));

    sbx_egr_pvid.drop = ((stp_state == BCM_STG_STP_FORWARD ||
                          stp_state == BCM_STG_STP_DISABLE) ? 0 : 1);
    BCM_IF_ERROR_RETURN(SOC_SBX_G3P1_EPV2E_SET(unit, port, vid,
                                               &sbx_egr_pvid));

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_stg_vid_stp_get(int unit,
                                 bcm_vlan_t vid,
                                 bcm_port_t port,
                                 int *stp_state)
{
    soc_sbx_g3p1_pv2e_t   sbx_pvid;

    soc_sbx_g3p1_pv2e_t_init(&sbx_pvid);
    BCM_IF_ERROR_RETURN(SOC_SBX_G3P1_PV2E_GET(unit, port, vid, &sbx_pvid));

    *stp_state = _bcm_caladan3_g3p1_stg_stp_translate_to_bcm(unit,
                                                           sbx_pvid.stpstate);

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_stg_stacked_vid_stp_set_get(int unit,
                                         bcm_vlan_t ovid,
                                         bcm_vlan_t ivid,
                                         bcm_port_t port,
                                         int *stp_state,
                                         int set_or_get)
{
    /* This depends on pvv2e */
    soc_sbx_g3p1_epv2e_t  sbx_egr_pvid;
    soc_sbx_g3p1_pvv2edata_t  sbx_pvv2e;

    soc_sbx_g3p1_pvv2edata_t_init(&sbx_pvv2e);

    BCM_IF_ERROR_RETURN
       (soc_sbx_g3p1_util_pvv2e_get(unit, port, ovid, ivid, &sbx_pvv2e));

    if (set_or_get) { /* SET */
        sbx_pvv2e.stpstate =
            _bcm_caladan3_g3p1_stg_stp_translate(unit, *stp_state);

        BCM_IF_ERROR_RETURN
            (soc_sbx_g3p1_util_pvv2e_set(unit, port, ovid, ivid, &sbx_pvv2e));

        soc_sbx_g3p1_epv2e_t_init(&sbx_egr_pvid);
        BCM_IF_ERROR_RETURN
            (SOC_SBX_G3P1_EPV2E_GET(unit, port, ovid, &sbx_egr_pvid));

        sbx_egr_pvid.drop = ((*stp_state == BCM_STG_STP_FORWARD ||
                              *stp_state == BCM_STG_STP_DISABLE) ? 0 : 1);
        BCM_IF_ERROR_RETURN
            (SOC_SBX_G3P1_EPV2E_SET(unit, port, ovid, &sbx_egr_pvid));
    } else { /* GET */
        *stp_state =
            _bcm_caladan3_g3p1_stg_stp_translate_to_bcm(unit,
                                                      sbx_pvv2e.stpstate);
    }

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_stg_label_stp_set_get(int unit,
                                       bcm_mpls_label_t label,
                                       int *stp_state,
                                       int set_or_get)
{
#if 0
    /* This depends on l2e */
    soc_sbx_g2p3_l2e_t sbx_l2e;

    soc_sbx_g2p3_l2e_t_init(&sbx_l2e);

    BCM_IF_ERROR_RETURN
       (soc_sbx_g2p3_l2e_get(unit, label, &sbx_l2e));

    if (set_or_get) { /* SET */
        sbx_l2e.stpstate = _bcm_caladan3_g3p1_stg_stp_translate(unit,
                                                              *stp_state);

        BCM_IF_ERROR_RETURN
           (soc_sbx_g2p3_l2e_set(unit, label, &sbx_l2e));
    } else { /* GET */
        *stp_state = _bcm_caladan3_g3p1_stg_stp_translate_to_bcm(unit,
                                                             sbx_l2e.stpstate);
    }
#endif

    return BCM_E_NONE;
}

int
_bcm_caladan3_g3p1_stg_init(int unit)
{
    uint32 exc_idx;
    soc_sbx_g3p1_xt_t     sbx_xt;    /* exception table */
    int                   rv = BCM_E_NONE;

    /* enable support for learning on STP_BLOCKED exception */
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_exc_stp_blocked_idx_get(unit, &exc_idx));

    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_xt_get(unit, exc_idx, &sbx_xt));
    sbx_xt.learn = 1;
    BCM_IF_ERROR_RETURN(soc_sbx_g3p1_xt_set(unit, exc_idx, &sbx_xt));

    return rv;
}


int
_bcm_caladan3_g3p1_stp_fast_set(int unit, bcm_port_t port,
                              uint32 stpState,
                              int fastSets[BCM_VLAN_COUNT])
{
    int rv, setNative;
    bcm_vlan_t vid, minVid, maxVid, nativeVid;
    uint32 hwStpState, drop;
    uint32 *fastStates = NULL, *fastDrops = NULL;

    fastStates = sal_alloc(BCM_VLAN_COUNT * sizeof(uint32),
                           "fastStates temp");
    if (fastStates == NULL) {
        return BCM_E_MEMORY;
    }

    fastDrops = sal_alloc(BCM_VLAN_COUNT * sizeof(uint32),
                           "fastStates temp");
    if (fastDrops == NULL) {
        sal_free(fastStates);
        return BCM_E_MEMORY;
    }

    minVid = BCM_VLAN_MAX;
    maxVid = BCM_VLAN_MIN;
    setNative = 0;

    rv = bcm_port_untagged_vlan_get(unit, port, &nativeVid);
    if (BCM_E_NONE != rv) {
        goto error;
    }

    hwStpState = _bcm_caladan3_g3p1_stg_stp_translate(unit, stpState);
    drop = !(stpState == BCM_STG_STP_FORWARD
             || stpState == BCM_STG_STP_DISABLE);

    for (vid = BCM_VLAN_MIN;  vid < BCM_VLAN_COUNT; vid++) {
        if (fastSets[vid] == 0) {
            continue;
        }

        if (vid > maxVid) {
            maxVid = vid;
        }
        if (vid < minVid) {
            minVid = vid;
        }
        if (vid == nativeVid) {
            setNative = 1;
        }

        fastStates[vid] = hwStpState;
        fastDrops[vid] = drop;
    }

    if (maxVid < minVid) {
        rv = BCM_E_PARAM;
        goto error;
    }

    rv = soc_sbx_g3p1_pv2e_stpstate_fast_set(unit, minVid, port,
                                             maxVid, port,
                                             &fastSets[minVid],
                                             &fastStates[minVid],
                                             BCM_VLAN_COUNT);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "failed to fast set pv2e, p=0x%x,minvid=0x%x,maxvid=0x%x\n"),
                   port, minVid, maxVid));
        goto error;
    }

    rv = soc_sbx_g3p1_epv2e_drop_fast_set(unit, minVid, port,
                                          maxVid, port,
                                          &fastSets[minVid],
                                          &fastDrops[minVid],
                                          BCM_VLAN_COUNT);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_STG,
                  (BSL_META_U(unit,
                              "failed to fast set epv2e, p=0x%x,minvid=0x%x,maxvid=0x%x\n"),
                   port, minVid, maxVid));
        goto error;
    }

    if (setNative) {
        /* In G3P1 there is no need to refresh the native VID */
        /* The pv2e entry vid=0 is not used */
        rv = BCM_E_NONE;
    }

 error:
    if (fastStates) {
        sal_free(fastStates);
    }
    if (fastDrops) {
        sal_free(fastDrops);
    }

    return rv;
}


#endif /* BCM_CALADAN3_G3P1_SUPPORT */
