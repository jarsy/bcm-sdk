/*
 * $Id: robo_northstarplus.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
 
#ifndef _ROBO_NORTHSTARPLUS_H
#define _ROBO_NORTHSTARPLUS_H


#define DRV_NORTHSTARPLUS_MCAST_GROUP_NUM         128
#define DRV_NORTHSTARPLUS_AGE_TIMER_MAX           1048575
#define DRV_NORTHSTARPLUS_TRUNK_GROUP_NUM         2
#define DRV_NORTHSTARPLUS_TRUNK_MAX_PORT_NUM      4
#define DRV_NORTHSTARPLUS_COS_QUEUE_NUM           8
#define DRV_NORTHSTARPLUS_MSTP_GROUP_NUM          8
#define DRV_NORTHSTARPLUS_SEC_MAC_NUM_PER_PORT    1
#define DRV_NORTHSTARPLUS_COS_QUEUE_MAX_WEIGHT_VALUE  255
#define DRV_NORTHSTARPLUS_AUTH_SUPPORT_PBMP       0x000000bf
#define DRV_NORTHSTARPLUS_RATE_CONTROL_SUPPORT_PBMP 0x0000000bf
#define DRV_NORTHSTARPLUS_VLAN_ENTRY_NUM  4095
#define DRV_NORTHSTARPLUS_BPDU_NUM    1
#define DRV_NORTHSTARPLUS_CFP_TCAM_SIZE 256
#define DRV_NORTHSTARPLUS_CFP_UDFS_NUM 93
#define DRV_NORTHSTARPLUS_CFP_UDFS_OFFSET_MAX (2 * (32 - 1))
#define DRV_NORTHSTARPLUS_AUTH_SEC_MODE (DRV_SECURITY_VIOLATION_NONE | \
                                     DRV_SECURITY_EAP_MODE_EXTEND | \
                                     DRV_SECURITY_EAP_MODE_SIMPLIFIED)
                                        
#define DRV_NORTHSTARPLUS_MAC_LOW_POWER_SUPPORT_PBMP 0x00000000
#define DRV_NORTHSTARPLUS_WRED_NUM                 (16)
#define DRV_NORTHSTARPLUS_WRED_MAX_AQD_PERIOD      (150) /* 150 us */
#define DRV_NORTHSTARPLUS_WRED_AQD_PERIOD_UNIT     (10) /* unit : 10 us */
#define DRV_NORTHSTARPLUS_CELL_UNIT                (256) /* unit : 256 bytes */
#define DRV_NORTHSTARPLUS_TC_MAX                   (7)
/* port4 and port5 within MACSEC attached */
#define DRV_NORTHSTARPLUS_MACSEC_PBMP           (0x30)  


/* SA learn limit */
#define DRV_NSP_ARL_ENTRY_NUM           (4096)
#define DRV_NSP_SYS_MAX_LEARN_LIMIT     DRV_NSP_ARL_ENTRY_NUM
#define DRV_NSP_PORT_MAX_LEARN_LIMIT    DRV_NSP_ARL_ENTRY_NUM

/* DEI and PCP format */
#define DRV_NSP_TAG_PRIORITY_MASK   (0x0F)
#define _DRV_NSP_TAGPRI_DEI_SHIFT   (3)
#define _DRV_NSP_TAGPRI_PCP_SHIFT   (0)
#define _DRV_NSP_TAGPRI_DEI_MASK    (0x1)
#define _DRV_NSP_TAGPRI_PCP_MASK    (0x7)
#define DRV_NSP_TAG_PRIORITY_DEI_SET(pri, dei)   \
        (pri) |= ((((dei) & _DRV_NSP_TAGPRI_DEI_MASK) << \
        _DRV_NSP_TAGPRI_DEI_SHIFT) & DRV_NSP_TAG_PRIORITY_MASK)  
#define DRV_NSP_TAG_PRIORITY_PCP_SET(pri, pcp)   \
        (pri) |= ((((pcp) & _DRV_NSP_TAGPRI_PCP_MASK) << \
        _DRV_NSP_TAGPRI_PCP_SHIFT) & DRV_NSP_TAG_PRIORITY_MASK)  
#define DRV_NSP_TAG_PRIORITY_DEI_GET(pri, dei)   \
        (dei) = ((((pri) & DRV_NSP_TAG_PRIORITY_MASK) >>  \
        _DRV_NSP_TAGPRI_DEI_SHIFT) & _DRV_NSP_TAGPRI_DEI_MASK)
#define DRV_NSP_TAG_PRIORITY_PCP_GET(pri, pcp)   \
        (pcp) = ((((pri) & DRV_NSP_TAG_PRIORITY_MASK) >>  \
        _DRV_NSP_TAGPRI_PCP_SHIFT) & _DRV_NSP_TAGPRI_PCP_MASK)

/* Egress rate */
/* kbps */
#define NSP_RATE_REFRESH_GRANULARITY  64

/* kbps */
#define NSP_RATE_MAX_REFRESH_RATE  (1000 * 1000)

/* byte */
#define NSP_RATE_BUCKET_UNIT_SIZE  64

/* bucket unit */
#define NSP_RATE_MAX_BUCKET_UNIT  0x3FFFF

/* max burst size (bytes)  */
#define NSP_RATE_MAX_BUCKET_SIZE \
    (NSP_RATE_MAX_BUCKET_UNIT * NSP_RATE_BUCKET_UNIT_SIZE)

/* unit of packet-based */
#define NSP_RATE_PACKET_BASED_REFRESH_UNIT           (125)  /* 125 pps */
#define NSP_RATE_PACKET_BASED_MAX_REFRESH_VALUE      (0x1FFFF)
#define NSP_RATE_PACKET_BASED_BUCKET_UNIT            (1)  /* 1 packet */
#define NSP_RATE_PACKET_BASED_MAX_BUCKET_VALUE       (0x1FFFF)


/* The Pause off threshold of Ingress rate control */
#define NSP_INGRESS_RATE_PAUSE_ON_THRESHOLD_KBYTES     (12) /* 12K bytes */


/* P5/P4 MUX REGISTER */
#define P5_MUX_REG_ADDR                                 (0x3f308)
#define P4_MUX_REG_ADDR                                 (0x3f30c)
#define MUX_REG_DUPLEX_FIELD_SHIFT                      (9)
#define MUX_REG_DUPLEX_FIELD_MASK                       (0x1)
#define MUX_REG_LINK_FIELD_SHIFT                        (8)
#define MUX_REG_LINK_FIELD_MASK                         (0x1)
#define MUX_REG_RX_PAUSE_FIELD_SHIFT                    (7)
#define MUX_REG_RX_PAUSE_FIELD_MASK                     (0x1)
#define MUX_REG_TX_PAUSE_FIELD_SHIFT                    (6)
#define MUX_REG_TX_PAUSE_FIELD_MASK                     (0x1)
#define MUX_REG_SPEED_FIELD_SHIFT                       (3)
#define MUX_REG_SPEED_FIELD_MASK                        (0x7)
#define MUX_REG_SPEED_1000M                             (0x2)
#define MUX_REG_SPEED_100M                              (0x1)
#define MUX_REG_SPEED_10M                               (0x0)



#endif
