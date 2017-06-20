/*
 * $Id: robo_dino8.h,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_DINO8_H
#define _ROBO_DINO8_H


#define DRV_DINO8_MCAST_GROUP_NUM         256
#define DRV_DINO8_AGE_TIMER_MAX           1048575
#define DRV_DINO8_TRUNK_GROUP_NUM         4
#define DRV_DINO8_TRUNK_MAX_PORT_NUM      2
#define DRV_DINO8_MSTP_GROUP_NUM          1
#define DRV_DINO8_SEC_MAC_NUM_PER_PORT    24   /* only one EAP_DA/port allowed */
#define DRV_DINO8_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_DINO8_AUTH_SUPPORT_PBMP       0x00000000
#define DRV_DINO8_RATE_CONTROL_SUPPORT_PBMP 0x0000000ff
#define DRV_DINO8_VLAN_ENTRY_NUM  4095
#define DRV_DINO8_BPDU_NUM    1
#define DRV_DINO8_AUTH_SEC_MODE DRV_SECURITY_VIOLATION_NONE

/* ------ DINO8 Storm Control related definition ------- */
#define DINO8_STORM_SUPPRESSION_DLF_MASK    0x20
#define DINO8_STORM_SUPPRESSION_BPDU_MASK   0x10
#define DINO8_STORM_SUPPRESSION_BROADCAST_MASK  0x0c
#define DINO8_STORM_SUPPRESSION_MULTICAST_MASK  0x02
#define DINO8_STORM_SUPPRESSION_UNICAST_MASK    0x01

#define DINO8_STORM_CONTROL_PKT_MASK (DRV_STORM_CONTROL_BCAST | \
                                        DRV_STORM_CONTROL_MCAST | \
                                        DRV_STORM_CONTROL_DLF)
#endif
