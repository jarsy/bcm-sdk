/*
 * $Id: starfighter_service.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _STARFIGHTER3_SERVICE_H
#define _STARFIGHTER3_SERVICE_H

int drv_gex_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val);
int drv_gex_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val);

int drv_gex_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val);

int drv_starfighter3_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_starfighter3_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new);

int drv_gex_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_gex_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);

int drv_gex_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_gex_security_egress_get(int unit, int port, int *enable);

int drv_sf3_arl_learn_count_set(int unit, uint32 port, uint32 type, int value);
int drv_sf3_arl_learn_count_get(int unit, uint32 port, uint32 type, int *value);

int drv_sf3_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter);
int drv_sf3_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter);
int drv_sf3_cfp_meter_rate_transform(int unit, uint32 kbits_sec, uint32 kbits_burst,
    uint32 *bucket_size, uint32 *ref_cnt, uint32 *ref_unit);

int drv_starfighter3_dev_prop_get(int unit,uint32 prop_type,uint32 *prop_val);
int drv_starfighter3_dev_prop_set(int unit,uint32 prop_type,uint32 prop_val);

int drv_sf3_fp_policer_control(int unit,  int stage_id, int op, void *entry,
    drv_policer_config_t *policer_cfg);
int drv_sf3_fp_stat_type_get(int unit, int stage_id, drv_policer_mode_t policer_mode,
    drv_field_stat_t stat, int *type1, int *type2, int *type3);
int drv_sf3_fp_stat_support_check(int unit, int stage_id, int op, int param0, void *mode);
int drv_sf3_fp_action_conflict(int unit, int stage_id,
    drv_field_action_t act1, drv_field_action_t act2);

int drv_sf3_rate_config_set(int unit, soc_pbmp_t pbmp, uint32 config_type, uint32 value);
int drv_sf3_rate_config_get(int unit, uint32 port, uint32 config_type, uint32 *value);

int drv_sf3_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, uint32 flags,
    uint32 kbits_sec_min, uint32 kbits_sec_max, uint32 burst_size);
int drv_sf3_rate_get(int unit, uint32 port, uint8 queue_n, int direction, uint32 *flags,
    uint32 *kbits_sec_min, uint32 *kbits_sec_max, uint32 *burst_size);

int drv_sf3_storm_control_enable_set(int unit, uint32 port, uint8 enable);
int drv_sf3_storm_control_enable_get(int unit, uint32 port, uint8 *enable);

int drv_sf3_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type,
                uint32 limit, uint32 burst_size);
int drv_sf3_storm_control_get(int unit, uint32 port, uint32 *type,
                uint32 *limit, uint32 *burst_size);
int drv_sf3_queue_prio_get(int unit, uint32 port, uint8 prio, uint8 *queue_n);
int drv_sf3_queue_prio_set(int unit, uint32 port, uint8 prio, uint8 queue_n);
int drv_sf3_queue_port_prio_to_queue_get(int unit, uint8 port, 
        uint8 prio, uint8 *queue_n);
int drv_sf3_queue_port_prio_to_queue_set(int unit, uint8 port, 
        uint8 prio, uint8 queue_n);

int drv_sf3_queue_prio_remap_get(int unit, uint32 port, 
        uint8 pre_prio, uint8 *prio);
int drv_sf3_queue_prio_remap_set(int unit, uint32 port, 
        uint8 pre_prio, uint8 prio);
int drv_sf3_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flags, 
        uint32 mode);
int drv_sf3_queue_mode_get(int unit, uint32 port, uint32 flags, 
        uint32 *mode);
int drv_sf3_queue_WRR_weight_set(int unit, uint32 port_type, 
        soc_pbmp_t bmp, uint8 queue, uint32 weight);
int drv_sf3_queue_WRR_weight_get(int unit, uint32 port_type, 
        uint32 port, uint8 queue, uint32 *weight);
int drv_sf3_queue_qos_control_set(int unit, uint32 port, 
        uint32 type, uint32 state);
int drv_sf3_queue_qos_control_get(int unit, uint32 port, 
        uint32 type, uint32 *state);

int drv_sf3_wred_init(int unit);
int drv_sf3_wred_config_create(int unit, uint32 flags, 
                                drv_wred_config_t *config, int *wred_idx);
int drv_sf3_wred_config_set(int unit, int wred_id, 
                                            drv_wred_config_t *config);
int drv_sf3_wred_config_get(int unit, int wred_id, 
                                            drv_wred_config_t *config);
int drv_sf3_wred_config_destroy(int unit, int wred_id);
int drv_sf3_wred_map_attach(int unit, int wred_id, 
                                            drv_wred_map_info_t *map);
int drv_sf3_wred_map_deattach(int unit, int wred_id, 
                                            drv_wred_map_info_t *map);
int drv_sf3_wred_map_get(int unit, int *wred_id, 
                                            drv_wred_map_info_t *map);

int drv_sf3_wred_control_set(int unit, soc_port_t port, int queue, 
            drv_wred_control_t type, uint32 value);
int drv_sf3_wred_control_get(int unit, soc_port_t port, int queue, 
            drv_wred_control_t type, uint32 *value);
int drv_sf3_wred_counter_enable_set(int unit, soc_port_t port, int queue,
    drv_wred_counter_t type, int enable);
int drv_sf3_wred_counter_enable_get(int unit, soc_port_t port, int queue,
    drv_wred_counter_t type, int *enable);
int drv_sf3_wred_counter_set(int unit, soc_port_t port, int queue,
        drv_wred_counter_t type, uint64 value);
int drv_sf3_wred_counter_get(int unit, soc_port_t port, int queue,
        drv_wred_counter_t type, uint64 *value);

extern drv_if_t drv_starfighter3_services;

#endif
