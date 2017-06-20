/*
 * $Id: robotxrx.c,v 1.1 2004/02/24 07:47:00 csm Exp $
 * $Copyright: (c) 2003 Broadcom Corp.
 */

#include <stdlib.h>
#include <end.h>
#include <taskLib.h>
#include <muxLib.h>
#include <muxTkLib.h>
#include <netBufLib.h>
#include <errnoLib.h>
#include <time.h>
#include <cacheLib.h>
#include <logLib.h>
#include <intLib.h>



#define ROBO_TXRX_DEBUG
#define ROBO_TX_TEST_XXXX

#define ROBO_MII_DEVICE_NAME "et"
#define ROBO_MII_DEV_UNIT 1
#define LINK_HDR_LEN 14

#define ROBO_TX_RX_PK_SZ    2048
#define ROBO_TXRX_CLBLKS    256
#define ROBO_TXRX_MBLKS     256
#define ROBO_TXRX_CLBUFS    256


typedef void (*cb_f)(void *b, int l);
typedef void * (*alloc_f)(int l);
typedef void (*free_f)(void *b);

typedef struct robo_txrx_info_s {
    void *          roboMuxBindID;
    void *          roboNetPoolID;
    M_CL_CONFIG     m_cl_blks;
    CL_DESC         cluster;
    CL_POOL_ID      clpool_id;
    cb_f            rx_cb;
    alloc_f         rx_alloc;
    free_f          rx_free;
    void *          rx_head;
    void *          rx_tail;
    UINT32          queue_rx_pkts;
    UINT32          rx_out_of_mem;
    UINT32          rx_cnt;
    UINT32          tx_cnt;
    UINT32          tx_fail;
} robo_txrx_info_t;

LOCAL robo_txrx_info_t robo_txrx_info;
#ifdef ROBO_TXRX_DEBUG
int  robo_txrx_debug = 0;
#endif

#define ROBO_TXRX_DEBUG_F   0x00000001
#define ROBO_TXRX_DEBUG_P   0x00000002
#define ROBO_TXRX_PRINT if (robo_txrx_debug & ROBO_TXRX_DEBUG_P) logMsg

/* Public functions */
void roboTxRxInit();
int bcmRoboTx(UINT8 *b, int l);
void bcm_robo_mii_rx(UINT8 *b, int *l);
void bcm_robo_mii_rx_start();
void bcm_robo_mii_rx_stop();

int roboMIIRxHandlerRegister(cb_f cf, alloc_f af, free_f ff);
int roboMIIRxHandlerUnRegister(cb_f f);

#ifdef ROBO_TXRX_DEBUG
/* debug test functions */
void robo_tx_test(int l);
void robo_rx_test();
void robo_tx_netpool_show();

#include <stdio.h>

/* Debug */
LOCAL void
robo_txrx_pp(UINT8 *p, int l)
{
    int i;

    for(i=0; i < l; i++) {
        if ((i % 16) == 0) {
            printf("\n%02x :", i);
        }
        printf("%02x ", *p++);
    }
    printf("\n");
}

/* Debug */
LOCAL void
robo_txrx_print_packet(M_BLK_ID pMblk)
{
    unsigned char *p;

    pMblk->mBlkHdr.mFlags &=(~(M_BCAST | M_MCAST));

    printf("pklen=%d mLen=%d mType=%d mNext=%08x mData=%08x\n",
        pMblk->mBlkPktHdr.len,
        pMblk->mBlkHdr.mLen,
        pMblk->mBlkHdr.mType,
        (int)pMblk->mBlkHdr.mNext,
        (int)pMblk->mBlkHdr.mData);

    p = (unsigned char *)pMblk->mBlkHdr.mData;
    robo_txrx_pp(p, pMblk->mBlkPktHdr.len);
}

UINT8 robo_tx_test_pkbuf[512] = {
 0x00 ,0x01 ,0x02 ,0x03 ,0x04 ,0x05 ,0x06 ,0x07 
,0x08 ,0x09 ,0x0a ,0x0b ,0x0c ,0x0d ,0x0e ,0x0f 
,0x10 ,0x11 ,0x12 ,0x13 ,0x14 ,0x15 ,0x16 ,0x17 
,0x18 ,0x19 ,0x1a ,0x1b ,0x1c ,0x1d ,0x1e ,0x1f 
,0x20 ,0x21 ,0x22 ,0x23 ,0x24 ,0x25 ,0x26 ,0x27 
,0x28 ,0x29 ,0x2a ,0x2b ,0x2c ,0x2d ,0x2e ,0x2f 
,0x30 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 
,0x38 ,0x39 ,0x3a ,0x3b ,0x3c ,0x3d ,0x3e ,0x3f 
,0x40 ,0x41 ,0x42 ,0x43 ,0x44 ,0x45 ,0x46 ,0x47 
,0x48 ,0x49 ,0x4a ,0x4b ,0x4c ,0x4d ,0x4e ,0x4f 
,0x50 ,0x51 ,0x52 ,0x53 ,0x54 ,0x55 ,0x56 ,0x57 
,0x58 ,0x59 ,0x5a ,0x5b ,0x5c ,0x5d ,0x5e ,0x5f 
,0x60 ,0x61 ,0x62 ,0x63 ,0x64 ,0x65 ,0x66 ,0x67 
,0x68 ,0x69 ,0x6a ,0x6b ,0x6c ,0x6d ,0x6e ,0x6f 
,0x70 ,0x71 ,0x72 ,0x73 ,0x74 ,0x75 ,0x76 ,0x77 
,0x78 ,0x79 ,0x7a ,0x7b ,0x7c ,0x7d ,0x7e ,0x7f 
,0x80 ,0x81 ,0x82 ,0x83 ,0x84 ,0x85 ,0x86 ,0x87 
,0x88 ,0x89 ,0x8a ,0x8b ,0x8c ,0x8d ,0x8e ,0x8f 
,0x90 ,0x91 ,0x92 ,0x93 ,0x94 ,0x95 ,0x96 ,0x97 
,0x98 ,0x99 ,0x9a ,0x9b ,0x9c ,0x9d ,0x9e ,0x9f 
,0xa0 ,0xa1 ,0xa2 ,0xa3 ,0xa4 ,0xa5 ,0xa6 ,0xa7 
,0xa8 ,0xa9 ,0xaa ,0xab ,0xac ,0xad ,0xae ,0xaf 
,0xb0 ,0xb1 ,0xb2 ,0xb3 ,0xb4 ,0xb5 ,0xb6 ,0xb7 
,0xb8 ,0xb9 ,0xba ,0xbb ,0xbc ,0xbd ,0xbe ,0xbf 
,0xc0 ,0xc1 ,0xc2 ,0xc3 ,0xc4 ,0xc5 ,0xc6 ,0xc7 
,0xc8 ,0xc9 ,0xca ,0xcb ,0xcc ,0xcd ,0xce ,0xcf 
,0xd0 ,0xd1 ,0xd2 ,0xd3 ,0xd4 ,0xd5 ,0xd6 ,0xd7 
,0xd8 ,0xd9 ,0xda ,0xdb ,0xdc ,0xdd ,0xde ,0xdf 
,0xe0 ,0xe1 ,0xe2 ,0xe3 ,0xe4 ,0xe5 ,0xe6 ,0xe7 
,0xe8 ,0xe9 ,0xea ,0xeb ,0xec ,0xed ,0xee ,0xef 
,0xf0 ,0xf1 ,0xf2 ,0xf3 ,0xf4 ,0xf5 ,0xf6 ,0xf7 
,0xf8 ,0xf9 ,0xfa ,0xfb ,0xfc ,0xfd ,0xfe ,0xff 
,0x00 ,0x01 ,0x02 ,0x03 ,0x04 ,0x05 ,0x06 ,0x07 
,0x08 ,0x09 ,0x0a ,0x0b ,0x0c ,0x0d ,0x0e ,0x0f 
,0x10 ,0x11 ,0x12 ,0x13 ,0x14 ,0x15 ,0x16 ,0x17 
,0x18 ,0x19 ,0x1a ,0x1b ,0x1c ,0x1d ,0x1e ,0x1f 
,0x20 ,0x21 ,0x22 ,0x23 ,0x24 ,0x25 ,0x26 ,0x27 
,0x28 ,0x29 ,0x2a ,0x2b ,0x2c ,0x2d ,0x2e ,0x2f 
,0x30 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 ,0x36 ,0x37 
,0x38 ,0x39 ,0x3a ,0x3b ,0x3c ,0x3d ,0x3e ,0x3f 
,0x40 ,0x41 ,0x42 ,0x43 ,0x44 ,0x45 ,0x46 ,0x47 
,0x48 ,0x49 ,0x4a ,0x4b ,0x4c ,0x4d ,0x4e ,0x4f 
,0x50 ,0x51 ,0x52 ,0x53 ,0x54 ,0x55 ,0x56 ,0x57 
,0x58 ,0x59 ,0x5a ,0x5b ,0x5c ,0x5d ,0x5e ,0x5f 
,0x60 ,0x61 ,0x62 ,0x63 ,0x64 ,0x65 ,0x66 ,0x67 
,0x68 ,0x69 ,0x6a ,0x6b ,0x6c ,0x6d ,0x6e ,0x6f 
,0x70 ,0x71 ,0x72 ,0x73 ,0x74 ,0x75 ,0x76 ,0x77 
,0x78 ,0x79 ,0x7a ,0x7b ,0x7c ,0x7d ,0x7e ,0x7f 
,0x80 ,0x81 ,0x82 ,0x83 ,0x84 ,0x85 ,0x86 ,0x87 
,0x88 ,0x89 ,0x8a ,0x8b ,0x8c ,0x8d ,0x8e ,0x8f 
,0x90 ,0x91 ,0x92 ,0x93 ,0x94 ,0x95 ,0x96 ,0x97 
,0x98 ,0x99 ,0x9a ,0x9b ,0x9c ,0x9d ,0x9e ,0x9f 
,0xa0 ,0xa1 ,0xa2 ,0xa3 ,0xa4 ,0xa5 ,0xa6 ,0xa7 
,0xa8 ,0xa9 ,0xaa ,0xab ,0xac ,0xad ,0xae ,0xaf 
,0xb0 ,0xb1 ,0xb2 ,0xb3 ,0xb4 ,0xb5 ,0xb6 ,0xb7 
,0xb8 ,0xb9 ,0xba ,0xbb ,0xbc ,0xbd ,0xbe ,0xbf 
,0xc0 ,0xc1 ,0xc2 ,0xc3 ,0xc4 ,0xc5 ,0xc6 ,0xc7 
,0xc8 ,0xc9 ,0xca ,0xcb ,0xcc ,0xcd ,0xce ,0xcf 
,0xd0 ,0xd1 ,0xd2 ,0xd3 ,0xd4 ,0xd5 ,0xd6 ,0xd7 
,0xd8 ,0xd9 ,0xda ,0xdb ,0xdc ,0xdd ,0xde ,0xdf 
,0xe0 ,0xe1 ,0xe2 ,0xe3 ,0xe4 ,0xe5 ,0xe6 ,0xe7 
,0xe8 ,0xe9 ,0xea ,0xeb ,0xec ,0xed ,0xee ,0xef 
,0xf0 ,0xf1 ,0xf2 ,0xf3 ,0xf4 ,0xf5 ,0xf6 ,0xf7 
,0xf8 ,0xf9 ,0xfa ,0xfb ,0xfc ,0xfd ,0xfe ,0xff
};
#endif

/* Netpool create */
LOCAL int
roboTxRxNetpoolCreate()
{
    robo_txrx_info.roboNetPoolID =  (NET_POOL_ID)calloc(1, sizeof(NET_POOL));

    if (robo_txrx_info.roboNetPoolID == NULL) {
        return(0);
    }

    robo_txrx_info.m_cl_blks.mBlkNum = ROBO_TXRX_MBLKS;
    robo_txrx_info.m_cl_blks.clBlkNum = ROBO_TXRX_CLBLKS;
    robo_txrx_info.m_cl_blks.memSize =
            (robo_txrx_info.m_cl_blks.mBlkNum * (M_BLK_SZ + sizeof(long))) +
            (robo_txrx_info.m_cl_blks.clBlkNum * (CL_BLK_SZ + sizeof(long)));;
    robo_txrx_info.m_cl_blks.memArea = 
            (char *)memalign (sizeof (long), robo_txrx_info.m_cl_blks.memSize);
    if (robo_txrx_info.m_cl_blks.memArea == NULL) {
        free(robo_txrx_info.roboNetPoolID);
        robo_txrx_info.roboNetPoolID = NULL;
        return(0);
    }

    robo_txrx_info.cluster.clNum = ROBO_TXRX_CLBUFS;
    robo_txrx_info.cluster.clSize = ROBO_TX_RX_PK_SZ;
    robo_txrx_info.cluster.memSize = robo_txrx_info.cluster.clNum *
            (robo_txrx_info.cluster.clSize + sizeof(long)) + sizeof(long);
    robo_txrx_info.cluster.memArea = cacheDmaMalloc(
                                    robo_txrx_info.cluster.memSize);
    if (robo_txrx_info.cluster.memArea == NULL) {
        free(robo_txrx_info.roboNetPoolID);
        robo_txrx_info.roboNetPoolID = NULL;
        free(robo_txrx_info.m_cl_blks.memArea);
        robo_txrx_info.m_cl_blks.memArea = NULL;
        return(0);
    }

    if (netPoolInit(robo_txrx_info.roboNetPoolID,
                &robo_txrx_info.m_cl_blks,
                &robo_txrx_info.cluster, 1, NULL) != OK) {

        free(robo_txrx_info.roboNetPoolID);
        robo_txrx_info.roboNetPoolID = NULL;
        free(robo_txrx_info.m_cl_blks.memArea);
        robo_txrx_info.m_cl_blks.memArea = NULL;
        free(robo_txrx_info.cluster.memArea);
        robo_txrx_info.cluster.memArea = NULL;
        return(0);
    }
    robo_txrx_info.clpool_id = netClPoolIdGet(robo_txrx_info.roboNetPoolID,
                                robo_txrx_info.cluster.clSize, FALSE);
    return(1);
}

LOCAL int
_roboTx(M_BLK_ID pMblk)
{
    char zd[] = {0,0,0,0,0,0};
#ifdef ROBO_TXRX_DEBUG
    if (robo_txrx_debug & ROBO_TXRX_DEBUG_F) {
        logMsg("Sending a packet %08x\n", (int)pMblk, 2, 3, 4, 5, 6);
        robo_txrx_print_packet(pMblk);
    }
#endif

    if (muxTkSend( robo_txrx_info.roboMuxBindID, pMblk, zd, 0x0806, 0) == ERROR)
    {
        logMsg("muxTkSend failed \n", 1, 2, 3, 4, 5, 6);
        netMblkClChainFree(pMblk);
        robo_txrx_info.tx_fail++;
        return(0);
    }
    robo_txrx_info.tx_cnt++;
    return(1);
}

LOCAL BOOL
roboRecv(void *netCallbackId, long type, M_BLK_ID pMblk, void *pSpareData)
{
    UINT8 *buf;
    robo_txrx_info.rx_cnt++;

#if 0
    /* Adjust the data ptr and len to get back the ETH header */
    pMblk->mBlkHdr.mData        -= LINK_HDR_LEN;
    pMblk->mBlkHdr.mLen         += LINK_HDR_LEN;
    pMblk->mBlkPktHdr.len       += LINK_HDR_LEN;
#endif

#ifdef ROBO_TXRX_DEBUG
    if (robo_txrx_debug & ROBO_TXRX_DEBUG_F) {
        robo_txrx_print_packet(pMblk);
    }
#endif
    if (robo_txrx_info.queue_rx_pkts) {
        if (robo_txrx_info.rx_cb != NULL) {
            ROBO_TXRX_PRINT("Received a packet %08x\n", (int)pMblk, 2, 3, 4, 5, 6);
            robo_txrx_info.rx_cb(pMblk, pMblk->mBlkPktHdr.len);
        } else {
            netMblkClChainFree(pMblk);
        }
    } else {
        /* copy the packet and pass it to the  application (TCL) */
        /*                                                       */
        if (robo_txrx_info.rx_cb != NULL) {
            buf = robo_txrx_info.rx_alloc(pMblk->mBlkPktHdr.len);
            if (buf != NULL) { 
                netMblkToBufCopy(pMblk, buf, NULL);
                robo_txrx_info.rx_cb(buf, pMblk->mBlkPktHdr.len);
            } else {
                robo_txrx_info.rx_out_of_mem++;
            }
        }
        /*                                                       */
        /* copy the packet and pass it to the  application (TCL) */

        netMblkClChainFree(pMblk);
    }

    return (1);
}

LOCAL BOOL
roboTxShutdownRtn(void *netCallbackId)
{
    return (1);
}

LOCAL BOOL
roboTxRestartRtn(void *netCallbackId)
{
    return (1);
}

LOCAL void
roboErrorRtn(void *netCallbackId, END_ERR * pError)
{
    return;
}

/* Publicly exported functions */

void roboTxRxInit()
{
    END_OBJ *pEndObj = NULL;

    if (robo_txrx_info.roboMuxBindID != NULL) {
        return;
    }

    roboTxRxNetpoolCreate();

    /* Bind roboTxRx Network service to the END driver. */
    robo_txrx_info.roboMuxBindID = (void *) muxTkBind(
                        ROBO_MII_DEVICE_NAME, ROBO_MII_DEV_UNIT,
                        roboRecv, roboTxShutdownRtn,
                        roboTxRestartRtn, roboErrorRtn,
                        MUX_PROTO_SNARF, "ROBO TX/RX",
                        pEndObj,(void *) 0, (void *) 0);
    if (!robo_txrx_info.roboMuxBindID)
    {
        logMsg("muxTkBind Failed (%08x).\n", errnoGet(), 2, 3, 4, 5, 6);
        return;
    }

    /* Promiscuous mode */
    muxIoctl( robo_txrx_info.roboMuxBindID, EIOCSFLAGS, (void *)IFF_PROMISC);
}

int
bcmRoboTx(UINT8 *b, int l)
{
    M_BLK_ID pMblk;

    pMblk = netTupleGet(robo_txrx_info.roboNetPoolID, l,
                        M_DONTWAIT, MT_DATA,FALSE);
    if (pMblk == NULL) {
        robo_txrx_info.tx_fail++;
        return(0);
    }
    /* cache invalidate */
    /* CACHE_INVALIDATE(pMblk->m_data, l); */
    pMblk->mBlkHdr.mFlags  |= M_PKTHDR;
    pMblk->mBlkHdr.mLen     = l;
    pMblk->mBlkPktHdr.len   = l;

    memcpy(pMblk->m_data, b, l);
    return (_roboTx(pMblk));
}

int
roboMIIRxHandlerRegister(cb_f cf, alloc_f af, free_f ff)
{
    if (robo_txrx_info.rx_cb == NULL) {
        robo_txrx_info.rx_alloc = af;
        robo_txrx_info.rx_free = ff;
        robo_txrx_info.rx_cb = cf;
        return(1);
    } else {
        return(0);
    }
}
int
roboMIIRxHandlerUnRegister(cb_f f)
{
    if (robo_txrx_info.rx_cb == f) {
        robo_txrx_info.rx_cb = NULL;
        return(1);
    } else {
        return(0);
    }
}

void robo_rx_drain_pkts()
{
    M_BLK_ID pMblk, cMblk;
    int s;

    s = intLock();
    pMblk = robo_txrx_info.rx_head;
    robo_txrx_info.rx_tail = robo_txrx_info.rx_head = NULL;
    while(pMblk) {
        cMblk = pMblk;
        netMblkClChainFree(cMblk);
        pMblk = pMblk->mBlkHdr.mNextPkt;
    }
}

void robo_rx_q_pkts(M_BLK_ID pMblk, int dummy)
{
    M_BLK_ID qMblk;
    int s;

    s = intLock();
    if (robo_txrx_info.rx_tail == NULL ) {
        robo_txrx_info.rx_tail = pMblk;
        robo_txrx_info.rx_head = pMblk;
    } else {
        qMblk = robo_txrx_info.rx_tail;
        qMblk->mBlkHdr.mNextPkt = pMblk;
        robo_txrx_info.rx_tail = pMblk;
        pMblk->mBlkHdr.mNextPkt = NULL;
    }
    intUnlock(s);

    ROBO_TXRX_PRINT("Queued a packet %08x\n", (int)pMblk, 2, 3, 4, 5, 6);

    /* Indicate that a packet is available */
    /* semTake(robo_txrx_info.rx_sem, WAIT_FOREVER); */
}

void bcm_robo_mii_rx(UINT8 *b, int *l)
{
    M_BLK_ID pMblk;
    int s;
    int n = 10;

    *l = 0;

    while(*l == 0) {
        pMblk = robo_txrx_info.rx_head;
        if (pMblk != NULL) {
            s = intLock();
            robo_txrx_info.rx_head = pMblk->mBlkHdr.mNextPkt;
            if (robo_txrx_info.rx_head == NULL) {
                robo_txrx_info.rx_tail = NULL;
            }
            intUnlock(s);


            *l = pMblk->mBlkPktHdr.len;
            netMblkToBufCopy(pMblk, b, NULL);
            netMblkClChainFree(pMblk);
            ROBO_TXRX_PRINT("DeQueued a packet %08x len = %d\n",
                            (int)pMblk, *l, 3, 4, 5, 6);
        } else {
            /* Wait for a packet */
            /* semTake(robo_txrx_info.rx_sem); */
            taskDelay(1);
            if (!(n--)) break;
        }
    }
}


void bcm_robo_mii_rx_start()
{
    ROBO_TXRX_PRINT("bcm_robo_mii_rx_start\n", 1, 2, 3, 4, 5, 6);
    robo_txrx_info.queue_rx_pkts = 1;
    roboMIIRxHandlerRegister(robo_rx_q_pkts, NULL, NULL);
}

void bcm_robo_mii_rx_stop()
{
    ROBO_TXRX_PRINT("bcm_robo_mii_rx_stop\n", 1, 2, 3, 4, 5, 6);
    if (roboMIIRxHandlerUnRegister(robo_rx_q_pkts)) {
        robo_txrx_info.queue_rx_pkts = 0;
        robo_rx_drain_pkts();
    }
}


#ifdef ROBO_TXRX_DEBUG
/* debug */

LOCAL void
robo_rx_test_cb(char *buf, int len)
{
    logMsg("robo_rx_test_cb:",1,2,3,4,5,6);
    robo_txrx_pp(buf, len);
    free(buf);
}

void
robo_tx_netpool_show()
{
    netPoolShow(robo_txrx_info.roboNetPoolID);
}

void
robo_rx_test()
{
    roboMIIRxHandlerRegister(robo_rx_test_cb, malloc, free);
}


void
robo_tx_test(int l)
{
    if (l > 512) {
        l = 64;
    }
    if (!bcmRoboTx(robo_tx_test_pkbuf, l)) {
        logMsg("Robo TX failed",1,2,3,4,5,6);
    }
}
#endif
