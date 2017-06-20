/*
 * $Id: switch.c,v 1.144 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        switch.c
 * Purpose:     BCM definitions for bcm_switch_control
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_SWITCH

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>

#include <soc/drv.h>

#include <soc/dnxc/legacy/dnxc_wb.h>

#include <bcm/switch.h>
#include <bcm/error.h>
#include <bcm/debug.h>

#include <bcm_int/dnx_dispatch.h>
#include <bcm_int/common/debug.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>
#include <bcm_int/dnx/legacy/switch.h>
#include <bcm_int/dnx/legacy/sw_db.h>
#include <bcm_int/dnx/legacy/utils.h>
#include <bcm_int/dnx/legacy/error.h>
#include <bcm_int/dnx/legacy/alloc_mngr.h>
#include <bcm_int/dnx/legacy/cosq.h>

#include <shared/swstate/sw_state.h>

#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/TMC/tmc_api_stack.h>
#include <soc/dnx/legacy/SAND/Management/sand_low_level.h>
#include <soc/dnx/legacy/dnx_wb_engine.h>
#include <shared/dnx_shr_template.h>

#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/shared/llm_appl.h>
#include <soc/shared/llm_msg.h>
#endif

#include <soc/dnx/legacy/JER/jer_mgmt.h>

/***************************************************************/
/***************************************************************/


/* 
 * Local defines
 *
 */
#define DNX_SWITCH_MSG(string)   "%s[%d]: " string, __FILE__, __LINE__

#define DNX_SWITCH_UNIT_VALID_CHECK \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
       LOG_ERROR(BSL_LS_BCM_SWITCH, \
                 (BSL_META_U(unit, \
                             "unit %d is not valid\n"), unit)); \
        return BCM_E_UNIT; \
    }
#define DNX_SWITCH_UNIT_INIT_CHECK
#define DNX_SWITCH_LOCK_TAKE
#define DNX_SWITCH_LOCK_RELEASE
#define DNX_SWITCH_TRAP_TO_CPU  (1)
#define DNX_SWITCH_DROP         (2)
#define DNX_SWITCH_FLOOD        (3)
#define DNX_SWITCH_FWD          (4)
#define DNX_SWITCH_SNOOP_TO_CPU (5)

#define DNX_FRWRD_IP_ALL_VRFS_ID SOC_PPC_FRWRD_IP_ALL_VRFS_ID

#define DNX_SWITCH_MAX_NUM_ADDITIONAL_TPIDS  4

#define SWITCH_ACCESS                           sw_state_access[unit].dnx.bcm._switch

#ifdef CRASH_RECOVERY_SUPPORT
extern soc_dnxc_cr_t *dnxc_cr_info[BCM_MAX_NUM_UNITS];
#endif

int
_bcm_dnx_switch_init(int unit)
{
    uint8 is_allocated;
    bcm_error_t rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
/*in order to recover the soc_control var in wb do this get*/
    if (SOC_WARM_BOOT(unit)) {
        rv = sw_state_access[unit].dnx.soc.config.autosync.get(unit, (uint32 *) &(SOC_CONTROL(unit)->autosync));
        BCMDNX_IF_ERR_EXIT(rv);
    }
#endif

    if(!SOC_WARM_BOOT(unit)) {
        rv = SWITCH_ACCESS.is_allocated(unit, &is_allocated);
        BCMDNX_IF_ERR_EXIT(rv);
        if(!is_allocated) {
            rv = SWITCH_ACCESS.alloc(unit);
            BCMDNX_IF_ERR_EXIT(rv);
        }
    }

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_switch_detach(int unit)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}


#ifdef BCM_WARM_BOOT_SUPPORT

#define _BCM_SYNC_SUCCESS(unit) \
  (BCM_SUCCESS(rv) || (BCM_E_INIT == rv) || (BCM_E_UNAVAIL == rv))
  

/* 
 * this function behave differently in different scenarios:
 * 1. when called in manual sync it perfors a full sync
 * 2. when called automatically by the dispatcher at autosync mode,
 *    it only sync modules that don't have autosync implementation.
 *    in the future all modules should have built in autosync and won't
 *    dispatcher level _bcm_dnx_switch_control_sync call
 * 3. when called in a TEST_MODE compilation - for now act the same. 
 *    the different behaviour is in the calling to this function 
 *     
 */
 
   
static int
_bcm_dnx_switch_control_sync(int unit, int arg)
{
    uint32 scache_handle;
    int rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;


    /****************/
    /*** autosync ***/
    /****************/
    if (SOC_AUTOSYNC_IS_ENABLE(unit)) {

        /* commit the specific modules that don't have autosync*/
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_SAT, 0);
        rv = soc_scache_commit_handle(unit, scache_handle, 0x0);
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_FIELD, 0);
        rv = soc_scache_commit_handle(unit, scache_handle, 0x0);
        BCMDNX_IF_ERR_EXIT(rv);

        /* Mark scache as clean */
        SOC_CONTROL_LOCK(unit);
        SOC_CONTROL(unit)->scache_dirty = 0;
        SOC_CONTROL_UNLOCK(unit);

        /* if autosync is enabled we don't need to sync all state
           for now sync only the data that does not support autosync
           so EXIT here */
        BCM_EXIT;
    }


    /* wb engine sync is not needed if running in autosync mode */
    rv = soc_dnx_wb_engine_sync(unit);
    BCMDNX_IF_ERR_EXIT(rv);

    /* now commit the scache to persistent storage (all scache buffers)*/
    rv = soc_scache_commit(unit);
    BCMDNX_IF_ERR_EXIT(rv);

    /* Mark scache as clean */
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 0;
    SOC_CONTROL_UNLOCK(unit);

    BCMDNX_IF_ERR_EXIT(rv);

exit:
    BCMDNX_FUNC_RETURN;
}
#endif /* BCM_WARM_BOOT_SUPPORT */



int
_bcm_dnx_switch_control_get(int unit,
                            bcm_switch_control_t bcm_type,
                            int *arg)
{
    int         rv = BCM_E_UNAVAIL; /* initialization value should not be changed */
    uint32      dnx_sand_rv;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32      flags;
#endif
    int     unsupported_switch = FALSE;
    int     fabric_synce_enable;
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_NULL_CHECK(arg);

    switch (bcm_type)
    {
    
    case bcmSwitchControlSync:
        break;
    
    case bcmSwitchControlAutoSync:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = sw_state_access[unit].dnx.soc.config.autosync.get(unit, (uint32 *) arg);
        *arg = (*arg && (!SOC_WARM_BOOT(unit)));
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;

    case bcmSwitchCrashRecoveryMode:
#ifdef CRASH_RECOVERY_SUPPORT
        rv = soc_dnxc_cr_journaling_mode_get(unit); 
#else
        rv = BCM_E_UNAVAIL;
#endif /* CRASH_RECOVERY_SUPPORT */
        break;
    case bcmSwitchStableSelect:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_get(unit, arg, &flags);
#endif /* BCM_WARM_BOOT_SUPPORT */
        break;
    case bcmSwitchStableSize:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_size_get(unit, arg);
#endif /* BCM_WARM_BOOT_SUPPORT */
        break;
    case bcmSwitchStableUsed:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_used_get(unit, arg);
#endif /* BCM_WARM_BOOT_SUPPORT */
        break;
    case bcmSwitchWarmBoot:
#ifdef BCM_WARM_BOOT_SUPPORT
        (*arg) = SOC_WARM_BOOT(unit);
        rv = BCM_E_NONE;
#endif /* BCM_WARM_BOOT_SUPPORT */
        break;
    case bcmSwitchSynchronousPortClockSource:
        DNXC_IF_ERR_EXIT(soc_dnx_fabric_sync_e_enable_get(unit, 1 ,&fabric_synce_enable));
        if (fabric_synce_enable) {
            rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_link_get, (unit, 1, arg));
        } else {
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_sel_port_get, (unit, 0, arg))); 
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
            rv = BCM_E_NONE;
        }
        break;
    case bcmSwitchSynchronousPortClockSourceBkup:
        DNXC_IF_ERR_EXIT(soc_dnx_fabric_sync_e_enable_get(unit, 0 ,&fabric_synce_enable));
        if (fabric_synce_enable) {
            rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_link_get, (unit, 0, arg));
        } else {
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_sel_port_get, (unit, 1, arg))); 
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
            rv = BCM_E_NONE;
        }
        break;
    case bcmSwitchSynchronousPortClockSourceDivCtrl:
        DNXC_IF_ERR_EXIT(soc_dnx_fabric_sync_e_enable_get(unit, 1 ,&fabric_synce_enable));
        if (fabric_synce_enable) {
            rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_divider_get, (unit, arg));
        } else {
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_div_get, (unit, 0, /*(JER2_ARAD_NIF_SYNCE_CLK_DIV*)*/arg))); 
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
#endif 
        }
        break;
    case bcmSwitchSynchronousPortClockSourceBkupDivCtrl:
        DNXC_IF_ERR_EXIT(soc_dnx_fabric_sync_e_enable_get(unit, 0 ,&fabric_synce_enable));
        if (fabric_synce_enable) {
            rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_divider_get, (unit, arg));
        } else {
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_div_get, (unit, 1, /*(JER2_ARAD_NIF_SYNCE_CLK_DIV*)*/arg))); 
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
#endif 
        }
        break;
    default:
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm switch control type")));
    }

    if (unsupported_switch) {
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm switch control type")));
    }

    BCMDNX_IF_ERR_EXIT(rv);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_switch_control_set(int unit, 
                            bcm_switch_control_t bcm_type,
                            int arg)
{
    int                         rv = BCM_E_NONE;
    int                         unsupported_switch = FALSE;
#if defined(BCM_WARM_BOOT_SUPPORT)
    int                     stable_select;
#endif
    int     fabric_synce_enable;

    BCMDNX_INIT_FUNC_DEFS;

    switch (bcm_type)
    {
    case bcmSwitchWarmBoot:
    case bcmSwitchStableSelect:
    case bcmSwitchStableSize:
    case bcmSwitchStableUsed:
        /* all should be configured through config.bcm */
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Unsupported switch control")));
    case bcmSwitchControlSync:
#if defined(BCM_WARM_BOOT_SUPPORT)
        if (!SOC_WARM_BOOT(unit)) {
            rv = _bcm_dnx_switch_control_sync(unit, arg);
            BCMDNX_IF_ERR_EXIT(rv);
        }
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;
    case bcmSwitchControlAutoSync:
#if defined(BCM_WARM_BOOT_SUPPORT)
        BCMDNX_IF_ERR_EXIT(_bcm_dnx_switch_control_get(unit,
                                                    bcmSwitchStableSelect, &stable_select));
        if (BCM_SWITCH_STABLE_NONE != stable_select) {
            SOC_CONTROL_LOCK(unit);
            arg = arg ? 1 : 0;

            /* perform a one time sync to make sure we start autosyncying when everything is in sync */
            /* important - need to call the sync before enabling the autosync flag */
            if (arg) {
                rv = _bcm_dnx_switch_control_sync(unit, arg);
                BCMDNX_IF_ERR_EXIT(rv);
            }

            /* this flag is used internally and recovered in wb */
            rv = sw_state_access[unit].dnx.soc.config.autosync.set(unit, arg);

            /* this flag is a common soc_control flag that may be used by common modules */
            SOC_CONTROL(unit)->autosync = arg;
            SOC_CONTROL_UNLOCK(unit);

        } else {
            rv = BCM_E_NOT_FOUND;
        }
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;

    case bcmSwitchCrashRecoveryMode:
#ifdef CRASH_RECOVERY_SUPPORT
        rv = soc_dnxc_cr_journaling_mode_set(unit, arg);
#else
        rv = BCM_E_UNAVAIL;
#endif /* CRASH_RECOVERY_SUPPORT */
        break;

    case bcmSwitchCrTransactionStart:
#ifdef CRASH_RECOVERY_SUPPORT
        /* start a new transaction */
        rv = soc_dnxc_cr_transaction_start(unit);
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;

    case bcmSwitchCrCommit:
#ifdef CRASH_RECOVERY_SUPPORT
        /* end the current transaction */
        rv = soc_dnxc_cr_commit(unit);
#else
        rv = BCM_E_UNAVAIL;
#endif
        break;

    /* Last transaction status setter function not available */
    case bcmSwitchCrLastTransactionStatus:
        rv = BCM_E_UNAVAIL;
        break;

    /* Could not recover setter function not available */
    case bcmSwitchCrCouldNotRecover:
        rv = BCM_E_UNAVAIL;
        break;

    case bcmSwitchSynchronousPortClockSource:
        if (arg >= SOC_INFO(unit).fabric_logical_port_base) {
            BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_link_set, (unit, 1, arg)));
        } else {
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_sel_port_set, (unit, 0, arg))); 
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
#endif 
        }
        break;
    case bcmSwitchSynchronousPortClockSourceBkup:
        if (arg >= SOC_INFO(unit).fabric_logical_port_base) {
            BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_link_set, (unit, 0, arg)));
        } else {
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_nif_synce_clk_sel_port_set,(unit, 1, arg)));
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
#endif 
        }
        break;
    case bcmSwitchSynchronousPortClockSourceDivCtrl:
    	/* there is an overlap in synce divider options in fabric and nif. Thus check if fabric synce is enabled */
        DNXC_IF_ERR_EXIT(soc_dnx_fabric_sync_e_enable_get(unit, 1 ,&fabric_synce_enable));
        if (fabric_synce_enable) {
          BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_divider_set, (unit, arg)));
        } else {
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_div_set, (unit, 0, arg)));
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
#endif 
        }
        break;
    case bcmSwitchSynchronousPortClockSourceBkupDivCtrl:
        DNXC_IF_ERR_EXIT(soc_dnx_fabric_sync_e_enable_get(unit, 1 ,&fabric_synce_enable));
        if (fabric_synce_enable) {
            BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_port_sync_e_divider_set, (unit, arg)));
        } else {
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_nif_synce_clk_div_set, (unit, 1, arg)));
            DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
#endif 
        }
        break;
    default:
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm switch control type")));
    }
 

    if (unsupported_switch) {
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm switch control type")));
    }

    BCMDNX_IF_ERR_EXIT(rv);
exit:
    BCMDNX_FUNC_RETURN;
}


int
_bcm_dnx_switch_control_port_header_type_map(int unit,
                                             int to_bcm,
                                             int *bcm_port_header_type, 
                                             DNX_TMC_PORT_HEADER_TYPE *dnx_tmcport_header_type)
{
    int                                  rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;

    if (to_bcm) {

        switch (*dnx_tmcport_header_type) {
        case DNX_TMC_PORT_HEADER_TYPE_NONE:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_NONE;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_ETH;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_RAW;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_TM;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_PROG:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_PROG;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_CPU;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_STACKING:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_STACKING;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TDM:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_TDM;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TDM_RAW:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_TDM_RAW;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
            *bcm_port_header_type = BCM_SWITCH_PORT_HEADER_TYPE_MPLS_RAW;
            break;
        default:
           BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm port header type, to_bcm.")));
        }

    } else {

        switch (*bcm_port_header_type) {
        case BCM_SWITCH_PORT_HEADER_TYPE_NONE:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_NONE;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_ETH:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_ETH;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_RAW:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_RAW;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_TM:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_TM;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_PROG:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_PROG;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_CPU:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_CPU;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_STACKING:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_STACKING;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_TDM:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_TDM;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_TDM_RAW:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_TDM_RAW;
            break;
        case BCM_SWITCH_PORT_HEADER_TYPE_MPLS_RAW:
            *dnx_tmcport_header_type = DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW;
            break;
        default:
           BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm port header type. from_bcm")));
        }

    }

    BCMDNX_IF_ERR_EXIT(rv);
exit:
    BCMDNX_FUNC_RETURN;
}
     

int
_bcm_dnx_switch_control_port_get(int unit,
                                 bcm_port_t port,
                                 bcm_switch_control_t bcm_type,
                                 int *arg)
{
    uint32                               soc_sand_rc;

    BCMDNX_INIT_FUNC_DEFS;
    switch (bcm_type) {
    case bcmSwitchPFCClass0Queue:
    case bcmSwitchPFCClass1Queue:
    case bcmSwitchPFCClass2Queue:
    case bcmSwitchPFCClass3Queue:
    case bcmSwitchPFCClass4Queue:
    case bcmSwitchPFCClass5Queue:
    case bcmSwitchPFCClass6Queue:
    case bcmSwitchPFCClass7Queue:
        soc_sand_rc = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_pfc_tc_map_get,(unit, bcm_type - bcmSwitchPFCClass0Queue, port, arg)));
        DNX_BCM_SAND_IF_ERR_EXIT(soc_sand_rc);
        break;

    default:
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm switch control port type")));
    }
exit:
    BCMDNX_FUNC_RETURN;
}   

int
_bcm_dnx_switch_control_port_set(int unit,
                                 bcm_port_t port,
                                 bcm_switch_control_t bcm_type,
                                 int arg)
{
    uint32                               dnx_sand_rc;

    BCMDNX_INIT_FUNC_DEFS;



    switch (bcm_type) {
    case bcmSwitchPFCClass0Queue:
    case bcmSwitchPFCClass1Queue:
    case bcmSwitchPFCClass2Queue:
    case bcmSwitchPFCClass3Queue:
    case bcmSwitchPFCClass4Queue:
    case bcmSwitchPFCClass5Queue:
    case bcmSwitchPFCClass6Queue:
    case bcmSwitchPFCClass7Queue:
        dnx_sand_rc = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_pfc_tc_map_set,(unit, bcm_type - bcmSwitchPFCClass0Queue, port, arg)));
        DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rc);
        break;
    default:
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Not supported bcm switch control port type")));
    }

exit:
    BCMDNX_FUNC_RETURN;
}   


/***************************************************************/
/***************************************************************/

/* 
 * Begin BCM Functions
 *
 */
/*
 * Function:
 *      bcm_dnx_switch_temperature_monitor_get
 * Purpose:
 *      Get temperature readings on all sensors on Petra.
 * Parameters:
 *      unit -
 *        (IN) Unit number.
 *      temperature_max -
 *        (IN) Maximal number of acceptable temperature sensors
 *      temperature_array -
 *        (OUT) Pointer to array to be loaded by this procedure. Each
 *        element contains current temperature and peak temperature.
 *      temperature_count -
 *        (OUT) Pointer to memory to be loaded by this procedure. This
 *        is the number of sensors (and, therefore, the number of valid
 *        elements on temperature_array).
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
   bcm_dnx_switch_temperature_monitor_get(
                        int unit, 
                        int temperature_max, 
                        bcm_switch_temperature_monitor_t *temperature_array, 
                        int *temperature_count)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;
    DNX_SWITCH_UNIT_INIT_CHECK;

    BCMDNX_NULL_CHECK(temperature_array);
    BCMDNX_NULL_CHECK(temperature_count);
    if (temperature_max <= 0)
    {
        BCMDNX_ERR_EXIT_MSG(BCM_E_FULL, (_BSL_SOC_MSG("temperature_max expected to be bigger than 0.\n")));
    }
    rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_temp_pvt_get, (unit, temperature_max, temperature_array, temperature_count));
    BCMDNX_IF_ERR_EXIT(rv);

exit: 
   BCMDNX_FUNC_RETURN; 
}
/*
 * Function:
 *      bcm_dnx_switch_control_get
 * Purpose:
 *      Configure port-specific and device-wide operating modes.
 * Parameters:
 *      unit - (IN) Unit number.
 *      type - (IN) <UNDEF>
 *      arg - (OUT) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_dnx_switch_control_get(int unit, 
                             bcm_switch_control_t type, 
                             int *arg)
{
    int rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    DNX_SWITCH_UNIT_VALID_CHECK;
    DNX_SWITCH_UNIT_INIT_CHECK;

    DNX_SWITCH_LOCK_TAKE;

    rv = _bcm_dnx_switch_control_get(unit, type, arg);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    /* do NOT add _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE 
       (this function is called by dispatcher after every api call) !! */
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_switch_control_set
 * Purpose:
 *      Configure port-specific and device-wide operating modes.
 * Parameters:
 *      unit - (IN) Unit number.
 *      type - (IN) <UNDEF>
 *      arg - (IN) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_dnx_switch_control_set(int unit, 
                             bcm_switch_control_t type, 
                             int arg)
{
    int rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    DNX_SWITCH_UNIT_VALID_CHECK;
    DNX_SWITCH_UNIT_INIT_CHECK;

    DNX_SWITCH_LOCK_TAKE;

    rv = _bcm_dnx_switch_control_set(unit, type, arg);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_switch_control_port_get
 * Purpose:
 *      Configure port-specific and device-wide operating modes.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      type - (IN) <UNDEF>
 *      arg - (OUT) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_dnx_switch_control_port_get(int unit, 
                                  bcm_port_t port, 
                                  bcm_switch_control_t type, 
                                  int *arg)
{
    int rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    DNX_SWITCH_UNIT_VALID_CHECK;
    DNX_SWITCH_UNIT_INIT_CHECK;

    DNX_SWITCH_LOCK_TAKE;

    rv = _bcm_dnx_switch_control_port_get(unit, port, type, arg);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_switch_control_port_set
 * Purpose:
 *      Configure port-specific and device-wide operating modes.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) <UNDEF>
 *      type - (IN) <UNDEF>
 *      arg - (IN) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_dnx_switch_control_port_set(int unit, 
                                  bcm_port_t port, 
                                  bcm_switch_control_t type, 
                                  int arg)
{
    int rv = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    DNX_SWITCH_UNIT_VALID_CHECK;
    DNX_SWITCH_UNIT_INIT_CHECK;

    DNX_SWITCH_LOCK_TAKE;

    rv = _bcm_dnx_switch_control_port_set(unit, port, type, arg);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_switch_stable_register
 * Purpose:
 *      Register read/write functions for the application provided
 *      stable for Level 2 Warm Boot
 * Parameters:
 *      unit - (IN) Unit number.
 *      rf - (IN) <UNDEF>
 *      wf - (IN) <UNDEF>
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_dnx_switch_stable_register(int unit, 
                                 bcm_switch_read_func_t rf, 
                                 bcm_switch_write_func_t wf)
{
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("bcm_dnx_switch_stable_register is not available"))); 
exit:
    BCMDNX_FUNC_RETURN;
}

/***************************************************************/
/***************************************************************/

/* 
 * Switch Event Function
 *
 */

/*
 * Function:
 *    bcm_dnx_switch_event_register
 * Description:
 *    Registers a call back function for switch critical events
 * Parameters:
 *    unit        - Device unit number
 *  cb          - The desired call back function to register for critical events.
 *  userdata    - Pointer to any user data to carry on.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *    Several call back functions could be registered, they all will be called upon
 *    critical event. If registered callback is called it is adviced to log the 
 *  information and reset the chip. 
 *  Same call back function with different userdata is allowed to be registered. 
 */
int 
bcm_dnx_switch_event_register(int unit, bcm_switch_event_cb_t cb, void *userdata)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    DNX_SWITCH_LOCK_TAKE; 
    rc = soc_event_register(unit, (soc_event_cb_t)cb, userdata);
     BCMDNX_IF_ERR_EXIT(rc);

exit:
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}


/*
 * Function:
 *    bcm_dnx_switch_event_unregister
 * Description:
 *    Unregisters a call back function for switch critical events
 * Parameters:
 *    unit        - Device unit number
 *  cb          - The desired call back function to unregister for critical events.
 *  userdata    - Pointer to any user data associated with a call back function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *  If userdata = NULL then all matched call back functions will be unregistered,
 */
int 
bcm_dnx_switch_event_unregister(int unit, bcm_switch_event_cb_t cb, void *userdata)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    DNX_SWITCH_LOCK_TAKE;
    rc = soc_event_unregister(unit, (soc_event_cb_t)cb, userdata);
     BCMDNX_IF_ERR_EXIT(rc);

exit:
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *    bcm_dnx_switch_event_control_set
 * Description:
 *    Set event control
 * Parameters:
 *    unit        - Device unit number
 *  type        - Event action.
 *  value       - Event value
 * Returns:
 *      BCM_E_xxx
 */
int bcm_dnx_switch_event_control_set(
    int unit, 
    bcm_switch_event_t switch_event,
    bcm_switch_event_control_t type, 
    uint32 value)
{
    int rc = BCM_E_NONE, nof_interrupts;
    soc_interrupt_db_t* interrupts;
    int inter = 0;
    soc_block_info_t *bi;
    int bi_index = 0, inf_index=type.index, port;
    int is_valid;
    soc_block_types_t block_types;

    BCMDNX_INIT_FUNC_DEFS;

    DNX_SWITCH_LOCK_TAKE;

    if (switch_event != BCM_SWITCH_EVENT_DEVICE_INTERRUPT) {
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_SOC_MSG("supports only interrupts event")));
    }

    interrupts = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info;
    if (NULL == interrupts) {
       BCMDNX_ERR_EXIT_MSG(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    /*verify interrupt id*/
    rc = soc_nof_interrupts(unit, &nof_interrupts);
    BCMDNX_IF_ERR_EXIT(rc);

    if (type.event_id != BCM_SWITCH_EVENT_CONTROL_ALL) {
        if (type.event_id >= nof_interrupts || type.event_id < 0) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid interrupt")));
        }

        /*verify block instance*/
        if (!SOC_REG_IS_VALID(unit, interrupts[type.event_id].reg)) {
            BCMDNX_ERR_EXIT_MSG(SOC_E_INTERNAL, (_BSL_SOC_MSG("Invalid interrupt register for the device")));
        }
        block_types = SOC_REG_INFO(unit, interrupts[type.event_id].reg).block;
        if (!SOC_BLOCK_IN_LIST(block_types, SOC_BLK_CLP) && !SOC_BLOCK_IN_LIST(block_types, SOC_BLK_XLP)) {
            rc = soc_is_valid_block_instance(unit, block_types, type.index, &is_valid);
            BCMDNX_IF_ERR_EXIT(rc);
            if (!is_valid) {
               BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Unsupported block instance")));
            }
        }

        inf_index = soc_interrupt_get_block_index_from_port(unit, type.event_id, type.index);
        if (inf_index < 0) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Unsupported block instance")));
        }

    }

    /*switch case for all*/
    switch (type.action) {

    case bcmSwitchEventMask:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
#ifdef BCM_CMICM_SUPPORT
            if (value) {
                soc_cmicm_intr2_disable(unit, 0x1e);
                soc_cmicm_intr3_disable(unit, 0xFFFFFFFF);
                soc_cmicm_intr4_disable(unit, 0xFFFFFFFF);
#ifdef BCM_JER2_JERICHO_SUPPORT 
                if ( SOC_IS_JERICHO(unit) ){
                    soc_cmicm_intr5_disable(unit, 0xFFFFFFFF);
                    soc_cmicm_intr6_disable(unit, 0xFFFFFFFF);
                }
#endif /* BCM_JER2_JERICHO_SUPPORT */
            } else {
                soc_cmicm_intr2_enable(unit, 0x1e);
                soc_cmicm_intr3_enable(unit, 0xFFFFFFFF);
                soc_cmicm_intr4_enable(unit, 0xFFFFFFFF);
#ifdef BCM_JER2_JERICHO_SUPPORT 
                if ( SOC_IS_JERICHO(unit) ){
                    soc_cmicm_intr5_enable(unit, 0xFFFFFFFF);
                    soc_cmicm_intr6_enable(unit, 0xFFFFFFFF);
                }
#endif /* BCM_JER2_JERICHO_SUPPORT */
            }

#else
           BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Mask all supported only if BCM_CMICM_SUPPORT ")));
#endif  /*BCM_CMICM_SUPPORT*/
        } else {
            /* Set per interrupt */
            if (value) {
                rc = soc_interrupt_disable(unit, type.index, &(interrupts[type.event_id]));
            } else {
                rc = soc_interrupt_enable(unit, type.index, &(interrupts[type.event_id]));
            }
            BCMDNX_IF_ERR_EXIT(rc);
        }

        break;
    case bcmSwitchEventForce:
        /* Set/clear per interrupt */
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            for (inter = 0; inter < nof_interrupts; inter++) {

                for (bi_index = 0;  SOC_BLOCK_INFO(unit, bi_index).type >= 0; bi_index++) {

                    if (!SOC_INFO(unit).block_valid[bi_index]) {
                        continue;
                    }

                    bi = &(SOC_BLOCK_INFO(unit, bi_index));

                    rc = soc_interrupt_is_valid(unit, bi, &(interrupts[inter]), &is_valid);
                    BCMDNX_IF_ERR_EXIT(rc);
                    if (is_valid) {

                        port = soc_interrupt_get_intr_port_from_index(unit, inter, bi->number);

                        rc = soc_interrupt_force(unit, port, &(interrupts[inter]), 1 - value);
                        BCMDNX_IF_ERR_EXIT(rc);
                    }
                }
            }
        } else {
            rc = soc_interrupt_force(unit, type.index, &(interrupts[type.event_id]), 1 - value);
            BCMDNX_IF_ERR_EXIT(rc);
        }

        break;
    case bcmSwitchEventClear:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            if (value) {
                rc = soc_interrupt_clear_all(unit);
                BCMDNX_IF_ERR_EXIT(rc);
            } else {
               BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid event clear parameter")));
            }
        } else {
            /* Set per interrupt */
            if (value) {
                if (NULL == interrupts[type.event_id].interrupt_clear) {
                    if (interrupts[type.event_id].vector_id == 0) {
                        /*BCMDNX_ERR_EXIT_MSG(SOC_E_UNAVAIL, (_BSL_SOC_MSG("Interrupt not cleared, NULL pointer of interrupt_clear, no vector_id"))); */
                        LOG_WARN(BSL_LS_BCM_SWITCH,
                                 (BSL_META_U(unit,
                                             "Warning: Interrupt not cleared, NULL pointer of interrupt_clear, no vector_id\n")));
                    } else {
                        LOG_WARN(BSL_LS_BCM_SWITCH,
                                 (BSL_META_U(unit,
                                             "Warning: call to interrupt clear for vector pointer, nothing done\n")));
                    }
                } else {
                    rc = interrupts[type.event_id].interrupt_clear(unit, type.index, type.event_id);
                    BCMDNX_IF_ERR_EXIT(rc);
                }

            } else {
               BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid event clear parameter")));
            }
        }
        break;
        case bcmSwitchEventRead:
            if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
                BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid read parameter (event_id)")));
            }
            break;

    case bcmSwitchEventStormTimedCount:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            for (inter = 0; inter < nof_interrupts; inter++) {
                for (bi_index = 0;  SOC_BLOCK_INFO(unit, bi_index).type >= 0; bi_index++) {

                    if (!SOC_INFO(unit).block_valid[bi_index]) {
                        continue;
                    }

                    bi = &(SOC_BLOCK_INFO(unit, bi_index));
                    rc = soc_interrupt_is_valid(unit, bi, &(interrupts[inter]), &is_valid);
                    BCMDNX_IF_ERR_EXIT(rc);

                    if (is_valid) {
                        rc = soc_interrupt_storm_timed_count_set(unit, inter, value);
                        BCMDNX_IF_ERR_EXIT(rc);
                        (interrupts[inter].storm_detection_occurrences)[bi->number] = 0x0;
                    }
                }
            }
        } else {
            /* Set per interrupt */
            rc = soc_interrupt_storm_timed_count_set(unit, type.event_id, value);
            BCMDNX_IF_ERR_EXIT(rc);
            (interrupts[type.event_id].storm_detection_occurrences)[inf_index] = 0x0;
        }
        break;

    case bcmSwitchEventStormTimedPeriod:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            for (inter = 0; inter < nof_interrupts; inter++) {
                for (bi_index = 0;  SOC_BLOCK_INFO(unit, bi_index).type >= 0; bi_index++) {

                    if (!SOC_INFO(unit).block_valid[bi_index]) {
                        continue;
                    }

                    bi = &(SOC_BLOCK_INFO(unit, bi_index));
                    rc = soc_interrupt_is_valid(unit, bi, &(interrupts[inter]), &is_valid);
                    BCMDNX_IF_ERR_EXIT(rc);

                    if (is_valid) {
                        rc = soc_interrupt_storm_timed_period_set(unit, inter, value);
                        BCMDNX_IF_ERR_EXIT(rc);

                        (interrupts[inter].storm_detection_start_time)[bi->number] = 0x0;
                    }
                }
            }
        } else {
            /* Set per interrupt */
            rc = soc_interrupt_storm_timed_period_set(unit, type.event_id, value);
            BCMDNX_IF_ERR_EXIT(rc);
            (interrupts[type.event_id].storm_detection_start_time)[inf_index] = 0;
        }
        break;

    case bcmSwitchEventStormNominal:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            SOC_SWITCH_EVENT_NOMINAL_STORM(unit) = value;
            /*Warm boot buffer updating*/
            DNXC_IF_ERR_EXIT(SWITCH_ACCESS.interrupts_event_storm_nominal.set(unit, value));
            for (inter = 0; inter < nof_interrupts; inter++) {
                for (bi_index = 0;  SOC_BLOCK_INFO(unit, bi_index).type >= 0; bi_index++) {

                    if (!SOC_INFO(unit).block_valid[bi_index]) {
                        continue;
                    }

                    bi = &(SOC_BLOCK_INFO(unit, bi_index));
                    rc = soc_interrupt_is_valid(unit, bi, &(interrupts[inter]), &is_valid);
                    BCMDNX_IF_ERR_EXIT(rc);

                    if (is_valid) {
                         (interrupts[inter].storm_nominal_count)[bi->number] = 0x0;
                         (interrupts[inter].storm_detection_occurrences)[bi->number] = 0x0;
                    }
                }
            }

        } 
        else {
            /* Set per interrupt */
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid nominal storm detection parameter (event_id)")));
        }
        break;

    case bcmSwitchEventStat:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            if (value) {
                for (inter = 0; inter < nof_interrupts; inter++) {
                    for (bi_index = 0;  SOC_BLOCK_INFO(unit, bi_index).type >= 0; bi_index++) {

                        if (!SOC_INFO(unit).block_valid[bi_index]) {
                            continue;
                        }

                        bi = &(SOC_BLOCK_INFO(unit, bi_index));
                        rc = soc_interrupt_is_valid(unit, bi, &(interrupts[inter]), &is_valid);
                        BCMDNX_IF_ERR_EXIT(rc);

                        if (is_valid) {
                            (interrupts[inter].statistics_count)[bi->number] = 0x0;
                        }
                    }
                }
            } else {
               BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid statistics parameter value")));
            }
        } else {
            (interrupts[type.event_id].statistics_count)[inf_index] = 0x0;
        }
        break;

    case bcmSwitchEventLog:

        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {

            uint32 flags;

            for (inter = 0; inter < nof_interrupts; inter++) {

                rc = soc_interrupt_flags_get(unit, inter, &flags);
                BCMDNX_IF_ERR_EXIT(rc);

                if (value == 0) {
                    SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE);
                } else {
                    SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE);
                }

                rc = soc_interrupt_flags_set(unit, inter, flags);
                BCMDNX_IF_ERR_EXIT(rc);

            }
        } else {

            uint32 flags;
            /* Set per interrupt */
            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            if (value == 0) {
                SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE);
            } else {
                SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE);
            }

            rc = soc_interrupt_flags_set(unit, type.event_id, flags);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;

    case bcmSwitchEventCorrActOverride:
    /* Value - 0 : Only bcm callback will be called for this interrupt.
       Value - 1 : Only user callback will be called for this interrupt. At this mode BCM driver will only print the interrupt information for logging.
       Value - 2 : User call back will be called immediately after bcm callback. */

        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {

            uint32 flags;

            for (inter = 0; inter < nof_interrupts; inter++) {

                rc = soc_interrupt_flags_get(unit, inter, &flags);
                BCMDNX_IF_ERR_EXIT(rc);

                if(SOC_IS_JERICHO(unit)) {
                    if (value == 2) {
                        flags |= SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB;
                    } else {
                        flags &= ~SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB;
                    }
                }

                if (value == 0) {
                    SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE);
                } else {
                    SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE);
                }

                rc = soc_interrupt_flags_set(unit, inter, flags);
                BCMDNX_IF_ERR_EXIT(rc);

            }
        } else {

            uint32 flags;

            /* Set per interrupt */
            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            if(SOC_IS_JERICHO(unit)) {
                if (value == 2) {
                    flags |= SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB;
                } else {
                    flags &= ~SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB;
                }
            }

            if (value == 0) {
                SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE);
            } else {
                SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE);
            }

            rc = soc_interrupt_flags_set(unit, type.event_id, flags);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;

    case bcmSwitchEventPriority:

        if (value > SOC_INTERRUPT_DB_FLAGS_PRIORITY_MAX_VAL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Priority value is out af range")));
        }

        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {

            uint32 flags;

            for (inter = 0; inter < nof_interrupts; inter++) {

                rc = soc_interrupt_flags_get(unit, inter, &flags);
                BCMDNX_IF_ERR_EXIT(rc);

                SHR_BITCOPY_RANGE(&flags, SOC_INTERRUPT_DB_FLAGS_PRIORITY_BITS_LSB, &value, 0, SOC_INTERRUPT_DB_FLAGS_PRIORITY_BITS_LEN);

                rc = soc_interrupt_flags_set(unit, inter, flags);
                BCMDNX_IF_ERR_EXIT(rc);
            }
        } else {

            uint32 flags;

            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            SHR_BITCOPY_RANGE(&flags, SOC_INTERRUPT_DB_FLAGS_PRIORITY_BITS_LSB, &value, 0, SOC_INTERRUPT_DB_FLAGS_PRIORITY_BITS_LEN);

            rc = soc_interrupt_flags_set(unit, type.event_id, flags);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;

    case bcmSwitchEventUnmaskAndClearDisable:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {

            uint32 flags;

            for (inter = 0; inter < nof_interrupts; inter++) {

                rc = soc_interrupt_flags_get(unit, inter, &flags);
                BCMDNX_IF_ERR_EXIT(rc);

                if (value == 0) {
                    SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_UNMASK_AND_CLEAR_DISABLE_BITS);
                } else {
                    SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_UNMASK_AND_CLEAR_DISABLE_BITS);
                }

                rc = soc_interrupt_flags_set(unit, inter, flags);
                BCMDNX_IF_ERR_EXIT(rc);

            }
        } else {

            uint32 flags;
            /* Set per interrupt */
            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            if (value == 0) {
                SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_UNMASK_AND_CLEAR_DISABLE_BITS);
            } else {
                SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_UNMASK_AND_CLEAR_DISABLE_BITS);
            }

            rc = soc_interrupt_flags_set(unit, type.event_id, flags);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;
    case bcmSwitchEventForceUnmask:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {

            uint32 flags;

            for (inter = 0; inter < nof_interrupts; inter++) {

                rc = soc_interrupt_flags_get(unit, inter, &flags);
                BCMDNX_IF_ERR_EXIT(rc);

                if (value == 0) {
                    SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_FORCE_UNMASK_BITS);
                } else {
                    SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_FORCE_UNMASK_BITS);
                }

                rc = soc_interrupt_flags_set(unit, inter, flags);
                BCMDNX_IF_ERR_EXIT(rc);

            }
        } else {

            uint32 flags;
            /* Set per interrupt */
            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            if (value == 0) {
                SHR_BITCLR(&flags, SOC_INTERRUPT_DB_FLAGS_FORCE_UNMASK_BITS);
            } else {
                SHR_BITSET(&flags, SOC_INTERRUPT_DB_FLAGS_FORCE_UNMASK_BITS);
            }

            rc = soc_interrupt_flags_set(unit, type.event_id, flags);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;

    default:
       BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_BCM_MSG_STR("Unsupported control")));
        break;
    }

exit:
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *    bcm_dnx_switch_event_control_get
 * Description:
 *    Get event control
 * Parameters:
 *    unit        - Device unit number
 *  type        - Event action.
 *  value       - Event value
 * Returns:
 *      BCM_E_xxx
 */
int bcm_dnx_switch_event_control_get(
    int unit, 
    bcm_switch_event_t switch_event,
    bcm_switch_event_control_t type, 
    uint32 *value)
{
    int rc = BCM_E_NONE, nof_interrupts;
    soc_interrupt_db_t* interrupts;
    int inter_get;
    int is_enable;
    soc_block_types_t block_types;
    int is_valid;
    int inf_index = type.index;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(value);
    DNX_SWITCH_LOCK_TAKE;

    if (switch_event != BCM_SWITCH_EVENT_DEVICE_INTERRUPT) {
       BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_SOC_MSG("supports only interrupts event")));
    }

    interrupts = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info;
    if (NULL == interrupts) {
       BCMDNX_ERR_EXIT_MSG(SOC_E_UNAVAIL, (_BSL_SOC_MSG("No interrupts for device")));
    }

    rc = soc_nof_interrupts(unit, &nof_interrupts);
    BCMDNX_IF_ERR_EXIT(rc);



    if (type.event_id != BCM_SWITCH_EVENT_CONTROL_ALL) {
        if (type.event_id >= nof_interrupts || type.event_id < 0) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid interrupt")));
        }

        /*verify block instance*/
        if (!SOC_REG_IS_VALID(unit, interrupts[type.event_id].reg)) {
            BCMDNX_ERR_EXIT_MSG(SOC_E_INTERNAL, (_BSL_SOC_MSG("Invalid interrupt register for the device")));
        }

        block_types = SOC_REG_INFO(unit, interrupts[type.event_id].reg).block;

        if (!SOC_BLOCK_IN_LIST(block_types, SOC_BLK_CLP) && !SOC_BLOCK_IN_LIST(block_types, SOC_BLK_XLP)) {
            rc = soc_is_valid_block_instance(unit, block_types, type.index, &is_valid);
            BCMDNX_IF_ERR_EXIT(rc);
            if (!is_valid) {
               BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Unsupported block instance")));
            }
        }

        inf_index = soc_interrupt_get_block_index_from_port(unit, type.event_id, type.index);
        if (inf_index < 0) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Unsupported block instance")));
        }
    }
   LOG_VERBOSE(BSL_LS_BCM_SWITCH,
               (BSL_META_U(unit,
                           "%s(): interrupt id=%3d, name=%s\n"), FUNCTION_NAME(), type.event_id, interrupts[type.event_id].name));

    switch (type.action) {
    case bcmSwitchEventMask:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            rc = soc_interrupt_is_all_mask(unit, (int *)value);
            BCMDNX_IF_ERR_EXIT(rc);
        } else {
            /* Get per interrupt */
            rc = soc_interrupt_is_enabled(unit, type.index, &(interrupts[type.event_id]), &is_enable);
            BCMDNX_IF_ERR_EXIT(rc);
            *value = (is_enable == 0);
        }
        break;

    case bcmSwitchEventForce:
        /* Set/clear per interrupt */
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid <controll all> event parameter for force option")));
        } else {
            rc = soc_interrupt_force_get(unit, type.index, &(interrupts[type.event_id]), &is_enable);
            BCMDNX_IF_ERR_EXIT(rc);
            *value = is_enable;
        }
        break;

    case bcmSwitchEventClear:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            rc = soc_interrupt_is_all_clear(unit, (int *)value);
            BCMDNX_IF_ERR_EXIT(rc);
        } else {
            /* Get per interrupt */
            rc = soc_interrupt_get(unit, type.index, &(interrupts[type.event_id]), &inter_get);
            BCMDNX_IF_ERR_EXIT(rc);
            *value = (inter_get == 0);
        }
        break;

    case bcmSwitchEventRead:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid read parameter (event_id)")));
        } else {
            {
                /* Get per interrupt */
                rc = soc_interrupt_get(unit, type.index, &(interrupts[type.event_id]), &inter_get);
                BCMDNX_IF_ERR_EXIT(rc);
                *value = (inter_get == 1);
            }
        }
        break;

    case bcmSwitchEventStormTimedPeriod:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Storm Timed Period parameter (event_id)")));
        } else {
            /* Get per interrupt */
            rc = soc_interrupt_storm_timed_period_get(unit, type.event_id, value);
            BCMDNX_IF_ERR_EXIT(rc);
        }
        break;

    case bcmSwitchEventStormTimedCount:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Storm Timed Count parameter (event_id)")));
        } else {
            /* Get per interrupt */
            rc = soc_interrupt_storm_timed_count_get(unit, type.event_id, value);
            BCMDNX_IF_ERR_EXIT(rc);

        }
        break;

    case bcmSwitchEventStormNominal:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
            *value = SOC_SWITCH_EVENT_NOMINAL_STORM(unit);
        } else {
            /* Get per interrupt */
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid nominal storm parameter (event_id)")));
        }
        break;

    case bcmSwitchEventStat:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Statistics Control parameter (event_id)")));
        } else {
            /* Get per interrupt */
            *value = (interrupts[type.event_id].statistics_count)[inf_index];
        }
        break;

    case bcmSwitchEventLog:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Print Control parameter (event_id)")));
        } else {

            uint32 flags;
            /* Get per interrupt */

            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            *value =  (SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_PRINT_ENABLE) != 0) ? 0x1 : 0x0;
        }
        break;

    case bcmSwitchEventCorrActOverride:
    /* Value - 0 : Only bcm callback will be called for this interrupt.
       Value - 1 : Only user callback will be called for this interrupt. At this mode BCM driver will only print the interrupt information for logging.
       Value - 2 : User call back will be called immediately after bcm callback. */
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Print Control parameter (event_id)")));
        } else {
            /* Get per interrupt */
            uint32 flags;

            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            if(SOC_IS_JERICHO(unit)) {
                if (flags & SOC_INTERRUPT_DB_FLAGS_BCM_AND_USR_CB) {
                    *value = 0x2;
                    break;
                } 
            }
            *value = (SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_CORR_ACT_OVERRIDE_ENABLE) != 0) ? 0x1 : 0x0;
        }
        break;

    case bcmSwitchEventPriority:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Print Control parameter (event_id)")));
        } else {
            uint32 flags;

            /* Get per interrupt */
            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            *value = ((flags & SOC_INTERRUPT_DB_FLAGS_PRIORITY_MASK) >> SOC_INTERRUPT_DB_FLAGS_PRIORITY_BITS_LSB);
        }
        break;

    case bcmSwitchEventUnmaskAndClearDisable:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Print Control parameter (event_id)")));
        } else {

            uint32 flags;
            /* Get per interrupt */

            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            *value =  (SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_UNMASK_AND_CLEAR_DISABLE_BITS) != 0) ? 0x1 : 0x0;
        }
        break;
    case bcmSwitchEventForceUnmask:
        if (type.event_id == BCM_SWITCH_EVENT_CONTROL_ALL) {
           BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Invalid Print Control parameter (event_id)")));
        } else {

            uint32 flags;
            /* Get per interrupt */

            rc = soc_interrupt_flags_get(unit, type.event_id, &flags);
            BCMDNX_IF_ERR_EXIT(rc);

            *value =  (SHR_BITGET(&flags, SOC_INTERRUPT_DB_FLAGS_FORCE_UNMASK_BITS) != 0) ? 0x1 : 0x0;
        }
        break;

    default:
       BCMDNX_ERR_EXIT_MSG(SOC_E_PARAM, (_BSL_SOC_MSG("Unsupported control")));
        break;
    }

exit:
    DNX_SWITCH_LOCK_RELEASE;
    BCMDNX_FUNC_RETURN;
}

