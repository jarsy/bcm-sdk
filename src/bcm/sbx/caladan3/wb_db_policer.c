/*
 * $Id: wb_db_policer.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: POLICER warmboot APIs
 *
 * Purpose:
 *     POLICER API for Caladan3 Packet Processor devices
 *     Warm boot support
 */

#include <shared/bsl.h>

#include <bcm/error.h>

#include <soc/sbx/caladan3/soc_sw_db.h>

#include <soc/sbx/wb_db_cmn.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_policer.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <bcm/policer.h>
#include <shared/idxres_afl.h>
#include <shared/idxres_fl.h>
#include <bcm_int/sbx/caladan3/policer.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_POLICER


bcm_caladan3_wb_policer_state_scache_info_t *
    _bcm_caladan3_wb_policer_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

STATIC int
_bcm_caladan3_wb_policer_state_scache_alloc(int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC(_bcm_caladan3_wb_policer_state_scache_info_p[unit],
        sizeof (bcm_caladan3_wb_policer_state_scache_info_t), "Scache for POLICER warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_policer_state_scache_free(int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE(_bcm_caladan3_wb_policer_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_policer_state_layout_init(int unit, int version, unsigned int *scache_len)
{
    bcm_caladan3_wb_policer_state_scache_info_t *wb_info_ptr = NULL;
    int max_policers;

    BCM_INIT_FUNC_DEFS;
    wb_info_ptr = BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    max_policers = _bcm_caladan3_policer_num_policers_get(unit);

    switch(version) {
        case BCM_CALADAN3_WB_POLICER_VERSION_1_0:
            *scache_len = 0;
            wb_info_ptr->version = version;

            wb_info_ptr->group_mode_offset = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(uint32, max_policers);
            wb_info_ptr->flags_offset = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(uint32, max_policers);
            wb_info_ptr->base_counter_offset = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(uint32, max_policers);

            wb_info_ptr->scache_len = *scache_len;
            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
    }

exit:
    BCM_FUNC_RETURN;
}


int
bcm_caladan3_wb_policer_state_init(int unit)
{
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_POLICER_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;
    bcm_caladan3_wb_policer_state_scache_info_t *wb_info_ptr = NULL;

    BCM_INIT_FUNC_DEFS;

    if (BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit) != NULL) {
        _bcm_caladan3_wb_policer_state_scache_free(unit);
    }
    BCM_IF_ERR_EXIT(_bcm_caladan3_wb_policer_state_scache_alloc(unit));
    wb_info_ptr = BCM_CALADAN3_POLICER_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET(handle, unit, BCM_MODULE_POLICER, 0);

    if (SOC_WARM_BOOT(unit))
    {
        /* WARM BOOT */

        /* fetch the existing warm boot space */
        _rv = bcm_caladan3_scache_ptr_get(unit, handle, socScacheRetrieve, flags,
            &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == _rv) {
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
			(BSL_META_U(unit,
				    "unit %d loading POLICER backing store state\n"),
			 unit));
            wb_info_ptr->scache_ptr = scache_ptr;

            BCM_IF_ERR_EXIT(_bcm_caladan3_wb_policer_state_layout_init(unit, version,
                &scache_len));
            if(scache_len != wb_info_ptr->scache_len)
            {
                LOG_ERROR(BSL_LS_BCM_COMMON,
			  (BSL_META_U(unit,
				      "Scache length %d is not same as stored length %d\n"),
			   scache_len, wb_info_ptr->scache_len));
            }

            wb_info_ptr->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_policer_state_scache_info_p[unit]->init_done = TRUE;

        } else {			/* if (BCM_E_NONE == _rv) */
            LOG_ERROR(BSL_LS_BCM_COMMON,
		      (BSL_META_U(unit,
				  "unable to get current warm boot state for"
                                  " unit %d POLICER instance: %d (%s)\n"),
                       unit, _rv, _SHR_ERRMSG (_rv)));
        }			/* if (BCM_E_NONE == _rv) */

    } else {
        /* COLD BOOT */

        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT(_bcm_caladan3_wb_policer_state_layout_init(unit, version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
		    (BSL_META_U(unit,
				"unit %d allocate %d bytes warm boot backing"
                                " store space\n"),
                     unit, scache_len));
        _rv = bcm_caladan3_scache_ptr_get(unit, handle, socScacheCreate,
            flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE != _rv) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
		      (BSL_META_U(unit,
				  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
		       scache_len, unit, _rv, _SHR_ERRMSG (_rv)));
            BCM_EXIT;
        } else {
            wb_info_ptr->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_policer_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
