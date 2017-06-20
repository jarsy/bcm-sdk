/*
 * $Id: stack.c,v 1.75 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stack.c
 * Purpose:     BCM level APIs for stacking applications
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/sirius.h>

#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/l3.h>
#include <bcm_int/sbx/error.h>

#include <bcm/error.h>
#include <bcm/stack.h>

#include <bcm_int/control.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif  /*  BCM_FE2000_P3_SUPPORT */
#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#endif /* BCM_CALADAN3_G3P1_SUPPORT */

#if defined(BCM_CALADAN3_G3P1_SUPPORT)
extern int bcm_caladan3_port_modid_set(int unit, bcm_module_t modid);
extern int _bcm_caladan3_l3_modid_set(int unit, int modid);
#endif  /* BCM_CALADAN3_G3P1_SUPPORT*/

STATIC bcm_sbx_stack_state_t stack_state[SOC_MAX_NUM_DEVICES];
STATIC int _sbx_stk_port_set(int unit, bcm_port_t port, uint32 flags);

int
bcm_sbx_stk_init(int unit)
{
    int i;
    int rv;

    LOG_VERBOSE(BSL_LS_BCM_STK,
                (BSL_META_U(unit,
                            "STK %d: Init\n"),
                 unit));
    COMPILER_REFERENCE(rv);

    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (SOC_IS_SBX_CALADAN3(unit)) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        if (SOC_WARM_BOOT(unit)) {
            uint32 mod = -1;
            int rv;

            rv = soc_sbx_g3p1_node_get(unit, &mod);
            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_STK,
                          (BSL_META_U(unit,
                                      "Failed to recover module id: %s\n"),
                           bcm_errmsg(rv)));
            }
            SOC_SBX_CONTROL(unit)->module_id0 = mod;
        }
#endif  /*  BCM_FE2000_P3_SUPPORT */
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    SOC_SBX_STATE(unit)->stack_state = &stack_state[unit];
    SOC_SBX_STATE(unit)->stack_state->gport_map = NULL;

    for (i=0; i < BCM_STK_MAX_MODULES; i++) {
	SOC_SBX_STATE(unit)->stack_state->protocol[i] = bcmModuleProtocolNone;
	SOC_SBX_STATE(unit)->stack_state->is_module_enabled[i] = FALSE;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    rv = bcm_sbx_wb_stack_state_init(unit);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_STK,
                  (BSL_META_U(unit,
                              "%s: error in WarmBoot stack state init \n"), 
                   FUNCTION_NAME()));
    }
#endif

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

/* Linked list of callback registrants */
typedef struct _sbx_stk_callback_s {
    struct _sbx_stk_callback_s *next;
    bcm_stk_cb_f fn;
    void *cookie;
} _sbx_stk_callback_t;

static _sbx_stk_callback_t *_sbx_stk_cb;
static _sbx_stk_callback_t *_sbx_stk_cb_tail;

int
bcm_sbx_stk_update_callback_register(int unit,
                                     bcm_stk_cb_f cb,
                                     void *cookie)
{
    _sbx_stk_callback_t *node;
    int rv = BCM_E_NONE;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }
    
    BCM_SBX_LOCK(unit);
    /* See if already present */
    node = _sbx_stk_cb;
    while (node != NULL) {
        if (node->fn == cb && node->cookie == cookie) {
            break; /* Found it */
        }
        node = node->next;
    }

    if (node == NULL) { /* Not there yet */
        node = sal_alloc(sizeof(*node), "bcm_sbx_stk_cb");
        if (node != NULL) {
            node->fn = cb;
            node->cookie = cookie;
            node->next = NULL;

            /* Enqueue at tail */
            if (_sbx_stk_cb_tail == NULL) {
                _sbx_stk_cb = node;
            } else {
                _sbx_stk_cb_tail->next = node;
            }
            _sbx_stk_cb_tail = node;
        } else {
            rv = BCM_E_MEMORY;
        }
    }
    BCM_SBX_UNLOCK(unit);

    return rv;
}

int
bcm_sbx_stk_update_callback_unregister(int unit,
                                       bcm_stk_cb_f cb,
                                       void *cookie)
{
    _sbx_stk_callback_t *node;
    _sbx_stk_callback_t *prev;
    int                 rv;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }
    
    BCM_SBX_LOCK(unit);
    node = _sbx_stk_cb;
    prev = NULL;

    /* Scan list for match of both function and cookie */
    while (node != NULL) {
        if ((node->fn == cb) && (node->cookie == cookie)) {
            break; /* Found it */
        }
        prev = node;
        node = node->next;
    }

    if (node != NULL) { /* Found */
        if (prev == NULL) { /* First on list */
            _sbx_stk_cb = node->next;
        } else { /* Update previous */
            prev->next = node->next;
        }
        if (_sbx_stk_cb_tail == node) { /* Was last on list */
            _sbx_stk_cb_tail = prev;
        }
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_NOT_FOUND;
    }
    BCM_SBX_UNLOCK(unit);

    if (node != NULL) {
        sal_free(node);
    }

    return (rv);
}


/****************************************************************
 * The ugly details of adding stack ports
 ****************************************************************/

STATIC int
_sbx_soc_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    if (SOC_IS_SIRIUS(unit)) {
#if defined(BCM_SIRIUS_SUPPORT)
        if (IS_ST_PORT(unit, port) || flags == 0) {
            /* Only stack ports may be set (flags != 0), but any port
               may be unset (flags == 0) */
            return BCM_E_NONE;
        }
#endif
    }

    return BCM_E_UNAVAIL;
}

/*
 * Force stack ports into ptable tagged bitmap and out of untagged bitmap
 * Note that if a device has no ethernet ports, this will just return.
 */
STATIC int
_bcm_sbx_stk_ptable_update(int unit)
{
    int port;

    PBMP_E_ITER(unit, port) {
        /* 
         * SBX devices do not support ptable
         * Currently nothing to do in here.
         */
    }

    return BCM_E_NONE;
}


STATIC int
_sbx_stk_update_callbacks(int unit, bcm_port_t port, uint32 flags)
{
    _sbx_stk_callback_t *node;

    /* Internal port-table update (for untagged port setting */
    BCM_IF_ERROR_RETURN(_bcm_sbx_stk_ptable_update(unit));

    /* Make callbacks.  Assumes lock is held by caller. */
    node = _sbx_stk_cb;
    while (node != NULL) {
        (node->fn)(unit, port, flags, node->cookie);
        node = node->next;
    }

    return BCM_E_NONE;
}


STATIC int
_sbx_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    int rv = BCM_E_NONE;

    /* Check if SL stacking should be enabled on the device */
    if ((!SOC_SL_MODE(unit)) && (flags & BCM_STK_ENABLE) &&
            (flags & BCM_STK_SL)) {
        return BCM_E_UNAVAIL;
    }

    /* Program the port appropriately */
    BCM_IF_ERROR_RETURN(_sbx_soc_stk_port_set(unit, port, flags));

    /* Record old stack port states; make changes to current */
    BCM_PBMP_ASSIGN(SOC_PBMP_STACK_PREVIOUS(unit), SOC_PBMP_STACK_CURRENT(unit));

    if (flags & BCM_STK_ENABLE) {
        SOC_PBMP_PORT_ADD(SOC_PBMP_STACK_CURRENT(unit), port);
        if (flags & BCM_STK_INACTIVE) {
            SOC_PBMP_PORT_ADD(SOC_PBMP_STACK_INACTIVE(unit), port);
        } else {
            SOC_PBMP_PORT_REMOVE(SOC_PBMP_STACK_INACTIVE(unit), port);
        }
    } else {
        SOC_PBMP_PORT_REMOVE(SOC_PBMP_STACK_CURRENT(unit), port);
        SOC_PBMP_PORT_REMOVE(SOC_PBMP_STACK_INACTIVE(unit), port);
    }

    /* Always make callbacks; callbacks are responsible for determining
     * whether or not anything needs to be done.
     */
    rv = _sbx_stk_update_callbacks(unit, port, flags);

    return rv;
}


/*
 * Function:
 *      bcm_sbx_stk_port_set
 * Purpose:
 *      Set stacking mode for a port
 * Parameters:
 *      unit     BCM device number
 *      port     BCM port number on device
 *      flags    See bcm/stack.h
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If neither simplex or duplex is specified, port should be
 *      considered "unresolved"; for example, if discovery has not
 *      completed yet.  In general, this should not change a port's
 *      settings, but it may affect the port's setup.
 *
 *      If port < 0, use the unique IPIC_PORT if present
 *
 *      If flags == 0, then the port does not have to be a stack port.
 */

int
bcm_sbx_stk_port_set(int unit, bcm_port_t port, uint32 flags)
{
    int rv = BCM_E_NONE;
    int txp, rxp;

    LOG_VERBOSE(BSL_LS_BCM_STK,
                (BSL_META_U(unit,
                            "STK %d: Port set: p %d. flags 0x%x\n"),
                 unit, port, flags));

    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        LOG_WARN(BSL_LS_BCM_STK,
                 (BSL_META_U(unit,
                             "STK: %s unit %d\n"),
                  SOC_UNIT_VALID(unit)?"Invalid":"Remote", unit));
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_INVALID == port) {
        port = IPIC_PORT(unit);
    } else if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }

    if (!IS_PORT(unit, port)) {
        LOG_WARN(BSL_LS_BCM_STK,
                 (BSL_META_U(unit,
                             "STK: invalid port (%d,%d)\n"),
                  unit, port));
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);
    if (flags & (BCM_STK_ENABLE | BCM_STK_CAPABLE)) {
        if (!IS_ST_PORT(unit, port)) {

            /* Setting a non-HG port to stacking implies SL mode */
            if ((flags & BCM_STK_HG) || !IS_GE_PORT(unit, port)) {
                LOG_WARN(BSL_LS_BCM_STK,
                         (BSL_META_U(unit,
                                     "STK: Invalid SL stk cfg. unit %d, port %d\n"),
                          unit, port));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }
            flags |= BCM_STK_SL; /* Set courtesy flag for callbacks */

            /* Turn pause off on SL stack ports */
            rv = bcm_port_pause_get(unit, port, &txp, &rxp);
            if (BCM_FAILURE(rv)) {
                LOG_WARN(BSL_LS_BCM_STK,
                         (BSL_META_U(unit,
                                     "STK: bcm_port_pause_get failure (%d)\n"),
                          rv));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }

            if ((txp != 0) || (rxp != 0)) {
                rv = bcm_port_pause_set(unit, port, 0, 0);
                if (BCM_FAILURE(rv)) {
                    LOG_WARN(BSL_LS_BCM_STK,
                             (BSL_META_U(unit,
                                         "STK: bcm_port_pause_set failure (%d)\n"),
                              rv));
                    BCM_SBX_UNLOCK(unit);
                    return rv;
                }
            }
        } else { /* HG port/HG2 over GE port */
            if (flags & BCM_STK_SL) {
                LOG_WARN(BSL_LS_BCM_STK,
                         (BSL_META_U(unit,
                                     "STK: Invalid HG stk cfg. unit %d, port %d\n"),
                          unit, port));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_PARAM;
            }
            flags |= BCM_STK_HG; /* Set courtesy flag for callbacks */
        }

        if ((flags & BCM_STK_SIMPLEX) && (flags & BCM_STK_DUPLEX)) {
            LOG_WARN(BSL_LS_BCM_STK,
                     (BSL_META_U(unit,
                                 "STK: Dimplex not supported. unit %d, port %d\n"),
                      unit, port));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    }

    if ((flags & BCM_STK_CUT) && (flags & BCM_STK_SL)) {
        flags |= BCM_STK_INACTIVE;
    }

    rv = _sbx_stk_port_set(unit, port, flags);

#ifdef BCM_WARM_BOOT_SUPPORT
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 1;
    SOC_CONTROL_UNLOCK(unit);
#endif

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_stk_mode_get(int unit,
                     uint32 *flags)
{
    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }
    
    if (SOC_SL_MODE(unit)) {
        *flags = BCM_STK_SL;
    } else if (BCM_PBMP_NOT_NULL(PBMP_ST_ALL(unit))) {
        /* HG/HG2 stacking */
        *flags = BCM_STK_HG;
    } else {
        *flags = BCM_STK_NONE;
    }
    
    return BCM_E_NONE;
}

int
bcm_sbx_stk_mode_set(int unit,
                     uint32 flags)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_port_get(int unit,
                     bcm_port_t port,
                     uint32 *flags)
{
    if ((!SOC_UNIT_VALID(unit)) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_INVALID == port) {     /* BCM_STK_USE_HG_IF */
        port = IPIC_PORT(unit);
    } else if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }

    if (!IS_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    *flags = 0;
    if (BCM_PBMP_MEMBER(SOC_PBMP_STACK_CURRENT(unit), port)) {
        *flags |= BCM_STK_ENABLE;
        if (BCM_PBMP_MEMBER(SOC_PBMP_STACK_INACTIVE(unit), port)) {
            *flags |= BCM_STK_INACTIVE;
        }
        if (IS_ST_PORT(unit, port)) {
            *flags |= BCM_STK_HG;
        } else {
            *flags |= BCM_STK_SL;
        }
    } else {
        *flags |= BCM_STK_NONE;
    }

    return BCM_E_NONE;
}

int
bcm_sbx_stk_sl_simplex_count_set(int unit,
                                 int count)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    LOG_VERBOSE(BSL_LS_BCM_STK,
                (BSL_META_U(unit,
                            "STK %d: SL count set to %d\n"),
                 unit, count));

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_sl_simplex_count_get(int unit,
                                 int *count)
{
    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) || (!BCM_IS_LOCAL(unit))) {
        return BCM_E_UNIT;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modid_set(int unit,
                      int modid)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        SOC_SBX_CONTROL(unit)->module_id0 = modid;
#if defined(BCM_CALADAN3_G3P1_SUPPORT)
        if (SOC_IS_SBX_CALADAN3(unit)) {
            rv = bcm_caladan3_port_modid_set(unit, modid);

            if (rv == BCM_E_NONE) {
                rv = _bcm_caladan3_l3_modid_set(unit, modid);
            }

	    BCM_SBX_UNLOCK(unit);
            return rv;
        }
#endif  /* BCM_CALADAN3_G3P1_SUPPORT*/

	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    } 

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_modid_set, (unit, modid)));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_modid_get(int unit,
                      int *modid)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        *modid = SOC_SBX_CONTROL(unit)->module_id0;
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_modid_get, (unit, modid)));
    
    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_modid_count(int unit,
                        int *num_modid)
{
    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit) || SOC_IS_SBX_QE(unit)) {
        *num_modid = 1;
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_my_modid_set(int unit,
                         int my_modid)
{
    int rv;

    BCM_SBX_LOCK(unit);

    if ( !SOC_MODID_ADDRESSABLE(unit, my_modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_BADID;
    }

    if (SOC_IS_SBX_FE(unit)) {
        SOC_SBX_CONTROL(unit)->module_id0 = my_modid;
#if defined(BCM_CALADAN3_G3P1_SUPPORT)
        if (SOC_IS_SBX_CALADAN3(unit)) {
            rv = bcm_caladan3_port_modid_set(unit, my_modid);

            if (rv == BCM_E_NONE) {
                rv = _bcm_caladan3_l3_modid_set(unit, my_modid);
            }

	    BCM_SBX_UNLOCK(unit);
            return rv;
        }
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }
    
    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_my_modid_set, (unit, my_modid)));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_my_modid_get(int unit,
                         int *my_modid)
{
    int rv;

    if (SOC_IS_SBX_FE(unit)) {
        *my_modid = SOC_SBX_CONTROL(unit)->module_id0;
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_my_modid_get, (unit, my_modid)));

    return rv;
}

int
bcm_sbx_stk_modport_set(int unit,
                        int modid,
                        bcm_port_t port)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_get_all(int unit,
                        int modid,
                        int port_max,
                        bcm_port_t *port_array,
                        int *port_count)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_get(int unit,
                        int modid,
                        bcm_port_t *port)
{
    *port = -1;
    return bcm_sbx_stk_modport_get_all(unit, modid, 1, port, NULL);
}

int
bcm_sbx_stk_modport_clear(int unit,
                          int modid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_clear_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_add(int unit,
                        int modid,
                        bcm_port_t port)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_modport_delete(int unit,
                           int modid,
                           bcm_port_t port)
{
    BCM_SBX_LOCK(unit);

    if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stk_module_enable(int unit,
                          bcm_module_t mod,
                          int nports,
                          int enable)
{
    int node;
    int rv = BCM_E_NONE;


    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);

    if (!BCM_STK_MOD_IS_NODE(mod)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if ((nports != -1) &&
	(nports < 0)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (SOC_IS_SBX_FE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    if (!(BCM_STK_MOD_IS_NODE(mod))) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    node = BCM_STK_MOD_TO_NODE(mod);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_module_enable, (unit, mod, nports, enable));

    if (rv != BCM_E_NONE) {
	BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    /* NOTE: If in future this API is supported for other devices              */
    /*       the following data structure will have to be accordingly modified */
    SOC_SBX_STATE(unit)->stack_state->is_module_enabled[node] = (enable == FALSE) ? FALSE: TRUE;
    rv = bcm_sbx_update_templates(unit, 0);

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_stk_get_modules_enabled(int unit, int *nbr_modules_enabled)
{
    int node;
    int rv = BCM_E_NONE;

    if (nbr_modules_enabled == NULL) {
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);

    (*nbr_modules_enabled) = 0;

    for (node = 0; node < BCM_STK_MAX_MODULES; node++) {
        if (SOC_SBX_STATE(unit)->stack_state->is_module_enabled[node] == TRUE) {
            (*nbr_modules_enabled)++;
        }
    }

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_stk_module_protocol_set(int unit,
                                bcm_module_t mod,
                                bcm_module_protocol_t protocol)
{
    int node;
    int rv;


    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) ||
	(SOC_SBX_STATE(unit)->stack_state != &stack_state[unit])) {
        return BCM_E_UNIT;
    }
    
    BCM_SBX_LOCK(unit);
    if (!BCM_STK_MOD_IS_NODE(mod)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if ((protocol < bcmModuleProtocolNone) ||
	(protocol > bcmModuleProtocolCustom1)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    /* additional consistency checks */
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol1) &&
	     (protocol != bcmModuleProtocol2) ) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
        if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol3) &&
	     (protocol != bcmModuleProtocol4) && (protocol != bcmModuleProtocol5) ) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) {
        if ( (SOC_SBX_CFG(unit)->module_custom1_in_system != TRUE) &&
                                         (protocol == bcmModuleProtocolCustom1) ) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
        else if ( (protocol != bcmModuleProtocolNone) && (protocol != bcmModuleProtocol1) &&
	     (protocol != bcmModuleProtocol2) ) {
            BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
        LOG_WARN(BSL_LS_BCM_STK,
                 (BSL_META_U(unit,
                             "Unexpected Fabric configuration(%d)\n"),
                  SOC_SBX_CFG(unit)->uFabricConfig));
    }

    node = BCM_STK_MOD_TO_NODE(mod);

    SOC_SBX_STATE(unit)->stack_state->protocol[node] = protocol;

    if (SOC_IS_SBX_FE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_module_protocol_set, (unit, node, protocol)));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_module_protocol_get(int unit,
				bcm_module_t mod,
				bcm_module_protocol_t *protocol)
{
    int node;
    int rv;


    if ((!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) ||
	(SOC_SBX_STATE(unit)->stack_state != &stack_state[unit])) {
        return BCM_E_UNIT;
    }

    BCM_SBX_LOCK(unit);
    if (!BCM_STK_MOD_IS_NODE(mod)) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    node = BCM_STK_MOD_TO_NODE(mod);
    *protocol = SOC_SBX_STATE(unit)->stack_state->protocol[node];

    if (SOC_IS_SBX_FE(unit)) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_module_protocol_get, (unit, node, protocol)));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_stk_fabric_map_set(int unit, 
			   bcm_gport_t switch_port, 
			   bcm_gport_t fabric_port)
{
    int fe_mod, qe_mod = -1, nodeid, subport = -1, eg_n = 0;
    bcm_port_t fe_port = -1, qe_port = -1, rq_port = FALSE;
    uint32 entry;
    uint32 found;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_gport_t *match_entry = NULL;
    bcm_sbx_stack_portmap_block_t *match_map = NULL;
    bcm_sbx_stack_portmap_block_t * map;
    bcm_sbx_stack_portmap_block_t * prev_map = NULL;
    int rv = BCM_E_NONE;
    int PFC_mapping = FALSE;

    BCM_SBX_LOCK(unit);


    if (switch_port == BCM_GPORT_INVALID) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    } else if ((switch_port >= 0) && (switch_port <= 15)) {
	/* for Sirius PFC source port ID generation */
	PFC_mapping = TRUE;
    }
    
    if (BCM_GPORT_IS_MODPORT(fabric_port)) { 
	qe_mod = BCM_GPORT_MODPORT_MODID_GET(fabric_port);
	qe_port = BCM_GPORT_MODPORT_PORT_GET(fabric_port);       
    } else if (BCM_GPORT_IS_CHILD(fabric_port)) {
	qe_mod = BCM_GPORT_CHILD_MODID_GET(fabric_port);
	qe_port = BCM_GPORT_CHILD_PORT_GET(fabric_port);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(fabric_port)) {
	qe_mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(fabric_port);
	qe_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(fabric_port);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(fabric_port)) {
	rv = bcm_sbx_cosq_egress_group_info_get(unit, fabric_port, &subport, &eg_n, NULL);
	if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_STK,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Egress Group 0x%x does not contain "
                                  "fabric_port, unit %d\n"),
                       FUNCTION_NAME(), fabric_port, unit));
            BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
	qe_mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(fabric_port);
	qe_port = subport;
    } else {
	if (SOC_IS_SBX_CALADAN3(unit)) {
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
    }

    if (PFC_mapping) {
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_fabric_map_set, (unit, switch_port, fabric_port)));
	if (rv != SOC_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (SOC_IS_SBX_CALADAN3(unit)) {
	rv = BCM_E_NONE;

        if (!BCM_GPORT_IS_MODPORT(switch_port)) {
            /* !BCM_GPORT_IS_MODPORT(fabric_port)) */
            LOG_ERROR(BSL_LS_BCM_STK,
                      (BSL_META_U(unit,
                                  "unit %d: switch_port %d or fabric_port %d is not modport "
                                  "gport\n"),
                       unit, switch_port, fabric_port));
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        fe_mod = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        fe_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);
        if (fe_mod >= SBX_MAX_MODIDS || fe_port >= SBX_MAX_PORTS) {
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
       
        SOC_SBX_CONTROL(unit)->modport[fe_mod][fe_port] =
            (qe_mod << 16) | qe_port;

        /* set the reverse mapping fabport -> feport */
        nodeid = BCM_STK_MOD_TO_NODE(qe_mod);

        if(nodeid < 0 || nodeid >= SBX_MAX_NODES) {
            BCM_SBX_UNLOCK(unit);
            return BCM_E_INIT;
        }

        SOC_SBX_CONTROL(unit)->fabnodeport2feport[nodeid][qe_port] =
            (fe_mod << 16) | fe_port;

    } else {
	if ((fabric_port != BCM_GPORT_INVALID) && (qe_port != -1) &&
	    SOC_SBX_STATE(unit)->port_state->subport_info) {
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[qe_port]);

            /* coverity[negative_shift : FALSE] */
            if ((sp_info != NULL) && (sp_info->parent_gport != 0xffffffff) &&
                IS_REQ_PORT(unit, BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport))) {
                rq_port = TRUE;
            }
        }

	/* Find the entry */
	for(found=0, map=SOC_SBX_STATE(unit)->stack_state->gport_map; !found && map!=NULL;
	    prev_map=map, map=map->next) {
	    if (map->count > 0) {
		for (entry=0; entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE; entry+=2) {
		    if (map->portmap[entry] == switch_port) {
			/* save the matched entry */
			found = 1;
			match_entry = &(map->portmap[entry]);
			match_map = map;
			break;
		    } else if ((map->portmap[entry] == BCM_GPORT_INVALID) && (match_entry == NULL)) {
			/* save first unused entry */
			match_entry = &(map->portmap[entry]);
			match_map = map;
		    }
		}
	    }
	}

	if (fabric_port == BCM_GPORT_INVALID) {
	    /* clear the map */
	    if (found == 1) {
		*match_entry = BCM_GPORT_INVALID;
		*(match_entry+1) = BCM_GPORT_INVALID;
		match_map->count--;
	    }
	} else {
	    /* set the map */
	    if (found == 1) {
		if (rq_port == FALSE) {
		    /* replacing the existing entry except for requeue path */
		    *(match_entry+1) = fabric_port;
		}
	    } else {
		/* creating a new entry */
		if (match_entry != NULL) {
		    /* found a unused entry, fill it */
		    *match_entry = switch_port;
		    *(match_entry+1) = fabric_port;
		    match_map->count++;
		} else {
		    /* only possible when all existing maps are full */
		    map = sal_alloc(sizeof(bcm_sbx_stack_portmap_block_t), "fabric map");
		    if (map == NULL) {
			BCM_SBX_UNLOCK(unit);
			return SOC_E_MEMORY;
		    }

		    sal_memset(map, BCM_GPORT_INVALID, sizeof(bcm_sbx_stack_portmap_block_t));
		    map->next = NULL;
		    map->count = 0;
		    if ( SOC_SBX_STATE(unit)->stack_state->gport_map == NULL ) {
			/* init linklist head */
			SOC_SBX_STATE(unit)->stack_state->gport_map = map;
		    } else {
			/* chain the single linklist */
			prev_map->next = map;
		    }
		    map->portmap[0] = switch_port;
		    map->portmap[1] = fabric_port;
		    map->count++;
		}
	    }
	}
    }

    if (SOC_IS_SBX_SIRIUS(unit)) {
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_fabric_map_set, (unit, switch_port, fabric_port)));
	if (rv != SOC_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_stk_fabric_map_get(int unit, 
			   bcm_gport_t switch_port, 
			   bcm_gport_t *fabric_port)
{
    int fe_mod, qe_mod;
    bcm_port_t fe_port, qe_port;
    uint32 entry;
    bcm_sbx_stack_portmap_block_t * map;
    int rv = BCM_E_NONE;
    int PFC_mapping = FALSE;
	
    BCM_SBX_LOCK(unit);

    if (switch_port == BCM_GPORT_INVALID || fabric_port == NULL) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    } else if ((switch_port >= 0) && (switch_port <= 15)) {
	/* for Sirius PFC source port ID generation */
	PFC_mapping = TRUE;
    }

    if (PFC_mapping) {
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_stk_fabric_map_get, (unit, switch_port, fabric_port)));
	if (rv != SOC_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
    }

    if (SOC_IS_SBX_CALADAN3(unit)) {
        if (!BCM_GPORT_IS_MODPORT(switch_port)) {
            LOG_ERROR(BSL_LS_BCM_STK,
                      (BSL_META_U(unit,
                                  "unit %d: switch_port %d is not a modport "
                                   "gport\n"), unit, switch_port));
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        fe_mod = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        fe_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);

        qe_mod = ((SOC_SBX_CONTROL(unit)->modport[fe_mod][fe_port] >> 16) 
                  & 0xffff);
        qe_port = (SOC_SBX_CONTROL(unit)->modport[fe_mod][fe_port]
                   & 0xffff);

        BCM_GPORT_MODPORT_SET(*fabric_port, qe_mod, qe_port);
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    } else { 

	/* Find the entry */
	for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
	    if (map->count > 0) {
		for (entry=0; entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE; entry+=2) {
		    if (map->portmap[entry] == switch_port) {
			/* return the matched entry */
			*fabric_port = map->portmap[entry+1];
			BCM_SBX_UNLOCK(unit);
			return BCM_E_NONE;
		    }
		}
	    }
	}
	
	*fabric_port = BCM_GPORT_INVALID;
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NOT_FOUND;
    }
}

/* Internal use only */
int
bcm_sbx_stk_fabric_map_get_switch_port(int unit, 
				       bcm_gport_t fabric_port, 
				       bcm_gport_t *switch_port)
{
    uint32 entry;
    bcm_sbx_stack_portmap_block_t * map;
	
    BCM_SBX_LOCK(unit);

    if (fabric_port == BCM_GPORT_INVALID || switch_port == NULL) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }
    if (SOC_IS_SBX_CALADAN3(unit)) {
        int  qe_mod, nodeid;
        bcm_port_t  qe_port;
        uint32 femodport=0;
    
        qe_mod = BCM_GPORT_MODPORT_MODID_GET(fabric_port);
        qe_port = BCM_GPORT_MODPORT_PORT_GET(fabric_port);       
        nodeid = BCM_STK_MOD_TO_NODE(qe_mod);

        if(nodeid < 0 || nodeid >= SBX_MAX_NODES) {
	    BCM_SBX_UNLOCK(unit);
            return BCM_E_INIT;
        }
        
        femodport = SOC_SBX_CONTROL(unit)->fabnodeport2feport[nodeid][qe_port];
        
        BCM_GPORT_MODPORT_SET(*switch_port, 
                              femodport >> 16, 
                              femodport & ((1 << 16) -1));
        
	BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    } else {

	/* Find the entry */
	for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
	    if (map->count > 0) {
		for (entry=0; entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE; entry+=2) {
#if 000
		  LOG_CLI((BSL_META_U(unit,
                                      "entry[%d]=0x%08x\n"), entry+1, map->portmap[entry+1]));
#endif
		    if (map->portmap[entry+1] == fabric_port) {
			/* return the matched entry */
			*switch_port = map->portmap[entry];
			BCM_SBX_UNLOCK(unit);
			return BCM_E_NONE;
		    }
		}
	    }
	}

	*switch_port = BCM_GPORT_INVALID;
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NOT_FOUND;
    }
}

int
bcm_sbx_stk_steering_unicast_set(int unit,
                                 int steer_id,
                                 bcm_module_t destmod_lo,
                                 bcm_module_t destmod_hi,
                                 int num_queue_groups,
                                 bcm_gport_t *queue_groups)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_unicast_set,
                                  (unit,
                                  steer_id,
                                  destmod_lo,
                                  destmod_hi,
                                  num_queue_groups,
                                  queue_groups));
    BCM_SBX_UNLOCK(unit);
    return result;
}

int
bcm_sbx_stk_steering_multicast_set(int unit,
                                      int steer_id,
                                      bcm_multicast_t mgid_lo,
                                      bcm_multicast_t mgid_hi,
                                      int num_queue_groups,
                                      bcm_gport_t *queue_groups)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_multicast_set,
                                  (unit,
                                  steer_id,
                                  mgid_lo,
                                  mgid_hi,
                                  num_queue_groups,
                                  queue_groups));
    BCM_SBX_UNLOCK(unit);
    return result;
}

int
bcm_sbx_stk_steering_clear(int unit,
                           int steer_id)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_clear,
                                  (unit,
                                  steer_id));
    BCM_SBX_UNLOCK(unit);
    return result;
}

int
bcm_sbx_stk_steering_clear_all(int unit)
{
    int result;

    BCM_SBX_LOCK(unit);
    result = MBCM_SBX_DRIVER_CALL(unit,
                                  mbcm_stk_steering_clear_all,
                                  (unit));
    BCM_SBX_UNLOCK(unit);
    return result;
}

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0

/* This function initializes the WB support for stack module. 
 *      1. During cold-boot: allocates a stable cache
 *      2. During warm-boot: Recovers the stack state from stable cache
 * 
 * bcm_sbx_stack_state_t Compressed format 
 * 
 * NOTE: For sirius, we check both BCM_SIRIUS_SUPPORT and if the unit is a sirius.
 *  The latter check is to save size for non-sirius stack devices.
 */
int
bcm_sbx_wb_stack_state_init(int unit)
{
    int                     rv = BCM_E_NONE;
    uint32                  scache_len;
    int                     stable_size;
    uint8                   *scache_ptr = NULL, *ptr, *end_ptr;
    bcm_sbx_stack_state_t   *p_ss;
    bcm_sbx_stack_portmap_block_t  *map;
    soc_scache_handle_t     scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;
    uint32                  index, cnt, idx;

    p_ss = SOC_SBX_STATE(unit)->stack_state;
    if (p_ss == NULL) {
        LOG_ERROR(BSL_LS_BCM_STK,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid stack state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }
    
    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    /* If device is during warm-boot, recover the state from scache */
    scache_len = 0;
        SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_STACK, 0);

    if (SOC_WARM_BOOT(unit)) {
        rv = soc_versioned_scache_ptr_get(unit, scache_handle,
					      FALSE, &scache_len, &scache_ptr,
					      default_ver, &recovered_ver);
	if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_STK,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                       FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
            return rv;
	}
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len; /* used for overrun checks*/

    /* now de-compress to stack state */
    for (index=0; index < BCM_STK_MAX_MODULES; index++) {
	__WB_DECOMPRESS_SCALAR(bcm_module_protocol_t, p_ss->protocol[index]);
	__WB_DECOMPRESS_SCALAR(int, p_ss->is_module_enabled[index]);
    }

    index=0;
    map = p_ss->gport_map;
    __WB_DECOMPRESS_SCALAR(bcm_sbx_stack_portmap_block_t  *, map);
    if (SOC_WARM_BOOT(unit)) {
        /* Need to re-allocate gport map */
        if (map != NULL) {
            p_ss->gport_map = sal_alloc(sizeof(bcm_sbx_stack_portmap_block_t), "fabric map");
            if (p_ss->gport_map == NULL) {
                return SOC_E_MEMORY;
            }
            map = p_ss->gport_map;
            sal_memset(map, BCM_GPORT_INVALID, sizeof(bcm_sbx_stack_portmap_block_t));
            map->next = NULL;
            map->count = 0;
        } else {
            index = BCM_STK_MAX_MODULES;
        }
    }
    while ((index++ < BCM_STK_MAX_MODULES) ||
           (SOC_WARM_BOOT(unit) && (map != NULL))) {
           /* coverity[var_deref_op] */
        __WB_DECOMPRESS_SCALAR(bcm_sbx_stack_portmap_block_t *, map->next);
        __WB_DECOMPRESS_SCALAR(uint32, map->count);
        if (SOC_WARM_BOOT(unit)) {
            cnt = map->count;
        } else {
            cnt = 2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE;
        }
        for (idx = 0; idx < (cnt*2); idx++) {
            __WB_DECOMPRESS_SCALAR(bcm_gport_t, map->portmap[idx]);
        }
        if (SOC_WARM_BOOT(unit)) {            
            if (map->next != NULL) {
                map->next = sal_alloc(sizeof(bcm_sbx_stack_portmap_block_t), "fabric map");
                if (map->next == NULL) {
                    return SOC_E_MEMORY;
                }
                map = map->next;
            } else {
                map = map->next;
                index = BCM_STK_MAX_MODULES;
            }
        }
    }

    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    if (!SOC_WARM_BOOT(unit)) {
	rv = soc_versioned_scache_ptr_get(unit, scache_handle,
					      TRUE, &scache_len, &scache_ptr,
					      default_ver, &recovered_ver);
	if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_STK,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) allocating WB scache handle on unit:%d \n"),
                       FUNCTION_NAME(), soc_errmsg(rv), unit));
            return rv;
        }
    }

    return rv;
}

/* This function compresses the info in bcm_sbx_stack_state_t and stores it
 * to stable memory.
 * Input param: sync --> indicates whether to sync scache to Persistent memory
 */
int
bcm_sbx_wb_stack_state_sync(int unit, int sync)
{
    uint8                   *scache_ptr = NULL, *ptr, *end_ptr;
    uint32                  scache_len;
    int                     stable_size;
    int                     rv;
    bcm_sbx_stack_state_t   *p_ss;
    soc_scache_handle_t     scache_handle;
    bcm_sbx_stack_portmap_block_t  *map;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;
    uint32                  index;

    if (SOC_WARM_BOOT(unit)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Cannot write to SCACHE during WarmBoot\n")));
        return SOC_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    p_ss = SOC_SBX_STATE(unit)->stack_state;
    if (p_ss == NULL) {
        LOG_ERROR(BSL_LS_BCM_STK,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid fabric state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_STACK, 0);

    scache_len = 0;
    rv = soc_versioned_scache_ptr_get(unit, scache_handle,
					  FALSE, &scache_len, &scache_ptr,
					  default_ver, &recovered_ver);
    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_STK,
                  (BSL_META_U(unit,
                              "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                   FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
        return rv;
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr+scache_len;

    /* now de-compress to stack state */
    for (index=0; index < BCM_STK_MAX_MODULES; index++) {
	__WB_COMPRESS_SCALAR(bcm_module_protocol_t, p_ss->protocol[index]);
	__WB_COMPRESS_SCALAR(int, p_ss->is_module_enabled[index]);
    }

    map = p_ss->gport_map;
    __WB_COMPRESS_SCALAR(bcm_sbx_stack_portmap_block_t *, map);
    while (map != NULL) {
        __WB_COMPRESS_SCALAR(bcm_sbx_stack_portmap_block_t *, map->next);
        __WB_COMPRESS_SCALAR(uint32, map->count);
        for (index = 0; index < (map->count*2); index++) {
            __WB_COMPRESS_SCALAR(bcm_gport_t, map->portmap[index]);
        }
        map = map->next;
    }

    if (sync) {
        rv = soc_scache_commit(unit);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_STK,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) sync'ing scache to Persistent memory. \n"),
                       FUNCTION_NAME(), soc_errmsg(rv)));
            return rv;
        }
    }

    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


#if defined(BCM_WARM_BOOT_SUPPORT_SW_DUMP)
static void
_sbx_cosq_gport_get_node_port(int unit, bcm_gport_t gport, 
                              int *node, int *port) {


  int sysport = 0;
  int modid = 0;
  bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
  int eg_n = 0;

  if (BCM_GPORT_IS_MODPORT(gport)) {
    modid = BCM_GPORT_MODPORT_MODID_GET(gport);
    *port  = BCM_GPORT_MODPORT_PORT_GET(gport);
  } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
    sysport = BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(gport);
    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
      modid = -1; *port = -1;
    } else {
      modid = SOC_SBX_STATE(unit)->sysport_state[sysport].node;
      *port  = SOC_SBX_STATE(unit)->sysport_state[sysport].port;
    }
  } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
    sysport = BCM_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_GET(gport);
    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
      modid = -1; *port = -1;
    } else {
      modid = SOC_SBX_STATE(unit)->sysport_state[sysport].node;
      *port  = SOC_SBX_STATE(unit)->sysport_state[sysport].port;
    }
  } else if (BCM_GPORT_IS_CHILD(gport)) {
    modid = BCM_GPORT_CHILD_MODID_GET(gport);
    *port  = BCM_GPORT_CHILD_PORT_GET(gport);
  } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
    modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
    *port  = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
  } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
    modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
    *port  = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
  } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
    modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
    p_eg  = SOC_SBX_STATE(unit)->egress_group_state;
    eg_n  = BCM_GPORT_EGRESS_GROUP_GET(gport);
    if ((p_eg == NULL) || (eg_n < 0) || (eg_n >= SOC_SBX_CFG(unit)->num_egress_group)) {
      /* egress group is not valid */
      LOG_CLI((BSL_META_U(unit,
                          "Egress Group (0x%08x) is invalid\n"),gport));
      *port = -1;
    } else {
      *port  = (p_eg + eg_n)->child_port;
    }
  }  else {
    /* gport type is qid based */
    *port = 0;
  }

  if (modid >= BCM_MODULE_FABRIC_BASE) {
    *node = BCM_STK_MOD_TO_NODE(modid);  
  } else {
    *node = modid;
  }

}

static char*
_sbx_get_gport_type_string(bcm_gport_t gport) {

  if (BCM_GPORT_IS_MODPORT(gport)) {
      return "ModPort";
  }else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)){
      return "Unicast Queue Group";
  }else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
      return "Multicast Queue Group";
  }else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)){
      return "Subscriber Unicast Queue Group";
  }else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)){
      return "Subscriber Multicast Queue Group";
  }else if (BCM_GPORT_IS_MCAST(gport)) {
      return "Multicast";
  }else if (BCM_GPORT_IS_CHILD(gport)) {
      return "Child";
  }else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
      return "Egress child";
  }else if (BCM_GPORT_IS_SCHEDULER(gport)) {
      return "Scheduler";
  }else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
      return "Egress ModPort";
  }else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
      return "Egress Group";
  }
  /* gport is invalid */
  return "Invalid";
}

void 
bcm_sbx_wb_stack_sw_dump(int unit)
{
    bcm_sbx_stack_state_t  *p_ss;
    bcm_sbx_stack_portmap_block_t * map;
    int                      index;
    uint32 entry;
    int switch_node = 0;
    int switch_port = 0;
    int fabric_node = 0;
    int fabric_port = 0;

    p_ss = SOC_SBX_STATE(unit)->stack_state;
    if (p_ss == NULL) {
        LOG_ERROR(BSL_LS_BCM_STK,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid stack state (NULL)\n"), 
                   FUNCTION_NAME()));
        return;
    }

    for (index=0; index < BCM_STK_MAX_MODULES; index++) {
	LOG_CLI((BSL_META_U(unit,
                            "protocol = %d\t"),p_ss->protocol[index]));
	LOG_CLI((BSL_META_U(unit,
                            "is_module_enable = %d\n"),p_ss->is_module_enabled[index]));
    }

    LOG_CLI((BSL_META_U(unit,
                        " Switch Gport     Type          Fabric Gport     Type  \t\t\t        Switch (Node|Port)  Fabric (Node|Port) \n")));
    LOG_CLI((BSL_META_U(unit,
                        " --------------------------------------------------------------------------------------------------------------------\n")));
    /* Show mappings of switch port, fabric port */
    for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
      if (map->count > 0) {
	for(entry=0;entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE;entry+=2) {
	  if (map->portmap[entry] == BCM_GPORT_INVALID) continue; 
	  _sbx_cosq_gport_get_node_port(unit,map->portmap[entry],&switch_node,&switch_port);
	  _sbx_cosq_gport_get_node_port(unit,map->portmap[entry+1],&fabric_node,&fabric_port);
	  LOG_CLI((BSL_META_U(unit,
                              "  0x%08x \t %-s \t 0x%08x \t %-32s  (%05d|%04d)\t      (%05d|%04d)\n"),
                   map->portmap[entry],_sbx_get_gport_type_string(map->portmap[entry]),
                   map->portmap[entry+1],_sbx_get_gport_type_string(map->portmap[entry+1]),
                   switch_node,switch_port,fabric_node,fabric_port));
	}
      }
    }


    LOG_CLI((BSL_META_U(unit,
                        "\n")));

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
