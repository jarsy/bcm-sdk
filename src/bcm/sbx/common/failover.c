/*
 * $Id: failover.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * FE2000 failover API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>

#include <bcm/error.h>
#include <bcm/failover.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/state.h>
#include <shared/gport.h>

#include <bcm/types.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx_dispatch.h>

STATIC bcm_sbx_failover_object_t *failover_state[SOC_MAX_NUM_DEVICES];


int
bcm_sbx_failover_init(int unit)
{
    int rv = BCM_E_NONE;
    int failover;
    int num_sysports;

    BCM_SBX_LOCK(unit);

    SOC_SBX_STATE(unit)->failover_state = NULL;

    if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) ||
	 (SOC_IS_SBX_FE(unit)) ) {
	/* FE or DMODE system doesn't support failover */
	BCM_SBX_UNLOCK(unit);
	return rv;
    }

    num_sysports = SOC_SBX_CFG(unit)->num_sysports;

    if (num_sysports == 0) {
	/* system without sysport support */
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NONE;
    }

    /* Allocate failover state array, assuming one profile for each
     * sysport in the system ?? Is this enough ??
     */
    failover_state[unit] = sal_alloc(sizeof(bcm_sbx_failover_object_t) * num_sysports,
				     "failover state memory");

    if (failover_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_MEMORY);
    }

    SOC_SBX_STATE(unit)->failover_state = failover_state[unit];

    for (failover = 0; failover < num_sysports; failover++) {
	failover_state[unit][failover].state = 0;
	failover_state[unit][failover].sysport = -1;
	failover_state[unit][failover].active_gport = BCM_GPORT_INVALID;
	failover_state[unit][failover].protect_gport = BCM_GPORT_INVALID;	
	failover_state[unit][failover].active_nodeport = -1;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_failover_deinit(unit)
{
    if (failover_state[unit]) {
        sal_free(failover_state[unit]);
        failover_state[unit] = NULL;
    }
    if (SOC_CONTROL(unit) && SOC_SBX_CONTROL(unit) && SOC_SBX_STATE(unit)) {
        SOC_SBX_STATE(unit)->failover_state = NULL;
    }
    return BCM_E_NONE;
}

int 
bcm_sbx_failover_create(int unit, uint32 flags, bcm_failover_t *failover_id)
{

    int rv = BCM_E_NONE;
    int failover, num_failovers;

    BCM_SBX_LOCK(unit);

    num_failovers = SOC_SBX_CFG(unit)->num_sysports;

    if ( (failover_id == NULL) || ( (flags & (BCM_FAILOVER_WITH_ID | BCM_FAILOVER_REPLACE)) &&
				    (( *failover_id < 0 ) || (*failover_id >= num_failovers)) ) ) {
	/* if pointer is NULL or passed in ID is not in range */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_STATE(unit)->failover_state == NULL) {
	/* if failover_state pointer is NULL, it's on a unit which doesn't support failover command */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!flags) {
	/* need to alloc failover profile, find the first one not in use */
	for (failover = 0; failover < num_failovers; failover++) {
	    if ( !(SOC_SBX_STATE(unit)->failover_state[failover].state & BCM_SBX_FAILOVER_RESERVED)) {
		*failover_id = failover;
		SOC_SBX_STATE(unit)->failover_state[*failover_id].state |= BCM_SBX_FAILOVER_RESERVED;
		break;
	    }
	}
    } else if ( (flags & BCM_FAILOVER_WITH_ID) && (!(flags & BCM_FAILOVER_REPLACE)) ){
	/* failover id passed in for failover creation, mark as in use */
	SOC_SBX_STATE(unit)->failover_state[*failover_id].state |= BCM_SBX_FAILOVER_RESERVED;
    } else {
	/* failover id passed in for failover update */
	if ( !(SOC_SBX_STATE(unit)->failover_state[*failover_id].state & BCM_SBX_FAILOVER_RESERVED) ) {
	    /* if not reserved, can not update */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	} else {
	    SOC_SBX_STATE(unit)->failover_state[*failover_id].state |= BCM_SBX_FAILOVER_UPDATED;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int 
bcm_sbx_failover_destroy(int unit, bcm_failover_t failover_id)
{
    int rv = BCM_E_NONE;
    int num_failovers;
    int sysport, node, protect_node, active_node, fabric_port = 0xff;
    bcm_sbx_cosq_sysport_group_state_t *p_spg;
    bcm_sbx_cosq_destport_state_t *p_dps = NULL;
    int aps;

    BCM_SBX_LOCK(unit);

    num_failovers = SOC_SBX_CFG(unit)->num_sysports;

    if ( ( failover_id < 0 ) || (failover_id >= num_failovers) ) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_STATE(unit)->failover_state == NULL) {
	/* if failover_state pointer is NULL, it's on a unit which doesn't support failover command */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if ( !(SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_RESERVED)) {
	/* if it's not reserved, can not destroy */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    aps = (SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_APS) ? TRUE : FALSE;

    if ( aps && (SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_INUSE) ) {
	/* when the failover is in use, remove the dummy sysport, no need to update 
	 * port remap table on QE2000. If on different node, the used remap table entry is 
	 * not harmful, if on same node, the remap table entry is being used by the sysport
	 */
	sysport = SOC_SBX_STATE(unit)->failover_state[failover_id].sysport;
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_failover_destroy, (unit, BCM_INT_SBX_SYSPORT_DUMMY(sysport))));	

	if (rv == BCM_E_NONE) {
	    /* manage sysport group state */
	    protect_node = BCM_GPORT_MODPORT_MODID_GET(SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport);
	    protect_node = BCM_STK_MOD_TO_NODE(protect_node);
	    active_node = (SOC_SBX_STATE(unit)->failover_state[failover_id].active_nodeport & 0xFFFF0000) >> 16;

	    if (active_node != protect_node) {
		/* only need to remove the node not used from node_cnt if protect node are not same as active node */
		if (SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_ENABLE ) {
		    node = protect_node;
		} else {
		    node = active_node;
		}
		p_spg = &(SOC_SBX_STATE(unit)->sysport_group_state[sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP]);
		p_dps = (bcm_sbx_cosq_destport_state_t*)SOC_SBX_STATE(unit)->destport_state;
		p_spg->node_cnt[node]--;
		if (p_spg->node_port[node] != 0xff) {
		  fabric_port = p_spg->node_port[node];
		  if (p_dps) {
		    p_dps = &p_dps[fabric_port];
		    p_dps->ref_cnt--;
		  }
		}
		if (p_spg->node_cnt[node] == 0) {
		  p_spg->node_port[node] = 0xff;
		}
	    }
	}
    }

    if (rv == BCM_E_NONE) {
	SOC_SBX_STATE(unit)->failover_state[failover_id].state = 0;
	SOC_SBX_STATE(unit)->failover_state[failover_id].sysport = -1;
	SOC_SBX_STATE(unit)->failover_state[failover_id].active_gport = BCM_GPORT_INVALID;
	SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport = BCM_GPORT_INVALID;	
	SOC_SBX_STATE(unit)->failover_state[failover_id].active_nodeport = -1;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Set a failover object to enable or disable
 */
int 
bcm_sbx_failover_set(int unit, bcm_failover_t failover_id, int enable)
{
    int rv = BCM_E_NONE;
    int sysport, node, port, old_node, old_port;
    int num_failovers;
    int aps, subport, intf_gport;

    BCM_SBX_LOCK(unit);

    num_failovers = SOC_SBX_CFG(unit)->num_sysports;

    if ( ( failover_id < 0 ) || (failover_id >= num_failovers) ) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_STATE(unit)->failover_state == NULL) {
	/* if failover_state pointer is NULL, it's on a unit which doesn't support failover command */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if ( !(SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_INUSE) ||
	 !(SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_RESERVED) ) {
	/* if it's not reserved or not in use, can not enable or disable */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if ( ((enable && (SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_ENABLE)) ||
	  (!enable && !(SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_ENABLE))) &&
	 !(SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_UPDATED) ){
	/* if already enabled or disabled, and not force updating, do nothing and return success */
	BCM_SBX_UNLOCK(unit);
        return rv;
    }

    aps = (SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_APS) ? TRUE : FALSE;

    if (aps == TRUE) {
	if ( (SOC_SBX_STATE(unit)->failover_state[failover_id].active_nodeport == -1) || 
	     (SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport == BCM_GPORT_INVALID) || 
	     (SOC_SBX_STATE(unit)->failover_state[failover_id].sysport == -1) ) {
	    /* the active/protect gport has to be set up before we can enable or disable the failover */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;	
	}

	sysport = SOC_SBX_STATE(unit)->failover_state[failover_id].sysport;
	old_node = SOC_SBX_STATE(unit)->sysport_state[sysport].node;
	old_port = SOC_SBX_STATE(unit)->sysport_state[sysport].port;

	if (enable) {
	    /* switch to protect node/port */
	    node = BCM_GPORT_MODPORT_MODID_GET(SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport);
	    if (node < BCM_MODULE_FABRIC_BASE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: gport modid invalid value (%d) is less than minimum (%d)\n"),
		           node, BCM_MODULE_FABRIC_BASE));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	    node = BCM_STK_MOD_TO_NODE(node);
	    port = BCM_GPORT_MODPORT_PORT_GET(SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport);
	} else {
	    /* switch back to active node/port */
	    node = (SOC_SBX_STATE(unit)->failover_state[failover_id].active_nodeport & 0xFFFF0000) >> 16;
	    port = (SOC_SBX_STATE(unit)->failover_state[failover_id].active_nodeport & 0xFFFF);
	}
	
	/* switch to new node/port */
	rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_failover_enable, (unit, sysport, node, port, old_node, old_port)));
	
	if (rv == BCM_E_NONE) {
	    /* update sysport states */
	    SOC_SBX_STATE(unit)->sysport_state[sysport].node = node;
	    SOC_SBX_STATE(unit)->sysport_state[sysport].port = port;
	}
    } else {
	if ( (SOC_SBX_STATE(unit)->failover_state[failover_id].active_gport == BCM_GPORT_INVALID) || 
	     (SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport == BCM_GPORT_INVALID) ) {
	    /* protect gport and active gport has to be set up before we can enable or disable the failover */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;	
	}

	if (enable) {
	    /* switch to protect gport */
	    intf_gport = SOC_SBX_STATE(unit)->failover_state[failover_id].protect_gport;
	} else {
	    /* switch to active gport */
	    intf_gport = SOC_SBX_STATE(unit)->failover_state[failover_id].active_gport;
	}

	/* go through all fabric ports associated with this failover ID and switch between active and protect gport */
	for (subport = 0; subport <  SB_FAB_DEVICE_MAX_FABRIC_PORTS; subport++) {
	    /* skip subports not in the mask */
	    if (SHR_BITGET(SOC_SBX_STATE(unit)->failover_state[failover_id].subportmask, subport) == 0) {
		continue;
	    }

	    /* switch the subport */
	    rv = (MBCM_SBX_DRIVER_CALL(unit, mbcm_failover_enable, (unit, -1, -1, subport, -1, intf_gport)));
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	}
    }

    if (rv == BCM_E_NONE) {
	/* update failover state */
	if (enable) {
	    SOC_SBX_STATE(unit)->failover_state[failover_id].state |= BCM_SBX_FAILOVER_ENABLE;
	} else {
	    SOC_SBX_STATE(unit)->failover_state[failover_id].state &= (~BCM_SBX_FAILOVER_ENABLE);
	}
	SOC_SBX_STATE(unit)->failover_state[failover_id].state &= (~BCM_SBX_FAILOVER_UPDATED);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


/* Get the enable status of a failover object. */
int 
bcm_sbx_failover_get(int unit, bcm_failover_t failover_id, int *enable)
{
    int rv = BCM_E_NONE;
    int num_failovers;

    BCM_SBX_LOCK(unit);

    num_failovers = SOC_SBX_CFG(unit)->num_sysports;

    if ( (enable == NULL) || ( failover_id < 0 ) || (failover_id >= num_failovers) ) {
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (SOC_SBX_STATE(unit)->failover_state == NULL) {
	/* if failover_state pointer is NULL, it's on a unit which doesn't support failover command */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if ( !(SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_RESERVED) ) {
	/* if it's not reserved and in use, can not enable */
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *enable = (SOC_SBX_STATE(unit)->failover_state[failover_id].state & BCM_SBX_FAILOVER_ENABLE) ? TRUE : FALSE;

    BCM_SBX_UNLOCK(unit);
    return rv;
}
