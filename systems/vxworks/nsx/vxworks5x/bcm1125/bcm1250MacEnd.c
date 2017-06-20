/* bcm1250MacEnd.c - END style BCM1250 MAC Ethernet driver */
/* Copyright 2001 Wind River Systems, Inc. */
#include "copyright_wrs.h"
/* $Id: bcm1250MacEnd.c,v 1.4 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */
/*
modification history
--------------------
01a,15nov01,agf  written.
*/
/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and
*  conditions of this license.  No title or ownership is transferred hereby.
********************************************************************* */     
/*
DESCRIPTION
This module implements the Broadcom bcm1250 on-chip ethernet MACs.
*/
#include "vxWorks.h"

#include "wdLib.h"
#include "iv.h"
#include "vme.h"
#include "net/mbuf.h"
#include "net/unixLib.h"
#include "net/protosw.h"
#include "sys/socket.h"
#include "sys/ioctl.h"
#include "errno.h"
#include "memLib.h"
#include "intLib.h"
#include "net/route.h"
#include "errnoLib.h"
#include "cacheLib.h"
#include "logLib.h"
#include "netLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "sysLib.h"
#include "etherLib.h"
#include "net/systm.h"
#include "sys/times.h"
#include "net/if_subr.h"
#include "miiLib.h"

#include "bcm1250Lib.h"

#undef  ETHER_MAP_IP_MULTICAST
#include "etherMultiLib.h"
#include "end.h"
#include "endLib.h"
#include "lstLib.h"
#include "semLib.h"

#include "bcm1250IntLib.h"
#include "config.h"
#include "end.h"
#include "netBufLib.h"

#include "bcm1250MacEnd.h"
/*
#define PHY_INT
*/
#undef DRV_DEBUG

#ifdef  DRV_DEBUG
#define DRV_DEBUG_OFF           0x0000
#define DRV_DEBUG_RX            0x0001
#define DRV_DEBUG_TX            0x0002
#define DRV_DEBUG_INT           0x0004
#define DRV_DEBUG_POLL          (DRV_DEBUG_POLL_RX | DRV_DEBUG_POLL_TX)
#define DRV_DEBUG_POLL_RX       0x0008
#define DRV_DEBUG_POLL_TX       0x0010
#define DRV_DEBUG_LOAD          0x0020
#define DRV_DEBUG_IOCTL         0x0040
#define DRV_DEBUG_RXD           0x0100
#define DRV_DEBUG_TXD           0x0200
#define DRV_DEBUG_POLL_REDIR   0x10000
#define DRV_DEBUG_LOG_NVRAM    0x20000

/*
LOCAL int     bcm1250MacDebug = DRV_DEBUG_LOAD | DRV_DEBUG_INT | DRV_DEBUG_TX |
                               DRV_DEBUG_RX | DRV_DEBUG_POLL;
*/

LOCAL int     bcm1250MacDebug = DRV_DEBUG_RXD | DRV_DEBUG_TXD;

#define DRV_LOG(FLG, X0, X1, X2, X3, X4, X5, X6)                        \
        if (bcm1250MacDebug & FLG)                                             \
            logMsg(X0, X1, X2, X3, X4, X5, X6);
#else /*DRV_DEBUG*/
#define DRV_LOG(DBG_SW, X0, X1, X2, X3, X4, X5, X6)
#endif /*DRV_DEBUG*/


/* DRV_CTRL flags access macros */
#define DRV_FLAGS_SET(setBits) \
        (pDrvCtrl->flags |= (setBits))

#define DRV_FLAGS_ISSET(setBits) \
        (pDrvCtrl->flags & (setBits))

#define DRV_FLAGS_CLR(clrBits) \
        (pDrvCtrl->flags &= ~(clrBits))

#define DRV_FLAGS_GET() \
        (pDrvCtrl->flags)

#define END_FLAGS_ISSET(pEnd, setBits) \
        ((pEnd)->flags & (setBits))

#define END_HADDR(pEnd) \
        ((pEnd)->mib2Tbl.ifPhysAddress.phyAddress)

#define END_HADDR_LEN(pEnd) \
        ((pEnd)->mib2Tbl.ifPhysAddress.addrLength)

/* DRV_CTRL user flags access macros */
#define DRV_USR_FLAGS_SET(setBits) \
        (pDrvCtrl->usrFlags |= (setBits))

#define DRV_USR_FLAGS_ISSET(setBits) \
        (pDrvCtrl->usrFlags & (setBits))

#define DRV_USR_FLAGS_CLR(clrBits) \
        (pDrvCtrl->usrFlags &= ~(clrBits))

#define DRV_USR_FLAGS_GET() \
        (pDrvCtrl->usrFlags)

#define DRV_PHY_FLAGS_ISSET(setBits) \
        (pDrvCtrl->miiPhyFlags & (setBits))

#if 1
#define K0TOPHYS(x) ((UINT32)x & 0x7fffffff)
#define PHYSTOK0(x) ((UINT32)x | 0x80000000)
#else
#define K0THYS(x) ((UINT32)x & 0x1fffffff)
#define PHYSTOK0(x) ((UINT32)x | 0xa0000000)
#endif

#define KVTOPHYS(x) K0TOPHYS(x)
#define PHYSTOV(x)  PHYSTOK0(x)

#define SBETH_READCSR(t) (*((volatile UINT64 *) (unsigned long) (t)))
#define SBETH_WRITECSR(t,v) *((volatile UINT64 *) (unsigned long) (t)) = (v)

#ifdef PHY_INT
 
#define SB_GPIO_REG_READ(reg) \
    MIPS3_LD(PHYS_TO_K1(A_GPIO_BASE)+reg)
 
#define SB_GPIO_REG_WRITE(reg,val) \
    MIPS3_SD((PHYS_TO_K1(A_GPIO_BASE)+reg), (val))

#endif                                                                          

#ifndef SB_MAC_REG_READ
#define SB_MAC_REG_READ(reg)  \
    MIPS3_LD(pDrvCtrl->sbm_macbase + reg)
#endif /* BCM1250_MAC_REG_READ */

#ifndef SB_MAC_REG_WRITE
#define SB_MAC_REG_WRITE(reg,val)  \
    MIPS3_SD((pDrvCtrl->sbm_macbase + reg), (val))
#endif /* SB1_MAC_REG_WRITE */


#ifndef SB_DMA_REG_READ
#define SB_DMA_REG_READ(reg)  \
    MIPS3_LD(pDrvCtrl->sbm_dmabase + reg)
#endif /* SB_DMA_REG_READ */

#ifndef SB_DMA_REG_WRITE
#define SB_DMA_REG_WRITE(reg,val)  \
    MIPS3_SD((pDrvCtrl->sbm_dmabase + reg), (val))
#endif /* SB_DMA_REG_WRITE */


#define NET_BUF_ALLOC() \
    netClusterGet (pDrvCtrl->endObj.pNetPool, pDrvCtrl->clPoolId)

#define NET_BUF_FREE(pBuf) \
    netClFree (pDrvCtrl->endObj.pNetPool, pBuf)

#define NET_MBLK_ALLOC() \
    mBlkGet (pDrvCtrl->endObj.pNetPool, M_DONTWAIT, MT_DATA)

#define NET_MBLK_FREE(pMblk) \
    netMblkFree (pDrvCtrl->endObj.pNetPool, (M_BLK_ID)pMblk)

#define NET_CL_BLK_ALLOC() \
    clBlkGet (pDrvCtrl->endObj.pNetPool, M_DONTWAIT)

#define NET_CL_BLK_FREE(pClblk) \
    clBlkFree (pDrvCtrl->endObj.pNetPool, (CL_BLK_ID)pClBlk)

#define NET_MBLK_BUF_FREE(pMblk) \
    netMblkClFree ((M_BLK_ID)pMblk)

#define NET_MBLK_CL_JOIN(pMblk, pClBlk) \
    netMblkClJoin ((pMblk), (pClBlk))

#define NET_CL_BLK_JOIN(pClBlk, pBuf, len) \
    netClBlkJoin ((pClBlk), (pBuf), (len), NULL, 0, 0, 0)

/* PHY/MII */

#define MII_COMMAND_START       0x01
#define MII_COMMAND_READ        0x02
#define MII_COMMAND_WRITE       0x01
#define MII_COMMAND_ACK         0x02                                           

#define BMCR_RESET     0x8000
#define BMCR_LOOPBACK  0x4000
#define BMCR_SPEED0    0x2000
#define BMCR_ANENABLE  0x1000
#define BMCR_POWERDOWN 0x0800
#define BMCR_ISOLATE   0x0400
#define BMCR_RESTARTAN 0x0200
#define BMCR_DUPLEX    0x0100
#define BMCR_COLTEST   0x0080
#define BMCR_SPEED1    0x0040
#define BMCR_SPEED1000 (BMCR_SPEED1|BMCR_SPEED0)
#define BMCR_SPEED100  (BMCR_SPEED0)
#define BMCR_SPEED10    0
 
#define BMSR_100BT4     0x8000
#define BMSR_100BT_FDX  0x4000
#define BMSR_100BT_HDX  0x2000
#define BMSR_10BT_FDX   0x1000
#define BMSR_10BT_HDX   0x0800
#define BMSR_100BT2_FDX 0x0400
#define BMSR_100BT2_HDX 0x0200
#define BMSR_1000BT_XSR 0x0100
#define BMSR_PRESUP     0x0040
#define BMSR_ANCOMPLT   0x0020
#define BMSR_REMFAULT   0x0010
#define BMSR_AUTONEG    0x0008
#define BMSR_LINKSTAT   0x0004
#define BMSR_JABDETECT  0x0002
#define BMSR_EXTCAPAB   0x0001                                               

#define K1STSR_MSMCFLT  0x8000
#define K1STSR_MSCFGRES 0x4000
#define K1STSR_LRSTAT   0x2000
#define K1STSR_RRSTAT   0x1000
#define K1STSR_LP1KFD   0x0800
#define K1STSR_LP1KHD   0x0400
#define K1STSR_LPASMDIR 0x0200

#define ANLPAR_NP       0x8000
#define ANLPAR_ACK      0x4000
#define ANLPAR_RF       0x2000
#define ANLPAR_ASYPAUSE 0x0800
#define ANLPAR_PAUSE    0x0400
#define ANLPAR_T4       0x0200
#define ANLPAR_TXFD     0x0100
#define ANLPAR_TXHD     0x0080
#define ANLPAR_10FD     0x0040
#define ANLPAR_10HD     0x0020
#define ANLPAR_PSB      0x0001  /* 802.3 */
                                                                 
#define PHYIDR1         0x2000
#define PHYIDR2         0x5C60                                 

#define MII_BMCR        0x00    /* Basic mode control register (rw) */
#define MII_BMSR        0x01    /* Basic mode status register (ro) */
#define MII_K1STSR      0x0A    /* 1K Status Register (ro) */
#define MII_ANLPAR      0x05    /* Autonegotiation lnk partner abilities (rw) */
  
#define M_MAC_MDIO_DIR_OUTPUT   0               /* for clarity */      

#ifdef PHY_INT
#define MII_ISR         0x1A
#define MII_IMR         0x1B
#define PHY_BMSR_LS     0x04
#endif                                                                          


/* for debug */
void sbrdshow (int inst, int chan);
void sbtdshow (int inst, int chan);
void sbmshow (int inst);
void sbphyshow (int inst);

/* LOCALS */

/* forward static functions */
LOCAL STATUS bcm1250MacMemInit(DRV_CTRL * pDrvCtrl);
LOCAL STATUS bcm1250MacDmaInit(DRV_CTRL * pDrvCtrl);
LOCAL STATUS bcm1250MacPktCopyTransmit(DRV_CTRL * pDrvCtrl, M_BLK * pMblk, 
                                       UINT16 pktType, int channel);
LOCAL void bcm1250MacTxHandle(DRV_CTRL *pDrvCtrl, int channel);
LOCAL void bcm1250MacRxHandle(DRV_CTRL *pDrvCtrl, int channel);
LOCAL STATUS bcm1250MacInitParse(DRV_CTRL *pDrvCtrl, char *initString);
LOCAL void bcm1250MacInt(DRV_CTRL *  pDrvCtrl);      
LOCAL void bcm1250MacRxFilterSet(DRV_CTRL *pDrvCtrl);
LOCAL UINT16 availableSbDmaDscrs(DRV_CTRL *pDrvCtrl, SB_MAC_DMA *pDma);
LOCAL STATUS bcm1250MacMblkWalk(M_BLK *pMblk, UINT8 *pFragNum, UINT16 *pPktType,
                                BOOL *pZeroCopyReady);  	
LOCAL STATUS bcm1250MacPktTransmit(DRV_CTRL *pDrvCtrl, M_BLK *pMblk, 
                                   UINT16 pktType,UINT8 fragNum,int channel);
    
LOCAL void sbeth_mii_poll(DRV_CTRL * pDrvCtrl);
LOCAL void sbeth_mii_findphy(DRV_CTRL * pDrvCtrl);

/* This is the only externally visible interface. */

END_OBJ*        bcm1250MacEndLoad (char* initString);

/* END Specific interfaces. */

LOCAL STATUS    bcm1250MacStart   (DRV_CTRL* pDrvCtrl);
LOCAL STATUS    bcm1250MacStop    (DRV_CTRL* pDrvCtrl);
LOCAL int       bcm1250MacIoctl   (DRV_CTRL* pDrvCtrl, int cmd, caddr_t data);
LOCAL STATUS    bcm1250MacUnload  (DRV_CTRL* pDrvCtrl);
LOCAL STATUS    bcm1250MacSend    (DRV_CTRL* pDrvCtrl, M_BLK_ID pBuf);

LOCAL STATUS    bcm1250MacMCastAdd (DRV_CTRL* pDrvCtrl, char* pAddress);
LOCAL STATUS    bcm1250MacMCastDel (DRV_CTRL* pDrvCtrl, char* pAddress);
LOCAL STATUS    bcm1250MacMCastGet (DRV_CTRL* pDrvCtrl, MULTI_TABLE* pTable);
LOCAL STATUS    bcm1250MacPollStart (DRV_CTRL* pDrvCtrl);
LOCAL STATUS    bcm1250MacPollStop (DRV_CTRL* pDrvCtrl);
LOCAL STATUS    bcm1250MacPollSend (DRV_CTRL* pDrvCtrl, M_BLK_ID pBuf);
LOCAL STATUS    bcm1250MacPollRcv (DRV_CTRL* pDrvCtrl, M_BLK_ID pBuf);

LOCAL int bcm1250MacSetDuplex(DRV_CTRL *pDrvCtrl, sbmac_duplex_t duplex, 
                              sbmac_fc_t fc);
LOCAL int bcm1250MacSetSpeed(DRV_CTRL *pDrvCtrl, sbmac_speed_t speed);

#ifdef PHY_INT
LOCAL STATUS bPHYConnect(DRV_CTRL* pDrvCtrl);
#endif

/*
 * Define the device function table.  This is static across all driver
 * instances.
 */

LOCAL NET_FUNCS bcm1250MacFuncTable =
    {
    (FUNCPTR) bcm1250MacStart,          /* Function to start the device. */
    (FUNCPTR) bcm1250MacStop,           /* Function to stop the device. */
    (FUNCPTR) bcm1250MacUnload,         /* Unloading function for the driver. */
    (FUNCPTR) bcm1250MacIoctl,          /* Ioctl function for the driver. */
    (FUNCPTR) bcm1250MacSend,           /* Send function for the driver. */

    (FUNCPTR) bcm1250MacMCastAdd,       /* Multicast add function for the */
    (FUNCPTR) bcm1250MacMCastDel,       /* Multicast delete function for */
    (FUNCPTR) bcm1250MacMCastGet,       /* Multicast retrieve function for */

    (FUNCPTR) bcm1250MacPollSend,       /* Polling send function */
    (FUNCPTR) bcm1250MacPollRcv,        /* Polling receive function */

    endEtherAddressForm,                /* put address info into a NET_BUFFER */
    (FUNCPTR) endEtherPacketDataGet,    /* get pointer to data in NET_BUFFER */
    (FUNCPTR) endEtherPacketAddrGet     /* Get packet addresses. */
    };


IMPORT STATUS   sysBcm1250MacEnetAddrGet (int unit, char *enetAdrs);

#ifdef L3P_TEST
/* zchen added for l3p debug */
#include "tickLib.h"
#define M_DMA_DSCRA_INTERRUPT 0
int prevTick, rxIntCount; 
#endif

LOCAL STATUS bcm1250MacEnetAddrGet
    (
    DRV_CTRL *pDrvCtrl,
    char *enetAdrs
    )
    {
    sysBcm1250MacEnetAddrGet(pDrvCtrl->unit, enetAdrs);

    return (OK);
    }

static UINT64 sbmac_addr2reg
    (
    unsigned char *ptr
    )
    {
    UINT64 reg = 0;

    ptr += 6;

    reg |= (UINT64) *(--ptr);
    reg <<= 8;
    reg |= (UINT64) *(--ptr);
    reg <<= 8;
    reg |= (UINT64) *(--ptr);
    reg <<= 8;
    reg |= (UINT64) *(--ptr);
    reg <<= 8;
    reg |= (UINT64) *(--ptr);
    reg <<= 8;
    reg |= (UINT64) *(--ptr);

    return reg;
    }

void bcm1250MacReset
    (
    DRV_CTRL * pDrvCtrl
    )
    {
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_macenable, M_MAC_PORT_RESET);
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_macenable, 0);
    }


/*******************************************************************************
*
* bcm1250MacEndLoad - initialize the driver and device
*
* This routine initializes the driver and the device to the operational state.
* All of the device specific parameters are passed in the initString.
*
* The string contains the target specific parameters like this:
*
* "register addr:int vector:int level:shmem addr:shmem size:shmem width"
*
* RETURNS: An END object pointer or NULL on error.
*/
END_OBJ* bcm1250MacEndLoad
    (
    char* initString            /* String to be parsed by the driver. */
    )
    {
    DRV_CTRL  *pDrvCtrl;
    char      enetAddr[6];

    DRV_LOG (DRV_DEBUG_LOAD, "Loading bcm1250MacEnd...\n", 1, 2, 3, 4, 5, 6);

    if (initString == NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacEndLoad: NULL initStr\n",
                 1, 2, 3, 4, 5, 6);
        return (NULL);
        }

    if (initString[0] == '\0')
        {
        bcopy((char *)SB_DEV_NAME, initString, SB_DEV_NAME_LEN);
        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacEndLoad: initString[0]==NUL\n",
                 1, 2, 3, 4, 5, 6);
        return (NULL);
        }

    /* allocate the device structure */
    pDrvCtrl = (DRV_CTRL *)calloc (sizeof (DRV_CTRL), 1);
    if (pDrvCtrl == NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, 
                 "bcm1250MacEndLoad: fail to alloc mem for DRV\n",
                 1, 2, 3, 4, 5, 6);
        goto errorExit;
        }

    DRV_LOG (DRV_DEBUG_LOAD, "Loading bcm1250MacEnd: pDrvCtrl=0x%x\n", 
             (int)pDrvCtrl, 2, 3, 4, 5, 6);

    /* parse the init string, filling in the device structure */
    if (bcm1250MacInitParse (pDrvCtrl, initString) == ERROR)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacEndLoad: init parse failed\n",
                 1, 2, 3, 4, 5, 6);
        goto errorExit;
        }

    /* Ask the BSP to provide the ethernet address. */
    bcm1250MacEnetAddrGet (pDrvCtrl, (char *)  enetAddr);

    DRV_LOG (DRV_DEBUG_LOAD, 
             "Loading bcm1250MacEnd: mac addr = %x:%x:%x:%x:%x:%x\n",
             enetAddr[0], enetAddr[1], enetAddr[2], enetAddr[3], enetAddr[4], 
             enetAddr[5]);

    /* initialize the END and MIB2 parts of the structure */

    /*
     * The M2 element must come from m2Lib.h
     * This template is set up for a DIX type ethernet device.
     */
    if ( END_OBJ_INIT(&pDrvCtrl->endObj, (DEV_OBJ *)pDrvCtrl, SB_DEV_NAME,
                      pDrvCtrl->unit, &bcm1250MacFuncTable, 
                      "BCM1250 END Driver.") == ERROR )
        goto errorExit;

    /* just reset mac once */
    bcm1250MacReset(pDrvCtrl);
    
    /* Perform memory allocation/distribution */

    if (bcm1250MacMemInit (pDrvCtrl) == ERROR)
        goto errorExit;

    if (bcm1250MacDmaInit (pDrvCtrl) == ERROR)
        goto errorExit;

#if 1
    sbeth_mii_findphy(pDrvCtrl);
    sbeth_mii_poll(pDrvCtrl);
#else
    pDrvCtrl->phyAddr = 1;
    pDrvCtrl->sbm_speed = sbmac_speed_100;
    pDrvCtrl->sbm_duplex = sbmac_duplex_full;
    pDrvCtrl->sbm_fc = sbmac_fc_disabled;
#endif 


    if (END_MIB_INIT (&pDrvCtrl->endObj, M2_ifType_ethernet_csmacd,
                      (UINT8*) &enetAddr[0], 6,
                       ETHERMTU, BCM1250_MAC_SPEED_DEF) == ERROR)
        {
        DRV_LOG  (DRV_DEBUG_LOAD, 
                  "bcm1250MacEndLoad: MIB init failed\n", 
                  1, 2, 3, 4, 5, 6);
        goto errorExit;
        }

#ifdef PHY_INT
    if (pDrvCtrl->unit == 1)
    {
    UINT64 val;
 
    val = SB_GPIO_REG_READ(R_GPIO_DIRECTION);
    val &= ~(7 << 2);
    logMsg("gpio direction = 0x%04x \n", val, 2,3,4,5,6);
    SB_GPIO_REG_WRITE(R_GPIO_DIRECTION,val);

    val = SB_GPIO_REG_READ(R_GPIO_CLR_EDGE);
    val |= 4;
    logMsg("gpio clr edge = 0x%04x \n", val, 2,3,4,5,6);
    SB_GPIO_REG_WRITE(R_GPIO_CLR_EDGE,val);

    val = SB_GPIO_REG_READ(R_GPIO_INPUT_INVERT);
    val |= 4;
    logMsg("gpio input invert = 0x%04x \n", val, 2,3,4,5,6);
    SB_GPIO_REG_WRITE(R_GPIO_INPUT_INVERT,val);

    val = SB_GPIO_REG_READ(R_GPIO_INT_TYPE);
/*
    val &=~M_GPIO_INTR_TYPE2;
    val |= V_GPIO_INTR_TYPE2(K_GPIO_INTR_EDGE);
*/
    val &= ~0xc;
    val |= 0x8;
    logMsg("gpio int type = 0x%04x \n", val, 2,3,4,5,6);
    SB_GPIO_REG_WRITE(R_GPIO_INT_TYPE,val);

    pDrvCtrl->intphysource = 34;
    }
#endif                                                                          

    /* set the flags to indicate readiness */
    END_OBJ_READY (&pDrvCtrl->endObj,
                    IFF_UP | IFF_RUNNING | IFF_NOTRAILERS | IFF_BROADCAST
                    | IFF_MULTICAST);
    DRV_LOG (DRV_DEBUG_LOAD, "Done loading ...", 1, 2, 3, 4, 5, 6);

    return (&pDrvCtrl->endObj);

errorExit:
    if (pDrvCtrl != NULL)
        free ((char *)pDrvCtrl);

    return NULL;
    }


/*******************************************************************************
* bcm1250MacInitParse
* parsing init string, the string is of format
* "<drvunit>:<hwunit>:<ivecnum>:<user_flags>:
* <numRds0>:<numTds0>:<numRds1>:<numTds1>"
*/
LOCAL STATUS bcm1250MacInitParse
    (
    DRV_CTRL	*pDrvCtrl,
    char	*initString
    )
    {
    char *	tok;		/* an initString token */
    char *	holder=NULL;	/* points to initString fragment beyond tok */

    SB_MAC_DMA *pRxDma0 = &pDrvCtrl->rxDma[0];
    SB_MAC_DMA *pRxDma1 = &pDrvCtrl->rxDma[1];
    SB_MAC_DMA *pTxDma0 = &pDrvCtrl->txDma[0];
    SB_MAC_DMA *pTxDma1 = &pDrvCtrl->txDma[1];

    DRV_LOG (DRV_DEBUG_LOAD, "InitParse: Initstr=%s\n",
             (int)initString, 0, 0, 0, 0, 0);

    tok = strtok_r(initString, ":", &holder);
    if (tok == NULL)
        return ERROR;
    pDrvCtrl->unit = atoi(tok);

    tok=strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return ERROR;
    pDrvCtrl->devNo = atoi(tok);

    switch (pDrvCtrl->devNo) 
        {
        case 0:
	    pDrvCtrl->sbm_macbase =
                PHYS_TO_K1(A_MAC_CHANNEL_BASE(pDrvCtrl->devNo));
            pDrvCtrl->sbm_dmabase = pDrvCtrl->sbm_macbase;
            pDrvCtrl->intsource = K_INT_MAC_0;
            break;
        case 1:
	    pDrvCtrl->sbm_macbase =
	        PHYS_TO_K1(A_MAC_CHANNEL_BASE(pDrvCtrl->devNo));
            pDrvCtrl->sbm_dmabase = pDrvCtrl->sbm_macbase;
            pDrvCtrl->intsource = K_INT_MAC_1;
            break;
        case 2:
            pDrvCtrl->sbm_macbase =
                PHYS_TO_K1(A_MAC_CHANNEL_BASE(pDrvCtrl->devNo));
            pDrvCtrl->sbm_dmabase = pDrvCtrl->sbm_macbase;
            pDrvCtrl->intsource = K_INT_MAC_2;
            break;
        default:
            return ERROR;
    }

    tok=strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return ERROR;
    pDrvCtrl->ivecnum = strtoul (tok, NULL, 10);

    tok=strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return (ERROR);
    pDrvCtrl->usrFlags = strtoul(tok, NULL, 16);

    /* channel 0 RX/TX descriptors */
    tok = strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return ERROR;
    if (atoi(tok) < 0)
        pRxDma0->sbdma_maxdescr = NUM_RDS_DEF;
    else
        pRxDma0->sbdma_maxdescr = atoi(tok);

    tok = strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return ERROR;
    if (atoi(tok) < 0)
        pTxDma0->sbdma_maxdescr = NUM_TDS_DEF;
    else
        pTxDma0->sbdma_maxdescr = atoi(tok);

    /* channel 1 RX/TX descriptors */
    tok = strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return ERROR;
    if (atoi(tok) < 0)
        pRxDma1->sbdma_maxdescr = NUM_RDS_DEF;
    else
        pRxDma1->sbdma_maxdescr = atoi(tok);

    tok = strtok_r(NULL, ":", &holder);
    if (tok == NULL)
        return ERROR;
    if (atoi(tok) < 0)
        pTxDma1->sbdma_maxdescr = NUM_TDS_DEF;
    else
        pTxDma1->sbdma_maxdescr = atoi(tok);

    DRV_LOG (DRV_DEBUG_LOAD,
             "EndLoad: flags=0x%x usrFlags=0x%x dmaBase=0x%x macBase=0x%x ivecnum=0x%x intsource=0x%x ",
             pDrvCtrl->flags, pDrvCtrl->usrFlags,
             pDrvCtrl->sbm_dmabase, pDrvCtrl->sbm_macbase,
             pDrvCtrl->ivecnum, pDrvCtrl->intsource);

    DRV_LOG (DRV_DEBUG_LOAD,
             "EndLoad: numRd0 %d numTd0 %d numRd1 %d numTd1 %d unit %d devNo%d \n",
             pRxDma0->sbdma_maxdescr, pTxDma0->sbdma_maxdescr,
             pRxDma1->sbdma_maxdescr, pTxDma1->sbdma_maxdescr,
             pDrvCtrl->unit, pDrvCtrl->devNo);

    return OK;
}


/*
* bcm1250MacMemInit
* Allocate and initialize the memory nessary for devices to operate
* Including tx/rx descripter, net buffers, and ...
* returns OK or ERROR
*/
STATUS bcm1250MacMemInit
    (
    DRV_CTRL * pDrvCtrl       /* device to be initialized */
    )
    {
    SB_MAC_DMA  * pDma;
    int         memSize, frameSize, clNum;
    M_CL_CONFIG	mClBlkConfig;
    CL_DESC	clDesc;                      /* cluster description */

    pDrvCtrl->sbm_macenable = R_MAC_ENABLE;
    pDrvCtrl->sbm_maccfg    = R_MAC_CFG;
    pDrvCtrl->sbm_fifocfg   = R_MAC_THRSH_CFG;
    pDrvCtrl->sbm_framecfg  = R_MAC_FRAMECFG;
    pDrvCtrl->sbm_rxfilter  = R_MAC_ADFILTER_CFG;
    pDrvCtrl->sbm_isr       = R_MAC_STATUS;
    pDrvCtrl->sbm_imr       = R_MAC_INT_MASK;
    pDrvCtrl->sbm_mdio      = R_MAC_MDIO;

    frameSize = MAX_MAC_FRAME_SIZE;

    memSize = ROUND_UP((frameSize + CACHELINESIZE), CACHELINESIZE) *
              (pDrvCtrl->rxDma[0].sbdma_maxdescr +
               pDrvCtrl->txDma[0].sbdma_maxdescr +
               pDrvCtrl->rxDma[1].sbdma_maxdescr +
               pDrvCtrl->txDma[1].sbdma_maxdescr +
               SB_RXDSCR_LOAN_NUM*2 + 2) + CACHELINESIZE;

    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - InitMem NetBufLib Size 0x%8x\n", 
             pDrvCtrl->unit, memSize, 0, 0, 0, 0);

    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - InitMem allocating memory\n", 
             pDrvCtrl->unit, 0, 0, 0, 0, 0);
    pDrvCtrl->bufBase = (char *) memalign ( CACHELINESIZE, memSize );
    pDrvCtrl->bufSize = memSize;
    DRV_FLAGS_SET (BCM1250_MAC_MEMOWN);

    /* allocate memory for Chan 0 RX and TX desc rings */
    pDma = &(pDrvCtrl->rxDma[0]);
    pDma->sbdma_dscrtable = (SB_DMA_DSCR *)memalign(CACHELINESIZE, 
                                                    pDma->sbdma_maxdescr * 
                                                    sizeof(SB_DMA_DSCR));
    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - InitMem Chan 0 Rx Ring at 0x%08x\n",
             pDrvCtrl->unit, (UINT32) pDrvCtrl->rxDma[0].sbdma_dscrtable, 
             0, 0, 0, 0);

    pDma = &(pDrvCtrl->txDma[0]);
    pDma->sbdma_dscrtable = (SB_DMA_DSCR *)memalign(CACHELINESIZE, 
                                                    pDma->sbdma_maxdescr * 
                                                    sizeof(SB_DMA_DSCR));
    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - InitMem Chan 0 Tx Ring at 0x%08x\n",
             pDrvCtrl->unit, (UINT32) pDrvCtrl->txDma[0].sbdma_dscrtable, 
             0, 0, 0, 0);

    /* allocate memory for Chan 1 RX and TX desc rings */
    pDma = &(pDrvCtrl->rxDma[1]);
    pDma->sbdma_dscrtable = (SB_DMA_DSCR *)memalign(CACHELINESIZE, 
                                                    pDma->sbdma_maxdescr * 
                                                    sizeof(SB_DMA_DSCR));
    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - InitMem Chan 0 Rx Ring at 0x%08x\n",
             pDrvCtrl->unit, (UINT32) pDrvCtrl->rxDma[1].sbdma_dscrtable, 
             0, 0, 0, 0);

    pDma = &(pDrvCtrl->txDma[1]);
    pDma->sbdma_dscrtable = (SB_DMA_DSCR *)memalign(CACHELINESIZE, 
                                                    pDma->sbdma_maxdescr * 
                                                    sizeof(SB_DMA_DSCR));
    DRV_LOG (DRV_DEBUG_LOAD, "sbe%d - InitMem Chan 0 Tx Ring at 0x%08x\n",
             pDrvCtrl->unit, (UINT32) pDrvCtrl->txDma[1].sbdma_dscrtable, 
             0, 0, 0, 0);

    bzero ((char *) pDrvCtrl->rxDma[0].sbdma_dscrtable, 
           pDrvCtrl->rxDma[0].sbdma_maxdescr*sizeof(SB_DMA_DSCR));
    bzero ((char *) pDrvCtrl->txDma[0].sbdma_dscrtable, 
           pDrvCtrl->txDma[0].sbdma_maxdescr*sizeof(SB_DMA_DSCR));
    bzero ((char *) pDrvCtrl->rxDma[1].sbdma_dscrtable, 
           pDrvCtrl->rxDma[1].sbdma_maxdescr*sizeof(SB_DMA_DSCR));
    bzero ((char *) pDrvCtrl->txDma[1].sbdma_dscrtable, 
           pDrvCtrl->txDma[1].sbdma_maxdescr*sizeof(SB_DMA_DSCR));

    /*
     * This is how we would set up and END netPool using netBufLib(1).
     * This code is pretty generic.
     */

    bzero ((char *)&mClBlkConfig, sizeof(mClBlkConfig));
    bzero ((char *)&clDesc, sizeof(clDesc));

    if ((pDrvCtrl->endObj.pNetPool = malloc (sizeof(NET_POOL))) == NULL)
        return (ERROR);

    clNum = pDrvCtrl->rxDma[0].sbdma_maxdescr +  
            pDrvCtrl->rxDma[1].sbdma_maxdescr +
            pDrvCtrl->txDma[0].sbdma_maxdescr +  
            pDrvCtrl->txDma[1].sbdma_maxdescr +
            SB_RXDSCR_LOAN_NUM * 2 + 2;

    mClBlkConfig.mBlkNum = clNum * 2;
    clDesc.clNum = clNum;
    mClBlkConfig.clBlkNum = clDesc.clNum;

    mClBlkConfig.memSize = (mClBlkConfig.mBlkNum * (M_BLK_SZ + sizeof (long))) +
                           (mClBlkConfig.clBlkNum * CL_BLK_SZ);
    if ( (mClBlkConfig.memArea = (char *) memalign (sizeof(long), 
          mClBlkConfig.memSize)) == NULL )
        return (ERROR);
    pDrvCtrl->mClBlkBase = mClBlkConfig.memArea;

    /* Calculate the memory size of all the clusters. */
    clDesc.clSize = ROUND_UP(frameSize+CACHELINESIZE, CACHELINESIZE) - 
                            CL_OVERHEAD;
    clDesc.memSize = (clDesc.clNum * (clDesc.clSize + CL_OVERHEAD)) + 
                     CACHELINESIZE;

    /* Allocate the memory for the clusters from cache safe memory. */
    clDesc.memArea = (char *) pDrvCtrl->bufBase;
       /* purposely misalign the region by CACHELINESIZE - NET_BUF_HDR_SIZE
       so that the guts of the buffers are aligned */
    clDesc.memArea += (CACHELINESIZE - CL_OVERHEAD);
    DRV_LOG (DRV_DEBUG_LOAD, 
             "sbe%d - InitMem NetBufLib Cluster memArea 0x%08x\n",
             pDrvCtrl->unit, (UINT32) clDesc.memArea, 0, 0, 0, 0);

    if ((pDrvCtrl->endObj.pNetPool = malloc (sizeof(NET_POOL))) == NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "Could not init pNetPool\n",
                1, 2, 3, 4, 5, 6);
        return (ERROR);
        }

    if (netPoolInit(pDrvCtrl->endObj.pNetPool, &mClBlkConfig, &clDesc, 1, 
                    NULL) == ERROR)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "Could not init buffering\n",
                1, 2, 3, 4, 5, 6);
        return (ERROR);
        }

    if ((pDrvCtrl->clPoolId = netClPoolIdGet (pDrvCtrl->endObj.pNetPool,
        frameSize, FALSE))  == NULL)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "Could not get clPoolId\n",
                1, 2, 3, 4, 5, 6);
        return (ERROR);
        }

    DRV_LOG (DRV_DEBUG_LOAD, "Memory setup complete\n", 1, 2, 3, 4, 5, 6);

    return OK;
    }


/*******************************************************************************
* bcm1250MacDmaInit
* Initialize dma channels
* context table are allocated
* rxds are almost filled up here
* return OK or ERROR
*/
STATUS bcm1250MacDmaInit
    (
    DRV_CTRL * pDrvCtrl       /* device to be initialized */
    )
    {
    SB_MAC_DMA  *pDma;
    int          chan;
    /* char        *pBuf;
    SB_DMA_DSCR *pDscr;     */

    DRV_LOG (DRV_DEBUG_LOAD, "DMA init start\n", 1, 2, 3, 4, 5, 6);
    for ( chan = 0; chan < 2; chan++ )
        {
        DRV_LOG (DRV_DEBUG_LOAD, "rxDMA ch#%d init \n", chan, 2, 3, 4, 5, 6);
        pDma = &(pDrvCtrl->rxDma[chan]);

        pDma->sbdma_config0 = R_MAC_DMA_REGISTER(0,chan,R_MAC_DMA_CONFIG0);
        pDma->sbdma_config1 = R_MAC_DMA_REGISTER(0,chan,R_MAC_DMA_CONFIG1);
        pDma->sbdma_dscrbase = R_MAC_DMA_REGISTER(0,chan,R_MAC_DMA_DSCR_BASE);
        pDma->sbdma_dscrcnt = R_MAC_DMA_REGISTER(0,chan,R_MAC_DMA_DSCR_CNT);
        pDma->sbdma_curdscr = R_MAC_DMA_REGISTER(0,chan,R_MAC_DMA_CUR_DSCRADDR);

        pDma->sbdma_ctxtable = (char **)malloc(pDma->sbdma_maxdescr * 
                                               sizeof(char *));

        pDma->sbdma_addindex = 0;
        pDma->sbdma_remindex = 0;

        DRV_LOG (DRV_DEBUG_LOAD, "set regs, dscrbase and config0/maxdscr \n", 
                 chan, 2, 3, 4, 5, 6);

#ifndef L3P_TEST         
        SB_DMA_REG_WRITE(pDma->sbdma_config1, 0);
        SB_DMA_REG_WRITE(pDma->sbdma_dscrbase, 
                         (UINT32)(KVTOPHYS(pDma->sbdma_dscrtable)));
        SB_DMA_REG_WRITE(pDma->sbdma_config0, 
                         V_DMA_RINGSZ(pDma->sbdma_maxdescr));       
#else 
        SB_DMA_REG_WRITE(pDma->sbdma_config1, V_DMA_INT_TIMEOUT(100));
	SB_DMA_REG_WRITE(pDma->sbdma_dscrbase, 
                         (UINT32)(KVTOPHYS(pDma->sbdma_dscrtable)));
        SB_DMA_REG_WRITE(pDma->sbdma_config0, 
                         V_DMA_RINGSZ(pDma->sbdma_maxdescr) |
                         V_DMA_INT_PKTCNT(16) |
                         M_DMA_EOP_INT_EN );
#endif
        }

    for (chan = 0; chan < 2; chan++)
        {
        DRV_LOG (DRV_DEBUG_LOAD, "txDMA ch#%d init \n", chan, 2, 3, 4, 5, 6);
        pDma = &(pDrvCtrl->txDma[chan]);

        pDma->sbdma_config0 = R_MAC_DMA_REGISTER(1,chan,R_MAC_DMA_CONFIG0);
        pDma->sbdma_config1 = R_MAC_DMA_REGISTER(1,chan,R_MAC_DMA_CONFIG1);
        pDma->sbdma_dscrbase = R_MAC_DMA_REGISTER(1,chan,R_MAC_DMA_DSCR_BASE);
        pDma->sbdma_dscrcnt = R_MAC_DMA_REGISTER(1,chan,R_MAC_DMA_DSCR_CNT);
        pDma->sbdma_curdscr = R_MAC_DMA_REGISTER(1,chan,R_MAC_DMA_CUR_DSCRADDR);

        pDma->sbdma_ctxtable = (char **)malloc(pDma->sbdma_maxdescr * 
                                               sizeof(char *));
        pDma->sbdma_ctxtype = (UINT8 *)malloc(pDma->sbdma_maxdescr * 
                                              sizeof(UINT8));

        pDma->sbdma_addindex = 0;
	pDma->sbdma_remindex = 0;

        DRV_LOG (DRV_DEBUG_LOAD, "set regs, dscrbase and config0/maxdscr \n", 
                 chan, 2, 3, 4, 5, 6);

#ifdef L3P_TEST
        SB_DMA_REG_WRITE(pDma->sbdma_config1, 0);
	SB_DMA_REG_WRITE(pDma->sbdma_dscrbase, 
                         (UINT32)(KVTOPHYS(pDma->sbdma_dscrtable)));

        SB_DMA_REG_WRITE(pDma->sbdma_config0, 
                         V_DMA_RINGSZ(pDma->sbdma_maxdescr) );
#else
        SB_DMA_REG_WRITE(pDma->sbdma_config1, V_DMA_INT_TIMEOUT(100));
	SB_DMA_REG_WRITE(pDma->sbdma_dscrbase, 
                         (UINT32)(KVTOPHYS(pDma->sbdma_dscrtable)));

        SB_DMA_REG_WRITE(pDma->sbdma_config0, 
                         V_DMA_RINGSZ(pDma->sbdma_maxdescr) |
                         V_DMA_INT_PKTCNT(16) |
                         M_DMA_EOP_INT_EN );
#endif         
        }

    DRV_LOG (DRV_DEBUG_LOAD, "DMA init complete\n", 1, 2, 3, 4, 5, 6);
    return (OK);

    }


/*******************************************************************************
* bcm1250MacStart - start the device
*
* This function calls BSP functions to connect interrupts and start the
* device running in interrupt mode.
*
* RETURNS: OK or ERROR
*/
LOCAL STATUS bcm1250MacStart
    (
    DRV_CTRL *	pDrvCtrl /* device to start */
    )
    {
    UINT         usrFlags;
    UINT64       reg;
    STATUS       retVal;
    int          idx;
    sbmac_port_t port;
    char         enetAddr[6];
    SB_MAC_DMA  *pDma;
    int          chan, ix;
    char        *pBuf;
    SB_DMA_DSCR *pDscr;

    usrFlags  = pDrvCtrl->usrFlags;
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart enter ......\n", pDrvCtrl->unit,
	     0, 0, 0, 0, 0);

    /* reset */
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_macenable, 0);

    /* get mac addr */
    bcm1250MacEnetAddrGet (pDrvCtrl, enetAddr);
    reg =  sbmac_addr2reg(enetAddr);
    DRV_LOG (DRV_DEBUG_IOCTL, "mac reg 0x%08x%08x\n", (int)(reg>>32), reg,
             3, 4, 5, 6);

    /* set its own hw addr */
    port = R_MAC_ETHERNET_ADDR;
    /*SB_MAC_REG_WRITE(port,reg);*/ /* pass 1 bug, temp fix */
    SB_MAC_REG_WRITE(port,0);

    /* clear hash table */
    port = R_MAC_HASH_BASE;
    for (idx = 0; idx < MAC_HASH_COUNT; idx++)
        {
        SB_MAC_REG_WRITE(port,0);
        port += sizeof(UINT64);
        }

    /* clear exact match addr table */
    port = R_MAC_ADDR_BASE;
    for (idx = 0; idx < MAC_ADDR_COUNT; idx++)
        {
        SB_MAC_REG_WRITE(port,0);
        port += sizeof(UINT64);
        }

    /* always interested in packtes addressed to self */
    port = R_MAC_ADDR_BASE;
    SB_MAC_REG_WRITE(port,reg);

    /* reset channel map register */
    port = R_MAC_CHLO0_BASE;
    for (idx = 0; idx < MAC_CHMAP_COUNT; idx++)
        {
        SB_MAC_REG_WRITE(port,0);
        port += sizeof(UINT64);
        }

    /* clear rx filter */
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_rxfilter, 0);

    /* no interrupt yet */
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_imr, 0);

    /* setup frame cfg */
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_framecfg,
                      V_MAC_MIN_FRAMESZ_DEFAULT |
                      V_MAC_MAX_FRAMESZ_DEFAULT |
                      V_MAC_BACKOFF_SEL(1) );

    /* setup fifo cfg */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_fifocfg,
                     V_MAC_TX_WR_THRSH(4) |       /* Must be '4' or '8' */
                     V_MAC_TX_RD_THRSH(4) |
                     V_MAC_TX_RL_THRSH(4) |
                     V_MAC_RX_PL_THRSH(4) |
                     V_MAC_RX_RD_THRSH(4) |       /* Must be '4' */
                     V_MAC_RX_PL_THRSH(4) |
                     V_MAC_RX_RL_THRSH(8));

    /* setup the main config reg */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_maccfg,
                     M_MAC_RETRY_EN |
                     M_MAC_TX_HOLD_SOP_EN |
                     V_MAC_TX_PAUSE_CNT_16K |
                     V_MAC_SPEED_SEL_100MBPS |
                     M_MAC_AP_STAT_EN |
                     M_MAC_FAST_SYNC |
                     M_MAC_SS_EN |
		     V_MAC_IPHDR_OFFSET(14));

    bcm1250MacSetSpeed(pDrvCtrl,pDrvCtrl->sbm_speed);
    bcm1250MacSetDuplex(pDrvCtrl,pDrvCtrl->sbm_duplex,pDrvCtrl->sbm_fc);  

    /* init rx dma's descrs */
    for ( chan = 0; chan < 2; chan++ )
        {
        DRV_LOG (DRV_DEBUG_LOAD, "rxDMA ch#%d init \n", chan, 2, 3, 4, 5, 6);
        pDma = &(pDrvCtrl->rxDma[chan]);

        pDscr = pDma->sbdma_dscrtable;

        /* write out the Rx desc. */
        for (ix = 0; ix < (pDma->sbdma_maxdescr - 2); ix++, pDscr++)
            {
	    pBuf = (char *) NET_BUF_ALLOC();
	    if (pBuf == NULL)
	        {
	        DRV_LOG (DRV_DEBUG_LOAD, "bcm1250Mac - netClusterGet failed\n",
                         1,2,3,4,5,6);
	        return (ERROR);
	        }

	    /* first buffer  */
	    pDscr->dscr_a = (UINT64)((UINT32) KVTOPHYS(pBuf)) |
	                    V_DMA_DSCRA_A_SIZE(
                                NUMCACHEBLKS(MAX_MAC_FRAME_SIZE)) |
                            M_DMA_DSCRA_INTERRUPT | VXW_RCV_BUF_OFFSET;

	    pDscr->dscr_b = 0;

            pDma->sbdma_addindex = (pDma->sbdma_addindex + 1) % 
                                   pDma->sbdma_maxdescr;
            DRV_LOG (DRV_DEBUG_LOAD, "set reg, dscrcnt \n", chan, 2, 3, 4, 5, 
                     6);
            DRV_LOG (DRV_DEBUG_LOAD, "descA manual read 0x%08x%08x\n", 
                     (pDscr->dscr_a)>>32,pDscr->dscr_a,0, 0, 0, 0);
            DRV_LOG (DRV_DEBUG_LOAD, "descB manual read 0x%08x%08x\n", 
                     (pDscr->dscr_b)>>32,pDscr->dscr_b, 0, 0, 0, 0);
	    /* give desc to the hw */
	    SB_DMA_REG_WRITE(pDma->sbdma_dscrcnt, 1);
            }
        }

    /* enable all */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_macenable,
                     M_MAC_RXDMA_EN0 |
                     M_MAC_TXDMA_EN0 |
		     M_MAC_TXDMA_EN1 |
                     M_MAC_RXDMA_EN1 |
                     M_MAC_RX_ENABLE |
                     M_MAC_TX_ENABLE);

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart: enabled \n", 1,2,3,4,5,6);

    /* conect interrupt handler */
    retVal = bcm1250IntConnect (pDrvCtrl->intsource, pDrvCtrl->ivecnum, 
                                bcm1250MacInt, (int) pDrvCtrl);
    if (retVal == ERROR)
        return (ERROR);
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart connect int \n", 1,2,3,4,5,6);

#ifdef PHY_INT
    if (pDrvCtrl->unit == 1)
	bPHYConnect(pDrvCtrl);
#endif

    /* unmaks the mac interrupt */
#ifndef L3P_TEST 
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_imr,
                     (M_MAC_INT_CHANNEL << S_MAC_TX_CH0) |
                     (M_MAC_INT_CHANNEL << S_MAC_RX_CH0) |
                     (M_MAC_INT_CHANNEL << S_MAC_TX_CH1) |
                     (M_MAC_INT_CHANNEL << S_MAC_RX_CH1));
#else
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_imr,
                     (M_MAC_INT_CHANNEL << S_MAC_TX_CH0) |
                     ((M_MAC_INT_EOP_COUNT | M_MAC_INT_EOP_TIMER) << S_MAC_RX_CH0) |
                     (M_MAC_INT_CHANNEL << S_MAC_TX_CH1) |
                     ((M_MAC_INT_EOP_COUNT | M_MAC_INT_EOP_TIMER) << S_MAC_RX_CH1));
#endif

     DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart, set intr mask\n", 1,2,3,4,5,6);

    /* set rxfilter */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_rxfilter,
                     M_MAC_UCAST_EN | M_MAC_BCAST_EN | 0 /*M_MAC_ALLPKT_EN*/);

/* zchen: temp enable mac1 prom */    
    if (pDrvCtrl->unit == 1)
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_rxfilter,
                     M_MAC_UCAST_EN | M_MAC_BCAST_EN | M_MAC_ALLPKT_EN);
   
    /* set flags to indicate interface is up */
    END_FLAGS_SET (&pDrvCtrl->endObj, (IFF_UP | IFF_RUNNING));
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart, interface up\n", 1,2,3,4,5,6);

    /* enable interrupts */
    bcm1250IntEnable(pDrvCtrl->intsource);
    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart: enable interrupt\n", 
             1,2,3,4,5,6);

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacStart done\n", 1,2,3,4,5,6);

#ifdef L3P_TEST
/* zchen added for l3p debug */
  prevTick = tickGet();
  rxIntCount = 0;
#endif

    return OK;
    }

/*******************************************************************************
* bcm1250MacStop - stop the device
*
* This function calls BSP functions to disconnect interrupts and stop
* the device from operating in interrupt mode.
*
* RETURNS: OK or ERROR.
*/
LOCAL STATUS bcm1250MacStop
    (
    DRV_CTRL	*pDrvCtrl
    )
    {
    int		retVal=OK;

    DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacStop enter ......\n", 1,2,3,4,5,6);

    /* mark the interface as down */
    END_FLAGS_CLR (&pDrvCtrl->endObj, (IFF_UP | IFF_RUNNING));

    /* disable mac interrupts */
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_imr, 0);

    /* disable mac hw */
    SB_MAC_REG_WRITE (pDrvCtrl->sbm_macenable, 0);

    /* diasble VxW interrupt */
    bcm1250IntDisable(pDrvCtrl->intsource);

    /* disconnect interrupt handler */
    bcm1250IntDisconnect(pDrvCtrl->intsource);

    DRV_LOG (DRV_DEBUG_LOAD, "bcm1250MacStop done.\n", 1,2,3,4,5,6);

    return (retVal);
    }


/*******************************************************************************
* bcm1250MacSend - send an Ethernet packet
*
* This routine() takes a M_BLK_ID and sends off the data in the M_BLK_ID.
* The buffer must already have the addressing information properly installed
* in it. This is done by a higher layer.
*
* muxSend() calls this routine each time it wants to send a packet.
*
* RETURNS: OK, or END_ERR_BLOCK, if no resources are available, or ERROR,
* if the device is currently working in polling mode.
*/
LOCAL STATUS bcm1250MacSend
    (
    DRV_CTRL *  pDrvCtrl,       /* pointer to DRV_CTRL structure */
    M_BLK *     pMblk           /* pointer to the mBlk/cluster pair */
    )
    {
    UINT8               fragNum = 0;    /* number of fragments in this mBlk */
    UINT16              pktType = 0;    /* packet type (unicast or multicast) */
    UINT16		dscrNum;
    BOOL                zeroCopyReady;

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250MacSend...\n"), 1, 2, 3, 4, 5, 6);

    END_TX_SEM_TAKE (&pDrvCtrl->endObj, WAIT_FOREVER);

    /* check device mode */

    if (DRV_FLAGS_ISSET (BCM1250_MAC_POLLING))
        {
        goto bcm1250MacSendError;
        }

    /* walk the mBlk */
    if (bcm1250MacMblkWalk (pMblk, &fragNum, &pktType, &zeroCopyReady) == ERROR)
        {
        goto bcm1250MacSendError;
        } 

    /* check we have enough resources */
    dscrNum = availableSbDmaDscrs(pDrvCtrl, &pDrvCtrl->txDma[0]);

    if ( zeroCopyReady && (dscrNum >= fragNum) ) 
        {
        DRV_LOG (DRV_DEBUG_TX, ("bcm1250MacSend fragNum = %d \n"), fragNum, 
                 2, 3, 4, 5, 6);
        /* transmit the packet in zero-copy mode */
        if (bcm1250MacPktTransmit (pDrvCtrl, pMblk, pktType, fragNum, 0) == 
            ERROR)
            {
            goto bcm1250MacSendError;
            }
        }
    else 
        {
        /* transmit the packet in copy mode */
        fragNum = 1;

        if (bcm1250MacPktCopyTransmit (pDrvCtrl, pMblk, pktType, 0) == ERROR)
            {
            goto bcm1250MacSendError;
            }
        }

    END_TX_SEM_GIVE (&pDrvCtrl->endObj);

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250MacSend...Done\n"), 1, 2, 3, 4, 5, 6);

    return (OK);

bcm1250MacSendError:
    END_TX_SEM_GIVE (&pDrvCtrl->endObj);

    return (END_ERR_BLOCK);
    }

/*******************************************************************************
* bcm1250MacPktCopyTransmit - copy and transmit a packet
*
* This routine transmits the packet described by the given parameters
* over the network, after copying the mBlk to the driver's buffer.
* It also updates statistics.
*
* RETURNS: OK, or ERROR if no resources were available.
*/
LOCAL STATUS bcm1250MacPktCopyTransmit
    (
    DRV_CTRL *  pDrvCtrl,       /* pointer to DRV_CTRL structure */
    M_BLK *     pMblk,          /* pointer to the mBlk */
    UINT16      pktType,        /* packet type */
    int         channel
    )
    {
    int           len = 0;        /* length of data to be sent */
    char         *pBuf = NULL;    /* pointer to data to be sent */
    SB_DMA_DSCR  *pTxDscr;
    SB_MAC_DMA   *pTxDma;
    UINT64        dscr_cnt;

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250Mac CopyTx ...\n"), 1, 2, 3, 4, 5, 6);

    pTxDma = &pDrvCtrl->txDma[channel];

    /* get a cluster buffer from the pool */
    pBuf = NET_BUF_ALLOC(); 
    if (pBuf == NULL)
        {
        /* set to stall condition */
        pDrvCtrl->txDma[channel].sbdma_txblocked = TRUE;
        return (ERROR);
        }

    dscr_cnt = (pTxDma->sbdma_addindex - pTxDma->sbdma_remindex + pTxDma->sbdma_maxdescr) %
               pTxDma->sbdma_maxdescr;

    if( dscr_cnt >=  (pTxDma->sbdma_maxdescr - 1) )
        {
        DRV_LOG (DRV_DEBUG_TX, "No available TxBufs \n", 0, 0, 0, 0, 0, 0);
        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_OUT_ERRS, +1);
        pDrvCtrl->txDma[channel].sbdma_txblocked = TRUE;

        if (pBuf)
            NET_BUF_FREE (pBuf); 

        return (ERROR); /* just return without freeing mBlk chain */
        }
    else
        pTxDscr= pTxDma->sbdma_dscrtable + pTxDma->sbdma_addindex;

    DRV_LOG (DRV_DEBUG_TXD, "pTxd->desc_a = 0x%08x\n", 
             (UINT32) &pTxDscr->dscr_a, 0, 0, 0, 0, 0);
    DRV_LOG (DRV_DEBUG_TXD, "pTxd->desc_b = 0x%08x\n", 
             (UINT32) &pTxDscr->dscr_b, 0, 0, 0, 0, 0);

    len = netMblkToBufCopy (pMblk, (char *) pBuf, NULL);
    netMblkClChainFree (pMblk);
    len = max (ETHERSMALL, len);

    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], pBuf[17]);
    /*
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[18], pBuf[19], pBuf[20], pBuf[21], pBuf[22], pBuf[23]);
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[24], pBuf[25], pBuf[26], pBuf[27], pBuf[28], pBuf[29]);
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[30], pBuf[31], pBuf[32], pBuf[33], pBuf[34], pBuf[35]);
    */

    /* set buffer ptr, size in cache line, enable intr, mark as 1st pkt */
    pTxDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS(pBuf)) |
                      V_DMA_DSCRA_A_SIZE(NUMCACHEBLKS(len)) |
                      M_DMA_DSCRA_INTERRUPT |
                      M_DMA_ETHTX_SOP;

    /* set pkt len and  option */
    pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS(K_DMA_ETHTX_APPENDCRC_APPENDPAD) |
                      V_DMA_DSCRB_PKT_SIZE(len);

    DRV_LOG (DRV_DEBUG_TXD, "dscr_a = 0x%08x%08x\n", (pTxDscr->dscr_a)>>32,
             pTxDscr->dscr_a, 0, 0, 0, 0);
    DRV_LOG (DRV_DEBUG_TXD, "dscr_b = 0x%08x%08x\n", (pTxDscr->dscr_b)>>32,
             pTxDscr->dscr_b, 0, 0, 0, 0);

    /* save the buf info */
    pTxDma->sbdma_ctxtable[pTxDma->sbdma_addindex] = pBuf;
    pTxDma->sbdma_ctxtype[pTxDma->sbdma_addindex] = BUF_TYPE_CL;	

    /* inc index */
    pTxDma->sbdma_addindex = (pTxDma->sbdma_addindex + 1) % 
                             pTxDma->sbdma_maxdescr;

    /* tell hw one more dscr to go */
    SB_DMA_REG_WRITE(pTxDma->sbdma_dscrcnt, 1);

    /* debug purpose */
    dscr_cnt = SB_DMA_REG_READ(pTxDma->sbdma_dscrcnt);

    DRV_LOG (DRV_DEBUG_TX, "tx%d DMA dscrcnt=0x%08x%08x\n",channel, 
             dscr_cnt>>32, dscr_cnt, 0, 0, 0);

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250Mac CopyTx Done. \n"), 1, 2, 3, 4, 5, 6);
    return (OK);
    }

/*******************************************************************************
*
* availableSbDmaDscrs - calculate available dma descriptors
*
*/
LOCAL UINT16 availableSbDmaDscrs
    (
    DRV_CTRL *pDrvCtrl,
    SB_MAC_DMA *pDma
    )
    {
    UINT32 cnt;

    cnt = (pDma->sbdma_addindex - pDma->sbdma_remindex + pDma->sbdma_maxdescr) %
          pDma->sbdma_maxdescr;
    
    return (pDma->sbdma_maxdescr - 1 - cnt); 
    }

/*******************************************************************************
*
* bcm1250MacMblkWalk - walk the mBlk
*
* This routine walks the mBlk whose address is in <pMblk>, computes
* the number of fragments it is made of, and returns it in the parameter
* <pFragNum>. In addition, it finds out whether the specified packet
* is unicast or multicast, and sets <pPktType> accordingly.
*
* RETURNS: OK, or ERROR in case of invalid mBlk.
*/
LOCAL STATUS bcm1250MacMblkWalk
    (
    M_BLK *     pMblk,          /* pointer to the mBlk */
    UINT8 *     pFragNum,       /* number of fragments */
    UINT16 *    pPktType,       /* packet type */
    BOOL*       pZeroCopyReady  	
    )
    {
    M_BLK *             pCurr = pMblk;          /* the current mBlk */
 
    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacMblkWalk: start\n", 1,2,3,4,5,6);

    if (pMblk == NULL)
        {
        DRV_LOG (DRV_DEBUG_TX, ("bcm1250MacMblkWalk: invalid pMblk\n"),
                                      0, 0, 0, 0, 0, 0);
        return (ERROR);
        }
                                                         
    *pZeroCopyReady = TRUE;
                         
    /* walk this mBlk */
    while (pCurr != NULL)
        {
        /* if not first buffer in a packet, must start at cache line boundary */
	if ( pCurr != pMblk ) 
            if ( ((UINT32)pCurr->mBlkHdr.mData & (CACHELINESIZE - 1)) != 0 )  
                { 
                DRV_LOG (DRV_DEBUG_TX, 
                         "bcm1250MacMblkWalk: beginning align fail\n", 
                         1, 2, 3, 4, 5, 6);
                *pZeroCopyReady = FALSE;
                }

        /* if not last buffer in a pacet, must end at cache line boundary */ 
        if ( pCurr->mBlkHdr.mNext ) 
            if ( (((UINT32)pCurr->mBlkHdr.mData + pCurr->mBlkHdr.mLen) & 
                  (CACHELINESIZE - 1)) != 0 )
                {
                DRV_LOG (DRV_DEBUG_TX, 
                         "bcm1250MacMblkWalk: ending align fail\n", 
                         1, 2, 3, 4, 5, 6);
                *pZeroCopyReady = FALSE;
                }

        /* keep track of the number of fragments in this mBlk */
        (*pFragNum)++;
 
        pCurr = ((M_BLK *) pCurr->mBlkHdr.mNext);
        DRV_LOG (DRV_DEBUG_TX, "bcm1250MacMblkWalk: next\n", 1,2,3,4,5,6);
        }
 
    /* set the packet type to multicast or unicast */
    if (pMblk->mBlkHdr.mData[0] & (UINT8) 0x01)
        {
        (*pPktType) = PKT_TYPE_MULTI;
        }
    else
        {
        (*pPktType) = PKT_TYPE_UNI;
        }
 
    return (OK);
    }                            


/*******************************************************************************
* bcm1250MacPktTransmit - zero copy packet transmit 
*
* This routine transmits the packet described by the given parameters
* over the network, zero mbuf copying
*
* RETURNS: OK, or ERROR if no resources were available or condition 
* does not meet.
*/
LOCAL STATUS bcm1250MacPktTransmit
    (
    DRV_CTRL *  pDrvCtrl,       /* pointer to DRV_CTRL structure */
    M_BLK *     pMblk,          /* pointer to the mBlk */
    UINT16      pktType,         /* packet type */
    UINT8       fragNum,
    int         channel
    )
    {
    int           len = 0;        /* length of data to be sent */
    UINT32        offset;
    char         *pBuf = NULL;    /* pointer to data to be sent */
    SB_DMA_DSCR  *pTxDscr;
    SB_MAC_DMA   *pTxDma;
    M_BLK        *pCurrentMblk;
    /* UINT64        dscr_cnt; */

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250Mac Zero-Copy Tx ...\n"), 1, 2, 3, 4, 5, 6);

    pTxDma = &pDrvCtrl->txDma[channel];	

#ifdef DRV_DEBUG    
    dscr_cnt = SB_DMA_REG_READ(pTxDma->sbdma_dscrcnt);
    DRV_LOG (DRV_DEBUG_TXD, "tx%d DMA dscrcnt=0x%08x%08x\n",
                            channel, dscr_cnt>>32, dscr_cnt, 0, 0, 0);

    if( (dscr_cnt + fragNum) >  (pTxDma->sbdma_maxdescr - 1) )
        {
        DRV_LOG (DRV_DEBUG_TX, "No available TxBufs \n", 0, 0, 0, 0, 0, 0);
        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_OUT_ERRS, +1);
        pDrvCtrl->txDma[channel].sbdma_txblocked = TRUE;

        return (ERROR); /* just return without freeing mBlk chain */
        }
    else
#endif
        pTxDscr= pTxDma->sbdma_dscrtable + pTxDma->sbdma_addindex;

    DRV_LOG (DRV_DEBUG_TXD, "pTxd->desc_a = 0x%08x\n", 
             (UINT32) &pTxDscr->dscr_a, 0, 0, 0, 0, 0);
    DRV_LOG (DRV_DEBUG_TXD, "pTxd->desc_b = 0x%08x\n", 
             (UINT32) &pTxDscr->dscr_b, 0, 0, 0, 0, 0);

    pCurrentMblk = pMblk;
    while ( pCurrentMblk )
        {
        pBuf = pCurrentMblk->mBlkHdr.mData;
        len = pCurrentMblk->mBlkHdr.mLen;
        offset = (UINT32)(pCurrentMblk->mBlkHdr.mData) & (CACHELINESIZE - 1);

       	/* set buffer ptr, size in cache line */
	pTxDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS(pBuf)) | 
                           V_DMA_DSCRA_A_SIZE(NUMCACHEBLKS(offset + len)); 
         
        pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS(K_DMA_ETHTX_APPENDCRC_APPENDPAD);
 
        pTxDma->sbdma_ctxtable[pTxDma->sbdma_addindex] = NULL;

	if ( pCurrentMblk == pMblk ) 
	    {
	    /* set pkt len and  option */
            pTxDscr->dscr_a |= M_DMA_ETHTX_SOP;       
	    pTxDscr->dscr_b |= V_DMA_DSCRB_PKT_SIZE(
                               pCurrentMblk->mBlkPktHdr.len);
            } 
           
	if ( pCurrentMblk->mBlkHdr.mNext == NULL ) 
            { 
	    pTxDscr->dscr_a |= M_DMA_DSCRA_INTERRUPT;	

            /* save the buf info */
            pTxDma->sbdma_ctxtable[pTxDma->sbdma_addindex] = (char *)pMblk;
            pTxDma->sbdma_ctxtype[pTxDma->sbdma_addindex] = BUF_TYPE_MBLK;
            }

        DRV_LOG (DRV_DEBUG_TXD, "dscr_a = 0x%08x%08x\n", 
                 (pTxDscr->dscr_a)>>32,pTxDscr->dscr_a,0,0,0,0);
        DRV_LOG (DRV_DEBUG_TXD, "dscr_b = 0x%08x%08x\n", 
                 (pTxDscr->dscr_b)>>32,pTxDscr->dscr_b,0,0,0,0);

    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
    DRV_LOG (DRV_DEBUG_TXD,
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], pBuf[17]);  
        /* inc index */
        pTxDma->sbdma_addindex = (pTxDma->sbdma_addindex + 1) %  
                                 pTxDma->sbdma_maxdescr; 
        pTxDscr= pTxDma->sbdma_dscrtable + pTxDma->sbdma_addindex;

	pCurrentMblk = pCurrentMblk->mBlkHdr.mNext; 
        }

    /* tell hw one more dscr to go */
    SB_DMA_REG_WRITE(pTxDma->sbdma_dscrcnt, fragNum);

#ifdef DRV_DEBUG
    /* debug purpose */
    dscr_cnt = SB_DMA_REG_READ(pTxDma->sbdma_dscrcnt);

    DRV_LOG (DRV_DEBUG_TX, "tx%d DMA dscrcnt=0x%08x%08x\n",channel, 
             dscr_cnt>>32, dscr_cnt,0,0,0);
#endif

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250Mac Zero-Copy Tx Done. \n"), 
             1, 2, 3, 4, 5, 6);
    return (OK);
    }

/*******************************************************************************
* bcm1250MacInt - entry point for handling interrupts
*
* The interrupting events are acknowledged to the device, so that the device
* will de-assert its interrupt signal.  The amount of work done here is kept
* to a minimum; the bulk of the work is deferred to the netTask.
*
*/
void bcm1250MacInt
    (
    DRV_CTRL *  pDrvCtrl       /* pointer to DRV_CTRL structure */
    )
    {
    UINT64      status;         /* status word */

    while ( (status = SB_MAC_REG_READ(pDrvCtrl->sbm_isr)) != 0 )
        {
        DRV_LOG (DRV_DEBUG_INT, ("status = 0x%x\n"), status, 0, 0, 0, 0, 0);

        /* handle transmit and receive interrupts */

        if ( (status & (M_MAC_INT_CHANNEL << S_MAC_RX_CH0)) && !pDrvCtrl->rxDpc[0] )
            {
            pDrvCtrl->rxDpc[0] = 1;
            (void) netJobAdd((FUNCPTR) bcm1250MacRxHandle, (int) pDrvCtrl, 
                             0, 0, 0, 0);
#ifdef L3P_TEST
/* zchen added for l3p debug */
  if (pDrvCtrl->unit == 1) {
    rxIntCount++;
    if ((tickGet() - prevTick) > sysClkRateGet()) {
      logMsg("RxIntCount %d \n", rxIntCount,2,3,4,5,6);
      rxIntCount = 0;
      prevTick = tickGet();
    }
  }
#endif

            }

        if ( (status & (M_MAC_INT_CHANNEL << S_MAC_RX_CH1)) && !pDrvCtrl->rxDpc[1] )
            {
            pDrvCtrl->rxDpc[1] = 1;
            (void) netJobAdd((FUNCPTR) bcm1250MacRxHandle, (int) pDrvCtrl, 1,
                             0, 0, 0);
            }

        if ( (status & (M_MAC_INT_CHANNEL << S_MAC_TX_CH0)) && !pDrvCtrl->txDpc[0] )
            {
            pDrvCtrl->txDpc[0] = 1;
            (void) netJobAdd((FUNCPTR) bcm1250MacTxHandle, (int) pDrvCtrl, 
                             0, 0, 0, 0);
            if (pDrvCtrl->txDma[0].sbdma_txblocked) {
                pDrvCtrl->txDma[0].sbdma_txblocked = FALSE;
                netJobAdd ((FUNCPTR)muxTxRestart, (int)&pDrvCtrl->endObj, 0, 0, 0, 0);
            }                                                                                               
            }

        if ( (status & (M_MAC_INT_CHANNEL << S_MAC_TX_CH1)) && !pDrvCtrl->txDpc[1] )
            {
            pDrvCtrl->txDpc[1] = 1;
            (void) netJobAdd((FUNCPTR) bcm1250MacTxHandle, (int) pDrvCtrl, 1,
                             0, 0, 0);
            if (pDrvCtrl->txDma[1].sbdma_txblocked) {
                pDrvCtrl->txDma[1].sbdma_txblocked = FALSE;
                netJobAdd ((FUNCPTR)muxTxRestart, (int)&pDrvCtrl->endObj, 0, 0, 0, 0);
            }                                                                                               
            }
        }

    }


/*******************************************************************************
*
* bcm1250MacRxHandle - service task-level interrupts for receive frames
*
* This routine is run in netTask's context, at task level.
*
* RETURNS: N/A
*/
LOCAL void bcm1250MacRxHandle
    (
    DRV_CTRL *  pDrvCtrl,        /* pointer to DRV_CTRL structure */
    int		channel
    )
    {
    SB_MAC_DMA  *pRxDma;
    SB_DMA_DSCR *pRxDscr, *pCurrDscr;
    M_BLK_ID     pMblk;
    CL_BLK_ID    pClBlk;
    char *       pBuf;
    char *       pData;
    int          len;

    DRV_LOG (DRV_DEBUG_RX, ("bcm1250MacRxHandle ......\n"), 0, 0, 0, 0, 0, 0);

    pRxDma = &pDrvCtrl->rxDma[channel];
    pDrvCtrl->rxDpc[channel] = 0;

    while ( TRUE )
        {
        /*
        dscr_count = SB_DMA_REG_READ(pRxDma->sbdma_dscrcnt);
        DRV_LOG (DRV_DEBUG_RX, ("bcm1250MacRxHandle: dscr count %n \n"), 
                 dscr_count, 0, 0, 0, 0, 0);

        if ( dscr_count >= pRxDma->sbdma_maxdescr )
	    break;
        */

        pCurrDscr = (SB_DMA_DSCR *)((UINT32)((SB_DMA_REG_READ(
                     pRxDma->sbdma_curdscr) & M_DMA_CURDSCR_ADDR)));

	pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_remindex;

        if ( (UINT32)pCurrDscr == KVTOPHYS(pRxDscr) )
            {
            DRV_LOG (DRV_DEBUG_RXD, "pRxDscr:0x%x catch pCurrPtr:0x%x \n",
                     (int)pRxDscr, (int)pCurrDscr, 0, 0, 0, 0);
            break;
            }

        if ( pRxDscr->dscr_a & M_DMA_ETHRX_BAD )
            {
            DRV_LOG (DRV_DEBUG_RXD, "bad frame\n", 0, 0, 0, 0, 0, 0);
            END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, +1);

	    pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_remindex;

            pData = (char *) PHYSTOV ((UINT32) (pRxDscr->dscr_a) );

	    pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_addindex;

            pRxDscr->dscr_a = KVTOPHYS((UINT32)pData) |
                              V_DMA_DSCRA_A_SIZE(
                                NUMCACHEBLKS(MAX_MAC_FRAME_SIZE)) |
                              M_DMA_DSCRA_INTERRUPT;

            pRxDscr->dscr_b = 0;

            pRxDma->sbdma_addindex = (pRxDma->sbdma_addindex + 1) % 
                                     pRxDma->sbdma_maxdescr;

            /* mark the descriptor ready to receive */
            SB_DMA_REG_WRITE(pRxDma->sbdma_dscrcnt, 1);

            /* advance management index */
            pRxDma->sbdma_remindex = (pRxDma->sbdma_remindex + 1) % 
                                      pRxDma->sbdma_maxdescr;
            }
        else
            {
            DRV_LOG (DRV_DEBUG_RXD, ("good frame \n"), 0, 0, 0, 0, 0, 0);
            END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_UCAST, +1);

            len = (int)G_DMA_DSCRB_PKT_SIZE(pRxDscr->dscr_b) - 4;
            pData = (char *) PHYSTOV ((UINT32) (pRxDscr->dscr_a) );

            pBuf=pData;
            DRV_LOG (DRV_DEBUG_RXD, "rx - pData= 0x%08x, len=%d \n", (int)pBuf, 
                     len,3,4,5,6);
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
                     pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
                     pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
                     pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], 
                     pBuf[17]);

            pMblk = NET_MBLK_ALLOC();
            pClBlk = NET_CL_BLK_ALLOC();
            pBuf = NET_BUF_ALLOC ();

            if ((pMblk == NULL) || (pBuf == NULL) || (pClBlk == NULL))
                {
                DRV_LOG (DRV_DEBUG_RX, "not available RxBufs \n", 
                         0, 0, 0, 0, 0, 0);
                END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, +1);
                if (pMblk)
                    NET_MBLK_FREE (pMblk);
                if (pBuf)
                    NET_BUF_FREE (pBuf);
                if (pClBlk)
                    NET_CL_BLK_FREE (pClBlk);
   
#if 1
                DRV_LOG (DRV_DEBUG_RX, "bad frame\n", 0, 0, 0, 0, 0, 0);
                END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, +1);

	        pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_remindex;

                pData = (char *) PHYSTOV ((UINT32) (pRxDscr->dscr_a) );

	        pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_addindex;

                pRxDscr->dscr_a = KVTOPHYS((UINT32)pData) |
                                  V_DMA_DSCRA_A_SIZE(
                                  NUMCACHEBLKS(MAX_MAC_FRAME_SIZE)) |
                                  M_DMA_DSCRA_INTERRUPT;

                pRxDscr->dscr_b = 0;

                pRxDma->sbdma_addindex = (pRxDma->sbdma_addindex + 1) % 
                                         pRxDma->sbdma_maxdescr;

                /* mark the descriptor ready to receive */
                SB_DMA_REG_WRITE(pRxDma->sbdma_dscrcnt, 1);

                /* advance management index */
                pRxDma->sbdma_remindex = (pRxDma->sbdma_remindex + 1) % 
                                          pRxDma->sbdma_maxdescr;
#endif
                }
            else
                {
                DRV_LOG (DRV_DEBUG_RXD, 
                         "rx - pMblk=0x%08x pClBlk=0x%08x pBuf= 0x%08x\n",
                         (int)pMblk, (int)pClBlk, (int)pBuf, 4,5,6);

                NET_CL_BLK_JOIN (pClBlk, pData - VXW_RCV_BUF_OFFSET, 
                                 MAX_MAC_FRAME_SIZE);
                NET_MBLK_CL_JOIN(pMblk, pClBlk);

                pMblk->mBlkHdr.mFlags |= M_PKTHDR;
                pMblk->mBlkHdr.mData = pData;
                pMblk->mBlkHdr.mLen = len;
                pMblk->mBlkPktHdr.len = pMblk->mBlkHdr.mLen;

	        pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_addindex;

                pRxDscr->dscr_a = KVTOPHYS((UINT32)pBuf) |
                              V_DMA_DSCRA_A_SIZE(
                              NUMCACHEBLKS(MAX_MAC_FRAME_SIZE)) |
                              M_DMA_DSCRA_INTERRUPT | VXW_RCV_BUF_OFFSET;

                pRxDscr->dscr_b = 0;
                pRxDma->sbdma_addindex = (pRxDma->sbdma_addindex + 1) % 
                                         pRxDma->sbdma_maxdescr;

                /* mark the descriptor ready to receive */
                SB_DMA_REG_WRITE( pRxDma->sbdma_dscrcnt, 1);

                /* inc ring index */
                pRxDma->sbdma_remindex = (pRxDma->sbdma_remindex + 1) % 
                                         pRxDma->sbdma_maxdescr;

#ifndef L3P_TEST 
                /* send the frame to the upper layer */
                END_RCV_RTN_CALL (&pDrvCtrl->endObj, pMblk);
#else
                drvRxPktCount++;
#endif
                }
	    }
        }

    DRV_LOG (DRV_DEBUG_RX, ("bcm1250MacRxHandle Done.\n"), 0, 0, 0, 0, 0, 0);
    }


/*******************************************************************************
*
* bcm1250MacTxHandle - service task-level interrupts for transmitted frames
*
* This routine is run in netTask's context, at task level.
*
* RETURNS: N/A
*/
LOCAL void bcm1250MacTxHandle
    (
    DRV_CTRL    *pDrvCtrl,
    int         channel
    )
    {
    SB_MAC_DMA      *pTxDma;
    SB_DMA_DSCR     *pTxDscr, *pCurrDscr;

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250MacTxHandle ......\n"), 0, 0, 0, 0, 0, 0);

    pTxDma = &(pDrvCtrl->txDma[channel]);
    pDrvCtrl->txDpc[channel] = 0;

    pTxDma->sbdma_txcleaning = TRUE;

    while ( TRUE )
        {
        pCurrDscr = (SB_DMA_DSCR *)((UINT32)(SB_DMA_REG_READ(
                    pTxDma->sbdma_curdscr) & M_DMA_CURDSCR_ADDR));

        pTxDscr = pTxDma->sbdma_dscrtable + pTxDma->sbdma_remindex;

        if ( (UINT32)pCurrDscr == KVTOPHYS(pTxDscr) )
             break;

        DRV_LOG (DRV_DEBUG_TX, "tx_remindex=0x%x, curdscr=0x%x, remdscr=0x%x\n",
                 pTxDma->sbdma_remindex, (int)pCurrDscr, (int)pTxDscr, 0, 0, 0);

        /* free the buffer */

        if ( pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex] != NULL)
            {
	    if ( pTxDma-> sbdma_ctxtype[pTxDma->sbdma_remindex] == BUF_TYPE_CL )
	        NET_BUF_FREE(pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex]);
            else if ( pTxDma-> sbdma_ctxtype[pTxDma->sbdma_remindex] == 
                      BUF_TYPE_MBLK )
                netMblkClChainFree((M_BLK *) 
                      pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex]);
            else 
                DRV_LOG (DRV_DEBUG_TX, 
                         "unknown buffer type when try to release tx buf \n", 
                         0, 0, 0, 0, 0, 0);

            pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex] = NULL;
            }

        pTxDma->sbdma_remindex =  (pTxDma->sbdma_remindex + 1) % 
                                  pTxDma->sbdma_maxdescr;
        }

    /* clear errors and stats */

    pTxDma->sbdma_txcleaning = FALSE;

    DRV_LOG (DRV_DEBUG_TX, ("bcm1250MacTxHandle Done. \n"), 0, 0, 0, 0, 0, 0);

    return;
    }


/*******************************************************************************
*
* bcm1250MacUnload - unload a driver from the system
*
* This routine deallocates lists, and free allocated memory.
*
* RETURNS: OK, always.
*/
LOCAL STATUS bcm1250MacUnload
    (
    DRV_CTRL *pDrvCtrl
    )
    {
    SB_MAC_DMA  *pDma;

    DRV_LOG (DRV_DEBUG_LOAD, "EndUnload ...\n", 0, 0, 0, 0, 0, 0);

    /* deallocate lists */
    END_OBJ_UNLOAD (&pDrvCtrl->endObj);

    /* free the rest of things that were alloced */
    pDma = &pDrvCtrl->rxDma[0];
    if ( pDma->sbdma_ctxtable )
        free(pDma->sbdma_ctxtable);
    if ( pDma->sbdma_dscrtable )
        free(pDma->sbdma_dscrtable);

    pDma = &pDrvCtrl->rxDma[1];
    if ( pDma->sbdma_ctxtable )
        free(pDma->sbdma_ctxtable);
    if ( pDma->sbdma_dscrtable )
        free(pDma->sbdma_dscrtable);

    pDma = &pDrvCtrl->txDma[0];
    if ( pDma->sbdma_ctxtable )
        free(pDma->sbdma_ctxtable);
    if ( pDma->sbdma_ctxtype )
        free(pDma->sbdma_ctxtype);
    if ( pDma->sbdma_dscrtable )
        free(pDma->sbdma_dscrtable);

    pDma = &pDrvCtrl->txDma[1];
    if ( pDma->sbdma_ctxtable )
        free(pDma->sbdma_ctxtable);
    if ( pDma->sbdma_ctxtype )
        free(pDma->sbdma_ctxtype);
    if ( pDma->sbdma_dscrtable )
        free(pDma->sbdma_dscrtable);

    if ( pDrvCtrl->mClBlkBase )
        free (pDrvCtrl->mClBlkBase);

    if ( pDrvCtrl->bufBase )
        free (pDrvCtrl->bufBase);

    cfree ((char *) pDrvCtrl);

    return (OK);
    }


/*******************************************************************************
* bcm1250MacPollSend - routine to send a packet in polled mode.
*
* This routine is called by a user to try and send a packet on the
* device.
*
* RETURNS: OK upon success.  EAGAIN if device is busy.
*/
LOCAL STATUS bcm1250MacPollSend
    (
    DRV_CTRL  *pDrvCtrl,
    M_BLK     *pMblk
    )
    {
    char        *pBuf;
    SB_MAC_DMA  *pTxDma;
    SB_DMA_DSCR *pTxDscr, *pCurrDscr;
    int          len;
    UINT64       dscr_cnt;

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacPollSend enter ...... \n", 
              0, 0, 0, 0, 0, 0); 

    /* get a cluster buffer from the pool */
    pBuf = NET_BUF_ALLOC();
    if (pBuf == NULL) 
        {
        return (EAGAIN);
        }

    /* let's just use channel 0 for the time being */
    pTxDma = &pDrvCtrl->txDma[0];

    dscr_cnt = (pTxDma->sbdma_addindex - pTxDma->sbdma_remindex + pTxDma->sbdma_maxdescr) %
               pTxDma->sbdma_maxdescr;

    if( dscr_cnt >=  (pTxDma->sbdma_maxdescr - 1) )
        {
        DRV_LOG (DRV_DEBUG_TX, "No available TxBufs \n", 0, 0, 0, 0, 0, 0);
        END_ERR_ADD (&pDrvCtrl->endObj, MIB2_OUT_ERRS, +1);

        if (pBuf)
            NET_BUF_FREE (pBuf);

        return (EAGAIN); /* just return without freeing mBlk chain */
        }
    else
        pTxDscr= pTxDma->sbdma_dscrtable + pTxDma->sbdma_addindex;

    len = netMblkToBufCopy (pMblk, (char *) pBuf, NULL);
    netMblkClChainFree (pMblk);
    len = max (ETHERSMALL, len);

    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
    DRV_LOG (DRV_DEBUG_TXD, 
             "tx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
             pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], pBuf[17]);

    /* set txbuf ptr, cache line size, ...  */
    pTxDscr->dscr_a = (UINT64)((UINT32)KVTOPHYS(pBuf)) |
                  V_DMA_DSCRA_A_SIZE(NUMCACHEBLKS(len)) |
                  M_DMA_DSCRA_INTERRUPT |
                  M_DMA_ETHTX_SOP;

    /* set pkt len and option */
    /*pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS(
                        K_DMA_ETHTX_REPLACESADDR_APPENDCRC_APPENDPAD) |
                        V_DMA_DSCRB_PKT_SIZE(len);*/ /* pass 1 bug, temp fix */
    pTxDscr->dscr_b = V_DMA_DSCRB_OPTIONS(K_DMA_ETHTX_APPENDCRC_APPENDPAD) |
                      V_DMA_DSCRB_PKT_SIZE(len);

    /* save the buf info */
    pTxDma->sbdma_ctxtable[pTxDma->sbdma_addindex] = pBuf;
    pTxDma->sbdma_ctxtype[pTxDma->sbdma_addindex] = BUF_TYPE_CL;

    /* Advance our management index */
    pTxDma->sbdma_addindex = (pTxDma->sbdma_addindex + 1) % 
                             pTxDma->sbdma_maxdescr;

    SB_DMA_REG_WRITE(pTxDma->sbdma_dscrcnt, 1);


    while ( TRUE )
        {
        pCurrDscr = (SB_DMA_DSCR *)((UINT32)(SB_DMA_REG_READ(
                        pTxDma->sbdma_curdscr) & M_DMA_CURDSCR_ADDR));

        pTxDscr = pTxDma->sbdma_dscrtable + pTxDma->sbdma_remindex;

        DRV_LOG (DRV_DEBUG_TX, "tx_remindex:0x%x, curdscr=0x%x, remdscr=0x%x\n",
                 pTxDma->sbdma_remindex, (int)pCurrDscr, (int)pTxDscr, 0, 0, 0);

        if ( (UINT32)pCurrDscr == KVTOPHYS(pTxDscr) )
            break;

        /* free the buffer */

        if ( pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex] != NULL)
            {
	    if ( pTxDma-> sbdma_ctxtype[pTxDma->sbdma_remindex] == BUF_TYPE_CL )
	        NET_BUF_FREE(pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex]);
            else if ( pTxDma-> sbdma_ctxtype[pTxDma->sbdma_remindex] == 
                      BUF_TYPE_MBLK )
                netMblkClChainFree((M_BLK *) 
                      pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex]);
            else 
                DRV_LOG (DRV_DEBUG_TX, 
                         "unknown buffer type when try to release tx buf \n", 
                         0, 0, 0, 0, 0, 0);

            pTxDma->sbdma_ctxtable[pTxDma->sbdma_remindex] = NULL;
            }

        pTxDma->sbdma_remindex =  (pTxDma->sbdma_remindex + 1) % 
                                  pTxDma->sbdma_maxdescr;
        }

    DRV_LOG (DRV_DEBUG_TX, "bcm1250MacPollSend Done \n", 0, 0, 0, 0, 0, 0); 

    return (OK);
    }


/*******************************************************************************
* bcm1250MacPollRcv - routine to receive a packet in polled mode.
*
* This routine is called by a user to try and get a packet from the
* device.
*
* RETURNS: OK upon success.  EAGAIN is returned when no packet is available.
*/
LOCAL STATUS bcm1250MacPollRcv
    (
    DRV_CTRL    *pDrvCtrl,
    M_BLK       *pMblk
    )
    {
    SB_MAC_DMA  *pRxDma;
    SB_DMA_DSCR *pRxDscr, *pCurrDscr;
    char        *pBuf;
    char        *pData;
    int          len, channel;
#ifdef L3P_TEST 
    M_BLK_ID     pMblk;
    CL_BLK_ID    pClBlk;
#endif

    DRV_LOG (DRV_DEBUG_RX, ("bcm1250MacPollRcv enter ...... \n"), 
             0, 0, 0, 0, 0, 0);

    for ( channel = 0; channel < 1; channel++ )
        {
        pRxDma = &pDrvCtrl->rxDma[channel];

        pCurrDscr = (SB_DMA_DSCR *)((UINT32)(SB_DMA_REG_READ(
                     pRxDma->sbdma_curdscr) & M_DMA_CURDSCR_ADDR));

	pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_remindex;

        DRV_LOG (DRV_DEBUG_RX, "rx_remindex=%d, pRxDscr=0x%x pCurrPtr=0x%x \n",
                                pRxDma->sbdma_remindex, (int)pRxDscr, 
                                (int)pCurrDscr, 0,0,0);

        if ( (UINT32)pCurrDscr == KVTOPHYS(pRxDscr) )
            {
            continue;
            }

        DRV_LOG (DRV_DEBUG_RX, "rx_remindex=%d, pRxDscr=0x%x pCurrPtr=0x%x \n",
                                pRxDma->sbdma_remindex, (int)pRxDscr, 
                                (int)pCurrDscr, 0,0,0);

        if ( pRxDscr->dscr_a & M_DMA_ETHRX_BAD )
            {
            DRV_LOG (DRV_DEBUG_RX, "Bad Frame\n", 0, 0, 0, 0, 0, 0);
            END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_ERRS, +1);

            pData = (char *)PHYSTOV((UINT32)(pRxDscr->dscr_a));

	    pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_addindex;
            pRxDscr->dscr_a = KVTOPHYS((UINT32)pData) |
                              V_DMA_DSCRA_A_SIZE(
                                  NUMCACHEBLKS(MAX_MAC_FRAME_SIZE)) |
                              M_DMA_DSCRA_INTERRUPT;
            pRxDscr->dscr_b = 0;

            /* mark the descriptor ready to receive  again */
            SB_DMA_REG_WRITE(pRxDma->sbdma_dscrcnt, 1);

            /* advance management index */
            pRxDma->sbdma_addindex = (pRxDma->sbdma_addindex + 1) % 
                                     pRxDma->sbdma_maxdescr;
            pRxDma->sbdma_remindex = (pRxDma->sbdma_remindex + 1) % 
                                     pRxDma->sbdma_maxdescr;
            }
        else
            {
            DRV_LOG (DRV_DEBUG_RX, ("Good Frame \n"), 0, 0, 0, 0, 0, 0);
            END_ERR_ADD (&pDrvCtrl->endObj, MIB2_IN_UCAST, +1);

            len = (int)G_DMA_DSCRB_PKT_SIZE(pRxDscr->dscr_b) - ETH_CRC_LEN;
            pData = (char *)PHYSTOV((UINT32)(pRxDscr->dscr_a));

            pBuf=pData;
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - pData= 0x%08x, len=%d \n", 
                     (int)pBuf, len,3,4,5,6);
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
                     pBuf[0], pBuf[1], pBuf[2], pBuf[3], pBuf[4], pBuf[5]);
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
                     pBuf[6], pBuf[7], pBuf[8], pBuf[9], pBuf[10], pBuf[11]);
            DRV_LOG (DRV_DEBUG_RXD, 
                     "rx - *pBuf= 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x \n",
                     pBuf[12], pBuf[13], pBuf[14], pBuf[15], pBuf[16], 
                     pBuf[17]);
#ifdef L3P_TEST 
            pMblk = NET_MBLK_ALLOC();
            pClBlk = NET_CL_BLK_ALLOC();
            pBuf = NET_BUF_ALLOC ();
            NET_CL_BLK_JOIN (pClBlk, pData - VXW_RCV_BUF_OFFSET, 
                             MAX_MAC_FRAME_SIZE);
            NET_MBLK_CL_JOIN(pMblk, pClBlk);
#endif
            pMblk->mBlkHdr.mFlags |= M_PKTHDR;
            pMblk->mBlkHdr.mLen = len;
            pMblk->mBlkPktHdr.len = pMblk->mBlkHdr.mLen;
#ifdef L3P_TEST 
            pMblk->mBlkHdr.mData = pData;
#endif
#ifndef L3P_TEST 
            bcopy (pData, (char *)pMblk->mBlkHdr.mData, len);
#else
	    endRcvRtnCall(&pDrvCtrl->endObj, pMblk);
#endif

	    pRxDscr = pRxDma->sbdma_dscrtable + pRxDma->sbdma_addindex;

#ifndef L3P_TEST
            pRxDscr->dscr_a = KVTOPHYS((UINT32)pData) |
#else
            pRxDscr->dscr_a = KVTOPHYS((UINT32)pBuf) |
#endif
                              V_DMA_DSCRA_A_SIZE(NUMCACHEBLKS(
                                                 MAX_MAC_FRAME_SIZE)) |
                              M_DMA_DSCRA_INTERRUPT |
                              2;
            pRxDscr->dscr_b = 0;

            /* mark the descriptor ready to receive  again */
            SB_DMA_REG_WRITE( pRxDma->sbdma_dscrcnt, 1);

            /* advance management index */
            pRxDma->sbdma_addindex = (pRxDma->sbdma_addindex + 1) % 
                                     pRxDma->sbdma_maxdescr;
            pRxDma->sbdma_remindex = (pRxDma->sbdma_remindex + 1) % 
                                     pRxDma->sbdma_maxdescr;

            return OK;
	    }
        }

    DRV_LOG (DRV_DEBUG_RX, ("bcm1250MacPollRcv done\n"), 0, 0, 0, 0, 0, 0);
    return (EAGAIN);
    }


/*******************************************************************************
*
* bsm1250MacPollStart - start polled mode operations
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS bcm1250MacPollStart
    (
    DRV_CTRL * pDrvCtrl       /* device to be polled */
    )
    {
    int         oldLevel;

    DRV_LOG (DRV_DEBUG_POLL, "PollStart enter ......\n",1,2,3,4,5,6);

    oldLevel = intLock();

    DRV_FLAGS_SET (BCM1250_MAC_POLLING);

    /* disable mac interrupts */
    /*SB_MAC_REG_WRITE (pDrvCtrl->sbm_imr, 0);*/

    /* diasble VxW interrupt */
    bcm1250IntDisable(pDrvCtrl->intsource);

    intUnlock (oldLevel);

    DRV_LOG (DRV_DEBUG_POLL, "PollStart done\n",1,2,3,4,5,6);

    return (OK);
    }

/*******************************************************************************
*
* bcm1250MacPollStop - stop polled mode operations
*
* This function terminates polled mode operation.  The device returns to
* interrupt mode.
*
* The device interrupts are enabled, the current mode flag is switched
* to indicate interrupt mode and the device is then reconfigured for
* interrupt operation.
*
* RETURNS: OK or ERROR.
*/

LOCAL STATUS bcm1250MacPollStop
    (
    DRV_CTRL * pDrvCtrl       /* device to be polled */
    )
    {
    int        oldLevel;

    DRV_LOG (DRV_DEBUG_POLL, "PollStop enter ......\n",1,2,3,4,5,6);

    oldLevel = intLock();

    bcm1250IntEnable(pDrvCtrl->intsource);

    /*SB_MAC_REG_WRITE(pDrvCtrl->sbm_imr,
                     (M_MAC_INT_CHANNEL << S_MAC_TX_CH0) |
                     (M_MAC_INT_CHANNEL << S_MAC_RX_CH0) |
                     (M_MAC_INT_CHANNEL << S_MAC_TX_CH1) |
                     (M_MAC_INT_CHANNEL << S_MAC_RX_CH1));  */

    DRV_FLAGS_CLR (BCM1250_MAC_POLLING);

    intUnlock (oldLevel);

    DRV_LOG (DRV_DEBUG_POLL, "PollStop done\n",1,2,3,4,5,6);

    return (OK);
    }


/*******************************************************************************
* 
* bcm1250MacRxFilterSet
*
* Set rx filter register according to DRV FLAGS
*
* RETURNS: void.
*/
LOCAL void bcm1250MacRxFilterSet
    (
    DRV_CTRL    *pDrvCtrl
    )
    {
    UINT64       rxFilterCfg;

    DRV_LOG (DRV_DEBUG_IOCTL, "RxFilter\n", 0, 0, 0, 0, 0, 0);

    /* read the current rx filter reg */
    rxFilterCfg = SB_MAC_REG_READ (pDrvCtrl->sbm_rxfilter);

    if (DRV_FLAGS_ISSET (BCM1250_MAC_MCAST))
        {
        rxFilterCfg |= M_MAC_MCAST_EN;
        }

    if (DRV_FLAGS_ISSET (BCM1250_MAC_PROMISC))
        rxFilterCfg |= M_MAC_ALLPKT_EN;

    SB_MAC_REG_WRITE (pDrvCtrl->sbm_rxfilter, rxFilterCfg);

    return;
    }


/*******************************************************************************
* 
* Macro and functions to calculate dest addr hash value.
* bcm1250 manual page 9-190
*/
#define sbeth_mchash(mca)       ((sbeth_crc32(mca, EADDR_LEN) >> 1) & 0x1FF)
static unsigned sbeth_crc32(unsigned char *databuf,int datalen)
{
    unsigned int idx, crc = 0xFFFFFFFFUL;

    static unsigned int crctab[] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };

    for (idx = 0; idx < datalen; idx++) {
        crc ^= *databuf++;
        crc = (crc >> 4) ^ crctab[crc & 0xf];
        crc = (crc >> 4) ^ crctab[crc & 0xf];
        }
    return crc;
}


/*******************************************************************************
* 
* bcm1250MacHashRegAdd
*
* Set Receiver Hash table registers
*
* RETURNS: should alwas OK
*/
LOCAL STATUS bcm1250MacHashRegAdd
    (
    DRV_CTRL *  pDrvCtrl,       /* pointer to device control structure */
    char     *  pAddr
    )
    {
    int          dw_offset, hash_val;
    UINT64       reg, oldReg;
    sbmac_port_t port;

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegAdd enter ...... \n", 
             0, 0, 0, 0, 0, 0);

    hash_val = sbeth_mchash(pAddr);

    dw_offset = hash_val >> 6;
    reg = 1 << ((hash_val&0x3f) - 1);

    port = R_MAC_HASH_BASE;

    oldReg = SB_MAC_REG_READ(port + dw_offset*8);
    SB_MAC_REG_WRITE(port + dw_offset*8, reg | oldReg);

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegAdd done\n", 0, 0, 0, 0, 0, 0);

    return (OK);
    }


/*******************************************************************************
* 
* bcm1250MacHashRegSet
*
* Set Receiver Hash table registers
*
* RETURNS: should alwas OK
*/
LOCAL STATUS bcm1250MacHashRegSet
    (
    DRV_CTRL *  pDrvCtrl       /* pointer to device control structure */
    )
    {
    int          dw_offset, hash_val, idx;
    UINT64       reg, oldReg;
    sbmac_port_t port;
    ETHER_MULTI *mCastNode = NULL;

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegSet enter ...... \n", 
             0, 0, 0, 0, 0, 0);

    /* clear hash table */
    port = R_MAC_HASH_BASE;
    for (idx = 0; idx < MAC_HASH_COUNT; idx++)
        {
        SB_MAC_REG_WRITE(port,0);
        port += sizeof(UINT64);
        }

    port = R_MAC_HASH_BASE;

    for (mCastNode = (ETHER_MULTI *) lstFirst (&pDrvCtrl->endObj.multiList);
         mCastNode != NULL;
         mCastNode = (ETHER_MULTI *) lstNext (&mCastNode->node))
        {
        hash_val = sbeth_mchash(mCastNode->addr);

        dw_offset = hash_val >> 6;
        reg = 1 << ((hash_val&0x3f) - 1);

        oldReg = SB_MAC_REG_READ(port + dw_offset*8);
        SB_MAC_REG_WRITE(port + dw_offset*8, reg | oldReg);
        }

    DRV_LOG (DRV_DEBUG_IOCTL, "MacHashRegSet done\n", 0, 0, 0, 0, 0, 0);

    return (OK);
    }


/*******************************************************************************
* 
* bcm1250MacMCastAddrAdd - add a multicast address for the device
*
* This routine adds a multicast address to whatever the driver
* is already listening for.
*
* SEE ALSO: bcm1250MacMCastAddrDel() bcm1250MacMCastAddrGet()
*
* RETURNS: OK or ERROR.
*/
LOCAL STATUS bcm1250MacMCastAdd
    (
    DRV_CTRL *  pDrvCtrl,
    char *      pAddr
    )
    {
    int         retVal;

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacMCastAdd\n", 0, 0, 0, 0, 0, 0);

    retVal = etherMultiAdd (&pDrvCtrl->endObj.multiList, pAddr);

    if (retVal == ENETRESET)
        {
        pDrvCtrl->endObj.nMulti++;

        return bcm1250MacHashRegAdd (pDrvCtrl, pAddr);
        }

    return ((retVal == OK) ? OK : ERROR);
    }


/*******************************************************************************
* 
* bcm1250MacMCastAddrDel - delete a multicast address for the device
*
* This routine deletes a multicast address from the current list of
* multicast addresses.
*
* SEE ALSO: bcm1250MacMCastAddrAdd() bcm1250MacMCastAddrGet()
*
* RETURNS: OK or ERROR.
*/
LOCAL STATUS bcm1250MacMCastDel
    (
    DRV_CTRL *  pDrvCtrl,
    char *      pAddr
    )
    {
    int         retVal;

    DRV_LOG (DRV_DEBUG_IOCTL, "bcm1250MacMCastDel\n", 0, 0, 0, 0, 0, 0);

    retVal = etherMultiDel (&pDrvCtrl->endObj.multiList, pAddr);

    if (retVal == ENETRESET)
        {
        pDrvCtrl->endObj.nMulti--;

        return bcm1250MacHashRegSet (pDrvCtrl);
        }

    return ((retVal == OK) ? OK : ERROR);
    }


/*******************************************************************************
* 
* bcm1250MacMCastAddrGet - get the current multicast address list
*
* This routine returns the current multicast address list in <pTable>
*
* SEE ALSO: bcm1250MacMCastAddrAdd() bcm1250MacMCastAddrDel()
*
* RETURNS: OK or ERROR.
*/
LOCAL STATUS bcm1250MacMCastGet
    (
    DRV_CTRL *  pDrvCtrl,
    MULTI_TABLE *pTable
    )
    {
    DRV_LOG (DRV_DEBUG_IOCTL, "sbMCastGet\n", 0, 0, 0, 0, 0, 0);

    return (etherMultiGet (&pDrvCtrl->endObj.multiList, pTable));
    }


/*******************************************************************************
* 
* bcm1250MacIoctl - the driver I/O control routine
*
* Process an ioctl request.
*
* RETURNS: A command specific response, usually OK or ERROR.
*/
LOCAL int bcm1250MacIoctl
    (
    DRV_CTRL * pDrvCtrl,
    int cmd,
    caddr_t data
    )
    {
    int         error=0;
    long        value;
    int         savedFlags;
    sbmac_port_t port;
    UINT64      reg;
    END_OBJ *   pEndObj=&pDrvCtrl->endObj;

    DRV_LOG (DRV_DEBUG_IOCTL, "Ioctl unit=0x%x cmd=%d data=0x%x\n",
             pDrvCtrl->unit, cmd, (int)data, 0, 0, 0);

    switch (cmd)
        {
        case EIOCSADDR:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCSADDR\n",0,0,0,0,0,0);
            if (data == NULL)
                error = EINVAL;
            else
                {
                /* Copy and install the new address */
                bcopy ((char *)data, (char *)END_HADDR(pEndObj),
                       END_HADDR_LEN(pEndObj));

                reg =  sbmac_addr2reg((char *)data);
                DRV_LOG (DRV_DEBUG_IOCTL, "mac addr = 0x%08x%08x\n",
                         (int)(reg>>32), reg, 3, 4, 5, 6);

		/* need pass 1 bug workaround */
                port = R_MAC_ADDR_BASE;
                SB_MAC_REG_WRITE(port,reg);

                port = R_MAC_ETHERNET_ADDR;
                SB_MAC_REG_WRITE(port,reg);
                }
            break;

        case EIOCGADDR:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCGADDR\n",0,0,0,0,0,0);
            if (data == NULL)
                error = EINVAL;
            else
                bcopy ((char *)END_HADDR(pEndObj), (char *)data,
                        END_HADDR_LEN(pEndObj));
            break;

        case EIOCSFLAGS:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCSAFLGS\n",0,0,0,0,0,0);
            value = (long) data;
            if (value < 0)
                {
                value = -value;
                value--;
                END_FLAGS_CLR (pEndObj, value);
                }
            else
                {
                END_FLAGS_SET (pEndObj, value);
                }

            /* handle IFF_PROMISC */
            savedFlags = DRV_FLAGS_GET();
            if (END_FLAGS_ISSET (pEndObj, IFF_PROMISC))
                DRV_FLAGS_SET (BCM1250_MAC_PROMISC);
            else
                DRV_FLAGS_CLR (BCM1250_MAC_PROMISC);

            if (END_FLAGS_GET(pEndObj) & (IFF_MULTICAST | IFF_ALLMULTI))
                DRV_FLAGS_SET (BCM1250_MAC_MCAST);
            else
                DRV_FLAGS_CLR (BCM1250_MAC_MCAST);

            DRV_LOG (DRV_DEBUG_IOCTL, 
                     "endFlags=0x%x savedFlags=0x%x newFlags=0x%x\n",
                     END_FLAGS_GET(pEndObj), savedFlags, DRV_FLAGS_GET(), 
                     0, 0, 0);

            if ((DRV_FLAGS_GET() != savedFlags) && (END_FLAGS_GET(pEndObj) & 
                IFF_UP))
                bcm1250MacRxFilterSet (pDrvCtrl);

            break;

        case EIOCGFLAGS:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCGFLAGS: 0x%x: 0x%x\n",
                    pEndObj->flags, *(long *)data, 0, 0, 0, 0);

            if (data == NULL)
                error = EINVAL;
            else
                *(long *)data = END_FLAGS_GET(pEndObj);

            break;

        case EIOCMULTIADD:
            DRV_LOG(DRV_DEBUG_IOCTL, "EIOCMULTIADD\n",0,0,0,0,0,0);
            error = bcm1250MacMCastAdd(pDrvCtrl, (char *) data);
            break;

        case EIOCMULTIDEL:
            DRV_LOG(DRV_DEBUG_IOCTL, "EIOCMULTIDEL\n",0,0,0,0,0,0);
            error = bcm1250MacMCastDel(pDrvCtrl, (char *) data);
            break;

        case EIOCMULTIGET:
            DRV_LOG(DRV_DEBUG_IOCTL, "EIOCMULITGET\n",0,0,0,0,0,0);
            error = bcm1250MacMCastGet(pDrvCtrl, (MULTI_TABLE *) data);
            break;

        case EIOCPOLLSTART:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCPOLLSTART\n",
                                            0, 0, 0, 0, 0, 0);
            bcm1250MacPollStart (pDrvCtrl);
            break;

        case EIOCPOLLSTOP:
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCPOLLSTOP\n",
                                            0, 0, 0, 0, 0, 0);
            bcm1250MacPollStop (pDrvCtrl);
            break;

        case EIOCGMIB2:                      /* move to mux */
            DRV_LOG (DRV_DEBUG_IOCTL, "EIOCGMIBS2\n",0,0,0,0,0,0);
            if (data == NULL)
                error=EINVAL;
            else
                bcopy((char *)&pEndObj->mib2Tbl, (char *)data,
                      sizeof(pEndObj->mib2Tbl));
            break;

        default:
            DRV_LOG (DRV_DEBUG_IOCTL, ("IOCTL Invalid Command\n"),
                                        0, 0, 0, 0, 0, 0);
            error = EINVAL;
        }

    return (error);
    }

/*******************************************************************************
*  
* bcm1250MacSetSpeed(pDrvCtrl,speed)
* 
*  Configure LAN speed for the specified MAC
*
*   Input parameters:
*      speed - speed to set MAC to (see sbeth_speed_t enum)
*
*   Return value:
*     1 if successful
*     0 indicates invalid parameters
*/
 
LOCAL int bcm1250MacSetSpeed
    (
    DRV_CTRL *pDrvCtrl, 
    sbmac_speed_t speed
    )
    {
    UINT64 cfg;
    UINT64 framecfg;
 
    /*
     * Read current register values
     */
 
    cfg = SB_MAC_REG_READ(pDrvCtrl->sbm_maccfg);
    framecfg = SB_MAC_REG_READ(pDrvCtrl->sbm_framecfg);
 
    /*
     * Mask out the stuff we want to change
     */
 
    cfg &= ~(M_MAC_BURST_EN | M_MAC_SPEED_SEL);
    framecfg &= ~(M_MAC_IFG_RX | M_MAC_IFG_TX | M_MAC_IFG_THRSH |
                  M_MAC_SLOT_SIZE);
     /*
     * Now add in the new bits
     */
 
    switch (speed) 
        {
        case sbmac_speed_10:
            framecfg |= V_MAC_IFG_RX_10 |
                V_MAC_IFG_TX_10 |
                K_MAC_IFG_THRSH_10 |
                V_MAC_SLOT_SIZE_10;
            cfg |= V_MAC_SPEED_SEL_10MBPS;
            break;
 
        case sbmac_speed_100:
            framecfg |= V_MAC_IFG_RX_100 |
                V_MAC_IFG_TX_100 |
                V_MAC_IFG_THRSH_100 |
                V_MAC_SLOT_SIZE_100;
            cfg |= V_MAC_SPEED_SEL_100MBPS ;
            break;
 
        case sbmac_speed_1000:
            framecfg |= V_MAC_IFG_RX_1000 |
                V_MAC_IFG_TX_1000 |
                V_MAC_IFG_THRSH_1000 |
                V_MAC_SLOT_SIZE_1000;
            cfg |= V_MAC_SPEED_SEL_1000MBPS | M_MAC_BURST_EN;
            break;
 
        default:
            return 0;
        }
    /*
     * Send the bits back to the hardware
     */
 
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_framecfg,framecfg);
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_maccfg,cfg);
 
    return 1; 
    }                                             

/*******************************************************************************
*  
*  bcm1250MacSetDuplex(pDrvCtrl,duplex,fc)
*
*  Set Ethernet duplex and flow control options for this MAC
*
*  Input parameters:
*      duplex - duplex setting (see sbeth_duplex_t)
*      fc - flow control setting (see sbeth_fc_t)
*
*  Return value:
*      1 if ok
*      0 if an invalid parameter combination was specified
*/
 
LOCAL int bcm1250MacSetDuplex
    (
    DRV_CTRL *pDrvCtrl, 
    sbmac_duplex_t duplex, 
    sbmac_fc_t fc
    )
    {
    UINT64 cfg;
 
    /*
     * Read current register values
     */
 
    cfg = SB_MAC_REG_READ(pDrvCtrl->sbm_maccfg);
 
    /*
     * Mask off the stuff we're about to change
     */
 
    cfg &= ~(M_MAC_FC_SEL | M_MAC_FC_CMD | M_MAC_HDX_EN);
 
 
    switch (duplex) 
        {
        case sbmac_duplex_half:
            switch (fc) 
                {                              
                case sbmac_fc_disabled:
                    cfg |= M_MAC_HDX_EN | V_MAC_FC_CMD_DISABLED;
                    break;
 
                case sbmac_fc_collision:
                    cfg |= M_MAC_HDX_EN | V_MAC_FC_CMD_ENABLED;
                    break;
 
                case sbmac_fc_carrier:
                    cfg |= M_MAC_HDX_EN | V_MAC_FC_CMD_ENAB_FALSECARR;
                    break;
 
                case sbmac_fc_frame:            /* not valid in half duplex */
                default:                        /* invalid selection */
                    return 0;
                }
                cfg &= ~(M_MAC_BURST_EN);
            break;
 
        case sbmac_duplex_full:
            switch (fc) 
               {
                case sbmac_fc_disabled:
                    cfg |= V_MAC_FC_CMD_DISABLED;
                    break;
 
                case sbmac_fc_frame:
                    cfg |= V_MAC_FC_CMD_ENABLED;
                    break;
 
                case sbmac_fc_collision:        /* not valid in full duplex */
                case sbmac_fc_carrier:          /* not valid in full duplex */
                    /* fall through */
                default:
                    return 0;
                }
            break;       
        }
 
    /*
     * Send the bits back to the hardware
     */
 
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_maccfg,cfg);
 
    return 1;
    }   
 

/*********************************************************************
* 
* sbeth_mii_sync - send sync bits to phy 
* 
* Return value: None
*/
 
static void sbeth_mii_sync
    (
    DRV_CTRL * pDrvCtrl
    )
    {
    int cnt;
    UINT64 bits;
 
    bits = M_MAC_MDIO_DIR_OUTPUT | M_MAC_MDIO_OUT;
 
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits);
 
    for (cnt = 0; cnt < 32; cnt++) 
        {
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits | M_MAC_MDC);
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits);
        }
    }
                                                                                
/**********************************************************************
* 
* beth_mii_senddata - send number of bits to phy
*  
* Return value: None
*/
 
static void sbeth_mii_senddata
    (
    DRV_CTRL * pDrvCtrl,
    unsigned int data, 
    int bitcnt
    )
    {
    int i;
    UINT64 bits;
    unsigned int curmask;
 
    bits = M_MAC_MDIO_DIR_OUTPUT;
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits);
 
    curmask = 1 << (bitcnt - 1);
 
    for (i = 0; i < bitcnt; i++) 
        {
        if (data & curmask) bits |= M_MAC_MDIO_OUT;
        else bits &= ~M_MAC_MDIO_OUT;
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits);
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits | M_MAC_MDC);
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,bits);
        curmask >>= 1;
        }
    }                                            

/**********************************************************************
* 
* sbeth_mii_read - read from phy register
*
* it first send sync bits, then START and READ command, followed by
* phy address and register index. It switch to input mode and clock 
* in phy register data
*/
 
static unsigned int sbeth_mii_read
    (
    DRV_CTRL * pDrvCtrl,
    int phyaddr,
    int regidx
    )
    {
    int idx;
    int error;
    int regval;
 
    /*
     * Synchronize ourselves so that the PHY knows the next
     * thing coming down is a command
     */
 
    sbeth_mii_sync(pDrvCtrl);
 
    /*
     * Send the data to the PHY.  The sequence is
     * a "start" command (2 bits)
     * a "read" command (2 bits)
     * the PHY addr (5 bits)
     * the register index (5 bits)
     */  

    sbeth_mii_senddata(pDrvCtrl,MII_COMMAND_START, 2);
    sbeth_mii_senddata(pDrvCtrl,MII_COMMAND_READ, 2);
    sbeth_mii_senddata(pDrvCtrl,phyaddr, 5);
    sbeth_mii_senddata(pDrvCtrl,regidx, 5);
 
    /*
     * Switch the port around without a clock transition.
     */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT);
 
    /*
     * Send out a clock pulse to signal we want the status
     */
 
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT | M_MAC_MDC);
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT);
 
    /*
     * If an error occured, the PHY will signal '1' back
     */
    error = SB_MAC_REG_READ(pDrvCtrl->sbm_mdio) & M_MAC_MDIO_IN;
 
    /*
     * Issue an 'idle' clock pulse, but keep the direction
     * the same.
     */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT | M_MAC_MDC);
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT);
 
    regval = 0;
    for (idx = 0; idx < 16; idx++) 
        {
        regval <<= 1;
 
        if (error == 0) 
            {
            if (SB_MAC_REG_READ(pDrvCtrl->sbm_mdio) & M_MAC_MDIO_IN) 
                regval |= 1;
            }
 
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT | M_MAC_MDC);
        SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_INPUT);
        }
 
    /* Switch back to output */
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_OUTPUT);
 
    if (error == 0) 
        return regval;
    return 0;
    }                                                              

#ifdef PHY_INT
 
/**********************************************************************
* 
* sbeth_mii_write - write to phy register
*
* It first sends START and READ command, followed by phy address and 
* register index. It then sends ACK and register value
*
* Return value: None
*/
 
static void sbeth_mii_write
    (
    DRV_CTRL * pDrvCtrl,
    int phyaddr,int regidx,
    unsigned int regval
    )
    {
 
    sbeth_mii_sync(pDrvCtrl);
 
    sbeth_mii_senddata(pDrvCtrl,MII_COMMAND_START,2);
    sbeth_mii_senddata(pDrvCtrl,MII_COMMAND_WRITE,2);
    sbeth_mii_senddata(pDrvCtrl,phyaddr, 5);
    sbeth_mii_senddata(pDrvCtrl,regidx, 5);
    sbeth_mii_senddata(pDrvCtrl,MII_COMMAND_ACK,2);
    sbeth_mii_senddata(pDrvCtrl,regval,16);
 
    SB_MAC_REG_WRITE(pDrvCtrl->sbm_mdio,M_MAC_MDIO_DIR_OUTPUT);
    }                                                            
#endif


/**********************************************************************
* 
* sbeth_mii_findphy - poke all the possible phy addrs to see if we connect
* to one
*
* Return value: None
*/
static void sbeth_mii_findphy
    (
    DRV_CTRL * pDrvCtrl
    )
    {
    int phy;
    uint16_t bmsr;
 
    for (phy = 0; phy < 31; phy++) 
        {
        bmsr = sbeth_mii_read(pDrvCtrl,phy,MII_BMSR);
        if (bmsr != 0) 
            {
            pDrvCtrl->phyAddr = phy;
            return;
            }
        }
    printf("Found no PHY \n");
 
    pDrvCtrl->phyAddr = 1;
    }                                                                     
  

/*********************************************************************
*  sbeth_mii_poll
*
*  Ask the PHY what is going on, and configure speed appropriately.
*  For the moment, we only support automatic configuration.
*
*  Input parameters:
*      s - sbeth structure
*
*  Return value:
*      nothing
*/
void sbeth_mii_poll
    (
    DRV_CTRL * pDrvCtrl
    )
    {
    uint16_t bmsr,bmcr,k1stsr,anlpar;
 
    /* Read the mode status and mode control registers. */
    bmsr = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_BMSR);
    bmcr = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_BMCR);
 
    /* get the link partner status */
    anlpar = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_ANLPAR);
 
    /* if supported, read the 1000baseT register */
    if (bmsr & BMSR_1000BT_XSR) 
        {
        k1stsr = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_K1STSR);
        }
    else 
        {
        k1stsr = 0;
        }
 
    printf("Link speed: ");

    if (k1stsr & K1STSR_LP1KFD) 
        {
        pDrvCtrl->sbm_speed = sbmac_speed_1000;
        pDrvCtrl->sbm_duplex = sbmac_duplex_full;
        pDrvCtrl->sbm_fc = sbmac_fc_frame;                                    
        printf("1000BaseT FDX\n");
        }
    else if (k1stsr & K1STSR_LP1KHD) 
        {
        pDrvCtrl->sbm_speed = sbmac_speed_1000;
        pDrvCtrl->sbm_duplex = sbmac_duplex_half;
        pDrvCtrl->sbm_fc = sbmac_fc_disabled;            
        printf("1000BaseT HDX\n");
        }
    else if (anlpar & ANLPAR_TXFD) 
        {
        pDrvCtrl->sbm_speed = sbmac_speed_100;
        pDrvCtrl->sbm_duplex = sbmac_duplex_full;
        pDrvCtrl->sbm_fc = (anlpar & ANLPAR_PAUSE) ? 
                           sbmac_fc_frame : sbmac_fc_disabled;
        printf("100BaseT FDX\n");
        }
    else if (anlpar & ANLPAR_TXHD) 
        {
        pDrvCtrl->sbm_speed = sbmac_speed_100;
        pDrvCtrl->sbm_duplex = sbmac_duplex_half;
        pDrvCtrl->sbm_fc = sbmac_fc_disabled;         
        printf("100BaseT HDX\n");
        }
    else if (anlpar & ANLPAR_10FD) 
        {
        pDrvCtrl->sbm_speed = sbmac_speed_10;
        pDrvCtrl->sbm_duplex = sbmac_duplex_full;
        pDrvCtrl->sbm_fc = sbmac_fc_frame;
        printf("10BaseT FDX\n");
        }
    else if (anlpar & ANLPAR_10HD) 
        {
        pDrvCtrl->sbm_speed = sbmac_speed_10;
        pDrvCtrl->sbm_duplex = sbmac_duplex_half;
        pDrvCtrl->sbm_fc = sbmac_fc_collision;                
        printf("10BaseT HDX\n");
        }
    else 
        {
        printf("Unknown\n");
        }
 
    printf("BMSR=%04X, BMCR=%04X, 1KSTSR=%04X, ANLPAR=%04X\n",bmsr,bmcr,
            k1stsr,anlpar);
    }                                                           


/***********************************************************************
*
* sbrdshow - display tx dma register values
*
*/
void sbrdshow 
    (
    int inst, 
    int chan
    )
    {
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    SB_MAC_DMA * pDma;
    UINT64   reg;
    UINT32   reg_hi, reg_lo;

    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);

    if ( (chan > 1) ||  (chan < 0) )
        printf("channel number must be 0 or 1\n");

    pDma = &pDrvCtrl->rxDma[chan];

    reg = SB_DMA_REG_READ(pDma->sbdma_config0);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("rx%d config0 = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_config1);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("rx%d config1 = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_dscrcnt);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("rx%d dscrcnt = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_dscrbase);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("rx%d dscrbase = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_curdscr);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("rx%d curdscr = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    printf("rx%d remindex = 0x%08x \n", chan, pDma->sbdma_remindex);
    printf("rx%d addindex = 0x%08x \n", chan, pDma->sbdma_addindex);
    }

/***********************************************************************
*
* sbtdshow - display rx dma register values
*
*/
void sbtdshow 
    (
    int inst, 
    int chan
    )
    {
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    SB_MAC_DMA * pDma;
    UINT64   reg;
    UINT32   reg_hi, reg_lo;

    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);

    if ( (chan > 1) ||  (chan < 0) )
        printf("channel number must be 0 or 1\n");

    pDma = &pDrvCtrl->txDma[chan];

    reg = SB_DMA_REG_READ(pDma->sbdma_config0);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx%d config0 = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_config1);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx%d config1 = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_dscrcnt);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx%d dscrcnt = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_dscrbase);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx%d dscrbase = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    reg = SB_DMA_REG_READ(pDma->sbdma_curdscr);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx%d curdscr = 0x%08x%08x \n", chan, reg_hi, reg_lo);

    printf("tx%d remindex = 0x%08x \n", chan, pDma->sbdma_remindex);
    printf("tx%d addindex = 0x%08x \n", chan, pDma->sbdma_addindex);
    }


/***********************************************************************
*
* sbmshow - display mac register values
*
*/
void sbmshow 
    (
    int inst
    )
    {
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    UINT64   reg;
    UINT32   reg_hi, reg_lo;

    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_macenable);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac macenable = 0x%08x%08x \n", reg_hi, reg_lo);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_maccfg);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac maccfg = 0x%08x%08x \n",  reg_hi, reg_lo);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_fifocfg);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac fifocfg = 0x%08x%08x \n", reg_hi, reg_lo);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_framecfg);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac framecfg = 0x%08x%08x \n", reg_hi, reg_lo);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_rxfilter);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac rxfilter = 0x%08x%08x \n", reg_hi, reg_lo);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_imr);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac imr = 0x%08x%08x \n", reg_hi, reg_lo);

    reg = SB_MAC_REG_READ(pDrvCtrl->sbm_isr);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("mac isr = 0x%08x%08x \n", reg_hi, reg_lo);
    }


/***********************************************************************
*
* sbphyshow - display phy register values
*
*/
void sbphyshow 
    (
    int inst
    )
    {
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);

    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);

    printf("PhyReg %02X val %04X\n",0,
           sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,0));
    printf("PhyReg %02X val %04X\n",1,
           sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,1));
    
    printf("PhyReg %02X val %04X\n",0x1a,
           sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,0x1a));
    printf("PhyReg %02X val %04X\n",0x1b,
           sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,0x1b));
    }

/*
void nbufshow(int inst)
{
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
 
    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);                                       
 
    printf("nBufAllocStart=%d nBufAllocCopy=%d, \n nBufFreeCopy=%d, nBufFreeTxH=%d nBufAllocNull=%d \n", 
              nBufAllocStart, nBufAllocCopy, nBufFreeCopy, nBufFreeTxH, nBufAllocNull);
    printf("nTxAddIdxCopy=%d;nTxAddIdxZCopy=%d;nTxRemIdxCopy=%d;nTxRemIdx=%d;\n",
            nTxAddIdxCopy,nTxAddIdxZCopy,nTxRemIdxCopy,nTxRemIdx);
}
*/

#ifdef PHY_INT
 
LOCAL void HandleCableInOut(DRV_CTRL *pDrvCtrl, char bCableIn)
/*
****************************************************************************
**********************
** Description:
**     This routine is called from ibmEmacPHYInt to handle a cable being
plugged in/out. This routine
** will indicate to the MUX that the link is up/down, and will perform any
reconfiguration of the EMAC.
**
** Parameters:
**    pDrvCtrl: points to the device control structure
**     bCableIn: TRUE if cable is plugged in, otherwise FALSE
**
** Returns:
**     NOTHING
**
****************************************************************************
******************* */
{
 UINT16 PHYValue;
 
    PHYValue = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_ISR);/* clean
ints*/
 
 /* As per the specs, we need to read this register twice to get the correct
value
 ** for the link status bit (it latches).
 */
 PHYValue = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_BMSR); /* get the
link status */
 PHYValue = sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,MII_BMSR); /* get the
link status */
 
 if (PHYValue & PHY_BMSR_LS)
  bCableIn = TRUE;
 else
  bCableIn = FALSE;
                                                                                
 if (bCableIn == FALSE)
  /*LinkStatusChange(pDrvCtrl, LINK_DOWN, ONLY_SEND_EVENT_NO);*/
  logMsg("link is down\n", 1,2,3,4,5,6);
 else
  {
  /* Note that the LinkStatusChange call below is necessary, as the
PHY_BMSR_LS will not indicate
  ** the correct link status when the speed or duplex mode is forced, and
the cable is removed and
  ** replaced.
  */
  /*LinkStatusChange(pDrvCtrl, LINK_UP, ONLY_SEND_EVENT_NO);*/
  logMsg("link is up\n", 1,2,3,4,5,6);
  }
}
 
void bPHYInt
    (
    DRV_CTRL *  pDrvCtrl       /* pointer to DRV_CTRL structure */
    )
{
/*vvv = 123;*/
    logMsg("PhyReg %02X val %04X\n",0x1a,
           sbeth_mii_read(pDrvCtrl,pDrvCtrl->phyAddr,0x1a),3,4,5,6); 
    logMsg("in phy int \n",0,0,0,0,0,0);
 
  netJobAdd((FUNCPTR)HandleCableInOut, (int)pDrvCtrl, FALSE, 0, 0, 0); /*
this is done */
 
}
 
 
LOCAL STATUS bPHYDisconnect(DRV_CTRL* pDrvCtrl)
{
    sbeth_mii_write(pDrvCtrl,pDrvCtrl->phyAddr, MII_IMR, 0xffff);
    bcm1250IntDisable (pDrvCtrl->intphysource);
    bcm1250IntDisconnect(pDrvCtrl->intphysource);
    return OK;
 
}
                                                                                
LOCAL STATUS bPHYConnect(DRV_CTRL* pDrvCtrl)
{
    STATUS       retVal;
    sbphyshow(1);
    retVal = bcm1250IntConnect (pDrvCtrl->intphysource, pDrvCtrl->ivecnum,
bPHYInt, (int) pDrvCtrl);
    taskDelay(100);
    logMsg("bPHYConnect source %d vector %d address %d\n",
pDrvCtrl->intphysource, pDrvCtrl->ivecnum,pDrvCtrl->phyAddr,0,0,0);
    taskDelay (100);
    if (retVal == ERROR){
        return (ERROR);
        logMsg("bPHYConnect error 1\n",0,0,0,0,0,0);
     }
    taskDelay(100);
    bcm1250IntEnable(pDrvCtrl->intphysource);
    taskDelay(100);
    logMsg ("bPHYConnect after enable\n",0,0,0,0,0,0);
    taskDelay(100);
    sbeth_mii_write(pDrvCtrl,pDrvCtrl->phyAddr, MII_IMR, 0xfffd);
    taskDelay(100);
    logMsg("bPHYConnect finish\n",0,0,0,0,0,0);
    taskDelay(100);
    return OK;
 
}
 
#endif                                                                          

#if 1
void sbrmonclr(int inst)
{
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
 
    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);
 
 
    SB_MAC_REG_WRITE(R_MAC_RMON_TX_BYTES, 0);
    SB_MAC_REG_WRITE(R_MAC_RMON_TX_BAD, 0);
    SB_MAC_REG_WRITE(R_MAC_RMON_TX_GOOD, 0);
 
}
 
void sbrmonshow(int inst)
{
    DRV_CTRL * pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    UINT64   reg;
    UINT32   reg_hi, reg_lo;
 
    pDrvCtrl = (DRV_CTRL *) endFindByName ("sbe", inst);
    if ( pDrvCtrl == NULL )
        printf("can't find sbe%d\n", inst);
 
 
    reg = SB_MAC_REG_READ(R_MAC_RMON_TX_BYTES);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx_byte = 0x%08x%08x \n", reg_hi, reg_lo);
 
    reg = SB_MAC_REG_READ(R_MAC_RMON_TX_BAD);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx_bad = 0x%08x%08x \n", reg_hi, reg_lo);
 
    reg = SB_MAC_REG_READ(R_MAC_RMON_TX_GOOD);
    reg_hi = (UINT32)(reg >> 32); reg_lo = (UINT32)(reg & 0xffffffff);
    printf("tx_good = 0x%08x%08x \n", reg_hi, reg_lo);
}
#endif
