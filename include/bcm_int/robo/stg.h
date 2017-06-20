/*
 * $Id: stg.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains STG definitions internal to the BCM library.
 */
#ifndef _BCM_INT_STG_H
#define _BCM_INT_STG_H
#define PVP_STP_DISABLED     0   /* Disabled */
#define PVP_STP_BLOCKING     1   /* Blocking/Listening */
#define PVP_STP_LEARNING     2   /* Learning */
#define PVP_STP_FORWARDING   3   /* Forwarding */
extern int _bcm_robo_stg_stp_translate(int unit, int bcm_state);
extern int _bcm_robo_stg_pvp_translate(int unit, int pvp_state);
extern int _bcm_robo_stg_valid(int unit, bcm_stg_t stg);
extern int _bcm_robo_stg_vlan_destroy(int unit, bcm_stg_t stg, bcm_vlan_t vid);
extern int bcm_robo_stg_detach(int unit);

/* MSTP mask support */
#define _BCM_STG_STP_TO_MSTP_MASK_ENABLE(stp_state) \
        (((stp_state) != BCM_STG_STP_DISABLE) && \
        ((stp_state) != BCM_STG_STP_BLOCK))

#endif
