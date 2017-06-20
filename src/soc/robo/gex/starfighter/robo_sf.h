/*
 * $Id: robo_sf.h,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_SF_H
#define _ROBO_SF_H


#define DRV_SF_MCAST_GROUP_NUM         128
#define DRV_SF_AGE_TIMER_MAX           1048575
#define DRV_SF_TRUNK_GROUP_NUM         2
#define DRV_SF_TRUNK_MAX_PORT_NUM      4
#define DRV_SF_MSTP_GROUP_NUM          8
#define DRV_SF_SEC_MAC_NUM_PER_PORT    1
#define DRV_SF_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_SF_AUTH_SUPPORT_PBMP       0x0000003f
#define DRV_SF_RATE_CONTROL_SUPPORT_PBMP 0x00000003f
#define DRV_SF_VLAN_ENTRY_NUM  4095
#define DRV_SF_BPDU_NUM    1
#define DRV_SF_CFP_TCAM_SIZE 256
#define DRV_SF_CFP_UDFS_NUM 93
#define DRV_SF_CFP_UDFS_OFFSET_MAX (2 * (32 - 1))
#define DRV_SF_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED)
                                        
#define DRV_SF_MAC_LOW_POWER_SUPPORT_PBMP 0x0000001f                                        

#endif
