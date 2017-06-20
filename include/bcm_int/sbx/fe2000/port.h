/* 
 * $Id: port.h,v 1.18 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        port.h
 * Purpose:     PORT internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_PORT_H_
#define _BCM_INT_SBX_FE2000_PORT_H_

/*
 * Port Handler
 *
 * The following structure stores the PORT module internal software
 * information on a device.
 *
 * Access to following structure should protected by PORT_LOCK.
 */
typedef struct _port_info_s {
    mac_driver_t        *p_mac;         /* Per port MAC driver */
    uint32            egr_remark_table_idx; /* egress remark index */
    bcm_port_congestion_config_t *e2ecc_config; 
                                        /* End-to-end congestion control configuration */ 
} _sbx_port_info_t;

typedef struct _port_handler_s {
    uint32              ete_l2;         /* Raw OHI untagged-ete */
    int                 ete_l2_valid;
    _sbx_port_info_t    port[SOC_MAX_NUM_PORTS];
} _sbx_port_handler_t;

extern _sbx_port_handler_t              *_sbx_port_handler[BCM_LOCAL_UNITS_MAX];

extern int _bcm_fe2000_port_link_get(int unit,
                                     bcm_port_t port, int hw, int *up);

extern int _bcm_fe2000_port_modid_set(int unit, 
                                      bcm_module_t modid);

extern int _bcm_fe2000_port_egr_remark_idx_get(int unit, bcm_port_t port,
                                               uint32 *idx);

extern int _bcm_fe2000_port_ilib_lp_init(int unit, bcm_module_t modid, 
                                         bcm_port_t port);

extern int _bcm_fe2000_port_vlan_lp_set(int unit, bcm_module_t modid,
                                        bcm_port_t port, bcm_vlan_t vlan);

extern int _bcm_fe2000_port_strip_tag(int unit, bcm_port_t port, int strip);

extern int _bcm_fe2000_port_default_qos_profile_set(int unit, bcm_port_t port,
                                                    bcm_vlan_t vid);
extern int _bcm_fe2000_port_untagged_vlan_touch(int unit, bcm_port_t phyPort,
                                                bcm_vlan_t vid);

extern int bcm_fe2000_port_qosmap_set(int unit, bcm_port_t port, 
                                      int ingrMap, int egrMap,
                                      uint32 ingFlags, uint32 egrFlags);

extern int bcm_fe2000_port_qosmap_get(int unit, bcm_port_t port, 
                                      int *ing_map, int *egr_map,
                                      uint32 *ing_flags, uint32 *egr_flags);

extern int bcm_fe2000_port_vlan_qosmap_set(int unit, bcm_port_t port, 
                                           bcm_vlan_t vid,
                                           int ing_map, int egr_map,
                                           uint32 ing_flags, uint32 egr_flags);

extern int bcm_fe2000_port_vlan_qosmap_get(int unit, bcm_port_t port, 
                                           bcm_vlan_t vid,
                                           int *ing_map, int *egr_map,
                                           uint32 *ing_flags, uint32 *egr_flags);

#endif /* _BCM_INT_SBX_FE2000_PORT_H_ */
