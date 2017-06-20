/*
 * $Id: wb_db_l2.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: L2 APIs
 *
 * Purpose:
 *     L2 API for Caladan3 Packet Processor devices
 *     Warm boot support
 */

#include <shared/bsl.h>

#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/bitop.h>

#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/wb_db_cmn.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>

#include <bcm_int/sbx/caladan3/bcm_sw_db.h>
#include <bcm_int/sbx/caladan3/wb_db_l2.h>
#include <bcm_int/sbx/caladan3/l2.h>

#ifdef BCM_WARM_BOOT_SUPPORT

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_LS_BCM_L2


/*
 *  Externs for L2
 */
extern INLINE int bcm_sbx_caladan3_l2_age_sec_get (int unit);
extern void bcm_sbx_caladan3_l2_state_dump(int unit);
extern int bcm_caladan3_l2_age_timer_set(int unit, int age_seconds);
extern uint32 *bcm_sbx_l2_egress_dest_port_ptr_get(int unit);
extern int _bcm_caladan3_l2_egress_restore(int unit);

extern void _bcm_caladan3_dump_ager_info(int unit, int start, int end,int stack);
extern _ageid_to_mac_info_t *p_age_id_mac_array[BCM_LOCAL_UNITS_MAX];
extern _age_id_stack_t age_indexes_stack[BCM_LOCAL_UNITS_MAX];


/*
 *  Locals for L2
 */
static bcm_caladan3_wb_l2_state_scache_info_t *_bcm_caladan3_wb_l2_state_scache_info_p[BCM_MAX_NUM_UNITS] = { 0 };

/*
 *  Defines for L2
 */
#define SBX_SCACHE_INFO_PTR(unit) (_bcm_caladan3_wb_l2_state_scache_info_p[unit])
#define SBX_SCACHE_INFO_CHECK(unit) ((_bcm_caladan3_wb_l2_state_scache_info_p[unit]) != NULL \
        && (_bcm_caladan3_wb_l2_state_scache_info_p[unit]->init_done == TRUE))

/*
 *  Warmboot APIs implementation for L2
 */
STATIC int
_bcm_caladan3_wb_l2_state_scache_alloc (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_ALLOC (_bcm_caladan3_wb_l2_state_scache_info_p[unit], sizeof (bcm_caladan3_wb_l2_state_scache_info_t), "Scache for L2 warm boot");

exit:
    BCM_FUNC_RETURN;

}

STATIC void
_bcm_caladan3_wb_l2_state_scache_free (int unit)
{

    BCM_INIT_FUNC_DEFS;
    BCM_FREE (_bcm_caladan3_wb_l2_state_scache_info_p[unit]);

    BCM_FUNC_RETURN_VOID;
}

STATIC int
_bcm_caladan3_wb_l2_state_layout_init (int unit, int version, unsigned int *scache_len)
{
    int rc = BCM_E_NONE;
    /* Local Variable for L2*/
    int macage_tb_elements = soc_sbx_g3p1_macage_table_size_get(unit);

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    switch(version)
    {
        case BCM_CALADAN3_WB_L2_VERSION_1_0:
            /* Initialize scache info */
            *scache_len = 0;
            SBX_SCACHE_INFO_PTR(unit)->version = version;
            
            /* Layout for scache lenght and offset calculation */
            SBX_WB_DB_LAYOUT_INIT(int, 1, age_sec);
            SBX_WB_DB_LAYOUT_INIT(uint32, SBX_MAX_L2_EGRESS_OHI, bcm_sbx_l2_egress_dest_port_ptr_get(unit));
            SBX_WB_DB_LAYOUT_INIT(_ageid_to_mac_info_t, macage_tb_elements, p_age_id_mac_array[unit]);
            SBX_WB_DB_LAYOUT_INIT(uint32, macage_tb_elements, age_indexes_stack[unit].age_indexes);
            SBX_WB_DB_LAYOUT_INIT(uint32, 1, age_indexes_stack[unit].p);


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
bcm_caladan3_wb_l2_state_sync (int unit, int arg)
{
    int rc = BCM_E_NONE;
    int version = 0;
    uint8 *scache_ptr_orig = NULL;
    /* Local Variable for L2*/
    int age_sec = 0;
    int macage_tb_elements = 0;

    BCM_INIT_FUNC_DEFS;
    
    if(SOC_WARM_BOOT(unit))
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Sync not allowed during warmboot on unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_CHECK(unit) != TRUE)
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Warm boot scache not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    version = SBX_SCACHE_INFO_PTR(unit)->version;
    scache_ptr_orig = SBX_SCACHE_INFO_PTR(unit)->scache_ptr;

    switch(version)
    {
        case BCM_CALADAN3_WB_L2_VERSION_1_0:
            age_sec = bcm_sbx_caladan3_l2_age_sec_get(unit);
            macage_tb_elements = soc_sbx_g3p1_macage_table_size_get(unit);
            /* Sync state to the scache */
	        LOG_CLI((BSL_META_U(unit,
                                    "Synced age sec %d\n"), age_sec));
            SBX_WB_DB_SYNC_VARIABLE(int, 1, age_sec);
            SBX_WB_DB_SYNC_MEMORY(uint32, SBX_MAX_L2_EGRESS_OHI, bcm_sbx_l2_egress_dest_port_ptr_get(unit));
            SBX_WB_DB_SYNC_MEMORY(_ageid_to_mac_info_t, macage_tb_elements, p_age_id_mac_array[unit]);
            SBX_WB_DB_SYNC_MEMORY(uint32, macage_tb_elements, age_indexes_stack[unit].age_indexes);
            SBX_WB_DB_SYNC_VARIABLE(uint32, 1, age_indexes_stack[unit].p);

            if(arg)
            {
	            LOG_CLI((BSL_META_U(unit,
                    "Stop the age timer thread and aging after final sync\n")));
                BCM_IF_ERR_EXIT(bcm_caladan3_l2_age_timer_set(unit, 0));
            }

            /* Restore the scache ptr to original */
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr_orig;

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_INFO)) {
                bcm_sbx_caladan3_l2_state_dump(unit);
                _bcm_caladan3_dump_ager_info(unit, 0xfffd0/*start*/, 0xfffff/*end*/, 0/*stack*/);
                _bcm_caladan3_dump_ager_info(unit, 0xfffd0/*start*/, 0xfffff/*end*/, 1/*stack*/);
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
_bcm_caladan3_wb_l2_state_restore (int unit)
{
    int rc = BCM_E_NONE;
    int version = 0;
    /* Local Variable for L2*/
    int age_sec = 0;
    int macage_tb_elements = soc_sbx_g3p1_macage_table_size_get(unit);

    BCM_INIT_FUNC_DEFS;

    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }
    version = SBX_SCACHE_INFO_PTR(unit)->version;

    switch(version)
    {
        case BCM_CALADAN3_WB_L2_VERSION_1_0:
            /* Restore state from scache */
            SBX_WB_DB_RESTORE_VARIABLE(int, 1, age_sec);
            SBX_WB_DB_RESTORE_MEMORY(uint32, SBX_MAX_L2_EGRESS_OHI, bcm_sbx_l2_egress_dest_port_ptr_get(unit));
            SBX_WB_DB_RESTORE_MEMORY(_ageid_to_mac_info_t, macage_tb_elements, p_age_id_mac_array[unit]);
            SBX_WB_DB_RESTORE_MEMORY(uint32, macage_tb_elements, age_indexes_stack[unit].age_indexes);
            SBX_WB_DB_RESTORE_VARIABLE(uint32, 1, age_indexes_stack[unit].p);

	        LOG_CLI((BSL_META_U(unit,
                "Restore L2 Egress Reservation \n")));
            BCM_IF_ERR_EXIT(_bcm_caladan3_l2_egress_restore(unit));

	        LOG_CLI((BSL_META_U(unit,
                "Restored age sec %d and started the age timer thread\n"), age_sec));
            BCM_IF_ERR_EXIT(bcm_caladan3_l2_age_timer_set(unit, (age_sec)));

            /* Dump state */
            if (LOG_CHECK(BSL_LS_BCM_L2 | BSL_INFO)) {
                bcm_sbx_caladan3_l2_state_dump(unit);
                _bcm_caladan3_dump_ager_info(unit, 0xfffd0/*start*/, 0xfffff/*end*/, 0/*stack*/);
                _bcm_caladan3_dump_ager_info(unit, 0xfffd0/*start*/, 0xfffff/*end*/, 1/*stack*/);
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
bcm_caladan3_wb_l2_state_init (int unit)
{
    int result = BCM_E_NONE;
    int flags = SOC_CALADAN3_SCACHE_DEFAULT;
    int exists = 0;
    uint16 version = BCM_CALADAN3_WB_L2_VERSION_CURR;
    uint16 recovered_version = 0;
    unsigned int scache_len = 0;
    soc_scache_handle_t handle = 0;
    uint8 *scache_ptr = NULL;

    BCM_INIT_FUNC_DEFS;
    if(SBX_SCACHE_INFO_PTR(unit))
    {
        _bcm_caladan3_wb_l2_state_scache_free (unit);
    }

    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l2_state_scache_alloc (unit));
    if(SBX_SCACHE_INFO_PTR(unit) == NULL)
    {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Warm boot not initialized for unit %d \n"),
                   unit));
        BCM_EXIT;
    }

    SOC_SCACHE_HANDLE_SET (handle, unit, BCM_MODULE_L2, 0);

    if (SOC_WARM_BOOT (unit))
    {
        /* WARM BOOT */
        /* fetch the existing warm boot space */
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRetrieve, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE == result)
        {
            LOG_VERBOSE(BSL_LS_BCM_L2,
                        (BSL_META_U(unit,
                                    "unit %d loading L2 backing store state\n"),
                         unit));
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l2_state_layout_init (unit, version, &scache_len));
            if(scache_len != SBX_SCACHE_INFO_PTR(unit)->scache_len)
            {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "Scache length %d is not same as stored length %d\n"),
                           scache_len, SBX_SCACHE_INFO_PTR(unit)->scache_len));
            }
            BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l2_state_restore (unit));
            if (BCM_E_NONE == result)
            {
                if (version != recovered_version)
                {
                    /* set up layout for the preferred version */
                    BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l2_state_layout_init (unit, version, &scache_len));
                    LOG_VERBOSE(BSL_LS_BCM_L2,
                                (BSL_META_U(unit,
                                            "unit %d reallocate %d bytes warm"
                                            " boot backing store space\n"),
                                 unit, scache_len));
                    /* reallocate the warm boot space */
                    result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheRealloc, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
                    if (BCM_E_NONE != result)
                    {
                        LOG_ERROR(BSL_LS_BCM_L2,
                                  (BSL_META_U(unit,
                                              "unable to reallocate %d bytes"
                                              " warm boot space for unit %d"
                                              " L2 instance: %d (%s)\n"),
                                   scache_len, unit, result, _SHR_ERRMSG (result)));
                        BCM_EXIT;
                    }
                    SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
                }		/* if (version != recovered_version) */
            }			/* if (BCM_E_NONE == result) */

            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_l2_state_scache_info_p[unit]->init_done = TRUE;
        }
        else
        {			/* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "unable to get current warm boot state for" " unit %d L2 instance: %d (%s)\n"),
                       unit, result, _SHR_ERRMSG (result)));
        }			/* if (BCM_E_NONE == result) */
    }
    else
    {
        /* COLD BOOT */
        /* set up layout for the preferred version */
        BCM_IF_ERR_EXIT (_bcm_caladan3_wb_l2_state_layout_init (unit, version, &scache_len));

        /* set up backing store space */
        LOG_VERBOSE(BSL_LS_BCM_L2,
                    (BSL_META_U(unit,
                                "unit %d allocate %d bytes warm boot backing" " store space\n"),
                     unit, scache_len));
        result = bcm_caladan3_scache_ptr_get (unit, handle, socScacheCreate, flags, &scache_len, &scache_ptr, version, &recovered_version, &exists);
        if (BCM_E_NONE != result)
        {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "unable to allocate %d bytes warm boot space"
                                  " for unit %d field instance: %d (%s)\n"),
                       scache_len, unit, result, _SHR_ERRMSG (result)));
            BCM_EXIT;
        }
        else
        {
            SBX_SCACHE_INFO_PTR(unit)->scache_ptr = scache_ptr;
            _bcm_caladan3_wb_l2_state_scache_info_p[unit]->init_done = TRUE;
        }
    }

exit:
    BCM_FUNC_RETURN;
}

#endif /* def BCM_WARM_BOOT_SUPPORT */
