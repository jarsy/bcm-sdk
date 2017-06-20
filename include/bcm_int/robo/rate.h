/*
 * $Id: rate.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _BCM_INT_RATE_H
#define _BCM_INT_RATE_H
/*
 * Rate flags
 */
#define BCM_RATE_UCAST      0x08
#define BCM_RATE_BCAST_L0   0x10
#define BCM_RATE_BCAST_L1   0x20
#define BCM_RATE_MCF        0x40

#define BCM_INGRESS_RATE    0x80
#define BCM_EGRESS_RATE     0x100

/*
 * Rate type flags
 */
#define RATE_FIXED          0x01
#define RATE_RATION         0x02

extern int bcm_robo_rate_limit_enable_set(int unit, bcm_port_t port, 
                                     int enable, int flags);
extern int bcm_robo_rate_limit_packet_type_set(int unit, int bucket_num, 
                                          int pkt_type);
extern int bcm_robo_rate_limit_packet_type_get(int unit, int bucket_num, 
                                          int *pkt_type);
extern int bcm_robo_rate_limit_drop_enable_set(int unit, int bucket_num, 
                                          int enable);
extern int _bcm_robo_rate_valid_pbmp_check(int unit, int port);

extern int _bcm_robo_rate_init(int unit);

#endif
