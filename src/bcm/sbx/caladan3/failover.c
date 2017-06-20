/*
 * $Id: failover.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Caladan3 failover API
 */

#include <soc/sbx/sbx_drv.h>

#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>

#include <bcm/types.h>
#include <bcm/failover.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/allocator.h>

#include <bcm/module.h>

typedef struct caladan3_failover_wb_mem_layout_s {
    uint8  fo_v[(SBX_PROTECTION_END + 1) / 8]; /* fo_idx valid   */
    uint8  fo_e[(SBX_PROTECTION_END + 1) / 8]; /* fo_idx enabled */
} caladan3_failover_wb_mem_layout_t;


typedef struct caladan3_failover_state_s {
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 wb_hdl;
    uint32 wb_size;
#else
    uint32 dummy;
#endif /* BCM_WARM_BOOT_SUPPORT */
} caladan3_failover_state_t;


#define FO_WB_VERSION_1_0        SOC_SCACHE_VERSION(1,0)
#define FO_WB_CURRENT_VERSION    FO_WB_VERSION_1_0


caladan3_failover_state_t  fo_state[BCM_MAX_NUM_UNITS];

#ifdef BCM_WARM_BOOT_SUPPORT

#define FO_WB_AVAIL(_u)  (fo_state[_u].wb_size > 0)

#define WB_FO_VAR_DEREF(lo__, var__, idx__)  (lo__)->var__[(idx__) / 8]

#define WB_FO_VAR_GET(lo__, var__, idx__) \
    (!!  (WB_FO_VAR_DEREF(lo__, var__, idx__)  & (1 << ((idx__) & 7))))

#define WB_FO_VAR_SET(lo__, var__, idx__, val__) \
    WB_FO_VAR_DEREF(lo__, var__, idx__) =                                  \
        ((WB_FO_VAR_DEREF(lo__, var__, idx__) & ~(1 << ((idx__) & 7)))  |  \
         (!!(val__) << ((idx__) & 7)));

#define WB_FO_ENABLE_SET(lo__, idx__, val__ )  \
    WB_FO_VAR_SET(lo__, fo_e, idx__, val__)
#define WB_FO_ENABLE_GET(lo__, idx__ )  \
    WB_FO_VAR_GET(lo__, fo_e, idx__)

#define WB_FO_VALID_SET(lo__, idx__, val__ )  \
    WB_FO_VAR_SET(lo__, fo_v, idx__, val__)
#define WB_FO_VALID_GET(lo__, idx__ )  \
    WB_FO_VAR_GET(lo__, fo_v, idx__)

#else /* BCM_WARM_BOOT_SUPPORT */

#define FO_WB_AVAIL(_u) 0

#endif /* BCM_WARM_BOOT_SUPPORT */


int bcm_caladan3_failover_create(int unit, uint32 flags, 
                               bcm_failover_t *failover_id);

#if 0
    /* Hierarchical failover not currently supported */
int
_bcm_caladan3_failover_hier_set(int unit, bcm_failover_t failover_id, int enable)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_rr_hier_t rr_hier;

#if 0 
    rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                failover_id);
    if (rv != BCM_E_EXISTS) {
        return rv;
    }
#endif
    rr_hier.backup = enable;
    rv = soc_sbx_g2p3_rr_hier_set(unit, failover_id, &rr_hier);
    return rv;
}

int 
_bcm_caladan3_failover_hier_get(int unit, bcm_failover_t failover_id, int *enable)
{
    int rv = BCM_E_NONE;
    soc_sbx_g2p3_rr_hier_t rr_hier;

#if 0
    rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                failover_id);
    if (rv != BCM_E_EXISTS) {
        return rv;
    }
#endif
    rv = soc_sbx_g2p3_rr_hier_get(unit, failover_id, &rr_hier);
    *enable = rr_hier.backup;
    
    return rv;
}
#endif


#ifdef BCM_WARM_BOOT_SUPPORT
static int
caladan3_failover_wb_layout_get(int unit, soc_scache_handle_t hdl,
                            caladan3_failover_wb_mem_layout_t **layout)
{
    uint32 size;
    soc_wb_cache_t *wbc;

    *layout = NULL;
    BCM_IF_ERROR_RETURN(
        soc_scache_ptr_get(unit, hdl, (uint8**)&wbc, &size));

    *layout = (caladan3_failover_wb_mem_layout_t*)wbc->cache;
    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


int 
bcm_caladan3_failover_init(int unit)
{
    int rv = BCM_E_NONE;

#ifdef BCM_WARM_BOOT_SUPPORT
    int fo_idx, upgrade;
    caladan3_failover_wb_mem_layout_t *layout;

    upgrade = 0;
    sal_memset(&fo_state[unit], 0, sizeof(fo_state[0]));
    
    SOC_SCACHE_HANDLE_SET(fo_state[unit].wb_hdl, unit, BCM_MODULE_FAILOVER, 0);
    fo_state[unit].wb_size = (sizeof(caladan3_failover_wb_mem_layout_t) +
                              SOC_WB_SCACHE_CONTROL_SIZE);
    
    rv = soc_wb_state_alloc_and_check(unit, fo_state[unit].wb_hdl,
                                      &fo_state[unit].wb_size,
                                      FO_WB_CURRENT_VERSION,
                                      &upgrade);
    if (upgrade) {
        return BCM_E_UNAVAIL;
    }
    if (rv == BCM_E_UNAVAIL) {
        rv = BCM_E_NONE;
        fo_state[unit].wb_size = 0;
    }
    if (rv == BCM_E_EXISTS) {
        rv = BCM_E_NONE;
    }

    /* Nothing more to do for cold boot */
    if (SOC_WARM_BOOT(unit) == FALSE || 
        FO_WB_AVAIL(unit)            ||
        BCM_FAILURE(rv)) {
        return rv;
    }

    /* This is a Warm boot, and there is an scache available to recover */
    rv = caladan3_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    for (fo_idx = SBX_PROTECTION_BASE; 
         fo_idx <= SBX_PROTECTION_END;
         fo_idx ++) {

        if (WB_FO_VALID_GET(layout, fo_idx)) {
            rv = bcm_caladan3_failover_create(unit, 
                                            BCM_FAILOVER_WITH_ID, &fo_idx);
            if (BCM_FAILURE(rv)) {
                return rv;
            }
        }
    }
    
#endif /* BCM_WARM_BOOT_SUPPORT */
    return rv;
}

/*
 * Set a failover object to enable or disable (note that failover object
 * 0 is reserved.
 */
int 
bcm_caladan3_failover_set(int unit, bcm_failover_t failover_id, int enable)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_rrt_t rr;
    int rr_size  COMPILER_ATTRIBUTE((unused));

    rr_size = soc_sbx_g3p1_rrt_table_size_get(unit);

#if 0
    /* Hierarchical failover not currently supported */
    if (failover_id > rr_size) {
        failover_id = failover_id - rr_size;
        return _bcm_caladan3_failover_hier_set(unit, failover_id, enable);
    }
#endif
    rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                failover_id);
    if (rv != BCM_E_EXISTS) {
        return rv;
    }

    rr.backup = enable;
    rv = soc_sbx_g3p1_rrt_set(unit, failover_id, &rr);


#ifdef BCM_WARM_BOOT_SUPPORT

    if (FO_WB_AVAIL(unit)) {
        caladan3_failover_wb_mem_layout_t *layout;

        soc_scache_handle_lock(unit, fo_state[unit].wb_hdl);

        rv = caladan3_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
        if (BCM_FAILURE(rv)) {
            soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
            return rv;
        }

        WB_FO_ENABLE_SET(layout, failover_id, enable);

        soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}


/* Get the enable status of a failover object. */
int 
bcm_caladan3_failover_get(int unit, bcm_failover_t failover_id, int *enable)
{
    int rv = BCM_E_NONE;
    soc_sbx_g3p1_rrt_t rr;
    int rr_size  COMPILER_ATTRIBUTE((unused));

    rr_size = soc_sbx_g3p1_rrt_table_size_get(unit);

#if 0
    /* Hierarchical failover not currently supported */
    if (failover_id > rr_size) {
        failover_id = failover_id - rr_size;
        return _bcm_caladan3_failover_hier_get(unit, failover_id, enable);
    }
#endif

    rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                failover_id);
    if (rv != BCM_E_EXISTS) {
        return rv;
    }

    rv = soc_sbx_g3p1_rrt_get(unit, failover_id, &rr);
    *enable = rr.backup;
    
    return rv;
}

int 
bcm_caladan3_failover_create(int unit, uint32 flags, bcm_failover_t *failover_id)
{

    int rv = BCM_E_NONE;
    int alloc_flags = 0;

    if (failover_id == NULL
        || (flags
            && flags != BCM_FAILOVER_WITH_ID
            && flags != BCM_FAILOVER_REPLACE)) {
        return BCM_E_PARAM;
    }

    if (flags & BCM_FAILOVER_WITH_ID) {
        rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                    *failover_id);
        if (rv == BCM_E_NOT_FOUND) {
            return rv;
        }
        rv = BCM_E_NONE;
    }

    if (flags & BCM_FAILOVER_WITH_ID) {
        alloc_flags = _SBX_CALADAN3_RES_FLAGS_RESERVE;
    }
    
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                 1, (uint32 *) failover_id, alloc_flags);

    if (rv == BCM_E_RESOURCE) {
        /* Id is managed externally */
        rv = BCM_E_NONE;
    }

    if (BCM_SUCCESS(rv)) {
        rv = bcm_caladan3_failover_set(unit, *failover_id, 0);
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rv) && FO_WB_AVAIL(unit)) {
        caladan3_failover_wb_mem_layout_t *layout;
        soc_scache_handle_lock(unit, fo_state[unit].wb_hdl);

        rv = caladan3_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
        if (BCM_FAILURE(rv)) {
            soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
            return rv;
        }

        WB_FO_VALID_SET(layout, *failover_id, 1);
        WB_FO_ENABLE_SET(layout, *failover_id, 0);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}


int 
bcm_caladan3_failover_destroy(int unit, bcm_failover_t failover_id)
{
    int rv = BCM_E_NONE;

    rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                failover_id);
    if (rv == BCM_E_EXISTS) {
        rv = bcm_caladan3_failover_set(unit, failover_id, 0);
        rv = _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_PROTECTION, 1,
                                    (uint32 *)&failover_id, 0);
    }


#ifdef BCM_WARM_BOOT_SUPPORT

    if (FO_WB_AVAIL(unit)) {
        caladan3_failover_wb_mem_layout_t *layout;

        soc_scache_handle_lock(unit, fo_state[unit].wb_hdl);

        rv = caladan3_failover_wb_layout_get(unit,fo_state[unit].wb_hdl, &layout);
        if (BCM_FAILURE(rv)) {
            soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
            return rv;
        }

        WB_FO_VALID_SET(layout, failover_id, 0);

        soc_scache_handle_unlock(unit, fo_state[unit].wb_hdl);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    return rv;
}

