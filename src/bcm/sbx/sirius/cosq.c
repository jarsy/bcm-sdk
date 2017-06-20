/*
 * $Id: cosq.c,v 1.449 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 *
 * Sirius Specific implementation
 *
 */

#include <shared/bsl.h>

#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbFabCommon.h>
#include <soc/sbx/sirius.h>
#include <soc/sbx/sirius_counter.h>
#include <soc/sbx/fabric/sbZfFabWredParameters.hx>

#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/device_wred.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/stack.h>

#include <bcm_int/sbx/sirius/multicast.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx_dispatch.h>
#include <soc/sbx/bm9600.h>


int _bcm_sirius_cosq_add_delete_queue(int unit, int add, int32 queue,
                                      bcm_sbx_cosq_queue_region_type_t queue_region,
                                      bcm_sbx_cosq_bw_group_params_t *p_bwparams,
                                      bcm_sbx_cosq_queue_params_ingress_t *p_qparams);

static void
_bcm_sirius_rate_delta_max_index_for_port_rate_chosen(int unit, uint32 bag_rate_kbps,
                                                      int32 *prate_delta_max_index);

static int
_bcm_sirius_cosq_select_queue_params(int unit, int queue, int queue_region,
				     bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
				     int32 *p_hold_ts, int32 *p_qtype);

static int
_bcm_sirius_cosq_ingress_flat_scheduler_bucket_select(int unit, int level,
						      int node, int *bucket);
static int bcm_sirius_fc_init(int unit);
static int bcm_sirius_fc_deinit(int unit);
static int bcm_sirius_fc_state_clear(int unit, int port,
                      bcm_sbx_cosq_egress_flow_control_state_t *cur_fc_map_state,
                      bcm_sbx_cosq_egress_flow_control_state_t *prev_fc_map_state,
                      int force_all);
static int bcm_sirius_cosq_ingress_scheduler_node_profile_get(int unit, int level, int node,
					                          int mode, int *node_profile);
static int _bcm_sirius_cosq_scheduler_adopt_all_priority_set(int unit, bcm_gport_t gport,
			    bcm_cos_queue_t cosq, bcm_cosq_control_t type, int value,
                            bcm_sbx_cosq_control_t *control_info);
static int _bcm_sirius_cosq_scheduler_adopt_all_priority_get(int unit, bcm_gport_t gport,
			    bcm_cos_queue_t cosq, bcm_cosq_control_t type, int *value,
                            bcm_sbx_cosq_control_t *control_info);

static int _bcm_sirius_cosq_p2l_scheduler(int unit,
					  int egress,
					  int level,
					  int node,
					  int *logical_scheduler);

/* physical scheduler state */
bcm_sbx_sirius_ingress_scheduler_state_t *is_state[SOC_MAX_NUM_DEVICES][SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS];
bcm_sbx_sirius_egress_scheduler_state_t  *es_state[SOC_MAX_NUM_DEVICES][SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS];

#define SIRIUS_INGRESS_SCHEDULER_RESERVED (2)

#define BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT(unit, level, node) \
    is_state[unit][level][node].in_use = FALSE;                    \
    is_state[unit][level][node].parent = -1;                       \
    is_state[unit][level][node].parent_gport = BCM_GPORT_INVALID;  \
    is_state[unit][level][node].first_child = -1;                  \
    is_state[unit][level][node].num_child = 0;                     \
    is_state[unit][level][node].logical_scheduler = -1;            \
    is_state[unit][level][node].scheduler_mode = BCM_COSQ_AF;      \
    is_state[unit][level][node].scheduler_param = 0;               \
    is_state[unit][level][node].min_shaper_rate_kbps = 0;          \
    is_state[unit][level][node].min_shaper_threshold_bits = 0;     \
    is_state[unit][level][node].max_shaper_rate_kbps = 0;          \
    is_state[unit][level][node].max_shaper_threshold_bits = 0;     \
    is_state[unit][level][node].bucket = node;                     \
    is_state[unit][level][node].adopt_all= FALSE;

#define BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(is_p) \
    if (is_p != NULL) {                                   \
	(is_p)->in_use = FALSE;				  \
	(is_p)->parent = -1;				  \
	(is_p)->parent_gport = BCM_GPORT_INVALID;	  \
	(is_p)->first_child = -1;			  \
	(is_p)->num_child = 0;				  \
	(is_p)->logical_scheduler = -1;			  \
	(is_p)->scheduler_mode = BCM_COSQ_AF0;		  \
	(is_p)->scheduler_param = 0;			  \
	(is_p)->min_shaper_rate_kbps = 0;		  \
	(is_p)->min_shaper_threshold_bits = 0;		  \
	(is_p)->max_shaper_rate_kbps = 0;		  \
	(is_p)->max_shaper_threshold_bits = 0;		  \
	(is_p)->adopt_all = FALSE;		          \
    }

#define BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT(unit, level, node) \
    es_state[unit][level][node].in_use = FALSE;                   \
    es_state[unit][level][node].intf = -1;                        \
    es_state[unit][level][node].channel = -1;                     \
    es_state[unit][level][node].subport = -1;                     \
    es_state[unit][level][node].num_child = 0;                    \
    es_state[unit][level][node].index = -1;                       \
    es_state[unit][level][node].scheduler_mode = SIRIUS_ES_SCHEDULER_MODE_WERR; \
    es_state[unit][level][node].scheduler_param = 0;              \
    es_state[unit][level][node].min_shaper_rate_kbps = 0;         \
    es_state[unit][level][node].min_shaper_threshold_bits = 0;    \
    es_state[unit][level][node].max_shaper_rate_kbps = 0;         \
    es_state[unit][level][node].max_shaper_threshold_bits = 0;

#define BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT_PTR(es_p) \
    if (es_p != NULL) {                                         \
	(es_p)->in_use = FALSE;					\
	(es_p)->intf = -1;					\
	(es_p)->channel = -1;					\
	(es_p)->subport = -1;					\
	(es_p)->num_child = 0;					\
	(es_p)->index = -1;					\
	(es_p)->scheduler_mode = SIRIUS_ES_SCHEDULER_MODE_WERR; \
	(es_p)->scheduler_param = 0;				\
	(es_p)->min_shaper_rate_kbps = 0;			\
	(es_p)->min_shaper_threshold_bits = 0;			\
	(es_p)->max_shaper_rate_kbps = 0;			\
	(es_p)->max_shaper_threshold_bits = 0;			\
    }

#define BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit) \
    if ((SOC_SBX_STATE(unit)->egress_scheduler_state == NULL) ||         \
        ((SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_FIC) &&         \
         (SOC_SBX_STATE(unit)->ingress_scheduler_state == NULL)) ||      \
	(gs_state[unit] == NULL)) {					 \
        LOG_ERROR(BSL_LS_BCM_COMMON, \
                  (BSL_META_U(unit, \
                              "scheduler states not initialized, unit(%d)\n"), \
                   unit)); \
	return BCM_E_INIT;					         \
    }

typedef struct bcm_sbx_sirius_egress_group_shaper_state_s {
    sbBool_t in_use;        /* TRUE/FALSE */
    int      max_shaper_rate_kbps;
    int      max_shaper_threshold_bits;
}bcm_sbx_sirius_egress_group_shaper_state_t;

/* physical scheduler state */
bcm_sbx_sirius_egress_group_shaper_state_t *gs_state[SOC_MAX_NUM_DEVICES];
bcm_sbx_cosq_egress_flow_control_state_t *fc_map_state[SOC_MAX_NUM_DEVICES];
bcm_sbx_sirius_fc_state                  *fc_state[SOC_MAX_NUM_DEVICES];
bcm_sbx_cosq_egress_flow_control_state_t *mc_fc_map_state[SOC_MAX_NUM_DEVICES];


int bcm_sirius_fc_reserved_fifoset_get(int unit, int *base_fifo, int *size);
static int
bcm_sirius_fc_reserved_fifoset_set(int unit, bcm_sbx_sirius_egress_scheduler_state_t *p_esstate,
				   int base_fifo, int size);

static int
bcm_sirius_cosq_ingress_scheduler_shift(int unit, int level, int node, int direction);

static int
_bcm_sirius_cosq_map_sp2gsp(int unit, int queue, int32 *p_bw_mode, int32 *p_sp_priority);

static int
_bcm_sirius_cosq_update_gsp(int unit, int queue, int q_type, int bw_mode);

static int
_bcm_sirius_cosq_ingress_shaper_burst_set(int unit, int queue, int burst_size_kbits);

static int
_bcm_sirius_cosq_ingress_shaper_burst_get(int unit, int queue, int *burst_size_kbits);

static int
_bcm_sirius_cosq_qsel_offset_get(int unit, bcm_gport_t gport, int *value);

static int
_bcm_sirius_cosq_qsel_offset_set(int unit, bcm_gport_t gport, int value);

static int
_bcm_sirius_cosq_destmod_destport_get(int unit, int queue,
                         bcm_gport_t gport, int32 *p_dest_switch_modid, int32 *p_dest_switch_port);

int
bcm_sirius_cosq_init(int unit)
{
    int rv = BCM_E_UNAVAIL;
    int create_ports;

    create_ports = SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, IF_SUBPORTS_CREATE);

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        /* Certain blocks are not fully inited in this case, can not write to memory in
	 * those blocks, skip trunk BCM init. This is here for emulator and bringup
	 * could be cleaned up after sirius pass bringup stage.
	 */
        return BCM_E_NONE;
    }

    rv = bcm_sirius_fc_init(unit);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    rv = bcm_sirius_scheduler_init(unit, create_ports);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    return rv;

err:

    bcm_sirius_fc_deinit(unit);
    return rv;
}

static int
_bcm_sirius_cosq_delete_sysport(int unit, uint32 sysport)
{
    int result;
    eg_fd_gmt_entry_t gmtEntry;
    uint32 mgid;
    uint32 tgtId = ~0;

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        /* Certain blocks are not fully inited in this case, can not write to memory in
	 * those blocks, skip trunk BCM init. This is here for emulator and bringup
	 * could be cleaned up after sirius pass bringup stage.
	 */
        return BCM_E_NONE;
    }

    /* get the sysport's pointer */
    mgid = sysport + SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET;
    result = READ_EG_FD_GMTm(unit, MEM_BLOCK_ANY, mgid, &gmtEntry);
    if (BCM_E_NONE == result) {
        tgtId = soc_mem_field32_get(unit, EG_FD_GMTm, &gmtEntry, MVRPf);
    } else {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "unable to read %d:MDB[%08X]: %d (%s)\n"),
                   unit,
                   mgid,
                   result,
                   _SHR_ERRMSG(result)));
    }

    /* get rid of multicast/distribution group */
    if ((BCM_E_NONE == result) &&
        (tgtId > SOC_MEM_INFO(unit, EG_FD_MDBm).index_min) &&
        (tgtId <= SOC_MEM_INFO(unit, EG_FD_MDBm).index_max)) {
        /*
         *  We could read the sysport's pointer, and it points into the valid
         *  MDB space, so it's either a multicast or distribution group.
         *
         *  Need to destroy this thing using the multicast APIs.
         */
        result = bcm_sirius_multicast_destroy(unit, mgid);
        if ((BCM_E_NONE != result) && (BCM_E_INIT != result)) {
            /* something went wrong with multicast delete */
            /*
             *  We ignore BCM_E_INIT here, since it is possible that we are
             *  destroying this after multicast has been detached.
             */
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "unable to delete unit %d multicast/distribution group"
                                   " for sysport %d at %08X: %d (%s)\n"),
                       unit,
                       sysport,
                       tgtId,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } /* if (successfuly read sysport pointer and it's in mulitcast space) */

    /* get rid of unicast */
    if ((BCM_E_NONE != result) ||
        (tgtId <= SOC_MEM_INFO(unit, EG_FD_MDBm).index_min) ||
        (tgtId > SOC_MEM_INFO(unit, EG_FD_MDBm).index_max)) {
        /*
         *  Could not read sysport's pointer -or- sysport's pointer is not
         *  pointing to a multicast group -or- was unable to destroy the
         *  multicast group; just clobber the pointer.
         */
        
        result = soc_sirius_fd_unicast_gmt_set(unit, sysport, 0);
        if (SOC_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "unable to write unit %d sysport %d pointer: %d (%s)\n"),
                       unit,
                       sysport,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } /* if (BCM_E_NONE == rv) */

    return result;
}

int
bcm_sirius_cosq_detach(int unit)
{
    int result = BCM_E_NONE;
    unsigned int curGroup;
    unsigned int minGroup;
    unsigned int maxGroup;
    unsigned int index;

    
    /*
     *  We need to clean up the distribution groups that were created from
     *  within the bcm_sirius_cosq_create_egress_internal_port function.
     */
    maxGroup = SOC_MEM_INFO(unit, EG_FD_GMTm).index_max;
    minGroup = maxGroup - 4095; 

    /* scale so it's sysport ID */
    maxGroup -= minGroup;
    minGroup = 0;

    /* clear the sysports (and release any distribution groups */
    for (curGroup = minGroup;
         (curGroup <= maxGroup) && (BCM_E_NONE == result);
         curGroup++) {
        result = _bcm_sirius_cosq_delete_sysport(unit, curGroup);
    }
    for (index = 0; index < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; index++) {
        if (is_state[unit][index]) {
            sal_free(is_state[unit][index]);
            is_state[unit][index] = NULL;
        }
    }
    if (es_state[unit][0]) {
        sal_free(es_state[unit][0]);
        for (index = 0; index < SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS; index++) {
            es_state[unit][index] = NULL;
        }
    }
    if (fc_map_state[unit]) {
        sal_free(fc_map_state[unit]);
        fc_map_state[unit] = NULL;
    }
    if (fc_state[unit]) {
        sal_free(fc_state[unit]);
        fc_state[unit] = NULL;
    }
    if (mc_fc_map_state[unit]) {
        sal_free(mc_fc_map_state[unit]);
        mc_fc_map_state[unit] = NULL;
    }
    if (gs_state[unit]) {
        sal_free(gs_state[unit]);
        gs_state[unit] = NULL;
    }

    return result;
}


int
bcm_sirius_scheduler_init(int unit, int create_ports)
{
    int rv = BCM_E_NONE;
    int is, es, gs;
    int level, node, num_level, num_node;
    bcm_sbx_sirius_egress_scheduler_state_t *p_esstate = NULL;
    bcm_sbx_sirius_egress_group_shaper_state_t *p_gsstate = NULL;
    int modid, port, fifo, hg, intf, intf_gport, subport, subports, flags, port_offset, intf_subports;
    int level7_node, level6_node;
    int profile;
    int creditor_state_map, adopt_pri_map, pri2, pri1, pri0, mult2, mult1;
    int child_level;
    uint16 dev_id;
    uint8 rev_id;

    /* init ingress scheduler state */
    for (level = 0; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
	if (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] > 0) {
	    if (is_state[unit][level] == NULL) {
		is_state[unit][level] = sal_alloc(sizeof(bcm_sbx_sirius_ingress_scheduler_state_t) * SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level],
						  "ingress scheduler state memory");
		if (is_state[unit][level] == NULL) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
		               FUNCTION_NAME(), unit));
		    return(BCM_E_MEMORY);
		}
	    }
	    
	    for (is = 0; is < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; is++) {
		BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT(unit, level, is);
	    }
	} else {
	    is_state[unit][level] = NULL;
	}
    }

    /* init egress schedulers state */
    if (es_state[unit][0] == NULL) {
	p_esstate = sal_alloc(sizeof(bcm_sbx_sirius_egress_scheduler_state_t) * SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER,
			      "egress scheduler state memory");

	if (p_esstate == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}

	es_state[unit][0] = p_esstate;
	es_state[unit][1] = es_state[unit][0] + SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0;
	es_state[unit][2] = es_state[unit][1] + SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
	es_state[unit][3] = es_state[unit][2] + SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
	es_state[unit][4] = es_state[unit][3] + SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3;
    } else {
	p_esstate = es_state[unit][0];
    }

    for(es = 0; es < SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER; es++) {
	BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT_PTR(p_esstate + es);
    }

    /* init egress group shaper state */
    if (gs_state[unit] == NULL) {
	p_gsstate = sal_alloc(sizeof(bcm_sbx_sirius_egress_group_shaper_state_t) * SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER,
			      "egress group shaper state memory");

	if (p_gsstate == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}
	gs_state[unit] = p_gsstate;
    } else {
	p_gsstate = gs_state[unit];
    }

    for(gs = 0; gs < SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER; gs++) {
	(p_gsstate + gs)->in_use = FALSE;
	(p_gsstate + gs)->max_shaper_rate_kbps = 0;
	(p_gsstate + gs)->max_shaper_threshold_bits = 0;
    }


    /* init Node profiles in all TS levels */
    for (level = 1; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
	if (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] > 0) {
	    /* starve and hungry mapped to hungry, satisfy to satisfy */
	    mult1 = 0xFFFF * SIRIUS_TS_NODE_PROFILE_MULTI1_PERCENT / 100;
	    mult2 = 0xFFFF * SIRIUS_TS_NODE_PROFILE_MULTI2_PERCENT / 100;
	    for (profile = 0; profile <= SIRIUS_TS_NODE_PROFILE_TEMPLATE_MAX; profile++) {
		creditor_state_map = 0x4;
                if (SOC_SBX_CFG(unit)->node_template_id == SOC_SBX_NODE_QOS_TEMPLATE_TYPE0) {
		    switch (profile) {
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT:
			    adopt_pri_map = 0xFFFF;			
			    pri2 = 0;
			    pri1 = 0;
			    pri0 = 0;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_DISABLED:
			    adopt_pri_map = 0x0000;
			    pri2 = 0;
			    pri1 = 0;
			    pri0 = 0;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF0:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 3;
			    pri0 = 7;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF1:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 4;
			    pri0 = 8;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF2:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 5;
			    pri0 = 9;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF3:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 6;
			    pri0 = 10;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP0:  
			    adopt_pri_map = 0x4003;
			    pri2 = 12;
			    pri1 = 12;
			    pri0 = 12;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP1:  
			    adopt_pri_map = 0x4003;
			    pri2 = 13;
			    pri1 = 13;
			    pri0 = 13;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_BE:  
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 2;
			    pri0 = 2;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 12;
			    pri0 = 12;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 13;
			    pri0 = 13;
			    break;
			case SIRIUS_TS_NODE_PROFILE_TEMPLATE_MAX:
		        default:
			    creditor_state_map = 0x0;
			    adopt_pri_map = 0x0000;
			    pri2 = 0;
			    pri1 = 0;
			    pri0 = 0;
			    break;
		    }
	        }
                else if (SOC_SBX_CFG(unit)->node_template_id == SOC_SBX_NODE_QOS_TEMPLATE_TYPE1) {
		    switch (profile) {
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT:
			    adopt_pri_map = 0xFFFF;			
			    pri2 = 0;
			    pri1 = 0;
			    pri0 = 0;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_DISABLED:
			    adopt_pri_map = 0x0000;
			    pri2 = 0;
			    pri1 = 0;
			    pri0 = 0;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF0:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 3;
			    pri0 = 7;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP0:  
			    adopt_pri_map = 0x4003;
			    pri2 = 6;
			    pri1 = 6;
			    pri0 = 6;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP1:  
			    adopt_pri_map = 0x4003;
			    pri2 = 7;
			    pri1 = 7;
			    pri0 = 7;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP2:  
			    adopt_pri_map = 0x4003;
			    pri2 = 8;
			    pri1 = 8;
			    pri0 = 8;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP3:  
			    adopt_pri_map = 0x4003;
			    pri2 = 9;
			    pri1 = 9;
			    pri0 = 9;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP4:  
			    adopt_pri_map = 0x4003;
			    pri2 = 10;
			    pri1 = 10;
			    pri0 = 10;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP5:  
			    adopt_pri_map = 0x4003;
			    pri2 = 11;
			    pri1 = 11;
			    pri0 = 11;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP6:  
			    adopt_pri_map = 0x4003;
			    pri2 = 12;
			    pri1 = 12;
			    pri0 = 12;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 6;
			    pri0 = 6;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1:
			    adopt_pri_map = 0x4003;
			    pri2 = 2;
			    pri1 = 7;
			    pri0 = 7;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP2:
			    adopt_pri_map = 0x4003;
			    pri2 = 3;
			    pri1 = 8;
			    pri0 = 8;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP3:
			    adopt_pri_map = 0x4003;
			    pri2 = 3;
			    pri1 = 9;
			    pri0 = 9;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP4:
			    adopt_pri_map = 0x4003;
			    pri2 = 4;
			    pri1 = 10;
			    pri0 = 10;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP5:
			    adopt_pri_map = 0x4003;
			    pri2 = 4;
			    pri1 = 11;
			    pri0 = 11;
			    break;
		        case SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP6:
			    adopt_pri_map = 0x4003;
			    pri2 = 5;
			    pri1 = 12;
			    pri0 = 12;
			    break;
			case SIRIUS_TS_NODE_PROFILE_TEMPLATE_MAX:
		        default:
			    creditor_state_map = 0x0;
			    adopt_pri_map = 0x0000;
			    pri2 = 0;
			    pri1 = 0;
			    pri0 = 0;
			    break;
		    }
                }
                else {
	             LOG_ERROR(BSL_LS_BCM_COSQ,
	                       (BSL_META_U(unit,
	                                   "ERROR: %s, node template not supported %d, Unit(%d)\n"), 
	                        FUNCTION_NAME(), SOC_SBX_CFG(unit)->node_template_id, unit));
	             return(BCM_E_PARAM);
                }
                BCM_IF_ERROR_RETURN
                    (soc_sirius_ts_node_profile_config(unit, level, profile, creditor_state_map,
                                                 adopt_pri_map, pri2, pri1, pri0, mult2, mult1));
            }
	}    
    }

    /* Reserve one FIFO set for mapping unused ports in E2ECC message   */
    rv = bcm_sirius_fc_reserved_fifoset_set(unit, p_esstate, -1,
                                               BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* Reserve one egress group shaper for multicast trunking */
    gs = SB_FAB_DEVICE_SIRIUS_MC_EGRESS_GROUP_SHAPER;
    rv = bcm_sirius_cosq_group_shaper_allocate(unit, TRUE, &gs);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: failed to allocate group max shaper for multicast trunking child gport\n")));
    }

    /* create the ingress schedulers for interface ports */
    for (intf = 0; intf < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[7]; intf++) {
	rv = bcm_sirius_cosq_scheduler_allocate(unit, 7, intf, 1, 
						FALSE, &level7_node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to create scheduler for intf %d, Unit(%d)\n"), 
	               FUNCTION_NAME(), intf, unit));
	    return BCM_E_RESOURCE;
	}
	
	/* parent is root, no child are connected */
	BCM_IF_ERROR_RETURN(soc_sirius_ts_node_hierachy_config(unit, 7, level7_node, 
							       0, 0, 0));
    }

    /* init all ES physical schedulers */
    num_level = 5;
    for (level = 0; level < num_level; level++) {
	num_node = SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level];
	for (node = 0; node < num_node; node++) {
	    rv = soc_sirius_scheduler_init(unit, level, node, TRUE);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ES failed to init level %d node %d\n"),
		           level, node));
		return rv;
	    }
	}
    }

    /* create the egress schedulers for interface ports */
    for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3; intf++) {
	rv = bcm_sirius_cosq_scheduler_allocate(unit, SIRIUS_ES_LEVEL_INTERFACE, 
						intf, 1, TRUE, &node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to allocate level 3 egress scheduler node, Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_RESOURCE;
	}
    }

    if ( 
	  (  (SOC_SBX_CFG(unit)->bHybridMode) ||
	     (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
	     (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)
	  )   &&
	  (SOC_SBX_CFG_SIRIUS(unit)->bDualLocalGrants) ) {
	/* Init default TS hierachy for NULL grant management, By default, level 7 node 0
	 * will be on plane A and reserve node 48 on level 6 and (last node - 1) on each 
	 * level below (L5-L1), this will make sure NULL grant from plane A always land on
	 * (last node - 1), these nodes will not be visible to user and can not be 
	 * disconnect or freed. This means level 7 node 0 (reserved for CPU) can only have
	 * 7 level 6 nodes visible to user.
	 * 
	 * We also reserve the last node of each level for NULL grant on plane B
	 *
	 * All other nodes are init to have first node point to last node on next level
	 * since we fix level 7 node 1 to be on plane B, this will make sure NULL grant
	 * from plane B always land on last node on each level. This will guarrantee that
	 * when user has not hooked up any tree down to Level 1. NULL grant from plane A
	 * and plane B will not land on same node.
	 *
	 * When user try to hooks up TS hierachy, if user hooks up hierachy rooted on 
	 * level 7 node 1, this will make sure NULL grant from plane B goes to some 
	 * node other than node 0 (since node 0 is already reserved), otherwise, NULL grant
	 * will always to go to last node of each level since all 
	 */

        soc_cm_get_id(unit, &dev_id, &rev_id);

	/* reserve level 6 - level 1 last 2 nodes, no need to connect  */
	for (level = 6; level >= 1; level--) {
            if (((dev_id == BCM56931_DEVICE_ID) || (dev_id == BCM56936_DEVICE_ID)) &&
		((level >= 2) && (level <= 3))) {
		    continue;
	    }

	    node = SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]-1;
	    rv = bcm_sirius_cosq_scheduler_allocate(unit, level, node, 1, FALSE, &node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to allocation child level %d scheduler node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, unit));
		return BCM_E_RESOURCE;
	    }

	    node = SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]-2;
	    rv = bcm_sirius_cosq_scheduler_allocate(unit, level, node, 1, FALSE, &node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to allocation child level %d scheduler node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, unit));
		return BCM_E_RESOURCE;
	    }
	}

	/* init L2-L7 schedulers to all point their first child to the last node of lower level*/
	for (level = 6; level >= 2; level--) {
            if ((dev_id == BCM56931_DEVICE_ID) || (dev_id == BCM56936_DEVICE_ID)) {	    
		if (level > 4) {
		    child_level = level - 1;
		} else if (level == 4) {
		    child_level = 1;
		} else {
		    continue;
		}
	    } else {
		child_level = level - 1;
	    }

	    /* point the (last_node - 1) to (last_node - 1) of child level.
	     * point all other nodes to last node of child level
	     */
	    for (node = 0; node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; node++) {
		if (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] < 3) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, TS level %d has less than 3 scheduler node, Unit(%d)\n"),
		               FUNCTION_NAME(), level, unit));
		    return BCM_E_CONFIG;
		}

		if (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level] < 3) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, TS level %d has less than 3 scheduler node, Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, unit));
		    return BCM_E_CONFIG;
		}

		rv = soc_sirius_scheduler_init(unit, level, node, FALSE);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to reinit ingress scheduler level %d node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, unit));
		    return rv;
		}

		if (node == (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] - 2)) {
		    rv = soc_sirius_ts_node_hierachy_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level]-2,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE);
		} else {
		    rv = soc_sirius_ts_node_hierachy_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level]-1,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE);
		}
		
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Failed to init TS level %d node %d first child node, Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, unit));		    
		    return rv;
		}
	    }

	    /* force last 2 l1 nodes's children all use sysport 0x1FF, cover both 8 child
	     * and 4 child cases in case user switch the mode using APIs later
	     */
	    for (node = SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1]*8-16; 
		 node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1]*8;
		 node++) {
		rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, node, 0x1FF);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Failed to init TS leaf level node %d destination, Unit(%d)\n"),
		               FUNCTION_NAME(), node, unit));		    
		    return rv;
		}
	    }

	    for (node = SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1]*4-16; 
		 node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1]*4;
		 node++) {
		rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, node, 0x1FF);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Failed to init TS leaf level node %d destination, Unit(%d)\n"),
		               FUNCTION_NAME(), node, unit));		    
		    return rv;
		}
	    }
	}
	
	/* connect level 6 node 48 to level 7 node 0 as the path for plane A NULL grants */
	level6_node = 48;
	level7_node = 0;
	rv = bcm_sirius_cosq_scheduler_allocate(unit, 6, level6_node, 1, FALSE, &node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to create level 6 ingress scheduler node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level6_node, unit));
	    return BCM_E_RESOURCE;
	}
	
	rv = bcm_sirius_cosq_scheduler_connect(unit, 7, level7_node, 6, level6_node, -1, FALSE, -1);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to connect level 6 ingress scheduler node %d,  Unit(%d)\n"),
	               FUNCTION_NAME(), level6_node, unit));
	    return BCM_E_RESOURCE;
	}

	/* point the level 6 node 48 first child to be (last_node - 1) on level 5 */
	rv = soc_sirius_ts_node_hierachy_config(unit, 6, level6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[5]-2,
						SOC_SIRIUS_API_PARAM_NO_CHANGE);

	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Failed to init TS level %d node %d first child node, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));		    
	    return rv;
	}

	/* connect level 6 node 0 to level 7 node 1 as the default path for plane B NULL grants
	 * in case of SDK create_ports==1, this default path will be override by the 
	 * bcm_sirius_fabric_port_create function for optimization (might only waste a level 5 node)
	 */
	level6_node = 0;
	level7_node = 1;
	rv = bcm_sirius_cosq_scheduler_allocate(unit, 6, level6_node, 1, FALSE, &node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to create level 6 ingress scheduler node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level6_node, unit));
	    return BCM_E_RESOURCE;
	}
	
	rv = bcm_sirius_cosq_scheduler_connect(unit, 7, level7_node, 6, level6_node, -1, FALSE, -1);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to connect level 6 ingress scheduler node %d,  Unit(%d)\n"),
	               FUNCTION_NAME(), level6_node, unit));
	    return BCM_E_RESOURCE;
	}	
    }


    /* create subport scheduler if told so */
    if (create_ports && !SOC_WARM_BOOT(unit)) {
	flags = 0;  /* create regular subports */
	rv = bcm_sirius_stk_modid_get(unit, &modid);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "%s, failed to get modid, Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return rv;
	}	

	subport = subports = 0;
	PBMP_ALL_ITER(unit, port) {
	    /* go through all line size higig ports and cpu port */
	  if ( IS_HG_PORT(unit, port) || IS_CPU_PORT(unit, port) || IS_REQ_PORT(unit, port) ) {
		if (IS_HG_PORT(unit, port) && 
		    ((SOC_PORT_OFFSET(unit, port) < 0) || 
		     (SOC_PORT_OFFSET(unit, port) >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS)) ) {
		    /* don't create subports on the fabric side higig ports */
		    continue;
		}

		BCM_GPORT_MODPORT_SET(intf_gport, modid, port);
		hg = SOC_PORT_OFFSET(unit, port);
		if (IS_REQ_PORT(unit, port)) {
		    intf = hg + SB_FAB_DEVICE_SIRIUS_RQ0_INTF;
		} else {
		    intf = hg + SB_FAB_DEVICE_SIRIUS_HG0_INTF;
		}

		if (IS_HG_PORT(unit, port) || IS_REQ_PORT(unit, port)) {
		    /* configured number of subports on higig port */
		    intf_subports = soc_property_port_get(unit, port, spn_IF_SUBPORTS,
                                                          SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf]);
		} else {
		    /* one subport on CPU, always enabled */
		    intf_subports = 1;
		    subport = SB_FAB_DEVICE_SIRIUS_CPU_HANDLE;
		}

		if (intf_subports >= SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, if_subports %d out of range for port %d, unit %d\n"),
		               FUNCTION_NAME(), intf_subports, port, unit));
		    return BCM_E_PARAM;
		}
		
		/* go through all subports on the port */
		for (port_offset = 0; port_offset < intf_subports; port_offset++) {
		    /* speed configuration */
		    if (IS_HG_PORT(unit, port) || IS_REQ_PORT(unit, port)) {
                        
                        SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 
                            soc_property_port_get(unit, subports, spn_PORT_INIT_SPEED, SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE]);
                        for (fifo = 0; fifo < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; fifo++) {
                            char name[10];
                            sal_sprintf(name, "fifo%d", fifo);
                            SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[fifo] = 
                                soc_property_suffix_num_get(unit, subports, spn_PORT_INIT_SPEED, name, SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE]);
                            /* The last entry of uSubportSpeed must contain the speed of the largest FIFO buffer requested */
                            if (SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[fifo] > 
                                SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE]) 
                                SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 
                                    SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[fifo];
                        }
			
			if ((SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) && 
			    (subports == SB_FAB_DEVICE_SIRIUS_CPU_HANDLE)){
			    /* skip the CPU handle */
			    subports++;
			}
		    }

                    /* Either use current subport value or use CPU HANDLE */
                    if (subport == 0) {
                        subport = subports;
                    } else {
                        /* Resetting subports due to CPU port */
                        subports--;
                    }

		    rv = bcm_sirius_fabric_port_create(unit, intf_gport, port_offset, flags, &subport);
		    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: %s, failed to create subports on port %d, Unit(%d)\n"), 
                                   FUNCTION_NAME(), port, unit));
                        return rv;
		    } else {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "created fabric gport(0x%x) on port(0x%x)\n"),
			          subport, intf_gport));
		    }

                    subport = 0;
		    subports++;
		}
	    }
	}
    }

    return rv;
}

int
bcm_sirius_cosq_destroy_egress_internal_port(int unit,
                                             bcm_gport_t physical_gport,
                                             int internal_port)
{
    int level0_node;
    int level1_node;
    int level2_node;
    int level;
    int fifo;
    int port = -1;
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (BCM_GPORT_IS_MODPORT(physical_gport)) {
	port = BCM_GPORT_MODPORT_PORT_GET(physical_gport);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(physical_gport)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(physical_gport);
    } else {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unsupported type for gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(),
	           physical_gport,
	           unit));
	return BCM_E_PARAM;
    }

    if (!SOC_PORT_VALID(unit, port)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(),
	           physical_gport,
	           unit));
	return BCM_E_PARAM;
    }

    if ( (SOC_SBX_STATE(unit)->port_state == NULL) || (internal_port < 0) ||
	 (internal_port >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) ) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "Unit %d subport %d out of range [0-SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)\n"),
                   unit, internal_port));
        return BCM_E_PARAM;	
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[internal_port]);

    if ( (sp_info == NULL) || (sp_info->valid != TRUE) ||
	 !(sp_info->flags & (SBX_SUBPORT_FLAG_TRUNK_UCAST | SBX_SUBPORT_FLAG_TRUNK_MCAST)) ||
	 !(sp_info->flags & SBX_SUBPORT_FLAG_ON_ES) ) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, invalid internal port %d, unit %d\n"),
                   FUNCTION_NAME(), internal_port, unit));
        return BCM_E_PARAM;
    }

    /* figure out which nodes were used at each level... */
    level0_node = sp_info->egroup[0].es_scheduler_level0_node;
    level1_node = sp_info->es_scheduler_level1_node[0];
    level2_node = sp_info->es_scheduler_level2_node;

    /* dispose of fifo level */
    level = 0;
    for (fifo = level0_node; fifo < level0_node + sp_info->egroup[0].num_fifos; fifo++) {
        rv = bcm_sirius_cosq_scheduler_free(unit, level, fifo, TRUE);
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, failed to free level 0 egress scheduler"
                                   " node %d for gport 0x%x, Unit(%d)\n"),
                       FUNCTION_NAME(),
                       fifo,
                       physical_gport,
                       unit));
            return rv;
        }
    }

    /* dispose of subport level */
    level = 1;
    rv = bcm_sirius_cosq_scheduler_free(unit, level, level1_node, TRUE);
    if (BCM_E_NONE != rv) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to create level 1 egress scheduler node"
	                       " on gport 0x%x, Unit(%d)\n"),
	           FUNCTION_NAME(),
	           physical_gport,
	           unit));
        return rv;
    }

    /* dispose of channel level */
    level = 2;
    rv = bcm_sirius_cosq_scheduler_free(unit, level, level2_node, TRUE);
    if (BCM_E_NONE != rv) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to free level 2 egress scheduler node"
	                       " on gport 0x%x, Unit(%d)\n"),
	           FUNCTION_NAME(),
	           physical_gport,
	           unit));
	return rv;
    }

    /* deallocate fifo memories in FF */
    SOC_SBX_SIRIUS_STATE(unit)->uTotalInternalSubports--;
    rv = bcm_sbx_cosq_egress_group_free(unit, sp_info->egroup[0].egroup_gport);

    return rv;
}

int
bcm_sirius_cosq_create_egress_internal_port(int unit,
					    bcm_gport_t physical_gport,
                                            bcm_sirius_cosq_internal_port_class_t type,
					    int num_fifos,
                                            bcm_gport_t original_subport,
					    int *internal_port)
{
    int rv = BCM_E_NONE;
    uint32 flags;
    int subport, group_shaper, node;
    int intf = 0, port = 0, idx = 0, eg_n = -1;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_sbx_subport_info_t *orig_sp_info = NULL;
    int target, fifo = -1;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (type == BCM_SIRIUS_COSQ_INTERNAL_PORT_UNICAST_OVERSUB) {
	/* keep track all the internal port created for this original
	 * subport, this would allow auto-config of group egress shaper
	 */
	flags = SBX_SUBPORT_FLAG_TRUNK_UCAST | SBX_SUBPORT_FLAG_IN_TRUNK;
    } else if (type == BCM_SIRIUS_COSQ_INTERNAL_PORT_MULTICAST) {
	/* allow creation even if egress_multicast port are not created,
	 * trunking is done but aggregate shaping will not be aviable since
	 * that requires a handlex
	 */
	flags = SBX_SUBPORT_FLAG_TRUNK_MCAST | SBX_SUBPORT_FLAG_IN_TRUNK;
    } else {
	flags = 0;
    }

    /*
     * Create interal ports after SOC property reserved ports
     * always try to allocate a handle for internal port
     * CPU PORT CURRENTLY FIXED AT PORT 128
     */
    
    if (flags & SBX_SUBPORT_FLAG_INTERNAL) {
        if (flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) {
	    if (BCM_GPORT_IS_EGRESS_CHILD(original_subport)) {
		*internal_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(original_subport);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to create internal Multicast Trunk port on ES using gport 0x%x, Unit(%d)\n"),
		           FUNCTION_NAME(), original_subport, unit));
		return BCM_E_INTERNAL;
	    }
	} else if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
	    /* extended port mode, auto-alloc handle, search from high to low */
	    *internal_port = -1;
	    for (idx = (SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS - 1); idx >= 0; idx--) {
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[idx]);
		if (sp_info->valid == FALSE) {
		    *internal_port = idx;
		    break;
		}
	    }
	    if (*internal_port < 0) {
		return BCM_E_RESOURCE;
	    }
	} else {
	    *internal_port = 0;
	    for (idx=SB_FAB_DEVICE_SIRIUS_HG0_INTF; idx < SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES; idx++) {
                *internal_port += SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[idx];
	    }
            *internal_port += SOC_SBX_SIRIUS_STATE(unit)->uTotalInternalSubports;
	}
    }

    target = *internal_port;
    if ((num_fifos > 0) && (num_fifos < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE)) {
	fifo = num_fifos;
    } else {
	fifo = SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize;
    }

    /* fill in the original gport */
    rv = bcm_sirius_fabric_port_create_es(unit, physical_gport, -1, 
					  fifo,
					  flags, &target);
    if (BCM_E_NONE != rv) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to create internal port on ES on gport 0x%x, Unit(%d)\n"),
	           FUNCTION_NAME(), physical_gport, unit));
	return rv;
    }

    if ( (SOC_SBX_STATE(unit)->port_state == NULL) || (target < 0) ||
	 (target >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) ) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "Unit %d subport %d out of range [0-SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)\n"),
                   unit, target));
        return BCM_E_PARAM;	
    }

    if (*internal_port < 0) {
	/* used the allocated handle */
	*internal_port = target;
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[*internal_port]);

    rv = bcm_sbx_port_get_intf_portoffset(unit, *internal_port, &intf, &port);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: Could not determine subport offsets\n")));
	return rv;
    }

    if (type == BCM_SIRIUS_COSQ_INTERNAL_PORT_UNICAST_OVERSUB) {
	sp_info->original_gport = original_subport;

	if (BCM_GPORT_IS_CHILD(original_subport)) {
	    subport = BCM_GPORT_CHILD_PORT_GET(original_subport);
	} else if (BCM_GPORT_IS_EGRESS_CHILD(original_subport)) {
	    subport = BCM_GPORT_EGRESS_CHILD_PORT_GET(original_subport);
	} else if (BCM_GPORT_IS_EGRESS_GROUP(original_subport)) {
	    subport = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, original_subport, &subport, &eg_n, &num_fifos);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, original_subport 0x%x does not contain fabric_port, unit %d\n"),
		           FUNCTION_NAME(), original_subport, unit));
		return BCM_E_PARAM;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, unsupported gport type original child gport 0x%x for internal trunking port creation, Unit(%d)\n"),
	               FUNCTION_NAME(), original_subport, unit));
	    return BCM_E_PARAM;
	}

	if ( (subport < 0) || (subport >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "Unit %d subport %d out of range [0-SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)\n"),
	               unit, subport));
	    return BCM_E_PARAM;	
	}

	orig_sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	if ((orig_sp_info->valid == TRUE) && (orig_sp_info->flags & SBX_SUBPORT_FLAG_ON_ES)) {
	    if ((orig_sp_info->flags & SBX_SUBPORT_FLAG_IN_TRUNK) == 0) {
		/* mark original gport as trunked */
		orig_sp_info->flags |= SBX_SUBPORT_FLAG_IN_TRUNK;

		/* allocate group shaper */
		group_shaper = -1;
		rv = bcm_sirius_cosq_group_shaper_allocate(unit, TRUE, &group_shaper);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to allocate group shaper resource gport 0x%x, Unit(%d)\n"),
		               FUNCTION_NAME(), original_subport, unit));
		    return rv;
		}

		orig_sp_info->group_shaper = group_shaper;
		sp_info->group_shaper = group_shaper;

		/* config group shaper membership for the original subport */
		for (node = orig_sp_info->egroup[0].es_scheduler_level0_node; 
		     node < (orig_sp_info->egroup[0].es_scheduler_level0_node + orig_sp_info->egroup[0].num_fifos);
		     node++) {
		    rv = soc_sirius_es_node_group_shaper_member_config(unit, group_shaper, node, TRUE);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config group shaper membership for level 0 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), node, unit));
			return rv;
		    }
		}
		
		/* config group shaper membership for the trunked internal subport */
		for (node = sp_info->egroup[0].es_scheduler_level0_node; 
		     node < (sp_info->egroup[0].es_scheduler_level0_node + sp_info->egroup[0].num_fifos);
		     node++) {
		    rv = soc_sirius_es_node_group_shaper_member_config(unit, group_shaper, node, TRUE);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config group shaper membership for level 0 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), node, unit));
			return rv;
		    }
		}

	    } else {
		group_shaper = orig_sp_info->group_shaper;
		sp_info->group_shaper = group_shaper;
		/* update group shaper membership to include the trunked internal subport */
		for (node = sp_info->egroup[0].es_scheduler_level0_node; 
		     node < (sp_info->egroup[0].es_scheduler_level0_node + sp_info->egroup[0].num_fifos);
		     node++) {
		    rv = soc_sirius_es_node_group_shaper_member_config(unit, group_shaper, node, TRUE);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config group shaper membership for level 0 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), node, unit));
			return rv;
		    }
		}
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid original child gport 0x%x for internal trunking port creation, Unit(%d)\n"),
	               FUNCTION_NAME(), original_subport, unit));
	    return BCM_E_PARAM;
	}
    } else {
	/* group shaper was pre-allocated, update group shaper membership only */
	group_shaper = SB_FAB_DEVICE_SIRIUS_MC_EGRESS_GROUP_SHAPER;
	for (node = sp_info->egroup[0].es_scheduler_level0_node; 
	     node < (sp_info->egroup[0].es_scheduler_level0_node + sp_info->egroup[0].num_fifos);
	     node++) {
	    rv = soc_sirius_es_node_group_shaper_member_config(unit, group_shaper, node, TRUE);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to config group shaper membership for level 0 node %d, Unit(%d)\n"),
		           FUNCTION_NAME(), node, unit));
		return rv;
	    }

	    /* set internal/CPU port speed to 1G */
            sp_info->egroup[0].port_speed[node - sp_info->egroup[0].es_scheduler_level0_node] = 1000;
	}
        sp_info->egroup[0].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 1000;
    }

    rv = soc_sirius_config_ff(unit, *internal_port, 0, FF_MEM_ALLOCATE);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Unit %d Internal Egress Fifo Port creation failed\n"),
	           unit));
	return rv;
    }

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "Unit %d Creating Internal Egress Fifo Port, phy_gport: 0x%x, type: 0x%x, orig_subport: 0x%x, internal_port: 0x%x\n"),
                 unit, physical_gport, type, original_subport, (*internal_port)));

    return rv;
}

static int
_bcm_sirius_cosq_destmod_destport_get(int unit, int queue,
                         bcm_gport_t gport, int32 *p_dest_switch_modid, int32 *p_dest_switch_port)
{
    int                                     rv = BCM_E_NONE;
    bcm_gport_t                             switch_port;
    int32                                   logical_eg;
    bcm_sbx_cosq_bw_group_state_t          *p_bwstate;
    bcm_sbx_cosq_queue_state_t             *p_qstate;
    bcm_sbx_cosq_egress_group_state_t      *p_eg;
    int32                                   bw_group;


    p_eg = SOC_SBX_STATE(unit)->egress_group_state;
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    bw_group = p_qstate->bw_group;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    if (BCM_GPORT_IS_MODPORT(gport)) {
            BCM_GPORT_CHILD_SET(gport, BCM_GPORT_MODPORT_MODID_GET(gport), BCM_GPORT_MODPORT_PORT_GET(gport));
    }
    else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        logical_eg = BCM_GPORT_EGRESS_GROUP_GET(gport);
        if ((logical_eg < 0) || (logical_eg >= SOC_SBX_CFG(unit)->num_egress_group) ||
                (SOC_SBX_STATE(unit)->egress_group_state[logical_eg].eg_in_use != TRUE)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, egress group %d not valid,  Unit(%d)\n"),
                       FUNCTION_NAME(), gport, unit));
            return BCM_E_PARAM;
        }	    
        BCM_GPORT_CHILD_SET(gport, BCM_GPORT_EGRESS_GROUP_MODID_GET(gport),(p_eg + logical_eg)->child_port);
    }

    /* Get the switch port associated with this fabric port */
    rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, gport, &switch_port);
    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: error returned from bcm_sbx_stk_fabric_map_get_switch_port gport(0x%08x) error(%d)\n"),
                   p_bwstate->gport, rv));
        return rv;
    }

    /* The physical port is of the form mod/port */
    (*p_dest_switch_modid) = BCM_GPORT_MODPORT_MODID_GET(switch_port);
    (*p_dest_switch_port) = BCM_GPORT_MODPORT_PORT_GET(switch_port);

    return(rv);
}

int
_bcm_sirius_cosq_add_queue(int unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region,
                          int sysport, int eset, int dest_node, int dest_port, int dest_mc, int dest_cos,
                          int32 dest_type, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                          bcm_sbx_cosq_bw_group_params_t *p_bwparams, int is_user_invoked,
                          int inhibit_write)
{
    int rv = BCM_E_NONE;
    int32 hold_ts, q_type;
    int32 base_queue;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int subport = 0, eg_n = 0, num_fifos = 0, ef_sysport;
    int32 bw_group;
    int32 dest_switch_modid;
    int32 dest_switch_port;
    int32 mc = 0, ef = 0, fct = 0;
    int intf=0;
    bcm_gport_t gport;
    int mymodid;
    int internal_eset, node, port;

    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    bw_group = p_qstate->bw_group;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    base_queue = p_bwstate->base_queue;

    ef = ((p_qstate->ingress.bw_mode == BCM_COSQ_EF) ? 1 : 0);

    if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        mc=1;
        sysport = eset;
    } else {
        mc=0;
    }

    /* If sysport is allocated, Vport or Vport Legacy mode and expanded esets
     * use sysport to overwrite node/port/mc.
     */

    if (dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        if (soc_feature(unit, soc_feature_egr_multicast_independent_fc) ||
                      (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) ) {
            if (p_qstate->ingress.bw_mode == BCM_COSQ_EF) {
                internal_eset = BCM_INT_SBX_COSQ_ESET_TO_EF_ESET(unit, eset);
            }
            else {
                internal_eset = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, eset);
            }
            BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_ESET(unit, internal_eset, &node, &port, &mc);
            if (mc == 1) {
                sysport = internal_eset;
            }
            else {
                BCM_INT_SBX_COSQ_GET_SYSPORT_FROM_NODE_PORT(unit, node, port, mc, &sysport);
            }
        }
    }
    else {
	if (soc_feature(unit, soc_feature_egr_independent_fc)) {
	    if (!SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
		/* at extended port mode, EF/!EF using same sysport/fcd */
		if (p_qstate->ingress.bw_mode == BCM_COSQ_EF) {
		    sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
		}
		else {
		    sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
		}
	    }
        }
    }

    if (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) {
	if (BCM_GPORT_IS_EGRESS_GROUP(p_bwstate->gport)) {
	    subport = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, p_bwstate->gport, &subport, &eg_n, &num_fifos);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
		           FUNCTION_NAME(), p_bwstate->gport, unit));
		return BCM_E_PARAM;
	    }    
	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	
	    fct = sp_info->egroup[eg_n].es_scheduler_level0_node;
	
	    /* Set Non-ef Fifo */
	    rv = soc_sirius_fd_fct_set(unit, sysport, 0, mc, fct + 1);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "[%d]:unable to write to unit %d:gmt for sysport %d destination port(%d)\n"),
		           __LINE__, 
		           unit,
		           sysport,
		           fct));
	    }
	
	    /* Set Ef Fifo */
	    rv = soc_sirius_fd_fct_set(unit, sysport, 1, mc, fct);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "[%d]:unable to write to unit %d:gmt for sysport %d destination port(%d)\n"),
		           __LINE__, 
		           unit,
		           sysport,
		           fct));
	    }
	}

	if (queue_region == bcm_sbx_cosq_queue_region_local) {

	    /* leaf node to queue, queue to leaf node are not setup here, since the leaf
	     * node can not be allocated till level 1 scheduler is allocated and the
	     * queue is attached to the level 1 scheduler
	     */

	} else {

	    /* FIC queues, update sysport and cos in Q2SC and S2Q tables */
	    rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, sysport, dest_cos);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: write to Q2SC table failed for queue(%d)\n"),
		           queue));
		return rv;
	    }

	    rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, base_queue);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
		           FUNCTION_NAME(), queue));
		return rv;
	    }

	    rv = soc_sirius_qs_sysport_to_node_set(unit, sysport, dest_node);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: write to S2N table failed for queue(%d)\n"),
		           queue));
		return rv;
	    }
	    if (soc_feature(unit, soc_feature_egr_independent_fc) && !ef) {
	        ef_sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
		rv = soc_sirius_qs_sysport_to_node_set(unit, ef_sysport, dest_node);
		if (rv) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: write to S2N table failed for queue(%d)\n"),
		               queue));
		    return rv;
		}
	    }
	}
    }

    /* call the add_delete queue interface */
    rv = _bcm_sirius_cosq_add_delete_queue(unit, TRUE /* add */, queue, queue_region, p_bwparams, p_qparams);
    if (rv) {
	return rv;
    }

    if ((BCM_INT_SBX_DEST_TYPE_MULTICAST != dest_type) &&
        (base_queue == queue) &&
        (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol)) {
        /*
         *  Not multicast, so unicast, and in XGS mode, with base queue as
         *  expected, so update XGS mode ingress processing.
         *
         *  Also load the unicast qsel_offset.
         *
         *  If application has taken over the ingress processing resources, the
         *  writes will not occur, though the other lookups are performed just
         *  to be sure everything is in proper order.
         */
        p_bwstate->cos_map = SOC_SBX_CFG_SIRIUS(unit)->ucast_cos_map;

        /*
         *  Determine how the GPORT maps as far as destination module and
         *  destination port.  Both of these are needed, and if they can not be
         *  found, SDK can't automatically set the ingress processing to
         *  include this queue set.
         */
        gport = p_bwstate->gport;
        rv = _bcm_sirius_cosq_destmod_destport_get(unit,
                                                   queue,
                                                   gport,
                                                   &dest_switch_modid,
                                                   &dest_switch_port);
        if (BCM_E_NONE == rv) {
            /* was able to look up destination port information */
            if (BCM_STK_MOD_TO_NODE(mymodid) == dest_node) {
                /*
                 *  Local node, so check interface to determine if this is
                 *  a CPU port.  If it is, then we want to update the CPU
                 *  destination predicate.
                 */
                rv = bcm_sbx_port_get_intf_portoffset(unit,
                                                      dest_port,
                                                      &intf,
                                                      &subport);
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR mapping a fabric port to an interface")));
                    return rv;
                }
                if (SB_FAB_DEVICE_SIRIUS_CPU_INTF == intf) {
                    /* program the remote cpu information */
                    rv = soc_sirius_rb_higig2_remote_cpu_config(unit,
                                                                dest_switch_modid,
                                                                dest_switch_port,
                                                                base_queue);
                    if (SOC_E_NONE != rv) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "Error programming the REMOTE CPU"
                                               " information\n")));
                        return rv;
                    }
                } /* if (SB_FAB_DEVICE_SIRIUS_CPU_INTF == intf) */
            } /* if (BCM_STK_MOD_TO_NODE(mymodid) == dest_node) */
            if (inhibit_write) {
                /*
                 *  Applications wants to prevent SDK updating the queue for
                 *  this particular destination.
                 */
                SHR_BITSET(SOC_SBX_SIRIUS_STATE(unit)->inhibitQsel, base_queue);
            } else { /* if (inhibit_write) */
                /* SDK needs to update the queue for this destination. */
                SHR_BITCLR(SOC_SBX_SIRIUS_STATE(unit)->inhibitQsel, base_queue);
                rv = soc_sirius_rb_higig2_header_unicast_queue_map_config(unit,
                                                                          dest_switch_modid,
                                                                          dest_switch_port,
                                                                          base_queue);
                if (SOC_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: write to RB queue map table failed switch"
                                           " modid/port(%d/%d) base queue(%d)\n"),
                               dest_switch_modid,
                               dest_switch_port,
                               base_queue));
                    return rv;
                }
            } /* if (inhibit_write) */
        } /* if (BCM_E_NONE == rv) */
        /*
         *  There is no else case for _bcm_sirius_cosq_destmod_destport_get
         *  since if it fails, we can not look up the destination port and so
         *  can not program either CPU destination predicate or the proper
         *  entry in the unicast qsel.  This is not necessarily an invalid
         *  condition, so it is not treated as an error.
         */
        /*
         *  The bcm_sirius_cosq_target_set function will update the GMT later,
         *  when bcm_sirius_cosq_add_queue is called.
         */
    } else if ((BCM_INT_SBX_DEST_TYPE_MULTICAST == dest_type) &&
               (base_queue == queue) &&
               (SOC_SBX_IF_PROTOCOL_XGS ==
                SOC_SBX_CFG(unit)->uInterfaceProtocol)) {
        /*
         *  Multicast, base queue as expected, and XGS mode.  Actual queue
         *  mapping will be done by bcm_multicast_fabric_distribution_set.
         *
         *  Load the multicast qsel_offset.
         */
        p_bwstate->cos_map = SOC_SBX_CFG_SIRIUS(unit)->mcast_cos_map;
    }

    /*
     * Set queue params: queue type and hold_ts (in QS)
     */
    rv = _bcm_sirius_cosq_select_queue_params(unit, queue, queue_region, p_qparams, &hold_ts, &q_type);
    if (rv) {
	return rv;
    }

    rv = soc_sirius_qs_queue_parameter_set(unit, queue, q_type, hold_ts);
    if (rv) {
	return rv;
    }


    if (q_type == BCM_COSQ_CALENDAR) {
        rv = soc_sirius_queue_min_util_set(unit, queue, 
					   SOC_SBX_CFG(unit)->connect_min_util_tdm_calendar_template);
        if (rv != SOC_E_NONE) {
            return(rv);
        }

        rv = soc_sirius_queue_max_age_set(unit, queue, 
					  SOC_SBX_CFG(unit)->connect_max_age_time_tdm_calendar_template);
        if (rv != SOC_E_NONE) {
            return(rv);
        }
    }

    /*
     * set anemic configuration
     */
    if (is_user_invoked == TRUE) {
        rv = soc_sirius_queue_min_util_set(unit, queue,
                                    SOC_SBX_CFG(unit)->connect_min_util_template[dest_cos]);
        if (rv != SOC_E_NONE) {
            return(rv);
        }

        rv = soc_sirius_queue_max_age_set(unit, queue,
                                    SOC_SBX_CFG(unit)->connect_max_age_time_template[dest_cos]);
        if (rv != SOC_E_NONE) {
            return(rv);
        }
    }
    else {
        /* anemic configuration is valid and need not be updated */
    }

    if (queue_region == bcm_sbx_cosq_queue_region_global) {
        if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            /* check to make sure that the Global Strict Priority configuration is */
            /* consistent                                                          */

            rv = _bcm_sirius_cosq_update_gsp(unit, queue, q_type, p_qparams->bw_mode);
            if (rv) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: set ingress params configuration (strict priority) error(%d)\n"),
                           rv));
                return(rv);
            }
        }
    }

    return rv;
}

int
bcm_sirius_cosq_add_queue(int unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region,
                          int sysport, int eset, int dest_node, int dest_port, int dest_mc, int dest_cos,
                          int32 dest_type, bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
                          bcm_sbx_cosq_bw_group_params_t *p_bwparams,
                          int inhibit_write)
{
    return(_bcm_sirius_cosq_add_queue(unit, queue, queue_region, sysport, eset, dest_node,
                                      dest_port, dest_mc, dest_cos,
                                      dest_type, p_qparams, p_bwparams, TRUE,
                                      inhibit_write));
}

int
bcm_sirius_cosq_target_set(int unit,
                           bcm_gport_t gport,
                           bcm_module_t module,
                           int sysport,
                           int port)
{
    int result;
    int xresult;
    bcm_trunk_t tid;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    bcm_gport_t trunkGport[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;
    bcm_if_t encaps[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    bcm_sbx_subport_info_t *sp_info = NULL;
    int fabric_port = -1, eg_n = 0;
    int modid = 0, mymodid = 0, fifo;
    int domain0 = 0, domain1 = 0;
    int rv = BCM_E_NONE;
    unsigned int index;

    result = bcm_sirius_trunk_find_and_get(unit,
                                           module,
                                           gport,
                                           &tid,
                                           &trunkInfo,
                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                           &(trunkMembers[0]),
                                           &memberCount);
    if (BCM_E_NONE == result) {
        /* this gport belongs to an aggregate; need to build distrib group */
        tid = sysport + SB_FAB_DEVICE_SIRIUS_SYSPORT_OFFSET;
        /* create the distribution group */
        result = bcm_sirius_multicast_aggregate_create_id(unit,
                                                          BCM_MULTICAST_TYPE_L2 |
                                                          BCM_MULTICAST_DISABLE_SRC_KNOCKOUT,
                                                          tid);
        if (BCM_E_NONE == result) {
            /* successfully created the distribution group */
            for (index = 0; index < memberCount; index++) {
                trunkGport[index] = trunkMembers[index].gport;
                encaps[index] = 0;
            }
            /* set the distribution group membership to same as aggregate */
            result = bcm_sirius_multicast_egress_set_override_oi(unit,
                                                                 tid,
                                                                 memberCount,
                                                                 &(trunkGport[0]),
                                                                 &(encaps[0]),
                                                                 FALSE);
            if (BCM_E_NONE != result) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "unable to add %d targets to distribution group on"
                                       " unit %d sysport %d incuding gport %08X: %d (%s)\n"),
                           memberCount,
                           unit,
                           sysport,
                           gport,
                           result,
                           _SHR_ERRMSG(result)));
                /* dispose of the distribution group */
                xresult = bcm_sirius_multicast_destroy(unit, tid);
                COMPILER_REFERENCE(xresult);
            }
	    
        } else { /* if (BCM_E_NONE == result) */
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "unable to create distribution group on unit %d sysport"
                                   " %d including gport %08X: %d (%s)\n"),
                       unit,
                       sysport,
                       gport,
                       result,
                       _SHR_ERRMSG(result)));
        } /* if (BCM_E_NONE == result) */
    } else { /* if (BCM_E_NONE == result) */
        /* this gport is not in an aggregate; set it up as unicast */
	if (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) {
	    if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode && !BCM_GPORT_IS_EGRESS_GROUP(gport)) {
		/* extended port mode, fabric port may not be fifo index */
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
		fifo = sp_info->egroup[0].es_scheduler_level0_node;
		port = (fifo / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE);

		/* update fct */
		if ((fifo % SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE) == 0) {
		    /* point to fifo 0/1 of the fifo group */
		    rv = soc_sirius_fd_fct_set(unit, sysport, 0, 0, 1);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		
		    rv = soc_sirius_fd_fct_set(unit, sysport, 1, 0, 0);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		} else {
		    /* point to fifo 2/3 of the fifo group */
		    rv = soc_sirius_fd_fct_set(unit, sysport, 0, 0, 3);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		
		    rv = soc_sirius_fd_fct_set(unit, sysport, 1, 0, 2);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		}
	    }

	    result = soc_sirius_fd_unicast_gmt_set(unit, sysport, port);
	    if (BCM_E_NONE != result) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "[%d]:unable to write to unit %d:gmt for sysport %d"
		                       " destination port(%d)\n"),
		           __LINE__, 
		           unit,
		           sysport,
		           port));
	    }
    
	    if ((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) &&
		!SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
		/* this gport is not in an aggregate; set it up as unicast */
		result = soc_sirius_fd_unicast_gmt_set(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport), port);
		if (BCM_E_NONE != result) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "[%d]unable to write to unit %d:gmt for sysport %d"
		                           " destination port(%d)\n"),
		               __LINE__,
		               unit,
		               BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport),
		               port));
		}
	    }

	    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));
	    if (BCM_GPORT_IS_CHILD(gport)) {
		modid = BCM_GPORT_CHILD_MODID_GET(gport);
	    } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
		modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
	    } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
		modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
	    } 
	    if (modid == mymodid) {

		/* this gport is not in an aggregate; set it up as unicast */
		result = bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &eg_n, NULL);
		if (result != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
		               FUNCTION_NAME(), gport, unit));
		    return BCM_E_PARAM;
		}
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
		
		/* setup DMT table */
                BCM_IF_ERROR_RETURN(soc_sirius_dmt_mem_get(unit, sysport, &domain0, &domain1));
		if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
		    /* both domains are used. domain_number0 is for ef=0 and use odd domain
		     * domain_number_1 is for ef=1 and use even domain. (seems this way for multicast)
		     */
		    if (sp_info->egroup[eg_n].nef_fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) {
			domain0 = sp_info->egroup[eg_n].nef_fcd;
		    }
		    if (sp_info->egroup[eg_n].ef_fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) {
			domain1 = sp_info->egroup[eg_n].ef_fcd;
		    }
                    
                    BCM_IF_ERROR_RETURN(soc_sirius_dmt_mem_set(unit, BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport), domain0, domain1));
		} else {
		    if (sp_info->egroup[eg_n].ef_fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) {
			/* only even domains are used */
			domain0 = domain1 = sp_info->egroup[eg_n].ef_fcd;
		    }
		}
                BCM_IF_ERROR_RETURN(soc_sirius_dmt_mem_set(unit, sysport, domain0, domain1));
	    }
	} else { /* if (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) */
	    result = BCM_E_NONE;
	}
    } /* if (BCM_E_NONE == result) */
    return result;
}

int
_bcm_sirius_cosq_delete_queue(int unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region, int is_user_invoked)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_cosq_queue_params_ingress_t *p_qparams = NULL;
    bcm_sbx_mgid_list_t *pElem = NULL;
    int32 bw_group = 0, base_queue = 0;
    bcm_gport_t switch_port = 0;
    int32 dest_switch_modid = 0;
    int32 dest_switch_port = 0;
    bcm_sbx_cosq_control_t control_info;
    int value = 0, is_deallocated = 0, template = 0, num_cos = 0;
    int32 mc = 0, sysport = 0, cosq = 0;
    uint32 status = 0, mgid = 0;
    int temp = 0;
    bcm_gport_t gport;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Deleting queue(%d)\n"),
              queue));

    status = soc_sirius_qs_queue_to_sysport_cos_get(unit, queue, &mc, &sysport, &cosq);
    if (status != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: Q2SC read failed\n")));
        return BCM_E_MEMORY;
    }

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_qparams = &p_qstate->ingress;
    bw_group = p_qstate->bw_group;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];
    base_queue = p_bwstate->base_queue;

    num_cos = sbx_num_cosq[unit];

    if (queue_region == bcm_sbx_cosq_queue_region_local) {
        cosq = queue - base_queue;
    }

    bcm_sirius_cosq_set_ingress_shaper(unit,
				       base_queue,
				       (bcm_cos_queue_t) cosq,
				       num_cos,
				       0,       /* shape_limit_kbps */
				       0,       /* logical shaper port */
				       FALSE);  /* enable_shaping */

    if (!mc) {
        /*
         *  Not multicast, so unicast, and in XGS mode, with base queue as
         *  expected, so update XGS mode ingress processing.
         *
         *  If application has taken over the ingress processing resources, the
         *  writes will not occur, though the other lookups are performed just
         *  to be sure everything is in proper order.
         */
        /*
         *  Determine how the GPORT maps as far as destination module and
         *  destination port.  Both of these are needed, and if they can not be
         *  found, SDK can't automatically set the ingress processing to
         *  include this queue set.
         */
        gport = p_bwstate->gport;
        if (BCM_GPORT_IS_MODPORT(gport)) {
            BCM_GPORT_EGRESS_CHILD_SET(gport,
                                       BCM_GPORT_MODPORT_MODID_GET(p_bwstate->gport),
                                       BCM_GPORT_MODPORT_PORT_GET(p_bwstate->gport));
        }
        rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, gport, &switch_port);
        if (BCM_E_NONE == rv) {
            /* was able to look up destination port information */
            /* The physical port is of the form mod/port */
            dest_switch_modid = BCM_GPORT_MODPORT_MODID_GET(switch_port);
            dest_switch_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);
            /* update the ingress unicast qsel, if applicable */
            if (SOC_SBX_IF_PROTOCOL_XGS ==
                SOC_SBX_CFG(unit)->uInterfaceProtocol) {
                /* XGS mode */
                if (!SHR_BITGET(SOC_SBX_SIRIUS_STATE(unit)->inhibitQsel,
                                base_queue)) {
                    /* update not inhibited by application request */
                    rv = soc_sirius_rb_higig2_header_unicast_queue_map_config(unit,
                                                                              dest_switch_modid,
                                                                              dest_switch_port,
                                                                              SIRIUS_Q_BASE_INVALID);
                    if (rv) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: write to RB queue map table failed switch"
                                               " modid/port(%d/%d) base queue(%d)\n"),
                                   dest_switch_modid,
                                   dest_switch_port,
                                   SIRIUS_Q_BASE_INVALID));
                        return rv;
                    }
                } /* if (not inhibiting updates by application request) */
            } /* if (XGS mode) */

            /* call the add_delete queue interface */
            if (p_qparams->enabled == TRUE) {
                rv = _bcm_sirius_cosq_add_delete_queue(unit, FALSE /* delete */, queue, queue_region, NULL, p_qparams);
                if (rv) {
                    return rv;
                }
            }
        } /* if (BCM_E_NONE == rv) */

#if 0 
        /* Set destination port in GMT table */
        rv = soc_sirius_fd_unicast_gmt_set(unit, sysport, 0);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: write to GMT table failed sysport(%d) destination port(%d)\n"),
                       sysport, 0));
            return rv;
        }
#endif 

        /* destroy distribution group if appropriate, and clear the sysport */
        rv = _bcm_sirius_cosq_delete_sysport(unit, sysport);
        if (rv) {
          LOG_ERROR(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "ERROR: _bcm_sirius_cosq_delete_sysport\n")));
          return rv;
        }
    } else { /* if (!mc) */
      /* Multicast queue delete */
      if (p_qstate->mgid_list != NULL) {
	while (p_qstate->mgid_list->data > 0) {
	  DQ_REMOVE_TAIL(&p_qstate->mgid_list->node, pElem);
	  p_qstate->mgid_list->data--;
      /* coverity[check_after_deref : FALSE] */
	  if (pElem == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: MGID element invalid\n")));
	    return BCM_E_MEMORY;
	  }

	  mgid = pElem->data;
	  sal_free(pElem);

	  rv = soc_sirius_rb_higig2_header_multicast_queue_map_read(unit, mgid, &temp);
	  if (rv) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: Read of RB queue map table failed switch mgid(%d)\n"),
	               mgid));
	    return rv;
	}
	  if (temp == base_queue) {
	    rv = soc_sirius_rb_higig2_header_multicast_queue_map_config(unit, mgid, SIRIUS_Q_BASE_INVALID);
	    if (rv) {
	      LOG_ERROR(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "ERROR: write to RB queue map table failed switch mgid(%d) base queue(%d)\n"),
	                 mgid, base_queue));
	      return rv;
	    }
	  }
	}
	sal_free(p_qstate->mgid_list);
	p_qstate->mgid_list = NULL;
      }
    }

    rv = soc_sirius_qs_queue_parameter_set(unit, queue, 0, 0);
    if (rv) {
	return rv;
    }

    
    /*
     * set anemic configuration
     */
    if (is_user_invoked == TRUE) {
        control_info.u.conn.queue = queue;
        control_info.u.conn.queue_region = queue_region;
	rv = bcm_sirius_cosq_control_get(unit, p_bwstate->gport, cosq,
					 bcmCosqControlFabricConnectMinUtilization,
					 &value, &control_info);
        if (rv != SOC_E_NONE) {
            return(rv);
        }
	/* free previous resource */
	soc_sbx_connect_min_util_dealloc(unit, 0, value, &is_deallocated, &template);

	rv = bcm_sirius_cosq_control_get(unit, p_bwstate->gport, cosq,
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

    if (queue_region == bcm_sbx_cosq_queue_region_local) {

	/* leaf node to queue, queue to leaf node are not setup here, since the leaf
	 * node can not be allocated till level 1 scheduler is allocated and the
	 * queue is attached to the level 1 scheduler
	 */
    } else {
        /* FIC queues, set Q2SC to invalid */
        rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, 1, 0xfff, 0xf);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: write to Q2SC table failed for queue(%d)\n"),
	               queue));
	    return rv;
	}
	/* FIC queues, set S2Q base queue to maximum */
	rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, SIRIUS_Q_BASE_INVALID);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
	               FUNCTION_NAME(), queue));
	    return rv;
	}
	/* Remove sysport to node setting */
	rv = soc_sirius_qs_sysport_to_node_set(unit, sysport, 0xff);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: write to S2N table failed for queue(%d)\n"),
	               queue));
	    return rv;
	}
    }

    return rv;
}

int
bcm_sirius_cosq_delete_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region)
{
    return(_bcm_sirius_cosq_delete_queue(unit, queue, queue_region, TRUE));
}

int
bcm_sirius_cosq_enable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region) 
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_queue_params_ingress_t *p_qparams = NULL;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_bw_group_params_t *p_bwparams = NULL;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_qparams = &p_qstate->ingress;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];
    p_bwparams = &p_bwstate->path;

    if (p_qstate->ingress.enabled == TRUE) {
	return BCM_E_NONE;
    }
    rv = _bcm_sirius_cosq_add_delete_queue(unit, TRUE /* add */, queue, queue_region, p_bwparams, p_qparams);

    return rv;
}

int
bcm_sirius_cosq_disable_queue(int  unit, int queue, bcm_sbx_cosq_queue_region_type_t queue_region) 
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_queue_params_ingress_t *p_qparams = NULL;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_bw_group_params_t *p_bwparams = NULL;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_qparams = &p_qstate->ingress;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];
    p_bwparams = &p_bwstate->path;

    if (p_qstate->ingress.enabled == FALSE) {
	return BCM_E_NONE;
    }
    rv = _bcm_sirius_cosq_add_delete_queue(unit, FALSE /* delete */, queue, queue_region, p_bwparams, p_qparams);

    return rv;
}

int
bcm_sirius_cosq_enable_fifo(int  unit, 
			    bcm_gport_t gport, 
			    bcm_cos_queue_t cosq, 
			    bcm_sbx_cosq_queue_region_type_t queue_region) 
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    fifo_map_table_entry_t fifo_entry;
    int fifo = 0, is_empty = 0, num_fifos = 0, eg_num = 0, fabric_port = -1, enable = 0;
    
    if (!(BCM_GPORT_IS_CHILD(gport) || 
	  BCM_GPORT_IS_EGRESS_CHILD(gport) || 
	  BCM_GPORT_IS_EGRESS_GROUP(gport))) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: Unsuported gport type 0x%x\n"),
	           gport));
	rv = BCM_E_PARAM;
	return rv;	
    }

    if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &eg_num, &num_fifos) != BCM_E_NONE) { 
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: fifo gport cos %d unknown\n"),
	           cosq));
	rv = BCM_E_PARAM;
	return rv;	
    }
	
    if (cosq >= num_fifos) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: Cosq %d exceeds %d fifos on gport 0x%x\n"),
	           cosq, num_fifos, gport));
	rv = BCM_E_PARAM;
	return rv;	
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
    fifo = sp_info->egroup[eg_num].es_scheduler_level0_node + cosq;

    BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, fifo, &fifo_entry));
    enable = soc_mem_field32_get(unit, FIFO_MAP_TABLEm, &fifo_entry, ENf);

    if (enable != 0) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "gport(0x%x) fifo(%d) already ENABLED\n"),
	          gport, cosq));
	return BCM_E_NONE;
    }

    /* Assert Flow Control */
    rv = soc_sirius_fifo_force_full_set(unit, fifo, TRUE);
    if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: fifo %d flush failed\n"),
	               fifo));
	    return rv;	
    }
    
    /* Check if fifo depth is zero i.e. empty */
    rv = soc_sirius_fifo_empty(unit, fifo, &is_empty);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: fifo empty check for fifo %d failed\n"),
	           fifo));
        goto err;
    }
    
    if (is_empty == FALSE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "FIFO not empty fifo: %d, gport: 0x%x\n"),
                  fifo, gport));
	
        /* Flush port */
        rv = soc_sirius_port_flush(unit, fifo);
        if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: fifo flush for fifo %d failed\n"),
	               fifo));
            goto err;
        }
    }
    
    rv = soc_sirius_es_node_hierachy_config(unit, 
					    SIRIUS_ES_LEVEL_FIFO,
					    fifo,
					    TRUE /* enable */, 
					    SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, 
					    SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE);
    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: fifo flush for fifo %d failed\n"),
                   fifo));
        goto err;
    }
err:
    rv = soc_sirius_fifo_force_full_set(unit, fifo, FALSE);
    if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: fifo %d flush failed\n"),
	               fifo));
    }

    return rv;	
}

int
bcm_sirius_cosq_disable_fifo(int  unit, 
			     bcm_gport_t gport, 
			     bcm_cos_queue_t cosq, 
			     bcm_sbx_cosq_queue_region_type_t queue_region) 
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    fifo_map_table_entry_t fifo_entry;
    int fifo = 0, is_empty = 0, num_fifos = 0, eg_num = 0, fabric_port = -1, enable = 0;

    if (!(BCM_GPORT_IS_CHILD(gport) || 
	  BCM_GPORT_IS_EGRESS_CHILD(gport) || 
	  BCM_GPORT_IS_EGRESS_GROUP(gport))) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: Unsuported gport type 0x%x\n"),
	           gport));
	rv = BCM_E_PARAM;
	return rv;	
    }

    if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &eg_num, &num_fifos) != BCM_E_NONE) { 
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: fifo gport cos %d unknown\n"),
	           cosq));
	return BCM_E_PARAM;
    }
    
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
    fifo = sp_info->egroup[eg_num].es_scheduler_level0_node + cosq;
    
    BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, fifo, &fifo_entry));
    enable = soc_mem_field32_get(unit, FIFO_MAP_TABLEm, &fifo_entry, ENf);

    if (enable == 0) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "gport(0x%x) fifo(%d) already DISABLED\n"),
	          gport, cosq));
	return BCM_E_NONE;
    }

    /* Assert Flow Control */
    rv = soc_sirius_fifo_force_full_set(unit, fifo, TRUE);
    if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: fifo %d flush failed\n"),
	               fifo));
	    return rv;	
    }
    
    /* Check if fifo depth is zero i.e. empty */
    rv = soc_sirius_fifo_empty(unit, fifo, &is_empty);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: fifo empty check for fifo %d failed\n"),
	           fifo));
        goto err;
    }
    
    if (is_empty == FALSE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "FIFO not empty fifo: %d, gport: 0x%x\n"),
                  fifo, gport));
	
        /* Flush port */
        rv = soc_sirius_port_flush(unit, fifo);
        if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: fifo flush for fifo %d failed\n"),
	               fifo));
            goto err;
        }
    }
    
    rv = soc_sirius_es_node_hierachy_config(unit, 
					    SIRIUS_ES_LEVEL_FIFO,
					    fifo,
					    FALSE /* disable */, 
					    SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, 
					    SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE);
    return rv;

err:
    rv = soc_sirius_fifo_force_full_set(unit, fifo, FALSE);

    return rv;
}

int
bcm_sirius_cosq_enable_get(int  unit, 
			   bcm_gport_t gport, 
			   bcm_cos_queue_t cosq,
			   int *enable) {
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    fifo_map_table_entry_t fifo_entry;
    int fifo = 0, num_fifos = 0, eg_num = 0, fabric_port = -1;

    if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &eg_num, &num_fifos) != BCM_E_NONE) { 
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: fifo gport 0x%x cos %d unknown\n"),
	           gport, cosq));
	return BCM_E_PARAM;
    }

    if (cosq >= num_fifos) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: Cosq %d exceeds %d fifos on gport 0x%x\n"),
	           cosq, num_fifos, gport));
	rv = BCM_E_PARAM;
	return rv;	
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
    fifo = sp_info->egroup[eg_num].es_scheduler_level0_node + cosq;

    BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, fifo, &fifo_entry));
    *enable = soc_mem_field32_get(unit, FIFO_MAP_TABLEm, &fifo_entry, ENf);

    return BCM_E_NONE;	
}

int
bcm_sirius_cosq_overlay_queue(int unit,
                              int queue,
                              bcm_sbx_cosq_queue_region_type_t queue_region,
                              int sysport,
                              int dest_node,
                              int dest_port,
                              int dest_mc,
                              int dest_cos,
                              int dest_type) {

    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_sirius_cosq_delete_overlay_queue(int unit,
				     int queue,
				     int base_queue,
                                     bcm_sbx_cosq_queue_region_type_t queue_region) {
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_sirius_cosq_set_ingress_params(int unit,
                                   int32 queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
                                   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
                                   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
                                   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
                                   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo) {
    int rv = BCM_E_UNAVAIL;
    int hold_ts, q_type;
    int old_hold_ts, old_q_type;
    int weight, old_weight, mc, leaf_node, level1_node, child_offset;
    int cir, old_cir;
    uint cir_mant, cir_exp;
    int sysport, dest_cos;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int bw_group = 0;
    uint32 regval = 0;
    int32 rate_delta_max_index = 0;
    int eset, internal_eset, node, port;
    voq_config_entry_t voq_config;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    bw_group = p_qstate->bw_group;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    

    /*
     * Queue type onfiguration
     */
    
    /* previous setting, cached value */
    rv = _bcm_sirius_cosq_select_queue_params(unit, queue, queue_region,
                                              p_oldqparams, &old_hold_ts, &old_q_type);
    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: _bcm_sirius_cosq_select_queue_params - oldParams, Unit:%d\n"),
                   unit));
        return(rv);
    }

    rv = _bcm_sirius_cosq_select_queue_params(unit, queue, queue_region,
                                              p_newqparams, &hold_ts, &q_type);
    if (rv) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: _bcm_sirius_cosq_select_queue_params - newParams, Unit:%d\n"),
                   unit));
        return(rv);
    }

    if ( (hold_ts != old_hold_ts) || (q_type != old_q_type) ) {
        /* Set queue params: queue type and hold_ts (in QS) */
        rv = soc_sirius_qs_queue_parameter_set(unit, queue, q_type, hold_ts);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: soc_sirius_qs_queue_parameter_set - QueueType, Unit:%d\n"),
                       unit));
            return(rv);
        }
    }

    /* Rate delta max reconfig */
    if (queue_region == bcm_sbx_cosq_queue_region_local) {
      
        LOG_DEBUG(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "  local\n")));
        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, DEMAND_ENABLEf, 0);
        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, RATE_DELTA_MAX_INDEXf, 0);
    } else { /* VOQ */
        /* The bw_group contains the bag rate  */
        _bcm_sirius_rate_delta_max_index_for_port_rate_chosen(unit, p_newbwparams->bag_rate_kbps,
                                                              &rate_delta_max_index);
	
        LOG_DEBUG(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "  VoQ RateDelatMaxindex(%d)\n"),
                   rate_delta_max_index));
        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, DEMAND_ENABLEf, 1);
        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, RATE_DELTA_MAX_INDEXf, rate_delta_max_index);
 
        SOC_IF_ERROR_RETURN(READ_VOQ_CONFIGm(unit, MEM_BLOCK_ANY, queue, &voq_config));
        soc_mem_field32_set(unit, VOQ_CONFIGm, &voq_config, DEMAND_ENABLEf, 1);
        soc_mem_field32_set(unit, VOQ_CONFIGm, &voq_config, RATE_DELTA_MAX_INDEXf, rate_delta_max_index);
        SOC_IF_ERROR_RETURN(WRITE_VOQ_CONFIGm(unit, MEM_BLOCK_ANY, queue, &voq_config));
    }

    /* For bcm88230 local queues, the template is shared for queue size and WRED 
     * so need to make sure min/max buff, gain and wred curve parameters all
     * match before we can share a template
     */
    
    /*
     * Queue size configuration
     */
    if ( (p_oldqparams->min_physical_queue_depth_bytes !=
          p_newqparams->min_physical_queue_depth_bytes) ||
         (p_oldqparams->max_physical_queue_depth_bytes >
          p_newqparams->max_physical_queue_depth_bytes) ) {
	
        /* If the queue was not enabled (deleted from bcm_cosq_gport_enable_set(false) */
        /* do not attempt to delete the queue again.  Instead, leave queue deleted.    */
        /* the user will enable the queue via gport_enable_set().                      */
        if (p_qstate->ingress.enabled == TRUE) {
            /* delete queue */
            rv = _bcm_sirius_cosq_add_delete_queue(unit, FALSE, queue, queue_region,
                                                   p_oldbwparams, p_oldqparams);
            if (rv) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: _bcm_sirius_cosq_add_delete_queue - Delete, Unit:%d\n"),
                           unit));
                return(rv);
            }
	    
            /* add queue */
            rv = _bcm_sirius_cosq_add_delete_queue(unit, TRUE, queue, queue_region,
                                                   p_newbwparams, p_newqparams);
            if (rv) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: _bcm_sirius_cosq_add_delete_queue - Add, Unit:%d\n"),
                           unit));
                return(rv);
            }
        }
	
    } else if (p_oldqparams->max_physical_queue_depth_bytes != p_newqparams->max_physical_queue_depth_bytes) {
        rv = soc_sirius_qm_queue_buffs_profile_entry_set(unit, queue, p_newqparams->template, TRUE);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: queue_buffs_profile update failed, Unit:%d\n"),
                       unit));
            return(rv);
        }
    }

    /*
     * local queues only, update child weight in level 1 node if needed
     */
    if (queue_region == bcm_sbx_cosq_queue_region_local) {
        switch (p_newqparams->bw_mode) {
	    case BCM_COSQ_AF0:
	    case BCM_COSQ_AF1:
	    case BCM_COSQ_AF2:
	    case BCM_COSQ_AF3:
	    case BCM_COSQ_AF4:
	    case BCM_COSQ_AF5:
	    case BCM_COSQ_AF6:
            weight = p_newqparams->bw_value.wfq_weight;
            if ( (weight < 0) || (weight > 0x7F ) ) {
                /* restore the software state */
                p_newqparams->bw_value.wfq_weight = p_oldqparams->bw_value.wfq_weight;
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s weight %d out of range [0-127], Unit:%d\n"),
                           FUNCTION_NAME(), weight, unit));
                return BCM_E_PARAM;
            }
            break;

        case BCM_COSQ_SP0:
        case BCM_COSQ_SP1:
        case BCM_COSQ_SP2:
        case BCM_COSQ_SP3:
        case BCM_COSQ_SP4:
        case BCM_COSQ_SP5:
        case BCM_COSQ_SP6:
            if ( (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ0) ||
                 (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ1) ||
                 (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ2) ||
                 (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ3) ||
                 (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ4) ||
                 (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ5) ||
                 (q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ6) ) {
                weight = p_newqparams->bw_value.wfq_weight;
            }
            else {
                /* NOTE:                                                              */
                /* If SP weights is required that functionality can be added. May     */
                /* require change to current application to set the weight correctly. */
                weight = 0;
            }
            break;

	    default:
            weight = 0;
            break;
        }

        switch (p_oldqparams->bw_mode) {
	    case BCM_COSQ_AF0:
	    case BCM_COSQ_AF1:
	    case BCM_COSQ_AF2:
	    case BCM_COSQ_AF3:
	    case BCM_COSQ_AF4:
	    case BCM_COSQ_AF5:
	    case BCM_COSQ_AF6:
            old_weight = p_oldqparams->bw_value.wfq_weight;
            break;

        case BCM_COSQ_SP0:
        case BCM_COSQ_SP1:
        case BCM_COSQ_SP2:
        case BCM_COSQ_SP3:
        case BCM_COSQ_SP4:
        case BCM_COSQ_SP5:
        case BCM_COSQ_SP6:
            if ( (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ0) ||
                 (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ1) ||
                 (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ2) ||
                 (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ3) ||
                 (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ4) ||
                 (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ5) ||
                 (old_q_type == SIRIUS_Q_TYPE_LOCAL_RCPQ6) ) {
                old_weight = p_oldqparams->bw_value.wfq_weight;
            }
            else {
                /* NOTE:                                                              */
                /* If SP weights is required that functionality can be added. May     */
                /* require change to current application to set the weight correctly. */
                old_weight = 0;
            }
            break;

	    default:
            old_weight = 0;
            break;
        }

        if (weight != old_weight) {
            /* find out the leaf node used */
            rv = soc_sirius_qs_queue_to_leaf_node_get(unit, queue, &mc, &leaf_node);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s failed to get leaf node for local queue %d, Unit:%d\n"),
                           FUNCTION_NAME(), queue, unit));
                return rv;
            }
	    
            /* find the level 1 node */
            level1_node = leaf_node / 8;
            child_offset = leaf_node % 8;
	    
            /* update the child weight for the child offset */
            rv = soc_sirius_ts_node_child_weight_config(unit, 1, level1_node, child_offset, weight);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, can not config queue %d weight %d on level 1 node %d offset %d, Unit(%d)\n"),
                           FUNCTION_NAME(), queue, weight, level1_node, child_offset, unit));
                return rv;
            } else {
                LOG_VERBOSE(BSL_LS_BCM_COSQ,
                            (BSL_META_U(unit,
                                        "queue(%d) weight (%d) configed in level 1 node %d offset %d\n"),
                             queue, weight, level1_node, child_offset));
            }
        }
    }

    /*
     * local queues only, update CIR if needed
     */
    if (queue_region == bcm_sbx_cosq_queue_region_local) {
        switch (p_newqparams->bw_mode) {
	    case BCM_COSQ_EF:
	    case BCM_COSQ_AF0:
	    case BCM_COSQ_AF1:
	    case BCM_COSQ_AF2:
	    case BCM_COSQ_AF3:
	    case BCM_COSQ_AF4:
	    case BCM_COSQ_AF5:
	    case BCM_COSQ_AF6:
            cir = p_newqparams->bw_value.guarantee_kbps;
            break;

	    case BCM_COSQ_SP0:
	    case BCM_COSQ_SP1:
	    case BCM_COSQ_SP2:
	    case BCM_COSQ_SP3:
	    case BCM_COSQ_SP4:
	    case BCM_COSQ_SP5:
	    case BCM_COSQ_SP6:
            cir = p_newqparams->bw_value.guarantee_kbps;
            if (cir == BCM_SBX_COSQ_RCPQ_CIR_EQUAL_ZERO) {
                cir = 0;
            }
            break;
	    default:
            cir = 0;
            break;
        }

        switch (p_oldqparams->bw_mode) {
	    case BCM_COSQ_EF:
	    case BCM_COSQ_AF0:
	    case BCM_COSQ_AF1:
	    case BCM_COSQ_AF2:
	    case BCM_COSQ_AF3:
	    case BCM_COSQ_AF4:
	    case BCM_COSQ_AF5:
	    case BCM_COSQ_AF6:
            old_cir = p_oldqparams->bw_value.guarantee_kbps;
            break;

	    case BCM_COSQ_SP0:
	    case BCM_COSQ_SP1:
	    case BCM_COSQ_SP2:
	    case BCM_COSQ_SP3:
	    case BCM_COSQ_SP4:
	    case BCM_COSQ_SP5:
	    case BCM_COSQ_SP6:
            old_cir = p_oldqparams->bw_value.guarantee_kbps;
            if (old_cir == BCM_SBX_COSQ_RCPQ_CIR_EQUAL_ZERO) {
                old_cir = 0;
            }
            break;
	    default:
            old_cir = 0;
            break;
        }

        if (cir != old_cir) {
            /* update both BAA_LEAK table for local queues */

            /* calculate rate */
            rv = soc_sbx_fabric_util_num_to_mant_exp(unit, BAA_LEAK_A0m, BAA_RATE_EXPf,
                                                     cir, &cir_mant, &cir_exp);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s failed to convert rate %d kbps to mant/exp format\n"),
                           FUNCTION_NAME(), cir));
                return(rv);
            }

            /* find the right table since the tables are indexed in a weird way */	    
            rv = soc_sirius_qs_baa_rate_set(unit, queue, cir_mant, cir_exp);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s failed to config rate %d kbps to mant/exp format\n"),
                           FUNCTION_NAME(), cir));
                return(rv);
            } else {
                LOG_VERBOSE(BSL_LS_BCM_COSQ,
                            (BSL_META_U(unit,
                                        "queue(%d) configed to have CIR of %d kbps\n"),
                             queue, cir));
            }
        }
    }

    if (queue_region == bcm_sbx_cosq_queue_region_global) {

        if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            /* check to make sure that the Global Strict Priority configuration is */
            /* consistent                                                          */

            rv = _bcm_sirius_cosq_update_gsp(unit, queue, q_type, p_newqparams->bw_mode);
            if (rv) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: set ingress params configuration (strict priority) error(%d)\n"),
                           rv));
                return(rv);
            }
        }

        if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_UNICAST) {
            if (soc_feature(unit, soc_feature_egr_independent_fc)) {

                if (p_newqparams->bw_mode != p_oldqparams->bw_mode) {

                    sysport = p_qstate->sysport;

                    rv = soc_sirius_qs_queue_to_sysport_cos_get(unit, queue, &mc, &sysport, &dest_cos);
                    if (rv != SOC_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: Q2SC read failed\n")));
                        return BCM_E_MEMORY;
                    }

                    if (p_newqparams->bw_mode == BCM_COSQ_EF) {
                        sysport = BCM_INT_SBX_SYSPORT_TO_EF_SYSPORT(unit, sysport);
                    }
                    else {
                        sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
                    }


                    /* FIC queues, update sysport and cos in Q2SC and S2Q tables */
                    rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, sysport, dest_cos);
                    if (rv) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: write to Q2SC table failed for queue(%d)\n"),
                                   queue));
                        return rv;
                    }

                    rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, p_bwstate->base_queue);
                    if (rv) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
                                   FUNCTION_NAME(), queue));
                        return rv;
                    }
                    /* SDK-45762 sysport stored in queue state should always be base sysport */
                    /* 2 sysports are assigned offset 0 for EF, offset +1 for NEF.  Base is  */
                    /* saved for independent flow control                                    */
                    /*  p_qstate->sysport = sysport; */
                }
            }
        }
        else {
            if (soc_feature(unit, soc_feature_egr_multicast_independent_fc) ||
                (!SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) ) {

                if ( (p_newqparams->bw_mode != p_oldqparams->bw_mode) && (p_qstate->attached_fifo == -1) ) {
                    rv = soc_sirius_qs_queue_to_sysport_cos_get(unit, queue, &mc, &sysport, &dest_cos);
                    if (rv != SOC_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: Q2SC read failed\n")));
                        return BCM_E_MEMORY;
                    }

                    BCM_INT_SBX_COSQ_GET_ESET_FROM_NODE_PORT(unit, p_bwstate->dest_node, p_bwstate->dest_port, p_bwstate->dest_mc, &eset);
                    if (p_qstate->ingress.bw_mode == BCM_COSQ_EF) {
                        internal_eset = BCM_INT_SBX_COSQ_ESET_TO_EF_ESET(unit, eset);
                    }
                    else {
                        internal_eset = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, eset);
                    }
                    BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_ESET(unit, internal_eset, &node, &port, &mc);
                    if (mc == 1) {
                        sysport = internal_eset;
                    }
                    else {
                        BCM_INT_SBX_COSQ_GET_SYSPORT_FROM_NODE_PORT(unit, node, port, mc, &sysport);
                    }

                    /* FIC queues, update sysport and cos in Q2SC and S2Q tables */
                    rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, sysport, dest_cos);
                    if (rv) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: write to Q2SC table failed for queue(%d)\n"),
                                   queue));
                        return rv;
                    }

                    rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, p_bwstate->base_queue);
                    if (rv) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
                                   FUNCTION_NAME(), queue));
                        return rv;
                    }
                }
            }
        }     
    }

    return(rv);
}


static int
_bcm_sirius_cosq_ingress_shaper_entry_set(int unit, int queue, int table, int table_index, shaper_leak_0_entry_t *shaper_leak_entry_p)
{
    int                         rc = BCM_E_NONE;
    shaper_leak_0_entry_t       shaper_leak_entry_dummy;

    /* set rate to very high, to avoid momentarily stop traffic */
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry_dummy, SHAPE_RATE_EXPf, 1);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry_dummy, SHAPE_RATE_MANTf, 0xff);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry_dummy, SHAPE_THRESH_EXPf, 1);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry_dummy, SHAPE_THRESH_MANTf, 0xff);

    /* configure entry */
    if (table == 0) {
	rc = WRITE_SHAPER_LEAK_0m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry_dummy);
    }
    else if (table == 1) {
	rc = WRITE_SHAPER_LEAK_1m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry_dummy);
    }
    else if (table == 2) {
	rc = WRITE_SHAPER_LEAK_2m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry_dummy);
    }
    else {
	rc = WRITE_SHAPER_LEAK_3m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry_dummy);
    }

    if (rc == SOC_E_NONE) {
	sal_usleep(10);

	if (table == 0) {
	    rc = WRITE_SHAPER_LEAK_0m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
	}
	else if (table == 1) {
	    rc = WRITE_SHAPER_LEAK_1m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
	}
	else if (table == 2) {
	    rc = WRITE_SHAPER_LEAK_2m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
	}
	else {
	    rc = WRITE_SHAPER_LEAK_3m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
	}
    }

    return(rc);
}


int
bcm_sirius_cosq_set_ingress_shaper(int unit,
                                   int base_queue,
                                   bcm_cos_queue_t cosq,
                                   int num_cos_levels,
                                   uint32 shape_limit_kbps,
                                   int set_logical_port_shaper,
                                   int enable_shaping)
{
    int                      rv = BCM_E_UNAVAIL;
    shaper_leak_0_entry_t    shaper_leak_entry;
    int                      queue, table_index, table;
    uint32                   rate_exp, rate_mant, burst_exp, burst_mant;
    uint32                   min_burst_size;
    uint64                   round_ns;
    uint32 reg_value;
    int32  voq_shape_inc, voq_shape_start, voq_shape_end, voq_shapers;
    int32  local_shape_inc, local_shape_start, local_shape_end, local_shapers;
    bcm_sbx_cosq_queue_state_t *p_qstate;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    /* config queue level ingress shaper */

    /* get the shaper range config */
    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &reg_value));
    local_shape_inc = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, reg_value, INCf);
    local_shape_start = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, reg_value, STARTQUEUEf);

    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, &reg_value));
    local_shape_end = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_ENDr, reg_value, ENDQUEUEf);

    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, &reg_value));
    voq_shape_inc = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, reg_value, INCf);
    voq_shape_start = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, reg_value, STARTQUEUEf);

    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_ENDr(unit, &reg_value));
    voq_shape_end = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_ENDr, reg_value, ENDQUEUEf);

    /* calculate number of shapers for each queue range */
    voq_shapers = (voq_shape_end - voq_shape_start);
    if (voq_shapers > 0) {
	if (voq_shape_inc == 1) {
	    /* one shaper every 4 queues */
	    voq_shapers = (voq_shapers / 4) + 1;
	} else {
	    voq_shapers++;
	}
    } else if (voq_shapers < 0) {
	/* misconfiged, assuming no voq shapers */
	voq_shapers = 0;
    }

    local_shapers = (local_shape_end - local_shape_start);
    if (local_shapers > 0) {
	if (local_shape_inc == 1) {
	    /* one shaper every 4 queues */
	    local_shapers = (local_shapers / 4) + 1;
	}
    } else if (local_shapers < 0) {
	/* misconfiged, assuming no voq shapers */
	local_shapers = 0;
    }

    /* make sure the queues picked has shaper resource, assuming local shapers is
     * after the voq shapers. And caculate the shaper resource
     */
    queue = base_queue + cosq;
    if ( (voq_shapers > 0) &&
	 (queue >= voq_shape_start) &&
	 (queue <= voq_shape_end) ) {
	/* fabric queues */
	if (voq_shape_inc == 1) {
	    if ((queue - voq_shape_start) % 4 != 0) {
		if (shape_limit_kbps != 0) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s no hardware ingress shaper resource for VOQ queue %d \n"),
		               FUNCTION_NAME(), queue));
		    return BCM_E_RESOURCE;
		} else {
		    return BCM_E_NONE;
		}
	    } else {
		table_index = (queue - voq_shape_start)/4;
	    }
	} else {
	    table_index = queue - voq_shape_start;
	}
    } else if ( (local_shapers > 0) &&
	 (queue >= local_shape_start) &&
	 (queue <= local_shape_end) ) {
	/* local queues */
	if (local_shape_inc == 1) {
	    if ( ((queue - local_shape_start) % 4) != 0) {
		if (shape_limit_kbps != 0) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s no hardware ingress shaper resource for Local queue %d \n"),
		               FUNCTION_NAME(), queue));
		    return BCM_E_RESOURCE;
		} else {
		    return BCM_E_NONE;
		}
	    } else {
		table_index = (queue - local_shape_start) / 4 + voq_shapers;
	    }
	} else {
	    table_index = (queue - local_shape_start) + voq_shapers;
	}
    } else {
	if (shape_limit_kbps != 0) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s no hardware ingress shaper resource for queue %d \n"),
	               FUNCTION_NAME(), queue));
	    return BCM_E_RESOURCE;
	} else {
	    return BCM_E_NONE;
	}
    }

    /* calculate table and table entry index */
    table = table_index & 3;
    table_index = table_index >> 2;

    /* retreive entry */
    if (table == 0) {
        rv = READ_SHAPER_LEAK_0m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry);
    }
    else if (table == 1) {
        rv = READ_SHAPER_LEAK_1m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry);
    }
    else if (table == 2) {
        rv = READ_SHAPER_LEAK_2m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry);
    }
    else {
        rv = READ_SHAPER_LEAK_3m(unit, MEM_BLOCK_ANY, table_index, &shaper_leak_entry);
    }

    if (rv != SOC_E_NONE) {
        return(rv);
    }

    rate_exp = rate_mant = burst_exp = burst_mant = 0;
    if (enable_shaping == TRUE) {
        if (p_qstate[(base_queue + cosq)].ingress.shape_burst_kbps == -1) {
	    rv = soc_sbx_fabric_util_num_to_mant_exp(unit, SHAPER_LEAK_0m, SHAPE_RATE_EXPf,
						 shape_limit_kbps, &rate_mant, &rate_exp);
            if (rv != SOC_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s failed to convert rate %d kbps to mant/exp format\n"),
	                   FUNCTION_NAME(), shape_limit_kbps));
                return(rv);
            }

	    /* default burst size, make sure the burst size will be at least 3 times of
	     * the leaked credit in each round. At the init time, we are hardcoding the
	     * loop size to be 16K cycles (max value, see SHAPER_LOOP_SIZEr). For high
	     * rate shapers (assuming 10 G), each round could accumulate around 400K bits
	     * given we don't want bucket to overflow, this will limit the minimum burst
	     * size for those high rate shapers.
	     */
	    COMPILER_64_SET(round_ns, 0, 16 * (1<<10) * 1000 / SOC_SBX_CFG(unit)->uClockSpeedInMHz);
            COMPILER_64_UMUL_32(round_ns, shape_limit_kbps);
	    rv = soc_sbx_div64(round_ns, 1000000, &min_burst_size);
	    if (rv != SOC_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s failed to calculate burst size for rate of %d kbps\n"),
	                   FUNCTION_NAME(), shape_limit_kbps));
                return(rv);
	    } else {
	        /* make sure burst_size is at least 3 times the amount of leaked credit */
	        min_burst_size = min_burst_size * 3;

	        /* set the burst size big enough to pass a MTU 1518 bytes */
	        if (min_burst_size <= (1518 * 24)) {
		    min_burst_size = 1518 * 24;
	        }
                if (min_burst_size > SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE) {
                    min_burst_size = SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE;
                }
	    }

	    rv = soc_sbx_fabric_util_num_to_mant_exp(unit, SHAPER_LEAK_0m, SHAPE_THRESH_EXPf,
						 min_burst_size, &burst_mant, &burst_exp);
            if (rv != SOC_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s failed to convert burst size %d bits to mant/exp format\n"),
	                   FUNCTION_NAME(), min_burst_size));
                return(rv);
            }	
        }
        else {
	    rv = soc_sbx_fabric_util_num_to_mant_exp(unit, SHAPER_LEAK_0m, SHAPE_RATE_EXPf,
						 shape_limit_kbps, &rate_mant, &rate_exp);
            if (rv != SOC_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s failed to convert rate %d kbps to mant/exp format\n"),
	                   FUNCTION_NAME(), shape_limit_kbps));
                return(rv);
            }

            min_burst_size = p_qstate[(base_queue + cosq)].ingress.shape_burst_kbps * 1000;
            if (min_burst_size > SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE) {
                min_burst_size = SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE;
            }
            rv = soc_sbx_fabric_util_num_to_mant_exp(unit, SHAPER_LEAK_0m, SHAPE_THRESH_EXPf,
                                               min_burst_size, &burst_mant, &burst_exp);
            if (rv != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s failed to convert burst size %d bits to mant/exp format\n"),
                           FUNCTION_NAME(), min_burst_size));
                return(rv);
            }	
        }
    }

    /* modify configuration */
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry, SHAPE_THRESH_EXPf, burst_exp);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry, SHAPE_THRESH_MANTf, burst_mant);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry, SHAPE_RATE_EXPf, rate_exp);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry, SHAPE_RATE_MANTf, rate_mant);

    /* configure entry */
    rv = _bcm_sirius_cosq_ingress_shaper_entry_set(unit, (base_queue + cosq), table, table_index, &shaper_leak_entry);

    if (rv != SOC_E_NONE) {
        return(rv);
    }

    /* Update state */
    if (set_logical_port_shaper == FALSE) {
	p_qstate[base_queue + cosq].ingress.enable_shaping = enable_shaping;
	p_qstate[base_queue + cosq].ingress.shape_limit_kbps = shape_limit_kbps;
    }

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "%s base queue %d num_cos %d use ingress shaper table %d index %d\n"),
                 FUNCTION_NAME(), base_queue, cosq, table, table_index));
    return(rv);
}

int
bcm_sirius_cosq_set_template_gain(int unit,
                  int queue,
                  int template,
                  int gain)
{
    int rv = BCM_E_NONE;
    qbuffsprofile_entry_t profileEntry;
    q_max_buffs_entry_t maxbufEntry;

    if ( (template < 0) || (template > 0x1FF) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, wred template %d out of range [0-0x1FF],  Unit(%d)\n"),
	           FUNCTION_NAME(), template, unit));
	return BCM_E_PARAM;
    }

    if ( (gain < 0) || (gain > 0xF) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, wred gain %d out of range [0-0xF],  Unit(%d)\n"),
	           FUNCTION_NAME(), gain, unit));
	return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_QBUFFSPROFILEm(unit, MEM_BLOCK_ANY, queue, &profileEntry));
    soc_mem_field32_set(unit, QBUFFSPROFILEm, &profileEntry, BUFF_TEMPLATEf, template);
    BCM_IF_ERROR_RETURN(WRITE_QBUFFSPROFILEm(unit, MEM_BLOCK_ANY, queue, &profileEntry));

    BCM_IF_ERROR_RETURN(READ_Q_MAX_BUFFSm(unit, MEM_BLOCK_ANY, template, &maxbufEntry));
    soc_mem_field32_set(unit, Q_MAX_BUFFSm, &maxbufEntry, GAINf, gain);
    BCM_IF_ERROR_RETURN(WRITE_Q_MAX_BUFFSm(unit, MEM_BLOCK_ANY, template, &maxbufEntry));

    return rv;
}

int
bcm_sirius_cosq_get_template_gain(int unit,
				  int queue,
				  int *template,
				  int *gain)
{
    int rv = BCM_E_NONE;
    qbuffsprofile_entry_t profileEntry;
    q_max_buffs_entry_t maxbufEntry;

    BCM_IF_ERROR_RETURN(READ_QBUFFSPROFILEm(unit, MEM_BLOCK_ANY, queue, &profileEntry));
    *template = soc_mem_field32_get(unit, QBUFFSPROFILEm, &profileEntry, BUFF_TEMPLATEf);

    BCM_IF_ERROR_RETURN(READ_Q_MAX_BUFFSm(unit, MEM_BLOCK_ANY, *template, &maxbufEntry));
    *gain = soc_mem_field32_get(unit, Q_MAX_BUFFSm, &maxbufEntry, GAINf);

    return rv;
}

int
bcm_sirius_cosq_gport_discard_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_NONE;
    int curve;
    wred_curve_entry_t wred_curve;
    sbZfFabWredParameters_t chip_params;
    q_max_buffs_entry_t maxbufEntry;

    if ( template >= SB_FAB_DEVICE_SIRIUS_MAX_WRED_TEMPLATES ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, template %d out of range [0-511],  Unit(%d)\n"),
	           FUNCTION_NAME(), template, unit));
	return BCM_E_PARAM;
    }

    if ( color >= BCM_SBX_COSQ_DISCARD_MAX_COLORS ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, color %d out of range [0-3],  Unit(%d)\n"),
	           FUNCTION_NAME(), color, unit));
	return BCM_E_PARAM;
    }

    curve = template * BCM_SBX_COSQ_DISCARD_MAX_COLORS + color;

    BCM_IF_ERROR_RETURN(READ_WRED_CURVEm(unit, MEM_BLOCK_ANY, curve, &wred_curve));
    if (discard->drop_probability == 0) {
	/* disabling drop probability */
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, PDROP_MAXf, color);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, TMAXf, 0xFFFF);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, TECNf, 0xFFFF);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, TMINf, 0xFFFF);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, SCALEf, 0xF);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, SLOPEf, 0xFFF);
    } else {
	/* enabling drop probability */
        rv = _bcm_sbx_device_wred_calc_config(unit, SOC_SBX_CFG(unit)->discard_probability_mtu,
					      queue_size, discard, &chip_params);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, _bcm_sbx_device_wred_calc_config,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
	    return rv;
        }

	if ((chip_params.m_nTmax > 0xFFFF) ||
	    (chip_params.m_nTecn > 0xFFFF) ||
	    (chip_params.m_nTmin > 0xFFFF) ||
	    (chip_params.m_nScale > 0xF)   ||
	    (chip_params.m_nSlope > 0xFFF)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, wred parameter out of range,  Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
	    return BCM_E_PARAM;
	}

	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, PDROP_MAXf, color);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, TMAXf, chip_params.m_nTmax);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, TECNf, chip_params.m_nTecn);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, TMINf, chip_params.m_nTmin);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, SCALEf, chip_params.m_nScale);
	soc_mem_field32_set(unit, WRED_CURVEm, &wred_curve, SLOPEf, chip_params.m_nSlope);
    }

    BCM_IF_ERROR_RETURN(WRITE_WRED_CURVEm(unit, MEM_BLOCK_ANY, curve, &wred_curve));

    /* min/max buff size and gain are also tied with template, update it here */
    rv = soc_sirius_qm_queue_min_buff_set(unit, template, min_queue_size);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: soc_sirius_qm_queue_min_buff_set, Unit:%d\n"),
	           unit));
	return(rv);
    }
    
    rv = soc_sirius_qm_queue_max_buff_set(unit, template, queue_size);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: soc_sirius_qm_queue_max_buff_set, Unit:%d\n"),
	           unit));
	return(rv);
    }

    BCM_IF_ERROR_RETURN(READ_Q_MAX_BUFFSm(unit, MEM_BLOCK_ANY, template, &maxbufEntry));
    soc_mem_field32_set(unit, Q_MAX_BUFFSm, &maxbufEntry, GAINf, discard->gain);
    BCM_IF_ERROR_RETURN(WRITE_Q_MAX_BUFFSm(unit, MEM_BLOCK_ANY, template, &maxbufEntry));

    return(rv);

}

int
bcm_sirius_cosq_gport_discard_get(int unit,
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
bcm_sirius_cosq_gport_stat_enable_set(int unit,
                                      bcm_gport_t   gport,
                                      int   enable)
{
    int rv = BCM_E_NONE;
    int current_enable;
    int current_base_queue;
    int base_queue;
    int num_cos;
    int max_queues;


    /* Convert gport to base queue number */
    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
    if (rv != BCM_E_NONE) return rv;

    max_queues = SOC_SBX_CFG(unit)->num_queues;

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "gport %d base q %d num_cos %d max_queues %d\n"),
                 gport, base_queue, num_cos, max_queues));

    if ( max_queues < base_queue + num_cos) return BCM_E_PARAM;

    if(enable == TRUE) {

        if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "soc_feature_cosq_gport_stat_ability Unavailable")));
            return BCM_E_UNAVAIL;
        }

        soc_sirius_counter_enable_get(unit, &current_base_queue, &current_enable);

        if (current_enable == TRUE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "Counter currently active\n")));
            return BCM_E_INTERNAL;
        }

        rv = soc_sirius_qm_counter_base_set(unit, base_queue, enable);

        if (rv == SOC_E_NONE ) rv = soc_sirius_counter_enable_set(unit, base_queue);

        return rv;

    } else if (enable == FALSE) {

        rv = soc_sirius_counter_enable_clear(unit);

    } else {
        return BCM_E_PARAM;
    }
    return rv;
}


int
bcm_sirius_cosq_gport_stat_enable_get(int unit,
        int   gport,
        int   *enable)
{
    int rv = BCM_E_NONE;
    int actual_base_queue;  /* Set to sirius */
    int soft_base_queue;
    int base_queue;
    int num_cos;

    if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "soc_feature_cosq_gport_stat_ability Unavailable")));
             return BCM_E_UNAVAIL;
    }

    /* Convert gport to base queue number */
    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
    if (rv != BCM_E_NONE) return rv;

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "gport %d base q %d num_cos %d\n"),
                 gport, base_queue, num_cos));


    rv = soc_sirius_qm_counter_base_get(unit, &actual_base_queue);

    if (rv != BCM_E_NONE) {
        return rv;
    }

    if (actual_base_queue != base_queue) {
	*enable = 0;
	return BCM_E_PARAM;
    }

    rv = soc_sirius_counter_enable_get(unit, &soft_base_queue, enable);

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "Enable %d,base_queue %d, actual_base_queue %d\n"),
                 *enable,base_queue,actual_base_queue));

    return rv;
}


/* This function is intended for clear of statistics */
int
bcm_sirius_cosq_gport_stat_set(int unit,
        int         base_queue,
        bcm_cos_queue_t   cosq,
        bcm_cosq_gport_stats_t stat,
        uint64  value)
{
    int rv = BCM_E_NONE;
    int block;
    int set;
    uint64 uuZero = COMPILER_64_INIT(0,0);

    if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "soc_feature_cosq_gport_stat_ability Unavailable")));
        return BCM_E_UNAVAIL;
    }

    block = 0; /* Since there is only one block on sirius force to zero */
    set = cosq;  /* cosq select the counter set */

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "soc_sbx_counter_port_set %d  %d %d cntr %d 0x%x%08x\n"),
                 unit,block,set,stat,COMPILER_64_HI(value), COMPILER_64_LO(value)));
    rv = soc_sbx_counter_set(unit,  block, set, stat, uuZero);

    return rv;
}

int
bcm_sirius_cosq_gport_stat_get(int unit,
                               int queue_base,
                               bcm_cos_queue_t   cosq,
                               bcm_cosq_gport_stats_t stat,
                               uint64   *value)
{
    int rv = BCM_E_NONE;
    int block;
    int set;

    if (!soc_feature(unit, soc_feature_cosq_gport_stat_ability)) {
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "soc_feature_cosq_gport_stat_ability Unavailable")));
        return BCM_E_UNAVAIL;
    }

    block = 0;    /* Force block to zero on sirius. Only one block */
    set = cosq;  /* cosq select the counter set */

    rv = soc_sbx_sirius_counter_port_get(unit, queue_base, block, set, stat, value);

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "soc_sbx_sirius_counter_port_get %d %d %d cntr %d 0x%x%08x\n"),
                 unit,block,set,stat,COMPILER_64_HI(*value), COMPILER_64_LO(*value)));

    return rv;
}

#define BCM_COSQ_GPORT_STATS_SLQ  CS_SLQ_ENABLE
#define BCM_COSQ_GPORT_STATS_GLOBAL CS_GBL_ENABLE
#define BCM_COSQ_GPORT_STATS_LATEST CS_FLUSHING
#define BCM_COSQ_GPORT_STATS_SET_ANY 0x100

/*
 * Function:
 *     bcm_sirius_slq_stats
 * Purpose:
 * Configure/Remove a queue for debug SLQ statistics
 *
 * Parameters:
 *     unit       - Device number
 *     queue_base - base queue of a cos group
 *     cosq       - specific queue to enable
 *     enable     - enable or disable an slq
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
bcm_sirius_slq_stats(uint32 unit,
		     int queue_base,
		     bcm_cos_queue_t cosq,
		     sbBool_t enable)
{
    int uStatus = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    uint32 slq = 0;


    /*
     * if any fields set,
     * enable queue_base for SLQ
     */
    if (enable) {
	/*
	 * if already enabled,
	 * keep track of resource count
	 */
	if (sir->cs.flags & CS_SLQ_ENABLE) {
	    BCM_IF_ERROR_RETURN(READ_QMB_SELECTED_Qr(unit, &slq));
	    if ((slq < queue_base) || (slq > queue_base + 16)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("profile_set: Unit %d SLQ %d not within range of"
		           " queue_base %d\n"), unit, slq, queue_base));
		return BCM_E_PARAM;
	    }
	    if (sir->cs.slq_q_active & (1 << cosq)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("profile_set: Unit %d Cosq %d currently active.\n"),
		           unit, cosq));
		return BCM_E_BUSY;
	    }
	} else {
	    /*
	     * enable
	     */
	    if (queue_base != -1) {
		BCM_IF_ERROR_RETURN(WRITE_QMB_SELECTED_Qr(unit, queue_base));
		sir->cs.flags |= CS_SLQ_ENABLE;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("profile_set: Unit %d SLQ queue_base %d not available\n"),
		           unit, queue_base));
		return BCM_E_PARAM;
	    }
	}
	/*
	 * enable the exact cosq within the
	 * queue_base range that is activated
	 */
	sir->cs.slq_q_active |= (1 << cosq);
	sir->cs.slq_usage++;
    } else {
	/*
	 * If active, then disable
	 * when the resource count
	 * reaches 0.
	 * If inactive, return
	 */
	if (sir->cs.flags & CS_SLQ_ENABLE) {
	    if ((sir->cs.slq_q_active & (1 << cosq)) == 0) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("profile_set: Unit %d Cosq %d currently inactive.\n"),
		           unit, cosq));
		return BCM_E_PARAM;
	    }
	    /*
	     * disable the exact cosq within
	     * the queue_base range that was activated
	     */
	    sir->cs.slq_q_active &= ~(1 << cosq);
	    if (--sir->cs.slq_usage == 0) {
		/*
		 * This flag stops the poll
		 * routine from checking
		 * the SLQ statistics, effectively
		 * shutting it down.
		 * When enabling, the queue
		 * in the register will be overwritten
		 * but it is good to keep the last
		 * queue around for debugging
		 */
		sir->cs.flags &= ~CS_SLQ_ENABLE;
	    }
	}
    }

    return uStatus;
}

#define SCHED_LOOKUP(_x)      _x
#define SCHED_EGRESS_CHECK(x) 1

static int bcm_sirius_get_egress_info(uint32 unit, bcm_gport_t gport, uint8 *bsize, uint8 *level,
				      uint16 *cntrId, int *intf)
{
    int rv, index = 0;
    int hgif = 0, po = 0, eg_n = -1;
    bcm_sbx_subport_info_t *sp_info = NULL;

    if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	index = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &index, &eg_n, NULL);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META("Invalid gport(0x%x). Unable to resolve to an intf \n"),
                       gport));
            return rv;
        }
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[index]);
	*cntrId = sp_info->egroup[eg_n].es_scheduler_level0_node;
        rv = bcm_sbx_port_get_intf_portoffset(unit, index, &hgif, &po);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META("Invalid gport(0x%x). Unable to resolve to an intf \n"),
                       gport));
            return rv;
        }
	*intf = hgif;
	*bsize = 9;
	*level = BCM_META0_FIFO_NUM;
    } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	index = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &index, &eg_n, NULL);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META("Invalid gport(0x%x). Unable to resolve to an intf \n"),
                       gport));
            return rv;
        }
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[index]);
        rv = bcm_sbx_port_get_intf_portoffset(unit, index, &hgif, &po);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META("Invalid gport(0x%x). Unable to resolve to an intf \n"),
                       gport));
            return rv;
        }
	*intf = hgif;
	*cntrId = sp_info->es_scheduler_level1_node[eg_n];
	*bsize = 8;
	*level = BCM_META0_SUBPORT_ID;
    } else if (BCM_GPORT_IS_SCHEDULER(gport)) {
	index = BCM_GPORT_SCHEDULER_GET(gport);
	if (SCHED_EGRESS_CHECK(index) == 0)
	    return BCM_E_PARAM;
	*cntrId = (uint16) SCHED_LOOKUP(index);
	*bsize = 8;
	*intf = 0;
	*level = BCM_META0_CHAN_ID;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	rv = bcm_sbx_port_get_intf_portoffset(unit, gport, &hgif, &index);
	if (rv != BCM_E_NONE) {
	    return rv;
	}

	*cntrId = *intf = hgif;
	*bsize = 3;
	*level = BCM_META0_INTF_NUM;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_sirius_ep_stat
 * Purpose:
 * Configure/Remove an EP Statistic
 *
 * Parameters:
 *     unit      - Device number
 *     gport     - Global Egress Port
 *     type      - Index into EP_STATS table
 *     enable    - flag to enable or disable statistic
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
static int
_bcm_sirius_ep_stat(uint32 unit,
		    bcm_gport_t gport,
		    uint8 type,
		    uint8 enable,
		    uint8 bit_size,
		    uint8 level,
		    uint16 cntrId,
		    int intf)
{
    soc_sbx_sirius_config_t *sir = SOC_SBX_CFG_SIRIUS(unit);
    ep_stats_ctrl_entry_t    ep_stats_ctrl;
    uint8 offset = 0, index = 0;
    int32  segment = -1;
    uint8 update = 0;

    /*
     * Locate segment. Segments are assigned per level,
     * so at most four segments will currently be configured
     * for the EP.
     */

    for (index=0; index <= SOC_MEM_INFO(unit, EP_STATS_CTRLm).index_max; index++) {
	BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, index, &ep_stats_ctrl));
	if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
	    (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf) == level))
	    segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_SEGMENTf);
	else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) != 0) &&
		 (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_META0_SELf) == level))
	    segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_SEGMENTf);
	if (segment >= 0)
	    goto ep_seg_done;
    }

ep_seg_done:

    /*
     * Locate an available or pre-existing entry to update
     * If removing statistics, the interface should already
     * exist in the table.
     */

    BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, type, &ep_stats_ctrl));

    /*
     * Program the Egress Processor CS table Entry
     */

    if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) == 0) &&
	(soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) == 0)) {

	/*
	 * Fill in new entry in STAT0
	 */

	offset = 0;

    } else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
	       (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf) == level)) {
	/*
	 * Update existing entry in STAT0
	 */
	offset = 0;

    } else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) != 0) &&
	       (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_META0_SELf) == level)) {
	/*
	 * Update existing entry in STAT1
	 */

	offset = STAT1_SEGMENTf - STAT0_SEGMENTf;

    } else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
	       (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) == 0)) {
	/*
	 * Fill in new entry in STAT1
	 */
	offset = STAT1_SEGMENTf - STAT0_SEGMENTf;

    } else {
	/*
	 * No resource currently available
	 */
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META("ep_stat: unit %d no segments available for Egress Stat"
	           " type %d\n"), unit, type));
	return BCM_E_RESOURCE;
    }

    if (enable) {

	/*
	 * If no segment has been found for the specific type,
	 * then we need to allocate one.
	 */

	if (segment < 0) {
	    if (soc_sbx_sirius_get_segment(unit, CU_NUM_EP, &segment) == SOC_E_UNAVAIL) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("gport_profile_set: unit %d no segments available"
		           " for Egress Stat level %d\n"), unit, level));
		return BCM_E_UNAVAIL;
	    }
	}

	/*
	 * Keep track of stat count
	 */
	if (sir->cs.ep_type_use[type][(offset) ? 1 : 0]++ == 0)
	    update = 1;

	soc_sbx_sirius_create_group(unit, segment, sir->cs.segment[segment].cu_num, cntrId, 1);

    } else {
	if (segment < 0 || segment >=CS_NUM_SEGMENTS) {
	    return BCM_E_NONE;
	}

	soc_sbx_sirius_remove_group(unit, segment, sir->cs.segment[segment].cu_num, cntrId, 1);
	if (--sir->cs.ep_type_use[type][(offset) ? 1 : 0] == 0) {
	    /*
	     * Clear the EP stat
	     */

	    update = 1;
	    level = 0;
	    bit_size = 0;
	    segment = 0;
	}
    }

    /*
     * Fill in Entry
     */

    if (update) {
	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_SEGMENTf + offset, segment);

	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf + offset, level);
	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f + offset, bit_size);
	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_OFFSET0f + offset, 0);

	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META1_SELf + offset, 0);
	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH1f + offset, 0);
	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_OFFSET1f + offset, 0);

	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_ADJUSTf + offset, 0);
	soc_mem_field32_set(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_PER_FRAME_ADJf + offset, 0);


	BCM_IF_ERROR_RETURN(WRITE_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, type, &ep_stats_ctrl));
    }

    return SOC_E_NONE;
}

static int 
_bcm_sirius_qm_color_to_dp(uint32 unit, uint32 color, uint32 *dp) 
{
    uint32 regval = 0;

    SOC_IF_ERROR_RETURN(READ_QMA_WRED_CONFIGr(unit, &regval));

    if (soc_reg_field_get(unit, QMA_WRED_CONFIGr, regval, DP_MAPPING_0f) == (color %4)) {
	*dp = 0;
    } else if (soc_reg_field_get(unit, QMA_WRED_CONFIGr, regval, DP_MAPPING_1f) == (color %4)) {
	*dp = 1;
    } else if (soc_reg_field_get(unit, QMA_WRED_CONFIGr, regval, DP_MAPPING_2f) == (color %4)) {
	*dp = 2;
    } else {
	*dp = 3;
    }

    *dp += ((color>>2)<<2);

    return(BCM_E_NONE);
}

/*
 * Function:
 *     bcm_sirius_qm_interface_stat
 * Purpose:
 * Configure an Interface stat
 *
 * Parameters:
 *     unit      - Device number
 *     gport     - Physical gport
 *     type      - Index into Type table
 *     color_map - DP values to enable/disable
 *     shared    - Share segment or create private
 *                 stats buffer
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
static int
_bcm_sirius_qm_interface_stat(uint32 unit,
			      bcm_gport_t gport,
			      int intf,
			      uint8 type,
			      uint8 color_map,
			      uint32 shared)
{
    int uStatus = BCM_E_NOT_FOUND;
    soc_sbx_sirius_config_t *sir = SOC_SBX_CFG_SIRIUS(unit);
    statscfg_entry_t   statsCfg;
    int idx = 0, dp = 0;
    int32 segment = -1;
    int32 found = FALSE;

    /*
     * The following (shared) code is solely intended
     * to retrieve a shared segment number
     * if one pre-exists, or to allocate it if not
     */

    if (shared) {
	/*
	 * search the statscfg table for physical i/f segment
	 * All interface stats reside in RECORD2
	 */
	for (idx=0; idx < 128; idx++) {
	    BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));
	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD2_EVENT_SELf)) {
		segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD2_SEGMENT_SELf);
		if ((sir->cs.segment[segment].shared & 0xFF) != 0) {
		    found = TRUE;
		    break;
		}
	    }
	}

	if (found == FALSE) {
	    /*
	     * No shared interface currently configured,
	     * allocate a segment and mark as shared.
	     * If color_map is empty, then this is a clear
	     * request and no shared segment need be allocated
	     */

	    if (color_map) {
		if (soc_sbx_sirius_get_segment(unit, CU_NUM_PHYSICAL, &segment) == SOC_E_UNAVAIL) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META("intf_stat: unit %d no segments available for Ingress"
		               " Physical Stat type %d\n"), unit, type));
		    return BCM_E_RESOURCE;
		}
		/*
		 * set segment configured for shared access per color
		 */
		for (dp=0; dp<8; dp++) {
		    if (color_map & (1 << dp))
			sir->cs.segment[segment].shared |= (1 << dp);
		}
	    }
	}
    }

    /*
     * At this point, we should have a segment number
     * for any colors shared per intf num. If shared is
     * unset, then we do not have a segment number yet.
     */

    /*
     * Enable/Disable Add/Drop DP statistics per type
     */

    for (dp=0; dp < 8; dp++) {

	idx = (type << 3) + dp;
	BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));

	/*
	 * If color set in map
	 */
	if ((color_map & (1 << dp)) != 0) {

	    /*
	     * Check if field is currently active
	     */
	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD2_EVENT_SELf) == 0) {

		if (segment == -1) {
		    /*
		     * We need to allocate a segment for this statistic
		     */
		    if (soc_sbx_sirius_get_segment(unit, CU_NUM_PHYSICAL, &segment) == SOC_E_UNAVAIL) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META("intf_stat: unit %d no segments available for"
			           " Ingress Physical Stat type %d\n"), unit, type));
			return BCM_E_RESOURCE;
		    }
		}
		/*
		 * Enable color
		 */
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD2_ADDR_SELf, (uint32) qm_addr_interface);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD2_EVENT_SELf, 0x1);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD2_SEGMENT_SELf, segment);
		BCM_IF_ERROR_RETURN(WRITE_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));
	    } else {
		/* It has already been configured; don't do it again */
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("intf_stat: unit %d type %d color %d entry already"
		           " configured\n"),
		           unit,
		           type,
		           dp));
		return BCM_E_CONFIG;
	    }
	    /*
	     * Whether we create this field or update our index count,
	     * we need to keep track of segment usage in order
	     * to clean resources when the usage count drops
	     * to 0
	     */
	    uStatus = soc_sbx_sirius_create_group(unit, segment, sir->cs.segment[segment].cu_num, intf, 1);
	    if (uStatus == BCM_E_NONE)
		sir->cs.statscfg_use[idx][qm_addr_interface]++;

	} else {

	    /*
	     * Disable Color
	     */

	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD2_EVENT_SELf) == 1) {
		segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg,RECORD2_SEGMENT_SELf);
		uStatus = soc_sbx_sirius_remove_group(unit, segment, sir->cs.segment[segment].cu_num, intf, 1);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("intf_stat: unit %d type %d color %d entry not"
		           " configured\n"), unit, type, dp));
		return BCM_E_PARAM;
	    }

	    /*
	     * We don't want to remove the entry if other resources
	     * believe that the entry is still in use.
	     */

	    if ((uStatus == BCM_E_NONE) && (--sir->cs.statscfg_use[idx][qm_addr_interface] == 0)) {

		/*
		 * Check to see if the
		 * entry is still configured
		 */

		soc_mem_field32_set(unit, STATSCFGm, &statsCfg, RECORD2_ADDR_SELf, 0);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg, RECORD2_EVENT_SELf, 0x0);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg, RECORD2_SEGMENT_SELf, 0);
		BCM_IF_ERROR_RETURN(WRITE_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));
	    }
	}
    }

    return uStatus;
}


/*
 * Function:
 *     bcm_sirius_qm_queue_stat
 * Purpose:
 * Configure an Queue stat
 *
 * Parameters:
 *     unit      - Device number
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
static int
_bcm_sirius_qm_queue_stat(uint32 unit,
			  bcm_gport_t gport,
			  bcm_gport_t lgl_gport,
			  int intf,
			  uint32 cntrId,
			  uint8 type,
			  uint8 color_map,
			  uint32 shared)
{
    int uStatus = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir = SOC_SBX_CFG_SIRIUS(unit);
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    statscfg_entry_t   statsCfg;
    int idx = 0, dp = 0;
    int32 segment = -1, new_seg = -1, curr_seg = -1, unused = -1;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[cntrId];

    if (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_IN_USE) {
	LOG_WARN(BSL_LS_BCM_COSQ,
	         (BSL_META("QID %d not configured\n"),
	          cntrId));
	return BCM_E_CONFIG;
    }

    /*
     * search the statscfg table for shared segment
     * All queue stats reside in RECORD0
     */
    for (idx=0; idx <= SOC_MEM_INFO(unit, STATSCFGm).index_max; idx++) {
	BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));
	if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf)) {
	    new_seg = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
	    if (shared && ((sir->cs.segment[new_seg].shared & 0xFF00) != 0)) {
		break;
	    }
	    new_seg = -1;
	}
    }
    
    if (new_seg == -1) {
	/*
	 * Allocate a segment if none still exist.
	 * If color_map is empty, then this is a clear
	 * request and no segment need be allocated
	 */
	if (color_map) {
	    if (soc_sbx_sirius_get_segment(unit, CU_NUM_QUEUE, &new_seg) == SOC_E_UNAVAIL) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("queue_stat: unit %d no segments available"
		           " for Ingress Queue Stat type %d\n"), unit, type));
		return BCM_E_RESOURCE;
	    }
	    unused = new_seg;
	    /*
	     * set segment configured for shared access per interface, if necessary
	     */
	    if (shared)
		sir->cs.segment[new_seg].shared |= ((1 << intf) << 8);
	}
    }
    
    /*
     * At this point, we should have a segment number
     * for any colors  per intf num.
     */

    /*
     * Enable/Disable Add/Drop DP statistics per type
     */

    for (dp=0; dp < 8; dp++) {

	BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, dp, &statsCfg));
	if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf) != 0) {
	    curr_seg = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
	} else {
	    curr_seg = -1;
	}

	idx = (type << 3) + dp;
	BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));

	/*
	 * If color set in map
	 */
	if ((color_map & (1 << dp)) != 0) {

	    /*
	     * Check if field is currently active
	     */
	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf) == 0) {

		/*
		 * Enable color
		 */
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD0_ADDR_SELf, (uint32) qm_addr_queue);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD0_EVENT_SELf, 0x1);
		if (curr_seg == -1) {
		    unused = -1;
		    segment = new_seg;
		    soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD0_SEGMENT_SELf, segment);
 		} else {
		    soc_mem_field32_set(unit, STATSCFGm, &statsCfg,RECORD0_SEGMENT_SELf, curr_seg);
		    segment = curr_seg;
		}
		BCM_IF_ERROR_RETURN(WRITE_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));
	    } else {
		segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
	    }

	    /*
	     * Whether we create this field or update our index count,
	     * we need to keep track of segment usage in order
	     * to clean resources when the usage count drops
	     * to 0
	     */
	    if ((p_qstate->enq_stat_in_use & (BCM_INT_SBX_STAT_ENQUEUE << dp)) == 0) {
		uStatus = soc_sbx_sirius_create_group(unit, segment, sir->cs.segment[segment].cu_num, cntrId, 1);
		if (uStatus == BCM_E_NONE) {
		    sir->cs.statscfg_use[idx][qm_addr_queue]++;
		    p_qstate->enq_stat_in_use |= (BCM_INT_SBX_STAT_ENQUEUE << dp);
		}
	    }

	} else {

	    /*
	     * Disable Color if enabled
	     */

	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf) == 1) {
		segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg,RECORD0_SEGMENT_SELf);
		
		/* Only remove shared if shared flag set and vice versa */
		if (!shared && ((sir->cs.segment[segment].shared & 0xFF00) != 0)) {
		    continue;
		} else if (shared && ((sir->cs.segment[segment].shared & 0xFF00) == 0)) {
		    continue;
		}
		
		if ((p_qstate->enq_stat_in_use & (BCM_INT_SBX_STAT_ENQUEUE << dp)) != 0) {
		    uStatus = soc_sbx_sirius_remove_group(unit, segment, sir->cs.segment[segment].cu_num, cntrId, 1);
		    
		    /*
		     * We don't want to remove the entry if other resources
		     * believe that the entry is still in use.
		     */
		    
		    if (uStatus == BCM_E_NONE) {
			sir->cs.statscfg_use[idx][qm_addr_queue]--;
			p_qstate->enq_stat_in_use &= ~(BCM_INT_SBX_STAT_ENQUEUE << dp);
		    }
		}
	    }

	    if ((curr_seg == -1) || (sir->cs.statscfg_use[dp][qm_addr_queue] == 0)) {
		
		/*
		 * Remove the entry from the STATSCFG table
		 */
		
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg, RECORD0_ADDR_SELf, 0);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf, 0x0);
		soc_mem_field32_set(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf, 0);
		BCM_IF_ERROR_RETURN(WRITE_STATSCFGm(unit, MEM_BLOCK_ANY, idx, &statsCfg));
		sir->cs.statscfg_use[idx][qm_addr_queue] = 0;
	    }
	}
    }

    if (unused != -1) {
	uStatus = soc_sbx_sirius_free_segment(unit, unused);
    }
    return uStatus;
}

/*
 * Function:
 *     bcm_sirius_qm_dequeue_stat
 * Purpose:
 * Configure a Dequeue stat
 *
 * Parameters:
 *     unit      - Device number
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
static int
_bcm_sirius_qm_dequeue_stat(uint32 unit,
			    uint32 cntrId, 
			    int32 enable)
{
    int uStatus = BCM_E_NONE;
    soc_sbx_sirius_config_t    *sir = SOC_SBX_CFG_SIRIUS(unit);
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    uint32                      regval = 0;
    int32                       segment = -1;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[cntrId];

    if (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_IN_USE) {
	LOG_WARN(BSL_LS_BCM_COSQ,
	         (BSL_META("QID %d not configured\n"),
	          cntrId));
	return BCM_E_CONFIG;
    }

    if (enable) {
	if ((p_qstate->deq_stat_in_use & BCM_INT_SBX_STAT_DEQUEUE) != 0) {
	    return BCM_E_NONE;
	}

	/*
	 * Enable Dequeue statistics and set default segment number
	 */
	BCM_IF_ERROR_RETURN(READ_QM_CS_CONFIG0r(unit, &regval));
	
	if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf) == 0) {
	    if (soc_sbx_sirius_get_segment(unit, CU_NUM_DEQUEUE, &segment) == SOC_E_UNAVAIL) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META("dequeue_stat: unit %d no segments available for Dequeue Stat\n"),
		           unit));
		return BCM_E_RESOURCE;
	    }
	    
	    soc_reg_field_set(unit, QM_CS_CONFIG0r, &regval, DEQ_CS_STATS_ENf, 1);
	    soc_reg_field_set(unit, QM_CS_CONFIG0r, &regval, CS_DEQ_SEGMENTf, segment);
	    BCM_IF_ERROR_RETURN(WRITE_QM_CS_CONFIG0r(unit, regval));
	} else {
	    segment = soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf);
	}

	uStatus = soc_sbx_sirius_create_group(unit, segment, sir->cs.segment[segment].cu_num, cntrId, 1);
	if (uStatus == BCM_E_NONE) {
	    p_qstate->deq_stat_in_use |= BCM_INT_SBX_STAT_DEQUEUE;
	}
    } else {
	if ((p_qstate->deq_stat_in_use & BCM_INT_SBX_STAT_DEQUEUE) == 0) {
	    return BCM_E_NONE;
	}

	BCM_IF_ERROR_RETURN(READ_QM_CS_CONFIG0r(unit, &regval));
	if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf) == 0) {
	    return BCM_E_NONE;
	}

	segment = soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf);
	uStatus = soc_sbx_sirius_remove_group(unit, segment, sir->cs.segment[segment].cu_num, cntrId, 1);
	if (uStatus == BCM_E_NONE) {
	    p_qstate->deq_stat_in_use &= ~BCM_INT_SBX_STAT_DEQUEUE;
	}
    }

    return uStatus;
}

#define CS_STAT_CHECK(_a, _b, _c)			\
    { int _i = 0, _bits = 0;				\
	for(_i=_a; _i <= _b; _i++)			\
	    _bits += SHR_BITGET(profile.stats, _i);	\
	_c = (_bits) ? 1 : 0;				\
    }

int
_bcm_sirius_cosq_gport_stat_config_set(int unit,
				       bcm_gport_t gport,
				       bcm_gport_t lgl_gport,
				       bcm_cos_queue_t cosq,
				       uint32 flags,
				       bcm_cosq_gport_stat_profile_t profile)
{
    int uStatus = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    qm_statscfg_addr_t qm_addr_sel = 0;
    uint8 color_map = 0, enable = 0, deq = 0;
    uint8 green = 0, yellow = 0, red = 0, black = 0, drop = 0;
    uint8 fd_all = 0, fd_green = 0, fd_yellow = 0, fd_red = 0, fd_mc = 0;
    uint32 shared = (flags & BCM_COSQ_GPORT_STATS_SHARED) ? 1 : 0;
    uint8 bit_size = 0, level = 0;
    uint16 cntrId = 0;
    uint8 type = 0, type2 = 0;
    int intf = 0, index = 0;
    uint32 queue_base = -1;
    uint32 dp = 0, regval = 0;
    int refresh1=0,refresh2=0;

    if (!SOC_IS_SBX_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    if ((gport == BCM_GPORT_TYPE_NONE) &&
	(lgl_gport == BCM_GPORT_TYPE_NONE)) {
	
	/*
	 * Admission Control
	 */
	
	CS_STAT_CHECK(bcmCosqGportOutLimitDroppedPkts,bcmCosqGportOutLimitDroppedPkts, fd_all);
	CS_STAT_CHECK(bcmCosqGportOutLimitDroppedGreenPkts,bcmCosqGportOutLimitDroppedGreenPkts, fd_green);
	CS_STAT_CHECK(bcmCosqGportOutLimitDroppedYellowPkts,bcmCosqGportOutLimitDroppedYellowPkts, fd_yellow);
	CS_STAT_CHECK(bcmCosqGportOutLimitDroppedRedPkts,bcmCosqGportOutLimitDroppedRedPkts, fd_red);
	CS_STAT_CHECK(bcmCosqGportOutLimitDroppedMulticastPkts,bcmCosqGportOutLimitDroppedMulticastPkts, fd_mc);
	
	SOC_IF_ERROR_RETURN(READ_FD_CONFIGr(unit, &regval));

	/* 
	 * Check if already configured 
	 */
	type = soc_reg_field_get(unit,FD_CONFIGr,regval,FD_PER_PORT_DROP_COUNT1_SELf);
	type2 = soc_reg_field_get(unit,FD_CONFIGr,regval,FD_PER_PORT_DROP_COUNT2_SELf);

	index = fd_all + fd_green + fd_yellow + fd_red + fd_mc;

	if ((index == 0) && (type == 0x7) && (type2 == 0x7)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "%s: unit %d Admission Control resources not configured\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_PARAM;
	} else if (index > 2) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "%s: unit %d Admission Control resources not available\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_PARAM;
	}
	
	/*
	 * Determine if register is already configured
	 */

	if (fd_all && (type == FD_ALL)) refresh1 = 1;
	if (fd_all && (type2 == FD_ALL)) refresh2 = 1;
	if (fd_green && (type == FD_GREEN)) refresh1 = 1;
	if (fd_green && (type2 == FD_GREEN)) refresh2 = 1;
	if (fd_yellow && (type == FD_YELLOW)) refresh1 = 1;
	if (fd_yellow && (type2 == FD_YELLOW)) refresh2 = 1;
	if (fd_red && (type == FD_RED)) refresh1 = 1;
	if (fd_red && (type2 == FD_RED)) refresh2 = 1;
	if (fd_mc && (type == FD_MC)) refresh1 = 1;
	if (fd_mc && (type2 == FD_MC)) refresh2 = 1;

	if (refresh1 == 0) type = 0x7;
	if (refresh2 == 0) type2 = 0x7;

	soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT1_SELf, type);
	soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT2_SELf, type2);

	refresh1 = refresh2 = 0;

	/* 
	 * Map new values and clear unused values
	 */

	if (fd_all && (type != FD_ALL) && (type2 != FD_ALL)) {
	    if (type == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT1_SELf, FD_ALL);
		refresh1 = 1;
		type = FD_ALL;
	    } else if (type2 == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT2_SELf, FD_ALL);
		refresh2 = 1;
		type2 = FD_ALL;
	    }
	} 
	
	if (fd_green && (type != FD_GREEN) && (type2 != FD_GREEN)) {
	    if (type == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT1_SELf, FD_GREEN);
		refresh1 = 1;
		type = FD_GREEN;
	    } else if (type2 == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT2_SELf, FD_GREEN);
		refresh2 = 1;
		type2 = FD_GREEN;
	    }
	}
	
	if (fd_yellow && (type != FD_YELLOW) && (type2 != FD_YELLOW)) {
	    if (type == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT1_SELf, FD_YELLOW);
		refresh1 = 1;
		type = FD_YELLOW;
	    } else if (type2 == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT2_SELf, FD_YELLOW);
		refresh2 = 1;
		type2 = FD_YELLOW;
	    }
	}
	
	if (fd_red && (type != FD_RED) && (type2 != FD_RED)) {
	    if (type == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT1_SELf, FD_RED);
		refresh1 = 1;
		type = FD_RED;
	    } else if (type2 == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT2_SELf, FD_RED);
		refresh2 = 1;
		type2 = FD_RED;
	    }
	}
	
	if (fd_mc && (type != FD_MC) && (type2 != FD_MC)) {
	    if (type == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT1_SELf, FD_MC);
		refresh1 = 1;
		type = FD_MC;
	    } else if (type2 == 0x7) {
		soc_reg_field_set(unit, FD_CONFIGr, &regval, FD_PER_PORT_DROP_COUNT2_SELf, FD_MC);
		refresh2 = 1;
		type2 = FD_MC;
	    }
	}
	
	SOC_IF_ERROR_RETURN(WRITE_FD_CONFIGr(unit, regval));

	if (refresh1 == 1) {
	    if ((uStatus = soc_sbx_sirius_process_fd_drop_stats(unit, 1)) != SOC_E_NONE) {
		LOG_VERBOSE(BSL_LS_BCM_COUNTER,
                            (BSL_META_U(unit,
                                        "%s: unit=%d  ADMISSION CONTROL (1) CLEAR STATS ERROR (%d)\n"),
                             FUNCTION_NAME(), unit, uStatus));
	    }
	}
	if (refresh2 == 1) {
	    if ((uStatus = soc_sbx_sirius_process_fd_drop_stats(unit, 2)) != SOC_E_NONE) {
		LOG_VERBOSE(BSL_LS_BCM_COUNTER,
                            (BSL_META_U(unit,
                                        "%s: unit=%d  ADMISSION CONTROL (2) CLEAR DROP STATS ERROR (%d)\n"),
                             FUNCTION_NAME(), unit, uStatus));
	    }
	}
	return BCM_E_NONE;
    }

    /*
     * Force disable SLQ STATS in CS mode 
     */
    flags &= ~BCM_COSQ_GPORT_STATS_SLQ;

    /*
     * These stats are reported via
     * Global stats only. Therefore,
     * as long as any of them are set,
     * enable capture of global stats
     */

    CS_STAT_CHECK(bcmCosqGportGreenDroppedPkts,bcmCosqGportGreenDroppedBytes, green);
    CS_STAT_CHECK(bcmCosqGportYellowDroppedPkts,bcmCosqGportYellowDroppedBytes, yellow);
    CS_STAT_CHECK(bcmCosqGportRedDroppedPkts,bcmCosqGportRedDroppedBytes, red);
    CS_STAT_CHECK(bcmCosqGportBlackDroppedPkts,bcmCosqGportBlackDroppedBytes, black);
    CS_STAT_CHECK(bcmCosqGportDequeuedPkts,bcmCosqGportDequeuedBytes, deq);

    if ((green || yellow || red || black) || ( deq && (flags & BCM_COSQ_GPORT_STATS_GLOBAL)))
	sir->cs.flags |= CS_GBL_ENABLE;
    else
	sir->cs.flags &= ~CS_GBL_ENABLE;

    /*
     * Check for statistics on Egress port
     */

    if (BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	BCM_GPORT_IS_EGRESS_MODPORT(gport)) {

	/*
	 * This determines cntrId, level, and bit_size from
	 * GPORT modules
	 */

	BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, gport, &bit_size, &level, &cntrId, &intf));

	CS_STAT_CHECK(bcmCosqGportOutPkts, bcmCosqGportOutBytes, enable);

	for(type=0; type < 16; type++)
	    if (sir->type_res_flags[PRED_TYPE_EP][type] & (1 << (intf + 4)))
		BCM_IF_ERROR_RETURN(_bcm_sirius_ep_stat(unit, gport, type, enable, bit_size, level, cntrId, intf));
    }

    /*
     * Check if we're focusing on Ingress statistics
     */

    if (BCM_GPORT_IS_MODPORT(gport) ||
	BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_MCAST(lgl_gport)) {

	uStatus = bcm_sbx_port_get_intf_portoffset(unit, gport, &intf, &index);
	if (uStatus != BCM_E_NONE) {
	    return uStatus;
	}

	/*
	 * We are dealing with INGRESS Statistics
	 */

	CS_STAT_CHECK(bcmCosqGportGreenAcceptedPkts, bcmCosqGportGreenAcceptedBytes, green);
	CS_STAT_CHECK(bcmCosqGportYellowAcceptedPkts, bcmCosqGportYellowAcceptedBytes, yellow);
	CS_STAT_CHECK(bcmCosqGportRedAcceptedPkts, bcmCosqGportRedAcceptedBytes, red);
	CS_STAT_CHECK(bcmCosqGportBlackAcceptedPkts, bcmCosqGportBlackAcceptedBytes, black);

	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_GREEN, &dp));
	color_map |= (green  << dp);
	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_YELLOW, &dp));
	color_map |= (yellow << dp);
	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_RED, &dp));
	color_map |= (red    << dp);
	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_BLACK, &dp));
	color_map |= (black  << dp);

	CS_STAT_CHECK(bcmCosqGportGreenDiscardDroppedPkts, bcmCosqGportGreenDiscardDroppedBytes, green);
	CS_STAT_CHECK(bcmCosqGportYellowDiscardDroppedPkts, bcmCosqGportYellowDiscardDroppedBytes, yellow);
	CS_STAT_CHECK(bcmCosqGportRedDiscardDroppedPkts, bcmCosqGportRedDiscardDroppedBytes, red);
	CS_STAT_CHECK(bcmCosqGportBlackDiscardDroppedPkts, bcmCosqGportBlackDiscardDroppedBytes, black);

	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_GREEN, &dp));
	color_map |= (green  << dp);
	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_YELLOW, &dp));
	color_map |= (yellow << dp);
	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_RED, &dp));
	color_map |= (red    << dp);
	BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_BLACK, &dp));
	color_map |= (black  << dp);

	if (lgl_gport != BCM_GPORT_TYPE_NONE) {
	    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport)) {
		qm_addr_sel = qm_addr_queue;

		if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		}

		/*
		 * If deq set in profile, enable stats for deq, otherwise disable stats if enabled
		 */
		
		if (!shared) {
		    uStatus = _bcm_sirius_qm_dequeue_stat(unit, queue_base + cosq, deq);

		    if (uStatus != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "Dequeue stat %s failed for 0x%x\n"),
			           (deq) ? "enable" : "disable",
			           lgl_gport));
			return uStatus;
		    }
		}		

		/*
		 * SLQ stats are available through
		 * Selected Queue counter registers.
		 * If any of them are configured, and
		 * we have a queue_base available,
		 * then enable the SLQ capture.
		 */

		if (flags & BCM_COSQ_GPORT_STATS_SLQ) {
		    CS_STAT_CHECK(bcmCosqGportGreenAcceptedPkts,bcmCosqGportGreenDiscardDroppedBytes, green);
		    CS_STAT_CHECK(bcmCosqGportYellowAcceptedPkts,bcmCosqGportYellowDiscardDroppedBytes, yellow);
		    CS_STAT_CHECK(bcmCosqGportRedAcceptedPkts,bcmCosqGportRedDiscardDroppedBytes, red);
		    CS_STAT_CHECK(bcmCosqGportBlackAcceptedPkts,bcmCosqGportBlackDiscardDroppedBytes, black);
		    CS_STAT_CHECK(bcmCosqGportNonWredDroppedPkts,
				  bcmCosqGportOverSubscribeGuaranteedDroppedBytes, drop);
		    if (green || yellow || red || black || drop)
			enable = 1;
		    else
			enable = 0;

		    BCM_IF_ERROR_RETURN(bcm_sirius_slq_stats(unit, queue_base, cosq, enable));
		}

	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "gport_profile_set: unit %d unknown Logical Interface"
		                       " %x\n"), unit, lgl_gport));

		/*
		 * Need to support stats_label and histogram mode, but for now,
		 * adding it here to appease coverity.
		 */
		qm_addr_sel = qm_addr_stats_label;
	    }
	} else {
	    /*
	     * Physical interface
	     */
	    qm_addr_sel = qm_addr_interface;
	}

	switch(qm_addr_sel) {
	    case qm_addr_interface:

		for (type=0; type<16; type++)
		    if ((sir->type_res_flags[PRED_TYPE_RB][type] & (1 << (intf + 4))) != 0)
			uStatus = _bcm_sirius_qm_interface_stat(unit, gport, intf, type, color_map, shared);
		break;

	    case qm_addr_queue:

		for (type=0; type<16; type++)
		    if ((sir->type_res_flags[PRED_TYPE_RB][type] & (1 << (intf + 4))) != 0)
			uStatus = _bcm_sirius_qm_queue_stat(unit, gport, lgl_gport, intf, queue_base + cosq, type, color_map, shared);

		break;

	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "gport_profile_set: unit %d unknown mode\n"),
		           unit));
		return BCM_E_PARAM;
	}
    }

    return uStatus;
}

#define CS_STAT_COLOR_SET(_a)						\
    {									\
	switch(_a) {							\
	    case 0:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportGreenAcceptedPkts, 2); \
		break;							\
	    case 1:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportYellowAcceptedPkts, 2); \
		break;							\
	    case 2:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportRedAcceptedPkts, 2); \
		break;							\
	    case 3:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportBlackAcceptedPkts, 2); \
		break;							\
	    case 4:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportGreenDiscardDroppedPkts, 2); \
		break;							\
	    case 5:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportYellowDiscardDroppedPkts, 2); \
		break;							\
	    case 6:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportRedDiscardDroppedPkts, 2);	\
		break;							\
	    case 7:							\
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportBlackDiscardDroppedPkts, 2); \
		break;							\
	}								\
    }


#define CHECK_EP_STAT(_a, _b, _c, _l, _id)				\
    {									\
	uint8 _meta0 = 0, _segment = 0;				\
	if (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, _a) != 0) { \
	    _segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, _b); \
	    _meta0 = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, _c); \
	    if ((_meta0 == _l) && COMPILER_64_BITTEST(sir->cs.segment[_segment].cntrIdMap, _id)) \
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutPkts, 2); \
	}								\
    }


int
_bcm_sirius_cosq_gport_stat_config_get(int unit,
				       bcm_gport_t gport,
				       bcm_gport_t lgl_gport,
				       bcm_cos_queue_t cosq,
				       uint32 flags,
				       bcm_cosq_gport_stat_profile_t *profile)
{
    int rv = BCM_E_NONE;
    soc_control_t           *soc = SOC_CONTROL(unit);
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    statscfg_entry_t   statsCfg;
    ep_stats_ctrl_entry_t    ep_stats_ctrl;
    qm_statscfg_addr_t qm_addr_sel = qm_addr_interface;
    uint8 dp = 0;

    uint32 shared = (flags & BCM_COSQ_GPORT_STATS_SHARED) ? 1 : 0;
    int32  segment = -1;
    uint16 cntrId = 0;
    uint8 type = 0;
    int intf = 0, index = 0;
    uint32 queue_base = -1, regval = 0;
    uint8 bit_size = 0, level = 0;

    if (!SOC_IS_SBX_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    /* Clear profile */
    SHR_BITCLR_RANGE(profile->stats, 0, bcmCosqGportStatCount);

    if ((gport == BCM_GPORT_TYPE_NONE) &&
	(lgl_gport == BCM_GPORT_TYPE_NONE)) {

	if (soc->counter_interval != 0) {
	    /* Admission Control */
	    
	    SOC_IF_ERROR_RETURN(READ_FD_CONFIGr(unit, &regval));
	    
	    type = soc_reg_field_get(unit,FD_CONFIGr,regval,FD_PER_PORT_DROP_COUNT1_SELf);
	    if (type == FD_ALL) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedPkts, 2);
	    } else if (type == FD_GREEN) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedGreenPkts, 2);
	    } else if (type == FD_YELLOW) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedYellowPkts, 2);
	    } else if (type == FD_RED) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedRedPkts, 2);
	    } else if (type == FD_MC) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedMulticastPkts, 2);
	    }
			    
	    type = soc_reg_field_get(unit,FD_CONFIGr,regval,FD_PER_PORT_DROP_COUNT2_SELf);
	    if (type == FD_ALL) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedPkts, 2);
	    } else if (type == FD_GREEN) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedGreenPkts, 2);
	    } else if (type == FD_YELLOW) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedYellowPkts, 2);
	    } else if (type == FD_RED) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedRedPkts, 2);
	    } else if (type == FD_MC) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportOutLimitDroppedMulticastPkts, 2);
	    }
	    return BCM_E_NONE;
	}
	
	return BCM_E_PARAM;
    }

    if (sir->cs.flags & BCM_COSQ_GPORT_STATS_GLOBAL) {
	SHR_BITSET_RANGE(profile->stats, bcmCosqGportGreenDroppedPkts, 2);
	SHR_BITSET_RANGE(profile->stats, bcmCosqGportYellowDroppedPkts, 2);
	SHR_BITSET_RANGE(profile->stats, bcmCosqGportRedDroppedPkts, 2);
	SHR_BITSET_RANGE(profile->stats, bcmCosqGportBlackDroppedPkts, 2);
	SHR_BITSET_RANGE(profile->stats, bcmCosqGportDequeuedPkts, 2);
    }

    /*
     * Handle Egress statistics appropriately.
     */

    if (BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	for (index=0; index <= SOC_MEM_INFO(unit, EP_STATS_CTRLm).index_max; index++) {
	    if ((sir->type_res_flags[PRED_TYPE_EP][index] & PRED_TYPE_FLAGS_INTF) == 0)
		continue;
	    BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, index, &ep_stats_ctrl));
	    BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, gport, &bit_size, &level, &cntrId, &intf));
	    CHECK_EP_STAT(STAT0_LENGTH0f, STAT0_SEGMENTf, STAT0_META0_SELf, level, cntrId);
	    CHECK_EP_STAT(STAT1_LENGTH0f, STAT1_SEGMENTf, STAT1_META0_SELf, level, cntrId);
	}
    }

    /*
     * Check if we're focusing on Ingress statistics
     */

    if (BCM_GPORT_IS_MODPORT(gport) ||
	BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport) ||
	BCM_GPORT_IS_MCAST(lgl_gport)) {

	/*
	 * Check Dequeue Statistics 
	 */

	if (!shared) {
	    BCM_IF_ERROR_RETURN(READ_QM_CS_CONFIG0r(unit, &regval));
	    if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf) == 1) {
		SHR_BITSET_RANGE(profile->stats, bcmCosqGportDequeuedPkts, 2);
	    }
	}

	rv = bcm_sbx_port_get_intf_portoffset(unit, gport, &intf, &index);
	if (rv != BCM_E_NONE) {
	    return rv;
	}

	/*
	 * We are dealing with INGRESS Statistics
	 */

	if (lgl_gport != BCM_GPORT_TYPE_NONE) {
	    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_QUEUE_GROUP (lgl_gport)) {

		qm_addr_sel = qm_addr_queue;

		if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form qid */
		    queue_base = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		}

		cntrId = queue_base + cosq;
		/*
		 * SLQ stats are available through
		 * Selected Queue counter registers.
		 * If any of them are configured, and
		 * we have a queue_base available,
		 * then enable the SLQ capture.
		 */

		if (flags & BCM_COSQ_GPORT_STATS_SLQ) {
		    if (sir->cs.flags & CS_SLQ_ENABLE) {
			SHR_BITSET_RANGE(profile->stats, bcmCosqGportGreenAcceptedPkts,6);
			SHR_BITSET_RANGE(profile->stats, bcmCosqGportYellowAcceptedPkts, 6);
			SHR_BITSET_RANGE(profile->stats, bcmCosqGportRedAcceptedPkts, 6);
			SHR_BITSET_RANGE(profile->stats, bcmCosqGportBlackAcceptedPkts,6);
			SHR_BITSET_RANGE(profile->stats, bcmCosqGportNonWredDroppedPkts, 8);
		    }
		}

	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "gport_profile_set: unit %d unknown Logical Interface"
		                       " %x\n"), unit, lgl_gport));
		return BCM_E_PARAM;
	    }
	}

	switch(qm_addr_sel) {
	    case qm_addr_interface:
		for (type=0; type<16; type++)
		    if (sir->type_res_flags[PRED_TYPE_RB][type] & (1 << (intf + 4))) {
			for (dp=0; dp < 8; dp++) {
			    BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, ((type << 3) + dp), &statsCfg));
			    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD2_EVENT_SELf)) {
				segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD2_SEGMENT_SELf);
				if (COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (1 << (intf)))) {
				    if (shared) {
					if (sir->cs.segment[segment].shared)
					    CS_STAT_COLOR_SET(dp);
				    } else {
					if (sir->cs.segment[segment].shared == 0)
					    CS_STAT_COLOR_SET(dp);
				    }
				}
			    }
			}
		    }
		break;

	    case qm_addr_queue:
		for (type=0; type<16; type++)
		    if ((sir->type_res_flags[PRED_TYPE_RB][type] & (1 << (intf + 4))) != 0) {
			for (dp=0; dp < 8; dp++) {
			    BCM_IF_ERROR_RETURN( READ_STATSCFGm(unit, MEM_BLOCK_ANY, ((type << 3) + dp), &statsCfg));
			    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf)) {
				segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
				if (COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap,(cntrId>>CS_CFG_GRP_SHIFT))) {
				    if (shared) {
					if (sir->cs.segment[segment].shared)
					    CS_STAT_COLOR_SET(dp);
				    } else {
					if (sir->cs.segment[segment].shared == 0)
					    CS_STAT_COLOR_SET(dp);
				    }
				}
			    }
			}
		    }
		break;
        /* coverity[dead_error_begin : FALSE] */
	    case qm_addr_stats_label:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "gport_profile_set: unit %d stats_label mode unsupported\n"),
		           unit));
		return BCM_E_UNAVAIL;
		break;

        /* coverity[dead_error_begin : FALSE] */
	    case qm_addr_histogram:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "gport_profile_set: unit %d histogram mode unsupported\n"),
		           unit));
		return BCM_E_UNAVAIL;
		break;
        /* coverity[dead_error_begin : FALSE] */
	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "gport_profile_set: unit %d unknown mode\n"),
		           unit));
		return BCM_E_UNAVAIL;
	}
    }

    return rv;
}

static int
_bcm_sirius_cosq_gport_statistic_set(int unit,
				     bcm_gport_t gport,
				     bcm_gport_t lgl_gport,
				     bcm_cos_queue_t cosq,
				     int flags,
				     bcm_cosq_gport_stats_t stat,
				     uint64 value)
{
    int rv = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    statscfg_entry_t   statsCfg;
    ep_stats_ctrl_entry_t    ep_stats_ctrl;
    bcm_trunk_t tid = 0;
    int i = 0, loopcnt = 1;
    sbBool_t shared = (flags & BCM_COSQ_GPORT_STATS_SHARED) ? 1 : 0;
    sbBool_t slq = 0, fdm = 1;
    sbBool_t global = (flags & BCM_COSQ_GPORT_STATS_GLOBAL) ? 1 : 0;
    sbBool_t set_any = (flags & BCM_COSQ_GPORT_STATS_SET_ANY) ? 1 : 0;
    sbBool_t ingress = BCM_GPORT_IS_MODPORT(gport) ? 1 : 0;
    sbBool_t logical = (lgl_gport != BCM_GPORT_TYPE_NONE) ? 1 : 0;
    uint8 bit_size = 0, level = 0, type = 0;
    uint16 cntrId = 0;
    int segment = -1;
    uint32 queue_base = 0, regval = 0, dp = 0;
    int intf = 0, index = 0, mod = 0, port = 0, idx = 0, eg_n = -1;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;

    /*
     * check to see if valid
     */
    if (!SOC_IS_SBX_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    /*
     * this routine only supports clearing statistics.
     * Therefore, value must be zero.
     */
    if (!set_any && (COMPILER_64_IS_ZERO(value) == FALSE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "stat_set: unit %d value 0x%08x%08x not 0\n"),
	           unit, COMPILER_64_HI(value), COMPILER_64_LO(value)));
	return BCM_E_PARAM;
    }

    if (sir->cs.flags & CS_SLQ_ENABLE) {
	slq = (flags & BCM_COSQ_GPORT_STATS_SLQ) ? 1 : 0;
    }

    switch (stat) {
	case bcmCosqGportOutLimitDroppedPkts:
	    idx = 1;
	    type = FD_ALL;
	    break;
	case bcmCosqGportOutLimitDroppedBytes:
	    idx = 0;
	    type = FD_ALL;
	    break;
	case bcmCosqGportOutLimitDroppedGreenPkts:
	    idx = 1;
	    type = FD_GREEN;
	    break;
	case bcmCosqGportOutLimitDroppedGreenBytes:
	    idx = 0;
	    type = FD_GREEN;
	    break;
	case bcmCosqGportOutLimitDroppedYellowPkts:
	    idx = 1;
	    type = FD_YELLOW;
	    break;
	case bcmCosqGportOutLimitDroppedYellowBytes:
	    idx = 0;
	    type = FD_YELLOW;
	    break;
	case bcmCosqGportOutLimitDroppedRedPkts:
	    idx = 1;
	    type = FD_RED;
	    break;
	case bcmCosqGportOutLimitDroppedRedBytes:
	    idx = 0;
	    type = FD_RED;
	    break;
	case bcmCosqGportOutLimitDroppedMulticastPkts:
	    idx = 1;
	    type = FD_MC;
	    break;
	case bcmCosqGportOutLimitDroppedMulticastBytes:
	    idx = 0;
	    type = FD_MC;
	    break;
	default:
	    fdm = 0;
    }

    if (fdm) {
	port = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, NULL, NULL);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "Invalid gport(0x%x) for stat %d\n"),
	               gport, stat));
	    return rv;
	}
	sir->cs.fd_drop[type][port*2+idx] = value;
	return BCM_E_NONE;
    }

    if (BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	BCM_GPORT_IS_EGRESS_MODPORT(gport)) {

	/*
	 * Egress statistics
	 */

	/*
	 * Get counter and level
	 */
	rv = bcm_sirius_get_egress_info(unit, gport, &bit_size, &level, &cntrId, &intf);
	if (rv != BCM_E_NONE) {
	    return rv;
	}

	if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	    mod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
	    port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
	} else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	    mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
	    port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
	} else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	    mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
	    port = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, NULL);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "Invalid gport(0x%x). Unable to resolve a port\n"),
		           gport));
		return rv;
	    }
	    idx = cosq;
	}

        rv = bcm_sirius_trunk_find_and_get(unit,
                                           mod,
                                           gport,
                                           &tid,
                                           &trunkInfo,
                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                           &(trunkMembers[0]),
                                           &memberCount);
	if (rv == BCM_E_NONE) {
            if ((memberCount > 0) && (memberCount <= SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS)) {
                loopcnt = memberCount;
	    } else {
		loopcnt = 1;
	    }
	} else {
	    loopcnt = 1;
	}

	for (i=0; i < loopcnt; i++) {
	    /*
	     * If member of trunk, loop through each member subport.
	     * Otherwise, pass through once using cntrId;
	     */
	    if (rv == BCM_E_NONE) {
                BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, trunkMembers[i].gport, &bit_size, &level, &cntrId, &intf));
	    }
	    cntrId += idx;

	    /*
	     * find the type associated with this interface
	     */
	    for (type = 0; type < 16; type++) {
		if (sir->type_res_flags[PRED_TYPE_EP][type] & (1 << (intf + 4))) {
		    break;
		}
	    }

	    /*
	     * If no type found, return an error
	     */
	    if (type == 16) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_set: unit %d Egress type unknown gport %d flags %d"
		                       " stat %d\n"), unit, gport, flags, stat));
		return BCM_E_UNAVAIL;
	    }

	    /*
	     * Find the segment matching level for STAT0 or STAT1 at index TYPE
	     */
	    BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, type, &ep_stats_ctrl));
	    if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
		(soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf) == level)) {
		segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_SEGMENTf);
	    } else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) != 0) &&
		       (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_META0_SELf) == level)) {
		segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_SEGMENTf);
	    }

	    /*
	     * If no level is configured for this type, then return a message
	     */
	    if (segment < 0) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_set: unit %d Egress level %d not configured on"
		                       " type %d\n"), unit, level, type));
		return BCM_E_UNAVAIL;
	    }

	    /*
	     * Validate the cntrId is configured on this segment.
	     * In egress, cntrId can never exceed 32 bits, so don't need to do
	     * complicated 64 bit manipulation.
	     */
	    if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId >> CS_CFG_GRP_SHIFT))) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_set: unit %d stat %d for level %d not currently"
		                       " configured\n"), unit, cntrId, level));
		return BCM_E_PARAM;
	    }

	    switch(stat) {
		case bcmCosqGportOutPkts:
		    sir->cs.segment[segment].ullCount[cntrId*2] = value;
		    break;
		case bcmCosqGportOutBytes:
		    sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "stat_set: unit %d Egress stat %d invalid\n"),
		               unit, stat));
		    return BCM_E_PARAM;
	    }
	}
	return BCM_E_NONE;
    }

    /*
     * Validate SLQ statistic
     */

    if (slq) {
	switch (stat) {
	    case bcmCosqGportGreenAcceptedPkts:
	    case bcmCosqGportGreenAcceptedBytes:
	    case bcmCosqGportGreenCongestionMarkedPkts:
	    case bcmCosqGportGreenCongestionMarkedBytes:
	    case bcmCosqGportGreenDiscardDroppedPkts:
	    case bcmCosqGportGreenDiscardDroppedBytes:
	    case bcmCosqGportYellowAcceptedPkts:
	    case bcmCosqGportYellowAcceptedBytes:
	    case bcmCosqGportYellowCongestionMarkedPkts:
	    case bcmCosqGportYellowCongestionMarkedBytes:
	    case bcmCosqGportYellowDiscardDroppedPkts:
	    case bcmCosqGportYellowDiscardDroppedBytes:
	    case bcmCosqGportRedAcceptedPkts:
	    case bcmCosqGportRedAcceptedBytes:
	    case bcmCosqGportRedCongestionMarkedPkts:
	    case bcmCosqGportRedCongestionMarkedBytes:
	    case bcmCosqGportRedDiscardDroppedPkts:
	    case bcmCosqGportRedDiscardDroppedBytes:
	    case bcmCosqGportBlackAcceptedPkts:
	    case bcmCosqGportBlackAcceptedBytes:
	    case bcmCosqGportBlackCongestionMarkedPkts:
	    case bcmCosqGportBlackCongestionMarkedBytes:
	    case bcmCosqGportBlackDiscardDroppedPkts:
	    case bcmCosqGportBlackDiscardDroppedBytes:
	    case bcmCosqGportNonWredDroppedPkts:
	    case bcmCosqGportNonWredDroppedBytes:
	    case bcmCosqGportDequeuedPkts:
	    case bcmCosqGportDequeuedBytes:
	    case bcmCosqGportOverSubscribeTotelDroppedPkts:
	    case bcmCosqGportOverSubscribeTotalDroppedBytes:
	    case bcmCosqGportOverSubscribeGuaranteeDroppedPkts:
	    case bcmCosqGportOverSubscribeGuaranteedDroppedBytes:
		ingress = 0;
		goto set_stat;
		break;
	    default:
		LOG_CLI((BSL_META_U(unit,
                                    "gport_profile_set: unit %d stat %d not supported under SLQ\n"), unit, stat));
		return BCM_E_PARAM;
	}
    }

    /*
     * validate global statistic if not slq.
     * The following list of statistics only apply to SLQ or Global. All other statistics apply
     * to ingress packets in the CS and take precedence.
     */

    switch(stat) {
	case bcmCosqGportGreenCongestionMarkedPkts:
	case bcmCosqGportGreenCongestionMarkedBytes:
	case bcmCosqGportGreenDroppedPkts:
	case bcmCosqGportGreenDroppedBytes:
	case bcmCosqGportYellowCongestionMarkedPkts:
	case bcmCosqGportYellowCongestionMarkedBytes:
	case bcmCosqGportYellowDroppedPkts:
	case bcmCosqGportYellowDroppedBytes:
	case bcmCosqGportRedCongestionMarkedPkts:
	case bcmCosqGportRedCongestionMarkedBytes:
	case bcmCosqGportRedDroppedPkts:
	case bcmCosqGportRedDroppedBytes:
	case bcmCosqGportBlackCongestionMarkedPkts:
	case bcmCosqGportBlackCongestionMarkedBytes:
	case bcmCosqGportBlackDroppedPkts:
	case bcmCosqGportBlackDroppedBytes:
	    ingress = 0;
	    global = 1;
	    goto set_stat;
	    break;
	case bcmCosqGportDequeuedPkts:
	case bcmCosqGportDequeuedBytes:
	    if (global) {
		ingress = 0;
		goto set_stat;
	    }
	    break;
	case bcmCosqGportNonWredDroppedPkts:
	case bcmCosqGportNonWredDroppedBytes:
	case bcmCosqGportOverSubscribeTotelDroppedPkts:
	case bcmCosqGportOverSubscribeTotalDroppedBytes:
	case bcmCosqGportOverSubscribeGuaranteeDroppedPkts:
	case bcmCosqGportOverSubscribeGuaranteedDroppedBytes:
	    ingress = 0;
	    slq = 1;
	    goto set_stat;
	    break;
	default:
	    break;
    }

    if (ingress) {

	/*
	 * If ingress, is this request physical or logical
	 */

	if (logical) {
	    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_QUEUE_GROUP (lgl_gport)) {

		if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form qid */
		    queue_base = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form qid */
		    queue_base = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		}
		cntrId = queue_base + cosq;
	    }
	} else {
	    /*
	     * requesting physical stat value
	     * get the physical interface
	     */

	    rv = bcm_sbx_port_get_intf_portoffset(unit, gport, &intf, &index);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	    
	    cntrId = intf;
	}

	/*
	 * validate the statistic for this
	 * set of parameters
	 */
	switch(stat) {
	    case bcmCosqGportGreenAcceptedPkts:
	    case bcmCosqGportGreenAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_GREEN, &dp));
		break;
	    case bcmCosqGportYellowAcceptedPkts:
	    case bcmCosqGportYellowAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_YELLOW, &dp));
		break;
	    case bcmCosqGportRedAcceptedPkts:
	    case bcmCosqGportRedAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_RED, &dp));
		break;
	    case bcmCosqGportBlackAcceptedPkts:
	    case bcmCosqGportBlackAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_BLACK, &dp));
		break;
	    case bcmCosqGportGreenDiscardDroppedPkts:
	    case bcmCosqGportGreenDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_GREEN, &dp));
		break;
	    case bcmCosqGportYellowDiscardDroppedPkts:
	    case bcmCosqGportYellowDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_YELLOW, &dp));
		break;
	    case bcmCosqGportRedDiscardDroppedPkts:
	    case bcmCosqGportRedDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_RED, &dp));
		break;
	    case bcmCosqGportBlackDiscardDroppedPkts:
	    case bcmCosqGportBlackDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_BLACK, &dp));
		break;
	    case bcmCosqGportDequeuedPkts:
	    case bcmCosqGportDequeuedBytes:
		dp = 8;
		break;
	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_set: unit %d, Ingress stat %d not valid\n"),
		           unit, stat));
		return BCM_E_PARAM;
	}


	/*
	 * Find the segment matching the shared or unshared
	 * DP value for this interface
	 */
	if (dp < 8) {
	    BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, dp, &statsCfg));
	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf)) {
		segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_set: unit %d Ingress stat %d not configured\n"),
		           unit, stat));
		return BCM_E_UNAVAIL;
	    }
	} else if (!shared && (dp == 8)) {
	    BCM_IF_ERROR_RETURN(READ_QM_CS_CONFIG0r(unit, &regval));
	    if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf)) {
		segment = soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_set: unit %d Ingress stat %d not configured\n"),
		           unit, stat));
		return BCM_E_UNAVAIL;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "stat_set: unit %d Ingress stat %d not configured\n"),
	               unit, stat));
	    return BCM_E_UNAVAIL;
	}

	/*
	 * Validate the cntrId is configured on this segment
	 */

	if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId>>CS_CFG_GRP_SHIFT))) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "stat_set: unit %d Ingress stat %d for segment %d not currently"
	                           " configured\n"), unit, stat, segment));
	    return BCM_E_PARAM;
	}
    }

set_stat:

    /*
     * set the appropriate statistic.
     * Several fields are overloaded with Debug SLQ or GLOBAL values,
     * so return the appropriate statistic based on the
     * passed parameters
     */

    if ((slq == 0) && (global == 0) && (segment == -1)) {
	/* If not slq or global, then must be ingress without proper GPORT */
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "%s: unit %d Illegal gport 0x%x\n"),
	           FUNCTION_NAME(), unit, gport));
	return BCM_E_PARAM;
    }

    switch(stat) {

	case bcmCosqGportGreenAcceptedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 0] = value;
	    else if (global)
		sir->cs.gbl_stats[1] = value;
	    break;
	case bcmCosqGportGreenAcceptedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 1] = value;
	    else if (global)
		sir->cs.gbl_stats[0] = value;
	    break;
	case bcmCosqGportGreenCongestionMarkedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 2] = value;
	    else if (global)
		sir->cs.gbl_stats[17] = value;
	    break;
	case bcmCosqGportGreenCongestionMarkedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 3] = value;
	    else if (global)
		sir->cs.gbl_stats[16] = value;
	    break;
	case bcmCosqGportGreenDiscardDroppedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 4] = value;
	    else if (global)
		sir->cs.gbl_stats[9] = value;
	    break;
	case bcmCosqGportGreenDiscardDroppedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 5] = value;
	    else if (global)
		sir->cs.gbl_stats[8] = value;
	    break;
	case bcmCosqGportGreenDroppedPkts:
	    if (global)
		sir->cs.gbl_stats[25] = value;
	    break;
	case bcmCosqGportGreenDroppedBytes:
	    if (global)
		sir->cs.gbl_stats[24] = value;
	    break;
	case bcmCosqGportYellowAcceptedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 6] = value;
	    else if (global)
		sir->cs.gbl_stats[3] = value;
	    break;
	case bcmCosqGportYellowAcceptedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 7] = value;
	    else if (global)
		sir->cs.gbl_stats[2] = value;
	    break;
	case bcmCosqGportYellowCongestionMarkedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 8] = value;
	    else if (global)
		sir->cs.gbl_stats[19] = value;
	    break;
	case bcmCosqGportYellowCongestionMarkedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 9] = value;
	    else if (global)
		sir->cs.gbl_stats[18] = value;
	    break;
	case bcmCosqGportYellowDiscardDroppedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 10] = value;
	    else if (global)
		sir->cs.gbl_stats[11] = value;
	    break;
	case bcmCosqGportYellowDiscardDroppedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 11] = value;
	    else if (global)
		sir->cs.gbl_stats[10] = value;
	    break;
	case bcmCosqGportYellowDroppedPkts:
	    if (global)
		sir->cs.gbl_stats[27] = value;
	    break;
	case bcmCosqGportYellowDroppedBytes:
	    if (global)
		sir->cs.gbl_stats[26] = value;
	    break;
	case bcmCosqGportRedAcceptedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 12] = value;
	    else if (global)
		sir->cs.gbl_stats[5] = value;
	    break;
	case bcmCosqGportRedAcceptedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 13] = value;
	    else if (global)
		sir->cs.gbl_stats[4] = value;
	    break;
	case bcmCosqGportRedCongestionMarkedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 14] = value;
	    else if (global)
		sir->cs.gbl_stats[21] = value;
	    break;
	case bcmCosqGportRedCongestionMarkedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 15] = value;
	    else if (global)
		sir->cs.gbl_stats[20] = value;
	    break;
	case bcmCosqGportRedDiscardDroppedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 16] = value;
	    else if (global)
		sir->cs.gbl_stats[13] = value;
	    break;
	case bcmCosqGportRedDiscardDroppedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 17] = value;
	    else if (global)
		sir->cs.gbl_stats[12] = value;
	    break;
	case bcmCosqGportRedDroppedPkts:
	    if (global)
		sir->cs.gbl_stats[29] = value;
	    break;
	case bcmCosqGportRedDroppedBytes:
	    if (global)
		sir->cs.gbl_stats[28] = value;
	    break;
	case bcmCosqGportBlackAcceptedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 18] = value;
	    else if (global)
		sir->cs.gbl_stats[7] = value;
	    break;
	case bcmCosqGportBlackAcceptedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 19] = value;
	    else if (global)
		sir->cs.gbl_stats[6] = value;
	    break;
	case bcmCosqGportBlackCongestionMarkedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 20] = value;
	    else if (global)
		sir->cs.gbl_stats[23] = value;
	    break;
	case bcmCosqGportBlackCongestionMarkedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 21] = value;
	    else if (global)
		sir->cs.gbl_stats[22] = value;
	    break;
	case bcmCosqGportBlackDiscardDroppedPkts:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 22] = value;
	    else if (global)
		sir->cs.gbl_stats[15] = value;
	    break;
	case  bcmCosqGportBlackDiscardDroppedBytes:
	    if (ingress)
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    else if (slq)
		sir->cs.slq_stats[cosq*16 + 23] = value;
	    else if (global)
		sir->cs.gbl_stats[14] = value;
	    break;
	case bcmCosqGportBlackDroppedPkts:
	    if (global)
		sir->cs.gbl_stats[31] = value;
	    break;
	case bcmCosqGportBlackDroppedBytes:
	    if (global)
		sir->cs.gbl_stats[30] = value;
	    break;
	case bcmCosqGportNonWredDroppedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 24] = value;
	    break;
	case bcmCosqGportNonWredDroppedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 25] = value;
	    break;
	case bcmCosqGportDequeuedPkts:
	    if (slq) 
		sir->cs.slq_stats[cosq*16 + 30] = value;
	    else if (global)
		sir->cs.gbl_stats[33] = value;
	    else
		sir->cs.segment[segment].ullCount[cntrId*2] = value;
	    break;
	case bcmCosqGportDequeuedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 31] = value;
	    else if (global)
		sir->cs.gbl_stats[32] = value;
	    else
		sir->cs.segment[segment].ullCount[cntrId*2 + 1] = value;
	    break;
	case bcmCosqGportOverSubscribeTotelDroppedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 26] = value;
	    break;
	case bcmCosqGportOverSubscribeTotalDroppedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 27] = value;
	    break;
	case bcmCosqGportOverSubscribeGuaranteeDroppedPkts:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 28] = value;
	    break;
	case bcmCosqGportOverSubscribeGuaranteedDroppedBytes:
	    if (slq)
		sir->cs.slq_stats[cosq*16 + 29] = value;
	    break;
	default:
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "gport_profile_set: unit %d unknown mode\n"),
	               unit));
	    return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

static int
_bcm_sirius_cosq_gport_statistic_get(int unit,
				     bcm_gport_t gport,
				     bcm_gport_t lgl_gport,
				     bcm_cos_queue_t cosq,
				     uint32 flags,
				     bcm_cosq_gport_stats_t stat,
				     uint64 *value)
{
    int rv = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    statscfg_entry_t   statsCfg;
    ep_stats_ctrl_entry_t    ep_stats_ctrl;
    bcm_trunk_t tid = 0;
    int i = 0, loopcnt = 1;
    sbBool_t shared = (flags & BCM_COSQ_GPORT_STATS_SHARED) ? 1 : 0;
    sbBool_t flush = (flags & BCM_COSQ_GPORT_STATS_LATEST) ? 1 : 0;
    sbBool_t slq = 0, fdm = 1;
    sbBool_t global = (flags & BCM_COSQ_GPORT_STATS_GLOBAL) ? 1 : 0;
    sbBool_t ingress = BCM_GPORT_IS_MODPORT(gport) ? 1 : 0;
    sbBool_t logical = (lgl_gport != BCM_GPORT_TYPE_NONE) ? 1 : 0;
    uint8 bit_size = 0, level = 0, type = 0;
    uint16 cntrId = 0;
    int segment = -1;
    uint32 queue_base = 0, regval = 0, dp = 0;
    int intf = 0, index = 0, mod = 0, port = 0, idx = 0, eg_n = -1;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;

    /*
     * check to see if valid
     */
    if (!SOC_IS_SBX_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (sir->cs.flags & CS_SLQ_ENABLE) {
	slq = (flags & BCM_COSQ_GPORT_STATS_SLQ) ? 1 : 0;
    }

    switch (stat) {
	case bcmCosqGportOutLimitDroppedPkts:
 	    idx = 1;
	    type = FD_ALL;
	    break;
	case bcmCosqGportOutLimitDroppedBytes:
	    idx = 0;
	    type = FD_ALL;
	    break;
	case bcmCosqGportOutLimitDroppedGreenPkts:
	    idx = 1;
	    type = FD_GREEN;
	    break;
	case bcmCosqGportOutLimitDroppedGreenBytes:
	    idx = 0;
	    type = FD_GREEN;
	    break;
	case bcmCosqGportOutLimitDroppedYellowPkts:
	    idx = 1;
	    type = FD_YELLOW;
	    break;
	case bcmCosqGportOutLimitDroppedYellowBytes:
	    idx = 0;
	    type = FD_YELLOW;
	    break;
	case bcmCosqGportOutLimitDroppedRedPkts:
	    idx = 1;
	    type = FD_RED;
	    break;
	case bcmCosqGportOutLimitDroppedRedBytes:
	    idx = 0;
	    type = FD_RED;
	    break;
	case bcmCosqGportOutLimitDroppedMulticastPkts:
	    idx = 1;
	    type = FD_MC;
	    break;
	case bcmCosqGportOutLimitDroppedMulticastBytes:
	    idx = 0;
	    type = FD_MC;
	    break;
	default:
	    fdm = 0;
    }

    COMPILER_64_ZERO(*value);

    if (fdm) {
	port = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, NULL, NULL);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "Invalid gport(0x%x) for stat %d\n"),
	               gport, stat));
	    return rv;
	}
	COMPILER_64_ADD_64(*value,  sir->cs.fd_drop[type][port*2+idx]);
	return BCM_E_NONE;
    }

    if (BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	BCM_GPORT_IS_EGRESS_MODPORT(gport)) {

	/*
	 * Get counter and level
	 */
	BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, gport, &bit_size, &level, &cntrId, &intf));

	/*
	 * Are we currently part of a trunk?
	 */

	if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	    mod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
	    port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
	} else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	    mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
	    port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
	} else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	    mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
	    port = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, NULL);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "Invalid gport(0x%x). Unable to resolve a port\n"),
		           gport));
		return rv;
	    }
	    idx = cosq;
	}

        loopcnt = 2; 
        rv = bcm_sirius_trunk_find_and_get(unit,
                                           mod,
                                           gport,
                                           &tid,
                                           &trunkInfo,
                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                           &(trunkMembers[0]),
                                           &memberCount);
	if (rv == BCM_E_NONE) {
            if ((memberCount > 0) && (memberCount <= SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS)) {
                loopcnt = memberCount;
	    } else {
                loopcnt = 1;
            }
	} else {
            loopcnt = 1;
        }

	for (i=0; i < loopcnt; i++) {
	    /*
	     * If member of trunk, loop through each member subport.
	     * Otherwise, pass through once using cntrId;
	     */
	    if (rv == BCM_E_NONE) {
                BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, trunkMembers[i].gport, &bit_size, &level, &cntrId, &intf));
	    }
	    cntrId += idx;

	    /*
	     * find the type associated with this interface
	     */
	    for (type = 0; type < 16; type++) {
		if (sir->type_res_flags[PRED_TYPE_EP][type] & (1 << (intf + 4)))
		    break;
	    }

	    /*
	     * If no type found, return an error
	     */
	    if (type == 16) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_get: unit %d Egress type unknown gport %d flags %d"
		                       " stat %d\n"), unit, gport, flags, stat));
		return BCM_E_UNAVAIL;
	    }

	    /*
	     * Find the segment matching level for STAT0 or STAT1 at index TYPE
	     */
	    BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, type, &ep_stats_ctrl));
	    if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
		(soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf) == level))
		segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_SEGMENTf);
	    else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) != 0) &&
		     (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_META0_SELf) == level))
		segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_SEGMENTf);

	    /*
	     * If no level is configured for this type, then return a message
	     */
	    if (segment < 0) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_get: unit %d Egress level %d not configured on"
		                       " type %d\n"), unit, level, type));
		return BCM_E_UNAVAIL;
	    }

	    /*
	     * Validate the cntrId is configured on this segment.
	     * On egress, cntrId can never exceed 32K,so don't need to do complicated 64 bit manipulation.
	     */
	    if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId >> CS_CFG_GRP_SHIFT))) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_get: unit %d stat %d for level %d not currently"
		                       " configured\n"), unit, cntrId, level));
		return BCM_E_PARAM;
	    }

	    /*
	     * if flush is requested, force a refresh of the statistics
	     */
	    if (flush)
		soc_sbx_sirius_flush_segment(unit, segment, cntrId, 1);

	    switch (stat) {
		case bcmCosqGportOutPkts:
		    COMPILER_64_ADD_64(*value,  sir->cs.segment[segment].ullCount[cntrId*2]);
		    break;
		case bcmCosqGportOutBytes:
		    COMPILER_64_ADD_64(*value,  sir->cs.segment[segment].ullCount[cntrId*2+1]);
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "stat_get: unit %d Egress stat %d invalid\n"),
		               unit, stat));
		    return BCM_E_PARAM;
	    }
	}
	return BCM_E_NONE;
    }

    /*
     * Validate SLQ statistic
     */

    if (slq) {
	switch (stat) {
	    case bcmCosqGportGreenAcceptedPkts:
	    case bcmCosqGportGreenAcceptedBytes:
	    case bcmCosqGportGreenCongestionMarkedPkts:
	    case bcmCosqGportGreenCongestionMarkedBytes:
	    case bcmCosqGportGreenDiscardDroppedPkts:
	    case bcmCosqGportGreenDiscardDroppedBytes:
	    case bcmCosqGportYellowAcceptedPkts:
	    case bcmCosqGportYellowAcceptedBytes:
	    case bcmCosqGportYellowCongestionMarkedPkts:
	    case bcmCosqGportYellowCongestionMarkedBytes:
	    case bcmCosqGportYellowDiscardDroppedPkts:
	    case bcmCosqGportYellowDiscardDroppedBytes:
	    case bcmCosqGportRedAcceptedPkts:
	    case bcmCosqGportRedAcceptedBytes:
	    case bcmCosqGportRedCongestionMarkedPkts:
	    case bcmCosqGportRedCongestionMarkedBytes:
	    case bcmCosqGportRedDiscardDroppedPkts:
	    case bcmCosqGportRedDiscardDroppedBytes:
	    case bcmCosqGportBlackAcceptedPkts:
	    case bcmCosqGportBlackAcceptedBytes:
	    case bcmCosqGportBlackCongestionMarkedPkts:
	    case bcmCosqGportBlackCongestionMarkedBytes:
	    case bcmCosqGportBlackDiscardDroppedPkts:
	    case bcmCosqGportBlackDiscardDroppedBytes:
	    case bcmCosqGportNonWredDroppedPkts:
	    case bcmCosqGportNonWredDroppedBytes:
	    case bcmCosqGportDequeuedPkts:
	    case bcmCosqGportDequeuedBytes:
	    case bcmCosqGportOverSubscribeTotelDroppedPkts:
	    case bcmCosqGportOverSubscribeTotalDroppedBytes:
	    case bcmCosqGportOverSubscribeGuaranteeDroppedPkts:
	    case bcmCosqGportOverSubscribeGuaranteedDroppedBytes:
		ingress = 0;
		goto get_stat;
		break;
	    default:
		LOG_CLI((BSL_META_U(unit,
                                    "gport_profile_get: unit %d stat %d not supported under SLQ\n"), unit, stat));
		return BCM_E_PARAM;
	}
    }

    /*
     * validate global statistic if not slq.
     * The following list of statistics only apply to SLQ or Global. All other statistics apply
     * to ingress packets in the CS and take precedence.
     */

    switch(stat) {
	case bcmCosqGportGreenCongestionMarkedPkts:
	case bcmCosqGportGreenCongestionMarkedBytes:
	case bcmCosqGportGreenDroppedPkts:
	case bcmCosqGportGreenDroppedBytes:
	case bcmCosqGportYellowCongestionMarkedPkts:
	case bcmCosqGportYellowCongestionMarkedBytes:
	case bcmCosqGportYellowDroppedPkts:
	case bcmCosqGportYellowDroppedBytes:
	case bcmCosqGportRedCongestionMarkedPkts:
	case bcmCosqGportRedCongestionMarkedBytes:
	case bcmCosqGportRedDroppedPkts:
	case bcmCosqGportRedDroppedBytes:
	case bcmCosqGportBlackCongestionMarkedPkts:
	case bcmCosqGportBlackCongestionMarkedBytes:
	case bcmCosqGportBlackDroppedPkts:
	case bcmCosqGportBlackDroppedBytes:
	    ingress = 0;
	    global = 1;
	    goto get_stat;
	    break;
	case bcmCosqGportDequeuedPkts:
	case bcmCosqGportDequeuedBytes:
	    if (global) {
		ingress = 0;
		goto get_stat;
	    }
	    break;
	case bcmCosqGportNonWredDroppedPkts:
	case bcmCosqGportNonWredDroppedBytes:
	case bcmCosqGportOverSubscribeTotelDroppedPkts:
	case bcmCosqGportOverSubscribeTotalDroppedBytes:
	case bcmCosqGportOverSubscribeGuaranteeDroppedPkts:
	case bcmCosqGportOverSubscribeGuaranteedDroppedBytes:
	    ingress = 0;
	    slq = 1;
	    goto get_stat;
	    break;
	default:
	    break;
    }

    if (ingress) {

	/*
	 * If ingress, is this request physical or logical
	 */

	if (logical) {
	    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
		BCM_GPORT_IS_MCAST_QUEUE_GROUP (lgl_gport)) {

		if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form qid */
		    queue_base = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form qid */
		    queue_base = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
		} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport)) {
		    /* The gport is of the form sysport/qid */
		    queue_base = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(lgl_gport);
		}
		cntrId = queue_base + cosq;
	    }
	} else {
	    /*
	     * requesting physical stat value
	     * get the physical interface
	     */

	    rv = bcm_sbx_port_get_intf_portoffset(unit, gport, &intf, &index);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	    cntrId = intf;
	}


	/*
	 * validate the statistic for this
	 * set of parameters
	 */
	switch(stat) {
	    case bcmCosqGportGreenAcceptedPkts:
	    case bcmCosqGportGreenAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_GREEN, &dp));
		break;
	    case bcmCosqGportYellowAcceptedPkts:
	    case bcmCosqGportYellowAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_YELLOW, &dp));
		break;
	    case bcmCosqGportRedAcceptedPkts:
	    case bcmCosqGportRedAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_RED, &dp));
		break;
	    case bcmCosqGportBlackAcceptedPkts:
	    case bcmCosqGportBlackAcceptedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_BLACK, &dp));
		break;
	    case bcmCosqGportGreenDiscardDroppedPkts:
	    case bcmCosqGportGreenDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_GREEN, &dp));
		break;
	    case bcmCosqGportYellowDiscardDroppedPkts:
	    case bcmCosqGportYellowDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_YELLOW, &dp));
		break;
	    case bcmCosqGportRedDiscardDroppedPkts:
	    case bcmCosqGportRedDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_RED, &dp));
		break;
	    case bcmCosqGportBlackDiscardDroppedPkts:
	    case bcmCosqGportBlackDiscardDroppedBytes:
		BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_BLACK, &dp));
		break;
	    case bcmCosqGportDequeuedPkts:
	    case bcmCosqGportDequeuedBytes:
		dp = 8;
		break;
	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_get: unit %d, Ingress stat %d not valid\n"),
		           unit, stat));
		return BCM_E_PARAM;
	}

	/*
	 * Find the segment matching the shared or unshared
	 * DP value for this interface
	 */
	if (dp < 8) {
	    BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, dp, &statsCfg));
	    if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf)) {
		segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_get: unit %d Ingress stat %d not configured\n"),
		           unit, stat));
		return BCM_E_UNAVAIL;
	    }
	} else if (!shared && (dp == 8)) {
	    BCM_IF_ERROR_RETURN(READ_QM_CS_CONFIG0r(unit, &regval));
	    if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf)) {
		segment = soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "stat_get: unit %d Ingress stat %d not configured\n"),
		           unit, stat));
		return BCM_E_UNAVAIL;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "stat_get: unit %d Ingress stat %d not configured\n"),
	               unit, stat));
	    return BCM_E_UNAVAIL;
	}


	/*
	 * Validate the cntrId is configured on this segment
	 */

	if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId>>CS_CFG_GRP_SHIFT))) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "stat_get: unit %d Ingress stat %d for segment %d not currently"
	                           " configured\n"), unit, stat, segment));
	    return BCM_E_PARAM;
	}


	/*
	 * if flush is requested, force a refresh of the statistics
	 */
	if (flush)
	    soc_sbx_sirius_flush_segment(unit, segment, cntrId, 1);
    }

    /*
     * get the appropriate statistic.
     * Several fields are overloaded with Debug SLQ or GLOBAL values,
     * so return the appropriate statistic based on the
     * passed parameters
     */

get_stat:

    if ((slq == 0) && (global == 0) && (segment == -1)) {
	/* If not slq or global, then must be ingress without proper GPORT */
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "%s: unit %d Illegal gport 0x%x\n"),
	           FUNCTION_NAME(), unit, gport));
	return BCM_E_PARAM;
    }

    switch(stat) {

	case bcmCosqGportGreenAcceptedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 0];
	    else if (global)
		*value = sir->cs.gbl_stats[1];
	    break;
	case bcmCosqGportGreenAcceptedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 1];
	    else if (global)
		*value = sir->cs.gbl_stats[0];
	    break;
	case bcmCosqGportGreenCongestionMarkedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 2];
	    else if (global)
		*value = sir->cs.gbl_stats[17];
	    break;
	case bcmCosqGportGreenCongestionMarkedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 3];
	    else if (global)
		*value = sir->cs.gbl_stats[16];
	    break;
	case bcmCosqGportGreenDiscardDroppedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 4];
	    else if (global)
		*value = sir->cs.gbl_stats[9];
	    break;
	case bcmCosqGportGreenDiscardDroppedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 5];
	    else if (global)
		*value = sir->cs.gbl_stats[8];
	    break;
	case bcmCosqGportGreenDroppedPkts:
	    if (global)
		*value = sir->cs.gbl_stats[25];
	    break;
	case bcmCosqGportGreenDroppedBytes:
	    if (global)
		*value = sir->cs.gbl_stats[24];
	    break;
	case bcmCosqGportYellowAcceptedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 6];
	    else if (global)
		*value = sir->cs.gbl_stats[3];
	    break;
	case bcmCosqGportYellowAcceptedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 7];
	    else if (global)
		*value = sir->cs.gbl_stats[2];
	    break;
	case bcmCosqGportYellowCongestionMarkedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 8];
	    else if (global)
		*value = sir->cs.gbl_stats[19];
	    break;
	case bcmCosqGportYellowCongestionMarkedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 9];
	    else if (global)
		*value = sir->cs.gbl_stats[18];
	    break;
	case bcmCosqGportYellowDiscardDroppedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 10];
	    else if (global)
		*value = sir->cs.gbl_stats[11];
	    break;
	case bcmCosqGportYellowDiscardDroppedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 11];
	    else if (global)
		*value = sir->cs.gbl_stats[10];
	    break;
	case bcmCosqGportYellowDroppedPkts:
	    if (global)
		*value = sir->cs.gbl_stats[27];
	    break;
	case bcmCosqGportYellowDroppedBytes:
	    if (global)
		*value = sir->cs.gbl_stats[26];
	    break;
	case bcmCosqGportRedAcceptedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 12];
	    else if (global)
		*value = sir->cs.gbl_stats[5];
	    break;
	case bcmCosqGportRedAcceptedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 13];
	    else if (global)
		*value = sir->cs.gbl_stats[4];
	    break;
	case bcmCosqGportRedCongestionMarkedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 14];
	    else if (global)
		*value = sir->cs.gbl_stats[21];
	    break;
	case bcmCosqGportRedCongestionMarkedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 15];
	    else if (global)
		*value = sir->cs.gbl_stats[20];
	    break;
	case bcmCosqGportRedDiscardDroppedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 16];
	    else if (global)
		*value = sir->cs.gbl_stats[13];
	    break;
	case bcmCosqGportRedDiscardDroppedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 17];
	    else if (global)
		*value = sir->cs.gbl_stats[12];
	    break;
	case bcmCosqGportRedDroppedPkts:
	    if (global)
		*value = sir->cs.gbl_stats[29];
	    break;
	case bcmCosqGportRedDroppedBytes:
	    if (global)
		*value = sir->cs.gbl_stats[28];
	    break;
	case bcmCosqGportBlackAcceptedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 18];
	    else if (global)
		*value = sir->cs.gbl_stats[7];
	    break;
	case bcmCosqGportBlackAcceptedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 19];
	    else if (global)
		*value = sir->cs.gbl_stats[6];
	    break;
	case bcmCosqGportBlackCongestionMarkedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 20];
	    else if (global)
		*value = sir->cs.gbl_stats[23];
	    break;
	case bcmCosqGportBlackCongestionMarkedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 21];
	    else if (global)
		*value = sir->cs.gbl_stats[22];
	    break;
	case bcmCosqGportBlackDiscardDroppedPkts:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 22];
	    else if (global)
		*value = sir->cs.gbl_stats[15];
	    break;
	case  bcmCosqGportBlackDiscardDroppedBytes:
	    if (ingress)
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    else if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 23];
	    else if (global)
		*value = sir->cs.gbl_stats[14];
	    break;
	case bcmCosqGportBlackDroppedPkts:
	    if (global)
		*value = sir->cs.gbl_stats[31];
	    break;
	case bcmCosqGportBlackDroppedBytes:
	    if (global)
		*value = sir->cs.gbl_stats[30];
	    break;
	case bcmCosqGportNonWredDroppedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 24];
	    break;
	case bcmCosqGportNonWredDroppedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 25];
	    break;
	case bcmCosqGportDequeuedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 30];
	    else if (global)
		*value = sir->cs.gbl_stats[33];
	    else
		*value = sir->cs.segment[segment].ullCount[cntrId*2];
	    break;
	case bcmCosqGportDequeuedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 31];
	    else if (global)
		*value = sir->cs.gbl_stats[32];
	    else
		*value = sir->cs.segment[segment].ullCount[cntrId*2 + 1];
	    break;
	case bcmCosqGportOverSubscribeTotelDroppedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 26];
	    break;
	case bcmCosqGportOverSubscribeTotalDroppedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 27];
	    break;
	case bcmCosqGportOverSubscribeGuaranteeDroppedPkts:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 28];
	    break;
	case bcmCosqGportOverSubscribeGuaranteedDroppedBytes:
	    if (slq)
		*value = sir->cs.slq_stats[cosq*16 + 29];
	    break;
	default:
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "gport_profile_get: unit %d unknown mode\n"),
	               unit));
	    return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

int
_bcm_sirius_cosq_gport_statistic_multi_set(int unit,
                                           bcm_gport_t gport,
                                           bcm_gport_t lgl_gport,
                                           bcm_cos_queue_t cosq,
                                           int stat_count,
                                           bcm_cosq_gport_stats_t *stat_info,
                                           uint64 value)
{
    int rv = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    bcm_sbx_cosq_queue_state_t *q_state = NULL;
    bcm_sbx_cosq_bw_group_state_t *bw_state = NULL;
    statscfg_entry_t   statsCfg;
    ep_stats_ctrl_entry_t    ep_stats_ctrl;
    bcm_trunk_t tid = 0;
    int i = 0, j = 0, k = 0, loopcnt = 1;
    sbBool_t logical = (lgl_gport != BCM_GPORT_TYPE_NONE) ? 1 : 0;
    uint8 bit_size = 0, level = 0, type = 0;
    uint16 cntrId = 0;
    int segment = -1;
    uint32 queue_base = 0, dp = 0, regval = 0;
    int intf = 0, index = 0, mod = 0, port = 0, idx = 0, num_cos = 1, stat = 0, eg_n = -1, cos = -1;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;

    /*
     * check to see if valid
     */
    if (!SOC_IS_SBX_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    /*
     * this routine only supports clearing statistics.
     * Therefore, value must be zero.
     *
     */

    if (COMPILER_64_IS_ZERO(value) == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "stat_multi_set: unit %d value %08x%08x not 0\n"),
                   unit, COMPILER_64_HI(value), COMPILER_64_LO(value)));
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_EGRESS_GROUP(gport) ||
        BCM_GPORT_IS_EGRESS_CHILD(gport) ||
        BCM_GPORT_IS_EGRESS_MODPORT(gport)) {

        for (j = 0; j < stat_count; j++) {
            /*
             * Get stat from stats array
             */

            stat = stat_info[j];

            /*
             * Egress statistics
             */

            /*
             * Get counter and level
             */
            rv = bcm_sirius_get_egress_info(unit, gport, &bit_size, &level, &cntrId, &intf);
            if (rv != BCM_E_NONE) {
                return rv;
            }

            if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
                mod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
                port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
            } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
                mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
                port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
            } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
                mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
                port = -1;
                rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, &cos);
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "Invalid gport(0x%x). Unable to resolve a port\n"),
                               gport));
                    return rv;
                }
                idx = cosq;
                if (cosq == -1) {
                    num_cos = cos;
                }
            }

            rv = bcm_sirius_trunk_find_and_get(unit,
                                               mod,
                                               gport,
                                               &tid,
                                               &trunkInfo,
                                               BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                               &(trunkMembers[0]),
                                               &memberCount);
            if (rv == BCM_E_NONE) {
                if ((memberCount > 0) && (memberCount <= SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS)) {
                    loopcnt = memberCount;
                } else {
                    loopcnt = 1;
                }
            } else {
                loopcnt = 1;
            }

            for (i=0; i < loopcnt; i++) {
                /*
                 * If member of trunk, loop through each member subport.
                 * Otherwise, pass through once using cntrId;
                 */
            
                if (rv == BCM_E_NONE) {
                    BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, trunkMembers[i].gport, &bit_size, &level, &cntrId, &intf));
                }
                cntrId += idx;
            
                /*
                 * find the type associated with this interface
                 */
                for (type = 0; type < 16; type++) {
                    if (sir->type_res_flags[PRED_TYPE_EP][type] & (1 << (intf + 4))) {
                        break;
                    }
                }
                /*
                 * If no type found, return an error
                 */
                if (type == 16) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "stat_multi_set: unit %d Egress type unknown gport %d"
                                           " stat %d\n"), unit, gport, stat));
                    return BCM_E_UNAVAIL;
                }

                /*
                 * Find the segment matching level for STAT0 or STAT1 at index TYPE
                 */
                BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, type, &ep_stats_ctrl));
                if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
                    (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf) == level)) {
                    segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_SEGMENTf);
                } else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) != 0) &&
                           (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_META0_SELf) == level)) {
                    segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_SEGMENTf);
                }
            
                /*
                 * If no level is configured for this type, then return a message
                 */
                if (segment < 0) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "stat_multi_set: unit %d Egress level %d not configured on"
                                           " type %d\n"), unit, level, type));
                    return BCM_E_UNAVAIL;
                }
            
                /*
                 * Validate the cntrId is configured on this segment.
                 * In egress, cntrId can never exceed 32 bits, so don't need to do
                 * complicated 64 bit manipulation.
                 */
                if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap,(cntrId >> CS_CFG_GRP_SHIFT))) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "stat_multi_set: unit %d stat %d for level %d not currently"
                                           " configured\n"), unit, cntrId, level));
                    return BCM_E_PARAM;
                }
            
                soc_sbx_sirius_flush_segment(unit, segment, cntrId, 1);
            
                for (k = 0; k < num_cos; k++) {
                    switch(stat) {
                    case bcmCosqGportOutPkts:

                        soc_intr_disable(unit, IRQ_FIFO_CH0_DMA);
                        COMPILER_64_ADD_64(value,  sir->cs.segment[segment].ullCount[(cntrId+k)*2]);
                        COMPILER_64_SET(sir->cs.segment[segment].ullCount[(cntrId+k)*2], 0, 0);
                        soc_intr_enable(unit, IRQ_FIFO_CH0_DMA);
                        break;
                    case bcmCosqGportOutBytes:
                
                        soc_intr_disable(unit, IRQ_FIFO_CH0_DMA);
                        COMPILER_64_ADD_64(value,  sir->cs.segment[segment].ullCount[(cntrId+k)*2+1]);
                        COMPILER_64_SET(sir->cs.segment[segment].ullCount[(cntrId+k)*2+1], 0, 0);
                        soc_intr_enable(unit, IRQ_FIFO_CH0_DMA);
                        break;
                    default:
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "stat_multi_set: unit %d Egress stat %d invalid\n"),
                                   unit, stat));
                        rv = BCM_E_PARAM;
                    }
                }
            }
        }
        return rv;
    }

    /*
     * If ingress, is this request physical or logical
     */

    if (logical) {
        if (BCM_GPORT_IS_UCAST_QUEUE_GROUP (lgl_gport) ||
            BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
            BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
            BCM_GPORT_IS_MCAST_QUEUE_GROUP (lgl_gport)) {

            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form sysport/qid */
                queue_base = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(lgl_gport);
            } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form qid */
                queue_base = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
            } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form qid */
                queue_base = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
            } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form sysport/qid */
                queue_base = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(lgl_gport);
            }

            /*
             * if Cosq is -1,
             * then operate on all cosq queues for this queue group
             */

            if (cosq == -1) {
                q_state =  &(SOC_SBX_STATE(unit)->queue_state[queue_base]);
                bw_state = &SOC_SBX_STATE(unit)->bw_group_state[q_state->bw_group];
                num_cos = bw_state->num_cos;
                cntrId = queue_base;
            } else {
                cntrId = queue_base + cosq;
            }
        }
    } else {
        /*
         * requesting physical stat value
         * get the physical interface
         */
        rv = bcm_sbx_port_get_intf_portoffset(unit, gport, &intf, &index);
        if (rv != BCM_E_NONE) {
            return rv;
        }
	
        if (cosq == -1) {
            num_cos = 4;
        }
        cntrId = intf;
    }

    /*
     * For multi stats, if cosq == -1, then iterate through
     * the cosqs, otherwise only iterate once for cntrid
     */
    for (k = 0; k < num_cos; k++) {
        for (j = 0; j < stat_count; j++) {
            /*
             * Get stat from stats array
             */

            stat = stat_info[j];

            /*
             * Find the segment matching the shared or unshared
             * DP value for this interface
             */


            switch(stat) {
            case bcmCosqGportGreenAcceptedPkts:
            case bcmCosqGportGreenAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_GREEN, &dp));
                break;
            case bcmCosqGportYellowAcceptedPkts:
            case bcmCosqGportYellowAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_YELLOW, &dp));
                break;
            case bcmCosqGportRedAcceptedPkts:
            case bcmCosqGportRedAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_RED, &dp));
                break;
            case bcmCosqGportBlackAcceptedPkts:
            case bcmCosqGportBlackAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_BLACK, &dp));
                break;
            case bcmCosqGportGreenDiscardDroppedPkts:
            case bcmCosqGportGreenDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_GREEN, &dp));
                break;
            case bcmCosqGportYellowDiscardDroppedPkts:
            case bcmCosqGportYellowDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_YELLOW, &dp));
                break;
            case bcmCosqGportRedDiscardDroppedPkts:
            case bcmCosqGportRedDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_RED, &dp));
                break;
            case bcmCosqGportBlackDiscardDroppedPkts:
            case bcmCosqGportBlackDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_BLACK, &dp));
                break;
	        case bcmCosqGportDequeuedPkts:
	        case bcmCosqGportDequeuedBytes:
                dp = 8;
                break;
            default:
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_set: unit %d Ingress stat %d not supported in multi mode\n"),
                           unit, stat));
                rv = BCM_E_UNAVAIL;
                continue;
            }

            /*
             * Find the segment associated with this physical interface
             */

            if (dp < 8) {
                BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, dp, &statsCfg));
                if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf)) {
                    segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
                } else {
                    continue;
                }
            } else {
                rv = READ_QM_CS_CONFIG0r(unit, &regval);
                if (rv == SOC_E_NONE) {
                    if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf)) {
                        segment = soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf);
                    } else {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "stat_set: unit %d Ingress stat %d not configured\n"),
                                   unit, stat));
                        rv = BCM_E_UNAVAIL;
                        continue;
                    }
                } else {
                    continue;
                }
            }

            /*
             * Validate the cntrId is configured on this segment
             */

            if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId>>CS_CFG_GRP_SHIFT))) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_set: unit %d Ingress stat %d for segment %d not currently"
                                       " configured\n"), unit, stat, segment));
                continue;
            }

            /*
             * set the appropriate statistic.
             */

            switch(stat) {
            case bcmCosqGportGreenAcceptedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportGreenAcceptedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportGreenDiscardDroppedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportGreenDiscardDroppedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportYellowAcceptedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportYellowAcceptedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportYellowDiscardDroppedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportYellowDiscardDroppedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportRedAcceptedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportRedAcceptedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportRedDiscardDroppedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportRedDiscardDroppedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportBlackAcceptedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case bcmCosqGportBlackAcceptedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
            case bcmCosqGportBlackDiscardDroppedPkts:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2] = value;
                break;
            case  bcmCosqGportBlackDiscardDroppedBytes:
                sir->cs.segment[segment].ullCount[(cntrId + k)*2 + 1] = value;
                break;
	        case bcmCosqGportDequeuedPkts:
                sir->cs.segment[segment].ullCount[(cntrId+k)*2] = value;
	            break;
	        case bcmCosqGportDequeuedBytes:
                sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1] = value;
                break;
            }
        }
    }
    return rv;
}

int
_bcm_sirius_cosq_gport_statistic_multi_get(int unit,
					   bcm_gport_t gport,
					   bcm_gport_t lgl_gport,
					   bcm_cos_queue_t cosq,
					   int stat_count,
					   bcm_cosq_gport_stats_t *stats_array,
					   int value_count,
					   uint64 *value_array)
{
    int rv = BCM_E_NONE;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    bcm_sbx_cosq_queue_state_t *q_state = NULL;
    bcm_sbx_cosq_bw_group_state_t *bw_state = NULL;
    statscfg_entry_t   statsCfg;
    ep_stats_ctrl_entry_t    ep_stats_ctrl;
    bcm_trunk_t tid = 0;
    int i = 0, j = 0, k = 0, loopcnt = 1;
    sbBool_t logical = (lgl_gport != BCM_GPORT_TYPE_NONE) ? 1 : 0;
    uint8 bit_size = 0, level = 0, type = 0;
    uint16 cntrId = 0;
    int segment = -1;
    uint32 queue_base = 0, dp = 0, regval = 0;
    int intf = 0, index = 0, mod = 0, port = 0, idx = 0, num_cos = 1, stat = 0, eg_n = -1, cos = -1;
    uint64 value = COMPILER_64_INIT(0,0);
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;

    /*
     * check to see if valid
     */
    if (!SOC_IS_SBX_SIRIUS(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (BCM_GPORT_IS_EGRESS_GROUP(gport) ||
        BCM_GPORT_IS_EGRESS_CHILD(gport) ||
        BCM_GPORT_IS_EGRESS_MODPORT(gport)) {

        COMPILER_64_ZERO(value);

        /*
         * Get counter and level
         */
        BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, gport, &bit_size, &level, &cntrId, &intf));

        /*
         * Are we currently part of a trunk?
         */

        if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
            mod = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
        } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
            mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
            port = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
        } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
            port = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, &cos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "Invalid gport(0x%x). Unable to resolve a port\n"),
                           gport));
                return rv;
            }
            idx = cosq;
            if (cosq == -1) {
                num_cos = cos;
            }
        }

        if (value_count < (num_cos * stat_count)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "stat_multi_get: unit %d value_count %d less than stats requested %d\n"),
                       unit, value_count, (num_cos * stat_count)));
            return BCM_E_PARAM;
        }

        /*
         * Initialize value_array to -1
         */
        for (j=0; j < value_count; j++) {
            COMPILER_64_SET(value_array[j], -1, -1);
        }

        rv = bcm_sirius_trunk_find_and_get(unit,
                                           mod,
                                           gport,
                                           &tid,
                                           &trunkInfo,
                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                           &(trunkMembers[0]),
                                           &memberCount);
        if (rv == BCM_E_NONE) {
            if ((memberCount > 0) && (memberCount <= SB_FAB_DEVICE_SIRIUS_NUM_HG_FABRIC_PORTS)) {
                loopcnt = memberCount;
            } else {
                loopcnt = 1;
            }
        } else {
            loopcnt = 1;
        }

        for (i=0; i < loopcnt; i++) {
            /*
             * If member of trunk, loop through each member subport.
             * Otherwise, pass through once using cntrId;
             */
            if (rv == BCM_E_NONE) {
                BCM_IF_ERROR_RETURN(bcm_sirius_get_egress_info(unit, trunkMembers[i].gport, &bit_size, &level, &cntrId, &intf));
            }
            cntrId += idx;

            /*
             * find the type associated with this interface
             */
            for (type = 0; type < 16; type++)
                if (sir->type_res_flags[PRED_TYPE_EP][type] & (1 << (intf + 4)))
                    break;

            /*
             * If no type found, return an error
             */
            if (type == 16) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_get: unit %d Egress type unknown gport %d\n"),
                           unit, gport));
                return BCM_E_UNAVAIL;
            }

            /*
             * Find the segment matching level for STAT0 or STAT1 at index TYPE
             */
            BCM_IF_ERROR_RETURN(READ_EP_STATS_CTRLm(unit, MEM_BLOCK_ANY, type, &ep_stats_ctrl));
            if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_LENGTH0f) != 0) &&
                (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_META0_SELf) == level))
                segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT0_SEGMENTf);
            else if ((soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_LENGTH0f) != 0) &&
                     (soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_META0_SELf) == level))
                segment = soc_mem_field32_get(unit, EP_STATS_CTRLm, &ep_stats_ctrl, STAT1_SEGMENTf);

            /*
             * If no level is configured for this type, then return a message
             */
            if (segment < 0) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_get: unit %d Egress level %d not configured on type %d\n"),
                           unit, level, type));
                return BCM_E_UNAVAIL;
            }

            /*
             * Validate the cntrId is configured on this segment.
             * On egress, cntrId can never exceed 32K,so don't need to do complicated 64 bit manipulation.
             */
            if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId >> CS_CFG_GRP_SHIFT))) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_get: unit %d stat %d for level %d not currently"
                                       " configured\n"), unit, cntrId, level));
                return BCM_E_PARAM;
            }

            soc_sbx_sirius_flush_segment(unit, segment, cntrId, 1);

            for (k = 0; k < num_cos; k++) {
                for (j = 0; j < stat_count; j++) {

                    /*
                     * Get stat from stats array
                     */

                    stat = stats_array[j];
                    COMPILER_64_SET(value, -1, -1);

                    switch (stat) {
                    case bcmCosqGportOutPkts:
                        COMPILER_64_ADD_64(value,  sir->cs.segment[segment].ullCount[(cntrId+k)*2]);
                        break;
                    case bcmCosqGportOutBytes:
                        COMPILER_64_ADD_64(value,  sir->cs.segment[segment].ullCount[(cntrId+k)*2+1]);
                        break;
                    default:
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "stat_multi_get: unit %d Egress stat %d invalid\n"),
                                   unit, stat));
                        return BCM_E_PARAM;
                    }
                }
                value_array[k*stat_count + j] =  value;
            }
        }
        return BCM_E_NONE;
    }

    /*
     * If ingress, is this request physical or logical
     */

    if (logical) {
        if (BCM_GPORT_IS_UCAST_QUEUE_GROUP (lgl_gport) ||
            BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
            BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (lgl_gport) ||
            BCM_GPORT_IS_MCAST_QUEUE_GROUP (lgl_gport)) {

            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form sysport/qid */
                queue_base = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(lgl_gport);
            } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form qid */
                queue_base = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
            } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form qid */
                queue_base = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(lgl_gport);
            } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(lgl_gport)) {
                /* The gport is of the form sysport/qid */
                queue_base = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(lgl_gport);
            }

            /*
             * if Cosq is -1,
             * then operate on all cosq queues for this queue group
             */

            if (cosq == -1) {
                q_state =  &(SOC_SBX_STATE(unit)->queue_state[queue_base]);
                bw_state = &SOC_SBX_STATE(unit)->bw_group_state[q_state->bw_group];
                num_cos = bw_state->num_cos;
                cntrId = queue_base;
            } else {
                cntrId = queue_base + cosq;
            }
        }
    } else {
        /*
         * requesting physical stat value
         * get the physical interface
         */
        rv = bcm_sbx_port_get_intf_portoffset(unit, gport, &intf, &index);
        if (rv != BCM_E_NONE) {
            return rv;
        }
	
        if (cosq == -1) {
            num_cos = 4;
        }
        cntrId = intf;
    }

    if (value_count < (num_cos * stat_count)) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "stat_multi_get: unit %d value_count %d less than stats requested %d\n"),
                   unit, value_count, (num_cos * stat_count)));
        return BCM_E_PARAM;
    }

    /*
     * Initialize value_array to -1
     */
    for (j=0; j < value_count; j++) {
        COMPILER_64_SET(value_array[j], -1, -1);
    }

    /*
     * For multi stats, if cosq == -1, then iterate through
     * the cosqs, otherwise only iterate once for cntrid
     */
    for (k = 0; k < num_cos; k++) {
        for (j = 0; j < stat_count; j++) {
            /*
             * Get stat from stats array
             */

            stat = stats_array[j];
            COMPILER_64_SET(value, -1, -1);

            /*
             * Find the segment matching the shared or unshared
             * DP value for this interface
             */


            switch(stat) {
            case bcmCosqGportGreenAcceptedPkts:
            case bcmCosqGportGreenAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_GREEN, &dp));
                break;
            case bcmCosqGportYellowAcceptedPkts:
            case bcmCosqGportYellowAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_YELLOW, &dp));
                break;
            case bcmCosqGportRedAcceptedPkts:
            case bcmCosqGportRedAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_RED, &dp));
                break;
            case bcmCosqGportBlackAcceptedPkts:
            case bcmCosqGportBlackAcceptedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, ADD_BLACK, &dp));
                break;
            case bcmCosqGportGreenDiscardDroppedPkts:
            case bcmCosqGportGreenDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_GREEN, &dp));
                break;
            case bcmCosqGportYellowDiscardDroppedPkts:
            case bcmCosqGportYellowDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_YELLOW, &dp));
                break;
            case bcmCosqGportRedDiscardDroppedPkts:
            case bcmCosqGportRedDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_RED, &dp));
                break;
            case bcmCosqGportBlackDiscardDroppedPkts:
            case bcmCosqGportBlackDiscardDroppedBytes:
                BCM_IF_ERROR_RETURN(_bcm_sirius_qm_color_to_dp(unit, DROP_BLACK, &dp));
                break;
	        case bcmCosqGportDequeuedPkts:
	        case bcmCosqGportDequeuedBytes:
                dp = 8;
                break;
            default:
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_get: unit %d Ingress stat %d not supported in multi mode\n"),
                           unit, stat));
                rv = BCM_E_UNAVAIL;
                continue;
            }

            /*
             * Find the type associated with this physical interface
             */
	   
            if (dp < 8) {
                BCM_IF_ERROR_RETURN(READ_STATSCFGm(unit, MEM_BLOCK_ANY, dp, &statsCfg));
                if (soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_EVENT_SELf)) {
                    segment = soc_mem_field32_get(unit, STATSCFGm, &statsCfg, RECORD0_SEGMENT_SELf);
                } else {
                    continue;
                }
            } else {
                rv = READ_QM_CS_CONFIG0r(unit, &regval);
                if (rv == SOC_E_NONE) {
                    if (soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf)) {
                        segment = soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf);
                    } else {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "stat_get: unit %d Ingress stat %d not configured\n"),
                                   unit, stat));
                        rv = BCM_E_UNAVAIL;
                        continue;
                    }
                } else {
                    continue;
                }
            }

            /*
             * Validate the cntrId is configured on this segment
             */
            if (!COMPILER_64_BITTEST(sir->cs.segment[segment].cntrIdMap, (cntrId>>CS_CFG_GRP_SHIFT))) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "stat_multi_get: unit %d Ingress stat %d for segment %d not currently"
                                       " configured\n"), unit, stat, segment));
                continue;
            }

            /*
             * get the appropriate statistic.
             */

            switch(stat) {

            case bcmCosqGportGreenAcceptedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportGreenAcceptedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportGreenDiscardDroppedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportGreenDiscardDroppedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportYellowAcceptedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportYellowAcceptedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportYellowDiscardDroppedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportYellowDiscardDroppedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportRedAcceptedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportRedAcceptedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportRedDiscardDroppedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportRedDiscardDroppedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportBlackAcceptedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case bcmCosqGportBlackAcceptedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            case bcmCosqGportBlackDiscardDroppedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
            case  bcmCosqGportBlackDiscardDroppedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
	        case bcmCosqGportDequeuedPkts:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2];
                break;
	        case bcmCosqGportDequeuedBytes:
                value = sir->cs.segment[segment].ullCount[(cntrId+k)*2 + 1];
                break;
            }
            value_array[k*stat_count + j] =  value;
        }
    }
    return rv;
}

int
bcm_sirius_cosq_gport_stat_config_set(int unit,
				      bcm_gport_t gport,
				      bcm_gport_t lgl_gport,
				      bcm_cos_queue_t cosq,
				      uint32 flags,
				      bcm_cosq_gport_stat_profile_t profile)
{
  return (_bcm_sirius_cosq_gport_stat_config_set(unit, gport, lgl_gport, cosq, flags, profile));
}

int
bcm_sirius_cosq_gport_stat_config_get(int unit,
				      bcm_gport_t gport,
				      bcm_gport_t lgl_gport,
				      bcm_cos_queue_t cosq,
				      uint32 flags,
				      bcm_cosq_gport_stat_profile_t *profile)
{
  return (_bcm_sirius_cosq_gport_stat_config_get(unit, gport, lgl_gport, cosq, flags, profile));
}

int
bcm_sirius_cosq_gport_statistic_set(int unit,
				    bcm_gport_t gport,
				    bcm_gport_t lgl_gport,
				    bcm_cos_queue_t cosq,
				    uint32 flags,
				    bcm_cosq_gport_stats_t stat,
				    uint64 value)
{
  return (_bcm_sirius_cosq_gport_statistic_set(unit, gport, lgl_gport, cosq, flags, stat, value));
}

int
bcm_sirius_cosq_gport_statistic_get(int unit,
				    bcm_gport_t gport,
				    bcm_gport_t lgl_gport,
				    bcm_cos_queue_t cosq,
				    uint32 flags,
				    bcm_cosq_gport_stats_t stat,
				    uint64 *value)
{
  return (_bcm_sirius_cosq_gport_statistic_get(unit, gport, lgl_gport, cosq, flags, stat, value));
}

int
bcm_sirius_cosq_gport_statistic_multi_set(int unit,
					  bcm_gport_t gport,
					  bcm_gport_t lgl_gport,
					  bcm_cos_queue_t cosq,
					  int stat_count,
					  bcm_cosq_gport_stats_t *stats_array,
					  uint64 value)
{
  return (_bcm_sirius_cosq_gport_statistic_multi_set(unit, gport, lgl_gport, cosq, stat_count, stats_array, value));
}

int
bcm_sirius_cosq_gport_statistic_multi_get(int unit,
					  bcm_gport_t gport,
					  bcm_gport_t lgl_gport,
					  bcm_cos_queue_t cosq,
					  int stat_count,
					  bcm_cosq_gport_stats_t *stats_array,
					  int value_count,
					  uint64 *value_array)
{
  return (_bcm_sirius_cosq_gport_statistic_multi_get(unit,
						     gport,
						     lgl_gport,
						     cosq,
						     stat_count,
						     stats_array,
						     value_count,
						     value_array));
}

int
bcm_sirius_cosq_sysport_port_remap(int unit, int sysport, int ef, int remap_port)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

int
bcm_sirius_cosq_state_get(int unit,
			  char *pbuf)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}


int
bcm_sirius_cosq_egress_size_set(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 bytes_min,
                                uint32 bytes_max)
{
    int rc = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int fcd = BCM_INT_SBX_INVALID_FCD, port = 0, eg_n = 0, num_fifos = 0;
    uint32 regval = 0;

    /* Global Initialization */
    if (gport == BCM_GPORT_INVALID) {
	if (cosq == 0) {        /* Global */
	    soc_reg_field_set(unit, FR_FLOW_CTL_GLOBALr, &regval, GLOBAL_THRESH_LOf, bytes_min);
	    soc_reg_field_set(unit, FR_FLOW_CTL_GLOBALr, &regval, GLOBAL_THRESH_HIf, bytes_max);
	    BCM_IF_ERROR_RETURN(WRITE_FR_FLOW_CTL_GLOBALr(unit, regval));
	} else if (cosq == 1) { /* Unicast Global */
	    soc_reg_field_set(unit, FR_FLOW_CTL_UNICASTr, &regval, UC_THRESH_LOf, bytes_min);
	    soc_reg_field_set(unit, FR_FLOW_CTL_UNICASTr, &regval, UC_THRESH_HIf, bytes_max);
	    BCM_IF_ERROR_RETURN(WRITE_FR_FLOW_CTL_UNICASTr(unit, regval));
	} else {
	    return BCM_E_PARAM;
	}
	return BCM_E_NONE;
    }

    /* Multicast fifos */
    if (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) {
	fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS - 1;
    } else if (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo) {
	fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
    }

    if (fcd != BCM_INT_SBX_INVALID_FCD) {
	rc = bcm_sirius_port_multicast_size_set(unit, fcd, cosq, bytes_min, bytes_max);
	return rc;
    }

    /* Unicast fifos */
    port = -1;

    rc = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, &num_fifos);
    if (rc != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, gport 0x%x does not contain fabric_port, unit %d\n"),
	           FUNCTION_NAME(), gport, unit));
	return BCM_E_PARAM;
    }

    if (cosq >= num_fifos) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "cos level out of range (0-%d).\n"),
                   num_fifos-1));
        return BCM_E_PARAM;
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);

    if ( sp_info->egroup[eg_n].fcd[cosq] == BCM_INT_SBX_INVALID_FCD) {
	if  (cosq == SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo) {
	    fcd = sp_info->egroup[eg_n].ef_fcd;
	} else {
	    fcd = sp_info->egroup[eg_n].nef_fcd;
	}
    } else {
	fcd = ATTACH_ID_FCD_GET(sp_info->egroup[eg_n].fcd[cosq]);
    }

    rc = bcm_sirius_port_size_set(unit, fcd, cosq, bytes_min, bytes_max);
    return rc;
}

int
bcm_sirius_cosq_egress_size_get(int unit,
                                bcm_gport_t gport,
                                bcm_cos_queue_t cosq,
                                uint32 *bytes_min,
                                uint32 *bytes_max)
{
    int rc = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int fcd = BCM_INT_SBX_INVALID_FCD, port = 0, eg_n = 0, num_fifos = 0;
    uint32 regval = 0;

    /* Global Initialization */
    if (gport == BCM_GPORT_INVALID) {
	if (cosq == 0) {        /* Global */
	    BCM_IF_ERROR_RETURN(READ_FR_FLOW_CTL_GLOBALr(unit, &regval));
	    *bytes_min = soc_reg_field_get(unit, FR_FLOW_CTL_GLOBALr, regval, GLOBAL_THRESH_LOf);
	    *bytes_max = soc_reg_field_get(unit, FR_FLOW_CTL_GLOBALr, regval, GLOBAL_THRESH_HIf);
	} else if (cosq == 1) { /* Unicast Global */
	    BCM_IF_ERROR_RETURN(READ_FR_FLOW_CTL_UNICASTr(unit, &regval));
	    *bytes_min = soc_reg_field_get(unit, FR_FLOW_CTL_UNICASTr, regval, UC_THRESH_LOf);
	    *bytes_max = soc_reg_field_get(unit, FR_FLOW_CTL_UNICASTr, regval, UC_THRESH_HIf);
	} else {
	    return BCM_E_PARAM;
	}
	return BCM_E_NONE;
    }

    /* Multicast fifos */
    if (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) {
	fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS - 1;
    } else if (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo) {
	fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
    }

    if (fcd != BCM_INT_SBX_INVALID_FCD) {
        rc = bcm_sirius_port_multicast_size_get(unit, fcd, cosq, bytes_min, bytes_max);
	return rc;
    }

    /* Unicast fifos */
    port = -1;

    rc = bcm_sbx_cosq_egress_group_info_get(unit, gport, &port, &eg_n, &num_fifos);
    if (rc != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, gport 0x%x does not contain fabric_port, unit %d\n"),
	           FUNCTION_NAME(), gport, unit));
	return BCM_E_PARAM;
    }

    if (cosq >= num_fifos) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "cos level out of range (0-%d).\n"),
                   num_fifos-1));
        return BCM_E_PARAM;
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);

    if ( sp_info->egroup[eg_n].fcd[cosq] == BCM_INT_SBX_INVALID_FCD) {
	if  (cosq == SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo) {
	    fcd = sp_info->egroup[eg_n].ef_fcd;
	} else {
	    fcd = sp_info->egroup[eg_n].nef_fcd;
	}
    } else {
	fcd = ATTACH_ID_FCD_GET(sp_info->egroup[eg_n].fcd[cosq]);
    }

    rc = bcm_sirius_port_size_get(unit, fcd, cosq, bytes_min, bytes_max);
    return rc;
}

int
bcm_sirius_cosq_attach_scheduler(int unit,
				 bcm_gport_t parent_gport,
				 bcm_gport_t child_gport,
				 bcm_cos_queue_t cosq,
				 int egress)
{
    int rv = BCM_E_UNAVAIL;
    int child_level = 0, parent_level = 0;
    int child_node = 0, parent_node = 0, num_fifos = -1;
    int node, base_node, queue, base_queue, cs_l = -1, ps_l = -1, eg_n = 0, subport = 0;
    int num_child_level1, num_queues, num_nodes, num_level1_nodes, level1_nodes[4], hide_level1 = FALSE;
    sbBool_t topDown = TRUE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *ps_i_state;
    bcm_sbx_sirius_egress_scheduler_state_t *ps_e_state;
    bcm_sbx_sirius_egress_scheduler_state_t *cs_e_state;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    uint16 dev_id;
    uint8 rev_id;
    int parent, first_child, num_child;
    bcm_gport_t first_child_gport;
    uint32 uRegValue = 0;
    uint32 old_shaper_loop = 0, old_shaper_start = 0, old_shaper_end = 0;
    int l4_node = -1, l5_node = -1, l6_node = -1, l7_node = -1, l6_old_profile = -1;
    int l4_pri, l5_pri, l6_pri, l7_pri;
    int l4_offset = -1, l5_offset = -1, l6_offset = -1;
    l6_nm_entry_t l6_nm_entry;
    l4_bk_entry_t l4_bk_entry;

#ifndef NOTDEF_NEW_INTF
    l4_n0_entry_t l4_n0_entry;
    l5_n0_entry_t l5_n0_entry;
    l6_n0_entry_t l6_n0_entry;
    l7_n0_entry_t l7_n0_entry;
    rt_st_entry_t rt_st_entry;
    int rt_pri, child_pri, current_pri, current_child, tmp_node;
    soc_field_t child_pri_fields[8] = {CHILD_PRI0f, CHILD_PRI1f, CHILD_PRI2f, CHILD_PRI3f,
				       CHILD_PRI4f, CHILD_PRI5f, CHILD_PRI6f, CHILD_PRI7f};
#endif /* NOTDEF_NEW_INTF */

    LOG_DEBUG(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "parent_gport: 0x%x, child_gport: 0x%x\n"),
               parent_gport, child_gport));

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	/* egress scheduler attach */
	if (BCM_GPORT_IS_SCHEDULER(parent_gport) ||
	    BCM_GPORT_IS_EGRESS_CHILD(parent_gport) ||
	    BCM_GPORT_IS_EGRESS_MODPORT(parent_gport)) {
	    /* allow level 2 and level 1 be scheduler type */
	    rv = bcm_sbx_port_get_scheduler(unit, parent_gport, &parent_level, &parent_node);
	    if ((rv != BCM_E_NONE) ||(parent_level < 0) || (parent_node < 0))  {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), parent_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    if (BCM_GPORT_IS_SCHEDULER(parent_gport)) {
		ps_l = BCM_INT_SBX_SCHEDULER_ID_GET(parent_gport);
	    }
	} else {
	    /* unsupported ingress child gport type */
	    return BCM_E_PARAM;
	}

	if (BCM_GPORT_IS_SCHEDULER(child_gport)) {
	    /* allow level 2 and level 1 be scheduler type */
	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    cs_l = BCM_INT_SBX_SCHEDULER_ID_GET(child_gport);
	    child_level = parent_level - 1;
	} else if ( BCM_GPORT_IS_EGRESS_GROUP(child_gport) ) {
	    if ( !BCM_GPORT_IS_SCHEDULER(parent_gport) ) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, egress group must attach to a scheduler gport,  Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_PARAM;
	    }

	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    subport = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, child_gport, &subport, &eg_n, &num_fifos);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		           FUNCTION_NAME(), subport, child_gport, unit));
		return BCM_E_PARAM;
	    }
	    
	    cs_l = BCM_GPORT_EGRESS_GROUP_GET(child_gport);
	    SOC_SBX_STATE(unit)->egress_group_state[cs_l].eg_scheduler = ps_l;
	} else {
	    /* unsupported egress child gport type */
	    return BCM_E_PARAM;
	}

	if  (parent_level != (child_level + 1)) {
	    return BCM_E_PARAM;
	} else if ( (uint) parent_level < (uint) child_level ) {
	    /* not supported for now, do this to make coverity happy */
	    topDown = FALSE;
	}

	if (topDown) {
	    ps_e_state = cs_e_state = NULL;

	    if ((parent_level < 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
		(parent_node >=  SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[parent_level])) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, invalid parent level %d node %d, unit %d\n"),
		           FUNCTION_NAME(), parent_level, parent_node, unit));
		return BCM_E_PARAM;
	    }

	    ps_e_state = &es_state[unit][parent_level][parent_node];
	    if (ps_e_state->in_use == FALSE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Parent node not initialized,  Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_INTERNAL;
	    }

	    switch (child_level) {
		case SIRIUS_ES_LEVEL_FIFO:
		    /* attach all fifo nodes to the parent (subport) node */
		    if ( !BCM_GPORT_IS_SCHEDULER(parent_gport) ) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, Level %d egress scheduler must attach to an egress scheduler gport,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, unit));
			return BCM_E_PARAM;
		    }

		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		    base_node = child_node;
		    for (node = base_node; node < (base_node + num_fifos); node++) {
			if (sp_info->egroup[eg_n].es_scheduler_level0_node == 0xffff) {
			    /* allocate fifo node when using default setup */
			    rv = bcm_sirius_cosq_scheduler_allocate(unit, 0, node, 1, egress, &node);
			    if (rv != BCM_E_NONE) {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, failed to allocation child level %d egress scheduler node,  Unit(%d)\n"),
				           FUNCTION_NAME(), child_level, unit));
				return BCM_E_RESOURCE;
			    }
			}

			rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, child_level, node, -1, TRUE, node - base_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, ES failed to attach fifo node %d\n"),
			               FUNCTION_NAME(), node));
			    return rv;
			}
		    }

		    sp_info->es_scheduler_level1_node[eg_n] = parent_node;

		    if (BCM_GPORT_IS_MODPORT(sp_info->parent_gport)) {
			node = BCM_GPORT_MODPORT_MODID_GET(sp_info->parent_gport) - BCM_MODULE_FABRIC_BASE;
		    } else {
			node = BCM_GPORT_EGRESS_MODPORT_MODID_GET(sp_info->parent_gport) - BCM_MODULE_FABRIC_BASE;
		    }

		    bcm_sirius_cosq_module_congestion_set(unit, sp_info->parent_gport, node);
                    
            /*
		     * Allocate fifo memory resources
		     */
            
            rv = soc_sirius_config_ff(unit, (uint16)subport, (uint8)eg_n, FF_MEM_ALLOCATE);
		    break;
		case SIRIUS_ES_LEVEL_SUBPORT:
		    /* allocate the subport node and attach to parent (channel) node */
		    if ( !BCM_GPORT_IS_EGRESS_CHILD(parent_gport) &&
			 !BCM_GPORT_IS_SCHEDULER(parent_gport) ) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, Level %d egress scheduler must attach to an egress child or scheduler gport,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, unit));
			return BCM_E_PARAM;
		    }

		    rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, child_node, 1, egress, &child_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to allocation child level %d egress scheduler node,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, unit));
			return BCM_E_RESOURCE;
		    }

		    rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, child_level, child_node, -1, TRUE, -1);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to attach subport node %d\n"),
			           FUNCTION_NAME(), child_node));
			return rv;
		    }

		    if (BCM_GPORT_IS_SCHEDULER(child_gport) &&
			(cs_l >= 0) &&
			(SOC_SBX_STATE(unit)->egress_scheduler_state != NULL)) {
			SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].node = child_node;
			SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].level = child_level;			
		    }
		    break;
		case SIRIUS_ES_LEVEL_CHANNEL:
		    /* allocate the channel node and attach to parent (intf) node */
		    if ( !BCM_GPORT_IS_EGRESS_MODPORT(parent_gport) ) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, Level %d egress scheduler must attach to an egress modport gport,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, unit));
			return BCM_E_PARAM;
		    }

		    rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, child_node, 1, egress, &child_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to allocation child level %d egress scheduler node,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, unit));
			return BCM_E_RESOURCE;
		    }

		    rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, child_level, child_node, -1, TRUE, -1);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to attach subport node %d\n"),
			           FUNCTION_NAME(), child_node));
			return rv;
		    }

		    if (BCM_GPORT_IS_SCHEDULER(child_gport) &&
			(cs_l >= 0) &&
			(SOC_SBX_STATE(unit)->egress_scheduler_state != NULL)) {
			SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].node = child_node;
			SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].level = child_level;			
		    }
		    break;
		default:
		    return BCM_E_INTERNAL;
	    }
	} else {
	    /* Will not support bottom down API sequence */
	    return BCM_E_UNAVAIL;
	}
    } else {
	/* ingress scheduler attach */
	child_level = -1;
	parent_level = -1;
	child_node = -1;
	parent_node = -1;
	base_queue = -1;
	cs_l = -1;
	ps_l = -1;

	/* Decode gport and get level/node info */
	if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(child_gport)) {
	    /* attaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(child_gport)) {
	    /* attaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(child_gport)) {
	    /* attaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(child_gport)) {
	    /* attaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_CHILD(child_gport)) {
	    /* attaching a subport, whose level is determined at init time */
	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for subport gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return BCM_E_INTERNAL;
	    }
	    if ((child_level < 0) || (child_level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ||
		(child_node >=  SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level])) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, invalid parent level %d node %d, unit %d\n"),
		           FUNCTION_NAME(), child_level, child_node, unit));
		return BCM_E_PARAM;
	    }
	} else if (BCM_GPORT_IS_SCHEDULER(child_gport)) {
	    /* attaching a ingress scheduler, whose level may or may not known,
	     * if attaching bottom up, then child level is known,
	     * if attaching top down, then child level is unknown
	     */
	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for scheduler gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return rv;
	    }

	    /* if SDK manage resource, get logical scheduler id */
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode != TRUE) {
		cs_l = BCM_INT_SBX_SCHEDULER_ID_GET(child_gport);
		LOG_DEBUG(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "cs_l: 0x%x\n"),
		           cs_l));
	    }
	} else {
	    /* unsupported ingress child gport type */
	    return BCM_E_PARAM;
	}

	if (BCM_GPORT_IS_SCHEDULER(parent_gport)) {
	    /* attaching to ingress scheduler, whose level may or may not known,
	     * if attaching bottom up, then parent level is unknown,
	     * if attaching top down, then parent level is known
	     */
	    rv = bcm_sbx_port_get_scheduler(unit, parent_gport, &parent_level, &parent_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for scheduler gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), parent_gport, unit));
		return rv;
	    }

	    /* if SDK manage resource, get logical scheduler id */
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode != TRUE) {
		ps_l = BCM_INT_SBX_SCHEDULER_ID_GET(parent_gport);
		LOG_DEBUG(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ps_l: 0x%x\n"),
		           ps_l));
	    }
	} else if ( (BCM_GPORT_IS_CHILD(parent_gport)) || (BCM_GPORT_IS_MODPORT(parent_gport)) ) {
	    /* attaching to a subport or interface, whose level is determined at init time */
	    rv = bcm_sbx_port_get_scheduler(unit, parent_gport, &parent_level, &parent_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for subport gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), parent_gport, unit));
		return BCM_E_INTERNAL;
	    }
	} else {
	    /* unsupported ingress parent gport type */
	    return BCM_E_PARAM;
	}

        soc_cm_get_id(unit, &dev_id, &rev_id);
	if ( (child_level == -1) && ((parent_level != -1) && (parent_node != -1)) ) {
	    /* Top-down scheme, parents are known */
	    topDown = TRUE;
            if (dev_id == BCM56931_DEVICE_ID || dev_id == BCM56936_DEVICE_ID) {
                if (parent_level == 4) {
                    child_level = 1;
                } else {
                    child_level = parent_level - 1;
                }
            } else {
	        /* child level unknown, assume it will be 1 level lower than parent */
	        child_level = parent_level - 1;
            }
	} else if ( ((child_level != -1) && (child_node != -1)) && (parent_level == -1) ) {
	    /* bottom up scheme, childs are known */
	    /* parent level unknown, assume it will be 1 level higher than child */
	    parent_level = child_level + 1;
	    topDown = FALSE;
	} else if ( (child_level != -1) && (parent_level != -1) ) {
	    if ((sir->bSubscriberNodeOptimize == TRUE) && 
		(sir->uSubscriberMaxCos == 8) && 
		(sir->b8kNodes == FALSE) && 
		(child_level == 0) && 
		(parent_level == 2)) {
		/* when optimize node usage, max cos level is 8 and TS hardware is 
		 * configured to be 4 leaf nodes for each level 1 node, we hide level 1 nodes
		 * allow user to connect level 0 to level 2 directly
		 */
		hide_level1 = TRUE;
	    } else if ( parent_level != (child_level + 1) ) {
		/* direct connect with levels in between auto created */
		
		return BCM_E_UNAVAIL;
	    } else if ( (child_node != -1) && (is_state[unit][child_level][child_node].in_use == TRUE) &&
			(child_level != 4)) {
		/* allow a special case for level 4 nodes per customer request, user could 
		 * connect an existing level 4 node to a level 5 node even not under 
		 * the user management mode.
		 */
		if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode != TRUE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, gport 0x%x already attached,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_gport, unit));
		    return BCM_E_PARAM;
		}
	    }
	} else {
	    /* child and parent level both unknown, don't support this */
	    return BCM_E_PARAM;
	}

	/* Allocate hardware scheduler node */
	if (topDown) {
	    if ((parent_level < 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ||
		(parent_node >=  SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[parent_level])) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, invalid parent level %d node %d, unit %d\n"),
		           FUNCTION_NAME(), parent_level, parent_node, unit));
		return BCM_E_PARAM;
	    }
	    ps_i_state = &is_state[unit][parent_level][parent_node];
	    if (ps_i_state->in_use != TRUE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Parent node not initialized,  Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_INTERNAL;
	    }

	    if (child_level == 0) {
		
		
		if (hide_level1 == FALSE) {
		    /* attaching queue to level 1 scheduler, level 0 leaf nodes are fixed mapped
		     * to level 1 nodes. mark all corresponding leaf nodes to be used
		     */
		    if (sir->b8kNodes) {
			/* 8 child per level 1 node */
			num_child_level1 = 8;
		    } else {
			/* 4 child per level 1 node */
			num_child_level1 = 4;
		    }

		    if (cosq > num_child_level1) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, cos level out of range [0-%d),  Unit(%d)\n"),
			           FUNCTION_NAME(), num_child_level1, unit));
			return BCM_E_PARAM;
		    } else if (cosq > 0) {
			num_queues = cosq;
		    } else {
			/* provided cos level out of range, use what's supported in hardware natually */
			num_queues = num_child_level1;
		    }

		    base_node = parent_node * num_child_level1;
		    for (node = base_node; node < (base_node + num_child_level1); node++) {
			rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, node, 1, egress, &child_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to allocation child level %d node %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), child_level, node, unit));
			    return BCM_E_RESOURCE;
			}
			
			queue = base_queue + child_node - base_node;
			if (queue >= (base_queue + num_queues)) {
			    /* only attach the number of queues specified in API, even though all leaf nodes will be 
			     * reserved
			     */
			    continue;
			}

			rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, child_level,
							       child_node, queue, egress, node - base_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to connect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), child_level, child_node, parent_level, parent_node, unit));
			    return rv;
			}
		    }
		} else {
		    /* parent node only allow one child be attached */
		    rv = soc_sirius_ts_node_hierachy_config_get(unit, parent_level, parent_node,
								&parent, &first_child, &num_child);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		    if (num_child != 0) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, Parent node already has child attached,  Unit(%d)\n"),
			           FUNCTION_NAME(), unit));
			return BCM_E_PARAM;
		    }

		    /* need to allocate level 1 nodes based on num of cos levels */
		    if (cosq <= 4) {
			num_level1_nodes = 1;
			if (cosq > 0) {
			    num_queues = cosq;
			} else {
			    num_queues = 4;
			}
		    } else if (cosq <= 8) {
			/* alloc 2 level 1 nodes */
			num_level1_nodes = 2;
			num_queues = cosq;
		    } else {
			/* assuming 16 cos levels, allocate 4 level 1 nodes */
			num_level1_nodes = 4;
			num_queues = (cosq > 16)? 16:cosq;
		    }

		    /* alloc level 1 nodes, following function will allocate a chunk of nodes */
		    rv = bcm_sirius_cosq_scheduler_allocate(unit, 1, BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT,
							    num_level1_nodes, egress, &level1_nodes[0]);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not allocate level 1 node,  Unit(%d)\n"),
			           FUNCTION_NAME(), unit));
			return BCM_E_RESOURCE;
		    }
		    level1_nodes[1] = level1_nodes[0] + 1;
		    level1_nodes[2] = level1_nodes[1] + 1;
		    level1_nodes[3] = level1_nodes[2] + 1;

		    for (node = 0; node < num_level1_nodes; node++) {
			rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, 1,
							       level1_nodes[node], -1, egress, -1);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to connect level 1 node %d to parent level %d node %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), level1_nodes[node], parent_level, parent_node, unit));
			    return rv;
			}

			if (num_level1_nodes > 1) {
			    /* config the level 1 nodes as flat scheduler leaf_delay nodes */
			    rv = soc_sirius_ts_node_mapping_config(unit, 1, level1_nodes[node],
								   SIRIUS_TS_NODE_TYPE_LEAF_RELAY,
								   SIRIUS_TS_BUCKET_TYPE_SUBTREE,
								   level1_nodes[0]);
			    if (rv != SOC_E_NONE) {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, fail to config ingress scheduler level 1 node %d bucket info, Unit(%d)\n"),
				           FUNCTION_NAME(), level1_nodes[node], unit));
				return rv;
			    } else {
				is_state[unit][1][level1_nodes[node]].bucket = level1_nodes[0];
			    }

			    rv = soc_sirius_ts_node_creditor_config(unit, 1, level1_nodes[node],
								    SOC_SIRIUS_API_PARAM_NO_CHANGE,
								    SOC_SIRIUS_API_PARAM_NO_CHANGE,
								    SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT);
			    
			    if (rv != SOC_E_NONE) {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, failed to config ingress creditor node profile for level 1 node %d, Unit(%d)\n"),
				           FUNCTION_NAME(), level1_nodes[node], unit));
				return rv;
			    } else {
				LOG_VERBOSE(BSL_LS_BCM_COSQ,
				            (BSL_META_U(unit,
				                        "%s config TS level 1 node %d use node_profile %d, Unit(%d)\n"),
				             FUNCTION_NAME(), level1_nodes[node],
				             SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT, unit));
			    }
			}
		    }

		    /* config the level 2 nodes as flat scheduler root_delay node */
		    rv = soc_sirius_ts_node_mapping_config(unit, parent_level, parent_node,
							   SIRIUS_TS_NODE_TYPE_ROOT_RELAY,
							   SIRIUS_TS_BUCKET_TYPE_BUCKET,
							   SOC_SIRIUS_API_PARAM_NO_CHANGE);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, fail to config ingress scheduler level 2 node %d bucket info, Unit(%d)\n"),
			           FUNCTION_NAME(), parent_node, unit));
			return rv;
		    }	    

		    /* attach level 0 nodes to corresponding level 1 nodes, here hardware is always mapping
		     * 4 leaf nodes to 1 level 1 node
		     */
		    base_node = level1_nodes[0] * 4;
		    for (node = base_node; node < (base_node + num_level1_nodes*4); node++) {
			rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, node, 1, egress, &child_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to allocation child level %d node %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), child_level, node, unit));
			    return BCM_E_RESOURCE;
			}
			
			queue = base_queue + child_node - base_node;
			if (queue >= (base_queue + num_queues)) {
			    /* only attach the number of queues specified in API, even though all leaf nodes will be 
			     * reserved
			     */
			    continue;
			}

			rv = bcm_sirius_cosq_scheduler_connect(unit, 1, level1_nodes[(node-base_node)/4],
							       child_level, child_node, queue, egress, node - base_node);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to connect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
			               FUNCTION_NAME(), child_level, child_node, parent_level, parent_node, unit));
			    return rv;
			}
		    }
		}
	    } else {
		/* intermediate levels, allocate child hardware nodes */
		if (parent_level == 2) {
		    rv = bcm_sbx_cosq_gport_attach_get(unit, parent_gport, &first_child_gport, &num_child);
		    if ((rv == BCM_E_NONE) && 
			(BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(first_child_gport) ||
			 BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(first_child_gport))) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, Parent node already has queue group attached,  Unit(%d)\n"),
			           FUNCTION_NAME(), unit));
			return BCM_E_PARAM;
		    }
		}

		/* user management mode */
		if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
		    rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, child_level,
							   child_node, -1, egress, -1);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to connect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, child_node, parent_level, parent_node, unit));
		    }
		    return rv;
		}

		if ((child_level == 4) && (child_node != -1)) {
		    /* allow user to attach existing level 4 node to level 5 node
		     * skip node allocation.
		     */

		    /* 
		     * Steps in case of traffic queued up in the existing level 4 nodes:
		     *    (1) before we attach the level 4 nodes, make sure the level 6
		     *       node the level 4 node eventually will root on will point to 
		     *       a node profile that always report priority of 0. This 
		     *       should stop the granting while we change the tree
		     *       structure underline
		     *       L6_nm.node_profile_ptr = 31 (init profile 31 to always report 0)
		     *       disable queue level shaper, write l4_bk to 0 to make sure 
		     *       no shaper event will be generated
		     *       disable egress to ingress flow control to prevent full_state event
		     *    (2) change the tree hierachy
		     *    (3) propagate the current_pri of the level 4 node (l4_current_pri), 
		     *        up to root level, set child_pri/current_pri/current_child 
		     *    (4) reenable everything
                     * 
		     */
		    l4_node = child_node;
		    l5_node = parent_node;
		    l6_node = is_state[unit][5][l5_node].parent;
		    if (l6_node < 0) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not find parent of level 5 node %d,  Unit(%d)\n"), 
			           FUNCTION_NAME(), l5_node, unit));
			return SOC_E_INTERNAL;
		    }
		    l7_node = is_state[unit][6][l6_node].parent;
		    if (l7_node < 0) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not find parent of level 6 node %d,  Unit(%d)\n"), 
			           FUNCTION_NAME(), l6_node, unit));
			return SOC_E_INTERNAL;
		    }

		    if (is_state[unit][5][l5_node].num_child == 0) {
		      l4_offset = 0;
		    } else {
		      l4_offset = l4_node - is_state[unit][5][l5_node].first_child;
		      if ((l4_offset < 0) || (l4_offset >= 8)) {
		    	LOG_ERROR(BSL_LS_BCM_COSQ,
		    	          (BSL_META_U(unit,
		    	                      "ERROR: %s, TS level 4 node %d is not child of level 5 node %d, Unit(%d)\n"),
		    	           FUNCTION_NAME(), l4_node, l5_node, unit));
			return SOC_E_INTERNAL;
		      }
		    }			

		    l5_offset = l5_node - is_state[unit][6][l6_node].first_child;
		    if ((l5_offset < 0) || (l5_offset >= 8)) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, TS level 5 node %d is not child of level 6 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l5_node, l6_node, unit));
			return SOC_E_INTERNAL;
		    }

		    l6_offset = l6_node - is_state[unit][7][l7_node].first_child;
		    if ((l6_offset < 0) || (l6_offset >= 8)) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, TS level 6 node %d is not child of level 7 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l6_node, l7_node, unit));
			return SOC_E_INTERNAL;
		    }

		    /* STEP1: point l6_node to a node profile that always report priority 0 */
		    SOC_IF_ERROR_RETURN(READ_L6_NMm(unit, MEM_BLOCK_ANY, l6_node, &l6_nm_entry));
		    l6_old_profile = soc_mem_field32_get(unit, L6_NMm, &l6_nm_entry, NODE_PROFILE_PTRf);

		    rv = soc_sirius_ts_node_creditor_config(unit, 6, l6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SIRIUS_TS_NODE_PROFILE_TEMPLATE_MAX);
		    
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config ingress creditor node profile for level 6 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l6_node, unit));
			return rv;		    
		    }

		    /* disable all queue level shapers */
		    SOC_IF_ERROR_RETURN(READ_SHAPER_LOOP_SIZEr(unit, &old_shaper_loop));
		    SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &old_shaper_start));
		    SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, &old_shaper_end));

		    SOC_IF_ERROR_RETURN(WRITE_SHAPER_LOOP_SIZEr(unit, 0));
		    SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, 0));
		    SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, 0));

		    /* clear the l4_node shaper bucket, double clear to make sure hardware periodic event not overwriting */
		    sal_memset(&l4_bk_entry, 0, sizeof(l4_bk_entry_t));
		    SOC_IF_ERROR_RETURN(WRITE_L4_BKm(unit, MEM_BLOCK_ANY, l4_node, &l4_bk_entry));
		    sal_usleep(200);
		    SOC_IF_ERROR_RETURN(WRITE_L4_BKm(unit, MEM_BLOCK_ANY, l4_node, &l4_bk_entry));

		    /* disable backpressure */
		    SOC_IF_ERROR_RETURN(READ_FR_CONFIG0r(unit, &uRegValue));
		    soc_reg_field_set(unit, FR_CONFIG0r, &uRegValue, FORCE_FULL_STATUS_DEBUGf, 1);
		    SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG0r(unit, uRegValue));
		    sal_usleep(10);
		} else if (ps_i_state->num_child == 0) {
		    if (child_level == 6) {
                /* fix mapping between level 7 and level 6 . 
                 * Level6 node 48 is reserved. For CPU (PCI IF), nodes 49-55 are used
                 */
                child_node = (parent_node == 0) ? 49 : ((parent_node - 1) * 8);
		    } else {
			/* first child, allocate anywhere */
			child_node = BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT;

			/* reserve a chunk of nodes if parent wants to pre-allocate */
			num_nodes = SOC_SBX_STATE(unit)->ingress_scheduler_state[ps_l].num_childs;
			if ( (ps_l >= 0) && (num_nodes > 0) ) {
			    rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, child_node,
								    num_nodes, egress, &child_node);
			    if (rv != BCM_E_NONE) {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, can not reserve %d child nodes,  Unit(%d)\n"), 
				           FUNCTION_NAME(), num_nodes, unit));
				return BCM_E_RESOURCE;
			    }

			    /* mark all the allocated nodes as reserved */
			    for (node = child_node; node < (child_node + num_nodes); node++) {
				is_state[unit][child_level][node].in_use = SIRIUS_INGRESS_SCHEDULER_RESERVED;
			    }
			    is_state[unit][child_level][child_node].in_use = FALSE;
			}
		    }
		    rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, child_node, 1, egress, &child_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not allocate child node,  Unit(%d)\n"),
			           FUNCTION_NAME(), unit));
			return BCM_E_RESOURCE;
		    }
		} else {
		    /* rules for the child of a particular node
		     *   all children has to be consective
		     *   support upto 8 children
		     */
		    if (ps_i_state->num_child > 8) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not attach more than 8 children to gport 0x%x,  Unit(%d)\n"),
			           FUNCTION_NAME(), parent_gport, unit));
			return BCM_E_RESOURCE;
		    }

		    child_node = ps_i_state->first_child + ps_i_state->num_child;
		    if (is_state[unit][child_level][child_node].in_use == SIRIUS_INGRESS_SCHEDULER_RESERVED) {
			/* if the next node is reserved, just take it */
			is_state[unit][child_level][child_node].in_use = TRUE;
		    } else {
			/* otherwise try to take it if it's free, or shift around to make room */
			rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, ps_i_state->first_child + ps_i_state->num_child,
								1, egress, &child_node);
			if (rv != BCM_E_NONE) {
			    /* search higher nodes, if there is a free node, shift existing nodes in the same level to
			     * make room for a new child node
			     */
			    rv = bcm_sirius_cosq_ingress_scheduler_shift(unit, child_level, ps_i_state->first_child, TRUE);
			    if (rv == BCM_E_RESOURCE) {
				/* search in lower nodes so that the new child could fit in */
				rv = bcm_sirius_cosq_ingress_scheduler_shift(unit, child_level, ps_i_state->first_child, FALSE);
				if (rv != BCM_E_NONE) {
				    LOG_ERROR(BSL_LS_BCM_COSQ,
				              (BSL_META_U(unit,
				                          "ERROR: %s, failed to allocate node at level %d,  Unit(%d)\n"),
				               FUNCTION_NAME(), child_level, unit));
				    return rv;
				}
				
				/* now the node right below the first child should be available */
				rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, 
									ps_i_state->first_child - 1,
									1, egress, &child_node);
				if (rv != BCM_E_NONE) {
				    /* this should never happen given the shift returned successfully */
				    LOG_ERROR(BSL_LS_BCM_COSQ,
				              (BSL_META_U(unit,
				                          "ERROR: %s, level %d node %d already used,  Unit(%d)\n"),
				               FUNCTION_NAME(), child_level, ps_i_state->first_child - 1, unit));
				    return BCM_E_INTERNAL;
				}
				
				/* update the first_child of parent, the bcm_sirius_cosq_scheduler_connect will update
				 * the num_child automatically
				 */
				ps_i_state->first_child--;
			    } else if (rv == BCM_E_NONE) {
				/* now the node right above the last child should be available */
				rv = bcm_sirius_cosq_scheduler_allocate(unit, child_level, 
									ps_i_state->first_child + ps_i_state->num_child,
									1, egress, &child_node);
				if (rv != BCM_E_NONE) {
				    /* this should never happen given the shift returned successfully */
				    LOG_ERROR(BSL_LS_BCM_COSQ,
				              (BSL_META_U(unit,
				                          "ERROR: %s, level %d node %d already used,  Unit(%d)\n"),
				               FUNCTION_NAME(), child_level, ps_i_state->first_child + ps_i_state->num_child, unit));
				    return BCM_E_INTERNAL;
				}
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, failed to allocate node at level %d,  Unit(%d)\n"),
				           FUNCTION_NAME(), child_level, unit));
				return rv;
			    }
			}
		    }
		}

		rv = bcm_sirius_cosq_scheduler_connect(unit, parent_level, parent_node, child_level,
						       child_node, -1, egress, -1);
		if (rv != BCM_E_NONE) {
		  LOG_ERROR(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "ERROR: %s, failed to connect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
		             FUNCTION_NAME(), child_level, child_node, parent_level, parent_node, unit));
		  return rv;
		}

		if (l6_old_profile >= 0) {
		    /* STEP2: mark priorities up the tree */
#ifdef NOTDEF_NEW_INTF
		    /*== read L4 current pri */
		    rv = soc_sirius_ts_node_get_pri(unit, 4, l4_node, (uint32 *)&l4_pri);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to get priority for level 4 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l4_node, unit));
			return rv;
		    }

		    /*== read and update L5 priorities */
		    rv = soc_sirius_ts_node_update_child_pri(unit, 5, l5_node, l4_node, l4_pri, (uint32 *)&l5_pri);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to update child %d priority for level 5 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l4_node, l5_node, unit));
			return rv;
		    }

		    /*== read and update L6 priorities if need */
		    rv = soc_sirius_ts_node_update_child_pri(unit, 6, l6_node, l5_node, l5_pri, (uint32 *)&l6_pri);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to update child %d priority for level 6 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l5_node, l6_node, unit));
			return rv;
		    }

		    /*== reenable the level 6 node to report priority */
		    rv = soc_sirius_ts_node_creditor_config(unit, 6, l6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    l6_old_profile);
		    
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config ingress creditor node profile for level 6 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l6_node, unit));
			return rv;		    
		    }

		    /*== read and update L7 priorities if need */
		    rv = soc_sirius_ts_node_update_child_pri(unit, 7, l7_node, l6_node, l6_pri, (uint32 *)&l7_pri);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to update child %d priority for level 7 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l6_node, l7_node, unit));
			return rv;
		    }

		    /*== read and update Root priorities if need */
		    rv = soc_sirius_ts_node_update_child_pri(unit, 8, l7_node, l7_node, l7_pri, NULL);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to regenerate priority for level 7 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l7_node, unit));
			return rv;
		    }
#else
		    /*== read L4 current pri */
		    SOC_IF_ERROR_RETURN(READ_L4_N0m(unit, MEM_BLOCK_ANY, l4_node, &l4_n0_entry));
		    l4_pri = soc_mem_field32_get(unit, L4_N0m, &l4_n0_entry, CURRENT_PRIf);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 4 node %d current pri is %d\n"),
		                 FUNCTION_NAME(), l4_node, l4_pri));

		    /*== read and update L5 priorities */
		    SOC_IF_ERROR_RETURN(READ_L5_N0m(unit, MEM_BLOCK_ANY, l5_node, &l5_n0_entry));

		    /* update child_pri in L5_n0 for the newly attached L4 node */
		    soc_mem_field32_set(unit, L5_N0m, &l5_n0_entry, child_pri_fields[l4_offset], l4_pri);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 5 node %d child %d pri is %d\n"),
		                 FUNCTION_NAME(), l5_node,
		                 l4_offset, l4_pri));

		    current_pri = 0;
		    current_child = 0;
		    for (tmp_node = 0;
		    	 tmp_node < is_state[unit][5][l5_node].num_child;
		     	 tmp_node++) {
		    	child_pri = soc_mem_field32_get(unit, L5_N0m, &l5_n0_entry, child_pri_fields[tmp_node]);
		    	if (child_pri > current_pri) {
			    current_pri = child_pri;
			    current_child = tmp_node;
		    	}
	            }
		    soc_mem_field32_set(unit, L5_N0m, &l5_n0_entry, CURRENT_PRIf, current_pri);
		    soc_mem_field32_set(unit, L5_N0m, &l5_n0_entry, CURRENT_CHILDf, current_child);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 5 node %d current_pri is %d current_child is %d\n"),
		                 FUNCTION_NAME(), l5_node,
		                 current_pri, current_child));
		    SOC_IF_ERROR_RETURN(WRITE_L5_N0m(unit, MEM_BLOCK_ANY, l5_node, &l5_n0_entry));
		    l5_pri = current_pri;

		    /*== read and update L6 priorities if need */
		    SOC_IF_ERROR_RETURN(READ_L6_N0m(unit, MEM_BLOCK_ANY, l6_node, &l6_n0_entry));
		    soc_mem_field32_set(unit, L6_N0m, &l6_n0_entry, child_pri_fields[l5_offset], l5_pri);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 6 node %d child %d pri is %d\n"),
		                 FUNCTION_NAME(), l6_node,
		                 l5_offset, l5_pri));
		    current_pri = 0;
		    current_child = 0;
		    for (tmp_node = 0;
		    	 tmp_node < is_state[unit][6][l6_node].num_child;
		     	 tmp_node++) {
		    	child_pri = soc_mem_field32_get(unit, L6_N0m, &l6_n0_entry, child_pri_fields[tmp_node]);
		    	if (child_pri > current_pri) {
			    current_pri = child_pri;
			    current_child = tmp_node;
		    	}
	            }
		    soc_mem_field32_set(unit, L6_N0m, &l6_n0_entry, CURRENT_PRIf, current_pri);
		    soc_mem_field32_set(unit, L6_N0m, &l6_n0_entry, CURRENT_CHILDf, current_child);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 6 node %d current_pri is %d current_child is %d\n"),
		                 FUNCTION_NAME(), l6_node,
		                 current_pri, current_child));
		    SOC_IF_ERROR_RETURN(WRITE_L6_N0m(unit, MEM_BLOCK_ANY, l6_node, &l6_n0_entry));
		    l6_pri = current_pri;

		    /*== reenable the level 6 node to report priority */
		    rv = soc_sirius_ts_node_creditor_config(unit, 6, l6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    SOC_SIRIUS_API_PARAM_NO_CHANGE,
							    l6_old_profile);
		    
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config ingress creditor node profile for level 6 node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), l6_node, unit));
			return rv;		    
		    }

		    /*== read and update L7 priorities if need */
		    SOC_IF_ERROR_RETURN(READ_L7_N0m(unit, MEM_BLOCK_ANY, l7_node, &l7_n0_entry));
		    soc_mem_field32_set(unit, L7_N0m, &l7_n0_entry, child_pri_fields[l6_offset], l6_pri);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 7 node %d child %d pri is %d\n"),
		                 FUNCTION_NAME(), l7_node,
		                 l6_offset, l6_pri));
		    current_pri = 0;
		    current_child = 0;
		    for (tmp_node = 0;
		    	 tmp_node < is_state[unit][7][l7_node].num_child;
		     	 tmp_node++) {
		    	child_pri = soc_mem_field32_get(unit, L7_N0m, &l7_n0_entry, child_pri_fields[tmp_node]);
		    	if (child_pri > current_pri) {
			    current_pri = child_pri;
			    current_child = tmp_node;
		    	}
	            }
		    soc_mem_field32_set(unit, L7_N0m, &l7_n0_entry, CURRENT_PRIf, current_pri);
		    soc_mem_field32_set(unit, L7_N0m, &l7_n0_entry, CURRENT_CHILDf, current_child);
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s, level 7 node %d current_pri is %d current_child is %d\n"),
		                 FUNCTION_NAME(), l7_node,
		                 current_pri, current_child));
		    SOC_IF_ERROR_RETURN(WRITE_L7_N0m(unit, MEM_BLOCK_ANY, l7_node, &l7_n0_entry));
		    l7_pri = current_pri;

		    /*== read and update RT_ST if need */
		    SOC_IF_ERROR_RETURN(READ_RT_STm(unit, MEM_BLOCK_ANY, l7_node, &rt_st_entry));
		    rt_pri = soc_mem_field32_get(unit, RT_STm, &rt_st_entry, CHILD_PRIf);
		    if (l7_pri > rt_pri) {
			soc_mem_field32_set(unit, RT_STm, &rt_st_entry, CHILD_PRIf, l7_pri);
		        LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                    (BSL_META_U(unit,
		                                "%s, rt_st node %d pri is %d\n"),
		                     FUNCTION_NAME(), l7_node,
		                     l7_pri));
			SOC_IF_ERROR_RETURN(WRITE_RT_STm(unit, MEM_BLOCK_ANY, l7_node, &rt_st_entry));
		    }
#endif /* NOTDEF_NEW_INTF */

		    /* reenable backpressure */
		    SOC_IF_ERROR_RETURN(READ_FR_CONFIG0r(unit, &uRegValue));
		    soc_reg_field_set(unit, FR_CONFIG0r, &uRegValue, FORCE_FULL_STATUS_DEBUGf, 0);
		    SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG0r(unit, uRegValue));

		    /* reenable queue level shapers */
		    SOC_IF_ERROR_RETURN(WRITE_SHAPER_LOOP_SIZEr(unit, old_shaper_loop));
		    SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, old_shaper_start));
		    SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, old_shaper_end));
		}
		
		if (cs_l >= 0) {
		    /* fill in mapping between logical scheduler and physical schedulers */
		    SOC_SBX_STATE(unit)->ingress_scheduler_state[cs_l].level = child_level;
		    SOC_SBX_STATE(unit)->ingress_scheduler_state[cs_l].node = child_node;
		    is_state[unit][child_level][child_node].logical_scheduler = cs_l;
		}

		if ((BCM_GPORT_IS_CHILD(parent_gport)) || (BCM_GPORT_IS_MODPORT(parent_gport))) {
		    /* save this child gport that schedulers will be attached to */
		    is_state[unit][child_level][child_node].parent_gport = parent_gport;
	        } 
		
		LOG_DEBUG(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "Update is_map, cs_l: 0x%x, level: 0x%x,"
		                       " node: 0x%x\n"), cs_l, child_level, child_node));
	    }
	} else {
	    
	    return BCM_E_UNAVAIL;
	}
    }

    return rv;
}

int
bcm_sirius_cosq_scheduler_attach_get(int unit, bcm_gport_t in_gport, int egress,
				     bcm_cos_queue_t *cosq, bcm_gport_t *out_gport)
{
    int rv = BCM_E_NONE;
    int base_queue = 0;
    int mc, level, node, leaf_node, level1_node, parent, child, first_child, num_child, parent_level, child_level;
    int logical_scheduler;
    int mymodid, port = BCM_GPORT_INVALID;
    int max_node, index, egress_group;
    bcm_pbmp_t pbmp;
    bcm_sbx_subport_info_t *sp_info;
    bcm_sbx_sirius_egress_scheduler_state_t *e_state;
    int cos = *cosq;
    uint16 dev_id;
    uint8 rev_id;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));

    if (egress) {
	rv = bcm_sbx_port_get_scheduler(unit, in_gport, &level, &node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, fail to map gport 0x%x to its hardware resources, Unit(%d)\n"),
	               FUNCTION_NAME(), in_gport, unit));
	    return rv;
	}

	if ((level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
	    (node >=  SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level])) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid parent level %d node %d, unit %d\n"),
	               FUNCTION_NAME(), level, node, unit));
	    return BCM_E_PARAM;
	}
	
	/* egress schedulers */
	if (BCM_GPORT_IS_EGRESS_CHILD(in_gport)) {
	    if (cos < 0) {
		/* return parent, egress MODPORT gport */
		rv = BCM_E_NONE;
		parent = es_state[unit][level][node].intf;
		if (parent == SB_FAB_DEVICE_SIRIUS_CPU_INTF) {
		    BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
		    PBMP_ITER(pbmp, port) {
			if (IS_CPU_PORT(unit, port)) {
			    break;
			}
		    }
		} else if ((parent == SB_FAB_DEVICE_SIRIUS_RQ0_INTF) ||
			   (parent == SB_FAB_DEVICE_SIRIUS_RQ1_INTF)) {
		    port = SOC_PORT_MIN(unit, req) + parent - 
			SB_FAB_DEVICE_SIRIUS_RQ0_INTF;
		} else if ((parent >= SB_FAB_DEVICE_SIRIUS_HG0_INTF) &&
			   (parent <= SB_FAB_DEVICE_SIRIUS_HG3_INTF)) {
		    port = SOC_PORT_MIN(unit, hg) + parent -
			SB_FAB_DEVICE_SIRIUS_HG0_INTF;
		} else {
		    rv = BCM_E_NOT_FOUND;
		    return rv;
		}
		
		BCM_GPORT_EGRESS_MODPORT_SET((*out_gport), mymodid, port);
	    } else {
		/* return child, scheduler gport type, use cosq as map_index */
		level = 1;
		max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
		rv = BCM_E_NOT_FOUND;
		*out_gport = BCM_GPORT_INVALID;
		for (child = 0; child < max_node; child++) {
		    e_state = &(es_state[unit][level][child]);
		    if (e_state->in_use != TRUE) {
			continue;
		    }
		    
		    if ( (e_state->channel == node) && (e_state->index == cos) ) {
			rv = _bcm_sirius_cosq_p2l_scheduler(unit, TRUE, level, child, &logical_scheduler);
			if (rv == BCM_E_NONE) {
			    BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), TRUE, logical_scheduler);
			}
			break;
		    }
		}
	    }
	} else if (BCM_GPORT_IS_EGRESS_MODPORT(in_gport)) {
	    /* could only return child */
	    if (cos < 0) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, ingress level %d node %d has no parent, unit %d\n"),
		           FUNCTION_NAME(), level, node, unit));
		return BCM_E_PARAM;
	    }

	    /* return child */
	    level = 2;
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
	    rv = BCM_E_NOT_FOUND;
	    *out_gport = BCM_GPORT_INVALID;
	    for (child = 0; child < max_node; child++) {
		if (es_state[unit][level][child].in_use != TRUE) {
		    continue;
		}
		
		if (soc_sirius_es_node_get_info(unit, level, child, &parent, &index) != SOC_E_NONE) {
		    continue;
		}

		/* return the level 2 node with matching map_index and parent node */
		if ( (parent == node) && (cos == index) ) {
		    if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP) == FALSE) {
			/* return child, which should be EGRESS Scheduler gport */
			rv = _bcm_sirius_cosq_p2l_scheduler(unit, TRUE, level, child, &logical_scheduler);
			if (rv == BCM_E_NONE) {
			    BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), TRUE, logical_scheduler);
			}
			break;
		    } else {
			/* return child, which should be EGRESS CHILD gport */
			rv = BCM_E_NOT_FOUND;
			*out_gport = BCM_GPORT_INVALID;

			/* search subport_info to find matching child port */
			if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
			    for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
				sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
				if (sp_info->valid == FALSE) {
				    continue;
				}
				if (sp_info->es_scheduler_level2_node == child) {
				    rv = BCM_E_NONE;
				    BCM_GPORT_EGRESS_CHILD_SET((*out_gport), mymodid, port);
				    break;
				}
			    }
			}
			break;
		    }
		}
	    }
	} else if (BCM_GPORT_IS_EGRESS_GROUP(in_gport)) {
	    if (cos < 0) {
		/* could only return parent as scheduler gport */
		egress_group = BCM_GPORT_EGRESS_GROUP_GET(in_gport);
		if ((egress_group < 0) || (egress_group >= SOC_SBX_CFG(unit)->num_egress_group) ||
		    (SOC_SBX_STATE(unit)->egress_group_state[egress_group].eg_in_use != TRUE)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress group 0x%x not created,  Unit(%d)\n"),
		               FUNCTION_NAME(), in_gport, unit));
		    return BCM_E_PARAM;
		}
		
		node = SOC_SBX_STATE(unit)->egress_group_state[egress_group].eg_scheduler;
		if (node >= 0) {
		    rv = BCM_E_NONE;
		    BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), TRUE, node);
		} else {
		    rv = BCM_E_NOT_FOUND;
		    *out_gport = BCM_GPORT_INVALID;
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s gport 0x%x is at lowest level and has no children.\n"),
		           FUNCTION_NAME(), in_gport));
		rv = BCM_E_NOT_FOUND;
		*out_gport = BCM_GPORT_INVALID;
	    }
	} else if (BCM_GPORT_IS_SCHEDULER(in_gport)) {
	    /* could return parent or child */
	    if (level == SIRIUS_ES_LEVEL_CHANNEL) {
		if (cos < 0) {
		    /* return parent, egress MODPORT gport */
		    rv = BCM_E_NONE;
		    parent = es_state[unit][level][node].intf;
		    if (parent == SB_FAB_DEVICE_SIRIUS_CPU_INTF) {
			BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
			PBMP_ITER(pbmp, port) {
			    if (IS_CPU_PORT(unit, port)) {
				break;
			    }
			}
		    } else if ((parent == SB_FAB_DEVICE_SIRIUS_RQ0_INTF) ||
			       (parent == SB_FAB_DEVICE_SIRIUS_RQ1_INTF)) {
			port = SOC_PORT_MIN(unit, req) + parent - 
			    SB_FAB_DEVICE_SIRIUS_RQ0_INTF;
		    } else if ((parent >= SB_FAB_DEVICE_SIRIUS_HG0_INTF) &&
			       (parent <= SB_FAB_DEVICE_SIRIUS_HG3_INTF)) {
			port = SOC_PORT_MIN(unit, hg) + parent -
			    SB_FAB_DEVICE_SIRIUS_HG0_INTF;
		    } else {
			rv = BCM_E_NOT_FOUND;
			return rv;
		    }
		    
		    BCM_GPORT_EGRESS_MODPORT_SET((*out_gport), mymodid, port);
		} else {
		    /* child is a scheduler gport */
		    level = SIRIUS_ES_LEVEL_SUBPORT;
		    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
		    rv = BCM_E_NOT_FOUND;
		    *out_gport = BCM_GPORT_INVALID;
		    for (child = 0; child < max_node; child++) {
			e_state = &(es_state[unit][level][child]);
			if ((e_state->in_use != TRUE) || (e_state->index != cos)) {
			    continue;
			}
			
			if (soc_sirius_es_node_get_info(unit, level, child, &parent, &index) != SOC_E_NONE) {
			    continue;
			}
			
			if ((parent == node) && (index == cos)) {
			    rv = _bcm_sirius_cosq_p2l_scheduler(unit, TRUE, level, child, &logical_scheduler);
			    if (rv == BCM_E_NONE) {
				BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), TRUE, logical_scheduler);
			    }
			    break;
			}
		    }
		}
	    } else {
		if (cos < 0) {
		    /* return parent */
		    parent = es_state[unit][level][node].channel;
		    rv = BCM_E_NOT_FOUND;
		    *out_gport = BCM_GPORT_INVALID;
		    if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP) == FALSE) {
			/* parent is a scheduler gport */
			if (parent >= 0) {
			    rv = _bcm_sirius_cosq_p2l_scheduler(unit, TRUE, level+1, parent, &logical_scheduler);
			    if (rv == BCM_E_NONE) {
				BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), TRUE, logical_scheduler);
			    }
			}
		    } else {
			if (parent >= 0) {
			    /* parent is an egress CHILD gport */
			    if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
				for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
				    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
				    if (sp_info->valid == FALSE) {
					continue;
				    }
				    if (sp_info->es_scheduler_level2_node == parent) {
					rv = BCM_E_NONE;
					BCM_GPORT_EGRESS_CHILD_SET((*out_gport), mymodid, port);
					break;
				    }
				}
			    }
			}
		    }
		} else {
		    /* child is an EGRESS group gport */
		    logical_scheduler = BCM_INT_SBX_SCHEDULER_ID_GET(in_gport);
		    
		    rv = BCM_E_NOT_FOUND;
		    *out_gport = BCM_GPORT_INVALID;
		    /* search egress group database to find the egress group with matching parent */
		    for (egress_group = 0; egress_group < SOC_SBX_CFG(unit)->num_egress_group; egress_group++) {
			if (SOC_SBX_STATE(unit)->egress_group_state[egress_group].eg_in_use != TRUE) {
			    continue;
			}
			
			if (logical_scheduler == SOC_SBX_STATE(unit)->egress_group_state[egress_group].eg_scheduler) {
			    if ( cos == 0 ) {
				rv = BCM_E_NONE;
				BCM_GPORT_EGRESS_GROUP_SET((*out_gport), mymodid, egress_group);
				break;
			    } else {
				cos--;
			    }
			}
		    }
		}
	    }
	}
    } else {
	/* ingress schedulers */
	if ( (BCM_GPORT_IS_UCAST_QUEUE_GROUP(in_gport)) ||
	     (BCM_GPORT_IS_MCAST_QUEUE_GROUP(in_gport)) ||
	     (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(in_gport)) ||
	     (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(in_gport)) ) {

	    /* queues level, ignore the cosq since we could only find parent */
	    if (cos >= 0) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s gport 0x%x has no children\n"),
		           FUNCTION_NAME(), in_gport));
		return BCM_E_PARAM;
	    }

	    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(in_gport)) {
		base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(in_gport);
	    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(in_gport)) {
		base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(in_gport);
	    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(in_gport)) {
		base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(in_gport);
	    } else {
		base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(in_gport);
	    }

	    /* get the leaf node */
	    rv = soc_sirius_qs_queue_to_leaf_node_get(unit, base_queue, &mc, &leaf_node);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to get leaf node for subscriber queue %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), base_queue, unit));
		return rv;
	    }

	    /* find the level 1 node */
	    if (SOC_SBX_CFG_SIRIUS(unit)->b8kNodes == TRUE) {
		level1_node = leaf_node / 8;
	    } else {
		level1_node = leaf_node / 4;
	    }

	    /* return parent scheduler gport */
	    logical_scheduler = is_state[unit][1][level1_node].logical_scheduler;

	    if (logical_scheduler < 0) {
		/* leve 1 node is hidden, find its parent */
		level = 1;
		rv = soc_sirius_ts_node_hierachy_config_get(unit, level, level1_node, &parent,
							    &first_child, &num_child);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
		               FUNCTION_NAME(), level, level1_node, unit));	    
		    return rv;
		}
		
		if ((is_state[unit][2][parent].in_use == FALSE) ||
		    (is_state[unit][2][parent].logical_scheduler < 0)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, ingress scheduler hierachy state inconsistent, Unit(%d)\n"),
		               FUNCTION_NAME(), unit));
		    return BCM_E_INTERNAL;
		} else {
		    logical_scheduler = is_state[unit][2][parent].logical_scheduler;
		    *cosq = is_state[unit][2][parent].num_child * 
			(SOC_SBX_CFG_SIRIUS(unit)->b8kNodes?8:4);
		}
	    } else {
		*cosq = SOC_SBX_CFG_SIRIUS(unit)->b8kNodes?8:4;
	    }
	    BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), FALSE, logical_scheduler);

	} else if ((BCM_GPORT_IS_CHILD(in_gport)) ||
		   (BCM_GPORT_IS_MODPORT(in_gport)) ||
		   (BCM_GPORT_IS_SCHEDULER(in_gport))) {
	    /* find the scheduler */
	    rv = bcm_sbx_port_get_scheduler(unit, in_gport, &level, &node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fail to map gport 0x%x to its hardware resources, Unit(%d)\n"),
		           FUNCTION_NAME(), in_gport, unit));
		return rv;
	    }
	    
	    if ((BCM_GPORT_IS_MODPORT(in_gport)) && (cos < 0)) {
		/* no parent for modport interface level scheduler, return first child */
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s gport 0x%x has no parent\n"),
		           FUNCTION_NAME(), in_gport));
		return BCM_E_PARAM;
	    }

	    /* get hierachy info */
	    rv = soc_sirius_ts_node_hierachy_config_get(unit, level, node, &parent,
							&first_child, &num_child);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, unit));	    
		return rv;
	    }

	    soc_cm_get_id(unit, &dev_id, &rev_id);

	    if (cos < 0) {
		/* return parent */

		if ((dev_id == BCM56931_DEVICE_ID || dev_id == BCM56936_DEVICE_ID) &&
		    (level == 1)) {
		    parent_level = 4;
		} else {
		    parent_level = level+1;
		}

		logical_scheduler = is_state[unit][level+1][parent].logical_scheduler;
		/* return number of children of imediate parent */
		*cosq = is_state[unit][parent_level][parent].num_child;

		if (logical_scheduler < 0) {
		    /* parent is not a scheduler gport */
		    if ((level == 6) || BCM_GPORT_IS_CHILD(in_gport)) {
			/* return modport */
			if (BCM_GPORT_IS_CHILD(in_gport)) {
			    /* search parent till reach level 6 */
			    while (level < 6) {
				rv = soc_sirius_ts_node_hierachy_config_get(unit, ++level, parent, &parent,
									    &first_child, &num_child);
				if (rv != SOC_E_NONE) {
				    LOG_ERROR(BSL_LS_BCM_COSQ,
				              (BSL_META_U(unit,
				                          "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
				               FUNCTION_NAME(), level, parent, unit));	    
				    return rv;
				}
			    }
			}

			BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));

			if (parent == SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_CPU_NODE) {
			    PBMP_ITER(pbmp, port) {
				if (IS_CPU_PORT(unit, port)) {
				    break;
				}
			    }
			} else if ((parent == SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ0_NODE) ||
				   (parent == SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ1_NODE)) {
			    port = SOC_PORT_MIN(unit, req) + parent - 
				SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_RQ0_NODE;
			} else {
			    port = SOC_PORT_MIN(unit, hg) + parent -
				SB_FAB_DEVICE_SIRIUS_INGRESS_SCHEDULER_HG0_NODE;
			}

			BCM_GPORT_MODPORT_SET((*out_gport), mymodid, port);
		    } else {
			/* return child gport */
			rv = BCM_E_NOT_FOUND;
			if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
			    for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
				sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
				if (sp_info->valid == FALSE) {
				    continue;
				}
				if ((sp_info->ts_scheduler_level == (parent_level)) &&
				    (sp_info->ts_scheduler_node == parent)) {
				    rv = BCM_E_NONE;
				    break;
				}
			    }
			}

			if (rv == BCM_E_NONE) {
			    BCM_GPORT_CHILD_SET((*out_gport), mymodid, port);
			} else {
			    *out_gport = BCM_GPORT_INVALID;
			}
		    }
		} else {
		    BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), FALSE, logical_scheduler);
		}
	    } else {
		/* return child */
		if (num_child <= 0) {
		    return BCM_E_PARAM;
		}

		if ((dev_id == BCM56931_DEVICE_ID || dev_id == BCM56936_DEVICE_ID) &&
		    (level == 4)) {
		    child_level = 1;
		} else {
		    child_level = level-1;
		}

		if ((level > 2) || 
		    ((level == 2) && (is_state[unit][1][first_child].logical_scheduler >= 0))) {
		    /* return scheduler gport */
		    if (cos >= num_child) {
			rv = BCM_E_NOT_FOUND;
			*out_gport = BCM_GPORT_INVALID;
		    } else {
			logical_scheduler = is_state[unit][child_level][first_child+cos].logical_scheduler;
			if (logical_scheduler >= 0) {
			    BCM_INT_SBX_SCHEDULER_GPORT_SET((*out_gport), FALSE, logical_scheduler);
			} else {
			    rv = BCM_E_INTERNAL;
			    *out_gport = BCM_GPORT_INVALID;
			}
		    }
		} else {
		    /* return queue group */
		    if (level == 2) {
			if (SOC_SBX_CFG_SIRIUS(unit)->b8kNodes == TRUE) {
			    leaf_node = first_child * 8;
			} else {
			    leaf_node = first_child * 4;
			}
		    } else {
			leaf_node = first_child;
		    }

		    rv = soc_sirius_qs_leaf_node_to_queue_get(unit, leaf_node, (uint32 *)&base_queue);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to find queue mapped to leaf node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), leaf_node, unit));
			return rv;
		    }

		    rv = soc_sirius_qs_queue_to_leaf_node_get(unit, base_queue, &mc, &node);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to find leaf node mapped to queue %d, Unit(%d)\n"),
			           FUNCTION_NAME(), base_queue, unit));
			return rv;
		    }

		    if (mc) {
			BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*out_gport, base_queue);			
		    } else {
			BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*out_gport, base_queue);
		    }
		}
	    }
	}
    }

    return rv;
}

/* NOTE:
 *   swap 2 sibling nodes  
 */
static int
bcm_sirius_cosq_sibling_node_swap(int unit, int level, int orig_node, int dest_node, int egress)
{
    int rv = BCM_E_NONE;
    int tmp_node = -1;
    int logical_scheduler;
    int l0children;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_o = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_d = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t is_t;

    if (egress) {
	return BCM_E_UNAVAIL;
    } else {
	/* find a free node in the level */
	rv = bcm_sirius_cosq_scheduler_allocate(unit, level, BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT,
						1, egress, &tmp_node);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to allocation temporary node on level %d for node swap,  Unit(%d)\n"),
	               FUNCTION_NAME(), level, unit));
	    return BCM_E_RESOURCE;
	}

	is_d = &is_state[unit][level][dest_node];
	is_o = &is_state[unit][level][orig_node];
	
	/* clone the orig_node to tmp_node */
	rv = soc_sirius_ts_node_clone(unit, level, orig_node, tmp_node);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to clone ingress scheduler level %d node %d to node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, orig_node, tmp_node, unit));
	    return rv;
	} else {
	    sal_memcpy(&is_t, is_o, sizeof(bcm_sbx_sirius_ingress_scheduler_state_t));
	}

	rv = soc_sirius_ts_node_clone(unit, level, dest_node, orig_node);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to clone ingress scheduler level %d node %d to node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, dest_node, orig_node, unit));
	    return rv;
	} else {
	    sal_memcpy(is_o, is_d, sizeof(bcm_sbx_sirius_ingress_scheduler_state_t));
	}

	rv = soc_sirius_ts_node_clone(unit, level, tmp_node, dest_node);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to clone ingress scheduler level %d node %d to node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, tmp_node, dest_node, unit));
	    return rv;
	} else {
	    sal_memcpy(is_d, &is_t, sizeof(bcm_sbx_sirius_ingress_scheduler_state_t));
	}

        /*
         *  If level 1 nodes are being swapped, the software state for their
         *  children must be updated so they follow their parents according
         *  to the hardware constraints.
         */
        if (1 == level) {
            if (SOC_SBX_CFG_SIRIUS(unit)->b8kNodes) {
                l0children = 8;
            } else {
                l0children = 4;
            }
            for (logical_scheduler = 0;
                 logical_scheduler < l0children;
                 logical_scheduler++) {
                sal_memcpy(&is_t,
                           &(is_state[unit][level-1][(orig_node*l0children)+logical_scheduler]),
                           sizeof(is_t));
                sal_memcpy(&(is_state[unit][level-1][(orig_node*l0children)+logical_scheduler]),
                           &(is_state[unit][level-1][(dest_node*l0children)+logical_scheduler]),
                           sizeof(is_t));
                sal_memcpy(&(is_state[unit][level-1][(dest_node*l0children)+logical_scheduler]),
                           &is_t,
                           sizeof(is_t));
                is_state[unit][level-1][(orig_node*l0children)+logical_scheduler].parent = orig_node;
                is_state[unit][level-1][(dest_node*l0children)+logical_scheduler].parent = dest_node;
            }
            is_d->first_child = dest_node * l0children;
            is_o->first_child = orig_node * l0children;
        }
	
	/* update logical schedulers */
	logical_scheduler = is_o->logical_scheduler;
	if (logical_scheduler >= 0) {
	    /* update the logical scheduler mapping to physical scheduler */
	    SOC_SBX_STATE(unit)->ingress_scheduler_state[logical_scheduler].node = orig_node;
	} else {
	    /* NOTE: level 5 nodes may associated to ports instead of logical scheduler, 
	     * in this case, the subport_info will be updated, which already happened during
	     * node clone
	     */
	}
	
	logical_scheduler = is_d->logical_scheduler;
	if (logical_scheduler >= 0) {
	    /* update the logical scheduler mapping to physical scheduler */
	    SOC_SBX_STATE(unit)->ingress_scheduler_state[logical_scheduler].node = dest_node;
	} else {
	    /* NOTE: level 5 nodes may associated to ports instead of logical scheduler, 
	     * in this case, the subport_info will be updated, which already happened during
	     * node clone
	     */
	}

	/* free the tmp node */
        rv = bcm_sirius_cosq_scheduler_free(unit, level, tmp_node, FALSE);
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, failed to free level %d node %d ingress scheduler, Unit(%d)\n"),
                       FUNCTION_NAME(), level, tmp_node, unit));
            return rv;
        }

        /*
         *  Only change the parent/child links for children when the
         *  parent is above level 1; the soc_sirius_ts_node_clone above
         *  made the adjustments to queue_to_sc_* and leafnode_to_queue.
         */
        /* update all children of the dest node and orig node */
        for (tmp_node = is_d->first_child; tmp_node < is_d->first_child + is_d->num_child; tmp_node++) {
            is_state[unit][level-1][tmp_node].parent = dest_node;
            if (level > 1) {
                rv = soc_sirius_ts_node_hierachy_config(unit, level-1, tmp_node, dest_node,
                                                        SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                                        SOC_SIRIUS_API_PARAM_NO_CHANGE);
                if (rv != SOC_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: %s, failed to update ingress scheduler level %d node %d parent, Unit(%d)\n"),
                               FUNCTION_NAME(), level, tmp_node, unit));
                    return rv;
                }
            }
        }

        for (tmp_node = is_o->first_child; tmp_node < is_o->first_child + is_o->num_child; tmp_node++) {
            is_state[unit][level-1][tmp_node].parent = orig_node;
            if (level > 1) {
                rv = soc_sirius_ts_node_hierachy_config(unit, level-1, tmp_node, orig_node,
                                                        SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                                        SOC_SIRIUS_API_PARAM_NO_CHANGE);
                if (rv != SOC_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: %s, failed to update ingress scheduler level %d node %d parent, Unit(%d)\n"),
                               FUNCTION_NAME(), level, tmp_node, unit));
                    return rv;
                }
            }
        }

	/* regenerate the child_pri/current_pri/current_child for the dest_node */
	rv = soc_sirius_ts_node_regenerate_pri(unit, level, dest_node, NULL, FALSE);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to update ingress scheduler level %d node %d child priorities, Unit(%d)\n"),
	               FUNCTION_NAME(), level, dest_node, unit));
	    return rv;
	}

	/* regenerate the child_pri/current_pri/current_child for the orig_node and bubble up */
	rv = soc_sirius_ts_node_regenerate_pri(unit, level, orig_node, NULL, TRUE);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to update ingress scheduler level %d node %d child priorities, Unit(%d)\n"),
	               FUNCTION_NAME(), level, orig_node, unit));
	    return rv;
	}
    }

    return rv;
}


/* NOTE:  Shifting requirement for various nodes
 *   regular nodes:    bucket_ptr is the new node ID.
 *   flat master:      bucket_ptr is the new node ID,
 *                     all slaves need to point to the new master
 *   flat slave:       bucket_ptr still points to old master (remains same)
 *   multipath master: bucket_ptr is the new node ID
 *                     all slaves need to point to the new master
 *                     last node of the multipath need to point NG to new master
 *   multipath slave:  bucket_ptr still points to old master (remains same)
 *                     slave's previous node need to point to new next node
 */

/* Description:
 *   search for a free node on specified level and starting node in the specified direction,
 *   shift all nodes between the starting node and the first available node so that we
 *   can add a node as the sibling of the starting node
 * level:       ingress scheduler level
 * node:        start node of search
 * direction:   TRUE -> search for higher nodes
 *              FALSE -> search for lower nodes
 */
static int
bcm_sirius_cosq_ingress_scheduler_shift(int unit, int level, int node, int direction)
{
    int rv = BCM_E_NONE;
    int tmp_node, start_node, dest_node, orig_node, loop_cnt;
    int logical_scheduler;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_c = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_p = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_o = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_d = NULL;

    start_node = node;
    loop_cnt = 0;

    while (1) {
	/* use loop_cnt as a fail safe method to prevent infinite loops */
	if (loop_cnt >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) {
	    /* This should never happen unless the algorithms has a bug */
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d start node %d shift internal failure, Unit(%d)\n"),
	               FUNCTION_NAME(), level, start_node, unit));
	    return BCM_E_INTERNAL;
	} else {
	    loop_cnt++;
	}

	/* search for free node in the specified direction */
	dest_node = -1;
	if (direction == TRUE) {
	    for (tmp_node = node; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; tmp_node++) {
		if (is_state[unit][level][tmp_node].in_use == FALSE) {
		    dest_node = tmp_node;
		    break;
		}
	    }
	} else {
	    for (tmp_node = node; tmp_node >= 0; tmp_node--) {
		if (is_state[unit][level][tmp_node].in_use == FALSE) {
		    dest_node = tmp_node;
		    break;
		}
	    }
	}
	if (dest_node < 0) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d exhausted, Unit(%d)\n"),
	               FUNCTION_NAME(), level, unit));
	    return BCM_E_RESOURCE;
	}
	is_d = &is_state[unit][level][dest_node];
	if ( is_d->in_use != FALSE ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d dest node %d state inconsistent, Unit(%d)\n"),
	               FUNCTION_NAME(), level, dest_node, unit));
	    return BCM_E_INTERNAL;
	}

	/* Clone the first or last child to the new node depends on direction
	 */
	if (direction == TRUE) {
	    tmp_node = dest_node - 1;
	} else {
	    tmp_node = dest_node + 1;
	}
	
	/* find the node next to the free node */
	is_c = &is_state[unit][level][tmp_node];
	if ( ((is_c->in_use == TRUE) && (is_c->parent < 0)) ||
	     (is_c->in_use == FALSE) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d node %d state inconsistent, Unit(%d)\n"),
	               FUNCTION_NAME(), level, tmp_node, unit));
	    return BCM_E_INTERNAL;
	} else if (is_c->in_use == SIRIUS_INGRESS_SCHEDULER_RESERVED) {
	    /* the node is reserved but not used, only the in_use state could be different
	     * update it so that next time this node will be found.
	     */
	    sal_memcpy(is_c, is_d, sizeof(bcm_sbx_sirius_ingress_scheduler_state_t));
	    is_d->in_use = SIRIUS_INGRESS_SCHEDULER_RESERVED;
	    continue;
	}

	/* find the parent */
	is_p = &is_state[unit][level+1][is_c->parent];
	if ( (is_p->in_use != TRUE) || (is_p->first_child < 0) || (is_p->num_child <=0) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d node %d state inconsistent, Unit(%d)\n"),
	               FUNCTION_NAME(), level, is_c->parent, unit));
	    return BCM_E_INTERNAL;
	}
	/* find the first or last child sharing the same parent */
	if (direction == TRUE) {
	    /* first child of the group */
	    orig_node = is_p->first_child;
	    if (orig_node <= start_node) {
		/* shift reached the start node */
		break;
	    }
	} else {
	    /* last child of the group */
	    orig_node = is_p->first_child + is_p->num_child - 1;
	    if (orig_node >= start_node) {
		/* shift reached the start node */
		break;
	    }
	}
	is_o = &is_state[unit][level][orig_node];
	if ( (is_o->in_use != TRUE) || (is_o->parent != is_c->parent) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d node %d state inconsistent, Unit(%d)\n"),
	               FUNCTION_NAME(), level, orig_node, unit));
	    return BCM_E_INTERNAL;
	}
	
	/* clone the found node to the free node */
	rv = soc_sirius_ts_node_clone(unit, level, orig_node, dest_node);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to clone ingress scheduler level %d node %d to node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, orig_node, dest_node, unit));
	    return rv;
	}
	
	/* update parent (only need to update first child, num_child should remain same) */
	if (direction == TRUE) {
	    is_p->first_child++;
	} else {
	    is_p->first_child--;
	}
	rv = soc_sirius_ts_node_hierachy_config(unit, level+1, is_c->parent,
						SOC_SIRIUS_API_PARAM_NO_CHANGE,
						is_p->first_child,
						SOC_SIRIUS_API_PARAM_NO_CHANGE);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to update ingress scheduler level %d node %d first_child, Unit(%d)\n"),
	               FUNCTION_NAME(), level, is_c->parent, unit));
	    return rv;
	}

	/* update child weights of the parent */
	rv = soc_sirius_ts_child_weight_update(unit, level+1, is_c->parent, 
					       is_p->num_child, direction?1:0, -1, -1);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to update child weight for level %d node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level+1, is_c->parent, unit));
	    return rv;
	}
	
	/* update the orig_node hardware configs, dest_node hardware configs are done at clone time */
	rv = soc_sirius_scheduler_init(unit, level, orig_node, FALSE);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to reinit ingress scheduler level %d node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, orig_node, unit));
	    return rv;
	}

	/* update the orig_node and dest_node software states */
	sal_memcpy(is_d, is_o, sizeof(bcm_sbx_sirius_ingress_scheduler_state_t));
	logical_scheduler = is_o->logical_scheduler;
	if (logical_scheduler >= 0) {
	    /* update the logical scheduler mapping to physical scheduler */
	    SOC_SBX_STATE(unit)->ingress_scheduler_state[logical_scheduler].node = dest_node;
	} else {
	    /* NOTE: level 5 nodes may associated to ports instead of logical scheduler, 
	     * in this case, the subport_info will be updated, which already happened during
	     * node clone
	     */
	}
	
	/* update all children of the orig node */
	if (level == 1) {
	    for (tmp_node = 0; tmp_node < is_d->num_child; tmp_node++) {
		sal_memcpy(&is_state[unit][0][is_d->first_child+tmp_node],
			   &is_state[unit][0][is_o->first_child+tmp_node],
			   sizeof(bcm_sbx_sirius_ingress_scheduler_state_t));
		
		BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(&(is_state[unit][0][is_o->first_child+tmp_node]));
	    }
	} else {
	    for (tmp_node = is_d->first_child; tmp_node < is_d->first_child + is_d->num_child; tmp_node++) {
		is_state[unit][level-1][tmp_node].parent = dest_node;
		rv = soc_sirius_ts_node_hierachy_config(unit, level-1, tmp_node, dest_node,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to update ingress scheduler level %d node %d parent, Unit(%d)\n"),
		               FUNCTION_NAME(), level, tmp_node, unit));
		    return rv;
		}
	    }
	}

	/* regenerate the child_pri/current_pri/current_child for the dest_node */
	rv = soc_sirius_ts_node_regenerate_pri(unit, level, dest_node, NULL, TRUE);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to update ingress scheduler level %d node %d child priorities, Unit(%d)\n"),
	               FUNCTION_NAME(), level, dest_node, unit));
	    return rv;
	}

	BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(is_o);
    }

    return rv;
}

int
bcm_sirius_cosq_detach_scheduler(int unit,
				 bcm_gport_t parent_gport,
				 bcm_gport_t child_gport,
				 bcm_cos_queue_t cosq,
				 int egress)
{
    int rv = BCM_E_UNAVAIL;
    int child_level = 0, parent_level = 0;
    int child_node = 0, parent_node = 0;
    int node, base_node, queue, base_queue, cs_l = -1, ps_l = -1, eg_n = 0, num_fifos = -1, subport = 0;
    int num_child_level1, num_queues, num_level1_nodes, mc, level1_nodes[4], hide_level1 = FALSE;
    int level1_children[4];
    int32 node_type, bucket_type, bucket, bucket_new;
    int tmp_node, first_child;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *ps_i_state;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);
    bcm_sbx_sirius_egress_scheduler_state_t *ps_e_state;
    bcm_sbx_sirius_egress_scheduler_state_t *cs_e_state;
    int l5_node = -1, l6_node = -1, l7_node = -1, l6_offset = -1, l6_old_profile=-1;
    int l5_pri, l6_pri, l7_pri;
    uint32 old_shaper_loop = 0, old_shaper_start = 0, old_shaper_end = 0;
    uint32 uRegValue = 0;
    l6_nm_entry_t l6_nm_entry;

#ifndef NOTDEF_NEW_INTF    
    l4_n0_entry_t l4_n0_entry;
    l5_n0_entry_t l5_n0_entry;
    l6_n0_entry_t l6_n0_entry;
    l7_n0_entry_t l7_n0_entry;
    rt_st_entry_t rt_st_entry;
    int l4_node=-1, tmp_node2, l4_pri, current_pri, current_child, child_pri;
    soc_field_t child_pri_fields[8] = {CHILD_PRI0f, CHILD_PRI1f, CHILD_PRI2f, CHILD_PRI3f,
				       CHILD_PRI4f, CHILD_PRI5f, CHILD_PRI6f, CHILD_PRI7f};
#endif /* NOTDEF_NEW_INTF */

    LOG_DEBUG(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "parent_gport: 0x%x, child_gport: 0x%x\n"),
               parent_gport, child_gport));

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	/* egress scheduler detach */
	if (BCM_GPORT_IS_SCHEDULER(parent_gport) ||
	    BCM_GPORT_IS_EGRESS_CHILD(parent_gport) ||
	    BCM_GPORT_IS_EGRESS_MODPORT(parent_gport)) {
	    /* allow level 2 and level 1 be scheduler type */
	    rv = bcm_sbx_port_get_scheduler(unit, parent_gport, &parent_level, &parent_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), parent_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    if (BCM_GPORT_IS_SCHEDULER(parent_gport)) {
		ps_l = BCM_INT_SBX_SCHEDULER_ID_GET(parent_gport);
	    }
	} else {
	    /* unsupported ingress child gport type */
	    return BCM_E_PARAM;
	}

	if (BCM_GPORT_IS_SCHEDULER(child_gport)) {
	    /* allow level 2 and level 1 be scheduler type */
	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    cs_l = BCM_INT_SBX_SCHEDULER_ID_GET(child_gport);
	} else if ( BCM_GPORT_IS_EGRESS_GROUP(child_gport) ) {
	    if ( !BCM_GPORT_IS_SCHEDULER(parent_gport) ) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, egress group must attach to a scheduler gport,  Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_PARAM;
	    }

	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return BCM_E_INTERNAL;
	    }

	    subport = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, child_gport, &subport, &eg_n, &num_fifos);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		           FUNCTION_NAME(), subport, child_gport, unit));
		return BCM_E_PARAM;
	    }
	    
	    cs_l = BCM_GPORT_EGRESS_GROUP_GET(child_gport);
	    SOC_SBX_STATE(unit)->egress_group_state[cs_l].eg_scheduler = ps_l;
	} else {
	    /* unsupported egress child gport type */
	    return BCM_E_PARAM;
	}

	if  (parent_level != (child_level + 1)) {
	    return BCM_E_PARAM;
	}

	ps_e_state = cs_e_state = NULL;

	if ((parent_level < 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
	    (parent_node >=  SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[parent_level])) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid parent level %d node %d, unit %d\n"),
	               FUNCTION_NAME(), parent_level, parent_node, unit));
	    return BCM_E_PARAM;
	}
	
	ps_e_state = &es_state[unit][parent_level][parent_node];
	if (ps_e_state->in_use == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Parent node not initialized,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_INTERNAL;
	}
	
	switch (child_level) {
	    case SIRIUS_ES_LEVEL_FIFO:
		/* detach all fifo nodes to the parent (subport) node and free all fifo nodes */
		if ( !BCM_GPORT_IS_SCHEDULER(parent_gport) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Level %d egress scheduler must attach to an egress scheduler gport,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, unit));
		    return BCM_E_PARAM;
		}

		base_node = child_node;
		for (node = base_node; node < (base_node + num_fifos); node++) {
		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, child_level, node, -1, TRUE, &tmp_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, ES failed to detach fifo node %d\n"),
			           FUNCTION_NAME(), node));
			return rv;
		    }

		    if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
			/* allocate fifo node when using default setup */
			rv = bcm_sirius_cosq_scheduler_free(unit, child_level, tmp_node, egress);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to free child level %d node %d (was %d) egress scheduler node,  Unit(%d)\n"),
			               FUNCTION_NAME(), child_level, tmp_node, node, unit));
			    return BCM_E_RESOURCE;
			}
		    }
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		sp_info->es_scheduler_level1_node[eg_n] = -1;

        /*
		 * Free fifo memory resources
		 */
        rv = soc_sirius_config_ff(unit, (uint16)subport, (uint8)eg_n, FF_MEM_DELETE);
		break;
	    case SIRIUS_ES_LEVEL_SUBPORT:
		/* detach from parent (channel) node and free the subport node */
		if ( !BCM_GPORT_IS_EGRESS_CHILD(parent_gport) &&
		     !BCM_GPORT_IS_SCHEDULER(parent_gport) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Level %d egress scheduler must attach to an egress child or scheduler gport,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, unit));
		    return BCM_E_PARAM;
		}

		rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, child_level, child_node, -1, TRUE, &tmp_node);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, ES failed to detach subport node %d\n"),
		               FUNCTION_NAME(), child_node));
		    return rv;
		}

		rv = bcm_sirius_cosq_scheduler_free(unit, child_level, tmp_node, egress);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free child level %d node %d (was %d) egress scheduler node,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, tmp_node, child_node, unit));
		    return BCM_E_RESOURCE;
		}

		if (BCM_GPORT_IS_SCHEDULER(child_gport) &&
		    (cs_l >= 0) &&
		    (SOC_SBX_STATE(unit)->egress_scheduler_state != NULL)) {
		    SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].node = -1;
		    SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].level = -1;
		}
		break;
	    case SIRIUS_ES_LEVEL_CHANNEL:
		/* detach to parent (intf) node and free the channel node */
		if ( !BCM_GPORT_IS_EGRESS_MODPORT(parent_gport) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Level %d egress scheduler must attach to an egress modport gport,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, unit));
		    return BCM_E_PARAM;
		}

		rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, child_level, child_node, -1, TRUE, &tmp_node);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, ES failed to detach subport node %d\n"),
		               FUNCTION_NAME(), child_node));
		    return rv;
		}

		rv = bcm_sirius_cosq_scheduler_free(unit, child_level, tmp_node, egress);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free child level %d node %d (was %d) egress scheduler node,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, tmp_node, child_node, unit));
		    return BCM_E_RESOURCE;
		}

		if (BCM_GPORT_IS_SCHEDULER(child_gport) &&
		    (cs_l >= 0) &&
		    (SOC_SBX_STATE(unit)->egress_scheduler_state != NULL)) {
		    SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].node = -1;
		    SOC_SBX_STATE(unit)->egress_scheduler_state[cs_l].level = -1;
		}
		break;
	    default:
		return BCM_E_INTERNAL;
	}
    } else {
	/* ingress scheduler detach */
	child_level = -1;
	parent_level = -1;
	child_node = -1;
	parent_node = -1;
	base_queue = -1;
	cs_l = -1;
	ps_l = -1;

	/* Decode gport and get level/node info */
	if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(child_gport)) {
	    /* detaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(child_gport)) {
	    /* detaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(child_gport)) {
	    /* detaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(child_gport)) {
	    /* detaching a queue group, which really is level 0 */
	    child_level = 0;
	    base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(child_gport);
	} else if (BCM_GPORT_IS_CHILD(child_gport)) {
	    /* detaching a subport, whose level is determined at init time */
	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for subport gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return BCM_E_INTERNAL;
	    }
	} else if (BCM_GPORT_IS_SCHEDULER(child_gport)) {
	    /* detaching a ingress scheduler, level/node should be know at this time */
	    rv = bcm_sbx_port_get_scheduler(unit, child_gport, &child_level, &child_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for scheduler gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), child_gport, unit));
		return rv;
	    }

	    /* if SDK manage resource, get logical scheduler id */
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode != TRUE) {
		cs_l = BCM_INT_SBX_SCHEDULER_ID_GET(child_gport);
		LOG_DEBUG(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "cs_l: 0x%x\n"),
		           cs_l));
	    }
	} else {
	    /* unsupported ingress child gport type */
	    return BCM_E_PARAM;
	}

	if (BCM_GPORT_IS_SCHEDULER(parent_gport)) {
	    /* detaching a ingress scheduler, level/node should be know at this time */
	    rv = bcm_sbx_port_get_scheduler(unit, parent_gport, &parent_level, &parent_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for scheduler gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), parent_gport, unit));
		return rv;
	    }

	    /* if SDK manage resource, get logical scheduler id */
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode != TRUE) {
		ps_l = BCM_INT_SBX_SCHEDULER_ID_GET(parent_gport);
		LOG_DEBUG(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ps_l: 0x%x\n"),
		           ps_l));
	    }
	} else if ( (BCM_GPORT_IS_CHILD(parent_gport)) || (BCM_GPORT_IS_MODPORT(parent_gport)) ) {
	    /* detaching to a subport or interface, whose level is determined at init time */
	    rv = bcm_sbx_port_get_scheduler(unit, parent_gport, &parent_level, &parent_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Scheduler unknown for subport gport(0x%x), Unit(%d)\n"),
		           FUNCTION_NAME(), parent_gport, unit));
		return BCM_E_INTERNAL;
	    }
	} else {
	    /* unsupported ingress child gport type */
	    return BCM_E_PARAM;
	}

	if ( (child_level == -1) || ((child_level != 0) && (child_node == -1)) ||
	     (parent_level == -1) || (parent_level == -1) ) {
	    /* child or parent level/node unknown */
	    return BCM_E_PARAM;
	} else {
	    if ((sir->bSubscriberNodeOptimize == TRUE) && 
		(sir->uSubscriberMaxCos == 8) && 
		(sir->b8kNodes == FALSE) && 
		(child_level == 0) && 
		(parent_level == 2)) {
		/* when optimize node usage, max cos level is 8 and TS hardware is 
		 * configured to be 4 leaf nodes for each level 1 node, we hide level 1 nodes
		 * allow user to connect level 0 to level 2 directly
		 */
		hide_level1 = TRUE;
	    } else if ( parent_level != (child_level + 1) ) {
		/* direct connect with levels in between auto created */
		
		return BCM_E_UNAVAIL;
	    }
	}

	/* Check scheduler node */
	if ((parent_level < 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ||
	    (parent_node >=  SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[parent_level])) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid parent level %d node %d, unit %d\n"),
	               FUNCTION_NAME(), parent_level, parent_node, unit));
	    return BCM_E_PARAM;
	}
	ps_i_state = &is_state[unit][parent_level][parent_node];
	if (ps_i_state->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Parent node not initialized,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_INTERNAL;
	}

	if (child_level == 0) {
	    if (hide_level1 == FALSE) {
		/* detaching queue to level 1 scheduler, level 0 leaf nodes are fixed mapped
		 * to level 1 nodes. mark all corresponding leaf nodes to be used
		 */
		if (sir->b8kNodes) {
		    /* 8 child per level 1 node */
		    num_child_level1 = 8;
		} else {
		    /* 4 child per level 1 node */
		    num_child_level1 = 4;
		}
		
		if (cosq > num_child_level1) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, cos level out of range [0-%d),  Unit(%d)\n"),
		               FUNCTION_NAME(), num_child_level1, unit));
		    return BCM_E_PARAM;
		} else if (cosq > 0) {
		    num_queues = cosq;
		} else {
		    /* provided cos level out of range, use what's supported in hardware natually */
		    num_queues = num_child_level1;
		}

		base_node = parent_node * num_child_level1;
		for (node = base_node; node < (base_node + num_child_level1); node++) {
		    queue = base_queue + node - base_node;
		    if (queue >= (base_queue + num_queues)) {
			/* only detach the number of queues specified in API, even though all leaf nodes will be
			 * reserved
			 */
			continue;
		    }

		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, child_level,
							      node, queue, egress, &tmp_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disconnect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, node, parent_level, parent_node, unit));
			return rv;
		    }

		    /* free the scheduler */
		    rv = bcm_sirius_cosq_scheduler_free(unit, child_level, tmp_node, egress);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not free level %d node %d (was %d),  Unit(%d)\n"),
			           FUNCTION_NAME(),
			           child_level, tmp_node, node, unit));
			return rv;
		    }
		}
	    } else {
		/* need to deallocate level 1 nodes based on num of cos levels */
		if (cosq <= 4) {
		    num_level1_nodes = 1;
		    if (cosq > 0) {
			num_queues = cosq;
		    } else {
			num_queues = 4;
		    }
		} else if (cosq <= 8) {
		    /* alloc 2 level 1 nodes */
		    num_level1_nodes = 2;
		    num_queues = cosq;
		} else {
		    /* assuming 16 cos levels, allocate 4 level 1 nodes */
		    num_level1_nodes = 4;
		    num_queues = (cosq > 16)? 16:cosq;
		}
		
		/* find out the level 1 nodes used */
		rv = soc_sirius_qs_queue_to_leaf_node_get(unit, base_queue, &mc, &level1_nodes[0]);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to get leaf node for subscriber queue %d,  Unit(%d)\n"),
		               FUNCTION_NAME(), base_queue, unit));
		    return rv;
		}
		/* always map 4 leaf node to 1 level 1 node in this case */
		level1_nodes[0] = level1_nodes[0]/4;
		level1_nodes[1] = level1_nodes[0] + 1;
		level1_nodes[2] = level1_nodes[1] + 1;
		level1_nodes[3] = level1_nodes[2] + 1;

		for (node = (num_level1_nodes-1); node >= 0; node--) {
		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, 1,
							      level1_nodes[node], -1, egress, &(level1_children[node]));
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disconnect level 1 node %d from parent level %d node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), level1_nodes[node], parent_level, parent_node, unit));
			return rv;
		    }
		}

		/* detach level 0 nodes to corresponding level 1 nodes, here hardware is always mapping
		 * 4 leaf nodes to 1 level 1 node
		 */
		base_node = level1_nodes[0] * 4;
		for (node = base_node; node < (base_node + num_level1_nodes*4); node++) {
		    queue = base_queue + node - base_node;
		    if (queue >= (base_queue + num_queues)) {
			/* only detach the number of queues specified in API, even though all leaf nodes will be
			 * reserved
			 */
			continue;
		    }

		    rv = bcm_sirius_cosq_scheduler_disconnect(unit, 1, level1_nodes[(node-base_node)/4],
							      child_level, node, queue, egress, &tmp_node);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disconnect child level %d node %d from parent level %d node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), child_level, node, parent_level, parent_node, unit));
			return rv;
		    }

		    /* free the scheduler */
		    rv = bcm_sirius_cosq_scheduler_free(unit, child_level, tmp_node, egress);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not free level %d node %d (was %d), Unit(%d)\n"),
			           FUNCTION_NAME(),
			           child_level, tmp_node, node, unit));
			return rv;
		    }
		}

		/* free level 1 nodes, following function will free a chunk of nodes */
		for (node = (num_level1_nodes-1); node >= 0; node--) {
		    rv = bcm_sirius_cosq_scheduler_free(unit, 1, level1_children[node], egress);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, can not free level 1 node %d,  Unit(%d)\n"),
			           FUNCTION_NAME(), level1_children[node], unit));
			return rv;
		    }
		}
	    }
	} else {
	    /* intermediate levels, disconnect child node */

	    /* user management mode */
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
		rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, child_level,
							  child_node, -1, egress, &tmp_node);
                
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to disconnect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, child_node, parent_level, parent_node, unit));
		}
		return rv;
	    }

	    /* if the disconnected node is part of flat scheduler and its bucket is used, elect
	     * a new bucket for the flat scheduler
	     */
	    rv = soc_sirius_ts_node_mapping_config_get(unit, child_level, child_node,
						       &node_type, &bucket_type, &bucket);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to get child level %d node %d bucket info,  Unit(%d)\n"),
		           FUNCTION_NAME(), child_level, child_node, unit));
		return BCM_E_RESOURCE;
	    }

	    /* select new bucket when the detached node's bucket is used for now and
	     * the detached node is the last child node of its parent
	     */
	    if ( (bucket_type == SIRIUS_TS_BUCKET_TYPE_SUBTREE) &&
		 (bucket == child_node) &&
		 (child_node == (is_state[unit][parent_level][parent_node].first_child+
		                 is_state[unit][parent_level][parent_node].num_child-1))) {
		/* when the child node is part of flat scheduler and the nodes of the
		 * flat scheduler on that level is pointing to the bucket of this node
		 * need to elect a new node on the level, keep it to be the first node
		 * of the level
		 */
		bucket_new = -1;
		for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level]; tmp_node++) {
		    if ( (is_state[unit][child_level][tmp_node].bucket == child_node) &&
			 (tmp_node != child_node) ) {
			if (bucket_new == -1) {
			    /* find the lowest node in the flat scheduler sharing the bucket 
			     * and pick the bucket of that node as the new bucket
			     */
			    bucket_new = tmp_node;
			}

			/* update all nodes to point to new bucket */
			rv = soc_sirius_ts_node_mapping_config(unit, child_level, tmp_node,
							       SOC_SIRIUS_API_PARAM_NO_CHANGE,
							       SOC_SIRIUS_API_PARAM_NO_CHANGE,
							       bucket_new);
			if (rv != SOC_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, fail to update ingress scheduler level %d node %d to use bucket %d, Unit(%d)\n"),
			               FUNCTION_NAME(), child_level, tmp_node, bucket_new, unit));
			    return rv;
			}
			is_state[unit][child_level][tmp_node].bucket = bucket_new;
		    }
		}
	    }

	    /* Get the tree hierachy */
	    if (child_level == 5) {
		l6_node = parent_node;
		l7_node = is_state[unit][6][l6_node].parent;
		if (l7_node < 0) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, can not find parent of level 6 node %d,  Unit(%d)\n"), 
		               FUNCTION_NAME(), l6_node, unit));
		    return SOC_E_INTERNAL;
		}

		l6_offset = l6_node - is_state[unit][7][l7_node].first_child;
		if ((l6_offset < 0) || (l6_offset >= 8)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, TS level 6 node %d is not child of level 7 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l6_node, l7_node, unit));
		    return SOC_E_INTERNAL;
		}

		/* STEP1: point l6_node to a node profile that always report priority 0 */
		SOC_IF_ERROR_RETURN(READ_L6_NMm(unit, MEM_BLOCK_ANY, l6_node, &l6_nm_entry));
		l6_old_profile = soc_mem_field32_get(unit, L6_NMm, &l6_nm_entry, NODE_PROFILE_PTRf);
		
		rv = soc_sirius_ts_node_creditor_config(unit, 6, l6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SIRIUS_TS_NODE_PROFILE_TEMPLATE_MAX);
		
		if (rv != SOC_E_NONE) {
		  LOG_ERROR(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "ERROR: %s, failed to config ingress creditor node profile for level 6 node %d, Unit(%d)\n"),
		             FUNCTION_NAME(), l6_node, unit));
		  return rv;		    
		}

		/* disable all queue level shapers */
		SOC_IF_ERROR_RETURN(READ_SHAPER_LOOP_SIZEr(unit, &old_shaper_loop));
		SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &old_shaper_start));
		SOC_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, &old_shaper_end));
		
		SOC_IF_ERROR_RETURN(WRITE_SHAPER_LOOP_SIZEr(unit, 0));
		SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, 0));
		SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, 0));
		
		/* disable backpressure */
		SOC_IF_ERROR_RETURN(READ_FR_CONFIG0r(unit, &uRegValue));
		soc_reg_field_set(unit, FR_CONFIG0r, &uRegValue, FORCE_FULL_STATUS_DEBUGf, 1);
		SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG0r(unit, uRegValue));
		sal_usleep(10);
	    }

	    /* handle the reserved nodes */
	    first_child = is_state[unit][parent_level][parent_node].first_child;

	    rv = bcm_sirius_cosq_scheduler_disconnect(unit, parent_level, parent_node, child_level,
						   child_node, -1, egress, &tmp_node);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to disconnect child level %d node %d to parent level %d node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), child_level, child_node, parent_level, parent_node, unit));
		return BCM_E_RESOURCE;
	    }

	    /* free the child node */
	    rv = bcm_sirius_cosq_scheduler_free(unit, child_level, tmp_node, egress);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, can not free level %d node %d (was %d),  Unit(%d)\n"),
		           FUNCTION_NAME(),
		           child_level, tmp_node, child_node, unit));
		return rv;
	    }

	    /*
	     * Steps in case of the detached node is the current child of the parent, 
	     *    we have to update the priorities up the tree till root level
	     * For now, support it on detach of Level 5 node only.
	     */
	    if (child_level == 5) {
	        /*== repopulate the L5_n0 node being delete, current disconnect code
		 *   will clear the n0 table
		 */
		l5_node = child_node;
#ifdef NOTDEF_NEW_INTF
		rv = soc_sirius_ts_node_regenerate_pri(unit, 5, l5_node, (uint32 *)&l5_pri, FALSE);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to regenerate priority for level 5 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l5_node, unit));
		    return rv;
		}		

		/*== read and update L6 priorities if need */
		rv = soc_sirius_ts_node_regenerate_pri(unit, 6, l6_node, (uint32 *)&l6_pri, FALSE);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to regenerate priority for level 6 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l6_node, unit));
		    return rv;
		}		

		/*== reenable the level 6 node to report priority */
		rv = soc_sirius_ts_node_creditor_config(unit, 6, l6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							l6_old_profile);
		
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to config ingress creditor node profile for level 6 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l6_node, unit));
		    return rv;		    
		}

		/*== read and update L7 priorities if need */
		rv = soc_sirius_ts_node_update_child_pri(unit, 7, l7_node, l6_node, l6_pri, (uint32 *)&l7_pri);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to update child %d priority for level 7 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l6_node, l7_node, unit));
		    return rv;
		}		

		/*== read and update Root priorities if need */
		rv = soc_sirius_ts_node_update_child_pri(unit, 8, l7_node, l7_node, l7_pri, NULL);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to regenerate priority for level 7 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l7_node, unit));
		    return rv;
		}		
#else
		SOC_IF_ERROR_RETURN(READ_L5_N0m(unit, MEM_BLOCK_ANY, l5_node, &l5_n0_entry));

		/* find the highest priority among valid children and update current child/pri */
		current_pri = 0;
		current_child = 0;
		for (tmp_node2 = 0;
		     tmp_node2 < 8;
		     tmp_node2++) {
		    if (tmp_node2 < is_state[unit][5][l5_node].num_child) {
			/* read l4 child's current_pri */
			l4_node = is_state[unit][5][l5_node].first_child + tmp_node2;
			SOC_IF_ERROR_RETURN(READ_L4_N0m(unit, MEM_BLOCK_ANY, l4_node, &l4_n0_entry));
			l4_pri = soc_mem_field32_get(unit, L4_N0m, &l4_n0_entry, CURRENT_PRIf);
		    } else {
			l4_pri = 0;
		    }

		    /* set the child's child_pri in L5_n0 */
		    soc_mem_field32_set(unit, L5_N0m, &l5_n0_entry, child_pri_fields[tmp_node2], l4_pri);
		    if (l4_pri > current_pri) {
			current_pri = l4_pri;
			current_child = tmp_node2;
		    }
		}
		soc_mem_field32_set(unit, L5_N0m, &l5_n0_entry, CURRENT_PRIf, current_pri);
		soc_mem_field32_set(unit, L5_N0m, &l5_n0_entry, CURRENT_CHILDf, current_child);
		SOC_IF_ERROR_RETURN(WRITE_L5_N0m(unit, MEM_BLOCK_ANY, l5_node, &l5_n0_entry));
		l5_pri = current_pri;

		/*== read and update L6 priorities if need */
		SOC_IF_ERROR_RETURN(READ_L6_N0m(unit, MEM_BLOCK_ANY, l6_node, &l6_n0_entry));

		/* find the highest priority among valid children and update current child/pri */
		current_pri = 0;
		current_child = 0;
		for (tmp_node2 = 0;
		     tmp_node2 < 8;
		     tmp_node2++) {

		    if (tmp_node2 < is_state[unit][6][l6_node].num_child) {
			l5_node = is_state[unit][6][l6_node].first_child + tmp_node2;
			SOC_IF_ERROR_RETURN(READ_L5_N0m(unit, MEM_BLOCK_ANY, l5_node, &l5_n0_entry));
			l5_pri = soc_mem_field32_get(unit, L5_N0m, &l5_n0_entry, CURRENT_PRIf);
		    } else {
			l5_pri = 0;
		    }

		    soc_mem_field32_set(unit, L6_N0m, &l6_n0_entry, child_pri_fields[tmp_node2], l5_pri);
		    if (l5_pri > current_pri) {
			current_pri = l5_pri;
			current_child = tmp_node2;
		    }
		}
		soc_mem_field32_set(unit, L6_N0m, &l6_n0_entry, CURRENT_PRIf, current_pri);
		soc_mem_field32_set(unit, L6_N0m, &l6_n0_entry, CURRENT_CHILDf, current_child);
		SOC_IF_ERROR_RETURN(WRITE_L6_N0m(unit, MEM_BLOCK_ANY, l6_node, &l6_n0_entry));

		/*== reenable the level 6 node to report priority */
		rv = soc_sirius_ts_node_creditor_config(unit, 6, l6_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							l6_old_profile);
		
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to config ingress creditor node profile for level 6 node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), l6_node, unit));
		    return rv;		    
		}

		/*== read and update L7 priorities if need */
		l6_pri = current_pri;
		SOC_IF_ERROR_RETURN(READ_L7_N0m(unit, MEM_BLOCK_ANY, l7_node, &l7_n0_entry));
		soc_mem_field32_set(unit, L7_N0m, &l7_n0_entry, child_pri_fields[l6_offset], l6_pri);
		
		/* find the highest priority among valid children and update current child/pri */
		current_pri = 0;
		current_child = 0;
		for (tmp_node2 = 0;
		     tmp_node2 < is_state[unit][7][l7_node].num_child;
		     tmp_node2++) {
		    child_pri = soc_mem_field32_get(unit, L7_N0m, &l7_n0_entry, child_pri_fields[tmp_node2]);
		    if (child_pri > current_pri) {
			current_pri = child_pri;
			current_child = tmp_node2;
		    }
		}
		soc_mem_field32_set(unit, L7_N0m, &l7_n0_entry, CURRENT_PRIf, current_pri);
		soc_mem_field32_set(unit, L7_N0m, &l7_n0_entry, CURRENT_CHILDf, current_child);
		SOC_IF_ERROR_RETURN(WRITE_L7_N0m(unit, MEM_BLOCK_ANY, l7_node, &l7_n0_entry));

		/*== read and update Root priorities if need */
		l7_pri=current_pri;
		SOC_IF_ERROR_RETURN(READ_RT_STm(unit, MEM_BLOCK_ANY, l7_node, &rt_st_entry));
		soc_mem_field32_set(unit, RT_STm, &rt_st_entry, CHILD_PRIf, l7_pri);
		SOC_IF_ERROR_RETURN(WRITE_RT_STm(unit, MEM_BLOCK_ANY, l7_node, &rt_st_entry));			
#endif /* NOTDEF_NEW_INTF */

		/* reenable backpressure */
		SOC_IF_ERROR_RETURN(READ_FR_CONFIG0r(unit, &uRegValue));
		soc_reg_field_set(unit, FR_CONFIG0r, &uRegValue, FORCE_FULL_STATUS_DEBUGf, 0);
		SOC_IF_ERROR_RETURN(WRITE_FR_CONFIG0r(unit, uRegValue));

		/* reenable queue level shapers */
		SOC_IF_ERROR_RETURN(WRITE_SHAPER_LOOP_SIZEr(unit, old_shaper_loop));
		SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, old_shaper_start));
		SOC_IF_ERROR_RETURN(WRITE_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, old_shaper_end));
	    }

	    /* mark the reserved nodes as free */
	    /* get the number of reserved nodes for the parent */
	    ps_l = is_state[unit][parent_level][parent_node].logical_scheduler;
	    if (SOC_SBX_STATE(unit)->ingress_scheduler_state[ps_l].num_childs > 0) {
		cosq = SOC_SBX_STATE(unit)->ingress_scheduler_state[ps_l].num_childs;
	    } else {
		cosq = 0;
	    }

	    if (first_child < 0) {
		cosq = 0;
	    } else {
		/* if it's not last children, remark the freed node as reserved,
		 * and make sure we don't mark it to be free later
		 */
		if (is_state[unit][parent_level][parent_node].num_child != 0) {
		    is_state[unit][child_level][tmp_node].in_use = SIRIUS_INGRESS_SCHEDULER_RESERVED;
		    cosq = 0;
		}
	    }

	    for (tmp_node = first_child; tmp_node < (first_child + cosq); tmp_node++) {
		if (is_state[unit][child_level][tmp_node].in_use == SIRIUS_INGRESS_SCHEDULER_RESERVED) {
		    is_state[unit][child_level][tmp_node].in_use = FALSE;
		}
	    }	    

	    if (cs_l >= 0) {
		/* clear mapping between logical scheduler and physical schedulers */
		SOC_SBX_STATE(unit)->ingress_scheduler_state[cs_l].level = -1;
		SOC_SBX_STATE(unit)->ingress_scheduler_state[cs_l].node = -1;
	    }

	    LOG_DEBUG(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "Update is_map, cs_l: 0x%x, level: 0x%x,"
	                           " node: 0x%x\n"), cs_l, child_level, tmp_node));
	}

    }

    return rv;
}

int
bcm_sirius_cosq_set_egress_scheduler_params(int unit,
					    int level,
					    int node,
					    int mode,
					    int weight)
{
    int rv = BCM_E_UNAVAIL;
    channel_map_table_entry_t channel_entry;
    subport_map_table_entry_t subport_entry;
    fifo_map_table_entry_t fifo_entry;
    int max_pri, max_node, parent, index, peer, peer_parent, peer_index;
    int found_peer;
    int config_mode;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    switch (level) {
	case SIRIUS_ES_LEVEL_CHANNEL:
	    max_pri = 63;
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
	    break;
	case SIRIUS_ES_LEVEL_SUBPORT:
	    max_pri = 63;
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
	    break;
	case SIRIUS_ES_LEVEL_FIFO:
	    max_pri = 15;
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0;
	    break;
	default:
	    return BCM_E_PARAM;
    }

    if ((mode == BCM_COSQ_EF) && (level != SIRIUS_ES_LEVEL_FIFO)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, egress scheduler level %d does not support EF scheduling discipline, Unit(%d)\n"),
	           FUNCTION_NAME(), level, unit));
        return(BCM_E_UNAVAIL);
    }

    /* Device specific mode re-map */
    switch (mode) {
        case BCM_COSQ_SP:
            config_mode = SIRIUS_ES_SCHEDULER_MODE_SP;
            break;
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
            config_mode = SIRIUS_ES_SCHEDULER_MODE_WERR;
            break;
        case BCM_COSQ_EF:
            config_mode = SIRIUS_ES_SCHEDULER_MODE_EF;
            break;
        default:
            config_mode = SIRIUS_ES_SCHEDULER_MODE_WERR;
            break;
    }


    if ((es_state[unit][level] == NULL) || (node < 0) ||
	(node >= max_node) || (es_state[unit][level][node].in_use != TRUE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, egress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    if ( (config_mode == SIRIUS_ES_SCHEDULER_MODE_SP) && ((weight < 0) || (weight > max_pri)) ) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, es strict pri mode invalid priority %d, valid range [0-%d]. Unit(%d)\n"),
                   FUNCTION_NAME(), weight, max_pri, unit));
        return BCM_E_PARAM;
    }

    if ( (config_mode == SIRIUS_ES_SCHEDULER_MODE_WERR) && ((weight < 0) || (weight > 0x7F)) ) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, es werr mode invalid weight %d, valid range [0-127]. Unit(%d)\n"),
                   FUNCTION_NAME(), weight, unit));
        return BCM_E_PARAM;
    }

    if ( (config_mode == SIRIUS_ES_SCHEDULER_MODE_EF) && (weight != 0) ) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, es ef mode invalid weight %d, Unit(%d), overriding it with 0\n"),
                   FUNCTION_NAME(), weight, unit));
        weight = 0;
    }

    if (config_mode == SIRIUS_ES_SCHEDULER_MODE_SP) {
	/* need to find the peer node (one with same parent and map_index)
	 * and swap the map_index with this node
	 */
	rv = soc_sirius_es_node_get_info(unit, level, node, &parent, &index);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, es failed to get level %d node %d parent node. Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));
	    return rv;
	}

	found_peer = FALSE;
	for (peer=0; peer < max_node; peer++) {
	    if (es_state[unit][level][peer].in_use != TRUE) {
		continue;
	    }

	    rv = soc_sirius_es_node_get_info(unit, level, peer, &peer_parent, &peer_index);
	    if (rv != SOC_E_NONE) {
		/* possible to mark the node as used (for flow control), but not config parents
		   LOG_ERROR(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s, es failed to get level %d node %d parent node. Unit(%d)\n"),
		              FUNCTION_NAME(), level, peer, unit));
		   return BCM_E_INTERNAL;
		*/
		continue;
	    }

	    if ( (parent == peer_parent) && (weight == peer_index) ) {
		found_peer = TRUE;
		break;
	    }
	}

	if (found_peer == TRUE) {
	    /* swap the map_index */
	    switch (level) {
		case SIRIUS_ES_LEVEL_CHANNEL:
		    /* swap the map_index with the peer */
		    BCM_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &channel_entry));
		    index = soc_mem_field32_get(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf);
		    soc_mem_field32_set(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf, peer_index);
		    BCM_IF_ERROR_RETURN(WRITE_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &channel_entry));
		    
		    BCM_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, peer, &channel_entry));
		    soc_mem_field32_set(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf, index);
		    BCM_IF_ERROR_RETURN(WRITE_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, peer, &channel_entry));
		    break;
		case SIRIUS_ES_LEVEL_SUBPORT:
		    BCM_IF_ERROR_RETURN(READ_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &subport_entry));
		    index = soc_mem_field32_get(unit, SUBPORT_MAP_TABLEm, &subport_entry, MAP_INDEXf);
		    soc_mem_field32_set(unit, SUBPORT_MAP_TABLEm, &subport_entry, MAP_INDEXf, peer_index);
		    BCM_IF_ERROR_RETURN(WRITE_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &subport_entry));

		    BCM_IF_ERROR_RETURN(READ_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, peer, &subport_entry));
		    soc_mem_field32_set(unit, SUBPORT_MAP_TABLEm, &subport_entry, MAP_INDEXf, index);
		    BCM_IF_ERROR_RETURN(WRITE_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, peer, &subport_entry));
		    break;
		case SIRIUS_ES_LEVEL_FIFO:
		    BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &fifo_entry));
		    index = soc_mem_field32_get(unit, FIFO_MAP_TABLEm, &fifo_entry, MAP_INDEXf);
		    soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MAP_INDEXf, peer_index);
		    BCM_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &fifo_entry));

		    BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, peer, &fifo_entry));
		    soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MAP_INDEXf, index);
		    BCM_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, peer, &fifo_entry));
		    break;
		default:
		    return BCM_E_INTERNAL;
	    }
	    es_state[unit][level][node].index = peer_index;
	    es_state[unit][level][peer].index = index;
	} else {
	    /* just config map_index to be the user provided priority */
	    index = weight;
	    switch (level) {
		case SIRIUS_ES_LEVEL_CHANNEL:
		    BCM_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &channel_entry));
		    soc_mem_field32_set(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf, index);
		    BCM_IF_ERROR_RETURN(WRITE_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &channel_entry));
		    break;
		case SIRIUS_ES_LEVEL_SUBPORT:
		    BCM_IF_ERROR_RETURN(READ_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &subport_entry));
		    soc_mem_field32_set(unit, SUBPORT_MAP_TABLEm, &subport_entry, MAP_INDEXf, index);
		    BCM_IF_ERROR_RETURN(WRITE_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &subport_entry));
		    break;
		case SIRIUS_ES_LEVEL_FIFO:
		    BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &fifo_entry));
		    soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MAP_INDEXf, index);
		    BCM_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, node, &fifo_entry));
		    break;
                /* coverity[dead_error_begin] */
		default:
		    return BCM_E_INTERNAL;
	    }
	    es_state[unit][level][node].index = index;	    
	}

    }

    /* min scheduler */
    rv = soc_sirius_es_node_scheduler_config(unit, level, node, TRUE, config_mode, weight);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to config min egress scheduler for level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return rv;
    }

    /* max scheduler */
    rv = soc_sirius_es_node_scheduler_config(unit, level, node, FALSE, config_mode, weight);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to config max egress scheduler for level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return rv;
    }

    es_state[unit][level][node].scheduler_mode = mode;
    es_state[unit][level][node].scheduler_param = weight;

    return rv;
}

int
bcm_sirius_cosq_get_egress_scheduler_params(int unit,
					    int level,
					    int node,
					    int *mode,
					    int *weight)
{
    int rv = BCM_E_NONE;
    int max_node;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    switch (level) {
	case SIRIUS_ES_LEVEL_CHANNEL:
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
	    break;
	case SIRIUS_ES_LEVEL_SUBPORT:
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
	    break;
	case SIRIUS_ES_LEVEL_FIFO:
	    max_node = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0;
	    break;
	default:
	    return BCM_E_PARAM;
    }

    if ((es_state[unit][level] == NULL) || (node < 0) ||
	(node >= max_node) || (es_state[unit][level][node].in_use != TRUE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, egress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    *mode = es_state[unit][level][node].scheduler_mode;
    *weight = es_state[unit][level][node].scheduler_param;

    return rv;
}

int
bcm_sirius_cosq_set_egress_shaper_params(int unit,
                                         int level,
                                         int node,
                                         int kbits_sec_min,
                                         int bits_thresh_min,
                                         int kbits_sec_max,
                                         int bits_thresh_max)
{
    int rv = BCM_E_UNAVAIL;
    int rate = 0;
    int enable = FALSE;
    int default_threshold=20000; /* affects CIR only */
    int max_threshold = 15000000; /* defautlt max threshold is 1.5Mbits */
    int shaper_rate;
    int tick_select;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (level == SIRIUS_ES_LEVEL_GROUP_SHAPER) {
	if ((gs_state[unit] == NULL) || (node < 0) || (node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER)
	    || (gs_state[unit][node].in_use != TRUE)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress group max shaper node %d not in use, Unit(%d)\n"),
	               FUNCTION_NAME(), node, unit));
	    return BCM_E_PARAM;
	}
    } else {
	if ((es_state[unit][level] == NULL) || (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
	    (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]) ||
	    (es_state[unit][level][node].in_use != TRUE)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress scheduler level %d node %d not in use, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));
	    return BCM_E_PARAM;
	}
    }

    if ( (level == SIRIUS_ES_LEVEL_INTERFACE) && (kbits_sec_min != 0) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, egress scheduler level %d nodes do not support min shapers, Unit(%d)\n"),
	           FUNCTION_NAME(), level, unit));
	return BCM_E_PARAM;
    }

    /* pick tick_select based on the minimum of valid rates, assuming min rate is
     *    always lower than max rate if both are valid (non-zero) rate
     *    make sure each refresh will have at least 9 bits credit but have less than
     *    (1.95us * rate) * 2^tick_sel >= 9 bits
     */
    if ( (kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) && (kbits_sec_min) ) {
	rate = kbits_sec_min;
    } else {
	if ( (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) && (kbits_sec_max) ) {
	    rate = kbits_sec_max;
	}
    }

    if (rate == 0) {
	if ( (kbits_sec_max == 0) && (kbits_sec_min == 0) ) {
	    /* both shapers are disabled */
	    tick_select = 0;
	} else {
	    tick_select = SOC_SIRIUS_API_PARAM_NO_CHANGE;
	}
    } else {
	tick_select = 0;
    }

    if (level != SIRIUS_ES_LEVEL_GROUP_SHAPER) {
	/* min shaper (CIR rate) */
	if (kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    if (kbits_sec_min == 0) {
		enable = FALSE;
	    } else {
		if (bits_thresh_min == 0) {
		    /* threshold not specified when rate is non-zero,
		     * SDK calculate the default threshold based on rate
		     * set the bitbucket to be 0x3FFF (14 bits)
		     */
		    bits_thresh_min = default_threshold;
		}
		enable = TRUE;
	    }
	} else {
	    enable = SOC_SIRIUS_API_PARAM_NO_CHANGE;
	}
	
	rv = soc_sirius_es_node_shaper_config(unit, level, node, TRUE, enable,
					      kbits_sec_min, bits_thresh_min, tick_select);
	
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to config min egress shaper for level %d node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));
	    return rv;
	}
    } else {
	if ((kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) &&
	    (kbits_sec_min != 0)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, group min shaper is not supported by sirius, Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return SOC_E_UNAVAIL;
	}	
    }

    /* max shaper (PIR rate) */
    if (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	shaper_rate = kbits_sec_max;
    } else {
	if (level == SIRIUS_ES_LEVEL_GROUP_SHAPER) {
	    shaper_rate = gs_state[unit][node].max_shaper_rate_kbps;
	} else {
	    shaper_rate = es_state[unit][level][node].max_shaper_rate_kbps;
	}
    }

    /* pick default threshold based on shaper rate */
    if (shaper_rate < 2000000) {
	default_threshold = 32000;      /* value proved in lab regression */
    } else if (shaper_rate < 4000000) {
	default_threshold = 64000;
    } else if (shaper_rate < 6000000) {
	default_threshold = 96000;
    } else if (shaper_rate < 8000000) {
	default_threshold = 128000;
    } else if (shaper_rate < 10000000) {
	default_threshold = 160000;
    } else {
	default_threshold = 208*1024;   /* value proved in lab regression */
    }

    /* pick max threshold based on shaper rate, here assumes the shaper rate is 
     * associated with port rate. Each block of fifo cache is 360 entries,
     * and number of blocks allocated for port is based on port rate.
     * also assumes that each fifo cache entry contains 40 bytes of data to be safe
     */
    if (shaper_rate < 2000000) {
	max_threshold = 360*40*8;
    } else if (shaper_rate < 4000000) {
	max_threshold = 360*40*8*2;
    } else if (shaper_rate < 6000000) {
	max_threshold = 360*40*8*3;
    } else if (shaper_rate < 8000000) {
	max_threshold = 360*40*8*4;
    } else if (shaper_rate < 10000000) {
	max_threshold = 360*40*8*5;
    } else {
	max_threshold = 360*40*8*6;
    }
    
    if (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	if (kbits_sec_max == 0) {
	    enable = FALSE;
	} else {
	    if (bits_thresh_max == 0) {
		/* threshold not specified when rate is non-zero,
		 * SDK calculate the default threshold based on rate
		 * set the bitbucket to be 0x3FFF (14 bits)
		 */
		bits_thresh_max = default_threshold;
	    }
	    enable = TRUE;
	}
    } else {
	enable = SOC_SIRIUS_API_PARAM_NO_CHANGE;
    }

    if ((bits_thresh_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) &&
	(bits_thresh_max > max_threshold)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, For egress shaper with rate %d kbps, burst size is required to be "
	                       "smaller than %d kbits, Unit(%d)\n"),
	           FUNCTION_NAME(), shaper_rate, max_threshold/1000, unit));
	return BCM_E_PARAM;
    }

    rv = soc_sirius_es_node_shaper_config(unit, level, node, FALSE, enable,
					  kbits_sec_max, bits_thresh_max, tick_select);

    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to config max egress shaper for level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return rv;
    }

    /* update the state */
    if (level == SIRIUS_ES_LEVEL_GROUP_SHAPER) {
	if (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    gs_state[unit][node].max_shaper_rate_kbps = kbits_sec_max;
	}
	
	if (bits_thresh_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    gs_state[unit][node].max_shaper_threshold_bits = bits_thresh_max;
	}
    } else {
	if (kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    es_state[unit][level][node].min_shaper_rate_kbps = kbits_sec_min;
	}
	
	if (bits_thresh_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    es_state[unit][level][node].min_shaper_threshold_bits = bits_thresh_min;
	}
	
	if (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    es_state[unit][level][node].max_shaper_rate_kbps = kbits_sec_max;
	}
	
	if (bits_thresh_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    es_state[unit][level][node].max_shaper_threshold_bits = bits_thresh_max;
	}
    }

    return rv;
}

int
bcm_sirius_cosq_get_egress_shaper_params(int unit,
					 int level,
					 int node,
					 int *kbits_sec_min,
					 int *bits_thresh_min,
					 int *kbits_sec_max,
					 int *bits_thresh_max)
{
    int rv = BCM_E_NONE;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (level == SIRIUS_ES_LEVEL_GROUP_SHAPER) {
	if ((gs_state[unit] == NULL) || (node < 0) || (node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER)
	    || (gs_state[unit][node].in_use != TRUE)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress group max shaper node %d not in use, Unit(%d)\n"),
	               FUNCTION_NAME(), node, unit));
	    return BCM_E_PARAM;
	}
    } else {
	if ((es_state[unit][level] == NULL) || (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
	    (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]) ||
	    (es_state[unit][level][node].in_use != TRUE)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress scheduler level %d node %d not in use, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));
	    return BCM_E_PARAM;
	}
    }

    if (level == SIRIUS_ES_LEVEL_GROUP_SHAPER) {
	if (kbits_sec_max != NULL) {
	    *kbits_sec_max = gs_state[unit][node].max_shaper_rate_kbps;
	}

	if (bits_thresh_max != NULL) {
	    *bits_thresh_max = gs_state[unit][node].max_shaper_threshold_bits;
	}
    } else {	
	if (kbits_sec_min != NULL) {
	    *kbits_sec_min = es_state[unit][level][node].min_shaper_rate_kbps;
	}
	
	if (bits_thresh_min != NULL) {
	    *bits_thresh_min = es_state[unit][level][node].min_shaper_threshold_bits;
	}
	
	if (kbits_sec_max != NULL) {
	    *kbits_sec_max = es_state[unit][level][node].max_shaper_rate_kbps;
	}
	
	if (bits_thresh_max != NULL) {
	    *bits_thresh_max = es_state[unit][level][node].max_shaper_threshold_bits;
	}
    }

    return rv;
}

int
bcm_sirius_cosq_set_ingress_scheduler_params(int unit,
					     int level,
					     int node,
					     int mode,
					     int weight)
{
    int rv = BCM_E_NONE;
    int node_profile = 0, parent, first_child, num_child, child_offset, grandparent;
    int node_type, node_type_parent, bucket_type, bucket_type_parent, bucket;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_i = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_p = NULL;
    soc_sbx_sirius_config_t *sir  = SOC_SBX_CFG_SIRIUS(unit);

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    /* NOTE: bucket_type could also be updated by the nexus API */
    if ( (level <= 0) || (level >= SIRIUS_TS_LEVEL_7) || 
	 (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not supported, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    /* read software state */
    s_i = &is_state[unit][level][node];
    if (s_i->in_use != TRUE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    if ((s_i->parent < 0) || (s_i->parent >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level+1])) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not attached, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }
    
    s_p = &is_state[unit][level+1][s_i->parent];
    if (s_p->in_use != TRUE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(), level+1, s_i->parent, unit));
	return BCM_E_PARAM;
    }

    if ( (s_p->num_child <= 0) || (s_p->first_child < 0) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d state not consistent, Unit(%d)\n"),
	           FUNCTION_NAME(), level+1, s_i->parent, unit));
	return BCM_E_INTERNAL;
    }

    /* read hardware state in N2 table */
    rv = soc_sirius_ts_node_hierachy_config_get(unit, level, node, &parent,
						&first_child, &num_child);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));	    
	return rv;
    }
    
    /* make sure both matches */
    if ( (parent != s_i->parent) || ((level > 1) && (s_i->first_child > 0) && (first_child != s_i->first_child)) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d hierachy state inconsistent, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));	    
	return BCM_E_INTERNAL;
    }

    rv = soc_sirius_ts_node_hierachy_config_get(unit, level+1, parent, &grandparent,
						&first_child, &num_child);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
	           FUNCTION_NAME(), level+1, parent, unit));	    
	return rv;
    }
    
    /* make sure both matches */
    if ( (num_child <= 0) || (first_child != s_p->first_child) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d hierachy state inconsistent, Unit(%d)\n"),
	           FUNCTION_NAME(), level+1, parent, unit));	    
	return BCM_E_INTERNAL;
    }
    
    /* update the right child weight based on the first_child and child node id */
    child_offset = node - first_child;
    if ( (child_offset < 0) || (child_offset >= num_child) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d hierachy inconsistent, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));	    
	return BCM_E_PARAM;
    }
    
    /* default node_type, bucket_type and bucket_table_ptr */
    node_type = SIRIUS_TS_NODE_TYPE_DEFAULT;
    bucket_type = SIRIUS_TS_BUCKET_TYPE_BUCKET;
    bucket = node;

    if (mode == BCM_COSQ_NONE) {
	/* check how many children its parent has, if only 1 and child passthrough disable is set
     * default as passthrough node, otherwise assume root_relay for flat scheduler
	 */
        if ((s_p->num_child == 1) && (SOC_SBX_CFG_SIRIUS(unit)->tsChildPassThroughDisable != 0)) {
	    /* pass-through node */
	    
	} else {
	    /* part of flat-scheduler */

	    /* update the node_type for itself, default to LEAF_RELAY assuming the API
	     * for BCM_COSQ_NONE will be done topdown. if parent's mode is not BCM_COSQ_NONE,
	     * then parent's node_type = ROOT_RELAY, otherwise, parent's node_type = INNER_RELAY
	     *     LEAF_RELAY and INNER_RELAY must to be SUBTREE bucket_type
	     */
	    if ((sir->bSubscriberNodeOptimize == TRUE) && 
		(sir->uSubscriberMaxCos == 8) && 
		(sir->b8kNodes == FALSE) && 
		(level == 2)) {

		rv = soc_sirius_ts_node_hierachy_config_get(unit, level, node, &parent,
							    &first_child, &num_child);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, unit));	    
		    return rv;
		}

		if (is_state[unit][1][first_child].logical_scheduler < 0) {
		    /* when level 1 node is skipped, it has no logical scheduler associate with it */
		    node_type = SIRIUS_TS_NODE_TYPE_INNER_RELAY;
		} else {
		    node_type = SIRIUS_TS_NODE_TYPE_LEAF_RELAY;
		}
	    } else {
		node_type = SIRIUS_TS_NODE_TYPE_LEAF_RELAY;
	    }
	    bucket_type = SIRIUS_TS_BUCKET_TYPE_SUBTREE;
	    if (s_p->scheduler_mode == BCM_COSQ_NONE) {
		node_type_parent = SIRIUS_TS_NODE_TYPE_INNER_RELAY;
		bucket_type_parent = SIRIUS_TS_BUCKET_TYPE_SUBTREE;
	    } else {
		node_type_parent = SIRIUS_TS_NODE_TYPE_ROOT_RELAY;
		bucket_type_parent = SIRIUS_TS_BUCKET_TYPE_BUCKET;
	    }
	    
	    /* update parent node mapping */
	    rv = soc_sirius_ts_node_mapping_config(unit, level+1, s_i->parent,
						   node_type_parent, bucket_type_parent,
						   SOC_SIRIUS_API_PARAM_NO_CHANGE);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fail to config ingress scheduler level %d node %d bucket info, Unit(%d)\n"),
		           FUNCTION_NAME(), level+1, s_i->parent, unit));
		return rv;
	    } else {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s config TS level %d node %d to node_type %d bucket_type %d, Unit(%d)\n"),
		             FUNCTION_NAME(), level+1, s_i->parent,
		             node_type_parent, bucket_type_parent, unit));
	    }

	    /* select a bucket for the node as part of flat scheduler */
	    rv = _bcm_sirius_cosq_ingress_flat_scheduler_bucket_select(unit, level, node, &bucket);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fail to select a bucket for ingress flat scheduler level %d node %d, Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, unit));
		return rv;
	    }
	}
    } else if ( ((mode == BCM_COSQ_AF0) ||
		(mode == BCM_COSQ_AF1) ||
		(mode == BCM_COSQ_AF2) ||
		(mode == BCM_COSQ_AF3) ||
		(mode == BCM_COSQ_AF4) ||
		(mode == BCM_COSQ_AF5) ||
		(mode == BCM_COSQ_AF6)) ||
               (((mode == BCM_COSQ_SP0) ||
		(mode == BCM_COSQ_SP1) ||
		(mode == BCM_COSQ_SP2) ||
		(mode == BCM_COSQ_SP3) ||
		(mode == BCM_COSQ_SP4) ||
		(mode == BCM_COSQ_SP5) ||
		(mode == BCM_COSQ_SP6)) &&
                (is_state[unit][level][node].min_shaper_rate_kbps != 0)) ) {
	/* update child weight in parent node in both regular scheduler (parent)
	 * flat scheduler case (parent leaf-relay node)
	 */
	rv = soc_sirius_ts_node_child_weight_config(unit, level+1, parent, child_offset, weight);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, fail to config ingress scheduler level %d node %d weight, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));	    
	    return rv;
	} else {
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s config TS level %d node %d child offset %d to weight %d, Unit(%d)\n"),
	                 FUNCTION_NAME(), level+1, parent, child_offset, weight, unit));
	}

	if ((sir->bSubscriberNodeOptimize == TRUE) && 
	    (sir->uSubscriberMaxCos == 8) && 
	    (sir->b8kNodes == FALSE) && 
	    (level == 2)) {

	    rv = soc_sirius_ts_node_hierachy_config_get(unit, level, node, &parent,
							&first_child, &num_child);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, fail to get ingress scheduler level %d node %d hierachy state, Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, unit));	    
		return rv;
	    }

	    if (is_state[unit][1][first_child].logical_scheduler < 0) {
		/* when level 1 node is skipped, it has no logical scheduler associate with it */
		node_type = SIRIUS_TS_NODE_TYPE_ROOT_RELAY;
	    }
	}
    }

    /* config bucket info of this node */
    rv = soc_sirius_ts_node_mapping_config(unit, level, node, node_type, bucket_type, bucket);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fail to config ingress scheduler level %d node %d bucket info, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));	    
	return rv;
    } else {
	is_state[unit][level][node].bucket = bucket;
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s config ingress scheduler level %d node %d node_type %d bucket_type %d, Unit(%d)\n"),
	             FUNCTION_NAME(), level, node, node_type, bucket_type, unit));
    }

    /* init state of this node:
     * caused too much traffic glitch when change parameters, disable it   
     * rv = soc_sirius_ts_node_state_init(unit, level, node);
     * if (rv != SOC_E_NONE) {
     *     LOG_ERROR(BSL_LS_BCM_COSQ,
     *               (BSL_META_U(unit,
     *                           "ERROR: %s, fail to init ingress scheduler level %d node %d state, Unit(%d)\n"),
     *                *	       FUNCTION_NAME(), level, node, unit));	    
     *	   return rv;
     * }
     */

    /* select Node profile based on the sched mode */
    if (is_state[unit][level][node].adopt_all == TRUE) {
        node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT;
    }
    else {
        rv = bcm_sirius_cosq_ingress_scheduler_node_profile_get(unit, level, node, mode, &node_profile);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, in getting ingress scheduler node profile (0x%x)\n"),
                       FUNCTION_NAME(), rv));
            return(rv);
        }
    }

    rv = soc_sirius_ts_node_creditor_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					    SOC_SIRIUS_API_PARAM_NO_CHANGE, node_profile);
    
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to config ingress creditor node profile for level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return rv;
    } else {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s config TS level %d node %d use node_profile %d, Unit(%d)\n"),
	             FUNCTION_NAME(), level, node, node_profile, unit));
    }

    /* update state */
    is_state[unit][level][node].scheduler_mode = mode;
    is_state[unit][level][node].scheduler_param = weight;

    return rv;
}

int
bcm_sirius_cosq_get_ingress_scheduler_params(int unit,
					     int level,
					     int node,
					     int *mode,
					     int *weight)
{
    int rv = BCM_E_NONE;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if ( (level <= 0) || (level > SIRIUS_TS_LEVEL_7) || 
	 (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not supported, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    if ((is_state[unit][level] == NULL) || (is_state[unit][level][node].in_use != TRUE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    *mode = is_state[unit][level][node].scheduler_mode;
    *weight = is_state[unit][level][node].scheduler_param;

    return rv;
}

int
bcm_sirius_cosq_get_ingress_scheduler(int unit,
				      int level,
				      int node,
				      bcm_sbx_sirius_ingress_scheduler_state_t *p_is) 
{
    int rv = BCM_E_NONE;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if ( (level <= 0) || (level > SIRIUS_TS_LEVEL_7) || 
	 (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not supported, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    if ((is_state[unit][level] == NULL) || (is_state[unit][level][node].in_use != TRUE)) {
      return BCM_E_EMPTY;
    }

    *p_is = is_state[unit][level][node];

    return rv;
}



int 
bcm_sirius_ingress_shaper_params_validate(int unit, 
					       int level, 
					       int kbits_sec_max)
{
    int rv = BCM_E_NONE;
    int last_node = 0, leak_cycles = 0;
    uint32 visit_period = 0, full_bucket = 0;
    uint32 regval = 0;

    switch(level) {
	case 1:
	    BCM_IF_ERROR_RETURN(READ_TS_LEVEL1_CONFIG0r(unit, &regval));
	    leak_cycles = soc_reg_field_get(unit, TS_LEVEL1_CONFIG0r, regval, LEAK_CYCLESf);
	    last_node = soc_reg_field_get(unit, TS_LEVEL1_CONFIG0r, regval, LAST_NODEf);
	    break;
	case 2:
	    BCM_IF_ERROR_RETURN(READ_TS_LEVEL2_CONFIG0r(unit, &regval));
	    leak_cycles = soc_reg_field_get(unit, TS_LEVEL2_CONFIG0r, regval, LEAK_CYCLESf);
	    last_node = soc_reg_field_get(unit, TS_LEVEL2_CONFIG0r, regval, LAST_NODEf);
	    break;
	case 3:
	    BCM_IF_ERROR_RETURN(READ_TS_LEVEL3_CONFIG0r(unit, &regval));
	    leak_cycles = soc_reg_field_get(unit, TS_LEVEL3_CONFIG0r, regval, LEAK_CYCLESf);
	    last_node = soc_reg_field_get(unit, TS_LEVEL3_CONFIG0r, regval, LAST_NODEf);
	    break;
	case 4:
	    BCM_IF_ERROR_RETURN(READ_TS_LEVEL4_CONFIG0r(unit, &regval));
	    leak_cycles = soc_reg_field_get(unit, TS_LEVEL4_CONFIG0r, regval, LEAK_CYCLESf);
	    last_node = soc_reg_field_get(unit, TS_LEVEL4_CONFIG0r, regval, LAST_NODEf);
	    break;
	default:
	    return BCM_E_NONE;
    }

    /* uClockSpeed : 405 */
    visit_period = (last_node * (leak_cycles + 16)) / SOC_SBX_CFG(unit)->uClockSpeedInMHz;

    if (visit_period == 0) {
	return BCM_E_NONE;
    }

    /* Full Bucket calculated to 10% of how quickly the bucket can fill
     * based on h/w parameters. That makes the below calculations of 40% and 70% 
     * easier */

    full_bucket = 100 * (1024 * 1024 - 1) / visit_period;

    /* If the desired kbits_sec_rate would fill more than 70% of the bucket in a 
     * single pass, then do not permit. If the kbits_sec_rate falls between 40% and 70%
     * then warn of precision loss */

    if (kbits_sec_max > (full_bucket * 7)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s Level %d rate accuracy comprimised using Last Node %d and Leak Cycles %d, unit %d\n"),
	           FUNCTION_NAME(),level, last_node, leak_cycles, unit));
	rv = BCM_E_FAIL;
    } else if (kbits_sec_max > (full_bucket * 4)) {
	LOG_WARN(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "WARNING: %s Level %d rate precision may be affected using Last Node %d and Leak Cycles %d, unit %d\n"),
	          FUNCTION_NAME(),level, last_node, leak_cycles, unit));
    }
    
    return rv;
}


int
bcm_sirius_cosq_set_ingress_shaper_params(int unit,
					  int level,
					  int node,
					  int kbits_sec_min,
					  int bits_thresh_min,
					  int kbits_sec_max,
					  int bits_thresh_max)
{
    int rv = BCM_E_NONE;
    int enable = FALSE, cir_enable = FALSE;
    int default_threshold = 256000; /* default to 32KB burst */
    int node_profile;
    int old_min_shaper_rate_kbps;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if ( (level <= 0) || (level > SIRIUS_TS_LEVEL_7) || 
	 (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not supported, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    if ((is_state[unit][level] == NULL) || (is_state[unit][level][node].in_use != TRUE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    /* min shaper (CIR rate), thresholds (node profile) is fixed */
    if (kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	if (kbits_sec_min == 0) {
	    enable = cir_enable = FALSE;
	} else {
	    enable = cir_enable = TRUE;
            if (kbits_sec_min == BCM_SBX_COSQ_RCPQ_CIR_EQUAL_ZERO) {
                if ((is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP0) ||
	               (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP1) ||
	               (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP2) ||
	               (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP3) ||
	               (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP4) ||
	               (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP5) ||
	               (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP6)) {
	            cir_enable = FALSE;
                }
            }
	}
    } else {
	enable = cir_enable = SOC_SIRIUS_API_PARAM_NO_CHANGE;
    }
    
    rv = bcm_sirius_ingress_shaper_params_validate(unit, level, kbits_sec_max);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Shaper Accuracy and Precision severely affected, Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_PARAM;
    }

    rv = soc_sirius_ts_node_creditor_config(unit, level, node, cir_enable,
					    kbits_sec_min, SOC_SIRIUS_API_PARAM_NO_CHANGE);
    
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to config ingress creditor for level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return rv;
    } else {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s config ingress creditor level %d node %d, Unit(%d)\n"),
	             FUNCTION_NAME(), level, node, unit));
    }

    /* if min shaper enabled and SP0/1, switch between SP and RCPQ node profile */
    if ((is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP0) ||
	(is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP1) ||
	(is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP2) ||
	(is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP3) ||
	(is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP4) ||
	(is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP5) ||
	(is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP6)) {

        /* may require re-configuring weights */
        old_min_shaper_rate_kbps = is_state[unit][level][node].min_shaper_rate_kbps;
        if (kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	    is_state[unit][level][node].min_shaper_rate_kbps = kbits_sec_min;
        }
    
        /* check if CIR has been setup */
        rv = bcm_sirius_cosq_set_ingress_scheduler_params(unit, level, node,
					     is_state[unit][level][node].scheduler_mode,
					     is_state[unit][level][node].scheduler_param);
        if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to config ingress params for level %d node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));
	    is_state[unit][level][node].min_shaper_rate_kbps = old_min_shaper_rate_kbps;
            return(rv);
        }

	if (enable == TRUE) {
	    if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP0) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0;
	    }
	    else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP1) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1;
	    }
	    else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP2) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP2;
	    }
	    else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP3) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP3;
	    }
	    else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP4) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP4;
	    }
	    else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP5) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP5;
	    }
            else {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP6;
	    }
	} else {
	    if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP0) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP0;
	    } else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP1) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP1;
	    } else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP2) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP2;
	    } else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP3) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP3;
	    } else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP4) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP4;
	    } else if (is_state[unit][level][node].scheduler_mode == BCM_COSQ_SP5) {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP5;
	    } else {
		node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP6;
	    }
	}
	rv = soc_sirius_ts_node_creditor_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						SOC_SIRIUS_API_PARAM_NO_CHANGE, node_profile);
	
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to config ingress creditor node profile for level %d node %d, Unit(%d)\n"),
	               FUNCTION_NAME(), level, node, unit));
	    return rv;
	} else {
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s config TS level %d node %d use node_profile %d, Unit(%d)\n"),
	                 FUNCTION_NAME(), level, node, node_profile, unit));
	}
    }

    /* max shaper (PIR rate) */
    if (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	if (kbits_sec_max == 0) {
	    enable = FALSE;
	} else {
	    if (bits_thresh_max == 0) {
		/* threshold not specified when rate is non-zero,
		 * SDK calculate the default threshold is 32kbits
		 */
		bits_thresh_max = default_threshold;
	    }
	    enable = TRUE;
	}
    } else {
	enable = SOC_SIRIUS_API_PARAM_NO_CHANGE;
    }

    rv = soc_sirius_ts_node_shaper_config(unit, level, node, enable,
					  kbits_sec_max, bits_thresh_max);

    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to config max ingress shaper for level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return rv;
    } else {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s config ingress shaper level %d node %d, Unit(%d)\n"),
	             FUNCTION_NAME(), level, node, unit));
    }

    /* update the state */
    if (kbits_sec_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	is_state[unit][level][node].min_shaper_rate_kbps = kbits_sec_min;
    }
    
    if (bits_thresh_min != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	is_state[unit][level][node].min_shaper_threshold_bits = bits_thresh_min;
    }
    
    if (kbits_sec_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	is_state[unit][level][node].max_shaper_rate_kbps = kbits_sec_max;
    }
    
    if (bits_thresh_max != SOC_SIRIUS_API_PARAM_NO_CHANGE) {
	is_state[unit][level][node].max_shaper_threshold_bits = bits_thresh_max;
    }

    return rv;
}

int
bcm_sirius_cosq_get_ingress_shaper_params(int unit,
					  int level,
					  int node,
					  int *kbits_sec_min,
					  int *bits_thresh_min,
					  int *kbits_sec_max,
					  int *bits_thresh_max)
{
    int rv = BCM_E_NONE;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if ( (level <= 0) || (level > SIRIUS_TS_LEVEL_7) || 
	 (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not supported, Unit(%d)\n"),
	           FUNCTION_NAME(), level, node, unit));
	return BCM_E_PARAM;
    }

    if ((is_state[unit][level] == NULL) || (is_state[unit][level][node].in_use != TRUE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress scheduler level %d node %d not in use, Unit(%d)\n"),
	           FUNCTION_NAME(),
	           level, node, unit));
	return BCM_E_PARAM;
    }
    
    if (kbits_sec_min != NULL) {
	*kbits_sec_min = is_state[unit][level][node].min_shaper_rate_kbps;
    }
    
    if (bits_thresh_min != NULL) {
	*bits_thresh_min = is_state[unit][level][node].min_shaper_threshold_bits;
    }
    
    if (kbits_sec_max != NULL) {
	*kbits_sec_max = is_state[unit][level][node].max_shaper_rate_kbps;
    }
    
    if (bits_thresh_max != NULL) {
	*bits_thresh_max = is_state[unit][level][node].max_shaper_threshold_bits;
    }

    return rv;
}

int
bcm_sirius_cosq_control_adjust(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_control_t type,
                            int *value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int result;

    switch (type) {
    case bcmCosqControlFabricConnectMinUtilization:
        result = soc_sirius_template_min_util_adjust(unit, *value, value);
        break;
    case bcmCosqControlFabricConnectMaxTime:
        result = soc_sirius_template_max_age_adjust(unit, *value, value);
        break;
    default:
        result = BCM_E_PARAM;
    }
    return result;
}

int
bcm_sirius_cosq_control_set(int unit,
			    bcm_gport_t gport,
			    bcm_cos_queue_t cosq,
			    bcm_cosq_control_t type,
			    int value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int level = 0, node = 0;
    int q_type = 0, hold_ts=0;
    int base_queue=0, num_cos=0, subport = 0, eg_n = 0, num_fifos = 0;
    bcm_sbx_cosq_queue_state_t *q_state = NULL;
    uint32 bytes_min[2], bytes_max[2], method = 0, uRegValue = 0;
    uint32 thresh0 = 0, thresh1 = 0, fifo = 0, old_fifo = 0;
    int hwVal, template, fcd = -1, ds_id = 0;
    int is_deallocated;
    int local, i;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_i_state = NULL;
    fifo_map_table_entry_t fifo_entry;
    eg_fd_fifo_thresh_entry_t fifo_thresh;
    eg_fd_fifo_thresh_offset_red_entry_t thresh_offset_red;
    eg_fd_fifo_thresh_offset_yellow_entry_t thresh_offset_yellow;
    eg_fd_fifo_thresh_reset_offset_entry_t thresh_reset_offset;
    int handle, port = -1;
    int mymodid = 0, modid = 0;
    bcm_trunk_t tid;
    int fc_state_index = 0;
    flow_control_state_table_entry_t       fc_state_table_entry;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;
    int level0_node = 0;
    int queue_type, end_queue, start_queue, queue;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    switch (type) {
	case bcmCosqControlBandwidthBurstMax:
	    if (BCM_GPORT_IS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport) ||
		BCM_GPORT_IS_SCHEDULER(gport)) {

		if ((BCM_GPORT_IS_CHILD(gport) && (soc_feature(unit, soc_feature_standalone) || soc_feature(unit, soc_feature_hybrid))) || 
		    (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport))) {
		    
		    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		    if (rv != BCM_E_NONE) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "gport control set, unsupported gport on unit %d\n"),
			          unit));
			rv = BCM_E_PARAM;
			return rv;
		    }

		    /* value is in kbits */
		    rv = bcm_sirius_cosq_set_ingress_shaper_params(unit,
					      level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					      SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					      value * 1000);
		    
		    if (rv != BCM_E_NONE) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "ERROR: %s BandwidthBurstMax set failed, Unit(%d)\n"),
			          FUNCTION_NAME(), unit));
		    }
		    return rv;
		}

		if ( BCM_GPORT_IS_CHILD(gport) &&
		     !soc_feature(unit, soc_feature_standalone) &&
		     !soc_feature(unit, soc_feature_hybrid) ) {
		    /* convert it to egress child port so that it will resolve to egress scheduler */
		    BCM_GPORT_EGRESS_CHILD_SET(gport, BCM_GPORT_CHILD_MODID_GET(gport), BCM_GPORT_CHILD_PORT_GET(gport));
		}

		/* this will make sure we return group shaper for child/egress child gport if trunked */
		level = SIRIUS_ES_LEVEL_GROUP_SHAPER;
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport control set, unsupported gport on unit %d\n"),
		              unit));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( (level != SIRIUS_ES_LEVEL_GROUP_SHAPER) && 
		     ((level < 0) || (node < 0)) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
		               gport));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
		    subport = -1;
		    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
			           FUNCTION_NAME(), subport, gport, unit));
			return BCM_E_PARAM;
		    }
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		    if (cosq < 0) {
			/* point to the parent level, node is in the handle */
			level++;
			node = sp_info->es_scheduler_level1_node[eg_n];
		    } else if (cosq < num_fifos) {
			node += cosq;
		    } else {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
			           cosq, num_fifos));
			rv = BCM_E_PARAM;
			return rv;	
		    }
		}

		/* value is in kbits */
		rv = bcm_sirius_cosq_set_egress_shaper_params(unit,
					  level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					  SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					  value * 1000);

		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s BandwidthBurstMax set failed, Unit(%d)\n"),
		              FUNCTION_NAME(), unit));
		}
	    } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
                       BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
                       BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
                       BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
                rv = _bcm_sirius_cosq_ingress_shaper_burst_set(unit, control_info->u.conn.queue, value);
 
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;
	case bcmCosqControlBandwidthBurstMin:
	    if (BCM_GPORT_IS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport) ||
		BCM_GPORT_IS_SCHEDULER(gport)) {

		if ((BCM_GPORT_IS_CHILD(gport) && (soc_feature(unit, soc_feature_standalone) || soc_feature(unit, soc_feature_hybrid))) || 
		    (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport))) {		    
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s BandwidthBurstMin not supported on ingress scheduler, Unit(%d)\n"),
		              FUNCTION_NAME(), unit));
		    rv = BCM_E_UNAVAIL;
		    return rv;
		}

		if ( BCM_GPORT_IS_CHILD(gport) &&
		     !soc_feature(unit, soc_feature_standalone) &&
		     !soc_feature(unit, soc_feature_hybrid) ) {
		    /* convert it to egress child port so that it will resolve to egress scheduler */
		    BCM_GPORT_EGRESS_CHILD_SET(gport, BCM_GPORT_CHILD_MODID_GET(gport), BCM_GPORT_CHILD_PORT_GET(gport));
		}

		/* this will make sure we return group shaper for child/egress child gport if trunked */
		level = SIRIUS_ES_LEVEL_GROUP_SHAPER;
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport control set, unsupported gport on unit %d\n"),
		              unit));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( (level != SIRIUS_ES_LEVEL_GROUP_SHAPER) && 
		     ((level < 0) || (node < 0)) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
		               gport));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
		    subport = -1;
		    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
			           FUNCTION_NAME(), subport, gport, unit));
			return BCM_E_PARAM;
		    }
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		    if (cosq < 0) {
			/* point to the parent level, node is in the handle */
			level++;
			node = sp_info->es_scheduler_level1_node[eg_n];
		    } else if (cosq < num_fifos) {
			node += cosq;
		    } else {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
			           cosq, num_fifos));
			rv = BCM_E_PARAM;
			return rv;	
		    }
		}

		/* value is in kbits */
		rv = bcm_sirius_cosq_set_egress_shaper_params(unit,
					  level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					  value * 1000, SOC_SIRIUS_API_PARAM_NO_CHANGE,
					  SOC_SIRIUS_API_PARAM_NO_CHANGE);

		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s BandwidthBurstMax set failed, Unit(%d)\n"),
		              FUNCTION_NAME(), unit));
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;
        case bcmCosqControlFabricConnectMinUtilization:
            /* rip the bits we want from API's 'fractional' representation */
            value = BCM_COSQ_CONTROL_FABRIC_CONNECT_MIN_UTILIZATION_WHOLE_GET(value);
            /* adjust the value to hardware representation */
            rv = soc_sirius_template_min_util_adjust(unit,
                                                     value,
                                                     &hwVal);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* get current template for this queue */
            rv = soc_sirius_queue_min_util_get(unit,
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
                rv = soc_sirius_template_min_util_set(unit,
                                                      control_info->u.conn.template,
                                                      hwVal);
            }

            if (SOC_E_NONE == rv) {
                /* set up new queue configuration */
                rv = soc_sirius_queue_min_util_set(unit,
                                                   control_info->u.conn.queue,
                                                   control_info->u.conn.template);
            } /* if (SOC_E_NONE == rv) */

            if (SOC_E_NONE == rv) {
                base_queue = control_info->u.conn.queue;
                q_state = (bcm_sbx_cosq_queue_state_t *)SOC_SBX_STATE(unit)->queue_state;
                q_state[base_queue].ingress.anemic_watermark_select = control_info->u.conn.template;
                /* update good; free the old template instance */
                rv = soc_sbx_connect_min_util_dealloc(unit,
                                                      0,
                                                      local,
                                                      &is_deallocated,
                                                      &template);
            } else { /* if (BCM_E_NONE == rv) */
                /* something went wrong; free newer template instance */
                rv = soc_sbx_connect_min_util_dealloc(unit,
                                                      0,
                                                      hwVal,
                                                      &is_deallocated,
                                                      &(control_info->u.conn.template));
                /* we don't return this call's result; prior err has preced. */
            } /* if (BCM_E_NONE == rv) */
            break;

        case bcmCosqControlFabricConnectMaxTime:
            /* adjust the value to hardware representation */
            rv = soc_sirius_template_max_age_adjust(unit,
                                                    value,
                                                    &hwVal);
            if (SOC_E_NONE != rv) {
                return rv;
            }

            /* get current template for this queue */
            /* coverity [callee_ptr_arith] */
            rv = soc_sirius_queue_max_age_get(unit,
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
                rv = soc_sirius_template_max_age_set(unit,
                                                     control_info->u.conn.template,
                                                     hwVal);
            }

            /* change queue configuration */
            if (SOC_E_NONE == rv) {
                rv = soc_sirius_queue_max_age_set(unit,
                                                  control_info->u.conn.queue,
                                                  control_info->u.conn.template);
            }

            if (SOC_E_NONE == rv) {
                /* update queue state to reflect new values */
                base_queue = control_info->u.conn.queue;
                q_state = (bcm_sbx_cosq_queue_state_t *)SOC_SBX_STATE(unit)->queue_state;
                q_state[base_queue].ingress.anemic_age_threshold_key = control_info->u.conn.template;
                /* free the old template instance */
                rv = soc_sbx_connect_max_age_dealloc(unit,
                                                     0,
                                                     local,
                                                     &is_deallocated,
                                                     &template);
            } else { /* if (SOC_E_NONE == rv) */
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
	    if (value > 16) {
		value = 16;
	    }
	    value--;
	    BCM_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
	    q_state =  &(SOC_SBX_STATE(unit)->queue_state[base_queue+cosq]);
	    BCM_IF_ERROR_RETURN(soc_sirius_qs_queue_parameter_get(unit, base_queue+cosq, &q_type, &hold_ts));
	    BCM_IF_ERROR_RETURN(soc_sirius_qs_queue_parameter_set(unit, base_queue+cosq, q_type, value));
	    q_state->ingress.hold_pri_num_timeslots = value;
            break;

        case bcmCosqControlPacketLengthAdjust:
	    rv = BCM_E_UNAVAIL;
	    break;

	case bcmCosqControlFabricPortScheduler:
	    /* map the egress group gport to the specified fabric port */
	    if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
		/* If not egress node, then nothing to do */
		BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));
		if (BCM_GPORT_EGRESS_GROUP_MODID_GET(gport) != mymodid) {	 
		    return BCM_E_NONE;
		} 
		
		/* map egress group to a fabric port */
		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		/* value is a child gport handle */
		if (BCM_GPORT_IS_CHILD(value)) {
		    handle = BCM_GPORT_CHILD_PORT_GET(value);
		} else if (BCM_GPORT_IS_EGRESS_CHILD(value)) {
		    handle = BCM_GPORT_EGRESS_CHILD_PORT_GET(value);
		} else {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport control set, unsupported gport type for value 0x%x on unit %d\n"),
		              value, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}

		if (handle >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, gport 0x%x uses invalid handle %d, unit %d\n"),
		               FUNCTION_NAME(), gport, handle, unit));
		    return BCM_E_PARAM;			
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[handle]);

		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}

		if (subport != handle) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d used at egress group creation does not match what's specified %d here, unit %d\n"),
		               FUNCTION_NAME(), subport, handle, unit));
		    return BCM_E_PARAM;			
		}

		if (((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) == 0) && 
		    (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP) != 0)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, es_fabric_port_hierachy_set was set to 1, egress schedulers are hooked"
		                           " up by SDK already, this API is not supported in this mode, unit %d\n"),
		               FUNCTION_NAME(), unit));
		    return BCM_E_UNAVAIL;
		}

		node = sp_info->egroup[eg_n].es_scheduler_level0_node;
		rv = soc_sirius_es_node_get_info(unit, SIRIUS_ES_LEVEL_FIFO, node,
						 &subport, &num_cos);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to get info for fifo level egress node %d, unit %d\n"),
		               FUNCTION_NAME(), node, unit));	
		    return rv;
		}

		sp_info->es_scheduler_level2_node = es_state[unit][SIRIUS_ES_LEVEL_FIFO][node].channel;
		sp_info->es_scheduler_level1_node[eg_n] = subport;
		sp_info->flags |= SBX_SUBPORT_FLAG_ON_ES;

		/* map flow control domain to the fifos used by the egress group, for now
		 * support tme/sporster mode only, also we are forcing the fcd/eg_group id/child port id
		 * to be same in this case.
		 */
        
		/* Tie multicast trunk gport to internal trunking structure */
		if (sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) {
		    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                if (BCM_GPORT_IS_MODPORT(sp_info->parent_gport)) {
                    port = BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport);
                } else if (BCM_GPORT_IS_EGRESS_MODPORT(sp_info->parent_gport)) {
                    port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(sp_info->parent_gport);
                }
                if ((port < 0) || (port > 3)) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: %s, gport 0x%x not connected to hgig, incorrect port %d\n"),
                               FUNCTION_NAME(), gport, unit));
                    return BCM_E_INTERNAL;
                }
                rv = _bcm_sirius_aggregate_data_map_internal_port(unit, port, handle);
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: %s, FAILED to create internal higig ports, unit %d\n"),
                               FUNCTION_NAME(), unit));
                    return rv;
                } else {
                    /* For admission control, block the related fifo on the other set of fifos for
                     * multicast ef and non-ef */
                    if ((SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo == SB_FAB_XCORE_COS_FIFO_MULTICAST_NEF) &&
                        (SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo == SB_FAB_XCORE_COS_FIFO_MULTICAST_EF)) {
                        if (num_fifos != SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE) {
                            LOG_WARN(BSL_LS_BCM_COSQ,
                                     (BSL_META_U(unit,
                                                 "WARNING: %s, Multicast defaults unchanged, Admission control settings ignored, unit %d\n"),
                                      FUNCTION_NAME(), unit));
                        }
                        /* Mcast FIFO Scheduler Mode is Strict Pri (Mni/Max SP = 1, EFf = 0) */
                        for (i = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0;
                             i < SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0 + SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; i++) {
                            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[i]);
                            level0_node = sp_info->egroup[eg_n].es_scheduler_level0_node;
                            for (fifo = level0_node; fifo < level0_node + sp_info->egroup[eg_n].num_fifos; fifo++) {
                                SOC_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, fifo , &fifo_entry));
                                /* mode == SIRIUS_ES_SCHEDULER_MODE_SP */
                                soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MIN_SPf, 1);
                                soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MAX_SPf, 1);
                                soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, EFf, 0);
                                SOC_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, fifo, &fifo_entry));
                            }
                        }
                    } else {
                        for (i = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0; 
                             i < SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0 + SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; i++) {
                            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[i]);
                            sal_memset(&fifo_thresh, 0, sizeof(eg_fd_fifo_thresh_entry_t));
                            /* NON EF - currently supporting 8 fifos */
                            thresh0 = SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo/4;
                            if (thresh0 == 1) {
                                fifo = sp_info->egroup[0].es_scheduler_level0_node + SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo%4;
                            } else {
                                fifo = sp_info->egroup[0].es_scheduler_level0_node + SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo%4 + 4;
                            }
                            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
                            /* EF */
                            thresh1 = SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo/4;
                            if (thresh1 == 1) {
                                fifo = sp_info->egroup[0].es_scheduler_level0_node + SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo%4;
                            } else {
                                fifo = sp_info->egroup[0].es_scheduler_level0_node + SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo%4 + 4;
                            }
                            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
                        }
                    }
                }
		    }
		}
        
	    } else if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP) != 0) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, es_fabric_port_hierachy_set was set to 1, egress schedulers are hooked"
                                   " up by SDK already, this API is not supported in this mode, unit %d\n"),
                       FUNCTION_NAME(), unit));
            rv = BCM_E_UNAVAIL;
	    } else if (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) {
            /* map egress scheduler to a fabric port */
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, scheduler gport 0x%x can not be mapped to a fabric port, unit %d\n"),
                       FUNCTION_NAME(), gport, unit));
            rv = BCM_E_PARAM;
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, gport 0x%x can not be mapped to a fabric port, unit %d\n"),
                       FUNCTION_NAME(), gport, unit));
            rv = BCM_E_PARAM;
	    }
	    break;
        
	case bcmCosqControlFabricPortIngressScheduler:
	    if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, TM_FABRIC_PORT_HIERARCHY_SETUP) != 0) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, tm_fabric_port_hierachy_set was set to 1, ingress schedulers are hooked"
                                   " up by SDK already, this API is not supported in this mode, unit %d\n"),
                       FUNCTION_NAME(), unit));
            rv = BCM_E_UNAVAIL;
	    } else {
            /* map the scheduler gport to the specified fabric port (flow control domain) */
            if (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport)) {
                rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
                if (rv != BCM_E_NONE) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "gport control set, unsupported gport on unit %d\n"),
                              unit));
                    rv = BCM_E_PARAM;
                    return rv;
                }
                
                if (level < 4) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "gport 0x%x is at level %d, only scheduler level 4 and above"
                                         " could be mapped on unit %d\n"), gport, level, unit));
                    rv = BCM_E_PARAM;
                    return rv;			
                }
                
                if (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "gport 0x%x is at node %d, out of level %d node range %d on unit %d"),
                              gport, node, level, SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level], unit));
                    rv = BCM_E_PARAM;
                    return rv;		    
                }
                
                s_i_state = &is_state[unit][level][node];
                if ((s_i_state == NULL) || (s_i_state->in_use == FALSE)) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "gport 0x%x is at level %d node %d not used on unit %d"),
                              gport, level, node, unit));
                    rv = BCM_E_PARAM;
                    return rv;			
                }
                
                /* value is a child gport handle */
		    if (BCM_GPORT_IS_CHILD(value)) {
                handle = BCM_GPORT_CHILD_PORT_GET(value);
		    } else if (BCM_GPORT_IS_EGRESS_CHILD(value)) {
                handle = BCM_GPORT_EGRESS_CHILD_PORT_GET(value);
		    } else {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "gport control set, unsupported gport type for value 0x%x on unit %d\n"),
                          value, unit));
                rv = BCM_E_PARAM;
                return rv;			
		    }
            
		    if ((handle < 0) || (handle >= SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS)) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "gport control set, invalid gport for value 0x%x on unit %d\n"),
                          value, unit));
                rv = BCM_E_PARAM;
                return rv;			
		    }

		    if (handle == SB_FAB_DEVICE_SIRIUS_INGRESS_MCAST_HANDLE) {
                /* ingress multicast port is at level 6 and resources are always available
                 * , no need for this API
                 */
                return BCM_E_NONE;
		    }
            
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[handle]);
		    if (sp_info->valid != TRUE) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "gport control set, invalid gport for value 0x%x on unit %d\n"),
                          value, unit));
                rv = BCM_E_PARAM;
                return rv;
		    }
            
		    fcd = sp_info->egroup[0].ef_fcd;
		    if ( (fcd < 0) || (fcd > SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) ) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "flow control domain %d out of range [0, %d] on unit %d"),
                          fcd, SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS, unit));
                rv = BCM_E_PARAM;
                return rv;			
		    }
            
		    if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
                rv = soc_sirius_ts_fullmap_config(unit, fcd, level, node);
		    } else {
                rv = soc_sirius_ts_fullmap_config(unit, fcd/2, level, node);
		    }
		    if (rv != SOC_E_NONE) {
                return rv;
		    }
            
		    sp_info->ts_scheduler_level = level;
		    sp_info->ts_scheduler_node = node;
		    sp_info->flags |= SBX_SUBPORT_FLAG_ON_TS;
		} else {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, gport 0x%x is not an ingress scheduler, unit %d\n"),
                           FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
            }
	    }
	    break;
        
        case bcmCosqControlQselOffset:
            rv = _bcm_sirius_cosq_qsel_offset_set(unit, gport, value);
            
            break;
            
	case bcmCosqControlEgressFlowControlThreshold3:
	case bcmCosqControlEgressFlowControlThreshold2:
	case bcmCosqControlEgressFlowControlThreshold1:
	case bcmCosqControlEgressFlowControlThreshold0:
        if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == TRUE) {
            if ((type == bcmCosqControlEgressFlowControlThreshold2) ||
                (type == bcmCosqControlEgressFlowControlThreshold3)) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: Threshold2 & Threshold3 types unsupported in Extended Port Mode\n")));
                rv = BCM_E_PARAM;
                return rv;
            }
        }
        
        if (value > 4194303) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, value %u out of range 0 - 4194303, unit %d\n"),
                       FUNCTION_NAME(), value, unit));
            return BCM_E_PARAM;
        }
        
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            
            if (sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) {
                fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
            } else if (SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo == cosq) {
                fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS -1;
            } else if (SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo == cosq) {
                fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
            } else {
                if (cosq >= num_fifos) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: %s, cosq %d exceeds %d fifos for gport 0x%x, unit %d\n"),
                               FUNCTION_NAME(), cosq, num_fifos, gport, unit));
                    return BCM_E_PARAM;
                }
                
                fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
                if (bcm_sbx_cosq_fcd_get_from_fifo(unit, fifo, &fcd, 0) == BCM_E_NOT_FOUND) {
                    fcd = sp_info->egroup[eg_n].ef_fcd;
                }
            }
            
            /* Get current settings */
            if (fcd >= SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS-1) {
                BCM_IF_ERROR_RETURN(READ_FR_CONFIG7r(unit, &uRegValue));
                method = soc_reg_field_get(unit, FR_CONFIG7r, uRegValue, MC_FLOW_CTL_METHODf);
            } else if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
                method = 0;
            } else {
                method = 1;
            }
            
            BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_get(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_EF, &bytes_min[0], &bytes_max[0]));
            if (method == 1) {
                BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_get(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_NEF, &bytes_min[1], &bytes_max[1]));
            }
            
            if (type == bcmCosqControlEgressFlowControlThreshold0) {
                bytes_min[0] = value;
                if (value > bytes_max[0]) {
                    LOG_WARN(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "WARN: %s threshold0 (0x%x) larger than threshold1 (0x%x),"
                                          "updating threshold1\n"),
                              FUNCTION_NAME(), value, bytes_max[0]));
                }
                bytes_max[0] = value;
                if (method == 1) {
                    if (value > bytes_min[1]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold0 (0x%x) larger than threshold2 (0x%x),"
                                              "updating threshold2\n"),
                                  FUNCTION_NAME(), value, bytes_min[1]));
                    }
                    bytes_min[1] = value;
                    if (value > bytes_max[1]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold0 (0x%x) larger than threshold3 (0x%x),"
                                              "updating threshold3\n"),
                                  FUNCTION_NAME(), value, bytes_max[1]));
                    }
                    bytes_max[1] = value;
                }
            } else if (type == bcmCosqControlEgressFlowControlThreshold1) {
                bytes_max[0] = value;
                if (value < bytes_min[0]) {
                    LOG_WARN(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "WARN: %s threshold1 (0x%x) less than threshold0 (0x%x),"
                                          "updating threshold0\n"),
                              FUNCTION_NAME(), value, bytes_min[0]));
                }
                bytes_min[0] = value;
                if (method == 1) {
                    if (value > bytes_min[1]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold1 (0x%x) larger than threshold2 (0x%x),"
                                              "updating threshold2\n"),
                                  FUNCTION_NAME(), value, bytes_min[1]));
                    }
                    bytes_min[1] = value;
                    if (value > bytes_max[1]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold1 (0x%x) larger than threshold3 (0x%x),"
                                              "updating threshold3\n"),
                                  FUNCTION_NAME(), value, bytes_max[1]));
                    }
                    bytes_max[1] = value;
                }
            }
            
            if (method == 1) {
                if (type == bcmCosqControlEgressFlowControlThreshold2) {
                    bytes_max[1] = value;
                    if (value < bytes_min[0]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold2 (0x%x) less than threshold0 (0x%x),"
                                              "updating threshold0\n"),
                                  FUNCTION_NAME(), value, bytes_min[0]));
                    }
                    bytes_min[0] = value;
                    if (value < bytes_max[0]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold2 (0x%x) less than threshold1 (0x%x),"
                                              "updating threshold1\n"),
                                  FUNCTION_NAME(), value, bytes_max[0]));
                    }
                    bytes_max[0] = value;
                    if (value > bytes_max[1]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold2 (0x%x) larger than threshold3 (0x%x),"
                                              "updating threshold3\n"),
                                  FUNCTION_NAME(), value, bytes_max[1]));
                    }
                    bytes_min[1] = value;
                } else {
                    bytes_max[1] = value;
                    if (value < bytes_min[0]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold3 (0x%x) less than threshold0 (0x%x),"
                                              "updating threshold0\n"),
                                  FUNCTION_NAME(), value, bytes_min[0]));
                    }
                    bytes_min[0] = value;
                    if (value < bytes_max[0]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold3 (0x%x) less than threshold1 (0x%x),"
                                              "updating threshold1\n"),
                                  FUNCTION_NAME(), value, bytes_max[0]));
                    }
                    bytes_max[0] = value;
                    if (value < bytes_min[1]) {
                        LOG_WARN(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "WARN: %s threshold3 (0x%x) less than threshold2 (0x%x),"
                                              "updating threshold2\n"),
                                  FUNCTION_NAME(), value, bytes_min[1]));
                    }
                    bytes_min[1] = value;
                    }
            }
            
            /* Update entry and write back */
            BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_set(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_EF, bytes_min[0], bytes_max[0]));
            if (method == 1) {
                BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_set(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_NEF, bytes_min[1], bytes_max[1]));
            }
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
        
	case bcmCosqControlClassMap:
	    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX) {
            LOG_DEBUG(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "Not supported in SBX mode\n")));
            return BCM_E_UNAVAIL;
	    }
	    if ((cosq >= SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE) ||
            (cosq == SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:Cosq must be within the range of 1 - 15\n")));
            rv = BCM_E_PARAM;
	    } else if (gport != BCM_GPORT_INVALID) {
            
            return BCM_E_UNAVAIL;
	    }
	    
	    for (i=SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0; 
             i < SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0 + SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; i++) {
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[i]);
            if(sp_info->valid) {
                fifo = sp_info->egroup[0].num_fifos;
                break;
            }
	    }
        
	    if (fifo == 0) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Multicast Egress Group resource not allocated\n"),
                       FUNCTION_NAME()));
            return BCM_E_UNAVAIL;
	    }
        
	    if (cosq >= fifo) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Multicast Egress Group configured for %d fifos, cosq exceeds limit.\n"),
                       FUNCTION_NAME(),fifo));
            return BCM_E_PARAM;
	    }
        
	    if (value == BCM_COS_MULTICAST_EF) {     
            old_fifo = SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo;
            SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo = cosq;
	    } else if (value == BCM_COS_MULTICAST_NON_EF) {
            old_fifo = SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo;
            SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo = cosq;
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported Class Map Value\n")));
            rv = BCM_E_PARAM;
            return rv;
	    }
        
	    /* set multicast for all ports */
	    
	    if (gport == BCM_GPORT_INVALID) {
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl) {
                if (value == BCM_COS_MULTICAST_EF) {
                    for (ds_id=0; ds_id < SOC_SBX_CFG(unit)->num_ds_ids*2; ds_id+=2) {
                        fifo = (SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo & ~3)>>2;
                        BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_set(unit, ds_id+fifo, 1, 1, SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo));
                        BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_set(unit, ds_id+fifo, 0, 1, SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo));
                    }
                }
                if (value == BCM_COS_MULTICAST_NON_EF) {
                    for (ds_id=SOC_SBX_CFG(unit)->num_ds_ids*2; ds_id < SOC_SBX_CFG(unit)->num_internal_ds_ids*2; ds_id+=2) {
                        fifo = (SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo & ~3)>>2;
                        BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_set(unit, ds_id+fifo, 1, 1, SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo));
                        BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_set(unit, ds_id+fifo, 0, 1, SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo));
                    }
                }
            } else {
                for (ds_id=0; ds_id < SOC_SBX_CFG(unit)->num_ds_ids*2; ds_id+=2) {
                    if (value == BCM_COS_MULTICAST_EF) {
                        fifo = (SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo & ~3)>>2;
                        BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_set(unit, ds_id+fifo, 1, 1, SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo));
                    }
                    if (value == BCM_COS_MULTICAST_NON_EF) {
                        fifo = (SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo & ~3)>>2;
                        BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_set(unit, ds_id+fifo, 0, 1, SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo));
                    }
                }
            }
            /* Update the FIFO EF/NEF setting in the FIFO_MAP_TABLE */
            for(i=SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0; i <= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE3; i++) {
                sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[i]);
                if(sp_info->valid) {
                    /* Update port speed */
                    sp_info->egroup[0].port_speed[cosq] = sp_info->egroup[0].port_speed[old_fifo];
                    sp_info->egroup[0].port_speed[old_fifo] = 0;
                    for (fifo=0; fifo < sp_info->egroup[0].num_fifos; fifo++) {
                        BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, 
                                                                 sp_info->egroup[0].es_scheduler_level0_node + fifo, &fifo_entry));
                        if ((fifo == SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo) ||
                            (fifo == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo)) {
                            soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, EFf, 1);
                        } else {
                            soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, EFf, 0);
                        }
                        BCM_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, 
                                                                  sp_info->egroup[0].es_scheduler_level0_node + fifo, &fifo_entry));
                    }
                }
                /* Reconfigure the multicast ports associated with new fifos */
                soc_sirius_config_ff(unit, i, 0, FF_MEM_UPDATE);
            }
	    }
	    break;
        
    case bcmCosqControlSchedulerAdoptAllPriority:
        rv = _bcm_sirius_cosq_scheduler_adopt_all_priority_set(unit, gport, cosq, type, value,
                                                               control_info);
	    break;
        
	case bcmCosqControlDropLimitAlpha:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            if ((value < 0) || (value > 8)) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: Drop Limit Alpha range 0-8\n")));
                rv = BCM_E_PARAM;
		    return rv;	
            }
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            
            if (cosq >= num_fifos) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
                return BCM_E_PARAM;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ( sp_info->valid == 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR %s, gport 0x%x not in use, unit %d"),
                          FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
                return rv;			
            }
            fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
            
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
                    
                    /* determine any Internal Multicast FIFOs (XGS Line card model).      */
                    /* Configure device accordingly                                       */
                    /* determine HiGig trunk membership                                   */
                    rv = bcm_sirius_stk_modid_get(unit, &modid);
                    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, failed to get modid, Unit(%d)\n"),
                                   FUNCTION_NAME(), unit));
                        return rv;
                    }	
                    
                    rv = bcm_sirius_trunk_find_and_get(unit,
                                                       modid,
                                                       sp_info->parent_gport,
                                                       &tid,
                                                       &trunkInfo,
                                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                       &(trunkMembers[0]),
                                                       &memberCount);
                    if (rv == BCM_E_NOT_FOUND) {
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                     FUNCTION_NAME(), gport));
                        rv = BCM_E_NONE;
                        goto droplimitalpha;
                    }
                    else if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                   FUNCTION_NAME(), gport));
                        return rv;;
                    }
                    
                    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                (BSL_META_U(unit,
                                            "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
                                 FUNCTION_NAME(), gport, memberCount, tid));
                    
                    /* determine the internal multicast ports */
                    for (i = 0; i < memberCount; i++) {
                        if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_COSQ,
                                      (BSL_META_U(unit,
                                                  "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
                                       FUNCTION_NAME(), trunkMembers[i].gport));

			    }
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Gport (0x%x) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
				rv = BCM_E_PARAM;
				return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
			    soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf, 1);
			    soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf, value);
			    BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
			    SOC_SBX_CFG_SIRIUS(unit)->fifo_thresh[fifo] = (1<<15) | value;
			}
			break;
		    }

		}
	    droplimitalpha:
            BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf, 1);
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf, value);
            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
            SOC_SBX_CFG_SIRIUS(unit)->fifo_thresh[fifo] = (1<<15) | value;
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
        
	case bcmCosqControlDropLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            if (value > 0x7fff) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: Drop Limit Bytes range 0-0x7fff\n")));
                rv = BCM_E_PARAM;
                return rv;	
            }
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            
            if (cosq >= num_fifos) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
                return BCM_E_PARAM;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ( sp_info->valid == 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR %s, gport 0x%x not in use, unit %d"),
                          FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
                return rv;			
            }
            fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
            
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
                    
                    /* determine any Internal Multicast FIFOs (XGS Line card model).      */
                    /* Configure device accordingly                                       */
                    /* determine HiGig trunk membership                                   */
                    rv = bcm_sirius_stk_modid_get(unit, &modid);
                    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, failed to get modid, Unit(%d)\n"),
                                   FUNCTION_NAME(), unit));
                        return rv;
                    }	
                    
                    rv = bcm_sirius_trunk_find_and_get(unit,
                                                       modid,
                                                       sp_info->parent_gport,
                                                       &tid,
                                                       &trunkInfo,
                                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                       &(trunkMembers[0]),
                                                       &memberCount);
                    if (rv == BCM_E_NOT_FOUND) {
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                     FUNCTION_NAME(), gport));
                        rv = BCM_E_NONE;
                        goto droplimitbytes;
                    }
                    else if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                   FUNCTION_NAME(), gport));
                        return rv;;
                    }
                    
                    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                (BSL_META_U(unit,
                                            "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
                                 FUNCTION_NAME(), gport, memberCount, tid));
                    
                    /* determine the internal multicast ports */
                    for (i = 0; i < memberCount; i++) {
                        if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_COSQ,
                                      (BSL_META_U(unit,
                                                  "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
                                       FUNCTION_NAME(), trunkMembers[i].gport));
                        }
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s HiGig Port (%d) part of trunk (%d)\n"),
                                     FUNCTION_NAME(), trunkMembers[i].gport, tid));
                        
                        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
                        if ( sp_info->valid == 0) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
                                             "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                  FUNCTION_NAME(), trunkMembers[i].gport, unit));
                            rv = BCM_E_PARAM;
                            return rv;			
                        }
                        fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
                        BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
                        soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf, 0);
                        soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf, value);
                        BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
                        SOC_SBX_CFG_SIRIUS(unit)->fifo_thresh[fifo] = value;
                    }
                    break;
                }
            }
	    droplimitbytes:
            BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf, 0);
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf, value);
            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
            SOC_SBX_CFG_SIRIUS(unit)->fifo_thresh[fifo] = value;
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
        
	case bcmCosqControlYellowDropLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            if (value > 0x7fff) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: Yellow Drop Limit Bytes range 0-0x7fff\n")));
                rv = BCM_E_PARAM;
                return rv;	
            }
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            
            if (cosq >= num_fifos) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
                return BCM_E_PARAM;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ( sp_info->valid == 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR %s, gport 0x%x not in use, unit %d"),
                          FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
                return rv;			
            }
            fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
            
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
                    
                    /* determine any Internal Multicast FIFOs (XGS Line card model).      */
                    /* Configure device accordingly                                       */
                    /* determine HiGig trunk membership                                   */
                    rv = bcm_sirius_stk_modid_get(unit, &modid);
                    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, failed to get modid, Unit(%d)\n"),
                                   FUNCTION_NAME(), unit));
                        return rv;
                    }	
                    
                    rv = bcm_sirius_trunk_find_and_get(unit,
                                                       modid,
                                                       sp_info->parent_gport,
                                                       &tid,
                                                       &trunkInfo,
                                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                       &(trunkMembers[0]),
                                                       &memberCount);
                    if (rv == BCM_E_NOT_FOUND) {
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                     FUNCTION_NAME(), gport));
                        rv = BCM_E_NONE;
                        goto yellowdroplimit;
                    }
                    else if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                   FUNCTION_NAME(), gport));
                        return rv;;
                    }
                    
                    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                (BSL_META_U(unit,
                                            "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
                                 FUNCTION_NAME(), gport, memberCount, tid));
                    
                    /* determine the internal multicast ports */
                    for (i = 0; i < memberCount; i++) {
                        if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_COSQ,
                                      (BSL_META_U(unit,
                                                  "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
                                       FUNCTION_NAME(), trunkMembers[i].gport));
                        }
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s HiGig Port (%d) part of trunk (%d)\n"),
                                     FUNCTION_NAME(), trunkMembers[i].gport, tid));
                        
                        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
                    rv = BCM_E_PARAM;
                    return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_YELLOWm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_yellow));
			    soc_mem_field32_set(unit, EG_FD_FIFO_THRESH_OFFSET_YELLOWm,&thresh_offset_yellow, FIFO_THRESH_OFFSET_YELLOWf, value);
			    BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESH_OFFSET_YELLOWm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_yellow));
                    }
                    break;
                }
            }
	    yellowdroplimit:
            BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_YELLOWm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_yellow));
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESH_OFFSET_YELLOWm,&thresh_offset_yellow, FIFO_THRESH_OFFSET_YELLOWf, value);
            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESH_OFFSET_YELLOWm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_yellow));
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
        
	case bcmCosqControlRedDropLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            if (value > 0x7fff) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: Red Drop Limit Bytes range 0-0x7fff\n")));
                rv = BCM_E_PARAM;
                return rv;	
            }
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            
            if (cosq >= num_fifos) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
                return BCM_E_PARAM;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ( sp_info->valid == 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR %s, gport 0x%x not in use, unit %d"),
                          FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
                return rv;			
            }
            fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
            
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
                    
                    /* determine any Internal Multicast FIFOs (XGS Line card model).      */
                    /* Configure device accordingly                                       */
                    /* determine HiGig trunk membership                                   */
                    rv = bcm_sirius_stk_modid_get(unit, &modid);
                    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, failed to get modid, Unit(%d)\n"),
                                   FUNCTION_NAME(), unit));
                        return rv;
                    }	
                    
                    rv = bcm_sirius_trunk_find_and_get(unit,
                                                       modid,
                                                       sp_info->parent_gport,
                                                       &tid,
                                                       &trunkInfo,
                                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                       &(trunkMembers[0]),
                                                       &memberCount);
                    if (rv == BCM_E_NOT_FOUND) {
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                     FUNCTION_NAME(), gport));
                        rv = BCM_E_NONE;
                        goto reddroplimit;
                    }
                    else if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                   FUNCTION_NAME(), gport));
                        return rv;;
                    }
                    
                    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                (BSL_META_U(unit,
                                            "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
                                 FUNCTION_NAME(), gport, memberCount, tid));
                    
                    /* determine the internal multicast ports */
                    for (i = 0; i < memberCount; i++) {
                        if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_COSQ,
                                      (BSL_META_U(unit,
                                                  "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
                                       FUNCTION_NAME(), trunkMembers[i].gport));
                        }
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s HiGig Port (%d) part of trunk (%d)\n"),
                                     FUNCTION_NAME(), trunkMembers[i].gport, tid));
                        
                        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
                        if ( sp_info->valid == 0) {
                            LOG_INFO(BSL_LS_BCM_COSQ,
                                     (BSL_META_U(unit,
                                                 "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                      FUNCTION_NAME(), trunkMembers[i].gport, unit));
                            rv = BCM_E_PARAM;
                            return rv;			
                        }
                        fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
                        BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_REDm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_red));
                        soc_mem_field32_set(unit, EG_FD_FIFO_THRESH_OFFSET_REDm,&thresh_offset_red, FIFO_THRESH_OFFSET_REDf, value);
                        BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESH_OFFSET_REDm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_red));
                    }
                    break;
                }
            }
	    reddroplimit:
            BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_REDm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_red));
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESH_OFFSET_REDm,&thresh_offset_red, FIFO_THRESH_OFFSET_REDf, value);
            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESH_OFFSET_REDm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_red));
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
        
	case bcmCosqControlResumeLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            if (value > 0x7fff) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: Drop Resume Bytes range 0-0x7fff\n")));
                rv = BCM_E_PARAM;
                return rv;	
            }
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            
            if (cosq >= num_fifos) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
                return BCM_E_PARAM;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ( sp_info->valid == 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR %s, gport 0x%x not in use, unit %d"),
                          FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
                return rv;			
            }
            fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
            
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
                if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
                    (cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {
                    
                    /* determine any Internal Multicast FIFOs (XGS Line card model).      */
                    /* Configure device accordingly                                       */
                    /* determine HiGig trunk membership                                   */
                    rv = bcm_sirius_stk_modid_get(unit, &modid);
                    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, failed to get modid, Unit(%d)\n"),
                                   FUNCTION_NAME(), unit));
                        return rv;
                    }	
                    
                    rv = bcm_sirius_trunk_find_and_get(unit,
                                                       modid,
                                                       sp_info->parent_gport,
                                                       &tid,
                                                       &trunkInfo,
                                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                       &(trunkMembers[0]),
                                                       &memberCount);
                    if (rv == BCM_E_NOT_FOUND) {
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                     FUNCTION_NAME(), gport));
                        rv = BCM_E_NONE;
                        goto resumedroplimit;
                    }
                    else if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COSQ,
                                  (BSL_META_U(unit,
                                              "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
                                   FUNCTION_NAME(), gport));
                        return rv;;
                    }
                    
                    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                (BSL_META_U(unit,
                                            "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
                                 FUNCTION_NAME(), gport, memberCount, tid));
                    
                    /* determine the internal multicast ports */
                    for (i = 0; i < memberCount; i++) {
                        if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                            if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                continue;
                            }
                        } else {
                            LOG_ERROR(BSL_LS_BCM_COSQ,
                                      (BSL_META_U(unit,
                                                  "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
                                       FUNCTION_NAME(), trunkMembers[i].gport));
                        }
                        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                    (BSL_META_U(unit,
                                                "%s HiGig Port (%d) part of trunk (%d)\n"),
                                     FUNCTION_NAME(), trunkMembers[i].gport, tid));
                        
                        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
                        if ( sp_info->valid == 0) {
                            LOG_INFO(BSL_LS_BCM_COSQ,
                                     (BSL_META_U(unit,
                                                 "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                      FUNCTION_NAME(), trunkMembers[i].gport, unit));
                            rv = BCM_E_PARAM;
                            return rv;			
                        }
                        fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
                        BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_RESET_OFFSETm(unit, MEM_BLOCK_ALL, fifo, &thresh_reset_offset));
                        soc_mem_field32_set(unit, EG_FD_FIFO_THRESH_RESET_OFFSETm,&thresh_reset_offset, FIFO_THRESH_RESET_OFFSETf, value);
                        BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESH_RESET_OFFSETm(unit, MEM_BLOCK_ALL, fifo, &thresh_reset_offset));
                    }
                    break;
                }
            }
	    resumedroplimit:
            BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_RESET_OFFSETm(unit, MEM_BLOCK_ALL, fifo, &thresh_reset_offset));
            soc_mem_field32_set(unit, EG_FD_FIFO_THRESH_RESET_OFFSETm,&thresh_reset_offset, FIFO_THRESH_RESET_OFFSETf, value);
            BCM_IF_ERROR_RETURN(WRITE_EG_FD_FIFO_THRESH_RESET_OFFSETm(unit, MEM_BLOCK_ALL, fifo, &thresh_reset_offset));
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
	case bcmCosqControlFlowControlState:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
            BCM_GPORT_IS_EGRESS_GROUP(gport)) {
            
            subport = -1;
            rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
                           FUNCTION_NAME(), subport, gport, unit));
                return BCM_E_PARAM;
            }
            
            sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
            if ( sp_info->valid == 0) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR %s, gport 0x%x not in use, unit %d"),
                          FUNCTION_NAME(), gport, unit));
                rv = BCM_E_PARAM;
                return rv;			
            }
            
            for (fifo = sp_info->egroup[eg_n].es_scheduler_level0_node; 
                 fifo < sp_info->egroup[eg_n].es_scheduler_level0_node + num_fifos;
                 fifo++, value >>= 1) {
                
                rv = soc_sirius_es_fc_map_table_get(unit, fifo/SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE,
                                                    0, &fc_state_index);
                if (rv != SOC_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR:failed to get flow control state index for gport 0x%x\n"),
                               gport));
                    return rv;
                }
                
                if (fc_state_index > SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max) {
                    LOG_ERROR(BSL_LS_BCM_COSQ,
                              (BSL_META_U(unit,
                                          "ERROR: flow control state index %d is not valid for gport 0x%x\n"),
                               fc_state_index, gport));
                    return BCM_E_PARAM;
                }
                
                SOC_IF_ERROR_RETURN(READ_FLOW_CONTROL_STATE_TABLEm(unit, MEM_BLOCK_ANY,
                                                                   fc_state_index, &fc_state_table_entry));
                
                uRegValue = soc_mem_field32_get(unit, FLOW_CONTROL_STATE_TABLEm,
                                                &fc_state_table_entry, FC_STATEf);
                uRegValue &= ~(1<<(fifo %4)); /* clear the bit for the fifo */
                uRegValue |= ((value & 0x1)<<(fifo %4)); /* put on the new bit */
                soc_mem_field32_set(unit, FLOW_CONTROL_STATE_TABLEm, &fc_state_table_entry,
                                    FC_STATEf, uRegValue);
                
                SOC_IF_ERROR_RETURN(WRITE_FLOW_CONTROL_STATE_TABLEm(unit, MEM_BLOCK_ANY,
                                                                    fc_state_index, &fc_state_table_entry));
            }
	    } else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR:unsupported gport type\n")));
            rv = BCM_E_PARAM;
	    }
	    break;
	case bcmCosqControlIgnoreAnemic:
	    if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
		BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {

		BCM_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
		if (cosq < 0) {
		    start_queue = base_queue;
		    end_queue = base_queue + num_cos - 1;
		} else if (cosq < num_cos) {
		    start_queue = end_queue = (base_queue + cosq);
		} else {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR:invalid num_cos\n")));
		    return BCM_E_PARAM;
		}
		for (queue = start_queue; queue <= end_queue; queue++) {
		    BCM_IF_ERROR_RETURN(soc_sirius_qs_queue_parameter_get(unit, queue, &queue_type, &hold_ts));
		    if (value) {
			/* Ignore Anemic pri, switch queue_type if need */
			if (queue_type == SIRIUS_Q_TYPE_LOCAL_EF) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_EF;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP0) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP0;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP1) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP1;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP2) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP2;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP3) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP3;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP4) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP4;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP5) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP5;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_SP6) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP6;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ0) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ0;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ1) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ1;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ2) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ2;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ3) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ3;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ4) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ4;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ5) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ5;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_RCPQ6) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ6;
			} else {
			    /* do nothing, already ignoring the anemic */
			}
		    } else {
			/* Apply Anemic pri */
			if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_EF) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_EF;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP0) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP0;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP1) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP1;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP2) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP2;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP3) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP3;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP4) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP4;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP5) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP5;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_SP6) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_SP6;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ0) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ0;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ1) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ1;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ2) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ2;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ3) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ3;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ4) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ4;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ5) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ5;
			} else if (queue_type == SIRIUS_Q_TYPE_LOCAL_NOANEMIC_RCPQ6) {
			    queue_type = SIRIUS_Q_TYPE_LOCAL_RCPQ6;
			} else {
			    /* do nothing, already support the anemic */
			}
		    }
		    BCM_IF_ERROR_RETURN(soc_sirius_qs_queue_parameter_set(unit, queue, queue_type, hold_ts));
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
            break;
	default:
	    rv = BCM_E_PARAM;
	    break;
    }
    
    return rv;
}

int
bcm_sirius_cosq_control_get(int unit,
			    bcm_gport_t gport,
			    bcm_cos_queue_t cosq,
			    bcm_cosq_control_t type,
			    int *value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int level = 0, node = 0, hwVal;
    int template;
    int q_type = 0, hold_ts=0, subport = 0, eg_n = 0, num_fifos = 0;
    int base_queue=0, num_cos=0;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_i_state = NULL;
    int max_fc_domain = 0, fc_domain=0;
    rt_fs_entry_t rt_fs_cfg;
    uint32 fifo = 0, bytes_min = 0, bytes_max = 0;
    int mymodid = -1, fcd = -1;
    int handle;
    eg_fd_fifo_thresh_entry_t fifo_thresh;
    eg_fd_fifo_thresh_offset_red_entry_t thresh_offset_red;
    eg_fd_fifo_thresh_offset_yellow_entry_t thresh_offset_yellow;
    eg_fd_fifo_thresh_reset_offset_entry_t thresh_reset_offset;
    int port = -1, modid = 0, i = 0;
    bcm_trunk_t tid;
    int fc_state_index = 0;
    flow_control_state_table_entry_t       fc_state_table_entry;
    uint32 uRegValue = 0;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    switch (type) {
	case bcmCosqControlBandwidthBurstMax:
	    if (BCM_GPORT_IS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport) ||
		BCM_GPORT_IS_SCHEDULER(gport)) {

		if ((BCM_GPORT_IS_CHILD(gport) && (soc_feature(unit, soc_feature_standalone) || soc_feature(unit, soc_feature_hybrid))) || 
		    (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport))) {		    
		    
		    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		    if (rv != BCM_E_NONE) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "gport control set, unsupported gport on unit %d\n"),
			          unit));
			rv = BCM_E_PARAM;
			return rv;
		    }

		    /* value is in kbits */
		    rv = bcm_sirius_cosq_get_egress_shaper_params(unit,
					      level, node, NULL, NULL, NULL,
					      value);
		    
		    if (rv != BCM_E_NONE) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "ERROR: %s BandwidthBurstMax set failed, Unit(%d)\n"),
			          FUNCTION_NAME(), unit));
		    }
		    return rv;
		}

		/* this will make sure we return group shaper for child/egress child gport if trunked */
		level = SIRIUS_ES_LEVEL_GROUP_SHAPER;
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport control get, unsupported gport on unit %d\n"),
		              unit));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( (level != SIRIUS_ES_LEVEL_GROUP_SHAPER) && 
		     ((level < 0) || (node < 0)) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
		               gport));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
		    subport = -1;
		    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
			           FUNCTION_NAME(), subport, gport, unit));
			return BCM_E_PARAM;
		    }
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		    if (cosq < 0) {
			/* point to the parent level, node is in the handle */
			level++;
			node = sp_info->es_scheduler_level1_node[eg_n];
		    } else if (cosq < num_fifos) {
			node += cosq;
		    } else {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
			           cosq, num_fifos));
			rv = BCM_E_PARAM;
			return rv;	
		    }
		}

		/* value is in kbits */
		rv = bcm_sirius_cosq_get_egress_shaper_params(unit,
					  level, node, NULL, NULL, NULL,
					  value);

		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s BandwidthBurstMax get failed, Unit(%d)\n"),
		              FUNCTION_NAME(), unit));
		}

		*value /= 1000;
	    } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
                       BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
                       BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
                       BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
                rv = _bcm_sirius_cosq_ingress_shaper_burst_get(unit, control_info->u.conn.queue, value);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;
	case bcmCosqControlBandwidthBurstMin:
	    if (BCM_GPORT_IS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport) ||
		BCM_GPORT_IS_SCHEDULER(gport)) {

		if ((BCM_GPORT_IS_CHILD(gport) && (soc_feature(unit, soc_feature_standalone) || soc_feature(unit, soc_feature_hybrid))) || 
		    (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport))) {		    
		    
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s BandwidthBurstMin not supported on ingress scheduler, Unit(%d)\n"),
		              FUNCTION_NAME(), unit));
		    rv = BCM_E_UNAVAIL;
		    return rv;
		}

		/* this will make sure we return group shaper for child/egress child gport if trunked */
		level = SIRIUS_ES_LEVEL_GROUP_SHAPER;
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport control get, unsupported gport on unit %d\n"),
		              unit));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( (level != SIRIUS_ES_LEVEL_GROUP_SHAPER) && 
		     ((level < 0) || (node < 0)) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
		               gport));
		    rv = BCM_E_PARAM;
		    return rv;
		}

		if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
		    subport = -1;
		    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		    if (rv != BCM_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
			           FUNCTION_NAME(), subport, gport, unit));
			return BCM_E_PARAM;
		    }
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		    if (cosq < 0) {
			/* point to the parent level, node is in the handle */
			level++;
			node = sp_info->es_scheduler_level1_node[eg_n];
		    } else if (cosq < num_fifos) {
			node += cosq;
		    } else {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
			           cosq, num_fifos));
			rv = BCM_E_PARAM;
			return rv;	
		    }
		}

		/* value is in kbits */
		rv = bcm_sirius_cosq_get_egress_shaper_params(unit,
					  level, node, NULL, value, NULL,
					  NULL);

		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR: %s BandwidthBurstMax get failed, Unit(%d)\n"),
		              FUNCTION_NAME(), unit));
		}

		*value /= 1000;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;
        case bcmCosqControlFabricConnectMinUtilization:
            /* determine template being used by the queue */
            rv = soc_sirius_queue_min_util_get(unit,
                                               control_info->u.conn.queue,
                                               &template);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            /* get template configuration */
            rv = soc_sbx_connect_min_util_get(unit, template, &hwVal);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            /* translate back to BCM representation */
            rv = soc_sirius_template_min_util_recall(unit,
                                                     hwVal,
                                                     value);
            break;

        case bcmCosqControlFabricConnectMaxTime:
            /* determine template being used by the queue */
            /* coverity [callee_ptr_arith] */
            rv = soc_sirius_queue_max_age_get(unit,
                                              control_info->u.conn.queue,
                                              &template);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            /* get template configuration */
            rv = soc_sbx_connect_max_age_get(unit, template, &hwVal);
            if (rv != BCM_E_NONE) {
                return(rv);
            }

            /* translate back to BCM representation */
            rv = soc_sirius_template_max_age_recall(unit,
                                                    hwVal,
                                                    value);
            break;

        case bcmCosqControlFabricConnectMax:
	    BCM_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));
	    BCM_IF_ERROR_RETURN(soc_sirius_qs_queue_parameter_get(unit, base_queue+cosq, &q_type, &hold_ts));
	    *value = hold_ts + 1;
	    break;

        case bcmCosqControlPacketLengthAdjust:
	    rv = BCM_E_UNAVAIL;
	    break;

	case bcmCosqControlFabricPortIngressScheduler:
	    /* find the flow control domain mapped to the ingress scheduler gport */
	    if (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport)) {		    
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, gport control set, unsupported gport on unit %d\n"),
		               FUNCTION_NAME(), unit));
		    rv = BCM_E_PARAM;
		    return rv;
		}
		
		if (level < 4) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, gport 0x%x is at level %d, only scheduler level 4 and above"
		                           " could be mapped on unit %d\n"), FUNCTION_NAME(), gport, level, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}
		
		if (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, gport 0x%x is at node %d, out of level %d node range %d on unit %d"),
		               FUNCTION_NAME(), gport, node, level, SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level], unit));
		    rv = BCM_E_PARAM;
		    return rv;		    
		}
		
		s_i_state = &is_state[unit][level][node];
		if ((s_i_state == NULL) || (s_i_state->in_use == FALSE)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR %s, gport 0x%x is at level %d node %d not used on unit %d"),
		               FUNCTION_NAME(), gport, level, node, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}

		if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
		    max_fc_domain = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
		} else {
		    max_fc_domain = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS / 2;
		}

		handle = -1;
		rv = BCM_E_NOT_FOUND;
		for (fc_domain = 0; fc_domain <= max_fc_domain; fc_domain++) {
		    BCM_IF_ERROR_RETURN(READ_RT_FMm(unit, MEM_BLOCK_ANY, fc_domain, &rt_fs_cfg));
		    if (soc_mem_field32_get(unit, RT_FMm, &rt_fs_cfg, FULL_ENABLEf)) {
			if (((level - 4) == soc_mem_field32_get(unit, RT_FMm, &rt_fs_cfg, FULL_LEVELf)) &&
			    (node == soc_mem_field32_get(unit, RT_FMm, &rt_fs_cfg, FULL_NODEf))) {
			    handle = fc_domain;
			    rv = BCM_E_NONE;
			    break;
			}
		    }
		}

		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to find flow control domain for gport 0x%x on unit %d"),
		               FUNCTION_NAME(), gport, unit));
		    return rv;
		}

		if (SOC_SBX_STATE(unit)->port_state->subport_info == NULL) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, no subport info on unit %d"),
		               FUNCTION_NAME(), unit));
		    return BCM_E_NOT_FOUND;		    
		}

		rv = BCM_E_NOT_FOUND;
		for (subport = 0; subport < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; subport++) {
		    /* subport need to be valid */
		    if (SOC_SBX_STATE(unit)->port_state->subport_info[subport].valid == FALSE) {
			continue;
		    }
    
		    /* subport need to match the flow control domain */
		    if (SOC_SBX_STATE(unit)->port_state->subport_info[subport].egroup[0].ef_fcd == handle) {
			handle = subport;
			rv = BCM_E_NONE;
			break;
		    }
		}

		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, no subport matches flow control domain on unit %d"),
		               FUNCTION_NAME(), unit));
		    return rv;
		} else {
		    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));
		    BCM_GPORT_CHILD_SET(*value, mymodid, handle);
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, gport 0x%x is not an ingress schedulers, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));
		rv = BCM_E_PARAM;
	    }
	    break;

        case bcmCosqControlQselOffset:
            rv = _bcm_sirius_cosq_qsel_offset_get(unit, gport, value);
	    break;

	case bcmCosqControlEgressFlowControlThreshold3:
	case bcmCosqControlEgressFlowControlThreshold2:
	case bcmCosqControlEgressFlowControlThreshold1:
	case bcmCosqControlEgressFlowControlThreshold0:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {

		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);

		if (sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) {
		    fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
		} else if (SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo == cosq) {
		    fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
		} else if (SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo == cosq) {
		    fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
		} else {
		    if (cosq >= num_fifos) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, cosq %d exceeds %d fifos for gport 0x%x, unit %d\n"),
			           FUNCTION_NAME(), cosq, num_fifos, gport, unit));
			return BCM_E_PARAM;
		    }
	      
		    fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
		    if (bcm_sbx_cosq_fcd_get_from_fifo(unit, fifo, &fcd, 0) == BCM_E_NOT_FOUND) {
			fcd = sp_info->egroup[eg_n].ef_fcd;
		    }
		}

		if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == TRUE) {
		    if ((type == bcmCosqControlEgressFlowControlThreshold2) ||
			(type == bcmCosqControlEgressFlowControlThreshold3)) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: Threshold2 & Threshold3 types unsupported in Extended Port Mode\n")));
			rv = BCM_E_PARAM;
			return rv;
		    }
		}

		/* Get current settings */

                BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_get(unit, fcd, cosq, &bytes_min, &bytes_max));
                if ((type == bcmCosqControlEgressFlowControlThreshold0) ||
                    (type == bcmCosqControlEgressFlowControlThreshold2)) {
                    *value = bytes_min;
                } else {
                    *value = bytes_max;
                }
	      
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;

	case bcmCosqControlClassMap:
	    if (gport != BCM_GPORT_INVALID) {
		
		return BCM_E_UNAVAIL;
	    }
	    if (cosq == BCM_COS_MULTICAST_EF) {
		*value = SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo;
	    } else if (cosq == BCM_COS_MULTICAST_NON_EF) {
		*value = SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo;
	    } else {
		*value = -1;
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported Class Map Value\n")));
		rv = BCM_E_PARAM;
	    }
	    break;

        case bcmCosqControlSchedulerAdoptAllPriority:
            rv = _bcm_sirius_cosq_scheduler_adopt_all_priority_get(unit, gport, cosq, type, value,
                                                                                     control_info);
	    break;

	case bcmCosqControlDropLimitAlpha:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	       
		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		if (cosq >= num_fifos) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
		    return BCM_E_PARAM;
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}
		fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
		
		if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
		    if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {

			/* determine any Internal Multicast FIFOs (XGS Line card model).      */
			/* Configure device accordingly                                       */
			/* determine HiGig trunk membership                                   */
			rv = bcm_sirius_stk_modid_get(unit, &modid);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, failed to get modid, Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return rv;
			}	
			
                        rv = bcm_sirius_trunk_find_and_get(unit,
                                                           modid,
                                                           sp_info->parent_gport,
                                                           &tid,
                                                           &trunkInfo,
                                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                           &(trunkMembers[0]),
                                                           &memberCount);
			if (rv == BCM_E_NOT_FOUND) {
			    LOG_VERBOSE(BSL_LS_BCM_COSQ,
			                (BSL_META_U(unit,
			                            "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			                 FUNCTION_NAME(), gport));
			    rv = BCM_E_NONE;
			    goto droplimitalpha;
			}
			else if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			               FUNCTION_NAME(), gport));
			    return rv;;
			}
			
			LOG_VERBOSE(BSL_LS_BCM_COSQ,
			            (BSL_META_U(unit,
			                        "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
			             FUNCTION_NAME(), gport, memberCount, tid));
			
			/* determine the internal multicast ports */
                        for (i = 0; i < memberCount; i++) {
                            if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                    continue;
                                }
                            } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
				continue;
			    }
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
				           FUNCTION_NAME(), trunkMembers[i].gport));
			    }
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Port (%d) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
				rv = BCM_E_PARAM;
				return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
			    if (soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf)) {
				*value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf);
			    } else {
				*value = -1;
				LOG_INFO(BSL_LS_BCM_COSQ,
				         (BSL_META_U(unit,
				                     "ERROR %s, Gport 0x%x Fifo Entry %d configured not dynamic, unit %d"),
				          FUNCTION_NAME(), gport, fifo, unit));
				rv = BCM_E_PARAM;
			    }
			    return rv;
			}
			break;
		    }
		}
	    droplimitalpha:
		BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
		if (soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf)) {
		    *value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf);	
		} else {
		    *value = -1;
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, Gport 0x%x Fifo Entry %d configured not dynamic, unit %d"),
		              FUNCTION_NAME(), gport, fifo, unit));
		    rv = BCM_E_PARAM;
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;

	case bcmCosqControlDropLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	       
		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		if (cosq >= num_fifos) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
		    return BCM_E_PARAM;
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}
		fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;
		
		if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
		    if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {

			/* determine any Internal Multicast FIFOs (XGS Line card model).      */
			/* Configure device accordingly                                       */
			/* determine HiGig trunk membership                                   */
			rv = bcm_sirius_stk_modid_get(unit, &modid);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, failed to get modid, Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return rv;
			}	
			
                        rv = bcm_sirius_trunk_find_and_get(unit,
                                                           modid,
                                                           sp_info->parent_gport,
                                                           &tid,
                                                           &trunkInfo,
                                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                           &(trunkMembers[0]),
                                                           &memberCount);
			if (rv == BCM_E_NOT_FOUND) {
			    LOG_VERBOSE(BSL_LS_BCM_COSQ,
			                (BSL_META_U(unit,
			                            "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			                 FUNCTION_NAME(), gport));
			    rv = BCM_E_NONE;
			    goto droplimitbytes;
			}
			else if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			               FUNCTION_NAME(), gport));
			    return rv;;
			}
			
			LOG_VERBOSE(BSL_LS_BCM_COSQ,
			            (BSL_META_U(unit,
			                        "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
			             FUNCTION_NAME(), gport, memberCount, tid));
			
			/* determine the internal multicast ports */
                        for (i = 0; i < memberCount; i++) {
                            if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                    continue;
                                }
                            } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
				continue;
			    }
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
				           FUNCTION_NAME(), trunkMembers[i].gport));
			    }
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Port (%d) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
				rv = BCM_E_PARAM;
				return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
			    if (soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf)==0) {
				*value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf);	
			    } else {
				*value = -1;
				LOG_INFO(BSL_LS_BCM_COSQ,
				         (BSL_META_U(unit,
				                     "ERROR %s, Gport 0x%x Fifo Entry %d configured for dynamic, unit %d"),
				          FUNCTION_NAME(), gport, fifo, unit));
				rv = BCM_E_PARAM;
			    }
			    return rv;
			}
			break;
		    }
		}
	    droplimitbytes:
		BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESHm(unit, MEM_BLOCK_ALL, fifo, &fifo_thresh));
		if (soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, DYNAMICf)==0) {
		    *value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESHm, &fifo_thresh, FIFO_THRESHf);	
		} else {
		    *value = -1;
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, Gport 0x%x Fifo Entry %d configured for dynamic, unit %d"),
		              FUNCTION_NAME(), gport, fifo, unit));
		    rv = BCM_E_PARAM;
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;

	case bcmCosqControlYellowDropLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {

		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		if (cosq >= num_fifos) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
		    return BCM_E_PARAM;
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}
		fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;

		if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
		    if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {

			/* determine any Internal Multicast FIFOs (XGS Line card model).      */
			/* Configure device accordingly                                       */
			/* determine HiGig trunk membership                                   */
			rv = bcm_sirius_stk_modid_get(unit, &modid);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, failed to get modid, Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return rv;
			}	
			
                        rv = bcm_sirius_trunk_find_and_get(unit,
                                                           modid,
                                                           sp_info->parent_gport,
                                                           &tid,
                                                           &trunkInfo,
                                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                           &(trunkMembers[0]),
                                                           &memberCount);
			if (rv == BCM_E_NOT_FOUND) {
			    LOG_VERBOSE(BSL_LS_BCM_COSQ,
			                (BSL_META_U(unit,
			                            "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			                 FUNCTION_NAME(), gport));
			    rv = BCM_E_NONE;
			    goto yellowdroplimit;
			}
			else if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			               FUNCTION_NAME(), gport));
			    return rv;;
			}
			
			LOG_VERBOSE(BSL_LS_BCM_COSQ,
			            (BSL_META_U(unit,
			                        "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
			             FUNCTION_NAME(), gport, memberCount, tid));
			
			/* determine the internal multicast ports */
                        for (i = 0; i < memberCount; i++) {
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Port (%d) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

                            if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                    continue;
                                }
                            } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
				continue;
			    }
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
				           FUNCTION_NAME(), trunkMembers[i].gport));
			    }
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Port (%d) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
				rv = BCM_E_PARAM;
				return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_YELLOWm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_yellow));
			    *value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESH_OFFSET_YELLOWm,&thresh_offset_yellow, FIFO_THRESH_OFFSET_YELLOWf);
			    return rv;
			}
			break;
		    }
		}
	    yellowdroplimit:
		BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_YELLOWm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_yellow));
		*value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESH_OFFSET_YELLOWm,&thresh_offset_yellow, FIFO_THRESH_OFFSET_YELLOWf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;

	case bcmCosqControlRedDropLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {

		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		if (cosq >= num_fifos) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
		    return BCM_E_PARAM;
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}
		fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;

		if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
		    if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {

			/* determine any Internal Multicast FIFOs (XGS Line card model).      */
			/* Configure device accordingly                                       */
			/* determine HiGig trunk membership                                   */
			rv = bcm_sirius_stk_modid_get(unit, &modid);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, failed to get modid, Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return rv;
			}	
			
                        rv = bcm_sirius_trunk_find_and_get(unit,
                                                           modid,
                                                           sp_info->parent_gport,
                                                           &tid,
                                                           &trunkInfo,
                                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                           &(trunkMembers[0]),
                                                           &memberCount);
			if (rv == BCM_E_NOT_FOUND) {
			    LOG_VERBOSE(BSL_LS_BCM_COSQ,
			                (BSL_META_U(unit,
			                            "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			                 FUNCTION_NAME(), gport));
			    rv = BCM_E_NONE;
			    goto reddroplimit;
			}
			else if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			               FUNCTION_NAME(), gport));
			    return rv;;
			}
			
			LOG_VERBOSE(BSL_LS_BCM_COSQ,
			            (BSL_META_U(unit,
			                        "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
			             FUNCTION_NAME(), gport, memberCount, tid));
			
			/* determine the internal multicast ports */
                        for (i = 0; i < memberCount; i++) {
                            if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                    continue;
                                }
                            } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
				continue;
			    }
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
				           FUNCTION_NAME(), trunkMembers[i].gport));
			    }
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Port (%d) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
				rv = BCM_E_PARAM;
				return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_REDm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_red));
			    *value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESH_OFFSET_REDm,&thresh_offset_red, FIFO_THRESH_OFFSET_REDf);
			    return rv;
			}
			break;
		    }
		}
	    reddroplimit:
		BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_OFFSET_REDm(unit, MEM_BLOCK_ALL, fifo, &thresh_offset_red));
		*value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESH_OFFSET_REDm,&thresh_offset_red, FIFO_THRESH_OFFSET_REDf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;

	case bcmCosqControlResumeLimitBytes:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {

		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		if (cosq >= num_fifos) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, cosq %d out of range (0-%d) for child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), cosq, (num_fifos-1), gport, unit));
		    return BCM_E_PARAM;
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}
		fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + cosq;

		if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
		    if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo) ||
			(cosq == SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo)) {

			/* determine any Internal Multicast FIFOs (XGS Line card model).      */
			/* Configure device accordingly                                       */
			/* determine HiGig trunk membership                                   */
			rv = bcm_sirius_stk_modid_get(unit, &modid);
			if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, failed to get modid, Unit(%d)\n"),
			               FUNCTION_NAME(), unit));
			    return rv;
			}	
			
                        rv = bcm_sirius_trunk_find_and_get(unit,
                                                           modid,
                                                           sp_info->parent_gport,
                                                           &tid,
                                                           &trunkInfo,
                                                           BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                                           &(trunkMembers[0]),
                                                           &memberCount);
			if (rv == BCM_E_NOT_FOUND) {
			    LOG_VERBOSE(BSL_LS_BCM_COSQ,
			                (BSL_META_U(unit,
			                            "%s, No HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			                 FUNCTION_NAME(), gport));
			    rv = BCM_E_NONE;
			    goto resumelimitbytes;
			}
			else if (rv != BCM_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "%s, error in getting HiGig Trunk membership, HiGig Gport: 0x%x\n"),
			               FUNCTION_NAME(), gport));
			    return rv;;
			}
			
			LOG_VERBOSE(BSL_LS_BCM_COSQ,
			            (BSL_META_U(unit,
			                        "%s HiGig Trunk found for Gport: 0x%x NbrPorts: %d Irunk: %d\n"),
			             FUNCTION_NAME(), gport, memberCount, tid));
			
			/* determine the internal multicast ports */
                        for (i = 0; i < memberCount; i++) {
                            if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                                    continue;
                                }
                            } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                                port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(trunkMembers[i].gport);
                                if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
				continue;
			    }
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "%s, error in HiGig Trunk member, HiGig Gport: 0x%x\n"),
				           FUNCTION_NAME(), trunkMembers[i].gport));
			    }
                            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                                        (BSL_META_U(unit,
                                                    "%s HiGig Port (%d) part of trunk (%d)\n"),
                                         FUNCTION_NAME(), trunkMembers[i].gport, tid));

			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0+port]);
			    if ( sp_info->valid == 0) {
                                LOG_INFO(BSL_LS_BCM_COSQ,
                                         (BSL_META_U(unit,
                                                     "ERROR %s, MCAST gport for gport 0x%x not valid, unit %d"),
                                          FUNCTION_NAME(), trunkMembers[i].gport, unit));
				rv = BCM_E_PARAM;
				return rv;			
			    }
			    fifo = sp_info->egroup[0].es_scheduler_level0_node + cosq;
			    BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_RESET_OFFSETm(unit, MEM_BLOCK_ALL, fifo, &thresh_reset_offset));
			    *value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESH_RESET_OFFSETm,&thresh_reset_offset, FIFO_THRESH_RESET_OFFSETf);
			    return rv;
			}
			break;
		    }
		}
	    resumelimitbytes:
		BCM_IF_ERROR_RETURN(READ_EG_FD_FIFO_THRESH_RESET_OFFSETm(unit, MEM_BLOCK_ALL, fifo, &thresh_reset_offset));
		*value = soc_mem_field32_get(unit, EG_FD_FIFO_THRESH_RESET_OFFSETm,&thresh_reset_offset, FIFO_THRESH_RESET_OFFSETf);
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;
	case bcmCosqControlFlowControlState:
	    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
		BCM_GPORT_IS_EGRESS_GROUP(gport)) {

		subport = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, fabric_port %d does not contain child_gport 0x%x, unit %d\n"),
		               FUNCTION_NAME(), subport, gport, unit));
		    return BCM_E_PARAM;
		}

		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
		if ( sp_info->valid == 0) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "ERROR %s, gport 0x%x not in use, unit %d"),
		              FUNCTION_NAME(), gport, unit));
		    rv = BCM_E_PARAM;
		    return rv;			
		}

		*value = 0;
		for (fifo = sp_info->egroup[eg_n].es_scheduler_level0_node; 
		     fifo < sp_info->egroup[eg_n].es_scheduler_level0_node + num_fifos;
		     fifo++) {

		    rv = soc_sirius_es_fc_map_table_get(unit, fifo/SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE,
							0, &fc_state_index);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR:failed to get flow control state index for gport 0x%x\n"),
			           gport));
			return rv;
		    }

		    if (fc_state_index > SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: flow control state index %d is not valid for gport 0x%x\n"),
			           fc_state_index, gport));
			return BCM_E_PARAM;
		    }

		    SOC_IF_ERROR_RETURN(READ_FLOW_CONTROL_STATE_TABLEm(unit, MEM_BLOCK_ANY,
								       fc_state_index, &fc_state_table_entry));
		    
		    uRegValue = soc_mem_field32_get(unit, FLOW_CONTROL_STATE_TABLEm,
						    &fc_state_table_entry, FC_STATEf);
		    *value |= (((uRegValue & (1<<(fifo % 4)))>>(fifo % 4))<<(fifo - sp_info->egroup[eg_n].es_scheduler_level0_node));
		}
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR:unsupported gport type\n")));
		rv = BCM_E_PARAM;
	    }
	    break;
	default:
	    rv = BCM_E_PARAM;
	    break;
    }

    return rv;
}

int
_bcm_sirius_cosq_add_delete_queue(int unit, int add, int32 queue,
				  bcm_sbx_cosq_queue_region_type_t queue_region,
				  bcm_sbx_cosq_bw_group_params_t *p_bwparams,
				  bcm_sbx_cosq_queue_params_ingress_t *p_qparams)
{
    int rv = BCM_E_NONE;
    uint32 regval;
    int buffs_template;
    int32 rate_delta_max_index = 0;
    int32 timeout_in_usec;
    int32 queue_add_done;
    soc_timeout_t timeout;
    uint32 current_buffs_guaranteed, nbr_bufs;
    qbuffsprofile_entry_t profileEntry;
    int queue_enable, rc;

    /* unlike QE2000, this function only add delete queue so that the queue can pass traffic.
     * queue buffering, shaping and scheduling parameters are processed in other call
     */

    /* queue config
     *   demand_enable            -- 1 for VOQ, 0 for LQ
     *   rate_delta_max_index     -- only for VOQ, setup based on the bag rate (config to a default one)
     *   buffs_template           -- template passed in as part of ingress parameters
     */
    if (add) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "adding queue(%d)\n"),
	           queue));

	BCM_IF_ERROR_RETURN(READ_QMB_CONFIG1r(unit, &regval));
	current_buffs_guaranteed = soc_reg_field_get(unit, QMB_CONFIG1r, regval, GUARANTEED_BUFFS_TOTALf);

        /* configure buffer guarantees */
        rv = soc_sirius_qm_queue_size_in_bufs_get(unit,
                            p_qparams->min_physical_queue_depth_bytes, &nbr_bufs);
	if (rv) {
	    return(rv);
	}

	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  Number Current Buffers(%d)\n"),
	           nbr_bufs));
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  Guaranteed Buffers(%d)\n"),
	           current_buffs_guaranteed));
	current_buffs_guaranteed += nbr_bufs;
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  Modified Guaranteed Buffers(%d)\n"),
	           current_buffs_guaranteed));
	if (current_buffs_guaranteed >= (SOC_SBX_CFG_SIRIUS(unit)->uMaxBuffers - 128)) {
	    /* make sure guaranteed buff not exceeds max number of buffers in system, 
	     * 128 buffers as leeway
	     */
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Run out of guarranteed buffers when adding queue(%d)\n"), queue));
	    return BCM_E_RESOURCE;
	}
	soc_reg_field_set(unit, QMB_CONFIG1r, &regval, GUARANTEED_BUFFS_TOTALf,
                                                          current_buffs_guaranteed);
	BCM_IF_ERROR_RETURN(WRITE_QMB_CONFIG1r(unit, regval));


        /* add queue */
	regval=0;
	buffs_template = p_qparams->template;
	soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, BUFFS_TEMPLATEf, buffs_template);
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  bufferTemplate(%d)\n"),
	           buffs_template));

        if (queue_region == bcm_sbx_cosq_queue_region_local) {

	    LOG_DEBUG(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "  local\n")));
	    soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, DEMAND_ENABLEf, 0);
	    soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, RATE_DELTA_MAX_INDEXf, 0);

	}
	else { /* VOQ */

	    /* The bw_group contains the bag rate  */
	    _bcm_sirius_rate_delta_max_index_for_port_rate_chosen(unit, p_bwparams->bag_rate_kbps,
                                                              &rate_delta_max_index);


	    LOG_DEBUG(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "  VoQ RateDelatMaxindex(%d)\n"),
	               rate_delta_max_index));
	    soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, DEMAND_ENABLEf, 1);
	    soc_reg_field_set(unit, QMB_QUEUE_CONFIG_DATAr, &regval, RATE_DELTA_MAX_INDEXf, rate_delta_max_index);
	}

	BCM_IF_ERROR_RETURN(WRITE_QMB_QUEUE_CONFIG_DATAr(unit, regval));
	regval=0;

	soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, ADD_DELETE_Nf, 1);
        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, CQ_ACKf, 1);
	soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, QUEUE_NUMBERf, queue);

        if (SAL_BOOT_PLISIM) {
	    /* Write in 2 steps due to model issue */
	    /* Remove this line when fixed.        */
	    BCM_IF_ERROR_RETURN(WRITE_QMB_QUEUE_CONFIG_CTRLr(unit, regval));
        }

        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, CQ_REQf, 1);
        BCM_IF_ERROR_RETURN(WRITE_QMB_QUEUE_CONFIG_CTRLr(unit, regval));

	/* Timeout awaiting add queue completion
	 */
	timeout_in_usec = 20000;
	queue_add_done = 0;
	soc_timeout_init(&timeout, timeout_in_usec,0);
	while(!soc_timeout_check(&timeout)) {
	    BCM_IF_ERROR_RETURN(READ_QMB_QUEUE_CONFIG_CTRLr(unit,&regval));

	    queue_add_done = soc_reg_field_get(unit, QMB_QUEUE_CONFIG_CTRLr, regval, CQ_ACKf);

	    if (queue_add_done) {
		break;
	    }
	}
	if (!queue_add_done) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout awaiting queue_add completion queue(%d)\n"), queue));
	    rv = BCM_E_TIMEOUT;
	}
	else if (soc_reg_field_get(unit, QMB_QUEUE_CONFIG_CTRLr, regval, CQ_ERRf)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "queue_add failed in hardware for queue(%d)\n"), queue));

	}
	else {
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "queue_add succeeded: rate_delta_max_index(%d)"
	                             " buffs_template(%d)\n"), rate_delta_max_index,
	                 buffs_template));
	}
    }
    else {

        if (SOC_IS_DETACHING(unit)) {
  	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
  	                (BSL_META_U(unit,
  	                            "%s: Unit %d detaching, queue_delete ignored.\n"),
  	                 FUNCTION_NAME(), unit));
            return BCM_E_NONE;
        }

      /* delete queue, and decrement the value in qm_config1.global_guaranteed_buffs_total */
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "deleting queue(%d)\n"),
	           queue));

	BCM_IF_ERROR_RETURN(READ_QMB_CONFIG1r(unit, &regval));
	current_buffs_guaranteed = soc_reg_field_get(unit, QMB_CONFIG1r, regval, GUARANTEED_BUFFS_TOTALf);

        /* configure buffer guarantees */
        rv = soc_sirius_qm_queue_size_in_bufs_get(unit,
                            p_qparams->min_physical_queue_depth_bytes, &nbr_bufs);
        if (rv) {
            return(rv);
        }

	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  Number Current Buffers(%d)\n"),
	           nbr_bufs));
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  Guaranteed Buffers(%d)\n"),
	           current_buffs_guaranteed));
        current_buffs_guaranteed -= nbr_bufs;
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "  Modified Guaranteed Buffers(%d)\n"),
	           current_buffs_guaranteed));


	soc_reg_field_set(unit, QMB_CONFIG1r, &regval, GUARANTEED_BUFFS_TOTALf, current_buffs_guaranteed);
	BCM_IF_ERROR_RETURN(WRITE_QMB_CONFIG1r(unit, regval));

	regval=0;

        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, ADD_DELETE_Nf, 0);
	soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, QUEUE_NUMBERf, queue);
        soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, CQ_ACKf, 1);

        if (SAL_BOOT_PLISIM) {
	    /* Write in 2 steps due to model issue */
	    /* Remove this line when fixed.        */
	    BCM_IF_ERROR_RETURN(WRITE_QMB_QUEUE_CONFIG_CTRLr(unit, regval));
        }

	soc_reg_field_set(unit, QMB_QUEUE_CONFIG_CTRLr, &regval, CQ_REQf, 1);
  	LOG_VERBOSE(BSL_LS_BCM_COSQ,
  	            (BSL_META_U(unit,
  	                        "  Write QMB_QUEUE_CONFIG_CTRL(0x%x)\n"),
  	             regval));
	BCM_IF_ERROR_RETURN(WRITE_QMB_QUEUE_CONFIG_CTRLr(unit, regval));

 	/* Timeout awaiting delete queue completion
	 */
	timeout_in_usec = 20000;
	queue_add_done = 0;
	soc_timeout_init(&timeout, timeout_in_usec,0);
	while(!soc_timeout_check(&timeout)) {
	    BCM_IF_ERROR_RETURN(READ_QMB_QUEUE_CONFIG_CTRLr(unit,&regval));

	    queue_add_done = soc_reg_field_get(unit, QMB_QUEUE_CONFIG_CTRLr, regval, CQ_ACKf);

	    if (queue_add_done) {
	        break;
	    }
	}
	if (!queue_add_done) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Timeout awaiting queue_delete completion queue(%d)\n"), queue));
	    rv = BCM_E_TIMEOUT;
	}
	else if (soc_reg_field_get(unit, QMB_QUEUE_CONFIG_CTRLr, regval, CQ_ERRf)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "queue_delete failed in hardware for queue(%d)\n"), queue));
            rv = BCM_E_FAIL;
	}
	else {
  	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
  	                (BSL_META_U(unit,
  	                            "queue_delete succeeded for queue(%d)\n"),
  	                 queue));
	}

        /* Consistency Check */
        if (rv == BCM_E_NONE) {
            rc = READ_QBUFFSPROFILEm(unit, MEM_BLOCK_ANY, queue, &profileEntry);
            if (rc != BCM_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "error reading QBUFPROFILE table, queue(%d)\n"), queue));
            }
            else {
  	        LOG_DEBUG(BSL_LS_BCM_COSQ,
  	                  (BSL_META_U(unit,
  	                              "  queue(%d) QBUFFSPROFILE(%d)\n"),
  	                   queue, profileEntry.entry_data[0]));

                queue_enable = soc_QBUFFSPROFILEm_field32_get(unit, &profileEntry, ENABLEf);
                if (queue_enable == TRUE) {
	            LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "  queue delete failed to update enable bit in QBUFPROFILE, queue(%d)\n"), queue));
                }
                else {
  	            LOG_DEBUG(BSL_LS_BCM_COSQ,
  	                      (BSL_META_U(unit,
  	                                  "  queue(%d) Enable(%d)\n"),
  	                       queue, queue_enable));
                }
            }
        }

	
	
    }

    return rv;
}

/* allocate group shaper resource
 *
 * egress: IN      "TRUE" if allocating egress group max shaper
 *                 "FALSE" if allocating ingress group shaper
 * shaper: IN/OUT   Auto allocated group shaper if -1, otherwise allocate the specified one
 */
int
bcm_sirius_cosq_group_shaper_allocate(int unit, int egress, int *shaper)
{
    int rv = BCM_E_NONE;
    bcm_sbx_sirius_egress_group_shaper_state_t *s_g_state = NULL;
    int group, group_start, group_end;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	/* egress group shapers */
	s_g_state = gs_state[unit];
	if ( (*shaper >= 0) && (*shaper < SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER) ) {
	    group_start = *shaper;
	    group_end = *shaper;
	} else {
	    group_start = 0;
	    group_end = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER - 1;
	}

	for (group = group_start; (group <= group_end) && (s_g_state != NULL); group++, s_g_state++) {
	    if (s_g_state->in_use == FALSE) {
		s_g_state->in_use = TRUE;
		s_g_state->max_shaper_rate_kbps = 0;
		s_g_state->max_shaper_threshold_bits = 0;
		*shaper = group;
		break;
	    }
	}
	if (*shaper < 0) {
	    rv = BCM_E_NOT_FOUND;
	} else {
	    /* init the group shaper */
	    rv = soc_sirius_es_node_shaper_config(unit, SIRIUS_ES_LEVEL_GROUP_SHAPER, *shaper,
						  FALSE /*max shaper*/, FALSE, 0, 0, 0);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: failed to init egress group shaper (%d)\n"),
		           *shaper));
		return rv;
	    }
	}
    } else {
	
	rv = BCM_E_UNAVAIL;
    }

    return rv;    
}

/* Free group shaper resource
 *
 * egress: IN      "TRUE" if allocating egress group max shaper
 *                 "FALSE" if allocating ingress group shaper
 * shaper: IN      Free alloc the specified shaper
 */
int
bcm_sirius_cosq_group_shaper_free(int unit, int egress, int shaper)
{
    int rv = BCM_E_NONE;
    bcm_sbx_sirius_egress_group_shaper_state_t *s_g_state = NULL;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	/* egress group shapers */
	s_g_state = gs_state[unit];
	if ( (s_g_state == NULL) ||
	     (shaper < 0) ||
	     (shaper >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: egress group shaper (%d) doesn't exist\n"),
	               shaper));
	    return BCM_E_PARAM;
	}

	s_g_state += shaper;

	if (s_g_state->in_use == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: egress group shaper (%d) not in use\n"),
	               shaper));
	    return BCM_E_PARAM;
	}

	s_g_state->in_use = FALSE;
	s_g_state->max_shaper_rate_kbps = 0;
	s_g_state->max_shaper_threshold_bits = 0;

	/* init the group shaper */
	rv = soc_sirius_es_node_shaper_config(unit, SIRIUS_ES_LEVEL_GROUP_SHAPER, shaper,
					      FALSE /*max shaper*/, FALSE, 0, 0, 0);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: failed to init egress group shaper (%d)\n"),
	               shaper));
	    return rv;
	}
    } else {
	
	rv = BCM_E_UNAVAIL;
    }

    return rv;    
}

/* Allocate physical scheduler nodes. (Internal use only)
 * IN:
 *     level           - scheduler level
 *     node_requested  - user specified node or first available if pass in
 *                       BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT
 *     num_nodes       - number of continues nodes need to allocate
 *                       (only supports level 0 for now)
 *     egress          - TRUE to allocate es schedulers
 *                       FALSE to allocate ts schedulers
 * OUT:
 *     p_node          - node id of the first allocated node
 *     rv              - BCM_E_NONE if successfully, all other indicate failure
 * Only nodes with in_use state == FALSE will be considered for allocation
 * nodes with in_use state as TRUE or SIRIUS_INGRESS_SCHEDULER_RESERVED will be
 * skipped
 */
int
bcm_sirius_cosq_scheduler_allocate(int unit, int level, int node_requested, 
				   int num_nodes, int egress, int *p_node)
{
    int rv = BCM_E_NONE;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_i_state = NULL;
    bcm_sbx_sirius_egress_scheduler_state_t *s_e_state = NULL;
    int node, tmp_node, in_use, found;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    /* for now, only allow allocation of upto 48 continues nodes on egress level 0 
     * or up to 8 on ingress nodes
     */
    if ( (num_nodes <= 0) || (num_nodes > 48) ) {
	return BCM_E_PARAM;
    } else if ( (num_nodes != 1) && 
		!(((egress == TRUE) && (level == 0)) ||
		  ((egress == FALSE) && (num_nodes <= 8))) ) {
	return BCM_E_PARAM;
    }

    if (egress) {
	if ( (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ) {
	    return BCM_E_INTERNAL;
	}
    } else {
	if ( (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ) {
	    return BCM_E_INTERNAL;
	}
    }

    *p_node = - 1;
    found = FALSE;
    if (egress) {
	if (node_requested == BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
	    /* auto allocation */
	    for (node = 0; node < SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]; node++) {
		/* find the first one available */
		if ( (SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level] - node) < num_nodes ) {
		    rv = BCM_E_RESOURCE;
		    break;
		}
		
		in_use = FALSE;
		for (tmp_node = node, s_e_state = &es_state[unit][level][node]; 
		     tmp_node < node + num_nodes; tmp_node++, s_e_state++) {
		    in_use |= s_e_state->in_use;
		}

		if (in_use == FALSE) {
		    *p_node = node;
		    for (tmp_node = node, s_e_state = &es_state[unit][level][node]; 
			 tmp_node < node + num_nodes; tmp_node++, s_e_state++) {
			BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT_PTR(s_e_state);
			s_e_state->in_use = TRUE;
		    }
		    found = TRUE;
		    break;
		}
	    }
	} else {
	    /* specified node */
	    if ( (node_requested + num_nodes) > SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]) {
		/* requested node is out of range */
		return BCM_E_RESOURCE;
	    }

	    in_use = FALSE;
	    for (tmp_node = node_requested, s_e_state = &es_state[unit][level][node_requested];
		 tmp_node < node_requested + num_nodes; tmp_node++, s_e_state++) {
		in_use |= s_e_state->in_use;
	    }

	    if (in_use == FALSE) {
		*p_node = node_requested;
		for (tmp_node = node_requested, s_e_state = &es_state[unit][level][node_requested]; 
		     tmp_node < node_requested + num_nodes; tmp_node++, s_e_state++) {
		    BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT_PTR(s_e_state);
		    s_e_state->in_use = TRUE;
		}
		found = TRUE;
	    } else {
		/* one of the requested nodes is already used */
		return BCM_E_RESOURCE;
	    }
	}

	if (found == TRUE) {
	    /* initialize all the allocated nodes */
	    for (tmp_node = *p_node; tmp_node < *p_node + num_nodes; tmp_node++) {
		rv = soc_sirius_scheduler_init(unit, level, tmp_node, TRUE /* egress scheduler */);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, can not init egress scheduler level %d node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), level, *p_node, unit));
		    return BCM_E_INTERNAL;
		}
	    }
	} else {
	    return BCM_E_RESOURCE;
	}	
    } else {
	if (node_requested == BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
	    /* auto allocation */
	    for (node = 0; node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; node++) {
		/* find the first one available */
		if ( (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] - node) < num_nodes ) {
		    rv = BCM_E_RESOURCE;
		    break;
		}
		
		in_use = FALSE;
		for (tmp_node = node, s_i_state = &is_state[unit][level][node]; 
		     tmp_node < node + num_nodes; tmp_node++, s_i_state++) {
		    in_use |= s_i_state->in_use;
		}

		if (in_use == FALSE) {
		    *p_node = node;
		    for (tmp_node = node, s_i_state = &is_state[unit][level][node]; 
			 tmp_node < node + num_nodes; tmp_node++, s_i_state++) {
			BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(s_i_state);
			s_i_state->in_use = TRUE;
		    }
		    found = TRUE;
		    break;
		}
	    }
	} else {
	    /* specified node */
	    if ( (node_requested + num_nodes) > SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) {
		/* requested node is out of range */
		return BCM_E_RESOURCE;
	    }

	    in_use = FALSE;
	    for (tmp_node = node_requested, s_i_state = &is_state[unit][level][node_requested];
		 tmp_node < node_requested + num_nodes; tmp_node++, s_i_state++) {
		in_use |= s_i_state->in_use;
	    }

	    if (in_use == FALSE) {
		*p_node = node_requested;
		for (tmp_node = node_requested, s_i_state = &is_state[unit][level][node_requested]; 
		     tmp_node < node_requested + num_nodes; tmp_node++, s_i_state++) {
		    BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(s_i_state);
		    s_i_state->in_use = TRUE;
		}
		found = TRUE;
	    } else {
		/* one of the requested nodes is already used */
		return BCM_E_RESOURCE;
	    }
	}

	if (found == TRUE) {
	    /* initialize all the allocated nodes */
	    for (tmp_node = *p_node; tmp_node < *p_node + num_nodes; tmp_node++) {
		rv = soc_sirius_scheduler_init(unit, level, tmp_node, FALSE /* ingress scheduler */);
		if (rv != SOC_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, can not init ingress scheduler level %d node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), level, *p_node, unit));
		    return BCM_E_INTERNAL;
		}
	    }
	} else {
	    return BCM_E_RESOURCE;
	}
    }

    return rv;
}

/* Free physical scheduler nodes. (Internal use only)
 * IN:
 *     level           - scheduler level
 *     node            - scheduler node
 *     egress          - TRUE to free es schedulers
 *                       FALSE to free ts schedulers
 * OUT:
 *     rv              - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 *     Ingress scheduler has to free all of it's children first
 *     No check was done for egress schedulers 
 *     Only update the specified level, node. If parents need to be 
 *     updated, requires scheduler_disconnect first.
 */
int
bcm_sirius_cosq_scheduler_free(int unit, int level, int node, int egress)
{
    int rv = BCM_E_NONE;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_i_state = NULL;
    bcm_sbx_sirius_egress_scheduler_state_t *s_e_state = NULL;
    int child_level, child_node;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	if ( (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ) {
	    return BCM_E_INTERNAL;
	}
    } else {
	if ( (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ) {
	    return BCM_E_INTERNAL;
	}
    }

    if (egress) {
	/* specified node */
	if ( (node < 0) || (node > SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]) ) {
	    /* requested node is out of range */
	    return BCM_E_PARAM;
	}

	s_e_state = &es_state[unit][level][node];

	if (s_e_state->in_use != TRUE) {
	    LOG_WARN(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "WARN: %s, egress scheduler level %d node %d is not in use Unit(%d)\n"),
	              FUNCTION_NAME(), level, node, unit));
	}
	BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT_PTR(s_e_state);
    } else {
	/* specified node */
	if ( (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	    /* requested node is out of range */
	    return BCM_E_PARAM;
	}

	s_i_state = &is_state[unit][level][node];
	if (s_i_state->in_use != TRUE) {
	    /* requested node is not used
	    LOG_WARN(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "WARN: %s, ingress scheduler level %d node %d is not in use Unit(%d)\n"),
	              FUNCTION_NAME(), level, node, unit));
	    */
	    return BCM_E_NONE;
	} else {
	    if (s_i_state->num_child != 0) {
		if (level == 5){
		    /* point parent of all its children to one of reserved node (last node on level 6 */
		    child_level = level - 1;
		    for (child_node = s_i_state->first_child;
			 child_node < (s_i_state->first_child+s_i_state->num_child);
			 child_node++) {
			rv = soc_sirius_ts_node_hierachy_config(unit, child_level, child_node,
								SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]-1,
								SOC_SIRIUS_API_PARAM_NO_CHANGE,
								SOC_SIRIUS_API_PARAM_NO_CHANGE);		    
			if (rv != SOC_E_NONE) {
			    LOG_ERROR(BSL_LS_BCM_COSQ,
			              (BSL_META_U(unit,
			                          "ERROR: %s, failed to disconnect child %d of ingress scheduler level %d node %d Unit(%d)\n"),
			               FUNCTION_NAME(), child_node, level, node, unit));
			    return BCM_E_INTERNAL;
			}
		    }

		    BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(s_i_state);
		} else {
		    /* special case for level 5 per customer request, allow level 5 schedulers be freed even with children */
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, ingress scheduler level %d node %d still has children, can not be freed Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, unit));
		    return BCM_E_PARAM;
		}
	    } else {
		BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(s_i_state);
	    }
	}
    }

    rv = soc_sirius_scheduler_init(unit, level, node, egress);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, can not re-init %s scheduler level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), egress?"egress":"ingress", level, node, unit));
    }

    return rv;
}

int
bcm_sirius_cosq_scheduler_init(int unit, int level, int node, int egress)
{
    int rv = BCM_E_NONE;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_i_state = NULL;
    bcm_sbx_sirius_egress_scheduler_state_t *s_e_state = NULL;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	if ( (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ) {
	    return BCM_E_INTERNAL;
	}
    } else {
	if ( (level < 0) || (level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ) {
	    return BCM_E_INTERNAL;
	}
    }

    if (egress) {
	/* specified node */
	if ( (node < 0) || (node > SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]) ) {
	    /* requested node is out of range */
	    return BCM_E_PARAM;
	}

	s_e_state = &es_state[unit][level][node];
	BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT_PTR(s_e_state);
    } else {
	/* specified node */
	if ( (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]) ) {
	    /* requested node is out of range */
	    return BCM_E_PARAM;
	}

	s_i_state = &is_state[unit][level][node];
	BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT_PTR(s_i_state);
    }

    rv = soc_sirius_scheduler_init(unit, level, node, egress);
    if (rv != SOC_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, can not init %s scheduler level %d node %d, Unit(%d)\n"),
	           FUNCTION_NAME(), egress?"egress":"ingress", level, node, unit));
    }

    return rv;
}

int
bcm_sirius_cosq_scheduler_connect(int unit, int parent_level, int parent_node, int child_level, int child_node,
				  int queue, int egress, int fifo_num)
{
    int rv = BCM_E_NONE;
    int destination, max_num_child;
    bcm_sbx_sirius_ingress_scheduler_state_t *ps_i_state;
    bcm_sbx_sirius_ingress_scheduler_state_t *cs_i_state;
    bcm_sbx_sirius_egress_scheduler_state_t *ps_e_state;
    bcm_sbx_sirius_egress_scheduler_state_t *cs_e_state;
    bcm_sbx_cosq_queue_state_t *q_state;
    bcm_sbx_cosq_bw_group_state_t *bw_state;
    channel_map_table_entry_t channel_entry;
    subport_map_table_entry_t subport_entry;
    fifo_map_table_entry_t fifo_entry;
    int intf, channel, ef = FALSE, index;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (egress) {
	if ( (parent_level <= 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
	     (child_level < 0) || (child_level >= (SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS - 1)) ) {
	    return BCM_E_INTERNAL;
	}
    } else {
	if ( (parent_level < 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ||
	     (child_level < 0) || (child_level >= (SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS - 1)) ||
	     (parent_node < 0) || (parent_node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[parent_level]) ||
	     (child_node < 0) || (child_node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level]) ) {
	    return BCM_E_INTERNAL;
	}
    }

    if (egress) {
	if (parent_level != (child_level + 1)) {
	    return BCM_E_PARAM;
	}

	switch (child_level) {
	    case SIRIUS_ES_LEVEL_FIFO:
		/* FIFO level, get the intf/channel from parent */
		if ( fifo_num == -1 ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 0 child node %d has no allocated fifos for Unit (%d)\n"),
		               FUNCTION_NAME(), child_node, unit));
		    return BCM_E_PARAM;
		}

		if ( (child_node < 0) || (child_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 0 child node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0 - 1), unit));
		    return BCM_E_PARAM;
		}

		if ( (parent_node < 0) || (parent_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 0 parent node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), parent_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 - 1), unit));
		    return BCM_E_PARAM;
		}

		rv = READ_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, parent_node, &subport_entry);
		if (BCM_E_NONE != rv) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "Unable to read %d:subportMap[%d]: %d (%s)\n"),
		               unit, parent_node,
		               rv, _SHR_ERRMSG(rv)));
		    return rv;
		}

		channel = soc_mem_field32_get(unit, SUBPORT_MAP_TABLEm, &subport_entry, CHNf);
		intf = soc_mem_field32_get(unit, SUBPORT_MAP_TABLEm, &subport_entry, INFf);

		/*
		 * Set 2-fifo and 4-fifo even fifo EF enabled as default
		 */

		if ((fifo_num == SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo) ||
		    (fifo_num == SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo)) {
		    ef = TRUE;
		} else {
		    ef = FALSE;
		}

		index = -1;
		rv = soc_sirius_es_node_map_index_first_available(unit, parent_level, parent_node, &index);
		if (rv != SOC_E_NONE) {
		    return rv;
		}

		rv = soc_sirius_es_node_hierachy_config(unit, child_level, child_node, TRUE, intf,
							channel, parent_node /* subport level */, ef);

		if (rv != SOC_E_NONE) {
		    break;
		}
		ps_e_state = &es_state[unit][parent_level][parent_node];
		cs_e_state = &es_state[unit][child_level][child_node];
		
		cs_e_state->intf = intf;
		cs_e_state->channel = channel;
		cs_e_state->subport = parent_node;
		cs_e_state->index = index;
		ps_e_state->num_child++;

		BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &fifo_entry));
		soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MAP_INDEXf, cs_e_state->index);
		BCM_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &fifo_entry));

		break;
	    case SIRIUS_ES_LEVEL_SUBPORT:
		if ( (child_node < 0) || (child_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 1 child node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 - 1), unit));
		    return BCM_E_PARAM;
		}

		if ( (parent_node < 0) || (parent_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 2 parent node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), parent_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2 - 1), unit));
		    return BCM_E_PARAM;
		}

		rv = READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, parent_node, &channel_entry);
		if (BCM_E_NONE != rv) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "Unable to read %d:channelMap[%d]: %d (%s)\n"),
		               unit, parent_node,
		               rv, _SHR_ERRMSG(rv)));
		    return rv;
		}


		index = -1;
		rv = soc_sirius_es_node_map_index_first_available(unit, parent_level, parent_node, &index);
		if (rv != SOC_E_NONE) {
		    return rv;
		}

		intf = soc_mem_field32_get(unit, CHANNEL_MAP_TABLEm, &channel_entry, INFf);

		rv = soc_sirius_es_node_hierachy_config(unit, child_level, child_node, TRUE, intf,
							parent_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE);
		if (rv != SOC_E_NONE) {
		    break;
		}
		ps_e_state = &es_state[unit][parent_level][parent_node];
		cs_e_state = &es_state[unit][child_level][child_node];
		
		cs_e_state->intf = intf;
		cs_e_state->channel = parent_node;
		cs_e_state->subport = -1;
		cs_e_state->index = index;

		ps_e_state->num_child++;
		
		BCM_IF_ERROR_RETURN(READ_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &subport_entry));
		soc_mem_field32_set(unit, SUBPORT_MAP_TABLEm, &subport_entry, MAP_INDEXf, cs_e_state->index);
		BCM_IF_ERROR_RETURN(WRITE_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &subport_entry));
		break;
	    case SIRIUS_ES_LEVEL_CHANNEL:
		if ( (child_node < 0) || (child_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 2 child node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2 - 1), unit));
		    return BCM_E_PARAM;
		}

		if ( (parent_node < 0) || (parent_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 3 parent node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), parent_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3 - 1), unit));
		    return BCM_E_PARAM;
		}

		index = -1;
		rv = soc_sirius_es_node_map_index_first_available(unit, parent_level, parent_node, &index);
		if (rv != SOC_E_NONE) {
		    return rv;
		}

		rv = soc_sirius_es_node_hierachy_config(unit, child_level, child_node, TRUE, parent_node,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE);

		if (rv != SOC_E_NONE) {
		    break;
		}

		ps_e_state = &es_state[unit][parent_level][parent_node];
		cs_e_state = &es_state[unit][child_level][child_node];
		
		cs_e_state->intf = parent_node;
		cs_e_state->index = index;
		ps_e_state->num_child++;

		BCM_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &channel_entry));
		soc_mem_field32_set(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf, cs_e_state->index);
		BCM_IF_ERROR_RETURN(WRITE_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &channel_entry));
		break;
	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, egress scheduler level 3 to 4 is fix mapped, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_INTERNAL;
	}

    } else {
	/* level 0, must attach lowest child node first, then can attach in any order
	 * other levels, must attach the node right next to first or last child
	 */
	ps_i_state = &is_state[unit][parent_level][parent_node];
	cs_i_state = &is_state[unit][child_level][child_node];

	if ( (child_level == 0) && (SOC_SBX_CFG_SIRIUS(unit)->b8kNodes == FALSE) ) {
	    max_num_child = 4;
	} else {
	    max_num_child = 8;
	}
	/* update states */
	if (ps_i_state->num_child == 0) {
	    ps_i_state->first_child = child_node;
	} else if (ps_i_state->num_child >= max_num_child) {
	    /* attached more than 4 or 8 children to a parent */
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Attached more than %d children to a parent node,  Unit(%d)\n"),
	               FUNCTION_NAME(), max_num_child, unit));
	    return BCM_E_PARAM;
	} else if ( (child_level != 0) && 
		    (child_node != (ps_i_state->first_child + ps_i_state->num_child)) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, child level %d node %d not expected last child %d of parent node, Unit(%d)\n"),
	               FUNCTION_NAME(), child_level, child_node, (ps_i_state->first_child + ps_i_state->num_child), unit));
	    return BCM_E_PARAM;
	} else if (child_level == 0) {
	    /* following check assuming level 0 has to attach lowest node first */
	    if ( (child_node >= (ps_i_state->first_child + max_num_child)) ||
		 (child_node <= ps_i_state->first_child)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, child level %d node %d not in valid change range of parent node, Unit(%d)\n"),
		           FUNCTION_NAME(), child_level, child_node, unit));
		return BCM_E_PARAM;
	    }
	}
	ps_i_state->num_child++;
	cs_i_state->parent = parent_node;

	/* connect child with parent */
	if (child_level == 0) {
	    /* attaching queue to leaf node */
	    q_state =  &(SOC_SBX_STATE(unit)->queue_state[queue]);
	    bw_state = &SOC_SBX_STATE(unit)->bw_group_state[q_state->bw_group];

	    /* queue to leaf node table, multicast bit is left alone */
	    rv = soc_sirius_qs_queue_to_leaf_node_set(unit, queue,
						      (bw_state->dest_type==BCM_INT_SBX_DEST_TYPE_MULTICAST)?1:0,
						      child_node);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, map queue to leaf node failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }

	    /* leaf node to queue table */
	    rv = soc_sirius_qs_leaf_node_to_queue_set(unit, child_node, queue);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, map leaf node to queue failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }

	    /* configure the leaf node queue destination */
	    if (bw_state->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
		/* anything greater than 264 will be treated as multicast
		 * by TS
		 */
		destination = SIRIUS_TS_LOCAL_SYSPORT_MC_OFFSET;
	    } else {
		if (q_state->sysport == BCM_INT_SBX_INVALID_SYSPORT) {
		    destination = bw_state->dest_port;
		} else {
		    destination = q_state->sysport - SIRIUS_TS_LOCAL_SYSPORT_BASE;
		}
	    }

	    rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, child_node, destination);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, set leaf node destination failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }
	} else {
	    /* simply connecting up the schedulers */
	    /* lx_n2 table of child to parent */
	    rv = soc_sirius_ts_node_hierachy_config(unit, child_level, child_node, parent_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						    SOC_SIRIUS_API_PARAM_NO_CHANGE);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, can not attach level %d children node %d to the parent node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), child_level, child_node, parent_node, unit));
		return rv;
	    }

	    /* lx_n2 table of parent to child */
	    rv = soc_sirius_ts_node_hierachy_config(unit, parent_level, parent_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						    ps_i_state->first_child, ps_i_state->num_child);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, can not attach level %d parent node %d to the child node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), parent_level, parent_node, child_node, unit));
		return rv;
	    }
	}
    }

    if (rv == BCM_E_NONE) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "connected %s level %d node %d to level %d node %d.\n"),
	             egress?"ES":"TS", child_level, child_node, parent_level, parent_node));
    }

    return rv;
}

/*
 *  Since this function can, under correct circumstances, swap a to-be-deleted
 *  child with the last child (a hardware requirement exists that demands all
 *  children be contiguous), it returns the newly disconnected child.  This is
 *  obligatory to be able to free or otherwise recycle the correct child.
 */
int
bcm_sirius_cosq_scheduler_disconnect(int unit, int parent_level, int parent_node, int child_level, int child_node,
				     int queue, int egress, int *discon_child)
{
    int rv = BCM_E_NONE;
    bcm_sbx_sirius_ingress_scheduler_state_t *ps_i_state;
    bcm_sbx_sirius_ingress_scheduler_state_t *cs_i_state;
    bcm_sbx_sirius_egress_scheduler_state_t *ps_e_state;
    bcm_sbx_sirius_egress_scheduler_state_t *cs_e_state;
    channel_map_table_entry_t channel_entry;
    subport_map_table_entry_t subport_entry;
    fifo_map_table_entry_t fifo_entry;
    int max_num_child;
    int last_child;

    BCM_SIRIUS_SCHEDULER_STATE_INIT_VERIFY(unit);

    if (!discon_child) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s discon_child argument must not be NULL\n"),
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    if (egress) {
	if ( (parent_level <= 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS) ||
	     (child_level < 0) || (child_level >= (SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS - 1)) ) {
	    return BCM_E_INTERNAL;
	}
    } else {
	if ( (parent_level < 0) || (parent_level >= SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS) ||
	     (child_level < 0) || (child_level >= (SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS - 1)) ||
	     (parent_node < 0) || (parent_node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[parent_level]) ||
	     (child_node < 0) || (child_node >= SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level]) ) {
	    return BCM_E_INTERNAL;
	}
    }

    /* assume disconnected child will be the specified node */
    *discon_child = child_node;

    if (egress) {
	if (parent_level != (child_level + 1)) {
	    return BCM_E_PARAM;
	}
	switch (child_level) {
	    case SIRIUS_ES_LEVEL_FIFO:
		if ( (child_node < 0) || (child_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 0 child node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0 - 1), unit));
		    return BCM_E_PARAM;
		}

		if ( (parent_node < 0) || (parent_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 0 parent node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), parent_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 - 1), unit));
		    return BCM_E_PARAM;
		}

		/* disable the node and point to last subport node */
		rv = soc_sirius_es_node_hierachy_config(unit, child_level, child_node, FALSE, 0, 0,
							(SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1-1), 0);

		if (rv != SOC_E_NONE) {
		    break;
		}

		/* update software state */
		ps_e_state = &es_state[unit][parent_level][parent_node];
		cs_e_state = &es_state[unit][child_level][child_node];
		
		cs_e_state->intf = -1;
		cs_e_state->channel = -1;
		cs_e_state->subport = -1;
		cs_e_state->index = -1;
		ps_e_state->num_child--;

		/* clear map index */
		BCM_IF_ERROR_RETURN(READ_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &fifo_entry));
		soc_mem_field32_set(unit, FIFO_MAP_TABLEm, &fifo_entry, MAP_INDEXf, 0);
		BCM_IF_ERROR_RETURN(WRITE_FIFO_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &fifo_entry));

		break;
	    case SIRIUS_ES_LEVEL_SUBPORT:
		if ( (child_node < 0) || (child_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 1 child node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 - 1), unit));
		    return BCM_E_PARAM;
		}

		if ( (parent_node < 0) || (parent_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 2 parent node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), parent_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2 - 1), unit));
		    return BCM_E_PARAM;
		}

		/* disable the node and point to last subport node */
		rv = soc_sirius_es_node_hierachy_config(unit, child_level, child_node, FALSE, 0,
							0, SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE);
		if (rv != SOC_E_NONE) {
		    break;
		}

		/* update software state */
		ps_e_state = &es_state[unit][parent_level][parent_node];
		cs_e_state = &es_state[unit][child_level][child_node];
		
		cs_e_state->intf = -1;
		cs_e_state->channel = -1;
		cs_e_state->subport = -1;
		cs_e_state->index = -1;
		ps_e_state->num_child--;

		/* clear map index */
		BCM_IF_ERROR_RETURN(READ_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &subport_entry));
		soc_mem_field32_set(unit, SUBPORT_MAP_TABLEm, &subport_entry, MAP_INDEXf, 0);
		BCM_IF_ERROR_RETURN(WRITE_SUBPORT_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &subport_entry));
		break;
	    case SIRIUS_ES_LEVEL_CHANNEL:
		if ( (child_node < 0) || (child_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 2 child node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2 - 1), unit));
		    return BCM_E_PARAM;
		}

		if ( (parent_node < 0) || (parent_node >= SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, egress scheduler level 3 parent node %d out of range [0-%d] Unit(%d)\n"),
		               FUNCTION_NAME(), parent_node, (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3 - 1), unit));
		    return BCM_E_PARAM;
		}

		/* disable the node and point to last subport node */
		rv = soc_sirius_es_node_hierachy_config(unit, child_level, child_node, FALSE, 0,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE,
							SOC_SIRIUS_API_PARAM_NO_CHANGE);

		if (rv != SOC_E_NONE) {
		    break;
		}

		/* update software state */
		ps_e_state = &es_state[unit][parent_level][parent_node];
		cs_e_state = &es_state[unit][child_level][child_node];
		
		cs_e_state->intf = -1;
		cs_e_state->index = -1;
		ps_e_state->num_child--;

		/* clear map index */
		BCM_IF_ERROR_RETURN(READ_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &channel_entry));
		soc_mem_field32_set(unit, CHANNEL_MAP_TABLEm, &channel_entry, MAP_INDEXf, 0);
		BCM_IF_ERROR_RETURN(WRITE_CHANNEL_MAP_TABLEm(unit, MEM_BLOCK_ANY, child_node, &channel_entry));
		break;
	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, egress scheduler level 3 to 4 is fix mapped, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return BCM_E_INTERNAL;
	}
	return rv;
    } else {
	/* all levels can detach in any order */
	ps_i_state = &is_state[unit][parent_level][parent_node];
	cs_i_state = &is_state[unit][child_level][child_node];

	/* connect child with parent */
	if (child_level == 0) {
	    /* check node range */
	    if (SOC_SBX_CFG_SIRIUS(unit)->b8kNodes == FALSE) {
		max_num_child = 4;
	    } else {
		max_num_child = 8;
	    }
	    last_child = ps_i_state->first_child + max_num_child - 1;
	    if ( (child_node < ps_i_state->first_child) ||
		 (child_node > last_child) ) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, child node %d not the child of the parent node %d. range [%d-%d],  Unit(%d)\n"),
		           FUNCTION_NAME(), child_node, parent_node, ps_i_state->first_child, last_child, unit));
		return BCM_E_PARAM;
	    }

	    /* detaching queue to leaf node */
	    /* queue to leaf node table, multicast bit is left alone */
	    rv = soc_sirius_qs_queue_to_leaf_node_set(unit, queue, 0, SIRIUS_Q_BASE_INVALID);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, map queue to leaf node failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }

	    /* leaf node to queue table */
	    rv = soc_sirius_qs_leaf_node_to_queue_set(unit, child_node, 0xFFFC | (child_node & 3));
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, map leaf node to queue failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }

	    /* leaf node queue destination */
	    rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, child_node, 0);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, set leaf node destination failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }

	    /* update state */
	    ps_i_state->num_child--;
	    cs_i_state->parent = -1;
	    if (ps_i_state->num_child == 0) {
		ps_i_state->first_child = -1;
	    }
	} else {
	    /* check node range */
	    last_child = ps_i_state->first_child + ps_i_state->num_child - 1;

	    /* user node management? */
	    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
		if (child_node != last_child) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, child node %d not the last child %d of the parent node %d.,  Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, last_child, parent_node, unit));
		    return BCM_E_PARAM;		    
		}
	    } else {
		if ( (child_node < ps_i_state->first_child) ||
		     (child_node > last_child) ) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, child node %d not the child of the parent node %d. range [%d-%d],  Unit(%d)\n"),
		               FUNCTION_NAME(), child_node, parent_node, ps_i_state->first_child, last_child, unit));
		    return BCM_E_PARAM;
		}
	    }

	    /* detaching intermediate nodes  */
	    if (child_node != last_child) {
		/* swap the last child to the child_node, the function will handle all hardware/software state */
		rv = bcm_sirius_cosq_sibling_node_swap(unit, child_level, child_node, last_child, FALSE);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to swap ingress scheduler level %d node %d to node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), child_level, child_node, last_child, unit));
		    return rv;
		}

		rv = soc_sirius_ts_child_weight_update(unit, parent_level, parent_node, 
						       ps_i_state->num_child, 2,
						       child_node - ps_i_state->first_child,
						       last_child - ps_i_state->first_child);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to swap child weight of ingress scheduler level %d node %d, Unit(%d)\n"),
		               FUNCTION_NAME(), parent_level, parent_node, unit));
		    return rv;
		}		

		/* disconnect the last child instead */
		child_node = last_child;

                /* disconnected child was changed; update it */
                *discon_child = child_node;
	    }

	    /* disconnect child from parent */
	    rv = soc_sirius_ts_node_hierachy_config(unit, child_level, child_node, SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[child_level+1]-1,
						    SOC_SIRIUS_API_PARAM_NO_CHANGE, SOC_SIRIUS_API_PARAM_NO_CHANGE);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, can not attach level %d children node %d to the parent node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), child_level, child_node, parent_node, unit));
		return rv;
	    }

	    /* update state */
	    cs_i_state = &is_state[unit][child_level][child_node];
	    ps_i_state->num_child--;
	    cs_i_state->parent = -1;
	    if (ps_i_state->num_child == 0) {
		ps_i_state->first_child = -1;
	    }

	    /* lx_n2 table of parent to child */
	    rv = soc_sirius_ts_node_hierachy_config(unit, parent_level, parent_node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						    (ps_i_state->first_child < 0) ? 0 : ps_i_state->first_child,
						    ps_i_state->num_child);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, can not detach level %d child node %d from the parent node %d,  Unit(%d)\n"),
		           FUNCTION_NAME(), child_level, child_node, parent_node, unit));
		return rv;
	    }
	}
    }

    if (rv == BCM_E_NONE) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "disconnected %s level %d node %d from level %d node %d.\n"),
	             egress?"ES":"TS", child_level, child_node, parent_level, parent_node));
    }

    return rv;
}


/* 
 * Attach an Ingress queue to an Egress fifo
 * IN:
 *
 * OUT:
 *     rv              - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 */
int 
bcm_sirius_cosq_gport_queue_attach(int unit, 
				   uint32 flags, 
				   bcm_gport_t ingress_queue, 
				   bcm_cos_t ingress_int_pri, 
				   bcm_gport_t egress_queue, 
				   bcm_cos_t egress_int_pri, 
				   int *attach_id)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_subport_info_t *eSp_info = NULL;
    uint16 base_queue = 0, queue = 0;
    int sysport = 0, base_node = 0;
    int fcd = 0;
    int eSubport = 0, eEg_n = 0, eNum_fifos = 0;
    int modid = -1, mymodid = -1, node = -1, port = -1;
    int fct = 0;
    sbBool_t ef = 0;
    int mc = 0;
    int eset, internal_eset;
    int32 sched_mode, sp_priority;
    int32 queue_type, priority, hungry_priority;
    uint32 regval;
    int32 hold_ts; /* CSP 341538 */
    bcm_sbx_cosq_queue_region_type_t queue_region;

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
        if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
            if (!(soc_feature(unit, soc_feature_egr_multicast_independent_fc))) {
                return(BCM_E_UNAVAIL);
            }
        }

        BCM_INT_SBX_COSQ_GET_ESET_FROM_NODE_PORT(unit, p_bwstate->dest_node, p_bwstate->dest_port, p_bwstate->dest_mc, &eset);

        if ( (egress_int_pri == BCM_COS_MULTICAST_EF) || (egress_int_pri == BCM_COS_MULTICAST_NON_EF) ) {
            if (egress_int_pri == BCM_COS_MULTICAST_EF) {
                internal_eset = BCM_INT_SBX_COSQ_ESET_TO_EF_ESET(unit, eset);
            }
            else {
                internal_eset = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, eset);
            }
        }
        else if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
            /* origial behaviour */
            internal_eset = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, eset);
        }
        else {
            internal_eset = BCM_INT_SBX_COSQ_ESET_TO_COS_ESET(unit, eset, egress_int_pri);
            rv = bcm_sbx_cosq_eset_to_cos_eset_fcd(unit, eset, egress_int_pri, &internal_eset, &fcd);
            if (rv != BCM_E_NONE) {
                return(rv);
            }
            
            /* consistency check - warning logged */
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl) {
                if ( ( (p_qstate->ingress.bw_mode == BCM_COSQ_EF) && (fcd != 0) ) ||
                     ( (p_qstate->ingress.bw_mode != BCM_COSQ_EF) && (fcd != 1) ) ) {
		    LOG_WARN(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Multicast queue attach FCD mismatch, Unit(%d) IngrGport:cos(0x%x:%d) EgrGport:cos(0x%x:%d)\n"),
		              unit, ingress_queue, ingress_int_pri, egress_queue, egress_int_pri));
                }
            }
        }

        BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_ESET(unit, internal_eset, &node, &port, (int *)&mc);
        if (mc == 1) {
            sysport = internal_eset;
        }
        else {
            BCM_INT_SBX_COSQ_GET_SYSPORT_FROM_NODE_PORT(unit, node, port, mc, &sysport);
        }

	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	    rv = soc_sirius_qs_queue_to_leaf_node_get(unit, queue, &mc, &base_node);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: Q2SC read failed\n")));
		return BCM_E_MEMORY;
	    }
	    
	    rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, base_node, sysport - SIRIUS_TS_LOCAL_SYSPORT_BASE);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, set leaf node destination failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }
	} else {
	    /* FIC queues, update sysport and cos in Q2SC and S2Q tables */
	    rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, sysport, ingress_int_pri);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s write to Q2SC table failed for queue(%d)\n"),
		           FUNCTION_NAME(), queue));
		return(rv);
	    }

	    rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, p_bwstate->base_queue);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
		           FUNCTION_NAME(), queue));
		return(rv);
	    }
	}

        return(rv);
    }

    if (BCM_GPORT_IS_EGRESS_MODPORT(egress_queue)) {
	modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(egress_queue);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(egress_queue)) {
	modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(egress_queue);
    } else if BCM_GPORT_IS_EGRESS_GROUP(egress_queue) {
	modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(egress_queue);
    } else {
	modid = 0;
    }

    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));

    sysport = ATTACH_ID_SYSPORT_GET(*attach_id);
    fcd = ATTACH_ID_FCD_GET(*attach_id);

    /* Don't redundantly overwrite tables with same information */
    if ((modid == mymodid) && (SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 1)) {
	eSubport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, egress_queue, &eSubport, &eEg_n, &eNum_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, original_subport 0x%x does not contain fabric_port, unit %d\n"),
	               FUNCTION_NAME(), egress_queue, unit));
	    return BCM_E_PARAM;
	}
	eSp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[eSubport]);
	
	port = eSp_info->egroup[eEg_n].es_scheduler_level0_node + egress_int_pri;
	
	rv = soc_sirius_fd_unicast_gmt_set(unit, sysport, port/SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE);

	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s [%d]:unable to write to unit %d:gmt for sysport %d destination port(%d)\n"),
	               FUNCTION_NAME(),
	               __LINE__, 
	               unit,
	               sysport,
	               port));
	}
	
	/*
	 * Update the Fifo Class Table
	 */	
	ef = ((p_qstate->ingress.bw_mode == BCM_COSQ_EF) ? 1 : 0);

	/* This code handles the case where there are 2 egress FIFOs assocated */
	/* with a single sysport.  When there are scheduling disciplines which */
	/* are SP to one FIFO and BAG related to the other FIFO, the EF bit    */
	/* must be set in the header.  The scheduling discipline needs to be   */
	/* set before doing the queue_attach().                                */
	/* if we already have a sysport to FIFO mapping for this sysport, that */
	/* means this is the second FIFO.                                      */
	if (SOC_SBX_STATE(unit)->sysport_state[sysport].fifo == -1) {
	    /* This is the first FIFO for the given sysport.... */
	    sched_mode = p_qstate->ingress.bw_mode;
	    sp_priority = p_qstate->ingress.bw_value.sp_priority;
	    
	    rv = soc_sbx_sched_get_internal_state_queue_attach(unit, sched_mode, sp_priority,
							       &queue_type, &priority, &hungry_priority);
	    
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: bcm_cosq_gport_sched_set() must be called, possible error,  Unit(%d)\n"),
		           unit));	    
		return (BCM_E_PARAM);
	    }
	    
	    /* If not in the bag and not best effort, then we should remap this  */
	    /* queue to simulate EF to direct to the appropriate FIFO on egress. */
	    if (queue_type > 1) {
		/* change the EF flag */
		ef = 1;
		SOC_IF_ERROR_RETURN(READ_GGI_CONFIG2r(unit, &regval));
		soc_reg_field_set(unit, GGI_CONFIG2r, &regval, EF_GRANT_SQUELCHf, 1 << priority);
		SOC_IF_ERROR_RETURN(WRITE_GGI_CONFIG2r(unit, regval));	    
	    }
	}

	fct = egress_int_pri + eSp_info->egroup[eEg_n].es_scheduler_level0_node;
	
	rv = soc_sirius_fd_fct_set(unit, sysport, ef, mc, fct);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s [%d]:unable to write to unit %d:gmt for sysport %d destination port(%d)\n"),
	               FUNCTION_NAME(),
	               __LINE__, 
	               unit,
	               sysport,
	               port));
	}

        BCM_IF_ERROR_RETURN(soc_sirius_dmt_mem_set(unit, sysport, fcd, fcd));

        if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
            if (eSp_info->egroup[eEg_n].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] < 1001) {
                rv = soc_sirius_dt_mem_set(unit, fcd, 0, 0xc300, 0x3FFF80);
            } else {
                rv = soc_sirius_dt_mem_set(unit, fcd, 0, 0x5dc00, 0x3FFF80);
            }
        } else {
            if (eSp_info->egroup[eEg_n].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] < 1001) {
                rv = soc_sirius_dt_mem_set(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_EF, 0x6180, 0xc300);
                rv = soc_sirius_dt_mem_set(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_NEF, 0x12480, 0x3fff80);
            } else {
                rv = soc_sirius_dt_mem_set(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_EF, 0x1f400, 0x3e800);
                rv = soc_sirius_dt_mem_set(unit, fcd, SB_FAB_XCORE_COS_FIFO_UNICAST_NEF, 0x5dc00, 0x3fff80);
            }
        }
    } else if ((modid == mymodid) && ((SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 2) &&
				      (p_qstate->sysport == sysport) && 
				      (p_qstate->default_sysport == sysport))) {

	eSubport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, egress_queue, &eSubport, &eEg_n, &eNum_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, original_subport 0x%x does not contain fabric_port, unit %d\n"),
	               FUNCTION_NAME(), egress_queue, unit));
	    return BCM_E_PARAM;
	}
	eSp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[eSubport]);
	
	port = eSp_info->egroup[eEg_n].es_scheduler_level0_node + egress_int_pri;
	
	ef = ((p_qstate->ingress.bw_mode == BCM_COSQ_EF) ? 1 : 0);
	fct = egress_int_pri + eSp_info->egroup[eEg_n].es_scheduler_level0_node;
	
	rv = soc_sirius_fd_fct_set(unit, sysport, ef, mc, fct);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s [%d]:unable to write to unit %d:gmt for sysport %d destination port(%d)\n"),
	               FUNCTION_NAME(),
	               __LINE__, 
	               unit,
	               sysport,
	               port));
	}

	SOC_SBX_STATE(unit)->sysport_state[sysport].fcd = ATTACH_ID_FCD_GET(*attach_id);
	SOC_SBX_STATE(unit)->sysport_state[sysport].fifo = fct;
	SOC_SBX_STATE(unit)->sysport_state[sysport].egport = egress_queue;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
        rv = soc_sirius_qs_queue_to_leaf_node_get(unit, queue, NULL, &base_node);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: Q2SC read failed\n")));
            return BCM_E_MEMORY;
        }
        
        rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, base_node, sysport - SIRIUS_TS_LOCAL_SYSPORT_BASE);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, set leaf node destination failed, Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            return rv;
        }
    } else {
        
        /* Update sysport and cos in Q2SC table and S2Q table */
        rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, sysport, ingress_int_pri);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s write to Q2SC table failed for queue(%d)\n"),
                       FUNCTION_NAME(), queue));
            return rv;
        }
    }
    
    if (SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 1) {
        if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME) {
            rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, base_queue);
            if (rv) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
                           FUNCTION_NAME(), base_queue));
                return rv;
            }
        }
        
        
        rv = soc_sirius_qs_sysport_to_node_set(unit, sysport, p_bwstate->dest_node);
        if (rv) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: write to S2N table failed for queue(%d)\n"),
                       queue));
            return rv;
        }
        
        SOC_SBX_STATE(unit)->sysport_state[sysport].fcd = ATTACH_ID_FCD_GET(*attach_id);
        SOC_SBX_STATE(unit)->sysport_state[sysport].fifo = fct;
        SOC_SBX_STATE(unit)->sysport_state[sysport].egport = egress_queue;
    }
    
    p_qstate->sysport = sysport;
    p_qstate->attached_fifo = egress_int_pri;
    
    if (egress_int_pri == BCM_COS_MULTICAST_EF) {
        if ( (p_qstate->ingress.bw_mode == BCM_COSQ_GSP0) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP1) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP2) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP3) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP4) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP5) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP6) ||
             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP7) ) {
            
            _bcm_sbx_cosq_queue_type_get(unit, queue, &queue_region);
            _bcm_sirius_cosq_select_queue_params(unit, queue, queue_region,
                                                 &p_qstate->ingress,
                                                 &hold_ts, &queue_type);

            SOC_IF_ERROR_RETURN(READ_GG_EF_TYPE_DECODEr(unit, &regval));
            regval = regval | (1 << queue_type);
            SOC_IF_ERROR_RETURN(WRITE_GG_EF_TYPE_DECODEr(unit, regval));
        }
    }

    return rv;
}


/* 
 * Get Egress Fifo information from an Ingress queue
 * IN:
 *
 * OUT:
 *     rv              - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 */
int 
bcm_sirius_cosq_gport_queue_attach_get(int unit, 
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
 *     rv              - BCM_E_NONE if successfully, all other indicate failure
 * Note:
 */
int 
bcm_sirius_cosq_gport_queue_detach(int unit, 
				   bcm_gport_t ingress_queue, 
				   bcm_cos_t ingress_int_pri, 
				   int attach_id)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    uint16 base_queue = 0, queue = 0;
    int sysport = 0, base_node = 0;
    int subport = 0, eg_n = 0, num_fifos = 0;
    int modid = -1, mymodid = -1, node = -1, port = -1;
    sbBool_t ef = 0;
    int mc = 0;
    int eset, internal_eset;

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
        if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
            if (!(soc_feature(unit, soc_feature_egr_multicast_independent_fc))) {
                return(BCM_E_UNAVAIL);
            }
        }

        BCM_INT_SBX_COSQ_GET_ESET_FROM_NODE_PORT(unit, p_bwstate->dest_node, p_bwstate->dest_port, p_bwstate->dest_mc, &eset);

        if (soc_feature(unit, soc_feature_egr_multicast_independent_fc)) {
            if (p_qstate->ingress.bw_mode == BCM_COSQ_EF) {
                internal_eset = BCM_INT_SBX_COSQ_ESET_TO_EF_ESET(unit, eset);
            }
            else {
                internal_eset = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, eset);
            }
            BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_ESET(unit, internal_eset, &node, &port, (int *)&mc);
            if (mc == 1) {
                sysport = internal_eset;
            }
            else {
                BCM_INT_SBX_COSQ_GET_SYSPORT_FROM_NODE_PORT(unit, node, port, mc, &sysport);
            }
        }


	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	    rv = soc_sirius_qs_queue_to_leaf_node_get(unit, queue, &mc, &base_node);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: Q2SC read failed\n")));
		return BCM_E_MEMORY;
	    }
	    
	    rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, base_node, 
						       p_qstate->default_sysport - SIRIUS_TS_LOCAL_SYSPORT_BASE);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, set leaf node destination failed, Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return rv;
	    }
	} else {
	    /* FIC queues, update sysport and cos in Q2SC and S2Q tables */
	    rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, sysport, ingress_int_pri);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s write to Q2SC table failed for queue(%d)\n"),
		           FUNCTION_NAME(), queue));
		return(rv);
	    }
	    
	    rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, p_bwstate->base_queue);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, write to S2Q table failed for queue(%d)\n"),
		           FUNCTION_NAME(), queue));
		return(rv);
	    }
	}

        return(rv);
    }

    if (bcm_stk_modid_get(unit, &mymodid) != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, cannot retrieve system modid on unit %d\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_INTERNAL;
    }

    if (BCM_GPORT_IS_MODPORT(p_bwstate->gport)) {
	modid = BCM_GPORT_MODPORT_MODID_GET(p_bwstate->gport);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(p_bwstate->gport)) {
	modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(p_bwstate->gport);
    } else if (BCM_GPORT_IS_CHILD(p_bwstate->gport)) {
	modid = BCM_GPORT_CHILD_MODID_GET(p_bwstate->gport);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(p_bwstate->gport)) {
	modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(p_bwstate->gport);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(p_bwstate->gport)) {
	modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(p_bwstate->gport);
    } else {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Unexpected gport from queue %d, 0x%x\n"),
	           FUNCTION_NAME(), queue, p_bwstate->gport));
	return BCM_E_INTERNAL;
    }

    sysport = ATTACH_ID_SYSPORT_GET(attach_id);

    /* Only erase tables when last sysport configured */
    if ((modid == mymodid) && (SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 1)) {
	subport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, p_bwstate->gport, &subport, &eg_n, &num_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
	               FUNCTION_NAME(), p_bwstate->gport, unit));
	    return BCM_E_PARAM;
	}    
      
	rv = soc_sirius_fd_unicast_gmt_set(unit, sysport, 0);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s [%d]:unable to write to unit %d:gmt for sysport %d\n"),
	               FUNCTION_NAME(),
	               __LINE__, 
	               unit,
	               sysport));
	}
	
	/*
	 * Update the Fifo Class Table
	 */
	
	ef = ((p_qstate->ingress.bw_mode == BCM_COSQ_EF) ? 1 : 0);
	
	rv = soc_sirius_fd_fct_set(unit, sysport, ef, mc, (ef) ? 0 : 1);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s [%d]:unable to write to unit %d:gmt for sysport %d\n"),
	               FUNCTION_NAME(),
	               __LINE__, 
	               unit,
	               sysport));
	}
	
	/*
	 * Update the flow control domain
	 */        
	
	if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
            BCM_IF_ERROR_RETURN(soc_sirius_dmt_mem_set(unit, sysport, SIRIUS_FR_FC_MC_DOMAIN_NON_EF - 2, SIRIUS_FR_FC_MC_DOMAIN_EF - 2));
	} else {
            BCM_IF_ERROR_RETURN(soc_sirius_dmt_mem_set(unit, sysport, SIRIUS_FR_FC_MC_DOMAIN - 2, SIRIUS_FR_FC_MC_DOMAIN - 2));
	}

	if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
            BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_set(unit, SOC_SBX_STATE(unit)->sysport_state[sysport].fcd, 0, 0xc300, 0x3FFF80));
	} else {
            BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_set(unit, 
                                                      SOC_SBX_STATE(unit)->sysport_state[sysport].fcd, 
                                                      SB_FAB_XCORE_COS_FIFO_UNICAST_EF,
                                                      0x6180, 0xc300));
            BCM_IF_ERROR_RETURN(soc_sirius_dt_mem_set(unit, 
                                                      SOC_SBX_STATE(unit)->sysport_state[sysport].fcd, 
                                                      SB_FAB_XCORE_COS_FIFO_UNICAST_NEF,
                                                      0x5dc00, 0x3fff80));
	}
    } else if ((modid == mymodid) && ((SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 2) &&
				      (p_qstate->sysport == sysport) && 
				      (p_qstate->default_sysport == sysport))) {

	subport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, p_bwstate->gport, &subport, &eg_n, &num_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
	               FUNCTION_NAME(), p_bwstate->gport, unit));
	    return BCM_E_PARAM;
	}    
      
	/*
	 * Update the Fifo Class Table
	 */
	
	ef = ((p_qstate->ingress.bw_mode == BCM_COSQ_EF) ? 1 : 0);
	
	rv = soc_sirius_fd_fct_set(unit, sysport, ef, mc, (ef) ? 0 : 1);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s [%d]:unable to write to unit %d:gmt for sysport %d\n"),
	               FUNCTION_NAME(),
	               __LINE__, 
	               unit,
	               sysport));
	}

	SOC_SBX_STATE(unit)->sysport_state[sysport].fcd = BCM_INT_SBX_INVALID_FCD;
	SOC_SBX_STATE(unit)->sysport_state[sysport].fifo = -1;
	SOC_SBX_STATE(unit)->sysport_state[sysport].egport = BCM_GPORT_INVALID;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	rv = soc_sirius_qs_queue_to_leaf_node_get(unit, queue, NULL, &base_node);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: Q2SC read failed\n")));
	    return BCM_E_MEMORY;
	}

	rv = soc_sirius_ts_leaf_node_dest_fifo_set(unit, base_node, 
						   p_qstate->default_sysport - SIRIUS_TS_LOCAL_SYSPORT_BASE);
	if (rv != SOC_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, set leaf node destination failed, Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return rv;
	}
    } else {
    
	/* Update sysport and cos in Q2SC and S2Q tables */
	rv = soc_sirius_qs_queue_to_sysport_cos_set(unit, queue, mc, p_qstate->default_sysport, ingress_int_pri);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s write to Q2SC table failed for queue(%d)\n"),
	               FUNCTION_NAME(), queue));
	    return rv;
	}
    }

    if (SOC_SBX_STATE(unit)->sysport_state[sysport].use_cnt == 1) {
	if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME) {
	    rv = soc_sirius_qs_sysport_to_queue_set(unit, mc, sysport, SIRIUS_Q_BASE_INVALID);
	    if (rv) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s write to S2Q table failed for sysport(%d)\n"),
		           FUNCTION_NAME(), sysport));
		return rv;
	    }
	}

	rv = soc_sirius_qs_sysport_to_node_set(unit, sysport, 0xff);
	if (rv) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: write to S2N table failed for queue(%d)\n"),
	               queue));
	    return rv;
	}

	SOC_SBX_STATE(unit)->sysport_state[sysport].fcd = BCM_INT_SBX_INVALID_FCD;
	SOC_SBX_STATE(unit)->sysport_state[sysport].fifo = -1;
	SOC_SBX_STATE(unit)->sysport_state[sysport].egport = BCM_GPORT_INVALID;
    }

    p_qstate->sysport = p_qstate->default_sysport;
    p_qstate->attached_fifo = -1;
    return rv;
}

static void _bcm_sirius_rate_delta_max_index_for_port_rate_chosen_mode0(uint32 bag_rate_kbps,
                                                                        int32 *prate_delta_max_index)
{
    uint32 uPercentOffset = 2; /* 17 percent */
    /*
     * The rate delta max table was set up as follows in sirius.c
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
    } else {
        *prate_delta_max_index = (15<<2) + uPercentOffset;
    }
}

static void _bcm_sirius_rate_delta_max_index_for_port_rate_chosen_mode1(uint32 bag_rate_kbps,
                                                                        int32 *prate_delta_max_index)
{
    uint32 uPercentOffset = 2; /* 17 percent */
    /*
     * The rate delta max table was set up as follows in sirius.c
     *
     * Entry RATE            10%     13%     17%     20%
     * 0      5M             648     842    1101    1259
     * 1      7M             906    1179    1541    1813
     * 2     10M            1296    1684    2202    2591
     * 3     20M            2591    3368    4405    5182
     * 4     50M            6487    8421   11013   12596
     * 5     70M            9069   11790   15418   18139
     * 6     100M          12956   16843   22026   25913
     * 7     200M          25913   33687   44053   51827
     * 8     500M          64873   84219  110132  125967
     * 9     700M          90697  117906  154185  181395
     * 10    1G           129567  168438  220265  259135
     * 11    2G           259134  336876  440530  518270
     * 12    5G           647839  842191 1101327 1295679
     * 13    10G         1295679 1684382 2202654 2591358
     * 14    20G         2591358 3368764 4405308 5182716
     * 15    25gb       5469108 7109838  9297483 10938215      
     * Set rate to 13% as an initial value - determine optimal after further testing
     */
    
    if (bag_rate_kbps < 5000) {
        *prate_delta_max_index = (0<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 7000) {
        *prate_delta_max_index = (1<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 10000) {
        *prate_delta_max_index = (2<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 20000) {
        *prate_delta_max_index = (3<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 50000) {
        *prate_delta_max_index = (4<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 70000) {
        *prate_delta_max_index = (5<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 100000) {
        *prate_delta_max_index = (6<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 200000) {
        *prate_delta_max_index = (7<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 500000) {
        *prate_delta_max_index = (8<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 700000) {
        *prate_delta_max_index = (9<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 1000000) {
        *prate_delta_max_index = (10<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 2000000) {
        *prate_delta_max_index = (11<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 5000000) {
        *prate_delta_max_index = (12<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 20000000) {
        *prate_delta_max_index = (13<<2) + uPercentOffset;
    } else if (bag_rate_kbps < 25000000) {
        *prate_delta_max_index = (14<<2) + uPercentOffset;
    } else  {
        *prate_delta_max_index = (15<<2) + uPercentOffset;
    }
}

static void _bcm_sirius_rate_delta_max_index_for_port_rate_chosen(int unit, uint32 bag_rate_kbps,
                                                                  int32 *prate_delta_max_index)
{
    int rate_delta_mode = 0;

    rate_delta_mode = soc_property_get(unit, "sirius_rate_delta_mode", rate_delta_mode);

    if (rate_delta_mode == 1) {

        _bcm_sirius_rate_delta_max_index_for_port_rate_chosen_mode1(bag_rate_kbps,
                                                                    prate_delta_max_index);
    } else {

        _bcm_sirius_rate_delta_max_index_for_port_rate_chosen_mode0(bag_rate_kbps,
                                                                    prate_delta_max_index);
    }
    return;
}

static int
_bcm_sirius_cosq_select_queue_params(int unit, int queue, int queue_region, 
				     bcm_sbx_cosq_queue_params_ingress_t *p_qparams,
				     int32 *p_hold_ts, int32 *p_qtype)
{
    int32 bw_mode;
    int32 sp_priority;
    uint32 uPriority;
    uint16 dev_id;
    uint8  rev_id;
    int32 disable_starving = FALSE;
    int32 low_cir_queue = FALSE;
    int32 cir_threshold = 600000; /* switch at 600Mbps */

    soc_cm_get_id(unit, &dev_id, &rev_id);
    if ((rev_id == BCM88230_A0_REV_ID) ||
	(rev_id == BCM88230_B0_REV_ID)) {
	disable_starving = TRUE;
    }
    if (p_qparams->bw_value.guarantee_kbps < cir_threshold) {
	low_cir_queue = TRUE;
    } else {
	low_cir_queue = FALSE;
    }

    bw_mode = p_qparams->bw_mode;
    sp_priority = p_qparams->bw_value.sp_priority;

    if (queue_region == bcm_sbx_cosq_queue_region_local) {
	/* local queues */

	if (SOC_SBX_CFG(unit)->local_template_id == SOC_SBX_QOS_TEMPLATE_TYPE0) {
	    switch ( bw_mode ) {
		case BCM_COSQ_BE :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_BE;
		    break;
		case BCM_COSQ_EF :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_EF;
		    break;
		case BCM_COSQ_SP0:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP0;
		    break;
		case BCM_COSQ_SP1:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP1;
		    break;
		case BCM_COSQ_SP2:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP2;
		    break;
		case BCM_COSQ_SP3:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP3;
		    break;
		case BCM_COSQ_AF0:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP4;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF0;
		    }
		    break;
		case BCM_COSQ_AF1:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP5;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF1;
		    }
		    break;
		case BCM_COSQ_AF2:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP6;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF2;
		    }
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "Unsupported bw_mode(%d) found during AddQueue"
		                           " operation\n"), p_qparams->bw_mode));
		    return -1;
	    }
	}
        else if (SOC_SBX_CFG(unit)->local_template_id == SOC_SBX_QOS_TEMPLATE_TYPE1) {
	    switch ( bw_mode ) {
		case BCM_COSQ_BE :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_BE;
		    break;
		case BCM_COSQ_EF :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_EF;
		    break;
		case BCM_COSQ_SP0:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP0;
		    } else {
			if (disable_starving && low_cir_queue) {
			    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP2;
			} else {
			    *p_qtype = SIRIUS_Q_TYPE_LOCAL_RCPQ0;
			}
		    }
		    break;
		case BCM_COSQ_SP1:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP1;
		    } else {
			if (disable_starving && low_cir_queue) {
			    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP3;
			} else {
			    *p_qtype = SIRIUS_Q_TYPE_LOCAL_RCPQ1;
			}
		    }
		    break;
		case BCM_COSQ_AF0:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP4;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF0;
		    }
		    break;
		case BCM_COSQ_AF1:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP5;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF1;
		    }
		    break;
		case BCM_COSQ_AF2:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP6;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF2;
		    }
		    break;
		case BCM_COSQ_AF3:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP7;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF3;
		    }
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "Unsupported bw_mode(%d) found during AddQueue"
		                           " operation\n"), p_qparams->bw_mode));
		    return -1;
	    }
	}
	else if ((SOC_SBX_CFG(unit)->local_template_id == SOC_SBX_QOS_TEMPLATE_TYPE2) ||
		 (SOC_SBX_CFG(unit)->local_template_id == SOC_SBX_QOS_TEMPLATE_TYPE4)) {
	    switch (bw_mode) {
		case BCM_COSQ_BE :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_BE;
		    break;
		case BCM_COSQ_EF :
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_EF;
		    break;
		case BCM_COSQ_SP0:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP0;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ0;
		    }
		    break;
		case BCM_COSQ_SP1:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP1;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ1;
		    }
		    break;
		case BCM_COSQ_SP2:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP2;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ2;
		    }
		    break;
		case BCM_COSQ_SP3:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP3;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ3;
		    }
		    break;
		case BCM_COSQ_SP4:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP4;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ4;
		    }
		    break;
		case BCM_COSQ_SP5:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP5;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ5;
		    }
		    break;
		case BCM_COSQ_SP6:
		    if (p_qparams->bw_value.guarantee_kbps == 0) {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_SP6;
		    }
                    else {
			(*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ6;
		    }
		    break;
		case BCM_COSQ_AF0:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ0;
		    break;
		case BCM_COSQ_AF1:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ1;
		    break;
		case BCM_COSQ_AF2:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ2;
		    break;
		case BCM_COSQ_AF3:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ3;
		    break;
		case BCM_COSQ_AF4:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ4;
		    break;
		case BCM_COSQ_AF5:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ5;
		    break;
		case BCM_COSQ_AF6:
		    (*p_qtype) = SIRIUS_Q_TYPE_LOCAL_RCPQ6;
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "Unsupported bw_mode(%d) found during AddQueue"
		                           " operation\n"), p_qparams->bw_mode));
		    return(-1);
	    }
        }
	else if (SOC_SBX_CFG(unit)->local_template_id == SOC_SBX_QOS_TEMPLATE_TYPE3) {
	    switch ( bw_mode ) {
		case BCM_COSQ_BE :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_BE;
		    break;
		case BCM_COSQ_EF :
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_EF;
		    break;
		case BCM_COSQ_SP0:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP0;
		    break;
		case BCM_COSQ_SP1:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP1;
		    break;
		case BCM_COSQ_SP2:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP2;
		    break;
		case BCM_COSQ_SP3:
		    *p_qtype = SIRIUS_Q_TYPE_LOCAL_SP3;
		    break;
		case BCM_COSQ_AF0:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP4;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF0;
		    }
		    break;
		case BCM_COSQ_AF1:
		    if (disable_starving && low_cir_queue) {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_SP5;
		    } else {
			*p_qtype = SIRIUS_Q_TYPE_LOCAL_AF1;
		    }
		    break;
		default:
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "Unsupported bw_mode(%d) found during AddQueue"
		                           " operation\n"), p_qparams->bw_mode));
		    return -1;
	    }
	}
        else {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "Unsupported scheduling discipline template (%d)\n"),
                       SOC_SBX_CFG(unit)->local_template_id));
            return(-1);
        }
    } else {
	/* global queues */
	
	/* Here are the values were are configuring:                      */
	/*                                                                */
	/*      5'b00000=BE,  5'b00001=AF,  5'b00010=EF                   */
	/*      5'b01000=SP0, 5'b01001=SP1, 5'b01010=SP2, 5'b01011=SP3    */
	/*      5'b01100=SP4, 5'b01101=SP5, 5'b01110=SP6, 5'b01111=SP7    */

        if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
            if (bw_mode == BCM_COSQ_SP) {
                _bcm_sirius_cosq_map_sp2gsp(unit, queue, &bw_mode, &sp_priority);
            }
        }
	
	switch ( bw_mode ) {
	    case BCM_COSQ_SP :
		
		/* Strict Pri is in the bag so it is setup as AF inside QOS           */
		/* This is because there are 2 definitions for SP - the QOS version   */
		/* and the BAA version, in the BAA, SP is set up as SP, in QOS, set   */
		/* up as AF so that SP traffic does not exceed the CIR bandwidth of   */
		/* the BAG */
		*p_qtype = SIRIUS_Q_TYPE_AF;
		break;
	    case BCM_COSQ_SP_GLOBAL :
		
                /* Strict Pri is out of the bag and set up as SP inside QOS           */
                /* 8 corresponds to SP0, 9 to SP1, ...up to 15 for SP7                */
                /* SP0 is the lowest priority and SP7 is the highest, so swap below   */
                /* in flib, the lowest COS level is the highest priority (when in the */
                /* bag, so make this operate similarly, lowest SP is the highest pri  */
                switch(sp_priority) {
                    case 0:
                        uPriority = SIRIUS_Q_TYPE_SP7;
                        break;
                    case 1:
                        uPriority = SIRIUS_Q_TYPE_SP6;
                        break;
                    case 2:
                        uPriority = SIRIUS_Q_TYPE_SP5;
                        break;
                    case 3:
                        uPriority = SIRIUS_Q_TYPE_SP4;
                        break;
                    case 4:
                        uPriority = SIRIUS_Q_TYPE_SP3;
                        break;
                    case 5:
                        uPriority = SIRIUS_Q_TYPE_SP2;
                        break;
                    case 6:
                        uPriority = SIRIUS_Q_TYPE_SP1;
                        break;
                    case 7:
                    default:
                        uPriority = SIRIUS_Q_TYPE_SP0;
                        break;
                }
                *p_qtype = uPriority;

                LOG_DEBUG(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "bw mode PATH_SP_GLOBAL, priority set to 0x%x\n"),
                           *p_qtype));

		break;
	    case BCM_COSQ_EF :
		*p_qtype = SIRIUS_Q_TYPE_EF;
		break;
	    case BCM_COSQ_AF :
		*p_qtype = SIRIUS_Q_TYPE_AF;
		break;
	    case BCM_COSQ_BE :
		*p_qtype = SIRIUS_Q_TYPE_BE;
		break;
	    case BCM_COSQ_WEIGHTED_FAIR_QUEUING :
		*p_qtype = SIRIUS_Q_TYPE_AF; /* WFQ is treated as AF by QOS */
		break;
            case BCM_COSQ_CALENDAR :
	        *p_qtype = 0x3;
		break;

            case BCM_COSQ_GSP0:
                *p_qtype = SIRIUS_Q_TYPE_SP0;
                break;
            case BCM_COSQ_GSP1:
                *p_qtype = SIRIUS_Q_TYPE_SP1;
                break;
            case BCM_COSQ_GSP2:
                *p_qtype = SIRIUS_Q_TYPE_SP2;
                break;
            case BCM_COSQ_GSP3:
                *p_qtype = SIRIUS_Q_TYPE_SP3;
                break;
            case BCM_COSQ_GSP4:
                *p_qtype = SIRIUS_Q_TYPE_SP4;
                break;
            case BCM_COSQ_GSP5:
                *p_qtype = SIRIUS_Q_TYPE_SP5;
                break;
            case BCM_COSQ_GSP6:
                *p_qtype = SIRIUS_Q_TYPE_SP6;
                break;
            case BCM_COSQ_GSP7:
                *p_qtype = SIRIUS_Q_TYPE_SP7;
                break;

	    default:
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "Unsupported bw_mode(%d) found during AddQueue"
		                       " operation\n"), p_qparams->bw_mode));
		return -1;
	}
    }
    LOG_DEBUG(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "Scheduling Discipline: 0x%x, Queue Type: 0x%x\n"),
               bw_mode, (*p_qtype)));


    /* hold_ts (default 1 for FIC, 2 for local) */
    {
        /* compute overrides */
        if (p_qparams->hold_pri_num_timeslots == BCM_DEFAULT_INT32_VALUE){
	    if (queue_region == bcm_sbx_cosq_queue_region_local) {
		/* Hold ts us dynamic for tme mode. Ts only holdS when it needs to because the interface
		 * bandwidth is too low. Hold ts should be configd to max (7) in tme mode
		 */
		*p_hold_ts = 7;
	    } else {
		*p_hold_ts = 1;
	    }
        }else {
            /* If user specified it, then use user's value*/
            *p_hold_ts = p_qparams->hold_pri_num_timeslots;
        }
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "hold_ts=0x%x\n"),
	           *p_hold_ts));
    }

    return BCM_E_NONE;
}

int
bcm_sirius_cosq_subscriber_map_add(int unit,
			    bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_sbx_cosq_queue_state_t *q_state __attribute__((unused))= NULL;
    ep_oi2qb_map_entry_t oi2qb;
    ep_dest_port_map_entry_t ep_entry;
    uint16 queue_id = 0;
    int32 minOitt = 0;
    int32 maxOitt = 0;
    int32 fifo = 0, i = 0, idx = 0;


    if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_HYBRID) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Supported in Hybrid mode only\n")));
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Not supported in XGS mode\n")));
	return BCM_E_UNAVAIL;
    }

    if (map == NULL) {
	return BCM_E_PARAM;
    }

    queue_id = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map->queue_id);
    q_state =  &(SOC_SBX_STATE(unit)->queue_state[queue_id]);
    
    
    /*
     *  Need to know how many OITTs we're going to be using.
     */
    minOitt = (SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage) << 9;
    maxOitt = SOC_MEM_INFO(unit, EP_OI2QB_MAPm).index_max;
    if (minOitt > maxOitt) {
        return BCM_E_CONFIG;
    }
    
    sal_memset(&oi2qb, 0, sizeof(oi2qb));
    
    switch(map->flags) {
	case BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN:
	    /*
	     * QID should be in the following format:
	     * 16    14               8              3      0
	     *  | REQ |    Dest Port  | VLAN Service | FCos |
	     *
	     * Write the VLAN service and FCOS to queue base
	     */

	    soc_mem_field32_set(unit, EP_OI2QB_MAPm, &oi2qb, QUEUE_BASEf, (queue_id & 0xf8));
	    rv = WRITE_EP_OI2QB_MAPm(unit, MEM_BLOCK_ANY, map->vlan + minOitt, &oi2qb);
	    if (rv != SOC_E_NONE) {
		return(rv);
	    }

	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[map->port]);
	    if (sp_info->valid == FALSE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, invalid fabric_port %d, unit %d\n"),
		           FUNCTION_NAME(), map->port, unit));
		return BCM_E_PARAM;
	    }
	    for (idx = 0; idx < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; idx++) {
		if ( sp_info->egroup[idx].es_scheduler_level0_node > SOC_MEM_INFO(unit, FIFO_MAP_TABLEm).index_max) {
		    continue;
		}
		fifo = sp_info->egroup[idx].es_scheduler_level0_node;
	    
		/*
		 * Update Subscriber Queue destination info
		 */
		sal_memset(&ep_entry, 0, sizeof(ep_entry));
		soc_mem_field32_set(unit, EP_DEST_PORT_MAPm, &ep_entry, QUEUEf, queue_id & 0xff00);
		
		for (i=0; i < sp_info->egroup[idx].num_fifos; i++) {
		    rv = WRITE_EP_DEST_PORT_MAPm(unit, MEM_BLOCK_ANY, fifo + i, &ep_entry);
		    if (rv != SOC_E_NONE) {
			return(rv);
		    }
		}
	    }

	    break;
    case BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID:
            BCM_IF_ERROR_RETURN(soc_sirius_queue_map_block_entry_set_sdk(unit,
                                                                         _SIRIUS_I_QUEUE_MAP_SEG_SBX_RQ +
                                                                         SIRIUS_QUEUE_MAP_BLOCK_OFFSET_INGRESS,
                                                                         map->encap_id,
                                                                         _SIRIUS_I_COS_PROFILE_ENCAPID +
                                                                         SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS,
                                                                         queue_id));
	    break;
	default:
	    return BCM_E_PARAM;
    }

    return rv;
}

int
bcm_sirius_cosq_subscriber_map_delete(int unit,
			       bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_sbx_cosq_queue_state_t *q_state __attribute__((unused))= NULL;
    ep_oi2qb_map_entry_t oi2qb;
    ep_dest_port_map_entry_t ep_entry;
    uint16 queue_id = 0;
    int32 minOitt = 0;
    int32 maxOitt = 0;
    int32 fifo = 0, i = 0, idx = 0;


    if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_HYBRID) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Supported in Hybrid mode only\n")));
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Not supported in XGS mode\n")));
	return BCM_E_UNAVAIL;
    }

    if (map == NULL) {
	return BCM_E_PARAM;
    }

    queue_id = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map->queue_id);
    q_state =  &(SOC_SBX_STATE(unit)->queue_state[queue_id]);
    
    
    /*
     *  Need to know how many OITTs we're going to be using.
     */
    minOitt = (SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage) << 9;
    maxOitt = SOC_MEM_INFO(unit, EP_OI2QB_MAPm).index_max;
    if (minOitt > maxOitt) {
        return BCM_E_CONFIG;
    }
    
    sal_memset(&oi2qb, 0, sizeof(oi2qb));

    switch(map->flags) {
	case BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN:
	    soc_mem_field32_set(unit, EP_OI2QB_MAPm, &oi2qb, QUEUE_BASEf, SIRIUS_Q_BASE_INVALID);
	    rv = WRITE_EP_OI2QB_MAPm(unit, MEM_BLOCK_ANY, map->vlan + minOitt, &oi2qb);
	    if (rv != SOC_E_NONE) {
		return(rv);
	    }

	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[map->port]);
	    if (sp_info->valid == FALSE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, invalid fabric_port %d, unit %d\n"),
		           FUNCTION_NAME(), map->port, unit));
		return BCM_E_PARAM;
	    }
	    for (idx = 0; idx < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; idx++) {
		if ( sp_info->egroup[idx].es_scheduler_level0_node > SOC_MEM_INFO(unit, FIFO_MAP_TABLEm).index_max) {
		    continue;
		}
		fifo = sp_info->egroup[idx].es_scheduler_level0_node;
		
		/*
		 * Update Subscriber Queue destination info
		 */
		
		for (i = 0; i < sp_info->egroup[idx].num_fifos; i++) {
		    sal_memset(&ep_entry, 0, sizeof(ep_entry));
		    soc_mem_field32_set(unit, EP_DEST_PORT_MAPm, &ep_entry, QUEUEf, (map->port << 8));
		    rv = WRITE_EP_DEST_PORT_MAPm(unit, MEM_BLOCK_ANY, fifo + i, &ep_entry);
		    if (rv != SOC_E_NONE) {
			return(rv);
		    }
		}
	    }

	    break;
	case BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID:
            BCM_IF_ERROR_RETURN(soc_sirius_queue_map_block_entry_set_sdk(unit,
                                                                         _SIRIUS_I_QUEUE_MAP_SEG_SBX_RQ +
                                                                         SIRIUS_QUEUE_MAP_BLOCK_OFFSET_INGRESS,
                                                                         map->encap_id,
                                                                         _SIRIUS_I_COS_PROFILE_GENERAL +
                                                                         SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS,
                                                                         SIRIUS_Q_BASE_INVALID));
	    break;
	default:
	    return BCM_E_PARAM;
    }

    return rv;
}

int
bcm_sirius_cosq_subscriber_map_delete_all(int unit)
{
    int rv = BCM_E_NONE;
    ep_oi2qb_map_entry_t oi2qb;
    ep_dest_port_map_entry_t ep_entry;
    int32 minOitt = 0;
    int32 maxOitt = 0;
    int32 index = 0;


    if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_HYBRID) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Supported in Hybrid mode only\n")));
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Not supported in XGS mode\n")));
	return BCM_E_UNAVAIL;
    }

    /*
     * Reset Dest_port map table to default values
     */
    sal_memset(&ep_entry, 0, sizeof(ep_entry));
    for (index = 0; index <= SOC_MEM_INFO(unit, EP_DEST_PORT_MAPm).index_max; index++) {
        soc_mem_field32_set(unit, EP_DEST_PORT_MAPm, &ep_entry, QUEUEf, ((index / 4) << 8));
	BCM_IF_ERROR_RETURN(WRITE_EP_DEST_PORT_MAPm(unit, MEM_BLOCK_ANY, index, &ep_entry));
    }

    /*
     *  Select appropriate part of ep_oi2qb table
     */
    minOitt = (SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage) << 9;
    maxOitt = SOC_MEM_INFO(unit, EP_OI2QB_MAPm).index_max;
    if (minOitt > maxOitt) {
        return BCM_E_CONFIG;
    }

    sal_memset(&oi2qb, 0, sizeof(oi2qb));
    soc_mem_field32_set(unit, EP_OI2QB_MAPm, &oi2qb, QUEUE_BASEf, SIRIUS_Q_BASE_INVALID);

    for (index=minOitt; index <= maxOitt; index++) {
	rv = WRITE_EP_OI2QB_MAPm(unit, MEM_BLOCK_ANY, index, &oi2qb);

	if (rv != SOC_E_NONE) {
	    return(rv);
	}
    }
    for (index = 0; index < 65536; index++) {
        BCM_IF_ERROR_RETURN(soc_sirius_queue_map_block_entry_set_sdk(unit,
                                                                     _SIRIUS_I_QUEUE_MAP_SEG_SBX_RQ +
                                                                     SIRIUS_QUEUE_MAP_BLOCK_OFFSET_INGRESS,
                                                                     index,
                                                                     _SIRIUS_I_COS_PROFILE_GENERAL +
                                                                     SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS,
                                                                     SIRIUS_Q_BASE_INVALID));
    }

    return rv;
}

int
bcm_sirius_cosq_subscriber_map_get(int unit,
			    bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;
    ep_oi2qb_map_entry_t oi2qb;
    ep_dest_port_map_entry_t ep_entry;
    uint16 queue_id = 0;
    int32 minOitt = 0;
    int32 maxOitt = 0;
    unsigned int queue;
    unsigned int cosmap;

    if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_HYBRID) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Supported in Hybrid mode only\n")));
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Not supported in XGS mode\n")));
	return BCM_E_UNAVAIL;
    }

    if (map == NULL) {
	return BCM_E_PARAM;
    }

    /*
     *  Need to know how many OITTs we're going to be using.
     */
    minOitt = (SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage) << 9;
    maxOitt = SOC_MEM_INFO(unit, EP_OI2QB_MAPm).index_max;
    if (minOitt > maxOitt) {
        return BCM_E_CONFIG;
    }

    sal_memset(&oi2qb, 0, sizeof(oi2qb));
    
    switch(map->flags) {
	case BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN:
	    if (map->port > 255) {
		return BCM_E_PARAM;
	    }

	    if (map->vlan > 4095) {
		return BCM_E_PARAM;
	    }

	    rv = READ_EP_OI2QB_MAPm(unit, MEM_BLOCK_ANY, map->vlan + minOitt, &oi2qb);
	    if (rv != SOC_E_NONE) {
		return(rv);
	    }
	    queue_id = soc_mem_field32_get(unit, EP_OI2QB_MAPm, &oi2qb, QUEUE_BASEf);

	    sal_memset(&ep_entry, 0, sizeof(ep_entry));

	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[map->port]);
	    if (sp_info->valid == FALSE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, invalid fabric_port %d, unit %d\n"),
		           FUNCTION_NAME(), map->port, unit));
		return BCM_E_PARAM;
	    }
	    
	    rv = READ_EP_DEST_PORT_MAPm(unit, MEM_BLOCK_ANY, sp_info->egroup[0].es_scheduler_level0_node, &ep_entry);
	    if (rv != SOC_E_NONE) {
		return(rv);
	    }
	    queue_id |= soc_mem_field32_get(unit, EP_DEST_PORT_MAPm, &ep_entry, QUEUEf);
	    BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(map->queue_id, queue_id);

	    break;
	case BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID:
	    if (map->encap_id < 8192) {
		return BCM_E_PARAM;
	    }
            BCM_IF_ERROR_RETURN(soc_sirius_queue_map_block_entry_get(unit,
                                                                     _SIRIUS_I_QUEUE_MAP_SEG_SBX_RQ +
                                                                     SIRIUS_QUEUE_MAP_BLOCK_OFFSET_INGRESS,
                                                                     map->encap_id,
                                                                     &cosmap,
                                                                     &queue));
	    BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(map->queue_id, queue);
	    break;
	default:
	    return BCM_E_PARAM;
    }
    return rv;
}

int
bcm_sirius_cosq_subscriber_traverse(int unit,
				 bcm_cosq_subscriber_map_traverse_cb cb,
				 void *user_data)
{

    bcm_cosq_subscriber_map_t *p_map = NULL;
    int rv = BCM_E_NONE;
    int cmd = 0;
    bcm_sbx_subscriber_map_cb_params_t *p_cb_params = (bcm_sbx_subscriber_map_cb_params_t *) user_data;
    
    if (p_cb_params != NULL) {
	cmd = p_cb_params->cmd;
    } else {
	cmd = BCM_SBX_COSQ_SUBSCRIBER_MAP_GET; /* default */
    }

    if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_HYBRID) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Supported in Hybrid mode\n")));
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_DEBUG(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "Not supported in XGS mode\n")));
	return BCM_E_UNAVAIL;
    }

    p_map = sal_alloc(sizeof(bcm_cosq_subscriber_map_t), "subscriber map");
    if (p_map == NULL) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
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

    return rv;
}


static int
bcm_sirius_fc_init(int unit)
{
    int rv = BCM_E_NONE;
    int fifo_nbr, hg, index;


    fc_map_state[unit] = sal_alloc((sizeof(bcm_sbx_cosq_egress_flow_control_state_t) *
                           BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS), "egress fc map state");
    if (fc_map_state[unit] == NULL) {
        return(BCM_E_MEMORY);
    }

    /* by default flow control mapping is disabled */
    sal_memset(fc_map_state[unit], 0x00, (sizeof(bcm_sbx_cosq_egress_flow_control_state_t) *
                                                      BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS));


    fc_state[unit] = sal_alloc((sizeof(bcm_sbx_sirius_fc_state) * SIRIUS_ES_FC_STATE_TABLE_SIZE),
			       "egress fc state");
    if (fc_state[unit] == NULL) {
        return(BCM_E_MEMORY);
    }
    sal_memset(fc_state[unit], 0x00, (sizeof(bcm_sbx_sirius_fc_state) * SIRIUS_ES_FC_STATE_TABLE_SIZE));

    /* reserve last entry for Internal ports */
    /* entry initialized during soc_init() to not assert any flow control */
    (fc_state[unit] + (SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max - 1))->in_use = TRUE;
    (fc_state[unit] + (SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max - 1))->module = -1;

    /* multicast state to determine Lossless/Non-Blocking service */
    mc_fc_map_state[unit] = sal_alloc((sizeof(bcm_sbx_cosq_egress_flow_control_state_t) *
                           SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS *
                           (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2)), "mc egress fc map state");
    if (mc_fc_map_state[unit] == NULL) {
        return(BCM_E_MEMORY);
    }

    /* by default the first group is set identical to the device wide setting */
    for (hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
        index = hg * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2;
        for (fifo_nbr = 0; fifo_nbr < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; fifo_nbr++) {
            (mc_fc_map_state[unit] + index + fifo_nbr)->int_pri = -1;
            (mc_fc_map_state[unit] + index + fifo_nbr)->flow_control_mask = -1;
        }
    }

    /* by default the 2nd group has all flow control mapping disabled */
    for (hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
        index = hg * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2;
        for (fifo_nbr = BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS;
                       (fifo_nbr < (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2)); fifo_nbr++) {
            (mc_fc_map_state[unit] + index + fifo_nbr)->int_pri = fifo_nbr;
            (mc_fc_map_state[unit] + index + fifo_nbr)->flow_control_mask = 0;
        }
    }

    rv = bcm_sirius_fc_state_clear(unit, BCM_GPORT_INVALID, NULL, NULL, FALSE);

    return(rv);
}


static int
bcm_sirius_fc_deinit(int unit)
{
    int rv = BCM_E_NONE;


    if (fc_map_state[unit] != NULL) {
	sal_free(fc_map_state[unit]);
        fc_map_state[unit] = NULL;
    }
    if (fc_state[unit] != NULL) {
	sal_free(fc_state[unit]);
        fc_state[unit] = NULL;
    }
    if (mc_fc_map_state[unit] != NULL) {
	sal_free(mc_fc_map_state[unit]);
        mc_fc_map_state[unit] = NULL;
    }

    return(rv);
}


int
bcm_sirius_fc_mc_group_is_non_blocking(int unit, int hg, int group, int *is_non_blocking)
{
    int rv = BCM_E_NONE;
    int fifo_nbr, index;


    (*is_non_blocking) = TRUE;
    index = (hg * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2) + 
                                (group * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS);
    for (fifo_nbr = 0; fifo_nbr < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; fifo_nbr++) {
        if ((mc_fc_map_state[unit] + index + fifo_nbr)->flow_control_mask != 0) {
            (*is_non_blocking) = FALSE;
            break;
        }
    }

    return(rv);
}


int
bcm_sirius_fc_reserved_fifoset_get(int unit, int *base_fifo, int *size)
{
    int rv = BCM_E_NONE;


    (*base_fifo) = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0 -
                                                    BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS;
    (*size) = BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS;

    return(rv);
}

static int
bcm_sirius_fc_reserved_fifoset_set(int unit, bcm_sbx_sirius_egress_scheduler_state_t *p_esstate,
                                                                       int base_fifo, int size)
{
    int rv = BCM_E_NONE;
    int es;


    if (size != BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS) {
        return(BCM_E_INTERNAL);
    }
    if (base_fifo != - 1) {
        return(BCM_E_INTERNAL);
    }

    /* NOTE: For now this should be sufficient to reserve this FIFO set */
    for (es = (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0 -
                                       BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS);
                              (es < SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0); es++) {
	(p_esstate + es)->in_use = TRUE;
    }

    return(rv);
}

static int
bcm_sirius_fc_mapping_set(int unit, int port,
                      bcm_sbx_cosq_egress_flow_control_state_t *cur_fc_map_state,
                      bcm_sbx_cosq_egress_flow_control_state_t *prev_fc_map_state,
                      int force_all)
{
    int rv = BCM_E_NONE;
    int fc_mask, flow_control_mask, fc_map, fc_nxlate_map;
    int int_pri;
    int i;
    flow_control_translate_table_entry_t translate_table_entry;


    if (port != BCM_GPORT_INVALID) {
        

        return(rv);
    }

    if (force_all == TRUE) {
        /* do all the configuration */
        for (fc_mask = 0; fc_mask < SIRIUS_ES_FC_TRANSLATE_TABLE_SIZE; fc_mask++) {
	    
            for (int_pri = 0, fc_map = 0;
                          int_pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; int_pri++) {
                flow_control_mask = (cur_fc_map_state + int_pri)->flow_control_mask;
                for (i = 0; i < 8; i++) {
                    if ( (fc_mask & (1 << i)) & (flow_control_mask & (1 << i)) ) {
                        fc_map |= (1 << int_pri);
                    }
                }
            }

            /* Update flow control mapping */
            fc_nxlate_map = fc_map;
            fc_map = (~(fc_map) & 0xF);
            soc_mem_field32_set(unit, FLOW_CONTROL_TRANSLATE_TABLEm,
                                           &translate_table_entry, FC_STATE_XLATEf, fc_map);
            WRITE_FLOW_CONTROL_TRANSLATE_TABLEm(unit, MEM_BLOCK_ANY, fc_mask,
                                                                    &translate_table_entry);

            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "Flow Control Translate Table, Index: %d, Mask: 0x%x HwMask: 0x%x\n"),
                         fc_mask, fc_nxlate_map, fc_map));
        }
    }
    else {
        /* do only the configuration that has changed */
    }

    return(rv);
}

static int
bcm_sirius_fc_state_clear(int unit, int port,
                      bcm_sbx_cosq_egress_flow_control_state_t *cur_fc_map_state,
                      bcm_sbx_cosq_egress_flow_control_state_t *prev_fc_map_state,
                      int force_all)
{
    int rv = BCM_E_NONE;
    flow_control_state_table_entry_t fc_state_table_entry;


    if (port != BCM_GPORT_INVALID) {
        

        return(rv);
    }

    /* clear state for FIFOs that are not mapped */
    /* last entry for Internal ports initialized to not assert any flow control */
    sal_memset(&fc_state_table_entry, 0, sizeof(flow_control_state_table_entry_t));
    soc_mem_field32_set(unit, FLOW_CONTROL_STATE_TABLEm, &fc_state_table_entry, FC_STATEf, 0xf);
    WRITE_FLOW_CONTROL_STATE_TABLEm(unit, MEM_BLOCK_ANY, SIRIUS_FC_NOT_MAPPED_FIFO_GROUP_STATE_TABLE_INDEX, &fc_state_table_entry);

    return(rv);
}

/* "max_ports" should include ports for aggregration */
int
bcm_sirius_cosq_module_congestion_allocate(int unit, bcm_module_t module,
                                                       int max_ports, int *base_index)
{
    int rv = BCM_E_NONE;
    int index, start_index = 0, nbr_available = 0;
    int is_allocated = FALSE;


    (*base_index) = 0;

    /* account for entry reserved for FIFOS that are never flow controlled */
    for (index = 0; index <= (SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max - 1); index++) {
        if ((fc_state[unit] + index)->in_use == FALSE) {
            if (nbr_available == 0) {
                start_index = index;
            }
            nbr_available++;
        }
        else {
            nbr_available = 0;
        }

        if (nbr_available == max_ports) {
           is_allocated = TRUE;
           break;
        }
    }

    if (is_allocated != TRUE) {
        return(BCM_E_RESOURCE);
    }

    for (index = start_index; index < (start_index + max_ports); index++) {
        (fc_state[unit] + index)->in_use = TRUE;
        (fc_state[unit] + index)->module = module;
    }

    (*base_index) = start_index;

    return(rv);
}

int
bcm_sirius_cosq_module_congestion_deallocate(int unit, bcm_module_t module, int max_ports)
{
  int index = 0, count = 0;


    for (index = 0; index <= SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max; index++) {
        if ( ((fc_state[unit] + index)->in_use == TRUE) &&
                                                 ((fc_state[unit] + index)->module == module) ) {
            (fc_state[unit] + index)->in_use = FALSE;
            (fc_state[unit] + index)->module = 0;
	    count++;
        }
    }

    if (count) {
	return BCM_E_NONE;
    } else {
	return BCM_E_NOT_FOUND;
    }
}

int
bcm_sirius_cosq_module_congestion_allocate_get(int unit, bcm_module_t module,
                                                              int *max_ports, int *base_index)
{
    int rv = BCM_E_NONE;
    int index, start_index = 0, nbr_port = 0;


    (*max_ports) = (*base_index) = 0;
    for (index = 0; index <= SOC_MEM_INFO(unit, FLOW_CONTROL_STATE_TABLEm).index_max; index++) {
        if ( ((fc_state[unit] + index)->in_use == TRUE) &&
                                                 ((fc_state[unit] + index)->module == module) ) {
            if (nbr_port == 0) {
                start_index = index;
            }
            nbr_port++;
        }
        else {
            if (nbr_port > 0) {
                break;
            }
        }
    }

    if (nbr_port == 0) {
        return(BCM_E_RESOURCE);
    }

    (*max_ports) = nbr_port;
    (*base_index) = start_index;

    return(rv);
}

static int
_bcm_sirius_cosq_requeue_intf_congestion_set(int unit, bcm_gport_t rq_gport)
{
    int                      rv = BCM_E_NONE;
    uint32                   flags_out = 0;
    bcm_gport_t              subport;
    int                      done = FALSE, i;
    int                     *fifo_map = NULL, nbr_fifos = 0;
    int                      cur_fifo;


    /* allocate temporary memory */
    fifo_map = sal_alloc((sizeof(int) * SIRIUS_ES_FC_STATE_TABLE_SIZE), "tmp fifo map");
    if (fifo_map == NULL) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, Temp memory allocation failed, unit: %d\n"),
	             FUNCTION_NAME(), unit));
	return(BCM_E_MEMORY);
    }

    subport = BCM_GPORT_INVALID;
    for (i = 0; i < SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS; i++) {
        rv = bcm_sirius_port_subport_getnext(unit, rq_gport, flags_out, &subport);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, bcm_sirius_port_subport_getnext(), unit: %d, Error(%d)\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        if (subport == BCM_GPORT_INVALID) {
            done = TRUE;
            break;
        }

        rv = bcm_sirius_port_subport_fifos_get(unit, subport, SBX_SUBPORT_FLAG_EGRESS_REQUEUE, fifo_map, &nbr_fifos, SIRIUS_ES_FC_STATE_TABLE_SIZE);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, bcm_sirius_port_subport_fifos_get(), unit: %d, Error(%d)\n"),
                       FUNCTION_NAME(), unit, rv));
            goto err;
        }

        if (nbr_fifos == 0) {
            continue;
        }

        for (cur_fifo = 0; cur_fifo < nbr_fifos; cur_fifo++) {
            rv = soc_sirius_es_fc_map_table_set(unit,
                                fifo_map[cur_fifo] / BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS,
                                fifo_map[cur_fifo],
                                SIRIUS_FC_NOT_MAPPED_FIFO_GROUP_STATE_TABLE_INDEX);
            if (rv != BCM_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, configuring fc map table, unit: %d, fifo Base: 0x%x Index: 0x%x\n"),
	                   FUNCTION_NAME(), unit, fifo_map[cur_fifo], SIRIUS_FC_NOT_MAPPED_FIFO_GROUP_STATE_TABLE_INDEX));
                goto err;
            }
        }
    }

    if (done != TRUE) {
        goto err;
    }

    sal_free(fifo_map);

    return(rv);

err:
    if (fifo_map != NULL) {
        sal_free(fifo_map);
    }

    return(rv);

}

static int
_bcm_sirius_cosq_requeue_intfs_congestion_set(int unit)
{
    int          rv = BCM_E_NONE;
    int          rq_intf, my_modid;


    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &my_modid));

    PBMP_REQ_ITER(unit, rq_intf) {
        rv = _bcm_sirius_cosq_requeue_intf_congestion_set(unit, rq_intf);
        if (rv != BCM_E_NONE) {
            break;
        }
    }

    return(rv);
}

int
bcm_sirius_cosq_fifo_set_get(int unit, bcm_gport_t fabric_port, int *fifo_map, int *nbr_fifos, int max_fifo_map_size)
{
    int rv = BCM_E_NONE;
    int port;
    int level1_node;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int cur_group, num_groups, i;
    int egroup_num, num_fifos;

    /* Determine the set of FIFOs associated with this "fabric port". */

    port = -1;
    rv = bcm_sbx_cosq_egress_group_info_get(unit, fabric_port, &port, &egroup_num, &num_fifos);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, retreiving fabric_port info, unit: %d, rv: 0x%x\n"),
	           FUNCTION_NAME(), unit, rv));
	return(rv);
    }

    if ( (SOC_SBX_CFG(unit)->fabric_egress_setup == TRUE) ||
                                 (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == TRUE) ) {
        /* determine the set of FIFOs associated with this "fabric port". In the current */
        /* implementation there is only one set of FIFOs associated with this            */
        /* "fabric port".                                                                */

        level1_node = SOC_SBX_STATE(unit)->port_state->subport_info[port].es_scheduler_level1_node[egroup_num];

        if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == TRUE) {
            (*fifo_map) = SOC_SBX_STATE(unit)->port_state->subport_info[port].egroup[0].es_scheduler_level0_node;
            (*nbr_fifos) = (SOC_SBX_STATE(unit)->port_state->subport_info[port].egroup[0].num_fifos + 1)/2;
        } else {
            (*fifo_map) = (level1_node * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS);
            (*nbr_fifos) = (SOC_SBX_STATE(unit)->port_state->subport_info[port].egroup[0].num_fifos + 3)/4;
        }

        

        
        /*       Internal Unicast FIFOs corresponding to Front panel/Fabrix port */
    }

    (*nbr_fifos) = 0;
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
    if ( (sp_info == NULL) || (sp_info->valid != TRUE) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid fabric port 0x%X, unit %d\n"),
	           FUNCTION_NAME(), fabric_port, unit));
	return(BCM_E_PARAM);
    }
    
    cur_group = 0;

    num_groups = num_fifos / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE;
    num_groups = (((num_fifos % SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE) == 0) ?
		  num_groups : num_groups + 1);
    num_groups = ((num_groups <= (max_fifo_map_size - cur_group)) ?
		  num_groups : (max_fifo_map_size - cur_group));

    
    for (i = 0; i < num_groups; i++, cur_group++) {
	*(fifo_map + cur_group) = sp_info->egroup[egroup_num].es_scheduler_level0_node +
	    (i * SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE);
    }
    (*nbr_fifos) = cur_group;

    
    /*        Internal Unicast FIFOs corresponding to Front panel/Fabric port */

    return(rv);
}

int
bcm_sirius_cosq_module_congestion_set(int unit, bcm_port_t higig_port, bcm_module_t module)
{
    int rv = BCM_E_NONE;
    int max_ports, nbr_port, cfg_max_ports;
    bcm_gport_t switch_port, fabric_port;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int *fifo_map = NULL, nbr_fifos = 0;
    int fc_state_index, i, fg, map_index = 0;
    int subport, port, port_offset;
    int higig_gport, modid;
    bcm_trunk_t tid;
    unsigned int target[4], target_count;
    int is_non_blocking, group1_index;
    int egroup_num, num_fifos;
    bcm_trunk_info_t trunkInfo;
    bcm_trunk_member_t trunkMembers[BCM_TRUNK_FABRIC_MAX_PORTCNT];
    int memberCount;


    /* configure all ports on requeue interfaces to be never flow conrolled */
    rv = _bcm_sirius_cosq_requeue_intfs_congestion_set(unit);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, requeue ports flow control config failure unit: %d, error: 0x%x\n"),
	           FUNCTION_NAME(), unit, rv));
        return(rv);
    }

    /* Determine the number of ports in flow control message for this module */
    rv = bcm_sirius_fabric_congestion_size_get(unit, module, &max_ports);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, retreiving congestion size, unit: %d, module: 0x%x\n"),
	           FUNCTION_NAME(), unit, module));
        return(rv);
    }

    if (max_ports == 0)  {
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "%s HiGig Port (%d) module (%d) has no ports reserved\n"),
                     FUNCTION_NAME(), higig_port, module));
        return rv;
    }

    /* retreive base index */
    rv = bcm_sirius_cosq_module_congestion_allocate_get(unit, module, &cfg_max_ports,
                                                                             &fc_state_index);
    if (rv != BCM_E_NONE) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, No congestion resources present, unit: %d, module: 0x%x\n"),
	             FUNCTION_NAME(), unit, module));
        return(rv);
    }

    /* allocate temporary memory */
    fifo_map = sal_alloc((sizeof(int) * 132), "tmp fifo map");

    if (fifo_map == NULL) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, Temp memory allocation failed, unit: %d\n"),
	             FUNCTION_NAME(), unit));
	return BCM_E_MEMORY;
    }

    /* Determine all front panel ports for a module and the corresponding fabric ports */
    rv = bcm_sirius_stk_modid_get(unit, &modid);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "%s, failed to get modid, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        goto err;
    }	

    for (nbr_port = 0; nbr_port < max_ports; nbr_port++) {
        /* generate front panel port */
        BCM_GPORT_MODPORT_SET(switch_port, module, nbr_port);

        /* determine corresponding Fabric Port */
        rv = bcm_sbx_stk_fabric_map_get(unit, switch_port, &fabric_port);
        if (rv == BCM_E_NOT_FOUND) {
            rv = BCM_E_NONE;
            continue;
        } else if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, retreiving fabric port, unit: %d, switch gport: 0x%x\n"),
	               FUNCTION_NAME(), unit, switch_port));
            goto err;
        }

        /* determine if this is a fabric port for the device */
        if (BCM_GPORT_IS_CHILD(fabric_port)) {
            if (BCM_GPORT_CHILD_MODID_GET(fabric_port) != modid) {
	        LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                    (BSL_META_U(unit,
	                                "%s, Skiping unit: %d, switch gport: 0x%x\n"),
	                     FUNCTION_NAME(), unit, switch_port));
                continue;
            }
        }
        else if (BCM_GPORT_IS_EGRESS_CHILD(fabric_port)) {
            if (BCM_GPORT_EGRESS_CHILD_MODID_GET(fabric_port) != modid) {
                continue;
            }
        }
        else if (BCM_GPORT_IS_EGRESS_GROUP(fabric_port)) {
            if (BCM_GPORT_EGRESS_GROUP_MODID_GET(fabric_port) != modid) {
                continue;
            }
        }
        else {
            /* should never get here, but forget this one if we do */
            continue;
        }
        

        /* determine if it is a CPU port */
        if ( (BCM_GPORT_IS_MODPORT(fabric_port)) || (BCM_GPORT_IS_EGRESS_MODPORT(fabric_port)) ) {
	    subport = fabric_port;
        }
        else {
	    subport = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, fabric_port, &subport, &egroup_num, &num_fifos);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, nexpected fabric port,  Switch Port: 0x%x, fabric port: 0x%x\n"),
		           FUNCTION_NAME(), switch_port, fabric_port));
		goto err;
	    }
	}

        if (subport != -1) {
            rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, &port_offset);
            if (rv != BCM_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, err: 0x%x\n"),
	                   FUNCTION_NAME(), rv));
                goto err;
            }
            if (IS_CPU_PORT(unit, port)) {
                LOG_VERBOSE(BSL_LS_BCM_COSQ,
                            (BSL_META_U(unit,
                                        "Switch Port: 0x%x, fabric port: 0x%x corresponds to CPU port\n"),
                             switch_port, fabric_port));
                continue;
            }
        }

	/*
 	 * Skip internal multicast ports
 	 */

	if ((SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == TRUE) &&
	    ((BCM_GPORT_IS_CHILD(fabric_port)) ||
	    (BCM_GPORT_IS_EGRESS_CHILD(fabric_port)))) {
                sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	        if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST)  ||
	            (sp_info->flags & SBX_SUBPORT_FLAG_INGRESS_MCAST)) {
		    continue;
		}
	}

        /* determine FIFOs corresponding to Fabric ports */
        rv = bcm_sirius_cosq_fifo_set_get(unit, fabric_port, fifo_map, &nbr_fifos, 132);
        if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, retreiving fifos for fabric port, unit: %d, fabric gport: 0x%x\n"),
	               FUNCTION_NAME(), unit, switch_port));
            goto err;
        }

        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "Switch Port: 0x%x, fabric port: 0x%x FifoMap: %d, nbrFifos: %d\n"),
                     switch_port, fabric_port, (*fifo_map), nbr_fifos));

	if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == TRUE) {
	    map_index = (nbr_port >> 1);
	} else {
	    map_index = nbr_port;
	}

        /* configure device */
        for (i = 0; i < nbr_fifos; i++) {
            rv = soc_sirius_es_fc_map_table_set(unit,
                         (*(fifo_map + i)) / BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS,
                         (*(fifo_map + i)),
                         (fc_state_index + map_index));
            if (rv != BCM_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, configuring fc map table, unit: %d, fifo Base: 0x%x Index: 0x%x\n"),
	                   FUNCTION_NAME(), unit, (*(fifo_map + i)), (fc_state_index + map_index)));
                goto err;
            }
        }
    }

    /* determine any Internal Multicast FIFOs (XGS Line card model).      */
    /* Configure device accordingly                                       */
    /* determine HiGig trunk membership                                   */
    rv = bcm_sirius_stk_modid_get(unit, &modid);
    if (rv != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "%s, failed to get modid, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        goto err;
    }	

    BCM_GPORT_MODPORT_SET(higig_gport, modid, higig_port);
    rv = bcm_sirius_trunk_find_and_get(unit,
                                       modid,
                                       higig_gport,
                                       &tid,
                                       &trunkInfo,
                                       BCM_TRUNK_FABRIC_MAX_PORTCNT,
                                       &(trunkMembers[0]),
                                       &memberCount);
    if (rv == BCM_E_NOT_FOUND) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, No HiGig Trunk membership, HiGig Port: %d, HiGig Gport: 0x%x\n"),
	             FUNCTION_NAME(), higig_port, higig_gport));
        rv = BCM_E_NONE;
        goto done;
    }
    else if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "%s, error in getting HiGig Trunk membership, HiGig Port: %d, HiGig Gport: 0x%x\n"),
	           FUNCTION_NAME(), higig_port, higig_gport));
         rv = BCM_E_NONE;
        goto done;
    }

    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "%s HiGig Trunk found for HiGig Port/Gport: %d(0x%x) NbrPorts: %d Irunk: %d\n"),
                 FUNCTION_NAME(), higig_port, higig_gport, memberCount, tid));

    /* is group 1 non-blocking */
    port = BCM_GPORT_MODPORT_PORT_GET(higig_gport);
    port = port - SOC_PORT_MIN(unit, hg);
    bcm_sirius_fc_mc_group_is_non_blocking(unit, port, 1, &is_non_blocking);
    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "%s is_non_blocking: %d\n"),
                 FUNCTION_NAME(), is_non_blocking));

    /* determine the internal multicast ports */
    for (i = 0; i < memberCount; i++) {
        if (!BCM_GPORT_IS_SET(trunkMembers[i].gport)) {
            BCM_GPORT_MODPORT_SET(higig_gport, modid, trunkMembers[i].gport);
        } else {
            higig_gport = trunkMembers[i].gport;
          if (BCM_GPORT_IS_MODPORT(trunkMembers[i].gport)) {
                if (modid != BCM_GPORT_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                    continue;
                }
            } else if (BCM_GPORT_IS_EGRESS_MODPORT(trunkMembers[i].gport)) {
                if (modid != BCM_GPORT_EGRESS_MODPORT_MODID_GET(trunkMembers[i].gport)) {
                    continue;
                }
            } else {
                /* not proper to have other types in these; skip it */
                continue;
            }
        }
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "%s HiGig Port (%d) part of trunk (%d)\n"),
                     FUNCTION_NAME(), trunkMembers[i].gport, tid));

        rv = bcm_sirius_aggregate_gport_translate(unit, 0 /* flags */, modid, modid, higig_gport, &target[0], &target_count, NULL);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "%s Error getting the internal Mc port for HiGig port/Gport %d(0x%x))\n"),
                       FUNCTION_NAME(), trunkMembers[i].gport, higig_gport));
            goto err;
        }
        if (target[0] >= SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "%s Unexpected internal Mc port (%d) for HiGig port/Gport %d(0x%x))\n"),
                       FUNCTION_NAME(), target[0], trunkMembers[i].gport, higig_gport));
            continue;
        }

        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "%s found internal Mc port (%d) for HiGig port/Gport %d(0x%x))\n"),
                     FUNCTION_NAME(), target[0], trunkMembers[i].gport, higig_gport));

        for(fg=0; fg < target_count; fg++) {
	  if ((fg != 0) && (is_non_blocking == TRUE)) {
                group1_index = SIRIUS_FC_NOT_MAPPED_FIFO_GROUP_STATE_TABLE_INDEX;
            }
            else {
                group1_index = fc_state_index + cfg_max_ports - 1;
            }

            rv = soc_sirius_es_fc_map_table_set(unit, target[0]+fg,
						(target[0]+fg) * SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE, 
						group1_index);
            if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, configuring fc map table, unit: %d, fifo Base: 0x%x Index: 0x%x\n"),
		           FUNCTION_NAME(), unit, 
		           ((target[0]+fg) * SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE),
		           group1_index));
                goto err;
            }
        }
    }

done:
    if (fifo_map != NULL) {
        sal_free(fifo_map);
    }

    return(rv);

err:
    if (fifo_map != NULL) {
        sal_free(fifo_map);
    }

    return(rv);
}

int
bcm_sirius_cosq_congestion_set(int unit,
                               bcm_port_t higig_port,
                               bcm_sbx_port_congestion_info_t *congestion_info,
                               bcm_port_congestion_config_t *config)
{
    int rv = BCM_E_NONE;
    bcm_module_t module = -1;
    bcm_gport_t switch_port, fabric_port, egress_fabric_port;
    int max_ports, cfg_max_ports, alloc_max_ports, fc_state_index;
    int is_encap_higig;
    int port;
    int module_index, nbr_modules = 0;
    bcm_module_t mod[SBX_MAX_MODULES_FP_ON_HIGIG];

    /* NOTE: This function is used for E2ECC RX only since E2ECC is module based */

    if (BCM_GPORT_IS_MODPORT(higig_port)) {
	port = BCM_GPORT_MODPORT_PORT_GET(higig_port);
    }
    else if (BCM_GPORT_IS_EGRESS_MODPORT(higig_port)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(higig_port);
    }
    else {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unsupported type for gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(), higig_port, unit));
	return(BCM_E_PARAM);
    }

    /* given a HiGig port, determine "modid". To determine this find a child port for     */
    /* which switch port is defined. From the switch port, "modid" is defined.            */

    nbr_modules = 0;
    for (module_index = 0; module_index < SBX_MAX_MODULES_FP_ON_HIGIG; module_index++) {
        mod[module_index] = -1;
    }
  
    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) {
        if (port >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) {
            port = port - SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS;
        }
    }
 
    fabric_port = BCM_GPORT_INVALID;
    do {
        rv = bcm_sirius_port_subport_getnext(unit, port, SBX_SUBPORT_FLAG_INTERNAL, &fabric_port);
        if (rv != BCM_E_NONE) {
            break;
        }
        if (fabric_port == BCM_GPORT_INVALID) {
            break;
        }
        if (BCM_GPORT_IS_CHILD(fabric_port)) {
            BCM_GPORT_EGRESS_CHILD_SET(egress_fabric_port,
                                BCM_GPORT_CHILD_MODID_GET(fabric_port),
                                BCM_GPORT_CHILD_PORT_GET(fabric_port));
        }
        else if (BCM_GPORT_IS_EGRESS_CHILD(fabric_port)) {
            egress_fabric_port = fabric_port;
            /* convert to child gport */
            BCM_GPORT_CHILD_SET(fabric_port,
                                BCM_GPORT_EGRESS_CHILD_MODID_GET(fabric_port),
                                BCM_GPORT_EGRESS_CHILD_PORT_GET(fabric_port));
        }
        else {
            /* should never get here, but forget this one if we do */
            continue;
        }

        rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, fabric_port, &switch_port);
        if (rv == BCM_E_NOT_FOUND) {

            /* try retreiving via Egress gport */
            rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, egress_fabric_port, &switch_port);
            if (rv == BCM_E_NOT_FOUND) {
	        LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                    (BSL_META_U(unit,
	                                "%s, retreiving switch port, unit: %d, fabric gport: 0x%x\n"),
	                     FUNCTION_NAME(), unit, fabric_port));
                continue;
            }
            else if (rv != BCM_E_NONE) {
                break;
            }
        }
        else if (rv != BCM_E_NONE) {
            break;
        }

        /* determine module corresponding to that child port */
        module = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        for (module_index = 0; module_index < SBX_MAX_MODULES_FP_ON_HIGIG; module_index++) {
            if (mod[module_index] != -1) {
                if (mod[module_index] == module) {
                    break;
                }
            }
            else {
                mod[module_index] = module;
                nbr_modules++;
                if (nbr_modules == SBX_MAX_MODULES_FP_ON_HIGIG) {
                    break;
                }
            }
        }

    } while (TRUE);

    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* if no switch port found, no configuration needs to be done */
    if (nbr_modules == 0) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, no Switch port found, unit: %d, higig gport: 0x%x\n"),
	             FUNCTION_NAME(), unit, higig_port));
        return(rv);
    }

    for (module_index = 0; module_index < nbr_modules; module_index++) {
        module = mod[module_index];

        /* Determine the number of ports in flow control message for this module */
        rv = bcm_sirius_fabric_congestion_size_get(unit, module, &max_ports);
        if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, retreiving congestion size, unit: %d, module: 0x%x\n"),
	               FUNCTION_NAME(), unit, module));
            return(rv);
        }

	if (max_ports == 0) {
	    continue;
        }

        /* retreive base index */
        rv = bcm_sirius_cosq_module_congestion_allocate_get(unit, module, &cfg_max_ports,
                                                                             &fc_state_index);
        if ( (rv != BCM_E_NONE) && (rv != BCM_E_RESOURCE) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, retreiving congestion resources, unit: %d, module: 0x%x\n"),
	               FUNCTION_NAME(), unit, module));
            return(rv);
        }

	if (SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
	    /* config half number of resource since 2 ports will share 1 fc entry, round up */
	    alloc_max_ports = (max_ports+1)/2;
        } else {
	    /* determine if an extra port needs to be reserved for MC merge */
	    alloc_max_ports = max_ports;
	    rv  = bcm_sbx_port_any_is_encap_higig(unit, &is_encap_higig);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, retreiving congestion resources, unit: %d, module: 0x%x\n"),
		           FUNCTION_NAME(), unit, module));
		return(rv);
	    }

	    /* Allocate an extra port to be reserved for MC merge if higig */
	    alloc_max_ports = (is_encap_higig == TRUE) ? (alloc_max_ports + 1): alloc_max_ports;
	}

        /* determine if the module resources need to be allocated */
        if (cfg_max_ports == 0) {

            rv = bcm_sirius_cosq_module_congestion_allocate(unit, module, alloc_max_ports, &fc_state_index);
            if (rv != BCM_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, allocating congestion resources, unit: %d, module: 0x%x\n"),
	                   FUNCTION_NAME(), unit, module));
                return(rv);
            }

            rv = soc_sirius_es_fc_base_table_set(unit, module, 64 /* move hardcode from low level to here */, fc_state_index, TRUE);
            if (rv != BCM_E_NONE) {
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, configuring congestion resources, unit: %d, module: 0x%x rc: 0x%x\n"),
	                   FUNCTION_NAME(), unit, module, rv));
                return(rv);
            }
        }
        else if (cfg_max_ports != alloc_max_ports) { /* consistency check */
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, inconsistency in congestion resource allocation, unit: %d, module: 0x%x Expected: 0x%x Configured: 0x%x\n"),
	               FUNCTION_NAME(), unit, module, alloc_max_ports, cfg_max_ports));
            return(BCM_E_CONFIG);
        }

        /* initiate configuraton for that module */
        rv = bcm_sirius_cosq_module_congestion_set(unit, higig_port, module);
        if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, configuring congestion, unit: %d, module: 0x%x, rv: 0x%x\n"),
	               FUNCTION_NAME(), unit, module, rv));
        }
    }

    return(rv);
}

/*
 * Currently only supporting port value of "BCM_GPORT_INVALID"
 */
int
bcm_sirius_cosq_flow_control_set(int unit,
                                 bcm_gport_t port,
                                 bcm_cos_t int_pri,
                                 uint32 flow_control_mask,
                                 void *info)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_egress_flow_control_state_t cur_fc_map_state[BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS];
    int pri;
    int child_port, hg, index;


    /* check for multicast support (Lossless/Non_Blocking) */
    if (port != BCM_GPORT_INVALID) {
        if (!BCM_GPORT_IS_CHILD(port)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Invalid gport: 0x%x, unit: %d, Expecting Child Gport\n"),
	               FUNCTION_NAME(), port, unit));
            return(BCM_E_PARAM);
        }

        child_port = BCM_GPORT_CHILD_PORT_GET(port);
        if (!SB_FAB_DEVICE_SIRIUS_IS_MCAST_HANDLE(child_port)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Invalid child port: %d, unit: %d, Expecting mc child port\n"),
	               FUNCTION_NAME(), child_port, unit));
            return(BCM_E_PARAM);
        }
        hg = SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE_HG_GET(child_port);

        if (int_pri >= (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2)) {
            return(BCM_E_PARAM);
        }
 
        index = (hg * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2) + int_pri;
        (mc_fc_map_state[unit] + index)->flow_control_mask = flow_control_mask;

        return(rv);
    }

    /* consistency checks - port */
    if (port != BCM_GPORT_INVALID) {
        /* coverity[dead_error_begin] */
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid gport, unit: %d, gport: 0x%x, expected gport: 0x%x\n"),
	           FUNCTION_NAME(), unit, port, BCM_GPORT_INVALID));
        return(BCM_E_PARAM);
    }

    /* currently only supporting 4 FIFO model */
    
    if ( (int_pri > (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS - 1)) && (int_pri != -1) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid int_pri, unit: %d, int_pri: 0x%x, max int_pri: 0x%x\n"),
	           FUNCTION_NAME(), unit, int_pri, (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS - 1)));
        return(BCM_E_PARAM);
    }

    /* only supporting 8 bit mask */
    if ((flow_control_mask & ~(0xFF)) > 0) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid flow control mask, unit: %d, flow_control_mask: 0x%x\n"),
	           FUNCTION_NAME(), unit, flow_control_mask));
        return(BCM_E_PARAM);
    }

    /* determine new mapping */
    for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
        cur_fc_map_state[pri].flow_control_mask = (fc_map_state[unit] + pri)->flow_control_mask;
    }
    if (int_pri != -1) {
        cur_fc_map_state[int_pri].flow_control_mask = flow_control_mask;
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "Set Flow Control, InternalPri: %d, Mask: 0x%x\n"),
                     int_pri, flow_control_mask));
    }
    else {
        for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
            cur_fc_map_state[pri].flow_control_mask = flow_control_mask;
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "Set Flow Control, InternalPri: %d, Mask: 0x%x\n"),
                         pri, flow_control_mask));
        }
    }

    for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "Set Flow Control Configuration, InternalPri: %d, Mask: 0x%x\n"),
                     pri, cur_fc_map_state[pri].flow_control_mask));
    }
    
    /* Configure Mapping */
    rv = bcm_sirius_fc_mapping_set(unit, port, cur_fc_map_state, fc_map_state[unit], TRUE);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, configuring flow control mapping, unit: %d, rc: 0x%x\n"),
	           FUNCTION_NAME(), unit, rv));
        return(rv);
    }

    /* clear any state that is required */
    rv = bcm_sirius_fc_state_clear(unit, port, cur_fc_map_state, fc_map_state[unit], TRUE);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, clearing flow control state, unit: %d, rc: 0x%x\n"),
	           FUNCTION_NAME(), unit, rv));
        return(rv);
    }

    /* update new mapping */
    for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
        (fc_map_state[unit] + pri)->flow_control_mask = cur_fc_map_state[pri].flow_control_mask;
        LOG_VERBOSE(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "Flow Control Configuration, InternalPri: %d, Mask: 0x%x\n"),
                     pri, (fc_map_state[unit] + pri)->flow_control_mask));
    }

    return(rv);
}

/*
 * Currently only supporting port value of "BCM_GPORT_INVALID"
 */
int
bcm_sirius_cosq_flow_control_get(int unit,
                                 bcm_gport_t port,
                                 bcm_cos_t int_pri,
                                 uint32 *flow_control_mask,
                                 void *info)
{
    int rv = BCM_E_NONE;
    int pri;


    /* consistency checks - port */
    if (port != BCM_GPORT_INVALID) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid gport, unit: %d, gport: 0x%x, expected gport: 0x%x\n"),
	           FUNCTION_NAME(), unit, port, BCM_GPORT_INVALID));
        return(BCM_E_PARAM);
    }

    /* currently only supporting 4 FIFO model */
    if ( (int_pri > (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS - 1)) && (int_pri != -1) ) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid int_pri, unit: %d, int_pri: 0x%x, max int_pri: 0x%x\n"),
	           FUNCTION_NAME(), unit, int_pri, (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS - 1)));
        return(BCM_E_PARAM);
    }

    if (int_pri == -1) {
        /* verify that the setting is consistent */
        for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
            if ((fc_map_state[unit] + 0)->flow_control_mask !=
                                                   (fc_map_state[unit] + pri)->flow_control_mask) {
                return(BCM_E_FAIL);
            }
        }
        (*flow_control_mask) = (fc_map_state[unit] + 0)->flow_control_mask;
    }
    else {
        (*flow_control_mask) = (fc_map_state[unit] + int_pri)->flow_control_mask;
    }

    return(rv);
}

int
bcm_sirius_cosq_mapping_set(int unit,
                            bcm_cos_t priority,
                            bcm_cos_queue_t cosq)
{
    int rv = BCM_E_NONE;


    if (SOC_SBX_CFG(unit)->uInterfaceProtocol != SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Unit: %d, functionality not supported for interface protocol: 0x%x\n"),
	           FUNCTION_NAME(), unit, SOC_SBX_CFG(unit)->uInterfaceProtocol));
        return(BCM_E_UNAVAIL);
    }

    rv = soc_sirius_cos_map_block_entry_set(unit, (SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS +
                                                _SIRIUS_I_COS_PROFILE_GENERAL), priority, cosq);

    return(rv);
}

int
bcm_sirius_cosq_mapping_get(int unit,
                            bcm_cos_t priority,
                            bcm_cos_queue_t *cosq)
{
    int rv = BCM_E_NONE;


    if (SOC_SBX_CFG(unit)->uInterfaceProtocol != SOC_SBX_IF_PROTOCOL_XGS) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Unit: %d, functionality not supported for interface protocol: 0x%x\n"),
	           FUNCTION_NAME(), unit, SOC_SBX_CFG(unit)->uInterfaceProtocol));
        return(BCM_E_UNAVAIL);
    }

    rv = soc_sirius_cos_map_block_entry_get(unit, (SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS +
                                 _SIRIUS_I_COS_PROFILE_GENERAL), priority, (unsigned int *)cosq);

    return(rv);
}

static int
_bcm_sirius_cosq_ingress_flat_scheduler_bucket_select(int unit,
						      int level,
						      int node,
						      int *bucket)
{
    int rv = BCM_E_NONE;
    int num_levels, tmp_node, tmp_level;
    bcm_sbx_sirius_ingress_scheduler_state_t *s_tmp = NULL;

    /* find the root of flat scheduler tree and make sure all nodes in the same
     * level are pointing to one of the node in the flat scheduler tree, we 
     * choose to always use the bucket of the first node on the level in the subtree
     * assuming the BCM_COSQ_NONE will be issued to all subtree nodes, since the APIs
     * are issued topdown, all inner relay nodes will be subtree nodes at certain
     * point, then we don't really need to go back to change the parent's bucket pointer
     * since it was handled already when the BCM_COSQ_NONE was issued to it.
     */

    /* find the converge point, the node which has a mode not equal to BCM_COSQ_NONE
     * we should always find the coverge point since the interface scheduler and
     * port scheduler default to BCM_COSQ_AF0
     */
    tmp_node = is_state[unit][level][node].parent;
    tmp_level = level+1;
    for (num_levels = 1; tmp_level <= SIRIUS_TS_LEVEL_7;
	 tmp_node = s_tmp->parent, tmp_level++, num_levels++) {
	/* we only search up to level 6, where the child port exists */
	s_tmp = &(is_state[unit][tmp_level][tmp_node]);
	if ( (s_tmp->in_use != TRUE) || (s_tmp->num_child <= 0) ) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, ingress scheduler level %d node %d hierachy state inconsistent, Unit(%d)\n"),
	               FUNCTION_NAME(), tmp_level, tmp_node, unit));	    
	    return BCM_E_INTERNAL;
	}
	
	if ( (s_tmp->scheduler_mode != BCM_COSQ_NONE) ) {
	    /* if we found a parent scheduler with a scheduler_mode not equal to BCM_COSQ_NONE
	     * it's the converge point
	     */
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "converge on level %d node %d up %d levels, unit %d\n"),
	              tmp_level, tmp_node, num_levels, unit));
	    break;
	}
    }
    
    /* go down to the level and point the bucket to the first node in the subtree */
    for (; (tmp_level > level) && (tmp_level <= SIRIUS_TS_LEVEL_7); tmp_level--) {
	tmp_node = is_state[unit][tmp_level][tmp_node].first_child;
    }
    *bucket = tmp_node;

    return rv;
}

static int
_bcm_sirius_cosq_map_sp2gsp(int unit, int queue, int32 *p_bw_mode, int32 *p_sp_priority)
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

static int
_bcm_sirius_cosq_update_gsp(int unit, int queue, int cur_q_type, int bw_mode)
{
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int32 bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 base_queue, cur_queue;
    int32 sp_num_cos;
    int rc = BCM_E_NONE, found = FALSE;
    int expected_qtype, hold_ts, q_type;

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    bw_group = p_qstate[queue].bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];
    base_queue = p_bwstate->base_queue;

    for (cur_queue = base_queue, sp_num_cos = 0;
                  cur_queue < (base_queue + p_bwstate->num_cos); cur_queue++) {

        found = FALSE;
        if ( (cur_queue == queue) && (bw_mode == BCM_COSQ_SP) ) {
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
                    expected_qtype = SIRIUS_Q_TYPE_SP7;
                    break;
                case 1:
                    expected_qtype = SIRIUS_Q_TYPE_SP6;
                    break;
                case 2:
                    expected_qtype = SIRIUS_Q_TYPE_SP5;
                    break;
                case 3:
                    expected_qtype = SIRIUS_Q_TYPE_SP4;
                    break;
                case 4:
                    expected_qtype = SIRIUS_Q_TYPE_SP3;
                    break;
                case 5:
                    expected_qtype = SIRIUS_Q_TYPE_SP2;
                    break;
                case 6:
                    expected_qtype = SIRIUS_Q_TYPE_SP1;
                    break;
                case 7:
                default:
                    expected_qtype = SIRIUS_Q_TYPE_SP0;
                    break;
            }

           /* retreive current queue type */
	   rc = soc_sirius_qs_queue_parameter_get(unit, cur_queue, &q_type, &hold_ts);
           if (rc != SOC_E_NONE) {
               LOG_ERROR(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "ERROR: queue param table read failure\n")));
               rc = -1;
               break;
           }
           /* determine if queue type has to be updated */
           if (expected_qtype != q_type) {
               rc = soc_sirius_qs_queue_parameter_set(unit, cur_queue, expected_qtype, hold_ts);
               if (rc != SOC_E_NONE) {
                   LOG_ERROR(BSL_LS_BCM_COSQ,
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


static int
_bcm_sirius_cosq_ingress_shaper_entry_addr_get(int unit, int queue, int *table_p, int *table_index_p)
{
    int                         rc = BCM_E_NONE;
    uint32                      reg_value;
    int32                       voq_shape_inc, voq_shape_start, voq_shape_end, voq_shapers;
    int32                       local_shape_inc, local_shape_start, local_shape_end, local_shapers;
    int                         table_index, table;


    /* get the shaper range config */
    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_STARTr(unit, &reg_value));
    local_shape_inc = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, reg_value, INCf);
    local_shape_start = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_STARTr, reg_value, STARTQUEUEf);

    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_LOCAL_RANGE_ENDr(unit, &reg_value));
    local_shape_end = soc_reg_field_get(unit, SHAPER_QUEUE_LOCAL_RANGE_ENDr, reg_value, ENDQUEUEf);

    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_STARTr(unit, &reg_value));
    voq_shape_inc = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, reg_value, INCf);
    voq_shape_start = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_STARTr, reg_value, STARTQUEUEf);

    BCM_IF_ERROR_RETURN(READ_SHAPER_QUEUE_FABRIC_RANGE_ENDr(unit, &reg_value));
    voq_shape_end = soc_reg_field_get(unit, SHAPER_QUEUE_FABRIC_RANGE_ENDr, reg_value, ENDQUEUEf);

    /* calculate number of shapers for voq queue range */
    voq_shapers = (voq_shape_end - voq_shape_start);
    if (voq_shapers > 0) {
	if (voq_shape_inc == 1) {
	    /* one shaper every 4 queues */
	    voq_shapers = (voq_shapers / 4) + 1;
	}
    }
    else if (voq_shapers < 0) {
	/* misconfiged, assuming no voq shapers */
	voq_shapers = 0;
    }

    /* calculate number of shapers for local queue range */
    local_shapers = (local_shape_end - local_shape_start);
    if (local_shapers > 0) {
	if (local_shape_inc == 1) {
	    /* one shaper every 4 queues */
	    local_shapers = (local_shapers / 4) + 1;
	}
    }
    else if (local_shapers < 0) {
	/* misconfiged, assuming no voq shapers */
	local_shapers = 0;
    }

    /* make sure the queues picked has shaper resource, assuming local shapers is */
    /* after the voq shapers. And caculate the shaper resource                    */
    if ( (voq_shapers > 0) && (queue >= voq_shape_start) && (queue <= voq_shape_end) ) {
	/* fabric queues */
	if (voq_shape_inc == 1) {
	    if ((queue - voq_shape_start) % 4 != 0) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s no hardware ingress shaper resource for VOQ queue %d \n"),
                           FUNCTION_NAME(), queue));
                return(BCM_E_RESOURCE);
	    }
            else {
		table_index = (queue - voq_shape_start)/4;
	    }
	}
        else {
	    table_index = queue - voq_shape_start;
	}
    }
    else if ( (local_shapers > 0) && (queue >= local_shape_start) && (queue <= local_shape_end) ) {
	/* local queues */
	if (local_shape_inc == 1) {
	    if ( ((queue - local_shape_start) % 4) != 0) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s no hardware ingress shaper resource for Local queue %d \n"),
                           FUNCTION_NAME(), queue));
                return(BCM_E_RESOURCE);
	    }
            else {
		table_index = (queue - local_shape_start) / 4 + voq_shapers;
	    }
	}
        else {
	    table_index = (queue - local_shape_start) + voq_shapers;
	}
    }
    else {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s no hardware ingress shaper resource for queue %d \n"),
	           FUNCTION_NAME(), queue));
	return(BCM_E_RESOURCE);
    }

    /* calculate table and table entry index */
    table = table_index & 3;
    table_index = table_index >> 2;

    (*table_p) = table;
    (*table_index_p) = table_index;

    return(rc);
}

static int
_bcm_sirius_cosq_ingress_shaper_entry_get(int unit, int queue, int table, int table_index, shaper_leak_0_entry_t *shaper_leak_entry_p)
{
    int                         rc = BCM_E_NONE;


    if (table == 0) {
        rc = READ_SHAPER_LEAK_0m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
    }
    else if (table == 1) {
        rc = READ_SHAPER_LEAK_1m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
    }
    else if (table == 2) {
        rc = READ_SHAPER_LEAK_2m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
    }
    else {
        rc = READ_SHAPER_LEAK_3m(unit, MEM_BLOCK_ANY, table_index, shaper_leak_entry_p);
    }

    return(rc);
}


static int
_bcm_sirius_cosq_ingress_shaper_default_burst_get(int unit, int queue, uint32 *burst_size)
{
    int                         rc = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    uint64                      round_ns;
    uint32                      min_burst_size;


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    if (p_qstate[queue].ingress.enable_shaping == FALSE) {
        (*burst_size) = 0;
        return(rc);
    }

    COMPILER_64_SET(round_ns, 0, 16 * (1<<10) * 1000 / SOC_SBX_CFG(unit)->uClockSpeedInMHz);
    COMPILER_64_UMUL_32(round_ns, p_qstate[queue].ingress.shape_limit_kbps);
    rc = soc_sbx_div64(round_ns, 1000000, &min_burst_size);
    if (rc != SOC_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s failed to calculate burst size for rate of %d kbps\n"),
                   FUNCTION_NAME(), p_qstate[queue].ingress.shape_limit_kbps));
        return(rc);
    }
    else {
        /* make sure burst_size is at least 3 times the amount of leaked credit */
        min_burst_size = min_burst_size * 3;

        /* set the burst size big enough to pass a MTU 1518 bytes */
        if (min_burst_size <= (1518 * 8)) {
            min_burst_size = 1518 * 8;
        }

        if (min_burst_size > SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE) {
            min_burst_size = SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE;
        }

        (*burst_size) = min_burst_size;
    }

    return(rc);
}

static int
_bcm_sirius_cosq_ingress_shaper_burst_set(int unit, int queue, int burst_size_kbits)
{
    int                         rc = BCM_E_NONE;
    int                         table_index, table;
    shaper_leak_0_entry_t       shaper_leak_entry;
    uint32                      burst_exp, burst_mant;
    int                         burst_size_bits;
    uint32                      min_burst_size;
    bcm_sbx_cosq_queue_state_t *p_qstate;


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    rc =  _bcm_sirius_cosq_ingress_shaper_entry_addr_get(unit, queue, &table, &table_index);
    if (rc != BCM_E_NONE) {
        if ((rc == BCM_E_RESOURCE) && ((burst_size_kbits == 0) || (burst_size_kbits == -1))) {
            p_qstate[queue].ingress.shape_burst_kbps = burst_size_kbits;
            rc = BCM_E_NONE;
        }
        return(rc);
    }

    /* retreive entry */
    rc = _bcm_sirius_cosq_ingress_shaper_entry_get(unit, queue, table, table_index, &shaper_leak_entry);
    if (rc != SOC_E_NONE) {
        return(rc);
    }

    if (burst_size_kbits != -1) {
        if (p_qstate[queue].ingress.enable_shaping == TRUE) {
            burst_size_bits = burst_size_kbits * 1000;
            if (burst_size_bits > SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE) {
                burst_size_bits = SIRIUS_INGRESS_SHAPER_MAX_BURST_SIZE;
            }
            rc = soc_sbx_fabric_util_num_to_mant_exp(unit, SHAPER_LEAK_0m, SHAPE_THRESH_EXPf,
                                               burst_size_bits, &burst_mant, &burst_exp);
            if (rc != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s failed to convert burst size %d bits to mant/exp format\n"),
                           FUNCTION_NAME(), burst_size_bits));
                return(rc);
            }	
        }
        else {
            burst_mant = burst_exp = 0;
        }
    }
    else {
        if (p_qstate[queue].ingress.enable_shaping == TRUE) {
            rc = _bcm_sirius_cosq_ingress_shaper_default_burst_get(unit, queue, &min_burst_size);
            if (rc != SOC_E_NONE) {
                return(rc);
            }

            rc = soc_sbx_fabric_util_num_to_mant_exp(unit, SHAPER_LEAK_0m, SHAPE_THRESH_EXPf,
                                               min_burst_size, &burst_mant, &burst_exp);
            if (rc != SOC_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COSQ,
                          (BSL_META_U(unit,
                                      "ERROR: %s failed to convert burst size %d bits to mant/exp format\n"),
                           FUNCTION_NAME(), min_burst_size));
                return(rc);
            }	
        }
        else {
            burst_mant = burst_exp = 0;
        }
    }

    /* modify configuration */
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry, SHAPE_THRESH_EXPf, burst_exp);
    soc_mem_field32_set(unit, SHAPER_LEAK_0m, &shaper_leak_entry, SHAPE_THRESH_MANTf, burst_mant);

    /* configure entry */
    rc = _bcm_sirius_cosq_ingress_shaper_entry_set(unit, queue, table, table_index, &shaper_leak_entry);
    if (rc != SOC_E_NONE) {
        return(rc);
    }

    p_qstate[queue].ingress.shape_burst_kbps = burst_size_kbits;

    return(rc);
}

static int
_bcm_sirius_cosq_ingress_shaper_burst_get(int unit, int queue, int *burst_size_kbits)
{
    int                         rc = BCM_E_NONE;
    uint32                      min_burst_size;
    bcm_sbx_cosq_queue_state_t *p_qstate;


    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    if (p_qstate[queue].ingress.shape_burst_kbps != -1) {
        (*burst_size_kbits) = p_qstate[queue].ingress.shape_burst_kbps;
    }
    else {
        rc = _bcm_sirius_cosq_ingress_shaper_default_burst_get(unit, queue, &min_burst_size);
        if (rc != SOC_E_NONE) {
            return(rc);
        }

        (*burst_size_kbits) = min_burst_size / 1000;
    }

    return(rc);
}

static int
_bcm_sirius_cosq_qsel_offset_set(int unit, bcm_gport_t gport, int value)
{
    int                             rc = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t     *p_qstate = NULL;
    bcm_sbx_cosq_queue_state_t     *p_base_qstate = NULL;
    bcm_sbx_cosq_bw_group_state_t  *p_bwstate;
    int32                           base_queue = 0, bw_group =0;
    int                             num_cos = 0;
    int32                           dest_switch_modid;
    int32                           dest_switch_port;
    bcm_gport_t                     bw_state_gport;


    if (SOC_SBX_CFG(unit)->uInterfaceProtocol != SOC_SBX_IF_PROTOCOL_XGS) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, Unit: %d, functionality not supported for interface protocol: 0x%x\n"),
                   FUNCTION_NAME(), unit, SOC_SBX_CFG(unit)->uInterfaceProtocol));
        return(BCM_E_UNAVAIL);
    }
    if ( !(BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
           BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
           BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
           BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) ) {
        return(BCM_E_PARAM);
    }
    if (soc_sirius_cos_map_block_check(unit, value) != SOC_E_FULL) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, Unit: %d, cos_map: %d not present\n"),
                   FUNCTION_NAME(), unit, value));
        return(BCM_E_PARAM);
    }
    
    if (_bcm_sbx_cosq_is_all(unit, gport)) {
        /* configuration for subsequent queue's */

        if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
                           BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
            SOC_SBX_CFG_SIRIUS(unit)->ucast_cos_map = value;
        }
        else {
            SOC_SBX_CFG_SIRIUS(unit)->mcast_cos_map = value;
        }

        return(rc);
    }

    BCM_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));

    if (SHR_BITGET(SOC_SBX_SIRIUS_STATE(unit)->inhibitQsel, base_queue)) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, Unit: %d, quue not part of standard SDK model\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_PARAM);
    }

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_base_qstate = &p_qstate[base_queue];
    bw_group = p_base_qstate->bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        /* cos map is always updated for unicast */
        p_bwstate->cos_map = value;
        bw_state_gport = p_bwstate->gport;
        rc = _bcm_sirius_cosq_destmod_destport_get(unit,
                                                   base_queue,
                                                   bw_state_gport,
                                                   &dest_switch_modid,
                                                   &dest_switch_port);
        if (BCM_E_NONE == rc) {
            /* was able to get dest port info, update qsel entry */
            rc = soc_sirius_rb_higig2_header_unicast_queue_map_config(unit,
                                                                      dest_switch_modid,
                                                                      dest_switch_port,
                                                                      base_queue);
        }
    } else {
        /* currently update cos map if not associated with any mcGroup */
        
        if (p_base_qstate->mgid_list != NULL) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Unit: %d, mgid list present\n"),
                       FUNCTION_NAME(), unit));
            return(BCM_E_PARAM);
        }
        p_bwstate->cos_map = value;
    }

    return(rc);
}

static int
_bcm_sirius_cosq_qsel_offset_get(int unit, bcm_gport_t gport, int *value)
{
    int                             rc = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t     *p_qstate = NULL;
    bcm_sbx_cosq_queue_state_t     *p_base_qstate = NULL;
    bcm_sbx_cosq_bw_group_state_t  *p_bwstate;
    int32                           base_queue = 0, bw_group = 0;
    int                             num_cos = 0;


    if (SOC_SBX_CFG(unit)->uInterfaceProtocol != SOC_SBX_IF_PROTOCOL_XGS) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, Unit: %d, functionality not supported for interface protocol: 0x%x\n"),
                   FUNCTION_NAME(), unit, SOC_SBX_CFG(unit)->uInterfaceProtocol));
        return(BCM_E_UNAVAIL);
    }
    if ( !(BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
           BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
           BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
           BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) ) {
        return(BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos));

    if (SHR_BITGET(SOC_SBX_SIRIUS_STATE(unit)->inhibitQsel, base_queue)) {
        return(BCM_E_PARAM);
    }

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_base_qstate = &p_qstate[base_queue];
    bw_group = p_base_qstate->bw_group;
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];
    (*value) = p_bwstate->cos_map;

    return(rc);
}


int bcm_sirius_fd_fct_get(int unit, int32 sysport, int32 ef, int32 mc, int32 *fct)
{
  BCM_IF_ERROR_RETURN(soc_sirius_fd_fct_get(unit, sysport, ef, mc, fct));
  return BCM_E_NONE;
}


static int bcm_sirius_cosq_ingress_scheduler_node_profile_get(int unit, int level, int node,
                                                                 int mode, int *node_profile)
{
    int         rc = BCM_E_NONE;


    if (SOC_SBX_CFG(unit)->node_template_id == SOC_SBX_NODE_QOS_TEMPLATE_TYPE0) {
        switch (mode) {
	    case BCM_COSQ_AF0:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF0;
	        break;
	    case BCM_COSQ_AF1:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF1;
	        break;
	    case BCM_COSQ_AF2:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF2;
	        break;
	    case BCM_COSQ_AF3:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_AF3;
	        break;
	    case BCM_COSQ_SP0:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP0;
	        } else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0;
	        }
	        break;
	    case BCM_COSQ_SP1:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP1;
	        } else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1;
	        }
	        break;
	    case BCM_COSQ_BE:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_BE;
	        break;
	    case BCM_COSQ_NONE:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT;
	        break;
	    default:
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, ingress scheduler level %d node %d unsupported sched mode %d, Unit(%d)\n"),
	                   FUNCTION_NAME(), level, node, mode, unit));	    
	        return BCM_E_PARAM;
        }
    }
    else if (SOC_SBX_CFG(unit)->node_template_id == SOC_SBX_NODE_QOS_TEMPLATE_TYPE1) {
        switch (mode) {
	    case BCM_COSQ_SP0:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP0;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0;
	        }
	        break;
	    case BCM_COSQ_SP1:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP1;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1;
	        }
	        break;
	    case BCM_COSQ_SP2:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP2;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP2;
	        }
	        break;
	    case BCM_COSQ_SP3:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP3;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP3;
	        }
	        break;
	    case BCM_COSQ_SP4:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP4;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP4;
	        }
	        break;
	    case BCM_COSQ_SP5:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP5;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP5;
	        }
	        break;
	    case BCM_COSQ_SP6:
	        if (is_state[unit][level][node].min_shaper_rate_kbps == 0) {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_SP6;
	        }
                else {
		    (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP6;
	        }
	        break;
	    case BCM_COSQ_AF0:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP0;
	        break;
	    case BCM_COSQ_AF1:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP1;
	        break;
	    case BCM_COSQ_AF2:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP2;
	        break;
	    case BCM_COSQ_AF3:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP3;
	        break;
	    case BCM_COSQ_AF4:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP4;
	        break;
	    case BCM_COSQ_AF5:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP5;
	        break;
	    case BCM_COSQ_AF6:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_RCPQ_SP6;
	        break;
	    case BCM_COSQ_NONE:
	        (*node_profile) = SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT;
	        break;
	    default:
	        LOG_ERROR(BSL_LS_BCM_COSQ,
	                  (BSL_META_U(unit,
	                              "ERROR: %s, ingress scheduler level %d node %d unsupported sched mode %d, Unit(%d)\n"),
	                   FUNCTION_NAME(), level, node, mode, unit));	    
	        return(BCM_E_PARAM);
        }
    }
    else {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, unsupportd node scheduling discipline template %d, Unit(%d)\n"),
                   FUNCTION_NAME(), SOC_SBX_CFG(unit)->node_template_id, unit));
        return(BCM_E_PARAM);
    }

    return(rc);
}

static int _bcm_sirius_cosq_scheduler_adopt_all_priority_set(int unit, bcm_gport_t gport,
			    bcm_cos_queue_t cosq, bcm_cosq_control_t type, int value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int         rc = BCM_E_NONE;
    int         level, node, node_profile, mode;


    if (!(BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport))) {		    
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport control set, adopt pri setting not support on gport(0x%x) on unit %d\n"),
                  gport, unit));
        return(BCM_E_UNAVAIL);
    }

    rc = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
    if (rc != BCM_E_NONE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport control set, unsupported gport on unit %d\n"),
                  unit));
        rc = BCM_E_PARAM;
        return(rc);
    }

    if ((value != TRUE) && (value != FALSE)) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport control set - adopt all priority, unsupported value (%d) on unit %d\n"),
                  value, unit));
        rc = BCM_E_PARAM;
        return(rc);
    }

    if (value == TRUE) {
        node_profile = SIRIUS_TS_NODE_PROFILE_TEMPLATE_DEFAULT;
        is_state[unit][level][node].adopt_all = TRUE;
    }
    else {
        mode = is_state[unit][level][node].scheduler_mode;
        rc = bcm_sirius_cosq_ingress_scheduler_node_profile_get(unit, level, node, mode, &node_profile);
        if (rc != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, in getting ingress scheduler node profile (0x%x)\n"),
                       FUNCTION_NAME(), rc));
            return(rc);
        }
        is_state[unit][level][node].adopt_all = FALSE;
    }
    rc = soc_sirius_ts_node_creditor_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                                 SOC_SIRIUS_API_PARAM_NO_CHANGE, node_profile);

    return(rc);
}

static int _bcm_sirius_cosq_scheduler_adopt_all_priority_get(int unit, bcm_gport_t gport,
			    bcm_cos_queue_t cosq, bcm_cosq_control_t type, int *value,
                            bcm_sbx_cosq_control_t *control_info)
{
    int         rc = BCM_E_NONE;
    int         level, node;


    if (!(BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_INGRESS(gport))) {		    
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport control get, adopt pri setting not support on gport(0x%x) on unit %d\n"),
                  gport, unit));
        return(BCM_E_UNAVAIL);
    }

    rc = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
    if (rc != BCM_E_NONE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport control get, unsupported gport on unit %d\n"),
                  unit));
        rc = BCM_E_PARAM;
        return(rc);
    }

    if (value == NULL) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport control get - value is a NULL pointer unit %d\n"),
                  unit));
        rc = BCM_E_PARAM;
        return(rc);
    }

    (*value) = (is_state[unit][level][node].adopt_all == TRUE) ? TRUE : FALSE;

    return(rc);
}

int
bcm_sirius_cosq_multipath_allocate(int unit, int egress, int *p_multipath)
{
    int rv = BCM_E_NONE;

    if (egress) {
	/* allocate a group shaper */
	rv = bcm_sirius_cosq_group_shaper_allocate(unit, TRUE, p_multipath);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: failed to allocate egress group max shaper\n")));
	}
    } else {
	/* do nothing, there is no physical resource associated with mulipath ingress
	 * shapers
	 */
    }

    return rv;
}

int
bcm_sirius_cosq_multipath_free(int unit, int egress, int multipath)
{
    int rv = BCM_E_NONE;

    if (egress) {
	/* free a group shaper */
	rv = bcm_sirius_cosq_group_shaper_free(unit, TRUE, multipath);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: failed to free egress group max shaper\n")));
	}
    } else {
	/* do nothing, there is no physical resource associated with mulipath ingress
	 * shapers
	 */
    }

    return rv;
}

int
bcm_sirius_cosq_multipath_add(int unit, int egress, int multipath, int level, int node)
{
    bcm_sbx_cosq_ingress_multipath_state_t *ps_i_state;
    bcm_sbx_cosq_egress_multipath_state_t *ps_e_state;
    int node_type, bucket_type, bucket_ptr;
    uint32 member[BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE];
    int i, j, tmp_node, head_node, prev_node, last_node, num_nodes;
    int rv = BCM_E_NONE;

    if (egress) {
	if (SOC_SBX_STATE(unit)->egress_multipath_state != NULL) {
	    ps_e_state = SOC_SBX_STATE(unit)->egress_multipath_state + multipath;
	} else {
	    return BCM_E_UNAVAIL;
	}

	/* enable multipath shaper for all matching fifos */
	if ((level < SIRIUS_ES_LEVEL_FIFO) || (level > SIRIUS_ES_LEVEL_INTERFACE) ||
	    (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level])) {
	    return BCM_E_PARAM;
	}

	for (i = 0; i < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; i++) {
	    member[i]=0;
	}

	/* go through all enable fifos and point to matching fifos to the multipath shaper */
	for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[0]; tmp_node++) {
	    if (es_state[unit][0][tmp_node].in_use == FALSE) {
		continue;
	    }
	    switch (level) {
		case SIRIUS_ES_LEVEL_FIFO:
		    if (tmp_node == node){
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
		case SIRIUS_ES_LEVEL_SUBPORT:
		    /* any fifos match the specified subport node */
		    if (es_state[unit][0][tmp_node].subport == node) {
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
		case SIRIUS_ES_LEVEL_CHANNEL:
		    /* any fifos match the specified channel node */
		    if (es_state[unit][0][tmp_node].channel == node) {
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
		case SIRIUS_ES_LEVEL_INTERFACE:
		    /* any fifos match the specified interface node */
		    if (es_state[unit][0][tmp_node].intf == node) {
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
                /* coverity[dead_error_begin] */
		default:
		    break;
	    }
	}

	for (i = 0; i < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; i++) {
	    for (j = 0; j < (sizeof(uint32)*8); j++) {
		if ( (member[i] & (1<<j)) != 0 ) {
		    rv = soc_sirius_es_node_group_shaper_member_config(unit, 
								       ps_e_state->node,
								       i*32+j,
								       TRUE);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to enable egress multipath shaper for fifo %d on unit %d\n"),
			           FUNCTION_NAME(), (i*32+j), unit));
			return rv;
		    }
		    ps_e_state->member[i] |= (1<<j);
		}
	    }
	}	
    } else {
	if (SOC_SBX_STATE(unit)->ingress_multipath_state != NULL) {
	    ps_i_state = SOC_SBX_STATE(unit)->ingress_multipath_state + multipath;
	} else {
	    return BCM_E_UNAVAIL;
	}

	if ((level < 2) || (level > 6)) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, ingress multipath only support level [2-6] on unit %d\n"),
                       FUNCTION_NAME(), unit));
	    return BCM_E_PARAM;
	}

	/* make sure the node is not a relay node */
	rv = soc_sirius_ts_node_mapping_config_get(unit, level, node, &node_type,
						   &bucket_type, &bucket_ptr);
	if (rv != SOC_E_NONE) {
	    return rv;
	} else if (bucket_type == SIRIUS_TS_BUCKET_TYPE_SUBTREE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, level %d node %d is a relay node, can not be part of multipath\n"),
                       FUNCTION_NAME(), level, node));	    
	    return BCM_E_PARAM;
	}

	if (ps_i_state->num_nodes == 0) {
	    /* current node is master */
	    rv = soc_sirius_ts_node_mapping_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						   SIRIUS_TS_BUCKET_TYPE_MULTIPATH_MASTER, node);
	    if (rv == SOC_E_NONE) {
		ps_i_state->node = node;
		is_state[unit][level][node].bucket = node;
	    } else {
		return rv;
	    }
	} else {
	    /* master already picked */
	    rv = soc_sirius_ts_node_mapping_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						   SIRIUS_TS_BUCKET_TYPE_MULTIPATH,
						   ps_i_state->node);
	    if (rv == SOC_E_NONE) {
		is_state[unit][level][node].bucket = ps_i_state->node;
	    } else {
		return rv;
	    }
	}

	/* update the multipath group control link list */
	head_node = ps_i_state->node;
	last_node = head_node;
	prev_node = head_node;
	num_nodes = 0;
	for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; tmp_node++) {
	    if ((is_state[unit][level][tmp_node].in_use == TRUE) &&
		(is_state[unit][level][tmp_node].bucket == head_node)) {
		if (tmp_node != head_node) {
		    rv = soc_sirius_ts_node_multipath_config(unit, level, prev_node,
							     tmp_node, FALSE);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		    prev_node = tmp_node;
		    last_node = tmp_node;
		}
		num_nodes++;
		if (num_nodes == (ps_i_state->num_nodes+1)) {
		    break;
		}
	    }
	}
	rv = soc_sirius_ts_node_multipath_config(unit, level, last_node,
						 head_node, TRUE);
	if (rv != SOC_E_NONE) {
	    return rv;
	}
    }    

    return rv;
}

int
bcm_sirius_cosq_multipath_delete(int unit, int egress, int multipath, int level, int node)
{
    bcm_sbx_cosq_ingress_multipath_state_t *ps_i_state;
    bcm_sbx_cosq_egress_multipath_state_t *ps_e_state;
    uint32 member[BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE];
    int new_master;
    int i, j, tmp_node, head_node, prev_node, last_node, num_nodes;
    int enable;
    int rv = BCM_E_NONE;

    if (egress) {
	if (SOC_SBX_STATE(unit)->egress_multipath_state != NULL) {
	    ps_e_state = SOC_SBX_STATE(unit)->egress_multipath_state + multipath;
	} else {
	    return BCM_E_UNAVAIL;
	}

	/* disable multipath shaper for all matching fifos */
	if ((level < SIRIUS_ES_LEVEL_FIFO) || (level > SIRIUS_ES_LEVEL_INTERFACE) ||
	    (node < 0) || (node >= SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level])) {
	    return BCM_E_PARAM;
	}

	for (i = 0; i < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; i++) {
	    member[i]=0;
	}

	/* go through all enable fifos and point to matching fifos to the multipath shaper */
	for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[0]; tmp_node++) {
	    if (es_state[unit][0][tmp_node].in_use == FALSE) {
		continue;
	    }
	    switch (level) {
		case SIRIUS_ES_LEVEL_FIFO:
		    if (tmp_node == node){
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
		case SIRIUS_ES_LEVEL_SUBPORT:
		    /* any fifos match the specified subport node */
		    if (es_state[unit][0][tmp_node].subport == node) {
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
		case SIRIUS_ES_LEVEL_CHANNEL:
		    /* any fifos match the specified channel node */
		    if (es_state[unit][0][tmp_node].channel == node) {
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
		case SIRIUS_ES_LEVEL_INTERFACE:
		    /* any fifos match the specified interface node */
		    if (es_state[unit][0][tmp_node].intf == node) {
			member[tmp_node/32] |= (1 << (tmp_node % 32));
		    }
		    break;
                /* coverity[dead_error_begin] */
		default:
		    break;
	    }
	}

	for (i = 0; i < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; i++) {
	    for (j = 0; j < (sizeof(uint32)*8); j++) {
		if ( (member[i] & (1<<j)) != 0 ) {
		    rv = soc_sirius_es_node_group_shaper_member_config(unit, 
								       ps_e_state->node,
								       i*32+j,
								       FALSE);
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to disable egress multipath shaper for fifo %d on unit %d\n"),
			           FUNCTION_NAME(), (i*32+j), unit));
			return rv;
		    }
		    ps_e_state->member[i] &= (~(1<<j));
		}
	    }
	}	
    } else {
	if (SOC_SBX_STATE(unit)->ingress_multipath_state != NULL) {
	    ps_i_state = SOC_SBX_STATE(unit)->ingress_multipath_state + multipath;
	} else {
	    return BCM_E_UNAVAIL;
	}

	if (ps_i_state->node == node) {
	    /* deleting master node */
	    rv = soc_sirius_ts_node_mapping_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						   SIRIUS_TS_BUCKET_TYPE_BUCKET, node);
	    if (rv == SOC_E_NONE) {
		is_state[unit][level][node].bucket = node;
	    }

	    if (ps_i_state->num_nodes > 1) {
		/* need to select a new master node, go through all nodes that point to old master
		 * node, pick the first one as the new master, we should be able to find 
		 * such node here.
		 */
		new_master = -1;
		for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; tmp_node++) {
		    if ((is_state[unit][level][tmp_node].in_use == TRUE) &&
			(is_state[unit][level][tmp_node].bucket == node) &&
			(tmp_node != node)) {
			if (new_master < 0) {
			    /* pick a new master */
			    new_master = tmp_node;
			    rv = soc_sirius_ts_node_mapping_config(unit, level, tmp_node,
								   SOC_SIRIUS_API_PARAM_NO_CHANGE,
								   SIRIUS_TS_BUCKET_TYPE_MULTIPATH_MASTER,
								   new_master);
			    if (rv == SOC_E_NONE) {
				is_state[unit][level][tmp_node].bucket = new_master;
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, ingress multipath failed to config new multipath master on unit %d\n"),
				           FUNCTION_NAME(), unit));
				return rv;
			    }
			} else {
			    /* point other nodes in same multipath to the new master */	
			    rv = soc_sirius_ts_node_mapping_config(unit, level, tmp_node,
								   SOC_SIRIUS_API_PARAM_NO_CHANGE,
								   SOC_SIRIUS_API_PARAM_NO_CHANGE,
								   new_master);
			    if (rv == SOC_E_NONE) {
				is_state[unit][level][tmp_node].bucket = new_master;
			    } else {
				LOG_ERROR(BSL_LS_BCM_COSQ,
				          (BSL_META_U(unit,
				                      "ERROR: %s, ingress multipath failed to point to new multipath master on unit %d\n"),
				           FUNCTION_NAME(), unit));
				return rv;
			    }
			}
		    }
		}
		if (new_master < 0) {
		    /* this should never happen */
		    return BCM_E_INTERNAL;
		} else {
		    /* copy over the old master's shaper parameters */
		    is_state[unit][level][new_master].max_shaper_rate_kbps      = is_state[unit][level][node].max_shaper_rate_kbps;
		    is_state[unit][level][new_master].max_shaper_threshold_bits = is_state[unit][level][node].max_shaper_threshold_bits;

		    if (is_state[unit][level][new_master].max_shaper_rate_kbps == 0) {
			enable = FALSE;
		    } else {
			enable = TRUE;
		    }
		    rv = soc_sirius_ts_node_shaper_config(unit, level, new_master, enable,
							  is_state[unit][level][new_master].max_shaper_rate_kbps,
							  is_state[unit][level][new_master].max_shaper_threshold_bits);
		    
		    if (rv != SOC_E_NONE) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "ERROR: %s, failed to config max ingress shaper for level %d node %d, Unit(%d)\n"),
			           FUNCTION_NAME(), level, new_master, unit));
			return rv;
		    }
		    ps_i_state->node = new_master;
		}
	    }

	    /* remove the old master's shaper parameter */
	    is_state[unit][level][node].max_shaper_rate_kbps = 0;
	    is_state[unit][level][node].max_shaper_threshold_bits = 0;
	    rv = soc_sirius_ts_node_shaper_config(unit, level, node, FALSE, 0, 0);
	    if (rv != SOC_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to config max ingress shaper for level %d node %d, Unit(%d)\n"),
		           FUNCTION_NAME(), level, node, unit));
		return rv;
	    }
	} else {
	    /* deleting non-master node */
	    rv = soc_sirius_ts_node_mapping_config(unit, level, node, SOC_SIRIUS_API_PARAM_NO_CHANGE,
						   SIRIUS_TS_BUCKET_TYPE_BUCKET, node);
	    if (rv == SOC_E_NONE) {
		is_state[unit][level][node].bucket = node;
	    }
	}

	/* clear the multipath group control table for the deleted node */
	rv = soc_sirius_ts_node_multipath_config(unit, level, node, 0, FALSE);
	if (rv != SOC_E_NONE) {
	    return rv;
	}

	/* update the multipath group control link list */
	head_node = ps_i_state->node;
	last_node = head_node;
	prev_node = head_node;
	num_nodes = 0;
	for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; tmp_node++) {
	    if ((is_state[unit][level][tmp_node].in_use == TRUE) &&
		(is_state[unit][level][tmp_node].bucket == head_node)) {
		if (tmp_node != head_node) {
		    rv = soc_sirius_ts_node_multipath_config(unit, level, prev_node,
							     tmp_node, FALSE);
		    if (rv != SOC_E_NONE) {
			return rv;
		    }
		    prev_node = tmp_node;
		    last_node = tmp_node;
		}
		num_nodes++;
		if (num_nodes == (ps_i_state->num_nodes-1)) {
		    break;
		}
	    }
	}
	rv = soc_sirius_ts_node_multipath_config(unit, level, last_node,
						 head_node, TRUE);
	if (rv != SOC_E_NONE) {
	    return rv;
	}
    }    

    return rv;
}

int
bcm_sirius_cosq_multipath_get(int unit, int egress, int multipath,
			      int *member_count,
			      bcm_gport_t *member_gport_array,
			      int *member_gport_info_array)
{
    bcm_sbx_cosq_ingress_multipath_state_t *ps_i_state;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int level, node, tmp_node, port = 0, found;
    int max_member = *member_count, num_member=0;
    int mymodid;
    int rv = BCM_E_NONE;

    if (egress) {
	rv = BCM_E_UNAVAIL;
    } else {
	if (SOC_SBX_STATE(unit)->ingress_multipath_state != NULL) {
	    ps_i_state = SOC_SBX_STATE(unit)->ingress_multipath_state + multipath;
	} else {
	    return BCM_E_UNAVAIL;
	}

	BCM_IF_ERROR_RETURN(bcm_stk_modid_get(unit, &mymodid));

	level = ps_i_state->level;
	node = ps_i_state->node;
	for (tmp_node = 0; tmp_node < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; tmp_node++) {
	    if (num_member >= max_member) {
		break;
	    }

	    if ((is_state[unit][level][tmp_node].in_use == TRUE) &&
		(is_state[unit][level][tmp_node].bucket == node)) {
		if (is_state[unit][level][tmp_node].logical_scheduler < 0) {
		    /* child gport, return node ID */
		    found = FALSE;
		    if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
			for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; port++) {
			    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[port]);
			    if (sp_info->valid == FALSE) {
				continue;
			    }
			    if ((sp_info->ts_scheduler_level == level) &&
				(sp_info->ts_scheduler_node == tmp_node)) {
				rv = BCM_E_NONE;
				found = TRUE;
				break;
			    }
			}
		    }

		    if (found) {
			BCM_GPORT_CHILD_SET((*(member_gport_array+num_member)), mymodid, port);
		    } else {
			(*(member_gport_array+num_member)) = BCM_GPORT_INVALID;
		    }
		} else {
		    /* scheduler gport */
		    BCM_INT_SBX_SCHEDULER_GPORT_SET((*(member_gport_array+num_member)), FALSE,
						    is_state[unit][level][tmp_node].logical_scheduler);
		}
		(*(member_gport_info_array+num_member)) = 0;
		num_member++;
	    }	
	}
	*member_count = num_member;
    }

    return rv;
}

static int
_bcm_sirius_cosq_p2l_scheduler(int unit,
			       int egress,
			       int level,
			       int node,
			       int *logical_scheduler) {
    int rv = BCM_E_NOT_FOUND;
    int ls;
    bcm_sbx_cosq_egress_scheduler_state_t *p_e_scheduler;

    *logical_scheduler = -1;

    if (egress) {
	p_e_scheduler = SOC_SBX_STATE(unit)->egress_scheduler_state;
	if (p_e_scheduler != NULL) {
            for (ls = 0; ls < SOC_SBX_CFG(unit)->num_egress_scheduler; ls++) {
                if (((p_e_scheduler + ls)->in_use == TRUE) &&
                    ((p_e_scheduler + ls)->level == level) &&
                    ((p_e_scheduler + ls)->node == node)) {
		    *logical_scheduler = ls;
		    rv = BCM_E_NONE;
                    break;
                }
            }	    
	}
    }

    return rv;
}

int bcm_sirius_cosq_pfc_config_set(int unit, 
				bcm_gport_t gport, 
				bcm_cos_queue_t cosq, 
				uint32 flags, 
				bcm_cosq_pfc_config_t *config)
{
    int       rv = BCM_E_NONE;
    uint16    dev_id;
    uint8     rev_id;
    int       hysteresis, class;
    uint32    regval = 0, regaddr;
    uint32    lossless, reserved_buffs, num_buffs;
    soc_reg_t xon_reg, xoff_reg, max_reg, rsvd_reg, sp_xon_reg, sp_xoff_reg, sp_max_reg;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    if (!soc_feature(unit, soc_feature_priority_flow_control)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, priority flow control not supported on Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_UNAVAIL;
    }
    
    /* make sure cosq is in range */
    if ((cosq < 0) || (cosq >= 8)) {

        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, cosq %d out of valid range [0-7], Unit(%d)\n"),
                   FUNCTION_NAME(), cosq, unit));
        return BCM_E_PARAM;

    }

    if (gport == BCM_GPORT_INVALID) {
        if (config->xoff_threshold > 0x3FFFF) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, xoff_threshold 0x%x can not be larger than 0x3FFFF, Unit(%d)\n"),
                       FUNCTION_NAME(), config->xoff_threshold, unit));
            return BCM_E_PARAM;
        }
        /* Sirius PG threshold allocation for DCB pkt loss in 10G 256 port mesh test */
        if (config->xon_threshold > 0x3FFFF) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, xon_threshold 0x%x can not be larger than 0x3FFFF, Unit(%d)\n"),
                       FUNCTION_NAME(), config->xon_threshold, unit));
            return BCM_E_PARAM;
        }
        if (config->drop_threshold > 0x3FFFF) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, drop_threshold 0x%x can not be larger than 0x3FFFF, Unit(%d)\n"),
                       FUNCTION_NAME(), config->drop_threshold, unit));
            return BCM_E_PARAM;
        }
    } else {
        if (config->xoff_threshold > 0xFFFFFFF) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, xoff_threshold 0x%x can not be larger than 0xFFFFFF, Unit(%d)\n"),
                       FUNCTION_NAME(), config->xoff_threshold, unit));
            return BCM_E_PARAM;
        }
        if (config->xon_threshold > 0xFFFFFFF) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, xon_threshold 0x%x can not be larger than 0xFFFFFFF, Unit(%d)\n"),
                       FUNCTION_NAME(), config->xon_threshold, unit));
            return BCM_E_PARAM;
        }
        if (config->drop_threshold > 0xFFFFFFF) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, drop_threshold 0x%x can not be larger than 0xFFFFFFF, Unit(%d)\n"),
                       FUNCTION_NAME(), config->drop_threshold, unit));
            return BCM_E_PARAM;
        }
    }

    if (config->xon_threshold >= config->xoff_threshold) {
        LOG_ERROR(BSL_LS_BCM_COSQ,
                  (BSL_META_U(unit,
                              "ERROR: %s, xon_threshold 0x%x can not equal or larger"
                               "than xoff_threshold 0x%x, Unit(%d)\n"), FUNCTION_NAME(),
                   config->xon_threshold, config->xoff_threshold, unit));
        return BCM_E_PARAM;
    }
    
    if (config->drop_threshold != 0) {
        if (config->drop_threshold < config->xoff_threshold) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "ERROR: %s, drop_threshold 0x%x must be larger"
                                   "than xoff_threshold 0x%x, Unit(%d)\n"), FUNCTION_NAME(),
                       config->drop_threshold, config->xoff_threshold, unit));
            return BCM_E_PARAM;
        }
    }

    /* check threshold ranges */
    if (gport == BCM_GPORT_INVALID) {
	/* set up the per priority group thresholds */
	if (rev_id == BCM88230_B0_REV_ID) {
	    hysteresis = config->xoff_threshold - config->xon_threshold;
	    if (hysteresis == 16) {
		hysteresis = 0;
	    } else if (hysteresis == (16<<1)) {
		hysteresis = 1;
	    } else if (hysteresis == (16<<2)) {
		hysteresis = 2;
	    } else if (hysteresis == (16<<3)) {
		hysteresis = 3;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, For current chip revision, xoff_threshold(0x%x) - xon_threshold(0x%x) "
		                       "has to be 16, 32, 64 or 128, Unit(%d)\n"), FUNCTION_NAME(),
		           config->xoff_threshold, config->xon_threshold, unit));
		return BCM_E_PARAM;
	    }
	    
	    /* update thresholds */
	    switch (cosq) {
		case 0:
		    xoff_reg = QM_PFC_THRESHOLD_COS0r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS0r;
		    break;
		case 1:
		    xoff_reg = QM_PFC_THRESHOLD_COS1r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS1r;
		    break;
		case 2:
		    xoff_reg = QM_PFC_THRESHOLD_COS2r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS2r;
		    break;
		case 3:
		    xoff_reg = QM_PFC_THRESHOLD_COS3r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS3r;
		    break;
		case 4:
		    xoff_reg = QM_PFC_THRESHOLD_COS4r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS4r;
		    break;
		case 5:
		    xoff_reg = QM_PFC_THRESHOLD_COS5r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS5r;
		    break;
		case 6:
		    xoff_reg = QM_PFC_THRESHOLD_COS6r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS6r;
		    break;
		case 7:
		    xoff_reg = QM_PFC_THRESHOLD_COS7r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS7r;
		    break;

                /*
                 * COVERITY
                 *
                 * This default is unreachable. It is kept intentionally
                 * as a defensive measure for future development. 
                 */ 
                /* coverity[dead_error_begin : FALSE] */
		default:
		    return BCM_E_PARAM;
	    }

	    /* update xon/xoff config */
	    regaddr = soc_reg_addr(unit, xoff_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	    soc_reg_field_set(unit, xoff_reg, &regval, XOFF_THRESHOLDf, config->xoff_threshold);
	    soc_reg_field_set(unit, xoff_reg, &regval, HYSTERESISf, hysteresis);
	    BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));

	    /* update max config */
	    regaddr = soc_reg_addr(unit, max_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
            soc_reg_field_set(unit, max_reg, &regval, MAX_THRESHOLDf, config->drop_threshold);
	    BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));

	    /* update lossless */
	    BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG0r(unit, &regval));
	    lossless = soc_reg_field_get(unit, QM_PFC_CONFIG0r, regval, PFC_LOSSLESS_ENf);
	    if (config->lossless) {
		lossless |= (1<<cosq);
	    } else {
		lossless &= (~(1<<cosq));
	    }
	    soc_reg_field_set(unit, QM_PFC_CONFIG0r, &regval, PFC_LOSSLESS_ENf, lossless);
	    BCM_IF_ERROR_RETURN(WRITE_QM_PFC_CONFIG0r(unit, regval));

	    /* update lossless reserved config, lossless reserved is sum of all lossless
	     * xoff_threshold + 10 buffers for each lossless class
	     */
	    reserved_buffs = 0;
	    for (class = 0; class < 8; class++) {
		if ((lossless & (1<<class))==0) {
		    continue;
		}
		switch (class) {
		case 0:
		    xoff_reg = QM_PFC_THRESHOLD_COS0r;
		    break;
		case 1:
		    xoff_reg = QM_PFC_THRESHOLD_COS1r;
		    break;
		case 2:
		    xoff_reg = QM_PFC_THRESHOLD_COS2r;
		    break;
		case 3:
		    xoff_reg = QM_PFC_THRESHOLD_COS3r;
		    break;
		case 4:
		    xoff_reg = QM_PFC_THRESHOLD_COS4r;
		    break;
		case 5:
		    xoff_reg = QM_PFC_THRESHOLD_COS5r;
		    break;
		case 6:
		    xoff_reg = QM_PFC_THRESHOLD_COS6r;
		    break;
		case 7:
		    xoff_reg = QM_PFC_THRESHOLD_COS7r;
		    break;
		}

		regaddr = soc_reg_addr(unit, xoff_reg, REG_PORT_ANY, 0);
		BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
		num_buffs = soc_reg_field_get(unit, xoff_reg, regval, XOFF_THRESHOLDf);

		/* make sure it has xoff_thresholds for lossless class */
		reserved_buffs += num_buffs;

		/* reserve 10 more buffers for each class to account for delays */
		reserved_buffs += 10;
	    }

	    BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG1r(unit, &regval));
	    soc_reg_field_set(unit, QM_PFC_CONFIG1r, &regval, PFC_LOSSLESS_RESERVEDf, reserved_buffs);
	    BCM_IF_ERROR_RETURN(WRITE_QM_PFC_CONFIG1r(unit, regval));

	} else {
	    /* C0 and later revision */
	    /* update thresholds */
	    switch (cosq) {
		case 0:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_0r;
		    xoff_reg = QM_PFC_THRESHOLD_COS0r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS0r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_0r;
		    break;
		case 1:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_1r;
		    xoff_reg = QM_PFC_THRESHOLD_COS1r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS1r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_1r;
		    break;
		case 2:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_2r;
		    xoff_reg = QM_PFC_THRESHOLD_COS2r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS2r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_2r;
		    break;
		case 3:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_3r;
		    xoff_reg = QM_PFC_THRESHOLD_COS3r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS3r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_3r;
		    break;
		case 4:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_4r;
		    xoff_reg = QM_PFC_THRESHOLD_COS4r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS4r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_4r;
		    break;
		case 5:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_5r;
		    xoff_reg = QM_PFC_THRESHOLD_COS5r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS5r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_5r;
		    break;
		case 6:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_6r;
		    xoff_reg = QM_PFC_THRESHOLD_COS6r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS6r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_6r;
		    break;
		case 7:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_7r;
		    xoff_reg = QM_PFC_THRESHOLD_COS7r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS7r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_7r;
		    break;

                /*
                 * COVERITY
                 *
                 * This default is unreachable. It is kept intentionally
                 * as a defensive measure for future development. 
                 */ 
                /* coverity[dead_error_begin : FALSE] */
		default:
		    return BCM_E_PARAM;
	    }
	    
	    /* update xoff config */
	    regaddr = soc_reg_addr(unit, xoff_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	    soc_reg_field_set(unit, xoff_reg, &regval, XOFF_THRESHOLDf, config->xoff_threshold);
	    BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));

	    /* update xon config */
	    regaddr = soc_reg_addr(unit, xon_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	    soc_reg_field_set(unit, xon_reg, &regval, XON_THRESHOLDf, config->xon_threshold);
	    BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));

	    /* update max config */
	    regaddr = soc_reg_addr(unit, max_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
            soc_reg_field_set(unit, max_reg, &regval, MAX_THRESHOLDf, config->drop_threshold);
	    BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));

	    /* update reserved config */
	    regaddr = soc_reg_addr(unit, rsvd_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
            soc_reg_field_set(unit, rsvd_reg, &regval, RSVD_THRESHOLDf, config->reserved_buffers);
	    BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));	    

	    /* update total reserved config */
	    reserved_buffs = 0;
	    for (class = 0; class < 8; class++) {
		switch (class) {
		    case 0:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_0r;
			break;
		    case 1:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_1r;
			break;
		    case 2:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_2r;
			break;
		    case 3:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_3r;
			break;
		    case 4:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_4r;
			break;
		    case 5:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_5r;
			break;
		    case 6:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_6r;
			break;
		    case 7:
			rsvd_reg = QM_PFC_RSVD_THRESHOLD_7r;
			break;		    
		}

		/* make sure it has xoff_thresholds for lossless class */
		regaddr = soc_reg_addr(unit, rsvd_reg, REG_PORT_ANY, 0);
		BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
		reserved_buffs += soc_reg_field_get(unit, rsvd_reg, regval, RSVD_THRESHOLDf);
	    }

	    /* update lossless, we probablly don't need it for C0?? */
	    BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG0r(unit, &regval));
	    lossless = soc_reg_field_get(unit, QM_PFC_CONFIG0r, regval, PFC_LOSSLESS_ENf);
	    if (config->lossless) {
		lossless |= (1<<cosq);
	    } else {
		lossless &= (~(1<<cosq));
	    }
	    soc_reg_field_set(unit, QM_PFC_CONFIG0r, &regval, PFC_LOSSLESS_ENf, lossless);
	    BCM_IF_ERROR_RETURN(WRITE_QM_PFC_CONFIG0r(unit, regval));
	}	
    } else if ((BCM_GPORT_IS_MODPORT(gport)) ||
	       (BCM_GPORT_IS_EGRESS_MODPORT(gport)) ||
	       (BCM_GPORT_IS_CHILD(gport)) ||
	       (BCM_GPORT_IS_EGRESS_CHILD(gport))) {
	/* set up the (source port, priority group) thresholds */

	if (!soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    /* (source port, priority) PFC not supported in B0 */
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, (Source port, PG) PFC not supported on Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_UNAVAIL;
	}
	
	/* all thresholds are same for all source ports, so look into PG only */
	switch (cosq) {
	    case 0:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_0r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_0r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_0r;
		break;
	    case 1:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_1r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_1r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_1r;
		break;
	    case 2:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_2r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_2r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_2r;
		break;
	    case 3:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_3r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_3r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_3r;
		break;
	    case 4:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_4r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_4r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_4r;
		break;
	    case 5:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_5r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_5r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_5r;
		break;
	    case 6:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_6r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_6r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_6r;
		break;
	    case 7:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_7r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_7r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_7r;
		break;

            /*
             * COVERITY
             *
             * This default is unreachable. It is kept intentionally
             * as a defensive measure for future development. 
             */ 
            /* coverity[dead_error_begin : FALSE] */
	    default:
		return BCM_E_PARAM;
	}

	/* update xoff config */
	regaddr = soc_reg_addr(unit, sp_xoff_reg, REG_PORT_ANY, 0);
	BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	soc_reg_field_set(unit, sp_xoff_reg, &regval, XOFF_THRESHOLDf, config->xoff_threshold);
	BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));
	
	/* update xon config */
	regaddr = soc_reg_addr(unit, sp_xon_reg, REG_PORT_ANY, 0);
	BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	soc_reg_field_set(unit, sp_xon_reg, &regval, XON_THRESHOLDf, config->xon_threshold);
	BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));
	
	/* update max config */
	regaddr = soc_reg_addr(unit, sp_max_reg, REG_PORT_ANY, 0);
	BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	soc_reg_field_set(unit, sp_max_reg, &regval, MAX_THRESHOLDf, config->drop_threshold);
	BCM_IF_ERROR_RETURN(soc_reg32_write(unit, regaddr, regval));	

	/* user configing per SP threshold, init the (source port, priority group mode) */
	BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG0r(unit, &regval));
	soc_reg_field_set(unit, QM_PFC_CONFIG0r, &regval, SP_PG_ENABLEf, 1);
	BCM_IF_ERROR_RETURN(WRITE_QM_PFC_CONFIG0r(unit, regval));
    }

    return rv;
}

int bcm_sirius_cosq_pfc_config_get(int unit, 
				bcm_gport_t gport, 
				bcm_cos_queue_t cosq, 
				uint32 flags, 
				bcm_cosq_pfc_config_t *config)
{
    int rv = BCM_E_NONE;
    uint16    dev_id;
    uint8     rev_id;
    uint32    regval = 0, regaddr;
    int       hysteresis;
    uint32    lossless;
    soc_reg_t xon_reg, xoff_reg, max_reg, rsvd_reg, sp_xon_reg, sp_xoff_reg, sp_max_reg;

    soc_cm_get_id(unit, &dev_id, &rev_id);

    if (!soc_feature(unit, soc_feature_priority_flow_control)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, priority flow control not supported on Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_UNAVAIL;
    }

    if (config == NULL) {
	return BCM_E_PARAM;
    }

    if (gport == BCM_GPORT_INVALID) {
	/* get the per priority group thresholds */
	if (rev_id == BCM88230_B0_REV_ID) {
	    BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG1r(unit, &regval));
	    config->reserved_buffers = soc_reg_field_get(unit, QM_PFC_CONFIG1r, regval, PFC_LOSSLESS_RESERVEDf);

	    /* update thresholds */
	    switch (cosq) {
		case 0:
		    xoff_reg = QM_PFC_THRESHOLD_COS0r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS0r;
		    break;
		case 1:
		    xoff_reg = QM_PFC_THRESHOLD_COS1r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS1r;
		    break;
		case 2:
		    xoff_reg = QM_PFC_THRESHOLD_COS2r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS2r;
		    break;
		case 3:
		    xoff_reg = QM_PFC_THRESHOLD_COS3r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS3r;
		    break;
		case 4:
		    xoff_reg = QM_PFC_THRESHOLD_COS4r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS4r;
		    break;
		case 5:
		    xoff_reg = QM_PFC_THRESHOLD_COS5r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS5r;
		    break;
		case 6:
		    xoff_reg = QM_PFC_THRESHOLD_COS6r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS6r;
		    break;
		case 7:
		    xoff_reg = QM_PFC_THRESHOLD_COS7r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS7r;
		    break;
		default:
		    return BCM_E_PARAM;
	    }

	    /* get xon/xoff config */
	    regaddr = soc_reg_addr(unit, xoff_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	    config->xoff_threshold = soc_reg_field_get(unit, xoff_reg, regval, XOFF_THRESHOLDf);
	    hysteresis = soc_reg_field_get(unit, xoff_reg, regval, HYSTERESISf);
	    config->xon_threshold = config->xoff_threshold - (16 * (1<<hysteresis));

	    /* get max config */
	    regaddr = soc_reg_addr(unit, max_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
            config->drop_threshold = soc_reg_field_get(unit, max_reg, regval, MAX_THRESHOLDf);

	} else {
	    /* C0 and later revision */
	    /* update thresholds */
	    switch (cosq) {
		case 0:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_0r;
		    xoff_reg = QM_PFC_THRESHOLD_COS0r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS0r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_0r;
		    break;
		case 1:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_1r;
		    xoff_reg = QM_PFC_THRESHOLD_COS1r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS1r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_1r;
		    break;
		case 2:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_2r;
		    xoff_reg = QM_PFC_THRESHOLD_COS2r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS2r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_2r;
		    break;
		case 3:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_3r;
		    xoff_reg = QM_PFC_THRESHOLD_COS3r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS3r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_3r;
		    break;
		case 4:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_4r;
		    xoff_reg = QM_PFC_THRESHOLD_COS4r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS4r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_4r;
		    break;
		case 5:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_5r;
		    xoff_reg = QM_PFC_THRESHOLD_COS5r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS5r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_5r;
		    break;
		case 6:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_6r;
		    xoff_reg = QM_PFC_THRESHOLD_COS6r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS6r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_6r;
		    break;
		case 7:
		    xon_reg = QM_PFC_PG_BUF_XON_THRESHOLD_7r;
		    xoff_reg = QM_PFC_THRESHOLD_COS7r;
		    max_reg = QM_PFC_MAX_THRESHOLD_COS7r;
		    rsvd_reg = QM_PFC_RSVD_THRESHOLD_7r;
		    break;
		default:
		    return BCM_E_PARAM;
	    }
	    
	    /* get xoff config */
	    regaddr = soc_reg_addr(unit, xoff_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	    config->xoff_threshold = soc_reg_field_get(unit, xoff_reg, regval, XOFF_THRESHOLDf);

	    /* get xon config */
	    regaddr = soc_reg_addr(unit, xon_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	    config->xon_threshold = soc_reg_field_get(unit, xon_reg, regval, XON_THRESHOLDf);

	    /* get max config */
	    regaddr = soc_reg_addr(unit, max_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
            config->drop_threshold = soc_reg_field_get(unit, max_reg, regval, MAX_THRESHOLDf);

	    /* get reserved config */
	    regaddr = soc_reg_addr(unit, rsvd_reg, REG_PORT_ANY, 0);
	    BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
            config->drop_threshold = soc_reg_field_get(unit, rsvd_reg, regval, RSVD_THRESHOLDf);
	}

	/* update lossless */
	BCM_IF_ERROR_RETURN(READ_QM_PFC_CONFIG0r(unit, &regval));
	lossless = soc_reg_field_get(unit, QM_PFC_CONFIG0r, regval, PFC_LOSSLESS_ENf);
	if (lossless & (1<<cosq)) {
	    config->lossless = TRUE;
	} else {
	    config->lossless = FALSE;
	}
    } else if ((BCM_GPORT_IS_MODPORT(gport)) ||
	       (BCM_GPORT_IS_EGRESS_MODPORT(gport)) ||
	       (BCM_GPORT_IS_CHILD(gport)) ||
	       (BCM_GPORT_IS_EGRESS_CHILD(gport))) {
	/* get the (source port, priority group) thresholds */

	if (!soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    /* (source port, priority) PFC not supported in B0 */
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, (Source port, PG) PFC not supported on Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return BCM_E_UNAVAIL;
	}
	
	switch (cosq) {
	    case 0:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_0r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_0r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_0r;
		break;
	    case 1:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_1r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_1r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_1r;
		break;
	    case 2:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_2r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_2r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_2r;
		break;
	    case 3:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_3r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_3r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_3r;
		break;
	    case 4:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_4r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_4r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_4r;
		break;
	    case 5:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_5r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_5r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_5r;
		break;
	    case 6:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_6r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_6r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_6r;
		break;
	    case 7:
		sp_xon_reg = QM_PFC_SP_PG_LINE_XON_THRESHOLD_7r;
		sp_xoff_reg = QM_PFC_SP_PG_LINE_XOFF_THRESHOLD_7r;
		sp_max_reg = QM_PFC_SP_PG_LINE_MAX_THRESHOLD_7r;
		break;
		default:
		    return BCM_E_PARAM;
	}

	/* update xoff config */
	regaddr = soc_reg_addr(unit, sp_xoff_reg, REG_PORT_ANY, 0);
	BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	config->xoff_threshold = soc_reg_field_get(unit, sp_xoff_reg, regval, XOFF_THRESHOLDf);
	
	/* update xon config */
	regaddr = soc_reg_addr(unit, sp_xon_reg, REG_PORT_ANY, 0);
	BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	config->xon_threshold = soc_reg_field_get(unit, sp_xon_reg, regval, XON_THRESHOLDf);
	
	/* update max config */
	regaddr = soc_reg_addr(unit, sp_max_reg, REG_PORT_ANY, 0);
	BCM_IF_ERROR_RETURN(soc_reg32_read(unit, regaddr, &regval));
	config->drop_threshold = soc_reg_field_get(unit, sp_max_reg, regval, MAX_THRESHOLDf);
    }

    return rv;
}

int
bcm_sirius_cosq_port_congestion_set(int unit, bcm_gport_t congestion_port,
				    bcm_gport_t gport, uint32 flags,
				    bcm_port_congestion_config_t config, int channel_id)
{
    int rv = BCM_E_UNAVAIL;
    int port, module;
    int max_ports, base_index, index_offset, fc_state_index, i;
    int channel_base, channel_offset;
    int *fifo_map = NULL, nbr_fifos = 0;
    int subport, port_offset, egroup_num, num_fifos;

    /* NOTE: This function is used for HCFC RX only */

    

    if (BCM_GPORT_IS_MODPORT(congestion_port)) {
	port = BCM_GPORT_MODPORT_PORT_GET(congestion_port);
	if (!IS_HG_PORT(unit, port)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, gport 0x%X not a higig port, unit %d\n"),
	               FUNCTION_NAME(), congestion_port, unit));
	    return(BCM_E_PARAM);
	} else {
	    port = port - SOC_PORT_MIN(unit, hg);
	}
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(congestion_port)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(congestion_port);
	if (!IS_HG_PORT(unit, port)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, gport 0x%X not a higig port, unit %d\n"),
	               FUNCTION_NAME(), congestion_port, unit));
	    return(BCM_E_PARAM);
	} else {
	    port = port - SOC_PORT_MIN(unit, hg);
	}
    } else if (BCM_GPORT_IS_CONGESTION(congestion_port)) {
	port = BCM_GPORT_CONGESTION_GET(congestion_port);
    } else {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unsupported type for gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(), congestion_port, unit));
	return(BCM_E_PARAM);
    }
    
    if (!(BCM_GPORT_IS_CHILD(gport) ||
	  BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	  BCM_GPORT_IS_EGRESS_GROUP(gport))) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unsupported type for gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(), gport, unit));
	return(BCM_E_PARAM);
    }

    channel_base = BCM_PORT_CHANNEL_ID_PORT_BASE_GET(channel_id);
    channel_offset = BCM_PORT_CHANNEL_ID_PORT_CH_GET(channel_id);
    if (channel_base >SIRIUS_HCFC_MAX_CHANNEL_BASE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, channel base of channel id 0x%X is out of range [0-0x1F], unit %d\n"),
	           FUNCTION_NAME(), channel_id, unit));
	return (BCM_E_PARAM);
    }
    
    if (config.port_bits == 0) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, invalid %d bits per port config, unit %d\n"),
	           FUNCTION_NAME(), config.port_bits, unit));
	return (BCM_E_PARAM);
    }

    if ((channel_offset + config.port_bits - 1) > SIRIUS_HCFC_MAX_CHANNEL_OFFSET) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, channel offset of channel id 0x%X is out of range [0-95] with %d"
	                       " bits per port, unit %d\n"), FUNCTION_NAME(), channel_id, config.port_bits, unit));
	return (BCM_E_PARAM);
    }
    
    /* HCFC message is fixed size, so always allocate same amount of flow control
     * state entry for each uniq channel base.
     */
    module = SIRIUS_HCFC_FC_BASE_INDEX_SET(port, 1, channel_base);
    rv = bcm_sirius_cosq_module_congestion_allocate_get(unit, module, &max_ports, &base_index);
    if (rv != BCM_E_NONE) {
	if (flags & BCM_PORT_CONGESTION_MAPPING_CLEAR) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, no resoure found for specified channel base 0x%x\n"),
	               FUNCTION_NAME(), channel_base));	    
	    return BCM_E_PARAM;
	} else {
	    /* need to allocate for the new channel base, allocate fixed number of entries */
	    if (config.port_bits == 8) {
		max_ports = (SIRIUS_HCFC_MAX_CHANNEL_OFFSET+1)/(config.port_bits/2);
	    } else {
		max_ports = (SIRIUS_HCFC_MAX_CHANNEL_OFFSET+1)/config.port_bits;
	    }
	    rv = bcm_sirius_cosq_module_congestion_allocate(unit, module, max_ports, &base_index);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, allocating congestion resources, unit: %d, module: 0x%x\n"),
		           FUNCTION_NAME(), unit, module));
		return(rv);
	    }
	    
	    rv = soc_sirius_es_fc_base_table_set(unit, module, max_ports, base_index, TRUE);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, configuring congestion resources, unit: %d, module: 0x%x rc: 0x%x\n"),
		           FUNCTION_NAME(), unit, module, rv));
		return(rv);
	    }

	    /* duplicate entry to ignore the MSG_TYPE */
	    module = SIRIUS_HCFC_FC_BASE_INDEX_SET(port, 0x2, channel_base);
	    rv = soc_sirius_es_fc_base_table_set(unit, module, max_ports, base_index, TRUE);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, configuring congestion resources, unit: %d, module: 0x%x rc: 0x%x\n"),
		           FUNCTION_NAME(), unit, module, rv));
		return(rv);
	    }
	}
    } else {
	if (flags & BCM_PORT_CONGESTION_MAPPING_CLEAR) {
	    if (channel_offset == 0) {
		/* use CLEAR flag and channel offset 0 to free all resources */
		rv = bcm_sirius_cosq_module_congestion_deallocate(unit, module, 1);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free congestion resources, unit: %d, module: 0x%x rc: 0x%x\n"),
		               FUNCTION_NAME(), unit, module, rv));		    
		    return rv;
		}

		/* free hardware resource */
		rv = soc_sirius_es_fc_base_table_set(unit, module, 0, 0, FALSE);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free congestion resources, unit: %d, module: 0x%x\n"),
		               FUNCTION_NAME(), unit, module));
		    return(rv);
		}

		/* duplicate entry to ignore the MSG_TYPE */
		module = SIRIUS_HCFC_FC_BASE_INDEX_SET(port, 0x2, channel_base);
		rv = soc_sirius_es_fc_base_table_set(unit, module, 0, 0, FALSE);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free congestion resources, unit: %d, module: 0x%x\n"),
		               FUNCTION_NAME(), unit, module));
		    return(rv);
		}
	    }
	} else {
	    /* uses existing entries for the channel base */
	}
    }
    
    /* get the fifos corresponding to the specified fabric gport */    
    subport = -1;
    rv = bcm_sbx_cosq_egress_group_info_get(unit, gport, &subport, &egroup_num, &num_fifos);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unexpected fabric port: 0x%x\n"),
	           FUNCTION_NAME(), gport));
	return rv;
    }

    if (subport != -1) {
	rv = bcm_sbx_port_get_port_portoffset(unit, subport, &port, &port_offset);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, err: 0x%x\n"),
	               FUNCTION_NAME(), rv));
	    return rv;
	}
	if (IS_CPU_PORT(unit, port)) {
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "fabric port: 0x%x corresponds to CPU port\n"),
	                 gport));
	    /* do nothing for CPU port */
	    return BCM_E_NONE;
	}
    }

    /* allocate temporary memory */
    fifo_map = sal_alloc((sizeof(int) * 132), "tmp fifo map");

    if (fifo_map == NULL) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, Temp memory allocation failed, unit: %d\n"),
	             FUNCTION_NAME(), unit));
	return BCM_E_MEMORY;
    }

    /* determine FIFOs corresponding to Fabric ports */
    rv = bcm_sirius_cosq_fifo_set_get(unit, gport, fifo_map, &nbr_fifos, 132);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, retreiving fifos for fabric port, unit: %d, fabric gport: 0x%x\n"),
	           FUNCTION_NAME(), unit, gport));
	if (fifo_map != NULL) {
	  sal_free(fifo_map);
	}
	return rv;
    }
    
    LOG_VERBOSE(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "fabric port: 0x%x FifoMap: %d, nbrFifos: %d\n"),
                 gport, (*fifo_map), nbr_fifos));

    /* configure flow control map table */
    if (flags & BCM_PORT_CONGESTION_MAPPING_CLEAR) {
	/* point to invalid fc state entry */
	fc_state_index = 0xFF;
    } else {
	index_offset = channel_offset/config.port_bits;
	if (config.port_bits == 8) {
	    index_offset = index_offset * 2;
	}
	fc_state_index = base_index + index_offset;
    }

    for (i = 0; i < nbr_fifos; i++) {
	if ((config.port_bits == 8) && (nbr_fifos >= 1) && (i==1)) {
	    /* if port_bits is 8 bits per port, and 8 fifos per port, each port occupy 
	     * 2 flow_control_state_entry. If more than 8 fifos, first 4 fifos point to
	     * 1st entry, all other fifos point to 2nd entry.
	     */
	    fc_state_index++;
	}

	rv = soc_sirius_es_fc_map_table_set(unit,
					    (*(fifo_map + i)) / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE,
					    (*(fifo_map + i)), fc_state_index);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, configuring fc map table, unit: %d, fifo Base: 0x%x Index: 0x%x\n"),
	               FUNCTION_NAME(), unit, (*(fifo_map + i)), fc_state_index));
	    if (fifo_map != NULL) {
	      sal_free(fifo_map);
	    }
	    return rv;
	}
    }

    if (fifo_map != NULL) {
        sal_free(fifo_map);
    }
    return rv;
}

int
bcm_sirius_cosq_port_congestion_get(int unit, bcm_gport_t congestion_port,
				    bcm_gport_t gport, uint32 flags, int *channel_id)
{
    int rv = BCM_E_UNAVAIL;
    int port, module;
    int max_ports, base_index, fc_state_index;
    int channel_base, channel_offset;
    int *fifo_map = NULL, nbr_fifos = 0;
    int is_enable;

    /* NOTE: This function is used for HCFC RX only */
    if (BCM_GPORT_IS_MODPORT(congestion_port)) {
	port = BCM_GPORT_MODPORT_PORT_GET(congestion_port);
	if (!IS_HG_PORT(unit, port)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, gport 0x%X not a higig port, unit %d\n"),
	               FUNCTION_NAME(), congestion_port, unit));
	    return(BCM_E_PARAM);
	} else {
	    port = port - SOC_PORT_MIN(unit, hg);
	}
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(congestion_port)) {
	port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(congestion_port);
	if (!IS_HG_PORT(unit, port)) {
	    LOG_ERROR(BSL_LS_BCM_COSQ,
	              (BSL_META_U(unit,
	                          "ERROR: %s, gport 0x%X not a higig port, unit %d\n"),
	               FUNCTION_NAME(), congestion_port, unit));
	    return(BCM_E_PARAM);
	} else {
	    port = port - SOC_PORT_MIN(unit, hg);
	}
    } else if (BCM_GPORT_IS_CONGESTION(congestion_port)) {
	port = BCM_GPORT_CONGESTION_GET(congestion_port);
    } else {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unsupported type for gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(), congestion_port, unit));
	return(BCM_E_PARAM);
    }
    
    if (!(BCM_GPORT_IS_CHILD(gport) ||
	  BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	  BCM_GPORT_IS_EGRESS_GROUP(gport))) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, unsupported type for gport 0x%X, unit %d\n"),
	           FUNCTION_NAME(), gport, unit));
	return(BCM_E_PARAM);
    }

    /* allocate temporary memory */
    fifo_map = sal_alloc((sizeof(int) * 132), "tmp fifo map");

    if (fifo_map == NULL) {
	LOG_VERBOSE(BSL_LS_BCM_COSQ,
	            (BSL_META_U(unit,
	                        "%s, Temp memory allocation failed, unit: %d\n"),
	             FUNCTION_NAME(), unit));
	return BCM_E_MEMORY;
    }
    
    /* determine FIFOs corresponding to Fabric ports */
    rv = bcm_sirius_cosq_fifo_set_get(unit, gport, fifo_map, &nbr_fifos, 132);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, retreiving fifos for fabric port, unit: %d, fabric gport: 0x%x\n"),
	           FUNCTION_NAME(), unit, gport));
	if (fifo_map != NULL) {
	    sal_free(fifo_map);
	}
	return rv;
    }

    /* get flow control map table */
    rv = soc_sirius_es_fc_map_table_get(unit, (*fifo_map) / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE,
					(*fifo_map), &fc_state_index);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, failed to get fc map table, unit: %d, fifo Base: 0x%x\n"),
	           FUNCTION_NAME(), unit, (*fifo_map)));
	if (fifo_map != NULL) {
	    sal_free(fifo_map);
	}
	return rv;
    }

    if ((fc_state_index >= SIRIUS_ES_FC_STATE_TABLE_SIZE) ||
	((fc_state[unit]+fc_state_index)->in_use != TRUE)) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fabric port 0x%x not mapped to valid channel_base/offset unit: %d\n"),
	           FUNCTION_NAME(), gport, unit));
	if (fifo_map != NULL) {
	    sal_free(fifo_map);
	}
	return BCM_E_PARAM;	
    }

    module = (fc_state[unit]+fc_state_index)->module;
    channel_base = SIRIUS_HCFC_FC_CHANNEL_BASE_GET(module);
    
    rv = soc_sirius_es_fc_base_table_get(unit, module, &max_ports, &base_index, &is_enable);
    if (rv != SOC_E_NONE) {
	if (fifo_map != NULL) {
	    sal_free(fifo_map);
	}
	return rv;
    }

    if (is_enable == FALSE) {
	LOG_ERROR(BSL_LS_BCM_COSQ,
	          (BSL_META_U(unit,
	                      "ERROR: %s, fabric port 0x%x not mapped to valid channel_base/offset in HCFC message unit: %d\n"),
	           FUNCTION_NAME(), gport, unit));
	if (fifo_map != NULL) {
	    sal_free(fifo_map);
	}
	return BCM_E_PARAM;
    }

    channel_offset = (fc_state_index - base_index) * ((SIRIUS_HCFC_MAX_CHANNEL_OFFSET+1)/max_ports);

    BCM_PORT_CHANNEL_ID_SET(*channel_id, channel_base, channel_offset);

    if (fifo_map != NULL) {
        sal_free(fifo_map);
    }
    rv = BCM_E_NONE;
    return rv;
}

int
bcm_sirius_cosq_set_template_pfc(int unit, int32 template, int pfc_en, int pfc_pg)
{
    int rv = BCM_E_NONE;
    q_min_buffs_entry_t qMinBuffs;

    if (soc_feature(unit, soc_feature_priority_flow_control)) {
	SOC_IF_ERROR_RETURN(READ_Q_MIN_BUFFSm(unit, MEM_BLOCK_ANY, template, &qMinBuffs));
	soc_mem_field32_set(unit, Q_MIN_BUFFSm, &qMinBuffs, PFC_ENf, (pfc_en?1:0));
	if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    if (pfc_pg < 0) {
		pfc_pg = 0;
	    } else if (pfc_pg > 7) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "ERROR: %s, priority group %d out of range [0-7] supported by unit: %d\n"),
		           FUNCTION_NAME(), pfc_pg, unit));
		return BCM_E_PARAM;
	    }

	    soc_mem_field32_set(unit, Q_MIN_BUFFSm, &qMinBuffs, PFC_PGf, pfc_pg);
	}
	SOC_IF_ERROR_RETURN(WRITE_Q_MIN_BUFFSm(unit, MEM_BLOCK_ANY, template, &qMinBuffs));
    }

    return rv;
}

int
bcm_sirius_cosq_get_template_pfc(int unit, int32 template, int *pfc_en, int *pfc_pg)
{
    int rv = BCM_E_NONE;
    q_min_buffs_entry_t qMinBuffs;

    if (soc_feature(unit, soc_feature_priority_flow_control)) {
	SOC_IF_ERROR_RETURN(READ_Q_MIN_BUFFSm(unit, MEM_BLOCK_ANY, template, &qMinBuffs));
	if (pfc_en != NULL) {
	    *pfc_en = (soc_mem_field32_get(unit, Q_MIN_BUFFSm, &qMinBuffs, PFC_ENf)?TRUE:FALSE);
	}
	if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    if (pfc_pg != NULL) {
		*pfc_pg = soc_mem_field32_get(unit, Q_MIN_BUFFSm, &qMinBuffs, PFC_PGf);
	    }
	}
    } else {
	*pfc_en = FALSE;
	*pfc_pg = 0;
    }

    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
static int
bcm_sirius_wb_cosq_queue_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int queue;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;    
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    uint16 tmp_uint16 = 0;
    uint32 tmp_uint32 = 0;
    int32  tmp_int32 = 0;
    uint32 queueMapBlock, base, size, qme_index, mcgroup, index;
    queue_map_entry_t qme;
    bcm_sbx_mgid_list_t *pRoot = NULL;
    bcm_sbx_mgid_list_t *pNew = NULL;
    dq_p_t pList;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     * typedef struct bcm_sbx_cosq_queue_params_ingress_s {
     *     uint8 state;                                    (1 bytes)
     *     int32 bw_group;                                 (0-16K compressed to 2 bytes)
     *     int16 sysport;                                  (0-4K compressed to 2 bytes)
     *     int16 default_sysport;                          (0-4K compressed to 2 bytes)
     *     int32 ingress.bw_mode;                          (0-BCM_COSQ_MAX, compressed to 1 byte)
     *     int32 ingress.bw_value.guarantee_kbps;          (4 bytes)
     *     int32 ingress.bw_value.sp_priority;             (0-255 compressed 1 byte)
     *     int32 ingress.bw_value.wfq_weight;              (0-0x3F compressed 1 byte)
     *     int32 ingress.given_weight;                     (NOT USED for sirius)
     *     int32 ingress.min_physical_queue_depth_bytes;   (4 bytes)
     *     int32 ingress.max_physical_queue_depth_bytes;   (4 bytes)
     *     int32 ingress.anemic_watermark_select;          (from hardware table)
     *     int32 ingress.ingress_spi4;                     (NOT USED for sirius)
     *     int32 ingress.ingress_spi4_port;                (NOT USED for sirius)
     *     int32 ingress.qla_demand_mask;                  (NOT USED for sirius)
     *     int32 ingress.queue_header_adjust_sign;         (NOT USED for sirius)
     *     int32 ingress.queue_header_adjust;              (NOT USED for sirius)
     *     int32 ingress.anemic_age_threshold_key;         (from hardware table)
     *     int32 ingress.shape_limit_kbps;                 (0-100M, compressed to 31bits)
     *     int32 ingress.enable_shaping;                   (1 bit, combined with shape_limit_kbps to 4 bytes)
     *     int32 ingress.shape_burst_kbps;                 (4 bytes)
     *     int32 ingress.local;                            (NOT USED for sirius)
     *     int32 ingress.hold_pri_num_timeslots;           (from hardware table)
     *     int32 ingress.template;                         (0-0x1FF, compressed to 10 bits for future expansion)
     *     int32 ingress.gain;                             (0-0xF, compressed to 6 bits for future expansion)
     *     int32 ingress.enabled;                          (1 bit, from HW)
     *     int8  enq_stat_in_use;                          (1 byte)
     *     int8  deq_stat_in_use;                          (1 byte)
     *     int8  attached_fifo;                            (1 byte)
     *     bcm_sbx_mgid_list_t *mgid_list;                 (linklist, from HW)
     * }bcm_sbx_cosq_queue_params_ingress_t;
     */
    for (queue = 0; queue < SOC_SBX_CFG(unit)->num_queues; queue++) {
	p_qstate = &(SOC_SBX_STATE(unit)->queue_state[queue]);
	switch (operation) {
	    case _WB_OP_SIZE:		/* calculate structures size requirement for worst case */
	    case _WB_OP_DECOMPRESS:	/* restore strucures from scache */
		__WB_DECOMPRESS_SCALAR(uint8, p_qstate->state);
		if (!SOC_WARM_BOOT(unit) || (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_AVAILABLE)) {
		    __WB_DECOMPRESS_SCALAR(int16, p_qstate->bw_group);
		    __WB_DECOMPRESS_SCALAR(int16, p_qstate->sysport);
		    __WB_DECOMPRESS_SCALAR(int16, p_qstate->default_sysport);
		    __WB_DECOMPRESS_SCALAR(int8,  p_qstate->ingress.bw_mode);
		    __WB_DECOMPRESS_SCALAR(int32, p_qstate->ingress.bw_value.guarantee_kbps);
		    __WB_DECOMPRESS_SCALAR(int8,  p_qstate->ingress.bw_value.sp_priority);
		    __WB_DECOMPRESS_SCALAR(int8,  p_qstate->ingress.bw_value.wfq_weight);
		    __WB_DECOMPRESS_SCALAR(int32, p_qstate->ingress.min_physical_queue_depth_bytes);
		    __WB_DECOMPRESS_SCALAR(int32, p_qstate->ingress.max_physical_queue_depth_bytes);

		    __WB_DECOMPRESS_SCALAR(uint32,tmp_uint32);
                    if (SOC_WARM_BOOT(unit)) {
                        if (tmp_uint32 & 0x1) {
                            p_qstate->ingress.enable_shaping = TRUE;
                            p_qstate->ingress.shape_limit_kbps = (tmp_uint32 & 0x7FFFFFFF) >> 1;
                        } else {
                            p_qstate->ingress.enable_shaping = FALSE;
                            p_qstate->ingress.shape_limit_kbps = (tmp_uint32 & 0x7FFFFFFF) >> 1;
                        }
                    }

		    __WB_DECOMPRESS_SCALAR(int32, p_qstate->ingress.shape_burst_kbps);

		    __WB_DECOMPRESS_SCALAR(uint16,tmp_uint16);
                    if (SOC_WARM_BOOT(unit)) {
                        p_qstate->ingress.template = (tmp_uint16 & 0x3FF);
                        p_qstate->ingress.gain = (tmp_uint16 >> 10) & 0x3F;
                    }

		    __WB_DECOMPRESS_SCALAR(int8,  p_qstate->enq_stat_in_use);
		    __WB_DECOMPRESS_SCALAR(int8,  p_qstate->deq_stat_in_use);
		    __WB_DECOMPRESS_SCALAR(int8,  p_qstate->attached_fifo);		

                    if (SOC_WARM_BOOT(unit)) {
                        /* recover stuff from HW
                         *     int32 ingress.anemic_watermark_select;          (from hardware table)
                         *     int32 ingress.anemic_age_threshold_key;         (from hardware table)
                         *     int32 ingress.hold_pri_num_timeslots;           (from hardware table)
                         *     int32 ingress.enabled;                          (1 bit, from HW)
                         */
                        SOC_IF_ERROR_RETURN(soc_sirius_queue_min_util_get(unit, queue,
                                                                          &(p_qstate->ingress.anemic_watermark_select)));
                        SOC_IF_ERROR_RETURN(soc_sirius_queue_max_age_get(unit, queue,
                                                                         &(p_qstate->ingress.anemic_age_threshold_key)));
                        SOC_IF_ERROR_RETURN(soc_sirius_qs_queue_parameter_get(unit, queue, &tmp_int32,
                                                                              &(p_qstate->ingress.hold_pri_num_timeslots)));
                        
                        SOC_IF_ERROR_RETURN(soc_sirius_qm_queue_buffs_profile_entry_get(unit, queue, &tmp_uint32,
                                                                                        (uint32 *)&(p_qstate->ingress.enabled)));
                    }
                }
		break;
	    case _WB_OP_COMPRESS:
		/* save structures to scache */
                __WB_COMPRESS_SCALAR(uint8, p_qstate->state);
		if (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_AVAILABLE) {
		    __WB_COMPRESS_SCALAR(int16, p_qstate->bw_group);
		    __WB_COMPRESS_SCALAR(int16, p_qstate->sysport);
		    __WB_COMPRESS_SCALAR(int16, p_qstate->default_sysport);
		    __WB_COMPRESS_SCALAR(int8,  p_qstate->ingress.bw_mode);
		    __WB_COMPRESS_SCALAR(int32, p_qstate->ingress.bw_value.guarantee_kbps);
		    __WB_COMPRESS_SCALAR(int8,  p_qstate->ingress.bw_value.sp_priority);
		    __WB_COMPRESS_SCALAR(int8,  p_qstate->ingress.bw_value.wfq_weight);
		    __WB_COMPRESS_SCALAR(int32, p_qstate->ingress.min_physical_queue_depth_bytes);
		    __WB_COMPRESS_SCALAR(int32, p_qstate->ingress.max_physical_queue_depth_bytes);

		    tmp_uint32 = (p_qstate->ingress.enable_shaping == TRUE)?1:0;
		    tmp_uint32 |= ((p_qstate->ingress.shape_limit_kbps & 0x7FFFFFFF) << 1);
		    __WB_COMPRESS_SCALAR(uint32,tmp_uint32);
		    __WB_COMPRESS_SCALAR(int32, p_qstate->ingress.shape_burst_kbps);

		    tmp_uint16 = (p_qstate->ingress.template & 0x3FF);
		    tmp_uint16 |= ((p_qstate->ingress.gain & 0x3F)<<10);
		    __WB_COMPRESS_SCALAR(uint16,tmp_uint16);

		    __WB_COMPRESS_SCALAR(int8,  p_qstate->enq_stat_in_use);
		    __WB_COMPRESS_SCALAR(int8,  p_qstate->deq_stat_in_use);
		    __WB_COMPRESS_SCALAR(int8,  p_qstate->attached_fifo);
		}
		break;
	    case _WB_OP_DUMP:
		/* dump the state for debug purpose */
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s dump queue %d on unit %d\n"),
		             FUNCTION_NAME(), queue, unit));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "state 0x%x, bw_group 0x%x sysport 0x%x default_sysport 0x%x\n"),
		             p_qstate->state, p_qstate->bw_group, p_qstate->sysport, p_qstate->default_sysport));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "bw_mode 0x%x bw_value 0x%x, 0x%x 0x%x\n"),
		             p_qstate->ingress.bw_mode, p_qstate->ingress.bw_value.guarantee_kbps,
		             p_qstate->ingress.bw_value.sp_priority,
		             p_qstate->ingress.bw_value.wfq_weight));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "given_weight 0x%x, min_physical_queue_depth_bytes 0x%x max_physical_queue_depth_bytes 0x%x\n"),
		             p_qstate->ingress.given_weight, p_qstate->ingress.min_physical_queue_depth_bytes,
		             p_qstate->ingress.max_physical_queue_depth_bytes));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "anemic_watermark_select 0x%x, anemic_age_threshold_key 0x%x shape_limit_kbps 0x%x\n"),
		             p_qstate->ingress.anemic_watermark_select, p_qstate->ingress.anemic_age_threshold_key,
		             p_qstate->ingress.shape_limit_kbps));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "enable_shaping 0x%x, shape_burst_kbps 0x%x hold_pri_num_timeslots 0x%x\n"),
		             p_qstate->ingress.enable_shaping, p_qstate->ingress.shape_burst_kbps,
		             p_qstate->ingress.hold_pri_num_timeslots));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "template 0x%x, gain 0x%x enabled 0x%x enq_stat_in_use 0x%x deq_stat_in_use 0x%x attached_fifo 0x%x\n"),
		             p_qstate->ingress.template, p_qstate->ingress.gain, p_qstate->ingress.enabled,
		             p_qstate->enq_stat_in_use, p_qstate->deq_stat_in_use, p_qstate->attached_fifo));

		if (p_qstate->mgid_list != NULL) {
		    index = 0;
		    DQ_TRAVERSE(&(p_qstate->mgid_list->node), pList);
		        LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                    (BSL_META_U(unit,
		                                "0x%x "),
		                     ((bcm_sbx_mgid_list_t*)pList)->data));
			if ((index%4)==3 ) {
			    LOG_VERBOSE(BSL_LS_BCM_COSQ,
			                (BSL_META_U(unit,
			                            "\n")));			
			}
			index++;
		    DQ_TRAVERSE_END(&(p_qstate->mgid_list->node), pList);
		}
		break;
	    default:
		return BCM_E_PARAM;
	}
    }

    /* recover mgid_list */
    if ((operation == _WB_OP_DECOMPRESS) &&
	(SOC_SBX_IF_PROTOCOL_SBX != SOC_SBX_CFG(unit)->uInterfaceProtocol)) {

	queueMapBlock = _SIRIUS_I_QUEUE_MAP_SEG_XGS_MULTICAST + SIRIUS_QUEUE_MAP_BLOCK_OFFSET_INGRESS;
	rv = soc_sirius_queue_map_block_get(unit, queueMapBlock, &base, &size);
	if (rv == SOC_E_NONE) {
	    for (qme_index = base; qme_index < (base+size); qme_index++) {
		rv = READ_QUEUE_MAPm(unit, MEM_BLOCK_ANY, qme_index & 0xFFFF, &qme);
		if (rv != SOC_E_NONE){
		    continue;
		}
		queue = soc_mem_field32_get(unit, QUEUE_MAPm, &qme, QID_BASEf);
		if ((queue == SIRIUS_Q_BASE_INVALID) ||
		    (queue >= SOC_SBX_CFG(unit)->num_queues)) {
		    /* ignore unused entry or invalid queues */
		    continue;
		}

		/* insert the mcgroup into the queue's mgid_list */
		p_qstate = &(SOC_SBX_STATE(unit)->queue_state[queue]);
		if (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_IN_USE) {
		    /* ignore unused queue */
		    continue;
		}

		if (p_qstate->mgid_list == NULL) {
		    pRoot = sal_alloc(sizeof(bcm_sbx_mgid_list_t), "Dq root");
		    if (!pRoot) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "unable to allocate memory for dq root\n")));
			rv = BCM_E_RESOURCE;
                        return (rv);
		    }
		    DQ_INIT(&(pRoot->node));
		    pRoot->data = 0;
		    p_qstate->mgid_list = pRoot;
		}

		/* insert mcgroup and all of its alias */
		for (index = 0; index < 4; index++) {
		    mcgroup = ((qme_index-base) & 0x3FFF) | (index << 15);

		    pNew = sal_alloc(sizeof(bcm_sbx_mgid_list_t), "Dq Elem");
		    if (!pNew) {
			LOG_ERROR(BSL_LS_BCM_COSQ,
			          (BSL_META_U(unit,
			                      "unable to allocate dq element\n")));
			rv = BCM_E_RESOURCE;
                        return (rv);
		    } else {
			DQ_INIT(&(pNew->node));
			pNew->data = mcgroup;
		    }
		    DQ_INSERT_TAIL(&(p_qstate->mgid_list->node), pNew);
		    p_qstate->mgid_list->data++;
		}
	    }
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s queue_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s queue_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s queue_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_ingress_scheduler_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int is;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;    
    uint8  tmp_uint8 = 0;
    uint32 tmp_uint32 = 0;
    bcm_sbx_cosq_ingress_scheduler_state_t *is_ptr = NULL;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     *  typedef struct bcm_sbx_cosq_ingress_scheduler_state_s {
     *             sbBool_t in_use;            (TRUE or FALSE, compressed to 1 bit)
     *             int      level;             (0 to 7,        compressed to 3 bits)
     *             int      node;              (0 to 16K,      compressed to 2 bytes)
     *             int      num_childs;        (0 to 8,        compressed to 4 bits)
     *        }bcm_sbx_cosq_ingress_scheduler_state_t;
     *
     */
    
    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG(unit)->num_ingress_scheduler != tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore ingress schedulers."
		                       "%d ingress schedulers in scache, expecting %d ingress schedulers\n"),
		           FUNCTION_NAME(), tmp_uint32, SOC_SBX_CFG(unit)->num_ingress_scheduler));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
            __WB_COMPRESS_SCALAR(uint32,  SOC_SBX_CFG(unit)->num_ingress_scheduler);
	    break;
	default:
	    break;
    }

    for (is = 0; is < SOC_SBX_CFG(unit)->num_ingress_scheduler; is++) {
	is_ptr = &(SOC_SBX_STATE(unit)->ingress_scheduler_state[is]);
	switch (operation) {
	    case _WB_OP_SIZE:		/* calculate structures size requirement for worst case */
	    case _WB_OP_DECOMPRESS:	/* restore strucures from scache */
		__WB_DECOMPRESS_SCALAR(uint8, tmp_uint8);
                __WB_DECOMPRESS_SCALAR(int16, is_ptr->node);
                if (SOC_WARM_BOOT(unit)) {
                    if (tmp_uint8 & 0x1) {
                        is_ptr->in_use = TRUE;
                        is_ptr->level = (tmp_uint8 >> 1) & 0x7;
                        is_ptr->num_childs = (tmp_uint8 >> 4) & 0xF;
                    } else {
                        is_ptr->in_use = FALSE;
                        is_ptr->level = -1;
                        is_ptr->num_childs = 0;
                    }
                }
		break;
	    case _WB_OP_COMPRESS:
		/* save structures to scache */
		tmp_uint8 = ((is_ptr->in_use?1:0) |
			     ((is_ptr->level & 0x7)<<1) |
			     ((is_ptr->num_childs & 0xF)<<4));
		__WB_COMPRESS_SCALAR(uint8, tmp_uint8);		
                __WB_COMPRESS_SCALAR(int16, is_ptr->node);
		break;
	    case _WB_OP_DUMP:
		/* dump the state for debug purpose */
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s dump ingress scheduler node %d on unit %d\n"),
		             FUNCTION_NAME(), is, unit));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "in_use %d, level %d, node %d, num_childs %d\n"),
		             is_ptr->in_use, is_ptr->level, is_ptr->node,
		             is_ptr->num_childs));
		break;
	    default:
		return BCM_E_PARAM;
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s ingress_scheduler_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s ingress_scheduler_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s ingress_scheduler_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_egress_scheduler_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int es;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;    
    uint16 tmp_uint16 = 0;
    uint32 tmp_uint32 = 0;
    bcm_sbx_cosq_egress_scheduler_state_t *es_ptr = NULL;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     *  typedef struct bcm_sbx_cosq_egress_scheduler_state_s {
     *             sbBool_t in_use;            (TRUE or FALSE, compressed to 1 bit)
     *             int      level;             (0 to 5,        compressed to 3 bits)
     *             int      node;              (0 to 132,      compressed to 12 bits)
     *        }bcm_sbx_cosq_egress_scheduler_state_t;
     *
     */    

    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG(unit)->num_egress_scheduler != tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore egress schedulers."
		                       "%d egress schedulers in scache, expecting %d egress schedulers\n"),
		           FUNCTION_NAME(), tmp_uint32, SOC_SBX_CFG(unit)->num_bw_groups));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_egress_scheduler);
	    break;
	default:
	    break;
    }

    for (es = 0; es < SOC_SBX_CFG(unit)->num_egress_scheduler; es++) {
	es_ptr = &(SOC_SBX_STATE(unit)->egress_scheduler_state[es]);
	switch (operation) {
	    case _WB_OP_SIZE:		/* calculate structures size requirement for worst case */
	    case _WB_OP_DECOMPRESS:	/* restore strucures from scache */
		__WB_DECOMPRESS_SCALAR(uint16, tmp_uint16);
                if (SOC_WARM_BOOT(unit)) {
                    if (tmp_uint16 & 0x1) {
                        es_ptr->in_use = TRUE;
                        es_ptr->level = (tmp_uint16 >> 1) & 0x7;
                        es_ptr->node = (tmp_uint16 >> 4) & 0xFFF;
                    } else {
                        es_ptr->in_use = FALSE;
                        es_ptr->level = -1;
                        es_ptr->node = -1;
                    }
                }
		break;
	    case _WB_OP_COMPRESS:
		/* save structures to scache */
		tmp_uint16 = ((es_ptr->in_use?1:0) |
			     ((es_ptr->level & 0x7)<<1) |
			     ((es_ptr->node & 0xFFF)<<4));
		__WB_COMPRESS_SCALAR(uint16, tmp_uint16);		
		break;
	    case _WB_OP_DUMP:
		/* dump the state for debug purpose */
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s dump egress scheduler node %d on unit %d\n"),
		             FUNCTION_NAME(), es, unit));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "in_use %d, level %d, node %d\n"),
		             es_ptr->in_use, es_ptr->level, es_ptr->node));
		break;
	    default:
		return BCM_E_PARAM;
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s egress_scheduler_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s egress_scheduler_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s egress_scheduler_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_ingress_multipath_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int is;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;    
    uint16 tmp_uint16 = 0;
    uint32 tmp_uint32 = 0;
    bcm_sbx_cosq_ingress_multipath_state_t *is_ptr = NULL;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     *  typedef struct bcm_sbx_cosq_ingress_multipath_state_s {
     *             sbBool_t in_use;            (TRUE or FALSE, compressed to 1 bit)
     *             int      level;             (2 to 6,        compressed to 3 bits)
     *             int      node;              (0 to 4K,       compressed to l3 bits with level)
     *             int      num_nodes;         (0 to 4k,       compressed to 15 bits with in_use)
     *        }bcm_sbx_cosq_ingress_multipath_state_t;
     *
     */
    
    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG(unit)->num_ingress_multipath != tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore ingress multipaths."
		                       "%d ingress multipaths in scache, expecting %d ingress multipaths\n"),
		           FUNCTION_NAME(), tmp_uint32, SOC_SBX_CFG(unit)->num_bw_groups));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_ingress_multipath);
	    break;
	default:
	    break;
    }

    for (is = 0; is < SOC_SBX_CFG(unit)->num_ingress_multipath; is++) {
	is_ptr = &(SOC_SBX_STATE(unit)->ingress_multipath_state[is]);
	switch (operation) {
	    case _WB_OP_SIZE:		/* calculate structures size requirement for worst case */
	    case _WB_OP_DECOMPRESS:
		/* restore strucures from scache */
		__WB_DECOMPRESS_SCALAR(uint16, tmp_uint16);
                if (SOC_WARM_BOOT(unit)) {
                    if (tmp_uint16 & 0x1) {
                        is_ptr->in_use = TRUE;
                        is_ptr->num_nodes = (tmp_uint16 >> 1) & 0x7FFF;
                    } else {
                        is_ptr->in_use = FALSE;
                        is_ptr->num_nodes = 0;
                    }
                }
                __WB_DECOMPRESS_SCALAR(uint16, tmp_uint16);
                if (SOC_WARM_BOOT(unit)) {
                    if (is_ptr->in_use == TRUE) {
                        is_ptr->level = tmp_uint16 & 0x7;
                        is_ptr->node = (tmp_uint16 >> 3) & 0x1FFF;
                    } else {
                        is_ptr->level = -1;
                        is_ptr->node = -1;
                    }
                }
		break;
	    case _WB_OP_COMPRESS:
		/* save structures to scache */
		tmp_uint16 = ((is_ptr->in_use?1:0) | ((is_ptr->num_nodes & 0x7FFF)<<1));
		__WB_COMPRESS_SCALAR(uint16, tmp_uint16);		
                tmp_uint16 = ((is_ptr->level & 0x7) | ((is_ptr->node & 0x1FFF)<<3));
                __WB_COMPRESS_SCALAR(int16, tmp_uint16);
		break;
	    case _WB_OP_DUMP:
		/* dump the state for debug purpose */
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s dump ingress multipath node %d on unit %d\n"),
		             FUNCTION_NAME(), is, unit));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "in_use %d, level %d, node %d, num_childs %d\n"),
		             is_ptr->in_use, is_ptr->level, is_ptr->node,
		             is_ptr->num_nodes));
		break;
	    default:
		return BCM_E_PARAM;
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s ingress_multipath_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s ingress_multipath_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s ingress_multipath_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_egress_multipath_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int es, i, es_node;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;    
    uint8  tmp_uint8 = 0;
    uint32 tmp_uint32 = 0;
    bcm_sbx_cosq_egress_multipath_state_t *es_ptr = NULL;
    group_member_table_entry_t group_member_entry;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     *  typedef struct bcm_sbx_cosq_egress_multipath_state_s {
     *             sbBool_t in_use;            (TRUE or FALSE, compressed to 1 bit)
     *             int      node;              (0 to 100,      compressed to 7 bits)
     *             uint32   member[];          (recovered from hardware)
     *        }bcm_sbx_cosq_egress_multipath_state_t;
     *
     */
    
    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG(unit)->num_egress_multipath != tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore egress multipaths."
		                       "%d egress multipaths in scache, expecting %d egress multipaths\n"),
		           FUNCTION_NAME(), tmp_uint32, SOC_SBX_CFG(unit)->num_bw_groups));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_egress_multipath);
	    break;
	default:
	    break;
    }

    for (es = 0; es < SOC_SBX_CFG(unit)->num_egress_multipath; es++) {
	es_ptr = &(SOC_SBX_STATE(unit)->egress_multipath_state[es]);
	switch (operation) {
	    case _WB_OP_SIZE:		/* calculate structures size requirement for worst case */
	    case _WB_OP_DECOMPRESS:	/* restore strucures from scache */
		__WB_DECOMPRESS_SCALAR(uint8, tmp_uint8);
                if (SOC_WARM_BOOT(unit)) {
                    if (tmp_uint8 & 0x1) {
                        es_ptr->in_use = TRUE;
                        es_ptr->node = (tmp_uint8 >> 1) & 0x7F;
                    } else {
                        es_ptr->in_use = FALSE;
                        es_ptr->node = -1;
                        for (i=0; i<BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; i++) {
                            es_ptr->member[i] = 0;
                        }
                    }
		}
		break;
	    case _WB_OP_COMPRESS:
		/* save structures to scache */
		tmp_uint8 = ((es_ptr->in_use?1:0) | ((es_ptr->node & 0x7F)<<1));
		__WB_COMPRESS_SCALAR(uint8, tmp_uint8);
		break;
	    case _WB_OP_DUMP:
		/* dump the state for debug purpose */
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s dump egress multipath node %d on unit %d\n"),
		             FUNCTION_NAME(), es, unit));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "in_use %d, node %d\n members"),
		             es_ptr->in_use, es_ptr->node));
		for (i=0; i<BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; i++) {
		    if (i%8 == 0) {
			LOG_VERBOSE(BSL_LS_BCM_COSQ,
			            (BSL_META_U(unit,
			                        "\n")));
		    }
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "0x%8x "),
		                 es_ptr->member[i]));
		}
		break;
	    default:
		return BCM_E_PARAM;
	}
    }

    if (operation == _WB_OP_DECOMPRESS) {
	/* recover the member table from hardware */
	for (i=0; i<=SOC_MEM_INFO(unit, GROUP_MEMBER_TABLEm).index_max; i++) {
	    SOC_IF_ERROR_RETURN(READ_GROUP_MEMBER_TABLEm(unit, MEM_BLOCK_ANY, i, &group_member_entry));
	    if (soc_mem_field32_get(unit, GROUP_MEMBER_TABLEm, &group_member_entry, ENf) == 0) {
		continue;
	    }
	    es_node = soc_mem_field32_get(unit, GROUP_MEMBER_TABLEm, &group_member_entry, GROUP_NUMf);
	    for (es=0; es< SOC_SBX_CFG(unit)->num_egress_multipath; es++) {
		if ((SOC_SBX_STATE(unit)->egress_multipath_state[es].in_use == TRUE) &&
		    (es_node == SOC_SBX_STATE(unit)->egress_multipath_state[es].node)) {
		    /* put the fifo in the member mask */
		    SOC_SBX_STATE(unit)->egress_multipath_state[es].member[i/32] |= (1<<(i%32));
		    break;
		}
	    }
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s egress_multipath_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s egress_multipath_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s egress_multipath_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_is_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int level, is;
    int32 parent, first_child, num_child, tmp;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    bcm_sbx_sirius_ingress_scheduler_state_t *is_ptr = NULL;
    uint32 tmp_uint32 = 0;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     *  typedef struct bcm_sbx_sirius_ingress_scheduler_state_s {
     *             int16    in_use;                      (compressed to 1 byte)
     *             int16    parent;                      (hw)
     *             bcm_gport_t parent_gport;             (4 bytes)
     *             int32    first_child;                 (hw)
     *             uint16   num_child;                   (hw)
     *             int32    logical_scheduler;           (4 bytes)
     *             int      scheduler_mode;              (compressed to 1 byte)
     *             int      scheduler_param;             (4 bytes)
     *             int      min_shaper_rate_kbps;        (4 bytes)
     *             int      min_shaper_threshold_bits;   (4 bytes)
     *             int      max_shaper_rate_kbps;        (4 bytes)
     *             int      max_shaper_threshold_bits;   (4 bytes)
     *             int      bucket;                      (hw)
     *             int8     adopt_all;                   (1 byte)
     *        }bcm_sbx_sirius_ingress_scheduler_state_t;
     *
     *  Level 0 nodes save in_use only (1 byte)
     *  Nodes of other levels will take 31 bytes when in use, or take 1 byte when not in use
     */
    
    /* calculate each element size based on which member and how the structure is saved */

    for (level = 0; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
	switch (operation) {
	    case _WB_OP_SIZE:
	    case _WB_OP_DECOMPRESS:
		__WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
		if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] != (int32)tmp_uint32)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "%s: failed to restore level %d ingress schedulers."
		                           "%d ingress schedulers in scache, expecting %d ingress schedulers\n"),
		               FUNCTION_NAME(), level, tmp_uint32,
		               SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]));
		    return BCM_E_INTERNAL;
		}
		break;
	    case _WB_OP_COMPRESS:
		__WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]);
		break;
	    default:
		break;
	}

	if ((SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] <= 0) ||
	    (is_state[unit][level] == NULL)) {
	    continue;
	}

	for (is = 0; is < SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level]; is++) {
	    is_ptr = &(is_state[unit][level][is]);
	    switch (operation) {
		case _WB_OP_SIZE:		    /* calculate structures size requirement for worst case */
		case _WB_OP_DECOMPRESS:		    /* restore strucures from scache */
                    if (SOC_WARM_BOOT(unit)) {
                        BCM_SIRIUS_INGRESS_SCHEDULER_STATE_INIT(unit, level, is);
                    }
		    __WB_DECOMPRESS_SCALAR(uint8, is_ptr->in_use);

		    if ((level != 0) && (!SOC_WARM_BOOT(unit) || (is_ptr->in_use == TRUE))) {
			/* level 0 nodes don't need other members of the structure */
			/* reserved node or node not in_use don't need to save other members */

			__WB_DECOMPRESS_SCALAR(bcm_gport_t, is_ptr->parent_gport);
			__WB_DECOMPRESS_SCALAR(int32, is_ptr->logical_scheduler);
			__WB_DECOMPRESS_SCALAR(uint8, is_ptr->scheduler_mode);
			__WB_DECOMPRESS_SCALAR(int, is_ptr->scheduler_param);
			__WB_DECOMPRESS_SCALAR(int, is_ptr->min_shaper_rate_kbps);
			__WB_DECOMPRESS_SCALAR(int, is_ptr->min_shaper_threshold_bits);
			__WB_DECOMPRESS_SCALAR(int, is_ptr->max_shaper_rate_kbps);
			__WB_DECOMPRESS_SCALAR(int, is_ptr->max_shaper_threshold_bits);
			__WB_DECOMPRESS_SCALAR(int8, is_ptr->adopt_all);

                        if (SOC_WARM_BOOT(unit)) {
                            /* parent, first_child and num_child recovered from hw */
                            rv = soc_sirius_ts_node_hierachy_config_get(unit, level, is, &parent, &first_child, &num_child);
                            if (rv != SOC_E_NONE) {
                                LOG_ERROR(BSL_LS_BCM_COSQ,
                                          (BSL_META_U(unit,
                                                      "ERROR: %s, failed to recover level %d node %d state from hardware on unit: %d\n"),
                                           FUNCTION_NAME(), level, is, unit));
                                return BCM_E_INTERNAL;
                            } else {
                                is_ptr->parent = parent;
                                is_ptr->first_child = first_child;
                                is_ptr->num_child = num_child;
                            }
                            
                            /* bucket recovered from hw */
                            rv = soc_sirius_ts_node_mapping_config_get(unit, level, is,
                                                                       &tmp, &tmp,
                                                                       &(is_ptr->bucket));
                            if (rv != SOC_E_NONE) {
                                LOG_ERROR(BSL_LS_BCM_COSQ,
                                          (BSL_META_U(unit,
                                                      "ERROR: %s, failed to recover level %d node %d bucket state from hardware on unit: %d\n"),
                                           FUNCTION_NAME(), level, is, unit));
                                return BCM_E_INTERNAL;
                            }
			}
		    }
		    break;
		case _WB_OP_COMPRESS:
		    /* save structures to scache */
		    __WB_COMPRESS_SCALAR(uint8, is_ptr->in_use);

		    if ((level != 0) && (is_ptr->in_use == TRUE)) {
			__WB_COMPRESS_SCALAR(bcm_gport_t, is_ptr->parent_gport);
			__WB_COMPRESS_SCALAR(int32, is_ptr->logical_scheduler);
			__WB_COMPRESS_SCALAR(uint8, is_ptr->scheduler_mode);
			__WB_COMPRESS_SCALAR(int, is_ptr->scheduler_param);
			__WB_COMPRESS_SCALAR(int, is_ptr->min_shaper_rate_kbps);
			__WB_COMPRESS_SCALAR(int, is_ptr->min_shaper_threshold_bits);
			__WB_COMPRESS_SCALAR(int, is_ptr->max_shaper_rate_kbps);
			__WB_COMPRESS_SCALAR(int, is_ptr->max_shaper_threshold_bits);
			__WB_COMPRESS_SCALAR(int8, is_ptr->adopt_all);

			/* level 0 nodes doesn't need other members of the structure */
			/* reserved node or node not in_use don't need to save other members */
			
			/* parent recovered from hw, no need to save */
			/* first_child recovered from hw, no need to save */
			/* num_child recovered from hw, no need to save */
			/* bucket recovered from hw, no need to save */
		    }
		    break;
		case _WB_OP_DUMP:
		    /* dump the state for debug purpose */
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s dump state level %d node %d on unit %d\n"),
		                 FUNCTION_NAME(), level, is, unit));
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "in_use %d, parent %d, parent_gport 0x%x, first_child %d, num_child %d,\n"
		                             "logical_scheduler 0x%x, scheduler_mode %d, scheduler_param %d,\n"
		                             "min_shaper_rate_kbps 0x%x min_shaper_threshold_bits 0x%x,\n"
		                             "max_shaper_rate_kbps 0x%x max_shaper_threshold_bits 0x%x,\n"
		                             "bucket %d, adopt_all %d\n"), is_ptr->in_use, is_ptr->parent,
		                 is_ptr->parent_gport, is_ptr->first_child, is_ptr->num_child,
		                 is_ptr->logical_scheduler, is_ptr->scheduler_mode, is_ptr->scheduler_param,
		                 is_ptr->min_shaper_rate_kbps, is_ptr->min_shaper_threshold_bits,
		                 is_ptr->max_shaper_rate_kbps, is_ptr->max_shaper_threshold_bits,
		                 is_ptr->bucket, is_ptr->adopt_all));
		    break;
		default:
		    return BCM_E_PARAM;
	    }
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s is_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s is_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s is_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_es_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int level, es, enable, ef;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    bcm_sbx_sirius_egress_scheduler_state_t *es_ptr = NULL;
    uint32 tmp_uint32 = 0;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /*
     * typedef struct bcm_sbx_sirius_egress_scheduler_state_s {
     *         sbBool_t in_use;                      (compressed to 1 byte)
     *         int      intf;                        (hw)
     *         int      channel;                     (hw)
     *         int      subport;                     (hw)
     *         int      num_child;                   (compressed to 1 byte)
     *         int      index;                       (hw)
     *         int      scheduler_mode;              (compressed to 1 byte)
     *         int      scheduler_param;             (4 bytes)
     *         int      min_shaper_rate_kbps;        (4 bytes)
     *         int      min_shaper_threshold_bits;   (4 bytes)
     *         int      max_shaper_rate_kbps;        (4 bytes)
     *         int      max_shaper_threshold_bits;   (4 bytes)
     * }bcm_sbx_sirius_egress_scheduler_state_t;
     *
     * Each node will take 23 bytes when in use, or take 1 byte when not in use
     *
     */

    for (level = 0; level < SB_FAB_DEVICE_SIRIUS_NUM_ES_LEVELS; level++) {
	switch (operation) {
	    case _WB_OP_SIZE:
	    case _WB_OP_DECOMPRESS:
		__WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
		if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level] != (int32)tmp_uint32)) {
		    LOG_ERROR(BSL_LS_BCM_COSQ,
		              (BSL_META_U(unit,
		                          "%s: failed to restore level %d egress schedulers."
		                           "%d egress schedulers in scache, expecting %d egress schedulers\n"),
		               FUNCTION_NAME(), level, tmp_uint32,
		               SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]));
		    return BCM_E_INTERNAL;
		}
		break;
	    case _WB_OP_COMPRESS:
		__WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]);
		break;
	    default:
		break;
	}

	if ((SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level] <= 0) ||
	    (es_state[unit][level] == NULL)) {
	    continue;
	}
	for (es = 0; es < SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[level]; es++) {
	    es_ptr = &(es_state[unit][level][es]);
	    switch (operation) {
		case _WB_OP_SIZE:		    /* calculate structures size requirement for worst case */
		case _WB_OP_DECOMPRESS:
                    if (SOC_WARM_BOOT(unit)) {
                        /* restore strucures from scache */
                        BCM_SIRIUS_EGRESS_SCHEDULER_STATE_INIT(unit, level, es);
                    }
		    __WB_DECOMPRESS_SCALAR(uint8, es_ptr->in_use);
		    if (!SOC_WARM_BOOT(unit) || (es_ptr->in_use == TRUE)) {
			/* node not in_use don't need to save other members */			
			__WB_DECOMPRESS_SCALAR(uint8, es_ptr->num_child);
			__WB_DECOMPRESS_SCALAR(uint8, es_ptr->scheduler_mode);
			__WB_DECOMPRESS_SCALAR(int, es_ptr->scheduler_param);
			__WB_DECOMPRESS_SCALAR(int, es_ptr->min_shaper_rate_kbps);
			__WB_DECOMPRESS_SCALAR(int, es_ptr->min_shaper_threshold_bits);
			__WB_DECOMPRESS_SCALAR(int, es_ptr->max_shaper_rate_kbps);
			__WB_DECOMPRESS_SCALAR(int, es_ptr->max_shaper_threshold_bits);
		    }

                    if (SOC_WARM_BOOT(unit)) {
                        /* recover intf, channel, subport, index recovered from hw */
                        rv = soc_sirius_es_node_hierachy_config_get(unit, level, es, &enable,
                                                                    &es_ptr->intf, &es_ptr->channel,
                                                                    &es_ptr->subport, &es_ptr->index, &ef);
                        if (rv != SOC_E_NONE) {
                            LOG_ERROR(BSL_LS_BCM_COSQ,
                                      (BSL_META_U(unit,
                                                  "ERROR: %s, failed to recover level %d node %d state from hardware on unit: %d\n"),
                                       FUNCTION_NAME(), level, es, unit));
                            return BCM_E_INTERNAL;
                        }
                    }

		    break;
		case _WB_OP_COMPRESS:
		    /* save structures to scache */
		    __WB_COMPRESS_SCALAR(uint8, es_ptr->in_use);

		    if (es_ptr->in_use == TRUE) {
			/* level 0 nodes doesn't need other members of the structure */
			/* reserved node or node not in_use don't need to save other members */
			__WB_COMPRESS_SCALAR(uint8, es_ptr->num_child);
			__WB_COMPRESS_SCALAR(uint8, es_ptr->scheduler_mode);
			__WB_COMPRESS_SCALAR(int, es_ptr->scheduler_param);
			__WB_COMPRESS_SCALAR(int, es_ptr->min_shaper_rate_kbps);
			__WB_COMPRESS_SCALAR(int, es_ptr->min_shaper_threshold_bits);
			__WB_COMPRESS_SCALAR(int, es_ptr->max_shaper_rate_kbps);
			__WB_COMPRESS_SCALAR(int, es_ptr->max_shaper_threshold_bits);
		    }
		    break;
		case _WB_OP_DUMP:
		    /* dump the state for debug purpose */
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "%s dump state level %d node %d on unit %d\n"),
		                 FUNCTION_NAME(), level, es, unit));
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "in_use %d, intf %d, channel %d, subport %d, num_child %d,\n"
		                             "index %d, scheduler_mode %d, scheduler_param %d,\n"
		                             "min_shaper_rate_kbps 0x%x min_shaper_threshold_bits 0x%x,\n"
		                             "max_shaper_rate_kbps 0x%x max_shaper_threshold_bits 0x%x,\n"),
		                 es_ptr->in_use, es_ptr->intf, es_ptr->channel, es_ptr->subport, es_ptr->num_child,
		                 es_ptr->index, es_ptr->scheduler_mode, es_ptr->scheduler_param,
		                 es_ptr->min_shaper_rate_kbps, es_ptr->min_shaper_threshold_bits,
		                 es_ptr->max_shaper_rate_kbps, es_ptr->max_shaper_threshold_bits));
		    break;
		default:
		    return BCM_E_PARAM;
	    }
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s es_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s es_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s es_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_gs_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    int gs;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    bcm_sbx_sirius_egress_group_shaper_state_t *gs_ptr = NULL;
    uint32 tmp_uint32 = 0;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    /* typedef struct bcm_sbx_sirius_egress_group_shaper_state_s {
     *         sbBool_t in_use;                      (compressed to 1 byte)
     *         int      max_shaper_rate_kbps;        (4 bytes)
     *         int      max_shaper_threshold_bits;   (4 bytes)
     * }bcm_sbx_sirius_egress_group_shaper_state_t;
     *
     * Each node will take 9 bytes when in use, or take 1 byte when not in use
     *
     */

    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER != (int32)tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore group shapers."
		                       "%d group shaper in scache, expecting %d group shaper\n"),
		           FUNCTION_NAME(), tmp_uint32,
		           SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER);
	    break;
	default:
	    break;
    }

    for(gs = 0; gs < SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER; gs++) {
	gs_ptr = &(gs_state[unit][gs]);
	switch (operation) {
	    case _WB_OP_SIZE:		/* calculate structures size requirement for worst case */
	    case _WB_OP_DECOMPRESS:	/* restore strucures from scache */
		__WB_DECOMPRESS_SCALAR(uint8, gs_ptr->in_use);
		
		if (!SOC_WARM_BOOT(unit) || (gs_ptr->in_use == TRUE)) {
		    /* node not in_use don't need to save other members */			
		    __WB_DECOMPRESS_SCALAR(int, gs_ptr->max_shaper_rate_kbps);
		    __WB_DECOMPRESS_SCALAR(int, gs_ptr->max_shaper_threshold_bits);
		}
		break;
	    case _WB_OP_COMPRESS:
		/* save structures to scache */
		__WB_COMPRESS_SCALAR(uint8, gs_ptr->in_use);
		
		if (gs_ptr->in_use == TRUE) {
		    /* level 0 nodes doesn't need other members of the structure */
		    /* node not in_use don't need to save other members */
		    __WB_COMPRESS_SCALAR(int, gs_ptr->max_shaper_rate_kbps);
		    __WB_COMPRESS_SCALAR(int, gs_ptr->max_shaper_threshold_bits);
		}
		break;
	    case _WB_OP_DUMP:
		/* dump the state for debug purpose */
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s dump state shaper %d on unit %d\n"),
		             FUNCTION_NAME(), gs, unit));
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "in_use %d, max_shaper_rate_kbps 0x%x max_shaper_threshold_bits 0x%x,\n"),
		             gs_ptr->in_use, gs_ptr->max_shaper_rate_kbps, gs_ptr->max_shaper_threshold_bits));
		break;
	    default:
		return BCM_E_PARAM;
	}
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s gs_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s gs_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s gs_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	default:
	    break;
    }

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_fc_map_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    uint32 byte_len, pri;
    uint32 tmp_uint32 = 0;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    byte_len = sizeof(bcm_sbx_cosq_egress_flow_control_state_t) * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS;

    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS != (int32)tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore fc map."
		                       "%d fc map in scache, expecting %d fc map\n"),
		           FUNCTION_NAME(), tmp_uint32,
		           BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS);
	    break;
	default:
	    break;
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    scache_len += byte_len;
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s fc_map_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            if (fc_map_state[unit] != NULL) {
                sal_memcpy(fc_map_state[unit], ptr, byte_len);
                ptr += byte_len;
            }
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s fc_map_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    if (fc_map_state[unit] != NULL) {
		sal_memcpy(ptr, fc_map_state[unit], byte_len);
		ptr += byte_len;
	    }
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s fc_map_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_DUMP:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s fc_map_state total %d bytes dumped on unit %d\n"),
	                 FUNCTION_NAME(), byte_len, unit));
	    for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "pri 0x%x, int_pri 0x%x flow_control_mask 0x%x,\n"),
		             pri, (fc_map_state[unit]+pri)->int_pri,
		             (fc_map_state[unit]+pri)->flow_control_mask));
	    }
	    break;
	default:
	    break;
    }    

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_fc_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    uint32 byte_len, index;
    uint32 tmp_uint32 = 0;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }

    /* typedef struct {
     * 	 int8          in_use;      (1 byte)
     * 	 bcm_module_t  module;      (4 bytes)
     * } bcm_sbx_sirius_fc_state;
     *
     * Each node will take 5 bytes when in use, or take 1 byte when not in use
     *
     */

    byte_len = (sizeof(bcm_sbx_sirius_fc_state) * SIRIUS_ES_FC_STATE_TABLE_SIZE);

    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (SIRIUS_ES_FC_STATE_TABLE_SIZE != (int32)tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore fc state."
		                       "%d fc state in scache, expecting %d fc state\n"),
		           FUNCTION_NAME(), tmp_uint32,
		           SIRIUS_ES_FC_STATE_TABLE_SIZE));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, SIRIUS_ES_FC_STATE_TABLE_SIZE);
	    break;
	default:
	    break;
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    scache_len += byte_len;
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s fc_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            if (fc_state[unit] != NULL) {
                for (index = 0; index < SIRIUS_ES_FC_STATE_TABLE_SIZE; index++) {
                    __WB_DECOMPRESS_SCALAR(uint8, (fc_state[unit] + index)->in_use);
                    
                    if (!SOC_WARM_BOOT(unit) || ((fc_state[unit] + index)->in_use == TRUE)) {
			__WB_DECOMPRESS_SCALAR(bcm_module_t, (fc_state[unit] + index)->module);
                    }
                }
            }
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s fc_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    if (fc_state[unit] != NULL) {
		for (index = 0; index < SIRIUS_ES_FC_STATE_TABLE_SIZE; index++) {
		    __WB_COMPRESS_SCALAR(uint8, (fc_state[unit] + index)->in_use);

		    if ((fc_state[unit] + index)->in_use == TRUE) {
			__WB_COMPRESS_SCALAR(bcm_module_t, (fc_state[unit] + index)->module);
		    }
		}
	    }
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s fc_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_DUMP:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s fc_state total %d bytes dumped on unit %d\n"),
	                 FUNCTION_NAME(), byte_len, unit));
	    if (fc_state[unit] != NULL) {
		for (index = 0; index < SIRIUS_ES_FC_STATE_TABLE_SIZE; index++) {
		    LOG_VERBOSE(BSL_LS_BCM_COSQ,
		                (BSL_META_U(unit,
		                            "fc_state %d, in_use %d module 0x%x,\n"),
		                 index, (fc_state[unit]+index)->in_use,
		                 (fc_state[unit]+index)->module));
		}
	    }
	    break;
	default:
	    break;
    }    
    
    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

static int
bcm_sirius_wb_cosq_mc_fc_map_state(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int rv = BCM_E_NONE;
    uint32 scache_len = 0;
    uint8  *ptr = NULL, *end_ptr = NULL;
    uint32 byte_len, pri;
    uint32 tmp_uint32 = 0;

    switch (operation) {
	case _WB_OP_SIZE:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot calculate size during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

            if (tmp_len == NULL) {
                return BCM_E_PARAM;
            }
            scache_len = *tmp_len;
            break;
        case _WB_OP_COMPRESS:
            if (SOC_WARM_BOOT(unit)) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "%s: Cannot write scache during WarmBoot, unit %d\n"),
		             FUNCTION_NAME(), unit));
                return SOC_E_INTERNAL;
            }

        case _WB_OP_DECOMPRESS:
            if ((pptr == NULL) ||
                (eptr == NULL)) {
                return BCM_E_PARAM;
            }
            ptr = *pptr;
            end_ptr = *eptr;
            break;
        default:
            break;
    }
    
    byte_len = sizeof(bcm_sbx_cosq_egress_flow_control_state_t) * BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS * 2;

    switch (operation) {
	case _WB_OP_SIZE:
	case _WB_OP_DECOMPRESS:
	    __WB_DECOMPRESS_SCALAR(uint32, tmp_uint32);
	    if (SOC_WARM_BOOT(unit) && (BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS != (int32)tmp_uint32)) {
		LOG_ERROR(BSL_LS_BCM_COSQ,
		          (BSL_META_U(unit,
		                      "%s: failed to restore mc fc map."
		                       "%d mc fc map in scache, expecting %d mc fc map\n"),
		           FUNCTION_NAME(), tmp_uint32,
		           BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS));
		return BCM_E_INTERNAL;
	    }
	    break;
	case _WB_OP_COMPRESS:
	    __WB_COMPRESS_SCALAR(uint32, BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS);
	    break;
	default:
	    break;
    }

    switch (operation) {
	case _WB_OP_SIZE:
	    scache_len += byte_len;
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s mc_fc_map_state total %d bytes reserved on unit %d\n"),
	                 FUNCTION_NAME(), (scache_len - *tmp_len), unit));
	    break;
	case _WB_OP_DECOMPRESS:
            if (mc_fc_map_state[unit] != NULL) {
                sal_memcpy(mc_fc_map_state[unit], ptr, byte_len);
                ptr += byte_len;
            }
            LOG_VERBOSE(BSL_LS_BCM_COSQ,
                        (BSL_META_U(unit,
                                    "%s mc_fc_map_state total %d bytes loaded on unit %d\n"),
                         FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_COMPRESS:
	    if (mc_fc_map_state[unit] != NULL) {
		sal_memcpy(ptr, mc_fc_map_state[unit], byte_len);
		ptr += byte_len;
	    }
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s mc_fc_map_state total %d bytes saved on unit %d\n"),
	                 FUNCTION_NAME(), (ptr - *pptr), unit));
	    break;
	case _WB_OP_DUMP:
	    LOG_VERBOSE(BSL_LS_BCM_COSQ,
	                (BSL_META_U(unit,
	                            "%s mc_fc_map_state total %d bytes dumped on unit %d\n"),
	                 FUNCTION_NAME(), byte_len, unit));
	    for (pri = 0; pri < BCM_SBX_COSQ_STANDARD_FIFO_SET_MAX_NO_COS; pri++) {
		LOG_VERBOSE(BSL_LS_BCM_COSQ,
		            (BSL_META_U(unit,
		                        "pri 0x%x, int_pri 0x%x flow_control_mask 0x%x,\n"),
		             pri, (mc_fc_map_state[unit]+pri)->int_pri,
		             (mc_fc_map_state[unit]+pri)->flow_control_mask));
	    }
	    break;
	default:
	    break;
    }    

    if (tmp_len != NULL) {
	*tmp_len = scache_len;
    }

    if (pptr != NULL) {
        *pptr = ptr;
    }

    return rv;
}

int
bcm_sirius_wb_cosq_state_sync(int unit, uint16 default_ver, uint16 recovered_ver, uint32 *tmp_len, uint8 **pptr, uint8 **eptr, int operation)
{
    int   rv = BCM_E_NONE;
    
    /* ==============================================================
     * following are data structures defined in common/cosq.c but moved here for optimization
     */

    /* queue state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_queue_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));

    /* ==============================================================
     * following are data structures defined in common/cosq.c but apply to sirius only for now
     */

    /* ingress scheduler state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_ingress_scheduler_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));

    /* egress scheduler state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_egress_scheduler_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));

    /* ingress multipath state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_ingress_multipath_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));

    /* egress multipath state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_egress_multipath_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));

    /* ==============================================================
     * following are data structures defined only in this file
     */

    /* is_state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_is_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));    
    
    /* es_state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_es_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));    
    
    /* gs_state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_gs_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));    
    
    /* fc_map_state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_fc_map_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));    
    
    /* fc_state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_fc_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));    
    
    /* mc_fc_map_state */
    BCM_IF_ERROR_RETURN(bcm_sirius_wb_cosq_mc_fc_map_state(unit, default_ver, recovered_ver, tmp_len, pptr, eptr, operation));

    return rv;
}

#endif /* BCM_WARM_BOOT_SUPPORT */
