/*
 * $Id: wb_db_vswitch.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:
 *     Warm boot support to handle vswitch module.
 */

#include <shared/bsl.h>
#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <soc/sbx/caladan3/soc_sw_db.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>

#include <bcm_int/common/debug.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/vswitch.h>
#include <bcm_int/sbx/caladan3/wb_db_vswitch.h>

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_VLAN

/*
 *  Externs for VSWITCH
 */

extern _bcm_caladan3_vswitch_state_t * bcm_sbx_caladan3_vswitch_cntl_ptr_get(int unit);


/*
 *  Locals for VSWITCH
 */
static bcm_caladan3_wb_vswitch_state_scache_info_t *_bcm_caladan3_wb_vswitch_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

/*
 *  Defines for VSWITCH
 */
#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_vswitch_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_vswitch_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_vswitch_state_scache_info_p[unit]->init_done == TRUE))
#define VSWITCH_MAX_PORTS 50

/*
 *  Warmboot APIs implementation for VSWITCH
 */
STATIC int
_bcm_caladan3_wb_vswitch_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_vswitch_state_scache_info_p[unit], sizeof (bcm_caladan3_wb_vswitch_state_scache_info_t), "Scache for VSWITCH warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_vswitch_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_vswitch_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_vswitch_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    int rc = BCM_E_NONE;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_VSWITCH_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;

            /* Layout for scache length and offset calculation */
            SBX_WB_DB_LAYOUT_INIT(bcm_gport_t, SBX_DYNAMIC_VSI_END(unit)*VSWITCH_MAX_PORTS, NULL);

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
bcm_caladan3_wb_vswitch_state_sync (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    uint8 *scache_ptr_orig = NULL;
    /* Local Variable for VSWITCH*/
    int vsi, port;

    BCM_INIT_FUNC_DEFS;

    if(SOC_WARM_BOOT(unit))
    {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "Sync not allowed during warmboot on unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    BCM_IF_ERR_EXIT(bcm_caladan3_wb_vswitch_state_init (unit));

    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE)
    {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "Warm boot scache not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    switch(version)
    {
        case BCM_CALADAN3_WB_VSWITCH_VERSION_1_0:
            /* Sync state to the scache */
            for (vsi=0; vsi<SBX_DYNAMIC_VSI_END(unit); vsi++) {
                _bcm_caladan3_vswitch_port_t *portList;
                portList = bcm_sbx_caladan3_vswitch_cntl_ptr_get(unit)->portList[vsi];
                for (port=0; port<VSWITCH_MAX_PORTS; port++) {
                    if (portList) {
                        SBX_WB_DB_SYNC_VARIABLE(int, 1, portList->gport);
                        portList = portList->next;
                    } else {
                        SBX_WB_DB_SYNC_VARIABLE(int, 1, 0xffffffff);
                    }
                }
            }
            
            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            break;

        default:
            rc = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (rc);
    }

exit:
    BCM_FUNC_RETURN;
}

STATIC int
_bcm_caladan3_wb_vswitch_state_restore (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    _bcm_caladan3_vswitch_state_t *vswitch_state;
    int vsi;
    int port;

    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;
    vswitch_state = bcm_sbx_caladan3_vswitch_cntl_ptr_get(unit);

    switch(version)
    {
        case BCM_CALADAN3_WB_VSWITCH_VERSION_1_0:
            /* Restore state from scache. */
            for (vsi=0; vsi<SBX_DYNAMIC_VSI_END(unit); vsi++) {
                /* SBX_CALADAN3_USR_RES_VSI is restored in vswitch_init. */
                if (_sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_VSI, vsi) == BCM_E_EXISTS) {
                    _bcm_caladan3_vswitch_port_t *tail=NULL;
                    _bcm_caladan3_vswitch_port_t *newEntry = NULL;
                    
                    for (port=0; port<VSWITCH_MAX_PORTS; port++) {
                        if (*(uint32 *)SBX_SCACHE_INFO_PTR(unit)->scache_ptr != 0xffffffff) {
                            bcm_gport_t gport = *(bcm_gport_t *)SBX_SCACHE_INFO_PTR(unit)->scache_ptr;
                            newEntry = sal_alloc(sizeof(_bcm_caladan3_vswitch_port_t),
                                                  "_bcm_caladan3_vswitch_port_list_entry");
                            if (!newEntry) {
                                /* coverity [leaked_storage] */
                                return BCM_E_MEMORY;
                            }
                            newEntry->gport = gport;
                            newEntry->next = NULL;
                            newEntry->prev = tail;
                            if (tail) {
                                tail->next = newEntry;
                            }
                            if (vswitch_state->portList[vsi] == NULL) {
                                vswitch_state->portList[vsi] = newEntry;
                            }
                            tail = newEntry;
                        }
                        SBX_SCACHE_INFO_PTR(unit)->scache_ptr += sizeof(bcm_gport_t);
                    }
                }
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
bcm_caladan3_wb_vswitch_state_init (int unit)
{
    int result = BCM_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_VSWITCH_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit))
    {
        _bcm_caladan3_wb_vswitch_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_vswitch_state_scache_alloc (unit));
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_VSWITCH, 0);

    if (SOC_WARM_BOOT (unit))
    {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRetrieve, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == result)
        {
            LOG_VERBOSE(BSL_LS_BCM_VLAN,
                        (BSL_META_U(unit,
                                    "unit %d loading VSWITCH backing store state\n"),
                         unit));
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_vswitch_state_layout_init (unit, version, &scache_len));
            if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len)
            {
                LOG_ERROR(BSL_LS_BCM_VLAN,
                          (BSL_META_U(unit,
                                      "Scache length %d is not same as stored length %d\n"),
                           scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
            }
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_vswitch_state_restore (unit));
            if (BCM_E_NONE == result)
            {
                if (version != recovered_version)
                {
                    /* set up layout for the preferred version */
                    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_vswitch_state_layout_init (unit, version, &scache_len));
                    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                                (BSL_META_U(unit,
                                            "unit %d reallocate %d bytes warm"
                                            " boot backing store space\n"),
                                 unit, scache_len));
                    /* reallocate the warm boot space */
                    result = soc_caladan3_scache_ptr_get (unit, handle, socScacheRealloc, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
                    if (BCM_E_NONE != result)
                    {
                        LOG_ERROR(BSL_LS_BCM_VLAN,
                                  (BSL_META_U(unit,
                                              "unable to reallocate %d bytes"
                                               " warm boot space for unit %d"
                                              " VSWITCH instance: %d (%s)\n"),
                                   scache_len, unit, result, _SHR_ERRMSG (result)));
                        BCM_EXIT;
                    }
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == result) */

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_vswitch_state_scache_info_p[unit]->init_done = TRUE;
        }
        else
        {			/* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for"
                                  " unit %d VSWITCH instance: %d (%s)\n"),
                       unit, result, _SHR_ERRMSG (result)));
        }			/* if (BCM_E_NONE == result) */
    }
    else
    {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_vswitch_state_layout_init (unit, version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_VLAN,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing"
                                " store space\n"),
                     unit, scache_len));
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheCreate, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE != result)
        {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, result, _SHR_ERRMSG (result)));
            BCM_EXIT;
        }
        else
        {
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_vswitch_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
