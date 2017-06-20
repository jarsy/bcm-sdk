/*
 * $Id: harrier_service.h,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _HARRIER_SERVICE_H
#define _HARRIER_SERVICE_H

int drv_harrier_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
    uint8* mac, uint32 bpdu_idx);
int drv_harrier_mac_get(int unit, uint32 port, uint32 mac_type, 
    soc_pbmp_t *bmp, uint8* mac);
int drv_harrier_arl_learn_enable_set(int unit, soc_pbmp_t pbmp, uint32 mode);
int drv_harrier_arl_learn_enable_get(int unit, soc_port_t port, uint32 *mode);
int drv_harrier_arl_learn_count_set(int unit, uint32 port, uint32 type, 
    int value);
int drv_harrier_arl_learn_count_get(int unit, uint32 port, uint32 type, 
    int *value);


int drv_harrier_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op);
int drv_harrier_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op);
int drv_harrier_trunk_hash_field_add(int unit, uint32 field_type);
int drv_harrier_trunk_hash_field_remove(int unit, uint32 field_type);

int drv_harrier_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode);
int drv_harrier_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode);
int drv_harrier_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_harrier_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_harrier_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight);
int drv_harrier_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight);
int drv_harrier_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n);
int drv_harrier_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n);
int drv_harrier_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n);
int drv_harrier_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n);
int drv_harrier_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state);
int drv_harrier_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state);
int drv_harrier_queue_rx_reason_set
    (int unit, uint8 reason, uint32 queue);
int drv_harrier_queue_rx_reason_get
    (int unit, uint8 reason, uint32 *queue);

int drv_harrier_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_harrier_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp);

int drv_harrier_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask);
int drv_harrier_trap_get(int unit, soc_port_t port, uint32 *trap_mask);
int drv_harrier_snoop_set(int unit, uint32 snoop_mask);
int drv_harrier_snoop_get(int unit, uint32 *snoop_mask);

int drv_harrier_rate_config_set(int unit, soc_pbmp_t pbmp, 
    uint32 config_type, uint32 value);
int drv_harrier_rate_config_get(int unit, uint32 port, 
    uint32 config_type, uint32 *value);
int drv_harrier_rate_set
    (int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size);
int drv_harrier_rate_get
    (int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flags, uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, uint32 *burst_size);

int drv_harrier_storm_control_enable_set
    (int unit, uint32 port, uint8 enable);
int drv_harrier_storm_control_enable_get
    (int unit, uint32 port, uint8 *enable);
int drv_harrier_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_harrier_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size);

int drv_harrier_igmp_mld_snoop_mode_get(int unit, int type, int *mode);
int drv_harrier_igmp_mld_snoop_mode_set(int unit, int type, int mode);

int drv_harrier_port_set(int unit, soc_pbmp_t bmp, uint32 prop_type, uint32 prop_val);
int drv_harrier_port_get(int unit, int port, uint32 prop_type, uint32 *prop_val);

int drv_harrier_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val);

int drv_harrier_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_harrier_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new);

int drv_harrier_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_harrier_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);

int drv_harrier_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_harrier_security_egress_get(int unit, int port, int *enable);

int drv_harrier_led_mode_get(int unit, int port, uint32 *led_mode);
int drv_harrier_led_mode_set(int unit, int port, uint32 led_mode);

int drv_harrier_led_funcgrp_select_get(int unit, int port, int *led_group);
int drv_harrier_led_funcgrp_select_set(int unit, int port, int led_group);

int drv_harrier_port_vlan_set(int unit,uint32 port,soc_pbmp_t bmp);
int drv_harrier_port_vlan_get(int unit,uint32 port,soc_pbmp_t *bmp);

int drv_harrier_vlan_vt_add(int unit, uint32 vt_type, uint32 port,  uint32 cvid, 
                uint32 sp_vid, uint32 pri, uint32 mode);
int drv_harrier_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid);
int drv_harrier_vlan_vt_delete_all(int unit, uint32 vt_type);
int drv_harrier_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                uint32 port, uint32 prop_val);
int drv_harrier_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                uint32 port, uint32 *prop_val);

int drv_harrier_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_harrier_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_harrier_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val);
int drv_harrier_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val);

int drv_harrier_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state); 
int drv_harrier_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state);               

int drv_harrier_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag);
int drv_harrier_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp);

int drv_harrier_mem_length_get(int unit, uint32 mem, uint32 *data);
int drv_harrier_mem_width_get(int unit, uint32 mem, uint32 *data);
int drv_harrier_mem_read(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_harrier_mem_write(int unit, uint32 mem, uint32 entry_id, 
                                                    uint32 count, uint32 *entry);
int drv_harrier_mem_field_get(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_harrier_mem_field_set(int unit, uint32 mem, uint32 field_index, 
                                                    uint32 *entry, uint32 *fld_data);
int drv_harrier_mem_clear(int unit, uint32 mem);
int drv_harrier_mem_search(int unit, uint32 mem, uint32 *key, 
                                                    uint32 *entry, uint32 *entry_1, uint32 flags);
int drv_harrier_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_harrier_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);

/* CFP */
int drv_harrier_cfp_init(int unit);
int drv_harrier_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param);
int drv_harrier_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);
int drv_harrier_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2);
int drv_harrier_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2);
int drv_harrier_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_harrier_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry);
int drv_harrier_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_harrier_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_harrier_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_harrier_cfp_meter_rate_transform(int unit, uint32 kbits_sec, 
   uint32 kbits_burst, uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit);
int drv_harrier_cfp_meter_get(int unit, drv_cfp_entry_t* entry, 
    uint32 *kbits_sec, uint32 *kbits_burst);
int drv_harrier_cfp_meter_set(int unit, drv_cfp_entry_t* entry, 
    uint32 kbits_sec, uint32 kbits_burst);
int drv_harrier_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, 
    uint32 *slice_id, uint32 flags);
int drv_harrier_cfp_slice_to_qset(int unit, uint32 slice_id, 
    drv_cfp_entry_t *entry);
int drv_harrier_cfp_stat_get(int unit, uint32 stat_type, uint32 index, 
    uint32* counter);
int drv_harrier_cfp_stat_set(int unit, uint32 stat_type, uint32 index, 
    uint32 counter);
int drv_harrier_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base);
int drv_harrier_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);
int drv_harrier_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max);
int drv_harrier_cfp_range_set(int unit, uint32 type, uint32 id, 
    uint32 param1, uint32 param2);
int drv_harrier_cfp_range_get(int unit, uint32 type, uint32 id, 
    uint32 *param1, uint32 *param2);

/* FP */
int drv_harrier_fp_init(int unit,int stage_id);
int drv_harrier_fp_deinit(int unit,int stage_id);
int drv_harrier_fp_qual_value_set(int unit,int stage_id,
    drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_harrier_fp_qual_value_get(int unit,int stage_id,
    drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_harrier_fp_udf_value_set(int unit,int stage_id,uint32 udf_idex,
    void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_harrier_fp_udf_value_get(int unit,int stage_id,uint32 udf_idex,
    void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_harrier_fp_entry_mem_control(int unit,int stage_id,int op,
    void *src_entry,void *dst_entry,void **alloc_entry);
int drv_harrier_fp_entry_tcam_control(int unit,int stage_id,
    void *drv_entry,int op,int param1,void *param2);
int drv_harrier_fp_action_conflict(int unit,int stage_id,
    drv_field_action_t act1,drv_field_action_t act2);
int drv_harrier_fp_action_support_check(int unit,int stage_id,
    drv_field_action_t action);
int drv_harrier_fp_action_add(int unit,int stage_id,void *drv_entry,
    drv_field_action_t action,uint32 param0,uint32 param1);
int drv_harrier_fp_action_remove(int unit,int stage_id,void *drv_entry,
    drv_field_action_t action,uint32 param0,uint32 param1);
int drv_harrier_fp_selcode_mode_get(int unit,int stage_id,void *qset,
    int mode,int8 *slice_id,uint32 *slice_map,void **drv_entry);
int drv_harrier_fp_selcode_to_qset(int unit,int stage_id,int slice_id,
    void *qset);
int drv_harrier_fp_qualify_support(int unit,int stage_id,void *qset);
int drv_harrier_fp_id_control(int unit,int type,int op,int flags,
    int *id,uint32 *count);
int drv_harrier_fp_policer_control(int unit,int stage_id,int op,
    void *entry,drv_policer_config_t *policer_cfg);
int drv_harrier_fp_stat_support_check(int unit,int stage_id,int op,
    int param0,void *mode);
int drv_harrier_fp_stat_type_get(int unit,int stage_id,
    drv_policer_mode_t policer_mode,drv_field_stat_t stat,int *type1,
    int *type2,int *type3);

int drv_harrier_dev_prop_get(int unit,uint32 prop_type,uint32 *prop_val);


extern drv_if_t drv_harrier_services;

#endif
