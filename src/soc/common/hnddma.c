/*
 * $Id: hnddma.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Generic Broadcom Home Networking Division (HND) DMA module.
 * This supports the following chips: BCM42xx, 44xx, 47xx .
 *
 * Note: This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 */

#include <shared/et/typedefs.h>
#include <shared/et/osl.h>
#include <shared/et/bcmendian.h>
#include <shared/et/sbconfig.h>
#include <shared/et/bcmutils.h>
#include <shared/et/aiutils.h>
#include <shared/et/bcmgmacrxh.h>
#include <soc/drv.h>

#if defined(KEYSTONE) || defined(ROBO_4704) || defined(IPROC_CMICD)

struct dma_soc_info;	/* forward declaration */
#define di_t struct dma_soc_info
#include <soc/hnddma.h>
#include <soc/sbhnddma.h>


/* debug/trace */
#ifdef BCMDBG
#define DMA_ERROR(args) ET_ERROR(args)
#define DMA_TRACE(args) ET_TRACE(args)
#else
#define DMA_ERROR(args)
#define DMA_TRACE(args)
#endif

/* default dma message level (if input msg_level pointer is null in dma_attach()) */
static uint dma_soc_msg_level =
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

#define DI_INFO(dmah)   (dma_soc_info_t *)dmah

/* dma engine software state */
typedef struct dma_soc_info {
    struct hnddma_pub hnddma;   /* exported structure, don't use hnddma_t,
                     * which could be const
                     */
    uint        *msg_level; /* message level pointer */
    char        name[MAXNAMEL]; /* callers name for diag msgs */

    void		*drv;		/* driver handle */
    void        *dev;       /* device handle */
    si_t        *sih;       /* sb handle */

    bool        dma64;      /* dma64 enabled */
    bool        addrext;    /* this dma engine supports DmaExtendedAddrChanges */

    dma32regs_soc_t *d32txregs; /* 32 bits dma tx engine registers */
    dma32regs_soc_t *d32rxregs; /* 32 bits dma rx engine registers */
    dma64regs_soc_t *d64txregs; /* 64 bits dma tx engine registers */
    dma64regs_soc_t *d64rxregs; /* 64 bits dma rx engine registers */

    uint32      dma64align; /* either 8k or 4k depends on number of dd */
    dma32dd_soc_t   *txd32;     /* pointer to dma32 tx descriptor ring */
    dma64dd_soc_t   *txd64;     /* pointer to dma64 tx descriptor ring */
    uint        ntxd;       /* # tx descriptors tunable */
    uint        txin;       /* index of next descriptor to reclaim */
    uint        txout;      /* index of next descriptor to post */
    void        **txp;      /* pointer to parallel array of pointers to packets */
    ulong       txdpa;      /* physical address of descriptor ring */
    uint        txdalign;   /* #bytes added to alloc'd mem to align txd */
    uint        txdalloc;   /* #bytes allocated for the ring */

    dma32dd_soc_t   *rxd32;     /* pointer to dma32 rx descriptor ring */
    dma64dd_soc_t   *rxd64;     /* pointer to dma64 rx descriptor ring */
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
    uint    pkt_mem; /* The memory type of packet buffer */
    uint    en_rxsephdr; /* RX separate header feature */
    uint    pkthdr_mem; /* The memory type of packet header buffer */
    uint    rxsephdrsize;
    uint        ddoffsetlow;    /* add to get dma address of descriptor ring, low 32 bits */
    uint        ddoffsethigh;   /*   high 32 bits */
    uint        dataoffsetlow;  /* add to get dma address of data buffer, low 32 bits */
    uint        dataoffsethigh; /*   high 32 bits */

    int     dma_id;    
} dma_soc_info_t;

#define DMA64_ENAB(di)  ((di)->dma64)

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
static bool _dma_isaddrext(dma_soc_info_t *di);
static bool _dma_alloc(dma_soc_info_t *di, uint direction);
static void _dma_detach(dma_soc_info_t *di);
static void _dma_ddtable_init(dma_soc_info_t *di, uint direction, ulong pa);
static void _dma_rxinit(dma_soc_info_t *di);
static void *_dma_rx(dma_soc_info_t *di);
static void _dma_rxfill(dma_soc_info_t *di);
static void _dma_rxreclaim(dma_soc_info_t *di);
static void _dma_rxenable(dma_soc_info_t *di);
static void * _dma_getnextrxp(dma_soc_info_t *di, bool forceall);

static void _dma_txblock(dma_soc_info_t *di);
static void _dma_txunblock(dma_soc_info_t *di);
static uint _dma_txactive(dma_soc_info_t *di);
static uint _dma_rxactive(dma_soc_info_t *di);
static uint _dma_txpending(dma_soc_info_t *di);

static void* _dma_peeknexttxp(dma_soc_info_t *di);
static uint* _dma_getvar(dma_soc_info_t *di, const char *name);
static void _dma_counterreset(dma_soc_info_t *di);
static void _dma_fifoloopbackenable(dma_soc_info_t *di, bool on);

/* ** 32 bit DMA prototypes */
static bool dma32_alloc(dma_soc_info_t *di, uint direction);
static bool dma32_txreset(dma_soc_info_t *di);
static bool dma32_rxreset(dma_soc_info_t *di);
static bool dma32_txsuspendedidle(dma_soc_info_t *di);
static int  dma32_txfast(dma_soc_info_t *di, void *p0, bool commit);
static void *dma32_getnexttxp(dma_soc_info_t *di, bool forceall);
static void *dma32_getnextrxp(dma_soc_info_t *di, bool forceall);
static void dma32_txrotate(dma_soc_info_t *di);
static bool dma32_rxidle(dma_soc_info_t *di);
static void dma32_txinit(dma_soc_info_t *di);
static bool dma32_txenabled(dma_soc_info_t *di);
static void dma32_txsuspend(dma_soc_info_t *di);
static void dma32_txresume(dma_soc_info_t *di);
static bool dma32_txsuspended(dma_soc_info_t *di);
static void dma32_txreclaim(dma_soc_info_t *di, bool forceall);
static bool dma32_txstopped(dma_soc_info_t *di);
static bool dma32_rxstopped(dma_soc_info_t *di);
static bool dma32_rxenabled(dma_soc_info_t *di);
static bool _dma32_addrext(dma_soc_info_t *di, dma32regs_soc_t *dma32regs);
static void dma32_rxrecycle(dma_soc_info_t *di);


/* ** 64 bit DMA prototypes and stubs */
static bool dma64_alloc(dma_soc_info_t *di, uint direction);
static bool dma64_txreset(dma_soc_info_t *di);
static bool dma64_rxreset(dma_soc_info_t *di);
static bool dma64_txsuspendedidle(dma_soc_info_t *di);
static int  dma64_txfast(dma_soc_info_t *di, void *p0, bool commit);
static void *dma64_getnexttxp(dma_soc_info_t *di, bool forceall);
static void *dma64_getnextrxp(dma_soc_info_t *di, bool forceall);
static void dma64_txrotate(dma_soc_info_t *di);

static bool dma64_rxidle(dma_soc_info_t *di);
static void dma64_txinit(dma_soc_info_t *di);
static bool dma64_txenabled(dma_soc_info_t *di);
static void dma64_txsuspend(dma_soc_info_t *di);
static void dma64_txresume(dma_soc_info_t *di);
static bool dma64_txsuspended(dma_soc_info_t *di);
static void dma64_txreclaim(dma_soc_info_t *di, bool forceall);
static bool dma64_txstopped(dma_soc_info_t *di);
static bool dma64_rxstopped(dma_soc_info_t *di);
static bool dma64_rxenabled(dma_soc_info_t *di);
static bool _dma64_addrext(dma_soc_info_t *di, dma64regs_soc_t *dma64regs);
static void dma64_rxsephdrctrl(dma_soc_info_t *di, bool enable, uint memtype, uint size);


static char* dma32_dumpring(dma_soc_info_t *di, char *b, dma32dd_soc_t *ring, uint start,
                            uint end, uint max_num);
static char* dma32_dump(dma_soc_info_t *di, char *b, bool dumpring);
static char* dma32_dumptx(dma_soc_info_t *di, char *b, bool dumpring);
static char* dma32_dumprx(dma_soc_info_t *di, char *b, bool dumpring);

static char* dma64_dumpring(dma_soc_info_t *di, char *b, dma64dd_soc_t *ring, uint start,
                            uint end, uint max_num);
static char* dma64_dump(dma_soc_info_t *di, char *b, bool dumpring);
static char* dma64_dumptx(dma_soc_info_t *di, char *b, bool dumpring);
static char* dma64_dumprx(dma_soc_info_t *di, char *b, bool dumpring);


static di_fcn_t dma64proc = {
    (di_detach_t)_dma_detach,
    (di_txinit_t)dma64_txinit,
    (di_txreset_t)dma64_txreset,
    (di_txenabled_t)dma64_txenabled,
    (di_txsuspend_t)dma64_txsuspend,
    (di_txresume_t)dma64_txresume,
    (di_txsuspended_t)dma64_txsuspended,
    (di_txsuspendedidle_t)dma64_txsuspendedidle,
    (di_txfast_t)dma64_txfast,
    (di_txstopped_t)dma64_txstopped,
    (di_txreclaim_t)dma64_txreclaim,
    (di_getnexttxp_t)dma64_getnexttxp,
    (di_peeknexttxp_t)_dma_peeknexttxp,
    (di_txblock_t)_dma_txblock,
    (di_txunblock_t)_dma_txunblock,
    (di_txactive_t)_dma_txactive,
    (di_txrotate_t)dma64_txrotate,

    (di_rxinit_t)_dma_rxinit,
    (di_rxreset_t)dma64_rxreset,
    (di_rxidle_t)dma64_rxidle,
    (di_rxstopped_t)dma64_rxstopped,
    (di_rxenable_t)_dma_rxenable,
    (di_rxenabled_t)dma64_rxenabled,
    (di_rx_t)_dma_rx,
    (di_rxfill_t)_dma_rxfill,
    NULL,
    (di_rxreclaim_t)_dma_rxreclaim,
    (di_getnextrxp_t)_dma_getnextrxp,

    (di_fifoloopbackenable_t)_dma_fifoloopbackenable,
    (di_getvar_t)_dma_getvar,
    (di_counterreset_t)_dma_counterreset,

    (di_dump_t)dma64_dump,
    (di_dumptx_t)dma64_dumptx,
    (di_dumprx_t)dma64_dumprx,
    (di_rxactive_t)_dma_rxactive,
    (di_txactive_t)_dma_txpending,
    (di_rxsephdrctrl_t)dma64_rxsephdrctrl,
    36
};

static di_fcn_t dma32proc = {
    (di_detach_t)_dma_detach,
    (di_txinit_t)dma32_txinit,
    (di_txreset_t)dma32_txreset,
    (di_txenabled_t)dma32_txenabled,
    (di_txsuspend_t)dma32_txsuspend,
    (di_txresume_t)dma32_txresume,
    (di_txsuspended_t)dma32_txsuspended,
    (di_txsuspendedidle_t)dma32_txsuspendedidle,
    (di_txfast_t)dma32_txfast,
    (di_txstopped_t)dma32_txstopped,
    (di_txreclaim_t)dma32_txreclaim,
    (di_getnexttxp_t)dma32_getnexttxp,
    (di_peeknexttxp_t)_dma_peeknexttxp,
    (di_txblock_t)_dma_txblock,
    (di_txunblock_t)_dma_txunblock,
    (di_txactive_t)_dma_txactive,
    (di_txrotate_t)dma32_txrotate,

    (di_rxinit_t)_dma_rxinit,
    (di_rxreset_t)dma32_rxreset,
    (di_rxidle_t)dma32_rxidle,
    (di_rxstopped_t)dma32_rxstopped,
    (di_rxenable_t)_dma_rxenable,
    (di_rxenabled_t)dma32_rxenabled,
    (di_rx_t)_dma_rx,
    (di_rxfill_t)_dma_rxfill,
    (di_rxrecycle_t)dma32_rxrecycle,
    (di_rxreclaim_t)_dma_rxreclaim,
    (di_getnextrxp_t)_dma_getnextrxp,

    (di_fifoloopbackenable_t)_dma_fifoloopbackenable,
    (di_getvar_t)_dma_getvar,
    (di_counterreset_t)_dma_counterreset,
    (di_dump_t)dma32_dump,
    (di_dumptx_t)dma32_dumptx,
    (di_dumprx_t)dma32_dumprx,
    (di_rxactive_t)_dma_rxactive,
    (di_txactive_t)_dma_txpending,
    NULL,
    36
};

hnddma_t *
dma_soc_attach(void *drv, char *name, int dma_id, void *dev, uint dma64, 
    void *dmaregstx, void *dmaregsrx, uint ntxd, uint nrxd, 
    uint rxbufsize, uint nrxpost, uint rxoffset, uint *msg_level,
    uint pkt_mem, uint pkthdr_mem, uint en_sephdr)
{
    dma_soc_info_t *di;
    uint size;

    /* allocate private info structure */
    if ((di = ET_MALLOC(sizeof(dma_soc_info_t))) == NULL) {
#ifdef BCMDBG
        printf("dma_attach: out of memory\n");
#endif
        return (NULL);
    }
    bzero((char *)di, sizeof(dma_soc_info_t));

    di->msg_level = msg_level ? msg_level : &dma_soc_msg_level;

    di->dma64 = dma64; 

    di->dma_id = dma_id;

    di->pkt_mem = pkt_mem;
    /* This value should bigger than the value of "rxoffset" */
    di->en_rxsephdr = en_sephdr; 
    di->rxsephdrsize = RXSEPHDRSZ;
    di->pkthdr_mem = pkthdr_mem;

    /* check arguments */
    ASSERT(ISPOWEROF2(ntxd));
    ASSERT(ISPOWEROF2(nrxd));
    if (nrxd == 0)
        ASSERT(dmaregsrx == NULL);
    if (ntxd == 0)
        ASSERT(dmaregstx == NULL);


    /* init dma reg pointer */
    if (di->dma64) {
        ASSERT(ntxd <= D64MAXDD);
        ASSERT(nrxd <= D64MAXDD);
        di->d64txregs = (dma64regs_soc_t *)dmaregstx;
        di->d64rxregs = (dma64regs_soc_t *)dmaregsrx;

        di->dma64align = D64RINGALIGN;
        if ((ntxd < D64MAXDD / 2) && (nrxd < D64MAXDD / 2)) {
            /* for smaller dd table, HW relax the alignment requirement */
            di->dma64align = D64RINGALIGN / 2;
        }
#if defined(IPROC_CMICD)
        di->dma64align = D64RINGALIGN;
#endif
    } else {
        ASSERT(ntxd <= D32MAXDD);
        ASSERT(nrxd <= D32MAXDD);
        di->d32txregs = (dma32regs_soc_t *)dmaregstx;
        di->d32rxregs = (dma32regs_soc_t *)dmaregsrx;
    }

    DMA_TRACE(("%s: dma_attach: %s drv 0x%x ntxd %d nrxd %d rxbufsize %d nrxpost %d "
               "rxoffset %d dmaregstx %p dmaregsrx %p\n",
               name, (di->dma64 ? "DMA64" : "DMA32"), (uint)drv, ntxd, nrxd, rxbufsize,
               nrxpost, rxoffset, dmaregstx, dmaregsrx));

    /* make a private copy of our callers name */
    strncpy(di->name, name, MAXNAMEL);
    di->name[MAXNAMEL-1] = '\0';

    di->drv = drv;
    di->dev = dev;

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

    /*
     * figure out the DMA physical address offset for dd and data
     *   for old chips w/o sb, use zero
     *   for new chips w sb,
     *     PCI/PCIE: they map silicon backplace address to zero based memory, need offset
     *     Other bus: use zero
     *     SI_BUS BIGENDIAN kludge: use sdram swapped region for data buffer, not descriptor
     */
    di->ddoffsetlow = 0;
    di->dataoffsetlow = 0;   

#if defined(__mips__) && defined(BE_HOST)
        /* use sdram swapped region for data buffers but not dma descriptors.
         * XXX this assumes that we are running on a 47xx mips with a swap window.
         * But __mips__ is too general, there should be one si_ishndmips() checking
         * for OUR mips
         */

    if (dma64) {
        di->ddoffsetlow = di->ddoffsetlow + SB_SDRAM_SWAPPED;
    } else {
        /* bcm5836 or bcm4704 */
        di->dataoffsetlow = di->dataoffsetlow + SB_SDRAM_SWAPPED;
    }
#endif

    di->addrext = _dma_isaddrext(di);

    /* allocate tx packet pointer vector */
    if (ntxd) {
        size = ntxd * sizeof(void *);
        if ((di->txp = ET_MALLOC(size)) == NULL) {
            DMA_ERROR(("%s: dma_attach: out of tx memory\n",
                       di->name));
            goto fail;
        }
        bzero((char *)di->txp, size);
    }

    /* allocate rx packet pointer vector */
    if (nrxd) {
        size = nrxd * sizeof(void *);
        if ((di->rxp = ET_MALLOC(size)) == NULL) {
            DMA_ERROR(("%s: dma_attach: out of rx memory\n",
                       di->name));
            goto fail;
        }
        bzero((char *)di->rxp, size);
    }

    /* allocate transmit descriptor ring, only need ntxd descriptors but it must be aligned */
    if (ntxd) {
        if (!_dma_alloc(di, DMA_TX))
            goto fail;
    }

    /* allocate receive descriptor ring, only need nrxd descriptors but it must be aligned */
    if (nrxd) {
        if (!_dma_alloc(di, DMA_RX))
            goto fail;
    }

    DMA_TRACE(("ddoffsetlow 0x%x ddoffsethigh 0x%x dataoffsetlow 0x%x dataoffsethigh "
               "0x%x addrext %d\n", di->ddoffsetlow, di->ddoffsethigh, di->dataoffsetlow,
               di->dataoffsethigh, di->addrext)); 

    /* initialize opsvec of function pointers */
    if (DMA64_ENAB(di)) {
        memcpy(&di->hnddma.di_fn, &dma64proc, sizeof(di_fcn_t));
    } else {
        memcpy(&di->hnddma.di_fn, &dma32proc, sizeof(di_fcn_t));
    }


    return ((hnddma_t *)di);

fail:
    _dma_detach(di);
    return (NULL);
}

/* init the tx or rx descriptor */
static INLINE void
dma32_dd_upd(dma_soc_info_t *di, dma32dd_soc_t *ddring, ulong pa, uint outidx, uint32 *flags,
             uint32 bufcount)
{
    /* dma32 uses 32 bits control to fit both flags and bufcounter */
    *flags = *flags | (bufcount & CTRL_BC_MASK);

    W_SM(&ddring[outidx].addr, BUS_SWAP32(pa + di->dataoffsetlow));
    W_SM(&ddring[outidx].ctrl, BUS_SWAP32(*flags));
}

/* init the tx or rx descriptor
 * XXX - how to handle native 64 bits addressing AND bit64 extension
 */
static INLINE void
dma64_dd_upd(dma_soc_info_t *di, dma64dd_soc_t *ddring, ulong pa, uint outidx, uint32 *flags,
             uint32 bufcount)
{
    uint32 ctrl2 = bufcount & D64_CTRL2_BC_MASK;

    /* PCI bus with big(>1G) physical address, use address extension */
    W_SM(&ddring[outidx].addrlow, BUS_SWAP32(pa + di->dataoffsetlow));
    W_SM(&ddring[outidx].addrhigh, BUS_SWAP32(0 + di->dataoffsethigh));
    W_SM(&ddring[outidx].ctrl1, BUS_SWAP32(*flags));
    W_SM(&ddring[outidx].ctrl2, BUS_SWAP32(ctrl2));
}

static bool
_dma32_addrext(dma_soc_info_t *di, dma32regs_soc_t *dma32regs)
{
    uint32 w;

    OR_REG(di->dev, &dma32regs->control, XC_AE);
    w = R_REG(di->dev, &dma32regs->control);
    AND_REG(di->dev, &dma32regs->control, ~XC_AE);
    return ((w & XC_AE) == XC_AE);
}

static bool
_dma_alloc(dma_soc_info_t *di, uint direction)
{
    if (DMA64_ENAB(di)) {
        return dma64_alloc(di, direction);
    } else {
        return dma32_alloc(di, direction);
    }
}

/* !! may be called with core in reset */
static void
_dma_detach(dma_soc_info_t *di)
{
    if (di == NULL)
        return;

    DMA_TRACE(("%s: dma_detach\n", di->name));

    /* shouldn't be here if descriptors are unreclaimed */
    ASSERT(di->txin == di->txout);
    ASSERT(di->rxin == di->rxout);

    /* free dma descriptor rings */
    if (DMA64_ENAB(di)) {
        if (di->txd64)
            DMA_FREE_CONSISTENT(di->dev, ((int8*)(uint *)di->txd64 - di->txdalign),
                                di->txdalloc, (di->txdpa - di->txdalign));
        if (di->rxd64)
            DMA_FREE_CONSISTENT(di->dev, ((int8*)(uint *)di->rxd64 - di->rxdalign),
                                di->rxdalloc, (di->rxdpa - di->rxdalign));
    } else {
        if (di->txd32)
            DMA_FREE_CONSISTENT(di->dev, ((int8*)(uint *)di->txd32 - di->txdalign),
                                di->txdalloc, (di->txdpa - di->txdalign));
        if (di->rxd32)
            DMA_FREE_CONSISTENT(di->dev, ((int8*)(uint *)di->rxd32 - di->rxdalign),
                                di->rxdalloc, (di->rxdpa - di->rxdalign));
    }

    /* free packet pointer vectors */
    if (di->txp)
        ET_MFREE((void *)di->txp, (di->ntxd * sizeof(void *)));
    if (di->rxp)
        ET_MFREE((void *)di->rxp, (di->nrxd * sizeof(void *)));  

    /* free our private info structure */
    ET_MFREE((void *)di, sizeof(dma_soc_info_t));

}

/* return TRUE if this dma engine supports DmaExtendedAddrChanges, otherwise FALSE */
static bool
_dma_isaddrext(dma_soc_info_t *di)
{
    if (DMA64_ENAB(di)) {
        /* DMA64 supports full 32 bits or 64 bits. AE is always valid */

        /* not all tx or rx channel are available */
        if (di->d64txregs != NULL) {
            if (!_dma64_addrext(di, di->d64txregs)) {
                DMA_ERROR(("%s: _dma_isaddrext: DMA64 tx doesn't have AE set\n",
                    di->name));
                ASSERT(0);
            }
            return TRUE;
        } else if (di->d64rxregs != NULL) {
            if (!_dma64_addrext(di, di->d64rxregs)) {
                DMA_ERROR(("%s: _dma_isaddrext: DMA64 rx doesn't have AE set\n",
                    di->name));
                ASSERT(0);
            }
            return TRUE;
        }
        return FALSE;
    } else if (di->d32txregs)
        return (_dma32_addrext(di, di->d32txregs));
    else if (di->d32rxregs)
        return (_dma32_addrext(di, di->d32rxregs));
    return FALSE;
}

/* initialize descriptor table base address */
static void
_dma_ddtable_init(dma_soc_info_t *di, uint direction, ulong pa)
{
    if (DMA64_ENAB(di)) {
        if (direction == DMA_TX) {
            W_REG(di->dev, &di->d64txregs->addrlow, (pa + di->ddoffsetlow));
            W_REG(di->dev, &di->d64txregs->addrhigh, di->ddoffsethigh);
        } else {
            W_REG(di->dev, &di->d64rxregs->addrlow, (pa + di->ddoffsetlow));
            W_REG(di->dev, &di->d64rxregs->addrhigh, di->ddoffsethigh);
        }

    } else {
        if (direction == DMA_TX) {
            W_REG(di->dev, &di->d32txregs->addr, (pa + di->ddoffsetlow));
        }
        else
            W_REG(di->dev, &di->d32rxregs->addr, (pa + di->ddoffsetlow));
    }
}

static void
_dma_fifoloopbackenable(dma_soc_info_t *di, bool on)
{
    DMA_TRACE(("%s: dma_fifoloopbackenable, on = %d\n", di->name, on));
    if (DMA64_ENAB(di))
        if (on) {
            OR_REG(di->dev, &di->d64txregs->control, D64_XC_LE);
        } else {
            AND_REG(di->dev, &di->d64txregs->control, ~D64_XC_LE);
        }
    else
        if (on) {
            OR_REG(di->dev, &di->d32txregs->control, XC_LE);
        } else {
            AND_REG(di->dev, &di->d32txregs->control, ~XC_LE);
        }
}

static void
_dma_rxinit(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_rxinit\n", di->name));

    if (di->nrxd == 0)
        return;

    di->rxin = di->rxout = 0;

    /* clear rx descriptor ring */
    if (DMA64_ENAB(di))
        BZERO_SM((void *)(uint *)di->rxd64, (di->nrxd * sizeof(dma64dd_soc_t)));
    else
        BZERO_SM((void *)(uint *)di->rxd32, (di->nrxd * sizeof(dma32dd_soc_t)));

    _dma_rxenable(di);
    _dma_ddtable_init(di, DMA_RX, di->rxdpa);
}

static void
_dma_rxenable(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_rxenable\n", di->name));

    /* XXX: Disabling parity checks, need to write parity gen code for descriptors!! */
    if (DMA64_ENAB(di)) {
    	if (di->en_rxsephdr) {     
            W_REG(di->dev, &di->d64rxregs->control,
              ((di->rxoffset << D64_RC_RO_SHIFT) | D64_RC_PD | 
              D64_RC_OC |D64_RC_RE | D64_RC_SH));
        } else {
            W_REG(di->dev, &di->d64rxregs->control,
              ((di->rxoffset << D64_RC_RO_SHIFT) | 
              D64_RC_OC | D64_RC_PD | D64_RC_RE));
		}
    } else {
        W_REG(di->dev, &di->d32rxregs->control,
              ((di->rxoffset << RC_RO_SHIFT) | RC_PD | RC_RE));
    }
}

/* !! rx entry routine, returns a pointer to the next frame received,
 * or NULL if there are no more
 */
static void *
_dma_rx(dma_soc_info_t *di)
{
    void *p;
    void *ptr_payload;
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
            ET_PKTFREE(di->dev, p, FALSE);
            continue;
        }

        /* Invalidate the cache so that the fresh hot data can be accessed */
        soc_cm_sinval((int)di->dev, ET_PKTDATA(di->dev, p), di->rxbufsize);

        len = htol32(*(uint32 *)(ET_PKTDATA(di->dev, p))) & 0xffff;
        DMA_TRACE(("%s: dma_rx len %d\n", di->name, len));
        
        /* Checking if the bit of overflow is set */ 
        {
            bcmgmacrxh_t *rxh;
            /*
             * For GMAC driver only,
             * since we enable the overflow bit in receive control regitser.
             */
            
            if (di->dma64) {
                rxh = (bcmgmacrxh_t *)ET_PKTDATA(di->dev, p);
                if (rxh->flags & GRXF_OVF) {
                    ET_PKTFREE(di->dev, p, FALSE);
                    continue;
                }
#ifdef GMAC_ERR_PKTS_DETECT
                magic = (uint32 *)rxh + GMAC_ERR_PKTS_MAGIC_OFFSET;
                if (((*magic) == GMAC_ERR_PKTS_MAGIC_TAG) &&
                    ((*(magic + 1)) == GMAC_ERR_PKTS_MAGIC_TAG)) {
                    ET_PKTFREE(di->dev, p, FALSE);
                    continue;
                }
#endif /* GMAC_ERR_PKTS_DETECT */                
            }
        }

        if (!di->en_rxsephdr) {
            /* bad frame length check */
            if (len > (di->rxbufsize - di->rxoffset)) {
                DMA_ERROR(("%s: dma_rx: bad frame length (%d)\n", di->name, len));
                if (len > 0)
                    skiplen = len - (di->rxbufsize - di->rxoffset);
                ET_PKTFREE(di->dev, p, FALSE);
                di->hnddma.rxgiants++;
                continue;
            }
            /* set actual length */
            ET_PKTSETLEN(di->dev, p, (di->rxoffset + len));
        } else { /* separate header */
            /* Packet buffer of first descriptor is header */ 
            if (len > (di->rxsephdrsize - di->rxoffset)) {
                ET_PKTSETLEN(di->dev, p, di->rxsephdrsize);
                /* Get the packet payload */
                ptr_payload = _dma_getnextrxp(di, FALSE);
                if (ptr_payload == NULL) {
                    ET_PKTFREE(di->dev, p, FALSE);
                    di->hnddma.rxnobuf++;
                    continue;
                }
                /* CHECK ME : could not set the next pointer, need increate the dv_dcnt */
                /* ET_PKTNEXT(di->dev, p) = ptr_payload; */
                ET_PKTSETLEN(di->dev, ptr_payload, (di->rxoffset + len - di->rxsephdrsize));
            } else {
                /* CHECK ME : could not set the next pointer, need increate the dv_dcnt */
                /* ET_PKTNEXT(di->dev, p) = NULL; */
                ET_PKTSETLEN(di->dev, p, (di->rxoffset + len));
            }
        }

        break;
    }

    return (p);
}

/* post receive buffers */
static void
_dma_rxfill(dma_soc_info_t *di)
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
        if (di->en_rxsephdr) {
            if ((p = ET_PKTGET(di->dev, di->dma_id, di->rxsephdrsize,
                            FALSE)) == NULL) {
                DMA_ERROR(("%s: dma_rxfill: out of rxbufs\n", di->name));
                di->hnddma.rxnobuf++;
                break;
            }
            /* PR3263 & PR3387 & PR4642 war: rxh.len=0 means dma writes not complete */
            /* Do a cached write instead of uncached write since DMA_MAP
             * will flush the cache.
             */
            *(uint32*)(ET_PKTDATA(di->dev, p)) = 0;
#ifdef GMAC_ERR_PKTS_DETECT
            /* Add the magic words to packet buffer */
            magic = (uint32*)(ET_PKTDATA(di->dev, p)) + 
                GMAC_ERR_PKTS_MAGIC_OFFSET;
            *(magic) = GMAC_ERR_PKTS_MAGIC_TAG;
            *(magic + 1) = GMAC_ERR_PKTS_MAGIC_TAG;
#endif /* GMAC_ERR_PKTS_DETECT */
            pa = (uint32) DMA_MAP(di->dev, ET_PKTDATA(di->dev, p));

            ASSERT(ISALIGNED(pa, 4));

            /* save the free packet pointer */
            ASSERT(di->rxp[rxout] == NULL);
            di->rxp[rxout] = p;

            /* reset flags for each descriptor */
            flags = 0;
            if (DMA64_ENAB(di)) {
                if (rxout == (di->nrxd - 1)) {
                    flags = D64_CTRL1_EOT;
                }
                flags |= D64_CTRL1_SOF; /* Start of Frame */

                dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, di->rxsephdrsize);
            } 
            rxout = NEXTRXD(rxout);
            i++;

            /* Get the packet buffer for Payload */
            if ((p = ET_PKTGET(di->dev, di->dma_id, (di->rxbufsize + extra_offset - di->rxsephdrsize),
                            FALSE)) == NULL) {
                DMA_ERROR(("%s: dma_rxfill: out of rxbufs\n", di->name));
                di->hnddma.rxnobuf++;
                break;
            }
            /* reserve an extra headroom, if applicable */
            if (extra_offset)
                ET_PKTPULL(di->dev, p, extra_offset);

            /* PR3263 & PR3387 & PR4642 war: rxh.len=0 means dma writes not complete */
            /* Do a cached write instead of uncached write since DMA_MAP
             * will flush the cache.
             */
            *(uint32*)(ET_PKTDATA(di->dev, p)) = 0;
#ifdef GMAC_ERR_PKTS_DETECT
            /* Add the magic words to packet buffer */
            magic = (uint32*)(ET_PKTDATA(di->dev, p)) + 
                GMAC_ERR_PKTS_MAGIC_OFFSET;
            *(magic) = GMAC_ERR_PKTS_MAGIC_TAG;
            *(magic + 1) = GMAC_ERR_PKTS_MAGIC_TAG;
#endif /* GMAC_ERR_PKTS_DETECT */            

            pa = (uint32) DMA_MAP(di->dev, ET_PKTDATA(di->dev, p));

            ASSERT(ISALIGNED(pa, 4));

            /* save the free packet pointer */
            ASSERT(di->rxp[rxout] == NULL);
            di->rxp[rxout] = p;

            /* reset flags for each descriptor */
            flags = 0;
            if (DMA64_ENAB(di)) {
                if (rxout == (di->nrxd - 1)) {
                    flags = D64_CTRL1_EOT;
                         }

                dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, di->rxbufsize - di->rxsephdrsize);
            }
            rxout = NEXTRXD(rxout);
        
        } else {
            if ((p = ET_PKTGET(di->dev, di->dma_id, di->rxbufsize + extra_offset,
                            FALSE)) == NULL) {
                DMA_ERROR(("%s: dma_rxfill: out of rxbufs\n", di->name));
                di->hnddma.rxnobuf++;
                break;
            }

            /* reserve an extra headroom, if applicable */
            if (extra_offset)
                ET_PKTPULL(di->dev, p, extra_offset);

            /* PR3263 & PR3387 & PR4642 war: rxh.len=0 means dma writes not complete */
            /* Do a cached write instead of uncached write since DMA_MAP
             * will flush the cache.
             */
            *(uint32*)(ET_PKTDATA(di->dev, p)) = 0;
#ifdef GMAC_ERR_PKTS_DETECT
            /* Add the magic words to packet buffer */
            magic = (uint32*)(ET_PKTDATA(di->dev, p)) + 
                GMAC_ERR_PKTS_MAGIC_OFFSET;
            *(magic) = GMAC_ERR_PKTS_MAGIC_TAG;
            *(magic + 1) = GMAC_ERR_PKTS_MAGIC_TAG;
#endif /* GMAC_ERR_PKTS_DETECT */            

            pa = (uint32) DMA_MAP(di->dev, ET_PKTDATA(di->dev, p));

            ASSERT(ISALIGNED(pa, 4));

            /* save the free packet pointer */
            ASSERT(di->rxp[rxout] == NULL);
            di->rxp[rxout] = p;

            /* reset flags for each descriptor */
            flags = 0;
            if (DMA64_ENAB(di)) {
                if (rxout == (di->nrxd - 1)) {
                    flags = D64_CTRL1_EOT;
                         }

                dma64_dd_upd(di, di->rxd64, pa, rxout, &flags, di->rxbufsize);
            } else {
                if (rxout == (di->nrxd - 1))
                    flags = CTRL_EOT;

                dma32_dd_upd(di, di->rxd32, pa, rxout, &flags, di->rxbufsize);
            }
            rxout = NEXTRXD(rxout);
        }
    }

    di->rxout = rxout;

    /* update the chip lastdscr pointer */
    if (DMA64_ENAB(di)) {
        W_REG(di->dev, &di->d64rxregs->ptr, I2B(rxout, dma64dd_soc_t));
    } else {
        W_REG(di->dev, &di->d32rxregs->ptr, I2B(rxout, dma32dd_soc_t));
    }
}

/* like getnexttxp but no reclaim */
static void *
_dma_peeknexttxp(dma_soc_info_t *di)
{
    uint end, i;

    if (di->ntxd == 0)
        return (NULL);

    if (DMA64_ENAB(di)) {
        end = B2I(R_REG(di->dev, 
            &di->d64txregs->status0) & D64_XS0_CD_MASK, dma64dd_soc_t);
    } else {
        end = B2I(R_REG(di->dev, 
            &di->d32txregs->status) & XS_CD_MASK, dma32dd_soc_t);
    }

    for (i = di->txin; i != end; i = NEXTTXD(i))
        if (di->txp[i])
            return (di->txp[i]);

    return (NULL);
}

static void
_dma_rxreclaim(dma_soc_info_t *di)
{
    void *p;

    /* "unused local" warning suppression for OSLs that
     * define PKTFREE() without using the di->osh arg
     */
    di = di;

    DMA_TRACE(("%s: dma_rxreclaim\n", di->name));

    while ((p = _dma_getnextrxp(di, TRUE)))
        ET_PKTFREE(di->dev, p, FALSE);
}

static void *
_dma_getnextrxp(dma_soc_info_t *di, bool forceall)
{
    if (di->nrxd == 0)
        return (NULL);

    if (DMA64_ENAB(di)) {
        return dma64_getnextrxp(di, forceall);
    } else {
        return dma32_getnextrxp(di, forceall);
    }
}

static void
_dma_txblock(dma_soc_info_t *di)
{
    di->hnddma.txavail = 0;
}

static void
_dma_txunblock(dma_soc_info_t *di)
{
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;
}

static uint
_dma_txactive(dma_soc_info_t *di)
{
    return (NTXDACTIVE(di->txin, di->txout));
}

static uint
_dma_txpending(dma_soc_info_t *di)
{
    uint curr;
    if (DMA64_ENAB(di)) {
        curr = B2I(R_REG(di->dev, 
            &di->d64txregs->status0) & D64_XS0_CD_MASK, dma64dd_soc_t);
    } else {
        curr = B2I(R_REG(di->dev, 
            &di->d32txregs->status) & XS_CD_MASK, dma32dd_soc_t);
    }
    return (NTXDACTIVE(curr, di->txout));
}

static uint
_dma_rxactive(dma_soc_info_t *di)
{
    return (NRXDACTIVE(di->rxin, di->rxout));
}

static void
_dma_counterreset(dma_soc_info_t *di)
{
    /* reset all software counter */
    di->hnddma.rxgiants = 0;
    di->hnddma.rxnobuf = 0;
    di->hnddma.txnobuf = 0;
}

/* get the address of the var in order to change later */
static uint *
_dma_getvar(dma_soc_info_t *di, const char *name)
{
    if (!strcmp(name, "&txavail"))
        return ((uint *) &(di->hnddma.txavail));
    else {
        ASSERT(0);
    }
    return (0);
}

static char*
dma32_dumpring(dma_soc_info_t *di, char *buf, dma32dd_soc_t *ring, uint start, uint end,
               uint max_num)
{
    uint i;

    for (i = start; i != end; i = XXD((i + 1), max_num)) {
        /* in the format of high->low 8 bytes */
        buf += sprintf(buf, "ring index %d: 0x%x %x\n", i, ring[i].addr, ring[i].ctrl);
    }
    return buf;
}

static char*
dma32_dumptx(dma_soc_info_t *di, char *buf, bool dumpring)
{
    if (di->ntxd == 0) {
        return buf;
    }

    buf += sprintf(buf, "DMA32: txd32 %p txdpa 0x%lx txp %p txin %d txout %d "
               "txavail %d\n", (void *)di->txd32, di->txdpa, (void *)di->txp, di->txin,
               di->txout, di->hnddma.txavail);

    buf += sprintf(buf, "xmtcontrol 0x%x xmtaddr 0x%x xmtptr 0x%x xmtstatus 0x%x\n",
        R_REG(di->dev, &di->d32txregs->control),
        R_REG(di->dev, &di->d32txregs->addr),
        R_REG(di->dev, &di->d32txregs->ptr),
        R_REG(di->dev, &di->d32txregs->status));

    if (dumpring && di->txd32)
        buf = dma32_dumpring(di, buf, di->txd32, di->txin, di->txout, di->ntxd);
    return buf;
}

static char*
dma32_dumprx(dma_soc_info_t *di, char *buf, bool dumpring)
{
    if (di->nrxd == 0) {
        return buf;
    }

    buf += sprintf(buf, "DMA32: rxd32 %p rxdpa 0x%lx rxp %p rxin %d rxout %d\n",
        (void *)di->rxd32, di->rxdpa, (void *)di->rxp, di->rxin, di->rxout);

    buf += sprintf(buf, "rcvcontrol 0x%x rcvaddr 0x%x rcvptr 0x%x rcvstatus 0x%x\n",
        R_REG(di->dev, &di->d32rxregs->control),
        R_REG(di->dev, &di->d32rxregs->addr),
        R_REG(di->dev, &di->d32rxregs->ptr),
        R_REG(di->dev, &di->d32rxregs->status));
    if (di->rxd32 && dumpring)
        buf = dma32_dumpring(di, buf, di->rxd32, di->rxin, di->rxout, di->nrxd);
    return buf;
}

static char*
dma32_dump(dma_soc_info_t *di, char *b, bool dumpring)
{
    b = dma32_dumptx (di, b, dumpring);
    b = dma32_dumprx(di, b, dumpring);
    return b;
}

static char*
dma64_dumpring(dma_soc_info_t *di, char *buf, dma64dd_soc_t *ring, uint start, uint end,
               uint max_num)
{
    uint i;

    for (i = start; i != end; i = XXD((i + 1), max_num)) {
        /* in the format of high->low 16 bytes */
        buf += sprintf(buf, "ring index %d: 0x%x %x %x %x\n",
                       i, ring[i].addrhigh, ring[i].addrlow, ring[i].ctrl2, ring[i].ctrl1);
    }
    return buf;
}

static char*
dma64_dumptx(dma_soc_info_t *di, char *buf, bool dumpring)
{
    if (di->ntxd == 0)
        return buf;

    buf += sprintf(buf, "DMA64: txd64 %p txdpa 0x%lx txp %p txin %d txout %d "
               "txavail %d\n", (void *)di->txd64, di->txdpa, (void *)di->txp, di->txin,
               di->txout, di->hnddma.txavail);

    buf += sprintf(buf, "xmtcontrol 0x%x xmtaddrlow 0x%x xmtaddrhigh 0x%x "
               "xmtptr 0x%x xmtstatus0 0x%x xmtstatus1 0x%x\n",
               R_REG(di->dev, &di->d64txregs->control),
               R_REG(di->dev, &di->d64txregs->addrlow),
               R_REG(di->dev, &di->d64txregs->addrhigh),
               R_REG(di->dev, &di->d64txregs->ptr),
               R_REG(di->dev, &di->d64txregs->status0),
               R_REG(di->dev, &di->d64txregs->status1));

    if (dumpring && di->txd64) {
        buf = dma64_dumpring(di, buf, di->txd64, di->txin, di->txout, di->ntxd);
    }
    return buf;
}

static char*
dma64_dumprx(dma_soc_info_t *di, char *buf, bool dumpring)
{
    if (di->nrxd == 0)
        return buf;

    buf += sprintf(buf, "DMA64: rxd64 %p rxdpa 0x%lx rxp %p rxin %d rxout %d\n",
        (void *)di->rxd64, di->rxdpa, (void *)di->rxp, di->rxin, di->rxout);

    buf += sprintf(buf, "rcvcontrol 0x%x rcvaddrlow 0x%x rcvaddrhigh 0x%x rcvptr "
               "0x%x rcvstatus0 0x%x rcvstatus1 0x%x\n",
               R_REG(di->dev, &di->d64rxregs->control),
               R_REG(di->dev, &di->d64rxregs->addrlow),
               R_REG(di->dev, &di->d64rxregs->addrhigh),
               R_REG(di->dev, &di->d64rxregs->ptr),
               R_REG(di->dev, &di->d64rxregs->status0),
               R_REG(di->dev, &di->d64rxregs->status1));
    if (di->rxd64 && dumpring) {
        buf = dma64_dumpring(di, buf, di->rxd64, di->rxin, di->rxout, di->nrxd);
    }
    return buf;
}

static char*
dma64_dump(dma_soc_info_t *di, char *b, bool dumpring)
{
    b = dma64_dumptx(di, b, dumpring);
    b = dma64_dumprx(di, b, dumpring);
    return b;
}


/* 32 bits DMA functions */
static void
dma32_txinit(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_txinit\n", di->name));

    if (di->ntxd == 0)
        return;

    di->txin = di->txout = 0;
    di->hnddma.txavail = di->ntxd - 1;

    /* clear tx descriptor ring */
    BZERO_SM((void *)(uint *)di->txd32, (di->ntxd * sizeof(dma32dd_soc_t)));
    /* XXX: Disabling parity checks, write parity gen code. */
    W_REG(di->dev, &di->d32txregs->control, XC_PD | XC_XE);
    _dma_ddtable_init(di, DMA_TX, di->txdpa);
}

static bool
dma32_txenabled(dma_soc_info_t *di)
{
    uint32 xc;

    /* If the chip is dead, it is not enabled :-) */
    xc = R_REG(di->dev, &di->d32txregs->control);
    return ((xc != 0xffffffff) && (xc & XC_XE));
}

static void
dma32_txsuspend(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_txsuspend\n", di->name));

    if (di->ntxd == 0)
        return;

    OR_REG(di->dev, &di->d32txregs->control, XC_SE);
}

static void
dma32_txresume(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_txresume\n", di->name));

    if (di->ntxd == 0)
        return;

    AND_REG(di->dev, &di->d32txregs->control, ~XC_SE);
}

static bool
dma32_txsuspended(dma_soc_info_t *di)
{
    return (di->ntxd == 0) || ((R_REG(di->dev, &di->d32txregs->control) & XC_SE) == XC_SE);
}

static void
dma32_txreclaim(dma_soc_info_t *di, bool forceall)
{
    void *p;

    DMA_TRACE(("%s: dma_txreclaim %s\n", di->name, forceall ? "all" : ""));

    while ((p = dma32_getnexttxp(di, forceall)))
        ET_PKTFREE(di->dev, p, TRUE);
}

static bool
dma32_txstopped(dma_soc_info_t *di)
{
    return ((R_REG(di->dev, &di->d32txregs->status) & XS_XS_MASK) == XS_XS_STOPPED);
}

static bool
dma32_rxstopped(dma_soc_info_t *di)
{
    return ((R_REG(di->dev, &di->d32rxregs->status) & RS_RS_MASK) == RS_RS_STOPPED);
}

static bool
dma32_alloc(dma_soc_info_t *di, uint direction)
{
    uint size;
    uint ddlen;
    void *va;

    ddlen = sizeof(dma32dd_soc_t);

    size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);

    /* Always add the size of alignment */
    size += D32RINGALIGN;

    if (direction == DMA_TX) {
        if ((va = DMA_ALLOC_CONSISTENT(di->dev, size, &di->txdpa)) == NULL) {
            DMA_ERROR(("%s: dma_alloc: DMA_ALLOC_CONSISTENT(ntxd) failed\n",
                       di->name));
            return FALSE;
        }
        di->txdpa = DMA_MAP(di->dev, va);
        di->txd32 = (dma32dd_soc_t *) ROUNDUP((uint *)va, D32RINGALIGN);
        di->txdalign = (uint)((int8*)(uint *)di->txd32 - (int8*)va);
        di->txdpa += di->txdalign;
        di->txdalloc = size;
        ASSERT(ISALIGNED((uint *)di->txd32, D32RINGALIGN));
    } else {
        if ((va = DMA_ALLOC_CONSISTENT(di->dev, size, &di->rxdpa)) == NULL) {
            DMA_ERROR(("%s: dma_alloc: DMA_ALLOC_CONSISTENT(nrxd) failed\n",
                       di->name));
            return FALSE;
        }
        di->rxdpa = DMA_MAP(di->dev, va);
        di->rxd32 = (dma32dd_soc_t *) ROUNDUP((uint *)va, D32RINGALIGN);
        di->rxdalign = (uint)((int8*)(uint *)di->rxd32 - (int8*)va);
        di->rxdpa += di->rxdalign;
        di->rxdalloc = size;
        ASSERT(ISALIGNED((uint *)di->rxd32, D32RINGALIGN));
    }

    return TRUE;
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
static bool
dma32_txreset(dma_soc_info_t *di)
{
    uint32 status;

    if (di->ntxd == 0)
        return TRUE;

    /* address PR8249/PR7577 issue */
    /* suspend tx DMA first */
    W_REG(di->dev, &di->d32txregs->control, XC_SE);
    SPINWAIT(((status = (R_REG(di->dev, &di->d32txregs->status) & XS_XS_MASK))
         != XS_XS_DISABLED) &&
         (status != XS_XS_IDLE) &&
         (status != XS_XS_STOPPED),
         (10000));

    /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
    W_REG(di->dev, &di->d32txregs->control, 0);
    SPINWAIT(((status = (R_REG(di->dev,
             &di->d32txregs->status) & XS_XS_MASK)) != XS_XS_DISABLED),
             10000);

    /* wait for the last transaction to complete */
    OSL_DELAY(300);

    return (status == XS_XS_DISABLED);
}

static bool
dma32_rxidle(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_rxidle\n", di->name));

    if (di->nrxd == 0)
        return TRUE;

    return ((R_REG(di->dev, &di->d32rxregs->status) & RS_CD_MASK) ==
            R_REG(di->dev, &di->d32rxregs->ptr));
}

static bool
dma32_rxreset(dma_soc_info_t *di)
{
    uint32 status;

    if (di->nrxd == 0)
        return TRUE;

    /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
    W_REG(di->dev, &di->d32rxregs->control, 0);
    SPINWAIT(((status = (R_REG(di->dev,
             &di->d32rxregs->status) & RS_RS_MASK)) != RS_RS_DISABLED),
             10000);

    return (status == RS_RS_DISABLED);
}

static bool
dma32_rxenabled(dma_soc_info_t *di)
{
    uint32 rc;

    rc = R_REG(di->dev, &di->d32rxregs->control);
    return ((rc != 0xffffffff) && (rc & RC_RE));
}

static bool
dma32_txsuspendedidle(dma_soc_info_t *di)
{
    if (di->ntxd == 0)
        return TRUE;

    if (!(R_REG(di->dev, &di->d32txregs->control) & XC_SE))
        return 0;

    if ((R_REG(di->dev, &di->d32txregs->status) & XS_XS_MASK) != XS_XS_IDLE)
        return 0;

    /*
     * PR20059 WAR(tested on d11mac only):
     *   when the driver sees the dma status as "idle" it waits for
     * a small amount of time, say a couple of us and then retests the status.
     * If it's still "idle" then the dma has indeed suspended, if not then wait
     * again for "idle", and so on (The false case is handled outside here).
     */
    OSL_DELAY(2);
    return ((R_REG(di->dev, &di->d32txregs->status) & XS_XS_MASK) == XS_XS_IDLE);
}

/* !! tx entry routine
 * supports full 32bit dma engine buffer addressing so
 * dma buffers can cross 4 Kbyte page boundaries.
 */
static int
dma32_txfast(dma_soc_info_t *di, void *p0, bool commit)
{
    void *p, *next;
    uchar *data;
    uint len;
    uint txout;
    uint32 flags = 0;
    uint32 pa;
    int chain;

    DMA_TRACE(("%s: dma_txfast\n", di->name));

    txout = di->txout;

    /*
     * Walk the chain of packet buffers
     * allocating and initializing transmit descriptor entries.
     */
    for (p = p0, chain = 0; p; p = next, chain++) {
        data = ET_PKTDATA(di->dev, p);
        len = ET_PKTLEN(di->dev, p);
#ifdef BCM_DMAPAD
        len += ET_PKTDMAPAD(di->osh, p);
#endif
        next = ET_PKTNEXT(di->dev, p);

        /* return nonzero if out of tx descriptors */
        if (NEXTTXD(txout) == di->txin)
            goto outoftxd;

        /* PR988 - skip zero length buffers */
        if (len == 0)
            continue;

        /* Flush the cached data to physical memory for DMA to process */
        soc_cm_sflush((int)di->dev, data, len);

        /* get physical address of buffer start */
        pa = (uint32) DMA_MAP(di->dev, data);

        flags = 0;
        if ((p == p0) && !chain)
            flags |= CTRL_SOF;
        if (next == NULL)
            flags |= (CTRL_IOC | CTRL_EOF);
        if (txout == (di->ntxd - 1))
            flags |= CTRL_EOT;

        dma32_dd_upd(di, di->txd32, pa, txout, &flags, len);
        ASSERT(di->txp[txout] == NULL);

        txout = NEXTTXD(txout);
    }

    /* if last txd eof not set, fix it */
    if (!(flags & CTRL_EOF))
        W_SM(&di->txd32[PREVTXD(txout)].ctrl, BUS_SWAP32(flags | CTRL_IOC | CTRL_EOF));

    /* save the packet */
    di->txp[PREVTXD(txout)] = p0;

    /* bump the tx descriptor index */
    di->txout = txout;

    /* kick the chip */
    if (commit) {
        W_REG(di->dev, &di->d32txregs->ptr, I2B(txout, dma32dd_soc_t));
    }

    /* tx flow control */
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

    return (0);

outoftxd:
    DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
    ET_PKTFREE(di->dev, p0, TRUE);
    di->hnddma.txavail = 0;
    di->hnddma.txnobuf++;
    return (-1);
}

/*
 * Reclaim next completed txd (txds if using chained buffers) and
 * return associated packet.
 * If 'force' is true, reclaim txd(s) and return associated packet
 * regardless of the value of the hardware "curr" pointer.
 */
static void *
dma32_getnexttxp(dma_soc_info_t *di, bool forceall)
{
    uint start, end, i;
    void *txp;

    DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name, forceall ? "all" : ""));

    if (di->ntxd == 0)
        return (NULL);

    txp = NULL;

    start = di->txin;
    if (forceall)
        end = di->txout;
    else
        end = B2I(R_REG(di->dev, &di->d32txregs->status) & XS_CD_MASK, dma32dd_soc_t);

    /* PR4738 - xmt disable/re-enable does not clear CURR */
    if ((start == 0) && (end > di->txout))
        goto bogus;

    for (i = start; i != end && !txp; i = NEXTTXD(i)) {
        DMA_UNMAP(di->dev, (BUS_SWAP32(R_SM(&di->txd32[i].addr)) - di->dataoffsetlow));

        W_SM(&di->txd32[i].addr, 0xdeadbeef);
        txp = di->txp[i];
        di->txp[i] = NULL;
    }

    di->txin = i;

    /* tx flow control */
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

    return (txp);

bogus:
/*
    DMA_ERROR(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
        start, end, di->txout, forceall));
*/
    return (NULL);
}

static void *
dma32_getnextrxp(dma_soc_info_t *di, bool forceall)
{
    uint i;
    void *rxp;

    /* if forcing, dma engine must be disabled */
    ASSERT(!forceall || !dma32_rxenabled(di));

    i = di->rxin;

    /* return if no packets posted */
    if (i == di->rxout)
        return (NULL);

    /* ignore curr if forceall */
    if (!forceall && (i == B2I(R_REG(di->dev, &di->d32rxregs->status) & RS_CD_MASK, dma32dd_soc_t)))
        return (NULL);

    /* get the packet pointer that corresponds to the rx descriptor */
    rxp = di->rxp[i];
    ASSERT(rxp);
    di->rxp[i] = NULL;

    /* clear this packet from the descriptor ring */
    DMA_UNMAP(di->dev, (BUS_SWAP32(R_SM(&di->rxd32[i].addr)) - di->dataoffsetlow));

    W_SM(&di->rxd32[i].addr, 0xdeadbeef);

    di->rxin = NEXTRXD(i);

    return (rxp);
}

/*
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
static void
dma32_txrotate(dma_soc_info_t *di)
{
    uint ad;
    uint nactive;
    uint rot;
    uint old, new;
    uint32 w;
    uint first, last;

    ASSERT(dma32_txsuspendedidle(di));

    nactive = _dma_txactive(di);
    ad = B2I(((R_REG(di->dev, 
        &di->d32txregs->status) & XS_AD_MASK) >> XS_AD_SHIFT), dma32dd_soc_t);
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
        w = BUS_SWAP32(R_SM(&di->txd32[old].ctrl)) & ~CTRL_EOT;
        if (new == (di->ntxd - 1))
            w |= CTRL_EOT;
        W_SM(&di->txd32[new].ctrl, BUS_SWAP32(w));
        W_SM(&di->txd32[new].addr, R_SM(&di->txd32[old].addr));

        /* zap the old tx dma descriptor address field */
        W_SM(&di->txd32[old].addr, BUS_SWAP32(0xdeadbeef));

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
    W_REG(di->dev, &di->d32txregs->ptr, I2B(di->txout, dma32dd_soc_t));
}

/* for bcm5836 and bcm4704 only */
static void
dma32_rxrecycle(dma_soc_info_t *di)
{
    void *p;
    uint i;
    void *rxp;
    uint32 pa;
    uint rxbufsize;
    uint ctrl;
    uint rxout;
    void **pkts;
    uint pktc = 0;

    rxbufsize = di->rxbufsize;
    
    /*
     * Re-write descriptor table address
     */
    W_REG(di->dev, &di->d32rxregs->addr, ((uint32)di->rxdpa + di->ddoffsetlow));

    /*
     * Make sure EOT flag is there for the last descriptor
     */
    ctrl = rxbufsize;
    ctrl |= CTRL_EOT;
    W_SM(&di->rxd32[di->nrxd - 1].ctrl, BUS_SWAP32(ctrl));

    /*
     * Allocate buffer to store extracted pkts
     */
    pkts = ET_MALLOC(di->nrxd * sizeof(void *));
    if (pkts == NULL) {
        /*
         * Since we're out of memory, just do it the old way.
         */
        _dma_rxreclaim(di);
        _dma_rxfill(di);
        return;
    }
    
    /*
     * Extract pkts for recycling
     */
    for(i=0; i<di->nrxd; i++) {
        rxp = di->rxp[i];
        if (rxp != NULL) {
            di->rxp[i] = NULL;
            
            pkts[pktc++] = rxp;
        
            DMA_UNMAP(di->dev, 
                     (BUS_SWAP32(R_SM(&di->rxd32[i].addr)) - di->ddoffsetlow));
            W_SM(&di->rxd32[i].addr, 0);
        }
    }
    
    /*
     * Fill pkts in table from the beginning (top down)
     */
    for(rxout=0; rxout<pktc; rxout++) {
        
        /* Get a pkt from the extracted buffer */
        p = pkts[rxout];
        
        /* rxh.len = 0 means dma writes not complete */
        *(uint32*)(OSL_UNCACHED(ET_PKTDATA(di->dev, p))) = 0;

        /* Get the physical address */
        pa = (uint32) DMA_MAP(di->dev, ET_PKTDATA(di->dev, p));

        /* Save the free pkt in table */
        di->rxp[rxout] = p;

        /* Init the rx descriptor */
        ctrl = rxbufsize;
        W_SM(&di->rxd32[rxout].ctrl, BUS_SWAP32(ctrl));
        W_SM(&di->rxd32[rxout].addr, BUS_SWAP32(pa + di->dataoffsetlow));

    }
    
    /* Try Filling pkts from rx queue (if we can) */
    for (; rxout < di->nrxd - 1; rxout++) {
            
        if ((p = ET_PKTGET(di->dev, di->dma_id, rxbufsize, FALSE)) == NULL) {
            break;
        }
        
        /* We got one more pkt */
        pktc++;
        
        /* rxh.len = 0 means dma writes not complete */
        *(uint32*)(OSL_UNCACHED(ET_PKTDATA(di->dev, p))) = 0;

        /* Get the physical address */
        pa = (uint32) DMA_MAP(di->dev, ET_PKTDATA(di->dev, p));

        /* Save the free pkt in table */
        di->rxp[rxout] = p;

        /* Init the rx descriptor */
        ctrl = rxbufsize;
        W_SM(&di->rxd32[rxout].ctrl, BUS_SWAP32(ctrl));
        W_SM(&di->rxd32[rxout].addr, BUS_SWAP32(pa + di->dataoffsetlow));
    }
    
    /* Now we have <pktc> descriptors. */
    di->rxin = 0;
    di->rxout = pktc;
    
    /* Update the chip lastdscr pointer */
    W_REG(di->dev, &di->d32rxregs->ptr, I2B(di->rxout, dma32dd_soc_t));
    
    /* Free the extraction buffer */
    ET_MFREE(pkts, di->nrxd * sizeof(void *));
}


/* 64 bits DMA functions */

static void
dma64_txinit(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_txinit\n", di->name));

    if (di->ntxd == 0)
        return;

    di->txin = di->txout = 0;
    di->hnddma.txavail = di->ntxd - 1;

    /* clear tx descriptor ring */
    BZERO_SM((void *)(uint *)di->txd64, (di->ntxd * sizeof(dma64dd_soc_t)));
    /* XXX: Disabling parity checks, write parity gen code. */
    W_REG(di->dev, &di->d64txregs->control, D64_XC_PD | D64_XC_XE);
    _dma_ddtable_init(di, DMA_TX, di->txdpa);
}

static bool
dma64_txenabled(dma_soc_info_t *di)
{
    uint32 xc;

    /* If the chip is dead, it is not enabled :-) */
    xc = R_REG(di->dev, &di->d64txregs->control);
    return ((xc != 0xffffffff) && (xc & D64_XC_XE));
}

static void
dma64_txsuspend(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_txsuspend\n", di->name));

    if (di->ntxd == 0)
        return;

    OR_REG(di->dev, &di->d64txregs->control, D64_XC_SE);
}

static void
dma64_txresume(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_txresume\n", di->name));

    if (di->ntxd == 0)
        return;

    AND_REG(di->dev, &di->d64txregs->control, ~D64_XC_SE);
}

static bool
dma64_txsuspended(dma_soc_info_t *di)
{
    return (di->ntxd == 0) || ((R_REG(di->dev, &di->d64txregs->control) & D64_XC_SE)
            == D64_XC_SE);
}

static void
dma64_txreclaim(dma_soc_info_t *di, bool forceall)
{
    void *p;

    DMA_TRACE(("%s: dma_txreclaim %s\n", di->name, forceall ? "all" : ""));

    while ((p = dma64_getnexttxp(di, forceall)))
        ET_PKTFREE(di->dev, p, TRUE);
}

static bool
dma64_txstopped(dma_soc_info_t *di)
{
    return ((R_REG(di->dev, &di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_STOPPED);
}

static bool
dma64_rxstopped(dma_soc_info_t *di)
{
    return ((R_REG(di->dev, &di->d64rxregs->status0) & D64_RS0_RS_MASK) == D64_RS0_RS_STOPPED);
}

static bool
dma64_alloc(dma_soc_info_t *di, uint direction)
{
    uint size;
    uint ddlen;
    uint32 alignbytes;
    void *va;
    ulong dpa;

    ddlen = sizeof(dma64dd_soc_t);

    size = (direction == DMA_TX) ? (di->ntxd * ddlen) : (di->nrxd * ddlen);

    alignbytes = di->dma64align;

    /* Always add the size of alignment */
    size += alignbytes;
    
    if (direction == DMA_TX) {
        if ((va = DMA_ALLOC_CONSISTENT(di->dev, size, &di->txdpa)) == NULL) {
            DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(ntxd) failed\n",
                       di->name));
            return FALSE;
        }
        
        di->txdpa = DMA_MAP(di->dev, va);
        dpa = ROUNDUP(di->txdpa, alignbytes);
        di->txdalign = dpa - di->txdpa;
        di->txdpa = dpa;
        di->txd64 = (dma64dd_soc_t *)(va + di->txdalign);
        di->txdalloc = size;
        ASSERT(ISALIGNED((uint *)di->txdpa, alignbytes));
    } else {
        if ((va = DMA_ALLOC_CONSISTENT(di->dev, size, &di->rxdpa)) == NULL) {
            DMA_ERROR(("%s: dma64_alloc: DMA_ALLOC_CONSISTENT(nrxd) failed\n",
                       di->name));
            return FALSE;
        }
        di->rxdpa = DMA_MAP(di->dev, va);
        dpa = ROUNDUP(di->rxdpa, alignbytes);
        di->rxdalign = dpa - di->rxdpa;
        di->rxdpa = dpa;
        di->rxd64 = (dma64dd_soc_t *)(va + di->rxdalign);
        di->rxdalloc = size;
        ASSERT(ISALIGNED((uint *)di->rxdpa, alignbytes));
    }

    return TRUE;
}

static bool
dma64_txreset(dma_soc_info_t *di)
{
    uint32 status;

    if (di->ntxd == 0)
        return TRUE;

    /* address PR8249/PR7577 issue */
    /* suspend tx DMA first */
    W_REG(di->dev, &di->d64txregs->control, D64_XC_SE);
    SPINWAIT(((status = (R_REG(di->dev, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
              D64_XS0_XS_DISABLED) &&
             (status != D64_XS0_XS_IDLE) &&
             (status != D64_XS0_XS_STOPPED),
             10000);

    /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
    W_REG(di->dev, &di->d64txregs->control, 0);
    SPINWAIT(((status = (R_REG(di->dev, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
              D64_XS0_XS_DISABLED),
             10000);

    /* wait for the last transaction to complete */
    OSL_DELAY(300);

    return (status == D64_XS0_XS_DISABLED);
}

static bool
dma64_rxidle(dma_soc_info_t *di)
{
    DMA_TRACE(("%s: dma_rxidle\n", di->name));

    if (di->nrxd == 0)
        return TRUE;

    return ((R_REG(di->dev, &di->d64rxregs->status0) & D64_RS0_CD_MASK) ==
        R_REG(di->dev, &di->d64rxregs->ptr));
}

static bool
dma64_rxreset(dma_soc_info_t *di)
{
    uint32 status;

    if (di->nrxd == 0)
        return TRUE;

    /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
    W_REG(di->dev, &di->d64rxregs->control, 0);
    SPINWAIT(((status = (R_REG(di->dev, &di->d64rxregs->status0) & D64_RS0_RS_MASK)) !=
              D64_RS0_RS_DISABLED),
             10000);

    return (status == D64_RS0_RS_DISABLED);
}

static bool
dma64_rxenabled(dma_soc_info_t *di)
{
    uint32 rc;

    rc = R_REG(di->dev, &di->d64rxregs->control);
    return ((rc != 0xffffffff) && (rc & D64_RC_RE));
}

static bool
dma64_txsuspendedidle(dma_soc_info_t *di)
{
    /* XXX - verify PR20059 has been fixed */

    if (di->ntxd == 0)
        return TRUE;

    if (!(R_REG(di->dev, &di->d64txregs->control) & D64_XC_SE))
        return 0;

    if ((R_REG(di->dev, &di->d64txregs->status0) & D64_XS0_XS_MASK) == D64_XS0_XS_IDLE)
        return 1;

    return 0;
}


/* !! tx entry routine */
static int
dma64_txfast(dma_soc_info_t *di, void *p0, bool commit)
{
    void *p, *next;
    uchar *data;
    uint len;
    uint txout;
    uint32 flags = 0;
    uint32 pa;
    int chain;

    DMA_TRACE(("%s: dma_txfast\n", di->name));

    txout = di->txout;

    /*
     * Walk the chain of packet buffers
     * allocating and initializing transmit descriptor entries.
     */
    for (p = p0, chain = 0; p; p = next, chain++) {
        data = ET_PKTDATA(di->osh, p);
        len = ET_PKTLEN(di->osh, p);
#ifdef BCM_DMAPAD
        len += PKTDMAPAD(di->osh, p);
#endif
        next = ET_PKTNEXT(di->osh, p);

        /* return nonzero if out of tx descriptors */
        if (NEXTTXD(txout) == di->txin)
            goto outoftxd;

        /* PR988 - skip zero length buffers */
        if (len == 0)
            continue;

        /* Flush the cached data to physical memory for DMA to process */
        soc_cm_sflush((int)di->dev, data, len);

        /* get physical address of buffer start */
        pa = (uint32) DMA_MAP(di->dev, data);

        if (ET_PKTFLAGTS(di->osh, p)) {
            flags = D64_CTRL1_TS;
        } else {
            flags = 0;
        }
        
        if ((p == p0) && !chain)
            flags |= D64_CTRL1_SOF;
        if (next == NULL)
            flags |= (D64_CTRL1_IOC | D64_CTRL1_EOF);
        if (txout == (di->ntxd - 1))
            flags |= D64_CTRL1_EOT;

        dma64_dd_upd(di, di->txd64, pa, txout, &flags, len);
        ASSERT(di->txp[txout] == NULL);

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
    if (commit) {
        W_REG(di->dev, &di->d64txregs->ptr, I2B(txout, dma64dd_soc_t));
    }


    /* tx flow control */
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

    return (0);

outoftxd:
    DMA_ERROR(("%s: dma_txfast: out of txds\n", di->name));
    ET_PKTFREE(di->dev, p0, TRUE);
    di->hnddma.txavail = 0;
    di->hnddma.txnobuf++;
    return (-1);
}

/*
 * Reclaim next completed txd (txds if using chained buffers) and
 * return associated packet.
 * If 'force' is true, reclaim txd(s) and return associated packet
 * regardless of the value of the hardware "curr" pointer.
 */
static void *
dma64_getnexttxp(dma_soc_info_t *di, bool forceall)
{
    uint start, end, i;
    void *txp;

    DMA_TRACE(("%s: dma_getnexttxp %s\n", di->name, forceall ? "all" : ""));

    if (di->ntxd == 0)
        return (NULL);

    txp = NULL;

    start = di->txin;
    if (forceall)
        end = di->txout;
    else
        end = B2I(R_REG(di->dev, 
            &di->d64txregs->status0) & D64_XS0_CD_MASK, dma64dd_soc_t);

    /* PR4738 - xmt disable/re-enable does not clear CURR */
    if ((start == 0) && (end > di->txout))
        goto bogus;

    for (i = start; i != end && !txp; i = NEXTTXD(i)) {
        /* XXX - dma 64 bits */
        DMA_UNMAP(di->dev, (BUS_SWAP32(R_SM(&di->txd64[i].addrlow)) - di->dataoffsetlow));

        W_SM(&di->txd64[i].addrlow, 0xdeadbeef);
        W_SM(&di->txd64[i].addrhigh, 0xdeadbeef);

        txp = di->txp[i];
        di->txp[i] = NULL;
    }

    di->txin = i;

    /* tx flow control */
    di->hnddma.txavail = di->ntxd - NTXDACTIVE(di->txin, di->txout) - 1;

    return (txp);

bogus:
/*
    DMA_ERROR(("dma_getnexttxp: bogus curr: start %d end %d txout %d force %d\n",
        start, end, di->txout, forceall));
*/
    return (NULL);
}

static void *
dma64_getnextrxp(dma_soc_info_t *di, bool forceall)
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
        (i == B2I(R_REG(di->dev, 
            &di->d64rxregs->status0) & D64_RS0_CD_MASK, dma64dd_soc_t)))
        return (NULL);

    /* get the packet pointer that corresponds to the rx descriptor */
    rxp = di->rxp[i];
    ASSERT(rxp);
    di->rxp[i] = NULL;

    /* clear this packet from the descriptor ring */
    DMA_UNMAP(di->dev, (BUS_SWAP32(R_SM(&di->rxd64[i].addrlow)) - di->dataoffsetlow));

    W_SM(&di->rxd64[i].addrlow, 0xdeadbeef);
    W_SM(&di->rxd64[i].addrhigh, 0xdeadbeef);

    di->rxin = NEXTRXD(i);

    return (rxp);
}

static bool
_dma64_addrext(dma_soc_info_t *di, dma64regs_soc_t *dma64regs)
{
    uint32 w;
    OR_REG(di->dev, &dma64regs->control, D64_XC_AE);
    w = R_REG(di->dev, &dma64regs->control);
    AND_REG(di->dev, &dma64regs->control, ~D64_XC_AE);
    return ((w & D64_XC_AE) == D64_XC_AE);
}

/*
 * Rotate all active tx dma ring entries "forward" by (ActiveDescriptor - txin).
 */
static void
dma64_txrotate(dma_soc_info_t *di)
{
    uint ad;
    uint nactive;
    uint rot;
    uint old, new;
    uint32 w;
    uint first, last;

    ASSERT(dma64_txsuspendedidle(di));

    nactive = _dma_txactive(di);
    ad = B2I((R_REG(di->dev, &di->d64txregs->status1) & D64_XS1_AD_MASK), dma64dd_soc_t);
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
    W_REG(di->dev, &di->d64txregs->ptr, I2B(di->txout, dma64dd_soc_t));
}


static void
dma64_rxsephdrctrl(dma_soc_info_t *di, bool enable, uint memtype, uint size)
{
    di->en_rxsephdr = enable;
    di->pkthdr_mem = memtype;
    di->rxsephdrsize = size;
}

#endif /* defined(KEYSTONE) || defined(ROBO_4704) || defined(IPROC_CMICD) */
