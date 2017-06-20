/*
 * $Id: robo_53128.h,v 1.5 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_BB2_H
#define _ROBO_BB2_H


#define DRV_BB2_MCAST_GROUP_NUM         256
#define DRV_BB2_AGE_TIMER_MAX           1048575
#define DRV_BB2_TRUNK_GROUP_NUM         2
#define DRV_BB2_TRUNK_MAX_PORT_NUM      4
#define DRV_BB2_MSTP_GROUP_NUM          8
#define DRV_BB2_SEC_MAC_NUM_PER_PORT    1   /* only one EAP_DA/port allowed */
#define DRV_BB2_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_BB2_AUTH_SUPPORT_PBMP       0x000001ff
#define DRV_BB2_RATE_CONTROL_SUPPORT_PBMP 0x0000001ff
#define DRV_BB2_VLAN_ENTRY_NUM  4095
#define DRV_BB2_BPDU_NUM    1
#define DRV_BB2_CFP_TCAM_SIZE 16
#define DRV_BB2_CFP_UDFS_NUM 0
#define DRV_BB2_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED)

#define DRV_BB2_MAC_LOW_POWER_SUPPORT_PBMP 0x000000ff                                        


#endif
