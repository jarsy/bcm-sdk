/*
 * $Id: cosq.c,v 1.333 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/sirius.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/mbcm.h>
#include <bcm_int/sbx_dispatch.h>

#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/device_cosq.h>
#include <bcm_int/sbx/qe2000.h>
#include <bcm_int/sbx/sirius.h>

#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/port.h>
#include <shared/gport.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <bcm_int/sbx/caladan3/cosq.h>
#endif

int sbx_num_cosq[SOC_MAX_NUM_DEVICES] = {0};

static bcm_sbx_cosq_queue_region_t *queue_regions[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_queue_state_t *queue_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_bw_group_state_t *bw_group_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_sysport_state_t *sysport_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_fcd_state_t *fcd_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_sysport_group_state_t *sysport_group_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_destport_state_t *dp_state[SOC_MAX_NUM_DEVICES] = {NULL};

/* This array is indexed by the eset so that the associated base queue can be determined */
static int *ds_id_base_queue[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_discard_state_t *discard_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_ingress_scheduler_state_t *ingress_scheduler_state[SOC_MAX_NUM_DEVICES] = {NULL};
static bcm_sbx_cosq_egress_scheduler_state_t *egress_scheduler_state[SOC_MAX_NUM_DEVICES] = {NULL};
static bcm_sbx_cosq_egress_group_state_t *egress_group_state[SOC_MAX_NUM_DEVICES] = {NULL};

static bcm_sbx_cosq_ingress_multipath_state_t *ingress_multipath_state[SOC_MAX_NUM_DEVICES];
static bcm_sbx_cosq_egress_multipath_state_t *egress_multipath_state[SOC_MAX_NUM_DEVICES];

#ifdef BCM_EASY_RELOAD_SUPPORT

static int last_restore_queue[SOC_MAX_NUM_DEVICES] = {0};

#endif /* BCM_EASY_RELOAD_SUPPORT */

static int
_bcm_sbx_cosq_queue_state_init(int unit, int32 num_cos);

static int
_bcm_sbx_cosq_bw_group_state_init(int unit, int32 num_cos);

static int
_bcm_sbx_cosq_sysport_state_init(int unit);

static int
_bcm_sbx_cosq_fcd_state_init(int unit);

static int
_bcm_sbx_cosq_destport_state_init(int unit);

static int
_bcm_sbx_cosq_sysport_state_hybrid_init(int unit);

static int
_bcm_sbx_cosq_discard_state_init(int unit);

static int
_bcm_sbx_cosq_scheduler_state_init(int unit);

static uint32
_bcm_sbx_cosq_get_wfq_weight(int unit, int queue);

static int
_bcm_sbx_cosq_queue_group_allocate(int unit,
                                   int32 requested_base_queue,
                                   int32 requested_sysport,
                                   int32 node,
                                   int32 port,
                                   int32 mc,
                                   int32 num_cos,
				   int32 egroup_num,
                                   int32 dest_type,
                                   int32 scan_up,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
                                   int32 *p_allocated_base_queue,
                                   int32 *p_allocated_sysport,
                                   int32 cosq_init_added);

static int
_bcm_sbx_cosq_bw_group_allocate(int unit, int *p_bw_group, int auto_select);

static int
_bcm_sbx_cosq_sysport_allocate(int unit, int flags, int requested_sysport, int dest_node, int fabric_port, int eg_n, bcm_sbx_cosq_queue_region_type_t queue_region, int *p_sysport);

static int
_bcm_sbx_cosq_fcd_allocate(int unit, int flags, int requested_fcd, int sysport, int dest_node, int fabric_port, int fifo, int *p_fcd);

static int
_bcm_sbx_cosq_scheduler_allocate(int unit, bcm_gport_t requested_scheduler, int egress, int *p_scheduler);

static int
_bcm_sbx_cosq_sysport_deallocate(int unit, int sysport);

static int
_bcm_sbx_cosq_fcd_free(int unit, int fcd, int node);

static int
_bcm_sbx_cosq_scheduler_free(int unit, int egress, int gport);

static int
_bcm_sbx_cosq_queue_group_free(int unit, int32 base_queue, int32 num_cos);

static int
_bcm_sbx_cosq_bw_group_free(int unit, int bw_group);

static void
_bcm_sbx_cosq_get_default_queue_ingress_params(int unit, bcm_sbx_cosq_queue_params_ingress_t *qparams, int32 base_queue, int cos);

static void
_bcm_sbx_cosq_get_default_bw_group_params(int unit, bcm_sbx_cosq_bw_group_params_t *bwparams);

static int
_bcm_sbx_cosq_get_base_queue_from_gport(int unit, bcm_gport_t gport, int *p_base_queue, int *p_num_cos_levels);

static int
_bcm_sbx_cosq_get_queue_from_gport(int unit, bcm_gport_t gport, int *p_queue, bcm_cos_queue_t cosq, int *p_num_cos_levels);

static int
_bcm_sbx_cosq_queue_is_local(int unit, int32 queue);

STATIC INLINE int
_bcm_sbx_get_template_ref(int unit, int template);

STATIC INLINE void
_bcm_sbx_dec_template_ref(int unit, int template);

STATIC INLINE void
_bcm_sbx_inc_template_ref(int unit, int template);

static int
_bcm_sbx_allocate_template(int unit, int *template);

static int
_bcm_sbx_deallocate_template(int unit, int template);

static int
_bcm_sbx_template_init(int unit, bcm_cos_queue_t cosq, bcm_sbx_cosq_discard_state_t *discard_state,
		       int min_size, int max_size,
		       bcm_sbx_cosq_discard_state_t *discard_state_src);

static int
_bcm_sbx_template_copy(int unit, bcm_sbx_cosq_discard_state_t *discard_state_dest,
		       bcm_sbx_cosq_discard_state_t *discard_state_src);

static int
_bcm_sbx_template_compare(int unit, bcm_sbx_cosq_discard_state_t *discard_state1,
			  bcm_sbx_cosq_discard_state_t *discard_state2, int mode);

/* static */ int
_bcm_sbx_template_match(int unit, bcm_sbx_cosq_discard_state_t *discard_state_p,
			int *match_template, int mode);

static int
_bcm_sbx_template_collapse(int unit, int template, int *match_template, int mode);

static int
_bcm_sbx_cosq_queue_update_template_gain(int unit, int template, int gain, int queue);

static void
_bcm_sbx_cosq_discard_color_to_internal_color_range(int unit, int color, int *start_color, int *nbr_color);

static int
_bcm_sbx_cosq_queue_discard_disabled(int unit, int template, int disabling_color);


static int
_bcm_cosq_gport_discard_no_change(int unit, int template, int gain, int color,
                                             bcm_cosq_gport_discard_t *discard, uint32 queue_size);

static int
_bcm_sbx_discard_profile_consistency_check(int unit,
                                           bcm_cosq_gport_discard_t *discard,
                                           int queue_sz);
static int
_bcm_sbx_cosq_gport_discard_disable(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               int queue,
                               uint32 color,
                               bcm_cosq_gport_discard_t *discard);

static int
_bcm_sbx_cosq_gport_discard_enable(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               int queue,
                               uint32 color,
                               bcm_cosq_gport_discard_t *discard);

static void
bcm_sbx_cosq_queue_get_template_gain(int unit, int32 queue, int32 *p_template, int32 *p_gain);

static int
bcm_sbx_cosq_queue_set_template_gain(int unit, int32 queue, int32 template, int32 gain);

static int
bcm_sbx_cosq_template_pfc_config(int unit, int32 template);

static int
_bcm_sbx_cosq_multipath_allocate(int unit, bcm_gport_t multipath_requested, int egress,
                                 bcm_gport_t *p_multipath_gport);

static int
_bcm_sbx_cosq_multipath_free(int unit, int egress, int gport);

static int
_bcm_sbx_cosq_pfc_enabled(int unit, bcm_cos_queue_t cos);

static int
_bcm_sbx_cosq_pfc_pg_get(int unit, bcm_cos_queue_t cos);

#ifdef BCM_WARM_BOOT_SUPPORT
#define BCM_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define BCM_WB_DEFAULT_VERSION            BCM_WB_VERSION_1_0

static int
bcm_sbx_wb_cosq_state_init(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */


int
bcm_sbx_cosq_config_set(int unit,
                        bcm_cos_queue_t numq)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (BCM_COSQ_QUEUE_VALID(unit, numq)) {
        sbx_num_cosq[unit] = numq;
        rv = BCM_E_NONE;
    } else if (numq >= NUM_COS(unit)) {
        sbx_num_cosq[unit] = NUM_COS(unit);
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_cosq_config_get(int unit,
                        bcm_cos_queue_t *numq)
{
    BCM_SBX_LOCK(unit);

    *numq = sbx_num_cosq[unit];

    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

int
bcm_sbx_cosq_mapping_set(int unit,
                         bcm_cos_t priority,
                         bcm_cos_queue_t cosq)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_mapping_set, (unit, priority, cosq));
    if (rc != BCM_E_NONE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "error(0x%x) cosq_mapping_set()\n"),
                  rc));
    }

    BCM_SBX_UNLOCK(unit);

    return(rc);
}

int
bcm_sbx_cosq_mapping_get(int unit,
                         bcm_cos_t priority,
                         bcm_cos_queue_t *cosq)
{
    int rc = BCM_E_NONE;


    BCM_SBX_LOCK(unit);

    if (cosq == NULL) {
        BCM_SBX_UNLOCK(unit);
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: API error - cosq pointer not being passed\n")));
        return(BCM_E_PARAM);
    }

    rc = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_mapping_get, (unit, priority, cosq));
    if (rc != BCM_E_NONE) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "error(0x%x) cosq_mapping_get()\n"),
                  rc));
    }

    BCM_SBX_UNLOCK(unit);

    return(rc);
}

/*
 * Add a unicast gport
 *
 * This function supports adding a gport of type BCM_GPORT_UCAST_QUEUE_GROUP
 * or BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP
 *
 * The user specifies a destination physical port (node/port) and num_cos_levels
 * queues are added on the ingress node/unit if the device is a queuing engine/bandwidth
 * manager.
 *
 * The gport returned for ucast queue group is as follows
 *
 *  bit 31       26*25                 13            0
 *      **********************************************
 *      *  type    * sysport (polaris) * queue id    *
 *      **********************************************
 *
 * The gport returned for ucast subscriber queue group is as follows
 *
 *  bit 31       26*25                 15            0
 *      **********************************************
 *      *  type    *                0  * queue id    *
 *      **********************************************
 *
 * The user can specify the queue (voq) id if they set the flag BCM_COSQ_GPORT_WITH_ID.
 * Otherwise, the queue is allocated from the free pool.
 *
 */
int
bcm_sbx_cosq_gport_add(int unit,
                       bcm_gport_t physical_port,
                       int num_cos_levels,
                       uint32 flags,
                       bcm_gport_t *req_gport)
{
    int rv = BCM_E_NONE;
    int user_allocated_req_gport = FALSE;
    int user_gport_overlay = FALSE;
    int user_scheduler = FALSE;
    int user_egroup = FALSE;
    int user_multipath = FALSE;
    int sysport_requested = BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT;
    int base_queue_requested =  BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT;
    int scheduler_requested = BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT;
    int sysport = -1;
    int queue = -1;
    int scheduler = -1, egress_group = -1;
    int base_queue;
    int level, node = 0, port = 0, fabric_port = 0, cos, max_cos, eg_n = -1, num_fifos = -1;
    int modid = 0;
    int my_modid = ~0, my_node;
    int bw_group;
    int egress = FALSE;
    int logical_scheduler;
    int protocol;
    int fifo_idx = 0;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_queue_params_ingress_t queue_params;
    bcm_sbx_cosq_bw_group_params_t bw_group_params;
    bcm_sbx_cosq_queue_region_type_t queue_region = bcm_sbx_cosq_queue_region_global;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int shaper_inc;
    bcm_gport_t tmp_gport = BCM_GPORT_INVALID;

    /* These are use only in the overlay case where we need to look up the existing state */
    bcm_sbx_cosq_bw_group_state_t *p_bwstate_in;
    bcm_sbx_cosq_queue_state_t *p_qstate_in;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport add request, unit(%d) physical_port(0x%x) num_cos_levels(%d) flags(0x%x) *req_gport(0x%x)\n"),
              unit, physical_port, num_cos_levels, flags, *req_gport));


#if 0 /* removed bcm_cosq_init soc property */
    /* gport_add is unavailable when cosq_init initializes all the queues */
    /* unless we are in standalone mode                                   */
    if ((SOC_SBX_CFG(unit)->bcm_cosq_init == TRUE) &&
        !(soc_feature(unit, soc_feature_standalone))) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
#endif

    /* Decode flags */

    if (BCM_COSQ_GPORT_SCHEDULER & flags) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "allocate scheduler gport\n")));
        user_scheduler = TRUE;
    }

    if (BCM_COSQ_GPORT_EGRESS_GROUP & flags) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "allocate egress group gport\n")));
	user_egroup = TRUE;
    }

    if (BCM_COSQ_GPORT_MULTIPATH & flags) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "allocate multipath gport\n")));
	user_multipath = TRUE;
	if (flags & ~BCM_COSQ_GPORT_MULTIPATH) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_MULTIPATH can not be used with other flags\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
    }

    if (!req_gport) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: user needs to provide pointer to req_gport to be filled with gport info\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* The user wants to allocate the qid */
    if (BCM_COSQ_GPORT_WITH_ID & flags) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "user allocated %s\n"),
                  (SOC_IS_SBX_BME(unit) && user_egroup) ? "egress group" : "qid"));
        user_allocated_req_gport = TRUE;
    }

    if ((BCM_COSQ_GPORT_WITHOUT_SHAPING & flags) ||
	(BCM_COSQ_GPORT_WITH_SHAPING & flags)) {
	if ((BCM_COSQ_GPORT_WITHOUT_SHAPING & flags) &&
	    (BCM_COSQ_GPORT_WITH_SHAPING & flags)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Can not set both BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING flag\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;		
	}	

	if (!SOC_IS_SIRIUS(unit)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING flags not supported on unit %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

        if (!(soc_feature(unit, soc_feature_standalone) ||
	      soc_feature(unit, soc_feature_hybrid)     ||
	      soc_feature(unit, soc_feature_node_hybrid))  && 
	    (BCM_COSQ_GPORT_WITHOUT_SHAPING & flags)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: all FIC queues has ingress shaper, BCM_COSQ_GPORT_WITHOUT_SHAPING flags not supported under fic mode %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

	rv = bcm_fabric_control_get(unit, bcmFabricShaperEgressQueueIncrement, &shaper_inc);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: failed to get local queue shaper increment config on unit %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	} else if (shaper_inc != 1) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING requires local queue"
	                           "shaper increment be 1 on unit %d\n"), unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;		
	}
	
	if (soc_feature(unit, soc_feature_hybrid) ||
	    soc_feature(unit, soc_feature_node_hybrid)) {
	    rv = bcm_fabric_control_get(unit, bcmFabricShaperQueueIncrement, &shaper_inc);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: failed to get voq queue shaper increment config on unit %d\n"),
		           unit));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    } else if (shaper_inc != 1) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING requires voq queue"
		                       "shaper increment be 1 on unit %d\n"), unit));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;		
	    }
	}

        if ((BCM_COSQ_GPORT_WITH_ID & flags) ||
	    (BCM_COSQ_GPORT_EGRESS_GROUP & flags) ||
	    (BCM_COSQ_GPORT_SCHEDULER & flags)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING flags not supported with other flags %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

    }

    /* If in EASY_RELOAD, all queues must be allocated 'with id' */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
        if (!(BCM_COSQ_GPORT_WITH_ID & flags)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: user needs to provide gport with ID if reloading (in easy_reload mode)\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
#endif
    }

    if (BCM_COSQ_GPORT_OVERLAY & flags) {
        /* Check that the user passed in the Queue ID for Overlay */
        if (user_allocated_req_gport != TRUE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: GPORT Overlay requires User-defined (predefined) QID to be passed in\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        } else {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "user qid overlay\n")));
            user_gport_overlay = TRUE;
        }
    }

    if (user_multipath) {
	/* allocate multipath shaper */
	if (BCM_GPORT_IS_MODPORT(physical_port)) {
	    egress = FALSE;
	} else if (BCM_GPORT_IS_EGRESS_MODPORT(physical_port)) {
	    egress = TRUE;
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Invalid physical port for multipath\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;	    
	}

	if (!SOC_IS_SIRIUS(unit)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: multipath gport not supported on unit %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
	}

	rv = _bcm_sbx_cosq_multipath_allocate(unit, BCM_INT_SBX_REQUESTED_MULTIPATH_AUTO_SELECT,
					      egress, req_gport);
	BCM_SBX_UNLOCK(unit);
	return rv;
    }

    if (user_scheduler) {
	/* allocate logical handle for scheduler */
	if (BCM_GPORT_IS_MODPORT(physical_port)) {
	    egress = FALSE;
	} else if (BCM_GPORT_IS_EGRESS_MODPORT(physical_port)) {
	    egress = TRUE;
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Invalid physical port for scheduler\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;	    
	}

	if (!SOC_IS_SIRIUS(unit)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: scheduler gport not supported on unit %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
	}

	if (BCM_COSQ_GPORT_WITH_ID & flags) {
	    if (BCM_GPORT_IS_SCHEDULER(*req_gport) &&
		(egress == FALSE)) {
		/* this mode user is mananging hardware resources, for now, only allows it on ingress schedulers */
		if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode < 0) {
		    SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = TRUE;
		} else if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == FALSE) {
		    rv = BCM_E_PARAM;
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: SDK managed scheduler resource automatically before,"
		                           "can not switch back to user scheduler management mode now\n")));
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}

		/* get level/node */
		level = BCM_INT_SBX_SCHEDULER_LEVEL_GET(*req_gport);
		node = BCM_INT_SBX_SCHEDULER_NODE_GET(*req_gport);

		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_allocate, (unit, level, node,
                                                                               1, egress, &scheduler));
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: scheduler allocate 0x%x failed error(%d)\n"),
		               *req_gport, rv));
		}
		BCM_SBX_UNLOCK(unit);
		return rv;
	    } else {
		rv = BCM_E_PARAM;
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: If user wants to manage scheduler resource directly, has to specify a"
		                       "valid ingress scheduler resource. Egress schdulers could not be managed directly\n")));
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	} else {
	    scheduler_requested = BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT;
	    if (egress == FALSE) {
		if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode < 0) {
		    SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = FALSE;
		} else if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
		    rv = BCM_E_PARAM;
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: User managed scheduler resource directly before,"
		                           "can not switch back to SDK scheduler management mode now\n")));
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
	    }
	}
	rv = _bcm_sbx_cosq_scheduler_allocate(unit, scheduler_requested, egress, &scheduler);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Could not allocate scheduler rv %d\n"),
	               rv));
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}

	if ((egress == FALSE) && (num_cos_levels > 0) && 
	    (scheduler_requested == BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT)) {
	    logical_scheduler = BCM_INT_SBX_SCHEDULER_ID_GET(scheduler);
	    if (num_cos_levels <= 8) {
		ingress_scheduler_state[unit][logical_scheduler].num_childs = num_cos_levels;
	    } else {
		LOG_WARN(BSL_LS_BCM_COMMON,
		         (BSL_META_U(unit,
		                     "Could not preallocate more than 8 children ingress schedulers, default to 8\n")));
		num_cos_levels = 8;
		ingress_scheduler_state[unit][logical_scheduler].num_childs = 8;
	    }
	}

	*req_gport = scheduler;
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NONE;
    } else if (user_egroup) {
	/* allocate egress groups */
	if (BCM_GPORT_IS_CHILD(physical_port)) {
	    /* Higig channels */
	    modid = BCM_GPORT_CHILD_MODID_GET(physical_port);
	    fabric_port = BCM_GPORT_CHILD_PORT_GET(physical_port);
	} else if (BCM_GPORT_IS_EGRESS_CHILD(physical_port)) {
	    /* Higig channels */
	    modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(physical_port);
	    fabric_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(physical_port);
	    if ((fabric_port >= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE0) &&
		(fabric_port <= SB_FAB_DEVICE_SIRIUS_MCAST_HANDLE3)) {
		egress = TRUE;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Unsupported physical gport when creating egress group\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;	    
	}	

	if ((SOC_SBX_CFG(unit)->fabric_egress_setup) && (egress == FALSE)) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Egress groups unavailable when fabric_egress_setup=1\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
	}
	
	/* Make sure egress group handle returned is similiar and consistent with fabric ports */

	rv = bcm_sbx_cosq_egress_group_allocate(unit, BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT, modid,
						 fabric_port, num_cos_levels, &egress_group);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Could not allocate egress group\n")));
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}

	*req_gport = egress_group;
	BCM_SBX_UNLOCK(unit);
	return BCM_E_NONE;
    } else if (BCM_GPORT_IS_MODPORT(physical_port)) {

        /* The physical port is of the form mod/port */
        modid = BCM_GPORT_MODPORT_MODID_GET(physical_port);
	port = BCM_GPORT_MODPORT_PORT_GET(physical_port);

        if (modid < BCM_MODULE_FABRIC_BASE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: gport modid invalid(%d) minimum (%d)\n"),
                       modid, BCM_MODULE_FABRIC_BASE));
            rv = BCM_E_PARAM;
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
        node = modid - BCM_MODULE_FABRIC_BASE;

	if (node < BCM_STK_MAX_MODULES) {
	    protocol = SOC_SBX_STATE(unit)->stack_state->protocol[node];
	} else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unknown node type for modid(%d)\n"),
                       modid));
            rv = BCM_E_INIT;
            BCM_SBX_UNLOCK(unit);
            return rv;
	}

	/* check the destination node type */
	if ( (protocol == bcmModuleProtocol3) || (protocol == bcmModuleProtocol4) || (protocol == bcmModuleProtocol5)) {
	    /* destination node is sirius */
	    if ( port == SB_FAB_DEVICE_SIRIUS_CPU_PORT ) {
		fabric_port = SB_FAB_DEVICE_SIRIUS_CPU_HANDLE;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: gport type MODPORT, must be cpu port\n")));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	} else {
	    /* Determine if SPI port or CPU port and subtract the offset     */
	    /* from the port number passed in.  The port number is based on  */
	    /* the bit position of the port in the ports bit mask.  All      */
	    /* ports are stored in the ports bit mask, including sci, sfi,   */
	    /* cpu, etc.. The spi subports must be contiguous.               */
	    BCM_INT_SBX_GPORT_GET_FABRICPORT_FROM_MODPORT(physical_port, &fabric_port);

	    if (fabric_port < 0) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: gport type MODPORT, port field invalid\n")));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	}

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "fabric_port=%d\n"),
                  fabric_port));

    } else if (BCM_GPORT_IS_EGRESS_GROUP(physical_port)) {

        modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(physical_port);

        if (modid < BCM_MODULE_FABRIC_BASE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: gport modid invalid(%d) minimum (%d)\n"),
                       modid, BCM_MODULE_FABRIC_BASE));
            rv = BCM_E_PARAM;
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
        node = modid - BCM_MODULE_FABRIC_BASE;

        
	fabric_port = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, physical_port, &fabric_port, &eg_n, &num_fifos);
	if (rv != BCM_E_NONE) {
	  LOG_ERROR(BSL_LS_BCM_COMMON,
	            (BSL_META_U(unit,
	                        "ERROR: %s, gport 0x%x does not contain fabric_port, unit %d\n"),
	             FUNCTION_NAME(), physical_port, unit));
          BCM_SBX_UNLOCK(unit);
	  return BCM_E_PARAM;
	}

    } else if (BCM_GPORT_IS_EGRESS_CHILD(physical_port)) {

        modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(physical_port);

        if (modid < BCM_MODULE_FABRIC_BASE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: gport modid invalid(%d) minimum (%d)\n"),
                       modid, BCM_MODULE_FABRIC_BASE));
            rv = BCM_E_PARAM;
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
        node = modid - BCM_MODULE_FABRIC_BASE;

        
        fabric_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(physical_port);
    }
    else if (BCM_GPORT_IS_CHILD(physical_port)) {

        modid = BCM_GPORT_CHILD_MODID_GET(physical_port);

        if (modid < BCM_MODULE_FABRIC_BASE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: gport modid invalid(%d) minimum (%d)\n"),
                       modid, BCM_MODULE_FABRIC_BASE));
            rv = BCM_E_PARAM;
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
        node = modid - BCM_MODULE_FABRIC_BASE;

        
        fabric_port = BCM_GPORT_CHILD_PORT_GET(physical_port);
    }

    /* This is a locally-switched queue */
    if ((BCM_COSQ_GPORT_LOCAL & flags) ||
        soc_feature(unit, soc_feature_standalone)) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "locally switched queue\n")));
        queue_region = bcm_sbx_cosq_queue_region_local;
	if (SOC_IS_SBX_QE2000(unit) || SOC_IS_SBX_SIRIUS(unit)) {
	  rv = bcm_sbx_stk_modid_get(unit, &my_modid);
	  if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR (%s) getting modid for unit:%d\n"),
	               bcm_errmsg(rv),unit));
	    BCM_SBX_UNLOCK(unit);
	    return(rv);
	  }

	  my_node = BCM_STK_MOD_TO_NODE(my_modid);
	  if (node != my_node) {
	    /* Local port not for me. Forget it. */
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
	  }
        }
    } else {
        queue_region = bcm_sbx_cosq_queue_region_global;
    }

    /* consistency check on num cos levels */
    
    if (BCM_GPORT_IS_MODPORT(physical_port)) {
        if (queue_region == bcm_sbx_cosq_queue_region_global) {
            port = BCM_GPORT_MODPORT_PORT_GET(physical_port);
            /* only for CPU port allow 16 cos levels */
            max_cos = (IS_CPU_PORT(unit, port)? 16 : 
                       BCM_SBX_COSQ_GLOBAL_MAX_NO_COS);
            if (num_cos_levels > max_cos) {
                rv = BCM_E_PARAM;
                BCM_SBX_UNLOCK(unit);

                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR:invalid queue group size(%d) max supported(%d)\n"),
                           (int)num_cos_levels, BCM_SBX_COSQ_GLOBAL_MAX_NO_COS));
                return(rv);
            }
        }
        else if (queue_region == bcm_sbx_cosq_queue_region_local) {
            port = BCM_GPORT_MODPORT_PORT_GET(physical_port);
            /* only for CPU port allow 16 cos levels */
            max_cos = (IS_CPU_PORT(unit, port)? 16 : 
                       BCM_SBX_COSQ_LOCAL_MAX_NO_COS);
            if (num_cos_levels > max_cos) {
                rv = BCM_E_PARAM;
                BCM_SBX_UNLOCK(unit);

                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR:invalid queue group size(%d) max supported(%d)\n"),
                           (int)num_cos_levels, BCM_SBX_COSQ_LOCAL_MAX_NO_COS));
                return(rv);
            } else if ((num_cos_levels == 0) || 
		       ((num_cos_levels % SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE) != 0)) {
                rv = BCM_E_PARAM;
                BCM_SBX_UNLOCK(unit);

                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR:invalid queue group size(%d)\n"),
                           (int)num_cos_levels));
                return(rv);
            }
        }
    }


    if (user_allocated_req_gport) {

      if (queue_region == bcm_sbx_cosq_queue_region_global) {
        /* the input req_gport is of the form sysport/qid */
        sysport_requested = BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(*req_gport);
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "sysport=%d\n"),
                  sysport_requested));
        if ( sysport_requested ==  _SHR_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_MASK ) {
            sysport_requested = BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT;
        }

        if (SOC_IS_RELOADING(unit) && (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
            if (sysport_requested == BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT) {
                rv = BCM_E_PARAM;
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "User must specify SYSPORT with ID if EASY_RELOAD and reloading software\n")));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
#endif
        }

        base_queue_requested = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(*req_gport);
      } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
        base_queue_requested = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(*req_gport);
      }
      LOG_INFO(BSL_LS_BCM_COSQ,
               (BSL_META_U(unit,
                           "base_queue_requested=%d\n"),
                base_queue_requested));

        if (base_queue_requested > SOC_SBX_CFG(unit)->num_queues) {

            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Queue(%d) requested is out of range (0-%d)\n"),
                       base_queue_requested, SOC_SBX_CFG(unit)->num_queues));

            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    }

    if (user_gport_overlay ) {
        p_qstate_in = &queue_state[unit][base_queue_requested];
        bw_group = p_qstate_in->bw_group;
        p_bwstate_in = &bw_group_state[unit][bw_group];


        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Gport Overlay 0x%x found num_cos 0x%x sysport 0x%x\n"),
                  *req_gport, p_bwstate_in->num_cos, sysport_requested));

	/*
	 * For QE2K, set overlay flag in ref_cnt to indicate 
	 * to sysport_allocate to ignore node_cnt
	 */
	if (dp_state[unit] != NULL) {
	    dp_state[unit][fabric_port].ref_cnt |= BCM_INT_SBX_DP_IGNORE;
	}

        /* allocate sysport for non-EF stream */
        rv = _bcm_sbx_cosq_sysport_allocate(unit, 0, sysport_requested, node, fabric_port, eg_n, queue_region, &sysport);

	/*
	 * For QE2K, set overlay flag in ref_cnt to indicate 
	 * to sysport_allocate to ignore node_cnt
	 */
	if (dp_state[unit] != NULL) {
	    dp_state[unit][fabric_port].ref_cnt &= ~BCM_INT_SBX_DP_IGNORE;
	}

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Gport overlay sysport allocate sysport_requested %d Node %d returned sysport 0x%x\n"),
                  sysport_requested, node, sysport));

        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Could not allocate sysport\n")));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        /* Check the size of the overlay request */
        if (num_cos_levels > p_bwstate_in->num_cos) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  " cos overlay count request(%d) exceeds allocated group size (%d)\n"),
                       num_cos_levels, p_bwstate_in->num_cos));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        /* Adjust the Overlay base_queue address */
        base_queue = base_queue_requested + p_bwstate_in->num_cos - num_cos_levels;

        if (rv == BCM_E_NONE) {
	  if (queue_region == bcm_sbx_cosq_queue_region_global) {
            /* Set up the returned unicast queue group, return different base queue and sysport */
            BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(*req_gport, sysport, base_queue);
	  } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
            /* Set up the returned unicast subscriber queue group, return different base queue */
            BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*req_gport, base_queue);
	  }
        }

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Setting overlay traffic of size %d at 0x%x. Original Base at 0x%x\n"),
                  num_cos_levels, base_queue, base_queue_requested));

        for ( queue=base_queue; queue < base_queue+num_cos_levels; queue++ ) {
            queue_state[unit][queue].sysport = sysport;
            queue_state[unit][queue].default_sysport = sysport;
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Overlay queue %d on Unit %d switch to sysport %d\n"),
                      queue, unit, sysport));

            cos = queue - base_queue_requested;
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_overlay_queue, (unit, queue, queue_region, sysport, node, fabric_port, 0,
                                      cos, BCM_INT_SBX_DEST_TYPE_UNICAST));

            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: overlay queue(%d) failed error(%d)\n"),
                           queue, rv));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
        }

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Overlay Node %d Fabric port %d mc %d num_cos %d base_queue 0x%x dest_type 0x%x\n"),
                  node, fabric_port, 0, num_cos_levels, base_queue, BCM_INT_SBX_DEST_TYPE_UNICAST));

        bw_group_state[unit][bw_group].overlay_in_use = TRUE;
        bw_group_state[unit][bw_group].overlay_dest_node = node;
        bw_group_state[unit][bw_group].overlay_dest_port = fabric_port;
        bw_group_state[unit][bw_group].overlay_num_cos = num_cos_levels;
        bw_group_state[unit][bw_group].overlay_base_queue = base_queue;

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "**** DEBUG Overlay Ret GPORT 0x%x SysPort 0x%x base_queue 0x%x\n"),
                  *req_gport, sysport, base_queue));

    } else {
	if (BCM_COSQ_GPORT_WITHOUT_SHAPING & flags) {
	    if (base_queue_requested == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) {
		base_queue_requested = BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING;
	    }
	} else if (BCM_COSQ_GPORT_WITH_SHAPING & flags) {
	    if (base_queue_requested == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) {
		base_queue_requested = BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING;
	    }
	}

        rv =  _bcm_sbx_cosq_queue_group_allocate(unit, base_queue_requested, sysport_requested,
                                                 node, fabric_port, 0, num_cos_levels, eg_n,
                                                 BCM_INT_SBX_DEST_TYPE_UNICAST, TRUE /* scan_up */, queue_region,
                                                 &base_queue, &sysport, FALSE /* cosq_init_added */);

        if (rv < 0) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: queue group resource unavailable\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_RESOURCE;
        }
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "base_queue(%d) sysport(%d) dest node(%d) dest qe port(%d)\n"),
                  base_queue, sysport, node, fabric_port));

        p_qstate = &queue_state[unit][base_queue];
        bw_group = p_qstate->bw_group;
        p_bwstate = &bw_group_state[unit][bw_group];

        p_bwstate->gport = physical_port;

        /* Get the default bw_group parameters and those set up from the SOC */
        _bcm_sbx_cosq_get_default_bw_group_params(unit, &bw_group_params);
        p_bwstate->path = bw_group_params;
        p_bwstate->flags = flags;
	p_bwstate->dest_port_kbits_sec_max = SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps;
        p_bwstate->dest_type = BCM_INT_SBX_DEST_TYPE_UNICAST;
        queue = base_queue;


        for ( cos = 0; cos < num_cos_levels; cos++) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport add unicast queue(%d) for destination node(%d) fabric_port(%d) cos(%d)\n"),
                      queue, node, fabric_port, cos));

            _bcm_sbx_cosq_get_default_queue_ingress_params(unit, &queue_params, base_queue, cos);

            p_qstate =  &queue_state[unit][queue];
            p_qstate->ingress = queue_params;

            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_add_queue, (unit, queue,
                                      queue_region, sysport,
                                      -1 /* eset placeholder */, node,
                                      fabric_port, 0, cos,
                                      BCM_INT_SBX_DEST_TYPE_UNICAST,
                                      &queue_params, &bw_group_params,
                                      (0 != (flags & BCM_COSQ_GPORT_QSEL_ENTRY_DISABLE))));

            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: add queue(%d) failed error(%d)\n"),
                           queue, rv));
                BCM_SBX_UNLOCK(unit);
                return rv;
            }
	    p_qstate->ingress.enabled = TRUE;

	    /* update templates, add queue function doesn't really pick template well */
	    if (soc_feature(unit, soc_feature_ingress_size_templates) &&
		soc_feature(unit, soc_feature_priority_flow_control) &&
		(_bcm_sbx_cosq_pfc_enabled(unit, cos) == TRUE)) {
		/* NOTE: if this cos level has PFC enabled, have to make sure the min depth is 0
		 * this might cause inconsistency default queue depth between qe and bm device
		 * it should not cause WRED issues since queue min depth is not used in WRED
		 * calculation and min size could really be different for individual nodes
		 */
		if (queue_region == bcm_sbx_cosq_queue_region_global) {
		    BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(tmp_gport, sysport, base_queue);
		} else if (queue_region == bcm_sbx_cosq_queue_region_local) {
		    BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(tmp_gport, base_queue);
		}

		rv = bcm_sbx_cosq_gport_size_set(unit, tmp_gport, cos, 0,
						 queue_params.max_physical_queue_depth_bytes);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: queue size set(%d) failed error(%d)\n"),
		               queue, rv));
		    BCM_SBX_UNLOCK(unit);		    
		    return rv;
		}		
	    }

            queue++;
        }

        /* set the target for unicast or distribution group as/if appropriate */
        if ((BCM_E_NONE == rv) &&
	    (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE)) {
            /*
             *  Note that some devices don't need this function, so if it's not
             *  implemented for the device, we don't consider it an error.
             */

	    /* this gport is not in an aggregate; set it up as unicast */
            if (SOC_IS_SIRIUS(unit)) {
                rv = bcm_stk_modid_get(unit, &my_modid);
                if (rv != BCM_E_NONE){
                    BCM_SBX_UNLOCK(unit);
                    return rv;
                }
	    }
	    if ((BCM_GPORT_IS_CHILD(physical_port)) ||
		(BCM_GPORT_IS_EGRESS_CHILD(physical_port)) ||
		(BCM_GPORT_IS_EGRESS_GROUP(physical_port))) {
		fabric_port = -1;
		rv = bcm_sbx_cosq_egress_group_info_get(unit, physical_port, &fabric_port, &eg_n, &num_fifos);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
		               FUNCTION_NAME(), physical_port, unit));
                    BCM_SBX_UNLOCK(unit);
		    return BCM_E_PARAM;
		}
		if (BCM_GPORT_IS_CHILD(physical_port)) {
		    modid = BCM_GPORT_CHILD_MODID_GET(physical_port);
		} else if (BCM_GPORT_IS_EGRESS_CHILD(physical_port)) {
		    modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(physical_port);
		} else {
		    modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(physical_port);
		}
		if (modid == my_modid) {
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
                    if (SOC_IS_SIRIUS(unit) && SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode) {
                        fifo_idx = fabric_port;
                    } else {
                        fifo_idx = (sp_info->egroup[eg_n].es_scheduler_level0_node / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE);
                    }
		    fcd_state[unit][sp_info->egroup[eg_n].ef_fcd].sysport = sysport;
		    fcd_state[unit][sp_info->egroup[eg_n].ef_fcd].fifo = sp_info->egroup[eg_n].es_scheduler_level0_node;
		    fcd_state[unit][sp_info->egroup[eg_n].nef_fcd].sysport = sysport;
		    fcd_state[unit][sp_info->egroup[eg_n].nef_fcd].fifo = sp_info->egroup[eg_n].es_scheduler_level0_node;
		}
	    } else {
		fifo_idx = fabric_port;
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[0]);
		modid = BCM_GPORT_MODPORT_MODID_GET(physical_port);
		if (modid == my_modid) {
		    if ((sp_info != NULL) && (fcd_state[unit] != NULL)) {
		      if (sp_info[fabric_port].egroup[0].es_scheduler_level0_node < SB_FAB_DEVICE_SIRIUS_FIFO_NUM) {
			  fifo_idx = sp_info[fabric_port].egroup[0].es_scheduler_level0_node / SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE;
		      }
			if (sp_info[fabric_port].egroup[0].ef_fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) {
			    fcd_state[unit][sp_info[fabric_port].egroup[0].ef_fcd].sysport = sysport;
			    fcd_state[unit][sp_info[fabric_port].egroup[0].ef_fcd].fifo = sp_info[fabric_port].egroup[0].es_scheduler_level0_node;
			    fcd_state[unit][sp_info[fabric_port].egroup[0].nef_fcd].sysport = sysport;
			    fcd_state[unit][sp_info[fabric_port].egroup[0].nef_fcd].fifo = sp_info[fabric_port].egroup[0].es_scheduler_level0_node;
			} else {
			    LOG_ERROR(BSL_LS_BCM_COMMON,
			              (BSL_META_U(unit,
			                          "ERROR: %s, Egress Group not initialized for fabric_port %d, unit %d\n"),
			               FUNCTION_NAME(), fabric_port, unit));
                            BCM_SBX_UNLOCK(unit);
			    return BCM_E_PARAM;			    
			}
		    }
		}
	    }

            rv = MBCM_SBX_DRIVER_MAYBE_CALL(unit,
                                            mbcm_cosq_target_set,
                                            (unit,
                                             physical_port,
                                             my_modid,
                                             sysport,
                                             fifo_idx));
            if (BCM_E_NONE != rv) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Unable to set target for unit %d port %08X module %d"
                                       " fabricPort %d on sysport %d: %d (%s)\n"),
                           unit,
                           physical_port,
                           my_modid,
                           fabric_port,
                           sysport,
                           rv,
                           _SHR_ERRMSG(rv)));
            }
        }

        if (rv == BCM_E_NONE) {
	  if (queue_region == bcm_sbx_cosq_queue_region_global) {
            /* Set up the returned unicast queue group, return different base queue and sysport */
            BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(*req_gport, sysport, base_queue);
	  } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
            /* Set up the returned unicast subscriber queue group, return different base queue */
            BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*req_gport, base_queue);
	  }
        }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;

}

/*
 *  Get the sysport from an egress group or egress child
 */
int
bcm_sbx_cosq_gport_sysport_from_egress_object(int unit, bcm_gport_t port, int *ef_sysport, int *nef_sysport)
{
    int target = -1;
    int egroup = 0;
    int fifos = 0;
    int result = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info = NULL;

    if ((!ef_sysport) || (!nef_sysport)) {
        return BCM_E_PARAM;
    }

    BCM_SBX_LOCK(unit);
    if (BCM_GPORT_IS_EGRESS_GROUP(port)) {
        result = bcm_sbx_cosq_egress_group_info_get(unit,
                                                    port,
                                                    &target,
                                                    &egroup,
                                                    &fifos);
    } else if (BCM_GPORT_IS_CHILD(port)) {
        target = BCM_GPORT_CHILD_PORT_GET(port);
        egroup = 0;
    } else if (BCM_GPORT_IS_EGRESS_CHILD(port)) {
        target = BCM_GPORT_EGRESS_CHILD_PORT_GET(port);
        egroup = 0;
    } else {
        result = BCM_E_PARAM;
    }
    if (BCM_E_NONE == result) {
        sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[target]);
        *ef_sysport = fcd_state[unit][sp_info->egroup[egroup].ef_fcd].sysport;
        *nef_sysport = fcd_state[unit][sp_info->egroup[egroup].nef_fcd].sysport;
    }
    BCM_SBX_UNLOCK(unit);
    return result;
}


/*
 * Get info about a unicast queue gport or a cosq_init created mod/port gport
 *
 * This function supports getting data about  a gport of type BCM_GPORT_UCAST_QUEUE_GROUP
 * or BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP or BCM_GPORT_MODPORT
 *
 * Information returned includes the destination QE mod(node)/port, the number of cos levels
 * allocated and flags indicating whether the user allocated the queue or not.
 *
 */
int
bcm_sbx_cosq_gport_get(int unit,
                       bcm_gport_t gport,
                       bcm_gport_t *physical_port,
                       int *num_cos_levels,
                       uint32 *flags)
{
    int rv = BCM_E_UNAVAIL;
    int base_queue = 0;
    int num_cos;
    int bw_group;
    int node, fabric_port;
    int modid = 0;
    int port;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;

    BCM_SBX_LOCK(unit);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport get request, unit(%d) gport(0x%x) *physical_port(%d) *num_cos_levels(0x%x)\n"),
              unit, gport, *physical_port, *num_cos_levels));

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if ((!physical_port) || (!num_cos_levels) || (!flags)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: API error - required pointers not being passed\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *physical_port = -1;
    *num_cos_levels = -1;

    rv = _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);

    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    /* If the gport is not in use, return error */
    p_qstate = &queue_state[unit][base_queue];

    if (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_IN_USE) {
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "gport(0x%x) base_queue(%d) not in use\n"),
                  gport, base_queue));
        rv = BCM_E_PARAM;
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    *num_cos_levels = num_cos;

    bw_group = p_qstate->bw_group;
    p_bwstate = &bw_group_state[unit][bw_group];

    *flags = p_bwstate->flags;

    node = p_bwstate->dest_node;
    fabric_port = p_bwstate->dest_port;

    BCM_INT_SBX_GPORT_GET_MODPORT_FROM_FABRICPORT(0 /*qetype=qe2000 */,
                                                   fabric_port, &port);
    
    if (((SOC_IS_SIRIUS(unit)) && (p_qstate->ingress.local == bcm_sbx_cosq_queue_region_global)) ||
	(SOC_IS_SBX_QE2000(unit)) || SOC_IS_SBX_BM9600(unit)) {
	modid = node + BCM_MODULE_FABRIC_BASE;
    }

    BCM_GPORT_MODPORT_SET(*physical_port, modid, port);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport(0x%x) base_queue(%d) num_cos_levels(%d) node(%d) port(%d)\n"),
              gport, base_queue, *num_cos_levels, node, port));

    /* all other types currently unavailable */
    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Add a multicast gport
 *
 * This function supports adding a gport of type BCM_GPORT_MCAST_QUEUE_GROUP
 * or BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP.
 *
 * The user specifies a multicast destination egress set (ds_id/eset) and num_cos_levels
 * queues are added on the ingress node/unit if the device is a queuing engine/bandwidth
 * manager.
 *
 * The gport returned for mcast queue group is as follows
 *
 *  bit 31       26*25                 13            0
 *      **********************************************
 *      *  type    * sysport (polaris) * queue id    *
 *      **********************************************
 *
 * The gport returned for mcast subscriber queue group is as follows
 *
 *  bit 31       26*25                 15            0
 *      **********************************************
 *      *  type    *                0  * queue id    *
 *      **********************************************
 *
 * The user can specify the base queue (voq) id if they set the flag BCM_COSQ_GPORT_WITH_ID.
 * Otherwise, num_cos_level queues are allocated from the free pool.
 *
 */
int
bcm_sbx_cosq_fabric_distribution_add(int unit,
                                     bcm_fabric_distribution_t ds_id,
                                     int num_cos_levels,
                                     uint32 flags,
                                     bcm_gport_t *req_gport)
{
    int rv = BCM_E_NONE;
    int user_allocated_req_gport = FALSE;
    int base_queue;
    int node, port, mc, cos;
    int sysport = 0;
    int sysport_requested = BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT;
    int base_queue_requested =  BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT;
    int queue = -1;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int bw_group;
    bcm_sbx_cosq_queue_params_ingress_t queue_params;
    bcm_sbx_cosq_bw_group_params_t bw_group_params;
    int eset;
    bcm_sbx_cosq_queue_region_type_t queue_region = bcm_sbx_cosq_queue_region_global;
    int shaper_inc;
    bcm_gport_t tmp_gport = BCM_GPORT_INVALID;

    BCM_SBX_LOCK(unit);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport fabric distribution add unit(%d) ds_id(0x%x) num_cos_levels(%d) flags(0x%x) *req_gport(0x%x)\n"),
              unit, ds_id, num_cos_levels, flags, *req_gport));

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }


    if (soc_feature(unit,soc_feature_standalone)) {
      queue_region = bcm_sbx_cosq_queue_region_local;
    }
/*     if ( soc_feature(unit, soc_feature_node) && */
/*                                             soc_feature(unit, soc_feature_standalone) ) { */
/*         queue_region = bcm_sbx_cosq_queue_region_local; */
/*     } */

#if 0
    /* gport_add is unavailable when cosq_init initializes all the queues */
    /* if we are not standalone mode                                      */
    if ((SOC_SBX_CFG(unit)->bcm_cosq_init == TRUE) &&
        !(soc_feature(unit, soc_feature_standalone))) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }
#endif

    if (!req_gport) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: user needs to provide pointer to req_gport\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* If in EASY_RELOAD, all queues must be allocated 'with id' */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
        if (!(BCM_COSQ_GPORT_WITH_ID & flags)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: user needs to provide gport with ID if reloading (in easy_reload mode)\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
#endif
    }

    /* The distribution ID is the ESET.  This needs to be written to the        */
    /* node/port field in various QE tables (q2ec, e2q) so that the destination */
    /* will correctly be given to the BM3200 for replication on multiple nodes  */
    eset = (int)ds_id;
    if (eset >= SOC_SBX_CFG(unit)->num_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: ds_id(%d) out of range (0-%d) valid\n"),
                   ds_id, (SOC_SBX_CFG(unit)->num_ds_ids - 1)));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    BCM_INT_SBX_COSQ_GET_NODE_PORT_FROM_ESET(unit, eset, &node, &port, &mc);

    /* Decode flags */
    if (BCM_COSQ_GPORT_WITH_ID & flags) {
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "user allocated base queue\n")));
        user_allocated_req_gport = TRUE;
    }

    if ((BCM_COSQ_GPORT_WITHOUT_SHAPING & flags) ||
	(BCM_COSQ_GPORT_WITH_SHAPING & flags)) {
	if ((BCM_COSQ_GPORT_WITHOUT_SHAPING & flags) &&
	    (BCM_COSQ_GPORT_WITH_SHAPING & flags)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: Can not set both BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING flag\n")));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;		
	}	

	if (!SOC_IS_SIRIUS(unit)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING flags not supported on unit %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

        if (!(soc_feature(unit, soc_feature_standalone) ||
	      soc_feature(unit, soc_feature_hybrid)     ||
	      soc_feature(unit, soc_feature_node_hybrid))   && 
	    (BCM_COSQ_GPORT_WITHOUT_SHAPING & flags)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: all FIC queues has ingress shaper, BCM_COSQ_GPORT_WITHOUT_SHAPING flags not supported under fic mode %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

	rv = bcm_fabric_control_get(unit, bcmFabricShaperEgressQueueIncrement, &shaper_inc);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: failed to get local queue shaper increment config on unit %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	} else if (shaper_inc != 1) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING requires local queue"
	                           "shaper increment be 1 on unit %d\n"), unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;		
	}
	
	if (soc_feature(unit, soc_feature_hybrid) ||
	    soc_feature(unit, soc_feature_node_hybrid)) {
	    rv = bcm_fabric_control_get(unit, bcmFabricShaperQueueIncrement, &shaper_inc);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: failed to get voq queue shaper increment config on unit %d\n"),
		           unit));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    } else if (shaper_inc != 1) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING requires voq queue"
		                       "shaper increment be 1 on unit %d\n"), unit));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;		
	    }
	}

        if ((BCM_COSQ_GPORT_WITH_ID & flags) ||
	    (BCM_COSQ_GPORT_EGRESS_GROUP & flags) ||
	    (BCM_COSQ_GPORT_SCHEDULER & flags)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: BCM_COSQ_GPORT_WITH_SHAPING/BCM_COSQ_GPORT_WITHOUT_SHAPING flags not supported with other flags %d\n"),
	               unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

    }

    if (user_allocated_req_gport) {

      if (queue_region == bcm_sbx_cosq_queue_region_global) {

        /* the input req_gport is of the form sysport/qid */
        sysport_requested = BCM_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_GET(*req_gport);
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "sysport=%d\n"),
                  sysport_requested));
        if ( sysport_requested == _SHR_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_MASK ) {
            sysport_requested = BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT;
        }

        base_queue_requested = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(*req_gport);
      } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
	if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(*req_gport)) {
	  base_queue_requested = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(*req_gport);
	} else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(*req_gport)) {
	  base_queue_requested = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(*req_gport);
	}
      }

        if (base_queue_requested < 0) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: req_gport value invalid\n")));
            BCM_SBX_UNLOCK(unit);
            return BCM_E_PARAM;
        }
    }

    if (BCM_COSQ_GPORT_WITHOUT_SHAPING & flags) {
	if (base_queue_requested == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) {
	    base_queue_requested = BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING;
	}
    } else if (BCM_COSQ_GPORT_WITH_SHAPING & flags) {
	if (base_queue_requested == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) {
	    base_queue_requested = BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING;
	}
    }

    rv =  _bcm_sbx_cosq_queue_group_allocate(unit, base_queue_requested, sysport_requested,
                                             node, port, mc, num_cos_levels, -1,
                                             BCM_INT_SBX_DEST_TYPE_MULTICAST, TRUE /* scan_up */, queue_region,
                                             &base_queue, &sysport, FALSE /* cosq_init_added */);

    if (rv < 0) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: queue group resource unavailable\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_RESOURCE;
    }


    p_qstate = &queue_state[unit][base_queue];
    bw_group = p_qstate->bw_group;
    p_bwstate = &bw_group_state[unit][bw_group];

    /* Get the default bw_group parameters and those set up from the SOC */
    _bcm_sbx_cosq_get_default_bw_group_params(unit, &bw_group_params);
    p_bwstate->path = bw_group_params;
    p_bwstate->flags = flags;
    p_bwstate->dest_type = BCM_INT_SBX_DEST_TYPE_MULTICAST;
    p_bwstate->dest_port_kbits_sec_max = SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps;
 
    queue = base_queue;

    for (cos = 0 ; cos < num_cos_levels; cos++) {

        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "gport add allocate multicast queue(%d) for eset(%d) destination node(%d) port(%d) cos(%d)\n"),
                  queue, eset, node, port, cos));


        _bcm_sbx_cosq_get_default_queue_ingress_params(unit, &queue_params, base_queue, cos);

        p_qstate =  &queue_state[unit][queue];
        p_qstate->ingress = queue_params;

        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_add_queue, (unit, queue,
                                  queue_region, sysport, eset, node,
                                  port, mc, cos,
                                  BCM_INT_SBX_DEST_TYPE_MULTICAST,
                                  &queue_params, &bw_group_params,
                                  FALSE));

        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: add queue(%d) failed error(%d)\n"),
                       queue, rv));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }
	p_qstate->ingress.enabled = TRUE;

	/* update templates, add queue function doesn't really pick template well */
	if (soc_feature(unit, soc_feature_ingress_size_templates)) {
	    if (queue_region == bcm_sbx_cosq_queue_region_global) {
		BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(tmp_gport, sysport, base_queue);
	    } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
		BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(tmp_gport, base_queue);
	    }
	    
	    rv = bcm_sbx_cosq_gport_size_set(unit, tmp_gport, cos, 
					     queue_params.min_physical_queue_depth_bytes,
					     queue_params.max_physical_queue_depth_bytes);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: queue size set(%d) failed error(%d)\n"),
		           queue, rv));
		BCM_SBX_UNLOCK(unit);		    
		return rv;
	    }		
	}

        queue++;
    }
    if (queue_region == bcm_sbx_cosq_queue_region_global) {
      BCM_GPORT_MCAST_QUEUE_GROUP_SYSQID_SET(*req_gport, sysport, base_queue);
    } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
      BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*req_gport, base_queue);
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_fabric_distribution_get(int unit,
                                     bcm_fabric_distribution_t ds_id,
                                     int *num_cos_levels,
                                     bcm_gport_t *req_gport,
                                     uint32 *flags)
{
    int rv = BCM_E_NONE;
    int base_queue = 0;
    int bw_group;
    bcm_sbx_cosq_queue_region_type_t queue_region = bcm_sbx_cosq_queue_region_global;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if ((!req_gport) || (!num_cos_levels) || (!flags)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error - required pointers not being passed\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }
    if (ds_id >= SOC_SBX_CFG(unit)->num_ds_ids) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error, ds_id(%d) out of valid range (0-%d)\n"),
                   ds_id,
                   (SOC_SBX_CFG(unit)->num_ds_ids - 1)));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if ( soc_feature(unit, soc_feature_node) &&
	 soc_feature(unit, soc_feature_standalone) ) {
        queue_region = bcm_sbx_cosq_queue_region_local;
    }

    *req_gport = -1;
    *num_cos_levels = -1;

    base_queue = ds_id_base_queue[unit][ds_id];

    if (base_queue == -1) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Queue not allocated for this distribution_group(%d)\n"),
                   ds_id));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    bw_group = queue_state[unit][base_queue].bw_group;
    *num_cos_levels = bw_group_state[unit][bw_group].num_cos;
    *flags = bw_group_state[unit][bw_group].flags;

    /* If the queues were pre-initialized, this eset must be of type MCAST */
#if 0
    if (SOC_SBX_CFG(unit)->bcm_cosq_init == TRUE) {
        BCM_GPORT_MCAST_DS_ID_SET(*req_gport, ds_id);
    } else {
#endif
      if (queue_region == bcm_sbx_cosq_queue_region_global) {
        BCM_GPORT_MCAST_QUEUE_GROUP_SET(*req_gport, base_queue);
      } else if (queue_region == bcm_sbx_cosq_queue_region_local) {
        BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*req_gport, base_queue);
      }
#if 0
    }
#endif

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport(0x%x) base_queue(%d) num_cos_levels(%d) ds_id(%d)\n"),
              *req_gport, base_queue, *num_cos_levels, ds_id));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_delete(int unit,
                          bcm_gport_t gport)
{
    int rv = BCM_E_UNAVAIL;
    int base_queue;
    int bw_group;
    int egress;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int num_cos;
    int queue;
    bcm_sbx_cosq_queue_region_type_t queue_region;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {

        base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);

    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {

        base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {

        base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(gport);
    } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {

        base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
    } else if (BCM_GPORT_IS_SCHEDULER(gport)) {
        egress = BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport);
        rv = _bcm_sbx_cosq_scheduler_free(unit, egress, gport);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "delete %s scheduler gport(0x%x) failed\n"),
                       (egress?"egress":"ingress"), gport));
        }
        BCM_SBX_UNLOCK(unit);
        return rv;
    } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	rv = bcm_sbx_cosq_egress_group_free(unit, gport);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "delete egress group 0x%x failed\n"),
                       gport));
        }
        BCM_SBX_UNLOCK(unit);
        return rv;    
    } else if (BCM_COSQ_GPORT_IS_MULTIPATH(gport)) {
        egress = BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport);
        rv = _bcm_sbx_cosq_multipath_free(unit, egress, gport);
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "delete %s multipath gport(0x%x) failed\n"),
                       (egress?"egress":"ingress"), gport));
        }
        BCM_SBX_UNLOCK(unit);
        return rv;	
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: unsupported gport type\n")));
        rv = BCM_E_PARAM;
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    bw_group = queue_state[unit][base_queue].bw_group;
    p_bwstate = &bw_group_state[unit][bw_group];

    _bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);

    if (p_bwstate->in_use) {

        /* We need to check that overlays are cleared first */
        if(p_bwstate->overlay_in_use) {
            if (base_queue == p_bwstate->overlay_base_queue) {
                /* overlay gport */

                
                num_cos = p_bwstate->overlay_num_cos;
                if ( (num_cos < 0) || (num_cos > p_bwstate->num_cos) ) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "delete overlay gport(0x%x) failed\n"),
                               gport));
                    BCM_SBX_UNLOCK(unit);
                    return BCM_E_INTERNAL;
                }

                for (queue = base_queue; queue<base_queue+num_cos; queue++) {
                    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_delete_overlay_queue, (unit, queue, p_bwstate->base_queue, queue_region));

                    if (rv != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "delete overlay queue(%d) failed\n"),
                                   queue));
                        BCM_SBX_UNLOCK(unit);
                        return BCM_E_INTERNAL;
                    }

                    /* use the sysport of base queue */
                    p_qstate = &queue_state[unit][queue];

                    /* free sysport associated with the queue */
                    if ( p_qstate->sysport != BCM_INT_SBX_INVALID_SYSPORT ) {
                        _bcm_sbx_cosq_sysport_deallocate(unit, p_qstate->sysport);
                    }
                    p_qstate->sysport = queue_state[unit][p_bwstate->base_queue].sysport;
                }

                p_bwstate->overlay_in_use = FALSE;
                p_bwstate->overlay_dest_node = -1;
                p_bwstate->overlay_dest_port = -1;
                p_bwstate->overlay_num_cos = -1;
                p_bwstate->overlay_base_queue = -1;
                BCM_SBX_UNLOCK(unit);
                return BCM_E_NONE;
            } else {
                /* original gport */
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Overlay queue exists base queue(%d) num_cos(%d). Delete overlay gport first\n"),
                           bw_group_state[unit][bw_group].overlay_base_queue, bw_group_state[unit][bw_group].overlay_num_cos));
                BCM_SBX_UNLOCK(unit);
                return BCM_E_INTERNAL;
            }
        } else {
            /* Delete all queues from hardware */
            num_cos = p_bwstate->num_cos;

            for (queue = base_queue; queue<base_queue+num_cos; queue++) {
		
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_delete_queue, (unit, queue, queue_region));

                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "delete queue(%d) failed\n"),
                               queue));
                    BCM_SBX_UNLOCK(unit);
                    return BCM_E_INTERNAL;
                }
            }

            /* Update the state, mark queue state, sysport state as available */
            _bcm_sbx_cosq_queue_group_free(unit, base_queue, num_cos);

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "delete gport(0x%x) base_queue(%d) num_cos_levels(%d)\n"),
                      gport, base_queue, num_cos));
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: bw_group(%d) not in use\n"),
                   bw_group));
        rv = BCM_E_RESOURCE;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_traverse(int unit,
                            bcm_cosq_gport_traverse_cb cb,
                            void *user_data)
{

  bcm_sbx_cosq_queue_state_t *p_qstate;
  bcm_sbx_cosq_bw_group_state_t *p_bwstate;
  bcm_gport_t *p_gport = NULL;
  int queue;
  int32 base_queue = 0;
  int32 sys_port = 0;
  int32 dest_type = 0;
  uint8 bQueuesInUse = FALSE;
  int retval = BCM_E_NONE;
  int rv;
  int cmd;
  bcm_sbx_cosq_queue_region_type_t queue_region = bcm_sbx_cosq_queue_region_global;
  bcm_sbx_gport_cb_params_t *p_cb_params = (bcm_sbx_gport_cb_params_t *)user_data;

  BCM_SBX_LOCK(unit);
  if (p_cb_params != NULL) {
    cmd = p_cb_params->cmd;
  } else {
    cmd = BCM_SBX_COSQ_GPORT_DELETE; /* default */
  }

  if (SOC_IS_SBX_FE(unit)) {
    BCM_SBX_UNLOCK(unit);
    return BCM_E_UNAVAIL;
  }

  if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
      BCM_SBX_UNLOCK(unit);
      return(BCM_E_UNAVAIL);
  }

  if ( soc_feature(unit, soc_feature_node) &&
       soc_feature(unit, soc_feature_standalone) ) {
    queue_region = bcm_sbx_cosq_queue_region_local;
  }

  p_gport = sal_alloc(sizeof(bcm_gport_t), "gport");
  if (p_gport == NULL) {
    LOG_ERROR(BSL_LS_BCM_COMMON,
              (BSL_META_U(unit,
                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
               FUNCTION_NAME(), unit));
    BCM_SBX_UNLOCK(unit);
    return(BCM_E_MEMORY);
  }

  for (queue = 0; queue < SOC_SBX_CFG(unit)->num_queues; queue++) {
    p_qstate = &queue_state[unit][queue];
    if (p_qstate->state == BCM_INT_SBX_QUEUE_STATE_IN_USE) {
      bQueuesInUse = TRUE;
      sys_port = p_qstate->sysport;
      p_bwstate = &bw_group_state[unit][p_qstate->bw_group];
      base_queue = p_bwstate->base_queue;
      dest_type = p_bwstate->dest_type;
      if ( soc_feature(unit, soc_feature_hybrid) ) {
	_bcm_sbx_cosq_queue_type_get(unit, queue, &queue_region);
      }
      /* build the GPORT */
      if (dest_type != BCM_INT_SBX_DEST_TYPE_MULTICAST) {
	if (queue_region == bcm_sbx_cosq_queue_region_global)
	  BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(*p_gport, sys_port, base_queue);
	else if (queue_region == bcm_sbx_cosq_queue_region_local)
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*p_gport, base_queue);
      } else {
	if (queue_region == bcm_sbx_cosq_queue_region_global)
	  BCM_GPORT_MCAST_QUEUE_GROUP_SYSQID_SET(*p_gport, sys_port, base_queue);
	else if (queue_region == bcm_sbx_cosq_queue_region_local)
	  BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(*p_gport, base_queue);
      }
      if (cmd == BCM_SBX_COSQ_GPORT_DELETE) {
        /* only delete gport/queues which were not added during bcm init */
        if (*p_gport != BCM_GPORT_INVALID && p_bwstate->cosq_init_added == FALSE) {
          /* call callback to delete it */
          LOG_INFO(BSL_LS_BCM_COSQ,
                   (BSL_META_U(unit,
                               "deleting gport(0x%x) base_queue(%d) sysport(%d)\n"),
                    *p_gport,base_queue,sys_port));
          rv = cb(unit,0/*port*/,0/*numq*/,0/*flags*/,*p_gport,0);
          if (BCM_FAILURE(rv)) {
            LOG_CLI((BSL_META_U(unit,
                                "Deleting gport(0x%x) failed (%s)\n"),*p_gport,
                     bcm_errmsg(rv)));
            retval = rv;
          }
        } else {
          LOG_INFO(BSL_LS_BCM_COSQ,
                   (BSL_META_U(unit,
                               "skipping gport(0x%x) queue(%d) sysport(%d) - (%s)\n"),
                    *p_gport,base_queue,sys_port,p_bwstate->cosq_init_added?"added during init":"is invalid"));
        }
      } else {
        rv = cb(unit,0/*port*/,0/*numq*/,0/*flags*/,*p_gport,user_data);
        if (BCM_FAILURE(rv)) {
          retval = rv;
        }
      }
      /* skip over num_cos queues that belong to this gport 
         -1 is to account for queue++ in for loop increment */
      queue += (p_bwstate->num_cos - 1); continue;
    }
  }

  if (bQueuesInUse == FALSE) {
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "%s[u:%d] has no queues in use.\n"),
              SOC_CHIP_STRING(unit),unit));
  }

  BCM_SBX_UNLOCK(unit);

  if (p_gport != NULL) {
    sal_free(p_gport);
  }
  return retval;

}

int
bcm_sbx_cosq_gport_size_set(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            uint32 bytes_min,
                            uint32 bytes_max)
{
    int rv = BCM_E_NONE;
    int queue = -1;
    bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
    bcm_sbx_cosq_queue_params_ingress_t old_qparams, *p_newqparams;
    bcm_sbx_cosq_bw_group_params_t *p_bwparams;
    int bw_group;
    int num_cos;
    bcm_sbx_cosq_queue_region_type_t queue_region;
    bcm_sbx_queue_size_info_t sizeInfo = {0, 0};
    int template, old_template, old_gain;
    int modid = -1, my_modid = -1, eg_n = 0;
    bcm_sbx_cosq_discard_state_t tmp_discard_state;
    int found = FALSE, update_template = FALSE;
    int color = 0;

    BCM_SBX_LOCK(unit);

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit))
    {
      	int    queue = cosq;
      	rv = bcm_caladan3_cosq_src_queue_set(unit, queue, bytes_min, bytes_max);
        BCM_SBX_UNLOCK(unit);
      	return rv;
    }
#endif

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* check if the operation is for Egress Port.                              */
    /* Functionality is currently implemented for QE2000 device. As additional */
    /* devices support this functionality this function will has to be         */
    /* accordingly enhanced (e.g. handle different "gport" types               */
    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	BCM_GPORT_IS_EGRESS_MODPORT(gport) ||
	BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        rv = bcm_sbx_stk_modid_get(unit, &my_modid);
	if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport size set, error getting modid for unit %d\n"),
                      unit));
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}

	if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	    modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
	} else if  (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	    modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
	} else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	    modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
	    p_eg = SOC_SBX_STATE(unit)->egress_group_state;
	    eg_n = BCM_GPORT_EGRESS_GROUP_GET(gport);
	    if ((p_eg == NULL) || (eg_n < 0) || (eg_n >= SOC_SBX_CFG(unit)->num_egress_group)) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, egress group gport 0x%x not valid, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));		
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }

	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "Unknown port derived from gport (0x%x)\n"),
	               gport));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

        if (modid != my_modid) {
            BCM_SBX_UNLOCK(unit);
            return(BCM_E_UNAVAIL);
        }

        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_egress_size_set, (unit,
                                  gport, cosq, bytes_min, bytes_max));
        if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport size set, error setting Egress Size\n")));
        }
        BCM_SBX_UNLOCK(unit);
        return(rv);
    } else if (gport == BCM_GPORT_INVALID) {
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_egress_size_set, (unit,
                                  gport, cosq, bytes_min, bytes_max));
        if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport size set, error setting Egress Size\n")));
        }
        BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);

    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    _bcm_sbx_cosq_queue_type_get(unit, queue, &queue_region);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport size set queue(%d) cos(%d)\n"),
              queue, cosq));

    /* for PFC enabled queues, hardware require min queue depths to be 0 */
    if (soc_feature(unit, soc_feature_priority_flow_control) &&
	(_bcm_sbx_cosq_pfc_enabled(unit, cosq) == TRUE) &&
	(bytes_min != 0)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Unit(%d) Cos %d has PFC feature enabled, queue MinThreshold must be 0.\n"),
	           FUNCTION_NAME(), unit, cosq)); 
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (queue_state[unit][queue].ingress.template != 0) {
	for (color=0; color < BCM_SBX_COSQ_DISCARD_MAX_COLORS; color++) {
	    if ((discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].flags == 0)
		continue;
	    if ((bytes_max < (discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].min_thresh) ||
		(bytes_max < (discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].max_thresh) ||
		(bytes_max < (discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].ecn_thresh)) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Unit(%d) Invalid max size 0x%x conflicts with discard template color 0x%x: "
		                       "MinThreshold: 0x%x MaxThreshold: 0x%x EcnThrehold: 0x%x\n"), FUNCTION_NAME(), unit, 
		           bytes_max, (1 << color),
		           (discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].min_thresh,
		           (discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].max_thresh,
		           (discard_state[unit]+queue_state[unit][queue].ingress.template)->config[color].ecn_thresh));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	}
    }

    /* Get old configuration */
    old_qparams = queue_state[unit][queue].ingress;

    /* Set new configuration */
    queue_state[unit][queue].ingress.min_physical_queue_depth_bytes = bytes_min;
    queue_state[unit][queue].ingress.max_physical_queue_depth_bytes = bytes_max;
    p_newqparams = &queue_state[unit][queue].ingress;

    bw_group = queue_state[unit][queue].bw_group;
    p_bwparams = &bw_group_state[unit][bw_group].path;

    /* Find old template of the queue */
    bcm_sbx_cosq_queue_get_template_gain(unit, queue, &old_template, &old_gain);

    if ((bytes_min != old_qparams.min_physical_queue_depth_bytes) ||
	(bytes_max != old_qparams.max_physical_queue_depth_bytes) ) {
	/* Check queue region type */
	if (queue_region == bcm_sbx_cosq_queue_region_global) {
	    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
		/* FIC queues on sirius */
		/* use templat 0 (WRED disabled) but use a different min/max size */
		rv = _bcm_sbx_template_init(unit, cosq, &tmp_discard_state, bytes_min,
					    bytes_max, discard_state[unit]);
		if (rv != BCM_E_NONE) {
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
		
		/* Search for a size template that matches the new min/max size */
		found =_bcm_sbx_template_match(unit, &tmp_discard_state, &template,
					       BCM_INT_SBX_QUEUE_SIZE_TEMPLATE);
		
		update_template = TRUE;
	    } else if (SOC_IS_SBX_BME(unit)) {
		/* FIC queues on BME devices */
		/* Search for a FIC WRED template that matches queue size and all FIC WRED parameter */
		rv = _bcm_sbx_template_init(unit, cosq, &tmp_discard_state, bytes_min, bytes_max,
					    (discard_state[unit]+old_template));
		if (rv != BCM_E_NONE) {
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
		
		found =_bcm_sbx_template_match(unit, &tmp_discard_state, &template,
					       BCM_INT_SBX_FIC_WRED_TEMPLATE);
		
		update_template = TRUE;
	    } else {
		/* FIC queues on qe2000, need to do nothing and just */
		update_template = FALSE;
	    }
	} else {
	    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
		/* Local queues on sirius */
		/* Search for a template that matches both the new min/max size and
		 *   WRED parameters of old queue template.
		 */
		rv = _bcm_sbx_template_init(unit, cosq, &tmp_discard_state, bytes_min, bytes_max,
					    (discard_state[unit]+old_template));
		if (rv != BCM_E_NONE) {
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
		
		found =_bcm_sbx_template_match(unit, &tmp_discard_state, &template,
					       BCM_INT_SBX_LOCAL_WRED_TEMPLATE);
		update_template = TRUE;
	    } else {
		
		update_template = FALSE;
	    }
	}

	if (update_template == TRUE) {
	    if (found) {
		/* Found, increment usage count of the matched template */
		if (old_template == template) {
		    BCM_SBX_UNLOCK(unit);
		    return BCM_E_NONE;
		} else {
		    _bcm_sbx_inc_template_ref(unit, template);
		}
	    } else {
		/* Not found, allocate new template */
		rv = _bcm_sbx_allocate_template(unit, &template);
		if (rv != BCM_E_NONE) {
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
		
		/* copy over old wred parameters and new min/max size info */
		rv = _bcm_sbx_template_copy(unit, (discard_state[unit]+template),
					    &tmp_discard_state);
		if (rv != BCM_E_NONE) {
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
		
		/* update the new template */
		rv = bcm_sbx_update_template(unit, template);
		if (rv != BCM_E_NONE) {
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
	    }
	    
	    /* Point the queue to use the new template */
	    p_newqparams->template = template;
	    
	    /* decrement the reference count and free the old template if no queue is using it */
	    rv = _bcm_sbx_deallocate_template(unit, old_template);
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	}
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_params, (unit,
			      queue,
			      queue_region,
			      p_newqparams,
			      &old_qparams,
			      p_bwparams,   /* new bwparams same as old */
			      p_bwparams,   /* old bwparams */
			      &sizeInfo));
    if (rv != BCM_E_NONE) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "gport size set, error setting ingress params\n")));
    }

    BCM_SBX_UNLOCK(unit);
    return rv;

}

int
bcm_sbx_cosq_gport_size_get(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            uint32 *bytes_min,
                            uint32 *bytes_max)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
    int queue = -1;
    int num_cos;
    int modid = -1, my_modid = -1;
    int eg_n = 0;

    BCM_SBX_LOCK(unit);

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_SBX_CALADAN3(unit))
    {
        int    queue = cosq;
        rv = bcm_caladan3_cosq_src_queue_get(unit, queue, bytes_min, bytes_max);
        BCM_SBX_UNLOCK(unit);

        return rv;
    }
#endif

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if ((!bytes_min) || (!bytes_max)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error - required pointers not being passed\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* check if the operation is for Egress Port.                              */
    /* Functionality is currently implemented for QE2000 device. As additional */
    /* devices support this functionality this function will has to be         */
    /* accordingly enhanced (e.g. handle different "gport" types               */
    if (BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	BCM_GPORT_IS_EGRESS_MODPORT(gport) ||
	BCM_GPORT_IS_EGRESS_GROUP(gport)) {
        rv = bcm_sbx_stk_modid_get(unit, &my_modid);
	if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport size set, error getting modid for unit %d\n"),
                      unit));
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}

	if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
	    modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
	} else if  (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	    modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
	} else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	    modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
	    p_eg = SOC_SBX_STATE(unit)->egress_group_state;
	    eg_n = BCM_GPORT_EGRESS_GROUP_GET(gport);
	    if ((p_eg == NULL) || (eg_n < 0) || (eg_n >= SOC_SBX_CFG(unit)->num_egress_group)) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, egress group gport 0x%x not valid, unit %d\n"),
		           FUNCTION_NAME(), gport, unit));		
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	    
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "Unknown port derived from gport (0x%x)\n"),
	               gport));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

        if (modid != my_modid) {
            BCM_SBX_UNLOCK(unit);
            return(BCM_E_UNAVAIL);
        }

        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_egress_size_get, (unit,
                                  gport, cosq, bytes_min, bytes_max));
        if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport size get, error getting Egress Size\n")));
        }

        BCM_SBX_UNLOCK(unit);
        return(rv);
    } else if (gport == BCM_GPORT_INVALID) {
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_egress_size_get, (unit,
                                  gport, cosq, bytes_min, bytes_max));
        if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "gport size get, error getting Egress Size\n")));
        }

        BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    *bytes_min = -1;
    *bytes_max = -1;

    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);

    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    *bytes_min = queue_state[unit][queue].ingress.min_physical_queue_depth_bytes;
    *bytes_max = queue_state[unit][queue].ingress.max_physical_queue_depth_bytes;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "gport(0x%x) queue(%d) cosq(%d) min depth bytes(%d) max depth bytes(%d)\n"),
              gport, queue, cosq, *bytes_min, *bytes_max));

    BCM_SBX_UNLOCK(unit);
    return rv;

}

int
bcm_sbx_cosq_gport_bandwidth_set(int unit,
                                 bcm_gport_t gport,
                                 bcm_cos_queue_t cosq,
                                 uint32 kbits_sec_min,
                                 uint32 kbits_sec_max,
                                 uint32 flags)
{
    int rv = BCM_E_UNAVAIL;
    int queue = -1;
    int num_cos;
    int bw_group;
    int base_queue = -1;
    bcm_sbx_cosq_queue_params_ingress_t old_qparams, *p_qparams, *p_newqparams;
    bcm_sbx_cosq_bw_group_params_t old_bwparams, *p_bwparams, *p_newbwparams;
    int set_logical_port_shaper = FALSE;
    bcm_sbx_cosq_queue_region_type_t queue_region;
    int ingress = FALSE, egress = FALSE, scheduler = FALSE;
    int level=0, node=0, num_fifos = 0, egroup_num = -1, fabric_port = -1;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_MCAST(gport)) {
	ingress = TRUE;
	egress = FALSE;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport) || 
	       BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	       (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) ||
	       (BCM_COSQ_GPORT_IS_MULTIPATH(gport) && BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport))) {
	ingress = FALSE;
	egress = TRUE;
    } else if ((BCM_GPORT_IS_SCHEDULER(gport) && !BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) ||
	       (BCM_COSQ_GPORT_IS_MULTIPATH(gport) && !BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport))) {
	ingress = TRUE;
	egress = FALSE;
	scheduler = TRUE;
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ||
	     !SOC_IS_SIRIUS(unit)) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* sirius only, used to specify the interface port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress mod port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_MODPORT_SET(gport, BCM_GPORT_MODPORT_MODID_GET(gport), BCM_GPORT_MODPORT_PORT_GET(gport));
	}
    } else if (BCM_GPORT_IS_CHILD(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* used to specify the child port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress child port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_CHILD_SET(gport, BCM_GPORT_CHILD_MODID_GET(gport), BCM_GPORT_CHILD_PORT_GET(gport));
	}
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:unsupported gport type\n")));
        rv = BCM_E_PARAM;
	BCM_SBX_UNLOCK(unit);
        return rv;
    }

    if (ingress) {
	if (scheduler == TRUE) {
	    /* ingress scheduler */
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);

	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "%s: can not find ingress scheduler for gport 0x%x on unit %d\n"),
		          FUNCTION_NAME(), gport, unit));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	    
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_shaper_params, (unit,
                                      level, node, (int)kbits_sec_min, 0, (int)kbits_sec_max, 0));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "%s, error setting ingress params\n"),
		          FUNCTION_NAME()));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	} else {
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
	    
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	    
	    _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
	    
	    _bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport bandwidth set queue(%d) base(%d) cos(%d)\n"),
	              queue, base_queue, cosq));
	    
	    bw_group = queue_state[unit][base_queue].bw_group;
	    
	    if (flags & BCM_COSQ_ALL) {
		set_logical_port_shaper = TRUE;
	    }
	    
	    /* If max rate > 0, we need to shape */
	    if (kbits_sec_max > 0) {
		
		/* This is set up in qe2000/cosq.c since we need to know whether the QE is shaping
		 * on the logical port or on the queue based upon nQueuesPerShaperIngress
		 *
		 * queue_state[unit][queue].ingress.enable_shaping = TRUE;
		 * queue_state[unit][queue].ingress.shape_limit_kbps = kbits_sec_max;
		 */
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_shaper, (unit, base_queue, cosq, num_cos,
					  kbits_sec_max, set_logical_port_shaper,
                                          TRUE /* enable shaping */));
		
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport bandwidth set, error setting ingress shaper\n")));
		    BCM_SBX_UNLOCK(unit);
		    return rv;	    
		}
	    } else {
		
		/* This is set up in qe2000/cosq.c since we need to know whether the QE is shaping
		 * on the logical port or on the queue based upon nQueuesPerShaperIngress
		 *
		 * queue_state[unit][queue].ingress.enable_shaping = FALSE;
		 * queue_state[unit][queue].ingress.shape_limit_kbps = 0;
		 */
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_shaper, (unit, base_queue, cosq, num_cos,
					  0 /* max rate */, set_logical_port_shaper,
                                          FALSE /* disable shaping */));
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport bandwidth set, error setting ingress shaper\n")));
		    BCM_SBX_UNLOCK(unit);
		    return rv;	    
		}
	    }
	    
	    if (kbits_sec_min > 0) {
		
		/* If BCM_COSQ_ALL flag is set, set bag rate and update ingress params (only required on BM3200) */
		if (flags & BCM_COSQ_ALL) {
		    
		    /* Get old configuration */
		    old_bwparams = bw_group_state[unit][bw_group].path;
		    
		    /* Set new configuration */
		    bw_group_state[unit][bw_group].path.bag_rate_kbps = kbits_sec_min;
		    /* For now, set dest port rate to the bag rate until gport_set_port_speed() API is approved */
		    bw_group_state[unit][bw_group].dest_port_kbits_sec_max = kbits_sec_min;
		    
		    p_newbwparams = &bw_group_state[unit][bw_group].path;
		    
		    p_qparams = &queue_state[unit][queue].ingress;
		    
		    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_params, (unit,
					      queue,
					      queue_region,
					      p_qparams, /* new qparams same as old */
					      p_qparams, /* old qparams */
					      p_newbwparams,
					      &old_bwparams,
                                              NULL));
		    if (rv != BCM_E_NONE) {
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "gport bandwidth set, error setting ingress params\n")));
		    }
		}
	    }
	    
	    /* Allow guarrantee rate to be 0 */
	    if ( (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF)        ||

		 ((queue_region == bcm_sbx_cosq_queue_region_local) &&
		  ((queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF0) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF1) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF2) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF3) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF4) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF5) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF6)))   ||

		 ((queue_region == bcm_sbx_cosq_queue_region_local) &&
		  ((queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP0) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP1) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP2) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP3) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP4) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP5) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP6))) ) {
		/* FIC AF, or Local AF0/1/2/3, or Local RCPQ could have guarranted rate */
		
		old_qparams = queue_state[unit][queue].ingress;
		
		/* otherwise, BCM_COSQ_ALL is not set, if AF, set guarantee */
		queue_state[unit][queue].ingress.bw_value.guarantee_kbps = kbits_sec_min;
		
		p_newqparams = &queue_state[unit][queue].ingress;
		
		p_bwparams = &bw_group_state[unit][bw_group].path;
		
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_params, (unit,
					  queue,
					  queue_region,
					  p_newqparams,
					  &old_qparams,
					  p_bwparams,   /* new bwparams same as old */
					  p_bwparams,   /* old bwparams */
                                          NULL));
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport bandwidth set, error setting ingress params\n")));
		}
	    } else if ( (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_EF) &&
			(SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE) ) {
		/* if using polaris and EF, set guarantee */
		old_qparams = queue_state[unit][queue].ingress;
		
		/* otherwise, BCM_COSQ_ALL is not set, if EF, set guarantee */
		queue_state[unit][queue].ingress.bw_value.guarantee_kbps = kbits_sec_min;
		
		p_newqparams = &queue_state[unit][queue].ingress;
		
		p_bwparams = &bw_group_state[unit][bw_group].path;
		
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_params, (unit,
					  queue,
					  queue_region,
					  p_newqparams,
					  &old_qparams,
					  p_bwparams,   /* new bwparams same as old */
					  p_bwparams,   /* old bwparams */
                                          NULL));
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "gport bandwidth set, error setting ingress params\n")));
		}
	    }
	}
    }

    if (egress) {
	if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth set, unsupported gport on unit %d\n"),
		          unit));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }

	    if ( (level < 0) || (node < 0) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: unattached scheduler (0x%x), please attach first\n"),
		           gport));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;		
	    }

	    if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &egroup_num, &num_fifos) != BCM_E_NONE) { 
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d unknown\n"),
		           cosq));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;	
	    }

	    if ((flags & BCM_COSQ_ALL) || (cosq < 0) ) {
		/* point to the parent level */
		level++;
		node = egress_group_state[unit][BCM_GPORT_EGRESS_GROUP_GET(gport)].eg_scheduler;
		if (node == -1) {
		    /* egress group created by SDK directly without attaching to a scheduler */
		    node = BCM_GPORT_EGRESS_GROUP_GET(gport);
		}
	    } else if (cosq < num_fifos) {
		node += cosq;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
		           cosq, num_fifos - 1));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;	
	    }
	} else {
	    /* this will make sure we return group shaper for child/egress child gport if trunked */
	    level = SB_FAB_DEVICE_GROUP_SHAPER_LEVEL;
	    
	    /* for multicast internal port on egress, use cosq to differeniate group shaper or
	     * subport on individual higig interface. -1 is group shaper, 0-3 is higig0 to higig3
	     */
	    node = cosq;
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth set, unsupported gport on unit %d\n"),
		          unit));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	    
	    if ( (level != SB_FAB_DEVICE_GROUP_SHAPER_LEVEL) && 
		 ((level < 0) || (node < 0)) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: unattached scheduler (0x%x), please attach first\n"),
		           gport));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;		
	    }
	}

	if (level == SB_FAB_DEVICE_GROUP_SHAPER_LEVEL) {
	    /* in this case, the max shaper is using group shapers, and min shaper is using
	     * the shaper associated with the node
	     */

	    /* setup max shaper first */
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_shaper_params, (unit,
				      level, node, (int)SOC_SIRIUS_API_PARAM_NO_CHANGE,
				      SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                      (int)kbits_sec_max, 0));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth set, error setting egress max shaper params\n")));
	    }

	    /* call again to get the level/node id for min shaper */
	    level = 0;
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth set, unsupported gport on unit %d\n"),
		          unit));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	    
	    /* setup min shaper next */
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_shaper_params, (unit,
				      level, node, (int)kbits_sec_min, 0,
				      (int)SOC_SIRIUS_API_PARAM_NO_CHANGE,
                                      SOC_SIRIUS_API_PARAM_NO_CHANGE));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth set, error setting egress min shaper params\n")));
	    }

	} else {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_shaper_params, (unit,
                                      level, node, (int)kbits_sec_min, 0, (int)kbits_sec_max, 0));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth set, error setting egress params\n")));
	    }
	}
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_bandwidth_get(int unit,
                                 bcm_gport_t gport,
                                 bcm_cos_queue_t cosq,
                                 uint32 *kbits_sec_min,
                                 uint32 *kbits_sec_max,
                                 uint32 *flags)
{
    int base_queue = 0;
    int bw_group;
    int queue = 0;
    int num_cos;
    int ingress = FALSE, egress = FALSE, scheduler = FALSE;
    int level = 0, node = 0, num_fifos = 0, egroup_num = -1, fabric_port = -1;
    bcm_sbx_cosq_queue_region_type_t queue_region;
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if ((!kbits_sec_min)|| (!kbits_sec_max) || (!flags)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error - required pointers not being passed\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *kbits_sec_min = -1;
    *kbits_sec_max = -1;

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_MCAST(gport)) {
	ingress = TRUE;
	egress = FALSE;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport) || 
	       BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	       (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) ||
	       (BCM_COSQ_GPORT_IS_MULTIPATH(gport) && BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport))) {
	ingress = FALSE;
	egress = TRUE;
    } else if ((BCM_GPORT_IS_SCHEDULER(gport) && !BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) ||
	       (BCM_COSQ_GPORT_IS_MULTIPATH(gport) && !BCM_INT_SBX_MULTIPATH_IS_EGRESS(gport))) {
	ingress = TRUE;
	egress = FALSE;
	scheduler = TRUE;
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ||
	     !SOC_IS_SIRIUS(unit)) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* sirius only, used to specify the interface port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress mod port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_MODPORT_SET(gport, BCM_GPORT_MODPORT_MODID_GET(gport), BCM_GPORT_MODPORT_PORT_GET(gport));
	}
    } else if (BCM_GPORT_IS_CHILD(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* used to specify the child port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress child port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_CHILD_SET(gport, BCM_GPORT_CHILD_MODID_GET(gport), BCM_GPORT_CHILD_PORT_GET(gport));
	}
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:unsupported gport type\n")));
        rv = BCM_E_PARAM;
	BCM_SBX_UNLOCK(unit);
        return rv;
    }

    if (ingress) {
	if (scheduler == TRUE) {
	    /* ingress scheduler */
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);

	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "%s: can not find ingress scheduler for gport 0x%x on unit %d\n"),
		          FUNCTION_NAME(), gport, unit));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	    
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_ingress_shaper_params, (unit,
				      level, node, (int*)kbits_sec_min, NULL,
                                      (int *)kbits_sec_max, NULL));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "%s, error getting ingress params\n"),
		          FUNCTION_NAME()));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	} else {
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
	    
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	    
	    _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
	    
	    _bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);
	    
	    bw_group = queue_state[unit][base_queue].bw_group;
	    
	    *kbits_sec_max = queue_state[unit][queue].ingress.shape_limit_kbps;
	    
	    
	    if ( (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF)        ||

		 ((queue_region == bcm_sbx_cosq_queue_region_local) &&
		  ((queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF0) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF1) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF2) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF3) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF4) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF5) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_AF6)))   ||

		 ((queue_region == bcm_sbx_cosq_queue_region_local) &&
		  ((queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP0) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP1) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP2) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP3) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP4) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP5) ||
		   (queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_SP6)))   ||

		 ((queue_state[unit][queue].ingress.bw_mode == BCM_COSQ_EF) &&
		  (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE)) ) {
		/* AF, EF or local RCPQ could have guaranteed rate */
		*kbits_sec_min = queue_state[unit][queue].ingress.bw_value.guarantee_kbps;
		*flags = 0;
	    } else {		
		*kbits_sec_min = bw_group_state[unit][bw_group].path.bag_rate_kbps;
		*flags = BCM_COSQ_ALL;
	    }
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport(0x%x) base_queue(%d) cosq(%d) queue(%d) kbits_sec_min(%d) kbits_sec_max(%d)\n"),
	              gport, base_queue, cosq, queue, *kbits_sec_min, *kbits_sec_max));
	}
    }
	
    if (egress) {
	/* this will make sure we return group shaper for child/egress child gport if trunked */
	level = SB_FAB_DEVICE_GROUP_SHAPER_LEVEL;
	rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);

	if (rv != BCM_E_NONE) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport bandwidth get, unsupported gport on unit %d\n"),
	              unit));
	    rv = BCM_E_PARAM;
	    BCM_SBX_UNLOCK(unit);
	    return rv;	    
	}

	if ( (level != SB_FAB_DEVICE_GROUP_SHAPER_LEVEL) && 
	     ((level < 0) || (node < 0)) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
	               gport));
	    rv = BCM_E_PARAM;
	    BCM_SBX_UNLOCK(unit);
	    return rv;		
	}

	*flags = 0;
	if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
	    if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &egroup_num, &num_fifos) != BCM_E_NONE) { 
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d unknown\n"),
		           cosq));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;	
	    }
	    if (cosq < 0) {
		/* point to the parent level, node is in the handle */
		level++;
		node = egress_group_state[unit][BCM_GPORT_EGRESS_GROUP_GET(gport)].eg_scheduler;
		if (node == -1) {
		    /* egress group created by SDK directly without attaching to a scheduler */
		    node = BCM_GPORT_EGRESS_GROUP_GET(gport);
		}
		*flags |= BCM_COSQ_ALL;
	    } else if (cosq < num_fifos) {
		node += cosq;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
		           cosq, num_fifos - 1));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;	
	    }
	}

	if (level == SB_FAB_DEVICE_GROUP_SHAPER_LEVEL) {
	    /* in this case, the max shaper is using group shapers, and min shaper is using
	     * the shaper associated with the node
	     */

	    /* get max shaper first */
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_shaper_params, (unit,
                                      level, node, NULL, NULL, (int*)kbits_sec_max, NULL));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth get, error getting egress max shaper params\n")));
	    }

	    /* call again to get the level/node id for min shaper */
	    level = 0;
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth get, unsupported gport on unit %d\n"),
		          unit));
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }
	    
	    /* get min shaper next */
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_shaper_params, (unit,
                                      level, node, (int*)kbits_sec_min, NULL, NULL, NULL));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth get, error getting egress min shaper params\n")));
	    }
	} else {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_shaper_params, (unit,
				      level, node, (int*)kbits_sec_min, NULL, (int*)kbits_sec_max, NULL));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport bandwidth get, error getting egress params\n")));
	    }
	}
    }

    BCM_SBX_UNLOCK(unit);
    return rv;

}

int
bcm_sbx_cosq_gport_sched_set(int unit,
                             bcm_gport_t gport,
                             bcm_cos_queue_t cosq,
                             int mode,
                             int weight)
{
    int rv = BCM_E_UNAVAIL;
    int base_queue = 0;
    int queue = -1;
    bcm_sbx_cosq_queue_params_ingress_t old_qparams, *p_newqparams;
    bcm_sbx_cosq_bw_group_params_t *p_bwparams;
    int bw_group;
    int num_cos;
    bcm_sbx_cosq_queue_region_type_t queue_region;
    int ingress = FALSE, egress = FALSE, scheduler = FALSE;
    int level = 0, node = 0, num_fifos = 0, egroup_num = -1, fabric_port = -1;
    int is_multicast;
    int nbr_scheduler_nodes, nbr_fifos, cur_node;
    int scheduler_nodes[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];


    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_MCAST(gport)) {
	ingress = TRUE;
	egress = FALSE;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport) || 
	       BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	       (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport))) {
	ingress = FALSE;
	egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(gport) && !BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) {
	ingress = TRUE;
	egress = FALSE;
	scheduler = TRUE;
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ||
	     !SOC_IS_SIRIUS(unit)) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* sirius only, used to specify the interface port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress mod port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_MODPORT_SET(gport, BCM_GPORT_MODPORT_MODID_GET(gport), BCM_GPORT_MODPORT_PORT_GET(gport));
	}
    } else if (BCM_GPORT_IS_CHILD(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* used to specify the child port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress child port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_CHILD_SET(gport, BCM_GPORT_CHILD_MODID_GET(gport), BCM_GPORT_CHILD_PORT_GET(gport));
	}
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:unsupported gport type\n")));
        rv = BCM_E_PARAM;
	BCM_SBX_UNLOCK(unit);
        return rv;
    }

    if (ingress) {
	if (scheduler == TRUE) {
	    /* ingress schedulers */
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport sched set, unsupported gport on unit %d\n"),
		          unit));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }

	    if ( (level < 0) || (node < 0) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: unattached scheduler (0x%x), please attach first\n"),
		           gport));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;		
	    }

            switch (SOC_SBX_CFG(unit)->node_template_id) {
                case SOC_SBX_NODE_QOS_TEMPLATE_TYPE0:
	            switch (mode) {
		        case BCM_COSQ_SP0:
		        case BCM_COSQ_SP1:
		        case BCM_COSQ_AF0:
		        case BCM_COSQ_AF1:
		        case BCM_COSQ_AF2:
		        case BCM_COSQ_AF3:
		        case BCM_COSQ_BE:
		        case BCM_COSQ_NONE:
		            break;
		        default:
		            LOG_ERROR(BSL_LS_BCM_COMMON,
		                      (BSL_META_U(unit,
		                                  "ERROR: unsupported mode value for ingress schedulers(%d)\n"),
		                       mode));
		            rv = BCM_E_PARAM;
		            BCM_SBX_UNLOCK(unit);
		            return rv;
	            }
                    break;
                case SOC_SBX_NODE_QOS_TEMPLATE_TYPE1:
	            switch (mode) {
                        case BCM_COSQ_SP0:
                        case BCM_COSQ_SP1:
                        case BCM_COSQ_SP2:
                        case BCM_COSQ_SP3:
                        case BCM_COSQ_SP4:
                        case BCM_COSQ_SP5:
                        case BCM_COSQ_SP6:
                        case BCM_COSQ_AF0:
                        case BCM_COSQ_AF1:
                        case BCM_COSQ_AF2:
                        case BCM_COSQ_AF3:
                        case BCM_COSQ_AF4:
                        case BCM_COSQ_AF5:
                        case BCM_COSQ_AF6:
		        case BCM_COSQ_NONE:
		            break;
		        default:
		            LOG_ERROR(BSL_LS_BCM_COMMON,
		                      (BSL_META_U(unit,
		                                  "ERROR: unsupported mode value for ingress schedulers(%d)\n"),
		                       mode));
		            rv = BCM_E_PARAM;
		            BCM_SBX_UNLOCK(unit);
		            return(rv);
	            }
                    break;
                default:
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: unsupported local scheduling template value (%d)\n"),
                               SOC_SBX_CFG(unit)->local_template_id));
                    rv = BCM_E_PARAM;
                    BCM_SBX_UNLOCK(unit);
                    return(rv);
            }

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_scheduler_params, (unit,
				      level, node, mode, weight));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport sched set, error setting ingress params\n")));
	    }
	    
	} else {
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
	    
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	    
	    _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
	    
	    _bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);
	    
#if 0
	    /* Check priority  */
	    if (!BCM_COSQ_PRIO_VALID(cosq)) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR:invalid cosq value(%d) only 0-7 currently supported\n"),
		           (int)cosq));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
#endif /* 0 */
	    
	    if (cosq >= num_cos) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR:invalid cosq value(%d) outside of num_cos_levels(%d)\n"),
		           (int)cosq, num_cos));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	    
	    if ( (queue_region == bcm_sbx_cosq_queue_region_local) && SOC_IS_SIRIUS(unit) ) {
		switch (SOC_SBX_CFG(unit)->local_template_id) {
		    case SOC_SBX_QOS_TEMPLATE_TYPE0:
			switch (mode) {
			    case BCM_COSQ_SP0:
			    case BCM_COSQ_SP1:
			    case BCM_COSQ_SP2:
			    case BCM_COSQ_SP3:
			    case BCM_COSQ_EF:
			    case BCM_COSQ_AF0:
			    case BCM_COSQ_AF1:
			    case BCM_COSQ_AF2:
			    case BCM_COSQ_BE:
				LOG_INFO(BSL_LS_BCM_COSQ,
				         (BSL_META_U(unit,
				                     "mode(%d)\n"),
				          mode));
				break;
			    default:
				LOG_ERROR(BSL_LS_BCM_COMMON,
				          (BSL_META_U(unit,
				                      "ERROR: unsupported mode value (%d)\n"),
				           mode));
				rv = BCM_E_PARAM;
				BCM_SBX_UNLOCK(unit);
				return rv;
			}
			break;
		    case SOC_SBX_QOS_TEMPLATE_TYPE1:
			switch (mode) {
			    case BCM_COSQ_SP0:
			    case BCM_COSQ_SP1:
			    case BCM_COSQ_EF:
			    case BCM_COSQ_AF0:
			    case BCM_COSQ_AF1:
			    case BCM_COSQ_AF2:
			    case BCM_COSQ_AF3:
			    case BCM_COSQ_BE:
				LOG_INFO(BSL_LS_BCM_COSQ,
				         (BSL_META_U(unit,
				                     "mode(%d)\n"),
				          mode));
				break;
			    default:
				LOG_ERROR(BSL_LS_BCM_COMMON,
				          (BSL_META_U(unit,
				                      "ERROR: unsupported mode value (%d)\n"),
				           mode));
				rv = BCM_E_PARAM;
				BCM_SBX_UNLOCK(unit);
				return rv;
			}
			break;
		    case SOC_SBX_QOS_TEMPLATE_TYPE2:
		    case SOC_SBX_QOS_TEMPLATE_TYPE4:
			switch (mode) {
                            case BCM_COSQ_EF:
                            case BCM_COSQ_SP0:
                            case BCM_COSQ_SP1:
                            case BCM_COSQ_SP2:
                            case BCM_COSQ_SP3:
                            case BCM_COSQ_SP4:
                            case BCM_COSQ_SP5:
                            case BCM_COSQ_SP6:
                            case BCM_COSQ_AF0:
                            case BCM_COSQ_AF1:
                            case BCM_COSQ_AF2:
                            case BCM_COSQ_AF3:
                            case BCM_COSQ_AF4:
                            case BCM_COSQ_AF5:
                            case BCM_COSQ_AF6:
				LOG_INFO(BSL_LS_BCM_COSQ,
				         (BSL_META_U(unit,
				                     "mode(%d)\n"),
				          mode));
				break;
			    default:
				LOG_ERROR(BSL_LS_BCM_COMMON,
				          (BSL_META_U(unit,
				                      "ERROR: unsupported mode value (%d)\n"),
				           mode));
				rv = BCM_E_PARAM;
				BCM_SBX_UNLOCK(unit);
				return rv;
			}
			break;
		    case SOC_SBX_QOS_TEMPLATE_TYPE3:
			switch (mode) {
			    case BCM_COSQ_SP0:
			    case BCM_COSQ_SP1:
			    case BCM_COSQ_SP2:
			    case BCM_COSQ_SP3:
			    case BCM_COSQ_EF:
			    case BCM_COSQ_AF0:
			    case BCM_COSQ_AF1:
			    case BCM_COSQ_BE:
				LOG_INFO(BSL_LS_BCM_COSQ,
				         (BSL_META_U(unit,
				                     "mode(%d)\n"),
				          mode));
				break;
			    default:
				LOG_ERROR(BSL_LS_BCM_COMMON,
				          (BSL_META_U(unit,
				                      "ERROR: unsupported mode value (%d)\n"),
				           mode));
				rv = BCM_E_PARAM;
				BCM_SBX_UNLOCK(unit);
				return rv;
			}
			break;
		    default:
			LOG_ERROR(BSL_LS_BCM_COMMON,
			          (BSL_META_U(unit,
			                      "ERROR: unsupported local scheduling template value (%d)\n"),
			           SOC_SBX_CFG(unit)->local_template_id));
			rv = BCM_E_PARAM;
			BCM_SBX_UNLOCK(unit);
			return rv;
		}
	    } else {

	        if ( (queue_region == bcm_sbx_cosq_queue_region_local) ||
                         (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) ) {
		    switch (mode) {
		        case BCM_COSQ_SP:
		        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
		        case BCM_COSQ_EF:
		        case BCM_COSQ_AF:
		        case BCM_COSQ_SP_GLOBAL:
		        case BCM_COSQ_BE:
			    LOG_INFO(BSL_LS_BCM_COSQ,
			             (BSL_META_U(unit,
			                         "mode(%d)\n"),
			              mode));
			    break;
		        default:
			    LOG_ERROR(BSL_LS_BCM_COMMON,
			              (BSL_META_U(unit,
			                          "ERROR: unsupported mode value (%d)\n"),
			               mode));
			    rv = BCM_E_PARAM;
			    BCM_SBX_UNLOCK(unit);
			    return rv;
                    }
		}
                else {
		    switch (mode) {
		        case BCM_COSQ_SP:
		        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
		        case BCM_COSQ_EF:
		        case BCM_COSQ_AF:
		        case BCM_COSQ_SP_GLOBAL:
		        case BCM_COSQ_BE:
		        case BCM_COSQ_GSP0:
		        case BCM_COSQ_GSP1:
		        case BCM_COSQ_GSP2:
		        case BCM_COSQ_GSP3:
		        case BCM_COSQ_GSP4:
		        case BCM_COSQ_GSP5:
		        case BCM_COSQ_GSP6:
		        case BCM_COSQ_GSP7:
			    LOG_INFO(BSL_LS_BCM_COSQ,
			             (BSL_META_U(unit,
			                         "mode(%d)\n"),
			              mode));
			    break;
		        default:
			    LOG_ERROR(BSL_LS_BCM_COMMON,
			              (BSL_META_U(unit,
			                          "ERROR: unsupported mode value (%d)\n"),
			               mode));
			    rv = BCM_E_PARAM;
			    BCM_SBX_UNLOCK(unit);
			    return rv;
                    }
                }
	    }
	
	
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport sched set queue(%d) base(%d) cos(%d)\n"),
	              queue, base_queue, cosq));
	    
	    /* Get old configuration */
	    old_qparams = queue_state[unit][queue].ingress;
	    
	    /* Set new configuration */
	    queue_state[unit][queue].ingress.bw_mode = mode;
	    queue_state[unit][queue].ingress.bw_value.wfq_weight = weight;
	    /* GNATS 24617 Weight has a double meaning.  If SP_GLOBAL, weight is also the priority */
	    queue_state[unit][queue].ingress.bw_value.sp_priority = weight;
	    p_newqparams = &queue_state[unit][queue].ingress;
	    
	    bw_group = queue_state[unit][queue].bw_group;
	    p_bwparams = &bw_group_state[unit][bw_group].path;
	    
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_ingress_params, (unit,
				      queue,
				      queue_region,
				      p_newqparams,
				      &old_qparams,
				      p_bwparams,   /* new bwparams same as old */
				      p_bwparams,   /* old bwparams */
				      NULL));
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "gport sched set, error setting ingress params\n")));
	    }
	}
    }

    if (egress) {

	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_is_egress_multicast, (unit, gport, cosq, &is_multicast));
	if (rv != BCM_E_NONE) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport sched set, error in determinhing egress external/internl ports, unit %d\n"),
	              unit));
	    BCM_SBX_UNLOCK(unit);
	    return(rv);	    
	}

        if (is_multicast == FALSE) {
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);

	    if (rv != BCM_E_NONE) {
	        LOG_INFO(BSL_LS_BCM_COSQ,
	                 (BSL_META_U(unit,
	                             "gport sched set, unsupported gport on unit %d\n"),
	                  unit));
	        rv = BCM_E_PARAM;
	        BCM_SBX_UNLOCK(unit);
	        return rv;	    
	    }

	    if ( (level < 0) || (node < 0) ) {
	        LOG_ERROR(BSL_LS_BCM_COMMON,
	                  (BSL_META_U(unit,
	                              "ERROR: unattached scheduler (0x%x), please attach first\n"),
	                   gport));
	        rv = BCM_E_PARAM;
	        BCM_SBX_UNLOCK(unit);
	        return rv;		
	    }

	    if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
	        if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &egroup_num, &num_fifos) != BCM_E_NONE) { 
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: fifo scheduler cos %d unknown\n"),
		               cosq));
		    rv = BCM_E_PARAM;
		    BCM_SBX_UNLOCK(unit);
		    return rv;	
	        }
	        if (cosq < 0) {
		    /* point to the parent level, node is in the handle */
		    level++;
		    node = egress_group_state[unit][BCM_GPORT_EGRESS_GROUP_GET(gport)].eg_scheduler;
		    if (node == -1) {
		        /* egress group created by SDK directly without attaching to a scheduler */
		        node = BCM_GPORT_EGRESS_GROUP_GET(gport);
		    }
	        } else if (cosq < num_fifos ) {
		    node += cosq;
	        } else {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
		               cosq, num_fifos - 1));
		    rv = BCM_E_PARAM;
		    BCM_SBX_UNLOCK(unit);
		    return rv;	
	        }
	    }

	    switch (mode) {
	        case BCM_COSQ_SP:
	        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                case BCM_COSQ_EF:
                    break;
	        default:
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: unsupported mode value for egress schedulers(%d)\n"),
		               mode));
		    rv = BCM_E_PARAM;
		    BCM_SBX_UNLOCK(unit);
		    return rv;
	    }	

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_scheduler_params, (unit,
				  level, node, mode, weight));

	    if (rv != BCM_E_NONE) {
	        LOG_INFO(BSL_LS_BCM_COSQ,
	                 (BSL_META_U(unit,
	                             "gport sched set, error setting egress params\n")));
	    }
        }
        else {
	    switch (mode) {
	        case BCM_COSQ_SP:
	        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                case BCM_COSQ_EF:
                    break;
	        default:
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: unsupported mode value for egress schedulers(%d)\n"),
		               mode));
		    rv = BCM_E_PARAM;
		    BCM_SBX_UNLOCK(unit);
		    return rv;
	    }	

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_egress_multicast_scheduler_get, (unit, gport,
                                    cosq, &level, &nbr_scheduler_nodes, scheduler_nodes));
	    if (rv != BCM_E_NONE) {
	        LOG_INFO(BSL_LS_BCM_COSQ,
	                 (BSL_META_U(unit,
	                             "gport sched set, error in determinhing egress port configuration, unit %d\n"),
	                  unit));
	        BCM_SBX_UNLOCK(unit);
	        return(rv);	    
	    }

	    if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {

	        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_egress_multicast_group_get, (unit, gport, cosq,
                                           &nbr_fifos, &nbr_scheduler_nodes, scheduler_nodes));
	        if (rv != BCM_E_NONE) {
	            LOG_INFO(BSL_LS_BCM_COSQ,
	                     (BSL_META_U(unit,
	                                 "gport sched set, error in determinhing egress group configuration, unit %d\n"),
	                      unit));
	            BCM_SBX_UNLOCK(unit);
	            return(rv);	    
	        }
            }

            for (cur_node = 0; cur_node < nbr_scheduler_nodes; cur_node++) {
	        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_egress_scheduler_params, (unit,
				  level, (scheduler_nodes[cur_node] + cosq), mode, weight));
	        if (rv != BCM_E_NONE) {
	            LOG_INFO(BSL_LS_BCM_COSQ,
	                     (BSL_META_U(unit,
	                                 "gport sched set, error setting egress multicast params\n")));
	        }
            }
        }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_sched_get(int unit,
                             bcm_gport_t gport,
                             bcm_cos_t cosq,
                             int *mode,
                             int *weight)
{
    int base_queue = 0;
    int queue = 0;
    int num_cos;
    int rv = BCM_E_NONE;
    int ingress = FALSE, egress = FALSE, scheduler = FALSE;
    int level = 0, node = 0, num_fifos = 0, egroup_num = -1, fabric_port = -1;
    int is_multicast;
    int nbr_scheduler_nodes, nbr_fifos;
    int scheduler_nodes[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if ((!mode) || (!weight)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error - required pointers not being passed\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *mode = -1;
    *weight = -1;

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_MCAST(gport)) {
	ingress = TRUE;
	egress = FALSE;
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport) || 
	       BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_GROUP(gport) ||
	       (BCM_GPORT_IS_SCHEDULER(gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport))) {
	ingress = FALSE;
	egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(gport) && !BCM_INT_SBX_SCHEDULER_IS_EGRESS(gport)) {
	ingress = TRUE;
	egress = FALSE;
	scheduler = TRUE;
    } else if (BCM_GPORT_IS_MODPORT(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ||
	     !SOC_IS_SIRIUS(unit)) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* sirius only, used to specify the interface port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress mod port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_MODPORT_SET(gport, BCM_GPORT_MODPORT_MODID_GET(gport), BCM_GPORT_MODPORT_PORT_GET(gport));
	}
    } else if (BCM_GPORT_IS_CHILD(gport)) {
	if ( soc_feature(unit, soc_feature_standalone) || 
	     soc_feature(unit, soc_feature_hybrid) ) {
	    /* used to specify the ingress scheduler associated with the child port */
	    ingress = TRUE;
	    egress = FALSE;
	    if (SOC_IS_SIRIUS(unit)) {
		/* interface level scheduler */
		scheduler = TRUE;
	    }
	} else {
	    /* used to specify the child port rate */
	    ingress = FALSE;
	    egress = TRUE;
	    /* convert it to egress child port so that it will resolve to egress scheduler */
	    BCM_GPORT_EGRESS_CHILD_SET(gport, BCM_GPORT_CHILD_MODID_GET(gport), BCM_GPORT_CHILD_PORT_GET(gport));
	}
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:unsupported gport type\n")));
        rv = BCM_E_PARAM;
	BCM_SBX_UNLOCK(unit);
        return rv;
    }

    if (ingress) {
	if (scheduler == TRUE) {
	    /* ingress schedulers */
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "%s, unsupported gport on unit %d\n"),
		          FUNCTION_NAME(), unit));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;	    
	    }

	    if ( (level < 0) || (node < 0) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, unattached scheduler (0x%x), please attach first\n"),
		           FUNCTION_NAME(), gport));
		rv = BCM_E_PARAM;
		BCM_SBX_UNLOCK(unit);
		return rv;		
	    }

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_ingress_scheduler_params, (unit,
				      level, node, mode, weight));
	    
	    if (rv != BCM_E_NONE) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "%s, error setting ingress params\n"),
		          FUNCTION_NAME()));
	    }
	} else {
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
	    
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	    
	    _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
	    
	    
	    *mode = queue_state[unit][queue].ingress.bw_mode;
	    *weight = queue_state[unit][queue].ingress.bw_value.wfq_weight;
	    
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport(0x%x) base_queue(%d) cosq(%d) queue(%d) mode(%d) weight(%d)\n"),
	              gport, base_queue, cosq, queue, *mode, *weight));
	}
    }

    if (egress) {

	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_is_egress_multicast, (unit, gport, cosq, &is_multicast));
	if (rv != BCM_E_NONE) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "gport sched set, error in determinhing egress external/internl ports, unit %d\n"),
	              unit));
	    BCM_SBX_UNLOCK(unit);
	    return(rv);	    
	}

        if (is_multicast == FALSE) {
	    rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);

	    if (rv != BCM_E_NONE) {
	        LOG_INFO(BSL_LS_BCM_COSQ,
	                 (BSL_META_U(unit,
	                             "gport sched set, unsupported gport on unit %d\n"),
	                  unit));
	        rv = BCM_E_PARAM;
	        BCM_SBX_UNLOCK(unit);
	        return rv;	    
	    }

	    if ( (level < 0) || (node < 0) ) {
	        LOG_ERROR(BSL_LS_BCM_COMMON,
	                  (BSL_META_U(unit,
	                              "ERROR: unattached scheduler (0x%x), please attach first\n"),
	                   gport));
	        rv = BCM_E_PARAM;
	        BCM_SBX_UNLOCK(unit);
	        return rv;		
	    }

	    if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {
	        if (bcm_sbx_cosq_egress_group_info_get(unit, gport, &fabric_port, &egroup_num, &num_fifos) != BCM_E_NONE) { 
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: fifo scheduler cos %d unknown\n"),
		               cosq));
		    rv = BCM_E_PARAM;
		    BCM_SBX_UNLOCK(unit);
		    return rv;	
	        }
	        if (cosq < 0) {
		    /* point to the parent level, node is in the handle */
		    level++;
		    node = egress_group_state[unit][BCM_GPORT_EGRESS_GROUP_GET(gport)].eg_scheduler;
		    if (node == -1) {
		    /* egress group created by SDK directly without attaching to a scheduler */
		        node = BCM_GPORT_EGRESS_GROUP_GET(gport);
		    }
	        } else if ( cosq < num_fifos ) {
		    node += cosq;
	        } else {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
		               cosq, num_fifos - 1));
		    rv = BCM_E_PARAM;
		    BCM_SBX_UNLOCK(unit);
		    return rv;	
	        }
	    }

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_scheduler_params, (unit,
				  level, node, mode, weight));

	    if (rv != BCM_E_NONE) {
	        LOG_INFO(BSL_LS_BCM_COSQ,
	                 (BSL_META_U(unit,
	                             "gport sched get, error setting egress params\n")));
	    }
        }
        else {
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_egress_multicast_scheduler_get, (unit, gport,
                                    cosq, &level, &nbr_scheduler_nodes, scheduler_nodes));
            if (rv != BCM_E_NONE) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "gport sched set, error in determinhing egress port configuration, unit %d\n"),
                          unit));
                BCM_SBX_UNLOCK(unit);
                return(rv);
            }
    
            if ( BCM_GPORT_IS_EGRESS_GROUP(gport) ) {

                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_port_egress_multicast_group_get, (unit, gport, cosq,
                                           &nbr_fifos, &nbr_scheduler_nodes, scheduler_nodes));
                if (rv != BCM_E_NONE) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "gport sched set, error in determinhing egress group configuration, unit %d\n"),
                              unit));
                    BCM_SBX_UNLOCK(unit);
                    return(rv);
                }
            }

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_egress_scheduler_params, (unit,
				  level, scheduler_nodes[0], mode, weight));

	    if (rv != BCM_E_NONE) {
	        LOG_INFO(BSL_LS_BCM_COSQ,
	                 (BSL_META_U(unit,
	                             "gport sched get, error setting egress params\n")));
	    }
        }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;

}

int
bcm_sbx_cosq_gport_discard_set(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_NONE;
    int num_cos;
    int queue;
    uint32 color;
    int color_val;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    /* Check if for the system configuration this device support WRED */
    /* functionality.                                                 */
    if (!(soc_feature(unit, soc_feature_discard_ability))) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Discard functionality not supported,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* If required allocate memory for WRED parameters */
    if (discard_state[unit] == NULL) {
        rv = _bcm_sbx_cosq_discard_state_init(unit);
        if (rv != BCM_E_NONE) {
            BCM_SBX_UNLOCK(unit);
            return(rv);
        }
    }

    /* Consistency Checks */
    if (discard == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Discard parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    /* fill in the color value */
    color = discard->flags & BCM_COSQ_DISCARD_COLOR_ALL;

    color_val = (color & BCM_COSQ_DISCARD_COLOR_ALL);
    if ( (color_val != BCM_COSQ_DISCARD_COLOR_GREEN) && (color_val != BCM_COSQ_DISCARD_COLOR_YELLOW) &&
	 (color_val != BCM_COSQ_DISCARD_COLOR_RED) && (color_val != BCM_COSQ_DISCARD_COLOR_BLACK) &&
	 (color_val != BCM_COSQ_DISCARD_COLOR_ALL) && !(color & BCM_COSQ_DISCARD_COLOR_ALL) &&
	 ((color & ~BCM_COSQ_DISCARD_COLOR_ALL) != 0) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Color Parameter, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if ( (color_val == BCM_COSQ_DISCARD_COLOR_BLACK) &&
	 !soc_feature(unit, soc_feature_discard_ability_color_black)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Color black not support on this device, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if ( (discard->drop_probability < BCM_SBX_COSQ_MIN_DROP_PROBABILITY) ||
	 (discard->drop_probability > BCM_SBX_COSQ_MAX_DROP_PROBABILITY) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Drop Probability, Unit(%d), DP: %d\n"),
                   FUNCTION_NAME(), unit, discard->drop_probability));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    /* update discard drop probability */
    if (!(discard->flags & BCM_COSQ_DISCARD_ENABLE)) {
        discard->drop_probability = 0;
    }

    /* NOTE: The following flags are ignored */
    /*       - BCM_COSQ_DISCARD_CAP_AVERAGE  */
    /*       - BCM_COSQ_DISCARD_BYTES        */
    /*       - BCM_COSQ_DISCARD_PACKETS      */

    if (discard->drop_probability != 0) {
        if ( (discard->gain != -1) && ((discard->gain < BCM_SBX_COSQ_MIN_GAIN) ||
				       (discard->gain > BCM_SBX_COSQ_MAX_GAIN)) ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Invalid Gain Parameter, Unit(%d)\n"),
                       FUNCTION_NAME(), unit));
            BCM_SBX_UNLOCK(unit);
            return(BCM_E_PARAM);
        }
    }

    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
    if (rv != BCM_E_NONE) { /* VOQ is not valid */
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid VOQ, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    /* Disable WRED */
    if (discard->drop_probability == 0) {
        rv = _bcm_sbx_cosq_gport_discard_disable(unit, gport, cosq, queue, color, discard);
    }

    /* Enable WRED */
    else {
        rv = _bcm_sbx_cosq_gport_discard_enable(unit, gport, cosq, queue, color, discard);
    }

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_cosq_gport_discard_get(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_NONE;
    int num_cos;
    int queue, template, gain;
    int color_val, start_color[BCM_SBX_COSQ_DISCARD_MAX_COLORS], nbr_color;
    uint32 color;

    BCM_SBX_LOCK(unit);

    /* Check if for the system configuration this device support WRED */
    /* functionality.                                                 */
    if (!(soc_feature(unit, soc_feature_discard_ability))) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Discard functionality not supported,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (discard_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Discard Data Structures Not allocated,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_INTERNAL);
    }

    /* Consistency Checks */
    if (discard == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Discard parameter is NULL, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    /* fill in the color value */
    color = discard->flags & BCM_COSQ_DISCARD_COLOR_ALL;

    color_val = (color & BCM_COSQ_DISCARD_COLOR_ALL);
    if ( (color_val != BCM_COSQ_DISCARD_COLOR_GREEN) && (color_val != BCM_COSQ_DISCARD_COLOR_YELLOW) &&
	 (color_val != BCM_COSQ_DISCARD_COLOR_RED) && (color_val != BCM_COSQ_DISCARD_COLOR_BLACK) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Color Parameter, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    if ( (color_val == BCM_COSQ_DISCARD_COLOR_BLACK) &&
	 !soc_feature(unit, soc_feature_discard_ability_color_black)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Color black not support on this device, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
    if (rv != BCM_E_NONE) { /* VOQ is not valid */
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid VOQ, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_PARAM);
    }

    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, color_val, start_color, &nbr_color);
    bcm_sbx_cosq_queue_get_template_gain(unit, queue, &template, &gain);

    (*discard) = (discard_state[unit] + template)->config[*(start_color + 0)];
    discard->gain = gain;

    BCM_SBX_UNLOCK(unit);
    return(rv);
}

int
bcm_sbx_cosq_gport_enable_set(int unit,
                              bcm_gport_t gport,
                              bcm_cos_queue_t cosq,
                              int enable)
{
    bcm_sbx_cosq_queue_region_type_t queue_region = bcm_sbx_cosq_queue_region_global;
    int base_queue = 0;
    int queue = 0;
    int num_cos, idx = 0;
    int rv = BCM_E_NONE;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {

	if (cosq >= 0) {
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);
	} else {
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, 0, &num_cos);
	}
            
	if (rv != BCM_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}

	_bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
	_bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);

	/*
	 * If cosq is valid, then loop only once.
	 * Otherwise, set cosq to 0 and loop num_cos
	 */

	if (cosq >= 0) {
	    num_cos = 1;
	} else {
	    cosq = 0;
	}

	for (idx = 0; idx < num_cos; idx++, cosq++) {
	    queue = base_queue + cosq;
	    
	    if (enable) {
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_enable_queue, (unit, queue, queue_region));
		queue_state[unit][queue].ingress.enabled = TRUE;
	    }
	    else {
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_disable_queue, (unit, queue, queue_region));
		queue_state[unit][queue].ingress.enabled = FALSE;
	    }
	}

    } else if (BCM_GPORT_IS_MODPORT(gport) || 
	       BCM_GPORT_IS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_MODPORT(gport) || 
	       BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	
	if (cosq < 0) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Invalid cos value %d, Unit(%d)\n"),
	               FUNCTION_NAME(), cosq, unit));
	    rv = BCM_E_PARAM;
	    BCM_SBX_UNLOCK(unit);
	    return rv;	
	}

	if (enable) {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_enable_fifo, (unit, gport, cosq, queue_region));
	}
	else {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_disable_fifo, (unit, gport, cosq, queue_region));
	}
    } else {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Unsupported gport type 0x%x\n"),
	           gport));
	rv = BCM_E_PARAM;
    }


    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_enable_get(int unit,
                              bcm_gport_t gport,
                              bcm_cos_queue_t cosq,
                              int *enable)
{
    int rv = BCM_E_NONE;
    int queue = -1;
    int num_cos;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    if ((!enable)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error - required pointers not being passed\n")));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (cosq < 0) {
	*enable = -1;
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid cos value %d, Unit(%d)\n"),
                   FUNCTION_NAME(), unit, cosq));
        BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *enable = 0;

    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
	BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
        BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {

	rv = _bcm_sbx_cosq_get_queue_from_gport(unit, gport, &queue, cosq, &num_cos);

	if (rv != BCM_E_NONE) {
	    BCM_SBX_UNLOCK(unit);
	    return rv;
	}
	
	*enable = queue_state[unit][queue].ingress.enabled;
	
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "gport(0x%x) queue(%d) cosq(%d) (%s)\n"),
	          gport, queue, cosq, enable? "ENABLED":"DISABLED"));

    } else if (BCM_GPORT_IS_MODPORT(gport) ||
	       BCM_GPORT_IS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_CHILD(gport) ||
	       BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	
	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_enable_get, (unit, gport, cosq, enable));

	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "gport(0x%x) fifo(%d) (%s)\n"),
	          gport, cosq, (*enable) ? "ENABLED":"DISABLED"));
    } else {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Unsupported gport type 0x%x\n"),
	           gport));
	rv = BCM_E_PARAM;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_attach(int unit,
                          bcm_gport_t sched_gport,
                          bcm_gport_t input_gport,
                          bcm_cos_queue_t cosq)

{
    int rv = BCM_E_UNAVAIL;
    int child_egress, parent_egress;

    BCM_SBX_LOCK(unit);

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* decode gports and make sure both are on same resource (ingress/egress) */
    if ( (BCM_GPORT_IS_UCAST_QUEUE_GROUP(input_gport)) ||
         (BCM_GPORT_IS_MCAST_QUEUE_GROUP(input_gport)) ||
	 (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(input_gport)) ||
	 (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(input_gport)) ||
         (BCM_GPORT_IS_CHILD(input_gport)) ) {
        child_egress = FALSE;
    } else if ( (BCM_GPORT_IS_EGRESS_GROUP(input_gport)) ||
                (BCM_GPORT_IS_EGRESS_CHILD(input_gport)) ) {
        child_egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(input_gport)) {
        if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(input_gport)) {
            child_egress = TRUE;
        } else {
            child_egress = FALSE;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported child gport type\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if ( (BCM_GPORT_IS_CHILD(sched_gport)) ||
         (BCM_GPORT_IS_MODPORT(sched_gport)) ) {
        parent_egress = FALSE;
    } else if ( (BCM_GPORT_IS_EGRESS_CHILD(sched_gport)) ||
                (BCM_GPORT_IS_EGRESS_MODPORT(sched_gport)) ) {
        parent_egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(sched_gport)) {
        if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(sched_gport)) {
            parent_egress = TRUE;
        } else {
            parent_egress = FALSE;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported parent gport type\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if ( child_egress != parent_egress ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Child and parent gport direction not match\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* call MCM layer */
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_attach_scheduler, (unit, sched_gport, input_gport, cosq, child_egress));

    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s attach gport(0x%x) to (0x%x) failed error(%d)\n"), 
                   FUNCTION_NAME(), input_gport, sched_gport, rv));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_detach(int unit,
                          bcm_gport_t sched_gport,
                          bcm_gport_t input_gport,
                          bcm_cos_queue_t cosq)
{
    int rv = BCM_E_UNAVAIL;
    int child_egress, parent_egress;

    BCM_SBX_LOCK(unit);

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* decode gports and make sure both are on same resource (ingress/egress) */
    if ( (BCM_GPORT_IS_UCAST_QUEUE_GROUP(input_gport)) ||
         (BCM_GPORT_IS_MCAST_QUEUE_GROUP(input_gport)) ||
	 (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(input_gport)) ||
	 (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(input_gport)) ||
         (BCM_GPORT_IS_CHILD(input_gport)) ) {
        child_egress = FALSE;
    } else if ( (BCM_GPORT_IS_EGRESS_GROUP(input_gport)) ||
                (BCM_GPORT_IS_EGRESS_CHILD(input_gport)) ) {
        child_egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(input_gport)) {
        if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(input_gport)) {
            child_egress = TRUE;
        } else {
            child_egress = FALSE;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported child gport type\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if ( (BCM_GPORT_IS_CHILD(sched_gport)) ||
         (BCM_GPORT_IS_MODPORT(sched_gport)) ) {
        parent_egress = FALSE;
    } else if ( (BCM_GPORT_IS_EGRESS_CHILD(sched_gport)) ||
                (BCM_GPORT_IS_EGRESS_MODPORT(sched_gport)) ) {
        parent_egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(sched_gport)) {
        if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(sched_gport)) {
            parent_egress = TRUE;
        } else {
            parent_egress = FALSE;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported parent gport type\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if ( child_egress != parent_egress ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Child and parent gport direction not match\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /* call MCM layer */
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_detach_scheduler, (unit, sched_gport, input_gport, cosq, child_egress));

    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s detach gport(0x%x) to (0x%x) failed error(%d)\n"), 
                   FUNCTION_NAME(), input_gport, sched_gport, rv));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * The API is used to find parents, children of a gport on ingress or egress hierachy
 * cosq = -1 means find parent, other cosq value means find the nth (n=cosq) children
 * if the number of children is less than "cosq", then return BCM_E_NOT_FOUND
 * and BCM_GPORT_INVALID
 */
int
bcm_sbx_cosq_gport_attach_get(int unit,
                              bcm_gport_t sched_gport,
                              bcm_gport_t *input_gport,
                              bcm_cos_queue_t *cosq)
{
    int rv = BCM_E_UNAVAIL;
    int egress;

    BCM_SBX_LOCK(unit);

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
	BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
    }

    /* ingress or egress hierachy */
    if ( (BCM_GPORT_IS_UCAST_QUEUE_GROUP(sched_gport)) ||
         (BCM_GPORT_IS_MCAST_QUEUE_GROUP(sched_gport)) ||
	 (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(sched_gport)) ||
	 (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(sched_gport)) ||
	 (BCM_GPORT_IS_CHILD(sched_gport)) ||
         (BCM_GPORT_IS_MODPORT(sched_gport)) ) {
        egress = FALSE;
    } else if ( (BCM_GPORT_IS_EGRESS_CHILD(sched_gport)) ||
		(BCM_GPORT_IS_EGRESS_GROUP(sched_gport)) ||
                (BCM_GPORT_IS_EGRESS_MODPORT(sched_gport)) ) {
        egress = TRUE;
    } else if (BCM_GPORT_IS_SCHEDULER(sched_gport)) {
        if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(sched_gport)) {
            egress = TRUE;
        } else {
            egress = FALSE;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported parent gport type\n")));
	BCM_SBX_UNLOCK(unit);
        return BCM_E_PARAM;
    }
    

    /* call MCM layer */
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_attach_get, (unit,
			      sched_gport, egress, cosq, input_gport));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

/*
 * Returns     +-------------+-----------------+
 * Attach ID : |   FC Domain |      Sysport    |
 *             +----10bits---+-------12bits----+
 *            22            12                 0
 */

int 
bcm_sbx_cosq_gport_queue_attach(
    int unit, 
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
    bcm_sbx_subport_info_t *sp_info = NULL;
    int32 requested_sysport = BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT;
    int32 requested_fcd = BCM_INT_SBX_REQUESTED_FCD_AUTO_SELECT;
    bcm_sbx_cosq_queue_region_type_t queue_region = bcm_sbx_cosq_queue_region_global;
    int dest_type = BCM_INT_SBX_DEST_TYPE_UNICAST;
    sbBool_t with_id = (flags & BCM_COSQ_GPORT_QUEUE_ATTACH_WITH_ID);
    uint16 base_queue = 0, queue = 0;
    int offset = 0, fcd_max = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3;
    int node = 0, port = 0, fcd = fcd_max, sysport = 0, modid = 0;
    int subport = 0, eg_n = 0, num_fifos = 0, fifo = 0, mymodid = -1;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if(BCM_GPORT_IS_UCAST_QUEUE_GROUP(ingress_queue)) {
        base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(ingress_queue)) {
        base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(ingress_queue);
    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(ingress_queue);
        queue_region = bcm_sbx_cosq_queue_region_local;
    } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(ingress_queue)) {
	base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(ingress_queue);
        queue_region = bcm_sbx_cosq_queue_region_local;
    } else {
	BCM_SBX_UNLOCK(unit);
	return(BCM_E_PARAM);
    }

    queue = base_queue + ingress_int_pri;
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];

    if (p_bwstate->base_queue != base_queue) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_queue 0x%x is not a base queue, unit %d\n"),
	           FUNCTION_NAME(), ingress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    node = p_bwstate->dest_node;
    port = p_bwstate->dest_port;

    if (ingress_int_pri >= p_bwstate->num_cos) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_int_pri %d exceeds %d cos for ingress_queue 0x%x, unit %d\n"),
	           FUNCTION_NAME(), ingress_int_pri, p_bwstate->num_cos, ingress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if ((p_bwstate->dest_mc == 0) && 
	((egress_int_pri == BCM_COS_MULTICAST_EF) || 
	 (egress_int_pri == BCM_COS_MULTICAST_NON_EF))) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, egress_int_pri %d indicates multicast, but egress_queue %d not multicast queue, unit %d\n"),
	           FUNCTION_NAME(), egress_int_pri, egress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        if (egress_queue != (bcm_gport_t)NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, multicast egress_queue is not NULL 0x%x\n"),
	               FUNCTION_NAME(), egress_queue));
	    BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }
        if ((egress_int_pri != BCM_COS_MULTICAST_EF) && (egress_int_pri != BCM_COS_MULTICAST_NON_EF)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, multicast egress_int_pri invalid 0x%x\n"),
	               FUNCTION_NAME(), egress_int_pri));
	    BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }

        /* call MCM layer */
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_queue_attach, (unit, flags, ingress_queue, ingress_int_pri, egress_queue, egress_int_pri, attach_id));

        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s ingress queue/cos (0x%x/%d) to egress cos (0x%x) failed error(%d)\n"),
                       FUNCTION_NAME(), ingress_queue, ingress_int_pri, egress_int_pri, rv));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        p_qstate->attached_fifo = egress_int_pri;

        BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    if (BCM_GPORT_IS_EGRESS_MODPORT(egress_queue)) {
	modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(egress_queue);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(egress_queue)) {
	modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(egress_queue);
    } else if BCM_GPORT_IS_EGRESS_GROUP(egress_queue) {
	modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(egress_queue);
    } else {
	BCM_SBX_UNLOCK(unit);
	return(BCM_E_PARAM);
    }
    
    if (attach_id == NULL) {
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    rv = bcm_stk_modid_get(unit, &mymodid); 
    if ((rv != BCM_E_NONE) && (rv != BCM_E_UNAVAIL)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, cannot retrieve system modid on unit %d\n"),
	           FUNCTION_NAME(), unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_INTERNAL;
    }

    if (modid != mymodid) {

	if (with_id != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s %s node must use BCM_COSQ_GPORT_QUEUE_ATTACH_WITH_ID flag\n"),
	               FUNCTION_NAME(), ((mymodid == -1) ? "BME" : "Ingress")));
	    
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
	
	if (p_qstate->sysport != p_qstate->default_sysport) {
	    ATTACH_ID_SET(*attach_id, sysport_state[unit][p_qstate->sysport].fcd, p_qstate->sysport);
	    LOG_WARN(BSL_LS_BCM_COMMON,
	             (BSL_META_U(unit,
	                         "WARNING: %s, Ingress Queue 0x%x cos %d is currently connected to egress queue 0x%x attach_id 0x%x, unit %d\n"),
	              FUNCTION_NAME(), ingress_queue, ingress_int_pri, 
	              sysport_state[unit][p_qstate->sysport].egport, *attach_id, unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}

	if (BCM_GPORT_IS_EGRESS_CHILD(egress_queue)) {
	    if (BCM_GPORT_EGRESS_CHILD_PORT_GET(egress_queue) != port) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s egress queue 0x%x not connected to ingress queue 0x%x, port mismatch\n"),
		           FUNCTION_NAME(),egress_queue, ingress_queue));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_RESOURCE;
	    }
	    if (BCM_STK_MOD_TO_NODE(BCM_GPORT_EGRESS_CHILD_MODID_GET(egress_queue)) != node) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s egress queue 0x%x not connected to ingress queue 0x%x, node mismatch\n"),
		           FUNCTION_NAME(),egress_queue, ingress_queue));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_RESOURCE;
	    }
	}
	
	requested_fcd = ATTACH_ID_FCD_GET(*attach_id);
	requested_sysport = ATTACH_ID_SYSPORT_GET(*attach_id);
	
	if (requested_sysport == BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s Requested sysport out of range(%d)\n"),
	               FUNCTION_NAME(), requested_sysport));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_RESOURCE;
	}
	
	if ((dest_type == BCM_INT_SBX_DEST_TYPE_UNICAST) &&
	    (requested_sysport > (SOC_SBX_CFG(unit)->num_sysports -1))) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s Requested sysport out of range(%d)\n"),
	               FUNCTION_NAME(), requested_sysport));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_RESOURCE;
	}
	
	if (sysport_state[unit][requested_sysport].use_cnt == 0) {
	    rv = _bcm_sbx_cosq_sysport_allocate(unit, BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE, 
						requested_sysport, node, port, eg_n, queue_region, &sysport);
	    
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s Could not allocate sysport 0x%x\n"),
		           FUNCTION_NAME(), requested_sysport));
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	} else {
	    /* Ingress node, no fcd */
	    sysport = requested_sysport;
	    sysport_state[unit][sysport].use_cnt++;
	}
	
	if (mymodid == -1) {
	    
            if (soc_feature(unit, soc_feature_arbiter)) {
                if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
                    (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
		    if (node >= SOC_SBX_CFG(unit)->cfg_num_nodes) {
			LOG_ERROR(BSL_LS_BCM_COMMON,
			          (BSL_META_U(unit,
			                      "ERROR: %s, Node (%d) exceeds num_nodes (%d),  Unit(%d)\n"),
			           FUNCTION_NAME(), unit, node, SOC_SBX_CFG(unit)->cfg_num_nodes));
                        BCM_SBX_UNLOCK(unit);
			return BCM_E_PARAM;
		    }
		    offset = node * fcd_max;
		}
	    }
	    
	    if (requested_fcd >= fcd_max) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s illegal attach_id 0x%x\n"),
		           FUNCTION_NAME(),*attach_id));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_RESOURCE;
	    }

	    if (fcd_state[unit][offset + requested_fcd].ref_cnt == 0) {
		rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_ALLOC, requested_fcd, sysport, node, port, -1, &fcd);
		
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s Could not allocate fcd\n"),
		               FUNCTION_NAME()));	
		    _bcm_sbx_cosq_sysport_deallocate(unit, sysport);	
		    BCM_SBX_UNLOCK(unit);
		    return rv;
		}
	    }
	}

	/* call MCM layer */
	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_queue_attach, 
				  (unit, flags, ingress_queue, ingress_int_pri, egress_queue, egress_int_pri, attach_id));
	
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s ingress queue (0x%x) to egress queue (0x%x) failed error(%d)\n"), 
	               FUNCTION_NAME(), ingress_queue, egress_queue, rv));
	    if (sysport_state[unit][sysport].use_cnt == 1) {
		_bcm_sbx_cosq_sysport_deallocate(unit, sysport);
	    } else {
		sysport_state[unit][sysport].use_cnt--;
	    }
	    _bcm_sbx_cosq_fcd_free(unit, fcd, node);
	    *attach_id = -1;
	}
	
	BCM_SBX_UNLOCK(unit);
	return rv;
    }

    /*
     * Check egress resources on egress node 
     */
    subport = -1;
    rv = bcm_sbx_cosq_egress_group_info_get(unit, egress_queue, &subport, &eg_n, &num_fifos);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, original_subport 0x%x does not contain fabric_port, unit %d\n"),
	           FUNCTION_NAME(), egress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }
    
    if (egress_int_pri >= num_fifos) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, egress_int_pri %d exceeds %d fifos for egress_queue 0x%x, unit %d\n"),
	           FUNCTION_NAME(), egress_int_pri, num_fifos, egress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
    fifo = sp_info->egroup[eg_n].es_scheduler_level0_node + egress_int_pri;
    
    /* If attach id already exists for this ingress_int_pri, then return an error */
    if (sp_info->egroup[eg_n].fcd[ingress_int_pri] != BCM_INT_SBX_INVALID_FCD) {
	LOG_WARN(BSL_LS_BCM_COMMON,
	         (BSL_META_U(unit,
	                     "WARNING: %s, Ingress Queue 0x%x cos %d is currently connected to egress queue 0x%x attach_id 0x%x, unit %d\n"),
	          FUNCTION_NAME(), ingress_queue, ingress_int_pri, 
	          sp_info->egroup[eg_n].egroup_gport, sp_info->egroup[eg_n].fcd[ingress_int_pri], unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (bcm_sbx_cosq_fcd_get_from_fifo(unit, fifo, &fcd, node) == BCM_E_NOT_FOUND) {

	if ((flags & BCM_COSQ_GPORT_QUEUE_ATTACH_WITH_ID) == 0) {
	    /*
	     * Allocate a flow control domain
	     */
	    rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_ALLOC, requested_fcd, -1, node, port, fifo, &fcd);
	    
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s Could not allocate fcd\n"),
		           FUNCTION_NAME()));
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }

	    rv = _bcm_sbx_cosq_sysport_allocate(unit, BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE, 
						requested_sysport, node, port, eg_n, queue_region, &sysport);
	    
	    if (rv != BCM_E_NONE) {
		_bcm_sbx_cosq_fcd_free(unit, fcd, node);
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s Could not allocate sysport\n"),
		           FUNCTION_NAME()));
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	} else {
	    requested_fcd = ATTACH_ID_FCD_GET(*attach_id);
	    if (requested_fcd >= fcd_max) requested_fcd = -1;
	    rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_ALLOC, requested_fcd, BCM_INT_SBX_INVALID_SYSPORT, node, port, fifo, &fcd);
	    
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s Could not allocate fcd\n"),
		           FUNCTION_NAME()));
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }

	    requested_sysport = ATTACH_ID_SYSPORT_GET(*attach_id);

	    if (requested_sysport == BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT) {
		/* coverity[dead_error_begin] */
		_bcm_sbx_cosq_fcd_free(unit, fcd, node);
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s Requested sysport out of range(%d)\n"),
		           FUNCTION_NAME(), requested_sysport));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_RESOURCE;
	    }

	    if ((dest_type == BCM_INT_SBX_DEST_TYPE_UNICAST) &&
		(requested_sysport > (SOC_SBX_CFG(unit)->num_sysports -1))) {
		_bcm_sbx_cosq_fcd_free(unit, fcd, node);
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s Requested sysport out of range(%d)\n"),
		           FUNCTION_NAME(), requested_sysport));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_RESOURCE;
	    }

	    rv = _bcm_sbx_cosq_sysport_allocate(unit, BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE, 
						requested_sysport, node, port, eg_n, queue_region, &sysport);
	    
	    if (rv != BCM_E_NONE) {
		_bcm_sbx_cosq_fcd_free(unit, fcd, node);
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s could not allocate sysport 0x%x\n"),
		           FUNCTION_NAME(), requested_sysport));
		BCM_SBX_UNLOCK(unit);
		return rv;
	    }
	}
	fcd_state[unit][fcd].sysport = sysport;
    } else {
	sysport = fcd_state[unit][fcd].sysport;
	sysport_state[unit][sysport].use_cnt++;
	fcd_state[unit][fcd].ref_cnt++;
    }

    ATTACH_ID_SET(*attach_id, fcd, sysport);

    /* call MCM layer */
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_queue_attach, 
			      (unit, flags, ingress_queue, ingress_int_pri, egress_queue, egress_int_pri, attach_id));

    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s ingress queue (0x%x) to egress queue (0x%x) failed error(%d)\n"), 
                   FUNCTION_NAME(), ingress_queue, egress_queue, rv));
	if (_bcm_sbx_cosq_fcd_free(unit, fcd, node) == BCM_E_NONE) {
	    sp_info->egroup[eg_n].fcd[ingress_int_pri] = -1;
	}
	if (sysport_state[unit][sysport].use_cnt == 1) {
	    _bcm_sbx_cosq_sysport_deallocate(unit, sysport);
	} else {
	    sysport_state[unit][sysport].use_cnt--;
	}
	*attach_id = -1;
        BCM_SBX_UNLOCK(unit);
        return rv;
    }
    sp_info->egroup[eg_n].fcd[ingress_int_pri] = *attach_id;

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int 
bcm_sbx_cosq_gport_queue_attach_get(
    int unit, 
    bcm_gport_t ingress_queue, 
    bcm_cos_t ingress_int_pri, 
    bcm_gport_t *egress_queue, 
    bcm_cos_t *egress_int_pri, 
    int attach_id)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int subport = 0, eg_n = 0, num_fifos = 0, temp = 0;
    uint16 base_queue = 0, queue = 0;
    int fifo = 0, fcd = 0, modid = 0, mymodid = -1;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
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
	BCM_SBX_UNLOCK(unit);
	return(BCM_E_PARAM);
    }

    queue = base_queue + ingress_int_pri;
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];

    if (p_bwstate->base_queue != base_queue) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_queue 0x%x is not a base queue, unit %d\n"),
	           FUNCTION_NAME(), ingress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (ingress_int_pri >= p_bwstate->num_cos) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_int_pri %d exceeds %d cos for ingress_queue 0x%x, unit %d\n"),
	           FUNCTION_NAME(), ingress_int_pri, p_bwstate->num_cos, ingress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        if (egress_int_pri == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, multicast egress_int_pri pointer is NULL\n"),
	               FUNCTION_NAME()));
	    BCM_SBX_UNLOCK(unit);
	    return(BCM_E_PARAM);
        }

        if (egress_queue != NULL) {
            (*egress_queue) = (bcm_gport_t)NULL;
        }
        if ( (p_qstate->attached_fifo != BCM_COS_MULTICAST_EF) && (p_qstate->attached_fifo != BCM_COS_MULTICAST_NON_EF) ) {
            (*egress_int_pri) = (p_qstate->ingress.bw_mode == BCM_COSQ_EF) ? BCM_COS_MULTICAST_EF : BCM_COS_MULTICAST_NON_EF;
        }
        else {
            (*egress_int_pri) = p_qstate->attached_fifo;
        }

        BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    fcd = ATTACH_ID_FCD_GET(attach_id);

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
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Unexpected gport from queue %d, 0x%x\n"),
	           FUNCTION_NAME(), queue, p_bwstate->gport));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_INTERNAL;
    }

    rv = bcm_stk_modid_get(unit, &mymodid);

    if ((rv != BCM_E_NONE) && (rv != BCM_E_UNAVAIL)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, cannot retrieve system modid on unit %d\n"),
	           FUNCTION_NAME(), unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_INTERNAL;
    }

    if (modid != mymodid) {
	if (mymodid != -1) {
	    /* remote node does not support this feature */
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s unsupported on remote node, unit %d\n"),
	               FUNCTION_NAME(), unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_UNAVAIL;
	}
	if (p_qstate->sysport != p_qstate->default_sysport) {
	    ATTACH_ID_SET(temp, sysport_state[unit][p_qstate->sysport].fcd, p_qstate->sysport);
	    if (temp == attach_id) {
		*egress_int_pri = p_qstate->attached_fifo;
		*egress_queue = sysport_state[unit][p_qstate->sysport].egport;
		BCM_SBX_UNLOCK(unit);
		return BCM_E_NONE;
	    } else {
		if ((*egress_queue == -1) && (*egress_int_pri == -1)) {
		    BCM_SBX_UNLOCK(unit);
		    return (temp);
		}
		
		*egress_queue = -1; 
		*egress_int_pri = -1; 

		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Attach Id 0x%x does not match 0x%x associated with queue 0x%x\n"),
		           FUNCTION_NAME(), attach_id, temp, ingress_queue));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Ingress queue 0x%x currently not associated with an Egress queue, unit %d\n"),
	               FUNCTION_NAME(), ingress_queue, unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
    }

    subport = -1;
    rv = bcm_sbx_cosq_egress_group_info_get(unit, p_bwstate->gport, &subport, &eg_n, &num_fifos);
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
	           FUNCTION_NAME(), p_bwstate->gport, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }    
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);

    if ((*egress_queue == -1) && (*egress_int_pri == -1)) {
        BCM_SBX_UNLOCK(unit);
        return (sp_info->egroup[eg_n].fcd[ingress_int_pri]);
    }

    *egress_queue = -1; 
    *egress_int_pri = -1; 

    if (sp_info->egroup[eg_n].fcd[ingress_int_pri] != attach_id) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_queue 0x%x does not contain attach_id 0x%x at int_pri %d, unit %d\n"),
	           FUNCTION_NAME(), ingress_queue, attach_id, ingress_int_pri, unit));
	rv = BCM_E_PARAM;
    } else {
	fifo = fcd_state[unit][fcd].fifo;
	if ((sp_info->egroup[eg_n].es_scheduler_level0_node <= fifo) && 
	    ((sp_info->egroup[eg_n].es_scheduler_level0_node + num_fifos) > fifo)) {
	    *egress_int_pri = fifo - sp_info->egroup[eg_n].es_scheduler_level0_node;
	    *egress_queue = sp_info->egroup[eg_n].egroup_gport;
	    rv = BCM_E_NONE;
	} else {
	    rv = BCM_E_NOT_FOUND;
	}
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int 
bcm_sbx_cosq_gport_queue_detach(
    int unit, 
    bcm_gport_t ingress_queue, 
    bcm_cos_t ingress_int_pri, 
    int attach_id)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate = NULL;
    bcm_sbx_cosq_queue_state_t *p_qstate = NULL;
    bcm_sbx_subport_info_t *sp_info = NULL;
    int subport = 0, eg_n = 0, num_fifos = 0;
    uint16 base_queue = 0, queue = 0;
    int fcd = ATTACH_ID_FCD_GET(attach_id);
    int sysport = ATTACH_ID_SYSPORT_GET(attach_id);
    int mymodid = -1, modid = 0;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        BCM_SBX_UNLOCK(unit);
        return(BCM_E_UNAVAIL);
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
	BCM_SBX_UNLOCK(unit);
	return(BCM_E_PARAM);
    }

    queue = base_queue + ingress_int_pri;
    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_qstate = &p_qstate[queue];
    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[p_qstate->bw_group];

    if (p_bwstate->base_queue != base_queue) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_queue 0x%x is not a base queue, unit %d\n"),
	           FUNCTION_NAME(), ingress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (ingress_int_pri >= p_bwstate->num_cos) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, ingress_int_pri %d exceeds %d cos for ingress_queue 0x%x, unit %d\n"),
	           FUNCTION_NAME(), ingress_int_pri, p_bwstate->num_cos, ingress_queue, unit));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;
    }

    if (p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {

        /* call MCM layer */
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_queue_detach, (unit, ingress_queue, ingress_int_pri, attach_id));
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s ingress queue/cos (0x%x/%d) failed error (%d)\n"),
                       FUNCTION_NAME(), ingress_queue, ingress_int_pri, rv));
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        p_qstate->attached_fifo = -1;

        BCM_SBX_UNLOCK(unit);
        return(rv);
    }

    rv = bcm_stk_modid_get(unit, &mymodid); 
    if ((rv != BCM_E_NONE) && (rv != BCM_E_UNAVAIL)) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, cannot retrieve system modid on unit %d\n"),
	           FUNCTION_NAME(), unit));
	BCM_SBX_UNLOCK(unit);
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
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, Unexpected gport from queue %d, 0x%x\n"),
	           FUNCTION_NAME(), queue, p_bwstate->gport));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_INTERNAL;
    }

    if (sysport_state[unit][sysport].use_cnt == 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s ingress queue/cos (0x%x/%d) not associated with attach_id 0x%x\n"),
	           FUNCTION_NAME(), ingress_queue, ingress_int_pri, attach_id));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_INTERNAL;
    }

    if (modid == mymodid) {
	subport = -1;
	rv = bcm_sbx_cosq_egress_group_info_get(unit, p_bwstate->gport, &subport, &eg_n, &num_fifos);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
	               FUNCTION_NAME(), p_bwstate->gport, unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}    
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	
	if (sp_info->egroup[eg_n].fcd[ingress_int_pri] != attach_id) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Ingress Queue 0x%x does not contain attach_id 0x%x at int_pri %d, unit %d\n"),
	               FUNCTION_NAME(), ingress_queue, attach_id, ingress_int_pri, unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
	
	if (attach_id == BCM_INT_SBX_INVALID_FCD) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Invalid attach ID 0x%x,  unit %d\n"),
	               FUNCTION_NAME(), attach_id, unit));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
    } else {
	if ((p_qstate->sysport == p_qstate->default_sysport) &&
	    (sysport_state[unit][sysport].use_cnt != 2)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s ingress queue/cos (0x%x/%d) failed error (%d)\n"),
                       FUNCTION_NAME(), ingress_queue, ingress_int_pri, BCM_E_PARAM));
	    BCM_SBX_UNLOCK(unit);
	    return BCM_E_PARAM;
	}
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_queue_detach, (unit, ingress_queue, ingress_int_pri, attach_id));
    
    if (rv != BCM_E_NONE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s ingress queue (0x%x) attach_id (0x%x) failed error(%d)\n"), 
	           FUNCTION_NAME(), ingress_queue, attach_id, rv));
	BCM_SBX_UNLOCK(unit);
	return rv;
    }

    if (modid == mymodid) {
	_bcm_sbx_cosq_fcd_free(unit, fcd, p_bwstate->dest_node);
	sp_info->egroup[eg_n].fcd[ingress_int_pri] = -1;
    } 

    if (mymodid == -1) {
	_bcm_sbx_cosq_fcd_free(unit, fcd, p_bwstate->dest_node);
    }
    
    if (sysport_state[unit][sysport].use_cnt == 1) {
	_bcm_sbx_cosq_sysport_deallocate(unit, sysport);
    } else {
	sysport_state[unit][sysport].use_cnt--;
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_cosq_gport_stat_enable_set(int unit,
        bcm_gport_t gport,
        int enable)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_stat_enable_set, (unit, gport, enable));


    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_stat_enable_get(int unit,
        bcm_gport_t gport,
        int *enable)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
    int base_queue;
    int num_cos;

    if (SOC_IS_SBX_QE2000(unit)) {

        /* Convert gport to base queue number */
        rv = _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, &num_cos);
        if (rv != BCM_E_NONE) {
            BCM_SBX_UNLOCK(unit);
            return rv;
        }

        LOG_VERBOSE(BSL_LS_BCM_COMMON,
                    (BSL_META_U(unit,
                                "gport %d base q %d num_cos %d\n"),
                     gport, base_queue, num_cos));

        rv = (bcm_qe2000_cosq_gport_stat_enable_get(unit, base_queue, enable));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_stat_enable_get, (unit, gport, enable));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_stat_get(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_gport_stats_t stat,
                            uint64 *value)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
    if (SOC_IS_SBX_QE2000(unit)) {
        rv = (bcm_qe2000_cosq_gport_stat_get(unit, gport, cosq, stat, value));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif
    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_stat_get, (unit, gport,  cosq, stat, value));

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int
bcm_sbx_cosq_gport_stat_set(int unit,
                            bcm_gport_t gport,
                            bcm_cos_queue_t cosq,
                            bcm_cosq_gport_stats_t stat,
                            uint64 value)
{
    int rv = BCM_E_UNAVAIL;

    BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
    if (SOC_IS_SBX_QE2000(unit)) {
        rv = (bcm_qe2000_cosq_gport_stat_set(unit, gport, cosq, stat, value));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }
#endif

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_stat_set, (unit, gport,  cosq, stat, value));
    BCM_SBX_UNLOCK(unit);
    return rv;
}


int
bcm_sbx_cosq_gport_stat_config_set(int unit,
                                   bcm_gport_t gport,
                                   bcm_gport_t lgl_gport,
                                   bcm_cos_queue_t cosq,
                                   uint32 flags,
                                   bcm_cosq_gport_stat_profile_t profile)
{
  int rv = BCM_E_UNAVAIL;
  BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
  if (SOC_IS_SBX_SIRIUS(unit)) {
    rv = (bcm_sirius_cosq_gport_stat_config_set(unit, gport, lgl_gport, cosq, flags, profile));
    BCM_SBX_UNLOCK(unit);
    return rv;
  }
#endif
  rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_stat_config_set, (unit, gport, lgl_gport, cosq, flags, profile));

  BCM_SBX_UNLOCK(unit);
  return rv;
}

int
bcm_sbx_cosq_gport_stat_config_get(int unit,
                                   bcm_gport_t gport,
                                   bcm_gport_t lgl_gport,
                                   bcm_cos_queue_t cosq,
                                   uint32 flags,
                                   bcm_cosq_gport_stat_profile_t *profile)
{
  int rv = BCM_E_UNAVAIL;
  BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
  if (SOC_IS_SBX_SIRIUS(unit)) {
    rv = (bcm_sirius_cosq_gport_stat_config_get(unit, gport, lgl_gport, cosq, flags, profile));
    BCM_SBX_UNLOCK(unit);
    return rv;
  }
#endif
  rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_stat_config_get, (unit, gport, lgl_gport,  cosq, flags, profile));

  BCM_SBX_UNLOCK(unit);
  return rv;
}

int
bcm_sbx_cosq_gport_statistic_set(int unit,
                                 bcm_gport_t gport,
                                 bcm_gport_t lgl_gport,
                                 bcm_cos_queue_t cosq,
                                 uint32 flags,
                                 bcm_cosq_gport_stats_t stat,
                                 uint64 value)
{
  int rv = BCM_E_UNAVAIL;
  BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
  if (SOC_IS_SBX_SIRIUS(unit)) {
    rv = (bcm_sirius_cosq_gport_statistic_set(unit, gport, lgl_gport, cosq, flags, stat, value));
    BCM_SBX_UNLOCK(unit);
    return rv;
  }
#endif
  rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_statistic_set, (unit, gport, lgl_gport, cosq, flags, stat, value));

  BCM_SBX_UNLOCK(unit);
  return rv;
}

int
bcm_sbx_cosq_gport_statistic_get(int unit,
                                 bcm_gport_t gport,
                                 bcm_gport_t lgl_gport,
                                 bcm_cos_queue_t cosq,
                                 uint32 flags,
                                 bcm_cosq_gport_stats_t stat,
                                 uint64 *value)
{
  int rv = BCM_E_UNAVAIL;
  BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
  if (SOC_IS_SBX_SIRIUS(unit)) {
    rv = (bcm_sirius_cosq_gport_statistic_get(unit, gport, lgl_gport, cosq, flags, stat, value));
    BCM_SBX_UNLOCK(unit);
    return rv;
  }
#endif
  rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_statistic_get, (unit, gport, lgl_gport, cosq, flags, stat, value));

  BCM_SBX_UNLOCK(unit);
  return rv;
}

int
bcm_sbx_cosq_gport_statistic_multi_set(int unit,
				       bcm_gport_t gport,
				       bcm_gport_t lgl_gport,
				       bcm_cos_queue_t cosq,
				       int stat_count,
				       bcm_cosq_gport_stats_t *stats_array,
				       uint64 value)
{
  int rv = BCM_E_UNAVAIL;
  BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
  if (SOC_IS_SBX_SIRIUS(unit)) {
    rv = (bcm_sirius_cosq_gport_statistic_multi_set(unit, gport, lgl_gport, cosq, stat_count, stats_array, value));
    BCM_SBX_UNLOCK(unit);
    return rv;
  }
#endif
  rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_statistic_multi_set, (unit, gport, lgl_gport, cosq, stat_count, stats_array, value));

  BCM_SBX_UNLOCK(unit);
  return rv;
}

int
bcm_sbx_cosq_gport_statistic_multi_get(int unit,
                                       bcm_gport_t gport,
				       bcm_gport_t lgl_gport,
				       bcm_cos_queue_t cosq,
				       int stat_count,
				       bcm_cosq_gport_stats_t *stats_array,
				       int value_count,
				       uint64 *value_array)
{
  int rv = BCM_E_UNAVAIL;
  BCM_SBX_LOCK(unit);

#ifdef OLD_WAY_CDP
  if (SOC_IS_SBX_SIRIUS(unit)) {
    rv = bcm_sirius_cosq_gport_statistic_multi_get(unit, gport, lgl_gport, cosq, stat_count, stats_array, value_count, value_array);
    BCM_SBX_UNLOCK(unit);
    return rv;
  }
#endif
  rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_statistic_multi_get, (unit, gport, lgl_gport, cosq, stat_count, stats_array, value_count, value_array));

  BCM_SBX_UNLOCK(unit);
  return rv;
}

/********************/
/* STATIC FUNCTIONS */
/********************/

static int
_bcm_sbx_cosq_queue_is_local(int unit, int32 queue)
{
    int is_local;


    is_local = (queue_state[unit][queue].ingress.local) ? TRUE : FALSE;
    return(is_local);
}

static int
_bcm_sbx_cosq_get_queue_from_gport(int unit, bcm_gport_t gport, int *p_queue, bcm_cos_queue_t cosq, int *p_num_cos_levels)
{
  int rv;
  int base_queue;

  *p_queue = -1;

  rv = _bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, &base_queue, p_num_cos_levels);

  if (rv != BCM_E_NONE) {
      return rv;
  }

#if 0
    /* Check priority  */
    if (!BCM_COSQ_PRIO_VALID(cosq)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:invalid cosq value(%d) only 0-7 currently supported\n"),
                   (int)cosq));
        rv = BCM_E_PARAM;
        return rv;
    }
#endif /* 0 */

    if (cosq >= *p_num_cos_levels) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR:invalid cosq value(%d) outside of num_cos_levels(%d)\n"),
                   (int)cosq, *p_num_cos_levels));
        rv = BCM_E_PARAM;
        return rv;
    }

    *p_queue = base_queue + cosq;

    return rv;
}

static int
_bcm_sbx_cosq_get_base_queue_from_gport(int unit, bcm_gport_t gport, int *p_base_queue, int *p_num_cos_levels)
{
    int rv = BCM_E_NONE;
    int node = 0, port = 0;
    int fabricport = 0, switch_port=0;
    int num_cos = 0;
    int ds_id = 0;
    int bw_group;
    int modid = 0;
    int protocol;

    if ((BCM_GPORT_IS_MODPORT(gport) == FALSE) &&
        (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) == FALSE) &&
        (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) == FALSE) &&
	(BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) == FALSE) &&
	(BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport) == FALSE) &&
        (BCM_GPORT_IS_MCAST(gport) == FALSE) &&
	(BCM_GPORT_IS_CHILD(gport) == FALSE) &&
        (BCM_GPORT_IS_EGRESS_CHILD(gport) == FALSE) &&
        (BCM_GPORT_IS_EGRESS_MODPORT(gport) == FALSE)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: gport %08X type is not supported\n"),
                   gport));
        rv = BCM_E_PARAM;
        return rv;
    }

    /* verify queue is in use */
    if (BCM_GPORT_IS_MODPORT(gport) ||
        BCM_GPORT_IS_EGRESS_MODPORT(gport)) {

        num_cos = sbx_num_cosq[unit];

        /* The gport port is of the form mod/port */
        if (BCM_GPORT_IS_MODPORT(gport)) {
        modid = BCM_GPORT_MODPORT_MODID_GET(gport);
        port = BCM_GPORT_MODPORT_PORT_GET(gport);
        } else {
            modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
            port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
        }
        if (modid < BCM_MODULE_FABRIC_BASE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: gport modid invalid value (%d) is less than minimum (%d)\n"),
                       modid, BCM_MODULE_FABRIC_BASE));
            rv = BCM_E_PARAM;
        }
        node = modid - BCM_MODULE_FABRIC_BASE;

	if (node < BCM_STK_MAX_MODULES) {
	    protocol = SOC_SBX_STATE(unit)->stack_state->protocol[node];
	} else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unknown node type for modid(%d)\n"),
                       modid));
            rv = BCM_E_INIT;
            BCM_SBX_UNLOCK(unit);
            return rv;
	}

	/* check the destination node type */
	if ( (protocol == bcmModuleProtocol3) || (protocol == bcmModuleProtocol4) || (protocol == bcmModuleProtocol5)) {
	    /* destination node is sirius */
	    if ( port == SB_FAB_DEVICE_SIRIUS_CPU_PORT ) {
		fabricport = SB_FAB_DEVICE_SIRIUS_CPU_HANDLE;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: gport type MODPORT, must be cpu port\n")));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	} else {
	    /* Determine if SPI port or CPU port and subtract the offset     */
	    /* from the port number passed in.  The port number is based on  */
	    /* the bit position of the port in the ports bit mask.  All      */
	    /* ports are stored in the ports bit mask, including sci, sfi,   */
	    /* cpu, etc.. The spi subports must be contiguous.               */
	    BCM_INT_SBX_GPORT_GET_FABRICPORT_FROM_MODPORT(gport, &fabricport);

	    if (fabricport < 0) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: gport type MODPORT, port field invalid\n")));
		BCM_SBX_UNLOCK(unit);
		return BCM_E_PARAM;
	    }
	}

        *p_base_queue = SOC_SBX_NODE_PORT_TO_QID(unit,node, fabricport, num_cos);

    } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        /* The gport is of the form sysport/qid */
        *p_base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);

        bw_group = queue_state[unit][*p_base_queue].bw_group;
        num_cos = bw_group_state[unit][bw_group].num_cos;

    } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        /* The gport is of the form qid */
        *p_base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);

        bw_group = queue_state[unit][*p_base_queue].bw_group;
        num_cos = bw_group_state[unit][bw_group].num_cos;

    } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        /* The gport is of the form qid */
        *p_base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);

        bw_group = queue_state[unit][*p_base_queue].bw_group;
        num_cos = bw_group_state[unit][bw_group].num_cos;

    } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
        /* The gport is of the form sysport/qid */
        *p_base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(gport);

        bw_group = queue_state[unit][*p_base_queue].bw_group;
        num_cos = bw_group_state[unit][bw_group].num_cos;

    } else if (BCM_GPORT_IS_MCAST(gport)) {
        ds_id = BCM_GPORT_MCAST_DS_ID_GET(gport);

        num_cos = sbx_num_cosq[unit];

        *p_base_queue = SOC_SBX_DS_ID_TO_QID(unit, ds_id, num_cos);
    } else if (BCM_GPORT_IS_CHILD(gport) || BCM_GPORT_IS_EGRESS_CHILD(gport)) {
	rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, gport, &switch_port);
	if (rv != SOC_E_NONE) {
	    /* try to search with the other child port type */
	    if (BCM_GPORT_IS_CHILD(gport)) {
		BCM_GPORT_EGRESS_CHILD_SET(switch_port, BCM_GPORT_CHILD_MODID_GET(gport), 
					   BCM_GPORT_CHILD_PORT_GET(gport));
	    } else {
		BCM_GPORT_CHILD_SET(switch_port, BCM_GPORT_EGRESS_CHILD_MODID_GET(gport),
				    BCM_GPORT_EGRESS_CHILD_PORT_GET(gport));
	    }
	    rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, switch_port, &switch_port);
	    if (rv != SOC_E_NONE) {
                /* try to search with modport */
                if (BCM_GPORT_IS_CHILD(gport)) {
                    BCM_GPORT_MODPORT_SET(switch_port,
                                          BCM_GPORT_CHILD_MODID_GET(gport),
                                          BCM_GPORT_CHILD_PORT_GET(gport));
                } else {
                    BCM_GPORT_MODPORT_SET(switch_port,
                                          BCM_GPORT_EGRESS_CHILD_MODID_GET(gport),
                                          BCM_GPORT_EGRESS_CHILD_PORT_GET(gport));
                }
                rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, switch_port, &switch_port);
	    }
            if (rv != SOC_E_NONE) {
                /* try to search with egress modport */
                if (BCM_GPORT_IS_CHILD(gport)) {
                    BCM_GPORT_EGRESS_MODPORT_SET(switch_port,
                                                BCM_GPORT_CHILD_MODID_GET(gport),
                                                BCM_GPORT_CHILD_PORT_GET(gport));
                } else {
                    BCM_GPORT_MODPORT_SET(switch_port,
                                          BCM_GPORT_EGRESS_CHILD_MODID_GET(gport),
                                          BCM_GPORT_EGRESS_CHILD_PORT_GET(gport));
                }
                rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, switch_port, &switch_port);
	    }
	}

	/* switch gport has to be a modport */
        if (BCM_GPORT_IS_MODPORT(switch_port)) {
	    modid = BCM_GPORT_MODPORT_MODID_GET(switch_port);
	    node = modid;
	    switch_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);
        } else if (BCM_GPORT_IS_EGRESS_MODPORT(switch_port)) {
            modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(switch_port);
            node = modid;
            switch_port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(switch_port);
        } else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "got non-MODPORT from"
                                   " bcm_sbx_stk_fabric_map_get_switch_port(%d,%08X,*)\n"),
                       unit,
                       gport));
            return BCM_E_PARAM;
	}

	num_cos = sbx_num_cosq[unit];

	/* assuming fixed mapping between queue group and front panel node/port, same assumption
	 * was made for MODPORT type
	 */
        *p_base_queue = SOC_SBX_NODE_PORT_TO_QID(unit, node, switch_port, num_cos);
    }

    *p_num_cos_levels = num_cos;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "base queue(%d) num cos levels(%d)\n"),
              *p_base_queue, num_cos));
    return rv;
}

int
_bcm_sbx_cosq_is_all(int unit, bcm_gport_t gport)
{
    int     base_queue;
    int     is_all = FALSE;


    if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        /* The gport is of the form sysport/qid */
        base_queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
        is_all = ((base_queue & _SHR_GPORT_UCAST_QUEUE_GROUP_QID_MASK) == _SHR_GPORT_UCAST_QUEUE_GROUP_QID_MASK) ? TRUE : FALSE;  
    }
    else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        /* The gport is of the form qid */
        base_queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
        is_all = ((base_queue & _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK) == _SHR_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK) ? TRUE : FALSE;  
    }
    else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)) {
        /* The gport is of the form qid */
        base_queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(gport);
        is_all = ((base_queue & _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK) == _SHR_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_MASK) ? TRUE : FALSE;  
    }
    else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
        /* The gport is of the form sysport/qid */
        base_queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(gport);
        is_all = ((base_queue & _SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK) == _SHR_GPORT_MCAST_QUEUE_GROUP_QID_MASK) ? TRUE : FALSE;  
    }
    else {
        is_all = FALSE;
    }

    return(is_all);
}


int _bcm_sbx_cosq_queue_regions_set(unit)
{
    int                             rv = BCM_E_NONE, region;
    int                             hybrid_mode_queue_demarcation = 0;
    bcm_sbx_cosq_queue_region_t     *p_qr;

    if ( soc_feature(unit, soc_feature_hybrid) ) {
        if (SOC_IS_SBX_SIRIUS(unit)) {
            hybrid_mode_queue_demarcation = SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq;
        } else {
            hybrid_mode_queue_demarcation = BM9600_BW_MAX_VOQ_NUM;
        }
    }

    for (region = 0; region < bcm_sbx_cosq_queue_region_last; region++) {
        p_qr = queue_regions[unit] + region;

        switch(region) {
            case bcm_sbx_cosq_queue_region_global:
                if ( soc_feature(unit, soc_feature_node) &&
                                  soc_feature(unit, soc_feature_standalone) ) {
                    p_qr->is_valid = FALSE;
                    p_qr->start = 0;
                    p_qr->end = 0;
                }
                else if ( soc_feature(unit, soc_feature_hybrid) ) {
                    p_qr->is_valid = TRUE;
                    p_qr->start = 0;
                    p_qr->end = hybrid_mode_queue_demarcation;
                }
                else {
                    p_qr->is_valid = TRUE;
                    p_qr->start = 0;
                    p_qr->end = SOC_SBX_CFG(unit)->num_queues;
                }

                break;

            case bcm_sbx_cosq_queue_region_local:
                if ( soc_feature(unit, soc_feature_standalone)) {
                    p_qr->is_valid = TRUE;
                    p_qr->start = 0;
                    p_qr->end = SOC_SBX_CFG(unit)->num_queues;
                }
                else if ( soc_feature(unit, soc_feature_node) &&
                                      soc_feature(unit, soc_feature_hybrid) ) {
                    p_qr->is_valid = TRUE;
                    p_qr->start = hybrid_mode_queue_demarcation;
                    p_qr->end = SOC_SBX_CFG(unit)->num_queues;
                }
                else {
                    p_qr->is_valid = FALSE;
                    p_qr->start = 0;
                    p_qr->end = 0;
                }

                break;

            /* coverity[dead_error_begin] */
            default:
                return BCM_E_INTERNAL;
        }
    }

    return rv;
}

static int
_bcm_sbx_cosq_queue_state_init(int unit, int32 num_cos) 
{
    int queue;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int num_queues;
    int rc = BCM_E_NONE;

    num_queues = SOC_SBX_CFG(unit)->num_queues;

    queue_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_queue_state_t) * num_queues,
                                  "queue state memory");

    if (queue_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }

    queue_regions[unit] = sal_alloc((sizeof(bcm_sbx_cosq_queue_region_t) *
                                     bcm_sbx_cosq_queue_region_last), "queue region memory");
    if (queue_regions[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        rc = BCM_E_MEMORY;
        goto err;
    }

    for (queue = 0; queue < SOC_SBX_CFG(unit)->num_queues; queue++) {

        p_qstate = &queue_state[unit][queue];

        /* clear queue state */
        sal_memset(p_qstate, 0, sizeof(bcm_sbx_cosq_queue_state_t));

        /* initialize specific parameters */
        p_qstate->state = BCM_INT_SBX_QUEUE_STATE_AVAILABLE;

        /* No bw_group allocated for this queue */
        p_qstate->bw_group = -1;

        /* No sysport allocated for this queue */
        p_qstate->sysport = BCM_INT_SBX_INVALID_SYSPORT;
        p_qstate->default_sysport = BCM_INT_SBX_INVALID_SYSPORT;
        p_qstate->attached_fifo = -1;
	p_qstate->enq_stat_in_use = 0;
	p_qstate->deq_stat_in_use = 0;

	/* No multicast group ids for this queue */
	p_qstate->mgid_list = NULL;
    }

    rc = _bcm_sbx_cosq_queue_regions_set(unit);
    if (rc != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unit:%d - %s, internal error setting queue_regions \n"), 
                   unit, FUNCTION_NAME()));
        goto err;
    }

    return BCM_E_NONE;

err:
    if (queue_state[unit] != NULL) {
        sal_free(queue_state[unit]);
        queue_state[unit] = NULL;
    }
    if (queue_regions[unit] != NULL) {
        sal_free(queue_regions[unit]);
        queue_regions[unit] = NULL;
    }

    return(rc);
}

static void
_bcm_sbx_cosq_queue_state_detach(int unit)
{
    if (queue_regions[unit] != NULL) {
        sal_free(queue_regions[unit]);
        queue_regions[unit] = NULL;
    }

    if (queue_state[unit] != NULL) {
        sal_free(queue_state[unit]);
        queue_state[unit] = NULL;
    }
}

/***********************************
 *  Initialize bw_group array
 */
static int
_bcm_sbx_cosq_bw_group_state_init(int unit, int32 num_cos) 
{
    int bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int ds_id;
    int num_bw_groups;
    int num_ds_ids;

    num_bw_groups = SOC_SBX_CFG(unit)->num_bw_groups;

    bw_group_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_bw_group_state_t) * num_bw_groups,
                                     "bw group state memory");

    if (bw_group_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }

    sal_memset(bw_group_state[unit],
               0x00,
               sizeof(bcm_sbx_cosq_bw_group_state_t) * num_bw_groups);

    for (bw_group = 0; bw_group < num_bw_groups; bw_group++) {
        p_bwstate = &bw_group_state[unit][bw_group];
        p_bwstate->path.bag_rate_kbps =  SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps;
        /* Until bcm_gport_get_port_speed() API is approved, set the port speed to the bag rate */
        p_bwstate->dest_port_kbits_sec_max = SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps;
        p_bwstate->num_cos = num_cos;
        p_bwstate->dest_node = -1;
        p_bwstate->dest_port = -1;
        p_bwstate->base_queue = -1;
        bw_group_state[unit][bw_group].in_use = FALSE;
        bw_group_state[unit][bw_group].cosq_init_added = FALSE;
        bw_group_state[unit][bw_group].cos_map = -1;
    }

    num_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;

    ds_id_base_queue[unit] = sal_alloc(sizeof(int) * num_ds_ids,
                                     "ds_id_base_queue memory");

    if (ds_id_base_queue[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }
    for (ds_id = 0; ds_id < num_ds_ids; ds_id++) {
        ds_id_base_queue[unit][ds_id] = -1;
    }
    return BCM_E_NONE;
}

static int
_bcm_sbx_cosq_sysport_state_hybrid_init(int unit)
{
    int rc = BCM_E_NONE;
    int node, port, fic_flow_start_port;
    int sysport_index = 0;

    /* account for the CPU port */
    fic_flow_start_port = SOC_SBX_CFG(unit)->max_ports -
                                         (SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule + 1);

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        /* The number of QE2000 arbitration port resource is optimized by using */
        /* Node/port values that do not overlap with local arbitration port.    */
        /* In this mode need not look at all the node mask configuration        */
        /* NOTE: The table can instead be only populated for valid entries      */
        /*       (BCM_INT_SBX_SYSPORT_STATE_GLOBAL)                             */
        for (node = 0; node < SOC_SBX_CFG(unit)->num_nodes; node++) {
            if (SOC_SBX_CFG(unit)->cfg_node_00_31_mask & (1 << node)) {
                for (port = 0; port < fic_flow_start_port; port++) {
                    if (sysport_index >= SOC_SBX_CFG(unit)->num_sysports) { /*consistency check*/
                        return(BCM_E_PARAM);
                    }
                    sysport_state[unit][sysport_index].state = BCM_INT_SBX_SYSPORT_STATE_LOCAL;
                    sysport_state[unit][sysport_index].flags = 0;
		    sysport_state[unit][sysport_index].use_cnt = 0;
		    sysport_state[unit][sysport_index].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][sysport_index].fifo = -1;
		    sysport_state[unit][sysport_index].egport = BCM_GPORT_INVALID;
                    sysport_state[unit][sysport_index].node = node;
                    sysport_state[unit][sysport_index++].port = port;
                }
                for (port = fic_flow_start_port; port < SOC_SBX_CFG(unit)->max_ports; port++) {
                    if (sysport_index >= SOC_SBX_CFG(unit)->num_sysports) { /*consistency check*/
                        return(BCM_E_PARAM);
                    }
                    sysport_state[unit][sysport_index].state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
                    sysport_state[unit][sysport_index].flags = 0;
		    sysport_state[unit][sysport_index].use_cnt = 0;
		    sysport_state[unit][sysport_index].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][sysport_index].fifo = -1;
		    sysport_state[unit][sysport_index].egport = BCM_GPORT_INVALID;
                    sysport_state[unit][sysport_index].node = node;
                    sysport_state[unit][sysport_index++].port = port;
                }
            }
            else {
                for (port = 0; port < SOC_SBX_CFG(unit)->max_ports; port++) {
                    if (sysport_index >= SOC_SBX_CFG(unit)->num_sysports) { /*consistency check*/
                        return(BCM_E_PARAM);
                    }
                    sysport_state[unit][sysport_index].state = BCM_INT_SBX_SYSPORT_STATE_LOCAL;
                    sysport_state[unit][sysport_index].flags = 0;
		    sysport_state[unit][sysport_index].use_cnt = 0;
		    sysport_state[unit][sysport_index].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][sysport_index].egport = BCM_GPORT_INVALID;
                    sysport_state[unit][sysport_index].node = node;
                    sysport_state[unit][sysport_index++].port = port;
                }
            }
        }
#if 0 
        /* if extended esets are used, then reserve sysport space for it */
        if (SOC_SBX_CFG(unit)->use_extended_esets) {
            int num_sysports;
            num_sysports = SOC_SBX_CFG(unit)->num_sysports;
            for (sysport_index = 0;
                 sysport_index < num_sysports;
                 sysport_index++) {
                if ((sysport_index & 0x3F) <
                    BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP) {
                    /* mark this sysport as reserved for eset */
                    sysport_state[unit][sysport_index].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
                    sysport_state[unit][sysport_index].in_use = TRUE;
                    sysport_state[unit][sysport_index].node = -1;
                    sysport_state[unit][sysport_index].port = -1;
                }
            }

            for (sysport_index = 0;
                 sysport_index < BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP;
                 sysport_index++) {
                /* If the groups reserved by esets, mark the groups as all used */
                sysport_group_state[unit][sysport_index].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
                sysport_group_state[unit][sysport_index].used = sysport_group_state[unit][sysport_index].size;
                sal_memset(&(sysport_group_state[unit][sysport_index].node_cnt), 0xffffffff, sizeof(uint32) * 32);
                sal_memset(&(sysport_group_state[unit][sysport_index].node_port), 0xffffffff, sizeof(uint32) * 32);
            }
        }
#endif 
#ifdef BCM_SIRIUS_SUPPORT
        /* Sirius FR -- GNATS 22809: ESET/sysport must not overlap */
        /*
         *  Due to the way that sysport allocation is managed between devices,
         *  the sysports that Sirius can not use must be prohibited on all
         *  SBX fabric devices in the system.
         */
        
        for (sysport_index = 0; sysport_index < 128; sysport_index++) {
            /* reserve all standard ESET IDs from being used as sysports */
            sysport_state[unit][sysport_index].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
            sysport_state[unit][sysport_index].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
            sysport_state[unit][sysport_index].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
            sysport_state[unit][sysport_index].node = -1;
            sysport_state[unit][sysport_index].port = -1;
        }
        /*
         *  Groups are laterally interleaved and therefore reserving
         *  the first 128 sysports does not actually take out all of
         *  any groups.
         */
#endif /* def BCM_SIRIUS_SUPPORT */
    }
    else {
        rc = BCM_E_UNAVAIL;
    }

    return(rc);
}

static void
_bcm_sbx_cosq_bw_group_state_detach(int unit)
{
    if (ds_id_base_queue[unit] != NULL) {
        sal_free(ds_id_base_queue[unit]);
        ds_id_base_queue[unit] = NULL;
    }
 
    if (bw_group_state[unit] != NULL) {
        sal_free(bw_group_state[unit]);
        bw_group_state[unit] = NULL; 
    }
}

/***********************************
 *  Initialize sysport_state and sysport_group_state
 */
static int
_bcm_sbx_cosq_sysport_state_init(int unit)
{
    int sysport, sysport_group;
    bcm_sbx_cosq_sysport_state_t *p_spstate;
    bcm_sbx_cosq_sysport_group_state_t *p_spgstate;
    int num_sysports;
    int rc = BCM_E_NONE;
    int prev_sysport_state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
    int sysport_entry;

    

    if (soc_feature(unit, soc_feature_standalone)) {
	num_sysports = SIRIUS_TS_LOCAL_SYSPORT_LIMIT;
    } else {
	num_sysports = SOC_SBX_CFG(unit)->num_sysports;
    }

    if (num_sysports == 0) {
        /* system without sysport support */
        return BCM_E_NONE;
    }

    /* Allocate sysport state array */
    sysport_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_sysport_state_t) * num_sysports,
                                     "sysport state memory");

    if (sysport_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if ( soc_feature(unit, soc_feature_hybrid) ) {
            rc = _bcm_sbx_cosq_sysport_state_hybrid_init(unit);
        }

        return(rc);
    }

    for (sysport = 0; sysport < num_sysports; sysport++) {
        p_spstate = &sysport_state[unit][sysport];
        sysport_state[unit][sysport].state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
        p_spstate->use_cnt = 0;
        p_spstate->flags = 0;
        p_spstate->node = -1;
        p_spstate->port = -1;
	p_spstate->fcd = BCM_INT_SBX_INVALID_FCD;
	p_spstate->fifo = -1;
	p_spstate->egport = BCM_GPORT_INVALID;
    }

    /*
     * To support TME Mode Dynamic Sysport, 
     * nothing else below applies
     */
    if (soc_feature(unit, soc_feature_standalone)) {
	return BCM_E_NONE;
    }

    /* To work with QE2000, sysport is divided into 64 groups, each has uniq lower bit[5:0]
     * Allocate sysport group state array. Group 0 has lower bit[5:0] = 0, Group 1 has
     * lower bit[5:0] = 1, etc..
     */
    sysport_group_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_sysport_group_state_t) *
                                          BCM_INT_SBX_MAX_SYSPORT_GROUP, "sysport group state memory");

    if (sysport_group_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }

    for (sysport_group = 0; sysport_group < BCM_INT_SBX_MAX_SYSPORT_GROUP; sysport_group++) {
        p_spgstate = &sysport_group_state[unit][sysport_group];
        p_spgstate->state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
        p_spgstate->size = num_sysports / BCM_INT_SBX_MAX_SYSPORT_GROUP;
        p_spgstate->used = 0;
	sal_memset(&p_spgstate->node_cnt, 0, sizeof(uint32) * 32);
	sal_memset(&p_spgstate->node_port, 0xffffffff, sizeof(uint32) * 32);
    }

    /* if extended esets are used, then reserve sysport space for it */
    if (SOC_SBX_CFG(unit)->use_extended_esets) {
        for (sysport = 0; sysport < num_sysports; sysport++) {
            p_spstate = &sysport_state[unit][sysport];
            if ((sysport & 0x3F) < BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP) {
                /* mark this sysport as reserved for eset */
                p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
                p_spstate->use_cnt = 0;
		p_spstate->flags = 0;
		p_spstate->flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		p_spstate->flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
                p_spstate->node = -1;
                p_spstate->port = -1;
		p_spstate->fcd = BCM_INT_SBX_INVALID_FCD;
		p_spstate->fifo = -1;
		p_spstate->egport = BCM_GPORT_INVALID;
            }
        }

        for (sysport_group = 0; sysport_group < BCM_INT_SBX_MAX_RESERVED_SYSPORT_GROUP; sysport_group++) {
            /* If the groups reserved by esets, mark the groups as all used */
            p_spgstate = &sysport_group_state[unit][sysport_group];
            p_spgstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
            p_spgstate->used = p_spgstate->size;
	    sal_memset(&p_spgstate->node_cnt, 0xffffffff, sizeof(uint32) * 32);
	    sal_memset(&p_spgstate->node_port, 0xffffffff, sizeof(uint32) * 32);
        }
    }
#ifdef BCM_SIRIUS_SUPPORT
    /* Sirius FR -- GNATS 22809: ESET/sysport must not overlap */
    /*
     *  Due to the way that sysport allocation is managed between devices,
     *  the sysports that Sirius can not use must be prohibited on all
     *  SBX fabric devices in the system.
     */
    
    if (SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap == FALSE) {
        for (sysport = 0; sysport < 128; sysport++) {
            /* reserve all standard ESET IDs from being used as sysports */
            p_spstate = &sysport_state[unit][sysport];
            p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
            p_spstate->use_cnt = 0;
	    p_spstate->flags = 0;
	    p_spstate->flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
	    p_spstate->flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
            p_spstate->node = -1;
            p_spstate->port = -1;
	    p_spstate->fcd = BCM_INT_SBX_INVALID_FCD;
	    p_spstate->fifo = -1;
	    p_spstate->egport = BCM_GPORT_INVALID;

            /*
             *  Groups are laterally interleaved and therefore reserving the
             *  first 128 sysports doesn't actually take out all of any groups.
             *  However the number of entries available in each group has to be updated.
             */
            sysport_group = sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP;
            sysport_entry = sysport / BCM_INT_SBX_MAX_SYSPORT_GROUP;
            p_spgstate = &sysport_group_state[unit][sysport_group];
            p_spgstate->used++;
        }
    }
#endif /* def BCM_SIRIUS_SUPPORT */

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
        if (SOC_SBX_CFG(unit)->allow_ucast_mcast_resource_overlap == FALSE) {
            for (sysport = 128; sysport < SOC_SBX_CFG(unit)->num_internal_ds_ids; sysport++) {
                /* reserve all standard ESET IDs from being used as sysports */
                p_spstate = &sysport_state[unit][sysport];
                p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET;
                p_spstate->use_cnt = 0;
	        p_spstate->flags = 0;
	        p_spstate->flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
	        p_spstate->flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
                p_spstate->node = -1;
                p_spstate->port = -1;
	        p_spstate->fcd = BCM_INT_SBX_INVALID_FCD;
	        p_spstate->fifo = -1;
	        p_spstate->egport = BCM_GPORT_INVALID;
	
                /*
                 *  Groups are laterally interleaved and therefore reserving the
                 *  first 128 sysports doesn't actually take out all of any groups.
                 *  However the number of entries available in each group has to be updated.
                 */
                sysport_group = sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP;
                sysport_entry = sysport / BCM_INT_SBX_MAX_SYSPORT_GROUP;
                p_spgstate = &sysport_group_state[unit][sysport_group];
                p_spgstate->used++;
            }
        }
    }

    if (soc_feature(unit, soc_feature_egr_independent_fc)) {
        if (SOC_SBX_CFG(unit)->arbitration_port_allocation == SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2) {
            /* If individual flow control is enabled 2 sysports need to be allocated. The two  */
            /* sysports allocated are consecutive entries from the same bucket. Thus they are  */
            /* off by 64. The symantics of the API remain the same. reserve alternate sysports */
            /* entries of a bucket. Only even sysport entries for bucket will be allocated.    */
            /* The buckets odd sysport entry usage goes along with the buckets even sysport    */
            /* entry. The sysports (for odd sysyport entries in a bucket) will be marked as    */
            /* not available. The number of bucket enries available will be accordingly        */
            /* adjusted.                                                                       */

            prev_sysport_state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;

            for (sysport = 0; sysport < num_sysports; sysport++) {
                p_spstate = &sysport_state[unit][sysport];
                sysport_group = sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP;
                sysport_entry = sysport / BCM_INT_SBX_MAX_SYSPORT_GROUP;
                p_spgstate = &sysport_group_state[unit][sysport_group];

                /* skip even sysports entries */
                if ((sysport_entry % 2) == 0) {
                    prev_sysport_state = p_spstate->state;
                    continue;
                }

                /* skip sysports reserved for ESETs */
                if (p_spstate->state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET) {
                    continue;
                }

                /* make sure that the even sysport entry is available */
                if (prev_sysport_state != BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
                    continue;
                }

                p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
		p_spstate->flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
                p_spgstate->used++;
            }
        }
        else {
            /* If individual flow control is enabled 2 sysports need to be allocated.          */
            /* The two sysports allocated are consecutive and this aids in supporting warmboot */
            /* The symantics of the API remain the same. reserve alternate/odd sysports.       */
            /* Only even sysports will be allocated. The odd sysport usage goes along with the */
            /* even sysport. The sysport groups (for odd sysyports) will be marked as not      */
            /* available. These will be all odd sysport groups.                                */

            prev_sysport_state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;

            for (sysport = 0; sysport < num_sysports; sysport++) {
                p_spstate = &sysport_state[unit][sysport];

                /* skip even sysports */
                if ((sysport % 2) == 0) {
                    prev_sysport_state = p_spstate->state;
                    continue;
                }

                /* skip sysports reserved for ESETs */
                if (p_spstate->state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET) {
                    continue;
                }

                /* make sure that the odd sysport is available */
                if (prev_sysport_state != BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
                    continue;
                }

                p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
            }

            for (sysport_group = 0; sysport_group < BCM_INT_SBX_MAX_SYSPORT_GROUP; sysport_group++) {
                p_spgstate = &sysport_group_state[unit][sysport_group];

                /* skip even sysport groups */
                if ((sysport_group % 2) == 0) {
                    continue;
                }

                /* skip sysport groups reserved for ESETs */
                if (p_spgstate->state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_ESET) {
                    continue;
                }

                /* mark the group as used */
                p_spgstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
                p_spgstate->size = num_sysports / BCM_INT_SBX_MAX_SYSPORT_GROUP;
                p_spgstate->used = p_spgstate->size;
	        sal_memset(&p_spgstate->node_cnt, 0xffffffff, sizeof(uint32) * 32);
	        sal_memset(&p_spgstate->node_port, 0xffffffff, sizeof(uint32) * 32);
            }
        }
    }

    
    if ( FALSE && 
	 ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
	  (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) ) {
	/* QE2000 + Polaris, or QE2000+Sirius+Polaris, need to reserve local sysport
	 * in hybrid mode
	 */
        if ( soc_feature(unit, soc_feature_hybrid) ) {
            _bcm_sbx_device_cosq_init(unit);

            for (sysport = 0; sysport < num_sysports; sysport++) {
                sysport_group = sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP;
                p_spstate = &sysport_state[unit][sysport];
                p_spgstate = &sysport_group_state[unit][sysport_group];

                if (sysport_state[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
                    if (!(_bcm_sbx_device_cosq_is_sysport_available(unit, sysport))) {
                        p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_LOCAL;
			p_spstate->flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
                        p_spgstate->used++;
                    }
                }

            }

            _bcm_sbx_device_cosq_deinit(unit);
        }
    }

    if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
	 (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
        /* reserve sysport 0 */
        sysport = 0;
        sysport_group = sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP;
        p_spstate = &sysport_state[unit][sysport];
        p_spgstate = &sysport_group_state[unit][sysport_group];
        
        if (sysport_state[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
            p_spstate->state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_GENERAL;
            p_spstate->flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
            p_spgstate->used++;
        }
    }

    return BCM_E_NONE;
}

static void
_bcm_sbx_cosq_sysport_state_detach(int unit)
{
    if (sysport_state[unit] != NULL) {
        sal_free(sysport_state[unit]);
        sysport_state[unit] = NULL;
    }

    if (sysport_group_state[unit] != NULL) {
        sal_free(sysport_group_state[unit]);
        sysport_group_state[unit] = NULL;
    }
}

/****************************************
 *  Initialize flow control domain state
 */

static int
_bcm_sbx_cosq_fcd_state_init(int unit)
{
    int fcd = 0, fcd_max = 0, idx = 1;
    bcm_sbx_cosq_fcd_state_t *p_fcdstate;

    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    idx = SOC_SBX_CFG(unit)->cfg_num_nodes;
	}
    }

    fcd_max = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3;
    
    /* Allocate flow control state array */
    fcd_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_fcd_state_t) * fcd_max * idx,
				"flow control domain state memory");
    
    if (fcd_state[unit] == NULL) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, sal_alloc flow control domain,  Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return(BCM_E_MEMORY);
    }
    
    for (fcd = 0; fcd < (fcd_max * idx); fcd++) {
	p_fcdstate = &fcd_state[unit][fcd];
	if (!soc_feature(unit, soc_feature_egr_independent_fc) && (fcd & 0x1)) {
	    p_fcdstate->in_use = FCD_BUSY;
	} else {
	    p_fcdstate->in_use = FCD_FREE;
	}
	p_fcdstate->ref_cnt = 0;
	p_fcdstate->fifo = -1;
	p_fcdstate->sysport = BCM_INT_SBX_INVALID_SYSPORT;
	p_fcdstate->node = -1;
	p_fcdstate->port = -1;
    }

    return BCM_E_NONE;
}


static void
_bcm_sbx_cosq_fcd_state_detach(int unit)
{
    if (fcd_state[unit] != NULL) {
	sal_free(fcd_state[unit]);
	fcd_state[unit] = NULL;
    }
}

/***********************************
 *  Initialize destport_state
 */
static int
_bcm_sbx_cosq_destport_state_init(int unit)
{
    int destport = 0;

    /*
     * Initialize dp_state to NULL. When dp_state is allocated to a pointer,
     * then it is considered active. Otherwise, when it is NULL, it should be ignored.
     */

    dp_state[unit] = NULL;

    if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) ||
	 (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
	 (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
	/* Allocate destport state array */
	if (SOC_IS_SBX_QE2000(unit)) {
	    dp_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_destport_state_t) * SBX_MAX_PORTS,
				       "destport state memory");
	    if (dp_state[unit] == NULL) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, sal_alloc,  Unit(%d)\n"),
		           FUNCTION_NAME(), unit));
		return(BCM_E_MEMORY);
	    }
	    
	    for (destport = 0; destport < SBX_MAX_PORTS; destport++) {
		dp_state[unit][destport].ref_cnt = 0;
	    }
	}
    }

    return BCM_E_NONE;
}

static void
_bcm_sbx_cosq_destport_state_detach(int unit)
{
    if (dp_state[unit] != NULL) {
        sal_free(dp_state[unit]);
        dp_state[unit] = NULL;
    }
}

/***********************************
 *  Get the WFQ weight, if none, return 0
 */
static uint32
_bcm_sbx_cosq_get_wfq_weight(int unit, int queue) {

    bcm_sbx_cosq_queue_state_t *p_qstate;

    p_qstate = &queue_state[unit][queue];

    if (p_qstate->state == BCM_INT_SBX_QUEUE_STATE_IN_USE) {
        switch ( p_qstate->ingress.bw_mode ) {
            case BCM_COSQ_AF:
                if ( SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE ) {
                    return p_qstate->ingress.bw_value.wfq_weight;
                }
                break;
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING :
                return p_qstate->ingress.bw_value.wfq_weight;
            default:
                break;
        }
    }
    return 0;
}
/***********************************
 *  Get the bandwidth available
 */
static int32
_bcm_sbx_cosq_bw_group_get_available_bw(int unit, int bw_group) {

    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 available_bw_kbps;
    int32 num_cos;
    int32 total_queue_bw_kbps = 0;
    int32 queue;
    int32 base_queue;

    p_bwstate = &bw_group_state[unit][bw_group];
    base_queue = p_bwstate->base_queue;
    num_cos = p_bwstate->num_cos;

    available_bw_kbps = p_bwstate->dest_port_kbits_sec_max;

    if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
         (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
         (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
        return available_bw_kbps;
    }

    for ( queue=base_queue; queue<base_queue+num_cos; queue++ ) {
        total_queue_bw_kbps += bcm_sbx_cosq_get_bw_guarantee(unit, queue);
    }
    available_bw_kbps -= total_queue_bw_kbps;

    return available_bw_kbps;
}
/***********************************
 * Allocate a bandwidth group
 * If auto_select is true, return the bw_group, otherwise, try to reserve
 * the bw_group number passed in.
 */
static int
_bcm_sbx_cosq_bw_group_allocate(int unit, int *p_bw_group, int auto_select)
{
    int rv = BCM_E_NONE;
    int bw_group;

    if (auto_select) {
        
        for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {
            if (bw_group_state[unit][bw_group].in_use == FALSE) {
                bw_group_state[unit][bw_group].in_use = TRUE;
                *p_bw_group = bw_group;
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "Allocated bw_group(%d)\n"),
                          bw_group));
                return rv;
            }
        }

    }else {
        bw_group = *p_bw_group;
        if (bw_group_state[unit][bw_group].in_use == FALSE) {
            bw_group_state[unit][bw_group].in_use = TRUE;
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "Allocated bw_group(%d)\n"),
                      bw_group));
            return rv;
        } else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "bw_group requested is already in use(%d)\n"),
                       bw_group));
            rv = BCM_E_RESOURCE;
        }
    }
    LOG_ERROR(BSL_LS_BCM_COMMON,
              (BSL_META_U(unit,
                          "There are no free bw_groups.  Resource error\n")));
    rv = BCM_E_RESOURCE;
    return rv;
}

/***********************************
 * Allocate a sysport
 */
static int
_bcm_sbx_cosq_sysport_allocate(int unit,
			       int flags,
                               int requested_sysport,
                               int dest_node,
			       int fabric_port,
			       int egroup_num,
                               bcm_sbx_cosq_queue_region_type_t queue_region,
                               int *p_sysport)
{
    int rv = BCM_E_NONE;
    int sysport, sysport_group, nef_sysport=0;
    uint min_use, min_spg;
    bcm_sbx_subport_info_t *sp_info = NULL;
    bcm_sbx_cosq_sysport_group_state_t *p_spg;
    int sysport_index;

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        /* System with Bm3200 + QE2000 */

	/* standalone mode (TME) don't need sysport allocated */
        if (soc_feature(unit, soc_feature_standalone)) {
            *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
            return(rv);
        }

        /* Sysport not supported in DMODE - FIC (global non- hybrid), Hybrid (Local) */
        if (!((queue_region == bcm_sbx_cosq_queue_region_global) &&
                           (soc_feature(unit, soc_feature_hybrid)))) { /* soc_feature_node_hybrid */

            /* Sysport not supported in DMODE */
            *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
            return(rv);
        }

        /* sysport is required in hybrid mode */
        for (sysport_index = 0; sysport_index < SOC_SBX_CFG(unit)->num_sysports; sysport_index++) {
            if (sysport_state[unit][sysport_index].state != BCM_INT_SBX_SYSPORT_STATE_GLOBAL) {
                continue;
            }
            if (sysport_state[unit][sysport_index].node != dest_node) {
                continue;
            }
	    if (sysport_state[unit][sysport_index].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
                continue;
            }
            if ( requested_sysport == BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT ) {
                sysport = BCM_INT_SBX_DMODE_SYSPORT_SET(sysport_state[unit][sysport_index].node,
                                                    sysport_state[unit][sysport_index].port);
                sysport_state[unit][sysport_index].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
                sysport_state[unit][sysport_index].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		sysport_state[unit][sysport_index].use_cnt++;
                sysport_state[unit][sysport_index].fcd = BCM_INT_SBX_INVALID_FCD;
		sysport_state[unit][sysport_index].fifo = -1;
		sysport_state[unit][sysport_index].egport = BCM_GPORT_INVALID;
               (*p_sysport) = sysport;
                break;
            }
            else {
                if ( (BCM_INT_SBX_DMODE_SYSPORT_NODE_GET(requested_sysport) ==
                                           sysport_state[unit][sysport_index].node) &&
                     (BCM_INT_SBX_DMODE_SYSPORT_PORT_GET(requested_sysport) ==
                                           sysport_state[unit][sysport_index].port) ) {
		    sysport_state[unit][sysport_index].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		    sysport_state[unit][sysport_index].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
                    sysport_state[unit][sysport_index].use_cnt++;
		    sysport_state[unit][sysport_index].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][sysport_index].fifo = -1;
		    sysport_state[unit][sysport_index].egport = BCM_GPORT_INVALID;
                    (*p_sysport) = requested_sysport;
                    break;
                }
            }
        }

        if (sysport_index >= SOC_SBX_CFG(unit)->num_sysports) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "sysport resource is not available, req_sysport(0x%x) node(%d)\n"),
                      requested_sysport, dest_node));
            return(BCM_E_RESOURCE);
        }

        return(rv);
    } else if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
		(SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
        /* System with Polaris + QE2000 and/or Sirius */

	/* standalone mode (TME) */
        if (soc_feature(unit, soc_feature_standalone)) {
	    if (SOC_IS_SBX_QE2000(unit)) {
		*p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
	    } else if (SOC_IS_SBX_SIRIUS(unit)) {
		/* TME mode, sysport is actually used in sirius local case
		 * just mark it as used even though the sysport is
		 * valid to this device only
		 */
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
		if ( soc_feature(unit, soc_feature_egr_independent_fc)) {
		    *p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd;
		} else {
		    *p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd/2;
		}
		if (*p_sysport >= SIRIUS_TS_LOCAL_SYSPORT_LIMIT) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "Invalid sysport %d\n"),
		               *p_sysport));
		    rv = BCM_E_INTERNAL;
		    return rv;
                }
		sysport_state[unit][*p_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		sysport_state[unit][*p_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		sysport_state[unit][*p_sysport].node = -1;
		sysport_state[unit][*p_sysport].port = -1;
		sysport_state[unit][*p_sysport].use_cnt++;
                sysport_state[unit][*p_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
                sysport_state[unit][*p_sysport].fifo = -1;
		sysport_state[unit][*p_sysport].egport = BCM_GPORT_INVALID;
	    }
	    return(rv);
        }

        /* Sysport not supported in Hybrid (Local) */
        if ((queue_region == bcm_sbx_cosq_queue_region_local) &&
            (soc_feature(unit, soc_feature_hybrid))) {
	    /* Hybrid case we have to reserve this sysport globally
	     * so that BME devices won't use it for VOQs
	     */
	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
	    if ( soc_feature(unit, soc_feature_egr_independent_fc)) {
		*p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd;
	    } else {
		*p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd/2;
	    }
	    if (*p_sysport >= SIRIUS_TS_LOCAL_SYSPORT_LIMIT) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "Invalid sysport %d\n"),
		           *p_sysport));
		rv = BCM_E_INTERNAL;
		return rv;
	    }
	    sysport_state[unit][*p_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
	    sysport_state[unit][*p_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
	    sysport_state[unit][*p_sysport].node = -1;
	    sysport_state[unit][*p_sysport].port = -1;
	    sysport_state[unit][*p_sysport].use_cnt++;
	    sysport_state[unit][*p_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
	    sysport_state[unit][*p_sysport].fifo = -1;
	    sysport_state[unit][*p_sysport].egport = BCM_GPORT_INVALID;
            return(rv);
        }

        if ( requested_sysport == BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT ) {
            /*
             * (1) all sysport reserved for eset already marked as in use
             * pick up available
             * (2) In FIC mode, since only port[5:0] are pass to QE2000 by BM9600
             * Sysport low, the sysport is divided into equal size groups,
             * within a sysport group, a particular node can only allocate
             * one sysport.
             * (3) To evenly distribute the sysport allocation among the
             * sysport groups, we always try to allocate the sysport
             * in the group used least number of sysports, then do
             * port remap on QE2000 side to map the sysport bit[5:0]
             * to the real port requested by the application.
             */

            /* scan the sysport group state to find the group with least number
             * of sysport used, and has not allocate a port for the requesting
             * node.
             */
            min_use = 0xFFFF;
            min_spg = 0xFFFF;
            for (sysport_group = 0; sysport_group < BCM_INT_SBX_MAX_SYSPORT_GROUP;
                 sysport_group++) {
                p_spg = &sysport_group_state[unit][sysport_group];
		/*
		 * dp_state exists only for Qe2k devices, otherwise check mask
		 */
		if (dp_state[unit] != NULL) {
		    /*
		     * upper bit of ref_cnt is set when allocating sysport from 
		     * overlay. Therefore, do not check node_cnt.
		     */
		    if ((dp_state[unit][fabric_port].ref_cnt & BCM_INT_SBX_DP_IGNORE) != 0) {
			if (p_spg->node_cnt[dest_node] > 0) {
			    if ((p_spg->node_port[dest_node] != fabric_port) ||
				((p_spg->node_port[dest_node] == fabric_port) && 
				 (p_spg->node_cnt[dest_node] > BCM_INT_SBX_NODE_PORT_THRESHOLD))) {
				continue;
			    }
			    min_spg = sysport_group;
			    break;
			}
		    } else {
			/* if the node already allocated from the group, don't use this group */
			if ( p_spg->node_cnt[dest_node] > 0 ) {
			    continue;
			}
		    }
		} else {
		    /* if the node already allocated from the group, don't use this group */
		    if ( p_spg->node_cnt[dest_node] > 0 ) {
			continue;
		    }
		}
                /* find the group with minimum number of sysport allocated */
                if (min_use > p_spg->used) {
                    min_use = p_spg->used;
                    min_spg = sysport_group;
                }
            }

            /* If found a sysport group and the sysport group is not used up */
            if ( (min_spg != 0xFFFF) &&
                 (sysport_group_state[unit][min_spg].used != sysport_group_state[unit][min_spg].size) ) {
                /* Found a qualified sysport group with minimum sysport usage */
                for (sysport = min_spg; sysport < SOC_SBX_CFG(unit)->num_sysports;
                     sysport += BCM_INT_SBX_MAX_SYSPORT_GROUP) {
		    if ((sysport_state[unit][sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) {
                        /* Allocate this sysport */
                        *p_sysport = sysport;
			sysport_state[unit][sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
			sysport_state[unit][sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
			sysport_state[unit][sysport].node = -1;
                        sysport_state[unit][sysport].port = -1;
			sysport_state[unit][sysport].use_cnt++;
			sysport_state[unit][sysport].fcd = BCM_INT_SBX_INVALID_FCD;
			sysport_state[unit][sysport].fifo = -1;
			sysport_state[unit][sysport].egport = BCM_GPORT_INVALID;

                        p_spg = &sysport_group_state[unit][sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP];
                        p_spg->used++;
			if (p_spg->node_cnt[dest_node] == 0) {
			    p_spg->node_port[dest_node] = fabric_port;
			}

			if (dp_state[unit] != NULL) {
			    if (dp_state[unit][fabric_port].ref_cnt & BCM_INT_SBX_DP_IGNORE) {
				/*
				 * When reusing the node / port, 
				 * copy corresponding information directly into 
				 * sysport_state
				 */
				
				sysport_state[unit][sysport].node = dest_node;
				sysport_state[unit][sysport].port = p_spg->node_port[dest_node];
			    }
			}
			p_spg->node_cnt[dest_node]++;
			if (dp_state[unit] != NULL) {
			    dp_state[unit][fabric_port].ref_cnt++;
			}

                        LOG_INFO(BSL_LS_BCM_COSQ,
                                 (BSL_META_U(unit,
                                             "Allocated sysport(%d) for node(%d) in sysport group(%d)\n"),
                                  sysport, dest_node, min_spg));

                        return rv;
                    }
                }
            }

        } else {

            /* if requested an sysport, check if it's available */
	    if ((sysport_state[unit][requested_sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) {
		sysport_state[unit][requested_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		sysport_state[unit][requested_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
                sysport_state[unit][requested_sysport].node = -1;
                sysport_state[unit][requested_sysport].port = -1;
		sysport_state[unit][requested_sysport].use_cnt++;
                sysport_state[unit][requested_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
                sysport_state[unit][requested_sysport].fifo = -1;
		sysport_state[unit][requested_sysport].egport = BCM_GPORT_INVALID;
                *p_sysport = requested_sysport;

                /* sysport group is identified by the lower bit[5:0] of sysport
                 * increment the usage count and mark the node usage
                 */
                p_spg = &sysport_group_state[unit][requested_sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP];
                p_spg->used++;
		if (p_spg->node_cnt[dest_node] == 0) {
		    p_spg->node_port[dest_node] = fabric_port;
		} else {
		    /*
		     * When reusing the node / port after queue creation, 
		     * copy corresponding information directly into 
		     * sysport_state
		     */
		    sysport_state[unit][requested_sysport].node = dest_node;
		    sysport_state[unit][requested_sysport].port = p_spg->node_port[dest_node];
		}

		p_spg->node_cnt[dest_node]++;
		if (dp_state[unit] != NULL) {
		    dp_state[unit][fabric_port].ref_cnt++;
		}
			
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "Requested sysport(%d) available\n"),
                          requested_sysport));
		
                return rv;
            } else {
                *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "requested sysport(%d) is not available\n"),
                          requested_sysport));
                rv = BCM_E_RESOURCE;
                return rv;
            }
        }
    } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
        /* System with Polaris + Sirius */
	if (queue_region == bcm_sbx_cosq_queue_region_local) {
	    /* Hybrid mode, we have to reserve this sysport globally
	     * so that BME devices won't use it for VOQs
	     */
	    /* TME mode, sysport is actually used in sirius local case
	     * just mark it as used even though the sysport is
	     * valid to sirius only
	     */
	    if ((flags & BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE) == 0) {
		sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
		if ( soc_feature(unit, soc_feature_egr_independent_fc)) {
		    *p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd;
		} else {
		    *p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd/2;
		}
		if (*p_sysport >= SIRIUS_TS_LOCAL_SYSPORT_LIMIT) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "Invalid sysport %d\n"),
		               *p_sysport));
		    rv = BCM_E_INTERNAL;
		    return rv;
                }
		sysport_state[unit][*p_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		sysport_state[unit][*p_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		sysport_state[unit][*p_sysport].node = -1;
		sysport_state[unit][*p_sysport].port = -1;
		sysport_state[unit][*p_sysport].use_cnt++;
		sysport_state[unit][*p_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
		sysport_state[unit][*p_sysport].fifo = -1;
		sysport_state[unit][*p_sysport].egport = BCM_GPORT_INVALID;
	    } else if ( requested_sysport == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT ) {
		/*
		 * Allocation the first free sysport in the TS_LOCAL_SYSPORT range
		 */
		for (sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + 256; 
		     sysport < SIRIUS_TS_LOCAL_SYSPORT_LIMIT; sysport++) {
		    if (sysport_state[unit][sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
			/* if already used, go to next */
			continue;
		    }

		    /* Allocate this sysport */
		    *p_sysport = sysport;
		    sysport_state[unit][sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		    sysport_state[unit][sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		    sysport_state[unit][sysport].node = -1;
		    sysport_state[unit][sysport].port = -1;
		    sysport_state[unit][sysport].use_cnt++;
		    sysport_state[unit][sysport].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][sysport].fifo = -1;
		    sysport_state[unit][sysport].egport = BCM_GPORT_INVALID;

		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Allocated sysport(%d) for node(%d)\n"),
		              sysport, dest_node));
		    rv = BCM_E_NONE;
		    return rv;
		}
	    } else {
		if ((requested_sysport < SIRIUS_TS_LOCAL_SYSPORT_BASE) ||
		    (requested_sysport >= SIRIUS_TS_LOCAL_SYSPORT_LIMIT)) {
		    *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "requested sysport(%d) out of range\n"),
		              requested_sysport));
		    rv = BCM_E_RESOURCE;
		} else if ((sysport_state[unit][requested_sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) {
		    /* if requested a sysport, check if it's available */
		    sysport_state[unit][requested_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		    sysport_state[unit][requested_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		    sysport_state[unit][requested_sysport].node = -1;
		    sysport_state[unit][requested_sysport].port = -1;
		    sysport_state[unit][requested_sysport].use_cnt++;
		    sysport_state[unit][requested_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][requested_sysport].fifo = -1;
		    sysport_state[unit][requested_sysport].egport = BCM_GPORT_INVALID;
		    *p_sysport = requested_sysport;

		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "Requested sysport(%d) available\n"),
		              requested_sysport));
		    rv = BCM_E_NONE;
		} else {
		    *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "requested sysport(%d) is not available\n"),
		              requested_sysport));
		    rv = BCM_E_RESOURCE;
		}
	    }
            return(rv);
        }

        if ( requested_sysport == BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT ) {
            /*
             * VPORT mode has no restrictions on sysport allocation, just
             * allocation the first free sysport
             */
            for (sysport = 0; sysport < SOC_SBX_CFG(unit)->num_sysports; sysport++) {
		if (sysport_state[unit][sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
  		    /* if already used, go to next */
		    continue;
		}
		nef_sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);

		if (soc_feature(unit, soc_feature_egr_independent_fc) && (sysport == nef_sysport))  {
		    /* if independent flow control and Non-EF sysports, go to next or 
		     * If nef sysport in fc pair, and not reserved for fc pair, then skip */
		    if ((sysport_state[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC) ||
			((sysport_state[unit][sysport].state == BCM_INT_SBX_SYSPORT_STATE_GLOBAL) &&
			 ((flags & BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE) == 0))) {
			continue;
		    }
		}



		/* Allocate this sysport */
		*p_sysport = sysport;
		sysport_state[unit][sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		sysport_state[unit][sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		sysport_state[unit][sysport].node = -1;
		sysport_state[unit][sysport].port = -1;
		sysport_state[unit][sysport].use_cnt++;
                sysport_state[unit][sysport].fcd = BCM_INT_SBX_INVALID_FCD;
                sysport_state[unit][sysport].fifo = -1;
		sysport_state[unit][sysport].egport = BCM_GPORT_INVALID;

		/* if independent flow control and Non-EF sysport, make it available 
		 * for allocation when EF sysport taken as single */
		if (soc_feature(unit, soc_feature_egr_independent_fc) && 
		    ((flags & BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE) == 1) &&
		    (sysport_state[unit][nef_sysport].state == BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC) &&
		    ((sysport_state[unit][nef_sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) &&
		    (sysport != BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport))) {
		    sysport_state[unit][nef_sysport].state = BCM_INT_SBX_SYSPORT_STATE_GLOBAL;
		}
		
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Allocated sysport(%d) for node(%d)\n"),
		          sysport, dest_node));
		return rv;
            }
        } else {
            /* if requested an sysport, check if it's available */
            if ((sysport_state[unit][requested_sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) {
		sysport_state[unit][requested_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		sysport_state[unit][requested_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
                sysport_state[unit][requested_sysport].node = -1;
                sysport_state[unit][requested_sysport].port = -1;
		sysport_state[unit][requested_sysport].use_cnt++;
                sysport_state[unit][requested_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
                sysport_state[unit][requested_sysport].fifo = -1;
		sysport_state[unit][requested_sysport].egport = BCM_GPORT_INVALID;
                *p_sysport = requested_sysport;

                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "Requested sysport(%d) available\n"),
                          requested_sysport));
                return rv;
            } else {
                *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "requested sysport(%d) is not available\n"),
                          requested_sysport));
                rv = BCM_E_RESOURCE;
                return rv;
            }
        }
    } else if (soc_feature(unit, soc_feature_standalone)) {
        /* standalone mode (TME) */
            if (SOC_IS_SBX_QE2000(unit)) {
                *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
            } else if (SOC_IS_SBX_SIRIUS(unit)) {
		
		/* TME mode, sysport is actually used in sirius local case.
		 * The sysport is valid to this device only
		 */
		if ((flags & BCM_INT_SBX_SYSPORT_FLAGS_FC_SINGLE) == 0) {
		    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[fabric_port]);
		    if ( soc_feature(unit, soc_feature_egr_independent_fc)) {
			*p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd;
		    } else {
			*p_sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + sp_info->egroup[egroup_num].ef_fcd/2;
		    }		    		    
		    if (*p_sysport >= SIRIUS_TS_LOCAL_SYSPORT_LIMIT) {
			LOG_ERROR(BSL_LS_BCM_COMMON,
			          (BSL_META_U(unit,
			                      "Invalid sysport %d\n"),
			           *p_sysport));
			rv = BCM_E_INTERNAL;
			return rv;
		    }
		    sysport_state[unit][*p_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
		    sysport_state[unit][*p_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
		    sysport_state[unit][*p_sysport].node = -1;
		    sysport_state[unit][*p_sysport].port = -1;
		    sysport_state[unit][*p_sysport].use_cnt++;
		    sysport_state[unit][*p_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
		    sysport_state[unit][*p_sysport].fifo = -1;
		    sysport_state[unit][*p_sysport].egport = BCM_GPORT_INVALID;
		} else if ( requested_sysport == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT ) {
		    /*
		     * Allocation the first free sysport in the TS_LOCAL_SYSPORT range
		     */
		    for (sysport = SIRIUS_TS_LOCAL_SYSPORT_BASE + 256; 
			 sysport < SIRIUS_TS_LOCAL_SYSPORT_LIMIT; sysport++) {
			if (sysport_state[unit][sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
			    /* if already used, go to next */
			    continue;
			}

			/* Allocate this sysport */
			*p_sysport = sysport;
			sysport_state[unit][sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
			sysport_state[unit][sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
			sysport_state[unit][sysport].node = -1;
			sysport_state[unit][sysport].port = -1;
			sysport_state[unit][sysport].use_cnt++;
			sysport_state[unit][sysport].fcd = BCM_INT_SBX_INVALID_FCD;
			sysport_state[unit][sysport].fifo = -1;
			sysport_state[unit][sysport].egport = BCM_GPORT_INVALID;

			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Allocated sysport(%d) for node(%d)\n"),
			          sysport, dest_node));
			rv = BCM_E_NONE;
			return rv;
		    }
		} else {
		    if ((requested_sysport < SIRIUS_TS_LOCAL_SYSPORT_BASE) ||
			(requested_sysport >= SIRIUS_TS_LOCAL_SYSPORT_LIMIT)) {
			*p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "requested sysport(%d) out of range\n"),
			          requested_sysport));
			rv = BCM_E_RESOURCE;
		    } else if ((sysport_state[unit][requested_sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) {
			/* if requested a sysport, check if it's available */
			sysport_state[unit][requested_sysport].flags |= BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE;
			sysport_state[unit][requested_sysport].flags &= ~BCM_INT_SBX_SYSPORT_STATE_FLAGS_OLAY_DONE;
			sysport_state[unit][requested_sysport].node = -1;
			sysport_state[unit][requested_sysport].port = -1;
			sysport_state[unit][requested_sysport].use_cnt++;
			sysport_state[unit][requested_sysport].fcd = BCM_INT_SBX_INVALID_FCD;
			sysport_state[unit][requested_sysport].fifo = -1;
			sysport_state[unit][requested_sysport].egport = BCM_GPORT_INVALID;
			*p_sysport = requested_sysport;

			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "Requested sysport(%d) available\n"),
			          requested_sysport));
			rv = BCM_E_NONE;
		    } else {
			*p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
			LOG_INFO(BSL_LS_BCM_COSQ,
			         (BSL_META_U(unit,
			                     "requested sysport(%d) is not available\n"),
			          requested_sysport));
			rv = BCM_E_RESOURCE;
		    }
		}
            }
            return(rv);
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unsupported fabric system mode\n")));
    }

    LOG_ERROR(BSL_LS_BCM_COMMON,
              (BSL_META_U(unit,
                          "There are no free sysports.  Resource error\n")));
    *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
    rv = BCM_E_RESOURCE;
    return rv;
}

static int
_bcm_sbx_cosq_sysport_deallocate(int unit, int sysport)
{
    int rv = BCM_E_NONE;
    int sysport_index, nef_sysport = 0;
    int dest_node;
    int fabric_port = 0xff;

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if (sysport == BCM_INT_SBX_INVALID_SYSPORT) {
            return(rv);
        }

        /* Sysport not supported in DMODE - TME */
        if (soc_feature(unit, soc_feature_standalone)) { /* bcm_sbx_cosq_queue_region_local */
            return(rv);
        }

        if (!(soc_feature(unit, soc_feature_hybrid))) {
            return(rv);
        }

#if 0
        /* Sysport not supported in DMODE - FIC (global non-hybrid), Hybrid (Local) */
        if (!((queue_region == bcm_sbx_cosq_queue_region_global) &&
                           (soc_feature(unit, soc_feature_hybrid)))) { /* soc_feature_node_hybrid */
            return(rv);

        }
#endif /* 0 */

        /* sysport is required in hybrid mode */
        for (sysport_index = 0; sysport_index < SOC_SBX_CFG(unit)->num_sysports; sysport_index++) {
            if ( (BCM_INT_SBX_DMODE_SYSPORT_NODE_GET(sysport) ==
                                           sysport_state[unit][sysport_index].node) &&
                     (BCM_INT_SBX_DMODE_SYSPORT_PORT_GET(sysport) ==
                                           sysport_state[unit][sysport_index].port) ) {
                /* mark sysport group state for the node */
                dest_node = sysport_state[unit][sysport_index].node;

		if (sysport_state[unit][sysport_index].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
		    /*
		     * Only update these fields when in use
		     */
		    sysport_group_state[unit][(sysport_index % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_cnt[dest_node]--;
		    
		    if (dp_state[unit] != NULL) {
			if (sysport_group_state[unit][(sysport_index % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_port[dest_node] != 0xff) {
			    fabric_port = sysport_group_state[unit][(sysport_index % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_port[dest_node];
			    dp_state[unit][fabric_port].ref_cnt--;
			}
		    }
		    if (sysport_group_state[unit][(sysport_index % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_cnt[dest_node] == 0) {
			sysport_group_state[unit][(sysport_index % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_port[dest_node] = 0xff;
		    }
		    
                    sysport_group_state[unit][(sysport_index % BCM_INT_SBX_MAX_SYSPORT_GROUP)].used--;
                }

                /* mark sysport state as not in use */
		sysport_state[unit][sysport_index].flags = 0;
                sysport_state[unit][sysport_index].node = -1;
                sysport_state[unit][sysport_index].port = -1;
		sysport_state[unit][sysport_index].use_cnt--;
                sysport_state[unit][sysport_index].fcd = BCM_INT_SBX_INVALID_FCD;
                sysport_state[unit][sysport_index].fifo = -1;
		sysport_state[unit][sysport_index].egport = BCM_GPORT_INVALID;
                break;
            }
        }

        if (sysport_index >= SOC_SBX_CFG(unit)->num_sysports) {
            rv = BCM_E_INTERNAL;
        }

        return(rv);
    }

    /* mode other then SOC_SBX_SYSTEM_CFG_DMODE */

    
    if (sysport == BCM_INT_SBX_INVALID_SYSPORT) {
        return(rv);
    }

    /* Sysport not supported in TME */
    if (soc_feature(unit, soc_feature_standalone)) { /* bcm_sbx_cosq_queue_region_local */
        return(rv);
    }

#if 0
    /* Sysport not supported in Hybrid (Local) */
    if ((queue_region == bcm_sbx_cosq_queue_region_local) &&
                                                  (soc_feature(unit, soc_feature_hybrid))) {
        *p_sysport = BCM_INT_SBX_INVALID_SYSPORT;
        return(rv);
    }
#endif /* 0 */

    if ( (sysport >= 0) &&
         (sysport < SOC_SBX_CFG(unit)->num_sysports) ) {
        if ( (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
	     (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) ) {
            /* mark sysport group state for the node, only required when sysports
             * are allocated in groups
             */

	    
            dest_node = sysport_state[unit][sysport].node;

	    if (sysport_state[unit][sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
		/*
		 * Only update fields when in use
		 */
		sysport_group_state[unit][(sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_cnt[dest_node]--;

		if (dp_state[unit] != NULL) {
		    if (sysport_group_state[unit][(sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_port[dest_node] != 0xff) {
			fabric_port = sysport_group_state[unit][(sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_port[dest_node];
			dp_state[unit][fabric_port].ref_cnt--;
		    }
		}

		if (sysport_group_state[unit][(sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_cnt[dest_node] == 0) {
		    sysport_group_state[unit][(sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP)].node_port[dest_node] = 0xff;
		}

                sysport_group_state[unit][(sysport % BCM_INT_SBX_MAX_SYSPORT_GROUP)].used--;
            }

        }
	if (sysport_state[unit][sysport].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) {
            /* make sysport state as not in use */
            sysport_state[unit][sysport].flags = 0;
            sysport_state[unit][sysport].node = -1;
            sysport_state[unit][sysport].port = -1;
	    sysport_state[unit][sysport].use_cnt--;
	    sysport_state[unit][sysport].fcd = BCM_INT_SBX_INVALID_FCD;
	    sysport_state[unit][sysport].fifo = -1;
	    sysport_state[unit][sysport].egport = BCM_GPORT_INVALID;

	    /* If in independent fc mode and the even sysport of a pair is freed,
	     * reset odd member to join pair again in fifo_fc mode if available. */
	    if ( soc_feature(unit, soc_feature_egr_independent_fc)) {
		nef_sysport = BCM_INT_SBX_SYSPORT_TO_NEF_SYSPORT(unit, sysport);
		if (sysport != nef_sysport) {
		    if (((sysport_state[unit][nef_sysport].flags & 
			  BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) &&
			(sysport_state[unit][nef_sysport].state != BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC)) {
			sysport_state[unit][nef_sysport].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
		    }
		} else {
		    /* When freeing the odd of the pair, reset it to available if the even of the pair
		     * is already freed */
		    if ((sysport_state[unit][((sysport>>1)<<1)].flags & 
			 BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == FALSE) {
			sysport_state[unit][nef_sysport].state = BCM_INT_SBX_SYSPORT_STATE_RESERVED_FIFO_FC;
		    }
		}
	    }
        }
    } else {
        rv = BCM_E_INTERNAL;
    }

    return(rv);
}

/***********************************
 * Allocate a flow control domain
 */
static int
_bcm_sbx_cosq_fcd_allocate(int unit,
			   int flags,
			   int requested_fcd,
			   int sysport,
			   int dest_node,
			   int fabric_port,
			   int fifo,
			   int *p_fcd)
{
    int rv = BCM_E_NONE;
    int fcd = BCM_INT_SBX_INVALID_FCD;
    int idx = 0;

    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    if (dest_node >= SOC_SBX_CFG(unit)->cfg_num_nodes) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Node (%d) exceeds num_nodes (%d),  Unit(%d)\n"),
		           FUNCTION_NAME(), unit, dest_node, SOC_SBX_CFG(unit)->cfg_num_nodes));
		return BCM_E_PARAM;
	    }
	    /* Extra 3 domains added for Multicast and accounting */
	    idx = dest_node * (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3);
	}
    }

    if ( requested_fcd == BCM_INT_SBX_REQUESTED_FCD_AUTO_SELECT ) {
	
	/* scan the flow control domain state to find an available fcd
	 */

	for (fcd = 0; fcd < (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS - 2); fcd++) {
	    if (fcd_state[unit][idx + fcd].in_use == FCD_FREE) {
		/* Allocate this flow control domain */
		*p_fcd = fcd;
		if (flags == FCD_ALLOC) {
		    fcd_state[unit][idx + fcd].in_use = FCD_ALLOC;
		} else {
		    fcd_state[unit][idx + fcd].in_use = FCD_TAKE;
		}
		fcd_state[unit][idx + fcd].ref_cnt++;
		fcd_state[unit][idx + fcd].fifo = fifo;
		fcd_state[unit][idx + fcd].sysport = sysport;
		fcd_state[unit][idx + fcd].node = dest_node;
		fcd_state[unit][idx + fcd].port = fabric_port;
		
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "Allocated fcd(%d) for node(%d)\n"),
		          fcd, dest_node));
		
		return rv;
	    }
	}
    } else if ((requested_fcd == SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) || 
	       (requested_fcd == (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS - 1))) {
	/* Multicast FCDs are fixed */
	fcd_state[unit][idx + requested_fcd].in_use = FCD_TAKE;
	fcd_state[unit][idx + requested_fcd].ref_cnt++;
	fcd_state[unit][idx + requested_fcd].fifo = -1;
	fcd_state[unit][idx + requested_fcd].sysport = -1;
	fcd_state[unit][idx + requested_fcd].node = -1;
	fcd_state[unit][idx + requested_fcd].port = -1;
	*p_fcd = requested_fcd;
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "Requested flow control domain(%d) available\n"),
	          requested_fcd));
		
	return rv;
    } else {
	/* if requested an fcd, check if it's available */
	if (fcd_state[unit][idx + requested_fcd].in_use == FCD_FREE) {
	    if (flags == FCD_ALLOC) {
		fcd_state[unit][idx + requested_fcd].in_use = FCD_ALLOC;
	    } else {
		fcd_state[unit][idx + requested_fcd].in_use = FCD_TAKE;
	    }
	    fcd_state[unit][idx + requested_fcd].ref_cnt++;
	    fcd_state[unit][idx + requested_fcd].fifo = fifo;
	    fcd_state[unit][idx + requested_fcd].sysport = sysport;
	    fcd_state[unit][idx + requested_fcd].node = dest_node;
	    fcd_state[unit][idx + requested_fcd].port = fabric_port;
	    *p_fcd = requested_fcd;

	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "Requested flow control domain(%d) available\n"),
	              requested_fcd));
		
	    return rv;
	} else {
	    *p_fcd = BCM_INT_SBX_INVALID_FCD;
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "requested fcd(%d) is not available\n"),
	              requested_fcd));
	    rv = BCM_E_RESOURCE;
	    return rv;
	}
    }

    LOG_ERROR(BSL_LS_BCM_COMMON,
              (BSL_META_U(unit,
                          "There are no free flow control domains.  Resource error\n")));
    *p_fcd = BCM_INT_SBX_INVALID_FCD;
    rv = BCM_E_RESOURCE;
    return rv;
}

int
bcm_sbx_cosq_fcd_get_from_fifo(int unit, int fifo, int *p_fcd, int node)
{
    int rv = BCM_E_NONE;
    int fcd = BCM_INT_SBX_INVALID_FCD;
    int idx = 0;

    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    if (node >= SOC_SBX_CFG(unit)->cfg_num_nodes) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Node (%d) exceeds num_nodes (%d),  Unit(%d)\n"),
		           FUNCTION_NAME(), unit, node, SOC_SBX_CFG(unit)->cfg_num_nodes));
		return BCM_E_PARAM;
	    }
	    idx = node * (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3);
	}
    }
    
    for (fcd = 0; fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS; fcd++) {
	if ((fcd_state[unit][idx + fcd].in_use != FCD_FREE) && 
	    (fcd_state[unit][idx + fcd].fifo == fifo)) {
	    *p_fcd = fcd;
	    return rv;
	}
    }
    return BCM_E_NOT_FOUND;
}

static int
_bcm_sbx_cosq_fcd_free(int unit, int fcd, int node)
{
    int rv = BCM_E_NONE;
    int idx = 0;

    if (fcd_state[unit] == NULL) {
	return BCM_E_INIT;
    }

    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    if (node >= SOC_SBX_CFG(unit)->cfg_num_nodes) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Node (%d) exceeds num_nodes (%d),  Unit(%d)\n"),
		           FUNCTION_NAME(), unit, node, SOC_SBX_CFG(unit)->cfg_num_nodes));
		return BCM_E_PARAM;
	    }
	    idx = node * (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3);
	}
    }
    
    if ( (fcd >= 0) && (fcd < SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS) ) {
	
        if ((fcd_state[unit][idx + fcd].in_use != FCD_FREE) && (fcd_state[unit][idx + fcd].ref_cnt == 1)) {
            /* make fcd state as not in use */
            fcd_state[unit][idx + fcd].in_use = FCD_FREE;
            fcd_state[unit][idx + fcd].ref_cnt = 0;
            fcd_state[unit][idx + fcd].fifo = -1;
            fcd_state[unit][idx + fcd].sysport = BCM_INT_SBX_INVALID_SYSPORT;
            fcd_state[unit][idx + fcd].node = -1;
            fcd_state[unit][idx + fcd].port = -1;
	} else {
            fcd_state[unit][idx + fcd].ref_cnt--;
	    rv = BCM_E_BUSY;
	}
    } else {
	if (fcd > (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS)) {
	    rv = BCM_E_PARAM;
	}
    }

    return(rv);
}

int
_bcm_sbx_cosq_queue_type_get(int unit, int32 queue, bcm_sbx_cosq_queue_region_type_t *queue_region)
{
    int rc = BCM_E_NONE;
    int region;


    for (region = 0; region < bcm_sbx_cosq_queue_region_last; region++) {
        switch(region) {
            case bcm_sbx_cosq_queue_region_global:
            case bcm_sbx_cosq_queue_region_local:
               if ((queue_regions[unit] + region)->is_valid == FALSE) {
                   continue;
               }
               if ( (queue >= (queue_regions[unit] + region)->start) &&
                        (queue < (queue_regions[unit] + region)->end) ) {
                   (*queue_region) = region;
                   return(rc);
               }
               break;

            /* coverity[dead_error_begin] */
            default:
                return(BCM_E_INTERNAL);
                break;
        }
    }

    if (region > bcm_sbx_cosq_queue_region_last) {
        return(BCM_E_INTERNAL);
    }

    return(rc);
}

/***********************************
 * Allocate a queue group
 */
static int
_bcm_sbx_cosq_queue_group_allocate(int unit,
                                   int32 requested_base_queue,
                                   int32 requested_sysport,
                                   int32 node,
                                   int32 port,
                                   int32 mc,
                                   int32 num_cos,
				   int32 egroup_num,
                                   int32 dest_type,
                                   int32 scan_up,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
                                   int32 *p_allocated_base_queue,
                                   int32 *p_allocated_sysport,
                                   int32 cosq_init_added) {

    int32 queue = 0, end_queue = 0;
    int32 num_avail = 0;
    int32 found_base_queue = FALSE;
    int32 done;
    int32 base_queue, queue_base = 0, arb_port_base_queue;
    int32 bw_group;
    int32 eset = 0;
    int32 rv;
    bcm_sbx_cosq_queue_region_t *queue_region_p;
    int32 shaper_start, shaper_end;
    int32 no_scan = FALSE, double_scan = FALSE, double_scan_start = 0, double_scan_end = 0;
#ifdef BCM_EASY_RELOAD_SUPPORT
    bcm_sbx_cosq_bw_group_state_cache_t *p_bwstate_cache;
    uint32 base_queue_cached;
#endif
    queue_region_p = (queue_regions[unit] + queue_region);
    if (queue_region_p->is_valid != TRUE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Requested base region (%d) not valid\n"),
                   queue_region));
        rv = BCM_E_RESOURCE;
        return rv;
    }

    *p_allocated_base_queue = -1;
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Allocating queue: requested_base_queue=%d\n"),
              requested_base_queue));

    if ((requested_base_queue != BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) &&
	(requested_base_queue != BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING) &&
	(requested_base_queue != BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING) &&
        ((requested_base_queue < queue_region_p->start) ||
	 (requested_base_queue > (queue_region_p->end - 1)))) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Requested base queue out of range(%d)\n"),
                   requested_base_queue));
        rv = BCM_E_RESOURCE;
        return rv;
    }

    if ((requested_sysport != BCM_INT_SBX_REQUESTED_SYSPORT_AUTO_SELECT) &&
        (dest_type == BCM_INT_SBX_DEST_TYPE_UNICAST) &&
        (requested_sysport > (SOC_SBX_CFG(unit)->num_sysports -1))) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Requested sysport out of range(%d)\n"),
                   requested_sysport));
        rv = BCM_E_RESOURCE;
        return rv;
    }

    if ((queue_region == bcm_sbx_cosq_queue_region_global) &&
	(SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX)) {
      if (soc_feature(unit, soc_feature_hybrid)) {
	queue_base += ((node * SBX_MAX_PORTS) << (_BITS(num_cos)));
      } else {
	queue_base += ((node * SBX_MAX_PORTS + (port)) << (_BITS(num_cos)));
      }
    }

    /* Allocate base queue */
    if ( requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT ) {
        if ( scan_up ) {
            queue = queue_region_p->start + queue_base; /* scan starting at 0 for xgs */
	    end_queue = queue_region_p->end;
        } else {
            queue = queue_region_p->end - 1; /* scan down from highest */
	    end_queue = queue_region_p->start;
        }
    } else if (( requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING ) ||
	       ( requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING )) {
	if (queue_region == bcm_sbx_cosq_queue_region_global) {
	    rv = bcm_fabric_control_get(unit, bcmFabricShaperQueueMin, &shaper_start);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	    rv = bcm_fabric_control_get(unit, bcmFabricShaperQueueMax, &shaper_end);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	} else {
	    rv = bcm_fabric_control_get(unit, bcmFabricShaperEgressQueueMin, &shaper_start);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	    rv = bcm_fabric_control_get(unit, bcmFabricShaperEgressQueueMax, &shaper_end);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}

	if ( requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING ) {
	    /* calculate the queue region that overlaps with shaper region */
	    if ( scan_up ) {	    
		queue = shaper_start;
		end_queue = shaper_end;
		if (queue < queue_region_p->start + queue_base) {
		    queue = queue_region_p->start + queue_base;
		}
		if (end_queue > queue_region_p->end) {
		    end_queue = queue_region_p->end;
		}
		if (queue > end_queue) {
		    no_scan = TRUE;
		}
	    } else {
		queue = shaper_end;
		end_queue = shaper_start;
		if (queue > (queue_region_p->end - 1)) {
		    queue = queue_region_p->end - 1;
		}
		if (end_queue < queue_region_p->start) {
		    end_queue = queue_region_p->start;
		}
		if (queue < end_queue) {
		    no_scan = TRUE;
		}
	    }
	} else {
	    /* calculate the queue region that excludes shaper region */
	    if ((queue_region_p->start + queue_base + num_cos) > shaper_start) {
		if ((queue_region_p->end - 1 - num_cos) < shaper_end) {
		    /* specified queue region all has shaper */
		    no_scan = TRUE;
		} else {
		    /* end of queue region has no shaper */
		    if (scan_up) {
			queue = shaper_end + 1;
			end_queue = queue_region_p->end - 1;
		    } else {
			queue = queue_region_p->end - 1;
			end_queue = shaper_end + 1;
		    }
		}
	    } else {
		if ((queue_region_p->end - 1 - num_cos) < shaper_end) {
		    /* head of queue region has no shaper */
		    if (scan_up) {
			queue = queue_region_p->start;
			end_queue = shaper_start - 1;
		    } else {
			queue = shaper_start - 1;
			end_queue = queue_region_p->start;
		    }
		} else {
		    /* both head and tail of queue region has no shaper */
		    if (scan_up) {
			queue = queue_region_p->start;
			end_queue = shaper_start - 1;

			double_scan = TRUE;
			double_scan_start = shaper_end + 1;
			double_scan_end = queue_region_p->end - 1;
		    } else {
			queue = queue_region_p->end - 1;
			end_queue = shaper_end + 1;

			double_scan = TRUE;
			double_scan_start = shaper_start - 1;
			double_scan_end = queue_region_p->start;
		    }
		}
	    }
	}
    } else {
        if (scan_up) {
            queue = requested_base_queue;
	    end_queue = queue_region_p->end;
        }
        else {
            /* When scanning down for local queues, start at the
             * requested queue + cos since we are scanning down
            */
            queue = requested_base_queue + num_cos - 1;
	    end_queue = queue_region_p->start;
        }
    }

    done = FALSE;
    while ( !done && !no_scan ) {

        if ( queue_state[unit][queue].state == BCM_INT_SBX_QUEUE_STATE_AVAILABLE ) {
            if (scan_up && (num_avail == 0) ) {
                *p_allocated_base_queue = queue;
            }
            num_avail++;
        }
        else if ((queue_region == bcm_sbx_cosq_queue_region_local) &&
                 (queue_state[unit][queue].state == BCM_INT_SBX_QUEUE_STATE_RESERVED_TME) &&
                 (dest_type == BCM_INT_SBX_DEST_TYPE_UNICAST)) {
            if ((bw_group_state[unit][queue_state[unit][queue].bw_group].dest_node == node) &&
                (bw_group_state[unit][queue_state[unit][queue].bw_group].dest_port == port)) {
                if (scan_up && num_avail == 0 ) {
                    *p_allocated_base_queue = queue;
                }
                num_avail++;
            }
            else {
                num_avail = 0;
            }
        }
        else {
            /* This queue is not available so reset the consecutive available counter */
            num_avail = 0;
        }

        /* for TME mode make sure we do not cross the ARB group.         */
        /* NOTE: number of queues in ARB port may have to be abstracted  */
        /*       to handle different QE devices. This is for QE2000 only */
        if ( (queue_region == bcm_sbx_cosq_queue_region_local) && SOC_IS_SBX_QE2000(unit) ) {
            if (num_avail != num_cos) {
                 if (scan_up) {
                     if (((queue + 1) % SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT) == 0) {
                         num_avail = 0;
                     }
                 }
                 else {
                    if ((queue % SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT) == 0) {
                         num_avail = 0;
                    }
                 }
            }
        }

        if ( num_avail == num_cos ) {
            found_base_queue = TRUE;
            if ( !scan_up ) {
                *p_allocated_base_queue = queue;
            }
            break;
        }
        if ( scan_up ) {
            queue++;
            done = ( queue >= end_queue );
        } else {
            queue--;
            done = ( queue < end_queue );
        }

	/* 2nd round of scan */
	if (double_scan && done) {
	    double_scan = FALSE;
	    done = FALSE;
	    queue = double_scan_start;
	    end_queue = double_scan_end;
	}
    }

    if ( (requested_base_queue != BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) &&
	 (requested_base_queue != BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING) &&
	 (requested_base_queue != BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING) ) {
        if ( *p_allocated_base_queue != requested_base_queue ) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "WARNING: Could not allocate %d cos levels with requested base queue(%d) node %d port %d.\n"),
                      num_cos, requested_base_queue, node, port));
            return BCM_E_RESOURCE;
        }
    }

    if (found_base_queue == FALSE ) {
        if ( (requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT) ||
	     (requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITH_SHAPING) ||
	     (requested_base_queue == BCM_INT_SBX_REQUESTED_QUEUE_AUTO_SELECT_WITHOUT_SHAPING) ) {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "Could not find (auto-select) base queue with %d cos levels available\n"),
                      num_cos));
            rv = BCM_E_INTERNAL;
        } else {
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "%d cos levels are not available from requested base queue(%d)\n"),
                      num_cos, requested_base_queue));
            rv = BCM_E_RESOURCE;
        }
        return rv;
    }

    base_queue = *p_allocated_base_queue;
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Allocated queue group base(%d)\n"),
              base_queue));

    /* If the unit is reloading, the bw_group was already updated in bcm_cosq_init() */
    /* The state was read from the hardware.  This is only supported for the bm3200  */
    /* If we are a QE2000, get a dummy bw_group since the bag is unused in the Qe2000 */
    if ((!SOC_IS_RELOADING(unit)) || (SOC_IS_SBX_QE2000(unit)))  {
        /* allocate bw group */
        rv = _bcm_sbx_cosq_bw_group_allocate(unit, &bw_group, TRUE /* auto_select */);

        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "Could not allocate bw_group\n")));
            return rv;
        }
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "Allocated bw group(%d)\n"),
                  bw_group));

    } else if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
	p_bwstate_cache =  SOC_SBX_STATE(unit)->bw_group_state_cache;
	/* The BAG was read during bcm_init() and stored in a cache, search cache to find entry */
	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {
	    if (p_bwstate_cache[bw_group].base_queue_of_bag < SOC_SBX_CFG(unit)->num_queues) {
		base_queue_cached = p_bwstate_cache[bw_group].base_queue_of_bag & ~(num_cos-1);
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "EASY_RELOAD bw_group(%d) base_queue_of_bag(0x%x) base_queue_cached(0x%x)\n"),
		          bw_group, 
		          p_bwstate_cache[bw_group].base_queue_of_bag,
		          base_queue_cached));

		if (base_queue_cached == base_queue) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "EASY_RELOAD bw_group(%d) for queue(%d) validated\n"),
		              bw_group, base_queue));
		    break;
		}
	    }
	}

        /* Reserve bw_group if this is valid */
        if ((bw_group >=0 ) || (bw_group < SOC_SBX_CFG(unit)->num_bw_groups)) {

            /* reserve bw group */
            rv = _bcm_sbx_cosq_bw_group_allocate(unit, &bw_group, FALSE /* no auto_select */);

            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: EASY_RELOAD Could not reserve bw_group(%d)\n"),
                           bw_group));
                return BCM_E_INTERNAL;
            }
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "EASY_RELOAD: Reloaded bw group(%d)\n"),
                      bw_group));

	    queue_state[unit][base_queue].bw_group = bw_group;
        } else {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: EASY_RELOAD could not find bw_group allocated in hardware for base queue(%d)\n"),
                       base_queue));
            return BCM_E_INTERNAL;
        }
#endif /* BCM_EASY_RELOAD_SUPPORT */
    }

    /* allocate sysport */
    if (dest_type != BCM_INT_SBX_DEST_TYPE_MULTICAST) {
	if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, DIAG_ASSIGN_SYSPORT)) {
	    /*   Temp setting for verfication bringup of polaris
	     *   sysport is 1-1 mapped with node,port
	     *   sysport[11:0] = node[5:0]port[5:0];
	     */
	    /* Polaris can support only 2816 sysports so make sure that the
	     * the node number never goes above 28
	     */
	    requested_sysport = (((node % 29) & 0x3F) << 6) + (port & 0x3F);
	}
	
	if (egroup_num == -1) egroup_num = 0; /* if uninitialized, set to 0 */

	if (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) {
	    rv = _bcm_sbx_cosq_sysport_allocate(unit, 0, requested_sysport, node, port, egroup_num, queue_region, p_allocated_sysport);

	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "Could not allocate sysport\n")));
		return rv;
	    }
	} else {
	    *p_allocated_sysport = BCM_INT_SBX_INVALID_SYSPORT;
	}
    } else {
        BCM_INT_SBX_COSQ_GET_ESET_FROM_NODE_PORT(unit, node, port, mc, &eset);

        ds_id_base_queue[unit][eset] = base_queue;

	if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
	    /* support 1K esets naturally, no need to use reserved sysport */
	    *p_allocated_sysport = BCM_INT_SBX_INVALID_SYSPORT;	    
	} else {
	    if (eset < BM9600_MAX_MULTICAST_ESETS) {
		/* no sysport allocated when eset is less than BM9600_MAX_MULTICAST_ESETS (128) */
		*p_allocated_sysport = BCM_INT_SBX_INVALID_SYSPORT;
	    } else {
		/* use reserved sysport space when eset is no less than 128 */
		BCM_INT_SBX_COSQ_GET_SYSPORT_FROM_NODE_PORT(unit, node, port, mc, p_allocated_sysport);
	    }
	}
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "Allocated sysport(%d)\n"),
              *p_allocated_sysport));

    for ( queue=base_queue; queue < base_queue+num_cos; queue++ ) {
        queue_state[unit][queue].state  = BCM_INT_SBX_QUEUE_STATE_IN_USE;
        queue_state[unit][queue].bw_group = bw_group;
        queue_state[unit][queue].sysport = *p_allocated_sysport;
        queue_state[unit][queue].default_sysport = *p_allocated_sysport;
        queue_state[unit][queue].attached_fifo = -1;
    }

    /* NOTE: number of queues in ARB port may have to be abstracted  */
    /*       to handle different QE devices. This is for QE2000 only */
    if ((queue_region == bcm_sbx_cosq_queue_region_local) &&
        SOC_IS_SBX_QE2000(unit)) {
        arb_port_base_queue = (base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT) *
            SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;
        for (queue = arb_port_base_queue;
            queue < (arb_port_base_queue + SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT); queue++) {
            if (queue_state[unit][queue].state == BCM_INT_SBX_QUEUE_STATE_AVAILABLE) {
                queue_state[unit][queue].state = BCM_INT_SBX_QUEUE_STATE_RESERVED_TME;
                queue_state[unit][queue].bw_group = bw_group;
            }
        }
    }

    bw_group_state[unit][bw_group].in_use = TRUE;
    bw_group_state[unit][bw_group].dest_node = node;
    bw_group_state[unit][bw_group].dest_port = port;
    bw_group_state[unit][bw_group].dest_mc = mc;
    bw_group_state[unit][bw_group].num_cos = num_cos;
    bw_group_state[unit][bw_group].base_queue = base_queue;
    bw_group_state[unit][bw_group].dest_type = dest_type;
    bw_group_state[unit][bw_group].overlay_in_use = FALSE;
    bw_group_state[unit][bw_group].overlay_dest_node = -1;
    bw_group_state[unit][bw_group].overlay_dest_port = -1;
    bw_group_state[unit][bw_group].overlay_num_cos = -1;
    bw_group_state[unit][bw_group].overlay_base_queue = -1;
    bw_group_state[unit][bw_group].cosq_init_added = cosq_init_added;
    bw_group_state[unit][bw_group].cos_map = -1;

    return TRUE;
}

/*********************************
 * Free a queue group
 */
static int
_bcm_sbx_cosq_queue_group_free(int unit, int32 base_queue, int32 num_cos) {
    int rv = BCM_E_NONE;
    int queue;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int node;
    int port;
    int mc;
    int eset;
    int32 arb_port_base_queue, arb_port_queue_use_count;
    bcm_sbx_cosq_queue_region_type_t queue_region;
    dq_p_t pOld = NULL;
    dq_p_t pList;

    _bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);

    /* Get associated bw_group */
    p_qstate = &queue_state[unit][base_queue];
    bw_group = p_qstate->bw_group;
    p_bwstate = &bw_group_state[unit][bw_group];

    if ( p_bwstate->dest_type == BCM_INT_SBX_DEST_TYPE_MULTICAST) {
        /* Free eset/ds_id related info first */
        node = p_bwstate->dest_node;
        port = p_bwstate->dest_port;
        mc = p_bwstate->dest_mc;

        BCM_INT_SBX_COSQ_GET_ESET_FROM_NODE_PORT(unit, node, port, mc, &eset);
        ds_id_base_queue[unit][eset] = -1;
    }

    rv = _bcm_sbx_cosq_bw_group_free(unit, bw_group);

    if (rv != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Could not free bw_group for base_queue(%d)\n"),
                   base_queue));
        return FALSE;
    }

    /* free specific queue related info */
    for ( queue=base_queue; queue < base_queue+num_cos; queue++ ) {

        p_qstate = &queue_state[unit][queue];

        if (p_qstate->state != BCM_INT_SBX_QUEUE_STATE_IN_USE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: Attempted to free queue(0x%x) which is not currently in use\n"),
                       queue));
            rv = BCM_E_INTERNAL;
        }

        /* free sysport associated with the queue */
        if ( p_qstate->sysport != BCM_INT_SBX_INVALID_SYSPORT ) {
            _bcm_sbx_cosq_sysport_deallocate(unit, p_qstate->sysport);
        }

        sal_memset(p_qstate, 0, sizeof(bcm_sbx_cosq_queue_state_t));
        if (soc_feature(unit, soc_feature_standalone) &&
            SOC_IS_SBX_QE2000(unit)) {
            p_qstate->state  = BCM_INT_SBX_QUEUE_STATE_RESERVED_TME;
        }
        else {
            p_qstate->state  = BCM_INT_SBX_QUEUE_STATE_AVAILABLE;
        }
        if (p_qstate->mgid_list) {
            pList = (dq_p_t)(p_qstate->mgid_list);
            do {
                DQ_REMOVE_HEAD(pList, pOld);
				/* coverity[check_after_deref] */
                if (pOld && (pOld != pList)) {
                    sal_free(pOld);
                }
            } while (pOld && (pOld != pList));
            sal_free(pList);
            p_qstate->mgid_list = NULL;
        } /* if (pList) */

        p_qstate->bw_group = -1;
        p_qstate->sysport = BCM_INT_SBX_INVALID_SYSPORT;
        p_qstate->default_sysport = BCM_INT_SBX_INVALID_SYSPORT;
        p_qstate->attached_fifo = -1;
    }

    /* if no queues in the same arb port is used, then change the queue state from TME to AVAILABLE */
    if (queue_region == bcm_sbx_cosq_queue_region_local) {
        arb_port_base_queue = (base_queue / SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT) *
            SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT;
        arb_port_queue_use_count = 0;
        for (queue = arb_port_base_queue; queue <
                (arb_port_base_queue + SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT); queue++) {
            if (queue_state[unit][queue].state == BCM_INT_SBX_QUEUE_STATE_IN_USE) {
                arb_port_queue_use_count++;
            }
        }

        if (arb_port_queue_use_count == 0) {
            for (queue = arb_port_base_queue; queue <
                    (arb_port_base_queue + SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT); queue++) {
                queue_state[unit][queue].state = BCM_INT_SBX_QUEUE_STATE_AVAILABLE;
            }
        }
    }

    return rv;
}
/*********************************
 * Free a bw group
 */
static int
_bcm_sbx_cosq_bw_group_free(int unit, int bw_group)
{
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int rv = BCM_E_NONE;

    p_bwstate = &bw_group_state[unit][bw_group];

    /* free bw_group related info */
    sal_memset(p_bwstate, 0, sizeof(bcm_sbx_cosq_bw_group_state_t));

    p_bwstate->in_use = FALSE;
    return rv;
}

/*******************************
 * Get default queue params
 */
static void
_bcm_sbx_cosq_get_default_queue_ingress_params(int unit, bcm_sbx_cosq_queue_params_ingress_t *qparams, int32 base_queue, int cos)
{
    bcm_sbx_cosq_queue_region_type_t region;

#if 0 /* REMOVE */
    /* In standalone mode all queues are local */
    if (soc_feature(unit, soc_feature_standalone)) {
        qparams->local = 1;
    } else {
        qparams->local = 0;
    }
#endif /* 0 */

    _bcm_sbx_cosq_queue_type_get(unit, base_queue, &region);
    qparams->local = (region == bcm_sbx_cosq_queue_region_local) ? 1 : 0;

    qparams->bw_mode = SOC_SBX_CFG(unit)->bcm_cosq_priority_mode[cos];

    qparams->bw_value.guarantee_kbps = 0;
    qparams->bw_value.wfq_weight = 0;
    qparams->bw_value.sp_priority = 0;
    if ( qparams->bw_mode == BCM_COSQ_AF ) {
        qparams->bw_value.guarantee_kbps = SOC_SBX_CFG(unit)->bcm_cosq_priority_min_bw_kbps[cos];
        qparams->bw_value.wfq_weight = SOC_SBX_CFG(unit)->bcm_cosq_priority_weight[cos];
    } else if ( qparams->bw_mode == BCM_COSQ_EF ) {
        qparams->bw_value.guarantee_kbps = SOC_SBX_CFG(unit)->bcm_cosq_priority_min_bw_kbps[cos];
    } else {
        qparams->bw_value.wfq_weight = SOC_SBX_CFG(unit)->bcm_cosq_priority_weight[cos];
    }

    qparams->given_weight = 0;

    qparams->min_physical_queue_depth_bytes = SOC_SBX_CFG(unit)->bcm_cosq_priority_min_depth_bytes[cos];
    qparams->max_physical_queue_depth_bytes = SOC_SBX_CFG(unit)->bcm_cosq_priority_max_depth_bytes[cos];

    qparams->shape_limit_kbps = SOC_SBX_CFG(unit)->bcm_cosq_priority_max_bw_kbps[cos];
    qparams->shape_burst_kbps = -1; /* SDK default */

    if (qparams->shape_limit_kbps != 0) {
        qparams->enable_shaping = TRUE;
    } else {
        qparams->enable_shaping = FALSE;
    }

#if 0
    qparams->anemic_watermark_select = BCM_DEFAULT_INT32_VALUE; /* if user doesn't set, we will override */
    qparams->anemic_age_threshold_key = BCM_DEFAULT_INT32_VALUE; /* if user doesn't set, we will override */
#endif /* 0 */

    if (qparams->local) {
        qparams->anemic_watermark_select = SOC_SBX_CFG(unit)->connect_min_util_template[cos];
    } else {
        qparams->anemic_watermark_select = SOC_SBX_CFG(unit)->connect_min_util_remote_template[cos];
    }
    qparams->anemic_age_threshold_key = SOC_SBX_CFG(unit)->connect_max_age_time_template[cos];

    qparams->ingress_spi4 = 0;
    qparams->ingress_spi4_port = 0x3f; /* off, no backpressure based on depth */
    qparams->qla_demand_mask = 1;
    qparams->queue_header_adjust_sign = 0;
    qparams->queue_header_adjust = 0;
    qparams->template = 0; /* WRED Disabled, Template zero is reserved for WRED to not kick in */
    qparams->gain = 0;
    qparams->hold_pri_num_timeslots = SOC_SBX_CFG(unit)->hold_pri_num_timeslots;
    qparams->enabled = FALSE;
}


/*******************************
 * Get default bw_group params
 */
static void
_bcm_sbx_cosq_get_default_bw_group_params(int unit, bcm_sbx_cosq_bw_group_params_t *bwparams)
{
    bwparams->bag_rate_kbps = SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps;
}


static int
_bcm_sbx_cosq_discard_state_init(int unit)
{
    int rc = BCM_E_NONE;
    int nbr_template, i;
    int cos, found, template;
    bcm_sbx_cosq_discard_state_t tmp_discard_state;

    if (!SAL_BOOT_BCMSIM && soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
        /*
         * LOG_VERBOSE(BSL_LS_BCM_COMMON,
         *             (BSL_META_U(unit,
         *                         "#### %s: DIAG_EMULATOR_PARTIAL_INIT is settt\n"),
         *              FUNCTION_NAME()));
         */
        return BCM_E_NONE;
    }

    /* determine if discard functionality is supported */
    if (!(soc_feature(unit, soc_feature_discard_ability) ||
	  soc_feature(unit, soc_feature_ingress_size_templates))) {
        return(rc);
    }

    /* allocate WRED data structures */
    nbr_template = SOC_SBX_CFG(unit)->nDiscardTemplates;
    if (1 > nbr_template) {
        
        nbr_template = 1;
    }
    discard_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_discard_state_t) * nbr_template, "discard_memory");
    if (discard_state[unit] == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, sal_alloc,  Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_MEMORY);
    }

    /* initialize data structures */
    for (i = 0; i < nbr_template; i++) {
        sal_memset((discard_state[unit] + i), 0, sizeof(bcm_sbx_cosq_discard_state_t));
        (discard_state[unit] + i)->template = i;
        (discard_state[unit] + i)->is_free = TRUE;
        (discard_state[unit] + i)->ref_cnt = 0;
    }

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
	/* preallocate templates for all default min/max queue size of all cos levels  */
	for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
	    rc = _bcm_sbx_template_init(unit, cos, &tmp_discard_state, 
					SOC_SBX_CFG(unit)->bcm_cosq_priority_min_depth_bytes[cos],
					SOC_SBX_CFG(unit)->bcm_cosq_priority_max_depth_bytes[cos],
					(discard_state[unit]));
	    if (rc != BCM_E_NONE) {
		return rc;
	    }

	    /* Search for a size template that matches the new min/max size */
	    found =_bcm_sbx_template_match(unit, &tmp_discard_state, &template,
					   BCM_INT_SBX_QUEUE_SIZE_TEMPLATE);
	    if (found == FALSE) {
		rc =_bcm_sbx_allocate_template(unit, &template);
		if (rc != BCM_E_NONE) {
		    return rc;
		}
		rc =_bcm_sbx_template_copy(unit, (discard_state[unit]+template), &tmp_discard_state);
		if (rc != BCM_E_NONE) {
		    return rc;
		}
		/* use -1 to mark this template as reserved and could not be freed */
		(discard_state[unit] + template)->template = BCM_INT_SBX_PREALLOCED_TEMPLATE;

		rc = bcm_sbx_update_template(unit, template);
		if (rc != BCM_E_NONE) {
		    return rc;
		}
	    }
	}
    } else {
	/* mark 1st template as being allocated */
	(discard_state[unit] + BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE)->is_free = FALSE;
	(discard_state[unit] + BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE)->template = BCM_INT_SBX_PREALLOCED_TEMPLATE;
    }

#ifdef BCM_EASY_RELOAD_SUPPORT
    /* initialize data structures used for restore */
    last_restore_queue[unit] = -1;
#endif /* BCM_EASY_RELOAD_SUPPORT */

    return(rc);
}

static void
_bcm_sbx_cosq_discard_state_detach(int unit)
{
     if (discard_state[unit] != NULL) {
         sal_free(discard_state[unit]);
         discard_state[unit] = NULL;
     }
}

static int
_bcm_sbx_cosq_scheduler_state_init(int unit)
{
    int rc = BCM_E_NONE;
    int num_scheduler, i, j, idx = 1;

    num_scheduler = SOC_SBX_CFG(unit)->num_ingress_scheduler + SOC_SBX_CFG(unit)->num_egress_scheduler;

    if (( num_scheduler == 0 ) &&
	(SOC_SBX_CFG(unit)->num_egress_group == 0)) {
        return(rc);
    }

    /* allocate ingress scheduler state */
    if (SOC_SBX_CFG(unit)->num_ingress_scheduler) {
	ingress_scheduler_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_ingress_scheduler_state_t) * SOC_SBX_CFG(unit)->num_ingress_scheduler,
						  "ingress_scheduler_memory");
	if (ingress_scheduler_state[unit] == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}
    }

    /* initialize ingress scheduler data structures */
    for (i = 0; i < SOC_SBX_CFG(unit)->num_ingress_scheduler; i++) {
        sal_memset((ingress_scheduler_state[unit] + i), 0, sizeof(bcm_sbx_cosq_ingress_scheduler_state_t));
        (ingress_scheduler_state[unit] + i)->in_use = FALSE;
        (ingress_scheduler_state[unit] + i)->level = -1;
        (ingress_scheduler_state[unit] + i)->node = -1;
        (ingress_scheduler_state[unit] + i)->num_childs = 0;
    }

    /* allocate egress scheduler state */
    if (SOC_SBX_CFG(unit)->num_egress_scheduler) {
	egress_scheduler_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_egress_scheduler_state_t) * SOC_SBX_CFG(unit)->num_egress_scheduler,
						 "egress_scheduler_memory");
	if (egress_scheduler_state[unit] == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}
    }

    /* initialize egress scheduler data structures */
    for (i = 0; i < SOC_SBX_CFG(unit)->num_egress_scheduler; i++) {
        sal_memset((egress_scheduler_state[unit] + i), 0, sizeof(bcm_sbx_cosq_egress_scheduler_state_t));
        (egress_scheduler_state[unit] + i)->in_use = FALSE;
        (egress_scheduler_state[unit] + i)->level = -1;
        (egress_scheduler_state[unit] + i)->node = -1;
    }

    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    idx = SOC_SBX_CFG(unit)->cfg_num_nodes;
	}
    }

    /* allocate egress group state */
    if (SOC_SBX_CFG(unit)->num_egress_group) {
        egress_group_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_egress_group_state_t) * SOC_SBX_CFG(unit)->num_egress_group * idx,
                                             "egress_group_memory");
	if (egress_group_state[unit] == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}
    }

    /* initialize egress group data structures */
    for (i = 0; i < (SOC_SBX_CFG(unit)->num_egress_group * idx); i++) {
        sal_memset((egress_group_state[unit] + i), 0, sizeof(bcm_sbx_cosq_egress_group_state_t));
        (egress_group_state[unit] + i)->in_use = FALSE;
        (egress_group_state[unit] + i)->eg_in_use = FALSE;
        (egress_group_state[unit] + i)->eg_scheduler = -1;
        (egress_group_state[unit] + i)->child_port = -1;
    }

    /* allocate ingress multipath state */
    if (SOC_SBX_CFG(unit)->num_ingress_multipath) {
	ingress_multipath_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_ingress_multipath_state_t) * SOC_SBX_CFG(unit)->num_ingress_multipath,
						  "ingress_multipath_memory");
	if (ingress_multipath_state[unit] == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}
    }

    /* initialize ingress multipath data structures */
    for (i = 0; i < SOC_SBX_CFG(unit)->num_ingress_multipath; i++) {
        sal_memset((ingress_multipath_state[unit] + i), 0, sizeof(bcm_sbx_cosq_ingress_multipath_state_t));
        (ingress_multipath_state[unit] + i)->in_use = FALSE;
        (ingress_multipath_state[unit] + i)->level = -1;
        (ingress_multipath_state[unit] + i)->node = -1;
        (ingress_multipath_state[unit] + i)->num_nodes = 0;
    }

    /* allocate egress multipath state */
    if (SOC_SBX_CFG(unit)->num_egress_multipath) {
	egress_multipath_state[unit] = sal_alloc(sizeof(bcm_sbx_cosq_egress_multipath_state_t) * SOC_SBX_CFG(unit)->num_egress_multipath,
						 "egress_multipath_memory");
	if (egress_multipath_state[unit] == NULL) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, sal_alloc,  Unit(%d)\n"),
	               FUNCTION_NAME(), unit));
	    return(BCM_E_MEMORY);
	}
    }

    /* initialize egress multipath data structures */
    for (i = 0; i < SOC_SBX_CFG(unit)->num_egress_multipath; i++) {
        sal_memset((egress_multipath_state[unit] + i), 0, sizeof(bcm_sbx_cosq_egress_multipath_state_t));
        (egress_multipath_state[unit] + i)->in_use = FALSE;
        (egress_multipath_state[unit] + i)->node = -1;
	for (j = 0; j < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; j++) {
	    (egress_multipath_state[unit] + i)->member[j] = 0;
	}
    }
    
    return(rc);
}

static void
_bcm_sbx_cosq_scheduler_state_detach(int unit)
{
    if (ingress_scheduler_state[unit] != NULL) {
        sal_free(ingress_scheduler_state[unit]);
        ingress_scheduler_state[unit] = NULL;
    }

    if (egress_scheduler_state[unit] != NULL) {
        sal_free(egress_scheduler_state[unit]);
        egress_scheduler_state[unit] = NULL;
    } 

    if (egress_group_state[unit] != NULL) {
        sal_free(egress_group_state[unit]);
        egress_group_state[unit] = NULL;
    } 

    if (ingress_multipath_state[unit] != NULL) {
        sal_free(ingress_multipath_state[unit]);
        ingress_multipath_state[unit] = NULL;
    }

    if (egress_multipath_state[unit] != NULL) {
        sal_free(egress_multipath_state[unit]);
        egress_multipath_state[unit] = NULL;
    } 
}

STATIC INLINE int
_bcm_sbx_get_template_ref(int unit, int template)
{
    return((discard_state[unit] + template)->ref_cnt);
}

STATIC INLINE void
_bcm_sbx_dec_template_ref(int unit, int template)
{
    (discard_state[unit] + template)->ref_cnt--;
}

STATIC INLINE void
_bcm_sbx_inc_template_ref(int unit, int template)
{
    (discard_state[unit] + template)->ref_cnt++;
}

static int
_bcm_sbx_allocate_template(int unit, int *template)
{
    int rv = BCM_E_NONE;
    int i, nbr_template, start_template;
    int found = FALSE;

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
	start_template = 0;
    } else {
	start_template = BCM_SBX_COSQ_DISCARD_AVAILABLE_TEMPLATE_START;
    }

    nbr_template = SOC_SBX_CFG(unit)->nDiscardTemplates;
    for (i = start_template; i < nbr_template; i++) {
        if ((discard_state[unit] + i)->is_free == TRUE) {
            found = TRUE;
            break;
        }
    }

    if (found == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, template resource not available, Unit(%d)\n"),
                   FUNCTION_NAME(), unit));
        return(BCM_E_RESOURCE);
    }

    (discard_state[unit] + i)->is_free = FALSE;
    _bcm_sbx_inc_template_ref(unit, i);
    (discard_state[unit] + i)->mtu_sz = 0;
    (discard_state[unit] + i)->queue_size = 0;
    (discard_state[unit] + i)->min_queue_size = 0;
    (discard_state[unit] + i)->pfc_en = FALSE;
    (discard_state[unit] + i)->pfc_pg = 0;

    (*template) = i;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "allocated template(%d)\n"),
              i));
    return(rv);
}

static int
_bcm_sbx_deallocate_template(int unit, int template)
{
    int rv = BCM_E_NONE;
    int color, max_color;

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    if ((_bcm_sbx_get_template_ref(unit, template) < 2) &&
	((discard_state[unit] + template)->template != BCM_INT_SBX_PREALLOCED_TEMPLATE)) {
        (discard_state[unit] + template)->mtu_sz = 0;
        (discard_state[unit] + template)->queue_size = 0;
        (discard_state[unit] + template)->min_queue_size = 0;
        (discard_state[unit] + template)->pfc_en = FALSE;
        (discard_state[unit] + template)->pfc_pg = 0;
        sal_memset((discard_state[unit] + template)->config, 0, (max_color * sizeof(bcm_cosq_gport_discard_t)));

	rv = bcm_sbx_cosq_template_pfc_config(unit, template);
	if (rv != BCM_E_NONE) {
	    return rv;
	}

        for (color = 0; color < max_color; color++) {
	    MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set,
				 (unit, 0, 0, color, template, 0, 0,
				 &((discard_state[unit] + BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE)->config[color])));
        }
        (discard_state[unit] + template)->is_free = TRUE;
    }
    if (_bcm_sbx_get_template_ref(unit, template) > 0) {
        _bcm_sbx_dec_template_ref(unit, template);
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "deallocated template(%d)\n"),
              template));
    return(rv);
}

/* init discard template
 *     if min/max size is -1, update with what's in discard_state_src if it's not NULL
 *        otherwise no update
 *     wred state from discard_state_src if not NULL pointer
 */
static int
_bcm_sbx_template_init(int unit, bcm_cos_queue_t cosq, bcm_sbx_cosq_discard_state_t *discard_state,
		       int min_size, int max_size,
		       bcm_sbx_cosq_discard_state_t *discard_state_src)
{
    int color, max_color;

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    if (discard_state == NULL) {
	return BCM_E_PARAM;
    }

    if ((0 > cosq) && (!discard_state_src)) {
        /* copy condition not valid without source */
        return BCM_E_PARAM;
    }

    if (min_size == -1) {
	if (discard_state_src == NULL) {
	    min_size = discard_state->min_queue_size;
	} else {
	    min_size = discard_state_src->min_queue_size;
	}
    }
    discard_state->min_queue_size = min_size;

    if (max_size == -1) {
	if (discard_state_src == NULL) {
	    max_size = discard_state->queue_size;
	} else {
	    max_size = discard_state_src->queue_size;
	}
    }
    discard_state->queue_size = max_size;

    if (discard_state_src != NULL) {
        discard_state->mtu_sz = discard_state_src->mtu_sz;
	if (soc_feature(unit, soc_feature_priority_flow_control) && (cosq >= 0)) {
	    discard_state->pfc_en = _bcm_sbx_cosq_pfc_enabled(unit, cosq);
	    if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
		discard_state->pfc_pg = _bcm_sbx_cosq_pfc_pg_get(unit, cosq);
	    } else {
		discard_state->pfc_pg = 0;
	    }
	} else {
	    discard_state->pfc_en = discard_state_src->pfc_en;
	    discard_state->pfc_pg = discard_state_src->pfc_pg;
	}

        for (color = 0; color < max_color; color++) {
	    discard_state->config[color].flags = discard_state_src->config[color].flags;
	    discard_state->config[color].min_thresh = discard_state_src->config[color].min_thresh;
	    discard_state->config[color].max_thresh = discard_state_src->config[color].max_thresh;
	    discard_state->config[color].drop_probability = discard_state_src->config[color].drop_probability;
	    discard_state->config[color].gain = discard_state_src->config[color].gain;
	    discard_state->config[color].ecn_thresh = discard_state_src->config[color].ecn_thresh;
        }
    } else {
	if (soc_feature(unit, soc_feature_priority_flow_control)) {
	    discard_state->pfc_en = _bcm_sbx_cosq_pfc_enabled(unit, cosq);
	    if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
		discard_state->pfc_pg = _bcm_sbx_cosq_pfc_pg_get(unit, cosq);
	    } else {
		discard_state->pfc_pg = 0;
	    }
	} else {
	    discard_state->pfc_en = FALSE;
	    discard_state->pfc_pg = 0;
	}
    }

    return BCM_E_NONE;
}

static int
_bcm_sbx_template_copy(int unit, bcm_sbx_cosq_discard_state_t *discard_state_dest,
		       bcm_sbx_cosq_discard_state_t *discard_state_src)
{
    return (_bcm_sbx_template_init(unit, -1, discard_state_dest, -1, -1, discard_state_src));
}

static int
_bcm_sbx_template_compare(int unit, bcm_sbx_cosq_discard_state_t *discard_state1,
			  bcm_sbx_cosq_discard_state_t *discard_state2, int mode)
{
    int i;
    int max_color;

    /*
     * BCM_INT_SBX_QUEUE_SIZE_TEMPLATE: compare queue min/max size only
     *
     * BCM_INT_SBX_FIC_WRED_TEMPLATE:   compare queue max size and all WRED parameters except gain
     *
     * BCM_INT_SBX_LOCAL_WRED_TEMPLATE: compare queue min/max size and all WRED parameters
     */

    /* pfc related */
    if (soc_feature(unit, soc_feature_priority_flow_control)) {
	if (discard_state1->pfc_en != discard_state2->pfc_en) {
	    return FALSE;
	}
	if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    if (discard_state1->pfc_pg != discard_state2->pfc_pg) {
		return FALSE;
	    }
	}
    }

    if (mode == BCM_INT_SBX_QUEUE_SIZE_TEMPLATE) {
	/* only compare min/max queue size */
	if ((discard_state1->min_queue_size == discard_state2->min_queue_size) &&
	    (discard_state1->queue_size == discard_state2->queue_size)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }

    if ( (discard_state1->mtu_sz != discard_state2->mtu_sz) ||
	 (discard_state1->queue_size != discard_state2->queue_size) ) {
        return(FALSE);
    }

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    for ( i = 0; i < max_color; i++) {
        if ( (discard_state1->config[i].min_thresh !=
                                     discard_state2->config[i].min_thresh) ||
             (discard_state1->config[i].max_thresh !=
                                     discard_state2->config[i].max_thresh) ||
             (discard_state1->config[i].drop_probability !=
                                     discard_state2->config[i].drop_probability) ||
             (discard_state1->config[i].ecn_thresh !=
                                     discard_state2->config[i].ecn_thresh) ) {
            return(FALSE);
        }
	if (mode == BCM_INT_SBX_LOCAL_WRED_TEMPLATE) {
	    if ( (discard_state1->config[i].gain != discard_state2->config[i].gain) ) {
		return(FALSE);
	    }	    
	}
    }

    if (mode == BCM_INT_SBX_LOCAL_WRED_TEMPLATE) {
	if ((discard_state1->min_queue_size != discard_state2->min_queue_size)) {
	    return(FALSE);
	}
    }

    return(TRUE);
}

/* static */ int
_bcm_sbx_template_match(int unit, bcm_sbx_cosq_discard_state_t *discard_state_p,
			int *match_template, int mode)
{
    int i, nbr_template, start_template;

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
	start_template = 0;
    } else {
	start_template = BCM_SBX_COSQ_DISCARD_AVAILABLE_TEMPLATE_START;
    }

    nbr_template = SOC_SBX_CFG(unit)->nDiscardTemplates;
    for (i = start_template; i < nbr_template; i++) {
        if (_bcm_sbx_template_compare(unit, discard_state_p, (discard_state[unit] + i), mode) != TRUE) {
            continue;
        }

        (*match_template) = i;
        return(TRUE);
    }

    return(FALSE);
}

static int
_bcm_sbx_template_collapse(int unit, int template, int *match_template, int mode)
{
    int i, nbr_template, start_template;

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
	start_template = 0;
    } else {
	start_template = BCM_SBX_COSQ_DISCARD_AVAILABLE_TEMPLATE_START;
    }

    nbr_template = SOC_SBX_CFG(unit)->nDiscardTemplates;
    for (i = start_template; i < nbr_template; i++) {
        if (template == i) {
            continue;
        }

        if (_bcm_sbx_template_compare(unit, (discard_state[unit] + template),
				      (discard_state[unit] + i), mode) != TRUE) {
            continue;
        }

        break;
    }

    if (i >= nbr_template) {
        (*match_template) = template;
        return(FALSE);
    }

    (*match_template) = i;

    _bcm_sbx_inc_template_ref(unit, i);
    _bcm_sbx_deallocate_template(unit, template);

    return(TRUE);
}

static int
_bcm_sbx_cosq_queue_update_template_gain(int unit, int template, int gain, int queue)
{
    int rv = BCM_E_NONE;
    int updated_gain = 0, cur_template, cur_gain;


    if (gain == -1) {
        bcm_sbx_cosq_queue_get_template_gain(unit, queue, &cur_template, &cur_gain);
        updated_gain = (cur_template == BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE) ?
                                                     BCM_SBX_COSQ_DEFAULT_GAIN : cur_gain;
    }
    else {
        updated_gain = gain;
    }

    bcm_sbx_cosq_queue_set_template_gain(unit, queue, template, updated_gain);
    return(rv);
}

static void
_bcm_sbx_cosq_discard_color_to_internal_color_range(int unit, int color, int *start_color, int *nbr_color)
{

    (*nbr_color) = 0;

    if (color & BCM_COSQ_DISCARD_COLOR_GREEN) {
        *(start_color + (*nbr_color)) = BCM_SBX_COSQ_DISCARD_COLOR_GREEN_DP0;
        (*nbr_color)++;
    }

    if (color & BCM_COSQ_DISCARD_COLOR_YELLOW) {
        *(start_color + (*nbr_color)) = BCM_SBX_COSQ_DISCARD_COLOR_YELLOW_DP1;
        (*nbr_color)++;
    }

    if (color & BCM_COSQ_DISCARD_COLOR_RED) {
        *(start_color + (*nbr_color)) = BCM_SBX_COSQ_DISCARD_COLOR_RED_DP2;
        (*nbr_color)++;
    }

    if ( (color & BCM_COSQ_DISCARD_COLOR_BLACK) && soc_feature(unit, soc_feature_discard_ability_color_black) ) {
        *(start_color + (*nbr_color)) = BCM_SBX_COSQ_DISCARD_COLOR_BLACK_DP3;
        (*nbr_color)++;
    }
}

static int
_bcm_sbx_cosq_queue_discard_disabled(int unit, int template, int disabling_color)
{
    int enabled_colors = 0, disabled_colors = 0;
    int lcl_color, start_color[BCM_SBX_COSQ_DISCARD_MAX_COLORS], nbr_color, max_color;
    int rv;

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    for (lcl_color = 0; lcl_color < max_color; lcl_color++) {
        enabled_colors |= ((discard_state[unit] + template)->config[lcl_color].drop_probability == 0) ?  0 : (1 << lcl_color);
    }

    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, disabling_color, start_color, &nbr_color);
    for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
        disabled_colors |= (1 << start_color[lcl_color]);
    }
    enabled_colors &= ~(disabled_colors);

    rv = (enabled_colors == 0) ? TRUE : FALSE;

    return(rv);
}

static int
_bcm_cosq_gport_discard_no_change(int unit, int template, int gain, int color,
                                          bcm_cosq_gport_discard_t *discard, uint32 queue_size)
{
    int lcl_color, start_color[BCM_SBX_COSQ_DISCARD_MAX_COLORS], nbr_color;


    if ((discard_state[unit] + template)->queue_size != queue_size) {
        return(FALSE);
    }

    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, color, start_color, &nbr_color);

    for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
        if ( ((discard_state[unit] + template)->config[*(start_color + lcl_color)].min_thresh !=
                                                           discard->min_thresh) ||
          ((discard_state[unit] + template)->config[*(start_color + lcl_color)].max_thresh !=
                                                           discard->max_thresh) ||
          ((discard_state[unit] + template)->config[*(start_color + lcl_color)].ecn_thresh !=
                                                           discard->ecn_thresh) ||
          ((discard_state[unit] + template)->config[*(start_color + lcl_color)].drop_probability !=
                                                           discard->drop_probability) ||
          ((gain != discard->gain) && (discard->gain != -1) ) ) {
            return(FALSE);
        }
    }

    return(TRUE);
}

static int
_bcm_sbx_discard_profile_consistency_check(int unit,
                                           bcm_cosq_gport_discard_t *discard,
                                           int queue_sz)
{
    int rv = BCM_E_NONE;
    int max_queue_depth = queue_sz;


    if (discard->drop_probability == 0) {
        return(rv);
    }

    if ((max_queue_depth < discard->max_thresh) ||
          (max_queue_depth < discard->min_thresh) ||
          (max_queue_depth < discard->ecn_thresh) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid Discard paramerers, Unit(%d), "
                               "MinThreshold: 0x%x MaxThreshold: 0x%x EcnThrehold: 0x%x\n"),
                   FUNCTION_NAME(), unit, discard->min_thresh,
                   discard->max_thresh, discard->ecn_thresh));
        return(BCM_E_PARAM);
    }

    if (discard->max_thresh < discard->min_thresh) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Max Threshoold needs to be bigger then Min threshold, Unit(%d), "
                               "MinThreshold: 0x%x MaxThreshold: 0x%x\n"),
                   FUNCTION_NAME(), unit, discard->min_thresh, discard->min_thresh));
        return(BCM_E_PARAM);
    }

    if ( (discard->ecn_thresh > discard->max_thresh) ||
                   (discard->ecn_thresh < discard->min_thresh) ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, Invalid ecn threshold, Unit(%d), "
                               "MinThreshold: 0x%x MaxThreshold: 0x%x EcnThreshold: 0x%x\n"),
                   FUNCTION_NAME(), unit, discard->min_thresh,
                   discard->min_thresh, discard->ecn_thresh));
        return(BCM_E_PARAM);
    }

    return(rv);
}

static int
_bcm_sbx_cosq_gport_discard_disable(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               int queue,
                               uint32 color,
                               bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_NONE;
    int template, gain;
    int update_template, nbr_nodes;
    int allocated_template = -1, match_template;
    int nbr_modules_enabled = 1, is_queue_sz_fixed = TRUE;
    uint32 min_queue_size, max_queue_sz = 0, queue_sz, adj_queue_sz, max_thresh;
    int color_val, lcl_color, start_color[BCM_SBX_COSQ_DISCARD_MAX_COLORS], nbr_color, max_color;
    bcm_sbx_cosq_discard_state_t tmp_discard_state;
    int found;

    color_val = (color & BCM_COSQ_DISCARD_COLOR_ALL);

    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, color_val, start_color, &nbr_color);

    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
        return(rv);
#endif /* BCM_EASY_RELOAD_SUPPORT */
    }

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {
	bcm_sbx_cosq_queue_get_template_gain(unit, queue, &template, &gain);
	if (_bcm_sbx_cosq_queue_discard_disabled(unit, template, 0)) {
	    /* all colors are disabled already */
	    return(rv);
	}

	/* this branch only applies to bcm88230 WRED on local queue, FIC queues
	 * should not have this API issued on sirius.
	 */
	if (_bcm_sbx_cosq_queue_is_local(unit, queue) == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, Discard functionality not supported on queue %d when device is"
	                           " under current mode, Unit(%d)\n"), FUNCTION_NAME(), queue, unit));
	    return(BCM_E_UNAVAIL);
	}

	/* get queue sizes */
	rv = bcm_sbx_cosq_gport_size_get(unit, gport, cosq, &min_queue_size, &max_queue_sz);
	if (rv != BCM_E_NONE) {
	    return(rv);
	}
	    
	/* if all color are disabled due to this function call */
	if ((color_val == BCM_COSQ_DISCARD_COLOR_ALL) ||
	    _bcm_sbx_cosq_queue_discard_disabled(unit, template, color_val)) {
	    _bcm_sbx_deallocate_template(unit, template);

	    /* search a template for matching min/max size and disabled WRED, allocate
	     * one if don't exist.
	     *
	     * NOTE: template 0 has all wred disabled 
	     */
	    rv = _bcm_sbx_template_init(unit, cosq, &tmp_discard_state, min_queue_size,
					max_queue_sz, (discard_state[unit]));
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	    found =_bcm_sbx_template_match(unit, &tmp_discard_state, &template,
					   BCM_INT_SBX_LOCAL_WRED_TEMPLATE);
	    if (found) {
		_bcm_sbx_inc_template_ref(unit, template);
	    } else {
		rv = _bcm_sbx_allocate_template(unit, &template);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "%s: ERROR: _bcm_sbx_allocate_template, Unit(%d)\n"),
		               FUNCTION_NAME(), unit));
		    return rv;
		}

		rv = _bcm_sbx_template_copy(unit, (discard_state[unit]+template),
					    &tmp_discard_state);
		if (rv != BCM_E_NONE) {
		    return rv;
		}

		/* update one color to set the template gain/sizes */
		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
					  0, template, (discard_state[unit]+template)->queue_size,
					  (discard_state[unit]+template)->min_queue_size,
					   &((discard_state[unit]+template)->config[0])));
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, mbcm_sbx_driver, Unit(%d), Error: 0x%x\n"),
		               FUNCTION_NAME(), unit, rv));
		    allocated_template = template;
		    goto err;
		}
	    }

	    /* Update Queue Configuration */
	    bcm_sbx_cosq_queue_set_template_gain(unit, queue, template, 0);

	    rv = bcm_sbx_cosq_template_pfc_config(unit, template);

	    return(rv);	    
	}

	/* still some color remains wred enabled */
	queue_sz = SOC_SBX_CFG(unit)->discard_queue_size;
	adj_queue_sz = queue_sz;
	if (queue_sz == 0) {
	    is_queue_sz_fixed = FALSE;
	    queue_sz = max_queue_sz;
	    adj_queue_sz = max_queue_sz;
	}

	/* build tmp discard state as how it should be after the specified colors are disabled */
	rv = _bcm_sbx_template_init(unit, cosq, &tmp_discard_state, min_queue_size,
				    queue_sz, (discard_state[unit]+template));
	if (rv != BCM_E_NONE) {
	    return rv;
	}
	
	/* disable all specified colors */
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    tmp_discard_state.config[*(start_color + lcl_color)] = discard_state[unit][0].config[0];		    
	}

	/* new template matches to any existing template? */
	found =_bcm_sbx_template_match(unit, &tmp_discard_state, &match_template,
				       BCM_INT_SBX_LOCAL_WRED_TEMPLATE);
	if (found == TRUE) {
	    /* matches, alloc/dealloc */
	    _bcm_sbx_inc_template_ref(unit, match_template);	    

	    rv = _bcm_sbx_deallocate_template(unit, template);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	    
	    if (match_template != template) {
		rv =_bcm_sbx_cosq_queue_update_template_gain(unit, match_template, discard->gain, queue);
	    }
	    return rv;
	} else {
	    if (_bcm_sbx_get_template_ref(unit, template) == 1) {
		/* no matches, this is the only queue using current template, update it */

		/* need to update all colors if queue size changed, this seems would never happen */
		if (queue_sz != (discard_state[unit] + template)->queue_size) {		
		    /* need to update all colors */
		    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
		}

		update_template = template;
	    } else {
		/* no matches, there are other queue sharing the template, alocate a new one */
		rv = _bcm_sbx_allocate_template(unit, &allocated_template);
		if (rv != BCM_E_NONE) {
		    return(rv);
		}
		
		/* copy template and disable the color specified */
		rv = _bcm_sbx_template_copy(unit, (discard_state[unit]+allocated_template), 
					    (discard_state[unit]+template));
		if (rv != BCM_E_NONE) {
		    return rv;
		}
		
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    sal_memset(&(discard_state[unit] + allocated_template)->config[*(start_color + lcl_color)], 0, sizeof(bcm_cosq_gport_discard_t));
		}
		
		/* need to update all colors */
		_bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
		
		update_template = allocated_template;		

		/* dereference the old one */
		rv = _bcm_sbx_deallocate_template(unit, template);
		if (rv != BCM_E_NONE) {
		    return rv;
		}
	    }
	}

	/* sanity check discard parameters of the updated template */
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    rv = _bcm_sbx_discard_profile_consistency_check(unit,
							    ((allocated_template == -1) ?
							     &(tmp_discard_state.config[*(start_color + lcl_color)]) :
							     &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)]),
							    adj_queue_sz);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	}

	/* update the colors of the template in hardware */
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
				      start_color[lcl_color], update_template, adj_queue_sz, min_queue_size,
				      ((allocated_template == -1) ?
				       &(tmp_discard_state.config[*(start_color + lcl_color)]):
				       &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)])));
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, mbcm_sbx_driver, Unit(%d), Error: 0x%x\n"),
		           FUNCTION_NAME(), unit, rv));
		goto err;
	    }
	}
	
	rv = bcm_sbx_cosq_template_pfc_config(unit, update_template);
	if (rv != BCM_E_NONE) {
	    return rv;
	}	

	/* update template software state */
	if ((allocated_template == -1))  {
	    /* update existing template with the temp_discard state, do it after hardware update */
	    rv = _bcm_sbx_template_init(unit, cosq, (discard_state[unit]+template), min_queue_size,
					queue_sz, &tmp_discard_state);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}

	rv = _bcm_sbx_cosq_queue_update_template_gain(unit, update_template, discard->gain, queue);
	
	return(rv);

    } else {
	/* Disabled WRED */
	bcm_sbx_cosq_queue_get_template_gain(unit, queue, &template, &gain);
	if (template == BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE) {
	    return(rv);
	}

	if ((color_val == BCM_COSQ_DISCARD_COLOR_ALL) ||
	    _bcm_sbx_cosq_queue_discard_disabled(unit, template, color_val)) {
	    /* if disable all or all colors are disabled */
	    _bcm_sbx_deallocate_template(unit, template);
	    
	    /* Update Queue Configuration */
	    bcm_sbx_cosq_queue_set_template_gain(unit, queue, BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE, 0);
	    return(rv);
	}	

	/* determine queue size */
	rv = bcm_sbx_cosq_gport_size_get(unit, gport, cosq, &min_queue_size, &max_queue_sz);
	if (rv != BCM_E_NONE) {
	    return(rv);
	}
	
	queue_sz = SOC_SBX_CFG(unit)->discard_queue_size;
	adj_queue_sz = queue_sz;
	if (queue_sz == 0) {
	    is_queue_sz_fixed = FALSE;
	    queue_sz = max_queue_sz;
	    adj_queue_sz = max_queue_sz;
	    if (_bcm_sbx_cosq_queue_is_local(unit, queue) == FALSE) {
		adj_queue_sz = SOC_SBX_CFG(unit)->cfg_num_nodes * max_queue_sz;
	    }
	}
	
	if (_bcm_sbx_get_template_ref(unit, template) == 1) {
	    
	    update_template = template;
	    
	    tmp_discard_state.queue_size = queue_sz;
	    
	    if (queue_sz != (discard_state[unit] + template)->queue_size) {
		for (lcl_color = 0; lcl_color < max_color; lcl_color++) {
		    tmp_discard_state.config[lcl_color] = (discard_state[unit] + template)->config[lcl_color];
		}
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    tmp_discard_state.config[*(start_color + lcl_color)] = discard_state[unit][0].config[0];
		}
		
		/* need to update all colors */
		_bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
	    }
	    else {
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    tmp_discard_state.config[*(start_color + lcl_color)] = discard_state[unit][0].config[0];
		}
	    }
	    
	}
	else {
	    rv = _bcm_sbx_allocate_template(unit, &allocated_template);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	    
	    /* this should not happen, code like this just to make coverity happy */
	    if (allocated_template == -1) {
		return BCM_E_INTERNAL;
	    }
	    
	    /* update template */
	    for (lcl_color = 0; lcl_color < max_color; lcl_color++) {
		(discard_state[unit] + allocated_template)->config[lcl_color] =
		    (discard_state[unit] + template)->config[lcl_color];
	    }
	    
	    for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		sal_memset(&(discard_state[unit] + allocated_template)->config[*(start_color + lcl_color)], 0, sizeof(bcm_cosq_gport_discard_t));
	    }
	    
	    (discard_state[unit] + allocated_template)->queue_size =
		(discard_state[unit] + template)->queue_size;
	    /* need to update all colors */
	    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
	    
	    update_template = allocated_template;
	    
	}
	
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    rv = _bcm_sbx_discard_profile_consistency_check(unit,
							    ((allocated_template == -1) ?
							     &(tmp_discard_state.config[*(start_color + lcl_color)]) :
							     &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)]),
							    adj_queue_sz);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	}
	
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    
	    /* adjust queue size */
	    if ( (is_queue_sz_fixed == FALSE) &&
		 (_bcm_sbx_cosq_queue_is_local(unit, queue) == FALSE) ) {
		rv = bcm_sbx_stk_get_modules_enabled(unit, &nbr_modules_enabled);
		if (rv == BCM_E_NONE) {
		    if (nbr_modules_enabled == 0) {
			nbr_modules_enabled = 1;
		    }
		} else {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, error in bcm_sbx_stk_get_modules_enabled"
		                           "), Unit(%d), Error: 0x%x\n"), FUNCTION_NAME(), unit, rv));
		    goto err;
		}
		max_thresh = ((allocated_template == -1) ?
			      tmp_discard_state.config[*(start_color + lcl_color)].max_thresh:
			      (discard_state[unit] + update_template)->config[*(start_color + lcl_color)].max_thresh);
		
		for (nbr_nodes = nbr_modules_enabled;
		     nbr_nodes < SOC_SBX_CFG(unit)->cfg_num_nodes; nbr_nodes++) {
		    if ((nbr_nodes * queue_sz) > max_thresh) {
			adj_queue_sz = nbr_nodes * queue_sz;
			break;
		    }
		}
	    }
	    
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
				      start_color[lcl_color], update_template, adj_queue_sz, min_queue_size,
				      ((allocated_template == -1) ?
				       &(tmp_discard_state.config[*(start_color + lcl_color)]):
				       &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)])));
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, mbcm_sbx_driver, Unit(%d), Error: 0x%x\n"),
		           FUNCTION_NAME(), unit, rv));
		goto err;
	    }

	    rv = bcm_sbx_cosq_template_pfc_config(unit, update_template);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }

	}
	
	/* update template */
	if ((allocated_template == -1))  {
	    (discard_state[unit] + update_template)->queue_size = queue_sz;
	    for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		(discard_state[unit] + update_template)->config[*(start_color + lcl_color)] = tmp_discard_state.config[*(start_color + lcl_color)];
	    }
	}
	else {
	    if (template != BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE) {
		_bcm_sbx_dec_template_ref(unit, template);
	    }
	}
	
	_bcm_sbx_template_collapse(unit, update_template, &match_template, BCM_INT_SBX_FIC_WRED_TEMPLATE);
	_bcm_sbx_cosq_queue_update_template_gain(unit, match_template, discard->gain, queue);
	
	return(rv);
    }

err:
    if (allocated_template != -1) {
        _bcm_sbx_deallocate_template(unit, allocated_template);
    }

    return(rv);
}


static int
_bcm_sbx_cosq_gport_discard_enable(int unit,
                               bcm_gport_t gport,
                               bcm_cos_queue_t cosq,
                               int queue,
                               uint32 color,
                               bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_NONE;
    int template, gain;
    int allocated_template = -1, match_template;
    int update_template, nbr_nodes;
    int nbr_modules_enabled = 1, is_queue_sz_fixed = TRUE;
    uint32 min_queue_size, max_queue_sz = 0, queue_sz, adj_queue_sz, max_thresh;
    int color_val, lcl_color, start_color[BCM_SBX_COSQ_DISCARD_MAX_COLORS], nbr_color, max_color;
    bcm_sbx_cosq_discard_state_t tmp_discard_state;
    int found;

    color_val = (color & BCM_COSQ_DISCARD_COLOR_ALL);

    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, color_val, start_color, &nbr_color);

    /* determine queue size */
    rv = bcm_sbx_cosq_gport_size_get(unit, gport, cosq, &min_queue_size, &max_queue_sz);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    queue_sz = SOC_SBX_CFG(unit)->discard_queue_size;
    adj_queue_sz = queue_sz;
    if (queue_sz == 0) {
        is_queue_sz_fixed = FALSE;
        queue_sz = max_queue_sz;
        adj_queue_sz = max_queue_sz;
        if (_bcm_sbx_cosq_queue_is_local(unit, queue) == FALSE) {
            adj_queue_sz = SOC_SBX_CFG(unit)->cfg_num_nodes * max_queue_sz;
        }
    }

    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT
        rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_get_template_gain, (unit, queue, &template, &gain));
        if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, mbcm_sbx_driver, Unit(%d), Error: 0x%x\n"),
                       FUNCTION_NAME(), unit, rv));
            return(rv);
        }

        /* check for valid template and gain */
        if ((template == BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE) ||
	    (template > SOC_SBX_CFG(unit)->nDiscardTemplates)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: %s, Incorrect Template: 0x%x\n"),
                       FUNCTION_NAME(), template));
            return(BCM_E_INTERNAL);
        }

        /* update / verify the parameters                                */
        /* NOTE: This may require a change when consistency across hw/sw */
        /*       has to be maintained.                                   */
        if (_bcm_sbx_get_template_ref(unit, template) == 0) {
            /* update profile */
            (discard_state[unit] + template)->queue_size = queue_sz;
            (discard_state[unit] + template)->mtu_sz = 0;
        }
        else {
            if ((discard_state[unit] + template)->queue_size != queue_sz) {
                return(BCM_E_INTERNAL);
            }
        }

        for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
            /* check if this template/color is already initialized */
            if ((discard_state[unit] + template)->config[*(start_color + lcl_color)].max_thresh != 0) {
                if ( ((discard_state[unit] + template)->config[*(start_color + lcl_color)].
                                     min_thresh != discard->min_thresh) ||
                     ((discard_state[unit] + template)->config[*(start_color + lcl_color)].
                                     max_thresh != discard->max_thresh) ||
                     ((discard_state[unit] + template)->config[*(start_color + lcl_color)].
                                     drop_probability != discard->drop_probability) ||
                     ((discard_state[unit] + template)->config[*(start_color + lcl_color)].
                                     ecn_thresh != discard->ecn_thresh) ) {
                    return(BCM_E_INTERNAL);
                }
            }
        }

        for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
            /* check if this template/color is not initialized */
            if ((discard_state[unit] + template)->config[*(start_color + lcl_color)].max_thresh == 0) {
                (discard_state[unit] + template)->config[*(start_color + lcl_color)] = (*discard);
            }
        }

        /* update template reference count                               */
        /* NOTE: It is required that all the discard configuration of a  */
        /*       queue is re-played before moving to the next queue.     */
        if (queue != last_restore_queue[unit]) {
            (discard_state[unit] + template)->is_free = FALSE;
            _bcm_sbx_inc_template_ref(unit, template);
            _bcm_sbx_cosq_queue_update_template_gain(unit, template, gain, queue);
            last_restore_queue[unit] = queue;
        }
#endif /* BCM_EASY_RELOAD_SUPPORT */
    }

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    /* Check if the queue is already using a template */
    bcm_sbx_cosq_queue_get_template_gain(unit, queue, &template, &gain);

    if (soc_feature(unit, soc_feature_ingress_size_templates)) {    
	/* build a temp template */
	rv = _bcm_sbx_template_init(unit, cosq, &tmp_discard_state, min_queue_size,
				    queue_sz, (discard_state[unit]+template));
	if (rv != BCM_E_NONE) {
	    return rv;
	}
	
	/* enable all specified colors */
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    tmp_discard_state.config[*(start_color + lcl_color)] = (*discard);
	}

	/* new template matches to any existing template? */
	found =_bcm_sbx_template_match(unit, &tmp_discard_state, &match_template,
				       BCM_INT_SBX_LOCAL_WRED_TEMPLATE);
	if (found == TRUE) {
	    /* matches, alloc/dealloc */
	    _bcm_sbx_inc_template_ref(unit, match_template);	    

	    rv = _bcm_sbx_deallocate_template(unit, template);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	    
	    if (match_template != template) {
		rv =_bcm_sbx_cosq_queue_update_template_gain(unit, match_template, discard->gain, queue);
	    }
	    return rv;
	} else {
	    if ((_bcm_sbx_get_template_ref(unit, template) == 1) &&
		((discard_state[unit]+template)->template != BCM_INT_SBX_PREALLOCED_TEMPLATE)) {
		/* no matches, if this is the only queue using current template, and the template is 
		 * not pre-allocated, update it */

		/* need to update all colors if queue size changed, this seems would never happen */
		if (queue_sz != (discard_state[unit] + template)->queue_size) {		
		    /* need to update all colors */
		    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
		}

		update_template = template;
	    } else {
		/* no matches, there are other queue sharing the template, alocate a new one */
		rv = _bcm_sbx_allocate_template(unit, &allocated_template);
		if (rv != BCM_E_NONE) {
		    return(rv);
		}
		
		/* copy template and disable the color specified */
		rv = _bcm_sbx_template_copy(unit, (discard_state[unit]+allocated_template), 
					    (discard_state[unit]+template));
		if (rv != BCM_E_NONE) {
		    return rv;
		}
		
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    (discard_state[unit]+allocated_template)->config[*(start_color + lcl_color)] = (*discard);;
		}
		
		/* need to update all colors */
		_bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
		
		update_template = allocated_template;		

		/* dereference the old one */
		rv = _bcm_sbx_deallocate_template(unit, template);
		if (rv != BCM_E_NONE) {
		    return rv;
		}
	    }
	}

	/* wred parameter check */
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    rv = _bcm_sbx_discard_profile_consistency_check(unit,
							    ((allocated_template == -1) ?
							     &(tmp_discard_state.config[*(start_color + lcl_color)]) :
							     &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)]),
							    adj_queue_sz);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	}
	
	/* update template in hardware */
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
				      start_color[lcl_color], update_template, adj_queue_sz, min_queue_size,
				      ((allocated_template == -1) ?
				       &(tmp_discard_state.config[*(start_color + lcl_color)]):
				       &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)])));
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, mbcm_sbx_driver, Unit(%d), Error: 0x%x\n"),
		           FUNCTION_NAME(), unit, rv));
		goto err;
	    }
	}
	
	rv = bcm_sbx_cosq_template_pfc_config(unit, update_template);
	if (rv != BCM_E_NONE) {
	    goto err;
	}	

	/* update template software state */
	if ((allocated_template == -1))  {
	    rv = _bcm_sbx_template_init(unit, cosq, (discard_state[unit]+template), min_queue_size,
					queue_sz, &tmp_discard_state);
	    if (rv != BCM_E_NONE) {
		return rv;
	    }
	}
	
	/* collaps the new template if it matches other existing template */
	rv = _bcm_sbx_cosq_queue_update_template_gain(unit, update_template, discard->gain, queue);

	return(rv);

    } else {

	if (template == BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE) {
	    
	    /* allocate template */
	    rv = _bcm_sbx_allocate_template(unit, &allocated_template);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	    
	    /* update profile */
	    (discard_state[unit] + allocated_template)->queue_size = queue_sz;
	    
	    for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		(discard_state[unit] + allocated_template)->config[*(start_color + lcl_color)] = (*discard);
	    }
	} else {
	    /* make sure that there is a change in configuration */
	    if (_bcm_cosq_gport_discard_no_change(unit, template, gain,
						  color_val, discard,
						  queue_sz) == TRUE) {
		return(rv);
	    }
	    
	    if (_bcm_sbx_get_template_ref(unit, template) == 1) {
		/* Current template can be modified */
	    } else {
		/* allocate template */
		rv = _bcm_sbx_allocate_template(unit, &allocated_template);
		if (rv != BCM_E_NONE) {
		    return(rv);
		}
		
		/* update profile */
		(discard_state[unit] + allocated_template)->queue_size = queue_sz;
		
		for (lcl_color = 0; lcl_color < max_color; lcl_color++) {
		    (discard_state[unit] + allocated_template)->config[lcl_color] =
			(discard_state[unit] + template)->config[lcl_color];
		}
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    (discard_state[unit] + allocated_template)->config[*(start_color + lcl_color)] = (*discard);
		}
	    }
	}
	
	if (allocated_template == -1) {
	    update_template = template;
	    
	    tmp_discard_state.queue_size = queue_sz;
	    
	    if (queue_sz != (discard_state[unit] + template)->queue_size) {
		for (lcl_color = 0; lcl_color < max_color; lcl_color++) {
		    tmp_discard_state.config[lcl_color] = (discard_state[unit] + template)->config[lcl_color];
		}
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    tmp_discard_state.config[*(start_color + lcl_color)] = (*discard);
		}
		
		/* need to update all colors */
		_bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
	    }
	    else {
		for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		    tmp_discard_state.config[*(start_color + lcl_color)] = (*discard);
		}
	    }
	} else {
	    /* need to update all colors */
	    _bcm_sbx_cosq_discard_color_to_internal_color_range(unit, BCM_COSQ_DISCARD_COLOR_ALL, start_color, &nbr_color);
	    update_template = allocated_template;
	}
	
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    rv = _bcm_sbx_discard_profile_consistency_check(unit,
							    ((allocated_template == -1) ?
							     &(tmp_discard_state.config[*(start_color + lcl_color)]) :
							     &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)]),
							    adj_queue_sz);
	    if (rv != BCM_E_NONE) {
		return(rv);
	    }
	}
	
	for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
	    
	    /* adjust queue size */
	    if ( (is_queue_sz_fixed == FALSE) &&
		 (_bcm_sbx_cosq_queue_is_local(unit, queue) == FALSE) ) {
		rv = bcm_sbx_stk_get_modules_enabled(unit, &nbr_modules_enabled);
		if (rv == BCM_E_NONE) {
		    if (nbr_modules_enabled == 0) {
			nbr_modules_enabled = 1;
		    }
		} else {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, error in bcm_sbx_stk_get_modules_enabled"
		                           "), Unit(%d), Error: 0x%x\n"), FUNCTION_NAME(), unit, rv));
		    goto err;
		}
		max_thresh = ((allocated_template == -1) ?
			      tmp_discard_state.config[*(start_color + lcl_color)].max_thresh:
			      (discard_state[unit] + update_template)->config[*(start_color + lcl_color)].max_thresh);
		
		for (nbr_nodes = nbr_modules_enabled;
		     nbr_nodes < SOC_SBX_CFG(unit)->cfg_num_nodes; nbr_nodes++) {
		    if ((nbr_nodes * queue_sz) > max_thresh) {
			adj_queue_sz = nbr_nodes * queue_sz;
			break;
		    }
		}
	    }
	    
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
				      start_color[lcl_color], update_template, adj_queue_sz, min_queue_size,
				      ((allocated_template == -1) ?
				       &(tmp_discard_state.config[*(start_color + lcl_color)]):
				       &(discard_state[unit] + update_template)->config[*(start_color + lcl_color)])));
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, mbcm_sbx_driver, Unit(%d), Error: 0x%x\n"),
		           FUNCTION_NAME(), unit, rv));
		goto err;
	    }

	    rv = bcm_sbx_cosq_template_pfc_config(unit, update_template);
	    if (rv != BCM_E_NONE) {
		goto err;
	    }	

	}
	
	/* update template */
	if ((allocated_template == -1))  {
	    (discard_state[unit] + update_template)->queue_size = queue_sz;
	    for (lcl_color = 0; lcl_color < nbr_color; lcl_color++) {
		(discard_state[unit] + update_template)->config[*(start_color + lcl_color)] = tmp_discard_state.config[*(start_color + lcl_color)];
	    }
	} else {
	    if (template != BCM_SBX_COSQ_DISCARD_DISABLE_TEMPLATE) {
		_bcm_sbx_dec_template_ref(unit, template);
	    }
	}
	
	_bcm_sbx_template_collapse(unit, update_template, &match_template, BCM_INT_SBX_FIC_WRED_TEMPLATE);
	_bcm_sbx_cosq_queue_update_template_gain(unit, match_template, discard->gain, queue);
	
	return(rv);
    }

err:
    if (allocated_template != -1) {
        _bcm_sbx_deallocate_template(unit, allocated_template);
    }

    return(rv);
}


/***********************************
 *  get template given queue
 */
static void
bcm_sbx_cosq_queue_get_template_gain(int unit, int32 queue, int32 *p_template, int32 *p_gain)
{
    *p_template = queue_state[unit][queue].ingress.template;
    *p_gain = queue_state[unit][queue].ingress.gain;
}

/***********************************
 *  update template for queue
 */
static int
bcm_sbx_cosq_queue_set_template_gain(int unit, int32 queue, int32 template, int32 gain)
{
    int rv;

    queue_state[unit][queue].ingress.template = template;
    queue_state[unit][queue].ingress.gain = gain;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_template_gain, (unit, queue, template, gain));

    return rv;
}

/***********************************
 *  update template for queue
 */
static int
bcm_sbx_cosq_template_pfc_config(int unit, int32 template)
{
    int rv;
    int pfc_en, pfc_pg;

    if (!soc_feature(unit, soc_feature_priority_flow_control)) {
	return BCM_E_NONE;
    }

    if (discard_state[unit] == NULL) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, discard state not init on Unit(%d)\n"),
	           FUNCTION_NAME(), unit));
	return BCM_E_INIT;
    }

    if (template >= SOC_SBX_CFG(unit)->nDiscardTemplates) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, template id %d out of range on Unit(%d)\n"),
	           FUNCTION_NAME(), template, unit));
	return BCM_E_PARAM;
    }

    if ((discard_state[unit] + template)->is_free == TRUE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: %s, template id %d not in use on Unit(%d)\n"),
	           FUNCTION_NAME(), template, unit));
	return BCM_E_PARAM;
    }

    pfc_en = (discard_state[unit] + template)->pfc_en;
    pfc_pg = (discard_state[unit] + template)->pfc_pg;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_set_template_pfc, (unit, template, pfc_en, pfc_pg));

    return rv;
}

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
int
bcm_sbx_cosq_get_state(int unit, char *pbuf)
{
    int rv;
    char *pbuf_current = pbuf;
    int queue;
    int bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_discard_state_t *p_discardstate;
    int color;
    int template;

    for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

        p_bwstate = &bw_group_state[unit][bw_group];

        if (p_bwstate->in_use) {

            rv = sal_sprintf(pbuf_current, "bw_group:%d\n", bw_group);
            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "rv = %d\n"),
                           rv));
                return BCM_E_RESOURCE;
            }
            pbuf_current += rv;

            rv = sal_sprintf(pbuf_current, "  base_queue(%d) num_cos(%d) dest_node(%d) dest_port(%d)\n",
                             p_bwstate->base_queue, p_bwstate->num_cos, p_bwstate->dest_node,
                             p_bwstate->dest_port);

            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "rv = %d\n"),
                           rv));
                return BCM_E_RESOURCE;
            }
            pbuf_current += rv;

            rv = sal_sprintf(pbuf_current, "  dest_type(%s)\n",
                             p_bwstate->dest_type==BCM_INT_SBX_DEST_TYPE_UNICAST? "Unicast": "Multicast");

            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "rv = %d\n"),
                           rv));
                return BCM_E_RESOURCE;
            }
            pbuf_current += rv;


            for (queue = p_bwstate->base_queue; queue < ((p_bwstate->base_queue)+(p_bwstate->num_cos)); queue++) {

                p_qstate = &queue_state[unit][queue];

                if (p_qstate->state == BCM_INT_SBX_QUEUE_STATE_IN_USE) {

                    rv = sal_sprintf(pbuf_current, "queue:%d %s\n", queue,
                                     p_qstate->ingress.enabled?"ENABLED":"DISABLED");

                    if (rv < 0) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "rv = %d\n"),
                                   rv));
                        return BCM_E_RESOURCE;
                    }
                    pbuf_current += rv;

                    rv = sal_sprintf(pbuf_current, "%d: bw_group(%d)\n",
				     p_qstate->ingress.bw_mode,
				     p_qstate->bw_group);

                    if (rv < 0) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "rv = %d\n"),
                                   rv));
                        return BCM_E_RESOURCE;
                    }
                    pbuf_current += rv;
                }
            }
            rv = sal_sprintf(pbuf_current, "\n");
            if (rv < 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "rv = %d\n"),
                           rv));
                return BCM_E_RESOURCE;
            }
            pbuf_current += rv;
        }
        rv = sal_sprintf(pbuf_current, "\n");
        if (rv < 0) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "rv = %d\n"),
                       rv));
            return BCM_E_RESOURCE;
        }
    }


    if (discard_state[unit]) {
        /* Now get template/gain information */
        for (template = 0; template < SOC_SBX_CFG(unit)->nDiscardTemplates; template++)  {

            p_discardstate = (discard_state[unit] + template);

            if (!(p_discardstate->is_free)) {

                rv = sal_sprintf(pbuf_current, "template:%d %s\n", template,
                                 p_discardstate->is_free?"FREE":"IN USE");

                if (rv < 0) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "rv = %d\n"),
                               rv));
                    return BCM_E_RESOURCE;
                }
                pbuf_current += rv;

                for (color = 0; color < 3; color++) {

                    rv = sal_sprintf(pbuf_current, "color(%d) dp(%d) gain(%d)\n",
                                     color,
                                     p_discardstate->config[color].drop_probability,
                                     p_discardstate->config[color].gain);

                    if (rv < 0) {
                        LOG_ERROR(BSL_LS_BCM_COMMON,
                                  (BSL_META_U(unit,
                                              "rv = %d\n"),
                                   rv));
                        return BCM_E_RESOURCE;
                    }
                    pbuf_current += rv;
                }
            }
        }
    }

    if (soc_feature(unit, soc_feature_node)) {
        if ((soc_feature(unit, soc_feature_standalone)) || (soc_feature(unit, soc_feature_hybrid))) {
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_state_get, (unit, pbuf_current));
        }
    }
    return BCM_E_NONE;
}

#endif /* EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* EASY_RELOAD_SUPPORT */

/************************/
/* SHARED FUNCTIONS     */
/************************/
/* verify the bandwidth for the queues in the bw group */
int32
bcm_sbx_cosq_bw_group_verify_queues(int unit, int bw_group) {

    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 available_bw_kbps;

    p_bwstate = &bw_group_state[unit][bw_group];
    available_bw_kbps = _bcm_sbx_cosq_bw_group_get_available_bw(unit, bw_group);

    if ( available_bw_kbps < 0 ) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: Not enough bw_group bandwidth for flow guarantees: group BW(%d kbps) Available BW(%d kbps).\n"),
                   available_bw_kbps, p_bwstate->dest_port_kbits_sec_max));
        return BCM_E_CONFIG;
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "bandwidth available(%d)\n"),
              available_bw_kbps));

    
    /* see sbFabPortLogical_VerifyFlowsQosCompatibility() */

    return BCM_E_NONE;
}
/***********************************
 *  Get the number of SP queues
 */
int32
bcm_sbx_cosq_get_num_sp_queues(int unit, int bw_group) {

    int32 sp_queues = 0;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 num_cos;
    int32 queue;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int32 base_queue;

    p_bwstate = &bw_group_state[unit][bw_group];
    num_cos = p_bwstate->num_cos;
    base_queue= p_bwstate->base_queue;

    for ( queue=base_queue; queue<base_queue + num_cos; queue++ ) {

        p_qstate = &queue_state[unit][queue];

        if (p_qstate->ingress.bw_mode == BCM_COSQ_SP) {
            sp_queues++;
        }
    }

    return (sp_queues);
}

/***********************************
 *  Get the number of flows in the BAG
 */
int32
bcm_sbx_cosq_get_num_queues_in_bag(int unit, int bw_group,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
                                   int *num_queues, int *start_queue) {

    int32 queues_in_bag = 0;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 num_cos;
    int32 queue;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int32 base_queue;
    int is_new_symantics = FALSE, starting_queue;
    int32 max_queue_in_bag = -1;

    p_bwstate = &bw_group_state[unit][bw_group];
    num_cos = p_bwstate->num_cos;
    base_queue= p_bwstate->base_queue;
    starting_queue = -1;

    for ( queue=base_queue; queue < base_queue+num_cos; queue++ ) {

        p_qstate = &queue_state[unit][queue];

        if (p_qstate->state == BCM_INT_SBX_QUEUE_STATE_IN_USE) {
            if (soc_feature(unit, soc_feature_standalone)) {
                if ((p_qstate->ingress.bw_mode == BCM_COSQ_SP) ||
                    (p_qstate->ingress.bw_mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING)){
                    queues_in_bag++;
		    if (queue > max_queue_in_bag) {
			max_queue_in_bag = queue;
		    }
                }
            } else { /* FIC mode */

                if ( (queue_region != bcm_sbx_cosq_queue_region_local) &&
                     ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
                     (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
                     (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) ) {
                    if ( (p_qstate->ingress.bw_mode == BCM_COSQ_SP) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_AF) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP0) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP1) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP2) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP3) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP4) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP5) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP6) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_GSP7) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_EF) ) {

                        queues_in_bag++;
			if (queue > max_queue_in_bag) {
			    max_queue_in_bag = queue;
			}
          
                        /* go to new symantic if its is GSP scheduling discipline */
                        if ( (p_qstate->ingress.bw_mode == BCM_COSQ_GSP0) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP1) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP2) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP3) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP4) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP5) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP6) ||
                             (p_qstate->ingress.bw_mode == BCM_COSQ_GSP7) ) {
                            is_new_symantics = TRUE;
                        }

                        if (starting_queue == -1) {
                            starting_queue =  queue;
                        }
                    }
                } else {
                    if ((p_qstate->ingress.bw_mode == BCM_COSQ_SP) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) ||
                        (p_qstate->ingress.bw_mode == BCM_COSQ_AF)) {
                        queues_in_bag++;
			if (queue > max_queue_in_bag) {
			    max_queue_in_bag = queue;
			}
                    }
                }
            }
        }
    }

    /* go to new symantic if its is GSP scheduling discipline */
    if (is_new_symantics == TRUE) {
        if ( ((starting_queue % 2) != 0) &&
               ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
                     (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) ) {
            if (starting_queue > base_queue) {
                starting_queue = starting_queue - 1;
                queues_in_bag++;
            }
        }
	(*num_queues) = queues_in_bag;
    } else {
	/* return max queue offset in bag, this would allow BE queues not in the end of queue group */
	if (max_queue_in_bag >= 0) {
	    (*num_queues) = max_queue_in_bag - base_queue + 1;
	} else {
	    (*num_queues) = 0;
	}
    }

    (*start_queue) = starting_queue;

    return (queues_in_bag);
}


/***********************************
 *  Update the WFQ weights
 */

int
bcm_sbx_cosq_update_given_weights(int unit, int bw_group) {

    int32 num_cos;
    int32 queue;
    bcm_sbx_cosq_queue_state_t *p_qstate = 0;
    uint32 total_q_weights = 0;
    int32 total_given_weights = 0;
    uint32 weight = 0;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int32 base_queue;
    uint32 weight_sum;

    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE ||
	(SOC_SBX_CFG(unit)->bTmeMode && SOC_IS_SBX_QE2000(unit))) {
      weight_sum = 128;
    } else {
      weight_sum = 256;
    }

    p_bwstate = &bw_group_state[unit][bw_group];
    base_queue= p_bwstate->base_queue;
    num_cos =  p_bwstate->num_cos;

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "update given weights bw_group(%d) num_cos(%d)\n"),
              bw_group, num_cos));

    for ( queue=base_queue; queue< base_queue+num_cos; queue++ ) {

        p_qstate = &queue_state[unit][queue];

        total_q_weights += _bcm_sbx_cosq_get_wfq_weight(unit, queue);
    }

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "total_q_weights(%d)\n"),
              total_q_weights));

    if ( total_q_weights ) {
        /* we have one or more WFQ flows, set the given weights... */
        for ( queue=base_queue; queue< base_queue+num_cos; queue++ ) {

            p_qstate = &queue_state[unit][queue];
            weight = _bcm_sbx_cosq_get_wfq_weight(unit, queue);

            if ( weight != 0 ) {
                /* Set the given weight for this flow */
                p_qstate->ingress.given_weight = (weight_sum*weight)/total_q_weights;
                total_given_weights += p_qstate->ingress.given_weight;
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "queue(%d) weight(%d) "),
                          queue, weight));
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "total_given_weights(%d) given_weight(%d)\n"),
                          total_given_weights, p_qstate->ingress.given_weight));
	    } else {
	      /* SDK-31409 stale gamma value */
	      p_qstate->ingress.given_weight = 0;
	    }
        }

        if ( total_given_weights != weight_sum ) {
            /* we need the given weight to sum to weight_sum, so adjust our weights */
            while ( total_given_weights > weight_sum ) {
                /* reduce weights */
                for ( queue=base_queue; queue < base_queue+num_cos; queue++ ) {

                    p_qstate = &queue_state[unit][queue];
                    p_qstate->ingress.given_weight--;
                    total_given_weights--;
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "queue(%d) reduce given_weight(%d) total_given_weight(%d)\n"),
                              queue, p_qstate->ingress.given_weight, total_given_weights));

                    if ( total_given_weights <= weight_sum ) {
                        break;
                    }
                }
            }

            while ( total_given_weights < weight_sum ) {
                /* increase weights */
                for ( queue=base_queue; queue< base_queue+num_cos; queue++ ) {

                    p_qstate = &queue_state[unit][queue];
                    p_qstate->ingress.given_weight++;
                    total_given_weights++;
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "queue(%d) increase given_weight(%d) total_given_weights(%d)\n"),
                              queue, p_qstate->ingress.given_weight, total_given_weights));
                    if ( total_given_weights >= weight_sum ) {
                        break;
                    }
                }
            }
        }

        for (queue=base_queue; queue<base_queue + num_cos; queue++) {

            p_qstate = &queue_state[unit][queue];
            weight = _bcm_sbx_cosq_get_wfq_weight(unit, queue);

            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "queue(%d) weight(%d) given_weight(%d) total_given_weights(%d)\n"),
                      queue,
                      weight, p_qstate->ingress.given_weight, total_given_weights));
        }

        if (total_given_weights != weight_sum) {
             LOG_INFO(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "queue(%d) weight(%d) given_weight(%d) total_given_weights(%d) out of range - must be %d\n"),
                       queue, weight, p_qstate->ingress.given_weight, total_given_weights, weight_sum));
            return BCM_E_CONFIG;
        }
    }
    return BCM_E_NONE;
}

/***********************************
 *  Get the bandwidth guarantee in kbps, if none, return 0
 */
uint32
bcm_sbx_cosq_get_bw_guarantee(int unit, int queue) {

    bcm_sbx_cosq_queue_state_t *p_qstate;

    p_qstate = &queue_state[unit][queue];

    if (p_qstate->state == BCM_INT_SBX_QUEUE_STATE_IN_USE) {
        switch ( p_qstate->ingress.bw_mode ) {
            case BCM_COSQ_AF :
            case BCM_COSQ_EF :
                return p_qstate->ingress.bw_value.guarantee_kbps;
                break;
            default:
                return 0;
                break;
        }
    }
    return 0;
}


/***********************************
 *  queue is available
 */
int
bcm_sbx_cosq_queue_available(int unit, int32 queue)
{
    if (queue_state[unit][queue].state == BCM_INT_SBX_QUEUE_STATE_AVAILABLE) {
 return TRUE;
    }
    return FALSE;
}

int
bcm_sbx_update_templates(int unit, int nbr_modules)
{
    int rv = BCM_E_NONE, result = BCM_E_NONE;
    int nbr_template, template, lcl_color;
    int max_color;
    int is_queue_sz_fixed = TRUE, nbr_nodes, nbr_modules_enabled;
    uint32 queue_sz, adj_queue_sz, max_thresh;


    /* functionality only required on Arbiter */
    if (!soc_feature(unit, soc_feature_arbiter)) {
        return(rv);
    }

    if (!(soc_feature(unit, soc_feature_discard_ability) ||
          soc_feature(unit, soc_feature_ingress_size_templates))) {
        return(rv);
    }

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    /* if it is a fixed queue size configuration, no update is required */
    queue_sz = SOC_SBX_CFG(unit)->discard_queue_size;
    if (queue_sz == 0) {
        is_queue_sz_fixed = FALSE;
    }
    if (is_queue_sz_fixed == TRUE) {
        return(rv);
    }

    nbr_modules_enabled = (nbr_modules > 0) ? nbr_modules : 1;

    nbr_template = SOC_SBX_CFG(unit)->nDiscardTemplates;
    for (template = 0; template < nbr_template; template++) {

        /* check if it is the default template (wred not enabled) */
        if ( (discard_state[unit] + template)->template == BCM_INT_SBX_PREALLOCED_TEMPLATE ) {
            continue;
        }

        /* check if there are no reference's to the template */
        if (_bcm_sbx_get_template_ref(unit, template) == 0) {
            continue;
        }

        for (lcl_color = 0; lcl_color < max_color; lcl_color++) {

            /* adjust size. The algorithm should match the algorithm used in discard API */
            queue_sz = (discard_state[unit] + template)->queue_size;
            adj_queue_sz = SOC_SBX_CFG(unit)->cfg_num_nodes * queue_sz;

            max_thresh = (discard_state[unit] + template)->config[lcl_color].max_thresh;

            for (nbr_nodes = nbr_modules_enabled; nbr_nodes < SOC_SBX_CFG(unit)->cfg_num_nodes; nbr_nodes++) {
                if ((nbr_nodes * queue_sz) > max_thresh) {
                    adj_queue_sz = nbr_nodes * queue_sz;
                    break;
                }
            }

            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
                                    lcl_color, template, adj_queue_sz,
                                    (discard_state[unit] + template)->min_queue_size,
                                    &(discard_state[unit] + template)->config[lcl_color]));
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, mbcm_cosq_gport_discard_set, Unit(%d) Err: 0x%x\n"),
                           FUNCTION_NAME(), unit, rv));
                result = rv;
            }
        }

	rv = bcm_sbx_cosq_template_pfc_config(unit, template);
	if (rv != BCM_E_NONE) {
	    return rv;
	}	
    }
    return(result);
}

int
bcm_sbx_update_template(int unit, int template)
{
    int rv = BCM_E_NONE, result = BCM_E_NONE;
    int nbr_template;
    int lcl_color, max_color;

    if (!(soc_feature(unit, soc_feature_discard_ability) ||
          soc_feature(unit, soc_feature_ingress_size_templates))) {
        return(rv);
    }

    if (soc_feature(unit, soc_feature_discard_ability_color_black)) {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS;
    } else {
	max_color = BCM_SBX_COSQ_DISCARD_MAX_COLORS - 1;
    }

    nbr_template = SOC_SBX_CFG(unit)->nDiscardTemplates;
    if ((template >= 0) && (template < nbr_template)) {
        for (lcl_color = 0; lcl_color < max_color; lcl_color++) {
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_discard_set, (unit, 0, 0,
				      lcl_color, template, (discard_state[unit] + template)->queue_size,
				      (discard_state[unit] + template)->min_queue_size,
				      &(discard_state[unit] + template)->config[lcl_color]));
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, mbcm_cosq_gport_discard_set, Unit(%d) Err: 0x%x\n"),
                           FUNCTION_NAME(), unit, rv));
                result = rv;
            }
        }
	rv = bcm_sbx_cosq_template_pfc_config(unit, template);
	if (rv != BCM_E_NONE) {
	    return rv;
	}
    }

    return(result);
}


/* Create a wrapper so lower level functions can accesss */
int
bcm_sbx_cosq_get_base_queue_from_gport(int unit, bcm_gport_t gport, int *p_base_queue, int *p_num_cos_levels)
{

      return(_bcm_sbx_cosq_get_base_queue_from_gport(unit, gport, p_base_queue, p_num_cos_levels));

}

int
bcm_sbx_cosq_get_gport_cos_from_qid(int unit, int queue, bcm_gport_t *gport, bcm_cos_queue_t *dest_cosq)
{
    int rv = BCM_E_NONE; 
    int base_queue, sysport;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;

   if (bcm_sbx_cosq_queue_available(unit, queue)) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "ERROR: Queue(%d) not allocated\n"),
                 queue));
      return BCM_E_PARAM;
    }

    p_qstate = &queue_state[unit][queue];
    p_bwstate = &bw_group_state[unit][p_qstate->bw_group];

    base_queue = p_bwstate->base_queue;
    sysport = p_qstate->sysport;

    BCM_GPORT_UCAST_QUEUE_GROUP_SET(*gport, base_queue);
    BCM_GPORT_UCAST_QUEUE_GROUP_SYSQID_SET(*gport, sysport, base_queue);

    return rv;
}

int
bcm_sbx_cosq_control_set(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                                           bcm_cosq_control_t type, int arg)
{
    int rv = BCM_E_UNAVAIL;
    bcm_sbx_cosq_control_t control_info;


    BCM_SBX_LOCK(unit);


    if (!(BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
                       BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
                       BCM_GPORT_IS_MCAST_QUEUE_GROUP(port) ||
                       BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(port))) {
	/* put in dummy control info, not used for these 2 types */
	control_info.u.conn.queue = 0;
	control_info.u.conn.queue_region = bcm_sbx_cosq_queue_region_local;
	rv = BCM_E_NONE;
    } else {	

        if ( (type == bcmCosqControlQselOffset) && _bcm_sbx_cosq_is_all(unit, port) ) {
	    /* put in dummy control info, not used for these 2 types */
	    control_info.u.conn.queue = 0;
	    control_info.u.conn.queue_region = bcm_sbx_cosq_queue_region_local;
	    rv = BCM_E_NONE;
        }
        else {
	    /* determine QID */
	    rv = _bcm_sbx_cosq_get_queue_from_gport(unit,
						port,
						&(control_info.u.conn.queue),
						cosq,
						&(control_info.u.conn.num_cos));
	    if (rv != BCM_E_NONE) {
		BCM_SBX_UNLOCK(unit);
	        return(rv);
	    }
	    /* get queue region/type */
	    
	    rv = _bcm_sbx_cosq_queue_type_get(unit,
					  control_info.u.conn.queue,
					  &(control_info.u.conn.queue_region));
        }
    }

    if (BCM_E_NONE == rv) {
        switch (type) {
            default:
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_control_set, (unit, port, cosq,
                                  type, arg, &control_info));
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, mbcm_cosq_gport_control_set, Unit(%d) Err: 0x%x\n"),
                               FUNCTION_NAME(), unit, rv));
                }
                break;
        }
    }

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int
bcm_sbx_cosq_control_get(int unit, bcm_gport_t port, bcm_cos_queue_t cosq,
                                          bcm_cosq_control_t type, int *arg)
{
    int rv = BCM_E_UNAVAIL;
    bcm_sbx_cosq_control_t control_info;

    BCM_SBX_LOCK(unit);

    if (!(BCM_GPORT_IS_UCAST_QUEUE_GROUP(port) ||
                       BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(port) ||
                       BCM_GPORT_IS_MCAST_QUEUE_GROUP(port) ||
                       BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(port))) {
	/* put in dummy control info, not used for these 2 types */
	control_info.u.conn.queue = 0;
	control_info.u.conn.queue_region = bcm_sbx_cosq_queue_region_local;
	rv = BCM_E_NONE;
    } else {	
	
	/* determine QID */
	rv = _bcm_sbx_cosq_get_queue_from_gport(unit,
						port,
						&(control_info.u.conn.queue),
						cosq,
						&(control_info.u.conn.num_cos));
	if (rv != BCM_E_NONE) {
            BCM_SBX_UNLOCK(unit);
	    return(rv);
	}
	/* get queue region/type */
	
	rv = _bcm_sbx_cosq_queue_type_get(unit,
					  control_info.u.conn.queue,
					  &(control_info.u.conn.queue_region));
    }

    if (BCM_E_NONE == rv) {
        switch (type) {
            default:
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_control_get, (unit, port, cosq,
                                  type, arg, &control_info));
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, mbcm_cosq_gport_control_get, Unit(%d) Err: 0x%x\n"),
                               FUNCTION_NAME(), unit, rv));
                }
                break;
        }
    }

    BCM_SBX_UNLOCK(unit);
    return rv;
}

static int
_bcm_sbx_cosq_scheduler_allocate(int unit, bcm_gport_t scheduler_requested, int egress,
                                 bcm_gport_t *p_scheduler_gport)
{
    int logical_scheduler;
    bcm_sbx_cosq_ingress_scheduler_state_t *p_i_scheduler;
    bcm_sbx_cosq_egress_scheduler_state_t *p_e_scheduler;

    p_i_scheduler = SOC_SBX_STATE(unit)->ingress_scheduler_state;
    p_e_scheduler = SOC_SBX_STATE(unit)->egress_scheduler_state;

    *p_scheduler_gport = -1;
    if (scheduler_requested == BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {

        /* allocate a logical handle based on the egress */
        if (egress) {
            if (p_e_scheduler == NULL) {
                return BCM_E_UNAVAIL;
            }
            for (logical_scheduler = 0; logical_scheduler < SOC_SBX_CFG(unit)->num_egress_scheduler;
                 logical_scheduler++) {
                if ( (p_e_scheduler + logical_scheduler)->in_use == FALSE ) {
                    *p_scheduler_gport = logical_scheduler;
                    (p_e_scheduler + logical_scheduler)->in_use = TRUE;
                    (p_e_scheduler + logical_scheduler)->level = -1;
                    (p_e_scheduler + logical_scheduler)->node = -1;
                    break;
                }
            }
        } else {
            if (p_i_scheduler == NULL) {
                return BCM_E_UNAVAIL;
            }
            for (logical_scheduler = 0; logical_scheduler < SOC_SBX_CFG(unit)->num_ingress_scheduler;
                 logical_scheduler++) {
                if ( (p_i_scheduler + logical_scheduler)->in_use == FALSE ) {
                    *p_scheduler_gport = logical_scheduler;
                    (p_i_scheduler + logical_scheduler)->in_use = TRUE;
                    (p_i_scheduler + logical_scheduler)->level = -1;
                    (p_i_scheduler + logical_scheduler)->node = -1;
                    break;
                }
            }
        }
    } else {
        if ( !BCM_GPORT_IS_SCHEDULER(scheduler_requested) ) {
            return BCM_E_PARAM;
        } else {
            logical_scheduler = BCM_INT_SBX_SCHEDULER_ID_GET(scheduler_requested);
        }

        if (BCM_INT_SBX_SCHEDULER_IS_EGRESS(scheduler_requested)) {
            egress = TRUE;
            if (p_e_scheduler == NULL) {
                return BCM_E_UNAVAIL;
            }
            if ( (logical_scheduler < 0) || (logical_scheduler >= SOC_SBX_CFG(unit)->num_egress_scheduler) ) {
                return BCM_E_PARAM;
            }
            if ( (p_e_scheduler + logical_scheduler)->in_use == FALSE ) {
                *p_scheduler_gport = logical_scheduler;
                (p_e_scheduler + logical_scheduler)->in_use = TRUE;
                (p_e_scheduler + logical_scheduler)->level = -1;
                (p_e_scheduler + logical_scheduler)->node = -1;
            }
        } else {
            egress = FALSE;
            if (p_i_scheduler == NULL) {
                return BCM_E_UNAVAIL;
            }
            if ( (logical_scheduler < 0) || (logical_scheduler >= SOC_SBX_CFG(unit)->num_ingress_scheduler) ) {
                return BCM_E_PARAM;
            }
            if ( (p_i_scheduler + logical_scheduler)->in_use == FALSE ) {
                *p_scheduler_gport = logical_scheduler;
                (p_i_scheduler + logical_scheduler)->in_use = TRUE;
		(p_i_scheduler + logical_scheduler)->level = -1;
		(p_i_scheduler + logical_scheduler)->node = -1;
            }
        }
    }

    /* return failure if not available */
    if (*p_scheduler_gport < 0) {
        *p_scheduler_gport = BCM_GPORT_INVALID;
        return BCM_E_RESOURCE;
    } else {
        /* convert to scheduler handle with proper type and egress bit */
        BCM_INT_SBX_SCHEDULER_GPORT_SET(*p_scheduler_gport, egress, *p_scheduler_gport);
        return BCM_E_NONE;
    }
}


static int
_bcm_sbx_cosq_scheduler_free(int unit, int egress, int gport)
{
    bcm_sbx_cosq_ingress_scheduler_state_t *p_i_scheduler;
    bcm_sbx_cosq_egress_scheduler_state_t *p_e_scheduler;
    int scheduler, level, node;
    int rv = BCM_E_NONE;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode == TRUE) {
	/* physical handle */
	/* free the physical resources */
	level = BCM_INT_SBX_SCHEDULER_LEVEL_GET(gport);
	node = BCM_INT_SBX_SCHEDULER_NODE_GET(gport);
	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_free, (unit,
				  level, node, egress));

	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to free %d level %d scheduler nodes %d for gport 0x%x, Unit(%d)\n"),
	               FUNCTION_NAME(), egress, level, node, gport, unit));
	    return rv;
	}
    } else {
	/* logical handle */
	/* free the handle and free the hardware resource if not detached yet */
	p_i_scheduler = SOC_SBX_STATE(unit)->ingress_scheduler_state;
	p_e_scheduler = SOC_SBX_STATE(unit)->egress_scheduler_state;
	scheduler = BCM_INT_SBX_SCHEDULER_ID_GET(gport);

        if (egress == TRUE) {
            if (p_e_scheduler == NULL) {
                return BCM_E_UNAVAIL;
            }
            if ( (scheduler < 0) || (scheduler >= SOC_SBX_CFG(unit)->num_egress_scheduler) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: egress scheduler id %d out of range\n"),
		           scheduler));
                return BCM_E_PARAM;
            }
            if ( (p_e_scheduler + scheduler)->in_use == FALSE ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: egress scheduler %d not used\n"),
		           scheduler));
		return BCM_E_PARAM;
            } else {
		/* hardware resource should already be freed during detach
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "%s: can not find egress scheduler for gport 0x%x on unit %d\n"),
		              FUNCTION_NAME(), gport, unit));
		    return rv;	    
		}

		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_free, (unit,
					  level, node, TRUE));
		
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free %d level %d egress scheduler nodes %d for gport 0x%x, Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, gport, unit));
		    return rv;
		}      
		*/
                (p_e_scheduler + scheduler)->in_use = FALSE;
		(p_e_scheduler + scheduler)->level = -1;
		(p_e_scheduler + scheduler)->node = -1;
	    }
        } else {
            if (p_i_scheduler == NULL) {
                return BCM_E_UNAVAIL;
            }
            if ( (scheduler < 0) || (scheduler >= SOC_SBX_CFG(unit)->num_ingress_scheduler) ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: ingress scheduler id %d out of range\n"),
		           scheduler));
                return BCM_E_PARAM;
            }
            if ( (p_i_scheduler + scheduler)->in_use == FALSE ) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: ingress scheduler %d not used\n"),
		           scheduler));
		return BCM_E_PARAM;
	    } else {
		/* hardware resource should already be freed during detach
		rv = bcm_sbx_port_get_scheduler(unit, gport, &level, &node);
		if (rv != BCM_E_NONE) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "%s: can not find ingress scheduler for gport 0x%x on unit %d\n"),
		              FUNCTION_NAME(), gport, unit));
		    return rv;	    
		}

		rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_free, (unit,
					  level, node, TRUE));
		
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: %s, failed to free %d level %d ingress scheduler nodes %d for gport 0x%x, Unit(%d)\n"),
		               FUNCTION_NAME(), level, node, gport, unit));
		    return BCM_E_RESOURCE;
		}	
		*/

                (p_i_scheduler + scheduler)->in_use = FALSE;
		(p_i_scheduler + scheduler)->level = -1;
		(p_i_scheduler + scheduler)->node = -1;
		(p_i_scheduler + scheduler)->num_childs = 0;
            }
        }
    }

    return rv;
}

static int
_bcm_sbx_cosq_multipath_allocate(int unit, bcm_gport_t multipath_requested, int egress,
                                 bcm_gport_t *p_multipath_gport)
{
    int rv = BCM_E_NONE;
    int logical_multipath;
    int member_group;
    bcm_sbx_cosq_ingress_multipath_state_t *p_i_multipath;
    bcm_sbx_cosq_egress_multipath_state_t *p_e_multipath;

    p_i_multipath = SOC_SBX_STATE(unit)->ingress_multipath_state;
    p_e_multipath = SOC_SBX_STATE(unit)->egress_multipath_state;

    *p_multipath_gport = -1;
    if (multipath_requested == BCM_INT_SBX_REQUESTED_MULTIPATH_AUTO_SELECT) {
        /* allocate a logical handle based on the egress */
	if (egress) {
            if (p_e_multipath == NULL) {
                return BCM_E_UNAVAIL;
            }
	    /* call mbcm layer to allocate egress multipath shaper */
	    logical_multipath = -1;
	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_allocate, (unit,
				      egress, &logical_multipath));
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: multipath allocate failed error(%d)\n"),
		           rv));
	    } else {
		/* use the allocated physical resource as the multipath id */
		if ((p_e_multipath + logical_multipath)->in_use) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "ERROR: multipath shaper already allocated\n")));
		    rv = BCM_E_INTERNAL;
		} else {
		    (p_e_multipath + logical_multipath)->in_use = TRUE;
		    (p_e_multipath + logical_multipath)->node = logical_multipath;
		    for (member_group = 0;
			 member_group < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE;
			 member_group++) {
			(p_e_multipath + logical_multipath)->member[member_group] = 0;
		    }
		    *p_multipath_gport = logical_multipath;
		}
	    }
	} else {
	    /* allocate logical multipath id only */
            if (p_i_multipath == NULL) {
                return BCM_E_UNAVAIL;
            }
            for (logical_multipath = 0; logical_multipath < SOC_SBX_CFG(unit)->num_ingress_multipath;
                 logical_multipath++) {
                if ( (p_i_multipath + logical_multipath)->in_use == FALSE ) {
                    *p_multipath_gport = logical_multipath;
                    (p_i_multipath + logical_multipath)->in_use = TRUE;
                    (p_i_multipath + logical_multipath)->level = -1;
                    (p_i_multipath + logical_multipath)->node = -1;
                    (p_i_multipath + logical_multipath)->num_nodes = 0;
                    break;
                }
            }
	}
    } else {
	/* Not support user manangement of multipath shaper */
	return BCM_E_UNAVAIL;
    }

    /* return failure if not available */
    if (*p_multipath_gport < 0) {
        *p_multipath_gport = BCM_GPORT_INVALID;
        return BCM_E_RESOURCE;
    } else {
        /* convert to multipath handle with proper type and egress bit */
        BCM_INT_SBX_MULTIPATH_GPORT_SET(*p_multipath_gport, egress, *p_multipath_gport);
        return BCM_E_NONE;
    }    
}

static int
_bcm_sbx_cosq_multipath_free(int unit, int egress, int gport)
{
    bcm_sbx_cosq_ingress_multipath_state_t *p_i_multipath;
    bcm_sbx_cosq_egress_multipath_state_t *p_e_multipath;
    int multipath, index;
    uint32 member = 0;
    int rv = BCM_E_NONE;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    /* free the handle and free the hardware resource if not detached yet */
    p_i_multipath = SOC_SBX_STATE(unit)->ingress_multipath_state;
    p_e_multipath = SOC_SBX_STATE(unit)->egress_multipath_state;
    multipath = BCM_INT_SBX_MULTIPATH_ID_GET(gport);
    
    if (egress == TRUE) {
	if (p_e_multipath == NULL) {
	    return BCM_E_UNAVAIL;
	}
	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_egress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
	if ( (p_e_multipath + multipath)->in_use == FALSE ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath %d not used\n"),
	               multipath));
	    return BCM_E_PARAM;
	} else {
	    for (index = 0; index < BCM_INT_SBX_MAX_EGRESS_MUTIPATH_MEMBER_ARRAY_SIZE; index++) {
		member |= (p_e_multipath + multipath)->member[index];
	    }

	    if (member != 0) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: egress multipath %d still has member in it, can not be freed\n"),
		           multipath));
		return BCM_E_PARAM;
	    }

	    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_free, (unit,
				      TRUE, multipath));
	       
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, failed to free egress multipath %d for gport 0x%x, Unit(%d)\n"),
		           FUNCTION_NAME(), multipath, gport, unit));
		return rv;
	    }      
	    (p_e_multipath + multipath)->in_use = FALSE;
	    (p_e_multipath + multipath)->node = -1;
	}
    } else {
	if (p_i_multipath == NULL) {
	    return BCM_E_UNAVAIL;
	}
	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_ingress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
	if ( (p_i_multipath + multipath)->in_use == FALSE ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath %d not used\n"),
	               multipath));
	    return BCM_E_PARAM;
	} else if ((p_i_multipath + multipath)->num_nodes != 0) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath %d still has member in it, can not be freed\n"),
	               multipath));
	    return BCM_E_PARAM;
	} else {
	    (p_i_multipath + multipath)->in_use = FALSE;
	    (p_i_multipath + multipath)->level = -1;
	    (p_i_multipath + multipath)->node = -1;
	    (p_i_multipath + multipath)->num_nodes = 0;
	}
    }

    return rv;
}

int 
bcm_sbx_cosq_egress_group_node_allocate(int unit, bcm_gport_t eg_requested, int modid, int child_port,
					int num_fifos, bcm_gport_t *p_eg_gport)
{
    int node = 0, logical_eg = -1, level0_node = -1;
    bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
    bcm_sbx_subport_info_t *sp_info;
    int32 requested_fcd = BCM_INT_SBX_REQUESTED_FCD_AUTO_SELECT;
    int fcd = BCM_INT_SBX_INVALID_FCD, rv = BCM_E_NONE;
    int idx = 0, enum_fifos = 0;

    if (SOC_SBX_STATE(unit)->port_state->subport_info == 0) {
	return BCM_E_UNAVAIL;
    }

    if (num_fifos > SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Only up to %d fifos per egress group allowed\n"),
	           SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;	    
    }

    node = BCM_STK_MOD_TO_NODE(modid);
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[(node * SB_FAB_DEVICE_MAX_FABRIC_PORTS) + child_port]);

    /*
     * Do not allocate CPU handle per node more than once
     */
    if ((sp_info->valid == TRUE) && 
        (sp_info->egroup[0].num_fifos != 0) && 
        (eg_requested == SB_FAB_DEVICE_SIRIUS_CPU_HANDLE)) {
        return BCM_E_NONE;
    }

    sp_info->valid = TRUE;

    /*
     * find available egroup
     */
    for (idx=0; idx < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; idx++) {
	if (sp_info->egroup[idx].egroup_gport == BCM_GPORT_INVALID)
	    break;
	enum_fifos += sp_info->egroup[idx].num_fifos;
	if ((enum_fifos + num_fifos) > SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, %d fifos exceeds maximum allowable fifos for child_port %d unit %d\n"),
	               FUNCTION_NAME(), num_fifos, child_port, unit));
	    return BCM_E_PARAM;
	}
    }

    if (idx == SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, max fifos in use for child_port %d unit %d\n"),
	               FUNCTION_NAME(), child_port, unit));
	    return BCM_E_RESOURCE;
    }

    p_eg = &(egress_group_state[unit][node * SOC_SBX_CFG(unit)->num_egress_group]);

    *p_eg_gport = -1;

    if (eg_requested == BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
	
	if (p_eg == NULL) {
	    return BCM_E_UNAVAIL;
	}
	for (logical_eg = 0; logical_eg < SOC_SBX_CFG(unit)->num_egress_group;
	     logical_eg++) {
	    if ( (p_eg + logical_eg)->eg_in_use == FALSE ) {
		*p_eg_gport = logical_eg;
		(p_eg + logical_eg)->eg_in_use = TRUE;
		(p_eg + logical_eg)->child_port = child_port;
		break;
	    }
	}
	
    } else {
	if (eg_requested >= SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS ) {
	    return BCM_E_PARAM;
	} else {
	    logical_eg = eg_requested;
	}
	
	if (p_eg == NULL) {
	    /* coverity[dead_error_line] */
	    return BCM_E_UNAVAIL;
	}
	if ( (logical_eg < 0) || (logical_eg >= SOC_SBX_CFG(unit)->num_egress_group) ) {
	    return BCM_E_PARAM;
	}
	if ( (p_eg + logical_eg)->eg_in_use == FALSE ) {
	    *p_eg_gport = logical_eg;
	    (p_eg + logical_eg)->eg_in_use = TRUE;
	    (p_eg + logical_eg)->child_port = child_port;
	    (p_eg + logical_eg)->eg_scheduler = -1;
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress group %d in use, Unit(%d)\n"),
	               FUNCTION_NAME(), logical_eg, unit));
	    return BCM_E_RESOURCE;
	}

    }

    /* return failure if not available */
    if (*p_eg_gport < 0) {
        *p_eg_gport = BCM_GPORT_INVALID;
        return BCM_E_RESOURCE;
    } else {
	BCM_GPORT_EGRESS_GROUP_SET(*p_eg_gport, modid, *p_eg_gport);
	sp_info->egroup[idx].egroup_gport = *p_eg_gport;
	sp_info->egroup[idx].num_fifos = num_fifos;

	if (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) {
	    if (eg_requested != BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
		requested_fcd = eg_requested * 2;
	    }

	    rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_TAKE, requested_fcd, -1, node, level0_node, level0_node, &fcd);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "Could not allocate fcd\n")));
		return rv;
	    }
	    sp_info->egroup[idx].ef_fcd = fcd;
		
	    if (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) {
		if (eg_requested != BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
		    requested_fcd++;
		}
		    
		rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_TAKE, requested_fcd, -1, node, level0_node, level0_node + 1, &fcd);
		if (rv != BCM_E_NONE) {
		    LOG_ERROR(BSL_LS_BCM_COMMON,
		              (BSL_META_U(unit,
		                          "Could not allocate fcd\n")));
		    return rv;
		}
		    
		sp_info->egroup[idx].nef_fcd = fcd;
	    } else {
		sp_info->egroup[idx].nef_fcd = sp_info->egroup[idx].ef_fcd;
	    }
	}
    }

    return BCM_E_NONE;
}


int
bcm_sbx_cosq_egress_group_allocate(int unit, bcm_gport_t eg_requested, int modid, int child_port,
				    int num_fifos, bcm_gport_t *p_eg_gport)
{
    int node = 0, logical_eg = -1, temp_eg = -1, level0_node = -1;
    bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
    bcm_sbx_subport_info_t *sp_info;
    int32 requested_fcd = BCM_INT_SBX_REQUESTED_FCD_AUTO_SELECT;
    int speed = 0, fifo = 0, fcd = BCM_INT_SBX_INVALID_FCD, rv = BCM_E_NONE;
    int idx = 0, enum_fifos = 0;

    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    rv = bcm_sbx_cosq_egress_group_node_allocate(unit, eg_requested, modid, child_port,
							 num_fifos, p_eg_gport);
	}
	return rv;
    }

    if (SOC_SBX_STATE(unit)->port_state->subport_info == 0) {
	return BCM_E_UNAVAIL;
    }
    
    if (num_fifos > SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: Only up to %d fifos per egress group allowed\n"),
	           SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE));
	BCM_SBX_UNLOCK(unit);
	return BCM_E_PARAM;	    
    }
    
    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[child_port]);
    
    if (SOC_IS_SIRIUS(unit)) {
	if (sp_info->valid == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_port %d, unit %d\n"),
	               FUNCTION_NAME(), child_port, unit));
	    return BCM_E_PARAM;
	}
    }
    
    /*
     * find available egroup
     */
    for (idx=0; idx < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; idx++) {
	if (sp_info->egroup[idx].egroup_gport == BCM_GPORT_INVALID)
	    break;
	enum_fifos += sp_info->egroup[idx].num_fifos;
	if ((enum_fifos + num_fifos) > SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, %d fifos exceeds maximum allowable fifos for child_port %d unit %d\n"),
	               FUNCTION_NAME(), num_fifos, child_port, unit));
	    return BCM_E_PARAM;
	}
    }
    
    if (idx == SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: %s, max fifos in use for child_port %d unit %d\n"),
                   FUNCTION_NAME(), child_port, unit));
        return BCM_E_RESOURCE;
    }
    
    p_eg = SOC_SBX_STATE(unit)->egress_group_state;
    
    *p_eg_gport = -1;
    
    if (eg_requested == BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
	
	if (p_eg == NULL) {
	    return BCM_E_UNAVAIL;
	}
	for (logical_eg = 0; logical_eg < SOC_SBX_CFG(unit)->num_egress_group;
	     logical_eg++) {
	    if ( (p_eg + logical_eg)->eg_in_use == FALSE ) {
		*p_eg_gport = logical_eg;
		(p_eg + logical_eg)->eg_in_use = TRUE;
		(p_eg + logical_eg)->child_port = child_port;
		temp_eg = logical_eg;
		break;
	    }
	}
        
	/* 
	 * if we have enough available fifos to use, then use them. Otherwise,
	 * allocate additional fifos.
	 */
	
	if (SOC_IS_SIRIUS(unit)) {
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_allocate, (unit, SIRIUS_ES_LEVEL_FIFO, 
                                      BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT, num_fifos, TRUE, &level0_node));
            
            if (rv != BCM_E_NONE) {
                (p_eg + temp_eg)->eg_in_use = FALSE;
                (p_eg + temp_eg)->child_port = -1;
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, failed to allocate %d level 0 egress scheduler nodes for gport 0x%x, Unit(%d)\n"),
                           FUNCTION_NAME(), num_fifos, sp_info->parent_gport, unit));
                return BCM_E_RESOURCE;
            }	
        }
    } else {
	if (eg_requested >= SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS ) {
	    return BCM_E_PARAM;
	} else {
	    logical_eg = eg_requested;
	}
	
	if (p_eg == NULL) {
	    return BCM_E_UNAVAIL;
	}
	if ( (logical_eg < 0) || (logical_eg >= SOC_SBX_CFG(unit)->num_egress_group) ) {
	    return BCM_E_PARAM;
	}
	if ( (p_eg + logical_eg)->eg_in_use == FALSE ) {
	    *p_eg_gport = logical_eg;
	    (p_eg + logical_eg)->eg_in_use = TRUE;
	    (p_eg + logical_eg)->child_port = child_port;
	    (p_eg + logical_eg)->eg_scheduler = -1;
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress group %d in use, Unit(%d)\n"),
	               FUNCTION_NAME(), logical_eg, unit));
	    return BCM_E_RESOURCE;
	}
        
	/* 
	 * if we have enough available fifos to use, then use them. Otherwise,
	 * allocate additional fifos.
	 */
        
	if (SOC_IS_SIRIUS(unit)) {
            rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_allocate, (unit, SIRIUS_ES_LEVEL_FIFO, 
                                      logical_eg * SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE, num_fifos, TRUE, &level0_node));
            if (rv != BCM_E_NONE) {
                (p_eg + logical_eg)->eg_in_use = FALSE;
                (p_eg + logical_eg)->child_port = -1;
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, failed to allocate %d level 0 egress scheduler nodes for gport 0x%x, Unit(%d)\n"),
                           FUNCTION_NAME(), num_fifos, sp_info->parent_gport, unit));
                return BCM_E_RESOURCE;
            }
        }
    }
    
    node = BCM_STK_MOD_TO_NODE(modid);
    /* return failure if not available */
    if (*p_eg_gport < 0) {
        *p_eg_gport = BCM_GPORT_INVALID;
        return BCM_E_RESOURCE;
    } else {
	BCM_GPORT_EGRESS_GROUP_SET(*p_eg_gport, modid, *p_eg_gport);
	sp_info->egroup[idx].egroup_gport = *p_eg_gport;
        
        sp_info->egroup[idx].es_scheduler_level0_node = level0_node;
        sp_info->egroup[idx].num_fifos = num_fifos;
        
        if (SOC_SBX_CFG(unit)->bUcqResourceAllocationMode == FALSE) {
            if (eg_requested != BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
                requested_fcd = eg_requested * 2;
            }
            
            if (sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) {
                requested_fcd = SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS;
            }
            
            rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_TAKE, requested_fcd, -1, node, level0_node, level0_node, &fcd);
            if (rv != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Could not allocate fcd\n")));
                return rv;
            }
            sp_info->egroup[idx].ef_fcd = fcd;
            
            if ((SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl) &&
                !(SOC_IS_SIRIUS(unit) && SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode)) {
                /* if sirius is in extended port mode, even with independed flow control we only assign 1 flow control domain */ 
                if (eg_requested != BCM_INT_SBX_REQUESTED_SCHEDULER_AUTO_SELECT) {
                    requested_fcd++;
                }
		
                rv = _bcm_sbx_cosq_fcd_allocate(unit, FCD_TAKE, requested_fcd, -1, node, level0_node, level0_node + 1, &fcd);
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "Could not allocate fcd\n")));
                    return rv;
                }
		
                sp_info->egroup[idx].nef_fcd = fcd;
            } else {
                sp_info->egroup[idx].nef_fcd = sp_info->egroup[idx].ef_fcd;
            }
        }

	if (SOC_IS_SIRIUS(unit)) {
	    /*
	     * Allocate port speed
	     */

            if ((sp_info->flags & SBX_SUBPORT_FLAG_TRUNK_MCAST) == 0) {
                if (sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] != 0) 
                    speed = sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE];
                else if (SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] != 0)
                    speed = SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE];
                else
                    speed = 1000;
                
                /* The overall port speed */
                sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = speed;
                
                for(fifo = 0; fifo < sp_info->egroup[idx].num_fifos; fifo++) {
                    if (SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[fifo] != 0) {
                        sp_info->egroup[idx].port_speed[fifo] = SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[fifo];
                    } else {
                        sp_info->egroup[idx].port_speed[fifo] = speed;
                    }
                }
            } else {
                sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 10000;
                for (fifo=0; fifo < SBX_FAB_MAX_MC_FIFOS; fifo++) {
                    if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
                        if ((SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo == fifo) ||
                            (SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo == fifo)) {
                            sp_info->egroup[idx].port_speed[fifo] = 
                                sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE];
                        } 
                    } else {
                        if ((SOC_SBX_CFG(unit)->is_mc_ef_cos[fifo] == TRUE) ||
                            (SOC_SBX_CFG(unit)->is_mc_ef_cos[fifo] == FALSE)) {
                            sp_info->egroup[idx].port_speed[fifo] = 
                                sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE];
                        } 
                    }
                }
            }

            /* Reinitialize to retrieve next soc property */
            for (fifo = 0; fifo <= SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; fifo++) {
                SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[fifo] = 0;
            }
	}
    }
    return rv;
}


int
bcm_sbx_cosq_egress_group_info_get(int unit, bcm_gport_t egress_group, int *fabric_port, 
				  int *egroup_num, int *num_fifos)
{
    int rv = BCM_E_NONE;
    bcm_sbx_subport_info_t *sp_info;
    bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
    int fport = 0, idx = 0, logical_eg = 0, max = 0, offset = 0, modid = 0;
    int node = 0;

    if (SOC_IS_SBX_QE(unit)) {
	/* Special case for DV testing, just return egress group num without 
	 * translation
	 */
        if (SHR_BITGET(SOC_SBX_CFG_SIRIUS(unit)->property, DIAG_EMULATOR_PARTIAL_INIT) || SOC_IS_SBX_QE2000(unit)) {
	    if (BCM_GPORT_IS_EGRESS_CHILD(egress_group)) {
		*fabric_port = BCM_GPORT_EGRESS_CHILD_PORT_GET(egress_group);
	    } else if (BCM_GPORT_IS_CHILD(egress_group)) {
		*fabric_port = BCM_GPORT_CHILD_PORT_GET(egress_group);
	    } else if (BCM_GPORT_IS_EGRESS_MODPORT(egress_group)) {
		*fabric_port = BCM_GPORT_EGRESS_MODPORT_PORT_GET(egress_group);
	    } else {
		*fabric_port = BCM_GPORT_EGRESS_GROUP_GET(egress_group);
	    }
	    *egroup_num = 0;
	    return BCM_E_NONE;
	}
    }

    if (SOC_SBX_STATE(unit)->port_state->subport_info == NULL) {
	return BCM_E_UNAVAIL;
    }

    if (SOC_SBX_STATE(unit)->egress_group_state == NULL) {
	return BCM_E_UNAVAIL;
    }

    if (fabric_port == NULL) {
	return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_CHILD(egress_group)) {
	fport = BCM_GPORT_CHILD_PORT_GET(egress_group);
	modid = BCM_GPORT_CHILD_MODID_GET(egress_group);
	max = 1;
    } else if (BCM_GPORT_IS_EGRESS_CHILD(egress_group)) {
	fport = BCM_GPORT_EGRESS_CHILD_PORT_GET(egress_group);
	modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(egress_group);
	max = 1;
    } else if (BCM_GPORT_IS_EGRESS_GROUP(egress_group)) {
	fport = BCM_GPORT_EGRESS_GROUP_GET(egress_group);
	modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(egress_group);
	max = SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX;
    } else {
	return BCM_E_PARAM;
    }
    
    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    if ( BCM_STK_MOD_TO_NODE(modid) >= SOC_SBX_CFG(unit)->cfg_num_nodes) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Node (%d) exceeds num_nodes (%d),  Unit(%d)\n"),
		           FUNCTION_NAME(), unit, node, SOC_SBX_CFG(unit)->cfg_num_nodes));
		return BCM_E_PARAM;
	    }
	    node = BCM_STK_MOD_TO_NODE(modid);
	    offset = BCM_STK_MOD_TO_NODE(modid) * SB_FAB_DEVICE_MAX_FABRIC_PORTS;
	}
    }

    if (*fabric_port == -1) {
	if (BCM_GPORT_IS_CHILD(egress_group) ||
	    BCM_GPORT_IS_EGRESS_CHILD(egress_group)) {
	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[offset + fport]);
	    *fabric_port = fport;
	    if (egroup_num != NULL) {
		*egroup_num = 0;
	    }
	    if (num_fifos != NULL) {
		*num_fifos = sp_info->egroup[0].num_fifos;
	    }
	    return rv;
	}

	p_eg = &(egress_group_state[unit][node * SOC_SBX_CFG(unit)->num_egress_group]);
	logical_eg = fport;
	if ((p_eg == NULL) || (logical_eg < 0) || (logical_eg >= SOC_SBX_CFG(unit)->num_egress_group)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress group gport 0x%x not valid, unit %d\n"),
	               FUNCTION_NAME(), egress_group, unit));		
	    return BCM_E_PARAM;
	}

	if ((p_eg + logical_eg)->eg_in_use == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "gport Egress Group not in use\n")));
	    return BCM_E_PARAM;
	}
	fport = (p_eg + logical_eg)->child_port;
	if ((fport < 0) || (fport >=  SB_FAB_DEVICE_MAX_FABRIC_PORTS)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, egress_group 0x%x does not associated with any fabric port, unit %d\n"),
	               FUNCTION_NAME(), egress_group, unit));	    
	    return BCM_E_PARAM;	    
	}
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[offset + fport]);
	if ((sp_info != NULL) && sp_info->valid) {
	    for (idx = 0; idx <  SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; idx++) {
		if (sp_info->egroup[idx].egroup_gport == egress_group) {
		    if (egroup_num != NULL) {
			*egroup_num = idx;
		    }
		    *fabric_port = fport;
		    if (num_fifos != NULL) {
			*num_fifos = sp_info->egroup[idx].num_fifos;
		    }
		    return rv;
		}
	    }
	}
	if (egroup_num != NULL) {
	    *egroup_num = -1;
	}
	if (num_fifos != NULL) {
	    *num_fifos = 0;
	}
	return BCM_E_INTERNAL;
    }

    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[offset + *fabric_port]);
    if (sp_info->valid == FALSE) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "ERROR: %s, invalid fabric_port %d, unit %d\n"),
                 FUNCTION_NAME(), *fabric_port, unit));
      return BCM_E_PARAM;
    }

    for (idx = 0; idx < max; idx++) {
	if (BCM_GPORT_EGRESS_GROUP_GET(sp_info->egroup[idx].egroup_gport) == fport) {
	    if (egroup_num != NULL) {
		*egroup_num = idx;
	    }
	    if (num_fifos != NULL) {
	      *num_fifos = sp_info->egroup[idx].num_fifos;
	    }
	    return rv;
	}
    }
    if (egroup_num != NULL) {
	*egroup_num = -1;
    }
    if (num_fifos != NULL) {
	*num_fifos = 0;
    }
    return BCM_E_PARAM;
}


int
bcm_sbx_cosq_egress_group_free(int unit, bcm_gport_t eg_gport)
{
    int logical_eg = 0;
    bcm_sbx_cosq_egress_group_state_t *p_eg;
    bcm_sbx_subport_info_t *sp_info;
    int child_port = 0, offset = 0;
    int fifo = 0, rv = BCM_E_NONE;
    int node = 0, idx = 0;

    if (SOC_SBX_STATE(unit)->port_state->subport_info == 0) {
	return BCM_E_UNAVAIL;
    }

    p_eg = SOC_SBX_STATE(unit)->egress_group_state;

    if (eg_gport == 0) {
	return BCM_E_PARAM;
    }

    if (p_eg == NULL) {
	return BCM_E_UNAVAIL;
    }

    logical_eg = BCM_GPORT_EGRESS_GROUP_GET(eg_gport);

    if ( (logical_eg < 0) || (logical_eg >= SOC_SBX_CFG(unit)->num_egress_group) ) {
	return BCM_E_PARAM;
    }

    node = BCM_STK_MOD_TO_NODE(BCM_GPORT_EGRESS_GROUP_MODID_GET(eg_gport));
    if (soc_feature(unit, soc_feature_arbiter)) {
        if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) ||
            (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
	    offset = node * SB_FAB_DEVICE_MAX_FABRIC_PORTS;
            (p_eg += offset);
	} else {
            return rv;
        }
    }
    
    if ( (p_eg + logical_eg)->eg_in_use == TRUE ) {
	(p_eg + logical_eg)->eg_in_use = FALSE;
	child_port = (p_eg + logical_eg)->child_port;
	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[offset + child_port]);
	if (sp_info->valid == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_port %d, unit %d\n"),
	               FUNCTION_NAME(), child_port, unit));
	    return BCM_E_INTERNAL;
	}
	for (idx = 0; idx <  SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; idx++) {
	    if (BCM_GPORT_EGRESS_GROUP_GET(sp_info->egroup[idx].egroup_gport) == logical_eg) {
		break;
	    }
	}
	if (idx == SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_port %d, unit %d\n"),
	               FUNCTION_NAME(),  child_port, unit));
	    return BCM_E_INTERNAL;
	}

	/* dispose of fifo level */
        if (SOC_IS_SIRIUS(unit)) {
            for (fifo = sp_info->egroup[idx].es_scheduler_level0_node; 
                 fifo < sp_info->egroup[idx].es_scheduler_level0_node + sp_info->egroup[idx].num_fifos;
                 fifo++) {
                rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_scheduler_free, (unit, SIRIUS_ES_LEVEL_FIFO, fifo, TRUE));
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "ERROR: %s, failed to free level 0 egress scheduler"
                                           " node %d for gport 0x%x, Unit(%d)\n"),
                               FUNCTION_NAME(),
                               fifo,
                               eg_gport,
                               unit));
                    return rv;
                }
            }
            
        }

	rv = _bcm_sbx_cosq_fcd_free(unit, sp_info->egroup[idx].ef_fcd, node);
	if (BCM_E_NONE != rv) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, failed to free fcd %d for gport 0x%x, Unit(%d)\n"),
	               FUNCTION_NAME(),
	               sp_info->egroup[idx].ef_fcd,
	               eg_gport,
	               unit));
	    return rv;
	}
	
        if ((SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode == 0) &&
            (SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl == 1)) {
            rv = _bcm_sbx_cosq_fcd_free(unit, sp_info->egroup[idx].nef_fcd, node);
            if (BCM_E_NONE != rv) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "ERROR: %s, failed to free fcd %d for gport 0x%x, Unit(%d)\n"),
                           FUNCTION_NAME(),
                           sp_info->egroup[idx].nef_fcd,
                           eg_gport,
                           unit));
                return rv;
            }                
        }
        
	sp_info->egroup[idx].egroup_gport = -1;
	sp_info->egroup[idx].num_fifos = 0;
	sp_info->egroup[idx].es_scheduler_level0_node = -1;
	sp_info->egroup[idx].ef_fcd = BCM_INT_SBX_INVALID_FCD;
	sp_info->egroup[idx].nef_fcd = BCM_INT_SBX_INVALID_FCD;
	for (fifo = 0; fifo < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE; fifo++) {
	  sp_info->egroup[idx].fcd[fifo] = BCM_INT_SBX_INVALID_FCD;
          sp_info->egroup[idx].port_speed[fifo] = 0;
	}
        sp_info->egroup[idx].port_speed[SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 0;
	(p_eg + logical_eg)->child_port = -1;

    } else {
	return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

void
bcm_sbx_cosq_multicast_queue_group_get(int unit, bcm_fabric_distribution_t ds_id, int32 *p_base_queue)
{
    int32 base_queue;

    base_queue = ds_id_base_queue[unit][ds_id];

    if (base_queue == -1) {
        LOG_CLI((BSL_META_U(unit,
                            "Queue not allocated for this distribution_group(%d)\n"), ds_id));
    }
    else {
      LOG_CLI((BSL_META_U(unit,
                          "ds_id(%d) associated with base queue(%d)\n"), ds_id, base_queue));
    }
    *p_base_queue = base_queue;
}

int
bcm_sbx_cosq_multicast_ds_id_from_queue_group(int unit, int32 base_queue, bcm_fabric_distribution_t *ds_id)
{
    bcm_fabric_distribution_t dsid;
    int result = BCM_E_NOT_FOUND;

    
    for (dsid = 0; dsid < SOC_SBX_CFG(unit)->num_ds_ids; dsid++) {
        if (base_queue == ds_id_base_queue[unit][dsid]) {
            /* found it */
            result = BCM_E_NONE;
            *ds_id = dsid;
            break;
        }
    }
    return result;
}

int
bcm_sbx_gport_fifo_get(int unit, bcm_gport_t gport, int *p_fifo, bcm_module_t *module)
{
    int rv = BCM_E_NONE;
    int logical_eg = 0;
    bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;

    
    p_eg = SOC_SBX_STATE(unit)->egress_group_state;

    if (BCM_GPORT_IS_CHILD(gport)) {
        *p_fifo = BCM_GPORT_CHILD_PORT_GET(gport);
        *module = BCM_GPORT_CHILD_MODID_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
        *p_fifo = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
        *module = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	logical_eg = BCM_GPORT_EGRESS_GROUP_GET(gport);
	if ( (logical_eg < 0) || (logical_eg >= SOC_SBX_CFG(unit)->num_egress_group) ) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "invalid egress group\n")));
	    return BCM_E_PARAM;
	}
	if ((p_eg + logical_eg)->eg_in_use == FALSE) {
            return BCM_E_NOT_FOUND;
	}
        *p_fifo = (p_eg + logical_eg)->child_port;
        *module = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
    } else {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "gport type unsupported\n")));
        rv = BCM_E_PARAM;
    }
    return rv;
}

/* This function is required when setting the FF FIFO sizes for different higig speed ports */

/* port is a higig port.  These ports are continguous.  Hg0 ports followed directly by Hg1  */
/* For example, assume there are 10 Hg0 ports and 10 Hg1, numbering would be from 0-19      */
int
bcm_sbx_cosq_speed_get(int unit, int port, int32 *port_speed_in_mbps)
{
    int rv = BCM_E_NONE;

    if (!SOC_IS_SBX_SIRIUS(unit)) {
        return BCM_E_PARAM;
    }
    *port_speed_in_mbps = 1000;

    return rv;
}

/* This function is required when configuring the ES node hierarchy and FIFO allocation     */

/* Interface is a higig interface number from 0 to 3                                        */
int
bcm_sbx_cosq_num_ports_get(int unit, int interface, int32 *p_num_ports)
{
    int rv = BCM_E_NONE;

    if (!SOC_IS_SBX_SIRIUS(unit)) {
        return BCM_E_PARAM;
    }
    *p_num_ports = 12;

    return rv;
}

int
bcm_sbx_cosq_detach(int unit)
{

    int32 num_cos = 0;
    int32 queue, base_queue;
    int rv;
    int bw_group;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_queue_region_type_t queue_region;

    BCM_SBX_LOCK(unit);

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    if (!(SOC_CONTROL(unit)->soc_flags & SOC_F_RE_INITED)) {
    for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

        p_bwstate = &bw_group_state[unit][bw_group];

        if (p_bwstate->in_use) {

            num_cos = p_bwstate->num_cos;
            base_queue = p_bwstate->base_queue;

            _bcm_sbx_cosq_queue_type_get(unit, base_queue, &queue_region);

            for (queue = base_queue; queue<base_queue+num_cos; queue++) {

              rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_delete_queue, (unit, queue, queue_region));
                if (rv != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "delete queue(%d) failed\n"),
                               queue));
                    BCM_SBX_UNLOCK(unit);
                    return BCM_E_INTERNAL;
                }
            }

            _bcm_sbx_cosq_queue_group_free(unit, base_queue, num_cos);
        }
    }
    }



    /* some chip types don't need their own local detach */
    rv = MBCM_SBX_DRIVER_MAYBE_CALL(unit,
                                    mbcm_cosq_detach,
                                    (unit));
    if (BCM_E_NONE != rv) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "error %d (%s) detaching unit %d cosq\n"),
                   rv,
                   _SHR_ERRMSG(rv),
                   unit));
        BCM_SBX_UNLOCK(unit);
        return rv;
    }


    _bcm_sbx_cosq_scheduler_state_detach(unit);
    _bcm_sbx_cosq_discard_state_detach(unit);
    _bcm_sbx_cosq_destport_state_detach(unit);
    _bcm_sbx_cosq_sysport_state_detach(unit);
    _bcm_sbx_cosq_fcd_state_detach(unit);
    _bcm_sbx_cosq_bw_group_state_detach(unit);
    _bcm_sbx_cosq_queue_state_detach(unit);
    
    BCM_SBX_UNLOCK(unit);
    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/* This function initializes the WB support for cosq module. 
 *      1. During cold-boot: allocates a stable cache
 *      2. During warm-boot: Recovers the cosq state from stable cache
 * 
 * bcm_sbx_cosq_state_t Compressed format 
 */

static int
bcm_sbx_wb_cosq_state_init(int unit)
{
    int                  rv = BCM_E_NONE;
    int                  i=0, size, tmp_len, tmp_data = 0;
    uint8                *scache_ptr = NULL, *ptr = NULL, *end_ptr = NULL;
    soc_scache_handle_t  scache_handle;
    uint32               scache_len = 0;
    int                  stable_size;
    uint16               default_ver = BCM_WB_DEFAULT_VERSION;
    uint16               recovered_ver = BCM_WB_DEFAULT_VERSION;

    if ( SOC_IS_SBX_BME3200(unit) &&
         (SOC_SBX_CFG_BM3200(unit)->uDeviceMode == SOC_SBX_BME_LCM_MODE)) {
        return BCM_E_NONE;
    }

    if (SOC_SBX_STATE(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid cosq state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_COSQ, 0);

    if (SOC_WARM_BOOT(unit)) {
        /* Recover the state from scache */
        rv = soc_versioned_scache_ptr_get(unit, scache_handle,
                                          FALSE, &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                       FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
            return rv;
        }
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len;

    __WB_DECOMPRESS_SCALAR(int, sbx_num_cosq[unit]);
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->max_ports);		     
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule);    
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uFabricConfig);		     
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->use_extended_esets);	     
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->arbitration_port_allocation);
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->cfg_node_00_31_mask);	     
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->cfg_node_32_63_mask);	     
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->cfg_node_64_95_mask);	     
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_ds_ids);

    /* Decompress Queue State */
    __WB_DECOMPRESS_SCALAR(uint32, tmp_data);
    if (SOC_WARM_BOOT(unit) && (SOC_SBX_CFG(unit)->num_queues != tmp_data)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: failed to restore queue state. %d queues in scache, expecting %d queues\n"),
                   FUNCTION_NAME(), tmp_data, SOC_SBX_CFG(unit)->num_queues));
        return BCM_E_INTERNAL;
    }

    /* queue state for sirius moved to sirius warmboot code for optimization */ 
    if (!SOC_IS_SIRIUS(unit)) {	    
        for (i=0; i < SOC_SBX_CFG(unit)->num_queues; i++) {
            __WB_DECOMPRESS_SCALAR(uint8, queue_state[unit][i].state);
            if (SOC_WARM_BOOT(unit) && (queue_state[unit][i].state == BCM_INT_SBX_QUEUE_STATE_AVAILABLE)) {
                /* all queue_state entries should have been initialized */
                continue;
            }
            __WB_DECOMPRESS_SCALAR(int16, queue_state[unit][i].bw_group);
            __WB_DECOMPRESS_SCALAR(int16, queue_state[unit][i].sysport);
            __WB_DECOMPRESS_SCALAR(int16, queue_state[unit][i].default_sysport);
		
            
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.bw_mode);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.bw_value.guarantee_kbps);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.bw_value.sp_priority);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.bw_value.wfq_weight);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.given_weight);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.min_physical_queue_depth_bytes);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.max_physical_queue_depth_bytes);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.anemic_watermark_select);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.ingress_spi4);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.ingress_spi4_port);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.qla_demand_mask);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.queue_header_adjust_sign);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.queue_header_adjust);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.anemic_age_threshold_key);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.shape_limit_kbps);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.enable_shaping);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.shape_burst_kbps);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.local);
            __WB_DECOMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.enabled);
            __WB_DECOMPRESS_SCALAR(int16, queue_state[unit][i].ingress.hold_pri_num_timeslots);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.template);
            __WB_DECOMPRESS_SCALAR(int32, queue_state[unit][i].ingress.gain);
            __WB_DECOMPRESS_SCALAR(uint8, queue_state[unit][i].enq_stat_in_use);
            __WB_DECOMPRESS_SCALAR(uint8, queue_state[unit][i].deq_stat_in_use);
            __WB_DECOMPRESS_SCALAR(uint8, queue_state[unit][i].attached_fifo);
        }
    }

    /* queue region */
    for (i = 0; i < bcm_sbx_cosq_queue_region_last; i++) {
        __WB_DECOMPRESS_SCALAR(uint32, queue_regions[unit][i].is_valid);
        __WB_DECOMPRESS_SCALAR(uint32, queue_regions[unit][i].start);
        __WB_DECOMPRESS_SCALAR(uint32, queue_regions[unit][i].end);
    }
	
    /* Compress bandwidth group State */
    __WB_DECOMPRESS_SCALAR(uint32, tmp_data);
    if (SOC_WARM_BOOT(unit) & (SOC_SBX_CFG(unit)->num_bw_groups != tmp_data)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: failed to restore bw groups. %d bw groups in scache, expecting %d bw groups\n"),
                   FUNCTION_NAME(), tmp_data, SOC_SBX_CFG(unit)->num_bw_groups));
        return BCM_E_INTERNAL;
    }

    for (i=0; i < SOC_SBX_CFG(unit)->num_bw_groups; i++) {
        __WB_DECOMPRESS_SCALAR(int8,  bw_group_state[unit][i].in_use);
        if (SOC_WARM_BOOT(unit) && (bw_group_state[unit][i].in_use == FALSE)) {
            /* all bw group entries should have been initialized */
            continue;
        }
        __WB_DECOMPRESS_SCALAR(bcm_gport_t, bw_group_state[unit][i].gport);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].base_queue);
        __WB_DECOMPRESS_SCALAR(int8,  bw_group_state[unit][i].num_cos);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].dest_node);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].dest_port);
        __WB_DECOMPRESS_SCALAR(int8,  bw_group_state[unit][i].dest_mc);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].dest_type);
        __WB_DECOMPRESS_SCALAR(int32, bw_group_state[unit][i].dest_port_kbits_sec_max);
        __WB_DECOMPRESS_SCALAR(int32, bw_group_state[unit][i].path.bag_rate_kbps);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].flags);
        __WB_DECOMPRESS_SCALAR(int8,  bw_group_state[unit][i].overlay_in_use);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].overlay_dest_node);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].overlay_dest_port);
        __WB_DECOMPRESS_SCALAR(int16, bw_group_state[unit][i].overlay_base_queue);
        __WB_DECOMPRESS_SCALAR(int8,  bw_group_state[unit][i].overlay_num_cos);
        __WB_DECOMPRESS_SCALAR(uint8, bw_group_state[unit][i].cosq_init_added);
        __WB_DECOMPRESS_SCALAR(uint8, bw_group_state[unit][i].cos_map);
    }
	
    /* ds ids */
    if (ds_id_base_queue[unit] != NULL) {
        for (i=0; i < SOC_SBX_CFG(unit)->num_ds_ids; i++) {
            __WB_DECOMPRESS_SCALAR(int, ds_id_base_queue[unit][i]);
        }
    }

    /* sysport state */
    __WB_DECOMPRESS_SCALAR(uint32, tmp_data);

    if (SOC_WARM_BOOT(unit) && 
        ((SOC_SBX_CFG(unit)->num_sysports != tmp_data) ||
         ((SOC_SBX_CFG(unit)->num_sysports != 0) &&
          (sysport_state[unit] == NULL)))) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: failed to restore sysports. %d sysports in scache, expecting %d sysports\n"),
                   FUNCTION_NAME(), tmp_data, SOC_SBX_CFG(unit)->num_sysports));
        return BCM_E_INTERNAL;
    }

    for (i=0; i < SOC_SBX_CFG(unit)->num_sysports; i++) {
        __WB_DECOMPRESS_SCALAR(int8, sysport_state[unit][i].flags);
        if (SOC_WARM_BOOT(unit) && ((sysport_state[unit][i].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == 0)) {
            continue;
        }
        __WB_DECOMPRESS_SCALAR(int16, sysport_state[unit][i].state);
        __WB_DECOMPRESS_SCALAR(int16, sysport_state[unit][i].fcd);
        __WB_DECOMPRESS_SCALAR(int8,  sysport_state[unit][i].fifo);
        __WB_DECOMPRESS_SCALAR(int8,  sysport_state[unit][i].use_cnt);
        __WB_DECOMPRESS_SCALAR(int32, sysport_state[unit][i].node);
        __WB_DECOMPRESS_SCALAR(int32, sysport_state[unit][i].port);
        __WB_DECOMPRESS_SCALAR(uint32,sysport_state[unit][i].egport);
    }
	
    /* sysport_group_state */
    if (sysport_group_state[unit] != NULL) {
        size = (sizeof(bcm_sbx_cosq_sysport_group_state_t) * BCM_INT_SBX_MAX_SYSPORT_GROUP);
        if (SOC_WARM_BOOT(unit)) {
            sal_memcpy(sysport_group_state[unit], (((uint8 *)ptr)), size);
            ptr += size;
        } else {
            scache_len += size;
        }
    }
	
    if ( soc_feature(unit, soc_feature_hybrid) ) {
        /* Restore device_cosq structures */
        _bcm_sbx_wb_device_cosq_state_sync(unit, &scache_len, &ptr, &end_ptr, (SOC_WARM_BOOT(unit) ? _WB_OP_DECOMPRESS : _WB_OP_SIZE));
    }
	
    /* fcd state */
    __WB_DECOMPRESS_SCALAR(uint32, tmp_data);
    if (SOC_WARM_BOOT(unit) && ((SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3) != tmp_data)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: failed to restore fcd. %d templates in scache, expecting %d templates\n"),
                   FUNCTION_NAME(), tmp_data, (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3)));
        return BCM_E_INTERNAL;
    }

    if (fcd_state[unit] != NULL) {
        for (i=0; i < (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3); i++) {
            __WB_DECOMPRESS_SCALAR(int8, fcd_state[unit][i].in_use);
            if (SOC_WARM_BOOT(unit) && (fcd_state[unit][i].in_use == FCD_FREE)) {
                continue;
            }
            __WB_DECOMPRESS_SCALAR(int8,  fcd_state[unit][i].ref_cnt);
            __WB_DECOMPRESS_SCALAR(uint16,fcd_state[unit][i].fifo);
            __WB_DECOMPRESS_SCALAR(uint32,fcd_state[unit][i].sysport);
            __WB_DECOMPRESS_SCALAR(int32, fcd_state[unit][i].node);
            __WB_DECOMPRESS_SCALAR(int32, fcd_state[unit][i].port);
        }
    }
	
    /* Discard state */
    __WB_DECOMPRESS_SCALAR(uint32, tmp_data);
    if (SOC_WARM_BOOT(unit) && 
        ((SOC_SBX_CFG(unit)->nDiscardTemplates != tmp_data) ||
         ((SOC_SBX_CFG(unit)->nDiscardTemplates != 0) &&
          (discard_state[unit] == NULL)))) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: failed to restore discard templates. %d templates in scache, expecting %d templates\n"),
                   FUNCTION_NAME(), tmp_data, SOC_SBX_CFG(unit)->nDiscardTemplates));
        return BCM_E_INTERNAL;
    }

    if (SOC_SBX_CFG(unit)->nDiscardTemplates == 0) 
        tmp_len = 1;
    else
        tmp_len = SOC_SBX_CFG(unit)->nDiscardTemplates;
    if (discard_state[unit] != NULL) {
        for (i=0; i < tmp_len; i++) {
            __WB_DECOMPRESS_SCALAR(int8, discard_state[unit][i].is_free);
            if (SOC_WARM_BOOT(unit) && (discard_state[unit][i].is_free == TRUE)) {
                continue;
            } 
            size = sizeof(bcm_sbx_cosq_discard_state_t);
            /* copy over whole structure */
            if (SOC_WARM_BOOT(unit)) {
                ptr -= (sizeof(int8)); /* roll back pointer */
                sal_memcpy(&discard_state[unit][i], (((uint8 *)ptr)), size);
                ptr += size;
            } else {
                scache_len += (size - sizeof(uint8));
            }
        }
    }

    /* egress group */
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_egress_group);
    if (egress_group_state[unit] != NULL) {
        for (i=0; i < SOC_SBX_CFG(unit)->num_egress_group; i++) {
            __WB_DECOMPRESS_SCALAR(int, egress_group_state[unit][i].eg_in_use);
            if (!SOC_WARM_BOOT(unit) || (egress_group_state[unit][i].eg_in_use == TRUE)) {
                __WB_DECOMPRESS_SCALAR(sbBool_t, egress_group_state[unit][i].in_use);
                __WB_DECOMPRESS_SCALAR(int, egress_group_state[unit][i].child_port);
                __WB_DECOMPRESS_SCALAR(int, egress_group_state[unit][i].eg_scheduler);
            }
        }
    }

    /* chip specific data structures */
    if (SOC_IS_SBX_QE2000(unit)) {
        /* destport state */
        if (dp_state[unit] != NULL) {
            if (SOC_WARM_BOOT(unit)) {
                sal_memcpy(dp_state[unit], (((uint8 *)ptr)), (sizeof(bcm_sbx_cosq_destport_state_t) * SBX_MAX_PORTS));
                ptr += (sizeof(bcm_sbx_cosq_destport_state_t) * SBX_MAX_PORTS);
            } else {
                scache_len += (sizeof(bcm_sbx_cosq_destport_state_t) * SBX_MAX_PORTS);
            }
        }

        __WB_DECOMPRESS_SCALAR(int, SOC_SBX_CONTROL(unit)->numQueues);

        if (SOC_WARM_BOOT(unit)) {
            rv = _bcm_qe2000_wb_cosq_state_sync(unit, &ptr, TRUE);
			COMPILER_REFERENCE(rv);
        } else {
            scache_len += _bcm_qe2000_wb_cosq_state_sync(unit, NULL, 2);
        }
    } else if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_wb_cosq_state_sync(unit, default_ver, recovered_ver,
                                           &scache_len, &ptr, &end_ptr,  (SOC_WARM_BOOT(unit) ? _WB_OP_DECOMPRESS : _WB_OP_SIZE));
        if (rv != BCM_E_NONE) {
            return rv;
        }
#endif /* BCM_SIRIUS_SUPPORT */
    }
    
    rv = soc_scache_handle_used_set(unit, scache_handle, (ptr - scache_ptr));

    if (!SOC_WARM_BOOT(unit)) {
        rv = soc_versioned_scache_ptr_get(unit, scache_handle,
                                          TRUE, &scache_len, &scache_ptr,
                                          default_ver, &recovered_ver);
        if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "%s: Error(%s) allocating WB scache handle on unit:%d \n"),
                       FUNCTION_NAME(), soc_errmsg(rv), unit));
            return rv;
        }
    }
    
    return rv;
}


/*
 * Function:
 *      bcm_sbx_wb_cosq_state_sync
 * Purpose:
 *      Record COSq module persisitent info for Level 2 Warm Boot
 *      This function compresses the info in bcm_sbx_cosq_state_t and stores it
 *      to stable memory.
 *      Input param: sync --> indicates whether to sync scache to Persistent memory
 *
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_sbx_wb_cosq_state_sync(int unit, int sync)
{
    uint8                   *scache_ptr = NULL;
    uint8                   *ptr, *end_ptr;
    uint32                  scache_len;
    int                     stable_size;
    int                     rv = BCM_E_NONE, i = 0, size, tmp_len;
    soc_scache_handle_t     scache_handle;
    uint16                  default_ver = BCM_WB_DEFAULT_VERSION;
    uint16                  recovered_ver = BCM_WB_DEFAULT_VERSION;

    if (SOC_WARM_BOOT(unit)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Cannot write to SCACHE during WarmBoot\n")));
        return SOC_E_INTERNAL;
    }

    if (SOC_SBX_STATE(unit) == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Internal error. Invalid cosq state (NULL) \n"), 
                   FUNCTION_NAME()));
        return BCM_E_INTERNAL;
    }

    /* check to see if an scache table has been configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(rv) || stable_size <= 0) {
        return rv;
    }

    scache_len = 0;
    SOC_SCACHE_HANDLE_SET(scache_handle, unit, BCM_MODULE_COSQ, 0);

    rv = soc_versioned_scache_ptr_get(unit, scache_handle,
					  FALSE, &scache_len, &scache_ptr,
					  default_ver, &recovered_ver);

    if (SOC_FAILURE(rv) && (rv != SOC_E_NOT_FOUND)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "%s: Error(%s) reading scache. scache_ptr:%p and len:%d\n"),
                   FUNCTION_NAME(), soc_errmsg(rv), scache_ptr, scache_len));
        return rv;
    }

    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len;

    /* now compress and store in the scache location */
    __WB_COMPRESS_SCALAR(int, sbx_num_cosq[unit]);
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->max_ports);		     
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule);    
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uFabricConfig);		     
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->use_extended_esets);	     
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->arbitration_port_allocation);
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->cfg_node_00_31_mask);	     
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->cfg_node_32_63_mask);	     
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->cfg_node_64_95_mask);	     
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_ds_ids);
           
    /* Compress Queue State */
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
    
    /* queue state for sirius moved to sirius warmboot code for optimization */ 
    if (!SOC_IS_SIRIUS(unit)) {	    
	for (i=0; i < SOC_SBX_CFG(unit)->num_queues; i++) {
	    __WB_COMPRESS_SCALAR(uint8, queue_state[unit][i].state);
	    if (queue_state[unit][i].state == BCM_INT_SBX_QUEUE_STATE_AVAILABLE) {
		/* all queue_state entries should have been initialized */
		continue;
	    }
	    __WB_COMPRESS_SCALAR(int16, queue_state[unit][i].bw_group);
	    __WB_COMPRESS_SCALAR(int16, queue_state[unit][i].sysport);
	    __WB_COMPRESS_SCALAR(int16, queue_state[unit][i].default_sysport);
	    
	    
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.bw_mode);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.bw_value.guarantee_kbps);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.bw_value.sp_priority);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.bw_value.wfq_weight);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.given_weight);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.min_physical_queue_depth_bytes);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.max_physical_queue_depth_bytes);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.anemic_watermark_select);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.ingress_spi4);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.ingress_spi4_port);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.qla_demand_mask);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.queue_header_adjust_sign);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.queue_header_adjust);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.anemic_age_threshold_key);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.shape_limit_kbps);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.enable_shaping);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.shape_burst_kbps);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.local);
	    __WB_COMPRESS_SCALAR(int8,  queue_state[unit][i].ingress.enabled);
	    __WB_COMPRESS_SCALAR(int16, queue_state[unit][i].ingress.hold_pri_num_timeslots);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.template);
	    __WB_COMPRESS_SCALAR(int32, queue_state[unit][i].ingress.gain);
	    __WB_COMPRESS_SCALAR(uint8, queue_state[unit][i].enq_stat_in_use);
	    __WB_COMPRESS_SCALAR(uint8, queue_state[unit][i].deq_stat_in_use);
	    __WB_COMPRESS_SCALAR(uint8, queue_state[unit][i].attached_fifo);
	}
    }
    
    /* queue region */
    for (i = 0; i < bcm_sbx_cosq_queue_region_last; i++) {
	__WB_COMPRESS_SCALAR(uint32, queue_regions[unit][i].is_valid);
	__WB_COMPRESS_SCALAR(uint32, queue_regions[unit][i].start);
	__WB_COMPRESS_SCALAR(uint32, queue_regions[unit][i].end);
    }
    
    /* bandwidth group state */
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_bw_groups);
    
    for (i=0; i < SOC_SBX_CFG(unit)->num_bw_groups; i++) {
	__WB_COMPRESS_SCALAR(int8,  bw_group_state[unit][i].in_use);
	if (bw_group_state[unit][i].in_use == FALSE) {
	    /* all bw group entries should have been initialized */
	    continue;
	}
	__WB_COMPRESS_SCALAR(bcm_gport_t, bw_group_state[unit][i].gport);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].base_queue);
	__WB_COMPRESS_SCALAR(int8,  bw_group_state[unit][i].num_cos);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].dest_node);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].dest_port);
	__WB_COMPRESS_SCALAR(int8,  bw_group_state[unit][i].dest_mc);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].dest_type);
	__WB_COMPRESS_SCALAR(int32, bw_group_state[unit][i].dest_port_kbits_sec_max);
	__WB_COMPRESS_SCALAR(int32, bw_group_state[unit][i].path.bag_rate_kbps);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].flags);
	__WB_COMPRESS_SCALAR(int8,  bw_group_state[unit][i].overlay_in_use);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].overlay_dest_node);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].overlay_dest_port);
	__WB_COMPRESS_SCALAR(int16, bw_group_state[unit][i].overlay_base_queue);
	__WB_COMPRESS_SCALAR(int8,  bw_group_state[unit][i].overlay_num_cos);
	__WB_COMPRESS_SCALAR(uint8, bw_group_state[unit][i].cosq_init_added);
	__WB_COMPRESS_SCALAR(uint8, bw_group_state[unit][i].cos_map);
    }

    /* ds ids */
    if (ds_id_base_queue[unit] != NULL) {
        for (i=0; i < SOC_SBX_CFG(unit)->num_ds_ids; i++) {
	    __WB_COMPRESS_SCALAR(int,  ds_id_base_queue[unit][i]);
        }
    }

    /* sysport state */
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_sysports);
    for (i=0; i < SOC_SBX_CFG(unit)->num_sysports; i++) {
	__WB_COMPRESS_SCALAR(int8, sysport_state[unit][i].flags);
	if ((sysport_state[unit][i].flags & BCM_INT_SBX_SYSPORT_STATE_FLAGS_IN_USE) == 0) {
	    continue;
	}
	__WB_COMPRESS_SCALAR(int16, sysport_state[unit][i].state);
	__WB_COMPRESS_SCALAR(int16, sysport_state[unit][i].fcd);
	__WB_COMPRESS_SCALAR(int8,  sysport_state[unit][i].fifo);
	__WB_COMPRESS_SCALAR(int8,  sysport_state[unit][i].use_cnt);
	__WB_COMPRESS_SCALAR(int32, sysport_state[unit][i].node);
	__WB_COMPRESS_SCALAR(int32, sysport_state[unit][i].port);
	__WB_COMPRESS_SCALAR(uint32, sysport_state[unit][i].egport);
    }
    
    /* sysport_group_state */
    if (sysport_group_state[unit] != NULL) {
	size = (sizeof(bcm_sbx_cosq_sysport_group_state_t) * BCM_INT_SBX_MAX_SYSPORT_GROUP);
        sal_memcpy((((uint8 *)ptr)), sysport_group_state[unit], size);
	ptr += size;
    }
    
    if ( soc_feature(unit, soc_feature_hybrid) ) {
        /* Save device_cosq structures */
        _bcm_sbx_wb_device_cosq_state_sync(unit, &scache_len, &ptr, &end_ptr, _WB_OP_COMPRESS);
    }

    /* fcd state */
    __WB_COMPRESS_SCALAR(uint32, (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3));
    if (fcd_state[unit] != NULL) {
	for (i=0; i < (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3); i++) {
	    __WB_COMPRESS_SCALAR(int8, fcd_state[unit][i].in_use);
	    if (fcd_state[unit][i].in_use == FCD_FREE) {
		continue;
	    }
	    __WB_COMPRESS_SCALAR(int8,  fcd_state[unit][i].ref_cnt);
	    __WB_COMPRESS_SCALAR(uint16, fcd_state[unit][i].fifo);
	    __WB_COMPRESS_SCALAR(uint32, fcd_state[unit][i].sysport);
	    __WB_COMPRESS_SCALAR(int32, fcd_state[unit][i].node);
	    __WB_COMPRESS_SCALAR(int32, fcd_state[unit][i].port);
	}
    }

    /* Discard state */
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->nDiscardTemplates);
    if (SOC_SBX_CFG(unit)->nDiscardTemplates == 0) 
        tmp_len = 1;
    else
        tmp_len = SOC_SBX_CFG(unit)->nDiscardTemplates;
    if (discard_state[unit] != NULL) {
	for (i=0; i < tmp_len; i++) {
	    __WB_COMPRESS_SCALAR(int8, discard_state[unit][i].is_free);
	    if (discard_state[unit][i].is_free == TRUE) {
		continue;
	    } else {
		ptr -= (sizeof(int8)); /* roll back pointer */
		/* copy over whole structure */
		sal_memcpy(&discard_state[unit][i], (((uint8 *)ptr)), sizeof(bcm_sbx_cosq_discard_state_t));
                ptr += sizeof(bcm_sbx_cosq_discard_state_t);
	    }
	}
    }

    /* egress group */
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_egress_group);
    if (egress_group_state[unit] != NULL) {
        for (i=0; i < SOC_SBX_CFG(unit)->num_egress_group; i++) {
            __WB_COMPRESS_SCALAR(int, egress_group_state[unit][i].eg_in_use);
            if (egress_group_state[unit][i].eg_in_use == TRUE) {
                __WB_COMPRESS_SCALAR(sbBool_t, egress_group_state[unit][i].in_use);
                __WB_COMPRESS_SCALAR(int, egress_group_state[unit][i].child_port);
                __WB_COMPRESS_SCALAR(int, egress_group_state[unit][i].eg_scheduler);
            }
        }
    }

    if (SOC_IS_SBX_QE2000(unit)) {
        /* destport state */
        if (dp_state[unit] != NULL) {
            size = (sizeof(bcm_sbx_cosq_destport_state_t) * SBX_MAX_PORTS);
            sal_memcpy((((uint8 *)ptr)), dp_state[unit], size);
            ptr += size;
        }

        __WB_COMPRESS_SCALAR(int, SOC_SBX_CONTROL(unit)->numQueues);

        rv = _bcm_qe2000_wb_cosq_state_sync(unit, &ptr, FALSE);
		COMPILER_REFERENCE(rv);
    } else if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_wb_cosq_state_sync(unit, default_ver, recovered_ver,
					   NULL, &ptr, &end_ptr, _WB_OP_COMPRESS);
		COMPILER_REFERENCE(rv);
#endif /* BCM_SIRIUS_SUPPORT */
    }

    if (sync) {
        rv = soc_scache_commit(unit);
        if (rv != SOC_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COMMON,
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

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_cosq_sw_dump(int unit)
{
    uint16               default_ver;
    uint16               recovered_ver;
#ifdef BCM_SIRIUS_SUPPORT
    int rv = BCM_E_NONE;
#endif
    int i,j;
    uint32 *ptr = NULL;

    default_ver = BCM_WB_DEFAULT_VERSION;
    recovered_ver = BCM_WB_DEFAULT_VERSION;
    
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "sbx_num_cosq(%d)\n"),
              sbx_num_cosq[unit]));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "max_ports(%d)\n"),
              SOC_SBX_CFG(unit)->max_ports));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "nMaxFabricPortsOnModule(%d)\n"),
              SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule));    
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "uFabricConfig(%d)\n"),
              SOC_SBX_CFG(unit)->uFabricConfig));		
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "extended(%d)\n"),
              SOC_SBX_CFG(unit)->use_extended_esets));	     	
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "arbitration_port_allocation(%d)\n"),
              SOC_SBX_CFG(unit)->arbitration_port_allocation));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "cfg_node_00_31_mask(%d)\n"),
              SOC_SBX_CFG(unit)->cfg_node_00_31_mask));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "cfg_node_32_63_mask(%d)\n"),
              SOC_SBX_CFG(unit)->cfg_node_32_63_mask));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "cfg_node_64_95_mask(%d)\n"),
              SOC_SBX_CFG(unit)->cfg_node_64_95_mask));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "num_ds_ids(%d)\n"),
              SOC_SBX_CFG(unit)->num_ds_ids));
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "num_queues(%d)\n"),
              SOC_SBX_CFG(unit)->num_queues));

    if (!SOC_IS_SIRIUS(unit)) {	    
	
    }

    /* queue region */
    for (i = 0; i < bcm_sbx_cosq_queue_region_last; i++) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "queue_regions.is_valid(%d)\n"),
	          queue_regions[unit][i].is_valid));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "queue_regions.start(%d)\n"),
	          queue_regions[unit][i].start));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "queue_regions.end(%d)\n"),
	          queue_regions[unit][i].end));
    }
    
    /* bandwidth group state */
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "num_bw_groups(%d)\n"),
              SOC_SBX_CFG(unit)->num_bw_groups));			     
    for (i=0; i<SOC_SBX_CFG(unit)->num_bw_groups; i++) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "bw_group(%d)  \n"),
	          i));	
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].in_use));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].gport));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].base_queue));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          bw_group_state[unit][0].num_cos));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].dest_node));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].dest_port));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].dest_mc));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          bw_group_state[unit][0].dest_type));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].dest_port_kbits_sec_max));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].path.bag_rate_kbps));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].flags));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          bw_group_state[unit][0].overlay_in_use));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].overlay_dest_node));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].overlay_dest_port));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].overlay_base_queue));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          bw_group_state[unit][0].overlay_num_cos));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          bw_group_state[unit][0].cosq_init_added));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          bw_group_state[unit][0].cos_map));
    }

    /* ds ids */
    if (ds_id_base_queue[unit] != NULL) {
	for (i=0; i < SOC_SBX_CFG(unit)->num_ds_ids; i++) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "ds_id_base_queue %d (%d)\n"),
	              i, ds_id_base_queue[unit][i]));
	}
    }
    
    /* sysport state */
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "num_sysports(%d)\n"),
              SOC_SBX_CFG(unit)->num_sysports));
    for (i=0; i<SOC_SBX_CFG(unit)->num_sysports; i++) {
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "sysport(%d)  \n"),
	          i));	
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          sysport_state[unit][i].flags));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          sysport_state[unit][i].state));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          sysport_state[unit][i].fcd));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          sysport_state[unit][i].fifo));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          sysport_state[unit][i].use_cnt));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          sysport_state[unit][i].node));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)  "),
	          sysport_state[unit][i].port));
	LOG_INFO(BSL_LS_BCM_COSQ,
	         (BSL_META_U(unit,
	                     "(%d)\n"),
	          sysport_state[unit][i].egport));    
    }

    /* sysport_group_state */
    if (sysport_group_state[unit] != NULL) {
        for (i=0; i<BCM_INT_SBX_MAX_SYSPORT_GROUP; i++) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "SysportGroup(%d)  \n"),
                      i));
            ptr = (uint32*)&(sysport_group_state[unit][i]);
            for (j=0; j<(sizeof(bcm_sbx_cosq_sysport_group_state_t)+3)/4; j++, ptr++) {
                LOG_INFO(BSL_LS_BCM_COSQ,
                         (BSL_META_U(unit,
                                     "(%d)  "),
                          *ptr));		
                if ((j%4) == 3) {
                    LOG_INFO(BSL_LS_BCM_COSQ,
                             (BSL_META_U(unit,
                                         "\n")));
                }
            }
        }
    }

    if ( soc_feature(unit, soc_feature_hybrid) ) {
	_bcm_sbx_wb_device_cosq_state_sync(unit, NULL, NULL, NULL, _WB_OP_DUMP);
    }

    /* fcd state */
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "num_fcds(%d)\n"),
              (SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3)));
    if (fcd_state[unit] != NULL) {
	for (i=0; i<(SB_FAB_DEVICE_SIRIUS_MAX_FC_DOMAINS + 3); i++) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "fcd(%d)  \n"),
	              i));	
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              fcd_state[unit][i].in_use));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              fcd_state[unit][i].ref_cnt));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              fcd_state[unit][i].fifo));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)\n"),
	              fcd_state[unit][i].sysport));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              fcd_state[unit][i].node));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              fcd_state[unit][i].port));
	}
    }

    /* Discard state */
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "nDiscardTemplates(%d)\n"),
              SOC_SBX_CFG(unit)->nDiscardTemplates));
    if (discard_state[unit] != NULL) {
	for (i=0; i<SOC_SBX_CFG(unit)->nDiscardTemplates; i++) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "DiscardTemplate(%d)  \n"),
	              i));
	    ptr = (uint32*)&(discard_state[unit][i]);
	    for (j=0; j<(sizeof(bcm_sbx_cosq_discard_state_t)+3)/4; j++, ptr++) {
		LOG_INFO(BSL_LS_BCM_COSQ,
		         (BSL_META_U(unit,
		                     "(%d)  "),
		          *ptr));		
		if ((j%4) == 3) {
		    LOG_INFO(BSL_LS_BCM_COSQ,
		             (BSL_META_U(unit,
		                         "\n")));
		}
	    }
	}
        LOG_INFO(BSL_LS_BCM_COSQ,
                 (BSL_META_U(unit,
                             "\n")));        
    }

    /* egress group */
    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "num_egress_group(%d)\n"),
              SOC_SBX_CFG(unit)->num_egress_group));
    if (egress_group_state[unit] != NULL) {
	for (i=0; i<SOC_SBX_CFG(unit)->num_egress_group; i++) {
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "egress_group(%d)  \n"),
	              i));	
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              egress_group_state[unit][i].in_use));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              egress_group_state[unit][i].child_port));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)  "),
	              egress_group_state[unit][i].eg_in_use));
	    LOG_INFO(BSL_LS_BCM_COSQ,
	             (BSL_META_U(unit,
	                         "(%d)\n"),
	              egress_group_state[unit][i].eg_scheduler));
	}
    }

    /* destport state */
    if (SOC_IS_SBX_QE2000(unit)) {
	

    } else if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        rv = bcm_sirius_wb_cosq_state_sync(unit, default_ver, recovered_ver,
					   NULL, NULL, NULL, _WB_OP_DUMP);
        if (rv != BCM_E_NONE) {
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "WB Cosq State sync error(%d)\n"),
                      rv));
        }
#endif /* BCM_SIRIUS_SUPPORT */		
    }

    return;
}
#endif

int
bcm_sbx_cosq_init(int unit)
{
    int32 num_cos;
    int rv;

    BCM_SBX_LOCK(unit);

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);

    if (num_cos < 1) {
        num_cos = 1;
    } else if (NUM_COS(unit) && num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }

    NUM_COS(unit) = sbx_num_cosq[unit] = num_cos;

    if ( soc_property_get(unit, spn_DIAG_ASSIGN_SYSPORT, 0) ) {
	SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, DIAG_ASSIGN_SYSPORT);
    }

    if (SOC_IS_SBX_FE(unit)) {
        BCM_SBX_UNLOCK(unit);
        return BCM_E_NONE;
    }

    if (queue_state[unit] != NULL) {
        /* detach */
        BCM_SBX_UNLOCK(unit);
        rv = bcm_sbx_cosq_detach(unit);
        BCM_IF_ERROR_RETURN(rv);
       
        BCM_SBX_LOCK(unit); 
    }

    /*
     * Initialize internal queue and bandwidth group management structures to
     * indicate all queues and groups are available.
     */
    rv = _bcm_sbx_cosq_queue_state_init(unit, num_cos);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = _bcm_sbx_cosq_bw_group_state_init(unit, num_cos);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = _bcm_sbx_cosq_sysport_state_init(unit);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = _bcm_sbx_cosq_fcd_state_init(unit);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = _bcm_sbx_cosq_destport_state_init(unit);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = _bcm_sbx_cosq_discard_state_init(unit);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    rv = _bcm_sbx_cosq_scheduler_state_init(unit);
    if (rv != BCM_E_NONE) {
        BCM_SBX_UNLOCK(unit);
        return rv;
    }

    SOC_SBX_STATE(unit)->queue_state = queue_state[unit];
    SOC_SBX_STATE(unit)->bw_group_state = bw_group_state[unit];
    SOC_SBX_STATE(unit)->sysport_state = sysport_state[unit];
    SOC_SBX_STATE(unit)->sysport_group_state = sysport_group_state[unit];
    SOC_SBX_STATE(unit)->destport_state = dp_state[unit];
    SOC_SBX_STATE(unit)->ingress_scheduler_state = ingress_scheduler_state[unit];
    SOC_SBX_STATE(unit)->egress_scheduler_state = egress_scheduler_state[unit];
    SOC_SBX_STATE(unit)->egress_group_state = egress_group_state[unit];
    SOC_SBX_STATE(unit)->ingress_multipath_state = ingress_multipath_state[unit];
    SOC_SBX_STATE(unit)->egress_multipath_state = egress_multipath_state[unit];
    SOC_SBX_STATE(unit)->fcd_state = fcd_state[unit];

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_init, (unit));

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(rv)) {
	rv = bcm_sbx_wb_cosq_state_init(unit);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: error in WarmBoot cosq state init \n"), 
	               FUNCTION_NAME()));
	}
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    BCM_SBX_UNLOCK(unit);
    return rv;
}

int 
bcm_sbx_cosq_subscriber_map_add(int unit, 
			    bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_NONE;
    
    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_subscriber_map_add, (unit, map));

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int 
bcm_sbx_cosq_subscriber_map_delete(int unit, 
			       bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_NONE;
    
    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_subscriber_map_delete, (unit, map));

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int 
bcm_sbx_cosq_subscriber_map_delete_all(int unit)
{
    int rv = BCM_E_NONE;
    
    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_subscriber_map_delete_all, (unit));

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int 
bcm_sbx_cosq_subscriber_map_get(int unit, 
			    bcm_cosq_subscriber_map_t *map)
{
    int rv = BCM_E_NONE;
    
    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_subscriber_map_get, (unit, map));

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int 
bcm_sbx_cosq_subscriber_traverse(int unit, 
				 bcm_cosq_subscriber_map_traverse_cb cb, 
				 void *user_data)
{
    int rv = BCM_E_NONE;
    
    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_subscriber_traverse, (unit, cb, user_data));

    BCM_SBX_UNLOCK(unit);

    return rv;
}

int
bcm_sbx_cosq_gport_flow_control_get(int unit, bcm_gport_t port, bcm_cos_t int_pri, uint32 *flow_control_mask)
{
    int rv = BCM_E_NONE;


    if ( !(soc_feature(unit, soc_feature_higig2)) ) {
        return(BCM_E_UNAVAIL);
    }

    if (!flow_control_mask) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: parameter error - required pointer (flow_control_mask) not being passed\n")));
        return(BCM_E_PARAM);
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_flow_control_get, (unit, port, int_pri, flow_control_mask, NULL));

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
bcm_sbx_cosq_gport_flow_control_set(int unit, bcm_gport_t port, bcm_cos_t int_pri, uint32 flow_control_mask)
{
    int rv = BCM_E_NONE;


    if ( !(soc_feature(unit, soc_feature_higig2)) ) {
        return(BCM_E_UNAVAIL);
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_flow_control_set, (unit, port, int_pri, flow_control_mask, NULL));

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
bcm_sbx_cosq_qinfo_get(int unit, int q, int *ds_id, int *fifo,
		       bcm_sbx_cosq_queue_state_t *p_qstate, 
		       bcm_sbx_cosq_bw_group_state_t *p_bwstate, 
		       bcm_sbx_cosq_queue_region_type_t *queue_region)
{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_params_ingress_t qparams;

    sal_memcpy(p_qstate, &queue_state[unit][q], sizeof(bcm_sbx_cosq_queue_state_t));
    sal_memcpy(p_bwstate,&bw_group_state[unit][p_qstate->bw_group], sizeof(bcm_sbx_cosq_bw_group_state_t));

    _bcm_sbx_cosq_get_default_queue_ingress_params(unit, &qparams, p_bwstate->base_queue, 0);
    *queue_region = qparams.local;

    if (p_bwstate->dest_mc) {
	bcm_sbx_cosq_multicast_ds_id_from_queue_group(unit, p_bwstate->base_queue, ds_id);
    } else {
	*ds_id = -1;
    }
    *fifo = 1;

    if (p_qstate->sysport != BCM_INT_SBX_INVALID_SYSPORT) {
	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_fd_fct_get, (unit, p_qstate->sysport, (p_qstate->ingress.bw_mode == BCM_COSQ_EF), p_bwstate->dest_mc, fifo));
    }
    return rv;
}

int
bcm_sbx_cosq_gport_sched_config_set(int unit, bcm_gport_t gport, int sched_mode, int int_pri,
                                                                                uint32 flags)
{
    int rv = BCM_E_NONE;


    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        return(BCM_E_UNAVAIL);
    }

    /* current support only for BM3200/QE2000 or BM9600/QE2000 configuration */
    if ((SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE) &&
	(SOC_SBX_CFG(unit)->uFabricConfig |= SOC_SBX_SYSTEM_CFG_VPORT_LEGACY ) &&
	(SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_MIX ) ) {
        return(BCM_E_UNAVAIL);
    }

    /* consistency checks */
    switch (sched_mode) {
        case BCM_COSQ_SP:
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
        case BCM_COSQ_EF:
        case BCM_COSQ_AF:
        case BCM_COSQ_SP_GLOBAL:
        case BCM_COSQ_BE:
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "mode(%d)\n"),
                      sched_mode));
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode value (%d)\n"),
                       sched_mode));
            return(BCM_E_PARAM);
    }
    if (flags & ~(BCM_COSQ_SCHED_CONFIG_EXPEDITE)) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: unsupported flags value (%d)\n"),
                   flags));
        return(BCM_E_PARAM);
    } 

    if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) ||
	(SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY ) ||
        (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX ) ) {
        if (gport != BCM_GPORT_INVALID) {
            return(BCM_E_PARAM);
        }
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_sched_config_set, (unit, gport, sched_mode, int_pri, flags));

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
bcm_sbx_cosq_gport_sched_config_get(int unit, bcm_gport_t gport, int sched_mode, int int_pri,
                                                                                uint32 *flags)

{
    int rv = BCM_E_NONE;


    if (!SOC_IS_SBX_NODE_ARBITER(unit)) {
        return(BCM_E_UNAVAIL);
    }

    /* current support only for BM3200/QE2000 or BM9600/QE2000 configuration */
    if ((SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_DMODE) &&
	(SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_LEGACY ) &&
         (SOC_SBX_CFG(unit)->uFabricConfig != SOC_SBX_SYSTEM_CFG_VPORT_MIX ) ) {
        return(BCM_E_UNAVAIL);
    }

    /* consistency checks */
    switch (sched_mode) {
        case BCM_COSQ_SP:
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
        case BCM_COSQ_EF:
        case BCM_COSQ_AF:
        case BCM_COSQ_SP_GLOBAL:
        case BCM_COSQ_BE:
            LOG_INFO(BSL_LS_BCM_COSQ,
                     (BSL_META_U(unit,
                                 "mode(%d)\n"),
                      sched_mode));
            break;

        default:
            LOG_ERROR(BSL_LS_BCM_COMMON,
                      (BSL_META_U(unit,
                                  "ERROR: unsupported mode value (%d)\n"),
                       sched_mode));
            return(BCM_E_PARAM);
    }

    if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) ||
	(SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY ) ||
        (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX )) {
        if (gport != BCM_GPORT_INVALID) {
            return(BCM_E_PARAM);
        }
    }

    BCM_SBX_LOCK(unit);

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_gport_sched_config_get, (unit, gport, sched_mode, int_pri, flags));

    BCM_SBX_UNLOCK(unit);

    return(rv);
}

int
bcm_sbx_cosq_sysport_to_ef_sysport(int unit, int sysport)
{
    int sysport_entry;


    if (SOC_SBX_CFG(unit)->arbitration_port_allocation == SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2) {
        sysport_entry = (sysport / BCM_INT_SBX_MAX_SYSPORT_GROUP);
        return(((sysport_entry % 2) == 0) ? sysport : (sysport - BCM_INT_SBX_MAX_SYSPORT_GROUP));
    }
    else{
        return(sysport & ~(0x1));
    }
}

int
bcm_sbx_cosq_sysport_to_nef_sysport(int unit, int sysport)
{
    int sysport_entry;


    if (SOC_SBX_CFG(unit)->arbitration_port_allocation == SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2) {
        sysport_entry = (sysport / BCM_INT_SBX_MAX_SYSPORT_GROUP);
        return(((sysport_entry % 2) == 0) ? (sysport + BCM_INT_SBX_MAX_SYSPORT_GROUP) : sysport);
    }
    else {
        return(sysport | (0x1));
    }
}

int
bcm_sbx_cosq_eset_to_ef_eset(int unit, int eset)
{
    int ef_eset;
    int cos, is_found = FALSE;


    if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
        if (eset < SOC_SBX_CFG(unit)->num_ds_ids) {
            ef_eset = eset;
        }
        else {
            ef_eset = eset - SOC_SBX_CFG(unit)->num_ds_ids;
        }
    }
    else {
        for (cos = 0; cos < SBX_FAB_MAX_MC_FIFOS; cos++) {
            if (SOC_SBX_CFG(unit)->is_mc_ef_cos[cos] == TRUE) {
                is_found = TRUE;
                break;
            }
        }

        if (is_found == FALSE) {
            return(-1);
        }

        ef_eset = BCM_INT_SBX_COSQ_ESET_TO_COS_ESET(unit, eset, cos);
    }

    return(ef_eset);
}

int
bcm_sbx_cosq_eset_to_nef_eset(int unit, int eset)
{
    int nef_eset;
    int cos, is_found = FALSE; 


    if (SBX_FAB_IS_STANDARD_ESET_RESOURCE_MODE(unit)) {
        if (eset < SOC_SBX_CFG(unit)->num_ds_ids) {
            nef_eset = SOC_SBX_CFG(unit)->num_ds_ids + eset;
            return(nef_eset);
        }
        else {
            nef_eset = eset;
        }
    }
    else {
        for (cos = 0; cos < SBX_FAB_MAX_MC_FIFOS; cos++) {
            if (SOC_SBX_CFG(unit)->is_mc_ef_cos[cos] == FALSE) {
                is_found = TRUE;
                break;
            }
        }

        if (is_found == FALSE) {
            return(-1);
        }

        nef_eset = BCM_INT_SBX_COSQ_ESET_TO_COS_ESET(unit, eset, cos);
    }

    return(nef_eset);
}

int
bcm_sbx_cosq_eset_to_cos_eset(int unit, int eset, int cos)
{
    int cos_eset;


    if (cos >= SOC_SBX_CFG(unit)->num_res_per_eset_spec) {
        return(-1);
    }

    cos_eset = eset + (SOC_SBX_CFG(unit)->num_ds_ids * cos);

    return(cos_eset);
}

int
bcm_sbx_cosq_eset_to_cos_eset_fcd(int unit, int eset, int cos, int *cos_eset, int *fcd)
{
    int rc = BCM_E_NONE;


    (*cos_eset) = bcm_sbx_cosq_eset_to_cos_eset(unit, eset, cos);
    if ((*cos_eset) == -1) {
        return(BCM_E_PARAM);
    }

    if ( (SOC_SBX_CFG(unit)->is_mc_ef_cos[cos] != TRUE) &&
                                     (SOC_SBX_CFG(unit)->is_mc_ef_cos[cos] != FALSE) ) {
        return(BCM_E_PARAM);
    }

    if (soc_feature(unit, soc_feature_egr_multicast_independent_fc)) {
        (*fcd) = (SOC_SBX_CFG(unit)->is_mc_ef_cos[cos] == TRUE) ? 0 : 1;
    }
    else {
        (*fcd) = 0;
    }

    return(rc);
}

int
bcm_sbx_cosq_get_node_port_from_sysport(int unit, int sysport, int *node_p, int *port_p, int *mc_p)
{
    (*port_p) = ((sysport) & 0x3f);
    (*node_p) = (((sysport) >> 6) & 0x3f);
    (*mc_p) = (((sysport) >> 12) & 0x1); 

    return(BCM_E_NONE);
}

int
bcm_sbx_cosq_get_sysport_from_node_port(int unit, int node, int port, int mc, int *sysport_p)
{
    (*sysport_p) = ((((mc) & 0x1) << 12) | (((node) & 0x3f) << 6) | ((port) & 0x3f));

    return(BCM_E_NONE);
}

int
bcm_sbx_cosq_get_eset_from_node_port(int unit, int node, int port, int mc, int *eset_p)
{
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
        (*eset_p) = ((node & 0x3f) << 6) | (port &0x3f);
    }
    else {
        if ((mc) == 1) {
            (*eset_p) = ((((node) & 1) << 6) + ((port) & 0x3f));
        }
        else {
            (*eset_p) = (((node) & 0x3F) * 14 + ((port) & 0xF) + BM9600_MAX_MULTICAST_ESETS);
        }
    }

    return(BCM_E_NONE);
}

int
bcm_sbx_cosq_get_node_port_from_eset(int unit, int eset, int *node_p, int *port_p, int *mc_p)
{
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT) {
        (*port_p) = ((eset) & 0x3f);
        (*node_p) = ((eset >> 6) & 0x3f);
        (*mc_p) = 1;
    }
    else {
        if ((eset) < BM9600_MAX_MULTICAST_ESETS) {
            (*port_p) = ((eset) & 0x3f);
            (*node_p) = ((eset) & 0x40) >> 6;
            (*mc_p) = 1;
        }
        else {
            (*port_p) = (((eset) - BM9600_MAX_MULTICAST_ESETS) % 14);
            (*node_p) = ((eset) - BM9600_MAX_MULTICAST_ESETS - (*port_p)) / 14;
            (*mc_p) = 0;
        }
    }

    return(BCM_E_NONE);
}

int bcm_sbx_cosq_gport_multipath_add(int unit, bcm_gport_t multipath_gport,
				     bcm_gport_t member_gport,
				     bcm_cos_queue_t member_gport_cosq)
{
    int egress;
    bcm_sbx_cosq_ingress_multipath_state_t *p_i_multipath;
    bcm_sbx_cosq_egress_multipath_state_t *p_e_multipath;
    int multipath, level, node;
    bcm_gport_t fabric_port;
    int egroup_num, num_fifos;
    int rv = BCM_E_NONE;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    /* free the handle and free the hardware resource if not detached yet */
    p_i_multipath = SOC_SBX_STATE(unit)->ingress_multipath_state;
    p_e_multipath = SOC_SBX_STATE(unit)->egress_multipath_state;
    multipath = BCM_INT_SBX_MULTIPATH_ID_GET(multipath_gport);
    
    if (BCM_COSQ_GPORT_IS_MULTIPATH(multipath_gport) == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: gport 0x%x not multipath gport\n"),
                   multipath_gport));
	return BCM_E_PARAM;
    }

    egress = BCM_INT_SBX_MULTIPATH_IS_EGRESS(multipath_gport);
    
    if (egress) {
	if ((BCM_GPORT_IS_EGRESS_GROUP(member_gport) == FALSE) &&
	    (BCM_GPORT_IS_EGRESS_CHILD(member_gport) == FALSE) &&
	    (BCM_GPORT_IS_EGRESS_MODPORT(member_gport) == FALSE) &&
	    (!(BCM_GPORT_IS_SCHEDULER(member_gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(member_gport)))) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: gport 0x%x not supported by egress multipath shaper\n"),
	               member_gport));
	    return BCM_E_PARAM;
	}

	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_egress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}

	p_e_multipath += multipath;
	if (p_e_multipath->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d not created\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
    } else {
	if (!((BCM_GPORT_IS_CHILD(member_gport)) ||
	      (BCM_GPORT_IS_SCHEDULER(member_gport) &&
	       BCM_INT_SBX_SCHEDULER_IS_INGRESS(member_gport)))) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: gport 0x%x type not supported by ingress multipath\n"),
	               member_gport));
	    return BCM_E_PARAM;
	}

	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_ingress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}

	p_i_multipath += multipath;
	if (p_i_multipath->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d not created\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
    }

    if (egress) {
	rv = bcm_sbx_port_get_scheduler(unit, member_gport, &level, &node);
	
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: can not find egress scheduler for gport 0x%x on unit %d\n"),
	               FUNCTION_NAME(), member_gport, unit));
	    return rv;	    
	}

	if ( (level < 0) || (node < 0) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
	               member_gport));
	    rv = BCM_E_PARAM;
	    return rv;		
	}

	if ( BCM_GPORT_IS_EGRESS_GROUP(member_gport) ) {
	    fabric_port = -1;
	    if (bcm_sbx_cosq_egress_group_info_get(unit, member_gport, &fabric_port, &egroup_num, &num_fifos) != BCM_E_NONE) { 
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d unknown\n"),
		           member_gport_cosq));
		rv = BCM_E_PARAM;
		return rv;	
	    }

	    if (member_gport_cosq < num_fifos) {
		node += member_gport_cosq;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
		           member_gport_cosq, num_fifos - 1));
		rv = BCM_E_PARAM;
		return rv;	
	    }
	}

	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_add,
				  (unit, egress, multipath, level, node));
    } else {
	/* update master level/node and num_nodes */
	rv = bcm_sbx_port_get_scheduler(unit, member_gport, &level, &node);
	
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: can not find ingress scheduler for gport 0x%x on unit %d\n"),
	               FUNCTION_NAME(), member_gport, unit));
	    return rv;	    
	}

	if ((p_i_multipath->level != -1) &&
	    (p_i_multipath->level != level)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress scheduler 0x%x not at same level as multipath gport\n"),
	               member_gport));
	    return BCM_E_PARAM;	    
	}

	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_add,
				  (unit, egress, multipath, level, node));
	if (rv == BCM_E_NONE) {
	    p_i_multipath->level = level;
	    p_i_multipath->num_nodes++;
	}
    }

    return rv;
}

int bcm_sbx_cosq_gport_multipath_delete(int unit, bcm_gport_t multipath_gport,
				     bcm_gport_t member_gport,
				     bcm_cos_queue_t member_gport_cosq)
{
    int egress;
    bcm_sbx_cosq_ingress_multipath_state_t *p_i_multipath;
    bcm_sbx_cosq_egress_multipath_state_t *p_e_multipath;
    int multipath, level, node;
    bcm_gport_t fabric_port;
    int egroup_num, num_fifos;
    int rv = BCM_E_NONE;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    /* free the handle and free the hardware resource if not detached yet */
    p_i_multipath = SOC_SBX_STATE(unit)->ingress_multipath_state;
    p_e_multipath = SOC_SBX_STATE(unit)->egress_multipath_state;
    multipath = BCM_INT_SBX_MULTIPATH_ID_GET(multipath_gport);
    
    if (BCM_COSQ_GPORT_IS_MULTIPATH(multipath_gport) == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: gport 0x%x not multipath gport\n"),
                   multipath_gport));
	return BCM_E_PARAM;
    }

    egress = BCM_INT_SBX_MULTIPATH_IS_EGRESS(multipath_gport);
    
    if (egress) {
	if ((BCM_GPORT_IS_EGRESS_GROUP(member_gport) == FALSE) &&
	    (BCM_GPORT_IS_EGRESS_CHILD(member_gport) == FALSE) &&
	    (BCM_GPORT_IS_EGRESS_MODPORT(member_gport) == FALSE) &&
	    (!(BCM_GPORT_IS_SCHEDULER(member_gport) && BCM_INT_SBX_SCHEDULER_IS_EGRESS(member_gport)))) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: gport 0x%x not supported by egress multipath shaper\n"),
	               member_gport));
	    return BCM_E_PARAM;
	}

	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_egress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}

	p_e_multipath += multipath;
	if (p_e_multipath->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d not created\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
    } else {
	if (!((BCM_GPORT_IS_CHILD(member_gport)) ||
	      (BCM_GPORT_IS_SCHEDULER(member_gport) &&
	       BCM_INT_SBX_SCHEDULER_IS_INGRESS(member_gport)))) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: gport 0x%x type not supported by ingress multipath\n"),
	               member_gport));
	    return BCM_E_PARAM;
	}

	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_ingress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}

	p_i_multipath += multipath;
	if (p_i_multipath->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d not created\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
    }

    if (egress) {
	rv = bcm_sbx_port_get_scheduler(unit, member_gport, &level, &node);
	
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: can not find egress scheduler for gport 0x%x on unit %d\n"),
	               FUNCTION_NAME(), member_gport, unit));
	    return rv;	    
	}

	if ( (level < 0) || (node < 0) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: unattached scheduler (0x%x), please attach first\n"),
	               member_gport));
	    rv = BCM_E_PARAM;
	    return rv;		
	}

	if ( BCM_GPORT_IS_EGRESS_GROUP(member_gport) ) {
	    fabric_port = -1;
	    if (bcm_sbx_cosq_egress_group_info_get(unit, member_gport, &fabric_port, &egroup_num, &num_fifos) != BCM_E_NONE) { 
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d unknown\n"),
		           member_gport_cosq));
		rv = BCM_E_PARAM;
		return rv;	
	    }

	    if (member_gport_cosq < num_fifos) {
		node += member_gport_cosq;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: fifo scheduler cos %d out of range [0-%d]\n"),
		           member_gport_cosq, num_fifos - 1));
		rv = BCM_E_PARAM;
		return rv;	
	    }
	}

	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_delete,
				  (unit, egress, multipath, level, node));	
    } else {
	/* update master level/node and num_nodes */
	rv = bcm_sbx_port_get_scheduler(unit, member_gport, &level, &node);
	
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "%s: can not find ingress scheduler for gport 0x%x on unit %d\n"),
	               FUNCTION_NAME(), member_gport, unit));
	    return rv;	    
	}

	if ((p_i_multipath->level != -1) &&
	    (p_i_multipath->level != level)) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress scheduler 0x%x not at same level as multipath gport\n"),
	               member_gport));
	    return BCM_E_PARAM;	    
	}

	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_delete,
				  (unit, egress, multipath, level, node));
	if (rv == BCM_E_NONE) {
	    p_i_multipath->num_nodes--;
	    if (p_i_multipath->num_nodes == 0) {
		p_i_multipath->level = -1;
		p_i_multipath->node = -1;
	    }
	}
    }

    return rv;
}

int bcm_sbx_cosq_gport_multipath_get(int unit, bcm_gport_t multipath_gport,
				     int member_gport_max,
				     bcm_gport_t *member_gport_array,
				     bcm_cos_queue_t *member_gport_cosq_array,
				     int *member_gport_count)
{
    int egress;
    bcm_sbx_cosq_ingress_multipath_state_t *p_i_multipath;
    bcm_sbx_cosq_egress_multipath_state_t *p_e_multipath;
    bcm_sbx_subport_info_t *sp_info;
    int multipath, node;
    int subport, egroup, cosq;
    int rv = BCM_E_NONE;

    if (!SOC_IS_SIRIUS(unit)) {
	return BCM_E_UNAVAIL;
    }

    /* free the handle and free the hardware resource if not detached yet */
    p_i_multipath = SOC_SBX_STATE(unit)->ingress_multipath_state;
    p_e_multipath = SOC_SBX_STATE(unit)->egress_multipath_state;
    multipath = BCM_INT_SBX_MULTIPATH_ID_GET(multipath_gport);
    
    if (BCM_COSQ_GPORT_IS_MULTIPATH(multipath_gport) == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "ERROR: gport 0x%x not multipath gport\n"),
                   multipath_gport));
	return BCM_E_PARAM;
    }

    egress = BCM_INT_SBX_MULTIPATH_IS_EGRESS(multipath_gport);
    *member_gport_count = 0;

    if (egress) {
	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_egress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}

	p_e_multipath += multipath;
	if (p_e_multipath->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: egress multipath id %d not created\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
    } else {
	if ( (multipath < 0) || (multipath >= SOC_SBX_CFG(unit)->num_ingress_multipath) ) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d out of range\n"),
	               multipath));
	    return BCM_E_PARAM;
	}

	p_i_multipath += multipath;
	if (p_i_multipath->in_use != TRUE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: ingress multipath id %d not created\n"),
	               multipath));
	    return BCM_E_PARAM;
	}
    }

    if (egress) {
	/* Only return Egress group gport*/
	for (subport = 0; subport < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; subport++) {
	    sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	    if (sp_info->valid == FALSE) {
		continue;
            }

	    for (egroup=0; egroup < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; egroup++) {
		if (sp_info->egroup[egroup].egroup_gport == BCM_GPORT_INVALID) {
		    continue;
		}
		if (sp_info->egroup[egroup].num_fifos <= 0) {
		    continue;
		}
		node = sp_info->egroup[egroup].es_scheduler_level0_node;
		for (cosq = node; cosq < node + sp_info->egroup[egroup].num_fifos; cosq++) {
		    /* if member is part of egroup, return it */
		    if ((p_e_multipath->member[cosq/32] & (1<<(cosq%32))) != 0) {
			*member_gport_array = sp_info->egroup[egroup].egroup_gport;
			*member_gport_cosq_array = cosq - node;
			member_gport_array++;
			member_gport_cosq_array++;
			*member_gport_count = (*member_gport_count + 1);
			if (*member_gport_count >= member_gport_max) {
			    return BCM_E_NONE;
			}
		    }
		}
	    }
	}
    } else {
	/* Scheduler or Child ports */

	/* Get all node that points to master, return logical scheduler id.
	 * for scheduler gport. 
	 * If the logical scheduler id doesn't exist, return level/node info
	 * and search subport info to return child gport
	 */
	*member_gport_count = member_gport_max;
	rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_multipath_get,
				  (unit, egress, multipath, member_gport_count,
				  member_gport_array, member_gport_cosq_array));
	if (rv != BCM_E_NONE) {
	    return rv;
	}
    }
    return rv;
}

int bcm_sbx_cosq_pfc_config_set(int unit, 
				bcm_gport_t gport, 
				bcm_cos_queue_t cosq, 
				uint32 flags, 
				bcm_cosq_pfc_config_t *config)
{
    int rv = BCM_E_UNAVAIL;

 
    if (config == NULL) {
        return(BCM_E_PARAM);
    }

    if (config->xoff_threshold < 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: invalid xoff_threshold\n")));
	return BCM_E_PARAM;
    }

    if (config->xon_threshold < 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: invalid xon_threshold\n")));
	return BCM_E_PARAM;
    }

    if (config->drop_threshold < 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: invalid drop_threshold\n")));
	return BCM_E_PARAM;
    }

    if (config->reserved_buffers < 0) {
	LOG_ERROR(BSL_LS_BCM_COMMON,
	          (BSL_META_U(unit,
	                      "ERROR: invalid reserved_buffers\n")));
	return BCM_E_PARAM;
    }

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_pfc_config_set,
			      (unit, gport, cosq, flags, config));
    
    return rv;
}

int bcm_sbx_cosq_pfc_config_get(int unit, 
				bcm_gport_t gport, 
				bcm_cos_queue_t cosq, 
				uint32 flags, 
				bcm_cosq_pfc_config_t *config)
{
    int rv = BCM_E_UNAVAIL;

    rv = MBCM_SBX_DRIVER_CALL(unit, mbcm_cosq_pfc_config_get,
			      (unit, gport, cosq, flags, config));
    
    return rv;
}

static int
_bcm_sbx_cosq_pfc_enabled(int unit, bcm_cos_queue_t cos)
{
    if (cos > 7) {
	/* return disabled if cos not supported */
	return FALSE;
    } else {
	if (SOC_SBX_CFG(unit)->pfc_cos_enable & (1<< (SOC_SBX_CFG(unit)->bcm_cosq_priority_group[cos]))) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    }
}

static int
_bcm_sbx_cosq_pfc_pg_get(int unit, bcm_cos_queue_t cos)
{
    if (cos > 7) {
	/* return 0 if cos not supported */
	return 0;
    } else {
	return SOC_SBX_CFG(unit)->bcm_cosq_priority_group[cos];
    }
}
