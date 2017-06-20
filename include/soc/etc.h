/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Common [OS-independent] header file for
 * Broadcom BCM44XX 10/100Mbps Ethernet Device Driver
 *
 * $Id: etc.h,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _ETC_SOC_H_
#define _ETC_SOC_H_

#include <soc/ethdma.h>

#define	MAXMULTILIST	32
#define ET_SOC_WATCHDOG_ENABLE 0

#if defined(IPROC_CMICD)
#define NUMTXQ      1
#define NUMRXQ      1
#else
#define NUMTXQ      4
#define NUMRXQ      4
#endif
#define NUM_STAG_TPID       4


#ifndef ch_t
#define	ch_t void
#endif

struct etc_soc_info;	/* forward declaration */

/* each chip type (4413, 47xx) supports a set of chip-type-specific ops */
typedef struct chops {
    bool (*id)(uint vendor, uint device);		/* return true if match */
    void *(*attach)(struct etc_soc_info *etc, void *dev, void *regs);
    void (*detach)(ch_t *ch);			/* free chip private state */
    void (*reset)(ch_t *ch);			/* chip reset */
    void (*init)(ch_t *ch, bool full);	/* chip init */
    bool (*tx)(ch_t *ch, void *p);			/* transmit frame */
    void *(*rx)(ch_t *ch);				/* receive frame */
    void (*rxfill)(ch_t *ch);			/* post dma rx buffers */
    int (*getintrevents)(ch_t *ch);			/* return intr events */
    bool (*errors)(ch_t *ch);			/* handle chip errors */
    void (*intrson)(ch_t *ch);			/* enable chip interrupts */
    void (*intrsoff)(ch_t *ch);			/* disable chip interrupts */
    void (*txreclaim)(ch_t *ch, bool all); /* reclaim transmit resources */
    void (*rxreclaim)(ch_t *ch); /* reclaim receive resources */
    void (*statsupd)(ch_t *ch);			/* update sw stat counters */
    void (*dumpmib)(ch_t *ch, char *buf);   /* get sw mib counters */
    void (*enablepme)(ch_t *ch);			/* enable PME */
    void (*disablepme)(ch_t *ch);			/* disable PME */
    void (*phyreset)(ch_t *ch, uint phyaddr);	/* reset phy */
    uint16 (*phyrd)(ch_t *ch, uint phyaddr, uint reg);	/* read phy register */
    void (*phywr)(ch_t *ch, uint phyaddr, uint reg, uint16 val);	
    /* write phy register */
    void (*dump)(ch_t *ch, char *buf);		/* debugging output */
    void (*longname)(ch_t *ch, char *buf, uint bufsize);	
    /* return descriptive name */
    void (*duplexupd)(ch_t *ch); /* keep mac duplex consistent */
    void (*rxreset)(ch_t *ch, int id);
    void (*rxinit)(ch_t *ch, int id);
    bool (*recover)(ch_t *ch);
    int (*cfprd)(ch_t *ch, void *buf);   /* read CFP entry from hw */
    int (*cfpwr)(ch_t *ch, void *buf);   /* write CFP entry into hw */
    int (*cfpfldrd)(ch_t *ch, void *buf);    /* get CFP field value */
    int (*cfpfldwr)(ch_t *ch, void *buf);    /* set CFP field value */
    int (*cfpudfrd)(ch_t *ch, void *buf);    /* get CFP UDF configuration */
    int (*cfpudfwr)(ch_t *ch, void *buf);    /* set CFP UDF configuration */
    int (*rxrateset)(ch_t *ch, uint channel, uint pps);  /* RX rate configuration */
    int (*rxrateget)(ch_t *ch, uint channel, uint *pps); /* RX rate configuration */
    int (*txrateset)(ch_t *ch, uint channel, uint rate, uint burst); /* TX rate configuration */
    int (*txrateget)(ch_t *ch, uint channel, uint *rate, uint *burst);   /* TX rate configuration */
    int (*flowctrlmodeset)(ch_t *ch, uint mode); /* set flow control mode */
    int (*flowctrlmodeget)(ch_t *ch, uint *mode); /* get flow control mode */
    int (*flowctrlautoset)(ch_t *ch, uint on_thresh, uint off_thresh); /* set on/off threshold for auto mode */
    int (*flowctrlautoget)(ch_t *ch, uint *on_thresh, uint *off_thresh); /* get on/off threshold for auto mode */
    int (*flowctrlcpuset)(ch_t *ch, uint pause_on); /* enable/disable pause frame for cpu mode */
    int (*flowctrlcpuget)(ch_t *ch, uint *pause_on); /* get the enable/disable pause frame for cpu mode */
    int (*flowctrlrxchanset)(ch_t *ch, uint chan, uint on_thresh, uint off_thresh); /* set rx channel thresholds */
    int (*flowctrlrxchanget)(ch_t *ch, uint chan, uint *on_thresh, uint *off_thresh); /* get rx channel thresholds */
    int (*tpidset)(ch_t *ch, uint index,uint tpid);  /* Set the S-TAG TPID value */
    int (*tpidget)(ch_t *ch, uint index,uint *tpid); /* Get the S-TAG TPID value */
    int (*pvtagset)(ch_t *ch, uint private_tag); /* Set the private tag value */
    int (*pvtagget)(ch_t *ch, uint *private_tag);    /* Get the private tag value */
    int (*rxsephdrset)(ch_t *ch, uint enable);	/* Enable/Disable the RX separate header */
    int (*rxsepgetget)(ch_t *ch, uint *enable);	/* Get the value of RX separate header feature */
    int (*txqosmodeset)(ch_t *ch, uint mode); /* set tx qos mode */
    int (*txqosmodeget)(ch_t *ch, uint *mode); /* get tx qos mode */
    int (*txqosweightset)(ch_t *ch, uint queue, uint weight); /* set tx queue weight */
    int (*txqosweightget)(ch_t *ch, uint queue, uint *weight); /* get tx queue weight */
} chops_t;

/*
 * "Common" os-independent software state structure.
 */
typedef struct etc_soc_info {
	void		*et;		/* pointer to os-specific private state */
	uint		unit;		/* device instance number */
	bool		up;		/* interface up and running */
	void        *mib;       /* pointer to s/w maintained mib counters */
	bool		promisc;	/* promiscuous destination address */
	bool        qos;        /* QoS priority determination on rx */
	bool		loopbk;		/* loopback override mode */
    

	int		forcespeed;	
	/* disable autonegotiation and force speed/duplex */
	uint		advertise; 
	uint		advertise2;
	/* control speed/duplex advertised capability bits */
	bool		needautoneg;	/* request restart autonegotiation */
	int		speed;		/* current speed: 10, 100 */
	int		duplex;		/* current duplex: 0=half, 1=full */

	bool		piomode;	/* enable programmed io (!dma) */
	void		*pioactive;	
	/* points to pio packet being transmitted */
	volatile uint   *txavail[NUMTXQ];   /* dma: # tx descriptors available */

	uint16		vendorid;	/* pci function vendor id */
	uint16		deviceid;	/* pci function device id */
	uint		chip;		/* chip number */
	uint		chiprev;	/* chip revision */
	uint		coreid;     /* core id */
	uint		corerev;    /* core revision */
	uint		hwrxoff;    /* length of rx header */    

	bool		nicmode;	/* is this core using its own pci i/f */

	struct chops	*chops;		/* pointer to chip-specific opsvec */
	void		*ch;		/* pointer to chip-specific state */

	uint        txq_state;  /* tx queues state bits */
	uint		coreunit;	/* sb chips: chip enet instance # */    
	uint		phyaddr;	/* sb chips: mdio 5-bit phy address */
	uint		mdcport;	
	/* sb chips: which mii to use (enet core #) to access our phy */
	bool        ext_config; /* enable configuration with external pins */

	struct ether_addr cur_etheraddr; /* our local ethernet address */
	struct ether_addr perm_etheraddr; 
	/* original sprom local ethernet address */

	struct ether_addr multicast[MAXMULTILIST];
	uint		nmulticast;
	bool		allmulti;	/* enable all multicasts */

	bool		linkstate;	/* link integrity state */
	bool		pm_modechange;	/* true if mode change is to due pm */

	uint32		now;		/* elapsed seconds */
    uint32      boardflags; /* board flags */
    uint32  pkt_mem; /* The memory type of packet buffer */
    uint32  pkthdr_mem; /* The memory type of packet header buffer */
    uint32  en_rxsephdr[NUMRXQ];

    /* rx rate */
    uint32      rx_pps[NUMRXQ];
    uint32      tx_rate[NUMTXQ];
    uint32      tx_burst[NUMTXQ];

    /* tx qos mode */
    uint32      txqos_mode;
    uint32      txqos_weight[NUMTXQ];  

    /* flow control */
    uint32      flowcntl_mode;
    uint32      flowcntl_auto_on_thresh;
    uint32      flowcntl_auto_off_thresh;
    uint32      flowcntl_cpu_pause_on;
    uint32      flowcntl_rx_on_thresh[NUMRXQ];
    uint32      flowcntl_rx_off_thresh[NUMRXQ];

    /* TPID */
    uint32      tpid[NUM_STAG_TPID];

    /* Private Tag */
    uint32      ptag;    

	/* chip-maintained plus a few sw-maintained stat counters */
	uint32		txframe;	/* transmitted frames */
	uint32		txbyte;		/* transmitted bytes */
	uint32		rxframe;	/* received frames */
	uint32		rxbyte;		/* received bytes */
	uint32		txerror;	/* total tx errors */
	uint32		txnobuf;	/* tx out-of-buffer errors */
	uint32		rxerror;	/* total rx errors */
	uint32		rxnobuf;	/* rx out-of-buffer errors */
	uint32		reset;		/* reset count */
	uint32		dmade;		/* pci descriptor errors */
	uint32		dmada;		/* pci data errors */
	uint32		dmape;		/* descriptor protocol error */
	uint32		rxdmauflo;	/* receive descriptor underflow */
	uint32		rxoflo;		/* receive fifo overflow */
	uint32		txuflo;		/* transmit fifo underflow */
	uint32		rxbadlen;	/* 802.3 len field != read length */
} etc_soc_info_t;

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

/* flow control */
#define FLOW_CTRL_MODE_DISABLE     0x0 /* disable mode */
#define FLOW_CTRL_MODE_AUTO     0x1 /* auto mode */
#define FLOW_CTRL_MODE_CPU     0x2 /* cpu mode */
#define FLOW_CTRL_MODE_MIX     0x3 /* auto + cpu mode */

#define FLWO_CTRL_AUTO_MODE_DEFAULT_ON_THRESH       0x6a4
#define FLWO_CTRL_AUTO_MODE_DEFAULT_OFF_THRESH       0x578

/* tx qos mode */
#define TXQOS_MODE_1STRICT_3WRR     0x0
#define TXQOS_MODE_2STRICT_2WRR     0x1
#define TXQOS_MODE_3STRICT_1WRR     0x2
#define TXQOS_MODE_ALL_STRICT     0x3
#define TXQOS_MODE_ALL_WRR     0x4

#define TXQOS_DEFAULT_WEIGHT        0xa


/* S-TAG TPID */
#define STAG_TPID_DEFAULT       0x88a8

extern uint32 etc_up2tc(uint32 up);
extern uint32 etc_priq(uint32 txq_state);


typedef struct et_soc_info {
    etc_soc_info_t	*etc;	/* pointer to common os-independent data */
    int         dev;		/* backpoint to device */
    sal_mutex_t soc_eth_dma_lock;	/* per-device perimeter lock */
#define ET_SOC_DMA_LOCK(s) sal_mutex_take(s->soc_eth_dma_lock, sal_mutex_FOREVER)
#define ET_SOC_DMA_UNLOCK(s)	sal_mutex_give(s->soc_eth_dma_lock)
    sal_mutex_t tx_dma_lock;	/* lock for tx chain */
#define ET_TX_DMA_LOCK(s) sal_mutex_take(s->tx_dma_lock, sal_mutex_FOREVER)
#define ET_TX_DMA_UNLOCK(s)	sal_mutex_give(s->tx_dma_lock)
    sal_mutex_t rx_dma_lock;	/* lock for rx chain */
#define ET_RX_DMA_LOCK(s) sal_mutex_take(s->rx_dma_lock, sal_mutex_FOREVER)
#define ET_RX_DMA_UNLOCK(s)	sal_mutex_give(s->rx_dma_lock)
    eth_dv_t	*txq_head;	/* head of send queue */
    eth_dv_t	*txq_tail;	/* tail of send queue */
    int		txq_cnt;	/* number of pending txq */

    eth_dv_t    *txq_done_head;
    eth_dv_t    *txq_done_tail;
    int     txq_done_cnt;

    eth_dv_t	*rxq_head[NUMRXQ];	/* head of receive queue */
    eth_dv_t	*rxq_tail[NUMRXQ];	/* tail of receive queue */
    int		rxq_cnt[NUMRXQ];	/* number of pending rxq */

    eth_dv_t    *rxq_done_head[NUMRXQ];
    eth_dv_t    *rxq_done_tail[NUMRXQ];

#if ET_SOC_WATCHDOG_ENABLE
    sal_usecs_t	timer;		/* one second watchdog timer */
#define SOC_WD_TIMER		10000000
#endif
    bool        et_soc_init;
    bool        et_soc_intr_pend;
    bool        et_soc_rxmon;
} et_soc_info_t;

/* interrupt event bitvec */
#define	INTR_TX		0x1
#define	INTR_RX		0x2
#define	INTR_ERROR	0x4
#define	INTR_TO		0x8
#define	INTR_NEW	0x10

/* special phyaddr values */
#define	PHY_NOMDC	30		/* phy is really another mac w/no mdc */
#define	PHY_NONE	31		/* no phy present */

/* phy address */
#define	MAXEPHY		32			/* mdio phy addresses are 5bit quantities */
#define	EPHY_MASK	0x1f			/* phy mask */
#define	EPHY_NONE	31			/* nvram: no phy present at all */
#define	EPHY_NOREG	30			/* nvram: no local phy regs */

#define	MAXPHYREG	32			/* max 32 registers per phy */

/* forcespeed values */
#define	ET_AUTO		-1
#define	ET_10HALF	0
#define	ET_10FULL	1
#define	ET_100HALF	2
#define	ET_100FULL	3
#define ET_1000HALF 4
#define ET_1000FULL 5
#define ET_2000FULL 6

/* a few misc mii/mdio definitions */
#define	CTL_RESET	(1 << 15)	/* reset */
#define	CTL_LOOP	(1 << 14)	/* loopback */
#define	CTL_SPEED	(1 << 13)	/* speed selection 0=10, 1=100 */
#define	CTL_ANENAB	(1 << 12)	/* autonegotiation enable */
#define	CTL_RESTART	(1 << 9)	/* restart autonegotiation */
#define	CTL_DUPLEX	(1 << 8)	/* duplex mode 0=half, 1=full */
#define	CTL_SPEED_MSB	(1 << 6)		/* speed selection msb */

#define	CTL_SPEED_10	((0 << 6) | (0 << 13))	/* speed selection CTL.6=0, CTL.13=0 */
#define	CTL_SPEED_100	((0 << 6) | (1 << 13))	/* speed selection CTL.6=0, CTL.13=1 */
#define	CTL_SPEED_1000	((1 << 6) | (0 << 13))	/* speed selection CTL.6=1, CTL.13=0 */


#define	ADV_10FULL (1 << 6) /* autonegotiate advertise 10full capability */
#define	ADV_10HALF (1 << 5) /* autonegotiate advertise 10half capability */
#define	ADV_100FULL (1 << 8) /* autonegotiate advertise 100full capability */
#define	ADV_100HALF (1 << 7) /* autonegotiate advertise 100half capability */

/* Link partner ability register. */
#define LPA_SLCT                0x001f  /* Same as advertise selector  */
#define LPA_10HALF              0x0020  /* Can do 10mbps half-duplex   */
#define LPA_10FULL              0x0040  /* Can do 10mbps full-duplex   */
#define LPA_100HALF             0x0080  /* Can do 100mbps half-duplex  */
#define LPA_100FULL             0x0100  /* Can do 100mbps full-duplex  */
#define LPA_100BASE4            0x0200  /* Can do 100mbps 4k packets   */
#define LPA_RESV                0x1c00  /* Unused...                   */
#define LPA_RFAULT              0x2000  /* Link partner faulted        */
#define LPA_LPACK               0x4000  /* Link partner acked us       */
#define LPA_NPAGE               0x8000  /* Next page bit               */

#define LPA_DUPLEX		(LPA_10FULL | LPA_100FULL)
#define LPA_100			(LPA_100FULL | LPA_100HALF | LPA_100BASE4)

/* 1000BASE-T control register */
#define	ADV_1000HALF	0x0100			/* advertise 1000BASE-T half duplex */
#define	ADV_1000FULL	0x0200			/* advertise 1000BASE-T full duplex */

/* 1000BASE-T status register */
#define	LPA_1000HALF	0x0400			/* link partner 1000BASE-T half duplex */
#define	LPA_1000FULL	0x0800			/* link partner 1000BASE-T full duplex */

/* 1000BASE-T extended status register */
#define	EST_1000THALF	0x1000			/* 1000BASE-T half duplex capable */
#define	EST_1000TFULL	0x2000			/* 1000BASE-T full duplex capable */
#define	EST_1000XHALF	0x4000			/* 1000BASE-X half duplex capable */
#define	EST_1000XFULL	0x8000			/* 1000BASE-X full duplex capable */

#define	STAT_REMFAULT	(1 << 4)	/* remote fault */
#define	STAT_LINK	(1 << 2)	/* link status */
#define	STAT_JAB	(1 << 1)	/* jabber detected */
#define	AUX_FORCED	(1 << 2)	/* forced 10/100 */
#define	AUX_SPEED	(1 << 1)	/* speed 0=10mbps 1=100mbps */
#define	AUX_DUPLEX	(1 << 0)	/* duplex 0=half 1=full */

/* common ioctl definitions */
#define	ETCUP		0
#define	ETCDOWN		1
#define ETCLOOP		2
#define ETCDUMP		3
#define ETCSETMSGLEVEL	4
#define	ETCPROMISC	5
#define	ETCRESV1	6
#define	ETCSPEED	7
#define ETCPHYRD	9
#define ETCPHYWR	10
#define	ETCQOS		11
#define ETCPHYRD2	12
#define ETCPHYWR2	13
#define ETCROBORD	14
#define ETCROBOWR	15
#define ETCCFPRD	16
#define ETCCFPWR      17
#define ETCCFPFIELDRD	18
#define ETCCFPFIELDWR      19
#define ETCCFPUDFRD	 20
#define ETCCFPUDFWR      21
#define ETCPKTMEMGET    22
#define ETCPKTMEMSET    23
#define ETCPKTHDRMEMGET 24
#define ETCPKTHDRMEMSET 25
#define ETCRXRATE 26
#define ETCTXRATE 27
#define ETCFLOWCTRLMODE 28
#define ETCFLOWCTRLAUTOSET 29
#define ETCFLOWCTRLCPUSET 30
#define ETCFLOWCTRLRXCHANSET 31
#define ETCTPID 32
#define ETCPVTAG 33
#define ETCRXSEPHDR    34
#define ETCTXQOSMODE    35
#define ETCTXQOSWEIGHTSET    36

/* rx header flags bits */
#define RXH_FLAGS(etc, rxh) (((etc)->deviceid == BCM53000_GMAC_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & (GRXF_CRC | GRXF_OVF | GRXF_OVERSIZE)) : \
	(ltoh16(((bcmenetrxh_t *)(rxh))->flags) & (RXF_NO | RXF_RXER | RXF_CRC | RXF_OV)))

#define RXH_OVERSIZE(etc, rxh) (((etc)->deviceid == BCM53000_GMAC_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & GRXF_OVERSIZE) : FALSE)

#define RXH_CRC(etc, rxh) (((etc)->deviceid == BCM53000_GMAC_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & GRXF_CRC) : \
	(ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_CRC))

#define RXH_OVF(etc, rxh) (((etc)->deviceid == BCM53000_GMAC_ID) ? \
	(ltoh16(((bcmgmacrxh_t *)(rxh))->flags) & GRXF_OVF) : \
	(ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_OV))

#define RXH_RXER(etc, rxh) (((etc)->deviceid == BCM53000_GMAC_ID) ? \
	FALSE : (ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_RXER))

#define RXH_NO(etc, rxh) (((etc)->deviceid == BCM53000_GMAC_ID) ? \
	FALSE : (ltoh16(((bcmenetrxh_t *)(rxh))->flags) & RXF_NO))


/* exported prototypes */
extern chops_t *etc_soc_chipmatch(uint vendor, uint device);
extern void *etc_soc_attach(void *et, uint vendor, uint device, uint unit, 
    void *dev, void *regsva);
extern void etc_soc_detach(etc_soc_info_t *etc);
extern void etc_soc_reset(etc_soc_info_t *etc);
extern void etc_soc_debug(etc_soc_info_t *etc);
extern void etc_soc_init(etc_soc_info_t *etc, bool full);
extern void etc_soc_up(etc_soc_info_t *etc);
extern uint etc_soc_down(etc_soc_info_t *etc, int reset);
extern int etc_soc_ioctl(etc_soc_info_t *etc, int cmd, void *arg);
extern void etc_soc_promisc(etc_soc_info_t *etc, uint on);
extern void etc_soc_qos(etc_soc_info_t *etc, uint on);
extern void etc_soc_dump(etc_soc_info_t *etc, uchar *buf, int size);
extern void etc_soc_watchdog(etc_soc_info_t *etc);
extern uint etc_soc_totlen(etc_soc_info_t *etc, void *p);
extern uint32 etc_soc_up2tc(uint32 up);
extern uint32 etc_soc_priq(uint32 txq_state);

#endif	/* _ETC_SOC_H_ */
