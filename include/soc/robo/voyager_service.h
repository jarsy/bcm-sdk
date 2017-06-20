/*
 * $Id: voyager_service.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _VOYAGER_SERVICE_H
#define _VOYAGER_SERVICE_H
int drv_tbx_port_set(int unit, soc_pbmp_t bmp, 
        uint32 prop_type, uint32 prop_val);
int drv_tbx_port_get(int unit, int port, 
        uint32 prop_type, uint32 *prop_val);

int drv_tbx_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val);

int drv_tbx_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_tbx_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new);

int drv_tbx_port_block_set(int unit, int port, uint32 block_type,
        soc_pbmp_t egress_pbmp);
int drv_tbx_port_block_get(int unit, int port, uint32 block_type, 
        soc_pbmp_t *egress_pbmp);

int drv_tbx_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_tbx_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);

int drv_tbx_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_tbx_security_egress_get(int unit, int port, int *enable);

int drv_tbx_security_eap_control_set(int unit, uint32 type, uint32 value);
int drv_tbx_security_eap_control_get(int unit, uint32 type, uint32 *value);

int drv_vo_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, 
    uint32 *slice_id, uint32 flags);
int drv_vo_cfp_udf_get(int unit, uint32 port, uint32 udf_index, 
    uint32 *offset, uint32 *base);
int drv_vo_cfp_udf_set(int unit, uint32 port, uint32 udf_index, 
    uint32 offset, uint32 base);
int drv_vo_cfp_range_set(int unit, uint32 type, uint32 id, 
    uint32 param1, uint32 param2);
int drv_vo_cfp_range_get(int unit, uint32 type, uint32 id, 
    uint32 *param1, uint32 *param2);

int _drv_vo_cfp_qual_value_get(int unit, drv_field_qualify_t qual, 
    drv_cfp_entry_t *drv_entry, uint32 *p_data, uint32 *p_mask);
int _drv_vo_cfp_qual_value_set(int unit, drv_field_qualify_t qual, 
    drv_cfp_entry_t *drv_entry, uint32 *p_data, uint32 *p_mask);
int _drv_vo_cfp_udf_value_set(int unit, uint32 udf_idx, 
    drv_cfp_entry_t *drv_entry, uint32 *p_data, uint32 *p_mask);
int _drv_vo_cfp_udf_value_get(int unit, uint32 udf_idx, 
    drv_cfp_entry_t *drv_entry, uint32 *p_data, uint32 *p_mask);
int _drv_vo_fp_entry_tcam_slice_id_set(int unit, int stage_id, 
    void *entry, int sliceId, void *slice_map);
int _drv_vo_fp_entry_cfp_tcam_policy_install(int unit, void *entry, 
    int tcam_idx, int tcam_chain_idx);
int _drv_vo_fp_entry_tcam_chain_mode_get(int unit, int stage_id, 
    void *drv_entry, int sliceId, void *mode);
int _drv_vo_qset_to_cfp(int unit, drv_field_qset_t qset, 
    drv_cfp_entry_t * drv_entry, int mode);
int _drv_vo_fp_cfp_qualify_support(int unit, drv_field_qset_t qset);
int _drv_vo_cfp_data_mask_read(int unit, uint32 ram_type, 
                         uint32 index, drv_cfp_entry_t *cfp_entry);
int _drv_vo_cfp_data_mask_write(int unit, uint32 ram_type, 
                              uint32 index, drv_cfp_entry_t *cfp_entry);
int _drv_vo_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int _drv_vo_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);

int _drv_vo_cfp_mem_hw2sw(int unit, uint32 *hw_entry, uint32 *sw_entry);
int _drv_vo_cfp_mem_sw2hw(int unit, uint32 *sw_entry, uint32 *hw_entry);




extern drv_if_t drv_voyager_services;

#endif
