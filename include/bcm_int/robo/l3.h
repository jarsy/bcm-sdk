/*
* $Id:l3.h,v 1.1 2013/6/28 09:05:00 Jianping Exp $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* This file contains the l3 intf and egress intf defintions for the MPLS operation 
*/

#ifndef _BCM_INT_ROBO_L3_H
#define _BCM_INT_ROBO_L3_H

#include <bcm/types.h>

typedef struct _bcm_robo_l3_entry_s{
    int32               l3_intf_index;
    bcm_mac_t           src_mac;
    bcm_mac_t           dst_mac;
    bcm_port_t          egr_port;
    bcm_vlan_t          vlan;
    bcm_mpls_label_t    vp_mpls_label;
    struct _bcm_robo_l3_entry_s *p_pre;
    struct _bcm_robo_l3_entry_s *p_next;
} _bcm_robo_l3_entry_t;


#endif



