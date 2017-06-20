/*
 * $Id: fabric.c,v 1.154 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Fabric Control API
 */

#include <shared/bsl.h>


#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>

#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/stack.h>
#include <soc/sbx/sbFabCommon.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sbx_util.h>
#include <soc/sbx/sirius.h>
#include <soc/mem.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/sirius.h>
#ifdef BCM_SIRIUS_SUPPORT
#include <soc/higig.h>
#endif

#define SBX_FABRIC_DEMAND_SCALE_ARRAY_SZ   32

static int bDModeDemandScaleAdjusted = FALSE;
STATIC bcm_sbx_fabric_state_t fabric_state[SOC_MAX_NUM_DEVICES];

static bcm_sbx_ds_state_t *ds_state_p[SOC_MAX_NUM_DEVICES];

static int32 nDModeDemandScale[SBX_FABRIC_DEMAND_SCALE_ARRAY_SZ] =
{
  0x6, /*  0 */
  0x6, /*  1 */
  0x6, /*  2 */
  0x6, /*  3 */
  0x6, /*  4 */
  0x5, /*  5 */
  0x5, /*  6 */
  0x5, /*  7 */
  0x5, /*  8 */
  0x4, /*  9 */
  0x4, /* 10 */
  0x4, /* 11 */
  0x4, /* 12 */
  0x4, /* 13 */
  0x4, /* 14 */
  0x4, /* 15 */
  0x4, /* 16 */
  0x4, /* 17 */
  0x4, /* 18 */
  0x4, /* 19 */
  0x4, /* 20 */
  0x4, /* 21 */
  0x4, /* 22 */
  0x4, /* 23 */
  0x4, /* 24 */
  0x0, /* 25 */
  0x0, /* 26 */
  0x0, /* 27 */
  0x0, /* 28 */
  0x0, /* 29 */
  0x0, /* 30 */
  0x0  /* 31 */
};

static int32 nNonDModeDemandScale[SBX_FABRIC_DEMAND_SCALE_ARRAY_SZ] =
{
  0x6, /*  0 */
  0x6, /*  1 */
  0x6, /*  2 */
  0x6, /*  3 */
  0x6, /*  4 */
  0x5, /*  5 */
  0x5, /*  6 */
  0x5, /*  7 */
  0x5, /*  8 */
  0x4, /*  9 */
  0x4, /* 10 */
  0x4, /* 11 */
  0x4, /* 12 */
  0x4, /* 13 */
  0x4, /* 14 */
  0x4, /* 15 */
  0x4, /* 16 */
  0x4, /* 17 */
  0x4, /* 18 */
  0x4, /* 19 */
  0x4, /* 20 */
  0x4, /* 21 */
  0x4, /* 22 */
  0x4, /* 23 */
  0x4, /* 24 */
  0x0, /* 25 */
  0x0, /* 26 */
  0x0, /* 27 */
  0x0, /* 28 */
  0x0, /* 29 */
  0x0, /* 30 */
  0x0  /* 31 */
};

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0

/* This function initializes the WB support for fabric module. 
 *      1. During cold-boot: allocates a stable cache
 *      2. During warm-boot: Recovers the fabric state from stable cache
 * 
 * bcm_sbx_fabric_state_t Compressed format 
 *      0-3Bytes:   ((TS << 16) | (scaled << 8) | (old_demand_scale))
 *      4-7Bytes:   xbar_state_length;
 *      7-XBytes:   xbar_state 
 * SS   X-(X+256):  congestion_size 
 * SS               (256 is SOC_HIGIG_HDR_MAX_MODID+1) 
 *     4B:          SOC_SBX_CFG(unit)->uMaxFailedLinks
 *     4B:          SOC_SBX_CFG(unit)->num_queues
 *     4B:          SOC_SBX_CFG(unit)->epoch_length_in_timeslots
 *     4B:          SOC_SBX_CFG(unit)->bTmeMode
 * SS  4B:          SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq
 * SS  4B:          SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos
 * SS  4B:          SOC_SBX_CFG_SIRIUS(unit)->b8kNodes
 * SS  4B:          SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize
 * 
 * NOTE: For sirius, we check both BCM_SIRIUS_SUPPORT and if the unit is a sirius.
 *  The latter check is to save size for non-sirius fabric devices.
 */
int
bcm_sbx_wb_fabric_state_init(int unit)
{
    int                     stable_size;
    uint8                   *scache_ptr = NULL;
    int                     rv;
    uint32                  scache_len, xbar_state_len;
    uint8                   *ptr;
    uint8                   *end_ptr;
    bcm_sbx_fabric_state_t  *p_fs;
#ifdef BCM_SIRIUS_SUPPORT
    uint32                  cs_len, index;
#endif
    soc_scache_handle_t     scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;
    uint32                  value = 0;
    
    rv = BCM_E_NONE;
#ifdef BCM_SIRIUS_SUPPORT
    cs_len = (SOC_HIGIG_HDR_MAX_MODID + 1);
#endif
    p_fs = SOC_SBX_STATE(unit)->fabric_state;
    if (p_fs == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid fabric state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }
    
    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }
    
    /* If device is during warm-boot, recover the state from scache */
    xbar_state_len = 0;
    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_FABRIC, 0);
    
    if (SOC_WARM_BOOT(unit)) {
        rv = soc_versioned_scache_ptr_get(unit, scache_handle,
                                          FALSE, &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                       FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
            return rv;
        }
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len; /* used for overrun checks*/
    
    /* Restore fabric redundancy handler pointer */
    __WB_DECOMPRESS_SCALAR(uint32, value);
    p_fs->red_f = (bcm_fabric_control_redundancy_handler_t) value;
    
    /* now de-compress to fabric state */
    __WB_DECOMPRESS_SCALAR(uint32, value);
    if (SOC_WARM_BOOT(unit)) {
        p_fs->timeslot_size = value >> 16;
        p_fs->scaled = (value >> 8) & 0xff;
        p_fs->old_demand_scale = value & 0xff;
    }
    
    if (!soc_feature(unit, soc_feature_standalone)) {
        __WB_DECOMPRESS_SCALAR(uint32, xbar_state_len);
        

        if (SOC_WARM_BOOT(unit) && 
            (xbar_state_len != (sizeof(bcm_sbx_xbar_state_t)))) {
            
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Internal error recovering fabric SCACHE memory \n"),
                       FUNCTION_NAME()));
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Invalid SCACHE state detected\n"),
                       FUNCTION_NAME()));
            return BCM_E_INTERNAL;
        }

        if (p_fs->xbar_state == NULL) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Memory for xbar_state is not allocated. \n"), 
                       FUNCTION_NAME()));
            return BCM_E_MEMORY;
        }

        if (SOC_WARM_BOOT(unit)) {
            __WB_CHECK_OVERRUN(xbar_state_len, "xbar_state");       
            sal_memcpy(p_fs->xbar_state, (uint8 *)ptr, sizeof(bcm_sbx_xbar_state_t));       
            ptr += sizeof(bcm_sbx_xbar_state_t);                
        } else {
            scache_len += sizeof(bcm_sbx_xbar_state_t);
        }
    }
        
#ifdef BCM_SIRIUS_SUPPORT
        
    if (SOC_IS_SIRIUS(unit)){
        if (soc_feature(unit, soc_feature_higig2)) {
            for(index = 0; index < cs_len; index++) 
                __WB_DECOMPRESS_SCALAR(uint8, p_fs->congestion_size[index]);
        }
    }
        
#endif
        
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uMaxFailedLinks);
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->bTmeMode);
    
    
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)){
        __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq);
        __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos);
        __WB_DECOMPRESS_SCALAR(uint32, value);
        if (SOC_WARM_BOOT(unit)) {
            SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = value >> 24;
            SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize = (value >> 16) & 0xFF;
        }
    }
#endif	

    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));
    
    if (!SOC_WARM_BOOT(unit)) {
        rv = soc_versioned_scache_ptr_get(unit, scache_handle,
                                          TRUE, &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) allocating WB scache handle on unit:%d \n"),
                       FUNCTION_NAME(), soc_errmsg(rv), unit));
            return rv;
        }
    }
    
    return rv;
}

/* This function compresses the info in bcm_sbx_fabric_state_t and stores it
 * to stable memory.
 * Input param: sync --> indicates whether to sync scache to Persistent memory
 */
int
bcm_sbx_wb_fabric_state_sync(int unit, int sync)
{
    uint8                   *scache_ptr = NULL;
    uint8                   *ptr, *end_ptr;
    int                     stable_size;
    int                     rv;
    uint32                  scache_len, xbar_state_len, req_len = 0;
    bcm_sbx_fabric_state_t  *p_fs;
    soc_scache_handle_t     scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;
    uint32                  value;
#ifdef BCM_SIRIUS_SUPPORT
    uint32                  index;
#endif /* BCM_SIRIUS_SUPPORT */
    
    if (SOC_WARM_BOOT(unit)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Cannot write to SCACHE during WarmBoot\n")));
        return SOC_E_INTERNAL;
    }
    
    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }
    
    p_fs = SOC_SBX_STATE(unit)->fabric_state;
    if (p_fs == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid fabric state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }
    
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_FABRIC, 0);
    
    scache_len = 0;
    rv = soc_versioned_scache_ptr_get(unit, scache_handle,
                                      FALSE, &scache_len, &scache_ptr,
                                      default_ver, &recovered_ver);
    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                   FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
        return rv;
    }
    
    ptr = scache_ptr;
    end_ptr = scache_ptr+scache_len;
    
    /* Save fabric redundancy handler pointer */
    value = (uint32) p_fs->red_f;
    __WB_COMPRESS_SCALAR(uint32, value);
    
    
    /* now compress and store in the scache location */
    value = ((p_fs->timeslot_size << 16) 
             | (p_fs->scaled << 8)
             | (p_fs->old_demand_scale));
    __WB_COMPRESS_SCALAR(uint32, value);
    
    if (!soc_feature(unit, soc_feature_standalone)) {
        xbar_state_len = sizeof(bcm_sbx_xbar_state_t);
        __WB_COMPRESS_SCALAR(uint32, xbar_state_len);
        
        req_len = (8 + sizeof(bcm_sbx_xbar_state_t));

        *(((uint32 *)ptr)) = xbar_state_len;  
        ptr += sizeof(uint32);
        
        if (p_fs->xbar_state) {
            sal_memcpy((uint8 *)ptr, p_fs->xbar_state, sizeof(bcm_sbx_xbar_state_t));
        } else {
            sal_memset((uint8 *)ptr, 0x0, sizeof(bcm_sbx_xbar_state_t));
        }
        ptr += (sizeof(bcm_sbx_xbar_state_t));
    }
    
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        req_len += (SOC_HIGIG_HDR_MAX_MODID + 1);
    }
#endif

#ifdef BCM_SIRIUS_SUPPORT
    if(SOC_IS_SIRIUS(unit)){
        if (soc_feature(unit, soc_feature_higig2)) {
            for(index = 0; index < (SOC_HIGIG_HDR_MAX_MODID + 1); index++) 
                __WB_COMPRESS_SCALAR(uint8, p_fs->congestion_size[index]);
        }
    }        
#endif
    
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uMaxFailedLinks);
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->bTmeMode);
    
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)){
        __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq);
        __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos);
        value = ((SOC_SBX_CFG_SIRIUS(unit)->b8kNodes << 24) |
                 (SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize << 16));
        __WB_COMPRESS_SCALAR(uint32, value);
    }
#endif    
    
    if (sync) {
        rv = soc_scache_commit(unit);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) sync'ing scache to Persistent memory. \n"),
                       FUNCTION_NAME(), soc_errmsg(rv)));
            return rv;
        }
    }
    
    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));
    
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
#if defined(BCM_WARM_BOOT_SUPPORT_SW_DUMP)
void 
bcm_sbx_wb_fabric_sw_dump(int unit)
{
    int                     xbar, node;
    bcm_sbx_fabric_state_t  *p_fs;
    int sfi_port;
#ifdef BCM_SIRIUS_SUPPORT
    int                      i;
#endif /*  BCM_SIRIUS_SUPPORT */

    p_fs = SOC_SBX_STATE(unit)->fabric_state;
    if (p_fs == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid fabric state (NULL) \n"), 
                   FUNCTION_NAME()));
        return;
    }

    LOG_CLI((BSL_META_U(unit,
                        "Timeslot:%d is_scaled:%d old_demand_scale:%d \n"),
             p_fs->timeslot_size, p_fs->scaled, p_fs->old_demand_scale));

    if (!soc_feature(unit, soc_feature_standalone)) {
        LOG_CLI((BSL_META_U(unit,
                            "XBAR_MAP\nxbars:  ")));
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; xbar++) {
            LOG_CLI((BSL_META_U(unit,
                                "%02d "), xbar));
        }
        
        
        for (node=0; node<SB_FAB_DEVICE_MAX_NODES; node++) {
            LOG_CLI((BSL_META_U(unit,
                                "\nnode%02d: "), node));
            for (xbar=0; xbar<SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; xbar++) {
                bcm_sbx_fabric_sfi_for_xbar_get(unit, xbar, node, &sfi_port);
                LOG_CLI((BSL_META_U(unit,
                                    "xbar%02d sfi_port%02d "), xbar, sfi_port));
            }
        }
    }

#ifdef BCM_SIRIUS_SUPPORT
    if(SOC_IS_SIRIUS(unit)){
        if (soc_feature(unit, soc_feature_higig2)) {
            LOG_CLI((BSL_META_U(unit,
                                "\nCongestion sizes in number of ports for each modid: ")));
            for (i=0; i<SOC_HIGIG_HDR_MAX_MODID; i++) {
                if (i%32 == 0) {
                    LOG_CLI((BSL_META_U(unit,
                                        "\nModid: %03d"), i));
                }
                LOG_CLI((BSL_META_U(unit,
                                    "%02d "), p_fs->congestion_size[i]));
            }
        }
    }
    LOG_CLI((BSL_META_U(unit,
                        "\nlchan_to_pchans: ")));
    for (i=0; i<SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS; i++) {
        LOG_CLI((BSL_META_U(unit,
                            "logical_channel{%2d] = physical_channel[%02d]\n"), i, p_fs->xbar_state->lchan_to_pchan[i]));
    }
    
    LOG_CLI((BSL_META_U(unit,
                        "\npchan_to_lchans: ")));
    for (i=0; i<SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS; i++) {
        LOG_CLI((BSL_META_U(unit,
                            "physical_channel{%2d] = logical_channel[%02d]\n"), i, p_fs->xbar_state->pchan_to_lchan[i]));
    }
#endif
    
    LOG_CLI((BSL_META_U(unit,
                        "\nMaxFailedLinks:%d  NumQueues:%d "), 
             SOC_SBX_CFG(unit)->uMaxFailedLinks, SOC_SBX_CFG(unit)->num_queues));
    LOG_CLI((BSL_META_U(unit,
                        "\nEpochLength:%d  TmeMode:%d "), 
             SOC_SBX_CFG(unit)->epoch_length_in_timeslots, 
             SOC_SBX_CFG(unit)->bTmeMode));
    
#ifdef BCM_SIRIUS_SUPPORT
    LOG_CLI((BSL_META_U(unit,
                        "\nMaxVoq:%d  SubscriberMaxCos:%d "), 
             SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq, 
             SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos));
    LOG_CLI((BSL_META_U(unit,
                        "\n8K-Nodes:%d  SubscriberNodeOptimize:%d "), 
             SOC_SBX_CFG_SIRIUS(unit)->b8kNodes, 
             SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize));
#endif
    
    LOG_CLI((BSL_META_U(unit,
                        "\n")));

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
int
bcm_sbx_fabric_demand_scale_init(int unit)
{
    int rc = BCM_E_NONE;
    int epoch_size;
    int max_epoch_size;
    int i, demand_shift_scale;


    /* this curently updates configuration tables for QoS optimizations. This */
    /* optimization is currently only supported for DMODE (BM3200/QE2000) and */
    /* FIC configuration only.                                                    */
    if (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE) {
        return(rc);
    }
    if ( (SOC_SBX_CFG(unit)->bHybridMode != FALSE) || (SOC_SBX_CFG(unit)->bTmeMode != FALSE) ) {
        return(rc);
    }

    /* update demand scaling to account for EPOCH size */
    epoch_size = SOC_SBX_CFG(unit)->epoch_length_in_timeslots;
    max_epoch_size = SB_FAB_DMODE_EPOCH_IN_TIMESLOTS;

    if (epoch_size >= max_epoch_size) {
        return(rc);
    }

    if (bDModeDemandScaleAdjusted == TRUE) {
        return(rc);
    }

    demand_shift_scale = 0;
    while (epoch_size < max_epoch_size) {
        epoch_size *= 2;
        demand_shift_scale++;
    }
    if (demand_shift_scale > 0) {
        demand_shift_scale--;
    }

    for (i = 0; i < SBX_FABRIC_DEMAND_SCALE_ARRAY_SZ; i++) {
        if (nDModeDemandScale[i] >= demand_shift_scale) {
            nDModeDemandScale[i] = nDModeDemandScale[i] - demand_shift_scale;
        }
        else {
            nDModeDemandScale[i] = 0;
        }
    }
    bDModeDemandScaleAdjusted = TRUE;

    return(rc);
}

int
bcm_sbx_fabric_detach(int unit)
{
    if (ds_state_p[unit] != NULL) {
        sal_free(ds_state_p[unit]->ds_grp_resource);
        sal_free(ds_state_p[unit]->mc_full_eval_min_state);
        sal_free(ds_state_p[unit]->ds_grp_membership);
        sal_free(ds_state_p[unit]);
        ds_state_p[unit] = NULL;
    }

    if (SOC_CONTROL(unit)) {
        if (SOC_SBX_CONTROL(unit)) {
            if (SOC_SBX_STATE(unit)->fabric_state) {
                if (SOC_SBX_STATE(unit)->fabric_state->xbar_state != NULL) {
                    sal_free(SOC_SBX_STATE(unit)->fabric_state->xbar_state);
                    SOC_SBX_STATE(unit)->fabric_state->xbar_state = NULL;
                }

                if (SOC_SBX_STATE(unit)->fabric_state->congestion_size != NULL) {
                    sal_free(SOC_SBX_STATE(unit)->fabric_state->congestion_size);
                    SOC_SBX_STATE(unit)->fabric_state->congestion_size = NULL;
                }

            }
        }
    }
    return BCM_E_NONE;
}

int
bcm_sbx_fabric_init(int unit)
{
    int rc = BCM_E_NONE;
    uint32  sz;
    bcm_sbx_ds_state_t *ds_p = NULL;
    bcm_fabric_distribution_t ds_id = 0;
    int *dist_modids_p = NULL;
    int node;
    int xbar;
    int sfi_port;
    bcm_sbx_xbar_state_t *xbar_state = NULL;
    int8 lchan, pchan;
#ifdef BCM_SIRIUS_SUPPORT
    uint8  *congestion_size_p;
#endif

    BCM_SBX_LOCK(unit);

    rc = bcm_sbx_fabric_detach(unit);
    if (rc != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rc;
    }
 
    SOC_SBX_STATE(unit)->fabric_state = &fabric_state[unit];
    sal_memset(SOC_SBX_STATE(unit)->fabric_state,
               0x00,
               sizeof(*(SOC_SBX_STATE(unit)->fabric_state)));
    SOC_SBX_STATE(unit)->fabric_state->scaled = FALSE;
    SOC_SBX_STATE(unit)->fabric_state->old_demand_scale = SOC_SBX_CFG(unit)->demand_scale;
    SOC_SBX_STATE(unit)->fabric_state->egress_aging = 0;



    if (!soc_feature(unit, soc_feature_standalone)) {
       
	    xbar_state = sal_alloc(sizeof(bcm_sbx_xbar_state_t), "xbar_state");
	    
	    if (xbar_state == NULL) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, sal_alloc xbar_state,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            rc = BCM_E_MEMORY;
            goto err;
            
	    }
        SOC_SBX_STATE(unit)->fabric_state->xbar_state = xbar_state;
        
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; xbar++) {
            
            /* Initialize xbar map to be 1 to 1 the sfi port crossbars */
            if (BCM_SBX_CROSSBAR_VALID(xbar)) {
                sfi_port = xbar;
            } else { 
                sfi_port = SB_FAB_DEVICE_INVALID_CROSSBAR;
            }
            
            for (node=0; node<SB_FAB_DEVICE_MAX_NODES; node++) {
                xbar_state->xbar_map[xbar][node] = sfi_port;
            }
        }    
        
        for (lchan=0; lchan<SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS; lchan++) {
            pchan = lchan;
            if (lchan >= SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS) {
              pchan = (uint8) SB_FAB_DEVICE_INVALID_DATA_CHANNEL;
            }
            xbar_state->lchan_to_pchan[lchan] = pchan;
        }
        
        for (pchan=0; pchan<SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS; pchan++) {
            lchan = pchan;
            xbar_state->pchan_to_lchan[pchan] = lchan;
        }
        
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_NODE_CROSSBARS; xbar++) {
            xbar_state->xbar_in_use[xbar]=TRUE;
        }
        
        for (xbar=0; xbar<SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; xbar++) {
            xbar_state->xbar_nodetype[xbar] = FALSE;
        }
    }

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {   
        /* Certain blocks are not fully inited in this case, can not write to memory in   
         * those blocks, skip BCM init. This is here for emulator and bringup   
         * could be cleaned up after sirius pass bringup stage.   
         * Moved after xbar_map setup for port enable/disable to work. 
         */   
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

        
#ifdef BCM_SIRIUS_SUPPORT
    if (soc_feature(unit, soc_feature_higig2)) {
        /* Allocate size for congestion message */
        congestion_size_p = sal_alloc((sizeof(uint8) * (SOC_HIGIG_HDR_MAX_MODID + 1)), "congestion size");
        if (congestion_size_p == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc congestion_size_p,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    rc = BCM_E_MEMORY;
	    goto err;
        }
	sal_memset(congestion_size_p, 0, (SOC_HIGIG_HDR_MAX_MODID + 1));
	
	SOC_SBX_STATE(unit)->fabric_state->congestion_size = congestion_size_p;
     
    }
#endif

#ifdef BCM_WARM_BOOT_SUPPORT
    rc = bcm_sbx_wb_fabric_state_init(unit);
    if (rc != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: error in WarmBoot fabric state init \n"), 
                   FUNCTION_NAME()));
        goto err;
    }
#endif

  if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
      (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE) ||
      SOC_IS_SBX_SIRIUS(unit)) {

    rc = bcm_sbx_fabric_demand_scale_init(unit);
    if (rc != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, bcm_sbx_fabric_demand_scale_init, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rc));
        goto err;
    }

    if (!soc_feature(unit, soc_feature_distribution_ability)) {
	BCM_SBX_UNLOCK(unit);
	return(rc);
    }
    
    /* allocate Distribution Group resources */
    ds_p = sal_alloc(sizeof(bcm_sbx_ds_state_t), "ds_group_memory");
    if (ds_p == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }
    sal_memset(ds_p, 0x00, sizeof(bcm_sbx_ds_state_t));

    ds_p->max_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
    ds_p->max_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
    ds_p->num_bytes_ds_desc = (ds_p->max_nodes / 8) + ((ds_p->max_nodes % 8) == 0 ? 0 : 1);
    ds_p->num_ds_grps_used = 0;
    ds_p->num_nodes = 0;

    ds_p->mc_full_eval_min_state = sal_alloc(sizeof(uint8) * SOC_SBX_CFG(unit)->num_ds_ids , "ds_group mc_full_eval_min_state");;
    if (ds_p->mc_full_eval_min_state == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }
    sal_memset(ds_p->mc_full_eval_min_state, BCM_FABRIC_DISTRIBUTION_SCHED_ANY, ds_p->max_ds_ids);

    sz = (ds_p->max_ds_ids / 8) + ((ds_p->max_ds_ids % 8) ? 1 : 0);
    ds_p->ds_grp_resource = sal_alloc(sz, "ds_group_resource");
    if (ds_p->ds_grp_resource == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }
    sal_memset(ds_p->ds_grp_resource, 0x00, sz);

    ds_p->ds_grp_membership = sal_alloc((ds_p->num_bytes_ds_desc * ds_p->max_ds_ids),
                                                               "ds_group_membership");
    if (ds_p->ds_grp_membership == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }
    sal_memset(ds_p->ds_grp_membership, 0x00, (ds_p->num_bytes_ds_desc * ds_p->max_ds_ids));

    ds_p->initialized = TRUE;
    ds_state_p[unit] = ds_p;

    /* setup Distribution Group/ESET 0 */
    rc = bcm_sbx_fabric_distribution_create(unit, BCM_FABRIC_DISTRIBUTION_WITH_ID, &ds_id);
    if (rc != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, bcm_fabric_distribution_create, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rc));
        goto err;
    }

    dist_modids_p = sal_alloc((sizeof(int) * ds_p->max_nodes), "temp_group_membership");
    if (dist_modids_p == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }

    /* Determine if SCI ports are preconfigured */
    for (node=0; node < ds_p->max_nodes; node++) {
	if (soc_property_port_get(unit,  node, spn_PORT_IS_SCI, 0)) {
	    *(dist_modids_p + ds_p->num_nodes++) = BCM_STK_NODE_TO_MOD(node);
	}
    }

    /* If no SCI ports found, default to range setting using num nodes configured */
    if (ds_p->num_nodes == 0) {
	for (node = 0; node < SOC_SBX_CFG(unit)->cfg_num_nodes; node++) {
	    *(dist_modids_p + ds_p->num_nodes++) = BCM_STK_NODE_TO_MOD(node);
	}
    }

    rc = bcm_sbx_fabric_distribution_set(unit, ds_id, ds_p->num_nodes, dist_modids_p);
    if (rc != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, bcm_fabric_distribution_set, Unit(%d), Err: 0x%x\n"),
                   FUNCTION_NAME(), unit, rc));
        goto err;
    }

    sal_free(dist_modids_p);
  }
    BCM_SBX_UNLOCK(unit);
    return(BCM_E_NONE);

err:
    if (SOC_SBX_STATE(unit)->fabric_state->xbar_state != NULL) {
        sal_free(SOC_SBX_STATE(unit)->fabric_state->xbar_state);
    }
 
    if (SOC_SBX_STATE(unit)->fabric_state->congestion_size != NULL) {
        sal_free(SOC_SBX_STATE(unit)->fabric_state->congestion_size);
    }

    if (ds_p != NULL) {
        if (ds_p->ds_grp_resource != NULL) {
            sal_free(ds_p->ds_grp_resource);
        }
        if (ds_p->ds_grp_membership != NULL) {
            sal_free(ds_p->ds_grp_membership);
        }
        sal_free(ds_p);

        if (dist_modids_p != NULL) {
            sal_free(dist_modids_p);
        }
    }
    ds_state_p[unit] = NULL;

    BCM_SBX_UNLOCK(unit);
    return(rc);
}


int
bcm_sbx_fabric_crossbar_connection_set(int unit,
                                       int xbar,
                                       int src_modid,
                                       bcm_port_t src_xbport,
                                       int dst_modid,
                                       bcm_port_t dst_xbport)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if ( (BCM_STK_MOD_IS_NODE(src_modid) == FALSE) || (BCM_STK_MOD_IS_NODE(dst_modid) == FALSE)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_connection_set, (unit, xbar, src_modid, src_xbport,
								     dst_modid, dst_xbport)));
    BCM_SBX_UNLOCK(unit);
    return rv;
}



/* Find a free logical crossbar or determine whether the lxbar requested
 * is free.  This function should only be called for the local node
 */
int
bcm_sbx_fabric_lxbar_alloc(int unit, int *lxbar, int with_id)
{
    bcm_error_t rv = BCM_E_INTERNAL;
    bcm_sbx_xbar_state_t *xbar_state;
    int xbar;

    xbar_state = SOC_SBX_STATE(unit)->fabric_state->xbar_state;
    if (lxbar == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: parameter error lxbar pointer null\n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }

    if (with_id == TRUE) { 

        if (*lxbar < SB_FAB_DEVICE_MAX_NODE_CROSSBARS) {

            if (xbar_state->xbar_in_use[*lxbar]==FALSE) {
                xbar_state->xbar_in_use[*lxbar] = TRUE;
                rv = BCM_E_NONE;
            } else {
	      LOG_ERROR(BSL_LS_BCM_COMMON,
	                (BSL_META_U(unit,
	                            "xbar(%d) already in use\n"),
	                 *lxbar));
	        rv = BCM_E_RESOURCE;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: lxbar(%d) requested out of range(0-23)\n"),
                       FUNCTION_NAME(), *lxbar));
            rv = BCM_E_INTERNAL;
        }
    } else {
        /* start at crossbar 21 because 22 and 23 only support 1 sfi channel */
        for (xbar=SB_FAB_DEVICE_MAX_NODE_CROSSBARS-3; xbar>=0; xbar--) {
            if (xbar_state->xbar_in_use[xbar] == FALSE) {
                xbar_state->xbar_in_use[xbar] = TRUE;
                *lxbar = xbar;
                rv = BCM_E_NONE;
                break;
            } else {
                rv = BCM_E_RESOURCE;
            }
        }
	if (rv == BCM_E_RESOURCE) {
	    if (xbar_state->xbar_in_use[22] == FALSE) {
	        xbar_state->xbar_in_use[22] = TRUE;
		*lxbar = 22;
		rv = BCM_E_NONE;
	    } else if (xbar_state->xbar_in_use[23] == FALSE) {
	        xbar_state->xbar_in_use[23] = TRUE;
		*lxbar = 23;
		rv = BCM_E_NONE;
	    }
	}
    }

#ifdef DEBUG_CROSSBAR_MAP
    if (rv == BCM_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "allocate lxbar(%d)\n"), *lxbar));
    }
#endif

    return rv;
}
/* Free a logical crossbar
 */
int
bcm_sbx_fabric_lxbar_free(int unit, int lxbar)
{
    bcm_error_t rv = BCM_E_NONE;
    bcm_sbx_xbar_state_t *xbar_state;

    if (lxbar >= SB_FAB_DEVICE_MAX_NODE_CROSSBARS) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s:xbar(%d) out of range (0-23)\n"),
                   FUNCTION_NAME(), lxbar));
        return rv;
    }
    xbar_state = SOC_SBX_STATE(unit)->fabric_state->xbar_state;

    if (xbar_state->xbar_in_use[lxbar] == TRUE) {
        xbar_state->xbar_in_use[lxbar] = FALSE;

#ifdef DEBUG_CROSSBAR_MAP
        LOG_CLI((BSL_META_U(unit,
                            "Freed lxbar(%d)\n"), lxbar));
#endif
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s:Freed lxbar(%d) state inconsistent, lxbar was never allocated\n"),
                   FUNCTION_NAME(), lxbar));
        rv = BCM_E_INTERNAL;   
    }

    return rv;
}

/* Get the SFI port associated with the logical crossbar on the given node */
int
bcm_sbx_fabric_sfi_for_xbar_get(int unit, int xbar, int node, int *sfi_port)
{
    bcm_error_t rv = BCM_E_NONE;
    if (node >= SB_FAB_DEVICE_MAX_NODES) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: node(%d) out of range\n"),
                   FUNCTION_NAME(), node));	
        return BCM_E_PARAM;
    }
    if (xbar >= SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: xbar(%d) out of range\n"),
                   FUNCTION_NAME(), xbar));	
        return BCM_E_PARAM;
    }
    if (SOC_SBX_STATE(unit)->fabric_state->xbar_state == NULL) {
       LOG_ERROR(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: xbar_state not initialized \n"),
                  FUNCTION_NAME()));	
        return BCM_E_RESOURCE;
    }

    if ((xbar < SB_FAB_DEVICE_MAX_NODE_CROSSBARS) &&
        (SOC_SBX_STATE(unit)->fabric_state->xbar_state->xbar_in_use[xbar])) {
        *sfi_port = SOC_SBX_STATE(unit)->fabric_state->xbar_state->xbar_map[xbar][node];
    } else {
        *sfi_port = SB_FAB_DEVICE_INVALID_CROSSBAR;
    }
    return rv;
} 

/* Set the sfi port for the logical crossbar and node given */
int
bcm_sbx_fabric_sfi_for_xbar_set(int unit, int xbar, int node, int sfi_port)
{
    bcm_error_t rv = BCM_E_NONE;

    if (node >= SB_FAB_DEVICE_MAX_NODES) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: node(%d) out of range\n"),
                   FUNCTION_NAME(), node));	
        return BCM_E_PARAM;
    }
    if (xbar >= SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: xbar(%d) out of range\n"),
                   FUNCTION_NAME(), xbar));	
        return BCM_E_PARAM;
    }
    if (SOC_SBX_STATE(unit)->fabric_state->xbar_state == NULL) {
       LOG_ERROR(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: xbar_state not initialized \n"),
                  FUNCTION_NAME()));	
        return BCM_E_RESOURCE;
    }
    if (sfi_port >= SB_FAB_DEVICE_MAX_NODE_CROSSBARS) {
       LOG_ERROR(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s: sfi_port(%d) out of range \n"),
                  FUNCTION_NAME(), sfi_port));	
        return BCM_E_PARAM;
    }
    SOC_SBX_STATE(unit)->fabric_state->xbar_state->xbar_map[xbar][node] = sfi_port;
    /* LOG_CLI((BSL_META_U(unit,
                           "xbar_map[logical_xbar %d][node %d] = port %d\n"), xbar, node, port)); */
    return rv;
} 

/* Function returns the logical xbar # associated with the sfi port */
int
bcm_sbx_fabric_xbar_for_sfi_get(int unit, int node, int sfi_port, int *lxbar, int *lxbar1)
{
    int xbar;
    int rv = BCM_E_NONE;
    int port;

    if (lxbar == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: need lxbar pointer passed\n"),
                   FUNCTION_NAME()));	
        return BCM_E_PARAM;
    }

    if (lxbar1 == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: need lxbar1 pointer passed\n"),
                   FUNCTION_NAME()));	
        return BCM_E_PARAM;
    }
    if (sfi_port >= SB_FAB_DEVICE_SIRIUS_LINKS) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: sfi_port out of range (%d)\n"),
                   FUNCTION_NAME(), sfi_port));	
        return BCM_E_PARAM;
    }

    if (node >= SB_FAB_DEVICE_MAX_NODES) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: node(%d) exceeds maximum (%d)\n"),
                   FUNCTION_NAME(), sfi_port, SB_FAB_DEVICE_MAX_NODES));	
        return BCM_E_PARAM;
    }

    *lxbar = SB_FAB_DEVICE_INVALID_CROSSBAR;
    *lxbar1 = SB_FAB_DEVICE_INVALID_CROSSBAR;

    for (xbar = 0; xbar < SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; xbar++) {

        rv = bcm_sbx_fabric_sfi_for_xbar_get(unit, xbar, node, &port);
        if (rv) {
            return rv;
        }
        if ( port == sfi_port) {
           if (!BCM_SBX_CROSSBAR_VALID(*lxbar)) {
                *lxbar = xbar;
            } else if (!BCM_SBX_CROSSBAR_VALID(*lxbar1)) {
                 *lxbar1 = xbar;
            } else {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "%s: more than 2 logical crossbars assigned to port(%d) node(%d)\n"),
                           FUNCTION_NAME(), sfi_port, node));	
                rv = BCM_E_CONFIG;
                break;
            }
        }
    }

#ifdef DEBUG_CROSSBAR_MAP_VERBOSE
    if (BCM_SBX_CROSSBAR_VALID(*lxbar)) {
        LOG_CLI((BSL_META_U(unit,
                            "port(%d) lxbar(%d)\n"), sfi_port, *lxbar));
    }
    if (BCM_SBX_CROSSBAR_VALID(*lxbar1)) {
        LOG_CLI((BSL_META_U(unit,
                            "port(%d) lxbar1(%d)\n"), sfi_port, *lxbar1));
    }

#endif
    return rv;
}

int
bcm_sbx_fabric_nodetype_for_xbar_get(int unit, 
                                     int xbar, 
                                     int *nt_qe2k)
{
    bcm_error_t rv = BCM_E_NONE;

    if (SOC_SBX_STATE(unit)->fabric_state != NULL) {
        *nt_qe2k = SOC_SBX_STATE(unit)->fabric_state->xbar_state->xbar_nodetype[xbar];
    } else {
        rv = BCM_E_PARAM;
    }
    return rv;
}

int bcm_sbx_fabric_nodetype_for_xbar_set(int unit,
                                         int xbar,
                                         int nt_qe2k)
{
    bcm_error_t rv = BCM_E_NONE;

    if (SOC_SBX_STATE(unit)->fabric_state != NULL) {
        SOC_SBX_STATE(unit)->fabric_state->xbar_state->xbar_nodetype[xbar] = nt_qe2k;
    } else {
        rv = BCM_E_PARAM;
    }
    return rv;
}

int
bcm_sbx_fabric_crossbar_connection_get(int unit,
                                       int xbar,
                                       int src_modid,
                                       bcm_port_t src_xbport,
                                       int dst_modid,
                                       bcm_port_t *dst_xbport)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if ( (BCM_STK_MOD_IS_NODE(src_modid) == FALSE) || (BCM_STK_MOD_IS_NODE(dst_modid) == FALSE)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_connection_get, (unit, xbar, src_modid, src_xbport,
								     dst_modid, dst_xbport)));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* This function returns the plane associated with the logical crossbar on the given node */
int
_bcm_sbx_fabric_crossbar_plane_get(int unit,
				   int lxbar,
				   int modid,
				   bcm_fabric_connection_mode_t *mode)
{
    bcm_sbx_xbar_state_t *xbar_state;
    int node;
    int phyxbar;
    uint8 **ability_p, ability;
    bcm_module_protocol_t protocol;
    bcm_error_t rv = BCM_E_NONE;
    int arbiter_id;

    rv = bcm_sbx_fabric_control_get(unit,
				    bcmFabricArbiterId,
				    &arbiter_id);

    ability_p =  SOC_SBX_STATE(unit)->port_state->ability;
    xbar_state = SOC_SBX_STATE(unit)->fabric_state->xbar_state;
    node = BCM_STK_MOD_TO_NODE(modid);
    protocol = SOC_SBX_STATE(unit)->stack_state->protocol[node];
    phyxbar = xbar_state->xbar_map[lxbar][node];

    *mode = -1;

    /* Phyiscal crossbar is unused */
    if (phyxbar == SB_FAB_DEVICE_INVALID_CROSSBAR) {
	return rv;
    }	

    ability = ability_p[phyxbar][node];
    /* LOG_CLI((BSL_META_U(unit,
                           "lxbar(%d) phyxbar(%d) ability(%d)\n"), lxbar, phyxbar, ability)); */



    /* ability    protocol1  protocol2  protocol3  protocol4  protocol5 */
    /* SFI            A          B          X         X           A     */
    /* DUAL_SFI       X          X         A-B       A-A          X     */
    /* DUAL_SFI_LOCAL A(+local)  B(+local)  X       A(+local)     A     */
    /* DUAL_SFI_LOOPBACK X       X          X         X           X     */
    switch (ability) {
	case BCM_PORT_ABILITY_SFI:
	    if ((protocol == bcmModuleProtocol1) || 
		(protocol == bcmModuleProtocol5)) {
		*mode = bcmFabricXbarConnectionModeA;
	    } else if (protocol == bcmModuleProtocol2) {
		*mode = bcmFabricXbarConnectionModeB;
	    }
	    break;
	case BCM_PORT_ABILITY_DUAL_SFI:
	    if (protocol == bcmModuleProtocol3){
		*mode = bcmFabricXbarConnectionModeC; /* both */
	    } else if (protocol == bcmModuleProtocol4) {
		*mode = bcmFabricXbarConnectionModeA;
	    } 
	    break;
	case BCM_PORT_ABILITY_DUAL_SFI_LOCAL:
	    if ((protocol == bcmModuleProtocol1) || 
		(protocol == bcmModuleProtocol5) ||
		(protocol == bcmModuleProtocol4)) {
		*mode = bcmFabricXbarConnectionModeA;
	    } else if (protocol == bcmModuleProtocol2) {
		*mode = bcmFabricXbarConnectionModeB;
	    }
	    break;
	case BCM_PORT_ABILITY_SFI_LOOPBACK:
	    break;
        case BCM_PORT_ABILITY_SFI_SCI:
	    if (arbiter_id == 0) {
		*mode = bcmFabricXbarConnectionModeA;
	    } else {
		*mode = bcmFabricXbarConnectionModeB;
	    }
	    break;
	default:
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "invalid ability for unit(%d) node(%d) lxbar(%d)\n"),
	               unit, node, lxbar));
	    rv = BCM_E_CONFIG;
	    break;
    }
    return rv;
}
                                   
/* Given the source node(from modid) and the destination node and the plane return the set  */
/* of logical crossbars between these nodes. This function is used for a Sirius system      */
/* where there are possibly 2 planes (a/b/both).  This function is analogous to             */
/* bcm_sbx_fabric_crossbar_status_get(unit, *xbars).  The original function doesn't         */
/* support differentiating between planes a/b crossbars.  This information required for     */
/* this function is in bcm_port_control_get(unit, bcm_port_t, bcmPortControlAbility, value) */
/* The user must call this function on the BME overlaying a gport over the bcm_port_t flag  */
/* so that the BME has knowledge of the physical ports associated with plane A and B on     */
/* a given node.                                                                            */
/* We also need to use the info from                                                        */
/* bcm_fabric_crossbar_mapping_get(unit, modid, switch_fabric_arbiter_id, xbar, bcm_port_t) */
/* to determine the logical to physical crossbar mapping associated with a given node.      */
/* The mapping set command has been updated so that bcm_port_t is now a gport so that we    */
/* can tell whether the command is meant for the given unit or whether it's just being      */
/* recorded for use by this command.  Then, the mapping set and the ability command are     */
/* played on the arbiter so that it can store and retrieve this info for use by this        */
/* function.                                                                                */
/* This function only works on the arbiter BM3200/BM9600                                    */
int
bcm_sbx_fabric_crossbar_connection_status_get(int unit,
					      int src_modid,
					      int dest_modid,
					      bcm_fabric_connection_mode_t mode,
					      uint64 *xbars)
{
    int rv = BCM_E_NONE;
    bcm_sbx_xbar_state_t *xbar_state;
    int src_node, dest_node;
    int src_phyxbar, dest_phyxbar;
    int logxbar;
    uint64 link_en_xbars = COMPILER_64_INIT(0,0);
    uint8 **ability_p;
    uint8 src_ability, dest_ability;

    COMPILER_64_ZERO(*xbars);

    xbar_state = SOC_SBX_STATE(unit)->fabric_state->xbar_state;

    if (SOC_SBX_CFG(unit)->uRedMode != bcmFabricRedELS) { 
	rv = bcm_sbx_fabric_crossbar_status_get(unit, xbars);
	return rv;
    }
    
    /* We are in enhanced load sharing mode, determine based upon ability and xbar map */
    for (logxbar=0; logxbar<SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; logxbar++) {
	
	ability_p =  (uint8**)SOC_SBX_STATE(unit)->port_state->ability;

	src_node = BCM_STK_MOD_TO_NODE(src_modid);
	src_phyxbar = xbar_state->xbar_map[logxbar][src_node];
	src_ability = ability_p[src_phyxbar][src_node];
	
	dest_node = BCM_STK_MOD_TO_NODE(dest_modid);
	dest_phyxbar = xbar_state->xbar_map[logxbar][dest_node];
	dest_ability = ability_p[dest_phyxbar][dest_node];
	
	/* Currently we only support Sirius A/B dual grant mode in this function */
	
	/* yet supported. SFI_SCI not yet supported.                             */
	/* if ((mode == bcmFabricXbarConnectionModeA) ||                         */
	/*     (mode == bcmFabricXbarConnectionModeB)) {                         */
	/* } else                                                                */
	if ((src_ability == dest_ability) && (src_ability == BCM_PORT_ABILITY_DUAL_SFI)) 
        { uint64 xbar_mask = COMPILER_64_INIT(0,1);
            COMPILER_64_SHL(xbar_mask, logxbar);
            COMPILER_64_OR(*xbars, xbar_mask);
	}
    }
    rv = bcm_sbx_fabric_crossbar_enable_get(unit, &link_en_xbars);
    if (rv) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "bcm_sbx_crossbar_connection_status_get() failed to determine link_enable\n")));
	return rv;
    }

    /* The set of logical crossbars is the and of the logical crossbars in the link enable */
    /* with those xbars which both have an ability of DUAL_SFI. Sirius is always dual SFI  */
    /* today since we don't yet support SFI_SCI and we don't support interop with QE2000   */
    /* and we don't support a Sirius of both channels being associated with plane A.       */
    COMPILER_64_AND(*xbars, link_en_xbars);

    return rv;
}
/* The operating interval is the epoch length in nanoseconds.  If the link_fail_count is 0   */
/* the current epoch length is calculated based upon the number of links enabled via         */
/* bcm_fabric_crossbar_enable_set().  The link_fail_count is subtracted from this number to  */
/* get the timeslot size chosen when the user wishes to address degraded system operation.   */
/*                                                                                           */
/* The user may need to know the epoch_length to determine how long a pass through the       */
/* the calendar table will take.  This is because the calendar restarts every time the start */
/* of epoch occurs.                                                                          */
/*                                                                                           */
/* This function will work on BM3200/BM9600/QE2000/Sirius                                    */
int
bcm_sbx_fabric_operating_interval_get(int unit,
				      int link_fail_count,
				      int *operating_interval)
{
    int rv = BCM_E_NONE;
    int xbar_count = 0;
    uint64 xbar_mask;
    uint32 xbar;
    uint32 timeslot_size_in_ns, epoch_length_in_ns;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
	return BCM_E_UNIT;
    }

    xbar_mask = SOC_SBX_CFG(unit)->xbar_link_en;

    for (xbar=0; xbar<64; xbar++) {
	if (COMPILER_64_BITTEST(xbar_mask, xbar)) {
	    xbar_count++;
	}
    }

    if (xbar_count == 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, crossbars not enabled, need to call bcm_fabric_crossbar_enable_set() Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_INTERNAL;
    }

    if (link_fail_count >= xbar_count) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid link_fail_count - greater than enabled crossbars, Unit(%d), link_fail_count(0x%x) link_en(0x%x)\n"),
                   FUNCTION_NAME(), unit, link_fail_count, xbar_count));
	return BCM_E_PARAM;
    }

    xbar_count = xbar_count - link_fail_count;

    timeslot_size_in_ns = soc_sbx_fabric_get_timeslot_size(unit,
							   xbar_count,
							   0 /* bSetTimeslotSizeForHalfBus */,
							   soc_feature(unit, soc_feature_hybrid));

    epoch_length_in_ns = SOC_SBX_CFG(unit)->epoch_length_in_timeslots * timeslot_size_in_ns;

    *operating_interval = epoch_length_in_ns;

    return rv;
}

/* The connection is the number of timeslots in an epoch.                                  */
/* The user needs this to calculate the limit of the table size.  link_fail_count is only  */
/* an input for completeness, it is not required to determine the number of timeslots in   */
/* an epoch as link_fail_count is not a factor.                                            */
/* This function works on the arbiter BM3200/BM9600 and on the QE2000/Sirius devices       */
int
bcm_sbx_fabric_connection_max_get(int unit,
				  int link_fail_count,
				  int *connection)
{
    int rv = BCM_E_UNAVAIL;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
	return BCM_E_UNIT;
    }

    *connection = SOC_SBX_CFG(unit)->epoch_length_in_timeslots;

    return rv;
}
/* The connection interval is the timeslot size in nanoseconds.  This can change based upon  */
/* the number of xbars currently in use.  To determine the current timeslot size on a BM3200 */
/* read the xbars in use then return either the timeslot size or the degraded timeslot size  */
/* To determine the current timeslot size on a BM9600, read the xbars in use, then read the  */
/* appropriate timeslot size register in fo_config18+                                        */
/* This function only works on the arbiter BM3200/BM9600 and on the QE2000/Sirius devices    */
int
bcm_sbx_fabric_connection_interval_get(int unit,
				       int link_fail_count,
				       int *connection_interval)
{

    int rv = BCM_E_NONE;
    int xbar_count = 0;
    uint64 xbar_mask;
    uint32 xbar;
    uint32 timeslot_size_in_ns;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
	return BCM_E_UNIT;
    }

    xbar_mask = SOC_SBX_CFG(unit)->xbar_link_en;

    for (xbar=0; xbar<64; xbar++) {
	if (COMPILER_64_BITTEST(xbar_mask, xbar)) {
	    xbar_count++;
	}
    }

    if (xbar_count == 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, crossbars not enabled, need to call bcm_fabric_crossbar_enable_set() Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_INTERNAL;
    }

    if (link_fail_count >= xbar_count) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid link_fail_count - greater than enabled crossbars, Unit(%d), link_fail_count(0x%x) link_en(0x%x)\n"),
                   FUNCTION_NAME(), unit, link_fail_count, xbar_count));
	return BCM_E_PARAM;
    }

    xbar_count = xbar_count - link_fail_count;

    timeslot_size_in_ns = soc_sbx_fabric_get_timeslot_size(unit,
							   xbar_count,
							   0 /* bSetTimeslotSizeForHalfBus */,
							   soc_feature(unit, soc_feature_hybrid));

    *connection_interval = timeslot_size_in_ns;

    return rv;
}

int
_bcm_sbx_fabric_node_type_get(int unit, int modid, int *node_type)
{
    int                     rv;
    bcm_module_protocol_t   protocol;

    rv = bcm_sbx_stk_module_protocol_get(unit, modid, &protocol);
    if BCM_FAILURE(rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "bcm_sbx_stk_module_protocol_get on module %d failed with "
                               "error:%s (%d)\n"), modid, bcm_errmsg(rv), rv));
        return rv;
    }

    switch (protocol) {
    case bcmModuleProtocol1:
    case bcmModuleProtocol2:
        *node_type = SB_FAB_NODE_TYPE_QE2K;
        break;
    case bcmModuleProtocol3:
    case bcmModuleProtocol4:
    case bcmModuleProtocol5:
        *node_type = SB_FAB_NODE_TYPE_SIRIUS_FIC;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Invalid module protocol set for modid: %d \n"),
                   modid));
        rv = SOC_E_PARAM;
        break;
    }

    return rv;
}
/* This function returns the number of bytes which are sent in a timeslot.
 * This is used by application to determine how to configure the TDM calendar 
 * for a given bps rate
 */
int
bcm_sbx_fabric_connection_bytes_get(int unit, int src_modid, int dest_modid,
                                    bcm_fabric_connection_mode_t mode,
                                    int link_fail_count, int *max_bytes)
{
    int     rv = BCM_E_NONE;
    int     xbar_count = 0;
    int     xbar, num_channels;
    int     src_node_type, dest_node_type;
    uint64  xbars;

    rv = bcm_sbx_fabric_crossbar_connection_status_get(unit, src_modid, 
                                                     dest_modid, mode, &xbars);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "bcm_sbx_fabric_crossbar_connection_status_get() "
                               "failed with error(%d)\n"), rv));
        return rv;
    }

    for (xbar=0; xbar<64; xbar++) {
        if (COMPILER_64_BITTEST(xbars, xbar)) {
            xbar_count++;
        }
    }

    BCM_IF_ERROR_RETURN(_bcm_sbx_fabric_node_type_get(unit, src_modid, 
                                                      &src_node_type));
    BCM_IF_ERROR_RETURN(_bcm_sbx_fabric_node_type_get(unit, dest_modid, 
                                                      &dest_node_type));
    if ((src_node_type == SB_FAB_NODE_TYPE_SIRIUS_FIC) && 
        (dest_node_type == SB_FAB_NODE_TYPE_SIRIUS_FIC)) {
        /* assume dual channels between sirius nodes  */
        num_channels = xbar_count * 2; 
    } else {
        num_channels = xbar_count;
    }
    rv = bcm_sbx_fabric_timeslot_burst_size_bytes_get(unit, src_node_type, 
                                      dest_node_type, num_channels, max_bytes);
    return rv;
}
/* this function is for enabling the TDM calendar and is only available on the SIRIUS device */
/* qs_config0.calendar_enable.  Only enable the calendar after the calendars on all devices  */
/* have been initialized.                                                                    */
/* This function only works on the sirius device.                                            */
int
bcm_sbx_fabric_tdm_enable_set(int unit,
			      int enable)
{
    int rv;
    
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_tdm_enable_set, (unit, enable)));
    return rv;
}

/* This function returns whether the TDM calendar is enabled.                                */
/* This function only works on the sirius device.                                            */
int
bcm_sbx_fabric_tdm_enable_get(int unit,
			      int *enable)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_tdm_enable_get, (unit, enable)));
    return rv;
}
/* This function returns the maximum number of calendar entries allowed.                     */
/* This function only works on the sirius device.                                            */
int
bcm_sbx_fabric_calendar_max_get(int unit,
				int *max_size)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_max_get, (unit, max_size)));
    return rv;
}

/* This function allows for the setting of the calendar size.  The maximum calendar size is  */
/* 2048*2 entries on Sirius. The multiplier is used because each entry has 2 possible queues */
/* one for each plane.                                                                       */
/* This function only works on the sirius device.                                            */
int
bcm_sbx_fabric_calendar_size_set(int unit,
				 int config_size)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_size_set, (unit, config_size)));
    return rv;
}
/* This function returns the current size set for the calendar. The maximum calendar size is */
/* 2048 entries on Sirius.                                                                   */
/* This function only works on the sirius device.                                            */
int
bcm_sbx_fabric_calendar_size_get(int unit,
				 int *config_size)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_size_get, (unit, config_size)));
    return rv;
}

/* This function allows the user to set up a single calendar entry.  cindex is the index into */
/* the calendar, connection is the timeslot number.  Even calendar entries access plane A and */
/* odd calendar entries access plane B.                                                       */
/* This function only works on the sirius device.                                             */
int
bcm_sbx_fabric_calendar_set(int unit,
			    int cindex,
			    int connection,
			    bcm_gport_t dest_port,
			    bcm_cos_queue_t dest_cosq)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_set, (unit, cindex, connection, dest_port, dest_cosq)));
    return rv;
}

/* This function returns a single calendar entry.  cindex is the index into the calendar,     */
/* connection is the timeslot number.  Even calendar entries access plane A and odd calendar  */
/* entries access plane B.                                                                    */
/* This function only works on the sirius device.                                             */
int
bcm_sbx_fabric_calendar_get(int unit,
			    int cindex,
			    int *connection,
			    bcm_gport_t *dest_port,
			    bcm_cos_queue_t *dest_cosq)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_get, (unit, cindex, connection, dest_port, dest_cosq)));
    return rv;
}

/* This function allows the user to set up multiple calendar entries.                         */
/* This function only works on the sirius device.                                             */
int
bcm_sbx_fabric_calendar_multi_set(int unit,
				  int array_size,
				  int *connection_array,
				  bcm_gport_t *dest_port_array,
				  bcm_cos_queue_t *dest_cosq_array)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_multi_set, (unit, array_size, connection_array, dest_port_array, dest_cosq_array)));
    return rv;
}


/* This function returns a set of calendar entries.                                           */
/* This function only works on the sirius device.                                             */
int
bcm_sbx_fabric_calendar_multi_get(int unit,
				  int array_size,
				  int *connection_array,
				  bcm_gport_t *dest_port_array,
				  bcm_cos_queue_t *dest_cosq_array)
{
    int rv;
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_multi_set, (unit, array_size, connection_array, dest_port_array, dest_cosq_array)));
    return rv;
}

/* This function forces a swap of the TDM calendar                                            */
/* This function only works on the BM9600 device                                              */
int
bcm_sbx_fabric_calendar_active(int unit)
{
    int rv;
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_calendar_active, (unit));
    return rv;
}


int
bcm_sbx_fabric_crossbar_mapping_set(int unit,
                                    int modid,
                                    int switch_fabric_arbiter_id,
                                    int xbar,
                                    bcm_port_t port)
{
    int rv;
    int node;
    int sfi_port;
    
    BCM_SBX_LOCK(unit);
    
    if (BCM_STK_MOD_IS_NODE(modid) == FALSE) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }
    
    if (switch_fabric_arbiter_id < 0 || switch_fabric_arbiter_id > 1) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }
    
    node = BCM_STK_MOD_TO_NODE(modid);    
    
    rv = bcm_sbx_port_to_sfi_base_get(unit, node, port, &sfi_port);
    if (rv) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_mapping_set, (unit, modid, switch_fabric_arbiter_id,
                               xbar, sfi_port)));
    
    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Mapping set failed(%d)\n"),
                   rv));        
        BCM_SBX_UNLOCK(unit);
        return rv;
    } 
 
    if (xbar != -1) {
        /* Store mapping of port to logical crossbar for the given node */
        rv = bcm_sbx_fabric_sfi_for_xbar_set(unit, xbar, node, sfi_port);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_crossbar_mapping_get(int unit,
                                    int modid,
                                    int switch_fabric_arbiter_id,
                                    int xbar,
                                    bcm_port_t *port)
{
    int rv;
    int node;
    int sfi_port;

    BCM_SBX_LOCK(unit);

    if (BCM_STK_MOD_IS_NODE(modid) == FALSE) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (switch_fabric_arbiter_id < 0 || switch_fabric_arbiter_id > 1) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_CFG(unit)->uRedMode == bcmFabricRedELS) { 
        /* Retrieve mapping of port to logical crossbar for the given node */
        node = BCM_STK_MOD_TO_NODE(modid);
        
        rv = bcm_sbx_fabric_sfi_for_xbar_get(unit, xbar, node, &sfi_port);
        if (rv) {
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        bcm_sbx_sfi_base_to_port_get(unit, node, sfi_port, port);

    }else {
        rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_mapping_get, 
                                   (unit, modid, switch_fabric_arbiter_id,
                                   xbar, port)));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

#ifdef NOTDEF
/* This returns the rate conversion value to be used in the QE2000             */
/* if the timeslot size value changes, this table will also need to            */
/* be recalculated from /system_sim/kamino/units/ka_qs/calculateRateConversion */
/* because it is based upon the number of logical crossbars.                   */
/* Note that this table is created for D-Mode epoch length of 16433 timeslots  */
/* This may need to be updated for Sirius/QE4000 devices                       */

static int32 nQe2000Bm3200RateConversion[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] =
/* 0            1           2           3          4         5            6           7          8           9                    */
/* 760,         8192,       5684,       3836,      2856,     2312,       1936,       1660,       1444,       1332ns timeslot size */
{ 0xbec6abcb, 0x8042ff4f, 0xb243b7ce, 0xf044884d,0xb344b74d, 0x90c4e24d, 0xf2c5874c, 0xd0459d4c,0xb545b4cc, 0xa6c5c44c,
/* 10           11          12          13         14        15           16          17         18                               */
/* 1172,        1064,       1008,       900,       848,      788,        760,         760,       760                              */
  0x92c5df4c, 0x8545f5cc, 0xfcc681cb, 0xe1c6914b,0xd4c69a4b, 0xc5c6a5cb, 0xbec6abcb, 0xbec6abcb, 0xbec6abcb };

static int32 nQe2000Bm9600RateConversion[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] =
{ 0xcdc69f4b, /* 0 820  */
  0x8042ff4f, /* 1 8192 */ 
  0xb243b7ce, /* 2 5684 */
  0xf044884d, /* 3 3836 */
  0xb344b74d, /* 4 2856 */
  0x90c4e24d, /* 5 2312 */
  0xf2c5874c, /* 6 1936 */
  0xd0459d4c, /* 7 1660 */
  0xb545b4cc, /* 8 1444 */
  0xa6c5c44c, /* 9 1332 */
  0x92c5df4c, /* 10 1172 */
  0x8545f5cc, /* 11 1064 */
  0xfcc681cb, /* 12 1008 */
  0xe1c6914b, /* 13 900  */
  0xd4c69a4b, /* 14 848  */
  0xcdc69f4b, /* 15 820  */
  0xcdc69f4b, /* 16 820  */
  0xcdc69f4b, /* 17 820  */
  0xcdc69f4b};/* 18 820 */

static int32 nQe2000HybridRateConversion[SB_FAB_DEVICE_QE2000_SFI_LINKS + 1] =
{
  0xbec6abcb, /*  0,  760 */
  0x8042ff4f, /*  1, 8192 */
  0xb243b7ce, /*  2, 5684 */
  0xf044884d, /*  3, 3836 */
  0xb344b74d, /*  4, 2856 */
  0x90c4e24d, /*  5, 2312 */
  0xf2c5874c, /*  6, 1936 */
  0xd0459d4c, /*  7, 1660 */
  0xb545b4cc, /*  8, 1444 */
  0xa6c5c44c, /*  9, 1332 */
  0x92c5df4c, /* 10, 1172 */
  0x8545f5cc, /* 11, 1064 */
  0xfec680cb, /* 12, 1016 */
  0xfec680cb, /* 13, 1016 */
  0xfec680cb, /* 14, 1016 */
  0xfec680cb, /* 15, 1016 */
  0xfec680cb, /* 16, 1016 */
  0xfec680cb, /* 17, 1016 */
  0xfec680cb  /* 18, 1016 */
};
#endif

static int
bcm_sbx_demand_scale_config_get(int unit, uint64 xbars, int *demand_scale)
{
    int rv = BCM_E_NONE, i;
    int32  nSfiCount;


    (*demand_scale) = 0;

    /* Do not override the SOC property. This is a fixed value for */
    /* all serdes links.                                           */
    if (SOC_SBX_CFG(unit)->is_demand_scale_fixed == TRUE) {
        (*demand_scale) = SOC_SBX_CFG(unit)->fixed_demand_scale;
        return(rv);
    }

    nSfiCount = 0;
    for (i = 0; i < 64; i++) {
        /* Count number of enabled links */
        nSfiCount += (COMPILER_64_BITTEST(xbars, i));
    }
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if (nSfiCount >= SB_FAB_DEVICE_QE2000_SFI_LINKS + 1) {
            nSfiCount = SB_FAB_DEVICE_QE2000_SFI_LINKS + 1;
	}
	(*demand_scale) = nDModeDemandScale[nSfiCount];
    }
    else if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
	      (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
	      (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
        
        if (nSfiCount >= SB_FAB_DEVICE_QE2000_SFI_LINKS + 1) {
            nSfiCount = SB_FAB_DEVICE_QE2000_SFI_LINKS + 1;
        }

	/* use highest demand scale to reduce divider trunking error in polaris BW allocation */
	if (0) {
	  (*demand_scale) = nNonDModeDemandScale[nSfiCount];
	}

	(*demand_scale) = 7;
    }
    else {
        return(BCM_E_INTERNAL);
    }

    return(rv);
}

int
bcm_sbx_adjust_demand_scale(int unit, uint64 xbars,
                  int adjust, int *adjusted_demand_shift, uint32 *adjusted_clocks_per_epoch)
{
    int rv = BCM_E_NONE;
    uint32  sfi_count;
    uint32  ts_size_ns;
    uint32  epoch_length_in_ns;
    uint32  clocks_per_epoch, adj_clocks_per_epoch;
    int     half_bus = FALSE;
    int     i;
    int     demand_shift_adjust = 0, demand_scale, max_scale;
    uint16 dev_id;
    uint8  rev_id;

    if (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT) {
        return(rv);
    }

    /* determine the timeslot size, epoch length in timeslots, clocks speed */
    sfi_count = 0;
    for (i = 0; i < 64; i++) {
        /* Count number of enabled links */
        sfi_count += (COMPILER_64_BITTEST(xbars, i));
    }

    /* only has sirius B0 and above in system */
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if ((SOC_IS_SIRIUS(unit) && (rev_id == BCM88230_A0_REV_ID)) ||
      	(SOC_SBX_CFG(unit)->diag_qe_revid == BCM88230_A0_REV_ID)) {
	max_scale = 7;
    } else {
	max_scale = 15;
    }

    /* NOTE: Hybrid mode not currently supported for Sirius */
    ts_size_ns = soc_sbx_fabric_get_timeslot_size(unit, sfi_count, half_bus,
                                                 soc_feature(unit, soc_feature_hybrid) );

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Unit:%d NumberSFILinks: %d, TimeslotSize: %d(ns), EpochLengthInTimeslots: %d, RateClockSpeed: %d\n"),
              FUNCTION_NAME(), unit, sfi_count, ts_size_ns,
              SOC_SBX_CFG(unit)->epoch_length_in_timeslots,
              SOC_SBX_CFG(unit)->uRateClockSpeed));

    epoch_length_in_ns = SOC_SBX_CFG(unit)->epoch_length_in_timeslots * ts_size_ns;
    clocks_per_epoch =  epoch_length_in_ns / 1000;
    clocks_per_epoch =  clocks_per_epoch * SOC_SBX_CFG(unit)->uClockSpeedInMHz;

    /* Adjust Demand Shift */
    for (adj_clocks_per_epoch = clocks_per_epoch;
                    adj_clocks_per_epoch > SOC_SBX_CFG(unit)->uMaxClocksInEpoch; ) {
        adj_clocks_per_epoch = adj_clocks_per_epoch >> 1;
        demand_shift_adjust++;
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s, Unit(%d) ClocksPerEpoch: %d(0x%x), DemandScaleAdjust(%d)\n"),
              FUNCTION_NAME(), unit, clocks_per_epoch, clocks_per_epoch, demand_shift_adjust));

    if (adjust == TRUE) {
        if (SOC_SBX_CFG(unit)->demand_scale + demand_shift_adjust > max_scale) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR %s, Unit(%d) DemandScaleAdjust(%d) Out Of Range\n"),
                       FUNCTION_NAME(), unit,
                       demand_shift_adjust));
            demand_shift_adjust = max_scale - SOC_SBX_CFG(unit)->demand_scale;
            adj_clocks_per_epoch = clocks_per_epoch >> demand_shift_adjust;

            /* QoS Guarantess not met */
            rv = BCM_E_INTERNAL;
        }
        SOC_SBX_CFG(unit)->demand_scale += demand_shift_adjust;
    }
    else {
        bcm_sbx_demand_scale_config_get(unit, xbars, &demand_scale);
        if (demand_scale + demand_shift_adjust > max_scale) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR %s, Unit(%d) DemandScaleAdjust(%d) Out Of Range\n"),
                       FUNCTION_NAME(), unit,
                       demand_shift_adjust));
            demand_shift_adjust = max_scale - demand_scale;
            adj_clocks_per_epoch = clocks_per_epoch >> demand_shift_adjust;

            /* QoS Guarantess not met */
            rv = BCM_E_INTERNAL;
        }
    }

    (*adjusted_demand_shift) = SOC_SBX_CFG(unit)->demand_scale;
    (*adjusted_clocks_per_epoch) = adj_clocks_per_epoch;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s, Unit(%d) DemandScaleAdjust(%d), NewDemandShift(%d), ClocksPerEpoch(%d), AdjClocksPerEpoch(%d)\n"),
              FUNCTION_NAME(), unit,
              demand_shift_adjust, SOC_SBX_CFG(unit)->demand_scale,
              clocks_per_epoch, adj_clocks_per_epoch));
    return(rv);
}

int32
bcm_sbx_fabric_get_rate_conversion(int unit, int32 nTotalLogicalCrossbars)
{
    int raw_rate_converter;
    int in_sign, in_exp, out_sign, out_exp;
    int new_in_sign, new_in_exp, new_out_sign, new_out_exp;
    int timeslot_size, done;
    uint64 in_mant, out_mant, mant_scaler_shift;
    uint32 in_mant_res = 0, out_mant_res = 0;
    int16 mant_scaler = (nTotalLogicalCrossbars > 2) ? 26 : 25;

    
    if ( (nTotalLogicalCrossbars < 0) ||
                     (nTotalLogicalCrossbars >= (SB_FAB_DEVICE_QE2000_SFI_LINKS + 1)) ) {
	return(0);
    }

    /* calculate rate converter based on the timeslot size (cycles), epoch size
     * assuming each queue process take 4 cycles, and credit cycle need to process 16K queues
     */
    timeslot_size = soc_sbx_fabric_get_timeslot_size(unit, nTotalLogicalCrossbars, 
						     SOC_SBX_CFG(unit)->bHalfBus,
						     soc_feature(unit, soc_feature_hybrid) );
    timeslot_size /= (1000 / SOC_SBX_CFG(unit)->uClockSpeedInMHz); /* convert to cycles */

    /* incoming converter */
    done = 0;
    in_sign = 0;
    in_exp = 0;
    COMPILER_64_SET(in_mant, 0, (8 * 4 * 16384));
    COMPILER_64_SHL(in_mant, mant_scaler);
    
    if(soc_sbx_div64(in_mant, ((timeslot_size * SOC_SBX_CFG(unit)->epoch_length_in_timeslots)), &in_mant_res) == -1) {
	LOG_CLI((BSL_META_U(unit,
                            "in_mant_res > 32bits\n")));
	return (-1);
    }
    COMPILER_64_SET(in_mant, 0, in_mant_res);
    COMPILER_64_SET(mant_scaler_shift,0,256);
    COMPILER_64_SHL(mant_scaler_shift, mant_scaler);
    while (!done) {
	if (COMPILER_64_LT(in_mant,mant_scaler_shift)) {
            COMPILER_64_UMUL_32(in_mant, 2);
	    in_sign = 1;
	    in_exp++;
	} else {
	    done = 1;
	}
    }

    COMPILER_64_SHR(in_mant,mant_scaler);
    
    /* outgoing converter */
    done = 0;
    out_sign = 0;
    out_exp = 0;
    COMPILER_64_SET(out_mant, 0, timeslot_size);
    COMPILER_64_UMUL_32(out_mant, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
    COMPILER_64_SHL(out_mant, mant_scaler);
    
    if(soc_sbx_div64(out_mant, (8 * 4 * 16384), &out_mant_res) == -1) {
	LOG_CLI((BSL_META_U(unit,
                            "out_mant_res > 32bits\n")));
	return (-1);
    }
    COMPILER_64_SET(out_mant, 0, out_mant_res);
    while (!done) {
	if (COMPILER_64_LT(out_mant, mant_scaler_shift)) {
            COMPILER_64_UMUL_32(out_mant, 2);
	    out_sign = 1;
	    out_exp++;
	} else {
	    done = 1;
	}
    }
    COMPILER_64_SHR(out_mant, mant_scaler);
    
    /* construct the rate coverter */
    raw_rate_converter = 0;
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_MANTISSA, raw_rate_converter, COMPILER_64_LO(in_mant));
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_SIGN, raw_rate_converter, in_sign);
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_EXPONENTIAL, raw_rate_converter, in_exp);
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_MANTISSA, raw_rate_converter, COMPILER_64_LO(out_mant));
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_SIGN, raw_rate_converter, out_sign);
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_EXPONENTIAL, raw_rate_converter, out_exp);	  

    /* demand scale */
    in_sign = SAND_HAL_GET_FIELD(KA, QS_RATE_CONVERSION, INCOMING_SIGN, raw_rate_converter);
    in_exp = SAND_HAL_GET_FIELD(KA, QS_RATE_CONVERSION, INCOMING_EXPONENTIAL, raw_rate_converter);
    out_sign = SAND_HAL_GET_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_SIGN, raw_rate_converter);
    out_exp = SAND_HAL_GET_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_EXPONENTIAL, raw_rate_converter);

    new_in_sign = (((in_exp * ((in_sign)?(-1):1)) + (int)(SOC_SBX_CFG(unit)->demand_scale)) < 0) ? 1 : 0;
    new_in_exp = ((in_exp * ((in_sign)?(-1):1)) + (int)(SOC_SBX_CFG(unit)->demand_scale)) * ((new_in_sign)?-1:1);

    new_out_sign = (((out_exp * ((out_sign)?(-1):1)) - (int)(SOC_SBX_CFG(unit)->demand_scale)) < 0) ? 1 : 0;
    new_out_exp = ((out_exp * ((out_sign)?(-1):1)) - (int)(SOC_SBX_CFG(unit)->demand_scale)) * ((new_out_sign)?-1:1);

    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_SIGN, raw_rate_converter, new_in_sign);
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, INCOMING_EXPONENTIAL, raw_rate_converter, new_in_exp);
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_SIGN, raw_rate_converter, new_out_sign);
    raw_rate_converter = SAND_HAL_MOD_FIELD(KA, QS_RATE_CONVERSION, OUTGOING_EXPONENTIAL, raw_rate_converter, new_out_exp);

    return raw_rate_converter;
}
int
bcm_sbx_fabric_crossbar_enable_set(int unit,
                                   uint64 xbars)
{
    int rv, rc;
    int adjusted_demand_shift, demand_scale;
    uint32 adj_clocks_per_epoch;


    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (SOC_SBX_STATE(unit)->fabric_state != NULL) {
	SOC_SBX_STATE(unit)->fabric_state->old_demand_scale = SOC_SBX_CFG(unit)->demand_scale;
    }

    rv = bcm_sbx_demand_scale_config_get(unit, xbars, &demand_scale);
    if (rv != BCM_E_NONE) {
	BCM_SBX_UNLOCK(unit);
	return(rv);
    }
    SOC_SBX_CFG(unit)->demand_scale = demand_scale;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s, Unit(%d) DemandScale(%d)\n"),
              FUNCTION_NAME(), unit,
              SOC_SBX_CFG(unit)->demand_scale));

    rc = bcm_sbx_adjust_demand_scale(unit, xbars, TRUE, &adjusted_demand_shift, &adj_clocks_per_epoch);
    if ((rc != BCM_E_NONE) && (rc != BCM_E_INTERNAL)) {
        /* ignore overflow error for now. This will configure the */
        /* hardware with the nearest possible configuration       */
        BCM_SBX_UNLOCK(unit);
        return(rc);
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s, Unit(%d) NewDemandScale(%d)\n"),
              FUNCTION_NAME(), unit,
              SOC_SBX_CFG(unit)->demand_scale));

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_enable_set, (unit, xbars));

    SOC_SBX_CFG(unit)->xbar_link_en = xbars;


    if (rc == BCM_E_INTERNAL) {
        /* return internal overflow error to application. QoS Guarantess not met */
      BCM_SBX_UNLOCK(unit);
        return(rc);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_crossbar_enable_get(int unit,
                                   uint64 *xbars)
{
    int rv;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_enable_get, (unit, xbars));

    if (rv == BCM_E_UNAVAIL) {
        /* retreive cached value */
        (*xbars) = SOC_SBX_CFG(unit)->xbar_link_en;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_crossbar_status_get(int unit,
                                   uint64 *xbars)
{
    int rv;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_crossbar_status_get, (unit, xbars));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_distribution_create(int unit,
                                   uint32  flags,
                                   bcm_fabric_distribution_t *ds_id)
{
    int rc = BCM_E_NONE;
    int i;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_distribution_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_fabric_distribution_create == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (ds_state_p[unit] == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }
    if (ds_id == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if (flags & (~BCM_FABRIC_DISTRIBUTION_WITH_ID)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Flags Parameter, Unit(%d), Flags(0x%x)\n"),
                   FUNCTION_NAME(), unit, flags));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if (flags & BCM_FABRIC_DISTRIBUTION_WITH_ID) { /* distribution id/ESET managed by user */
        if ((*ds_id) >= ds_state_p[unit]->max_ds_ids) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Invalid DsId Resource specified, Unit(%d), dsId: 0x%x\n"),
                       unit, (*ds_id)));
	    BCM_SBX_UNLOCK(unit);
            return(BCM_E_RESOURCE);
        }

        if (BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, (*ds_id))) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Resource already allocated, Unit(%d), DsId(%d)\n"),
                       FUNCTION_NAME(), unit, (*ds_id)));
	    BCM_SBX_UNLOCK(unit);
            return(BCM_E_PARAM);
        }
    }
    else { /* allocate an unused distribution id/ESET */

	if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: must allocate DsId (ESET) with ID when reloading, Unit(%d)\n"),
                       unit));
	    BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
#endif /* BCM_EASY_RELOAD_SUPPORT */
	}

        /* check if resource is available */
        if (ds_state_p[unit]->num_ds_grps_used >= ds_state_p[unit]->max_ds_ids) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: no DsId Resource available, Unit(%d)\n"),
                       unit));
	    BCM_SBX_UNLOCK(unit);
            return(BCM_E_RESOURCE);
        }

        /* search for first available resource */
        for (i = 0; i < ds_state_p[unit]->max_ds_ids; i++) {
            if (BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, i)) {
                continue;
            }

            (*ds_id) = i;
            break;
        }

        if (i >= ds_state_p[unit]->max_ds_ids) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Inconsistency, no DsId Resource available, Unit(%d)\n"),
                       unit));
	    BCM_SBX_UNLOCK(unit);
            return(BCM_E_RESOURCE);
        }

    }

    BCM_SBX_DS_ID_SET_RESOURCE(unit, (*ds_id));
    ds_state_p[unit]->num_ds_grps_used++;

#if 0
    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_distribution_create,unit, ds_id));
#endif /* 0 */

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_fabric_distribution_destroy(int unit,
                                    bcm_fabric_distribution_t  ds_id)
{
    int rc = BCM_E_NONE;
    int node;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_distribution_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_fabric_distribution_destroy == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (ds_state_p[unit] == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }
    if (ds_id >= ds_state_p[unit]->max_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid DsId, Unit(%d), DsId(%d) MaxDsId(%d)\n"),
                   FUNCTION_NAME(), unit, ds_id, ds_state_p[unit]->max_ds_ids));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if (!(BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, ds_id))) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "ERROR: %s, Resource not allocated, Unit(%d), DsId(%d)\n"),
                     FUNCTION_NAME(), unit, ds_id));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_distribution_destroy, (unit, ds_id));

    for (node = 0; node < ds_state_p[unit]->max_nodes; node++) {
        BCM_SBX_DS_ID_CLEAR_NODE_MEMBER(unit, ds_id, node);
    }
    BCM_SBX_DS_ID_CLEAR_RESOURCE(unit, ds_id);
    ds_state_p[unit]->num_ds_grps_used--;

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_fabric_distribution_set(int unit,
                                bcm_fabric_distribution_t  ds_id,
                                int modid_count,
                                int *dist_modids)
{
    int rc = BCM_E_NONE;
    int i, iter;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_distribution_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_fabric_distribution_set == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (ds_state_p[unit] == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }
    if ( (modid_count != 0) && (dist_modids == NULL) ) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (modid_count > ds_state_p[unit]->max_nodes) { /* duplicate configuration is not allowed */
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (ds_id >= ds_state_p[unit]->max_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid DsId, Unit(%d), DsId(%d) MaxDsId(%d)\n"),
                   FUNCTION_NAME(), unit, ds_id, ds_state_p[unit]->max_ds_ids));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    for (i = 0; i < modid_count; i++) {
        if (BCM_STK_MOD_TO_NODE(*(dist_modids + i)) >= ds_state_p[unit]->max_nodes)  {
           LOG_ERROR(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "ERROR: %s, Invalid ModId, Unit(%d), ModId(%d) MaxModId(%d)\n"),
                      FUNCTION_NAME(), unit, *(dist_modids + i),
                      BCM_STK_NODE_TO_MOD(ds_state_p[unit]->max_nodes - 1)));
	   BCM_SBX_UNLOCK(unit);
	   return(BCM_E_PARAM);
        }
    }
    if (!(BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, ds_id))) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "ERROR: %s, Resource not allocated, Unit(%d), DsId(%d)\n"),
                     FUNCTION_NAME(), unit, ds_id));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_distribution_set, (unit,
			      ds_id, modid_count, dist_modids, ds_state_p[unit]->mc_full_eval_min_state[ds_id]));

    BCM_SBX_DS_ID_CLEAR_ALL_NODE_MEMBER(unit, ds_id, iter);
    for (i = 0; i < modid_count; i++) {
        BCM_SBX_DS_ID_SET_NODE_MEMBER(unit, ds_id, BCM_STK_MOD_TO_NODE(*(dist_modids + i)));
    }

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_fabric_distribution_get(int unit,
                                bcm_fabric_distribution_t  ds_id,
                                int max_count,
                                int *dist_modids,
                                int *count)
{
    int rc = BCM_E_NONE;
    int node, num_nodes;
#if 0
    int mc_full_eval_min;
#endif
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_distribution_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_fabric_distribution_get == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (ds_state_p[unit] == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }
    if ( (max_count == 0) || (dist_modids == NULL) || (count == NULL) ) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (ds_id >= ds_state_p[unit]->max_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid DsId, Unit(%d), DsId(%d) MaxDsId(%d)\n"),
                   FUNCTION_NAME(), unit, ds_id, ds_state_p[unit]->max_ds_ids));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (!(BCM_SBX_DS_ID_IS_RESOURCE_ALLOCATED(unit, ds_id))) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "ERROR: %s, Resource not allocated, Unit(%d), DsId(%d)\n"),
                     FUNCTION_NAME(), unit, ds_id));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    for (node = 0, num_nodes = 0;
            ((node < ds_state_p[unit]->max_nodes) && (num_nodes < max_count)); node++) {
        if (BCM_SBX_DS_ID_IS_NODE_MEMBER(unit, ds_id, node)) {
            dist_modids[num_nodes++] = BCM_STK_NODE_TO_MOD(node);
        }
    }
    (*count) = num_nodes;

#if 0
    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_distribution_get, (unit, ds_id, max_count,
                                                                dist_modids, count, &mc_full_eval_min));
#endif /* 0 */

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_fabric_control_set(int unit,
                           bcm_fabric_control_t type,
                           int arg)
{
    int rv;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    switch (type) {
        case bcmFabricArbiterId:
            if ( (arg < 0) || (arg > 1)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "bcmFabricArbiterId %d out of range\n"), arg));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }
            break;
        case bcmFabricActiveArbiterId:
            if ( (arg < 0) || (arg > 1)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "bcmFabricActiveArbiterId %d out of range\n"), arg));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }
            break;
        case bcmFabricMaximumFailedLinks:
            if ( (arg < 0) || (arg > SB_FAB_DEVICE_QE2000_SFI_LINKS)) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "bcmFabricMaximumFailedLinks %d out of range\n"), arg));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }
            SOC_SBX_CFG(unit)->uMaxFailedLinks = arg;
            break;
        case bcmFabricRedundancyMode:
            if ( (arg < bcmFabricRedManual) || (arg > bcmFabricRedELS) ) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "bcmFabricRedundancyMode %d out of range\n"), arg));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }
            SOC_SBX_CFG(unit)->uRedMode = arg;
            break;
        case bcmFabricArbiterConfig:
        case bcmFabricMode:
        case bcmFabricMcGroupSourceKnockout:
        case bcmFabricQueueMin:
        case bcmFabricQueueMax:
        case bcmFabricEgressQueueMin:
        case bcmFabricEgressQueueMax:
        case bcmFabricSubscriberCosLevels:
        case bcmFabricSubscriberCosLevelAllocation:
        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
        case bcmFabricShaperQueueMin:
        case bcmFabricShaperQueueMax:
        case bcmFabricShaperQueueIncrement:
        case bcmFabricShaperEgressQueueMin:
        case bcmFabricShaperEgressQueueMax:
        case bcmFabricShaperEgressQueueIncrement:
        case bcmFabricManager:
        case bcmFabricTsApplicationHierachySetup:
        case bcmFabricMaxPorts:
        case bcmFabricEgressDropLimitBytes:
        case bcmFabricEgressYellowDropLimitBytes:
        case bcmFabricEgressRedDropLimitBytes:
        case bcmFabricDemandCalculationEnable:
        case bcmFabricOperatingIntervalEnable:
        case bcmFabricIngressLevel1NumSchedulers:
        case bcmFabricIngressLevel2NumSchedulers:
        case bcmFabricIngressLevel3NumSchedulers:
        case bcmFabricIngressLevel4NumSchedulers:
        case bcmFabricIngressLevel5NumSchedulers:
        case bcmFabricIngressLevel6NumSchedulers:
        case bcmFabricIngressLevel7NumSchedulers:
        case bcmFabricIngressLevel1SchedulerUpdateCycles:
        case bcmFabricIngressLevel2SchedulerUpdateCycles:
        case bcmFabricIngressLevel3SchedulerUpdateCycles:
        case bcmFabricIngressLevel4SchedulerUpdateCycles:
        case bcmFabricIngressLevel5SchedulerUpdateCycles:
        case bcmFabricIngressLevel6SchedulerUpdateCycles:
        case bcmFabricIngressLevel7SchedulerUpdateCycles:
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "bcm_fabric_control_set unrecognized fabric control type %d \n"), (int)type));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_control_set, (unit, type, arg));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_control_get(int unit,
                           bcm_fabric_control_t type,
                           int *arg)
{
    int rv;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_control_get, (unit, type, arg));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_control_redundancy_register(int unit,
                                           bcm_fabric_control_redundancy_handler_t f)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) ||
	(SOC_SBX_STATE(unit)->fabric_state != &fabric_state[unit])) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    SOC_SBX_STATE(unit)->fabric_state->red_f = f;

    BCM_SBX_UNLOCK(unit);
    return (BCM_E_NONE);
}

int
bcm_sbx_fabric_control_redundancy_unregister(int unit,
                                             bcm_fabric_control_redundancy_handler_t f)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) ||
	(SOC_SBX_STATE(unit)->fabric_state != &fabric_state[unit])) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    SOC_SBX_STATE(unit)->fabric_state->red_f = NULL;

    BCM_SBX_UNLOCK(unit);
    return (BCM_E_NONE);
}

int
bcm_sbx_fabric_packet_adjust_set(int unit,
                                 int pkt_adjust_selector,
                                 int pkt_adjust_len)
{
    int rv;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return(BCM_E_UNIT);
    }

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_packet_adj_len))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((pkt_adjust_selector & BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK) >
        SOC_SBX_CFG(unit)->max_pkt_len_adj_sel) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (pkt_adjust_len > SOC_SBX_CFG(unit)->max_pkt_len_adj_value) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_fabric_packet_adjust_get == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_packet_adjust_set, (unit, pkt_adjust_selector,
								         pkt_adjust_len)));

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_fabric_packet_adjust_get(int unit, int pkt_adjust_selector,
                                               int *pkt_adjust_len)
{
    int rv;

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return(BCM_E_UNIT);
    }

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_packet_adj_len))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((pkt_adjust_selector & BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK) >
        SOC_SBX_CFG(unit)->max_pkt_len_adj_sel) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (pkt_adjust_len == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_fabric_packet_adjust_get == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_packet_adjust_get, (unit, pkt_adjust_selector,
								         pkt_adjust_len)));

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_fabric_port_failover_set(int unit, bcm_gport_t active_gport,
				 bcm_failover_t failover_id,
				 bcm_gport_t protect_gport)
{
    int rv = BCM_E_NONE;
    int num_failovers;
    int in_use;
    int sysport;
    int fabric_port = 0xff;
    int active_node, active_port, active_queue, protect_node, protect_port;
    int old_protect_gport, old_protect_node;
    bcm_sbx_failover_object_t *fo_state;
    bcm_sbx_cosq_sysport_group_state_t *p_spg;
    int aps, cos, port, subport, offset;
    SHR_BITDCLNAME(subportmask,  SB_FAB_DEVICE_MAX_FABRIC_PORTS);

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return(BCM_E_UNIT);
    }

    BCM_SBX_LOCK(unit);

    /* clear the bitmask */
    sal_memset(subportmask, 0x00, sizeof(subportmask));
	
    sysport = BCM_INT_SBX_INVALID_SYSPORT;
    active_node = -1;
    active_port = -1;
    num_failovers = SOC_SBX_CFG(unit)->num_sysports;

    if ( ( failover_id < 0 ) || (failover_id >= num_failovers) ) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_STATE(unit)->failover_state == NULL) {
	/* if failover_state pointer is NULL, it's on a unit which doesn't support failover command */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    fo_state = &(SOC_SBX_STATE(unit)->failover_state[failover_id]);

    if ( !(fo_state->state & BCM_SBX_FAILOVER_RESERVED)) {
	/* if it's not reserved, can not set failover */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_MODPORT(protect_gport) &&
	(BCM_GPORT_IS_CHILD(active_gport) ||
	 (BCM_GPORT_IS_MODPORT(active_gport) &&
	  SOC_IS_SBX_SIRIUS(unit)))) {
	/* the protected gport has to be modport (interfaces), if the active gport is 
	 * child gport (sirius device) or if active port is a modport on a sirius device
	 * then this API is to setup fabric port failover instead of queue group APS.
	 */
	aps = FALSE;
    } else {
	aps = TRUE;
    }

    in_use = fo_state->state & BCM_SBX_FAILOVER_INUSE;

    if (aps == TRUE) {
	/* APS, queue group failover to another fabric port */
	if ( (in_use) && (active_gport != fo_state->active_gport) ) {
	    /* if it's in use, we don't allow user to switch to another active gport
	     * user has to alloc and use another failover id
	     */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

	old_protect_gport = BCM_GPORT_INVALID;
	
	if ( !in_use ) {
	    /* the failover profile is not in use, first time to setup */
	    if (BCM_GPORT_IS_MODPORT(active_gport)) {
		if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, BCM_COSQ_INIT)) {
		    /* The bcm_sbx_fabric_port_failover_set should make sure the active_gport
		     * stored are legal, so any failure here should report BCM_E_INTERNAL 
		     */
		    /* get node/port */
		    active_node = BCM_GPORT_MODPORT_MODID_GET(active_gport);
		    if (active_node < BCM_MODULE_FABRIC_BASE) {
			LOG_ERROR(BSL_LS_BCM_COMMON,
			          (BSL_META_U(unit,
			                      "ERROR: gport modid invalid value (%d) is less than minimum (%d)\n"),
			           active_node, BCM_MODULE_FABRIC_BASE));
			BCM_SBX_UNLOCK(unit);
			return BCM_E_PARAM;
		    }
		    active_node = BCM_STK_MOD_TO_NODE(active_node);
		    active_port = BCM_GPORT_MODPORT_PORT_GET(active_gport);
		    
		    /* get queue id from node/port */
		    if (soc_feature(unit, soc_feature_standalone)) {
			active_queue = SOC_SBX_NODE_PORT_TO_QID(unit,active_node, active_port, 16);
		    } else {
			active_queue = SOC_SBX_NODE_PORT_TO_QID(unit,active_node, active_port, NUM_COS(unit));
		    }
		    
		    /* get sysport from queue state */
		    sysport = SOC_SBX_STATE(unit)->queue_state[active_queue].sysport;
		    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
			BCM_SBX_UNLOCK(unit);
			return BCM_E_PARAM;
		    }
		    
		} else {
		    /* if the active gport is in mod/port format, we have to be in the bcm_cosq_init=1 config
		     * which will preallocate all queues, otherwise, there must be some error in param checking
		     * of the bcm_sbx_fabric_port_failover_set
		     */
		    BCM_SBX_UNLOCK(unit);
		    return BCM_E_PARAM;	
		}
	    } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(active_gport)) {	
		sysport = BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(active_gport);
		if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
		    BCM_SBX_UNLOCK(unit);
		    return BCM_E_PARAM;
		}
		
		active_node = SOC_SBX_STATE(unit)->sysport_state[sysport].node;
		active_port = SOC_SBX_STATE(unit)->sysport_state[sysport].port;
	    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(active_gport)) {
		/*
		 * Unsupported functionality in TME mode
		 */
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	    
	    fo_state->active_gport = active_gport;
	    fo_state->sysport = sysport;
	    fo_state->active_nodeport = (((active_node & 0xFFFF) << 16) | (active_port & 0xFFFF));
	    
	} else {
	    /* the failover profile is in use, in this case, the active node/port and sysport would not change,
	     * also, the node/port in sysport state is not reliable since it record the current node/port used
	     * by sysport, which could be the protect node/port 
	     */
	    old_protect_gport = fo_state->protect_gport;
	    active_node = ((fo_state->active_nodeport & 0xFFFF0000) >> 16);
	    active_port = (fo_state->active_nodeport & 0xFFFF);
	    sysport = fo_state->sysport;
	}
	
	if (BCM_GPORT_IS_MODPORT(protect_gport)) {
	    fo_state->protect_gport = protect_gport;
	    protect_node = BCM_GPORT_MODPORT_MODID_GET(protect_gport);
	    if (protect_node < BCM_MODULE_FABRIC_BASE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: gport modid invalid value (%d) is less than minimum (%d)\n"),
		           protect_node, BCM_MODULE_FABRIC_BASE));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	    protect_node = BCM_STK_MOD_TO_NODE(protect_node);
	    protect_port = BCM_GPORT_MODPORT_PORT_GET(protect_gport);	
	} else {
	    /* protect gport has to be in node/port format */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
	
	fo_state->state |= BCM_SBX_FAILOVER_INUSE;
	fo_state->state |= BCM_SBX_FAILOVER_APS;
	
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_failover_set, (unit, BCM_INT_SBX_SYSPORT_DUMMY(sysport),
				   protect_node, protect_port, active_node, active_port)));
	
	if (rv == BCM_E_NONE) {
	    
	    /* update sysport group states */
	    p_spg = &(SOC_SBX_STATE(unit)->sysport_group_state[sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP]);
	    
	    if (old_protect_gport != BCM_GPORT_INVALID) {
		old_protect_node = BCM_GPORT_MODPORT_MODID_GET(old_protect_gport);
		old_protect_node = BCM_STK_MOD_TO_NODE(old_protect_node);
		p_spg->node_cnt[old_protect_node]--;
		if (p_spg->node_port[old_protect_node] != 0xff) {
		    fabric_port = p_spg->node_port[old_protect_node];
		}
		if (p_spg->node_cnt[old_protect_node] == 0) {
		    p_spg->node_port[old_protect_node] = 0xff;
		}
	    }
	    
	    if ((active_node >= 0) && (active_node < 128)) {
		/* The setup of new node_cnt need to happen after the removal of old_protect_node from
		 * mask, since it's possible the old_protect_node is same as active_node or protect_node
		 */
		
		if (p_spg->node_cnt[active_node] == 0) {
		    p_spg->node_port[active_node] = fabric_port;
		}
		p_spg->node_cnt[active_node]++;
	    }
	    
	    if (p_spg->node_cnt[protect_node] == 0) {
		p_spg->node_port[protect_node] = fabric_port;
	    }
	    p_spg->node_cnt[protect_node]++;
	}
    } else {
	/* fabric port failover to another interface port */
	if (in_use && (protect_gport != fo_state->protect_gport)) {
	    /* if it's in use, we don't allow user to switch to another protect gport
	     * user has to alloc and use another failover id
	     */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

	if (fo_state->state & BCM_SBX_FAILOVER_APS) {
	    /* the failover id already used for APS feature */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

	if (BCM_GPORT_IS_CHILD(active_gport)) {
	    /* make sure the child port belongs to same interface port */		
	    active_port = BCM_GPORT_CHILD_PORT_GET(active_gport);
	    if ((active_port < 0) || (active_port >=  SB_FAB_DEVICE_MAX_FABRIC_PORTS) ||
		(SOC_SBX_STATE(unit)->port_state->subport_info[active_port].valid == FALSE)) {
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }

	    SHR_BITSET(subportmask, active_port);

	    /* find the parent */
	    BCM_GPORT_EGRESS_CHILD_SET(active_gport, BCM_GPORT_CHILD_MODID_GET(active_gport), 
				       BCM_GPORT_CHILD_PORT_GET(active_gport));

	    cos = -1;
	    rv = bcm_cosq_gport_attach_get(unit, active_gport, &active_gport, &cos);
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }

	    BCM_GPORT_MODPORT_SET(active_gport, BCM_GPORT_EGRESS_MODPORT_MODID_GET(active_gport),
				  BCM_GPORT_EGRESS_MODPORT_PORT_GET(active_gport));
	} else if (BCM_GPORT_IS_MODPORT(active_gport)) {
	    /* user could use modport to protect all child port on the same interface */
	    active_port = BCM_GPORT_MODPORT_PORT_GET(active_gport);

	    for (subport = 0; subport <  SB_FAB_DEVICE_MAX_FABRIC_PORTS; subport++) {
		if (SOC_SBX_STATE(unit)->port_state->subport_info[subport].valid == FALSE) {
		    continue;
		}
		rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, &offset);
		if ((rv == BCM_E_NONE) && (port == active_port)) {
		    SHR_BITSET(subportmask, subport);
		}
	    }
	    rv = BCM_E_NONE;
	}

	if ( !in_use ) {
	    fo_state->active_gport = active_gport;
	    fo_state->protect_gport = protect_gport;
	} else {
	    if (in_use && (active_gport != fo_state->active_gport)) {
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	}

	/* update SDK software state only, hardware reconfig is done at bcm_failover_set */
	fo_state->state |= BCM_SBX_FAILOVER_INUSE;
	SHR_BITOR_RANGE(fo_state->subportmask, subportmask, 0,
			 SB_FAB_DEVICE_MAX_FABRIC_PORTS, fo_state->subportmask);
    }

    BCM_SBX_UNLOCK(unit);
    return(rv);
}


int
bcm_sbx_fabric_port_failover_get(int unit, bcm_gport_t active_gport,
				 bcm_failover_t *failover_id,
				 bcm_gport_t *protect_gport)
{
    int rv = BCM_E_NONE;
    int num_failovers, failover;
    int found;
    bcm_sbx_failover_object_t *fo_state;
    int aps, active_port;
    int subport, port, offset;
    SHR_BITDCLNAME(subportmask,  SB_FAB_DEVICE_MAX_FABRIC_PORTS);
    SHR_BITDCLNAME(tempmask,  SB_FAB_DEVICE_MAX_FABRIC_PORTS);

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return(BCM_E_UNIT);
    }

    BCM_SBX_LOCK(unit);

    if ( ( failover_id == NULL ) || (protect_gport == NULL) ) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_STATE(unit)->failover_state == NULL) {
	/* if failover_state pointer is NULL, it's on a unit which doesn't support failover command */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    /* clear the bitmask */
    sal_memset(subportmask, 0x00, sizeof(subportmask));
    sal_memset(tempmask, 0x00, sizeof(tempmask));

    num_failovers = SOC_SBX_CFG(unit)->num_sysports;
    found = FALSE;
    *failover_id = -1;
    *protect_gport = BCM_GPORT_INVALID;

    if (BCM_GPORT_IS_CHILD(active_gport) || (BCM_GPORT_IS_MODPORT(active_gport) &&
					     SOC_IS_SBX_SIRIUS(unit))) {
	aps = FALSE;
    } else {
	aps = TRUE;
    }

    if (aps == TRUE) {
	if ( (BCM_GPORT_IS_MODPORT(active_gport) && (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, BCM_COSQ_INIT))) ||
	     (BCM_GPORT_IS_UCAST_QUEUE_GROUP(active_gport)) ||
	     (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(active_gport)) ) {
	    /* active_gport could be in Mod/Port format if bcm_cosq_init == 1, otherwise it has to be
	     * a unicast gport in sysport/queue group format
	     */
	    for (failover = 0; failover < num_failovers; failover++) {
		fo_state = &(SOC_SBX_STATE(unit)->failover_state[failover]);
		if ( (fo_state->state & BCM_SBX_FAILOVER_RESERVED) &&
		     (fo_state->state & BCM_SBX_FAILOVER_INUSE) && 
		     (fo_state->state & BCM_SBX_FAILOVER_APS) && 
		     (fo_state->active_gport == active_gport) ) {
		    found = TRUE;
		    *failover_id = failover;
		    *protect_gport = fo_state->protect_gport;
		}
	    }
	} else {
	    rv = BCM_E_PARAM;
	}
    } else {
	/* convert active_gport to bit positon, if it's interface port, convert to all fabric
	 * ports on the interface port
	 */	
	if (BCM_GPORT_IS_CHILD(active_gport)) {
	    active_port = BCM_GPORT_CHILD_PORT_GET(active_gport);
	    if ((active_port < 0) || (active_port >=  SB_FAB_DEVICE_MAX_FABRIC_PORTS) ||
		(SOC_SBX_STATE(unit)->port_state->subport_info[active_port].valid == FALSE)) {
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }

	    SHR_BITSET(subportmask, active_port);
	} else if(BCM_GPORT_IS_MODPORT(active_gport)) {
	    active_port = BCM_GPORT_MODPORT_PORT_GET(active_gport);

	    for (subport = 0; subport <  SB_FAB_DEVICE_MAX_FABRIC_PORTS; subport++) {
		rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, &offset);
		if ((rv == BCM_E_NONE) && (port == active_port)) {
		    SHR_BITSET(subportmask, subport);
		}
	    }
	    rv = BCM_E_NONE;
	}

	/* search failover database, if there is a superset, we found a match */
	for (failover = 0; failover < num_failovers; failover++) {
	    fo_state = &(SOC_SBX_STATE(unit)->failover_state[failover]);
	    if ((fo_state->state & BCM_SBX_FAILOVER_RESERVED) &&
		(fo_state->state & BCM_SBX_FAILOVER_INUSE) &&
		 ((fo_state->state & BCM_SBX_FAILOVER_APS) == 0)) {
		SHR_BITAND_RANGE(subportmask, fo_state->subportmask, 0,
				  SB_FAB_DEVICE_MAX_FABRIC_PORTS, tempmask);

		if (SHR_BITEQ_RANGE(subportmask, tempmask, 0,  SB_FAB_DEVICE_MAX_FABRIC_PORTS)) {
		    found = TRUE;
		    *failover_id = failover;
		    *protect_gport = fo_state->protect_gport;
		    break;
		}
	    }
	}
    }

    if (!found) {
	/* If not found, report it */
	rv = BCM_E_NOT_FOUND;
    }

    BCM_SBX_UNLOCK(unit);
    return(rv);

}

int
bcm_sbx_fabric_distribution_control_set(int unit, bcm_fabric_distribution_t ds_id,
					bcm_fabric_distribution_control_t type,
					int value)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (ds_id >= ds_state_p[unit]->max_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid DsId, Unit(%d), DsId(%d) MaxDsId(%d)\n"),
                   FUNCTION_NAME(), unit, ds_id, ds_state_p[unit]->max_ds_ids));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    switch (type) {
	case  bcmFabricDistributionSched:

	    rc = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_distribution_control_set, 
				       (unit, ds_id, type, value)));

	    if (rc == BCM_E_NONE) {
		ds_state_p[unit]->mc_full_eval_min_state[ds_id] = value;
	    }
	    break;
        default:
            BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
    }

    BCM_SBX_UNLOCK(unit);
    return rc;
}

int
bcm_sbx_fabric_port_create(int unit,
                    bcm_gport_t parent_port,
                    int offset,
                    uint32 flags,
                    bcm_gport_t *port)
{
    int rv = BCM_E_UNAVAIL;
    int modid, my_modid, subport = -1, egress = FALSE;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (BCM_GPORT_IS_MODPORT(parent_port) || BCM_GPORT_IS_EGRESS_MODPORT(parent_port)) {
        /* target modid */
	if (BCM_GPORT_IS_MODPORT(parent_port)) {
	    modid = BCM_GPORT_MODPORT_MODID_GET(parent_port);
	    subport = BCM_GPORT_CHILD_PORT_GET(*port);
    	} else {
	    modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(parent_port);
	    subport = BCM_GPORT_EGRESS_CHILD_PORT_GET(*port);
	    egress = TRUE;
	}
        if (modid < BCM_MODULE_FABRIC_BASE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: gport modid invalid(%d) minimum (%d)\n"),
                       modid, BCM_MODULE_FABRIC_BASE));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        /* unit's modid */
        rv = bcm_stk_modid_get(unit, &my_modid);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Fail to get modid for unit (%d)\n"),
                       unit));
            BCM_SBX_UNLOCK(unit);
            return(rv);
        }

        /* modid have to match */
        if (modid != my_modid) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Target modid %d not match unit modid %d\n"),
                       modid, my_modid));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: unsupported parent gport (0x%x) for fabric port creation (%d)\n"),
                   parent_port, rv));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_port_create, (unit, parent_port, offset, flags, &subport));
    if ((rv != BCM_E_NONE) && (rv != BCM_E_EXISTS)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port create failed error(%d)\n"),
                   rv));
    } else {
	/* return handle */
	if (egress == TRUE) {
	    BCM_GPORT_EGRESS_CHILD_SET(*port, modid, subport);
	} else {
	    BCM_GPORT_CHILD_SET(*port, modid, subport);
	}
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_port_destroy(int unit,
                 bcm_gport_t port)
{
    int rv = BCM_E_UNAVAIL;

    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_port_destroy, (unit, port));
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: port destroy failed error(%d)\n"),
                   rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_distribution_control_get(int unit, bcm_fabric_distribution_t ds_id,
					bcm_fabric_distribution_control_t type,
					int *value)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (ds_id >= ds_state_p[unit]->max_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid DsId, Unit(%d), DsId(%d) MaxDsId(%d)\n"),
                   FUNCTION_NAME(), unit, ds_id, ds_state_p[unit]->max_ds_ids));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    switch (type) {
	case  bcmFabricDistributionSched:
            if (!(soc_feature(unit, soc_feature_arbiter) || soc_feature(unit, soc_feature_arbiter_capable))) {
                BCM_SBX_UNLOCK(unit);
	        return BCM_E_UNAVAIL;
            }
#if 1 /* cached value */
	    (*value) = ds_state_p[unit]->mc_full_eval_min_state[ds_id];
#else /* read from h/w */
            rc = (MBCM_SBX_DRIVER_CALL(unit, mbcm_fabric_distribution_control_get, 
			                                (unit, ds_id, type, value)));
#endif
            break;
        default:
            BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
    }


    BCM_SBX_UNLOCK(unit);
    return rc;
}

int
bcm_sbx_fabric_timeslot_burst_size_bytes_get(int unit, int src_node_type, 
                                             int dest_node_type, 
                                             int num_channels, 
                                             int *bytes_per_timeslot)
{
    int rv = BCM_E_NONE;
#ifdef BCM_SIRIUS_SUPPORT
    int els;
#endif

#ifdef BCM_QE2000_SUPPORT
    int bs_in_lines, unused;
#endif

    switch (src_node_type) {
    case SB_FAB_NODE_TYPE_QE2K:
#ifdef BCM_QE2000_SUPPORT
        rv = soc_qe2000_burst_size_lines_get(unit, &bs_in_lines, &unused, SOC_SBX_CFG(unit)->xbar_link_en);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Could not determine burst size for source node \n")));
        } else {
            *bytes_per_timeslot = bs_in_lines * 16;
        }
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;
    case SB_FAB_NODE_TYPE_SIRIUS_FIC:
#ifdef BCM_SIRIUS_SUPPORT
        els = ((SOC_SBX_CFG(unit)->uRedMode == bcmFabricRedELS) || 
               (SOC_SBX_CFG(unit)->uRedMode == bcmFabricRed1Plus1ELS));
        rv = soc_sirius_ts_burst_size_bytes_get(unit, els, dest_node_type, 
                                             num_channels, bytes_per_timeslot);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Could not determine burst size for source node \n")));
        }
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Invalid source node type:%d. (Valid ones :%d or %d)\n"), 
                   src_node_type, SB_FAB_NODE_TYPE_QE2K, 
                   SB_FAB_NODE_TYPE_SIRIUS_FIC));
        rv = BCM_E_PARAM;
        break;
    }

    return rv;
}

int
bcm_sbx_fabric_congestion_size_set(int unit, bcm_module_t module_id, int max_ports)
{
    int rv = BCM_E_NONE;

    if (!soc_feature(unit, soc_feature_higig2)) {
	return(BCM_E_UNAVAIL);
    }

    BCM_SBX_LOCK(unit);
    
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_congestion_size_set,
                              (unit,
                              module_id,
                              max_ports));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_fabric_congestion_size_get(int unit, bcm_module_t module_id, int *max_ports)
{
    int rv = BCM_E_NONE;

    if (!soc_feature(unit, soc_feature_higig2)) {
        return(BCM_E_UNAVAIL);
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_congestion_size_get,
                              (unit,
                              module_id,
                              max_ports));
    BCM_SBX_UNLOCK(unit);

    return rv;
}

int
bcm_sbx_fabric_ds_id_to_ef_ds_id(int unit, bcm_fabric_distribution_t ds_id,
                                           bcm_fabric_distribution_t *ef_ds_id_p)
{
    int rv = BCM_E_UNAVAIL;


    return rv;
}

int
bcm_sbx_fabric_dsid_to_nef_dsid(int unit, bcm_fabric_distribution_t ds_id,
                                          bcm_fabric_distribution_t *nef_ds_id_p)
{
    int rv = BCM_E_UNAVAIL;


    return rv;
}

/* Create or update a fabric predicate */
int
bcm_sbx_fabric_predicate_create(int unit,
                                bcm_fabric_predicate_info_t *pred_info,
                                bcm_fabric_predicate_t *pred_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if (!pred_id) {
        return BCM_E_PARAM;
    }
    if (!pred_info) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_create,
                              (unit,
                              pred_info,
                              pred_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroys an existing predicate */
int
bcm_sbx_fabric_predicate_destroy(int unit,
                                 bcm_fabric_predicate_t pred_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_destroy,
                              (unit,
                              pred_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroys all fabric predicates on the unit */
int
bcm_sbx_fabric_predicate_destroy_all(int unit)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_destroy_all,
                              (unit));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Retrieve the information about a specific predicate */
int
bcm_sbx_fabric_predicate_get(int unit,
                             bcm_fabric_predicate_t pred_id,
                             bcm_fabric_predicate_info_t *pred_info)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if (!pred_info) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_get,
                              (unit,
                              pred_id,
                              pred_info));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Traverse the existing predicates, and invoke an application provided
 * callback for each one
 */
int
bcm_sbx_fabric_predicate_traverse(int unit,
                                  bcm_fabric_predicate_traverse_cb cb,
                                  void *user_data)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_traverse,
                              (unit,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Create or update a fabric action */
int
bcm_sbx_fabric_action_create(int unit,
                             bcm_fabric_action_info_t *action_info,
                             bcm_fabric_action_t *action_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if (!action_id) {
        return BCM_E_PARAM;
    }
    if (!action_info) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_action_create,
                              (unit,
                              action_info,
                              action_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroy a fabric action */
int
bcm_sbx_fabric_action_destroy(int unit,
                              bcm_fabric_action_t action_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_action_destroy,
                              (unit,
                              action_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroy all fabric actions on the unit */
int
bcm_sbx_fabric_action_destroy_all(int unit)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_action_destroy_all,
                              (unit));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Get information about a fabric action */
int
bcm_sbx_fabric_action_get(int unit,
                          bcm_fabric_action_t action_id,
                          bcm_fabric_action_info_t *action_info)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if (!action_info) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_action_get,
                              (unit,
                              action_id,
                              action_info));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Traverse the existing actions, and invoke an application provided
 * callback for each one
 */
int
bcm_sbx_fabric_action_traverse(int unit,
                               bcm_fabric_action_traverse_cb cb,
                               void *user_data)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_action_traverse,
                              (unit,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* This creates or updates a qsel */
int
bcm_sbx_fabric_qsel_create(int unit,
                           uint32 flags,
                           int base,
                           int count,
                           bcm_fabric_qsel_t *qsel_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if (!qsel_id) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_create,
                              (unit,
                              flags,
                              base,
                              count,
                              qsel_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* This destroys a qsel */
int
bcm_sbx_fabric_qsel_destroy(int unit,
                            bcm_fabric_qsel_t qsel_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_destroy,
                              (unit,
                              qsel_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroys all qsel on the unit */
int
bcm_sbx_fabric_qsel_destroy_all(int unit)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_destroy_all,
                              (unit));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* This gets information about a qsel */
int
bcm_sbx_fabric_qsel_get(int unit,
                        bcm_fabric_qsel_t qsel_id,
                        uint32 *flags,
                        int *base,
                        int *count)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if ((!flags) || (!base) || (!count)) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_get,
                              (unit,
                              qsel_id,
                              flags,
                              base,
                              count));
    BCM_SBX_UNLOCK(unit);
    return rv;
}
/*
 * Traverse existing fabric qsels, calling the provided callback one time
 * per existing qsel
 */
int
bcm_sbx_fabric_qsel_traverse(int unit,
                             bcm_fabric_qsel_traverse_cb cb,
                             void *user_data)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_traverse,
                              (unit,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Sets an entry within a qsel.  To effectively clear an entry, provide
 * BCM_GPORT_INVALID as the queue.
 */
int bcm_sbx_fabric_qsel_entry_set(int unit,
                                  bcm_fabric_qsel_t qsel_id,
                                  int offset,
                                  bcm_gport_t queue,
                                  bcm_fabric_qsel_offset_t qsel_offset_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_entry_set,
                              (unit,
                              qsel_id,
                              offset,
                              queue,
                              qsel_offset_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Gets an entry within a qsel.  Entries which are not in use will return
 * BCM_GPORT_INVALID as their queue.
 */
int
bcm_sbx_fabric_qsel_entry_get(int unit,
                              bcm_fabric_qsel_t qsel_id,
                              int offset,
                              bcm_gport_t *queue,
                              bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if ((!queue) || (!qsel_offset_id)) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_entry_get,
                              (unit,
                              qsel_id,
                              offset,
                              queue,
                              qsel_offset_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Sets a group of entries within a qsel.  To effectively clear entries,
 * set their respective queues to BCM_GPORT_INVALID.
 */
int
bcm_sbx_fabric_qsel_entry_multi_set(int unit,
                                    bcm_fabric_qsel_t qsel_id,
                                    int offset,
                                    int count,
                                    bcm_gport_t *queue,
                                    bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if ((!queue) || (!qsel_offset_id)) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_entry_multi_set,
                              (unit,
                              qsel_id,
                              offset,
                              count,
                              queue,
                              qsel_offset_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Gets a group of entries within a qsel.  Entries that are not in use
 * will have BCM_GPORT_INVALID as their queue.
 */
int
bcm_sbx_fabric_qsel_entry_multi_get(int unit,
                                    bcm_fabric_qsel_t qsel_id,
                                    int offset,
                                    int count,
                                    bcm_gport_t *queue,
                                    bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    if ((!queue) || (!qsel_offset_id)) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_entry_multi_get,
                              (unit,
                              qsel_id,
                              offset,
                              count,
                              queue,
                              qsel_offset_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Traverse existing entries within a specified qsel, calling the
 * provided callback one time per existing entry
 */
int
bcm_sbx_fabric_qsel_entry_traverse(int unit,
                                   bcm_fabric_qsel_t qsel_id,
                                   bcm_fabric_qsel_entry_traverse_cb cb,
                                   void *user_data)
{
    int rv;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_entry_traverse,
                              (unit,
                              qsel_id,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* This creates or updates a qsel_offset */
int
bcm_sbx_fabric_qsel_offset_create(int unit,
                                  uint32 flags,
                                  bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_create,
                              (unit,
                              flags,
                              qsel_offset_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* This destroys a qsel_offset */
int
bcm_sbx_fabric_qsel_offset_destroy(int unit,
                                   bcm_fabric_qsel_offset_t qsel_offset_id)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_destroy,
                              (unit,
                              qsel_offset_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroys all qsel_offset on the unit */
int
bcm_sbx_fabric_qsel_offset_destroy_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_destroy_all,
                              (unit));
    BCM_SBX_UNLOCK(unit);
    return rv;
}


/*
 * Traverse existing qsel_offsets, calling the provided callback one time
 * per existing qsel_offset
 */
int
bcm_sbx_fabric_qsel_offset_traverse(int unit,
                                    bcm_fabric_qsel_offset_traverse_cb cb,
                                    void *user_data)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_traverse,
                              (unit,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}


/* Set an entry of a qsel_offset */
int
bcm_sbx_fabric_qsel_offset_entry_set(int unit,
                                     bcm_fabric_qsel_offset_t qsel_offset_id,
                                     bcm_cos_t int_pri,
                                     int offset)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_entry_set,
                              (unit,
                              qsel_offset_id,
                              int_pri,
                              offset));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Get an entry of a qsel_offset */
int
bcm_sbx_fabric_qsel_offset_entry_get(int unit,
                                     bcm_fabric_qsel_offset_t qsel_offset_id,
                                     bcm_cos_t int_pri,
                                     int *offset)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_entry_get,
                              (unit,
                              qsel_offset_id,
                              int_pri,
                              offset));
    BCM_SBX_UNLOCK(unit);
    return rv;
}


/*
 * Traverse entries in a qsel_offset, calling the provided callback one
 * time per existing entry
 */
int
bcm_sbx_fabric_qsel_offset_entry_traverse(int unit,
                                          bcm_fabric_qsel_offset_t qsel_offset_id,
                                          bcm_fabric_qsel_offset_entry_traverse_cb cb,
                                          void *user_data)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_qsel_offset_entry_traverse,
                              (unit,
                              qsel_offset_id,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Create or update a fabric action */
int
bcm_sbx_fabric_predicate_action_create(int unit,
                                       bcm_fabric_predicate_action_info_t *predicate_action,
                                       bcm_fabric_predicate_action_t *predicate_action_id)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_action_create,
                              (unit,
                              predicate_action,
                              predicate_action_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Read a fabric predicate action */
int
bcm_sbx_fabric_predicate_action_get(int unit,
                                    bcm_fabric_predicate_action_t predicate_action_id,
                                    bcm_fabric_predicate_action_info_t *predicate_action_info)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_action_get,
                              (unit,
                              predicate_action_id,
                              predicate_action_info));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroy a fabric predicate action */
int
bcm_sbx_fabric_predicate_action_destroy(int unit,
                                        bcm_fabric_predicate_action_t predicate_action_id)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_action_destroy,
                              (unit,
                              predicate_action_id));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/* Destroy all fabric predicate actions on the unit */
int bcm_sbx_fabric_predicate_action_destroy_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_action_destroy_all,
                              (unit));
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Traverse existing fabric predicate actions, calling the provided
 * callback one time per existing fabric predicate action
 */
int
bcm_sbx_fabric_predicate_action_traverse(int unit,
                                         bcm_fabric_predicate_action_traverse_cb cb,
                                         void *user_data)
{
    int rv = BCM_E_UNAVAIL;
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) {
        return BCM_E_UNIT;
    }
    BCM_SBX_LOCK(unit);
    rv = MBCM_SBX_DRIVER_CALL(unit,
                              mbcm_fabric_predicate_action_traverse,
                              (unit,
                              cb,
                              user_data));
    BCM_SBX_UNLOCK(unit);
    return rv;
}


