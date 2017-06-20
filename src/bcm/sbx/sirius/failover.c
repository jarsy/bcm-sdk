/*
 * $Id: failover.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 failover API
 */

#include <shared/bsl.h>

#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/failover.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/stack.h>


int
bcm_sirius_failover_enable(int unit,
			   int sysport,
			   int node, 
			   int port,
			   int old_node, 
			   int old_port)
{
    int rv = BCM_E_UNAVAIL;
    int intf, offset, egroup, map_index;
    int level2_node, level1_node, level0_node;
    bcm_sbx_subport_info_t *sp_info = NULL;
    channel_map_table_entry_t channel_entry;

    /* Switch a group of fabric ports to another interface, only ES for now */
    if ((port < 0) || (port > SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fabric port id %d out of range, Unit(%d)\n"),
	           FUNCTION_NAME(), port, unit));	
	return BCM_E_PARAM;
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
    if (sp_info->valid == FALSE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fabric port with id %d not valid, Unit(%d)\n"),
	           FUNCTION_NAME(), port, unit));	
	return BCM_E_PARAM;   
    }

    if (!BCM_GPORT_IS_MODPORT(old_port)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, protect gport 0x%x not a MODPORT, Unit(%d)\n"),
	           FUNCTION_NAME(), old_port, unit));	
	return BCM_E_PARAM;	
    } else {
	rv = bcm_sbx_port_get_intf_portoffset(unit, old_port, &intf, &offset);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to find interface for protect gport 0x%x, Unit(%d)\n"),
	               FUNCTION_NAME(), old_port, unit));	
	    return rv;
	}
	if ((intf < SB_FAB_DEVICE_SIRIUS_HG0_INTF) ||
	    (intf > SB_FAB_DEVICE_SIRIUS_HG3_INTF)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, protect gport 0x%x is not a higig interface, Unit(%d)\n"),
	               FUNCTION_NAME(), old_port, unit));	
	    return BCM_E_PARAM;
	}
    }

    /* find the first available map_index on the new interface */
    rv = soc_sirius_es_node_map_index_first_available(unit, SIRIUS_ES_LEVEL_INTERFACE, intf,
						      &map_index);
    if (rv != SOC_E_NONE) {
	return rv;
    }

    /* disable the channel level node first */
    level2_node = sp_info->es_scheduler_level2_node;
    rv = soc_sirius_es_node_hierachy_config(unit, SIRIUS_ES_LEVEL_CHANNEL, level2_node, FALSE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE);
    if (rv != SOC_E_NONE) {
	return rv;
    }

    /* switch interface of all subports and fifos under the fabric port */
    for (egroup = 0; egroup < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; egroup++) {
	if ((sp_info->egroup[egroup].num_fifos > 0) &&
	    (sp_info->egroup[egroup].num_fifos <= SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX)) {
	    level1_node = sp_info->es_scheduler_level1_node[egroup];
	    if (level1_node >= 0) {
		rv = soc_sirius_es_node_hierachy_config(unit, SIRIUS_ES_LEVEL_SUBPORT, level1_node,
							SOC_SIRIUS_API_PARAM_NO_CHANGE, intf,
							SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE);
		if (rv != SOC_E_NONE) {
		    return rv;
		}
	    }

	    for (level0_node = sp_info->egroup[egroup].es_scheduler_level0_node;
		 level0_node < sp_info->egroup[egroup].es_scheduler_level0_node + sp_info->egroup[egroup].num_fifos;
		 level0_node++) {
		if ((level0_node % 4) == 0) {
		    /* every 4 fifos is a fifo group */
		    rv = soc_sirius_es_node_hierachy_config(unit, SIRIUS_ES_LEVEL_FIFO, level0_node,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE, intf,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		}
	    }
	}
    }
    
    /* switch the chanel level node interface, update map_index to make sure it's uniq on new interface */
    rv = soc_sirius_es_node_hierachy_config(unit, SIRIUS_ES_LEVEL_CHANNEL, level2_node,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, intf,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE);
    if (rv != SOC_E_NONE) {
	return rv;
    }

    SOC_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, level2_node, &channel_entry));
    soc_mem_field32_set(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf, map_index);
    SOC_IF_ERROR_RETURN(WRITE_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, level2_node, &channel_entry));

    /* reenable the channel level node */
    rv = soc_sirius_es_node_hierachy_config(unit, SIRIUS_ES_LEVEL_CHANNEL, level2_node,
					    TRUE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE);
    if (rv != SOC_E_NONE) {
	return rv;
    }

    return rv;
}

int
bcm_sirius_failover_set(int unit,
			int sysport,
			int protect_node, 
			int protect_port,
			int active_node, 
			int active_port)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_sirius_failover_destroy(int unit,
			    int sysport)
{
    int rv = BCM_E_NONE;
    return rv;
}



