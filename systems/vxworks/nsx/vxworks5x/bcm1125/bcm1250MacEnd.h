/* bcm1250MacEnd.h - END based MAC Ethernet header */

/* Copyright 2001 Wind River Systems, Inc. */

/*********************************************************************
*
*  Copyright 2000,2001
*  Broadcom Corporation. All rights reserved.
*
*  This software is furnished under license to Wind River Systems, Inc.
*  and may be used only in accordance with the terms and
*  conditions of this license.  No title or ownership is transferred hereby.
********************************************************************* */     

/* $Id: bcm1250MacEnd.h,v 1.4 2011/07/21 16:14:43 yshtil Exp $
 * This file has been developed or significantly modified by the
 * MIPS Center of Excellence Dedicated Engineering Staff.
 * This notice is as per the MIPS Center of Excellence Master Partner
 * Agreement, do not remove this notice without checking first with
 * WR/Platforms MIPS Center of Excellence engineering management.
 */

/*
modification history
--------------------
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

#define VXW_RCV_BUF_OFFSET   2
#define NET_BUF_LIB_HDR_SIZE 4
#define ETH_CRC_LEN          4
#define EADDR_LEN            6       /* ethernet address length */
#define NUM_RDS_DEF          32      /* default number of rx dscrs */
#define NUM_TDS_DEF          32      /* default number of tx dscrs */

#define SB_RXDSCR_LOAN_NUM   32
#define CL_OVERHEAD          NET_BUF_LIB_HDR_SIZE
#define MAX_MAC_FRAME_SIZE   (ETHERMTU + ENET_HDR_REAL_SIZ + 4 + 2)


#define BCM1250_MAC_MEMOWN       0x0001  /* TDs and RDs allocated by driver */
#define BCM1250_MAC_DUAL_CHAN    0x0002  /* Use two RX/TX channels */
#define BCM1250_MAC_POLLING      0x0004  /* Poll mode, io mode */
#define BCM1250_MAC_PROMISC      0x0008  /* Promiscuous, rx mode */
#define BCM1250_MAC_MCAST        0x0010  /* Multicast, rx mode */

#define MAC0_REG_OFFSET         0x4000
#define MAC1_REG_OFFSET         0x5000
#define MAC2_REG_OFFSET         0x6000

typedef unsigned long sbmac_port_t;
typedef UINT64 sbmac_physaddr_t;
typedef UINT64 sbmac_enetaddr_t;

typedef enum 
    { 
    sbmac_speed_10, 
    sbmac_speed_100,
    sbmac_speed_1000 
    } sbmac_speed_t;
 
typedef enum 
    { 
    sbmac_duplex_half,
    sbmac_duplex_full 
    } sbmac_duplex_t;
 
typedef enum 
    { 
    sbmac_fc_disabled, 
    sbmac_fc_frame,
    sbmac_fc_collision, 
    sbmac_fc_carrier 
    } sbmac_fc_t;             

#define CACHELINESIZE 32

#define NUMCACHEBLKS(x) (((x)+CACHELINESIZE-1)/CACHELINESIZE)


typedef struct sbdmadscr_s 
    {
    UINT64  dscr_a;
    UINT64  dscr_b;
    } sbdmadscr_t, SB_DMA_DSCR;

typedef struct sbmacdma_s 
    {

    /*
     * This stuff is used to identify the channel and the registers
     * associated with it.
     */

    /* struct sbmac_softc *sbdma_eth; */ /* back pointer to associated MAC */
    int              sbdma_channel;     /* channel number */
    int              sbdma_txdir;       /* direction (1=transmit) */
    int              sbdma_maxdescr;    /* total # of descriptors in ring */

    sbmac_port_t     sbdma_config0;     /* DMA config register 0 */
    sbmac_port_t     sbdma_config1;     /* DMA config register 1 */
    sbmac_port_t     sbdma_dscrbase;    /* Descriptor base address */
    sbmac_port_t     sbdma_dscrcnt;     /* Descriptor count register */
    sbmac_port_t     sbdma_curdscr;     /* current descriptor address */

    /*
     * This stuff is for maintenance of the ring
     */

    sbdmadscr_t     *sbdma_dscrtable;   /* base of descriptor table */
    sbdmadscr_t     *sbdma_dscrtable_end;/* end of descriptor table */

    char           **sbdma_ctxtable;    /* context table, one per descr */
    UINT8           *sbdma_ctxtype;     /* context type table, one per descr */
    /*paddr_t       sbdma_dscrtable_phys;*/ /* and also the phys addr */
    sbdmadscr_t     *sbdma_addptr;      /* next dscr for sw to add */
    sbdmadscr_t     *sbdma_remptr;      /* next dscr for sw to remove */
    int              sbdma_addindex;
    int              sbdma_remindex;

    BOOL             sbdma_txblocked;
    BOOL             sbdma_txcleaning; 
    } SB_MAC_DMA;

typedef struct end_device 
    {
    END_OBJ          endObj;            /* The class we inherit from. */
    int              unit;              /* unit number */
    int              devNo;             /* Mac Device Number */

    int              ivecnum;           /* interrupt vector number */
    int              intsource;         /* interrupt source */

    long             flags;             /* Our local flags. */
    long             usrFlags;

    UCHAR            enetAddr[6];       /* ethernet address */

    sbmac_port_t     sbm_macbase;
    sbmac_port_t     sbm_dmabase;

    sbmac_port_t     sbm_macenable;     /* MAC Enable Register */
    sbmac_port_t     sbm_maccfg;        /* MAC Configuration Register */
    sbmac_port_t     sbm_fifocfg;       /* FIFO configuration register */
    sbmac_port_t     sbm_framecfg;      /* Frame configuration register */
    sbmac_port_t     sbm_rxfilter;      /* receive filter register */
    sbmac_port_t     sbm_isr;           /* Interrupt status register */
    sbmac_port_t     sbm_imr;           /* Interrupt mask register */
    sbmac_port_t     sbm_mdio;          /* mii regster */

    sbmac_speed_t    sbm_speed;
    sbmac_duplex_t   sbm_duplex;
    sbmac_fc_t       sbm_fc;

    int 	     phyAddr;	        /* phy address */
    char *           mClBlkBase;        /* Net Pool ClBlk, MBlk addr */
    ULONG            mClBlkSize;        /* Net Pool ClBlk, MBlk size */
    char *           bufBase;           /* Net Pool cluster buffer addr */
    ULONG            bufSize;           /* Net Pool cluster buffer size */

    CL_POOL_ID       clPoolId;          /* cluster pool */

    SB_MAC_DMA       txDma[2];
    SB_MAC_DMA       rxDma[2];

    int              rxDpc[2];
    int              txDpc[2];
    } DRV_CTRL;

#define BCM1250_MAC_SPEED_10     10000000             /* 10 Mbps */
#define BCM1250_MAC_SPEED_100    100000000            /* 100 Mbps */
#define BCM1250_MAC_SPEED_1000   1000000000           /* 1000 Mbps - 1Gbps */
#define BCM1250_MAC_SPEED_DEF    BCM1250_MAC_SPEED_10

#define BUF_TYPE_CL             0x1     /* this's a cluster pointer */
#define BUF_TYPE_MBLK           0x2     /* this's a mblk pointer */     

#define PKT_TYPE_MULTI          0x1     /* packet with a multicast address */
#define PKT_TYPE_UNI            0x2     /* packet with a unicast address */
#define PKT_TYPE_NONE           0x4     /* address type is not meaningful */   
 
#endif  /* _ASMLANGUAGE */
 
#ifdef __cplusplus
}
#endif
 
#endif /* __INCbcm1250MacEndh */              
