/* 
 * $Id: mim.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mim.h
 * Purpose:     MIM internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_CALADAN3_MIM_H_
#define _BCM_INT_SBX_CALADAN3_MIM_H_

#include <bcm/mim.h>

#define _BCM_CALADAN3_GET_PORTCB_FROM_LIST(e, var) \
            (var) = DQ_ELEMENT(bcm_caladan3_mim_port_control_t*,\
                               (e), (var), listnode)


typedef enum {
    _BCM_CALADAN3_MIM_ACCESS_PORT,
    _BCM_CALADAN3_MIM_BACKBONE_PORT,
    _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT,
    _BCM_CALADAN3_MIM_PORT_MAX
} bcm_caladan3_mim_port_type_e_t;

typedef struct {
    bcm_caladan3_mim_port_type_e_t mimType; /* only _BCM_CALADAN3_MIM_DEF_BACKBONE_PORT for now */
    dq_t                       plist;
} bcm_caladan3_mim_trunk_association_t;

/*
 *  This structure contains information necessary to map MIM GPORT ID to
 *  logical port ID.
 */
typedef struct {
    uint16 lpid[SBX_MAX_GPORTS];  /* map GPORT ID to logical port ID */
} _bcm_caladan3_mimgp_info_t;

typedef struct {
    /* service access points on this vpn - list of mim-ports excludes default back bone port*/
    dq_t                         vpn_access_sap_head;
    dq_t                         vpn_bbone_sap_head;
    dq_t                         def_bbone_plist;
    bcm_mim_vpn_t                vpnid;
    int                          isid;
    uint32                       lport;                  /* instrumentation */
    bcm_multicast_t              broadcast_group;        /* Broadcast group */
    bcm_multicast_t              unknown_unicast_group;  /* Unknown unicast group */
    bcm_multicast_t              unknown_multicast_group;/* Unknown multicast group */
    bcm_policer_t                policer_id;
} bcm_caladan3_mim_vpn_control_t;

typedef struct {
    bcm_caladan3_mim_vpn_control_t  *key;  /* search key */
    bcm_caladan3_mim_vpn_control_t **datum;
} _bcm_caladan3_mim_lookup_data_t;

typedef enum {
    _BCM_CALADAN3_MIM_PORT,
    _BCM_CALADAN3_MIM_STAG_1_1,
    _BCM_CALADAN3_MIM_STAG_BUNDLED,
    _BCM_CALADAN3_MIM_MAX_SUPPORTED,
    _BCM_CALADAN3_MIM_ITAG
} bcm_caladan3_mim_service_type_e_t;

typedef struct {
    uint32      lport;
    uint32      ftidx;    /* forwarding index */
    uint32      ohi;      /* global */
    uint32      ete;
    uint32      ismacidx; /* Ingress LSM index - applicable only for default back bone port*/
    uint32      esmacidx; /* Egress encapsulation smac idx */
    uint32      egrremarkidx; /* Egress remarking Index if applicable */
    uint32      egrremarkflags;
    /*bcm_failover_t failover_id;*/
} bcm_caladan3_mim_g3p1_hw_info_t;

typedef struct bcm_caladan3_mim_port_control_s {
    dq_t        listnode;
#define _MIM_PCB_TRUNK_NODE_POS (1)
    dq_t        trunklistnode;
    bcm_gport_t gport; /* virtual port  */
    bcm_gport_t port;  /* modport gport */
    bcm_mac_t   smac;  /* SMAC for back bone ports */
    bcm_mac_t   dmac;
    bcm_vlan_t  vlan;  /* back bone vlan, match vlan for access port */
    uint16      tpid;
    bcm_vlan_t  isidvlan; /* ISID - VSI */
    uint32      isid;
    bcm_policer_t policer_id;  /* Policer ID to be associated with the MiM */
    uint32      isidlport;  /* ISID logical port */
    bcm_caladan3_mim_g3p1_hw_info_t hwinfo;
    bcm_caladan3_mim_port_type_e_t type;
    bcm_mim_port_match_t criteria;  /* Match criteria. */
    uint32 flags;                   /* Configuration flags */
    bcm_caladan3_mim_service_type_e_t service; /* Applicable only for Access ports */
    uint32      refcount; /* used to track number of users on a mim default back bone port*/
}bcm_caladan3_mim_port_control_t;


extern int _bcm_caladan3_mim_policer_set(int unit,
                            bcm_gport_t port,
                            bcm_policer_t pol_id);
extern int _bcm_caladan3_mim_qosmap_set(int unit, bcm_gport_t port, 
                                        int ing_idx, int egr_idx,
                                        uint32 ingFlags, uint32 egrFlags);

extern int _bcm_caladan3_mim_qosmap_get(int unit, bcm_gport_t port, 
                                        int *ing_idx, int *egr_idx,
                                        uint32 *ing_flags, uint32 *egr_flags);

extern int _bcm_caladan3_mim_vpn_lookup_compare(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2,
                                          void *lkupdata);

#endif /* _BCM_INT_SBX_CALADAN3_MIM_H_ */

