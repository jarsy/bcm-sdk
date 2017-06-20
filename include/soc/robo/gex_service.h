/*
 * $Id: gex_service.h,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _GEX_SERVICE_H
#define _GEX_SERVICE_H

int drv_gex_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
    uint8* mac, uint32 bpdu_idx);
int drv_gex_mac_get(int unit, uint32 port, uint32 mac_type, 
    soc_pbmp_t *bmp, uint8* mac);
int drv_gex_arl_learn_enable_set(int unit, soc_pbmp_t pbmp, uint32 mode);
int drv_gex_arl_learn_enable_get(int unit, soc_port_t port, uint32 *mode);


int drv_gex_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op);
int drv_gex_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op);
int drv_gex_trunk_hash_field_add(int unit, uint32 field_type);
int drv_gex_trunk_hash_field_remove(int unit, uint32 field_type);

int drv_gex_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode);
int drv_gex_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode);
int drv_gex_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_gex_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_gex_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight);
int drv_gex_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight);
int drv_gex_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n);
int drv_gex_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n);
int drv_gex_queue_dfsv_set(int unit, uint8 code_point, uint8 queue_n);
int drv_gex_queue_dfsv_get(int unit, uint8 code_point, uint8 *queue_n);
int drv_gex_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state);
int drv_gex_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state);
int drv_gex_queue_port_prio_to_queue_set
    (int unit, uint8 port, uint8 prio, uint8 queue_n);
int drv_gex_queue_port_prio_to_queue_get
    (int unit, uint8 port, uint8 prio, uint8 *queue_n);
int drv_gex_queue_port_dfsv_set(int unit, uint8 port, uint8 dscp, 
    uint8 prio, uint8 queue_n);
int drv_gex_queue_port_dfsv_get(int unit, uint8 port, uint8 dscp, 
    uint8 *prio, uint8 *queue_n);
int drv_gex_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio,
    uint8 prio);
int drv_gex_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio, 
    uint8 *prio);
int drv_gex_queue_dfsv_remap_set(int unit, uint8 dscp, uint8 prio);
int drv_gex_queue_dfsv_remap_get(int unit, uint8 dscp, uint8 *prio);
int drv_gex_queue_rx_reason_set(int unit, uint8 reason, uint32 queue);
int drv_gex_queue_rx_reason_get(int unit, uint8 reason, uint32 *queue);

int drv_gex_mirror_set(int unit, uint32 enable, soc_pbmp_t mport_bmp, 
    soc_pbmp_t ingress_bmp, soc_pbmp_t egress_bmp);
int drv_gex_mirror_get(int unit, uint32 *enable, soc_pbmp_t *mport_bmp, 
    soc_pbmp_t *ingress_bmp, soc_pbmp_t *egress_bmp);

int drv_gex_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask);
int drv_gex_trap_get(int unit, soc_port_t port, uint32 *trap_mask);
int drv_gex_snoop_set(int unit, uint32 snoop_mask);
int drv_gex_snoop_get(int unit, uint32 *snoop_mask);

int drv_gex_rate_config_set(int unit, soc_pbmp_t pbmp, 
    uint32 config_type, uint32 value);
int drv_gex_rate_config_get(int unit, uint32 port, 
    uint32 config_type, uint32 *value);
int drv_gex_rate_set(int unit, soc_pbmp_t bmp, uint8 queue_n, int direction,
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size);
int drv_gex_rate_get(int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flags, uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, uint32 *burst_size);

int drv_gex_storm_control_enable_set
    (int unit, uint32 port, uint8 enable);
int drv_gex_storm_control_enable_get
    (int unit, uint32 port, uint8 *enable);
int drv_gex_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_gex_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size);

int drv_gex_igmp_mld_snoop_mode_get(int unit, int type, int *mode);
int drv_gex_igmp_mld_snoop_mode_set(int unit, int type, int mode);

int drv_gex_vlan_vt_add(int unit, uint32 vt_type, uint32 port,  uint32 cvid, 
                uint32 sp_vid, uint32 pri, uint32 mode);
int drv_gex_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid);
int drv_gex_vlan_vt_delete_all(int unit, uint32 vt_type);
int drv_gex_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                uint32 port, uint32 prop_val);
int drv_gex_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                uint32 port, uint32 *prop_val);
int drv_gex_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val);
int drv_gex_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val);
int drv_gex_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val);
int drv_gex_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val);
                
int drv_gex_port_vlan_set(int unit,uint32 port,soc_pbmp_t bmp);
int drv_gex_port_vlan_get(int unit,uint32 port,soc_pbmp_t *bmp);

int drv_gex_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state);
int drv_gex_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state);

int drv_gex_mcast_bmp_set(int unit, uint32 *entry, soc_pbmp_t bmp, uint32 flag);
int drv_gex_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp);

/* CFP */
int drv_gex_cfp_init(int unit);
int drv_gex_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param);
int drv_gex_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);
int drv_gex_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2);
int drv_gex_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2);
int drv_gex_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_gex_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry);
int drv_gex_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_gex_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_gex_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_gex_cfp_meter_rate_transform(int unit, uint32 kbits_sec, 
  uint32 kbits_burst, uint32 *bucket_size, uint32 * ref_cnt, uint32 *ref_unit);
int drv_gex_cfp_meter_get(int unit, drv_cfp_entry_t* entry, 
    uint32 *kbits_sec, uint32 *kbits_burst);
int drv_gex_cfp_meter_set(int unit, drv_cfp_entry_t* entry, 
    uint32 kbits_sec, uint32 kbits_burst);
int drv_gex_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, 
    uint32 *slice_id, uint32 flags);
int drv_gex_cfp_slice_to_qset(int unit, uint32 slice_id, 
    drv_cfp_entry_t *entry);
int drv_gex_cfp_stat_get(int unit, uint32 stat_type, uint32 index, 
    uint32* counter);
int drv_gex_cfp_stat_set(int unit, uint32 stat_type, uint32 index, 
    uint32 counter);
int drv_gex_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base);
int drv_gex_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);
int drv_gex_cfp_sub_qual_by_udf(int unit, int enable, int slice_id, 
    uint32 sub_qual, drv_cfp_qual_udf_info_t * qual_udf_info);

/* EAV */
int drv_gex_eav_control_set(int unit,uint32 type,uint32 param);
int drv_gex_eav_control_get(int unit,uint32 type,uint32 *param);
int drv_gex_eav_enable_set(int unit,uint32 port,uint32 enable);
int drv_gex_eav_enable_get(int unit,uint32 port,uint32 *enable);
int drv_gex_eav_link_status_set(int unit,uint32 port,uint32 link);
int drv_gex_eav_link_status_get(int unit,uint32 port,uint32 *link);
int drv_gex_eav_egress_timestamp_get(int unit,uint32 port,
    uint32 *param);
int drv_gex_eav_time_sync_set(int unit,uint32 type,uint32 p0,uint32 p1);
int drv_gex_eav_time_sync_get(int unit,uint32 type,uint32 *p0,uint32 *p1);
int drv_gex_eav_queue_control_set(int unit,uint32 port,uint32 type,
    uint32 param);
int drv_gex_eav_queue_control_get(int unit,uint32 port,uint32 type,
    uint32 *param);
int drv_gex_eav_time_sync_mac_set(int unit,uint8 *mac,
    uint16 ethertype);
int drv_gex_eav_time_sync_mac_get(int unit,uint8 *mac,
    uint16 *ethertype);

int drv_gex_timesync_config_set(
     int unit, uint32 port, soc_port_phy_timesync_config_t *conf);
int drv_gex_timesync_config_get(
     int unit, uint32 port, soc_port_phy_timesync_config_t *conf);

int drv_gex_control_timesync_set(
     int unit, uint32 port, soc_port_control_phy_timesync_t type, uint64 value);
int drv_gex_control_timesync_get(
     int unit, uint32 port, soc_port_control_phy_timesync_t type, uint64 *value);


int drv_gex_dos_enable_set(int unit,uint32 type,uint32 param);
int drv_gex_dos_enable_get(int unit,uint32 type,uint32 *param);

/* FP */
int drv_gex_fp_init(int unit,int stage_id);
int drv_gex_fp_deinit(int unit,int stage_id);
int drv_gex_fp_qual_value_set(int unit,int stage_id,
    drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_gex_fp_qual_value_get(int unit,int stage_id,
    drv_field_qualify_t qual,void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_gex_fp_udf_value_set(int unit,int stage_id,uint32 udf_idex,
    void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_gex_fp_udf_value_get(int unit,int stage_id,uint32 udf_idex,
    void *drv_entry,uint32 *p_data,uint32 *p_mask);
int drv_gex_fp_entry_mem_control(int unit,int stage_id,int op,
    void *src_entry,void *dst_entry,void **alloc_entry);
int drv_gex_fp_entry_tcam_control(int unit,int stage_id,
    void *drv_entry,int op,int param1,void *param2);
int drv_gex_fp_action_conflict(int unit,int stage_id,
    drv_field_action_t act1,drv_field_action_t act2);
int drv_gex_fp_action_support_check(int unit,int stage_id,
    drv_field_action_t action);
int drv_gex_fp_action_add(int unit,int stage_id,void *drv_entry,
    drv_field_action_t action,uint32 param0,uint32 param1);
int drv_gex_fp_action_remove(int unit,int stage_id,void *drv_entry,
    drv_field_action_t action,uint32 param0,uint32 param1);
int drv_gex_fp_selcode_mode_get(int unit,int stage_id,void *qset,
    int mode,int8 *slice_id,uint32 *slice_map,void **drv_entry);
int drv_gex_fp_selcode_to_qset(int unit,int stage_id,int slice_id,
    void *qset);
int drv_gex_fp_qualify_support(int unit,int stage_id,void *qset);
int drv_gex_fp_id_control(int unit,int type,int op,int flags,
    int *id,uint32 *count);
int drv_gex_fp_tcam_parity_check(int unit,
    drv_fp_tcam_checksum_t *drv_fp_tcam_chksum);
int drv_gex_fp_policer_control(int unit,int stage_id,int op,
    void *entry,drv_policer_config_t *policer_cfg);
int drv_gex_fp_stat_support_check(int unit,int stage_id,int op,
    int param0,void *mode);
int drv_gex_fp_stat_type_get(int unit,int stage_id,
    drv_policer_mode_t policer_mode,drv_field_stat_t stat,int *type1,
    int *type2,int *type3);

int drv_gex_mem_length_get(int unit, uint32 mem, uint32 *data);
int drv_gex_mem_width_get(int unit, uint32 mem, uint32 *data);
int drv_gex_mem_read(int unit, uint32 mem, uint32 entry_id, 
                            uint32 count, uint32 *entry);
int drv_gex_mem_write(int unit, uint32 mem, uint32 entry_id, 
                            uint32 count, uint32 *entry);
int drv_gex_mem_field_get(int unit, uint32 mem, uint32 field_index, 
                            uint32 *entry, uint32 *fld_data);
int drv_gex_mem_field_set(int unit, uint32 mem, uint32 field_index, 
                            uint32 *entry, uint32 *fld_data);
int drv_gex_mem_clear(int unit, uint32 mem);
int drv_gex_mem_search(int unit, uint32 mem, uint32 *key, 
                            uint32 *entry, uint32 *entry_1, uint32 flags);
int drv_gex_mem_insert(int unit, uint32 mem, uint32 *entry, 
                            uint32 flags);
int drv_gex_mem_delete(int unit, uint32 mem, uint32 *entry, 
                            uint32 flags);


#endif
