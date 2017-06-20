/*
 * $Id: cosq.c,v 1.86 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * BM9600 Fabric Control API
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/fabric/sbZfFabWredParameters.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR0WdtEntry.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct0AEntry.hx>
#include <soc/sbx/fabric/sbZfFabBm9600BwR1Wct0BEntry.hx>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/device_wred.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/bm9600.h>


typedef struct {
    int32 in_use;              /* portset in use state */
    int32 offset_in_use;       /* in use mask for each of 16 offset in portset */
    int32 virtual;             /* virtual or physical */
    int32 eopp;                 /* eopp mask for quad 1-3 */
    int32 eg_node;             /* egress node */
    int32 start_port;          /* starting egress port */
    int32 next;                /* next pointer */
    int32 prev;                /* previous pointer */
    int32 sysports[16];        /* sysports */
}bcm_sbx_cosq_portset_state_t;

typedef struct {
    int32 node;                /* egress node */
    int32 port;                /* egress port */
    int32 sysport;             /* sysport */
    int32 ps;                  /* portset */
    int32 os;                  /* offset */
}bcm_sbx_cosq_nps_info_t;

#define _BCM_INT_BM9600_PORTSET_OFFSET_USE_SET(portset, offset) \
    ps_state[unit][(portset)].offset_in_use |= (1 << (offset))

#define _BCM_INT_BM9600_PORTSET_OFFSET_USE_CLEAR(portset, offset) \
    ps_state[unit][(portset)].offset_in_use &= (~(1 << (offset)))

#define _BCM_INT_BM9600_PORTSET_OFFSET_USED(portset, offset) \
    ((ps_state[unit][(portset)].offset_in_use & (1 << (offset))) != 0)

#define _BCM_INT_BM9600_PORTSET_QUAD_USED(portset, quad) \
    ((ps_state[unit][(portset)].offset_in_use & (0xF << (quad * 4))) != 0)

#define _BCM_INT_BM9600_PORTSET_USED(portset) \
    ((ps_state[unit][(portset)].offset_in_use & (0xFFFF)) != 0)


/*******************************************
 * portset state
 * portset management strategy:
 *   (1) For a particular node, first all virtual ports of the node
 *   are linked, then all physical ports are linked, the existence
 *   of virtual port portset/offset entry for a particular port means
 *   the portset/offset entry in the following physical port portset
 *   linklist is invalid and will not be used.
 *   (2) within either virtual port linklist or physical port linklist
 *   ports are in strict increasing order.
 *   (3) a particular port always occupy a continuous bit range in the virtual
 *   port list, a particular port will only occupy a single bit in the
 *   physical port list. The port may be in both virtual port list and
 *   physical port list, in that case, the bit in the physical port list
 *   must be marked as unused, and the bit range in virtual port list
 *   take priority.
 */
static bcm_sbx_cosq_portset_state_t *ps_state[SOC_MAX_NUM_DEVICES];

static int ps_head[SOC_MAX_NUM_DEVICES];


#ifdef BCM_EASY_RELOAD_SUPPORT
/* These functions are used to set/get a stored hardware token   */
/* If a sysport/portset setup is in progress while a software    */
/* crash occurs, it may not be possible to recreate the software */
/* state from the hardware.  These tokens allow the software to  */
/* recognize that prior to the software reload, whether the      */
/* software was attempting to configure these tables (through    */
/* bcm_cosq_gport_add() or fabric_port_failover_set())           */

static int
_bcm_bm9600_cosq_take_token(int unit);

static int
_bcm_bm9600_cosq_free_token(int unit);

static int
_bcm_bm9600_cosq_get_token_state(int unit);
#endif /* BCM_EASY_RELOAD_SUPPORT */

#ifdef BCM_EASY_RELOAD_SUPPORT
static bcm_sbx_cosq_bw_group_state_cache_t *bw_group_state_cache[SOC_MAX_NUM_DEVICES];
#endif /* BCM_EASY_RELOAD_SUPPORT */

static int
_bcm_bm9600_cosq_update_bag(int unit,
			    int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region, int add);

static int
_bcm_bm9600_get_nbr_node_enabled(int unit, int *nbr_node_enabled);

static int
_bcm_bm9600_portset_state_init(int unit);

static int
_bcm_bm9600_alloc_portset(int unit,
			  int bVirtual,
			  int eg_node,
			  int start_port,
			  int *portset);

static int
_bcm_bm9600_init_portset(int unit,
			 int ps,
			 bcm_sbx_cosq_portset_state_t *p_psstate);

static int
_bcm_bm9600_insert_portset(int unit,
			   int portset);

static int
_bcm_bm9600_insert_portset_after(int unit,
				 int portset,
				 int prev_ps);

static int
_bcm_bm9600_free_portset(int unit,
			 int portset);

static int
_bcm_bm9600_delete_portset(int unit,
			   int portset);

static int
_bcm_bm9600_get_node_port_from_portset(int unit,
				       int portset,
				       int offset,
				       int *node,
				       int *port,
				       int *bIn_use);

static int
_bcm_bm9600_get_portset_from_node_port(int unit,
				       int node,
				       int port,
				       int bVirtual,
				       int *bAlloced,
				       int *bAvailable,
				       int *portset,
				       int *offset);

static int
_bcm_bm9600_get_sysport_count_for_node_port(int unit,
					    int node,
					    int port,
					    int *count);

static int
_bcm_bm9600_unmap_sysport_to_portset(int unit,
				     int sysport,
				     int portset,
				     int offset);

static int
_bcm_bm9600_move_sysport(int unit,
			 int old_portset,
			 int old_offset,
			 int new_portset,
			 int new_offset);

static int
_bcm_bm9600_rebuild_virtual_portset_for_add_delete(int unit,
						   int node,
						   int port,
						   int sysport,
						   int add);


int
bcm_bm9600_cosq_init(int unit)
{
    int rv = BCM_E_UNAVAIL;
    int32 bw_group;
    int sysport;

#ifdef BCM_EASY_RELOAD_SUPPORT
    uint32 num_queues_in_bag, base_queue_of_bag, bag_rate_bytes_per_epoch;
    bcm_sbx_cosq_bw_group_state_cache_t *p_bwstate_cache;
#endif

    rv = _bcm_bm9600_portset_state_init(unit);

    /* Read PRT table from the hardware if in EASY_RELOAD mode and store in table
     * write queue to bw_group_state index and write bw_group index to the base queue_state.
     */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT

        /*
         * Only perform if we are not in LCM or straight xbar mode.
         * This table is only valid when configured as an arbiter and the
         * bw block is not enabled in this case.
         */
        if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode != SOC_SBX_BME_LCM_MODE) &&
                (SOC_SBX_CFG_BM9600(unit)->uDeviceMode != SOC_SBX_BME_XBAR_MODE)) {
	    
	    bw_group_state_cache[unit] = sal_alloc(sizeof(bcm_sbx_cosq_bw_group_state_cache_t) * (SOC_SBX_CFG(unit)->num_bw_groups),
						   "bw group state cache memory");

	    sal_memset(bw_group_state_cache[unit], 0, sizeof(bcm_sbx_cosq_bw_group_state_cache_t));

	    p_bwstate_cache = bw_group_state_cache[unit];

	    if (bw_group_state_cache[unit] == NULL) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, sal_alloc,  Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return(BCM_E_MEMORY);
	    }

	    SOC_SBX_STATE(unit)->bw_group_state_cache = bw_group_state_cache[unit];

	    for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

	        /* Read PRT */
	        rv = soc_bm9600_bag_read(unit, bw_group, &num_queues_in_bag,
				     &base_queue_of_bag, &bag_rate_bytes_per_epoch);

	        if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: BM9600 read of bag table failed for bw_group(%d)\n"),
		               bw_group));
		    return BCM_E_FAIL;
	        }
	        if (base_queue_of_bag < SOC_SBX_CFG(unit)->num_queues) {
		    p_bwstate_cache[bw_group].base_queue_of_bag = base_queue_of_bag;
		    p_bwstate_cache[bw_group].bag_rate_bytes_per_epoch = bag_rate_bytes_per_epoch;
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "EASY_RELOAD bw_group(%d) base_queue_of_bag(%d)\n"),
		              bw_group, base_queue_of_bag));
	        }
	    }
        }
#endif
    } else {
    /* If we are not reloading, then invalidate all PRT entries base_queue during cosq init */
    /* This is how we know the entry is not in use when reloading                           */
	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {


	    /* Only perform if we are not in LCM or straight xbar mode.
	     * This table is only valid when configured as an arbiter and the
	     * bw block is not enabled in this case.
	     */
	    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode != SOC_SBX_BME_LCM_MODE) &&
		(SOC_SBX_CFG_BM9600(unit)->uDeviceMode != SOC_SBX_BME_XBAR_MODE)) {
		
		rv = soc_bm9600_bag_write(unit, bw_group,  0 /* num_queues_in_bag */,
					  0x3fffff /* base_queue */ , 0 /* bag rate */);
		
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: BM9600 write of PRT table failed for bw_group(%d)\n"),
		               bw_group));
		    return BCM_E_FAIL;
		}
	    }
	}

	/* Also invalidate sysport mapping table */
	for (sysport=0; sysport<SOC_SBX_CFG(unit)->num_sysports; sysport++) {
	    rv = soc_bm9600_ina_sysport_map_table_write_all(unit, sysport, 0xff /* portset */, 0 /* offset */);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Failed to write sysport map table, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_INTERNAL;
	    }
	}
    }

    return rv;
}

int
bcm_bm9600_cosq_add_queue(int unit,
                          int queue,
                          bcm_sbx_cosq_queue_region_type_t queue_region,
                          int sysport,
                          int eset,
                          int dest_node,
                          int dest_port,
                          int dest_mc,
                          int dest_cos,
                          int32 dest_type,
                          bcm_sbx_cosq_queue_params_ingress_t *p_qparams, /* unused for BM9600 */
                          bcm_sbx_cosq_bw_group_params_t *p_bwparams,     /* unused for BM9600 */
                          int inhibit_write)                  /* unused for BM9600 */

{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *queue_state;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int subport = 0, eg_n = 0, num_fifos = 0;
    int32 bw_group;
    int mapped_dest_port;
    sbBool_t is_portset1_allocated = FALSE, is_portset2_allocated = FALSE;

#ifdef BCM_EASY_RELOAD_SUPPORT
    int32 portset, offset;
    int32 eopp, virtual, start_port, eg_node;
    int32 sysport_read;
    bcm_sbx_cosq_portset_state_t *p_psstate;
#endif

    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
	if (_bcm_bm9600_cosq_get_token_state(unit)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: EASY_RELOAD, cold boot required because internal state is unknown - crashed during sysport/portset setup\n")));
	    return BCM_E_INTERNAL;
	}
#endif
    }

    if ( (sysport == BCM_INT_SBX_INVALID_SYSPORT) && (dest_type != BCM_INT_SBX_DEST_TYPE_MULTICAST) 
	 && (queue_region != bcm_sbx_cosq_queue_region_local )) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Sysport required for the unicast queues")));
	return BCM_E_INTERNAL;
    }

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    if ( dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST || 
	 queue_region == bcm_sbx_cosq_queue_region_local) {
	/* Multicast reserve sysport but don't use portset
	 * Do nothing here!
	 */

    } else {
        if ((BCM_GPORT_IS_CHILD(p_bwstate->gport)) ||
            (BCM_GPORT_IS_EGRESS_CHILD(p_bwstate->gport)) ||
            (BCM_GPORT_IS_EGRESS_GROUP(p_bwstate->gport))) {
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, p_bwstate->gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
                           FUNCTION_NAME(), p_bwstate->gport, unit));
                return BCM_E_PARAM;
            }    
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[(dest_node * SB_FAB_DEVICE_MAX_FABRIC_PORTS) + subport]);
            /* COS 0 is by default EF traffic */
            if (dest_cos == 0) {
                mapped_dest_port = sp_info->egroup[eg_n].ef_fcd;
            } else {
                mapped_dest_port = sp_info->egroup[eg_n].nef_fcd;
            }
            
            if ( soc_feature(unit, soc_feature_egr_independent_fc) == FALSE) {
                mapped_dest_port /= 2;
            }
            
        } else {
            mapped_dest_port = (soc_feature(unit, soc_feature_egr_independent_fc)) ?
                BCM_INT_SBX_PORT_TO_EF_PORT(dest_port) : dest_port;
        }
        
#ifdef BCM_EASY_RELOAD_SUPPORT

        _bcm_bm9600_cosq_take_token(unit);
	
	/* If we are reloading, don't go through normal path, we just want to verify what we can from the hardware */
	if (SOC_IS_RELOADING(unit)) {
	    
	    rv = soc_bm9600_ina_sysport_map_table_read(unit, sysport, &portset, &offset);
	    
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD sysport mapping table read failed for sysport(%d) coldboot required\n"),
		           sysport));
		return BCM_E_INTERNAL;
	    }
	    
	    if (portset == 0xff) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD portset entry invalid for sysport(%d), coldboot required\n"),
		           sysport));
		return BCM_E_INTERNAL;
	    }
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "EASY_RELOAD: portset(%d) for sysport(%d)\n"),
	              portset, sysport));
	    
	    rv = soc_bm9600_portset_info_table_read(unit, portset, &virtual, &eopp, &start_port, &eg_node);
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "EASY_RELOAD: portset(%d) virtual(%d) eopp(%d) start_port(%d) eg_node(%d)\n"),
	              portset, virtual, eopp, start_port, eg_node));

	    if (eg_node != dest_node) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD egress node(%d) from portset info doesn't match node(%d) in GPORT_ADD\n"),
		           eg_node, dest_node));
		return BCM_E_INTERNAL;
	    }
	    /* Physical port */
  	    if (!virtual) {
 		if (mapped_dest_port != (start_port + offset)) {
  		    LOG_ERROR(BSL_LS_BCM_COMMON,
  		              (BSL_META_U(unit,
  		                          "ERROR: EASY_RELOAD egress port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
  		               start_port + offset, mapped_dest_port));
  		    return BCM_E_INTERNAL;
		    
  		}
  	    }
  	    /* Virtual port */
  	    else {
  		if ((offset >= 0) && (offset < 4)) {
 		    if (mapped_dest_port != start_port) {
  			LOG_ERROR(BSL_LS_BCM_COMMON,
  			          (BSL_META_U(unit,
  			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
  			           start_port, mapped_dest_port));
  			return BCM_E_INTERNAL;
  
  		    }
  		}
  		else if ((offset >= 4) && (offset < 8) && (eopp&1)) {
 		    if( mapped_dest_port != (start_port + 1) ) {
  			LOG_ERROR(BSL_LS_BCM_COMMON,
  			          (BSL_META_U(unit,
  			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
  			           start_port, mapped_dest_port));
  			return BCM_E_INTERNAL;
			
  		    }
  		}
 		else if ((offset >= 4) && (offset < 8) && (((eopp)&1) == 0)) {
 		    if (mapped_dest_port != start_port) {
  			LOG_ERROR(BSL_LS_BCM_COMMON,
  			          (BSL_META_U(unit,
  			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
  			           start_port, mapped_dest_port));
  			return BCM_E_INTERNAL;
			
  		    }
  		}
 		else if ((offset >= 8) && (offset < 12) && ((eopp&3) == 0)) {
 		    if (mapped_dest_port != start_port) {
  			LOG_ERROR(BSL_LS_BCM_COMMON,
  			          (BSL_META_U(unit,
  			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
  			           start_port, mapped_dest_port));
  			return BCM_E_INTERNAL;
			
  		    }
  		}
 		else if ((offset >= 8) && (offset < 12) && ((eopp&3) == 1)) {
 		    if (mapped_dest_port != start_port + 1) {
 			LOG_ERROR(BSL_LS_BCM_COMMON,
 			          (BSL_META_U(unit,
 			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
 			           start_port, mapped_dest_port));
 			return BCM_E_INTERNAL;
                                     
 		    }
 		}
 		else if ((offset >= 8) && (offset < 12) && ((eopp&3) == 2)) {
 		    if (mapped_dest_port != start_port + 2) {
 			LOG_ERROR(BSL_LS_BCM_COMMON,
 			          (BSL_META_U(unit,
 			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
 			           start_port, mapped_dest_port));
 			return BCM_E_INTERNAL;
                                     
 		    }
 		}
 		else if ((offset >= 12) && (offset < 16) && ((eopp&7) == 0)) {
 		    if (mapped_dest_port != start_port) {
 			LOG_ERROR(BSL_LS_BCM_COMMON,
 			          (BSL_META_U(unit,
 			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
 			           start_port, mapped_dest_port));
 			return BCM_E_INTERNAL;
                                     
 		    }
 		}
 		else if ((offset >= 12) && (offset < 16) && ((eopp&7) == 1)) {
 		    if (mapped_dest_port != start_port + 1) {
 			LOG_ERROR(BSL_LS_BCM_COMMON,
 			          (BSL_META_U(unit,
 			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
 			           start_port, mapped_dest_port));
 			return BCM_E_INTERNAL;
                                     
 		    }
 		}
 		else if ((offset >= 12) && (offset < 16) && ((eopp&7) == 3)) {
 		    if (mapped_dest_port != start_port + 2) {
 			LOG_ERROR(BSL_LS_BCM_COMMON,
 			          (BSL_META_U(unit,
 			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
 			           start_port, mapped_dest_port));
 			return BCM_E_INTERNAL;
                                     
 		    }
 		}
 		else if ((offset >= 12) && (offset < 16) && ((eopp&7) == 7)) {
 		    if (mapped_dest_port != start_port + 3) {
 			LOG_ERROR(BSL_LS_BCM_COMMON,
 			          (BSL_META_U(unit,
 			                      "ERROR: EASY_RELOAD egress virtual port(%d) from portset info doesn't match port(%d) in GPORT_ADD\n"),
 			           start_port, mapped_dest_port));
 			return BCM_E_INTERNAL;
			
 		    }
 		}
  		/* Not a valid configuration...eopp setting invalid with offset */
  		else {
  		    LOG_ERROR(BSL_LS_BCM_COMMON,
  		              (BSL_META_U(unit,
  		                          "ERROR: EASY_RELOAD virtual portset info invalid start_port(%d) eopp(%d) dest_port(%d) offset(%d)\n"),
  		               start_port, eopp, dest_port, offset));
		}

	    } /* End else if virtual port */

	    /* Read sysport array table and confirm that this matches the value */
	    rv = soc_bm9600_nm_sysport_array_table_read(unit, portset, offset, &sysport_read);

	    if (sysport_read != sysport) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD sysport array table entry(%d) doesn't match sysport(%d)\n"),
		           sysport_read, sysport));
		return BCM_E_INTERNAL;

	    }

	    /* Manage sysport and portset for unicast ports */
	    if ((SOC_SBX_STATE(unit)->sysport_state[sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE) == FALSE ) {
		SOC_SBX_STATE(unit)->sysport_state[sysport].node = dest_node;
		SOC_SBX_STATE(unit)->sysport_state[sysport].port = dest_port;
		SOC_SBX_STATE(unit)->sysport_state[sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		
	    } else {
		/* the sysport already mapped, eg: add multiqueue to same sysport */
		if ( (SOC_SBX_STATE(unit)->sysport_state[sysport].node != dest_node) ||
		     (SOC_SBX_STATE(unit)->sysport_state[sysport].port != dest_port) ) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: Sysport (%d) already mapped to Node/Port(%d/%d)"),
		               sysport, SOC_SBX_STATE(unit)->sysport_state[sysport].node,
		               SOC_SBX_STATE(unit)->sysport_state[sysport].port));
		    return BCM_E_INTERNAL;
		}
	    }

	    /* Update portset state from hardware */
	    p_psstate = &ps_state[unit][portset];
	    p_psstate->in_use = TRUE;
	    _BCM_INT_BM9600_PORTSET_OFFSET_USE_SET(portset, offset);
	    p_psstate->virtual = virtual;
	    p_psstate->eopp = eopp;
	    p_psstate->start_port = start_port;
	    p_psstate->eg_node = eg_node;
	    /* NEXT and PREV read during bcm_cosq_init */
	    /* p_psstate->next = 0xFF;                 */
	    /* p_psstate->prev = 0xFF;                 */
	    p_psstate->sysports[offset] = sysport;

	    } else /* we aren't reloading */
#endif /* BCM_EASY_RELOAD_SUPPORT */

	{

	  /* Manage sysport and portset for unicast ports */
	  if ((SOC_SBX_STATE(unit)->sysport_state[sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE) == FALSE ) {
		rv = bcm_bm9600_map_sysport_to_nodeport(unit, sysport, dest_node, mapped_dest_port);
		if ( rv != BCM_E_NONE ) {
		    goto err;
		} else {
		    SOC_SBX_STATE(unit)->sysport_state[sysport].node = dest_node;
		    SOC_SBX_STATE(unit)->sysport_state[sysport].port = dest_port;
		}
		is_portset1_allocated = TRUE;
		SOC_SBX_STATE(unit)->sysport_state[sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;

		if (soc_feature(unit, soc_feature_egr_independent_fc)) {
		    rv = bcm_bm9600_map_sysport_to_nodeport(unit,
							    BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport), dest_node,
							    BCM_INT_SBX_PORT_TO_NEF_PORT(dest_port));
		    if (rv != BCM_E_NONE) {
			goto err;
		    }
		    is_portset2_allocated = TRUE;
		}
	    } else {
		/* the sysport already mapped, eg: add multiqueue to same sysport */
		if ( (SOC_SBX_STATE(unit)->sysport_state[sysport].node != dest_node) ||
		     (SOC_SBX_STATE(unit)->sysport_state[sysport].port != dest_port) ) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: Sysport (%d) already mapped to Node/Port(%d/%d)"),
		               sysport, SOC_SBX_STATE(unit)->sysport_state[sysport].node,
		               SOC_SBX_STATE(unit)->sysport_state[sysport].port));

#ifdef BCM_EASY_RELOAD_SUPPORT
		    _bcm_bm9600_cosq_free_token(unit);
#endif
		    return BCM_E_INTERNAL;
		}
	    }
	}
    }

    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_LCM_MODE) ||
       (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_XBAR_MODE)) {
       /* Don't update bag when BM not enabled */
#ifdef BCM_EASY_RELOAD_SUPPORT
        _bcm_bm9600_cosq_free_token(unit);
#endif
         return BCM_E_NONE;
     }


    /* update bag */
    rv = _bcm_bm9600_cosq_update_bag(unit, bw_group, queue_region, TRUE /* add */);
    if (rv != BCM_E_NONE) {
        goto err;
    }
#ifdef BCM_EASY_RELOAD_SUPPORT
    _bcm_bm9600_cosq_free_token(unit);
#endif
    return rv;

err:

    if (is_portset1_allocated == TRUE) {
        bcm_bm9600_unmap_sysport(unit, sysport);
    }
    if (is_portset2_allocated == TRUE) {
        bcm_bm9600_unmap_sysport(unit, (sysport + 1));
    }
#ifdef BCM_EASY_RELOAD_SUPPORT
		_bcm_bm9600_cosq_free_token(unit);
#endif
    return(rv);
}



int
bcm_bm9600_cosq_delete_queue(int unit,
                             int queue,
                             bcm_sbx_cosq_queue_region_type_t queue_region)
{
    int rv = BCM_E_UNAVAIL;
    int sysport;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    int cos;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];
    sysport = p_qstate->sysport;

    if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        return(BCM_E_NONE);
    }

    if (sysport != BCM_INT_SBX_INVALID_SYSPORT) {

#ifdef BCM_EASY_RELOAD_SUPPORT
    _bcm_bm9600_cosq_take_token(unit);
#endif
	rv = bcm_bm9600_unmap_sysport(unit, sysport);
	if ( rv != BCM_E_NONE ) {
	    return rv;
	}

	if (soc_feature(unit, soc_feature_egr_independent_fc)) {
	    rv = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
	    if ( rv != BCM_E_NONE ) {
		return rv;
	    }
	}

#ifdef BCM_EASY_RELOAD_SUPPORT
	_bcm_bm9600_cosq_free_token(unit);
#endif
    } else {
	/* Multicast queues don't have sysport */
         rv = BCM_E_NONE;
    }

    cos = p_bwstate->base_queue - queue;
    /* Last queue, delete this one */
    if (p_bwstate->num_cos == cos+1) {
      /* update bag */
      rv = _bcm_bm9600_cosq_update_bag(unit, p_qstate->bw_group, queue_region, FALSE /* delete */);
    }

    return rv;
}

int
bcm_bm9600_cosq_overlay_queue(int unit,
			      int queue,
                              bcm_sbx_cosq_queue_region_type_t queue_region,
			      int sysport,
			      int dest_node,
			      int dest_port,
			      int dest_mc,
			      int dest_cos,
			      int dest_type) {

    int rv = BCM_E_NONE;

    if ( (sysport == BCM_INT_SBX_INVALID_SYSPORT) && (dest_type != BCM_INT_SBX_DEST_TYPE_MULTICAST) ) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Sysport required for the unicast queues")));
	return BCM_E_INTERNAL;
    }

    /* map sysport to node/port */
    if ( dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST ) {
	/* Multicast reserve sysport but don't use portset
	 * Do nothing here!
	 */

    } else {
	/* Manage sysport and portset for unicast ports */
	if ((SOC_SBX_STATE(unit)->sysport_state[sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE) == FALSE ) {
	    rv = bcm_bm9600_map_sysport_to_nodeport(unit, sysport, dest_node, dest_port);
	    if ( rv != BCM_E_NONE ) {
		return rv;
	    } else {
	        SOC_SBX_STATE(unit)->sysport_state[sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		SOC_SBX_STATE(unit)->sysport_state[sysport].node = dest_node;
		SOC_SBX_STATE(unit)->sysport_state[sysport].port = dest_port;
	    }
	} else {
	    /* the sysport already mapped, eg: add multiqueue to same sysport */
	    if ( (SOC_SBX_STATE(unit)->sysport_state[sysport].node != dest_node) ||
		 (SOC_SBX_STATE(unit)->sysport_state[sysport].port != dest_port) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: Sysport (%d) already mapped to Node/Port(%d/%d)"),
		           sysport, SOC_SBX_STATE(unit)->sysport_state[sysport].node,
		           SOC_SBX_STATE(unit)->sysport_state[sysport].port));
		return BCM_E_INTERNAL;
	    }
	}
    }

    /* no need to update bag, should already be updated when queues are added */

    return rv;
}

int
bcm_bm9600_cosq_delete_overlay_queue(int unit,
				     int queue,
				     int base_queue,
                                     bcm_sbx_cosq_queue_region_type_t queue_region) {

    int rv = BCM_E_UNAVAIL;
    int sysport;
    bcm_sbx_cosq_queue_state_t *queue_state;

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    sysport = queue_state[queue].sysport;
    if (sysport == BCM_INT_SBX_INVALID_SYSPORT) {
	return BCM_E_INTERNAL;
    }

    SOC_SBX_STATE(unit)->sysport_state[sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;

    /* only need to unmap the sysport, mapping of sysport to queue is done in QE chips */
    rv = bcm_bm9600_unmap_sysport(unit, sysport);

    

    return rv;
}

int
bcm_bm9600_cosq_set_ingress_params(int unit,
				   int32 queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
				   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
				   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo) {

    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *queue_state;
    int32 bw_group;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Set ingress params for queue(%d)\n"),
              queue));
    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[queue].bw_group;

    if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_LCM_MODE) ||
       (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_XBAR_MODE)) {
       /* Don't update bag when BM not enabled */
         return BCM_E_NONE;
     }

    /* Parameters are updated with respect to the ingress parameters in */
    /* the queue state structure and the bw_group structure, not those  */
    /* passed in above                                                  */
    rv = _bcm_bm9600_cosq_update_bag(unit, bw_group, queue_region, TRUE /* add */);

    if (rv) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: set ingress params BM9600 configuration bw_group(%d) error(%d)\n"),
	           bw_group, rv));
	return rv;
    }
    return rv;
}

int
bcm_bm9600_cosq_set_template_gain(int unit,
				  int queue,
				  int template,
				  int gain)
{
    int rv = BCM_E_NONE;
    uint32 read_data;
    uint32 addr;
    sbZfFabBm9600BwR0WdtEntry_t zfWdtEntry;
    uint32 write_value = 0;
    int status;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Update WRED template, queue(%d) template(%d) gain(%d) on BM9600(%d)\n"),
              queue, template, gain, unit));

    /* this is a 32 bit entry but the field is 16 bits */
    addr = queue >> 1;

    status = soc_bm9600_BwR0WdtRead(unit, addr, (uint32*)&read_data);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "WDT entry read(0x%x)\n"),
              read_data));

    if (status == SOC_E_NONE) {

	sbZfFabBm9600BwR0WdtEntry_Unpack(&zfWdtEntry, (uint8*)&read_data, 4);

	if (queue & 1) {

	    zfWdtEntry.m_uTemplate1 = template;
	    zfWdtEntry.m_uGain1 = gain;
	    zfWdtEntry.m_uSpare1 = 0;

	} else {

	    zfWdtEntry.m_uTemplate0 = template;
	    zfWdtEntry.m_uGain0 = gain;
	    zfWdtEntry.m_uSpare0  = 0;
	}

	sbZfFabBm9600BwR0WdtEntry_Pack(&zfWdtEntry, (uint8*)&write_value, 4);
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "WDT entry value to write(0x%x)\n"),
	          write_value));

	status = soc_bm9600_BwR0WdtWrite(unit, addr, write_value);
    }
    if (status != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, soc_bm9600_BwR0WdtRead/Write, Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	rv = BCM_E_INTERNAL;
    }

    return rv;
}

int
bcm_bm9600_cosq_get_template_gain(int unit,
				  int queue,
				  int *template,
				  int *gain)
{
    uint32 read_data;
    uint32 addr;
    sbZfFabBm9600BwR0WdtEntry_t zfWdtEntry;
    int status = BCM_E_NONE;

    /* this is a 32 bit entry but the field is 16 bits */
    addr = queue >> 1;

    status = soc_bm9600_BwR0WdtRead(unit, addr, (uint32*)&read_data);
    if (status) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, soc_bm9600_BwR0WdtRead, Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
    }
    else {

	sbZfFabBm9600BwR0WdtEntry_Unpack(&zfWdtEntry, (uint8*)&read_data, 4);

	if (queue & 1) {

	    *template = zfWdtEntry.m_uTemplate1;
	    *gain = zfWdtEntry.m_uGain1;
	} else {

	    *template = zfWdtEntry.m_uTemplate0;
	    *gain = zfWdtEntry.m_uGain0;
	}

	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Read WRED template, queue(%d) template(%d) gain(%d) on BM9600(%d)\n"),
	          queue, *template, *gain, unit));

    }
    return status;
}


/* 
 * Attach an Ingress queue to an Egress fifo
 * IN:
 *
 * OUT:
 *     rv - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 */
int 
bcm_bm9600_cosq_gport_queue_attach(
    int unit, 
    uint32 flags, 
    bcm_gport_t ingress_queue, 
    bcm_cos_t ingress_int_pri, 
    bcm_gport_t egress_queue, 
    bcm_cos_t egress_int_pri, 
    int *attach_id)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    uint16 base_queue = 0, queue = 0;
    int sysport = ATTACH_ID_SYSPORT_GET(*attach_id);
    int fcd = ATTACH_ID_FCD_GET(*attach_id);
    int32 priority, hungry_priority, sched_mode, sp_priority, queue_type;
    uint32 uData;
#ifdef BCM_EASY_RELOAD_SUPPORT
    int32 portset, offset;
    int32 eopp, virtual, start_port, eg_node;
    int32 sysport_read;
    bcm_sbx_cosq_portset_state_t *p_psstate;
#endif

    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
	if (_bcm_bm9600_cosq_get_token_state(unit)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: EASY_RELOAD, cold boot required because internal state is unknown - crashed during sysport/portset setup\n")));
	    return BCM_E_INTERNAL;
	}
#endif
    }

    if(BCM_GPORT_IS_UCAST_QUEUE_GROUP(ingress_queue)) {
        base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(ingress_queue)) {
        base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(ingress_queue);
    } else {
	return(BCM_E_PARAM);
    }

    queue = base_queue + ingress_int_pri;
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];
    
    if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        return(BCM_E_UNAVAIL);
    }

    /* This code handles the case where there are 2 egress FIFOs assocated  */
    /* with a single sysport.  When there are scheduling disciplines which  */
    /* are SP to one FIFO and BAG related to the other FIFO, we need to set */
    /* the full thresholds appropriately. The scheduling discipline needs to*/
    /* be set before doing the queue_attach().                              */
    /* if we already have a sysport to FIFO mapping for this sysport, that  */
    /* means this is the second FIFO.                                       */
    if (SOC_SBX_STATE(unit)->sysport_state[sysport].fifo == -1) {
	/* This is the first FIFO for the given sysport.... */
	sched_mode = p_qstate->ingress.bw_mode;
	sp_priority = p_qstate->ingress.bw_value.sp_priority;
	
	rv = soc_sbx_sched_get_internal_state_queue_attach(unit, sched_mode, sp_priority,
							   &queue_type, &priority, &hungry_priority);


	if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: bcm_cosq_sched_set() must be called, possible error,  Unit(%d)\n"),
                       unit));	    
	    return (BCM_E_PARAM);
	}

        uData = SAND_HAL_READ_STRIDE(unit, PL, INA, p_bwstate->dest_node, INA0_PRI_FULL_THRESH);

	/* If the priority is greater than hungry, set the FIFO thesholds to '11' */
	if (priority > hungry_priority) {
            uData &= ~(0x3 << ((priority - 1) * 2));
            uData |= (0x3 << ((priority - 1) * 2));
        }

        SAND_HAL_WRITE_STRIDE(unit, PL, INA, p_bwstate->dest_node, INA0_PRI_FULL_THRESH, uData);
    }

    if (SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 1) {


#ifdef BCM_EASY_RELOAD_SUPPORT

        _bcm_bm9600_cosq_take_token(unit);

	/* If we are reloading, don't go through normal path, we just want to verify what we can from the hardware */
	if (SOC_IS_RELOADING(unit)) {

	    rv = soc_bm9600_ina_sysport_map_table_read(unit, sysport, &portset, &offset);

	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD sysport mapping table read failed for sysport(%d) coldboot required\n"),
		           sysport));
		return BCM_E_INTERNAL;
	    }

	    if (portset == 0xff) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD portset entry invalid for sysport(%d), coldboot required\n"),
		           sysport));
		return BCM_E_INTERNAL;
	    }

	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "EASY_RELOAD: portset(%d) for sysport(%d)\n"),
	              portset, sysport));

	    rv = soc_bm9600_portset_info_table_read(unit, portset, &virtual, &eopp, &start_port, &eg_node);

	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "EASY_RELOAD: portset(%d) virtual(%d) eopp(%d) start_port(%d) eg_node(%d)\n"),
	              portset, virtual, eopp, start_port, eg_node));

	    if (eg_node != p_bwstate->dest_node) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD egress node(%d) from portset info doesn't match node(%d) in GPORT_ADD\n"),
		           eg_node, p_bwstate->dest_node));
		return BCM_E_INTERNAL;
	    }

	    /* Read sysport array table and confirm that this matches the value */
	    rv = soc_bm9600_nm_sysport_array_table_read(unit, portset, offset, &sysport_read);

	    if (sysport_read != sysport) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD sysport array table entry(%d) doesn't match sysport(%d)\n"),
		           sysport_read, sysport));
		return BCM_E_INTERNAL;

	    }

	    /* Update portset state from hardware */
	    p_psstate = &ps_state[unit][portset];
	    p_psstate->in_use = TRUE;
	    _BCM_INT_BM9600_PORTSET_OFFSET_USE_SET(portset, offset);
	    p_psstate->virtual = virtual;
	    p_psstate->eopp = eopp;
	    p_psstate->start_port = start_port;
	    p_psstate->eg_node = eg_node;
	    /* NEXT and PREV read during bcm_cosq_init */
	    /* p_psstate->next = 0xFF;                 */
	    /* p_psstate->prev = 0xFF;                 */
	    p_psstate->sysports[offset] = sysport;

	} else /* we aren't reloading */
#endif /* BCM_EASY_RELOAD_SUPPORT */

	{
	    if ((SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) && soc_feature(unit, soc_feature_egr_independent_fc)) {
		if (sysport == BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport)) {
		    rv = bcm_bm9600_map_sysport_to_nodeport(unit,
							    BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport), 
							    p_bwstate->dest_node,
							    fcd);
		} else {
		    rv = bcm_bm9600_map_sysport_to_nodeport(unit,
							    BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport), 
							    p_bwstate->dest_node,
							    fcd+1);
		}
	    } else {
		rv = bcm_bm9600_map_sysport_to_nodeport(unit, sysport, p_bwstate->dest_node, fcd/2);
	    }
	    
	} /* not reloading */
	SOC_SBX_STATE(unit)->sysport_state[sysport].fcd = ATTACH_ID_FCD_GET(*attach_id);
	SOC_SBX_STATE(unit)->sysport_state[sysport].fifo = 1;
	SOC_SBX_STATE(unit)->sysport_state[sysport].egport = egress_queue;

#ifdef BCM_EASY_RELOAD_SUPPORT
	_bcm_bm9600_cosq_free_token(unit);
#endif
    } /* use_cnt=1 */
    
    p_qstate->sysport = sysport;
    p_qstate->attached_fifo = egress_int_pri;

    return rv;
}


/* 
 * Get Egress Fifo information from an Ingress queue
 * IN:
 *
 * OUT:
 *     rv - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 */
int 
bcm_bm9600_cosq_gport_queue_attach_get(
    int unit, 
    bcm_gport_t ingress_queue, 
    bcm_cos_t ingress_int_pri, 
    bcm_gport_t *egress_queue, 
    bcm_cos_t *egress_int_pri, 
    int attach_id)
{
    return BCM_E_UNAVAIL;
}


/* 
 * Detach an Ingress queue to an Egress fifo
 * IN:
 *
 * OUT:
 *     rv - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 */
int 
bcm_bm9600_cosq_gport_queue_detach(
    int unit, 
    bcm_gport_t ingress_queue, 
    bcm_cos_t ingress_int_pri, 
    int attach_id)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    uint16 base_queue = 0, queue = 0;
    int sysport = ATTACH_ID_SYSPORT_GET(attach_id);

    if(BCM_GPORT_IS_UCAST_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(ingress_queue)) {
        base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(ingress_queue);
    } else {
	return(BCM_E_PARAM);
    }

    queue = base_queue + ingress_int_pri;
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];
    
    if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        return(BCM_E_NONE);
    }

    if (SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 1) {

	if ((SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) && soc_feature(unit, soc_feature_egr_independent_fc)) {
	    if (sysport == BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport)) {
		rv = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport));
	    } else {
		rv = bcm_bm9600_unmap_sysport(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport));
	    }
	} else {
	    rv = bcm_bm9600_unmap_sysport(unit, sysport);
	    if ( rv != BCM_E_NONE ) {
		return rv;
	    }
	}
	
	SOC_SBX_STATE(unit)->sysport_state[sysport].node = -1;
	SOC_SBX_STATE(unit)->sysport_state[sysport].port = -1;
	SOC_SBX_STATE(unit)->sysport_state[sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;

	SOC_SBX_STATE(unit)->sysport_state[sysport].fcd = BCM_INT_SBX_INVALID_FCD;
	SOC_SBX_STATE(unit)->sysport_state[sysport].fifo = -1;
	SOC_SBX_STATE(unit)->sysport_state[sysport].egport = BCM_GPORT_INVALID;
    }

    p_qstate->sysport = p_qstate->default_sysport;
    p_qstate->attached_fifo = -1;

    return rv;
}


int
bcm_bm9600_cosq_set_ingress_shaper(int unit,
				   int base_queue,
				   bcm_cos_queue_t cosq,
				   int num_cos_levels,
				   uint32 shape_limit_kbps,
				   int set_logical_port_shaper,
				   int enable_shaping) {
  int rv = BCM_E_NONE;
  return rv;
}


int
bcm_bm9600_cosq_gport_discard_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard)
{
    int                            rv = BCM_E_NONE;
    sbZfFabWredParameters_t        chip_params;
    sbZfFabBm9600BwR1Wct0AEntry_t  chip_params0;
    sbZfFabBm9600BwR1Wct0BEntry_t  chip_params1;
    uint32                       write_value[2] = {~0, ~0};
    uint32                       addr;
    uint32                       status;
    int                            nbr_node_enabled;

    switch (color) {
        case BCM_SBX_COSQ_DISCARD_COLOR_GREEN_DP0:
        case BCM_SBX_COSQ_DISCARD_COLOR_YELLOW_DP1:
	case BCM_SBX_COSQ_DISCARD_COLOR_RED_DP2:
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, color (%d) out of range [0-2],  Unit(%d)\n"),
                       FUNCTION_NAME(), color, unit));
            rv = BCM_E_INTERNAL;
            goto err;
            break;
    }

    if (discard->drop_probability == 0) { /* disabling drop probability */
        write_value[0] = 0xFFFFFFFF;
#if 0
	addr = template * 8;
        addr = addr + (color * 2);

        status = soc_bm9600_BwR1Wct0AWrite(unit, addr, write_value[0]);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_bm9600_BwR1Wct0AWrite, Unit(%d), Color(%d)\n"),
                       FUNCTION_NAME(), unit, color));
            rv = BCM_E_INTERNAL;
            goto err;
        }
        status = soc_bm9600_BwR1Wct0AWrite(unit, (addr + 1), write_value[0]);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_bm9600_BwR1Wct0BWrite, Unit(%d), Color(%d)\n"),
                       FUNCTION_NAME(), unit, color));
            rv = BCM_E_INTERNAL;
            goto err;
        }
#else /* !(0) */

        addr = template * 8;
        addr = addr + (color * 2);
        switch (color) {
            case BCM_SBX_COSQ_DISCARD_COLOR_GREEN_DP0:
                status = soc_bm9600_BwR1Wct0AWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct0AWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                addr += 1;
                status = soc_bm9600_BwR1Wct0BWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct0BWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                break;

            case BCM_SBX_COSQ_DISCARD_COLOR_YELLOW_DP1:
                status = soc_bm9600_BwR1Wct1AWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct1AWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                addr += 1;
                status = soc_bm9600_BwR1Wct1BWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct1BWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                break;

            case BCM_SBX_COSQ_DISCARD_COLOR_RED_DP2:
            default:
                status = soc_bm9600_BwR1Wct2AWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct2AWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                addr += 1;
                status = soc_bm9600_BwR1Wct2BWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct2BWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                break;
        }
#endif /* !(0) */
    }
    else {
        rv = _bcm_bm9600_get_nbr_node_enabled(unit, &nbr_node_enabled);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, _bcm_bm9600_get_nbr_node_enabled, Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            goto err;
        }

        rv = _bcm_sbx_device_wred_calc_config(unit, SOC_SBX_CFG(unit)->discard_probability_mtu,
					      queue_size, discard, &chip_params);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, _bcm_sbx_device_wred_calc_config,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            goto err;
        }

        chip_params0.m_uTMinDp0 = chip_params.m_nTmin;
        chip_params0.m_uTMaxDp0 = chip_params.m_nTmax;
        sbZfFabBm9600BwR1Wct0AEntry_Pack(&chip_params0, (uint8 *)&write_value[0],
					 (SB_ZF_FAB_WRED_PARAMETERS_SIZE / 2));

        chip_params1.m_uTEcnDp0 = chip_params.m_nTecn;
        chip_params1.m_uScaleDp0 = chip_params.m_nScale;
        chip_params1.m_uSlopeDp0 = chip_params.m_nSlope;
        sbZfFabBm9600BwR1Wct0BEntry_Pack(&chip_params1, (uint8 *)&write_value[1],
					 (SB_ZF_FAB_WRED_PARAMETERS_SIZE / 2));

        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Enable WRED dp(%d) entry(%d) on BM3200(%d)\n"),
                     color, template, unit));
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "Write word0(0x%x) word1(0x%x)\n"),
                     write_value[0], write_value[1]));

#if 0
        /* This is the starting address of the template */
        addr = template * 8;
        addr = addr + (color * 2);

        status = soc_bm9600_BwR1Wct0AWrite(unit, addr, write_value[0]);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_bm9600_BwR1Wct0AWrite,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }

        status = soc_bm9600_BwR1Wct0AWrite(unit, (addr + 1), write_value[1]);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_bm9600_BwR1Wct0BWrite,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }
#else /* !(0) */

        addr = template * 8;
        addr = addr + (color * 2);
        switch (color) {
            case BCM_SBX_COSQ_DISCARD_COLOR_GREEN_DP0:
                status = soc_bm9600_BwR1Wct0AWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct0AWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                addr += 1;
                status = soc_bm9600_BwR1Wct0BWrite(unit, addr, write_value[1]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct0BWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                break;

            case BCM_SBX_COSQ_DISCARD_COLOR_YELLOW_DP1:
                status = soc_bm9600_BwR1Wct1AWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct1AWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                addr += 1;
                status = soc_bm9600_BwR1Wct1BWrite(unit, addr, write_value[1]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct1BWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                break;

            case BCM_SBX_COSQ_DISCARD_COLOR_RED_DP2:
            default:
                status = soc_bm9600_BwR1Wct2AWrite(unit, addr, write_value[0]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct2AWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                addr += 1;
                status = soc_bm9600_BwR1Wct2BWrite(unit, addr, write_value[1]);
                if (status) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, soc_bm9600_BwR1Wct2BWrite, Unit(%d), Color(%d)\n"),
                               FUNCTION_NAME(), unit, color));
                    rv = BCM_E_INTERNAL;
                    goto err;
                }

                break;
        }
#endif /* !(0) */
    }

    return(rv);

err:
    return(rv);
}

int
bcm_bm9600_cosq_gport_discard_get(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/********************/
/* Static functions */
/********************/
static int
_bcm_bm9600_cosq_update_bag(int unit,
			    int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region, int add)

{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    uint gamma = 0, sigma = 0;
    uint32 guarantee_in_kbps = 0;
    uint64 uu_epoch_length_in_ns = COMPILER_64_INIT(0,0);
    uint64 uu_guarantee_in_bytes_per_sec;
    uint32 guarantee_in_bytes_per_epoch = 0;
    int32 bag_rate_kbps;
    uint32 bag_rate_bytes_per_epoch = 0;
    uint64 uu_bag_rate_in_bytes_per_sec;
    int32 queue;
    uint32 num_queues_in_bag;
    uint32 base_queue = 0;
    int num_queues, start_queue;

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "update bw_group(%d)\n"),
                 bw_group));

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    rv = bcm_sbx_cosq_bw_group_verify_queues(unit, bw_group);

    if (rv) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: verifying queue setup for bw_group(%d) error(%d)\n"),
	           bw_group, rv));
	return rv;
    }

    rv = bcm_sbx_cosq_update_given_weights(unit, bw_group);
    if (rv) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: update WFQ weights failed\n")));
	return rv;
    }

    COMPILER_64_SET(uu_epoch_length_in_ns, 0,SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
    COMPILER_64_UMUL_32(uu_epoch_length_in_ns, SOC_SBX_STATE(unit)->fabric_state->timeslot_size); /* timeslot size ns */

    bag_rate_kbps = p_bwstate->path.bag_rate_kbps;
    uu_bag_rate_in_bytes_per_sec = uu_epoch_length_in_ns;
    COMPILER_64_UMUL_32(uu_bag_rate_in_bytes_per_sec, (bag_rate_kbps / 8));
    
    if (soc_sbx_div64(uu_bag_rate_in_bytes_per_sec, 1000000, &bag_rate_bytes_per_epoch) == -1) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: update BAG rate per epoch failed\n")));
	return BCM_E_INTERNAL;
    }
    bag_rate_bytes_per_epoch >>= SOC_SBX_CFG(unit)->demand_scale;

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "bag_rate_kbps=(%d) bag_rate_bytes_per_epoch(%d)\n"),
                 bag_rate_kbps, bag_rate_bytes_per_epoch));

    base_queue = p_bwstate->base_queue;

    /* write queue indexed BWP table */
    for (queue=base_queue; queue < (base_queue + p_bwstate->num_cos); queue++) {

	gamma = 0;
	sigma = 0;

	/* SDK-31409 stale gamma value - if deleting queue, remove this setup */
	if (add) {

	    if (p_qstate[queue].ingress.bw_mode== BCM_COSQ_SP) {
		sigma = (int32) bag_rate_bytes_per_epoch;
	    }
	    
	    if ( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP0) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP1) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP2) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP3) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP4) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP5) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP6) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP7) ) {
		sigma = (int32) bag_rate_bytes_per_epoch;
	    }
	    
	    if ( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) ||
		 (p_qstate[queue].ingress.bw_mode == BCM_COSQ_AF) ) {
		gamma = p_qstate[queue].ingress.given_weight;
	    }
	    
	    if ( (p_qstate[queue].ingress.bw_mode== BCM_COSQ_AF) ||
		 (p_qstate[queue].ingress.bw_mode== BCM_COSQ_EF) ) {

		guarantee_in_kbps = bcm_sbx_cosq_get_bw_guarantee(unit, queue);
                uu_guarantee_in_bytes_per_sec = uu_epoch_length_in_ns;
                COMPILER_64_UMUL_32(uu_guarantee_in_bytes_per_sec, (guarantee_in_kbps / 8));
		
		if (soc_sbx_div64(uu_guarantee_in_bytes_per_sec, 1000000, &guarantee_in_bytes_per_epoch) == -1) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: update Guarantee failed\n")));
		    return BCM_E_INTERNAL;
		}
		guarantee_in_bytes_per_epoch >>= SOC_SBX_CFG(unit)->demand_scale;
		
		sigma = (int32) guarantee_in_bytes_per_epoch;
		
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "queue(%d) guarantee in bytes/epoch(%d) guarantee_in_bytes_per_msec(%x%08x) epoch_length_in_ns(%x%08x)\n"),
		             queue, guarantee_in_bytes_per_epoch, COMPILER_64_HI(uu_guarantee_in_bytes_per_sec), COMPILER_64_LO(uu_guarantee_in_bytes_per_sec), 
		             COMPILER_64_HI(uu_epoch_length_in_ns), COMPILER_64_LO(uu_epoch_length_in_ns)));
	    }

	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "queue(%d) guarantee in kbps(%d) sigma(%d) gamma(%d)\n"),
	                 queue, guarantee_in_kbps, sigma, gamma));
	} /* end if (add) */
	rv = soc_bm9600_bwp_write(unit, queue, gamma, sigma);

	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BM9600 Write to BWP table failed for queue(%d)\n"),
	               queue));
	    return BCM_E_FAIL;
	}
    }

    num_queues_in_bag = 0;

    if (add) {
	num_queues_in_bag = bcm_sbx_cosq_get_num_queues_in_bag(unit, bw_group,  queue_region, &num_queues, &start_queue);

	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "num_queues_in_bag(%d)\n"),
	             num_queues_in_bag));

	/* update parameters to account for new symantics */
	num_queues_in_bag = num_queues;


	base_queue = p_bwstate->base_queue;

	/* update parameters to account for new symantics */
	num_queues_in_bag = num_queues;

	base_queue = start_queue;

#ifdef BCM_EASY_RELOAD_SUPPORT
	/* Even if there are no queues in the bag, we need to save the base queue */
	/* in case of easy reload.                                                */
	if ((start_queue == -1) && (num_queues_in_bag==0)) {
	    base_queue = p_bwstate->base_queue;
	}
#endif /* BCM_EASY_RELOAD_SUPPORT */
    }

    /* If there is nothing in the bag, clear the PRT entry information */
    if (num_queues_in_bag == 0) {

	bag_rate_bytes_per_epoch = 0;
	/* deleting this queue group */
	if (add == FALSE) {
	  base_queue = 0x3fffff;
	}
    }

    /* write PRT table used always */
    rv = soc_bm9600_bag_write(unit, bw_group, num_queues_in_bag,
			      base_queue, bag_rate_bytes_per_epoch);

    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: BM9600 Write to BAG tables failed for bw_group(%d)\n"),
	           bw_group));
	return BCM_E_FAIL;
    }
    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "Bag update successful for bw_group(%d)\n"),
                 bw_group));

    return BCM_E_NONE;
}

static int
_bcm_bm9600_get_nbr_node_enabled(int unit, int *nbr_node_enabled)
{
    int rc = BCM_E_NONE;
    uint32 uData, ina_enable=0;
    int i;

    (*nbr_node_enabled) = 0;

    for (i = 0; i < 72; i++) {
        if (i == 0) {
	    uData = SAND_HAL_READ(unit, PL, FO_CONFIG4);
	    ina_enable = SAND_HAL_GET_FIELD(PL, FO_CONFIG4, INA_ENABLE_0_23, uData);
        } else if (i == 24) {
	    uData = SAND_HAL_READ(unit, PL, FO_CONFIG5);
	    ina_enable = SAND_HAL_GET_FIELD(PL, FO_CONFIG5, INA_ENABLE_24_47, uData);
        } else if (i == 48) {
	    uData = SAND_HAL_READ(unit, PL, FO_CONFIG6);
	    ina_enable = SAND_HAL_GET_FIELD(PL, FO_CONFIG6, INA_ENABLE_48_71, uData);
        }
        if (ina_enable & (1 << (i%24))) {
            (*nbr_node_enabled)++;
        }
    }

    return(rc);
}

int
bcm_bm9600_cosq_gport_sched_config_set(int unit, bcm_gport_t gport,
				       int sched_mode, int int_pri, uint32 flags)
{
    int rv = BCM_E_NONE;
    int queue_type, priority, priority2;
    uint32 uData;
    int node;


    rv = soc_sbx_sched_config_set_params_verify(unit, sched_mode, int_pri);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    rv = soc_sbx_sched_get_internal_state(unit, sched_mode, int_pri,
					  &queue_type, &priority, &priority2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    for (node = 0; node < SB_FAB_DEVICE_BM9600_NUM_AI_PORTS; node++) {
        uData = SAND_HAL_READ_STRIDE(unit, PL, INA, node, INA0_PRI_FULL_THRESH);
        if (flags & BCM_COSQ_SCHED_CONFIG_EXPEDITE) {
            uData &= ~(0x3 << ((priority - 1) * 2));
            uData |= (0x2 << ((priority - 1) * 2));
        }
        else {
            uData &= ~(0x3 << ((priority - 1) * 2));
            uData |= (0x1 << ((priority - 1) * 2));
        }
        SAND_HAL_WRITE_STRIDE(unit, PL, INA, node, INA0_PRI_FULL_THRESH, uData);
    }

    return(rv);
}

int
bcm_bm9600_cosq_gport_sched_config_get(int unit, bcm_gport_t gport,
				       int sched_mode, int int_pri, uint32 *flags)
{
    int rv = BCM_E_NONE;
    int queue_type, priority, priority2;
    uint32 uData;


    rv = soc_sbx_sched_config_params_verify(unit, sched_mode, int_pri);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    rv = soc_sbx_sched_get_internal_state(unit, sched_mode, int_pri,
					  &queue_type, &priority, &priority2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    (*flags) = 0;
    uData = SAND_HAL_READ_STRIDE(unit, PL, INA, 0, INA0_PRI_FULL_THRESH);
    if ((uData & (0x3 << ((priority - 1) * 2))) == (0x2 << ((priority - 1) * 2))) {
        (*flags) |= BCM_COSQ_SCHED_CONFIG_EXPEDITE;
    }

    return(rv);
}


/****************************************
 * Allocate and init portset states
 */
static int
_bcm_bm9600_portset_state_init(int unit)
{
    int rv = BCM_E_NONE;
    int ps, offset;
    bcm_sbx_cosq_portset_state_t *p_psstate;
#ifdef BCM_EASY_RELOAD_SUPPORT
    int next_ps;
    int prev_ps = 0xff;
#endif

    ps_state[unit] =  sal_alloc(sizeof(bcm_sbx_cosq_portset_state_t) * BM9600_MAX_PORTSETS,
				"portset state memory");

    if (ps_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }

    for (ps = 0; ps < BM9600_MAX_PORTSETS; ps++) {
	p_psstate = &ps_state[unit][ps];
	p_psstate->in_use = FALSE;
	p_psstate->offset_in_use = 0x0;
	p_psstate->virtual = FALSE;
	p_psstate->eopp = 0;
	p_psstate->eg_node = -1;
	p_psstate->start_port = -1;
	p_psstate->next = 0xFF;
	p_psstate->prev = 0xFF;
	for (offset = 0; offset < 16; offset++) {
	    p_psstate->sysports[offset] = BCM_INT_SBX_INVALID_SYSPORT;
	}
    }

    ps_head[unit] = 0xFF;

#ifdef BCM_EASY_RELOAD_SUPPORT

    if (SOC_IS_RELOADING(unit)) {
	ps_head[unit] = SAND_HAL_READ(unit, PL, NM_PORTSET_HEAD);
	ps = ps_head[unit];

	while (ps != 0xff) {
	    /* Read the portset link table from the hardware */
	    rv = soc_bm9600_portset_link_table_read(unit, ps, &next_ps);

	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: Failed to read portset link table for row %d"),
		           ps));
		return BCM_E_INTERNAL;
	    }
	    ps_state[unit][ps].next = next_ps;
	    ps_state[unit][ps].prev = prev_ps;

	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Portset(%d) next(%d) prev(%d)\n"),
	              ps, next_ps, prev_ps));

	    prev_ps = ps;
	    ps = next_ps;

	}
    }
#endif

    return rv;
}

/****************************************
 * Allocate and init a portset entry
 */
static int
_bcm_bm9600_alloc_portset(int unit,
			  int bVirtual,
			  int eg_node,
			  int start_port,
			  int *portset)
{
    int rv = BCM_E_NONE;
    int ps, offset;
    bcm_sbx_cosq_portset_state_t *p_psstate;
    int bFound = FALSE;

    for (ps = 0; ps < BM9600_MAX_PORTSETS; ps++) {
	p_psstate = &ps_state[unit][ps];
	if ( p_psstate->in_use == FALSE ) {
	    bFound = TRUE;
	    /* init the entry */
	    p_psstate->in_use = TRUE;
	    p_psstate->offset_in_use = 0x0;
	    p_psstate->virtual = bVirtual;
	    p_psstate->eopp = 0;
	    p_psstate->start_port = start_port;
	    p_psstate->eg_node = eg_node;
	    p_psstate->next = 0xFF;
	    p_psstate->prev = 0xFF;
	    for (offset = 0; offset < 16; offset++) {
		p_psstate->sysports[offset] = BCM_INT_SBX_INVALID_SYSPORT;
	    }
	    *portset = ps;
	    break;
	}
    }

    if (bFound == FALSE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "There are no free portset.  Resource error\n")));
	rv = BCM_E_RESOURCE;
    } else {
	ps = *portset;
	p_psstate = &ps_state[unit][ps];

	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Portset %d allocated\n"),
	          ps));

	rv = _bcm_bm9600_init_portset(unit, ps, p_psstate);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to init portset %d"),
	               ps));
	    return BCM_E_INTERNAL;
	}
    }

    return rv;
}

static int
_bcm_bm9600_init_portset(int unit,
			 int ps,
			 bcm_sbx_cosq_portset_state_t *p_psstate)
{
    int rv = BCM_E_NONE;


    /* update portset info table */
    rv = soc_bm9600_portset_info_table_write(unit, ps,
					     p_psstate->virtual,
					     p_psstate->eopp,
					     p_psstate->start_port,
					     p_psstate->eg_node);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Failed to update portset info table")));
	return BCM_E_INTERNAL;
    }

    /* init the sysport array table */
    rv = soc_bm9600_nm_sysport_array_table_write(unit,
						 ps,
						 0,     /* offset 0 */
						 0xFFF, /* unused sysport */
						 1      /* new row */);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Failed to update sysport array table")));
	return BCM_E_INTERNAL;
    }

    /* init the port priority table */
    rv = soc_bm9600_portpri_init(unit, ps);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Failed to init port priority table")));
	return BCM_E_INTERNAL;
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Portset %d inited\n"),
              ps));

    return rv;
}

/****************************************
 * Insert a portset to portset linklist
 * update both ps_state and hardware tables
 */
static int
_bcm_bm9600_insert_portset(int unit,
			   int portset)
{
    int rv = BCM_E_NONE;
    int ps;
    bcm_sbx_cosq_portset_state_t *p_psstate;
    int cur_weight, weight, prev_ps, next_ps, first_ps;

    if ( ps_head[unit] == 0xFF ) {
	ps_head[unit] = portset;
	SAND_HAL_WRITE(unit, PL, NM_PORTSET_HEAD, ps_head[unit]);
	ps_state[unit][portset].next = 0xFF;
	ps_state[unit][portset].prev = 0xFF;
	return rv;
    }

    /* Insertion algorithm:
     * we manage the portset linklist is in following order
     *     portsets with same node are grouped together (we should not assume
     *       order among portsets of different nodes since the node with less
     *       ports should be in the begining of the whole linklist)
     *     physical portset are before all virtual portsets.
     *     within physical portset linklist or virtual portset linklist
     *       the port are in increasing order.
     */
    prev_ps = -1;
    first_ps = -1;
    weight = (((ps_state[unit][portset].virtual)?1:0) << 16) +
	(ps_state[unit][portset].start_port & 0xFF);

    for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {
	p_psstate = &ps_state[unit][ps];
	if ( ( p_psstate->in_use == FALSE ) ||
	     ( p_psstate->eg_node != ps_state[unit][portset].eg_node ) ) {
	    /* ignore the portset not used, or the portset has a different eg_node
	     */
	    continue;
	}

	if (first_ps == -1) {
	    /* record first portset used by the node */
	    first_ps = ps;
	}

	cur_weight = (((p_psstate->virtual)?1:0) << 16) + (p_psstate->start_port & 0xFF);
	if (cur_weight <= weight) {
	    /* record the portset which has the largest weight smaller than the
	       weight of portset to be inserted
	    */
	    prev_ps = ps;
	}
    }

    if ( ( first_ps == -1 ) ||
	 ( (first_ps != -1) && ( prev_ps == -1 ) && (ps_state[unit][first_ps].prev == 0xFF) ) ) {
	/* first portset of the node, make it head of whole linklist
	 * or not first one, but should be head of the node, and the node is head of whole linklist
	 */
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Portset %d replace old head %d\n"),
	          portset, ps_head[unit]));

	ps_state[unit][portset].prev = 0xFF;
	ps_state[unit][portset].next = ps_head[unit];

	rv = soc_bm9600_portset_link_table_write(unit, portset, ps_state[unit][portset].next);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to update portset link table for row %d"),
	               portset));
	    return BCM_E_INTERNAL;
	}

	ps_state[unit][ps_head[unit]].prev = portset;
	ps_head[unit] = portset;
	SAND_HAL_WRITE(unit, PL, NM_PORTSET_HEAD, ps_head[unit]);

    } else {
	if (prev_ps == -1) {
	    /* first portset of the node, should insert before first_ps */
	    prev_ps = ps_state[unit][first_ps].prev;
	}
	/* should insert after the prev_ps */
	ps_state[unit][portset].prev = prev_ps;
	ps_state[unit][portset].next = ps_state[unit][prev_ps].next;
	ps_state[unit][prev_ps].next = portset;

	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Portset %d insert between %d/%d\n"),
	          portset,
	          prev_ps, ps_state[unit][portset].next));

	rv = soc_bm9600_portset_link_table_write(unit, portset, ps_state[unit][portset].next);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to update portset link table for row %d"),
	               portset));
	    return BCM_E_INTERNAL;
	}

	rv = soc_bm9600_portset_link_table_write(unit, prev_ps, ps_state[unit][prev_ps].next);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to update portset link table for row %d"),
	               prev_ps));
	    return BCM_E_INTERNAL;
	}

	next_ps = ps_state[unit][portset].next;
	if (next_ps != 0xFF) {
	    ps_state[unit][next_ps].prev = portset;
	}
    }

    return rv;
}

static int
_bcm_bm9600_insert_portset_after(int unit,
				 int portset,
				 int prev_ps)
{
    int rv = BCM_E_NONE;
    int next_ps;

    if ( prev_ps == 0xFF ) {
	/* new head */
	next_ps = ps_head[unit];
	ps_head[unit] = portset;
	SAND_HAL_WRITE(unit, PL, NM_PORTSET_HEAD, ps_head[unit]);
	ps_state[unit][portset].next = next_ps;
	ps_state[unit][portset].prev = 0xFF;

	if (next_ps != 0xFF) {
	    ps_state[unit][next_ps].prev = portset;
	}
	return rv;
    }

    ps_state[unit][portset].prev = prev_ps;
    ps_state[unit][portset].next = ps_state[unit][prev_ps].next;
    ps_state[unit][prev_ps].next = portset;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Portset %d insert between %d/%d\n"),
              portset,
              prev_ps, ps_state[unit][portset].next));

    rv = soc_bm9600_portset_link_table_write(unit, portset, ps_state[unit][portset].next);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Failed to update portset link table for row %d"),
	           portset));
	return BCM_E_INTERNAL;
    }

    rv = soc_bm9600_portset_link_table_write(unit, prev_ps, ps_state[unit][prev_ps].next);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Failed to update portset link table for row %d"),
	           prev_ps));
	return BCM_E_INTERNAL;
    }

    next_ps = ps_state[unit][portset].next;
    if (next_ps != 0xFF) {
	ps_state[unit][next_ps].prev = portset;
    }

    return rv;
}

/****************************************
 * Delete a portset from portset linklist
 * update both ps_state and hardware tables
 */
static int
_bcm_bm9600_delete_portset(int unit,
			   int portset)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_portset_state_t *p_psstate;
    int prev_ps, next_ps;

    p_psstate = &ps_state[unit][portset];

    prev_ps = p_psstate->prev;
    next_ps = p_psstate->next;

    if ( prev_ps == 0xFF) {
	/* head of the whole list */
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Portset %d deleted, new ps_head %d\n"),
	          portset,
	          next_ps));

	ps_head[unit] = next_ps;
	if (ps_head[unit] == 0xFF) {
	/* Polaris device can't take 0xff as head - revert to bringup value of 0 */
	  SAND_HAL_WRITE(unit, PL, NM_PORTSET_HEAD, 0);
	} else {
	  SAND_HAL_WRITE(unit, PL, NM_PORTSET_HEAD, ps_head[unit]);
	}
    } else {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Portset %d deleted between %d/%d\n"),
	          portset,
	          prev_ps, next_ps));

	ps_state[unit][prev_ps].next = next_ps;
	rv = soc_bm9600_portset_link_table_write(unit, prev_ps, ps_state[unit][prev_ps].next);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Failed to update portset link table for row %d"),
	               prev_ps));
	    return BCM_E_INTERNAL;
	}

    }

    if (next_ps != 0xFF) {
      ps_state[unit][next_ps].prev = prev_ps;
    }

    rv = _bcm_bm9600_free_portset(unit, portset);
    if (rv != BCM_E_NONE) {
	return rv;
    }

    rv = soc_bm9600_portset_link_table_write(unit, portset, ps_state[unit][portset].next);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Failed to update portset link table for row %d"),
	           portset));
	return BCM_E_INTERNAL;
    }

    return rv;
}


/****************************************
 * Free and clear a portset entry
 */
static int
_bcm_bm9600_free_portset(int unit,
			 int portset)
{
    int rv = BCM_E_NONE;
    int offset;
    bcm_sbx_cosq_portset_state_t *p_psstate;

    if (ps_state[unit] == NULL) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Portset not inited, wrong unit?.\n")));
	return BCM_E_UNIT;
    }

    if ( (portset < 0) || (portset >= BM9600_MAX_PORTSETS)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Portset out of range (0 - %d).\n"),
	           BM9600_MAX_PORTSETS));
	return BCM_E_PARAM;
    }

    p_psstate = &ps_state[unit][portset];

    /* clear the entry */
    p_psstate->in_use = FALSE;
    p_psstate->offset_in_use = 0x0;
    p_psstate->virtual = FALSE;
    p_psstate->eopp = 0;
    p_psstate->eg_node = 0;
    p_psstate->start_port = 0;
    p_psstate->next = 0xFF;
    p_psstate->prev = 0xFF;
    for (offset = 0; offset < 16; offset++) {
	p_psstate->sysports[offset] = BCM_INT_SBX_INVALID_SYSPORT;
    }

    rv = _bcm_bm9600_init_portset(unit, portset, p_psstate);

    p_psstate->eg_node = -1;
    p_psstate->start_port = -1;

    return rv;
}

/****************************************
 * Get node/port info for a portset/offset
 */
static int
_bcm_bm9600_get_node_port_from_portset(int unit,
				       int portset,
				       int offset,
				       int *node,
				       int *port,
				       int *bIn_use)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_portset_state_t *p_psstate;

    if ( (ps_state[unit] == NULL) ||
	 (ps_state[unit][portset].in_use == FALSE) ) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Portset not inited, wrong unit?.\n")));
	return BCM_E_UNIT;
    }

    p_psstate = &ps_state[unit][portset];

    *node = p_psstate->eg_node;

    if (p_psstate->virtual) {
	/* Virtual mode */
	if ( (offset < 0) || (offset >= 16) ) {
	    *port = -1;
	} else if (offset < 4) {
	    *port = (p_psstate->start_port);
	} else if (offset < 8) {
	    *port = (p_psstate->start_port) + (p_psstate->eopp & 0x1);
	} else if (offset < 12) {
	    *port = (p_psstate->start_port) + (p_psstate->eopp & 0x1) +
		((p_psstate->eopp & 0x2) >> 1);
	} else {
	    *port = (p_psstate->start_port) + (p_psstate->eopp & 0x1) +
		((p_psstate->eopp & 0x2) >> 1) + ((p_psstate->eopp & 0x4) >> 2);
	}
    } else {
	/* Physical mode */
	*port = (p_psstate->start_port + offset);
    }

    if ( (*port == -1) || (*node == -1) ) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Port/Node retrieved from portset/offset (%d / %d) is not valid.\n"),
	           portset, offset));
	rv = BCM_E_INTERNAL;
    }

    /* return whether the portset/offset is used */
    /* coverity[negative_shift] */
    *bIn_use = _BCM_INT_BM9600_PORTSET_OFFSET_USED(portset, offset);

    return rv;
}

/****************************************
 * Get portset/offset for a node/port
 *   node[in]:        dest node
 *   port[in]:        dest port
 *   bVirtual[in]:    virtual or physical portset
 *   bAlloced[out]:   an portset/offset is allocated for this node/port in existing portsets
 *   bAvailable[out]: set if the portset/offset returned is available to use
 *   portset/offset[out]: If not available, return the last portset/offset in use for the node/port
 *                    this helps caller to find where to insert the new quad into portset linklist.
 *                    If available, return the portset/offset could be used
 */
static int
_bcm_bm9600_get_portset_from_node_port(int unit,
				       int node,
				       int port,
				       int bVirtual,
				       int *bAlloced,
				       int *bAvailable,
				       int *portset,
				       int *offset)
{

    int rv = BCM_E_NONE;
    int ps, os, os_node, os_port, os_in_use;

    *bAlloced = FALSE;
    *bAvailable = FALSE;
    *portset = -1;
    *offset = -1;

    if ( (ps_state[unit] == NULL) ) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Portset not inited, wrong unit?.\n")));
	rv = BCM_E_UNIT;
	return rv;
    }

    /* search through all linked portset */
    for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {
	if ( (ps_state[unit][ps].eg_node != node) ||
	     (ps_state[unit][ps].virtual != bVirtual) ) {
	    /* skip if node or virtual mode doesn't match */
	    continue;
	}

	if (bVirtual) {
	    /* Virtual mode portsets */
	    for (os = 0; os < 16; os++) {
		_bcm_bm9600_get_node_port_from_portset(unit, ps, os, &os_node,
						       &os_port, &os_in_use);
		if (os_port == port) {
		    *bAlloced = TRUE;
		    *portset = ps;
		    *offset = os;

		    if (os_in_use == FALSE) {
			/* return the first available portset/offset for the node/port */
			*bAvailable = TRUE;
			return rv;
		    }
		}
	    }
	} else {
	    /* Physical mode portset */
	    if ( (port >= ps_state[unit][ps].start_port) &&
		 (port < (ps_state[unit][ps].start_port + 16)) ) {
		/* return the portset/offset for the node/port regardless if it's used.
		 */
		*portset = ps;
		*offset = (port - ps_state[unit][ps].start_port);
		*bAlloced = TRUE;
		if ( _BCM_INT_BM9600_PORTSET_OFFSET_USED((*portset), (*offset)) ) {
		    *bAvailable = FALSE;
		} else {
		    *bAvailable = TRUE;
		}
	    }
	}
    }

    return rv;
}

/****************************************
 * Get number of sysport in use for node/port
 */
static int
_bcm_bm9600_get_sysport_count_for_node_port(int unit,
					    int node,
					    int port,
					    int *count)
{
    int ps, offset;
    int current_port, current_node, in_use;

    if ( (ps_state[unit] == NULL) ) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Portset not inited, wrong unit?.\n")));
	return BCM_E_UNIT;
    }

    /* search through all linked portset */
    *count = 0;
    for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {
	if ( (ps_state[unit][ps].eg_node != node) ) {
	    /* skip if node doesn't match */
	    continue;
	}

	for (offset = 0; offset < 16; offset++) {
	    _bcm_bm9600_get_node_port_from_portset(unit, ps, offset, &current_node,
						   &current_port, &in_use);
	    if ( (current_port == port) && (in_use == TRUE) ) {
	      (*count) ++;
	    }
	}
    }

    return BCM_E_NONE;
}

/****************************************
 * map sysport to node/port.
 */
int
bcm_bm9600_map_sysport_to_nodeport(int unit,
				   int sysport,
				   int node,
				   int port)
{
    int bAlloc_p, bAlloc_v, bAvailable_p, bAvailable_v;
    int portset_p, offset_p, portset_v, offset_v;
    int sysport_count = 0;
    int portset, offset;
    int rv = BCM_E_NONE;


    /* find if the portsets already allocated for the node/port */
    _bcm_bm9600_get_portset_from_node_port(unit, node, port, 0, /* Physical portsets */
					   &bAlloc_p, &bAvailable_p,
					   &portset_p, &offset_p);

    _bcm_bm9600_get_portset_from_node_port(unit, node, port, 1, /* Virtual portsets */
					   &bAlloc_v, &bAvailable_v,
					   &portset_v, &offset_v);

    /* find how many sysports are already used for the node/port */
    _bcm_bm9600_get_sysport_count_for_node_port(unit, node, port,
						&sysport_count);

    if ( sysport_count == 0 ){
	/* this is the first sysport for the node/port */
	if ( bAlloc_p == FALSE ) {
	    /* Allocate and init a physical portset when the port
	     * is not in any existing physical portsets.
	     * In physical mode, start port of portset is forced
	     * to have lower 4 bits of 0.  If we use the requesting port
	     * as the start port, portset might overlap with each other.
	     * For example, port 20 is first allocated, if we put 20 as
	     * start port, and if port 19 is requested later on, we would
	     * have to create another portset with start port as 19, port
	     * 20~34 would existing in both portsets.
	     */
	    rv = _bcm_bm9600_alloc_portset(unit, FALSE, /* physical */ node,
					   (port & 0xF0), &portset);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	    /* Insert the portset into the portset linklist */
	    rv = _bcm_bm9600_insert_portset(unit, portset);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	    /* add the sysport to the portset at offset */
	    offset = port & 0xF;
	    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, portset, offset,
						   1 /* new row */);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	} else {
	    /* Use existing physical portset */
	    portset = portset_p;
	    offset = offset_p;

	    /* Update the portset state, mark the portset/offset in use */
	    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, portset, offset,
						   0 /* old row */);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}

    } else if ( sysport_count == 1 ) {
	/* first sysport is always in physical portset,
	 * Need to move the port from physical to virtual portset
	 * virtual portset may already allocated due to sharing
	 * of virtual portset by different node/port.
	 */

	if (bAlloc_v) {
	    /* since virtual portset can support at least 4 sysports
	     * for a node/port, we can put both sysport into the quad
	     */
	    portset = portset_v;
	    offset = offset_v;
	    if ((offset & 0x3) != 0) {
		/* here offset_v should always be the first of a quad, (0,4,8,12) */
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Virtual portset should not be used when there is only 1 sysport\n"),
		           FUNCTION_NAME()));
		return BCM_E_INTERNAL;
	    }

	    /* Move existing physical port to the first offset in the new quad
	     */
	    rv = _bcm_bm9600_move_sysport(unit, portset_p, offset_p, portset, offset);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	    /* deallocate the physical portset if the portset is empty */
	    if ( !_BCM_INT_BM9600_PORTSET_USED(portset_p) ) {
		rv = _bcm_bm9600_delete_portset(unit, portset_p);
		if (rv != BCM_E_NONE) {
		    return rv;
		}
	    }

	    /* Add the new sysport to the second offset in the quad */
	    offset++;
	    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, portset, offset,
						   0 /* old row */);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	} else {
	    /* need to move quads around to make room for the sysport */
	    rv = _bcm_bm9600_rebuild_virtual_portset_for_add_delete(unit, node, port, sysport, 1 /*add*/);

	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}
    } else {
	/* already has more than one sysports for the node/port */
	if (bAvailable_v) {
	    /* has space in the virtual portset for 1 more sysport */
	    portset = portset_v;
	    offset = offset_v; /* here offset_v should always be 0 */

	    /* Add the new sysport to the available offset in the existing portset */
	    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, portset, offset,
						   0 /* old row */);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	} else {
	    /* need to move quads around to make room for the sysport */
	    rv = _bcm_bm9600_rebuild_virtual_portset_for_add_delete(unit, node, port, sysport, 1 /*add*/);

	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}
    }

    return BCM_E_NONE;
}

/****************************************
 * unmap sysport
 */
int
bcm_bm9600_unmap_sysport(int unit,
			 int sysport)
{
    int ps, os=0;
    int node, port, in_use;
    int rv = BCM_E_NONE;
    int bFound;
    bcm_sbx_cosq_portset_state_t *p_psstate;


    bFound = FALSE;
    for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {
	p_psstate = &ps_state[unit][ps];

	if ( ( p_psstate->in_use == FALSE ) ||
	     ( p_psstate->offset_in_use == 0 ) ) {
	    continue;
	}
	for (os = 0; os < 16; os++) {
	    if (p_psstate->sysports[os] == sysport) {
		bFound = TRUE;
		break;
	    }
	}
	if (bFound) {
	    break;
	}
    }

    if (bFound) {
	/* find out node/port */
	rv = _bcm_bm9600_get_node_port_from_portset(unit, ps, os, &node, &port, &in_use);
	if (rv != BCM_E_NONE) {
	    return rv;
	}

	rv = _bcm_bm9600_unmap_sysport_to_portset(unit, sysport, ps, os);
	if (rv != BCM_E_NONE) {
	    return rv;
	}

	/* if the portset is empty, remove the portset */
	if ( ps_state[unit][ps].offset_in_use == 0 ) {
	    rv = _bcm_bm9600_delete_portset(unit, ps);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	    /* if the portset is empty, we don't need to move around
	     * anything. even if the portset is in virtual mode,
	     * this means previous portset is also for this port,
	     * and the sysport offset has to be offset 0.
	     * simply remove the portset would work for all cases
	     */
	    return rv;
	}

	/* rebuild virtual portsets after deletion */
	if (ps_state[unit][ps].virtual == TRUE) {
	    rv = _bcm_bm9600_rebuild_virtual_portset_for_add_delete(unit, node, port, sysport, 0 /*delete*/);

	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}

    } else {
        /* return success if not found. It could have been deleted in the  */
        /* previous iteration. i.e. delete of the base queue results in    */
        /* deleting the entire queue group                                 */
        /* NOTE: It is best if this condition is handled at an upper layer */
	return BCM_E_NONE;
    }

    return rv;

}

/****************************************
 * get portset/offset used for a  sysport
 */
int
bcm_bm9600_get_portset_from_sysport(int unit,
				    int sysport,
				    int *portset,
				    int *offset)
{
    int rv = BCM_E_NOT_FOUND;
    int ps, os;

    if ( (ps_state[unit] == NULL) ) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "Portset not inited, wrong unit?.\n")));
	return BCM_E_UNIT;
    }

    /* search through all linked portset */
    for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {
	for (os = 0; os < 16; os++) {
	    if ( (_BCM_INT_BM9600_PORTSET_OFFSET_USED(ps, os)) &&
		 (ps_state[unit][ps].sysports[os] == sysport) ) {
		*portset = ps;
		*offset = os;
		return BCM_E_NONE;
	    }
	}
    }

    return rv;
}

/****************************************
 * map sysport to portset/offset.
 */
int
bcm_bm9600_map_sysport_to_portset(int unit,
				  int sysport,
				  int portset,
				  int offset,
				  int new_row)
{
    int rv = BCM_E_NONE;

    if (sysport == BCM_INT_SBX_INVALID_SYSPORT) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Invalid sysport %d, Unit(%d)\n"),
	           FUNCTION_NAME(), sysport, unit));
	return BCM_E_INTERNAL;
    }

    /* Update the portset state, mark the portset/offset in use */
    _BCM_INT_BM9600_PORTSET_OFFSET_USE_SET(portset, offset);
    ps_state[unit][portset].sysports[offset] = sysport;

    if ( BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	/* convert dummy sysport to real sysport */
	rv = soc_bm9600_nm_sysport_array_table_write(unit, portset, offset,
						     BCM_INT_SBX_SYSPORT_REAL(sysport), new_row);
    } else {
	rv = soc_bm9600_nm_sysport_array_table_write(unit, portset, offset,
						     sysport, new_row);
    }

    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Failed to write sysport array table, Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_INTERNAL;
    }

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	rv = soc_bm9600_ina_sysport_map_table_write_all(unit, sysport, portset, offset);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Failed to write sysport map table, Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_INTERNAL;
	}
    }

    return rv;
}

/****************************************
 * map sysport to portset/offset.
 */
static int
_bcm_bm9600_unmap_sysport_to_portset(int unit,
				     int sysport,
				     int portset,
				     int offset)
{
    int rv = BCM_E_NONE;

    if (sysport == BCM_INT_SBX_INVALID_SYSPORT) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Invalid sysport %d, Unit(%d)\n"),
	           FUNCTION_NAME(), sysport, unit));
	return BCM_E_INTERNAL;
    }

    /* Update the portset state, mark the portset/offset in use */
    _BCM_INT_BM9600_PORTSET_OFFSET_USE_CLEAR(portset, offset);
    ps_state[unit][portset].sysports[offset] = BCM_INT_SBX_INVALID_SYSPORT;

    rv = soc_bm9600_nm_sysport_array_table_write(unit, portset, offset, 0xFFF,
						 0 /* old row */);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Failed to write sysport array table, Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_INTERNAL;
    }

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(sysport) ) {
	rv = soc_bm9600_ina_sysport_map_table_write_all(unit, sysport, (uint8) BCM_INT_SBX_INVALID_SYSPORT, 0);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Failed to write sysport map table, Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_INTERNAL;
	}
    }

    return rv;
}

/****************************************
 * move sysport to new  portset/offset.
 */
static int
_bcm_bm9600_move_sysport(int unit,
			 int old_portset,
			 int old_offset,
			 int new_portset,
			 int new_offset)
{
    int old_sysport;
    int new_row;
    int rv = BCM_E_NONE;

    /* get the old sysport */
    old_sysport = ps_state[unit][old_portset].sysports[old_offset];

    /* portset maintanence */
    if (!_BCM_INT_BM9600_PORTSET_USED(new_portset)) {
	new_row = 1;
    } else {
	new_row = 0;
    }

    _BCM_INT_BM9600_PORTSET_OFFSET_USE_CLEAR(old_portset, old_offset);
    ps_state[unit][old_portset].sysports[old_offset] = BCM_INT_SBX_INVALID_SYSPORT;

    _BCM_INT_BM9600_PORTSET_OFFSET_USE_SET(new_portset, new_offset);
    ps_state[unit][new_portset].sysports[new_offset] = old_sysport;

    if (old_sysport == BCM_INT_SBX_INVALID_SYSPORT) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Invalid old sysport %d, Unit(%d)\n"),
	           FUNCTION_NAME(), old_sysport, unit));
	return BCM_E_INTERNAL;
    }

    /* remove sysport from old portset/offset in sysport array (invalide sysport 0xFFF) */
    rv = soc_bm9600_nm_sysport_array_table_write(unit, old_portset, old_offset,
						 0xFFF, 0);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Failed to move sysport(%d) from (%d/%d) to (%d/%d) in sysport array, Unit(%d)\n"),
	           FUNCTION_NAME(), old_sysport, old_portset, old_offset, new_portset,
	           new_offset, unit));
	return BCM_E_INTERNAL;
    }

    /* put sysport in new portset/offset in sysport array */
    if ( BCM_INT_SBX_SYSPORT_IS_DUMMY(old_sysport) ) {
	rv = soc_bm9600_nm_sysport_array_table_write(unit, new_portset, new_offset,
						     BCM_INT_SBX_SYSPORT_REAL(old_sysport), new_row);
    } else {
	rv = soc_bm9600_nm_sysport_array_table_write(unit, new_portset, new_offset,
						     old_sysport, new_row);
    }

    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Failed to move sysport(%d) from (%d/%d) to (%d/%d) in sysport array, Unit(%d)\n"),
	           FUNCTION_NAME(), old_sysport, old_portset, old_offset, new_portset,
	           new_offset, unit));
	return BCM_E_INTERNAL;
    }

    if ( !BCM_INT_SBX_SYSPORT_IS_DUMMY(old_sysport) ) {
	/* update sysport map table, and port priority table in INA */
	rv = soc_bm9600_move_sysport_pri(unit, old_sysport, old_portset, old_offset,
					 new_portset, new_offset);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Failed to move sysport(%d) from (%d/%d) to (%d/%d), Unit(%d)\n"),
	               FUNCTION_NAME(), old_sysport, old_portset, old_offset, new_portset,
	               new_offset, unit));
	    return BCM_E_INTERNAL;
	}
    } else {
	/* dummy sysport doesn't need to move priority */
    }

    return BCM_E_NONE;
}

static int
_bcm_bm9600_rebuild_virtual_portset_for_add_delete(int unit,
						   int node,
						   int port,
						   int sysport,
						   int add)
{
    bcm_sbx_cosq_portset_state_t *rebuild_ps_state;
    bcm_sbx_cosq_nps_info_t *nps_map;
    int bAlloc_p, bAvailable_p;
    int portset_p, offset_p;
    int nps, nps_count, sysport_count;
    int ps, os, ps_count, rebuild_ps, rebuild_os, next_ps;
    int os_node, os_port, os_in_use;
    int delete = (add == FALSE);
    int added = FALSE;
    int to_physical = FALSE;
    int to_virtual = FALSE;
    bcm_sbx_cosq_nps_info_t to_physical_sysport;
    bcm_sbx_cosq_nps_info_t to_virtual_sysport;
    int vp_offset, max_offset, curr_port;
    int node_head, node_tail;
    int rebuild_ps_count;
    int rebuilded = FALSE;
    int tmp_ps1, tmp_ps2;
    int rv = BCM_E_NONE;

    rebuild_ps_state = sal_alloc(sizeof(bcm_sbx_cosq_portset_state_t) * BM9600_MAX_PORTSETS,
				"portset state memory rebuild");

    if (rebuild_ps_state == NULL) {
      LOG_INFO(BSL_LS_BCM_COSQ,
               (BSL_META_U(unit,
                           "Failed to alloc rebuid_ps_state buffer")));
      return BCM_E_MEMORY;
    }

    nps_map = sal_alloc(sizeof(bcm_sbx_cosq_nps_info_t) * BM9600_MAX_NUM_SYSPORTS, 
			"sysport map rebuild");

    if (nps_map == NULL) {
      LOG_INFO(BSL_LS_BCM_COSQ,
               (BSL_META_U(unit,
                           "Failed to alloc nps_map buffer"))); 
      sal_free(rebuild_ps_state);
      return BCM_E_MEMORY;
    }

    /* init the new portsets */
    for (ps = 0; ps < BM9600_MAX_PORTSETS; ps++) {
	rebuild_ps_state[ps].in_use = FALSE;
	rebuild_ps_state[ps].offset_in_use = 0x0;
	rebuild_ps_state[ps].virtual = FALSE;
	rebuild_ps_state[ps].eopp = 0;
	rebuild_ps_state[ps].eg_node = -1;
	rebuild_ps_state[ps].start_port = -1;
	rebuild_ps_state[ps].next = 0xFF;
	rebuild_ps_state[ps].prev = 0xFF;
	for (os = 0; os < 16; os++) {
	    rebuild_ps_state[ps].sysports[os] = BCM_INT_SBX_INVALID_SYSPORT;
	}
    }

    /* init the node/port/sysport map */
    for (nps = 0; nps < BM9600_MAX_NUM_SYSPORTS; nps++) {
	nps_map[nps].node = -1;
	nps_map[nps].port = -1;
	nps_map[nps].sysport = BCM_INT_SBX_INVALID_SYSPORT;
	nps_map[nps].ps = -1;
	nps_map[nps].os = -1;
    }

    {
	to_virtual_sysport.node = -1;
	to_virtual_sysport.port = -1;
	to_virtual_sysport.sysport = BCM_INT_SBX_INVALID_SYSPORT;
	to_virtual_sysport.ps = -1;
	to_virtual_sysport.os = -1;

	to_physical_sysport.node = -1;
	to_physical_sysport.port = -1;
	to_physical_sysport.sysport = BCM_INT_SBX_INVALID_SYSPORT;
	to_physical_sysport.ps = -1;
	to_physical_sysport.os = -1;
    }

    /* find if need to move from physical portset to virtual portset */
    if (add == TRUE) {
	/* loop through all physical portsets of the node */
	for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {

	    if ( (ps_state[unit][ps].eg_node != node) || (ps_state[unit][ps].virtual != FALSE) ) {
		/* skip all portset not for the node or not physical */
		continue;
	    }

	    /* loop through all offsets of the physical portsets */
	    for (os = 0; os < 16; os++) {

		/* get the node/port/in_use for current offset */
		_bcm_bm9600_get_node_port_from_portset(unit, ps, os, &os_node,
						       &os_port, &os_in_use);

		if ( (os_in_use == TRUE) && (os_node == node) && (os_port == port) ) {
		    if ( ps_state[unit][ps].sysports[os] != sysport ){
			to_virtual = TRUE;
			to_virtual_sysport.node = node;
			to_virtual_sysport.port = port;
			to_virtual_sysport.sysport = ps_state[unit][ps].sysports[os];
			to_virtual_sysport.ps = ps;
			to_virtual_sysport.os = os;
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Move sysport %d from physical portset %d to virtual portset\n"),
			          to_virtual_sysport.sysport, ps));
		    }
		}
	    }
	}
    }

    /* search through all linked portset to find the head of virtual portset for the node */
    node_head = -1;
    node_tail = -1;
    for (ps = ps_head[unit]; ps != 0xFF; ps = ps_state[unit][ps].next) {
	if ( (ps_state[unit][ps].eg_node != node) ||
	     (ps_state[unit][ps].virtual != TRUE) ) {
	    /* skip if node or virtual mode doesn't match */
	    continue;
	} else {
	    if (node_head < 0) {
		node_head = ps;
	    }
	    node_tail = ps;
	}
    }

    if (node_head != -1) {
	/***************************************************
	 * virtual portset exist for the node
	 ***************************************************/

	/* find on deletion, if we need to move sysport to physical portset */
	if (delete == TRUE) {
	    /* count how many sysports are for the node/port in deletion */
	    sysport_count = 0;

	    /* loop through all virtual portsets of the node */
	    for (ps = node_head;
		 ((ps != 0xFF) && (ps_state[unit][ps].eg_node == node) && (ps_state[unit][ps].virtual == TRUE));
		 ps = ps_state[unit][ps].next) {

		/* loop through all offsets of the virtual portsets */
		for (os = 0; os < 16; os++) {

		    /* get the node/port/in_use for current offset */
		    _bcm_bm9600_get_node_port_from_portset(unit, ps, os, &os_node,
							   &os_port, &os_in_use);

		    if ( (os_in_use == TRUE) && (os_node == node) && (os_port == port) ) {
			sysport_count ++;
			if (ps_state[unit][ps].sysports[os] != sysport) {
			    to_physical_sysport.node = node;
			    to_physical_sysport.port = port;
			    to_physical_sysport.sysport = sysport;
			    to_physical_sysport.ps = ps;
			    to_physical_sysport.os = os;
			}
		    }
		}
	    }
	    if (sysport_count <= 2) {
		/* when there are less than 2 sysports for the node/port, and we need to remove one sysport,
		 * the node/port should move to physical portset
		 */
		to_physical = TRUE;
	    }
	}

	/***************************************************
	 * build node/port/sysport map, update map with the
	 * sysport add/delete
	 ***************************************************/

	/* loop through all virtual portsets of the node */
	ps_count = 0;
	for (ps = node_head, nps_count = 0;
	     ((ps != 0xFF) && (ps_state[unit][ps].eg_node == node) && (ps_state[unit][ps].virtual == TRUE));
	     ps = ps_state[unit][ps].next) {

	    ps_count ++;

	    /* loop through all offsets of the virtual portsets */
	    for (os = 0; os < 16; os++) {

		/* get the node/port/in_use for current offset */
		_bcm_bm9600_get_node_port_from_portset(unit, ps, os, &os_node,
						       &os_port, &os_in_use);

		if ((add == TRUE) && (added == FALSE) && (port < os_port)) {
		    /* if adding, add before the smallest port larger than the port */
		    if (to_virtual == TRUE) {
			/* if moving from physical to virtual, add here */
			nps_map[nps_count].node = to_virtual_sysport.node;
			nps_map[nps_count].port = to_virtual_sysport.port;
			nps_map[nps_count].sysport = to_virtual_sysport.sysport;
			nps_map[nps_count].ps = to_virtual_sysport.ps;
			nps_map[nps_count].os = to_virtual_sysport.os;
			nps_count ++;
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "To_virtual sysport %d added into nps map\n"),
			          to_virtual_sysport.sysport));
		    }
		    nps_map[nps_count].node = node;
		    nps_map[nps_count].port = port;
		    nps_map[nps_count].sysport = sysport;
		    nps_count ++;
		    added = TRUE;
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "sysport %d added into nps map\n"),
		              sysport));
		}

		if (os_in_use == TRUE) {
		    if ((add == TRUE) || (os_node != node) || (os_port != port)) {
			/* if adding, or node/port doesn't match, add to nps_map */
			nps_map[nps_count].node = os_node;
			nps_map[nps_count].port = os_port;
			nps_map[nps_count].sysport = ps_state[unit][ps].sysports[os];
			nps_map[nps_count].ps = ps;
			nps_map[nps_count].os = os;
			nps_count ++;
		    } else {
			/* if deleting and node/port match, add only if there is more than 1 sysport after deletion
			 * (not converting to physical portset) and the sysport is not the one being deleted
			 * Note: if move from virtual to physical, none of the sysport for the ports will be
			 *       added into nps_map
			 */
			if ( (to_physical == FALSE) && (sysport != ps_state[unit][ps].sysports[os]) ) {
			    nps_map[nps_count].node = os_node;
			    nps_map[nps_count].port = os_port;
			    nps_map[nps_count].sysport = ps_state[unit][ps].sysports[os];
			    nps_map[nps_count].ps = ps;
			    nps_map[nps_count].os = os;
			    nps_count ++;
			}
		    }
		}
	    }
	}

	/* it's possible that the port need to be added to the end of nps_map */
	if ((add == TRUE) && (added == FALSE)) {
	    if (to_virtual == TRUE) {
		/* if moving from physical to virtual, add here */
		nps_map[nps_count].node = to_virtual_sysport.node;
		nps_map[nps_count].port = to_virtual_sysport.port;
		nps_map[nps_count].sysport = to_virtual_sysport.sysport;
		nps_map[nps_count].ps = to_virtual_sysport.ps;
		nps_map[nps_count].os = to_virtual_sysport.os;
		nps_count ++;
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "To_virtual sysport %d added into nps map\n"),
		          to_virtual_sysport.sysport));
	    }
	    nps_map[nps_count].node = node;
	    nps_map[nps_count].port = port;
	    nps_map[nps_count].sysport = sysport;
	    nps_count ++;
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "sysport %d added into nps map\n"),
	              sysport));
	}

	for (nps = 0; nps < nps_count; nps++) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "nps_map entry %d: node %d port %d sysport %d ps %d os %d\n"),
	              nps, nps_map[nps].node,  nps_map[nps].port,  nps_map[nps].sysport,
	              nps_map[nps].ps,  nps_map[nps].os));

	}

	/***************************************************
	 * rebuild portset based on node/port/sysport map
	 * Note:
	 *   empty quads could be shared by the node/port if following are meet:
	 *   (shared port - last port in portset) <= number of empty quads).
	 *   for example:
	 *      a portset [16][16][xx][xx] would allow to be
	 *      shared by port 16, 17 or 18.
	 *      a portset [16][16][17][xx] would allow to be
	 *      shared by port 17 or 18
	 *   This sharing might result the sysport space in virtual portset
	 *   even if there is no sysport for the node/port. for example:
	 *      a portset [16][xx][xx][xx] and add port 18 to
	 *      it would make it [16][17][18][xx], a quad is
	 *      reserved for port 17, even though port 17 might not
	 *      has any sysport.
	 *   It's similiar with physical portset, when one port is
	 *   put in the portset, it allocate sysport space in the
	 *   portset for all other 15 ports.
	 ***************************************************/
	rebuild_ps = 0;
	rebuild_os = 0;
	rebuild_ps_count = 0;

	/* add all sysport into the rebuild portset */
	for (nps = 0; nps < nps_count; nps++) {
	    if (rebuild_os == 0) {
		rebuild_ps_state[rebuild_ps].in_use = TRUE;
		rebuild_ps_state[rebuild_ps].eopp = 0;
		rebuild_ps_state[rebuild_ps].virtual = TRUE;
		rebuild_ps_state[rebuild_ps].eg_node = nps_map[nps].node;
		rebuild_ps_state[rebuild_ps].start_port = nps_map[nps].port;

		/* add at rebuild_ps, rebuild_os, then move to next offset */
		rebuild_ps_state[rebuild_ps].offset_in_use = (1 << rebuild_os);
		rebuild_ps_state[rebuild_ps].sysports[rebuild_os] = nps_map[nps].sysport;

		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Add sysport %d to ps/os=%d/%d with eopp = 0x%x\n"),
		          nps_map[nps].sysport, rebuild_ps, rebuild_os, rebuild_ps_state[rebuild_ps].eopp));

		rebuild_os++;
		rebuild_ps_count++;
	    } else {
		/* vp_offset: virtual port offset of rebuild_os due to eopp
		 * max_offset: maximum addtional port offset by changing remainiing eopp bits to be 1
		 */
		if (rebuild_os <= 4) {
		    max_offset = 3;
		} else if (rebuild_os <= 8) {
		    max_offset = 2;
		} else if (rebuild_os <= 12) {
		    max_offset = 1;
		} else {
		    max_offset = 0;
		}

		if (rebuild_os < 4) {
		    vp_offset = 0;
		} else if (rebuild_os < 8) {
		    vp_offset = (rebuild_ps_state[rebuild_ps].eopp & 0x1);
		} else if (rebuild_os < 12) {
		    vp_offset = ((rebuild_ps_state[rebuild_ps].eopp & 0x1) + ((rebuild_ps_state[rebuild_ps].eopp & 0x2)>>1));
		} else {
		    vp_offset = ((rebuild_ps_state[rebuild_ps].eopp & 0x1) + ((rebuild_ps_state[rebuild_ps].eopp & 0x2)>>1) +
				 ((rebuild_ps_state[rebuild_ps].eopp & 0x4)>>2) );
		}

		curr_port = rebuild_ps_state[rebuild_ps].start_port + vp_offset;
		if (nps_map[nps].port == curr_port) {
		    /* add at rebuild_ps, rebuild_os, then move to next offset */
		    rebuild_ps_state[rebuild_ps].offset_in_use |= (1 << rebuild_os);
		    rebuild_ps_state[rebuild_ps].sysports[rebuild_os] = nps_map[nps].sysport;

		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Add sysport %d to ps/os=%d/%d with eopp = 0x%x\n"),
		              nps_map[nps].sysport, rebuild_ps, rebuild_os, rebuild_ps_state[rebuild_ps].eopp));

		    if (rebuild_os < 15) {
			added = TRUE;
			rebuild_os++;
			continue;
		    }
		} else if ( (nps_map[nps].port - curr_port) <= max_offset ) {
		    /* share portset by update eopp */
		    if ((rebuild_os & 0x3) == 0 ) {
			rebuild_ps_state[rebuild_ps].eopp |= (((1 << (nps_map[nps].port - curr_port)) - 1) << ((rebuild_os >> 2) - 1) );
		    } else {
			rebuild_ps_state[rebuild_ps].eopp |= (((1 << (nps_map[nps].port - curr_port)) - 1) << (rebuild_os >> 2) );
		    }
		    rebuild_os = ((rebuild_os + (nps_map[nps].port - curr_port) * 4 - 1) & 0xC);

		    /* add at new quad */
		    rebuild_ps_state[rebuild_ps].offset_in_use |= (1 << rebuild_os);
		    rebuild_ps_state[rebuild_ps].sysports[rebuild_os] = nps_map[nps].sysport;

		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Add sysport %d to ps/os=%d/%d with eopp = 0x%x\n"),
		              nps_map[nps].sysport, rebuild_ps, rebuild_os, rebuild_ps_state[rebuild_ps].eopp));
		    added = TRUE;
		    rebuild_os++;
		    continue;
		} else {
		    /* can not share current portset, move to next portset */
		    added = FALSE;
		}

		/* here we need to move to next portset */
		rebuild_ps++;
		rebuild_os = 0;
		rebuild_ps_state[rebuild_ps].in_use = TRUE;
		rebuild_ps_state[rebuild_ps].eopp = 0;
		rebuild_ps_state[rebuild_ps].virtual = TRUE;
		rebuild_ps_state[rebuild_ps].eg_node = nps_map[nps].node;
		rebuild_ps_state[rebuild_ps].start_port = nps_map[nps].port;

		/* if not added yet, add the node/port/sysport */
		if (added == FALSE) {
		    /* in this case, we must just allocated a new portset */
		    rebuild_ps_state[rebuild_ps].offset_in_use |= (1 << rebuild_os);
		    rebuild_ps_state[rebuild_ps].sysports[rebuild_os] = nps_map[nps].sysport;

		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Add sysport %d to ps/os=%d/%d with eopp = 0x%x\n"),
		              nps_map[nps].sysport, rebuild_ps, rebuild_os, rebuild_ps_state[rebuild_ps].eopp));

		    added = TRUE;
		    rebuild_os++;
		    rebuild_ps_count++;
		}
	    }
	}

	/***************************************************
	 * compare the rebuild portsets with old portset, move sysports based
	 * on rebuild portsets, skip all unchanged portsets
	 ***************************************************/
	rebuild_ps = 0;
	rebuilded = FALSE;

	for (rebuild_ps = 0, ps = node_head; rebuild_ps < rebuild_ps_count;  ps = ps_state[unit][ps].next, rebuild_ps++) {
	    if ( (rebuild_ps_state[rebuild_ps].start_port != ps_state[unit][ps].start_port) ||
		 (rebuild_ps_state[rebuild_ps].eopp != ps_state[unit][ps].eopp) ||
		 (rebuild_ps_state[rebuild_ps].eg_node != ps_state[unit][ps].eg_node) ) {
		rebuilded = TRUE;
		break;
	    }
	    for (os = 0; os < 16; os++) {
		if (rebuild_ps_state[rebuild_ps].sysports[os] != ps_state[unit][ps].sysports[os]) {
		    rebuilded = TRUE;
		    break;
		}
	    }
	    if (rebuilded) {
		break;
	    }
	    if (ps == node_tail) {
	        break;
	    }
	}

	if (rebuilded == FALSE) {
	    if (delete == TRUE) {
		if (to_physical == TRUE) {
		    _bcm_bm9600_get_portset_from_node_port(unit, to_physical_sysport.node, to_physical_sysport.port,
							   0, /* Physical portsets */  &bAlloc_p, &bAvailable_p,
							   &portset_p, &offset_p);
		    if ( bAlloc_p == FALSE ) {
			rv = _bcm_bm9600_alloc_portset(unit, FALSE, /* physical */ to_physical_sysport.node,
						       (to_physical_sysport.port & 0xF0), &ps);
			if (rv != BCM_E_NONE) {
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return rv;
			}
			
			/* Insert the portset into the physical porset linklist of the node */
			rv = _bcm_bm9600_insert_portset(unit, ps);
			if (rv != BCM_E_NONE) {
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return rv;
			}
			
			os = to_physical_sysport.port & 0xF;
		    } else {
			/* Use existing physical portset, in this case, the physical portset should always be available
			 * since when there are 2 or more sysports for the node/port, they are always in virtual portset
			 */
			ps = portset_p;
			os = offset_p;
		    }
                    /* coverity[negative_returns] */
		    rv = _bcm_bm9600_move_sysport(unit, to_physical_sysport.ps, to_physical_sysport.os, ps, os);
		    if (rv != BCM_E_NONE) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Failed to move sysport %d from (%d/%d) into physical portset (%d/%d) during rebuild\n"),
			          to_physical_sysport.sysport, to_physical_sysport.ps, to_physical_sysport.os, ps, os));
			sal_free(rebuild_ps_state);
			sal_free(nps_map);
			return rv;
		    } else {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Moved sysport %d from (%d/%d) into physical portset (%d/%d) during rebuild\n"),
			          to_physical_sysport.sysport, to_physical_sysport.ps, to_physical_sysport.os, ps, os));
		    }

		    if ( !_BCM_INT_BM9600_PORTSET_USED(to_physical_sysport.ps) ) {
			rv = _bcm_bm9600_delete_portset(unit, to_physical_sysport.ps);
			if (rv != BCM_E_NONE) {
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return rv;
			}
		    }
		}

		/* When deleting, it's possible the deletion don't lead to any eop change */
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return BCM_E_NONE;
	    } else {
	        if ((add == TRUE) && (added == TRUE)) {
		    /* adding a virtual portset at the end of portsets for the node */
		} else {
		    /* When adding, we should find something different, at least sysport add/deleted would be different */
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Failed to detect portset difference during rebuild\n")));
		    sal_free(rebuild_ps_state);
		    sal_free(nps_map);
		    return BCM_E_INTERNAL;
		}
	    }
	}

	/* After considering all case, worst case we need 2 extra empty portsets to
	 * move things around. allocated 2 empty portsets and link right before the
	 * changed portset
	 */
	rv = _bcm_bm9600_alloc_portset(unit, TRUE, /* virtual */ node, rebuild_ps_state[rebuild_ps].start_port, &tmp_ps1);
	if (rv != BCM_E_NONE) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Failed to alloc temp portsets during rebuild\n")));
	    sal_free(rebuild_ps_state);
	    sal_free(nps_map);
	    return rv;
	}

	rv = _bcm_bm9600_insert_portset_after(unit, tmp_ps1, ps_state[unit][ps].prev);
	if (rv != BCM_E_NONE) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Failed to insert temp portsets during rebuild\n")));
	    sal_free(rebuild_ps_state);
	    sal_free(nps_map);
	    return rv;
	}

	if ((rebuild_ps_count - rebuild_ps) > 1) {
	    rv = _bcm_bm9600_alloc_portset(unit, TRUE, /* virtual */ node, rebuild_ps_state[rebuild_ps+1].start_port, &tmp_ps2);
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Failed to alloc 2nd temp portsets during rebuild\n")));
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    }

	    /* Insert the portset into the porset linklist */
	    rv = _bcm_bm9600_insert_portset_after(unit, tmp_ps2, tmp_ps1);
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Failed to insert 2nd temp portsets during rebuild\n")));
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    }
	}

	/* move the sysport to physical portset, alloc new portset if necessary */
	if (to_physical == TRUE) {
	    _bcm_bm9600_get_portset_from_node_port(unit, to_physical_sysport.node, to_physical_sysport.port,
						   0, /* Physical portsets */  &bAlloc_p, &bAvailable_p,
						   &portset_p, &offset_p);
	    if ( bAlloc_p == FALSE ) {
		rv = _bcm_bm9600_alloc_portset(unit, FALSE, /* physical */ to_physical_sysport.node,
					       (to_physical_sysport.port & 0xF0), &ps);
		if (rv != BCM_E_NONE) {
		    sal_free(rebuild_ps_state);
		    sal_free(nps_map);
		    return rv;
		}

		/* Insert the portset into the physical porset linklist of the node */
		rv = _bcm_bm9600_insert_portset(unit, ps);
		if (rv != BCM_E_NONE) {
		    sal_free(rebuild_ps_state);
		    sal_free(nps_map);
		    return rv;
		}

		os = to_physical_sysport.port & 0xF;
	    } else {
		/* Use existing physical portset, in this case, the physical portset should always be available
		 * since when there are 2 or more sysports for the node/port, they are always in virtual portset
		 */
		ps = portset_p;
		os = offset_p;
	    }
	    rv = _bcm_bm9600_move_sysport(unit, to_physical_sysport.ps, to_physical_sysport.os, ps, os);
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Failed to move sysport %d from (%d/%d) into physical portset (%d/%d) during rebuild\n"),
		          to_physical_sysport.sysport, to_physical_sysport.ps, to_physical_sysport.os, ps, os));
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    } else {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Moved sysport %d from (%d/%d) into physical portset (%d/%d) during rebuild\n"),
		          to_physical_sysport.sysport, to_physical_sysport.ps, to_physical_sysport.os, ps, os));
	    }
	}

	/* move sysports from old portsets to the rebuilded portset, the common portion are not touched */
	for (ps = tmp_ps1; rebuild_ps < rebuild_ps_count;
	     rebuild_ps++, ps = ps_state[unit][ps].next) {
	    if ( _BCM_INT_BM9600_PORTSET_USED(ps) ) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Portset %d should be empty before rebuild\n"),
		          ps));
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return BCM_E_INTERNAL;
	    }

	    /* init the portset based on the eopp, virtual, start node/port of the rebuild portset info */
	    rv = _bcm_bm9600_init_portset(unit, ps, &rebuild_ps_state[rebuild_ps]);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: Failed to init portset %d"),
		           ps));
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return BCM_E_INTERNAL;
	    }
	    ps_state[unit][ps].in_use = rebuild_ps_state[rebuild_ps].in_use;
	    ps_state[unit][ps].virtual = rebuild_ps_state[rebuild_ps].virtual;
	    ps_state[unit][ps].eopp = rebuild_ps_state[rebuild_ps].eopp;
	    ps_state[unit][ps].start_port = rebuild_ps_state[rebuild_ps].start_port;
	    ps_state[unit][ps].eg_node = rebuild_ps_state[rebuild_ps].eg_node;
	    ps_state[unit][ps].offset_in_use = 0x0;
	    for (os = 0; os < 16; os++) {
		ps_state[unit][ps].sysports[os] = BCM_INT_SBX_INVALID_SYSPORT;
	    }

	    /* go through all offset and move sysports into the portset */
	    for (os = 0; os < 16; os++) {
		if ( (rebuild_ps_state[rebuild_ps].offset_in_use & (1 << os)) != 0 ) {
		    /* the offset should have some sysport */
		    if ( (add == TRUE) && (rebuild_ps_state[rebuild_ps].sysports[os] == sysport) ) {
			/* the newly added sysport, if offset is 0, treat as new row */
			rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, ps, os, (os == 0)?1:0);
			if (rv != BCM_E_NONE) {
			    LOG_INFO(BSL_LS_BCM_COSQ,
			             (BSL_META_U(unit,
			                         "Failed to add sysport %d into portset during rebuild\n"),
			              sysport));
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return rv;
			}
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Adding sysport %d to (%d/%d)\n"),
			          sysport, ps, os));
		    } else {
			for (nps = 0; nps < nps_count; nps++) {
			    if (rebuild_ps_state[rebuild_ps].sysports[os] == nps_map[nps].sysport) {
				/* found matching sysport in the map */
				break;
			    }
			}

			if (nps == nps_count) {
			    /* should always find the matching sysport in the map */
			    LOG_INFO(BSL_LS_BCM_COSQ,
			             (BSL_META_U(unit,
			                         "Failed to find sysport %d in the node/port/sysport map\n"),
			              rebuild_ps_state[rebuild_ps].sysports[os]));
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return BCM_E_INTERNAL;
			}

			/* move the sysport to new ps/os in the rebuild portset */
			if (ps_state[unit][nps_map[nps].ps].sysports[nps_map[nps].os] != rebuild_ps_state[rebuild_ps].sysports[os]) {
			    LOG_INFO(BSL_LS_BCM_COSQ,
			             (BSL_META_U(unit,
			                         "nps sysport %d not maching with ps sysport %d\n"),
			              rebuild_ps_state[rebuild_ps].sysports[os], ps_state[unit][nps_map[nps].ps].sysports[nps_map[nps].os]));
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return BCM_E_INTERNAL;
			}

			rv = _bcm_bm9600_move_sysport(unit, nps_map[nps].ps, nps_map[nps].os, ps, os);
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Moving sysport %d from ps/os (%d/%d) to (%d/%d)\n"),
			          rebuild_ps_state[rebuild_ps].sysports[os], nps_map[nps].ps,
			          nps_map[nps].os, ps, os));
			if (rv != BCM_E_NONE) {
			    sal_free(rebuild_ps_state);
			    sal_free(nps_map);
			    return rv;
			}
		    }
		}
	    }
	}

	/* remove unused portsets at the end of rebuild portsets */
	for (; ((ps != 0xFF) && (ps_state[unit][ps].eg_node == node)); ps = next_ps) {
	    next_ps = ps_state[unit][ps].next;
	    rv = _bcm_bm9600_delete_portset(unit, ps);
	    if (rv != BCM_E_NONE) {
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    }
	}
	if (to_virtual == TRUE) {
	    /* if move sysport to virtual portset make the physical portset empty, delete it */
	    if ( !_BCM_INT_BM9600_PORTSET_USED(to_virtual_sysport.ps) ) {
		rv = _bcm_bm9600_delete_portset(unit, to_virtual_sysport.ps);
		if (rv != BCM_E_NONE) {
		    sal_free(rebuild_ps_state);
		    sal_free(nps_map);
		    return rv;
		}
	    }
	}
    } else {
	/***************************************************
	 * no virtual portset exist for the node
	 ***************************************************/
	if (add == TRUE) {
	    /* allocated a new portset for the node */
	    rv = _bcm_bm9600_alloc_portset(unit, TRUE, /* virtual */ node, port, &ps);
	    if (rv != BCM_E_NONE) {
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    }

	    /* Insert the portset into portset linklist */
	    rv = _bcm_bm9600_insert_portset(unit, ps);
	    if (rv != BCM_E_NONE) {
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    }

	    /* move sysport from physical to virtual portset if needed */
	    os = 0;
	    if (to_virtual == TRUE) {
		rv = _bcm_bm9600_move_sysport(unit, to_virtual_sysport.ps, to_virtual_sysport.os, ps, os++);
		if (rv != BCM_E_NONE) {
		    sal_free(rebuild_ps_state);
		    sal_free(nps_map);
		    return rv;
		}
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Moving sysport %d from ps/os (%d/%d) to virtual ps/os(%d/%d)\n"),
		          ps_state[unit][to_virtual_sysport.ps].sysports[to_virtual_sysport.os],
		          to_virtual_sysport.ps, to_virtual_sysport.os, ps, os));
	    }

	    /* add the sysport into virtual portset */
	    rv = bcm_bm9600_map_sysport_to_portset(unit, sysport, ps, os, 0 /* old row */);
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Failed to add sysport %d into portset\n"),
		          sysport));
		sal_free(rebuild_ps_state);
		sal_free(nps_map);
		return rv;
	    }

	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Adding sysport %d to (%d/%d)\n"),
	              sysport, ps, os));
	}
    }

    sal_free(rebuild_ps_state);
    sal_free(nps_map);
    return rv;
}

#ifdef BCM_EASY_RELOAD_SUPPORT
/* The register bw_debug_mux_config.dm_sel is used as a semaphore scratchpad */
/* in the hardware it is a debug mux config field - which should never be    */
/* used with easy reload.                                                    */
static int
_bcm_bm9600_cosq_get_token_state(int unit)
{
    int bw_debug_mux_config;
    int token_value;

    bw_debug_mux_config = SAND_HAL_READ(unit, PL, BW_DEBUG_MUX_CONFIG);
    token_value = SAND_HAL_GET_FIELD(PL, BW_DEBUG_MUX_CONFIG, DM_SEL, bw_debug_mux_config);

    if (token_value == 0xf) {
	return TRUE;
    }else {
	return FALSE;
    }
}
static int
_bcm_bm9600_cosq_take_token(int unit)
{
    int bw_debug_mux_config;
    int token_value = 0xf;

    bw_debug_mux_config = SAND_HAL_READ(unit, PL, BW_DEBUG_MUX_CONFIG);
    bw_debug_mux_config = SAND_HAL_SET_FIELD(PL, BW_DEBUG_MUX_CONFIG, DM_SEL, token_value);
    SAND_HAL_WRITE(unit, PL, BW_DEBUG_MUX_CONFIG, bw_debug_mux_config);

    return BCM_E_NONE;
}

static int
_bcm_bm9600_cosq_free_token(int unit)
{
    int bw_debug_mux_config;
    int token_value = 0x0;

    bw_debug_mux_config = SAND_HAL_READ(unit, PL, BW_DEBUG_MUX_CONFIG);
    bw_debug_mux_config = SAND_HAL_SET_FIELD(PL, BW_DEBUG_MUX_CONFIG, DM_SEL, token_value);
    SAND_HAL_WRITE(unit, PL, BW_DEBUG_MUX_CONFIG, bw_debug_mux_config);
    return BCM_E_NONE;
}


#endif

