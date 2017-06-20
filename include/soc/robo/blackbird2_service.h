/*
 * $Id: blackbird2_service.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _BLACKBIRD2_SERVICE_H
#define _BLACKBIRD2_SERVICE_H

int drv_gex_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val);
int drv_gex_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val);

int drv_gex_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val);

int drv_blackbird2_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_blackbird2_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new);

int drv_blackbird2_port_cross_connect_set(int unit,uint32 port,soc_pbmp_t bmp);
int drv_blackbird2_port_cross_connect_get(int unit,uint32 port,soc_pbmp_t *bmp);

int drv_gex_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_gex_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);

int drv_gex_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_gex_security_egress_get(int unit, int port, int *enable);

/* CFP */
int drv_blackbird2_cfp_init(int unit);
int drv_blackbird2_cfp_action_get(int unit, uint32* action, 
            drv_cfp_entry_t* entry, uint32* act_param);
int drv_blackbird2_cfp_action_set(int unit, uint32 action, 
            drv_cfp_entry_t* entry, uint32 act_param1, uint32 act_param2);
int drv_blackbird2_cfp_control_get(int unit, uint32 control_type, 
    uint32 param1, uint32 *param2);
int drv_blackbird2_cfp_control_set(int unit, uint32 control_type, 
    uint32 param1, uint32 param2);
int drv_blackbird2_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_blackbird2_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry);
int drv_blackbird2_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_blackbird2_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val);
int drv_blackbird2_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, 
    uint32 *slice_id, uint32 flags);
int drv_blackbird2_cfp_slice_to_qset(int unit, uint32 slice_id, 
    drv_cfp_entry_t *entry);

int drv_blackbird2_dev_prop_get(int unit,uint32 prop_type,uint32 *prop_val);
int drv_blackbird2_dev_prop_set(int unit,uint32 prop_type,uint32 prop_val);


extern drv_if_t drv_blackbird2_services;

#endif
