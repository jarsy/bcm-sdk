/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$Id: state.h,v 1.13 Broadcom SDK $
 * All Rights Reserved.$
 *
 * File:        state.h
 * Purpose:     Single point of reference for all BCM state of a unit.
 */

#ifndef	_BCM_INT_SBX_STATE_H_
#define	_BCM_INT_SBX_STATE_H_
#ifdef BCM_WARM_BOOT_SUPPORT
#include "soc/types.h"
#endif /* BCM_WARM_BOOT_SUPPORT */

typedef struct bcm_sbx_state_s {
    struct bcm_sbx_cosq_discard_state_s *discard_state;
    struct bcm_sbx_fabric_state_s *fabric_state;
    struct bcm_sbx_stack_state_s *stack_state;
    struct bcm_sbx_cosq_queue_state_s *queue_state;       /* pointer to array of queue_state_t */
    struct bcm_sbx_cosq_bw_group_state_s *bw_group_state; /* pointer to array of bw_group_state_t */
    struct bcm_sbx_cosq_sysport_state_s *sysport_state;   /* pointer to array of sysport_state_t */
    struct bcm_sbx_cosq_destport_state_s *destport_state; /* pointer to array of destport_state_t */
    struct bcm_sbx_cosq_sysport_group_state_s *sysport_group_state; /* pointer to array of sysport_group_state_t */
    struct bcm_sbx_cosq_ingress_scheduler_state_s *ingress_scheduler_state; /* pointer to array of ingress_scheduler_state_t */
    struct bcm_sbx_cosq_egress_scheduler_state_s *egress_scheduler_state; /* pointer to array of egress_scheduler_state_t */
    struct bcm_sbx_cosq_egress_group_state_s *egress_group_state; /* pointer to array of egress_group_state_t */
    struct bcm_sbx_cosq_ingress_multipath_state_s *ingress_multipath_state; /* pointer to array of ingress_multipath_state_t */
    struct bcm_sbx_cosq_egress_multipath_state_s *egress_multipath_state; /* pointer to array of egress_multipath_state_t */
    struct bcm_sbx_failover_object_s *failover_state;     /* pointer to array of failover_state_t */
    struct bcm_sbx_port_state_s *port_state;              /* pointer to array of port_state_t */
    struct bcm_sbx_cosq_fcd_state_s *fcd_state;           /* pointer to array of fcd_state_t */
#ifdef BCM_WARM_BOOT_SUPPORT
    uint8 *cosq_index;                                    /* current location in scache */
    uint32 cache_cosq; /* if set cache cosq changes (do not commit changes to hardware) */
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_EASY_RELOAD_SUPPORT
    struct bcm_sbx_cosq_bw_group_state_cache_s *bw_group_state_cache; /* pointer to array of bw_group_state_cache_t */
#endif /* BCM_EASY_RELOAD_SUPPORT */
    uint32 l2_age_delete; /* if set, l2_age will delete MAC entries */
    uint32 cache_l2; /* if set cache l2 changes (do not commit changes to hardware) */
    uint32 cache_ipmc; /* if set cache ipmc changes (do not commit changes to hardware) */
    uint32 cache_l3host; /* if set cache l3host changes (do not commit changes to hardware) */
    uint32 cache_l3route; /* if set cache l3route changes (do not commit changes to hardware) */
    uint32 cache_field; /* if set cache field changes (do not commit changes to hardware) */
} bcm_sbx_state_t;

#endif	/* _BCM_INT_SBX_STATE_H_ */
