/*
 * $Id: rx.h,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        rx.h
 * Purpose:     
 */

#ifndef   _BCM_INT_ROBO_RX_H_
#define   _BCM_INT_ROBO_RX_H_
#include <bcm_int/common/rx.h>

extern int bcm_rx_debug(int unit);

#define _BCM_ROBO_TB_RX_REASON_CFP_CPU_COPY         0x01

#define _BCM_ROBO_TB_RX_REASON_ARL_KNOWN_DA_FORWARD 0x02
#define _BCM_ROBO_TB_RX_REASON_ARL_UNKNOWN_DA_FLOOD 0x04
#define _BCM_ROBO_TB_RX_REASON_ARL_CONTROL_PKT      0x06
#define _BCM_ROBO_TB_RX_REASON_ARL_APPL_PKT         0x08
#define _BCM_ROBO_TB_RX_REASON_ARL_VLAN_FORWARD     0x0a
#define _BCM_ROBO_TB_RX_REASON_ARL_CFP_FORWARD      0x0c
#define _BCM_ROBO_TB_RX_REASON_ARL_CPU_LOOPBACK     0x0e
#define _BCM_ROBO_TB_RX_REASON_ARL_MASK             0x0e

#define _BCM_ROBO_TB_RX_REASON_MIRROR_COPY          0x10
#define _BCM_ROBO_TB_RX_REASON_INGRESS_SFLOW        0x20
#define _BCM_ROBO_TB_RX_REASON_EGRESS_SFLOW         0x40

#define _BCM_ROBO_TB_RX_REASON_SA_MOVE              0x80
#define _BCM_ROBO_TB_RX_REASON_SA_UNKNOWN           0x100
#define _BCM_ROBO_TB_RX_REASON_SA_OVERLIMIT         0x180
#define _BCM_ROBO_TB_RX_REASON_SA_MASK              0x180

#define _BCM_ROBO_TB_RX_REASON_VLAN_NONMEMBER       0x200
#define _BCM_ROBO_TB_RX_REASON_VLAN_UNKNOWN         0x400


#define BCM_ROBO_RX_REASON_MIRROR 0x1
                /* Mirroring */
#define BCM_ROBO_RX_REASON_SW_LEARN 0x2
                /* Software SA learning */
#define BCM_ROBO_RX_REASON_SWITCHING 0x4
                /* Normal switching */
#define BCM_ROBO_RX_REASON_PROTOCOL_TERMINATION 0x8
                /* Protocol Termination */
#define BCM_ROBO_RX_REASON_PROTOCOL_SNOOP 0x10 
                 /* Protocol Snooping */
#define BCM_ROBO_RX_REASON_EXCEPTION_FLOOD 0x20
                 /* Exception porcessing or flooding */


/*
 * DV and DCB related macros
 */
#define ROBO_DV_INFO(dv)          (((rx_dv_info_t *)(((eth_dv_t *)dv)->_DV_INFO)))
#define ROBO_DV_INDEX(dv)                 (ROBO_DV_INFO(dv)->idx)
#define ROBO_DV_STATE(dv)                 (ROBO_DV_INFO(dv)->state)
#define ROBO_DV_CHANNEL(dv)               (ROBO_DV_INFO(dv)->chan)
#define ROBO_DV_ABORT_CLEANUP(dv)         (ROBO_DV_INFO(dv)->abort_cleanup)

#define ROBO_DV_PKT(dv, i) \
    (RX_PKT(dv->dv_unit, ROBO_DV_CHANNEL(dv), ROBO_DV_INFO(dv)->idx, i))
#define ROBO_DV_PKTS_PROCESSED(dv)        (ROBO_DV_INFO(dv)->pkt_done_cnt)
#define ROBO_RX_DV(unit, chan, i)         (RX_CHAN_CTL(unit, chan).robo_dv[i])

extern int bcm_robo_rx_deinit(int unit);
#endif /* _BCM_INT_RX_H_ */
