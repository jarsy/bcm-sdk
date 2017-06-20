/*
 * $Id: lotus_service.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _LOTUS_SERVICE_H
#define _LOTUS_SERVICE_H

int drv_gex_port_set(int unit, soc_pbmp_t bmp, 
                uint32 prop_type, uint32 prop_val);
int drv_gex_port_get(int unit, int port, 
                uint32 prop_type, uint32 *prop_val);

int drv_gex_port_status_get(int unit, uint32 port, uint32 status_type, uint32 *val);

int drv_lotus_port_pri_mapop_set(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 pri_new, uint32 cfi_new);
int drv_lotus_port_pri_mapop_get(int unit, int port, int op_type, 
                uint32 pri_old, uint32 cfi_old, uint32 *pri_new, uint32 *cfi_new);

int drv_lotus_port_cross_connect_set(int unit,uint32 port,soc_pbmp_t bmp);
int drv_lotus_port_cross_connect_get(int unit,uint32 port,soc_pbmp_t *bmp);

int drv_gex_security_set(int unit, soc_pbmp_t bmp, uint32 state, uint32 mask);
int drv_gex_security_get(int unit, uint32 port, uint32 *state, uint32 *mask);

int drv_gex_security_egress_set(int unit, soc_pbmp_t bmp, int enable);
int drv_gex_security_egress_get(int unit, int port, int *enable);
int drv_lotus_dev_prop_get(int unit,uint32 prop_type,uint32 *prop_val);
int drv_lotus_dev_prop_set(int unit,uint32 prop_type,uint32 prop_val);


extern drv_if_t drv_lotus_services;

#endif
