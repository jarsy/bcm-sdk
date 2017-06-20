/*
 * $Id: switch.c,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        switch.c
 * Purpose:     BCM definitions  for bcm_switch_control and
 *              bcm_switch_port_control functions
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_scoreboard.h>
#include <soc/sbx/hal_ka_auto.h>

#include <bcm/switch.h>
#include <bcm/error.h>

#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/multicast.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/common/link.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0
#endif /* BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_WARM_BOOT_SUPPORT

#define _BCM_SYNC_SUCCESS(unit) \
        (BCM_SUCCESS(rv) || (BCM_E_INIT == rv) || (BCM_E_UNAVAIL == rv))

int
bcm_sbx_switch_state_sync(int unit, int arg)
{
    int rv = BCM_E_UNAVAIL;
    int sync;

    if (!arg) {
        LOG_CLI((BSL_META_U(unit,
                            "arg=0. Skipping state sync \n")));
        return BCM_E_NONE; /* nothing to sync */
    }
    rv = BCM_E_NONE;

    sync = 0; /* do not sync after each individual module */
    /* SOC state */
    BCM_IF_ERROR_RETURN(soc_sbx_wb_sync(unit, sync));
    if (SOC_IS_SBX_QE2000(unit) || SOC_IS_SBX_SIRIUS(unit)) {
        BCM_IF_ERROR_RETURN(soc_sbx_wb_connect_state_sync(unit, sync));
    }

    /* BCM State */
    BCM_IF_ERROR_RETURN(bcm_linkscan_sync(unit, sync));
    /* PORT State */
    BCM_IF_ERROR_RETURN(bcm_sbx_wb_port_state_sync(unit, sync));
    /* FABRIC State */
    BCM_IF_ERROR_RETURN(bcm_sbx_wb_fabric_state_sync(unit, sync));
    /* STACK State */
    BCM_IF_ERROR_RETURN(bcm_sbx_wb_stack_state_sync(unit, sync));
    /* COSQ State */
    BCM_IF_ERROR_RETURN(bcm_sbx_wb_cosq_state_sync(unit, sync));
    if (SOC_IS_SIRIUS(unit)) {
        /* Device TRUNK State */
        BCM_IF_ERROR_RETURN(bcm_sbx_wb_trunk_state_sync(unit, sync));
        /* Device MULTICAST State */
        BCM_IF_ERROR_RETURN(bcm_sbx_wb_multicast_state_sync(unit, sync));
    }

    /* Now commit the scache to Persistent memory */
    rv = soc_scache_commit(unit);
    if (rv != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: Error(%s) sync'ing scache to Persistent memory. \n"),
                 FUNCTION_NAME(), soc_errmsg(rv)));
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

int
bcm_sbx_switch_control_set(int unit, bcm_switch_control_t type, int arg)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_WARM_BOOT_SUPPORT
    int stable_select;
#endif

    LOG_CLI((BSL_META_U(unit,
                        "bcm_sbx_switch_control_set %d 0x%x\n"), type, arg));

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);
    switch (type) {
    case bcmSwitchControlSync:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = bcm_sbx_switch_state_sync(unit, arg);
#else
	    rv = BCM_E_UNAVAIL;
#endif
        break;
    case bcmSwitchControlAutoSync:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = bcm_sbx_switch_control_get(unit, bcmSwitchStableSelect, 
                                        &stable_select);
        if (BCM_SUCCESS(rv)) {
            if (stable_select != BCM_SWITCH_STABLE_NONE) {
                SOC_CONTROL(unit)->autosync = (arg ? 1 : 0);
            } else {
                rv = BCM_E_NOT_FOUND;
            }
        }
#endif
        break;
    case bcmSwitchStableSelect:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_set(unit, arg, 0);
#endif
        break;
    case bcmSwitchStableSize:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_size_set(unit, arg);
#endif
        break;
    case bcmSwitchWarmBoot:
#ifdef BCM_WARM_BOOT_SUPPORT
        /* If true, set the Warm Boot state; clear otherwise */
        if (arg) {
            SOC_WARM_BOOT_START(unit);
        } else {
            SOC_WARM_BOOT_DONE(unit);
        }
        rv = BCM_E_NONE; 
#endif
        break;
    default: 
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_control_set, (unit, type, arg));
        break;
    }
    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_switch_control_get(int unit, bcm_switch_control_t type, int *arg)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 flags;
#endif

    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    switch (type) {
    case bcmSwitchAutoQueues:
        *arg = SOC_SBX_CFG(unit)->bcm_cosq_init;
        rv = BCM_E_NONE;
        break;
    case bcmSwitchControlAutoSync:
#ifdef BCM_WARM_BOOT_SUPPORT
        *arg = SOC_CONTROL(unit)->autosync;
        rv = BCM_E_NONE;
#endif
        break;
    case bcmSwitchStableSelect:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_get(unit, arg, &flags);
#endif
        break;
    case bcmSwitchStableSize:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_size_get(unit, arg);
#endif
        break;
    case bcmSwitchStableUsed:
#ifdef BCM_WARM_BOOT_SUPPORT
        rv = soc_stable_used_get(unit, arg);
#endif
        break;
    case bcmSwitchWarmBoot:
#ifdef BCM_WARM_BOOT_SUPPORT
        *arg = SOC_WARM_BOOT(unit);
        rv = BCM_E_NONE;
#endif
        break;
    default:
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_control_get, (unit, type, arg));
        break;
    }
    return rv;
}


/*
 * Function:
 *      bcm_switch_event_register
 * Description:
 *      Registers a call back function for switch critical events
 * Parameters:
 *      unit        - Device unit number
 *  cb          - The desired call back function to register for critical events.
 *  userdata    - Pointer to any user data to carry on.
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *      Several call back functions could be registered, they all will be called upon
 *      critical event. If registered callback is called it is adviced to log the 
 *  information and reset the chip. 
 *  Same call back function with different userdata is allowed to be registered. 
 */
int 
bcm_sbx_switch_event_register(int unit, 
			      bcm_switch_event_cb_t cb, 
                              void *userdata)
{
    return MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_event_register, (unit, cb, userdata));
}


/*
 * Function:
 *      bcm_switch_event_unregister
 * Description:
 *      Unregisters a call back function for switch critical events
 * Parameters:
 *      unit        - Device unit number
 *  cb          - The desired call back function to unregister for critical events.
 *  userdata    - Pointer to any user data associated with a call back function
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      
 *  If userdata = NULL then all matched call back functions will be unregistered,
 */
int 
bcm_sbx_switch_event_unregister(int unit, 
				bcm_switch_event_cb_t cb, 
                                void *userdata)
{
    return MBCM_SBX_DRIVER_CALL(unit, mbcm_switch_event_unregister, (unit, cb, userdata));
}
