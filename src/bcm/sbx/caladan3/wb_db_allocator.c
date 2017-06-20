/*
 * $Id: wb_db_allocator.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: ALLOCATOR APIs
 *
 * Purpose:
 *     ALLOCATOR API for Caladan3 Packet Processor devices
 *     Warm boot support
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_allocator.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_INIT


/*
 *  Externs for ALLOCATOR
 */

extern void _sbx_mac_tracker_dump(int unit);
extern int bcm_sbx_caladan3_global_ftType_dump(int unit);
extern int bcm_sbx_caladan3_global_vsiType_dump(int unit);
extern int soc_sbx_g3p1_allocator_reservation_shell_print(int unit);
extern int soc_sbx_allocator_shell_print(int unit);
extern void bcm_sbx_caladan3_ete_dump(int unit);

extern _sbx_smac_idx_tracker_t* bcm_sbx_caladan3_smac_tracker_ptr(int unit);
extern uint8 *_sbx_global_ftType[BCM_MAX_NUM_UNITS];
extern uint8 *_sbx_global_vsiType[BCM_MAX_NUM_UNITS];
extern int bcm_sbx_caladan3_global_vsiType_size(int unit, uint32 *size);
extern int bcm_sbx_caladan3_global_ftType_size(int unit, uint32 *size);

/*
 *  Locals for ALLOCATOR
 */
static bcm_caladan3_wb_allocator_state_scache_info_t *_bcm_caladan3_wb_allocator_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

/*
 *  Defines for ALLOCATOR
 */
#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_allocator_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_allocator_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_allocator_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Warmboot APIs implementation for ALLOCATOR
 */
STATIC int
_bcm_caladan3_wb_allocator_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_allocator_state_scache_info_p[unit], sizeof (bcm_caladan3_wb_allocator_state_scache_info_t), "Scache for ALLOCATOR warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_allocator_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_allocator_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_allocator_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    int rc = BCM_E_NONE;
    /* Local Variable for ALLOCATOR */
    uint32 size = 0;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_ALLOCATOR_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;
            
            /* Layout for scache lenght and offset calculation */
            SBX_WB_DB_LAYOUT_INIT(_sbx_track_mac_data_t, bcm_sbx_caladan3_smac_tracker_ptr(unit)->ismac.numMacs, 
                                                                bcm_sbx_caladan3_smac_tracker_ptr(unit)->ismac->macData[0]);
            SBX_WB_DB_LAYOUT_INIT(_sbx_track_mac_data_t, bcm_sbx_caladan3_smac_tracker_ptr(unit)->esmac.numMacs, 
                                                                bcm_sbx_caladan3_smac_tracker_ptr(unit)->esmac->macData[0]);
            BCM_IF_ERR_EXIT (bcm_sbx_caladan3_global_ftType_size(unit, &size));
            SBX_WB_DB_LAYOUT_INIT(uint8, size, _sbx_global_ftType[unit]);
            BCM_IF_ERR_EXIT (bcm_sbx_caladan3_global_vsiType_size(unit, &size));
            SBX_WB_DB_LAYOUT_INIT(uint8, size, _sbx_global_vsiType[unit]);
            SBX_WB_DB_LAYOUT_INIT(uint32, SBX_MAX_PORTS, SOC_SBX_PORT_ETE(unit, 0));
            SBX_WB_DB_LAYOUT_INIT(uint32, SBX_MAX_PORTS, SOC_SBX_PORT_UT_ETE(unit, 0));
            SBX_WB_DB_LAYOUT_INIT(uint32, SBX_MAX_TRUNKS, SOC_SBX_TRUNK_ETE(unit, 0));
            SBX_WB_DB_LAYOUT_INIT(uint32, 1, SOC_SBX_INVALID_L2ETE(unit));


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
bcm_caladan3_wb_allocator_state_sync (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    uint8 *scache_ptr_orig = NULL;
    /* Local Variable for ALLOCATOR */
    uint32 size = 0;
    int array_idx = 0;

    BCM_INIT_FUNC_DEFS;
    
    if(SOC_WARM_BOOT(unit))
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Sync not allowed during warmboot on unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE)
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot scache not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    switch(version)
    {
        case BCM_CALADAN3_WB_ALLOCATOR_VERSION_1_0:
            /* Sync state to the scache */
            SBX_WB_DB_SYNC_MEMORY(_sbx_track_mac_data_t, bcm_sbx_caladan3_smac_tracker_ptr(unit)->ismac.numMacs, 
                                                                bcm_sbx_caladan3_smac_tracker_ptr(unit)->ismac.macData);
            SBX_WB_DB_SYNC_MEMORY(_sbx_track_mac_data_t, bcm_sbx_caladan3_smac_tracker_ptr(unit)->esmac.numMacs, 
                                                                bcm_sbx_caladan3_smac_tracker_ptr(unit)->esmac.macData);
            BCM_IF_ERR_EXIT (bcm_sbx_caladan3_global_ftType_size(unit, &size));
            SBX_WB_DB_SYNC_MEMORY(uint8, size, _sbx_global_ftType[unit]);
            BCM_IF_ERR_EXIT (bcm_sbx_caladan3_global_vsiType_size(unit, &size));
            SBX_WB_DB_SYNC_MEMORY(uint8, size, _sbx_global_vsiType[unit]);
            SBX_WB_DB_SYNC_ARRAY(uint32, SBX_MAX_PORTS, SOC_SBX_PORT_ETE(unit, array_idx));
            SBX_WB_DB_SYNC_ARRAY(uint32, SBX_MAX_PORTS, SOC_SBX_PORT_UT_ETE(unit, array_idx));
            SBX_WB_DB_SYNC_ARRAY(uint32, SBX_MAX_TRUNKS, SOC_SBX_TRUNK_ETE(unit, array_idx));
            SBX_WB_DB_SYNC_VARIABLE(uint32, 1, SOC_SBX_INVALID_L2ETE(unit));
           

            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_INIT | BSL_INFO)) {
                _sbx_mac_tracker_dump(unit);
                bcm_sbx_caladan3_global_ftType_dump(unit);
                bcm_sbx_caladan3_global_vsiType_dump(unit);
                soc_sbx_g3p1_allocator_reservation_shell_print(unit);
                soc_sbx_allocator_shell_print(unit);
                bcm_sbx_caladan3_ete_dump(unit);
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
_bcm_caladan3_wb_allocator_state_restore (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    /* Local Variable for ALLOCATOR */
    uint32 size = 0;
    int array_idx = 0;
    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    switch(version)
    {
        case BCM_CALADAN3_WB_ALLOCATOR_VERSION_1_0:
            /* Restore state from scache */
            SBX_WB_DB_RESTORE_MEMORY(_sbx_track_mac_data_t, bcm_sbx_caladan3_smac_tracker_ptr(unit)->ismac.numMacs, 
                                                                bcm_sbx_caladan3_smac_tracker_ptr(unit)->ismac.macData);
            SBX_WB_DB_RESTORE_MEMORY(_sbx_track_mac_data_t, bcm_sbx_caladan3_smac_tracker_ptr(unit)->esmac.numMacs, 
                                                                bcm_sbx_caladan3_smac_tracker_ptr(unit)->esmac.macData);
            BCM_IF_ERR_EXIT (bcm_sbx_caladan3_global_ftType_size(unit, &size));
            SBX_WB_DB_RESTORE_MEMORY(uint8, size, _sbx_global_ftType[unit]);
            BCM_IF_ERR_EXIT (bcm_sbx_caladan3_global_vsiType_size(unit, &size));
            SBX_WB_DB_RESTORE_MEMORY(uint8, size, _sbx_global_vsiType[unit]);
            SBX_WB_DB_RESTORE_ARRAY(uint32, SBX_MAX_PORTS, SOC_SBX_PORT_ETE(unit, array_idx));
            SBX_WB_DB_RESTORE_ARRAY(uint32, SBX_MAX_PORTS, SOC_SBX_PORT_UT_ETE(unit, array_idx));
            SBX_WB_DB_RESTORE_ARRAY(uint32, SBX_MAX_TRUNKS, SOC_SBX_TRUNK_ETE(unit, array_idx));
            SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, SOC_SBX_INVALID_L2ETE(unit));
            BCM_IF_ERR_EXIT ( _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, SBX_MAX_PORTS, &(SOC_SBX_PORT_ETE(unit, 0)),_SBX_CALADAN3_RES_FLAGS_RESERVE));
            BCM_IF_ERR_EXIT ( _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, SBX_MAX_PORTS, &(SOC_SBX_PORT_UT_ETE(unit, 0)),_SBX_CALADAN3_RES_FLAGS_RESERVE));
            BCM_IF_ERR_EXIT ( _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, SBX_MAX_TRUNKS, &(SOC_SBX_TRUNK_ETE(unit, 0)),_SBX_CALADAN3_RES_FLAGS_RESERVE));
            BCM_IF_ERR_EXIT ( _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, 1, &(SOC_SBX_INVALID_L2ETE(unit)),_SBX_CALADAN3_RES_FLAGS_RESERVE));

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_INIT | BSL_INFO)) {
                _sbx_mac_tracker_dump(unit);
                bcm_sbx_caladan3_global_ftType_dump(unit);
                bcm_sbx_caladan3_global_vsiType_dump(unit);
                soc_sbx_g3p1_allocator_reservation_shell_print(unit);
                soc_sbx_allocator_shell_print(unit);
                bcm_sbx_caladan3_ete_dump(unit);
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
bcm_caladan3_wb_allocator_state_init (int unit)
{
    int result = BCM_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_ALLOCATOR_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit))
    {
        _bcm_caladan3_wb_allocator_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_allocator_state_scache_alloc (unit));
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_ALLOCATOR, 0);

    if (SOC_WARM_BOOT (unit))
    {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRetrieve, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == result)
        {
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
                        (BSL_META_U(unit,
                                    "unit %d loading ALLOCATOR backing store state\n"),
                         unit));
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_allocator_state_layout_init (unit, version, &scache_len));
            if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len)
            {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Scache length %d is not same as stored length %d\n"),
                           scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
            }
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_allocator_state_restore (unit));
            if (BCM_E_NONE == result)
            {
                if (version != recovered_version)
                {
                    /* set up layout for the preferred version */
                    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_allocator_state_layout_init (unit, version, &scache_len));
                    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                                (BSL_META_U(unit,
                                            "unit %d reallocate %d bytes warm"
                                            " boot backing store space\n"),
                                 unit, scache_len));
                    /* reallocate the warm boot space */
                    result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRealloc, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
                    if (BCM_E_NONE != result)
                    {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "unable to reallocate %d bytes"
                                              " warm boot space for unit %d"
                                              " ALLOCATOR instance: %d (%s)\n"),
                                   scache_len, unit, result, _SHR_ERRMSG (result)));
                        BCM_EXIT;
                    }
                    SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == result) */

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
             _bcm_caladan3_wb_allocator_state_scache_info_p[unit]->init_done = TRUE;
        }
        else
        {			/* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                  " unit %d ALLOCATOR instance: %d (%s)\n"),
                       unit, result, _SHR_ERRMSG (result)));
        }			/* if (BCM_E_NONE == result) */
    }
    else
    {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_allocator_state_layout_init (unit, version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing"
                                " store space\n"),
                     unit, scache_len));
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheCreate, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE != result)
        {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, result, _SHR_ERRMSG (result)));
            BCM_EXIT;
        }
        else
        {
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_allocator_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
