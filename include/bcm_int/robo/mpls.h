/*
* $Id:mpls.h,v 1.1 2013/6/28 13:05:00 Jianping Exp $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file contains the mpls table 
*/
#ifndef _BCM_INT_ROBO_MPLS_H
#define _BCM_INT_ROBO_MPLS_H

typedef struct _bcm_robo_mpls_switch_entry_s {
    bcm_mpls_label_t            ing_vp_mpls_label;/* for the key , it is the 20bits label*/
    bcm_mpls_label_t            egr_vp_mpls_label;/* for swap, it is the 32bits including label,S exp,TTL*/
    bcm_port_t                  ing_port_id;
    bcm_mpls_switch_action_t    action;           /* POP or SWAP*/
    int32                       egr_intf_index;   /* valid for SWAP action */
    bcm_vpn_t                   vpn;              /* valid for POP action */
    struct _bcm_robo_mpls_switch_entry_s *p_pre;
    struct _bcm_robo_mpls_switch_entry_s *p_next;
} _bcm_robo_mpls_switch_entry_t;

typedef enum _mpls_port_type_e{
    MPLS_PORT_TYPE_UNI,
    MPLS_PORT_TYPE_NNI
} _mpls_port_type_t;

typedef struct _bcm_robo_mpls_port_s {
    int32                       port_index;
    bcm_port_t                  port_id;
    bcm_vlan_t                  vlan_id;
    bcm_mpls_label_t            match_vc_mpls_label;/* 20bits label,valid for NNI port*/
    bcm_mpls_label_t            egr_vc_mpls_label;  /* 32 bits including label,S,exp and TTL*/
    _mpls_port_type_t           port_type;
    bcm_mpls_port_match_t       criteria;
    int32                       egr_intf_index;
    struct _bcm_robo_mpls_port_s *p_pre;
    struct _bcm_robo_mpls_port_s *p_next;
} _bcm_robo_mpls_port_t;

typedef struct _bcm_robo_mpls_vpws_vpn_s {
    bcm_vpn_t                   vpn;
    int32                       uni_port_index;
    int32                       nni_port_index;
    struct _bcm_robo_mpls_vpws_vpn_s *p_pre;
    struct _bcm_robo_mpls_vpws_vpn_s *p_next;
}_bcm_robo_mpls_vpws_vpn_t;

#define _MPLS_ROBO_MAX_ASSCO_DATA 6
#define _MPLS_ROBO_MAX_KEY_LEN  8
#define _MPLS_ROBO_MAX_ENCAP_LEN 32
typedef struct _bcm_robo_mpls_lookup_entry_s {
    int32                       uni_port_index;
    int32                       nni_port_index;
    int32                       l3_intf_index;
    bcm_mpls_label_t            vp_mpls_label;
    bcm_port_t                  port;
    bcm_vpn_t                   vpn;
    uint8                       key[_MPLS_ROBO_MAX_KEY_LEN];     /* lookup key */
    uint32                      idx;					   
    uint8                       data[_MPLS_ROBO_MAX_ASSCO_DATA]; /* associcate data */
    uint8                       encap[_MPLS_ROBO_MAX_ENCAP_LEN];
    struct _bcm_robo_mpls_lookup_entry_s *p_pre;
    struct _bcm_robo_mpls_lookup_entry_s *p_next;
}_bcm_robo_mpls_lookup_entry_t;

#endif

