/*
 * $Id: bcm-knet.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef __LINUX_BCM_KNET_H__
#define __LINUX_BCM_KNET_H__

#ifndef __KERNEL__
#include <stdint.h>
#endif

typedef struct  {
    int rc;
    int len;
    int bufsz;
    int reserved;
    uint64_t buf;
} bkn_ioctl_t;

#ifdef __KERNEL__

/*
 * Call-back interfaces for other Linux kernel drivers.
 */
#include <linux/skbuff.h>

typedef struct sk_buff *
(*knet_skb_cb_f)(struct sk_buff *skb, int dev_no, void *meta);

typedef int
(*knet_filter_cb_f)(uint8_t *pkt, int size, int dev_no, void *meta,
                    int chan, kcom_filter_t *filter);

extern int
bkn_rx_skb_cb_register(knet_skb_cb_f rx_cb);

extern int
bkn_rx_skb_cb_unregister(knet_skb_cb_f rx_cb);

extern int
bkn_tx_skb_cb_register(knet_skb_cb_f tx_cb);

extern int
bkn_tx_skb_cb_unregister(knet_skb_cb_f tx_cb);

extern int
bkn_filter_cb_register(knet_filter_cb_f filter_cb);

extern int
bkn_filter_cb_unregister(knet_filter_cb_f filter_cb);

#endif

#endif /* __LINUX_BCM_KNET_H__ */
