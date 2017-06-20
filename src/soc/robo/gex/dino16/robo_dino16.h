/*
 * $Id: $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_DINO16_H
#define _ROBO_DINO16_H


#define DRV_DINO16_MCAST_GROUP_NUM         256
#define DRV_DINO16_AGE_TIMER_MAX           1048575
#define DRV_DINO16_TRUNK_GROUP_NUM         4
#define DRV_DINO16_TRUNK_MAX_PORT_NUM      8
#define DRV_DINO16_MSTP_GROUP_NUM          32
#define DRV_DINO16_SEC_MAC_NUM_PER_PORT    256
#define DRV_DINO16_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_DINO16_AUTH_SUPPORT_PBMP       0x0000ffff
#define DRV_DINO16_RATE_CONTROL_SUPPORT_PBMP 0x0000ffff
#define DRV_DINO16_VLAN_ENTRY_NUM  4095
#define DRV_DINO16_BPDU_NUM    1
#define DRV_DINO16_AUTH_SEC_MODE DRV_SECURITY_VIOLATION_NONE

/* ------ DINO16 Storm Control related definition ------- */
#define DINO16_STORM_SUPPRESSION_DLF_MASK    0x20
#define DINO16_STORM_SUPPRESSION_BPDU_MASK   0x10
#define DINO16_STORM_SUPPRESSION_BROADCAST_MASK  0x0c
#define DINO16_STORM_SUPPRESSION_MULTICAST_MASK  0x02
#define DINO16_STORM_SUPPRESSION_UNICAST_MASK    0x01

#define DINO16_STORM_CONTROL_PKT_MASK (DRV_STORM_CONTROL_BCAST | \
                                        DRV_STORM_CONTROL_MCAST | \
                                        DRV_STORM_CONTROL_DLF)
#endif
