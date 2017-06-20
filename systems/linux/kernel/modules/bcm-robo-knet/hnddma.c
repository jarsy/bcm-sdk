/*
 * Generic Broadcom Home Networking Division (HND) DMA module.
 *
 *
 * $Id: hnddma.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */



#include <gmodule.h> /* Must be included first */
#include <linux-bde.h>

#include <shared/et/typedefs.h>
#include <shared/et/bcmgmacrxh.h>
#include <shared/et/bcmendian.h>

#include "hnddma.h"

#define BCMDBG 
/* debug/trace */
#ifdef BCMDBG
#define DMA_ERROR(args) if (!(*di->msg_level & DMA_DEBUG_MSG_ERR)); else gprintk args
#define DMA_TRACE(args) if (!(*di->msg_level & DMA_DEBUG_MSG_TRACE)); else gprintk args
#elif defined(BCMDBG_ERR)
#define DMA_ERROR(args) if (!(*di->msg_level & DMA_DEBUG_MSG_ERR)); else gprintk args
#define DMA_TRACE(args)
#else
#define DMA_ERROR(args)
#define DMA_TRACE(args)
#endif

/* default dma message level (if input msg_level pointer is null in dma_attach()) */
static uint dma_msg_level =
#ifdef BCMDBG_ERR
    1;
#else
    0;
#endif

#if defined(KEYSTONE)
/* 
 * Added the magic word to check if the received packets is erroneous
 * while the RX overflow happened.
 */
#define GMAC_ERR_PKTS_DETECT
#define GMAC_ERR_PKTS_MAGIC_TAG             0xdeadbeef
/* 
 * Since the first word and second word of received packet will be updated
 * and the buffer header is 32 bytes (8 words).
 * So the offset value of the magic word is (2 + 8)
 */
#define GMAC_ERR_PKTS_MAGIC_OFFSET          (10)
#endif /* KEYSTONE */

#define MAXNAMEL    8       /* 8 char names */


/* dma engine software state */
typedef struct dma_info {
    struct hnddma_pub hnddma;   /* exported structure, don't use hnddma_t,
                     * which could be const
                     */
    uint        *msg_level; /* message level pointer */
    char        name[MAXNAMEL]; /* callers name for diag msgs */
    int         dma_id;    

    void        *osh;       /* os handle */

    bool        dma64;      /* dma64 enabled */
    bool        addrext;    /* this dma engine supports DmaExtendedAddrChanges */
    bool        apitx;
    
    dma64regs_t *d64txregs; /* 64 bits dma tx engine registers */
    dma64regs_t *d64rxregs; /* 64 bits dma rx engine registers */

    uint        dma64align; /* either 8k or 4k depends on number of dd */
    dma64dd_t   *txd64;     /* pointer to dma64 tx descriptor ring */
    int         *txd64priv;    /* pointer to priviate info of the dma64 tx descriptor ring */
    uint        ntxd;       /* # tx descriptors tunable */
    uint        txin;       /* index of next descriptor to reclaim */
    uint        txout;      /* index of next descriptor to post */
    void        **txp;      /* pointer to parallel array of pointers to packets */
    ulong       txdpa;      /* physical address of descriptor ring */
    uint        txdalign;   /* #bytes added to alloc'd mem to align txd */
    uint        txdalloc;   /* #bytes allocated for the ring */

    dma64dd_t   *rxd64;     /* pointer to dma64 rx descriptor ring */
    uint        nrxd;       /* # rx descriptors tunable */
    uint        rxin;       /* index of next descriptor to reclaim */
    uint        rxout;      /* index of next descriptor to post */
    void        **rxp;      /* pointer to parallel array of pointers to packets */
    ulong       rxdpa;      /* physical address of descriptor ring */
    uint        rxdalign;   /* #bytes added to alloc'd mem to align rxd */
    uint        rxdalloc;   /* #bytes allocated for the ring */

    /* tunables */
    uint        rxbufsize;  /* rx buffer size in bytes,
                     * not including the extra headroom
                     */
    uint        nrxpost;    /* # rx buffers to keep posted */
    uint        rxoffset;   /* rxcontrol offset */
    uint        rxsephdrsize;
    uint        ddoffsetlow;    /* add to get dma address of descriptor ring, low 32 bits */
    uint        ddoffsethigh;   /*   high 32 bits */
    uint        dataoffsetlow;  /* add to get dma address of data buffer, low 32 bits */
    uint        dataoffsethigh; /*   high 32 bits */
} dma_info_t;


/* descriptor bumping macros */
#define XXD(x, n)   ((x) & ((n) - 1))   /* faster than %, but n must be power of 2 */
#define TXD(x)      XXD((x), di->ntxd)
#define RXD(x)      XXD((x), di->nrxd)
#define NEXTTXD(i)  TXD(i + 1)
#define PREVTXD(i)  TXD(i - 1)
#define NEXTRXD(i)  RXD(i + 1)
#define NTXDACTIVE(h, t)    TXD(t - h)
#define NRXDACTIVE(h, t)    RXD(t - h)

/* macros to convert between byte offsets and indexes */
#define B2I(bytes, type)    ((bytes) / sizeof(type))
#define I2B(index, type)    ((index) * sizeof(type))


/* common prototypes */
static bool _dma_isaddrext(dma_info_t *di);
static bool _dma_alloc(dma_info_t *di, uint direction);
static void _dma_detach(dma_info_t *di);
static void _dma_ddtable_init(dma_info_t *di, uint direction, ulong pa);
static void _dma_rxinit(dma_info_t *di);
static void *_dma_rx(dma_info_t *di);
static void _dma_rxfill(dma_info_t *di);
static void _dma_rxreclaim(dma_info_t *di);
static void _dma_rxenable(dma_info_t *di);
static void *_dma_getnextrxp(dma_info_t *di, bool forceall);

static void _dma_txblock(dma_info_t *di);
static void _dma_txunblock(dma_info_t *di);
static uint _dma_txactive(dma_info_t *di);
static uint _dma_rxactive(dma_info_t *di);
static uint _dma_txpending(dma_info_t *di);

static void *_dma_peeknexttxp(dma_info_t *di);
static uint * _dma_getvar(dma_info_t *di, const char *name);
static void _dma_counterreset(dma_info_t *di);
static void _dma_fifoloopbackenable(dma_info_t *di, bool on);


/* ** 64 bit DMA prototypes and stubs */
static bool dma64_alloc(dma_info_t *di, uint direction);
static bool dma64_txreset(dma_info_t *di);
static bool dma64_rxreset(dma_info_t *di);
static bool dma64_txsuspendedidle(dma_info_t *di);
static int  dma64_txfast(dma_info_t *di, void *p0, bool commit);
static void *dma64_getnexttxp(dma_info_t *di, bool forceall);
static void *dma64_getnextrxp(dma_info_t *di, bool forceall);
static void dma64_txrotate(dma_info_t *di);

static bool dma64_rxidle(dma_info_t *di);
static void dma64_txinit(dma_info_t *di);
static bool dma64_txenabled(dma_info_t *di);
static void dma64_txsuspend(dma_info_t *di);
static void dma64_txresume(dma_info_t *di);
static bool dma64_txsuspended(dma_info_t *di);
static void * dma64_txreclaim(dma_info_t *di, bool forceall);
static bool dma64_txstopped(dma_info_t *di);
static bool dma64_rxstopped(dma_info_t *di);
static bool dma64_rxenabled(dma_info_t *di);
static bool _dma64_addrext(void *osh, dma64regs_t *dma64regs);


#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static char* dma64_dumpring(dma_info_t *di, char *b, 
                dma64dd_t *ring, uint start,
                uint end, uint max_num);

static char* dma64_dump(dma_info_t *di, char *b, bool dumpring);
static char* dma64_dumptx(dma_info_t *di, char *b, bool dumpring);
static char* dma64_dumprx(dma_info_t *di, char *b, bool dumpring);

#endif


static di_fcn_t dma64proc = {
    (di_detach_t) _dma_detach,
    (di_txinit_t) dma64_txinit,
    (di_txreset_t) dma64_txreset,
    (di_txenabled_t) dma64_txenabled,
    (di_txsuspend_t) dma64_txsuspend,
    (di_txresume_t) dma64_txresume,
    (di_txsuspended_t) dma64_txsuspended,
    (di_txsuspendedidle_t) dma64_txsuspendedidle,
    (di_txfast_t) dma64_txfast,
    (di_txstopped_t) dma64_txstopped,
    (di_txreclaim_t) dma64_txreclaim,
    (di_getnexttxp_t) dma64_getnexttxp,
    (di_peeknexttxp_t) _dma_peeknexttxp,
    (di_txblock_t) _dma_txblock,
    (di_txunblock_t) _dma_txunblock,
    (di_txactive_t) _dma_txactive,
    (di_txrotate_t) dma64_txrotate,

    (di_rxinit_t) _dma_rxinit,
    (di_rxreset_t) dma64_rxreset,
    (di_rxidle_t) dma64_rxidle,
    (di_rxstopped_t) dma64_rxstopped,
    (di_rxenable_t) _dma_rxenable,
    (di_rxenabled_t) dma64_rxenabled,
    (di_rx_t) _dma_rx,
    (di_rxfill_t) _dma_rxfill,
    (di_rxreclaim_t) _dma_rxreclaim,
    (di_getnextrxp_t) _dma_getnextrxp,

    (di_fifoloopbackenable_t) _dma_fifoloopbackenable,
    (di_getvar_t) _dma_getvar,
    (di_counterreset_t) _dma_counterreset,
#if defined(BCMDBG)
    (di_dump_t) dma64_dump,
    (di_dumptx_t) dma64_dumptx,
    (di_dumprx_t) dma64_dumprx,
#else
    NULL,
    NULL,
    NULL,    
#endif
    (di_rxactive_t) _dma_rxactive,
    (di_txactive_t) _dma_txpending,
    NULL,
    36
};
#define SI_SDRAM_SWAPPED    0x10000000


hnddma_t *dma_attach(void *osh, char *name, uint dma_id, void *dmaregstx,
		     void *dmaregsrx, uint ntxd, uint nrxd, uint rxbufsize,
		     uint nrxpost, uint rxoffset, uint *msg_level)
{
    dma_info_t *di;
    uint size;
 
    /* allocate private info structure */
    if ((di = MALLOC(osh, sizeof(dma_info_t))) == NULL) {
    	DMA_ERROR(("dma_attach: out of memory\n"));
        return (NULL);
    }
    bzero((char *) di, sizeof(dma_info_t));

    di->msg_level = msg_level ? msg_level : &dma_msg_level;

    di->dma_id = dma_id;
    di->dma64 = TRUE;

    /* check arguments */
    ASSERT(ISPOWEROF2(ntxd));
    ASSERT(ISPOWEROF2(nrxd));
    if (nrxd == 0)
        ASSERT(dmaregsrx == NULL);
    if (ntxd == 0)
        ASSERT(dmaregstx == NULL);


    /* init dma reg pointer */
    ASSERT(ntxd <= D64MAXDD);
    ASSERT(nrxd <= D64MAXDD);
    di->d64txregs = (dma64regs_t *) dmaregstx;
    di->d64rxregs = (dma64regs_t *) dmaregsrx;

    di->dma64align = D64RINGALIGN;
    if ((ntxd < D64MAXDD / 2) && (nrxd < D64MAXDD / 2)) {
        /* for smaller dd table, HW relax the alignment requirement */
        di->dma64align = D64RINGALIGN / 2;
    }

#if defined(IPROC_CMICD)
    /* align the descriptor table on 8KB boundary */
    di->dma64align = D64RINGALIGN;
#endif

    DMA_TRACE(("%s: dma_attach: %s osh %p" 
        "ntxd %d nrxd %d rxbufsize %d nrxpost %d " 
        "rxoffset %d dmaregstx %p dmaregsrx %p\n", 
        name, (di->dma64 ? "DMA64" : "DMA32"), 
        osh, ntxd, nrxd, rxbufsize, nrxpost, 
        rxoffset, dmaregstx, dmaregsrx));

    /* make a private copy of our callers name */
    strncpy(di->name, name, MAXNAMEL);
    di->name[MAXNAMEL - 1] = '\0';

    di->osh = osh;


    /* save tunables */
    di->ntxd = ntxd;
    di->nrxd = nrxd;

    /* the actual dma size doesn't include the extra headroom */
    if (rxbufsize > BCMEXTRAHDROOM)
        di->rxbufsize = rxbufsize - BCMEXTRAHDROOM;
    else
        di->rxbufsize = rxbufsize;

    di->nrxpost = nrxpost;
    di->rxoffset = rxoffset;
    di->ddoffsetlow = 0;
    di->dataoffsetlow = 0;

#if defined(__mips__) && defined(BE_HOST)

        /* use sdram swapped region for data buffers but not dma descriptors.
         * XXX this assumes that we are running on a 47xx mips with a swap window.
         * But __mips__ is too general, there should be one si_ishndmips() checking
         * for OUR mips
         */
    di->ddoffsetlow = di->ddoffsetlow + SI_SDRAM_SWAPPED;
#endif

    di->addrext = _dma_isaddrext(di);

    /* allocate tx packet pointer vector */
    if (ntxd) {
        size = ntxd * sizeof(void *);
        if ((di->txp = MALLOC(osh, size)) == NULL) {
	    DMA_ERROR(("%s: dma_attach: out of tx memory\n", di->name));
            goto fail;
        }
	bzero((char *) di->txp, size);
    }

    /* allocate rx packet pointer vector */
    if (nrxd) {
        size = nrxd * sizeof(void *);
        if ((di->rxp = MALLOC(osh, size)) == NULL) {
	    DMA_ERROR(("%s: dma_attach: out of rx memory\n", di->name));
            goto fail;
        }
	bzero((char *) di->rxp, size);
    }

    /* allocate transmit descriptor ring, only need ntxd descriptors but it must be aligned */
    if (ntxd) {
        if (!_dma_alloc(di, DMA_TX)){
            goto fail;
        }
        size = ntxd * sizeof(int);
        if ((di->txd64priv = MALLOC(osh, size)) == NULL) {
	        DMA_ERROR(("%s: dma_attach: out of txd64priv memory\n", di->name));
            goto fail;
        }
        bzero((char *) di->txd64priv, size);

    }

    /* allocate receive descriptor ring, only need nrxd descriptors but it must be aligned */
    if (nrxd) {
        if (!_dma_alloc(di, DMA_RX)){
            goto fail;
        }
        
    }
    DMA_TRACE(("ddoffsetlow 0x%x ddoffsethigh 0x%x" 
        "dataoffsetlow 0x%x dataoffsethigh 0x%x addrext %d\n", 
        di->ddoffsetlow, di->ddoffsethigh, di->dataoffsetlow, 
        di->dataoffsethigh, di->addrext));


    /* initialize opsvec of function pointers */

    memcpy(&di->hnddma.di_fn, &dma64proc, sizeof(di_fcn_t));

    return ((hnddma_t *) di);

  fail:
    _dma_detach(di);
    return (NULL);
}


/* init the tx or rx descriptor
 * XXX - how to handle native 64 bits addressing AND bit64 extension
 */
static INLINE void
dma64_dd_upd(dma_info_t * di, dma64dd_t * ddring, ulong pa, uint outidx,
	     uint32 * flags, uint32 bufcount)
{
    uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;
    W_SM(&ddring[outidx].addrlow, BUS_SWAP32(pa + di->dataoffsetlow));
    W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(0 + di->dataoffsethigh));
    W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
    W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
 
}


static bool _dma_alloc(dma_info_t * di, uint direction)
{
    return dma64_alloc(di, direction);
}

/* !! may be called with core in reset */
static void _dma_detach(dma_info_t *di)
{
    if (di == NULL)
        return;

    DMA_TRACE(("%s: dma_detach\n", di->name));

    /* shouldn't be here if descriptors are unreclaimed */
    ASSERT(di->txin == di->txout);
    ASSERT(di->rxin == di->rxout);

    /* free dma descriptor rings */
    if (di->txd64priv) {
        MFREE(di->osh, (void *) di->txd64priv, (di->ntxd * sizeof(int)));
    }

    if (di->txd64)
        DMA_FREE_CONSISTENT(di->osh,
			((int8 *) (uint *) di->txd64 -
			 di->txdalign), di->txdalloc,
			(di->txdpa - di->txdalign));
    if (di->rxd64)
        DMA_FREE_CONSISTENT(di->osh,
			((int8 *) (uint *) di->rxd64 -
			 di->rxdalign), di->rxdalloc,
			(di->rxdpa - di->rxdalign));

    /* free packet pointer vectors */
    if (di->txp)
    	MFREE(di->osh, (void *) di->txp, (di->ntxd * sizeof(void *)));
    if (di->rxp)
	    MFREE(di->osh, (void *) di->rxp, (di->nrxd * sizeof(void *)));


    /* free our private info structure */
    MFREE(di->osh, (void *) di, sizeof(dma_info_t));

}

/* return TRUE if this dma engine supports DmaExtendedAddrChanges, otherwise FALSE */
static bool _dma_isaddrext(dma_info_t *di)
{

    /* DMA64 supports full 32 bits or 64 bits. AE is always valid */

    /* not all tx or rx channel are available */
    if (di->d64txregs != NULL) {
        if (!_dma64_addrext(di->osh, di->d64txregs)) {
	DMA_ERROR(("%s: _dma_isaddrext: DMA64 tx doesn't have AE set\n", di->name));
            ASSERT(0);
        }
        return TRUE;
    } else if (di->d64rxregs != NULL) {
        if (!_dma64_addrext(di->osh, di->d64rxregs)) {
	DMA_ERROR(("%s: _dma_isaddrext: DMA64 rx doesn't have AE set\n", di->name));
            ASSERT(0);
        }
        return TRUE;
    }
    return FALSE;
}

/* initialize descriptor table base address */
static void _dma_ddtable_init(dma_info_t * di, uint direction, ulong pa)
{

    /* XXX - DMA64 full 64 bits address extension */


    if (direction == DMA_TX) {
        W_REG(di->osh, &di->d64txregs->addrlow,
              (pa + di->ddoffsetlow));
        W_REG(di->osh, &di->d64txregs->addrhigh, di->ddoffsethigh);
    } else {
        W_REG(di->osh, &di->d64rxregs->addrlow,
              (pa + di->ddoffsetlow));
        W_REG(di->osh, &di->d64rxregs->addrhigh, di->ddoffsethigh);
    }
}

static void _dma_fifoloopbackenable(dma_info_t * di, bool on)
{
    DMA_TRACE(("%s: dma_fifoloopbackenable, on = %d\n", di->name, on));

    if (on) {
        OR_REG(di->osh, &di->d64txregs->control, D64_XC_LE);
    } else {
        AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_LE);
    }
}

static void _dma_rxinit(dma_info_t *di)
{
    DMA_TRACE(("%s: dma_rxinit\n", di->name));

    if (di->nrxd == 0)
        return;

    di->rxin = di->rxout = 0;

    /* clear rx descriptor ring */
	BZERO_SM((void *) (uint *) di->rxd64,
		 (di->nrxd * sizeof(dma64dd_t)));

    _dma_rxenable(di);
    _dma_ddtable_init(di, DMA_RX, di->rxdpa);
}

static void _dma_rxenable(dma_info_t *di)
{
    DMA_TRACE(("%s: dma_rxenable\n", di->name));

    /* XXX: Disabling parity checks, need to write parity gen code for descriptors!! */

    W_REG(di->osh, &di->d64rxregs->control,
      ((di->rxoffset << D64_RC_RO_SHIFT) | 
      D64_RC_OC | D64_RC_PD | D64_RC_RE));
 
}

/* !! rx entry routine, returns a pointer to the next frame received,
 * or NULL if there are no more
 */
static void *_dma_rx(dma_info_t *di)
{
    void *p;
    uint len;
    int skiplen = 0;
#ifdef GMAC_ERR_PKTS_DETECT
    uint32 *magic = NULL;
#endif /* GMAC_ERR_PKTS_DETECT */    
    

    while ((p = _dma_getnextrxp(di, FALSE))) {
        /* skip giant packets which span multiple rx descriptors */
        if (skiplen > 0) {
            skiplen -= di->rxbufsize;
            if (skiplen < 0)
                skiplen = 0;
            PKTFREE(di->osh, di->dma_id, p, FALSE);
            continue;
        }


	len = htol16(*(uint16 *) (PKTDATA(di->osh, di->dma_id, p, 0))) & 0xffff;


        DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));

        /* Checking if the bit of overflow is set */ 
        {
            bcmgmacrxh_t *rxh;
            
	    rxh = (bcmgmacrxh_t *) PKTDATA(di->osh, di->dma_id, p, 0);
            if (rxh->flags & GRXF_OVF) {
                PKTFREE(di->osh, di->dma_id, p, FALSE);
                DMA_ERROR(("------free pkt in _dma_rx %d\n",__LINE__));

                continue;
            }
#ifdef GMAC_ERR_PKTS_DETECT
            magic = (uint32 *)rxh + GMAC_ERR_PKTS_MAGIC_OFFSET;
            if (((*magic) == GMAC_ERR_PKTS_MAGIC_TAG) &&
                ((*(magic + 1)) == GMAC_ERR_PKTS_MAGIC_TAG)) {
                PKTFREE(di->osh, di->dma_id, p, FALSE);
                DMA_ERROR(("------free pkt in _dma_rx %d len %d intstatus %x\n",
                    __LINE__,len,*(volatile unsigned int *)(0xb800d020)));
                continue;
            }
#endif /* GMAC_ERR_PKTS_DETECT */                            
        }

        /* bad frame length check */
        if (len > (di->rxbufsize - di->rxoffset)) {
            DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name,
                len));
            if (len > 0)
                skiplen = len - (di->rxbufsize - di->rxoffset);
            PKTFREE(di->osh, di->dma_id, p, FALSE);
            di->hnddma.rxgiants++;
            continue;
        }
        /* set actual length */
        PKTSETLEN(di->osh, di->dma_id, p, (di->rxoffset + len));
        break;
    }

    return (p);
}

/* post receive buffers */
static void _dma_rxfill(dma_info_t *di)
{
    void *p;
    uint rxin, rxout;
    uint32 flags = 0;
    uint n;
    uint i;
    uint32 pa;
    uint extra_offset = 0;
#ifdef GMAC_ERR_PKTS_DETECT
    uint32 *magic = NULL;
#endif /* GMAC_ERR_PKTS_DETECT */
    

    /*
     * Determine how many receive buffers we're lacking
     * from the full complement, allocate, initialize,
     * and post them, then update the chip rx lastdscr.
     */

    rxin = di->rxin;
    rxout = di->rxout;

    n = di->nrxpost - NRXDACTIVE(rxin, rxout);

    DMA_TRACE(("%s: dma_rxfill: post %d\n", di->name, n));

    if (di->rxbufsize > BCMEXTRAHDROOM)
        extra_offset = BCMEXTRAHDROOM;

    for (i = 0; i < n; i++) {
        /* the di->rxbufsize doesn't include the extra headroom, we need to add it to the
           size to be allocated
        */

        if ((p = PKTGET(di->osh, di->dma_id, di->rxbufsize + extra_offset,
            FALSE)) == NULL) {
            DMA_ERROR(("%s: dma_rxfill: out of rxbufs\n", di->name));
            di->hnddma.rxnobuf++;
            break;
        }


        /* PR3263 & PR3387 & PR4642 war: rxh.len=0 means dma writes not complete */
        /* Do a cached write instead of uncached write since DMA_MAP
        * will flush the cache.
        */
        *(uint16 *) (PKTDATA(di->osh, di->dma_id, p, 0)) = 0;
#ifdef GMAC_ERR_PKTS_DETECT
        /* Add the magic words to packet buffer */
        magic = (uint32*)(PKTDATA(di->osh, di->dma_id, p, 0)) + 
            GMAC_ERR_PKTS_MAGIC_OFFSET;
        *(magic) = GMAC_ERR_PKTS_MAGIC_TAG;
        *(magic + 1) = GMAC_ERR_PKTS_MAGIC_TAG;
#endif /* GMAC_ERR_PKTS_DETECT */            


        pa = (uint32) DMA_MAP(di->osh, di->dma_id, 
                PKTDATA(di->osh, di->dma_id, p, 0),
                di->rxbufsize, DMA_RX, p);

 
        /* save the free packet pointer */
        ASSERT(di->rxp[rxout] == NULL);
        di->rxp[rxout] = p;

        /* reset flags for each descriptor */
        flags = 0;

        if (rxout == (di->nrxd - 1)) {
            flags = D64_CTRL1_EOT;
        }
 
        dma64_dd_upd(di, di->rxd64, pa, rxout, &flags,
            di->rxbufsize);
        rxout = NEXTRXD(rxout);

    }

    di->rxout = rxout;

    /* update the chip lastdscr pointer */

    W_REG(di->osh, &di->d64rxregs->ptr, I2B(rxout, dma64dd_t));
}

/* like getnexttxp but no reclaim */
static void *_dma_peeknexttxp(dma_info_t *di)
{
    uint end, i;

    if (di->ntxd == 0)
	return NULL;


	end =
	    B2I(R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK,
		dma64dd_t);

    for (i = di->txin; i != end; i = NEXTTXD(i))
        if (di->txp[i])
            return (di->txp[i]);

    return (NULL);
}

static void _dma_rxreclaim(dma_info_t *di)
{
    void *p;

    /* "unused local" warning suppression for OSLs that
     * define PKTFREE() without using the di->osh arg
     */
    di = di;

    DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

    while ((p = _dma_getnextrxp(di, TRUE))){
        PKTFREE(di->osh, di->dma_id, p, FALSE);
    }
}

static void *_dma_getnextrxp(dma_info_t *di, bool forceall)
{
    if (di->nrxd == 0)
        return (NULL);
    return dma64_getnextrxp(di, forceall);

}

static void _dma_txblock(dma_info_t *di)
{
    di->hnddma.txavail = 0;
}

static void _dma_txunblock(dma_info_t *di)
{
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
}

static uint _dma_txactive(dma_info_t *di)
{
    return (NTXDACTIVE(di->txin, di->txout));
}

static uint _dma_txpending(dma_info_t *di)
{
    uint curr;

	curr =
	    B2I(R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK,
		dma64dd_t);
    return (NTXDACTIVE(curr, di->txout));
}

static uint _dma_rxactive(dma_info_t *di)
{
    return (NRXDACTIVE(di->rxin, di->rxout));
}

static void _dma_counterreset(dma_info_t *di)
{
    /* reset all software counter */
    di->hnddma.rxgiants = 0;
    di->hnddma.rxnobuf = 0;
    di->hnddma.txnobuf = 0;
}

/* get the address of the var in order to change later */
static uint * _dma_getvar(dma_info_t * di, const char *name)
{
    if (!strcmp(name, "&txavail"))
	return ((uint *) & (di->hnddma.txavail));
    else {
        ASSERT(0);
    }
    return (0);
}


#if defined(BCMDBG)

static char*
dma64_dumpring(dma_info_t *di, char *buf, dma64dd_t *ring,
	       uint start, uint end, uint max_num)
{
    uint i;

    for (i = start; i != end; i = XXD((i + 1), max_num)) {
        /* in the format of high->low 16 bytes */
        buf += sprintf(buf, "ring index %d: 0x%x %x %x %x\n",
            i, ring[i].addrhigh, ring[i].addrlow, ring[i].ctrl2, 
            ring[i].ctrl1);
    }
    return buf;

}

static char*
dma64_dumptx(dma_info_t *di, char *buf, bool dumpring)
{
    if (di->ntxd == 0)
        return buf;

    buf += sprintf(buf, "%s: txd64 %p txdpa 0x%lx txp %p txin %d txout %d "
               "txavail %d\n",di->name, di->txd64, di->txdpa, di->txp, di->txin,
               di->txout, di->hnddma.txavail);

    buf += sprintf(buf, "/t xmtcontrol 0x%x xmtaddrlow 0x%x xmtaddrhigh 0x%x "
               "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
               R_REG(di->dev, &di->d64txregs->control),
               R_REG(di->dev, &di->d64txregs->addrlow),
               R_REG(di->dev, &di->d64txregs->addrhigh),
               R_REG(di->dev, &di->d64txregs->ptr),
               R_REG(di->dev, &di->d64txregs->status0),
               R_REG(di->dev, &di->d64txregs->status1));

	if (dumpring && di->txd64) {
        dma64_dumpring(di, buf, di->txd64, di->txin, di->txout, di->ntxd);
    }
 
    return buf;
}

static char*
dma64_dumprx(dma_info_t *di, char *buf, bool dumpring)
{
    
    if (di->nrxd == 0)
        return buf;

    buf += sprintf(buf, "%s: rxd64 %p rxdpa 0x%lx rxp %p rxin %d rxout %d\n",
         di->name, di->rxd64, di->rxdpa, di->rxp, di->rxin, di->rxout);
    
    buf += sprintf(buf, "/t rcvcontrol 0x%x rcvaddrlow 0x%x rcvaddrhigh 0x%x rcvptr "
            "0x%x rcvstatus0 0x%x rcvstatus1 0x%x\n",
            R_REG(di->dev, &di->d64rxregs->control),
            R_REG(di->dev, &di->d64rxregs->addrlow),
            R_REG(di->dev, &di->d64rxregs->addrhigh),
            R_REG(di->dev, &di->d64rxregs->ptr),
            R_REG(di->dev, &di->d64rxregs->status0),
            R_REG(di->dev, &di->d64rxregs->status1));


     if (di->rxd64 && dumpring) {        
        dma64_dumpring(di, buf, di->rxd64, di->rxin, di->rxout, di->nrxd);
     }

     return buf;

}

static char*
dma64_dump(dma_info_t *di, char *b, bool dumpring)
{

    b = dma64_dumptx(di, b, dumpring);
    b = dma64_dumprx(di, b, dumpring);
     

    return b;
}

#endif  /* BCMDBG  */


/* 64 bits DMA functions */

static void dma64_txinit(dma_info_t *di)
{
    DMA_TRACE(("%s: dma_txinit\n", di->name));

    if (di->ntxd == 0)
        return;

    di->txin = di->txout = 0;
    di->hnddma.txavail = di->ntxd - 1;

    /* clear tx descriptor ring */
    BZERO_SM((void *) (uint *) di->txd64, (di->ntxd * sizeof(dma64dd_t)));
    /* XXX: Disabling parity checks, write parity gen code. */
    W_REG(di->osh, &di->d64txregs->control, D64_XC_PD | D64_XC_XE);
    _dma_ddtable_init(di, DMA_TX, di->txdpa);
}

static bool dma64_txenabled(dma_info_t *di)
{
    uint32 xc;

    /* If the chip is dead, it is not enabled :-) */
    xc = R_REG(di->osh, &di->d64txregs->control);
    return ((xc != 0xffffffff) && (xc & D64_XC_XE));
}

static void dma64_txsuspend(dma_info_t *di)
{
    DMA_TRACE(("%s: dma_txsuspend\n", di->name));

    if (di->ntxd == 0)
        return;

    OR_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
}

static void dma64_txresume(dma_info_t *di)
{
    DMA_TRACE(("%s: dma_txresume\n", di->name));

    if (di->ntxd == 0)
        return;

    AND_REG(di->osh, &di->d64txregs->control, ~D64_XC_SE);
}

static bool dma64_txsuspended(dma_info_t *di)
{
    return (di->ntxd == 0)
	|| ((R_REG(di->osh, &di->d64txregs->control) & D64_XC_SE)
            == D64_XC_SE);
}

static void* dma64_txreclaim(dma_info_t *di, bool forceall)
{
    void *p, *return_p = NULL;

    DMA_TRACE(("%s: dma_txreclaim %s\n", di->name, forceall ? "all" : ""));
    
    while ((p = dma64_getnexttxp(di, forceall))) {
        PKTFREE(di->osh, di->dma_id, p, di->apitx);
        if (di->apitx) {
            return_p = p;
        }
        
    }

    return return_p;
}

static bool dma64_txstopped(dma_info_t *di)
{
    return ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) ==
	    D64_XS0_XS_STOPPED);
}

static bool dma64_rxstopped(dma_info_t *di)
{
    return ((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK) ==
	    D64_RS0_RS_STOPPED);
}

static bool dma64_alloc(dma_info_t *di, uint direction)
{
    uint size;
    uint ddlen;
    uint32 alignbytes;
    void *va;
    ulong dpa;

    ddlen = sizeof(dma64dd_t);

    size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);

    alignbytes = di->dma64align;

    if (!ISALIGNED(DMA_CONSISTENT_ALIGN, alignbytes))
        size += alignbytes;

    if (direction == DMA_TX) {
        if ((va =
            DMA_ALLOC_CONSISTENT(di->osh, size, &di->txdpa))== NULL) {
            DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(ntxd) failed\n", di->name));
            return FALSE;
        }
        dpa = ROUNDUP(di->txdpa, alignbytes);
        di->txdalign = dpa - di->txdpa;
        di->txd64 = (dma64dd_t *)va + di->txdalign;
        di->txdalloc = size;
        ASSERT(ISALIGNED((uint *) di->txd64, alignbytes));
    } else {
        if ((va = DMA_ALLOC_CONSISTENT(di->osh, size, &di->rxdpa)) == NULL) {

            DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(nrxd) failed\n", di->name));
            return FALSE;
        }
        dpa = ROUNDUP(di->rxdpa, alignbytes);
        di->rxdalign = dpa - di->rxdpa;
        di->rxd64 = (dma64dd_t *)va + di->rxdalign;
        di->rxdalloc = size;
        ASSERT(ISALIGNED((uint *) di->rxd64, alignbytes));
    }

    return TRUE;
}

static bool dma64_txreset(dma_info_t *di)
{
    uint32 status;

    if (di->ntxd == 0)
        return TRUE;

    /* address PR8249/PR7577 issue */
    /* suspend tx DMA first */
    W_REG(di->osh, &di->d64txregs->control, D64_XC_SE);
    SPINWAIT(((status =
	       (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK))
	      != D64_XS0_XS_DISABLED) && (status != D64_XS0_XS_IDLE)
	     && (status != D64_XS0_XS_STOPPED), 10000);

    /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
    W_REG(di->osh, &di->d64txregs->control, 0);
    SPINWAIT(((status =
	       (R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK))
	      != D64_XS0_XS_DISABLED), 10000);

    /* wait for the last transaction to complete */
    OSL_DELAY(300);

    return (status == D64_XS0_XS_DISABLED);
}

static bool dma64_rxidle(dma_info_t *di)
{
    DMA_TRACE(("%s: dma_rxidle\n", di->name));

    if (di->nrxd == 0)
        return TRUE;

    return ((R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK) ==
        R_REG(di->osh, &di->d64rxregs->ptr));
}

static bool dma64_rxreset(dma_info_t *di)
{
    uint32 status;

    if (di->nrxd == 0)
        return TRUE;

    /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
    W_REG(di->osh, &di->d64rxregs->control, 0);
    SPINWAIT(((status =
	       (R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK))
	      != D64_RS0_RS_DISABLED), 10000);
 
    return (status == D64_RS0_RS_DISABLED);
}

static bool dma64_rxenabled(dma_info_t *di)
{
    uint32 rc;

    rc = R_REG(di->osh, &di->d64rxregs->control);
    return ((rc != 0xffffffff) && (rc & D64_RC_RE));
}

static bool dma64_txsuspendedidle(dma_info_t *di)
{
    /* XXX - verify PR20059 has been fixed */

    if (di->ntxd == 0)
        return TRUE;

    if (!(R_REG(di->osh, &di->d64txregs->control) & D64_XC_SE))
        return 0;

    if ((R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK) ==
	D64_XS0_XS_IDLE)
        return 1;

    return 0;
}


/* !! tx entry routine */
static int dma64_txfast(dma_info_t *di, void *p0, bool commit)
{
    void *p, *next;
    uchar *data;
    uint len;
    uint txout;
    uint32 flags = 0;
    uint32 pa;

    DMA_TRACE(("%s: dma_txfast\n", di->name));

    txout = di->txout;

    /*
     * Walk the chain of packet buffers
     * allocating and initializing transmit descriptor entries.
     */
    for (p = p0; p; p = next) {
        data = PKTDATA(di->osh, di->dma_id, p, 1);

        len = PKTLEN(di->osh, di->dma_id, p, 1);

        next = PKTNEXT(di->osh, di->dma_id, p, 1);

        /* return nonzero if out of tx descriptors */
        if (NEXTTXD(txout) == di->txin)
            goto outoftxd;

        /* PR988 - skip zero length buffers */
        if (len == 0)
            continue;

        /* get physical address of buffer start */
	    pa = (uint32) DMA_MAP(di->osh, di->dma_id, data, len, DMA_TX, p);
       
        if (PKTFLAGTS(di->osh, p)) {
            flags = D64_CTRL1_TS;
        } else {
            flags = 0;
        }
        if (p == p0)
            flags |= D64_CTRL1_SOF;
        if (next == NULL)
            flags |= (D64_CTRL1_IOC | D64_CTRL1_EOF);
        if (txout == (di->ntxd - 1))
            flags |= D64_CTRL1_EOT;
        DMA_TRACE(("%s: txout %d len %d flags %x next %p\n",
            di->name,txout,len, flags, next));
 
        dma64_dd_upd(di, di->txd64, pa, txout, &flags, len);
        ASSERT(di->txp[txout] == NULL);

        di->txd64priv[txout] = PKTFROM(di->osh, di->dma_id);

        txout = NEXTTXD(txout);
    }

    /* if last txd eof not set, fix it */
    if (!(flags & D64_CTRL1_EOF))
        W_SM(&di->txd64[PREVTXD(txout)].ctrl1,
             BUS_SWAP32(flags | D64_CTRL1_IOC | D64_CTRL1_EOF));


    /* save the packet */
    di->txp[PREVTXD(txout)] = p0;

    /* bump the tx descriptor index */
    di->txout = txout;

    /* kick the chip */
    if (commit)
        W_REG(di->osh, &di->d64txregs->ptr, I2B(txout, dma64dd_t));

    /* tx flow control */
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

    return 0;

  outoftxd:
    DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
    PKTFREE(di->osh, di->dma_id, p0, TRUE);
    di->hnddma.txavail = 0;
    di->hnddma.txnobuf++;
    return -1;
}

/*
 * Reclaim next completed txd (txds if using chained buffers) and
 * return associated packet.
 * If 'force' is true, reclaim txd(s) and return associated packet
 * regardless of the value of the hardware "curr" pointer.
 */
static void *dma64_getnexttxp(dma_info_t *di, bool forceall)
{
    uint start, end, i;
    void *txp;

    DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name,
	       forceall ? "all" : ""));

    if (di->ntxd == 0)
	return NULL;

    txp = NULL;

    start = di->txin;
    if (forceall)
        end = di->txout;
    else
	end =
	    B2I(R_REG(di->osh, &di->d64txregs->status0) & D64_XS0_CD_MASK,
		dma64dd_t);

    /* PR4738 - xmt disable/re-enable does not clear CURR */
    if ((start == 0) && (end > di->txout))
        goto bogus;

    for (i = start; i != end && !txp; i = NEXTTXD(i)) {
        /* XXX - dma 64 bits */
	    DMA_UNMAP(di->osh, di->dma_id,
		  (BUS_SWAP32(R_SM(&di->txd64[i].addrlow)) -
		   di->dataoffsetlow),
		  (BUS_SWAP32(R_SM(&di->txd64[i].ctrl2)) &
		   D64_CTRL2_BC_MASK), DMA_TX, di->txp[i]);

        W_SM(&di->txd64[i].addrlow, 0xdeadbeef);
        W_SM(&di->txd64[i].addrhigh, 0xdeadbeef);

        txp = di->txp[i];
        di->txp[i] = NULL;

        if (di->txd64priv[i]) {
            di->apitx = 1;
        } else {
            di->apitx = 0;
        }

    }

    di->txin = i;

    /* tx flow control */
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;       

    return txp;

  bogus:
/*
    DMA_ERROR(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
        start, end, di->txout, forceall));
*/
    return NULL;
}

static void *dma64_getnextrxp(dma_info_t *di, bool forceall)
{
    uint i;
    void *rxp;

    /* if forcing, dma engine must be disabled */
    ASSERT(!forceall || !dma64_rxenabled(di));

    i = di->rxin;

    /* return if no packets posted */
    if (i == di->rxout)
        return (NULL);

    /* ignore curr if forceall */
    if (!forceall &&
	(i ==
	 B2I(R_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_CD_MASK,
	     dma64dd_t)))
        return (NULL);

    /* get the packet pointer that corresponds to the rx descriptor */
    rxp = di->rxp[i];
    ASSERT(rxp);
    di->rxp[i] = NULL;

    /* clear this packet from the descriptor ring */
    DMA_UNMAP(di->osh, di->dma_id,
	      (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) -
	       di->dataoffsetlow), di->rxbufsize, DMA_RX, rxp);

    W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
    W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);

    di->rxin = NEXTRXD(i);

    return (rxp);
}

static bool _dma64_addrext(void *osh, dma64regs_t *dma64regs)
{
    uint32 w;
    OR_REG(osh, &dma64regs->control, D64_XC_AE);
    w = R_REG(osh, &dma64regs->control);
    AND_REG(osh, &dma64regs->control, ~D64_XC_AE);
    return ((w & D64_XC_AE) == D64_XC_AE);
}

/*
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
static void dma64_txrotate(dma_info_t *di)
{
    uint ad;
    uint nactive;
    uint rot;
    uint old, new;
    uint32 w;
    uint first, last;

    ASSERT(dma64_txsuspendedidle(di));

    nactive = _dma_txactive(di);
    ad = B2I((R_REG(di->osh, &di->d64txregs->status1) & D64_XS1_AD_MASK),
	     dma64dd_t);
    rot = TXD(ad - di->txin);

    ASSERT(rot < di->ntxd);

    /* full-ring case is a lot harder - don't worry about this */
    if (rot >= (di->ntxd - nactive)) {
        DMA_ERROR(("%s: dma_txrotate: ring full - punt\n", di->name));
        return;
    }

    first = di->txin;
    last = PREVTXD(di->txout);

    /* move entries starting at last and moving backwards to first */
    for (old = last; old != PREVTXD(first); old = PREVTXD(old)) {
        new = TXD(old + rot);

        /*
         * Move the tx dma descriptor.
         * EOT is set only in the last entry in the ring.
         */
        w = BUS_SWAP32(R_SM(&di->txd64[old].ctrl1)) & ~D64_CTRL1_EOT;
        if (new == (di->ntxd - 1))
            w |= D64_CTRL1_EOT;
        W_SM(&di->txd64[new].ctrl1, BUS_SWAP32(w));

        w = BUS_SWAP32(R_SM(&di->txd64[old].ctrl2));
        W_SM(&di->txd64[new].ctrl2, BUS_SWAP32(w));

        W_SM(&di->txd64[new].addrlow, R_SM(&di->txd64[old].addrlow));
        W_SM(&di->txd64[new].addrhigh, R_SM(&di->txd64[old].addrhigh));

        /* zap the old tx dma descriptor address field */
        W_SM(&di->txd64[old].addrlow, BUS_SWAP32(0xdeadbeef));
        W_SM(&di->txd64[old].addrhigh, BUS_SWAP32(0xdeadbeef));

        /* move the corresponding txp[] entry */
        ASSERT(di->txp[new] == NULL);
        di->txp[new] = di->txp[old];
        di->txp[old] = NULL;
    }

    /* update txin and txout */
    di->txin = ad;
    di->txout = TXD(di->txout + rot);
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

    /* kick the chip */
    W_REG(di->osh, &di->d64txregs->ptr, I2B(di->txout, dma64dd_t));
}



