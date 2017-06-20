/* $SDK/src/bcm/sbx/caladan3/wb_db_field.c */
/*
 * $Id: wb_db_field.c,v Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: FIELD APIs
 *
 * Purpose:
 *     Warm boot support for FIELD API for Caladan3 Packet Processor devices
 */

#include <shared/bsl.h>
#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <bcm_int/common/debug.h>
#include <sal/core/sync.h>
#include <shared/idxres_fl.h>
#include <bcm/field.h>

#define _SBX_CALADAN3_FIELD_H_NEEDED_ TRUE

#include <bcm_int/sbx/caladan3/field.h>
#include <soc/sbx/wb_db_cmn.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_field.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_FP

/*
 *  Set _BCM_CALADAN3_FIELD_WARMBOOT_WRITE_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time the FIELD backing store is written.
 */
#define _BCM_CALADAN3_FIELD_WARMBOOT_WRITE_TRACKING FALSE

/*
 *  Set _BCM_CALADAN3_FIELD_WARMBOOT_READ_TRACKING to TRUE if you want diagnostics
 *  displayed at VERB level every time FIELD backing store is read.
 */
#define _BCM_CALADAN3_FIELD_WARMBOOT_READ_TRACKING FALSE

/*
 *  Externs for FIELD
 */

extern _sbx_caladan3_field_unit_info_t _sbx_caladan3_field[];

/*
 *  Locals for FIELD
 */
static bcm_caladan3_wb_field_state_scache_info_t 
        *_bcm_caladan3_wb_field_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };




/*
 *  Defines for FIELD
 */
#define SBX_SCACHE_INFO_PTR(unit) \
                (_bcm_caladan3_wb_field_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) \
      ((_bcm_caladan3_wb_field_state_scache_info_p[unit]) != NULL \
      && (_bcm_caladan3_wb_field_state_scache_info_p[unit]->init_done == TRUE))




/*
 *  Warmboot APIs implementation for FIELD
 */
STATIC int
_bcm_caladan3_wb_field_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_field_state_scache_info_p[unit], 
                sizeof (bcm_caladan3_wb_field_state_scache_info_t),
                "Scache for FIELD warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_field_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_field_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}


STATIC int
_bcm_caladan3_wb_field_state_layout_init (int           unit,
                                          int           version,
                                          unsigned int *scache_len)
{

    _bcm_caladan3_g3p1_field_glob_t *thisUnit;
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
        case BCM_CALADAN3_WB_FIELD_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;

            /* Layout for scache length and offset calculation */

            thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)_sbx_caladan3_field[unit].data;

            SBX_WB_DB_LAYOUT_INIT(_bcm_caladan3_g3p1_field_glob_t, 1, thisUnit);
            SBX_WB_DB_LAYOUT_INIT(_bcm_caladan3_g3p1_field_group_t,
                thisUnit->groupTotal, thisUnit->group);
            SBX_WB_DB_LAYOUT_INIT(_bcm_caladan3_g3p1_field_entry_t,
                thisUnit->entryTotal, thisUnit->entry);
            SBX_WB_DB_LAYOUT_INIT(_bcm_caladan3_field_range_t,
                thisUnit->rangeTotal, thisUnit->range);

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
bcm_caladan3_wb_field_state_sync (int unit)
{
    int version   = 0;
    uint8 *scache_ptr_orig = NULL;
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;

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
        case BCM_CALADAN3_WB_FIELD_VERSION_1_0:
            thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)_sbx_caladan3_field[unit].data;

            SBX_WB_DB_SYNC_VARIABLE(_bcm_caladan3_g3p1_field_glob_t, 1, *thisUnit);
            SBX_WB_DB_SYNC_MEMORY(_bcm_caladan3_g3p1_field_group_t,
                thisUnit->groupTotal, thisUnit->group);
            SBX_WB_DB_SYNC_MEMORY(_bcm_caladan3_g3p1_field_entry_t,
                thisUnit->entryTotal, thisUnit->entry);
            SBX_WB_DB_SYNC_MEMORY(_bcm_caladan3_field_range_t,
                thisUnit->rangeTotal, thisUnit->range);

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
_bcm_caladan3_wb_field_state_restore (int unit)
{
    int version      = 0;
    _bcm_caladan3_g3p1_field_glob_t *thisUnit;
    _bcm_caladan3_g3p1_field_glob_t glob;

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
        case BCM_CALADAN3_WB_FIELD_VERSION_1_0:
            /* Restore state from scache */
            thisUnit = (_bcm_caladan3_g3p1_field_glob_t*)_sbx_caladan3_field[unit].data;

            SBX_WB_DB_RESTORE_VARIABLE(_bcm_caladan3_g3p1_field_glob_t, 1, glob);
            SBX_WB_DB_RESTORE_MEMORY(_bcm_caladan3_g3p1_field_group_t,
                thisUnit->groupTotal, thisUnit->group);
            SBX_WB_DB_RESTORE_MEMORY(_bcm_caladan3_g3p1_field_entry_t,
                thisUnit->entryTotal, thisUnit->entry);
            SBX_WB_DB_RESTORE_MEMORY(_bcm_caladan3_field_range_t,
                thisUnit->rangeTotal, thisUnit->range);
            memcpy(&thisUnit->rulebase[0], &glob.rulebase[0], sizeof(glob.rulebase));
            thisUnit->groupFreeCount = glob.groupFreeCount;
            thisUnit->groupTotal     = glob.groupTotal;
            thisUnit->entryFreeCount = glob.entryFreeCount;
            thisUnit->entryTotal     = glob.entryTotal;
            thisUnit->groupFreeHead  = glob.groupFreeHead;
            thisUnit->dropPolicer    = glob.dropPolicer;
            thisUnit->uMaxSupportedStages = glob.uMaxSupportedStages;
            
            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_field_state_init (int unit)
{
    int    flags   = SOC_CALADAN3_SCACHE_DEFAULT;
    int    exists  = 0;
    uint16 version = BCM_CALADAN3_WB_FIELD_VERSION_CURR;
    uint16 recovered_version   = 0;
    uint8 *scache_ptr          = NULL;
    unsigned int scache_len    = 0;
    soc_scache_handle_t handle = 0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit)) {
        _bcm_caladan3_wb_field_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_field_state_scache_alloc (unit));
    if (SBX_SCACHE_INFO_PTR(unit) == NULL) {
        LOG_ERROR (BSL_LS_BCM_FP,
                   (BSL_META_U(unit,
                               "Warm boot not initialized for unit %d \n"),
                    unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_FIELD, 0);

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
                                   " unit %d FIELD instance: %d (%s)\n"),
                        unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }

        LOG_VERBOSE (BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                "unit %d loading FIELD state\n"),
                      unit));
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;

        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_field_state_layout_init (unit,
                                                  version, &scache_len));

        if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len) {
            LOG_ERROR (BSL_LS_BCM_FP,
                       (BSL_META_U(unit,
                                   "Scache length %d is not same as "
                                   "stored length %d\n"),
                       scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
        }
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_field_state_restore (unit));
        if (version != recovered_version) {
            /* set up layout for the preferred version */
            BCM_IF_ERR_EXIT (
                      _bcm_caladan3_wb_field_state_layout_init (unit,
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
                                "space for unit %d FIELD instance: %d (%s)\n"),
                           scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
                BCM_EXIT;
            }
        }		/* if (version != recovered_version) */

        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_field_state_scache_info_p[unit]->init_done = TRUE;

    } else {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_field_state_layout_init (unit, 
                                                     version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE (BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "unit %d FIELD: allocate %d bytes warm boot "
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
                                  " for unit %d field instance: %d (%s)\n"),
                         scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        }
        SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
        _bcm_caladan3_wb_field_state_scache_info_p[unit]->init_done = TRUE;
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
