/*
 * $Id: sbx_rx.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * == sbx_rx.h - SBX Packet Receive helper functions ==
 */

#ifndef _SBX_RX_H_
#define _SBX_RX_H_

#include <bcm/pkt.h>

/*
 * Public Functions
 *
 */

int sbxpkt_rx_sync_start(int unit);
int sbxpkt_rx_sync(int unit, bcm_pkt_t *pkt, int timeout);
int sbxpkt_rxs_sync(int unit, bcm_pkt_t **pkt, int timeout, int *num_rx);
int sbxpkt_rx_sync_stop(int unit);
int sbxpkt_rx_sync_set_priority(int pri);
int sbxpkt_rx_sync_queue_size(int size);
int  sbxpkt_data_memget(bcm_pkt_t *pkt, int idx, uint8 *dst, int len);
void sbxpkt_data_clear(bcm_pkt_t *pkt);
void sbxpkt_one_buf_setup(bcm_pkt_t *pkt, unsigned char *buf, int len);
void sbxpkt_one_buf_free(bcm_pkt_t *pkt);

#endif /* _SBX_RX_H_ */

