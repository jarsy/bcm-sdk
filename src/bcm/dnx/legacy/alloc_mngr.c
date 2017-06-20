/*
 * $Id: alloc_mngr.c,v 1.312 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        alloc_mngr.c
 * Purpose:     Resource allocation manager for DNX_SAND chips.
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_COMMON

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <bcm_int/common/debug.h>
#include <soc/debug.h>
#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/module.h>

#include <shared/swstate/sw_state_resmgr.h>
#include <shared/dnx_shr_template.h>

#include <bcm_int/dnx/legacy/error.h>
#include <bcm_int/dnx/legacy/alloc_mngr_shr.h>
#include <bcm_int/dnx/legacy/alloc_mngr.h>
#include <bcm_int/dnx/legacy/alloc_mngr_utils.h>
#include <bcm_int/dnx/legacy/alloc_mngr_cosq.h>
#include <bcm_int/dnx/legacy/cosq.h>

#include <soc/dnx/legacy/dnx_config_defs.h>

#include <soc/dnx/legacy/TMC/tmc_api_end2end_scheduler.h>
#include <soc/dnx/legacy/TMC/tmc_api_action_cmd.h>
#include <soc/dnx/legacy/TMC/tmc_api_egr_queuing.h>

#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnx/legacy/drv.h>

/*
 *  Alloc manager is designed to allow some flexibility in how things are
 *  allocated and tracked.  Several options are available for underlying
 *  resource managers, and a number of these options are designed to be used
 *  for specific purposes, optimising certain behaviours for example.
 *
 *  The initial configuration is established here, and exported to the
 *  rest of the DNX code.
 */

/* Egress thresh */
#define _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_LOW_ID (0)
#define _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_MAX_ENTITIES(unit) (_BCM_DNX_NOF_TM_PORTS(unit))
#define _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_SIZE   (sizeof(bcm_dnx_cosq_egress_thresh_key_info_t))

/* Mirror profile */
#define _DNX_AM_TEMPLATE_MIRROR_ACTION_MAX_ENTITIES (SOC_PPC_NOF_TRAP_CODES)  
#define _DNX_AM_TEMPLATE_MIRROR_ACTION_SIZE (sizeof(uint32))


/* Egress Interface Unicast thresh  - JER2_ARAD only*/
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_LOW_ID (0)
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_COUNT (8)
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_MAX_ENTITIES (SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces))
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_SIZE   (sizeof(bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t))

/* Egress Interface Multicast thresh - JER2_ARAD only*/
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_LOW_ID (0)
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_COUNT (4)
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_MAX_ENTITIES (SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces))
#define _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_SIZE   (sizeof(bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t))

/* COSQ Egress mapping */
#define _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_LOW_ID (0)
#define _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_COUNT (8)
#define _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_MAX_ENTITIES(unit) (_BCM_DNX_NOF_TM_PORTS(unit))
#define _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_SIZE   (sizeof(bcm_dnx_cosq_egress_queue_mapping_info_t))

/* NRDY threshold mapping */
#define _DNX_AM_TEMPLATE_NRDY_THRESHOLD_LOW_ID (0)
#define _DNX_AM_TEMPLATE_NRDY_THRESHOLD_COUNT (12)
#define _DNX_AM_TEMPLATE_NRDY_THRESHOLD_MAX_ENTITIES(unit) (SOC_DNX_IMP_DEFS_MAX(NOF_CORE_INTERFACES))
#define _DNX_AM_TEMPLATE_NRDY_THRESHOLD_SIZE   (sizeof(uint32))


/* Queue Rate class */
#define _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_LOW_ID (0)
#define _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_COUNT (DNX_TMC_ITM_RATE_CLASS_MAX+1)
#define _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_MAX_ENTITIES(unit) (SOC_DNX_DEFS_GET(unit, nof_queues) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores)
#define _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_SIZE   (sizeof(bcm_dnx_cosq_ingress_rate_class_info_t))

/* System RED drop probability */
#define _DNX_AM_TEMPLATE_SYS_RED_DP_PR_LOW_ID             (0)
#define _DNX_AM_TEMPLATE_SYS_RED_DP_PR_COUNT              (DNX_TMC_ITM_SYS_RED_DRP_PROBS)
#define _DNX_AM_TEMPLATE_SYS_RED_DP_PR_MAX_ENTITIES(unit) (SOC_DNX_DEFS_GET(unit, nof_queues) * 2) /* Mulipale NOF Queues by 2 since we have 2 pointer per Queue */
#define _DNX_AM_TEMPLATE_SYS_RED_DP_PR_SIZE               (sizeof(bcm_dnx_cosq_ingress_system_red_dp_pr_info_t))

/* VSQ PG TC mapping profile */
#define _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_LOW_ID (0)
#define _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_COUNT (DNX_TMC_NOF_VSQ_PG_MAPPING_PROFILES)
/*nof VSQFs*/
#define _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_MAX_ENTITIES(unit) (DNX_TMC_ITM_VSQ_GROUPF_SZE(unit))
#define _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING (sizeof(uint32))

/* VSQ Rate class */
#define _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID (0)
#define _DNX_AM_TEMPLATE_VSQ_A_RATE_CLS_COUNT(unit) (DNX_TMC_ITM_VSQ_A_QT_RT_CLS_MAX(unit) + 1)
#define _DNX_AM_TEMPLATE_VSQ_RATE_CLS_COUNT(unit) (DNX_TMC_ITM_VSQ_QT_RT_CLS_MAX(unit) + 1)
#define _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, vsq_group) (DNX_TMC_ITM_NOF_VSQS_IN_GROUP(unit, vsq_group) * SOC_DNX_DEFS_GET(unit, nof_cores))
#define _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(vsq_group)                                                               (\
            (vsq_group < DNX_TMC_ITM_VSQ_GROUP_SRC_PORT ) ? (sizeof(bcm_dnx_cosq_queue_vsq_rate_class_info_t)   ) : (\
            (vsq_group == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) ? (sizeof(bcm_dnx_cosq_src_port_vsq_rate_class_info_t[DNX_TMC_ITM_NOF_RSRC_POOLS])) : (\
            (vsq_group == DNX_TMC_ITM_VSQ_GROUP_PG      ) ? (sizeof(bcm_dnx_cosq_pg_vsq_rate_class_info_t)      ) : 0)))

void* _bcm_dnx_am_template_vsq_rate_cls_data (
        const DNX_TMC_ITM_VSQ_GROUP vsq_group, 
        const bcm_dnx_cosq_vsq_rate_class_info_t* data_rate_cls) {
    if (vsq_group < DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        return (void *)&(data_rate_cls->queue[vsq_group]);
    }
    if (vsq_group == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
        return (void *)&(data_rate_cls->port);
    }
    if (vsq_group == DNX_TMC_ITM_VSQ_GROUP_PG) {
        return (void *)&(data_rate_cls->pg);
    }
    return (void *)NULL;
}

/* Ingress Queue Discount Class */
#define _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_LOW_ID (0)
/* in new mechanism only half(8) profiles are actually active, in legacy mode all(16) profiles are active */
#define _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_COUNT(unit)  (DNX_TMC_ITM_PER_PACKET_COMPENSATION_ENABLED(unit) ? \
                                                            (DNX_TMC_ITM_NOF_CR_DISCNT_CLS_NDXS/2) : \
                                                             DNX_TMC_ITM_NOF_CR_DISCNT_CLS_NDXS)

#define _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_MAX_ENTITIES(unit) SOC_DNX_DEFS_GET(unit, nof_queues)
#define _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_SIZE   (sizeof(DNX_TMC_ITM_CR_DISCOUNT_INFO))

/* Egress Port Discount Class */
#define _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_LOW_ID (0)
#define _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_COUNT  (DNX_TMC_PORT_NOF_EGR_HDR_CR_DISCOUNT_TYPES)
/* nof entities is been done by UC/MC * header_type (port) */
#define _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_MAX_ENTITIES(unit) (DNX_TMC_NOF_FAP_PORTS_JER2_ARAD)
#define _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_SIZE   (sizeof(DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO))


/* e2e port hr flow control profile */
#define _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_LOW_ID        (0)
#define _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_COUNT         (DNX_TMC_SCH_LOW_FC_NOF_AVAIL_CONFS)
#define _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_MAX_ENTRIES   (DNX_TMC_NOF_FAP_PORTS_MAX)
#define _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_MAX_ENTRIES_PER_DEVICE(unit) (SOC_DNX_DEFS_GET(unit, nof_logical_ports)+1)
#define _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_SIZE          (sizeof(int))

/* sched (cl) class profile */
#define _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_LOW_ID        (0)
#define _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_COUNT         (DNX_TMC_SCH_NOF_CLASS_TYPES)
#define _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_MAX_ENTRIES(unit) ((DNX_TMC_CL_SE_ID_MAX_JER2_ARAD + 1) * 2)

#define _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_SIZE          (sizeof(bcm_dnx_cosq_sched_class_data_t))

/* TDM direct routing link pointer */
#define _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_LOW_ID (0)
#define _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_COUNT (36)
#define _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_MAX_ENTITIES (DNX_TMC_NOF_FAP_PORTS_JER2_ARAD) 
#define _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_SIZE   (sizeof(DNX_TMC_TDM_DIRECT_ROUTING_INFO))


/* Ingress Flow TC Mapping - JER2_ARAD ONLY */
#define _DNX_AM_TEMPLATE_INGRESS_FLOW_TC_MAPPING_LOW_ID (0)
#define _DNX_AM_TEMPLATE_INGRESS_FLOW_TC_MAPPING_COUNT (DNX_TMC_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES)
#define _DNX_AM_TEMPLATE_INGRESS_FLOW_TC_MAPPING_CLS_SIZE  (sizeof(DNX_TMC_ITM_TC_MAPPING))

/* Ingress UC TC Mapping - JER2_ARAD ONLY */
#define _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_LOW_ID (0)
#define _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_COUNT (DNX_TMC_NOF_INGRESS_UC_TC_MAPPING_PROFILES)
#define _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_MAX_ENTITIES(unit) (SOC_DNX_DEFS_GET(unit, nof_queues))
#define _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_CLS_SIZE  (sizeof(DNX_TMC_ITM_TC_MAPPING))

/* FC PFC Generic Bitmap Mapping - JER2_ARAD ONLY */
#define _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_LOW_ID (0)
#define _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_COUNT (SOC_DNX_DEFS_GET(unit, nof_fc_pfc_generic_bitmaps) + 1) /* NOF profiles + 1 dummy profile */
#define _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_MAX_ENTITIES (1024)
#define _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_CLS_SIZE (sizeof(DNX_TMC_FC_PFC_GENERIC_BITMAP))

/* PTP port Profiles - JER2_ARAD ONLY */
#define _DNX_AM_TEMPLATE_PTP_PORT_PROFILE_LOW_ID       (0)
#define _DNX_AM_TEMPLATE_PTP_PORT_PROFILE_COUNT        (SOC_PPC_PTP_IN_PP_PORT_PROFILES_NUM)
#define _DNX_AM_TEMPLATE_PTP_PORT_PROFILE_MAX_ENTITIES (_BCM_DNX_NOF_LOCAL_PORTS(unit))
#define _DNX_AM_TEMPLATE_PTP_PORT_PROFILE_CLS_SIZE     (sizeof(SOC_PPC_PTP_PORT_INFO))


#define ALLOC_MNGR_ACCESS                   sw_state_access[unit].dnx.bcm.alloc_mngr

int
bcm_dnx_am_mc_alloc(
        int unit,
        uint32 flags, /* flags should be SW_STATE_RES_ALLOC_WITH_ID; */
        DNX_TMC_MULT_ID  *mc_id,
        uint8 is_egress)
{
    int elem = *mc_id;
    int rv;
    BCMDNX_INIT_FUNC_DEFS;
    if (is_egress != FALSE) {
            elem += SOC_DNX_CONFIG(unit)->tm.nof_mc_ids;

    }
    rv = dnx_am_res_alloc(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_res_mc_dynamic,flags,1,&elem);

    BCMDNX_IF_ERR_EXIT(rv);
exit:
    BCMDNX_FUNC_RETURN;
}

int
bcm_dnx_am_mc_dealloc(
        int unit,
        DNX_TMC_MULT_ID mc_id,
        uint8 is_egress)
{
    int rv;
    int elem = mc_id;
    BCMDNX_INIT_FUNC_DEFS;
    if (is_egress != FALSE) {
            elem += SOC_DNX_CONFIG(unit)->tm.nof_mc_ids;
    }
    rv = dnx_am_res_free(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_res_mc_dynamic, 1, elem);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    BCMDNX_FUNC_RETURN;
}

int
bcm_dnx_am_mc_is_alloced(
        int unit,
        DNX_TMC_MULT_ID mc_id,
        uint8 is_egress)
{
    int elem = mc_id;
    if (is_egress != FALSE) {
            elem += SOC_DNX_CONFIG(unit)->tm.nof_mc_ids;
    }
    return dnx_am_res_check(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_res_mc_dynamic, 1, elem);
}





/* 
 * Template management - Start
 */ 
/* Util functions compare */
int
_bcm_dnx_am_template_memcpy_to_stream(const void* data, void *to_stream, size_t data_size)
{
    BCMDNX_INIT_FUNC_DEFS_NO_UNIT;
    sal_memcpy(to_stream,data,data_size);

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_memcpy_from_stream(void* data,const void* from_stream, size_t data_size)
{
    BCMDNX_INIT_FUNC_DEFS_NO_UNIT;
    sal_memcpy(data,from_stream,data_size);

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

int 
  _bcm_dnx_am_template_discount_cls_from_stream(void* data,const void* from_stream, size_t data_size)
{
    DNX_TMC_ITM_CR_DISCOUNT_INFO *discount_info = (DNX_TMC_ITM_CR_DISCOUNT_INFO *)data;
    const int *in_buffer = (const int*)from_stream;

    BCMDNX_INIT_FUNC_DEFS_NO_UNIT;
    DNX_TMC_ITM_CR_DISCOUNT_INFO_clear(discount_info);
    discount_info->discount = in_buffer[0];
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/* Discount class streams */
int 
  _bcm_dnx_am_template_discount_cls_to_stream(const void* data,void* to_stream, size_t data_size)    
{
    const DNX_TMC_ITM_CR_DISCOUNT_INFO *discount_info = (const DNX_TMC_ITM_CR_DISCOUNT_INFO *)data;
    int *out_buffer = (int*)to_stream;

    BCMDNX_INIT_FUNC_DEFS_NO_UNIT;
    sal_memset(out_buffer,0x0,data_size);

    out_buffer[0] = discount_info->discount;
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}



/* These arrays are used to intialize the dnx_shr_template callback arrays with the bcm template callbacks. */
dnx_shr_template_to_stream_t _bcm_dnx_am_template_to_stream_arr[_bcm_dnx_am_template_hash_compare_cb_idx_count] = {
    _bcm_dnx_am_template_memcpy_to_stream,
    NULL, /*_bcm_dnx_am_template_user_defined_trap_to_stream,*/
    NULL, /*_bcm_dnx_am_template_snoop_cmd_to_stream,*/
    _bcm_dnx_am_template_discount_cls_to_stream
};

dnx_shr_template_from_stream_t _bcm_dnx_am_template_from_stream_arr[_bcm_dnx_am_template_hash_compare_cb_idx_count] = {
    _bcm_dnx_am_template_memcpy_from_stream,
    NULL, /*_bcm_dnx_am_template_user_defined_trap_from_stream,*/
    NULL, /*_bcm_dnx_am_template_snoop_cmd_from_stream,*/
    _bcm_dnx_am_template_discount_cls_from_stream
};
                               

dnx_shr_template_print_func_t _bcm_dnx_am_template_print_func[dnx_am_template_count] = { NULL };

/* 
 * Initialize template management pool data structures for each pool type (not including some cosq related pools
 * which handle warmboot within the cosq module)
 */
int
_bcm_dnx_template_setup(int unit, int template_id, int core_id, int pool_id) {
    int rv = BCM_E_NONE;
    char *desc_str = "Unknown template";
    bcm_dnx_am_template_info_t t_info;
    
    BCMDNX_INIT_FUNC_DEFS;
    
    sal_memset(&t_info, 0, sizeof(bcm_dnx_am_template_info_t));
    t_info.pool_id = pool_id;
    t_info.template_id =  template_id;
    t_info.core_id = core_id;
    t_info.hashExtra = _bcm_dnx_am_template_hash_compare_cb_idx_memcpy;

    switch (template_id) 
    {
        case dnx_am_template_egress_thresh:
            /* mostly direct allocation */
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_COUNT(unit);

            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            t_info.max_entities = _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_MAX_ENTITIES(unit);
#endif 
            t_info.data_size = _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_SIZE;
            desc_str = "Egress threshold for drop and flow control";
            break;
    case dnx_am_template_egress_interface_unicast_thresh:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_MAX_ENTITIES;
            t_info.data_size = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_SIZE;
            desc_str = "Egress threshold for interface uc flow control";
            break;
        case dnx_am_template_egress_interface_multicast_thresh:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_MAX_ENTITIES;
            t_info.data_size = _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_SIZE;
            desc_str = "Egress threshold for interface mc flow control";
            break;
        case dnx_am_template_egress_queue_mapping:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_COUNT;
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            t_info.max_entities = _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_MAX_ENTITIES(unit);
#endif 
            t_info.data_size = _DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_SIZE;
            desc_str = "Egress queue mapping Per TM port";
            break;
        case dnx_am_template_nrdy_threshold:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_NRDY_THRESHOLD_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_NRDY_THRESHOLD_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_NRDY_THRESHOLD_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_NRDY_THRESHOLD_SIZE;
            desc_str = "NRDY threshold profile";
            break;                     
        case dnx_am_template_queue_rate_cls:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_QUEUE_RATE_CLS_SIZE;
            desc_str = "Queue Rate class";
            break;
        case dnx_am_template_system_red_dp_pr:
            t_info.manager       = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start        = _DNX_AM_TEMPLATE_SYS_RED_DP_PR_LOW_ID;
            t_info.count        = _DNX_AM_TEMPLATE_SYS_RED_DP_PR_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_SYS_RED_DP_PR_MAX_ENTITIES(unit);
            t_info.data_size    = _DNX_AM_TEMPLATE_SYS_RED_DP_PR_SIZE;
            desc_str = "System RED drop probability";
            break;
    case dnx_am_template_vsq_pg_tc_mapping:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING;
            desc_str = "PG TC mapping profile";
            break;
        case dnx_am_template_vsq_rate_cls_ct:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_A_RATE_CLS_COUNT(unit);
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, DNX_TMC_ITM_VSQ_GROUP_CTGRY);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(DNX_TMC_ITM_VSQ_GROUP_CTGRY);
            desc_str = "Category VSQ Rate class";
            break;
        case dnx_am_template_vsq_rate_cls_cttc:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_COUNT(unit);
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS);
            desc_str = "Traffic class VSQ Rate class";
            break;
        case dnx_am_template_vsq_rate_cls_ctcc:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_COUNT(unit);
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, DNX_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(DNX_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS);
            desc_str = "Connection class VSQ Rate class";
            break;
        case dnx_am_template_vsq_rate_cls_pp:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_COUNT(unit);
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG);
            desc_str = "Statistics VSQ Rate class";
            break;
        case dnx_am_template_vsq_rate_cls_src_port:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_COUNT(unit);
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, DNX_TMC_ITM_VSQ_GROUP_SRC_PORT);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(DNX_TMC_ITM_VSQ_GROUP_SRC_PORT);
            desc_str = "Source Port VSQ Rate class";
            break;
        case dnx_am_template_vsq_rate_cls_pg:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_COUNT(unit);
            t_info.max_entities = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, DNX_TMC_ITM_VSQ_GROUP_PG);
            t_info.data_size = _DNX_AM_TEMPLATE_VSQ_RATE_CLS_SIZE(DNX_TMC_ITM_VSQ_GROUP_PG);
            desc_str = "PG VSQ Rate class";
            break;
        case dnx_am_template_queue_discount_cls:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_LOW_ID;
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            t_info.count = _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_COUNT(unit);
#endif 
            t_info.max_entities = _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_SIZE;
            t_info.hashExtra = _bcm_dnx_am_template_hash_compare_cb_idx_discount_cls;
            desc_str = "Queue discount class";
            break;
        case dnx_am_template_egress_port_discount_cls_type_raw:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_SIZE;
            desc_str = "Egress port discount class type raw";
            break;
        case dnx_am_template_egress_port_discount_cls_type_cpu:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_SIZE;
            desc_str = "Egress port discount class type cpu";
            break;
        case dnx_am_template_egress_port_discount_cls_type_eth:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_SIZE;
            desc_str = "Egress port discount class type eth";
            break;
        case dnx_am_template_egress_port_discount_cls_type_tm:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_EGRESS_PORT_DISCNT_CLS_SIZE;
            desc_str = "Egress port discount class type tm";
            break;
        case dnx_am_template_cosq_port_hr_flow_control:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_MAX_ENTRIES_PER_DEVICE(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_SIZE;
            desc_str = "Cosq port hr fc";
            break;
        case dnx_am_template_cosq_sched_class:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_MAX_ENTRIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_COSQ_SCHED_CLASS_SIZE;
            desc_str = "sched class";
            break;
            
        case dnx_am_template_fabric_tdm_link_ptr:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_MAX_ENTITIES;
            t_info.data_size = _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_SIZE;
            desc_str = "TDM direct routing";
            break;
        case dnx_am_template_ingress_flow_tc_mapping:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_INGRESS_FLOW_TC_MAPPING_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_INGRESS_FLOW_TC_MAPPING_COUNT;
            t_info.max_entities = SOC_DNX_DEFS_GET(unit, nof_flows);
            t_info.data_size = _DNX_AM_TEMPLATE_INGRESS_FLOW_TC_MAPPING_CLS_SIZE;
            desc_str = "Ingress Flow TC Mapping Profiles";
            break;
        case dnx_am_template_ingress_uc_tc_mapping:
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_MAX_ENTITIES(unit);
            t_info.data_size = _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_CLS_SIZE;
            desc_str = "Ingress UC TC Mapping Profiles";
            break;
        case dnx_am_template_fc_generic_pfc_mapping:
        case dnx_am_template_fc_generic_pfc_mapping_c0:
        case dnx_am_template_fc_generic_pfc_mapping_c1:
        case dnx_am_template_fc_generic_pfc_mapping_c2:
        case dnx_am_template_fc_generic_pfc_mapping_c3:
        case dnx_am_template_fc_generic_pfc_mapping_c4:
        case dnx_am_template_fc_generic_pfc_mapping_c5:
        case dnx_am_template_fc_generic_pfc_mapping_c6:
        case dnx_am_template_fc_generic_pfc_mapping_c7:  
            t_info.manager = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start = _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_LOW_ID;
            t_info.count = _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_MAX_ENTITIES;
            t_info.data_size = _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_CLS_SIZE;
            desc_str = "FC PFC Generic Bitmap Mapping Profiles";
            break;
    case dnx_am_template_scheduler_adjust_size_final_delta: 
        if (SOC_IS_JERICHO(unit)) {
            t_info.manager      = DNX_SHR_TEMPLATE_MANAGE_HASH;
            t_info.start        = _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_PROFILE_LOW_ID;
            t_info.count        = _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_PROFILE_COUNT;
            t_info.max_entities = _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_MAX_ENTITIES;
            t_info.data_size    = _DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_CLS_SIZE;
            desc_str = "Scheduler final delta mapping profile";
        } else {
            BCM_EXIT;
        }

        break;
    default:
        rv = BCM_E_INTERNAL;
    }
 
    BCMDNX_IF_ERR_EXIT(rv);
    if (t_info.max_entities) {
        rv = dnx_shr_template_pool_set(unit,
                                   t_info.pool_id,
                                   t_info.manager,
                                   t_info.start,
                                   t_info.count,
                                   t_info.max_entities,
                                   t_info.global_max,
                                   t_info.data_size,
                                   &t_info.hashExtra,
                                   desc_str);
        BCMDNX_IF_ERR_EXIT(rv);

        rv = dnx_shr_template_type_set(unit,
                                   t_info.pool_id,
                                   t_info.pool_id,
                                   desc_str);
        BCMDNX_IF_ERR_EXIT(rv);
    }
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_thresh_init(int unit, int core ,int template_init_id,bcm_dnx_cosq_egress_thresh_key_info_t *egr_thresh_data_init)
{
    int index, result = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif    


    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
            result = dnx_am_template_allocate_group(unit, core, dnx_am_template_egress_thresh, flags, egr_thresh_data_init, 
                                                 _DNX_AM_TEMPLATE_COSQ_EGR_THRESH_MAX_ENTITIES(unit), &is_allocated,&template);
            BCMDNX_IF_ERR_EXIT(result);
#endif     
    /* Add all entities to template init id with given data  */

    /* We are not going to change the macro SOC_DNX_DEFS_MAX to avoid such cases */
    /* coverity[identical_branches] */
    for (index = 0; index < SOC_DNX_DEFS_MAX(NOF_LOGICAL_PORTS); index++) {
        result = ALLOC_MNGR_ACCESS.egr_thresh_map.set(unit, index, template_init_id);
        BCMDNX_IF_ERR_EXIT(result);
    }
     
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_thresh_data_get(int unit, int core, int port, bcm_dnx_cosq_egress_thresh_key_info_t * data)
{
    int old_profile;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = ALLOC_MNGR_ACCESS.egr_thresh_map.get(unit, port, &old_profile);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_profile < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_thresh,old_profile,data);

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_thresh_exchange(int unit, int core, int port, bcm_dnx_cosq_egress_thresh_key_info_t * data, int *old_template, int *is_last, int *template,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int temp;
    int temp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    rc = ALLOC_MNGR_ACCESS.egr_thresh_map.get(unit, port, &temp);
    BCMDNX_IF_ERR_EXIT(rc);

    if (temp < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    if (old_template != NULL) {
        *old_template = temp;
    }

    rc = dnx_am_template_exchange(unit, core, dnx_am_template_egress_thresh,0,data,temp,&temp_is_last,template,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (is_last != NULL) {
        *is_last = temp_is_last;
    }

    /* Since port-profile mapping is SW, update also the mapping. */
    rc = ALLOC_MNGR_ACCESS.egr_thresh_map.set(unit,port,*template);
    BCMDNX_IF_ERR_EXIT(rc);
    
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_egr_thresh_ref_get(int unit, int template, uint32 *ref_count)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_ref_count_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_egress_thresh, template, ref_count);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_egr_thresh_allocate_group(int unit, uint32 flags, bcm_dnx_cosq_egress_thresh_key_info_t *data, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_egress_thresh, flags, (void *)data, ref_count, is_allocated, template);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_egr_thresh_tdata_get(int unit, int template, bcm_dnx_cosq_egress_thresh_key_info_t *data)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_egress_thresh, template, (void *)data);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

/* Cosq Egress Thresh - End */

static int
_bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_port_profile_mapping_get(int unit, int core, int port, int *profile)
{
    int         index;
    bcm_error_t rv;
    BCMDNX_INIT_FUNC_DEFS;

    index = port + core*SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces);
    rv = ALLOC_MNGR_ACCESS.egr_interface_unicast_thresh_map.get(unit, index, profile);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    BCMDNX_FUNC_RETURN;
}

static int
_bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_port_profile_mapping_get(int unit, int core, int port, int *profile)
{
    int         index;
    bcm_error_t rv;
    BCMDNX_INIT_FUNC_DEFS;

    index = port + core*SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces);
    rv = ALLOC_MNGR_ACCESS.egr_interface_multicast_thresh_map.get(unit, index, profile);
    BCMDNX_IF_ERR_EXIT(rv);
exit:
    BCMDNX_FUNC_RETURN;
}


static int
_bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_port_profile_mapping_set(int unit, int core, int port, int profile)
{
    int rv, index;

    BCMDNX_INIT_FUNC_DEFS;

    /* _bcm_dnx_egr_interface_unicast_thresh_map[unit].map_port_thresh_type[port] = profile; */
    index = port + core*SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces);
    rv = ALLOC_MNGR_ACCESS.egr_interface_unicast_thresh_map.set(unit, index, profile);
    BCMDNX_IF_ERR_EXIT(rv);      


    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_init(int unit,
                                                                int core,
                                              int template_init_id,
                                              bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t *egr_interface_unicast_thresh_data_init)
{
    int index, is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template = template_init_id;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif    

    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;    
    
    /* Add all entities to template init id with given data */
    result = dnx_am_template_allocate_group(unit, core, dnx_am_template_egress_interface_unicast_thresh, flags, egr_interface_unicast_thresh_data_init, 
                                         _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_MAX_ENTITIES,&is_allocated,&template);
    BCMDNX_IF_ERR_EXIT(result);
    for (index = 0; index < _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_UNICAST_THRESH_MAX_ENTITIES; index++) {
        result = ALLOC_MNGR_ACCESS.egr_interface_unicast_thresh_map.set(unit,
            index + core*SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces),
            template_init_id);
        BCMDNX_IF_ERR_EXIT(result);
    }

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_data_get(int unit, int core, int interface_id, bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t * data)
{
    int old_profile;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_port_profile_mapping_get(unit, core, interface_id,&old_profile);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_profile < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_interface_unicast_thresh,old_profile,data);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_exchange(int unit, int core, int interface_id, bcm_dnx_cosq_egress_interface_unicast_thresh_key_info_t * data, int *old_template, int *is_last, int *template,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int temp;
    int temp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_port_profile_mapping_get(unit,core,interface_id,&temp);
    BCMDNX_IF_ERR_EXIT(rc);

    if (temp < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    if (old_template != NULL) {
        *old_template = temp;
    }

    rc = dnx_am_template_exchange(unit, core ,dnx_am_template_egress_interface_unicast_thresh,0,data,temp,&temp_is_last,template,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (is_last != NULL) {
        *is_last = temp_is_last;
    }

    /* Since port-profile mapping is SW, update also the mapping. */
    _bcm_dnx_am_template_cosq_egr_interface_unicast_thresh_port_profile_mapping_set(unit,core,interface_id,*template);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}
/* Cosq Egress Interface Unicast Thresh - End */

static int
_bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_port_profile_mapping_set(int unit, int core, int interface_id, int profile)
{
    int rv,index;

    BCMDNX_INIT_FUNC_DEFS;

    /* _bcm_dnx_egr_interface_multicast_thresh_map[unit].map_port_thresh_type[interface_id] = profile; */
    index = interface_id + core*SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces);
    rv = ALLOC_MNGR_ACCESS.egr_interface_multicast_thresh_map.set(unit, index, profile);
    BCMDNX_IF_ERR_EXIT(rv);      

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_init(int unit,
                                                                  int core,
                                              int template_init_id,
                                              bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t *egr_interface_multicast_thresh_data_init)
{
    int index, is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template = template_init_id;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif    

    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;    
    
    result = dnx_am_template_allocate_group(unit, core, dnx_am_template_egress_interface_multicast_thresh, flags, egr_interface_multicast_thresh_data_init, 
                                         _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_MAX_ENTITIES, &is_allocated,&template);
    BCMDNX_IF_ERR_EXIT(result);    
    /* Add all entities to template init id with given data */
    for (index = 0; index < _DNX_AM_TEMPLATE_COSQ_EGR_INTERFACE_MULTICAST_THRESH_MAX_ENTITIES; index++) {
        result = ALLOC_MNGR_ACCESS.egr_interface_multicast_thresh_map.set(unit,
            index + core*SOC_DNX_IMP_DEFS_GET(unit, nof_core_interfaces),
            template_init_id);
        BCMDNX_IF_ERR_EXIT(result);
    }
 
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_data_get(int unit, int core, int interface_id, bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t * data)
{
    int old_profile;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_port_profile_mapping_get(unit,core,interface_id,&old_profile);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_profile < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    rc = dnx_am_template_data_get(unit, core,dnx_am_template_egress_interface_multicast_thresh,old_profile,data);

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_exchange(int unit, int core, int interface_id, bcm_dnx_cosq_egress_interface_multicast_thresh_key_info_t * data, int *old_template, int *is_last, int *template,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int temp;
    int temp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_port_profile_mapping_get(unit,core,interface_id,&temp);
    BCMDNX_IF_ERR_EXIT(rc);

    if (temp < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    if (old_template != NULL) {
        *old_template = temp;
    }

    rc = dnx_am_template_exchange(unit, core,dnx_am_template_egress_interface_multicast_thresh,0,data,temp,&temp_is_last,template,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (is_last != NULL) {
        *is_last = temp_is_last;
    }

    /* Since port-profile mapping is SW, update also the mapping. */
    _bcm_dnx_am_template_cosq_egr_interface_multicast_thresh_port_profile_mapping_set(unit,core, interface_id,*template);
    BCMDNX_IF_ERR_EXIT(rc);
    
exit:
    BCMDNX_FUNC_RETURN;
}
/* Cosq Egress Interface Multicast Thresh - End */



/* Cosq Egress Queue Mapping - Start */
int _bcm_dnx_am_template_egress_queue_mapping_init(int unit,int template_init_id, int core, const bcm_dnx_cosq_egress_queue_mapping_info_t* mapping_profile)
{
  
    
    BCMDNX_INIT_FUNC_DEFS;
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    template = template_init_id;
    /* Add all entities to template init id with given data */
    result = dnx_am_template_allocate_group(unit, core, dnx_am_template_egress_queue_mapping, flags, mapping_profile,_DNX_AM_TEMPLATE_EGR_QUEUE_MAPPING_MAX_ENTITIES(unit), &is_allocated,&template);
    BCMDNX_IF_ERR_EXIT(result);
exit:
#endif 

    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_egress_queue_mapping_data_get(int unit, uint32 tm_port, int core, bcm_dnx_cosq_egress_queue_mapping_info_t * mapping_profile)
{
    int index;
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    rc = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_egr_q_profile_map_get,(unit, core, tm_port, (uint32*)&index));
    BCMDNX_IF_ERR_EXIT(rc);

    if (index < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_queue_mapping, index, mapping_profile);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_egress_queue_mapping_exchange(int unit, uint32 tm_port, int core, CONST bcm_dnx_cosq_egress_queue_mapping_info_t * mapping_profile, int *old_profile, int *is_last, int *new_profile,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_profile;
    int tmp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    rc = MBCM_DNX_SOC_DRIVER_CALL(unit,mbcm_dnx_egr_q_profile_map_get,(unit, core, tm_port, (uint32*)&tmp_old_profile));
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange(unit, core, dnx_am_template_egress_queue_mapping, 0, mapping_profile, tmp_old_profile, &tmp_is_last, new_profile, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_profile != NULL) {
        *old_profile = tmp_old_profile;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

exit:
    BCMDNX_FUNC_RETURN;
}

/* Cosq Egress Queue Mapping - End */

/* NRDY threshold - Start */

int
_bcm_dnx_am_template_nrdy_threshold_alloc(int unit, int core, int flags, uint32 *data, int *is_allocated, int *profile)
{
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_allocate(unit, core, dnx_am_template_nrdy_threshold, flags, data, is_allocated, profile);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_nrdy_threshold_free(int unit, int core, int profile, int *is_last)
{
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_free(unit, core, dnx_am_template_nrdy_threshold, profile, is_last);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_nrdy_threshold_exchange(int unit, int core, uint32 tm_port, int flags, uint32 *data,
                                             int *old_profile, int *is_last, int *profile, int *is_allocated)
{
    int rc = BCM_E_NONE;
    soc_port_t port;

    BCMDNX_INIT_FUNC_DEFS;

    /* get old profile */
    BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_tm_to_local_port_get, (unit, core, tm_port, &port)));
    BCMDNX_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_port_nrdy_th_profile_get, (unit, port, (uint32*)old_profile)));

    /* exchange */
    rc = dnx_am_template_exchange(unit, core, dnx_am_template_nrdy_threshold, flags, data, *old_profile, is_last, profile, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

/* NRDY threshold - End */




/* COSQ Ingress rate class - Start */
static int
_bcm_dnx_am_template_queue_rate_cls_mapping_get(int unit, int core, int queue, int *rate_cls)
{
    uint32 dnx_sand_rv;    
    DNX_TMC_ITM_QUEUE_INFO queue_info;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(rate_cls);

    DNX_TMC_ITM_QUEUE_INFO_clear(&queue_info);

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_queue_info_get,(unit, core, queue,&queue_info)));
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);

    *rate_cls = queue_info.rate_cls;

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_queue_rate_cls_init(int unit,int template_init_id,const bcm_dnx_cosq_ingress_rate_class_info_t* data_rate_cls)
{
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;
    BCMDNX_INIT_FUNC_DEFS;
    /*Relevant only for only for simples queue to rate cls mapping mode*/
    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;   
    template = template_init_id;
        
    /* Add all entities to template init id with given data */ 
    result = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_template_queue_rate_cls,flags,data_rate_cls,_DNX_AM_TEMPLATE_QUEUE_RATE_CLS_MAX_ENTITIES(unit),&is_allocated,&template);
    BCMDNX_IF_ERR_EXIT(result);

exit:
    BCMDNX_FUNC_RETURN;
}
int _bcm_dnx_am_template_queue_rate_cls_data_get(int unit, int core, int queue,bcm_dnx_cosq_ingress_rate_class_info_t * data_rate_cls)
{
    int rate_cls;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_queue_rate_cls_mapping_get(unit, core, queue, &rate_cls);
    BCMDNX_IF_ERR_EXIT(rc);

    if (rate_cls < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("failed to get rate class")));
    }

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_template_queue_rate_cls,rate_cls,data_rate_cls);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

/* Exchange the rate class with a new one with the given data, return the new rate class and if the previous rate class is now unused. */
int _bcm_dnx_am_template_queue_rate_cls_exchange(int unit, int core, int queue, bcm_dnx_cosq_ingress_rate_class_info_t * data_rate_cls, int *old_rate_cls, int *is_last, int *new_rate_cls,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_rate_cls;
    int tmp_is_last, core_index;
    BCMDNX_INIT_FUNC_DEFS;
    /* Get current rate class for queue from HW */
    rc = _bcm_dnx_am_template_queue_rate_cls_mapping_get(unit, core, queue, &tmp_old_rate_cls);
    BCMDNX_IF_ERR_EXIT(rc);
    rc = dnx_am_template_exchange(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_template_queue_rate_cls,0,data_rate_cls,tmp_old_rate_cls,&tmp_is_last,new_rate_cls,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (core == BCM_CORE_ALL) {
        /*If we map all cores then exchange with ID for the rest of the core*/
        for (core_index = 0; core_index < (SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores - 1); core_index++) {
            rc = dnx_am_template_exchange(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_template_queue_rate_cls,DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID,data_rate_cls,tmp_old_rate_cls,&tmp_is_last,new_rate_cls,is_allocated);
            BCMDNX_IF_ERR_EXIT(rc);
        }
    }
    if (old_rate_cls != NULL) {
        *old_rate_cls = tmp_old_rate_cls;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}


/* Test if an exchange the rate class with a new one with the given data is possible, return the new rate class and if the previous rate class is now unused. */
int _bcm_dnx_am_template_queue_rate_cls_exchange_test(int unit, int core, int queue, bcm_dnx_cosq_ingress_rate_class_info_t * data_rate_cls, int *old_rate_cls, int *is_last, int *new_rate_cls,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_rate_cls;
    int tmp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    /* Get current rate class for queue from HW */
    rc = _bcm_dnx_am_template_queue_rate_cls_mapping_get(unit, core, queue, &tmp_old_rate_cls);
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange_test(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_template_queue_rate_cls,0,data_rate_cls,tmp_old_rate_cls,&tmp_is_last,new_rate_cls,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_rate_cls != NULL) {
        *old_rate_cls = tmp_old_rate_cls;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_queue_rate_cls_ref_get(int unit, int template, uint32 *ref_count)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_ref_count_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_queue_rate_cls, template, ref_count);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_queue_rate_cls_allocate_group(int unit, uint32 flags, bcm_dnx_cosq_ingress_rate_class_info_t *data_rate_cls, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;
    int is_last;

    BCMDNX_INIT_FUNC_DEFS;
    if (ref_count > 0) {
        rc = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_queue_rate_cls, flags, (void *)data_rate_cls, ref_count, is_allocated, template);
        BCMDNX_IF_ERR_EXIT(rc);
    } else {
        rc = dnx_am_template_free_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_queue_rate_cls, *template, -ref_count, &is_last);
        BCMDNX_IF_ERR_EXIT(rc);
    }

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_queue_rate_cls_tdata_get(int unit, int template, bcm_dnx_cosq_ingress_rate_class_info_t *data_rate_cls)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_queue_rate_cls, template, (void *)data_rate_cls);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}


/* COSQ Ingress rate class - End */

/* COSQ VSQ rate class - Start */
int
bcm_dnx_am_vsq_src_port_alloc(
    int unit,
    int core_id,
    uint32 flags,
    int src_pp_port,
    int *vsq_index)
{
    int rv = BCM_E_NONE;
    int src_port_vsq_index;
    int pg_base;
    uint8 enable;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(vsq_index);
    if (src_pp_port >= SOC_MAX_NUM_PORTS) {
        BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }
    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }
    rv = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_itm_src_vsqs_mapping_get ,(unit, core_id, src_pp_port, &src_port_vsq_index, &pg_base, &enable)));
    BCMDNX_IF_ERR_EXIT(rv);
    if (enable) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PORT, (_BSL_BCM_MSG("Source PP port %d, is already mapped."), src_pp_port));
    }
    rv = dnx_am_res_alloc(unit, core_id, dnx_am_res_vsq_src_port, flags, 1, vsq_index);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    BCMDNX_FUNC_RETURN;
}

int
bcm_dnx_am_vsq_src_port_free(
    int unit,
    int core_id,
    int vsq_index)
{
    int rv = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    if (vsq_index >= DNX_TMC_ITM_VSQ_GROUPE_SZE(unit)) {
        BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }
    if (core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }
    rv = dnx_am_res_free(unit, core_id, dnx_am_res_vsq_src_port, 1, vsq_index);
    BCMDNX_IF_ERR_EXIT(rv);

exit:
    BCMDNX_FUNC_RETURN;
}
int
bcm_dnx_am_pg_init(int unit)
{
    int rv = BCM_E_NONE, core_index;
    int pg_base = 0;
    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif    

    BCM_DNX_CORES_ITER(BCM_CORE_ALL, core_index) {
        rv = bcm_dnx_am_pg_alloc(unit, core_index, DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID, 1, &pg_base);
        BCMDNX_IF_ERR_EXIT(rv);

        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.pg_numq.set(unit, core_index, pg_base, 1);
        BCMDNX_IF_ERR_EXIT(rv);
    }
exit:
    BCMDNX_FUNC_RETURN;
}
int
bcm_dnx_am_pg_alloc(
    int unit,
    int core_id,
    uint32 flags,
    int numq,
    int *pg_base)
{
    int rv = BCM_E_NONE, core_index;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(pg_base);
    
    rv = dnx_am_res_alloc(unit, core_id, dnx_am_res_vsq_pg, flags, numq, pg_base);
    BCMDNX_IF_ERR_EXIT(rv);

    BCM_DNX_CORES_ITER(core_id, core_index) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.pg_numq.set(unit, core_index, *pg_base, numq);
        BCMDNX_IF_ERR_EXIT(rv);
    }
exit:
    BCMDNX_FUNC_RETURN;
}
int
bcm_dnx_am_pg_check(
    int unit,
    int core_id,
    int pg_base
    )
{
    BCMDNX_INIT_FUNC_DEFS;
    BCM_RETURN_VAL_EXIT(dnx_am_res_check(unit, core_id, dnx_am_res_vsq_pg, 1, pg_base));
exit:
    BCMDNX_FUNC_RETURN;
}
int
bcm_dnx_am_pg_get(
    int unit,
    int core_id,
    int pg_base,
    int *numq
    )
{
    int rv = BCM_E_NONE, core_index;
    uint8 numq_get;
    BCMDNX_INIT_FUNC_DEFS;
    core_index = (core_id == BCM_CORE_ALL) ? 0 : core_id;
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.pg_numq.get(unit, core_index, pg_base, &numq_get);
    DNXC_IF_ERR_EXIT(rv);
    *numq = numq_get;
exit:
    BCMDNX_FUNC_RETURN;
}

int
bcm_dnx_am_pg_free(
    int unit,
    int core_id,
    int pg_base)
{
    int rv = BCM_E_NONE, core_index;
    int numq;
    BCMDNX_INIT_FUNC_DEFS;

    rv = bcm_dnx_am_pg_get(unit, core_id, pg_base, &numq);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_am_res_free(unit, core_id, dnx_am_res_vsq_pg, numq, pg_base);
    BCMDNX_IF_ERR_EXIT(rv);

    BCM_DNX_CORES_ITER(core_id, core_index) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.tm_info.pg_numq.set(unit, core_index, pg_base, 0);
        DNXC_IF_ERR_EXIT(rv);
    }
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_vsq_pg_tc_mapping_profile_init(int unit, int core_index, uint32 pg_tc_bitmap){
    int core_id = 0;
    bcm_error_t rc = BCM_E_NONE;
    int is_allocated;
    uint32 flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    int template = 0; /*Default profile for all PGs*/
    BCMDNX_INIT_FUNC_DEFS;
#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif    

    BCM_DNX_CORES_ITER(core_index, core_id) {
        rc = dnx_am_template_allocate_group(
           unit,
           core_id,
           dnx_am_template_vsq_pg_tc_mapping,
           flags,
           &pg_tc_bitmap,
           _DNX_AM_TEMPLATE_VSQ_PG_TC_MAPPING_MAX_ENTITIES(unit),
           &is_allocated,
           &template);
        BCMDNX_IF_ERR_EXIT(rc);
    }
exit:
    BCMDNX_FUNC_RETURN;
}
int _bcm_dnx_am_template_vsq_pg_tc_mapping_profile_data_get(int unit, int core_id, int pg_tc_profile, uint32 *pg_tc_bitmap){
    bcm_error_t rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(pg_tc_bitmap);

    rc = dnx_am_template_data_get(unit, core_id, dnx_am_template_vsq_pg_tc_mapping, pg_tc_profile, pg_tc_bitmap);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}
    
int _bcm_dnx_am_template_vsq_pg_tc_mapping_exchange(int unit, int core_id, uint32 pg_tc_bitmap, int old_pg_tc_profile, int *is_last, int *new_pg_tc_profile,int *is_allocated) {
    int rc = BCM_E_NONE;
    uint32 flags = 0;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(is_last);
    BCMDNX_NULL_CHECK(new_pg_tc_profile);
    BCMDNX_NULL_CHECK(is_allocated);

    rc = dnx_am_template_exchange(unit, core_id, dnx_am_template_vsq_pg_tc_mapping, flags, &pg_tc_bitmap, old_pg_tc_profile, is_last, new_pg_tc_profile, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}
static int
_bcm_dnx_am_template_vsq_rate_cls_mapping_get(int unit,int core_id, DNX_TMC_ITM_VSQ_NDX vsq_id, DNX_TMC_ITM_VSQ_GROUP *vsq_group, int *rate_cls)
{
    bcm_error_t rc = BCM_E_NONE;
    uint32 dnx_sand_rv;
    DNX_TMC_ITM_VSQ_NDX vsq_in_group_ndx = 0;
    uint32 tmp = 0;
    uint8 is_ocb_only = FALSE;
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(rate_cls);
    BCMDNX_NULL_CHECK(vsq_group);

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL_WITHOUT_DEV_ID(unit,mbcm_dnx_itm_vsq_index_global2group,(unit, vsq_id, vsq_group, &vsq_in_group_ndx, &is_ocb_only)));
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_vsq_qt_rt_cls_get,(unit, core_id, is_ocb_only, *vsq_group, vsq_in_group_ndx, &tmp)));
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);
    *rate_cls = tmp;

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_vsq_rate_cls_init(int unit, int template_init_id, const bcm_dnx_cosq_vsq_rate_class_info_t* data_rate_cls)
{
    DNX_TMC_ITM_VSQ_GROUP vsq_group;
    const int vsq_template[DNX_TMC_NOF_VSQ_GROUPS] = {dnx_am_template_vsq_rate_cls_ct,
                                                      dnx_am_template_vsq_rate_cls_cttc,
                                                      dnx_am_template_vsq_rate_cls_ctcc,
                                                      dnx_am_template_vsq_rate_cls_pp,
                                                      dnx_am_template_vsq_rate_cls_src_port,
                                                      dnx_am_template_vsq_rate_cls_pg};
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;

    BCMDNX_INIT_FUNC_DEFS;
    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    
    template = template_init_id;
    
    for (vsq_group = 0; vsq_group < DNX_TMC_NOF_VSQ_GROUPS; vsq_group++) {
        /* Add all entities to template init id with given data */
        result = dnx_am_template_allocate_group(
                    unit, 
                    BCM_DNX_AM_DEFAULT_POOL_IDX, 
                    vsq_template[vsq_group], 
                    flags, 
                    _bcm_dnx_am_template_vsq_rate_cls_data(vsq_group, data_rate_cls), 
                    _DNX_AM_TEMPLATE_VSQ_RATE_CLS_MAX_ENTITIES(unit, vsq_group), 
                    &is_allocated, 
                    &template);
        BCMDNX_IF_ERR_EXIT(result);
    }

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_vsq_rate_cls_data_get(int unit, int core_id, int vsq_id, bcm_dnx_cosq_vsq_rate_class_info_t * data_rate_cls)
{
    int rate_cls;
    int rc = BCM_E_NONE;
    DNX_TMC_ITM_VSQ_GROUP vsq_group;
    const int vsq_template[DNX_TMC_NOF_VSQ_GROUPS] = {dnx_am_template_vsq_rate_cls_ct,
                                                      dnx_am_template_vsq_rate_cls_cttc,
                                                      dnx_am_template_vsq_rate_cls_ctcc,
                                                      dnx_am_template_vsq_rate_cls_pp,
                                                      dnx_am_template_vsq_rate_cls_src_port,
                                                      dnx_am_template_vsq_rate_cls_pg};
    BCMDNX_INIT_FUNC_DEFS;

    if ((core_id < 0 || core_id >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core_id != BCM_CORE_ALL) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("unit(%d) core out of range %d"), unit, core_id));
    }

    rc = _bcm_dnx_am_template_vsq_rate_cls_mapping_get(unit, core_id, vsq_id, &vsq_group, &rate_cls);
    BCMDNX_IF_ERR_EXIT(rc);

    if (rate_cls < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get rate class")));
    }

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, vsq_template[vsq_group], rate_cls, _bcm_dnx_am_template_vsq_rate_cls_data(vsq_group, data_rate_cls));
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_vsq_rate_cls_exchange(int unit, int core_id, int vsq_id, bcm_dnx_cosq_vsq_rate_class_info_t * data_rate_cls, int *old_rate_cls, int *is_last, int *new_rate_cls, int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_rate_cls;
    int tmp_is_last;
    const int vsq_template[DNX_TMC_NOF_VSQ_GROUPS] = {dnx_am_template_vsq_rate_cls_ct,
                                                      dnx_am_template_vsq_rate_cls_cttc,
                                                      dnx_am_template_vsq_rate_cls_ctcc,
                                                      dnx_am_template_vsq_rate_cls_pp,
                                                      dnx_am_template_vsq_rate_cls_src_port,
                                                      dnx_am_template_vsq_rate_cls_pg};
    DNX_TMC_ITM_VSQ_GROUP vsq_group;
    BCMDNX_INIT_FUNC_DEFS;

    rc = _bcm_dnx_am_template_vsq_rate_cls_mapping_get(unit, core_id, vsq_id, &vsq_group, &tmp_old_rate_cls);
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, vsq_template[vsq_group], 0, _bcm_dnx_am_template_vsq_rate_cls_data(vsq_group, data_rate_cls), tmp_old_rate_cls, &tmp_is_last, new_rate_cls, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_rate_cls != NULL) {
        *old_rate_cls = tmp_old_rate_cls;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_vsq_rate_cls_ref_get(int unit, int template, DNX_TMC_ITM_VSQ_GROUP vsq_group, uint32 *ref_count)
{
    int rc = BCM_E_NONE;
    const int vsq_template[DNX_TMC_NOF_VSQ_GROUPS] = {dnx_am_template_vsq_rate_cls_ct,
                                                      dnx_am_template_vsq_rate_cls_cttc,
                                                      dnx_am_template_vsq_rate_cls_ctcc,
                                                      dnx_am_template_vsq_rate_cls_pp,
                                                      dnx_am_template_vsq_rate_cls_src_port,
                                                      dnx_am_template_vsq_rate_cls_pg};

    BCMDNX_INIT_FUNC_DEFS;
    if(vsq_group < 0 || vsq_group >= DNX_TMC_NOF_VSQ_GROUPS) {
        BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }
    rc = dnx_am_template_ref_count_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, vsq_template[vsq_group], template, ref_count);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_vsq_rate_cls_allocate_group(int unit, uint32 flags, DNX_TMC_ITM_VSQ_GROUP vsq_group, bcm_dnx_cosq_vsq_rate_class_info_t *data_rate_cls, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;
    const int vsq_template[DNX_TMC_NOF_VSQ_GROUPS] = {dnx_am_template_vsq_rate_cls_ct,
                                                      dnx_am_template_vsq_rate_cls_cttc,
                                                      dnx_am_template_vsq_rate_cls_ctcc,
                                                      dnx_am_template_vsq_rate_cls_pp,
                                                      dnx_am_template_vsq_rate_cls_src_port,
                                                      dnx_am_template_vsq_rate_cls_pg};
    BCMDNX_INIT_FUNC_DEFS;
    if(vsq_group < 0 || vsq_group >= DNX_TMC_NOF_VSQ_GROUPS) {
        BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }
    rc = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, vsq_template[vsq_group], flags, _bcm_dnx_am_template_vsq_rate_cls_data(vsq_group, data_rate_cls), ref_count, is_allocated, template);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_vsq_rate_cls_tdata_get(int unit, int template, DNX_TMC_ITM_VSQ_GROUP vsq_group, bcm_dnx_cosq_vsq_rate_class_info_t *data_rate_cls)
{
    int rc = BCM_E_NONE;
    const int vsq_template[DNX_TMC_NOF_VSQ_GROUPS] = {dnx_am_template_vsq_rate_cls_ct,
                                              dnx_am_template_vsq_rate_cls_cttc,
                                              dnx_am_template_vsq_rate_cls_ctcc,
                                              dnx_am_template_vsq_rate_cls_pp,
                                              dnx_am_template_vsq_rate_cls_src_port,
                                              dnx_am_template_vsq_rate_cls_pg};
    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, vsq_template[vsq_group], template, _bcm_dnx_am_template_vsq_rate_cls_data(vsq_group, data_rate_cls));
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}


/* COSQ vsq rate class - End */


/* System RED drop probability value class - Start */
static int
_bcm_dnx_am_template_system_red_dp_pr_mapping_get(int unit, int dp_pr_ndx, int *dp_pr_template_ndx)
{
    DNX_TMC_ITM_SYS_RED_DROP_PROB info;

    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(dp_pr_template_ndx);

    DNX_TMC_ITM_SYS_RED_DROP_PROB_clear(&info);

    *dp_pr_template_ndx = dp_pr_ndx;

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_system_red_dp_pr_init(int unit,int template_init_id,const bcm_dnx_cosq_ingress_system_red_dp_pr_info_t* data_sys_red_dp_pr)
{
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif    

    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;   
    template = template_init_id;
    
    /* Add all entities to template init id with given data */ 
    result = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_system_red_dp_pr, flags,data_sys_red_dp_pr, 
                                         _DNX_AM_TEMPLATE_SYS_RED_DP_PR_MAX_ENTITIES(unit), &is_allocated, &template);
    BCMDNX_IF_ERR_EXIT(result);
 
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_system_red_dp_pr_data_get(int unit, int dp_pr_ndx, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t * data_sys_red_dp_pr)
{
    int sys_red_dp_pr_template_ndx;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_system_red_dp_pr_mapping_get(unit, dp_pr_ndx, &sys_red_dp_pr_template_ndx);
    BCMDNX_IF_ERR_EXIT(rc);

    if (dp_pr_ndx < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("failed to get Drop pr")));
    }

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_system_red_dp_pr, sys_red_dp_pr_template_ndx, data_sys_red_dp_pr);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

/* Exchange the DP PR with a new one with the given data, return the new DP PR  and if the previous DP PR is now unused. */
int _bcm_dnx_am_template_system_red_dp_pr_exchange(int unit, int dp_pr_ndx, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t *data_sys_red_dp_pr, 
                                                   int *old_sys_red_dp_pr_ndx, int *is_last, int *new_sys_red_dp_pr_ndx, int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_sys_red_dp_pr_ndx;
    int tmp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    
    rc = _bcm_dnx_am_template_system_red_dp_pr_mapping_get(unit ,dp_pr_ndx, &tmp_old_sys_red_dp_pr_ndx);
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_system_red_dp_pr, 0, data_sys_red_dp_pr, tmp_old_sys_red_dp_pr_ndx,
                               &tmp_is_last, new_sys_red_dp_pr_ndx, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_sys_red_dp_pr_ndx != NULL) {
        *old_sys_red_dp_pr_ndx = tmp_old_sys_red_dp_pr_ndx;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_system_red_dp_pr_ref_get(int unit, int template, uint32 *ref_count)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_ref_count_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_system_red_dp_pr, template, ref_count);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_system_red_dp_pr_allocate_group(int unit, uint32 flags, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t *data_sys_red_dp_pr, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_system_red_dp_pr, flags, (void *)data_sys_red_dp_pr, ref_count, is_allocated, template);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_system_red_dp_pr_tdata_get(int unit, int template, bcm_dnx_cosq_ingress_system_red_dp_pr_info_t *data_sys_red_dp_pr)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_system_red_dp_pr, template, (void *)data_sys_red_dp_pr);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}


/* System RED drop probability value class - END */


/* COSQ Ingress discount class - Start */
int
_bcm_dnx_am_template_queue_discount_cls_mapping_get(int unit, int core, int queue, int *discount_cls)
{
    bcm_error_t rc = BCM_E_NONE;
    uint32 dnx_sand_rv;
    DNX_TMC_ITM_QUEUE_INFO queue_info;

    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(discount_cls);

    DNX_TMC_ITM_QUEUE_INFO_clear(&queue_info);

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_queue_info_get,(unit,core ,queue,&queue_info)));    
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);

    *discount_cls = queue_info.credit_cls;

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_queue_discount_cls_init(int unit, int template_init_id,const DNX_TMC_ITM_CR_DISCOUNT_INFO* data_discount_cls)
{
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;
    int core;

    BCMDNX_INIT_FUNC_DEFS;
    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    template = template_init_id;
    
    /* Add all entities to template init id with given data */
    BCM_DNX_CORES_ITER(BCM_CORE_ALL, core) {
        result = dnx_am_template_allocate_group(unit, core, dnx_am_template_queue_discount_cls, flags, data_discount_cls, 
                                                _DNX_AM_TEMPLATE_QUEUE_DISCNT_CLS_MAX_ENTITIES(unit), &is_allocated,&template);
        BCMDNX_IF_ERR_EXIT(result);
    }
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_queue_discount_cls_data_get(int unit, int core, int queue,DNX_TMC_ITM_CR_DISCOUNT_INFO * data_discount_cls)
{
    int discount_cls;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_queue_discount_cls_mapping_get(unit, core, queue,&discount_cls);
    BCMDNX_IF_ERR_EXIT(rc);

    if (discount_cls < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get discount class")));
    }

    rc = dnx_am_template_data_get(unit, core, dnx_am_template_queue_discount_cls,discount_cls,data_discount_cls);

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_queue_discount_cls_exchange(int unit, int core, int queue, DNX_TMC_ITM_CR_DISCOUNT_INFO * data_discount_cls, int *old_discount_cls, int *is_last, int *new_discount_cls,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_discount_cls;
    int tmp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_queue_discount_cls_mapping_get(unit, core, queue,&tmp_old_discount_cls);
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange(unit, core, dnx_am_template_queue_discount_cls,0,data_discount_cls,tmp_old_discount_cls,&tmp_is_last,new_discount_cls,is_allocated);
    if (rc != BCM_E_NONE) {
        BCMDNX_ERR_EXIT_MSG(rc, (_BSL_BCM_MSG("failed to get a free credit discount template")));
    }

    if (old_discount_cls != NULL) {
        *old_discount_cls = tmp_old_discount_cls;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_queue_discount_cls_ref_get(int unit, int core, int template, uint32 *ref_count)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_ref_count_get(unit, core, dnx_am_template_queue_discount_cls, template, ref_count);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_queue_discount_cls_allocate_group(int unit, int core, uint32 flags, DNX_TMC_ITM_CR_DISCOUNT_INFO *data_discount_cls, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_allocate_group(unit, core, dnx_am_template_queue_discount_cls, flags, (void *)data_discount_cls, ref_count, is_allocated, template);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_queue_discount_cls_tdata_get(int unit, int core, int template, DNX_TMC_ITM_CR_DISCOUNT_INFO *data_discount_cls)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_data_get(unit, core, dnx_am_template_queue_discount_cls, template, (void *)data_discount_cls);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}


/* COSQ Ingress discount class - End */

/* COSQ Egress discount class - Start */

static int _bcm_dnx_hedear_type_map(int unit ,DNX_TMC_PORT_HEADER_TYPE header_type, DNX_TMC_PORT_HEADER_TYPE *header_type_val){
    
    bcm_error_t rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;
    switch (header_type) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
        case DNX_TMC_PORT_HEADER_TYPE_NONE:
        case DNX_TMC_PORT_HEADER_TYPE_STACKING:
        case DNX_TMC_PORT_HEADER_TYPE_TDM:
        case DNX_TMC_PORT_HEADER_TYPE_TDM_RAW:        
        case DNX_TMC_PORT_HEADER_TYPE_XGS_HQoS: 
        case DNX_TMC_PORT_HEADER_TYPE_XGS_MAC_EXT:
        case DNX_TMC_PORT_HEADER_TYPE_MIRROR_RAW:
            *header_type_val =  DNX_TMC_PORT_HEADER_TYPE_RAW;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
        case DNX_TMC_PORT_HEADER_TYPE_UDH_ETH:
            *header_type_val =  DNX_TMC_PORT_HEADER_TYPE_ETH;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            *header_type_val =  DNX_TMC_PORT_HEADER_TYPE_TM;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_INJECTED:
        case DNX_TMC_PORT_HEADER_TYPE_INJECTED_PP:
        case DNX_TMC_PORT_HEADER_TYPE_INJECTED_2: 
        case DNX_TMC_PORT_HEADER_TYPE_INJECTED_2_PP: 
        case DNX_TMC_PORT_HEADER_TYPE_PROG:            
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            *header_type_val = DNX_TMC_PORT_HEADER_TYPE_CPU;
            break;  
		 case DNX_TMC_PORT_HEADER_TYPE_L2_ENCAP_EXTERNAL_CPU:
            *header_type_val =  DNX_TMC_PORT_HEADER_TYPE_L2_ENCAP_EXTERNAL_CPU;
            break;	      
        default:
            rc = BCM_E_PARAM;
            break;
   }
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

static int
_bcm_dnx_am_template_egress_port_discount_cls_mapping_get(int unit, int core, uint32 tm_port, int *discount_profile,DNX_TMC_PORT_HEADER_TYPE *header_type_outgoing)
{
    bcm_error_t rc = BCM_E_NONE;
    uint32 dnx_sand_rv;
    DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_TYPE discount_type = DNX_TMC_PORT_NOF_EGR_HDR_CR_DISCOUNT_TYPES;
    DNX_TMC_PORT_HEADER_TYPE header_type_incoming;

    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(discount_profile);

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_port_egr_hdr_credit_discount_select_get,(unit, core, tm_port,&discount_type)));    
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);   
    *discount_profile = discount_type;

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_port_header_type_get,(unit,core,tm_port,&header_type_incoming,header_type_outgoing)));
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);

    
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_egress_port_discount_cls_free(int unit, int core, uint32 tm_port, int* is_last)
{
    int rc = BCM_E_NONE;
    int template;
    DNX_TMC_PORT_HEADER_TYPE header_type, header_type_map;
    _dnx_am_template_t template_type = -1;

    BCMDNX_INIT_FUNC_DEFS;

    rc = _bcm_dnx_am_template_egress_port_discount_cls_mapping_get(unit, core, tm_port,&template, &header_type);
    BCMDNX_IF_ERR_EXIT(rc);

    rc = _bcm_dnx_hedear_type_map(unit,header_type, &header_type_map);
    BCMDNX_IF_ERR_EXIT(rc);

    switch (header_type_map) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
            template_type = dnx_am_template_egress_port_discount_cls_type_raw;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            template_type = dnx_am_template_egress_port_discount_cls_type_cpu;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
            template_type = dnx_am_template_egress_port_discount_cls_type_eth;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            template_type = dnx_am_template_egress_port_discount_cls_type_tm;
            break;
        default: 
            BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }

    rc = dnx_am_template_free(unit, core, template_type,template,is_last);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_egress_port_discount_cls_init(int unit, int core, uint32 tm_port,const DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO* data_discount_cls)
{
    int is_allocated, result = BCM_E_NONE;
    bcm_error_t rc;
    uint32 flags = 0;
    int template;
    DNX_TMC_PORT_HEADER_TYPE header_type, header_type_map;
    _dnx_am_template_t template_type = -1;

    BCMDNX_INIT_FUNC_DEFS;
    /*flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;*/
    rc = _bcm_dnx_am_template_egress_port_discount_cls_mapping_get(unit, core, tm_port,&template, &header_type);
    BCMDNX_IF_ERR_EXIT(rc);
    rc = _bcm_dnx_hedear_type_map(unit,header_type, &header_type_map);
    BCMDNX_IF_ERR_EXIT(rc);

    switch (header_type_map) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
            template_type = dnx_am_template_egress_port_discount_cls_type_raw;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            template_type = dnx_am_template_egress_port_discount_cls_type_cpu;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:  
            template_type = dnx_am_template_egress_port_discount_cls_type_eth;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            template_type = dnx_am_template_egress_port_discount_cls_type_tm;
            break;
        default: 
            BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }

    result = dnx_am_template_allocate(unit, core, template_type, flags, data_discount_cls, &is_allocated,&template);
    BCMDNX_IF_ERR_EXIT(result);

    DNX_BCM_SAND_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_port_egr_hdr_credit_discount_select_set,(unit, core, tm_port,template)));

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_egress_port_discount_cls_data_get(int unit, int core, uint32 tm_port,DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO * data_discount_cls)
{
    int discount_profile;
    DNX_TMC_PORT_HEADER_TYPE header_type,header_type_map;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;

    rc = _bcm_dnx_am_template_egress_port_discount_cls_mapping_get(unit, core, tm_port, &discount_profile, &header_type);
    
    BCMDNX_IF_ERR_EXIT(rc);

    rc = _bcm_dnx_hedear_type_map(unit,header_type, &header_type_map);
    BCMDNX_IF_ERR_EXIT(rc);

    if (discount_profile < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get discount profile")));
    }
    switch (header_type_map) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
            rc = dnx_am_template_data_get(unit, core,dnx_am_template_egress_port_discount_cls_type_raw,discount_profile,data_discount_cls);
            break;    
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            rc = dnx_am_template_data_get(unit, core,dnx_am_template_egress_port_discount_cls_type_cpu,discount_profile,data_discount_cls);
            break;   
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
            rc = dnx_am_template_data_get(unit, core,dnx_am_template_egress_port_discount_cls_type_eth,discount_profile,data_discount_cls);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            rc = dnx_am_template_data_get(unit, core,dnx_am_template_egress_port_discount_cls_type_tm,discount_profile,data_discount_cls);
            break;
        default: 
            rc = BCM_E_PARAM;

    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_egress_port_discount_cls_exchange(int unit, int core, uint32 tm_port, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO * data_discount_cls, int *old_discount_profile, int *is_last, int *new_discount_cls,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_discount_profile;
    DNX_TMC_PORT_HEADER_TYPE header_type,header_type_map;
    int tmp_is_last;
    _dnx_am_template_t template_type = -1;

    BCMDNX_INIT_FUNC_DEFS;
    
    rc = _bcm_dnx_am_template_egress_port_discount_cls_mapping_get(unit, core, tm_port, &tmp_old_discount_profile,&header_type);
    BCMDNX_IF_ERR_EXIT(rc);
    rc = _bcm_dnx_hedear_type_map(unit,header_type, &header_type_map);
    BCMDNX_IF_ERR_EXIT(rc);


    switch (header_type_map) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
            template_type = dnx_am_template_egress_port_discount_cls_type_raw;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            template_type = dnx_am_template_egress_port_discount_cls_type_cpu;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
            template_type = dnx_am_template_egress_port_discount_cls_type_eth;
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            template_type = dnx_am_template_egress_port_discount_cls_type_tm;
            break;
        default: 
            BCMDNX_IF_ERR_EXIT(BCM_E_PARAM);
    }

    rc = dnx_am_template_exchange(unit,core,template_type,0,data_discount_cls,tmp_old_discount_profile,&tmp_is_last,new_discount_cls,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);


    if (old_discount_profile != NULL) {
        *old_discount_profile = tmp_old_discount_profile;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_egress_port_discount_cls_per_header_type_ref_get(int unit, int core, int template, DNX_TMC_PORT_HEADER_TYPE header_type, uint32 *ref_count)
{
    DNX_TMC_PORT_HEADER_TYPE header_type_map;
    int rc = BCM_E_NONE;
  
    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_hedear_type_map(unit,header_type, &header_type_map);
    BCMDNX_IF_ERR_EXIT(rc);
    switch (header_type_map) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW: 
            rc = dnx_am_template_ref_count_get(unit, core, dnx_am_template_egress_port_discount_cls_type_raw, template, ref_count);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            rc = dnx_am_template_ref_count_get(unit, core, dnx_am_template_egress_port_discount_cls_type_cpu, template, ref_count);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
            rc = dnx_am_template_ref_count_get(unit, core, dnx_am_template_egress_port_discount_cls_type_eth, template, ref_count);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            rc = dnx_am_template_ref_count_get(unit, core, dnx_am_template_egress_port_discount_cls_type_tm, template, ref_count);
            break;
        default: 
            rc = BCM_E_PARAM;
    }

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_egress_port_discount_cls_per_header_type_tdata_get(int unit, int core, int template, DNX_TMC_PORT_HEADER_TYPE header_type, DNX_TMC_PORT_EGR_HDR_CR_DISCOUNT_INFO *data_discount_cls)
{
    DNX_TMC_PORT_HEADER_TYPE header_type_map;
    int rc = BCM_E_NONE;
    

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_hedear_type_map(unit,header_type, &header_type_map);
    BCMDNX_IF_ERR_EXIT(rc);
    switch (header_type_map) {
        case DNX_TMC_PORT_HEADER_TYPE_RAW:
            rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_port_discount_cls_type_raw, template, (void *)data_discount_cls);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_CPU:
            rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_port_discount_cls_type_cpu, template, (void *)data_discount_cls);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_ETH:
        case DNX_TMC_PORT_HEADER_TYPE_MPLS_RAW:
            rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_port_discount_cls_type_eth, template, (void *)data_discount_cls);
            break;
        case DNX_TMC_PORT_HEADER_TYPE_TM:
            rc = dnx_am_template_data_get(unit, core, dnx_am_template_egress_port_discount_cls_type_tm, template, (void *)data_discount_cls);
            break;
        default: 
            rc = BCM_E_PARAM;
    }
    
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}




/* COSQ Egress discount class - End */

/* COSQ Port hr flow control profile - Start */

/* port ->  flow control profile */
typedef struct _dnx_cosq_port_hr_fc_map_s {
    int profile[_DNX_AM_TEMPLATE_COSQ_PORT_HR_FC_MAX_ENTRIES];
} _dnx_cosq_port_hr_fc_map_t;

static _dnx_cosq_port_hr_fc_map_t _bcm_dnx_cosq_port_hr_fc_map[BCM_MAX_NUM_UNITS][SOC_DNX_DEFS_MAX(NOF_CORES)];

static int
_bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_get(int unit, int core, int port, int *profile)
{
    BCMDNX_INIT_FUNC_DEFS;

    (*profile) = _bcm_dnx_cosq_port_hr_fc_map[unit][core].profile[port];

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/* Set mapping - since get mapping is implemented as s/w cache */ 
int
_bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_set(int unit, int core, int port, int profile)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCM_DNX_UNIT_CHECK(unit);

    _bcm_dnx_cosq_port_hr_fc_map[unit][core].profile[port] = profile;

    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

/* Template_init_id will always be present in the system */
int
_bcm_dnx_am_template_cosq_port_hr_fc_init(int unit, int core, int template_init_id, int *fc_data_init)
{
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    int index, is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;


    BCMDNX_INIT_FUNC_DEFS;
    /* template_init_id will always be present in the system */

    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    template = template_init_id;

    
    result =
        dnx_am_template_allocate_group(
            unit, core, dnx_am_template_cosq_port_hr_flow_control,
            flags, fc_data_init,_BCM_DNX_NOF_TM_PORTS(unit), &is_allocated, &template);
    BCMDNX_IF_ERR_EXIT(result);

    /* Add all entities to template init id with given data */
    for (index = 0; index <= _BCM_DNX_NOF_TM_PORTS(unit); index++) {
        result = _bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_set(unit, core, index, template_init_id);
        BCMDNX_IF_ERR_EXIT(result);
    }
   
exit:
    BCMDNX_FUNC_RETURN;
#endif 
    return -1;
}

/*
 * Note: this procedure is currently not in use.
 */
int
_bcm_dnx_am_template_cosq_port_hr_fc_data_get(int unit, int core, int port, int *data)
{
    int old_profile;
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;
    _bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_get(unit, core, port, &old_profile);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_profile < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    rc = dnx_am_template_data_get(unit, core, dnx_am_template_cosq_port_hr_flow_control, old_profile, data);

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_port_hr_fc_exchange(int unit, int core, int port, int *data, int *old_template, int *is_last, int *template, int *is_allocated)
{
    int rc = BCM_E_NONE;
    int temp;
    int temp_is_last;


    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_get(unit, core, port, &temp);
    BCMDNX_IF_ERR_EXIT(rc);

    if (temp < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    if (old_template != NULL) {
        *old_template = temp;
    }

    rc = dnx_am_template_exchange(unit, core, dnx_am_template_cosq_port_hr_flow_control, 0, data, temp, &temp_is_last, template, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (is_last != NULL) {
        *is_last = temp_is_last;
    }

    /* Since port-profile mapping is implemented as s/w cache. */
    _bcm_dnx_am_template_cosq_port_hr_fc_profile_mapping_set(unit, core, port, (*template));
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_hr_fc_ref_get(int unit, int core, int template, uint32 *ref_count)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_ref_count_get(unit, core, dnx_am_template_cosq_port_hr_flow_control, template, ref_count);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_hr_fc_allocate_group(int unit, int core, uint32 flags, int *data, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_allocate_group(unit, core, dnx_am_template_cosq_port_hr_flow_control, flags, (void *)data, ref_count, is_allocated, template);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}



/* COSQ Port Hr flow control profile - End */


/* COSQ CL scheduler profile - Start */

int
_bcm_dnx_am_template_cosq_sched_class_init(int unit)
{
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_sched_class_allocate(int unit, int core, int flags, int *data, int *is_allocated, int *class_template)
{
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_allocate(unit, core /* BCM_DNX_AM_DEFAULT_POOL_IDX */, dnx_am_template_cosq_sched_class, flags, data, is_allocated, class_template);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_sched_class_free(int unit, int core, int class_template, int *is_last)
{
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_free(unit, core /* BCM_DNX_AM_DEFAULT_POOL_IDX */, dnx_am_template_cosq_sched_class, class_template, is_last);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_sched_class_data_get(int unit, int core, int class_template, int *data)
{
    int rc = BCM_E_NONE;

    
    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_data_get(unit, core /* BCM_DNX_AM_DEFAULT_POOL_IDX */, dnx_am_template_cosq_sched_class, class_template, data);
    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_sched_class_exchange(int unit, int core, int flags, int *data, int old_class_template, int *is_last, int *class_template, int *is_allocated)
{
    int rc = BCM_E_NONE;
    int temp_is_last;

    
    BCMDNX_INIT_FUNC_DEFS;
    rc = dnx_am_template_exchange(unit, core /* BCM_DNX_AM_DEFAULT_POOL_IDX */, dnx_am_template_cosq_sched_class, flags, data, old_class_template, &temp_is_last, class_template, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (is_last != NULL) {
        (*is_last) = temp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_sched_class_ref_get(int unit, int core, int template, uint32 *ref_count)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_ref_count_get(unit, core /* BCM_DNX_AM_DEFAULT_POOL_IDX */, dnx_am_template_cosq_sched_class, template, ref_count);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_dnx_am_template_cosq_sched_class_allocate_group(int unit, int core, uint32 flags, int *data, int ref_count, int *is_allocated, int *template)
{
    int rc = BCM_E_NONE;


    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_allocate_group(unit, core /* was BCM_DNX_AM_DEFAULT_POOL_IDX */, dnx_am_template_cosq_sched_class, flags, (void *)data, ref_count, is_allocated, template);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}


/* COSQ CL scheduler profile - End */



/* Fabric TDM direct routing - Start */
static int
_bcm_dnx_am_template_fabric_tdm_link_ptr_mapping_get(int unit, int port,int *link_ptr)
{
    bcm_error_t rc = BCM_E_NONE;
    uint32 dnx_sand_rv;
    uint32 link_pointer = 0; 
    
    BCMDNX_INIT_FUNC_DEFS;
    BCMDNX_NULL_CHECK(link_ptr);

    dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_tdm_direct_routing_profile_map_get,(unit,port,&link_pointer)));    
    DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);    

    *link_ptr = link_pointer;

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_fabric_tdm_link_ptr_init(int unit,int template_init_id,const DNX_TMC_TDM_DIRECT_ROUTING_INFO* routing_info)
{
    int index, is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif   
    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    template = template_init_id;
    
    /* Add all entities to template init id with given data */
    for (index = 0; index < _DNX_AM_TEMPLATE_FABRIC_TDM_LINK_PTR_MAX_ENTITIES; index++) {
        result = dnx_am_template_allocate(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, dnx_am_template_fabric_tdm_link_ptr, flags, routing_info, &is_allocated,&template);
        BCMDNX_IF_ERR_EXIT(result);
    }

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_fabric_tdm_link_ptr_data_get(int unit,int port,DNX_TMC_TDM_DIRECT_ROUTING_INFO* routing_info)
{
    int link_pointer;
    int rc = BCM_E_NONE;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_fabric_tdm_link_ptr_mapping_get(unit,port,&link_pointer);
    BCMDNX_IF_ERR_EXIT(rc);

    if (link_pointer < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get link pointer")));
    }

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_fabric_tdm_link_ptr_exchange(int unit, int port, DNX_TMC_TDM_DIRECT_ROUTING_INFO * routing_info, int *old_link_ptr, int *is_last, int *new_link_ptr,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_link_ptr;
    int tmp_is_last;

    BCMDNX_INIT_FUNC_DEFS;
    rc = _bcm_dnx_am_template_fabric_tdm_link_ptr_mapping_get(unit,port,&tmp_old_link_ptr);
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange(unit, BCM_DNX_AM_DEFAULT_POOL_IDX,dnx_am_template_fabric_tdm_link_ptr,0,routing_info,tmp_old_link_ptr,&tmp_is_last,new_link_ptr,is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_link_ptr != NULL) {
        *old_link_ptr = tmp_old_link_ptr;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

    BCMDNX_IF_ERR_EXIT(rc);
exit:
    BCMDNX_FUNC_RETURN;
}
/* Fabric TDM direct routing - End */


/* Ingress Flow TC Mapping - Start */
static int
_bcm_dnx_am_template_ingress_flow_tc_mapping_get(int unit, int core_id, int dest_ndx ,uint32 *profile_ndx)
{
  bcm_error_t rc = BCM_E_NONE;
  uint32 dnx_sand_rv;

  BCMDNX_INIT_FUNC_DEFS;
  BCMDNX_NULL_CHECK(profile_ndx);

  dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ipq_tc_profile_get,(unit, core_id, 1, dest_ndx , profile_ndx)));
  DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);

  BCMDNX_IF_ERR_EXIT(rc);
exit:
  BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_ingress_flow_tc_mapping_data_get(int unit, int core_id, int dest_ndx,DNX_TMC_ITM_TC_MAPPING *tc_mapping)
{
  uint32 profile_ndx;
  int rc = BCM_E_NONE;
  int core = (core_id == BCM_CORE_ALL) ? BCM_DNX_AM_DEFAULT_POOL_IDX : core_id;
  BCMDNX_INIT_FUNC_DEFS;
  rc = _bcm_dnx_am_template_ingress_flow_tc_mapping_get(unit, core, dest_ndx, &profile_ndx);
  BCMDNX_IF_ERR_EXIT(rc);

  rc = dnx_am_template_data_get(unit, core, dnx_am_template_ingress_flow_tc_mapping, profile_ndx, tc_mapping);
  BCMDNX_IF_ERR_EXIT(rc);

exit:
  BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_ingress_flow_tc_mapping_init(int unit,int template_init_id,const DNX_TMC_ITM_TC_MAPPING* tc_mapping)
{
  int is_allocated, result = BCM_E_NONE;
  uint32 flags = 0;
  int template;
  int core_id;
  BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif 

  flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
  template = template_init_id;

  /* Add all entities to template init id with given data */
  SOC_DNX_ASSYMETRIC_CORES_ITER(BCM_CORE_ALL, core_id) {
      result = dnx_am_template_allocate_group(unit, core_id, dnx_am_template_ingress_flow_tc_mapping, flags, tc_mapping, SOC_DNX_DEFS_GET(unit, nof_flows), &is_allocated,&template);
      BCMDNX_IF_ERR_EXIT(result);
  }
exit:
  BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_ingress_flow_tc_mapping_exchange(int unit, int core_id, int dest_ndx, DNX_TMC_ITM_TC_MAPPING *tc_mapping, uint32 *old_mapping_profile, int *is_last, int *new_mapping_profile,int *is_allocated)
{
  int rc = BCM_E_NONE;
  uint32 tmp_old_mapping_profile;
  int tmp_is_last;

  BCMDNX_INIT_FUNC_DEFS;
  rc = _bcm_dnx_am_template_ingress_flow_tc_mapping_get(unit, core_id, dest_ndx, &tmp_old_mapping_profile);
  BCMDNX_IF_ERR_EXIT(rc);

  rc = dnx_am_template_exchange(unit, core_id, dnx_am_template_ingress_flow_tc_mapping, 0, tc_mapping,tmp_old_mapping_profile,&tmp_is_last,new_mapping_profile,is_allocated);
  BCMDNX_IF_ERR_EXIT(rc);

  if (old_mapping_profile != NULL) {
    *old_mapping_profile = tmp_old_mapping_profile;
  }
  if (is_last != NULL) {
    *is_last = tmp_is_last;
  }

  BCMDNX_IF_ERR_EXIT(rc);
exit:
  BCMDNX_FUNC_RETURN;
}

/* Ingress Flow TC Mapping - End */

/* Ingress UC TC Mapping - Start */
static int
_bcm_dnx_am_template_ingress_uc_tc_mapping_get(int unit, int core_id, int dest_ndx ,uint32 *profile_ndx)
{
  bcm_error_t rc = BCM_E_NONE;
  uint32 dnx_sand_rv;

  BCMDNX_INIT_FUNC_DEFS;
  BCMDNX_NULL_CHECK(profile_ndx);

  dnx_sand_rv = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ipq_tc_profile_get,(unit, core_id, 0, dest_ndx , profile_ndx)));    
  DNX_BCM_SAND_IF_ERR_EXIT(dnx_sand_rv);

  BCMDNX_IF_ERR_EXIT(rc);
exit:
  BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_ingress_uc_tc_mapping_data_get(int unit, int core_id, int dest_ndx, DNX_TMC_ITM_TC_MAPPING *tc_mapping)
{
  uint32 profile_ndx;
  int rc = BCM_E_NONE;
  int core = (core_id == BCM_CORE_ALL) ? BCM_DNX_AM_DEFAULT_POOL_IDX : core_id;
  BCMDNX_INIT_FUNC_DEFS;
  rc = _bcm_dnx_am_template_ingress_uc_tc_mapping_get(unit, core, dest_ndx, &profile_ndx);
  BCMDNX_IF_ERR_EXIT(rc);

  rc = dnx_am_template_data_get(unit, core, dnx_am_template_ingress_uc_tc_mapping, profile_ndx, tc_mapping);
  BCMDNX_IF_ERR_EXIT(rc);

exit:
  BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_ingress_uc_tc_mapping_init(int unit, int template_init_id,const DNX_TMC_ITM_TC_MAPPING* tc_mapping)
{
  /*int index, is_allocated, result = BCM_E_NONE;*/
  int is_allocated, result = BCM_E_NONE;
  uint32 flags = 0;
  int template;
  int core_id;
  BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif 

  /*flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;*/
  flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
  template = template_init_id;

  /* Add all entities to template init id with given data */
  SOC_DNX_ASSYMETRIC_CORES_ITER(BCM_CORE_ALL, core_id) {
      result = dnx_am_template_allocate_group(unit, core_id, dnx_am_template_ingress_uc_tc_mapping, flags, tc_mapping, _DNX_AM_TEMPLATE_INGRESS_UC_TC_MAPPING_MAX_ENTITIES(unit), &is_allocated,&template);
      BCMDNX_IF_ERR_EXIT(result);
  }
  
exit:
  BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_ingress_uc_tc_mapping_exchange(int unit, int core_id, int dest_ndx, DNX_TMC_ITM_TC_MAPPING *tc_mapping, uint32 *old_mapping_profile, int *is_last, int *new_mapping_profile,int *is_allocated)
{
  int rc = BCM_E_NONE;
  uint32 tmp_old_mapping_profile;
  int tmp_is_last;

  BCMDNX_INIT_FUNC_DEFS;
  rc = _bcm_dnx_am_template_ingress_uc_tc_mapping_get(unit, core_id, dest_ndx, &tmp_old_mapping_profile);
  BCMDNX_IF_ERR_EXIT(rc);

  rc = dnx_am_template_exchange(unit, core_id, dnx_am_template_ingress_uc_tc_mapping, 0, tc_mapping, tmp_old_mapping_profile, &tmp_is_last, new_mapping_profile, is_allocated);
  BCMDNX_IF_ERR_EXIT(rc);

  if (old_mapping_profile != NULL) {
    *old_mapping_profile = tmp_old_mapping_profile;
  }
  if (is_last != NULL) {
    *is_last = tmp_is_last;
  }

exit:
  BCMDNX_FUNC_RETURN;
}

/* Ingress UC TC Mapping - End */

/* Flow Control Generic PFC Mapping - Start */

int _bcm_dnx_am_template_fc_generic_pfc_mapping_init(int unit, int priority, int template_id, DNX_TMC_FC_PFC_GENERIC_BITMAP *generic_bm)
{
    int rc = BCM_E_NONE;
    int is_allocated;
    int template_pool;
    
    BCMDNX_INIT_FUNC_DEFS;

#ifdef  BCM_JER2_JERICHO_SUPPORT
    if (SOC_IS_JERICHO(unit)) {
        template_pool = dnx_am_template_fc_generic_pfc_mapping_c0 + priority;
    }
    else 
#endif
    {
        template_pool = dnx_am_template_fc_generic_pfc_mapping;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif

    /* -1 to indicate use dummy profile, good for driver init */
    template_id = (template_id != -1) ? template_id : (_DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_COUNT -1);

    rc = dnx_am_template_allocate_group(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, template_pool, DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID, 
                                       generic_bm, _DNX_AM_TEMPLATE_FC_PFC_GENERIC_BITMAP_MAPPING_MAX_ENTITIES, &is_allocated, &template_id);

    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_fc_generic_pfc_mapping_data_get(int unit, int priority, int generic_bm_id, DNX_TMC_FC_PFC_GENERIC_BITMAP *generic_bm)
{
    int rc = BCM_E_NONE;
    int template_pool;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef  BCM_JER2_JERICHO_SUPPORT
    if (SOC_IS_JERICHO(unit)) {
        template_pool = dnx_am_template_fc_generic_pfc_mapping_c0 + priority;
    }
    else 
#endif
    {
        template_pool = dnx_am_template_fc_generic_pfc_mapping;
    }

    rc = dnx_am_template_data_get(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, template_pool, generic_bm_id,generic_bm);
    BCMDNX_IF_ERR_EXIT(rc);
  
exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_fc_generic_pfc_mapping_exchange(int unit, int priority, int old_generic_bm_id, DNX_TMC_FC_PFC_GENERIC_BITMAP *new_generic_bm, int *is_last, int *new_mapping_profile,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int flags;
    int tmp_is_last;
    int tmp_old_generic_bm_id = 0;
    int template_pool;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef  BCM_JER2_JERICHO_SUPPORT
    if (SOC_IS_JERICHO(unit)) {
        template_pool = dnx_am_template_fc_generic_pfc_mapping_c0 + priority;
    }
    else 
#endif
    {
        template_pool = dnx_am_template_fc_generic_pfc_mapping;
    }

    flags = 0;
    tmp_old_generic_bm_id = old_generic_bm_id;

    rc = dnx_am_template_exchange(unit, BCM_DNX_AM_DEFAULT_POOL_IDX, template_pool, flags, new_generic_bm,
                              tmp_old_generic_bm_id, &tmp_is_last, new_mapping_profile, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

exit:
    BCMDNX_FUNC_RETURN;
}
/* Flow Control Generic PFC Mapping - End */


 /* DESCRIPTION:  init template for packet size compensation feature for CRPS use */
int _bcm_dnx_am_template_stat_interface_pktSize_compensation_init(int unit, int core ,_dnx_am_template_t template_id, int delta, int template)
{
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif

    result = dnx_am_template_allocate_group(unit, core, template_id, flags, &delta, MAX_PORTS_IN_CORE*2, &is_allocated, &template);
    BCMDNX_IF_ERR_EXIT(result);

exit:
    BCMDNX_FUNC_RETURN;
}

/* DESCRIPTION:  exchange template for packet size compensation feature for CRPS use */
int _bcm_dnx_am_template_crps_pktSize_compensation_exchange(int unit, int core, _dnx_am_template_t template_id, int port, int data, int old_template, int *is_last, int *template,int *is_allocated)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_exchange(unit, core, template_id, 0, &data, old_template, is_last, template, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);
    if(*template < 0)
    {
        BCMDNX_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BSL_BCM_MSG("template is negative")));
    }
exit:
    BCMDNX_FUNC_RETURN;
}
/* DESCRIPTION:  exchange template for packet size compensation feature for statistics interface use */
int _bcm_dnx_am_template_stat_interface_pktSize_compensation_exchange(int unit, int core, _dnx_am_template_t template_id, int port, int data, int old_template, int *is_last, int *template,int *is_allocated)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    rc = dnx_am_template_exchange(unit, core, template_id, 0, &data, old_template, is_last, template, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);
    if(*template < 0)
    {
        BCMDNX_ERR_EXIT_MSG(BCM_E_INTERNAL, (_BSL_BCM_MSG("template is negative")));
    }
exit:
    BCMDNX_FUNC_RETURN;
}

/* Cosq scheduler adjust size (compensation) final delta mapping - Start */
int _bcm_dnx_am_template_scheduler_adjust_size_final_delta_mapping_init(int unit, int template_init_id, int *final_delta)
{
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = 0;
    int template;
    int core;

    BCMDNX_INIT_FUNC_DEFS;
    flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;
    template = template_init_id;
    
    /* Add all entities to template init id with given data */
    BCM_DNX_CORES_ITER(BCM_CORE_ALL, core) {
        result = dnx_am_template_allocate_group(unit, core, dnx_am_template_scheduler_adjust_size_final_delta, flags, final_delta,_DNX_AM_TEMPLATE_SCHEDULER_ADJUST_SIZE_FINAL_DELTA_MAX_ENTITIES, &is_allocated,&template);
        BCMDNX_IF_ERR_EXIT(result);
    }

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_scheduler_adjust_size_final_delta_mapping_data_get(int unit, int core, int delta , int *final_delta)
{
    int index;
    int rc = BCM_E_NONE;
    int core_index;

    BCMDNX_INIT_FUNC_DEFS;

    core_index = (core == BCM_CORE_ALL) ? 0 : core;
    rc = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_itm_sch_final_delta_map_get,(unit, core_index, delta, (uint32*)&index));
    BCMDNX_IF_ERR_EXIT(rc);

    if (index < 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Failed to get the old profile")));
    }

    rc = dnx_am_template_data_get(unit, core_index, dnx_am_template_scheduler_adjust_size_final_delta, index, final_delta);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

int _bcm_dnx_am_template_scheduler_adjust_size_final_delta_mapping_exchange(int unit, int core, int delta, int *final_delta, int *old_profile, int *is_last, int *new_profile,int *is_allocated)
{
    int rc = BCM_E_NONE;
    int tmp_old_profile;
    int core_index;
    int tmp_is_last;

    BCMDNX_INIT_FUNC_DEFS;

    core_index = (core == BCM_CORE_ALL) ? 0 : core;
    rc = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_itm_sch_final_delta_map_get,(unit, core_index, delta, (uint32*)&tmp_old_profile));
    BCMDNX_IF_ERR_EXIT(rc);

    rc = dnx_am_template_exchange(unit, core_index, dnx_am_template_scheduler_adjust_size_final_delta, 0, final_delta, tmp_old_profile, &tmp_is_last, new_profile, is_allocated);
    BCMDNX_IF_ERR_EXIT(rc);

    if (old_profile != NULL) {
        *old_profile = tmp_old_profile;
    }
    if (is_last != NULL) {
        *is_last = tmp_is_last;
    }

exit:
    BCMDNX_FUNC_RETURN;
}
/* Cosq scheduler adjust size (compensation) final delta mapping - End */

/* 
 * Template management - End
 */ 

 /* DESCRIPTION:  init template for packet size compensation feature for CRPS use */
int _bcm_dnx_am_template_crps_pktSize_compensation_init(int unit, int core ,_dnx_am_template_t template_id, int delta, int template)
{
    int is_allocated, result = BCM_E_NONE;
    uint32 flags = DNX_SHR_TEMPLATE_MANAGE_SET_WITH_ID;

    BCMDNX_INIT_FUNC_DEFS;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* In warmboot we already restored entries, no need to init */
    if (SOC_WARM_BOOT(unit)) BCM_EXIT;
#endif

    result = dnx_am_template_allocate_group(unit, core, template_id, flags, &delta, MAX_PORTS_IN_CORE*2, &is_allocated, &template);
    BCMDNX_IF_ERR_EXIT(result);

exit:
    BCMDNX_FUNC_RETURN;
}





