/*
 * VxWorks 5.x END OS Layer
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: vx_osl.c,v 1.5 Broadcom SDK $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <vxshared.h>
#include <cacheLib.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <bcmallocache.h>

#include <drv/pci/pciConfigLib.h>
#include <logLib.h>
#include "tickLib.h"
#include "sysLib.h"
#include "taskLib.h"
#include <netLib.h>
#include <wdLib.h>
#include <time.h>

/* PED: shunt packet pool */
#define SRAM_START          (0xb9000000)
#define SRAM_PHY_START      (0x19000000)
#define SRAM_PKTSIZE        (2048)
#define MAX_SRAM_PKTS       (224)
#define RXHEADER_LEN        (32)
static SHPKT *pool_shunt_pkts = NULL;

/* 
 * vx OSL expects this mipsclk variable to be exported by BSP, and it should
 * set to mips operating clock freq.
*/
#ifdef __mips__
extern uint32 mipsclk;
/* mips_c0_count_per_us default 100 assuming mips is running at 200 MHz */
static int c0_counts_per_us = 100;
#else

static int loopcount_per_us = 20000;
#endif

#define	PKTSETTAG(m, n)		(((M_BLK_ID)m)->mBlkPktHdr.len = (int)n)

#define TAGCACHE_RECLAIM_TIME	1000 /* Clean up the working set every 1 seconds */
#define TAGCACHE_LOCK(osh)		semTake((osh)->cache_lock, WAIT_FOREVER)
#define TAGCACHE_UNLOCK(osh)		semGive((osh)->cache_lock)

#define PCI_CFG_RETRY 10	/* PR15065: retry count for pci cfg accesses */
#define PCI_BAR0_WIN 0x80	/* Offset for BAR0 Window */

struct osl_info {
	END_OBJ *endobj;
	void *devinfo;
	bcmcache_t *pkttag_memp;	/* Working set of allocated memory */
	uint32 alloced;			/* Total allocated pkttags */
	WDOG_ID cache_wd;		/* Watchdog Timer Id for working set reclaim */
	SEM_ID cache_lock;		/* Lock for accessing cache */
	pktfree_cb_fn_t tx_fn;
	void *tx_ctx;
};

static bool osl_cachereclaim_timer_start(osl_t *osh);
static void osl_cachereclaim_timer(void * data);


osl_t *
osl_attach(void *pdev, END_OBJ *endobj, bool pkttag)
{
	static bool init_done = FALSE;
	osl_t *osh;

	/* One time inits */
	if (init_done == FALSE) {
#ifdef __mips__
		if (mipsclk == 0) {
			/* XXX: should never happen */
			ASSERT(mipsclk);
			return NULL;
		}

		/* co counts at 1/2 the speed of mips clock */
		c0_counts_per_us = mipsclk/2000000;

		if (c0_counts_per_us == 0) {
			ASSERT(c0_counts_per_us);
			return NULL;
		}
#endif

		init_done = TRUE;
	}

	osh = (osl_t *)malloc(sizeof(osl_t));

	if (osh) {
		bzero(osh, sizeof(osl_t));
		osh->devinfo = pdev;
		osh->endobj = endobj;
		if (pkttag) {
			/* Start a timer to reclaim buffers from cache, and create the cache */
			if (!osl_cachereclaim_timer_start(osh)) {
				free(osh);
				return NULL;
			}
			osh->pkttag_memp = bcmcache_create(osh, "pkttag", OSL_PKTTAG_SZ);
		}
	}

	return (void*)osh;
}

void
osl_detach(osl_t *osh)
{
	if (osh == NULL)
		return;

	/* Destroy working set cache if exists */
	if (osh->pkttag_memp) {
#ifdef BCMDBG
		if (osh->alloced != 0)
			printf("Pkttag leak of %d pkttags. Packets leaked\n",
			       osh->alloced);
#endif /* BCMDBG */
		ASSERT(osh->alloced == 0);
		bcmcache_destroy(osh->pkttag_memp);
	}

	free(osh);
}

void
osl_pktfree_cb_set(osl_t *osh, pktfree_cb_fn_t tx_fn, void *tx_ctx)
{
	osh->tx_fn = tx_fn;
	osh->tx_ctx = tx_ctx;
}



/* Allocate a new packet. Attach a pkttag if requested
 * If pkttag allocation fails, pkt is not allocated.
 */
void*
osl_pktget(osl_t *osh, unsigned int len)
{
    /* PED: remove parameter 'send' */
	END_OBJ *end = osh->endobj;
	M_BLK_ID m;
	void *pkttag = NULL;

	/* size requested should fit in our cluster buffer */
	ASSERT(len <= PKTBUFSZ);

	if ((m = netTupleGet(end->pNetPool, (len + PKTRSV), M_DONTWAIT, MT_DATA, FALSE))) {

		/* reserve a few bytes */
		m->mBlkHdr.mData += PKTRSV;
		m->mBlkHdr.mLen = len;

		/* Our QOS/WMM implementation mandates that the default
		 * netBufLib routine is used to free the cluster
		 */
		ASSERT(m->pClBlk->pClFreeRtn == NULL);

	}

	return ((void*) m);
}

/* Free the packet. Free the packettags from the chained packets */
void
osl_pktfree(M_BLK_ID m) /* PED: remove osh parameter */
{
	netMblkClChainFree(m);
}

uchar *
osl_pktpush(osl_t *osh, M_BLK_ID m, unsigned int nbytes)
{
	ASSERT(M_HEADROOM(m) >= (int)nbytes);
	m->mBlkHdr.mData -= nbytes;
	m->mBlkHdr.mLen += nbytes;
	return ((uchar *)(m->mBlkHdr.mData));
}

uchar*
osl_pktpull(osl_t *osh, M_BLK_ID m, unsigned int nbytes)
{
	ASSERT((int)nbytes <= m->mBlkHdr.mLen);
	m->mBlkHdr.mData += nbytes;
	m->mBlkHdr.mLen -= nbytes;
	return ((uchar *)(m->mBlkHdr.mData));
}

/* Clone a packet. The cloned packet gets a pkttag if requested
 * The pkttag contents are NOT cloned. Also, if pkttag cannot be allocated
 * cloning fails. For chained buffers, only the head buffer gets a pkttag.
 */
void*
osl_pktdup(osl_t *osh, void *p)
{
	M_BLK_ID m, nextm;
	void *pkttag = NULL;


	m = netMblkChainDup(((M_BLK_ID)p)->pClBlk->pNetPool,
	                    (M_BLK_ID)p, 0, M_COPYALL, M_DONTWAIT);


	return (m);
}

#ifdef BCMDBG_ASSERT
void
osl_assert(char *exp, char *file, int line)
{
	char tempbuf[255];

	sprintf(tempbuf, "\nassertion \"%s\" failed: file \"%s\", line %d\n", exp, file, line);
	logMsg(tempbuf, 0, 0, 0, 0, 0, 0);
	abort();
}
#endif /* BCMDBG_ASSERT */

void*
osl_dma_alloc_consistent(osl_t *osh, unsigned int size, ulong *pap)
{
	void *va;

	va = cacheDmaMalloc(size);
	*pap = (ulong)CACHE_DMA_VIRT_TO_PHYS(va);
	return (va);
}

void
osl_dma_free_consistent(osl_t *osh, unsigned int size, void *va, ulong pa)
{
	cacheDmaFree(va);
}

void*
osl_dma_map(osl_t *osh, void *va, unsigned int size, unsigned int direction)
{
    if (((uint)va & 0x20000000) == 0)
    {
	if (direction == DMA_TX)
		cacheFlush(DATA_CACHE, va, size);
	else
		cacheInvalidate(DATA_CACHE, va, size);
    }

	return ((void*)CACHE_DMA_VIRT_TO_PHYS(va));
}

void
osl_delay(unsigned int us)
{
#ifdef __mips__
	uint32 curr, interval;

	curr = readCount();
	
	ASSERT(c0_counts_per_us);
	interval = us * c0_counts_per_us;

	while (readCount() - curr < interval);
#else /* __mips__ */
	
	uint i;

	printf("vxWorks wl builds for x86 platforms doesn't work\n");
	ASSERT(0);

	i = us * loopcount_per_us;
	while (i--);
#endif /* __mips__ */
}

uint32
osl_pci_read_config(osl_t *osh, unsigned int offset, unsigned int size)
{
	uint32 val;
	uint retry = PCI_CFG_RETRY;	 /* PR15065: faulty cardbus controller bug */
	pciinfo_t *pciinfo = (pciinfo_t *)(osh->devinfo);

	ASSERT(size == 4);

	do {
		pciConfigInLong(pciinfo->bus, pciinfo->dev, pciinfo->func, offset, &val);
		if (val != 0xffffffff)
			break;
	} while (retry--);

#ifdef BCMDBG
	if (retry < PCI_CFG_RETRY)
		printf("PCI CONFIG READ access to %d required %d retries\n", offset,
		       (PCI_CFG_RETRY - retry));
#endif /* BCMDBG */

return (val);
}

void
osl_pci_write_config(osl_t *osh, unsigned int offset, unsigned int size, unsigned int val)
{
	uint retry = PCI_CFG_RETRY;	 /* PR15065: faulty cardbus controller bug */
	pciinfo_t *pciinfo = (pciinfo_t *)(osh->devinfo);

	ASSERT(size == 4);

	do {
		pciConfigOutLong(pciinfo->bus, pciinfo->dev, pciinfo->func, offset, val);
		/* PR15065: PCI_BAR0_WIN is believed to be the only pci cfg write that can occur
		 * when dma activity is possible
		 */
		if (offset != PCI_BAR0_WIN)
			break;
		if (osl_pci_read_config(osh, offset, size) == val)
			break;
	} while (retry--);

#ifdef BCMDBG
if (retry < PCI_CFG_RETRY)
	printf("PCI CONFIG WRITE access to %d required %d retries\n", offset,
	       (PCI_CFG_RETRY - retry));
#endif /* BCMDBG */
}

/* return bus# for the pci device pointed by osh->devinfo */
uint
osl_pci_bus(osl_t *osh)
{
	pciinfo_t *pciinfo = (pciinfo_t *)(osh->devinfo);

	return pciinfo->bus;
}

/* return slot# for the pci device pointed by osh->devinfo */
uint
osl_pci_slot(osl_t *osh)
{
	pciinfo_t *pciinfo = (pciinfo_t *)(osh->devinfo);

	return pciinfo->dev;
}

void
osl_pcmcia_read_attr(osl_t *osh, unsigned int offset, void *buf, int size)
{
	int i;
	void *shared = osh->devinfo;

	for (i = 0; i < size; i++)
		((uint8*)buf)[i] = rreg8(((uint8*)shared + (offset + i) * 2));
}

void
osl_pcmcia_write_attr(osl_t *osh, unsigned int offset, void *buf, int size)
{
	int i;
	void *shared = osh->devinfo;

	for (i = 0; i < size; i++)
		wreg8(((uint8*)shared + (offset + i) * 2), ((uint8*)buf)[i]);
}

/* translate bcmerros into vx errors */
int
osl_error(int bcmerror)
{
	if (bcmerror)
		return ERROR;
	else
		return OK;
}

/* Convert a native(OS) packet to driver packet.
 * In the process, native packet is destroyed, there is no copying
 * Also, a packettag is attached to the head buffer if requested.
 * The conversion fails if pkttag cannot be allocated
 */
void *
osl_pkt_frmnative(osl_t *osh, M_BLK_ID m)
{

	return (void *)m;
}

/* Convert a driver packet to native(OS) packet
 * In the process, packettag is removed
 */
M_BLK_ID
osl_pkt_tonative(osl_t *osh, void *pkt)
{
	M_BLK_ID m = (M_BLK_ID) pkt;
	
	
	return m;
}

/* Cache Reclaim routines */
/* Free some buffers every 1 second */
static void
osl_cachereclaim(void * data)
{
	osl_t *osh = (osl_t *)data;
	uint ticks = TAGCACHE_RECLAIM_TIME / (1000/ sysClkRateGet());
	if (TAGCACHE_RECLAIM_TIME % (1000 / sysClkRateGet()))
		ticks++;

	TAGCACHE_LOCK(osh);
	bcmcache_reclaim(osh->pkttag_memp);
	TAGCACHE_UNLOCK(osh);

	wdStart(osh->cache_wd, ticks, (FUNCPTR) osl_cachereclaim_timer, (int) osh);
}

/* Timer handler */
static void
osl_cachereclaim_timer(void * data)
{
	netJobAdd((FUNCPTR)osl_cachereclaim, (int)data, 0, 0, 0, 0);
}

/* Start the 1 second timer */
static bool
osl_cachereclaim_timer_start(osl_t *osh)
{
	uint ticks;

	if ((osh->cache_lock = semBCreate(SEM_Q_FIFO, SEM_FULL)) == NULL)
		return FALSE;

	if ((osh->cache_wd = wdCreate()) == NULL)
		return FALSE;

	ticks = TAGCACHE_RECLAIM_TIME / (1000/ sysClkRateGet());
	if (TAGCACHE_RECLAIM_TIME % (1000 / sysClkRateGet()))
		ticks++;

	if ((wdStart(osh->cache_wd, ticks, (FUNCPTR) osl_cachereclaim_timer, (int) osh)) != OK)
		return FALSE;
	return TRUE;
}

/* PED */
void 
osl_shunt_pkt_init(void)
{
    static int inited = 0;
    
    if (!inited) {
        uint i;
        SHPKT *p;

        inited = 1;
        for(i=0; i<MAX_SRAM_PKTS; i++) {
            p = (SHPKT *)malloc(sizeof(SHPKT));
            p->buffer = (uchar *)SRAM_START + SRAM_PKTSIZE * i;
            p->buffer_p = (uchar *)SRAM_PHY_START + SRAM_PKTSIZE * i;
            p->body_p = p->buffer_p + RXHEADER_LEN;
            p->next = pool_shunt_pkts;
            pool_shunt_pkts = p;
        }
    }
}

/* PED */
void *
osl_pktget_shunt(void)
{
    SHPKT *p = pool_shunt_pkts;
    pool_shunt_pkts = p->next;
    return p;
}

/* PED */
void
osl_pktfree_shunt(SHPKT *p)
{
    p->next = pool_shunt_pkts;
    pool_shunt_pkts = p;
}

