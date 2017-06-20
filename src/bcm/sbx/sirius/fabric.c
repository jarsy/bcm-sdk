/*
 * $Id: fabric.c,v 1.160 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SIRIUS Fabric Control API
 */

#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <soc/sbx/sbFabCommon.h>

#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>
#include <soc/higig.h>

/* QMB_PKT_HDR_ADJUST registers */
static const soc_reg_t _qmb_pkt_hdr_adjust[17] = {QMB_PKT_HDR_ADJUST3r,
                                                  QMB_PKT_HDR_ADJUST3r,
                                                  QMB_PKT_HDR_ADJUST3r,
                                                  QMB_PKT_HDR_ADJUST3r,
                                                  QMB_PKT_HDR_ADJUST2r,
                                                  QMB_PKT_HDR_ADJUST2r,
                                                  QMB_PKT_HDR_ADJUST2r,
                                                  QMB_PKT_HDR_ADJUST2r,
                                                  QMB_PKT_HDR_ADJUST1r,
                                                  QMB_PKT_HDR_ADJUST1r,
                                                  QMB_PKT_HDR_ADJUST1r,
                                                  QMB_PKT_HDR_ADJUST1r,
                                                  QMB_PKT_HDR_ADJUST0r,
                                                  QMB_PKT_HDR_ADJUST0r,
                                                  QMB_PKT_HDR_ADJUST0r,
                                                  QMB_PKT_HDR_ADJUST0r,
                                                  QMB_PKT_HDR_ADJUST4r};
/* TX_PKT_HDR_ADJUST registers */
static const soc_reg_t _tx_pkt_hdr_adjust[17] = {TX_PKT_HDR_ADJUST3r,
                                                 TX_PKT_HDR_ADJUST3r,
                                                 TX_PKT_HDR_ADJUST3r,
                                                 TX_PKT_HDR_ADJUST3r,
                                                 TX_PKT_HDR_ADJUST2r,
                                                 TX_PKT_HDR_ADJUST2r,
                                                 TX_PKT_HDR_ADJUST2r,
                                                 TX_PKT_HDR_ADJUST2r,
                                                 TX_PKT_HDR_ADJUST1r,
                                                 TX_PKT_HDR_ADJUST1r,
                                                 TX_PKT_HDR_ADJUST1r,
                                                 TX_PKT_HDR_ADJUST1r,
                                                 TX_PKT_HDR_ADJUST0r,
                                                 TX_PKT_HDR_ADJUST0r,
                                                 TX_PKT_HDR_ADJUST0r,
                                                 TX_PKT_HDR_ADJUST0r,
                                                 TX_PKT_HDR_ADJUST4r};
/* PKT_HDR_ADJUST fields (QMB and TX registers) */
static const soc_field_t _pkt_hdr_adjust[17] = {PKT_HDR_ADJUST0f,
                                                PKT_HDR_ADJUST1f,
                                                PKT_HDR_ADJUST2f,
                                                PKT_HDR_ADJUST3f,
                                                PKT_HDR_ADJUST4f,
                                                PKT_HDR_ADJUST5f,
                                                PKT_HDR_ADJUST6f,
                                                PKT_HDR_ADJUST7f,
                                                PKT_HDR_ADJUST8f,
                                                PKT_HDR_ADJUST9f,
                                                PKT_HDR_ADJUST10f,
                                                PKT_HDR_ADJUST11f,
                                                PKT_HDR_ADJUST12f,
                                                PKT_HDR_ADJUST13f,
                                                PKT_HDR_ADJUST14f,
                                                PKT_HDR_ADJUST15f,
                                                GLOBAL_PKT_HDR_ADJUSTf};
/* PKT_HDR_ADJUST_SIGN fields (QMB and TX registers) */
static const soc_field_t _pkt_hdr_adjust_sign[17] = {PKT_HDR_ADJUST_SIGN0f,
                                                     PKT_HDR_ADJUST_SIGN1f,
                                                     PKT_HDR_ADJUST_SIGN2f,
                                                     PKT_HDR_ADJUST_SIGN3f,
                                                     PKT_HDR_ADJUST_SIGN4f,
                                                     PKT_HDR_ADJUST_SIGN5f,
                                                     PKT_HDR_ADJUST_SIGN6f,
                                                     PKT_HDR_ADJUST_SIGN7f,
                                                     PKT_HDR_ADJUST_SIGN8f,
                                                     PKT_HDR_ADJUST_SIGN9f,
                                                     PKT_HDR_ADJUST_SIGN10f,
                                                     PKT_HDR_ADJUST_SIGN11f,
                                                     PKT_HDR_ADJUST_SIGN12f,
                                                     PKT_HDR_ADJUST_SIGN13f,
                                                     PKT_HDR_ADJUST_SIGN14f,
                                                     PKT_HDR_ADJUST_SIGN15f,
                                                     GLOBAL_PKT_HDR_ADJUST_SIGNf};


int
bcm_sirius_fabric_port_destroy(int unit, bcm_gport_t child_gport)
{
    int rv = BCM_E_UNAVAIL;
    bcm_sbx_subport_info_t *sp_info = NULL;
    uint32 flags = 0;
    uint32 level, node;
    int child_node;
    int32  parent_level, parent_node, num_child, first_child, tmp_node;
    int domain, index, eg;
    int level0_node = -1, level1_node = -1, level2_node = -1, level3_node = -1;
    int intf = -1, fifo;
    bcm_gport_t egroup_gport = -1;
    int handle = 0, eg_n = 0;
    int num_fifos;

    if (BCM_GPORT_IS_CHILD(child_gport)) {
	handle = BCM_GPORT_CHILD_PORT_GET(child_gport);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(child_gport)) {
	handle = BCM_GPORT_EGRESS_CHILD_PORT_GET(child_gport);
    } else if (BCM_GPORT_IS_MODPORT(child_gport) && 
	       IS_CPU_PORT(unit, BCM_GPORT_MODPORT_PORT_GET(child_gport))) {
	handle = SOC_SBX_STATE(unit)->port_state->cpu_fabric_port;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(child_gport) && 
	       IS_CPU_PORT(unit, BCM_GPORT_EGRESS_MODPORT_PORT_GET(child_gport))) {
	handle = SOC_SBX_STATE(unit)->port_state->cpu_fabric_port;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "ERROR: unsupported gport (0x%x) for fabric port destroy (%d)\n"),
                   child_gport, rv));
	return BCM_E_PARAM;
    }

    if ( (handle < 0) || (handle >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) ) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "ERROR: gport 0x%x not valid on unit %d\n"),
                   child_gport, unit));
        return BCM_E_PARAM;	
    } else {
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[handle]);
	if (sp_info->valid != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: gport 0x%x not in use on unit %d\n"),
	               child_gport, unit));
	    return BCM_E_PARAM;	
	}

	flags = sp_info->flags;
	/* release TS resources for internal multicast port */
	if ( flags & SBX_SUBPORT_FLAG_INGRESS_MCAST ) {
	    /* Ingress multicast port always on level 6 and on one of RQ interface nodes */
	    level = sp_info->ts_scheduler_level;
	    node = sp_info->ts_scheduler_node;
	    if (level != 6) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: ingress multicast gport 0x%x not at level 6 on unit %d\n"),
		           child_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    /* make sure no children is attached to this scheduler */
	    rv = soc_sirius_ts_node_hierachy_config_get(unit, level, node,
							&parent_node, &first_child, &num_child);
	    if (rv != SOC_E_NONE) {
		return rv;
	    }

	    if ((parent_node != SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ0_NODE) &&
		(parent_node != SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ1_NODE)) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: ingress multicast gport 0x%x not attached to proper level 7 nodes on unit %d\n"),
		           child_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    if (num_child != 0) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: ingress multicast gport 0x%x still has children, could not be destroyed on unit %d\n"),
		           child_gport, unit));
		return BCM_E_PARAM;
	    }

	    /* disconnect the scheduler */
	    parent_level = 7;
	    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, level,
						      node, -1, FALSE, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to disconnect child level %d node %d from parent level %d node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, parent_level, parent_node, unit));
		return rv;
	    }	    

	    /* dealloc the scheduler */
	    rv = bcm_sirius_cosq_scheduler_free(unit, level, child_node, FALSE);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, can not free level %d node %d ingress scheduler,  Unit(%d)\n"),
		           FUNCTION_NAME(), level, child_node, unit));
		return rv;
	    }	    

	    /* update the full map, point to a invalid level/node would disable the domain */	    
	    if (parent_node == SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ0_NODE) {
		intf = SB_FAB_DEVICE_SIRIUS_RQ0_INTF;
	    } else {
		intf = SB_FAB_DEVICE_SIRIUS_RQ1_INTF;
	    }
            SOC_SBX_SIRIUS_STATE(unit)->uNumInternalSubports[intf]--;
		
	    if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) {
                domain = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS / 2;
	    } else {
		domain = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS - 1;
	    }
	    
	    rv = soc_sirius_ts_fullmap_config(unit, domain, 0, 0);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to disable fullmap for gport %d, Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return rv;
	    }	    
	} else {
	    /* release ES resources */
	    if (flags & SBX_SUBPORT_FLAG_ON_ES) {

		/* deallocate hardware scheduler resources */
		level2_node = sp_info->es_scheduler_level2_node;

		if ((SOC_SBX_CFG(unit)->fabric_egress_setup) ||
		    (handle == SOC_SBX_STATE(unit)->port_state->cpu_fabric_port) ||
		    (flags & SBX_SUBPORT_FLAG_INTERNAL)) {
		    /* should free level 0 - 2 */
		    level1_node = sp_info->es_scheduler_level1_node[eg_n];
		    level0_node = sp_info->egroup[eg_n].es_scheduler_level0_node;
		    egroup_gport = sp_info->egroup[eg_n].egroup_gport;
		    num_fifos = SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize;

		    if ((level0_node >= 0) && (level1_node >= 0)) {
			/* detach fifo level nodes */
			for (fifo = level0_node; fifo < level0_node + num_fifos; fifo++) {
			    rv = bcm_sirius_cosq_scheduler_disconnect(unit, SIRIUS_ES_LEVEL_SUBPORT, level1_node,
								      SIRIUS_ES_LEVEL_FIFO, fifo, 0, TRUE, &child_node);
			    if (rv != BCM_E_NONE) {
				LOG_ERROR(BSL_LS_BCM_FABRIC,
				          (BSL_META_U(unit,
				                      "ERROR: %s, ES failed to dettach fifo node %d\n"),
				           FUNCTION_NAME(), fifo));
				return rv;
			    }
			}
		    }

		    if ((level1_node >= 0) && (level2_node >= 0)) {
			/* detach subport level nodes */
			rv = bcm_sirius_cosq_scheduler_disconnect(unit, SIRIUS_ES_LEVEL_CHANNEL, level2_node,
								  SIRIUS_ES_LEVEL_SUBPORT, level1_node, 0, TRUE, &child_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, ES failed to dettach subport node %d\n"),
			               FUNCTION_NAME(), level1_node));
			    return rv;
			}

			/* free subport level nodes */
			rv = bcm_sirius_cosq_scheduler_free(unit, SIRIUS_ES_LEVEL_SUBPORT, child_node, TRUE);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, ES failed to free subport node %d\n"),
			               FUNCTION_NAME(), child_node));
			    return rv;
			}
		    } else {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to find parent node for subport node %d\n"),
			           FUNCTION_NAME(), level1_node));
			return BCM_E_INTERNAL;
		    }

		    /* free egroup */
		    rv = bcm_sbx_cosq_egress_group_free(unit, egroup_gport);

		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to free egress group associated with the fabric port\n"),
			           FUNCTION_NAME()));
			return rv;
		    }
		}

		if ((level2_node >= 0) && 
		    (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP))) {
		    rv = soc_sirius_es_node_get_info(unit, SIRIUS_ES_LEVEL_CHANNEL, level2_node,
						     &level3_node, &index);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to find parent node for channel node %d\n"),
			           FUNCTION_NAME(), level2_node));
			return rv;
		    }
		    
		    if ((level3_node < 0) || (level3_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3)) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to find parent node for channel node %d\n"),
			           FUNCTION_NAME(), level2_node));
			return BCM_E_INTERNAL;			    
		    }
		    
		    /* detach channel level nodes */
		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, SIRIUS_ES_LEVEL_INTERFACE, level3_node,
							      SIRIUS_ES_LEVEL_CHANNEL, level2_node, 0, TRUE, &child_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to dettach channel node %d\n"),
			           FUNCTION_NAME(), level2_node));
			return rv;
		    }
		    
		    /* free channel level nodes */
		    rv = bcm_sirius_cosq_scheduler_free(unit, SIRIUS_ES_LEVEL_CHANNEL, child_node, TRUE);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to free channel node %d\n"),
			           FUNCTION_NAME(), child_node));
			return rv;
		    }

                    /* This assumes intf and level3_nodes are identical, see sirius.h for this */
                    intf = level3_node;

                    if ( flags & (SBX_SUBPORT_FLAG_TRUNK_UCAST | 
                                  SBX_SUBPORT_FLAG_TRUNK_MCAST |
                                  SBX_SUBPORT_FLAG_EGRESS_REQUEUE) ) {
                        /* these are internal subports on ES */
                        SOC_SBX_SIRIUS_STATE(unit)->uTotalInternalSubports--;
                        SOC_SBX_SIRIUS_STATE(unit)->uNumInternalSubports[intf]--;
                    } else {
                        /* these are external subports on ES */
                        SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf]--;
                    }

                    /* 
                     * Update Root scheduler based on the number of ports on higig interfaces
                     */
                    rv = soc_sirius_es_root_scheduler_config(unit);
                    if (rv != SOC_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "ERROR: %s, ES failed to config root scheduler, unit: %d\n"),
                                   FUNCTION_NAME(), unit));
                        return(rv);
                    }
                }
	    }

	    /* release TS resources */
	    if (flags & SBX_SUBPORT_FLAG_ON_TS) {
		level = sp_info->ts_scheduler_level;
		node = sp_info->ts_scheduler_node;

		/* make sure no children is attached to this scheduler */
		rv = soc_sirius_ts_node_hierachy_config_get(unit, level, node,
							    &parent_node, &first_child, &num_child);
		if (rv != SOC_E_NONE) {
		    return rv;
		}

		if (num_child != 0) {
		    LOG_ERROR(BSL_LS_BCM_FABRIC,
		              (BSL_META_U(unit,
		                          "ERROR: fabric gport 0x%x still has children, could not be destroyed on unit %d\n"),
		               child_gport, unit));
		    return BCM_E_PARAM;
		}

		/* disconnect the scheduler */
		if (level == 6) {
		    parent_level = 7;
		    rv = soc_sirius_ts_node_hierachy_config_get(unit, parent_level, parent_node,
								&tmp_node, &first_child, &num_child);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }

		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, level,
							      node, -1, FALSE, &child_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disconnect child level %d node %d from parent level %d node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), level, node, parent_level, parent_node, unit));
			return rv;
		    }

		    /* dealloc the scheduler, the node might be swapped during disconnect, 
		     * and should always free the last child of parent 
		     */
		    node  = first_child + num_child - 1;
		    rv = bcm_sirius_cosq_scheduler_free(unit, level, child_node, FALSE);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not free level %d node %d ingress scheduler,  Unit(%d)\n"),
			           FUNCTION_NAME(), level, child_node, unit));
			return rv;
		    }
		} else if (level == 5) {
		    /* at level 5 */
		    parent_level = 6;
		    rv = soc_sirius_ts_node_hierachy_config_get(unit, parent_level, parent_node,
								&tmp_node, &first_child, &num_child);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }

		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, level,
							      node, -1, FALSE, &child_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disconnect child level %d node %d from parent level %d node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), level, node, parent_level, parent_node, unit));
			return rv;
		    }

		    /* dealloc the scheduler, the node might be swapped during disconnect, 
		     * and should always the last child of parent 
		     */
		    node  = first_child + num_child - 1;
		    rv = bcm_sirius_cosq_scheduler_free(unit, level, child_node, FALSE);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not free level %d node %d ingress scheduler,  Unit(%d)\n"),
			           FUNCTION_NAME(), level, child_node, unit));
			return rv;
		    }
		    
		    /* get parent info */
		    level = parent_level;
		    node = parent_node;
		    if (num_child == 0) {
			/* was the last child of parent */
			rv = soc_sirius_ts_node_hierachy_config_get(unit, parent_level, parent_node,
								&parent_node, &first_child, &num_child);
			if (rv != SOC_E_NONE) {
			    return rv;
			}

			parent_level = 7;
			rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, level,
								  node, -1, FALSE, &child_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to disconnect child level %d node %d from parent level %d node %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), level, node, parent_level, parent_node, unit));
			    return rv;
			}

			node  = first_child + num_child - 1;
			rv = bcm_sirius_cosq_scheduler_free(unit, level, child_node, FALSE);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, can not free level %d node %d ingress scheduler,  Unit(%d)\n"),
			               FUNCTION_NAME(), level, child_node, unit));
			    return rv;
			}
		    }
		}

		/* update full map */
		if (sp_info->egroup[0].ef_fcd != (uint16)BCM_INT_SBX_INVALID_FCD) {
		    if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
			domain = sp_info->egroup[0].ef_fcd;
		    } else {
			domain = sp_info->egroup[0].ef_fcd/2;
		    }
		    rv = soc_sirius_ts_fullmap_config(unit, domain, 0, 0);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disable fullmap for gport %d, Unit(%d)\n"),
			           FUNCTION_NAME(), child_gport, unit));
			return rv;
		    }		
		}
	    }

	    /* Free memory resources for the CPU port */
	    if ((SOC_SBX_CFG(unit)->fabric_egress_setup) || (handle == SOC_SBX_STATE(unit)->port_state->cpu_fabric_port)) {
		rv = soc_sirius_config_ff(unit, handle, 0, FF_MEM_DELETE);
		if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "Unit %d Dynamic port destroy failed to delete egress fifos memory\n"),
                               unit));
		    return rv;
		}
	    }
	}

	/* update software states */
	sp_info->valid = FALSE;
	sp_info->flags = 0;
	sp_info->parent_gport = BCM_GPORT_INVALID;
	sp_info->original_gport = BCM_GPORT_INVALID;
	sp_info->group_shaper = -1;
	sp_info->port_offset = -1;
	sp_info->ts_scheduler_level = -1;
	sp_info->ts_scheduler_node = -1;
	sp_info->es_scheduler_level2_node = -1;
	for (eg=0; eg < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; eg++) {
	    sp_info->es_scheduler_level1_node[eg] = -1;
	}
    }

    return rv;
}

int
bcm_sirius_fabric_port_get(int unit, bcm_gport_t parent_port,
			      int offset, uint32 flags, int *subport)
{
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_gport_t intf_gport = BCM_GPORT_INVALID, intf_e_gport = BCM_GPORT_INVALID;
    int indx, port = -1;

    if (BCM_GPORT_IS_MODPORT(parent_port)) {
	port = BCM_GPORT_MODPORT_PORT_GET(parent_port);
	intf_gport = parent_port;
	BCM_GPORT_EGRESS_MODPORT_SET(intf_e_gport, BCM_GPORT_MODPORT_MODID_GET(parent_port), port);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(parent_port)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(parent_port);
	BCM_GPORT_MODPORT_SET(intf_gport, BCM_GPORT_EGRESS_MODPORT_MODID_GET(parent_port), port);
	intf_e_gport = parent_port;
    } else {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, gport 0x%x unsupported type for egress scheduler, unit %d\n"),
                   FUNCTION_NAME(), parent_port, unit));
	return BCM_E_PARAM;
    }

    if (offset >= SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, offset %d out of range for interface, unit %d\n"),
	           FUNCTION_NAME(), offset, unit));
	return BCM_E_PARAM;
    }

    for (indx = 0; indx < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; indx++) {
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[indx]);
	if (sp_info->valid == TRUE) {
	    if (((sp_info->parent_gport == intf_gport) || (sp_info->parent_gport == intf_e_gport)) && 
		(sp_info->port_offset == offset)) {
		*subport = indx;
		return BCM_E_EXISTS;
	    }
	}
    }
    return BCM_E_NOT_FOUND;
}

int
bcm_sirius_fabric_port_create(int unit, bcm_gport_t parent_port,
			      int offset, uint32 flags, int *subport)
{
    int rv = BCM_E_UNAVAIL;
    int port, hg, intf;
    uint32 es_flags, ts_flags;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int num_fifos = 0;

    es_flags = 0;
    ts_flags = 0;

    if (BCM_GPORT_IS_MODPORT(parent_port) || BCM_GPORT_IS_EGRESS_MODPORT(parent_port)) {
	if (BCM_GPORT_IS_MODPORT(parent_port)) {
	    port = BCM_GPORT_MODPORT_PORT_GET(parent_port);
	} else {
	    port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(parent_port);
	}

	/* get interface info */
	if (IS_GX_PORT(unit, port)) {
	    hg = SOC_PORT_OFFSET(unit, port);
	    if ( (hg < 0) || (hg >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) ) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: Invalid higig interface port %d\n"),
		           hg));
		return BCM_E_PARAM;
	    }
	    intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
	} else if (IS_CPU_PORT(unit, port)) {
	    intf = SB_FAB_DEVICE_SIRIUS_CPU_INTF;
	    if (SOC_SBX_STATE(unit)->port_state->cpu_fabric_port != -1) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: cpu port already created\n")));
		return BCM_E_PARAM;
	    }
	} else if (IS_REQ_PORT(unit, port)) {
	    /* create internal subport on requeue interface 
	     *    requeue subport on ES only             
	     *         (for hybrid mode requeue)
	     *         (for multicast requeue under TME mode-"BCM_FABRIC_PORT_EGRESS_MULTICAST")
	     *    ingress multicast scheduler port on TS only
	     *         (for multicast ingress scheduling-"BCM_FABRIC_PORT_INGRESS_MULTICAST")
	     *    ingress unicast requeue port on both TS and ES
	     *         (for special customer request, requeue unicast traffic under TME mode)
	     */
	    intf = SOC_PORT_OFFSET(unit, port);
	    intf += SB_FAB_DEVICE_SIRIUS_RQ0_INTF;

	    if (((flags & BCM_FABRIC_PORT_EGRESS_MULTICAST) == 0) &&
		((flags & BCM_FABRIC_PORT_INGRESS_MULTICAST) == 0) &&
		(soc_feature(unit, soc_feature_standalone))) {
		/* ingress unicast requeueu port under TME mode */
		ts_flags |= SBX_SUBPORT_FLAG_SUBSCRIBER_REQUEUE;
		es_flags |= SBX_SUBPORT_FLAG_SUBSCRIBER_REQUEUE;
	    } else {
		es_flags |= SBX_SUBPORT_FLAG_EGRESS_REQUEUE;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Invalid destination interface gport 0x%x for port creation, unit %d\n"),
	               FUNCTION_NAME(), parent_port, unit));
	    return BCM_E_PARAM;
	}
    } else if (BCM_GPORT_IS_SCHEDULER(parent_port)) {
	
        return BCM_E_UNAVAIL;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "ERROR: unsupported parent gport (0x%x) for fabric port creation (%d)\n"),
                   parent_port, rv));
        return BCM_E_PARAM;
    }

    num_fifos = SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize;

    if (flags & BCM_FABRIC_PORT_EGRESS_MULTICAST) {
	/* only need to return a handle for shaping purpose later.
	 * reserve a fixed handle and fixed group shaper for MC trunking
	 * we don't mark the subport as used since there is really no
	 * resources associated with this handle
	 */

	switch(port) {
	    case 0:
		*subport = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0;
		break;
	    case 1:
		*subport = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE1;
		break;
	    case 2:
		*subport = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE2;
		break;
	    case 3:
		*subport = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE3;
		break;
	    default:
		*subport = -1;
		return BCM_E_UNAVAIL;
		break;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[*subport]);
	if (sp_info->valid == FALSE) {
	    sp_info->valid = TRUE;
	    sp_info->parent_gport = parent_port;
	    sp_info->flags |= SBX_SUBPORT_FLAG_TRUNK_MCAST;
	}
	return BCM_E_NONE;
    }

    if (flags & BCM_FABRIC_PORT_INGRESS_MULTICAST) {
	ts_flags |= SBX_SUBPORT_FLAG_INGRESS_MCAST;
    }

    if ( ts_flags & SBX_SUBPORT_FLAG_INGRESS_MCAST ) {
	/* create internal multicast port on TS, use the last fabric port */
	*subport = SB_FAB_DEVICE_SIRIUS_INGRESS_MCAST_HANDLE;

	rv = bcm_sirius_fabric_port_create_ts(unit, parent_port, ts_flags, *subport);
	if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "Unit %d Dynamic port creation failed to create ingress scheduler\n"),
                       unit));
	    return rv;
	}

	/* no need to allocate fifos for internal multicast ports */
    }
    else
    {
	/* Check if port already exists */
	rv = bcm_sirius_fabric_port_get(unit, parent_port, offset, flags, subport);
	if (rv == BCM_E_EXISTS) {
	    return rv;
	} else if (rv != BCM_E_NOT_FOUND) {
	    *subport = -1;
	    return rv;
	}

	/* create regular child ports on Higig, CPU, Requeue interfaces on ES*/
	rv = bcm_sirius_fabric_port_create_es(unit, parent_port, offset, num_fifos,
					      es_flags, subport);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "Unit %d Dynamic port creation failed to create egress scheduler\n"),
	               unit));
	    return rv;
	}

	if (intf == SB_FAB_DEVICE_SIRIUS_CPU_INTF) {
	    SOC_SBX_STATE(unit)->port_state->cpu_fabric_port = *subport;
	}

        if ( (soc_feature(unit, soc_feature_standalone) ||
	      soc_feature(unit, soc_feature_hybrid)) &&
	     ((flags & BCM_FABRIC_PORT_EGRESS_MULTICAST) == 0) &&
	     ((es_flags & SBX_SUBPORT_FLAG_EGRESS_REQUEUE) == 0) ) {

	    if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, TM_FABRIC_PORT_HIERARCHY_SETUP) != 0) {
		/* create regular child ports on Higig, CPU on TS */
		rv = bcm_sirius_fabric_port_create_ts(unit, parent_port, ts_flags, *subport);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_FABRIC,
		              (BSL_META_U(unit,
		                          "Unit %d Dynamic port creation failed to create ingress scheduler\n"),
		               unit));
		    return rv;
		}
	    } else {
		/* skip regular child port creation on TS when told so */
		rv = BCM_E_NONE;
	    }
	}

	/* Allocate or Update memory resources for the CPU port */
	if ((SOC_SBX_CFG(unit)->fabric_egress_setup) || (*subport == SOC_SBX_STATE(unit)->port_state->cpu_fabric_port)) {
	    rv = soc_sirius_config_ff(unit, *subport, 0, FF_MEM_UPDATE);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "Unit %d Dynamic port creation failed to create egress fifos\n"),
		           unit));
		return rv;
	    }
	}
    }

    return rv;
}

int
bcm_sirius_fabric_congestion_size_set(int unit, bcm_module_t module_id, int max_ports)
{
    int rv = BCM_E_NONE;
    int max_port_limit = SOC_HIGIG_E2ECC_MAX_PORT_STATUS;
    
    if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
	max_port_limit *= 2 -1;
    }

    if (max_ports > max_port_limit) {
	return(BCM_E_PARAM);
    }
    if ((module_id < 0) || (module_id > SOC_HIGIG_HDR_MAX_MODID)) {
	return(BCM_E_PARAM);
    }

    if (max_ports == 0) {
	rv = bcm_sirius_cosq_module_congestion_deallocate(unit, module_id, max_ports);
	if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "ERROR: %s, deallocating congestion resources, unit: %d, module: 0x%x\n"),
                       FUNCTION_NAME(), unit, module_id));
	    return(rv);
	}

	rv = soc_sirius_es_fc_base_table_set(unit, module_id, 0, 0, FALSE);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, clearing congestion resources, unit: %d, module: 0x%x rc: 0x%x\n"),
	               FUNCTION_NAME(), unit, module_id, rv));
	    return(rv);
	}
    }
    
    
    /*       before HiGig congestion is enabled. Need to do add consistency check */  
    
    SOC_SBX_STATE(unit)->fabric_state->congestion_size[module_id] = (max_ports & 0xFF);
    
    return rv;
}

int
bcm_sirius_fabric_congestion_size_get(int unit, bcm_module_t module_id, int *max_ports)
{
    int rv = BCM_E_NONE;

    if ((module_id < 0) || (module_id > SOC_HIGIG_HDR_MAX_MODID)) {
        return(BCM_E_PARAM);
    }

    *max_ports = SOC_SBX_STATE(unit)->fabric_state->congestion_size[module_id];

    return rv;
}


/* create a node hierachy in TS block till the fabric port level
 * In:
 *    physical_gport   - physical interface gport
 *    flags            - flags for this subport
 *    handle           - handle to identify the fabric port
 *                       returned by the bcm_sirius_fabric_port_create_es, 
 *                       so have to call that first except the ingress multicast subport
 */
int
bcm_sirius_fabric_port_create_ts(int unit,
				 bcm_gport_t physical_gport,
				 uint32 flags,
				 int handle)
{
    bcm_port_t port, hg_port;
    bcm_gport_t intf_gport;
    int level = 0, node = 0, level7_node, level6_node, level5_node;
    int subport, intf, hg_intf;
    int hg_subport_cnt, hgs_subport_cnt;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int domain;
    int rv = BCM_E_NONE;

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Unit %d in FIC mode, can not create ingress scheduler hierachy\n"),
                   unit));
        return BCM_E_UNAVAIL;	
    }

    if ((flags & SBX_SUBPORT_FLAG_INGRESS_MCAST) == 0) {
	if ( (handle < 0) || (handle >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) ) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "Unit %d subport %d out of range [0-SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)\n"),
	               unit, handle));
	    return BCM_E_INTERNAL;	
	} else {
	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[handle]);
	    if (sp_info->flags & SBX_SUBPORT_FLAG_ON_TS) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "Unit %d subport %d already has ingress schedulers\n"),
		           unit, handle));
		return BCM_E_INTERNAL;
	    }
	}
    }

    /* create ingress schedulers for subports */
    if (BCM_GPORT_IS_MODPORT(physical_gport)) {
	port = BCM_GPORT_MODPORT_PORT_GET(physical_gport);
	intf_gport = physical_gport;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(physical_gport)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(physical_gport);
	BCM_GPORT_MODPORT_SET(intf_gport, BCM_GPORT_EGRESS_MODPORT_MODID_GET(physical_gport), port);
    } else {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, gport 0x%x unsupported type for ingress scheduler , unit %d\n"),
                   FUNCTION_NAME(), physical_gport, unit));
	return BCM_E_PARAM;    
    }

    if(!SOC_PORT_VALID(unit, port)) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid interface gport 0x%x, unit %d\n"),
	           FUNCTION_NAME(), physical_gport, unit));
	return BCM_E_PARAM;
    }

    /* make sure ingress multicast child port could only be created on REQ port
     * and req port can only create ingress multicast child port
     */
    if ( (flags & SBX_SUBPORT_FLAG_INGRESS_MCAST) && (!IS_REQ_PORT(unit, port)) ) {
	/* for multicast, only allow it be created on req port
	 *  otherwise will waste child resource for a higig interfaces (up to 8 child only)
	 */
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid interface gport 0x%x for BCM_FABRIC_PORT_INGRESS_MULTICAST flag, require REQ port. unit %d\n"),
	           FUNCTION_NAME(), physical_gport, unit));
	return BCM_E_PARAM;	
    }

    /* get the interface level (level 7) ingress scheduler node */
    rv = bcm_sbx_port_get_scheduler(unit, intf_gport, &level, &level7_node);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to find level 7 ingress scheduler node for gport 0x%x, Unit(%d)\n"),
	           FUNCTION_NAME(), physical_gport, unit));
	return BCM_E_RESOURCE;
    }

    /* Assumption: intf is equal to level 7 ingress node used */
    intf = level7_node; 

    /* mapping between level 7 nodes and level 6 nodes.
     * This is for NULL grants management
     * level 7 node 0 --> level 6 node 48-55. 48 reserved for plane A
     *                    only nodes 49-55 are usable to user
     * level 7 node 1 --> level 6 node 0--7. 0 is reserved for plane B
     *                    only nodes 1-7 are usable to uer
     * level 7 node 2 --> level 6 node 8--15
     * level 7 node 3 --> level 6 node 16-23
     * level 7 node 4 --> level 6 node 24-31
     * level 7 node 5 --> level 6 node 32-39
     * level 7 node 6 --> level 6 node 40-47
     * 
     */
    if (IS_CPU_PORT(unit, port) || (flags & SBX_SUBPORT_FLAG_INGRESS_MCAST)) {
	/* CPU or Multicast default to level 6 */
	level = 6;
	if (level7_node == 0) {
	    /* node 49 is the first node available to user */
	    level6_node = 49;
	} else {
	    level6_node = (level7_node-1) * 8;	
	}

        domain = handle;

	rv = bcm_sirius_cosq_scheduler_allocate(unit, 6, level6_node, 1, FALSE, &node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to create level 6 ingress scheduler node on gport %d, Unit(%d)\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_RESOURCE;
	}
	
	rv = bcm_sirius_cosq_scheduler_connect(unit, 7, level7_node, 6, level6_node, -1, FALSE, -1);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to connect level 6 ingress scheduler node on gport %d,  Unit(%d)\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_RESOURCE;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[handle]);
	sp_info->valid = TRUE;
	sp_info->flags |= SBX_SUBPORT_FLAG_ON_TS;
	if (flags & SBX_SUBPORT_FLAG_INGRESS_MCAST) {
	    /* ingress multicast subport only exist on TS */
	    sp_info->flags |= SBX_SUBPORT_FLAG_INGRESS_MCAST;
	    sp_info->flags &= ~(SBX_SUBPORT_FLAG_ON_ES);
            SOC_SBX_SIRIUS_STATE(unit)->uNumInternalSubports[intf]++;

	    if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) {
                domain = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS / 2;
	    } else {
		domain = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS - 1;
	    }
	}

	sp_info->ts_scheduler_level = level;
	sp_info->ts_scheduler_node = level6_node;

	/* update the full map */
	rv = soc_sirius_ts_fullmap_config(unit, domain, sp_info->ts_scheduler_level,
					  sp_info->ts_scheduler_node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to config fullmap for subport %d, Unit(%d)\n"),
	               FUNCTION_NAME(), handle, unit));
	    return rv;
	}	
    } else if ((IS_GX_PORT(unit, port)) || 
	       (flags & SBX_SUBPORT_FLAG_SUBSCRIBER_REQUEUE)) {
	/* Adjust the level/nodes based on number of external ports on an higig interface,
	 * if equal or more than 8, put all subport level nodes at level 5, otherwise 
	 * put all subport level nodes at level 6.
	 */

	/* we have to reallocate all ingress scheduler nodes because the TS node
	 * connection is not flexible
	 */
      
	/* first free all ingress schedulers for higig ports */
	for (level = 4; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
	    for (node = 0; node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; node++) {

		if ( (level == 7) && (node == SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_CPU_NODE) ) {
		    /* no need to reconfig level 7 nodes for CPU ports */
		    continue;
		}

		if ( (level == 6) && 
		     (((node >= 48) &&
		       (node < (48 + 8))))) {
		    /* no need to reconfig level 6 nodes for CPU ports */
		    continue;
		}

		if ( (level != 7) && (node >= (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] - 2)) ) {
		    /* don't reconfig last 2 nodes on the level */
		    continue;
		}

		rv = bcm_sirius_cosq_scheduler_init(unit, level, node, FALSE);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_FABRIC,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free ingress scheduler level %d node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, unit));
		    return BCM_E_RESOURCE;
		}
	    }
	}

	/* recreate the interface schedulers */
	level = 7;
	for (node = 0; node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; node++) {
	    if (node == SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_CPU_NODE) {
		continue;
	    }

	    rv = bcm_sirius_cosq_scheduler_allocate(unit, level, node, 1, FALSE, &node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to create scheduler for intf %d, Unit(%d)\n"),
		           FUNCTION_NAME(),
		           node, unit));
		return BCM_E_RESOURCE;
	    }	    
	    /* parent is root, no child are connected */
	    SOC_IF_ERROR_RETURN(soc_sirius_ts_node_hierachy_config(unit, level, node, 0, 0, 0));
	}

	/* recalculate level 6/5 nodes for the subports, bcm_sirius_fabric_port_create_es is required to
	 * be called first, so the the subport database should already setup except the ts_scheduler_level
	 * and ts_scheduler_node
	 */
	hgs_subport_cnt = 0; /* count all subports on all higigs */
	for (hg_intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF; hg_intf <= SB_FAB_DEVICE_SIRIUS_RQ1_INTF; hg_intf++) {
	    if (hg_intf < SB_FAB_DEVICE_SIRIUS_RQ0_INTF) {
		hg_port = SOC_PORT_MIN(unit, gx) + (hg_intf - SB_FAB_DEVICE_SIRIUS_HG0_INTF);
	    } else {
		hg_port = SOC_PORT_MIN(unit, req) + (hg_intf - SB_FAB_DEVICE_SIRIUS_RQ0_INTF);
	    }

	    subport = BCM_GPORT_INVALID;
	    hg_subport_cnt = 0; /* count all subport on this higig */

            if (SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[hg_intf] <= 8) {
		if (hg_intf == SB_FAB_DEVICE_SIRIUS_HG0_INTF) {
		    if (SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[hg_intf] <= 7) {
			level = 6;
		    } else {
			level = 5;
		    }
		} else {
		    level = 6;
		}
	    } else {
		level = 5;
	    }

	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "%s %d external subports on intf %d unit %d\n"),
	              FUNCTION_NAME(), SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[hg_intf],
	              hg_intf, unit));

	    if (hg_intf == SB_FAB_DEVICE_SIRIUS_HG0_INTF) {
		/* reserve level6 node 0 for plane B NULL grant, even if there is no supborts on higig 0 */
		level6_node = 0;
		
		rv = bcm_sirius_cosq_scheduler_allocate(unit, 6, level6_node, 1, FALSE, &level6_node);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_FABRIC,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to create level 6 scheduler node for port %d on higig %d, Unit(%d)\n"),
		               FUNCTION_NAME(), port, hg_intf, unit));
		    return BCM_E_RESOURCE;
		}
		
		rv = bcm_sirius_cosq_scheduler_connect(unit, 7, hg_intf, 6, level6_node, -1, FALSE, -1);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_FABRIC,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to connect level 6 scheduler node for port %d on higig %d,  Unit(%d)\n"),
		               FUNCTION_NAME(), port, hg_intf, unit));
		    return BCM_E_RESOURCE;
		}
		
		hg_subport_cnt++;		    
	    }

	    do { /* while (no errors and not yet run out of subports) */
		/* go through all existing subports on the interface */
		rv = bcm_sirius_port_subport_getnext(unit, hg_port, SBX_SUBPORT_FLAG_INTERNAL, &subport);
		if (rv != BCM_E_NONE) {
		    /* errors */
		    break;
		}
		if (subport == BCM_GPORT_INVALID) {
		    /* done with current interface */
		    break;
		} else if (BCM_GPORT_IS_CHILD(subport)) {
		    handle = BCM_GPORT_CHILD_PORT_GET(subport);
		} else {
		    handle = BCM_GPORT_EGRESS_CHILD_PORT_GET(subport);
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[handle]);
		sp_info->ts_scheduler_level = level;
		sp_info->flags |= SBX_SUBPORT_FLAG_ON_TS;

		if (level == 6) {
		    level6_node = (hg_intf - 1) * 8 + hg_subport_cnt;

		    rv = bcm_sirius_cosq_scheduler_allocate(unit, 6, level6_node, 1, FALSE, &level6_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to create level 6 scheduler node for port %d on higig %d, Unit(%d)\n"),
			           FUNCTION_NAME(), port, hg_intf, unit));
			return BCM_E_RESOURCE;
		    }
		    
		    rv = bcm_sirius_cosq_scheduler_connect(unit, 7, hg_intf, 6, level6_node, -1, FALSE, -1);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to connect level 6 scheduler node for port %d on higig %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), port, hg_intf, unit));
			return BCM_E_RESOURCE;
		    }
		    
		    sp_info->ts_scheduler_node = level6_node;
		} else {
		    /* connect level 6 nodes */
		    if ( (hg_subport_cnt % 8) == 0) {
			level6_node = (hg_intf - 1) * 8 + (hg_subport_cnt / 8);

			rv = bcm_sirius_cosq_scheduler_allocate(unit, 6, level6_node, 1, FALSE, &level6_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to create level 6 scheduler node for port %d on higig %d, Unit(%d)\n"),
			               FUNCTION_NAME(), port, hg_intf, unit));
			    return BCM_E_RESOURCE;
			}
			
			rv = bcm_sirius_cosq_scheduler_connect(unit, 7, hg_intf, 6, level6_node, -1, FALSE, -1);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to connect level 6 scheduler node for port %d on higig %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), port, hg_intf, unit));
			    return BCM_E_RESOURCE;
			}
		    }

		    if ((hg_intf == SB_FAB_DEVICE_SIRIUS_HG0_INTF) && (hgs_subport_cnt == 0)) {
			/* if higig 0 has more than 7 subports, reserve level 5 node 0 for plane B NULL grant path */
			level5_node = 0;
			level6_node = 0;
			rv = bcm_sirius_cosq_scheduler_allocate(unit, 5, level5_node, 1, FALSE, &level5_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to create level 5 scheduler node 0, Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return BCM_E_RESOURCE;
			}
			
			rv = bcm_sirius_cosq_scheduler_connect(unit, 6, level6_node, 5, level5_node, -1, FALSE, -1);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_FABRIC,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to connect level 5 scheduler node 0,  Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return BCM_E_RESOURCE;
			}
			hgs_subport_cnt++;
		    }

		    /* connect level 5 nodes */
		    level5_node = hgs_subport_cnt;
		    rv = bcm_sirius_cosq_scheduler_allocate(unit, 5, level5_node, 1, FALSE, &level5_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to create level 5 scheduler node for port %d on higig %d, Unit(%d)\n"),
			           FUNCTION_NAME(), port, hg_intf, unit));
			return BCM_E_RESOURCE;
		    }
		    
		    rv = bcm_sirius_cosq_scheduler_connect(unit, 6, level6_node, 5, level5_node, -1, FALSE, -1);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to connect level 5 scheduler node for port %d on higig %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), port, hg_intf, unit));
			return BCM_E_RESOURCE;
		    }

		    sp_info->ts_scheduler_node = level5_node;
		}

		/* update the full map */
		if (sp_info->egroup[0].ef_fcd != (uint16)BCM_INT_SBX_INVALID_FCD) {
		    if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
			domain = sp_info->egroup[0].ef_fcd;
		    } else {
			domain = sp_info->egroup[0].ef_fcd/2;
		    }
		    rv = soc_sirius_ts_fullmap_config(unit, domain, sp_info->ts_scheduler_level,
						      sp_info->ts_scheduler_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_FABRIC,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config fullmap for subport %d, Unit(%d)\n"),
			           FUNCTION_NAME(), handle, unit));
			return rv;
		    }
		}

		hg_subport_cnt++;
		hgs_subport_cnt++;
	    } while (1);
	}
    }

    return rv;
}


/* create a node hierachy in ES block till the fifo level for a fabric or internal port
 * In:
 *    physical_gport   - physical gport
 *    num_fifos        - number of fifos for the fabric port
 *    offset           - port offset within the interface port
 *    flags            - flags for this subport
 * In/Out:
 *    handle           - handle to identify the fabric port
 *                       specify a subport if in range [0-SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS),
 *                       <0 to auto-allocate, other values illegal
 */
int
bcm_sirius_fabric_port_create_es(int unit,
				 bcm_gport_t physical_gport,
				 int offset,
				 int num_fifos,
				 uint32 flags,
				 int *handle)
{
    int level0_node = -1, level1_node = -1, level2_node = -1, level3_node = -1, level = 0;
    int intf = -1, fifo, subport = -1, port = -1, modid=0, port_offset = 0;
    bcm_gport_t egroup_gport = -1;
    channel_map_table_entry_t channel_entry;
    bcm_gport_t intf_gport = BCM_GPORT_INVALID, intf_e_gport = BCM_GPORT_INVALID, child_e_gport = BCM_GPORT_INVALID;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int create_level2, create_level1;
    int rv = BCM_E_NONE;

    /* If passed in MODPORT gport, allocate at level 2/1/0, could be used to
     *    create child gports.
     * if passed in Child gport, allocate at level 1/0 only, could be used to
     *    create egress group gports
     * if ES_FABRIC_PORT_HIERARCHY_SETUP == 0, only return a uniq handle without
     *    allocate at any level
     */
    if (BCM_GPORT_IS_MODPORT(physical_gport)) {
	port = BCM_GPORT_MODPORT_PORT_GET(physical_gport);
	modid = BCM_GPORT_MODPORT_MODID_GET(physical_gport);
	intf_gport = physical_gport;
	BCM_GPORT_EGRESS_MODPORT_SET(intf_e_gport, BCM_GPORT_MODPORT_MODID_GET(physical_gport), port);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(physical_gport)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(physical_gport);
	modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(physical_gport);
	BCM_GPORT_MODPORT_SET(intf_gport, BCM_GPORT_EGRESS_MODPORT_MODID_GET(physical_gport), port);
	intf_e_gport = physical_gport;
    } else if (BCM_GPORT_IS_CHILD(physical_gport)) {
	subport = BCM_GPORT_CHILD_PORT_GET(physical_gport);
	modid = BCM_GPORT_CHILD_MODID_GET(physical_gport);
	BCM_GPORT_EGRESS_CHILD_SET(child_e_gport, modid, subport);
	rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, &port_offset);
	if (rv == BCM_E_NONE) {
	    if (!SOC_PORT_VALID(unit, port)) {
		return BCM_E_PORT;
	    }
	}
    } else if (BCM_GPORT_IS_EGRESS_CHILD(physical_gport)) {
	subport = BCM_GPORT_EGRESS_CHILD_PORT_GET(physical_gport);
	modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(physical_gport);
	child_e_gport = physical_gport;
	rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, &port_offset);
	if (rv == BCM_E_NONE) {
	    if (!SOC_PORT_VALID(unit, port)) {
		return BCM_E_PORT;
	    }
	}
    } else {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, gport 0x%x unsupported type for egress scheduler , unit %d\n"),
	           FUNCTION_NAME(), physical_gport, unit));
	return BCM_E_PARAM;
    }

    /* multicast internal ports only needed on TS */
    if (flags & SBX_SUBPORT_FLAG_INGRESS_MCAST) {
	return BCM_E_PARAM;
    }

    /* only support num_fifos in multiple of 4, and less than 48 */
    if ( (num_fifos <= 0) || (num_fifos > 48) ) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "ERROR: %s, num_fifos %d not in range (0-48] for gport 0x%x, unit %d\n"),
	           FUNCTION_NAME(), num_fifos, physical_gport, unit));
	return BCM_E_PARAM;
    } else {
	/* round up to next multiple of 4 */
	num_fifos = ((num_fifos + 3) >> 2) << 2;
    }

    /* For CPU port and internal port, level 2 to 0 are always created,
     * if ES_FABRIC_PORT_HIERARCHY_SETUP == FALSE, level2 and below are not allocated and connected
     * if SOC_SBX_CFG(unit)->fabric_egress_setup == FALSE, level 1 and below are not allocated and connected
     *    but level 2 is still created, the level 2 node id is returned as handle to guarantee the 
     *    uniq of fabric port handle
     * otherwise level 2 to 0 are always created and connected
     */
    if (IS_CPU_PORT(unit, port) ||
	(flags & SBX_SUBPORT_FLAG_INTERNAL)) {
	create_level2 = TRUE;
	create_level1 = TRUE;
    } else if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP) == FALSE) {
	create_level2 = FALSE;
	create_level1 = FALSE;
    } else if (SOC_SBX_CFG(unit)->fabric_egress_setup == FALSE) {
	create_level2 = TRUE;
	create_level1 = FALSE;
    } else {
	create_level2 = TRUE;
	create_level1 = TRUE;
    }

    if (BCM_GPORT_IS_MODPORT(physical_gport) || BCM_GPORT_IS_EGRESS_MODPORT(physical_gport)) {
	if(!SOC_PORT_VALID(unit, port)) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid gport 0x%x, unit %d\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_PARAM;
	}

	/* get the interface level (level 3) egress scheduler node */
	rv = bcm_sbx_port_get_scheduler(unit, intf_e_gport, &level, &level3_node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to find level 3 egress scheduler node for gport 0x%x, Unit(%d)\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_RESOURCE;
	}

	if (create_level2) {
	    /* the level 2 nodes in ES is channel level, should be allocated then attached to level 3 nodes 
	     * with a uniq map_index
	     */
	    /* channel level */
	    rv = bcm_sirius_cosq_scheduler_allocate(unit, SIRIUS_ES_LEVEL_CHANNEL, 
						    BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT,
						    1, TRUE, &level2_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to allocate level 2 egress scheduler node on gport 0x%x, Unit(%d)\n"),
		           FUNCTION_NAME(), physical_gport, unit));
		return BCM_E_RESOURCE;
	    }
	    
	    rv = bcm_sirius_cosq_scheduler_connect(unit, SIRIUS_ES_LEVEL_INTERFACE, 
						   level3_node, SIRIUS_ES_LEVEL_CHANNEL,
						   level2_node, -1, TRUE, -1);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, ES failed to attach channel node %d\n"),
		           FUNCTION_NAME(), level2_node));
		return rv;
	    }
	} else {
	    level2_node = -1;
	}

	intf = level3_node;
    } else {
	if (create_level2) {
	    /* get the channel level (level 2) egress scheduler node */
	    rv = bcm_sbx_port_get_scheduler(unit, child_e_gport, &level, &level2_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to find level 2 egress scheduler node for gport 0x%x, Unit(%d)\n"),
		           FUNCTION_NAME(), physical_gport, unit));
		return BCM_E_RESOURCE;
	    }
	    
	    /* get the interface level (level 3) egress scheduler node */
	    SOC_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, level2_node, &channel_entry));
	    level3_node = soc_mem_field32_get(unit, CHANNEL_MAP_TABLEm, &channel_entry, INTFf);
	    
	    intf = level3_node; /* This assumes the intf ID and level 3 node ID are same */
	    /* the interface level gport of the created port should be same as its parent child gport */
	    intf_gport = SOC_SBX_STATE(unit)->port_state->subport_info[subport].parent_gport;
	} else {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, es_fabric_port_hierarchy_setup is 0, 0x%x has no level 2 node resource"
	                           "can not create egress group on it. Unit(%d)\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_PARAM;
	}
    }

    if ((create_level2 == TRUE) && (create_level1 == TRUE)) {
	/* the level 1 nodes in ES is subport level, should be allocated then attached to level 2 nodes
	 * with a uniq map_index, if we only allow 1 subport for each channel (may not be true anymore)
	 * then we can live with the default map_index of 0
	 */
	
	if (IS_CPU_PORT(unit, port)) {
	    level1_node = *handle;
	} else {
	    level1_node = BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT;
	}
	
	/* subport level */
	rv = bcm_sirius_cosq_scheduler_allocate(unit, SIRIUS_ES_LEVEL_SUBPORT, 
						level1_node,
						1, TRUE, &level1_node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to create level 1 egress scheduler node on gport 0x%x, Unit(%d)\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_RESOURCE;
	}
	
	rv = bcm_sirius_cosq_scheduler_connect(unit, SIRIUS_ES_LEVEL_CHANNEL, level2_node,
					       SIRIUS_ES_LEVEL_SUBPORT, level1_node, -1, TRUE, -1);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ES failed to attach suport node %d\n"),
	               FUNCTION_NAME(), level1_node));
	    return rv;
	}
	
	if (*handle < 0) {
	    /* no longer user level1_node as subport id, this is to support more than 132 ports */
	    *handle = level1_node;
	}
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[*handle]);

	if ((sp_info->valid != FALSE) && ((flags & SBX_SUBPORT_FLAG_IN_TRUNK) == 0))  {
	    return BCM_E_RESOURCE;
	} else {
	    sp_info->valid = TRUE;
	    sp_info->parent_gport = intf_gport;
	    sp_info->port_offset = offset;
	    sp_info->flags = flags;
	}
 
	/* the level 0 nodes could be allocated and attached to level 1 nodes with a uniq map_index
	 * Fifos are always allocated in the multiple of num_fifos, and in a consective chunk
	 */

	if (sp_info->egroup[0].es_scheduler_level0_node == 0xffff) {
	    if (IS_CPU_PORT(unit, port))  {
		rv = bcm_sbx_cosq_egress_group_allocate(unit, level1_node, modid,
							*handle, num_fifos, &egroup_gport);
	    } else {
		rv = bcm_sbx_cosq_egress_group_allocate(unit, BCM_INT_SBX_REQUESTED_EGROUP_AUTO_SELECT, modid,
							*handle, num_fifos, &egroup_gport);
	    }
	}
	
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to allocate egress group on gport 0x%x, Unit(%d)\n"),
	               FUNCTION_NAME(), physical_gport, unit));
	    return BCM_E_RESOURCE;
	}
	
	level0_node = sp_info->egroup[0].es_scheduler_level0_node;
	for (fifo = level0_node; fifo < level0_node + num_fifos; fifo++) {
	    rv = bcm_sirius_cosq_scheduler_connect(unit, SIRIUS_ES_LEVEL_SUBPORT, level1_node,
						   SIRIUS_ES_LEVEL_FIFO, fifo, -1, TRUE, fifo - level0_node);
	    if (rv != BCM_E_NONE) {
		bcm_sbx_cosq_egress_group_free(unit, egroup_gport);
		sp_info->valid = FALSE;

		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, ES failed to attach fifo node %d\n"),
		           FUNCTION_NAME(), fifo));
		return rv;
	    }
	}	
    } else if ((create_level2 == FALSE) && (create_level1 == FALSE)) {
	level1_node = -1;
	level2_node = -1;

	if (*handle < 0) {
	    for (subport = 0; subport < SB_FAB_DEVICE_MAX_FABRIC_PORTS; subport++) {
		if (SOC_SBX_STATE(unit)->port_state->subport_info[subport].valid == FALSE) {
		    *handle = subport;
		    break;
		}
	    }
	    if (*handle < 0) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, out of subport id resource\n"),
		           FUNCTION_NAME()));
		return BCM_E_RESOURCE;
	    }
	} else if (*handle < SB_FAB_DEVICE_MAX_FABRIC_PORTS) {
	    if (SOC_SBX_STATE(unit)->port_state->subport_info[*handle].valid == TRUE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "ERROR: %s, subport id %d already used\n"),
		           FUNCTION_NAME(), *handle));
		return BCM_E_PARAM;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: %s, subport id %d out of range, Unit(%d)\n"),
	               FUNCTION_NAME(), *handle, unit));
	    return BCM_E_PARAM;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[*handle]);
	sp_info->valid = TRUE;
	sp_info->parent_gport = intf_gport;
	sp_info->port_offset = offset;
	sp_info->flags = flags;

    } else {
	/*
	 * reusing child_e_gport for child_gport in diag debug
	 */
	BCM_GPORT_CHILD_SET(child_e_gport, modid, level2_node);
	if (IS_GX_PORT(unit, port)) {
	    LOG_VERBOSE(BSL_LS_BCM_FABRIC,
	                (BSL_META_U(unit,
	                            "created port %d for higig %d offset %d, handle (0x%x)\n"),
	                 level2_node, port, offset, child_e_gport));
	} else if (IS_REQ_PORT(unit, port)) { 
	    LOG_VERBOSE(BSL_LS_BCM_FABRIC,
	                (BSL_META_U(unit,
	                            "created port %d for requeue offset %d, handle (0x%x)\n"),
	                 level2_node, level2_node, child_e_gport));
	} else {
	    return BCM_E_PARAM;
	}

	/* Use the level 2 node id as the port handle, this will guarantee
	 * it's in range for fabric ports
	 */
	*handle = level2_node;
 
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[*handle]);
	sp_info->valid = TRUE;
	sp_info->parent_gport = intf_gport;
	sp_info->port_offset = offset;
	sp_info->flags = flags;
    }

    /* Update the subport state */
    sp_info->es_scheduler_level2_node = level2_node;
    sp_info->es_scheduler_level1_node[0] = level1_node;

    if ( flags & (SBX_SUBPORT_FLAG_TRUNK_UCAST | 
		  SBX_SUBPORT_FLAG_TRUNK_MCAST |
		  SBX_SUBPORT_FLAG_EGRESS_REQUEUE) ) {
	/* these are internal subports on ES */
        SOC_SBX_SIRIUS_STATE(unit)->uTotalInternalSubports++;
        SOC_SBX_SIRIUS_STATE(unit)->uNumInternalSubports[intf]++;

	sp_info->flags |= SBX_SUBPORT_FLAG_ON_ES;
	
	/* these internal subports won't be on TS scheduler hierachy */
	sp_info->flags &= (~SBX_SUBPORT_FLAG_ON_TS);
    } else {
	/* these are external subports on ES */
        SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf]++;

	sp_info->flags |= SBX_SUBPORT_FLAG_ON_ES;
    }

    /* 
     * Update Root scheduler based on the number of ports on higig interfaces
     */
    rv = soc_sirius_es_root_scheduler_config(unit);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ES failed to config root scheduler, unit: %d\n"), unit));
	return(rv);
    }
    
    return BCM_E_NONE;
}



int
bcm_sirius_fabric_crossbar_connection_set(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t dst_xbport)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}


int
bcm_sirius_fabric_sci_plane_get(int unit, bcm_port_t port, int *plane_a)
{
    int rc = BCM_E_NONE;
    bcm_port_t sci_port;


    if (!IS_SCI_PORT(unit, port)) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "Not an SCI port(%d)\n"),
	           port));
	return BCM_E_PARAM;
    }

    /* assume arbiter ID 0 is plane A and arbiter ID 1 is plane B                   */
    /* per SDK assumptions.  Also assume SCI0 is arbiter ID 0 and                   */
    /* SCI1 is arbiter id 1.  This assumption must be revisited                     */
    
    sci_port = port - 31;

    if (sci_port == 0) {
	*plane_a = TRUE;
    } else {
	*plane_a = FALSE;
    }
    return rc;
}

int
bcm_sirius_fabric_crossbar_connection_get(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}
int
bcm_sirius_fabric_tdm_enable_set(int unit,
				 int enable)
{
    int rv = BCM_E_NONE;
    uint32 uRegValue;

    if (enable) {
	
	/* Use polling scheme to use the queue length polling scheme - when a queue */
	/* length request is received from the BM9600 for queue 0x3ffe will cause a */
	/* calendar swap.  Initially, we are calendar 0 at time=0                   */
	SOC_IF_ERROR_RETURN(READ_SC_CONFIG_TDMCALSWAPr(unit, &uRegValue));
	soc_reg_field_set(unit, SC_CONFIG_TDMCALSWAPr, &uRegValue, POLLING_SWITCH_SELECTf, 1);
	SOC_IF_ERROR_RETURN(WRITE_SC_CONFIG_TDMCALSWAPr(unit, uRegValue));

	/* queue to switch on is 0x3ffe and disable the next generation BME (not polaris) use mechanism */
	SOC_IF_ERROR_RETURN(READ_SC_CONFIG_TDMCALSWAPr(unit, &uRegValue));
	soc_reg_field_set(unit, SC_CONFIG_TDMCALSWAPr, &uRegValue, POLLING_SWITCH_QUEUEf, 0x3ffe);
	soc_reg_field_set(unit, SC_CONFIG_TDMCALSWAPr, &uRegValue, GRANTTAG7_SWITCH_ENABLEf, 0);
	soc_reg_field_set(unit, SC_CONFIG_TDMCALSWAPr, &uRegValue, START_POLLING_STATEf, 0);
	SOC_IF_ERROR_RETURN(WRITE_SC_CONFIG_TDMCALSWAPr(unit, uRegValue));

	SOC_IF_ERROR_RETURN(READ_QS_CONFIG0r(unit, &uRegValue));
	soc_reg_field_set(unit, QS_CONFIG0r, &uRegValue, CALENDAR_ENABLEf, 1);
	SOC_IF_ERROR_RETURN(WRITE_QS_CONFIG0r(unit, uRegValue));

	SOC_IF_ERROR_RETURN(READ_CALENDAR_CONFIGr(unit, &uRegValue));
	soc_reg_field_set(unit, CALENDAR_CONFIGr, &uRegValue, QUEUE_B_ENf,1);
	soc_reg_field_set(unit, CALENDAR_CONFIGr, &uRegValue, QUEUE_A_ENf,1);
	SOC_IF_ERROR_RETURN(WRITE_CALENDAR_CONFIGr(unit, uRegValue));
	
    } else {

	SOC_IF_ERROR_RETURN(READ_QS_CONFIG0r(unit, &uRegValue));
	soc_reg_field_set(unit, QS_CONFIG0r, &uRegValue, CALENDAR_ENABLEf, 0);
	SOC_IF_ERROR_RETURN(WRITE_QS_CONFIG0r(unit, uRegValue));

	SOC_IF_ERROR_RETURN(READ_CALENDAR_CONFIGr(unit, &uRegValue));
	soc_reg_field_set(unit, CALENDAR_CONFIGr, &uRegValue, QUEUE_B_ENf,0);
	soc_reg_field_set(unit, CALENDAR_CONFIGr, &uRegValue, QUEUE_A_ENf,0);
	SOC_IF_ERROR_RETURN(WRITE_CALENDAR_CONFIGr(unit, uRegValue));

    }
    return rv;
}

int
bcm_sirius_fabric_tdm_enable_get(int unit,
				 int *enable)
{
    int rv = BCM_E_NONE;
    uint32 uRegValue;

    SOC_IF_ERROR_RETURN(READ_QS_CONFIG0r(unit, &uRegValue));
    *enable = soc_reg_field_get(unit, QS_CONFIG0r, uRegValue, CALENDAR_ENABLEf);
    
    return rv;
}

int
bcm_sirius_fabric_calendar_max_get(int unit, int *max_size) 
{
    int rv=BCM_E_NONE;
    
    /* Each calendar entry is comprised of a plane A queue and a plane B queue.   */
    /* The calendar size returned is the number of plane A and plane B entries.   */
    /* Odd indexes map to plane A and even entries map to plane B.                */
    if (SOC_SBX_CFG(unit)->epoch_length_in_timeslots < SB_FAB_DEVICE_SIRIUS_NUM_TDM_CALENDAR_ENTRIES_PER_PLANE) {

	*max_size = SOC_SBX_CFG(unit)->epoch_length_in_timeslots *  SB_FAB_MAX_NUM_GRANT_PLANES; 

    } else {

	*max_size = SB_FAB_DEVICE_SIRIUS_NUM_TDM_CALENDAR_ENTRIES_PER_PLANE * SB_FAB_MAX_NUM_GRANT_PLANES;
    }
    return rv;
}

int
bcm_sirius_fabric_calendar_size_set(int unit, int config_size)
{
    int rv = BCM_E_NONE;
    uint32 uRegValue;

    if (config_size % SB_FAB_MAX_NUM_GRANT_PLANES) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Calendar size must be an even number: value(%d) invalid\n"),
                   config_size));  
	return BCM_E_PARAM;
    }

    
    SOC_IF_ERROR_RETURN(READ_CALENDAR_CONFIGr(unit, &uRegValue));
    soc_reg_field_set(unit, CALENDAR_CONFIGr, &uRegValue, CAL_SIZEf,config_size/SB_FAB_MAX_NUM_GRANT_PLANES - 1);
    SOC_IF_ERROR_RETURN(WRITE_CALENDAR_CONFIGr(unit, uRegValue));

    return rv;
}

extern int
bcm_sirius_fabric_calendar_size_get(int unit, int *config_size)
{
    int rv = BCM_E_NONE;
    uint32 uRegValue;

    SOC_IF_ERROR_RETURN(READ_CALENDAR_CONFIGr(unit, &uRegValue));
    *config_size = soc_reg_field_get(unit, CALENDAR_CONFIGr, uRegValue, CAL_SIZEf);
    *config_size = *config_size * SB_FAB_MAX_NUM_GRANT_PLANES;

    return rv;
}


int
bcm_sirius_fabric_calendar_set(int unit,
			       int cindex,
			       int connection,
			       bcm_gport_t dest_port,
			       bcm_cos_queue_t dest_cosq)
{
    int rv = BCM_E_NONE;
    calendar_entry_t calendarEntry;
    uint32 cal_index = cindex/SB_FAB_MAX_NUM_GRANT_PLANES;
    uint32 cal_plane = cindex%SB_FAB_MAX_NUM_GRANT_PLANES;
    int base_queue;
    int queue;
    int active_calendar;
    uint32 reg_value;

    if (cal_index > SOC_MEM_INFO(unit, CALENDAR0m).index_max) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "Calendar index (%d) larger than calendar size\n"),
	           cindex));
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(dest_port)) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "Calendar only supports unicast queue group gport dest_port at this time\n")));
	return BCM_E_PARAM;
    }
    if (connection > SOC_SBX_CFG(unit)->epoch_length_in_timeslots) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "timeslot requested(%d) for calendar is greater than the epoch length(%d)\n"), 
	           connection, SOC_SBX_CFG(unit)->epoch_length_in_timeslots));
	return BCM_E_PARAM;
    }

    base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(dest_port);
    queue = base_queue + dest_cosq;

    if (queue > SB_FAB_DEVICE_SIRIUS_NUM_QUEUES) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "gport queue is greater than maximum allowed(%d)\n"),
	           queue));
    } 

    /* Write the entry */
    sal_memset(&calendarEntry, 0, sizeof(calendar_entry_t));
    soc_mem_field32_set(unit, CALENDARm, &calendarEntry, TIMESLOTf, connection);
    if (cal_plane == 1) {
	soc_mem_field32_set(unit, CALENDARm, &calendarEntry, CALENDAR_QUEUE1f, queue);
    } else {
	soc_mem_field32_set(unit, CALENDARm, &calendarEntry, CALENDAR_QUEUE0f, queue);
    }

    /* Determine the currently active calendar */
    SOC_IF_ERROR_RETURN(READ_CALENDAR_CONFIGr(unit, &reg_value));
    active_calendar = soc_reg_field_get(unit, CALENDAR_CONFIGr, reg_value, CALENDAR_ACTIVE_IDf);

    /* Write to the opposite calendar that is active */
    if (active_calendar == 1) {
	SOC_IF_ERROR_RETURN(WRITE_CALENDAR0m(unit, MEM_BLOCK_ANY, cal_index, &calendarEntry));
    }else {
	SOC_IF_ERROR_RETURN(WRITE_CALENDAR1m(unit, MEM_BLOCK_ANY, cal_index, &calendarEntry));
    }

    return rv;
}

int
bcm_sirius_fabric_calendar_get(int unit,
			       int cindex,
			       int *connection,
			       bcm_gport_t *dest_port,
			       bcm_cos_queue_t *dest_cosq)
{
    int rv = BCM_E_NONE;
    calendar_entry_t calendarEntry;
    uint32 cal_index = cindex/SB_FAB_MAX_NUM_GRANT_PLANES;
    uint32 cal_plane = cindex%SB_FAB_MAX_NUM_GRANT_PLANES;
    int queue;
    int active_calendar;
    uint32 reg_value;

    if (cal_index > SOC_MEM_INFO(unit, CALENDAR0m).index_max) {
	LOG_ERROR(BSL_LS_BCM_FABRIC,
	          (BSL_META_U(unit,
	                      "Calendar index (%d) larger than calendar size\n"),
	           cindex));
        return BCM_E_PARAM;
    }

    /* Determine the currently active calendar */
    SOC_IF_ERROR_RETURN(READ_CALENDAR_CONFIGr(unit, &reg_value));
    active_calendar = soc_reg_field_get(unit, CALENDAR_CONFIGr, reg_value, CALENDAR_ACTIVE_IDf);

    if (active_calendar == 1) {
	SOC_IF_ERROR_RETURN(READ_CALENDAR0m(unit, MEM_BLOCK_ANY, cal_index, &calendarEntry));
    } else {
	SOC_IF_ERROR_RETURN(READ_CALENDAR1m(unit, MEM_BLOCK_ANY, cal_index, &calendarEntry));
    }

    *connection = soc_mem_field32_get(unit, CALENDARm, &calendarEntry, TIMESLOTf);
    if (cal_plane == 1) {
	queue = soc_mem_field32_get(unit, CALENDARm, &calendarEntry, CALENDAR_QUEUE1f);
    } else {
	queue = soc_mem_field32_get(unit, CALENDARm, &calendarEntry, CALENDAR_QUEUE0f);
    }

    rv = bcm_sbx_cosq_get_gport_cos_from_qid(unit, queue, dest_port, dest_cosq);

    return rv;
}


int
bcm_sirius_fabric_calendar_multi_set(int unit,
				     int array_size,
				     int *connection_array,
				     bcm_gport_t *dest_port_array,
				     bcm_cos_queue_t *dest_cosq_array)
{
    int rv = BCM_E_NONE;
    int cindex;

    for (cindex = 0; cindex < array_size; cindex++) {
	rv = bcm_sirius_fabric_calendar_set(unit,
					    cindex,
					    connection_array[cindex],
					    dest_port_array[cindex],
					    dest_cosq_array[cindex]);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "Error configuring cindex(%d), aborting\n"),
	               cindex));
	    return rv;
	}    
    }    
    return rv;
}

int
bcm_sirius_fabric_calendar_multi_get(int unit,
				     int array_size,
				     int *connection_array,
				     bcm_gport_t *dest_port_array,
				     bcm_gport_t *dest_cosq_array)
{
    int rv = BCM_E_NONE;
    int cindex;

    for (cindex = 0; cindex < array_size; cindex++) {
	rv = bcm_sirius_fabric_calendar_get(unit,
					    cindex,
					    &connection_array[cindex],
					    &dest_port_array[cindex],
					    &dest_cosq_array[cindex]);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "Error accessing cindex(%d), aborting\n"),
	               cindex));
	    return rv;
	}    
    }    
    return rv;
}

/* 
 * Map a logical channel (0-45) to a physical channel (0-45)
 * in the SFI block
 */
static int
_bcm_sirius_fabric_map_lchan_to_pchan(int unit, 
                                      int lchan, 
                                      int pchan)
{
    int rv = BCM_E_NONE;
    uint32 regval = 0;
    uint32 regAddr;
    uint32 index;

    if ((lchan >= SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS) || 
        (pchan >= SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS)) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "lchan[%d] or pchan[%d] out of range\n"),
                   lchan, pchan));
        return rv;
    }

    /* mapping:
     *  sfi_port 0-19 mapped to SC_TOP_SFI_NUM_REMAP0 0-19
     *  sfi_port 20-43 mapped to SFI_NUM_REMAP0 0-23
     *  sfi_port 44-45 mapped to SC_TOP_SFI_NUM_REMAP0 20-21
     */
    if (pchan < 20) {
        index = pchan;
        regAddr = SC_TOP_SFI_NUM_REMAP0r;
    } else if (pchan < 44) {
        index = pchan - 20;
        regAddr = SFI_NUM_REMAP0r;
    } else {
        index = pchan - 24;
        regAddr = SC_TOP_SFI_NUM_REMAP0r;
    }
    
    SOC_IF_ERROR_RETURN(soc_reg32_read(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                       &regval));
    
    soc_reg_field_set(unit, regAddr, &regval, REMAPPED_SFI_NUM_0f, lchan);
    
    SOC_IF_ERROR_RETURN(soc_reg32_write(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                        regval));
    
    SOC_SBX_STATE(unit)->fabric_state->xbar_state->lchan_to_pchan[lchan] = pchan;
    SOC_SBX_STATE(unit)->fabric_state->xbar_state->pchan_to_lchan[pchan] = lchan;

    return rv;
}


/*
 * Get the pchan (0-45) in SFI associated with the lchan (0-45).  If lchan is not 
 * mapped, search remap table to see if lchan is associated with a different pchan in 
 * hardware.  If so, unmap. If there is an associated pchan mapped to the
 * lchannel, just unmap that one without searching.
 */
static int
_bcm_sirius_fabric_unmap_lchan_to_pchan(int unit, 
                                        int lchan /* 0-45 */)
{
    int rv = BCM_E_NONE;
    uint32 regval = 0;
    uint32 regAddr;
    uint32 index;
    int    pchan;
    int    lchan_read;

    /* No effect if channel invalid indication */
    if (lchan == -1) {
        return BCM_E_NONE;
    }

    if (lchan >= SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "ERROR: unit(%d) lchan out of range(%d) max(%d)\n"),
                   unit, lchan,
                   SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS));
        return BCM_E_PARAM;
    }
    pchan = SOC_SBX_STATE(unit)->fabric_state->xbar_state->lchan_to_pchan[lchan];
    
    /* Channel not mapped */
    if (pchan == SB_FAB_DEVICE_INVALID_DATA_CHANNEL) {
	
        /* Make sure it's not reverse mapped, to a different lchan */
        for (pchan=0; pchan<SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS; pchan++) {
            /* mapping:
             *  sfi_port 0-19 mapped to SC_TOP_SFI_NUM_REMAP0 0-19
             *  sfi_port 20-43 mapped to SFI_NUM_REMAP0 0-23
             *  sfi_port 44-45 mapped to SC_TOP_SFI_NUM_REMAP0 20-21
             */
            if (pchan < 20) {
                index = pchan;
                regAddr = SC_TOP_SFI_NUM_REMAP0r;
            } else if (pchan < 44) {
                index = pchan - 20;
                regAddr = SFI_NUM_REMAP0r;
            } else {
                index = pchan - 24;
                regAddr = SC_TOP_SFI_NUM_REMAP0r;
            }
            
            SOC_IF_ERROR_RETURN(soc_reg32_read(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                               &regval));
            
            lchan_read = soc_reg_field_get(unit, regAddr, regval, REMAPPED_SFI_NUM_0f);
            
            /* Unmap this channel */
            if (lchan_read == lchan) {
                soc_reg_field_set(unit, regAddr, &regval, REMAPPED_SFI_NUM_0f, 0x3f);
		
                SOC_IF_ERROR_RETURN(soc_reg32_write(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                                    regval));
                
                SOC_SBX_STATE(unit)->fabric_state->xbar_state->pchan_to_lchan[pchan] = SB_FAB_DEVICE_INVALID_DATA_CHANNEL;
                SOC_SBX_STATE(unit)->fabric_state->xbar_state->lchan_to_pchan[lchan] = SB_FAB_DEVICE_INVALID_DATA_CHANNEL;

            }
        }
    } else {
        
        /* mapping:
         *  sfi_port 0-19 mapped to SC_TOP_SFI_NUM_REMAP0 0-19
         *  sfi_port 20-43 mapped to SFI_NUM_REMAP0 0-23
         *  sfi_port 44-45 mapped to SC_TOP_SFI_NUM_REMAP0 20-21
         */
        if (pchan < 20) {
            index = pchan;
            regAddr = SC_TOP_SFI_NUM_REMAP0r;
        } else if (pchan < 44) {
            index = pchan - 20;
            regAddr = SFI_NUM_REMAP0r;
        } else {
            index = pchan - 24;
            regAddr = SC_TOP_SFI_NUM_REMAP0r;
        }
        
        SOC_IF_ERROR_RETURN(soc_reg32_read(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                           &regval));
        
        soc_reg_field_set(unit, regAddr, &regval, REMAPPED_SFI_NUM_0f, 0x3f);
        
        SOC_IF_ERROR_RETURN(soc_reg32_write(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                            regval));

        SOC_SBX_STATE(unit)->fabric_state->xbar_state->pchan_to_lchan[pchan] = SB_FAB_DEVICE_INVALID_DATA_CHANNEL;
        SOC_SBX_STATE(unit)->fabric_state->xbar_state->lchan_to_pchan[lchan] = SB_FAB_DEVICE_INVALID_DATA_CHANNEL;
        
    }
    
    return rv;
}

/* 
 * Unmap the lxbar link status bit to internal SFI (0-23) mapping 
*/ 
int
bcm_sirius_fabric_unmap_link_status(int unit, 
                                    int sfi_port)
{
    int rv = BCM_E_NONE;
    int lxbar;
    uint32 shift = 0, uData = 0;
    uint32 mask = 0x1f;

    /* 
     * loop through all logical xbars, if the port remap in the link_status_remap matches
     * to the port specified, init the setting to 0x3f (disable)
     */
    for (lxbar = 0; lxbar < SB_FAB_DEVICE_MAX_LOGICAL_CROSSBARS; lxbar++) {
        shift = (lxbar % 6) * 5;
        if ( lxbar < 6 ) {
            SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP0r(unit, &uData));
            
            if (((uData & (mask << shift))>>shift) == sfi_port) {
                uData |= (0x1f << shift);
                SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP0r(unit, uData));
            }
        } else if ( lxbar < 12 ) {
            SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP1r(unit, &uData));
            
            if (((uData & (mask << shift))>>shift) == sfi_port) {
                uData |= (0x1f << shift);
                SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP1r(unit, uData));
            }
        } else if ( lxbar < 18 ) {
            SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP2r(unit, &uData));
            
            if (((uData & (mask << shift))>>shift) == sfi_port) {
                uData |= (0x1f << shift);
                SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP2r(unit, uData));
            }
        } else if (lxbar < 24) {
            SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP3r(unit, &uData));
            
            if (((uData & (mask << shift))>>shift) == sfi_port) {
                uData |= (0x1f << shift);
                SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP3r(unit, uData));
            }
        } else if (lxbar < 30) {
            SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP4r(unit, &uData));
	    
            if (((uData & (mask << shift))>>shift) == sfi_port) {
                uData |= (0x1f << shift);
                SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP4r(unit, uData));
            }
        } else {
            SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP5r(unit, &uData));
            
            if (((uData & (mask << shift))>>shift) == sfi_port) {
                uData |= (0x1f << shift);
                SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP5r(unit, uData));
            }			
        }
    }
    return rv;
}

/*
 * Map the sfi port link status to the lxbar passed in.
 */
int
bcm_sirius_fabric_map_link_status(int unit, 
                                  int lxbar, 
                                  int sfi_port)
{
    int rv = BCM_E_NONE;
    uint32 shift = 0, regval = 0;
    uint32 mask = 0x1F;

#ifdef DEBUG_CROSSBAR_MAP_VERBOSE 
    LOG_CLI((BSL_META_U(unit,
                        "%s: lxbar(%d) sfi_port(%d)\n"), FUNCTION_NAME(), lxbar, sfi_port));
#endif

    /* Status remap logical crossbar to physical crossbar */
    shift = (lxbar % 6) * 5;
    if ( lxbar < 6 ) {
        SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP0r(unit, &regval));
        regval &= ~(mask << shift);
        regval |= ((sfi_port & mask) << shift);
        SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP0r(unit, regval));
    } else if ( lxbar < 12 ) {
        SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP1r(unit, &regval));
        regval &= ~(mask << shift);
        regval |= ((sfi_port & mask) << shift);
        SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP1r(unit, regval));
    } else if (lxbar < 18) {
        SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP2r(unit, &regval));
        regval &= ~(mask << shift);
        regval |= ((sfi_port & mask) << shift);
        SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP2r(unit, regval));
    } else if (lxbar < 24) {
        SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP3r(unit, &regval));
        regval &= ~(mask << shift);
        regval |= ((sfi_port & mask) << shift);
        SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP3r(unit, regval));
    } else if (lxbar < 30) {
        SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP4r(unit, &regval));
        regval &= ~(mask << shift);
        regval |= ((sfi_port & mask) << shift);
        SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP4r(unit, regval));
    } else {
        SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP5r(unit, &regval));
        regval &= ~(mask << shift);
        regval |= ((sfi_port & mask) << shift);
        SOC_IF_ERROR_RETURN(WRITE_SC_LINK_STATUS_REMAP5r(unit, regval));
    }
    return rv;
}

/* 
 * Returns whether lxbar is enabled in either global channel mask a or b 
 */
static int
_bcm_sirius_fabric_global_lchan_get(int unit, 
                                    int lxbar, 
                                    int *used_in_a,
                                    int *used_in_b)
{
    int rv = BCM_E_NONE;
    uint32 regval = 0;
    uint32 lchan_mask = 0;

    if ((used_in_a == NULL) || (used_in_b == NULL)) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s invalid pointer passed for lxbar(%d)\n"),
                   FUNCTION_NAME(), lxbar));
        return BCM_E_INTERNAL;
    }

    if (lxbar >= SB_FAB_DEVICE_MAX_NODE_CROSSBARS) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s lxbar(%d) out of range (0-23)\n"),
                   FUNCTION_NAME(), lxbar));
        return BCM_E_INTERNAL;
    }

    *used_in_a = FALSE;
    *used_in_b = FALSE;

    /* Read global channel enable mask */
    if (lxbar < 16) {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_A_LOr(unit, &regval));
        lchan_mask = 3 << lxbar * 2;
        if (regval & lchan_mask) {
            *used_in_a = TRUE;
        }
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_B_LOr(unit, &regval));
        if (regval & lchan_mask) {
            *used_in_b = TRUE;
        }        
    } else if (lxbar < 22) {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_A_HIr(unit, &regval));
        lchan_mask = 3 << (lxbar-16) * 2;
        if (regval & lchan_mask) {
            *used_in_a = TRUE;
        }
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_B_HIr(unit, &regval));
        if (regval & lchan_mask) {
            *used_in_b = TRUE;
        }
    } /* channel_enable_plane_a[44] = sc_grant.remapped_link_en_a[22] &
       *  per_qe_type_channel_mask_a[44] &
       *  qe_link_enable_config.channel_mask_a[44] & 
       *  (sc_grant.rx_plane_a==0 | qe_link_enable_config.plane_crossover_links[22])
       * Duplicated for b - but special case where doesn't map to 2 channel mask bits
       */
    else if (lxbar == 22) { /* SCI0 special case */
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_A_HIr(unit, &regval));
        lchan_mask = 1 << (44 - 32);
        if (regval & lchan_mask) {
            *used_in_a = TRUE;
        }
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_B_HIr(unit, &regval));
        if (regval & lchan_mask) {
            *used_in_b = TRUE;
        }        
    }
      /* channel_enable_plane_a[45] = sc_grant.remapped_link_en_a[23] &
       *  per_qe_type_channel_mask_a[45] &
       *  qe_link_enable_config.channel_mask_a[45] & 
       *  (sc_grant.rx_plane_a==0 | qe_link_enable_config.plane_crossover_links[23])
       * Duplicated for b - but special case where doesn't map to 2 channel mask bits
       */
    else if (lxbar == 23) {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_A_HIr(unit, &regval));
        lchan_mask = 1 << (45 - 32);
        if (regval & lchan_mask) {
            *used_in_a = TRUE;
        }
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_B_HIr(unit, &regval));
        if (regval & lchan_mask) {
            *used_in_b = TRUE;
        }        
    }

#ifdef DEBUG_CROSSBAR_MAP_VERBOSE 
    if (*used_in_a || *used_in_b) {
        LOG_CLI((BSL_META_U(unit, \
                            "%s: lxbar(%d) A(%s) B(%s)\n"), FUNCTION_NAME(), lxbar, \
                 *used_in_a?"USED":"NOT USED", *used_in_b?"USED":"NOT USED"));
    }
#endif
    
    return rv;
}

/*
 * Update the qe type specific masks for the lxbars passed in.
 * There can be up to 2 logical crossbars associated with a single
 * SFI port (A/A configuration).
 */
static int
_bcm_sirius_fabric_update_qe_type_lchan(int unit, 
                                        int lxbar, 
                                        int lxbar1, 
                                        int node_type, 
                                        int ability, 
                                        int unmap)
{
    int rv = BCM_E_NONE;
    uint32 regval = 0, mask = 0, mask_a = 0, mask_b = 0;

    if (!BCM_SBX_CROSSBAR_VALID(lxbar)) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s lxbar(%d) out of range (0-23)\n"),
                   FUNCTION_NAME(), lxbar));
        return BCM_E_INTERNAL;
    }

    if ((unmap == TRUE) && (BCM_SBX_CROSSBAR_VALID(lxbar1))) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s lxbar1(%d) not valid for unmap\n"),
                   FUNCTION_NAME(), lxbar1));
        return BCM_E_INTERNAL;
    }

    if (unmap == TRUE) {
        /*
         * disable both a and b in all node type channel masks for lxbar (both fr and qm)
         * node type=2 a 00 b 00 in qe_type2_channel_mask and fr_config12/13 (ss hybrid)
         * node type=1 a 00 b 00 in qe type1_channel_mask and fr_config10/11 (qe2k)
         * node type=0 a 00 b 00 in qe_type0_channel_mask and fr_config8/9 (ss only)
         * disable plane crossover
         */
        SOC_IF_ERROR_RETURN(READ_PLANE_CROSSOVERr(unit, &regval));
        regval &= ~(1 << lxbar);
        SOC_IF_ERROR_RETURN(WRITE_PLANE_CROSSOVERr(unit, regval));

        if (lxbar < 16) {
            mask = ~(3 << lxbar * 2);
            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A0_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A1_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A1_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A3_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A3_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B0_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B1_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B1_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B3_LOr(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B3_LOr(unit, regval));
 
            SOC_IF_ERROR_RETURN(READ_FR_CONFIG8r(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG8r(unit, regval));

            SOC_IF_ERROR_RETURN(READ_FR_CONFIG10r(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG10r(unit, regval));

            SOC_IF_ERROR_RETURN(READ_FR_CONFIG12r(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG12r(unit, regval));
           
           SOC_IF_ERROR_RETURN(READ_FR_CONFIG14r(unit, &regval));
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG14r(unit, regval));
  
           /* crossover */
            SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
            regval &= (mask);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));

        } else {
            if (lxbar == 22) {
                mask = ~(1 << (44 - 32));
            } else if (lxbar == 23) {
                mask = ~(1 << (45 - 32));
            } else {
                mask = ~(3 << (lxbar - 16) * 2);
            }

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A1_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A1_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A3_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A3_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B1_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B1_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B3_HIr(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B3_HIr(unit, regval));

            SOC_IF_ERROR_RETURN(READ_FR_CONFIG9r(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG9r(unit, regval));

            SOC_IF_ERROR_RETURN(READ_FR_CONFIG11r(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG11r(unit, regval));

            SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

            SOC_IF_ERROR_RETURN(READ_FR_CONFIG15r(unit, &regval));
            regval &= (mask) & 0x3fff;
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG15r(unit, regval));

            /* crossover */
            SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
            regval &= (mask & 0x3fff);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
        }

    } else {
        switch (node_type) {
            case SIRIUS_NODE_TYPE_FIC:

                if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s lxbar1(%d) not valid for ss node type, there are never 2 xbars\n"),
                               FUNCTION_NAME(), lxbar1));
                    return BCM_E_INTERNAL;
                }
                /* if DUAL_SFI always A/B local
                 * a 01 b 10 qe_type0_channel_mask and fr_config8/9 (ab 11) (ss only)
                 */
                switch (ability) {
                    case BCM_PORT_ABILITY_DUAL_SFI:

                        SOC_IF_ERROR_RETURN(READ_PLANE_CROSSOVERr(unit, &regval));
                        regval |= (1 << lxbar);
                        SOC_IF_ERROR_RETURN(WRITE_PLANE_CROSSOVERr(unit, regval));

                        if (lxbar < 16) {
                            mask  = ~(3 << lxbar * 2);
                            mask_a = (1 << lxbar * 2);
                            mask_b = (2 << lxbar * 2);
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_LOr(unit, &regval));
                            regval &= (mask);
                            regval |= mask_a;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A0_LOr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_LOr(unit, &regval));
                            regval &= (mask);
                            regval |= mask_b;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B0_LOr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG8r(unit, &regval));
                            regval &= (mask);
                            regval |= mask_a | mask_b;
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG8r(unit, regval));

                            /* crossover */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
                            regval &= (mask);
                            regval |= (mask_a | mask_b);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));

                        } else {
                            if (lxbar == 22) {                                
                                mask = ~(1 << (44 - 32));
                                mask_a =(1 << (44 - 32));
                                mask_b =(0 << (44 - 32));

                            } else if (lxbar == 23) {
                                mask = ~(1 << (45 - 32));
                                mask_a =(0 << (45 - 32));
                                mask_b =(1 << (45 - 32));
                            } else {
                                mask = ~(3 << (lxbar - 16) * 2);
                                mask_a =(1 << (lxbar - 16) * 2);
                                mask_b =(2 << (lxbar - 16) * 2);
                            }
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_a & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_b & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG9r(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG9r(unit, regval));
                            
                            /* crossover */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                        }
                        break;
                    case BCM_PORT_ABILITY_SFI_SCI:
                        /* if SFI_SCI sci0 is plane A and sci1 is plane B
                         * a 01 b 10 qe_type0_channel_mask and fr_config8/9 (ab 11) (ss only)
                         * disable crossover
                         */
                        SOC_IF_ERROR_RETURN(READ_PLANE_CROSSOVERr(unit, &regval));
                        regval &= ~(1 << lxbar);
                        SOC_IF_ERROR_RETURN(WRITE_PLANE_CROSSOVERr(unit, regval));

                        if (lxbar < 22) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s SFI_SCI must be lxbar(%d) 22 or 23\n"),
                                       FUNCTION_NAME(), lxbar));
                            rv = BCM_E_UNAVAIL;
                            break;
                        } else {
                           if (lxbar == 22) {                                
                               mask = ~(1 << (44 - 32));
                               mask_a =(1 << (44 - 32));
                               mask_b =(0 << (44 - 32));

                            } else if (lxbar == 23) {
                               mask = ~(1 << (45 - 32));
                               mask_a =(0 << (45 - 32));
                               mask_b =(1 << (45 - 32));
                            }
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_a & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_b & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG9r(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG9r(unit, regval));

                            /* disable crossover */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                            regval &= ~((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                        }

                        break;
                    default:
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unsupported ability(%d) to reconfigure for ss node type\n"),
                                   FUNCTION_NAME(), ability));
                        rv = BCM_E_UNAVAIL;
                        break;
                }
                break;


            case SIRIUS_NODE_TYPE_QE2K:

                if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s lxbar1(%d) not valid for ss node type\n"),
                               FUNCTION_NAME(), lxbar1));
                    return BCM_E_INTERNAL;
                }

                /* assume SFI */
                /* a 01 b 10 qe_type1_channel_mask and fr_config10/11 ab (01) (qe2k) */
                if (lxbar < 16) {
                    mask  = ~(3 << lxbar * 2);
                    mask_a = (1 << lxbar * 2);
                    /* plane crossover on receive */
                    mask_b = (2 << lxbar * 2);
                    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A1_LOr(unit, &regval));
                    regval &= (mask);
                    regval |= mask_a;
                    SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A1_LOr(unit, regval));
                    
                    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B1_LOr(unit, &regval));
                    regval &= (mask);
                    regval |= mask_b;
                    SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B1_LOr(unit, regval));
 
                    mask = (3 << lxbar * 2);
                    SOC_IF_ERROR_RETURN(READ_FR_CONFIG10r(unit, &regval));
                    regval |= (mask);
                    SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG10r(unit, regval));
                    
                } else {
                    if (lxbar == 22) {                                
                        mask = ~(1 << (44 - 32));
                        mask_a =(1 << (44 - 32));
                        mask_b =(0 << (44 - 32));
                        
                    } else if (lxbar == 23) {
                        mask = ~(1 << (45 - 32));
                        mask_a =(0 << (45 - 32));
                        mask_b =(1 << (45 - 32));
                    } else {
                        /* Need to enable crossover for traffic on non-SFI_SCI links */
                        SOC_IF_ERROR_RETURN(READ_PLANE_CROSSOVERr(unit, &regval));
                        regval |= (1 << lxbar);
                        SOC_IF_ERROR_RETURN(WRITE_PLANE_CROSSOVERr(unit, regval));

                        mask = ~(3 << (lxbar - 16) * 2);
                        mask_a =(1 << (lxbar - 16) * 2);
                        mask_b =(2 << (lxbar - 16) * 2);

                        /* enable crossover for traffic on non-SFI_SCI links */
                        SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));
                        regval |= ~mask;
                        SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                    }

                    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A1_HIr(unit, &regval));
                    regval &= (mask) & 0x3fff;
                    regval |= mask_a & 0x3fff;
                    SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A1_HIr(unit, regval));

                    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B1_HIr(unit, &regval));
                    regval &= (mask) & 0x3fff;
                    regval |= mask_b & 0x3fff;
                    SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B1_HIr(unit, regval));

		    mask =(3 << (lxbar - 16) * 2);
                    SOC_IF_ERROR_RETURN(READ_FR_CONFIG11r(unit, &regval));
                    regval |= (mask) & 0x3fff;
                    SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG11r(unit, regval));
                }

                break;

            case SIRIUS_NODE_TYPE_HYBRID:
                /* Need to enable crossover for traffic SFI in SFI_SCI is always local */
                SOC_IF_ERROR_RETURN(READ_PLANE_CROSSOVERr(unit, &regval));
                regval |= (1 << lxbar);
                SOC_IF_ERROR_RETURN(WRITE_PLANE_CROSSOVERr(unit, regval));

                switch (ability) {
                    case BCM_PORT_ABILITY_DUAL_SFI_LOCAL:

                        if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s lxbar1(%d) not valid for ss node type\n"),
                                       FUNCTION_NAME(), lxbar1));
                            return BCM_E_INTERNAL;
                        }
                        /* 
                         * a 01 b 10 set up associated qe_type2_channel_mask and fr_config12/13 (ab 11) (ss hybrid) 
                         */
                        if (lxbar < 16) {
                            mask  = ~(3 << lxbar * 2);
                            mask_a = (1 << lxbar * 2);
                            mask_b = (2 << lxbar * 2);
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &regval));
                            regval &= (mask);
                            regval |= mask_a;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, regval));

                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
                            regval &= (mask);
                            regval |= mask_b;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG12r(unit, &regval));
                            regval &= (mask);
                            regval |= (mask_a | mask_b);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG12r(unit, regval));

                           /* enable crossover */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
                            regval &= (mask);
                            regval |= (mask_a | mask_b);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));


                        } else {
                            if (lxbar == 22) {                                
                                mask = ~(1 << (44 - 32));
                                mask_a =(1 << (44 - 32));
                                mask_b =(0 << (44 - 32));
                                
                            } else if (lxbar == 23) {
                                mask = ~(1 << (45 - 32));
                                mask_a =(1 << (45 - 32));
                                mask_b =(0 << (45 - 32));
                            } else {
                                mask = ~(3 << (lxbar - 16) * 2);
                                mask_a =(1 << (lxbar - 16) * 2);
                                mask_b =(2 << (lxbar - 16) * 2);
                            }

                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_a & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_b & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

                            /* enable crossover SFI_SCI traffic is always local */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                        }
                        break;


                    case BCM_PORT_ABILITY_DUAL_SFI:

                        if (!BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                            /*   a 11 b 00 qe_type2_channel_mask and fr_config12/13 (ss hybrid) (ab 11)
                             */
                            if (lxbar < 16) {
                                mask  = ~(3 << lxbar * 2);
                                mask_a = (3 << lxbar * 2);
                                mask_b = (0 << lxbar * 2);
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &regval));
                                regval &= (mask);
                                regval |= mask_a;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
                                regval &= (mask);
                                regval |= mask_b;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, regval));
                            
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG12r(unit, &regval));
                                regval &= (mask);
                                regval |= (mask_a | mask_b);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG12r(unit, regval));

                                /* enable crossover */
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
                                regval &= (mask);
                                regval |= (mask_a | mask_b);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));
                                
                            } else {
                                if (lxbar == 22) {                                
                                    mask = ~(1 << (44 - 32));
                                    mask_a =(1 << (44 - 32));
                                    mask_b =(0 << (44 - 32));
                                
                                } else if (lxbar == 23) {
                                    mask = ~(1 << (45 - 32));
                                    mask_a =(1 << (45 - 32));
                                    mask_b =(0 << (45 - 32));
                                } else {
                                    mask = ~(3 << (lxbar - 16) * 2);
                                    mask_a =(3 << (lxbar - 16) * 2);
                                    mask_b =(0 << (lxbar - 16) * 2);
                                }

                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= mask_a & 0x3fff;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= mask_b & 0x3fff;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= ((mask_a | mask_b) & 0x3fff);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

                                /* enable crossover */
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                                regval |= ((mask_a | mask_b) & 0x3fff);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                                
                            }
                        } else {
                            /*   else lxbar a 01 b 10 lxbar1 a1 01 b1 10  fr config12/13 (ab 11) (ab1 11)
                             */
                            if (lxbar < 16) {
                                mask  = ~(3 << lxbar * 2);
                                mask_a = (1 << lxbar * 2);
                                mask_b = (0 << lxbar * 2);
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &regval));
                                regval &= (mask);
                                regval |= mask_a;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
                                regval &= (mask);
                                regval |= mask_b;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG12r(unit, &regval));
                                regval &= (mask);
                                mask_b = (2 << lxbar * 2);
                                regval |= (mask_a | mask_b);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG12r(unit, regval));

                                /* enable crossover, both channels in fr */
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
                                regval &= (mask);
                                regval |= ~(mask);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));
                                
                            } else {
                                if (lxbar == 22) {                                
                                    mask = ~(1 << (44 - 32));
                                    mask_a =(1 << (44 - 32));
                                    mask_b =(0 << (44 - 32));
                                    
                                } else if (lxbar == 23) {
                                    mask = ~(1 << (45 - 32));
                                    mask_a =(1 << (45 - 32));
                                    mask_b =(0 << (45 - 32));
                                } else {
                                    mask = ~(3 << (lxbar - 16) * 2);
                                    mask_a =(1 << (lxbar - 16) * 2);
                                    mask_b =(0 << (lxbar - 16) * 2);
                                }
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= mask_a & 0x3fff;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= mask_b & 0x3fff;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                mask_b = (2 << (lxbar - 16) * 2);
                                regval |= ((mask_a | mask_b) & 0x3fff);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

                                /* enable crossover on both */
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                                regval |= ~mask;
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
				
                            }

                            if (lxbar1 < 16) {
                                mask  = ~(3 << lxbar1 * 2);
                                mask_a = (1 << lxbar1 * 2);
                                mask_b = (0 << lxbar1 * 2);
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &regval));
                                regval &= (mask);
                                regval |= mask_a;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
                                regval &= (mask);
                                regval |= mask_b;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG12r(unit, &regval));
                                regval &= (mask);
                                mask_b = (2 << lxbar1 * 2);
                                regval |= (mask_a | mask_b);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG12r(unit, regval));

                                /* enable crossover, both channels in fr */
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
                                regval &= (mask);
                                regval |= ~(mask);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));

                                
                            } else {
                                if (lxbar1 == 22) {
                                    mask = ~(1 << (44 - 32));
                                    mask_a =(1 << (44 - 32));
                                    mask_b =(0 << (44 - 32));                      
                                    
                                } else if (lxbar1 == 23) {
                                    mask = ~(1 << (45 - 32));
                                    mask_a =(1 << (45 - 32));
                                    mask_b =(0 << (45 - 32));
                                } else {
                                    mask = ~(3 << (lxbar1 - 16) * 2);
                                    mask_a =(1 << (lxbar1 - 16) * 2);
                                    mask_b =(0 << (lxbar1 - 16) * 2);
                                }
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= mask_a & 0x3fff;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                regval |= mask_b & 0x3fff;
                                SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));
                                
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
                                regval &= (mask) & 0x3fff;
                                mask_b =(2 << (lxbar1 - 16) * 2);
                                regval |= ((mask_a | mask_b) & 0x3fff);
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

                                /* enable crossover on both */
                                SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                                regval |= ~mask;
                                SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));

                            }
                        }
                        break;
                    case BCM_PORT_ABILITY_SFI_LOOPBACK:

                        if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s lxbar1(%d) not valid for ss node type\n"),
                                       FUNCTION_NAME(), lxbar1));
                            return BCM_E_INTERNAL;
                        }
                        /* 
                         * a 00 b 11 set up associated qe_type2_channel_mask and fr_config12/13 (ss hybrid) 
                         */
                        if (lxbar < 16) {
                            mask  = ~(3 << lxbar * 2);
                            mask_a = (0 << lxbar * 2);
                            mask_b = (3 << lxbar * 2);
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &regval));
                            regval &= (mask);
                            regval |= mask_a;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, regval));

                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
                            regval &= (mask);
                            regval |= mask_b;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG12r(unit, &regval));
                            regval &= (mask);
                            regval |= (mask_a | mask_b);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG12r(unit, regval));

                            /* enable crossover, both channels in fr */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG16r(unit, &regval));	    
                            regval &= (mask);
                            regval |= ~(mask);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG16r(unit, regval));

			    
                        } else {
                            if (lxbar == 22) {                                
                                mask = ~(1 << (44 - 32));
                                mask_a =(0 << (44 - 32));
                                mask_b =(1 << (44 - 32));
                                
                            } else if (lxbar == 23) {
                                mask = ~(1 << (45 - 32));
                                mask_a =(0 << (45 - 32));
                                mask_b =(1 << (45 - 32));
                            } else {
                                mask = ~(3 << (lxbar - 16) * 2);
                                mask_a =(0 << (lxbar - 16) * 2);
                                mask_b =(3 << (lxbar - 16) * 2);
                            }
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_a & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_b & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            mask_b = (3 << (lxbar - 16) * 2);
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

                            /* enable crossover on both */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                            regval |= ~mask;
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                        }
                        break;
                    case BCM_PORT_ABILITY_SFI_SCI:
                        /* if SFI_SCI sci0 is plane A and sci1 is plane B
                         * a 01 b 10 qe_type0_channel_mask and fr_config8/9 (ab 11) (ss only)
                         * disable crossover
                         */
                        SOC_IF_ERROR_RETURN(READ_PLANE_CROSSOVERr(unit, &regval));
                        regval &= ~(1 << lxbar);
                        SOC_IF_ERROR_RETURN(WRITE_PLANE_CROSSOVERr(unit, regval));

                        if (lxbar < 22) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s SFI_SCI must be lxbar(%d) 22 or 23\n"),
                                       FUNCTION_NAME(), lxbar));
                            rv = BCM_E_UNAVAIL;
                            break;
                        } else {
                           if (lxbar == 22) {                                
                               mask = ~(1 << (44 - 32));
                               mask_a =(0 << (44 - 32));
                               mask_b =(1 << (44 - 32));

                            } else if (lxbar == 23) {
                               mask = ~(1 << (45 - 32));
                               mask_a =(0 << (45 - 32));
                               mask_b =(1 << (45 - 32));
                            }
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_a & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= mask_b & 0x3fff;
                            SOC_IF_ERROR_RETURN(WRITE_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, regval));
                            
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG13r(unit, &regval));
                            regval &= (mask) & 0x3fff;
                            regval |= ((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG13r(unit, regval));

                            /* disable crossover */
                            SOC_IF_ERROR_RETURN(READ_FR_CONFIG17r(unit, &regval));	    
                            regval &= ~((mask_a | mask_b) & 0x3fff);
                            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG17r(unit, regval));
                        }

                        break;
                        
                    default:
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s ability(%d) unsupported for mapping set\n"),
                                   FUNCTION_NAME(), ability));
                        rv = BCM_E_UNAVAIL;
                        break;
                }
                
                break;
            default:
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s node_type(%d) unsupported\n"),
                           FUNCTION_NAME(), node_type));
                rv = BCM_E_UNAVAIL;
                break;
        }
    }
    return rv;
}

/*
 * Enable/disable SFI loopback for SFI pchan on
 * associated port - loopback required for recycling.
 */
static int
_bcm_sirius_fabric_pchan_loopback_set(int unit, 
                                      int pchan, 
                                      int loopback_en)
{
    int rv = BCM_E_NONE;
    uint32 regval = 0;
    uint32 index;
    uint32 regAddr;

    if (pchan > SB_FAB_DEVICE_MAX_NODE_PHYSICAL_DATA_CHANNELS) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s: pchan %d out of range (0-45)\n"),
                   FUNCTION_NAME(), pchan));
        return BCM_E_PARAM;
    }

    /******************* 
     * Loopback enable 
     *******************/
    /* mapping:
     *  sfi_channel 0-19 mapped to SC_TOP_SFI_PORT_CONFIG0 0-19
     *  sfi_channel 20-43 mapped to SFI_PORT_CONFIG0 0-23
     *  sfi_channel 44-45 mapped to SC_TOP_SFI_PORT_CONFIG0 20-21
     */
        
    if (pchan < 20) {
        index = pchan;
        regAddr = SC_TOP_SFI_PORT_CONFIG0r;
    } else if (pchan < 44) {
        index = (pchan) - 20;
        regAddr = SFI_PORT_CONFIG0r;
    } else {
        index = (pchan) - 24;
        regAddr = SC_TOP_SFI_PORT_CONFIG0r;
    }
    
    SOC_IF_ERROR_RETURN(soc_reg32_read(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                       &regval));
    
    if (loopback_en == TRUE) {
        
        soc_reg_field_set(unit, regAddr, &regval, LOOPBACK_ENf, 1);
        
    } else {
        soc_reg_field_set(unit, regAddr, &regval, LOOPBACK_ENf, 0);
    }
	
    SOC_IF_ERROR_RETURN(soc_reg32_write(unit, soc_reg_addr(unit, regAddr, REG_PORT_ANY, index),
                                        regval));
    return rv;
}

#define BCM_SIRIUS_DELAY_ON_FORCE_NULL_EFFECTIVE_IN_USEC 50
static int
_bcm_sirius_fabric_force_null_ts(int unit, int enable)
{
    int rv = BCM_E_NONE;
    int node;

    for (node=0; node< SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[7]; node++) {
        if (enable) { /* force null */
            SOC_IF_ERROR_RETURN(soc_sirius_ts_node_creditor_config(unit, 7, node, 
                                                                   SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                                                   SOC_SIRIUS_API_PARAM_NO_CHANGE, 
                                                                   SIRIUS_TS_NODE_PROFILE_TEMPLATE_DISABLED));

        } else {
            SOC_IF_ERROR_RETURN(soc_sirius_ts_node_creditor_config(unit, 7, node, 
                                                                   SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                                                   SOC_SIRIUS_API_PARAM_NO_CHANGE, 
                                                                   SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT));
        }
    }

    /* Delay after forcing null grants */
    sal_udelay(BCM_SIRIUS_DELAY_ON_FORCE_NULL_EFFECTIVE_IN_USEC);



    return rv;
}

/* Clears or resets the global channel masks
 * for the given logical crossbar to enable or disable the lxbar.
 * the reset of the mask is taken from the "or" of the node_type specific
 * channel mask registers
 */ 
static int
bcm_sirius_fabric_global_lchan_set(int unit,
                                   int lxbar, 
                                   int enable)
{
    uint32 regval = 0, mask = 0;
    uint32 channel_mask_a0_hi;
    uint32 channel_mask_a0_lo;
    uint32 channel_mask_a1_hi;
    uint32 channel_mask_a1_lo;
    uint32 channel_mask_a2_hi;
    uint32 channel_mask_a2_lo;
    uint32 channel_mask_b0_hi;
    uint32 channel_mask_b0_lo;
    uint32 channel_mask_b1_hi;
    uint32 channel_mask_b1_lo;
    uint32 channel_mask_b2_hi;
    uint32 channel_mask_b2_lo;
    uint32 channel_mask_a_hi;
    uint32 channel_mask_a_lo;
    uint32 channel_mask_b_hi;
    uint32 channel_mask_b_lo;

    if (lxbar >= SB_FAB_DEVICE_MAX_NODE_CROSSBARS) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s: lxbar(%d) out of range (0-23)\n"),
                   FUNCTION_NAME(), lxbar));
        return BCM_E_INTERNAL;
    }
    
    if (lxbar < 16) {
        mask = (3 << lxbar * 2);
    } else if (lxbar == 22) {
        mask = (1 << (44 - 32)) & 0x3fff;
    } else if (lxbar == 23) {
        mask = (1 << (45 - 32)) & 0x3fff;
    } else {
        mask = (3 << (lxbar-16) * 2) & 0x3fff;
    }

#ifdef DEBUG_CROSSBAR_MAP 
    LOG_CLI((BSL_META_U(unit,
                        "%s: lxbar(%d):%s\n"), FUNCTION_NAME(), 
             lxbar, enable?"ENABLE":"DISABLE"));
#endif

    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_HIr(unit, &channel_mask_a0_hi));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A0_LOr(unit, &channel_mask_a0_lo));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A1_HIr(unit, &channel_mask_a1_hi));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A1_LOr(unit, &channel_mask_a1_lo));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_HIr(unit, &channel_mask_a2_hi));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_A2_LOr(unit, &channel_mask_a2_lo));

    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_HIr(unit, &channel_mask_b0_hi));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B0_LOr(unit, &channel_mask_b0_lo));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B1_HIr(unit, &channel_mask_b1_hi));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B1_LOr(unit, &channel_mask_b1_lo));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &channel_mask_b2_hi));
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &channel_mask_b2_lo));

    channel_mask_a_hi = channel_mask_a0_hi | channel_mask_a1_hi | channel_mask_a2_hi;
    channel_mask_a_lo = channel_mask_a0_lo | channel_mask_a1_lo | channel_mask_a2_lo;
    channel_mask_b_hi = channel_mask_b0_hi | channel_mask_b1_hi | channel_mask_b2_hi;
    channel_mask_b_lo = channel_mask_b0_lo | channel_mask_b1_lo | channel_mask_b2_lo;

    if (lxbar < 16) {
       channel_mask_a_lo &= (3 << lxbar * 2);
       channel_mask_b_lo &= (3 << lxbar * 2);
    } else if (lxbar == 22) {
        channel_mask_a_hi &= (1 << (44 - 32)) & 0x3fff;
        channel_mask_b_hi &= (1 << (44 - 32)) & 0x3fff;
    } else if (lxbar == 23) {
        channel_mask_a_hi &= (1 << (45 - 32)) & 0x3fff;
        channel_mask_b_hi &= (1 << (45 - 32)) & 0x3fff;
    } else {
        channel_mask_a_hi &= (3 << (lxbar-16) * 2) & 0x3fff;
        channel_mask_b_hi &= (3 << (lxbar-16) * 2) & 0x3fff;
    }

    /******************************** 
     * Global Channel Mask Setting 
     ********************************/
    if (lxbar >= 16) {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_A_HIr(unit, &regval));
        if (enable == TRUE) {
            regval |= channel_mask_a_hi & mask;
        } else {
            regval &= ~(channel_mask_a_hi & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_CHANNEL_MASK_A_HIr(unit, regval));

        SOC_IF_ERROR_RETURN(READ_FR_CONFIG4r(unit, &regval));
        if (enable == TRUE) {
            regval |= channel_mask_a_hi & mask;
        } else {
            regval &= ~(channel_mask_a_hi & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG4r(unit, regval));

	} else {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_A_LOr(unit, &regval));
       if (enable == TRUE) {
            regval |= channel_mask_a_lo & mask;
        } else {
            regval &= ~(channel_mask_a_lo & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_CHANNEL_MASK_A_LOr(unit, regval));

        SOC_IF_ERROR_RETURN(READ_FR_CONFIG3r(unit, &regval));
       if (enable == TRUE) {
            regval |= channel_mask_a_lo & mask;
        } else {
            regval &= ~(channel_mask_a_lo & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG3r(unit, regval));
	}

    if (lxbar >= 16) {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_B_HIr(unit, &regval));
       if (enable == TRUE) {
            regval |= channel_mask_b_hi & mask;
        } else {
            regval &= ~(channel_mask_b_hi & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_CHANNEL_MASK_B_HIr(unit, regval));

        SOC_IF_ERROR_RETURN(READ_FR_CONFIG6r(unit, &regval));
       if (enable == TRUE) {
            regval |= channel_mask_b_hi & mask;
        } else {
            regval &= ~(channel_mask_b_hi & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG6r(unit, regval));

    } else {
        SOC_IF_ERROR_RETURN(READ_CHANNEL_MASK_B_LOr(unit, &regval));
       if (enable == TRUE) {
            regval |= channel_mask_b_lo & mask;
        } else {
            regval &= ~(channel_mask_b_lo & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_CHANNEL_MASK_B_LOr(unit, regval));

        SOC_IF_ERROR_RETURN(READ_FR_CONFIG5r(unit, &regval));
       if (enable == TRUE) {
            regval |= channel_mask_b_lo & mask;
        } else {
            regval &= ~(channel_mask_b_lo & mask);
        }
        SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG5r(unit, regval));
    }

    /******************* 
     * Force link enable
     *******************/
    if (soc_feature(unit, soc_feature_hybrid)) { 
        if (enable == TRUE) {
            
            SOC_IF_ERROR_RETURN(READ_FORCE_LINK_ENABLE_Br(unit, &regval));
            
            if ((lxbar < 16) && (channel_mask_b2_lo & mask)) {
                regval |= 1 << lxbar;
            } else if ((lxbar >= 16) && (channel_mask_b2_hi & mask)) {
                regval |= 1 << lxbar;
            }
            SOC_IF_ERROR_RETURN(WRITE_FORCE_LINK_ENABLE_Br(unit, regval));
            
            
            SOC_IF_ERROR_RETURN(READ_FR_CONFIG2r(unit, &regval));
            
            if ((lxbar < 16) && (channel_mask_b2_lo & mask)) {
                regval |= 1 << lxbar;
            } else if ((lxbar >= 16) && (channel_mask_b2_hi & mask)) {
                regval |= 1 << lxbar;
            }
            
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG2r(unit, regval));
            
        } else {
            SOC_IF_ERROR_RETURN(READ_FORCE_LINK_ENABLE_Br(unit, &regval));
            regval &= ~(1 << lxbar);
            SOC_IF_ERROR_RETURN(WRITE_FORCE_LINK_ENABLE_Br(unit, regval));
        
            
            SOC_IF_ERROR_RETURN(READ_FR_CONFIG2r(unit, &regval));
            regval &= ~(1 << lxbar);
            SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG2r(unit, regval));
            
        }
    }
    
    return BCM_E_NONE;
}

/*
 * Returns the count of channels used for local (recyling grant)
 * Used for Sirius hybrid node.
 */
static int
_bcm_sirius_fabric_xbar_local_lchan_cnt_get(int unit, 
                                            int *local_channel_count)
{
    int rv = BCM_E_NONE;
    uint32 regval;
    volatile uint32 lchannel_b_mask_hi, lchannel_b_mask_lo;
    int idx, cnt;

    cnt = 0;
    
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_HIr(unit, &regval));
    lchannel_b_mask_hi = regval;
    
    SOC_IF_ERROR_RETURN(READ_QE_TYPE_CHANNEL_MASK_B2_LOr(unit, &regval));
    lchannel_b_mask_lo = regval;
    
    for (idx=0; idx<SB_FAB_DEVICE_MAX_NODE_LOGICAL_DATA_CHANNELS; idx++) {
        
        if (idx > 31) {
            if (lchannel_b_mask_hi & (1 << (idx - 32))) {
                cnt++;
            }
        } else {
            if (lchannel_b_mask_lo & (1 << idx)) {
                cnt++;
            }
        }
    }
    *local_channel_count = cnt;
    
    return rv;
}
/* 
 * Given the lxbar, return the associated
 * channels. 
 */

static int
_bcm_sirius_fabric_lchan_get(int unit, 
                             int lxbar, 
                             int *lchan_odd, 
                             int *lchan_even)
{
    int rv = BCM_E_NONE;
    if (lxbar > 23) {
        return BCM_E_INTERNAL;
    }

    if (lxbar < 22) {
        *lchan_odd = lxbar*2+1;
        *lchan_even = lxbar*2;

    } else if (lxbar == 22) { 
        *lchan_odd = -1; /* not mapped */
        *lchan_even = 44;

    } else if (lxbar == 23) {
        *lchan_odd = 45;
        *lchan_even = -1;  /* not mapped */
    }
    return rv;
}

/* 
 * Given the sfi_port, return the pchans 
*/
static int
_bcm_sirius_fabric_pchan_get(int unit, 
                             int sfi_port, 
                             int *pchan_odd, 
                             int *pchan_even)
{
    int rv = BCM_E_NONE;

    if (sfi_port > 23) {
        return BCM_E_INTERNAL;
    }

    if (sfi_port < 22) {
        *pchan_odd = sfi_port*2+1;
        *pchan_even = sfi_port*2;

    } else if (sfi_port == 22) {
        *pchan_odd = 44;
        *pchan_even = -1; /* SCI0 */
    } else if (sfi_port == 23) {
        *pchan_odd = 45;
        *pchan_even = -1; /* SCI1 */
    }
    return rv;
}


/* 
 * Perform any updates required to the logical to physical mapping
 * when the ability of a port changes 
 */
int
bcm_sirius_fabric_update_lchan_for_port_ability(int unit, 
                                                int sfi_port, 
                                                int new_ability, 
                                                int old_ability)
{
    int rv = BCM_E_NONE;
    int lxbar, lxbar1;
    int used_in_a, used_in_b;
    int lchan_odd, lchan_even, pchan_odd, pchan_even;
 
#ifdef DEBUG_CROSSBAR_MAP 
    LOG_CLI((BSL_META_U(unit,
                        "%s: sfi_port(%d) new ability(%d) old ability(%d)\n"), FUNCTION_NAME(), sfi_port, new_ability, old_ability));
#endif

    /* There can be up to 2 logical crossbars associated with an sfi port, check for both */
    BCM_IF_ERROR_RETURN(bcm_sbx_fabric_xbar_for_sfi_get(unit, SOC_SBX_CONTROL(unit)->node_id, sfi_port, &lxbar, &lxbar1));

    /* If we had something mapped to this lxbar */
    if (BCM_SBX_CROSSBAR_VALID(lxbar)) {

        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_global_lchan_get(unit, lxbar, &used_in_a, &used_in_b));

        if ((used_in_a == TRUE) || (used_in_b == TRUE)) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "WARNING! disable sfi port(%d) before changing ability to avoid errors due to channel mask mismatch,\n"),
                       sfi_port));
        }

        if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {

            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_global_lchan_get(unit, lxbar1, &used_in_a, &used_in_b));
            
            if ((used_in_a == TRUE) || (used_in_b == TRUE)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "WARNING! disable sfi port(%d) before changing ability to avoid errors due to channel mask mismatch,\n"),
                           sfi_port));
            }
        }

        /* Only update if sirius hybrid mode */
        if (!soc_feature(unit, soc_feature_hybrid)) {
            return BCM_E_NONE;
        } 

       /* If we are reconfiguring a B/B local channel to something else, need to free the allocated lxbar */
        if ((old_ability == BCM_PORT_ABILITY_SFI_LOOPBACK) && 
            (new_ability != BCM_PORT_ABILITY_SFI_LOOPBACK)) {
            
            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_free(unit, lxbar));
            
            
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, lxbar, &lchan_odd, &lchan_even));
        
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_even));
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_odd));
            
            /* Unmap logical and physical channels
             */
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, lxbar, SB_FAB_DEVICE_INVALID_CROSSBAR, 
                                                                        0, 0, TRUE /* UNMAP */));
            
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));
            
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE));
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE));
        }
        
    }

    /* 
     * if ability is BB, allocate logical crossbar from the high end (23-0).  Return error if none available.
     */
    if ((new_ability == BCM_PORT_ABILITY_SFI_LOOPBACK) && 
        (new_ability != old_ability)) {

        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_alloc(unit, &lxbar, FALSE));

        /* update state to reflect */
        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, lxbar, SOC_SBX_CONTROL(unit)->node_id, sfi_port));


        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, lxbar, &lchan_odd, &lchan_even));
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));

        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even /* even */));
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd /* odd */));
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, TRUE));
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, TRUE));

        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, lxbar, SB_FAB_DEVICE_INVALID_CROSSBAR, 
                                                                    SIRIUS_NODE_TYPE_HYBRID, new_ability, FALSE));
    } else if ((new_ability == BCM_PORT_ABILITY_SFI_SCI) && 
               ((sfi_port == 22) || (sfi_port == 23))) {
        
        /*  Ports 22 and 23 are sfi_sci ports - force allocation xbars 22/23 */ 
        lxbar = sfi_port;
        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_alloc(unit, &lxbar, TRUE));
        
        /* update state to reflect */
        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, lxbar, SOC_SBX_CONTROL(unit)->node_id, sfi_port));
        
        
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, lxbar, &lchan_odd, &lchan_even));
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));
        
        if (lxbar == 22) {
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_odd /* odd */));
        } else {
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd /* odd */));
        }
        
        if (soc_feature(unit, soc_feature_hybrid)) {

            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, TRUE));
        
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, lxbar, SB_FAB_DEVICE_INVALID_CROSSBAR, 
                                                                        SIRIUS_NODE_TYPE_HYBRID, new_ability, FALSE));
        } else {
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE));
        
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, lxbar, SB_FAB_DEVICE_INVALID_CROSSBAR, 
                                                                        SIRIUS_NODE_TYPE_FIC, new_ability, FALSE));
        }
    }

    return rv;
}

/*
 * This function updates the global channel mask upon port enable/disable
 * If there are local B channels (hybrid mode), the local burst size
 * is also updated.  Null grants are forced around the operation in the TS
 * block to prevent any misconfiguration.
 */
int
bcm_sirius_fabric_update_lchan_for_port_enable_set(int unit, 
                                                   int sfi_port, 
                                                   int enable)
{
    int rv = BCM_E_NONE;
    int lxbar, lxbar1;
    int used_in_a, used_in_b = FALSE, used_in_b1 = FALSE;
    int local_lchan_cnt;
    int burst_size;
    uint32 regval;
    
#ifdef DEBUG_CROSSBAR_MAP 
    LOG_CLI((BSL_META_U(unit,
                        "%s: sfi_port(%d) (%s)\n"), FUNCTION_NAME(), sfi_port, enable?"ENABLE":"DISABLE"));
#endif
    
    BCM_IF_ERROR_RETURN(bcm_sbx_fabric_xbar_for_sfi_get(unit, SOC_SBX_CONTROL(unit)->node_id, sfi_port, &lxbar, &lxbar1)); 
    
    if (!BCM_SBX_CROSSBAR_VALID(lxbar)) {
        /* Sfi port not mapped to crossbar, nothing to do... */
#ifdef DEBUG_CROSSBAR_MAP_VERBOSE
        LOG_CLI((BSL_META_U(unit,
                            "sfi_port(%d):crossbar not valid\n"), sfi_port));
#endif
        return rv;
    }
    
    if (soc_feature(unit, soc_feature_hybrid)) {
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_force_null_ts(unit, TRUE));
    }
    
    /* Disable or enable lxbar(s) associated with sfi_port in global channel mask */
    BCM_IF_ERROR_RETURN(bcm_sirius_fabric_global_lchan_set(unit, lxbar, enable));
    
    if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
        BCM_IF_ERROR_RETURN(bcm_sirius_fabric_global_lchan_set(unit, lxbar1, enable));
    }
    
    if (soc_feature(unit, soc_feature_hybrid)) {
        
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_global_lchan_get(unit, lxbar, &used_in_a, &used_in_b));
        if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_global_lchan_get(unit, lxbar1, &used_in_a, &used_in_b1));
        }
        
        if (used_in_b || used_in_b1) {
            /* update local burst size */
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_xbar_local_lchan_cnt_get(unit, &local_lchan_cnt));
            
            rv = soc_sirius_ts_burst_size_bytes_get(unit, 0 /* ignore els */, 0 /* ignore node_type */,
                                                    local_lchan_cnt, &burst_size);
            
#ifdef DEBUG_CROSSBAR_MAP_VERBOSE 
            LOG_CLI((BSL_META_U(unit,
                                "lchan_cnt(%d) burst_size(%d) for local grant\n"), local_lchan_cnt, burst_size));
#endif
            
            soc_reg_field_set(unit, GG_LOCAL_BSr, &regval, LOCAL_BURST_SIZEf, burst_size/16);
            SOC_IF_ERROR_RETURN(WRITE_GG_LOCAL_BSr(unit, regval));
        }
        
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_force_null_ts(unit, FALSE));
        
    }
    return rv;
}

int
bcm_sirius_fabric_crossbar_mapping_set(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t port)
{
    int rv = BCM_E_NONE;
    int node;
    int sfi_port, old_sfi_port;
    int my_ability, old_ability;
    int used_in_a, used_in_b;
    int lxbar, lxbar1, new_lxbar;
    int my_node = SOC_SBX_CONTROL(unit)->node_id;
    int node_type = SIRIUS_NODE_TYPE_FIC;
    bcm_module_protocol_t protocol;
    int pchan_odd, pchan_even, lchan_odd, lchan_even;
    int old_lxbar, old_lxbar1;
    
    /* Port passed in from common sbx has been adjusted to base sfi port */
    sfi_port = port;

    node = BCM_STK_MOD_TO_NODE(modid);    
    
    BCM_IF_ERROR_RETURN(bcm_sbx_port_node_ability_get(unit, sfi_port, my_node, &my_ability));

    if (my_ability == BCM_PORT_ABILITY_SFI_LOOPBACK) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "ERROR! mapping set invalid for BCM_PORT_ABILITY_SFI_LOOPBACK port(%d)\n"),
                   sfi_port));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(bcm_sbx_stk_module_protocol_get(unit, BCM_STK_NODE_TO_MOD(node), &protocol));

        
    if ((protocol == bcmModuleProtocol1) || (protocol == bcmModuleProtocol2)) {
        node_type = SIRIUS_NODE_TYPE_QE2K;
    } else if ((protocol == bcmModuleProtocol3) || (protocol == bcmModuleProtocol5)) {
        node_type = SIRIUS_NODE_TYPE_FIC;
    } else if (protocol == bcmModuleProtocol4) {
        node_type = SIRIUS_NODE_TYPE_HYBRID;
    }

    /*********
     * UNMAP *
     *********/ 
    if (xbar == -1) {
        
        xbar = SB_FAB_DEVICE_INVALID_CROSSBAR;
        
        /* There can be up to 2 logical crossbars associated with an sfi port, check for both */
        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_xbar_for_sfi_get(unit, my_node, sfi_port, &old_lxbar, &old_lxbar1));
        
        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_global_lchan_get(unit, old_lxbar, &used_in_a, &used_in_b));
        
        if ((used_in_a == TRUE) || (used_in_b == TRUE)) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "WARNING! disable sfi port(%d) before unmapping to avoid traffic loss\n"),
                       sfi_port));
        }
        
        if (BCM_SBX_CROSSBAR_VALID(old_lxbar)) {        

            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, old_lxbar, old_lxbar1, 0, 0, TRUE /* UNMAP */));
            
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, old_lxbar, &lchan_odd, &lchan_even));
            
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_even));  
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_odd));

            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_free(unit, old_lxbar));

            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));
            
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE));
            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE));
            
            BCM_IF_ERROR_RETURN(bcm_sirius_fabric_unmap_link_status(unit, sfi_port));
            
        
            if (BCM_SBX_CROSSBAR_VALID(old_lxbar1)) {
                
                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, old_lxbar1, &lchan_odd, &lchan_even));
                
                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_even));  
                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_odd));
                
                BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_free(unit, old_lxbar1));
                
            } 
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "WARNING: unmapping: sfi_port(%d) is not mapped to a logical crossbar\n"),
                       sfi_port)); 
            return BCM_E_INTERNAL;
        } 
    } 
    /*********
     * MAP   *
     *********/ 
    else {

        if ((node == my_node) && (soc_feature(unit, soc_feature_hybrid))) {
                
            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_get(unit, xbar, my_node, &old_sfi_port));

            if (BCM_SBX_CROSSBAR_VALID(old_sfi_port)) {
                
                /* If the crossbar associated with the port was associated with a BB channel */
                BCM_IF_ERROR_RETURN(bcm_sbx_port_node_ability_get(unit, old_sfi_port, my_node, &old_ability));

                /* If the that used to be associated with the current crossbar was B/B LOOPBACK
                 * then we need to allocate a new xbar and free up the old one so that it can be
                 * used for this mapping set command (if possible).
                 */
                if (old_ability == BCM_PORT_ABILITY_SFI_LOOPBACK) {
                
                    /* alloc new xbar for bb
                     * since the xbar passed in was already allocated, we don't need to allocate it again */
                    rv = bcm_sbx_fabric_lxbar_alloc(unit, &new_lxbar, FALSE);
                    
                    if (rv) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "WARNING: no crossbar available for SFI_LOOPBACK port(%d)\n"),
                                   old_sfi_port));
                        return rv;
                    }

                    
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, xbar, &lchan_odd, &lchan_even));
                    
                    /* Unmap */
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_even));
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_odd));
                    
                    /* Unmap the old xbar */
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, xbar, SB_FAB_DEVICE_INVALID_CROSSBAR,
                                                                                0, 0, TRUE /* UNMAP */));
                    
                    /* Free xbar */
                    BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_free(unit, xbar));
                    
                    /* Map new BB */
                    BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, new_lxbar, my_node, old_sfi_port));
                    
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, new_lxbar, &lchan_odd, &lchan_even));
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, old_sfi_port, &pchan_odd, &pchan_even));
                    
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even));
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd));
                    
                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, new_lxbar, SB_FAB_DEVICE_INVALID_CROSSBAR,
                                                                                node_type,
                                                                                BCM_PORT_ABILITY_SFI_LOOPBACK, FALSE));
                }
            }
        }
                
        switch (node_type) {
            case (SIRIUS_NODE_TYPE_FIC):
                switch (my_ability) {
                    case (BCM_PORT_ABILITY_DUAL_SFI):
                        if (node == my_node) {
                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_alloc(unit, &xbar, TRUE /* with_id */));

                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, xbar, my_node, sfi_port));

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, xbar, &lchan_odd, &lchan_even));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd));

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE));

                            BCM_IF_ERROR_RETURN(bcm_sirius_fabric_map_link_status(unit, xbar, sfi_port));

                        }
                
                        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, xbar, SB_FAB_DEVICE_INVALID_CROSSBAR,
                                                                                    node_type, my_ability, FALSE));
                        break;
                 
                case (BCM_PORT_ABILITY_SFI_SCI):

                    if (((xbar == 22) && (sfi_port == 22)) ||  
                        ((xbar == 23) && (sfi_port == 23))) {

                        if (node == my_node) {


                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_alloc(unit, &xbar, TRUE));

                            /* update state to reflect */
                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, xbar, my_node, sfi_port));


                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, xbar, &lchan_odd, &lchan_even));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));
                            
                            if (xbar == 22) {
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_odd /* odd */));
                            } else {
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd /* odd */));
                            }

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE));

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, xbar, SB_FAB_DEVICE_INVALID_CROSSBAR, 
                                                                                        node_type, my_ability, FALSE));

                            BCM_IF_ERROR_RETURN(bcm_sirius_fabric_map_link_status(unit, xbar, sfi_port));
                                

                        }
                    } else {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "logical crossbar must be 22 sci0 or 23 sci1 for SFI_SCI ports\n")));
                    }
                    break;
                    default:
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "Other abilities currently unsupported for this node type\n")));
                        break;
                }
                break;

            case (SIRIUS_NODE_TYPE_QE2K):
                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, xbar, SB_FAB_DEVICE_INVALID_CROSSBAR, 
                                                                            node_type, my_ability, FALSE));     
                break;
            case (SIRIUS_NODE_TYPE_HYBRID):
                switch (my_ability) {
                    /* A FIC/ B LOCAL */
                    case (BCM_PORT_ABILITY_DUAL_SFI_LOCAL):
                        if (node == my_node) {
                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_alloc(unit, &xbar, TRUE /* with_id */));

                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, xbar, my_node, sfi_port));

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, xbar, &lchan_odd, &lchan_even));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));


                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd));

                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE));
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, TRUE));

                            BCM_IF_ERROR_RETURN(bcm_sirius_fabric_map_link_status(unit, xbar, sfi_port));
                        }
                        
                        BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, xbar, SB_FAB_DEVICE_INVALID_CROSSBAR,
                                                                                    node_type, my_ability, FALSE));
                        break;
                    case (BCM_PORT_ABILITY_DUAL_SFI):

                        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_xbar_for_sfi_get(unit, my_node, sfi_port, &lxbar, &lxbar1));

                        if (BCM_SBX_CROSSBAR_VALID(lxbar1)) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "ERROR!: sfi port(%d) already set up with 2 lxbars(%d/%d), inconsistent state\n"), 
                                       sfi_port, lxbar, lxbar1));
                            rv = BCM_E_PARAM;
                            break;
                        }

                        if (node == my_node) {
                            BCM_IF_ERROR_RETURN(bcm_sbx_fabric_lxbar_alloc(unit, &xbar, TRUE /* with_id */));
                        }

                        /* Update state to reflect new xbar in use with this sfi port */
                        BCM_IF_ERROR_RETURN(bcm_sbx_fabric_sfi_for_xbar_set(unit, xbar, my_node, sfi_port));

                        /* Crossbar is already allocated for this sfi port */
                        if (BCM_SBX_CROSSBAR_VALID(lxbar)) {
                            /* Unmap */
                            _bcm_sirius_fabric_update_qe_type_lchan(unit, lxbar, SB_FAB_DEVICE_INVALID_CROSSBAR,
                                                                    node_type, my_ability, TRUE /* UNMAP */);
                            
                            /* Map new with both crossbars */
                            _bcm_sirius_fabric_update_qe_type_lchan(unit, lxbar, xbar,
                                                                    node_type, my_ability, FALSE /* REMAP */);
                            
                            if (node == my_node) {

                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, lxbar, &lchan_odd, &lchan_even));
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));

                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_even));
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_unmap_lchan_to_pchan(unit, lchan_odd));

                                BCM_IF_ERROR_RETURN(bcm_sirius_fabric_unmap_link_status(unit, sfi_port));

                                /* Map previous lxbar fic even to even pchan */
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even));

                                /* Map new lxbar to odd pchan */
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, xbar, &lchan_odd, &lchan_even));
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_odd));

                                /* coverity[check_return : FALSE] */
                                _bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE);
                                /* coverity[check_return : FALSE] */
                                _bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE);

                                BCM_IF_ERROR_RETURN(bcm_sirius_fabric_map_link_status(unit, lxbar, sfi_port));
                                BCM_IF_ERROR_RETURN(bcm_sirius_fabric_map_link_status(unit, xbar, sfi_port));

                            }
                        } else { /* Crossbar is first A/A allocate 1 lxbar */
                            
                            BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_update_qe_type_lchan(unit, xbar, 
                                                                                        SB_FAB_DEVICE_INVALID_CROSSBAR,
                                                                                        node_type, my_ability, 
                                                                                        FALSE /* REMAP */));
                            if (node == my_node) {

                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_lchan_get(unit, xbar, &lchan_odd, &lchan_even));
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_get(unit, sfi_port, &pchan_odd, &pchan_even));

                                if (xbar < 22) {
                                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even));
                                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_odd, pchan_odd));
                                } else  {
                                    BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_map_lchan_to_pchan(unit, lchan_even, pchan_even));
                                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                                              (BSL_META_U(unit,
                                                          "WARNING! A/A single crossbar configuration for lxbar(%d) not recommended\n"),
                                               xbar));
                                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                                              (BSL_META_U(unit,
                                                          "Recommend second crossbar associated with sfi port(%d)\n"),
                                               sfi_port));
                                    
                                } 
 
                                BCM_IF_ERROR_RETURN(bcm_sirius_fabric_map_link_status(unit, xbar, sfi_port));
                                
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_odd, FALSE));
                                BCM_IF_ERROR_RETURN(_bcm_sirius_fabric_pchan_loopback_set(unit, pchan_even, FALSE));
                            }
                        }
                        
                        break;
                    default:
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "Other abilities currently unsupported for this node type\n")));
                        rv = BCM_E_UNAVAIL;
                        break;
                        
                }
                break;
        }
    } /* MAP */
    return rv;
}

int
bcm_sirius_fabric_crossbar_mapping_get(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id,
                                       int xbar,
                                       bcm_port_t *port)
{
    int rv = BCM_E_NONE;

    uint32 shift = 0, uData = 0;
    uint32 mask = 0x1F;

    if (SOC_SBX_CONTROL(unit)->node_id != BCM_STK_MOD_TO_NODE(modid)) {
        /* return error if modid doesn't match, helps bcmx */
        return BCM_E_PARAM;
    }
    if (xbar < 0 || xbar >= SB_FAB_DEVICE_SIRIUS_SFI_LINKS) { 
        return BCM_E_PARAM;
    }

    shift = (xbar % 6) * 5;

    if ( xbar < 6 ) {
	SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP0r(unit, &uData));
    } else if ( xbar < 12 ) {
	SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP1r(unit, &uData));
    } else if ( xbar < 18 ) {
	SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP2r(unit, &uData));
    } else {
	SOC_IF_ERROR_RETURN(READ_SC_LINK_STATUS_REMAP3r(unit, &uData));
    }

    /* Enable remap logical crossbar to physical crossbar */
    *port = ((uData & (mask << shift)) >> shift);

    if (*port == 0x1f) {
	/* return -1 if the link remap is disabled */
	*port = -1;
    } else {
	*port += SOC_PORT_MIN(unit, sfi);
    }

    return rv;
}

int
bcm_sirius_fabric_crossbar_enable_set(int unit,
                                      uint64 xbars)
{
    int     rv = BCM_E_NONE;
    int     i;
    uint32  sfi_count;
    uint32  ts_size_ns;
    uint32  epoch_length_in_ns;
    uint32  clocks_per_epoch;
    uint32  reg_value;
    int     half_bus = FALSE;
    int     adjusted_demand_shift;
    uint32  adj_clocks_per_epoch;
    uint16 dev_id;
    uint8  rev_id;

    sfi_count = 0;
    for (i = 0; i < 64; i++) {
        /* Count number of enabled links */
        sfi_count += COMPILER_64_BITTEST(xbars, i);
    }

    /* NOTE: Hybrid mode not currently supported for Sirius */
    ts_size_ns = soc_sbx_fabric_get_timeslot_size(unit, sfi_count, half_bus,
                                                 soc_feature(unit, soc_feature_hybrid) );

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Unit:%d NumberSFILinks: %d TimeslotSize: %d(ns), EpochLengthInTimeslots: %d\n"),
              FUNCTION_NAME(), unit, sfi_count, ts_size_ns, SOC_SBX_CFG(unit)->epoch_length_in_timeslots));

    epoch_length_in_ns = SOC_SBX_CFG(unit)->epoch_length_in_timeslots * ts_size_ns;
    clocks_per_epoch =  epoch_length_in_ns / 1000;
    clocks_per_epoch =  clocks_per_epoch * SOC_SBX_CFG(unit)->uClockSpeedInMHz;

    adjusted_demand_shift = SOC_SBX_CFG(unit)->demand_scale;
    adj_clocks_per_epoch = clocks_per_epoch;
    bcm_sbx_adjust_demand_scale(unit, xbars,
                            FALSE, &adjusted_demand_shift, &adj_clocks_per_epoch);
    if (adj_clocks_per_epoch > SOC_SBX_CFG(unit)->uMaxClocksInEpoch) {
        adj_clocks_per_epoch = SOC_SBX_CFG(unit)->uMaxClocksInEpoch;

        /* QoS Guarantess not met */
        rv = BCM_E_INTERNAL;
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Unit:%d ClocksPerEpoch: %d(0x%x), AdjustedClocksPerEpoch: %d(0x%x)\n"),
              FUNCTION_NAME(), unit, clocks_per_epoch, clocks_per_epoch,
              adj_clocks_per_epoch, adj_clocks_per_epoch));

    /* configure clocks per epoch */
    SOC_IF_ERROR_RETURN(READ_RU_CONFIG1r(unit, &reg_value));
    soc_reg_field_set(unit, RU_CONFIG1r, &reg_value, CLOCKS_PER_EPOCHf, 
                      (adj_clocks_per_epoch & 0x7fffff));
    SOC_IF_ERROR_RETURN(WRITE_RU_CONFIG1r(unit, reg_value));

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Unit:%d DemandScale: %d\n"),
              FUNCTION_NAME(), unit, SOC_SBX_CFG(unit)->demand_scale));


    
    /*       scale direction, etc                                                */
    /* configure rate update scale */
    soc_cm_get_id(unit, &dev_id, &rev_id);
    if ((rev_id == BCM88230_A0_REV_ID) && (SOC_SBX_CFG(unit)->demand_scale > 7)) {
	rv = BCM_E_INTERNAL;
	return rv;
    }

    SOC_IF_ERROR_RETURN(READ_RU_CONFIG2r(unit, &reg_value));
    soc_reg_field_set(unit, RU_CONFIG2r, &reg_value, RATE_UPDATE_SCALE_DIRf, 0);
    soc_reg_field_set(unit, RU_CONFIG2r, &reg_value, RATE_UPDATE_SCALEf,
		      (SOC_SBX_CFG(unit)->demand_scale > 7)?7:SOC_SBX_CFG(unit)->demand_scale);
    SOC_IF_ERROR_RETURN(WRITE_RU_CONFIG2r(unit, reg_value));

    /* update demand scale */
    SOC_IF_ERROR_RETURN(READ_QMC_CONFIG0r(unit, &reg_value));
    if (rev_id == BCM88230_A0_REV_ID) {    
	soc_reg_field_set(unit, QMC_CONFIG0r, &reg_value, RATE_SHIFTf, SOC_SBX_CFG(unit)->demand_scale);
    } else {
	if (SOC_SBX_CFG(unit)->demand_scale > 7) {
	    soc_reg_field_set(unit, QMC_CONFIG0r, &reg_value, RATE_SHIFTf,
			      SOC_SBX_CFG(unit)->demand_scale - 7);
#ifdef  RATE_SHIFT_MSBf
	    soc_reg_field_set(unit, QMC_CONFIG0r, &reg_value, RATE_SHIFT_MSBf, 1);
#endif /*  RATE_SHIFT_MSBf */
	} else {
	    soc_reg_field_set(unit, QMC_CONFIG0r, &reg_value, RATE_SHIFTf,
			      SOC_SBX_CFG(unit)->demand_scale);
	}
    }
    SOC_IF_ERROR_RETURN(WRITE_QMC_CONFIG0r(unit, reg_value));

    /* Update Max Arrival bytes */
    SOC_IF_ERROR_RETURN(READ_QMC_DC_CONFIG0r(unit, &reg_value));
    soc_reg_field_set(unit, QMC_DC_CONFIG0r, &reg_value, MAX_ARRIVAL_BYTESf,
              (epoch_length_in_ns/1000 * SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs/8));
    SOC_IF_ERROR_RETURN(WRITE_QMC_DC_CONFIG0r(unit, reg_value));

    return(rv);
}

int
bcm_sirius_fabric_crossbar_enable_get(int unit,
                                      uint64 *xbars)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_sirius_fabric_crossbar_status_get(int unit,
                                      uint64 *xbars)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_sirius_fabric_distribution_create(int unit,
                                      bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_NONE;
    return rv;
}

int
bcm_sirius_fabric_distribution_destroy(int unit,
                                       bcm_fabric_distribution_t  ds_id)
{
    int rv = BCM_E_NONE;
    return rv;
}

int
bcm_sirius_fabric_distribution_set(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int modid_count,
                                   int *dist_modids,
				   int mc_full_eval_min)
{
    int rv = BCM_E_NONE;
    int node, mods;
    int qe2k = FALSE, hybrid = FALSE;
    int eset_node_type, eset_burst_size;
    int entry;
    soc_field_t field;
    eset_to_node_type_entry_t type_entry;    
    burst_size_per_eset_entry_t burst_size_entry;

    /* update eset node type table 
     *   if any of the node in the eset is qe2k, set it to be qe2k type
     *   if any of the node in the eset is sirius hybrid, set it to be sirius hybrid
     */
    for (mods = 0; mods < modid_count; mods++) {
	if (!BCM_STK_MOD_IS_NODE(dist_modids[mods])) {
	    return BCM_E_PARAM;
	}

	node = BCM_STK_MOD_TO_NODE(dist_modids[mods]);
	if ( (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol1) ||
	     (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol2) ) {
	    qe2k = TRUE;
	} else if (SOC_SBX_STATE(unit)->stack_state->protocol[node] == bcmModuleProtocol4) {
	    hybrid = TRUE;
	}
    }

    /* setup burst size per eset, eset to node type */
    if ((qe2k == TRUE) && (hybrid == FALSE)) {
	/* qe2k mixing with sirius for this eset */
	eset_burst_size = SIRIUS_BURST_SIZE_QE2K;
	eset_node_type = SIRIUS_NODE_TYPE_QE2K;
    } else if ((qe2k == TRUE) && (hybrid == TRUE)) {
	/* qe2k mixing with sirius hybrid node for this eset */
	eset_burst_size = SIRIUS_BURST_SIZE_QE2K;
	eset_node_type = SIRIUS_NODE_TYPE_QE2K;
    } else if ((qe2k == FALSE) && (hybrid == TRUE)) {
	/* sirius FIC mixing with sirius hybrid node */
	eset_burst_size = SIRIUS_BURST_SIZE_HYBRID;
	eset_node_type = SIRIUS_NODE_TYPE_HYBRID;
    } else {
	/* sirius FIC only */
	eset_burst_size = SIRIUS_BURST_SIZE_FIC;
	eset_node_type = SIRIUS_NODE_TYPE_FIC;	
    }

    entry = ds_id / 32;
    switch (ds_id % 32) {
	case 0:
	    field = ESET_NODE_TYPE0f;
	    break;
	case 1:
	    field = ESET_NODE_TYPE1f;
	    break;
	case 2:
	    field = ESET_NODE_TYPE2f;
	    break;
	case 3:
	    field = ESET_NODE_TYPE3f;
	    break;
	case 4:
	    field = ESET_NODE_TYPE4f;
	    break;
	case 5:
	    field = ESET_NODE_TYPE5f;
	    break;
	case 6:
	    field = ESET_NODE_TYPE6f;
	    break;
	case 7:
	    field = ESET_NODE_TYPE7f;
	    break;
	case 8:
	    field = ESET_NODE_TYPE8f;
	    break;
	case 9:
	    field = ESET_NODE_TYPE9f;
	    break;
	case 10:
	    field = ESET_NODE_TYPE10f;
	    break;
	case 11:
	    field = ESET_NODE_TYPE11f;
	    break;
	case 12:
	    field = ESET_NODE_TYPE12f;
	    break;
	case 13:
	    field = ESET_NODE_TYPE13f;
	    break;
	case 14:
	    field = ESET_NODE_TYPE14f;
	    break;
	case 15:
	    field = ESET_NODE_TYPE15f;
	    break;
	case 16:
	    field = ESET_NODE_TYPE16f;
	    break;
	case 17:
	    field = ESET_NODE_TYPE17f;
	    break;
	case 18:
	    field = ESET_NODE_TYPE18f;
	    break;
	case 19:
	    field = ESET_NODE_TYPE19f;
	    break;
	case 20:
	    field = ESET_NODE_TYPE20f;
	    break;
	case 21:
	    field = ESET_NODE_TYPE21f;
	    break;
	case 22:
	    field = ESET_NODE_TYPE22f;
	    break;
	case 23:
	    field = ESET_NODE_TYPE23f;
	    break;
	case 24:
	    field = ESET_NODE_TYPE24f;
	    break;
	case 25:
	    field = ESET_NODE_TYPE25f;
	    break;
	case 26:
	    field = ESET_NODE_TYPE26f;
	    break;
	case 27:
	    field = ESET_NODE_TYPE27f;
	    break;
	case 28:
	    field = ESET_NODE_TYPE28f;
	    break;
	case 29:
	    field = ESET_NODE_TYPE29f;
	    break;
	case 30:
	    field = ESET_NODE_TYPE30f;
	    break;
	default:
	    field = ESET_NODE_TYPE31f;
	    break;
    }

    /* eset node type */
    SOC_IF_ERROR_RETURN(READ_ESET_TO_NODE_TYPEm(unit, MEM_BLOCK_ANY, entry, &type_entry));
    soc_mem_field32_set(unit, ESET_TO_NODE_TYPEm, &type_entry, field, eset_node_type);
    SOC_IF_ERROR_RETURN(WRITE_ESET_TO_NODE_TYPEm(unit, MEM_BLOCK_ANY, entry, &type_entry));

    /* eset burst size */
    SOC_IF_ERROR_RETURN(READ_BURST_SIZE_PER_ESETm(unit, MEM_BLOCK_ANY, ds_id, &burst_size_entry));
    soc_mem_field32_set(unit, BURST_SIZE_PER_ESETm, &burst_size_entry, BURST_SIZE_ESETf, eset_burst_size);
    SOC_IF_ERROR_RETURN(WRITE_BURST_SIZE_PER_ESETm(unit, MEM_BLOCK_ANY, ds_id, &burst_size_entry));

    /* Update ESET_TYPE_TAB for fr config */
    entry = ds_id;
    SOC_IF_ERROR_RETURN(WRITE_ESET_TYPE_TABm(unit, MEM_BLOCK_ANY, entry, &eset_node_type));

    return rv;
}

int
bcm_sirius_fabric_distribution_get(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int max_count,
                                   int *dist_modids,
                                   int *count)
{
    int rv = BCM_E_NONE;
    return rv;
}


int
bcm_sirius_fabric_packet_adjust_set(int unit,
                                    int pkt_adjust_selector,
                                    int pkt_adjust_len)
{
    uint32 uData = 0;
    uint32 vData = 0;
    uint32 pkt_adj_data;
    uint32 flags;
    ep_length_adj_map_entry_t epLenAdjData;
    int ingrGlobal = 0;
    int adjusted;
    int indexed[17];
    int original[17];
    int index = 0;
    int result = BCM_E_NONE;
    int auxres;

    /*
     *  For Sirius, there is no true 'global' adjust value.  Multicast is
     *  conditionally adjusted by the 'global' value, but unicast does not seem
     *  to be affected by it at all.  This, of course, means that we have to
     *  fake having a global adjust value (so all unicast adjust values are
     *  biased by global offset that is also written to the 'global' value so
     *  it affects multicast (and we set the multicast adjust enable TRUE).
     *
     *  Note that this affects the allowed range for the indexed adjust values.
     *  The 'global' can be -127..+127, but so can the indexed values, which
     *  means that, for example, a -127 global will bias the valid range for
     *  indexed values to 0..+254, rather than its usual value.
     *
     *  It also means that simply aborting on most failures is a Bad Thing, if
     *  we're updating the 'global' adjust.  We need to back out changes that
     *  have been made so far in case of failure.
     *
     *  Oh, the ingress settings are also paired, so aborting there is a Bad
     *  Thing even if not doing a global adjustment.  Need to back out partial
     *  changes in case of errors on ingress indexed adjustments.
     */

    /* get the flags */
    flags = pkt_adjust_selector & ~(BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK);
    /* figure out which way to set, 'neither' means 'all applicable' */
    if (!(flags & (BCM_FABRIC_PACKET_ADJUST_EGRESS |
                   BCM_FABRIC_PACKET_ADJUST_INGRESS))) {
        flags |= (BCM_FABRIC_PACKET_ADJUST_INGRESS |
                  BCM_FABRIC_PACKET_ADJUST_EGRESS);
    }
    /* get the correct adjust index to set */
    pkt_adjust_selector &= BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK;

    if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
        /* working with ingress; get ingress global bias value */
        /* can abort if failure here */
        BCM_IF_ERROR_RETURN(soc_reg32_read(unit,
                                           soc_reg_addr(unit,
                                                        _qmb_pkt_hdr_adjust[16],
                                                        REG_PORT_ANY,
                                                        0),
                                           &uData));
        ingrGlobal = soc_reg_field_get(unit,
                                       _qmb_pkt_hdr_adjust[16],
                                       uData,
                                       _pkt_hdr_adjust[16]);
        if (soc_reg_field_get(unit,
                              _qmb_pkt_hdr_adjust[16],
                              uData,
                              _pkt_hdr_adjust_sign[16])) {
            ingrGlobal = -ingrGlobal;
        }
    } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */

    if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) {
        /* adjusting the global value */
        /*
         *  For this case, we must adjust all of the indexed values as well
         *  as the 'global' value, since the 'global' value affects only
         *  multicast, and the indexed values affect only unicast.
         */
        /* hardware is unsigned 7b plus explicit sign bit; -0 is invalid */
        /* can abort if invalid arguments here */
        if ((-127 > pkt_adjust_len) || (127 < pkt_adjust_len)) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s pkt_adjust_len(%d) out of range (-127..+127)\n"),
                       FUNCTION_NAME(),
                       pkt_adjust_len));
            return BCM_E_PARAM;
        }
        if (pkt_adjust_selector) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s nonzero selector not valid for 'global'\n"),
                       FUNCTION_NAME()));
            return BCM_E_PARAM;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /*
             *  Collect all of the indexed settings, removing the current
             *  bias from them, applying the new bias to them, and making
             *  sure all of them will still fit.  Keep a copy of them as
             *  they are currently in case something goes wrong.
             */
            /* can abort if something goes wrong here */
            for (index = 0;
                 (index < 16) && (BCM_E_NONE == result);
                 index++) {
                BCM_IF_ERROR_RETURN(soc_reg32_read(unit,
                                                   soc_reg_addr(unit,
                                                                _qmb_pkt_hdr_adjust[index],
                                                                REG_PORT_ANY,
                                                                0),
                                                   &uData));
                indexed[index] = soc_reg_field_get(unit,
                                                   _qmb_pkt_hdr_adjust[index],
                                                   uData,
                                                   _pkt_hdr_adjust[index]);
                if (soc_reg_field_get(unit,
                                      _qmb_pkt_hdr_adjust[index],
                                      uData,
                                      _pkt_hdr_adjust_sign[index])) {
                    indexed[index] = -indexed[index];
                }
                original[index] = indexed[index];
                indexed[index] += (pkt_adjust_len - ingrGlobal);
                if ((-127 > indexed[index]) || (127 < indexed[index])) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s new global %d pushes"
                                           " pkt_adjust_len[%d] out of range (would"
                                           " be %d; limit -127..+127)\n"),
                               FUNCTION_NAME(),
                               pkt_adjust_len,
                               index,
                               indexed[index]));
                    return BCM_E_CONFIG;
                } /* if (newly adjusted value is out of range) */
            } /* for (all indexed adjust registers unless error) */
            /* set new global value in the array */
            indexed[16] = pkt_adjust_len;
            original[16] = ingrGlobal;
            /* Commit changes to the adjust values, including global */
            /* must not abort for failure here -- restore is obligatory */
            for (index = 0; index < 17; index++) {
                if (0 > indexed[index]) {
                    pkt_adj_data = -indexed[index];
                } else {
                    pkt_adj_data = indexed[index];
                }
                result = soc_reg32_read(unit,
                                        soc_reg_addr(unit,
                                                     _qmb_pkt_hdr_adjust[index],
                                                     REG_PORT_ANY,
                                                     0),
                                        &uData);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s unable to get qmb_pkt_hdr_adjust[%d]:"
                                           " %d (%s)\n"),
                               FUNCTION_NAME(),
                               index,
                               result,
                               _SHR_ERRMSG(result)));
                } else { /* if (BCM_E_NONE != result) */
                    soc_reg_field_set(unit,
                                      _qmb_pkt_hdr_adjust[index],
                                      &uData,
                                      _pkt_hdr_adjust[index],
                                      pkt_adj_data);
                    soc_reg_field_set(unit,
                                      _qmb_pkt_hdr_adjust[index],
                                      &uData,
                                      _pkt_hdr_adjust_sign[index],
                                      (0 > indexed[index]));
                    if (QMB_PKT_HDR_ADJUST4r == _qmb_pkt_hdr_adjust[index]) {
                        soc_reg_field_set(unit,
                                          _qmb_pkt_hdr_adjust[index],
                                          &uData,
                                          MC_USE_GLOBAL_LEN_ADJ_IDXf,
                                          (0 != indexed[index]));
                    }
                    result = soc_reg32_write(unit,
                                             soc_reg_addr(unit,
                                                          _qmb_pkt_hdr_adjust[index],
                                                          REG_PORT_ANY,
                                                          0),
                                             uData);
                    if (BCM_E_NONE != result) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to set qmb_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   result,
                                   _SHR_ERRMSG(result)));
                    }
                } /* if (BCM_E_NONE != result) */
                if (BCM_E_NONE == result) {
                    result = soc_reg32_read(unit,
                                            soc_reg_addr(unit,
                                                         _tx_pkt_hdr_adjust[index],
                                                         REG_PORT_ANY,
                                                         0),
                                            &uData);
                    if (BCM_E_NONE != result) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to get tx_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   result,
                                   _SHR_ERRMSG(result)));
                    }
                } /* if (BCM_E_NONE == result) */
                if (BCM_E_NONE == result) {
                    soc_reg_field_set(unit,
                                      _qmb_pkt_hdr_adjust[index],
                                      &uData,
                                      _pkt_hdr_adjust[index],
                                      pkt_adj_data);
                    soc_reg_field_set(unit,
                                      _qmb_pkt_hdr_adjust[index],
                                      &uData,
                                      _pkt_hdr_adjust_sign[index],
                                      (0 > indexed[index]));
                    if (TX_PKT_HDR_ADJUST4r == _tx_pkt_hdr_adjust[index]) {
                        soc_reg_field_set(unit,
                                          _qmb_pkt_hdr_adjust[index],
                                          &uData,
                                          MC_USE_GLOBAL_LEN_ADJ_IDXf,
                                          (0 != indexed[index]));
                    }
                    result = soc_reg32_write(unit,
                                             soc_reg_addr(unit,
                                                          _tx_pkt_hdr_adjust[index],
                                                          REG_PORT_ANY,
                                                          0),
                                             uData);
                    if (BCM_E_NONE != result) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to set tx_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   result,
                                   _SHR_ERRMSG(result)));
                    }
                } /* if (BCM_E_NONE == result) */
                if (BCM_E_NONE != result) {
                    /* ensure we don't increment the index */
                    break;
                }
            } /* for (all adjust values unless error) */
            if (BCM_E_NONE != result) {
                /* something went wrong; must back out changes */
                /* complain for errors here, but only return original */
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s backing out ingress changes after error\n"),
                           FUNCTION_NAME()));
                /* can not abort for error here, but any errors = corruption */
                for (; index >= 0; index--) {
                    if (0 > original[index]) {
                        pkt_adj_data = -original[index];
                    } else {
                        pkt_adj_data = original[index];
                    }
                    auxres = soc_reg32_read(unit,
                                            soc_reg_addr(unit,
                                                         _qmb_pkt_hdr_adjust[index],
                                                         REG_PORT_ANY,
                                                         0),
                                            &uData);
                    if (BCM_E_NONE != auxres) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to get qmb_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   auxres,
                                   _SHR_ERRMSG(auxres)));
                    } else { /* if (BCM_E_NONE != result) */
                        soc_reg_field_set(unit,
                                          _qmb_pkt_hdr_adjust[index],
                                          &uData,
                                          _pkt_hdr_adjust[index],
                                          pkt_adj_data);
                        soc_reg_field_set(unit,
                                          _qmb_pkt_hdr_adjust[index],
                                          &uData,
                                          _pkt_hdr_adjust_sign[index],
                                          (0 > original[index]));
                        if (QMB_PKT_HDR_ADJUST4r == _qmb_pkt_hdr_adjust[index]) {
                            soc_reg_field_set(unit,
                                              _qmb_pkt_hdr_adjust[index],
                                              &uData,
                                              MC_USE_GLOBAL_LEN_ADJ_IDXf,
                                              (0 != original[index]));
                        }
                        auxres = soc_reg32_write(unit,
                                                 soc_reg_addr(unit,
                                                              _qmb_pkt_hdr_adjust[index],
                                                              REG_PORT_ANY,
                                                              0),
                                                 uData);
                        if (BCM_E_NONE != auxres) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s unable to set qmb_pkt_hdr_adjust[%d]:"
                                                   " %d (%s)\n"),
                                       FUNCTION_NAME(),
                                       index,
                                       auxres,
                                       _SHR_ERRMSG(auxres)));
                        }
                    } /* if (BCM_E_NONE != result) */
                    auxres = soc_reg32_read(unit,
                                            soc_reg_addr(unit,
                                                         _tx_pkt_hdr_adjust[index],
                                                         REG_PORT_ANY,
                                                         0),
                                            &uData);
                    if (BCM_E_NONE != auxres) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to get tx_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   auxres,
                                   _SHR_ERRMSG(auxres)));
                    } else {
                        soc_reg_field_set(unit,
                                          _qmb_pkt_hdr_adjust[index],
                                          &uData,
                                          _pkt_hdr_adjust[index],
                                          pkt_adj_data);
                        soc_reg_field_set(unit,
                                          _qmb_pkt_hdr_adjust[index],
                                          &uData,
                                          _pkt_hdr_adjust_sign[index],
                                          (0 > original[index]));
                        if (TX_PKT_HDR_ADJUST4r == _tx_pkt_hdr_adjust[index]) {
                            soc_reg_field_set(unit,
                                              _qmb_pkt_hdr_adjust[index],
                                              &uData,
                                              MC_USE_GLOBAL_LEN_ADJ_IDXf,
                                              (0 != original[index]));
                        }
                        auxres = soc_reg32_write(unit,
                                                 soc_reg_addr(unit,
                                                              _tx_pkt_hdr_adjust[index],
                                                              REG_PORT_ANY,
                                                              0),
                                                 uData);
                        if (BCM_E_NONE != auxres) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s unable to set tx_pkt_hdr_adjust[%d]:"
                                                   " %d (%s)\n"),
                                       FUNCTION_NAME(),
                                       index,
                                       auxres,
                                       _SHR_ERRMSG(auxres)));
                        }
                    } /* if (BCM_E_NONE != result) */
                } /* for (; index >= 0; index--) */
                /* can finally abort for the error now */
                return result;
            } /* if (BCM_E_NONE != result) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /*
             *  Collect all of the indexed settings, removing the current bias
             *  from them, applying the new bias to them, and making sure all
             *  of them will still fit.  Keep a copy of them as they are
             *  currently in case something goes wrong.
             */
            /* can abort if something goes wrong here */
            for (index = 0; index < 16; index++) {
                BCM_IF_ERROR_RETURN(soc_mem_read(unit,
                                                 EP_LENGTH_ADJ_MAPm,
                                                 MEM_BLOCK_ANY,
                                                 index,
                                                 &epLenAdjData));
                pkt_adj_data = soc_mem_field32_get(unit,
                                                   EP_LENGTH_ADJ_MAPm,
                                                   &epLenAdjData,
                                                   ADJf);
                if (pkt_adj_data & 0x80) {
                    indexed[index] = -(pkt_adj_data & 0x7F);
                } else {
                    indexed[index] = pkt_adj_data & 0x7F;
                }
                original[index] = indexed[index];
                indexed[index] += (pkt_adjust_len -
                                   SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust);
                if ((-127 > indexed[index]) || (127 < indexed[index])) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s new global %d pushes ep_pkt_adjust[%d] out"
                                           " of range (would be %d; limit -127..+127)\n"),
                               FUNCTION_NAME(),
                               pkt_adjust_len,
                               index,
                               indexed[index]));
                    
                    return BCM_E_CONFIG;
                }
            } /* for (index = 0; index < 16; index++) */
            /* Commit changes to the adjust values */
            /* must not abort for failure here -- restore is obligatory */
            for (index = 0; index < 16; index++) {
                if (0 > indexed[index]) {
                    pkt_adj_data = -(indexed[index]) | 0x80;
                } else {
                    pkt_adj_data = indexed[index];
                }
                result = soc_mem_read(unit,
                                      EP_LENGTH_ADJ_MAPm,
                                      MEM_BLOCK_ANY,
                                      index,
                                      &epLenAdjData);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s unable to get ep_length_adj_map[%d]:"
                                           " %d (%s)\n"),
                               FUNCTION_NAME(),
                               index,
                               result,
                               _SHR_ERRMSG(result)));
                } else { /* if (BCM_E_NONE != result) */
                    soc_mem_field32_set(unit,
                                        EP_LENGTH_ADJ_MAPm,
                                        &epLenAdjData,
                                        ADJf,
                                        pkt_adj_data);
                    result = soc_mem_write(unit,
                                           EP_LENGTH_ADJ_MAPm,
                                           MEM_BLOCK_ALL,
                                           index,
                                           &epLenAdjData);
                    if (BCM_E_NONE != result) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to set ep_length_adj_map[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   result,
                                   _SHR_ERRMSG(result)));
                    }
                } /* if (BCM_E_NONE != result) */
                if (BCM_E_NONE != result) {
                    /* ensure we don't increment the index */
                    break;
                }
            } /* for (index = 0; index < 16; index++) */
            if (BCM_E_NONE == result) {
                /* now update the global setting (virtual) */
                SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust = pkt_adjust_len;
            } else { /* if (BCM_E_NONE == result) */
                /* something went wrong; must back out changes */
                /* complain for errors here, but only return original */
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s backing out ingress changes after error\n"),
                           FUNCTION_NAME()));
                /* can not abort for error here, but any errors = corruption */
                for (; index >= 0; index--) {
                    if (0 > original[index]) {
                        pkt_adj_data = -(original[index]) | 0x80;
                    } else {
                        pkt_adj_data = original[index];
                    }
                    auxres = soc_mem_read(unit,
                                          EP_LENGTH_ADJ_MAPm,
                                          MEM_BLOCK_ANY,
                                          index,
                                          &epLenAdjData);
                    if (BCM_E_NONE != auxres) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to get ep_length_adj_map[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   auxres,
                                   _SHR_ERRMSG(auxres)));
                    } else { /* if (BCM_E_NONE != result) */
                        soc_mem_field32_set(unit,
                                            EP_LENGTH_ADJ_MAPm,
                                            &epLenAdjData,
                                            ADJf,
                                            pkt_adj_data);
                        auxres = soc_mem_write(unit,
                                               EP_LENGTH_ADJ_MAPm,
                                               MEM_BLOCK_ALL,
                                               index,
                                               &epLenAdjData);
                        if (BCM_E_NONE != auxres) {
                            LOG_ERROR(BSL_LS_BCM_FABRIC,
                                      (BSL_META_U(unit,
                                                  "%s unable to set ep_length_adj_map[%d]:"
                                                   " %d (%s)\n"),
                                       FUNCTION_NAME(),
                                       index,
                                       auxres,
                                       _SHR_ERRMSG(auxres)));
                        }
                    } /* if (BCM_E_NONE != result) */
                } /* for (; index >= 0; index--) */
                /* can finally abort for the error now */
                return result;
            } /* if (BCM_E_NONE == result) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } else { /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
        /* adjusting one of the indexed values */
        /* can abort if invalid arguments here */
        if ((0 > pkt_adjust_selector) || (15 < pkt_adjust_selector)) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s pkt_adjust_selector(%d) out of range\n"),
                       FUNCTION_NAME(),
                       pkt_adjust_selector));
            return BCM_E_PARAM;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /* adjust indexed ingress setting */
            /* can abort if invalid arguments here */
            adjusted = pkt_adjust_len + ingrGlobal;
            if ((-127 > adjusted) || (127 < adjusted)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s pkt_adjust_selector(%d) new value %d is out of"
                                       " range (%d..%d with %d global)\n"),
                           FUNCTION_NAME(),
                           pkt_adjust_selector,
                           pkt_adjust_len,
                           -127 - ingrGlobal,
                           127 - ingrGlobal,
                           ingrGlobal));
                return BCM_E_PARAM;
            }
            if (0 > adjusted) {
                pkt_adj_data = -adjusted;
            } else {
                pkt_adj_data = adjusted;
            }
            original[2] = FALSE;
            original[5] = FALSE;
            /* initial read can abort for errors */
            BCM_IF_ERROR_RETURN(soc_reg32_read(unit,
                                               soc_reg_addr(unit,
                                                            _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                                            REG_PORT_ANY,
                                                            0),
                                               &uData));
            original[0] = soc_reg_field_get(unit,
                                            _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                            uData,
                                            _pkt_hdr_adjust[pkt_adjust_selector]);
            original[1] = soc_reg_field_get(unit,
                                            _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                            uData,
                                            _pkt_hdr_adjust_sign[pkt_adjust_selector]);
            original[2] = TRUE;
            soc_reg_field_set(unit,
                              _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                              &uData,
                              _pkt_hdr_adjust[pkt_adjust_selector],
                              pkt_adj_data);
            soc_reg_field_set(unit,
                              _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                              &uData,
                              _pkt_hdr_adjust_sign[pkt_adjust_selector],
                              (0 > adjusted));
            /* but after this point, need to back out if errors */
            result = soc_reg32_write(unit,
                                     soc_reg_addr(unit,
                                                  _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                                  REG_PORT_ANY,
                                                  0),
                                     uData);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s unable to set qmb_pkt_hdr_adjust[%d]:"
                                       " %d (%s)\n"),
                           FUNCTION_NAME(),
                           index,
                           result,
                           _SHR_ERRMSG(result)));
            } else {
                result = soc_reg32_read(unit,
                                        soc_reg_addr(unit,
                                                     _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                                     REG_PORT_ANY,
                                                     0),
                                        &vData);
                if (BCM_E_NONE == result) {
                    original[3] = soc_reg_field_get(unit,
                                                    _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                                    vData,
                                                    _pkt_hdr_adjust[pkt_adjust_selector]);
                    original[4] = soc_reg_field_get(unit,
                                                    _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                                    vData,
                                                    _pkt_hdr_adjust_sign[pkt_adjust_selector]);
                    original[5] = TRUE;
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s unable to get tx_pkt_hdr_adjust[%d]:"
                                           " %d (%s)\n"),
                               FUNCTION_NAME(),
                               index,
                               result,
                               _SHR_ERRMSG(result)));
                }
            }
            if (BCM_E_NONE == result) {
                soc_reg_field_set(unit,
                                  _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                  &vData,
                                  _pkt_hdr_adjust[pkt_adjust_selector],
                                  pkt_adj_data);
                soc_reg_field_set(unit,
                                  _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                  &vData,
                                  _pkt_hdr_adjust_sign[pkt_adjust_selector],
                                  (0 > adjusted));
                result = soc_reg32_write(unit,
                                         soc_reg_addr(unit,
                                                      _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                                      REG_PORT_ANY,
                                                      0),
                                         vData);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "%s unable to set tx_pkt_hdr_adjust[%d]:"
                                           " %d (%s)\n"),
                               FUNCTION_NAME(),
                               index,
                               result,
                               _SHR_ERRMSG(result)));
                }
            }
            if (BCM_E_NONE != result) {
                /* something went wrong; must back out changes */
                /* complain for errors here, but only return original */
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s backing out ingress changes after error\n"),
                           FUNCTION_NAME()));
                /* can not abort for error here, but any errors = corruption */
                if (original[2]) {
                    soc_reg_field_set(unit,
                                      _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                      &uData,
                                      _pkt_hdr_adjust[pkt_adjust_selector],
                                      original[0]);
                    soc_reg_field_set(unit,
                                      _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                      &uData,
                                      _pkt_hdr_adjust_sign[pkt_adjust_selector],
                                      original[1]);
                    /* but after this point, need to back out if errors */
                    auxres = soc_reg32_write(unit,
                                             soc_reg_addr(unit,
                                                          _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                                          REG_PORT_ANY,
                                                          0),
                                             uData);
                    if (BCM_E_NONE != auxres) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to set qmb_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   auxres,
                                   _SHR_ERRMSG(auxres)));
                    }
                } /* if (original[2]) */
                if (original[5]) {
                    soc_reg_field_set(unit,
                                      _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                      &vData,
                                      _pkt_hdr_adjust[pkt_adjust_selector],
                                      original[3]);
                    soc_reg_field_set(unit,
                                      _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                      &vData,
                                      _pkt_hdr_adjust_sign[pkt_adjust_selector],
                                      original[4]);
                    auxres = soc_reg32_write(unit,
                                             soc_reg_addr(unit,
                                                          _tx_pkt_hdr_adjust[pkt_adjust_selector],
                                                          REG_PORT_ANY,
                                                          0),
                                             vData);
                    if (BCM_E_NONE != auxres) {
                        LOG_ERROR(BSL_LS_BCM_FABRIC,
                                  (BSL_META_U(unit,
                                              "%s unable to set tx_pkt_hdr_adjust[%d]:"
                                               " %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   index,
                                   auxres,
                                   _SHR_ERRMSG(auxres)));
                    }
                } /* if (original[5]) */
            } /* if (BCM_E_NONE != result) */
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /* adjust indexed egress setting */
            /* this is a single write (thence atomic) so can always abort */
            adjusted = (pkt_adjust_len +
                        SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust);
            if ((-127 > adjusted) || (127 < adjusted)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "%s pkt_adjust_selector(%d) new value %d is out of"
                                       " range (%d..%d with %d global)\n"),
                           FUNCTION_NAME(),
                           pkt_adjust_selector,
                           pkt_adjust_len,
                           -127 - SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust,
                           127 - SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust,
                           SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust));
                return BCM_E_PARAM;
            }
            if (0 > adjusted) {
                pkt_adj_data = (-adjusted) | 0x80 ;
            } else {
                pkt_adj_data = adjusted;
            }
            BCM_IF_ERROR_RETURN(soc_mem_read(unit,
                                             EP_LENGTH_ADJ_MAPm,
                                             MEM_BLOCK_ANY,
                                             pkt_adjust_selector,
                                             &epLenAdjData));
            soc_mem_field32_set(unit,
                                EP_LENGTH_ADJ_MAPm,
                                &epLenAdjData,
                                ADJf,
                                pkt_adj_data);
            BCM_IF_ERROR_RETURN(soc_mem_write(unit,
                                              EP_LENGTH_ADJ_MAPm,
                                              MEM_BLOCK_ALL,
                                              pkt_adjust_selector,
                                              &epLenAdjData));
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
    return result;
}

int
bcm_sirius_fabric_packet_adjust_get(int unit,
                                    int pkt_adjust_selector,
                                    int *pkt_adjust_len)
{
    uint32 uData = 0, flags;
    ep_length_adj_map_entry_t epLenAdjData;
    int adjusted;
    int ingrGlobal = 0;

    /*
     *  There are two copies of everything ingress, kept in sync normally
     *  (unless somebody writes the registers directly); we prefer the qmb copy
     *  simply because it occurred that way.
     *
     *  Aborts are safe here because we're just reading.
     */

    /* get the flags */
    flags = pkt_adjust_selector & ~(BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK);
    /* get the correct adjust index to set */
    pkt_adjust_selector &= BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK;

    if (!(flags & (BCM_FABRIC_PACKET_ADJUST_EGRESS |
                   BCM_FABRIC_PACKET_ADJUST_INGRESS))) {
        flags |= BCM_FABRIC_PACKET_ADJUST_INGRESS;
    }
    if ((flags & (BCM_FABRIC_PACKET_ADJUST_INGRESS |
                  BCM_FABRIC_PACKET_ADJUST_EGRESS)) ==
        (BCM_FABRIC_PACKET_ADJUST_INGRESS | BCM_FABRIC_PACKET_ADJUST_EGRESS)) {
        /* can't read both */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "%s unable to read both ingress and egress\n"),
                   FUNCTION_NAME()));
        return BCM_E_PARAM;
    }
    /* get the correct adjust index to set */
    pkt_adjust_selector &= BCM_FABRIC_PACKET_ADJUST_SELECTOR_MASK;

    if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
        /* working with ingress; get ingress global bias value */
        /* can abort if failure here */
        BCM_IF_ERROR_RETURN(soc_reg32_read(unit,
                                           soc_reg_addr(unit,
                                                        _qmb_pkt_hdr_adjust[16],
                                                        REG_PORT_ANY,
                                                        0),
                                           &uData));
        ingrGlobal = soc_reg_field_get(unit,
                                       _qmb_pkt_hdr_adjust[16],
                                       uData,
                                       _pkt_hdr_adjust[16]);
        if (soc_reg_field_get(unit,
                              _qmb_pkt_hdr_adjust[16],
                              uData,
                              _pkt_hdr_adjust_sign[16])) {
            ingrGlobal = -ingrGlobal;
        }
    } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */


    if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) {
        /* requesting global value */
        if (pkt_adjust_selector) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s global pkt_adjust_selector(%d) out of range\n"),
                       FUNCTION_NAME(),
                       pkt_adjust_selector));
            return BCM_E_PARAM;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /* get ingress 'global' */
            *pkt_adjust_len = ingrGlobal;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /* get egress 'global' */
            *pkt_adjust_len = SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust;
        }
    } else { /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
        /* requesting local value */
        /* 16 templates available; the selector indicates the template */
        if ((0 > pkt_adjust_selector) || (15 < pkt_adjust_selector)) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s pkt_adjust_selector(%d) out of range\n"),
                       FUNCTION_NAME(),
                       pkt_adjust_selector));
            return BCM_E_PARAM;
        }
        if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) {
            /* read ingress adjustment */
            BCM_IF_ERROR_RETURN(soc_reg32_read(unit,
                                               soc_reg_addr(unit,
                                                            _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                                            REG_PORT_ANY,
                                                            0),
                                               &uData));
            adjusted = soc_reg_field_get(unit,
                                         _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                         uData,
                                         _pkt_hdr_adjust[pkt_adjust_selector]);

            if (soc_reg_field_get(unit,
                                  _qmb_pkt_hdr_adjust[pkt_adjust_selector],
                                  uData,
                                  _pkt_hdr_adjust_sign[pkt_adjust_selector])) {
                adjusted *= -1;
            }
            *pkt_adjust_len = adjusted - ingrGlobal;
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_INGRESS) */
        if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) {
            /* read egress adjustment */
            BCM_IF_ERROR_RETURN(soc_mem_read(unit,
                                             EP_LENGTH_ADJ_MAPm,
                                             MEM_BLOCK_ANY,
                                             pkt_adjust_selector,
                                             &epLenAdjData));
            adjusted = soc_mem_field32_get(unit,
                                           EP_LENGTH_ADJ_MAPm,
                                           &epLenAdjData,
                                           ADJf);
            if (adjusted & 0x80) {
                adjusted = -(adjusted & 0x7F);
            } else {
                adjusted = adjusted & 0x7F;
            }
            *pkt_adjust_len = (adjusted -
                               SOC_SBX_CFG_SIRIUS(unit)->egrGlobalAdjust);
        } /* if (flags & BCM_FABRIC_PACKET_ADJUST_EGRESS) */
    } /* if (flags & BCM_FABRIC_PACKET_ADJUST_GLOBAL) */
    return BCM_E_NONE;
}

int32
bcm_sbx_fabric_timeslot_burst_size_bytes_set(int unit, int els)
{
    int rv = BCM_E_NONE;
    int channel, node_type, idx, value;

    if (!SOC_IS_SBX_SIRIUS(unit)) {
        /* only for Sirius*/
        return BCM_E_UNAVAIL;
    }

    if ((els != 0) && (els != 1)) {
        /* els is not 0 or 1*/
        return BCM_E_PARAM; 
    }

    /* +2 -> to cover 0 and 47 channels */
    for (channel=0; (channel < (SIRIUS_NUM_SFI_CHANNELS + 2)); channel++) {
        for (node_type = 0; node_type < SIRIUS_MAX_NODE_TYPES; node_type++) {
            idx = (channel * SIRIUS_MAX_NODE_TYPES) + node_type;
            BCM_IF_ERROR_RETURN(soc_sirius_ts_burst_size_bytes_get(unit, 
                                             els, node_type, channel, &value));
            SOC_IF_ERROR_RETURN(WRITE_TIMESLOT_BURST_SIZE_BYTESm(unit, 
                                                  MEM_BLOCK_ANY, idx, &value));
        }
    }

    return rv;
}

int
_bcm_sirius_fabric_control_voq_max_set(int unit, int value)
{
    int     rv = BCM_E_NONE, nMaxVoq;
    uint32  rval = 0;

    if (value > SB_FAB_DEVICE_SIRIUS_MAX_VOQ) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Specified value(%d) exceeds supported max for "
                              "bcmFabricQueueMax(%d) \n"),
                   value, SB_FAB_DEVICE_SIRIUS_MAX_VOQ));
        return BCM_E_PARAM;
    }

    /* nMaxVoq is treated as the first local qid. Hence +1 if not 0 */
    nMaxVoq = (value? (value + 1): value);

    /* reconfigure only if different from current value */
    if (nMaxVoq != SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq) {
        /* configure QM */
        SOC_IF_ERROR_RETURN(READ_QMA_CONFIG1r(unit, &rval));
        soc_reg_field_set(unit, QMA_CONFIG1r, &rval, VOQ_THRESHOLDf, nMaxVoq); 
        SOC_IF_ERROR_RETURN(WRITE_QMA_CONFIG1r(unit, rval));

        /* configure QS */
        SOC_IF_ERROR_RETURN(READ_QS_CONFIG1r(unit, &rval));
        soc_reg_field_set(unit, QS_CONFIG1r, &rval, LOCAL_BOUNDARYf, nMaxVoq);
        SOC_IF_ERROR_RETURN(WRITE_QS_CONFIG1r(unit, rval));

        /* Re-adjust the shapers */
        rv = _soc_sirius_hw_init_qs_shaper(unit, nMaxVoq);

        if (rv == BCM_E_NONE) {
            /* store the new value */
            SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = nMaxVoq;

	    rv = _bcm_sbx_cosq_queue_regions_set(unit);
	    if (rv != BCM_E_NONE) {
	      LOG_CLI((BSL_META_U(unit,
                                  "Queue region returned error(%d)\n"), rv));
	    }
        }
    }

    return rv;
}

/* 
 * this function handles bcmFabric[Egress]Queue[Min/Max] set/get operations 
 * value: is IN/OUT param 
 */
int
_bcm_sirius_fabric_control_queue_set_get(int unit, 
                                         bcm_fabric_control_t type,
                                         int *value,
                                         int set)
{
    int rv = BCM_E_NONE;
    int TME = 0, FIC = 0;

    if (soc_feature(unit, soc_feature_standalone)) {
        TME = 1;
    } else if (soc_feature(unit, soc_feature_hybrid) == 0) {
        FIC = 1;
    }

    if (set) {
        switch (type) {
        case bcmFabricQueueMin:
            if (*value) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "Invalid value(%d) for bcm_fabric_control_set of "
                                       "bcmFabricQueueMin. Only 0 is valid. \n"), *value));
                rv = BCM_E_PARAM;
            } else {
                /* do nothing. Its already set to 0. */
            }
            break;
        case bcmFabricQueueMax:
            if (TME) {
                if (*value) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "Invalid value(%d) for bcm_fabric_control_set of"
                                           " bcmFabricQueueMax. In TME mode only 0 is "
                                           "valid\n"), *value));
                    rv = BCM_E_PARAM;
                } else {
                    /* do nothing. Its already set to 0. */
                }
            } else {
                /* FIC & hybrid */
                rv = _bcm_sirius_fabric_control_voq_max_set(unit, *value);
            }
            break;
        case bcmFabricEgressQueueMin:
            if (FIC || TME) {
                if (*value) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "Invalid value(%d) for bcm_fabric_control_set"
                                           " of bcmFabricEgressQueueMin. In FIC & TME "
                                           "modes only 0 is valid\n"), *value));
                    rv = BCM_E_PARAM;
                } else {
                    /* do nothing. Its already set to 0. */
                }
            } else {
                /* hybrid */
                rv = _bcm_sirius_fabric_control_voq_max_set(unit, 
                                                            ((*value) - 1));
            }
            break;
        case bcmFabricEgressQueueMax:
            if (FIC) {
                if (*value) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "Invalid value(%d) for bcm_fabric_control_set"
                                           " of bcmFabricEgressQueueMax. In FIC mode only"
                                           " 0 is valid\n"), *value));
                    rv = BCM_E_PARAM;
                } else {
                    /* do nothing. Its already set to 0. */
                }
            } else {
                /* TME & hybrid */
                if (*value != (SB_FAB_DEVICE_SIRIUS_NUM_QUEUES -1)) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "Invalid value(%d) for bcm_fabric_control_set"
                                           " of bcmFabricEgressQueueMax. In FIC mode only"
                                           " 65535 is valid\n"), *value));
                    rv = BCM_E_PARAM;
                } else {
                    /* do nothing. Its already set to 65535. */
                }
            }
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s: Invalid fabric control type passed in \n"), 
                       FUNCTION_NAME()));
            rv = BCM_E_PARAM;
            break;
        }
    } else { 
        /* get */
        switch (type) {
        case bcmFabricQueueMin:
            *value = 0; /* VOQs always start at 0 */
            break;
        case bcmFabricQueueMax:
            *value = SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq -1;
            break;
        case bcmFabricEgressQueueMin:
            if (FIC || TME) {
                /* FIC: no local queues, TME: all local queues */
                *value = 0;
            } else {
                /* hybrid: */
                *value = SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq;
            }
            break;
        case bcmFabricEgressQueueMax:
            if (FIC) {
                *value = 0; /* no local queues */
            } else {
                /* TME & hybrid: all 64k are local */
                *value = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES - 1;
            }
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "%s: Invalid fabric control type passed in \n"), 
                       FUNCTION_NAME()));
            rv = BCM_E_PARAM;
            break;
        }
    }

    return rv;
}


int
bcm_sirius_fabric_control_set(int unit,
                              bcm_fabric_control_t type,
                              int arg)
{
    int             rv = BCM_E_NONE;
    uint32          uData = 0;
    bcm_multicast_t group;
    int             source_knockout, level;
    int             enable_auto_switchover, one_plus_one, els;
    uint32          reg_value;
    int32           local_shape_inc, fic_shape_inc;

    switch (type) {
    case bcmFabricActiveArbiterId:
        BCM_IF_ERROR_RETURN(READ_SC_CONFIG0r(unit, &uData));
        soc_reg_field_set(unit, SC_CONFIG0r, &uData, DEFAULT_BMf, arg);
        BCM_IF_ERROR_RETURN(WRITE_SC_CONFIG0r(unit, uData));
        break;
    case bcmFabricMcGroupSourceKnockout:
        group = arg & BCM_FABRIC_MC_GROUP_MASK;
        source_knockout = ((arg & BCM_FABRIC_MC_GROUP_SOURCE_KNOCKOUT_ENABLE) ? 
                           TRUE : FALSE);
        rv = bcm_sirius_multicast_source_knockout_set(unit, group, 
                                                      source_knockout);
        break;
    case bcmFabricMode:
      if (arg) {
	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
	  /* already in FIC mode */
	  return BCM_E_NONE;
      /* coverity[mixed_enums: : FALSE] */
	} else if (arg == bcmFabricModeFabric) {
	  /* switch QE to FIC mode */
	  SOC_SBX_CFG(unit)->bTmeMode = SOC_SBX_QE_MODE_FIC;
	} else if (arg == 2) {
	  SOC_SBX_CFG(unit)->bTmeMode = SOC_SBX_QE_MODE_HYBRID;
	}
      } else {
	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	  /* already in TME mode */
	  return BCM_E_NONE;
	} else {
	  /* switch QE to TME mode */
	  SOC_SBX_CFG(unit)->bTmeMode = SOC_SBX_QE_MODE_TME;
	}
      } 
      soc_sbx_set_epoch_length(unit);
      SOC_SBX_CFG(unit)->reset_ul = 1;
      
      /* Set feature cache, since used by mutex creation */
      if (SOC_DRIVER(unit)->feature) {
	soc_feature_init(unit);
      }
      
      rv = soc_sirius_init(unit, SOC_SBX_CFG(unit));
      break;
    case bcmFabricRedundancyMode:
        enable_auto_switchover = 0;
        one_plus_one = 0;
        els = 0;
        switch (arg) {
        /* coverity[mixed_enums: : FALSE] */
        case bcmFabricRed1Plus1Both:
            enable_auto_switchover = 1;
            one_plus_one = 1;
            break;
        /* coverity[mixed_enums: : FALSE] */
        case bcmFabricRed1Plus1ELS:
            els = 1; /* intentional fall through */
        /* coverity[mixed_enums: : FALSE] */
        case bcmFabricRed1Plus1LS:
            enable_auto_switchover = 1;
            break;
        /* coverity[mixed_enums: : FALSE] */
        case bcmFabricRedELS:
            els = 1; /* intentional fall through */
        case bcmFabricRedLS:
        /* coverity[mixed_enums: : FALSE] */
        case bcmFabricRedManual:
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unsupported bcmFabricRedundancyMode:%d\n"),
                       arg));
            rv = BCM_E_PARAM;
            break;
        }
        if(rv != BCM_E_NONE) {
            return rv;
        }
        BCM_IF_ERROR_RETURN(READ_SC_CONFIG0r(unit, &uData));
        soc_reg_field_set(unit, SC_CONFIG0r, &uData, ENABLE_AUTO_SWITCHOVERf,
                          enable_auto_switchover);
        soc_reg_field_set(unit, SC_CONFIG0r, &uData, MODEf, one_plus_one);
        BCM_IF_ERROR_RETURN(WRITE_SC_CONFIG0r(unit, uData));

        BCM_IF_ERROR_RETURN(READ_FR_CONFIG0r(unit, &uData));
        soc_reg_field_set(unit, FR_CONFIG0r, &uData, MODEf, els);
        BCM_IF_ERROR_RETURN(WRITE_FR_CONFIG0r(unit, uData));

        /* Clear interrupts, re-arm redundancy */
        BCM_IF_ERROR_RETURN(READ_SC_ERROR1r(unit, &uData));
        soc_reg_field_set(unit, SC_ERROR1r, &uData, AUTO_SWITCH_EVENT_CTRLf, 1);
        soc_reg_field_set(unit, SC_ERROR1r, &uData, AUTO_SWITCH_EVENT_DATAf, 1);
        BCM_IF_ERROR_RETURN(WRITE_SC_ERROR1r(unit, uData));

        /* set the TIMESLOT_BURST_SIZE_BYTES table */
        rv = bcm_sbx_fabric_timeslot_burst_size_bytes_set(unit, els);

        if (rv == BCM_E_NONE) {
            /* Set the burst sizes in case of back-pressure */
            rv = soc_sirius_bp_burst_size_bytes_set(unit);
        }

        break;
    case bcmFabricQueueMin: /* intentional fall through */
    case bcmFabricQueueMax: /* intentional fall through */
    case bcmFabricEgressQueueMin: /* intentional fall through */
    case bcmFabricEgressQueueMax:
        rv = _bcm_sirius_fabric_control_queue_set_get(unit, type, &arg, 1);
        break;
    case bcmFabricSubscriberCosLevels:
	if (arg <= 4) {
	    SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos = 4;
	    /* if max cos level is 4, configure TS to map 4 leaf nodes to each level 1 node */
	    SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = FALSE;
	} else if (arg <= 8) {
	    SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos = 8;
	    /* if max cos level is 8, configure TS to map 4 leaf nodes to each level 1 node
	     * only if we are trying to optimize the node usage.
	     */
	    if (SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize == TRUE) {
		SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = FALSE;
	    } else {
		SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = TRUE;
	    }
	} else {
	    rv = BCM_E_PARAM;
	}

	/* update hardware config */
	SOC_IF_ERROR_RETURN(READ_TS_LEVEL1_CONFIG0r(unit, &uData));
	soc_reg_field_set(unit, TS_LEVEL1_CONFIG0r, &uData, EIGHTK_NODESf,
			  ((SOC_SBX_CFG_SIRIUS(unit)->b8kNodes) ? 1 : 0));
	SOC_IF_ERROR_RETURN(WRITE_TS_LEVEL1_CONFIG0r(unit, uData));
	break;
    case bcmFabricSubscriberCosLevelAllocation:
	if (arg == 4) {
	    SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize = TRUE;
	    SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = FALSE;
	} else if (arg == 8) {
	    SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize = FALSE;
	    if (SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos == 8) {
		SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = TRUE;
	    } else {
		SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = FALSE;
	    }
	} else {
	    rv = BCM_E_PARAM;
	}

	/* update hardware config */
	SOC_IF_ERROR_RETURN(READ_TS_LEVEL1_CONFIG0r(unit, &uData));
	soc_reg_field_set(unit, TS_LEVEL1_CONFIG0r, &uData, EIGHTK_NODESf,
			  ((SOC_SBX_CFG_SIRIUS(unit)->b8kNodes) ? 1 : 0));
	SOC_IF_ERROR_RETURN(WRITE_TS_LEVEL1_CONFIG0r(unit, uData));
	break;
    case bcmFabricArbitrationMapFabric:
    case bcmFabricArbitrationMapSubscriber:
    case bcmFabricArbitrationMapHierarchicalSubscriber:
        rv = BCM_E_UNAVAIL;
        break;


    
    case bcmFabricShaperEgressQueueMin:
        if (!( soc_feature(unit, soc_feature_standalone) ||
               soc_feature(unit, soc_feature_hybrid) ||
               soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }
        if ((arg < 0) || (arg > (SB_FAB_DEVICE_SIRIUS_NUM_QUEUES - 1))) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &reg_value));
        soc_reg_field_set(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, &reg_value, STARTQUEUEf, arg);
        SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, reg_value));
        break;

    case bcmFabricShaperEgressQueueMax:
        if (!( soc_feature(unit, soc_feature_standalone) ||
               soc_feature(unit, soc_feature_hybrid) ||
               soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }
        if ((arg < 0) || (arg > (SB_FAB_DEVICE_SIRIUS_NUM_QUEUES - 1))) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, &reg_value));
        soc_reg_field_set(unit, SHAPER_QUEUE_LOCAL_RANGE_ENDr, &reg_value, ENDQUEUEf, arg);
        SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, reg_value));
        break;

    case bcmFabricShaperEgressQueueIncrement:
        if (!( soc_feature(unit, soc_feature_standalone) ||
               soc_feature(unit, soc_feature_hybrid) ||
               soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }
        if ((arg != 1) && (arg != 4)) {
            return(BCM_E_PARAM);
        }

        local_shape_inc = (arg == 1) ? 0 : 1;
        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &reg_value));
        soc_reg_field_set(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, &reg_value, INCf, local_shape_inc);
        SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, reg_value));
        break;

    case bcmFabricShaperQueueMin:
        if (!( !(soc_feature(unit, soc_feature_standalone)) ||
                 soc_feature(unit, soc_feature_hybrid) ||
                 soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }
        if ((arg < 0) || (arg > (SB_FAB_SWITCH_CONFIG_MAX_VOQS - 1))) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, &reg_value));
        soc_reg_field_set(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, &reg_value, STARTQUEUEf, arg);
        SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, reg_value));
        break;

    case bcmFabricShaperQueueMax:
        if (!( !(soc_feature(unit, soc_feature_standalone)) ||
                 soc_feature(unit, soc_feature_hybrid) ||
                 soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }
        if ((arg < 0) || (arg > (SB_FAB_SWITCH_CONFIG_MAX_VOQS - 1))) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_ENDr(unit, &reg_value));
        soc_reg_field_set(unit, SHAPER_QUEUE_FABRIC_RANGE_ENDr, &reg_value, ENDQUEUEf, arg);
        SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_FABRIC_RANGE_ENDr(unit, reg_value));
        break;

    case bcmFabricShaperQueueIncrement:
        if ( !(soc_feature(unit, soc_feature_standalone)) ||
                 soc_feature(unit, soc_feature_hybrid) ||
                 soc_feature(unit, soc_feature_node_hybrid) ) {
            return(BCM_E_PARAM);
        }
        if ((arg != 1) && (arg != 4)) {
            return(BCM_E_PARAM);
        }

        fic_shape_inc = (arg == 1) ? 0 : 1;
        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, &reg_value));
        soc_reg_field_set(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, &reg_value, INCf, fic_shape_inc);
        SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, reg_value));
        break;

    case bcmFabricTsApplicationHierachySetup:
	if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
	    if (!arg) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "User managing scheduler resource, could not switch to SDK management mode\n")));
		return(BCM_E_PARAM);
	    }
	} else if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == FALSE) {
	    if (arg != FALSE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "SDK managing scheduler resource, could not switch to user management mode\n")));
		return(BCM_E_PARAM);
	    }
	} else {
	    if (arg) {
		SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = TRUE;
	    } else {
		SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = FALSE;
	    }
	}
	break;
    case bcmFabricMaxPorts:
	if ((arg < 0) || (arg >= 256)) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "SDK does not support %d fabric ports\n"),
	               arg));
	    return(BCM_E_PARAM);	    
	}

        SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts = arg;

        if (SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts > 128) {
	    if ((SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME) &&
		(SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME_BYPASS)) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "Only TME or TME_BYPASS mode supports more than 128 ports on unit %d"),
		           unit));
		return BCM_E_PARAM;
	    }

	    /* forcing independent flow control */
	    if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == FALSE) {
		LOG_ERROR(BSL_LS_BCM_FABRIC,
		          (BSL_META_U(unit,
		                      "Require independent flow control mode to support more than 128 ports on unit %d"),
		           unit));
		return BCM_E_PARAM;	    
	    }
	    
	    SOC_IF_ERROR_RETURN(READ_ES_LL_FC_CONFIGr(unit, &uData));
	    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &uData, FC_MAX_PORT_ENABLEf, 1);        /* Max ports */
	    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &uData, FC_EVEN_PORT_STATE_MASKf, 0xC); /* Even port mask */
	    SOC_IF_ERROR_RETURN(WRITE_ES_LL_FC_CONFIGr(unit, uData));

	    SOC_IF_ERROR_RETURN(READ_TS_CONFIG4r(unit, &uData));
	    soc_reg_field_set(unit, TS_CONFIG4r, &uData, PRIORITY_THRESH1f, 13);
	    SOC_IF_ERROR_RETURN(WRITE_TS_CONFIG4r(unit, uData));

	    SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = TRUE;
	} else {
	    SOC_IF_ERROR_RETURN(READ_ES_LL_FC_CONFIGr(unit, &uData));
	    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &uData, FC_MAX_PORT_ENABLEf, 0);        /* Max ports */
	    soc_reg_field_set(unit, ES_LL_FC_CONFIGr, &uData, FC_EVEN_PORT_STATE_MASKf, 0x5); /* Register Default */
	    SOC_IF_ERROR_RETURN(WRITE_ES_LL_FC_CONFIGr(unit, uData));

	    SOC_IF_ERROR_RETURN(READ_TS_CONFIG4r(unit, &uData));
	    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
		soc_reg_field_set(unit, TS_CONFIG4r, &uData, PRIORITY_THRESH1f, 15);
	    } else {
		soc_reg_field_set(unit, TS_CONFIG4r, &uData, PRIORITY_THRESH1f, 0);
	    }
	    SOC_IF_ERROR_RETURN(WRITE_TS_CONFIG4r(unit, uData));

	    SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = FALSE;
	}
	break;
	  
    case bcmFabricEgressDropLimitBytes:
	if (arg > 0x7fff) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "ERROR: Drop Limit Bytes range 0-0x7fff\n")));
	    rv = BCM_E_PARAM;
	    return rv;	
	}
	soc_reg_field_set(unit, FD_TOTAL_BUFFER_LIMITr, &uData, TOTAL_BUFFER_LIMITf, arg);
	BCM_IF_ERROR_RETURN(WRITE_FD_TOTAL_BUFFER_LIMITr(unit, uData));
	break;
    case bcmFabricEgressYellowDropLimitBytes:
	if (arg > 0x7fff) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: Yellow Drop Limit Bytes range 0-0x7fff\n")));
	    rv = BCM_E_PARAM;
	    return rv;	
	}
	soc_reg_field_set(unit, FD_TOTAL_BUFFER_LIMIT_YELLOWr, &uData, TOTAL_BUFFER_LIMIT_YELLOWf, arg);
	SOC_IF_ERROR_RETURN(WRITE_FD_TOTAL_BUFFER_LIMIT_YELLOWr(unit, uData));
	break;
    case bcmFabricEgressRedDropLimitBytes:
	if (arg > 0x7fff) {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "ERROR: Red Drop Limit Bytes range 0-0x7fff\n")));
	    rv = BCM_E_PARAM;
	    return rv;	
	}
	soc_reg_field_set(unit, FD_TOTAL_BUFFER_LIMIT_REDr, &uData, TOTAL_BUFFER_LIMIT_REDf, arg);
	SOC_IF_ERROR_RETURN(WRITE_FD_TOTAL_BUFFER_LIMIT_REDr(unit, uData));
	break;
    case bcmFabricIngressLevel1NumSchedulers:
    case bcmFabricIngressLevel2NumSchedulers:
    case bcmFabricIngressLevel3NumSchedulers:
    case bcmFabricIngressLevel4NumSchedulers:
    case bcmFabricIngressLevel5NumSchedulers:
    case bcmFabricIngressLevel6NumSchedulers:
    case bcmFabricIngressLevel7NumSchedulers:
	if (type == bcmFabricIngressLevel7NumSchedulers) {
	    level = 7;
	} else if (type == bcmFabricIngressLevel6NumSchedulers) {
	    level = 6;
	} else if (type == bcmFabricIngressLevel5NumSchedulers) {
	    level = 5;
	} else if (type == bcmFabricIngressLevel4NumSchedulers) {
	    level = 4;
	} else if (type == bcmFabricIngressLevel3NumSchedulers) {
	    level = 3;
	} else if (type == bcmFabricIngressLevel2NumSchedulers) {
	    level = 2;
	} else {
	    level = 1;
	}
	
	rv = soc_sirius_ts_level_config(unit, level, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					arg, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					SOC_SIRIUS_API_PARAM_NO_CHANGE);
	if (rv == SOC_E_NONE) {
	    /* update software state so that logical scheduler could be allocated properly */
	    SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] = arg;
	}
	break;
    case bcmFabricIngressLevel1SchedulerUpdateCycles:
    case bcmFabricIngressLevel2SchedulerUpdateCycles:
    case bcmFabricIngressLevel3SchedulerUpdateCycles:
    case bcmFabricIngressLevel4SchedulerUpdateCycles:
    case bcmFabricIngressLevel5SchedulerUpdateCycles:
    case bcmFabricIngressLevel6SchedulerUpdateCycles:
    case bcmFabricIngressLevel7SchedulerUpdateCycles:
	if (type == bcmFabricIngressLevel7SchedulerUpdateCycles) {
	    level = 7;
	} else if (type == bcmFabricIngressLevel6SchedulerUpdateCycles) {
	    level = 6;
	} else if (type == bcmFabricIngressLevel5SchedulerUpdateCycles) {
	    level = 5;
	} else if (type == bcmFabricIngressLevel4SchedulerUpdateCycles) {
	    level = 4;
	} else if (type == bcmFabricIngressLevel3SchedulerUpdateCycles) {
	    level = 3;
	} else if (type == bcmFabricIngressLevel2SchedulerUpdateCycles) {
	    level = 2;
	} else {
	    level = 1;
	}

	rv = soc_sirius_ts_level_config(unit, level, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					SOC_SIRIUS_API_PARAM_NO_CHANGE, arg,
					SOC_SIRIUS_API_PARAM_NO_CHANGE);
	break;
    default:
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Unsupported fabric control type (%d) \n"),
                   type));
        rv = BCM_E_PARAM;
        break;
    }

    return rv;
}

int
bcm_sirius_fabric_control_get(int unit,
                              bcm_fabric_control_t type,
                              int *arg)
{
    int             rv = BCM_E_NONE;
    uint32          uData;
    bcm_multicast_t group;
    int             source_knockout, level;
    uint32          reg_value;
    int32           local_shape_inc, local_shape_start, local_shape_end;
    int32           fic_shape_inc, fic_shape_start, fic_shape_end;

    switch (type) {
    case bcmFabricActiveId:
        BCM_IF_ERROR_RETURN(READ_SC_STATUS0r(unit, &uData));
        *arg = soc_reg_field_get(unit, SC_STATUS0r, uData, SELECTED_BMf);
        break;
    case bcmFabricActiveArbiterId:
        BCM_IF_ERROR_RETURN(READ_SC_CONFIG0r(unit, &uData));
        *arg = soc_reg_field_get(unit, SC_CONFIG0r, uData, DEFAULT_BMf);
        break;
    case bcmFabricMcGroupSourceKnockout:
        group = (*arg) & BCM_FABRIC_MC_GROUP_MASK;
        /* coverity[uninit_use_in_call] */
        rv = bcm_sirius_multicast_source_knockout_get(unit, group, 
                                                      &source_knockout);
        if (rv == BCM_E_NONE) {
            (*arg) &= BCM_FABRIC_MC_GROUP_MASK;
            (*arg) |= ((source_knockout == TRUE) ? 
                       BCM_FABRIC_MC_GROUP_SOURCE_KNOCKOUT_ENABLE : 0);
        }
        break;
    case bcmFabricMode:
      if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	*arg = bcmFabricModeTME;
      } else if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
	*arg = bcmFabricModeFabric;
      } else {
	rv = BCM_E_INTERNAL;
      }
      break;
    case bcmFabricRedundancyMode:
        /* return cached value */
        *arg = SOC_SBX_CFG(unit)->uRedMode;
        break;
    case bcmFabricQueueMin: /* intentional fall through */
    case bcmFabricQueueMax: /* intentional fall through */
    case bcmFabricEgressQueueMin: /* intentional fall through */
    case bcmFabricEgressQueueMax:
        rv = _bcm_sirius_fabric_control_queue_set_get(unit, type, arg, 0);
        break;
    case bcmFabricSubscriberCosLevels:
	*arg = SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos;
	break;
    case bcmFabricSubscriberCosLevelAllocation:
	if ( SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize == TRUE) {
	    *arg = 4;
	} else {
	    *arg = 8;
	}
	break;
    case bcmFabricIngressLevel1NumSchedulers:
    case bcmFabricIngressLevel2NumSchedulers:
    case bcmFabricIngressLevel3NumSchedulers:
    case bcmFabricIngressLevel4NumSchedulers:
    case bcmFabricIngressLevel5NumSchedulers:
    case bcmFabricIngressLevel6NumSchedulers:
    case bcmFabricIngressLevel7NumSchedulers:
	if (type == bcmFabricIngressLevel7NumSchedulers) {
	    level = 7;
	} else if (type == bcmFabricIngressLevel6NumSchedulers) {
	    level = 6;
	} else if (type == bcmFabricIngressLevel5NumSchedulers) {
	    level = 5;
	} else if (type == bcmFabricIngressLevel4NumSchedulers) {
	    level = 4;
	} else if (type == bcmFabricIngressLevel3NumSchedulers) {
	    level = 3;
	} else if (type == bcmFabricIngressLevel2NumSchedulers) {
	    level = 2;
	} else {
	    level = 1;
	}
	
	*arg = SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level];
	break;
    case bcmFabricIngressLevel1SchedulerUpdateCycles:
    case bcmFabricIngressLevel2SchedulerUpdateCycles:
    case bcmFabricIngressLevel3SchedulerUpdateCycles:
    case bcmFabricIngressLevel4SchedulerUpdateCycles:
    case bcmFabricIngressLevel5SchedulerUpdateCycles:
    case bcmFabricIngressLevel6SchedulerUpdateCycles:
    case bcmFabricIngressLevel7SchedulerUpdateCycles:
	if (type == bcmFabricIngressLevel7SchedulerUpdateCycles) {
	    level = 7;
	} else if (type == bcmFabricIngressLevel6SchedulerUpdateCycles) {
	    level = 6;
	} else if (type == bcmFabricIngressLevel5SchedulerUpdateCycles) {
	    level = 5;
	} else if (type == bcmFabricIngressLevel4SchedulerUpdateCycles) {
	    level = 4;
	} else if (type == bcmFabricIngressLevel3SchedulerUpdateCycles) {
	    level = 3;
	} else if (type == bcmFabricIngressLevel2SchedulerUpdateCycles) {
	    level = 2;
	} else {
	    level = 1;
	}

	switch (level) {
	    case 1:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL1_CONFIG0r(unit, &reg_value));
                *arg = soc_reg_field_get(unit, TS_LEVEL1_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	    case 2:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL2_CONFIG0r(unit, &reg_value));
		*arg = soc_reg_field_get(unit, TS_LEVEL2_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	    case 3:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL3_CONFIG0r(unit, &reg_value));
		*arg = soc_reg_field_get(unit, TS_LEVEL3_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	    case 4:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL4_CONFIG0r(unit, &reg_value));
		*arg = soc_reg_field_get(unit, TS_LEVEL4_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	    case 5:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL5_CONFIG0r(unit, &reg_value));
		*arg = soc_reg_field_get(unit, TS_LEVEL5_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	    case 6:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL6_CONFIG0r(unit, &reg_value));
		*arg = soc_reg_field_get(unit, TS_LEVEL6_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	    case 7:
		SOC_IF_ERROR_RETURN(READ_TS_LEVEL7_CONFIG0r(unit, &reg_value));
		*arg = soc_reg_field_get(unit, TS_LEVEL7_CONFIG0r, reg_value, LEAK_CYCLESf);
		break;
	}
	break;
    case bcmFabricArbitrationMapFabric:
    case bcmFabricArbitrationMapSubscriber:
    case bcmFabricArbitrationMapHierarchicalSubscriber:
        rv = BCM_E_UNAVAIL;
        break;


    case bcmFabricShaperEgressQueueMin:
        if (!( soc_feature(unit, soc_feature_standalone) ||
               soc_feature(unit, soc_feature_hybrid) ||
               soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &reg_value));
        local_shape_start = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, reg_value, STARTQUEUEf);
        (*arg) = local_shape_start;
        break;

    case bcmFabricShaperEgressQueueMax:
        if (!( soc_feature(unit, soc_feature_standalone) ||
               soc_feature(unit, soc_feature_hybrid) ||
               soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, &reg_value));
        local_shape_end = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_ENDr, reg_value, ENDQUEUEf);
        (*arg) = local_shape_end;
        break;

    case bcmFabricShaperEgressQueueIncrement:
        if (!( soc_feature(unit, soc_feature_standalone) ||
               soc_feature(unit, soc_feature_hybrid) ||
               soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &reg_value));
        local_shape_inc = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, reg_value, INCf);
        (*arg) = (local_shape_inc == 0) ? 1 : 4;
        break;

    case bcmFabricShaperQueueMin:
        if (!( !(soc_feature(unit, soc_feature_standalone)) ||
                 soc_feature(unit, soc_feature_hybrid) ||
                 soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, &reg_value));
        fic_shape_start = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, reg_value, STARTQUEUEf);
        (*arg) = fic_shape_start;
        break;

    case bcmFabricShaperQueueMax:
        if (!( !(soc_feature(unit, soc_feature_standalone)) ||
                 soc_feature(unit, soc_feature_hybrid) ||
                 soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_ENDr(unit, &reg_value));
        fic_shape_end = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_ENDr, reg_value, ENDQUEUEf);
        (*arg) = fic_shape_end;
        break;

    case bcmFabricShaperQueueIncrement:
        if (!( !(soc_feature(unit, soc_feature_standalone)) ||
                 soc_feature(unit, soc_feature_hybrid) ||
                 soc_feature(unit, soc_feature_node_hybrid) )) {
            return(BCM_E_PARAM);
        }

        SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, &reg_value));
        fic_shape_inc = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, reg_value, INCf);
        (*arg) = (fic_shape_inc == 0) ? 1 : 4;
        break;

    case bcmFabricTsApplicationHierachySetup:
	if ((SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) ||
	    (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == FALSE)) {
	    (*arg) = SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode;
	} else {
	    LOG_ERROR(BSL_LS_BCM_FABRIC,
	              (BSL_META_U(unit,
	                          "scheduler resource managing mode have not been set yet\n")));
	    return(BCM_E_PARAM);
	}
	break;

    case bcmFabricMaxPorts:
        *arg = SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts;
	break;
	  
    case bcmFabricEgressDropLimitBytes:
	BCM_IF_ERROR_RETURN(READ_FD_TOTAL_BUFFER_LIMITr(unit, &uData));
	*arg = soc_reg_field_get(unit, FD_TOTAL_BUFFER_LIMITr, uData, TOTAL_BUFFER_LIMITf);
	break;

    case bcmFabricEgressYellowDropLimitBytes:
	SOC_IF_ERROR_RETURN(READ_FD_TOTAL_BUFFER_LIMIT_YELLOWr(unit, &uData));
	*arg = soc_reg_field_get(unit, FD_TOTAL_BUFFER_LIMIT_YELLOWr, uData, TOTAL_BUFFER_LIMIT_YELLOWf);
	break;
	    
    case bcmFabricEgressRedDropLimitBytes:
	SOC_IF_ERROR_RETURN(READ_FD_TOTAL_BUFFER_LIMIT_REDr(unit, &uData));
	*arg = soc_reg_field_get(unit, FD_TOTAL_BUFFER_LIMIT_REDr, uData, TOTAL_BUFFER_LIMIT_REDf);
	break;
    default:
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Unsupported fabric control type (%d) \n"),
                   type));
        rv = BCM_E_PARAM;
        break;
    }

    return rv;
}

/* Create or update a fabric predicate */
int
bcm_sirius_fabric_predicate_create(int unit,
                                   bcm_fabric_predicate_info_t *pred_info,
                                   bcm_fabric_predicate_t *pred_id)
{
    int meta = ~0;
    int range;
    unsigned int offset = ~0;
    unsigned int mask_low;
    unsigned int data_high;
    unsigned int pid = ~0;
    uint32 flags = 0;
    int result;

    if (!pred_info) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Unable to create predicate with NULL descriptor\n")));
        return BCM_E_PARAM;
    }
    if (!pred_id) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "NULL pred_id\n")));
        return BCM_E_PARAM;
    }
    if (((pred_info->flags & (BCM_FABRIC_PREDICATE_INFO_INGRESS |
                              BCM_FABRIC_PREDICATE_INFO_EGRESS)) ==
         (BCM_FABRIC_PREDICATE_INFO_INGRESS |
          BCM_FABRIC_PREDICATE_INFO_EGRESS)) ||
        (0 == (pred_info->flags & (BCM_FABRIC_PREDICATE_INFO_INGRESS |
                                   BCM_FABRIC_PREDICATE_INFO_EGRESS)))) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "predicate must be either ingress or egress\n")));
        return BCM_E_PARAM;
    }
    /*
     *  Find an available predicate (or use the specified one)
     */
    if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_INGRESS) {
        flags |= SIRIUS_PREDICATE_FLAGS_INGRESS;
    }
    if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_EGRESS) {
        flags |= SIRIUS_PREDICATE_FLAGS_EGRESS;
    }
    if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_WITH_ID) {
        flags |= SIRIUS_PREDICATE_FLAGS_WITH_ID;
        pid = *pred_id;
    }
    if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_REPLACE) {
        flags |= SIRIUS_PREDICATE_FLAGS_REPLACE;
    }
    result = soc_sirius_predicate_allocate(unit,
                                           flags,
                                           &pid);
    if (SOC_E_NONE != result) {
        /* unable to allocate predicate */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate predicate on unit %d: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_INGRESS) {
        /* validate ingress predicate settings */
        switch (pred_info->source) {
        case bcmFabricPredicateTypePacket:
            /* predicate uses data from frame or header */
            meta = FALSE;
            if (16 != pred_info->field.length) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "ingress predicates must consider 16 bits\n")));
                result = BCM_E_PARAM;
                break;
            }
            if (0 != (pred_info->field.offset & 7)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "ingress predicates must be byte aligned\n")));
                result = BCM_E_PARAM;
                break;
            }
            offset = pred_info->field.offset >> 3;
            if (1 > offset) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "ingress predicate must not start before header\n")));
                result = BCM_E_PARAM;
                break;
            }
            /* hardware uses high byte as pivot point, BCM uses low */
            offset--;
            break;
        case bcmFabricPredicateTypeInterface:
            /* predicate uses interface number */
            meta = TRUE;
            offset = 0;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unsupported source %d for ingress predicate\n"),
                       pred_info->source));
            result = BCM_E_PARAM;
        }
    } /* if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_INGRESS) */
    if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_EGRESS) {
        /* validate egress predicate settings */
        switch (pred_info->source) {
        case bcmFabricPredicateTypePacket:
            /* predicate uses data from frame or header */
            meta = FALSE;
            if (16 != pred_info->field.length) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "egress predicates must consider 16 bits\n")));
                result = BCM_E_PARAM;
                break;
            }
            if (0 != (pred_info->field.offset & 7)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "egress predicates must be byte aligned\n")));
                result = BCM_E_PARAM;
                break;
            }
            offset = pred_info->field.offset >> 3;
            if ((1 > offset) ||
                (0x3E < offset)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "egress predicate must start in bytes 1 through"
                                       " 62\n")));
                result = BCM_E_PARAM;
            }
            /* hardware uses high byte as pivot point, BCM uses low */
            offset--;
            break;
        case bcmFabricPredicateTypeSysport:
            /* predicate uses sysport from fabric */
            meta = TRUE;
            offset = 1;
            break;
        case bcmFabricPredicateTypeChannel:
            /* predicate uses channel ID */
            meta = TRUE;
            offset = 2;
            break;
        case bcmFabricPredicateTypeFifo:
            /* predicate uses FIFO number */
            meta = TRUE;
            offset = 3;
            break;
        case bcmFabricPredicateTypeInterface:
            /* predicate uses interface number */
            meta = TRUE;
            offset = 4;
            break;
        case bcmFabricPredicateTypeSubport:
            /* predicate uses subport number */
            meta = TRUE;
            offset = 5;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unsupported source %d for egress predicate\n"),
                       pred_info->source));
            result = BCM_E_PARAM;
        }
    } /* if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_EGRESS) */
    if (BCM_E_NONE == result) {
        if (meta) {
            if ((0 != pred_info->field.length) ||
                (0 != pred_info->field.offset)) {
                LOG_WARN(BSL_LS_BCM_FABRIC,
                         (BSL_META_U(unit,
                                     "ignoring offset+length for metadatum predicate\n")));
            }
        }
        if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_RANGE) {
            range = TRUE;
            mask_low = pred_info->range_low;
            data_high = pred_info->range_high;
        } else {
            range = FALSE;
            mask_low = pred_info->mask;
            data_high = pred_info->data;
        }
        result = soc_sirius_predicate_set(unit,
                                          pid,
                                          offset,
                                          meta,
                                          range,
                                          mask_low,
                                          data_high);
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE != result) {
        /* something went wrong after allocation */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to write unit %d predicate %d: %d (%s)\n"),
                   unit,
                   pid,
                   result,
                   _SHR_ERRMSG(result)));
        if (0 == (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_REPLACE)) {
            /* did not replace, so new alloc; must free it */
            meta = soc_sirius_predicate_free(unit, pid);
            if (SOC_E_NONE != meta) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to release unit %d predicate %d after error:"
                                       " %d (%s)\n"),
                           unit,
                           pid,
                           meta,
                           _SHR_ERRMSG(meta)));
            }
        }
    } else { /* if (BCM_E_NONE != result) */
        /* okay, return predicate ID */
        *pred_id = pid;
    } /* if (BCM_E_NONE != result) */
    return BCM_E_NONE;
}

/* Destroys an existing predicate */
int
bcm_sirius_fabric_predicate_destroy(int unit,
                                    bcm_fabric_predicate_t pred_id)
{
    int result;

    /* free the chosen predicate */
    result = soc_sirius_predicate_free(unit, pred_id);
    if (SOC_E_NONE == result) {
        /* ensure the predicate does not match (impossible condition) */
        result = soc_sirius_predicate_set(unit,
                                          pred_id,
                                          0,
                                          FALSE,
                                          FALSE,
                                          0x0000,
                                          0xFFFF);
        if (SOC_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unable to destroy unit %d freed predicate %d: %d (%s)\n"),
                       unit,
                       pred_id,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } else {
        /* unable to free the predicate */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to free unit %d predicate %d: %d (%s)\n"),
                   unit,
                   pred_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* Destroys all fabric predicates on the unit */
int
bcm_sirius_fabric_predicate_destroy_all(int unit)
{
    int result;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_predicate_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            LOG_VERBOSE(BSL_LS_BCM_FABRIC,
                        (BSL_META_U(unit,
                                    "destroy unit %d predicate %d\n"),
                         unit,
                         pid));
            result = bcm_sirius_fabric_predicate_destroy(unit, pid);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to destroy unit %d predicate %d: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d predicate after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while (SOC_E_NOT_FOUND != result);
    return BCM_E_NONE;
}

/* Retrieve the information about a specific predicate */
int
bcm_sirius_fabric_predicate_get(int unit,
                                bcm_fabric_predicate_t pred_id,
                                bcm_fabric_predicate_info_t *pred_info)
{
    int result;
    unsigned int offset;
    int meta;
    int range;
    unsigned int mask_low;
    unsigned int data_high;

    if (!pred_info) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "NULL pointer to obligatory out argument pred_info\n")));
        return BCM_E_PARAM;
    }
    if (pred_id < 0) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d pred_id %d is not valid\n"),
                   unit, pred_id));
        return BCM_E_NOT_FOUND;
    }
    result = soc_sirius_predicate_check(unit, pred_id);
    if (SOC_E_EMPTY == result) {
        /* predicate has not been allocated */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d predicate %d is not in use\n"),
                   unit,
                   pred_id));
        return BCM_E_NOT_FOUND;
    } else if (SOC_E_FULL != result) {
        /* something unexpected went wrong */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to verify unit %d predicate %d: %d (%s)\n"),
                   unit,
                   pred_id,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    result = soc_sirius_predicate_get(unit,
                                      pred_id,
                                      &offset,
                                      &meta,
                                      &range,
                                      &mask_low,
                                      &data_high);
    if (SOC_E_NONE == result) {
        bcm_fabric_predicate_info_t_init(unit, pred_info);
        if ((pred_id >= SIRIUS_PREDICATE_OFFSET_INGRESS) &&
            (pred_id < (SIRIUS_PREDICATE_OFFSET_INGRESS +
                       SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES))) {
            /* predicate is ingress */
            pred_info->flags = BCM_FABRIC_PREDICATE_INFO_INGRESS;
            if (meta) {
                pred_info->source = bcmFabricPredicateTypeInterface;
                pred_info->field.offset = 0;
                pred_info->field.length = 0;
            } else {
                pred_info->source = bcmFabricPredicateTypePacket;
                pred_info->field.offset = (offset + 1) * 8;
                pred_info->field.length = 16;
            }
            if (range) {
                pred_info->flags |= BCM_FABRIC_PREDICATE_INFO_RANGE;
                pred_info->range_high = data_high;
                pred_info->range_low = mask_low;
            } else {
                pred_info->data = data_high;
                pred_info->mask = mask_low;
            }
        } else if ((pred_id >= SIRIUS_PREDICATE_OFFSET_EGRESS) &&
                   (pred_id < (SIRIUS_PREDICATE_OFFSET_EGRESS +
                              SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES))) {
            /* predicate is egress */
            pred_info->flags = BCM_FABRIC_PREDICATE_INFO_EGRESS;
            if (meta) {
                switch (offset) {
                case 1:
                    pred_info->source = bcmFabricPredicateTypeSysport;
                    break;
                case 2:
                    pred_info->source = bcmFabricPredicateTypeChannel;
                    break;
                case 3:
                    pred_info->source = bcmFabricPredicateTypeFifo;
                    break;
                case 4:
                    pred_info->source = bcmFabricPredicateTypeInterface;
                    break;
                case 5:
                    pred_info->source = bcmFabricPredicateTypeSubport;
                    break;
                default:
                    /* should not be possible; for Coverity et al */
                    pred_info->source = bcmFabricPredicateTypePacket;
                }
            } else {
                pred_info->source = bcmFabricPredicateTypePacket;
            }
            if (bcmFabricPredicateTypePacket == pred_info->source) {
                pred_info->field.offset = (offset + 1) * 8;
                pred_info->field.length = 16;
            } else {
                pred_info->field.offset = 0;
                pred_info->field.length = 0;
            }
            if (range) {
                pred_info->flags |= BCM_FABRIC_PREDICATE_INFO_RANGE;
                pred_info->range_high = data_high;
                pred_info->range_low = mask_low;
            } else {
                pred_info->data = data_high;
                pred_info->mask = mask_low;
            }
        } else {
            /* neither ingress nor egress; something went wrong */
            result = BCM_E_INTERNAL;
        }
    } else { /* if (SOC_E_NONE == result) */
        /* unable to read the predicate */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to read unit %d predicate %d: %d (%s)\n"),
                   unit,
                   pred_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 * Traverse the existing predicates, and invoke an application provided
 * callback for each one
 */
int
bcm_sirius_fabric_predicate_traverse(int unit,
                                     bcm_fabric_predicate_traverse_cb cb,
                                     void *user_data)
{
    int result;
    int cbRes = BCM_E_NONE;
    bcm_fabric_predicate_info_t pred_info;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_predicate_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            result = bcm_sirius_fabric_predicate_get(unit,
                                                     pid,
                                                     &pred_info);
            if (BCM_E_NONE == result) {
                
                cbRes = (*cb)(unit, pid, &pred_info, user_data);
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to get unit %d predicate %d: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d predicate after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while ((SOC_E_NOT_FOUND != result) && (BCM_SUCCESS(cbRes)));
    return cbRes;
}

static int
bcm_sirius_fabric_action_egress_source_to_int(int unit,
                                              bcm_fabric_predicate_type_t predType,
                                              unsigned int *source)
{
    switch (predType) {
    case bcmFabricPredicateTypePacket:
        *source = 0;
        break;
    case bcmFabricPredicateTypeSysport:
        *source = 1;
        break;
    case bcmFabricPredicateTypeChannel:
        *source = 2;
        break;
    case bcmFabricPredicateTypeFifo:
        *source = 3;
        break;
    case bcmFabricPredicateTypeInterface:
        *source = 4;
        break;
    case bcmFabricPredicateTypeSubport:
        *source = 5;
        break;
    case bcmFabricPredicateTypePacketClass:
        *source = 6;
        break;
    case bcmFabricPredicateTypeFlags:
        *source = 7;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d egress does not support source %d\n"),
                   unit,
                   predType));
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

static int
bcm_sirius_fabric_action_int_to_egress_source(int unit,
                                              unsigned int source,
                                              bcm_fabric_predicate_type_t *predType)
{
    switch (source) {
    case 0:
        *predType = bcmFabricPredicateTypePacket;
        break;
    case 1:
        *predType = bcmFabricPredicateTypeSysport;
        break;
    case 2:
        *predType = bcmFabricPredicateTypeChannel;
        break;
    case 3:
        *predType = bcmFabricPredicateTypeFifo;
        break;
    case 4:
        *predType = bcmFabricPredicateTypeInterface;
        break;
    case 5:
        *predType = bcmFabricPredicateTypeSubport;
        break;
    case 6:
        *predType = bcmFabricPredicateTypePacketClass;
        break;
    case 7:
        *predType = bcmFabricPredicateTypeFlags;
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d egress does not support source %d\n"),
                   unit,
                   source));
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/* Create or update a fabric action */
int
bcm_sirius_fabric_action_create(int unit,
                                bcm_fabric_action_info_t *action_info,
                                bcm_fabric_action_t *action_id)
{
    unsigned int pid = ~0;
    unsigned int temp;
    uint32 flags = 0;
    int result;
    int auxres;
    soc_sirius_parser_info_t parserInfo;

    if (!action_info) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Unable to create action with NULL descriptor\n")));
        return BCM_E_PARAM;
    }
    if (!action_id) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "NULL action_id\n")));
        return BCM_E_PARAM;
    }
    if (((action_info->flags & (BCM_FABRIC_ACTION_INFO_INGRESS |
                                BCM_FABRIC_ACTION_INFO_EGRESS)) ==
         (BCM_FABRIC_ACTION_INFO_INGRESS |
          BCM_FABRIC_ACTION_INFO_EGRESS)) ||
        (0 == (action_info->flags & (BCM_FABRIC_ACTION_INFO_INGRESS |
                                     BCM_FABRIC_ACTION_INFO_EGRESS)))) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "action must be either ingress or egress\n")));
        return BCM_E_PARAM;
    }
    /*
     *  Find an available predicate (or use the specified one)
     */
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_INGRESS) {
        flags |= SIRIUS_PARSER_FLAGS_INGRESS;
    }
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_EGRESS) {
        flags |= SIRIUS_PARSER_FLAGS_EGRESS;
    }
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_WITH_ID) {
        flags |= SIRIUS_PARSER_FLAGS_WITH_ID;
        pid = *action_id;
    }
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_REPLACE) {
        flags |= SIRIUS_PARSER_FLAGS_REPLACE;
    }
    result = soc_sirius_parser_allocate(unit,
                                        flags,
                                        &pid);
    if (SOC_E_NONE != result) {
        /* unable to allocate predicate */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate action on unit %d: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    sal_memset(&parserInfo, 0x00, sizeof(parserInfo));
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_INGRESS) {
        /* validate ingress parser settings */
        parserInfo.ingress.flags = flags;
        if (17 > action_info->queue_field_low.length) {
            parserInfo.ingress.queue_length0 = action_info->queue_field_low.length;
            if (parserInfo.ingress.queue_length0) {
                if (504 > action_info->queue_field_low.offset) {
                    parserInfo.ingress.queue_byte0 = (action_info->queue_field_low.offset >> 3);
                    parserInfo.ingress.queue_bit0 = (action_info->queue_field_low.offset & 7);
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d queue_field_low.offset %d, must be 0..503\n"),
                               unit,
                               action_info->queue_field_low.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_field_low.length %d, must be 0..16\n"),
                       unit,
                       action_info->queue_field_low.length));
            result = BCM_E_PARAM;
        }
        if (16 > action_info->queue_field_high.length) {
            parserInfo.ingress.queue_length1 = action_info->queue_field_high.length;
            if (parserInfo.ingress.queue_length1) {
                if (504 > action_info->queue_field_high.offset) {
                    parserInfo.ingress.queue_byte1 = (action_info->queue_field_high.offset >> 3);
                    parserInfo.ingress.queue_bit1 = (action_info->queue_field_high.offset & 7);
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d queue_field_high.offset %d, must be 0..503\n"),
                               unit,
                               action_info->queue_field_high.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_field_high.length %d, must be 0..15\n"),
                       unit,
                       action_info->queue_field_high.length));
            result = BCM_E_PARAM;
        }
        if (16 < (action_info->queue_field_low.length +
                  action_info->queue_field_high.length)) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d low and high queue length must be 0..16\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        parserInfo.ingress.cosProfile = action_info->qsel_offset;
        parserInfo.ingress.segmentSel = action_info->qsel;
        if ((504 > action_info->qsel_offset_index_field.offset) &&
            (5 > (action_info->qsel_offset_index_field.offset & 7))) {
            parserInfo.ingress.cos_byte = (action_info->qsel_offset_index_field.offset >> 3);
            parserInfo.ingress.cos_bit = (action_info->qsel_offset_index_field.offset & 7);
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d qsel_offset_index_field.offset %d, must be"
                                   " 0..503 and must exist in a single byte\n"),
                       unit,
                       action_info->qsel_offset_index_field.offset));
            result = BCM_E_PARAM;
        }
        if (4 == action_info->qsel_offset_index_field.length) {
            parserInfo.ingress.flags |= SIRIUS_PARSER_FLAGS_USE_COS_MAP;
        } else if (0 != action_info->qsel_offset_index_field.length) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d qsel_offset_index_field.length %d, must be 0 to"
                                   " disable qsel_offset use, or 4 to enable it\n"),
                       unit,
                       action_info->qsel_offset_index_field.length));
            result = BCM_E_PARAM;
        }
        if (4 > action_info->override_dp) {
            parserInfo.ingress.dp = action_info->override_dp;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d override_dp %d, must be negative to disable or 0..3\n"),
                       unit,
                       action_info->override_dp));
            result = BCM_E_PARAM;
        }
        if (4 > action_info->override_ecn) {
            parserInfo.ingress.ecn = action_info->override_ecn;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d override_ecn %d, must be negative to disable or 0..3\n"),
                       unit,
                       action_info->override_ecn));
            result = BCM_E_PARAM;
        }
        if (16 > action_info->stat0_field_low.length) {
            parserInfo.ingress.stat_length0 = action_info->stat0_field_low.length;
            if (parserInfo.ingress.stat_length0) {
                if (504 > action_info->stat0_field_low.offset) {
                    parserInfo.ingress.stat_byte0 = (action_info->stat0_field_low.offset >> 3);
                    parserInfo.ingress.stat_bit0 = (action_info->stat0_field_low.offset & 7);
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d stat0_field_low.offset %d, must be 0..503\n"),
                               unit,
                               action_info->stat0_field_low.offset));
                    result = BCM_E_PARAM;
                }
                if (action_info->stat0_field_low_type != bcmFabricPredicateTypePacket) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d only support ingress stats from frame data\n"),
                               unit));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat0_field_low.length %d, must be 0..15\n"),
                       unit,
                       action_info->stat0_field_low.length));
            result = BCM_E_PARAM;
        }
        if (16 > action_info->stat0_field_high.length) {
            parserInfo.ingress.stat_length1 = action_info->stat0_field_high.length;
            if (parserInfo.ingress.stat_length1) {
                if (action_info->stat0_segment) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d does not support specific statistics"
                                           " segment selection on ingress\n"),
                               unit));
                    result = BCM_E_PARAM;
                }
                if (504 > action_info->stat0_field_high.offset) {
                    parserInfo.ingress.stat_byte1 = (action_info->stat0_field_high.offset >> 3);
                    parserInfo.ingress.stat_bit1 = (action_info->stat0_field_high.offset & 7);
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d stat0_field_high.offset %d, must be 0..503\n"),
                               unit,
                               action_info->stat0_field_high.offset));
                    result = BCM_E_PARAM;
                }
                if (action_info->stat0_field_high_type != bcmFabricPredicateTypePacket) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d only support ingress stats from frame data\n"),
                               unit));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat0_field_high.length %d, must be 0..15)\n"),
                       unit,
                       action_info->stat0_field_high.length));
            result = BCM_E_PARAM;
        }
        if ((action_info->stat0_field_low.length ||
             action_info->stat0_field_high.length) &&
            action_info->stat0_adjust) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat0_adjust not supported on ingress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (action_info->stat1_field_low.length ||
            action_info->stat1_field_high.length) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat1_field_* not supported on ingress\n"),
                       unit));
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_QUEUE_DEFAULT) {
            parserInfo.ingress.flags |= SIRIUS_PARSER_FLAGS_QUEUE_DEFAULT;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_USE_TAG_OFFSET) {
            parserInfo.ingress.flags |= SIRIUS_PARSER_FLAGS_USE_TAG_OFFSET;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_OVERWRITE_DEST) {
            parserInfo.ingress.flags |= SIRIUS_PARSER_FLAGS_OVERWRITE_DEST;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_LENGTH_ADJUST) {
            parserInfo.ingress.flags |= SIRIUS_PARSER_FLAGS_LENGTH_ADJUST;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_CLEAR_TEST_BIT) {
            parserInfo.ingress.flags |= SIRIUS_PARSER_FLAGS_CLEAR_TEST_BIT;
        }
        if (action_info->encap_id_field.length) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d encap_id write not supported on ingress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (action_info->queue_id_field.length) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d directed queue_id write not supported on ingress;"
                                   "use _OVERWRITE_DEST flag instead\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (0 <= action_info->clear_ehv_bit) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d clear_ehv_bit not supported on ingress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (0 <= action_info->clear_mp_bit) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d clear_mp_bit not supported on ingress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (0 <= action_info->four_byte_remove_offset) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d four_byte_remove not supported on ingress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (0 < action_info->header_bytes_remove) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d header_bytes_remove not supported on ingress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (action_info->flags & (~(BCM_FABRIC_ACTION_INFO_INGRESS |
                                    BCM_FABRIC_ACTION_INFO_WITH_ID |
                                    BCM_FABRIC_ACTION_INFO_REPLACE |
                                    BCM_FABRIC_ACTION_INFO_QUEUE_DEFAULT |
                                    BCM_FABRIC_ACTION_INFO_USE_TAG_OFFSET |
                                    BCM_FABRIC_ACTION_INFO_OVERWRITE_DEST |
                                    BCM_FABRIC_ACTION_INFO_LENGTH_ADJUST |
                                    BCM_FABRIC_ACTION_INFO_CLEAR_TEST_BIT))) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d flags %08X contains unsupported bits %08X\n"),
                       unit,
                       action_info->flags,
                       action_info->flags & (~(BCM_FABRIC_ACTION_INFO_INGRESS |
                       BCM_FABRIC_ACTION_INFO_WITH_ID |
                       BCM_FABRIC_ACTION_INFO_REPLACE |
                       BCM_FABRIC_ACTION_INFO_QUEUE_DEFAULT |
                       BCM_FABRIC_ACTION_INFO_USE_TAG_OFFSET |
                       BCM_FABRIC_ACTION_INFO_OVERWRITE_DEST |
                       BCM_FABRIC_ACTION_INFO_LENGTH_ADJUST |
                       BCM_FABRIC_ACTION_INFO_CLEAR_TEST_BIT))));
            result = BCM_E_PARAM;
        }
    } /* if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_INGRESS) */
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_EGRESS) {
        /* validate egress parser settings */
        parserInfo.egress.flags = flags;
        if (18 > action_info->stat0_field_low.length) {
            parserInfo.egress.stat0_length0 = action_info->stat0_field_low.length;
            if (parserInfo.egress.stat0_length0) {
                if (256 > action_info->stat0_field_low.offset) {
                    parserInfo.egress.stat0_offset0 = action_info->stat0_field_low.offset;
                    result = bcm_sirius_fabric_action_egress_source_to_int(unit,
                                                                              action_info->stat0_field_low_type,
                                                                              &temp);
                    if (BCM_E_NONE == result) {
                        parserInfo.egress.stat0_source0 = temp;
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d stat0_field_low.offset %d, must be 0..255\n"),
                               unit,
                               action_info->stat0_field_low.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat0_field_low.length %d, must be 0..17\n"),
                       unit,
                       action_info->stat0_field_low.length));
            result = BCM_E_PARAM;
        }
        if (18 > action_info->stat0_field_high.length) {
            parserInfo.egress.stat0_length1 = action_info->stat0_field_high.length;
            if (parserInfo.egress.stat0_length1) {
                if (256 > action_info->stat0_field_high.offset) {
                    parserInfo.egress.stat0_offset1 = action_info->stat0_field_high.offset;
                    result = bcm_sirius_fabric_action_egress_source_to_int(unit,
                                                                              action_info->stat0_field_high_type,
                                                                              &temp);
                    if (BCM_E_NONE == result) {
                        parserInfo.egress.stat0_source1 = temp;
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d stat0_field_high.offset %d, must be 0..255\n"),
                               unit,
                               action_info->stat0_field_high.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat0_field_high.length %d, must be 0..17\n"),
                       unit,
                       action_info->stat0_field_high.length));
            result = BCM_E_PARAM;
        }
        if (action_info->stat0_field_low.length +
            action_info->stat0_field_high.length) {
            if (32 > action_info->stat0_segment) {
                parserInfo.egress.stat0_segment = action_info->stat0_segment;
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d stat0_segment %d, must be 0..31\n"),
                           unit,
                           action_info->stat0_segment));
                result = BCM_E_PARAM;
            }
            if ((-128 < action_info->stat0_adjust) &&
                (128 > action_info->stat0_adjust)) {
                if (0 > action_info->stat0_adjust) {
                    temp = 0x80 | (-action_info->stat0_adjust);
                } else {
                    temp = action_info->stat0_adjust;
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d stat0_adjust %d, must be -127..127\n"),
                           unit,
                           action_info->stat0_adjust));
                result = BCM_E_PARAM;
            }
            if (17 < (action_info->stat0_field_low.length +
                      action_info->stat0_field_high.length)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d stat0 combined length %d, must be 0..17\n"),
                           unit,
                           action_info->stat0_field_low.length +
                           action_info->stat0_field_high.length));
                result = BCM_E_PARAM;
            }
            if (action_info->flags & BCM_FABRIC_ACTION_INFO_STAT0_ADJ_PER_PACKET) {
                parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_STAT0_ADJ_PER_PACKET;
            }
        }
        if (18 > action_info->stat1_field_low.length) {
            parserInfo.egress.stat1_length0 = action_info->stat1_field_low.length;
            if (parserInfo.egress.stat1_length0) {
                if (256 > action_info->stat1_field_low.offset) {
                    parserInfo.egress.stat1_offset0 = action_info->stat1_field_low.offset;
                    result = bcm_sirius_fabric_action_egress_source_to_int(unit,
                                                                              action_info->stat1_field_low_type,
                                                                              &temp);
                    if (BCM_E_NONE == result) {
                        parserInfo.egress.stat1_source0 = temp;
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d stat1_field_low.offset %d, must be 0..255\n"),
                               unit,
                               action_info->stat1_field_low.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat1_field_low.length %d, must be 0..17\n"),
                       unit,
                       action_info->stat1_field_low.length));
            result = BCM_E_PARAM;
        }
        if (18 > action_info->stat1_field_high.length) {
            parserInfo.egress.stat1_length1 = action_info->stat1_field_high.length;
            if (parserInfo.egress.stat1_length1) {
                if (256 > action_info->stat1_field_high.offset) {
                    parserInfo.egress.stat1_offset1 = action_info->stat1_field_high.offset;
                    result = bcm_sirius_fabric_action_egress_source_to_int(unit,
                                                                              action_info->stat1_field_high_type,
                                                                              &temp);
                    if (BCM_E_NONE == result) {
                        parserInfo.egress.stat1_source1 = temp;
                    }
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d stat1_field_high.offset %d, must be 0..255\n"),
                               unit,
                               action_info->stat1_field_high.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d stat1_field_high.length %d, must be 0..17\n"),
                       unit,
                       action_info->stat1_field_high.length));
            result = BCM_E_PARAM;
        }
        if (action_info->stat1_field_low.length +
            action_info->stat1_field_high.length) {
            if (32 > action_info->stat1_segment) {
                parserInfo.egress.stat1_segment = action_info->stat1_segment;
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d stat1_segment %d, must be 0..31\n"),
                           unit,
                           action_info->stat1_segment));
                result = BCM_E_PARAM;
            }
            if ((-128 < action_info->stat1_adjust) &&
                (128 > action_info->stat1_adjust)) {
                if (0 > action_info->stat1_adjust) {
                    temp = 0x80 | (-action_info->stat1_adjust);
                } else {
                    temp = action_info->stat1_adjust;
                }
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d stat1_adjust %d, must be -127..127\n"),
                           unit,
                           action_info->stat1_adjust));
                result = BCM_E_PARAM;
            }
            if (17 < (action_info->stat1_field_low.length +
                      action_info->stat1_field_high.length)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d stat1 combined length %d, must be 0..17\n"),
                           unit,
                           action_info->stat1_field_low.length +
                           action_info->stat1_field_high.length));
                result = BCM_E_PARAM;
            }
            if (action_info->flags & BCM_FABRIC_ACTION_INFO_STAT1_ADJ_PER_PACKET) {
                parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_STAT1_ADJ_PER_PACKET;
            }
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_STAT_COMBINE) {
            parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_STAT_COMBINE;
            if ((0 == (action_info->stat0_field_low.length +
                       action_info->stat0_field_high.length)) ||
                (0 == (action_info->stat1_field_low.length +
                       action_info->stat1_field_high.length))) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d requires both stat0 and stat1 enabled if"
                                       " combining stats\n"),
                           unit));
                result = BCM_E_PARAM;
            }
            if (action_info->stat0_segment !=
                action_info->stat1_segment) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d requires stat0 and stat1 segments equal"
                                       " when combining stats\n"),
                           unit));
                result = BCM_E_PARAM;
            }
            if (17 < (action_info->stat0_field_low.length +
                      action_info->stat0_field_high.length +
                      action_info->stat1_field_low.length +
                      action_info->stat1_field_high.length)) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d combined stat length %d, must be 0..17\n"),
                           unit,
                           action_info->stat0_field_low.length +
                           action_info->stat0_field_high.length +
                           action_info->stat1_field_low.length +
                           action_info->stat1_field_high.length));
                result = BCM_E_PARAM;
            }
        }
        if (25 > action_info->encap_id_field.length) {
            parserInfo.egress.oi_write_length = action_info->encap_id_field.length;
            if (parserInfo.egress.oi_write_length) {
                if ((248 > action_info->encap_id_field.offset) &&
                    (0 == (action_info->encap_id_field.offset & 7))) {
                    parserInfo.egress.oi_write_byte = action_info->encap_id_field.offset >> 3;
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d encap_id_field.offset %d, must be 0..240"
                                           " on exact byte boundaries\n"),
                               unit,
                               action_info->encap_id_field.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d encap_id_field.length %d, must be 0..24\n"),
                       unit,
                       action_info->encap_id_field.length));
            result = BCM_E_PARAM;
        }
        if (5 > action_info->len_adj_index_field.length) {
            parserInfo.egress.lengthAdj_length = action_info->len_adj_index_field.length;
            if (parserInfo.egress.lengthAdj_length) {
                if ((252 > action_info->len_adj_index_field.offset) &&
                    (0 == (action_info->len_adj_index_field.offset & 3))) {
                    parserInfo.egress.lengthAdj_nybble = action_info->len_adj_index_field.offset >> 2;
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d len_adj_index_field.offset %d, must be"
                                           "0.. 248 on exact nybble boundaries\n"),
                               unit,
                               action_info->len_adj_index_field.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d len_adj_index_field.length %d, must be 0..4\n"),
                       unit,
                       action_info->len_adj_index_field.length));
            result = BCM_E_PARAM;
        }
        switch (action_info->queue_id_source) {
        case bcmFabricActionQueueIdxFromPacket:
            parserInfo.egress.requeueSel = 0;
            break;
        case bcmFabricActionQueueIdxFromReplicantId:
            parserInfo.egress.requeueSel = 1;
            break;
        case bcmFabricActionQueueIdxFromReplicantLookup:
            parserInfo.egress.requeueSel = 2;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_id_source %d is invalid\n"),
                       unit,
                       action_info->queue_id_source));
            result = BCM_E_PARAM;
        }
        if (25 > action_info->queue_field_low.length) {
            parserInfo.egress.oi_read_length = action_info->queue_field_low.length;
            if (parserInfo.egress.oi_read_length) {
                if ((248 > action_info->queue_field_low.offset) &&
                    (0 == (action_info->queue_field_low.offset & 7))) {
                    parserInfo.egress.oi_read_byte = action_info->queue_field_low.offset >> 3;
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d queue_field_low.offset %d, must be 0..240,"
                                           " exactly aligned to a byte\n"),
                               unit,
                               action_info->queue_field_low.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_field_low.length %d, must be 0..24\n"),
                       unit,
                       action_info->queue_field_low.length));
            result = BCM_E_PARAM;
        }
        if ((131072 > action_info->queue_index_offset) &&
            (0 == (action_info->queue_index_offset & 0x1FF))) {
            parserInfo.egress.oi_index_offset = action_info->queue_index_offset >> 9;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_index_offset %d, must be 0..130560,"
                                   " exactly aligned to an increment of 512\n"),
                       unit,
                       action_info->queue_index_offset));
            result = BCM_E_PARAM;
        }
        if (8 > action_info->qsel_offset_index_field.length) {
            parserInfo.egress.fcos_length = action_info->qsel_offset_index_field.length;
            if (parserInfo.egress.fcos_length) {
                if (256 > action_info->qsel_offset_index_field.offset) {
                    parserInfo.egress.fcos_bit = action_info->qsel_offset_index_field.offset;
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d qsel_offset_index_field.offset %d, must be 0..255\n"),
                               unit,
                               action_info->qsel_offset_index_field.length));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d qsel_offset_index_field.length %d, must be 0..7\n"),
                       unit,
                       action_info->qsel_offset_index_field.length));
            result = BCM_E_PARAM;
        }
        if (256 > action_info->clear_ehv_bit) {
            parserInfo.egress.eh_bit = action_info->clear_ehv_bit;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d clear_ehv_bit %d, must be negative to disable"
                                   " or 0..255 to specify which bit to clear\n"),
                       unit,
                       action_info->clear_ehv_bit));
            result = BCM_E_PARAM;
        }
        if (25 > action_info->queue_id_field.length) {
            parserInfo.egress.queue_write_length = action_info->queue_id_field.length;
            if (parserInfo.egress.queue_write_length) {
                if ((256 > action_info->queue_id_field.offset) &&
                    (0 == (action_info->queue_id_field.offset & 7))) {
                    parserInfo.egress.queue_write_byte = action_info->queue_id_field.offset >> 3;
                } else {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unit %d queue_id_field.offset %d, must be 0..240"
                                           " on exact byte boundaries\n"),
                               unit,
                               action_info->queue_id_field.offset));
                    result = BCM_E_PARAM;
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_id_field.length %d, must be 0..24\n"),
                       unit,
                       action_info->queue_id_field.length));
            result = BCM_E_PARAM;
        }
        if (17 > action_info->header_bytes_remove) {
            parserInfo.egress.hdrRemove_bytes = action_info->header_bytes_remove;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d header_bytes_remove %d, must be 0..16\n"),
                       unit,
                       action_info->header_bytes_remove));
            result = BCM_E_PARAM;
        }
        if ((0 > action_info->four_byte_remove_offset) ||
            ((256 > action_info->four_byte_remove_offset) &&
             (0 == (action_info->four_byte_remove_offset & 0x1F)))) {
            parserInfo.egress.fourByteRemove_quadbyte = action_info->four_byte_remove_offset >> 5;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d four_byte_remove_offset %d, must be 0..224 on"
                                   "exact quadbyte boundaries\n"),
                       unit,
                       action_info->four_byte_remove_offset));
            result = BCM_E_PARAM;
        }
        if (256 > action_info->clear_mp_bit) {
            parserInfo.egress.mp_bit = action_info->clear_mp_bit;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d clear_mp_bit %d, must be negative to disable"
                                   " or 0..255 to specify bit to clear\n"),
                       unit,
                       action_info->clear_mp_bit));
            result = BCM_E_PARAM;
        }
        switch (action_info->insert_dest_select) {
        case bcmFabricActionInsertRepId16:
            temp = 0;
            break;
        case bcmFabricActionInsertRepId18:
            temp = 1;
            break;
        case bcmFabricActionInsertEncapId16:
            temp = 2;
            break;
        case bcmFabricActionInsertEncapId18:
            temp = 3;
            break;
        case bcmFabricActionInsertQueueIndex16:
            temp = 4;
            break;
        case bcmFabricActionInsertQueueIndex18:
            temp = 5;
            break;
        case bcmFabricActionInsertQueueId16:
            temp = 6;
            break;
        case bcmFabricActionInsertQueueId18:
            temp = 7;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d insert_dest_select is not valid\n"),
                       unit));
            temp = 0;
        }
        parserInfo.egress.insertSel = temp;
        if (action_info->queue_field_high.length) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue_field_high not supported on egress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (0 <= action_info->override_dp) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d override_dp not supported on egress; must be"
                                   " negative to disable\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (0 <= action_info->override_ecn) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d override_ecn not supported on egress; must be"
                                   " negative to disable\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (action_info->qsel) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d qsel not supported on egress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (action_info->qsel_offset) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d qsel_offset not supported on egress\n"),
                       unit));
            result = BCM_E_PARAM;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_LOOKUP_ENCAPID) {
            parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_LOOKUP_ENCAPID;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_CLEAR_MC_BIT) {
            parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_CLEAR_MC_BIT;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_LEN_ADJ_ON_SHAPING) {
            parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_LEN_ADJ_ON_SHAPING;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_LOOKUP_QUEUEID) {
            parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_LOOKUP_QUEUEID;
        }
        if (action_info->flags & BCM_FABRIC_ACTION_INFO_XGS_MODE) {
            parserInfo.egress.flags |= SIRIUS_PARSER_FLAGS_XGS_MODE;
        }
        if (action_info->flags & (~(BCM_FABRIC_ACTION_INFO_EGRESS |
                                    BCM_FABRIC_ACTION_INFO_WITH_ID |
                                    BCM_FABRIC_ACTION_INFO_REPLACE |
                                    BCM_FABRIC_ACTION_INFO_LOOKUP_ENCAPID |
                                    BCM_FABRIC_ACTION_INFO_CLEAR_MC_BIT |
                                    BCM_FABRIC_ACTION_INFO_LEN_ADJ_ON_SHAPING |
                                    BCM_FABRIC_ACTION_INFO_LOOKUP_QUEUEID |
                                    BCM_FABRIC_ACTION_INFO_XGS_MODE |
                                    BCM_FABRIC_ACTION_INFO_STAT0_ADJ_PER_PACKET |
                                    BCM_FABRIC_ACTION_INFO_STAT1_ADJ_PER_PACKET |
                                    BCM_FABRIC_ACTION_INFO_STAT_COMBINE))) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d flags %08X contains unsupported bits %08X\n"),
                       unit,
                       action_info->flags,
                       action_info->flags & (~(BCM_FABRIC_ACTION_INFO_EGRESS |
                       BCM_FABRIC_ACTION_INFO_WITH_ID |
                       BCM_FABRIC_ACTION_INFO_REPLACE |
                       BCM_FABRIC_ACTION_INFO_LOOKUP_ENCAPID |
                       BCM_FABRIC_ACTION_INFO_CLEAR_MC_BIT |
                       BCM_FABRIC_ACTION_INFO_LEN_ADJ_ON_SHAPING |
                       BCM_FABRIC_ACTION_INFO_LOOKUP_QUEUEID |
                       BCM_FABRIC_ACTION_INFO_XGS_MODE |
                       BCM_FABRIC_ACTION_INFO_STAT0_ADJ_PER_PACKET |
                       BCM_FABRIC_ACTION_INFO_STAT1_ADJ_PER_PACKET |
                       BCM_FABRIC_ACTION_INFO_STAT_COMBINE))));
            result = BCM_E_PARAM;
        }
    } /* if (pred_info->flags & BCM_FABRIC_PREDICATE_INFO_EGRESS) */
    if (BCM_E_NONE == result) {
        result = soc_sirius_parser_set(unit,
                                       pid,
                                       &parserInfo);
    } /* if (BCM_E_NONE == result) */
    if (BCM_E_NONE != result) {
        /* something went wrong after allocation */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to write unit %d action %d: %d (%s)\n"),
                   unit,
                   pid,
                   result,
                   _SHR_ERRMSG(result)));
        if (0 == (action_info->flags & BCM_FABRIC_ACTION_INFO_REPLACE)) {
            /* did not replace, so new alloc; must free it */
            auxres = soc_sirius_parser_free(unit, pid);
            if (SOC_E_NONE != auxres) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to release unit %d action %d after error:"
                                       " %d (%s)\n"),
                           unit,
                           pid,
                           auxres,
                           _SHR_ERRMSG(auxres)));
            }
        }
    } else { /* if (BCM_E_NONE != result) */
        /* okay, return action ID */
        *action_id = pid;
    } /* if (BCM_E_NONE != result) */
    return BCM_E_NONE;
}

/* Destroy a fabric action */
int
bcm_sirius_fabric_action_destroy(int unit,
                                 bcm_fabric_action_t action_id)
{
    int result;

    /* free the chosen action */
    result = soc_sirius_parser_free(unit, action_id);
    if (SOC_E_NONE != result) {
        /* unable to free the action */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to free unit %d action_id %d: %d (%s)\n"),
                   unit,
                   action_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* Destroy all fabric actions on the unit */
int
bcm_sirius_fabric_action_destroy_all(int unit)
{
    int result;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_parser_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            result = soc_sirius_parser_free(unit, pid);
            if (SOC_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to destroy unit %d action %d: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d action after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while (SOC_E_NOT_FOUND != result);
    return BCM_E_NONE;
}

/* Get information about a fabric action */
int
bcm_sirius_fabric_action_get(int unit,
                             bcm_fabric_action_t action_id,
                             bcm_fabric_action_info_t *action_info)
{
    int result;
    soc_sirius_parser_info_t parserInfo;

    if (!action_info) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "action_info argument must not be NULL for get\n")));
        return BCM_E_PARAM;
    }
    result = soc_sirius_parser_check(unit, action_id);
    if (SOC_E_EMPTY == result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d action %d is not in use\n"),
                   unit,
                   action_id));
        return BCM_E_NOT_FOUND;
    } else if (SOC_E_FULL != result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to access unit %d action %d: %d (%s)\n"),
                   unit,
                   action_id,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    result = soc_sirius_parser_get(unit,
                                   action_id,
                                   &parserInfo);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to read unit %d action %d: %d (%s)\n"),
                   unit,
                   action_id,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    sal_memset(action_info, 0x00, sizeof(*action_info));
    if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_INGRESS) {
        action_info->flags |= BCM_FABRIC_ACTION_INFO_INGRESS;
    }
    if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_EGRESS) {
        action_info->flags |= BCM_FABRIC_ACTION_INFO_EGRESS;
    }
    if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_WITH_ID) {
        action_info->flags |= BCM_FABRIC_ACTION_INFO_WITH_ID;
    }
    if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_REPLACE) {
        action_info->flags |= BCM_FABRIC_ACTION_INFO_REPLACE;
    }
    /* now the parsing diverges a bit... */
    if (action_info->flags & BCM_FABRIC_ACTION_INFO_INGRESS) {
        action_info->queue_field_low.length = parserInfo.ingress.queue_length0;
        action_info->queue_field_low.offset = ((parserInfo.ingress.queue_byte0 << 3) |
                                               parserInfo.ingress.queue_bit0);
        action_info->queue_field_high.length = parserInfo.ingress.queue_length1;
        action_info->queue_field_high.offset = ((parserInfo.ingress.queue_byte1 << 3) |
                                                parserInfo.ingress.queue_bit1);
        action_info->qsel_offset = parserInfo.ingress.cosProfile;
        action_info->qsel = parserInfo.ingress.segmentSel;
        if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_USE_COS_MAP) {
            action_info->qsel_offset_index_field.length = 4;
        } else {
            action_info->qsel_offset_index_field.length = 0;
        }
        action_info->qsel_offset_index_field.offset = ((parserInfo.ingress.cos_byte << 3) |
                                                       parserInfo.ingress.cos_bit);
        action_info->override_dp = parserInfo.ingress.dp;
        action_info->override_ecn = parserInfo.ingress.ecn;
        action_info->stat0_field_low.length = parserInfo.ingress.stat_length0;
        action_info->stat0_field_low.offset = ((parserInfo.ingress.stat_byte0 << 3) |
                                               parserInfo.ingress.stat_bit0);
        action_info->stat0_field_low_type = bcmFabricPredicateTypePacket;
        action_info->stat0_field_high.length = parserInfo.ingress.stat_length1;
        action_info->stat0_field_high.offset = ((parserInfo.ingress.stat_byte1 << 3) |
                                                parserInfo.ingress.stat_bit1);
        action_info->stat0_field_high_type = bcmFabricPredicateTypePacket;

        if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_QUEUE_DEFAULT) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_QUEUE_DEFAULT;
        }
        if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_USE_TAG_OFFSET) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_USE_TAG_OFFSET;
        }
        if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_OVERWRITE_DEST) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_OVERWRITE_DEST;
        }
        if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_LENGTH_ADJUST) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_LENGTH_ADJUST;
        }
        if (parserInfo.ingress.flags & SIRIUS_PARSER_FLAGS_CLEAR_TEST_BIT) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_CLEAR_TEST_BIT;
        }
        action_info->clear_ehv_bit = -1;
        action_info->clear_mp_bit = -1;
        action_info->four_byte_remove_offset = -1;
    } else if (action_info->flags & BCM_FABRIC_ACTION_INFO_EGRESS) {
        action_info->stat0_segment = parserInfo.egress.stat0_segment;
        action_info->stat0_field_low.length = parserInfo.egress.stat0_length0;
        action_info->stat0_field_low.offset = parserInfo.egress.stat0_offset0 << 3;
        result = bcm_sirius_fabric_action_int_to_egress_source(unit,
                                                               parserInfo.egress.stat0_source0,
                                                               &(action_info->stat0_field_low_type));
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d unable to resolve stat0_low type %d: %d (%s)\n"),
                       unit,
                       parserInfo.egress.stat0_source0,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        action_info->stat0_field_high.length = parserInfo.egress.stat0_length1;
        action_info->stat0_field_high.offset = parserInfo.egress.stat0_offset1 << 3;
        result = bcm_sirius_fabric_action_int_to_egress_source(unit,
                                                               parserInfo.egress.stat0_source1,
                                                               &(action_info->stat0_field_high_type));
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d unable to resolve stat0_high type %d: %d (%s)\n"),
                       unit,
                       parserInfo.egress.stat0_source1,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        action_info->stat0_adjust = parserInfo.egress.stat0_adjust;
        action_info->stat1_segment = parserInfo.egress.stat1_segment;
        action_info->stat1_field_low.length = parserInfo.egress.stat1_length0;
        action_info->stat1_field_low.offset = parserInfo.egress.stat1_offset0 << 3;
        result = bcm_sirius_fabric_action_int_to_egress_source(unit,
                                                               parserInfo.egress.stat1_source0,
                                                               &(action_info->stat1_field_low_type));
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d unable to resolve stat1_low type %d: %d (%s)\n"),
                       unit,
                       parserInfo.egress.stat1_source0,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        action_info->stat1_field_high.length = parserInfo.egress.stat1_length1;
        action_info->stat1_field_high.offset = parserInfo.egress.stat1_offset1 << 3;
        result = bcm_sirius_fabric_action_int_to_egress_source(unit,
                                                               parserInfo.egress.stat1_source1,
                                                               &(action_info->stat1_field_high_type));
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d unable to resolve stat1_high type %d: %d (%s)\n"),
                       unit,
                       parserInfo.egress.stat1_source1,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
        action_info->stat1_adjust = parserInfo.egress.stat1_adjust;
        if (parserInfo.egress.flags & SIRIUS_PARSER_FLAGS_STAT_COMBINE) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_STAT_COMBINE;
        }
        action_info->encap_id_field.length = parserInfo.egress.oi_write_length;
        action_info->encap_id_field.offset = parserInfo.egress.oi_write_byte << 3;
        action_info->len_adj_index_field.length = parserInfo.egress.lengthAdj_length;
        action_info->len_adj_index_field.offset = parserInfo.egress.lengthAdj_nybble << 2;
        switch (parserInfo.egress.requeueSel) {
        case 0:
            action_info->queue_id_source = bcmFabricActionQueueIdxFromPacket;
            break;
        case 1:
            action_info->queue_id_source = bcmFabricActionQueueIdxFromReplicantId;
            break;
        case 2: /* Requeue select values 2 and 3 are equivalent in hardware */
        case 3:
            action_info->queue_id_source = bcmFabricActionQueueIdxFromReplicantLookup;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d unable to resolve requeue selection %d\n"),
                       unit,
                       parserInfo.egress.requeueSel));
            return BCM_E_INTERNAL;
        }
        action_info->queue_field_low.length = parserInfo.egress.oi_read_length;
        action_info->queue_field_low.offset = parserInfo.egress.oi_read_byte << 3;
        action_info->queue_index_offset = parserInfo.egress.oi_index_offset << 9;
        action_info->qsel_offset_index_field.length = parserInfo.egress.fcos_length;
        action_info->qsel_offset_index_field.offset = parserInfo.egress.fcos_bit;
        action_info->clear_ehv_bit = parserInfo.egress.eh_bit;
        action_info->queue_id_field.length = parserInfo.egress.queue_write_length;
        action_info->queue_id_field.offset = parserInfo.egress.queue_write_byte << 3;
        action_info->header_bytes_remove = parserInfo.egress.hdrRemove_bytes;
        if (0 > parserInfo.egress.fourByteRemove_quadbyte) {
            action_info->four_byte_remove_offset = -1;
        } else {
            action_info->four_byte_remove_offset = parserInfo.egress.fourByteRemove_quadbyte << 5;
        }
        action_info->clear_mp_bit = parserInfo.egress.mp_bit;
        switch (parserInfo.egress.insertSel) {
        case 0:
            action_info->insert_dest_select = bcmFabricActionInsertRepId16;
            break;
        case 1:
            action_info->insert_dest_select = bcmFabricActionInsertRepId18;
            break;
        case 2:
            action_info->insert_dest_select = bcmFabricActionInsertEncapId16;
            break;
        case 3:
            action_info->insert_dest_select = bcmFabricActionInsertEncapId18;
            break;
        case 4:
            action_info->insert_dest_select = bcmFabricActionInsertQueueIndex16;
            break;
        case 5:
            action_info->insert_dest_select = bcmFabricActionInsertQueueIndex18;
            break;
        case 6:
            action_info->insert_dest_select = bcmFabricActionInsertQueueId16;
            break;
        case 7:
            action_info->insert_dest_select = bcmFabricActionInsertQueueId18;
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d unable to resolve insertSel %d\n"),
                       unit,
                       parserInfo.egress.insertSel));
            return BCM_E_INTERNAL;
        }
        action_info->override_dp = -1;
        action_info->override_ecn = -1;
        if (parserInfo.egress.flags & SIRIUS_PARSER_FLAGS_LOOKUP_ENCAPID) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_LOOKUP_ENCAPID;
        }
        if (parserInfo.egress.flags & SIRIUS_PARSER_FLAGS_CLEAR_MC_BIT) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_CLEAR_MC_BIT;
        }
        if (parserInfo.egress.flags & SIRIUS_PARSER_FLAGS_LEN_ADJ_ON_SHAPING) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_LEN_ADJ_ON_SHAPING;
        }
        if (parserInfo.egress.flags & SIRIUS_PARSER_FLAGS_LOOKUP_QUEUEID) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_LOOKUP_QUEUEID;
        }
        if (parserInfo.egress.flags & SIRIUS_PARSER_FLAGS_XGS_MODE) {
            action_info->flags |= BCM_FABRIC_ACTION_INFO_XGS_MODE;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "invalid condition of parser from SOC layer\n")));
        return BCM_E_INTERNAL;
    }
    return BCM_E_NONE;
}

/*
 * Traverse the existing actions, and invoke an application provided
 * callback for each one
 */
int
bcm_sirius_fabric_action_traverse(int unit,
                                  bcm_fabric_action_traverse_cb cb,
                                  void *user_data)
{
    int result;
    int cbRes = BCM_E_NONE;
    unsigned int pid = ~0;
    bcm_fabric_action_info_t action_info;

    do {
        result = soc_sirius_parser_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            result = bcm_sirius_fabric_action_get(unit,
                                                  pid,
                                                  &action_info);
            if (SOC_E_NONE == result) {
                cbRes = (*cb)(unit,
                              pid,
                              &action_info,
                              user_data);
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to get unit %d action %d info: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d action after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while ((SOC_E_NOT_FOUND != result) && (BCM_SUCCESS(cbRes)));
    return cbRes;
}

/* Create or update a fabric action */
int
bcm_sirius_fabric_predicate_action_create(int unit,
                                          bcm_fabric_predicate_action_info_t *predicate_action,
                                          bcm_fabric_predicate_action_t *predicate_action_id)
{
    unsigned int pid = ~0;
    unsigned int temp;
    int result;
    soc_sirius_predicate_parser_rule_t predParser;

    if (!predicate_action) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "Unable to create predicate_action with NULL descriptor\n")));
        return BCM_E_PARAM;
    }
    if (!predicate_action_id) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "NULL predicate_action_info\n")));
        return BCM_E_PARAM;
    }
    if (((predicate_action->flags & (BCM_FABRIC_PREDICATE_ACTION_INFO_INGRESS |
                                     BCM_FABRIC_PREDICATE_ACTION_INFO_EGRESS)) ==
         (BCM_FABRIC_PREDICATE_ACTION_INFO_INGRESS |
          BCM_FABRIC_PREDICATE_ACTION_INFO_EGRESS)) ||
        (0 == (predicate_action->flags & (BCM_FABRIC_PREDICATE_ACTION_INFO_INGRESS |
                                          BCM_FABRIC_PREDICATE_ACTION_INFO_EGRESS)))) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "predicate_action must be either ingress or egress\n")));
        return BCM_E_PARAM;
    }
    sal_memset(&predParser, 0x00, sizeof(predParser));
    if (predicate_action->flags & BCM_FABRIC_PREDICATE_ACTION_INFO_INGRESS) {
        predParser.flags |= SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS;
    }
    if (predicate_action->flags & BCM_FABRIC_PREDICATE_ACTION_INFO_EGRESS) {
        predParser.flags |= SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS;
    }
    if (predicate_action->flags & BCM_FABRIC_PREDICATE_ACTION_INFO_WITH_ID) {
        predParser.flags |= SIRIUS_PREDICATE_PARSER_RULE_FLAG_WITH_ID;
        pid = *predicate_action_id;
    }
    if (predicate_action->flags & BCM_FABRIC_PREDICATE_ACTION_INFO_REPLACE) {
        predParser.flags |= SIRIUS_PREDICATE_PARSER_RULE_FLAG_REPLACE;
    }
    for (temp = 0;
         temp < BCM_FABRIC_PREDICATE_COUNT;
         temp++) {
        if (SHR_BITGET(predicate_action->mask, temp)) {
            /* coverity[unsigned_compare : FALSE] */
            if ((predParser.flags &
                 SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS) &&
                (SIRIUS_PREDICATE_OFFSET_INGRESS <= temp) &&
                ((SIRIUS_PREDICATE_OFFSET_INGRESS +
                  SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES) > temp)) {
                /* valid ingress predicate for ingress rule */
                predParser.predMask |= (1 << (temp -
                                              SIRIUS_PREDICATE_OFFSET_INGRESS));
            } else if ((predParser.flags &
                        SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS) &&
                       (SIRIUS_PREDICATE_OFFSET_EGRESS <= temp) &&
                       ((SIRIUS_PREDICATE_OFFSET_EGRESS +
                         SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES) > temp)) {
                /* valid egress predicate for egress rule */
                predParser.predMask |= (1 << (temp -
                                              SIRIUS_PREDICATE_OFFSET_EGRESS));
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d predicate %d not valid for %s rule\n"),
                           unit,
                           temp,
                           (predParser.flags &
                           SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS)?
                           "ingress":
                           "egress"));
                return BCM_E_PARAM;
            }
        }
        if (SHR_BITGET(predicate_action->data, temp)) {
            if ((predParser.flags &
                 SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS) &&
                (SIRIUS_PREDICATE_OFFSET_INGRESS <= temp) &&
                ((SIRIUS_PREDICATE_OFFSET_INGRESS +
                  SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES) > temp)) {
                /* valid ingress predicate for ingress rule */
                predParser.predState |= (1 << (temp -
                                               SIRIUS_PREDICATE_OFFSET_INGRESS));
            } else if ((predParser.flags &
                        SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS) &&
                       (SIRIUS_PREDICATE_OFFSET_EGRESS <= temp) &&
                       ((SIRIUS_PREDICATE_OFFSET_EGRESS +
                         SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES) > temp)) {
                /* valid egress predicate for egress rule */
                predParser.predState |= (1 << (temp -
                                               SIRIUS_PREDICATE_OFFSET_EGRESS));
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d predicate %d not valid for %s rule\n"),
                           unit,
                           temp,
                           (predParser.flags &
                           SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS)?
                           "ingress":
                           "egress"));
                return BCM_E_PARAM;
            }
        }
    }
    if ((0 > predicate_action->priority) ||
        (255 < predicate_action->priority)) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d predicate_action priority %d invalid;"
                               " must be 0..255\n"),
                   unit,
                   predicate_action->priority));
        return BCM_E_PARAM;
    }
    predParser.priority = predicate_action->priority;
    predParser.parser = predicate_action->action_id;
    result = soc_sirius_predicate_parser_map_add(unit,
                                                 &predParser,
                                                 &pid);
    if (SOC_E_NONE == result) {
        *predicate_action_id = pid;
    }
    return result;
}

static int
_bcm_sirius_fabric_predicate_action_from_soc_layer(int unit,
                                                   bcm_fabric_predicate_action_t predicate_action_id,
                                                   soc_sirius_predicate_parser_rule_t *predicate_parser_info,
                                                   bcm_fabric_predicate_action_info_t *predicate_action_info)
{
    unsigned int idx;
    unsigned int base;

    if ((!predicate_parser_info) || (!predicate_action_info)){
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "at least one obligatory pointer argument was NULL\n")));
        return BCM_E_PARAM;
    }
    sal_memset(predicate_action_info, 0x00, sizeof(*predicate_action_info));
    if (SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS ==
        (predicate_parser_info->flags &
         (SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS |
         SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS))) {
        predicate_action_info->flags = BCM_FABRIC_PREDICATE_ACTION_INFO_INGRESS;
        base = SIRIUS_PREDICATE_OFFSET_INGRESS;
    } else if (SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS ==
               (predicate_parser_info->flags &
                (SIRIUS_PREDICATE_PARSER_RULE_FLAG_INGRESS |
                 SIRIUS_PREDICATE_PARSER_RULE_FLAG_EGRESS))) {
        predicate_action_info->flags = BCM_FABRIC_PREDICATE_ACTION_INFO_EGRESS;
        base = SIRIUS_PREDICATE_OFFSET_EGRESS;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "predicate_action must be one of ingress or egress\n")));
        return BCM_E_PARAM;
    }
    for (idx = 0;
         idx < SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES;
         idx++) {
        if (predicate_parser_info->predMask & (1 << idx)) {
            SHR_BITSET(predicate_action_info->mask, idx + base);
        }
        if (predicate_parser_info->predState & (1 << idx)) {
            SHR_BITSET(predicate_action_info->data, idx + base);
        }
    }
    predicate_action_info->action_id = predicate_parser_info->parser;
    predicate_action_info->priority = predicate_parser_info->priority;
    return BCM_E_NONE;
}

/* Read a fabric predicate action */
int
bcm_sirius_fabric_predicate_action_get(int unit,
                                       bcm_fabric_predicate_action_t predicate_action_id,
                                       bcm_fabric_predicate_action_info_t *predicate_action_info)
{
    soc_sirius_predicate_parser_rule_t predicate_parser_info;
    int result;

    if (!predicate_action_info) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "predicate_action_info argument must not be NULL for get\n")));
        return BCM_E_PARAM;
    }
    result = soc_sirius_predicate_parser_map_check(unit, predicate_action_id);
    if (SOC_E_EMPTY == result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d predicate_action %d is not in use\n"),
                   unit,
                   predicate_action_id));
        return BCM_E_NOT_FOUND;
    } else if (SOC_E_FULL != result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to access unit %d predicate_action %d: %d (%s)\n"),
                   unit,
                   predicate_action_id,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    result = soc_sirius_predicate_parser_map_get(unit,
                                                 predicate_action_id,
                                                 &predicate_parser_info);
    if (BCM_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to read unit %d predicate_action %d: %d (%s)\n"),
                   unit,
                   predicate_action_id,
                   result,
                   _SHR_ERRMSG(result)));
        return result;
    }
    return _bcm_sirius_fabric_predicate_action_from_soc_layer(unit,
                                                              predicate_action_id,
                                                              &predicate_parser_info,
                                                              predicate_action_info);
}

/* Destroy a fabric predicate action */
int
bcm_sirius_fabric_predicate_action_destroy(int unit,
                                           bcm_fabric_predicate_action_t predicate_action_id)
{
    int result;

    /* free the chosen action */
    result = soc_sirius_predicate_parser_map_delete(unit, predicate_action_id);
    if (SOC_E_NONE != result) {
        /* unable to free the action */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to free unit %d predicate_action %d: %d (%s)\n"),
                   unit,
                   predicate_action_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* Destroy all fabric predicate actions on the unit */
int
bcm_sirius_fabric_predicate_action_destroy_all(int unit)
{
    int result;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_predicate_parser_map_get_next(unit,
                                                          pid,
                                                          &pid,
                                                          NULL);
        if (SOC_E_NONE == result) {
            result = soc_sirius_predicate_parser_map_delete(unit, pid);
            if (SOC_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to destroy unit %d predicate_action %d:"
                                       " %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d predicate_action"
                                   " after %d: %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while (SOC_E_NOT_FOUND != result);
    return BCM_E_NONE;
}

/*
 * Traverse existing fabric predicate actions, calling the provided
 * callback one time per existing fabric predicate action
 */
int
bcm_sirius_fabric_predicate_action_traverse(int unit,
                                            bcm_fabric_predicate_action_traverse_cb cb,
                                            void *user_data)
{
    int result;
    int cbRes = BCM_E_NONE;
    unsigned int pid = ~0;
    soc_sirius_predicate_parser_rule_t predicate_parser_info;
    bcm_fabric_predicate_action_info_t predicate_action_info;

    do {
        result = soc_sirius_predicate_parser_map_get_next(unit,
                                                          pid,
                                                          &pid,
                                                          &predicate_parser_info);
        if (SOC_E_NONE == result) {
            result = _bcm_sirius_fabric_predicate_action_from_soc_layer(unit,
                                                                        pid,
                                                                        &predicate_parser_info,
                                                                        &predicate_action_info);
            if (SOC_E_NONE == result) {
                cbRes = (*cb)(unit,
                              pid,
                              &predicate_action_info,
                              user_data);
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to parse unit %d predicate_action %d info:"
                                       " %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d predicate_action"
                                   " after %d: %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while ((SOC_E_NOT_FOUND != result) && (BCM_SUCCESS(cbRes)));
    return cbRes;
}

/* This creates or updates a qsel */
int
bcm_sirius_fabric_qsel_create(int unit,
                              uint32 flags,
                              int base,
                              int count,
                              bcm_fabric_qsel_t *qsel_id)
{
    uint32 iflags;
    unsigned int qsid;
    int result;

    if (!qsel_id) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "NULL pred_id\n")));
        return BCM_E_PARAM;
    }
    if ((0 == (flags & BCM_FABRIC_QSEL_INGRESS)) ||
        (0 != (flags & BCM_FABRIC_QSEL_EGRESS))) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "qsel_offset must be ingress for unit %d\n"),
                   unit));
        return BCM_E_PARAM;
    }
    iflags = SIRIUS_QUEUE_MAP_BLOCK_FLAGS_INGRESS;
    if (flags & BCM_FABRIC_QSEL_WITH_ID) {
        iflags |= SIRIUS_QUEUE_MAP_BLOCK_FLAGS_WITH_ID;
        qsid = *qsel_id;
    }
    if (flags & BCM_FABRIC_QSEL_REPLACE) {
        iflags |= SIRIUS_QUEUE_MAP_BLOCK_FLAGS_REPLACE;
    }
    result = soc_sirius_queue_map_block_allocate(unit,
                                                 iflags,
                                                 base,
                                                 count,
                                                 &qsid);
    if (SOC_E_NONE == result) {
        *qsel_id = qsid;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate qsel on unit %d: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* This destroys a qsel */
int
bcm_sirius_fabric_qsel_destroy(int unit,
                               bcm_fabric_qsel_t qsel_id)
{
    int result;

    /* free the chosen qsel */
    result = soc_sirius_queue_map_block_free(unit, qsel_id);
    if (SOC_E_NONE != result) {
        /* unable to free the qsel */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to free unit %d qsel_id %d: %d (%s)\n"),
                   unit,
                   qsel_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* Destroys all qsel on the unit */
int
bcm_sirius_fabric_qsel_destroy_all(int unit)
{
    int result;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_queue_map_block_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            LOG_VERBOSE(BSL_LS_BCM_FABRIC,
                        (BSL_META_U(unit,
                                    "destroy unit %d qsel %d\n"),
                         unit,
                         pid));
            result = soc_sirius_queue_map_block_free(unit, pid);
            if (SOC_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to destroy unit %d qsel %d: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d qsel after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while (SOC_E_NOT_FOUND != result);
    return BCM_E_NONE;
}

/* This gets information about a qsel */
int
bcm_sirius_fabric_qsel_get(int unit,
                           bcm_fabric_qsel_t qsel_id,
                           uint32 *flags,
                           int *base,
                           int *count)
{
    int result;
    unsigned int ibase;
    unsigned int isize;

    result = soc_sirius_queue_map_block_get(unit,
                                            qsel_id,
                                            &ibase,
                                            &isize);
    if (SOC_E_NONE == result) {
        *flags = BCM_FABRIC_QSEL_OFFSET_INGRESS;
        *base = ibase;
        *count = isize;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to read unit %d qsel %d: %d (%s)\n"),
                   unit,
                   qsel_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 * Traverse existing fabric qsels, calling the provided callback one time
 * per existing qsel
 */
int
bcm_sirius_fabric_qsel_traverse(int unit,
                                bcm_fabric_qsel_traverse_cb cb,
                                void *user_data)
{
    int result;
    int cbRes = BCM_E_NONE;
    unsigned int pid = ~0;
    unsigned int base;
    unsigned int size;

    do {
        result = soc_sirius_queue_map_block_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            result = soc_sirius_queue_map_block_get(unit,
                                                    pid,
                                                    &base,
                                                    &size);
            if (SOC_E_NONE == result) {
                /* we only support ingress on Sirius */
                cbRes = (*cb)(unit,
                              pid,
                              BCM_FABRIC_QSEL_OFFSET_INGRESS,
                              base,
                              size,
                              user_data);
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to get unit %d qsel %d info: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d qsel after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while ((SOC_E_NOT_FOUND != result) && (BCM_SUCCESS(cbRes)));
    return cbRes;
}

/*
 * Sets an entry within a qsel.  To effectively clear an entry, provide
 * BCM_GPORT_INVALID as the queue.
 */
int
bcm_sirius_fabric_qsel_entry_set(int unit,
                                 bcm_fabric_qsel_t qsel_id,
                                 int offset,
                                 bcm_gport_t queue,
                                 bcm_fabric_qsel_offset_t qsel_offset_id)
{
    unsigned int qid;
    int result;
    bcm_sbx_cosq_queue_state_t *p_qstate;

    if (BCM_GPORT_INVALID == queue) {
        /* this case 'deletes' the entry */
        qid = SIRIUS_Q_BASE_INVALID;
    } else if (!BCM_GPORT_IS_SET(queue)) {
        /* backdoor - direct queue ID */
        
        qid = queue;
    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(queue)) {
        qid = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(queue);
    } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(queue)) {
        qid = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(queue);
    } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(queue)) {
        qid = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(queue);
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(queue)) {
        qid = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(queue);
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to determine underlying queue from value\n")));
        return BCM_E_PARAM;
    }
    if (SIRIUS_Q_BASE_INVALID != qid) {
        /* requested qid is not special case 'disable' queue; check it */
        p_qstate = (bcm_sbx_cosq_queue_state_t*)(SOC_SBX_STATE(unit)->queue_state);
        p_qstate = &p_qstate[qid];
        if (BCM_INT_SBX_QUEUE_STATE_IN_USE != p_qstate->state) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unit %d queue %08X is not active\n"),
                       unit, queue));
            return BCM_E_NOT_FOUND;
        }
    }

    result = soc_sirius_queue_map_block_entry_set(unit,
                                                  qsel_id,
                                                  offset,
                                                  qsel_offset_id,
                                                  qid);
    if (SOC_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to set unit %d qsel %d offset %d: %d (%s)\n"),
                   unit,
                   qsel_id,
                   offset,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 *  Given a little bit of additional information, translate a queue into some
 *  apporpriate form of GPORT.
 */
int
_bcm_sirius_fabric_qsel_entry_gport_extrapolate(int unit,
                                                unsigned int queue,
                                                bcm_gport_t *gport)
{
    int sysport;
    int result;
    int mc;

    if (SIRIUS_Q_BASE_INVALID == queue) {
        *gport = BCM_GPORT_INVALID;
        result = BCM_E_NONE;
    } else {
        result = soc_sirius_qs_queue_to_sysport_cos_get(unit,
                                                        queue,
                                                        &mc,
                                                        &sysport,
                                                        NULL);
        if (BCM_E_NONE == result) {
            if (mc) {
                /* queue is multicast */
                
                if (SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq > queue) {
                    /* fabric queue */
                    BCM_GPORT_MCAST_QUEUE_GROUP_SYSQID_SET(*gport,
                                                           sysport,
                                                           queue);
                    if ((_SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK & queue) !=
                        queue) {
                        LOG_WARN(BSL_LS_BCM_FABRIC,
                                 (BSL_META_U(unit,
                                             "unable to represent queue %d in proper"
                                              " GPORT form: field is too small\n"),
                                  queue));
                    }
                } else {
                    /* local queue */
                    BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*gport,
                                                                   queue);
                }
            } else {
                /* queue is unicast */
                if (SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq > queue) {
                    /* fabric queue */
                    BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(*gport,
                                                           sysport,
                                                           queue);
                    if ((_SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK & queue) !=
                        queue) {
                        LOG_WARN(BSL_LS_BCM_FABRIC,
                                 (BSL_META_U(unit,
                                             "unable to represent queue %d in proper"
                                              " GPORT form: field is too small\n"),
                                  queue));
                    }
                } else {
                    /* local queue */
                    BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*gport,
                                                                   queue);
                }
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unable to resolve unit %d hardware queue %d to"
                                   " gport; returning it raw instead\n"),
                       unit,
                       queue));
            *gport = queue;
        }
        result = BCM_E_NONE;
    }
    return result;
}


/*
 * Gets an entry within a qsel.  Entries which are not in use will return
 * BCM_GPORT_INVALID as their queue.
 */
int
bcm_sirius_fabric_qsel_entry_get(int unit,
                                 bcm_fabric_qsel_t qsel_id,
                                 int offset,
                                 bcm_gport_t *queue,
                                 bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    unsigned int qid;
    unsigned int qoid;
    int result;

    result = soc_sirius_queue_map_block_entry_get(unit,
                                                  qsel_id,
                                                  offset,
                                                  &qoid,
                                                  &qid);
    if (SOC_E_NONE == result) {
        result = _bcm_sirius_fabric_qsel_entry_gport_extrapolate(unit,
                                                                 qid,
                                                                 queue);
        if (BCM_E_NONE == result) {
            *qsel_offset_id = qoid;
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unable to determine GPORT from unit %d queue %d:"
                                   " %d (%s)\n"),
                       unit,
                       qid,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to read unit %d qsel %d offset %d: %d (%s)\n"),
                   unit,
                   qsel_id,
                   offset,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 * Sets a group of entries within a qsel.  To effectively clear entries,
 * set their respective queues to BCM_GPORT_INVALID.
 */
int
bcm_sirius_fabric_qsel_entry_multi_set(int unit,
                                       bcm_fabric_qsel_t qsel_id,
                                       int offset,
                                       int count,
                                       bcm_gport_t *queue,
                                       bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    unsigned int *qid = NULL;
    unsigned int *qoid = NULL;
    unsigned int idx;
    int result = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;

    /*
     *  We need to edit the queue array so it reflects only queue number.  It
     *  should not involve the BCM layer metadata.  The qsel_offset_id array
     *  does not need to be manipulated thus, and can be passed after merely
     *  casting it as unsigned int, but we'll copy it as well.
     */
    qid = sal_alloc(sizeof(*qid) * count, "queue translation space");
    if (!qid) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate work buffer for bulk write qsel"
                               " entries (queue)\n")));
        return BCM_E_RESOURCE;
    }
    qoid = sal_alloc(sizeof(*qoid) * count, "qsel_offset workspace");
    if (!qoid) {
        sal_free(qid);
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate work buffer for bulk write qsel"
                               " entries (qsel_offset)\n")));
        return BCM_E_RESOURCE;
    }
    p_qstate = (bcm_sbx_cosq_queue_state_t*)(SOC_SBX_STATE(unit)->queue_state);
    for (idx = 0; idx < count; idx++) {
        if (BCM_GPORT_INVALID == queue[idx]) {
            /* this case 'deletes' the entry */
            qid[idx] = SIRIUS_Q_BASE_INVALID;
        } else if (!BCM_GPORT_IS_SET(queue[idx])) {
            /* backdoor - direct queue ID */
            
            qid[idx] = queue[idx];
        } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(queue[idx])) {
            qid[idx] = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(queue[idx]);
        } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(queue[idx])) {
            qid[idx] = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(queue[idx]);
        } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(queue[idx])) {
            qid[idx] = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(queue[idx]);
        } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(queue[idx])) {
            qid[idx] = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(queue[idx]);
        } else {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unable to determine underlying queue from value %d ="
                                   " %d(%08X)\n"),
                       idx,
                       qid[idx],
                       qid[idx]));
            result = BCM_E_PARAM;
            break;
        }
        qoid[idx] = qsel_offset_id[idx];
        if (SIRIUS_Q_BASE_INVALID != qid[idx]) {
            /* requested qid is not special case 'disable' queue; check it */
            if (BCM_INT_SBX_QUEUE_STATE_IN_USE != (&p_qstate[qid[idx]])->state) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unit %d queue %08X is not active\n"),
                           unit, queue[idx]));
                result = BCM_E_NOT_FOUND;
                break;
            }
        }
    }
    if (BCM_E_NONE == result) {
        result = soc_sirius_queue_map_block_entry_multi_set(unit,
                                                            qsel_id,
                                                            offset,
                                                            count,
                                                            &(qoid[0]),
                                                            &(qid[0]));
        if (SOC_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unable to set unit %d qsel %d offset %d..%d: %d (%s)\n"),
                       unit,
                       qsel_id,
                       offset,
                       offset + count - 1,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }
    sal_free(qid);
    sal_free(qoid);
    return result;
}

/*
 * Gets a group of entries within a qsel.  Entries that are not in use
 * will have BCM_GPORT_INVALID as their queue.
 */
int
bcm_sirius_fabric_qsel_entry_multi_get(int unit,
                                       bcm_fabric_qsel_t qsel_id,
                                       int offset,
                                       int count,
                                       bcm_gport_t *queue,
                                       bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    unsigned int *qid = NULL;
    unsigned int *qoid = NULL;
    unsigned int idx;
    int result = BCM_E_NONE;

    /*
     *  We need to edit the queue array so it reflects original handle.  The
     *  SOC layer does not keep the BCM layer metadata.  The qsel_offset_id
     *  array does not need to be manipulated thus, and can be passed after
     *  merely casting it as unsigned int, but we'll copy it as well.
     */
    qid = sal_alloc(sizeof(*qid) * count, "queue translation space");
    if (!qid) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate work buffer for bulk write qsel"
                               " entries (queue)\n")));
        return BCM_E_RESOURCE;
    }
    qoid = sal_alloc(sizeof(*qoid) * count, "qsel_offset workspace");
    if (!qoid) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate work buffer for bulk write qsel"
                               " entries (qsel_offset)\n")));
        sal_free(qid);
        return BCM_E_RESOURCE;
    }
    result = soc_sirius_queue_map_block_entry_multi_get(unit,
                                                        qsel_id,
                                                        offset,
                                                        count,
                                                        &(qoid[0]),
                                                        &(qid[0]));
    if (SOC_E_NONE == result) {
        for (idx = 0; idx < count; idx++) {
            result = _bcm_sirius_fabric_qsel_entry_gport_extrapolate(unit,
                                                                     qid[idx],
                                                                     &(queue[idx]));
            if (SOC_E_NONE != result) {
                break;
            }
            qoid[idx] = qsel_offset_id[idx];
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to get unit %d qsel %d offset %d..%d: %d (%s)\n"),
                   unit,
                   qsel_id,
                   offset,
                   offset + count - 1,
                   result,
                   _SHR_ERRMSG(result)));
    }
    sal_free(qid);
    sal_free(qoid);
    return result;
}

/*
 * Traverse existing entries within a specified qsel, calling the
 * provided callback one time per existing entry
 */
int
bcm_sirius_fabric_qsel_entry_traverse(int unit,
                                      bcm_fabric_qsel_t qsel_id,
                                      bcm_fabric_qsel_entry_traverse_cb cb,
                                      void *user_data)
{
    int result;
    int cbRes = BCM_E_NONE;
    unsigned int pid = ~0;
    unsigned int qid;
    unsigned int qoid;
    bcm_gport_t queue;

    do {
        result = soc_sirius_queue_map_block_entry_next(unit,
                                                       qsel_id,
                                                       pid,
                                                       &pid);
        if (SOC_E_NONE == result) {
            result = soc_sirius_queue_map_block_entry_get(unit,
                                                          qsel_id,
                                                          pid,
                                                          &qoid,
                                                          &qid);
            if (SOC_E_NONE == result) {
                result = _bcm_sirius_fabric_qsel_entry_gport_extrapolate(unit,
                                                                         qid,
                                                                         &queue);
                if (BCM_E_NONE != result) {
                    LOG_ERROR(BSL_LS_BCM_FABRIC,
                              (BSL_META_U(unit,
                                          "unable to determine GPORT from unit %d queue %d:"
                                           " %d (%s)\n"),
                               unit,
                               qid,
                               result,
                               _SHR_ERRMSG(result)));
                    return result;
                }
                cbRes = (*cb)(unit,
                              qsel_id,
                              pid,
                              queue,
                              qoid,
                              user_data);
            } else {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to get unit %d qsel %d entry %dinfo: %d (%s)\n"),
                           unit,
                           qsel_id,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d qsel %d entry after %d:"
                                   " %d (%s)\n"),
                       unit,
                       qsel_id,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while ((SOC_E_NOT_FOUND != result) && (BCM_SUCCESS(cbRes)));
    return cbRes;
}

/* This creates or updates a qsel_offset */
int
bcm_sirius_fabric_qsel_offset_create(int unit,
                                     uint32 flags,
                                     bcm_fabric_qsel_offset_t *qsel_offset_id)
{
    int result;
    unsigned int iflags;
    unsigned int pid = ~0;

    if (!qsel_offset_id) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "NULL qsel_offset_id\n")));
        return BCM_E_PARAM;
    }
    if ((0 == (flags & BCM_FABRIC_QSEL_OFFSET_INGRESS)) ||
        (0 != (flags & BCM_FABRIC_QSEL_OFFSET_EGRESS))) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "qsel_offset must be ingress for unit %d\n"),
                   unit));
        return BCM_E_PARAM;
    }
    iflags = SIRIUS_COS_MAP_BLOCK_FLAGS_INGRESS;
    if (flags & BCM_FABRIC_QSEL_OFFSET_WITH_ID) {
        iflags |= SIRIUS_COS_MAP_BLOCK_FLAGS_WITH_ID;
        pid = *qsel_offset_id;
    }
    if (flags & BCM_FABRIC_QSEL_OFFSET_REPLACE) {
        iflags |= SIRIUS_COS_MAP_BLOCK_FLAGS_REPLACE;
    }
    result = soc_sirius_cos_map_block_allocate(unit, iflags, &pid);
    if (SOC_E_NONE != result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to allocate qsel_offset on unit %d: %d (%s)\n"),
                   unit,
                   result,
                   _SHR_ERRMSG(result)));
    } else {
        *qsel_offset_id = pid;
    }
    return result;
}

/* This destroys a qsel_offset */
int
bcm_sirius_fabric_qsel_offset_destroy(int unit,
                                      bcm_fabric_qsel_offset_t qsel_offset_id)
{
    int result;

    /* free the chosen qsel_offset */
    result = soc_sirius_cos_map_block_free(unit, qsel_offset_id);
    if (SOC_E_NONE != result) {
        /* unable to free the qsel_offset */
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to free unit %d qsel_offset %d: %d (%s)\n"),
                   unit,
                   qsel_offset_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* Destroys all qsel_offset on the unit */
int
bcm_sirius_fabric_qsel_offset_destroy_all(int unit)
{
    int result;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_cos_map_block_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            LOG_VERBOSE(BSL_LS_BCM_FABRIC,
                        (BSL_META_U(unit,
                                    "destroy unit %d qsel_offset %d\n"),
                         unit,
                         pid));
            result = bcm_sirius_fabric_qsel_offset_destroy(unit, pid);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_FABRIC,
                          (BSL_META_U(unit,
                                      "unable to destroy unit %d qsel_offset %d: %d (%s)\n"),
                           unit,
                           pid,
                           result,
                           _SHR_ERRMSG(result)));
                return result;
            }
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d qsel_offset after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while (SOC_E_NOT_FOUND != result);
    return BCM_E_NONE;
}

/*
 * Traverse existing qsel_offsets, calling the provided callback one time
 * per existing qsel_offset
 */
int
bcm_sirius_fabric_qsel_offset_traverse(int unit,
                                       bcm_fabric_qsel_offset_traverse_cb cb,
                                       void *user_data)
{
    int result;
    int cbRes = BCM_E_NONE;
    unsigned int pid = ~0;

    do {
        result = soc_sirius_cos_map_block_next(unit, pid, &pid);
        if (SOC_E_NONE == result) {
            /* we only support ingress on Sirius */
            cbRes = (*cb)(unit, pid, BCM_FABRIC_QSEL_OFFSET_INGRESS, user_data);
        } else if (SOC_E_NOT_FOUND != result) {
            LOG_ERROR(BSL_LS_BCM_FABRIC,
                      (BSL_META_U(unit,
                                  "unexpected error getting unit %d qsel_offset after %d:"
                                   " %d (%s)\n"),
                       unit,
                       pid,
                       result,
                       _SHR_ERRMSG(result)));
            return result;
        }
    } while ((SOC_E_NOT_FOUND != result) && (BCM_SUCCESS(cbRes)));
    return cbRes;
}

/* Set an entry of a qsel_offset */
int
bcm_sirius_fabric_qsel_offset_entry_set(int unit,
                                       bcm_fabric_qsel_offset_t qsel_offset_id,
                                       bcm_cos_t int_pri,
                                       int offset)
{
    int result;

    if (0 > int_pri) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "invalid int_pri value; must be positive\n")));
        return BCM_E_PARAM;
    }
    if (0 > offset) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "invalid offset value; must be positive\n")));
        return BCM_E_PARAM;
    }
    result = soc_sirius_cos_map_block_check(unit, qsel_offset_id);
    if (BCM_E_FULL == result) {
        result = soc_sirius_cos_map_block_entry_set(unit,
                                                    qsel_offset_id,
                                                    int_pri,
                                                    offset);
    } else if (SOC_E_EMPTY == result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d qsel_offset %d is not in use\n"),
                   unit,
                   qsel_offset_id));
        result = BCM_E_NOT_FOUND;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to set unit %d qsel_offset %d int_pri %d: %d (%s)\n"),
                   unit,
                   qsel_offset_id,
                   int_pri,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/* Get an entry of a qsel_offset */
int
bcm_sirius_fabric_qsel_offset_entry_get(int unit,
                                        bcm_fabric_qsel_offset_t qsel_offset_id,
                                        bcm_cos_t int_pri,
                                        int *offset)
{
    int result;
    unsigned int value;

    if (0 > int_pri) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "invalid int_pri value; must be positive\n")));
        return BCM_E_PARAM;
    }
    result = soc_sirius_cos_map_block_check(unit, qsel_offset_id);
    if (SOC_E_FULL == result) {
        result = soc_sirius_cos_map_block_entry_get(unit,
                                                    qsel_offset_id,
                                                    int_pri,
                                                    &value);
        if (SOC_E_NONE == result) {
            *offset = value;
        }
    } else if (SOC_E_EMPTY == result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d qsel_offset %d is not in use\n"),
                   unit,
                   qsel_offset_id));
        result = BCM_E_NOT_FOUND;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to get unit %d qsel_offset %d int_pri %d: %d (%s)\n"),
                   unit,
                   qsel_offset_id,
                   int_pri,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}

/*
 * Traverse entries in a qsel_offset, calling the provided callback one
 * time per existing entry
 */
int
bcm_sirius_fabric_qsel_offset_entry_traverse(int unit,
                                             bcm_fabric_qsel_offset_t qsel_offset_id,
                                             bcm_fabric_qsel_offset_entry_traverse_cb cb,
                                             void *user_data)
{
    int result;
    unsigned int value;
    unsigned int idx;

    result = soc_sirius_cos_map_block_check(unit, qsel_offset_id);
    if (SOC_E_FULL == result) {
        for (idx = 0; idx < 16; idx++) {
            result = soc_sirius_cos_map_block_entry_get(unit,
                                                        qsel_offset_id,
                                                        idx,
                                                        &value);
            if (SOC_E_NONE == result) {
                result = (*cb)(unit, qsel_offset_id, idx, value, user_data);
                if (BCM_FAILURE(result)) {
                    break;
                }
            } else {
                break;
            }
        }
    } else if (SOC_E_EMPTY == result) {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unit %d qsel_offset %d is not in use\n"),
                   unit,
                   qsel_offset_id));
        result = BCM_E_NOT_FOUND;
    } else {
        LOG_ERROR(BSL_LS_BCM_FABRIC,
                  (BSL_META_U(unit,
                              "unable to access unit %d qsel_offset %d: %d (%s)\n"),
                   unit,
                   qsel_offset_id,
                   result,
                   _SHR_ERRMSG(result)));
    }
    return result;
}




