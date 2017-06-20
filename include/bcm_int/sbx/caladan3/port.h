/* 
 * $Id: port.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        port.h
 * Purpose:     PORT internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_CALADAN3_PORT_H_
#define _BCM_INT_SBX_CALADAN3_PORT_H_

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

#define PORT(_unit, _port)            (_sbx_port_handler[_unit]->port[_port])

/* Reference usage count for each tpid */
typedef struct _tpid_count_s {
    int    ctpid;
    int    stpid0;
    int    stpid1;
    int    twin;
} _tpid_count_t;

#define TPID_COUNT(unit)    _port_state[unit].tpid_count

#define TPID_DEFAULT         0x8100
#define TPID_CTPID_DEFAULT   TPID_DEFAULT
#define TPID_STPID0_DEFAULT  0x88a8
#define TPID_STPID1_DEFAULT  0x9100

/*  Profile reference counts
 *  Maintain a reference count for each qos profile rsesource.  Logical ports
 *  may share profiles, iff all entries are equivalent.
 */
#define PROFILE_REF_COUNT(unit, profile) \
  _port_state[unit].profile_count[profile]

typedef struct _g3p1_port_s {

    _tpid_count_t  tpid_count;
    int            *profile_count;
} _g3p1_port_t;


#ifdef BCM_WARM_BOOT_SUPPORT
extern INLINE _sbx_port_handler_t *bcm_sbx_caladan3_port_handler_get(int unit);
#endif

extern int bcm_caladan3_port_selective_set(int unit, bcm_port_t port, bcm_port_info_t *info);

extern int bcm_caladan3_port_link_get(int unit,
                                     bcm_port_t port, int hw, int *up);

extern int bcm_caladan3_port_modid_set(int unit, 
                                       bcm_module_t modid);

extern int bcm_caladan3_port_egr_remark_idx_get(int unit, bcm_port_t port,
                                                 uint32 *idx);


extern int bcm_caladan3_port_strip_tag(int unit, bcm_port_t port, int strip);

extern int bcm_caladan3_port_untagged_vlan_touch(int unit, bcm_port_t phyPort,
                                                bcm_vlan_t vid);
extern int bcm_caladan3_port_default_qos_profile_set(int unit, bcm_port_t port,
                                                    bcm_vlan_t vid);

extern int bcm_caladan3_port_qosmap_set(int unit, bcm_port_t port, 
                                      int ingrMap, int egrMap,
                                      uint32 ingFlags, uint32 egrFlags);

extern int bcm_caladan3_port_qosmap_get(int unit, bcm_port_t port, 
                                      int *ing_map, int *egr_map,
                                      uint32 *ing_flags, uint32 *egr_flags);

extern int bcm_caladan3_port_vlan_qosmap_set(int unit, bcm_port_t port, 
                                           bcm_vlan_t vid,
                                           int ing_map, int egr_map,
                                           uint32 ing_flags, uint32 egr_flags);

extern int bcm_caladan3_port_vlan_qosmap_get(int unit, bcm_port_t port, 
                                           bcm_vlan_t vid,
                                           int *ing_map, int *egr_map,
                                           uint32 *ing_flags, uint32 *egr_flags);

#endif /* _BCM_INT_SBX_CALADAN3_PORT_H_ */
