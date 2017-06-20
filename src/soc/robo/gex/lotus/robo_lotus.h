/*
 * $Id: robo_lotus.h,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_LOTUS_H
#define _ROBO_LOTUS_H


#define DRV_LOTUS_MCAST_GROUP_NUM         256
#define DRV_LOTUS_AGE_TIMER_MAX           1048575
#define DRV_LOTUS_TRUNK_GROUP_NUM         2
#define DRV_LOTUS_TRUNK_MAX_PORT_NUM      4
#define DRV_LOTUS_MSTP_GROUP_NUM          8
#define DRV_LOTUS_SEC_MAC_NUM_PER_PORT    1
#define DRV_LOTUS_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_LOTUS_AUTH_SUPPORT_PBMP       0x0000003f
#define DRV_LOTUS_RATE_CONTROL_SUPPORT_PBMP 0x00000003f
#define DRV_LOTUS_VLAN_ENTRY_NUM  4095
#define DRV_LOTUS_BPDU_NUM    1
#define DRV_LOTUS_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED)

#define DRV_LOTUS_MAC_LOW_POWER_SUPPORT_PBMP 0x0000001f

#endif
