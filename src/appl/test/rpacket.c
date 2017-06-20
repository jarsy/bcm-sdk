/*
 * $Id: rpacket.c,v 1.81 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Packet receive test that uses the BCM api.
 *
 * December, 2002:  Options added to support RX APIs.
 *    This also adds:
 *        interrupt/non-interrupt handling;
 *        packet stealing;
 *        if stealing, freeing packets with DPC (always with interrupt,
 *               optionally with non-interrupt) or immediately;
 *
 *    When stealing packets, the system will crash if the rate is
 *    high enough:  Over 4K pkts/sec on Mousse, and 6K pkts/sec on BMW.
 *    This does not appear to be related to DPC.
 *
 *    Current max rates for RX API, no packet stealing:
 *       BMW (8245) on 5690, interrupt:           11.6 K pkts/sec
 *       BMW, non-interrupt:                      11.6 K pkts/sec
 *       Mousse, non-interrupt:                   10.4 K pkts/sec
 *
 *    Update on max rates for RX API:
 */

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/stats.h>
#include <sal/appl/sal.h>
#include <sal/core/dpc.h>
#include <shared/bsl.h>
#include <soc/util.h>
#include <soc/drv.h>
#ifdef BCM_FE2000_SUPPORT
#include <soc/sbx/sbx_drv.h>
#ifdef BCM_FE2000_SUPPORT
#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* def BCM_FE2000_P3_SUPPORT */
#endif /* def BCM_FE2000_SUPPORT */
#endif

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/shell.h>

#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/mcast.h>
#include <bcm/port.h>
#include <bcm/link.h>
#include <bcm/field.h>
#include <bcm/rx.h>
#include <bcm/pkt.h>
#include <bcm/stack.h>
#include <bcm/vlan.h>

#include <bcm_int/common/rx.h>
#include <bcm_int/common/tx.h>
#if defined (BCM_PETRA_SUPPORT)
#include <soc/dpp/drv.h>
#endif

#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
#include <bcm/knet.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/mman.h>
    
#include <sched.h>
#include <sal/core/thread.h>
#endif


#include "testlist.h"

#if defined(BCM_FIELD_SUPPORT) || defined(BCM_FE2000_SUPPORT) || \
    defined(BCM_ROBO_SUPPORT) || defined (BCM_PETRA_SUPPORT)

typedef struct p_s {
    int                         p_init; /* TRUE --> initialized */

    /* parameters changed or watched by other threads -- must be volatile */
    volatile uint32             p_count_packets; /* Finished test - just drain */
    volatile uint32             p_received;
    volatile uint32             p_drained;
    volatile uint32             p_time_us;
    volatile uint32             p_error_num;

    /* internal parameters */
    int                         p_num_test_ports;
    int                         p_use_fp_action;
    int                         p_port[2]; /* Active port */
    int                         p_time; /* Duration (seconds) */
    bcm_port_info_t             p_port_info[2]; /* saved port config */
    int                         p_pkts_per_sec;
    int                         p_hw_rate;  /* use hw rate limiting */
    int                         p_max_q_len;
    int                         p_per_cos; /* Run tests with per-cos settings */
    int                         p_burst;
    uint32                      reg_flags;  /* Flags used for register */
    int                         p_intr_cb;
    int                         p_dump_rx;
#if defined(BCM_FIELD_SUPPORT)
    bcm_field_entry_t           p_field_entry[2];
#endif /* BCM_FIELD_SUPPORT */
    int                         p_l_start; /* Length start */
    int                         p_l_end; /* Length end */
    int                         p_l_inc; /* Length increment */
    int                         p_free_buffer;
    bcm_pkt_t                   *p_pkt; /* Tx - Packet buffer */
    bcm_rx_cfg_t                p_rx_cfg;
    int                         rx_mode;
    int                         p_txrx_unit;
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
    int                         p_use_socket;
    int                         p_sock;
    int                         p_netid;
    int                         p_filterid;
    sal_thread_t                p_threadid;
    int                         p_ringbuf_size;
    char *                      p_ringbuf;
    int                         p_use_socket_send;
#endif
} p_t;

static sal_mac_addr_t rp_mac_dest = {0x00, 0x11, 0x22, 0x33, 0x44, 0x99};
static sal_mac_addr_t rp_mac_src  = {0x00, 0x11, 0x22, 0x33, 0x44, 0x66};

#ifdef BCM_PETRA_SUPPORT
int is_force_local = 0;
int is_force_fabric = 0;
static uint8 rp_ptch_header[2]  = {0xd0, 0x00};
static uint8 mypacket[] = {


/*
                          0x0,0x0,0x0,0x0,       0x0,0x1,0x0,0x0,
                          0x0,0x0,0x0,0xe2,      0x81,0x00,0x00,0x00,
                          
                          0x90,0x00,*/0x1,0x2, 




/*                          0x0,0x0,0x0,0x0,       0x0,0x1,0x0,0x0,
                          0x0,0x0,0x0,0xe2,      0x90,0x00,0x1,0x2,
                          
                          0x03,0x4,0x5,0x6, */     0x7,0x8,0x9,0xa,
                          0xb,0xc,0xd,0xe,       0xf,0x10,0x11,0x12,
                          
                          0x13,0x14,0x15,0x16,   0x17,0x18,0x19,0x1a,
                          0x1b,0x1c,0x1d,0x1e,   0x1f,0x20,0x21,0x22,
                          
                          0x23,0x24,0x25,0x26,   0x27,0x28,0x29,0x2a,
                          0x2b,0x2c,0x2d,0x2e,   0x2f,0x30,0x31,0x32,
                          
                          0x33,0x34,0x35,0x36,   0x37,0x38,0x39,0x3a,
                          0x3b,0x3c,0x3d,0x3e,   0x3f,0x40,0x41,0x42,

                          0x43,0x44,0x45,0x46,   0x47,0x48,0x49,0x4a,
                          0x4b,0x4c,0x4d,0x4e,   0x4f,0x50,0x51,0x52,


                          0x53,0x54,0x55,0x56,   0x57,0x58,0x59,0x5a,
                          0x5b,0x5c,0x5d,0x5e,   0x5f,0x60,0x61,0x62,

                          0x63,0x64,0x65,0x66,   0x67,0x68,0x69,0x6a,
                          0x6b,0x6c,0x6d,0x6e,   0x6f,0x70,0x71,0x72,

                          0x73,0x74,0x75,0x76,   0x77,0x78,0x79,0x7a,
                          0x7b,0x7c,0x7d,0x7e,   0x7f,0x80,0x81,0x82 };
#endif


static p_t *p_control[SOC_MAX_NUM_DEVICES];

#define RP_RECEIVER_PRIO                255    /* Priority of rx handler */
#define RP_MAX_PKT_LENGTH               2048
#define RP_RX_CHANNEL                   1      /* DMA channel */

#ifdef BCM_ROBO_SUPPORT
/* Configurate rate value */ 
#define IMP_MIN_RATE_PPS   384    /* Packet Per Second */
#define IMP_MIN_RATE_BPS   256    /* KBits Per Second */
#define IMP_BURST_SIZE 48

static int imp_rate_pps = IMP_MIN_RATE_PPS;
static int imp_rate_default = 0;
static int imp_burst_size_default = 0;

static bcm_port_info_t  port_info[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS];
#endif /* BCM_ROBO_SUPPORT */

#define RP_CHK(rv, f)                      \
   if (BCM_FAILURE((rv))) {                \
       cli_out("call to %s line %d failed:%d %s\n", #f, __LINE__,\
               (rv), bcm_errmsg((rv))); \
    }


#define RP_MAX_PPC SOC_DV_PKTS_MAX
#ifdef BCM_FE2000_SUPPORT
#undef RP_MAX_PPC
#define RP_MAX_PPC SBX_MAX_RXLOAD_FIFO
#endif



STATIC void 
debug_dump(int len, uint8 *data)
{
    int i;
    char tbuf[128];

    sal_memset(tbuf, 0, 128);
    for (i = 0; i< len; i++) {
           sal_sprintf(tbuf+(i % 16)*3, "%02x ", data[i]);
           if (((i+1) % 16) == 0) {
               cli_out("%s\n", tbuf);
               sal_memset(tbuf, 0, 128);
           }
    }
    cli_out("%s\n", tbuf);
}


STATIC void
packet_measure(p_t *p, int s_len, int r_len, uint8 *buf)
{
    static sal_usecs_t  time_start = 0;
    sal_usecs_t  time_end = 0;

    /* keep accumulating stats */
    p->p_received++;
    /* one packet has been received; track stats */
    if (p->p_received == 1) {
        time_start = sal_time_usecs();
    } else {
        time_end = sal_time_usecs();
        /* again overkill to calculate on each packet, but
           the main test is going to make off with this value
           at any time, so we have to keep it up to date */
        p->p_time_us = (int)(time_end - time_start);
    }

    if (abs(r_len - s_len) > 4) {
        /* packet error */
        p->p_error_num++;
        if (p->p_error_num == 1) {
            cli_out("S:%d R:%d\n", s_len, r_len);
            debug_dump(96, buf);
        }
    }
}

#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))


#define RXRING_FRAME_SIZE  (2048)
#define MAX_RXRING_FRAMES  (1024)

static int idx = 0;


STATIC void
_set_thread_priority(int prio)
{
    struct sched_param param;
    param.sched_priority = prio;
    if (sched_setscheduler(0, SCHED_RR, &param)) {       
        cli_out("priority set: \n");
    }
}

STATIC int 
knet_stats_get(long unsigned int *ic, long unsigned int *tc)
{
    FILE* f;
    unsigned int tmp, int_cnt, pkt_cnt0, pkt_cnt1;

    if ((f=fopen("/proc/bcm/knet/stats", "rw" )) == NULL) {
        *tc = 0;
        return -1;
    }
    (void)fscanf(f, "Device stats (unit %d):\n"\
                        "  Tx packets  %10u\n"
                        "  Rx0 packets %10u\n"\
                        "  Rx1 packets %10u\n"\
                        "  Rx0 pkts/intr %8u\n"\
                        "  Rx1 pkts/intr %8u\n"\
                        "  Interrupts  %10u\n",
                        (int *)&tmp, &tmp, 
                        &pkt_cnt0, &pkt_cnt1,  
                        &tmp, &tmp, 
                        &int_cnt);
    * ic = int_cnt;                  
    * tc =  pkt_cnt0 + pkt_cnt1;            
    fclose(f);
    
    return 0;
    
}

STATIC int 
knet_stats_clear(void)
{
    FILE* f;

    if ((f=fopen("/proc/bcm/knet/stats", "w+" )) == NULL) {
        cli_out("fail to open stats for writing\n");
        return -1;
    }
    fwrite("clear", 5, 1, f);
    fclose(f);
    return 0;
    
}

#ifdef FIX_IDX_ORDER

STATIC int 
fix_idx(int idx ,volatile p_t* p, int *len) 
{
    int max = p-> p_ringbuf_size;
    int i =  (idx+1) % max;
    
    while (i != idx) {
#ifdef PACKET_VERSION
        volatile struct tpacket2_hdr *head = (volatile struct tpacket2_hdr *)
            (p->p_ringbuf + i * RXRING_FRAME_SIZE);
#else 
        volatile struct tpacket_hdr *head = (volatile struct tpacket_hdr *)
            (p->p_ringbuf + i * RXRING_FRAME_SIZE);        
#endif 
        if (head->tp_status & TP_STATUS_USER) {
            * len = head->tp_len;
            head->tp_len = 0;
            head->tp_snaplen = 0;
            head->tp_status = TP_STATUS_KERNEL;
            
            return i;
        }
        i=(i+1) % max;
    }
    return i;
}
#endif

STATIC int 
sock_read_one_pkt (volatile p_t* p, uint8 ** pbuf)
{
    static char sock_buf[RXRING_FRAME_SIZE];
    int len = 0;

    if (p->p_ringbuf) {   /* ring buffer receive */                 

#ifdef PACKET_VERSION
        volatile struct tpacket2_hdr *head = (volatile struct tpacket2_hdr *)
            (p->p_ringbuf + idx * RXRING_FRAME_SIZE);
#else 
        volatile struct tpacket_hdr *head = (volatile struct tpacket_hdr *)
            (p->p_ringbuf + idx * RXRING_FRAME_SIZE);        
#endif
        
        * pbuf = (uint8 *)head;

        if (head->tp_status & TP_STATUS_USER) { /* it's our packet */
            len = head->tp_len;
            /* release the ringbuf entry back to the kernel */
            head->tp_len = 0;
            head->tp_snaplen = 0;
            head->tp_status = TP_STATUS_KERNEL;
        
            /* next packet */
            idx = (idx + 1) % (p->p_ringbuf_size);
        } else {
#ifdef FIX_IDX_ORDER
            int fixed_idx = fix_idx(idx, p, &len);
            if (fixed_idx !=  idx) {
                idx = (fixed_idx+1) %(p->p_ringbuf_size);
                
            } 
#endif
        }

    } else {  /* standard socket receive */
        * pbuf = (uint8 *)sock_buf;
        len = recvfrom(p->p_sock, sock_buf, RXRING_FRAME_SIZE, MSG_DONTWAIT, NULL, NULL);  
        if (len < 0) {
            len = 0;
        }
    }

    return len;
} /* sock_read_one_pkt*/


STATIC void 
socket_receive(void *arg)
{
    p_t *p = (p_t *)arg;
    int          len = 0;
    struct       pollfd pfd;    
    uint8 * buf;
   
    _set_thread_priority(50);
       
    if (p->p_sock <= 0) {
        cli_out("bad socket %d\n", p->p_sock);
        return;
    }

    pfd.fd = p->p_sock;
    pfd.events = POLLIN /*| POLLERR*/;  
    pfd.revents = 0;

    while (TRUE) {
        pfd.revents = 0;
        /* wait for data to arrive*/
        len = poll(&pfd, 1, 1000);

        if (len == -1) {
            cli_out("poll error: errno %d\n", errno);
            continue;   
        } else if (len == 0) { /* no ready fd's; should be timeout */
            continue;
        } else if ((pfd.revents | POLLIN) == 0) {
            cli_out("poll without data; revents %x\n", pfd.revents);
        }
  
        /* pull packet from socket until empty*/
        while ((len = sock_read_one_pkt (p, &buf)) > 0) {
            if (p->p_count_packets) {
                packet_measure(p, p->p_pkt->_pkt_data.len, len, buf);
            } else {
                p->p_drained++;
            }
        } /* while packet len > 0 */

    } /* while processing packets */

    cli_out("socket_receive thread done.\n");
    /* spin forever, waiting to be killed */
    for (;;) {
    }

} /* socket_receive */


STATIC int 
bind_device(int sock, char* dev)
{
    int rv;
    struct sockaddr_ll addr;    
    struct ifreq req;
    
    sal_memset(&req, 0, sizeof(req));
    sal_strncpy(req.ifr_name, dev, sizeof(req.ifr_name));
    if ((rv = ioctl(sock, SIOCGIFINDEX, &req)) < 0) {
        cli_out("ioctl error %d\n", errno);
        return rv;
    }  

    if ((rv = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, 
                        (void *)&req, sizeof(req))) < 0) {
        cli_out("bind_device setsockopt error %d\n", errno); 
        return rv;
    }

    sal_memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(0x8100);
    addr.sll_ifindex = req.ifr_ifindex;
    addr.sll_hatype = 0;
    addr.sll_pkttype = 0;
    addr.sll_halen = 0;
    if ((rv = bind(sock, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
        cli_out("bind error %d\n", errno);
        return rv;
    }
    return 0;
}

STATIC int
set_promisc_up(int sock,  char* dev)
{
    struct ifreq req;       
    sal_strncpy(req.ifr_name, dev, sizeof(req.ifr_name));
    
    if (ioctl(sock, SIOCGIFFLAGS, &req)==-1) {
        cli_out("ioctl error");
        return -1;
    }
    req.ifr_flags |= (IFF_PROMISC | IFF_UP | IFF_RUNNING);
    if (ioctl(sock, SIOCSIFFLAGS, &req)==-1) {
        cli_out("ioctl error");
        return -1;
    }
    return 0;
}
STATIC int
knetif_setup(int unit, p_t *p) 
{  
    int rv;
    bcm_knet_netif_t netif;
    bcm_knet_filter_t filter;
    struct tpacket_req  req;
    
    bcm_knet_netif_t_init(&netif);
    netif.type = BCM_KNET_NETIF_T_TX_CPU_INGRESS;
    sal_memcpy(netif.mac_addr, rp_mac_dest, 6);
    
    p->p_netid = 0;
    rv = bcm_knet_netif_create(unit, &netif);
    if (BCM_SUCCESS(rv)) {
        p->p_netid = netif.id;
    } else {
        cli_out("bcm_knet_netif_create failed: %d\n", rv);
        return rv;
    }
    
    p->p_filterid = 0;
    bcm_knet_filter_t_init(&filter);
    filter.type = BCM_KNET_FILTER_T_RX_PKT;
    if (p->p_use_socket) {
        filter.dest_type = BCM_KNET_DEST_T_NETIF;
        filter.dest_id = p->p_netid; 
    } else {
        filter.dest_type = BCM_KNET_DEST_T_BCM_RX_API;
    }
    rv = bcm_knet_filter_create(unit, &filter);
    if (BCM_SUCCESS(rv)) {
        p->p_filterid = filter.id;
    } else {
        cli_out("bcm_knet_filter_create:%d\n", rv);
        return rv;
    }
    
    p->p_sock = socket(AF_PACKET, SOCK_RAW, htons(0x8100));

    set_promisc_up(p->p_sock, "bcm0");
    set_promisc_up(p->p_sock, netif.name);

        /* set up ring buffer, if needed */
    if ((p->p_use_socket) && (p->p_ringbuf_size > 0)) {
        int buf_size = p->p_ringbuf_size * RXRING_FRAME_SIZE; 
#ifdef PACKET_VERSION 
        int tpacket_version = TPACKET_V2;
        /* set tpacket hdr version. */
        if (-1 == setsockopt (p->p_sock, SOL_PACKET, PACKET_VERSION, 
            &tpacket_version, sizeof (int))) {
            cli_out("set tpacket version failure.\n");
        }
#endif        
        idx = 0;

        req.tp_block_size = buf_size;
        req.tp_block_nr = 1;
        req.tp_frame_size = RXRING_FRAME_SIZE;
        req.tp_frame_nr = p->p_ringbuf_size;
        
        if (-1 == setsockopt(p->p_sock, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req))) {
            cli_out("setsockopt PACKET_RX_RING error\n");  
        }
        p->p_ringbuf = (char*)mmap(0, buf_size, PROT_READ|PROT_WRITE, MAP_SHARED, p->p_sock, 0);
        if (MAP_FAILED == p->p_ringbuf) {
            cli_out("mmap error\n");
            p->p_ringbuf = NULL;
            p->p_ringbuf_size = 0;
        } else {
            memset(p->p_ringbuf, 0, buf_size);
        }
    }
    bind_device(p->p_sock, netif.name);
    if (p->p_use_socket) {     
        p->p_threadid = sal_thread_create("sock_rx", 8192, 50, socket_receive, p);
    }
    return rv;
}

STATIC void
knetif_clean(int unit, p_t *p) 
{
    if (p->p_threadid != NULL) {
        sal_thread_destroy(p->p_threadid);
        p->p_threadid = NULL;
    }
    
    if (p->p_ringbuf != NULL) {
        struct tpacket_req  req;
        int buf_size = p->p_ringbuf_size * RXRING_FRAME_SIZE;
        sal_memset(&req, 0, sizeof(req));
        setsockopt(p->p_sock, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req));
        munmap(p->p_ringbuf, buf_size);
        p->p_ringbuf = NULL;
    }
    if (p->p_sock > 0) {
        struct ifreq req;
        bcm_knet_netif_t netif;
        bcm_knet_netif_get(unit, p->p_netid, &netif);
       
        sal_strncpy(req.ifr_name, netif.name, sizeof(req.ifr_name));
        
        if (ioctl(p->p_sock, SIOCGIFFLAGS, &req)==-1) {
            cli_out("ioctl SIOCGIFFLAGS error");
        }
        req.ifr_flags &= (~(IFF_UP | IFF_RUNNING));
        if (ioctl(p->p_sock, SIOCSIFFLAGS, &req)==-1) {
            cli_out("ioctl SIOCSIFFLAGS error");
        }
        close(p->p_sock);
        p->p_sock = 0;
    }
    if (p->p_filterid > 0) {
        bcm_knet_filter_destroy(unit, p->p_filterid);
        p->p_filterid = 0;
    }
    
    if (p->p_netid > 0) {
        bcm_knet_netif_destroy(unit, p->p_netid);
        p->p_netid = 0;
    }
}

#endif

/*
 * The following function may be called in interrupt context.
 * Does no packet classification; assume we're to see all pkts.
 */

STATIC bcm_rx_t
rpacket_rx_receive(int unit, bcm_pkt_t *pkt, void *vp)
{
    p_t         *p = (p_t *)vp;
    int          send_len = p->p_pkt->_pkt_data.len;
    int          len = pkt->tot_len;
    if (!p->p_count_packets) {
        /*
         * Just drain packets.
         */
        return BCM_RX_HANDLED;
    }

    packet_measure(p, send_len, len, (uint8 *)pkt->pkt_data->data);

    if (p->p_free_buffer) {
        if (p->p_intr_cb) { /* Queue in interrupt context */
            bcm_rx_free_enqueue(unit, pkt->_pkt_data.data);
        } else {
            bcm_rx_free(unit, pkt->alloc_ptr);
        }
        return BCM_RX_HANDLED_OWNED;
    }

    return BCM_RX_HANDLED;
}

STATIC INLINE int
rpacket_register(int unit, p_t *p)
{
    if (SOC_IS_ARAD(unit)) {
        return _bcm_common_rx_register(unit, "rpkt-rx", rpacket_rx_receive, RP_RECEIVER_PRIO, p, p->reg_flags);

    } else {
        return bcm_rx_register(unit, "rpkt-rx", rpacket_rx_receive,
                           RP_RECEIVER_PRIO, p, p->reg_flags);
    }
}

STATIC INLINE int
rpacket_unregister(int unit, p_t *p)
{
    if (SOC_IS_ARAD(unit)) {
        return _bcm_common_rx_unregister(unit, rpacket_rx_receive, RP_RECEIVER_PRIO);
    } else {
        return bcm_rx_unregister(unit, rpacket_rx_receive, RP_RECEIVER_PRIO);
    }
}

STATIC int
rpacket_receiver_activate(int unit, p_t *p)
{
    int rv;

    /* Set up common attributes first */
    if (bcm_rx_active(unit)) {
        cli_out("Stopping active RX.\n");
        rv = bcm_rx_stop(unit, NULL);
        if (!BCM_SUCCESS(rv)) {
            cli_out("Unable to stop RX: %s\n", bcm_errmsg(rv));
            return -1;
        }
    }

    if (!soc_feature(unit, soc_feature_packet_rate_limit)) {
        /* Only set the burst size if the unit doesn't have
         * CPU packet rate limiting feature in HW. Otherwise,
         * packets are dropped causing the test to fail.
         */
        if ( SOC_IS_ARAD(unit)) { 
            rv = _bcm_common_rx_burst_set(unit, p->p_burst);
        } else {
            rv = bcm_rx_burst_set(unit, p->p_burst);
        }
        if (BCM_FAILURE(rv)) {
            cli_out("Unable to set RX burst limit: %s\n",
                    bcm_errmsg(rv));
        }
    }

    if (p->p_hw_rate) { 
        if (p->p_per_cos) {
            if (!SOC_IS_KATANA(unit)) {
                bcm_rx_cos_rate_set(unit, 0, p->p_pkts_per_sec);
                bcm_rx_cos_burst_set(unit, 0, p->p_burst);
                p->p_rx_cfg.global_pps = 0;
                p->p_rx_cfg.max_burst = 0;
            }
        } else {
            rv = bcm_port_rate_egress_pps_set (unit, CMIC_PORT(unit), 
                              p->p_pkts_per_sec, p->p_burst);
        } 
    } else {
            bcm_rx_cos_rate_set(unit, BCM_RX_COS_ALL, 0);
            bcm_rx_cos_burst_set(unit, BCM_RX_COS_ALL, 0);
            p->p_rx_cfg.global_pps = p->p_pkts_per_sec;
            p->p_rx_cfg.max_burst = p->p_burst;
    }  
    if (p->p_max_q_len >= 0) {
        cli_out("Setting MAX Q length to %d\n", p->p_max_q_len);
        bcm_rx_cos_max_len_set(unit, BCM_RX_COS_ALL, p->p_max_q_len);
    }
    if (SOC_IS_ARAD(unit)) {
        rv = _bcm_common_rx_start(unit, &p->p_rx_cfg  );
    } else {
        rv = bcm_rx_start(unit, &p->p_rx_cfg  );
    }
    if (!BCM_SUCCESS(rv)) {
        cli_out("Unable to Start RX: %s\n", bcm_errmsg(rv));
        return -1;
    }

    
#ifdef BCM_FE2000_SUPPORT
    p->reg_flags = BCM_RCO_F_ALL_COS;
#else
    p->reg_flags = BCM_RCO_F_ALL_COS +
        (p->p_intr_cb ? BCM_RCO_F_INTR : 0);
#endif /* BCM_FE2000_SUPPORT */

    return 0;
}



STATIC int
rpacket_setup(int unit, p_t *p)
{
    int         got_port;
    int         rv = BCM_E_UNAVAIL;
    uint8       *fill_addr;
    soc_port_t  port;
    int         port_idx;
    pbmp_t      gxe_pbm;
    pbmp_t      tmp_pbmp;
    int         speed = 0;
    int         txrx_modid = 0;
#ifdef BCM_ROBO_SUPPORT
#if defined(BCM_FIELD_SUPPORT)
    pbmp_t      redirect_pbmp;
#endif /* BCM_FIELD_SUPPORT */
#endif /* BCM_ROBO_SUPPORT */
#ifdef BCM_FE2000_SUPPORT
    int         modid;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_epv2e_t p3epv2e;
#endif /* def BCM_FE2000_P3_SUPPORT */
#endif /* def BCM_FE2000_SUPPORT */
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
     pbmp_t wan_pbmp;
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */

    p->p_txrx_unit = unit;
    p->p_use_fp_action = 0;
    p->p_num_test_ports = 1;

    if (SOC_IS_TOMAHAWKX(unit) || SOC_IS_TRIDENT2(unit) || SOC_IS_HELIX4(unit)){
        p->p_use_fp_action = 1;
        p->p_num_test_ports = 2;
    }

#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE(unit)) {
        p->p_num_test_ports = 2;
    }
#endif /* def BCM_FE2000_SUPPORT */

#ifdef BCM_ROBO_SUPPORT
    /*
     * Pick a port, and redirect the packet back out that port with the
     * FFP (for bcm5395 and bcm53115).  On other ROBO chips, 2 ports are chosen, 
     * and redirect one to the other.
     */
    if (SOC_IS_ROBO(unit)) {
        if (SOC_IS_VULCAN(unit) ||
            SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
            SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
            SOC_IS_STARFIGHTER3(unit)) {
            p->p_num_test_ports = 1;
        } else {
            p->p_num_test_ports = 2;
        }
    }
#endif /* def BCM_ROBO_SUPPORT */

#ifdef BCM_PETRA_SUPPORT
    if(SOC_IS_ARAD(unit)) {
        p->p_num_test_ports = 2;
    }
#endif


    /*
     * Pick a port, and redirect the packet back out that port with the
     * FFP.  On SBX, 2 ports are chosen, and redirect one to the other.  Single
     * port redirection drops packets on ingress due to the split horizon check
     */

    got_port = FALSE;
    BCM_PBMP_ASSIGN(gxe_pbm, PBMP_GE_ALL(unit));
    BCM_PBMP_OR(gxe_pbm, PBMP_XE_ALL(unit));
    BCM_PBMP_OR(gxe_pbm, PBMP_CE_ALL(unit));

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        if (SOC_IS_VO(unit)) {
            BCM_PBMP_REMOVE(gxe_pbm,SOC_INFO(unit).gmii_pbm);
        }
       /* If there are no GE ports, then selects FE ports for some ROBO chips */
        if (BCM_PBMP_IS_NULL(gxe_pbm)) {
            BCM_PBMP_ASSIGN(gxe_pbm, PBMP_FE_ALL(unit));
        }
    }
#endif /* def BCM_ROBO_SUPPORT */

#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
        wan_pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
        SOC_PBMP_AND(wan_pbmp, PBMP_ALL(unit));
    }
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */


    port_idx = 0;
    PBMP_ITER(gxe_pbm, port) {
        if (IS_ST_PORT(unit, port)) {
            continue;
        }
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
        if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
            if (SOC_PBMP_MEMBER(wan_pbmp, port)) {
                cli_out("Skip WAN port(port%d)\n",port);
                continue;
            }
        }
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */
 
        rv = bcm_port_info_save(unit, port, &p->p_port_info[port_idx]);
        RP_CHK(rv, bcm_port_info_save);

        if (BCM_SUCCESS(rv)) {
            rv = bcm_port_linkscan_set(unit, port, BCM_LINKSCAN_MODE_NONE);
            RP_CHK(rv, bcm_port_linkscan_set);
        }
#ifdef BCM_PETRA_SUPPORT
        if ((SOC_IS_ARAD(unit) && SOC_DPP_PP_ENABLE(unit)) || (!SOC_IS_ARAD(unit))) 
#endif
        {
            if (BCM_SUCCESS(rv)) {
                rv = bcm_port_speed_max(unit, port, &speed);
                RP_CHK(rv, bcm_port_speed_max);
            }
            if (BCM_SUCCESS(rv)) {
                rv = bcm_port_speed_set(unit, port, speed);
                RP_CHK(rv, bcm_port_speed_set);
            }
        }
        if (BCM_SUCCESS(rv)) {
           if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
               rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_PHY);
#endif /* BCM_ROBO_SUPPORT */
           } else {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_FE2000_SUPPORT) || defined(BCM_PETRA_SUPPORT)
               if (SOC_IS_HELIX4(unit)) {
                   rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_PHY);
               } else {
                   rv = bcm_port_loopback_set(unit, port, BCM_PORT_LOOPBACK_MAC);
               }
#endif /* BCM_ESW_SUPPORT  || defined(BCM_FE2000_SUPPORT) */
           }
           RP_CHK(rv, bcm_port_loopback_set);
        }
        if (BCM_SUCCESS(rv)) {
            rv = bcm_port_pause_set(unit, port, 0, 0);
            RP_CHK(rv, bcm_port_pause_set);
        }

        if ((got_port = BCM_SUCCESS(rv))) {
            p->p_port[port_idx++] = port;

            if (port_idx >= p->p_num_test_ports) {
                break;
            }
            got_port = BCM_E_NOT_FOUND;
        }
    }

    if (!got_port) {
        test_error(unit, "Unable to find suitable XE/GE port.\n");
        return -1;
    }

#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE(unit)) {
        int node_id, fab_port; /* don't cares, just want the QE's Mod & unit */

        rv = bcm_stk_modid_get(unit, &modid);
        if (BCM_FAILURE(rv)) {
            test_error(unit, "Failed to find module id\n");
            return -1;
        }

        rv = soc_sbx_node_port_get (unit, modid, p->p_port[0] ,
                                    &p->p_txrx_unit,
                                    &node_id, &fab_port);
        if (BCM_FAILURE(rv)) {
            test_error(unit, "Failed to find QE Module\n");
            return -1;
        }
        SOC_SBX_MODID_FROM_NODE(node_id, txrx_modid);

    /*
     *  Set the egress port/vid to strip so the mac header doesn't accumulate
     *  to an uncontrollable size.  By default, the RCE forward does not strip,
     *  an the header is added on egress, for each packet
     */
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        for (port_idx = 0; port_idx < p->p_num_test_ports; port_idx++) {
                       rv = soc_sbx_g2p3_epv2e_get(unit,
                                                   1,
                                                   p->p_port[port_idx],
                                                   &p3epv2e);
            p3epv2e.strip = TRUE;
                       rv = soc_sbx_g2p3_epv2e_set(unit,
                                                   1,
                                                   p->p_port[port_idx],
                                                   &p3epv2e);
        }
        break;
#endif /* def BCM_FE2000_P3_SUPPORT */
    default:
        test_error(unit, "Unsupported microcode on FE2K unit\n");
        return -1;
    }
    }
#endif /* def BCM_FE2000_SUPPORT */

    /* Disable all other ports (can run with active links plugged in) */
    BCM_PBMP_ASSIGN(tmp_pbmp, PBMP_E_ALL(unit));

    /* Remove CPU port, it's not real ports for phy driver */
    BCM_PBMP_REMOVE(tmp_pbmp, PBMP_CMIC(unit));
    for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
        BCM_PBMP_PORT_REMOVE(tmp_pbmp, p->p_port[port_idx]);
    }

    PBMP_ITER(tmp_pbmp, port) {
#ifdef BCM_ROBO_SUPPORT
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
        if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
            if (SOC_PBMP_MEMBER(wan_pbmp, port)) {
                cli_out("Skip storing WAN port(%d) status\n", port);
                continue;
            }
        }
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */

       /* Save the current settings of non-testing ports  */
       if (SOC_IS_ROBO(unit)) {
            rv = bcm_port_info_save(unit, port, &port_info[unit][port]);
            RP_CHK(rv, bcm_port_info_save);
        }
#endif /* BCM_ROBO_SUPPORT */
        rv = bcm_port_enable_set(unit, port, FALSE);
        RP_CHK(rv, bcm_port_enable_set);
    }

#if defined(BCM_FIELD_SUPPORT)
    if (soc_feature(unit, soc_feature_field)) {
        if (SOC_IS_ESW(unit)) {
#if defined(BCM_ESW_SUPPORT) || defined(BCM_FE2000_SUPPORT)
            bcm_field_qset_t        qset;
            bcm_field_group_t       fg;
            bcm_field_entry_t       fent[2];
            int                     stat_id = -1;
            bcm_field_stat_t        stat_type = bcmFieldStatPackets;

            BCM_FIELD_QSET_INIT(qset);

#ifdef BCM_FE2000_SUPPORT
            if (SOC_IS_SBX_FE(unit)) {
                BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
                BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyStageIngressQoS);
            } else
#endif /* BCM_FE2000_SUPPORT */
            {
                BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
            }

            rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, &fg);

            for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                if (BCM_SUCCESS(rv)) {
                    rv = bcm_field_entry_create(unit, fg, &fent[port_idx]);
                }

                if (BCM_SUCCESS(rv)) {
                    p->p_field_entry[port_idx] = fent[port_idx];

#ifdef BCM_FE2000_SUPPORT
                    if (SOC_IS_SBX_FE(unit)) {
                        BCM_PBMP_CLEAR(tmp_pbmp);
                        BCM_PBMP_PORT_ADD(tmp_pbmp, p->p_port[port_idx]);

                        rv = bcm_field_qualify_InPorts(unit, fent[port_idx],
                                                       tmp_pbmp, PBMP_ALL(unit));
                        RP_CHK(rv, bcm_field_qualify_InPorts);
                    } else
#endif /* BCM_FE2000_SUPPORT */
                    {
                        rv = bcm_field_qualify_InPort(unit, fent[port_idx],
                                                      p->p_port[port_idx],
                                                      BCM_FIELD_EXACT_MATCH_MASK);
                    }
                }
            }

            if (BCM_SUCCESS(rv)) {
                for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                    rv = bcm_field_stat_create(unit, fg, 1, &stat_type, &stat_id);
                    RP_CHK(rv, bcm_field_stat_create);

                    rv = bcm_field_entry_stat_attach(unit, fent[port_idx], stat_id);
                    RP_CHK(rv, bcm_field_entry_stat_attach);
                    stat_id = -1;
                }
            }

            for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                if (p->p_use_fp_action) {
                    if (port_idx == 1) {
                        if (BCM_SUCCESS(rv)) {
                            rv = bcm_field_action_add(unit, fent[port_idx], 
                                                      bcmFieldActionCopyToCpu,
                                                      0, 0);
                            RP_CHK(rv, bcm_field_action_add);
                        }
                    }
                } else {
                    if (BCM_SUCCESS(rv)) {
                        rv = bcm_field_action_add(unit, fent[port_idx], 
                                                  bcmFieldActionCopyToCpu,
                                                  0, 0);
                        RP_CHK(rv, bcm_field_action_add);
                    }
                }
                if (BCM_SUCCESS(rv)) {

                    int port = p->p_port[port_idx];
#ifdef BCM_FE2000_SUPPORT
                    if (SOC_IS_SBX_FE(unit)) {
                        port = p->p_port[!port_idx];
                    }
#endif /* BCM_FE2000_SUPPORT */

                    if (p->p_use_fp_action) {
                        if (port_idx == 0) {
                            rv = bcm_field_action_add(unit, fent[port_idx], 
                                                  bcmFieldActionRedirect,
                                                  0, port);
                            RP_CHK(rv, bcm_field_action_add);

                            rv = bcm_field_action_add(unit, fent[port_idx], 
                                                  bcmFieldActionMirrorEgress,
                                                  0, p->p_port[1]);
                            RP_CHK(rv, bcm_field_action_add);

                        }
                    } else {
                        rv = bcm_field_action_add(unit, fent[port_idx], 
                                                  bcmFieldActionRedirect,
                                                  0, port);
                        RP_CHK(rv, bcm_field_action_add);
                    }
                }
            }
#endif /* BCM_ESW_SUPPORT || BCM_FE2000_SUPPORT */
        } else {
#ifdef BCM_ROBO_SUPPORT
            if (SOC_IS_VULCAN(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                bcm_field_qset_t        qset;
                bcm_field_group_t       fg;
                bcm_field_entry_t       fent[2];     
                int                     stat_id = -1;
                bcm_field_stat_t        stat_type = bcmFieldStatPackets;
        
                BCM_FIELD_QSET_INIT(qset);
        
                BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPort);
        
                if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpType);
                }
    
                rv = bcm_field_group_create(unit, qset, BCM_FIELD_GROUP_PRIO_ANY, &fg);
        
                for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                    if (BCM_SUCCESS(rv)) {
                        rv = bcm_field_entry_create(unit, fg, &fent[port_idx]);
                    }
        
                    if (BCM_SUCCESS(rv)) {
                        p->p_field_entry[port_idx] = fent[port_idx];
    
                        rv = bcm_field_qualify_InPort(unit, fent[port_idx],
                                                      p->p_port[port_idx],
                                                      BCM_FIELD_EXACT_MATCH_MASK);
                    }
                }
        
                /* For BCM53115 : the default qualify IP_type is IPv4.
                 * Tx a packet(NonIP type) from CPU : need to add bcmFieldIpTypeNonIP to qualify.
                 */
                if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    if (BCM_SUCCESS(rv)) {
                        rv = bcm_field_qualify_IpType(unit, fent[0], bcmFieldIpTypeNonIp);
                    }
                }
        
                if (BCM_SUCCESS(rv)) {
                    for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {

                        rv = bcm_field_stat_create(unit, fg, 1, &stat_type, &stat_id);
                        RP_CHK(rv, bcm_field_stat_create);

                        rv = bcm_field_entry_stat_attach(unit, fent[port_idx], stat_id);
                        RP_CHK(rv, bcm_field_entry_stat_attach);
                        stat_id = -1;
                    }
                }
        
                /* ROBO chips :
                 *  The bcmFieldActionCopyToCpu and bcmFieldActionRedirect actions are incompatible,
                 *  using bcmFieldActionRedirectPbmp to do the same behavior.
                 *  Now support bcmFieldActionRedirectPbmp : BCM5395, BCM53115.
                 */
                BCM_PBMP_ASSIGN(redirect_pbmp, PBMP_CMIC(unit));
                for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                    BCM_PBMP_PORT_ADD(redirect_pbmp, p->p_port[port_idx]);
                }
                if (BCM_SUCCESS(rv)) {
                    rv = bcm_field_action_add(unit, fent[0], bcmFieldActionRedirectPbmp,
                                                SOC_PBMP_WORD_GET(redirect_pbmp, 0), 0);
                }
                /* For BCM53115 : need to enable bcmFieldActionLoopback 
                 * if forwarding the packet to the receiving.
                 */
                if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                    if (BCM_SUCCESS(rv)) {
                        rv = bcm_field_action_add(unit, fent[0], bcmFieldActionLoopback,
                                                    TRUE, 0);
                    }
                }
            }
#endif
        }
    }

#endif /* BCM_FIELD_SUPPORT */
    if (BCM_FAILURE(rv)) {
        test_error(unit, "Unable to configure filter: %s\n", bcm_errmsg(rv));
        return -1;
    }

    /* Disable learning on that port */
#ifdef BCM_PETRA_SUPPORT        
    if ((SOC_IS_ARAD(unit) && SOC_DPP_PP_ENABLE(unit)) || (!SOC_IS_ARAD(unit))) 
#endif        
    {
        for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
            /* coverity [returned_value] */
            rv = bcm_port_learn_set(unit, p->p_port[port_idx], BCM_PORT_LEARN_FWD);

    #ifdef BCM_FE2000_SUPPORT
            /* SBX controls learning per vlan, not per port */
            if (BCM_FAILURE(rv)) {
                bcm_vlan_control_vlan_t vlan_ctl;
                rv = bcm_vlan_control_vlan_get(unit, 1, &vlan_ctl);
                vlan_ctl.flags |= BCM_VLAN_LEARN_DISABLE;
                rv = bcm_vlan_control_vlan_set(unit, 1, vlan_ctl);
                RP_CHK(rv, bcm_vlan_control_vlan_set);

            }
    #endif /* BCM_FE2000_SUPPORT */
        }
    }
    /* Create Maximum size packet requested */
    BCM_IF_ERROR_RETURN(bcm_pkt_alloc(p->p_txrx_unit , p->p_l_end, 0, &(p->p_pkt)));
    if (p->p_pkt == NULL) {
        test_error(unit, "Failed to allocate Tx packet\n");
        return -1;
    }

#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit)) {
        /* Fill in buffer */
        rp_ptch_header[1] = p->p_port[0];
        SOC_DPP_PKT_PTCH_HDR_SET(p->p_pkt, rp_ptch_header);
        SOC_DPP_PKT_HDR_DMAC_SET_WITH_PTCH(p->p_pkt, rp_mac_dest);
        SOC_DPP_PKT_HDR_SMAC_SET_WITH_PTCH(p->p_pkt, rp_mac_src);
        SOC_DPP_PKT_HDR_TPID_SET_WITH_PTCH(p->p_pkt, ENET_DEFAULT_TPID);
        SOC_DPP_PKT_HDR_VTAG_CONTROL_SET_WITH_PTCH(p->p_pkt, VLAN_CTRL(0, 0, 1));
        /* Fill address starts 2 * MAC + TPID + VTAG */
        fill_addr = BCM_PKT_DMAC(p->p_pkt) + 2 + 6 + 6 + 2 + 2;
        sal_memcpy( fill_addr, mypacket, p->p_l_end -
                   (fill_addr - BCM_PKT_DMAC(p->p_pkt)));
    } 
    else 
#endif
    {
        /* Fill in buffer */
        BCM_PKT_HDR_DMAC_SET(p->p_pkt, rp_mac_dest);
        BCM_PKT_HDR_SMAC_SET(p->p_pkt, rp_mac_src);
        BCM_PKT_HDR_TPID_SET(p->p_pkt, ENET_DEFAULT_TPID);
        BCM_PKT_HDR_VTAG_CONTROL_SET(p->p_pkt, VLAN_CTRL(0, 0, 1));
        /* Fill address starts 2 * MAC + TPID + VTAG */
        fill_addr = BCM_PKT_DMAC(p->p_pkt) + 6 + 6 + 2 + 2;
        sal_memset(fill_addr, 0xff, p->p_l_end -
                   (fill_addr - BCM_PKT_DMAC(p->p_pkt)));
    }

    /* Set the dest port for the packet 
     *  For all devices, the packet will target the first port
     */
    BCM_PKT_PORT_SET(p->p_pkt, p->p_port[0], FALSE, FALSE);
    p->p_pkt->dest_mod = txrx_modid;
    p->p_pkt->flags = BCM_TX_CRC_REGEN;
    p->p_pkt->opcode = BCM_PKT_OPCODE_UC;

    /* Set up the proper configuration */
    rv = rpacket_receiver_activate(p->p_txrx_unit, p);
    if (!BCM_SUCCESS(rv)) {
        test_error(unit, "Could not setup receiver\n");
        return -1;
    }

    return 0;
}

 /*
 * Function:
 *      rpacket_storm_start
 * Purpose:
 *      Start packet storm in switch to serve as packet generator
 * Parameters:
 *      u - unit #.
 *      p - pointer to test parameters
 *      l - length of packet to send
 * Returns:
 *      0 on success, -1 on failure
 * Notes:
 */

int
rpacket_storm_start (int unit, p_t *p, int l)
{
    int rv = 0;
    int tx_pkt_idx;
    int tx_pkt_cnt = 1;
    
#if defined(BCM_FIELD_SUPPORT)
        if (soc_feature(unit, soc_feature_field)) {
            int port_idx = 0;
            if (SOC_IS_ESW(unit)) {

                for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                    rv = bcm_field_entry_install(unit, p->p_field_entry[port_idx]);
                    RP_CHK(rv, bcm_field_entry_install);
                }
            } else {
#ifdef BCM_ROBO_SUPPORT
                if (SOC_IS_VULCAN(unit) ||
                    SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                    SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                    SOC_IS_STARFIGHTER3(unit)) {

                    for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                        rv = bcm_field_entry_install(unit, p->p_field_entry[port_idx]);
                        RP_CHK(rv, bcm_field_entry_install);
                    }
                }
#endif /* BCM_ROBO_SUPPORT */
            }
        }
#endif

        if (BCM_FAILURE(rv)) {
            test_error(unit, "Unable to install filter: %s\n", bcm_errmsg(rv));
            return rv;
        }

    /* determine num packets needed to test this architecture */
#ifdef BCM_FE2000_SUPPORT
    if (SOC_IS_SBX_FE(unit)) {
        tx_pkt_cnt = 5;
    }
#endif /* BCM_FE2000_SUPPORT */

    /* set up test ports */
#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        int port_idx = 0;
        if ((!SOC_IS_VULCAN(unit)) &&
            (!SOC_IS_STARFIGHTER(unit)) && (!SOC_IS_POLAR(unit)) && 
            (!SOC_IS_NORTHSTAR(unit)) && (!SOC_IS_NORTHSTARPLUS(unit)) && 
            (!SOC_IS_STARFIGHTER3(unit))) {

            for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                rv = bcm_port_enable_set(unit, p->p_port[port_idx], 1);
                if (BCM_FAILURE(rv)) {
                    test_error(unit, "Unable to enable port %d: %s\n", 
                        p->p_port[port_idx], bcm_errmsg(rv));
                    return rv;
                }
            }
            /* Sleep a bit to let current set of port be handled */
            sal_sleep(3);
        }
    }
#endif /* BCM_ROBO_SUPPORT */

    /* Send packet(s) to kick off packet storm */

    BCM_PKT_TX_LEN_SET(p->p_pkt, l);
#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARADPLUS(unit)) {
        SOC_DPP_PKT_HDR_TAGGED_LEN_SET_WITH_PTCH(p->p_pkt, l); /* Set length */
        if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
            uint32 reg_val = 0;
            READ_IPT_FORCE_LOCAL_OR_FABRICr(unit,  &reg_val);
            is_force_fabric = soc_reg_field_get(unit, IPT_FORCE_LOCAL_OR_FABRICr, reg_val, FORCE_FABRICf);
            is_force_local = soc_reg_field_get(unit, IPT_FORCE_LOCAL_OR_FABRICr, reg_val, FORCE_LOCALf);
            soc_reg_field_set(unit, IPT_FORCE_LOCAL_OR_FABRICr, &reg_val, FORCE_FABRICf, 0);
            soc_reg_field_set(unit, IPT_FORCE_LOCAL_OR_FABRICr, &reg_val, FORCE_LOCALf, 1);
            WRITE_IPT_FORCE_LOCAL_OR_FABRICr(unit,  reg_val);
        }
    } 
    else 
#endif
    {
        BCM_PKT_HDR_TAGGED_LEN_SET(p->p_pkt, l); /* Set length */
    }

    for (tx_pkt_idx = 0; tx_pkt_idx < tx_pkt_cnt; tx_pkt_idx++) {
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
        if (p->p_use_socket_send) {
            rv = send (p->p_sock, p->p_pkt->_pkt_data.data, l, 0);
            if (rv < 0) { 
                cli_out("Send to socket %d returned len %d errno %d\n", 
                        p->p_sock, rv, errno);
                rv = BCM_E_FAIL;
            }

        } else 
#endif 
        {
            rv = bcm_tx(p->p_txrx_unit, p->p_pkt, NULL);
        }

    }

    return rv;
} /* rpacket_storm_start */



/*
 * Function:
 *      rpacket_storm_stop
 * Purpose:
 *      Stop packet storm; clear out remaining packets
 * Parameters:
 *      u - unit #.
 *      p - pointer to test parameters
 * Returns:
 *      0 on success, -1 on failure
 * Notes:
 */
int
rpacket_storm_stop (int unit, p_t *p)
{
    int rv = 0;
    /*
     * Stop the packets from flowing.
     */
#if defined(BCM_FIELD_SUPPORT)
    if (soc_feature(unit, soc_feature_field)) {
        int port_idx = 0;
        if (SOC_IS_ESW(unit)) {

            for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                rv = bcm_field_entry_remove(unit, p->p_field_entry[port_idx]);
                RP_CHK(rv, bcm_field_entry_remove);
            }

        } else {
#ifdef BCM_ROBO_SUPPORT
            if (SOC_IS_VULCAN(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {

                for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                    rv = bcm_field_entry_remove(unit, p->p_field_entry[port_idx]);
                    RP_CHK(rv, bcm_field_entry_remove);
                }

            }
#endif /* BCM_ROBO_SUPPORT */
        }
    }
#endif
    if (BCM_FAILURE(rv)) {
        test_error(unit, "Unable to remove filter: %s\n",
                   bcm_errmsg(rv));
        return rv;
    }

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        int port_idx = 0;
        if ((!SOC_IS_VULCAN(unit)) &&
            (!SOC_IS_STARFIGHTER(unit)) && (!SOC_IS_POLAR(unit)) && 
            (!SOC_IS_NORTHSTAR(unit)) && (!SOC_IS_NORTHSTARPLUS(unit)) &&
            (!SOC_IS_STARFIGHTER3(unit))) {

            for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
                rv = bcm_port_enable_set(unit, p->p_port[port_idx], 0);
                if (BCM_FAILURE(rv)) {
                    test_error(unit, "Unable to disable port %d: %s\n", 
                        p->p_port[port_idx], bcm_errmsg(rv));
                }
            }

        }
    }
#endif /* BCM_ROBO_SUPPORT */

#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        uint32 reg_val = 0;
        READ_IPT_FORCE_LOCAL_OR_FABRICr(unit,  &reg_val);
        soc_reg_field_set(unit, IPT_FORCE_LOCAL_OR_FABRICr, &reg_val, FORCE_FABRICf, is_force_fabric);
        soc_reg_field_set(unit, IPT_FORCE_LOCAL_OR_FABRICr, &reg_val, FORCE_LOCALf, is_force_local);
        WRITE_IPT_FORCE_LOCAL_OR_FABRICr(unit,  reg_val);
    }
#endif
    
    return rv;
} /* rpacket_storm_stop */


/*
 * Function:
 *      rpacket_test
 * Purpose:
 *      Test packet reception interface.
 * Parameters:
 *      u - unit #.
 *      a - pointer to arguments.
 *      pa - ignored cookie.
 * Returns:
 *      0 on success, -1 on failure
 * Notes:
 *      There remains an issue when stealing packets that the
 *      system will crash when the rate is high enough.  This has
 *      been experimentally determined to be between 4-5K pkts/sec
 *      on Mousse (8240) and 6-7K pkts/sec on BMW (8245).
 */

int
rpacket_test(int unit, args_t *a, void *pa)
{
    p_t *p = (p_t *)pa;
    int l, rv = BCM_E_UNAVAIL;
    int test_rv = 0;
    int use_socket  = 0;
    int ringbuf_size = 0;
    sal_cpu_stats_t  cpu_start;
    sal_cpu_stats_t  cpu_end;

#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__))) 
    long unsigned int knet_pkt=0;
    long unsigned int knet_ic =0;
#endif
    soc_control_t    *soc = SOC_CONTROL(unit);
    int desc_intr_count = 0;
    int chain_intr_count = 0;    
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__))) 
    use_socket = p->p_use_socket;
    ringbuf_size = p->p_ringbuf_size ;
#endif    
    /* coverity[dead_error_condition] */
    cli_out("\n"
            "Rate: %d/%d (%s). %s %s. %s %d PPC. Packets%s freed.\n",
            p->p_pkts_per_sec,
            p->p_burst,
            p->p_hw_rate  ? "HW" : "SW",
            "IF:",
            use_socket ? 
            (ringbuf_size ? "ring" : "socket") :
            "bcm_rx",
            (use_socket) ? "" : ((p->p_intr_cb) ? "Intr CB." : "Task CB."),
            p->p_rx_cfg.pkts_per_chain,
            p->p_free_buffer ? "" : " not"
            );
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
    cli_out("\n"
            "  Packet |          Rate           |  Total               CPU %%               Knet tot  Knet Rate|\n"
            "   Len   |  Pkt / s  |  MB/s       |  packets  |  Time  | Idle /user /kern  |  packets | Pkt/s    |Interrupts\n"
            " --------+-----------+-------------+-----------+--------+-------------------+----------+----------+----------\n");
#else
    cli_out("\n"
            "  Packet |          Rate           |  Total               CPU %%            |\n"
            "   Len   |  Pkt / s  |  MB/s       |  packets  |  Time  | Idle /user /kern  | Desc_Intr   Chain_Intr\n"
            " --------+-----------+-------------+-----------+--------+-------------------+----------+------------\n");
#endif

    if (!use_socket) {
            /* Register the callback handler */
            rv = rpacket_register(p->p_txrx_unit, p);
            if (BCM_FAILURE(rv)) {
                test_error(unit, "Unable to register handler, %s\n",
                            bcm_errmsg(rv));
                test_rv = -1;
                goto done;
            }
    }

    for (l = p->p_l_start; l <= p->p_l_end; l += p->p_l_inc) {

        if (SOC_IS_KATANA(unit) && p->p_hw_rate)  {
            bcm_port_rate_egress_set(unit, CMIC_PORT(unit), 
                                      p->p_pkts_per_sec * l / 125, p->p_burst * l /125);
        }
        rv = rpacket_storm_start (unit, p, l);
        if (BCM_FAILURE(rv)) {
            test_error(unit, "Failed to start packet storm: %s\n", bcm_errmsg(rv));
            test_rv = -1;
            goto done;
        }

        /* Start clean */
        p->p_received = 0;
        p->p_drained = 0;
        p->p_time_us = 0;
        p->p_error_num = 0;

        /* measure rx packets over known interval*/
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
        knet_stats_clear();
#endif
        sal_cpu_stats_get (&cpu_start);
        desc_intr_count = soc->stat.intr_desc;
        chain_intr_count =  soc->stat.intr_chain;
        p->p_count_packets = TRUE;
        sal_sleep(p->p_time);
        p->p_count_packets = FALSE;
        desc_intr_count = soc->stat.intr_desc - desc_intr_count;
        chain_intr_count = soc->stat.intr_chain - chain_intr_count;
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
        knet_stats_get(&knet_ic, &knet_pkt);
#endif
        sal_cpu_stats_get (&cpu_end);

        rv = rpacket_storm_stop (unit, p);
        if (BCM_FAILURE(rv)) {
            test_error(unit, "Failed to stop packet storm: %s\n", bcm_errmsg(rv));
            test_rv = -1;
            goto done;
        }

        /* display test results */
        {
            int td, bps, pps, idle, user,kernel ;
            int idle_diff, user_diff, kernel_diff, total_diff;

            td = (p->p_time_us + 500UL) / 1000UL; /* round to ms */
            if (td == 0) { /* avoid divide by zero */
                td = 1;
            }
            if (p->p_received == 0) {
                pps = bps = 0;
            } else {
                pps = (1000UL * p->p_received) / td;
                bps = (p->p_received * l) / td;
            }

            COMPILER_64_SUB_64(cpu_end.total, cpu_start.total);
            total_diff = (int)(u64_L(cpu_end.total));
            COMPILER_64_SUB_64(cpu_end.idle, cpu_start.idle);
            idle_diff = (int)(u64_L(cpu_end.idle));
            COMPILER_64_SUB_64(cpu_end.user, cpu_start.user);
            user_diff = (int)(u64_L(cpu_end.user));
            COMPILER_64_SUB_64(cpu_end.kernel, cpu_start.kernel);
            kernel_diff = (int)(u64_L(cpu_end.kernel));

            if (total_diff != 0) {
                idle = idle_diff * 10000UL / total_diff;
                user = user_diff * 10000UL / total_diff;
                kernel = kernel_diff * 10000UL / total_diff;
            } else {
                idle = 0;
                user = 0;
                kernel = 0;
            }

#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
            cli_out("  %5u  | %8u  | %5u.%03u   | %8u  | %3u.%03u| %2u.%02u/%2u.%02u/%2u.%02u | %8lu | %8u | %8lu ",
                    #else
                    cli_out("  %5u  | %8u  | %5u.%03u   | %8u  | %3u.%03u| %2u.%02u/%2u.%02u/%2u.%02u | %8u | %8u ",
                            #endif
                            l,
                            (int)pps,
                            (int)(bps / 1000), (int)(bps % 1000),
                            p->p_received,
                            td / 1000, td % 1000,
                            idle / 100, idle % 100,
                            user / 100, user % 100,
                            kernel / 100, kernel % 100
                            #if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
                            ,knet_pkt, (td < p->p_time*1000) ? (int)(knet_pkt/p->p_time): (int)((knet_pkt * 1000) / td),
                            knet_ic);
                    #else
                    ,desc_intr_count, chain_intr_count);
#endif

            if (p->p_error_num > 0) {
                cli_out(" e:%d \n", p->p_error_num);
            } else {
                cli_out("\n");
            }
        }
         /* Sleep a bit to let current backlog of packets drain */
        sal_sleep(2);

    }

    if (!use_socket) {
            /* Unregister the handler, let discard take over */
            rv = rpacket_unregister(p->p_txrx_unit, p);
            if (BCM_FAILURE(rv)) {
                test_error(unit, "Unable to unregister handler, %s\n",
                           bcm_errmsg(rv));
                test_rv = -1;
                goto done;
            }
    }

done:
    if (p->p_dump_rx) {
#ifdef  BROADCOM_DEBUG
        bcm_rx_show(unit);
#endif  /* BROADCOM_DEBUG */
    }
    return test_rv;
}


STATIC int
rpacket_receiver_deactivate(int unit, p_t *p)
{
    return 0;
}



/*ARGSUSED*/
int
rpacket_done(int unit, void *pa)
/*
 * Function:    rpacket_done
 * Purpose:     Clean up after rpacket test.
 * Parameters:  unit - unit #
 *              pa - cookie (as returned by rpacket_init();
 * Returns:     0 - OK
 *              -1 - failed
 */
{
    p_t         *p = p_control[unit];
    int         rv;
    int         port_idx;
#ifdef BCM_ROBO_SUPPORT
    pbmp_t t_pbm;
    int no_que = 0;
    bcm_port_t  port = 0;
#endif /* BCM_ROBO_SUPPORT */
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
    pbmp_t wan_pbmp;
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */


    if (p == NULL) {
        return 0;
    }

    if (p->p_pkt != NULL) {
        bcm_pkt_free(unit, p->p_pkt);
        p->p_pkt = NULL;
    }

    rv = bcm_rx_unregister(p->p_txrx_unit, rpacket_rx_receive, RP_RECEIVER_PRIO);

    rv = rpacket_receiver_deactivate(p->p_txrx_unit, p);
    if (BCM_FAILURE(rv)) {
        test_error(unit, "Unable to deactivate receiver.\n");
        return -1;
    }

    /* Restore port */
    for (port_idx = 0; port_idx < p->p_num_test_ports; port_idx++) {
        rv = bcm_port_info_restore(unit, p->p_port[port_idx], 
                                   &p->p_port_info[port_idx]);
        if (BCM_FAILURE(rv)) {
            test_error(unit, "Unable to restore port %d: %s\n",
                       p->p_port[port_idx], bcm_errmsg(rv));
            return -1;
        }
    }

#ifdef BCM_ROBO_SUPPORT
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
    if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
        wan_pbmp = soc_property_get_pbmp(unit, spn_PBMP_WAN_PORT, 0);
    }   
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */

   /* Restore all other non-testing ports */
   if (SOC_IS_ROBO(unit)) {
        BCM_PBMP_ASSIGN(t_pbm, PBMP_E_ALL(unit));
        for (port_idx=0; port_idx < p->p_num_test_ports; port_idx++) {
            BCM_PBMP_PORT_REMOVE(t_pbm, p->p_port[port_idx]);
        }
#if defined(BCM_NORTHSTAR_SUPPORT)||defined(BCM_NORTHSTARPLUS_SUPPORT)
        if (SOC_IS_NORTHSTAR(unit)||SOC_IS_NORTHSTARPLUS(unit)) {
            BCM_PBMP_REMOVE(t_pbm, wan_pbmp);
        }
#endif /* BCM_NORTHSTAR_SUPPORT||BCM_NORTHSTARPLUS_SUPPORT */
    
        PBMP_ITER(t_pbm, port) {
            rv = bcm_port_info_restore(unit, port, &port_info[unit][port]);
            if (BCM_FAILURE(rv)) {
                test_error(unit, "Unable to restore port %d: %s\n", port, bcm_errmsg(rv));
                return -1;
            }
        }
    }
#endif /* BCM_ROBO_SUPPORT */

#ifdef KEYSTONE
   /* Do dma reinit after rpacket test done */
   if (SOC_IS_ROBO(unit)) {
        soc_eth_dma_reinit(unit);
   }
#endif

    /*
     * Try to remove filter, ignore error since we don't know how far
     * we got in initialization.
     */
#if defined(BCM_FIELD_SUPPORT)
    if (soc_feature(unit, soc_feature_field)) {
        int i = 0;
        if (SOC_IS_ESW(unit)) {
            for (i = 0; i < p->p_num_test_ports; i++) {
                rv = bcm_field_entry_remove(unit, p->p_field_entry[i]);
                RP_CHK(rv, bcm_field_entry_remove);
                rv = bcm_field_entry_destroy(unit, p->p_field_entry[i]);
                RP_CHK(rv, bcm_field_entry_destroy);
            }
        } else {
#ifdef BCM_ROBO_SUPPORT
            if (SOC_IS_VULCAN(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_POLAR(unit) || 
                SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                for (i = 0; i < p->p_num_test_ports; i++) {
                    rv = bcm_field_entry_remove(unit, p->p_field_entry[i]);
                    RP_CHK(rv, bcm_field_entry_remove);
                    rv = bcm_field_entry_destroy(unit, p->p_field_entry[i]);
                    RP_CHK(rv, bcm_field_entry_destroy);
                }
            }
#endif /* BCM_ROBO_SUPPORT */
        }
    }
#endif

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(unit)) {
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            /* IMP_SW_PROTECT */
            /* Restore IMP port egress rate to previous status */
            BCM_PBMP_CLEAR(t_pbm);
            BCM_PBMP_ASSIGN(t_pbm, PBMP_CMIC(unit));
    
            DRV_RATE_SET
                (unit, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                0, 0, imp_rate_default, imp_burst_size_default);
        }
    }
#endif /* BCM_ROBO_SUPPORT */

#ifdef IPROC_CMICD
    /* Do dma reinit after rpacket test done */
    if (SOC_IS_ROBO(unit)) {
        soc_eth_dma_reinit(unit);
        sal_sleep(3);
    }
#endif
    /*
     * Don't free the p_control entry,
     * keep it around to save argument state
     */
     
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
        knetif_clean(unit, p);
#endif
    return 0;
}

char rpacket_usage[] = 
"Receive Packet Test Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
"  Time=<value>         : Specify the test duration for each packet length. (default=2)\n"
"  LengthStart=<value>  : Specify the start pakcet length. (default=64)\n"
"  LengthEnd=<value>    : Specify the end pakcet length. (default=1522)\n"
"  LengthInc=<value>    : Specify the increasing step of pakcet length. (default=64)\n"
"  FreeBuffer=<value>   : Indicate if packet buffer will be freed. (default=FALSE)\n"
"  QLen=<value>         : Specify Max number of packets permitted in cos queue. (default=200)\n"
"  PERCos=<value>       : Indicate if test is running with per-cos settings. (default=FALSE)\n"
"  Rate=<value>         : Specify packets per second for a cos queue. (default=0)\n"
"  Burst=<value>        : Specify packets to be received in a single burst. (default=100)\n"
"  HWrate=<value>       : Indicate if hw rate limiting is used. (default=FALSE)\n"
"  PktsPerChain=<value> : Specify number of packets per DMA chain. (default=16)\n"
"  Chains=<value>       : Specify number of chains (DVs) set up.. (default=4)\n"
"  useINTR=<value>      : Indicate if packets are received in interrupt context. (default=TRUE)\n"
"  DumpRX=<value>       : Indicate if to show packet rx setting information. (default=FALSE)\n"
"  RxMode=<value>       : Indicate if rx mode is set via flags (parse received packets). (default=FALSE)\n"
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
"  SOCKet=<value>       : Indicate if socket is used for packet rx. (default=TRUE)\n"
"  RingBuf=<value>      : Specify ring buffer size for socket. (default=0)\n"
"  SocketTx=<value>     : Indicate if socket is used for packet tx. (default=FALSE)\n"
#endif
#endif
;

/*
 * Function:
 *      rpacket_init
 * Purpose:
 *      Initialize the rpacket test.
 * Parameters:
 *      u - unit #
 *      a - pointer to args
 *      pa - Pointer to cookie
 * Returns:
 *      0 - success, -1 - failed.
 */

int
rpacket_init(int u, args_t *a, void **pa)
{
    p_t                 *p = p_control[u];
    parse_table_t       pt;

#ifdef BCM_ROBO_SUPPORT
    int imp_port;
    pbmp_t t_pbm;
    uint32 limit = 0, burst_size = 0, flags = 0;
    int no_que = 0;
#endif /* BCM_ROBO_SUPPORT */

    if (p == NULL) {
        p = sal_alloc(sizeof(p_t), "rpacket");
        if (p == NULL) {
            test_error(u, "ERROR: cannot allocate memory\n");
            return -1;
        }
        sal_memset(p, 0, sizeof(p_t));
        p_control[u] = p;
    }

    if (!p->p_init) {                   /* Init defaults first time */
        p->p_l_start = 64;
        p->p_l_end = 1522;
        p->p_l_inc = 64;
        p->p_time  = 2;

        p->p_pkts_per_sec = 0;
        p->p_max_q_len = -1;
        p->p_per_cos = FALSE;
        p->p_burst = 100;
        p->p_hw_rate = FALSE;

        p->p_intr_cb = TRUE;
        p->p_dump_rx = FALSE;
        p->p_count_packets = FALSE;
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
        p->p_use_socket = TRUE; 
        p->p_use_socket_send = FALSE; 
#endif
        /* Init the RX cfg here.  Not much exposed */
       if (SOC_IS_ROBO(u)) {
#ifdef BCM_ROBO_SUPPORT
           p->p_rx_cfg.pkt_size = ROBO_RX_PKT_SIZE_DFLT; 
           p->p_rx_cfg.pkts_per_chain = 1;
#endif /* BCM_ROBO_SUPPORT */
       } else {
           p->p_rx_cfg.pkt_size = 8 * 1024;
#if defined(BCM_ESW_SUPPORT) || defined(BCM_PETRA_SUPPORT)
           p->p_rx_cfg.pkts_per_chain = 16; 
#endif /* BCM_ESW_SUPPORT */
#ifdef BCM_FE2000_SUPPORT
           p->p_rx_cfg.pkts_per_chain = 16;
#endif /* BCM_FE2000_SUPPORT */
       }
       p->p_rx_cfg.global_pps = p->p_pkts_per_sec;
       p->p_rx_cfg.max_burst = p->p_burst;
       if (SOC_IS_ROBO(u)) {
           p->p_rx_cfg.chan_cfg[0].chains = 4;
           p->p_rx_cfg.chan_cfg[0].flags = 0;
           p->p_rx_cfg.chan_cfg[0].cos_bmp = 0xff;
       } else {
           p->p_rx_cfg.chan_cfg[1].chains = 4;
           p->p_rx_cfg.chan_cfg[1].flags = 0;
           p->p_rx_cfg.chan_cfg[1].cos_bmp = 0xff;
       }
       p->rx_mode = 0;
       /* Not initializing alloc/free functions */
        p->p_init  = TRUE;
    }

    parse_table_init(u, &pt);
    parse_table_add(&pt, "Time", PQ_DFL|PQ_INT, 0, &p->p_time, 0);
    parse_table_add(&pt, "LengthStart", PQ_DFL|PQ_INT, 0, &p->p_l_start, 0);
    parse_table_add(&pt, "LengthEnd", PQ_DFL|PQ_INT, 0, &p->p_l_end, 0);
    parse_table_add(&pt, "LengthInc", PQ_DFL|PQ_INT, 0, &p->p_l_inc, 0);
    parse_table_add(&pt, "FreeBuffer", PQ_DFL|PQ_BOOL, 0, &p->p_free_buffer,
                    0);

    parse_table_add(&pt, "QLen", PQ_DFL|PQ_INT, 0, &p->p_max_q_len, 0);
    parse_table_add(&pt, "PERCos", PQ_DFL|PQ_INT, 0, &p->p_per_cos, 0);
    parse_table_add(&pt, "Rate", PQ_DFL|PQ_INT, 0, &p->p_pkts_per_sec, 0);
    parse_table_add(&pt, "Burst", PQ_DFL|PQ_INT, 0, &p->p_burst, 0);
    parse_table_add(&pt, "HWrate", PQ_DFL|PQ_INT, 0, &p->p_hw_rate, 0);
    
    parse_table_add(&pt, "PktsPerChain", PQ_DFL|PQ_INT, 0,
                    &p->p_rx_cfg.pkts_per_chain, 0);
#ifdef BCM_ROBO_SUPPORT
    parse_table_add(&pt, "Chains", PQ_DFL|PQ_INT, 0,
                   &p->p_rx_cfg.chan_cfg[0].chains, 0);
#else
    parse_table_add(&pt, "Chains", PQ_DFL|PQ_INT, 0,
                    &p->p_rx_cfg.chan_cfg[1].chains, 0);
#endif
    parse_table_add(&pt, "useINTR", PQ_DFL|PQ_BOOL, 0, &p->p_intr_cb, 0);
    parse_table_add(&pt, "DumpRX", PQ_DFL|PQ_BOOL, 0, &p->p_dump_rx, 0);
    parse_table_add(&pt, "RxMode", PQ_DFL|PQ_INT, 0, &p->rx_mode, 0);
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
    parse_table_add(&pt, "SOCKet", PQ_DFL|PQ_INT, 0, &p->p_use_socket, 0);
    parse_table_add(&pt, "RingBuf", PQ_DFL|PQ_INT, 0, &p->p_ringbuf_size, 0);
    parse_table_add(&pt, "SocketTx", PQ_DFL|PQ_INT, 0, &p->p_use_socket_send, 0);
#endif  
    
    if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
        test_error(u, "%s: Invalid option: %s\n",
                   ARG_CMD(a), ARG_CUR(a) ? ARG_CUR(a) : "*");
        cli_out("%s\n", rpacket_usage);
        parse_arg_eq_done(&pt);
        return -1;
    }
    parse_arg_eq_done(&pt);

    if (p->p_time < 1) {
        test_error(u, "%s: Invalid duration: %d (must be 1 <= time)\n",
                   ARG_CMD(a), p->p_time);
        return -1;
    }
    
    if (p->p_per_cos) {
        p->p_hw_rate = 1;
    }
    
    if (p->p_rx_cfg.pkts_per_chain > RP_MAX_PPC) {
        cli_out("Too many pkts/chain (%d).  Setting to max (%d)\n",
                p->p_rx_cfg.pkts_per_chain, RP_MAX_PPC);
        p->p_rx_cfg.pkts_per_chain = RP_MAX_PPC;
    }
    if (p->rx_mode) {
        p->p_rx_cfg.flags |= BCM_RX_F_PKT_UNPARSED; 
    }
#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__))) 
    if (!p->p_use_socket) {
        p->p_ringbuf_size = 0;
    }  
    p->p_ringbuf = NULL;
    if (p->p_ringbuf_size > MAX_RXRING_FRAMES) {
        p->p_ringbuf_size = MAX_RXRING_FRAMES;
    }
    if (p->p_ringbuf_size > 0) {
        p->p_use_socket = TRUE;  /* ringbuf implies raw socket */
    } else {
        p->p_ringbuf_size = 0;
    } 
#endif    

    if (rpacket_setup(u, p) < 0) {
        (void)rpacket_done(u, p);
        return -1;
    }

    *pa = (void *)p;

#ifdef BCM_ROBO_SUPPORT
    if (SOC_IS_ROBO(u)) {
        /* IMP_SW_PROTECT */
        imp_port = CMIC_PORT(u);
        BCM_PBMP_CLEAR(t_pbm);
        BCM_PBMP_ASSIGN(t_pbm, PBMP_CMIC(u));
        if (SOC_IS_VULCAN(u) || SOC_IS_STARFIGHTER(u) ||
            SOC_IS_POLAR(u) || SOC_IS_NORTHSTAR(u) ||
            SOC_IS_NORTHSTARPLUS(u) || SOC_IS_STARFIGHTER3(u)) {
            /* Record IMP port previous egress rate : restore back while testing done */
            if (DRV_RATE_GET
                (u, imp_port, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                &flags, NULL, &limit, &burst_size) < 0) {
                return -1;
            }
    
            imp_rate_default = limit;
            imp_burst_size_default = burst_size;
    
            SOC_IF_ERROR_RETURN(DRV_RATE_SET
                (u, t_pbm, no_que, DRV_RATE_CONTROL_DIRECTION_EGRESSS, 
                0, 0, imp_rate_pps, IMP_BURST_SIZE));

        }
    }
#endif /* BCM_ROBO_SUPPORT */

#if (defined(INCLUDE_KNET) && defined(LINUX) && (!defined(__KERNEL__)))
        (void)knetif_setup(u, p);
#endif
    return 0;
}

#endif /* BCM_FIELD_SUPPORT || BCM_FE2000_SUPPORT || 
          BCM_ROBO_SUPPORT || BCM_PETRA_SUPPORT */
