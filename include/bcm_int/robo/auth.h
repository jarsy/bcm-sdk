/*
 * $Id: auth.h,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains Auth definitions internal to the BCM library.
 */
#ifndef _BCM_INT_AUTH_H
#define _BCM_INT_AUTH_H

#include <soc/drv.h>
#define BCM_AUTH_SEC_NONE           0x00010000
#define BCM_AUTH_SEC_STATIC_ACCEPT  0x00020000
#define BCM_AUTH_SEC_STATIC_REJECT  0x00040000
#define BCM_AUTH_SEC_SA_NUM         0x00080000
#define BCM_AUTH_SEC_SA_MATCH       0x00100000
#define BCM_AUTH_SEC_SA_MOVEMENT    0x00200000  /* The SA of a received packet 
                                                                                         matches an ARL entry with the 
                                                                                           associated port/LAG different 
                                                                                           from the corresponding ingress 
                                                                                           port/LAG.*/
#define BCM_AUTH_SEC_EXTEND_MODE    0x00400000
#define BCM_AUTH_SEC_SIMPLIFY_MODE  0x00800000

#ifdef BCM_TB_SUPPORT
#define BCM_AUTH_SEC_SA_OVERLIMIT_DROP    BCM_AUTH_SEC_SA_NUM | \
                                                                              BCM_AUTH_SEC_EXTEND_MODE
#define BCM_AUTH_SEC_SA_OVERLIMIT_CPUCOPY    BCM_AUTH_SEC_SA_NUM | \
                                                                                    BCM_AUTH_SEC_SIMPLIFY_MODE
#define BCM_AUTH_SEC_SA_UNKNOWN_DROP    BCM_AUTH_SEC_SA_MATCH | \
                                                                             BCM_AUTH_SEC_EXTEND_MODE
#define BCM_AUTH_SEC_SA_UNKNOWN_CPUCOPY    BCM_AUTH_SEC_SA_MATCH | \
                                                                                   BCM_AUTH_SEC_SIMPLIFY_MODE
#define BCM_AUTH_SEC_SA_MOVEMENT_DROP    BCM_AUTH_SEC_SA_MOVEMENT | \
                                                                              BCM_AUTH_SEC_EXTEND_MODE
#define BCM_AUTH_SEC_SA_MOVEMENT_CPUCOPY    BCM_AUTH_SEC_SA_MOVEMENT | \
                                                                                    BCM_AUTH_SEC_SIMPLIFY_MODE
#endif /* BCM_TB_SUPPORT */

/* 
  * To indicate EAP PDU CPUCopy is disable (drop EAP packet) 
  * 0 : Drop EAP packet.
  * 1 : EAP PDU CPUCopy is enabled.
  */
#define BCM_AUTH_SEC_RX_EAP_DROP  0x01000000

#define STATIC_MAC_WRITE 0  /* For Static Mac Security Write operateion */
#define STATIC_MAC_READ  1  /* For Static Mac Security Read operateion */

#define MAX_SEC_MAC	16 /* Sec Macs per port */


extern int bcm_robo_auth_sec_mac_add(int unit, 
                bcm_port_t port, bcm_mac_t mac);
extern int bcm_robo_auth_sec_mac_delete(int unit, 
                bcm_port_t port, bcm_mac_t mac);
extern int bcm_robo_auth_sec_mac_get(int unit, 
                bcm_port_t port,bcm_mac_t *mac, int *num);
extern int bcm_robo_auth_sec_mode_set(int unit, 
                bcm_port_t port, int mode,int mac_num);
extern int bcm_robo_auth_sec_mode_get(int unit, 
                bcm_port_t port, int *mode, int *mac_num);
extern int 
_bcm_robo_auth_sec_mode_set(int unit, bcm_port_t port, int mode,
                 int mac_num);


#endif
