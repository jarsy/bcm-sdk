/* bcm1250MacEnd.h - END style BCM1250 MAC Ethernet driver definitions */

/* Copyright 2002 Wind River Systems, Inc. */

/*********************************************************************
*
* Copyright 2000,2001
* Broadcom Corporation. All rights reserved.
*
* This software is furnished under license to Wind River Systems, Inc.
* and may be used only in accordance with the terms and conditions of 
* this license.  No title or ownership is transferred hereby.
**********************************************************************/     

/* $Id: bcm1250MacEnd.h,v 1.3 2011/07/21 16:14:49 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
01d,20mar02,pgh  Apply code review fixes.
01c,14mar02,pgh  Made the code compliant with the coding standard.
                 Eliminated unused variables.
                 Added some new variables.
01b,07dec01,agf  apply coding standard fix-ups
01a,15nov01,agf  written.
*/

#ifndef __INCbcm1250MacEndh 
#define __INCbcm1250MacEndh 

#ifdef __cplusplus
extern "C" {
#endif                    

#ifndef _ASMLANGUAGE

#define SB_DEV_NAME "sbe"
#define SB_DEV_NAME_LEN 4

#define VXW_RCV_BUF_OFFSET      2
#define ETH_CRC_LEN             4   /* ethernet CRC length */
#define ETH_ADDR_LEN            6   /* ethernet address length */
#define NUM_RDS_DEF             32  /* default number of RX dscrs */
#define NUM_TDS_DEF             32  /* default number of TX dscrs */

#define RXDSCR_LOAN_NUM 32      /* Number of descriptors available to loan */
#define CACHELINESIZE   32
#define MAX_FRAME_SIZE  (ETHERMTU + ENET_HDR_REAL_SIZ + 4 + 2)
#define NUMCACHEBLKS(x) (((x) + (CACHELINESIZE - 1)) / CACHELINESIZE)
#define MAX_FRAME_CACHE_BLKS    (NUMCACHEBLKS (MAX_FRAME_SIZE))

#define BCM1250_MAC_DUAL_CHAN   0x0001  /* Use two RX/TX channels */
#define BCM1250_MAC_POLLING     0x0002  /* Poll mode, io mode */
#define BCM1250_MAC_PROMISC     0x0004  /* Promiscuous, rx mode */
#define BCM1250_MAC_MCAST       0x0008  /* Multicast, rx mode */

#define BCM1250_MAC_SPEED_10     10000000       /* 10 Mbps */
#define BCM1250_MAC_SPEED_100    100000000      /* 100 Mbps */
#define BCM1250_MAC_SPEED_1000   1000000000     /* 1000 Mbps - 1Gbps */
#define BCM1250_MAC_SPEED_DEF    BCM1250_MAC_SPEED_10

typedef unsigned long MAC_REG;

typedef enum 
    { 
    BUF_TYPE_NONE,
    BUF_TYPE_CL,
    BUF_TYPE_MBLK 
    } BUF_TYPE;
 
typedef enum 
    { 
    MAC_SPEED_10, 
    MAC_SPEED_100,
    MAC_SPEED_1000 
    } MAC_SPEED;
 
typedef enum 
    { 
    MAC_DUPLEX_HALF,
    MAC_DUPLEX_FULL 
    } MAC_DUPLEX;
 
typedef enum 
    { 
    MAC_FC_DISABLED, 
    MAC_FC_FRAME,
    MAC_FC_COLLISION, 
    MAC_FC_CARRIER 
    } MAC_FC;             

typedef struct ethDmaDscr 
    {
    UINT64  dscr_a;     /* DMA descriptor first doubleword */
    UINT64  dscr_b;     /* DMA descriptor second doubleword */
    } ETH_DMA_DSCR;

typedef struct txBufTable 
    {
    char *      pBuf;   /* buffer pointer */
    BUF_TYPE    type;   /* buffer type */
    } TX_BUF_TABLE;

typedef struct ethMacDma 
    {
    MAC_REG         regConfig0;     /* DMA config register 0 */
    MAC_REG         regConfig1;     /* DMA config register 1 */
    MAC_REG         regDscrBase;    /* Descriptor base address */
    MAC_REG         regDscrCnt;     /* Descriptor count register */
    MAC_REG         regCurDscr;     /* current descriptor address */

    ETH_DMA_DSCR *  pDscrTable;     /* base of descriptor table */
    TX_BUF_TABLE *  bufTable;       /* buffer table, one per descr */
    int             maxDescr;       /* total # of descriptors in ring */
    int             tailIndex;      /* tail index to DMA buffer descr ring */
    int             headIndex;      /* head index to DMA buffer descr ring */
    int             ringCount;      /* count of DMA buffer descr in ring */

    BOOL            hndlAct;        /* handle routine is active */
    BOOL            txBlocked;      /* doesn't have resources to transmit */
    } ETH_MAC_DMA;

typedef struct drvCtrl 
    {
    END_OBJ     endObj;        /* The class we inherit from. */
    END_ERR     lastError;     /* Last error passed to muxError */
    int         unit;          /* ethernet interface unit number */
    int         iVecNum;       /* interrupt vector number */
    int         intSource;     /* interrupt source */
    int         flags;         /* Our local flags. */

    MAC_REG     regMacBase;
    MAC_REG     regDmaBase;
    MAC_REG     regMacEnable;  /* MAC Enable Register */
    MAC_REG     regMacCfg;     /* MAC Configuration Register */
    MAC_REG     regFifoCfg;    /* FIFO configuration register */
    MAC_REG     regFrameCfg;   /* Frame configuration register */
    MAC_REG     regRxFilter;   /* receive filter register */
    MAC_REG     regIsr;        /* Interrupt status register */
    MAC_REG     regImr;        /* Interrupt mask register */
    MAC_REG     regMdio;       /* mii regster */

    MAC_SPEED   macSpeed;
    MAC_DUPLEX  macDuplex;
    MAC_FC      macFc;

    int         phyAddr;        /* phy address */
    char *      mClBlkBase;     /* Net Pool ClBlk, MBlk addr */
    char *      bufBase;        /* Net Pool cluster buffer addr */

    CL_POOL_ID  clPoolId;       /* cluster pool */

    ETH_MAC_DMA txDma;
    ETH_MAC_DMA rxDma;
    } DRV_CTRL;

#endif  /* _ASMLANGUAGE */
 
#ifdef __cplusplus
}
#endif
 
#endif /* __INCbcm1250MacEndh */              
