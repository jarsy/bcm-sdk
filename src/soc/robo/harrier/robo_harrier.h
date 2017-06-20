/*
 * $Id: robo_harrier.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_HARRIER_H
#define _ROBO_HARRIER_H


#define DRV_HARRIER_MCAST_GROUP_NUM         4096
#define DRV_HARRIER_AGE_TIMER_MAX               1048575
#define DRV_HARRIER_TRUNK_GROUP_NUM         14
#define DRV_HARRIER_TRUNK_MAX_PORT_NUM      8
#define DRV_HARRIER_MSTP_GROUP_NUM          256
#define DRV_HARRIER_SEC_MAC_NUM_PER_PORT    1   /* only one EAP_DA/port allowed */
#define DRV_HARRIER_COS_QUEUE_MAX_WEIGHT_VALUE  55
#define DRV_HARRIER_AUTH_SUPPORT_PBMP_BCM53242 0x06ffffff
#define DRV_HARRIER_AUTH_SUPPORT_PBMP_BCM53262 0x1effffff
#define DRV_HARRIER_RATE_CONTROL_SUPPORT_PBMP_BCM53242 0x07ffffff
#define DRV_HARRIER_RATE_CONTROL_SUPPORT_PBMP_BCM53262 0x1fffffff
#define DRV_HARRIER_VLAN_ENTRY_NUM  4095
#define DRV_HARRIER_BPDU_NUM    3
#define DRV_HARRIER_CFP_TCAM_SIZE 1024
#define DRV_HARRIER_CFP_UDFS_NUM 25
#define DRV_HARRIER_CFP_UDFS_OFFSET_MAX (2 * (64 - 1))
#define DRV_HARRIER_CFP_RNG_NUM 4
#define DRV_HARRIER_CFP_VID_RNG_NUM 4
#define DRV_HARRIER_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE |\
                                        DRV_SECURITY_EAP_MODE_EXTEND |\
                                        DRV_SECURITY_EAP_MODE_SIMPLIFIED |\
                                        DRV_SECURITY_VIOLATION_SA_NUM |\
                                        DRV_SECURITY_VIOLATION_SA_MATCH )

/* ------ HARRIER Ingress/Egress Rate Control related definition ------- */
#define HARRIER_IRC_PKT_MASK  0x7f

#define HARRIER_RATE_MAX_REF_CNTS     0x3fff
#define HARRIER_RATE_MAX_BUCKET_SIZE  0x7f

#define HARRIER_RATE_REF_UNIT_64K  0
#define HARRIER_RATE_REF_UNIT_1M   1

#define HARRIER_RATE_METER_MAX  16383000  /* (HARRIER_RATE_MAX_REF_CNTS*1000) */
#define HARRIER_RATE_METER_MIN  0
#define HARRIER_RATE_BURST_MAX  8128  /* (1016Kbyte * 8) */
#define HARRIER_RATE_BURST_MIN  0

#define HARRIER_RATE_MAX_LIMIT_KBPS_FE  100000
#define HARRIER_RATE_MAX_LIMIT_KBPS_GE  1000000

/* ------ HARRIER Storm Control related definition ------- */
#define HARRIER_STORM_SUPPRESSION_UNKNOW_SA_MASK  0x40
#define HARRIER_STORM_SUPPRESSION_DLF_MASK        0x20
#define HARRIER_STORM_SUPPRESSION_MLF_MASK        0x10
#define HARRIER_STORM_SUPPRESSION_BROADCAST_MASK  0x08
#define HARRIER_STORM_SUPPRESSION_BPDU_MASK       0x04
#define HARRIER_STORM_SUPPRESSION_MULTICAST_MASK  0x02
#define HARRIER_STORM_SUPPRESSION_UNICAST_MASK    0x01

#define HARRIER_STORM_CONTROL_PKT_MASK (DRV_STORM_CONTROL_BCAST | \
                                        DRV_STORM_CONTROL_MCAST | \
                                        DRV_STORM_CONTROL_DLF | \
                                        DRV_STORM_CONTROL_SALF | \
                                        DRV_STORM_CONTROL_RSV_MCAST | \
                                        DRV_STORM_CONTROL_UCAST)

#endif /* !_ROBO_HARRIER_H */
