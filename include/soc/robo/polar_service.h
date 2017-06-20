/*
 * $Id: polar_service.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _POLAR_SERVICE_H
#define _POLAR_SERVICE_H

int drv_gex_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val);
int drv_gex_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val);

int drv_gex_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val);

int drv_polar_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_polar_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new);

int drv_gex_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_gex_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);

int drv_gex_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_gex_security_egress_get(int unit, int port, int *enable);

int drv_polar_dev_prop_get(int unit,uint32 prop_type,uint32 *prop_val);
int drv_polar_dev_prop_set(int unit,uint32 prop_type,uint32 prop_val);

int drv_polar_queue_mode_set(int unit, soc_pbmp_t bmp, uint32 flag, 
    uint32 mode);
int drv_polar_queue_mode_get(int unit, uint32 port, uint32 flag, 
    uint32 *mode);
int drv_polar_queue_count_set(int unit, uint32 port_type, uint8 count);
int drv_polar_queue_count_get(int unit, uint32 port_type, uint8 *count);
int drv_polar_queue_WRR_weight_set(int unit, uint32 port_type, 
    soc_pbmp_t bmp, uint8 queue, uint32 weight);
int drv_polar_queue_WRR_weight_get(int unit, uint32 port_type, 
    uint32 port, uint8 queue, uint32 *weight);
int drv_polar_queue_prio_set(int unit, uint32 port, uint8 prio, 
    uint8 queue_n);
int drv_polar_queue_prio_get(int unit, uint32 port, uint8 prio, 
    uint8 *queue_n);
int drv_polar_queue_mapping_type_set(int unit, soc_pbmp_t bmp, 
    uint32 mapping_type, uint8 state);
int drv_polar_queue_mapping_type_get(int unit, uint32 port, 
    uint32 mapping_type, uint8 *state);
int drv_polar_queue_port_prio_to_queue_set
    (int unit, uint8 port, uint8 prio, uint8 queue_n);
int drv_polar_queue_port_prio_to_queue_get
    (int unit, uint8 port, uint8 prio, uint8 *queue_n);

extern drv_if_t drv_polar_services;

#endif
