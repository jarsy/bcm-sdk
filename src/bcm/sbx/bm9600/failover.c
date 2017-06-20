/*
 * $Id: failover.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 failover API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/bm9600.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/stack.h>


int
bcm_bm9600_failover_enable(int unit,
			   int sysport,
			   int node, 
			   int port,
			   int old_node, 
			   int old_port)
{
    int rv = BCM_E_NONE;
    int dummy_sysport;
    int ps, os, dummy_ps, dummy_os;
    int indp_fc_sysport, dummy_indp_fc_sysport;
    int indp_fc_ps, indp_fc_os, dummy_indp_fc_ps, dummy_indp_fc_os;

    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
	return BCM_E_PARAM;
    }

    dummy_sysport = BCM_INT_SBX_SYSPORT_DUMMY(sysport);
    indp_fc_sysport = BCM_INT_SBX_INVALID_SYSPORT;
    dummy_indp_fc_sysport = BCM_INT_SBX_INVALID_SYSPORT;

    /* by now, both the sysport and dummy sysport should be in portset,
     * search for their ps/os
     */
    rv = bcm_bm9600_get_portset_from_sysport(unit, sysport, &ps, &os);
    if (rv != BCM_E_NONE) {
	return BCM_E_INTERNAL;
    }

    rv = bcm_bm9600_get_portset_from_sysport(unit, dummy_sysport, &dummy_ps, &dummy_os);
    if (rv != BCM_E_NONE) {
	return BCM_E_INTERNAL;
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        indp_fc_sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
        dummy_indp_fc_sysport = BCM_INT_SBX_SYSPORT_DUMMY(indp_fc_sysport);

        rv = bcm_bm9600_get_portset_from_sysport(unit, indp_fc_sysport, &indp_fc_ps, &indp_fc_os);
        if (rv != BCM_E_NONE) {
            return BCM_E_INTERNAL;
        }

        rv = bcm_bm9600_get_portset_from_sysport(unit, dummy_indp_fc_sysport, &dummy_indp_fc_ps, &dummy_indp_fc_os);
        if (rv != BCM_E_NONE) {
            return BCM_E_INTERNAL;
        }
    }

    /* Swap to the portset/offset used by dummy sysport */
    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, dummy_ps, dummy_os, 0);
    if (rv != BCM_E_NONE) {
	return rv;
    }

    rv = bcm_bm9600_map_sysport_to_portset(unit, dummy_sysport, ps, os, 0);

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv = bcm_bm9600_map_sysport_to_portset(unit, indp_fc_sysport, dummy_indp_fc_ps, dummy_indp_fc_os, 0);
        if (rv != BCM_E_NONE) {
            return rv;
        }

        rv = bcm_bm9600_map_sysport_to_portset(unit, dummy_indp_fc_sysport, indp_fc_ps, indp_fc_os, 0);
        if (rv != BCM_E_NONE) {
            return rv;
        }
    }

    return rv;
}

int
bcm_bm9600_failover_set(int unit,
			int sysport,
			int protect_node, 
			int protect_port,
			int active_node, 
			int active_port)
{
    int rv = BCM_E_NONE;
    int mapped_protect_port;
    sbBool_t is_prot_resource_alloc = FALSE, is_independent_fc_prot_resource_alloc = FALSE;

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	return BCM_E_INTERNAL;
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
    }
		
    /* Remove the dummy sysport from portset if exist */
    rv = bcm_bm9600_unmap_sysport(unit, sysport);
    if (rv != BCM_E_NONE) {
	return rv;
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
        if (rv != BCM_E_NONE) {
            return rv;
        }
    }

    /* Insert the dummy sysport for node/port */
    mapped_protect_port = (soc_feature(unit, soc_feature_egr_independent_fc)) ?
                                  BCM_INT_SBX_PORT_TO_EF_PORT(protect_port) : protect_port;
    /* Insert the dummy sysport for node/port */
    rv = bcm_bm9600_map_sysport_to_nodeport(unit, sysport, protect_node, mapped_protect_port);
    if (rv != BCM_E_NONE) {
        goto err;
    }
    is_prot_resource_alloc = TRUE;

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv = bcm_bm9600_map_sysport_to_nodeport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport), protect_node,
                                                      BCM_INT_SBX_PORT_TO_NEF_PORT(protect_port));
        if (rv != BCM_E_NONE) {
            goto err;
        }
        is_independent_fc_prot_resource_alloc = TRUE;
    }

    return rv;

err:

    if (is_prot_resource_alloc == TRUE) {
        bcm_bm9600_unmap_sysport(unit, sysport);
    }
    if (is_independent_fc_prot_resource_alloc == TRUE) {
        /* coverity[dead_error_line] */
        bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
    }

    return rv;
}


int
bcm_bm9600_failover_destroy(int unit,
			    int sysport)

{
    int rv = BCM_E_NONE, rv1 = BCM_E_NONE;

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	return BCM_E_INTERNAL;
    }
		
    /* Remove the dummy sysport from portset */
    rv = bcm_bm9600_unmap_sysport(unit, sysport);

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        rv1 = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
        rv = (rv != BCM_E_NONE) ? rv : rv1;
    }

    return rv;
}
