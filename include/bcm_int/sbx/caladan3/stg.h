/*
 * $Id: stg.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains STG definitions internal to the BCM library.
 */

#ifndef _BCM_INT_SBX_CALADAN3_STG_H_
#define _BCM_INT_SBX_CALADAN3_STG_H_

#define BCM_STG_MAX     BCM_VLAN_COUNT

/*
 * The STG info structure is protected by STG_DB_LOCK. The hardware PTABLE and
 * hardware STG table are protected by memory locks in the lower level.
 */
typedef struct bcm_stg_info_s {
    int         init;       /* TRUE if STG module has been initialized */
    sal_mutex_t lock;       /* Database lock */
    bcm_stg_t   stg_min;    /* STG table min index */
    bcm_stg_t   stg_max;    /* STG table max index */
    bcm_stg_t   stg_defl;   /* Default STG */
    SHR_BITDCL *stg_bitmap; /* Bitmap of allocated STGs */
    bcm_pbmp_t *stg_enable; /* array of port bitmaps indicating whether the
                               port+stg has STP enabled */
    bcm_pbmp_t *stg_state_h;/* array of port bitmaps indicating STP state for the */
    bcm_pbmp_t *stg_state_l;/* port+stg combo. Only valid if stg_enable = TRUE */
    int         stg_count;  /* Number STGs allocated */
    /* STG reverse map - keep a linked list of VLANs in each STG */
    bcm_vlan_t *vlan_first; /* Indexed by STG (also links free list) */
    bcm_vlan_t *vlan_next;  /* Indexed by VLAN ID */

} bcm_stg_info_t;

#define STG_DB_LOCK(unit) \
        sal_mutex_take(caladan3_stg_info[(unit)].lock, sal_mutex_FOREVER)

#define STG_DB_UNLOCK(unit) sal_mutex_give(caladan3_stg_info[(unit)].lock)

#define STG_CNTL(unit)  caladan3_stg_info[(unit)]


extern int _bcm_caladan3_stg_map_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg);

extern int bcm_caladan3_stg_default_get(int unit, bcm_stg_t *stg_ptr);

extern int bcm_caladan3_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid);

extern int _bcm_caladan3_stg_vlan_remove(int unit,
                                       bcm_stg_t stg,
                                       bcm_vlan_t vid,
                                       int destroy);

extern int bcm_caladan3_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state);

extern int bcm_caladan3_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state);

extern int _bcm_caladan3_stg_vlan_port_add(int unit, bcm_vlan_t vid, bcm_port_t port);

extern int _bcm_caladan3_stg_vlan_port_remove(int unit, bcm_vlan_t vid, bcm_port_t port);

#endif	/* _BCM_INT_SBX_CALADAN3_STG_H_ */
