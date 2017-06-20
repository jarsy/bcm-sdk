/*
 * $Id: cosq.c,v 1.110 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 *
 * QE2000 Specific implementation
 *
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_counter.h>
#include <soc/sbx/qe2000_scoreboard.h>
#include <soc/sbx/fabric/sbZfFabWredParameters.hx>
#include <soc/sbx/fabric/sbZfFabQe2000BwPortConfigEntry.hx>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/qe2000_intr.h>
#include <soc/sbx/g2eplib/sbG2Eplib.h>
#include <soc/sbx/g2eplib/sbG2EplibTable.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/device_cosq.h>
#include <bcm_int/sbx/qe2000.h>
#include <bcm_int/sbx/device_wred.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/port.h>

#define TME_ARB_PORT_OPTIMIZE 1
/* #define DEBUG_TME_ARB_PORT_OPTIMIZE 1 */

/*
 * Temporary Fix.
 * Already pending work item is to move to dynamic allocation of
 * data structures.
 */
#define HW_QE2000_MAX_UNIT_NBR     6
#define MAX_VIRTUAL_NODE_PORT_UNICAST     991
#define MIN_VIRTUAL_NODE_PORT_MULTICAST  (MAX_VIRTUAL_NODE_PORT_UNICAST + 1)
#define MAX_VIRTUAL_NODE_PORT_MULTICAST   BCM_SBX_DEVICE_COSQ_MAX_LNAS


arb_group_t *level_two_arb_group[SOC_MAX_NUM_DEVICES];


int32 min_buffers_for_queue[HW_QE2000_MAX_UNIT_NBR][HW_QE2000_MAX_QUEUES];

virtual_node_port_t virtual_node_port[HW_QE2000_MAX_UNIT_NBR][BCM_SBX_DEVICE_COSQ_MAX_LNAS];


/* static function prototypes */
static int
_bcm_qe2000_cosq_update_bag(int unit,
                int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region);
static void
_bcm_qe2000_cosq_queue_depth_denormalize(int32 *p_min_physical_queue_depth_in_bytes, int32 *p_max_physical_queue_depth_in_bytes, sbBool_t bAdd);

static int
_bcm_qe2000_cosq_init_virtual_node_port(int unit);

static int32
_bcm_qe2000_cosq_allocate_virtual_node_port(int unit, int32 base_queue, int32 dest_node, int32 dest_port, int32 multicast);



static int32
_bcm_qe2000_cosq_get_virtual_node_port(int unit, int32 base_queue);

static int32
_bcm_qe2000_cosq_get_virtual_node_port_ref_count(int unit, int32 base_queue);

static int32
_bcm_qe2000_cosq_free_virtual_node_port(int unit, int32 base_queue);

static void _bcm_qe2000_cosq_anemic_watermark_select_for_params_chosen(bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                                       int32 *p_anemic_watermark_select);

static int
_bcm_qe2000_cosq_add_delete_queue(int unit, int add, int32 queue, int32 rate_delta_max_index,
                  bcm_sbx_cosq_queue_params_ingress_t *p_qparams);

static int
_bcm_qe2000_cosq_select_queue_params(int unit, int queue, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                     int32 *p_hold_ts, int32 *p_qtype,
                     uint32 *p_rate_a, uint32 *p_rate_b,
                     bcm_sbx_cosq_queue_region_type_t queue_region);

static int
_bcm_qe2000_cosq_set_ingress_shaper(int unit,
                    int32 queue,
                    uint32 shape_limit_kbps,
                    int enable_shaping);

static int
_bcm_qe2000_cosq_set_queue_group_ingress_shaper(int unit,
                        int base_queue,
                        int num_cos_levels,
                        uint32 shape_limit_kbps,
                        int enable_shaping);
static void
_bcm_qe2000_anemic_age_threshold_key_for_params_chosen(int unit, int queue, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                               int32 *p_anemic_age_threshold_key);

static void
_bcm_qe2000_rate_delta_max_index_for_port_rate_chosen(int unit, int bw_mode, uint32 bag_rate_kbps,
                              int32 *prate_delta_max_index);

static int
_bcm_qe2000_cosq_enable_disable_queue(int unit, int queue, int enable);

static int
_bcm_qe2000_cosq_map_sp2gsp(int unit, int queue, int32 *p_bw_mode, int32 *p_sp_priority);

static int
_bcm_qe2000_cosq_update_gsp(int unit, int queue);

static int
_bcm_qe2000_cosq_queue_packet_length_adjust_get(int unit, int queue, int *adjust_p);

static int
_bcm_qe2000_cosq_queue_packet_length_adjust_set(int unit, int queue, int adjust);

static int
_bcm_qe2000_cosq_queue_burst_set(int unit, int base_queue, bcm_cos_queue_t cosq, int num_cos, int queue, int burst_rate_kbits, int is_burst_size_context);

static int
_bcm_qe2000_cosq_queue_burst_get(int unit, int base_queue, bcm_cos_queue_t cosq, int num_cos, int queue, int *burst_rate_kbits, int is_burst_size_context);

static INLINE uint32 _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(int unit, uint32 per_queue_shape_burst_kbps);
static INLINE uint32 _bcm_qe2000_cosq_burst_get(int unit, int num_cos, uint32 shape_rate_256_kbits);
static INLINE uint32 _bcm_qe2000_cosq_queuegroup_to_queue_shape_burst_get(int unit, int num_cos, uint32 queuegroup_shape_burst_kbits);

int
bcm_qe2000_cosq_init(int unit)
{
    int num_cos, rv = BCM_E_NONE;
    int32 queue;
    int32 bw_group;
    uint32 data[4] = {0,0,0,0};
    sbZfFabQe2000BwPortConfigEntry_t zfPortConfigEntry;
#ifdef BCM_EASY_RELOAD_SUPPORT
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
#endif

    /* Initialize the scoreboard thread */
    rv = soc_sbx_qe2000_scoreboard_init(unit);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Unit(%d) error starting scoreboard thread"),
                   unit));
	return rv;
    } 

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);
 
    if (num_cos < 1) {
        num_cos = 1;
    } else if (NUM_COS(unit) && num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }
 
    for ( queue=0; queue < (SOC_SBX_CONTROL(unit)->numQueues * num_cos); queue++ ) {
         min_buffers_for_queue[unit][queue] = 0x0;
    }


    /* SDK-24645 early demand request error event, enable - interrupt */
    if (! (soc_feature(unit, soc_feature_standalone)) ) { 
	   soc_qe2000_qmgr_intr_enable(unit);
    }

    if (! (soc_feature(unit, soc_feature_standalone) || soc_feature(unit, soc_feature_hybrid)) ) {
        return rv;
    }

    if (unit >= HW_QE2000_MAX_UNIT_NBR) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Unexpected Unit, Max Qe2000 Unit(%d) Unit(%d)"),
                   HW_QE2000_MAX_UNIT_NBR, unit));
        return(rv);
    }

    sbZfFabQe2000BwPortConfigEntry_InitInstance(&zfPortConfigEntry);

    /* Initialize list of virtual node ports */
    rv = _bcm_qe2000_cosq_init_virtual_node_port(unit);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: VNP memory allocation error\n")));
    }


    /* Force Q2EC memory to harmless default values so that when read in EASY_RELOAD           */
    /* the hardware is in a state indicating that the arb port is not in use for local queues  */
    for ( queue=0; queue<SOC_SBX_CONTROL(unit)->numQueues; queue++ ) {
	rv = soc_qe2000_q2ec_set(unit, queue, 0, 63, 0, 0);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: timeout writing to q2ec memory queue(%d)"),
	               queue));
	    return BCM_E_TIMEOUT;
	}
    }

    /* Read  table from the hardware if in EASY_RELOAD mode
     * write queue to bw_group_state index and write bw_group index to the base queue_state.
     */

    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT

	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

	    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
	    p_bwstate = &p_bwstate[bw_group];

	    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

	    /* Read entry */
	    rv = soc_qe2000_qm_mem_read(unit, bw_group, /* addr */
					0xC, /* BW Port Config table */
					&data[0],
					&data[1],
					&data[2],
					&data[3]);

	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: EASY_RELOAD: Reading Bw port configuration entry bw_group(%d) failed.\n"),
		           bw_group));
		return BCM_E_TIMEOUT;
	    }

	    sbZfFabQe2000BwPortConfigEntry_Unpack(&zfPortConfigEntry, (uint8*)data, 6);

	    /* Update the number of queues and the number of strict priority queues
	     */
	    if (zfPortConfigEntry.m_nBaseQueue < SOC_SBX_CFG(unit)->num_queues) {
		p_bwstate->base_queue = zfPortConfigEntry.m_nBaseQueue;
		p_qstate[zfPortConfigEntry.m_nBaseQueue].bw_group = bw_group;
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "EASY_RELOAD bw_group(%d) base_queue(%d)\n"),
		          bw_group, p_bwstate->base_queue));
	    }
	}
#endif
    } else {
    /* If we are not reloading, then invalidate all BWP entries base_queue during cosq init */
    /* This is how we know the entry is not in use when reloading                           */
	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

	    zfPortConfigEntry.m_nLineRate = 0;
	    zfPortConfigEntry.m_nQueues = 0;
	    zfPortConfigEntry.m_nSpQueues = 0;
	    zfPortConfigEntry.m_nBaseQueue = 0x3fffff;

	    sbZfFabQe2000BwPortConfigEntry_Pack(&zfPortConfigEntry, (uint8*)data, 6);

	    /* Write entry */
	    rv = soc_qe2000_qm_mem_write(unit,
					 bw_group /* addr */,
					 0xC /* BW Port Config table */,
					 data[0],
					 data[1],
					 data[2],
					 data[3]);

	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: Writing Bw port configuration table entry (%d) failed.\n"),
		           bw_group));
		return BCM_E_FAIL;
	    }
	}
    }
    return rv;
}

#define BCM_QE2000_COSQ_MULTICAST_PORT          (50)

int
_bcm_qe2000_cosq_add_queue(int unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region,
                          int sysport, int dest_node, int dest_port, int dest_mc, int dest_cos,
                          int32 dest_type, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                          bcm_sbx_cosq_bw_group_params_t *p_bwparams, int is_user_invoked)
{

    int32 vnp_id = -1;
    int32 status;
    int32 mc=0;
    int32 rate_delta_max_index = 0;
    uint32 rate_a = 0, rate_b = 0;
    int32 anemic_age_threshold_key;
    int32 qtype;
    int32 hold_ts;
    bcm_sbx_cosq_queue_state_t *queue_state;
    int32 bw_group;
    int32 idx = 0;
    int32 remap_node = dest_node;
    int32 remap_port = dest_port;
    /* int32 initial_sysport = sysport; */
    int32 ef;
    int32 is_allocated = FALSE;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Adding queue(%d)\n"),
              queue));

    if (queue_region == bcm_sbx_cosq_queue_region_local) {
        /* In TME mode, the multicast determination is made during the */
        /* EG fifo remap, not based on queue type.  In fact, the LNA   */
        /* requires that MC=0 at all times.  Set dest port to mcast    */
        /* fifo                                                        */
        mc = 0;
         /*  make this conditional on MC dest */
        if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
             dest_port = BCM_QE2000_COSQ_MULTICAST_PORT;
        }

    } else if ( (sysport != -1) && ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
				    (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) ) {
        /* If sysport is allocated, Vport or Vport Legacy mode and expanded esets
         * use sysport to overwrite node/port/mc.
         */
        if (soc_feature(unit, soc_feature_egr_independent_fc)) {
            queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
            if (queue_state[queue].ingress.bw_mode == BCM_COSQ_EF) {
                sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
            }
            else {
                sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
            }
        }

        BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_SYSPORT(unit, sysport, &dest_node, &dest_port, &dest_mc);
        mc = dest_mc;

        if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Adding queue(%d) as multicast queue\n"),
                      queue));
        } else {
	    /* need to save the node/port used for the sysport, APS feature
	     * require this info even on qe2000
	     */
	  
	    /* If in egress independent fifo mode, two sysports are used, but only
             * the ef sysport is allocated, its non-ef sysport is marked reserved.
	     * Need to set the node for the ef sysport, so when the sysport is deallocated it 
	     * will decrement the correct node_cnt for the ef sysport */
	  if (soc_feature(unit,soc_feature_egr_independent_fc)) {
	    SOC_SBX_STATE(unit)->sysport_state[BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport)].node = remap_node;
	    SOC_SBX_STATE(unit)->sysport_state[BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport)].port = remap_port;
	  } else {
	    SOC_SBX_STATE(unit)->sysport_state[sysport].node = remap_node;
	    SOC_SBX_STATE(unit)->sysport_state[sysport].port = remap_port;
	  }
	}

    } else if ( (sysport != -1) && (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) ) {
        /* sysport is not allocated for multicast (DMode) */
        /* It is alloacted to handle hybrid configuration */
        dest_node = BCM_INT_SBX_DMODE_SYSPORT_NODE_GET(sysport);
        dest_port = BCM_INT_SBX_DMODE_SYSPORT_PORT_GET(sysport);
        mc = 0;

    } else if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) { /* (sysport == -1) */
        /* If sysport is not allocated and multicast, it's first 128 eset,
         * use node/port/mc specified
         */
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Adding queue(%d) as multicast queue\n"),
                  queue));

        mc = 1;

    } else { /* (sysport == -1) */
        /* If sysport is not allocated and unicast, it's DMode unicast */
        mc = 0;
    }

    /* Write the LNA memory to fixed configuration if standalone */
    if (queue_region == bcm_sbx_cosq_queue_region_local) {


	/* only allocate for base queue in queue group */
        if ((is_user_invoked == TRUE) && (dest_cos == 0)) {
            vnp_id = _bcm_qe2000_cosq_allocate_virtual_node_port(unit, queue, dest_node, dest_port, 
								 (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) ? 1:0);

            if (vnp_id < 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "Error: virtual node port allocation failed\n")));
		if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
		    status = BCM_E_INTERNAL;
#endif
		} else {
		    return BCM_E_RESOURCE;
                }
		goto err;
	    }
	    is_allocated = TRUE;
	} else {
	    vnp_id = _bcm_qe2000_cosq_get_virtual_node_port(unit, (queue & 0xfff0));
	    if (vnp_id == -1){
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: Could not find virtual np ")));
		status = BCM_E_MEMORY;
		goto err;
	    }
	}

	if ((is_user_invoked == TRUE) &&
	    (_bcm_qe2000_cosq_get_virtual_node_port_ref_count(unit, (queue & 0xfff0)) == 1)) {
	    status = soc_qe2000_lna_mem_full_remap_table_write(unit, vnp_id, dest_port);

	    if (status != 0) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: timeout on lna memory access")));
		status = BCM_E_TIMEOUT;
		goto err;
	    }

            /* the FIFO configuration is as follows - 0&1 are associated with port 0 */
            /*                                        2&3 are associated with port 1 */
            /*                                        etc...                         */
            /* the first FIFO in the group is for EF, second non-EF                  */
	    ef = (p_qparams->bw_mode == BCM_COSQ_EF);
	    status = soc_qe2000_eg_mem_port_remap_table_write(unit, vnp_id,
                                                              dest_type,
							      1, /* tme remap */
                                                              1, /* enable fifo */
                                                              (dest_port*2 + ((ef)?0:1)) /* fifo number */);
	    if (status != 0) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: timeout on port remap memory access")));
		status = BCM_E_TIMEOUT;
		goto err;
	    }
	}

        /* do e2q_set for all cos levels using vnp value of base queue */
        if(soc_qe2000_e2q_set(unit, 0, (vnp_id>>5)&0x1F, vnp_id&0x1F , (queue & ~(0xF)), 1) != SOC_E_NONE ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: E2Q write failure")));
            status = BCM_E_MEMORY;
            goto err;
        }
        if (soc_qe2000_q2ec_set(unit, queue, 0, (vnp_id>>5)&0x1F, vnp_id&0x1F, (queue & 0xF)) != SOC_E_NONE ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Q2EC write failure\n")));
            status = BCM_E_MEMORY;
            goto err;
        }

    } else {

        if (soc_qe2000_e2q_set(unit, mc, dest_node, dest_port, queue-dest_cos, 1) != SOC_E_NONE ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: E2Q write failure\n")));
            status = BCM_E_MEMORY;
            goto err;
        }

        if (soc_qe2000_q2ec_set(unit, queue, mc, dest_node, dest_port, dest_cos) != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Q2EC write failure\n")));
            status = BCM_E_MEMORY;
            goto err;
        }

        /* If sysport allocated, which means it's not in DMode, and it's one of unicast port or
         * extended eset, we need to config port remap in QE2000 eg block to map the sysport
         * lower bit[5:0] to the real destination port specified by the application
         */
        if (sysport != BCM_INT_SBX_INVALID_SYSPORT) {
            
            if ( ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
                  (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) &&
                 ((sysport & 0x3F) < BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP) &&
                 (SOC_SBX_CFG(unit)->num_ds_ids>128) ) {
                /* Extended eset */
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "extended eset Remapping sysport(0x%x) to port(0x%x)\n"),
                          sysport, remap_port));

                /* SDK-38024, update both EF and non-EF fifo mapping regardless of EF priority */
                for (ef=0; ef<=1; ef++) {
                    status = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, remap_port);
                    if (status != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "ERROR: Failed to remap sysport\n")));
                        goto err;
                    }
                }
            }
            /* Unicast and remapping */
            else if (SOC_SBX_CONTROL(unit)->node_id == remap_node){
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "other Remapping sysport(0x%x) to port(0x%x)\n"),
                          sysport, remap_port));
                /* SDK-38024, update both EF and non-EF fifo mapping regardless of EF priority */
                for (ef=0; ef <=1; ef++) {
                    status = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, remap_port);
                    if (status != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "ERROR: Failed to remap sysport\n")));
                        goto err;
                    }
                }
            }
        }
    }

   {
        status = _bcm_qe2000_cosq_select_queue_params(unit, queue, p_qparams, &hold_ts, &qtype, &rate_a, &rate_b, queue_region);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: select queue params failure\n")));
            goto err;
        }

        status = soc_qe2000_queue_para_set(unit, queue, p_qparams->local, hold_ts, qtype);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: queue param table write failure\n")));
            status = -1;
            goto err;
        }

        status = soc_qe2000_rate_a_set(unit, queue, rate_a);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Rate A table write failed\n")));
            status = -1;
            goto err;
        }

        status = soc_qe2000_rate_b_set (unit, queue, rate_b);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: rate B table write failed\n")));
            status = -1;
            goto err;
        }
    }


    _bcm_qe2000_cosq_set_ingress_shaper(unit, queue,
                                        p_qparams->shape_limit_kbps,
                                        p_qparams->enable_shaping);

    {
        _bcm_qe2000_anemic_age_threshold_key_for_params_chosen(unit, queue, p_qparams, &anemic_age_threshold_key);

        status = soc_qe2000_age_thresh_key_set(unit, queue, anemic_age_threshold_key);
        if (status != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: age threshold key set write failed\n")));
            goto err;
        }
    }

    status = soc_qe2000_depth_length_set(unit, queue, 0x0 /* depth */, 0x0 /* pkt length */);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: queue depth/length set failed\n")));
        goto err;
    }

    /* The bw_group contains the bag rate  */
    _bcm_qe2000_rate_delta_max_index_for_port_rate_chosen(unit, p_qparams->bw_mode,
                                                          p_bwparams->bag_rate_kbps,
                                                          &rate_delta_max_index);


    status = _bcm_qe2000_cosq_add_delete_queue(unit,
                                               TRUE /*add*/,
                                               queue,
                                               rate_delta_max_index,
                                               p_qparams);

    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "add/delete queue failed\n")));
        goto err;
    }

    /* bug 24731, clear priority entry on queue add */
    idx = ((dest_cos & 0xF) << 13) | ((mc & 1)<< 12) | ((dest_node & 0x1F) << 6) | (dest_port & 0x3F);

    status = soc_qe2000_priority_set (unit, idx, 0 /* shaped */, 0 /* pri */,
                                      0 /* next_pri */);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "priority table clear failed for queue(%d) idx(%d)\n"),
                   queue, idx));
        goto err;
    }

    /* Update the bag info */
    if (queue_region == bcm_sbx_cosq_queue_region_local) {
	queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
	bw_group = queue_state[queue].bw_group;

	_bcm_qe2000_cosq_update_bag(unit, bw_group, queue_region);
    }

    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if (p_qparams->bw_mode == BCM_COSQ_SP) {
            /* check to make sure that the Global Strict Priority configuration is */
            /* consistent                                                          */

            status = _bcm_qe2000_cosq_update_gsp(unit, queue);
            if (status) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: set ingress params configuration (strict priority) error(%d)\n"),
                           status));
                return(status);
            }
        }
    }


    return 0;

err:
    if ((is_allocated == TRUE) && (dest_cos==0)) {
        _bcm_qe2000_cosq_free_virtual_node_port(unit, (queue & 0xfff0));
    }

    return(status);
}

int
bcm_qe2000_cosq_add_queue(int unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region,
                          int sysport, int eset, int dest_node, int dest_port, int dest_mc, int dest_cos,
                          int32 dest_type, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                          bcm_sbx_cosq_bw_group_params_t *p_bwparams,
                          int inhibit_write)
{
    return(_bcm_qe2000_cosq_add_queue(unit, queue, queue_region, sysport, dest_node,
                                      dest_port, dest_mc, dest_cos,
                                      dest_type, p_qparams, p_bwparams, TRUE));
}

int
_bcm_qe2000_cosq_delete_queue(int unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region, int is_user_invoked) {
    sbFabStatus_t status;

    bcm_sbx_cosq_destport_state_t *p_dps = NULL;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    /* These will be read from Q2EC, then used to address E2Q */
    int32 dest_type; /* unicast/multicast */
    int32 dest_node;
    int32 dest_port;
    int32 dest_cos;
    uint32 data0, data1, data2, data3;

    int32 last_queue = 1; /* ab 022806 this is last queue so clear E2Q  - in future this will come from up above*/
    int32 cos_per_port = 16; /* ab 022806 this should come down too for FIC mode */
    uint32 tmp_cos;
    uint32 tmp_base_queue = 0;
    uint32 enable = 0;
    int tmp_dest_node, tmp_dest_port, tmp_dest_cos;
    int idx = 0;
    int mc = 0;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    int32 bw_group = 0;
    bcm_sbx_cosq_control_t control_info;
    int value = 0, is_deallocated = 0, template = 0, rv = 0;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Deleting queue(%d)\n"),
              queue));

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    bw_group = p_qstate->bw_group;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    /* Read Q2EC memory to get the MC/Node/Port for writing to E2Q memory */
    status = soc_qe2000_q2ec_get(unit, queue, &dest_type, &dest_node, &dest_port, &dest_cos);
    if (status != (sbFabStatus_et) SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Q2EC read failed\n")));
        return BCM_E_MEMORY;
    }
    if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        mc = 1;
    }

    /* Force Q2EC memory to harmless default values, only if we are really deleting the queue */
    /* don't do this for the modify queue case which deletes and re-adds the queue            */
    if (is_user_invoked) {
	status = soc_qe2000_q2ec_set(unit, queue, 0, 63, 0, 0);
	if (status != (sbFabStatus_et) SOC_E_NONE) {
	    return BCM_E_MEMORY;
	}
    }

    /* Bug 23212 Clear Shaping data when queue is deleted              */
    /* Don't clear the queue state params though, as we need this info */
    /* when the queue is re-added.                                     */
    {
        _bcm_qe2000_cosq_set_ingress_shaper(unit, queue,
                                            0,       /* shape_limit_kbps */
                                            FALSE);  /* enable_shaping */

    }


    
    /* many cos's in this logical port in FIC mode */
    if (queue_region == bcm_sbx_cosq_queue_region_local){

        status = soc_qe2000_e2q_get(unit, dest_type, (dest_node<<6), (dest_port&0x3F), (int32*)&tmp_base_queue, (int32*)&enable);
        if (status != (sbFabStatus_et) SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: E2Q get failed\n")));
            return BCM_E_MEMORY;
        }
        /* search 16 queues for the magic node=63 number  - bogus! */
        for (tmp_cos=0;tmp_cos<cos_per_port;tmp_cos++){
            status = soc_qe2000_q2ec_get(unit, tmp_base_queue+tmp_cos, &dest_type, &tmp_dest_node, &tmp_dest_port, &tmp_dest_cos);
            if (SB_FAB_STATUS_OK != status) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Q2EC get failed\n")));
                return BCM_E_MEMORY;
            }
            if (tmp_dest_node != 63){
                last_queue = 0;
                break;
            }
        }
    }

    if ((last_queue) && (is_user_invoked == TRUE) ) {

	if (queue_region == bcm_sbx_cosq_queue_region_local) {
	    if (is_user_invoked == TRUE) {
		_bcm_qe2000_cosq_free_virtual_node_port(unit, queue);
	    }
	}

        /* Force E2Q memory to a disabled queue state */
        status = soc_qe2000_e2q_set(unit, dest_type, dest_node, (dest_port&0x3F), 0x3FF /* queue */, 0 /* disable */);
        if (status != (sbFabStatus_et) SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: E2Q set failed\n")));
            return BCM_E_MEMORY;
        }

	/*
	 * Fifo disable, part 1
	 */

	p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
	p_qstate = &p_qstate[queue];
	p_dps = (bcm_sbx_cosq_destport_state_t*)SOC_SBX_STATE(unit)->destport_state;
        if (dest_port < SBX_MAX_PORTS) {
	p_dps = &p_dps[dest_port];
        } else {
            p_dps = NULL;
        }
    }
    
    
    /*
     * set anemic configuration
     */
    if (is_user_invoked == TRUE) {
        control_info.u.conn.queue = queue;
        control_info.u.conn.queue_region = queue_region;
        rv = bcm_qe2000_cosq_control_get(unit, p_bwstate->gport, -1 /* not used */,
                                         bcmCosqControlFabricConnectMinUtilization,
                                         &value, &control_info);
        if (rv != SOC_E_NONE) {
            return(rv);
        }

        /* free previous resource */ 
        soc_sbx_connect_min_util_dealloc(unit, 0, value, &is_deallocated, &template);

        rv = bcm_qe2000_cosq_control_get(unit, p_bwstate->gport, -1 /* not used */,
                                         bcmCosqControlFabricConnectMaxTime,  
                                         &value, &control_info);
        if (rv != SOC_E_NONE) {
            return(rv);
        }

        /* free previous resource */
        soc_sbx_connect_max_age_dealloc(unit, 0, value, &is_deallocated, &template);
    }
    else { 
        /* anemic configuration is valid and need not be updated */
    }

    status = _bcm_qe2000_cosq_add_delete_queue(unit,
                                               FALSE /*add*/,
                                               queue,
                                               0, /* rate_delta_max_index */
                                               0);

    if (status != (sbFabStatus_et) SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:delete queue failed\n")));
        return BCM_E_MEMORY;
    }

    /* disable aging for this queue, as it is flushed */
    status = soc_qe2000_qm_mem_read(unit, queue, 0x02,
                                    &data0, &data1, &data2, &data3);
    if (status != (sbFabStatus_et) SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: QM memory read failed\n")));
        return BCM_E_MEMORY;
    }

    data0 |= 0x10;    /* Set Queue Empty bit */
    data0 &= ~(0x0F); /* Clear Queue Aging time */
    status = soc_qe2000_qm_mem_write(unit, queue, 0x02,
                                     data0, data1, data2, data3);
    if (status != (sbFabStatus_et) SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: QM memory write failed\n")));
        return BCM_E_MEMORY;
    }

    /* bug 24731, clear priority entry on queue delete */
    idx = ((dest_cos & 0xF) << 13) | ((mc & 1)<< 12) | ((dest_node & 0x1F) << 6) | (dest_port & 0x3F);

    status = soc_qe2000_priority_set (unit, idx, 0 /* shaped */, 0 /* pri */,
                                      0 /* next_pri */);
    if (status != (sbFabStatus_et) SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "priority table clear failed for queue(%d) idx(%d)\n"),
                   queue, idx));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_cosq_delete_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region)
{
    return(_bcm_qe2000_cosq_delete_queue(unit, queue, queue_region, TRUE));
}

int
bcm_qe2000_cosq_enable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region) {

    int status;

    status = _bcm_qe2000_cosq_enable_disable_queue(unit, queue, TRUE /* enable */);

    return status;
}

int
bcm_qe2000_cosq_disable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region) {
    int status;

    status = _bcm_qe2000_cosq_enable_disable_queue(unit, queue, FALSE /* disable */);

    return status;
}

int
bcm_qe2000_cosq_enable_fifo(int unit, 
			    bcm_gport_t gport, 
			    bcm_cos_queue_t cosq,
			    bcm_sbx_cosq_queue_region_type_t queue_region)
{
    bcm_sbx_cosq_queue_state_t *queue_state;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 bw_group = 0;
    int sysport;
    int base_queue;
    int num_cos;
    int local;

    if ((BCM_GPORT_IS_MODPORT(gport) == FALSE) &&
	(BCM_GPORT_IS_CHILD(gport) == FALSE)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Unsuported gport type 0x%x\n"),
	           gport));
	return BCM_E_PARAM;
    }

    /* Convert gport to base queue number */
    if (bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos) != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: gport 0x%x not associated with base queue\n"),
	           gport));
	return BCM_E_PARAM;
    }

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[base_queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    sysport = queue_state[base_queue].sysport;
    local = queue_state[base_queue].ingress.local;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Enable FIFO gport = 0x%x, sysport = 0x%x local = %d\n"),
              gport, sysport, local));

    return bcm_qe2000_port_fifo_enable_set(unit, cosq, sysport, p_bwstate->dest_port, local, TRUE);
}

int
bcm_qe2000_cosq_disable_fifo(int unit, 
			     bcm_gport_t gport, 
			     bcm_cos_queue_t cosq,
			     bcm_sbx_cosq_queue_region_type_t queue_region) 
{
    bcm_sbx_cosq_queue_state_t *queue_state;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 bw_group = 0;
    int sysport;
    int base_queue;
    int num_cos;
    int local;
    
    if ((BCM_GPORT_IS_MODPORT(gport) == FALSE) &&
	(BCM_GPORT_IS_CHILD(gport) == FALSE)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Unsuported gport type 0x%x\n"),
	           gport));
	return BCM_E_PARAM;
    }

    /* Convert gport to base queue number */
    if (bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos) != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: gport 0x%x not associated with base queue\n"),
	           gport));
	return BCM_E_PARAM;
    }

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[base_queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    sysport = queue_state[base_queue].sysport;
    local = queue_state[base_queue].ingress.local;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Disable FIFO gport = 0x%x, sysport = 0x%x local = %d\n"),
              gport, sysport, local));

    return bcm_qe2000_port_fifo_enable_set(unit, cosq, sysport, p_bwstate->dest_port, local, FALSE);
}

int
bcm_qe2000_cosq_enable_get(int unit, 
			   bcm_gport_t gport, 
			   bcm_cos_queue_t cosq,
			   int *enable)
{
    bcm_sbx_cosq_queue_state_t *queue_state;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 bw_group = 0;
    int sysport;
    int base_queue;
    int num_cos;
    int local;

    if ((BCM_GPORT_IS_MODPORT(gport) == FALSE) &&
	(BCM_GPORT_IS_CHILD(gport) == FALSE)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Unsuported gport type 0x%x\n"),
	           gport));
	return BCM_E_PARAM;
    }

    /* Convert gport to base queue number */
    if (bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos) != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: gport 0x%x not associated with base queue\n"),
	           gport));
	return BCM_E_PARAM;
    }

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[base_queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    sysport = queue_state[base_queue].sysport;
    local = queue_state[base_queue].ingress.local;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Enable get gport = 0x%x, sysport = 0x%x, local =  %d\n"),
              gport, sysport, local));

    return bcm_qe2000_port_fifo_enable_get(unit, cosq, sysport, p_bwstate->dest_port, local, enable);
}

int
bcm_qe2000_cosq_overlay_queue(int unit,
                              int queue,
                              bcm_sbx_cosq_queue_region_type_t queue_region,
                              int sysport,
                              int dest_node,
                              int dest_port,
                              int dest_mc,
                              int dest_cos,
                              int dest_type) {

    int32 status = BCM_E_NONE;
    int32 remap_node = dest_node;
    int32 remap_port = dest_port;
    int32 ef;

    /* modify the Q2EC and E2Q table to map the queue to new sysport */

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Overlaying queue(%d)\n"),
              queue));

    if (sysport == BCM_INT_SBX_INVALID_SYSPORT) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Overlay sysport not specified\n")));
        return BCM_E_INTERNAL;
    }

    /*
     * from sysport calculate the dest_node/port/mc which is input for soc_qe2000_e2q_set
     * and soc_qe2000_q2ec_set
     */
    BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_SYSPORT(unit, sysport, &dest_node, &dest_port, &dest_mc);

    if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Overlay not support multicast\n")));
        return BCM_E_PARAM;
    }

    if (!(queue_region == bcm_sbx_cosq_queue_region_local)) {

        if (soc_qe2000_e2q_set(unit, dest_mc, dest_node, dest_port, queue-dest_cos, 1) != SOC_E_NONE ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: E2Q write failure\n")));
            return BCM_E_MEMORY;
        }

        if (soc_qe2000_q2ec_set(unit, queue, dest_mc, dest_node, dest_port, dest_cos) != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Q2EC write failure\n")));
            return BCM_E_MEMORY;
        }

	if (SOC_SBX_CONTROL(unit)->node_id == remap_node) {
	    /* Only need to remap sysport on the destination node */
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Remapping sysport(0x%x) to port(0x%x)\n"),
	              sysport, remap_port));


        /* SDK-38024, update both EF and non-EF fifo mapping regardless of EF priority */
        for (ef=0; ef<=1; ef++) {
            status = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, remap_port);
            if (status != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to remap sysport\n")));
                return status;
            }
        }
	}
    }

    return status;
}

int
bcm_qe2000_cosq_delete_overlay_queue(int unit,
				     int queue,
				     int base_queue,
                                     bcm_sbx_cosq_queue_region_type_t queue_region) {
    int32 status = BCM_E_NONE;
    int sysport, base_sysport;
    int32 dest_mc;
    int32 dest_node;
    int32 dest_port;
    int32 dest_cos;
    bcm_sbx_cosq_queue_state_t *queue_state;
    /* int local, hold_ts, qtype; */

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    sysport = queue_state[queue].sysport;
    base_sysport = queue_state[base_queue].sysport;

    if ( (sysport < 0) || (base_sysport < 0) ) {
	return BCM_E_INTERNAL;
    }

    if (!(queue_region == bcm_sbx_cosq_queue_region_local)) {

	/* map the queue to the sysport in the base queue */

	/*
	 * from sysport calculate the dest_node/port/mc which is input for soc_qe2000_e2q_set
	 * and soc_qe2000_q2ec_set
	 */
	BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_SYSPORT(unit, sysport, &dest_node, &dest_port, &dest_mc);

	/* point e2q to invalid queue */
        if (soc_qe2000_e2q_set(unit, dest_mc, dest_node, dest_port, 0xFFF, 1) != SOC_E_NONE ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: E2Q write failure\n")));
            return BCM_E_MEMORY;
        }

	/* read q2ec to get the dest_cos, we don't want to modify dest_cos field */
	status = soc_qe2000_q2ec_get(unit, queue, &dest_mc, &dest_node, &dest_port, &dest_cos);
	if (SB_FAB_STATUS_OK != status) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Q2EC get failed\n")));
	    return BCM_E_MEMORY;
	}

	/* need to overwrite the mc/node/port part (which is the sysport) */
	BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_SYSPORT(unit, base_sysport, &dest_node, &dest_port, &dest_mc);

        if (soc_qe2000_q2ec_set(unit, queue, dest_mc, dest_node, dest_port, dest_cos) != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Q2EC write failure\n")));
            return BCM_E_MEMORY;
        }

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Removing sysport(0x%x) overlay on queue(0x%x)\n"),
                  sysport, queue));

	/* port remap table is left alone, will be updated if the sysport is used
	 * for something else later on
	 */

	

    }

    return status;
}

int
bcm_qe2000_cosq_set_ingress_params(int unit,
                                   int32 queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
                                   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
                                   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
                                   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
                                   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo) {
    int32 status = SOC_E_NONE;
    int32 enable, old_anemic_watermark_select, old_min_buffers, old_max_buffers;
    sbBool_t add_delete_queue = FALSE;
    int32 dest_mc, mc;
    int32 dest_node;
    int32 dest_port;
    int32 dest_cos;
    uint32 old_rate_a = 0, old_rate_b = 0;
    int32 old_qtype;
    int32 old_hold_ts;
    uint32 rate_a = 0, rate_b = 0;
    int32 qtype;
    int32 hold_ts;
    int32 anemic_age_threshold_key;
    int32 old_anemic_age_threshold_key;
    int32 rate_delta_max_index;
#if 0
    int32 qla_demand_mask;
#endif /* 0 */
    bcm_sbx_cosq_queue_state_t *queue_state;
    int32 bw_group;
    int32 sysport = BCM_INT_SBX_INVALID_SYSPORT;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 ef, old_ef;
    int32 remap_node;
    int32 remap_port;
    int32 anemic_watermark_select;
    sbBool_t configure_rate_delta_max;
    int adjust;
    int update_remap;

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];


    /* Here the params could be modified without side-effect by overwriting tables in the memory.
     */

    /* check if requires to delete/readd queue
     */
    {
        status = soc_qe2000_qstate_mem_read(unit, queue, &enable, &old_anemic_watermark_select,
                                            &old_min_buffers, &old_max_buffers);
        if ( status != SOC_E_NONE ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to read queue state memory for queue(%d)\n"),
                       queue));
            return status;
        }

        if ( enable == 1 ) {
            /* if queue enabled, and path parameter change (bwMode) caused the anemic_watermark_select change
             * then need to delete/readd queue
             */
            _bcm_qe2000_cosq_anemic_watermark_select_for_params_chosen(p_newqparams,
                                                                       &(anemic_watermark_select));

            /* If queue is active, and any parameters which require delete/readd queue need to be modified
             * we just modify all parameters using existing add/delete queue functions
             */
            if ( (p_newqparams->min_physical_queue_depth_bytes != p_oldqparams->min_physical_queue_depth_bytes) ||
                 (p_newqparams->max_physical_queue_depth_bytes != p_oldqparams->max_physical_queue_depth_bytes) ||
                 (anemic_watermark_select != p_oldqparams->anemic_watermark_select) ||
                 (p_newqparams->qla_demand_mask != p_oldqparams->qla_demand_mask) ) {
                add_delete_queue = TRUE;
            }

            if (soc_feature(unit, soc_feature_egr_independent_fc)) {
                /* check if the queue changed from EF to N-EF or vice versa */
                if ( (p_newqparams->bw_mode != p_oldqparams->bw_mode) &&
                         ((p_newqparams->bw_mode == BCM_COSQ_EF) ||
                         (p_oldqparams->bw_mode == BCM_COSQ_EF) ) ) {
                    add_delete_queue = TRUE;
                }
            }

        } else {
            /* Should not reach here, this is only called when queue is enabled */
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: reconfiguration of queue which is not enabled\n")));
            status = BCM_E_RESOURCE;
            return status;
        }
    }

    status = soc_qe2000_q2ec_get(unit, queue, &mc, &dest_node, &dest_port, &dest_cos);
    if (status) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: q2ec table read error\n")));
        return status;
    }

    /* Retrieve sysport information */
    dest_mc = mc;
    sysport = ((dest_mc << 12) + (dest_node << 6) + dest_port); /* sysport and mc/node/port are same thing */
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        /* Sysport not supported in DMODE */
        if (soc_feature(unit, soc_feature_hybrid)) {
            sysport = queue_state[queue].sysport;
        }
        else {
            sysport = -1;
        }
    } else if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
                (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
        if ( (p_bwstate->overlay_in_use == TRUE) &&
             (queue >= p_bwstate->overlay_base_queue) ) {
            /* overlayed queues, assume dest_mc should always be same for the overlay queues
             * this assumes there is no overlay for multicast gports
             */
            dest_node = p_bwstate->overlay_dest_node;
            dest_port = p_bwstate->overlay_dest_port;
            dest_mc = p_bwstate->dest_mc;
        } else {
            dest_node = p_bwstate->dest_node;
            dest_port = p_bwstate->dest_port;
            dest_mc = p_bwstate->dest_mc;
        }
        
        if ( (((sysport & 0x3F) < 14 ) &&
              SOC_SBX_CFG(unit)->use_extended_esets && mc == 0)) {
            /* This is an extended ESET MC queue.  Use the assigned sysport. */
            mc = 1;
        }
        else {
            if ( dest_mc == 1 && mc == 1) {
                /* Regular ESET queue.  No sysport used. */
                mc = 1;
                sysport = BCM_INT_SBX_INVALID_SYSPORT;
            }
            else {
                /* sysport is for unicast ports */
                mc = 0;
            }
        }
        
    } else {
        
    }
    
    /* modify queue through delete/readd queue
     */
    if ( add_delete_queue == TRUE ) {

        /* retreive packet length adjustment */
        status = _bcm_qe2000_cosq_queue_packet_length_adjust_get(unit, queue, &adjust);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: error retreiving queue packet length adjustment\n")));
            return status;
        }

        /* Delete the queue */
        status = _bcm_qe2000_cosq_delete_queue(unit, queue, queue_region, FALSE);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: delete queue failed\n")));
            return status;
        }

        /* Readd the queue */
        status = _bcm_qe2000_cosq_add_queue(unit, queue, queue_region, sysport, dest_node, dest_port, dest_mc, dest_cos, mc,
                                           p_newqparams, p_newbwparams, FALSE);
        if (status) {
            return status;
        }

        /* configure packet length adjustment */
        status = _bcm_qe2000_cosq_queue_packet_length_adjust_set(unit, queue, adjust);
        return status;

    }

    /* Here either queue is disabled (empty) or the params could be modified without
     * side-effect by overwriting tables in the memory
     */
    {
        /* anemic watermark threshold */
        if ( (p_newqparams->anemic_age_threshold_key != BCM_DEFAULT_INT32_VALUE) &&
             (p_newqparams->anemic_age_threshold_key != p_oldqparams->anemic_age_threshold_key) ) {
            /* only change when user specifies the anemic_age_threshold_key */
            status = soc_qe2000_age_thresh_key_set(unit, queue, p_newqparams->anemic_age_threshold_key);
            if (status) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to modify Anemic Age Threshold for queue(%d)\n"),
                           queue));
                return status;
            }
        }

        /* wred
         * NOTE:
         *     Heard that there is no need to keep retring to update for WRED entry config
         *     like what the addqueue function does. The wred enable order fix this issue.
         */
        /* updating Template/Gain */
        if ( (p_newqparams->template != p_oldqparams->template) ||
             (p_newqparams->gain != p_oldqparams->gain) ) {
            status = soc_qe2000_qwred_config_set(unit, queue, p_newqparams->template, p_newqparams->gain);
            if ( status ) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR:Failed to modify WRED template and gain for queue(%d)\n"),
                           queue));
                return status;
            }
        }

        /* ingress shaper */
        if ( (p_newqparams->shape_limit_kbps != p_oldqparams->shape_limit_kbps) ||
             (p_newqparams->enable_shaping != p_oldqparams->enable_shaping) ) {

            status = _bcm_qe2000_cosq_set_ingress_shaper(unit, queue,
                                                         p_newqparams->shape_limit_kbps,
                             p_newqparams->enable_shaping);
            if ( status ) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to modify shaper config for queue(%d)\n"),
                           queue));
                return status;
            }
        }

        configure_rate_delta_max = FALSE;

        /* Determine if there is change in template due to "sched mode" change */
        if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            if ( ((p_newqparams->bw_mode == BCM_COSQ_SP) ||
                           (p_oldqparams->bw_mode == BCM_COSQ_SP)) &&
                                  (p_newqparams->bw_mode != p_oldqparams->bw_mode) ) {
                configure_rate_delta_max = TRUE;
            }
        }

        /* qla demand mask and bag rate changes */
        if ( (p_newqparams->qla_demand_mask != p_oldqparams->qla_demand_mask) ||
             (p_newbwparams->bag_rate_kbps != p_oldbwparams->bag_rate_kbps) ) {
            configure_rate_delta_max = TRUE;
        }

        if (configure_rate_delta_max == TRUE) {

#if 0
            status = soc_qe2000_qdemand_config_read(unit, queue,
                                            &rate_delta_max_index, &qla_demand_mask);
            if ( status ) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to modify qla_demand_mask for queue(%d)\n"),
                           queue));
                return status;
            }
#endif /* 0 */

            _bcm_qe2000_rate_delta_max_index_for_port_rate_chosen(unit, p_newqparams->bw_mode,
                                                               p_newbwparams->bag_rate_kbps,
                                                               &rate_delta_max_index);

            status = soc_qe2000_qdemand_config_set(unit, queue, rate_delta_max_index,
                                               p_newqparams->qla_demand_mask);
            if ( status) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to modify qla_demand_mask for queue(%d)\n"),
                           queue));
                return status;
            }
        }

    /* queue header adjust
     */
        if ( (p_newqparams->queue_header_adjust_sign != p_oldqparams->queue_header_adjust_sign) ||
             (p_newqparams->queue_header_adjust != p_oldqparams->queue_header_adjust) ) {
            status = soc_qe2000_qbyte_adjust_set(unit, queue, p_newqparams->queue_header_adjust_sign,
                                                 p_newqparams->queue_header_adjust);
            if ( status ) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to modify Queue header adjust sign/bytes for queue(%d)\n"),
                           queue));
                return status;
            }
        }
    }

    /* modify the QOS parameters for this queue */
    {
        status = _bcm_qe2000_cosq_select_queue_params(unit, queue, p_oldqparams, &old_hold_ts, &old_qtype,
                                                      &old_rate_a, &old_rate_b, queue_region);
        if (status) {
            return status;
        }

        status = _bcm_qe2000_cosq_select_queue_params(unit, queue, p_newqparams, &hold_ts, &qtype,
                                                      &rate_a, &rate_b, queue_region);
        if (status) {
            return status;
        }

        if ( (old_hold_ts != hold_ts) || (old_qtype != qtype) ) {
            status =  soc_qe2000_queue_para_set(unit, queue, p_oldqparams->local, hold_ts, qtype);
            if (status) {
                return BCM_E_MEMORY;
            }
        }

        if ( (old_rate_a != rate_a) || (old_rate_b != rate_b) ) {
            status = soc_qe2000_rate_a_set(unit, queue, rate_a);
            if (status) {
                return BCM_E_MEMORY;
            }
            status = soc_qe2000_rate_b_set(unit, queue, rate_b);
            if (status) {
                return BCM_E_MEMORY;
            }
        }
    }

    /* pathParams change could change anemic_age_threshold_key of this queue */
    {
        _bcm_qe2000_anemic_age_threshold_key_for_params_chosen(unit, queue, p_oldqparams, &old_anemic_age_threshold_key);

        _bcm_qe2000_anemic_age_threshold_key_for_params_chosen(unit, queue, p_newqparams, &anemic_age_threshold_key);

        if ( old_anemic_age_threshold_key != anemic_age_threshold_key ) {
            status = soc_qe2000_age_thresh_key_set(unit, queue, anemic_age_threshold_key);
            if (status) {
                return BCM_E_MEMORY;
            }
        }
    }

    /* when talking to polaris, dynamic port remap is used, change between EF and non-EF requires
     * the sysport to be mapped to different fifos
     */
    /* If sysport allocated, which means it's not in DMode, and it's one of unicast port or
     * extended eset, we need to config port remap in QE2000 eg block to map the sysport
     * lower bit[5:0] to the real destination port specified by the application
     */
    if (sysport != BCM_INT_SBX_INVALID_SYSPORT) {

        if ( (p_bwstate->overlay_in_use == TRUE) &&
             (queue >= p_bwstate->overlay_base_queue) ) {
            /* overlayed queues, assume dest_mc should always be same for the overlay queues
             * this assumes there is no overlay for multicast gports
             */
            remap_node = p_bwstate->overlay_dest_node;
        } else {
            remap_node = p_bwstate->dest_node;
        }
        
        update_remap = FALSE;

        if ( /* Multicast extended eset sysport case */
            ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
             (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) &&
            ((sysport & 0x3F) < BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP) &&
            (SOC_SBX_CFG(unit)->num_ds_ids>128) ) {

            update_remap = TRUE;

            /* Unicast sysport case */
        } else if (SOC_SBX_CONTROL(unit)->node_id == remap_node) {

            update_remap = TRUE;
        }

        if (update_remap == TRUE) {
            
            ef = (p_newqparams->bw_mode == BCM_COSQ_EF);
            old_ef = (p_oldqparams->bw_mode == BCM_COSQ_EF);
            
            /* map the sysport to the right fifo (EF/non-EF) when the bw_mode changed */
            if (ef != old_ef) {
                sysport = queue_state[queue].sysport;
                if ( (p_bwstate->overlay_in_use == TRUE) &&
                     (queue >= p_bwstate->overlay_base_queue) ) {
                    /* overlayed queues, assume dest_mc should always be same for the overlay queues
                     * this assumes there is no overlay for multicast gports
                     */
                    remap_port = p_bwstate->overlay_dest_port;
                } else {
                    remap_port = p_bwstate->dest_port;
                }
                
                if (soc_feature(unit, soc_feature_egr_independent_fc)) {
                    sysport = (ef) ? BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport) :
                        BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
                }
                
                
                /* SDK-38024, update both EF and non-EF fifo mapping regardless of EF priority */
                for (ef=0; ef<=1; ef++) {
                    status = bcm_qe2000_cosq_sysport_port_remap(unit, sysport, ef, remap_port);
                    if (status != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "ERROR: Failed to remap sysport\n")));
                        return status;
                    }
                }
            }
        }
    }
    
    if (queue_region == bcm_sbx_cosq_queue_region_local) {
	/* Parameters are updated with respect to the ingress parameters in */
	/* the queue state structure and the bw_group structure, not those  */
	/* passed in above                                                  */
	status = _bcm_qe2000_cosq_update_bag(unit, bw_group, queue_region);

	if (status) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: set ingress params configuration bw_group(%d) error(%d)\n"),
	               bw_group, status));
	    return status;
	}
    }

    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if (p_newqparams->bw_mode == BCM_COSQ_SP) {
            /* check to make sure that the Global Strict Priority configuration is */
            /* consistent                                                          */

            status = _bcm_qe2000_cosq_update_gsp(unit, queue);
            if (status) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: set ingress params configuration (strict priority) bw_group(%d) error(%d)\n"),
                           bw_group, status));
                return(status);
            }
        }
    }

    return status;
}

int
bcm_qe2000_cosq_set_ingress_shaper(int unit,
                                   int base_queue,
                                   bcm_cos_queue_t cosq,
                                   int num_cos_levels,
                                   uint32 shape_limit_kbps,
                                   int set_logical_port_shaper,
                                   int enable_shaping) {
    int status = BCM_E_PARAM;
    bcm_sbx_cosq_queue_state_t *p_qstate;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    /* ignore 'set_logical_port_shaper' param and set it based on SOC property */
    set_logical_port_shaper = (SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress>1);

    if ((SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress == 1) && (!set_logical_port_shaper)) {


        status = _bcm_qe2000_cosq_set_ingress_shaper(unit, base_queue + cosq,
                                                     shape_limit_kbps,
                                                     enable_shaping);

        /* Set the shape_limit_kbps and enable in the qstate structure for later updates */
        p_qstate[base_queue + cosq].ingress.enable_shaping = enable_shaping;
        p_qstate[base_queue + cosq].ingress.shape_limit_kbps = shape_limit_kbps;

    } else if  ((SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress != 1) && (set_logical_port_shaper)) {

        status = _bcm_qe2000_cosq_set_queue_group_ingress_shaper(unit,
                                                                 base_queue,
                                                                 num_cos_levels,
                                                                 shape_limit_kbps,
                                                                 enable_shaping);
        /* shape_limit_kbps and enable for qstate is set up in the   */
        /* bcm_qe2000_cosq_set_queue_group_ingress_shaper() function */
        /* in this case                                              */
    }
    return status;
}

int
bcm_qe2000_cosq_set_template_gain(int unit,
				  int queue,
				  int template,
				  int gain)
{

    /*     Hardcoding the bitfield position and length since hal_ka_auto.h doesn't have those
     *     definitions
     */
    uint32 data0 = ( ((template << 4) & 0x000000F0) | (gain & 0x0000000F) );
    uint32 status;
    int rv = BCM_E_NONE;

    status = soc_qe2000_qm_mem_write(unit, queue, 0x0F, data0, 0x00, 0x00, 0x00);
    if (status) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, soc_qe2000_qm_mem_write,  Unit(%d) queue(%d)\n"),
                   FUNCTION_NAME(), unit, queue));
        rv = BCM_E_INTERNAL;
    }
    return rv;
}

int
bcm_qe2000_cosq_get_template_gain(int unit,
				  int queue,
				  int *template,
				  int *gain)
{
    int rv = BCM_E_NONE;
    uint32 data0, data1, data2, data3;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Retrieve WRED template, queue(%d) on QE2000(%d)\n"),
              queue, unit));

    /* this is a 32 bit entry but the field is 16 bits */

    rv = soc_qe2000_qm_mem_read(unit, queue, 0x0F,
				&data0, &data1, &data2, &data3);

    if (rv) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: read of template/gain from WRED table on QE2000 queue(%d) timed out\n"),
	           queue));
	return rv;
    }

    *template = (data0 >> 4) & 0xf;
    *gain = data0 & 0xf;

    return rv;
}

#define BCM_QE2000_COSQ_DISCARD_MAX_COLORS (3)

int
bcm_qe2000_cosq_gport_discard_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_NONE;
    uint8 data[SB_ZF_FAB_WRED_PARAMETERS_SIZE];
    uint32 *p_data, data0, data1, data2, data3;
    sbZfFabWredParameters_t chip_params;
    uint32 status;


    if (discard->drop_probability == 0) { /* disabling drop probability */
        data0 = data1 = data2 = data3 = 0xFFFFFFFF;

        status = soc_qe2000_qm_mem_write(unit, (template * BCM_QE2000_COSQ_DISCARD_MAX_COLORS + color), 0x10,
                                data3, data2, data1, data0);
        if (status) {
           LOG_ERROR(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "ERROR: %s, soc_qe2000_qm_mem_write,  Unit(%d)\n"),
                      FUNCTION_NAME(), unit));
           rv = BCM_E_INTERNAL;
           goto err;
        }

    }

    else { /* enabling drop probability */
        rv = _bcm_sbx_device_wred_calc_config(unit, SOC_SBX_CFG(unit)->discard_probability_mtu,
                                 queue_size, discard, &chip_params);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, _bcm_sbx_device_wred_calc_config,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            goto err;
        }

        sbZfFabWredParameters_Pack(&chip_params, data, SB_ZF_FAB_WRED_PARAMETERS_SIZE);

        p_data = (uint32 *)data;
        data0 = *p_data;
        data1 = *(p_data + 1);

        status = soc_qe2000_qm_mem_write(unit, (template * BCM_QE2000_COSQ_DISCARD_MAX_COLORS + color), 0x10,
                                         data0, data1, 0x00, 0x00);
        if (status) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, soc_qe2000_qm_mem_write,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }
    }

    return(rv);

err:
    return(rv);
}

int
bcm_qe2000_cosq_gport_discard_get(int unit,
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



int
bcm_qe2000_cosq_gport_stat_enable_set(int unit,
                                      bcm_gport_t   gport,
                                      int   enable)
{
    int rv = BCM_E_UNAVAIL;
    int current_enable;
    int current_base_queue;
    int base_queue;
    int num_cos;
    int max_queues;


    /* Convert gport to base queue number */
    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
    if (rv != BCM_E_NONE) return rv;

    max_queues = SOC_SBX_CFG(unit)->num_queues;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "gport %d base q %d num_cos %d max_queues %d\n"),
                 gport, base_queue, num_cos, max_queues));

    if ( max_queues < base_queue + num_cos) return BCM_E_PARAM;

    if(enable == TRUE) {

        if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "soc_feature_cosq_gport_stat_ability Unavailable")));
            return BCM_E_UNAVAIL;
        }

        soc_qe2000_counter_enable_get(unit, &current_base_queue, &current_enable);

        if (current_enable == TRUE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Counter currently active\n")));
            return BCM_E_INTERNAL;
        }

        rv = soc_qe2000_qm_counter_base_set(unit, base_queue, enable);

        if (rv == SOC_E_NONE ) rv = soc_qe2000_counter_enable_set(unit, base_queue);

        return rv;

    } else if (enable == FALSE) {

        rv = soc_qe2000_counter_enable_clear(unit);

    } else {
        return BCM_E_PARAM;
    }
    return rv;
}


int
bcm_qe2000_cosq_gport_stat_enable_get(int unit,
        int   gport,
        int   *enable)
{
    int rv = BCM_E_UNAVAIL;
    int actual_base_queue;  /* Set to qe2000  */
    int soft_base_queue;
    int base_queue;
     int num_cos;



    if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "soc_feature_cosq_gport_stat_ability Unavailable")));
             return BCM_E_UNAVAIL;
    }

    /* Convert gport to base queue number */
    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
    if (rv != BCM_E_NONE) return rv;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "gport %d base q %d num_cos %d\n"),
                 gport, base_queue, num_cos));


    rv = soc_qe2000_qm_counter_base_get(unit, &actual_base_queue);

    if (rv != BCM_E_NONE) {
        return rv;
    }

    rv = soc_qe2000_counter_enable_get(unit, &soft_base_queue, enable);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "Enable %d,base_queue %d, actual_base_queue %d\n"),
                 *enable,base_queue,actual_base_queue));


    return rv;
}


/* This function is intended for clear of statistics */
int
bcm_qe2000_cosq_gport_stat_set(int unit,
        int         base_queue,
        bcm_cos_queue_t   cosq,
        bcm_cosq_gport_stats_t stat,
        uint64  value)
{
    int rv = BCM_E_UNAVAIL;
    int block;
    int set;
    uint64 uuSet = COMPILER_64_INIT(0,0);

    if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "soc_feature_cosq_gport_stat_ability Unavailable")));
        return BCM_E_UNAVAIL;
    }

    block = 0; /* Since there is only one block on QE2000 force to zero */
    set = cosq;  /* cosq select the counter set */

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "soc_sbx_counter_port_set %d  %d %d cntr %d 0x%x%08x\n"),
                 unit,block,set,stat,COMPILER_64_HI(value), COMPILER_64_LO(value)));
    rv = soc_sbx_counter_set(unit,  block, set, stat, uuSet);


    return rv;
}

int
bcm_qe2000_cosq_gport_stat_get(int unit,
                               int queue_base,
                               bcm_cos_queue_t   cosq,
                               bcm_cosq_gport_stats_t stat,
                               uint64   *value)
{
    int rv = BCM_E_UNAVAIL;
    int block;
    int set;

    if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "soc_feature_cosq_gport_stat_ability Unavailable")));
        return BCM_E_UNAVAIL;
    }

    block = 0;    /* Force block to zero on QE2000. Only one block */
    set = cosq;  /* cosq select the counter set */

    rv = soc_sbx_qe2000_counter_port_get(unit, queue_base, block, set, stat, value);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "soc_sbx_qe2000_counter_port_get %d %d %d cntr %d 0x%x%08x\n"),
                 unit,block,set,stat,COMPILER_64_HI(*value), COMPILER_64_LO(*value)));

    return rv;
}


/********************/
/* STATIC functions */
/********************/
static int
_bcm_qe2000_cosq_update_bag(int unit,
                int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region)

{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_fabric_state_t *p_fabricstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int gamma = 0, sigma = 0;
    uint32 guarantee_in_kbps = 0;
    uint64 uu_epoch_length_in_ns = COMPILER_64_INIT(0,0);
    uint64 uu_guarantee_in_kbytes;
    uint32 guarantee_in_bytes_per_epoch = 0;
    uint32 bag_rate_bytes_per_epoch;
    uint64 uu_bag_rate_kbytes;
    int32 queue;
    int32 num_sp_queues;
    int32 num_queues_in_bag;
    int32 base_queue = 0;
    uint32 data[4] = {0,0,0,0};
    sbZfFabQe2000BwPortConfigEntry_t zfPortConfigEntry;
    int num_queues, start_queue;

    /* No bag updates required if not in standalone Mode */
    if (!(queue_region == bcm_sbx_cosq_queue_region_local)) {
        return BCM_E_NONE;
    }

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_fabricstate = (bcm_sbx_fabric_state_t*)SOC_SBX_STATE(unit)->fabric_state;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    rv = bcm_sbx_cosq_bw_group_verify_queues(unit, bw_group);

    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: verifying queue setup for bw_group(%d) error(%d)\n"),
                   bw_group, rv));
    }

    rv = bcm_sbx_cosq_update_given_weights(unit, bw_group);
    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: update WFQ weights failed\n")));
        return rv;
    }
    base_queue = p_bwstate->base_queue;

    if ( soc_feature(unit, soc_feature_standalone) ) {
        COMPILER_64_SET(uu_epoch_length_in_ns,0,SB_FAB_TME_EPOCH_IN_NS);
    } else {
        COMPILER_64_SET(uu_epoch_length_in_ns,0,SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
        COMPILER_64_UMUL_32(uu_epoch_length_in_ns, p_fabricstate->timeslot_size); /* ns */
    }

    /* write queue indexed BWP table */
    for (queue=base_queue; queue < base_queue + p_bwstate->num_cos; queue++) {
        gamma = 0;
        sigma = 0;

        if (p_qstate[queue].ingress.bw_mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) {
            gamma = p_qstate[queue].ingress.given_weight;
        }
        if (p_qstate[queue].ingress.bw_mode== BCM_COSQ_AF) {

            guarantee_in_kbps = bcm_sbx_cosq_get_bw_guarantee(unit, queue);

            uu_guarantee_in_kbytes =  uu_epoch_length_in_ns;
            COMPILER_64_UMUL_32(uu_guarantee_in_kbytes, (guarantee_in_kbps/8));
	    
	    if (soc_sbx_div64(uu_guarantee_in_kbytes , 1000000, &guarantee_in_bytes_per_epoch) == -1) {
	      LOG_ERROR(BSL_LS_BCM_COMMON,
	                (BSL_META_U(unit,
	                            "ERROR: update Guarantee failed\n")));
	      return BCM_E_INTERNAL;
	    }
            guarantee_in_bytes_per_epoch >>= SOC_SBX_CFG(unit)->demand_scale;

            sigma = (int32) guarantee_in_bytes_per_epoch;

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "queue(%d) guarantee in bytes/epoch(%d)\n"),
                      queue, guarantee_in_bytes_per_epoch));
        }

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "queue(%d) guarantee in kbps(%d) sigma(%d) gamma(%d)\n"),
                  queue, guarantee_in_kbps, sigma, gamma));

        /* Write entry */
        rv = soc_qe2000_qm_mem_write(unit,
                                     queue, /* addr */
                                     0xD, /* BAA config */
                                     gamma,
                                     data[0],  /* dummy 0 field */
                                     data[0],  /* dummy 0 field */
                                     data[0]); /* dummy 0 field */

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gamma set to %d\n"),
                  gamma));

        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Writing BAA configuration entry (%d) failed.\n"),
                       queue));
            return BCM_E_FAIL;
        }
    }

    num_sp_queues = bcm_sbx_cosq_get_num_sp_queues(unit, bw_group);
    num_queues_in_bag = bcm_sbx_cosq_get_num_queues_in_bag(unit, bw_group, queue_region,  &num_queues, &start_queue);
    uu_bag_rate_kbytes =  uu_epoch_length_in_ns;
    COMPILER_64_UMUL_32(uu_bag_rate_kbytes, (p_bwstate->path.bag_rate_kbps / 8));
    
    if (soc_sbx_div64(uu_bag_rate_kbytes, 1000000, &bag_rate_bytes_per_epoch) == -1) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "ERROR: update BAG rate per epoch failed\n")));
      return BCM_E_INTERNAL;
    }
    bag_rate_bytes_per_epoch >>= SOC_SBX_CFG(unit)->demand_scale;
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Bag rate: %d\n"),
              bag_rate_bytes_per_epoch));


    base_queue = p_bwstate->base_queue;

    /* If there is nothing in the bag, clear the PRT entry information */
    if (num_queues_in_bag == 0) {
        base_queue = 0;
        bag_rate_bytes_per_epoch = 0;
        num_sp_queues = 0;
    }

    /* Read entry */
    rv = soc_qe2000_qm_mem_read(unit, bw_group, /* addr */
                                0xC, /* BW Port Config table */
                                &data[0],
                                &data[1],
                                &data[2],
                                &data[3]);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Reading Bw Port Cfg: 0x%08x 0x%08x 0x%08x 0x%08x\n"),
              data[0],data[1],data[2],data[3]));

    if (rv != SOC_E_NONE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Reading Bw port configuration entry bw_group(%d) failed.\n"),
                  bw_group));
        return BCM_E_FAIL;
    }

    sbZfFabQe2000BwPortConfigEntry_Unpack(&zfPortConfigEntry, (uint8*)data, 6);

    /* Update the number of queues and the number of strict priority queues
     */
    zfPortConfigEntry.m_nSpQueues = num_sp_queues;
    zfPortConfigEntry.m_nQueues = num_queues_in_bag;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Updating Port Configuration Table entry (%d) num flows in the bag (%d)\n"),
              bw_group, zfPortConfigEntry.m_nQueues));


    zfPortConfigEntry.m_nBaseQueue = base_queue;
    zfPortConfigEntry.m_nLineRate = bag_rate_bytes_per_epoch;


    sbZfFabQe2000BwPortConfigEntry_Pack(&zfPortConfigEntry, (uint8*)data, 6);

    /* Write entry */
    rv = soc_qe2000_qm_mem_write(unit,
                                 bw_group /* addr */,
                                 0xC /* BW Port Config table */,
                                 data[0],
                                 data[1],
                                 data[2],
                                 data[3]);

    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Writing Bw port configuration table entry (%d) failed.\n"),
                   bw_group));
        return BCM_E_FAIL;
    }


    return BCM_E_NONE;
}

static int
_bcm_qe2000_cosq_arbport_state_hybrid_init(int unit)
{
    int rc = BCM_E_NONE;
    int node, port, fic_flow_start_port;
    int ef = 0;
    int arb_port;


    /* account for the CPU port */
    fic_flow_start_port = SOC_SBX_CFG(unit)->max_ports -
                                         (SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule + 1);

    /* The number of QE2000 arbitration port resource is optimized  by       */
    /* using Node/port values that do not overlap with local arbitration     */
    /* ports. In this mode need not look at all the node mask configuration  */
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        /* mark the overlap of FIC Node/Port space (E2Q/Q2EC tables on Ingress) */
        for (node = 0; node < SOC_SBX_CFG(unit)->num_nodes; node++) {
            if (SOC_SBX_CFG(unit)->cfg_node_00_31_mask & (1 < node)) {

                /* node/port not used for FIC flows */
                for (port = 0; port < fic_flow_start_port; port++) {
                    if (port >= SB_FAB_DEVICE_QE2000_ARB_PORT_MAX_PORT) {
                        continue;
                    }
                    arb_port = ( ((node << SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_SHIFT) &
                                             SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_MASK) |
                                 ((port << SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_SHIFT) &
                                             SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_MASK) );
                    virtual_node_port[unit][arb_port].state = BCM_INT_SBX_ARBPORT_STATE_LOCAL;
                }

                /* node/port used for FIC flows */
                for (port = fic_flow_start_port; port < SOC_SBX_CFG(unit)->max_ports; port++) {
                    if (port >= SB_FAB_DEVICE_QE2000_ARB_PORT_MAX_PORT) {
                        continue;
                    }
                    arb_port = ( ((node << SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_SHIFT) &
                                             SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_MASK) |
                                 ((port << SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_SHIFT) &
                                             SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_MASK) );
                    virtual_node_port[unit][arb_port].state = BCM_INT_SBX_ARBPORT_STATE_GLOBAL;
                }
            }
            else {
                /* node/port not used for FIC flows */
                for (port = 0; port < SOC_SBX_CFG(unit)->max_ports; port++) {
                    if (port >= SB_FAB_DEVICE_QE2000_ARB_PORT_MAX_PORT) {
                        continue;
                    }
                    arb_port = ( ((node << SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_SHIFT) &
                                             SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_MASK) |
                                 ((port << SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_SHIFT) &
                                             SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_MASK) );
                    virtual_node_port[unit][arb_port].state = BCM_INT_SBX_ARBPORT_STATE_LOCAL;
                }
            }
        }

        /* mark the overlap of FIC port remap space (Egress). */
        for (port = fic_flow_start_port; port < SOC_SBX_CFG(unit)->max_ports; port++) {
            for (ef = 0; ef <= 1; ef++) {
                arb_port = ( ((port << SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_PORT_SHIFT) &
                                    SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_PORT_MASK) |
                             ((ef << SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_EF_SHIFT) &
                                    SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_EF_MASK) );
                virtual_node_port[unit][arb_port].state = BCM_INT_SBX_ARBPORT_STATE_GLOBAL;
            }
        }

    }
    else {
        rc = BCM_E_UNAVAIL;
    }

    return(rc);
}

static int
_bcm_qe2000_cosq_arbport_state_bm9600_hybrid_init(int unit)
{
    int rc = BCM_E_NONE;
    int lna;


    _bcm_sbx_device_cosq_init(unit);

    for (lna = 0; lna < BCM_SBX_DEVICE_COSQ_MAX_LNAS; lna++) {
        if (_bcm_sbx_device_cosq_is_lna_available(unit, lna) == FALSE) {
            virtual_node_port[unit][lna].state = BCM_INT_SBX_ARBPORT_STATE_GLOBAL;
        }
    }

    _bcm_sbx_device_cosq_deinit(unit);

    return(rc);
}

/* Initialize virtual node port array for later allocation */
static int
_bcm_qe2000_cosq_init_virtual_node_port(int unit)
{
    int32 vnp_id;
    int rc = BCM_E_NONE;

#ifdef TME_ARB_PORT_OPTIMIZE
    int32 l1_arb_id;
    int32 l2_arb_id;
    arb_group_t *p_l2_arb_group;
#endif

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "initialize virtual node port list\n")));

    /* update data structures assuming TME mode */
    for (vnp_id=0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {
	virtual_node_port[unit][vnp_id].arb_group = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX;
        virtual_node_port[unit][vnp_id].free = TRUE;
        virtual_node_port[unit][vnp_id].base_queue = -1;

        virtual_node_port[unit][vnp_id].ref_count = 0;
        virtual_node_port[unit][vnp_id].state = BCM_INT_SBX_ARBPORT_STATE_LOCAL;
	if (vnp_id  == 0) {
	    /* hw erratum 21802: don't use vnp zero */
	    virtual_node_port[unit][vnp_id].free = FALSE;
	    virtual_node_port[unit][vnp_id].ref_count = 1;
	}

	/* SDK-29099 leakage excessive on mod32 local arb ports, e.g. 32, 64, 96... reserve these and don't use */
	if (vnp_id%32 == 0) {
	    virtual_node_port[unit][vnp_id].state = BCM_INT_SBX_ARBPORT_STATE_RESERVED;
	}
    }

#ifdef TME_ARB_PORT_OPTIMIZE

    level_two_arb_group[unit] = sal_alloc(sizeof(arb_group_t) * BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX,
					  "qe2000 arb group memory");

    if (level_two_arb_group[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }

    for (l2_arb_id=0; l2_arb_id<BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX; l2_arb_id++) {

	p_l2_arb_group = &level_two_arb_group[unit][l2_arb_id];

	sal_memset(p_l2_arb_group, 0, sizeof(arb_group_t));

	for (l1_arb_id=0; l1_arb_id<BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX; l1_arb_id++) {
	    p_l2_arb_group->dest_node[l1_arb_id] = -1;
	    p_l2_arb_group->dest_port[l1_arb_id] = -1;
	}
    }
#endif

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if ( soc_feature(unit, soc_feature_hybrid) ) {
            rc = _bcm_qe2000_cosq_arbport_state_hybrid_init(unit);
        }
    }

    
    else if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
	      (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
        if ( soc_feature(unit, soc_feature_hybrid) ) {
            rc = _bcm_qe2000_cosq_arbport_state_bm9600_hybrid_init(unit);
        }
    }
    return BCM_E_NONE;

#ifdef TME_ARB_PORT_OPTIMIZE
err:
    if (level_two_arb_group[unit] != NULL) {
        sal_free(level_two_arb_group[unit]);
        level_two_arb_group[unit] = NULL;
    }
    return rc;
#endif
}

int
bcm_qe2000_cosq_detach(int unit)
{
    int rv = BCM_E_NONE;

    if (level_two_arb_group[unit] != NULL) {
        sal_free(level_two_arb_group[unit]);
        level_two_arb_group[unit] = NULL;
    }

   /* Detach scoreboard thread if already running */
    rv = soc_sbx_scoreboard_detach(unit);
   if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Unit(%d) error detaching scoreboard thread"),
                   unit));
	return rv;
    } 

    return BCM_E_NONE;
}

#ifdef TME_ARB_PORT_OPTIMIZE
static int32
_bcm_qe2000_cosq_l2_arb_group_has_destination(int unit, int32 l2_arb_id, int32 dest_node, int32 dest_port)
{
    arb_group_t *p_l2_arb_group;
    int32 l1_arb_id;

    p_l2_arb_group = &level_two_arb_group[unit][l2_arb_id];

    for (l1_arb_id=0; l1_arb_id<BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX; l1_arb_id++) {

	if ((p_l2_arb_group->dest_node[l1_arb_id] == dest_node) &&
	    (p_l2_arb_group->dest_port[l1_arb_id] == dest_port)) {

	    return TRUE;
	}    
    }
    return FALSE;
}
static int32
_bcm_qe2000_cosq_l2_arb_group_empty(int unit, int32 l2_arb_id)
{
    int32 vnp_id;
    int32 l2_arb_free=TRUE;

    for (vnp_id = l2_arb_id*BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	 vnp_id < l2_arb_id*BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX + BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	 vnp_id++) {
	if (virtual_node_port[unit][vnp_id].free == FALSE) {
	    l2_arb_free = FALSE;
	    break;
	}
    }
    return l2_arb_free;
}

static void
_bcm_qe2000_reserve_arb_group(int unit, int32 vnp_id, int32 dest_node, int32 dest_port)
{
    arb_group_t *p_l2_arb_group;
    int32 l2_arb_id;
    int32 l1_arb_id;

    l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
    p_l2_arb_group = &level_two_arb_group[unit][l2_arb_id];
    
    l1_arb_id = vnp_id%BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
    
    p_l2_arb_group->dest_node[l1_arb_id] = dest_node;
    p_l2_arb_group->dest_port[l1_arb_id] = dest_port;
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE
    LOG_CLI((BSL_META_U(unit,
                        "l1_arb_id(%d) l2_arb_id(%d)\n"), l1_arb_id, l2_arb_id));
#endif
}

/* 1 of the 32 arb node ports (vnps 992-1023 is reserved for multicast) */
static int32
_bcm_qe2000_allocate_virtual_node_port_multicast(int unit, int32 base_queue)
{
    int32 vnp_id;
    int32 arb_port_base_queue;

    arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;
    
    for (vnp_id = MIN_VIRTUAL_NODE_PORT_MULTICAST;
	 vnp_id < MAX_VIRTUAL_NODE_PORT_MULTICAST;
	 vnp_id++) {
	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}
	
	if (virtual_node_port[unit][vnp_id].base_queue != arb_port_base_queue){
	    continue;
	}
	
	virtual_node_port[unit][vnp_id].ref_count++;
	
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "allocate multicast virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
	          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
	
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	
	LOG_CLI((BSL_META_U(unit,
                            "0:  allocate multicast virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
                 vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
#endif	
	_bcm_qe2000_reserve_arb_group(unit, vnp_id, -1, -1);
	
	return(vnp_id);
    }
    
    /* attempt allocating new virtual node/port */
    for (vnp_id = MIN_VIRTUAL_NODE_PORT_MULTICAST;
	 vnp_id < MAX_VIRTUAL_NODE_PORT_MULTICAST;
	 vnp_id++) {

	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}

	if (virtual_node_port[unit][vnp_id].free == TRUE) {
	    virtual_node_port[unit][vnp_id].free = FALSE;
	    virtual_node_port[unit][vnp_id].base_queue = arb_port_base_queue;
	    virtual_node_port[unit][vnp_id].ref_count = 1;
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "allocate multicast virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
	              vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
	    
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	    
	    LOG_CLI((BSL_META_U(unit,
                                "1:  allocate multicast virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
                     vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
#endif		
	    _bcm_qe2000_reserve_arb_group(unit, vnp_id, -1, -1);

	    return(vnp_id);
	}
    } 
    return -1;
}
/* Allocate VPs striped across 32 arb node groups for each destination port, after all 32 full, fill remaining slots */
/* SDK-30464 fill remaining slots striped also.                                                                      */
/* 1 of the 32 arb node ports (vnps 992-1023 is reserved for multicast) */
static int32
_bcm_qe2000_allocate_virtual_node_port_unicast(int unit, int32 base_queue, int32 dest_node, int32 dest_port)
{
    int32 vnp_id;
    int32 arb_port_base_queue;
    int32 l2_arb_id;
    int32 l1_arb_id;

    arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;

    /* determine if virtual node/port is already allocated and try to allocate to arb port not with this dest */
    for (vnp_id = 0; vnp_id < MAX_VIRTUAL_NODE_PORT_UNICAST; vnp_id++) {
	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}
	
	if (virtual_node_port[unit][vnp_id].base_queue != arb_port_base_queue){
	    continue;
	}
	
	/* skip this arb port if there is any other available which doesn't have this destination ...*/
	l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	if (_bcm_qe2000_cosq_l2_arb_group_has_destination(unit, l2_arb_id, dest_node, dest_port) == TRUE) {
	    continue;
	}
	
	virtual_node_port[unit][vnp_id].ref_count++;
	
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
	          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
	
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	
	LOG_CLI((BSL_META_U(unit,
                            "0:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                 vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif	
	_bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
	
	return(vnp_id);
    }
    
    /* attempt allocating new virtual node/port which doesn't have match in group */
    for (vnp_id=0; vnp_id < MAX_VIRTUAL_NODE_PORT_UNICAST; vnp_id++) {
	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}
	
	/* skip this arb port if there is any other available in group which doesn't have this destination ...*/
	l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	if (_bcm_qe2000_cosq_l2_arb_group_has_destination(unit, l2_arb_id, dest_node, dest_port) == TRUE) {
	    continue;
	}
	
	if (virtual_node_port[unit][vnp_id].free == TRUE) {
	    virtual_node_port[unit][vnp_id].free = FALSE;
	    virtual_node_port[unit][vnp_id].base_queue = arb_port_base_queue;
	    virtual_node_port[unit][vnp_id].ref_count = 1;
		    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "allocate virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
	              vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
	    
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	    
	    LOG_CLI((BSL_META_U(unit,
                                "1:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                     vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif	    
	    _bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
	    
	    return(vnp_id);
	}
    } 
    
    /* determine if virtual node/port is already allocated and allocate any remaining - no choice */
    for (vnp_id = 0; vnp_id < MAX_VIRTUAL_NODE_PORT_UNICAST; vnp_id++) {
	if (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) {
	    continue;
	}
	
	if (virtual_node_port[unit][vnp_id].base_queue != arb_port_base_queue){
	    continue;
	}
	
	virtual_node_port[unit][vnp_id].ref_count++;
	
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "re-use virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
	          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	
	LOG_CLI((BSL_META_U(unit,
                            "2:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                 vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif		
	_bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
	
	return(vnp_id);
    }
    
    /* attempt allocating new virtual node/port regardless of match  */
    /* SDK-30464 stripe these                                        */
    for (l1_arb_id=0; l1_arb_id<BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX; l1_arb_id++) {
	for (l2_arb_id=0; l2_arb_id<BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX; l2_arb_id++) {
	    vnp_id = (l2_arb_id << 5) + l1_arb_id;

	    if (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) {
		continue;
	    }
	
	    if (virtual_node_port[unit][vnp_id].free == TRUE) {
		virtual_node_port[unit][vnp_id].free = FALSE;
		virtual_node_port[unit][vnp_id].base_queue = arb_port_base_queue;
		virtual_node_port[unit][vnp_id].ref_count = 1;
		
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "allocate virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
		          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
	    
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	    
		LOG_CLI((BSL_META_U(unit,
                                    "2:  WARNING: allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                         vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif		    
		    
		_bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
		return(vnp_id);
	    }
	}
    }
    return -1;
}
/* Allocate VPs for each destination node/port to one of 32 arb node groups, after full for a given destination, return error */
/* 1 of the 32 arb node ports (vnps 992-1023 is reserved for multicast) */
static int32
_bcm_qe2000_allocate_virtual_node_port_unicast_fair_excess(int unit, int32 base_queue, int32 dest_node, int32 dest_port)
{
    int32 vnp_id;
    int32 arb_port_base_queue;
    int32 l2_arb_id;
    
    arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;

#if 000    /* Remove this ifdef if support for multiple queue groups to the same destination within an arb port is allowed */

    /* determine if virtual node/port is already allocated and try to allocate to arb port with this destination */
    for (vnp_id = 0; vnp_id < MAX_VIRTUAL_NODE_PORT_UNICAST; vnp_id++) {

	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}
	
	if (virtual_node_port[unit][vnp_id].base_queue != arb_port_base_queue){
	    continue;
	}
	
	/* choose this arb port if it's already being used for this port */
	l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	if (_bcm_qe2000_cosq_l2_arb_group_has_destination(unit, l2_arb_id, dest_node, dest_port) == TRUE) {
	    
	    virtual_node_port[unit][vnp_id].ref_count++;
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
	              vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
	    
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE	    
	    LOG_CLI((BSL_META_U(unit,
                                "0:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                     vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif	    
	    _bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
	    
	    return(vnp_id);
	}
    }
#endif
    /* attempt allocating new virtual node/port which has match in group */
    for (vnp_id=0; vnp_id < MAX_VIRTUAL_NODE_PORT_UNICAST; vnp_id++) {

	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}
	    
	l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	if (_bcm_qe2000_cosq_l2_arb_group_has_destination(unit, l2_arb_id, dest_node, dest_port) == TRUE) {
	    
	    if (virtual_node_port[unit][vnp_id].free == TRUE) {
		virtual_node_port[unit][vnp_id].free = FALSE;
		virtual_node_port[unit][vnp_id].base_queue = arb_port_base_queue;
		virtual_node_port[unit][vnp_id].ref_count = 1;
		
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "allocate virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
		          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
		
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE		
		LOG_CLI((BSL_META_U(unit,
                                    "1:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                         vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif
		
		_bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
		
		return(vnp_id);
	    }
	} 
    }
    /* attempt allocating new virtual node/port as long as it is empty */
    for (vnp_id=0; vnp_id < MAX_VIRTUAL_NODE_PORT_UNICAST; vnp_id++) {
	if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
	    (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	    continue;
	}
	l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	
	if (_bcm_qe2000_cosq_l2_arb_group_empty(unit, l2_arb_id)) {
	    
	    if (virtual_node_port[unit][vnp_id].free == TRUE) {
		virtual_node_port[unit][vnp_id].free = FALSE;
		virtual_node_port[unit][vnp_id].base_queue = arb_port_base_queue;
		virtual_node_port[unit][vnp_id].ref_count = 1;
		
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "allocate virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
		          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
		
#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE		
		LOG_CLI((BSL_META_U(unit,
                                    "2:  allocate empty arbiter node virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                         vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif		
		
		_bcm_qe2000_reserve_arb_group(unit, vnp_id, dest_node, dest_port);
		return(vnp_id);
	    }
	}
    }

    return -1;
}

#endif

/* allocate a virtual node/port for a given queue group */
static int32
_bcm_qe2000_cosq_allocate_virtual_node_port(int unit, int32 base_queue, int32 dest_node, int32 dest_port, int32 multicast)
{
#if !defined(TME_ARB_PORT_OPTIMIZE) || defined(BCM_EASY_RELOAD_SUPPORT)
    int32 vnp_id;
#endif
#ifndef TME_ARB_PORT_OPTIMIZE
    int32 arb_port_base_queue;
#endif
#ifdef BCM_EASY_RELOAD_SUPPORT
    int dest_type;
    int dest_node1;
    int dest_port1;
    int dest_cos;
    int status;
#endif

    /* If we are reloading, read the virtual node port from the hardware and allocate that one */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
	status = soc_qe2000_q2ec_get(unit, base_queue, &dest_type, &dest_node1, &dest_port1, &dest_cos);
	if (status != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: EASY_RELOAD: Q2EC read failed for queue(%d)\n"),
	               base_queue));
	    return  BCM_E_TIMEOUT;
	}

	/* For unused entries, more than 5 bits will be set in the node field */
	if ((dest_node1 > 0x1f) || (dest_port1 > 0x1f)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: EASY_RELOAD: Q2EC read of virtual node port node(%d) port(%d) invalid\n"),
	               dest_node, dest_port));
	    return BCM_E_INTERNAL;
	}

	vnp_id = (dest_node1 << 5 & 0x1F) | (dest_port1 & 0x1F);

	if (virtual_node_port[unit][vnp_id].free == TRUE) {

            virtual_node_port[unit][vnp_id].free = FALSE;
            virtual_node_port[unit][vnp_id].base_queue = base_queue;

            return(vnp_id);
        }
	else {
	    virtual_node_port[unit][vnp_id].ref_count++;
	}
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "RELOADING: virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
	          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
#endif
    } else {
#ifdef TME_ARB_PORT_OPTIMIZE
	
	if (multicast) {
	    return (_bcm_qe2000_allocate_virtual_node_port_multicast(unit, base_queue));

	} else if (SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness) {
	    return (_bcm_qe2000_allocate_virtual_node_port_unicast_fair_excess(unit, base_queue, dest_node, dest_port));
	}  else {
	    return (_bcm_qe2000_allocate_virtual_node_port_unicast(unit, base_queue, dest_node, dest_port));
	}

#else /* !TME_ARB_PORT_OPTIMIZE */

        arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;

        /* determine if virtual node/port is already allocated */
	for (vnp_id = 0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {
	    if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
		(virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
	      continue;
	    }
	    if (virtual_node_port[unit][vnp_id].free == TRUE) {
                continue;
            }
            if (virtual_node_port[unit][vnp_id].base_queue != arb_port_base_queue){
                continue;
            }

            virtual_node_port[unit][vnp_id].ref_count++;

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "allocate virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
                      vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));

#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE
	    LOG_CLI((BSL_META_U(unit,
                                "old 0:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                     vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif

            return(vnp_id);
        }

        /* attempt allocating virtual node/port */
        for (vnp_id=0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {
	    if ((virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) ||
		(virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_RESERVED)) {
		continue;
	    }
	    if (virtual_node_port[unit][vnp_id].free == TRUE) {
		virtual_node_port[unit][vnp_id].free = FALSE;
		virtual_node_port[unit][vnp_id].base_queue = arb_port_base_queue;
		virtual_node_port[unit][vnp_id].ref_count = 1;

		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "allocate virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
		          vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));

#ifdef DEBUG_TME_ARB_PORT_OPTIMIZE
		LOG_CLI((BSL_META_U(unit,
                                    "old 1:  allocate virtual node/port(%d) ref_count(%d) base_queue(%d) dest node/port(%d/%d)\n"),
                         vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue, dest_node, dest_port));
#endif
		return(vnp_id);
	    }
	}
#endif /* !TME_ARB_PORT_OPTIMIZE */
    }
    return(-1);
}


/* get the virtual node/port for a given queue group */
static int32
_bcm_qe2000_cosq_get_virtual_node_port(int unit, int32 base_queue)
{
    int32 vnp_id;
    int32 arb_port_base_queue;


    arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;

    for (vnp_id=0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {
        if (virtual_node_port[unit][vnp_id].base_queue == arb_port_base_queue) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "found virtual node port(%d) base_queue(%d)\n"),
                      vnp_id, base_queue));
            return(vnp_id);
        }
    }
    return(-1);
}

/* get the reference count of virtual node/port for a given queue group */
static int32
_bcm_qe2000_cosq_get_virtual_node_port_ref_count(int unit, int32 base_queue)
{
    int32 vnp_id;
    int32 arb_port_base_queue;


    arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;

    for (vnp_id = 0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {
        if (virtual_node_port[unit][vnp_id].base_queue == arb_port_base_queue) {
            return(virtual_node_port[unit][vnp_id].ref_count);
        }
    }
    return(-1);
}

/* free a virtual node/port */
static int32
_bcm_qe2000_cosq_free_virtual_node_port(int unit, int32 base_queue)
{
    int vnp_id;
    int32 arb_port_base_queue;
#ifdef TME_ARB_PORT_OPTIMIZE
    int32 l2_arb_id;
    int32 l1_arb_id;
    arb_group_t *p_l2_arb_group;
#endif
    arb_port_base_queue = base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;

    for (vnp_id = 0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {
        if (virtual_node_port[unit][vnp_id].free == TRUE) {
            continue;
        }

        if (virtual_node_port[unit][vnp_id].base_queue != arb_port_base_queue) {
            continue;
        }

        virtual_node_port[unit][vnp_id].ref_count--;

        if (virtual_node_port[unit][vnp_id].ref_count == 0) {
            virtual_node_port[unit][vnp_id].free = TRUE;
            virtual_node_port[unit][vnp_id].base_queue = -1;

#ifdef TME_ARB_PORT_OPTIMIZE
	    l2_arb_id = vnp_id/BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	    p_l2_arb_group = &level_two_arb_group[unit][l2_arb_id];
	    
	    l1_arb_id = vnp_id%BCM_INT_SBX_ARB_GROUP_LEVEL_ONE_MAX;
	    
	    p_l2_arb_group->dest_node[l1_arb_id] = -1;
	    p_l2_arb_group->dest_port[l1_arb_id] = -1;
#endif

        }

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "free virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
                  vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));


#ifdef TME_ARB_PORT_OPTIMIZE
        LOG_CLI((BSL_META_U(unit,
                            "free virtual node/port(%d) ref_count(%d) base_queue(%d)\n"),
                 vnp_id, virtual_node_port[unit][vnp_id].ref_count, base_queue));
#endif

        return(0);
    }

    LOG_ERROR(BSL_LS_BCM_COMMON,
              (BSL_META_U(unit,
                          "ERROR: attempt to free virtual node port(%d) which has not been allocated\n"),
               vnp_id));
    return(-1);
}

static void
_bcm_qe2000_cosq_queue_depth_denormalize(int32 *p_min_physical_queue_depth_in_bytes, int32 *p_max_physical_queue_depth_in_bytes, sbBool_t bAdd)
{

    int32 bufs[2];
    int32 bufs_unshifted[2];
    int32 buf_index;
    int value;
    int shift;
    int start_value;

    bufs_unshifted[0] = *p_min_physical_queue_depth_in_bytes;
    bufs_unshifted[1] = *p_max_physical_queue_depth_in_bytes;

    /* init to interesting values, if these are seen consistantly      */
    /* in the queue state memory then something is wrong with the      */
    /* calculation loop below.  I.e. we expect these to be overwritten */
    bufs[0] = 0xAB0;
    bufs[1] = 0xCD0;

    /* Determine the minimum/maximum number of buffers required for this queue, round up if required */
    /* A buffer is 4096 bytes - 16 bytes for last line of overhead. */
    for ( buf_index=0; buf_index<2; buf_index++ ) {
        /* 24175: instead of rounding after, round before by adding one less than the divisor to the dividend */
        bufs_unshifted[buf_index] = (bufs_unshifted[buf_index]+(SB_FAB_DEVICE_QE2000_QUEUE_BUFFER_SIZE - 17))/(SB_FAB_DEVICE_QE2000_QUEUE_BUFFER_SIZE - 16);
        /* 24175: arithmetically this was incorrect, rounding before the divide is more efficient */
        /*            if (bufs_unshifted[buf_index]%SB_FAB_DEVICE_QE2000_QUEUE_BUFFER_SIZE) {     */
        /*                bufs_unshifted[buf_index]++;                                            */
        /*            }                                                                           */

        /* now, we need to get as close as possible (but not less) than our unshifted */
        /* value using a base/shift as done by the QMGR.  We have a 11-bit value with */
        /* a 3-bit shift.  So our final buffers will be:                              */
        /*                                                                            */
        /*           #bufs = value[10:0] << shift[2:0]                                */
        /*                                                                            */
        /* with only the 11-bit value, we can handle upto 2048 buffers, or ~8Mbyte of */
        /* queue depth. */
        {
        value = 0;
        start_value = bufs_unshifted[buf_index];

        for ( shift=0; shift<8; shift++ ) {
                if ( start_value < (1<<(11+shift)) ) {
                    value = start_value>>shift;

                    /* now that we have shifted down, round the value up if it   */
                    /* turns out that we've just removed any non-zero bits.      */
                    /* I.E. it's ok for us to assign a higher number of buffers  */
                    /*      than requested, but not ok to assign a lower numnber */
                    if ( (start_value & ((1<<shift)-1)) != 0 ) {
                        value++;
                    }
                    break;
                }
            }
            /* 24175: value of 0 is reasonable if original min depth byte value was 0 also */
            if ( (value == 0) && !((buf_index == 0) && (*p_min_physical_queue_depth_in_bytes == 0))) {

                /* we were not able to determine a reasonable value above */
                /* log this as an error, but pick a reasonable operating  */
                /* value.  With 256Mb as the smallest memory size  we can */
                /* have 4 pages pere queue. */
                if ( bAdd ) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META("Could not resolve queue depth (%d).  Fixing queue depth at (%d bytes)\n"),
                              bufs_unshifted[buf_index], 4*4080));
                    value = 4;
                    shift = 0;
                }
            }

            if ( shift == 7 && (value&0x400)==0x400 ) {
                /* this would produce a value outside the dynamic range */
                /* So we just give it the maximum possible value        */
                value = 0x7FF;
            }

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META("(%s) Value(0x%x, %d) Shift(0x%x, %d)\n"),
                      ((buf_index == 0) ? "MinDepth" : "MaxDepth"),
                      value, value, shift, shift));

            bufs[buf_index] = (value&0x7FF)<<3 | (shift&0x7);
        }
    }

    /* return the denormalized queue depths */

    *p_min_physical_queue_depth_in_bytes = bufs[0];
    *p_max_physical_queue_depth_in_bytes = bufs[1];
}


static void _bcm_qe2000_cosq_anemic_watermark_select_for_params_chosen(bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                                                                       int32 *p_anemic_watermark_select)
{
    if (p_qparams->bw_mode == BCM_COSQ_EF) {
        *p_anemic_watermark_select = 3; /* 3 EF    5/10 linesPerTimeslot    */
    } else {
        *p_anemic_watermark_select = 6; /* bug 23458 set anemic watermark to 8/10 linesPerTimeslot for non-EF */
    }

    /* if user specified it, then use user's value */
    /* NOTE: The following will always get executed as "BCM_DEFAULT_INT32_VALUE" is not used */
    if (p_qparams->anemic_watermark_select != BCM_DEFAULT_INT32_VALUE) {
        *p_anemic_watermark_select = p_qparams->anemic_watermark_select;
    }
}

#define BCM_QE2000_COSQ_QUEUESIZE_NORMALIZE(x)           (((x)>>3) << ((x)&0x7))

static int
_bcm_qe2000_cosq_add_delete_queue(int unit, int add, int32 queue, int32 rate_delta_max_index,
                  bcm_sbx_cosq_queue_params_ingress_t *p_qparams)
{
    uint32 data;
    uint32 timeout;
    int32 min_bufs;
    int32 max_bufs;
    int32 min_physical_queue_depth_in_bytes = 0;
    int32 max_physical_queue_depth_in_bytes = 0;
    int32 local = 0;
    int32 qetype = 0;
    int32 qla_demand_mask = 0;
    int32 queue_header_adjust_sign = 0;
    int32 queue_header_adjust = 0;
    int32 gamma = 0;
    int32 template = 0;
    int32 gain = 0;
    int32 ingress_spi4 = 0;
    int32 ingress_spi4_port = 0;
    int32 bufs[2];
    int32 total_min_bufs_allocated;
    int32 anemic_watermark_select;
    int32 total_bufs_available = 0;
    int32 min_bufs_allocated = 0;
    int32  minimum_bufs;
    int32  delta;

    if ( p_qparams != NULL ) {
        /* verify assumption that params are only valid during an add */
	if (add != TRUE) {
	    return BCM_E_INTERNAL;
	}

        min_physical_queue_depth_in_bytes = p_qparams->min_physical_queue_depth_bytes;
        max_physical_queue_depth_in_bytes = p_qparams->max_physical_queue_depth_bytes;
        local = p_qparams->local;
        qetype = 1; /* 1 = qe2k 0 = qe1k */
        qla_demand_mask = p_qparams->qla_demand_mask;
        queue_header_adjust_sign = p_qparams->queue_header_adjust_sign;
        queue_header_adjust = p_qparams->queue_header_adjust;
        template = p_qparams->template;
        gain = p_qparams->gain;
        ingress_spi4 = p_qparams->ingress_spi4;
        ingress_spi4_port = p_qparams->ingress_spi4_port;
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "(%s) MinBytes(0x%x, %d) MaxBytes(0x%x, %d)\n"),
              ((add == TRUE) ? "Add" : "Delete"),
              min_physical_queue_depth_in_bytes,
              min_physical_queue_depth_in_bytes,
              max_physical_queue_depth_in_bytes,
              max_physical_queue_depth_in_bytes));

    /* address 22039: calculation of min/max buffers is incorrect for QE2000 */
    {

    /* denormalize the queue depth */
    bufs[0] = min_physical_queue_depth_in_bytes;
    bufs[1] = max_physical_queue_depth_in_bytes;
    _bcm_qe2000_cosq_queue_depth_denormalize(&bufs[0], &bufs[1], add);

        if ( add ) {
            min_bufs = bufs[0];
            max_bufs = bufs[1];
        } else {
            /* on delete we just need min bufs */
            min_bufs = min_buffers_for_queue[unit][queue]; /* was saved for use during delete */
            max_bufs = min_bufs;
        }


        if ( add ) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Queue parameter qm_queue_config_data1.min_buf (0x%04x) (num buffers: %d)\n"),
                      min_bufs, ((min_bufs>>3)<<(min_bufs&0x7))));
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Queue parameter qm_queue_config_data0.max_bufs (0x%04x) (num buffers: %d)\n"),
                      max_bufs, ((max_bufs>>3)<<(max_bufs&0x7))));
        }

        /* consistency check */
        if (add) {

            /* Total Available Buffers */
            data = SAND_HAL_READ(unit, KA, QM_CONFIG0);
            total_bufs_available = SAND_HAL_GET_FIELD(KA, QM_CONFIG0, TOTAL_BUFS_AVL, data);

            /* Minimum Buffers already Allocated */
            data = SAND_HAL_READ(unit, KA, QM_CONFIG1);
            min_bufs_allocated = SAND_HAL_GET_FIELD(KA, QM_CONFIG1, TOTAL_MIN_BUFS_ALLOCATED, data);
            minimum_bufs = BCM_QE2000_COSQ_QUEUESIZE_NORMALIZE(min_bufs);

            /* Check if space available to allocate minimum buffers */
            if ((min_bufs_allocated + minimum_bufs) > total_bufs_available) {

                if (total_bufs_available > min_bufs_allocated) {
                    /* Modify minimum buffers to allocate. Allocate buffers so that no */
                    /* scaling is required                                            */
                    delta = ((total_bufs_available - min_bufs_allocated) > 0x7FF) ?
                        0x7FF : (total_bufs_available - min_bufs_allocated);
                }
                else {
                    /* no space available */
                    delta = 0;
                }

                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "Minimum Buffers Exceeds the Available Buffers, TotalBufs: 0x%x MinBufAllocated: 0x%x MinBufAllocateReq: 0x%x MinBufAllocated: 0x%x\n"),
                          total_bufs_available, min_bufs_allocated, minimum_bufs, delta));

                /* Update minimum buffers to allocate, scale is always 0 */
                min_bufs = (delta << 3);
            }
        }

    }


    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Queue(%d) local(%d)\n"),
              (add==TRUE)?"Adding":"Deleting",
              queue, local));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Queue(%d) Full Params: max_bufs(%d) ingress_spi4(%d) ingress_spi4_port(%d)\n"),
              (add==TRUE)?"Adding":"Deleting",
              queue,
              max_bufs, ingress_spi4, ingress_spi4_port));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Queue(%d) Full Params: qetype(%d) rate_delta_max_index(%d) qla_demand_mask(%d)\n"),
              (add==TRUE)?"Adding":"Deleting",
              queue,
              qetype, rate_delta_max_index, qla_demand_mask));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Queue(%d) Full Params: queue_header_adjust_sign(%d) queue_header_adjust(%d) min_bufs(%d)\n"),
              (add==TRUE)?"Adding":"Deleting",
              queue,
              queue_header_adjust_sign, queue_header_adjust, min_bufs));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s Queue(%d) Full Params: gamma(%d) template(%d) gain(%d)\n"),
              (add==TRUE)?"Adding":"Deleting",
              queue,
              gamma, template, gain));


    /* 22313: when compiling under VxWorks, LOCAL is already defined as 'static'   */
    /* 22313: so for files that use LOCAL (only this file) we do a #undef on LOCAL */
    /* 22313: to prevent the macro below from building an improper key:            */
    /* 22313: src/sbFabTargetDeviceQe2000.c:884: `SAND_HAL_KA_QM_QUEUE_CONFIG_DATA0_static_SHIFT' undeclared */
    data = SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA0, LOCAL, local) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA0, MAX_BUFS, max_bufs) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA0, INGRESS_SPI4, ingress_spi4) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA0, INGRESS_PORT, ingress_spi4_port) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA0, QE_TYPE, qetype);

    /* bug 23458 - Configure anemic watermark threshold for latency spike issue */
    if (add) {
	if (p_qparams == NULL) {
	    /* when add queue, has to provide ingress queue parameters */
	    return BCM_E_INTERNAL;
	}
        _bcm_qe2000_cosq_anemic_watermark_select_for_params_chosen(p_qparams, &anemic_watermark_select);
        data = SAND_HAL_MOD_FIELD(KA, QM_QUEUE_CONFIG_DATA0, ANEMIC_WATERMARK_SEL, data, anemic_watermark_select);
    }

    SAND_HAL_WRITE(unit, KA, QM_QUEUE_CONFIG_DATA0, data);

    data = SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA1, RATE_DELTA_MAX_INDEX, rate_delta_max_index) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA1, QLA_DEMAND_MASK, qla_demand_mask) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA1, QUEUE_HDR_ADJUST_SIGN, queue_header_adjust_sign) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA1, QUEUE_HDR_ADJUST, queue_header_adjust) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA1, MIN_BUFS, min_bufs);

    SAND_HAL_WRITE(unit, KA, QM_QUEUE_CONFIG_DATA1, data);

    data = SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA2, GAMMA, gamma) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA2, TEMPLATE, template) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_DATA2, GAIN, gain);

    SAND_HAL_WRITE(unit, KA, QM_QUEUE_CONFIG_DATA2, data);

    data = SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_CTRL, CQ_REQ, 0x1) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_CTRL, ADD_DELETE_N, add) |
        SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_CTRL, QUEUE_NUMBER, queue);


    SAND_HAL_WRITE(unit, KA, QM_QUEUE_CONFIG_CTRL, data);

    timeout = 100;

    while(timeout--) {
        data = SAND_HAL_READ_POLL(unit, KA, QM_QUEUE_CONFIG_CTRL);

        if (SAND_HAL_GET_FIELD(KA, QM_QUEUE_CONFIG_CTRL, CQ_ACK, data)) {
            SAND_HAL_WRITE(unit, KA, QM_QUEUE_CONFIG_CTRL,
                           SAND_HAL_SET_FIELD(KA, QM_QUEUE_CONFIG_CTRL, CQ_ACK, 0x1));
            break;
        }
    }

    if ( timeout == 0 ) {
        if ( add ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "queue add failed\n")));
            return -1;
        } else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "queue delete failed\n")));
            return -1;
        }
    }

    /* address 22048: register QM_CONFIG1 total_min_bufs_allocated needs to be set */
    {
        data = SAND_HAL_READ((sbhandle)unit, KA, QM_CONFIG1);
        total_min_bufs_allocated = SAND_HAL_GET_FIELD(KA, QM_CONFIG1, TOTAL_MIN_BUFS_ALLOCATED, data);
        if ( add ) {
            if (min_buffers_for_queue[unit][queue] != 0 ) {
                LOG_WARN(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "WARNING: Found min_buffers_for_queue[%d][%d]!=0 before setting value (%d)\n"),
                          unit, queue, min_buffers_for_queue[unit][queue]));
            }
            min_buffers_for_queue[unit][queue] = min_bufs; /* save for use during delete */
            total_min_bufs_allocated += BCM_QE2000_COSQ_QUEUESIZE_NORMALIZE(min_bufs);
        } else {
            total_min_bufs_allocated -= BCM_QE2000_COSQ_QUEUESIZE_NORMALIZE(min_buffers_for_queue[unit][queue]);
            min_buffers_for_queue[unit][queue] = 0;
        }


        if ( total_min_bufs_allocated < 0 ) {
            /* it's possible this could happen if we've had some failed operations   */
            /* (i.e. good deleteQueue, followed by bad addQueue).  Just push in zero */
            LOG_CLI((BSL_META_U(unit,
                                "Found total_min_bufs_allocated<0.  Resetting to 0, but system may be unstable.\n")));
            total_min_bufs_allocated = 0;
        } else {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Setting total_min_bufs_allocated to %d\n"),
                      total_min_bufs_allocated));
        }

        data = SAND_HAL_MOD_FIELD(KA, QM_CONFIG1, TOTAL_MIN_BUFS_ALLOCATED, data, total_min_bufs_allocated);
        SAND_HAL_WRITE((sbhandle)unit, KA, QM_CONFIG1, data);
    }

    return SB_FAB_STATUS_OK;
}

static int
_bcm_qe2000_cosq_select_queue_params(int unit, int queue, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                     int32 *p_hold_ts, int32 *p_qtype,
                     uint32 *p_rate_a, uint32 *p_rate_b,
                     bcm_sbx_cosq_queue_region_type_t queue_region)
{
    uint32 guaranteed_rate_in_kbps;
    uint32 rate_in_256_kbits_integer;
    uint32 rate_in_256_kbits_fraction;
    int32 bw_mode;
    int32 sp_priority;


    bw_mode = p_qparams->bw_mode;
    sp_priority = p_qparams->bw_value.sp_priority;
    *p_rate_a = *p_rate_b = 0;

    /* From the QE2000 Spec, here are the values were are configuring: */
    /*                                                                 */
    /*      4'b0000=BE, 4'b0001=AF, 4'b0010=EF                         */
    /*      4'b1000=SP0, 4'b1001=SP1, 4'b1010=SP2, 4'b1011=SP3         */
    /*      4'b1100=SP4, 4'b1101=SP5, 4'b1110=SP6, 4'b1111=SP7         */

    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if (bw_mode == BCM_COSQ_SP) {
            _bcm_qe2000_cosq_map_sp2gsp(unit, queue, &bw_mode, &sp_priority);
        }
    }

    switch ( bw_mode ) {
        case BCM_COSQ_SP :

            /* Strict Pri is in the bag so it is setup as AF inside QOS           */
            /* This is because there are 2 definitions for SP - the QOS version   */
            /* and the BAA version, in the BAA, SP is set up as SP, in QOS, set   */
            /* up as AF so that SP traffic does not exceed the CIR bandwidth of   */
            /* the BAG */
            *p_qtype = 1;
            break;
        case BCM_COSQ_SP_GLOBAL :
            {
                uint32 uPriority;

                /* Strict Pri is out of the bag and set up as SP inside QOS           */
                /* 8 corresponds to SP0, 9 to SP1, ...up to 15 for SP7                */
                /* SP0 is the lowest priority and SP7 is the highest, so swap below   */
                /* in flib, the lowest COS level is the highest priority (when in the */
                /* bag, so make this operate similarly, lowest SP is the highest pri  */
                switch(sp_priority) {
                    case 0:
                        uPriority = 15;
                        break;
                    case 1:
                        uPriority = 14;
                        break;
                    case 2:
                        uPriority = 13;
                        break;
                    case 3:
                        uPriority = 12;
                        break;
                    case 4:
                        uPriority = 11;
                        break;
                    case 5:
                        uPriority = 10;
                        break;
                    case 6:
                        uPriority = 9;
                        break;
                    case 7:
                    default:
                        uPriority = 8;
                        break;
                }
                *p_qtype = uPriority;

                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "bw mode PATH_SP_GLOBAL, priority set to 0x%x\n"),
                          *p_qtype));

            }
            break;

        case BCM_COSQ_EF :
            *p_qtype = 0x2;
            break;
        case BCM_COSQ_AF :
            *p_qtype = 0x1;
            {
        guaranteed_rate_in_kbps  = p_qparams->bw_value.guarantee_kbps;

                rate_in_256_kbits_integer = (guaranteed_rate_in_kbps * 1024)/(1000000/256);
                rate_in_256_kbits_fraction = (guaranteed_rate_in_kbps * 1024)%(1000000/256);

                rate_in_256_kbits_fraction = rate_in_256_kbits_fraction/256;
                *p_rate_a = (rate_in_256_kbits_integer << 8) | (rate_in_256_kbits_fraction & 0xff);
                *p_rate_b = *p_rate_a;

                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "AF set guarantee(%d)b/s integer(%d) per 256Kns fraction (%d) per 256Kns\n"),
                          guaranteed_rate_in_kbps,
                          (int32)rate_in_256_kbits_integer,
                          (int32)rate_in_256_kbits_fraction));
            }
            
            break;
        case BCM_COSQ_BE :
            *p_qtype = 0x0;
            break;
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING :
            *p_qtype = 0x1; /* WFQ is treated as AF by QOS */
            break;

        case BCM_COSQ_GSP0:
            *p_qtype = 8;
            break;
        case BCM_COSQ_GSP1:
            *p_qtype = 9;
            break;
        case BCM_COSQ_GSP2:
            *p_qtype = 10;
            break;
        case BCM_COSQ_GSP3:
            *p_qtype = 11;
            break;
        case BCM_COSQ_GSP4:
            *p_qtype = 12;
            break;
        case BCM_COSQ_GSP5:
            *p_qtype = 13;
            break;
        case BCM_COSQ_GSP6:
            *p_qtype = 14;
            break;
        case BCM_COSQ_GSP7:
            *p_qtype = 15;
            break;


        default:
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Unsupported bw_mode(%d) found during AddQueue operation\n"),
                      p_qparams->bw_mode));
            return -1;
    }

    /* if in halfbus mode, use hold_ts of 4 (3 in table), otherwise, use 2 (1 in table) */
    {

        /* compute overrides */
        if (p_qparams->hold_pri_num_timeslots == BCM_DEFAULT_INT32_VALUE){
            if (SOC_SBX_CFG(unit)->bHalfBus == TRUE) {
                *p_hold_ts = 3;
            }else {
                *p_hold_ts = 1;
            }
        }else {
            /* If user specified it, then use user's value*/
            *p_hold_ts = p_qparams->hold_pri_num_timeslots;
        }
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "hold_ts=0x%x\n"),
	          *p_hold_ts));
    }

    return SB_FAB_STATUS_OK;
}

static int
_bcm_qe2000_cosq_set_queue_group_ingress_shaper(int unit,
                        int base_queue,
                        int num_cos_levels,
                        uint32 shape_limit_kbps,
                        int enable_shaping) {
    int status;
    int saved_status = BCM_E_NONE;
    int32 queue;
    uint32 per_queue_shape_limit_kbps;
    bcm_sbx_cosq_queue_state_t *p_qstate;

    per_queue_shape_limit_kbps = _bcm_qe2000_cosq_queuegroup_to_queue_shape_burst_get(unit, num_cos_levels, shape_limit_kbps);

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Setting per queue shape(%d) based on port_shape(%d) and num_queues(%d)\n"),
              per_queue_shape_limit_kbps, shape_limit_kbps, num_cos_levels));

    
    for ( queue = base_queue;
          queue<(base_queue + num_cos_levels);
          queue++ ) {

        status = _bcm_qe2000_cosq_set_ingress_shaper(unit, queue,
                                                     per_queue_shape_limit_kbps,
                                                     enable_shaping);
        if ( status ) {
            /* save it, but do not return.  Instead, continue and try and config all queues */
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Failed to setup shaper for queue(%d)\n"),
                       queue));
            saved_status = status;
        }

        p_qstate[queue].ingress.enable_shaping = enable_shaping;
        p_qstate[queue].ingress.shape_limit_kbps = per_queue_shape_limit_kbps;

    }

    return saved_status;
}

static int
_bcm_qe2000_cosq_set_ingress_shaper(int unit,
                    int32 queue,
                    uint32 shape_limit_kbps,
                    int enable_shaping) {
    int status;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int shape_enable, maxburst;
    uint32 shape_max_burst_table_value = 0;
    uint32 shape_rate_256_kbits = _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(unit, shape_limit_kbps);


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    /* initialize chip bucket state to 0. This will reduce latency. It could */
    /* temporarily have an effect on burst.                                  */
    status = soc_qe2000_shape_bucket_set(unit, queue, 0);
    if (status != SOC_E_NONE) {
        return -1;
    }

    /* In order to achieve the requested shape rate, max burst at a minimum */
    /* must be one shaper interval's worth of bits.  Force that to be the   */
    /* case here. */
    shape_max_burst_table_value = shape_rate_256_kbits *
                                  SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress;

    status = soc_qe2000_shape_rate_set(unit, queue, 0 /*static mode always */, shape_rate_256_kbits);
    if (status != SOC_E_NONE) {
        return -1;
    }

    if (enable_shaping ) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Shaping parameter shape_rate_256_kbits (%d)\n"),
                  shape_rate_256_kbits));
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Enable shaping, max burst (%d) bits\n"),
                  shape_max_burst_table_value));
    } else {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Disable shaping, max burst (%d) bits\n"),
                  shape_max_burst_table_value));
    }


    if (p_qstate[queue].ingress.shape_burst_kbps == -1) {
        /* set both enable and burst */
        status = soc_qe2000_shape_maxburst_set(unit, queue, enable_shaping, shape_max_burst_table_value);
    }
    else {
        /* only set enable */
        status = soc_qe2000_shape_maxburst_get(unit, queue, &shape_enable, &maxburst);
        if (status != SOC_E_NONE) {
            return(status);
        }

        status = soc_qe2000_shape_maxburst_set(unit, queue, enable_shaping, maxburst);
        if (status != SOC_E_NONE) {
            return(status);
        }
    }
    if (status) {
        return -1;
    }

    return 0;
}

static void _bcm_qe2000_anemic_age_threshold_key_for_params_chosen(int unit, int queue, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                                   int32 *p_anemic_age_threshold_key)
{
    int32 bw_mode;
    int32 sp_priority;


    bw_mode = p_qparams->bw_mode;
    sp_priority = p_qparams->bw_value.sp_priority;
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if (bw_mode == BCM_COSQ_SP) {
            _bcm_qe2000_cosq_map_sp2gsp(unit, queue, &bw_mode, &sp_priority);
        }
    }

    /* compute overrides */
    if (bw_mode == BCM_COSQ_EF){
        *p_anemic_age_threshold_key = 0; /* 0 EF    0x04  (32us) 0xff   */
    }else if (bw_mode == BCM_COSQ_SP_GLOBAL && sp_priority == 0){
        *p_anemic_age_threshold_key = 2;  /* 1 SP7   0x0C  (96us) 0xff  */
    }else if (bw_mode == BCM_COSQ_SP_GLOBAL && sp_priority == 1){
        *p_anemic_age_threshold_key = 4;  /* 1 SP6   0x14  (160us) 0xff */
    }else{
        *p_anemic_age_threshold_key = 6; /* bug 23459 set anemic age threshold by default to 240/0xff */
    }

    /* NOTE: The following will always get executed as "BCM_DEFAULT_INT32_VALUE" is not used */
    /* If user specified it, then use user's value*/
    if (p_qparams->anemic_age_threshold_key != BCM_DEFAULT_INT32_VALUE)
        *p_anemic_age_threshold_key = p_qparams->anemic_age_threshold_key;
}

static void _bcm_qe2000_rate_delta_max_index_for_port_rate_chosen(int unit, int bw_mode,
                                  uint32 bag_rate_kbps,
                                  int32 *prate_delta_max_index)
{
#if 0
    uint32 uPercentOffset = 2; /* 17 percent */
#endif /* 0 */
    /*
     * The rate delta max table was set up as follows in qe2000_init.c
     *
     * Entry RATE            10%     13%     17%     20%
     * 0      1M             129     168     220     259
     * 1      2M             259     336     440     518
     * 2      5M             648     842    1101    1259
     * 3      7M             906    1179    1541    1813
     * 4     10M            1296    1684    2202    2591
     * 5     20M            2591    3368    4405    5182
     * 6     50M            6487    8421   11013   12596
     * 7     70M            9069   11790   15418   18139
     * 8     100M          12956   16843   22026   25913
     * 9     200M          25913   33687   44053   51827
     * 10    500M          64873   84219  110132  125967
     * 11    700M          90697  117906  154185  181395
     * 12    1G           129567  168438  220265  259135
     * 13    2G           259134  336876  440530  518270
     * 14    5G           647839  842191 1101327 1295679
     * 15    10G         1295679 1684382 2202654 2591358
     * Set rate to 13% as an initial value - determine optimal after further testing
     */

    soc_qe2000_rate_delta_index_get(0 /* not used */, bag_rate_kbps, (uint32 *)prate_delta_max_index);

    /* Determine if there is change in template due to "sched mode" change */
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if (bw_mode == BCM_COSQ_SP) {
           (*prate_delta_max_index) = 0;
        }
    }

#if 0
    if (bag_rate_kbps < 1000) {
        *prate_delta_max_index = (0<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 2000) {
        *prate_delta_max_index = (1<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 5000) {
        *prate_delta_max_index = (2<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 7000) {
        *prate_delta_max_index = (3<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 10000) {
        *prate_delta_max_index = (4<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 20000) {
        *prate_delta_max_index = (5<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 50000) {
        *prate_delta_max_index = (6<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 70000) {
        *prate_delta_max_index = (7<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 100000) {
        *prate_delta_max_index = (8<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 200000) {
        *prate_delta_max_index = (9<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 500000) {
        *prate_delta_max_index = (10<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 700000) {
        *prate_delta_max_index = (11<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 1000000) {
        *prate_delta_max_index = (12<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 2000000) {
        *prate_delta_max_index = (13<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 5000000) {
        *prate_delta_max_index = (14<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 10000000) {
        *prate_delta_max_index = (15<<2) + uPercentOffset;
    }
#endif /* 0 */
}


#define SB_FAB_DEVICE_QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK  0x01

static int
_bcm_qe2000_cosq_enable_disable_queue(int unit, int queue, int enable)
{
    int status = BCM_E_NONE;
    uint32 data0, data1, data2, data3;

    /* read queue state entry */
    status = soc_qe2000_qm_mem_read(unit, queue, 0x00, &data0, &data1, &data2, &data3);
    if (status) {
        return(status);
    }

    if (enable) {
        /* enable queue */
        data0 |= SB_FAB_DEVICE_QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK;
    }
    else {
        /* disable queue */
        data0 &= ~(SB_FAB_DEVICE_QE2000_QM_MEM_ACC_DATA0_QS_ENABLE_MASK);
    }

    status = soc_qe2000_qm_mem_write(unit, queue, 0x00, data0, data1, data2, data3);
    if (status) {
        return(status);
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Queue is (%s)\n"),
              ((enable == TRUE) ? "ENABLED" : "DISABLED")));
    return(status);
}

int
bcm_qe2000_cosq_sysport_port_remap(int unit, int sysport, int ef, int remap_port)
{
    int nVirtualNodePort, nEgressFifoId;
    sbBool_t bMc, bTme;
    int txdma;
    int status = BCM_E_NONE;

    if ( ( remap_port < 0 ) ||
         ( remap_port > 49 ) ) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Remap_port (%d) is out of range\n"),
                  remap_port));
        return BCM_E_PARAM;
    }

    /* When the max esets is <= 128 , do not use lower sysports for multicast */
    if ( ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
	  (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) &&
         ((sysport & 0x3F) < BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP) &&
         (SOC_SBX_CFG(unit)->num_ds_ids>128) ) {
        /* Extended eset */
        bMc = 1;
        if (ef) {
            nEgressFifoId = 100;
        } else {
            nEgressFifoId = 101;
        }
    } else {
        /* Unicast port
         *  EF goto even fifo, non-EF goto odd fifo
         *  Fifo = remap *2 + (!ef)
         */
        bMc = 0;
        if (ef) {
            nEgressFifoId = remap_port * 2;
        } else {
            nEgressFifoId = remap_port * 2 + 1;
        }
    }

    /* address = [txdma][ef][qe1k][sysport5:0], 9 bits total program both txdma = 0 and txdma = 1 entry
     */
    bTme = 0;
    txdma = 0;
    nVirtualNodePort = (((txdma << 8) | (ef << 7))) | (sysport & 0x3F);
    status = soc_qe2000_eg_mem_port_remap_table_write(unit, nVirtualNodePort, bMc, bTme, 1, nEgressFifoId);
    if (status) {
        return BCM_E_INTERNAL;
    }

    txdma = 1;
    nVirtualNodePort = (((txdma << 8) | (ef << 7))) | (sysport & 0x3F);
    status = soc_qe2000_eg_mem_port_remap_table_write(unit, nVirtualNodePort, bMc, bTme, 1, nEgressFifoId);
    if (status) {
        return BCM_E_INTERNAL;
    }

    return BCM_E_NONE;
}

static int
_bcm_qe2000_cosq_map_sp2gsp(int unit, int queue, int32 *p_bw_mode, int32 *p_sp_priority)
{
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int32 bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 base_queue, cur_queue;
    int32 num_cos, sp_num_cos;
    int rc = BCM_E_NONE;


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    bw_group = p_qstate[queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];
    base_queue = p_bwstate->base_queue;

    for (cur_queue = base_queue, num_cos = 0, sp_num_cos = 0;
                  cur_queue < (base_queue + p_bwstate->num_cos); cur_queue++, num_cos++) {
        if (p_qstate[cur_queue].ingress.bw_mode == BCM_COSQ_SP) {
            sp_num_cos++;
        }

        if (cur_queue == queue) {
            (*p_bw_mode) = BCM_COSQ_SP_GLOBAL;

            /* no of queues of type BCM_COSQ_SP have to be less then 8 */
            (*p_sp_priority) = (sp_num_cos <= 8) ? (sp_num_cos - 1) : 7;
            break;
        }
    }

    return(rc);
}

int
_bcm_qe2000_cosq_update_gsp(int unit, int queue)
{
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int32 bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 base_queue, cur_queue;
    int32 sp_num_cos;
    int rc = BCM_E_NONE, found = FALSE;
    int expected_qtype, local, hold_ts, q_type;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    bw_group = p_qstate[queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];
    base_queue = p_bwstate->base_queue;

    for (cur_queue = base_queue, sp_num_cos = 0;
                  cur_queue < (base_queue + p_bwstate->num_cos); cur_queue++) {

        found = FALSE;
        if (cur_queue == queue) {
            found = TRUE;
        }
        else if (p_qstate[cur_queue].ingress.bw_mode == BCM_COSQ_SP) {
            found = TRUE;
        }

        if (found == TRUE) {
            sp_num_cos++;
            found = FALSE;

            /* determine expected queue type */
            switch((sp_num_cos - 1)) {
                case 0:
                    expected_qtype = 15;
                    break;
                case 1:
                    expected_qtype = 14;
                    break;
                case 2:
                    expected_qtype = 13;
                    break;
                case 3:
                    expected_qtype = 12;
                    break;
                case 4:
                    expected_qtype = 11;
                    break;
                case 5:
                    expected_qtype = 10;
                    break;
                case 6:
                    expected_qtype = 9;
                    break;
                case 7:
                default:
                    expected_qtype = 8;
                    break;
            }

           /* retreive current queue type */
           rc = soc_qe2000_queue_para_get(unit, cur_queue, &local, &hold_ts, &q_type);
           if (rc != SOC_E_NONE) {
               LOG_ERROR(BSL_LS_BCM_COMMON,
                         (BSL_META_U(unit,
                                     "ERROR: queue param table read failure\n")));
               rc = -1;
               break;
           }
           /* determine if queue type has to be updated */
           if (expected_qtype != q_type) {
               rc = soc_qe2000_queue_para_set(unit, cur_queue, local, hold_ts, expected_qtype);
               if (rc != SOC_E_NONE) {
                   LOG_ERROR(BSL_LS_BCM_COMMON,
                             (BSL_META_U(unit,
                                         "ERROR: queue param table write failure\n")));
                   rc = -1;
                   break;
               }
           }
        }
    }

    return(rc);
}

#define QE2000_SOC_IF_ERROR_RETURN(op)  \
  do {                                  \
      int __rv__;                       \
      if ((__rv__ = (op)) < 0) {        \
          return(__rv__);               \
      }                                 \
  } while(0)

int
bcm_qe2000_cosq_control_set(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_control_t type,
                            int value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int rc = BCM_E_NONE;
    int rv = SOC_E_NONE;
    bcm_sbx_cosq_queue_state_t *queue_state;
    int32 bw_group;
    int32 queue;
    int local = 0, hold_ts = 0, q_type = 0;
    int base_queue=0, num_cos=0;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_params_ingress_t new_qparams;
    bcm_sbx_cosq_bw_group_params_t new_bwparams;
    bcm_sbx_queue_size_info_t sizeInfo = {0, 0};
    int hwVal, template;
    int is_deallocated;


    switch (type) {
        case bcmCosqControlFabricConnectMinUtilization:
            /* rip the bits we want from API's 'fractional' representation */
            value = BCM_COSQ_CONTROL_FABRIC_CONNECT_MIN_UTILIZATION_WHOLE_GET(value);
            /* adjust the value to hardware representation */
            rv = soc_qe2000_template_min_util_adjust(unit,
                                                     (bcm_sbx_cosq_queue_region_local==control_info->u.conn.queue_region)?SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL:SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000,
                                                     value,
                                                     &hwVal);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* get current template for this queue */
            rv = soc_qe2000_queue_min_util_get(unit,
                                               control_info->u.conn.queue,
                                               &template);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* get template's cached setting */
            rv = soc_sbx_connect_min_util_get(unit, template, &local);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* see if we actually need to do anything */
            if (hwVal == local) {
                /* already at desired value, successfully did nothing */
                return BCM_E_NONE;
            }

            /* need to find a new template to use; alloc or find existing */
            rv = soc_sbx_connect_min_util_alloc(unit,
                                                0,
                                                hwVal,
                                                &(control_info->u.conn.is_allocated),
                                                &(control_info->u.conn.template));
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* if required, configure newly allocated template */
            if (control_info->u.conn.is_allocated) {
                rv = soc_qe2000_template_min_util_set(unit,
                                                      control_info->u.conn.template,
                                                      hwVal);
            }

            /* set up new queue configuration */
            if (SOC_E_NONE == rv) {
                /* queue related configuration                      */
                /* This requires deleting and then adding the queue */
                queue = control_info->u.conn.queue;
                queue_state = (bcm_sbx_cosq_queue_state_t *)SOC_SBX_STATE(unit)->queue_state;
                bw_group = queue_state[queue].bw_group;
                p_bwstate = (bcm_sbx_cosq_bw_group_state_t *)SOC_SBX_STATE(unit)->bw_group_state;
                p_bwstate = &p_bwstate[bw_group];
                new_qparams = queue_state[queue].ingress;
                new_bwparams = p_bwstate->path;

                /* update with the template configuration */
                new_qparams.anemic_watermark_select = control_info->u.conn.template;

                rc = bcm_qe2000_cosq_set_ingress_params(unit,
                                                        queue,
                                                        control_info->u.conn.queue_region,
                                                        &new_qparams,
                                                        &queue_state[queue].ingress,
                                                        &new_bwparams,
                                                        &p_bwstate->path,
                                                        &sizeInfo);
            } else { /* if (SOC_E_NONE == rv) */
                rc = rv;
            } /* if (SOC_E_NONE == rv) */

            if (BCM_E_NONE == rc) {
                /* update good; free the old template instance */
                rv = soc_sbx_connect_min_util_dealloc(unit,
                                                      0,
                                                      local,
                                                      &is_deallocated,
                                                      &template);
                rc = rv;
            } else { /* if (BCM_E_NONE == rc) */
                /* something went wrong; free newer template instance */
                rv = soc_sbx_connect_min_util_dealloc(unit,
                                                      0,
                                                      hwVal,
                                                      &is_deallocated,
                                                      &(control_info->u.conn.template));
                /* we don't return this call's result; prior err has preced. */
            } /* if (BCM_E_NONE == rc) */
            break;

        case bcmCosqControlFabricConnectMaxTime:
            /* adjust the value to hardware representation */
            rv = soc_qe2000_template_max_age_adjust(unit,
                                                    value,
                                                    &hwVal);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* get current template for this queue */
            rv = soc_qe2000_queue_max_age_get(unit,
                                              control_info->u.conn.queue,
                                              &template);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* get template's cached setting */
            rv = soc_sbx_connect_max_age_get(unit, template, &local);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* see if we actually need to do anything */
            if (hwVal == local) {
                /* already at desired value, successfully did nothing */
                return BCM_E_NONE;
            }

            /* need to find a new template to use; alloc or find existing */
            rv = soc_sbx_connect_max_age_alloc(unit,
                                               0,
                                               hwVal,
                                               &(control_info->u.conn.is_allocated),
                                               &(control_info->u.conn.template));
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* if required, configure newly allocated template */
            if (control_info->u.conn.is_allocated) {
                rv = soc_qe2000_template_max_age_set(unit,
                                                     control_info->u.conn.template,
                                                     hwVal);
            }

            /* change queue configuration */
            if (SOC_E_NONE == rv) {
                rv = soc_qe2000_queue_max_age_set(unit,
                                                  control_info->u.conn.queue,
                                                  control_info->u.conn.template);
            }

            if (SOC_E_NONE == rv) {
                /* update queue state to reflect new values */
                queue = control_info->u.conn.queue;
                queue_state = (bcm_sbx_cosq_queue_state_t *)SOC_SBX_STATE(unit)->queue_state;
                queue_state[queue].ingress.anemic_age_threshold_key = control_info->u.conn.template;
                /* free the old template instance */
                rv = soc_sbx_connect_max_age_dealloc(unit,
                                                     0,
                                                     local,
                                                     &is_deallocated,
                                                     &template);
                rc = rv;
            } else { /* if (SOC_E_NONE == rv) */
                rc = rv;
                /* something went wrong; free newer template instance */
                rv = soc_sbx_connect_max_age_dealloc(unit,
                                                     0,
                                                     hwVal,
                                                     &is_deallocated,
                                                     &(control_info->u.conn.template));
                /* we don't return this call's result; prior err has preced. */
            } /* if (SOC_E_NONE == rv) */
            break;

        case bcmCosqControlFabricConnectMax:
	    if (value < 1) {
		value = 1;
	    }
	    if (value > 8) {
		value = 8;
	    }
	    /*
	     * adjust value (h/w adds 1)
	     */
	    value--;
	    SOC_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
	    queue_state =  &(SOC_SBX_STATE(unit)->queue_state[base_queue+cosq]);
	    SOC_IF_ERROR_RETURN(soc_qe2000_queue_para_get(unit, base_queue+cosq, &local, &hold_ts, &q_type));
	    SOC_IF_ERROR_RETURN(soc_qe2000_queue_para_set(unit, base_queue+cosq, local, value, q_type));
	    queue_state->ingress.hold_pri_num_timeslots = value;
            break;

        case bcmCosqControlPacketLengthAdjust:
            rc = _bcm_qe2000_cosq_queue_packet_length_adjust_set(unit, control_info->u.conn.queue, value);
            break;

        case bcmCosqControlBandwidthBurstMax:
            /* currently only unicast/multicast gport support (i.e on a queue) */
	    SOC_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
            rc = _bcm_qe2000_cosq_queue_burst_set(unit, base_queue, cosq, control_info->u.conn.num_cos, control_info->u.conn.queue, value, TRUE);
            break;

        default:
            rc = BCM_E_PARAM;
            break;

    }

    return(rc);
}

int
bcm_qe2000_cosq_control_get(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_control_t type,
                            int *value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int rc = BCM_E_NONE;
    int template, local=0, hold_ts=0, q_type=0;
    int base_queue=0, num_cos=0;

    switch (type) {
        case bcmCosqControlFabricConnectMinUtilization:
            /* determine template being used by the queue */
            rc = soc_qe2000_queue_min_util_get(unit,
                                               control_info->u.conn.queue,
                                               &template);
            if (BCM_E_NONE != rc) {
                return(rc);
            }

            /* get template configuration from cache */
            rc = soc_sbx_connect_min_util_get(unit, template, &local);
            if (BCM_E_NONE != rc) {
                return(rc);
            }

            /* convert it back to what the caller uses */
            rc = soc_qe2000_template_min_util_recall(unit,
                                                     (bcm_sbx_cosq_queue_region_local==control_info->u.conn.queue_region)?SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL:SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000,
                                                     local,
                                                     &local);
            if (BCM_E_NONE == rc) {
                /* munge it back to the API 'fractional' format */
                *value = BCM_COSQ_CONTROL_FABRIC_CONNECT_MIN_UTILIZATION_SET(local, 0);
            }
            break;

        case bcmCosqControlFabricConnectMaxTime:
            /* determine template being used by the queue */
            rc = soc_qe2000_queue_max_age_get(unit,
                                              control_info->u.conn.queue,
                                              &template);
            if (rc != BCM_E_NONE) {
                return(rc);
            }

            /* get template configuration */
            rc = soc_sbx_connect_max_age_get(unit, template, &local);
            if (rc != BCM_E_NONE) {
                return(rc);
            }

            /* convert it back to what the caller uses */
            rc = soc_qe2000_template_max_age_recall(unit,
                                                    local,
                                                    value);
            break;

        case bcmCosqControlFabricConnectMax:
	    SOC_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
	    SOC_IF_ERROR_RETURN(soc_qe2000_queue_para_get(unit, base_queue+cosq, &local, &hold_ts, &q_type));
	    *value = hold_ts + 1;
	    break;

        case bcmCosqControlPacketLengthAdjust:
            rc = _bcm_qe2000_cosq_queue_packet_length_adjust_get(unit, control_info->u.conn.queue, value);
            break;

        case bcmCosqControlBandwidthBurstMax:
            /* currently only unicast/multicast gport support (i.e on a queue) */
	    SOC_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
            rc = _bcm_qe2000_cosq_queue_burst_get(unit, gport, cosq, control_info->u.conn.num_cos, control_info->u.conn.queue, value, TRUE);
            break;

        default:
            rc = BCM_E_PARAM;
            break;
    }

    return(rc);
}


#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_qe2000_cosq_state_get(int unit,
			  char *pbuf)
{
    int rv;
    char *pbuf_current = pbuf;
    int vnp_id;

    if (soc_feature(unit, soc_feature_node)) {
	if ((soc_feature(unit, soc_feature_standalone)) || (soc_feature(unit, soc_feature_hybrid))) {

	    for (vnp_id = 0; vnp_id < BCM_SBX_DEVICE_COSQ_MAX_LNAS; vnp_id++) {

		if (virtual_node_port[unit][vnp_id].state == BCM_INT_SBX_ARBPORT_STATE_GLOBAL) {
		    continue;
		}
		if (virtual_node_port[unit][vnp_id].free == TRUE) {
		    continue;
		}

		rv = sal_sprintf(pbuf_current, "local vnp_id:%d is in use base_queue(%d) ref_count(%d)\n", vnp_id,
				 virtual_node_port[unit][vnp_id].base_queue,
				 virtual_node_port[unit][vnp_id].ref_count);
		if (rv < 0) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "rv = %d\n"),
		               rv));
		    return BCM_E_RESOURCE;
		}
		pbuf_current += rv;
	    }
	    return BCM_E_NONE;
	}
    }
    return BCM_E_NONE;
}
#endif
#endif

int
bcm_qe2000_cosq_egress_size_set(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 bytes_min,
                                uint32 bytes_max)
{
    int rc = BCM_E_NONE;
    bcm_port_t port;

    if (gport == BCM_GPORT_INVALID) {
	return BCM_E_PARAM;
    }

    /* Unicast fifos are cos levels 0 and 1 */
    if ((cosq == BCM_INT_XCORE_COS_FIFO_UNICAST_EF) || 
	(cosq == BCM_INT_XCORE_COS_FIFO_UNICAST_NEF)) {
	
	if (BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport) >= 0) {
	    port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
	    rc = bcm_qe2000_port_size_set(unit, port, cosq, bytes_min, bytes_max);
	} else {
	    rc = BCM_E_PARAM;
	}
    }
    /* Multicast fifos are cos levels 2 and 3 */
    else if ((cosq == BCM_INT_XCORE_COS_FIFO_MULTICAST_EF) || 
	     (cosq == BCM_INT_XCORE_COS_FIFO_MULTICAST_NEF)) {

	rc = bcm_qe2000_port_multicast_size_set(unit, cosq, bytes_min, bytes_max);
    }
    else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "cos level out of range (0-3) valid value passed (%d)\n"),
                   cosq));
        rc = BCM_E_PARAM;
    }

    return(rc);
}

int
bcm_qe2000_cosq_egress_size_get(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 *bytes_min,
                                uint32 *bytes_max)
{
    int rc = BCM_E_NONE;
    bcm_port_t port;

    if (gport == BCM_GPORT_INVALID) {
	return BCM_E_PARAM;
    }

    if  ((cosq == BCM_INT_XCORE_COS_FIFO_UNICAST_EF) || 
	 (cosq == BCM_INT_XCORE_COS_FIFO_UNICAST_NEF)) {

	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
	rc = bcm_qe2000_port_size_get(unit, port, cosq, bytes_min, bytes_max);
    }
    else if  ((cosq == BCM_INT_XCORE_COS_FIFO_MULTICAST_EF) || 
	      (cosq == BCM_INT_XCORE_COS_FIFO_MULTICAST_NEF)) {
        rc = bcm_qe2000_port_multicast_size_get(unit, cosq, bytes_min, bytes_max);
    }
    else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "cos level out of range (0-3) valid value passed (%d)\n"),
                   cosq));
        rc = BCM_E_PARAM;
    } 
  return(rc);
}

int
bcm_qe2000_cosq_subscriber_map_add(int unit,
				   bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_FE2000_SUPPORT
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbG2EplibCtxt_st *ep;
    uint32 ip_table_mem[4];
    int oix,queue,vlan, port;
    int base_queue = 0, offset = 0;

    if (sbx == NULL) {
	return BCM_E_INIT;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
        LOG_CLI((BSL_META_U(unit,
                            "\n map_add not allowed in FIC mode\n")));
	return BCM_E_UNAVAIL;
    }

    ep = (sbG2EplibCtxt_st *) sbx->drv;

    if (map == NULL) {
	return BCM_E_PARAM;
    }

    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
    switch(map->flags) {
	case BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN:
	    base_queue = map->encap_id;
	    queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map->queue_id);
	    if (queue == 0xffffffff) {
		return BCM_E_PARAM;
	    }
	    offset = queue - base_queue;

	    vlan = map->vlan;
	    rv = sbG2EplibVlanRemapGet(ep, vlan, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: showing vlan(%d) for %s[u:%d] failed\n"), vlan,
                         SOC_CHIP_STRING(unit),unit));
	    }
	    base_queue = (base_queue >> 8) | ((base_queue & 0xFF) << 8);
	    switch (vlan & 3) {
		case 0:
		    ip_table_mem[1] &= 0xffff;
		    ip_table_mem[1] |= (base_queue << 16);
		    break;
		case 1:
		    ip_table_mem[1] &= 0xffff0000;
		    ip_table_mem[1] |= base_queue;
		    break;
		case 2:
		    ip_table_mem[0] &= 0xffff;
		    ip_table_mem[0] |= (base_queue << 16);
		    break;
		case 3:
		    ip_table_mem[0] &= 0xffff0000;
		    ip_table_mem[0] |= base_queue;
		    break;
	    }
	    rv = sbG2EplibVlanRemapSet(ep, vlan, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: setting vlan(%d) for %s[u:%d] failed\n"), vlan,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } else {
		rv = 0;
	    }

	    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
	    port = map->port;
	    rv = sbG2EplibPortEncapGet(ep, port, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: showing port(%d) for %s[u:%d] failed\n"), port,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } 
	    offset = (offset >> 8) | ((offset & 0xFF) << 8);
	    switch (port & 3) {
		case 0:
		    ip_table_mem[1] &= 0xffff;
		    ip_table_mem[1] |= (offset << 16);
		    break;
		case 1:
		    ip_table_mem[1] &= 0xffff0000;
		    ip_table_mem[1] |= offset;
		    break;
		case 2:
		    ip_table_mem[0] &= 0xffff;
		    ip_table_mem[0] |= (offset << 16);
		    break;
		case 3:
		    ip_table_mem[0] &= 0xffff0000;
		    ip_table_mem[0] |= offset;
		    break;
	    }
	    rv = sbG2EplibPortEncapSet(ep, port, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: setting port(%d) for %s[u:%d] failed\n"), port,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } else {
		rv = 0;
	    }
	    break;

	case BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID:
	    oix = map->encap_id;
	    queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map->queue_id);
	    if (queue == 0xffffffff) {
		return BCM_E_PARAM;
	    }
	    rv = sbG2EplibHQosRemapGet(ep, oix, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		return(rv);
	    }
	    queue = ((queue >> 8) | ((queue & 0xFF) << 8));
	    switch (oix & 3) {
		case 0:
		    ip_table_mem[1] &= 0xffff;
		    ip_table_mem[1] |= (queue << 16);
		    break;
		case 1:
		    ip_table_mem[1] &= 0xffff0000;
		    ip_table_mem[1] |= queue;
		    break;
		case 2:
		    ip_table_mem[0] &= 0xffff;
		    ip_table_mem[0] |= (queue << 16);
		    break;
		case 3:
		    ip_table_mem[0] &= 0xffff0000;
		    ip_table_mem[0] |= queue;
		    break;
	    }
	    rv = sbG2EplibHQosRemapSet(ep, oix, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		return(rv);
	    }
	    break;

	default:
	    return BCM_E_PARAM;
    }
#endif
    return rv;
}

int
bcm_qe2000_cosq_subscriber_map_delete(int unit,
				      bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_FE2000_SUPPORT
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbG2EplibCtxt_st *ep;
    uint32 ip_table_mem[4];
    int oix,queue,vlan, port;
    int base_queue = 0, offset = 0;

    if (sbx == NULL) {
	return BCM_E_INIT;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
        LOG_CLI((BSL_META_U(unit,
                            "\n map_add not allowed in FIC mode\n")));
	return BCM_E_UNAVAIL;
    }

    ep = (sbG2EplibCtxt_st *) sbx->drv;

    if (map == NULL) {
	return BCM_E_PARAM;
    }

    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
    switch(map->flags) {
	case BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN:
	    base_queue = 0;
	    offset = 0;

	    vlan = map->vlan;
	    rv = sbG2EplibVlanRemapGet(ep, vlan, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: showing vlan(%d) for %s[u:%d] failed\n"), vlan,
                         SOC_CHIP_STRING(unit),unit));
	    }
	    switch (vlan & 3) {
		case 0:
		    ip_table_mem[1] &= 0xffff;
		    ip_table_mem[1] |= (base_queue << 16);
		    break;
		case 1:
		    ip_table_mem[1] &= 0xffff0000;
		    ip_table_mem[1] |= base_queue;
		    break;
		case 2:
		    ip_table_mem[0] &= 0xffff;
		    ip_table_mem[0] |= (base_queue << 16);
		    break;
		case 3:
		    ip_table_mem[0] &= 0xffff0000;
		    ip_table_mem[0] |= base_queue;
		    break;
	    }
	    rv = sbG2EplibVlanRemapSet(ep, vlan, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: setting vlan(%d) for %s[u:%d] failed\n"), vlan,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } else {
		rv = 0;
	    }

	    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
	    port = map->port;
	    rv = sbG2EplibPortEncapGet(ep, port, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: showing port(%d) for %s[u:%d] failed\n"), port,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } 
	    switch (port & 3) {
		case 0:
		    ip_table_mem[1] &= 0xffff;
		    ip_table_mem[1] |= (offset << 16);
		    break;
		case 1:
		    ip_table_mem[1] &= 0xffff0000;
		    ip_table_mem[1] |= offset;
		    break;
		case 2:
		    ip_table_mem[0] &= 0xffff;
		    ip_table_mem[0] |= (offset << 16);
		    break;
		case 3:
		    ip_table_mem[0] &= 0xffff0000;
		    ip_table_mem[0] |= offset;
		    break;
	    }
	    rv = sbG2EplibPortEncapSet(ep, port, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: setting port(%d) for %s[u:%d] failed\n"), port,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } else {
		rv = 0;
	    }
	    break;

	case BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID:
	    oix = map->encap_id;
	    queue = 0;
	    rv = sbG2EplibHQosRemapGet(ep, oix, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		return(rv);
	    }
	    switch (oix & 3) {
		case 0:
		    ip_table_mem[1] &= 0xffff;
		    ip_table_mem[1] |= (queue << 16);
		    break;
		case 1:
		    ip_table_mem[1] &= 0xffff0000;
		    ip_table_mem[1] |= queue;
		    break;
		case 2:
		    ip_table_mem[0] &= 0xffff;
		    ip_table_mem[0] |= (queue << 16);
		    break;
		case 3:
		    ip_table_mem[0] &= 0xffff0000;
		    ip_table_mem[0] |= queue;
		    break;
	    }
	    rv = sbG2EplibHQosRemapSet(ep, oix, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		return(rv);
	    }
	    break;

	default:
	    return BCM_E_PARAM;
    }
#endif
    return rv;
}

int
bcm_qe2000_cosq_subscriber_map_delete_all(int unit)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_FE2000_SUPPORT
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbG2EplibCtxt_st *ep;
    uint32 ip_table_mem[4];
    int port = 0, vlan = 0, oix = 0;
    uint32 nAddr, nBase, nEnd, nESize;
    
    if (sbx == NULL) {
	return BCM_E_INIT;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
        LOG_CLI((BSL_META_U(unit,
                            "\n map_add not allowed in FIC mode\n")));
	return BCM_E_UNAVAIL;
    }

    ep = (sbG2EplibCtxt_st *) sbx->drv;

    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
    nBase  = sbG2EplibGetSegBase(ep, SEG_VLAN_REMAP);
    nEnd   = sbG2EplibGetSegEnd(ep, SEG_VLAN_REMAP);
    nESize = sbG2EplibGetSegEntrySize(ep, SEG_VLAN_REMAP);
    nAddr = nBase;
    
    for (vlan = 0; nAddr <= nEnd; vlan++) {
	nAddr = nBase + nESize * vlan / 64;
	rv = sbG2EplibVlanRemapSet(ep, vlan, ip_table_mem);
	if (rv != SB_ELIB_OK) {
	    LOG_CLI((BSL_META_U(unit,
                                "ERROR: setting vlan(%d) for %s[u:%d] failed\n"), vlan,
                     SOC_CHIP_STRING(unit),unit));
	    rv = -1;
	} else {
	    rv = 0;
	}
    }
    
    nBase  = sbG2EplibGetSegBase(ep, SEG_PORT_REMAP);
    nEnd   = sbG2EplibGetSegEnd(ep, SEG_PORT_REMAP);
    nESize = sbG2EplibGetSegEntrySize(ep, SEG_PORT_REMAP);
    nAddr = nBase;
    
    for (port=0; nAddr <= nEnd; port++) {
	nAddr = nBase + nESize * port / 64;
	rv = sbG2EplibPortEncapSet(ep, port, ip_table_mem);
	if (rv != SB_ELIB_OK) {
	    LOG_CLI((BSL_META_U(unit,
                                "ERROR: setting port(%d) for %s[u:%d] failed\n"), port,
                     SOC_CHIP_STRING(unit),unit));
	    rv = -1;
	} else {
	    rv = 0;
	}
    }
    
    nBase  = sbG2EplibGetSegBase(ep, SEG_HQOS_REMAP);
    nEnd   = sbG2EplibGetSegEnd(ep, SEG_HQOS_REMAP);
    nESize = sbG2EplibGetSegEntrySize(ep, SEG_HQOS_REMAP);
    nAddr = nBase;
    
    for (oix = 0; nAddr <= nEnd; oix++) {
	nAddr = nBase + nESize * oix / 64;
	rv = sbG2EplibHQosRemapSet(ep, oix, ip_table_mem);
	if (rv != SB_ELIB_OK) {
	    return(rv);
	}
    }
#endif
    return rv;
}

int
bcm_qe2000_cosq_subscriber_map_get(int unit,
				   bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_FE2000_SUPPORT
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    sbG2EplibCtxt_st *ep;
    uint32 ip_table_mem[4];
    int oix,queue,vlan, port;
    int base_queue = 0, offset = 0;

    if (sbx == NULL) {
	return BCM_E_INIT;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
        LOG_CLI((BSL_META_U(unit,
                            "\n map_add not allowed in FIC mode\n")));
	return BCM_E_UNAVAIL;
    }

    ep = (sbG2EplibCtxt_st *) sbx->drv;

    if (map == NULL) {
	return BCM_E_PARAM;
    }

    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
    switch(map->flags) {
	case BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN:
	    vlan = map->vlan;
	    rv = sbG2EplibVlanRemapGet(ep, vlan, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: showing vlan(%d) for %s[u:%d] failed\n"), vlan,
                         SOC_CHIP_STRING(unit),unit));
	    }
	    switch (vlan & 3) {
		case 0:
		    base_queue = ip_table_mem[1] >> 16;
		    break;
		case 1:
		    base_queue = ip_table_mem[1] & 0xffff;
		    break;
		case 2:
		    base_queue = ip_table_mem[0] >> 16;
		    break;
		case 3:
		    base_queue = ip_table_mem[0] & 0xffff;
		    break;
	    }
	    base_queue = (((base_queue & 0xFF) << 8) | (base_queue >> 8));
	    map->encap_id = base_queue;

	    sal_memset(&ip_table_mem, 0, sizeof(ip_table_mem));
	    port = map->port;
	    rv = sbG2EplibPortEncapGet(ep, port, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		LOG_CLI((BSL_META_U(unit,
                                    "ERROR: showing port(%d) for %s[u:%d] failed\n"), port,
                         SOC_CHIP_STRING(unit),unit));
		rv = -1;
	    } 
	    switch (port & 3) {
		case 0:
		    offset = ip_table_mem[1] >> 16;
		    break;
		case 1:
		    offset = ip_table_mem[1] & 0xffff;
		    break;
		case 2:
		    offset = ip_table_mem[0] >> 16;
		    break;
		case 3:
		    offset = ip_table_mem[0] & 0xffff;
		    break;
	    }
	    offset = (((offset & 0xFF) << 8) | (offset >> 8));
	    BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(queue, (base_queue + offset));
	    map->queue_id = queue;
	    break;

	case BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID:
	    oix = map->encap_id;
	    rv = sbG2EplibHQosRemapGet(ep, oix, ip_table_mem);
	    if (rv != SB_ELIB_OK) {
		return(rv);
	    }
	    switch (oix & 3) {
		case 0:
		    offset = ip_table_mem[1] >> 16;
		    break;
		case 1:
		    offset = ip_table_mem[1] & 0xffff;
		    break;
		case 2:
		    offset = ip_table_mem[0] >> 16;
		    break;
		case 3:
		    offset = ip_table_mem[0] & 0xffff;
		    break;
	    }
	    offset = (((offset & 0xFF) << 8) | (offset >> 8));
	    BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(queue, offset);
	    map->queue_id = queue;
	    break;

	default:
	    return BCM_E_PARAM;
    }
#endif
    return rv;
}

int
bcm_qe2000_cosq_subscriber_traverse(int unit,
				    bcm_cosq_subscriber_map_traverse_cb cb,
				    void *user_data)
{
    int rv = BCM_E_UNAVAIL;
#ifdef BCM_FE2000_SUPPORT
    bcm_cosq_subscriber_map_t *p_map = NULL;
    int cmd = 0;
    bcm_sbx_subscriber_map_cb_params_t *p_cb_params = (bcm_sbx_subscriber_map_cb_params_t *) user_data;
    
    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC) {
	return BCM_E_UNAVAIL;
    }

    if (p_cb_params != NULL) {
	cmd = p_cb_params->cmd;
    } else {
	cmd = BCM_SBX_COSQ_SUBSCRIBER_MAP_GET; /* default */
    }

    p_map = sal_alloc(sizeof(bcm_cosq_subscriber_map_t), "subscriber map");
    if (p_map == NULL) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return(BCM_E_MEMORY);
    }

    if (cmd == BCM_SBX_COSQ_SUBSCRIBER_MAP_DELETE_ALL) {
	rv = cb(unit, 0, 0);
	if (BCM_FAILURE(rv)) {
	    LOG_CLI((BSL_META_U(unit,
                                "Retrieving Map failed (%s)\n"), bcm_errmsg(rv)));
        }
    } else {
	rv = cb(unit, p_map, user_data);
	if (BCM_FAILURE(rv)) {
	    LOG_CLI((BSL_META_U(unit,
                                "Retrieving Map failed (%s)\n"), bcm_errmsg(rv)));
        }
    }

    if (p_map != NULL) {
	sal_free(p_map);
    }
#endif
    return rv;
}

int
bcm_qe2000_cosq_gport_sched_config_set(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 flags)
{
    int rv = BCM_E_NONE;
    int queue_type, priority, priority2;
    uint32 data;


    rv = soc_sbx_sched_config_set_params_verify(unit, sched_mode, int_pri);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    rv = soc_sbx_sched_get_internal_state(unit, sched_mode, int_pri,
                                                        &queue_type, &priority, &priority2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    data = SAND_HAL_READ((sbhandle)unit, KA, QS_CONFIG5);
    if (flags & BCM_COSQ_SCHED_CONFIG_EXPEDITE) {
        data |= (((1 << queue_type)) << SAND_HAL_KA_QS_CONFIG5_QTYPE_EF_SHIFT);
        data |= (((1 << priority)) << SAND_HAL_KA_QS_CONFIG5_PRIORITY_EF_SHIFT);
    }
    else {
        data &= ~(((1 << queue_type)) << SAND_HAL_KA_QS_CONFIG5_QTYPE_EF_SHIFT);
        data &= ~(((1 << priority)) << SAND_HAL_KA_QS_CONFIG5_PRIORITY_EF_SHIFT);
    }
    SAND_HAL_WRITE(unit, KA, QS_CONFIG5, data);

    return(rv);
}

int
bcm_qe2000_cosq_gport_sched_config_get(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 *flags)
{
    int rv = BCM_E_NONE;
    int queue_type, priority, priority2;
    uint32 data;


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
    data = SAND_HAL_READ((sbhandle)unit, KA, QS_CONFIG5);
    if (data & (((1 << queue_type)) << SAND_HAL_KA_QS_CONFIG5_QTYPE_EF_SHIFT)) {
        (*flags) |= BCM_COSQ_SCHED_CONFIG_EXPEDITE;
    }

    return(rv);
}


int
_bcm_qe2000_cosq_queue_packet_length_adjust_get(int unit, int queue, int *adjust_p)
{
    int rv = BCM_E_NONE;
    uint32 data0 = 0, data1 = 0, data2 = 0, data3 = 0;


    /* Read entry */
    rv = soc_qe2000_qm_mem_read(unit, queue, 0x6, &data0, &data1, &data2, &data3);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    (*adjust_p) = (data0 & 0x3F);
    if ((data0 >> 6) && (0x1)) {
        (*adjust_p) = -(*adjust_p);
    }

    return(rv);
}

int
_bcm_qe2000_cosq_queue_packet_length_adjust_set(int unit, int queue, int adjust)
{
    int rv = BCM_E_NONE;
    uint32 data0 = 0, data1 = 0, data2 = 0, data3 = 0;


    /* consistency checks */
    if ((adjust <= -64)  || (adjust >= 64)) {
       return(BCM_E_PARAM);
    }

    data0 = (adjust < 0) ? -adjust: adjust;
    data0 = (data0 & 0x3F);

    if (adjust < 0) {
        data0 |= (1 << 6);
    }

    /* Write entry */
    rv = soc_qe2000_qm_mem_write(unit, queue, 0x6, data0, data1, data2, data3);

    return(rv);
}

static int
_bcm_qe2000_cosq_queue_burst_set(int unit, int base_queue, bcm_cos_queue_t cosq, int num_cos, int queue, int burst_rate_kbits, int is_burst_size_context)
{
    int rv = BCM_E_NONE, rc;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    uint32 shape_rate_kbits, per_queue_shape_rate_kbits;
    uint32 burst_rate_256_kbits;
    uint32 shape_max_burst_table_value;
    int32 cur_queue;


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    if (SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress == 1) { /* per queue shaping */
        if (burst_rate_kbits == -1) {
            /* determine burst size based on Shaping rate */
             shape_rate_kbits = p_qstate[queue].ingress.shape_limit_kbps;
             burst_rate_256_kbits = _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(unit, shape_rate_kbits);
             shape_max_burst_table_value = _bcm_qe2000_cosq_burst_get(unit, 1, burst_rate_256_kbits);
        }
        else {
            /* determine configuration based on application passed burst size */
             shape_max_burst_table_value = burst_rate_kbits * 1000;
        }

        if (shape_max_burst_table_value > SB_FAB_DEVICE_QE2000_INGRESS_SHAPER_MAX_BURST) {
            shape_max_burst_table_value = SB_FAB_DEVICE_QE2000_INGRESS_SHAPER_MAX_BURST;
        }

        rv = soc_qe2000_shape_maxburst_set(unit, queue, p_qstate[queue].ingress.enable_shaping, shape_max_burst_table_value);
        p_qstate[queue].ingress.shape_burst_kbps = burst_rate_kbits;
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Failed to configure Queue Burst Size, Queue: 0x%x\n"),
                       queue));
        }
    }
    else { /* queue group shaping */
        if (is_burst_size_context == TRUE) {
            if (cosq != 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Logical Queue Shaping Burst Size can be applied only to cos=0, cos=%d\n"),
                           cosq));
                rv = BCM_E_PARAM;
                return(rv);
            }
        }
      
        if (burst_rate_kbits == -1) {
             shape_rate_kbits = p_qstate[queue].ingress.shape_limit_kbps;
             per_queue_shape_rate_kbits = shape_rate_kbits;
             burst_rate_256_kbits = _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(unit, per_queue_shape_rate_kbits);
             shape_max_burst_table_value = _bcm_qe2000_cosq_burst_get(unit, num_cos, burst_rate_256_kbits);
        }
        else {
            /* determine configuration based on application passed burst size */
             shape_max_burst_table_value = burst_rate_kbits * 1000;
        }

        if (shape_max_burst_table_value > SB_FAB_DEVICE_QE2000_INGRESS_SHAPER_MAX_BURST) {
            shape_max_burst_table_value = SB_FAB_DEVICE_QE2000_INGRESS_SHAPER_MAX_BURST;
        }

        
        for (cur_queue = base_queue; cur_queue < (base_queue + num_cos); cur_queue++) {
            rc = soc_qe2000_shape_maxburst_set(unit, cur_queue, p_qstate[queue].ingress.enable_shaping, shape_max_burst_table_value);
            p_qstate[queue].ingress.shape_burst_kbps = burst_rate_kbits;

            if (rc != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: Failed to configure Queue Burst Size, Queue: 0x%x\n"),
                           cur_queue));
                rv = rc; 
            }
        }
    }

    return(rv);
}

/*
 * symantics of "get" returning the value configured via "set"
 */
static int
_bcm_qe2000_cosq_queue_burst_get(int unit, int base_queue, bcm_cos_queue_t cosq, int num_cos, int queue, int *burst_rate_kbits, int is_burst_size_context)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    uint32 shape_rate_kbits, per_queue_shape_rate_kbits;
    uint32 burst_rate_256_kbits;


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    if (SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress == 1) { /* per queue shaping */
        if (p_qstate[queue].ingress.shape_burst_kbps != -1) {
            (*burst_rate_kbits) = p_qstate[queue].ingress.shape_burst_kbps;
        }
        else {
            shape_rate_kbits = p_qstate[queue].ingress.shape_limit_kbps;
            burst_rate_256_kbits = _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(unit, shape_rate_kbits);
            *(burst_rate_kbits) = (_bcm_qe2000_cosq_burst_get(unit, 1, burst_rate_256_kbits)) / 1000;
        }
    }
    else { /* queue group shaping */
        if (p_qstate[queue].ingress.shape_burst_kbps != -1) {
            (*burst_rate_kbits) = p_qstate[queue].ingress.shape_burst_kbps;
        }
        else {
            shape_rate_kbits = p_qstate[queue].ingress.shape_limit_kbps;
            per_queue_shape_rate_kbits = shape_rate_kbits;
            burst_rate_256_kbits = _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(unit, per_queue_shape_rate_kbits);
            *(burst_rate_kbits) = (_bcm_qe2000_cosq_burst_get(unit, num_cos, burst_rate_256_kbits)) / 1000;
        }
    }
    return(rv);
}

static INLINE uint32 _bcm_qe2000_cosq_shape_burst_rate_256kbits_get(int unit, uint32 per_queue_shape_burst_kbps)
{
    return(((per_queue_shape_burst_kbps+1)*1024)/(1000000/256)); /* D-Mode 16k queues */
}

static INLINE uint32 _bcm_qe2000_cosq_burst_get(int unit, int num_cos, uint32 shape_rate_256_kbits)
{
    return(shape_rate_256_kbits * SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress);
}

static INLINE uint32 _bcm_qe2000_cosq_queuegroup_to_queue_shape_burst_get(int unit, int num_cos, uint32 queuegroup_shape_burst_kbits)
{
    
    return((queuegroup_shape_burst_kbits + num_cos) / num_cos);
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_qe2000_wb_cosq_state_sync(int unit, uint8 **pptr, int flag)
{
    int   rv = BCM_E_NONE, scache_len = 0;
    uint8 *ptr;

    /* allocate space for structures */
    if (flag == 2) {
	/* min buffers for queue */
	scache_len += (sizeof(int32) * HW_QE2000_MAX_QUEUES);
	
	/* virtual node port */
	scache_len += (sizeof(virtual_node_port_t) * BCM_SBX_DEVICE_COSQ_MAX_LNAS);
	
#ifdef TME_ARB_PORT_OPTIMIZE
	/* level two arb group */
	if (level_two_arb_group[unit] != NULL) {
	    scache_len += (sizeof(arb_group_t) * BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX);
	}
#endif
	return scache_len;
    }

    ptr = *pptr;

    if (flag == FALSE) {
	/* min buffers for queue */
	sal_memcpy(&min_buffers_for_queue[unit], (((uint8 *)ptr)), (sizeof(int32) * HW_QE2000_MAX_QUEUES));
	ptr += (sizeof(int32) * HW_QE2000_MAX_QUEUES);
	
	/* virtual node port */
	sal_memcpy(&virtual_node_port[unit], (((uint8 *)ptr)), 
		   (sizeof(virtual_node_port_t) * BCM_SBX_DEVICE_COSQ_MAX_LNAS));
	ptr += (sizeof(virtual_node_port_t) * BCM_SBX_DEVICE_COSQ_MAX_LNAS);

#ifdef TME_ARB_PORT_OPTIMIZE
	/* level two arb group */
	if (level_two_arb_group[unit] != NULL) {
        /* coverity[overrun-buffer-arg : FALSE] */
	    sal_memcpy(&level_two_arb_group[unit], (((uint8 *)ptr)), 
		       (sizeof(arb_group_t) * BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX));
	    ptr += (sizeof(arb_group_t) * BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX);
	}
#endif
    } else if (flag == TRUE) {
	/* min buffers for queue */
	sal_memcpy((((uint8 *)ptr)), &min_buffers_for_queue[unit], (sizeof(int32) * HW_QE2000_MAX_QUEUES));
	ptr += (sizeof(int32) * HW_QE2000_MAX_QUEUES);
	
	/* virtual node port */
	sal_memcpy((((uint8 *)ptr)), &virtual_node_port[unit], 
		   (sizeof(virtual_node_port_t) * BCM_SBX_DEVICE_COSQ_MAX_LNAS));
	ptr += (sizeof(virtual_node_port_t) * BCM_SBX_DEVICE_COSQ_MAX_LNAS);

#ifdef TME_ARB_PORT_OPTIMIZE
	/* level two arb group */
	if (level_two_arb_group[unit] != NULL) {
	    sal_memcpy((((uint8 *)ptr)), &level_two_arb_group[unit],
		       (sizeof(arb_group_t) * BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX));
	    ptr += (sizeof(arb_group_t) * BCM_INT_SBX_ARB_GROUP_LEVEL_TWO_MAX);
	}
#endif
    } else {
	return BCM_E_PARAM;
    }

    *pptr = ptr;
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */
