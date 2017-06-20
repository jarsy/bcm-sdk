/* $SDK/src/bcm/sbx/caladan3/wb_db_mirror.c */
/*
 * $Id: wb_db_mirror.c,v Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: MIRROR APIs
 *
 * Purpose:
 *     Warm boot support for MIRROR API for Caladan3 Packet Processor devices
 */

#include <shared/bsl.h>
#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <bcm_int/common/debug.h>
#include <sal/core/sync.h>

#define _SBX_CALADAN3_MIRROR_H_NEEDED_ TRUE

#include <bcm_int/sbx/caladan3/mirror.h>
#include <soc/sbx/wb_db_cmn.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_mirror.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_FP

/*
 *  Set _BCM_CALADAN3_MIRROR_WARMBOOT_WRITE_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time the MIRROR backing store is written.
 */
#define _BCM_CALADAN3_MIRROR_WARMBOOT_WRITE_TRACKING FALSE

/*
 *  Set _BCM_CALADAN3_MIRROR_WARMBOOT_READ_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time MIRROR backing store is read.
 */
#define _BCM_CALADAN3_MIRROR_WARMBOOT_READ_TRACKING FALSE

/*
 *  Externs for MIRROR
 */


/*
 *  Locals for MIRROR
 */
static bcm_caladan3_wb_mirror_state_scache_info_t 
        *_bcm_caladan3_wb_mirror_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };




/*
 *  Defines for MIRROR
 */
#define SBX_SCACHE_INFO_PTR(unit) \
                (_bcm_caladan3_wb_mirror_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) \
      ((_bcm_caladan3_wb_mirror_state_scache_info_p[unit]) != NULL \
      && (_bcm_caladan3_wb_mirror_state_scache_info_p[unit]->init_done == TRUE))




/*
 *  Warmboot APIs implementation for MIRROR
 */
STATIC int
_bcm_caladan3_wb_mirror_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_mirror_state_scache_info_p[unit], 
                sizeof (bcm_caladan3_wb_mirror_state_scache_info_t),
                "Scache for MIRROR warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_mirror_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_mirror_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}


STATIC int
_bcm_caladan3_wb_mirror_state_layout_init (int           unit,
                                          int           version,
                                          unsigned int *scache_len)
{
    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR (BSL_LS_BCM_FP,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_MIRROR_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;

            /* Layout for scache length and offset calculation */

            SBX_WB_DB_LAYOUT_INIT(_caladan3_mirror_t,
                _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_max,
                _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors);
            SBX_WB_DB_LAYOUT_INIT(_caladan3_mirror_t,
                _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_max,
                _mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors);

            /* Update scache length */
            SBX_SCACHE_INFO_PTR(unit)->scache_len = *scache_len;
            break;
        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_mirror_state_sync (int unit)
{
    int version   = 0;
    uint8 *scache_ptr_orig = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE) {
        LOG_ERROR (BSL_LS_BCM_FP,
                   (BSL_META_U(unit,
                           "Warm boot scache not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR (BSL_LS_BCM_FP,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    switch(version)
    {
        case BCM_CALADAN3_WB_MIRROR_VERSION_1_0:

            SBX_WB_DB_SYNC_MEMORY(_caladan3_mirror_t,
                _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_max,
                _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors);
            SBX_WB_DB_SYNC_MEMORY(_caladan3_mirror_t,
                _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_max,
                _mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors);

            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_caladan3_wb_mirror_state_restore (int unit)
{
    int version      = 0;

    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR (BSL_LS_BCM_FP,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    switch(version)
    {
        case BCM_CALADAN3_WB_MIRROR_VERSION_1_0:
            /* Restore state from scache */
            
            SBX_WB_DB_RESTORE_MEMORY(_caladan3_mirror_t,
                _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirror_max,
                _mirror_glob[unit]->state[_caladan3_ingress_mirror].mirrors);
            SBX_WB_DB_RESTORE_MEMORY(_caladan3_mirror_t,
                _mirror_glob[unit]->state[_caladan3_egress_mirror].mirror_max,
                _mirror_glob[unit]->state[_caladan3_egress_mirror].mirrors);

            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_mirror_state_init (int unit)
{
    int    flags   = SOC_CALADAN3_SCACHE_DEFAULT;
    int    exists  = 0;
    uint16 version = BCM_CALADAN3_WB_MIRROR_VERSION_CURR;
    uint16 recovered_version   = 0;
    uint8 *scache_ptr          = NULL;
    unsigned int scache_len    = 0;
    soc_scache_handle_t handle = 0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit)) {
        _bcm_caladan3_wb_mirror_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mirror_state_scache_alloc (unit));
    if(SBX_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR (BSL_LS_BCM_FP,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_MIRROR, 0);

    if (SOC_WARM_BOOT (unit)) {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        _rv = bcm_caladan3_scache_ptr_get (unit,
                                           handle,
                                           socScacheRetrieve,
                                           flags,
                                          &scache_len,
                                          &scache_ptr,
                                           version,
                                          &recovered_version,
                                          &exists);
    
        if (BCM_FAILURE(_rv)) {
            LOG_ERROR (BSL_LS_BCM_FP,
                       (BSL_META_U(unit,
                                   "unable to get current warm boot state for"
                                   " unit %d MIRROR instance: %d (%s)\n"),
                        unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }

        LOG_VERBOSE (BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                "unit %d loading MIRROR state\n"),
                      unit));
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;

        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mirror_state_layout_init (unit,
                                                  version, &scache_len));

        if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len) {
            LOG_ERROR (BSL_LS_BCM_FP,
                       (BSL_META_U(unit,
                                   "Scache length %d is not same as "
                                   "stored length %d\n"),
                       scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
        }
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mirror_state_restore (unit));
        if (version != recovered_version) {
            /* set up layout for the preferred version */
            BCM_IF_ERR_EXIT (
                      _bcm_caladan3_wb_mirror_state_layout_init (unit,
                                                 version, &scache_len));
            LOG_VERBOSE (BSL_LS_BCM_FP,
                         (BSL_META_U(unit,
                                     "unit %d reallocate %d bytes warm boot "
                                     "backing store space\n"),
                          unit, scache_len));

             /* reallocate the warm boot space */
            _rv = bcm_caladan3_scache_ptr_get (unit,
                                               handle,
                                               socScacheRealloc,
                                               flags,
                                              &scache_len,
                                              &scache_ptr,
                                               version,
                                              &recovered_version,
                                              &exists);
            if (BCM_FAILURE(_rv)) {
                LOG_ERROR (BSL_LS_BCM_FP,
                           (BSL_META_U(unit,
                                     "unable to reallocate %d bytes warm boot "
                                "space for unit %d MIRROR instance: %d (%s)\n"),
                           scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
                BCM_EXIT;
            }
        }		/* if (version != recovered_version) */

        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_mirror_state_scache_info_p[unit]->init_done = TRUE;

    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_mirror_state_layout_init (unit, 
                                                     version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE (BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "unit %d MIRROR: allocate %d bytes warm boot "
                                 "backing store space\n"),
                      unit, scache_len));
        _rv = bcm_caladan3_scache_ptr_get (unit,
                                           handle,
                                           socScacheCreate,
                                           flags,
                                          &scache_len,
                                          &scache_ptr,
                                           version,
                                          &recovered_version,
                                          &exists);
        if (BCM_FAILURE(_rv)) {
            LOG_ERROR (BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d mirror instance: %d (%s)\n"),
                         scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_mirror_state_scache_info_p[unit]->init_done = TRUE;
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
