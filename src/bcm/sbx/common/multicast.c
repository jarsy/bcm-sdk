/*
 * $Id: multicast.c,v 1.26 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>

#include <soc/defs.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/multicast.h>
#include <bcm/l2.h>
#include <bcm/port.h>
#include <bcm/error.h>
#include <bcm/module.h>
#include <bcm/multicast.h>

#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/state.h>
#ifdef BCM_SIRIUS_SUPPORT
#include <bcm_int/sbx/sirius.h>
#endif /* BCM_SIRIUS_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
static int
bcm_sbx_wb_multicast_state_init(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

int
bcm_sbx_multicast_init(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* some SBX devices do not implement this call, but have MC support */
    rc = MBCM_SBX_DRIVER_MAYBE_CALL(unit, mbcm_multicast_init, (unit));

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rc)) {
	rc = bcm_sbx_wb_multicast_state_init(unit);
	if (rc != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: error in WarmBoot multicast state init \n"),
	               FUNCTION_NAME()));
	}
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_detach(int unit)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit]!=NULL) &&
        (mbcm_sbx_driver[unit]->mbcm_multicast_init == NULL) &&
        (mbcm_sbx_driver[unit]->mbcm_multicast_detach == NULL)) {
        /* implied init, so implied detach is not invalid here */
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_NONE);
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_detach, (unit));

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_create(int unit,
                         uint32 flags,
                         bcm_multicast_t *group)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_create == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_create(unit, flags, group);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_destroy(int unit, bcm_multicast_t group)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_destroy == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_destroy(unit, group);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_group_get(int unit,
                            bcm_multicast_t group,
                            uint32 *flags)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (!flags) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_group_get, (unit, group, flags));

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_group_traverse(int unit,
                                 bcm_multicast_group_traverse_cb_t cb,
                                 uint32 flags,
                                 void *user_data)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (!cb) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_group_traverse, (unit, cb, flags, user_data));

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_fabric_distribution_set(int unit, bcm_multicast_t group, bcm_fabric_distribution_t ds_id)
{
    int rv = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (ds_id >= SOC_SBX_CFG(unit)->num_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: ds_id(%d) out of range (0-%d) valid\n"),
                   ds_id, (SOC_SBX_CFG(unit)->num_ds_ids -1)));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_fabric_distribution_set, (unit, group, ds_id));

    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: multicast fabric distribution set failed for group(%d) ds_id(%d) error(%d)\n"),
                   group, ds_id, rv));
    }
    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_multicast_fabric_distribution_get(int unit, bcm_multicast_t group,  bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_NONE;

    if (!ds_id) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: required outbound pointer is NULL\n")));
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_multicast_fabric_distribution_get, (unit, group, ds_id));

    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: multicast fabric distribution get failed for group(%d) error(%d)\n"),
                   group, rv));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_multicast_egress_add(int unit,
                             bcm_multicast_t group,
                             bcm_gport_t port,
                             bcm_if_t encap_id)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_egress_add == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_add(unit, group, port, encap_id);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_egress_delete(int unit,
                                bcm_multicast_t group,
                                bcm_gport_t port,
                                bcm_if_t encap_id)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete(unit, group, port, encap_id);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_egress_subscriber_add(int unit,
                                        bcm_multicast_t group,
                                        bcm_gport_t port,
                                        bcm_if_t encap_id,
                                        bcm_gport_t subscriber_queue)
{
    int rc = BCM_E_NONE;

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        return BCM_E_UNAVAIL;
    }

    if ((NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_add)) {
        BCM_SBX_LOCK(unit);
        rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_add(unit,
                                                                         group,
                                                                         port,
                                                                         encap_id,
                                                                         subscriber_queue);
        BCM_SBX_UNLOCK(unit);
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_egress_subscriber_delete(int unit,
                                           bcm_multicast_t group,
                                           bcm_gport_t port,
                                           bcm_if_t encap_id,
                                           bcm_gport_t subscriber_queue)
{
    int rc = BCM_E_NONE;

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        return BCM_E_UNAVAIL;
    }

    if ((NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_delete)) {
        BCM_SBX_LOCK(unit);
        rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_delete(unit,
                                                                            group,
                                                                            port,
                                                                            encap_id,
                                                                            subscriber_queue);
        BCM_SBX_UNLOCK(unit);
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_egress_delete_all(int unit,
                                    bcm_multicast_t group)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete_all == NULL)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_delete_all(unit, group);

    BCM_SBX_UNLOCK(unit);

    return(rc);
}

int
bcm_sbx_multicast_egress_set(int unit,
                             bcm_multicast_t group,
                             int port_count,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if ((mbcm_sbx_driver[unit] == NULL) || (mbcm_sbx_driver[unit]->mbcm_multicast_egress_set == NULL)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (port_count == 0) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, invalid port_count parameter, Unit(%d), port_max: 0x%x\n"),
                   FUNCTION_NAME(), unit, port_count));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (port_array == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, port_array parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }
    if (encap_id_array == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, encap_id_array parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_set(unit, group, port_count, port_array, encap_id_array);

    BCM_SBX_UNLOCK(unit);

    return(rc);
}

int
bcm_sbx_multicast_egress_get(int unit,
                             bcm_multicast_t group,
                             int port_max,
                             bcm_gport_t *port_array,
                             bcm_if_t *encap_id_array,
                             int *port_count)
{
    int rc = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (!(soc_feature(unit, soc_feature_mc_group_ability))) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }
    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_egress_get == NULL) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_get(unit, group, port_max, port_array, encap_id_array, port_count);

    BCM_SBX_UNLOCK(unit);
    return(rc);
}

int
bcm_sbx_multicast_egress_subscriber_set(int unit,
                                        bcm_multicast_t group,
                                        int port_count,
                                        bcm_gport_t *port_array,
                                        bcm_if_t *encap_id_array,
                                        bcm_gport_t *subscriber_queue_array)
{
    int rc;

    if (soc_feature(unit, soc_feature_mc_group_ability) &&
        (NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_set)) {
        if ((0 < port_count) &&
            (NULL != port_array) &&
            (NULL != encap_id_array) &&
            (NULL != subscriber_queue_array)) {
            BCM_SBX_LOCK(unit);
            rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_set(unit,
                                                                             group,
                                                                             port_count,
                                                                             port_array,
                                                                             encap_id_array,
                                                                             subscriber_queue_array);
            BCM_SBX_UNLOCK(unit);
        } else {
            rc = BCM_E_PARAM;
        }
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}

int
bcm_sbx_multicast_egress_subscriber_get(int unit,
                                        bcm_multicast_t group,
                                        int port_max,
                                        bcm_gport_t *port_array,
                                        bcm_if_t *encap_id_array,
                                        bcm_gport_t *subscriber_queue_array,
                                        int *port_count)
{
    int rc;

    if (soc_feature(unit, soc_feature_mc_group_ability) &&
        (NULL != mbcm_sbx_driver[unit]) &&
        (NULL != mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_get)) {
        if ((NULL != port_count) &&
            ((0 == port_max) ||
             ((0 < port_max) &&
              (NULL != port_array) &&
              (NULL != encap_id_array) &&
              (NULL != subscriber_queue_array)))) {
            BCM_SBX_LOCK(unit);
            rc = mbcm_sbx_driver[unit]->mbcm_multicast_egress_subscriber_get(unit,
                                                                             group,
                                                                             port_max,
                                                                             port_array,
                                                                             encap_id_array,
                                                                             subscriber_queue_array,
                                                                             port_count);
            BCM_SBX_UNLOCK(unit);
        } else {
            rc = BCM_E_PARAM;
        }
    } else {
        rc = BCM_E_UNAVAIL;
    }

    return rc;
}


#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_sbx_multicast_get_state(int unit, char *pbuf)
{
    int rv;
    char *pbuf_current = pbuf;

    if (mbcm_sbx_driver[unit]==NULL || mbcm_sbx_driver[unit]->mbcm_multicast_state_get == NULL) {
        return(BCM_E_UNAVAIL);
    }

    rv = mbcm_sbx_driver[unit]->mbcm_multicast_state_get(unit, pbuf_current);

    return rv;
}

#endif /* EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* EASY_RELOAD_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
/* This function initializes the WB support for multicast module. 
 *      1. During cold-boot: allocates a stable cache
 *      2. During warm-boot: Recovers the cosq state from stable cache
 */

static int
bcm_sbx_wb_multicast_state_init(int unit)
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
                              "%s: Internal error. Invalid multicast state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }
    
    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MULTICAST, 0);
    
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
    end_ptr = scache_ptr + scache_len;

    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_wb_multicast_state_sync(unit, default_ver, recovered_ver,
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
 *      bcm_sbx_wb_multicast_state_sync
 * Purpose:
 *      Record Multicast module persisitent info for Level 2 Warm Boot
 *      Input param: sync --> indicates whether to sync scache to Persistent memory
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sbx_wb_multicast_state_sync(int unit, int sync)
{
    uint8                   *scache_ptr = NULL;
    uint8                   *ptr, *end_ptr;
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
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_MULTICAST, 0);

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
        rv = bcm_sirius_wb_multicast_state_sync(unit, default_ver, recovered_ver,
                                                NULL, &ptr, &end_ptr, _WB_OP_COMPRESS);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) wb multicast state sync. \n"),
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
bcm_sbx_wb_multicast_sw_dump(int unit)
{
    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        bcm_sirius_wb_multicast_state_sync(unit, BCM_WB_DEFAULT_VERSION,
					   BCM_WB_DEFAULT_VERSION,
					   NULL, NULL, NULL, _WB_OP_DUMP);
#endif
    }

    return;
}
#endif
