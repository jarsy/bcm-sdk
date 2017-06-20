/*
 * $Id: wb_db_trunk.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: TRUNK APIs
 *
 * Purpose:
 *     TRUNK API for Caladan3 Packet Processor devices
 *     Warm boot support
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>

#include <bcm_int/sbx/caladan3/trunk.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_trunk.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_TRUNK


/*
 *  Externs for TRUNK
 */
extern void _bcm_caladan3_trunk_debug(int unit);
extern trunk_cntl_t * bcm_sbx_caladan3_trunk_cntl_ptr_get(int unit);


/*
 *  Locals for TRUNK
 */
static bcm_caladan3_wb_trunk_state_scache_info_t *_bcm_caladan3_wb_trunk_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

/*
 *  Defines for TRUNK
 */
#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_trunk_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_trunk_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_trunk_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Warmboot APIs implementation for TRUNK
 */
STATIC int
_bcm_caladan3_wb_trunk_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_trunk_state_scache_info_p[unit], sizeof (bcm_caladan3_wb_trunk_state_scache_info_t), "Scache for TRUNK warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_trunk_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_trunk_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_trunk_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    int rc = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_TRUNK_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;
            
            /* Layout for scache lenght and offset calculation */
            SBX_WB_DB_LAYOUT_INIT(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->ngroups);
            SBX_WB_DB_LAYOUT_INIT(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->nports);
            SBX_WB_DB_LAYOUT_INIT(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->psc);
            SBX_WB_DB_LAYOUT_INIT(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->invalid_oi);
            SBX_WB_DB_LAYOUT_INIT(trunk_private_t, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->ngroups, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->t_info[0]);

            /* Update scache length */
            SBX_SCACHE_INFO_PTR(unit)->scache_len = *scache_len;
            break;
        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}


int
bcm_caladan3_wb_trunk_state_sync (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    uint8 *scache_ptr_orig = NULL;
    /* Local Variable for TRUNK*/
    bcm_trunk_t tid = 0;
    int max_tid = 0;
    trunk_private_t *ti;

    BCM_INIT_FUNC_DEFS;

    if(SOC_WARM_BOOT(unit))
    {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Sync not allowed during warmboot on unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE)
    {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Warm boot scache not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    switch(version)
    {
        case BCM_CALADAN3_WB_TRUNK_VERSION_1_0:
            /* Sync state to the scache */
            SBX_WB_DB_SYNC_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->ngroups);
            SBX_WB_DB_SYNC_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->nports);
            SBX_WB_DB_SYNC_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->psc);
            SBX_WB_DB_SYNC_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->invalid_oi);
            ti = bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->t_info;
            max_tid = bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->ngroups; 
            for (tid = 0; tid < max_tid; tid++)
            {
                SBX_WB_DB_SYNC_MEMORY(trunk_private_t, 1, ti);
                ti++;
            }
            
            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_TRUNK | BSL_INFO)) {
                _bcm_caladan3_trunk_debug(unit);
            }
            break;


        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_caladan3_wb_trunk_state_restore (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    bcm_trunk_t tid = 0;
    int max_tid = 0;
    trunk_private_t *ti;

    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    switch(version)
    {
        case BCM_CALADAN3_WB_TRUNK_VERSION_1_0:
            /* Restore state from scache */
            SBX_WB_DB_RESTORE_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->ngroups);
            SBX_WB_DB_RESTORE_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->nports);
            SBX_WB_DB_RESTORE_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->psc);
            SBX_WB_DB_RESTORE_VARIABLE(int, 1, bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->invalid_oi);

            ti = bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->t_info;
            max_tid = bcm_sbx_caladan3_trunk_cntl_ptr_get(unit)->ngroups; 
            for (tid = 0; tid < max_tid; tid++)
            {
                SBX_WB_DB_RESTORE_MEMORY(trunk_private_t, 1, ti);
                ti++;
            }

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_TRUNK | BSL_INFO)) {
                _bcm_caladan3_trunk_debug(unit);
            }
            break;


        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_trunk_state_init (int unit)
{
    int result = BCM_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_TRUNK_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit))
    {
        _bcm_caladan3_wb_trunk_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_trunk_state_scache_alloc (unit));
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_TRUNK, 0);

    if (SOC_WARM_BOOT (unit))
    {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRetrieve, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == result)
        {
            LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                        (BSL_META_U(unit,
                                    "unit %d loading TRUNK backing store state\n"),
                         unit));
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_trunk_state_layout_init (unit, version, &scache_len));
            if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len)
            {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Scache length %d is not same as stored length %d\n"),
                           scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
            }
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_trunk_state_restore (unit));
            if (BCM_E_NONE == result)
            {
                if (version != recovered_version)
                {
                    /* set up layout for the preferred version */
                    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_trunk_state_layout_init (unit, version, &scache_len));
                    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                                (BSL_META_U(unit,
                                            "unit %d reallocate %d bytes warm"
                                            " boot backing store space\n"),
                                 unit, scache_len));
                    /* reallocate the warm boot space */
                    result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRealloc, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
                    if (BCM_E_NONE != result)
                    {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "unable to reallocate %d bytes"
                                              " warm boot space for unit %d"
                                              " TRUNK instance: %d (%s)\n"),
                                   scache_len, unit, result, _SHR_ERRMSG (result)));
                        BCM_EXIT;
                    }
                    SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == result) */

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_trunk_state_scache_info_p[unit]->init_done = TRUE;

        }
        else
        {			/* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                  " unit %d TRUNK instance: %d (%s)\n"),
                       unit, result, _SHR_ERRMSG (result)));
        }			/* if (BCM_E_NONE == result) */
    }
    else
    {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_trunk_state_layout_init (unit, version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing"
                                " store space\n"),
                     unit, scache_len));
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheCreate, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE != result)
        {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, result, _SHR_ERRMSG (result)));
            BCM_EXIT;
        }
        else
        {
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_trunk_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
