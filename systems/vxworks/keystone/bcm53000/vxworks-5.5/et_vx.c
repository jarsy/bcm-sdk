/*
 * vxWorks END device driver for
 * Broadcom BCM47XX 10/100 Mbps Ethernet Controller
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: et_vx.c,v 1.6 Broadcom SDK $
 */

#ifndef lint
static const char cvsid[] = "$Id: et_vx.c,v 1.6 Broadcom SDK $";
#endif /* lint */

#define __INCunixLibh       /* XXX temp defeat unixLib.h MALLOC() definition */

#include <typedefs.h>
#include <vxshared.h>
#include <endLib.h>
#include <intLib.h>
#include <ioctl.h>
#include <netLib.h>
#include <wdLib.h>
#include <taskLib.h>

#include <drv/pci/pciConfigLib.h>

#undef  ETHER_MAP_IP_MULTICAST
#include <etherMultiLib.h>

#include <osl.h>

#include <config.h>

#include <epivers.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <proto/ethernet.h>
#include <bcmgmacmib.h>
#include <bcmgmacrxh.h>
#include <etioctl.h>
#include <et_dbg.h>
#include <etc.h>

/* PED */
#include <cacheLib.h>
#include <siutils.h>
#include <gmac0_core.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <etcgmac.h>
#include <hndsoc.h>


/* mblk queue */
struct mblkq {
    M_BLK_ID    head;
    M_BLK_ID    tail;
    uint        len;
};

/* PED: for fast access to txp/rxp with in/out byte offset */
typedef struct _pkt_wrapper {
    void        *pkt;
    uint32      dummy[3];
} pktwrap_t;

/* PED */
typedef struct dma_info {
    dma64regs_t *d64txregs; /* 64 bits dma tx engine registers */
    dma64regs_t *d64rxregs; /* 64 bits dma rx engine registers */

    dma64dd_t   *txd64;     /* pointer to dma64 tx descriptor ring */
    ulong       txdpa;      /* physical address of descriptor ring */
    uint        txin;       /* byte offset of next descriptor to reclaim */
    uint        txout;      /* byte offset of next descriptor to post */
    pktwrap_t   *txp;       /* pointer to parallel array of pointers to packets */

    dma64dd_t   *rxd64;     /* pointer to dma64 rx descriptor ring */
    ulong       rxdpa;      /* physical address of descriptor ring */
    uint        rxin;       /* byte offset of next descriptor to reclaim */
    uint        rxout;      /* byte offset of next descriptor to post */
    pktwrap_t   *rxp;       /* pointer to parallel array of pointers to packets */
    
    struct dma_info     *peer;  /* di for the same channel of another GMAC */
    osl_t *osh;
} dma_info_t;

/* PED: modified for byte offset (was index) */
#define DESCSIZE            sizeof(dma64dd_t)
#define XXD(x, n)           ((x) & ((n) - DESCSIZE))
#define TXD(x)              XXD((x), NTXD*DESCSIZE)
#define RXD(x)              XXD((x), NRXD*DESCSIZE)
#define NEXTTXD(i)          TXD((i) + DESCSIZE)
#define PREVTXD(i)          TXD((i) - DESCSIZE)
#define NEXTRXD(i)          RXD((i) + DESCSIZE)
#define PREVRXD(i)          RXD((i) - DESCSIZE)
#define NTXDACTIVE(h, t)    (TXD(t - h) / DESCSIZE)
#define NRXDACTIVE(h, t)    (RXD(t - h) / DESCSIZE)
#define IS_LAST_TXD(i)      (i == (NTXD - 1) * DESCSIZE)
#define IS_LAST_RXD(i)      (i == (NRXD - 1) * DESCSIZE)
#define GETTXD(di,off)      ((volatile dma64dd_t *)(((uint)di->txd64) | (off)))
#define GETRXD(di,off)      ((volatile dma64dd_t *)(((uint)di->rxd64) | (off)))
#define GETTXP(di,off)      (((pktwrap_t *)(((uint)di->txp) | (off)))->pkt)
#define GETRXP(di,off)      (((pktwrap_t *)(((uint)di->rxp) | (off)))->pkt)
#define RXFILL_PKTSIZE      (RXBUFSZ)

/*
 * Principle vx-specific, per-device software state.
 *
 * The END_OBJ must be the first field in this structure since
 * we overload entry point END_OBJ* with et_info_t* .
 */
typedef struct et_info {
    END_OBJ     endobj;     /* end object */
    etc_info_t  *etc;       /* pointer to common os-independent data */
    SEM_ID      mutex;      /* private lock */
    bool        netjobpend; /* netjob(et_intr) is pending */
    bool        shutdown;   /* et is going to shutdown */
    WDOG_ID     watchdogid; /* 1 sec et_watchdog timer id */
    bool        dogtimerset;    /* if true: watchdog is running */
    void        *descmem;   /* uncached dma descriptor memory */
    pciinfo_t   pciinfo;
    int     ivec;       /* interrupt vector */
    int     irq;        /* interrupt vector */

    struct mblkq    txq;        /* private transmit software queue */
    bool        txblocked;  /* TRUE if tx side flow controlled */

    long        flags;      /* our local flags. */
    M_CL_CONFIG mclconfig;  /* packet pool */
    CL_DESC     cldesc[1];  /* cluster buffer pool descriptors */
    struct et_info  *next;  /* point to next et_info_t in chain */
    
    /* PED */
    gmac0regs_t     *regs;      /* point to chip registers */
    dma_info_t      di[NUMRXQ];
    osl_t           *osh;
} et_info_t;

#define MAXUNITS    8
static et_info_t *et_global[MAXUNITS];
static et_info_t *et_list = NULL;

#define ONE_SECOND  sysClkRateGet()

#ifdef __i386__

/* perf logging */
#define rdtscl(low) \
    __asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")
#define rdmsr(msr, val1, val2) \
    __asm__ __volatile__("rdmsr" \
        : "=a" (val1), "=d" (val2) \
        : "c" (msr))
#define wrmsr(msr, val1, val2) \
    __asm__ __volatile__("wrmsr" \
        : /* no outputs */ \
        : "c" (msr), "a" (val1), "d" (val2))

#define INT_NUM(_a)     (_a)
#define INT_NUM_IRQ0        0x20
#define INT_ENABLE(_a)      sysIntEnablePIC(_a)

#elif __mips__

#include <vxbsp.h>

#define INT_NUM_IRQ0        0
#define INT_ENABLE(_a)      intEnable(_a)

#endif  /* __mips__ */

#define CLASS_NET_ETH       ((PCI_CLASS_NETWORK_CTLR << 16) | (PCI_SUBCLASS_NET_ETHERNET << 8))
#define DATAHIWAT           50

/* functions called by etc.c */
void et_init(et_info_t *et, uint options);
void et_reset(et_info_t *et);
void et_link_up(et_info_t *et);
void et_link_down(et_info_t *et);
void et_up(et_info_t *et);
void et_down(et_info_t *et, int reset);
void et_dump(et_info_t *et, struct bcmstrbuf *b);

/* local functions */
END_OBJ *et_load(char *initString, void *unused);
static int et_find_device(int unit, int *bus, int *dev, int *func,
    uint16 *vendorid, uint16 *deviceid);
static STATUS et_unload(et_info_t *et);
static STATUS et_netpool_alloc(et_info_t *et);
static void et_netpool_free(et_info_t *et);
static void et_free(et_info_t *et);
static void et_vxwatchdog(et_info_t *et);
static void et_vxintr(et_info_t *et);
static void et_sendnext(et_info_t *et);
static void et_watchdog(et_info_t *et);
static void et_sendup(et_info_t *et, struct mbuf *m);
static STATUS et_end_start(et_info_t *et);
static STATUS et_end_stop(et_info_t *et);
static int et_ioctl(et_info_t *et, int cmd, caddr_t data);
static STATUS et_send(et_info_t *et, M_BLK_ID pBuf);
static STATUS et_mcastAdd(et_info_t *et, char* pAddress);
static STATUS et_mcastDel(et_info_t *et, char* pAddress);
static STATUS et_mcastGet(et_info_t *et, MULTI_TABLE* pTable);
static void et_config(et_info_t *et);
static void et_enq(struct mblkq *q, M_BLK_ID m);
static M_BLK_ID et_deq(struct mblkq *q);
#ifdef BCMDBG
static void et_dumpet(et_info_t *et, struct bcmstrbuf *b);
#endif /* BCMDBG */

static NET_FUNCS etFuncTable = {
    (FUNCPTR) et_end_start,     /* function to start the device. */
    (FUNCPTR) et_end_stop,      /* function to stop the device. */
    (FUNCPTR) et_unload,        /* unloading function for the driver. */
    (FUNCPTR) et_ioctl,     /* ioctl function for the driver. */
    (FUNCPTR) et_send,      /* send function for the driver. */
    (FUNCPTR) et_mcastAdd,      /* multicast add function for the driver. */
    (FUNCPTR) et_mcastDel,      /* multicast delete function for the driver. */
    (FUNCPTR) et_mcastGet,      /* multicast retrieve function for the driver. */
    (FUNCPTR) NULL,         /* polling send function -  not supported */
    (FUNCPTR) NULL,         /* polling receive function  -  not supported */
    endEtherAddressForm,        /* put address info into a NET_BUFFER */
    endEtherPacketDataGet,      /* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet       /* get packet addresses. */
};

#define ET_LOCK(et)         semTake((et)->mutex, WAIT_FOREVER)
#define ET_UNLOCK(et)       semGive((et)->mutex)

/*
 * vxWorks 5.x does not directly support shared interrupts.  Unbelievable.
 * Redefine intConnect().
 */
#ifndef __mips__
#if defined(INCLUDE_IL_END) || defined(INCLUDE_IL)
extern STATUS cmn_int_connect(VOIDFUNCPTR *ivec, VOIDFUNCPTR dev_vxintr, int param);
#define intConnect  cmn_int_connect
#endif /* defined(INCLUDE_IL_END) || defined(INCLUDE_IL) */
#endif /* __mips__ */

/* PED */
static void
et_dma_alloc(dma_info_t *di)
{
    void *va;
    
    va = cacheDmaMalloc(sizeof(dma64dd_t) * NTXD + D64RINGALIGN);
    di->txd64 = (dma64dd_t *)ROUNDUP((uintptr)va, D64RINGALIGN);
    di->txdpa = (ulong)CACHE_DMA_VIRT_TO_PHYS(di->txd64);
    bzero(di->txd64, sizeof(dma64dd_t) * NTXD);

    va = cacheDmaMalloc(sizeof(dma64dd_t) * NRXD + D64RINGALIGN);
    di->rxd64 = (dma64dd_t *)ROUNDUP((uintptr)va, D64RINGALIGN);
    di->rxdpa = (ulong)CACHE_DMA_VIRT_TO_PHYS(di->rxd64);
    bzero(di->rxd64, sizeof(dma64dd_t) * NRXD);
    
    va = MALLOC(NULL, sizeof(pktwrap_t) * NTXD + D64RINGALIGN);
    di->txp = (pktwrap_t *)ROUNDUP((uintptr)va, D64RINGALIGN);
    bzero(di->txp, sizeof(pktwrap_t) * NTXD);
    
    va = MALLOC(NULL, sizeof(pktwrap_t) * NRXD + D64RINGALIGN);
    di->rxp = (pktwrap_t *)ROUNDUP((uintptr)va, D64RINGALIGN);
    bzero(di->rxp, sizeof(pktwrap_t) * NRXD);
}

END_OBJ*
et_load(char *initString, void *unused)
{
    et_info_t *et;
    char *tok;
    int unit;
    uint16 vendorid, deviceid;
    uint32 regaddr;
    void *regptr;
    char irq;
    int bus, dev, func;
    osl_t *osh;
    int gmac_int = 0;


    et = NULL;
    bus = dev = func = 0;
    regaddr = 0;
    irq = 0;

    if (initString == NULL)
        return (NULL);

    /* end driver first pass init */
    if (initString[0] == '\0') {
        bcopy("et", initString, 3);
        return (NULL);
    }

    /* get our unit number from the init string */
    if ((tok = strtok(initString, ":")) == NULL) {
        ET_ERROR(("et_load: garbled initString (%s) - unit number not found\n",
            initString));
        goto fail;
    }
    unit = bcm_atoi(tok);

    ET_TRACE(("et%d: et_load\n", unit));

    if (et_find_device(unit, &bus, &dev, &func, &vendorid, &deviceid) == ERROR) {
        ET_ERROR(("et%d: et_load: et_find_device() failed \n", unit));
        goto fail;
    }

    pciConfigInLong(bus, dev, func, PCI_CFG_BASE_ADDRESS_0, &regaddr);
    pciConfigInByte(bus, dev, func, PCI_CFG_DEV_INT_LINE, &irq);

    /* map chip registers */
    regptr = REG_MAP(regaddr, 0);

    /* allocate our private device structure */
    if ((et = (et_info_t*) MALLOC(NULL, sizeof(et_info_t))) == NULL) {
        ET_ERROR(("et%d: et_load: malloc et_info_t failed!\n", unit));
        goto fail;
    }

    bzero((char*) et, sizeof(et_info_t));

#if 0 
      /* Sould consist with the configuration in si_mips_init()@hndmips.c */
        gmac_int = IV_IORQ2_VEC;
#else 
    gmac_int = irq;
#endif

    et->irq = gmac_int;
    et->ivec = INUM_TO_IVEC(gmac_int);

    et->pciinfo.bus = bus;
    et->pciinfo.dev = dev;
    et->pciinfo.func = func;

    osh = osl_attach((void*)&et->pciinfo, &et->endobj, FALSE);

    /* PED */
    et->osh = osh;

    ET_TRACE(("et%d: et_load: regptr %p ivec %d irq %d\n",
        unit, regptr, et->ivec, et->irq));

    /* initialize private semaphore */
    if ((et->mutex = semBCreate(SEM_Q_FIFO, SEM_FULL)) == NULL) {
        ET_ERROR(("et%d: et_load: semBCreate fail\n", unit));
        goto fail;
    }

    /* PED */
    if (unit == 0) {
        et->regs = (gmac0regs_t *)0xb8002000;
    } else {
        et->regs = (gmac0regs_t *)0xb800d000;
    }

    /* PED */
    if (et_list == NULL) {
        PKTPOLL_INIT_SHUNT();
    }

    /* common load-time initialization */
    if ((et->etc = etc_attach((void*)et, vendorid, deviceid, unit, osh,
        regptr)) == NULL) {
        ET_ERROR(("et%d: etc_attach failed\n", unit));
        goto fail;
    }

    /* create a watchdog timer */
    if ((et->watchdogid = wdCreate()) == NULL) {
        ET_ERROR(("et%d: error creating watchdog timer\n", unit));
        goto fail;
    }
    et->dogtimerset = FALSE;

    /* initialize the END and MIB2 parts of the structure */
    if (END_OBJ_INIT(&et->endobj, (DEV_OBJ*)et, "et", unit, &etFuncTable, "END et Driver")
        == ERROR)
        goto fail;

    if (END_MIB_INIT(&et->endobj, M2_ifType_ethernet_csmacd, (char *)&et->etc->cur_etheraddr,
        ETHER_ADDR_LEN, ETHERMTU, 100000000) == ERROR)
        goto fail;

    /* allocate and initialize our private mblk/clblk netpool */
    if (et_netpool_alloc(et) == ERROR)
        goto fail;

    /* set the flags to indicate readiness */
    END_OBJ_READY(&et->endobj, (IFF_NOTRAILERS|IFF_BROADCAST|IFF_MULTICAST));

    /* register our interrupt handler */
    if (intConnect((VOIDFUNCPTR*)et->ivec, (VOIDFUNCPTR)et_vxintr, (int)et) == ERROR) {
        ET_ERROR(("et%d: et_load: interrupt connect error\n", unit));
        goto fail;
    }

    et_global[unit] = et;

    /* add us to the global linked list */
    et->next = et_list;
    et_list = et;

    /* PED: peer init for SHUNT */
    if (et->next != NULL) {
        et->di[0].peer = &et->next->di[0];
        et->next->di[0].peer = &et->di[0];
        et->di[1].peer = &et->next->di[1];
        et->next->di[1].peer = &et->di[1];
    }
    
    return (&et->endobj);

fail:
    et_free(et);
    return (NULL);
}

/*
 * et_unload - unload a driver from the system
 */
static STATUS
et_unload(et_info_t *et)
{
    ET_TRACE(("et%d: et_unload\n", et->etc->unit));

    ASSERT(!et->etc->up);

    END_OBJECT_UNLOAD(&et->endobj);

    et_global[et->etc->unit] = NULL;

    et_free(et);

    return (OK);
}

static int
et_find_device(int unit, int *bus, int *dev, int *func, uint16 *vendorid, uint16 *deviceid)
{
    int found;
    int i;

    found = 0;

    /* find the unit'th instance of a recognized et device */
    for (i = 0; i < 32; i++) {
        if (pciFindClass(CLASS_NET_ETH, i, bus, dev, func) != OK)
            continue;
        if (pciConfigInWord(*bus, *dev, *func, 0, vendorid) != OK)
            continue;
        if (pciConfigInWord(*bus, *dev, *func, 2, deviceid) != OK)
            continue;
        if (!etc_chipmatch(*vendorid, *deviceid))
            continue;
#if 0 
        if ((unit == 0) && (found == 1)) return (OK);
        if ((unit == 1) && (found == 0)) return (OK);
#else
    
        if (found == unit)
            return (OK);
#endif
        found++;
    }

    ET_ERROR(("et%d: et_find_device:  error finding device\n", unit));

    return (ERROR);
}

static void
et_free(et_info_t *et)
{
    et_info_t **prev;

    if (et == NULL)
        return;

    ET_TRACE(("et: et_free\n"));

    /* delete the watchdog timer */
    if (et->watchdogid) {
        wdDelete(et->watchdogid);
        et->watchdogid = NULL;
    }

    /* free common resources */
    if (et->etc) {
        etc_detach(et->etc);
        et->etc = NULL;
    }

    /* free private netpool */
    et_netpool_free(et);

    /* remove us from the global linked list */
    for (prev = &et_list; *prev; prev = &(*prev)->next)
        if (*prev == et) {
            *prev = et->next;
            break;
        }

    MFREE(NULL, (char*)et, sizeof(et_info_t));
}

/* alloc and init a private mblk/clblk netpool */
STATUS
et_netpool_alloc(et_info_t *et)
{
    ET_TRACE(("et%d: et_netpool_alloc\n", et->etc->unit));

    /* alloc the netpool structure itself */
    if ((et->endobj.pNetPool = (NET_POOL_ID) MALLOC(NULL, sizeof(NET_POOL))) == NULL)
        goto error;

    et->mclconfig.mBlkNum = MIN_PKTS;
    et->mclconfig.clBlkNum = MIN_PKTS;

    /* sanity check */
    ASSERT(BUFSZ == VXCLSIZE);

    /* create a single 2Kbyte cluster buffer pool */
    et->cldesc[0].clNum = MIN_CLUSTERS;
    et->cldesc[0].clSize = VXCLSIZE;

    /* calculate the total memory for all the M_BLKs and CL_BLKs */
    et->mclconfig.memSize = 
#if PED_USE_LINK_BUFFER_POOL /* PED: shortcut netTupleGet */
        (et->mclconfig.mBlkNum * ROUND_UP((sizeof(M_LINK) + sizeof(NET_POOL_ID)), NETBUF_ALIGN)) + NETBUF_ALIGN;
#else 
        (et->mclconfig.mBlkNum * (MSIZE + sizeof(long))) + (et->mclconfig.clBlkNum * (CL_BLK_SZ + sizeof(long)));
#endif

    /* allocate the M_BLK&CL_BLK pool */
    if ((et->mclconfig.memArea = MALLOC(NULL, et->mclconfig.memSize)) == NULL)
        goto error;

    /* calculate the memory size of all the clusters. */
    et->cldesc[0].memSize = 
#if PED_USE_LINK_BUFFER_POOL /* PED: shortcut netTupleGet */
        (et->cldesc[0].clNum * ROUND_UP(et->cldesc[0].clSize, NETBUF_ALIGN)) + NETBUF_ALIGN;
#else
        (et->cldesc[0].clNum * (VXCLSIZE + 8)) + sizeof(int);
#endif

    /* allocate the cluster buffer pool */
#if PED_USE_UNCACHED_BUFFER /* PED: uncached */    
    if ((et->cldesc[0].memArea = (void *)(((uint)memalign(2048, et->cldesc[0].memSize)) | 0xa0000000)) == NULL)
#else
    if ((et->cldesc[0].memArea = memalign(2048, et->cldesc[0].memSize)) == NULL)
#endif
        goto error;
    ASSERT(ISALIGNED((uintptr)et->cldesc[0].memArea, 2048));

    /* now init the netpool */
    if (netPoolInit(et->endobj.pNetPool, &et->mclconfig, &et->cldesc[0], 1, 
#if PED_USE_LINK_BUFFER_POOL /* PED: shortcut netTupleGet */
        _pLinkPoolFuncTbl
#else
        NULL
#endif
        ) == ERROR)
        goto error;

    return (OK);

error:
    ET_ERROR(("et%d: et_netpool_alloc: malloc failed!\n", et->etc->unit));
    et_netpool_free(et);
    return (ERROR);
}

static void
et_netpool_free(et_info_t *et)
{
    ET_TRACE(("et%d: et_netpool_free\n", et->etc->unit));

    if (et->cldesc[0].memArea) {
        MFREE(NULL, et->cldesc[0].memArea, et->cldesc[0].memSize);
        et->cldesc[0].memSize = 0;
        et->cldesc[0].memArea = NULL;
    }

    if (et->mclconfig.memArea) {
        MFREE(NULL, et->mclconfig.memArea, et->mclconfig.memSize);
        et->mclconfig.memSize = 0;
        et->mclconfig.memArea = NULL;
    }

    if (et->endobj.pNetPool) {
        netPoolDelete(et->endobj.pNetPool);
        et->endobj.pNetPool = NULL;
    }
}

void
et_init(et_info_t *et, uint options)
{
    ET_TRACE(("et%d: et_init\n", et->etc->unit));

    /* reset the interface */
    et_reset(et);

    /* initialize the driver and chip */
    etc_init(et->etc, options);

    et->endobj.flags |= (IFF_RUNNING | IFF_UP);
}

void
et_reset(et_info_t *et)
{
    ET_TRACE(("et%d: et_reset\n", et->etc->unit));

    /* common reset code */
    etc_reset(et->etc);

    et->endobj.flags &= ~IFF_RUNNING;
}

void
et_link_up(et_info_t *et)
{
    et->endobj.mib2Tbl.ifSpeed = 100000000;
    ET_ERROR(("et%d: link up (%d%s)\n",
        et->etc->unit, et->etc->speed, (et->etc->duplex? "FD" : "HD")));
}

void
et_link_down(et_info_t *et)
{
    et->endobj.mib2Tbl.ifSpeed = 0;
    ET_ERROR(("et%d: link down\n", et->etc->unit));
}

void
et_up(et_info_t *et)
{
    ET_TRACE(("et%d: et_up\n", et->etc->unit));

    /* Calling et_up when it's already up means to re-initialize (re-config) */
    if (et->etc->up) {
        et_down(et, 0);
    }

    etc_up(et->etc);

    END_FLAGS_SET(&et->endobj, IFF_UP | IFF_RUNNING);

    /* start a one second the watchdog timer */
    if (et->dogtimerset == FALSE) {
        if (wdStart(et->watchdogid, ONE_SECOND, (FUNCPTR)et_vxwatchdog, (int)et) != OK) {
            ET_ERROR(("et%d: et_up: Error starting the watchdog\n", et->etc->unit));
        }
        et->dogtimerset = TRUE;
    }

}

void
et_down(et_info_t *et, int reset)
{
    etc_info_t *etc;
    M_BLK_ID m;

    etc = et->etc;

    ET_TRACE(("et%d: et_down\n", etc->unit));

    /* cancel the watchdog timer */
    if (et->dogtimerset) {
        wdCancel(et->watchdogid);
        et->dogtimerset = FALSE;
    }

    /* 
     * Shutdown gracefully: wait for outstanding tasks to be processed.
     */
    if (et->etc->up) {
        
        /* PED */
        WR_REG(NULL, &et->regs->intmask, 0);
        (void)RD_REG(NULL, &et->regs->intmask);  /* sync readback */
        WR_REG(NULL, &et->regs->intstatus, 0xFF);
        
        taskDelay(1);
        et->shutdown = TRUE;
        while (et->netjobpend) {
            ET_UNLOCK(et);
            taskDelay(1);
            ET_LOCK(et);
        }
        et->shutdown = FALSE;
        et->netjobpend = FALSE;
    }

    etc_down(etc, reset);

    /* mark the driver as down */
    END_FLAGS_CLR(&et->endobj, IFF_UP | IFF_RUNNING);

    /* flush the txq */
    while ((m = (M_BLK_ID) et_deq(&et->txq)))
        netMblkClChainFree(m);
}

void
et_dump(et_info_t *et, struct bcmstrbuf *b)
{
    bcm_bprintf(b, "et%d: %s %s version %s\n", et->etc->unit,
        __DATE__, __TIME__, EPI_VERSION_STR);

#ifdef BCMDBG
    et_dumpet(et, b);
    etc_dump(et->etc, b);
#endif /* BCMDBG */
}

void
et_intrson(et_info_t *et)
{
    WR_REG(NULL, &et->regs->intmask, DEF_INTMASK);
}

static void
et_vxwatchdog(et_info_t *et)
{
    netJobAdd((FUNCPTR)et_watchdog, (int)et, 0, 0, 0, 0);
}

static void
et_watchdog(et_info_t *et)
{
    ET_TRACE(("et%d: watchdog\n", et->etc->unit));

    ET_LOCK(et);

    etc_watchdog(et->etc);

    /* restart timer */
    wdStart(et->watchdogid, ONE_SECOND, (FUNCPTR)et_vxwatchdog, (int)et);

    ET_UNLOCK(et);
}

/* PED */
#define _MBLK_SINGLE 0
#define _MBLK_START 1
#define _MBLK_END 2
#define _MBLK_OTHERS 3

static INLINE void
_do_tx(dma_info_t *di, M_BLK_ID m, int type)
{
    uint flags;
    uint txout = di->txout;
    volatile dma64dd_t *dd;
    uint len;
    uint32 pa;

    /* check tx descriptor */
    if (di->txin == NEXTTXD(txout)) {
        PKTFREE(di->osh, m, FALSE, MEMORY_DDRRAM);
        return;
    }
    
    /* Fill TX descriptor */
    if (type == _MBLK_START) {
        flags = D64_CTRL1_SOF;
    } else if (type == _MBLK_END) {
        flags = D64_CTRL1_EOF | D64_CTRL1_IOC;
    } else if (type == _MBLK_SINGLE) {
        flags = D64_CTRL1_SOF | D64_CTRL1_EOF | D64_CTRL1_IOC;
    } else {
        flags = 0;
    }
    if (IS_LAST_TXD(txout)) {
        flags |= D64_CTRL1_EOT;
    }
    dd = GETTXD(di, txout);

    len = PKTLEN(di->osh, m);

    pa = (uint32)DMA_MAP(di->osh, PKTDATA(di->osh, m), len, DMA_TX, m, NULL);
    dd->addrlow = BUS_SWAP32(pa);
    dd->ctrl1 = BUS_SWAP32(flags);
    dd->ctrl2 = BUS_SWAP32(len);
    
    /* Save the packet pointer for reclaiming later */
    GETTXP(di, txout) = m;
    
    /* Advance txout */
    txout = NEXTTXD(txout);
    
    /* Kick off TX */
    if ((type == _MBLK_END) || (type == _MBLK_SINGLE)) {
        WR_REG(NULL, &di->d64txregs->ptr, txout);
    }
    
    /* Store current txout */
    di->txout = txout;
}

static void
et_sendnext(et_info_t *et)
{
    M_BLK_ID m;
    etc_info_t *etc;
    int blk_count = 0;
    M_BLK_ID p;
    M_BLK_ID p_next;

    etc = et->etc;

    ET_TRACE(("et%d: et_sendnext\n", etc->unit));
    ET_LOG("et%d: et_sendnext", etc->unit, 0);

    /* dequeue and send each packet */
    while (
           (etc->pioactive == NULL) &&
           (m = et_deq(&et->txq))) {

        ET_PRHDR("tx", (struct ether_header*) m->mBlkHdr.mData, etc_totlen(etc, (void*)m),
                 etc->unit);
        ET_PRPKT("txpkt", m->mBlkHdr.mData, m->mBlkHdr.mLen, etc->unit);
        /* PED: transmit the frame */
        p = m;
        blk_count = 0;
        for(; p; ) {
            p_next = PKTNEXT(etc->osh, p);
            if (p_next) {
                if (!blk_count) {
                    _do_tx(&et->di[0], p, _MBLK_START);
                } else {
                    _do_tx(&et->di[0], p, _MBLK_OTHERS);
                }
            } else {
                if (!blk_count) {
                    _do_tx(&et->di[0], p, _MBLK_SINGLE);
                } else {
                    _do_tx(&et->di[0], p, _MBLK_END);
                }
            } 
            blk_count++;
            p = p_next;
        }

        etc->txframe++;
        etc->txbyte += m->mBlkHdr.mLen;
    }

    ET_LOG("et%d: et_sendnext ret\n", etc->unit, 0);
}

/* PED */
static INLINE void
_do_rx(dma_info_t *di, et_info_t *et)
{
    /* RX variables */
    uint rxin = di->rxin;

    /* variables used in loop (declaring them globally is better) */
    void *rxp = NULL;
    uchar *header;
    uint len;

    for(;;) {

        if (rxin == (RD_REG(NULL, &di->d64rxregs->status0) & D64_RS0_CD_MASK)) {

            /* No packet available */
            break;

        } else {

            /* Get received packet */
            rxp = GETRXP(di, rxin);
            
            /* Advance rxin */    
            rxin = NEXTRXD(rxin);
            
            /* Get RX header */
            header = PKTDATA(di->osh, rxp);
            
            /* Check if any of error bits set */
            if (((bcmgmacrxh_t *)header)->flags & (GRXF_OVF | GRXF_OVERSIZE | GRXF_CRC)) {
                /* Error bit set, packet damaged */
                PKTFREE(di->osh, rxp, FALSE, MEMORY_DDRRAM);
                continue;
            }

            /* Get length (skip CRC and rxheader) */
            len = htol16(*(uint16 *)header) - 4; /* 4: CRC */
            PKTSETLEN(di->osh, rxp, len); 
            
            /* Strip off rxheader */
            ((M_BLK_ID)rxp)->mBlkHdr.mData += HWRXOFF;

            /* Send up to vxWorks */
            et_sendup(et, (M_BLK_ID)rxp);

        }
    }
    
    /* Do RX refill if any packets received */
    if (rxp) {
        
        /* Store current rxin */
        di->rxin = rxin;
        
        /* rxfill */
        {
            uint rxout;
            uint32 pa;
            volatile dma64dd_t *dd;
            
            rxout = di->rxout;
            for(len = NRXBUFPOST - NRXDACTIVE(rxin, rxout); len; len--) {
                rxp = PKTGET(di->osh, RXFILL_PKTSIZE, FALSE, MEMORY_DDRRAM);
                GETRXP(di, rxout) = rxp;
                dd = GETRXD(di, rxout);
                pa = (uint32)DMA_MAP(di->osh, PKTDATA(di->osh, rxp), RXBUFSZ, DMA_RX, rxp, NULL);
                dd->addrlow = BUS_SWAP32(pa);
                dd->ctrl1 = BUS_SWAP32(IS_LAST_RXD(rxout)? D64_CTRL1_EOT : 0);
                dd->ctrl2 = BUS_SWAP32(RXBUFSZ);
                rxout = NEXTRXD(rxout);
            }
            WR_REG(NULL, &di->d64rxregs->ptr, rxout);
            di->rxout = rxout;
        }
    }
}

/* PED */
static INLINE void
_do_shunt(dma_info_t *di)
{
    /* RX variables */
    uint rxin = di->rxin;

    /* TX variables */
    uint flags;
    dma_info_t *peer = di->peer;
    uint txout = peer->txout;
    volatile dma64dd_t *dd = NULL; /* Also for indicating packet received */
    
    /* variables used in loop (declaring them globally is better) */
    void *rxp;
    uchar *header;
    uint len;
    
    for(;;) {

        if (rxin == (RD_REG(NULL, &di->d64rxregs->status0) & D64_RS0_CD_MASK)) {

            /* No packet available */
            break;

        } else {
            
            /* Get received packet */
            rxp = GETRXP(di, rxin);
            
            /* Advance rxin */    
            rxin = NEXTRXD(rxin);
            
            /* Get RX header */
            header = PKTDATA_SHUNT(NULL, rxp);
            
            /* Check if any of error bits set */
            if (((bcmgmacrxh_t *)header)->flags & (GRXF_OVF | GRXF_OVERSIZE | GRXF_CRC)) {
                /* Error bit set, packet damaged */
                PKTFREE_SHUNT(NULL, rxp, FALSE, MEMORY_SOCRAM);
                continue;
            }
            
            /* Get length */
            len = htol16(*(uint16 *)header) - 4; /* 4: CRC */
            
            /* Fill TX descriptor */
            flags = D64_CTRL1_SOF | D64_CTRL1_EOF;
            if (IS_LAST_TXD(txout)) {
                flags |= D64_CTRL1_EOT;
            }
            dd = GETTXD(peer, txout);
            dd->addrlow = BUS_SWAP32((uint32)PKTBODYP_SHUNT(rxp));
            dd->ctrl1 = BUS_SWAP32(flags);
            dd->ctrl2 = BUS_SWAP32(len);
            
            /* Save the packet pointer for reclaiming later */
            GETTXP(peer, txout) = rxp;
            
            /* Advance txout */
            txout = NEXTTXD(txout);
        }
    }
    
    /* Kick off TX and do RX refill if any packets received */
    if (dd) {
        
        /* Enable TX interrupt for the last packet */
        dd->ctrl1 = BUS_SWAP32(flags | D64_CTRL1_IOC);
        
        /* Kick off TX */
        WR_REG(NULL, &peer->d64txregs->ptr, txout);
        
        /* Store current txout */
        peer->txout = txout;
        
        /* Store current rxin */
        di->rxin = rxin;
        
        /* rxfill */
        {
            uint rxout;
            
            rxout = di->rxout;
            for(len = NRXBUFPOST - NRXDACTIVE(rxin, rxout); len; len--) {
                rxp = PKTGET_SHUNT(NULL, BUFSZ, FALSE, MEMORY_SOCRAM);
                GETRXP(di, rxout) = rxp;
                dd = GETRXD(di, rxout);
                dd->addrlow = BUS_SWAP32((uint32)PKTDATAP_SHUNT(rxp));
                dd->ctrl1 = BUS_SWAP32(IS_LAST_RXD(rxout)? D64_CTRL1_EOT : 0);
                dd->ctrl2 = BUS_SWAP32(RXBUFSZ);
                rxout = NEXTRXD(rxout);
            }
            WR_REG(NULL, &di->d64rxregs->ptr, rxout);
            di->rxout = rxout;
        }
    }
}

/* PED */
static INLINE void 
_txreclaim(dma_info_t *di)
{
    uint start, end;
    void *txp;
    
    end = RD_REG(NULL, &di->d64txregs->status0) & D64_XS0_CD_MASK;
    for(start = di->txin; start != end; start = NEXTTXD(start)) {
        txp = GETTXP(di, start);
        PKTFREE(di->osh, txp, FALSE, MEMORY_DDRRAM);
    }
    di->txin = start;
}

/* PED */
static INLINE void 
_txreclaim_shunt(dma_info_t *di)
{
    uint start, end;
    void *txp;

    end = RD_REG(NULL, &di->d64txregs->status0) & D64_XS0_CD_MASK;
    for(start = di->txin; start != end; start = NEXTTXD(start)) {
        txp = GETTXP(di, start);
        PKTFREE_SHUNT(NULL, txp, FALSE, MEMORY_SOCRAM);
    }
    di->txin = start;
}

/* PED */
#if PED_ALL_SHUNT
#define CH0_RX(di, et)  _do_shunt(di)    
#define CH0_TX(di)      _txreclaim_shunt(di)
#else /* !PED_ALL_SHUNT */
#define CH0_RX(di, et)  _do_rx(di, et)
#define CH0_TX(di)      _txreclaim(di)
#endif  /* PED_ALL_SHUNT */

/* PED: completely revised */
static void
et_intr(et_info_t *et, uint events)
{
    dma_info_t *di;

    ET_LOCK(et);

    /* Channel 0 */
    {
        di = &et->di[0];
        if (events & INTMASK_RCVINTERRUPT_0_MASK) {
            /* Packets received; process them and refill packets */
            CH0_RX(di, et);
        }
        if (events & INTMASK_XMT0INTERRUPT_MASK) {
            /* Transmit done; do txreclaim */
            CH0_TX(di);
        }
    }

#if !PED_ALL_SHUNT
    /* Channel 1 */
    {
        di = &et->di[1];
        if (events & INTMASK_RCVINTERRUPT_1_MASK) {
            /* Packets received; forward them and refill packets */
            _do_shunt(di);
        }
        if (events & INTMASK_XMT1INTERRUPT_MASK) {
            /* Transmit done; do txreclaim */
            _txreclaim_shunt(di);
        }
    }
#endif

    if (events & I_ERRORS) {
            et_init(et, ET_INIT_INTROFF);
    }

#if !PED_ALL_SHUNT
    /* run the tx queue */
    if (et->txq.len)
        et_sendnext(et);

    /* if tx pkts had been blocked but should no longer be */
    if (et->txblocked && (et->txq.len <= DATAHIWAT)) {
        et->txblocked = FALSE;
        netJobAdd((FUNCPTR)muxTxRestart, (int)&et->endobj, 0, 0, 0, 0);
    }
#endif

    et->netjobpend = FALSE;

    /* If et is going to shutdown, don't enable interrupts */
    if (!et->shutdown) {
        /* re-enable interrupts */
        WR_REG(NULL, &et->regs->intmask, DEF_INTMASK);
    }

    ET_UNLOCK(et);
}

/* just a wrapper for et_intr() */
static void
et_vxintr(et_info_t *et)
{
    uint events;

    /* guard against shared interrupts */
    if (!et->etc->up) {
        return;
    }

    /* PED */
    events = RD_REG(NULL, &et->regs->intstatus) & DEF_INTMASK;
    if (events == 0 || et->netjobpend) {
        return;
    }

    if (netJobAdd((FUNCPTR)et_intr, (int)et, events, 0, 0, 0) == OK) {
        /* disable device interrupts */

        /* PED */
        WR_REG(NULL, &et->regs->intmask, 0);
        WR_REG(NULL, &et->regs->intstatus, events);

        et->netjobpend = TRUE;
    }
}

static void
et_sendup(et_info_t *et, M_BLK_ID m)
{


    ET_TRACE(("et%d: et_sendup: %d bytes\n", et->etc->unit, m->mBlkHdr.mLen));
    ET_LOG("et%d: et_sendup: len %d", et->etc->unit, m->mBlkHdr.mLen);

    et->etc->rxframe++;
    et->etc->rxbyte += m->m_len;

    /* eh should now be aligned 2-mod-4 */
    ASSERT(((uint)m->mBlkHdr.mData & 3) == 2);


    ET_PRHDR("rx", (struct ether_header*)m->mBlkHdr.mData, m->mBlkHdr.mLen, et->etc->unit);
    ET_PRPKT("rxpkt", (uchar*)m->mBlkHdr.mData, m->mBlkHdr.mLen, et->etc->unit);

    /* Ensure that default netBufLib routines are used to allocate and
     * free netBufLib structures. This is because we use the clFreeArg3
     * field in the CL_BLK to store out-of-band QOS info and the netBufLib
     * allocate and free routines don't use this field
     */
    ASSERT(M_HASCL(m));
    ASSERT((m->pClBlk->pClFreeRtn == NULL) && (m->pClBlk->clFreeArg3 == 0));

    /* Extract priority from payload and store it out-of-band in the clFreeArg3
     * field of the CL_BLK
     */
    if (et->etc->qos)
        pktsetprio(m, FALSE);


    m->mBlkHdr.mFlags |= M_PKTHDR;
    m->mBlkPktHdr.len = m->mBlkHdr.mLen;

    /* The vx router 1.0 stack is reentrant so we
     * need to drop the perimeter lock here
     */
    ET_UNLOCK(et);

    END_RCV_RTN_CALL(&et->endobj, m);

    ET_LOCK(et);

    ET_TRACE(("et%d: et_sendup ret\n", et->etc->unit, 0));
    ET_LOG("et%d: et_sendup ret", et->etc->unit, 0);

    return;

}

#ifdef BCMDBG
static void
et_dumpet(et_info_t *et, struct bcmstrbuf *b)
{
    bcm_bprintf(b, "et %p flags 0x%x txq.len %d\n",
        et, (uint)et->flags, et->txq.len);
    bcm_bprintf(b, "\n");
}
#endif /* BCMDBG */

/*
 * et_end_start - start the device
 */
static STATUS
et_end_start(et_info_t *et)
{
    ET_TRACE(("et%d: et_end_start\n", et->etc->unit));
    ET_LOCK(et);

    /* clear ifSpeed to mark link down */
    et->endobj.mib2Tbl.ifSpeed = 0;

    et_up(et);

#ifndef MSI
    INT_ENABLE(et->irq);
#endif /* MSI */

    ET_UNLOCK(et);
    return (OK);
}

/*
 * et_end_stop - stop the device
 */
static STATUS
et_end_stop(et_info_t *et)
{
    ET_TRACE(("et%d: et_end_stop\n", et->etc->unit));
    ET_LOCK(et);
    et_down(et, 1);
    ET_UNLOCK(et);
    return (OK);
}

/*
 * et_send - the driver send routine
 */
/* PED: rename for ping from both interfaces */
static STATUS
et_send_ext(et_info_t *et, M_BLK_ID m)
{
    ET_TRACE(("et%d: et_send: len %d \n", et->etc->unit, m->mBlkPktHdr.len));

    END_TX_SEM_TAKE(&et->endobj, WAIT_FOREVER);
    ET_LOCK(et);

    /* discard if transmit queue is too full */
    if (et->txq.len > DATAHIWAT)
        goto qfull;

    et_enq(&et->txq, m);
    et_sendnext(et);

    ET_UNLOCK(et);
    END_TX_SEM_GIVE(&et->endobj);

    /* bump the statistic counter. */
    END_ERR_ADD(&et->endobj, MIB2_OUT_UCAST, +1);

    return (OK);

qfull:
    ET_ERROR(("et%d: et_send: txq full\n", et->etc->unit));
    et->etc->txnobuf++;
    et->etc->txerror++;
    et->txblocked = TRUE;
    ET_UNLOCK(et);
    END_TX_SEM_GIVE(&et->endobj);
    return (END_ERR_BLOCK);
}

/* PED: Override for ping from both interfaces */
static STATUS
et_send(et_info_t *et, M_BLK_ID m)
{
#if !PED_ALL_SHUNT
    et_send_ext(et,m);
    return OK;
#else /* PED_ALL_SHUNT */
    return OK;
#endif /* !PED_ALL_SHUNT */
}

static int
et_ioctl(et_info_t *et, int cmd, caddr_t data)
{
    int error;
    long value;
    etc_info_t  *etc;

    etc = et->etc;
    error = 0;

    ET_TRACE(("et%d: et_ioctl\n", et->etc->unit));

    switch (cmd) {
    case EIOCSADDR:
        if (data == NULL)
            return (EINVAL);
        bcopy((char*)data, (char*)et->endobj.mib2Tbl.ifPhysAddress.phyAddress,
              ETHER_ADDR_LEN);
        bcopy((char*)data, (char*)&etc->cur_etheraddr, ETHER_ADDR_LEN);
        et_config(et);
        break;

    case EIOCGADDR:
        if (data == NULL)
            return (EINVAL);
        bcopy((char*)&etc->cur_etheraddr, (char *)data, ETHER_ADDR_LEN);
        break;

    case EIOCSFLAGS:
        value = (long)data;
        if (value < 0) {
            value = ~value;
            END_FLAGS_CLR(&et->endobj, value);
        }
        else {
            END_FLAGS_SET(&et->endobj, value);
        }
        et_config(et);
        break;

    case EIOCGFLAGS:
        *(int*)data = END_FLAGS_GET(&et->endobj);
        break;

    case EIOCPOLLSTART:
        ASSERT(0);  /* not supported */
        break;

    case EIOCPOLLSTOP:
        ASSERT(0);  /* not supported */
        break;

    case EIOCGMIB2:     /* return MIB information */
        if (data == NULL)
            return (EINVAL);

    /* update some of the mib counters */
    et->endobj.mib2Tbl.ifInErrors = etc->rxerror;
    et->endobj.mib2Tbl.ifOutErrors = etc->txerror;
    et->endobj.mib2Tbl.ifInOctets = etc->rxbyte;
    et->endobj.mib2Tbl.ifOutOctets = etc->txbyte;
    et->endobj.mib2Tbl.ifInUcastPkts = etc->rxframe;
    et->endobj.mib2Tbl.ifOutUcastPkts = etc->txframe;
        bcopy((char *)&et->endobj.mib2Tbl, (char *)data,
            sizeof(et->endobj.mib2Tbl));
        break;

    case EIOCGFBUF:     /* return minimum First Buffer for chaining */
        if (data == NULL)
            return (EINVAL);
        *(int *)data = ETHER_MAX_LEN;
        break;

    case EIOCQOS:
        ET_TRACE(("et%d: EIOCQOS: Value = %d\n", et->etc->unit, (int) data));
        etc_qos(etc, (uint) data);
        break;

    default:
        error = EINVAL;
    }

    return (error);
}

/*
 * et_config - Reconfigure the interface to match the current flag/multicast settings
 */
static void
et_config(et_info_t *et)
{
    int i;
    ETHER_MULTI *pCurr;
    uchar *src, *dst;

    ET_TRACE(("et%d: et_config\n", et->etc->unit));

    ET_LOCK(et);

    et->etc->allmulti = (et->endobj.flags & IFF_ALLMULTI)? TRUE: FALSE;

    /* set up address filter for multicasting */
    pCurr = END_MULTI_LST_FIRST(&et->endobj);
    i = 0;
    while (pCurr != NULL) {
        src = (unsigned char *)&pCurr->addr;
        dst = (unsigned char *)&et->etc->multicast[i];
        bcopy(src, dst, ETHER_ADDR_LEN);

        pCurr = END_MULTI_LST_NEXT(pCurr);
        i++;
    }

    et->etc->nmulticast = i;
    if (i) {
        if (et->etc->nmulticast == et->endobj.nMulti) {
            ET_ERROR(("et%d: et_config: etc.nmulticast %d, endobj.nMulti %d\n",
                et->etc->unit, et->etc->nmulticast, et->endobj.nMulti));
        }
    }

    if (et->endobj.flags & IFF_UP)
        et_up(et);
    else if (et->endobj.flags & IFF_RUNNING)
        et_down(et, 1);

    ET_UNLOCK(et);
}

/*
 * et_mcastAdd - add a multicast address for the device
 */
static STATUS
et_mcastAdd(et_info_t *et, char* pAddress)
{
    int error;

    ET_TRACE(("et%d: et_mcastAdd\n", et->etc->unit));

    if ((error = etherMultiAdd(&et->endobj.multiList, pAddress)) == ENETRESET)
        et_config(et);

    return (OK);
}

/*
 * et_mcastDel - delete a multicast address for the device
 */
static STATUS
et_mcastDel(et_info_t *et, char* pAddress)
{
    int error;

    ET_TRACE(("et%d: et_mcastDel\n", et->etc->unit));

    if ((error = etherMultiDel(&et->endobj.multiList, (char *)pAddress)) == ENETRESET)
        et_config(et);

    return (OK);
}

/*
 * et_mcastGet - get the multicast address list for the device
 */
static STATUS
et_mcastGet(et_info_t *et, MULTI_TABLE* pTable)
{
    ET_TRACE(("et%d: et_mcastGet\n", et->etc->unit));

    return (etherMultiGet(&et->endobj.multiList, pTable));
}

/* enqueue mblk on queue */
static void
et_enq(struct mblkq *q, M_BLK_ID m)
{
    ASSERT(m->mBlkHdr.mNextPkt == NULL);

    if (q->tail == NULL) {
        ASSERT(q->head == NULL);
        q->head = q->tail = m;
    }
    else {
        ASSERT(q->head);
        ASSERT(q->tail->mBlkHdr.mNextPkt == NULL);
        q->tail->mBlkHdr.mNextPkt = m;
        q->tail = m;
    }
    q->len++;
}

/* dequeue mblk from queue */
static M_BLK_ID
et_deq(struct mblkq *q)
{
    M_BLK_ID m;

    if ((m = q->head)) {
        ASSERT(q->tail);
        q->head = m->mBlkHdr.mNextPkt;
        m->mBlkHdr.mNextPkt = NULL;
        q->len--;
        if (q->head == NULL)
            q->tail = NULL;
    }
    else {
        ASSERT(q->tail == NULL);
    }

    return (m);
}

#ifdef BCM47XX_CHOPS
/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 */
void*
et_phyfind(et_info_t *et, uint coreunit)
{
    et_info_t *tmp;

    /* walk the list et's */
    for (tmp = et_list; tmp; tmp = tmp->next) {
        if (et->etc == NULL)
            continue;
        if (tmp->pciinfo.bus != et->pciinfo.bus)
            continue;
        if (tmp->etc->coreunit != coreunit)
            continue;
        break;
    }
    return (tmp);
}

/* shared phy read entry point */
uint16
et_phyrd(et_info_t *et, uint phyaddr, uint reg)
{
    uint16 val;

    ET_LOCK(et);
    val = et->etc->chops->phyrd(et->etc->ch, phyaddr, reg);
    ET_UNLOCK(et);

    return (val);
}

/* shared phy write entry point */
void
et_phywr(et_info_t *et, uint phyaddr, uint reg, uint16 val)
{
    ET_LOCK(et);
    et->etc->chops->phywr(et->etc->ch, phyaddr, reg, val);
    ET_UNLOCK(et);
}
#endif  /* BCM47XX_CHOPS */

/* debugging routines callable from the shell */
#ifdef BCMINTERNAL

static char buf1[64 * 1024];

void dump_etlog(int unit) {
    bcmdumplog(buf1, 4096);
    printf("%s", buf1);
}

#ifdef BCMDBG
void
set_et_msg_level(int level) {
    et_msg_level = level;
}

void
dump_etstats(int unit) {
    struct bcmstrbuf b;
    bcm_binit(&b, buf1, 4096);
    et_dump(et_global[unit], &b);
    printf("%s", buf1);
}

void up_et(int unit) {
    et_up(et_global[unit]);
}

#endif /* BCMDBG */
#endif /* BCMINTERNAL */

/* PED */
void 
et_dma_txreset(void *eti)
{
    et_info_t *et = (et_info_t *)eti;
    dma_info_t *di;
    uint i;
    uint32 status;

    for(i=0; i<NUMTXQ; i++) {
        di = &et->di[i];

        WR_REG(NULL, &di->d64txregs->control, D64_XC_SE);
        SPINWAIT(((status = (RD_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
                  D64_XS0_XS_DISABLED) &&
                 (status != D64_XS0_XS_IDLE) &&
                 (status != D64_XS0_XS_STOPPED),
                 10000);
    
        /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
        WR_REG(NULL, &di->d64txregs->control, 0);
        SPINWAIT(((status = (RD_REG(di->osh, &di->d64txregs->status0) & D64_XS0_XS_MASK)) !=
                  D64_XS0_XS_DISABLED),
                 10000);
    
        /* wait for the last transaction to complete */
        OSL_DELAY(300);
    }
}

/* PED */
void 
et_dma_rxreset(void *eti)
{
    et_info_t *et = (et_info_t *)eti;
    dma_info_t *di;
    uint i;
    uint32 status;

    for(i=0; i<NUMRXQ; i++) {
        di = &et->di[i];

        /* PR2414 WAR: DMA engines are not disabled until transfer finishes */
        WR_REG(NULL, &di->d64rxregs->control, 0);
        SPINWAIT(((status = (RD_REG(di->osh, &di->d64rxregs->status0) & D64_RS0_RS_MASK)) !=
                  D64_RS0_RS_DISABLED),
                 10000);
    }
}

/* PED */
void
et_dma_txinit(void *eti)
{
    et_info_t *et = (et_info_t *)eti;
    dma_info_t *di;
    uint i;
    uint offset = 0;
    
#ifdef IL_BIGENDIAN    
    offset = SI_SDRAM_SWAPPED;
#endif /* IL_BIGENDIAN */

    for(i=0; i<NUMTXQ; i++) {
        di = &et->di[i];
        
        di->txin = di->txout = 0;
        bzero(di->txd64, sizeof(dma64dd_t) * NTXD);
        WR_REG(NULL, &di->d64txregs->control, D64_XC_PD | D64_XC_XE);
        WR_REG(NULL, &di->d64txregs->addrlow, di->txdpa + offset);
        WR_REG(NULL, &di->d64txregs->addrhigh, 0);
    }
}

/* PED */
static INLINE void
_dma_init_rxfill(dma_info_t *di)
{
    uint rxout = 0;
    uint len;
    void *rxp;
    volatile dma64dd_t *dd;
    uint32 pa;
    
    for(len = NRXBUFPOST; len; len--) {
        rxp = PKTGET(di->osh, RXFILL_PKTSIZE, FALSE, MEMORY_DDRRAM);
        GETRXP(di, rxout) = rxp;
        dd = GETRXD(di, rxout);
        pa = (uint32)DMA_MAP(di->osh, PKTDATA(di->osh, rxp), RXBUFSZ, DMA_RX, rxp, NULL);
        dd->addrlow = BUS_SWAP32(pa);
        dd->ctrl1 = BUS_SWAP32(IS_LAST_RXD(rxout)? D64_CTRL1_EOT : 0);
        dd->ctrl2 = BUS_SWAP32(RXBUFSZ);
        rxout = NEXTRXD(rxout);
    }
    WR_REG(NULL, &di->d64rxregs->ptr, rxout);
    di->rxout = rxout;
}

/* PED */
static INLINE void
_dma_init_rxfill_shunt(dma_info_t *di)
{
    uint rxout = 0;
    uint len;
    void *rxp;
    volatile dma64dd_t *dd;
    
    for(len = NRXBUFPOST; len; len--) {
        rxp = PKTGET_SHUNT(NULL, BUFSZ, FALSE, MEMORY_SOCRAM);
        GETRXP(di, rxout) = rxp;
        dd = GETRXD(di, rxout);
        dd->addrlow = BUS_SWAP32((uint32)PKTDATAP_SHUNT(rxp));
        dd->ctrl1 = BUS_SWAP32(IS_LAST_RXD(rxout)? D64_CTRL1_EOT : 0);
        dd->ctrl2 = BUS_SWAP32(RXBUFSZ);
        rxout = NEXTRXD(rxout);
    }
    WR_REG(NULL, &di->d64rxregs->ptr, rxout);
    di->rxout = rxout;
}

/* PED */
void
et_dma_rxinit(void *eti)
{
    et_info_t *et = (et_info_t *)eti;
    dma_info_t *di;
    uint i;
    uint offset = 0;
    
#ifdef IL_BIGENDIAN    
    offset = SI_SDRAM_SWAPPED;
#endif /* IL_BIGENDIAN */

    for(i=0; i<NUMRXQ; i++) {
        di = &et->di[i];
        
        di->rxin = di->rxout = 0;
        bzero(di->rxd64, sizeof(dma64dd_t) * NRXD);
        WR_REG(NULL, &di->d64rxregs->control,
          ((HWRXOFF << D64_RC_RO_SHIFT) | D64_RC_OC | D64_RC_PD | D64_RC_RE));
        WR_REG(NULL, &di->d64rxregs->addrlow, di->rxdpa + offset);
        WR_REG(NULL, &di->d64rxregs->addrhigh, 0);

#if PED_ALL_SHUNT
        break; /* Only initialize channel 0 */
#endif
    }
    
#if PED_ALL_SHUNT
    _dma_init_rxfill_shunt(&et->di[0]);
#else /* !PED_ALL_SHUNT */
    _dma_init_rxfill(&et->di[0]);
    _dma_init_rxfill_shunt(&et->di[1]);
#endif /* PED_ALL_SHUNT */
}

/* PED */
void
et_dma_rxenable(void *eti)
{
    et_info_t *et = (et_info_t *)eti;
    dma_info_t *di;
    uint i;

    for(i=0; i<NUMRXQ; i++) {
        di = &et->di[i];
        
        WR_REG(NULL, &di->d64rxregs->control,
          ((HWRXOFF << D64_RC_RO_SHIFT) | D64_RC_OC | D64_RC_PD | D64_RC_RE));
    }
}

/* PED */
void
et_dma_attach(void *eti)
{
    et_info_t *et = (et_info_t *)eti;
    et->di[0].osh = et->osh;
    et->di[0].d64txregs = (dma64regs_t *)DMAREG(et, DMA_TX, TX_Q0);
    et->di[0].d64rxregs = (dma64regs_t *)DMAREG(et, DMA_RX, RX_Q0);
    et_dma_alloc(&et->di[0]);
    et->di[1].osh = et->osh;
    et->di[1].d64txregs = (dma64regs_t *)DMAREG(et, DMA_TX, TX_Q1);
    et->di[1].d64rxregs = (dma64regs_t *)DMAREG(et, DMA_RX, RX_Q1);
    et_dma_alloc(&et->di[1]);
}

/* PED */
static INLINE void 
_dma_init_txreclaim(dma_info_t *di)
{
    uint start, end;
    void *txp;
    
    end = di->txout;
    for(start = di->txin; start != end; start = NEXTTXD(start)) {
        txp = GETTXP(di, start);
        PKTFREE(di->osh, txp, FALSE, MEMORY_DDRRAM);
    }
    di->txin = start;
}

/* PED */
static INLINE void 
_dma_init_txreclaim_shunt(dma_info_t *di)
{
    uint start, end;
    void *txp;

    end = di->txout;
    for(start = di->txin; start != end; start = NEXTTXD(start)) {
        txp = GETTXP(di, start);
        PKTFREE_SHUNT(NULL, txp, FALSE, MEMORY_SOCRAM);
    }
    di->txin = start;
}

/* PED */
void
et_dma_txreclaim(void *eti)
{
    et_info_t *et = (et_info_t *)eti;

#if PED_ALL_SHUNT
    _dma_init_txreclaim_shunt(&et->di[0]);
#else /* !PED_ALL_SHUNT */
    _dma_init_txreclaim(&et->di[0]);
    _dma_init_txreclaim_shunt(&et->di[1]);
#endif /* PED_ALL_SHUNT */
}

/* PED */
static INLINE void 
_rxreclaim_shunt(dma_info_t *di)
{
    uint rxin = di->rxin;
    void *rxp;
    while(rxin != di->rxout) {
        rxp = GETRXP(di, rxin);
        rxin = NEXTRXD(rxin);
        PKTFREE_SHUNT(NULL, rxp, FALSE, MEMORY_SOCRAM);
    }
    di->rxin = rxin;
}

/* PED */
static INLINE void 
_rxreclaim(dma_info_t *di)
{
    uint rxin = di->rxin;
    void *rxp;
    while(rxin != di->rxout) {
        rxp = GETRXP(di, rxin);
        rxin = NEXTRXD(rxin);
        PKTFREE(di->osh, rxp, FALSE, MEMORY_DDRRAM);
    }
    di->rxin = rxin;
}

/* PED */
void
et_dma_rxreclaim(void *eti)
{
    et_info_t *et = (et_info_t *)eti;

#if PED_ALL_SHUNT
    _rxreclaim_shunt(&et->di[0]);
#else /* !PED_ALL_SHUNT */
    _rxreclaim(&et->di[0]);
    _rxreclaim_shunt(&et->di[1]);
#endif /* PED_ALL_SHUNT */
}

#if defined(SHUNT_COMMAND)
/* command line symbol for shunt configuration */
#define _CFP_SHUNT_KEY_INVALID 0
#define _CFP_SHUNT_KEY_DST_MAC 1
#define _CFP_SHUNT_KEY_SRC_MAC 2
#define _CFP_SHUNT_KEY_ETHER_TYPE 3
#define _CFP_SHUNT_KEY_DST_IP 4
#define _CFP_SHUNT_KEY_SRC_IP 5

/* 
  * dst mac/src mac, 
  * - each shunt type has to configure 3 cfp entries, ipv4/ipv6/non-ip 
  * ether type,
  * - 1 cfp entry, non-ip
  * dst ip/src ip, 
  * - each shunt type has to configure 1 cfp entry, ipv4
  */
#define CFP_SHUNT_ENTRY_NUM 9
#define _CFP_SHUNT_DST_MAC_IDX 0
#define _CFP_SHUNT_SRC_MAC_IDX 3
#define _CFP_SHUNT_ETHER_TYPE_IDX 6
#define _CFP_SHUNT_DST_IP_IDX 7
#define _CFP_SHUNT_SRC_IP_IDX 8


#define SET_CFP_FIELD_PARAM(fld_idx, fld_val, ram, l3_fram, s_id, buf_ptr) { \
    buf_ptr->field_idx = fld_idx; buf_ptr->field_value = fld_val; \
    buf_ptr->ram_type = ram; buf_ptr->l3_framing = l3_fram; \
    buf_ptr->slice_id = s_id;}

static
int _xdigit2i(int digit)
{
    if (digit >= '0' && digit <= '9') return (digit - '0'     );
    if (digit >= 'a' && digit <= 'f') return (digit - 'a' + 10);
    if (digit >= 'A' && digit <= 'F') return (digit - 'A' + 10);
    return 0;
}

static
int  _ctoi(const char *s, char **end)
{
    unsigned int	n, neg;
    int	base = 10;

    if (s == 0) {
	if (end != 0) {
	    end = 0;
	}
	return 0;
    }

    s += (neg = (*s == '-'));

    if (*s == '0') {
	s++;
	if (*s == 'x' || *s == 'X') {
	    base = 16;
	    s++;
	} else if (*s == 'b' || *s == 'B') {
	    base = 2;
	    s++;
	} else {
	    base = 8;
	}
    }

    for (n = 0; ((*s >= 'a' && *s < 'a' + base - 10) ||
		 (*s >= 'A' && *s < 'A' + base - 10) ||
		 (*s >= '0' && *s <= '9')); s++) {
	n = n * base + ((*s <= '9' ? *s : *s + 9) & 15);
    }

    if (end != 0) {
	*end = (char *) s;
    }

    return (int) (neg ? -n : n);
}

static
int _parse_integer(const char *s)
{
    unsigned int	n, neg, base = 10;

    s += (neg = (*s == '-'));

    if (*s == '0') {
	s++;

	if (*s == 'x' || *s == 'X') {
	    base = 16;
	    s++;
	} else if (*s == 'b' || *s == 'B') {
	    base = 2;
	    s++;
	} else {
	    base = 8;
	}
    }

    for (n = 0; ((*s >= 'a' && *s <= 'z' && base > 10) ||
		 (*s >= 'A' && *s <= 'Z' && base > 10) ||
		 (*s >= '0' && *s <= '9')); s++) {
	n = n * base +
	    (*s >= 'a' ? *s - 'a' + 10 :
	     *s >= 'A' ? *s - 'A' + 10 :
	     *s - '0');
    }

    return (neg ? -n : n);
}
	
static
int _parse_ipaddr(char *s, uint32 *ipaddr)
{
    char *ts;
    int i, x;
    uint32 ip = 0;

    if (strchr(s, '.')) {               /* dotted notation */
        for (i = 0; i < 4; i++) {
            x = _ctoi(s, &ts);
            if ((x > 0xff) || (x < 0)) {
                return(-1);
            } 
            ip = (ip << 8) | x;
            if (*ts != '.') {   /* End of string */
                break;
            }
            s = ts + 1;
        }
        if (((i != 3) || (*ts != '\0'))) {
            return(-1);
        } else {
            *ipaddr = ip;
            return(0);
        }
    } else {
        return(-1);
    }
}

static
int _parse_macaddr_hex(char *str, uint8 macaddr[6])
{
    char *s;
    int i;

    memset(macaddr, 0, 6);
    if ((strlen(str) > 12) || (strlen(str) == 0)) {
        return -1;
    }
    s = str + strlen(str) - 1;
    for (i = 0; i < 6; i++) {
        if (s < str) {  /* done w/ string; fill w/ 0. */
            macaddr[5-i] = 0;
        } else {
            if (!bcm_isxdigit((unsigned) *s)) {  /* bad character */
                return -1;
            }
            macaddr[5-i] = _xdigit2i((unsigned) *(s--));
            if (bcm_isxdigit((unsigned) *s)) {
                macaddr[5-i] += 16 * _xdigit2i((unsigned) *(s--));
            }
        }
    }

    return 0;
}
	
static
int _parse_macaddr(char *str, uint8 macaddr[6])
{
    char *s;
    int i;

    if (strchr(str, ':')) {             /* Colon format */
        s = str;
        for (i = 0; i < 6; i++) {
            if (!bcm_isxdigit((unsigned)*s)) {  /* bad character */
                return -1;
            }
            macaddr[i] = _xdigit2i((unsigned)*(s++));
            if (bcm_isxdigit((unsigned)*s)) {
                macaddr[i] *= 16;
                macaddr[i] += _xdigit2i((unsigned)*(s++));
            }
            if ((i < 5) && (*(s++) != ':')) {  /* bad character */
                return -1;
            }
        }
        if (*s) {
            return -1;
        }
    } else if (str[0] == '0' && bcm_tolower((int)str[1]) == 'x') {
        return _parse_macaddr_hex(&(str[2]), macaddr);
    } else {
        return -1;
    }

    return 0;
}

static void
bcm_cfp_entry_init(etc_info_t * etc, uint l3_fram, uint slice_id, void *arg)
{
    int i;
    cfp_ioctl_buf_t *cfp_buf_ptr;

    cfp_buf_ptr = (cfp_ioctl_buf_t *)arg;

    for (i=0; i < CFP_TCAM_ENTRY_WORD; i++) {
        cfp_buf_ptr->cfp_entry.tcam[i] = 0;
        cfp_buf_ptr->cfp_entry.tcam_mask[i] = 0;
    }
    cfp_buf_ptr->cfp_entry.action= 0;

    /* Create valid entry */
    cfp_buf_ptr->entry_idx = 0;
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buf_ptr));

    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_VALID, 3, CFP_RAM_TYPE_TCAM_MASK, 
        CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buf_ptr));

    /* Configure the L3 Framming value */
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_L3_FRAMING, l3_fram, 
        CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buf_ptr));
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_L3_FRAMING, 0x3, 
        CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buf_ptr));

    /* set slice id */
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, slice_id, 
            CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buf_ptr));
    SET_CFP_FIELD_PARAM(CFP_FIELD_IPV4_SLICE_ID, 3, 
            CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_IPV4, slice_id, cfp_buf_ptr);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buf_ptr));
}

static int
_bcm_shunt_da_set(etc_info_t * etc, cfp_ioctl_buf_t *cfp_buffer, char *value, int start_cfp_idx)
{
    uint slice_id, type_start_idx;
    uint i, l3_fram;
    uint udf_field_idx = 0, field_value;
    uint udf_valid_field_idx = 0;
    uint8   mac_addr[6];

    /* Choose the UDF 0, 1, 2 of SLICE 2 to use */
    slice_id = 0;

    type_start_idx = start_cfp_idx;

    for (l3_fram = 0; l3_fram <= CFP_L3_FRAMING_NONIP; l3_fram++) { 
        /* Check L3 framing */
        if ((l3_fram != CFP_L3_FRAMING_IPV4) && 
            (l3_fram != CFP_L3_FRAMING_IPV6) &&
            (l3_fram != CFP_L3_FRAMING_NONIP)) {
            continue;
        }

        bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

        /* Configure the UDF offset value and base */
        for (i= 0; i < 3; i++) {
            cfp_buffer->field_idx = i; /* UDF index */
            cfp_buffer->l3_framing = l3_fram;
            cfp_buffer->slice_id= slice_id;
            cfp_buffer->field_value = (2 * i); /* offset value */
            cfp_buffer->flags = CFP_UDF_OFFSET_BASE_STARTOFFRAME;
            etc_ioctl(etc, ETCCFPUDFWR, (void *)(cfp_buffer));
         }
     }

    /* CFP entries */
    for (l3_fram = 0; l3_fram <= CFP_L3_FRAMING_NONIP; l3_fram++) { 
        /* Check L3 framing */
        if ((l3_fram != CFP_L3_FRAMING_IPV4) && 
            (l3_fram != CFP_L3_FRAMING_IPV6) &&
            (l3_fram != CFP_L3_FRAMING_NONIP)) {
            continue;
        }

        switch (l3_fram) {
            case CFP_L3_FRAMING_IPV4:
                udf_field_idx = CFP_FIELD_IPV4_UDF0;
                udf_valid_field_idx = CFP_FIELD_IPV4_UDF0_VLD;
                break;
            case CFP_L3_FRAMING_IPV6:
                udf_field_idx = CFP_FIELD_IPV6_UDF0;
                udf_valid_field_idx = CFP_FIELD_IPV6_UDF0_VLD;
                break;
            case CFP_L3_FRAMING_NONIP:
                udf_field_idx = CFP_FIELD_NONIP_UDF0;
                udf_valid_field_idx = CFP_FIELD_NONIP_UDF0_VLD;
                break;
        }

        bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

        cfp_buffer->entry_idx = type_start_idx++;

        if (_parse_macaddr(value, mac_addr) < 0) {
            MFREE(NULL, cfp_buffer, sizeof(cfp_ioctl_buf_t));
            return -1;
        }

        for (i= 0; i < 3; i++) {
            field_value = ((mac_addr[(2 * i)] << 8) | (mac_addr[(2 * i) + 1]));
            /* UDF value */
            SET_CFP_FIELD_PARAM(udf_field_idx + i, field_value, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_field_idx + i, 0xffff, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
            /* UDF valid */
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        }


        SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_RX_CHANNEL_ID, 1, CFP_RAM_TYPE_ACTION, 
            l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        /* write this entry to CFP */
        etc_ioctl(etc, ETCCFPWR, (void *)(cfp_buffer));

    } 

    return 0;
}

static int
_bcm_shunt_sa_set(etc_info_t * etc, cfp_ioctl_buf_t *cfp_buffer, char *value, int start_cfp_idx)
{
    uint slice_id, type_start_idx;
    uint i, l3_fram;
    uint udf_field_idx = 0, field_value;
    uint udf_valid_field_idx = 0;
    uint8   mac_addr[6];

    /* Choose the UDF 3, 4, 5 of SLICE 2 to use */
    slice_id = 0;

    type_start_idx = start_cfp_idx;
    for (l3_fram = 0; l3_fram <= CFP_L3_FRAMING_NONIP; l3_fram++) { 
        /* Check L3 framing */
        if ((l3_fram != CFP_L3_FRAMING_IPV4) && 
            (l3_fram != CFP_L3_FRAMING_IPV6) &&
            (l3_fram != CFP_L3_FRAMING_NONIP)) {
            continue;
        }

        bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

        /* Configure the UDF offset value and base */
        for (i= 0; i < 3; i++) {
            cfp_buffer->field_idx = i+3; /* UDF index */
            cfp_buffer->l3_framing = l3_fram;
            cfp_buffer->slice_id= slice_id;
            cfp_buffer->field_value = (2 * i)+6; /* offset value */
            cfp_buffer->flags = CFP_UDF_OFFSET_BASE_STARTOFFRAME;
            etc_ioctl(etc, ETCCFPUDFWR, (void *)(cfp_buffer));
         }
     }

    /* CFP entries */
    for (l3_fram = 0; l3_fram <= CFP_L3_FRAMING_NONIP; l3_fram++) { 
        /* Check L3 framing */
        if ((l3_fram != CFP_L3_FRAMING_IPV4) && 
            (l3_fram != CFP_L3_FRAMING_IPV6) &&
            (l3_fram != CFP_L3_FRAMING_NONIP)) {
            continue;
        }

        switch (l3_fram) {
            case CFP_L3_FRAMING_IPV4:
                udf_field_idx = CFP_FIELD_IPV4_UDF3;
                udf_valid_field_idx = CFP_FIELD_IPV4_UDF3_VLD;
                break;
            case CFP_L3_FRAMING_IPV6:
                udf_field_idx = CFP_FIELD_IPV6_UDF3;
                udf_valid_field_idx = CFP_FIELD_IPV6_UDF3_VLD;
                break;
            case CFP_L3_FRAMING_NONIP:
                udf_field_idx = CFP_FIELD_NONIP_UDF3;
                udf_valid_field_idx = CFP_FIELD_NONIP_UDF3_VLD;
                break;
        }

        bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

        cfp_buffer->entry_idx = type_start_idx++;

        if (_parse_macaddr(value, mac_addr) < 0) {
            MFREE(NULL, cfp_buffer, sizeof(cfp_ioctl_buf_t));
            return -1;
        }

        for (i= 0; i < 3; i++) {
            field_value = ((mac_addr[(2 * i)] << 8) | (mac_addr[(2 * i) + 1]));
            /* UDF value */
            SET_CFP_FIELD_PARAM(udf_field_idx + i, field_value, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_field_idx + i, 0xffff, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
            /* UDF valid */
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
            SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
                CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
            etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        }


        SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_RX_CHANNEL_ID, 1, CFP_RAM_TYPE_ACTION, 
            l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        /* write this entry to CFP */
        etc_ioctl(etc, ETCCFPWR, (void *)(cfp_buffer));

    } 

    return 0;
}

static int
_bcm_shunt_ethertype_set(etc_info_t * etc, cfp_ioctl_buf_t *cfp_buffer, char *value, int start_cfp_idx)
{
    uint32 etype;
    uint slice_id, type_start_idx;
    uint l3_fram;

    etype = _parse_integer(value);

    /* Choose the UDF 0, 1, 2 of SLICE 2 to use */
    slice_id = 0;

    type_start_idx = start_cfp_idx;

    l3_fram = CFP_L3_FRAMING_NONIP;

    bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

    cfp_buffer->entry_idx = type_start_idx;

    SET_CFP_FIELD_PARAM(CFP_FIELD_NONIP_ETHERTYPE_SAP, etype, 
        CFP_RAM_TYPE_TCAM, CFP_L3_FRAMING_NONIP, 0, cfp_buffer);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
    
    SET_CFP_FIELD_PARAM(CFP_FIELD_NONIP_ETHERTYPE_SAP, 0xffff, 
        CFP_RAM_TYPE_TCAM_MASK, CFP_L3_FRAMING_NONIP, 0, cfp_buffer);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));

    SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_RX_CHANNEL_ID, 1, CFP_RAM_TYPE_ACTION, 
        l3_fram, slice_id, cfp_buffer);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
    /* write this entry to CFP */
    etc_ioctl(etc, ETCCFPWR, (void *)(cfp_buffer));

    return 0;
}

static int
_bcm_shunt_dip_set(etc_info_t * etc, cfp_ioctl_buf_t *cfp_buffer, char *value, int start_cfp_idx)
{
    uint32 ip_addr;
    uint slice_id, type_start_idx;
    uint i, l3_fram;
    uint udf_field_idx = 0, field_value;
    uint udf_valid_field_idx = 0;

    /* Choose the UDF 0, 1 of SLICE 2 to use */
    slice_id = 0;

    l3_fram = CFP_L3_FRAMING_IPV4;

    bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

    /* Configure the UDF offset value and base */
    for (i= 0; i < 2; i++) {
        cfp_buffer->field_idx = i; /* UDF index */
        cfp_buffer->l3_framing = l3_fram;
        cfp_buffer->slice_id= slice_id;
        cfp_buffer->field_value = (2 * i) + 16 ; /* offset value */
        cfp_buffer->flags = CFP_UDF_OFFSET_BASE_ENDOFL2;
        etc_ioctl(etc, ETCCFPUDFWR, (void *)(cfp_buffer));
     }

    type_start_idx = start_cfp_idx;


    udf_field_idx = CFP_FIELD_IPV4_UDF0;
    udf_valid_field_idx = CFP_FIELD_IPV4_UDF0_VLD;

    cfp_buffer->entry_idx = type_start_idx;

    if (_parse_ipaddr(value, &ip_addr)) {
        printf("Expecting IP address in the format "
               "w.x.y.z or 0x<value>\n");
        return(-1);
    }

    for (i= 0; i < 2; i++) {
        if (i == 0) {
            field_value = (ip_addr >> 16) & 0xffff;
        } else {
            field_value = ip_addr & 0xffff;
        }

        /* UDF value */
        SET_CFP_FIELD_PARAM(udf_field_idx + i, field_value, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_field_idx + i, 0xffff, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        /* UDF valid */
        SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
    }

    SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_RX_CHANNEL_ID, 1, CFP_RAM_TYPE_ACTION, 
        l3_fram, slice_id, cfp_buffer);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
    /* write this entry to CFP */
    etc_ioctl(etc, ETCCFPWR, (void *)(cfp_buffer));

    return 0;
}

static int
_bcm_shunt_sip_set(etc_info_t * etc, cfp_ioctl_buf_t *cfp_buffer, char *value, int start_cfp_idx)
{
    uint32 ip_addr;
    uint slice_id, type_start_idx;
    uint i, l3_fram;
    uint udf_field_idx = 0, field_value;
    uint udf_valid_field_idx = 0;

    /* Choose the UDF 2, 3 of SLICE 2 to use */
    slice_id = 0;

    l3_fram = CFP_L3_FRAMING_IPV4;

    bcm_cfp_entry_init(etc, l3_fram, slice_id, cfp_buffer);

    /* Configure the UDF offset value and base */
    for (i= 0; i < 2; i++) {
        cfp_buffer->field_idx = i+2; /* UDF index */
        cfp_buffer->l3_framing = l3_fram;
        cfp_buffer->slice_id= slice_id;
        cfp_buffer->field_value = (2 * i) + 12 ; /* offset value */
        cfp_buffer->flags = CFP_UDF_OFFSET_BASE_ENDOFL2;
        etc_ioctl(etc, ETCCFPUDFWR, (void *)(cfp_buffer));
     }

    type_start_idx = start_cfp_idx;


    udf_field_idx = CFP_FIELD_IPV4_UDF2;
    udf_valid_field_idx = CFP_FIELD_IPV4_UDF2_VLD;

    cfp_buffer->entry_idx = type_start_idx;

    if (_parse_ipaddr(value, &ip_addr)) {
        printf("Expecting IP address in the format "
               "w.x.y.z or 0x<value>\n");
        return(-1);
    }

    for (i= 0; i < 2; i++) {
        if (i == 0) {
            field_value = (ip_addr >> 16) & 0xffff;
        } else {
            field_value = ip_addr & 0xffff;
        }

        /* UDF value */
        SET_CFP_FIELD_PARAM(udf_field_idx + i, field_value, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_field_idx + i, 0xffff, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        /* UDF valid */
        SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
            CFP_RAM_TYPE_TCAM, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
        SET_CFP_FIELD_PARAM(udf_valid_field_idx + i, 0x1, 
            CFP_RAM_TYPE_TCAM_MASK, l3_fram, slice_id, cfp_buffer);
        etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
    }

    SET_CFP_FIELD_PARAM(CFP_FIELD_ACT_RX_CHANNEL_ID, 1, CFP_RAM_TYPE_ACTION, 
        l3_fram, slice_id, cfp_buffer);
    etc_ioctl(etc, ETCCFPFIELDWR, (void *)(cfp_buffer));
    /* write this entry to CFP */
    etc_ioctl(etc, ETCCFPWR, (void *)(cfp_buffer));

    return 0;
}

int
bcm_shunt_set(char *key, char *value)
{
    int unit = 0; 
    etc_info_t *etc;
    uint32 key_type = _CFP_SHUNT_KEY_INVALID;
    cfp_ioctl_buf_t *cfp_buffer;
    uint shunt_start_idx, type_start_idx = 0;
    int r = 0;

    /* 
      * Use et0's handler for the call of etc_ioctl().
      * Since the CFP is gma common feature, use 
      * et0 or et1's handler is not mattered.
      */
    etc = et_global[unit]->etc;

    shunt_start_idx = 0;

    /*
      * CFP entry start idx:
      * _CFP_SHUNT_KEY_DST_MAC: shunt_start_idx
      * _CFP_SHUNT_KEY_SRC_MAC: shunt_start_idx + 3
      * _CFP_SHUNT_KEY_DST_IP    : shunt_start_idx + 6
      * _CFP_SHUNT_KEY_SRC_IP    : shunt_start_idx + 9
      * _CFP_SHUNT_KEY_ETHER_TYPE: shunt_start_idx + 12
      */
    if (!strcmp(key, "dst mac") || !strcmp(key, "destination mac address")) {
        key_type = _CFP_SHUNT_KEY_DST_MAC;
        type_start_idx = shunt_start_idx + _CFP_SHUNT_DST_MAC_IDX;
    }

    if (!strcmp(key, "src mac") || !strcmp(key, "source mac address")) {
        key_type = _CFP_SHUNT_KEY_SRC_MAC;
        type_start_idx = shunt_start_idx + _CFP_SHUNT_SRC_MAC_IDX;
    }

    if (!strcmp(key, "ether type")) {
        key_type = _CFP_SHUNT_KEY_ETHER_TYPE;
        type_start_idx = shunt_start_idx + _CFP_SHUNT_ETHER_TYPE_IDX;
    }

    if (!strcmp(key, "dst ip") || !strcmp(key, "destination ip address")) {
        key_type = _CFP_SHUNT_KEY_DST_IP;
        type_start_idx = shunt_start_idx + _CFP_SHUNT_DST_IP_IDX;
    }

    if (!strcmp(key, "src ip") || !strcmp(key, "source ip address")) {
        key_type = _CFP_SHUNT_KEY_SRC_IP;
        type_start_idx = shunt_start_idx + _CFP_SHUNT_SRC_IP_IDX;
    }

    if (type_start_idx >= CFP_TCAM_NUM) {
        printf("Exceeds maximum CFP entry count\n");
        return -1;
    }

    if (key_type == _CFP_SHUNT_KEY_INVALID) {
        printf("Command format is: bcm_shunt_set(type, value)\n");
        printf("\tExample: bcm_shunt_set(\"dst mac\", \"0xffffffffffff\")\n");
        printf("\t         bcm_shunt_set(\"ether type\", \"0x86ee\")\n");
        printf("Supported types are:\n");
        printf("\tdst mac (destination mac address)\n");
        printf("\tsrc mac (source mac address)\n");
        printf("\tether type\n");
        printf("\tdst ip (destination ip address)\n");
        printf("\tsrc ip (source ip address)\n");
        return -1;
    }


    if (!(cfp_buffer = (cfp_ioctl_buf_t *)MALLOC(NULL, sizeof(cfp_ioctl_buf_t)))) {
        printf("Error :memory allocation failed");
        return -1;
    }


    if (key_type == _CFP_SHUNT_KEY_DST_MAC) {
        r = _bcm_shunt_da_set(etc, cfp_buffer, value, type_start_idx);
    }

    if (key_type == _CFP_SHUNT_KEY_SRC_MAC) {
        r = _bcm_shunt_sa_set(etc, cfp_buffer, value, type_start_idx);
    }

    if (key_type == _CFP_SHUNT_KEY_ETHER_TYPE) {
        r = _bcm_shunt_ethertype_set(etc, cfp_buffer, value, type_start_idx);
    }

    if (key_type == _CFP_SHUNT_KEY_DST_IP) {
        r = _bcm_shunt_dip_set(etc, cfp_buffer, value, type_start_idx);
    }

    if (key_type == _CFP_SHUNT_KEY_SRC_IP) {
        r = _bcm_shunt_sip_set(etc, cfp_buffer, value, type_start_idx);
    }


    if (cfp_buffer) {
        MFREE(NULL, cfp_buffer, sizeof(cfp_ioctl_buf_t));
    }

    return r;
}

int
bcm_shunt_dump(int cfp_idx)
{
    int unit = 0; 
    etc_info_t *etc;
    cfp_ioctl_buf_t *cfp_buffer;
    int i;

    /* 
      * Use et0's handler for the call of etc_ioctl().
      * Since the CFP is gma common feature, use 
      * et0 or et1's handler is not mattered.
      */
    etc = et_global[unit]->etc;

    if (!(cfp_buffer = (cfp_ioctl_buf_t *)MALLOC(NULL, sizeof(cfp_ioctl_buf_t)))) {
        printf("Error :memory allocation failed");
        return -1;
    }

    cfp_buffer->entry_idx = cfp_idx;
    etc_ioctl(etc, ETCCFPRD, (void *)(cfp_buffer));

    printf("CFP IDX: %d\n", cfp_buffer->entry_idx);
    printf("DATA:");
    for (i=0;i<8;i++) {
        printf(" %8x",cfp_buffer->cfp_entry.tcam[i]);
    }
    printf("\n");
    printf("MASK:");
    for (i=0;i<8;i++) {
        printf(" %8x",cfp_buffer->cfp_entry.tcam_mask[i]);
    }
    printf("\n");
    printf("ACTION: %x\n", cfp_buffer->cfp_entry.action);

    if (cfp_buffer) {
        MFREE(NULL, cfp_buffer, sizeof(cfp_ioctl_buf_t));
    }

    return 0;
}
#endif
