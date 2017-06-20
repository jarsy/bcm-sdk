/*
 * $Id: failover.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 failover API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/stack.h>


int
bcm_qe2000_failover_enable(int unit,
			   int sysport,
			   int node, 
			   int port,
			   int old_node, 
			   int old_port)
{
    int rv = BCM_E_NONE;
    int ef;

    /* port remap when the protect port are on the same node */
    if ( (node == old_node) && (SOC_SBX_CONTROL(unit)->node_id == node) ) {
	ef = 0;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, port);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to remap sysport\n")));
	    return rv;
	}

	ef = 1;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, port);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to remap sysport\n")));
	    return rv;
	}
    }

    return rv;
}

int
bcm_qe2000_failover_set(int unit,
			int sysport,
			int protect_node, 
			int protect_port,
			int active_node, 
			int active_port)
{
    int rv = BCM_E_NONE;
    int ef;

    /* port remap when the protect port are on the different node */
    if ( (protect_node != active_node) && (SOC_SBX_CONTROL(unit)->node_id == protect_node) ) {
	ef = 0;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, protect_port);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to remap sysport\n")));
	    return rv;
	}

	ef = 1;
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
        }
	rv = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, protect_port);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to remap sysport\n")));
	    return rv;
	}
    }

    return rv;
}



