/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _DINO16_SERVICE_H
#define _DINO16_SERVICE_H

extern drv_if_t drv_dino16_services;

int drv_dino16_mac_set(int unit, soc_pbmp_t pbmp, uint32 mac_type, 
    uint8* mac, uint32 bpdu_idx);
int drv_dino16_mac_get(int unit, uint32 port, uint32 mac_type, 
    soc_pbmp_t *bmp, uint8* mac);

int drv_dino16_trunk_set(int unit, int tid, soc_pbmp_t bmp, 
    uint32 flag, uint32 hash_op);
int drv_dino16_trunk_get(int unit, int tid, soc_pbmp_t *bmp, 
    uint32 flag, uint32 *hash_op);
int drv_dino16_trunk_hash_field_add(int unit, uint32 field_type);
int drv_dino16_trunk_hash_field_remove(int unit, uint32 field_type);

int drv_dino16_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode);
int drv_dino16_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode);
int drv_dino16_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_dino16_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_dino16_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n);
int drv_dino16_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n);
int drv_dino16_queue_port_prio_to_queue_set(int unit, uint8 port, 
    uint8 prio,uint8 queue_n);
int drv_dino16_queue_port_prio_to_queue_get(int unit, uint8 port, 
    uint8 prio,uint8 * queue_n);
int drv_dino16_queue_prio_remap_set(int unit, uint32 port, uint8 pre_prio,
    uint8 prio);
int drv_dino16_queue_prio_remap_get(int unit, uint32 port, uint8 pre_prio,
    uint8 * prio);

int drv_dino16_trap_set(int unit, soc_pbmp_t bmp, uint32 trap_mask);
int drv_dino16_trap_get(int unit, soc_port_t port, uint32 *trap_mask);
int drv_dino16_snoop_set(int unit, uint32 snoop_mask);
int drv_dino16_snoop_get(int unit, uint32 *snoop_mask);

int drv_dino16_rate_config_set(int unit, soc_pbmp_t pbmp, 
    uint32 config_type, uint32 value);
int drv_dino16_rate_config_get(int unit, uint32 port, 
    uint32 config_type, uint32 *value);
int drv_dino16_rate_set
    (int unit, soc_pbmp_t bmp, uint8 queue_n, int direction, 
    uint32 flags, uint32 kbits_sec_min, 
    uint32 kbits_sec_max, uint32 burst_size);
int drv_dino16_rate_get
    (int unit, uint32 port, uint8 queue_n, int direction, 
    uint32 *flags, uint32 *kbits_sec_min, 
    uint32 *kbits_sec_max, uint32 *burst_size);

int drv_dino16_storm_control_enable_set
    (int unit, uint32 port, uint8 enable);
int drv_dino16_storm_control_enable_get
    (int unit, uint32 port, uint8 *enable);
int drv_dino16_storm_control_set(int unit, soc_pbmp_t bmp, uint32 type, 
    uint32 limit, uint32 burst_size);
int drv_dino16_storm_control_get(int unit, uint32 port, uint32 *type, 
    uint32 *limit, uint32 *burst_size);

int drv_dino16_igmp_mld_snoop_mode_get
    (int unit, int type, int *mode);
int drv_dino16_igmp_mld_snoop_mode_set
    (int unit, int type, int mode);

int drv_gex_port_set(int unit, soc_pbmp_t bmp, 
    uint32 prop_type, uint32 prop_val);
int drv_gex_port_get(int unit, int port, 
    uint32 prop_type, uint32 *prop_val);

int drv_gex_port_status_get(int unit, uint32 port, uint32 
    status_type, uint32 *val);

int drv_dino16_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, 
    uint32 port_state);
int drv_dino16_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, 
    uint32 *port_state);

int drv_dino16_security_set(int unit, soc_pbmp_t bmp, 
    uint32 state, uint32 mask);
int drv_dino16_security_get(int unit, uint32 port, 
    uint32 *state, uint32 *mask);

int drv_dino16_security_egress_set(int unit, soc_pbmp_t bmp, 
    int enable);
int drv_dino16_security_egress_get(int unit, int port, 
    int *enable);

int drv_dino16_mem_read(int unit, uint32 mem, uint32 entry_id, 
    uint32 count, uint32 *entry);
int drv_dino16_mem_write(int unit, uint32 mem, uint32 entry_id, 
    uint32 count, uint32 *entry);
int drv_dino16_mem_field_get(int unit, uint32 mem, uint32 field_index, 
    uint32 *entry, uint32 *fld_data);
int drv_dino16_mem_field_set(int unit, uint32 mem, uint32 field_index, 
     uint32 *entry, uint32 *fld_data);
int drv_dino16_mem_clear(int unit, uint32 mem);
int drv_dino16_mem_search(int unit, uint32 mem, uint32 *key, 
    uint32 *entry, uint32 *entry1, uint32 flags);
int drv_dino16_mem_insert(int unit, uint32 mem, uint32 *entry, 
    uint32 flags);
int drv_dino16_mem_delete(int unit, uint32 mem, uint32 *entry, 
    uint32 flags);

int drv_dino16_dev_prop_get(int unit, uint32 prop_type, 
    uint32 *prop_val);

#endif
