/*
 * $Id: bcm-robo-knet.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */


 /*
  * This module implements a Linux network driver for Broadcom
  * ROBO switch devices. The driver simultaneously serves a
  * number of vitual Linux network devices and a Tx/Rx API
  * implemented in user space.
  *
  * Packets received from the switch device are sent to either
  * a virtual Linux network device or the user mode Rx API
  * based on a set of packet filters.
  *
  * Packets from the virtual Linux network devices and the user
  * mode Tx API are multiplexed with priority given to the Tx API.
  *
  * A message-based IOCTL interface is used for managing packet
  * filters and virtual Linux network interfaces.
  *
  *
  * For a list of supported module parameters, please see below.
  */

#include <gmodule.h> /* Must be included first */
#include <linux-bde.h>
#include <kcom.h>
#include <bcm-knet.h>
  
#include <linux/pci.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/seq_file.h>
  
#include "hnddma.h"

#include <soc/gmac0_core.h>
#include <soc/ethdma.h>
#include <soc/robo/robo_drv.h>

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("Network Device Driver for Broadcom BCM TxRx API");
MODULE_LICENSE("Proprietary");


static int debug;
LKM_MOD_PARAM(debug, "i", int, 0);
MODULE_PARM_DESC(debug,
"Debug level (default 0)");

static char *mac_addr = NULL;
LKM_MOD_PARAM(mac_addr, "s", charp, 0);
MODULE_PARM_DESC(mac_addr,
"Ethernet MAC address (default 02:10:18:xx:xx:xx)");

static int rx_buffer_size = 9216;
LKM_MOD_PARAM(rx_buffer_size, "i", int, 0);
MODULE_PARM_DESC(rx_buffer_size,
"Size of RX packet buffers for skb mode (default 9216)");

static char *base_dev_name = NULL;
LKM_MOD_PARAM(base_dev_name, "s", charp, 0);
MODULE_PARM_DESC(base_dev_name,
"Base device name (default bcm0, bcm1, etc.)");

static int use_rx_skb_chan = 0;
LKM_MOD_PARAM(use_rx_skb_chan, "i", int, 0);
MODULE_PARM_DESC(use_rx_skb_chan,
"Use socket buffers for receive operation in chan x in bitmap form. \
use_rx_skb_chan=0x1 means socket buffers for rx deploy in chan 0.\
Defaul 0: no chan deploy skb mode.");

static int use_tx_skb = 1;
LKM_MOD_PARAM(use_tx_skb, "i", int, 0);
MODULE_PARM_DESC(use_tx_skb,
"Netif tx operates in chan 1 if use_tx_skb=1");

static int tx_chan_inuse = 0;

/* Debug levels */
#define DBG_LVL_VERB        0x1
#define DBG_LVL_DCB         0x2
#define DBG_LVL_PKT         0x4
#define DBG_LVL_SKB         0x8
#define DBG_LVL_CMD         0x10
#define DBG_LVL_EVT         0x20
#define DBG_LVL_IRQ         0x40
#define DBG_LVL_NAPI        0x80
#define DBG_LVL_PDMP        0x100
#define DBG_LVL_FLTR        0x200
#define DBG_LVL_KCOM        0x400
#define DBG_LVL_DMAERR      0x800
#define DBG_LVL_DMA         0x1000


#define DBG_VERB(_s)    do { if (debug & DBG_LVL_VERB) gprintk _s; } while (0)
#define DBG_DCB(_s)     do { if (debug & DBG_LVL_DCB)  gprintk _s; } while (0)
#define DBG_PKT(_s)     do { if (debug & DBG_LVL_PKT)  gprintk _s; } while (0)
#define DBG_SKB(_s)     do { if (debug & DBG_LVL_SKB)  gprintk _s; } while (0)
#define DBG_CMD(_s)     do { if (debug & DBG_LVL_CMD)  gprintk _s; } while (0)
#define DBG_EVT(_s)     do { if (debug & DBG_LVL_EVT)  gprintk _s; } while (0)
#define DBG_IRQ(_s)     do { if (debug & DBG_LVL_IRQ)  gprintk _s; } while (0)
#define DBG_NAPI(_s)    do { if (debug & DBG_LVL_NAPI) gprintk _s; } while (0)
#define DBG_PDMP(_s)    do { if (debug & DBG_LVL_PDMP) gprintk _s; } while (0)
#define DBG_FLTR(_s)    do { if (debug & DBG_LVL_FLTR) gprintk _s; } while (0)
#define DBG_KCOM(_s)    do { if (debug & DBG_LVL_KCOM) gprintk _s; } while (0)

/* Module Information */
#define MODULE_MAJOR 121
#define MODULE_NAME "linux-bcm-robo-knet"

/*
 * Only the old device-dependent NAPI interface is currently
 * supported and it should only be enabled if the BCM Rx API
 * is not used.
 */
#ifndef NAPI_SUPPORT
#define NAPI_SUPPORT 0
#endif


/*
 * If proxy support is compiled in the module will attempt to use
 * the user/kernel message service provided by the linux-uk-proxy
 * kernel module, otherwise device IOCTL will be used.
 */
#ifndef PROXY_SUPPORT
#define PROXY_SUPPORT 1
#endif


#if PROXY_SUPPORT

#include <linux-uk-proxy.h>

static int use_proxy = 1;
LKM_MOD_PARAM(use_proxy, "i", int, 0);
MODULE_PARM_DESC(use_proxy,
"Use Linux User/Kernel proxy (default 1)");

#define PROXY_SERVICE_CREATE(_s,_q,_f)  linux_uk_proxy_service_create(_s,_q,_f)
#define PROXY_SERVICE_DESTROY(_s)       linux_uk_proxy_service_destroy(_s); 
#define PROXY_SEND(_s,_m,_l)            linux_uk_proxy_send(_s,_m,_l)
#define PROXY_RECV(_s,_m,_l)            linux_uk_proxy_recv(_s,_m,_l)

#else

static int use_proxy = 0;

#define PROXY_SERVICE_CREATE(_s,_q,_f)
#define PROXY_SERVICE_DESTROY(_s)
#define PROXY_SEND(_s,_m,_l)
#define PROXY_RECV(_s,_m,_l) (-1)

#endif

/* Compatibility */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
#define skb_copy_to_linear_data(_skb, _pkt, _len) \
    eth_copy_and_sum(_skb, _pkt, _len, 0)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
#define SKB_PADTO(_skb,_len) (((_skb = skb_padto(_skb,_len)) == NULL) ? -1 : 0)
#else
#define SKB_PADTO(_skb,_len) skb_padto(_skb,_len)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12))
#define skb_header_cloned(_skb) \
    skb_cloned(_skb)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,27)
static inline void *netdev_priv(struct net_device *dev)
{
        return dev->priv;
}
#endif /* KERNEL_VERSION(2,4,27) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23)
/* Special check for MontaVista 2.4.20 MIPS */
#if !(defined(MAX_USER_RT_PRIO) && defined(CONFIG_MIPS))
static inline void free_netdev(struct net_device *dev)
{
        kfree(dev);
}
#endif

#endif /* KERNEL_VERSION(2,4,23) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,21)
static struct sk_buff *skb_pad(struct sk_buff *skb, int pad)
{
        struct sk_buff *nskb;
        
        /* If the skbuff is non linear tailroom is always zero.. */
        if(skb_tailroom(skb) >= pad)
        {
                memset(skb->data+skb->len, 0, pad);
                return skb;
        }
        
        nskb = skb_copy_expand(skb, skb_headroom(skb), skb_tailroom(skb) + pad, GFP_ATOMIC);
        kfree_skb(skb);
        if(nskb)
                memset(nskb->data+nskb->len, 0, pad);
        return nskb;
}       
static inline struct sk_buff *skb_padto(struct sk_buff *skb, unsigned int len)
{
        unsigned int size = skb->len;
        if(likely(size >= len))
                return skb;
        return skb_pad(skb, len-size);
}
#endif /* KERNEL_VERSION(2,4,21) */



static ibde_t *kernel_bde = NULL;

#define POLL_INTERVAL (HZ / 10)

#if defined(IPROC_CMICD)
#define ETCGMAC_HWRXOFF     30
#else
#define ETCGMAC_HWRXOFF		32
#endif

#define DMAREG(ch, dir, qnum)	((dir == DMA_TX) ? \
             (void *)(uint *)(&(ch->regs->d64xmt0control) + (0x10 * qnum)) : \
             (void *)(uint *)(&(ch->regs->d64rcv0control) + (0x10 * qnum)))


/* DCB chain info */
typedef struct bkn_dcb_chain_s {
    struct list_head list;
    int dcb_cnt;
    int dcb_cur;
    uint32_t *dcb_mem;
    dma_addr_t dcb_dma;
    eth_dcb_t   *eth_dcb;
} bkn_dcb_chain_t;


#if defined(IPROC_CMICD)
#define NUMTXQ  1
#else
#define NUMTXQ  4
#endif
#define NUMRXQ  NUMTXQ

#define TC_BK       0   /* background traffic class */
#define TC_BE       1   /* best effort traffic class */
#define TC_CL       2   /* controlled load traffic class */
#define TC_VO       3   /* voice traffic class */
#define TC_NONE     -1  /* traffic class none */

#define RX_Q0       0   /* receive DMA queue */
#define RX_Q1       1   /* receive DMA queue */
#define RX_Q2       2   /* receive DMA queue */
#define RX_Q3       3   /* receive DMA queue */

#define TX_Q0       TC_BK   /* DMA txq 0 */
#define TX_Q1       TC_BE   /* DMA txq 1 */
#define TX_Q2       TC_CL   /* DMA txq 2 */
#define TX_Q3       TC_VO   /* DMA txq 3 */


/* tunables */
#define NTXD		32		/* # tx dma ring descriptors (must be ^2) */
#define NRXD		64		/* # rx dma ring descriptors (must be ^2) */
 /* try to keep this # rbufs posted to the chip */
#define NRXBUFPOST   (NRXD - 1)

#define BUFSZ		2048		/* packet data buffer size */
#define RXBUFSZ		(BUFSZ - 256)	/* receive buffer size */

#ifndef RXBND
#define RXBND		16	/* max # rx frames to process in dpc */
#endif


/* Device control info */
typedef struct bkn_gmac_info_s {
    struct list_head list;
    volatile void *base_addr;   /* Base address for PCI register access */
    struct pci_dev *pdev;       /* Required for DMA memory control */
    struct net_device *dev;     /* Base network device */
    struct timer_list timer;    /* Retry/resource timer */
    spinlock_t lock;            /* Main lock for device */
    int dev_no;                 /* Device number (from BDE) */
    hnddma_t    *di[NUMTXQ];    /* dma engine software state */
    gmac0regs_t *regs;      /* pointer to chip registers */
    uint32_t      intstatus;  /* saved interrupt condition bits */
    uint32_t      intmask;    /* current software interrupt mask */
    int tx_active_chan;         /* DMA TX chan from BCM API*/
    uint32_t dma_events;        /* DMA events pending for BCM API */
    int rx_active_chan;         /* DMA RX chan to BCM API */
    uint32_t tx_desc_seqno;
    uint32_t rx_desc_seqno;    
    struct {
        int api_active;         /* BCM Tx API is in progress */        
        unsigned long suspends; /* Calls to netif_stop_queue (debug only) */        
        struct semaphore sem;   /* Lock required to start Tx DMA */
        struct list_head api_dcb_list; /* Tx DCB chains from BCM Tx API */
        unsigned long pkts;     /* Packet counter */
    }tx[NUMTXQ];        
    struct {                    
        uint32 rx_dcb_seqno;
        int buffer_size;       
        struct list_head api_dcb_list; /* Rx DCB chains from BCM Rx API */
        unsigned long pkts;     /* Packet counter */
    }rx[NUMRXQ];        

    int events;         /* bit channel between isr and dpc */
    int events_intsts;         /* bit channel between isr and dpc */


#if !(NAPI_SUPPORT)
    struct tasklet_struct tasklet;	/* dpc tasklet */
#endif	
    bool resched;		/* dpc was rescheduled */

}bkn_gmac_info_t;

/* Default random MAC address has Broadcom OUI with local admin bit set */
static u8 bkn_dev_mac[6] = { 0x02, 0x10, 0x18, 0x00, 0x00, 0x00 };

/* Driver Proc Entry root */
static struct proc_dir_entry *bkn_proc_root = NULL;


typedef struct bkn_priv_s {
    struct list_head list;
    struct net_device_stats stats; 
    struct net_device *dev;
#if NAPI_SUPPORT
    struct napi_struct napi;
#endif
    bkn_gmac_info_t *minfo;
    int id;
    int type;
    int port;
    uint32_t vlan;
    uint32_t flags;
    int switch_arch;
} bkn_priv_t;

typedef struct bkn_filter_s {
    struct list_head list;
    int dev_no;
    unsigned long hits;
    kcom_filter_t kf;
    int switch_arch;
} bkn_filter_t;

static wait_queue_head_t evt_wq;
static int evt_wq_put;
static int evt_wq_get;



/* Switch/gmac  devices */
LIST_HEAD(_minfo_list);

/* Net devices */
LIST_HEAD(_ndev_list);

/* Packet filters */
LIST_HEAD(_filter_list);

/* Free API buffers */
LIST_HEAD(_buf_free_list);


/*
 * Thread management
 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10))
/*
 * Old style using kernel_thread()
 */
typedef struct {
    const char * name;
    volatile int pid;
    volatile int run;
    struct completion completion;
    int state;
} bkn_thread_ctrl_t;

static int
bkn_thread_start(bkn_thread_ctrl_t *tc, const char *name,
                 int (*threadfn)(void *))
{
    if (name == NULL) {
        return -1;
    }
    tc->name = name;
    tc->pid = kernel_thread(threadfn, tc, 0);
    if (tc->pid < 0) {
        tc->pid = 0;
        return -1;
    }
    tc->run = 1;
    init_completion(&tc->completion);
    return 0;
}

static int
bkn_thread_stop(bkn_thread_ctrl_t *tc)
{
    if (tc->pid == 0) {
        return 0;
    }
    tc->run = 0;
    kill_proc(tc->pid, SIGTERM, 1);
    wait_for_completion(&tc->completion);
    return 0;
}

static int
bkn_thread_should_stop(bkn_thread_ctrl_t *tc)
{
    if (tc->run) {
        return 0;
    }
    tc->pid = 0;
    return 1;
}

static void
bkn_thread_boot(bkn_thread_ctrl_t *tc)
{
    siginitsetinv(&current->blocked, sigmask(SIGTERM) | sigmask(SIGKILL));
}

static void
bkn_thread_exit(bkn_thread_ctrl_t *tc)
{
    complete_and_exit(&tc->completion, 0);
}

static void
bkn_sleep(int clicks)
{
    wait_queue_head_t wq;

    init_waitqueue_head(&wq);
    sleep_on_timeout(&wq, clicks);
}
#else
/*
 * New style using kthread API
 */
#include <linux/kthread.h>
typedef struct {
    const char * name;
    struct task_struct *task;
    int state;
} bkn_thread_ctrl_t;

static int
bkn_thread_start(bkn_thread_ctrl_t *tc, const char *name,
                 int (*threadfn)(void *))
{
    if (name == NULL) {
        return -1;
    }
    tc->name = name;
    tc->task = kthread_run(threadfn, tc, name);
    if (IS_ERR(tc->task)) {
        tc->task = NULL;
        return -1;
    }
    return 0;
}

static int
bkn_thread_stop(bkn_thread_ctrl_t *tc)
{
    if (tc->task == NULL) {
        return 0;
    }
    send_sig(SIGTERM, tc->task, 0);
    return kthread_stop(tc->task);
}

static int
bkn_thread_should_stop(bkn_thread_ctrl_t *tc)
{
    return kthread_should_stop();
}

static void
bkn_thread_boot(bkn_thread_ctrl_t *tc)
{
    allow_signal(SIGTERM);
    allow_signal(SIGKILL);
}

static void
bkn_thread_exit(bkn_thread_ctrl_t *tc)
{
}

static void
bkn_sleep(int clicks)
{
    wait_queue_head_t wq;

    init_waitqueue_head(&wq);
    wait_event_timeout(wq, 0, clicks);
}

#endif

static bkn_thread_ctrl_t bkn_cmd_ctrl;
static bkn_thread_ctrl_t bkn_evt_ctrl;


/* Return a new packet. zero out pkttag */
void *osl_pktget(void *osh, int chan, uint len)
{

    if (use_rx_skb_chan & (1 << chan)){
        struct sk_buff *skb;       
        if ((skb = dev_alloc_skb(len))) {
            skb_put(skb, len);
            skb->priority = 0;           
        }    
        return ((void *) skb);
    } else {
        bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
        bkn_dcb_chain_t *dcb_chain;
        eth_dcb_t *dcb;

        if (!list_empty(&minfo->rx[chan].api_dcb_list)) {
            dcb_chain = list_entry(minfo->rx[chan].api_dcb_list.next,
                                   bkn_dcb_chain_t, list);

            DBG_DCB(("Start API Rx DMA, first DCB @ 0x%08x (%d DCBs).\n",
                     (uint32_t)dcb_chain->dcb_dma, dcb_chain->dcb_cnt));

            dcb = (eth_dcb_t *)dcb_chain->eth_dcb;
            list_del(&dcb_chain->list);
            kfree(dcb_chain);
        } else {
            dcb = NULL;
        }        
        return ((void *)dcb);
    }        
}

/* Free the driver packet. Free the tag if present */
void osl_pktfree(void *osh, int chan, void *p, bool send)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
    bool use_api = TRUE;

    if (send) {
#if defined(IPROC_CMICD)
        if (use_tx_skb) {
            use_api = FALSE;		
        }
#else
        /* di->apitx == 1*/
        if (chan == use_tx_skb) {
            use_api = FALSE;		
        } else {
            use_api = TRUE;
        }
#endif
    } else {
        if (use_rx_skb_chan & (1 << chan)){
            use_api = FALSE;
        }

#if defined(IPROC_CMICD)
        if (use_tx_skb) {
            use_api = FALSE;		
        }
#else
        if (chan == use_tx_skb){        
            use_api = FALSE;        
        }
#endif
    }

    if (use_api) {
        if (send) {
            minfo->tx_desc_seqno = ((eth_dcb_t *)p)->desc_seqno;            
            ((eth_dcb_t *)p)->desc_status = KNET_ETH_DCB_APITX_DONE;
        } else {
            minfo->rx[chan].rx_dcb_seqno = ((eth_dcb_t *)p)->desc_seqno;
            ((eth_dcb_t *)p)->desc_status = KNET_ETH_DCB_RX_ERR;
        }
    } else {
    	struct sk_buff *skb, *nskb;

        skb = (struct sk_buff *) p;
    	/* perversion: we use skb->next to chain multi-skb packets */
    	while (skb) {
    		nskb = skb->next;
    		skb->next = NULL;
    		if (skb->destructor) {
    			/* cannot kfree_skb() on hard IRQ (net/core/skbuff.c) if destructor exists
    			 */
    			dev_kfree_skb_any(skb);
    		} else {
    			/* can free immediately (even in_irq()) if destructor does not exist */
    			dev_kfree_skb(skb);
    		}
    		skb = nskb;
    	}
    }

}


unsigned char *osl_pktdata(void *osh, int chan, void *skb, bool send)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
    bool use_api = TRUE;
    void *pkt, *pkt_data;

  
    pkt = skb;
    if (send) {
        if (minfo->tx_active_chan  & (1 << chan)) {
            use_api = TRUE;        
        } else {
            use_api = FALSE;
        }
    } else {
        if (use_rx_skb_chan & (1 << chan)){
            use_api = FALSE;
        }
    }
    if (use_api) {
        pkt_data = (void *)kernel_bde->p2l(
             minfo->dev_no,
            ((eth_dcb_t *)pkt)->dcb_paddr);
    } else {
        pkt_data = ((struct sk_buff *) pkt)->data;
    }       

    return (unsigned char *)pkt_data;
}

uint osl_pktlen(void *osh, int chan, void *skb, bool send)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
    bool use_api = TRUE;
    uint len;

    if (send) {
        if (minfo->tx_active_chan  & (1 << chan)) {
            use_api = TRUE;        
        } else {
            use_api = FALSE;
        }
    } else {
        if (use_rx_skb_chan & (1 << chan)){
            use_api = FALSE;
        }
    }

    if (use_api) {        
        len = ((eth_dcb_t *)skb)->len;        
    } else {
        len = ((struct sk_buff *) skb)->len;
    }

    return len;

}

void *osl_pktnext(void *osh, int chan, void *skb, bool send)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
    bool use_api = TRUE;
    void *next;

    if (send) {
        if (minfo->tx_active_chan  & (1 << chan)) {
            use_api = TRUE;        
        } else {
            use_api = FALSE;
        }

    } else {
        if (use_rx_skb_chan & (1 << chan)){
            use_api = FALSE;
        }
    }

    if (use_api) {
        next = kernel_bde->p2l(
                     minfo->dev_no,
                (unsigned long)(((eth_dcb_t *)skb)->next_paddr));

    } else {
        next = ((struct sk_buff *) skb)->next;
    }

    return next;
}


void osl_pktsetlen(void *osh, int chan, void *skb, uint len)
{
    bool use_api = TRUE;

    if (use_rx_skb_chan & (1 << chan)){
        use_api = FALSE;
    }
    if (use_api) {
        ((eth_dcb_t *)skb)->len = len;
    }  else {
        __skb_trim((struct sk_buff *) skb, len);
    }
}


unsigned char *osl_pktpull(void *osh, int chan, void *skb, int bytes)
{

    bool use_api = TRUE;
    
    if (use_rx_skb_chan & (1 << chan)){
        use_api = FALSE;
    }

    if (use_api) {
        sal_paddr_t paddr;
        ((eth_dcb_t *)skb)->dcb_paddr += bytes;
        ((eth_dcb_t *)skb)->len -= bytes;
        paddr = ((eth_dcb_t *)skb)->dcb_paddr;
        return (char *)(unsigned long)paddr;
    }  else {
        return skb_pull((struct sk_buff *) skb, bytes);
    }
}


uint32 osl_dma_map(void *osh, int chan, void *va, uint size, int direction)
{

    int dir;
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
    bool use_api = TRUE;
    uint32 addr;


    if (direction == DMA_TX) {
        if (minfo->tx_active_chan  & (1 << chan)) {
            use_api = TRUE;        
        } else {
            use_api = FALSE;
        }
        dir = PCI_DMA_TODEVICE;
    } else {
        if (use_rx_skb_chan & (1 << chan)){
            use_api = FALSE;
        }
        dir = PCI_DMA_FROMDEVICE;
    }

    if (use_api) {
        addr = kernel_bde->l2p(minfo->dev_no, va);
    } else {
        addr = pci_map_single(minfo->pdev, va, size, dir);
    }

    return addr;
}

void osl_dma_unmap(void *osh, int chan, uint pa, uint size, int direction)
{
	int dir;
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;
    bool use_api = TRUE;

    if (direction == DMA_TX) {
        if (minfo->tx_active_chan  & (1 << chan)) {
            use_api = TRUE;        
        } else {
            use_api = FALSE;
        }
        dir = PCI_DMA_TODEVICE;

    } else {
        if (use_rx_skb_chan & (1 << chan)){
            use_api = FALSE;
        }
        dir = PCI_DMA_FROMDEVICE;
    }


    if (use_api) {
        kernel_bde->p2l(minfo->dev_no, pa);
    } else {
        pci_unmap_single(minfo->pdev, (uint32) pa, size, dir);
    }

}

void *osl_dma_alloc_consistent(void *osh, uint size, ulong *pap)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;


    return (pci_alloc_consistent(minfo->pdev, size, (dma_addr_t *) pap));
}

void osl_dma_free_consistent(void *osh, void *va, uint size, ulong pa)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;


    pci_free_consistent(minfo->pdev, size, va, (dma_addr_t) pa);
}



int osl_pktfrom(void *osh, int chan)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)osh;

    if (minfo->tx_active_chan & (1 << chan)) {
        return 1;
    } else {
        return 0;
    }
}


/*
 * Device Statistics Proc Entry
 */
static int 
bkn_proc_dma_show(struct seq_file *m, void *v){
    struct list_head *list;
    bkn_gmac_info_t *minfo;
    char buf[300];

    list_for_each(list, &_minfo_list) {
        minfo = (bkn_gmac_info_t *)list;

        seq_printf(m, "DMA Configuration:\n");
        dma_dumptx(minfo->di[0],buf,FALSE);
        seq_printf(m, buf);

        dma_dumprx(minfo->di[0],buf,FALSE);
        seq_printf(m, buf);

#if !defined(IPROC_CMICD)
        dma_dumptx(minfo->di[1],buf,FALSE);
        seq_printf(m, buf);

        dma_dumprx(minfo->di[1],buf,FALSE);
        seq_printf(m, buf);

        dma_dumptx(minfo->di[2],buf,FALSE);
        seq_printf(m, buf);

        dma_dumprx(minfo->di[2],buf,FALSE);
        seq_printf(m, buf);

        dma_dumptx(minfo->di[3],buf,FALSE);
        seq_printf(m, buf);

        dma_dumprx(minfo->di[3],buf,FALSE);
        seq_printf(m, buf);
#endif		

    }

    return 0;
}

static int bkn_proc_dma_open(struct inode * inode, struct file * file) {
    return single_open(file, bkn_proc_dma_show, NULL);
}

/*
 * Driver Debug Proc Entry
 */
static int 
bkn_proc_debug_show(struct seq_file *m, void *v){
    seq_printf(m, "Configuration:\n");
    seq_printf(m, "  debug:          0x%x\n", debug);
    seq_printf(m, "  mac_addr:       %02x:%02x:%02x:%02x:%02x:%02x\n",
                    bkn_dev_mac[0], bkn_dev_mac[1], bkn_dev_mac[2],
                    bkn_dev_mac[3], bkn_dev_mac[4], bkn_dev_mac[5]);
    seq_printf(m, "  rx_buffer_size: %d (0x%x)\n",
                    rx_buffer_size, rx_buffer_size);

    seq_printf(m, "  use_rx_skb_chan:     0x%x\n", use_rx_skb_chan);
    seq_printf(m, "  use_tx_skb:          0x%x\n", use_tx_skb);

    seq_printf(m, "Thread states:\n");
    seq_printf(m, "  Command thread: %d\n", bkn_cmd_ctrl.state);
    seq_printf(m, "  Event thread:   %d\n", bkn_evt_ctrl.state);
    return 0;
}

static int
bkn_proc_debug_open(struct inode * inode, struct file * file) {
    return single_open(file, bkn_proc_debug_show, NULL);
}

/*
 * Device Statistics Proc Entry
 */
static int 
bkn_proc_stats_show(struct seq_file *m, void *v){
    int unit = 0;
    struct list_head *list;
    bkn_gmac_info_t *minfo;
    int chan;

    list_for_each(list, &_minfo_list) {
        minfo = (bkn_gmac_info_t *)list;

        seq_printf(m,"tx active chan %x\n", minfo->tx_active_chan);
        seq_printf(m,"rx active chan %x\n", minfo->rx_active_chan);

        for (chan = 0; chan < NUMTXQ; chan++) {
            seq_printf(m, "Device stats:\n");
            seq_printf(m, "  Tx%d packets  %10lu\n",
                        chan, minfo->tx[chan].pkts);
            seq_printf(m, "  Rx%d packets %10lu\n",
                            chan, minfo->rx[chan].pkts);
            seq_printf(m, "  Tx%d suspends %10lu\n",
                            chan, minfo->tx[chan].suspends);
            seq_printf(m, "DMA event :\n");
            seq_printf(m, "  Tx%d api_tx status %d dcb_list empty %d \n",
                chan,
                minfo->tx[chan].api_active,
                list_empty(&minfo->tx[chan].api_dcb_list));
            seq_printf(m, "  Rx%d dcb_list empty %d dma active %d\n",
                chan,
                list_empty(&minfo->rx[chan].api_dcb_list),
                dma_rxactive(minfo->di[chan]));
            seq_printf(m, "  rx_dcb_seqno[%d] :%d\n",
                chan, minfo->rx[chan].rx_dcb_seqno);
            seq_printf(m, "  tx_desc_seqno[%d] :%d\n",
                chan,minfo->tx_desc_seqno);
        }
        unit++;
    }
    return 0;
}

static int
bkn_proc_stats_open(struct inode * inode, struct file * file) {
    return single_open(file, bkn_proc_stats_show, NULL);
}

struct file_operations bkn_proc_dma_fops = {
    owner:      THIS_MODULE,
    open:       bkn_proc_dma_open,
    read:       seq_read,
    llseek:     seq_lseek,
    release:    single_release,
};

struct file_operations bkn_proc_debug_fops = {
    owner:      THIS_MODULE,
    open:       bkn_proc_debug_open,
    read:       seq_read,
    llseek:     seq_lseek,
    release:    single_release,
};

struct file_operations bkn_proc_stats_fops = {
    owner:      THIS_MODULE,
    open:       bkn_proc_stats_open,
    read:       seq_read,
    llseek:     seq_lseek,
    release:    single_release,
};

static int
bkn_proc_init(void)
{
    struct proc_dir_entry* entry;

    PROC_CREATE(entry, "dma", 0, bkn_proc_root, &bkn_proc_dma_fops);
    if (entry == NULL) {
        return -1;
    }
    PROC_CREATE(entry, "debug", 0, bkn_proc_root, &bkn_proc_debug_fops);
    if (entry == NULL) {
        return -1;
    }
    PROC_CREATE(entry, "stats", 0, bkn_proc_root, &bkn_proc_stats_fops);
    if (entry == NULL) {
        return -1;
    }

    return 0;  
}

static int
bkn_proc_cleanup(void)
{
    remove_proc_entry("stats", bkn_proc_root); 
    remove_proc_entry("dma", bkn_proc_root); 
    remove_proc_entry("debug", bkn_proc_root); 
    return 0;
}

/* chip interrupt bit error summary */
#define	I_ERRORS		(INTMASK_PCIDESCERROR_MASK | INTMASK_PCIDATAERROR_MASK | \
     INTMASK_DESCERROR_MASK  | INTMASK_XMTFIFOUNDERFLOW_MASK)

#define RX_UF_INTMASK (INTMASK_RCVDESCUF_3_MASK | INTMASK_RCVDESCUF_2_MASK|\
        INTMASK_RCVDESCUF_1_MASK | INTMASK_RCVDESCUF_0_MASK)    

#define	DEF_INTMASK		(INTMASK_XMT0INTERRUPT_MASK | INTMASK_XMT1INTERRUPT_MASK \
    | INTMASK_XMT2INTERRUPT_MASK | INTMASK_XMT3INTERRUPT_MASK | INTMASK_RCVINTERRUPT_0_MASK | \
    INTMASK_RCVINTERRUPT_1_MASK | INTMASK_RCVINTERRUPT_2_MASK | INTMASK_RCVINTERRUPT_3_MASK | \
    I_ERRORS)

    /* interrupt event bitvec */
#define	INTR_TX		0x1
#define	INTR_RX		0x2
#define	INTR_ERROR	0x4
#define	INTR_TO		0x8
#define	INTR_NEW	0x10


/* get current and pending interrupt events */
static int
_irq_get_events(bkn_gmac_info_t *minfo, bool in_isr, uint32 *intsts)
{
    uint32 intstatus;
    int events;

    events = 0;
    *intsts = 0;


    /* read the interrupt status register */
    intstatus = R_REG(minfo, &minfo->regs->intstatus);
    DBG_IRQ(("actual intstatus %x\n", intstatus));

    /* defer unsolicited interrupts */
    intstatus &= (in_isr ? minfo->intmask : DEF_INTMASK);
    DBG_IRQ(("intstatus %x\n", intstatus));

    if (intstatus != 0)
        events = INTR_NEW;
   
    /* or new bits into persistent intstatus */
    intstatus = (minfo->intstatus |= intstatus);
    DBG_IRQ(("intstatus %x minfo->intstatus %x\n", 
        intstatus, minfo->intstatus));


    /* return if no events */
    if (intstatus == 0)
        return (0);

    /* convert chip-specific intstatus bits into generic intr event bits */
    if (intstatus & (INTMASK_RCVINTERRUPT_0_MASK | INTMASK_RCVINTERRUPT_1_MASK |
            INTMASK_RCVINTERRUPT_2_MASK | INTMASK_RCVINTERRUPT_3_MASK))
        events |= INTR_RX;
    if (intstatus & (INTMASK_XMT0INTERRUPT_MASK | INTMASK_XMT1INTERRUPT_MASK
            | INTMASK_XMT2INTERRUPT_MASK | INTMASK_XMT3INTERRUPT_MASK))
        events |= INTR_TX;
    if (intstatus & I_ERRORS)
        events |= INTR_ERROR;


    *intsts = intstatus;
    return (events);
}



static void
_irq_enable(bkn_gmac_info_t *minfo)
{   
   minfo->intmask = DEF_INTMASK;
   DBG_IRQ(("_irq_enable intmask %x\n",minfo->intmask));
   W_REG(minfo, &minfo->regs->intmask, minfo->intmask);
   
}

static void
_irq_disable(bkn_gmac_info_t *minfo)
{
    /* disable further interrupts from gmac */
    W_REG(minfo, &minfo->regs->intmask, 0);
    (void) R_REG(minfo, &minfo->regs->intmask);  /* sync readback */
    minfo->intmask = 0;
    DBG_IRQ(("_irq_disable intmask %x \n",minfo->intstatus));

    /* clear the interrupt conditions */
    W_REG(minfo, &minfo->regs->intstatus, minfo->intstatus);

}
static void
bkn_dump_pkt(uint8_t *data, int size)
{
    int idx;
    char str[128];


    if ((debug & DBG_LVL_PDMP) == 0) {
        return;
    }
    if (size > 64)
        size = 64;

    for (idx = 0; idx < size; idx++) {
        if ((idx & 0xf) == 0) {
            sprintf(str, "%04x: ", idx);
        }
        if ((idx & 0xf) == 8) {
            sprintf(&str[strlen(str)], "- ");
        }
        sprintf(&str[strlen(str)], "%02x ", data[idx]);
        if ((idx & 0xf) == 0xf) {
            sprintf(&str[strlen(str)], "\n");
            gprintk(str);
        }
    }
    if ((idx & 0xf) != 0) {
        sprintf(&str[strlen(str)], "\n");
        gprintk(str);
    }
}

static void
bkn_api_rx(bkn_gmac_info_t *minfo, int chan)
{

    /* Rx API currently not supported in SKB mode */
    if (use_rx_skb_chan & (1 << chan)) {
        return;
    }

    if (!list_empty(&minfo->rx[chan].api_dcb_list)) {
       dma_rxfill(minfo->di[chan]);
    }
}

#define HARRIER_IMP_TAG_OFFSET (14) /*after 0x8874*/
#define VULCAN_IMP_TAG_OFFSET (12)

static bkn_filter_t *
bkn_match_rx_pkt(uint8_t *pkt, int chan)
{
    struct list_head *list;
    bkn_filter_t *filter;
    kcom_filter_t scratch, *kf;
    uint8_t *oob=NULL;
    int size, wsize;
    int idx, match;
    int imp_tag_shift;
    int i, pkt_size, pkt_offset;


    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        kf = &filter->kf;

        if (filter->switch_arch == SOC_ROBO_ARCH_TBX) {
            oob = &pkt[0];
            pkt = &pkt[2*sizeof(brcm_t)];
        }

        if (filter->switch_arch == SOC_ROBO_ARCH_FEX) {
            oob = &pkt[HARRIER_IMP_TAG_OFFSET];
        }
        if (filter->switch_arch == SOC_ROBO_ARCH_VULCAN) {
            oob = &pkt[VULCAN_IMP_TAG_OFFSET];
        }


        memcpy(&scratch.data.b[0],
               &oob[kf->oob_data_offset], kf->oob_data_size);

        if (filter->switch_arch == SOC_ROBO_ARCH_TBX) {
            memcpy(&scratch.data.b[kf->oob_data_size],
               &pkt[kf->pkt_data_offset], kf->pkt_data_size);
        } else {
            imp_tag_shift = sizeof(brcm_t);
            if (filter->switch_arch == SOC_ROBO_ARCH_FEX) {
                /* length of 0x8874 */
                imp_tag_shift += 2;
            }

            if (kf->pkt_data_offset >= VULCAN_IMP_TAG_OFFSET) {
                memcpy(&scratch.data.b[kf->oob_data_size],
                   &pkt[kf->pkt_data_offset + imp_tag_shift], 
                   kf->pkt_data_size);
            } else {
                i = 0;
                pkt_offset = kf->pkt_data_offset;
                pkt_size = kf->pkt_data_size;
                while(((pkt_offset+i) < VULCAN_IMP_TAG_OFFSET) && (pkt_size > 0)){
                    scratch.data.b[kf->oob_data_size+i] =                        
                        pkt[pkt_offset+i];
                    i++;
                    pkt_size --;
                }

                if (pkt_size) {
                    memcpy(&scratch.data.b[kf->oob_data_size + i],
                       &pkt[pkt_offset + i + imp_tag_shift], pkt_size);
                }
            }
        }

        size = kf->oob_data_size + kf->pkt_data_size;
        wsize = BYTES2WORDS(size);
        DBG_FLTR(("Filter: size = %d (%d), data = 0x%08x,0x%08x mask = 0x%08x,0x%08x\n",
                  size, wsize, kf->data.w[0], kf->data.w[1],
                  kf->mask.w[0], kf->mask.w[1]));

        for (idx = 0; idx < wsize; idx++) {
            scratch.data.w[idx] &= kf->mask.w[idx];
        }
        DBG_FLTR(("Packet: orig = 0x%08x, masked = 0x%08x\n",
                  ((uint32_t*)pkt)[0], scratch.data.w[0]));
        DBG_FLTR(("Packet: orig[1] = 0x%08x, masked[1] = 0x%08x\n",
                  ((uint32_t*)pkt)[1], scratch.data.w[1]));

        match = 1;
      
        if (match) {
            for (idx = 0; idx < wsize; idx++) {
                if (scratch.data.w[idx] != kf->data.w[idx]) {
                    match = 0;
                }
            }
        }
        if (match) {
            return filter;
        }
    }

    return NULL;
}



static int
bkn_do_rx(bkn_gmac_info_t *minfo, int chan, int budget)
{
    struct list_head *list;
    bkn_priv_t *priv;
    struct sk_buff *skb;
    void *p, *last_p = NULL;
    uint8 *buf = NULL;
    int processed = 0;
    bkn_filter_t *filter;
    int drop_api;
    int found;    
    int pktlen;
    int idx, imptag_len=0;


    while ((p = dma_rx(minfo->di[chan]))) {
        minfo->rx[chan].pkts++;

        /* skip the rx header */
        PKTPULL(minfo, chan, p, ETCGMAC_HWRXOFF);
        buf = PKTDATA(minfo, chan, p, 0);
        pktlen = PKTLEN(minfo, chan, p, 0);

        bkn_dump_pkt(buf, 32);
     
        filter = bkn_match_rx_pkt(buf, chan);
       
        drop_api = 1;

        if (filter) {
            DBG_FLTR(("Match filter ID %d\n", filter->kf.id));
            filter->hits++;
            switch (filter->kf.dest_type) {
                case KCOM_DEST_T_API:
                    DBG_FLTR(("Send to Rx API\n"));
                    drop_api = 0;
                    break;
                case KCOM_DEST_T_NETIF:
                    found = 0;
                    priv = NULL;
                    list_for_each(list, &_ndev_list) {
                        priv = (bkn_priv_t *)list;
                        if (priv->id == filter->kf.dest_id) {
                            found = 1;
                            break;
                        }
                    }
                    if (found && priv) {
                        DBG_FLTR(("Send to netif %d (%s)\n",
                                  priv->id, priv->dev->name));

                        if (use_rx_skb_chan & (1<<chan)) {
                            skb = (struct sk_buff *)p;
                        } else {
                            /* descriptors from user space */
                            skb = dev_alloc_skb(pktlen);
                            if (skb == NULL) {
                                break;
                            }   

                            skb_copy_to_linear_data(skb, buf, pktlen);
                            skb_put(skb, pktlen);
                        }                            
                        if (filter->switch_arch == SOC_ROBO_ARCH_TBX) {
                            imptag_len = 2* sizeof(brcm_t);
                            skb_pull(skb, imptag_len);
                        } else {
                            if (filter->switch_arch == SOC_ROBO_ARCH_FEX) {
                                imptag_len = (sizeof(brcm_t) + 2);
                            }
                            if (filter->switch_arch == SOC_ROBO_ARCH_VULCAN) {
                                imptag_len = sizeof(brcm_t);
                            }

                            /* strip the IMP tag */
                            for(idx = 11;idx >= 0;idx --){
                                ((u8*)skb->data)[idx + imptag_len] = 
                                    ((u8*)skb->data)[idx];
                            }

                            skb_pull(skb, imptag_len);
                            pktlen -= imptag_len;
                        }

                        bkn_dump_pkt(skb->data, skb->len);

                        if (filter->kf.flags & KCOM_FILTER_F_STRIP_TAG) {
                            /* Strip VLAN tag */
                            DBG_FLTR(("Strip VLAN tag\n"));
                            ((u32*)skb->data)[3] = ((u32*)skb->data)[2];
                            ((u32*)skb->data)[2] = ((u32*)skb->data)[1];
                            ((u32*)skb->data)[1] = ((u32*)skb->data)[0];
                            skb_pull(skb, 4);
                            pktlen -= 4;
                        }

                        skb_trim(skb, skb->len - 4);/* strip crc*/
                        bkn_dump_pkt(skb->data, skb->len);                        

                        skb->dev = priv->dev;                            

                        priv->stats.rx_packets++;
                        priv->stats.rx_bytes += pktlen;

                        skb->protocol = eth_type_trans(skb, skb->dev);

                        /* Unlock while calling up network stack */
                        spin_unlock(&minfo->lock);

#if NAPI_SUPPORT
                        netif_receive_skb(skb);
#else
                        netif_rx(skb);
#endif
                        spin_lock(&minfo->lock);
                        if (filter->kf.mirror_type == KCOM_DEST_T_API) {
                            drop_api = 0;
                        }
                    } else {
                        DBG_FLTR(("Unknown netif %d\n",
                                  filter->kf.dest_id));
                    }
                    break;
                default:
                    /* Drop packet */
                    DBG_FLTR(("Unknown dest type %d\n",
                              filter->kf.dest_type));
                    break;
            }
        }

        if (use_rx_skb_chan & (1 << chan)) {
            dma_rxfill(minfo->di[chan]);
        } else {
            DBG_FLTR(("drop_api %d\n", drop_api));
            if (drop_api){   
                ((eth_dcb_t *)p)->desc_status = KNET_ETH_DCB_ETHRX_DONE;
            } else {
                ((eth_dcb_t *)p)->desc_status = KNET_ETH_DCB_APIRX_DONE;
            }

        }
        last_p = p;
        if (++processed >= budget) {
            break;
        }
    }
    if(last_p)
        minfo->rx[chan].rx_dcb_seqno = ((eth_dcb_t *)last_p)->desc_seqno;

    return processed;
}

static int
bkn_rx_desc_done(bkn_gmac_info_t *minfo, int chan, int budget)
{
    int processed;
    DBG_IRQ(("Rx%d desc done\n", chan));

    processed = bkn_do_rx(minfo, chan, budget);
    minfo->intstatus &= ~(INTMASK_RCVINTERRUPT_0_MASK << chan);     


    if (use_rx_skb_chan & (1 << chan)){
        return processed;
    }
    minfo->dma_events |= KCOM_DMA_INFO_F_RX_DONE;
    minfo->rx_active_chan = chan;
    minfo->rx_desc_seqno = minfo->rx[chan].rx_dcb_seqno ;

    evt_wq_put++;
    wake_up_interruptible(&evt_wq);
    return processed;
}


static void
bkn_suspend_tx(bkn_gmac_info_t *minfo, int chan)
{
    struct list_head *list;
    bkn_priv_t *priv = netdev_priv(minfo->dev);

    /* Unlock while calling up network stack */
    spin_unlock(&minfo->lock);
    /* Stop main device */
    netif_stop_queue(priv->dev);
    minfo->tx[chan].suspends++;
    /* Stop associated virtual devices */
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (priv->minfo->dev_no != minfo->dev_no) {
            continue;
        }
        netif_stop_queue(priv->dev);
    }
    spin_lock(&minfo->lock);
}

static void
bkn_resume_tx(bkn_gmac_info_t *minfo, int chan)
{
    struct list_head *list;
    bkn_priv_t *priv;

    /* Check main device */
    if (netif_queue_stopped(minfo->dev) && 
        minfo->di[chan]->txavail) {
        /* Unlock while calling up network stack */
        spin_unlock(&minfo->lock);
        netif_wake_queue(minfo->dev);
        spin_lock(&minfo->lock);
    }
    /* Check associated virtual devices */
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (priv->minfo->dev_no != minfo->dev_no) {
            continue;
        }
        if (netif_queue_stopped(priv->dev) && 
            minfo->di[chan]->txavail) {
            /* Unlock while calling up network stack */
            spin_unlock(&minfo->lock);
            netif_wake_queue(priv->dev);
            spin_lock(&minfo->lock);
        }
    }
}

static void
bkn_api_tx(bkn_gmac_info_t *minfo, int chan)
{
    bkn_dcb_chain_t *dcb_chain;
    eth_dcb_t *dcb;


    /* Assume that driver lock is held */
    if (list_empty(&minfo->tx[chan].api_dcb_list)) {
        minfo->tx[chan].api_active = 0;
    } else {
        minfo->tx[chan].pkts++;
        minfo->tx[chan].api_active = 1;
        dcb_chain = list_entry(minfo->tx[chan].api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
        DBG_DCB(("Start API Tx DMA, first DCB @ 0x%08x (%d DCBs).\n",
                 (uint32_t)dcb_chain->dcb_dma, dcb_chain->dcb_cnt));

        minfo->tx_active_chan |= (1 << chan);
        dcb = (eth_dcb_t *)dcb_chain->eth_dcb;

        dma_txfast(minfo->di[chan],dcb, 1);
        list_del(&dcb_chain->list);
        kfree(dcb_chain);
        minfo->tx_active_chan &= ~(1 << chan);
    }
}

static void
bkn_tx_chain_done(bkn_gmac_info_t *minfo, int chan)
{
    eth_dcb_t *dcb;


    DBG_IRQ(("Tx chain %d done \n", chan));
    
    dcb = (eth_dcb_t *)dma_txreclaim(minfo->di[chan], 0);
    minfo->intstatus &= ~(INTMASK_XMT0INTERRUPT_MASK << chan);       

    if (minfo->tx[chan].api_active) {
        if (dcb == NULL){
            gprintk("bkn_tx_chain_done ---error--- should not be NULL\n");
        }
        
        if (!minfo->tx_desc_seqno){
            DBG_IRQ(("tx_desc_seqno %d %d\n",
                minfo->tx_desc_seqno,dcb->desc_seqno));
        } else {
            minfo->dma_events |= KCOM_DMA_INFO_F_TX_DONE;

            DBG_IRQ(("--seqno %d\n",minfo->tx_desc_seqno));
            evt_wq_put++;
            wake_up_interruptible(&evt_wq);        
        }
        bkn_api_tx(minfo, chan);
    }

}



#if NAPI_SUPPORT
static int
bkn_poll(struct napi_struct *napi, int budget)
{
    bkn_priv_t *priv = container_of(napi, bkn_priv_t, napi);
    bkn_gmac_info_t *minfo = priv->minfo;
    struct net_device *dev = priv->dev;	
    int quota = budget;

#else				
static void bkn_dpc(ulong data)
{
    int quota = RXBND;
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *) data;
#endif	
    int i;
    uint32 txchan, rxchan, intstatus;
    int processed = 0;

    spin_lock(&minfo->lock);
    DBG_NAPI(("bkn_dpc / bkn_poll\n"));
    /* get interrupt condition bits again when dpc was rescheduled */
    if (minfo->resched) {
        minfo->events = _irq_get_events(minfo, FALSE, &intstatus);
        minfo->resched = FALSE;
    }
    DBG_NAPI(("handle events %x\n",minfo->events));
    if (minfo->events & INTR_RX) {
        rxchan = (int)((minfo->events_intsts >> INTMASK_RCVINTERRUPT_0_SHIFT)& 0xf);

        for (i = (NUMRXQ - 1); i >= 0; i--){
            if (rxchan & (1 << i)){
                processed = bkn_rx_desc_done(minfo, i, quota);
                if (processed >= quota) {
                    minfo->resched = TRUE;
                    break;
                }
            }
        }

    }
    for (i = 0; i < NUMTXQ; i++){
        bkn_api_rx(minfo, i);
    }

    if (minfo->events & INTR_TX) {
        txchan = (int)((minfo->events_intsts >> INTMASK_XMT0INTERRUPT_SHIFT)& 0xf);
        for (i = 0; i < NUMTXQ; i++){
            if (txchan & (1 << i)){
                if (i == tx_chan_inuse) {
                    bkn_resume_tx(minfo, i);
                }
                bkn_tx_chain_done(minfo, i);
            }
        }
    }


    /* clear this before re-enabling interrupts */
    minfo->events = 0;    

#if !(NAPI_SUPPORT)
    /* there may be frames left, reschedule bkn_dpc() */
    if (minfo->resched)
        tasklet_schedule(&minfo->tasklet);
    /* re-enable interrupts */
    else
        _irq_enable(minfo);
#endif				/* BCM_NAPI */

    spin_unlock(&minfo->lock);


#if (NAPI_SUPPORT)
    /* we got packets but no quota */
    if (minfo->resched)
    /* indicate that we are not done, don't enable
     * interrupts yet. linux network core will call
     * us again.
     */
        return (quota);
    /* enable interrupts now */
    _irq_enable(minfo);

    netif_rx_complete(dev, napi);

    /* indicate that we are done */
    return (processed);
#else
    return;
#endif


}


static void
bkn_isr(void *isr_data)
{
    bkn_gmac_info_t *minfo = isr_data;
    uint32 events, intstatus;
#if NAPI_SUPPORT
    struct list_head *list;
    bkn_priv_t *priv = netdev_priv(minfo->dev);
#endif


    events = _irq_get_events(minfo, TRUE, &intstatus);
    DBG_IRQ(("%s events %x intstatus %x\n",FUNCTION_NAME(),events,intstatus));

    if (!(events & INTR_NEW)) {
        return;
    }
    _irq_disable(minfo);


    minfo->events = events;
    minfo->events_intsts = intstatus;
#if NAPI_SUPPORT
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (priv->minfo->dev_no != minfo->dev_no) {
            continue;
        }
            /* allow the device to be added to the cpu polling list if we are up */
        if (netif_rx_schedule_prep(minfo->dev, &priv->napi)) {
            /* tell the network core that we have packets to send up */
            __netif_rx_schedule(minfo->dev, &priv->napi);
        } else {
            _irq_enable(minfo);        
        }
    }
#else
    /* schedule dpc */
    tasklet_schedule(&minfo->tasklet);
#endif	

    return;
}

static int 
bkn_open(struct net_device *dev)
{
    netif_start_queue(dev);
    return 0;
}

static int
bkn_change_mtu(struct net_device *dev, int new_mtu)
{
    if (new_mtu < 68 || new_mtu > (rx_buffer_size)) {
        return -EINVAL;
    }
    dev->mtu = new_mtu;
    return 0;
}


static int
bkn_stop(struct net_device *dev)
{
    netif_stop_queue(dev);
    return 0;
}	

/* 
 * Network Device Statistics. 
 * Cleared at init time.  
 */
static struct net_device_stats *
bkn_get_stats(struct net_device *dev)
{
    bkn_priv_t *priv = netdev_priv(dev);

    return &priv->stats;
}

/* Fake multicast ability */
static void
bkn_set_multicast_list(struct net_device *dev)
{
}


static int 
bkn_tx(struct sk_buff *skb, struct net_device *dev)
{
    bkn_priv_t *priv = netdev_priv(dev);
    bkn_gmac_info_t *minfo = priv->minfo;
    struct sk_buff *new_skb;
    int pktlen;
    int taglen;
    unsigned long flags;
    int imp_tag_len, imp_tag_offset, imp_tag_8874, padlen, inter_tag;
    brcm_t imp_tag[2];
    int packet_offset;
    int i;
    uint32 tmp;

    DBG_VERB(("Netif Tx\n"));

    if (priv->id <= 0) {
        /* Do not transmit on base device */
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);
        return 0;
    }
    if (down_trylock(&minfo->tx[tx_chan_inuse].sem) != 0) {
        DBG_VERB(("Netif Tx while suspended - dropping SKB\n"));
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);
        return 0;
    }

    spin_lock_irqsave(&minfo->lock, flags);
    if (minfo->di[tx_chan_inuse]->txavail > 0 ){


    taglen = 0;    
    if (priv->port < 0 || (priv->flags & KCOM_NETIF_F_ADD_TAG)) {
         /* Need to add VLAN tag if packet is untagged */
         if (skb->data[12] != 0x81 && skb->data[13] != 0x00) {
             taglen = 4;
        }
    }

    /* IMP ingress tag */
    imp_tag[0].brcm_val = 0;
    imp_tag[1].brcm_val = 0;

    imp_tag_offset = 0;
    packet_offset = 0;
    imp_tag_len = 0;
    imp_tag_8874 = 0;
    inter_tag = 0;

    switch(priv->switch_arch) {
        case SOC_ROBO_ARCH_TBX:
            imp_tag_len = 2* sizeof(brcm_t);
            imp_tag_offset = 0;
            packet_offset = 8;
            if (priv->port < 0) {
                imp_tag[0].brcm_opcode_53280 = BRCM_OP_UCAST_53280;
            } else {
                imp_tag[0].brcm_opcode_53280 = BRCM_OP_EGR_DIR_53280;
                imp_tag[0].brcm_tc_53280 = skb->priority; 
                imp_tag[0].brcm_dp_53280 = 1; 
                imp_tag[0].brcm_filter_53280 = 0xff;
                imp_tag[0].brcm_dstid_53280 = priv->port;
                imp_tag[1].brcm_dnm_53280 = 1; 
                imp_tag[1].brcm_ing_vid_53280 = 1;
                imp_tag[1].brcm_flow_id_53280 = 0; 
            }
            break;
        case SOC_ROBO_ARCH_VULCAN:
            imp_tag_len = sizeof(brcm_t);
            /* imp_tag start offset*/
            imp_tag_offset = VULCAN_IMP_TAG_OFFSET;
            if (priv->port < 0) {
                imp_tag[0].brcm_op = BRCM_OP_UCAST;
            } else {
                imp_tag[0].brcm_op = BRCM_OP_MCAST;
                imp_tag[0].brcm_53115_multi_port_tag_stamp._dst_pbmp 
                    =  1 << priv->port;
                imp_tag[0].brcm_tc = skb->priority;
                if (taglen) {
                    imp_tag[0].brcm_te = TAG_ENFORCEMENT;            
                } else{
                    imp_tag[0].brcm_te = TAG_NO_ENFORCEMENT;            
                }
            }
            break;
        case SOC_ROBO_ARCH_FEX:
            /* imp_tag (after 0x8874) start offset */
            imp_tag_offset = HARRIER_IMP_TAG_OFFSET;
            /* length of 0x8874 */
            imp_tag_8874 = 2;
            imp_tag_len = sizeof(brcm_t) + imp_tag_8874;

            if (priv->port < 0) {
                imp_tag[0].brcm_op = BRCM_OP_UCAST;

            } else {
                imp_tag[0].brcm_tq_53242 = skb->priority/2;
                if (priv->port/24){
                    /* IMP & GE port*/
                    imp_tag[0].brcm_op = BRCM_OP_MULTI_PORT_IMP_HS;
                    imp_tag[0].brcm_tq_te_bmp._dst_pbmp = 
                        1 << (priv->port - 24);
                } else{
                    /* 0-23 port (FE ports)*/
                    imp_tag[0].brcm_op = BRCM_OP_MULTI_PORT_LS;
                    imp_tag[0].brcm_tq_te_bmp._dst_pbmp = 1<<priv->port;
                }
                if (taglen) {
                    imp_tag[0].brcm_te_53242 = TAG_ENFORCEMENT;            
                } else{
                    imp_tag[0].brcm_te_53242 = TAG_NO_ENFORCEMENT;            
                }
            }

            /* harrier need vlan tag internally */
            inter_tag = 4;
            taglen = 0;


            break;
    }

    padlen = imp_tag_len;
    
    /* vlan (4 bytes) */
    padlen += taglen;
    /* da(6 bytes) + sa(6 bytes) */
    padlen += 12;

    padlen += inter_tag;
    
    /* allocate the header buffer contain imp_tag(4 or 8) +da(6)+sa(6)+vlan(4)*/
    new_skb = dev_alloc_skb(padlen);
    if (skb == NULL) {
        DBG_VERB(("Tx drop: No SKB memory\n"));
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);
        spin_unlock_irqrestore(&minfo->lock, flags);
        up(&minfo->tx[tx_chan_inuse].sem);
        return 0;
    }        

    skb_put(new_skb, padlen);           


    DBG_PKT(("imp_tag %x %x\n",
        imp_tag[0].brcm_val,imp_tag[1].brcm_val));

    tmp = imp_tag[0].brcm_val;
    imp_tag[0].brcm_val = htonl(tmp);
    tmp = imp_tag[1].brcm_val;
    imp_tag[1].brcm_val = htonl(tmp);

    for (i = 0; i < imp_tag_len/sizeof(brcm_t); i++) {
        memcpy(&new_skb->data[imp_tag_offset + (4 * i)], (uint8 *)&imp_tag[i], sizeof(brcm_t));

    }

    memcpy(&new_skb->data[packet_offset], &skb->data[0], 6);
    memcpy(&new_skb->data[packet_offset + 6], &skb->data[6], 6);

    if (taglen || inter_tag){
        new_skb->data[imp_tag_len + 12] = 0x81;
        new_skb->data[imp_tag_len + 13] = 0x00;
        new_skb->data[imp_tag_len + 14] = (priv->vlan >> 8) & 0xf;
        new_skb->data[imp_tag_len + 15] = priv->vlan & 0xff;
    }

    if (imp_tag_8874) {
        new_skb->data[packet_offset + 12] = 0x88;
        new_skb->data[packet_offset + 13] = 0x74;        
    }
    if (inter_tag) {
        skb->len += 4;
    }

    bkn_dump_pkt(new_skb->data, new_skb->len);

    skb_pull(skb, 12);

    new_skb->next = skb;
    pktlen = new_skb->len + skb->len;

    if (pktlen < (64 + taglen + imp_tag_len + imp_tag_8874)) {
        pktlen = (64 + taglen + imp_tag_len + imp_tag_8874) - 
            new_skb->len;
        if (SKB_PADTO(skb, pktlen) != 0) {
            priv->stats.tx_dropped++;
            DBG_SKB(("Tx drop: skb_padto failed\n"));
            spin_unlock_irqrestore(&minfo->lock, flags);
            up(&minfo->tx[tx_chan_inuse].sem);
            return 0;
        }

        DBG_SKB(("Packet padded to %d bytes skb->len %d\n", pktlen,skb->len));
        
        skb->len = pktlen;
    }
    bkn_dump_pkt(skb->data, skb->len);

    dma_txfast(minfo->di[tx_chan_inuse], new_skb, 1);

    priv->stats.tx_packets++;
    priv->stats.tx_bytes += pktlen;
    minfo->tx[tx_chan_inuse].pkts++;
    } else {
        priv->stats.tx_dropped++;
        dev_kfree_skb(skb);

    }

    /* Check our Tx resources */

    if (minfo->di[tx_chan_inuse]->txavail < 1) {
        bkn_suspend_tx(minfo, tx_chan_inuse);
    }

    dev->trans_start = jiffies;

    spin_unlock_irqrestore(&minfo->lock, flags);
    up(&minfo->tx[tx_chan_inuse].sem);

    return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29))
static struct net_device_ops robo_knet_ops = {
     .ndo_open         = bkn_open,
     .ndo_stop         = bkn_stop,
     .ndo_start_xmit   = bkn_tx,
     .ndo_do_ioctl     = NULL,
     .ndo_set_rx_mode  = bkn_set_multicast_list,
     .ndo_get_stats    = bkn_get_stats,
     .ndo_change_mtu   = bkn_change_mtu,
};
#endif

static struct net_device *
bkn_init_ndev(u8 *mac, char *name)
{
    struct net_device *dev;

    /* Create Ethernet device */
    dev = alloc_etherdev(sizeof(bkn_priv_t));

    if (dev == NULL) {
        DBG_VERB(("Error allocating Ethernet device.\n"));
        return NULL;
    }
#ifdef SET_MODULE_OWNER
    SET_MODULE_OWNER(dev);
#endif
    /* Set the device MAC address */
    memcpy(dev->dev_addr, mac, 6);
    
    /* Device vectors */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31))
    dev->netdev_ops = &robo_knet_ops;
#else
    dev->open = bkn_open;
    dev->hard_start_xmit = bkn_tx;
    dev->stop = bkn_stop;
    dev->set_multicast_list = bkn_set_multicast_list;
    dev->do_ioctl = NULL;
    dev->get_stats = bkn_get_stats;
    dev->change_mtu = bkn_change_mtu;
#endif

    if (name && *name) {
        strncpy(dev->name, name, IFNAMSIZ-1);
    }

    /* Register the kernel Ethernet device */
    if (register_netdev(dev)) {
        DBG_VERB(("Error registering Ethernet device.\n"));
        free_netdev(dev);
        return NULL;
    }
    DBG_VERB(("Created Ethernet device %s.\n", dev->name));

    return dev;
}


static bkn_gmac_info_t *
bkn_minfo_from_unit(int unit)
{
    struct list_head *list;
    bkn_gmac_info_t *minfo;

    list_for_each(list, &_minfo_list) {
        minfo = (bkn_gmac_info_t *)list;
        if (minfo->dev_no == unit) {
            return minfo;
        }
    }

    return NULL;
}
static int
bkn_dma_abort_tx(bkn_gmac_info_t *minfo, int chan)
{
    bkn_dcb_chain_t *dcb_chain;

    DBG_VERB(("Aborting Tx DMA.\n"));

    while (!list_empty(&minfo->tx[chan].api_dcb_list)) {
	dcb_chain = list_entry(minfo->tx[chan].api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
	list_del(&dcb_chain->list);
        DBG_DCB(("Freeing Tx DCB chain.\n"));
        kfree(dcb_chain);
    }

    return 0;
}

static int
bkn_dma_abort_rx(bkn_gmac_info_t *minfo, int chan)
{
    bkn_dcb_chain_t *dcb_chain;


    DBG_VERB(("Aborting Rx%d DMA.\n", chan));
   
    while (!list_empty(&minfo->rx[chan].api_dcb_list)) {
	    dcb_chain = list_entry(minfo->rx[chan].api_dcb_list.next,
                               bkn_dcb_chain_t, list);	
    	list_del(&dcb_chain->list);
        DBG_DCB(("Freeing Rx%d DCB chain.\n", chan));
        kfree(dcb_chain);
    }

    return 0;
}



static int
bkn_dma_config(bkn_gmac_info_t *minfo, int chan, unsigned long flags, 
                        unsigned long val)
{
    int i, start, end;

    if (chan == KCOM_ETH_HW_C_ALL) {
        start = 0;
        end = NUMTXQ;
    } else {
        start = chan;
        end = chan + 1;
    }

    for (i = start; i < end; i++) {
        if (flags & KCOM_ETH_HW_OTHER_F_FIFO_LOOPBACK) {
            dma_fifoloopbackenable(minfo->di[i], val);        
        }
    }

    if (flags == KCOM_ETH_HW_OTHER_F_INTERRUPT) {
        if (val)
            _irq_enable(minfo);
        else
            _irq_disable(minfo);

    }
    return 0;
}

static int
bkn_dma_reset(bkn_gmac_info_t *minfo, int chan, unsigned long flags,
                    unsigned long val)
{
    int i, start, end;

    if (chan == KCOM_ETH_HW_C_ALL) {
        start = 0;
        end = NUMTXQ;
    } else {
        start = chan;
        end = chan + 1;
    }

    DBG_CMD(("bkn_dma_reset chan %d flags %lx \n",chan, flags));

    for (i = start; i < end; i++) {
        if (flags & KCOM_ETH_HW_RESET_F_TX) {
            dma_txreset(minfo->di[i]);        
        }
        if (flags & KCOM_ETH_HW_RESET_F_RX) {
            dma_rxreset(minfo->di[i]);
        }
        if (flags & KCOM_ETH_HW_RESET_F_TX_RECLAIM) {            
            dma_txreclaim(minfo->di[i], val);
            minfo->intstatus &= ~(INTMASK_XMT0INTERRUPT_MASK << i);
            bkn_dma_abort_tx(minfo, i);
            minfo->tx_desc_seqno = 0;
        }
        if (flags & KCOM_ETH_HW_RESET_F_RX_RECLAIM) {
            dma_rxreclaim(minfo->di[i]);
            minfo->intstatus &= ~(INTMASK_RCVINTERRUPT_0_MASK << i);
            bkn_dma_abort_rx(minfo, i);
            minfo->rx_desc_seqno = 0;
            minfo->rx[i].rx_dcb_seqno = 0;
        }
    }

    return 0;
}

static int
bkn_dma_init(bkn_gmac_info_t *minfo, int chan, unsigned long flags,
            unsigned long val)
{
    int i, start, end;

    if (chan == KCOM_ETH_HW_C_ALL) {
        start = 0;
        end = NUMTXQ;
    } else {
        start = chan;
        end = chan + 1;
    }
    DBG_CMD(("bkn_dma_init chan %d flags %lx \n",chan, flags));

    for (i = start; i < end; i++) {
        if (flags & KCOM_ETH_HW_INIT_F_TX) {
            dma_txinit(minfo->di[i]);
        }
        if (flags & KCOM_ETH_HW_INIT_F_RX) {
            if(!dma_rxenabled(minfo->di[i]) || (val == 1)) {
                dma_rxinit(minfo->di[i]);
            }
        }
        if (flags & KCOM_ETH_HW_INIT_F_RX_FILL) {
            dma_rxfill(minfo->di[i]);
        }
    }

    return 0;
}

static int
bkn_dma_abort(bkn_gmac_info_t *minfo)
{

    int chan;

    for (chan = 0; chan < NUMTXQ; chan++) {
        bkn_dma_reset(minfo, chan, 
            KCOM_ETH_HW_RESET_F_TX | KCOM_ETH_HW_RESET_F_RX |
            KCOM_ETH_HW_RESET_F_TX_RECLAIM |
            KCOM_ETH_HW_RESET_F_RX_RECLAIM, 1);
    }
    return 0;
}


static void
bkn_timer(unsigned long context)
{
    bkn_gmac_info_t *minfo = (bkn_gmac_info_t *)context;



    minfo->timer.expires = jiffies + POLL_INTERVAL;
    add_timer(&minfo->timer);

}

static void
bkn_destroy_minfo(bkn_gmac_info_t *minfo)
{
    int i;
    /* free dma state */
    for (i = 0; i < NUMTXQ; i++){
        if (minfo->di[i] != NULL) {
            dma_detach(minfo->di[i]);
            minfo->di[i] = NULL;
        }
    }
#if !(NAPI_SUPPORT)
    /* kill dpc */
    tasklet_kill(&minfo->tasklet);
#endif	

    del_timer_sync(&minfo->timer);
    list_del(&minfo->list);

    kfree(minfo);
}

static uint32 msglevel=0;

static bkn_gmac_info_t *
bkn_create_minfo(int dev_no)
{
    bkn_gmac_info_t *minfo;
    char name[16];
    int i;


    if ((minfo = kmalloc(sizeof(*minfo), GFP_KERNEL)) == NULL) {
        return NULL;
    }
    memset(minfo, 0, sizeof(*minfo));
    minfo->base_addr = lkbde_get_dev_virt(dev_no);
    minfo->pdev = lkbde_get_hw_dev(dev_no);
    minfo->dev_no = dev_no;

    DBG_VERB(("base_addr %lx pdev %p dev_no %d\n",
        (unsigned long)(minfo->base_addr), minfo->pdev, minfo->dev_no));

    minfo->regs = minfo->base_addr;
    
    if(debug & DBG_LVL_DMAERR){
        msglevel |= DMA_DEBUG_MSG_ERR; 
    }  
    if(debug & DBG_LVL_DMA){
        msglevel |= DMA_DEBUG_MSG_TRACE; 
    } 

    for (i = 0; i < NUMRXQ; i++) {
        if (use_rx_skb_chan & (1 << i)) {
            minfo->rx[i].buffer_size = rx_buffer_size;
        } else {
            minfo->rx[i].buffer_size = RXBUFSZ;
        }
    }

    /* dma attach */
    sprintf(name, "et%d", minfo->dev_no);    
    /* allocate dma resources for txqs */
    /* TX: TC_BK, RX: RX_Q0 */
    minfo->di[0] = dma_attach(minfo, "dma0", 0, 
                        DMAREG(minfo, DMA_TX, TX_Q0),
                        DMAREG(minfo, DMA_RX, RX_Q0),
                        NTXD, NRXD, minfo->rx[0].buffer_size, 
                        NRXBUFPOST, ETCGMAC_HWRXOFF,                        
                        &msglevel);
#if !defined(IPROC_CMICD)
    /* TX: TC_BE, RX: RX_Q1 */
    minfo->di[1] = dma_attach(minfo, "dma1", 1, 
                        DMAREG(minfo, DMA_TX, TX_Q1),
                        DMAREG(minfo, DMA_RX, RX_Q1),
                        NTXD, NRXD, minfo->rx[1].buffer_size, 
                        NRXBUFPOST, ETCGMAC_HWRXOFF,                        
                        &msglevel);
    /* TX: TC_CL, RX: RX_Q2 */
    minfo->di[2] = dma_attach(minfo, "dma2", 2, 
                        DMAREG(minfo, DMA_TX, TX_Q2),
                        DMAREG(minfo, DMA_RX, RX_Q2),
                        NTXD, NRXD, minfo->rx[2].buffer_size, 
                        NRXBUFPOST, ETCGMAC_HWRXOFF,                        
                        &msglevel);
    /* TX: TC_VO, RX: RX_Q3 */
    minfo->di[3] = dma_attach(minfo, "dma3", 3, 
                        DMAREG(minfo, DMA_TX, TX_Q3),
                        DMAREG(minfo, DMA_RX, RX_Q3),
                        NTXD, NRXD, minfo->rx[3].buffer_size, 
                        NRXBUFPOST, ETCGMAC_HWRXOFF,                        
                        &msglevel);
#endif

    for (i = 0; i < NUMTXQ; i++) {
        if (minfo->di[i] == NULL) {
            gprintk("dma_attach failed\n");            
            return NULL;
        } else {
            sema_init(&minfo->tx[i].sem, 1);

        }
    }

    spin_lock_init(&minfo->lock);

    init_timer(&minfo->timer);
    minfo->timer.expires = jiffies + POLL_INTERVAL;
    minfo->timer.data = (unsigned long)minfo;
    minfo->timer.function = bkn_timer;
    add_timer(&minfo->timer);

#if !(NAPI_SUPPORT)
    /* setup the bottom half handler */
    tasklet_init(&minfo->tasklet, bkn_dpc, (ulong)minfo);
#endif	

    for (i = 0; i < NUMTXQ; i++) {
        INIT_LIST_HEAD(&minfo->tx[i].api_dcb_list);
        INIT_LIST_HEAD(&minfo->rx[i].api_dcb_list);
    }


    list_add_tail(&minfo->list, &_minfo_list);

    return  minfo;
}    


/*
 * Generic module functions
 */

static int
_pprint(void)
{	
    pprintf("Broadcom BCM ROBO KNET Linux Network Driver\n");

    return 0;
}

static int
bkn_knet_dma_info(kcom_msg_dma_info_t *kmsg, int len)
{
    bkn_gmac_info_t *minfo;
    bkn_dcb_chain_t *dcb_chain;
    unsigned long flags;
    int chan;


    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    minfo = bkn_minfo_from_unit(kmsg->hdr.unit);
    if (minfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    chan = kmsg->dma_info.chan;
    dcb_chain = kmalloc(sizeof(*dcb_chain), GFP_KERNEL);
    if (dcb_chain == NULL) {
        gprintk("Fatal error: No memory for dcb_chain\n");
        return 0;
    }
    memset(dcb_chain, 0, sizeof(*dcb_chain));
    dcb_chain->dcb_cnt = kmsg->dma_info.cnt;
    dcb_chain->dcb_dma = kmsg->dma_info.data.dcb_start;

    dcb_chain->eth_dcb = (eth_dcb_t *)kernel_bde->p2l(minfo->dev_no,
                                         dcb_chain->dcb_dma);


    if (kmsg->dma_info.type == KCOM_DMA_INFO_T_TX_DCB) {    
        spin_lock_irqsave(&minfo->lock, flags);
         if (chan == tx_chan_inuse) {
             /* Hold back packets from kernel */
             bkn_suspend_tx(minfo, chan);
        }
        list_add_tail(&dcb_chain->list, &minfo->tx[chan].api_dcb_list);

         /* Acquire Tx resources */
         if (down_trylock(&minfo->tx[chan].sem) == 0) {
             if (minfo->tx[chan].api_active == 0 && 
                minfo->di[chan]->txavail > 1) {
                 bkn_api_tx(minfo, chan);
             }
             up(&minfo->tx[chan].sem);
         }
         spin_unlock_irqrestore(&minfo->lock, flags);

     } else if (kmsg->dma_info.type == KCOM_DMA_INFO_T_RX_DCB) {
         spin_lock_irqsave(&minfo->lock, flags);
         list_add_tail(&dcb_chain->list, &minfo->rx[chan].api_dcb_list);
         bkn_api_rx(minfo, chan);        
         spin_unlock_irqrestore(&minfo->lock, flags);

     } else {
         DBG_DCB(("Unknown DCB_INFO type (%d).\n", kmsg->dma_info.type));
         kfree(dcb_chain);
         return 0;
     }

     return sizeof(kcom_msg_hdr_t);


}

static int
bkn_knet_version(kcom_msg_version_t *kmsg, int len)
{
    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;
    kmsg->version = KCOM_VERSION;
    kmsg->netif_max = KCOM_NETIF_MAX;
    kmsg->filter_max = KCOM_FILTER_MAX;

    return sizeof(kcom_msg_version_t);
}

static int
bkn_knet_eth_hw_config(kcom_msg_eth_hw_config_t *kmsg, int len)
{
    bkn_gmac_info_t *minfo;
    unsigned long flags,config_flags, val;
    int chan,type;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;
    DBG_CMD(("kmsg unit %d\n",kmsg->hdr.unit));


    minfo = bkn_minfo_from_unit(kmsg->hdr.unit);
    if (minfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }

    spin_lock_irqsave(&minfo->lock, flags);

    type = kmsg->config.type;
    chan = kmsg->config.chan;
    config_flags = kmsg->config.flags;
    val = kmsg->config.value;

    DBG_CMD(("bkn_knet_eth_hw_config chan %d type %d flags %lx val %lx\n",
        chan, type,config_flags, val));

    if ( KCOM_ETH_HW_T_INIT == type) {
        bkn_dma_init(minfo, chan, config_flags, val);
    } else if ( KCOM_ETH_HW_T_RESET == type) {
        bkn_dma_reset(minfo, chan, config_flags, val);
    } else if ( KCOM_ETH_HW_T_OTHER == type) {
        bkn_dma_config(minfo, chan, config_flags, val);
    } else {
        DBG_DCB(("Unknown ETH_HW_CONFIG type (%d).\n", kmsg->config.type));
        spin_unlock_irqrestore(&minfo->lock, flags);
        return 0;
    }

    spin_unlock_irqrestore(&minfo->lock, flags);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_netif_create(kcom_msg_netif_create_t *kmsg, int len)
{
    bkn_gmac_info_t *minfo;
    struct net_device *dev;
    struct list_head *list;
    bkn_priv_t *priv, *lpriv;
    int found, id;
    uint8 *ma;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;


    switch (kmsg->netif.type) {
    case KCOM_NETIF_T_VLAN:
    case KCOM_NETIF_T_PORT:
    case KCOM_NETIF_T_META:
        break;
    default:
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    minfo = bkn_minfo_from_unit(kmsg->hdr.unit);
    if (minfo == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    ma = kmsg->netif.macaddr;
    if ((ma[0] | ma[1] | ma[2] | ma[3] | ma[4] | ma[5]) == 0) {
        bkn_dev_mac[5]++;
        ma = bkn_dev_mac;
    }
    if ((dev = bkn_init_ndev(ma, kmsg->netif.name)) == NULL) {
        kmsg->hdr.status = KCOM_E_RESOURCE;
        return -1;
    }
    priv = netdev_priv(dev);
    priv->dev = dev;
    priv->minfo = minfo;

    priv->switch_arch = kmsg->hdr.reserved;

#if NAPI_SUPPORT
    netif_napi_add(dev, &priv->napi, bkn_poll, 16);        
#endif
    
    priv->type = kmsg->netif.type;
    priv->vlan = kmsg->netif.vlan;
    if (priv->type == KCOM_NETIF_T_PORT) {
        priv->port = kmsg->netif.port;
    } else {
        priv->port = -1;
    }
    priv->flags = kmsg->netif.flags;


    /* Prevent (incorrect) compiler warning */
    lpriv = NULL;

    /*
     * We insert network interfaces sorted by ID.
     * In case an interface is destroyed, we reuse the ID
     * the next time an interface is created.
     */
    found = 0;
    id = 1;
    list_for_each(list, &_ndev_list) {
        lpriv = (bkn_priv_t *)list;
        if (id < lpriv->id) {
            found = 1;
            break;
        }
        id = lpriv->id + 1;
    }
    priv->id = id;
    if (found) {
        /* Replace previously removed interface */
        list_add_tail(&priv->list, &lpriv->list);
    } else {
        /* No holes - add to end of list */
        list_add_tail(&priv->list, &_ndev_list);
    }

    DBG_VERB(("Assigned ID %d to Ethernet device %s\n",
              priv->id, dev->name));

    kmsg->netif.id = priv->id;
    memcpy(kmsg->netif.macaddr, dev->dev_addr, 6);
    memcpy(kmsg->netif.name, dev->name, KCOM_NETIF_NAME_MAX - 1);

    return sizeof(*kmsg);
}

static int
bkn_knet_netif_destroy(kcom_msg_netif_destroy_t *kmsg, int len)
{
    struct net_device *dev;
    bkn_priv_t *priv;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (kmsg->hdr.id == priv->id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    list_del(&priv->list);
    dev = priv->dev;
    DBG_VERB(("Removing virtual Ethernet device %s (%d).\n",
              dev->name, priv->id));
    unregister_netdev(dev);
    free_netdev(dev);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_netif_list(kcom_msg_netif_list_t *kmsg, int len)
{
    bkn_priv_t *priv;
    struct list_head *list;
    int idx;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    idx = 0;
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        kmsg->id[idx] = priv->id;
        idx++;
    }
    kmsg->ifcnt = idx;

    return sizeof(*kmsg) - sizeof(kmsg->id) + (idx * sizeof(kmsg->id[0]));
}

static int
bkn_knet_netif_get(kcom_msg_netif_get_t *kmsg, int len)
{
    bkn_priv_t *priv;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_ndev_list) {
        priv = (bkn_priv_t *)list;
        if (kmsg->hdr.id == priv->id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    memcpy(kmsg->netif.macaddr, priv->dev->dev_addr, 6);
    memcpy(kmsg->netif.name, priv->dev->name, KCOM_NETIF_NAME_MAX - 1);
    kmsg->netif.vlan = priv->vlan;
    kmsg->netif.type = priv->type;
    kmsg->netif.id = priv->id;
    kmsg->netif.flags = priv->flags;

    if (priv->port < 0) {
        kmsg->netif.port = 0;
    } else {
        kmsg->netif.port = priv->port;
    }

    return sizeof(*kmsg);
}

static int
bkn_knet_filter_create(kcom_msg_filter_create_t *kmsg, int len)
{
    struct list_head *list;
    bkn_filter_t *filter, *lfilter;
    int found, id;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    switch (kmsg->filter.type) {
    case KCOM_FILTER_T_RX_PKT:
        break;
    default:
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }

    /*
     * Find available ID
     */
    found = 1;
    id = 0;
    while (found && ++id < 256) {
        found = 0;
        list_for_each(list, &_filter_list) {
            lfilter = (bkn_filter_t *)list;
            if (id == lfilter->kf.id) {
                found = 1;
                break;
            }
        }
    }
    if (found) {
        /* Too many filters */
        kmsg->hdr.status = KCOM_E_RESOURCE;
        return -1;
    }

    filter = kmalloc(sizeof(*filter), GFP_KERNEL);
    if (filter == NULL) {
        kmsg->hdr.status = KCOM_E_PARAM;
        return -1;
    }
    memset(filter, 0, sizeof(*filter));
    memcpy(&filter->kf, &kmsg->filter, sizeof(filter->kf));
    filter->kf.id = id;

    filter->switch_arch = kmsg->hdr.reserved;

    /* Add according to priority */
    found = 0;
    list_for_each(list, &_filter_list) {
        lfilter = (bkn_filter_t *)list;
        if (filter->kf.priority < lfilter->kf.priority) {
            list_add_tail(&filter->list, &lfilter->list);
            found = 1;
            break;
        }
    }
    if (!found) {
        list_add_tail(&filter->list, &_filter_list);
    }

    kmsg->filter.id = filter->kf.id;

    DBG_VERB(("Created filter ID %d (%s).\n",
              filter->kf.id, filter->kf.desc));

    return len;
}

static int
bkn_knet_filter_destroy(kcom_msg_filter_destroy_t *kmsg, int len)
{
    bkn_filter_t *filter;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        if (kmsg->hdr.id == filter->kf.id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    list_del(&filter->list);
    DBG_VERB(("Removing filter ID %d.\n", filter->kf.id));
    kfree(filter);

    return sizeof(kcom_msg_hdr_t);
}

static int
bkn_knet_filter_list(kcom_msg_filter_list_t *kmsg, int len)
{
    bkn_filter_t *filter;
    struct list_head *list;
    int idx;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    idx = 0;
    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        kmsg->id[idx] = filter->kf.id;
        idx++;
    }
    kmsg->fcnt = idx;

    return sizeof(*kmsg) - sizeof(kmsg->id) + (idx * sizeof(kmsg->id[0]));
}

static int
bkn_knet_filter_get(kcom_msg_filter_get_t *kmsg, int len)
{
    bkn_filter_t *filter;
    struct list_head *list;
    int found;

    kmsg->hdr.type = KCOM_MSG_TYPE_RSP;

    found = 0;
    list_for_each(list, &_filter_list) {
        filter = (bkn_filter_t *)list;
        if (kmsg->hdr.id == filter->kf.id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        kmsg->hdr.status = KCOM_E_NOT_FOUND;
        return -1;
    }

    memcpy(&kmsg->filter, &filter->kf, sizeof(kmsg->filter));

    len = sizeof(*kmsg);

    return len;
}



static int
bkn_handle_cmd_req(kcom_msg_t *kmsg, int len)
{
    /* Silently drop events and unrecognized message types */
    if (kmsg->hdr.type != KCOM_MSG_TYPE_CMD) {
        if (kmsg->hdr.opcode == KCOM_M_STRING) {
            DBG_VERB(("Debug string: '%s'\n", kmsg->string.val));
            return 0;
        }
        DBG_VERB(("Unsupported message (type=%d, opcode=%d)\n",
                  kmsg->hdr.type, kmsg->hdr.opcode));
        return 0;
    }

    switch (kmsg->hdr.opcode) {
    case KCOM_M_DMA_INFO:
        DBG_VERB(("KCOM_M_DMA_INFO\n"));
        /* Packet buffer */
        len = bkn_knet_dma_info(&kmsg->dma_info, len);
        break;
    case KCOM_M_VERSION:
        DBG_VERB(("KCOM_M_VERSION\n"));
        /* Return procotol version */
        len = bkn_knet_version(&kmsg->version, len);
        break;
    case KCOM_M_ETH_HW_CONFIG:
        DBG_VERB(("KCOM_M_ETH_HW_CONFIG\n"));
        len = bkn_knet_eth_hw_config(&kmsg->eth_hw_config, len);
        break;
    case KCOM_M_NETIF_CREATE:
        DBG_VERB(("KCOM_M_NETIF_CREATE\n"));
        /* Create network interface */
        len = bkn_knet_netif_create(&kmsg->netif_create, len);
        break;
    case KCOM_M_NETIF_DESTROY:
        DBG_VERB(("KCOM_M_NETIF_DESTROY\n"));
        /* Destroy network interface */
        len = bkn_knet_netif_destroy(&kmsg->netif_destroy, len);
        break;
    case KCOM_M_NETIF_LIST:
        DBG_VERB(("KCOM_M_NETIF_LIST\n"));
        /* Return list of IDs of installed network interfaces */
        len = bkn_knet_netif_list(&kmsg->netif_list, len);
        break;
    case KCOM_M_NETIF_GET:
        DBG_VERB(("KCOM_M_NETIF_GET\n"));
        /* Return network interface info */
        len = bkn_knet_netif_get(&kmsg->netif_get, len);
        break;
    case KCOM_M_FILTER_CREATE:
        DBG_VERB(("KCOM_M_FILTER_CREATE\n"));
        /* Create packet filter */
        len = bkn_knet_filter_create(&kmsg->filter_create, len);
        break;
    case KCOM_M_FILTER_DESTROY:
        DBG_VERB(("KCOM_M_FILTER_DESTROY\n"));
        /* Destroy packet filter */
        len = bkn_knet_filter_destroy(&kmsg->filter_destroy, len);
        break;
    case KCOM_M_FILTER_LIST:
        DBG_VERB(("KCOM_M_FILTER_LIST\n"));
        /* Return list of IDs of installed packet filters */
        len = bkn_knet_filter_list(&kmsg->filter_list, len);
        break;
    case KCOM_M_FILTER_GET:
        DBG_VERB(("KCOM_M_FILTER_GET\n"));
        /* Return packet filter info */
        len = bkn_knet_filter_get(&kmsg->filter_get, len);
        break;
    default:
        DBG_VERB(("Unsupported command (type=%d, opcode=%d)\n",
                  kmsg->hdr.type, kmsg->hdr.opcode));
        kmsg->hdr.opcode = 0;
        len = sizeof(kcom_msg_hdr_t);
        break;
    }
    return len;
}


static int
bkn_cmd_thread(void *context)
{
    bkn_thread_ctrl_t *tc = (bkn_thread_ctrl_t *)context;
    kcom_msg_t kmsg;
    unsigned int len, rlen;

    bkn_thread_boot(tc);

    DBG_VERB(("Command thread starting\n"));
    tc->state = 1;
    while (!bkn_thread_should_stop(tc)) {
        len = sizeof(kmsg);
        tc->state = 2;
       if(PROXY_RECV(KCOM_CHAN_KNET, &kmsg, &len) >= 0){
            DBG_VERB(("Received %d bytes from KCOM_CHAN_CMD\n", len));
            tc->state = 3;
            rlen = bkn_handle_cmd_req(&kmsg, len);               
            tc->state = 4;
            if (rlen > 0) {
              PROXY_SEND(KCOM_CHAN_KNET, &kmsg, rlen);
            }
        } else {
        /* Thread interrupted */
            bkn_sleep(1);
        }
    }
    DBG_VERB(("Command thread done\n"));

    bkn_thread_exit(tc);
    return 0;
}

static int
bkn_get_next_dma_event(kcom_msg_dma_info_t *kmsg)
{
    static int last_dev_no = 0;
    bkn_gmac_info_t *minfo;
    unsigned long flags;
    int dev_no;

    dev_no = last_dev_no;

    while (1) {
        dev_no++;
        if (dev_no > kernel_bde->num_devices(BDE_ALL_DEVICES)){
            dev_no = 0;
        }
        minfo = bkn_minfo_from_unit(dev_no);
        if (minfo == NULL) {
            minfo = bkn_minfo_from_unit(dev_no);
        }

        if (minfo && minfo->dma_events) {
            DBG_VERB(("Next DMA events (0x%08x)\n", minfo->dma_events));
            kmsg->hdr.unit = minfo->dev_no;

            spin_lock_irqsave(&minfo->lock, flags);
            kmsg->dma_info.flags = minfo->dma_events;
            if (minfo->dma_events & KCOM_DMA_INFO_F_RX_DONE){
                kmsg->dma_info.chan = minfo->rx_active_chan;
                kmsg->dma_info.data.seqno.rx = minfo->rx_desc_seqno;
                minfo->rx_active_chan = 0;
            } 
            if (minfo->dma_events & KCOM_DMA_INFO_F_TX_DONE){
                kmsg->dma_info.data.seqno.tx = minfo->tx_desc_seqno;
            }

            minfo->dma_events = 0;
            spin_unlock_irqrestore(&minfo->lock, flags);

            last_dev_no = dev_no;
            break;
        }

        if (dev_no == last_dev_no) {
            wait_event_interruptible(evt_wq,  evt_wq_get != evt_wq_put);
            DBG_VERB(("Event thread wakeup\n"));

            /* Thread interrupted */
            if (signal_pending(current)) {
               return 0;
            }

            evt_wq_get = evt_wq_put;
        }
    }
    return sizeof(*kmsg);
}

static int
bkn_evt_thread(void *context)
{
    bkn_thread_ctrl_t *tc = (bkn_thread_ctrl_t *)context;
    kcom_msg_dma_info_t kmsg;
    int len;

    bkn_thread_boot(tc);

    memset(&kmsg, 0, sizeof(kmsg));
    kmsg.hdr.type = KCOM_MSG_TYPE_EVT;
    kmsg.hdr.opcode = KCOM_M_DMA_INFO;

    DBG_VERB(("Event thread starting\n"));
    tc->state = 1;
    while (!bkn_thread_should_stop(tc)) {
        tc->state = 2;
        len = bkn_get_next_dma_event(&kmsg);
        tc->state = 3;
        if (len) {
            PROXY_SEND(KCOM_CHAN_KNET, &kmsg, len);
        } else {
            /* Thread interrupted */
            bkn_sleep(1);
        }
    }
    DBG_VERB(("Event thread done\n"));

    bkn_thread_exit(tc);
    return 0;
}

static int
_cleanup(void)
{
    struct list_head *list;
    struct net_device *dev;
    bkn_filter_t *filter;
    bkn_priv_t *priv;
    bkn_gmac_info_t *minfo;
    unsigned long flags;

    /* Shut down event thread */
    bkn_thread_stop(&bkn_evt_ctrl);

    /* Shut down command thread */
    bkn_thread_stop(&bkn_cmd_ctrl);

    /* Remove KCOM channel */
    PROXY_SERVICE_DESTROY(KCOM_CHAN_KNET); 

    bkn_proc_cleanup();
    remove_proc_entry("bcm/knet", NULL);
    remove_proc_entry("bcm", NULL);

    list_for_each(list, &_minfo_list) {
        minfo = (bkn_gmac_info_t *)list;

        spin_lock_irqsave(&minfo->lock, flags);

        _irq_disable(minfo);

        DBG_IRQ(("Unregister ISR.\n"));
        kernel_bde->interrupt_disconnect(minfo->dev_no);

        bkn_dma_abort(minfo);
        spin_unlock_irqrestore(&minfo->lock, flags);
    }

    /* Destroy all filters */
    while (!list_empty(&_filter_list)) {
	filter = list_entry(_filter_list.next, bkn_filter_t, list);	
	list_del(&filter->list);
        DBG_VERB(("Removing filter ID %d.\n", filter->kf.id));
        kfree(filter);
    }

    /* Destroy all virtual net devices */
    while (!list_empty(&_ndev_list)) {
	priv = list_entry(_ndev_list.next, bkn_priv_t, list);	
	list_del(&priv->list);
        dev = priv->dev;
        DBG_VERB(("Removing virtual Ethernet device %s.\n", dev->name));
        unregister_netdev(dev);
        free_netdev(dev);
    }

    /* Destroy all switch devices */
    while (!list_empty(&_minfo_list)) {
    minfo = list_entry(_minfo_list.next, bkn_gmac_info_t, list);  
        if (minfo->dev) {
            DBG_VERB(("Removing Ethernet device %s.\n", minfo->dev->name));
            unregister_netdev(minfo->dev);
            free_netdev(minfo->dev);
        }
        DBG_VERB(("Removing switch device.\n"));
        bkn_destroy_minfo(minfo);
    }

    return 0;
}


static int
_init(void)
{
    int idx;
    int num_dev;
    uint32_t dev_type;
    struct net_device *dev;
    bkn_gmac_info_t *minfo;
    bkn_priv_t *priv;
    char *bdev_name;

    /* Connect to the kernel bde */
    if ((linux_bde_create(NULL, &kernel_bde) < 0) || kernel_bde == NULL) {
        return -ENODEV;
    }

    /* Randomize Lower 3 bytes of the MAC address (TESTING ONLY) */
    get_random_bytes(&bkn_dev_mac[3], 3);

    /* Check for user-supplied MAC address (recommended) */
    if (mac_addr != NULL && strlen(mac_addr) == 17) {
        for (idx = 0; idx < 6; idx++) {
            bkn_dev_mac[idx] = simple_strtoul(&mac_addr[idx*3], NULL, 16);
        }
        /* Do not allow multicast address */
        bkn_dev_mac[0] &= ~0x01;
    }

    /* Base network device name */
    bdev_name = "bcm%d";
    if (base_dev_name) {
        if (strlen(base_dev_name) < IFNAMSIZ) {
            bdev_name = base_dev_name;
        } else {
            DBG_VERB(("Base device name too long\n"));
        }
    }

    num_dev = kernel_bde->num_devices(BDE_ALL_DEVICES);
    for (idx = 0; idx < num_dev; idx++) {
        dev_type = kernel_bde->get_dev_type(idx);
        DBG_VERB(("Found device type 0x%x\n", dev_type));
        if (!(dev_type & BDE_ETHER_DEV_TYPE)) {
            DBG_VERB(("Not eth device - skipping\n"));
            continue;
        }
        if ((minfo = bkn_create_minfo(idx)) == NULL) {
            _cleanup();
            return -ENOMEM;
        }


        /* Register interrupt handler */
        kernel_bde->interrupt_connect(minfo->dev_no,
                                      bkn_isr, minfo);

        /* Create base virtual net device */
        bkn_dev_mac[5]++;
        if ((dev = bkn_init_ndev(bkn_dev_mac, bdev_name)) == NULL) {
            _cleanup();
            return -ENOMEM;
        } else {
            minfo->dev = dev;
            priv = netdev_priv(dev);
            priv->dev = dev;
            priv->minfo = minfo;
            priv->vlan = 1;
            priv->port = -1;
            priv->id = -1;
        }
	}

    /* Initialize proc files */
    proc_mkdir("bcm", NULL);
    bkn_proc_root = proc_mkdir("bcm/knet", NULL);

    bkn_proc_init();

    init_waitqueue_head(&evt_wq);

    if (use_proxy) {
        PROXY_SERVICE_CREATE(KCOM_CHAN_KNET, 1, 0); 

        DBG_VERB(("Starting command thread\n"));
        bkn_thread_start(&bkn_cmd_ctrl, "bkncmd", bkn_cmd_thread);

        DBG_VERB(("Starting event thread\n"));
        bkn_thread_start(&bkn_evt_ctrl, "bknevt", bkn_evt_thread);
    }

#if defined(IPROC_CMICD)
    /* only one channel on this platform  */
    tx_chan_inuse = 0;
#else
    tx_chan_inuse = use_tx_skb;
#endif

    return 0;
}

static int 
_ioctl(unsigned int cmd, unsigned long arg)
{

    return 0;
}

static gmodule_t _gmodule = {
    name: MODULE_NAME, 
    major: MODULE_MAJOR, 
    init: _init,
    cleanup: _cleanup, 
    pprint: _pprint, 
    ioctl: _ioctl,
    open: NULL, 
    close: NULL, 
}; 


gmodule_t*
gmodule_get(void)
{
    EXPORT_NO_SYMBOLS;
    return &_gmodule;
}

