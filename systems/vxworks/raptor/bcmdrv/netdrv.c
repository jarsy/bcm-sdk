/*
 * $Id: netdrv.c,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Copyright 2002-2004 Wind River Systems, Inc.
 *
 */
#include "vxWorks.h"
#include "stdlib.h"
#include "cacheLib.h"
#include "intLib.h"
#include "end.h" /* Common END structures. */
#include "endLib.h"
#include "lstLib.h" /* Needed to maintain protocol list. */
#include "wdLib.h"
#include "iv.h"
#include "semLib.h"
#include "etherLib.h"
#include "logLib.h"
#include "netLib.h"
#include "stdio.h"
#include "sysLib.h"
#include "errno.h"
#include "errnoLib.h"
#include "memLib.h"
#include "iosLib.h"
#include "etherMultiLib.h"
#include "taskLib.h"
#if (CPU_FAMILY == MIPS)
#include "mbz.h"
#endif
#include "config.h"
#include "systemInit.h"
#include "dmaOps.h"
#include "salIntf.h"

#define NETDRV_POLLING  0x2
#define NETDRV_MIN_FBUF 1518   /* min first buffer size */
#define NETDRV_CL_LEN   (1536)

#define END_SPEED_10M   (10000000)      /* 10Mbs */
#define END_SPEED_100M  (100000000)     /* 100Mbs */
#define END_SPEED_1000M (1000000000)    /* 1000Mbs */
#define END_SPEED       END_SPEED_1000M

typedef struct free_args {
    void* arg1;
    void* arg2;
} FREE_ARGS;

typedef struct end_device
{
    END_OBJ     end;            /* The class we inherit from. */
    int         unit;           /* unit number */
    int         ivec;           /* interrupt vector */
    int         ilevel;         /* interrupt level */
    char*       pShMem;         /* real ptr to shared memory */
    long        flags;          /* Our local flags. */
    UCHAR       enetAddr[6];    /* ethernet address */
    CACHE_FUNCS cacheFuncs;     /* cache function pointers */
    FUNCPTR     freeRtn[128];   /* Array of free routines. */
    struct free_args freeData[128]; /* Array of free arguments */
    CL_POOL_ID  pClPoolId;      /* cluster pool */
    BOOL        rxHandling;     /* rcv task is scheduled */
} END_DEVICE;

typedef struct {
    int len;
    char  pData[1536];
} PKT;

typedef struct rfd {
    PKT *  pPkt;
    struct rfd * next;
} RFD; /* dummy rx frame descriptor */

extern unsigned char gucDrvBoardType;
extern unsigned char gSwitchMac[6];
IMPORT  int endMultiLstCnt (END_OBJ* pEnd);

/* #undef BOOTROM_DEBUG */

/* Configuration items */

/* Cache macros */
#define END_CACHE_INVALIDATE(address, len) \
    CACHE_DRV_INVALIDATE (&pDrvCtrl->cacheFuncs, (address), (len))

#define END_CACHE_PHYS_TO_VIRT(address) \
    CACHE_DRV_PHYS_TO_VIRT (&pDrvCtrl->cacheFuncs, (address))

#define END_CACHE_VIRT_TO_PHYS(address) \
    CACHE_DRV_VIRT_TO_PHYS (&pDrvCtrl->cacheFuncs, (address))

/*
 * Default macro definitions for BSP interface.
 * These macros can be redefined in a wrapper file, to generate
 * a new module with an optimized interface.
 */

/* Macro to connect interrupt handler to vector */

/*
 * Macros to do a short (UINT16) access to the chip. Default
 * assumes a normal memory mapped device.
 */

#ifndef TEMPLATE_OUT_SHORT
#define TEMPLATE_OUT_SHORT(pDrvCtrl,addr,value) \
    (*(USHORT *)addr = value)
#endif

#ifndef TEMPLATE_IN_SHORT
#define TEMPLATE_IN_SHORT(pDrvCtrl,addr,pData) \
    (*pData = *addr)
#endif

/* A shortcut for getting the hardware address from the MIB II stuff. */

#define END_HADDR(pEnd) \
    ((pEnd)->mib2Tbl.ifPhysAddress.phyAddress)

#define END_HADDR_LEN(pEnd) \
    ((pEnd)->mib2Tbl.ifPhysAddress.addrLength)

/* typedefs */

/*
 * This will only work if there is only a single unit, for multiple
 * unit device drivers these should be integrated into the END_DEVICE
 * structure.
 */

M_CL_CONFIG NetdrvMclBlkConfig = { /* network mbuf configuration table */
    /*
    no. mBlks		no. clBlks	memArea		memSize
    -----------		----------	-------		-------
    */
    0,          0,      NULL,       0
};

CL_DESC NetdrvClDescTbl [] =  /* network cluster pool configuration table */
{
    /*
    clusterSize		num	  memArea	memSize
    -----------		----  -------	-------
    */
    {NETDRV_CL_LEN,    0,    NULL,       0}
};

int NetdrvClDescTblNumEnt = (NELEMENTS(NetdrvClDescTbl));

NET_POOL NetdrvCmpNetPool;

/* static int g_RxPort; */

#define g_RxMinUnit   0
#define g_RxMaxUnit   MAX_DEVICES

/* Definitions for the flags field */

/* Status register bits, returned by NetdrvStatusRead() */

#define NETDRV_RINT        0x1    /* Rx interrupt pending */
#define NETDRV_TINT        0x2    /* Tx interrupt pending */
#define NETDRV_RXON        0x4    /* Rx on (enabled) */
#define NETDRV_VALID_INT   0x3    /* Any valid interrupt pending */

#define DRV_DEBUG_OFF         0x0000
#define DRV_DEBUG_RX          0x0001
#define DRV_DEBUG_TX          0x0002
#define DRV_DEBUG_INT         0x0004
#define DRV_DEBUG_POLL        (DRV_DEBUG_POLL_RX | DRV_DEBUG_POLL_TX)
#define DRV_DEBUG_POLL_RX     0x0008
#define DRV_DEBUG_POLL_TX     0x0010
#define DRV_DEBUG_LOAD        0x0020
#define DRV_DEBUG_IOCTL       0x0040
#define DRV_DEBUG_POLL_REDIR  0x10000
#define DRV_DEBUG_LOG_NVRAM   0x20000

int     NetdrvTxInts=0;

#if 0
#define DRV_LOG(FLG, X0, X1, X2, X3, X4, X5, X6) \
    logMsg(X0, X1, X2, X3, X4, X5, X6);
#else
#define DRV_LOG(FLG, X0...) \
            printf(X0)
            
#endif

#define DRV_PRINT(FLG,X)  printf##X

/* LOCALS */

/* forward static functions */

/*LOCAL void	NetdrvReset	(END_DEVICE *pDrvCtrl);*/ /*ysw@2001-07-25*/
void            NetdrvHandleRcvInt(END_DEVICE *pDrvCtrl, int unit);
int             NetdrvRecv(END_DEVICE *pDrvCtrl, bcm_pkt_t *pPkt);
LOCAL void      NetdrvConfig(END_DEVICE *pDrvCtrl);
LOCAL UINT      NetdrvStatusRead(END_DEVICE *pDrvCtrl);

/* END Specific interfaces. */

/* This is the only externally visible interface. */

END_OBJ*    NetdrvLoad (char* initString, void *ap);

LOCAL STATUS    NetdrvStart (END_DEVICE * pDrvCtrl);
LOCAL STATUS    NetdrvStop  (END_DEVICE * pDrvCtrl);
LOCAL int       NetdrvIoctl   (END_DEVICE * pDrvCtrl, int cmd, caddr_t data);
LOCAL STATUS    NetdrvUnload    (END_DEVICE * pDrvCtrl);
LOCAL STATUS    NetdrvSend  (END_DEVICE * pDrvCtrl, M_BLK_ID pBuf);

LOCAL STATUS    NetdrvMCastAdd (END_DEVICE * pDrvCtrl, char* pAddress);
LOCAL STATUS    NetdrvMCastDel (END_DEVICE * pDrvCtrl, char* pAddress);
LOCAL STATUS    NetdrvMCastGet (END_DEVICE * pDrvCtrl, MULTI_TABLE* pTable);
LOCAL STATUS    NetdrvPollStart (END_DEVICE * pDrvCtrl);
LOCAL STATUS    NetdrvPollStop (END_DEVICE * pDrvCtrl);
LOCAL STATUS    NetdrvPollSend (END_DEVICE * pDrvCtrl, M_BLK_ID pBuf);
LOCAL STATUS    NetdrvPollRcv (END_DEVICE * pDrvCtrl, M_BLK_ID pBuf);
LOCAL void      NetdrvAddrFilterSet(END_DEVICE *pDrvCtrl);

/*LOCAL STATUS	NetdrvParse	();*//*ysw@2001-07-25*/
STATUS NetdrvMemInit(END_DEVICE * pDrvCtrl);

/*
 * Declare our function table.  This is static across all driver
 * instances.
 */

LOCAL NET_FUNCS NetdrvFuncTable =
{
    (FUNCPTR) NetdrvStart,      /* Function to start the device. */
    (FUNCPTR) NetdrvStop,       /* Function to stop the device. */
    (FUNCPTR) NetdrvUnload,     /* Unloading function for the driver. */
    (FUNCPTR) NetdrvIoctl,      /* Ioctl function for the driver. */
    (FUNCPTR) NetdrvSend,       /* Send function for the driver. */
    (FUNCPTR) NetdrvMCastAdd,   /* Multicast add function for the */
    (FUNCPTR) NetdrvMCastDel,   /* Multicast delete function for */
    (FUNCPTR) NetdrvMCastGet,   /* Multicast retrieve function for */
    (FUNCPTR) NetdrvPollSend,   /* Polling send function */
    (FUNCPTR) NetdrvPollRcv,    /* Polling receive function */
    endEtherAddressForm,        /* put address info into a NET_BUFFER */
    endEtherPacketDataGet,      /* get pointer to data in NET_BUFFER */
    endEtherPacketAddrGet       /* Get packet addresses. */
};

int GetMacFromFlash(UCHAR *mac) {
    return sysEnetAddrGet("netdrv", 0, mac);
}


/**
 *
 * NetdrvLoad - initialize the driver and device
 *
 * This routine initializes the driver and the device to the operational state.
 * All of the device specific parameters are passed in the initString.
 *
 * The string contains the target specific parameters like this:
 *
 * "register addr:int vector:int level:shmem addr:shmem size:shmem width"
 *
 * RETURNS: An END object pointer or NULL on error.
 *
 **/
static END_DEVICE *__netDriver;

END_OBJ*
NetdrvLoad(char* initString, void *ap)
{
    END_DEVICE    *pDrvCtrl;

    /* parse the init string, filling in the device structure */
    if(initString == NULL) {
        return (NULL);
    }

    if (initString[0] == 0) {
        strcpy(initString,"netdrv");
        return (NULL);
    }

    /* allocate the device structure */
    pDrvCtrl = (END_DEVICE *)calloc (sizeof(END_DEVICE), 1);
    if (pDrvCtrl == NULL) {
        goto errorExit;
    }

    /* Parse */
    pDrvCtrl->unit = 0;
    pDrvCtrl->ivec = 0;
    pDrvCtrl->ilevel = 7;

    /*Ask the BSP to provide the ethernet address.*/
    GetMacFromFlash(pDrvCtrl->enetAddr);

    /* initialize the END and MIB2 parts of the structure */
    /*
     * The M2 element must come from m2Lib.h
     * This Netdrv is set up for a DIX type ethernet device.
     */

    if ((END_OBJ_INIT (&pDrvCtrl->end, (DEV_OBJ *)pDrvCtrl, "netdrv",
        pDrvCtrl->unit, &NetdrvFuncTable,
        "END Net Driver.") == ERROR)
        || (END_MIB_INIT (&pDrvCtrl->end, M2_ifType_ethernet_csmacd,
        &pDrvCtrl->enetAddr[0], 6, ETHERMTU /*END_BUFSIZ*/, END_SPEED) == ERROR)) {
        goto errorExit;
    }

    /* Perform memory allocation/distribution */
    if (NetdrvMemInit (pDrvCtrl) == ERROR) {
        goto errorExit;
    }

    /* reset and reconfigure the device */
    /* NetdrvReset (pDrvCtrl);
       NetdrvConfig (pDrvCtrl);
    */

    /* set the flags to indicate readiness */
    END_OBJ_READY (&pDrvCtrl->end,
        IFF_UP | IFF_RUNNING | IFF_NOTRAILERS | IFF_BROADCAST
        | IFF_MULTICAST);

#ifdef POLLING_MODE
     /* NetdrvStart(pDrvCtrl); */
#endif
    DRV_LOG (1, "Net driver is loaded successfully!\n",1,2,3,4,5,6);

    return (&pDrvCtrl->end);

    errorExit:
    DRV_LOG (1, "FATAL ERROR :: fail to load net driver!\n",1,2,3,4,5,6);

    if (pDrvCtrl != NULL) {
        free ((char *)pDrvCtrl);
    }

    return NULL;
}


/**
 *
 * NetdrvMemInit - initialize memory for the chip
 *
 * This routine is highly specific to the device.
 *
 * Input: device to be initialized
 * RETURNS: OK or ERROR.
 *
 *
 **/
STATUS
NetdrvMemInit(END_DEVICE * pDrvCtrl)
{
    

    /*
     * This is how we would set up and END netPool using netBufLib(1).
     * This code is pretty generic.
     */
    if(pDrvCtrl == NULL)
        return ERROR;

    if((pDrvCtrl->end.pNetPool = malloc (sizeof(NET_POOL))) == NULL)
        return (ERROR);

    NetdrvMclBlkConfig.mBlkNum = 1024 /*32*/;
    NetdrvClDescTbl[0].clNum = 512 /* 1MB */;
    NetdrvMclBlkConfig.clBlkNum = NetdrvClDescTbl[0].clNum;

    /* Calculate the total memory for all the M-Blks and CL-Blks. */
    NetdrvMclBlkConfig.memSize =
        (NetdrvMclBlkConfig.mBlkNum * (MSIZE + sizeof (int))) +
        (NetdrvMclBlkConfig.clBlkNum * (CL_BLK_SZ));

    NetdrvMclBlkConfig.memArea =
        (char *) memalign (4, (UINT) NetdrvMclBlkConfig.memSize);

    if(NetdrvMclBlkConfig.memArea == NULL)
        return (ERROR);

    /* Calculate the memory size of all the clusters. */
    NetdrvClDescTbl[0].memSize = (NetdrvClDescTbl[0].clNum *
        (NetdrvClDescTbl[0].clSize + sizeof(int)));

    /* Allocate the memory for the clusters from cache safe memory. */
    NetdrvClDescTbl[0].memArea =
        (char *) sal_dma_alloc(NetdrvClDescTbl[0].memSize, "mema");

    if (NetdrvClDescTbl[0].memArea == NULL) {
        PRINTF_ERROR(("\r\nsystem memory unavailable!"));
        return (ERROR);
    }

    /* Initialize the memory pool. */
    if (netPoolInit(pDrvCtrl->end.pNetPool, &NetdrvMclBlkConfig,
        &NetdrvClDescTbl[0], NetdrvClDescTblNumEnt, NULL) == ERROR) {
        PRINTF_ERROR(("\r\nCould not init buffering"));
        return (ERROR);
    }

    /*
     * If you need clusters to store received packets into then get them
     * here ahead of time.
     */
    if ((pDrvCtrl->pClPoolId =
        netClPoolIdGet (pDrvCtrl->end.pNetPool, sizeof (RFD), FALSE)) == NULL) {
        return (ERROR);
    }

    PRINTF_DEBUG(("\r\nMemory setup complete"));

    return OK;
}

rxq_ctrl_t g_rxq_ctrl;

static int _n = 0;
unsigned long g_ulPktSend = 0;
unsigned long g_ulPktRecv = 0;
unsigned long g_ulPktUp = 0;

#ifdef POLLING_MODE

extern int _n_devices;
extern int rx_thread_dv_check(int unit);

#define  DRV_RX_DMA_CHAN 1

extern void soc_dma_done_chain(int unit, uint32 chan);

static void
NetdrvHandleRcvPkt(END_DEVICE *pDrvCtrl, int unit, uint32 stat)
{
    /* pDrvCtrl->rxHandling = TRUE; */

#if 0 /* Should process this, BRCM to confirm */
    if (stat & DS_DESC_DONE_TST(DRV_RX_DMA_CHAN)) {
        soc_dma_done_desc(unit, (uint32)(rxchan), DRV_RX_DMA_CHAN);
    }
#endif

    if (stat & DS_CHAIN_DONE_TST(DRV_RX_DMA_CHAN)) {
        soc_dma_done_chain(unit, (uint32)(DRV_RX_DMA_CHAN));
    }

    /* pDrvCtrl->rxHandling = FALSE; */
}


void
DrvPollRcvPktTask(END_DEVICE *pDrvCtrl)
{
    uint32 stat = 0;
    uint32 rxmask = DS_DESC_DONE_TST(DRV_RX_DMA_CHAN) |
        DS_CHAIN_DONE_TST(DRV_RX_DMA_CHAN);
    uint32 txmask = DS_CHAIN_DONE_TST(0);
    int unit;
    int i = 0;

    PRINTF_DEBUG(("BCM NetDriver is starting polling mode.......\n"));

    for(;;) {
        for (unit = g_RxMinUnit; unit < _n_devices; unit++) {
            i++;
            stat = soc_pci_read(unit, CMIC_DMA_STAT);
            if ((stat & rxmask) != 0) {
                /* soc_dma_done_chain(unit, (uint32)0); */
                NetdrvHandleRcvPkt(pDrvCtrl, unit, stat);
            }

            rx_thread_dv_check(unit);
            if (i > 100) {
                taskDelay(1) ;
                i = 0;
            }
        }
    }
}

int gPollRecvTaskId = 0;
#endif  /* POLLING_MODE */

/**
 *
 * NetdrvStart - start the device
 *
 * This function calls BSP functions to connect interrupts and start the
 * device running in interrupt mode.
 *
 * RETURNS: OK or ERROR
 *
 **/
int started = 0;

int
NetdrvStart(END_DEVICE * pDrvCtrl)
{

#ifdef POLLING_MODE
    unsigned int unit;

   if (started == 1) { return OK; }
#endif

    /** start RX Tasks */
    PRINTF_DEBUG(("BCM NetDriver is starting .......\n"));

#ifdef POLLING_MODE
    if (_n_devices > MAX_DEVICES) {
        printf("\r\nCannot support switch chips over 4\r\n");
        return ERROR;
    }
#endif /* POLLING_MODE */

    __netDriver = pDrvCtrl;

    if (GetMacFromFlash(pDrvCtrl->enetAddr) == ERROR) {
        printf("Error: unable to start netdrv (Invalid MAC Address)\n");
        return ERROR;
    }

    bcopy ((char *)pDrvCtrl->enetAddr, 
           (char *)END_HADDR(&pDrvCtrl->end),
           END_HADDR_LEN(&pDrvCtrl->end));
    {
        extern void   bcmSystemInit();
        bcmSystemInit();
        taskDelay(100);
    }

    END_FLAGS_SET (&pDrvCtrl->end, IFF_UP | IFF_RUNNING);

#ifdef POLLING_MODE
    gPollRecvTaskId =
        taskSpawn("RxPollTask", 100, 0, 16384, (FUNCPTR)DrvPollRcvPktTask,
                   (int)pDrvCtrl, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (gPollRecvTaskId == ERROR) {
        return ERROR;
    }
    started = 1;
#endif /* POLLING_MODE */

    return (OK);
}


/*
 * Send the received packet to END driver
 */
int
NetdrvSendToEnd(bcm_pkt_t *pPkt)
{
    int         len;
    M_BLK_ID    pMblk;
    char*       pCluster = NULL;
    CL_BLK_ID   pClBlk;
    END_DEVICE  *pDrvCtrl = __netDriver;

    if (pDrvCtrl == NULL) {
        return ERROR;
    }

    /* Add one to our unicast data. */
    END_ERR_ADD (&pDrvCtrl->end, MIB2_IN_UCAST, +1);

    /*
     * We implicitly are loaning here, if copying is necessary this
     * step may be skipped, but the data must be copied before being
     * passed up to the protocols.
     */
    pCluster = netClusterGet (pDrvCtrl->end.pNetPool, pDrvCtrl->pClPoolId);

    if (pCluster == NULL) {
        DRV_LOG (1, "Cannot loan!\n",1,2,3,4,5,6);
        END_ERR_ADD (&pDrvCtrl->end, MIB2_IN_ERRS, +1);
        goto cleanRXD;
    }

    /* Grab a cluster block to marry to the cluster we received. */
    if ((pClBlk = netClBlkGet (pDrvCtrl->end.pNetPool, M_DONTWAIT)) == NULL) {
        netClFree (pDrvCtrl->end.pNetPool, (UCHAR *)pCluster);
        DRV_LOG (DRV_DEBUG_RX, "Out of Cluster Blocks!\n", 1, 2, 3, 4, 5, 6);
        END_ERR_ADD (&pDrvCtrl->end, MIB2_IN_ERRS, +1);
        goto cleanRXD;
    }

    /*
     * OK we've got a spare, let's get an M_BLK_ID and marry it to the
     * one in the ring.
     */
    if ((pMblk = mBlkGet (pDrvCtrl->end.pNetPool, M_DONTWAIT, MT_DATA)) == NULL) {
        netClBlkFree (pDrvCtrl->end.pNetPool, pClBlk);
        netClFree (pDrvCtrl->end.pNetPool, (UCHAR *)pCluster);
        DRV_LOG (DRV_DEBUG_RX, "Out of M Blocks!\n", 1, 2, 3, 4, 5, 6);
        END_ERR_ADD (&pDrvCtrl->end, MIB2_IN_ERRS, +1);
        goto cleanRXD;
    }

    /* remove tag */
    len = pPkt->tot_len - 8;

    memcpy(pCluster + 2, (unsigned char *)(&(pPkt->_pkt_data.data[0])), 12);
    memcpy(pCluster + 14, (unsigned char *)(&(pPkt->_pkt_data.data[16])), len-12);

    /* Free buffer previously stolen */
    bcm_rx_free(pPkt->unit, pPkt->_pkt_data.data);

    /* Join the cluster to the MBlock */
    netClBlkJoin (pClBlk, pCluster, len, NULL, 0, 0, 0);
    netMblkClJoin (pMblk, pClBlk);

    pMblk->mBlkHdr.mLen = len;
    pMblk->mBlkHdr.mData += 2;
    pMblk->mBlkHdr.mFlags |= M_PKTHDR;
    pMblk->mBlkPktHdr.len = len;

    /* Send the packet to END driver */
    END_RCV_RTN_CALL(&__netDriver->end, pMblk);
    g_ulPktUp++;

    return OK;

    cleanRXD:
    return (ERROR);
}


struct _rxctrl *bcm_get_rx_control_ptr(int unit);
int rx_thread_dv_check(int unit);


void
rx_pkt_thread(void *param)
{
    int unit = (int) param;
    struct _rxctrl *prx_control = bcm_get_rx_control_ptr(unit);
    bcm_pkt_t pPkt;

    PRINTF_DEBUG(("RX:  Packet thread starting\n"));

    /* Sleep on sem */
    while (1) {
        rx_thread_dv_check(unit);

        prx_control->pkt_notify_given = FALSE;

        /* Service as many packets as possible */
        while (!RXQ_EMPTY(&g_rxq_ctrl)) {
            RXQ_DEQUEUE(&g_rxq_ctrl, &pPkt);
            NetdrvSendToEnd(&pPkt);
        }

        sal_sem_take(prx_control->pkt_notify, sal_sem_FOREVER);
    }

    prx_control->thread_exit_complete = TRUE;
    PRINTF_DEBUG(("RX: Packet thread exitting\n"));
    sal_thread_exit(0);

    return;
}

/*
 * NetdrvRecv - process the next incoming packet
 *
 * Handle one incoming packet.  The packet is checked for errors.
 *
 * RETURNS: N/A.
 */

int
netdrvProcessRecv(bcm_pkt_t *pPkt)
{
    
    if(pPkt == NULL) {
        return ERROR;
    }

#ifdef POLLING_MODE
    return (NetdrvSendToEnd(pPkt));
#else /* intr mode */

    /* Just queue the packet so it can be processed in non-intr context */
    RXQ_ENQUEUE(&g_rxq_ctrl, pPkt);

    /* Mark as stolen, will be released by NetdrvSendToEnd() */
    pPkt->_pkt_data.data = NULL;
    pPkt->alloc_ptr = NULL;

    g_ulPktRecv++;

    return OK;
#endif
}


/**
 *
 * NetdrvSend - the driver send routine
 *
 * This routine takes a M_BLK_ID sends off the data in the M_BLK_ID.
 * The buffer must already have the addressing information properly installed
 * in it.  This is done by a higher layer.  The last arguments are a VOS_Free
 * routine to be called when the device is done with the buffer and a pointer
 * to the argument to pass to the free routine.
 *
 * RETURNS: OK or ERROR.
 *
 *
 *
 **/
extern int Drv_Tx(int unit, uint32 port, uint8 *pkt, uint32 len);

extern int _n_devices;

int pkt_bcm_tx(int unit, uint8 *pdata, int32 len);

LOCAL STATUS
NetdrvSend(END_DEVICE * pDrvCtrl, M_BLK_ID pMblk)
{
    unsigned char  *pucPktData=NULL;
    uint32 len = 0;
    uint32 ulPort = -1;
    ULONG ulRet=OK;
    unsigned int unit = 0;
    int i;

    /* taskDelay(0); */

#ifdef BOOTROM_DEBUG
    PRINTF_DEBUG2("NetdrvSend\n"); /* DEL_ME */
#endif

    /* END_TX_SEM_TAKE(&pDrvCtrl->end, WAIT_FOREVER); */

    if (pMblk->mBlkPktHdr.len > NETDRV_CL_LEN - 4) {
        ulRet = ERROR;
        goto sendErr;
    }

    pucPktData = sal_dma_alloc(NETDRV_CL_LEN, "pucPktData");
    if (NULL==pucPktData) {
        ulRet = ERROR;
        goto sendErr;
    }

    /* memset(pucPktData, 0, NETDRV_CL_LEN); */

    len = (ULONG)netMblkToBufCopy(pMblk, pucPktData, NULL);
    len = max(ETHERSMALL, len);

    for (unit = 0; unit < _n_devices; unit++) {
        if (pkt_bcm_tx(unit, pucPktData, len) != 0) { /* Synchronous TX */
            printf("ERROR: fail to tx pkt unit %d\n", unit);
            ulRet = ERROR;
            goto sendErr;
        }
    }

    g_ulPktSend++;

    sendErr:

    netMblkClChainFree (pMblk);

    /* for the packet already sending, we don't want to free them */
    if (NULL!=pucPktData) {
        cacheDmaFree(pucPktData);
        pucPktData = NULL;
    }

    /* END_TX_SEM_GIVE(&pDrvCtrl->end); */

    /* Bump the statistic counter. */
    END_ERR_ADD (&pDrvCtrl->end, MIB2_OUT_UCAST, +1);

    return ulRet;
}


/**
 *
 * NetdrvIoctl - the driver I/O control routine
 *
 * Process an ioctl request.
 *
 * RETURNS: A command specific response, usually OK or ERROR.
 *
 *
 **/

LOCAL int
NetdrvIoctl(END_DEVICE * pDrvCtrl, /* device receiving command */
            int cmd,               /* ioctl command code */
            caddr_t data           /* command argument */)
{
    int error = 0;
    long value;

    switch (cmd) {
        case EIOCSADDR:
            if (data == NULL) {
                return (EINVAL);
            }
            bcopy ((char *)data, (char *)END_HADDR(&pDrvCtrl->end),
                END_HADDR_LEN(&pDrvCtrl->end));
            break;

        case EIOCGADDR:
            if (data == NULL) {
                return (EINVAL);
            }
            bcopy ((char *)END_HADDR(&pDrvCtrl->end), (char *)data,
                END_HADDR_LEN(&pDrvCtrl->end));
            break;

        case EIOCSFLAGS:
            value = (long)data;
            if (value < 0) {
                value = -(--value);
                END_FLAGS_CLR (&pDrvCtrl->end, value);
            } else {
                END_FLAGS_SET (&pDrvCtrl->end, value);
            }
            NetdrvConfig(pDrvCtrl);
            break;

        case EIOCGFLAGS:
            *(int *)data = END_FLAGS_GET(&pDrvCtrl->end);
            break;

        case EIOCPOLLSTART: /* Begin polled operation */
            NetdrvPollStart (pDrvCtrl);
            break;

        case EIOCPOLLSTOP: /* End polled operation */
            NetdrvPollStop (pDrvCtrl);
            break;

        case EIOCGMIB2: /* return MIB information */
            if(data == NULL) {
                return (EINVAL);
            }
            bcopy((char *)&pDrvCtrl->end.mib2Tbl, (char *)data,
                sizeof(pDrvCtrl->end.mib2Tbl));
            break;

        case EIOCGFBUF: /* return minimum First Buffer for chaining */
            if(data == NULL) {
                return (EINVAL);
            }
            *(int *)data = NETDRV_MIN_FBUF;
            break;

        default:
            error = EINVAL;
    }

    return (error);
}


/******************************************************************************
 *
 * NetdrvConfig - reconfigure the interface under us.
 *
 * Reconfigure the interface setting promiscuous mode, and changing the
 * multicast interface list.
 *
 * Input: device to be re-configured
 * RETURNS: N/A.
 */

LOCAL void
NetdrvConfig(END_DEVICE *pDrvCtrl)
{
    /* Set promiscuous mode if it's asked for. */

    if (END_FLAGS_GET(&pDrvCtrl->end) & IFF_PROMISC) {
        PRINTF_DEBUG(("\r\nSetting promiscuous mode on!"));
    } else {
        /*VOS_printf("\r\nSetting promiscuous mode off!");*/
    }

    /* Set up address filter for multicasting. */

    if (END_MULTI_LST_CNT(&pDrvCtrl->end) > 0) {
        NetdrvAddrFilterSet (pDrvCtrl);
    }

    

    

    

    return;
}


/**
 *
 * NetdrvAddrFilterSet - set the address filter for multicast addresses
 *
 * This routine goes through all of the multicast addresses on the list
 * of addresses (added with the endAddrAdd() routine) and sets the
 * device's filter correctly.
 *
 * Input: device to be updated
 * RETURNS: N/A.
 *
 *
 **/

void
NetdrvAddrFilterSet(END_DEVICE *pDrvCtrl)
{
    ETHER_MULTI* pCurr;

    pCurr = END_MULTI_LST_FIRST (&pDrvCtrl->end);

    while (pCurr != NULL) {
        
        pCurr = END_MULTI_LST_NEXT(pCurr);
    }

    
}


/**
 *
 * NetdrvPollRcv - routine to receive a packet in polled mode.
 *
 * This routine is called by a user to try and get a packet from the
 * device.
 *
 * RETURNS: OK upon success.  EAGAIN is returned when no packet is available.
 *
 *
 **/

LOCAL STATUS
NetdrvPollRcv(END_DEVICE * pDrvCtrl, /* device to be polled */
              M_BLK_ID     pMblk)    /* ptr to buffer */
{
    u_short stat;
    char* pPacket;
    int len;

    DRV_LOG (DRV_DEBUG_POLL_RX,
            "NetdrvPollRcv ....... Not Implemented\n", 1, 2, 3, 4, 5, 6);

    stat = NetdrvStatusRead(pDrvCtrl);

    

    if (!(stat & NETDRV_RINT)) {
        printf("\r\nNetdrvPollRcv no data");
        return (EAGAIN);
    }

    /* Get packet and  length from device buffer/descriptor */

    pPacket = NULL; /* DUMMY CODE */
    len = 64;       /* DUMMY CODE */

    /* Upper layer must provide a valid buffer. */

    if ((pMblk->mBlkHdr.mLen < len) || (!(pMblk->mBlkHdr.mFlags & M_EXT))) {
#ifdef BOOTROM_DEBUG
        printf("\r\nPRX bad mblk");
#endif

        return (EAGAIN);
    }

    

    

    END_ERR_ADD(&pDrvCtrl->end, MIB2_IN_UCAST, +1);

    
    bcopy (pPacket, pMblk->m_data, len);
    pMblk->mBlkHdr.mFlags |= M_PKTHDR; /* set the packet header */
    pMblk->mBlkHdr.mLen    = len;      /* set the data len */
    pMblk->mBlkPktHdr.len  = len;      /* set the total len */

    
#ifdef BOOTROM_DEBUG
    printf("\r\nNetdrvPollRcv OK");
#endif

    return (OK);
}


/*******************************************************************************
 *
 * NetdrvPollSend - routine to send a packet in polled mode.
 *
 * This routine is called by a user to try and send a packet on the
 * device.
 *
 * RETURNS: OK upon success.  EAGAIN if device is busy.
 */

LOCAL STATUS
NetdrvPollSend(END_DEVICE* pDrvCtrl, /* device to be polled */
               M_BLK_ID    pMblk     /* packet to send */)
{
    int     len;
    u_short stat;

#ifdef BOOTROM_DEBUG
    printf("\r\nNetdrvPollSend");
#endif

    
    stat = NetdrvStatusRead(pDrvCtrl); /* dummy code */
    if((stat & NETDRV_TINT) == 0) {
        return ((STATUS) EAGAIN);
    }

    

    len = max(ETHERSMALL, pMblk->m_len);

    

    /* Bump the statistic counter. */

    END_ERR_ADD (&pDrvCtrl->end, MIB2_OUT_UCAST, +1);

    /* Free the data if it was accepted by device */

    netMblkClFree (pMblk);

#ifdef BOOTROM_DEBUG
    printf("\r\nleaving NetdrvPollSend");
#endif

    return (OK);
}


/**
 *
 * NetdrvMCastAdd - add a multicast address for the device
 *
 * This routine adds a multicast address to whatever the driver
 * is already listening for.  It then resets the address filter.
 *
 * RETURNS: OK or ERROR.
 *
 *
 **/

LOCAL STATUS
NetdrvMCastAdd(END_DEVICE *pDrvCtrl, /* device pointer */
               char* pAddress        /* new address to add */)
{
    int error;

    if ((error = etherMultiAdd (&pDrvCtrl->end.multiList, pAddress)) == ENETRESET) {
        NetdrvConfig (pDrvCtrl);
    }

    return (OK);
}


/**
 *
 * NetdrvMCastDel - delete a multicast address for the device
 *
 * This routine removes a multicast address from whatever the driver
 * is listening for.  It then resets the address filter.
 *
 * RETURNS: OK or ERROR.
 *
 *
 **/
LOCAL STATUS
NetdrvMCastDel(END_DEVICE *pDrvCtrl, /* device pointer */
               char* pAddress)       /* address to be deleted */
{
    int error;

    if ((error = etherMultiDel (&pDrvCtrl->end.multiList,
                                (char *)pAddress)) == ENETRESET) {
        NetdrvConfig(pDrvCtrl);
    }

    return (OK);
}


/**
 *
 * NetdrvMCastGet - get the multicast address list for the device
 *
 * This routine gets the multicast list of whatever the driver
 * is already listening for.
 *
 * RETURNS: OK or ERROR.
 */

LOCAL STATUS
NetdrvMCastGet(END_DEVICE *pDrvCtrl, /* device pointer */
               MULTI_TABLE* pTable   /*address table to be filled in*/)
{
    return (etherMultiGet(&pDrvCtrl->end.multiList, pTable));
}


/*******************************************************************************
 *
 * NetdrvStop - stop the device
 *
 * This function calls BSP functions to disconnect interrupts and stop
 * the device from operating in interrupt mode.
 *
 * Input: device to be stopped
 * RETURNS: OK or ERROR.
 */

LOCAL STATUS
NetdrvStop(END_DEVICE *pDrvCtrl)
{
    /* PRINTF_DEBUG(("NOT IMPLEMENTED YET.............\n")); */
    return (OK);
}


/******************************************************************************
 *
 * NetdrvUnload - unload a driver from the system
 *
 * This function first brings down the device, and then frees any
 * stuff that was allocated by the driver in the load function.
 *
 * Input: device to be unloaded
 * RETURNS: OK or ERROR.
 */

LOCAL STATUS
NetdrvUnload(END_DEVICE* pDrvCtrl)
{
    END_OBJECT_UNLOAD (&pDrvCtrl->end);

    

    return (OK);
}


/**
 *
 * NetdrvPollStart - start polled mode operations
 *
 * Input: device to be polled
 * RETURNS: OK or ERROR.
 *
 *
 **/

LOCAL STATUS
NetdrvPollStart(END_DEVICE * pDrvCtrl)
{
    int  oldLevel;

    PRINTF_DEBUG(("\r\n  NetdrvPollStart STARTED"));

    oldLevel = intLock (); /* disable ints during update */

    pDrvCtrl->flags |= NETDRV_POLLING;

    intUnlock (oldLevel); /* now NetdrvInt won't get confused */

    NetdrvConfig (pDrvCtrl); /* reconfigure device */

    return (OK);
}


/**
 *
 * NetdrvPollStop - stop polled mode operations
 *
 * This function terminates polled mode operation.  The device returns to
 * interrupt mode.
 *
 * The device interrupts are enabled, the current mode flag is switched
 * to indicate interrupt mode and the device is then reconfigured for
 * interrupt operation.
 *
 * Input: device to be polled
 * RETURNS: OK or ERROR.
 *
 **/

LOCAL STATUS
NetdrvPollStop(END_DEVICE * pDrvCtrl)
{
    int  oldLevel;

    oldLevel = intLock (); /* disable ints during register updates */

    
    pDrvCtrl->flags &= ~NETDRV_POLLING;

    intUnlock(oldLevel);

    NetdrvConfig(pDrvCtrl);

    PRINTF_DEBUG(("\r\nNO POLLING"));

    return (OK);
}


/**
 *
 * NetdrvStatusRead - get current device state/status
 *
 * RETURNS: status bits.
 *
 */
LOCAL UINT
NetdrvStatusRead(END_DEVICE *pDrvCtrl)
{
    
    return (0);
}
