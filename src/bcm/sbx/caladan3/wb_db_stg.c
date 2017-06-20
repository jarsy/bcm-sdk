/*
 * $Id: wb_db_stg.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: STG warmboot APIs
 *
 * Purpose:
 *     STG API for Caladan3 Packet Processor devices
 *     Warm boot support
 */

#include <shared/bsl.h>

#include <bcm/error.h>

#include <shared/bsl.h>

#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>

#include <bcm/types.h>
#include <bcm_int/sbx/caladan3/stg.h>
#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_stg.h>
#if 0
#include <bcm_int/sbx/caladan3/stg.h>
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#endif

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_STG


extern bcm_stg_info_t  caladan3_stg_info[BCM_MAX_NUM_UNITS];
extern int _bcm_caladan3_stg_sw_dump(int unit);

bcm_caladan3_wb_stg_state_scache_info_t *
    _bcm_caladan3_wb_stg_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };


STATIC int
_bcm_caladan3_wb_stg_state_scache_alloc(int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC(_bcm_caladan3_wb_stg_state_scache_info_p[unit],
        sizeof (bcm_caladan3_wb_stg_state_scache_info_t), "Scache for STG warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_stg_state_scache_free(int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE(_bcm_caladan3_wb_stg_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_stg_state_layout_init(int unit, int version, unsigned int *scache_len)
{
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    bcm_stg_info_t      *si;
    int                 size;

    BCM_INIT_FUNC_DEFS;

    si = &STG_CNTL(unit);

    wb_info_ptr = BCM_CALADAN3_STG_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version) {
        case BCM_CALADAN3_WB_STG_VERSION_1_0:
            *scache_len = 0;
            wb_info_ptr->version = version;

            wb_info_ptr->stg_defl_offset = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(bcm_stg_t, 1);

            wb_info_ptr->stg_bitmap_offset = *scache_len;
            size = SHR_BITALLOCSIZE(si->stg_max+1);
            *scache_len += size;

            wb_info_ptr->stg_enable_offset = *scache_len;
            size = sizeof(bcm_pbmp_t) * (si->stg_max + 1);
            *scache_len += size;

            wb_info_ptr->stg_state_h_offset = *scache_len;
            size = sizeof(bcm_pbmp_t) * (si->stg_max + 1);
            *scache_len += size;

            wb_info_ptr->stg_state_l_offset = *scache_len;
            size = sizeof(bcm_pbmp_t) * (si->stg_max + 1);
            *scache_len += size;

            wb_info_ptr->stg_count_offset = *scache_len;
            SBX_WB_DB_LAYOUT_INIT_NV(int, 1);

            wb_info_ptr->vlan_first_offset = *scache_len;
            size = (si->stg_max + 1) * sizeof (bcm_vlan_t);
            *scache_len += size;

            wb_info_ptr->vlan_next_offset = *scache_len;
            size = BCM_VLAN_COUNT * sizeof (bcm_vlan_t);
            *scache_len += size;

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
_bcm_caladan3_wb_stg_state_restore(int unit)
{
    int version = 0;
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;
    bcm_stg_info_t *si;
    uint8           *scache_ptr;
    uint32          *wb_bitop_ptr;
    bcm_pbmp_t      *wb_pbmp_ptr;
    bcm_vlan_t      *wb_vlan_ptr;
    int             i;

    BCM_INIT_FUNC_DEFS;

    si = &STG_CNTL(unit);

    wb_info_ptr = BCM_CALADAN3_STG_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }
    version = wb_info_ptr->version;

    switch(version) {
        case BCM_CALADAN3_WB_STG_VERSION_1_0:
            scache_ptr = wb_info_ptr->scache_ptr;

            SBX_WB_DB_RESTORE_VARIABLE(bcm_stg_t, 1, si->stg_defl);

            wb_bitop_ptr = (uint32 *)(scache_ptr + wb_info_ptr->stg_bitmap_offset);
            SHR_BITCOPY_RANGE(si->stg_bitmap, 0, wb_bitop_ptr, 0, si->stg_max+1);

            wb_pbmp_ptr = (bcm_pbmp_t *)(scache_ptr + wb_info_ptr->stg_enable_offset);
            for (i = si->stg_min; i <= si->stg_max; i++) {
                BCM_PBMP_ASSIGN(si->stg_enable[i], wb_pbmp_ptr[i]);
            }

            wb_pbmp_ptr = (bcm_pbmp_t *)(scache_ptr + wb_info_ptr->stg_state_h_offset);
            for (i = si->stg_min; i <= si->stg_max; i++) {
                BCM_PBMP_ASSIGN(si->stg_state_h[i], wb_pbmp_ptr[i]);
            }

            wb_pbmp_ptr = (bcm_pbmp_t *)(scache_ptr + wb_info_ptr->stg_state_l_offset);
            for (i = si->stg_min; i <= si->stg_max; i++) {
                BCM_PBMP_ASSIGN(si->stg_state_l[i], wb_pbmp_ptr[i]);
            }

            si->stg_count = *(int *)(scache_ptr + wb_info_ptr->stg_count_offset);

            wb_vlan_ptr = (bcm_vlan_t *)(scache_ptr + wb_info_ptr->vlan_first_offset);
            for(i = si->stg_min; i <= si->stg_max; i++) {
                si->vlan_first[i] = wb_vlan_ptr[i];
            }

            wb_vlan_ptr = (bcm_vlan_t *)(scache_ptr + wb_info_ptr->vlan_next_offset);
            for(i = 0; i < BCM_VLAN_COUNT; i++) {
                si->vlan_next[i] = wb_vlan_ptr[i];
            }
            break;


        default:
            _rv = BCM_E_INTERNAL;
            BCM_IF_ERR_EXIT (_rv);
            /* coverity[dead_error_line] */
            break;
    }

    /* Dump state */
    if (bsl_fast_check(BSL_DEBUG|BSL_LS_BCM_STG)) {
        _bcm_caladan3_stg_sw_dump(unit);
    }

exit:
    BCM_FUNC_RETURN;
}

int
bcm_caladan3_wb_stg_state_init(int unit)
{
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_STG_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;
    bcm_caladan3_wb_stg_state_scache_info_t *wb_info_ptr = NULL;

    BCM_INIT_FUNC_DEFS;

    if (BCM_CALADAN3_STG_SCACHE_INFO_PTR(unit) != NULL) {
        _bcm_caladan3_wb_stg_state_scache_free(unit);
    }
    BCM_IF_ERR_EXIT(_bcm_caladan3_wb_stg_state_scache_alloc(unit));
    wb_info_ptr = BCM_CALADAN3_STG_SCACHE_INFO_PTR(unit);
    if(wb_info_ptr == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET(handle, unit, BCM_MODULE_STG, 0);

    if (SOC_WARM_BOOT(unit))
    {
        /* WARM BOOT */

        /* fetch the existing warm boot space */
        _rv = bcm_caladan3_scache_ptr_get(unit, handle, socScacheRetrieve, flags,
            &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == _rv) {
            LOG_VERBOSE(BSL_LS_BCM_COMMON,
			(BSL_META_U(unit,
				    "unit %d loading STG backing store state\n"),
			 unit));
            wb_info_ptr->scache_ptr = scache_ptr;

            BCM_IF_ERR_EXIT(_bcm_caladan3_wb_stg_state_layout_init(unit, version,
                &scache_len));
            if(scache_len != wb_info_ptr->scache_len)
            {
                LOG_ERROR(BSL_LS_BCM_COMMON,
			  (BSL_META_U(unit,
				      "Scache length %d is not same as stored length %d\n"),
			   scache_len, wb_info_ptr->scache_len));
            }

            wb_info_ptr->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_stg_state_scache_info_p[unit]->init_done = TRUE;

        } else {			/* if (BCM_E_NONE == _rv) */
            LOG_ERROR(BSL_LS_BCM_COMMON,
		      (BSL_META_U(unit,
				  "unable to get current warm boot state for"
                                  " unit %d STG instance: %d (%s)\n"),
                       unit, _rv, _SHR_ERRMSG (_rv)));
        }			/* if (BCM_E_NONE == _rv) */

    } else {
        /* COLD BOOT */

        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT(_bcm_caladan3_wb_stg_state_layout_init(unit, version, &scache_len));

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
            _bcm_caladan3_wb_stg_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
