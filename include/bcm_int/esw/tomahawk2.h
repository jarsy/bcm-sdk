/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        tomahawk2.h
 * Purpose:     Function declarations for Tomahawk2 Internal functions.
 */

#ifndef _BCM_INT_TOMAHAWK2_H_
#define _BCM_INT_TOMAHAWK2_H_

#if defined(BCM_TOMAHAWK2_SUPPORT)

#include <bcm/types.h>
#include <bcm/l3.h>
#include <soc/defs.h>
#include <soc/esw/port.h>
#include <bcm/failover.h>
#include <bcm/cosq.h>
#include <bcm/pstats.h>

#if defined(INCLUDE_L3)

/* L3 ECMP DLB */
extern void _bcm_th2_ecmp_dlb_deinit(int unit);
extern int _bcm_th2_ecmp_dlb_init(int unit);
extern int bcm_th2_l3_egress_dlb_attr_set(int unit,
                                          int nh_index,
                                          bcm_l3_egress_t *egr,
                                          bcm_l3_egress_t *old_egr);
extern int bcm_th2_l3_egress_dlb_attr_destroy(int unit,
                                              int nh_index,
                                              bcm_l3_egress_t *egr);
extern int bcm_th2_l3_egress_dlb_attr_get(int unit,
                                          int nh_index,
                                          bcm_l3_egress_t *egr);
extern int bcm_th2_l3_egress_ecmp_dlb_create(int unit,
                                             bcm_l3_egress_ecmp_t *ecmp,
                                             int intf_count,
                                             bcm_if_t *intf_array,
                                             int primary_count);
extern int bcm_th2_l3_egress_ecmp_dlb_destroy(int unit, bcm_if_t mpintf);
extern int bcm_th2_l3_egress_ecmp_dlb_get(int unit, bcm_l3_egress_ecmp_t *ecmp);
extern int _bcm_th2_ecmp_dlb_config_set(int unit,
                                        bcm_switch_control_t type,
                                        int arg);
extern int _bcm_th2_ecmp_dlb_config_get(int unit,
                                        bcm_switch_control_t type,
                                        int *arg);
extern int bcm_th2_l3_egress_ecmp_member_status_set(int unit,
                                                    bcm_if_t intf,
                                                    int status);
extern int bcm_th2_l3_egress_ecmp_member_status_get(int unit,
                                                    bcm_if_t intf,
                                                    int *status);
extern int bcm_th2_l3_egress_ecmp_dlb_ethertype_set(int unit,
                                                    uint32 flags,
                                                    int ethertype_count,
                                                    int *ethertype_array);
extern int bcm_th2_l3_egress_ecmp_dlb_ethertype_get(int unit,
                                                    uint32 *flags,
                                                    int ethertype_max,
                                                    int *ethertype_array,
                                                    int *ethertype_count);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_th2_ecmp_dlb_wb_alloc_size_get(int unit, int *size);
extern int bcm_th2_ecmp_dlb_sync(int unit, uint8 **scache_ptr);
extern int bcm_th2_ecmp_dlb_scache_recover(int unit, uint8 **scache_ptr);
extern int bcm_th2_ecmp_dlb_hw_recover(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifndef BCM_SW_STATE_DUMP_DISABLE
extern void bcm_th2_ecmp_dlb_sw_dump(int unit);
extern void bcm_th2_dgm_sw_dump(int unit);
#endif /* ifndef BCM_SW_STATE_DUMP_DISABLE */

/* Number of protection switching groups
 * represented per entry in the PROT_SW table TX_INITIAL_PROT_GROUP_TABLE
 */
#define BCM_TH2_FAILOVER_PROT_GROUPS_PER_ENTRY 128

extern int  bcm_th2_failover_init(int unit);
extern void bcm_th2_failover_unlock(int unit);
extern int  bcm_th2_failover_lock(int unit);
extern int  bcm_th2_failover_cleanup(int unit);
extern int  bcm_th2_failover_create(int unit, uint32 flags,
                                    bcm_failover_t *failover_id);
extern int  bcm_th2_failover_destroy(int unit, bcm_failover_t  failover_id);
extern int  bcm_th2_failover_status_set(int unit,
                bcm_failover_element_t *failover, int value);
extern int  bcm_th2_failover_status_get(int unit,
                bcm_failover_element_t *failover, int  *value);

#endif /* INCLUDE_L3 */

extern int bcmi_th2_port_fn_drv_init(int unit);
extern int bcmi_th2_port_soc_resource_init(int unit, int nport,
                                 bcm_port_resource_t *resource,
                                 soc_port_resource_t *soc_resource);
extern int bcmi_th2_port_resource_validate(int unit,
                            soc_port_schedule_state_t *port_schedule_state);
extern int bcmi_th2_pre_flexport_tdm(int unit,
                            soc_port_schedule_state_t *port_schedule_state);
extern int bcmi_th2_post_flexport_tdm(int unit,
                            soc_port_schedule_state_t *port_schedule_state);
extern int bcmi_th2_clport_update(int unit,
                            soc_port_schedule_state_t *port_schedule_state);
extern int bcmi_th2_port_attach_mmu(int unit, int port);
extern int bcmi_th2_port_detach_asf(int unit, bcm_port_t port);
extern int bcm_th2_ipmc_repl_port_attach(int unit, bcm_port_t port);
extern int bcm_th2_ipmc_repl_port_detach(int unit, bcm_port_t port);
extern int bcmi_th2_port_enable(int unit,
                             soc_port_schedule_state_t *port_schedule_state);
extern int bcmi_th2_port_disable(int unit,
                             soc_port_schedule_state_t *port_schedule_state);
extern int bcmi_th2_port_speed_1g_update(int unit, bcm_port_t port, int speed);
extern int bcmi_th2_speed_ability_get(int unit, bcm_port_t port,
                                      bcm_port_abil_t *mask);
extern int bcmi_th2_tsc_xgxs_reset(int unit, pbmp_t pbmp);

#if defined (INCLUDE_TCB) && defined (BCM_TCB_SUPPORT)
extern int bcm_th2_cosq_tcb_init(int unit);
extern int _bcm_th2_cosq_tcb_deinit(int unit);
#ifndef BCM_SW_STATE_DUMP_DISABLE
extern void bcm_th2_cosq_tcb_sw_dump(int unit);
#endif /* BCM_SW_STATE_DUMP_DISABLE */
extern int bcm_th2_cosq_tcb_config_set(int unit, bcm_cosq_buffer_id_t buffer_id, 
                                  bcm_cosq_tcb_config_t *config);
extern int bcm_th2_cosq_tcb_config_get(int unit, bcm_cosq_buffer_id_t buffer_id, 
                                  bcm_cosq_tcb_config_t *config);
extern int bcm_th2_cosq_tcb_gport_threshold_mapping_set(int unit, 
                                               bcm_gport_t port, 
                                               bcm_cos_queue_t cosq, 
                                               bcm_cosq_buffer_id_t buffer_id, 
                                               int index);
extern int bcm_th2_cosq_tcb_gport_threshold_mapping_get(int unit,
                                               bcm_gport_t port, 
                                               bcm_cos_queue_t cosq, 
                                               bcm_cosq_buffer_id_t buffer_id, 
                                               int *index);
extern int bcm_th2_cosq_tcb_threshold_profile_create(int unit, int flags,
                                            bcm_cosq_buffer_id_t buffer_id, 
                                            bcm_cosq_tcb_threshold_profile_t *threshold,
                                            int *index);
extern int bcm_th2_cosq_tcb_threshold_profile_get(int unit, 
                                         bcm_cosq_buffer_id_t buffer_id, int index,
                                         bcm_cosq_tcb_threshold_profile_t *threshold);
extern int bcm_th2_cosq_tcb_threshold_profile_destroy(int unit, 
                                         bcm_cosq_buffer_id_t buffer_id, int index);
extern int bcm_th2_cosq_tcb_buffer_multi_get(int unit, 
                                  bcm_cosq_buffer_id_t buffer_id, int count, 
                                  bcm_cosq_tcb_buffer_t *entries, int *num);
extern int bcm_th2_cosq_tcb_event_multi_get(int unit, 
                                 bcm_cosq_buffer_id_t buffer_id, int count, 
                                 bcm_cosq_tcb_event_t *entries, int *num, 
                                 int *overflow_count);
extern int bcm_th2_cosq_tcb_control_get(int unit, bcm_cosq_buffer_id_t buffer_id, 
                                        bcm_cosq_tcb_control_t type, int *arg);
extern int bcm_th2_cosq_tcb_enable(int unit, bcm_cosq_buffer_id_t buffer_id, 
                                          int enable);
extern int bcm_th2_cosq_tcb_freeze(int unit, bcm_cosq_buffer_id_t buffer_id, 
                                          int enable);
extern int bcm_th2_cosq_tcb_current_phase_get(int unit, 
                                   bcm_cosq_buffer_id_t buffer_id, int *status);
#ifdef BCM_WARM_BOOT_SUPPORT
extern int bcm_th2_cosq_tcb_reinit(int unit);
#endif
#endif /* INCLUDE_TCB && BCM_TCB_SUPPORT */

extern void bcm_th2_pstats_deinit(int unit);
extern int bcm_th2_pstats_init(int unit);

#endif /* BCM_TOMAHAWK2_SUPPORT  */

#endif /* !_BCM_INT_TOMAHAWK2_H_ */
