/*
 * $Id: rx.h,v 1.7 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx.h
 * Purpose:     Internal structures and definitions for RX module
 */

#ifndef   _BCM_INT_SBX_RX_H_
#define   _BCM_INT_SBX_RX_H_

#include <sal/appl/sal.h>
#include <bcm/error.h>
#include <bcm/rx.h>
#include <bcm_int/common/rx.h>

/* Some default values */
#define SBX_RX_PKT_SIZE_DFLT       (16 * 1024)   /* 16 KB packets 
                                              * 256 B, 512, 1Kb, 2, 4, 8 KB valid*/
#define SBX_RX_PPC_DFLT            32            /* pkts/chain */
#define SBX_RX_PPS_DFLT            1000          /* 1000 pkts/sec */
#define SBX_RX_THREAD_PRI_DFLT     200

/* device specific configuration routines */
int sbx_qe2000_rx_config(int unit, bcm_rx_cfg_t *cfg);
int sbx_fe2000_rx_config(int unit, bcm_rx_cfg_t *cfg);

int _bcm_to_sbx_reasons(int unit,
                        bcm_rx_reason_t rx_reason, 
                        uint32 *sb_reasons,
                        int *reason_count);

#endif  /* _BCM_INT_SBX_RX_H_ */
