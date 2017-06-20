/*
 * $Id: mirror.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _BCM_INT_MIRROR_H
#define _BCM_INT_MIRROR_H
#include <shared/pbmp.h>
#include <shared/types.h>
#include <soc/macipadr.h>


#define _BCM_MIRROR_INGRESS_FROM_FP   0x100
/*
 * One entry for each SOC device containing mirroring 
 * information for that device.
 */
typedef struct mirror_cntl_s {
    int  mirr_enable;     /* is mirroring globally enabled ? */
    int  mirr_to_port;
    pbmp_t mirr_to_ports; 
    int  blk_none_mirr_traffic; 
    /* All traffic to MIRROR_CAPTURE_PORT will be blocked 
       except mirror traffic.
     */
    uint16  ingress_mirr_ctrl;
    /* bit[15:14] - filter mode. 
       00:Mirror all ingress frames, 
       01:Mirror all received frames with DA=IN_MIRROR_MAC
       10:Mirror all received frames with SA=IN_MIRROR_MAC
       bit13 - Ingress Divider Enable
       bit[12:0] - Ingress Mirror Port Mask 
     */
    uint16  in_mirr_div; /* bit[15:10] - Reserved, bit[9:0] - in_mirr_div */
    uint16  egress_mirr_ctrl;  
    /* bit[15:14] - filter mode.
       00: Mirror all ingress frames
       01: Mirror all received frames with DA = IN_MIRROR_MAC
       10: Mirror all received frames with SA= IN)_MIRROR_MAC
       bit13 - egress Divider Enable
       bit[12:0] - egress Mirror Port Mask 
     */
    uint16  en_mirr_div; /* bit[15:10] - Reserved, bit[9:0] - en_mirr_div */
    sal_mac_addr_t ingress_mac;
    sal_mac_addr_t egress_mac;    
} mirror_cntl_t;


extern int bcm_robo_dirty_mirror_mode_set(int unit, mirror_cntl_t *mirror_ctrl);
extern int bcm_robo_dirty_mirror_mode_get(int unit, mirror_cntl_t *mirror_ctrl);
extern int mirrEn5380;

extern int bcm_robo_mirror_deinit(int unit);
extern int _bcm_robo_mirror_fp_dest_add(int unit, int flags, int port);
extern int _bcm_robo_mirror_fp_dest_delete(int unit, int flags, int port);
extern int _bcm_robo_mirror_to_port_check(int unit, bcm_pbmp_t mport_pbmp, 
    bcm_pbmp_t igr_pbmp, bcm_pbmp_t egr_pbmp);
#endif
