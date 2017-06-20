/*
 * $Id: types.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifndef _BCM_INT_TYPES_H
#define _BCM_INT_TYPES_H

/*
 * This structure should expose only as much of the higig
 * header as necessary above the bcm level.  It should be
 * usable for tx and rx.
 */
typedef struct bcm_higig_info_s {
    int        hg_dmod;         /* Valid for TX only */
    int        hg_dport;        /* Valid for TX only */
    int        hg_smod;         /* Valid for RX only  */
    int        hg_sport;        /* Valid for RX only  */
    int        hg_opcode;       /* Header opcode */
    int        hg_cos;          /* COS from HG hdr */
    int        hg_ctag;         /* Valid for RX only */
    int        hg_pri;          /* VLAN priority (0-7) */
    int        hg_cfi;          /* VLAN CFI bit */
    int        hg_vid;          /* VLAN ID */
} bcm_higig_info_t;
#endif
