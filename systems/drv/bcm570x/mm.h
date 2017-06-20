/*
 * $Id: mm.h,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/******************************************************************************/
/*                                                                            */
/* Broadcom BCM5700 Linux Network Driver, Copyright (c) 2000 Broadcom         */
/* Corporation.                                                               */
/* All rights reserved.                                                       */
/*                                                                            */
/******************************************************************************/

#ifndef MM_H
#define MM_H

#define __PROTOTYPE_5_0		/* Get stdarg prototypes for logMsg */

#include <vxWorks.h>
#include <sys/types.h>
#include <sysLib.h>
#include <intLib.h>
#include <taskLib.h>
#include <semLib.h>

#define __raw_readl readl
#define __raw_writel writel

#if ( _BYTE_ORDER == _BIG_ENDIAN )
#define BIG_ENDIAN_HOST 1
#define readl(addr) (*(volatile unsigned int*)(addr))
#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))
#else
#define readl(addr) \
              (LONGSWAP((*(volatile unsigned int *)(addr))))
#define writel(b,addr) \
              ((*(volatile unsigned int *)(addr)) = (LONGSWAP(b)))
#endif

/* Define memory barrier function here if needed */
#define wmb()
#define membar()

#include "lm.h"
#include "queue.h"
#include "tigon3.h"

#if DBG
#define STATIC
#else
#define STATIC static
#endif

extern int MM_Packet_Desc_Size;

#define MM_PACKET_DESC_SIZE MM_Packet_Desc_Size

DECLARE_QUEUE_TYPE(UM_RX_PACKET_Q, MAX_RX_PACKET_DESC_COUNT+1);

#define MAX_MEM 16

typedef void * dma_addr_t;


#if 0
/* Linux Device Control Structure, show here for comparison */

typedef struct _UM_DEVICE_BLOCK {
	LM_DEVICE_BLOCK lm_dev;
	struct net_device *dev;
	struct pci_dev *pdev;
	struct net_device *next_module;
	char *name;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *pfs_entry;
	char pfs_name[32];
#endif
	void *mem_list[MAX_MEM];
	dma_addr_t dma_list[MAX_MEM];
	int mem_size_list[MAX_MEM];
	int mem_list_num;
	int index;
	int opened;
	int delayed_link_ind; /* Delay link status during initial load */
	int adapter_just_inited; /* the first few seconds after init. */
	int timer_interval;
	int adaptive_expiry;
        int tx_full;
	int tx_queued;
	int line_speed;		/* in Mbps, 0 if link is down */
	UM_RX_PACKET_Q rx_out_of_buf_q;
	int rx_out_of_buf;
	int rx_low_buf_thresh;
	struct timer_list timer;
	int do_global_lock;
	spinlock_t global_lock;
	spinlock_t undi_lock;
	long undi_flags;
	volatile int interrupt;
	int tasklet_pending;
	struct tasklet_struct tasklet;
	struct net_device_stats stats;
#ifdef NICE_SUPPORT
	void (*nice_rx)( struct sk_buff*, void* );
	void* nice_ctx;
#endif /* NICE_SUPPORT */
	int rx_adaptive_coalesce;
	uint rx_last_cnt;
	uint tx_last_cnt;
	uint rx_curr_coalesce_frames;
	uint rx_curr_coalesce_ticks;
} UM_DEVICE_BLOCK, *PUM_DEVICE_BLOCK;


#define MM_ACQUIRE_UNDI_LOCK(_pDevice) \
	if (!(((PUM_DEVICE_BLOCK)(_pDevice))->do_global_lock)) {	\
		long flags;						\
		spin_lock_irqsave(&((PUM_DEVICE_BLOCK)(_pDevice))->undi_lock, flags);	\
		((PUM_DEVICE_BLOCK)(_pDevice))->undi_flags = flags; \
	}

#define MM_RELEASE_UNDI_LOCK(_pDevice) \
	if (!(((PUM_DEVICE_BLOCK)(_pDevice))->do_global_lock)) {	\
		long flags = ((PUM_DEVICE_BLOCK) (_pDevice))->undi_flags; \
		spin_unlock_irqrestore(&((PUM_DEVICE_BLOCK)(_pDevice))->undi_lock, flags); \
	}

#define MM_ACQUIRE_INT_LOCK(_pDevice) \
	while (((PUM_DEVICE_BLOCK) _pDevice)->interrupt)

#define MM_RELEASE_INT_LOCK(_pDevice)

#define MM_UINT_PTR(_ptr)   ((unsigned long) (_ptr))
	    
#endif   /* End of example data structures from Linux */

/* Synch */
typedef SEM_ID mutex_t;
typedef SEM_ID spinlock_t;

/* PCI device */
typedef struct pci_dev_s{
    int bus, device,func;
} pci_dev;

/* VxWorks device control */
typedef struct _UM_DEVICE_BLOCK {
	LM_DEVICE_BLOCK lm_dev;
        struct bcm570x_end_t*  end;
        pci_dev* pdev;
	char *name;
	void *mem_list[MAX_MEM];
	dma_addr_t dma_list[MAX_MEM];
	int mem_size_list[MAX_MEM];
	int mem_list_num;
        int mtu;
        int index;
	int opened;
	int delayed_link_ind; /* Delay link status during initial load */
	int adapter_just_inited; /* the first few seconds after init. */
	int spurious_int;            /* new -- unsupported */
	int timer_interval;  
	int adaptive_expiry;
	int crc_counter_expiry;         /* new -- unsupported */
	int poll_tib_expiry;         /* new -- unsupported */
        int tx_full;
	int tx_queued;
	int line_speed;		/* in Mbps, 0 if link is down */
	UM_RX_PACKET_Q rx_out_of_buf_q;
	int rx_out_of_buf;
	int rx_low_buf_thresh; /* changed to rx_buf_repl_thresh */
	int rx_buf_repl_panic_thresh;
	int rx_buf_align;            /* new -- unsupported */
        FUNCPTR timer;
	int do_global_lock;
        mutex_t global_lock;
        mutex_t undi_lock;
	long undi_flags;
	volatile int interrupt;
	int tasklet_pending;
	int tasklet_busy;	     /* new -- unsupported */
#ifdef NICE_SUPPORT   /* unsupported, this is a linux ioctl */
	void (*nice_rx)(void*, void* );
	void* nice_ctx;
#endif /* NICE_SUPPORT */
	int rx_adaptive_coalesce;
	unsigned int rx_last_cnt;
	unsigned int tx_last_cnt;
	unsigned int rx_curr_coalesce_frames;
	unsigned int rx_curr_coalesce_ticks;
	unsigned int tx_curr_coalesce_frames;  /* new -- unsupported */
#if TIGON3_DEBUG          /* new -- unsupported */
        uint tx_zc_count;
        uint tx_chksum_count;
        uint tx_himem_count;
        uint rx_good_chksum_count;
#endif
        unsigned int rx_bad_chksum_count;   /* new -- unsupported */
        unsigned int rx_misc_errors;        /* new -- unsupported */
} UM_DEVICE_BLOCK, *PUM_DEVICE_BLOCK;


#include "bcm570xEnd.h"

/* Driver options */
#if (CPU == RC32364 || CPU == MIPS32)
#undef USE_ZERO_COPY_TX   /* On send, use fragmented mbuf with no copy*/
#define USE_ZERO_COPY_RX 1  /* On receive, DMA into rx buf shifted by "align" */
#else
#undef USE_ZERO_COPY_TX  /* On send, use fragmented mbuf with no copy*/
#define USE_ZERO_COPY_RX 1  /* On receive, DMA into rx buf shifted by "align" */
#endif
/* Physical/PCI DMA address */
typedef union {
        dma_addr_t dma_map;
} dma_map_t;

/* Packet */
typedef struct
_UM_PACKET {
    LM_PACKET lm_packet;
    void* skbuff;      /* Address of first cluster buffer in MBUF chain */
    M_BLK_ID mb;       /* Packet MBuf, could be fragmented */
} UM_PACKET, *PUM_PACKET;

/* Gigabit Ethernet SENS END object */
typedef struct bcm570x_end_s {
    struct end_object	ge_eo;		/* SENS: END_OBJ */
    unsigned char       ge_mac[6];	/* SENS: Interface Mac address */
    unsigned short	ge_vlan;	/* SENS: Interface VLAN ID */
    int			ge_unit;	/* 570x Unit # */
    int                 irq;            /* 570x IRQ # */
    int                 mtu;            /* Maximum Transmission Unit */
    int                 polled_mode;    /* True when in polled mode */
    int                 pkt_out;        /* True when frame sent */
    int                 pkt_in;         /* True when frame sent */
    int                 rx_dpc;         /* True when Rx DPC is scheduled */
    unsigned int        mbar;           /* PCI Memory Base address */
    PLM_DEVICE_BLOCK    pDevice;        /* 570x softc */
    PUM_DEVICE_BLOCK    pUmDevice;
    int align; 		/* Number of pullup bytes for IP hdr alignment */
    /* Driver stats for this instance */
    int bcm570x_n_tx ;
    int bcm570x_tx_errs ;
    int bcm570x_n_rx ;
    int bcm570x_rx_errs ;
    int bcm570x_allocs ;
    int bcm570x_frees ;
    /* Linked list of devices */
    struct bcm570x_end_s *ge_next;      
} bcm570x_end_t;

#define MM_ACQUIRE_UNDI_LOCK(_pDevice) \
	if (!(((PUM_DEVICE_BLOCK)(_pDevice))->do_global_lock)) {  \
		semTake(((PUM_DEVICE_BLOCK)(_pDevice))->undi_lock,-1);\
	}

#define MM_RELEASE_UNDI_LOCK(_pDevice) \
	if (!(((PUM_DEVICE_BLOCK)(_pDevice))->do_global_lock)) {	\
		semGive(((PUM_DEVICE_BLOCK)(_pDevice))->undi_lock);\
	}

#define MM_ACQUIRE_INT_LOCK(_pDevice) \
	while (((PUM_DEVICE_BLOCK) _pDevice)->interrupt)

#define MM_RELEASE_INT_LOCK(_pDevice)

#define MM_UINT_PTR(_ptr)   ((unsigned long) (_ptr))

/* Macro for setting 64bit address struct */
#define set_64bit_addr(paddr, low, high) \
        (paddr)->Low = low;             \
        (paddr)->High = high;

/* Assume that PCI controller's view of host memory is same as host */

#define MEM_TO_PCI_PHYS(addr) (addr)

extern void MM_SetAddr (LM_PHYSICAL_ADDRESS *paddr, dma_addr_t addr);
extern void MM_SetT3Addr(T3_64BIT_HOST_ADDR *paddr, dma_addr_t addr);
extern void MM_MapTxDma (PLM_DEVICE_BLOCK pDevice,
    struct _LM_PACKET *pPacket, T3_64BIT_HOST_ADDR *paddr,
    LM_UINT32 *len, int frag);
extern void MM_MapRxDma ( PLM_DEVICE_BLOCK pDevice,
    struct _LM_PACKET *pPacket, T3_64BIT_HOST_ADDR *paddr);


/* BSP needs to provide sysUsecDelay and sysSerialPrintString */

extern void sysSerialPrintString (char *s);
#define MM_Wait(usec) sysUsecDelay(usec)

/* Define memory barrier function here if needed */
#define wmb() 

#ifdef BIG_ENDIAN_HOST
#ifdef BIG_ENDIAN_PCI
#define cpu_to_le32(val) val
#else
#define cpu_to_le32(val) LONGSWAP(val)
#endif
#endif

#endif /* MM_H */
