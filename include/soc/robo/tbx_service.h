/*
 * $Id: tbx_service.h,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _TBX_SERVICE_H
#define _TBX_SERVICE_H

int drv_tbx_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
    uint8* mac, uint32 bpdu_idx);
int drv_tbx_mac_get(int unit, uint32 port, uint32 mac_type, 
    soc_pbmp_t *bmp, uint8* mac);
int drv_tbx_arl_learn_enable_set(int unit, soc_pbmp_t pbmp, uint32 mode);
int drv_tbx_arl_learn_enable_get(int unit, soc_port_t port, uint32 *mode);
int drv_tbx_arl_learn_count_set(int unit, uint32 port, uint32 type, 
    int value);
int drv_tbx_arl_learn_count_get(int unit, uint32 port, uint32 type, 
    int *value);


int drv_tbx_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op);
int drv_tbx_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op);
int drv_tbx_trunk_hash_field_add(int unit, uint32 field_type);
int drv_tbx_trunk_hash_field_remove(int unit, uint32 field_type);

int drv_tbx_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode);
int drv_tbx_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode);
int drv_tbx_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_tbx_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_tbx_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight);
int drv_tbx_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight);
int drv_tbx_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n);
int drv_tbx_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n);
int drv_tbx_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state);
int drv_tbx_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state);
int drv_tbx_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio);
int drv_tbx_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio);
int drv_tbx_queue_dfsv_unmap_set(int unit, uint8 prio, uint8 dscp);
int drv_tbx_queue_dfsv_unmap_get(int unit, uint8 prio, uint8 *dscp);
int drv_tbx_queue_rx_reason_set
    (int unit, uint8 reason, uint32 queue);
int drv_tbx_queue_rx_reason_get
    (int unit, uint8 reason, uint32 *queue);
int drv_tbx_queue_port_txq_pause_set
    (int unit, uint32 port, uint8 queue_n, uint8 enable);
int drv_tbx_queue_port_txq_pause_get
    (int unit, uint32 port, uint8 queue_n, uint8 *enable);
int drv_tbx_queue_qos_control_set(int unit, uint32 port, uint32 type, uint32 state);
int drv_tbx_queue_qos_control_get(int unit, uint32 port, uint32 type, uint32 *state);

int drv_tbx_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_tbx_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp);

int drv_tbx_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask);
int drv_tbx_trap_get(int unit, soc_port_t port, uint32 *trap_mask);
int drv_tbx_snoop_set(int unit, uint32 snoop_mask);
int drv_tbx_snoop_get(int unit, uint32 *snoop_mask);

int drv_tbx_rate_config_set(int unit, soc_pbmp_t pbmp, 
    uint32 config_type, uint32 value);
int drv_tbx_rate_config_get(int unit, uint32 port, 
    uint32 config_type, uint32 *value);
int drv_tbx_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size);
int drv_tbx_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flasg, uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, uint32 *burst_size);

int drv_tbx_storm_control_enable_set
    (int unit, uint32 port, uint8 enable);
int drv_tbx_storm_control_enable_get
    (int unit, uint32 port, uint8 *enable);
int drv_tbx_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_tbx_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size);

int drv_tbx_igmp_mld_snoop_mode_get(int unit, int type, int *mode);
int drv_tbx_igmp_mld_snoop_mode_set(int unit, int type, int mode);

int drv_tbx_vlan_mode_get(int unit, uint32 *mode);
int drv_tbx_vlan_mode_set(int unit, uint32 mode);
int drv_tbx_vlan_prop_get(int unit, uint32 prop_type, uint32 * prop_val);
int drv_tbx_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_tbx_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 * val);
int drv_tbx_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val);
int drv_tbx_port_vlan_set(int unit, uint32 port, soc_pbmp_t bmp);
int drv_tbx_port_vlan_get(int unit, uint32 port, soc_pbmp_t *bmp);
int drv_tbx_port_vlan_pvid_set(int unit, uint32 port, uint32 outer_tag, uint32 inner_tag);
int drv_tbx_port_vlan_pvid_get(int unit, uint32 port, uint32 *outer_tag, uint32 *inner_tag);

int drv_tbx_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state);
int drv_tbx_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state);

int drv_tbx_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag);
int drv_tbx_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp);

int drv_tbx_mcrep_vpgrp_vport_config_set(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param);
int drv_tbx_mcrep_vpgrp_vport_config_get(int unit, uint32 mc_group,
            uint32 port, drv_mcrep_control_flag_t op, int *param);
int drv_tbx_mcrep_vport_config_set(int unit, uint32 port, 
            drv_mcrep_control_flag_t op, uint32 vport, uint32 vid);
int drv_tbx_mcrep_vport_config_get(int unit, uint32 port, 
            drv_mcrep_control_flag_t op, uint32 *vport, uint32 *vid);
int drv_tbx_mcrep_vport_vid_search(int unit, uint32 port, 
            uint32 *vport, int *param);
            
/* CFP */
int drv_tbx_cfp_init(int unit);
int drv_tbx_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param);
int drv_tbx_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);
int drv_tbx_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2);
int drv_tbx_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2);
int drv_tbx_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_tbx_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_tbx_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_tbx_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_tbx_cfp_meter_rate_transform(int unit, uint32 kbits_sec, 
  uint32 kbits_burst, uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit);
int drv_tbx_cfp_meter_get(int unit, drv_cfp_entry_t* entry, 
    uint32 *kbits_sec, uint32 *kbits_burst);
int drv_tbx_cfp_meter_set(int unit, drv_cfp_entry_t* entry, 
    uint32 kbits_sec, uint32 kbits_burst);
int drv_tbx_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, 
    uint32 *slice_id, uint32 flags);
int drv_tbx_cfp_slice_to_qset(int unit, uint32 slice_id, 
    drv_cfp_entry_t *entry);
int drv_tbx_cfp_stat_get(int unit, uint32 stat_type, uint32 index, 
    uint32* counter);
int drv_tbx_cfp_stat_set(int unit, uint32 stat_type, uint32 index, 
    uint32 counter);
int drv_tbx_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base);
int drv_tbx_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);

int drv_tbx_dos_enable_set(int unit,uint32 type,uint32 param);
int drv_tbx_dos_enable_get(int unit,uint32 type,uint32 *param);
int drv_tbx_dos_event_bitmap_get(int unit,uint32 op,uint32 *event_bitmap);

/* VM */
int drv_tbx_vm_init(int unit);
int drv_tbx_vm_deinit(int unit);
int drv_tbx_vm_field_get(int unit,uint32 mem_type,uint32 field_type,
    drv_vm_entry_t *entry,uint32 *fld_val);
int drv_tbx_vm_field_set(int unit,uint32 mem_type,uint32 field_type,
    drv_vm_entry_t *entry,uint32 *fld_val);
int drv_tbx_vm_action_get(int unit,uint32 action,drv_vm_entry_t *entry,
    uint32 *act_param);
int drv_tbx_vm_action_set(int unit,uint32 action,drv_vm_entry_t *entry,
    uint32 act_param);
int drv_tbx_vm_entry_read(int unit,uint32 index,uint32 ram_type,
    drv_vm_entry_t *entry);
int drv_tbx_vm_entry_write(int unit,uint32 index,uint32 ram_type,
    drv_vm_entry_t *entry);
int drv_tbx_vm_qset_get(int unit,uint32 qual,drv_vm_entry_t *entry,
    uint32 *val);
int drv_tbx_vm_qset_set(int unit,uint32 qual,drv_vm_entry_t *entry,
    uint32 val);
int drv_tbx_vm_format_id_select(int unit,drv_vm_entry_t *entry,
    uint32 *format_id,uint32 flags);
int drv_tbx_vm_format_to_qset(int unit,uint32 mem_type,uint32 id,
    drv_vm_entry_t *entry);
int drv_tbx_vm_range_set(int unit,uint32 id,uint32 min,uint32 max);
int drv_tbx_vm_flow_alloc(int unit,uint32 type,int *flow_id);
int drv_tbx_vm_flow_free(int unit,int flow_id);
int drv_tbx_vm_ranger_inc(int unit,int ranger_id);
int drv_tbx_vm_ranger_dec(int unit,int ranger_id);
int drv_tbx_vm_ranger_count_get(int unit,int ranger_id,uint32 *count);

int drv_tbx_dev_control_set(int unit,uint32 *ctrl_cnt,uint32 *type_list,
    int *value_list);
int drv_tbx_dev_control_get(int unit,uint32 *ctrl_cnt,uint32 *type_list,
    int *value_list);

/* FP */
int drv_tbx_fp_init(int unit,int stage_id);
int drv_tbx_fp_deinit(int unit,int stage_id);
int drv_tbx_fp_qual_value_set(int unit,int stage_id,
    drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_tbx_fp_qual_value_get(int unit,int stage_id,
    drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_tbx_fp_udf_value_set(int unit,int stage_id,uint32 udf_idex,
    void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_tbx_fp_udf_value_get(int unit,int stage_id,uint32 udf_idex,
    void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_tbx_fp_entry_mem_control(int unit,int stage_id,int op,
    void *src_entry,void *dst_entry,void **alloc_entry);
int drv_tbx_fp_entry_tcam_control(int unit,int stage_id,
    void *drv_entry,int op,int param1,void *param2);
int drv_tbx_fp_action_conflict(int unit,int stage_id,
    drv_field_action_t act1,drv_field_action_t act2);
int drv_tbx_fp_action_support_check(int unit,int stage_id,
    drv_field_action_t action);
int drv_tbx_fp_action_add(int unit,int stage_id,void *drv_entry,
    drv_field_action_t action,uint32 param0,uint32 param1);
int drv_tbx_fp_action_remove(int unit,int stage_id,void *drv_entry,
    drv_field_action_t action,uint32 param0,uint32 param1);
int drv_tbx_fp_selcode_mode_get(int unit,int stage_id,void *qset,
    int mode,int8 *slice_id,uint32 *slice_map,void **drv_entry);
int drv_tbx_fp_selcode_to_qset(int unit,int stage_id,int slice_id,
    void *qset);
int drv_tbx_fp_qualify_support(int unit,int stage_id,void *qset);
int drv_tbx_fp_id_control(int unit,int type,int op,int flags,
    int *id,uint32 *count);
int drv_tbx_fp_tcam_parity_check(int unit,
    drv_fp_tcam_checksum_t *drv_fp_tcam_chksum);
int drv_tbx_fp_policer_control(int unit,int stage_id,int op,
    void *entry,drv_policer_config_t *policer_cfg);
int drv_tbx_fp_stat_support_check(int unit,int stage_id,int op,
    int param0,void *mode);
int drv_tbx_fp_stat_type_get(int unit,int stage_id,
    drv_policer_mode_t policer_mode,drv_field_stat_t stat,int *type1,
    int *type2,int *type3);

int drv_tbx_dev_prop_get(int unit,uint32 prop_type,uint32 *prop_val);
             
        
int drv_tbx_mem_insert(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_tbx_mem_search(int unit, uint32 mem, uint32 *key, uint32 *entry, 
                uint32 *entry_1, uint32 flags);
int drv_tbx_mem_clear(int unit, uint32 mem);
int drv_tbx_mem_delete(int unit, uint32 mem, uint32 *entry, uint32 flags);
int drv_tbx_mem_field_get(int unit, uint32 mem, 
                uint32 field_index, uint32 *entry, uint32 *fld_data);
int drv_tbx_mem_field_set(int unit, uint32 mem, 
                uint32 field_index, uint32 *entry, uint32 *fld_data);
int drv_tbx_mem_length_get(int unit, uint32 mem, uint32 *data);
int drv_tbx_mem_width_get(int unit, uint32 mem, uint32 *data);
int drv_tbx_mem_read(int unit, uint32 mem, uint32 entry_id, 
                uint32 count, uint32 *entry);
int drv_tbx_mem_write(int unit, uint32 mem, uint32 entry_id, 
                uint32 count, uint32 *entry);
int drv_tbx_mem_fill(int unit, uint32 mem, 
    int entry_id, int count, uint32 *entry);
int drv_tbx_mem_move(int unit, uint32 mem,int src_index, 
    int dest_index, int count, int flag);
      
#endif  /* !_TBX_SERVICE_H */
