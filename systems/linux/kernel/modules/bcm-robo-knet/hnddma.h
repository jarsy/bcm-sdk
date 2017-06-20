/*
 * Generic Broadcom Home Networking Division (HND) DMA engine SW interface
 *
 * $Id: hnddma.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$  
 */

#ifndef	_knet_hnddma_h_
#define	_knet_hnddma_h_


typedef const struct hnddma_pub hnddma_t;

/* dma function type */
typedef void (*di_detach_t) (hnddma_t *dmah);
typedef bool(*di_txreset_t) (hnddma_t *dmah);
typedef bool(*di_rxreset_t) (hnddma_t *dmah);
typedef bool(*di_rxidle_t) (hnddma_t *dmah);
typedef void (*di_txinit_t) (hnddma_t *dmah);
typedef bool(*di_txenabled_t) (hnddma_t *dmah);
typedef void (*di_rxinit_t) (hnddma_t *dmah);
typedef void (*di_txsuspend_t) (hnddma_t *dmah);
typedef void (*di_txresume_t) (hnddma_t *dmah);
typedef bool(*di_txsuspended_t) (hnddma_t *dmah);
typedef bool(*di_txsuspendedidle_t) (hnddma_t *dmah);
typedef int (*di_txfast_t) (hnddma_t *dmah, void *p, bool commit);
typedef void (*di_fifoloopbackenable_t) (hnddma_t *dmah, bool on);
typedef bool(*di_txstopped_t) (hnddma_t *dmah);
typedef bool(*di_rxstopped_t) (hnddma_t *dmah);
typedef bool(*di_rxenable_t) (hnddma_t *dmah);
typedef bool(*di_rxenabled_t) (hnddma_t *dmah);
typedef void *(*di_rx_t) (hnddma_t *dmah);
typedef void (*di_rxfill_t) (hnddma_t *dmah);
typedef void *(*di_txreclaim_t) (hnddma_t *dmah, bool forceall);
typedef void (*di_rxreclaim_t) (hnddma_t *dmah);
typedef uint *(*di_getvar_t) (hnddma_t *dmah, const char *name);
typedef void *(*di_getnexttxp_t) (hnddma_t *dmah, bool forceall);
typedef void *(*di_getnextrxp_t) (hnddma_t *dmah, bool forceall);
typedef void *(*di_peeknexttxp_t) (hnddma_t *dmah);
typedef void (*di_txblock_t) (hnddma_t *dmah);
typedef void (*di_txunblock_t) (hnddma_t *dmah);
typedef uint(*di_txactive_t) (hnddma_t *dmah);
typedef void (*di_txrotate_t) (hnddma_t *dmah);
typedef void (*di_counterreset_t) (hnddma_t *dmah);
typedef char *(*di_dump_t) (hnddma_t *dmah, char *b,
			    bool dumpring);
typedef char *(*di_dumptx_t) (hnddma_t *dmah, char *b,
			      bool dumpring);
typedef char *(*di_dumprx_t) (hnddma_t *dmah, char *b,
			      bool dumpring);
typedef uint(*di_rxactive_t) (hnddma_t *dmah);
typedef uint(*di_txpending_t) (hnddma_t *dmah);
typedef void (*di_rxsephdrctrl_t) (hnddma_t *dmah, bool enable,
        uint memtype, uint size);

/* dma opsvec */
typedef struct di_fcn_s {
    di_detach_t             detach;
    di_txinit_t             txinit;
    di_txreset_t            txreset;
    di_txenabled_t          txenabled;
    di_txsuspend_t          txsuspend;
    di_txresume_t           txresume;
    di_txsuspended_t        txsuspended;
    di_txsuspendedidle_t    txsuspendedidle;
    di_txfast_t             txfast;
    di_txstopped_t          txstopped;
    di_txreclaim_t          txreclaim;
    di_getnexttxp_t         getnexttxp;
    di_peeknexttxp_t        peeknexttxp;
    di_txblock_t            txblock;
    di_txunblock_t          txunblock;
    di_txactive_t           txactive;
    di_txrotate_t           txrotate;

    di_rxinit_t             rxinit;
    di_rxreset_t            rxreset;
    di_rxidle_t             rxidle;
    di_rxstopped_t          rxstopped;
    di_rxenable_t           rxenable;
    di_rxenabled_t          rxenabled;
    di_rx_t                 rx;
    di_rxfill_t             rxfill;
    di_rxreclaim_t          rxreclaim;
    di_getnextrxp_t         getnextrxp;

    di_fifoloopbackenable_t fifoloopbackenable;
    di_getvar_t             d_getvar;
    di_counterreset_t       counterreset;
    di_dump_t               dump;
    di_dumptx_t             dumptx;
    di_dumprx_t             dumprx;
    di_rxactive_t           rxactive;
    di_txpending_t          txpending;
    di_rxsephdrctrl_t       rxsephdrctrl;
    uint                    endnum;
} di_fcn_t;

/*
 * Exported data structure (read-only)
 */
/* export structure */
struct hnddma_pub {
	di_fcn_t	di_fn;		/* DMA function pointers */
	uint		txavail;	/* # free tx descriptors */

	/* rx error counters */
	uint		rxgiants;	/* rx giant frames */
	uint		rxnobuf;	/* rx out of dma descriptors */
	/* tx error counters */
	uint		txnobuf;	/* tx out of dma descriptors */
};

#define DMA_DEBUG_MSG_ERR   1
#define DMA_DEBUG_MSG_TRACE 2

extern hnddma_t *dma_attach(void *osh, char *name, uint dma_id,
			    void *dmaregstx, void *dmaregsrx, uint ntxd,
			    uint nrxd, uint rxbufsize, uint nrxpost,
			    uint rxoffset, uint *msg_level);
#define dma_detach(di)			((di)->di_fn.detach(di))
#define dma_txreset(di)			((di)->di_fn.txreset(di))
#define dma_rxreset(di)			((di)->di_fn.rxreset(di))
#define dma_rxidle(di)			((di)->di_fn.rxidle(di))
#define dma_txinit(di)                  ((di)->di_fn.txinit(di))
#define dma_txenabled(di)               ((di)->di_fn.txenabled(di))
#define dma_rxinit(di)                  ((di)->di_fn.rxinit(di))
#define dma_txsuspend(di)               ((di)->di_fn.txsuspend(di))
#define dma_txresume(di)                ((di)->di_fn.txresume(di))
#define dma_txsuspended(di)             ((di)->di_fn.txsuspended(di))
#define dma_txsuspendedidle(di)         ((di)->di_fn.txsuspendedidle(di))
#define dma_txfast(di, p, commit)	((di)->di_fn.txfast(di, p, commit))
#define dma_fifoloopbackenable(di, on)      ((di)->di_fn.fifoloopbackenable(di, on))
#define dma_txstopped(di)               ((di)->di_fn.txstopped(di))
#define dma_rxstopped(di)               ((di)->di_fn.rxstopped(di))
#define dma_rxenable(di)                ((di)->di_fn.rxenable(di))
#define dma_rxenabled(di)               ((di)->di_fn.rxenabled(di))
#define dma_rx(di)                      ((di)->di_fn.rx(di))
#define dma_rxfill(di)                  ((di)->di_fn.rxfill(di))
#define dma_txreclaim(di, forceall)	((di)->di_fn.txreclaim(di, forceall))
#define dma_rxreclaim(di)               ((di)->di_fn.rxreclaim(di))
#define dma_getvar(di, name)		((di)->di_fn.d_getvar(di, name))
#define dma_getnexttxp(di, forceall)    ((di)->di_fn.getnexttxp(di, forceall))
#define dma_getnextrxp(di, forceall)    ((di)->di_fn.getnextrxp(di, forceall))
#define dma_peeknexttxp(di)             ((di)->di_fn.peeknexttxp(di))
#define dma_txblock(di)                 ((di)->di_fn.txblock(di))
#define dma_txunblock(di)               ((di)->di_fn.txunblock(di))
#define dma_txactive(di)                ((di)->di_fn.txactive(di))
#define dma_rxactive(di)                ((di)->di_fn.rxactive(di))
#define dma_txrotate(di)                ((di)->di_fn.txrotate(di))
#define dma_counterreset(di)            ((di)->di_fn.counterreset(di))
#define dma_txpending(di)		((di)->di_fn.txpending(di))
#define dma_rxsephdrctrl(di, enable, memtype, size)            ((di)->di_fn.rxsephdrctrl(di, enable, memtype, size))

#define dma_dump(di, buf, dumpring)	((di)->di_fn.dump(di, buf, dumpring))
#define dma_dumptx(di, buf, dumpring)	((di)->di_fn.dumptx(di, buf, dumpring))
#define dma_dumprx(di, buf, dumpring)	((di)->di_fn.dumprx(di, buf, dumpring))



/* dma registers per channel(xmt or rcv) */
typedef volatile struct {
	unsigned int	control;		/* enable, et al */
	unsigned int	ptr;			/* last descriptor posted to chip */
	unsigned int	addrlow;		/* descriptor ring base address low 32-bits (8K aligned) */
	unsigned int	addrhigh;		/* descriptor ring base address bits 63:32 (8K aligned) */
	unsigned int	status0;		/* current descriptor, xmt state */
	unsigned int	status1;		/* active descriptor, xmt error */
} dma64regs_t;


/*
 * DMA Descriptor
 * Descriptors are only read by the hardware, never written back.
 */
typedef volatile struct {
	unsigned int	ctrl1;		/* misc control bits & bufcount */
	unsigned int	ctrl2;		/* buffer count and address extension */
	unsigned int	addrlow;	/* memory address of the date buffer, bits 31:0 */
	unsigned int	addrhigh;	/* memory address of the date buffer, bits 63:32 */
} dma64dd_t;


/* bcopy, bcmp, and bzero */
#define	bcopy(src, dst, len)	memcpy(dst, src, len)
#define	bcmp(b1, b2, len)	memcmp(b1, b2, len)
#define	bzero(b, len)		memset(b, '\0', len)

#define	MALLOC(osh, size)		kmalloc(size, GFP_ATOMIC)
#define	MFREE(osh, addr, size)	kfree((void*)(addr))


#define ASSERT(exp)
#define ISALIGNED(a, x)		(((uint)(a) & ((x)-1)) == 0)
#define ISPOWEROF2(x)		((((x)-1)&(x))==0)

/*
 * Each descriptor ring must be 8kB aligned, and fit within a contiguous 8kB physical addresss.
 */
#define D64MAXRINGSZ	8192
#define D64RINGALIGN	8192
#define D64MAXDD	(D64MAXRINGSZ / sizeof (dma64dd_t))


/* transmit channel control */
#define D64_XC_XE		0x00000001	/* transmit enable */
#define D64_XC_SE		0x00000002	/* transmit suspend request */
#define D64_XC_LE		0x00000004	/* loopback enable */
#define D64_XC_FL		0x00000010	/* flush request */
#define D64_XC_PD		0x00000800	/* parity check disable */
#define D64_XC_AE		0x00030000	/* address extension bits */
#define D64_XC_AE_SHIFT		16

/* transmit descriptor table pointer */
#define D64_XP_LD_MASK		0x00000fff	/* last valid descriptor */

/* transmit channel status */
#define D64_XS0_CD_MASK		0x00001fff	/* current descriptor pointer */
#define D64_XS0_XS_MASK		0xf0000000     	/* transmit state */
#define D64_XS0_XS_SHIFT		28
#define D64_XS0_XS_DISABLED	0x00000000	/* disabled */
#define D64_XS0_XS_ACTIVE	0x10000000	/* active */
#define D64_XS0_XS_IDLE		0x20000000	/* idle wait */
#define D64_XS0_XS_STOPPED	0x30000000	/* stopped */
#define D64_XS0_XS_SUSP		0x40000000	/* suspend pending */

#define D64_XS1_AD_MASK		0x0001ffff	/* active descriptor */
#define D64_XS1_XE_MASK		0xf0000000     	/* transmit errors */
#define D64_XS1_XE_SHIFT		28
#define D64_XS1_XE_NOERR	0x00000000	/* no error */
#define D64_XS1_XE_DPE		0x10000000	/* descriptor protocol error */
#define D64_XS1_XE_DFU		0x20000000	/* data fifo underrun */
#define D64_XS1_XE_DTE		0x30000000	/* data transfer error */
#define D64_XS1_XE_DESRE	0x40000000	/* descriptor read error */
#define D64_XS1_XE_COREE	0x50000000	/* core error */

/* receive channel control */
#define D64_RC_RE		0x00000001	/* receive enable */
#define D64_RC_RO_MASK		0x000000fe	/* receive frame offset */
#define D64_RC_RO_SHIFT		1
#define D64_RC_FM		0x00000100	/* direct fifo receive (pio) mode */
#define D64_RC_SH		0x00000200	/* separate rx header descriptor enable */
#define D64_RC_OC		0x00000400	/* overflow continue */
#define D64_RC_PD		0x00000800	/* parity check disable */
#define D64_RC_AE		0x00030000	/* address extension bits */
#define D64_RC_AE_SHIFT		16

/* receive descriptor table pointer */
#define D64_RP_LD_MASK		0x00000fff	/* last valid descriptor */

/* receive channel status */
#define D64_RS0_CD_MASK		0x00001fff	/* current descriptor pointer */
#define D64_RS0_RS_MASK		0xf0000000     	/* receive state */
#define D64_RS0_RS_SHIFT		28
#define D64_RS0_RS_DISABLED	0x00000000	/* disabled */
#define D64_RS0_RS_ACTIVE	0x10000000	/* active */
#define D64_RS0_RS_IDLE		0x20000000	/* idle wait */
#define D64_RS0_RS_STOPPED	0x30000000	/* stopped */
#define D64_RS0_RS_SUSP		0x40000000	/* suspend pending */

#define D64_RS1_AD_MASK		0x0001ffff	/* active descriptor */
#define D64_RS1_RE_MASK		0xf0000000     	/* receive errors */
#define D64_RS1_RE_SHIFT		28
#define D64_RS1_RE_NOERR	0x00000000	/* no error */
#define D64_RS1_RE_DPO		0x10000000	/* descriptor protocol error */
#define D64_RS1_RE_DFU		0x20000000	/* data fifo overflow */
#define D64_RS1_RE_DTE		0x30000000	/* data transfer error */
#define D64_RS1_RE_DESRE	0x40000000	/* descriptor read error */
#define D64_RS1_RE_COREE	0x50000000	/* core error */

/* fifoaddr */
#define D64_FA_OFF_MASK		0xffff		/* offset */
#define D64_FA_SEL_MASK		0xf0000		/* select */
#define D64_FA_SEL_SHIFT	16
#define D64_FA_SEL_XDD		0x00000		/* transmit dma data */
#define D64_FA_SEL_XDP		0x10000		/* transmit dma pointers */
#define D64_FA_SEL_RDD		0x40000		/* receive dma data */
#define D64_FA_SEL_RDP		0x50000		/* receive dma pointers */
#define D64_FA_SEL_XFD		0x80000		/* transmit fifo data */
#define D64_FA_SEL_XFP		0x90000		/* transmit fifo pointers */
#define D64_FA_SEL_RFD		0xc0000		/* receive fifo data */
#define D64_FA_SEL_RFP		0xd0000		/* receive fifo pointers */
#define D64_FA_SEL_RSD		0xe0000		/* receive frame status data */
#define D64_FA_SEL_RSP		0xf0000		/* receive frame status pointers */

/* descriptor control flags 1 */
#define D64_CTRL1_TS		((uint32)1 << 27)	/* tx time stamp */
#define D64_CTRL1_EOT		((uint32)1 << 28)	/* end of descriptor table */
#define D64_CTRL1_IOC		((uint32)1 << 29)	/* interrupt on completion */
#define D64_CTRL1_EOF		((uint32)1 << 30)	/* end of frame */
#define D64_CTRL1_SOF		((uint32)1 << 31)	/* start of frame */

/* descriptor control flags 2 */
#define D64_CTRL2_BC_MASK	0x00007fff	/* buffer byte count mask */
#define D64_CTRL2_AE		0x00030000	/* address extension bits */
#define D64_CTRL2_AE_SHIFT	16

/* control flags in the range [27:20] are core-specific and not defined here */
#define D64_CTRL_CORE_MASK	0x0ff00000


/* packet headroom necessary to accomodate the largest header in the system, (i.e TXOFF).
 * By doing, we avoid the need  to allocate an extra buffer for the header when bridging to WL.
 * There is a compile time check in wlc.c which ensure that this value is at least as big
 * as TXOFF. This value is used in dma_rxfill (hnddma.c).
 */
#define BCMEXTRAHDROOM 160

/* map/unmap direction */
#define DMA_TX	1	/* TX direction for DMA */
#define DMA_RX	2	/* RX direction for DMA */


/* allocate/free shared (dma-able) consistent memory */
#define DMA_CONSISTENT_ALIGN	PAGE_SIZE

#define DMA_ALLOC_CONSISTENT(osh, size, pap) \
	osl_dma_alloc_consistent((osh), (size), (pap))
#define DMA_FREE_CONSISTENT(osh, va, size, pa) \
	osl_dma_free_consistent((osh), (void *)(va), (size), (pa))


/* map/unmap shared (dma-able) memory */
#define DMA_MAP(osh, chan, va, size, direction, p) \
	osl_dma_map((osh), (chan), (va), (size), (direction))

#define DMA_UNMAP(osh, chan, pa, size, direction, p) \
	osl_dma_unmap((osh), (chan), (pa), (size), (direction))


/* shared (dma-able) memory access macros */

#define R_SM(r)			*(r)
#define W_SM(r, v)		(*(r) = (v))
#define BZERO_SM(r, len)	memset((r), '\0', (len))

/* host/bus architecture-specific byte swap */
#define BUS_SWAP32(v)		(v)

/* register access macros */

#define R_REG(osh, r)	((sizeof *(r) == sizeof (uint32)) ? readl((uint32 *)(r)) : readw((uint16*)(r)))
#define W_REG(osh, r, v)							\
	do {								\
		if (sizeof *(r) == sizeof (uint32))			\
			writel(v, (uint32*)r);					\
		else							\
			writew((uint16)(v), (uint16*)(r));	\
	} while (0)

#define AND_REG(osh, r, v)  W_REG(osh, (r), R_REG(osh, r) & (v))
#define OR_REG(osh, r, v)   W_REG(osh, (r), R_REG(osh, r) | (v))


/* packet primitives */
#define PKTGET(osh, chan, len, send)		osl_pktget((osh), (chan), (len))
#define PKTFREE(osh, chan, skb, send)		osl_pktfree((osh), (chan), (skb), (send))
#define PKTDATA(osh, chan, skb, send)		osl_pktdata((osh), (chan), (skb), (send))
#define PKTLEN(osh, chan, skb, send)		osl_pktlen((osh), (chan), (skb), (send))
#define PKTNEXT(osh, chan, skb, send)		osl_pktnext((osh), (chan), (skb), (send))
#define PKTSETLEN(osh, chan, skb, len)	osl_pktsetlen((osh), (chan), (skb), (len))
#define PKTPUSH(osh, skb, bytes)	osl_pktpush((osh), (skb), (bytes))
#define PKTPULL(osh, chan, skb, bytes)	osl_pktpull((osh), (chan), (skb), (bytes))
#define PKTDUP(osh, skb)		osl_pktdup((osh), (skb))
#define PKTFLAGTS(osh, lb)      (0)
#define PKTFROM(osh, chan)        osl_pktfrom((osh),(chan))


extern void *osl_pktget(void *osh, int chan, uint len);
extern void osl_pktfree(void *osh, int chan, void *skb, bool send);
extern unsigned char *osl_pktdata(void *osh, int chan, void *skb, bool send);
extern unsigned int osl_pktlen(void *osh, int chan, void *skb, bool send);
extern void osl_pktsetlen(void *osh, int chan, void *skb, unsigned int len);
extern void *osl_pktnext(void *osh, int chan, void *skb, bool send);
extern unsigned char *osl_pktpull(void *osh, int chan, void *skb, int bytes);
extern uint32 osl_dma_map(void *osh, int chan, void *va, uint size, int direction);
extern void osl_dma_unmap(void *osh, int chan, uint pa, uint size, int direction);
extern void *osl_dma_alloc_consistent(void *osh, uint size, ulong *pap);
extern void osl_dma_free_consistent(void *osh, void *va, uint size,
				    ulong pa);
extern int osl_pktfrom(void *osh, int chan);


#include <linux/delay.h>

#define	OSL_DELAY(usec)	udelay(usec)
/*
 * Spin at most 'us' microseconds while 'exp' is true.
 * Caller should explicitly test 'exp' when this completes
 * and take appropriate error action if 'exp' is still true.
 */
#define SPINWAIT(exp, us) { \
	uint countdown = (us) + 9; \
	while ((exp) && (countdown >= 10)) {\
		OSL_DELAY(10); \
		countdown -= 10; \
	} \
}
#define	ROUNDUP(x, y)		((((ulong)(x)+((y)-1))/(y))*(y))



#endif	/* _knet_hnddma_h_ */
