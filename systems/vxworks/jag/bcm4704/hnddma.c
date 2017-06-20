/*
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/
/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Copyright(c) 2001, Broadcom Corp.
 * $Id: hnddma.c,v 1.1 2004/02/24 07:47:00 csm Exp $
 */

#include <hnbutypedefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>

struct dma_info;	/* forward declaration */
#define di_t struct dma_info
#include <hnddma.h>

#define BCMDBG 1
/* debug/trace */
#ifdef BCMDBG
#define	DMA_ERROR(args) if (!(*di->msg_level & 1)) ; else printf args
#define	DMA_TRACE(args) if (!(*di->msg_level & 2)) ; else printf args
#else
#define	DMA_ERROR(args)
#define	DMA_TRACE(args)
#endif

/* default dma message level(if input msg_level pointer is null in dma_attach()) */
static uint dma_msg_level = 0;

#define	MAXNAMEL	8
#define	MAXDD		(DMAMAXRINGSZ / sizeof (dmadd_t))

/* dma engine software state */
typedef struct dma_info {
	hnddma_t	hnddma;		/* exported structure */
	uint		*msg_level;	/* message level pointer */

	char		name[MAXNAMEL];	/* callers name for diag msgs */
	void		*drv;		/* driver handle */
	void		*dev;		/* device handle */
	dmaregs_t	*regs;		/* dma engine registers */

	dmadd_t		*txd;		/* pointer to chip-specific tx descriptor ring */
	uint		txin;		/* index of next descriptor to reclaim */
	uint		txout;		/* index of next descriptor to post */
	uint		txavail;	/* # free tx descriptors */
	void		*txp[MAXDD];	/* parallel array of pointers to packets */
	void		*txdpa;		/* physical address of descriptor ring */
	uint		txdalign;	/* #bytes added to alloc'd mem to align txd */

	dmadd_t		*rxd;		/* pointer to chip-specific rx descriptor ring */
	uint		rxin;		/* index of next descriptor to reclaim */
	uint		rxout;		/* index of next descriptor to post */
	void		*rxp[MAXDD];	/* parallel array of pointers to packets */
	void		*rxdpa;		/* physical address of descriptor ring */
	uint		rxdalign;	/* #bytes added to alloc'd mem to align rxd */

	/* tunables */
	uint		ntxd;		/* # tx descriptors */
	uint		nrxd;		/* # rx descriptors */
	uint		rxbufsize;	/* rx buffer size in bytes */
	uint		nrxpost;	/* # rx buffers to keep posted */
	uint		rxoffset;	/* rxcontrol offset */
	uint		ddoffset;	/* add to get dma address of descriptor ring */
	uint		dataoffset;	/* add to get dma address of data buffer */
} dma_info_t;

/* descriptor bumping macros */
#define	NEXTTXD(i)	((i + 1) & (di->ntxd - 1))
#define	PREVTXD(i)	((i - 1) & (di->ntxd - 1))
#define	NEXTRXD(i)	((i + 1) & (di->nrxd - 1))
#define	NTXDACTIVE(h, t)	((t - h) & (di->ntxd - 1))
#define	NRXDACTIVE(h, t)	((t - h) & (di->nrxd - 1))

/* macros to convert between byte offsets and indexes */
#define	B2I(bytes)	((bytes) / sizeof (dmadd_t))
#define	I2B(index)	((index) * sizeof (dmadd_t))

void*
dma_attach(void *drv, void *dev, char *name, dmaregs_t *regs, uint ntxd, uint nrxd,
	uint rxbufsize, uint nrxpost, uint rxoffset, uint ddoffset, uint dataoffset, uint *msg_level)
{
	dma_info_t *di;
	void *va;

	ASSERT(ntxd <= MAXDD);
	ASSERT(nrxd <= MAXDD);

	/* allocate private info structure */
	if ((di = MALLOC(sizeof (dma_info_t))) == NULL)
		return (NULL);
	bzero((char*)di, sizeof (dma_info_t));

	/* set message level */
	di->msg_level = msg_level ? msg_level : &dma_msg_level;

	DMA_TRACE(("%s: dma_attach: drv 0x%x dev 0x%x regs 0x%x ntxd %d nrxd %d rxbufsize %d nrxpost %d rxoffset %d ddoffset 0x%x dataoffset 0x%x\n", name, (uint)drv, (uint)dev, (uint)regs, ntxd, nrxd, rxbufsize, nrxpost, rxoffset, ddoffset, dataoffset));

	/* make a private copy of our callers name */
	strncpy(di->name, name, MAXNAMEL);
	di->name[MAXNAMEL-1] = '\0';

	di->drv = drv;
	di->dev = dev;
	di->regs = regs;

	/* allocate transmit descriptor ring */
	if (ntxd) {
		if ((va = DMA_ALLOC_CONSISTENT(dev, (DMAMAXRINGSZ + DMARINGALIGN), &di->txdpa)) == NULL)
			goto fail;
		di->txd = (dmadd_t*) ROUNDUP(va, DMARINGALIGN);
		di->txdalign = ((uint)di->txd - (uint)va);
		di->txdpa = (void*) ((uint)di->txdpa + di->txdalign);
		ASSERT(ISALIGNED(di->txd, DMARINGALIGN));
	}

	/* allocate receive descriptor ring */
	if (nrxd) {
		if ((va = DMA_ALLOC_CONSISTENT(dev, (DMAMAXRINGSZ + DMARINGALIGN), &di->rxdpa)) == NULL)
			goto fail;
		di->rxd = (dmadd_t*) ROUNDUP(va, DMARINGALIGN);
		di->rxdalign = ((uint)di->rxd - (uint)va);
		di->rxdpa = (void*) ((uint)di->rxdpa + di->rxdalign);
		ASSERT(ISALIGNED(di->rxd, DMARINGALIGN));
	}

	/* save tunables */
	di->ntxd = ntxd;
	di->nrxd = nrxd;
	di->rxbufsize = rxbufsize;
	di->nrxpost = nrxpost;
	di->rxoffset = rxoffset;
	di->ddoffset = ddoffset;
	di->dataoffset = dataoffset;

	return ((void*)di);

fail:
	dma_detach((void*)di);
	return (NULL);
}

void
dma_detach(dma_info_t *di)
{
	if (di == NULL)
		return;

	DMA_TRACE(("%s: dma_detach\n", di->name));

	/* shouldn't be here if descriptors are unreclaimed */
	ASSERT(di->txin == di->txout);
	ASSERT(di->rxin == di->rxout);

	/* free dma descriptor rings */
	if (di->txd)
		DMA_FREE_CONSISTENT(di->dev, (void *)(di->txd - di->txdalign), (DMAMAXRINGSZ + DMARINGALIGN), di->txdpa);
	if (di->rxd)
		DMA_FREE_CONSISTENT(di->dev, (void *)(di->rxd - di->rxdalign), (DMAMAXRINGSZ + DMARINGALIGN), di->rxdpa);

	/* free our private info structure */
	MFREE((void*)di, sizeof (dma_info_t));
}

/* 
 * PR2414 WAR: When the DMA channel is in the FetchDescriptor state,
 * it does not notice that the enable bit has been turned off. If the
 * enable bit is turned back on before the descriptor fetch completes,
 * at least some of the DMA channel does not get reset. In particular,
 * it will fetch a descriptor from the address it was trying to fetch
 * from when it was disabled. 
 *
 * For all cores other than USB, the workaround is simply to clear the
 * enable bit, and then read back status until the state shows up as
 * Disabled before re-enabling the channel.
 */

void
dma_txreset(dma_info_t *di)
{
	uint32 status;

	DMA_TRACE(("%s: dma_txreset\n", di->name));

	/* address PR8249/PR7577 issue */
	/* suspend tx DMA first */
	W_REG(&di->regs->xmtcontrol, XC_SE);
	SPINWAIT((status = (R_REG(&di->regs->xmtstatus) & XS_XS_MASK)) != XS_XS_DISABLED &&
		 status != XS_XS_IDLE &&
		 status != XS_XS_STOPPED,
		 10000);

	/* PR2414 WAR: DMA engines are not disabled until transfer finishes */
	W_REG(&di->regs->xmtcontrol, 0);
	SPINWAIT((status = (R_REG(&di->regs->xmtstatus) & XS_XS_MASK)) != XS_XS_DISABLED,
		 10000);

	if (status != XS_XS_DISABLED) {
		DMA_ERROR(("%s: dma_txreset: dma cannot be stopped\n", di->name));
	}

	/* wait for the last transaction to complete */
	OSL_DELAY(300);
}

void
dma_rxreset(dma_info_t *di)
{
	uint32 status;

	DMA_TRACE(("%s: dma_rxreset\n", di->name));

	/* PR2414 WAR: DMA engines are not disabled until transfer finishes */
	W_REG(&di->regs->rcvcontrol, 0);
	SPINWAIT((status = (R_REG(&di->regs->rcvstatus) & RS_RS_MASK)) != RS_RS_DISABLED,
		 10000);

	if (status != RS_RS_DISABLED) {
		DMA_ERROR(("%s: dma_rxreset: dma cannot be stopped\n", di->name));
	}
}

void
dma_txinit(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txinit\n", di->name));

	di->txin = di->txout = 0;
	di->txavail = di->ntxd - 1;

	/* clear tx descriptor ring */
	BZERO_SM((void*)di->txd, (di->ntxd * sizeof (dmadd_t)));

	W_REG(&di->regs->xmtcontrol, XC_XE);
	W_REG(&di->regs->xmtaddr, ((uint32)di->txdpa + di->ddoffset));
}

bool
dma_txenabled(dma_info_t *di)
{
	uint32 xc;

	/* If the chip is dead, it is not enabled :-) */
	xc = R_REG(&di->regs->xmtcontrol);
	return ((xc != 0xffffffff) && (xc & XC_XE));
}

void
dma_txsuspend(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txsuspend\n", di->name));
	OR_REG(&di->regs->xmtcontrol, XC_SE);
}

void
dma_txresume(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_txresume\n", di->name));
	AND_REG(&di->regs->xmtcontrol, ~XC_SE);
}

bool
dma_txsuspended(dma_info_t *di)
{
	uint32 xc;
	uint32 xs;

	xc = R_REG(&di->regs->xmtcontrol);
	if (xc & XC_SE) {
		xs = R_REG(&di->regs->xmtstatus);
		return ((xs & XS_XS_MASK) == XS_XS_IDLE);
	}
	return 0;
}

bool
dma_txstopped(dma_info_t *di)
{
	return ((R_REG(&di->regs->xmtstatus) & XS_XS_MASK) == XS_XS_STOPPED);
}

bool
dma_rxstopped(dma_info_t *di)
{
	return ((R_REG(&di->regs->rcvstatus) & RS_RS_MASK) == RS_RS_STOPPED);
}

void
dma_fifoloopbackenable(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_fifoloopbackenable\n", di->name));
	OR_REG(&di->regs->xmtcontrol, XC_LE);
}

void
dma_rxinit(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxinit\n", di->name));

	di->rxin = di->rxout = 0;

	/* clear rx descriptor ring */
	BZERO_SM((void*)di->rxd, (di->nrxd * sizeof (dmadd_t)));

	dma_rxenable(di);
	W_REG(&di->regs->rcvaddr, ((uint32)di->rxdpa + di->ddoffset));
}

void
dma_rxenable(dma_info_t *di)
{
	DMA_TRACE(("%s: dma_rxenable\n", di->name));
	W_REG(&di->regs->rcvcontrol, ((di->rxoffset << RC_RO_SHIFT) | RC_RE));
}

bool
dma_rxenabled(dma_info_t *di)
{
	uint32 rc;

	rc = R_REG(&di->regs->rcvcontrol);
	return ((rc != 0xffffffff) && (rc & RC_RE));
}

/*
 * The BCM47XX family supports full 32bit dma engine buffer addressing so
 * dma buffers can cross 4 Kbyte page boundaries.
 */
int
dma_txfast(dma_info_t *di, void *p0, uint32 coreflags)
{
	void *p, *next;
	uchar *data;
	uint len;
	uint txout;
	uint32 ctrl;
	uint32 pa;
	DMA_TRACE(("%s: dma_txfast\n", di->name));

	txout = di->txout;
	ctrl = 0;

	/*
	 * Walk the chain of packet buffers
	 * allocating and initializing transmit descriptor entries.
	 */
	for (p = p0; p; p = next) {
		data = PKTDATA(di->drv, p);
		len = PKTLEN(di->drv, p);
		next = PKTNEXT(di->drv, p);

		/* return nonzero if out of tx descriptors */
		if (NEXTTXD(txout) == di->txin)
			goto outoftxd;

		/* PR988 - skip zero length buffers */
		if (len == 0)
			continue;

		/* get physical address of buffer start */
		pa = (uint32) DMA_MAP(di->dev, data, len, DMA_TX, p);

		/* build the descriptor control value */
		ctrl = len & CTRL_BC_MASK;

		/* PR3697: Descriptor flags are not ignored for descriptors where SOF is clear */
		ctrl |= coreflags;
		
		if (p == p0)
			ctrl |= CTRL_SOF;
		if (next == NULL)
			ctrl |= (CTRL_IOC | CTRL_EOF);
		if (txout == (di->ntxd - 1))
			ctrl |= CTRL_EOT;

		/* init the tx descriptor */
		W_SM(&di->txd[txout].ctrl, BUS_SWAP32(ctrl));
		W_SM(&di->txd[txout].addr, BUS_SWAP32(pa + di->dataoffset));

		ASSERT(di->txp[txout] == NULL);

		txout = NEXTTXD(txout);
	}

	/* if last txd eof not set, fix it */
	if (!(ctrl & CTRL_EOF))
		W_SM(&di->txd[PREVTXD(txout)].ctrl, BUS_SWAP32(ctrl | CTRL_IOC | CTRL_EOF));

	/* save the packet */
	di->txp[di->txout] = p0;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	W_REG(&di->regs->xmtptr, I2B(txout));

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
	PKTFREE(di->drv, p0, TRUE);
	di->txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

#define	PAGESZ		4096
#define	PAGEBASE(x)	((uint)(x) & ~4095)

/*
 * Just like above except go through the extra effort of splitting
 * buffers that cross 4Kbyte boundaries into multiple tx descriptors.
 */
int
dma_tx(dma_info_t *di, void *p0, uint32 coreflags)
{
	void *p, *next;
	uchar *data;
	uint plen, len;
	uchar *page, *start, *end;
	uint txout;
	uint32 ctrl;
	uint32 pa;

	DMA_TRACE(("%s: dma_tx\n", di->name));

	txout = di->txout;
	ctrl = 0;

	/*
	 * Walk the chain of packet buffers
	 * splitting those that cross 4 Kbyte boundaries
	 * allocating and initializing transmit descriptor entries.
	 */
	for (p = p0; p; p = next) {
		data = PKTDATA(di->drv, p);
		plen = PKTLEN(di->drv, p);
		next = PKTNEXT(di->drv, p);

		/* PR988 - skip zero length buffers */
		if (plen == 0)
			continue;

		for (page = (uchar*)PAGEBASE(data);
			page <= (uchar*)PAGEBASE(data + plen - 1);
			page += PAGESZ) {

			/* return nonzero if out of tx descriptors */
			if (NEXTTXD(txout) == di->txin)
				goto outoftxd;

			start = (page == (uchar*)PAGEBASE(data))?  data: page;
			end = (page == (uchar*)PAGEBASE(data + plen))?
				(data + plen): (page + PAGESZ);
			len = end - start;

			/* build the descriptor control value */
			ctrl = len & CTRL_BC_MASK;

			/* PR3697: Descriptor flags are not ignored for descriptors where SOF is clear */
			ctrl |= coreflags;

			if ((p == p0) && (start == data))
				ctrl |= CTRL_SOF;
			if ((next == NULL) && (end == (data + plen)))
				ctrl |= (CTRL_IOC | CTRL_EOF);
			if (txout == (di->ntxd - 1))
				ctrl |= CTRL_EOT;

			/* get physical address of buffer start */
			pa = (uint32) DMA_MAP(di->dev, start, len, DMA_TX, p);

			/* init the tx descriptor */
			W_SM(&di->txd[txout].ctrl, BUS_SWAP32(ctrl));
			W_SM(&di->txd[txout].addr, BUS_SWAP32(pa + di->dataoffset));

			ASSERT(di->txp[txout] == NULL);

			txout = NEXTTXD(txout);
		}
	}

	/* if last txd eof not set, fix it */
	if (!(ctrl & CTRL_EOF))
		W_SM(&di->txd[PREVTXD(txout)].ctrl, BUS_SWAP32(ctrl | CTRL_IOC | CTRL_EOF));

	/* save the packet */
	di->txp[di->txout] = p0;

	/* bump the tx descriptor index */
	di->txout = txout;

	/* kick the chip */
	W_REG(&di->regs->xmtptr, I2B(txout));

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (0);

outoftxd:
	DMA_ERROR(("%s: dma_tx: out of txds\n", di->name));
	PKTFREE(di->drv, p0, TRUE);
	di->txavail = 0;
	di->hnddma.txnobuf++;
	return (-1);
}

/* returns a pointer to the next frame received, or NULL if there are no more */
void*
dma_rx(dma_info_t *di)
{
	void *p;
	uint len;
	int skiplen = 0;

	while ((p = dma_getnextrxp(di, FALSE))) {
		/* skip giant packets which span multiple rx descriptors */
		if (skiplen > 0) {
			skiplen -= di->rxbufsize;
			if (skiplen < 0)
				skiplen = 0;
			PKTFREE(di->drv, p, FALSE);
			continue;
		}

		len = ltoh16(*(uint16*)(PKTDATA(di->drv, p)));
		DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));

		/* bad frame length check */
		if (len > (di->rxbufsize - di->rxoffset)) {
			DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name, len));
			if (len > 0)
				skiplen = len - (di->rxbufsize - di->rxoffset);
			PKTFREE(di->drv, p, FALSE);
			di->hnddma.rxgiants++;
			continue;
		}

		/* set actual length */
		PKTSETLEN(di->drv, p, (di->rxoffset + len));

		break;
	}

	return (p);
}

/* post receive buffers */
void
dma_rxfill(dma_info_t *di)
{
	void *p;
	uint rxin, rxout;
	uint ctrl;
	uint n;
	uint i;
	uint32 pa;
	uint rxbufsize;

	/*
	 * Determine how many receive buffers we're lacking
	 * from the full complement, allocate, initialize,
	 * and post them, then update the chip rx lastdscr.
	 */

	rxin = di->rxin;
	rxout = di->rxout;
	rxbufsize = di->rxbufsize;

	n = di->nrxpost - NRXDACTIVE(rxin, rxout);

	DMA_TRACE(("%s: dma_rxfill: post %d\n", di->name, n));

	for (i = 0; i < n; i++) {
		if ((p = PKTGET(di->drv, rxbufsize, FALSE)) == NULL) {
			DMA_ERROR(("%s: dma_rxfill: out of rxbufs\n", di->name));
			di->hnddma.rxnobuf++;
			break;
		}

		/* PR3263 & PR3387 & PR4642 war: rxh.len=0 means dma writes not complete */
		*(uint32*)(OSL_UNCACHED(PKTDATA(di->drv, p))) = 0;

		pa = (uint32) DMA_MAP(di->dev, PKTDATA(di->drv, p), rxbufsize, DMA_RX, p);
		ASSERT(ISALIGNED(pa, 4));

		/* save the free packet pointer */
#if 0
		ASSERT(di->rxp[rxout] == NULL);
#endif
		di->rxp[rxout] = p;

		/* paranoia */
		ASSERT(R_SM(&di->rxd[rxout].addr) == 0);

		/* prep the descriptor control value */
		ctrl = rxbufsize;
		if (rxout == (di->nrxd - 1))
			ctrl |= CTRL_EOT;

		/* init the rx descriptor */
		W_SM(&di->rxd[rxout].ctrl, BUS_SWAP32(ctrl));
		W_SM(&di->rxd[rxout].addr, BUS_SWAP32(pa + di->dataoffset));

		DMA_TRACE(("%s: dma_rxfill:  ctrl %08x dataoffset: %08x\n", di->name, BUS_SWAP32(ctrl), BUS_SWAP32(pa + di->dataoffset)));

		rxout = NEXTRXD(rxout);
	}

	di->rxout = rxout;

	/* update the chip lastdscr pointer */
	W_REG(&di->regs->rcvptr, I2B(rxout));
}

void
dma_txreclaim(dma_info_t *di, bool forceall)
{
	void *p;

	DMA_TRACE(("%s: dma_txreclaim %s\n", di->name, forceall ? "all" : ""));

	while ((p = dma_getnexttxp(di, forceall)))
		PKTFREE(di->drv, p, TRUE);
}

/*
 * Reclaim next completed txd (txds if using chained buffers) and
 * return associated packet.
 * If 'force' is true, reclaim txd(s) and return associated packet
 * regardless of the value of the hardware "curr" pointer.
 */
void*
dma_getnexttxp(dma_info_t *di, bool forceall)
{
	uint start, end, i;
	void *txp;

	DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name, forceall ? "all" : ""));

	txp = NULL;

	/* if forcing, dma engine must be disabled */
	ASSERT(!forceall || !dma_txenabled(di));

	start = di->txin;
	if (forceall)
		end = di->txout;
	else
		end = B2I(R_REG(&di->regs->xmtstatus) & XS_CD_MASK);

	/* PR4738 - xmt disable/re-enable does not clear CURR */
	if ((start == 0) && (end > di->txout))
		goto bogus;

	for (i = start; i != end && !txp; i = NEXTTXD(i)) {
		DMA_UNMAP(di->dev, (BUS_SWAP32(R_SM(&di->txd[i].addr)) - di->dataoffset),
			  (BUS_SWAP32(R_SM(&di->txd[i].ctrl)) & CTRL_BC_MASK), DMA_TX, di->txp[i]);
		W_SM(&di->txd[i].addr, 0);
		txp = di->txp[i];
		di->txp[i] = NULL;
	}

	di->txin = i;

	/* tx flow control */
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

	return (txp);

bogus:
/*
	DMA_ERROR(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
		start, end, di->txout, forceall));
*/
	return (NULL);
}

void
dma_rxreclaim(dma_info_t *di)
{
	void *p;

	DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

	while ((p = dma_getnextrxp(di, TRUE)))
		PKTFREE(di->drv, p, FALSE);
}

void *
dma_getnextrxp(dma_info_t *di, bool forceall)
{
	uint i;
	void *rxp;

	/* if forcing, dma engine must be disabled */
	ASSERT(!forceall || !dma_rxenabled(di));

	i = di->rxin;

	/* return if no packets posted */
	if (i == di->rxout)
		return (NULL);

	/* ignore curr if forceall */
	if (!forceall && (i == B2I(R_REG(&di->regs->rcvstatus) & RS_CD_MASK)))
		return (NULL);

	/* get the packet pointer that corresponds to the rx descriptor */
	rxp = di->rxp[i];
	ASSERT(rxp);
	di->rxp[i] = NULL;

	/* clear this packet from the descriptor ring */
	DMA_UNMAP(di->dev, (BUS_SWAP32(R_SM(&di->rxd[i].addr)) - di->dataoffset),
		  di->rxbufsize, DMA_RX, rxp);
	W_SM(&di->rxd[i].addr, 0);

	di->rxin = NEXTRXD(i);

	return (rxp);
}

char*
dma_dump(dma_info_t *di, char *buf)
{
	buf += sprintf(buf, "txd 0x%lx txdpa 0x%lx txp 0x%lx txin %d txout %d\n",
		(ulong)di->txd, (ulong)di->txdpa, (ulong)di->txp, di->txin, di->txout);
	buf += sprintf(buf, "rxd 0x%lx rxdpa 0x%lx rxp 0x%lx rxin %d rxout %d\n",
		(ulong)di->rxd, (ulong)di->rxdpa, (ulong)di->rxp, di->rxin, di->rxout);
	buf += sprintf(buf, "xmtcontrol 0x%x xmtaddr 0x%x xmtptr 0x%x xmtstatus 0x%x\n",
		R_REG(&di->regs->xmtcontrol),
		R_REG(&di->regs->xmtaddr),
		R_REG(&di->regs->xmtptr),
		R_REG(&di->regs->xmtstatus));
	buf += sprintf(buf, "rcvcontrol 0x%x rcvaddr 0x%x rcvptr 0x%x rcvstatus 0x%x\n",
		R_REG(&di->regs->rcvcontrol),
		R_REG(&di->regs->rcvaddr),
		R_REG(&di->regs->rcvptr),
		R_REG(&di->regs->rcvstatus));
	return (buf);
}

uint
dma_getvar(dma_info_t *di, char *name)
{
	if (!strcmp(name, "&txavail"))
		return ((uint) &di->txavail);
	else {
		ASSERT(0);
	}
	return (0);
}

void
dma_txblock(dma_info_t *di)
{
	di->txavail = 0;
}

void
dma_txunblock(dma_info_t *di)
{
	di->txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
}

uint
dma_txactive(dma_info_t *di)
{
	return (NTXDACTIVE(di->txin, di->txout));
}
char*
dma_dumptx(dma_info_t *di, char *buf)
{
	buf += sprintf(buf, "txd 0x%lx txdpa 0x%lx txp 0x%lx txin %d txout %d txavail %d\n",
		(ulong)di->txd, di->txdpa, (ulong)di->txp, di->txin, di->txout, di->txavail);
	buf += sprintf(buf, "xmtcontrol 0x%x xmtaddr 0x%x xmtptr 0x%x xmtstatus 0x%x\n",
		R_REG(&di->regs->xmtcontrol),
		R_REG(&di->regs->xmtaddr),
		R_REG(&di->regs->xmtptr),
		R_REG(&di->regs->xmtstatus));
	return (buf);
}

char*
dma_dumprx(dma_info_t *di, char *buf)
{
	buf += sprintf(buf, "rxd 0x%lx rxdpa 0x%lx rxp 0x%lx rxin %d rxout %d\n",
		(ulong)di->rxd, di->rxdpa, (ulong)di->rxp, di->rxin, di->rxout);
	buf += sprintf(buf, "rcvcontrol 0x%x rcvaddr 0x%x rcvptr 0x%x rcvstatus 0x%x\n",
		R_REG(&di->regs->rcvcontrol),
		R_REG(&di->regs->rcvaddr),
		R_REG(&di->regs->rcvptr),
		R_REG(&di->regs->rcvstatus));
	return (buf);
}

