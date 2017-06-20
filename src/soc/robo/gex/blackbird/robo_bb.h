/*
 * $Id: robo_53118.h,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_BB_H
#define _ROBO_BB_H


#define DRV_BB_MCAST_GROUP_NUM         256
#define DRV_BB_AGE_TIMER_MAX           1048575
#define DRV_BB_TRUNK_GROUP_NUM         2
#define DRV_BB_TRUNK_MAX_PORT_NUM      4
#define DRV_BB_MSTP_GROUP_NUM          8
#define DRV_BB_SEC_MAC_NUM_PER_PORT    1   /* only one EAP_DA/port allowed */
#define DRV_BB_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_BB_AUTH_SUPPORT_PBMP       0x000001ff
#define DRV_BB_RATE_CONTROL_SUPPORT_PBMP 0x0000001ff
#define DRV_BB_VLAN_ENTRY_NUM  4095
#define DRV_BB_BPDU_NUM    1
#define DRV_BB_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED)

#endif
