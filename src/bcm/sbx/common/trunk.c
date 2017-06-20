/*
 * $Id: trunk.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    trunk.c
 * Purpose: BCM level APIs for trunking (a.k.a. Port Aggregation)
 */

#include <shared/bsl.h>

#include <soc/drv.h>

#include <bcm/module.h>
#include <bcm/error.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm/trunk.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
static int
bcm_sbx_wb_trunk_state_init(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

int
bcm_sbx_trunk_init(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
	return(BCM_E_UNAVAIL);
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_trunk_init, (unit));

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rc)) {
	rc = bcm_sbx_wb_trunk_state_init(unit);
	if (rc != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: error in WarmBoot trunk state init \n"),
	               FUNCTION_NAME()));
	}
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_detach(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_detach == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_detach(unit);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_create(int unit,
                     uint32 flags,
                     bcm_trunk_t *tid)
{
    int rc = BCM_E_UNAVAIL;

    if (!tid) {
        return BCM_E_PARAM;
    }
    BCM_SBX_LOCK(unit);
    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
    if (mbcm_sbx_driver[unit]) {
        if (mbcm_sbx_driver[unit]->mbcm_trunk_create) {
            rc = mbcm_sbx_driver[unit]->mbcm_trunk_create(unit, flags, tid);
        } else {
            if (flags & BCM_TRUNK_FLAG_WITH_ID) {
                if (mbcm_sbx_driver[unit]->mbcm_trunk_create_id) {
                    rc = mbcm_sbx_driver[unit]->mbcm_trunk_create_id(unit,
                                                                     *tid);
                }
            } else {
                if (mbcm_sbx_driver[unit]->mbcm_trunk_create_old) {
                    rc = mbcm_sbx_driver[unit]->mbcm_trunk_create_old(unit,
                                                                      tid);
                }
            }
        }
    }
    BCM_SBX_UNLOCK(unit);
    return rc;
}

int
bcm_sbx_trunk_chip_info_get(int unit,
                            bcm_trunk_chip_info_t *ta_info)
{
    int rc = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);
    if (soc_feature(unit, soc_feature_mc_group_ability)) {
        if ((mbcm_sbx_driver[unit]) &&
            (mbcm_sbx_driver[unit]->mbcm_trunk_chip_info_get)) {
            rc = mbcm_sbx_driver[unit]->mbcm_trunk_chip_info_get(unit, 
                                                                 ta_info);
        }
    }
    BCM_SBX_UNLOCK(unit);
    return rc;
}

int
bcm_sbx_trunk_set_old(int unit,
                  bcm_trunk_t tid,
                  bcm_trunk_add_info_t *add_info)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_set_old == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_set_old(unit, tid, add_info);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int 
bcm_sbx_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
        int member_count, bcm_trunk_member_t *member_array)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);
    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit]) &&
        (mbcm_sbx_driver[unit]->mbcm_trunk_set)) {
        rc = mbcm_sbx_driver[unit]->mbcm_trunk_set(unit,
                                                   tid,
                                                   trunk_info,
                                                   member_count,
                                                   member_array);
    } else {
        rc = BCM_E_UNAVAIL;
    }

    BCM_SBX_UNLOCK(unit);
    return(rc);
}  

int
bcm_sbx_trunk_destroy(int unit,
                      bcm_trunk_t tid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_destroy == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_destroy(unit, tid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_trunk_get_old(int unit,
                  bcm_trunk_t tid,
                  bcm_trunk_add_info_t *t_data)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_get_old == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_get_old(unit, tid, t_data);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int 
bcm_sbx_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
        int member_max, bcm_trunk_member_t *member_array, int *member_count)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);
    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit]) &&
        (mbcm_sbx_driver[unit]->mbcm_trunk_get)) {
        rc = mbcm_sbx_driver[unit]->mbcm_trunk_get(unit,
                                                   tid,
                                                   trunk_info,
                                                   member_max,
                                                   member_array,
                                                   member_count);
    } else {
        rc = BCM_E_UNAVAIL;
    }

    BCM_SBX_UNLOCK(unit);
    return(rc);
}     

int
bcm_sbx_trunk_find(int unit,
                   bcm_module_t modid,
                   bcm_port_t port,
                   bcm_trunk_t *tid)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_trunk_find == NULL) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_trunk_find(unit, modid, port, tid);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}


#ifdef BCM_WARM_BOOT_SUPPORT
/* This function initializes the WB support for trunk module. 
 *      1. During cold-boot: allocates a stable cache
 *      2. During warm-boot: Recovers the cosq state from stable cache
 */

static int
bcm_sbx_wb_trunk_state_init(int unit)
{
    int                  rv = BCM_E_NONE;
    uint8                *scache_ptr = NULL, *ptr, *end_ptr;
    soc_scache_handle_t  scache_handle;
    uint32               scache_len = 0;
    int                  stable_size;
    uint16               default_ver = BCM_WB_DEFAULT_VERSION;
    uint16               recovered_ver = BCM_WB_DEFAULT_VERSION;

    if (!SOC_IS_SIRIUS(unit)) {
        return BCM_E_NONE;
    }

    if (SOC_SBX_STATE(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid trunk state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRUNK, 0);

    if (SOC_WARM_BOOT(unit)) {
        /* Recover the state from scache */
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
    end_ptr = scache_ptr + scache_len;

    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_wb_trunk_state_sync(unit, default_ver, recovered_ver,
                                            &scache_len, &ptr, &end_ptr, 
                                            (SOC_WARM_BOOT(unit) ? _WB_OP_DECOMPRESS : _WB_OP_SIZE));
        if (rv != BCM_E_NONE) {
            return rv;
        }
#endif /* BCM_SIRIUS_SUPPORT */
    }      
       
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


/*
 * Function:
 *      bcm_sbx_wb_trunk_state_sync
 * Purpose:
 *      Record Trunk module persisitent info for Level 2 Warm Boot
 *      Input param: sync --> indicates whether to sync scache to Persistent memory
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sbx_wb_trunk_state_sync(int unit, int sync)
{
    uint8                   *scache_ptr = NULL, *ptr, *end_ptr;
    uint32                  scache_len;
    int                     stable_size;
    int                     rv = BCM_E_NONE;
    soc_scache_handle_t     scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;

    if (SOC_WARM_BOOT(unit)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Cannot write to SCACHE during WarmBoot\n")));
        return SOC_E_INTERNAL;
    }

    if (SOC_SBX_STATE(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid cosq state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_TRUNK, 0);
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
    end_ptr = scache_ptr + scache_len;

    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_wb_trunk_state_sync(unit, default_ver, recovered_ver,
                                            NULL, &ptr, &end_ptr, _WB_OP_COMPRESS);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) for wb trunk state sync. \n"),
                       FUNCTION_NAME(), soc_errmsg(rv)));
            return rv;
        }
#endif
    }

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

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_trunk_sw_dump(int unit)
{
    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        bcm_sirius_wb_trunk_state_sync(unit, BCM_WB_DEFAULT_VERSION,
				       BCM_WB_DEFAULT_VERSION,
				       NULL, NULL, NULL, _WB_OP_DUMP);
#endif					   
    }

    return;
}
#endif
